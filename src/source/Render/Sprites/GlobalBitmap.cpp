// GlobalBitmap.cpp: implementation of the CGlobalBitmap class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "turbojpeg.h"
#include "Render/Sprites/GlobalBitmap.h"
#include "Render/RHI/RHI.h"
#include "Core/Platform/PathResolve.h"

#include <algorithm>
#include <array>
#include <cstdint>
#include <codecvt>
#include <cstring>
#include <fstream>
#include <iterator>
#include <locale>
#include <limits>
#include <memory>
#include <string>
#include <vector>



CBitmapCache::CBitmapCache() = default;
CBitmapCache::~CBitmapCache() { Release(); }

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

        tjhandle get() const { return handle; }
        bool valid() const { return handle != nullptr; }

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

    std::string NarrowPath(const std::wstring& wide)
    {
        std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> conv;
#ifdef _WIN32
        return conv.to_bytes(wide);
#else
        // Asset paths are Windows-spelled (backslashes, mixed case); resolve
        // them against the case-sensitive filesystem.
        return MuResolvePath(conv.to_bytes(wide).c_str());
#endif
    }
}

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
    configureCache(m_QuickCache[QUICK_CACHE_MAINFRAME], BITMAP_INTERFACE_NEW_MAINFRAME_BEGIN, BITMAP_INTERFACE_NEW_MAINFRAME_END);
    configureCache(m_QuickCache[QUICK_CACHE_SKILLICON], BITMAP_INTERFACE_NEW_SKILLICON_BEGIN, BITMAP_INTERFACE_NEW_SKILLICON_END);
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
    return m_mapCacheMain.size() + m_mapCachePlayer.size() +
        m_mapCacheInterface.size() + m_mapCacheEffect.size();
}

void CBitmapCache::Update()
{
    m_ManageTimer.UpdateTime();

    if (m_ManageTimer.IsTime())
    {
        auto mi = m_mapCacheMain.begin();
        for (; mi != m_mapCacheMain.end(); )
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
        for (; mi != m_mapCachePlayer.end(); )
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
        for (; mi != m_mapCacheInterface.end(); )
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
        for (; mi != m_mapCacheEffect.end(); )
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
        g_ConsoleDebug->Write(MCD_NORMAL, L"M,P,I,E : (%d, %d, %d, %d)", m_mapCacheMain.size(),
            m_mapCachePlayer.size(), m_mapCacheInterface.size(), m_mapCacheEffect.size());
#endif // DEBUG_BITMAP_CACHE
    }
}

bool CBitmapCache::Find(GLuint uiBitmapIndex, BITMAP_t** ppBitmap)
{
    for (int i = 0; i < NUMBER_OF_QUICK_CACHE; i++)
    {
        if (uiBitmapIndex >= m_QuickCache[i].dwBitmapIndexMin &&
            uiBitmapIndex < m_QuickCache[i].dwBitmapIndexMax)
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
    unsigned int UICLAMP = GL_CLAMP_TO_EDGE;
    unsigned int UIREPEAT = GL_REPEAT;

    if (uiWrapMode != UICLAMP && uiWrapMode != UIREPEAT)
    {
#ifdef _DEBUG
        static unsigned int	uiCnt2 = 0;
        int			iBuff;	iBuff = 0;

        wchar_t		szDebugOutput[256];

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
                g_ErrorReport.Write(L"File not found %ls (%d)->%ls\r\n", pBitmap->FileName, uiBitmapIndex, filename.c_str());
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
            // DXP-12: RHI::DestroyTexture invalidates BindState's texture cache internally
            // (matches BindState.h's documented contract -- see RHI_GL.cpp's DestroyTexture).
            RHI::DestroyTexture(RHI::TextureHandle{ pBitmap->TextureNumber });

            // DXP-12: BufferStorage.size() (actual allocated bytes), not Width*Height*Components --
            // Components is a blend-mode semantic flag now decoupled from real buffer byte layout
            // for JPEG-sourced bitmaps (stays 3 even though the RHI-uploaded buffer is 4
            // bytes/pixel, see OpenJpegTurbo's comment), so it would under-count the decrement here.
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
#ifdef _DEBUG
    if (!m_mapBitmap.empty())
        g_ErrorReport.Write(L"Unload Images\r\n");
#endif // _DEBUG

    for (auto& pair : m_mapBitmap)
    {
        BITMAP_t* pBitmap = pair.second.get();

#ifdef _DEBUG
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
        static BITMAP_t s_Error;
        memset(&s_Error, 0, sizeof(BITMAP_t));
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
        g_ConsoleDebug->Write(MCD_NORMAL, L"CacheSize=%d(NumberOfTexture=%d)", m_BitmapCache.GetCacheSize(), GetNumberOfTexture());
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

bool CGlobalBitmap::OpenJpegTurbo(GLuint uiBitmapIndex, const std::wstring& filename, GLuint uiFilter, GLuint uiWrapMode)
{
    std::wstring filename_ozj;
    ExchangeExt(filename, L"OZJ", filename_ozj);

    std::ifstream compressedFile(NarrowPath(filename_ozj), std::ios::binary);
    if (!compressedFile)
    {
        return false;
    }

    std::vector<unsigned char> jpegBuf((std::istreambuf_iterator<char>(compressedFile)), std::istreambuf_iterator<char>());
    compressedFile.close();

    if (jpegBuf.size() <= 24)
    {
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

    auto headerResult = tjDecompressHeader3(tjHandle.get(), jpegData, jpegSize, &jpegWidth, &jpegHeight, &jpegSubsamp, &jpegColorspace);
    if (headerResult != 0 || jpegWidth <= 0 || jpegHeight <= 0 || jpegWidth > MAX_WIDTH || jpegHeight > MAX_HEIGHT)
    {
        ReportTurboError(L"tjDecompressHeader3");
        return false;
    }

    std::vector<unsigned char> decompressedBuffer(static_cast<std::size_t>(jpegWidth) * static_cast<std::size_t>(jpegHeight) * 3u);
    auto decompressResult = tjDecompress2(
        tjHandle.get(),
        jpegData,
        jpegSize,
        decompressedBuffer.data(),
        jpegWidth,
        0,
        jpegHeight,
        TJPF_RGB,
        TJFLAG_FASTDCT);
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
    // DXP-12: RHI's texture format contract is RGBA8 always (DXGI has no 24-bit RGB format), so
    // the RGB->RGBA expansion (alpha=255) that used to happen implicitly on the GL side (upload
    // format=GL_RGB into an internalformat=GL_RGBA8 texture) now happens explicitly here, in the
    // loader, so both backends receive identical RGBA8 payloads.
    //
    // Components stays 3, NOT 4, despite the buffer now being 4 bytes/pixel. Found the hard way
    // (2026-08-03 runtime regression -- torch/particle effects went from additive-transparent to
    // solid black squares): Components isn't just a byte count, it's a semantic "does this bitmap
    // have a real per-pixel alpha channel" flag that ~8 render call sites branch on to pick blend
    // mode / vertex color (ZzzEffectParticle.cpp:8939 EnableAlphaBlend vs EnableAlphaTest,
    // ZzzOpenglUtil.cpp:1098, ZzzEffectPointer.cpp:96, ZzzBMD.cpp:1535/1551/1572,
    // SideHair.cpp:55/72, EditObjects.cpp:274). JPEG source data has no alpha channel (every pixel
    // would come out alpha=255 either way, implicit-fill or explicit) -- setting Components=4
    // wrongly told all of those "this texture has real alpha," routing black background pixels
    // through alpha-blend instead of additive. Components==3 for "no real alpha" / ==4 for "real
    // alpha" is the existing tree-wide contract; RHI's buffer format is an internal upload detail
    // that must NOT change it. (Memory accounting below no longer reads Components for this
    // reason -- see UnloadImage's BufferStorage.size()-based fix.)
    pNewBitmap->Components = 3;
    pNewBitmap->Ref = 1;

    const auto textureBufferSize = static_cast<std::size_t>(textureWidth) * static_cast<std::size_t>(textureHeight) * 4u;
    pNewBitmap->BufferStorage.resize(textureBufferSize);
    pNewBitmap->Buffer = pNewBitmap->BufferStorage.data();
    m_dwUsedTextureMemory += static_cast<std::uint32_t>(textureBufferSize);

    // Per-pixel expand (can't memcpy across differing bytes/pixel like the pre-RHI fast path
    // did) -- textureWidth/Height are always >= jpegWidth/Height (NextPowerOfTwo rounds up, and
    // jpegWidth/Height > MAX_WIDTH/HEIGHT was already rejected above), so cols/rows just guard
    // the copy bounds; padding beyond them stays zero from BufferStorage's resize.
    const int rows = std::min<int>(jpegHeight, textureHeight);
    const int cols = std::min<int>(jpegWidth, textureWidth);
    for (int row = 0; row < rows; ++row)
    {
        const unsigned char* srcRow = &decompressedBuffer[static_cast<std::size_t>(row) * jpegWidth * 3u];
        unsigned char* dstRow = &pNewBitmap->Buffer[static_cast<std::size_t>(row) * textureWidth * 4u];
        for (int col = 0; col < cols; ++col)
        {
            dstRow[col * 4 + 0] = srcRow[col * 3 + 0];
            dstRow[col * 4 + 1] = srcRow[col * 3 + 1];
            dstRow[col * 4 + 2] = srcRow[col * 3 + 2];
            dstRow[col * 4 + 3] = 255;
        }
    }

    RHI::TextureDesc desc;
    desc.width = textureWidth;
    desc.height = textureHeight;
    desc.filter = (uiFilter == GL_LINEAR) ? RHI::TexFilter::Linear : RHI::TexFilter::Nearest;
    desc.wrap = (uiWrapMode == GL_REPEAT) ? RHI::TexWrap::Repeat : RHI::TexWrap::Clamp;
    pNewBitmap->TextureNumber = RHI::CreateTexture(desc, pNewBitmap->Buffer).id;

    m_mapBitmap.insert(type_bitmap_map::value_type(uiBitmapIndex, std::move(pNewBitmap)));

    return true;
}

bool CGlobalBitmap::OpenTga(GLuint uiBitmapIndex, const std::wstring& filename, GLuint uiFilter, GLuint uiWrapMode)
{
    std::wstring filename_ozt;
    ExchangeExt(filename, L"OZT", filename_ozt);

    std::ifstream input(NarrowPath(filename_ozt), std::ios::binary);
    if (!input)
    {
        return false;
    }

    std::vector<unsigned char> pakBuffer((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    input.close();

    if (pakBuffer.size() < 18) // minimal TGA header length check for OZT payload
    {
        return false;
    }

    int index = 12;
    index += 4;
    std::int16_t nx, ny;
    std::memcpy(&nx, &pakBuffer[index], sizeof(nx)); index += 2;
    std::memcpy(&ny, &pakBuffer[index], sizeof(ny)); index += 2;
    const char bit = pakBuffer[index]; index += 1;
    index += 1;

    if (bit != 32 || nx <= 0 || ny <= 0 || nx > MAX_WIDTH || ny > MAX_HEIGHT)
    {
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

    RHI::TextureDesc desc;
    desc.width = Width;
    desc.height = Height;
    desc.filter = (uiFilter == GL_LINEAR) ? RHI::TexFilter::Linear : RHI::TexFilter::Nearest;
    desc.wrap = (uiWrapMode == GL_REPEAT) ? RHI::TexWrap::Repeat : RHI::TexWrap::Clamp;
    pNewBitmap->TextureNumber = RHI::CreateTexture(desc, pNewBitmap->Buffer).id;

    m_mapBitmap.insert(type_bitmap_map::value_type(uiBitmapIndex, std::move(pNewBitmap)));

    // DXP-08a: GL_TEXTURE_ENV/GL_TEXTURE_ENV_MODE is FFP-only texture-combiner state Core Profile
    // removes outright (the id=1280 "<target> or <pname> require feature(s) disabled" violation);
    // the shader path (PassthroughShader/IR::) never reads it, and this resets to GL's own default
    // (GL_MODULATE) anyway, so it's a no-op on the shader pipeline regardless of profile. Guarded rather
    // than deleted since this is a rarely-hit texture-load path, not proven dead with the same
    // exhaustive attribution as the hot-path matrix-stack/color-bridge calls. Stays raw GL --
    // texture-env is FFP pipeline state, not texture-object state, so it's out of RHI's scope
    // (RHI.h has no equivalent, deliberately -- see DXP-11 design doc's Q1).
    if (!g_CoreProfile) glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    return true;
}

void CGlobalBitmap::SplitFileName(IN const std::wstring& filepath, OUT std::wstring& filename, bool bIncludeExt)
{
    wchar_t __fname[_MAX_FNAME] = { 0, };
    wchar_t __ext[_MAX_EXT] = { 0, };
    _wsplitpath(filepath.c_str(), NULL, NULL, __fname, __ext);
    filename = __fname;
    if (bIncludeExt)
        filename += __ext;
}
void CGlobalBitmap::SplitExt(IN const std::wstring& filepath, OUT std::wstring& ext, bool bIncludeDot)
{
    wchar_t __ext[_MAX_EXT] = { 0, };
    _wsplitpath(filepath.c_str(), NULL, NULL, NULL, __ext);
    if (bIncludeDot) {
        ext = __ext;
    }
    else {
        if ((__ext[0] == '.') && __ext[1])
            ext = __ext + 1;
    }
}
void CGlobalBitmap::ExchangeExt(IN const std::wstring& in_filepath, IN const std::wstring& ext, OUT std::wstring& out_filepath)
{
    wchar_t __drive[_MAX_DRIVE] = { 0, };
    wchar_t __dir[_MAX_DIR] = { 0, };
    wchar_t __fname[_MAX_FNAME] = { 0, };
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

    std::wstring strPath = drive; strPath += dir;
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
    const auto srcPath = NarrowPath(src);
    const auto destPath = NarrowPath(dest);

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
