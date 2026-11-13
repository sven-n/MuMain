// Bitmap decoding, upload, registration and unload share the production bitmap state.
#include "stdafx.h"
#include "turbojpeg.h"
#include "Render/Sprites/GlobalBitmap.h"
#include "Core/Content/ContentStore.h"
#include "Core/Utilities/Log/MuLogger.h"
#include "Render/Renderer/BitmapUpload.h"
#include "Render/Renderer/MuRenderer.h"
#include "Render/Sprites/BitmapTextureLimits.h"
#include "Render/Sprites/GpuUploadSkip.h"
#include "Render/Sprites/TextureDecode.h"

#include "Render/RmlUi/RmlPremultiply.h"
#include "Render/Textures/AlphaTestHoles.h"
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
#include <cstdarg>
#include <cwchar>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <future>
#include <memory>
#include <optional>
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

std::optional<TextureFileBytes> ReadTextureFile(const std::filesystem::path& source, std::wstring path,
                                                bool fromTexturePack)
{
    const std::optional<std::string> bytes = Content::ReadFile(source);
    if (!bytes)
    {
        return std::nullopt;
    }
    TextureFileBytes file;
    file.bytes.assign(bytes->begin(), bytes->end());
    file.path = std::move(path);
    file.fromTexturePack = fromTexturePack;
    return file;
}

std::optional<TextureFileBytes> ReadOriginal(const std::wstring& path)
{
    return ReadTextureFile(path, path, false);
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
    return ReadTextureFile(*copy, std::move(copyPath), true);
}

template <typename Read> std::optional<TextureFileBytes> TimedRead(double& seconds, Read&& read)
{
    const auto start = std::chrono::steady_clock::now();
    std::optional<TextureFileBytes> file = read();
    seconds += std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    return file;
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

using Render::Sprites::MipLevels;

// Set by tests only (Render::Sprites::SetGpuUploadsSkipped).
bool s_gpuUploadsSkipped = false;

bool UploadTextureSDLGpu(BITMAP_t* bitmap, std::span<const std::uint8_t> pixels, int width, int height,
                         SDL_GPUFilter filter, SDL_GPUSamplerAddressMode wrapMode, const MipLevels& mips,
                         double& seconds)
{
    if (s_gpuUploadsSkipped)
    {
        bitmap->mipLevels = 1;
        return true;
    }
    const auto start = std::chrono::steady_clock::now();
    auto result = mu::UploadBitmapPixels(pixels, width, height, filter, wrapMode, mips.chain, mips.generate);
    seconds += std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
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
                              bool padToPowerOfTwo, bool premultiplyAlpha, bool keepPixels,
                              LoadFailureReporter reportFailure)
{
    // A queued load of this index finishes first, so the checks below see it.
    FlushPending(uiBitmapIndex);

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

    const bool jpeg = 0 == _wcsicmp(ext.c_str(), L"jpg");
    const auto start = std::chrono::steady_clock::now();
    bool loaded = false;
    bool queued = false;
    if (jpeg || 0 == _wcsicmp(ext.c_str(), L"tga"))
    {
        Render::Sprites::TextureDecodeRequest request;
        request.index = uiBitmapIndex;
        request.filename = filename;
        ExchangeExt(filename, jpeg ? L"OZJ" : L"OZT", request.path);
        request.jpeg = jpeg;
        request.filter = uiFilter;
        request.wrapMode = uiWrapMode;
        request.worldTexture = Render::Textures::IsWorldTexturePath(filename);
        request.padToPowerOfTwo = padToPowerOfTwo;
        request.premultiplyAlpha = premultiplyAlpha;
        request.keepPixels = keepPixels;
        request.filteringEnabled = m_textureFilteringEnabled;
        request.reportFailure = reportFailure;
        // A canvas is drawn on through Buffer as soon as it is loaded. A missing file fails
        // here, at once: callers look for a texture in several folders by what this returns.
        // shortcut: a file that is there but does not decode fails only when it commits, so such a
        // search stops at it; check the header here too if a broken file in an early folder matters.
        queued = m_pending != nullptr && !keepPixels && Render::Sprites::TextureFileExists(m_texturePack, request.path);
        if (queued)
        {
            QueueLoad(std::move(request));
            loaded = true;
        }
        else
        {
            loaded = CommitTexture(request, Render::Sprites::DecodeTextureFile(m_texturePack, request));
        }
    }
    m_textureLoadSeconds += std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    if (!loaded && reportFailure != nullptr)
    {
        reportFailure(filename.c_str());
    }
#ifdef _EDITOR
    if (loaded)
        m_loadWorlds[uiBitmapIndex] = Core::AssetLoadWorld::Get();
#endif
    return loaded;
}
void CGlobalBitmap::UnloadImage(GLuint uiBitmapIndex, bool bForce)
{
    FlushPending(uiBitmapIndex);
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
    FlushAllPending();
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

namespace
{
using Render::Sprites::DecodedTexture;
using Render::Sprites::TextureDecodeRequest;
using Render::Sprites::TextureDecodeResult;

std::wstring FormatError(const wchar_t* format, ...)
{
    wchar_t buffer[1024] = {};
    va_list args;
    va_start(args, format);
    vswprintf(buffer, std::size(buffer), format, args);
    va_end(args);
    return buffer;
}

std::wstring TurboError(const wchar_t* context, const std::wstring& path, tjhandle handle)
{
    const char* message = tjGetErrorStr2(handle);
    if (message == nullptr)
    {
        message = "Unknown TurboJPEG error";
    }
    return FormatError(L"[TurboJPEG] %ls %ls: %hs", context, path.c_str(), message);
}

std::optional<DecodedTexture> DecodeJpeg(const TextureDecodeRequest& request, const TextureFileBytes& file,
                                         TextureDecodeResult& result)
{
    const std::vector<unsigned char>& jpegBuf = file.bytes;
    if (jpegBuf.size() <= 24)
    {
        result.errors.push_back(
            FormatError(L"OpenJpegTurbo: file too small %ls (%zu bytes)\r\n", file.path.c_str(), jpegBuf.size()));
        return std::nullopt;
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
        result.errors.push_back(TurboError(L"tjInitDecompress", file.path, nullptr));
        return std::nullopt;
    }

    auto headerResult =
        tjDecompressHeader3(tjHandle.get(), jpegData, jpegSize, &jpegWidth, &jpegHeight, &jpegSubsamp, &jpegColorspace);
    if (headerResult != 0 || jpegWidth <= 0 || jpegHeight <= 0)
    {
        result.errors.push_back(TurboError(L"tjDecompressHeader3", file.path, tjHandle.get()));
        return std::nullopt;
    }
    if (jpegWidth > maxWidth || jpegHeight > maxHeight)
    {
        result.errors.push_back(FormatError(L"OpenJpegTurbo: image too large %ls (%dx%d, at most %dx%d)\r\n",
                                            file.path.c_str(), jpegWidth, jpegHeight, maxWidth, maxHeight));
        return std::nullopt;
    }

    std::vector<unsigned char> decompressedBuffer(static_cast<std::size_t>(jpegWidth) *
                                                  static_cast<std::size_t>(jpegHeight) * 3u);
    auto decompressResult = tjDecompress2(tjHandle.get(), jpegData, jpegSize, decompressedBuffer.data(), jpegWidth, 0,
                                          jpegHeight, TJPF_RGB, TJFLAG_FASTDCT);
    if (decompressResult != 0)
    {
        result.errors.push_back(TurboError(L"tjDecompress2", file.path, tjHandle.get()));
        return std::nullopt;
    }

    const int textureWidth = request.padToPowerOfTwo ? NextPowerOfTwo(jpegWidth, maxWidth) : jpegWidth;
    const int textureHeight = request.padToPowerOfTwo ? NextPowerOfTwo(jpegHeight, maxHeight) : jpegHeight;
    const int scale = Render::Textures::Upscale::ReadScale(jpegBuf, textureWidth, textureHeight);
    const bool upscaled = scale > Render::Textures::Upscale::NoScale && !request.keepPixels;

    auto pNewBitmap = CreateBitmapStorage(request.index, request.filename, textureWidth, textureHeight, 3);
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

    DecodedTexture decoded;
    decoded.uploadWidth = textureWidth;
    decoded.uploadHeight = textureHeight;
    if (request.keepPixels && scale > Render::Textures::Upscale::NoScale)
    {
        ShrinkCanvas(*pNewBitmap, decoded.uploadWidth, decoded.uploadHeight, scale);
    }

    pNewBitmap->TextureNumber = request.index;
    pNewBitmap->worldTexture = request.worldTexture;
    pNewBitmap->wrapRepeat = request.wrapMode == GL_REPEAT;
    pNewBitmap->scale = static_cast<std::uint8_t>(upscaled ? scale : 1);
    pNewBitmap->fromTexturePack = file.fromTexturePack;
    decoded.rgba = PadRGBToRGBA(pNewBitmap->Buffer, decoded.uploadWidth, decoded.uploadHeight);
    decoded.mips = PlanMipLevels(upscaled || (request.worldTexture && request.filteringEnabled), decoded.rgba.data(),
                                 decoded.uploadWidth, decoded.uploadHeight, false, result.mipSeconds);
    decoded.bitmap = std::move(pNewBitmap);
    return decoded;
}

std::optional<DecodedTexture> DecodeTga(const TextureDecodeRequest& request, const TextureFileBytes& file,
                                        TextureDecodeResult& result)
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
        result.errors.push_back(
            FormatError(L"OpenTga: file too small %ls (%zu bytes)\r\n", file.path.c_str(), pakBuffer.size()));
        return std::nullopt;
    }
    if (header.status == Ozt::Status::InvalidFormat)
    {
        result.errors.push_back(FormatError(L"OpenTga: invalid format %ls (bit=%d, %dx%d, at most %dx%d)\r\n",
                                            file.path.c_str(), header.bitsPerPixel, header.width, header.height,
                                            maxWidth, maxHeight));
        return std::nullopt;
    }

    const int Width = request.padToPowerOfTwo ? NextPowerOfTwo(header.width, maxWidth) : header.width;
    const int Height = request.padToPowerOfTwo ? NextPowerOfTwo(header.height, maxHeight) : header.height;
    const int scale = Render::Textures::Upscale::ReadScale(pakBuffer, Width, Height);
    const bool upscaled = scale > Render::Textures::Upscale::NoScale && !request.keepPixels;

    auto pNewBitmap = CreateBitmapStorage(request.index, request.filename, Width, Height, 4);
    pNewBitmap->Width = static_cast<float>(Width / scale);
    pNewBitmap->Height = static_cast<float>(Height / scale);
    const std::size_t BufferSize = pNewBitmap->BufferStorage.size();

    Ozt::CopyPixels(pakBuffer, header, std::span<std::uint8_t>(pNewBitmap->Buffer, BufferSize), Width);
    pNewBitmap->hasHoles = Render::Textures::HasAlphaTestHoles(
        std::span<const std::uint8_t>(pNewBitmap->Buffer, BufferSize), header.width, header.height, Width);
    if (request.premultiplyAlpha)
    {
        Render::RmlUi::PremultiplyAlpha(pNewBitmap->Buffer, BufferSize / Ozt::BytesPerPixel);
    }

    DecodedTexture decoded;
    decoded.uploadWidth = Width;
    decoded.uploadHeight = Height;
    if (request.keepPixels && scale > Render::Textures::Upscale::NoScale)
    {
        ShrinkCanvas(*pNewBitmap, decoded.uploadWidth, decoded.uploadHeight, scale);
    }

    pNewBitmap->TextureNumber = request.index;
    pNewBitmap->worldTexture = request.worldTexture;
    pNewBitmap->wrapRepeat = request.wrapMode == GL_REPEAT;
    pNewBitmap->scale = static_cast<std::uint8_t>(upscaled ? scale : 1);
    pNewBitmap->fromTexturePack = file.fromTexturePack;
    decoded.mips = PlanMipLevels(upscaled || (request.worldTexture && request.filteringEnabled), pNewBitmap->Buffer,
                                 decoded.uploadWidth, decoded.uploadHeight, true, result.mipSeconds);
    decoded.bitmap = std::move(pNewBitmap);
    return decoded;
}
} // namespace

TextureDecodeResult Render::Sprites::DecodeTextureFile(const Render::Textures::TexturePackFiles& pack,
                                                       const TextureDecodeRequest& request)
{
    const char* loader = request.jpeg ? "OpenJpegTurbo" : "OpenTga";
    TextureDecodeResult result;
    const auto decode = [&](const TextureFileBytes& file)
    { return request.jpeg ? DecodeJpeg(request, file, result) : DecodeTga(request, file, result); };

    if (const std::optional<TextureFileBytes> packCopy =
            TimedRead(result.readSeconds, [&] { return ReadPackCopy(pack, request.path); }))
    {
        result.texture = decode(*packCopy);
        if (result.texture)
        {
            mu::log::Get("render")->debug("{} -- read from the texture pack: {}", loader,
                                          mu_wchar_to_utf8(packCopy->path.c_str()));
            return result;
        }
        mu::log::Get("render")->warn("{} -- the texture pack's copy did not load, using the original: {}", loader,
                                     mu_wchar_to_utf8(packCopy->path.c_str()));
    }

    const std::optional<TextureFileBytes> original =
        TimedRead(result.readSeconds, [&] { return ReadOriginal(request.path); });
    if (!original)
    {
        mu::log::Get("render")->debug("{} -- file open failed: {}", loader, mu_wchar_to_utf8(request.path.c_str()));
        return result;
    }
    result.texture = decode(*original);
    return result;
}

void CGlobalBitmap::BeginLoadBatch(unsigned decodeThreads)
{
    if (m_batchDepth++ == 0)
    {
        m_pending = std::make_unique<Render::Sprites::PendingTextureLoads>();
        if (decodeThreads > 0)
        {
            m_pending->threads = decodeThreads;
        }
        mu::BeginBitmapUploadBatch();
    }
}

void CGlobalBitmap::EndLoadBatch()
{
    if (--m_batchDepth > 0)
    {
        return;
    }
    const auto start = std::chrono::steady_clock::now();
    FlushAllPending();
    mu::EndBitmapUploadBatch();
    const std::size_t queued = m_pending->queued;
    const unsigned threads = m_pending->pool ? m_pending->pool->ThreadCount() : 0u;
    m_pending.reset();
    if (queued > 0)
    {
        mu::log::Get("render")->info(
            "Texture load batch: {} files decoded on {} threads; committing the last of them took {:.0f} ms", queued,
            threads, std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - start).count());
    }
}

void CGlobalBitmap::QueueLoad(TextureDecodeRequest&& request)
{
    const GLuint index = request.index;
    auto decode = std::make_shared<std::packaged_task<TextureDecodeResult()>>(
        [request, &pack = m_texturePack, &decodedBytes = m_pending->decodedBytes]
        {
            TextureDecodeResult result = Render::Sprites::DecodeTextureFile(pack, request);
            decodedBytes += Render::Sprites::DecodedBytes(result);
            return result;
        });
    Render::Sprites::PendingTextureLoads::Load load{std::move(request), decode->get_future()};
    if (!m_pending->pool)
    {
        m_pending->pool.emplace(m_pending->threads);
    }
    m_pending->pool->Post([decode] { (*decode)(); });
    m_pending->loads.emplace(index, std::move(load));
    m_pending->order.push_back(index);
    ++m_pending->queued;

    // Decoded pixels wait in memory until they commit. At most one load per worker runs ahead, and while the
    // decoded ones hold more than the budget the oldest commit first: at most the budget plus a texture per worker.
    constexpr std::size_t DecodedBytesBudget = std::size_t{256} * 1024u * 1024u;
    const std::size_t maxPending = m_pending->pool->ThreadCount();
    while (!m_pending->order.empty() &&
           (m_pending->loads.size() > maxPending || m_pending->decodedBytes > DecodedBytesBudget))
    {
        const GLuint oldest = m_pending->order.front();
        m_pending->order.pop_front();
        FlushPending(oldest);
    }
}

void CGlobalBitmap::FlushPending(GLuint uiBitmapIndex)
{
    if (!m_pending)
    {
        return;
    }
    const auto found = m_pending->loads.find(uiBitmapIndex);
    if (found == m_pending->loads.end())
    {
        return;
    }
    Render::Sprites::PendingTextureLoads::Load load = std::move(found->second);
    m_pending->loads.erase(found);

    const auto start = std::chrono::steady_clock::now();
    TextureDecodeResult result = load.result.get();
    m_pending->decodedBytes -= Render::Sprites::DecodedBytes(result);
    const bool loaded = CommitTexture(load.request, std::move(result));
    m_textureLoadSeconds += std::chrono::duration<double>(std::chrono::steady_clock::now() - start).count();
    if (!loaded)
    {
#ifdef _EDITOR
        m_loadWorlds.erase(uiBitmapIndex);
#endif
        if (load.request.reportFailure != nullptr)
        {
            load.request.reportFailure(load.request.filename.c_str());
        }
    }
}

void CGlobalBitmap::FlushPending(const std::wstring& filename)
{
    if (!m_pending)
    {
        return;
    }
    // Collected first: a commit can report a failure whose Quit unloads everything, which drains `order`.
    std::vector<GLuint> matching;
    for (const GLuint index : m_pending->order)
    {
        const auto found = m_pending->loads.find(index);
        if (found != m_pending->loads.end() && 0 == _wcsicmp(found->second.request.filename.c_str(), filename.c_str()))
        {
            matching.push_back(index);
        }
    }
    for (const GLuint index : matching)
    {
        FlushPending(index);
    }
}

void CGlobalBitmap::FlushAllPending()
{
    if (!m_pending)
    {
        return;
    }
    while (!m_pending->order.empty())
    {
        const GLuint index = m_pending->order.front();
        m_pending->order.pop_front();
        FlushPending(index);
    }
}

std::size_t Render::Sprites::DecodedBytes(const TextureDecodeResult& result)
{
    if (!result.texture)
    {
        return 0;
    }
    const DecodedTexture& texture = *result.texture;
    std::size_t bytes = texture.bitmap->BufferStorage.size() + texture.rgba.size();
    for (const Render::Textures::MipLevel& level : texture.mips.chain)
    {
        bytes += level.rgba.size();
    }
    return bytes;
}

bool Render::Sprites::TextureFileExists(const Render::Textures::TexturePackFiles& pack, const std::wstring& path)
{
    return pack.Find(path) != nullptr || Content::Probe(path) == Content::Found::File;
}

bool CGlobalBitmap::CommitTexture(const TextureDecodeRequest& request, TextureDecodeResult&& result)
{
    for (const std::wstring& error : result.errors)
    {
        g_ErrorReport.Write(L"%ls", error.c_str());
    }
    m_fileReadSeconds += result.readSeconds;
    m_mipChainSeconds += result.mipSeconds;
    if (!result.texture)
    {
        return false;
    }
    DecodedTexture& decoded = *result.texture;
    // Before the bitmap moves: the pixels can be its own buffer.
    const std::span<const std::uint8_t> pixels = decoded.UploadPixels();
    return CommitBitmap(std::move(decoded.bitmap), pixels, decoded.uploadWidth, decoded.uploadHeight, decoded.mips,
                        request.filter, request.wrapMode, request.keepPixels);
}

bool CGlobalBitmap::CommitBitmap(std::unique_ptr<BITMAP_t> bitmap, std::span<const std::uint8_t> pixels, int width,
                                 int height, const MipLevels& mips, GLuint uiFilter, GLuint uiWrapMode, bool keepPixels)
{
    if (!UploadTextureSDLGpu(bitmap.get(), pixels, width, height, MapGLFilterToSDL(uiFilter),
                             MapGLWrapToSDL(uiWrapMode), mips, m_uploadSeconds))
    {
        return false;
    }
    bitmap->UploadedBytes = UploadedBytes(width, height);
    if (!keepPixels)
    {
        ReleaseUploadedPixels(*bitmap);
    }

    const GLuint uiBitmapIndex = bitmap->BitmapIndex;
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

// The upload of a model file's image (LoadImagePixels). Its pixels stay on the bitmap: ReadBitmapPixels
// cannot read them back from a file when texture filtering later wants their mip chain.
bool CGlobalBitmap::UploadBitmap(std::unique_ptr<BITMAP_t> bitmap, std::span<const std::uint8_t> rgba, GLuint uiFilter,
                                 GLuint uiWrapMode)
{
    const bool hasAlpha = bitmap->Components == 4;
    const int width = static_cast<int>(bitmap->Width);
    const int height = static_cast<int>(bitmap->Height);
    bitmap->TextureNumber = bitmap->BitmapIndex;
    bitmap->wrapRepeat = uiWrapMode == GL_REPEAT;
    const MipLevels mips = PlanMipLevels(bitmap->worldTexture && m_textureFilteringEnabled, rgba.data(), width, height,
                                         hasAlpha, m_mipChainSeconds);
    return CommitBitmap(std::move(bitmap), rgba, width, height, mips, uiFilter, uiWrapMode, true);
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
        bitmap->hasHoles = Render::Textures::HasAlphaTestHoles(rgba, width, height, width);
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

    // The queued decodes read the pack's files.
    FlushAllPending();
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
    const std::filesystem::path root(folder);
    if (Content::Probe(root) != Content::Found::Folder)
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
    // The queued decodes planned their mip levels for the old setting.
    FlushAllPending();
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
        "{:.1f} MiB on the GPU; texture loading {:.0f} ms, of which file reads {:.0f} ms, mip chains {:.0f} ms, "
        "uploads {:.0f} ms (waiting for upload slots {:.0f} ms)",
        context, m_textureFilteringEnabled ? "Enhanced" : "Classic", m_anisotropy, world, mipmapped, screenSpace,
        static_cast<double>(gpuBytes) / (1024.0 * 1024.0), m_textureLoadSeconds * 1000.0, m_fileReadSeconds * 1000.0,
        m_mipChainSeconds * 1000.0, m_uploadSeconds * 1000.0, mu::BitmapUploadFenceWaitSeconds() * 1000.0);
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
