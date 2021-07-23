//*****************************************************************************
// File: QuestMng.h
//
// Desc: interface for the QuestMng class.
//		 신규 퀘스트 매니저 클래스.(09.05.20)
//
//		 이번에 추가된 신규 퀘스트 시스템은 퀘스트 인덱스 단위로 관리된다.
//		 퀘스트 인덱스는 DWORD형으로 상위 2바이트는 에피소드(EP), 하위 2바이트는 퀘스트 스테이트(QS
//		)로 구성되어 있다.
//		 퀘스트는 크게 하나의 에피소드(EP)로 구성된다. QS가 255이면 퀘스트 완료.
//		 예외로 EP값이 0이면 퀘스트가 아닌 NPC의 고유 기능으로 분류한다.(예 : 버프를 받음, 상점 기
//		능 등)
//		 퀘스트 인덱스의 변경은 서버에서 루아 스크립트로 관리하며, 해당 인덱스의 대사는 클라이언트
//		스크립트로 관리한다.(QuestProgress.txt, QuestWords(kor).txt)
//		 해당 인덱스의 '요구 사항', '조건'은 서버에서 루아 스크립트로 관리한다.
//		 위와 같이 관리하는 이유는 기존 퀘스트 시스템에선 하나의 퀘스트를 변경하기 위해선 서버와 클
//		라이언트를 수정해야하기 때문에 한쪽에서 잘못된 값을 수정하는 경우 어느쪽에서 잘못된 값을 수
//		정했는지 알아내기가 힘들다.
//
// producer: Ahn Sang-Gyu
//*****************************************************************************

#if !defined(AFX_QUESTMNG_H__1ACBD446_412D_488D_9E0F_AC5BC22BEFE8__INCLUDED_)
#define AFX_QUESTMNG_H__1ACBD446_412D_488D_9E0F_AC5BC22BEFE8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifdef ASG_ADD_NEW_QUEST_SYSTEM

#include "WSclient.h"

#ifdef ASG_ADD_UI_NPC_DIALOGUE
#define QM_MAX_ND_ANSWER	10	// NPC 대화 선택문 개수.
#endif	// ASG_ADD_UI_NPC_DIALOGUE
#define QM_MAX_ANSWER		5	// 대답 최대 개수
#define QM_MAX_REQUEST_REWARD_TEXT_LEN	64	// 요구 사항, 보상 텍스트 최대 글자수.

#ifdef ASG_ADD_UI_NPC_DIALOGUE
// NPC 대화 구조체
struct SNPCDialogue
{
	int		m_nNPCWords;			// NPC 대사 번호
	int		m_anAnswer[QM_MAX_ND_ANSWER*2];	// 선택문 및 결과값
};
#endif	// ASG_ADD_UI_NPC_DIALOGUE

// 퀘스트 대사 진행 구조체
#pragma pack(push, 1)
struct SQuestProgress
{
	BYTE	m_byUIType;				// UI 타잎(0: NPC에 의한 퀘스트 진행창, 1: 기타 상황으로 인한 퀘스트 진행창)
	int		m_nNPCWords;			// NPC 대사 번호
	int		m_nPlayerWords;			// 플래이어 대사 번호
	int		m_anAnswer[QM_MAX_ANSWER];	// 대답 번호
	int		m_nSubject;				// 제목 번호
	int		m_nSummary;				// 요약 번호
};
#pragma pack(pop)

// 퀘스트 대사 구조체
#ifndef ASG_MOD_QUEST_WORDS_SCRIPTS		// 정리시 삭제.
struct SQuestWords
{
	short	m_nAction;				// 액션 애니메이션 번호
	BYTE	m_byActCount;			// 액션 애니 횟수
	string	m_strWords;				// 대사
};
#endif	// ASG_MOD_QUEST_WORDS_SCRIPTS

// 요구사항 구조체
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

// 보상 정보 구조체
struct SQuestReward
{	
	DWORD	m_dwType;
	WORD	m_wIndex;
	DWORD	m_dwValue;
	ITEM*	m_pItem;
};

// 요구 사항, 보상 정보 구조체
#ifdef ASG_ADD_QUEST_REQUEST_REWARD_TYPE
struct SQuestRequestReward
{
	BYTE	m_byRequestCount;			// 요구 사항 개수
	BYTE	m_byGeneralRewardCount;		// 일반 보상 개수
	BYTE	m_byRandRewardCount;		// 랜덤 보상 개수
	BYTE	m_byRandGiveCount;			// 랜덤 보상 지급 개수
	SQuestRequest	m_aRequest[5];		// 요구 사항
	SQuestReward	m_aReward[5];		// 보상(일반+랜덤이 5개를 넘으면 안됨)
};
#else	// ASG_ADD_QUEST_REQUEST_REWARD_TYPE
struct SQuestRequestReward
{
	BYTE	m_byRequestCount;		// 요구 사항 개수
	BYTE	m_byRewardCount;		// 보상 개수
	SQuestRequest	m_aRequest[5];	// 요구 사항
	SQuestReward	m_aReward[5];	// 보상
};
#endif	// ASG_ADD_QUEST_REQUEST_REWARD_TYPE

enum REQUEST_REWARD_CLASSIFY
{
	RRC_NONE	= 0,
	RRC_REQUEST	= 1,
	RRC_REWARD	= 2
};

// 요구 사항, 조건 텍스트
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

using namespace std;

#ifdef ASG_ADD_UI_NPC_DIALOGUE
typedef map<DWORD, SNPCDialogue> NPCDialogueMap;
#endif	// ASG_ADD_UI_NPC_DIALOGUE
typedef map<DWORD, SQuestProgress> QuestProgressMap;
#ifdef ASG_MOD_QUEST_WORDS_SCRIPTS
typedef map<int, string> QuestWordsMap;
#else	// ASG_MOD_QUEST_WORDS_SCRIPTS
typedef map<int, SQuestWords> QuestWordsMap;
#endif	// ASG_MOD_QUEST_WORDS_SCRIPTS
typedef map<DWORD, SQuestRequestReward> QuestRequestRewardMap;
typedef list<DWORD> DWordList;

class CQuestMng  
{
protected:
#ifdef ASG_ADD_UI_NPC_DIALOGUE
	NPCDialogueMap			m_mapNPCDialogue;		// NPC 대화("NPC 인덱스 * 0x10000 + 대화 상태 번호"로 검색)
#endif	// ASG_ADD_UI_NPC_DIALOGUE
	QuestProgressMap		m_mapQuestProgress;		// 퀘스트 대사 진행(퀘스트 인덱스로 검색)
	QuestWordsMap			m_mapQuestWords;		// 퀘스트 대사(대사 번호로 검색)
	QuestRequestRewardMap	m_mapQuestRequestReward;// 퀘스트 요구 사항, 보상(퀘스트 인덱스로 검색)

	map<WORD, bool>			m_mapEPRequestRewardState;	// 퀘스트 요구 사항이 있는 QS가 진행중인지 아닌지 여부(EP 번호로 검색)
							//퀘스트 확인 버튼 누르기 전까지는 true, 누른 후 부터 false로 세팅 됨.

	int		m_nNPCIndex;			// 현재 선택된 NPC 인덱스.
	char	m_szNPCName[32];		// 현재 선택된 NPC 이름.

	DWordList				m_listQuestIndexByEtc;	// 기타 상황에 의한 퀘스트 인덱스 리스트.
	DWordList				m_listCurQuestIndex;	// 현재 진행중인 퀘스트 리스트.

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
#ifdef _BLUE_SERVER		//  블루서버에만 적용
	void DelAnswerAPDPUpBuff();
#endif // _BLUE_SERVER
#endif	// ASG_ADD_UI_NPC_DIALOGUE
	void LoadQuestProgressScript();
	void LoadQuestWordsScript();
};

extern CQuestMng g_QuestMng;

#endif	// ASG_ADD_NEW_QUEST_SYSTEM

#endif // !defined(AFX_QUESTMNG_H__1ACBD446_412D_488D_9E0F_AC5BC22BEFE8__INCLUDED_)
