// Bitmap decoding, upload, registration and unload share the production bitmap state.
#include "stdafx.h"
#include "turbojpeg.h"
#include "Render/Sprites/GlobalBitmap.h"
#include "Core/Utilities/Log/MuLogger.h"
#include "Render/Renderer/BitmapUpload.h"
#include "Render/Renderer/MuRenderer.h"
#include "Render/Sprites/BitmapTextureLimits.h"
#include "Render/Sprites/GpuUploadSkip.h"
#include "GlobalBitmapPath.h"

#include "Render/RmlUi/RmlPremultiply.h"
#include "Render/Textures/MipChain.h"
#include "Render/Textures/OztFile.h"
#include "Render/Textures/TextureFiltering.h"
#include "Render/Textures/TexturePack.h"
#include "Render/Textures/UpscaleMarker.h"
#ifdef _EDITOR
#include "Core/Utilities/AssetLoadWorld.h"
#endif

#include <SDL3/SDL_gpu.h>
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <system_error>
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

void ReportTurboError(const wchar_t* context, const std::wstring& path)
{
    const char* message = tjGetErrorStr();
    if (message == nullptr)
    {
        message = "Unknown TurboJPEG error";
    }
    g_ErrorReport.Write(L"[TurboJPEG] %ls %ls: %hs", context, path.c_str(), message);
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

std::optional<TextureFileBytes> ReadTextureFile(std::ifstream input, std::wstring path, bool fromTexturePack)
{
    // is_open, not operator bool: a stream that was never opened has no
    // error flag set.
    if (!input.is_open())
    {
        return std::nullopt;
    }
    TextureFileBytes file;
    file.bytes.assign(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
    file.path = std::move(path);
    file.fromTexturePack = fromTexturePack;
    return file;
}

std::optional<TextureFileBytes> ReadOriginal(const std::wstring& path)
{
    return ReadTextureFile(std::ifstream(Render::Sprites::NarrowBitmapPath(path), std::ios::binary), path, false);
}

// The copy of a texture file in the active pack; nothing when no pack is
// active or the pack has no such file.
std::optional<TextureFileBytes> ReadPackCopy(const Render::Textures::TexturePackFiles& pack, const std::wstring& path)
{
    const std::filesystem::path* copy = pack.Find(path);
    if (copy == nullptr)
    {
        return std::nullopt;
    }
    const std::u8string utf8 = copy->u8string();
    std::wstring copyPath = mu_utf8_to_wchar(std::string(utf8.begin(), utf8.end()));
    return ReadTextureFile(std::ifstream(*copy, std::ios::binary), std::move(copyPath), true);
}

// Loads a texture file through `load`: the copy in the active pack first, and
// the original in Data when the pack has no copy or its copy does not load,
// so a broken file in a pack costs only that file's replacement.
template <typename Load>
bool LoadPackCopyOrOriginal(const Render::Textures::TexturePackFiles& pack, const std::wstring& path,
                            const char* loader, Load&& load)
{
    if (const std::optional<TextureFileBytes> packCopy = ReadPackCopy(pack, path))
    {
        if (load(*packCopy))
        {
            mu::log::Get("render")->debug("{} -- read from the texture pack: {}", loader,
                                          mu_wchar_to_utf8(packCopy->path.c_str()));
            return true;
        }
        mu::log::Get("render")->warn("{} -- the texture pack's copy did not load, using the original: {}", loader,
                                     mu_wchar_to_utf8(packCopy->path.c_str()));
    }

    const std::optional<TextureFileBytes> original = ReadOriginal(path);
    if (!original)
    {
        mu::log::Get("render")->debug("{} -- file open failed: {}", loader, mu_wchar_to_utf8(path.c_str()));
        return false;
    }
    return load(*original);
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

// Nothing reads the CPU copy of a texture after the upload except for the
// canvases (LoadCanvasBitmap), which are redrawn through Buffer. The copy of an
// upscaled texture is up to 16 times the original; keeping the copies took the
// 32 bit client past 3.3 GB of address space.
void ReleaseUploadedPixels(BITMAP_t& bitmap)
{
    std::vector<std::uint8_t>().swap(bitmap.BufferStorage);
    bitmap.Buffer = nullptr;
}

// The size of the base level of an uploaded texture, which the loader counts as
// texture memory.
std::uint32_t UploadedBytes(int width, int height)
{
    return static_cast<std::uint32_t>(width) * static_cast<std::uint32_t>(height) * 4u;
}

// A canvas is drawn on through Buffer at the original size, so an upscaled file
// loaded into one is brought back to that size. Width and height become the
// size to upload.
void ShrinkCanvas(BITMAP_t& bitmap, int& width, int& height, int scale)
{
    bitmap.BufferStorage =
        Render::Textures::Upscale::ShrinkToOriginal(bitmap.BufferStorage, width, height, bitmap.Components, scale);
    bitmap.Buffer = bitmap.BufferStorage.data();
    width /= scale;
    height /= scale;
}

// The levels below the image: built on the CPU (`chain`) or by the GPU (`generate`).
struct MipLevels
{
    std::vector<Render::Textures::MipLevel> chain;
    bool generate = false;
};

// Set by tests only (Render::Sprites::SetGpuUploadsSkipped).
bool s_gpuUploadsSkipped = false;

bool UploadTextureSDLGpu(BITMAP_t* bitmap, std::span<const std::uint8_t> pixels, int width, int height,
                         SDL_GPUFilter filter, SDL_GPUSamplerAddressMode wrapMode, const MipLevels& mips)
{
    if (s_gpuUploadsSkipped)
    {
        bitmap->mipLevels = 1;
        return true;
    }
    auto result = mu::UploadBitmapPixels(pixels, width, height, filter, wrapMode, mips.chain, mips.generate);
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
    bitmap->mipLevels = static_cast<std::uint8_t>(
        mips.generate ? Render::Textures::CountMipLevels(static_cast<std::uint32_t>(width),
                                                         static_cast<std::uint32_t>(height))
                      : mips.chain.size() + 1u);
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

// Upscaled art, and world textures while texture filtering is on, get every mip
// level. The GPU builds them for an opaque image, box filtered as the CPU would;
// an image with alpha keeps its alpha-test coverage at every level on the CPU.
MipLevels PlanMipLevels(bool mipmapped, const std::uint8_t* rgba, int width, int height, bool hasAlpha,
                        double& seconds)
{
    if (!mipmapped)
    {
        return {};
    }
    if (!hasAlpha)
    {
        return {{}, true};
    }
    return {BuildBitmapMipChain(rgba, width, height, true, seconds), false};
}

// GPU bytes of a bitmap's RGBA8 texture, all of its mip levels included.

std::uint64_t GpuTextureBytes(const BITMAP_t& bitmap)
{
    auto width = static_cast<std::uint32_t>(bitmap.Width) * bitmap.scale;
    auto height = static_cast<std::uint32_t>(bitmap.Height) * bitmap.scale;
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

void Render::Sprites::SetGpuUploadsSkipped(bool skipped)
{
    s_gpuUploadsSkipped = skipped;
}

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
                                         bool padToPowerOfTwo, bool premultiplyAlpha)
{
    GLuint uiNewTextureIndex = GenerateTextureIndex();
    if (true == LoadImage(uiNewTextureIndex, filename, uiFilter, uiWrapMode, padToPowerOfTwo, premultiplyAlpha))
    {
        m_listNonamedIndex.push_back(uiNewTextureIndex);
        return uiNewTextureIndex;
    }
    return BITMAP_UNKNOWN;
}
bool CGlobalBitmap::LoadImage(GLuint uiBitmapIndex, const std::wstring& filename, GLuint uiFilter, GLuint uiWrapMode,
                              bool padToPowerOfTwo, bool premultiplyAlpha, bool keepPixels)
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
        loaded =
            OpenJpegTurbo(uiBitmapIndex, filename, uiFilter, uiWrapMode, worldTexture, padToPowerOfTwo, keepPixels);
    else if (0 == _wcsicmp(ext.c_str(), L"tga"))
        loaded = OpenTga(uiBitmapIndex, filename, uiFilter, uiWrapMode, worldTexture, padToPowerOfTwo,
                         premultiplyAlpha, keepPixels);
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

            m_dwUsedTextureMemory -= pBitmap->UploadedBytes;

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
                                  GLuint uiWrapMode, bool worldTexture, bool padToPowerOfTwo, bool keepPixels)
{
    std::wstring filename_ozj;
    ExchangeExt(filename, L"OZJ", filename_ozj);

    return LoadPackCopyOrOriginal(m_texturePack, filename_ozj, "OpenJpegTurbo",
                                  [&](const TextureFileBytes& file)
                                  {
                                      return LoadJpegFile(uiBitmapIndex, filename, file, uiFilter, uiWrapMode,
                                                          worldTexture, padToPowerOfTwo, keepPixels);
                                  });
}

bool CGlobalBitmap::LoadJpegFile(GLuint uiBitmapIndex, const std::wstring& filename, const TextureFileBytes& file,
                                 GLuint uiFilter, GLuint uiWrapMode, bool worldTexture, bool padToPowerOfTwo,
                                 bool keepPixels)
{
    const std::vector<unsigned char>& jpegBuf = file.bytes;
    if (jpegBuf.size() <= 24)
    {
        g_ErrorReport.Write(L"OpenJpegTurbo: file too small %ls (%zu bytes)\r\n", file.path.c_str(), jpegBuf.size());
        return false;
    }

    // A marked file holds an image up to its scale times the largest original.
    const int markedScale = Render::Textures::Upscale::ReadMarkedScale(jpegBuf);
    const int maxWidth = Render::Sprites::kBitmapMaximumWidth * markedScale;
    const int maxHeight = Render::Sprites::kBitmapMaximumHeight * markedScale;

    // Skip first 24 bytes (OZJ header)
    const unsigned char* jpegData = jpegBuf.data() + 24;
    const auto jpegSize = static_cast<unsigned long>(jpegBuf.size() - 24);

    int jpegWidth = 0, jpegHeight = 0;
    int jpegSubsamp = TJSAMP_444;
    int jpegColorspace = TJCS_RGB;

    TurboJpegHandle tjHandle;
    if (!tjHandle.valid())
    {
        ReportTurboError(L"tjInitDecompress", file.path);
        return false;
    }

    auto headerResult =
        tjDecompressHeader3(tjHandle.get(), jpegData, jpegSize, &jpegWidth, &jpegHeight, &jpegSubsamp, &jpegColorspace);
    if (headerResult != 0 || jpegWidth <= 0 || jpegHeight <= 0)
    {
        ReportTurboError(L"tjDecompressHeader3", file.path);
        return false;
    }
    if (jpegWidth > maxWidth || jpegHeight > maxHeight)
    {
        g_ErrorReport.Write(L"OpenJpegTurbo: image too large %ls (%dx%d, at most %dx%d)\r\n", file.path.c_str(),
                            jpegWidth, jpegHeight, maxWidth, maxHeight);
        return false;
    }

    std::vector<unsigned char> decompressedBuffer(static_cast<std::size_t>(jpegWidth) *
                                                  static_cast<std::size_t>(jpegHeight) * 3u);
    auto decompressResult = tjDecompress2(tjHandle.get(), jpegData, jpegSize, decompressedBuffer.data(), jpegWidth, 0,
                                          jpegHeight, TJPF_RGB, TJFLAG_FASTDCT);
    if (decompressResult != 0)
    {
        ReportTurboError(L"tjDecompress2", file.path);
        return false;
    }

    const int textureWidth = padToPowerOfTwo ? NextPowerOfTwo(jpegWidth, maxWidth) : jpegWidth;
    const int textureHeight = padToPowerOfTwo ? NextPowerOfTwo(jpegHeight, maxHeight) : jpegHeight;
    const int scale = Render::Textures::Upscale::ReadScale(jpegBuf, textureWidth, textureHeight);
    const bool upscaled = scale > Render::Textures::Upscale::NoScale && !keepPixels;

    auto pNewBitmap = CreateBitmapStorage(uiBitmapIndex, filename, textureWidth, textureHeight, 3);
    pNewBitmap->Width = static_cast<float>(textureWidth / scale);
    pNewBitmap->Height = static_cast<float>(textureHeight / scale);

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

    int uploadWidth = textureWidth;
    int uploadHeight = textureHeight;
    if (keepPixels && scale > Render::Textures::Upscale::NoScale)
    {
        ShrinkCanvas(*pNewBitmap, uploadWidth, uploadHeight, scale);
    }

    pNewBitmap->TextureNumber = uiBitmapIndex;
    pNewBitmap->worldTexture = worldTexture;
    pNewBitmap->wrapRepeat = uiWrapMode == GL_REPEAT;
    pNewBitmap->scale = static_cast<std::uint8_t>(upscaled ? scale : 1);
    pNewBitmap->fromTexturePack = file.fromTexturePack;
    std::vector<std::uint8_t> rgbaData = PadRGBToRGBA(pNewBitmap->Buffer, uploadWidth, uploadHeight);
    const MipLevels mips = PlanMipLevels(upscaled || (worldTexture && m_textureFilteringEnabled), rgbaData.data(),
                                         uploadWidth, uploadHeight, false, m_mipChainSeconds);
    if (!UploadTextureSDLGpu(pNewBitmap.get(), rgbaData, uploadWidth, uploadHeight, MapGLFilterToSDL(uiFilter),
                             MapGLWrapToSDL(uiWrapMode), mips))
    {
        return false;
    }
    pNewBitmap->UploadedBytes = UploadedBytes(uploadWidth, uploadHeight);
    if (!keepPixels)
    {
        ReleaseUploadedPixels(*pNewBitmap);
    }

    SDL_GPUTexture* rawTexture = pNewBitmap->sdlTexture;
    SDL_GPUSampler* rawSampler = pNewBitmap->sdlSampler;
    const BITMAP_t& loaded = *pNewBitmap;
    const std::uint32_t uploadedBytes = pNewBitmap->UploadedBytes;
    m_mapBitmap.insert(type_bitmap_map::value_type(uiBitmapIndex, std::move(pNewBitmap)));
    mu::RegisterTexture(uiBitmapIndex, rawTexture);
    mu::RegisterSampler(uiBitmapIndex, rawSampler);
    RegisterFilteredSampler(loaded);
    m_dwUsedTextureMemory += uploadedBytes;

    return true;
}

bool CGlobalBitmap::OpenTga(GLuint uiBitmapIndex, const std::wstring& filename, GLuint uiFilter, GLuint uiWrapMode,
                            bool worldTexture, bool padToPowerOfTwo, bool premultiplyAlpha, bool keepPixels)
{
    std::wstring filename_ozt;
    ExchangeExt(filename, L"OZT", filename_ozt);

    return LoadPackCopyOrOriginal(m_texturePack, filename_ozt, "OpenTga",
                                  [&](const TextureFileBytes& file)
                                  {
                                      return LoadTgaFile(uiBitmapIndex, filename, file, uiFilter, uiWrapMode,
                                                         worldTexture, padToPowerOfTwo, premultiplyAlpha, keepPixels);
                                  });
}

bool CGlobalBitmap::LoadTgaFile(GLuint uiBitmapIndex, const std::wstring& filename, const TextureFileBytes& file,
                                GLuint uiFilter, GLuint uiWrapMode, bool worldTexture, bool padToPowerOfTwo,
                                bool premultiplyAlpha, bool keepPixels)
{
    namespace Ozt = Render::Textures::Ozt;

    const std::vector<unsigned char>& pakBuffer = file.bytes;

    // A marked file holds an image up to its scale times the largest original.
    const int markedScale = Render::Textures::Upscale::ReadMarkedScale(pakBuffer);
    const int maxWidth = Render::Sprites::kBitmapMaximumWidth * markedScale;
    const int maxHeight = Render::Sprites::kBitmapMaximumHeight * markedScale;

    // A file cut off in its header or its pixels is too small.
    const Ozt::Header header = Ozt::ReadHeader(pakBuffer, maxWidth, maxHeight);
    if (header.status == Ozt::Status::TooSmall)
    {
        g_ErrorReport.Write(L"OpenTga: file too small %ls (%zu bytes)\r\n", file.path.c_str(), pakBuffer.size());
        return false;
    }
    if (header.status == Ozt::Status::InvalidFormat)
    {
        g_ErrorReport.Write(L"OpenTga: invalid format %ls (bit=%d, %dx%d, at most %dx%d)\r\n", file.path.c_str(),
                            header.bitsPerPixel, header.width, header.height, maxWidth, maxHeight);
        return false;
    }

    const int Width = padToPowerOfTwo ? NextPowerOfTwo(header.width, maxWidth) : header.width;
    const int Height = padToPowerOfTwo ? NextPowerOfTwo(header.height, maxHeight) : header.height;
    const int scale = Render::Textures::Upscale::ReadScale(pakBuffer, Width, Height);
    const bool upscaled = scale > Render::Textures::Upscale::NoScale && !keepPixels;

    auto pNewBitmap = CreateBitmapStorage(uiBitmapIndex, filename, Width, Height, 4);
    pNewBitmap->Width = static_cast<float>(Width / scale);
    pNewBitmap->Height = static_cast<float>(Height / scale);
    const std::size_t BufferSize = pNewBitmap->BufferStorage.size();

    Ozt::CopyPixels(pakBuffer, header, std::span<std::uint8_t>(pNewBitmap->Buffer, BufferSize), Width);
    if (premultiplyAlpha)
    {
        Render::RmlUi::PremultiplyAlpha(pNewBitmap->Buffer, BufferSize / Ozt::BytesPerPixel);
    }

    int uploadWidth = Width;
    int uploadHeight = Height;
    if (keepPixels && scale > Render::Textures::Upscale::NoScale)
    {
        ShrinkCanvas(*pNewBitmap, uploadWidth, uploadHeight, scale);
    }

    pNewBitmap->TextureNumber = uiBitmapIndex;
    pNewBitmap->worldTexture = worldTexture;
    pNewBitmap->wrapRepeat = uiWrapMode == GL_REPEAT;
    pNewBitmap->scale = static_cast<std::uint8_t>(upscaled ? scale : 1);
    pNewBitmap->fromTexturePack = file.fromTexturePack;
    const MipLevels mips = PlanMipLevels(upscaled || (worldTexture && m_textureFilteringEnabled), pNewBitmap->Buffer,
                                         uploadWidth, uploadHeight, true, m_mipChainSeconds);
    if (!UploadTextureSDLGpu(pNewBitmap.get(), pNewBitmap->BufferStorage, uploadWidth, uploadHeight,
                             MapGLFilterToSDL(uiFilter), MapGLWrapToSDL(uiWrapMode), mips))
    {
        return false;
    }
    pNewBitmap->UploadedBytes = UploadedBytes(uploadWidth, uploadHeight);
    if (!keepPixels)
    {
        ReleaseUploadedPixels(*pNewBitmap);
    }

    SDL_GPUTexture* rawTexture = pNewBitmap->sdlTexture;
    SDL_GPUSampler* rawSampler = pNewBitmap->sdlSampler;
    const BITMAP_t& loaded = *pNewBitmap;
    const std::uint32_t uploadedBytes = pNewBitmap->UploadedBytes;
    m_mapBitmap.insert(type_bitmap_map::value_type(uiBitmapIndex, std::move(pNewBitmap)));
    mu::RegisterTexture(uiBitmapIndex, rawTexture);
    mu::RegisterSampler(uiBitmapIndex, rawSampler);
    RegisterFilteredSampler(loaded);
    m_dwUsedTextureMemory += uploadedBytes;

    return true;
}

// The upload of a model file's image (LoadImagePixels). Its pixels stay on the bitmap: ReadBitmapPixels
// cannot read them back from a file when texture filtering later wants their mip chain.
bool CGlobalBitmap::UploadBitmap(std::unique_ptr<BITMAP_t> bitmap, std::span<const std::uint8_t> rgba, GLuint uiFilter,
                                 GLuint uiWrapMode)
{
    const bool hasAlpha = bitmap->Components == 4;
    const GLuint uiBitmapIndex = bitmap->BitmapIndex;
    const int width = static_cast<int>(bitmap->Width);
    const int height = static_cast<int>(bitmap->Height);
    bitmap->TextureNumber = uiBitmapIndex;
    bitmap->wrapRepeat = uiWrapMode == GL_REPEAT;
    const MipLevels mips = PlanMipLevels(bitmap->worldTexture && m_textureFilteringEnabled, rgba.data(), width, height,
                                         hasAlpha, m_mipChainSeconds);
    if (!UploadTextureSDLGpu(bitmap.get(), rgba, width, height, MapGLFilterToSDL(uiFilter), MapGLWrapToSDL(uiWrapMode),
                             mips))
    {
        return false;
    }
    bitmap->UploadedBytes = UploadedBytes(width, height);

    SDL_GPUTexture* rawTexture = bitmap->sdlTexture;
    SDL_GPUSampler* rawSampler = bitmap->sdlSampler;
    const BITMAP_t& loaded = *bitmap;
    const std::uint32_t uploadedBytes = bitmap->UploadedBytes;
    m_mapBitmap.insert(type_bitmap_map::value_type(uiBitmapIndex, std::move(bitmap)));
    mu::RegisterTexture(uiBitmapIndex, rawTexture);
    mu::RegisterSampler(uiBitmapIndex, rawSampler);
    RegisterFilteredSampler(loaded);
    m_dwUsedTextureMemory += uploadedBytes;

    return true;
}

GLuint CGlobalBitmap::LoadImagePixels(const std::wstring& name, std::span<const std::uint8_t> rgba, int width,
                                      int height, bool hasAlpha, GLuint uiFilter, GLuint uiWrapMode)
{
    constexpr std::size_t RgbaComponents = 4;
    constexpr std::size_t RgbComponents = 3;
    const std::size_t pixelCount = static_cast<std::size_t>(width) * static_cast<std::size_t>(height);
    if (width <= 0 || height <= 0 || rgba.size() != pixelCount * RgbaComponents)
    {
        return BITMAP_UNKNOWN;
    }
    if (BITMAP_t* loaded = FindTexture(name); loaded && 0 == _wcsicmp(loaded->FileName, name.c_str()))
    {
        loaded->Ref++;
        return loaded->BitmapIndex;
    }

    const GLuint uiBitmapIndex = GenerateTextureIndex();
    auto bitmap = std::make_unique<BITMAP_t>();
    bitmap->BitmapIndex = uiBitmapIndex;
    wcsncpy(bitmap->FileName, name.c_str(), MAX_BITMAP_FILE_NAME - 1);
    bitmap->FileName[MAX_BITMAP_FILE_NAME - 1] = L'\0';
    bitmap->Width = static_cast<float>(width);
    bitmap->Height = static_cast<float>(height);
    bitmap->Ref = 1;
    bitmap->worldTexture = Render::Textures::IsWorldTexturePath(name);
    // Like a .jpg (RGB) or a .tga (RGBA) texture: Components 4 is what makes a model mesh blend.
    bitmap->Components = hasAlpha ? static_cast<int>(RgbaComponents) : static_cast<int>(RgbComponents);
    if (hasAlpha)
    {
        bitmap->BufferStorage.assign(rgba.begin(), rgba.end());
    }
    else
    {
        bitmap->BufferStorage.resize(pixelCount * RgbComponents);
        for (std::size_t pixel = 0; pixel < pixelCount; ++pixel)
        {
            std::memcpy(&bitmap->BufferStorage[pixel * RgbComponents], &rgba[pixel * RgbaComponents], RgbComponents);
        }
    }
    bitmap->Buffer = bitmap->BufferStorage.data();
    if (!UploadBitmap(std::move(bitmap), rgba, uiFilter, uiWrapMode))
    {
        return BITMAP_UNKNOWN;
    }
    m_listNonamedIndex.push_back(uiBitmapIndex);
#ifdef _EDITOR
    m_loadWorlds[uiBitmapIndex] = Core::AssetLoadWorld::Get();
#endif
    return uiBitmapIndex;
}

void CGlobalBitmap::SetTextureFiltering(bool enabled, int anisotropy)
{
    m_textureFilteringRequested = enabled;
    m_requestedAnisotropy = anisotropy;
    m_textureFilteringSwitchPending = true;
}

void CGlobalBitmap::SetTexturePack(const std::wstring& pack)
{
    namespace TexturePack = Render::Textures::TexturePack;

    m_texturePack = {};
    if (pack.empty())
    {
        return;
    }
    if (!TexturePack::IsValidName(pack))
    {
        mu::log::Get("render")->warn("Texture pack name not valid, using the original textures: {}",
                                     mu_wchar_to_utf8(pack.c_str()));
        return;
    }

    const std::wstring folder = TexturePack::PackFolder(pack);
    const std::filesystem::path root = Render::Sprites::NarrowBitmapPath(folder);
    std::error_code error;
    if (!std::filesystem::is_directory(root, error))
    {
        mu::log::Get("render")->warn("Texture pack not found, using the original textures: {}",
                                     mu_wchar_to_utf8(folder.c_str()));
        return;
    }

    m_texturePack = Render::Textures::TexturePackFiles::List(root);
    mu::log::Get("render")->info("Texture pack: {} ({} files)", mu_wchar_to_utf8(pack.c_str()), m_texturePack.Count());
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
    const bool upscaled = bitmap.scale > 1u;
    if ((!bitmap.worldTexture && !upscaled) || bitmap.mipLevels <= 1u)
    {
        return;
    }
    SDL_GPUSampler* sampler = GetFilteredSampler(bitmap.wrapRepeat);
    if (!sampler)
    {
        return;
    }
    if (bitmap.worldTexture)
    {
        mu::RegisterFilteredSampler(bitmap.BitmapIndex, sampler);
    }
    // Upscaled art is drawn smaller than it is stored in both looks.
    if (upscaled)
    {
        mu::RegisterSampler(bitmap.BitmapIndex, sampler);
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
    // The static Bitmaps runs this again at exit, after the renderer and the logger are gone.
    if (std::none_of(m_filteredSamplers.begin(), m_filteredSamplers.end(),
                     [](const SDL_GPUSampler* sampler) { return sampler != nullptr; }))
    {
        return;
    }
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

bool CGlobalBitmap::ReadBitmapPixels(const BITMAP_t& bitmap, std::vector<std::uint8_t>& rgba)
{
    namespace Ozt = Render::Textures::Ozt;

    const std::wstring filename = bitmap.FileName;
    std::wstring ext;
    SplitExt(filename, ext, false);
    const bool jpeg = 0 == _wcsicmp(ext.c_str(), L"jpg");
    if (!jpeg && 0 != _wcsicmp(ext.c_str(), L"tga"))
    {
        return false;
    }
    std::wstring packed;
    ExchangeExt(filename, jpeg ? L"OZJ" : L"OZT", packed);
    // The file the texture was loaded from, so the mip levels match level 0.
    const std::optional<TextureFileBytes> read =
        bitmap.fromTexturePack ? ReadPackCopy(m_texturePack, packed) : ReadOriginal(packed);
    if (!read)
    {
        return false;
    }
    const std::vector<unsigned char>& file = read->bytes;

    // The texture as loaded: an original image, padded to this size.
    const auto width = static_cast<int>(bitmap.Width);
    const auto height = static_cast<int>(bitmap.Height);
    rgba.assign(static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * 4u, 0u);
    if (!jpeg)
    {
        const Ozt::Header header = Ozt::ReadHeader(file, width, height);
        if (header.status != Ozt::Status::Valid)
        {
            return false;
        }
        Ozt::CopyPixels(file, header, rgba, width);
        return true;
    }

    TurboJpegHandle tjHandle;
    int jpegWidth = 0, jpegHeight = 0, jpegSubsamp = 0, jpegColorspace = 0;
    if (file.size() <= 24 || !tjHandle.valid())
    {
        return false;
    }
    const unsigned char* jpegData = file.data() + 24;
    const auto jpegSize = static_cast<unsigned long>(file.size() - 24);
    if (tjDecompressHeader3(tjHandle.get(), jpegData, jpegSize, &jpegWidth, &jpegHeight, &jpegSubsamp,
                            &jpegColorspace) != 0 ||
        jpegWidth <= 0 || jpegHeight <= 0 || jpegWidth > width || jpegHeight > height)
    {
        return false;
    }
    std::vector<unsigned char> rgb(static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * 3u, 0u);
    if (tjDecompress2(tjHandle.get(), jpegData, jpegSize, rgb.data(), jpegWidth, width * 3, jpegHeight, TJPF_RGB,
                      TJFLAG_FASTDCT) != 0)
    {
        return false;
    }
    rgba = PadRGBToRGBA(rgb.data(), width, height);
    return true;
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
    if (!device || !bitmap.worldTexture || bitmap.mipLevels > 1u || width <= 0 || height <= 0 ||
        (bitmap.Components != 3 && bitmap.Components != 4))
    {
        return false;
    }

    // The pixels were released after the upload (a canvas keeps its own).
    const bool hasAlpha = bitmap.Components == 4;
    std::vector<std::uint8_t> rgba;
    if (bitmap.Buffer)
    {
        rgba = hasAlpha ? std::vector<std::uint8_t>(bitmap.Buffer, bitmap.Buffer + bitmap.BufferStorage.size())
                        : PadRGBToRGBA(bitmap.Buffer, width, height);
    }
    else if (!ReadBitmapPixels(bitmap, rgba))
    {
        return false;
    }
    const MipLevels mips = PlanMipLevels(true, rgba.data(), width, height, hasAlpha, m_mipChainSeconds);

    // Only the texture is replaced: the bitmap keeps its Classic sampler, so the
    // filter passed here does not matter.
    const auto result = mu::UploadBitmapPixels(
        rgba, width, height, SDL_GPU_FILTER_LINEAR,
        bitmap.wrapRepeat ? SDL_GPU_SAMPLERADDRESSMODE_REPEAT : SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE, mips.chain,
        mips.generate);
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
    bitmap.mipLevels = static_cast<std::uint8_t>(
        mips.generate ? Render::Textures::CountMipLevels(static_cast<std::uint32_t>(width),
                                                         static_cast<std::uint32_t>(height))
                      : mips.chain.size() + 1u);
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
