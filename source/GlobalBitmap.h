// GlobalBitmap.h: interface for the CGlobalBitmap class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GLOBALBITMAP_H__D28A9680_1E6A_49C8_995B_1E90654A074E__INCLUDED_)
#define AFX_GLOBALBITMAP_H__D28A9680_1E6A_49C8_995B_1E90654A074E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef KJH_ADD_VS2008PORTING_ARRANGE_INCLUDE
#include "gl/gl.h"
#include "_types.h"
#else // KJH_ADD_VS2008PORTING_ARRANGE_INCLUDE
#pragma warning(disable : 4786)
#endif // KJH_ADD_VS2008PORTING_ARRANGE_INCLUDE


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

	void Add(GLuint uiBitmapIndex, BITMAP_t* pBitmap);	//. 주의: 중복등록에 대한 예외처리 없음!!
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

#ifdef LDS_OPTIMIZE_FORLOADING

public:
	// Search를 하는데 std::map 순차적 접근으로 stricmp 로 보다 명확한 검색을 한다.
	// m_mapBitmap_Namemap 의 FullFileName을 검색한다.
	BITMAP_t* FindTexture_Linear(const std::string& filename);
	// m_mapBitmap_Namemap_identity 의 FileName을 검색한다.
	BITMAP_t* FindTextureByName_Linear(const std::string& name);

private:
	// string을 모두 소문자로 변환 하여준다.
	void StringToLower( string &str_ )
	{
		unsigned int uiLen = str_.size();
		for( unsigned int ui_ = 0; ui_ < uiLen; ++ui_ )
		{
			str_[ui_] = tolower( str_[ui_] );
		}
	}

	// string(FileName)으로 NameMap 검색시 binary Search로 속도 최적화를 위한 std::map의 type 형을 선언한다.
	typedef std::map<string, BITMAP_t*>	type_bitmap_namemap;
	
	type_bitmap_namemap m_mapBitmap_Namemap;			// FileName namemap
	type_bitmap_namemap m_mapBitmap_Namemap_identity;	// Full FileName namemap
#endif // LDS_OPTIMIZE_FORLOADING
	
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
	
	GLuint LoadImage(const std::string& filename, GLuint uiFilter = GL_NEAREST, GLuint uiWrapMode = GL_CLAMP);
	bool LoadImage(GLuint uiBitmapIndex, const std::string& filename, GLuint uiFilter = GL_NEAREST, GLuint uiWrapMode = GL_CLAMP);
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

	bool OpenJpeg(GLuint uiBitmapIndex, const std::string& filename, GLuint uiFilter = GL_NEAREST, GLuint uiWrapMode = GL_CLAMP);
	bool OpenTga(GLuint uiBitmapIndex, const std::string& filename, GLuint uiFilter = GL_NEAREST, GLuint uiWrapMode = GL_CLAMP);
	void SplitFileName(IN const std::string& filepath, OUT std::string& filename, bool bIncludeExt);
	void SplitExt(IN const std::string& filepath, OUT std::string& ext, bool bIncludeDot);
	void ExchangeExt(IN const std::string& in_filepath, IN const std::string& ext, OUT std::string& out_filepath);

	bool Save_Image(const unicode::t_string& src, const unicode::t_string& dest, int cDumpHeader);

	static void my_error_exit (j_common_ptr cinfo);
};

#endif // !defined(AFX_GLOBALBITMAP_H__D28A9680_1E6A_49C8_995B_1E90654A074E__INCLUDED_)
