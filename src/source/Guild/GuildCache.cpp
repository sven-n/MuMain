#include "stdafx.h"
#include "UIManager.h"
#include "ZzzOpenglUtil.h"
#include "ZzzTexture.h"
#include "GuildCache.h"
#include "GuildConstants.h"

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
    for (int i = 0; i < MAX_MARKS; ++i)
        GuildMark[i].Key = GuildConstants::INVALID_MARK_INDEX;
}

int CGuildCache::GetGuildMarkIndex(int nGuildKey)
{
    for (int i = 0; i <= (int)m_dwCurrIndex; ++i)
    {
        if (GuildMark[i].Key == nGuildKey)
            return i;
    }
    return GuildConstants::INVALID_MARK_INDEX;
}

BOOL CGuildCache::IsExistGuildMark(int nGuildKey)
{
    if (GetGuildMarkIndex(nGuildKey) == GuildConstants::INVALID_MARK_INDEX)
        return FALSE;
    else
        return TRUE;
}

int CGuildCache::MakeGuildMarkIndex(int nGuildKey)
{
    if (m_dwCurrIndex >= MAX_MARKS)
    {
        assert(!"Guild mark buffer exceeded");
        return GuildConstants::INVALID_MARK_INDEX;
    }

    GuildMark[m_dwCurrIndex].Key = nGuildKey;
    return m_dwCurrIndex++;
}

int CGuildCache::SetGuildMark(int nGuildKey, char* UnionName, char* GuildName, BYTE* Mark)
{
    int nIndex = GetGuildMarkIndex(nGuildKey);
    if (nIndex != GuildConstants::INVALID_MARK_INDEX)
    {
        CMultiLanguage::ConvertFromUtf8(GuildMark[nIndex].UnionName, UnionName, GuildConstants::GUILD_NAME_LENGTH);
        CMultiLanguage::ConvertFromUtf8(GuildMark[nIndex].GuildName, GuildName, GuildConstants::GUILD_NAME_LENGTH);
        GuildMark[nIndex].UnionName[GuildConstants::GUILD_NAME_LENGTH] = L'\0';
        GuildMark[nIndex].GuildName[GuildConstants::GUILD_NAME_LENGTH] = L'\0';

        // Unpack compressed mark data: each byte contains two 4-bit values (nibbles)
        for (int i = 0; i < GuildConstants::GUILD_MARK_SIZE; ++i)
        {
            if (i % 2 == 0)
                GuildMark[nIndex].Mark[i] = (Mark[i / 2] >> GuildConstants::MARK_NIBBLE_SHIFT) & GuildConstants::MARK_NIBBLE_MASK;
            else
                GuildMark[nIndex].Mark[i] = Mark[i / 2] & GuildConstants::MARK_NIBBLE_MASK;
        }
    }
    else
        assert(!"Guild mark not found");

    return nIndex;
}