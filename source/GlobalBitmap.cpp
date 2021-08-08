// GlobalBitmap.cpp: implementation of the CGlobalBitmap class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "GlobalBitmap.h"
#include "./Utilities/Log/ErrorReport.h"
#include "./Utilities/Log/muConsoleDebug.h"

CBitmapCache::CBitmapCache() 
{
	memset(m_QuickCache, 0, sizeof(QUICK_CACHE)*NUMBER_OF_QUICK_CACHE);
}
CBitmapCache::~CBitmapCache() { Release(); }

bool CBitmapCache::Create()
{
	Release();

	//////////////////////////////////////////////////////////////////////////
	//. QuickCache 생성
	DWORD dwRange = 0;

	//. 맵타일
	dwRange =  BITMAP_MAPTILE_END - BITMAP_MAPTILE_BEGIN;
	m_QuickCache[QUICK_CACHE_MAPTILE].dwBitmapIndexMin = BITMAP_MAPTILE_BEGIN;
	m_QuickCache[QUICK_CACHE_MAPTILE].dwBitmapIndexMax = BITMAP_MAPTILE_END;
	m_QuickCache[QUICK_CACHE_MAPTILE].dwRange = dwRange;
	m_QuickCache[QUICK_CACHE_MAPTILE].ppBitmap = new BITMAP_t* [dwRange];
	memset(m_QuickCache[QUICK_CACHE_MAPTILE].ppBitmap, 0, dwRange*sizeof(BITMAP_t*));

	//. 맵글래스
	dwRange =  BITMAP_MAPGRASS_END - BITMAP_MAPGRASS_BEGIN;
	m_QuickCache[QUICK_CACHE_MAPGRASS].dwBitmapIndexMin = BITMAP_MAPGRASS_BEGIN;
	m_QuickCache[QUICK_CACHE_MAPGRASS].dwBitmapIndexMax = BITMAP_MAPGRASS_END;
	m_QuickCache[QUICK_CACHE_MAPGRASS].dwRange = dwRange;
	m_QuickCache[QUICK_CACHE_MAPGRASS].ppBitmap = new BITMAP_t* [dwRange];
	memset(m_QuickCache[QUICK_CACHE_MAPGRASS].ppBitmap, 0, dwRange*sizeof(BITMAP_t*));

	//. 맵워터
	dwRange =  BITMAP_WATER_END - BITMAP_WATER_BEGIN;
	m_QuickCache[QUICK_CACHE_WATER].dwBitmapIndexMin = BITMAP_WATER_BEGIN;
	m_QuickCache[QUICK_CACHE_WATER].dwBitmapIndexMax = BITMAP_WATER_END;
	m_QuickCache[QUICK_CACHE_WATER].dwRange = dwRange;
	m_QuickCache[QUICK_CACHE_WATER].ppBitmap = new BITMAP_t* [dwRange];
	memset(m_QuickCache[QUICK_CACHE_WATER].ppBitmap, 0, dwRange*sizeof(BITMAP_t*));

	//. 마우스 커서
	dwRange = BITMAP_CURSOR_END - BITMAP_CURSOR_BEGIN;
	m_QuickCache[QUICK_CACHE_CURSOR].dwBitmapIndexMin = BITMAP_CURSOR_BEGIN;
	m_QuickCache[QUICK_CACHE_CURSOR].dwBitmapIndexMax = BITMAP_CURSOR_END;
	m_QuickCache[QUICK_CACHE_CURSOR].dwRange = dwRange;
	m_QuickCache[QUICK_CACHE_CURSOR].ppBitmap = new BITMAP_t* [dwRange];
	memset(m_QuickCache[QUICK_CACHE_CURSOR].ppBitmap, 0, dwRange*sizeof(BITMAP_t*));

	//. 폰트
	dwRange = BITMAP_FONT_END - BITMAP_FONT_BEGIN;
	m_QuickCache[QUICK_CACHE_FONT].dwBitmapIndexMin = BITMAP_FONT_BEGIN;
	m_QuickCache[QUICK_CACHE_FONT].dwBitmapIndexMax = BITMAP_FONT_END;
	m_QuickCache[QUICK_CACHE_FONT].dwRange = dwRange;
	m_QuickCache[QUICK_CACHE_FONT].ppBitmap = new BITMAP_t* [dwRange];
	memset(m_QuickCache[QUICK_CACHE_FONT].ppBitmap, 0, dwRange*sizeof(BITMAP_t*));

	//. 메인프레임
	dwRange = BITMAP_INTERFACE_NEW_MAINFRAME_END - BITMAP_INTERFACE_NEW_MAINFRAME_BEGIN;
	m_QuickCache[QUICK_CACHE_MAINFRAME].dwBitmapIndexMin = BITMAP_INTERFACE_NEW_MAINFRAME_BEGIN;
	m_QuickCache[QUICK_CACHE_MAINFRAME].dwBitmapIndexMax = BITMAP_INTERFACE_NEW_MAINFRAME_END;
	m_QuickCache[QUICK_CACHE_MAINFRAME].dwRange = dwRange;
	m_QuickCache[QUICK_CACHE_MAINFRAME].ppBitmap = new BITMAP_t* [dwRange];
	memset(m_QuickCache[QUICK_CACHE_MAINFRAME].ppBitmap, 0, dwRange*sizeof(BITMAP_t*));

	//. 스킬아이콘
	dwRange = BITMAP_INTERFACE_NEW_SKILLICON_END - BITMAP_INTERFACE_NEW_SKILLICON_BEGIN;
	m_QuickCache[QUICK_CACHE_SKILLICON].dwBitmapIndexMin = BITMAP_INTERFACE_NEW_SKILLICON_BEGIN;
	m_QuickCache[QUICK_CACHE_SKILLICON].dwBitmapIndexMax = BITMAP_INTERFACE_NEW_SKILLICON_END;
	m_QuickCache[QUICK_CACHE_SKILLICON].dwRange = dwRange;
	m_QuickCache[QUICK_CACHE_SKILLICON].ppBitmap = new BITMAP_t* [dwRange];
	memset(m_QuickCache[QUICK_CACHE_SKILLICON].ppBitmap, 0, dwRange*sizeof(BITMAP_t*));
	
	//. 플레이어 텍스트
	dwRange = BITMAP_PLAYER_TEXTURE_END - BITMAP_PLAYER_TEXTURE_BEGIN;
	m_QuickCache[QUICK_CACHE_PLAYER].dwBitmapIndexMin = BITMAP_PLAYER_TEXTURE_BEGIN;
	m_QuickCache[QUICK_CACHE_PLAYER].dwBitmapIndexMax = BITMAP_PLAYER_TEXTURE_END;
	m_QuickCache[QUICK_CACHE_PLAYER].dwRange = dwRange;
	m_QuickCache[QUICK_CACHE_PLAYER].ppBitmap = new BITMAP_t* [dwRange];
	memset(m_QuickCache[QUICK_CACHE_PLAYER].ppBitmap, 0, dwRange*sizeof(BITMAP_t*));

	//////////////////////////////////////////////////////////////////////////

	m_pNullBitmap = new BITMAP_t;
	memset(m_pNullBitmap, 0, sizeof(BITMAP_t));

	m_ManageTimer.SetTimer(1500);

	return true;
}
void CBitmapCache::Release()
{
	SAFE_DELETE(m_pNullBitmap);

	RemoveAll();

	for(int i=0; i<NUMBER_OF_QUICK_CACHE; i++)
	{
		if(m_QuickCache[i].ppBitmap)
			delete [] m_QuickCache[i].ppBitmap;
	}
	memset(m_QuickCache, 0, sizeof(QUICK_CACHE)*NUMBER_OF_QUICK_CACHE);
}

void CBitmapCache::Add(GLuint uiBitmapIndex, BITMAP_t* pBitmap)
{
	//. 가능하다면, 퀵캐쉬에 등록한다.
	for(int i=0; i<NUMBER_OF_QUICK_CACHE; i++)
	{
		if(uiBitmapIndex > m_QuickCache[i].dwBitmapIndexMin && uiBitmapIndex < m_QuickCache[i].dwBitmapIndexMax)
		{
			DWORD dwVI = uiBitmapIndex - m_QuickCache[i].dwBitmapIndexMin;
			if(pBitmap)
				m_QuickCache[i].ppBitmap[dwVI] = pBitmap;
			else
				m_QuickCache[i].ppBitmap[dwVI] = m_pNullBitmap;
			return;		//. 등록 완료
		}
	}
	if(pBitmap)
	{
		if(BITMAP_PLAYER_TEXTURE_BEGIN <= uiBitmapIndex && BITMAP_PLAYER_TEXTURE_END >= uiBitmapIndex)
			m_mapCachePlayer.insert(type_cache_map::value_type(uiBitmapIndex, pBitmap));
		else if(BITMAP_INTERFACE_TEXTURE_BEGIN <= uiBitmapIndex && BITMAP_INTERFACE_TEXTURE_END >= uiBitmapIndex)
			m_mapCacheInterface.insert(type_cache_map::value_type(uiBitmapIndex, pBitmap));
		else if(BITMAP_EFFECT_TEXTURE_BEGIN <= uiBitmapIndex && BITMAP_EFFECT_TEXTURE_END >= uiBitmapIndex)
			m_mapCacheEffect.insert(type_cache_map::value_type(uiBitmapIndex, pBitmap));
		else
			m_mapCacheMain.insert(type_cache_map::value_type(uiBitmapIndex, pBitmap));
	}
}
void CBitmapCache::Remove(GLuint uiBitmapIndex)
{
	//. 퀵캐쉬에 있다면 제거한다.
	for(int i=0; i<NUMBER_OF_QUICK_CACHE; i++)
	{
		if(uiBitmapIndex > m_QuickCache[i].dwBitmapIndexMin && uiBitmapIndex < m_QuickCache[i].dwBitmapIndexMax)
		{
			DWORD dwVI = uiBitmapIndex - m_QuickCache[i].dwBitmapIndexMin;
			m_QuickCache[i].ppBitmap[dwVI] = NULL;
			return;		//. 제거 완료
		}
	}

	//. 없다면 맵캐쉬들을 검색
	if(BITMAP_PLAYER_TEXTURE_BEGIN <= uiBitmapIndex && BITMAP_PLAYER_TEXTURE_END >= uiBitmapIndex)
	{
		type_cache_map::iterator mi = m_mapCachePlayer.find(uiBitmapIndex);
		if(mi != m_mapCachePlayer.end())
			m_mapCachePlayer.erase(mi);
	}
	else if(BITMAP_INTERFACE_TEXTURE_BEGIN <= uiBitmapIndex && BITMAP_INTERFACE_TEXTURE_END >= uiBitmapIndex)
	{
		type_cache_map::iterator mi = m_mapCacheInterface.find(uiBitmapIndex);
		if(mi != m_mapCacheInterface.end())
			m_mapCacheInterface.erase(mi);
	}
	else if(BITMAP_EFFECT_TEXTURE_BEGIN <= uiBitmapIndex && BITMAP_EFFECT_TEXTURE_END >= uiBitmapIndex)
	{
		type_cache_map::iterator mi = m_mapCacheEffect.find(uiBitmapIndex);
		if(mi != m_mapCacheEffect.end())
			m_mapCacheEffect.erase(mi);
	}
	else
	{
		type_cache_map::iterator mi = m_mapCacheMain.find(uiBitmapIndex);
		if(mi != m_mapCacheMain.end())
			m_mapCacheMain.erase(mi);
	}
}
void CBitmapCache::RemoveAll()
{
	for(int i=0; i<NUMBER_OF_QUICK_CACHE; i++)
	{
		memset(m_QuickCache[i].ppBitmap, 0, m_QuickCache[i].dwRange*sizeof(BITMAP_t*));
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

	if(m_ManageTimer.IsTime())	//. 1.5초에 한번
	{
		//. Main Map Cache
		type_cache_map::iterator mi = m_mapCacheMain.begin();
		for(; mi != m_mapCacheMain.end(); )
		{
			BITMAP_t* pBitmap = (*mi).second;
			if(pBitmap->dwCallCount > 0)
			{
				pBitmap->dwCallCount = 0;
				mi++;
			}
			else
			{
				//. 사용하지 않는것들은 캐쉬에서 제거한다.
				mi = m_mapCacheMain.erase(mi);
			}
		}

		//. Player Map Cache
		mi = m_mapCachePlayer.begin();
		for(; mi != m_mapCachePlayer.end(); )
		{
			BITMAP_t* pBitmap = (*mi).second;

			if(pBitmap->dwCallCount > 0)
			{
				pBitmap->dwCallCount = 0;
				mi++;
			}
			else
			{
				//. 사용하지 않는것들은 캐쉬에서 제거한다.
				mi = m_mapCachePlayer.erase(mi);
			}
		}

		//. Interface Map Cache
		mi = m_mapCacheInterface.begin();
		for(; mi != m_mapCacheInterface.end(); )
		{
			BITMAP_t* pBitmap = (*mi).second;
			if(pBitmap->dwCallCount > 0)
			{
				pBitmap->dwCallCount = 0;
				mi++;
			}
			else
			{
				//. 사용하지 않는것들은 캐쉬에서 제거한다.
				mi = m_mapCacheInterface.erase(mi);
			}
		}

		//. Effect Map Cache
		mi = m_mapCacheEffect.begin();
		for(; mi != m_mapCacheEffect.end(); )
		{
			BITMAP_t* pBitmap = (*mi).second;
			if(pBitmap->dwCallCount > 0)
			{
				pBitmap->dwCallCount = 0;
				mi++;
			}
			else
			{
				//. 사용하지 않는것들은 캐쉬에서 제거한다.
				mi = m_mapCacheEffect.erase(mi);
			}
		}

#ifdef DEBUG_BITMAP_CACHE
		g_ConsoleDebug->Write(MCD_NORMAL, "M,P,I,E : (%d, %d, %d, %d)", m_mapCacheMain.size(), 
			m_mapCachePlayer.size(), m_mapCacheInterface.size(), m_mapCacheEffect.size());
#endif // DEBUG_BITMAP_CACHE
	}
}

bool CBitmapCache::Find(GLuint uiBitmapIndex, BITMAP_t** ppBitmap)
{
	//. 퀵캐쉬에 있다면 찾는다.
	for(int i=0; i<NUMBER_OF_QUICK_CACHE; i++)
	{
		if(uiBitmapIndex > m_QuickCache[i].dwBitmapIndexMin && 
			uiBitmapIndex < m_QuickCache[i].dwBitmapIndexMax)
		{
			DWORD dwVI = uiBitmapIndex - m_QuickCache[i].dwBitmapIndexMin;
			if(m_QuickCache[i].ppBitmap[dwVI])
			{
				if(m_QuickCache[i].ppBitmap[dwVI] == m_pNullBitmap)	//. 찾았으나 Null 비트맵이라면
					*ppBitmap = NULL;
				else
				{
					*ppBitmap = m_QuickCache[i].ppBitmap[dwVI];
				}
				return true;
			}
			return false;
		}
	}

	//. 없다면 맵캐쉬들을 검색
	if(BITMAP_PLAYER_TEXTURE_BEGIN <= uiBitmapIndex && BITMAP_PLAYER_TEXTURE_END >= uiBitmapIndex)
	{
		type_cache_map::iterator mi = m_mapCachePlayer.find(uiBitmapIndex);
		if(mi != m_mapCachePlayer.end())
		{
			*ppBitmap = (*mi).second;
			(*ppBitmap)->dwCallCount++;
			return true;
		}
	}
	else if(BITMAP_INTERFACE_TEXTURE_BEGIN <= uiBitmapIndex && BITMAP_INTERFACE_TEXTURE_END >= uiBitmapIndex)
	{
		type_cache_map::iterator mi = m_mapCacheInterface.find(uiBitmapIndex);
		if(mi != m_mapCacheInterface.end())
		{
			*ppBitmap = (*mi).second;
			(*ppBitmap)->dwCallCount++;
			return true;
		}
	}
	else if(BITMAP_EFFECT_TEXTURE_BEGIN <= uiBitmapIndex && BITMAP_EFFECT_TEXTURE_END >= uiBitmapIndex)
	{
		type_cache_map::iterator mi = m_mapCacheEffect.find(uiBitmapIndex);
		if(mi != m_mapCacheEffect.end())
		{
			*ppBitmap = (*mi).second;
			(*ppBitmap)->dwCallCount++;
			return true;
		}
	}
	else
	{
		type_cache_map::iterator mi = m_mapCacheMain.find(uiBitmapIndex);
		if(mi != m_mapCacheMain.end())
		{
			*ppBitmap = (*mi).second;
			(*ppBitmap)->dwCallCount++;
			return true;
		}
	}
	return false;
}


////////////////////////////////////////////////////////////////////////// CGlobalBitmap
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

GLuint CGlobalBitmap::LoadImage(const std::string& filename, GLuint uiFilter, GLuint uiWrapMode)
{
	BITMAP_t* pBitmap = FindTexture(filename);
	if(pBitmap)
	{
		if(pBitmap->Ref > 0)
		{
			if(0 == _stricmp(pBitmap->FileName, filename.c_str()))
			{
				pBitmap->Ref++;

				return pBitmap->BitmapIndex;
			}
		}
	}
	else
	{
		GLuint uiNewTextureIndex = GenerateTextureIndex();
		if(true == LoadImage(uiNewTextureIndex, filename, uiFilter, uiWrapMode))
		{
			m_listNonamedIndex.push_back(uiNewTextureIndex);

			return uiNewTextureIndex;
		}
	}
	return BITMAP_UNKNOWN;
}
bool CGlobalBitmap::LoadImage(GLuint uiBitmapIndex, const std::string& filename, GLuint uiFilter, GLuint uiWrapMode)
{
	unsigned int UICLAMP	= GL_CLAMP_TO_EDGE;
	unsigned int UIREPEAT	= GL_REPEAT;

	if( uiWrapMode != UICLAMP && uiWrapMode != UIREPEAT )
	{
#ifdef _DEBUG
		static unsigned int	uiCnt2 = 0;
		int			iBuff;	iBuff = 0;
		
		char		szDebugOutput[256];
		
		iBuff = iBuff + sprintf( iBuff + szDebugOutput, "%d. Call No CLAMP & No REPEAT. \n", 
			uiCnt2++);
		OutputDebugString( szDebugOutput );
#endif
	}

	type_bitmap_map::iterator mi = m_mapBitmap.find(uiBitmapIndex);
	if(mi != m_mapBitmap.end())
	{
		BITMAP_t* pBitmap = (*mi).second;
		if(pBitmap->Ref > 0)
		{
			if(0 == _stricmp(pBitmap->FileName, filename.c_str()))
			{
				pBitmap->Ref++;
				return true;
			}
			else
			{
				g_ErrorReport.Write("인덱스를 공유하거나 Delete없이 사용하였음%s(0x%00000008X)->%s\r\n", 
					pBitmap->FileName, uiBitmapIndex, filename.c_str());
				UnloadImage(uiBitmapIndex, true);	//. 강제로 지운다.
			}
		}
	}
	
	std::string ext;
	SplitExt(filename, ext, false);
	
	if(0 == _stricmp(ext.c_str(), "jpg"))
		return OpenJpeg(uiBitmapIndex, filename, uiFilter, uiWrapMode);
	else if(0 == _stricmp(ext.c_str(), "tga"))
		return OpenTga(uiBitmapIndex, filename, uiFilter, uiWrapMode);
	
	return false;
}
void CGlobalBitmap::UnloadImage(GLuint uiBitmapIndex, bool bForce)
{
	type_bitmap_map::iterator mi = m_mapBitmap.find(uiBitmapIndex);
	if(mi != m_mapBitmap.end())
	{
		BITMAP_t* pBitmap = (*mi).second;

#ifdef LDS_OPTIMIZE_FORLOADING
		string strKey(pBitmap->FileName), strKey_Identity;
		// Full FileName에서 파일명만 따로 strKey_Identity로 가져온다.
		SplitFileName(strKey, strKey_Identity, true);		
		
		// string들을 비교 시 map::find는 대소문자에 대한 구분을 하기 때문에 전부 소문자로 변환 하여 준다.
		StringToLower( strKey );
		StringToLower( strKey_Identity );

		type_bitmap_namemap::iterator iter_ = m_mapBitmap_Namemap.end();
		
		// FullNameMap에 존재여부 검사 후 존재시 erase 하여준다.
		// erase 인자값으로 map::iterator를 넣어도 되고, firstKey를 인자로 해도 무방하다.
		// namemap 상의 FileName String 삭제
		iter_	= m_mapBitmap_Namemap.find( strKey );
		if( iter_ != m_mapBitmap_Namemap.end() )
		{
			m_mapBitmap_Namemap.erase( strKey );
		}

		//  namemap_identity 상의 FileName String 삭제
		iter_	= m_mapBitmap_Namemap_identity.find( strKey_Identity );
		if( iter_ != m_mapBitmap_Namemap_identity.end() )
		{
			m_mapBitmap_Namemap_identity.erase( strKey_Identity );
		}
		
#endif // LDS_OPTIMIZE_FORLOADING

		if(--pBitmap->Ref == 0 || bForce)
		{
			// OpenGL 텍스쳐 삭제
			glDeleteTextures( 1, &(pBitmap->TextureNumber));

#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
			m_dwUsedTextureMemory -= (DWORD)(pBitmap->Width * pBitmap->Height * pBitmap->Components);
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
			m_dwUsedTextureMemory -= (pBitmap->Width*pBitmap->Height*pBitmap->Components);
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
			delete [] pBitmap->Buffer;
			delete pBitmap;
			m_mapBitmap.erase(mi);
			
			if(uiBitmapIndex >= BITMAP_NONAMED_TEXTURES_BEGIN && uiBitmapIndex <= BITMAP_NONAMED_TEXTURES_END)
			{
				m_listNonamedIndex.remove(uiBitmapIndex);	//. 인덱스 캐쉬에서 제거
			}
			m_BitmapCache.Remove(uiBitmapIndex);	//. 비트맵 캐쉬에서 제거
		}
	}
}
void CGlobalBitmap::UnloadAllImages()
{
#ifdef _DEBUG
	if(!m_mapBitmap.empty())
		g_ErrorReport.Write("해제되지 않은 비트맵 메모리:\r\n");
#endif // _DEBUG
	
	type_bitmap_map::iterator mi = m_mapBitmap.begin();
	for(; mi != m_mapBitmap.end(); mi++)
	{
		BITMAP_t* pBitmap = (*mi).second;

#ifdef _DEBUG
		if(pBitmap->Ref > 1)
		{
			g_ErrorReport.Write("Bitmap %s(RefCount= %d)\r\n", pBitmap->FileName, pBitmap->Ref);
		}
#endif // _DEBUG
		delete [] pBitmap->Buffer;
		delete pBitmap;
	}

	m_mapBitmap.clear();
	m_listNonamedIndex.clear();		//. 인덱스 캐쉬 클리어
	m_BitmapCache.RemoveAll();		//. 비트맵 캐쉬 클리어

#ifdef LDS_OPTIMIZE_FORLOADING
	m_mapBitmap_Namemap.clear();	//. bitmap texturefullfile명 namemap 클리어
	m_mapBitmap_Namemap_identity.clear();  //. bitmap texturefile명 namemap 클리어
#endif // LDS_OPTIMIZE_FORLOADING

	Init();
}

BITMAP_t* CGlobalBitmap::GetTexture(GLuint uiBitmapIndex)
{
	BITMAP_t* pBitmap = NULL;
	if(false == m_BitmapCache.Find(uiBitmapIndex, &pBitmap))	//. 캐쉬에 없다면
	{
		type_bitmap_map::iterator mi = m_mapBitmap.find(uiBitmapIndex);	//. 맵에서 찾는다.
		if(mi != m_mapBitmap.end())
			pBitmap = (*mi).second;
		m_BitmapCache.Add(uiBitmapIndex, pBitmap);	//. 캐쉬에 추가
	}
	if(NULL == pBitmap)	//. 없는 인덱스라면
	{
		static BITMAP_t s_Error;
		memset(&s_Error, 0, sizeof(BITMAP_t));
		strcpy(s_Error.FileName, "CGlobalBitmap::GetTexture Error!!!");
		pBitmap = &s_Error;
	}
	return pBitmap;
}
BITMAP_t* CGlobalBitmap::FindTexture(GLuint uiBitmapIndex)
{
	BITMAP_t* pBitmap = NULL;
	if(false == m_BitmapCache.Find(uiBitmapIndex, &pBitmap))	//. 캐쉬에 없다면
	{
		type_bitmap_map::iterator mi = m_mapBitmap.find(uiBitmapIndex);	//. 맵에서 찾는다.
		if(mi != m_mapBitmap.end())
			pBitmap = (*mi).second;
		if(pBitmap != NULL)
			m_BitmapCache.Add(uiBitmapIndex, pBitmap);	//. 캐쉬에 추가
	}
	return pBitmap;
}

#ifdef LDS_OPTIMIZE_FORLOADING
// firstkey(string(filename))를 기준으로 std::map::find로 binary search 하여 secondkey값인 BITMAP_t* 를 반환 하여준다.
BITMAP_t* CGlobalBitmap::FindTexture(const std::string& filename)
{
	// 정확한 검색을 위해 find의 firstkey 값 string을 모두 소문자로 변환 한다. insert시에 모두 소문자로 변환이 된다.
	string strFileName( filename );
	StringToLower( strFileName );
	
	type_bitmap_namemap::iterator iter_bitmapnamemap = m_mapBitmap_Namemap.find( strFileName );
	if( iter_bitmapnamemap != m_mapBitmap_Namemap.end() )
	{			
		return (*iter_bitmapnamemap).second;
	}
				
	return NULL;
}
#else // LDS_OPTIMIZE_FORLOADING
BITMAP_t* CGlobalBitmap::FindTexture(const std::string& filename)
{
	type_bitmap_map::iterator mi = m_mapBitmap.begin();
	for(; mi != m_mapBitmap.end(); mi++)
	{
		BITMAP_t* pBitmap = (*mi).second;
		if(0 == stricmp(filename.c_str(), pBitmap->FileName)) 
			return pBitmap;
	}
	return NULL;
}
#endif // LDS_OPTIMIZE_FORLOADING


#ifdef LDS_OPTIMIZE_FORLOADING
// firstkey(string(filename))를 기준으로 std::map::find로 binary search 하여 secondkey값인 BITMAP_t* 를 반환 하여준다.
// FullFile명 중 FileName만을 Split 하여 firstkey로 잡은 std::map(Namemap_identity)의 find로 BinarySearch 한다.
BITMAP_t* CGlobalBitmap::FindTextureByName(const std::string& name)
{
	string strFileName( name );
	StringToLower( strFileName );

	type_bitmap_namemap::iterator iter_bitmapnamemap = m_mapBitmap_Namemap_identity.find( strFileName );
	if( iter_bitmapnamemap != m_mapBitmap_Namemap_identity.end() )
	{						
		return (*iter_bitmapnamemap).second;
	}

	return NULL;
}
#else // LDS_OPTIMIZE_FORLOADING
BITMAP_t* CGlobalBitmap::FindTextureByName(const std::string& name)
{
	type_bitmap_map::iterator mi = m_mapBitmap.begin();
	for(; mi != m_mapBitmap.end(); mi++)
	{
		BITMAP_t* pBitmap = (*mi).second;
		std::string texname;
		SplitFileName(pBitmap->FileName, texname, true);
		if(0 == stricmp(texname.c_str(), name.c_str())) 
			return pBitmap;
	}
	return NULL;
}
#endif // LDS_OPTIMIZE_FORLOADING




#ifdef LDS_OPTIMIZE_FORLOADING
// std::map::find로 String을 firstkey로 두어 find 할 경우 기존의 
// LinearSearch로 각 String을 모두 stricmp 하여 각 character를 명확히 대조하는
// 방식이 아닌 단순 대소 비교 이기에 만에 하나 있을 수 있는 비교 fail 이 
// 생기지 않도록 기존의 비교 방식을 따로 함수로 구성한다. 
BITMAP_t* CGlobalBitmap::FindTexture_Linear(const std::string& filename)
{
	type_bitmap_map::iterator mi = m_mapBitmap.begin();
	for(; mi != m_mapBitmap.end(); mi++)
	{
		BITMAP_t* pBitmap = (*mi).second;

		if(0 == _stricmp(filename.c_str(), pBitmap->FileName)) 
		{
			return pBitmap;
		}
	}
	
	return NULL;
}

BITMAP_t* CGlobalBitmap::FindTextureByName_Linear(const std::string& name)
{
	type_bitmap_map::iterator mi = m_mapBitmap.begin();
	for(; mi != m_mapBitmap.end(); mi++)
	{
		BITMAP_t* pBitmap = (*mi).second;
		std::string texname;
		SplitFileName(pBitmap->FileName, texname, true);
		if(0 == stricmp(texname.c_str(), name.c_str())) 
		{
			return pBitmap;
		}
	}
	
	return NULL;
}
#endif // LDS_OPTIMIZE_FORLOADING


DWORD CGlobalBitmap::GetUsedTextureMemory() const
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
	if(m_DebugOutputTimer.IsTime())
	{
		g_ConsoleDebug->Write(MCD_NORMAL, "CacheSize=%d(NumberOfTexture=%d)", m_BitmapCache.GetCacheSize(), GetNumberOfTexture());
	}
#endif // DEBUG_BITMAP_CACHE
	m_BitmapCache.Update();
}

GLuint CGlobalBitmap::GenerateTextureIndex()
{
	GLuint uiAvailableTextureIndex = FindAvailableTextureIndex(m_uiTextureIndexStream);
	if(uiAvailableTextureIndex >= BITMAP_NONAMED_TEXTURES_END)	//. 범위 초과시
	{
		m_uiAlternate++;
		m_uiTextureIndexStream = BITMAP_NONAMED_TEXTURES_BEGIN;
		uiAvailableTextureIndex = FindAvailableTextureIndex(m_uiTextureIndexStream);	//. 재검색
	}
	return m_uiTextureIndexStream = uiAvailableTextureIndex;
}
GLuint CGlobalBitmap::FindAvailableTextureIndex(GLuint uiSeed)
{
	if(m_uiAlternate > 0)
	{
		type_index_list::iterator li = std::find(m_listNonamedIndex.begin(), m_listNonamedIndex.end(), uiSeed+1);
		if(li != m_listNonamedIndex.end())	//. 있다면
			return FindAvailableTextureIndex(uiSeed+1);	//. 재검색
	}
	return uiSeed+1;
}

bool CGlobalBitmap::OpenJpeg(GLuint uiBitmapIndex, const std::string& filename, GLuint uiFilter, GLuint uiWrapMode)
{
#ifdef DO_PROFILING_FOR_LOADING
	if( g_pProfilerForLoading )
	{
		g_pProfilerForLoading->BeginUnit( EPROFILING_LOADING_OPENJPEG, PROFILING_LOADING_OPENJPEG );
	}
#endif // DO_PROFILING_FOR_LOADING

	

	std::string filename_ozj;
	ExchangeExt(filename, "OZJ", filename_ozj);

	FILE* infile = fopen(filename_ozj.c_str(), "rb");
	if(infile == NULL) 
	{
#ifdef DO_PROFILING_FOR_LOADING
		if( g_pProfilerForLoading )
		{
			g_pProfilerForLoading->EndUnit( EPROFILING_LOADING_OPENJPEG );
		}
#endif // DO_PROFILING_FOR_LOADING
		return false;
	}

	fseek(infile,24,SEEK_SET);	//. Skip Dump
	
	struct jpeg_decompress_struct cinfo;
	struct my_error_mgr jerr;
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;
	if (setjmp(jerr.setjmp_buffer)) 
	{
		jpeg_destroy_decompress(&cinfo);
		fclose(infile);

#ifdef DO_PROFILING_FOR_LOADING
		if( g_pProfilerForLoading )
		{
			g_pProfilerForLoading->EndUnit( EPROFILING_LOADING_OPENJPEG );
		}
#endif // DO_PROFILING_FOR_LOADING

		return false;
	}

	jpeg_create_decompress(&cinfo);
	jpeg_stdio_src(&cinfo, infile);
	(void) jpeg_read_header(&cinfo, TRUE);
	(void) jpeg_start_decompress(&cinfo);

	if(cinfo.output_width<=MAX_WIDTH && cinfo.output_height<=MAX_HEIGHT)
	{
		int Width, Height;
		for(int i=1;i<=MAX_WIDTH;i<<=1)
		{
			Width = i;
			if(i >= (int)cinfo.output_width) break;
		}
		for(int i=1;i<=MAX_HEIGHT;i<<=1)
		{
			Height = i;
			if(i >= (int)cinfo.output_height) break;
		}

		BITMAP_t* pNewBitmap = new BITMAP_t;
		memset(pNewBitmap, 0, sizeof(BITMAP_t));

		pNewBitmap->BitmapIndex = uiBitmapIndex;

		filename._Copy_s(pNewBitmap->FileName, MAX_BITMAP_FILE_NAME, MAX_BITMAP_FILE_NAME);

		pNewBitmap->Width      = (float)Width;
		pNewBitmap->Height     = (float)Height;
		pNewBitmap->Components = 3;
		pNewBitmap->Ref = 1;
		
		size_t BufferSize = Width*Height*pNewBitmap->Components;
		pNewBitmap->Buffer = new BYTE[BufferSize];
		m_dwUsedTextureMemory += BufferSize;
		
		int offset=0;
		int row_stride = cinfo.output_width * cinfo.output_components;
		JSAMPARRAY buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);
		while (cinfo.output_scanline < cinfo.output_height) 
		{
			if(offset+row_stride > (int)BufferSize)
				break;

			(void) jpeg_read_scanlines(&cinfo, buffer, 1);
			memcpy(pNewBitmap->Buffer+(cinfo.output_scanline-1)*Width*3,buffer[0],row_stride);
			offset += row_stride;
		}

#ifdef LDS_OPTIMIZE_FORLOADING
		// Texture Image Open의 Bitmap_map이 구성될 시점에 Bitmap_namemap에 또한 검색 키 값이 되는 
		// filename을 기준으로 Firstkey(FileName), Secondkey(BITMAP_t*) 키로 삽입 한다.
		// 삽입시 각 FileName은 필수로 소문자화 시킨다.
		std::string strFileName, strFullFileName( pNewBitmap->FileName );

		StringToLower(strFullFileName);
		SplitFileName(strFullFileName, strFileName, true);

		// 1. Namemap 에 추가. FindTexture(string) 함수의 검색시 활용 된다.
		m_mapBitmap_Namemap.insert( type_bitmap_namemap::value_type(strFullFileName, pNewBitmap) );

		// 2. NameMap Identity 추가  한다. FindTextureByName(string) 함수의 검색시 활용 된다.
		m_mapBitmap_Namemap_identity.insert( type_bitmap_namemap::value_type(strFileName, pNewBitmap) );
#endif // LDS_OPTIMIZE_FORLOADING

		m_mapBitmap.insert(type_bitmap_map::value_type(uiBitmapIndex, pNewBitmap));
		
		glGenTextures( 1, &(pNewBitmap->TextureNumber));

		glBindTexture(GL_TEXTURE_2D, pNewBitmap->TextureNumber);

        glTexImage2D(GL_TEXTURE_2D, 0, 3, Width, Height, 0, GL_RGB, GL_UNSIGNED_BYTE, pNewBitmap->Buffer);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, uiFilter);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, uiFilter);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, uiWrapMode);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, uiWrapMode);
		
#if defined(LDS_MR0_OUTPUT_GLERROR_IN_BITMAPLOAD) && defined(_DEBUG)
		CheckGLError( __FILE__, __LINE__ );
#endif // LDS_MR0_OUTPUT_GLERROR_IN_BITMAPLOAD
	}
	(void) jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	fclose(infile);

#ifdef DO_PROFILING_FOR_LOADING
	if( g_pProfilerForLoading )
	{
		g_pProfilerForLoading->EndUnit( EPROFILING_LOADING_OPENJPEG );
	}
#endif // DO_PROFILING_FOR_LOADING

	return true;
}
bool CGlobalBitmap::OpenTga(GLuint uiBitmapIndex, const std::string& filename, GLuint uiFilter, GLuint uiWrapMode)
{
#ifdef DO_PROFILING_FOR_LOADING
	if( g_pProfilerForLoading )
	{
		g_pProfilerForLoading->BeginUnit( EPROFILING_LOADING_OPENTGA, PROFILING_LOADING_OPENTGA );
	}
#endif // DO_PROFILING_FOR_LOADING

	std::string filename_ozt;
	ExchangeExt(filename, "OZT", filename_ozt);

	// 파일 오픈과 실패시 처리.
    FILE *fp = fopen(filename_ozt.c_str(), "rb");
    if(fp == NULL)
	{
#ifdef DO_PROFILING_FOR_LOADING
		if( g_pProfilerForLoading )
		{
			g_pProfilerForLoading->EndUnit( EPROFILING_LOADING_OPENTGA );
		}
#endif // DO_PROFILING_FOR_LOADING
		return false;
	}

	// 파일 크기를 얻음.
	fseek(fp,0,SEEK_END);
	int Size = ftell(fp);
	fseek(fp,0,SEEK_SET);

	// 오픈한 파일 내용을 담을 메모리 할당 후 파일을 읽음.
	unsigned char *PakBuffer = new unsigned char [Size];
	fread(PakBuffer,1,Size,fp);
	fclose(fp);
	
	// OZT확장자 헤더정보를 읽음.
	// 인덱스 16(12 + 4)부터.
    int index = 12;
	index += 4;
	// 인덱스 16은 이미지 가로크기.
    short nx = *((short *)(PakBuffer+index));index+=2;
	// 인덱스 18은 이미지 세로크기.
    short ny = *((short *)(PakBuffer+index));index+=2;
	// 인덱스 20은 이미지 도트의 color bit.(예: 32bit, 16bit)
    char bit = *((char *)(PakBuffer+index));index+=1;
	index += 1;	// 현재 인덱스는 22.

	// 32bit color가 아니거나 이미지 최대 크기를 넘으면 실패.
    if(bit!=32 || nx>MAX_WIDTH || ny>MAX_HEIGHT)
	{
#ifdef KJH_FIX_ARRAY_DELETE
		SAFE_DELETE_ARRAY(PakBuffer);
#else // KJH_FIX_ARRAY_DELETE
		delete PakBuffer;
#endif // KJH_FIX_ARRAY_DELETE

#ifdef DO_PROFILING_FOR_LOADING
		if( g_pProfilerForLoading )
		{
			g_pProfilerForLoading->EndUnit( EPROFILING_LOADING_OPENTGA );
		}
#endif // DO_PROFILING_FOR_LOADING

		return false;
	}

	int Width = 0, Height = 0;
	for(int i=1;i<=MAX_WIDTH;i<<=1)
	{
		Width = i;
		if(i >= nx) break;
	}
	for(int i=1;i<=MAX_HEIGHT;i<<=1)
	{
		Height = i;
		if(i >= ny) break;
	}

	BITMAP_t* pNewBitmap = new BITMAP_t;
	memset(pNewBitmap, 0, sizeof(BITMAP_t));
	
	pNewBitmap->BitmapIndex = uiBitmapIndex;

	filename._Copy_s(pNewBitmap->FileName, MAX_BITMAP_FILE_NAME, MAX_BITMAP_FILE_NAME);
	
	pNewBitmap->Width      = (float)Width;
	pNewBitmap->Height     = (float)Height;
	pNewBitmap->Components = 4;
	pNewBitmap->Ref = 1;
	
	size_t BufferSize = Width*Height*pNewBitmap->Components;
	pNewBitmap->Buffer     = (unsigned char*)new BYTE[BufferSize];

	m_dwUsedTextureMemory += BufferSize;

    for(int y=0;y<ny;y++)
	{
        unsigned char *src = &PakBuffer[index];
		index += nx * 4;
		unsigned char *dst = &pNewBitmap->Buffer[(ny-1-y)*Width*pNewBitmap->Components];

		for(int x=0;x<nx;x++)
        {
			dst[0] = src[2];
			dst[1] = src[1];
			dst[2] = src[0];
			dst[3] = src[3];
			src += 4;
			dst += pNewBitmap->Components;
        }
	}
#ifdef KJH_FIX_ARRAY_DELETE
	SAFE_DELETE_ARRAY(PakBuffer);
#else //KJH_FIX_ARRAY_DELETE
	delete PakBuffer;
#endif // KJH_FIX_ARRAY_DELETE

#ifdef LDS_OPTIMIZE_FORLOADING
	std::string strFileName, strFullFileName( pNewBitmap->FileName );
	
	StringToLower(strFullFileName);
	SplitFileName(strFullFileName, strFileName, true);
	
	m_mapBitmap_Namemap.insert( type_bitmap_namemap::value_type(strFullFileName, pNewBitmap) );
	
	m_mapBitmap_Namemap_identity.insert( type_bitmap_namemap::value_type(strFileName, pNewBitmap) );
#endif // LDS_OPTIMIZE_FORLOADING

	m_mapBitmap.insert(type_bitmap_map::value_type(uiBitmapIndex, pNewBitmap));
	
	glGenTextures( 1, &(pNewBitmap->TextureNumber));

	glBindTexture(GL_TEXTURE_2D, pNewBitmap->TextureNumber);

    glTexImage2D(GL_TEXTURE_2D, 0, 4, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pNewBitmap->Buffer);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, uiFilter);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, uiFilter);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, uiWrapMode);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, uiWrapMode);

#if defined(LDS_MR0_OUTPUT_GLERROR_IN_BITMAPLOAD) && defined(_DEBUG)
	CheckGLError( __FILE__, __LINE__ );
#endif // LDS_MR0_OUTPUT_GLERROR_IN_BITMAPLOAD


#ifdef DO_PROFILING_FOR_LOADING
	if( g_pProfilerForLoading )
	{
		g_pProfilerForLoading->EndUnit( EPROFILING_LOADING_OPENTGA );
	}
#endif // DO_PROFILING_FOR_LOADING

	return true;
}

void CGlobalBitmap::SplitFileName(IN const std::string& filepath, OUT std::string& filename, bool bIncludeExt) 
{
	char __fname[_MAX_FNAME] = {0, };
	char __ext[_MAX_EXT] = {0, };
	_splitpath(filepath.c_str(), NULL, NULL, __fname, __ext);
	filename = __fname;
	if(bIncludeExt)
		filename += __ext;
}
void CGlobalBitmap::SplitExt(IN const std::string& filepath, OUT std::string& ext, bool bIncludeDot) 
{
	char __ext[_MAX_EXT] = {0, };
	_splitpath(filepath.c_str(), NULL, NULL, NULL, __ext);
	if(bIncludeDot) {
		ext = __ext;
	}
	else {
		if((__ext[0] == '.') && __ext[1])
			ext = __ext+1;
	}
}
void CGlobalBitmap::ExchangeExt(IN const std::string& in_filepath, IN const std::string& ext, OUT std::string& out_filepath) 
{
	char __drive[_MAX_DRIVE] = {0, };
	char __dir[_MAX_DIR] = {0, };
	char __fname[_MAX_FNAME] = {0, };
	_splitpath(in_filepath.c_str(), __drive, __dir, __fname, NULL);
	
	out_filepath = __drive;
	out_filepath += __dir;
	out_filepath += __fname;
	out_filepath += '.';
	out_filepath += ext;
}

bool CGlobalBitmap::Convert_Format(const unicode::t_string& filename)
{
	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];
	
	::_splitpath(filename.c_str(), drive, dir, fname, ext);
	
	std::string strPath = drive; strPath += dir;
	std::string strName = fname;
	
	if(_stricmp(ext, ".jpg") == 0) 
	{
		unicode::t_string strSaveName = strPath + strName + ".OZJ";
		return Save_Image(filename, strSaveName.c_str(), 24);
	}
	else if(_stricmp(ext, ".tga") == 0) 
	{
		unicode::t_string strSaveName = strPath + strName + ".OZT";
		return Save_Image(filename, strSaveName.c_str(), 4);
	}
	else if(_stricmp(ext, ".bmp") == 0) 
	{
		unicode::t_string strSaveName = strPath + strName + ".OZB";
		return Save_Image(filename, strSaveName.c_str(), 4);
	}
	else
	{
		// 이미지 포멧이 다릅니다.
	}

	return false;
}

bool CGlobalBitmap::Save_Image(const unicode::t_string& src, const unicode::t_string& dest, int cDumpHeader)
{
	FILE* fp = fopen(src.c_str(), "rb");
	if(fp == NULL) 
	{
		return false;
	}
	
	fseek(fp, 0, SEEK_END);
	int size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	
	char* pTempBuf = new char[size];
	fread(pTempBuf, 1, size, fp);
	fclose(fp);
	
	fp = fopen(dest.c_str(), "wb");
	if(fp == NULL) 
		return false;
	
	fwrite(pTempBuf, 1, cDumpHeader, fp);
	fwrite(pTempBuf, 1, size, fp);
	fclose(fp);
	
	delete [] pTempBuf;
	
	return true;
}

void CGlobalBitmap::my_error_exit(j_common_ptr cinfo)
{
	my_error_ptr myerr = (my_error_ptr) cinfo->err;
	(*cinfo->err->output_message) (cinfo);
	longjmp(myerr->setjmp_buffer, 1);
}


////////////////////////////////////////////////////////////////////////// CGlobalBitmap