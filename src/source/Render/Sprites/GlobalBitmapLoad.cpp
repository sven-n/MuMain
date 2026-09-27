// Bitmap decoding, upload, registration and unload share the production bitmap state.
#include "stdafx.h"
#include "turbojpeg.h"
#include "Render/Sprites/GlobalBitmap.h"
#include "Core/Utilities/Log/MuLogger.h"
#include "Render/Renderer/BitmapUpload.h"
#include "Render/Renderer/MuRenderer.h"
#include "Render/Sprites/BitmapTextureLimits.h"
#include "GlobalBitmapPath.h"

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
                         SDL_GPUFilter filter, SDL_GPUSamplerAddressMode wrapMode)
{
    auto result = mu::UploadBitmapPixels(pixels, width, height, filter, wrapMode);
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
bool CGlobalBitmap::LoadImage(GLuint uiBitmapIndex, const std::wstring& filename, GLuint uiFilter, GLuint uiWrapMode)
{
    unsigned int UICLAMP = GL_CLAMP_TO_EDGE;
    unsigned int UIREPEAT = GL_REPEAT;

    if (uiWrapMode != UICLAMP && uiWrapMode != UIREPEAT)
    {
#ifdef _DEBUG
        static unsigned int uiCnt2 = 0;
        int iBuff;
        iBuff = 0;

        wchar_t szDebugOutput[256];

        iBuff = iBuff + mu_swprintf(iBuff + szDebugOutput, L"%d. Call No CLAMP & No REPEAT. \n", uiCnt2++);
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

    if (0 == _wcsicmp(ext.c_str(), L"jpg"))
        return OpenJpegTurbo(uiBitmapIndex, filename, uiFilter, uiWrapMode);
    else if (0 == _wcsicmp(ext.c_str(), L"tga"))
        return OpenTga(uiBitmapIndex, filename, uiFilter, uiWrapMode);

    return false;
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

            if (uiBitmapIndex >= BITMAP_NONAMED_TEXTURES_BEGIN && uiBitmapIndex <= BITMAP_NONAMED_TEXTURES_END)
            {
                m_listNonamedIndex.remove(uiBitmapIndex);
            }
            m_BitmapCache.Remove(uiBitmapIndex);
        }
    }
}
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
                                  GLuint uiWrapMode)
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

    const int textureWidth = NextPowerOfTwo(jpegWidth, Render::Sprites::kBitmapMaximumWidth);
    const int textureHeight = NextPowerOfTwo(jpegHeight, Render::Sprites::kBitmapMaximumHeight);

    auto pNewBitmap = std::make_unique<BITMAP_t>();

    pNewBitmap->BitmapIndex = uiBitmapIndex;

    wcsncpy(pNewBitmap->FileName, filename.c_str(), MAX_BITMAP_FILE_NAME - 1);
    pNewBitmap->FileName[MAX_BITMAP_FILE_NAME - 1] = L'\0';

    pNewBitmap->Width = static_cast<float>(textureWidth);
    pNewBitmap->Height = static_cast<float>(textureHeight);
    pNewBitmap->Components = 3;
    pNewBitmap->Ref = 1;

    const auto textureBufferSize =
        static_cast<std::size_t>(textureWidth) * static_cast<std::size_t>(textureHeight) * 3u;
    pNewBitmap->BufferStorage.resize(textureBufferSize);
    pNewBitmap->Buffer = pNewBitmap->BufferStorage.data();

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
    std::vector<std::uint8_t> rgbaData = PadRGBToRGBA(pNewBitmap->Buffer, textureWidth, textureHeight);
    if (!UploadTextureSDLGpu(pNewBitmap.get(), rgbaData, textureWidth, textureHeight, MapGLFilterToSDL(uiFilter),
                             MapGLWrapToSDL(uiWrapMode)))
    {
        return false;
    }

    SDL_GPUTexture* rawTexture = pNewBitmap->sdlTexture;
    SDL_GPUSampler* rawSampler = pNewBitmap->sdlSampler;
    m_mapBitmap.insert(type_bitmap_map::value_type(uiBitmapIndex, std::move(pNewBitmap)));
    mu::RegisterTexture(uiBitmapIndex, rawTexture);
    mu::RegisterSampler(uiBitmapIndex, rawSampler);
    m_dwUsedTextureMemory += static_cast<std::uint32_t>(textureBufferSize);

    return true;
}

bool CGlobalBitmap::OpenTga(GLuint uiBitmapIndex, const std::wstring& filename, GLuint uiFilter, GLuint uiWrapMode)
{
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

    // OZT prefixes the standard 18-byte TGA header with four extra bytes.
    constexpr std::size_t oztHeaderSize = 4 + 18;
    if (pakBuffer.size() < oztHeaderSize)
    {
        g_ErrorReport.Write(L"OpenTga: file too small %ls (%zu bytes)\r\n", filename_ozt.c_str(), pakBuffer.size());
        return false;
    }

    int index = 12;
    index += 4;
    std::int16_t nx, ny;
    std::memcpy(&nx, &pakBuffer[index], sizeof(nx));
    index += 2;
    std::memcpy(&ny, &pakBuffer[index], sizeof(ny));
    index += 2;
    const char bit = pakBuffer[index];
    index += 1;
    index += 1;

    if (bit != 32 || nx <= 0 || ny <= 0 || nx > Render::Sprites::kBitmapMaximumWidth ||
        ny > Render::Sprites::kBitmapMaximumHeight)
    {
        g_ErrorReport.Write(L"OpenTga: invalid format %ls (bit=%d, %dx%d)\r\n", filename_ozt.c_str(), bit, nx, ny);
        return false;
    }

    const auto pixelBytes = static_cast<std::size_t>(nx) * static_cast<std::size_t>(ny) * 4u;
    if (pixelBytes > pakBuffer.size() - oztHeaderSize)
    {
        g_ErrorReport.Write(L"OpenTga: truncated pixels %ls (%zu bytes)\r\n", filename_ozt.c_str(), pakBuffer.size());
        return false;
    }

    const int Width = NextPowerOfTwo(nx, Render::Sprites::kBitmapMaximumWidth);
    const int Height = NextPowerOfTwo(ny, Render::Sprites::kBitmapMaximumHeight);

    auto pNewBitmap = std::make_unique<BITMAP_t>();

    pNewBitmap->BitmapIndex = uiBitmapIndex;

    wcsncpy(pNewBitmap->FileName, filename.c_str(), MAX_BITMAP_FILE_NAME - 1);
    pNewBitmap->FileName[MAX_BITMAP_FILE_NAME - 1] = L'\0';

    pNewBitmap->Width = static_cast<float>(Width);
    pNewBitmap->Height = static_cast<float>(Height);
    pNewBitmap->Components = 4;
    pNewBitmap->Ref = 1;

    const std::size_t BufferSize = static_cast<std::size_t>(Width) * static_cast<std::size_t>(Height) * 4u;
    pNewBitmap->BufferStorage.resize(BufferSize);
    pNewBitmap->Buffer = pNewBitmap->BufferStorage.data();

    for (int y = 0; y < ny; y++)
    {
        const unsigned char* src = &pakBuffer[index];
        index += nx * 4;
        unsigned char* dst = &pNewBitmap->Buffer[(ny - 1 - y) * Width * pNewBitmap->Components];

        for (int x = 0; x < nx; x++)
        {
            dst[0] = src[2];
            dst[1] = src[1];
            dst[2] = src[0];
            dst[3] = src[3];
            src += 4;
            dst += pNewBitmap->Components;
        }
    }

    pNewBitmap->TextureNumber = uiBitmapIndex;
    if (!UploadTextureSDLGpu(pNewBitmap.get(), pNewBitmap->BufferStorage, Width, Height, MapGLFilterToSDL(uiFilter),
                             MapGLWrapToSDL(uiWrapMode)))
    {
        return false;
    }

    SDL_GPUTexture* rawTexture = pNewBitmap->sdlTexture;
    SDL_GPUSampler* rawSampler = pNewBitmap->sdlSampler;
    m_mapBitmap.insert(type_bitmap_map::value_type(uiBitmapIndex, std::move(pNewBitmap)));
    mu::RegisterTexture(uiBitmapIndex, rawTexture);
    mu::RegisterSampler(uiBitmapIndex, rawSampler);
    m_dwUsedTextureMemory += static_cast<std::uint32_t>(BufferSize);

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
