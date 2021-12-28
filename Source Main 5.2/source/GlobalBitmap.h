// GlobalBitmap.h: interface for the CGlobalBitmap class.
//
//////////////////////////////////////////////////////////////////////
#pragma once

#pragma warning(disable : 4786)

#include <map>
#include <deque>
#include <string>
#include <setjmp.h>
#include "Jpeglib.h"
#include "./Time/Timer.h"

#define MAX_BITMAP_FILE_NAME 256

#pragma pack(push, 1)
typedef struct
{
	GLuint	BitmapIndex;
	char	FileName[MAX_BITMAP_FILE_NAME];
	float	Width;
	float	Height;
	char	Components;
	GLuint	TextureNumber;
	BYTE	Ref;
	BYTE*   Buffer;

private:
	friend class CBitmapCache;
	DWORD	dwCallCount;
} BITMAP_t;
#pragma pack(pop)

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

	typedef struct _QUICK_CACHE
	{
		DWORD		dwBitmapIndexMin;
		DWORD		dwBitmapIndexMax;
		DWORD		dwRange;
		BITMAP_t**	ppBitmap;
	} QUICK_CACHE;
	typedef std::map<GLuint, BITMAP_t*, std::less<GLuint> > type_cache_map;
	
	type_cache_map		m_mapCacheMain;
	type_cache_map		m_mapCachePlayer;
	type_cache_map		m_mapCacheInterface;
	type_cache_map		m_mapCacheEffect;
	
	QUICK_CACHE			m_QuickCache[NUMBER_OF_QUICK_CACHE];
	BITMAP_t*			m_pNullBitmap;

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
	struct my_error_mgr 
	{
		struct jpeg_error_mgr pub;	
		jmp_buf setjmp_buffer;	
	};
	typedef struct my_error_mgr* my_error_ptr;

	enum 
	{
		MAX_WIDTH = 1024,
		MAX_HEIGHT = 1024,	
	};

	typedef std::map<GLuint, BITMAP_t*, std::less<GLuint> >	type_bitmap_map;
	typedef std::list<GLuint> type_index_list;

	type_bitmap_map	m_mapBitmap;
	type_index_list m_listNonamedIndex;

	GLuint	m_uiAlternate, m_uiTextureIndexStream;
	DWORD	m_dwUsedTextureMemory;

	CBitmapCache	m_BitmapCache;
#ifdef DEBUG_BITMAP_CACHE
	CTimer2				m_DebugOutputTimer;
#endif // DEBUG_BITMAP_CACHE

	void Init();
	
public:
	CGlobalBitmap();
	virtual ~CGlobalBitmap();
	
	GLuint LoadImage(const std::string& filename, GLuint uiFilter = GL_NEAREST, GLuint uiWrapMode = GL_CLAMP_TO_EDGE);
	bool LoadImage(GLuint uiBitmapIndex, const std::string& filename, GLuint uiFilter = GL_NEAREST, GLuint uiWrapMode = GL_CLAMP_TO_EDGE);
	void UnloadImage(GLuint uiBitmapIndex, bool bForce = false);
	void UnloadAllImages();

	BITMAP_t* GetTexture(GLuint uiBitmapIndex);
	BITMAP_t* FindTexture(GLuint uiBitmapIndex);
	BITMAP_t* FindTexture(const std::string& filename);
	BITMAP_t* FindTextureByName(const std::string& name);

	DWORD GetUsedTextureMemory() const;
	size_t GetNumberOfTexture() const;

	bool Convert_Format(const unicode::t_string& filename);

	void Manage();
	
	inline BITMAP_t& operator [] (GLuint uiBitmapIndex) { return *GetTexture(uiBitmapIndex); }

protected:
	GLuint GenerateTextureIndex();
	GLuint FindAvailableTextureIndex(GLuint uiSeed);

	bool OpenJpeg(GLuint uiBitmapIndex, const std::string& filename, GLuint uiFilter = GL_NEAREST, GLuint uiWrapMode = GL_CLAMP_TO_EDGE);
	bool OpenTga(GLuint uiBitmapIndex, const std::string& filename, GLuint uiFilter = GL_NEAREST, GLuint uiWrapMode = GL_CLAMP_TO_EDGE);
	void SplitFileName(IN const std::string& filepath, OUT std::string& filename, bool bIncludeExt);
	void SplitExt(IN const std::string& filepath, OUT std::string& ext, bool bIncludeDot);
	void ExchangeExt(IN const std::string& in_filepath, IN const std::string& ext, OUT std::string& out_filepath);

	bool Save_Image(const unicode::t_string& src, const unicode::t_string& dest, int cDumpHeader);

	static void my_error_exit (j_common_ptr cinfo);
};

extern CGlobalBitmap Bitmaps;
