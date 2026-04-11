// GlobalBitmap.cpp: implementation of the CGlobalBitmap class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MuLogger.h"
#include "turbojpeg.h"
#include "GlobalBitmap.h"

// Story 4.4.1 — Texture System Migration: SDL_gpu upload path.
// MuRenderer.h is included for mu::GetRenderer().GetDevice() (IMuRenderer::GetDevice virtual method).
// Only included on the SDL_gpu path to keep the OpenGL path build-clean.
#ifdef MU_ENABLE_SDL3
#include <SDL3/SDL_gpu.h>
#include "RenderFX/MuRenderer.h"
#endif

#include <algorithm>
#include <array>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iterator>
#include <locale>
#include <limits>
#include <memory>
#include <string>
#include <vector>

CBitmapCache::CBitmapCache() = default;
CBitmapCache::~CBitmapCache()
{
    Release();
}

namespace
{
constexpr std::uint32_t RangeFor(std::uint32_t begin, std::uint32_t end)
{
    return (end > begin) ? (end - begin) : 0;
}

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
    mu::log::Get("data")->error("[TurboJPEG] {}: {}", mu_wchar_to_utf8(context), message);
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

} // namespace

// ---------------------------------------------------------------------------
// Story 4.4.1 — Texture System Migration: SDL_gpu texture registry accessors.
// Forward declarations for the free functions implemented in MuRendererSDLGpu.cpp.
// UnregisterTexture is declared unconditionally because UnloadImage calls it on both paths
// (the registry is a no-op on the OpenGL path since no textures are registered there).
// RegisterTexture / RegisterSampler / UnregisterSampler are SDL_gpu-only.
// ---------------------------------------------------------------------------
namespace mu
{
void UnregisterTexture(std::uint32_t id);
#ifdef MU_ENABLE_SDL3
void RegisterTexture(std::uint32_t id, void* pTex);
void RegisterSampler(std::uint32_t id, void* pSampler);
void UnregisterSampler(std::uint32_t id);
// Story 7.3.0: Forward-declare registry clear functions for UnloadAllImages fallback path
// [VS0-QUAL-BUILDCOMPAT-MACOS]
void ClearTextureRegistry();
void ClearSamplerRegistry();
#endif
} // namespace mu

// ---------------------------------------------------------------------------
// Story 4.4.1 — Texture System Migration: GL→SDL mapping helpers and pixel padding.
//
// These functions are defined at file scope (global namespace) so that the test TU
// (test_texturesystemmigration.cpp) can forward-declare and link them directly.
// They are implementation details of the texture upload path; the names are
// sufficiently specific to avoid collision.
//
// GL filter constants:
//   GL_NEAREST        = 0x2600
//   GL_LINEAR         = 0x2601
//
// GL wrap constants (numeric literals used directly — no GL headers needed):
//   clamp-to-edge = 0x812F
//   repeat        = 0x2901
//
// SDL_GPUFilter enum (from SDL3/SDL_gpu.h release-3.2.8):
//   SDL_GPU_FILTER_NEAREST = 0
//   SDL_GPU_FILTER_LINEAR  = 1
//
// SDL_GPUSamplerAddressMode enum:
//   SDL_GPU_SAMPLER_ADDRESS_MODE_REPEAT        = 0
//   SDL_GPU_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT = 1
//   SDL_GPU_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE = 2
// ---------------------------------------------------------------------------

// MapGLFilterToSDL: maps GL_NEAREST/GL_LINEAR to SDL_GPUFilter integer values.
// Declared with int return type so the test TU can forward-declare without SDL3 headers.
// On the SDL_gpu path the return value is cast to SDL_GPUFilter at the call site.
[[nodiscard]] int MapGLFilterToSDL(unsigned int uiFilter)
{
    // GL_NEAREST = 0x2600 → SDL_GPU_FILTER_NEAREST = 0
    // GL_LINEAR  = 0x2601 → SDL_GPU_FILTER_LINEAR  = 1
    switch (uiFilter)
    {
    case 0x2600u: // GL_NEAREST
        return 0; // SDL_GPU_FILTER_NEAREST
    case 0x2601u: // GL_LINEAR
        return 1; // SDL_GPU_FILTER_LINEAR
    default:
        return 0; // default: SDL_GPU_FILTER_NEAREST (safe)
    }
}

// MapGLWrapToSDL: maps 0x812F/0x2901 wrap modes to SDL_GPUSamplerAddressMode integer values.
// Declared with int return type so the test TU can forward-declare without SDL3 headers.
[[nodiscard]] int MapGLWrapToSDL(unsigned int uiWrapMode)
{
    // 0x812F (clamp-to-edge) → SDL_GPU_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE = 2
    // 0x2901 (repeat)        → SDL_GPU_SAMPLER_ADDRESS_MODE_REPEAT        = 0
    switch (uiWrapMode)
    {
    case 0x812Fu: // clamp-to-edge
        return 2; // SDL_GPU_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE
    case 0x2901u: // repeat
        return 0; // SDL_GPU_SAMPLER_ADDRESS_MODE_REPEAT
    default:
        return 2; // default: SDL_GPU_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE (safe)
    }
}

// PadRGBToRGBA: expands a packed RGB pixel buffer to RGBA8 by inserting alpha=255.
// Input: rgbData points to width*height*3 bytes (R,G,B per pixel, row-major).
// Output: vector of width*height*4 bytes (R,G,B,255 per pixel).
// No GPU device required — pure CPU logic.
[[nodiscard]] std::vector<std::uint8_t> PadRGBToRGBA(const std::uint8_t* rgbData, int width, int height)
{
    const std::size_t pixelCount = static_cast<std::size_t>(width) * static_cast<std::size_t>(height);
    std::vector<std::uint8_t> rgba(pixelCount * 4u);
    for (std::size_t i = 0; i < pixelCount; ++i)
    {
        rgba[i * 4u + 0u] = rgbData[i * 3u + 0u]; // R
        rgba[i * 4u + 1u] = rgbData[i * 3u + 1u]; // G
        rgba[i * 4u + 2u] = rgbData[i * 3u + 2u]; // B
        rgba[i * 4u + 3u] = 255u;                 // A = fully opaque
    }
    return rgba;
}

// ---------------------------------------------------------------------------
// Story 4.4.1 — Texture System Migration: UploadTextureSDLGpu
//
// Creates an SDL_GPUTexture + SDL_GPUSampler from decoded pixel data.
// Uses a synchronous copy command buffer (AcquireGPUCommandBuffer → BeginGPUCopyPass
// → SDL_UploadToGPUTexture → EndGPUCopyPass → SubmitGPUCommandBuffer).
// On success: stores handles in pBitmap->sdlTexture and pBitmap->sdlSampler.
// On any failure: logs via mu::log::Get("data") and returns false.
//
// Parameters:
//   pBitmap    — destination BITMAP_t (must be non-null)
//   pixelData  — source pixel buffer (width*height*(3 or 4) bytes)
//   width      — texture width in pixels
//   height     — texture height in pixels
//   format     — SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM (JPEG/TGA both upload as RGBA8)
//   filter     — SDL_GPUFilter value (from MapGLFilterToSDL)
//   wrapMode   — SDL_GPUSamplerAddressMode value (from MapGLWrapToSDL)
//   filename   — asset filename for error log context
// ---------------------------------------------------------------------------
#ifdef MU_ENABLE_SDL3
[[nodiscard]] static bool UploadTextureSDLGpu(BITMAP_t* pBitmap, const std::uint8_t* pixelData, int width, int height,
                                              SDL_GPUTextureFormat format, SDL_GPUFilter filter,
                                              SDL_GPUSamplerAddressMode wrapMode, const std::wstring& filename)
{
    // Obtain device via IMuRenderer::GetDevice() virtual method (Option B per Dev Notes).
    SDL_GPUDevice* device = static_cast<SDL_GPUDevice*>(mu::GetRenderer().GetDevice());
    if (!device)
    {
        mu::log::Get("data")->error("ASSET: texture upload -- SDL_GPUDevice not available for {}",
                                    mu_wchar_to_utf8(filename.c_str()));
        return false;
    }

    // Create the GPU texture.
    SDL_GPUTextureCreateInfo texInfo{};
    texInfo.type = SDL_GPU_TEXTURETYPE_2D;
    texInfo.format = format;
    texInfo.width = static_cast<Uint32>(width);
    texInfo.height = static_cast<Uint32>(height);
    texInfo.layer_count_or_depth = 1;
    texInfo.num_levels = 1;
    texInfo.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;

    SDL_GPUTexture* gpuTex = SDL_CreateGPUTexture(device, &texInfo);
    if (!gpuTex)
    {
        mu::log::Get("data")->error("ASSET: texture upload -- SDL_CreateGPUTexture failed for {}: {}",
                                    mu_wchar_to_utf8(filename.c_str()), SDL_GetError());
        return false;
    }

    // Determine pixel byte size based on format.
    // Currently all callers supply SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM (4 bytes/pixel).
    // If a non-RGBA8 format is ever added (e.g., R8G8B8_UNORM = 3 bytes/pixel), this
    // calculation must be updated — otherwise the transfer buffer will be incorrectly sized.
    assert(format == SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM &&
           "UploadTextureSDLGpu: non-RGBA8 format requires updating pixelBytes calculation");
    const Uint32 pixelBytes = 4u; // SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM: 4 bytes/pixel
    const Uint32 dataSize = static_cast<Uint32>(width) * static_cast<Uint32>(height) * pixelBytes;

    // Create a transfer buffer and upload pixel data via a copy pass.
    SDL_GPUTransferBufferCreateInfo transferInfo{};
    transferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    transferInfo.size = dataSize;

    SDL_GPUTransferBuffer* transferBuf = SDL_CreateGPUTransferBuffer(device, &transferInfo);
    if (!transferBuf)
    {
        mu::log::Get("data")->error("ASSET: texture upload -- SDL_CreateGPUTransferBuffer failed for {}: {}",
                                    mu_wchar_to_utf8(filename.c_str()), SDL_GetError());
        SDL_ReleaseGPUTexture(device, gpuTex);
        return false;
    }

    // Map transfer buffer, copy pixel data, unmap.
    void* pMapped = SDL_MapGPUTransferBuffer(device, transferBuf, false);
    if (!pMapped)
    {
        mu::log::Get("data")->error("ASSET: texture upload -- SDL_MapGPUTransferBuffer failed for {}: {}",
                                    mu_wchar_to_utf8(filename.c_str()), SDL_GetError());
        SDL_ReleaseGPUTransferBuffer(device, transferBuf);
        SDL_ReleaseGPUTexture(device, gpuTex);
        return false;
    }
    std::memcpy(pMapped, pixelData, dataSize);
    SDL_UnmapGPUTransferBuffer(device, transferBuf);

    // Issue copy command: upload from transfer buffer to GPU texture.
    SDL_GPUCommandBuffer* copyCmd = SDL_AcquireGPUCommandBuffer(device);
    if (!copyCmd)
    {
        mu::log::Get("data")->error("ASSET: texture upload -- SDL_AcquireGPUCommandBuffer failed for {}: {}",
                                    mu_wchar_to_utf8(filename.c_str()), SDL_GetError());
        SDL_ReleaseGPUTransferBuffer(device, transferBuf);
        SDL_ReleaseGPUTexture(device, gpuTex);
        return false;
    }

    {
        SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(copyCmd);
        if (!copyPass)
        {
            mu::log::Get("data")->error("ASSET: texture upload -- SDL_BeginGPUCopyPass failed for {}: {}",
                                        mu_wchar_to_utf8(filename.c_str()), SDL_GetError());
            // Must submit even on failure to release the command buffer back to the device.
            SDL_SubmitGPUCommandBuffer(copyCmd);
            SDL_ReleaseGPUTransferBuffer(device, transferBuf);
            SDL_ReleaseGPUTexture(device, gpuTex);
            return false;
        }

        SDL_GPUTextureTransferInfo srcInfo{};
        srcInfo.transfer_buffer = transferBuf;
        srcInfo.offset = 0;
        srcInfo.pixels_per_row = static_cast<Uint32>(width);
        srcInfo.rows_per_layer = static_cast<Uint32>(height);

        SDL_GPUTextureRegion dstRegion{};
        dstRegion.texture = gpuTex;
        dstRegion.mip_level = 0;
        dstRegion.layer = 0;
        dstRegion.x = 0;
        dstRegion.y = 0;
        dstRegion.z = 0;
        dstRegion.w = static_cast<Uint32>(width);
        dstRegion.h = static_cast<Uint32>(height);
        dstRegion.d = 1;

        SDL_UploadToGPUTexture(copyPass, &srcInfo, &dstRegion, false);
        SDL_EndGPUCopyPass(copyPass);
        SDL_SubmitGPUCommandBuffer(copyCmd);
    }

    SDL_ReleaseGPUTransferBuffer(device, transferBuf);

    // Create per-texture sampler with the requested filter/wrap parameters.
    SDL_GPUSamplerCreateInfo samplerInfo{};
    samplerInfo.min_filter = filter;
    samplerInfo.mag_filter = filter;
    samplerInfo.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST;
    samplerInfo.address_mode_u = wrapMode;
    samplerInfo.address_mode_v = wrapMode;
    samplerInfo.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;

    SDL_GPUSampler* gpuSampler = SDL_CreateGPUSampler(device, &samplerInfo);
    if (!gpuSampler)
    {
        mu::log::Get("data")->error("ASSET: texture upload -- SDL_CreateGPUSampler failed for {}: {}",
                                    mu_wchar_to_utf8(filename.c_str()), SDL_GetError());
        SDL_ReleaseGPUTexture(device, gpuTex);
        return false;
    }

    // Store handles in the BITMAP_t.
    pBitmap->sdlTexture = gpuTex;
    pBitmap->sdlSampler = gpuSampler;
    return true;
}
#endif

bool CBitmapCache::Create()
{
    Release();

    auto configureCache = [](QUICK_CACHE& cache, std::uint32_t minIndex, std::uint32_t maxIndex)
    {
        cache.dwBitmapIndexMin = minIndex;
        cache.dwBitmapIndexMax = maxIndex;
        cache.bitmaps.assign(RangeFor(minIndex, maxIndex), nullptr);
    };

    configureCache(m_QuickCache[QUICK_CACHE_MAPTILE], BITMAP_MAPTILE_BEGIN, BITMAP_MAPTILE_END);
    configureCache(m_QuickCache[QUICK_CACHE_MAPGRASS], BITMAP_MAPGRASS_BEGIN, BITMAP_MAPGRASS_END);
    configureCache(m_QuickCache[QUICK_CACHE_WATER], BITMAP_WATER_BEGIN, BITMAP_WATER_END);
    configureCache(m_QuickCache[QUICK_CACHE_CURSOR], BITMAP_CURSOR_BEGIN, BITMAP_CURSOR_END);
    configureCache(m_QuickCache[QUICK_CACHE_FONT], BITMAP_FONT_BEGIN, BITMAP_FONT_END);
    configureCache(m_QuickCache[QUICK_CACHE_MAINFRAME], BITMAP_INTERFACE_NEW_MAINFRAME_BEGIN,
                   BITMAP_INTERFACE_NEW_MAINFRAME_END);
    configureCache(m_QuickCache[QUICK_CACHE_SKILLICON], BITMAP_INTERFACE_NEW_SKILLICON_BEGIN,
                   BITMAP_INTERFACE_NEW_SKILLICON_END);
    configureCache(m_QuickCache[QUICK_CACHE_PLAYER], BITMAP_PLAYER_TEXTURE_BEGIN, BITMAP_PLAYER_TEXTURE_END);

    m_pNullBitmap = new BITMAP_t;
    *m_pNullBitmap = {};

    m_ManageTimer.SetTimer(1500);

    return true;
}
void CBitmapCache::Release()
{
    SAFE_DELETE(m_pNullBitmap);

    RemoveAll();
    for (auto& cache : m_QuickCache)
    {
        cache.bitmaps.clear();
    }
}

void CBitmapCache::Add(GLuint uiBitmapIndex, BITMAP_t* pBitmap)
{
    for (auto& cache : m_QuickCache)
    {
        if (uiBitmapIndex >= cache.dwBitmapIndexMin && uiBitmapIndex < cache.dwBitmapIndexMax)
        {
            const auto index = static_cast<std::size_t>(uiBitmapIndex - cache.dwBitmapIndexMin);
            cache.bitmaps[index] = pBitmap ? pBitmap : m_pNullBitmap;
            return;
        }
    }

    if (pBitmap)
    {
        if (BITMAP_PLAYER_TEXTURE_BEGIN <= uiBitmapIndex && BITMAP_PLAYER_TEXTURE_END >= uiBitmapIndex)
            m_mapCachePlayer.insert(type_cache_map::value_type(uiBitmapIndex, pBitmap));
        else if (BITMAP_INTERFACE_TEXTURE_BEGIN <= uiBitmapIndex && BITMAP_INTERFACE_TEXTURE_END >= uiBitmapIndex)
            m_mapCacheInterface.insert(type_cache_map::value_type(uiBitmapIndex, pBitmap));
        else if (BITMAP_EFFECT_TEXTURE_BEGIN <= uiBitmapIndex && BITMAP_EFFECT_TEXTURE_END >= uiBitmapIndex)
            m_mapCacheEffect.insert(type_cache_map::value_type(uiBitmapIndex, pBitmap));
        else
            m_mapCacheMain.insert(type_cache_map::value_type(uiBitmapIndex, pBitmap));
    }
}
void CBitmapCache::Remove(GLuint uiBitmapIndex)
{
    for (int i = 0; i < NUMBER_OF_QUICK_CACHE; i++)
    {
        if (uiBitmapIndex >= m_QuickCache[i].dwBitmapIndexMin && uiBitmapIndex < m_QuickCache[i].dwBitmapIndexMax)
        {
            const auto dwVI = static_cast<std::size_t>(uiBitmapIndex - m_QuickCache[i].dwBitmapIndexMin);
            m_QuickCache[i].bitmaps[dwVI] = nullptr;
            return;
        }
    }

    if (BITMAP_PLAYER_TEXTURE_BEGIN <= uiBitmapIndex && BITMAP_PLAYER_TEXTURE_END >= uiBitmapIndex)
    {
        auto mi = m_mapCachePlayer.find(uiBitmapIndex);
        if (mi != m_mapCachePlayer.end())
            m_mapCachePlayer.erase(mi);
    }
    else if (BITMAP_INTERFACE_TEXTURE_BEGIN <= uiBitmapIndex && BITMAP_INTERFACE_TEXTURE_END >= uiBitmapIndex)
    {
        auto mi = m_mapCacheInterface.find(uiBitmapIndex);
        if (mi != m_mapCacheInterface.end())
            m_mapCacheInterface.erase(mi);
    }
    else if (BITMAP_EFFECT_TEXTURE_BEGIN <= uiBitmapIndex && BITMAP_EFFECT_TEXTURE_END >= uiBitmapIndex)
    {
        auto mi = m_mapCacheEffect.find(uiBitmapIndex);
        if (mi != m_mapCacheEffect.end())
            m_mapCacheEffect.erase(mi);
    }
    else
    {
        auto mi = m_mapCacheMain.find(uiBitmapIndex);
        if (mi != m_mapCacheMain.end())
            m_mapCacheMain.erase(mi);
    }
}
void CBitmapCache::RemoveAll()
{
    for (int i = 0; i < NUMBER_OF_QUICK_CACHE; i++)
    {
        std::fill(m_QuickCache[i].bitmaps.begin(), m_QuickCache[i].bitmaps.end(), nullptr);
    }
    m_mapCacheMain.clear();
    m_mapCachePlayer.clear();
    m_mapCacheInterface.clear();
    m_mapCacheEffect.clear();
}

size_t CBitmapCache::GetCacheSize()
{
    return m_mapCacheMain.size() + m_mapCachePlayer.size() + m_mapCacheInterface.size() + m_mapCacheEffect.size();
}

void CBitmapCache::Update()
{
    m_ManageTimer.UpdateTime();

    if (m_ManageTimer.IsTime())
    {
        auto mi = m_mapCacheMain.begin();
        for (; mi != m_mapCacheMain.end();)
        {
            BITMAP_t* pBitmap = (*mi).second;
            if (pBitmap->dwCallCount > 0)
            {
                pBitmap->dwCallCount = 0;
                mi++;
            }
            else
            {
                mi = m_mapCacheMain.erase(mi);
            }
        }

        mi = m_mapCachePlayer.begin();
        for (; mi != m_mapCachePlayer.end();)
        {
            BITMAP_t* pBitmap = (*mi).second;

            if (pBitmap->dwCallCount > 0)
            {
                pBitmap->dwCallCount = 0;
                mi++;
            }
            else
            {
                mi = m_mapCachePlayer.erase(mi);
            }
        }

        mi = m_mapCacheInterface.begin();
        for (; mi != m_mapCacheInterface.end();)
        {
            BITMAP_t* pBitmap = (*mi).second;
            if (pBitmap->dwCallCount > 0)
            {
                pBitmap->dwCallCount = 0;
                mi++;
            }
            else
            {
                mi = m_mapCacheInterface.erase(mi);
            }
        }

        mi = m_mapCacheEffect.begin();
        for (; mi != m_mapCacheEffect.end();)
        {
            BITMAP_t* pBitmap = (*mi).second;
            if (pBitmap->dwCallCount > 0)
            {
                pBitmap->dwCallCount = 0;
                mi++;
            }
            else
            {
                mi = m_mapCacheEffect.erase(mi);
            }
        }

#ifdef DEBUG_BITMAP_CACHE
        mu::log::Get("data")->info("M,P,I,E : ({}, {}, {}, {})", m_mapCacheMain.size(), m_mapCachePlayer.size(),
                                   m_mapCacheInterface.size(), m_mapCacheEffect.size());
#endif // DEBUG_BITMAP_CACHE
    }
}

bool CBitmapCache::Find(GLuint uiBitmapIndex, BITMAP_t** ppBitmap)
{
    for (int i = 0; i < NUMBER_OF_QUICK_CACHE; i++)
    {
        if (uiBitmapIndex >= m_QuickCache[i].dwBitmapIndexMin && uiBitmapIndex < m_QuickCache[i].dwBitmapIndexMax)
        {
            const auto dwVI = static_cast<std::size_t>(uiBitmapIndex - m_QuickCache[i].dwBitmapIndexMin);
            BITMAP_t* cached = m_QuickCache[i].bitmaps[dwVI];
            if (cached != nullptr)
            {
                *ppBitmap = (cached == m_pNullBitmap) ? nullptr : cached;
                return true;
            }
            return false;
        }
    }

    if (BITMAP_PLAYER_TEXTURE_BEGIN <= uiBitmapIndex && BITMAP_PLAYER_TEXTURE_END >= uiBitmapIndex)
    {
        auto mi = m_mapCachePlayer.find(uiBitmapIndex);
        if (mi != m_mapCachePlayer.end())
        {
            *ppBitmap = (*mi).second;
            (*ppBitmap)->dwCallCount++;
            return true;
        }
    }
    else if (BITMAP_INTERFACE_TEXTURE_BEGIN <= uiBitmapIndex && BITMAP_INTERFACE_TEXTURE_END >= uiBitmapIndex)
    {
        auto mi = m_mapCacheInterface.find(uiBitmapIndex);
        if (mi != m_mapCacheInterface.end())
        {
            *ppBitmap = (*mi).second;
            (*ppBitmap)->dwCallCount++;
            return true;
        }
    }
    else if (BITMAP_EFFECT_TEXTURE_BEGIN <= uiBitmapIndex && BITMAP_EFFECT_TEXTURE_END >= uiBitmapIndex)
    {
        auto mi = m_mapCacheEffect.find(uiBitmapIndex);
        if (mi != m_mapCacheEffect.end())
        {
            *ppBitmap = (*mi).second;
            (*ppBitmap)->dwCallCount++;
            return true;
        }
    }
    else
    {
        auto mi = m_mapCacheMain.find(uiBitmapIndex);
        if (mi != m_mapCacheMain.end())
        {
            *ppBitmap = (*mi).second;
            (*ppBitmap)->dwCallCount++;
            return true;
        }
    }
    return false;
}

CGlobalBitmap::CGlobalBitmap()
{
    Init();
    m_BitmapCache.Create();

#ifdef DEBUG_BITMAP_CACHE
    m_DebugOutputTimer.SetTimer(5000);
#endif // DEBUG_BITMAP_CACHE
}
CGlobalBitmap::~CGlobalBitmap()
{
    UnloadAllImages();
}
void CGlobalBitmap::Init()
{
    m_uiAlternate = 0;
    m_uiTextureIndexStream = BITMAP_NONAMED_TEXTURES_BEGIN;
    m_dwUsedTextureMemory = 0;
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
bool CGlobalBitmap::LoadImage(GLuint uiBitmapIndex, const std::wstring& filename, GLuint uiFilter, GLuint uiWrapMode)
{
    // Story 7.3.0: Use numeric literals instead of GL symbolic constants — avoids requiring
    // OpenGL headers on the macOS SDL3 build path. [VS0-QUAL-BUILDCOMPAT-MACOS]
    unsigned int UICLAMP = 0x812Fu;  // clamp-to-edge
    unsigned int UIREPEAT = 0x2901u; // repeat

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
                mu::log::Get("data")->warn("File not found {} ({})->{}", mu_wchar_to_utf8(pBitmap->FileName),
                                           uiBitmapIndex, mu_wchar_to_utf8(filename.c_str()));
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
            // Subtask 5.1 — Unregister from TextureRegistry before erasing the bitmap entry.
            mu::UnregisterTexture(uiBitmapIndex);
#ifdef MU_ENABLE_SDL3
            mu::UnregisterSampler(uiBitmapIndex);
#endif

#ifndef MU_ENABLE_SDL3
            // Non-SDL3: no GPU resources to release (stubs removed in story 7-9-6).
#else
            // Subtask 5.3 — Release SDL_gpu resources on the SDL_gpu path.
            {
                SDL_GPUDevice* device = static_cast<SDL_GPUDevice*>(mu::GetRenderer().GetDevice());
                if (device)
                {
                    if (pBitmap->sdlSampler != nullptr)
                    {
                        SDL_ReleaseGPUSampler(device, pBitmap->sdlSampler);
                        pBitmap->sdlSampler = nullptr;
                    }
                    if (pBitmap->sdlTexture != nullptr)
                    {
                        SDL_ReleaseGPUTexture(device, pBitmap->sdlTexture);
                        pBitmap->sdlTexture = nullptr;
                    }
                }
            }
#endif

            const auto memoryUsed = static_cast<std::uint32_t>(pBitmap->Width * pBitmap->Height * pBitmap->Components);
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
#ifdef _DEBUG
    if (!m_mapBitmap.empty())
        mu::log::Get("data")->info("Unload Images");
#endif // _DEBUG

#ifdef MU_ENABLE_SDL3
    // Release SDL_gpu textures and samplers before clearing the bitmap map.
    // m_mapBitmap.clear() would destroy BITMAP_t objects without releasing GPU resources,
    // causing leaks during map transitions and incorrect TextureRegistry entries.
    {
        SDL_GPUDevice* device = static_cast<SDL_GPUDevice*>(mu::GetRenderer().GetDevice());
        if (device)
        {
            for (auto& pair : m_mapBitmap)
            {
                BITMAP_t* pBitmap = pair.second.get();
                mu::UnregisterTexture(pBitmap->BitmapIndex);
                mu::UnregisterSampler(pBitmap->BitmapIndex);
                if (pBitmap->sdlSampler != nullptr)
                {
                    SDL_ReleaseGPUSampler(device, pBitmap->sdlSampler);
                    pBitmap->sdlSampler = nullptr;
                }
                if (pBitmap->sdlTexture != nullptr)
                {
                    SDL_ReleaseGPUTexture(device, pBitmap->sdlTexture);
                    pBitmap->sdlTexture = nullptr;
                }
            }
        }
        else
        {
            // GPU device already shut down (shutdown order: device destroyed before bitmap destructor).
            // Cannot release GPU objects — log the leak and clear registries to prevent
            // dangling-pointer use-after-free on any subsequent LookupTexture/LookupSampler calls.
            mu::log::Get("data")->warn(
                "ASSET: UnloadAllImages -- SDL_GPUDevice unavailable; GPU texture/sampler objects leaked");
            mu::ClearTextureRegistry();
            mu::ClearSamplerRegistry();
        }
    }
#endif

    for ([[maybe_unused]] auto& pair : m_mapBitmap)
    {
#ifdef _DEBUG
        BITMAP_t* pBitmap = pair.second.get();
        if (pBitmap->Ref > 1)
        {
            mu::log::Get("data")->info("Bitmap {}(RefCount= {})", mu_wchar_to_utf8(pBitmap->FileName), pBitmap->Ref);
        }
#endif // _DEBUG
    }

    m_mapBitmap.clear();
    m_listNonamedIndex.clear();
    m_BitmapCache.RemoveAll();

    Init();
}

BITMAP_t* CGlobalBitmap::GetTexture(GLuint uiBitmapIndex)
{
    BITMAP_t* pBitmap = nullptr;
    if (false == m_BitmapCache.Find(uiBitmapIndex, &pBitmap))
    {
        auto mi = m_mapBitmap.find(uiBitmapIndex);
        if (mi != m_mapBitmap.end())
            pBitmap = mi->second.get();
        m_BitmapCache.Add(uiBitmapIndex, pBitmap);
    }
    if (nullptr == pBitmap)
    {
        static BITMAP_t s_Error{};
        wcscpy(s_Error.FileName, L"CGlobalBitmap::GetTexture Error!!!");
        pBitmap = &s_Error;
    }
    return pBitmap;
}
BITMAP_t* CGlobalBitmap::FindTexture(GLuint uiBitmapIndex)
{
    BITMAP_t* pBitmap = nullptr;
    if (false == m_BitmapCache.Find(uiBitmapIndex, &pBitmap))
    {
        auto mi = m_mapBitmap.find(uiBitmapIndex);
        if (mi != m_mapBitmap.end())
            pBitmap = mi->second.get();
        if (pBitmap != nullptr)
            m_BitmapCache.Add(uiBitmapIndex, pBitmap);
    }
    return pBitmap;
}

BITMAP_t* CGlobalBitmap::FindTexture(const std::wstring& filename)
{
    for (auto& pair : m_mapBitmap)
    {
        BITMAP_t* pBitmap = pair.second.get();
        if (0 == wcsicmp(filename.c_str(), pBitmap->FileName))
            return pBitmap;
    }
    return nullptr;
}

BITMAP_t* CGlobalBitmap::FindTextureByName(const std::wstring& name)
{
    for (auto& pair : m_mapBitmap)
    {
        BITMAP_t* pBitmap = pair.second.get();
        std::wstring texname;
        SplitFileName(pBitmap->FileName, texname, true);
        if (0 == wcsicmp(texname.c_str(), name.c_str()))
            return pBitmap;
    }
    return nullptr;
}

std::uint32_t CGlobalBitmap::GetUsedTextureMemory() const
{
    return m_dwUsedTextureMemory;
}
size_t CGlobalBitmap::GetNumberOfTexture() const
{
    return m_mapBitmap.size();
}

void CGlobalBitmap::Manage()
{
#ifdef DEBUG_BITMAP_CACHE
    m_DebugOutputTimer.UpdateTime();
    if (m_DebugOutputTimer.IsTime())
    {
        mu::log::Get("data")->info("CacheSize={}(NumberOfTexture={})", m_BitmapCache.GetCacheSize(),
                                   GetNumberOfTexture());
    }
#endif // DEBUG_BITMAP_CACHE
    m_BitmapCache.Update();
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

    std::ifstream compressedFile(mu_narrow_path(filename_ozj), std::ios::binary);
    if (!compressedFile)
    {
        mu::log::Get("data")->warn("OpenJpegTurbo: file not found {}", mu_wchar_to_utf8(filename_ozj.c_str()));
        return false;
    }

    std::vector<unsigned char> jpegBuf((std::istreambuf_iterator<char>(compressedFile)),
                                       std::istreambuf_iterator<char>());
    compressedFile.close();

    if (jpegBuf.size() <= 24)
    {
        mu::log::Get("data")->warn("OpenJpegTurbo: file too small {} ({} bytes)",
                                   mu_wchar_to_utf8(filename_ozj.c_str()), jpegBuf.size());
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
    if (headerResult != 0 || jpegWidth <= 0 || jpegHeight <= 0 || jpegWidth > MAX_WIDTH || jpegHeight > MAX_HEIGHT)
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

    const int textureWidth = NextPowerOfTwo(jpegWidth, MAX_WIDTH);
    const int textureHeight = NextPowerOfTwo(jpegHeight, MAX_HEIGHT);

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
    m_dwUsedTextureMemory += static_cast<std::uint32_t>(textureBufferSize);

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

#ifndef MU_ENABLE_SDL3
    // Non-SDL3: register bitmap without GPU upload (stubs removed in story 7-9-6).
    m_mapBitmap.insert(type_bitmap_map::value_type(uiBitmapIndex, std::move(pNewBitmap)));
#else
    // SDL_gpu path: pad RGB→RGBA8, upload texture, create sampler.
    // Capture raw sdlTexture pointer BEFORE std::move (move invalidates pNewBitmap).
    std::vector<std::uint8_t> rgbaData = PadRGBToRGBA(pNewBitmap->Buffer, textureWidth, textureHeight);
    if (!UploadTextureSDLGpu(pNewBitmap.get(), rgbaData.data(), textureWidth, textureHeight,
                             SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
                             static_cast<SDL_GPUFilter>(MapGLFilterToSDL(uiFilter)),
                             static_cast<SDL_GPUSamplerAddressMode>(MapGLWrapToSDL(uiWrapMode)), filename))
    {
        mu::log::Get("data")->error("ASSET: OZJ upload FAILED for idx={} {} ({}x{})", uiBitmapIndex,
                                    mu_wchar_to_utf8(filename.c_str()), textureWidth, textureHeight);
        return false;
    }

    // Capture raw texture pointer before the move so RegisterTexture can be called after insert.
    SDL_GPUTexture* rawTex = pNewBitmap->sdlTexture;
    SDL_GPUSampler* rawSampler = pNewBitmap->sdlSampler;

    m_mapBitmap.insert(type_bitmap_map::value_type(uiBitmapIndex, std::move(pNewBitmap)));

    // Register texture (and sampler) in the TextureRegistry so LookupTexture(uiBitmapIndex) resolves.
    mu::RegisterTexture(uiBitmapIndex, rawTex);
    mu::RegisterSampler(uiBitmapIndex, rawSampler);
#endif

    // Subtask 3.3 — Return true on both paths (existing structure preserved).
    return true;
}

bool CGlobalBitmap::OpenTga(GLuint uiBitmapIndex, const std::wstring& filename, GLuint uiFilter, GLuint uiWrapMode)
{
    std::wstring filename_ozt;
    ExchangeExt(filename, L"OZT", filename_ozt);

    std::ifstream input(mu_narrow_path(filename_ozt), std::ios::binary);
    if (!input)
    {
        mu::log::Get("data")->warn("OpenTga: file not found {}", mu_wchar_to_utf8(filename_ozt.c_str()));
        return false;
    }

    std::vector<unsigned char> pakBuffer((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    input.close();

    if (pakBuffer.size() < 21) // minimal TGA header length check for OZT payload (needs bytes 0..20)
    {
        mu::log::Get("data")->warn("OpenTga: file too small {} ({} bytes)", mu_wchar_to_utf8(filename_ozt.c_str()),
                                   pakBuffer.size());
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

    if (bit != 32 || nx <= 0 || ny <= 0 || nx > MAX_WIDTH || ny > MAX_HEIGHT)
    {
        mu::log::Get("data")->error("OpenTga: invalid format {} (bit={}, {}x{})",
                                    mu_wchar_to_utf8(filename_ozt.c_str()), bit, nx, ny);
        return false;
    }

    const int Width = NextPowerOfTwo(nx, MAX_WIDTH);
    const int Height = NextPowerOfTwo(ny, MAX_HEIGHT);

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

    m_dwUsedTextureMemory += static_cast<std::uint32_t>(BufferSize);

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

#ifndef MU_ENABLE_SDL3
    // Non-SDL3: register bitmap without GPU upload (stubs removed in story 7-9-6).
    m_mapBitmap.insert(type_bitmap_map::value_type(uiBitmapIndex, std::move(pNewBitmap)));
#else
    // SDL_gpu path: RGBA data already decoded in pNewBitmap->Buffer (4 components from OZT decode loop).
    // Upload directly as RGBA8 — no padding required.
    if (!UploadTextureSDLGpu(pNewBitmap.get(), pNewBitmap->Buffer, Width, Height, SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM,
                             static_cast<SDL_GPUFilter>(MapGLFilterToSDL(uiFilter)),
                             static_cast<SDL_GPUSamplerAddressMode>(MapGLWrapToSDL(uiWrapMode)), filename))
    {
        mu::log::Get("data")->error("ASSET: OZT upload FAILED for idx={} {} ({}x{})", uiBitmapIndex,
                                    mu_wchar_to_utf8(filename.c_str()), Width, Height);
        return false;
    }

    // Capture raw texture pointer before the move so RegisterTexture can be called after insert.
    SDL_GPUTexture* rawTex = pNewBitmap->sdlTexture;
    SDL_GPUSampler* rawSampler = pNewBitmap->sdlSampler;

    m_mapBitmap.insert(type_bitmap_map::value_type(uiBitmapIndex, std::move(pNewBitmap)));

    // Register texture (and sampler) in the TextureRegistry so LookupTexture(uiBitmapIndex) resolves.
    mu::RegisterTexture(uiBitmapIndex, rawTex);
    mu::RegisterSampler(uiBitmapIndex, rawSampler);
#endif

    return true;
}

void CGlobalBitmap::SplitFileName(IN const std::wstring& filepath, OUT std::wstring& filename, bool bIncludeExt)
{
    wchar_t __fname[_MAX_FNAME] = {
        0,
    };
    wchar_t __ext[_MAX_EXT] = {
        0,
    };
    _wsplitpath(filepath.c_str(), NULL, NULL, __fname, __ext);
    filename = __fname;
    if (bIncludeExt)
        filename += __ext;
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

bool CGlobalBitmap::Convert_Format(const std::wstring& filename)
{
    wchar_t drive[_MAX_DRIVE];
    wchar_t dir[_MAX_DIR];
    wchar_t fname[_MAX_FNAME];
    wchar_t ext[_MAX_EXT];

    ::_wsplitpath(filename.c_str(), drive, dir, fname, ext);

    std::wstring strPath = drive;
    strPath += dir;
    std::wstring strName = fname;

    if (_wcsicmp(ext, L".jpg") == 0)
    {
        auto strSaveName = strPath + strName + L".OZJ";
        return Save_Image(filename, strSaveName.c_str(), 24);
    }
    else if (_wcsicmp(ext, L".tga") == 0)
    {
        auto strSaveName = strPath + strName + L".OZT";
        return Save_Image(filename, strSaveName.c_str(), 4);
    }
    else if (_wcsicmp(ext, L".bmp") == 0)
    {
        auto strSaveName = strPath + strName + L".OZB";
        return Save_Image(filename, strSaveName.c_str(), 4);
    }
    else
    {
    }

    return false;
}

bool CGlobalBitmap::Save_Image(const std::wstring& src, const std::wstring& dest, int cDumpHeader)
{
    const auto srcPath = mu_narrow_path(src);
    const auto destPath = mu_narrow_path(dest);

    std::ifstream input(srcPath, std::ios::binary);
    if (!input)
    {
        return false;
    }

    std::vector<char> buffer((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    input.close();

    if (buffer.empty())
    {
        return false;
    }

    const auto headerBytes = static_cast<std::size_t>(std::max<int>(0, cDumpHeader));
    const auto headerCount = std::min<std::size_t>(headerBytes, buffer.size());

    std::ofstream output(destPath, std::ios::binary);
    if (!output)
    {
        return false;
    }

    output.write(buffer.data(), static_cast<std::streamsize>(headerCount));
    output.write(buffer.data(), static_cast<std::streamsize>(buffer.size()));
    output.flush();

    return output.good();
}
