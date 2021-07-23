//////////////////////////////////////////////////////////////////////////
//  
//  GuildCache.cpp
//  
//  내  용 : 길드정보 캐싱
//  
//  날  짜 : 2004년 11월 09일
//  
//  작성자 : 강 병 국
//  
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UIManager.h"
#include "ZzzOpenglUtil.h"
#include "ZzzTexture.h"
#include "GuildCache.h"

CGuildCache g_GuildCache;

CGuildCache::CGuildCache()
{
	Reset();
}

CGuildCache::~CGuildCache()
{
}

void CGuildCache::Reset()
{
	m_dwCurrIndex = 0;
	for( int i=0 ; i<MAX_MARKS; ++i )
		GuildMark[i].Key = -1;
}

int CGuildCache::GetGuildMarkIndex( int nGuildKey )
{
#ifdef _VS2008PORTING
	for( int i=0 ; i<=(int)m_dwCurrIndex ; ++i )
#else // _VS2008PORTING
	for( int i=0 ; i<=m_dwCurrIndex ; ++i )
#endif // _VS2008PORTING
	{
		if( GuildMark[i].Key == nGuildKey )
			return i;
	}
	return -1;
}

BOOL CGuildCache::IsExistGuildMark( int nGuildKey )
{
	if( GetGuildMarkIndex( nGuildKey ) == -1 )
		return FALSE;
	else
		return TRUE;
}

int CGuildCache::MakeGuildMarkIndex( int nGuildKey )
{
	if( m_dwCurrIndex >= MAX_MARKS )
	{
		assert( !"길드마크 버퍼초과" );
		return -1;
	}

	GuildMark[m_dwCurrIndex].Key = nGuildKey;
	return m_dwCurrIndex++;
}

int CGuildCache::SetGuildMark( int nGuildKey, BYTE* UnionName, BYTE* GuildName, BYTE* Mark )
{
	int nIndex = GetGuildMarkIndex( nGuildKey );
	if( nIndex != -1 )
	{
		memcpy( GuildMark[nIndex].UnionName, UnionName, 8 );
		GuildMark[nIndex].UnionName[8] = NULL;
		memcpy( GuildMark[nIndex].GuildName, GuildName, 8 );
		GuildMark[nIndex].GuildName[8] = NULL;
		for( int i=0 ; i<64 ; ++i )
		{
			if( i%2 == 0 )
				GuildMark[nIndex].Mark[i] = ( Mark[i/2]>>4 )&0x0f;
			else
				GuildMark[nIndex].Mark[i] = Mark[i/2]&0x0f;
		}
	}
	else
		assert( !"없는 길드마크" );

	return nIndex;
}
