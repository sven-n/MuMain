// MuRummy.cpp: implementation of the CMuRummy class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MuRummy.h"
#include "DSProtocol.h"
#include "EventInventory.h"
#include "GameMain.h"
#include "ItemBagManager.h"
#include "ItemManager.h"
#include "ServerInfo.h"
#include "Util.h"

CMuRummyMng gMuRummy;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMuRummyMng::CMuRummyMng() // OK
{

}

CMuRummyMng::~CMuRummyMng() // OK
{

}

int CMuRummyMng::IsMuRummyEventOn() // OK
{
	#if(GAMESERVER_UPDATE>=802)

	return gServerInfo.m_MuRummySwitch;

	#else

	return 0;

	#endif
}

int CMuRummyMng::GetRewardScore1() // OK
{
	#if(GAMESERVER_UPDATE>=802)

	return gServerInfo.m_MuRummyRewardScore1;

	#else

	return 0;

	#endif
}

int CMuRummyMng::GetRewardScore2() // OK
{
	#if(GAMESERVER_UPDATE>=802)

	return gServerInfo.m_MuRummyRewardScore2;

	#else

	return 0;

	#endif
}

void CMuRummyMng::SetCardDeck(CMuRummyInfo* pMuRummyInfo) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	int nCardArr = 0;

	if (!pMuRummyInfo)
	{
		return;
	}

	CMuRummyCardInfo* pCCardInfo = pMuRummyInfo->GetCardInfo();

	if (!pCCardInfo)
	{
		return;
	}

	for (int nCardColor = 0; nCardColor < MURUMMY_MAX_CARD_COLOR; nCardColor++)
	{
		for (int nCardNum = 0; nCardNum < MURUMMY_MAX_SLOT; nCardNum++)
		{
			if (nCardArr < 0 || nCardArr >= MURUMMY_MAX_CARD)
			{
				return;
			}

			pCCardInfo[nCardArr].SetColor(nCardColor + 1);
			pCCardInfo[nCardArr].SetNumber(nCardNum + 1);
			pCCardInfo[nCardArr].SetState(0);
			pCCardInfo[nCardArr].SetSlotNum(0);
			nCardArr++;
		}
	}

	LPBYTE pSlotIndexInfo = pMuRummyInfo->GetSlotIndexInfo();

	if (!pSlotIndexInfo)
	{
		return;
	}

	for (int i = 0; i < MURUMMY_MAX_SLOT; i++)
	{
		pSlotIndexInfo[i] = -1;
	}

	pMuRummyInfo->SetCardDeckCnt(MURUMMY_MAX_CARD);

	#endif
}

void CMuRummyMng::CardShuffle(CMuRummyCardInfo* pCCardInfo) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	CMuRummyCardInfo CardInfoTmp;

	for (int nCnt = 0; nCnt < MURUMMY_MAX_CARD; nCnt++)
	{
		DWORD iIndex = nCnt + GetLargeRand() % (MURUMMY_MAX_CARD - nCnt);

		if (iIndex >= MURUMMY_MAX_CARD)
		{
			return;
		}

		CardInfoTmp.SetColor(pCCardInfo[nCnt].GetColor());
		CardInfoTmp.SetNumber(pCCardInfo[nCnt].GetNumber());
		CardInfoTmp.SetState(pCCardInfo[nCnt].GetState());
		CardInfoTmp.SetSlotNum(pCCardInfo[nCnt].GetSlotNum());

		pCCardInfo[nCnt].SetColor(pCCardInfo[iIndex].GetColor());
		pCCardInfo[nCnt].SetNumber(pCCardInfo[iIndex].GetNumber());
		pCCardInfo[nCnt].SetState(pCCardInfo[iIndex].GetState());
		pCCardInfo[nCnt].SetSlotNum(pCCardInfo[iIndex].GetSlotNum());

		pCCardInfo[iIndex].SetColor(CardInfoTmp.GetColor());
		pCCardInfo[iIndex].SetNumber(CardInfoTmp.GetNumber());
		pCCardInfo[iIndex].SetState(CardInfoTmp.GetState());
		pCCardInfo[iIndex].SetSlotNum(CardInfoTmp.GetSlotNum());
	}

	#endif
}

bool CMuRummyMng::SetPlayCardInfo(CMuRummyInfo* pMuRummyInfo, _tagMuRummyCardInfo* pOutCardInfo) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	int nCardCnt = 0;

	if (!pMuRummyInfo)
	{
		return false;
	}

	CMuRummyCardInfo* pCCardInfo = pMuRummyInfo->GetCardInfo();

	if (!pCCardInfo)
	{
		return false;
	}

	LPBYTE pSlotIndexInfo = pMuRummyInfo->GetSlotIndexInfo();

	if (!pSlotIndexInfo )
	{
		return false;
	}

	for (int nCnt = 0; nCnt < MURUMMY_MAX_SLOT; nCnt++)
	{
		int nCardArr = pSlotIndexInfo[nCnt];

		if (nCardArr < 0 || nCardArr >= MURUMMY_MAX_CARD)
		{
			continue;
		}

		if (pCCardInfo[nCardArr].GetState() == 1)
		{
			if (nCardCnt < 0 || nCardCnt > 4)
			{
				return false;
			}

			pOutCardInfo[nCardCnt].btColor = pCCardInfo[nCardArr].GetColor();
			pOutCardInfo[nCardCnt].btNumber = pCCardInfo[nCardArr].GetNumber();
			pOutCardInfo[nCardCnt].btSlotNum = pCCardInfo[nCardArr].GetSlotNum();
			nCardCnt++;
		}
	}

	return true;

	#else

	return 0;

	#endif
}

BYTE CMuRummyMng::GetTempCardShowSlot(LPBYTE pSlotCardIndex) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	int iSlotCnt = 0;
	
	for (int i = 0; i < 8; i++)
	{
		if (pSlotCardIndex[i] != (BYTE)-1 )
		{
			if (i >= 5)
			{
				return -2;
			}

			iSlotCnt++;
		}
	}

	if (iSlotCnt >= 5)
	{
		return -1;
	}

	for (int i = 0; i < 5; i++ )
	{
		if (pSlotCardIndex[i] == (BYTE)-1)
		{
			return i;
		}
	}

	return -1;

	#else

	return -1;

	#endif
}

BYTE CMuRummyMng::CardSlotMove(CMuRummyInfo* pCMuRummyInfo, int sSlot, int tSlot, CMuRummyCardInfo* pCOutCardInfo, LPOBJ obj) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	CMuRummyCardInfo* pCCardInfo = pCMuRummyInfo->GetCardInfo();

	if (!pCCardInfo)
	{
		return -1;
	}

	LPBYTE pSlotIndexInfo = pCMuRummyInfo->GetSlotIndexInfo();

	if (!pSlotIndexInfo)
	{
		return -1;
	}

	BYTE sCardArr = pSlotIndexInfo[sSlot];
	BYTE tCardArr = pSlotIndexInfo[tSlot];

	if (sCardArr == (BYTE)-1)
	{
		return -1;
	}

	if (tCardArr != (BYTE)-1)
	{
		return -2;
	}

	if (sCardArr < 0 || sCardArr >= MURUMMY_MAX_CARD)
	{
		return -2;
	}

	if (pCCardInfo[sCardArr].GetState() == 1)
	{
		pSlotIndexInfo[sSlot] = -1;
		pSlotIndexInfo[tSlot] = sCardArr;
		pCCardInfo[sCardArr].SetSlotNum(tSlot);
		memcpy(pCOutCardInfo, &pCCardInfo[sCardArr], sizeof(CMuRummyCardInfo));
		return 0;
	}

	return -1;

	#else

	return -1;

	#endif
}

BYTE CMuRummyMng::CardSlotReMove(CMuRummyInfo* pCMuRummyInfo, int sSlot) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	CMuRummyCardInfo* pCCardInfo = pCMuRummyInfo->GetCardInfo();

	if (!pCCardInfo )
	{
		return -1;
	}

	LPBYTE pSlotIndexInfo = pCMuRummyInfo->GetSlotIndexInfo();

	if (!pSlotIndexInfo )
	{
		return -1;
	}

	if (pSlotIndexInfo[sSlot] == (BYTE)-1)
	{
		return -1;
	}

	BYTE sCardArr = pSlotIndexInfo[sSlot];

	if (pCCardInfo[sCardArr].GetSlotNum() != sSlot)
	{
		return -1;
	}

	pCCardInfo[sCardArr].SetSlotNum(-1);
	pCCardInfo[sCardArr].SetState(2);
	pSlotIndexInfo[sSlot] = -1;
	return sCardArr;

	#else

	return -1;

	#endif
}

BYTE CMuRummyMng::CardMatchCheck(CMuRummyInfo* pCMuRummyInfo, int& iOutScore, _tagMuRummyCardUpdateDS* pOutCardUpdateDS, LPOBJ obj) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	CMuRummyCardInfo* pCCardInfo = pCMuRummyInfo->GetCardInfo();
	
	if (!pCCardInfo)
	{
		return -1;
	}

	LPBYTE pSlotIndexInfo = pCMuRummyInfo->GetSlotIndexInfo();

	if (!pSlotIndexInfo)
	{
		return -1;
	}

	CMuRummyCardInfo CCardInfoTmp[3];

	int nChkCnt = 0;

	for (int nCnt = 5; nCnt < MURUMMY_MAX_SLOT; nCnt++)
	{
		if (pSlotIndexInfo[nCnt] != (BYTE)-1)
		{
			CCardInfoTmp[nChkCnt].SetColor(pCCardInfo[pSlotIndexInfo[nCnt]].GetColor());
			CCardInfoTmp[nChkCnt].SetNumber(pCCardInfo[pSlotIndexInfo[nCnt]].GetNumber());
			CCardInfoTmp[nChkCnt].SetState(pCCardInfo[pSlotIndexInfo[nCnt]].GetState());
			CCardInfoTmp[nChkCnt].SetSlotNum(pCCardInfo[pSlotIndexInfo[nCnt]].GetSlotNum());
			nChkCnt++;
		}
	}

	if (nChkCnt != 3)
	{
		return -1;
	}

	int nRetCardColor = this->IsCardSameColor(CCardInfoTmp);
	int nRetCardNum = this->IsCardNumber(CCardInfoTmp);

	if ((nRetCardColor == 1 && nRetCardNum == 3) || (nRetCardColor == 2 && nRetCardNum == 3) || (nRetCardColor == 1 && nRetCardNum == 4))
	{
		if (nRetCardColor == 1 && nRetCardNum == 3)
		{
			iOutScore = 10 * (CCardInfoTmp->GetNumber() - 1) + 10;
		}

		if (nRetCardColor == 2 && nRetCardNum == 3)
		{
			iOutScore = 10 * (CCardInfoTmp->GetNumber() - 1) + 50;
		}

		if (nRetCardColor == 1 && nRetCardNum == 4)
		{
			iOutScore = 10 * (CCardInfoTmp->GetNumber() - 1) + 20;
		}

		int iCnt = 0;

		for (int i = 5; i < MURUMMY_MAX_SLOT; i++)
		{
			if (pSlotIndexInfo[i] == (BYTE)-1 )
			{
				continue;
			}

			if (pSlotIndexInfo[i] < 0 || pSlotIndexInfo[i] >= MURUMMY_MAX_CARD)
			{
				return -2;
			}

			pCCardInfo[pSlotIndexInfo[i]].SetSlotNum( -1);
			pCCardInfo[pSlotIndexInfo[i]].SetState(3);

			pOutCardUpdateDS[iCnt].btSeq = pSlotIndexInfo[i];
			pOutCardUpdateDS[iCnt].btSlotNum = pCCardInfo[pSlotIndexInfo[i]].GetSlotNum();
			pOutCardUpdateDS[iCnt].btStatus = pCCardInfo[pSlotIndexInfo[i]].GetState();

			pSlotIndexInfo[i] = -1;
			iCnt++;
		}

		return 0;
	}

	int nSlotNum = 5;

	for (int n = 0; n < 5; n++)
	{
		if (pSlotIndexInfo[n] != (BYTE)-1 )
		{
			continue;
		}

		if (nSlotNum < 0 || nSlotNum >= MURUMMY_MAX_SLOT)
		{
			return -2;
		}

		pSlotIndexInfo[n] = pSlotIndexInfo[nSlotNum];

		if (pSlotIndexInfo[n] < 0 || pSlotIndexInfo[n] >= MURUMMY_MAX_CARD)
		{
			return -2;
		}
		
		pCCardInfo[pSlotIndexInfo[n]].SetSlotNum(n);
		pSlotIndexInfo[nSlotNum] = -1;
		nSlotNum++;
	}

	iOutScore = 0;

	return -1;

	#else

	return -1;

	#endif
}

void CMuRummyMng::CardSortAscending(CMuRummyCardInfo* pCCardInfo) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	CMuRummyCardInfo CCardInfoTmp;

	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 3 - i - 1; j++)
		{
			if (pCCardInfo[j].GetNumber() > pCCardInfo[j + 1].GetNumber())
			{
				CCardInfoTmp.SetColor(pCCardInfo[j].GetColor());
				CCardInfoTmp.SetNumber(pCCardInfo[j].GetNumber());
				CCardInfoTmp.SetState(pCCardInfo[j].GetState());
				CCardInfoTmp.SetSlotNum(pCCardInfo[j].GetSlotNum());

				pCCardInfo[j].SetColor(pCCardInfo[j + 1].GetColor());
				pCCardInfo[j].SetNumber(pCCardInfo[j + 1].GetNumber());
				pCCardInfo[j].SetState(pCCardInfo[j + 1].GetState());
				pCCardInfo[j].SetSlotNum(pCCardInfo[j + 1].GetSlotNum());

				pCCardInfo[j + 1].SetColor(CCardInfoTmp.GetColor());
				pCCardInfo[j + 1].SetNumber(CCardInfoTmp.GetNumber());
				pCCardInfo[j + 1].SetState(CCardInfoTmp.GetState());
				pCCardInfo[j + 1].SetSlotNum(CCardInfoTmp.GetSlotNum());
			}
		}
	}

	#endif
}

int CMuRummyMng::IsCardSameColor(CMuRummyCardInfo* pCCardInfo) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	for (int i = 0; i < 2; i++)
	{
		if (pCCardInfo[i].GetColor() != pCCardInfo[i + 1].GetColor())
		{
			return 1;
		}
	}

	return 2;

	#else

	return 0;

	#endif
}

int CMuRummyMng::IsCardNumber(CMuRummyCardInfo* pCCardInfo) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	for (int i = 0; ; i++)
	{
		if (i >= 2 )
		{
			return 4;
		}

		if (pCCardInfo[i].GetNumber() != pCCardInfo[i + 1].GetNumber())
		{
			break;
		}
	}

	this->CardSortAscending(pCCardInfo);

	for (int j = 0; j < 2; j++)
	{
		if ( pCCardInfo[j + 1].GetNumber() - pCCardInfo[j].GetNumber() != 1)
		{
			return 0;
		}
	}

	return 3;

	#else

	return 0;

	#endif
}

bool CMuRummyMng::FillEmptySlotCard(CMuRummyInfo* pMuRummyInfo, _tagMuRummyCardInfo* pOutCardInfo, LPOBJ obj) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	CMuRummyCardInfo* pCCardInfo = pMuRummyInfo->GetCardInfo();

	if (!pCCardInfo)
	{
		return false;
	}

	LPBYTE pSlotIndexInfo = pMuRummyInfo->GetSlotIndexInfo();

	if (!pSlotIndexInfo)
	{
		return false;
	}

	int nCardArr = 0;

	for (int i = 0; i < 5; i++)
	{
		if (pSlotIndexInfo[i] != (BYTE)-1)
		{
			continue;
		}

		if (nCardArr < 0 || nCardArr > 4)
		{
			return false;
		}

		int nCardDeckCnt = pMuRummyInfo->GetCardDeckCnt();
		int nCardSeq = MURUMMY_MAX_CARD - nCardDeckCnt;

		if (nCardSeq >= MURUMMY_MAX_CARD )
		{
			return false;
		}

		pCCardInfo[nCardSeq].SetSlotNum(i);
		pCCardInfo[nCardSeq].SetState(1);
		pSlotIndexInfo[i] = nCardSeq;
		pOutCardInfo[nCardArr].btColor = pCCardInfo[nCardSeq].GetColor();
		pOutCardInfo[nCardArr].btNumber = pCCardInfo[nCardSeq].GetNumber();
		pOutCardInfo[nCardArr].btSlotNum = pCCardInfo[nCardSeq].GetSlotNum();
		pMuRummyInfo->SetCardDeckCnt(nCardDeckCnt - 1);
		nCardArr++;

		if (pMuRummyInfo->GetCardDeckCnt() < 0)
		{
			return false;
		}

		this->GDReqSlotInfoUpdate(obj, nCardSeq, pCCardInfo[nCardSeq].GetSlotNum(), pCCardInfo[nCardSeq].GetState());

		if (!pMuRummyInfo->GetCardDeckCnt())
		{
			return true;
		}
	}

	return true;

	#else

	return 0;

	#endif
}

void CMuRummyMng::CGReqMuRummyEventOpen(_tagPMSG_REQ_MURUMMY_EVENT_OPEN* lpMsg, int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	_tagPMSG_ANS_MURUMMY_EVENT_OPEN pMsg;
	pMsg.h.set(0x4D, 0xF, sizeof(_tagPMSG_ANS_MURUMMY_EVENT_OPEN));
	pMsg.btResult = -1;

	if (!OBJECT_RANGE(aIndex))
	{
		return;
	}

	if (!gObjIsConnected(aIndex))
	{
		return;
	}

	LPOBJ lpObj = &gObj[aIndex];

	if (!lpObj)
	{
		return;
	}

	if (lpObj->CloseType != -1) 
	{
		return;
	}

	if (lpObj->Interface.use > 0 && lpObj->Interface.type != 3)
	{
		return;
	}

	if (lpObj->PShopOpen == 1 || lpObj->ChaosLock == 1)
	{
		return;
	}

	CMuRummyInfo* pMuRummyInfo = lpObj->MuRummyInfo;

	if (!pMuRummyInfo)
	{
		return;
	}

	if (pMuRummyInfo->IsWaitReward())
	{
		return;
	}

	CTime ExpireTime(gServerInfo.m_EventInventoryExpireYear,gServerInfo.m_EventInventoryExpireMonth,gServerInfo.m_EventInventoryExpireDay,0,0,0,-1);

	DWORD time = (DWORD)(ExpireTime.GetTime()-CTime::GetTickCount().GetTime());

	pMsg.btEventTime1 = SET_NUMBERHB(SET_NUMBERHW(time));
	pMsg.btEventTime2 = SET_NUMBERLB(SET_NUMBERHW(time));
	pMsg.btEventTime3 = SET_NUMBERHB(SET_NUMBERLW(time));
	pMsg.btEventTime4 = SET_NUMBERLB(SET_NUMBERLW(time));

	if (!pMuRummyInfo->IsGetPlayCard())
	{
		pMsg.btResult = 1;
		DataSend(aIndex, (LPBYTE)&pMsg, pMsg.h.size);
		return;
	}

	pMsg.btResult = 2;
	DataSend(aIndex, (LPBYTE)&pMsg, pMsg.h.size);
	this->CGReqMuRummyStart(0, aIndex);

	#endif
}

void CMuRummyMng::CGReqMuRummyStart(_tagPMSG_REQ_MURUMMY_INFO* lpMsg, int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	_tagPMSG_ANS_MURUMMY_INFO pMsg;
	pMsg.h.set(0x4D, 0x10, sizeof(_tagPMSG_ANS_MURUMMY_INFO));

	if (!OBJECT_RANGE(aIndex))
	{
		return;
	}

	if (!gObjIsConnected(aIndex))
	{
		return;
	}

	LPOBJ lpObj = &gObj[aIndex];

	if (!lpObj)
	{
		return;
	}

	if (lpObj->CloseType != -1) 
	{
		return;
	}

	CMuRummyInfo* pMuRummyInfo = lpObj->MuRummyInfo;

	if (!pMuRummyInfo)
	{
		return;
	}

	if (!pMuRummyInfo->IsMuRummyInfoLoad())
	{
		return;
	}

	_tagMuRummyCardInfo stCardInfo[5];
	bool bNewGame = false;

	if (pMuRummyInfo->IsGetPlayCard())
	{
		if (this->SetPlayCardInfo(pMuRummyInfo, stCardInfo))
		{
			memcpy(pMsg.stMuRummyCardInfo, stCardInfo, sizeof(stCardInfo));
		}
	}
	else
	{
		bool bItem = 0;

		for (int x = 0; x < 32; x++)
		{
			if ( lpObj->EventInventory[x].IsItem() == 1 && lpObj->EventInventory[x].m_Index == GET_ITEM(14,216) )
			{
				gEventInventory.EventInventoryDelItem(aIndex,x);
				gEventInventory.GCEventItemDeleteSend(aIndex,x,1);
				bItem = true;
				break;
			}
		}

		if (!bItem)
		{
			this->GCSendMsg(aIndex, 0, 0);
			return;
		}

		pMuRummyInfo->Clear();
		pMuRummyInfo->SetGetPlayCard(true);
		this->SetCardDeck(pMuRummyInfo);
		this->CardShuffle(pMuRummyInfo->GetCardInfo());
		this->GDReqCardInfoInsert(lpObj);
		bNewGame = true;
	}

	pMsg.btCardDeckCnt = pMuRummyInfo->GetCardDeckCnt();
	pMsg.btTotScoreH = SET_NUMBERHB(pMuRummyInfo->GetScore());
	pMsg.btTotScoreL = SET_NUMBERLB(pMuRummyInfo->GetScore());
	DataSend(aIndex, (LPBYTE)&pMsg, pMsg.h.size);

	if (bNewGame)
	{
		this->GCSendMsg( aIndex, 1, 0);
	}

	#endif
}

void CMuRummyMng::CGReqCardReveal(_tagPMSG_REQ_REVEAL_CARD* lpMsg, int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	_tagPMSG_ANS_REVEAL_CARD pMsg;
	pMsg.h.set(0x4D, 0x11, sizeof(_tagPMSG_ANS_REVEAL_CARD));

	if (!OBJECT_RANGE(aIndex))
	{
		return;
	}

	if (!gObjIsConnected(aIndex))
	{
		return;
	}

	LPOBJ lpObj = &gObj[aIndex];

	if (!lpObj)
	{
		return;
	}

	if (lpObj->CloseType != -1) 
	{
		return;
	}

	CMuRummyInfo* pMuRummyInfo = lpObj->MuRummyInfo;

	if (!pMuRummyInfo)
	{
		return;
	}

	if (!pMuRummyInfo->GetCardInfo())
	{
		return;
	}

	if (!pMuRummyInfo->GetCardDeckCnt())
	{
		this->GCSendMsg(aIndex, 2, 0);
		return;
	}

	LPBYTE pSlotIndexInfo = pMuRummyInfo->GetSlotIndexInfo();

	if (!pSlotIndexInfo)
	{
		return;
	}

	BYTE btSlotNum = -1;
	btSlotNum = this->GetTempCardShowSlot(pSlotIndexInfo);

	if (btSlotNum == (BYTE)-1)
	{
		this->GCSendMsg(aIndex, 2, 0);
		return;
	}

	if (btSlotNum == (BYTE)-2)
	{
		this->GCSendMsg(aIndex, 3, 0);
		return;
	}

	_tagMuRummyCardInfo stCardInfo[5];
	int iBeforeCardCnt = pMuRummyInfo->GetCardDeckCnt();

	if (this->FillEmptySlotCard(pMuRummyInfo, stCardInfo, lpObj))
	{
		memcpy(pMsg.stMuRummyCardInfo, stCardInfo, sizeof(stCardInfo));
		pMsg.btCardDeckCnt = pMuRummyInfo->GetCardDeckCnt();
		DataSend(aIndex, (LPBYTE)&pMsg, pMsg.h.size);
	}

	#endif
}

void CMuRummyMng::CGReqCardMove(_tagPMSG_REQ_CARD_MOVE* lpMsg, int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	_tagPMSG_ANS_CARD_MOVE pMsg;
	pMsg.h.set(0x4D, 0x12, sizeof(_tagPMSG_ANS_CARD_MOVE));

	if (!OBJECT_RANGE(aIndex))
	{
		return;
	}

	if (!gObjIsConnected(aIndex))
	{
		return;
	}

	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->CloseType != -1) 
	{
		return;
	}

	int sSlot = lpMsg->sSlot;
	int tSlot = lpMsg->tSlot;

	if (sSlot < 0 || sSlot >= MURUMMY_MAX_SLOT)
	{
		return;
	}

	if (tSlot < 0 || tSlot >= MURUMMY_MAX_SLOT)
	{
		return;
	}

	if (sSlot == tSlot)
	{
		return;
	}

	CMuRummyInfo* pCMuRummyInfo = lpObj->MuRummyInfo;

	if (!pCMuRummyInfo)
	{
		return;
	}

	CMuRummyCardInfo pCOutCardInfo;
	BYTE Ret = this->CardSlotMove(pCMuRummyInfo, sSlot, tSlot, &pCOutCardInfo, lpObj);

	if (Ret == (BYTE)-1)
	{
		return;
	}
	else if (Ret == (BYTE)-2)
	{
		sSlot = (BYTE)-1;
	}

	pMsg.sSlot = sSlot;
	pMsg.tSlot = pCOutCardInfo.GetSlotNum();
	pMsg.btColor = pCOutCardInfo.GetColor();
	pMsg.btNumber = pCOutCardInfo.GetNumber();
	DataSend(aIndex, (LPBYTE)&pMsg, pMsg.h.size);

	#endif
}

void CMuRummyMng::CGReqCardReMove(_tagPMSG_REQ_CARD_REMOVE* lpMsg, int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	_tagPMSG_ANS_CARD_REMOVE pMsg;
	pMsg.h.set(0x4D, 0x13, sizeof(_tagPMSG_ANS_CARD_REMOVE));

	if (!OBJECT_RANGE(aIndex))
	{
		return;
	}

	if (!gObjIsConnected(aIndex))
	{
		return;
	}

	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->CloseType != -1) 
	{
		return;
	}

	int sSlot = lpMsg->btSlot;

	if (sSlot >= MURUMMY_MAX_SLOT)
	{
		return;
	}

	CMuRummyInfo* pCMuRummyInfo = lpObj->MuRummyInfo;

	if (!pCMuRummyInfo)
	{
		return;
	}

	BYTE Ret = this->CardSlotReMove(pCMuRummyInfo, sSlot);

	if (Ret == (BYTE)-1)
	{
		return;
	}

	CMuRummyCardInfo* pCCardInfo = pCMuRummyInfo->GetCardInfo();

	if (!pCCardInfo)
	{
		return;
	}

	this->GDReqCardInfoUpdate(lpObj, &pCCardInfo[Ret], Ret);
	pMsg.btResult = sSlot;
	DataSend(aIndex, (LPBYTE)&pMsg, pMsg.h.size);

	this->GCSendMsg(aIndex, 4, 0);

	#endif
}

void CMuRummyMng::CGReqCardMatch(_tagPMSG_REQ_CARD_MATCH* lpMsg, int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	_tagPMSG_ANS_CARD_MATCH pMsg;
	pMsg.h.set(0x4D, 0x14, sizeof(_tagPMSG_ANS_CARD_MATCH));

	if (!OBJECT_RANGE(aIndex))
	{
		return;
	}

	if (!gObjIsConnected(aIndex))
	{
		return;
	}

	LPOBJ lpObj = &gObj[aIndex];
	CMuRummyInfo* pCMuRummyInfo = lpObj->MuRummyInfo;

	if (!pCMuRummyInfo)
	{
		return;
	}

	int nScore = 0;
	_tagMuRummyCardUpdateDS stCardUpdateDS[3];
	BYTE btRet = this->CardMatchCheck(pCMuRummyInfo, nScore, stCardUpdateDS, lpObj);
	CMuRummyCardInfo* pCCardInfo = pCMuRummyInfo->GetCardInfo();

	if (!pCCardInfo)
	{
		return;
	}

	if (btRet)
	{
		this->GCSendMsg(aIndex, 6, 0);
	}
	else
	{
		pCMuRummyInfo->SetScore(nScore + pCMuRummyInfo->GetScore());

		pMsg.btResult = 1;
		pMsg.btScoreH = SET_NUMBERHB(nScore);
		pMsg.btScoreL = SET_NUMBERLB(nScore);
		pMsg.btTotScoreH = SET_NUMBERHB(pCMuRummyInfo->GetScore());
		pMsg.btTotScoreL = SET_NUMBERLB(pCMuRummyInfo->GetScore());
		DataSend(aIndex, (LPBYTE)&pMsg, pMsg.h.size);

		this->GCSendMsg(aIndex, 5, 0);
		this->GDReqScoreUpdate(lpObj, pCMuRummyInfo->GetScore(), stCardUpdateDS);
	}

	this->GCSendCardList(aIndex);

	#endif
}

void CMuRummyMng::CGReqMuRummyEnd(_tagPMSG_REQ_MURUMMY_END* lpMsg, int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	if (!OBJECT_RANGE(aIndex))
	{
		return;
	}

	if (!gObjIsConnected(aIndex))
	{
		return;
	}

	LPOBJ lpObj = &gObj[aIndex];

	if (lpObj->CloseType != -1) 
	{
		return;
	}

	CMuRummyInfo* pCMuRummyInfo = lpObj->MuRummyInfo;

	if (!pCMuRummyInfo)
	{
		return;
	}

	if (pCMuRummyInfo->IsWaitReward())
	{
		return;
	}

	if (pCMuRummyInfo->GetScore() >= this->GetRewardScore1() && !gItemManager.CheckItemInventorySpace(lpObj, 2, 4))
	{
		this->GCSendMsg(aIndex, 7, 0);
		return;
	}

	if (pCMuRummyInfo->GetScore() >= this->GetRewardScore1())
	{
		if (pCMuRummyInfo->GetScore() >= this->GetRewardScore2())
		{
			CItem item;

			if(gItemBagManager.GetItemBySpecialValue(ITEM_BAG_MU_RUMMY2,lpObj,&item) == 0)
			{
				this->GCSendMsg(aIndex, 7, 0);
				return;
			}

			GDCreateItemSend(lpObj->Index,0xEB,0,0,item.m_Index,(BYTE)item.m_Level,0,item.m_Option1,item.m_Option2,item.m_Option3,-1,item.m_NewOption,0,0,0,item.m_SocketOption,item.m_SocketOptionBonus,0);

			this->GCSendMsg(aIndex, 9, item.m_Index);

			_tagPMSG_ANS_MURUMMY_END pMsg;
			pMsg.btResult = 1;
			pMsg.h.set(0x4D, 0x15, sizeof(_tagPMSG_ANS_MURUMMY_END));
			DataSend(aIndex, (LPBYTE)&pMsg, pMsg.h.size);

			this->GCSendMsg(aIndex, 8, 0);
			pCMuRummyInfo->Clear();
			this->GDReqScoreDelete(lpObj);
		}
		else
		{
			CItem item;

			if(gItemBagManager.GetItemBySpecialValue(ITEM_BAG_MU_RUMMY1,lpObj,&item) == 0)
			{
				this->GCSendMsg(aIndex, 7, 0);
				return;
			}

			GDCreateItemSend(lpObj->Index,0xEB,0,0,item.m_Index,(BYTE)item.m_Level,0,item.m_Option1,item.m_Option2,item.m_Option3,-1,item.m_NewOption,0,0,0,item.m_SocketOption,item.m_SocketOptionBonus,0);

			this->GCSendMsg(aIndex, 9, item.m_Index);

			_tagPMSG_ANS_MURUMMY_END pMsg;
			pMsg.btResult = 1;
			pMsg.h.set(0x4D, 0x15, sizeof(_tagPMSG_ANS_MURUMMY_END));
			DataSend(aIndex, (LPBYTE)&pMsg, pMsg.h.size);

			this->GCSendMsg(aIndex, 8, 0);
			pCMuRummyInfo->Clear();
			this->GDReqScoreDelete(lpObj);
		}
	}
	else
	{
		if (!gObjCheckMaxMoney(aIndex, gServerInfo.m_MuRummyRewardMoneyAmount))
		{
			this->GCSendMsg(aIndex, 11, 0);
			return;
		}

		this->GCSendMsg(aIndex, 10, 0);
		gObj[aIndex].Money += gServerInfo.m_MuRummyRewardMoneyAmount;
		GCMoneySend(aIndex, gObj[aIndex].Money);

		_tagPMSG_ANS_MURUMMY_END pMsg;
		pMsg.btResult = 1;
		pMsg.h.set(0x4D, 0x15, sizeof(_tagPMSG_ANS_MURUMMY_END));
		DataSend(aIndex, (LPBYTE)&pMsg, pMsg.h.size);

		this->GCSendMsg(aIndex, 8, 0);
		pCMuRummyInfo->Clear();
		this->GDReqScoreDelete(lpObj);
	}

	#endif
}

void CMuRummyMng::GCSendCardList(int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	_tagPMSG_SEND_SLOTCARD_LIST pMsg;
	pMsg.h.set(0x4D, 0x16, sizeof(_tagPMSG_SEND_SLOTCARD_LIST));

	if (!OBJECT_RANGE(aIndex))
	{
		return;
	}

	if (!gObjIsConnected(aIndex))
	{
		return;
	}

	LPOBJ lpObj = &gObj[aIndex];
	CMuRummyInfo* pCMuRummyInfo = lpObj->MuRummyInfo;

	if (!pCMuRummyInfo)
	{
		return;
	}

	CMuRummyCardInfo* pCCardInfo = pCMuRummyInfo->GetCardInfo();

	if (!pCCardInfo)
	{
		return;
	}

	LPBYTE pSlotIndexInfo = pCMuRummyInfo->GetSlotIndexInfo();

	if (!pSlotIndexInfo)
	{
		return;
	}

	int nSlotNum = 0;

	for (int i = 0; i < MURUMMY_MAX_SLOT; i++)
	{
		if (pSlotIndexInfo[i] != (BYTE)-1)
		{
			if (nSlotNum < 0 || nSlotNum > 4)
			{
				return;
			}

			pMsg.stMuRummyCardInfo[nSlotNum].btColor = pCCardInfo[pSlotIndexInfo[i]].GetColor();
			pMsg.stMuRummyCardInfo[nSlotNum].btNumber = pCCardInfo[pSlotIndexInfo[i]].GetNumber();
			pMsg.stMuRummyCardInfo[nSlotNum].btSlotNum = pCCardInfo[pSlotIndexInfo[i]].GetSlotNum();
			nSlotNum++;
		}
	}

	DataSend(aIndex, (LPBYTE)&pMsg, pMsg.h.size);

	#endif
}

void CMuRummyMng::GCSendMsg(int aIndex, BYTE btNotiIndex, int iValue) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	_tagPMSG_SEND_MURUMMY_MSG pMsg;
	pMsg.h.set(0x4D, 0x17, sizeof(_tagPMSG_SEND_MURUMMY_MSG));

	if (!OBJECT_RANGE(aIndex))
	{
		return;
	}

	if (!gObjIsConnected(aIndex))
	{
		return;
	}

	pMsg.btNotiIndex = btNotiIndex;
	pMsg.btValueH = SET_NUMBERHB(iValue);
	pMsg.btValueL = SET_NUMBERLB(iValue);

	DataSend(aIndex, (LPBYTE)&pMsg, pMsg.h.size);

	#endif
}

void CMuRummyMng::GDReqCardInfo(LPOBJ obj) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	if (!OBJECT_RANGE(obj->Index))
	{
		return;
	}

	if (!gObjIsConnected(obj->Index))
	{
		return;
	}

	if (!this->IsMuRummyEventOn())
	{
		return;
	}

	_tagPMSG_REQ_MURUMMY_SELECT_DS pMsg;
	pMsg.h.set(0x12, 0x00, sizeof(_tagPMSG_REQ_MURUMMY_SELECT_DS));

	memcpy(pMsg.AccountID, obj->Account, sizeof(pMsg.AccountID));
	memcpy(pMsg.Name, obj->Name, sizeof(pMsg.Name));
	pMsg.aIndex = obj->Index;

	gDataServerConnection.DataSend((BYTE*)&pMsg, pMsg.h.size);

	#endif
}

void CMuRummyMng::GDAnsCardInfo(_tagPMSG_ANS_MURUMMY_SELECT_DS* lpMsg) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	int aIndex = lpMsg->aIndex;

	if (!OBJECT_RANGE(aIndex))
	{
		return;
	}

	if (!gObjIsConnected(aIndex))
	{
		return;
	}

	LPOBJ lpObj = &gObj[aIndex];

	if (!lpObj)
	{
		return;
	}

	CMuRummyInfo* pMuRummyInfo = lpObj->MuRummyInfo;

	if (!pMuRummyInfo)
	{
		return;
	}

	CMuRummyCardInfo* pCCardInfo = pMuRummyInfo->GetCardInfo();

	if (!pCCardInfo)
	{
		return;
	}

	LPBYTE pSlotIndexInfo = pMuRummyInfo->GetSlotIndexInfo();

	if (!pSlotIndexInfo)
	{
		return;
	}

	if (!this->IsMuRummyEventOn())
	{
		return;
	}

	_tagPMSG_SEND_MURUMMYEVENT_ON pMsg;
	pMsg.h.set(0x4D, 0x18, sizeof(_tagPMSG_SEND_MURUMMYEVENT_ON));

	if (!lpMsg->btResult)
	{
		pMuRummyInfo->SetGetPlayCard(false);
		pMuRummyInfo->SetMuRummyInfoLoad(true);
		DataSend(aIndex, (LPBYTE)&pMsg, pMsg.h.size);
		return;
	}

	pMuRummyInfo->SetScore(lpMsg->wScore);

	for (int i = 0; i < MURUMMY_MAX_SLOT; i++)
	{
		pSlotIndexInfo[i] = (BYTE)-1;
	}

	int iCardCnt = 0;

	for (int nCardArr = 0; nCardArr < MURUMMY_MAX_CARD; nCardArr++)
	{
		int nArr = lpMsg->stMuRummyCardInfoDS[nCardArr].btSeq;
		int iSlotNum = lpMsg->stMuRummyCardInfoDS[nCardArr].btSlotNum;

		pCCardInfo[nArr].SetColor(lpMsg->stMuRummyCardInfoDS[nCardArr].btColor);
		pCCardInfo[nArr].SetNumber(lpMsg->stMuRummyCardInfoDS[nCardArr].btNumber);
		pCCardInfo[nArr].SetState(lpMsg->stMuRummyCardInfoDS[nCardArr].btStatus);
		pCCardInfo[nArr].SetSlotNum(iSlotNum);

		if (iSlotNum >= 0 && iSlotNum < MURUMMY_MAX_SLOT && pCCardInfo[nArr].GetState() == 1 )
		{
			pSlotIndexInfo[iSlotNum] = nArr;
		}
		 
		if (!pCCardInfo[nArr].GetState())
		{
			iCardCnt++;
		}
	}

	pMuRummyInfo->SetCardDeckCnt(iCardCnt);
	pMuRummyInfo->SetGetPlayCard(1);
	pMuRummyInfo->SetMuRummyInfoLoad(1);
	DataSend(aIndex, (LPBYTE)&pMsg, pMsg.h.size);

	#endif
}

void CMuRummyMng::GDReqCardInfoInsert(LPOBJ obj) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	if (!OBJECT_RANGE(obj->Index))
	{
		return;
	}

	if (!gObjIsConnected(obj->Index))
	{
		return;
	}

	if (!this->IsMuRummyEventOn())
	{
		return;
	}

	CMuRummyInfo* pMuRummyInfo = obj->MuRummyInfo;

	if (!pMuRummyInfo)
	{
		return;
	}

	if (!pMuRummyInfo->IsGetPlayCard())
	{
		return;
	}

	CMuRummyCardInfo* pCCardInfo = pMuRummyInfo->GetCardInfo();

	if (!pCCardInfo)
	{
		return;
	}

	LPBYTE pSlotIndexInfo = pMuRummyInfo->GetSlotIndexInfo();

	if (!pSlotIndexInfo)
	{
		return;
	}

	_tagPMSG_REQ_MURUMMY_INSERT_DS pMsg;
	pMsg.h.set(0x12, 0x30, sizeof(_tagPMSG_REQ_MURUMMY_INSERT_DS));

	memcpy(pMsg.AccountID, obj->Account, sizeof(pMsg.AccountID));
	memcpy(pMsg.Name, obj->Name, sizeof(pMsg.Name));
	pMsg.aIndex = obj->Index;

	for (int i = 0; i < MURUMMY_MAX_CARD; i++)
	{
		pMsg.stMuRummyCardInfoDS[i].btColor = pCCardInfo[i].GetColor();
		pMsg.stMuRummyCardInfoDS[i].btNumber = pCCardInfo[i].GetNumber();
		pMsg.stMuRummyCardInfoDS[i].btSlotNum = pCCardInfo[i].GetSlotNum();
		pMsg.stMuRummyCardInfoDS[i].btStatus = pCCardInfo[i].GetState();
		pMsg.stMuRummyCardInfoDS[i].btSeq = i;
	}

	gDataServerConnection.DataSend((BYTE*)&pMsg, pMsg.h.size);

	#endif
}

void CMuRummyMng::GDReqCardInfoUpdate(LPOBJ obj, CMuRummyCardInfo* pCCardInfo, int iSeq) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	_tagPMSG_REQ_MURUMMY_UPDATE_DS pMsg;
	pMsg.h.set(0x12, 0x31, sizeof(_tagPMSG_REQ_MURUMMY_INSERT_DS));
	memcpy(pMsg.AccountID, obj->Account, sizeof(pMsg.AccountID));
	memcpy(pMsg.Name, obj->Name, sizeof(pMsg.Name));
	pMsg.btSlotNum = pCCardInfo->GetSlotNum();
	pMsg.btStatus = pCCardInfo->GetState();
	gDataServerConnection.DataSend((BYTE*)&pMsg, pMsg.h.size);

	#endif
}

void CMuRummyMng::GDReqScoreUpdate(LPOBJ obj, WORD wScore, _tagMuRummyCardUpdateDS* pCardUpdateDS) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	_tagPMSG_REQ_MURUMMY_SCORE_UPDATE_DS pMsg;
	pMsg.h.set(0x12, 0x32, sizeof(_tagPMSG_REQ_MURUMMY_SCORE_UPDATE_DS));
	memcpy(pMsg.AccountID, obj->Account, sizeof(pMsg.AccountID));
	memcpy(pMsg.Name, obj->Name, sizeof(pMsg.Name));
	memcpy(pMsg.stCardUpdateDS, &pCardUpdateDS->btSlotNum, sizeof(pMsg.stCardUpdateDS));
	pMsg.wScore = wScore;
	gDataServerConnection.DataSend((BYTE*)&pMsg, pMsg.h.size);

	#endif
}

void CMuRummyMng::GDReqScoreDelete(LPOBJ obj) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	_tagPMSG_REQ_MURUMMY_DELETE_DS pMsg;
	pMsg.h.set(0x12, 0x33, sizeof(_tagPMSG_REQ_MURUMMY_SCORE_UPDATE_DS));
	memcpy(pMsg.AccountID, obj->Account, sizeof(pMsg.AccountID));
	memcpy(pMsg.Name, obj->Name, sizeof(pMsg.Name));
	gDataServerConnection.DataSend((BYTE*)&pMsg, pMsg.h.size);

	#endif
}

void CMuRummyMng::GDReqSlotInfoUpdate(LPOBJ obj, BYTE btSeq, BYTE btSlotNum, BYTE btStatus) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	_tagPMSG_REQ_MURUMMY_SLOTUPDATE_DS pMsg;
	pMsg.h.set(0x12, 0x34, sizeof(_tagPMSG_REQ_MURUMMY_SLOTUPDATE_DS));

	if (btSlotNum < 0 || btSlotNum > 4)
	{
		return;
	}

	pMsg.stCardUpdateDS.btSeq = btSeq;
	pMsg.stCardUpdateDS.btStatus = btStatus;
	pMsg.stCardUpdateDS.btSlotNum = btSlotNum;
	memcpy(pMsg.AccountID, obj->Account, sizeof(pMsg.AccountID));
	memcpy(pMsg.Name, obj->Name, sizeof(pMsg.Name));
	gDataServerConnection.DataSend((BYTE*)&pMsg, pMsg.h.size);

	#endif
}

void CMuRummyMng::GDReqMuRummyInfoUpdate(LPOBJ obj) // OK
{
	#if(GAMESERVER_UPDATE>=802)

	if (!OBJECT_RANGE(obj->Index))
	{
		return;
	}

	if (!gObjIsConnected(obj->Index))
	{
		return;
	}

	if (!this->IsMuRummyEventOn())
	{
		return;
	}

	CMuRummyInfo* pMuRummyInfo = obj->MuRummyInfo;

	if (!pMuRummyInfo)
	{
		return;
	}

	if (!pMuRummyInfo->IsGetPlayCard())
	{
		return;
	}

	CMuRummyCardInfo* pCCardInfo = pMuRummyInfo->GetCardInfo();

	if (!pCCardInfo)
	{
		return;
	}

	_tagPMSG_REQ_MURUMMY_INFO_UPDATE_DS pMsg;
	pMsg.h.set(0x12, 0x35, sizeof(_tagPMSG_REQ_MURUMMY_INFO_UPDATE_DS));
	memcpy(pMsg.AccountID, obj->Account, sizeof(pMsg.AccountID));
	memcpy(pMsg.Name, obj->Name, sizeof(pMsg.Name));
	pMsg.wScore = pMuRummyInfo->GetScore();

	for (int i = 0; i < MURUMMY_MAX_CARD; i++)
	{
		pMsg.stMuRummyCardUpdateDS[i].btSlotNum = pCCardInfo[i].GetSlotNum();
		pMsg.stMuRummyCardUpdateDS[i].btStatus = pCCardInfo[i].GetState();
		pMsg.stMuRummyCardUpdateDS[i].btSeq = i;
	}

	gDataServerConnection.DataSend((BYTE*)&pMsg, pMsg.h.size);

	#endif
}
