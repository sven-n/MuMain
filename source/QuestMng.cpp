//*****************************************************************************
// File: CQuestMng.cpp
//
// Desc: implementation of the CQuestMng class.
//
// producer: Ahn Sang-Gyu
//*****************************************************************************

#include "stdafx.h"
#include "QuestMng.h"
#include "./Utilities/Log/ErrorReport.h"
#include "wsclientinline.h"

#include <crtdbg.h>		// _ASSERT() 사용.

#ifdef ASG_ADD_NEW_QUEST_SYSTEM

#ifdef ASG_ADD_UI_NPC_DIALOGUE
// NPC 대화 스크립트 파일명.
#ifdef USE_NPCDIALOGUETEST_BMD
#define	QM_NPCDIALOGUE_FILE			"Data\\Local\\NPCDialoguetest.bmd"
#else
#define	QM_NPCDIALOGUE_FILE			"Data\\Local\\NPCDialogue.bmd"
#endif
#endif	// ASG_ADD_UI_NPC_DIALOGUE

// 퀘스트 진행 스크립트 파일명.
#ifdef USE_QUESTPROGRESSTEST_BMD
#define	QM_QUESTPROGRESS_FILE		"Data\\Local\\QuestProgresstest.bmd"
#else
#define	QM_QUESTPROGRESS_FILE		"Data\\Local\\QuestProgress.bmd"
#endif

// 퀘스트 대사 스크립트 파일명.
#ifdef LJH_ADD_SUPPORTING_MULTI_LANGUAGE
#ifdef USE_QUESTWORDSTEST_BMD
#define	QM_QUESTWORDS_FILE			string("Data\\Local\\"+g_strSelectedML+"\\QuestWordstest_"+g_strSelectedML+".bmd").c_str()
#else
#define	QM_QUESTWORDS_FILE			string("Data\\Local\\"+g_strSelectedML+"\\QuestWords_"+g_strSelectedML+".bmd").c_str()
#endif
#else  //LJH_ADD_SUPPORTING_MULTI_LANGUAGE
#ifdef USE_QUESTWORDSTEST_BMD
#define	QM_QUESTWORDS_FILE			"Data\\Local\\QuestWordstest.bmd"
#else
#define	QM_QUESTWORDS_FILE			"Data\\Local\\QuestWords.bmd"
#endif
#endif //LJH_ADD_SUPPORTING_MULTI_LANGUAGE

CQuestMng g_QuestMng;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CQuestMng::CQuestMng()
{
	m_nNPCIndex = 0;
	::memset(m_szNPCName, 0, sizeof(char)*32);
}

CQuestMng::~CQuestMng()
{

}

//*****************************************************************************
// 함수 이름 : LoadQuestScript()
// 함수 설명 : 퀘스트 스크립트 로드.
//*****************************************************************************
void CQuestMng::LoadQuestScript()
{
#ifdef ASG_ADD_UI_NPC_DIALOGUE
	LoadNPCDialogueScript();
#endif	// ASG_ADD_UI_NPC_DIALOGUE
	LoadQuestProgressScript();
	LoadQuestWordsScript();
}

#ifdef ASG_ADD_UI_NPC_DIALOGUE
//*****************************************************************************
// 함수 이름 : LoadNPCDialogueScript()
// 함수 설명 : NPC 대화 스크립트 로드.
//*****************************************************************************
void CQuestMng::LoadNPCDialogueScript()
{
	FILE* fp = ::fopen(QM_NPCDIALOGUE_FILE, "rb");
	if (fp == NULL)
	{
		char szMessage[256];
		::sprintf(szMessage, "%s file not found.\r\n", QM_NPCDIALOGUE_FILE);
		g_ErrorReport.Write(szMessage);
		::MessageBox(g_hWnd, szMessage, NULL, MB_OK);
		::PostMessage(g_hWnd, WM_DESTROY, 0, 0);
		return;
	}

	const int nSize = sizeof(DWORD) + sizeof(SNPCDialogue);
	BYTE abyBuffer[nSize];
	DWORD dwIndex = 0;
	SNPCDialogue sNPCDialogue;

	while (0 != ::fread(abyBuffer, nSize, 1, fp))
	{
		::BuxConvert(abyBuffer, nSize);

		::memcpy(&dwIndex, abyBuffer, sizeof(DWORD));
		::memcpy(&sNPCDialogue, abyBuffer + sizeof(DWORD), sizeof(SNPCDialogue));

		m_mapNPCDialogue.insert(make_pair(dwIndex, sNPCDialogue));
	}
	
	::fclose(fp);

#ifdef _BLUE_SERVER		//  블루서버에만 적용
	DelAnswerAPDPUpBuff();
#endif // _BLUE_SERVER
}

#ifdef _BLUE_SERVER		//  블루서버에만 적용
//*****************************************************************************
// 함수 이름 : DelAnswerAPDPUpBuff()
// 함수 설명 : 선택문의 쉐도우 팬텀 효과(공방업 버프) 제거.
//			   (쉐도우 팬텀 부대원 NPC의 0번 대화 상태만 검사함)
//*****************************************************************************
void CQuestMng::DelAnswerAPDPUpBuff()
{
	const DWORD dwNPCIndex = 257;	// 쉐도우 팬텀 부대원 NPC 인덱스.
	const DWORD dwDlgState = 0;		// 0번 대화 상태.
	const DWORD dwNPCDlgIndex = dwNPCIndex * 0x10000 + dwDlgState;

	NPCDialogueMap::iterator iter = m_mapNPCDialogue.find(dwNPCDlgIndex);
	if (iter == m_mapNPCDialogue.end())
		return;

	bool bLShift = false;
	int i;
	for (i = 0; i < QM_MAX_ND_ANSWER; ++i)
	{
		if (bLShift)
		{
			iter->second.m_anAnswer[(i-1)*2] = iter->second.m_anAnswer[i*2];
			iter->second.m_anAnswer[(i-1)*2+1] = iter->second.m_anAnswer[i*2+1];
		}
		else if (902 == iter->second.m_anAnswer[i*2+1])	// 선택문 결과가 공방업(902)인가?
			bLShift = true;		// 다음 선택문 부터 이전 선택문으로 카피.
	}

	if (bLShift)
	{	// 마지막 선택문은 0으로 초기화.
		iter->second.m_anAnswer[(i-1)*2] = 0;
		iter->second.m_anAnswer[(i-1)*2+1] = 0;
	}
}
#endif // _BLUE_SERVER
#endif	// ASG_ADD_UI_NPC_DIALOGUE

//*****************************************************************************
// 함수 이름 : LoadQuestProgressScript()
// 함수 설명 : 퀘스트 대사 진행 스크립트 로드.
//*****************************************************************************
void CQuestMng::LoadQuestProgressScript()
{
	FILE* fp = ::fopen(QM_QUESTPROGRESS_FILE, "rb");
	if (fp == NULL)
	{
		char szMessage[256];
		::sprintf(szMessage, "%s file not found.\r\n", QM_QUESTPROGRESS_FILE);
		g_ErrorReport.Write(szMessage);
		::MessageBox(g_hWnd, szMessage, NULL, MB_OK);
		::PostMessage(g_hWnd, WM_DESTROY, 0, 0);
		return;
	}

	const int nSize = sizeof(DWORD) + sizeof(SQuestProgress);
	BYTE abyBuffer[nSize];
	DWORD dwIndex = 0;
	SQuestProgress sQuestProgress;

	while (0 != ::fread(abyBuffer, nSize, 1, fp))
	{
		::BuxConvert(abyBuffer, nSize);

		::memcpy(&dwIndex, abyBuffer, sizeof(DWORD));
		::memcpy(&sQuestProgress, abyBuffer + sizeof(DWORD), sizeof(SQuestProgress));

		m_mapQuestProgress.insert(make_pair(dwIndex, sQuestProgress));
	}
	
	::fclose(fp);
}

//*****************************************************************************
// 함수 이름 : LoadQuestWordsScript()
// 함수 설명 : 퀘스트 대사 스크립트 로드.
//*****************************************************************************
void CQuestMng::LoadQuestWordsScript()
{
	FILE* fp = ::fopen(QM_QUESTWORDS_FILE, "rb");
	if (fp == NULL)
	{
		char szMessage[256];
		::sprintf(szMessage, "%s file not found.\r\n", QM_QUESTWORDS_FILE);
		g_ErrorReport.Write(szMessage);
		::MessageBox(g_hWnd, szMessage, NULL, MB_OK);
		::PostMessage(g_hWnd, WM_DESTROY, 0, 0);
		return;
	}

#pragma pack(push, 1)
	struct SQuestWordsHeader
	{
		int		m_nIndex;				// 대사 인덱스
#ifndef ASG_MOD_QUEST_WORDS_SCRIPTS	// 정리시 삭제.
		short	m_nAction;				// 액션 애니메이션 번호
		BYTE	m_byActCount;			// 액션 애니 횟수
#endif	// ASG_MOD_QUEST_WORDS_SCRIPTS
		short	m_nWordsLen;			// 대사 글자 길이
	};
#pragma pack(pop)

	int nSize = sizeof(SQuestWordsHeader);
	SQuestWordsHeader sQuestWordsHeader;
	char szWords[1024];
#ifdef ASG_MOD_QUEST_WORDS_SCRIPTS
	string	strWords;
#else	// ASG_MOD_QUEST_WORDS_SCRIPTS
	SQuestWords sQuestWords;
#endif	// ASG_MOD_QUEST_WORDS_SCRIPTS
	
	while (0 != ::fread(&sQuestWordsHeader, nSize, 1, fp))
	{
		::BuxConvert((BYTE*)&sQuestWordsHeader, nSize);

		::fread(szWords, sQuestWordsHeader.m_nWordsLen, 1, fp);
		::BuxConvert((BYTE*)szWords, sQuestWordsHeader.m_nWordsLen);
#ifdef ASG_MOD_QUEST_WORDS_SCRIPTS
		strWords = szWords;

		m_mapQuestWords.insert(make_pair(sQuestWordsHeader.m_nIndex, strWords));
#else	// ASG_MOD_QUEST_WORDS_SCRIPTS
		sQuestWords.m_nAction = sQuestWordsHeader.m_nAction;
		sQuestWords.m_byActCount = sQuestWordsHeader.m_byActCount;
		sQuestWords.m_strWords = szWords;

		m_mapQuestWords.insert(make_pair(sQuestWordsHeader.m_nIndex, sQuestWords));
#endif	// ASG_MOD_QUEST_WORDS_SCRIPTS
	}
	
	::fclose(fp);
}

//*****************************************************************************
// 함수 이름 : SetQuestRequestReward()
// 함수 설명 : 요구, 보상 정보 저장.
// 매개 변수 : pRequestRewardPacket	: 요구, 보상 정보.
//*****************************************************************************
void CQuestMng::SetQuestRequestReward(BYTE* pbyRequestRewardPacket)
{
	LPPMSG_NPC_QUESTEXP_INFO pRequestRewardPacket
		= (LPPMSG_NPC_QUESTEXP_INFO)pbyRequestRewardPacket;
	DWORD dwQuestIndex = pRequestRewardPacket->m_dwQuestIndex;
	int i;

// 기존 인덱스에 요구사항, 보상 정보가 있으면 관련 아이템 정보를 삭제한다.
	const SQuestRequestReward* pOldRequestReward = GetRequestReward(dwQuestIndex);
	if (pOldRequestReward)
	{
		for (i = 0; i < pOldRequestReward->m_byRequestCount; ++i)
			g_pNewItemMng->DeleteItem(pOldRequestReward->m_aRequest[i].m_pItem);
#ifdef ASG_ADD_QUEST_REQUEST_REWARD_TYPE
		BYTE byRewardCount
			= pOldRequestReward->m_byGeneralRewardCount + pOldRequestReward->m_byRandRewardCount;
		for (i = 0; i < byRewardCount; ++i)
#else	// ASG_ADD_QUEST_REQUEST_REWARD_TYPE
		for (i = 0; i < pOldRequestReward->m_byRewardCount; ++i)
#endif	// ASG_ADD_QUEST_REQUEST_REWARD_TYPE
			g_pNewItemMng->DeleteItem(pOldRequestReward->m_aReward[i].m_pItem);
	}

// 새로운 요구사항, 보상 정보 세팅.
	SQuestRequestReward sRequestReward;
	::memset(&sRequestReward, 0, sizeof(SQuestRequestReward));

// 요구 사항.
	LPNPC_QUESTEXP_REQUEST_INFO pRequestPacket
		= (LPNPC_QUESTEXP_REQUEST_INFO)(pbyRequestRewardPacket + sizeof(PMSG_NPC_QUESTEXP_INFO));

	// '없음'이라고 표시해야하기 때문에 카운트는 1.
	if (pRequestPacket->m_dwType == QUEST_REQUEST_NONE
		|| pRequestRewardPacket->m_byRequestCount == 0)
	{
		sRequestReward.m_byRequestCount = 1;
	}
	else
	{
		sRequestReward.m_byRequestCount = pRequestRewardPacket->m_byRequestCount;
		for (i = 0; i < sRequestReward.m_byRequestCount; ++i)
		{
			sRequestReward.m_aRequest[i].m_dwType = pRequestPacket->m_dwType;
			sRequestReward.m_aRequest[i].m_wIndex = pRequestPacket->m_wIndex;
			sRequestReward.m_aRequest[i].m_dwValue = pRequestPacket->m_dwValue;
#ifdef ASG_ADD_TIME_LIMIT_QUEST
			sRequestReward.m_aRequest[i].m_dwCurValue = pRequestPacket->m_dwCurValue;
#else	// ASG_ADD_TIME_LIMIT_QUEST
			sRequestReward.m_aRequest[i].m_wCurValue = pRequestPacket->m_wCurValue;
#endif	// ASG_ADD_TIME_LIMIT_QUEST
			if (pRequestPacket->m_dwType == QUEST_REQUEST_ITEM)
				sRequestReward.m_aRequest[i].m_pItem
				= g_pNewItemMng->CreateItem(pRequestPacket->m_byItemInfo);
			++pRequestPacket;
		}
	}

// 보상.
	LPNPC_QUESTEXP_REWARD_INFO pRewardPacket
		= (LPNPC_QUESTEXP_REWARD_INFO)(pbyRequestRewardPacket + sizeof(PMSG_NPC_QUESTEXP_INFO)
			+ sizeof(NPC_QUESTEXP_REQUEST_INFO) * 5);

	// '없음'이라고 표시해야하기 때문에 카운트는 1.
	if (pRewardPacket->m_dwType == QUEST_REWARD_NONE
		|| pRequestRewardPacket->m_byRewardCount == 0)
	{
#ifdef ASG_ADD_QUEST_REQUEST_REWARD_TYPE
		sRequestReward.m_byGeneralRewardCount = 1;
#else	// ASG_ADD_QUEST_REQUEST_REWARD_TYPE
		sRequestReward.m_byRewardCount = 1;
#endif	// ASG_ADD_QUEST_REQUEST_REWARD_TYPE
	}
	else
	{
#ifdef ASG_ADD_QUEST_REQUEST_REWARD_TYPE
		sRequestReward.m_byRandGiveCount = pRequestRewardPacket->m_byRandRewardCount;

	// 일반 보상과 랜덤 보상을 분류.
		BYTE byGeneralCount = 0;
		BYTE byRandCount = 0;
		SQuestReward aTempRandReward[5];
		::memset(aTempRandReward, 0, sizeof(SQuestReward) * 5);

		for (i = 0; i < pRequestRewardPacket->m_byRewardCount; ++i)
		{
			if (QUEST_REWARD_TYPE(pRewardPacket->m_dwType & 0xFFE0) == QUEST_REWARD_RANDOM)	// 랜덤 보상인가?
			{
			// 랜덤보상이면 임시 기억장소에 저장.
				aTempRandReward[byRandCount].m_dwType = pRewardPacket->m_dwType & 0x1F;	// QUEST_REWARD_RANDOM은 자름.
				aTempRandReward[byRandCount].m_wIndex = pRewardPacket->m_wIndex;
				aTempRandReward[byRandCount].m_dwValue = pRewardPacket->m_dwValue;
				if (aTempRandReward[byRandCount].m_dwType == QUEST_REWARD_ITEM)
					aTempRandReward[byRandCount].m_pItem
						= g_pNewItemMng->CreateItem(pRewardPacket->m_byItemInfo);
				++byRandCount;
			}
			else
			{
			// 일반보상일 경우 sRequestReward.m_aReward에 먼저 채워 넣는다.
				sRequestReward.m_aReward[byGeneralCount].m_dwType = pRewardPacket->m_dwType;
				sRequestReward.m_aReward[byGeneralCount].m_wIndex = pRewardPacket->m_wIndex;
				sRequestReward.m_aReward[byGeneralCount].m_dwValue = pRewardPacket->m_dwValue;
				if (pRewardPacket->m_dwType == QUEST_REWARD_ITEM)
					sRequestReward.m_aReward[byGeneralCount].m_pItem
						= g_pNewItemMng->CreateItem(pRewardPacket->m_byItemInfo);
				++byGeneralCount;
			}

			++pRewardPacket;
		}

		sRequestReward.m_byGeneralRewardCount = byGeneralCount;
		sRequestReward.m_byRandRewardCount = byRandCount;

	// 랜덤 보상을 sRequestReward.m_aReward에 채워 넣는다.
		for (i = 0; i < sRequestReward.m_byRandRewardCount; ++i)
			sRequestReward.m_aReward[byGeneralCount++] = aTempRandReward[i];
#else	// ASG_ADD_QUEST_REQUEST_REWARD_TYPE
		sRequestReward.m_byRewardCount = pRequestRewardPacket->m_byRewardCount;
		for (i = 0; i < sRequestReward.m_byRewardCount; ++i)
		{
			sRequestReward.m_aReward[i].m_dwType = pRewardPacket->m_dwType;
			sRequestReward.m_aReward[i].m_wIndex = pRewardPacket->m_wIndex;
			sRequestReward.m_aReward[i].m_dwValue = pRewardPacket->m_dwValue;
			if (pRewardPacket->m_dwType == QUEST_REWARD_ITEM)
				sRequestReward.m_aReward[i].m_pItem
				= g_pNewItemMng->CreateItem(pRewardPacket->m_byItemInfo);
			++pRewardPacket;
		}
#endif	// ASG_ADD_QUEST_REQUEST_REWARD_TYPE
	}

	m_mapQuestRequestReward[dwQuestIndex] = sRequestReward;
}

//*****************************************************************************
// 함수 이름 : GetRequestReward()
// 함수 설명 : 퀘스트 인덱스의 요구사항, 보상 정보 얻기.
// 매개 변수 : dwQuestIndex	: 퀘스트 인덱스.
// 반환 값	 : 퀘스트 인덱스의 요구사항, 보상 정보.
//*****************************************************************************
const SQuestRequestReward* CQuestMng::GetRequestReward(DWORD dwQuestIndex)
{
	QuestRequestRewardMap::const_iterator iter = m_mapQuestRequestReward.find(dwQuestIndex);
	if (iter == m_mapQuestRequestReward.end())
		return NULL;

	return &(iter->second);
}

//*****************************************************************************
// 함수 이름 : SetNPC()
// 함수 설명 : NPC 세팅.
// 매개 변수 : nNPCIndex	: NPC 인덱스.
//*****************************************************************************
void CQuestMng::SetNPC(int nNPCIndex)
{
	m_nNPCIndex = nNPCIndex;
	::strcpy(m_szNPCName, ::getMonsterName(nNPCIndex));
}

//*****************************************************************************
// 함수 이름 : GetNPCIndex()
// 함수 설명 : NPC 인덱스 얻기.
// 반환 값	 : NPC 인덱스.
//*****************************************************************************
int CQuestMng::GetNPCIndex()
{
	return m_nNPCIndex;
}

//*****************************************************************************
// 함수 이름 : GetNPCName()
// 함수 설명 : NPC 이름 얻기.
// 반환 값	 : NPC 이름.
//*****************************************************************************
char* CQuestMng::GetNPCName()
{
	return m_szNPCName;
}

//*****************************************************************************
// 함수 이름 : SetCurQuestProgress()
// 함수 설명 : 상항에 맞는 퀘스트 진행창 세팅.
// 매개 변수 : dwQuestIndex	: 퀘스트 인덱스.
//*****************************************************************************
void CQuestMng::SetCurQuestProgress(DWORD dwQuestIndex)
{
	AddCurQuestIndexList(dwQuestIndex);

// QS값(하위 2바이트)이 255(0x00FF)면 퀘스트 종료.
	if (LOWORD(dwQuestIndex) == 0x00FF)
	{
#ifdef ASG_ADD_UI_QUEST_PROGRESS
		if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_QUEST_PROGRESS))
			g_pNewUISystem->Hide(SEASON3B::INTERFACE_QUEST_PROGRESS);
#endif	// ASG_ADD_UI_QUEST_PROGRESS
#ifdef ASG_ADD_UI_QUEST_PROGRESS_ETC
		if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_QUEST_PROGRESS_ETC))
			g_pNewUISystem->Hide(SEASON3B::INTERFACE_QUEST_PROGRESS_ETC);
#endif	// ASG_ADD_UI_QUEST_PROGRESS_ETC

		g_pChatListBox->AddText("", GlobalText[2814], SEASON3B::TYPE_ERROR_MESSAGE);	// 2814 "퀘스트를 성공적으로 완료하였습니다"

		return;
	}

	// NPC용 퀘스트 진행창.
	if (0 == m_mapQuestProgress[dwQuestIndex].m_byUIType)
	{
#ifdef ASG_ADD_UI_QUEST_PROGRESS
		g_pQuestProgress->SetContents(dwQuestIndex);
		if (!g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_QUEST_PROGRESS))
			g_pNewUISystem->Show(SEASON3B::INTERFACE_QUEST_PROGRESS);
#endif	// ASG_ADD_UI_QUEST_PROGRESS
	}
	// 아이템, 기타용 퀘스트 진행 창.
	else
	{
#ifdef ASG_ADD_UI_QUEST_PROGRESS_ETC
		g_pQuestProgressByEtc->SetContents(dwQuestIndex);
		if (!g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_QUEST_PROGRESS_ETC))
			g_pNewUISystem->Show(SEASON3B::INTERFACE_QUEST_PROGRESS_ETC);
#endif	// ASG_ADD_UI_QUEST_PROGRESS_ETC
	}
}

//*****************************************************************************
// 함수 이름 : GetWords()
// 함수 설명 : 퀘스트 대사 얻기.
// 매개 변수 : nWordsIndex	: 대사 인덱스.
// 반환 값	 : 퀘스트 대사.
//*****************************************************************************
const char* CQuestMng::GetWords(int nWordsIndex)
{
	QuestWordsMap::const_iterator iter = m_mapQuestWords.find(nWordsIndex);
	if (iter == m_mapQuestWords.end())
		return NULL;

#ifdef ASG_MOD_QUEST_WORDS_SCRIPTS
	return iter->second.c_str();
#else	// ASG_MOD_QUEST_WORDS_SCRIPTS
	return iter->second.m_strWords.c_str();
#endif	// ASG_MOD_QUEST_WORDS_SCRIPTS
}

#ifdef ASG_ADD_UI_NPC_DIALOGUE
//*****************************************************************************
// 함수 이름 : GetNPCDlgNPCWords()
// 함수 설명 : NPC 대화의 NPC 대사 얻기.
// 매개 변수 : dwDlgState	: 대화 상태 번호.
// 반환 값	 : NPC 대사.
//*****************************************************************************
const char* CQuestMng::GetNPCDlgNPCWords(DWORD dwDlgState)
{
	if (0 == m_nNPCIndex)
		return NULL;

	DWORD dwNPCDlgIndex = (DWORD)m_nNPCIndex * 0x10000 + dwDlgState;

	NPCDialogueMap::const_iterator iter = m_mapNPCDialogue.find(dwNPCDlgIndex);
	if (iter == m_mapNPCDialogue.end())
		return NULL;

	return GetWords(iter->second.m_nNPCWords);
}

//*****************************************************************************
// 함수 이름 : GetNPCDlgAnswer()
// 함수 설명 : NPC 대화의 선택문 얻기.
// 매개 변수 : dwDlgState	: 대화 상태 번호.
//			   nAnswer		: 선택문번호 (0 ~ 9).
// 반환 값	 : 선택문.
//*****************************************************************************
const char* CQuestMng::GetNPCDlgAnswer(DWORD dwDlgState, int nAnswer)
{
	if (0 == m_nNPCIndex)
		return NULL;

	_ASSERT(0 <= nAnswer || nAnswer < QM_MAX_ND_ANSWER);

	DWORD dwNPCDlgIndex = (DWORD)m_nNPCIndex * 0x10000 + dwDlgState;

	NPCDialogueMap::const_iterator iter = m_mapNPCDialogue.find(dwNPCDlgIndex);
	if (iter == m_mapNPCDialogue.end())
		return NULL;

	DWORD nNowAnswer = iter->second.m_anAnswer[nAnswer*2];
	if (0 == nNowAnswer)
		return NULL;

	return GetWords(nNowAnswer);
}

//*****************************************************************************
// 함수 이름 : GetNPCDlgAnswerResult()
// 함수 설명 : NPC 대화의 선택문 결과 값 얻기.
// 매개 변수 : dwDlgState	: 대화 상태 번호.
//			   nAnswer		: 선택문번호 (0 ~ 9).
// 반환 값	 : 선택문 결과 값. -1이면 선택문이 없음.
//*****************************************************************************
int CQuestMng::GetNPCDlgAnswerResult(DWORD dwDlgState, int nAnswer)
{
	if (0 == m_nNPCIndex)
		return NULL;

	_ASSERT(0 <= nAnswer || nAnswer < QM_MAX_ND_ANSWER);

	DWORD dwNPCDlgIndex = (DWORD)m_nNPCIndex * 0x10000 + dwDlgState;

	NPCDialogueMap::const_iterator iter = m_mapNPCDialogue.find(dwNPCDlgIndex);
	if (iter == m_mapNPCDialogue.end())
		return -1;

	DWORD nNowAnswer = iter->second.m_anAnswer[nAnswer*2];
	if (0 == nNowAnswer)
		return -1;

	return iter->second.m_anAnswer[nAnswer*2+1];
}
#endif	// ASG_ADD_UI_NPC_DIALOGUE

//*****************************************************************************
// 함수 이름 : GetNPCWords()
// 함수 설명 : 퀘스트 진행의 NPC 대사 얻기.
// 매개 변수 : dwQuestIndex	: 퀘스트 인덱스.
// 반환 값	 : NPC 대사.
//*****************************************************************************
const char* CQuestMng::GetNPCWords(DWORD dwQuestIndex)
{
	QuestProgressMap::const_iterator iter = m_mapQuestProgress.find(dwQuestIndex);
	if (iter == m_mapQuestProgress.end())
		return NULL;

	return GetWords(iter->second.m_nNPCWords);
}

//*****************************************************************************
// 함수 이름 : GetPlayerWords()
// 함수 설명 : 퀘스트 진행의 플래이어 대사 얻기.
// 매개 변수 : dwQuestIndex	: 퀘스트 인덱스.
// 반환 값	 : 플래이어 대사.
//*****************************************************************************
const char* CQuestMng::GetPlayerWords(DWORD dwQuestIndex)
{
	QuestProgressMap::const_iterator iter = m_mapQuestProgress.find(dwQuestIndex);
	if (iter == m_mapQuestProgress.end())
		return NULL;

	return GetWords(iter->second.m_nPlayerWords);
}

//*****************************************************************************
// 함수 이름 : GetAnswer()
// 함수 설명 : 퀘스트 진행의 선택문 얻기.
// 매개 변수 : dwQuestIndex	: 퀘스트 인덱스.
//			   nAnswer		: 선택문번호 (0 ~ 4).
// 반환 값	 : 선택문.
//*****************************************************************************
const char* CQuestMng::GetAnswer(DWORD dwQuestIndex, int nAnswer)
{
	_ASSERT(0 <= nAnswer || nAnswer < QM_MAX_ANSWER);

	QuestProgressMap::const_iterator iter = m_mapQuestProgress.find(dwQuestIndex);
	if (iter == m_mapQuestProgress.end())
		return NULL;

	DWORD nNowAnswer = iter->second.m_anAnswer[nAnswer];
	if (0 == nNowAnswer)
		return NULL;

	return GetWords(nNowAnswer);
}

//*****************************************************************************
// 함수 이름 : GetSubject()
// 함수 설명 : 퀘스트 진행의 퀘스트 인덱스의 제목 얻기.
// 반환 값	 : 퀘스트 인덱스의 제목.
//*****************************************************************************
const char* CQuestMng::GetSubject(DWORD dwQuestIndex)
{
	QuestProgressMap::const_iterator iter = m_mapQuestProgress.find(dwQuestIndex);
	if (iter == m_mapQuestProgress.end())
		return NULL;

	return GetWords(iter->second.m_nSubject);
}

//*****************************************************************************
// 함수 이름 : GetSummary()
// 함수 설명 : 퀘스트 진행의 퀘스트 인덱스의 요약 얻기.
// 반환 값	 : 퀘스트 인덱스의 요약.
//*****************************************************************************
const char* CQuestMng::GetSummary(DWORD dwQuestIndex)
{
	QuestProgressMap::const_iterator iter = m_mapQuestProgress.find(dwQuestIndex);
	if (iter == m_mapQuestProgress.end())
		return NULL;

	return GetWords(iter->second.m_nSummary);
}

//*****************************************************************************
// 함수 이름 : IsRequestRewardQS()
// 함수 설명 : 요구사항, 보상 정보가 있는 QS(Quest State)인가?
// 매개 변수 : dwQuestIndex	: 퀘스트 인덱스.
// 반환 값	 : 요구사항, 보상 정보가 있으면 true.
//*****************************************************************************
bool CQuestMng::IsRequestRewardQS(DWORD dwQuestIndex)
{
	QuestProgressMap::const_iterator iter = m_mapQuestProgress.find(dwQuestIndex);
	_ASSERT(iter != m_mapQuestProgress.end());	// 퀘스트 정보가 존재하여야함.

	// 선택문 0번째 값이 0이면 요구사항, 보상 정보가 있는 QS.
	if (0 == iter->second.m_anAnswer[0])
		return true;
	else
		return false;	
}

//*****************************************************************************
// 함수 이름 : GetRequestRewardText()
// 함수 설명 : 요구사항, 보상 텍스트를 얻음.
// 매개 변수 : aDest		: SRequestRewardText구조체 배열 이름.
//			   nDestCount	: SRequestRewardText구조체 배열 개수.
//			   dwQuestIndex	: 퀘스트 인덱스.
// 반환 값	 : 요구사항 만족 여부.
//*****************************************************************************
bool CQuestMng::GetRequestRewardText(SRequestRewardText* aDest, int nDestCount, DWORD dwQuestIndex)
{
	SQuestRequestReward* pRequestReward = &m_mapQuestRequestReward[dwQuestIndex];

	// GlobalText[2809] "요구 사항", GlobalText[2810] "보     상",
	//GlobalText[3082] "랜덤 보상(%lu가지 지급)"을 출력하기 위해 3줄이 더 필요.
#ifdef ASG_ADD_QUEST_REQUEST_REWARD_TYPE
	if (pRequestReward->m_byRequestCount + pRequestReward->m_byGeneralRewardCount
		+ pRequestReward->m_byRandRewardCount + 3 > nDestCount)
#else	// ASG_ADD_QUEST_REQUEST_REWARD_TYPE
	if (pRequestReward->m_byRequestCount + pRequestReward->m_byRewardCount + 2 > nDestCount)
#endif	// ASG_ADD_QUEST_REQUEST_REWARD_TYPE
		return false;

	::memset(aDest, 0, sizeof(SRequestRewardText) * nDestCount);

	int nLine = 0;
	bool bRequestComplete = true;
	int i;

// 요구 사항.
	aDest[nLine].m_hFont = g_hFontBold;
	aDest[nLine].m_dwColor = ARGB(255, 179, 230, 77);
	::strcpy(aDest[nLine++].m_szText, GlobalText[2809]);	// 2809 "요구 사항"

	SQuestRequest* pRequestInfo;
	for (i = 0; i < pRequestReward->m_byRequestCount; ++i, ++nLine)
	{
		pRequestInfo = &pRequestReward->m_aRequest[i];

		aDest[nLine].m_hFont = g_hFont;
		aDest[nLine].m_eRequestReward = RRC_REQUEST;
		aDest[nLine].m_dwType = pRequestInfo->m_dwType;
		aDest[nLine].m_wIndex = pRequestInfo->m_wIndex;
		aDest[nLine].m_pItem = pRequestInfo->m_pItem;

		switch (pRequestInfo->m_dwType)
		{
		case QUEST_REQUEST_NONE:
			aDest[nLine].m_dwColor = ARGB(255, 223, 191, 103);
			::strcpy(aDest[nLine].m_szText, GlobalText[1361]);	// 1361 "없음"
			break;

#ifdef ASG_ADD_TIME_LIMIT_QUEST
		case QUEST_REQUEST_MONSTER:
		case QUEST_REQUEST_ITEM:
		case QUEST_REQUEST_LEVEL:
		case QUEST_REQUEST_ZEN:
		case QUEST_REQUEST_PVP_POINT:
			if (pRequestInfo->m_dwCurValue < pRequestInfo->m_dwValue)
			{
				aDest[nLine].m_dwColor = ARGB(255, 255, 30, 30);
				bRequestComplete = false;
			}
			else
				aDest[nLine].m_dwColor = ARGB(255, 223, 191, 103);
			
			switch (pRequestInfo->m_dwType)
			{
			case QUEST_REQUEST_MONSTER:
				::sprintf(aDest[nLine].m_szText, "Mon.: %s x %lu/%lu",
					::getMonsterName(int(pRequestInfo->m_wIndex)),
					MIN(pRequestInfo->m_dwCurValue, pRequestInfo->m_dwValue),
					pRequestInfo->m_dwValue);
				break;
			case QUEST_REQUEST_ITEM:
				{
					char szItemName[32];
					::GetItemName((int)pRequestInfo->m_pItem->Type,
						(pRequestInfo->m_pItem->Level>>3)&15, szItemName);
					::sprintf(aDest[nLine].m_szText, "Item: %s x %lu/%lu", szItemName,
						MIN(pRequestInfo->m_dwCurValue, pRequestInfo->m_dwValue),
						pRequestInfo->m_dwValue);
				}
				break;
			case QUEST_REQUEST_LEVEL:
				::sprintf(aDest[nLine].m_szText, "Level: %lu %s",
					pRequestInfo->m_dwValue, GlobalText[2812]);	// 2812 "이상"
				break;
			case QUEST_REQUEST_ZEN:
				::sprintf(aDest[nLine].m_szText, "Zen : %lu", pRequestInfo->m_dwValue);
				break;
			case QUEST_REQUEST_PVP_POINT:
				::sprintf(aDest[nLine].m_szText, GlobalText[3278],	// 3278	"적 겐스원 x %lu/%lu"
					MIN(pRequestInfo->m_dwCurValue, pRequestInfo->m_dwValue),
					pRequestInfo->m_dwValue);
				break;
			}
			break;
#endif	// ASG_ADD_TIME_LIMIT_QUEST

#ifndef ASG_ADD_TIME_LIMIT_QUEST	// 정리시 삭제.
		case QUEST_REQUEST_MONSTER:
			if ((DWORD)pRequestInfo->m_wCurValue < pRequestInfo->m_dwValue)
			{
				aDest[nLine].m_dwColor = ARGB(255, 255, 30, 30);
				bRequestComplete = false;
			}
			else
				aDest[nLine].m_dwColor = ARGB(255, 223, 191, 103);

			::sprintf(aDest[nLine].m_szText, "Mon.: %s x %lu/%lu",
				::getMonsterName(int(pRequestInfo->m_wIndex)),
				MIN((DWORD)pRequestInfo->m_wCurValue, pRequestInfo->m_dwValue),
				pRequestInfo->m_dwValue);
			break;
#endif	// ASG_ADD_TIME_LIMIT_QUEST	// 정리시 삭제.

		case QUEST_REQUEST_SKILL:
#ifdef ASG_ADD_TIME_LIMIT_QUEST
			if (0 == pRequestInfo->m_dwCurValue)
#else	// ASG_ADD_TIME_LIMIT_QUEST
			if (0 == pRequestInfo->m_wCurValue)
#endif	// ASG_ADD_TIME_LIMIT_QUEST
			{
				aDest[nLine].m_dwColor = ARGB(255, 255, 30, 30);
				bRequestComplete = false;
			}
			else
				aDest[nLine].m_dwColor = ARGB(255, 223, 191, 103);

			::sprintf(aDest[nLine].m_szText, "Skill: %s",
				SkillAttribute[pRequestInfo->m_wIndex].Name);
			break;

#ifndef ASG_ADD_TIME_LIMIT_QUEST	// 정리시 삭제.
		case QUEST_REQUEST_ITEM:
			if ((DWORD)pRequestInfo->m_wCurValue < pRequestInfo->m_dwValue)
			{
				aDest[nLine].m_dwColor = ARGB(255, 255, 30, 30);
				bRequestComplete = false;
			}
			else
				aDest[nLine].m_dwColor = ARGB(255, 223, 191, 103);

			char szItemName[32];
			::GetItemName((int)pRequestInfo->m_pItem->Type, (pRequestInfo->m_pItem->Level>>3)&15,
				szItemName);
			::sprintf(aDest[nLine].m_szText, "Item: %s x %lu/%lu", szItemName,
				MIN((DWORD)pRequestInfo->m_wCurValue, pRequestInfo->m_dwValue),
				pRequestInfo->m_dwValue);
			break;

		case QUEST_REQUEST_LEVEL:
			if ((DWORD)pRequestInfo->m_wCurValue < pRequestInfo->m_dwValue)
			{
				aDest[nLine].m_dwColor = ARGB(255, 255, 30, 30);
				bRequestComplete = false;
			}
			else
				aDest[nLine].m_dwColor = ARGB(255, 223, 191, 103);

			::sprintf(aDest[nLine].m_szText, "Level: %lu %s",
				pRequestInfo->m_dwValue, GlobalText[2812]);	// 2812 "이상"
			break;
#endif	// ASG_ADD_TIME_LIMIT_QUEST	// 정리시 삭제.

		case QUEST_REQUEST_TUTORIAL:
#ifdef ASG_ADD_TIME_LIMIT_QUEST
			if (pRequestInfo->m_dwCurValue == 1)
#else	// ASG_ADD_TIME_LIMIT_QUEST
			if (pRequestInfo->m_wCurValue == 1)
#endif	// ASG_ADD_TIME_LIMIT_QUEST
				aDest[nLine].m_dwColor = ARGB(255, 223, 191, 103);
			else
			{
				aDest[nLine].m_dwColor = ARGB(255, 255, 30, 30);
				bRequestComplete = false;
			}
			
			switch (dwQuestIndex)
			{
			case 0x10009:	// 캐릭터창 열기.
				::sprintf(aDest[nLine].m_szText, "%s", GlobalText[2819]);	// 2819 "캐릭터 창(C) 열기"
				break;
			case 0x1000F:	// 장비창 열기.
				::sprintf(aDest[nLine].m_szText, "%s", GlobalText[2820]);	// 2820 "장비 창(I, V) 열기"
				break;
			}
			break;

		case QUEST_REQUEST_BUFF:
#ifdef ASG_ADD_QUEST_REQUEST_REWARD_TYPE
			{
#endif	// ASG_ADD_QUEST_REQUEST_REWARD_TYPE
#ifdef ASG_ADD_TIME_LIMIT_QUEST
				if (pRequestInfo->m_dwCurValue == 0)
#else	// ASG_ADD_TIME_LIMIT_QUEST
				if (pRequestInfo->m_wCurValue == 0)
#endif	// ASG_ADD_TIME_LIMIT_QUEST
				{
					aDest[nLine].m_dwColor = ARGB(255, 255, 30, 30);
					bRequestComplete = false;
				}
				else
					aDest[nLine].m_dwColor = ARGB(255, 223, 191, 103);

				const BuffInfo buffinfo = g_BuffInfo((eBuffState)pRequestInfo->m_wIndex);
				::sprintf(aDest[nLine].m_szText, "Bonus: %s", buffinfo.s_BuffName);
#ifdef ASG_ADD_QUEST_REQUEST_REWARD_TYPE
			}
#endif	// ASG_ADD_QUEST_REQUEST_REWARD_TYPE
			break;

#ifdef ASG_ADD_QUEST_REQUEST_NPC_SEARCH
		case QUEST_REQUEST_NPC_TALK:	//pRequestInfo->m_wIndex는 NPC 인덱스로 쓰이지만 클라에서는 사용 안함.
			if (pRequestInfo->m_dwCurValue == 0)
			{
				aDest[nLine].m_dwColor = ARGB(255, 255, 30, 30);
				bRequestComplete = false;
			}
			else
				aDest[nLine].m_dwColor = ARGB(255, 223, 191, 103);

			::sprintf(aDest[nLine].m_szText, "%s", GlobalText[3249]);	// 3249	"의뢰인 찾기"
			break;
#endif	// ASG_ADD_QUEST_REQUEST_NPC_SEARCH

#ifdef ASG_ADD_QUEST_REQUEST_REWARD_TYPE
		case QUEST_REQUEST_EVENT_MAP_MON_KILL:
		case QUEST_REQUEST_EVENT_MAP_BLOOD_GATE:
		case QUEST_REQUEST_EVENT_MAP_USER_KILL:
		case QUEST_REQUEST_EVENT_MAP_DEVIL_POINT:
			{
#ifdef ASG_ADD_TIME_LIMIT_QUEST
				if (pRequestInfo->m_dwCurValue < pRequestInfo->m_dwValue)
#else	// ASG_ADD_TIME_LIMIT_QUEST
				if ((DWORD)pRequestInfo->m_wCurValue < pRequestInfo->m_dwValue)
#endif	// ASG_ADD_TIME_LIMIT_QUEST
				{
					aDest[nLine].m_dwColor = ARGB(255, 255, 30, 30);
					bRequestComplete = false;
				}
				else
					aDest[nLine].m_dwColor = ARGB(255, 223, 191, 103);

				int nTextIndex = 0;
				switch (pRequestInfo->m_dwType)
				{
				case QUEST_REQUEST_EVENT_MAP_MON_KILL:
					nTextIndex = 3074;	// 3074	"카오스캐슬 Lv.%lu 근위병 x %lu/%lu"
					break;
				case QUEST_REQUEST_EVENT_MAP_BLOOD_GATE:
					nTextIndex = 3077;	// 3077	"블러드캐슬 Lv.%lu 성문 파괴 x %lu/%lu"
					break;
				case QUEST_REQUEST_EVENT_MAP_USER_KILL:
					nTextIndex = 3075;	// 3075	"카오스캐슬 Lv.%lu 플레이어 x %lu/%lu"
					break;
				case QUEST_REQUEST_EVENT_MAP_DEVIL_POINT:
					nTextIndex = 3079;	// 3079	"악마의광장 Lv.%lu Point x %lu/%lu"
					break;
				}
				::sprintf(aDest[nLine].m_szText, GlobalText[nTextIndex], pRequestInfo->m_wIndex,
#ifdef ASG_ADD_TIME_LIMIT_QUEST
					MIN(pRequestInfo->m_dwCurValue, pRequestInfo->m_dwValue),
#else	// ASG_ADD_TIME_LIMIT_QUEST
					MIN((DWORD)pRequestInfo->m_wCurValue, pRequestInfo->m_dwValue),
#endif	// ASG_ADD_TIME_LIMIT_QUEST
					pRequestInfo->m_dwValue);
			}
			break;

		case QUEST_REQUEST_EVENT_MAP_CLEAR_BLOOD:
		case QUEST_REQUEST_EVENT_MAP_CLEAR_CHAOS:
		case QUEST_REQUEST_EVENT_MAP_CLEAR_DEVIL:
		case QUEST_REQUEST_EVENT_MAP_CLEAR_ILLUSION:
			{
#ifdef ASG_ADD_TIME_LIMIT_QUEST
				if (pRequestInfo->m_dwCurValue == 0)
#else	// ASG_ADD_TIME_LIMIT_QUEST
				if (pRequestInfo->m_wCurValue == 0)
#endif	// ASG_ADD_TIME_LIMIT_QUEST
				{
					aDest[nLine].m_dwColor = ARGB(255, 255, 30, 30);
					bRequestComplete = false;
				}
				else
					aDest[nLine].m_dwColor = ARGB(255, 223, 191, 103);

				int nTextIndex = 0;
				switch (pRequestInfo->m_dwType)
				{
				case QUEST_REQUEST_EVENT_MAP_CLEAR_BLOOD:
					nTextIndex = 3078;	// 3078	"블러드캐슬 Lv.%lu 클리어"
					break;
				case QUEST_REQUEST_EVENT_MAP_CLEAR_CHAOS:
					nTextIndex = 3076;	// 3076	"카오스캐슬 Lv.%lu 클리어"
					break;
				case QUEST_REQUEST_EVENT_MAP_CLEAR_DEVIL:
					nTextIndex = 3080;	// 3080	"악마의광장 Lv.%lu 클리어"
					break;
				case QUEST_REQUEST_EVENT_MAP_CLEAR_ILLUSION:
					nTextIndex = 3081;	// 3081	"환영사원 Lv.%lu 클리어"
					break;
				}
				::sprintf(aDest[nLine].m_szText, GlobalText[nTextIndex], pRequestInfo->m_wIndex);
			}
			break;
#endif	// ASG_ADD_QUEST_REQUEST_REWARD_TYPE
		}
		aDest[nLine].m_szText[QM_MAX_REQUEST_REWARD_TEXT_LEN - 1] = 0;	// 글자 제한 수를 넘을 경우 종료 문자 강제로 넣어줌.
	}

// 보상.
#ifdef ASG_ADD_QUEST_REQUEST_REWARD_TYPE
	BYTE byRewardCount;
	SQuestReward* pRewardInfo;
	i = 0;
	int j;
	for (j = 0; j < 2; ++j)
	{
		if (0 == j && pRequestReward->m_byGeneralRewardCount)	// 일반 보상 내용이 있다면.
			::strcpy(aDest[nLine].m_szText, GlobalText[2810]);		// 2810 "보     상"
		else if (1 == j && pRequestReward->m_byRandRewardCount)	// 랜덤 보상 내용이 있다면. 
			::sprintf(aDest[nLine].m_szText, GlobalText[3082], pRequestReward->m_byRandGiveCount);// 3082 "랜덤 보상(%lu가지 지급)"
		else
			continue;
		aDest[nLine].m_hFont = g_hFontBold;
		aDest[nLine++].m_dwColor = ARGB(255, 179, 230, 77);

		byRewardCount = 0 == j
			? pRequestReward->m_byGeneralRewardCount
			: pRequestReward->m_byGeneralRewardCount + pRequestReward->m_byRandRewardCount;
		for (; i < byRewardCount; ++i, ++nLine)
		{
			pRewardInfo = &pRequestReward->m_aReward[i];
			
			aDest[nLine].m_hFont = g_hFont;
			aDest[nLine].m_dwColor = 0 == j ? ARGB(255, 223, 191, 103) : ARGB(255, 103, 103, 223);
			aDest[nLine].m_eRequestReward = RRC_REWARD;
			aDest[nLine].m_dwType = pRewardInfo->m_dwType;
			aDest[nLine].m_wIndex = pRewardInfo->m_wIndex;
			aDest[nLine].m_pItem = pRewardInfo->m_pItem;
		
			switch (pRewardInfo->m_dwType)
			{
			case QUEST_REWARD_NONE:
				::strcpy(aDest[nLine].m_szText, GlobalText[1361]);	// 1361 "없음"
				break;
				
			case QUEST_REWARD_EXP:
				::sprintf(aDest[nLine].m_szText, "Exp.: %lu", pRewardInfo->m_dwValue);
				break;
				
			case QUEST_REWARD_ZEN:
				::sprintf(aDest[nLine].m_szText, "Zen: %lu", pRewardInfo->m_dwValue);
				break;
				
			case QUEST_REWARD_ITEM:
				char szItemName[32];
				::GetItemName((int)pRewardInfo->m_pItem->Type, (pRewardInfo->m_pItem->Level>>3)&15,
					szItemName);
				::sprintf(aDest[nLine].m_szText, "Item: %s x %lu",
					szItemName, pRewardInfo->m_dwValue);
				break;
				
			case QUEST_REWARD_BUFF:
				{
					const BuffInfo buffinfo = g_BuffInfo((eBuffState)pRewardInfo->m_wIndex);
					::sprintf(aDest[nLine].m_szText, "Bonus: %s x %lu%s", buffinfo.s_BuffName,
						pRewardInfo->m_dwValue, GlobalText[2300]);	// '분'
				}
				break;
				
	#ifdef ASG_ADD_GENS_SYSTEM
			case QUEST_REWARD_CONTRIBUTE:
				::sprintf(aDest[nLine].m_szText, GlobalText[2994], pRewardInfo->m_dwValue);	// 2994 "기여도: %lu"
				break;
	#endif	// ASG_ADD_GENS_SYSTEM
			}
			aDest[nLine].m_szText[QM_MAX_REQUEST_REWARD_TEXT_LEN - 1] = 0;
		}
	}
#else	// ASG_ADD_QUEST_REQUEST_REWARD_TYPE
	aDest[nLine].m_hFont = g_hFontBold;
	aDest[nLine].m_dwColor = ARGB(255, 179, 230, 77);
	::strcpy(aDest[nLine++].m_szText, GlobalText[2810]);	// 2810 "보     상"

	SQuestReward* pRewardInfo;
	for (i = 0; i < pRequestReward->m_byRewardCount; ++i, ++nLine)
	{
		pRewardInfo = &pRequestReward->m_aReward[i];

		aDest[nLine].m_hFont = g_hFont;
		aDest[nLine].m_dwColor = ARGB(255, 223, 191, 103);
		aDest[nLine].m_eRequestReward = RRC_REWARD;
		aDest[nLine].m_dwType = pRewardInfo->m_dwType;
		aDest[nLine].m_wIndex = pRewardInfo->m_wIndex;
		aDest[nLine].m_pItem = pRewardInfo->m_pItem;

		switch (pRewardInfo->m_dwType)
		{
		case QUEST_REWARD_NONE:
			::strcpy(aDest[nLine].m_szText, GlobalText[1361]);	// 1361 "없음"
			break;

		case QUEST_REWARD_EXP:
			::sprintf(aDest[nLine].m_szText, "Exp.: %lu", pRewardInfo->m_dwValue);
			break;

		case QUEST_REWARD_ZEN:
			::sprintf(aDest[nLine].m_szText, "Zen: %lu", pRewardInfo->m_dwValue);
			break;
			
		case QUEST_REWARD_ITEM:
			char szItemName[32];
			::GetItemName((int)pRewardInfo->m_pItem->Type, (pRewardInfo->m_pItem->Level>>3)&15,
				szItemName);
			::sprintf(aDest[nLine].m_szText, "Item: %s x %lu",
				szItemName, pRewardInfo->m_dwValue);
			break;
			
		case QUEST_REWARD_BUFF:
			{
				const BuffInfo buffinfo = g_BuffInfo((eBuffState)pRewardInfo->m_wIndex);
				::sprintf(aDest[nLine].m_szText, "Bonus: %s x %lu%s", buffinfo.s_BuffName,
					pRewardInfo->m_dwValue, GlobalText[2300]);	// '분'
			}
			break;

#ifdef ASG_ADD_GENS_SYSTEM
		case QUEST_REWARD_CONTRIBUTE:
			::sprintf(aDest[nLine].m_szText, GlobalText[2994], pRewardInfo->m_dwValue);	// 2994 "기여도: %lu"
			break;
#endif	// ASG_ADD_GENS_SYSTEM
		}
		aDest[nLine].m_szText[QM_MAX_REQUEST_REWARD_TEXT_LEN - 1] = 0;
	}
#endif	// ASG_ADD_QUEST_REQUEST_REWARD_TYPE

	return bRequestComplete;
}

//*****************************************************************************
// 함수 이름 : SetEPRequestRewardState()
// 함수 설명 : 에피소드 요구, 보상 상태 세팅.
// 매개 변수 : dwQuestIndex	: 퀘스트 인덱스.
//			   ProgressState: 요구사항이 있는 QS 진행 중일때만 true.
//*****************************************************************************
void CQuestMng::SetEPRequestRewardState(DWORD dwQuestIndex, bool ProgressState)
{
	m_mapEPRequestRewardState[HIWORD(dwQuestIndex)] = ProgressState;
}

//*****************************************************************************
// 함수 이름 : IsEPRequestRewardState()
// 함수 설명 : 에피소드가 요구, 보상 상태인가?.
// 매개 변수 : dwQuestIndex	: 퀘스트 인덱스.
// 반환 값	 : 요구사항이 있는 QS 진행 중일때만 true. 그 외는 false.
//*****************************************************************************
bool CQuestMng::IsEPRequestRewardState(DWORD dwQuestIndex)
{
	WORD wEP = HIWORD(dwQuestIndex);

	map<WORD, bool>::const_iterator iter = m_mapEPRequestRewardState.find(wEP);
	if (iter == m_mapEPRequestRewardState.end())
		return false;

	return m_mapEPRequestRewardState[wEP];
}

//*****************************************************************************
// 함수 이름 : IsQuestByEtc()
// 함수 설명 : 퀘스트 인덱스(dwQuestIndex)가 기타 상황에 의한 퀘스트인가?
// 매개 변수 : dwQuestIndex	: 퀘스트 인덱스.
// 반환 값	 : 요구사항이 있는 QS 진행 중일때만 true. 그 외는 false.
//*****************************************************************************
bool CQuestMng::IsQuestByEtc(DWORD dwQuestIndex)
{
	QuestProgressMap::const_iterator iter = m_mapQuestProgress.find(dwQuestIndex);
	_ASSERT(iter != m_mapQuestProgress.end());

	if (iter->second.m_byUIType == 1)	// 기타 상황에 의한 퀘스트 진행(1)이라면.
		return true;
	else
		return false;
}

//*****************************************************************************
// 함수 이름 : SetQuestIndexByEtcList()
// 함수 설명 : 기타 상황에 의한 퀘스트 인덱스 리스트 세팅.
// 매개 변수 : adwSrcQuestIndex	: 원본 퀘스트 인덱스 배열 주소.
//								 (NULL이면 리스트 초기화)
//			   nIndexCount		: 원본 퀘스트 인덱스 개수.
//*****************************************************************************
void CQuestMng::SetQuestIndexByEtcList(DWORD* adwSrcQuestIndex, int nIndexCount)
{
	m_listQuestIndexByEtc.clear();

	if (NULL == adwSrcQuestIndex)
		return;
	
	int i;
	for (i = 0; i < nIndexCount; ++i)
		m_listQuestIndexByEtc.push_back(adwSrcQuestIndex[i]);
}

//*****************************************************************************
// 함수 이름 : IsQuestIndexByEtcListEmpty()
// 함수 설명 : 기타 상황에 의한 퀘스트 인덱스 리스트가 비어있는가?
// 반환 값	 : true이면 비어 있음.
//*****************************************************************************
bool CQuestMng::IsQuestIndexByEtcListEmpty()
{
	return m_listQuestIndexByEtc.empty();
}

//*****************************************************************************
// 함수 이름 : SendQuestIndexByEtcSelection()
// 함수 설명 : 기타 상황에 의한 퀘스트 인덱스를 선택해서 서버로 알림.
//*****************************************************************************
void CQuestMng::SendQuestIndexByEtcSelection()
{
	if (IsQuestIndexByEtcListEmpty())
		return;

	DWordList::iterator iter = m_listQuestIndexByEtc.begin();
	SendQuestSelection(*iter, 0);
}

//*****************************************************************************
// 함수 이름 : DelQuestIndexByEtcList()
// 함수 설명 : 기타 상황에 의한 퀘스트 인덱스 리스트에서 매개변수(dwQuestIndex)
//			  와 같은 에피소드 퀘스트 삭제.
// 매개 변수 : dwQuestIndex	: 삭제할 에피소드의 퀘스트 인덱스.
//*****************************************************************************
void CQuestMng::DelQuestIndexByEtcList(DWORD dwQuestIndex)
{
	// QS가 0이면 진행중인 퀘스트 리스트에 안나오므로 삭제하면 안됨.
	//다시 로그인 하기 전에는 해당 에피소드 퀘스트에 접근할 방법이 없기 때문.
	if (0x0000 == LOWORD(dwQuestIndex))
		return;

	// 같은 EP(에피소드)가 리스트에 있으면 삭제.
	DWordList::iterator iter;
	for (iter = m_listQuestIndexByEtc.begin(); iter != m_listQuestIndexByEtc.end(); advance(iter, 1))
	{
		if (HIWORD(*iter) == HIWORD(dwQuestIndex))
		{
			m_listQuestIndexByEtc.erase(iter);
			break;
		}
	}
}

//*****************************************************************************
// 함수 이름 : SetCurQuestIndexList()
// 함수 설명 : 현재 진행 중인 퀘스트 인덱스 리스트 세팅.
// 매개 변수 : adwCurQuestIndex	: 현재 진행중인 퀘스트 인덱스 배열 주소.
//			   nIndexCount		: 현재 진행중인 퀘스트 인덱스 개수.
//*****************************************************************************
void CQuestMng::SetCurQuestIndexList(DWORD* adwCurQuestIndex, int nIndexCount)
{
	m_listCurQuestIndex.clear();

	int i;
	for (i = 0; i < nIndexCount; ++i)
#ifdef LJH_FIX_BUG_DISPLAYING_NULL_TITLED_QUEST_LIST
		if (GetSubject(adwCurQuestIndex[i]) != NULL)
			m_listCurQuestIndex.push_back(adwCurQuestIndex[i]);
#else  //LJH_FIX_BUG_DISPLAYING_NULL_TITLED_QUEST_LIST
		m_listCurQuestIndex.push_back(adwCurQuestIndex[i]);
#endif //LJH_FIX_BUG_DISPLAYING_NULL_TITLED_QUEST_LIST

	// 퀘스트 정보창의 진행중인 퀘스트 리스트 박스 갱신.
	g_pMyQuestInfoWindow->SetCurQuestList(&m_listCurQuestIndex);
}

//*****************************************************************************
// 함수 이름 : AddCurQuestIndexList()
// 함수 설명 : 진행 중인 퀘스트 인덱스 리스트에 퀘스트 인덱스(dwQuestIndex) 추
//			  가.
//			   리스트 내에 같은 에피소드인 퀘스트가 있다면 dwQuestIndex를 추가
//            후 삭제.
//			   QS(퀘스트 상태)가 0, 255인 경우는 리스트 내에 같은 에피소드인 퀘
//			  스트 인덱스가 있다면 추가는 않고 삭제만 함.
// 매개 변수 : dwQuestIndex	: 추가할 퀘스트 인덱스.
//*****************************************************************************
void CQuestMng::AddCurQuestIndexList(DWORD dwQuestIndex)
{
	WORD wEP = HIWORD(dwQuestIndex);	// 에피소드.
	WORD wQS = LOWORD(dwQuestIndex);	// 퀘스트 상태.
	bool bNotFound = true;

	DWordList::iterator iter;
	for (iter = m_listCurQuestIndex.begin(); iter != m_listCurQuestIndex.end(); advance(iter, 1))
	{
		if (wEP == HIWORD(*iter))	// 같은 에피소드인가?
		{
#ifdef ASG_FIX_QUEST_GIVE_UP
			// 0, 255가 아니면 바로 앞에 삽입.
			if (wQS != 0x0000 && wQS != 0x00ff)
#else	// ASG_FIX_QUEST_GIVE_UP
			// 0, 254, 255가 아니면 바로 앞에 삽입.
			if (wQS != 0x0000 && wQS != 0x00fe && wQS != 0x00ff)
#endif	// ASG_FIX_QUEST_GIVE_UP
				m_listCurQuestIndex.insert(iter, dwQuestIndex);

			// 같은 에피소드인 진행중인 퀘스트 인덱스 삭제.
			m_listCurQuestIndex.erase(iter);

			bNotFound = false;
			break;
		}
	}

	if (bNotFound)	// 못 찾았다면.
	{
#ifdef ASG_FIX_QUEST_GIVE_UP
		// 0, 255가 아니면 리스트 뒤에 추가.
		if (wQS != 0x0000 && wQS != 0x00ff)
#else	// ASG_FIX_QUEST_GIVE_UP
		// 0, 254, 255가 아니면 리스트 뒤에 추가.
		if (wQS != 0x0000 && wQS != 0x00fe && wQS != 0x00ff)
#endif	// ASG_FIX_QUEST_GIVE_UP
			m_listCurQuestIndex.push_back(dwQuestIndex);
	}

	m_listCurQuestIndex.sort();	// 리스트 정렬.

	// 퀘스트 정보창의 진행중인 퀘스트 리스트 박스 갱신.
	g_pMyQuestInfoWindow->SetCurQuestList(&m_listCurQuestIndex);
}

//*****************************************************************************
// 함수 이름 : RemoveCurQuestIndexList()
// 함수 설명 : 리스트 내에 같은 에피소드인 퀘스트가 있다면 해당 인덱스 삭제.
// 매개 변수 : dwQuestIndex	: 퀘스트 인덱스.
//*****************************************************************************
void CQuestMng::RemoveCurQuestIndexList(DWORD dwQuestIndex)
{
	WORD wEP = HIWORD(dwQuestIndex);	// 에피소드.

	DWordList::iterator iter;
	for (iter = m_listCurQuestIndex.begin(); iter != m_listCurQuestIndex.end(); advance(iter, 1))
	{
		if (wEP == HIWORD(*iter))	// 같은 에피소드인가?
		{	
			// 같은 에피소드인 진행중인 퀘스트 인덱스 삭제.
			m_listCurQuestIndex.erase(iter);			
			break;
		}
	}
	
	// 퀘스트 정보창의 진행중인 퀘스트 리스트 박스 갱신.
	g_pMyQuestInfoWindow->SetCurQuestList(&m_listCurQuestIndex);
}

//*****************************************************************************
// 함수 이름 : IsIndexInCurQuestIndexList()
// 함수 설명 : 진행 중인 퀘스트 인덱스 리스트에 퀘스트 인덱스(dwQuestIndex)가
//			  있는가?
// 매개 변수 : dwQuestIndex	: 찾을 퀘스트 인덱스.
// 반환 값	 : true이면 있음.
//*****************************************************************************
bool CQuestMng::IsIndexInCurQuestIndexList(DWORD dwQuestIndex)
{
	DWordList::iterator iter;
	for (iter = m_listCurQuestIndex.begin(); iter != m_listCurQuestIndex.end(); advance(iter, 1))
	{
		if (*iter == dwQuestIndex)
			return true;
	}

	return false;
}

#endif	// ASG_ADD_NEW_QUEST_SYSTEM