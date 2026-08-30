// GlobalBitmap.h: interface for the CGlobalBitmap class.
//
//////////////////////////////////////////////////////////////////////
#pragma once

#pragma warning(disable : 4786)

#include <array>
#include <deque>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <setjmp.h>
#include <cstdint>
#include "Core/Time/Timer.h"

struct SDL_GPUTexture;
struct SDL_GPUSampler;

#define MAX_BITMAP_FILE_NAME 256

#pragma pack(push, 1)
struct BITMAP_t
{
    GLuint BitmapIndex;
    wchar_t FileName[MAX_BITMAP_FILE_NAME];
    float Width;
    float Height;
    char Components;
    GLuint TextureNumber;
    std::uint8_t Ref;
    bool IsSkin;
    bool IsHair;
    BYTE* Buffer;
    std::vector<std::uint8_t> BufferStorage;

private:
    friend class CBitmapCache;
    std::uint32_t dwCallCount;

public:
#pragma pack(pop)
    SDL_GPUTexture* sdlTexture = nullptr;
    SDL_GPUSampler* sdlSampler = nullptr;
};

class CBitmapCache
{
    enum
    {
        QUICK_CACHE_MAPTILE = 0,
        QUICK_CACHE_MAPGRASS,
        QUICK_CACHE_WATER,
        QUICK_CACHE_CURSOR,
        QUICK_CACHE_FONT,
        QUICK_CACHE_MAINFRAME,
        QUICK_CACHE_SKILLICON,
        QUICK_CACHE_PLAYER,

        NUMBER_OF_QUICK_CACHE,
    };

    struct QUICK_CACHE
    {
        std::uint32_t dwBitmapIndexMin = 0;
        std::uint32_t dwBitmapIndexMax = 0;
        std::vector<BITMAP_t*> bitmaps;
    };
    using type_cache_map = std::map<GLuint, BITMAP_t*>;

    type_cache_map		m_mapCacheMain;
    type_cache_map		m_mapCachePlayer;
    type_cache_map		m_mapCacheInterface;
    type_cache_map		m_mapCacheEffect;

    QUICK_CACHE			m_QuickCache[NUMBER_OF_QUICK_CACHE];
    BITMAP_t* m_pNullBitmap;

    CTimer2				m_ManageTimer;

public:
    CBitmapCache();
    ~CBitmapCache();

    bool Create();
    void Release();

    void Add(GLuint uiBitmapIndex, BITMAP_t* pBitmap);
    void Remove(GLuint uiBitmapIndex);
    void RemoveAll();

    size_t GetCacheSize();

    void Update();

    bool Find(GLuint uiBitmapIndex, BITMAP_t** ppBitmap);
};

class CGlobalBitmap
{
    enum
    {
        MAX_WIDTH = 1024,
        MAX_HEIGHT = 1024,
    };

    using BitmapPtr = std::unique_ptr<BITMAP_t>;
    using type_bitmap_map = std::map<GLuint, BitmapPtr>;
    using type_index_list = std::list<GLuint>;

    type_bitmap_map	m_mapBitmap;
    type_index_list m_listNonamedIndex;

    GLuint m_uiAlternate, m_uiTextureIndexStream;
    std::uint32_t m_dwUsedTextureMemory;

    CBitmapCache	m_BitmapCache;
#ifdef DEBUG_BITMAP_CACHE
    CTimer2				m_DebugOutputTimer;
#endif // DEBUG_BITMAP_CACHE

    void Init();

public:
    CGlobalBitmap();
    virtual ~CGlobalBitmap();

    GLuint LoadImage(const std::wstring& filename, GLuint uiFilter = GL_NEAREST, GLuint uiWrapMode = GL_CLAMP_TO_EDGE);
    bool LoadImage(GLuint uiBitmapIndex, const std::wstring& filename, GLuint uiFilter = GL_NEAREST, GLuint uiWrapMode = GL_CLAMP_TO_EDGE);
    // Like LoadImage(filename, ...), but never shares an existing by-filename match -- always
    // creates its own independent entry, even if the same file is already resident under a
    // numbered slot (e.g. BITMAP_LOG_IN+7). Needed by callers that cache the resolved GPU
    // texture handle indefinitely (RmlUiRenderInterface::LoadTexture): a numbered slot can be
    // force-reassigned to a completely different file at any time (ReleaseLogoSceneData() ->
    // OpenCharacterSceneData() reusing BITMAP_LOG_IN+7 for a different asset, say), which
    // invalidates a shared handle out from under a caller with no way to know it happened. A
    // dedicated entry costs a small amount of duplicate VRAM for the caller's own textures but
    // is immune to any other scene's slot churn.
    GLuint LoadImageExclusive(const std::wstring& filename, GLuint uiFilter = GL_NEAREST, GLuint uiWrapMode = GL_CLAMP_TO_EDGE);
    void UnloadImage(GLuint uiBitmapIndex, bool bForce = false);
    void UnloadAllImages();

    BITMAP_t* GetTexture(GLuint uiBitmapIndex);
    BITMAP_t* FindTexture(GLuint uiBitmapIndex);
    BITMAP_t* FindTexture(const std::wstring& filename);
    BITMAP_t* FindTextureByName(const std::wstring& name);

    std::uint32_t GetUsedTextureMemory() const;
    size_t GetNumberOfTexture() const;

    bool Convert_Format(const std::wstring& filename);

    void Manage();

    inline BITMAP_t& operator [] (GLuint uiBitmapIndex) { return *GetTexture(uiBitmapIndex); }

protected:
    GLuint GenerateTextureIndex();
    GLuint FindAvailableTextureIndex(GLuint uiSeed);

    bool OpenJpegTurbo(GLuint uiBitmapIndex, const std::wstring& filename, GLuint uiFilter = GL_NEAREST, GLuint uiWrapMode = GL_CLAMP_TO_EDGE);
    bool OpenTga(GLuint uiBitmapIndex, const std::wstring& filename, GLuint uiFilter = GL_NEAREST, GLuint uiWrapMode = GL_CLAMP_TO_EDGE);
    void SplitFileName(IN const std::wstring& filepath, OUT std::wstring& filename, bool bIncludeExt);
    void SplitExt(IN const std::wstring& filepath, OUT std::wstring& ext, bool bIncludeDot);
    void ExchangeExt(IN const std::wstring& in_filepath, IN const std::wstring& ext, OUT std::wstring& out_filepath);

    bool Save_Image(const std::wstring& src, const std::wstring& dest, int cDumpHeader);
};

extern CGlobalBitmap Bitmaps;
