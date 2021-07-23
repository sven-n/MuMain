//////////////////////////////////////////////////////////////////////////
//  
//  GuildCache.h
//  
//  내  용 : 길드정보 캐싱
//  
//  날  짜 : 2004년 11월 09일
//  
//  작성자 : 강 병 국
//  
//////////////////////////////////////////////////////////////////////////

#if !defined(AFX_GUILDCACHE_H__5B2412C7_A182_4E40_99F5_2CA896A9EE64__INCLUDED_)
#define AFX_GUILDCACHE_H__5B2412C7_A182_4E40_99F5_2CA896A9EE64__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

extern MARK_t GuildMark[MAX_MARKS];

// 길드 정보 캐시
class CGuildCache
{
public:
	CGuildCache();
	virtual ~CGuildCache();

protected:
	DWORD	m_dwCurrIndex;											// 빈공간 인덱스 (0부터 누적)

public:
	void Reset();													// 서버이동하거나 재접속시 호출

	BOOL IsExistGuildMark( int nGuildKey );							// 길드마크 존재여부 확인
	int GetGuildMarkIndex( int nGuildKey );							// 길드키로 길드마크 인덱스 반환

	int MakeGuildMarkIndex( int nGuildKey );						// 길드마크 공간확보
	int SetGuildMark( int nGuildKey, BYTE* UnionName, BYTE* GuildName,BYTE* Mark );		// 길드마크 공간에 마크 넣기
};

#endif // !defined(AFX_GUILDCACHE_H__5B2412C7_A182_4E40_99F5_2CA896A9EE64__INCLUDED_)
