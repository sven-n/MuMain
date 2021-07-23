// DuelMgr.h: interface for the CDuelMgr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DUELMGR_H__9B443DA1_CBFB_49E7_9BE9_2BB718489DE3__INCLUDED_)
#define AFX_DUELMGR_H__9B443DA1_CBFB_49E7_9BE9_2BB718489DE3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef YDG_ADD_NEW_DUEL_SYSTEM

typedef struct _DUEL_PLAYER_INFO
{
	int m_iIndex;		// 인덱스
	char m_szID[24];	// ID
	int m_iScore;		// 점수
	float m_fHPRate;	// HP 비율 (0~1)
	float m_fSDRate;	// SD 비율 (0~1)
} DUEL_PLAYER_INFO;

enum _DUEL_PLAYER_TYPE
{
	DUEL_HERO,
	DUEL_ENEMY,
	MAX_DUEL_PLAYERS
};

typedef struct _DUEL_CHANNEL_INFO
{
	BOOL m_bEnable;		// 방 개설 여부
	BOOL m_bJoinable;	// 관전 가능 여부
	char m_szID1[24];	// 유저1 ID
	char m_szID2[24];	// 유저2 ID
} DUEL_CHANNEL_INFO;

#define MAX_DUEL_CHANNELS 4

class CDuelMgr  
{
public:
	CDuelMgr();
	virtual ~CDuelMgr();

	void Reset();

//////////////////////////////////////////////////////////////////////////
// 결투 직접 할때 관련
public:
	void EnableDuel(BOOL bEnable);
	BOOL IsDuelEnabled();

	void EnablePetDuel(BOOL bEnable);
	BOOL IsPetDuelEnabled();

	void SetDuelPlayer(int iPlayerNum, int iIndex, char * pszID);
	void SetHeroAsDuelPlayer(int iPlayerNum);
	void SetScore(int iPlayerNum, int iScore);
	void SetHP(int iPlayerNum, int iRate);
	void SetSD(int iPlayerNum, int iRate);

	char * GetDuelPlayerID(int iPlayerNum);
	int GetScore(int iPlayerNum);
	float GetHP(int iPlayerNum);
	float GetSD(int iPlayerNum);

#ifdef YDG_FIX_DUEL_SUMMON_CLOAK
	BOOL IsDuelPlayer(CHARACTER * pCharacter, int iPlayerNum, BOOL bIncludeSummon = TRUE);
#else	// YDG_FIX_DUEL_SUMMON_CLOAK
	BOOL IsDuelPlayer(CHARACTER * pCharacter, int iPlayerNum);
#endif	// YDG_FIX_DUEL_SUMMON_CLOAK
	BOOL IsDuelPlayer(WORD wIndex, int iPlayerNum);

	void SendDuelRequestAnswer(int iPlayerNum, BOOL bOK);

protected:
	BOOL m_bIsDuelEnabled;		// 결투 상태 여부
	BOOL m_bIsPetDuelEnabled;	// LHJ - 결투중에 다크로드의 다크스피릿이 공격중 인지 판단하는 변수
	DUEL_PLAYER_INFO m_DuelPlayer[MAX_DUEL_PLAYERS];	// 결투자 상태

//////////////////////////////////////////////////////////////////////////
// 결투 관전 관련
public:
	void SetDuelChannel(int iChannelIndex, BOOL bEnable, BOOL bJoinable, char * pszID1, char * pszID2);
	BOOL IsDuelChannelEnabled(int iChannelIndex) { return m_DuelChannels[iChannelIndex].m_bEnable; }
	BOOL IsDuelChannelJoinable(int iChannelIndex) { return m_DuelChannels[iChannelIndex].m_bJoinable; }
	char * GetDuelChannelUserID1(int iChannelIndex) { return m_DuelChannels[iChannelIndex].m_szID1; }
	char * GetDuelChannelUserID2(int iChannelIndex) { return m_DuelChannels[iChannelIndex].m_szID2; }

	void SetCurrentChannel(int iChannel = -1) { m_iCurrentChannel = iChannel; }
	int GetCurrentChannel() { return m_iCurrentChannel; }

	void RemoveAllDuelWatchUser();
	void AddDuelWatchUser(char * pszUserID);
	void RemoveDuelWatchUser(char * pszUserID);
	char * GetDuelWatchUser(int iIndex);
	int GetDuelWatchUserCount() { return m_DuelWatchUserList.size(); }

	BOOL GetFighterRegenerated() { return m_bRegenerated; }
	void SetFighterRegenerated(BOOL bFlag) { m_bRegenerated = bFlag; }

protected:
	DUEL_CHANNEL_INFO m_DuelChannels[MAX_DUEL_CHANNELS];
	int m_iCurrentChannel;

	BOOL m_bRegenerated;	// 막 부활했나

	std::list<std::string> m_DuelWatchUserList;
};

extern CDuelMgr g_DuelMgr;

#endif	// YDG_ADD_NEW_DUEL_SYSTEM

#endif // !defined(AFX_DUELMGR_H__9B443DA1_CBFB_49E7_9BE9_2BB718489DE3__INCLUDED_)
