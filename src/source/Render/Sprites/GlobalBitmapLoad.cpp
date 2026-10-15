// Bitmap decoding, upload, registration and unload share the production bitmap state.
#include "stdafx.h"
#include "turbojpeg.h"
#include "Render/Sprites/GlobalBitmap.h"
#include "Core/Utilities/Log/MuLogger.h"
#include "Render/Renderer/BitmapUpload.h"
#include "Render/Renderer/MuRenderer.h"
#include "Render/Sprites/BitmapTextureLimits.h"
#include "GlobalBitmapPath.h"

#include "Render/Textures/MipChain.h"
#include "Render/Textures/OztFile.h"
#include "Render/Textures/TextureFiltering.h"
#ifdef _EDITOR
#include "Core/Utilities/AssetLoadWorld.h"
#endif

#include <SDL3/SDL_gpu.h>
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iterator>
#include <memory>
#include <span>
#include <string>
#include <vector>

namespace mu
{
void RegisterTexture(std::uint32_t id, void* texture);
void UnregisterTexture(std::uint32_t id);
void RegisterSampler(std::uint32_t id, void* sampler);
void UnregisterSampler(std::uint32_t id);
void ClearTextureRegistry();
void ClearSamplerRegistry();
void RegisterFilteredSampler(std::uint32_t id, void* sampler);
void MarkFilteredSamplerPending(std::uint32_t id);
void TakeFilteredSamplerRequests(std::vector<std::uint32_t>& requests);
void SetTextureFilteringEnabled(bool enabled);
} // namespace mu

namespace
{
class TurboJpegHandle
{
public:
    TurboJpegHandle() : handle(tjInitDecompress()) {}
    ~TurboJpegHandle()
    {
        if (handle != nullptr)
        {
            tjDestroy(handle);
        }
    }

    tjhandle get() const
    {
        return handle;
    }
    bool valid() const
    {
        return handle != nullptr;
    }

private:
    tjhandle handle;
};

void ReportTurboError(const wchar_t* context)
{
    const char* message = tjGetErrorStr();
    if (message == nullptr)
    {
        message = "Unknown TurboJPEG error";
    }
    g_ErrorReport.Write(L"[TurboJPEG] %ls: %hs", context, message);
}

int NextPowerOfTwo(int value, int maxValue)
{
    int result = 1;
    while (result < value && result < maxValue)
    {
        result <<= 1;
    }
    return std::min<int>(result, maxValue);
}

std::unique_ptr<BITMAP_t> CreateBitmapStorage(GLuint index, const std::wstring& filename, int width, int height,
                                              int components)
{
    auto bitmap = std::make_unique<BITMAP_t>();
    bitmap->BitmapIndex = index;
    wcsncpy(bitmap->FileName, filename.c_str(), MAX_BITMAP_FILE_NAME - 1);
    bitmap->FileName[MAX_BITMAP_FILE_NAME - 1] = L'\0';
    bitmap->Width = static_cast<float>(width);
    bitmap->Height = static_cast<float>(height);
    bitmap->Components = components;
    bitmap->Ref = 1;
    const auto bufferSize =
        static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * static_cast<std::size_t>(components);
    bitmap->BufferStorage.resize(bufferSize);
    bitmap->Buffer = bitmap->BufferStorage.data();
    return bitmap;
}

SDL_GPUFilter MapGLFilterToSDL(GLuint filter)
{
    return filter == GL_NEAREST ? SDL_GPU_FILTER_NEAREST : SDL_GPU_FILTER_LINEAR;
}

SDL_GPUSamplerAddressMode MapGLWrapToSDL(GLuint wrapMode)
{
    return wrapMode == GL_REPEAT ? SDL_GPU_SAMPLERADDRESSMODE_REPEAT : SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
}

std::vector<std::uint8_t> PadRGBToRGBA(const BYTE* rgbData, int width, int height)
{
    const auto pixelCount = static_cast<std::size_t>(width) * static_cast<std::size_t>(height);
    std::vector<std::uint8_t> rgba(pixelCount * 4u);
    for (std::size_t i = 0; i < pixelCount; ++i)
    {
        rgba[i * 4u + 0u] = rgbData[i * 3u + 0u];
        rgba[i * 4u + 1u] = rgbData[i * 3u + 1u];
        rgba[i * 4u + 2u] = rgbData[i * 3u + 2u];
        rgba[i * 4u + 3u] = 255u;
    }
    return rgba;
}

bool UploadTextureSDLGpu(BITMAP_t* bitmap, std::span<const std::uint8_t> pixels, int width, int height,
                         SDL_GPUFilter filter, SDL_GPUSamplerAddressMode wrapMode,
                         std::span<const Render::Textures::MipLevel> chain)
{
    auto result = mu::UploadBitmapPixels(pixels, width, height, filter, wrapMode, chain);
    if (!result)
    {
        const char* driver = mu::BitmapUploadDriverName();
        g_ErrorReport.Write(
            L"SDL bitmap upload failed at %hs: %hs (id=%u, asset=%ls, %dx%d, bytes=%zu, driver=%hs)\r\n",
            result.stage.c_str(), result.error.c_str(), bitmap->BitmapIndex, bitmap->FileName, width, height,
            pixels.size(), driver ? driver : "unknown");
        return false;
    }
    bitmap->sdlTexture = result.texture;
    bitmap->sdlSampler = result.sampler;
    bitmap->mipLevels = static_cast<std::uint8_t>(chain.size() + 1u);
    return true;
}

// Enhanced samplers cover the whole chain (Vulkan's VK_LOD_CLAMP_NONE).
constexpr float UnclampedMaxLod = 1000.0f;

// Build mip chains requested by world draws for at most this long per frame.
constexpr std::chrono::milliseconds MipChainBudgetPerFrame{4};

// The mip chain of a bitmap's RGBA8 pixels. Textures with an alpha channel
// keep their alpha-test coverage at every level; opaque ones are box filtered.
std::vector<Render::Textures::MipLevel> BuildBitmapMipChain(const std::uint8_t* rgba, int width, int height,
                                                            bool hasAlpha, double& seconds)
{
    const auto start = std::chrono::steady_clock::now();
    const std::span<const std::uint8_t> pixels(rgba, static_cast<std::size_t>(width) * height * 4u);
    std::vector<Render::Textures::MipLevel> chain =
        hasAlpha ? Render::Textures::BuildCoveragePreservingMipChain(
                       pixels, static_cast<std::uint32_t>(width), static_cast<std::uint32_t>(height),
                       Render::Textures::AlphaTest::FromReference(Render::Textures::AlphaTestReference))
                 : Render::Textures::BuildMipChain(pixels, static_cast<std::uint32_t>(width),
                                                   static_cast<std::uint32_t>(height));
    seconds += std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    return chain;
}

// GPU bytes of a bitmap's RGBA8 texture, all of its mip levels included.

std::uint64_t GpuTextureBytes(const BITMAP_t& bitmap)
{
    auto width = static_cast<std::uint32_t>(bitmap.Width);
    auto height = static_cast<std::uint32_t>(bitmap.Height);
    std::uint64_t bytes = 0;
    for (std::uint32_t level = 0; level < bitmap.mipLevels; ++level)
    {
        bytes += static_cast<std::uint64_t>(width) * height * 4u;
        width = std::max<std::uint32_t>(1u, width / 2u);
        height = std::max<std::uint32_t>(1u, height / 2u);
    }
    return bytes;
}
} // namespace

GLuint CGlobalBitmap::LoadImage(const std::wstring& filename, GLuint uiFilter, GLuint uiWrapMode)
{
    BITMAP_t* pBitmap = FindTexture(filename);
    if (pBitmap)
    {
        if (pBitmap->Ref > 0)
        {
            if (0 == _wcsicmp(pBitmap->FileName, filename.c_str()))
            {
                pBitmap->Ref++;

                return pBitmap->BitmapIndex;
            }
        }
    }
    else
    {
        GLuint uiNewTextureIndex = GenerateTextureIndex();
        if (true == LoadImage(uiNewTextureIndex, filename, uiFilter, uiWrapMode))
        {
            m_listNonamedIndex.push_back(uiNewTextureIndex);

            return uiNewTextureIndex;
        }
    }
    return BITMAP_UNKNOWN;
}
GLuint CGlobalBitmap::LoadImageExclusive(const std::wstring& filename, GLuint uiFilter, GLuint uiWrapMode,
                                         bool padToPowerOfTwo)
{
    GLuint uiNewTextureIndex = GenerateTextureIndex();
    if (true == LoadImage(uiNewTextureIndex, filename, uiFilter, uiWrapMode, padToPowerOfTwo))
    {
        m_listNonamedIndex.push_back(uiNewTextureIndex);
        return uiNewTextureIndex;
    }
    return BITMAP_UNKNOWN;
}
bool CGlobalBitmap::LoadImage(GLuint uiBitmapIndex, const std::wstring& filename, GLuint uiFilter, GLuint uiWrapMode,
                              bool padToPowerOfTwo)
{
    unsigned int UICLAMP = GL_CLAMP_TO_EDGE;
    unsigned int UIREPEAT = GL_REPEAT;

    if (uiWrapMode != UICLAMP && uiWrapMode != UIREPEAT)
    {
#ifdef _DEBUG
        static unsigned int uiCnt2 = 0;

        wchar_t szDebugOutput[256];

        mu_swprintf_s(szDebugOutput, L"%d. Call No CLAMP & No REPEAT. \n", uiCnt2++);
        OutputDebugString(szDebugOutput);
#endif
    }

    auto mi = m_mapBitmap.find(uiBitmapIndex);
    if (mi != m_mapBitmap.end())
    {
        BITMAP_t* pBitmap = mi->second.get();
        if (pBitmap->Ref > 0)
        {
            if (0 == _wcsicmp(pBitmap->FileName, filename.c_str()))
            {
                pBitmap->Ref++;
#ifdef _EDITOR
                // The last loader, as for models: a map that loads the file
                // again has loaded it too.
                m_loadWorlds[uiBitmapIndex] = Core::AssetLoadWorld::Get();
#endif
                return true;
            }
            else
            {
                mu::log::Get("render")->debug("SDL_gpu -- bitmap id {} replaced: {} -> {}", uiBitmapIndex,
                                              mu_wchar_to_utf8(pBitmap->FileName), mu_wchar_to_utf8(filename.c_str()));
                UnloadImage(uiBitmapIndex, true);
            }
        }
    }

    std::wstring ext;
    SplitExt(filename, ext, false);

    const bool worldTexture = Render::Textures::IsWorldTexturePath(filename);
    const auto start = std::chrono::steady_clock::now();
    bool loaded = false;
    if (0 == _wcsicmp(ext.c_str(), L"jpg"))
        loaded = OpenJpegTurbo(uiBitmapIndex, filename, uiFilter, uiWrapMode, worldTexture, padToPowerOfTwo);
    else if (0 == _wcsicmp(ext.c_str(), L"tga"))
        loaded = OpenTga(uiBitmapIndex, filename, uiFilter, uiWrapMode, worldTexture, padToPowerOfTwo);
    m_textureLoadSeconds += std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
#ifdef _EDITOR
    if (loaded)
        m_loadWorlds[uiBitmapIndex] = Core::AssetLoadWorld::Get();
#endif
    return loaded;
}
void CGlobalBitmap::UnloadImage(GLuint uiBitmapIndex, bool bForce)
{
    auto mi = m_mapBitmap.find(uiBitmapIndex);
    if (mi != m_mapBitmap.end())
    {
        BITMAP_t* pBitmap = mi->second.get();

        if (--pBitmap->Ref == 0 || bForce)
        {
            mu::UnregisterTexture(uiBitmapIndex);
            mu::UnregisterSampler(uiBitmapIndex);
            SDL_GPUDevice* device = static_cast<SDL_GPUDevice*>(mu::GetRenderer().GetDevice());
            if (device)
            {
                pBitmap->sdlSampler = nullptr;
                if (pBitmap->sdlTexture)
                {
                    mu::ReleaseBitmapTexture(device, pBitmap->sdlTexture);
                    pBitmap->sdlTexture = nullptr;
                }
            }

            const auto memoryUsed = static_cast<std::uint32_t>(pBitmap->BufferStorage.size());
            m_dwUsedTextureMemory -= memoryUsed;

            m_mapBitmap.erase(mi);
#ifdef _EDITOR
            m_loadWorlds.erase(uiBitmapIndex);
#endif

            if (uiBitmapIndex >= BITMAP_NONAMED_TEXTURES_BEGIN && uiBitmapIndex <= BITMAP_NONAMED_TEXTURES_END)
            {
                m_listNonamedIndex.remove(uiBitmapIndex);
            }
            m_BitmapCache.Remove(uiBitmapIndex);
        }
    }
}
#ifdef _EDITOR
std::optional<int> CGlobalBitmap::GetLoadWorld(GLuint uiBitmapIndex) const
{
    const auto found = m_loadWorlds.find(uiBitmapIndex);
    if (found == m_loadWorlds.end())
        return std::nullopt;
    return found->second;
}
#endif

void CGlobalBitmap::UnloadAllImages()
{
    if (m_mapBitmap.empty())
    {
        // The bitmaps may have been unloaded one by one; the shared Enhanced
        // samplers outlive them.
        ReleaseFilteredSamplers();
        m_mipChainRequests.clear();
        return;
    }

#ifdef _DEBUG
    if (!m_mapBitmap.empty())
        g_ErrorReport.Write(L"Unload Images\r\n");
#endif // _DEBUG

    SDL_GPUDevice* device = static_cast<SDL_GPUDevice*>(mu::GetRenderer().GetDevice());
    if (device)
    {
        for (auto& pair : m_mapBitmap)
        {
            BITMAP_t* pBitmap = pair.second.get();
            mu::UnregisterTexture(pBitmap->BitmapIndex);
            mu::UnregisterSampler(pBitmap->BitmapIndex);
            pBitmap->sdlSampler = nullptr;
            if (pBitmap->sdlTexture)
            {
                mu::ReleaseBitmapTexture(device, pBitmap->sdlTexture);
                pBitmap->sdlTexture = nullptr;
            }
        }
    }
    else
    {
        mu::ClearTextureRegistry();
        mu::ClearSamplerRegistry();
    }
    ReleaseFilteredSamplers();
    m_mipChainRequests.clear();

    for ([[maybe_unused]] auto& pair : m_mapBitmap)
    {
#ifdef _DEBUG
        BITMAP_t* pBitmap = pair.second.get();
        if (pBitmap->Ref > 1)
        {
            g_ErrorReport.Write(L"Bitmap %ls(RefCount= %d)\r\n", pBitmap->FileName, pBitmap->Ref);
        }
#endif // _DEBUG
    }

    m_mapBitmap.clear();
#ifdef _EDITOR
    m_loadWorlds.clear();
#endif
    m_listNonamedIndex.clear();
    m_BitmapCache.RemoveAll();

    Init();
}

GLuint CGlobalBitmap::GenerateTextureIndex()
{
    GLuint uiAvailableTextureIndex = FindAvailableTextureIndex(m_uiTextureIndexStream);
    if (uiAvailableTextureIndex >= BITMAP_NONAMED_TEXTURES_END)
    {
        m_uiAlternate++;
        m_uiTextureIndexStream = BITMAP_NONAMED_TEXTURES_BEGIN;
        uiAvailableTextureIndex = FindAvailableTextureIndex(m_uiTextureIndexStream);
    }
    return m_uiTextureIndexStream = uiAvailableTextureIndex;
}
GLuint CGlobalBitmap::FindAvailableTextureIndex(GLuint uiSeed)
{
    if (m_uiAlternate > 0)
    {
        auto li = std::find(m_listNonamedIndex.begin(), m_listNonamedIndex.end(), uiSeed + 1);
        if (li != m_listNonamedIndex.end())
            return FindAvailableTextureIndex(uiSeed + 1);
    }
    return uiSeed + 1;
}

bool CGlobalBitmap::OpenJpegTurbo(GLuint uiBitmapIndex, const std::wstring& filename, GLuint uiFilter,
                                  GLuint uiWrapMode, bool worldTexture, bool padToPowerOfTwo)
{
    std::wstring filename_ozj;
    ExchangeExt(filename, L"OZJ", filename_ozj);

    std::ifstream compressedFile(Render::Sprites::NarrowBitmapPath(filename_ozj), std::ios::binary);
    if (!compressedFile)
    {
        mu::log::Get("render")->debug("OpenJpegTurbo -- file open failed: {}", mu_wchar_to_utf8(filename_ozj.c_str()));
        return false;
    }

    std::vector<unsigned char> jpegBuf((std::istreambuf_iterator<char>(compressedFile)),
                                       std::istreambuf_iterator<char>());
    compressedFile.close();

    if (jpegBuf.size() <= 24)
    {
        g_ErrorReport.Write(L"OpenJpegTurbo: file too small %ls (%zu bytes)\r\n", filename_ozj.c_str(), jpegBuf.size());
        return false;
    }

    // Skip first 24 bytes (OZJ header)
    const unsigned char* jpegData = jpegBuf.data() + 24;
    const auto jpegSize = static_cast<unsigned long>(jpegBuf.size() - 24);

    int jpegWidth = 0, jpegHeight = 0;
    int jpegSubsamp = TJSAMP_444;
    int jpegColorspace = TJCS_RGB;

    TurboJpegHandle tjHandle;
    if (!tjHandle.valid())
    {
        ReportTurboError(L"tjInitDecompress");
        return false;
    }

    auto headerResult =
        tjDecompressHeader3(tjHandle.get(), jpegData, jpegSize, &jpegWidth, &jpegHeight, &jpegSubsamp, &jpegColorspace);
    if (headerResult != 0 || jpegWidth <= 0 || jpegHeight <= 0 || jpegWidth > Render::Sprites::kBitmapMaximumWidth ||
        jpegHeight > Render::Sprites::kBitmapMaximumHeight)
    {
        ReportTurboError(L"tjDecompressHeader3");
        return false;
    }

    std::vector<unsigned char> decompressedBuffer(static_cast<std::size_t>(jpegWidth) *
                                                  static_cast<std::size_t>(jpegHeight) * 3u);
    auto decompressResult = tjDecompress2(tjHandle.get(), jpegData, jpegSize, decompressedBuffer.data(), jpegWidth, 0,
                                          jpegHeight, TJPF_RGB, TJFLAG_FASTDCT);
    if (decompressResult != 0)
    {
        ReportTurboError(L"tjDecompress2");
        return false;
    }

    const int textureWidth =
        padToPowerOfTwo ? NextPowerOfTwo(jpegWidth, Render::Sprites::kBitmapMaximumWidth) : jpegWidth;
    const int textureHeight =
        padToPowerOfTwo ? NextPowerOfTwo(jpegHeight, Render::Sprites::kBitmapMaximumHeight) : jpegHeight;

    auto pNewBitmap = CreateBitmapStorage(uiBitmapIndex, filename, textureWidth, textureHeight, 3);
    const std::size_t textureBufferSize = pNewBitmap->BufferStorage.size();

    const int jpegRowSize = jpegWidth * 3;
    const int textureRowSize = textureWidth * 3;
    const int rows = std::min<int>(jpegHeight, textureHeight);

    std::size_t offset = 0;
    if (jpegWidth != textureWidth)
    {
        for (int row = 0; row < rows; ++row)
        {
            memcpy(&pNewBitmap->Buffer[offset], &decompressedBuffer[static_cast<std::size_t>(row) * jpegRowSize],
                   static_cast<std::size_t>(jpegRowSize));
            offset += static_cast<std::size_t>(textureRowSize);
        }
    }
    else
    {
        memcpy(pNewBitmap->Buffer, decompressedBuffer.data(),
               static_cast<std::size_t>(jpegHeight) * static_cast<std::size_t>(jpegWidth) * 3u);
    }

    pNewBitmap->TextureNumber = uiBitmapIndex;
    pNewBitmap->worldTexture = worldTexture;
    pNewBitmap->wrapRepeat = uiWrapMode == GL_REPEAT;
    std::vector<std::uint8_t> rgbaData = PadRGBToRGBA(pNewBitmap->Buffer, textureWidth, textureHeight);
    const std::vector<Render::Textures::MipLevel> chain =
        worldTexture && m_textureFilteringEnabled
            ? BuildBitmapMipChain(rgbaData.data(), textureWidth, textureHeight, false, m_mipChainSeconds)
            : std::vector<Render::Textures::MipLevel>{};
    if (!UploadTextureSDLGpu(pNewBitmap.get(), rgbaData, textureWidth, textureHeight, MapGLFilterToSDL(uiFilter),
                             MapGLWrapToSDL(uiWrapMode), chain))
    {
        return false;
    }

    SDL_GPUTexture* rawTexture = pNewBitmap->sdlTexture;
    SDL_GPUSampler* rawSampler = pNewBitmap->sdlSampler;
    const BITMAP_t& loaded = *pNewBitmap;
    m_mapBitmap.insert(type_bitmap_map::value_type(uiBitmapIndex, std::move(pNewBitmap)));
    mu::RegisterTexture(uiBitmapIndex, rawTexture);
    mu::RegisterSampler(uiBitmapIndex, rawSampler);
    RegisterFilteredSampler(loaded);
    m_dwUsedTextureMemory += static_cast<std::uint32_t>(textureBufferSize);

    return true;
}

bool CGlobalBitmap::OpenTga(GLuint uiBitmapIndex, const std::wstring& filename, GLuint uiFilter, GLuint uiWrapMode,
                            bool worldTexture, bool padToPowerOfTwo)
{
    namespace Ozt = Render::Textures::Ozt;

    std::wstring filename_ozt;
    ExchangeExt(filename, L"OZT", filename_ozt);

    std::ifstream input(Render::Sprites::NarrowBitmapPath(filename_ozt), std::ios::binary);
    if (!input)
    {
        mu::log::Get("render")->debug("OpenTga -- file open failed: {}", mu_wchar_to_utf8(filename_ozt.c_str()));
        return false;
    }

    std::vector<unsigned char> pakBuffer((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    input.close();

    // A file cut off in its header or its pixels is too small.
    const Ozt::Header header =
        Ozt::ReadHeader(pakBuffer, Render::Sprites::kBitmapMaximumWidth, Render::Sprites::kBitmapMaximumHeight);
    if (header.status == Ozt::Status::TooSmall)
    {
        g_ErrorReport.Write(L"OpenTga: file too small %ls (%zu bytes)\r\n", filename_ozt.c_str(), pakBuffer.size());
        return false;
    }
    if (header.status == Ozt::Status::InvalidFormat)
    {
        g_ErrorReport.Write(L"OpenTga: invalid format %ls (bit=%d, %dx%d)\r\n", filename_ozt.c_str(),
                            header.bitsPerPixel, header.width, header.height);
        return false;
    }

    const int Width =
        padToPowerOfTwo ? NextPowerOfTwo(header.width, Render::Sprites::kBitmapMaximumWidth) : header.width;
    const int Height =
        padToPowerOfTwo ? NextPowerOfTwo(header.height, Render::Sprites::kBitmapMaximumHeight) : header.height;

    auto pNewBitmap = CreateBitmapStorage(uiBitmapIndex, filename, Width, Height, 4);
    const std::size_t BufferSize = pNewBitmap->BufferStorage.size();

    Ozt::CopyPixels(pakBuffer, header, std::span<std::uint8_t>(pNewBitmap->Buffer, BufferSize), Width);

    pNewBitmap->TextureNumber = uiBitmapIndex;
    pNewBitmap->worldTexture = worldTexture;
    pNewBitmap->wrapRepeat = uiWrapMode == GL_REPEAT;
    const std::vector<Render::Textures::MipLevel> chain =
        worldTexture && m_textureFilteringEnabled
            ? BuildBitmapMipChain(pNewBitmap->Buffer, Width, Height, true, m_mipChainSeconds)
            : std::vector<Render::Textures::MipLevel>{};
    if (!UploadTextureSDLGpu(pNewBitmap.get(), pNewBitmap->BufferStorage, Width, Height, MapGLFilterToSDL(uiFilter),
                             MapGLWrapToSDL(uiWrapMode), chain))
    {
        return false;
    }

    SDL_GPUTexture* rawTexture = pNewBitmap->sdlTexture;
    SDL_GPUSampler* rawSampler = pNewBitmap->sdlSampler;
    const BITMAP_t& loaded = *pNewBitmap;
    m_mapBitmap.insert(type_bitmap_map::value_type(uiBitmapIndex, std::move(pNewBitmap)));
    mu::RegisterTexture(uiBitmapIndex, rawTexture);
    mu::RegisterSampler(uiBitmapIndex, rawSampler);
    RegisterFilteredSampler(loaded);
    m_dwUsedTextureMemory += static_cast<std::uint32_t>(BufferSize);

    return true;
}

void CGlobalBitmap::SetTextureFiltering(bool enabled, int anisotropy)
{
    m_textureFilteringRequested = enabled;
    m_requestedAnisotropy = anisotropy;
    m_textureFilteringSwitchPending = true;
}

void CGlobalBitmap::UpdateTextureFiltering()
{
    if (m_textureFilteringSwitchPending)
    {
        ApplyTextureFilteringSwitch();
    }
    if (!m_textureFilteringEnabled)
    {
        return;
    }

    mu::TakeFilteredSamplerRequests(m_mipChainRequests);
    if (m_mipChainRequests.empty())
    {
        return;
    }

    const auto start = std::chrono::steady_clock::now();
    std::size_t built = 0;
    while (built < m_mipChainRequests.size())
    {
        BuildMipChain(m_mipChainRequests[built++]);
        if (std::chrono::steady_clock::now() - start >= MipChainBudgetPerFrame)
        {
            break;
        }
    }
    m_mipChainRequests.erase(m_mipChainRequests.begin(),
                             m_mipChainRequests.begin() + static_cast<std::ptrdiff_t>(built));
}

void CGlobalBitmap::ApplyTextureFilteringSwitch()
{
    m_textureFilteringSwitchPending = false;

    const Render::Textures::AnisotropyLevel anisotropy = Render::Textures::ClampAnisotropy(m_requestedAnisotropy);
    if (anisotropy.clamped && !m_anisotropyClampLogged)
    {
        mu::log::Get("render")->warn("Texture filtering: anisotropy {} is above the highest level the GPU supports; "
                                     "using {}",
                                     m_requestedAnisotropy, anisotropy.value);
        m_anisotropyClampLogged = true;
    }
    if (anisotropy.value != m_anisotropy)
    {
        // The shared samplers bake in the level; rebuild them and point every
        // chained texture at the new ones.
        ReleaseFilteredSamplers();
        m_anisotropy = anisotropy.value;
        for (const auto& [index, bitmap] : m_mapBitmap)
        {
            RegisterFilteredSampler(*bitmap);
        }
    }

    const bool changed = m_textureFilteringEnabled != m_textureFilteringRequested;
    m_textureFilteringEnabled = m_textureFilteringRequested;
    mu::SetTextureFilteringEnabled(m_textureFilteringEnabled);
    m_mipChainRequests.clear();
    if (m_textureFilteringEnabled)
    {
        // World textures loaded while Classic have no chain yet: build each one
        // the first time a world draw uses it.
        for (const auto& [index, bitmap] : m_mapBitmap)
        {
            if (bitmap->worldTexture && bitmap->mipLevels <= 1u)
            {
                mu::MarkFilteredSamplerPending(index);
            }
        }
    }

    if (changed)
    {
        LogTextureFilteringSummary(m_textureFilteringEnabled ? "switched to Enhanced" : "switched to Classic");
    }
}

void CGlobalBitmap::RegisterFilteredSampler(const BITMAP_t& bitmap)
{
    if (!bitmap.worldTexture || bitmap.mipLevels <= 1u)
    {
        return;
    }
    SDL_GPUSampler* sampler = GetFilteredSampler(bitmap.wrapRepeat);
    if (sampler)
    {
        mu::RegisterFilteredSampler(bitmap.BitmapIndex, sampler);
    }
}

SDL_GPUSampler* CGlobalBitmap::GetFilteredSampler(bool wrapRepeat)
{
    SDL_GPUSampler*& sampler = m_filteredSamplers[wrapRepeat ? 1u : 0u];
    if (sampler)
    {
        return sampler;
    }

    SDL_GPUDevice* device = static_cast<SDL_GPUDevice*>(mu::GetRenderer().GetDevice());
    if (!device)
    {
        return nullptr;
    }

    const SDL_GPUSamplerAddressMode wrapMode =
        wrapRepeat ? SDL_GPU_SAMPLERADDRESSMODE_REPEAT : SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    const std::uint32_t anisotropy = std::max<std::uint32_t>(1u, m_anisotropy);

    SDL_GPUSamplerCreateInfo samplerInfo{};
    samplerInfo.min_filter = SDL_GPU_FILTER_LINEAR;
    samplerInfo.mag_filter = SDL_GPU_FILTER_LINEAR;
    samplerInfo.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_LINEAR;
    samplerInfo.address_mode_u = wrapMode;
    samplerInfo.address_mode_v = wrapMode;
    samplerInfo.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    samplerInfo.min_lod = 0.0f;
    samplerInfo.max_lod = UnclampedMaxLod;
    samplerInfo.enable_anisotropy = anisotropy > 1u;
    samplerInfo.max_anisotropy = static_cast<float>(anisotropy);

    sampler = SDL_CreateGPUSampler(device, &samplerInfo);
    if (!sampler)
    {
        mu::log::Get("render")->error("Texture filtering: creating the Enhanced sampler failed: {}", SDL_GetError());
    }
    return sampler;
}

void CGlobalBitmap::ReleaseFilteredSamplers()
{
    SDL_GPUDevice* device = static_cast<SDL_GPUDevice*>(mu::GetRenderer().GetDevice());
    for (SDL_GPUSampler*& sampler : m_filteredSamplers)
    {
        if (sampler && device)
        {
            SDL_ReleaseGPUSampler(device, sampler);
        }
        sampler = nullptr;
    }
}

bool CGlobalBitmap::BuildMipChain(GLuint uiBitmapIndex)
{
    auto found = m_mapBitmap.find(uiBitmapIndex);
    if (found == m_mapBitmap.end())
    {
        return false;
    }
    BITMAP_t& bitmap = *found->second;
    const auto width = static_cast<int>(bitmap.Width);
    const auto height = static_cast<int>(bitmap.Height);
    SDL_GPUDevice* device = static_cast<SDL_GPUDevice*>(mu::GetRenderer().GetDevice());
    if (!device || !bitmap.worldTexture || bitmap.mipLevels > 1u || !bitmap.Buffer || width <= 0 || height <= 0 ||
        (bitmap.Components != 3 && bitmap.Components != 4))
    {
        return false;
    }

    const bool hasAlpha = bitmap.Components == 4;
    const std::vector<std::uint8_t> padded =
        hasAlpha ? std::vector<std::uint8_t>{} : PadRGBToRGBA(bitmap.Buffer, width, height);
    const std::uint8_t* rgba = hasAlpha ? bitmap.Buffer : padded.data();
    const std::vector<Render::Textures::MipLevel> chain =
        BuildBitmapMipChain(rgba, width, height, hasAlpha, m_mipChainSeconds);

    // Only the texture is replaced: the bitmap keeps its Classic sampler, so the
    // filter passed here does not matter.
    const auto result = mu::UploadBitmapPixels(
        std::span<const std::uint8_t>(rgba, static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * 4u),
        width, height, SDL_GPU_FILTER_LINEAR,
        bitmap.wrapRepeat ? SDL_GPU_SAMPLERADDRESSMODE_REPEAT : SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE, chain);
    if (!result)
    {
        mu::log::Get("render")->error("Texture filtering: creating the mipmapped texture {} failed at {}: {}",
                                      mu_wchar_to_utf8(bitmap.FileName), result.stage, result.error);
        return false;
    }
    SDL_GPUTexture* texture = result.texture;

    // Runs between frames, so no recorded draw still refers to the old texture.
    SDL_GPUTexture* previous = bitmap.sdlTexture;
    bitmap.sdlTexture = texture;
    bitmap.mipLevels = static_cast<std::uint8_t>(chain.size() + 1u);
    mu::RegisterTexture(uiBitmapIndex, texture);
    if (previous)
    {
        mu::ReleaseBitmapTexture(device, previous);
    }
    RegisterFilteredSampler(bitmap);
    return true;
}

void CGlobalBitmap::LogTextureFilteringSummary(const char* context) const
{
    std::size_t world = 0;
    std::size_t mipmapped = 0;
    std::size_t screenSpace = 0;
    std::uint64_t gpuBytes = 0;
    for (const auto& [index, bitmap] : m_mapBitmap)
    {
        if (bitmap->worldTexture)
        {
            ++world;
        }
        else
        {
            ++screenSpace;
        }
        if (bitmap->mipLevels > 1u)
        {
            ++mipmapped;
        }
        gpuBytes += GpuTextureBytes(*bitmap);
    }

    mu::log::Get("render")->info(
        "Texture filtering ({}): {}, anisotropy {}; {} world textures ({} mipmapped), {} screen-space; "
        "{:.1f} MiB on the GPU; texture loading {:.0f} ms, of which mip chains {:.0f} ms",
        context, m_textureFilteringEnabled ? "Enhanced" : "Classic", m_anisotropy, world, mipmapped, screenSpace,
        static_cast<double>(gpuBytes) / (1024.0 * 1024.0), m_textureLoadSeconds * 1000.0, m_mipChainSeconds * 1000.0);
}

void CGlobalBitmap::SplitExt(IN const std::wstring& filepath, OUT std::wstring& ext, bool bIncludeDot)
{
    wchar_t __ext[_MAX_EXT] = {
        0,
    };
    _wsplitpath(filepath.c_str(), NULL, NULL, NULL, __ext);
    if (bIncludeDot)
    {
        ext = __ext;
    }
    else
    {
        if ((__ext[0] == '.') && __ext[1])
            ext = __ext + 1;
    }
}
void CGlobalBitmap::ExchangeExt(IN const std::wstring& in_filepath, IN const std::wstring& ext,
                                OUT std::wstring& out_filepath)
{
    wchar_t __drive[_MAX_DRIVE] = {
        0,
    };
    wchar_t __dir[_MAX_DIR] = {
        0,
    };
    wchar_t __fname[_MAX_FNAME] = {
        0,
    };
    _wsplitpath(in_filepath.c_str(), __drive, __dir, __fname, NULL);

    out_filepath = __drive;
    out_filepath += __dir;
    out_filepath += __fname;
    out_filepath += '.';
    out_filepath += ext;
}
