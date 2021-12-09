// PcPoint.cpp: implementation of the CPcPoint class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PcPoint.h"
#include "BonusManager.h"
#include "CashShop.h"
#include "EffectManager.h"
#include "GameMain.h"
#include "Map.h"
#include "MemScript.h"
#include "ServerInfo.h"
#include "Util.h"

CPcPoint gPcPoint;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPcPoint::CPcPoint() // OK
{
	#if(GAMESERVER_UPDATE<=402)

	this->Init();

	#endif
}

CPcPoint::~CPcPoint() // OK
{

}

void CPcPoint::Load(char* path) // OK
{
	#if(GAMESERVER_UPDATE<=402)

	CMemScript* lpMemScript = new CMemScript;

	if(lpMemScript == 0)
	{
		ErrorMessageBox(MEM_SCRIPT_ALLOC_ERROR,path);
		return;
	}

	if(lpMemScript->SetBuffer(path) == 0)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
		delete lpMemScript;
		return;
	}

	this->Init();

	try
	{
		while(true)
		{
			if(lpMemScript->GetToken() == TOKEN_END)
			{
				break;
			}

			if(strcmp("end",lpMemScript->GetString()) == 0)
			{
				break;
			}

			int ItemIndex = SafeGetItem(GET_ITEM(lpMemScript->GetNumber(),lpMemScript->GetAsNumber()));

			int ItemLevel = lpMemScript->GetAsNumber();

			int ItemDurability = lpMemScript->GetAsNumber();

			int ItemOption1 = lpMemScript->GetAsNumber();

			int ItemOption2 = lpMemScript->GetAsNumber();

			int ItemOption3 = lpMemScript->GetAsNumber();

			int ItemNewOption = lpMemScript->GetAsNumber();

			int ItemValue = lpMemScript->GetAsNumber();

			int AncOption = lpMemScript->GetAsNumber();

			int JOH	= lpMemScript->GetAsNumber();

			int OpEx = lpMemScript->GetAsNumber();

			int Socket1 = lpMemScript->GetAsNumber();

			int Socket2 = lpMemScript->GetAsNumber();

			int Socket3 = lpMemScript->GetAsNumber();

			int Socket4 = lpMemScript->GetAsNumber();

			int Socket5 = lpMemScript->GetAsNumber();

			this->InsertItemNew(ItemIndex,ItemLevel,ItemDurability,ItemOption1,ItemOption2,ItemOption3,ItemNewOption,AncOption,JOH,OpEx,Socket1,Socket2,Socket3,Socket4,Socket5,ItemValue);

			//this->InsertItem(ItemIndex,ItemLevel,ItemDurability,ItemOption1,ItemOption2,ItemOption3,ItemNewOption,ItemValue);
		}
	}
	catch(...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;

	#endif
}

void CPcPoint::MainProc() // OK
{
	#if(GAMESERVER_UPDATE<=402)

	//for(int n=OBJECT_START_USER;n < MAX_OBJECT;n++)
	//{
	//	if(gObjIsConnectedGP(n) == 0)
	//	{
	//		continue;
	//	}

	//	LPOBJ lpObj = &gObj[n];

	//	if((GetTickCount()-lpObj->OnlineRewardCoin1) >= ((DWORD)gServerInfo.m_OnlineRewardCoin1Delay*60000))
	//	{
	//		lpObj->OnlineRewardCoin1 = GetTickCount();
	//		GDSetCoinSend(lpObj->Index, (it->Value1 < 0)?0:it->Value1, (it->Value2 < 0)?0:it->Value2, (it->Value3 < 0)?0:it->Value3,"OnlineReward");
	//	}

	//	if((GetTickCount()-lpObj->OnlineRewardCoin2) >= ((DWORD)gServerInfo.m_OnlineRewardCoin2Delay*60000))
	//	{
	//		lpObj->OnlineRewardCoin2 = GetTickCount();
	//		GDSetCoinSend(lpObj->Index, gServerInfo.m_OnlineRewardCoin2Value[lpObj->AccountLevel], (it->Value2 < 0)?0:it->Value2, (it->Value3 < 0)?0:it->Value3,"OnlineReward");
	//	}

	//	if((GetTickCount()-lpObj->OnlineRewardCoin3) >= ((DWORD)gServerInfo.m_OnlineRewardCoin3Delay*60000))
	//	{
	//		lpObj->OnlineRewardCoin3 = GetTickCount();
	//		this->GDPcPointAddPointSaveSend(lpObj->Index,gBonusManager.GetBonusValue(lpObj,BONUS_INDEX_GLOBIN_POINT,gServerInfo.m_OnlineRewardCoin3Value[lpObj->AccountLevel],-1,-1,-1,-1));
	//		this->GDPcPointPointSend(lpObj->Index);
	//	}
	//}

	#endif
}

void CPcPoint::CGPcPointItemBuyRecv(PMSG_PC_POINT_ITEM_BUY_RECV* lpMsg,int aIndex) // OK
{
	#if(GAMESERVER_UPDATE<=402)

	if(gServerInfo.m_PcPointSwitch == 0)
	{
		return;
	}

	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	if(((lpObj->PcPointTransaction[1]==0)?(lpObj->PcPointTransaction[1]++):lpObj->PcPointTransaction[1]) != 0)
	{
		return;
	}

	SDHP_PC_POINT_ITEM_BUY_SEND pMsg;

	pMsg.header.set(0x19,0x01,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,lpObj->Account,sizeof(pMsg.account));

	pMsg.slot = lpMsg->slot;

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CPcPoint::CGPcPointOpenRecv(PMSG_PC_POINT_OPEN_RECV* lpMsg,int aIndex) // OK
{
	#if(GAMESERVER_UPDATE<=402)

	if(gServerInfo.m_PcPointSwitch == 0)
	{
		return;
	}

	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	PMSG_PC_POINT_OPEN_SEND pMsg;

	pMsg.header.set(0xD0,0x06,sizeof(pMsg));

	pMsg.result = 0;

	if(lpObj->Interface.use != 0)
	{
		pMsg.result = 3;
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(this->GetItemCount() == 0)
	{
		pMsg.result = 2;
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(gMap[lpObj->Map].CheckAttr(lpObj->X,lpObj->Y,1) == 0)
	{
		pMsg.result = 1;
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	lpObj->Interface.use = 1;

	lpObj->Interface.type = INTERFACE_SHOP;

	lpObj->Interface.state = 0;

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);

	this->GCShopItemListSend(aIndex);

	#endif
}

void CPcPoint::DGPcPointPointRecv(SDHP_PC_POINT_POINT_RECV* lpMsg) // OK
{
	#if(GAMESERVER_UPDATE<=402)

	if(gObjIsAccountValid(lpMsg->index,lpMsg->account) == 0)
	{
		LogAdd(LOG_RED,"[DGPcPointPointRecv] Invalid Account [%d](%s)",lpMsg->index,lpMsg->account);
		CloseClient(lpMsg->index);
		return;
	}

	LPOBJ lpObj = &gObj[lpMsg->index];

	lpObj->PcPointTransaction[0] = 0;

	PMSG_PC_POINT_POINT_SEND pMsg;

	pMsg.header.set(0xD0,0x04,sizeof(pMsg));

	pMsg.MinPcPoint = (WORD)lpMsg->PcPoint;

	pMsg.MaxPcPoint = gServerInfo.m_PcPointMaxPoint;

	pMsg.PcPointType = 0;

	DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CPcPoint::DGPcPointItemBuyRecv(SDHP_PC_POINT_ITEM_BUY_RECV* lpMsg) // OK
{
	#if(GAMESERVER_UPDATE<=402)

	if(gObjIsAccountValid(lpMsg->index,lpMsg->account) == 0)
	{
		LogAdd(LOG_RED,"[DGPcPointItemBuyRecv] Invalid Account [%d](%s)",lpMsg->index,lpMsg->account);
		CloseClient(lpMsg->index);
		return;
	}

	LPOBJ lpObj = &gObj[lpMsg->index];

	lpObj->PcPointTransaction[1] = 0;

	PMSG_PC_POINT_ITEM_BUY_SEND pMsg;

	pMsg.header.set(0xD0,0x05,sizeof(pMsg));

	pMsg.result = 0;

	pMsg.ItemSlot = 0xFF;

	if(lpMsg->result != 0)
	{
		pMsg.result = 7;
		DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(lpObj->Interface.use == 0 || lpObj->Interface.type != INTERFACE_SHOP)
	{
		pMsg.result = 5;
		DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	gObjFixInventoryPointer(lpObj->Index);

	if(lpObj->Transaction == 1)
	{
		pMsg.result = 6;
		DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	CItem item;

	if(this->GetItem(&item,lpMsg->slot) == 0)
	{
		pMsg.result = 8;
		DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(lpMsg->PcPoint < ((DWORD)item.m_PcPointValue))
	{
		pMsg.result = 9;
		DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	EFFECT_INFO* lpInfo = gEffectManager.GetInfoByItem(item.m_Index);

	if(lpInfo == 0)
	{
		if(gItemManager.InventoryInsertItemStack(lpObj,&item) == 0)
		{
			if((pMsg.ItemSlot=gItemManager.InventoryInsertItem(lpObj->Index,item)) == 0xFF)
			{
				pMsg.result = 3;
				DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);
				return;
			}

			this->GDPcPointSubPointSaveSend(lpObj->Index,item.m_PcPointValue);

			this->GDPcPointPointSend(lpObj->Index);

			gItemManager.ItemByteConvert(pMsg.ItemInfo,item);

			DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);
		}
		else
		{
			this->GDPcPointSubPointSaveSend(lpObj->Index,item.m_PcPointValue);

			this->GDPcPointPointSend(lpObj->Index);

			DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);

		}
	}
	else
	{
		if(gEffectManager.GetEffect(lpObj,lpInfo->Index) != 0)
		{
			pMsg.result = 3;
			DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);
			return;
		}

		gEffectManager.AddEffect(lpObj,0,lpInfo->Index,0,0,0,0,0);

		this->GDPcPointSubPointSaveSend(lpObj->Index,item.m_PcPointValue);

		this->GDPcPointPointSend(lpObj->Index);

		DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);

	}

	#endif
}

void CPcPoint::DGPcPointRecievePointRecv(SDHP_PC_POINT_RECIEVE_POINT_RECV* lpMsg) // OK
{
	#if(GAMESERVER_UPDATE<=402)

	if(gObjIsAccountValid(lpMsg->index,lpMsg->account) == 0)
	{
		LogAdd(LOG_RED,"[DGPcPointRecievePointRecv] Invalid Account [%d](%s)",lpMsg->index,lpMsg->account);
		CloseClient(lpMsg->index);
		return;
	}

	LPOBJ lpObj = &gObj[lpMsg->index];

	((void(*)(LPOBJ,DWORD,DWORD,DWORD,DWORD,DWORD))lpMsg->CallbackFunc)(lpObj,lpMsg->CallbackArg1,lpMsg->CallbackArg2,0,0,lpMsg->PcPoint);

	#endif
}

void CPcPoint::GDPcPointPointSend(int aIndex) // OK
{
	#if(GAMESERVER_UPDATE<=402)

	if(gServerInfo.m_PcPointSwitch == 0)
	{
		return;
	}

	if(gObjIsAccountValid(aIndex,gObj[aIndex].Account) == 0)
	{
		return;
	}

	if(((gObj[aIndex].PcPointTransaction[0]==0)?(gObj[aIndex].PcPointTransaction[0]++):gObj[aIndex].PcPointTransaction[0]) != 0)
	{
		return;
	}

	LPOBJ lpObj = &gObj[aIndex];

	SDHP_PC_POINT_POINT_SEND pMsg;

	pMsg.header.set(0x19,0x00,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,lpObj->Account,sizeof(pMsg.account));

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CPcPoint::GDPcPointRecievePointSend(int aIndex,DWORD CallbackFunc,DWORD CallbackArg1,DWORD CallbackArg2) // OK
{
	#if(GAMESERVER_UPDATE<=402)

	if(gObjIsAccountValid(aIndex,gObj[aIndex].Account) == 0)
	{
		return;
	}

	LPOBJ lpObj = &gObj[aIndex];

	SDHP_PC_POINT_RECIEVE_POINT_SEND pMsg;

	pMsg.header.set(0x19,0x02,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,lpObj->Account,sizeof(pMsg.account));

	pMsg.CallbackFunc = CallbackFunc;

	pMsg.CallbackArg1 = CallbackArg1;

	pMsg.CallbackArg2 = CallbackArg2;

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CPcPoint::GDPcPointAddPointSaveSend(int aIndex,DWORD AddPcPoint) // OK
{
	#if(GAMESERVER_UPDATE<=402)

	SDHP_PC_POINT_ADD_POINT_SAVE_SEND pMsg;

	pMsg.header.set(0x19,0x30,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,gObj[aIndex].Account,sizeof(pMsg.account));

	pMsg.AddPcPoint = AddPcPoint;

	pMsg.MaxPcPoint = gServerInfo.m_PcPointMaxPoint;

	gDataServerConnection.DataSend((BYTE*)&pMsg,sizeof(pMsg));

	gObj[aIndex].Coin3 += AddPcPoint;

	gCashShop.GCSendCoin(&gObj[aIndex], gObj[aIndex].Coin1, gObj[aIndex].Coin2, gObj[aIndex].Coin3);

	#endif
}

void CPcPoint::GDPcPointSubPointSaveSend(int aIndex,DWORD SubPcPoint) // OK
{
	#if(GAMESERVER_UPDATE<=402)

	SDHP_PC_POINT_SUB_POINT_SAVE_SEND pMsg;

	pMsg.header.set(0x19,0x31,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,gObj[aIndex].Account,sizeof(pMsg.account));

	pMsg.SubPcPoint = SubPcPoint;

	pMsg.MaxPcPoint = gServerInfo.m_PcPointMaxPoint;

	gDataServerConnection.DataSend((BYTE*)&pMsg,sizeof(pMsg));

	gObj[aIndex].Coin3 -= SubPcPoint;

	gCashShop.GCSendCoin(&gObj[aIndex], gObj[aIndex].Coin1, gObj[aIndex].Coin2, gObj[aIndex].Coin3);

	#endif
}
