// GlobalBitmap.cpp: implementation of the CGlobalBitmap class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Render/Sprites/GlobalBitmap.h"
#include "Core/Platform/PathResolve.h"
#include "GlobalBitmapPath.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iterator>
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

} // namespace

namespace Render::Sprites
{
std::string NarrowBitmapPath(const std::wstring& wide)
{
    const std::string utf8 = mu_wchar_to_utf8(wide.c_str());
#ifdef _WIN32
    return utf8;
#else
    return MuResolvePath(utf8.c_str());
#endif
}
} // namespace Render::Sprites

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
        g_ConsoleDebug->Write(MCD_NORMAL, L"M,P,I,E : (%d, %d, %d, %d)", m_mapCacheMain.size(), m_mapCachePlayer.size(),
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
        // Quick-cache ranges also retain misses. A later LoadImage call replaces
        // the sentinel, while repeated terrain probes avoid a map lookup per tile.
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
        g_ConsoleDebug->Write(MCD_NORMAL, L"CacheSize=%d(NumberOfTexture=%d)", m_BitmapCache.GetCacheSize(),
                              GetNumberOfTexture());
    }
#endif // DEBUG_BITMAP_CACHE
    m_BitmapCache.Update();
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
    const auto srcPath = Render::Sprites::NarrowBitmapPath(src);
    const auto destPath = Render::Sprites::NarrowBitmapPath(dest);

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
