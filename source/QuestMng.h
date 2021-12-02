//*****************************************************************************
// File: QuestMng.h
//*****************************************************************************

#if !defined(AFX_QUESTMNG_H__1ACBD446_412D_488D_9E0F_AC5BC22BEFE8__INCLUDED_)
#define AFX_QUESTMNG_H__1ACBD446_412D_488D_9E0F_AC5BC22BEFE8__INCLUDED_

#pragma once

#ifdef ASG_ADD_NEW_QUEST_SYSTEM

#include "WSclient.h"

#ifdef ASG_ADD_UI_NPC_DIALOGUE
#define QM_MAX_ND_ANSWER	10
#endif	// ASG_ADD_UI_NPC_DIALOGUE
#define QM_MAX_ANSWER		5
#define QM_MAX_REQUEST_REWARD_TEXT_LEN	64

#ifdef ASG_ADD_UI_NPC_DIALOGUE
struct SNPCDialogue
{
	int		m_nNPCWords;
	int		m_anAnswer[QM_MAX_ND_ANSWER*2];
};
#endif	// ASG_ADD_UI_NPC_DIALOGUE

#pragma pack(push, 1)
struct SQuestProgress
{
	BYTE	m_byUIType;
	int		m_nNPCWords;
	int		m_nPlayerWords;
	int		m_anAnswer[QM_MAX_ANSWER];
	int		m_nSubject;
	int		m_nSummary;
};
#pragma pack(pop)

#ifndef ASG_MOD_QUEST_WORDS_SCRIPTS
struct SQuestWords
{
	short	m_nAction;
	BYTE	m_byActCount;
	string	m_strWords;
};
#endif	// ASG_MOD_QUEST_WORDS_SCRIPTS

struct SQuestRequest
{
	DWORD	m_dwType;
	WORD	m_wIndex;
	DWORD	m_dwValue;
#ifdef ASG_ADD_TIME_LIMIT_QUEST
	DWORD	m_dwCurValue;
#else	// ASG_ADD_TIME_LIMIT_QUEST
	WORD	m_wCurValue;
#endif	// ASG_ADD_TIME_LIMIT_QUEST
	ITEM*	m_pItem;
};

struct SQuestReward
{	
	DWORD	m_dwType;
	WORD	m_wIndex;
	DWORD	m_dwValue;
	ITEM*	m_pItem;
};

#ifdef ASG_ADD_QUEST_REQUEST_REWARD_TYPE
struct SQuestRequestReward
{
	BYTE	m_byRequestCount;
	BYTE	m_byGeneralRewardCount;	
	BYTE	m_byRandRewardCount;
	BYTE	m_byRandGiveCount;
	SQuestRequest	m_aRequest[5];
	SQuestReward	m_aReward[5];
};
#else	// ASG_ADD_QUEST_REQUEST_REWARD_TYPE
struct SQuestRequestReward
{
	BYTE	m_byRequestCount;
	BYTE	m_byRewardCount;
	SQuestRequest	m_aRequest[5];
	SQuestReward	m_aReward[5];
};
#endif	// ASG_ADD_QUEST_REQUEST_REWARD_TYPE

enum REQUEST_REWARD_CLASSIFY
{
	RRC_NONE	= 0,
	RRC_REQUEST	= 1,
	RRC_REWARD	= 2
};

struct SRequestRewardText
{
	HFONT	m_hFont;
	DWORD	m_dwColor;
	char	m_szText[QM_MAX_REQUEST_REWARD_TEXT_LEN];
	REQUEST_REWARD_CLASSIFY	m_eRequestReward;		// 
	DWORD	m_dwType;
	WORD	m_wIndex;
	ITEM*	m_pItem;
};

#ifdef ASG_ADD_UI_NPC_DIALOGUE
typedef std::map<DWORD, SNPCDialogue> NPCDialogueMap;
#endif	// ASG_ADD_UI_NPC_DIALOGUE
typedef std::map<DWORD, SQuestProgress> QuestProgressMap;
#ifdef ASG_MOD_QUEST_WORDS_SCRIPTS
typedef std::map<int, std::string> QuestWordsMap;
#else	// ASG_MOD_QUEST_WORDS_SCRIPTS
typedef map<int, SQuestWords> QuestWordsMap;
#endif	// ASG_MOD_QUEST_WORDS_SCRIPTS
typedef std::map<DWORD, SQuestRequestReward> QuestRequestRewardMap;
typedef std::list<DWORD> DWordList;

class CQuestMng  
{
protected:
#ifdef ASG_ADD_UI_NPC_DIALOGUE
	NPCDialogueMap			m_mapNPCDialogue;
#endif	// ASG_ADD_UI_NPC_DIALOGUE
	QuestProgressMap		m_mapQuestProgress;
	QuestWordsMap			m_mapQuestWords;
	QuestRequestRewardMap	m_mapQuestRequestReward;

	std::map<WORD, bool>			m_mapEPRequestRewardState;

	int		m_nNPCIndex;
	char	m_szNPCName[32];

	DWordList				m_listQuestIndexByEtc;
	DWordList				m_listCurQuestIndex;

public:
	CQuestMng();
	virtual ~CQuestMng();

	void LoadQuestScript();
	void SetQuestRequestReward(BYTE* pbyRequestRewardPacket);
	const SQuestRequestReward* GetRequestReward(DWORD dwQuestIndex);

	void SetNPC(int nNPCIndex);
	int GetNPCIndex();
	char* GetNPCName();

	void SetCurQuestProgress(DWORD dwQuestIndex);

	const char* GetWords(int nWordsIndex);
#ifdef ASG_ADD_UI_NPC_DIALOGUE
	const char* GetNPCDlgNPCWords(DWORD dwDlgState);
	const char* GetNPCDlgAnswer(DWORD dwDlgState, int nAnswer);
	int GetNPCDlgAnswerResult(DWORD dwDlgState, int nAnswer);
#endif	// ASG_ADD_UI_NPC_DIALOGUE
	const char* GetNPCWords(DWORD dwQuestIndex);
	const char* GetPlayerWords(DWORD dwQuestIndex);
	const char* GetAnswer(DWORD dwQuestIndex, int nAnswer);
	const char* GetSubject(DWORD dwQuestIndex);
	const char* GetSummary(DWORD dwQuestIndex);
	bool IsRequestRewardQS(DWORD dwQuestIndex);
	bool GetRequestRewardText(SRequestRewardText* aDest, int nDestCount, DWORD dwQuestIndex);
	void SetEPRequestRewardState(DWORD dwQuestIndex, bool ProgressState);
	bool IsEPRequestRewardState(DWORD dwQuestIndex);
	bool IsQuestByEtc(DWORD dwQuestIndex);

	void SetQuestIndexByEtcList(DWORD* adwSrcQuestIndex, int nIndexCount);
	bool IsQuestIndexByEtcListEmpty();
	void SendQuestIndexByEtcSelection();
	void DelQuestIndexByEtcList(DWORD dwQuestIndex);

	void SetCurQuestIndexList(DWORD* adwCurQuestIndex, int nIndexCount);
	void AddCurQuestIndexList(DWORD dwQuestIndex);
	void RemoveCurQuestIndexList(DWORD dwQuestIndex);
	bool IsIndexInCurQuestIndexList(DWORD dwQuestIndex);

protected:
#ifdef ASG_ADD_UI_NPC_DIALOGUE
	void LoadNPCDialogueScript();
#endif	// ASG_ADD_UI_NPC_DIALOGUE
	void LoadQuestProgressScript();
	void LoadQuestWordsScript();
};

extern CQuestMng g_QuestMng;

#endif	// ASG_ADD_NEW_QUEST_SYSTEM

#endif // !defined(AFX_QUESTMNG_H__1ACBD446_412D_488D_9E0F_AC5BC22BEFE8__INCLUDED_)
