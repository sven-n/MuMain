#pragma once

#include "Protocol.h"
#include "User.h"

#define MURUMMY_MAX_CARD 24
#define MURUMMY_MAX_CARD_COLOR 3
#define MURUMMY_MAX_SLOT 8

struct _tagMuRummyCardInfo
{
	char btColor;
	char btNumber;
	char btSlotNum;
};

struct _tagMuRummyCardInfoDS
{
	_tagMuRummyCardInfoDS()
	{
		this->btColor = -1;
		this->btNumber = -1;
		this->btSlotNum = -1;
		this->btSeq = -1;
		this->btStatus = -1;
	};

	BYTE btColor;
	BYTE btNumber;
	BYTE btSlotNum;
	BYTE btSeq;
	BYTE btStatus;
};

struct _tagMuRummyCardUpdateDS
{
	_tagMuRummyCardUpdateDS()
	{
		this->btSlotNum = -1;
		this->btSeq = -1;
		this->btStatus = -1;
	};

	BYTE btSlotNum;
	BYTE btSeq;
	BYTE btStatus;
};

struct _tagPMSG_REQ_MURUMMY_EVENT_OPEN
{
	PSBMSG_HEAD h; // C1:4D:0F
};

struct _tagPMSG_ANS_MURUMMY_EVENT_OPEN
{
	PSBMSG_HEAD h; // C1:4D:0F
	BYTE btResult;
	BYTE btEventTime1;
	BYTE btEventTime2;
	BYTE btEventTime3;
	BYTE btEventTime4;
};

struct _tagPMSG_REQ_MURUMMY_INFO
{
	PSBMSG_HEAD h; // C1:4D:10
};

struct _tagPMSG_ANS_MURUMMY_INFO
{
	_tagPMSG_ANS_MURUMMY_INFO()
	{
		this->btTotScoreH = 0;
		this->btTotScoreL = 0;
		this->btCardDeckCnt = 0;
	};

	PSBMSG_HEAD h; // C1:4D:10
	BYTE btTotScoreH;
	BYTE btTotScoreL;
	BYTE btCardDeckCnt;
	_tagMuRummyCardInfo stMuRummyCardInfo[5];
};

struct _tagPMSG_REQ_REVEAL_CARD
{
	PSBMSG_HEAD h; // C1:4D:11
};

struct _tagPMSG_ANS_REVEAL_CARD
{
	_tagPMSG_ANS_REVEAL_CARD()
	{
		this->btCardDeckCnt = 0;
	};

	PSBMSG_HEAD h; // C1:4D:11
	_tagMuRummyCardInfo stMuRummyCardInfo[5];
	BYTE btCardDeckCnt;
};

struct _tagPMSG_REQ_CARD_MOVE
{
	PSBMSG_HEAD h; // C1:4D:12
	BYTE sSlot;
	BYTE tSlot;
};

struct _tagPMSG_ANS_CARD_MOVE
{
	_tagPMSG_ANS_CARD_MOVE()
	{
		this->sSlot = -1;
		this->tSlot = -1;
		this->btColor = -1;
		this->btNumber = -1;
	};

	PSBMSG_HEAD h; // C1:4D:12
	BYTE sSlot;
	BYTE tSlot;
	BYTE btColor;
	BYTE btNumber;
};

struct _tagPMSG_REQ_CARD_REMOVE
{
	PSBMSG_HEAD h; // C1:4D:13
	BYTE btSlot;
};

struct _tagPMSG_ANS_CARD_REMOVE
{
	PSBMSG_HEAD h; // C1:4D:13
	BYTE btResult;
};

struct _tagPMSG_REQ_CARD_MATCH
{
	PSBMSG_HEAD h; // C1:4D:14
};

struct _tagPMSG_ANS_CARD_MATCH
{
	PSBMSG_HEAD h; // C1:4D:14
	BYTE btScoreH;
	BYTE btScoreL;
	BYTE btTotScoreH;
	BYTE btTotScoreL;
	BYTE btResult;
};

struct _tagPMSG_REQ_MURUMMY_END
{
	PSBMSG_HEAD h; // C1:4D:15
};

struct _tagPMSG_ANS_MURUMMY_END
{
	PSBMSG_HEAD h; // C1:4D:15
	BYTE btResult;
};

struct _tagPMSG_SEND_SLOTCARD_LIST
{
	PSBMSG_HEAD h; // C1:4D:16
	_tagMuRummyCardInfo stMuRummyCardInfo[5];
};

struct _tagPMSG_SEND_MURUMMY_MSG
{
	_tagPMSG_SEND_MURUMMY_MSG()
	{
		this->btNotiIndex = 0;
		this->btValueH = 0;
		this->btValueL = 0;
	};

	PSBMSG_HEAD h; // C1:4D:17
	BYTE btNotiIndex;
	BYTE btValueH;
	BYTE btValueL;
};

struct _tagPMSG_SEND_MURUMMYEVENT_ON
{
	PSBMSG_HEAD h; // C1:4D:18
};

struct _tagPMSG_REQ_MURUMMY_SELECT_DS
{
	PSBMSG_HEAD h; // C1:12:00
	char AccountID[11];
	char Name[11];
	WORD aIndex;
};

struct _tagPMSG_ANS_MURUMMY_SELECT_DS
{
	PSBMSG_HEAD h; // C1:12:00
	WORD aIndex;
	WORD wScore;
	BYTE btResult;
	_tagMuRummyCardInfoDS stMuRummyCardInfoDS[MURUMMY_MAX_CARD];
};

struct _tagPMSG_REQ_MURUMMY_INSERT_DS
{
	PSBMSG_HEAD h; // C1:12:30
	char AccountID[11];
	char Name[11];
	WORD aIndex;
	_tagMuRummyCardInfoDS stMuRummyCardInfoDS[MURUMMY_MAX_CARD];
};

struct _tagPMSG_REQ_MURUMMY_UPDATE_DS 
{
	PSBMSG_HEAD h; // C1:12:31
	char AccountID[11];
	char Name[11];
	BYTE btSlotNum;
	BYTE btStatus;
	BYTE btSequence;
};

struct _tagPMSG_REQ_MURUMMY_SCORE_UPDATE_DS
{
	PSBMSG_HEAD h; // C1:12:32
	char AccountID[11];
	char Name[11];
	WORD wScore;
	_tagMuRummyCardUpdateDS stCardUpdateDS[3];
};

struct _tagPMSG_REQ_MURUMMY_DELETE_DS
{
	PSBMSG_HEAD h; // C1:12:33
	char AccountID[11];
	char Name[11];
};

struct _tagPMSG_REQ_MURUMMY_SLOTUPDATE_DS
{
	PSBMSG_HEAD h; // C1:12:34
	char AccountID[11];
	char Name[11];
	_tagMuRummyCardUpdateDS stCardUpdateDS;
};

struct _tagPMSG_REQ_MURUMMY_INFO_UPDATE_DS
{
	PSBMSG_HEAD h; // C1:12:35
	char AccountID[11];
	char Name[11];
	WORD wScore;
	_tagMuRummyCardUpdateDS stMuRummyCardUpdateDS[MURUMMY_MAX_CARD];
};

class CMuRummyCardInfo
{
public:
	CMuRummyCardInfo(){}
	void SetColor(int iColor) { this->m_iColor = iColor; };
	void SetNumber(int iNumber) { this->m_iNumber = iNumber; };
	void SetState(int iState) { this->m_iState = iState; };
	void SetSlotNum(int iSlotNum) { this->m_iSlotNum = iSlotNum; };
	int GetColor() { return this->m_iColor; };
	int GetNumber() { return this->m_iNumber; };
	int GetState() { return this->m_iState; };
	int GetSlotNum() { return this->m_iSlotNum; };
private:
	int m_iColor;
	int m_iNumber;
	int m_iState;
	int m_iSlotNum;
};

class CMuRummyInfo
{
public:
	CMuRummyInfo()
	{
		this->Init();
	}

	virtual ~CMuRummyInfo()
	{
		if (this->m_pCCardInfo)
		{
			delete []this->m_pCCardInfo;
			this->m_pCCardInfo = NULL;
		}

		if (this->m_pSlotCardIndex)
		{
			delete this->m_pSlotCardIndex;
			this->m_pSlotCardIndex = NULL;
		}
	}

	void Init()
	{
		this->m_pCCardInfo = 0;
		this->m_pSlotCardIndex = 0;
		this->m_iScore = 0;
		this->m_iCardDeckCnt = 0;
		this->m_bPlayCard = 0;
		this->m_bMuRummyInfoLoad = 0;

		if (!this->m_pCCardInfo)
		{
			this->m_pCCardInfo = new CMuRummyCardInfo[MURUMMY_MAX_CARD];
		}

		if (!this->m_pSlotCardIndex)
		{
			this->m_pSlotCardIndex = new BYTE[MURUMMY_MAX_SLOT];
		}
	}

	void Release()
	{
	}

	void Clear()
	{
		for (int i = 0; i < MURUMMY_MAX_CARD; i++)
		{
			this->m_pCCardInfo[i].SetColor(0);
			this->m_pCCardInfo[i].SetNumber(0);
			this->m_pCCardInfo[i].SetState(0);
			this->m_pCCardInfo[i].SetSlotNum(-1);
		}

		for (int i = 0; i < MURUMMY_MAX_SLOT; i++)
		{
			this->m_pSlotCardIndex[i] = -1;
		}

		this->m_iScore = 0;
		this->m_iCardDeckCnt = 0;
		this->m_bPlayCard = 0;
		this->m_bWaitReward = 0;
	}

	CMuRummyCardInfo* GetCardInfo()
	{
		if (!this->m_pCCardInfo)
		{
			return 0;
		}

		return this->m_pCCardInfo;
	}

	BYTE* GetSlotIndexInfo()
	{
		if (!this->m_pSlotCardIndex)
		{
			return 0;
		}

		return this->m_pSlotCardIndex;
	}

	void SetScore(int iScore) { this->m_iScore = iScore; };
	void SetCardDeckCnt(int iCardDeckCnt) { this->m_iCardDeckCnt = iCardDeckCnt; };
	void SetGetPlayCard(bool bPlayCard) { this->m_bPlayCard = bPlayCard; };
	void SetMuRummyInfoLoad(bool bMuRummyInfoLoad) { this->m_bMuRummyInfoLoad = bMuRummyInfoLoad; };
	void SetWaitReward(bool bWaitReward) { this->m_bWaitReward = bWaitReward; };
	int GetScore() { return this->m_iScore; };
	int GetCardDeckCnt() { return this->m_iCardDeckCnt; };
	bool IsGetPlayCard() { return this->m_bPlayCard; };
	bool IsMuRummyInfoLoad() { return this->m_bMuRummyInfoLoad; };
	bool IsWaitReward() { return this->m_bWaitReward; };
private:
	int m_iScore;
	int m_iCardDeckCnt;
	bool m_bPlayCard;
	bool m_bMuRummyInfoLoad;
	bool m_bWaitReward;
	CMuRummyCardInfo* m_pCCardInfo;
	LPBYTE m_pSlotCardIndex;
};

class CMuRummyMng
{
public:
	CMuRummyMng();
	virtual ~CMuRummyMng();
	int IsMuRummyEventOn();
	int GetRewardScore1();
	int GetRewardScore2();
	void SetCardDeck(CMuRummyInfo* pMuRummyInfo);
	void CardShuffle(CMuRummyCardInfo* pCCardInfo);
	bool SetPlayCardInfo(CMuRummyInfo* pMuRummyInfo, _tagMuRummyCardInfo* pOutCardInfo);
	BYTE GetTempCardShowSlot(LPBYTE pSlotCardIndex);
	BYTE CardSlotMove(CMuRummyInfo* pMuRummyInfo, int sSlot, int tSlot, CMuRummyCardInfo* pCOutCardInfo, LPOBJ obj);
	BYTE CardSlotReMove(CMuRummyInfo* pCMuRummyInfo, int sSlot);
	BYTE CardMatchCheck(CMuRummyInfo* pCMuRummyInfo, int& iOutScore, _tagMuRummyCardUpdateDS* pOutCardUpdateDS, LPOBJ obj);
	void CardSortAscending(CMuRummyCardInfo* pCCardInfo);
	int IsCardSameColor(CMuRummyCardInfo* pCCardInfo);
	int IsCardNumber(CMuRummyCardInfo* pCCardInfo);
	bool FillEmptySlotCard(CMuRummyInfo* pMuRummyInfo, _tagMuRummyCardInfo* pOutCardInfo, LPOBJ obj); 
	void CGReqMuRummyEventOpen(_tagPMSG_REQ_MURUMMY_EVENT_OPEN* lpMsg, int aIndex);
	void CGReqMuRummyStart(_tagPMSG_REQ_MURUMMY_INFO* lpMsg, int aIndex);
	void CGReqCardReveal(_tagPMSG_REQ_REVEAL_CARD* lpMsg, int aIndex);
	void CGReqCardMove(_tagPMSG_REQ_CARD_MOVE* lpMsg, int aIndex);
	void CGReqCardReMove(_tagPMSG_REQ_CARD_REMOVE* lpMsg, int aIndex);
	void CGReqCardMatch(_tagPMSG_REQ_CARD_MATCH* lpMsg, int aIndex);
	void CGReqMuRummyEnd(_tagPMSG_REQ_MURUMMY_END* lpMsg, int aIndex);
	void GCSendCardList(int aIndex);
	void GCSendMsg(int aIndex, BYTE btNotiIndex, int iValue);
	void GDReqCardInfo(LPOBJ obj);
	void GDAnsCardInfo(_tagPMSG_ANS_MURUMMY_SELECT_DS* lpMsg);
	void GDReqCardInfoInsert(LPOBJ obj);
	void GDReqCardInfoUpdate(LPOBJ obj, CMuRummyCardInfo* pCCardInfo, int iSeq);
	void GDReqScoreUpdate(LPOBJ obj, WORD wScore, _tagMuRummyCardUpdateDS* lpMsg);
	void GDReqScoreDelete(LPOBJ obj);
	void GDReqSlotInfoUpdate(LPOBJ obj, BYTE btSeq, BYTE btSlotNum, BYTE btStatus);
	void GDReqMuRummyInfoUpdate(LPOBJ obj);
};

extern CMuRummyMng gMuRummy;
