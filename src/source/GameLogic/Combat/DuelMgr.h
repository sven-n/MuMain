#pragma once

#include <array>
#include <string>
#include <vector>

struct DUEL_PLAYER_INFO
{
    short m_sIndex = 0;
    wchar_t m_szID[MAX_USERNAME_SIZE + 1]{};
    int m_iScore = 0;
    float m_fHPRate = 0.0f;
    float m_fSDRate = 0.0f;
};

enum _DUEL_PLAYER_TYPE
{
    DUEL_HERO,
    DUEL_ENEMY,
    MAX_DUEL_PLAYERS
};

struct DUEL_CHANNEL_INFO
{
    BOOL m_bEnable = FALSE;
    BOOL m_bJoinable = FALSE;
    wchar_t m_szID1[MAX_USERNAME_SIZE + 1]{};
    wchar_t m_szID2[MAX_USERNAME_SIZE + 1]{};
};

constexpr int MAX_DUEL_CHANNELS = 4;

class CDuelMgr
{
public:
    CDuelMgr();
    virtual ~CDuelMgr();
    void Reset();

public:
    void EnableDuel(BOOL bEnable);
    BOOL IsDuelEnabled();

    void EnablePetDuel(BOOL bEnable);
    BOOL IsPetDuelEnabled();

    void SetDuelPlayer(int iPlayerNum, short iIndex, const wchar_t* pszID);
    void SetHeroAsDuelPlayer(int iPlayerNum);
    void SetScore(int iPlayerNum, int iScore);
    void SetHP(int iPlayerNum, int iRate);
    void SetSD(int iPlayerNum, int iRate);

    const wchar_t* GetDuelPlayerID(int iPlayerNum) const;
    int GetScore(int iPlayerNum);
    float GetHP(int iPlayerNum);
    float GetSD(int iPlayerNum);

    BOOL IsDuelPlayer(CHARACTER* pCharacter, int iPlayerNum, BOOL bIncludeSummon = TRUE);
    BOOL IsDuelPlayer(WORD wIndex, int iPlayerNum);

    void SendDuelRequestAnswer(int iPlayerNum, BOOL bOK);

protected:
    BOOL m_bIsDuelEnabled;
    BOOL m_bIsPetDuelEnabled;

public:
    void SetDuelChannel(int iChannelIndex, BOOL bEnable, BOOL bJoinable, const wchar_t* pszID1, const wchar_t* pszID2);
    BOOL IsDuelChannelEnabled(int iChannelIndex)
    {
        return m_DuelChannels[iChannelIndex].m_bEnable;
    }
    BOOL IsDuelChannelJoinable(int iChannelIndex)
    {
        return m_DuelChannels[iChannelIndex].m_bJoinable;
    }
    const wchar_t* GetDuelChannelUserID1(int iChannelIndex) const
    {
        return m_DuelChannels[iChannelIndex].m_szID1;
    }
    const wchar_t* GetDuelChannelUserID2(int iChannelIndex) const
    {
        return m_DuelChannels[iChannelIndex].m_szID2;
    }

    void SetCurrentChannel(int iChannel = -1)
    {
        m_iCurrentChannel = iChannel;
    }
    int GetCurrentChannel()
    {
        return m_iCurrentChannel;
    }

    void RemoveAllDuelWatchUser();
    void AddDuelWatchUser(const wchar_t* pszUserID);
    void RemoveDuelWatchUser(const wchar_t* pszUserID);
    const wchar_t* GetDuelWatchUser(int iIndex) const;
    int GetDuelWatchUserCount() const
    {
        return static_cast<int>(m_DuelWatchUserList.size());
    }

    BOOL GetFighterRegenerated()
    {
        return m_bRegenerated;
    }
    void SetFighterRegenerated(BOOL bFlag)
    {
        m_bRegenerated = bFlag;
    }

protected:
    std::array<DUEL_PLAYER_INFO, MAX_DUEL_PLAYERS> m_DuelPlayer{};
    std::array<DUEL_CHANNEL_INFO, MAX_DUEL_CHANNELS> m_DuelChannels{};
    int m_iCurrentChannel;

    BOOL m_bRegenerated;

    std::vector<std::wstring> m_DuelWatchUserList;
};

extern CDuelMgr g_DuelMgr;
