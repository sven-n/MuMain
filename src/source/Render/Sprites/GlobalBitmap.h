#pragma once

#pragma warning(disable : 4786)

#include <array>
#include <deque>
#include <map>
#include <optional>
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
    // The size the game works with. For upscaled art (Render/Textures/UpscaleMarker.h)
    // the uploaded texture is larger.
    float Width;
    float Height;
    char Components;
    GLuint TextureNumber;
    std::uint8_t Ref;
    bool IsSkin;
    bool IsHair;
    // The pixels, released after the upload except for a canvas (LoadCanvasBitmap:
    // BITMAP_GUILD, BITMAP_INTERFACE_MAP), which keeps them at the size above.
    BYTE* Buffer;
    std::vector<std::uint8_t> BufferStorage;

private:
    friend class CBitmapCache;
    std::uint32_t dwCallCount;

public:
#pragma pack(pop)
    SDL_GPUTexture* sdlTexture = nullptr;
    SDL_GPUSampler* sdlSampler = nullptr; // Borrowed from the renderer until device shutdown.
    // The base level, counted as texture memory.
    std::uint32_t UploadedBytes = 0;

    // Enhanced texture filtering: a world texture (see IsWorldTexturePath) can
    // get a mip chain; sdlSampler stays the Classic sampler either way.
    bool worldTexture = false;
    bool wrapRepeat = false;
    std::uint8_t mipLevels = 1;
    // Upscaled art: the uploaded texture is this times Width x Height, always
    // mipmapped and sampled through the shared anisotropic samplers.
    std::uint8_t scale = 1;
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
    BITMAP_t* m_pNullBitmap = nullptr;

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
    using BitmapPtr = std::unique_ptr<BITMAP_t>;
    using type_bitmap_map = std::map<GLuint, BitmapPtr>;
    using type_index_list = std::list<GLuint>;

    type_bitmap_map	m_mapBitmap;
#ifdef _EDITOR
    std::map<GLuint, int> m_loadWorlds;
#endif
    type_index_list m_listNonamedIndex;

    GLuint m_uiAlternate, m_uiTextureIndexStream;
    std::uint32_t m_dwUsedTextureMemory;

    CBitmapCache	m_BitmapCache;

    // Enhanced texture filtering. The requested
    // state is applied between frames by UpdateTextureFiltering.
    bool m_textureFilteringEnabled = false;
    bool m_textureFilteringRequested = false;
    bool m_textureFilteringSwitchPending = false;
    int m_requestedAnisotropy = 8;
    std::uint32_t m_anisotropy = 0;
    bool m_anisotropyClampLogged = false;
    // Shared Enhanced samplers: [0] clamp to edge, [1] repeat.
    std::array<SDL_GPUSampler*, 2> m_filteredSamplers{};
    std::vector<std::uint32_t> m_mipChainRequests;
    double m_textureLoadSeconds = 0.0;
    double m_mipChainSeconds = 0.0;
#ifdef DEBUG_BITMAP_CACHE
    CTimer2				m_DebugOutputTimer;
#endif // DEBUG_BITMAP_CACHE

    void Init();

public:
    CGlobalBitmap();
    virtual ~CGlobalBitmap();

    GLuint LoadImage(const std::wstring& filename, GLuint uiFilter = GL_NEAREST, GLuint uiWrapMode = GL_CLAMP_TO_EDGE);
    // keepPixels: the game draws on the texture through Buffer (a canvas), so Buffer stays
    // at the original size, also for an upscaled file.
    bool LoadImage(GLuint uiBitmapIndex, const std::wstring& filename, GLuint uiFilter = GL_NEAREST,
                   GLuint uiWrapMode = GL_CLAMP_TO_EDGE, bool padToPowerOfTwo = true, bool premultiplyAlpha = false,
                   bool keepPixels = false);
    // Like LoadImage(filename, ...), but never shares an existing by-filename match -- always
    // creates its own independent entry, even if the same file is already resident under a
    // numbered slot (e.g. BITMAP_LOG_IN+7). Needed by callers that cache the resolved GPU
    // texture handle indefinitely (RmlUiRenderInterface::LoadTexture): a numbered slot can be
    // force-reassigned to a completely different file at any time (ReleaseLogoSceneData() ->
    // OpenCharacterSceneData() reusing BITMAP_LOG_IN+7 for a different asset, say), which
    // invalidates a shared handle out from under a caller with no way to know it happened. A
    // dedicated entry costs a small amount of duplicate VRAM for the caller's own textures but
    // is immune to any other scene's slot churn.
    // RmlUi uses unpadded images; native callers retain power-of-two padding by default.
    // RmlUi also blends premultiplied: premultiplyAlpha scales the colours of an image with
    // alpha (TGA/OZT) before the upload.
    GLuint LoadImageExclusive(const std::wstring& filename, GLuint uiFilter = GL_NEAREST, GLuint uiWrapMode = GL_CLAMP_TO_EDGE, bool padToPowerOfTwo = true, bool premultiplyAlpha = false);
    void UnloadImage(GLuint uiBitmapIndex, bool bForce = false);
    void UnloadAllImages();

    BITMAP_t* GetTexture(GLuint uiBitmapIndex);
    BITMAP_t* FindTexture(GLuint uiBitmapIndex);
    BITMAP_t* FindTexture(const std::wstring& filename);
    BITMAP_t* FindTextureByName(const std::wstring& name);
#ifdef _EDITOR
    // The world that was active when the texture of this number was last
    // loaded, also when the file was loaded already (Core::AssetLoadWorld:
    // the loading screen or a map), for the editor; nullopt when none is
    // loaded.
    std::optional<int> GetLoadWorld(GLuint uiBitmapIndex) const;
#endif

    std::uint32_t GetUsedTextureMemory() const;
    size_t GetNumberOfTexture() const;

    bool Convert_Format(const std::wstring& filename);

    void Manage();

    // Enhanced texture filtering: mip chains and anisotropic sampling for world
    // textures. SetTextureFiltering only records the request; the main loop
    // applies it between frames through UpdateTextureFiltering, which also
    // builds the mip chains that world draws asked for since the last frame.
    void SetTextureFiltering(bool enabled, int anisotropy);
    // Keeps the requested filtering state; the shared samplers are rebuilt between frames.
    void SetAnisotropy(int anisotropy)
    {
        SetTextureFiltering(m_textureFilteringRequested, anisotropy);
    }
    int RequestedAnisotropy() const
    {
        return m_requestedAnisotropy;
    }
    void UpdateTextureFiltering();
    bool IsTextureFilteringEnabled() const
    {
        return m_textureFilteringEnabled;
    }
    // Logs how many textures are world (and how many of those have a mip
    // chain) and screen-space, their GPU size, and the time spent loading.
    void LogTextureFilteringSummary(const char* context) const;

    inline BITMAP_t& operator [] (GLuint uiBitmapIndex) { return *GetTexture(uiBitmapIndex); }

    // Drops the quick-cache entry for this index (including a cached "empty" one)
    // so the next lookup re-resolves from the live bitmap map. Needed when a slot
    // that was polled while empty (and cached as empty) is later filled via
    // LoadImage at runtime - e.g. the in-editor texture import. Does not free the
    // bitmap itself.
    inline void RefreshCacheEntry(GLuint uiBitmapIndex) { m_BitmapCache.Remove(uiBitmapIndex); }

protected:
    GLuint GenerateTextureIndex();
    GLuint FindAvailableTextureIndex(GLuint uiSeed);

    bool OpenJpegTurbo(GLuint uiBitmapIndex, const std::wstring& filename, GLuint uiFilter, GLuint uiWrapMode,
                       bool worldTexture, bool padToPowerOfTwo = true, bool keepPixels = false);
    bool OpenTga(GLuint uiBitmapIndex, const std::wstring& filename, GLuint uiFilter, GLuint uiWrapMode,
                 bool worldTexture, bool padToPowerOfTwo = true, bool premultiplyAlpha = false,
                 bool keepPixels = false);
    void RegisterFilteredSampler(const BITMAP_t& bitmap);
    SDL_GPUSampler* GetFilteredSampler(bool wrapRepeat);
    void ReleaseFilteredSamplers();
    void ApplyTextureFilteringSwitch();
    // The RGBA pixels of a loaded original texture, read from its file again.
    bool ReadBitmapPixels(const BITMAP_t& bitmap, std::vector<std::uint8_t>& rgba);
    bool BuildMipChain(GLuint uiBitmapIndex);
    void SplitFileName(IN const std::wstring& filepath, OUT std::wstring& filename, bool bIncludeExt);
    void SplitExt(IN const std::wstring& filepath, OUT std::wstring& ext, bool bIncludeDot);
    void ExchangeExt(IN const std::wstring& in_filepath, IN const std::wstring& ext, OUT std::wstring& out_filepath);

    bool Save_Image(const std::wstring& src, const std::wstring& dest, int cDumpHeader);
};

extern CGlobalBitmap Bitmaps;
