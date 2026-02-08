// DuelMgr.cpp: implementation of the CDuelMgr class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DuelMgr.h"
#include "ZzzInfomation.h"
#include "ZzzCharacter.h"

#include "CharacterManager.h"
#include "WSclient.h"

#include <algorithm>
#include <cwchar>

CDuelMgr g_DuelMgr;

CDuelMgr::CDuelMgr()
{
    Reset();
}

CDuelMgr::~CDuelMgr()
{
    Reset();
}

void CDuelMgr::Reset()
{
    m_bIsDuelEnabled = FALSE;
    m_bIsPetDuelEnabled = FALSE;
    m_DuelPlayer = {};
    m_DuelChannels = {};
    m_iCurrentChannel = -1;
    m_bRegenerated = FALSE;
    RemoveAllDuelWatchUser();
}

void CDuelMgr::EnableDuel(BOOL bEnable)
{
    m_bIsDuelEnabled = bEnable;

    if (bEnable == FALSE)
    {
        Reset();
    }
}

BOOL CDuelMgr::IsDuelEnabled()
{
    return m_bIsDuelEnabled;
}

void CDuelMgr::EnablePetDuel(BOOL bEnable)
{
    m_bIsPetDuelEnabled = bEnable;
}

BOOL CDuelMgr::IsPetDuelEnabled()
{
    return m_bIsPetDuelEnabled;
}

void CDuelMgr::SetDuelPlayer(int iPlayerNum, short sIndex, const wchar_t* pszID)
{
    m_DuelPlayer[iPlayerNum].m_sIndex = sIndex;
    if (pszID != nullptr)
    {
        wcsncpy_s(m_DuelPlayer[iPlayerNum].m_szID, pszID, MAX_USERNAME_SIZE);
    }
    else
    {
        m_DuelPlayer[iPlayerNum].m_szID[0] = L'\0';
    }
    g_ConsoleDebug->Write(MCD_NORMAL, L"[SetDuelPlayer] %d, %ls", sIndex, pszID ? pszID : L"(null)");
}

void CDuelMgr::SetHeroAsDuelPlayer(int iPlayerNum)
{
    m_DuelPlayer[iPlayerNum].m_sIndex = Hero->Key;
    wcsncpy_s(m_DuelPlayer[iPlayerNum].m_szID, Hero->ID, MAX_USERNAME_SIZE);
}

void CDuelMgr::SetScore(int iPlayerNum, int iScore)
{
    m_DuelPlayer[iPlayerNum].m_iScore = iScore;
}

void CDuelMgr::SetHP(int iPlayerNum, int iRate)
{
    m_DuelPlayer[iPlayerNum].m_fHPRate = iRate * 0.01f;
}

void CDuelMgr::SetSD(int iPlayerNum, int iRate)
{
    m_DuelPlayer[iPlayerNum].m_fSDRate = iRate * 0.01f;
}

const wchar_t* CDuelMgr::GetDuelPlayerID(int iPlayerNum) const
{
    return m_DuelPlayer[iPlayerNum].m_szID;
}

int CDuelMgr::GetScore(int iPlayerNum)
{
    return m_DuelPlayer[iPlayerNum].m_iScore;
}

float CDuelMgr::GetHP(int iPlayerNum)
{
    return m_DuelPlayer[iPlayerNum].m_fHPRate;
}

float CDuelMgr::GetSD(int iPlayerNum)
{
    return m_DuelPlayer[iPlayerNum].m_fSDRate;
}

BOOL CDuelMgr::IsDuelPlayer(CHARACTER* pCharacter, int iPlayerNum, BOOL bIncludeSummon)
{
    if (pCharacter->Key == m_DuelPlayer[iPlayerNum].m_sIndex && wcsncmp(pCharacter->ID, m_DuelPlayer[iPlayerNum].m_szID, MAX_USERNAME_SIZE) == 0)
    {
        return TRUE;
    }
    else if (bIncludeSummon == TRUE && gCharacterManager.GetBaseClass(pCharacter->Class) == 0 && wcsncmp(pCharacter->OwnerID, m_DuelPlayer[iPlayerNum].m_szID, MAX_USERNAME_SIZE) == 0)
    {
        return TRUE;
    }

    return FALSE;
}

BOOL CDuelMgr::IsDuelPlayer(WORD wIndex, int iPlayerNum)
{
    return (m_DuelPlayer[iPlayerNum].m_sIndex == wIndex);
}

void CDuelMgr::SendDuelRequestAnswer(int iPlayerNum, BOOL bOK)
{
    SocketClient->ToGameServer()->SendDuelStartResponse(bOK, m_DuelPlayer[iPlayerNum].m_sIndex, m_DuelPlayer[iPlayerNum].m_szID);
}

void CDuelMgr::SetDuelChannel(int iChannelIndex, BOOL bEnable, BOOL bJoinable, const wchar_t* pszID1, const wchar_t* pszID2)
{
    auto& channel = m_DuelChannels[iChannelIndex];
    channel.m_bEnable = bEnable;
    channel.m_bJoinable = bJoinable;
    if (pszID1 != nullptr)
    {
        wcsncpy_s(channel.m_szID1, pszID1, MAX_USERNAME_SIZE);
    }
    else
    {
        channel.m_szID1[0] = L'\0';
    }
    if (pszID2 != nullptr)
    {
        wcsncpy_s(channel.m_szID2, pszID2, MAX_USERNAME_SIZE);
    }
    else
    {
        channel.m_szID2[0] = L'\0';
    }
}

void CDuelMgr::RemoveAllDuelWatchUser()
{
    m_DuelWatchUserList.clear();
}

void CDuelMgr::AddDuelWatchUser(const wchar_t* pszUserID)
{
    if (pszUserID == nullptr)
    {
        return;
    }

    m_DuelWatchUserList.emplace_back(pszUserID, wcsnlen(pszUserID, MAX_USERNAME_SIZE));
}

void CDuelMgr::RemoveDuelWatchUser(const wchar_t* pszUserID)
{
    if (pszUserID == nullptr)
    {
        return;
    }

    const auto matcher = [pszUserID](const std::wstring& name)
    {
        return wcsncmp(name.c_str(), pszUserID, MAX_USERNAME_SIZE) == 0;
    };

    const auto iter = std::remove_if(m_DuelWatchUserList.begin(), m_DuelWatchUserList.end(), matcher);
    if (iter != m_DuelWatchUserList.end())
    {
        m_DuelWatchUserList.erase(iter, m_DuelWatchUserList.end());
    }
    else
    {
        assert(!L"RemoveDuelWatchUser!");
    }
}

const wchar_t* CDuelMgr::GetDuelWatchUser(int iIndex) const
{
    if (iIndex < 0 || static_cast<std::size_t>(iIndex) >= m_DuelWatchUserList.size())
    {
        return nullptr;
    }

    return m_DuelWatchUserList[static_cast<std::size_t>(iIndex)].c_str();
}