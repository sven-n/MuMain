//////////////////////////////////////////////////////////////////////////
//  GuildCache.h
//////////////////////////////////////////////////////////////////////////

#if !defined(AFX_GUILDCACHE_H__5B2412C7_A182_4E40_99F5_2CA896A9EE64__INCLUDED_)
#define AFX_GUILDCACHE_H__5B2412C7_A182_4E40_99F5_2CA896A9EE64__INCLUDED_

#pragma once

extern MARK_t GuildMark[MAX_MARKS];

class CGuildCache
{
public:
    CGuildCache();
    virtual ~CGuildCache();

protected:
    DWORD	m_dwCurrIndex;

public:
    void Reset();

    BOOL IsExistGuildMark(int nGuildKey);
    int GetGuildMarkIndex(int nGuildKey);

    int MakeGuildMarkIndex(int nGuildKey);
    int SetGuildMark(int nGuildKey, char* UnionName, char* GuildName, BYTE* Mark);
};

extern CGuildCache g_GuildCache;

#endif // !defined(AFX_GUILDCACHE_H__5B2412C7_A182_4E40_99F5_2CA896A9EE64__INCLUDED_)
