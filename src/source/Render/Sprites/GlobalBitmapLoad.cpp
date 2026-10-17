// Bitmap decoding, upload, registration and unload share the production bitmap state.
#include "stdafx.h"
#include "turbojpeg.h"
#include "Render/Sprites/GlobalBitmap.h"
#include "Core/Utilities/Log/MuLogger.h"
#include "Render/Renderer/BitmapUpload.h"
#include "Render/Renderer/MuRenderer.h"
#include "Render/Sprites/BitmapTextureLimits.h"
#include "GlobalBitmapPath.h"

#include "Render/RmlUi/RmlPremultiply.h"
#include "Render/Textures/OztFile.h"
#include "Render/Textures/UpscaleMarker.h"
#ifdef _EDITOR
#include "Core/Utilities/AssetLoadWorld.h"
#endif

#include <SDL3/SDL_gpu.h>
#include <algorithm>
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

bool UploadTextureSDLGpu(BITMAP_t* bitmap, std::span<const std::uint8_t> pixels, int width, int height,
                         SDL_GPUFilter filter, SDL_GPUSamplerAddressMode wrapMode, bool mipmapped)
{
    auto result = mu::UploadBitmapPixels(pixels, width, height, filter, wrapMode, mipmapped);
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
    return true;
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

    bool loaded = false;
    if (0 == _wcsicmp(ext.c_str(), L"jpg"))
        loaded = OpenJpegTurbo(uiBitmapIndex, filename, uiFilter, uiWrapMode, padToPowerOfTwo, keepPixels);
    else if (0 == _wcsicmp(ext.c_str(), L"tga"))
        loaded = OpenTga(uiBitmapIndex, filename, uiFilter, uiWrapMode, padToPowerOfTwo, premultiplyAlpha, keepPixels);
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
                                  GLuint uiWrapMode, bool padToPowerOfTwo, bool keepPixels)
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
        ReportTurboError(L"tjInitDecompress");
        return false;
    }

    auto headerResult =
        tjDecompressHeader3(tjHandle.get(), jpegData, jpegSize, &jpegWidth, &jpegHeight, &jpegSubsamp, &jpegColorspace);
    if (headerResult != 0 || jpegWidth <= 0 || jpegHeight <= 0)
    {
        ReportTurboError(L"tjDecompressHeader3");
        return false;
    }
    if (jpegWidth > maxWidth || jpegHeight > maxHeight)
    {
        g_ErrorReport.Write(L"OpenJpegTurbo: image too large %ls (%dx%d, at most %dx%d)\r\n", filename_ozj.c_str(),
                            jpegWidth, jpegHeight, maxWidth, maxHeight);
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
    std::vector<std::uint8_t> rgbaData = PadRGBToRGBA(pNewBitmap->Buffer, uploadWidth, uploadHeight);
    if (!UploadTextureSDLGpu(pNewBitmap.get(), rgbaData, uploadWidth, uploadHeight, MapGLFilterToSDL(uiFilter),
                             MapGLWrapToSDL(uiWrapMode), upscaled))
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
    const std::uint32_t uploadedBytes = pNewBitmap->UploadedBytes;
    m_mapBitmap.insert(type_bitmap_map::value_type(uiBitmapIndex, std::move(pNewBitmap)));
    mu::RegisterTexture(uiBitmapIndex, rawTexture);
    mu::RegisterSampler(uiBitmapIndex, rawSampler);
    m_dwUsedTextureMemory += uploadedBytes;

    return true;
}

bool CGlobalBitmap::OpenTga(GLuint uiBitmapIndex, const std::wstring& filename, GLuint uiFilter, GLuint uiWrapMode,
                            bool padToPowerOfTwo, bool premultiplyAlpha, bool keepPixels)
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

    // A marked file holds an image up to its scale times the largest original.
    const int markedScale = Render::Textures::Upscale::ReadMarkedScale(pakBuffer);
    const int maxWidth = Render::Sprites::kBitmapMaximumWidth * markedScale;
    const int maxHeight = Render::Sprites::kBitmapMaximumHeight * markedScale;

    // A file cut off in its header or its pixels is too small.
    const Ozt::Header header = Ozt::ReadHeader(pakBuffer, maxWidth, maxHeight);
    if (header.status == Ozt::Status::TooSmall)
    {
        g_ErrorReport.Write(L"OpenTga: file too small %ls (%zu bytes)\r\n", filename_ozt.c_str(), pakBuffer.size());
        return false;
    }
    if (header.status == Ozt::Status::InvalidFormat)
    {
        g_ErrorReport.Write(L"OpenTga: invalid format %ls (bit=%d, %dx%d, at most %dx%d)\r\n", filename_ozt.c_str(),
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
    if (!UploadTextureSDLGpu(pNewBitmap.get(), pNewBitmap->BufferStorage, uploadWidth, uploadHeight,
                             MapGLFilterToSDL(uiFilter), MapGLWrapToSDL(uiWrapMode), upscaled))
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
    const std::uint32_t uploadedBytes = pNewBitmap->UploadedBytes;
    m_mapBitmap.insert(type_bitmap_map::value_type(uiBitmapIndex, std::move(pNewBitmap)));
    mu::RegisterTexture(uiBitmapIndex, rawTexture);
    mu::RegisterSampler(uiBitmapIndex, rawSampler);
    m_dwUsedTextureMemory += uploadedBytes;

    return true;
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
