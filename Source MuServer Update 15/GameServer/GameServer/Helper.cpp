// Helper.cpp: implementation of the CHelper class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Helper.h"
#include "CastleSiege.h"
#include "GameMain.h"
#include "Map.h"
#include "MapManager.h"
#include "Message.h"
#include "Notice.h"
#include "ServerInfo.h"
#include "Util.h"

CHelper gHelper;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CHelper::CHelper() // OK
{

}

CHelper::~CHelper() // OK
{

}

void CHelper::MainProc() // OK
{
	#if(GAMESERVER_UPDATE>=603)

	for(int n=OBJECT_START_USER;n < MAX_OBJECT;n++)
	{
		if(gObjIsConnected(n) == 0)
		{
			continue;
		}

		LPOBJ lpObj = &gObj[n];

		if(lpObj->HelperDelayTime != 0)
		{
			int HelperStage = 0;

			if(gMapManager.GetMapHelperEnable(lpObj->Map) == 0)
			{
				this->DisableHelper(lpObj);
				continue;
			}

			if(gMap[lpObj->Map].CheckAttr(lpObj->X,lpObj->Y,1) != 0)
			{
				this->DisableHelper(lpObj);
				continue;
			}

			if((HelperStage=((GetTickCount()-lpObj->HelperTotalTime)/12000000)) >= MAX_HELPER_STAGE)
			{
				this->DisableHelper(lpObj);
				continue;
			}

			if((GetTickCount()-lpObj->HelperDelayTime) >= ((DWORD)(gServerInfo.m_HelperActiveDelay*60000)))
			{
				if(lpObj->Money < ((DWORD)((lpObj->Level+lpObj->MasterLevel)*gServerInfo.m_HelperActiveMoney[HelperStage])))
				{
					this->DisableHelper(lpObj);
					continue;
				}

				lpObj->HelperDelayTime = GetTickCount();

				lpObj->Money -= (lpObj->Level+lpObj->MasterLevel)*gServerInfo.m_HelperActiveMoney[HelperStage];

				GCMoneySend(lpObj->Index,lpObj->Money);

				this->GCHelperStartSend(lpObj->Index,((GetTickCount()-lpObj->HelperTotalTime)/60000),((lpObj->Level+lpObj->MasterLevel)*gServerInfo.m_HelperActiveMoney[HelperStage]),0);
			}
		}
	}

	#endif
}

void CHelper::DisableHelper(LPOBJ lpObj) // OK
{
	#if(GAMESERVER_UPDATE>=603)

	if(lpObj->HelperDelayTime == 0)
	{
		return;
	}

	lpObj->HelperDelayTime = 0;

	lpObj->HelperTotalTime = 0;

	this->GCHelperStartSend(lpObj->Index,0,0,1);

	#endif
}

void CHelper::CGHelperDataRecv(PMSG_HELPER_DATA_RECV* lpMsg,int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=603)

	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnected(aIndex) == 0)
	{
		return;
	}

	this->GDHelperDataSaveSend(aIndex,lpMsg->data);

	#endif
}

void CHelper::CGHelperStartRecv(PMSG_HELPER_START_RECV* lpMsg,int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=603)

	if(gServerInfo.m_HelperSwitch == 0)
	{
		return;
	}

	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	if(lpObj->Level < gServerInfo.m_HelperActiveLevel)
	{
		return;
	}

	if(gMapManager.GetMapHelperEnable(lpObj->Map) == 0)
	{
		return;
	}

	if(gMap[lpObj->Map].CheckAttr(lpObj->X,lpObj->Y,1) != 0)
	{
		return;
	}

	//Fix helper + custom attack
	if(lpObj->AttackCustom == 1)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(764));
		return;
	}

	if(lpMsg->type == 0)
	{
		lpObj->HelperDelayTime = GetTickCount();

		lpObj->HelperTotalTime = GetTickCount();

		this->GCHelperStartSend(aIndex,0,0,0);
	}
	else
	{
		lpObj->HelperDelayTime = 0;

		lpObj->HelperTotalTime = 0;

		this->GCHelperStartSend(aIndex,0,0,1);
	}

	#endif
}

void CHelper::GCHelperStartSend(int aIndex,int time,int money,int result) // OK
{
	#if(GAMESERVER_UPDATE>=603)

	PMSG_HELPER_START_SEND pMsg;

	pMsg.header.set(0xBF,0x51,sizeof(pMsg));

	pMsg.time = time;

	pMsg.money = money;

	pMsg.result = result;

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CHelper::DGHelperDataRecv(SDHP_HELPER_DATA_RECV* lpMsg) // OK
{
	#if(GAMESERVER_UPDATE>=603)

	if(gObjIsAccountValid(lpMsg->index,lpMsg->account) == 0)
	{
		LogAdd(LOG_RED,"[DGHelperDataRecv] Invalid Account [%d](%s)",lpMsg->index,lpMsg->account);
		CloseClient(lpMsg->index);
		return;
	}

	PMSG_HELPER_DATA_SEND pMsg;

	pMsg.header.set(0xAE,sizeof(pMsg));

	pMsg.result = lpMsg->result;

	memcpy(pMsg.data,lpMsg->data,sizeof(pMsg.data));

	DataSend(lpMsg->index,(BYTE*)&pMsg,sizeof(pMsg));

	#endif
}

void CHelper::GDHelperDataSend(int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=603)

	if(gObjIsAccountValid(aIndex,gObj[aIndex].Account) == 0)
	{
		return;
	}

	SDHP_HELPER_DATA_SEND pMsg;

	pMsg.header.set(0x17,0x00,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,gObj[aIndex].Account,sizeof(pMsg.account));

	memcpy(pMsg.name,gObj[aIndex].Name,sizeof(pMsg.name));

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CHelper::GDHelperDataSaveSend(int aIndex,BYTE* data) // OK
{
	#if(GAMESERVER_UPDATE>=603)

	SDHP_HELPER_DATA_SAVE_SEND pMsg;

	pMsg.header.set(0x17,0x30,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,gObj[aIndex].Account,sizeof(pMsg.account));

	memcpy(pMsg.name,gObj[aIndex].Name,sizeof(pMsg.name));

	memcpy(pMsg.data,data,sizeof(pMsg.data));

	gDataServerConnection.DataSend((BYTE*)&pMsg,sizeof(pMsg));

	#endif
}
