// MiningSystem.cpp: implementation of the CMiningSystem class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MiningSystem.h"
#include "DSProtocol.h"
#include "ItemManager.h"
#include "JewelMix.h"
#include "MemScript.h"
#include "Message.h"
#include "Notice.h"
#include "ObjectManager.h"
#include "Util.h"

CMiningSystem gMiningSystem;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMiningSystem::CMiningSystem() // OK
{
	#if(GAMESERVER_UPDATE>=801)

	this->m_MiningInfo.clear();

	#endif
}

CMiningSystem::~CMiningSystem() // OK
{

}

void CMiningSystem::Load(char* path) // OK
{
	#if(GAMESERVER_UPDATE>=801)

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

	this->m_MiningInfo.clear();

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

			MINING_INFO info;

			info.Index = lpMemScript->GetNumber();

			info.Stage = lpMemScript->GetAsNumber();

			info.Value = lpMemScript->GetAsNumber();

			info.ItemRate = lpMemScript->GetAsNumber();

			info.ItemIndex = lpMemScript->GetAsNumber();

			info.ItemCount = lpMemScript->GetAsNumber();

			info.BonusItemRate = lpMemScript->GetAsNumber();

			info.BonusItemIndex = lpMemScript->GetAsNumber();

			info.BonusItemCount = lpMemScript->GetAsNumber();

			this->m_MiningInfo.push_back(info);
		}
	}
	catch(...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;

	#endif
}

bool CMiningSystem::GetMiningInfo(int index,int stage,MINING_INFO* lpInfo) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	for(std::vector<MINING_INFO>::iterator it=this->m_MiningInfo.begin();it != this->m_MiningInfo.end();it++)
	{
		if(it->Index == index)
		{
			if(it->Stage == stage)
			{
				(*lpInfo) = (*it);
				return 1;
			}
		}
	}

	return 0;

	#else

	return 0;

	#endif
}

void CMiningSystem::CreateMiningReward(int aIndex,int ItemIndex,int PackIndex,int ItemCount) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	int value = ItemCount;

	int table[4] = {0,0,0,0};

	table[3] = value/30;
	value = value%30;

	table[2] = value/20;
	value = value%20;

	table[1] = value/10;
	value = value%10;

	table[0] = value;
	value = 0;

	for(int n=0;n < table[3];n++)
	{
		GDCreateItemSend(aIndex,0xEB,0,0,PackIndex,2,0,0,0,0,aIndex,0,0,0,0,0,0xFF,0);
	}

	for(int n=0;n < table[2];n++)
	{
		GDCreateItemSend(aIndex,0xEB,0,0,PackIndex,1,0,0,0,0,aIndex,0,0,0,0,0,0xFF,0);
	}

	for(int n=0;n < table[1];n++)
	{
		GDCreateItemSend(aIndex,0xEB,0,0,PackIndex,0,0,0,0,0,aIndex,0,0,0,0,0,0xFF,0);
	}

	for(int n=0;n < table[0];n++)
	{
		GDCreateItemSend(aIndex,0xEB,0,0,ItemIndex,0,0,0,0,0,aIndex,0,0,0,0,0,0xFF,0);
	}

	#endif
}

void CMiningSystem::CGMiningStartRecv(PMSG_MINING_START_RECV* lpMsg,int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	if(lpMsg->index != aIndex)
	{
		return;
	}

	if(gObjIsConnected(lpMsg->MonsterIndex) == 0)
	{
		return;
	}

	MINING_INFO MiningInfo;

	if(this->GetMiningInfo(lpMsg->MonsterClass,(lpMsg->stage+1),&MiningInfo) == 0)
	{
		this->GCMiningStartSend(aIndex,lpMsg->MonsterIndex,lpMsg->MonsterClass,lpMsg->map,lpMsg->stage,0,3);
		return;
	}

	if(gObj[lpMsg->MonsterIndex].Live == 0)
	{
		this->GCMiningStartSend(aIndex,lpMsg->MonsterIndex,lpMsg->MonsterClass,lpMsg->map,lpMsg->stage,0,3);
		return;
	}

	if(gObj[lpMsg->MonsterIndex].Map != lpObj->Map || gObj[lpMsg->MonsterIndex].Map != lpMsg->map)
	{
		this->GCMiningStartSend(aIndex,lpMsg->MonsterIndex,lpMsg->MonsterClass,lpMsg->map,lpMsg->stage,0,3);
		return;
	}

	if(OBJECT_RANGE(gObj[lpMsg->MonsterIndex].MiningIndex) != 0 && gObj[lpMsg->MonsterIndex].MiningIndex != aIndex)
	{
		this->GCMiningStartSend(aIndex,lpMsg->MonsterIndex,lpMsg->MonsterClass,lpMsg->map,lpMsg->stage,0,8);
		return;
	}

	if(lpObj->Change >= 0)
	{
		this->GCMiningStartSend(aIndex,lpMsg->MonsterIndex,lpMsg->MonsterClass,lpMsg->map,lpMsg->stage,0,10);
		return;
	}

	if(gItemManager.GetInventoryEmptySlotCount(lpObj) < MAX_MINING_JEWEL_PACK)
	{
		this->GCMiningStartSend(aIndex,lpMsg->MonsterIndex,lpMsg->MonsterClass,lpMsg->map,lpMsg->stage,0,11);
		return;
	}

	if(lpMsg->stage == 0 && (lpObj->Interface.use != 0))
	{
		this->GCMiningStartSend(aIndex,lpMsg->MonsterIndex,lpMsg->MonsterClass,lpMsg->map,lpMsg->stage,0,12);
		return;
	}

	if(lpMsg->stage != 0 && (lpObj->Interface.use == 0 || lpObj->Interface.type != INTERFACE_MINING))
	{
		this->GCMiningStartSend(aIndex,lpMsg->MonsterIndex,lpMsg->MonsterClass,lpMsg->map,lpMsg->stage,0,12);
		return;
	}

	if(lpObj->PShopOpen != 0)
	{
		this->GCMiningStartSend(aIndex,lpMsg->MonsterIndex,lpMsg->MonsterClass,lpMsg->map,lpMsg->stage,0,13);
		return;
	}

	if(lpObj->MiningStage != lpMsg->stage)
	{
		this->GCMiningStartSend(aIndex,lpMsg->MonsterIndex,lpMsg->MonsterClass,lpMsg->map,lpMsg->stage,0,17);
		return;
	}

	if(OBJECT_RANGE(lpObj->MiningIndex) != 0 && lpObj->MiningIndex != lpMsg->MonsterIndex)
	{
		this->GCMiningStartSend(aIndex,lpMsg->MonsterIndex,lpMsg->MonsterClass,lpMsg->map,lpMsg->stage,0,17);
		return;
	}

	if(lpObj->Inventory[0].IsItem() == 0 || lpObj->Inventory[0].m_Index != GET_ITEM(0,41) || lpObj->Inventory[0].m_Durability < MiningInfo.Value)
	{
		this->GCMiningStartSend(aIndex,lpMsg->MonsterIndex,lpMsg->MonsterClass,lpMsg->map,lpMsg->stage,0,18);
		return;
	}

	if((GetLargeRand()%10000) < MiningInfo.ItemRate)
	{
		lpObj->Inventory[0].m_Durability = (lpObj->Inventory[0].m_Durability-MiningInfo.Value);

		gItemManager.GCItemDurSend(aIndex,0,(BYTE)lpObj->Inventory[0].m_Durability,0);

		lpObj->Interface.use = 1;

		lpObj->Interface.type = INTERFACE_MINING;

		lpObj->Interface.state = 0;

		lpObj->MiningStage++;

		lpObj->MiningIndex = lpMsg->MonsterIndex;

		gObj[lpMsg->MonsterIndex].MiningStage = 1;

		gObj[lpMsg->MonsterIndex].MiningIndex = aIndex;

		this->GCMiningStartSend(aIndex,lpMsg->MonsterIndex,lpMsg->MonsterClass,lpMsg->map,lpObj->MiningStage,MiningInfo.ItemCount,((lpObj->MiningStage==MAX_MINING_STAGE)?1:0));
	}
	else
	{
		lpObj->Inventory[0].m_Durability = (lpObj->Inventory[0].m_Durability-MiningInfo.Value);

		gItemManager.GCItemDurSend(aIndex,0,(BYTE)lpObj->Inventory[0].m_Durability,0);

		lpObj->Interface.use = 1;

		lpObj->Interface.type = INTERFACE_MINING;

		lpObj->Interface.state = 0;

		lpObj->MiningStage = lpObj->MiningStage;

		lpObj->MiningIndex = lpMsg->MonsterIndex;

		gObj[lpMsg->MonsterIndex].MiningStage = 1;

		gObj[lpMsg->MonsterIndex].MiningIndex = aIndex;

		this->GCMiningStartSend(aIndex,lpMsg->MonsterIndex,lpMsg->MonsterClass,lpMsg->map,lpMsg->stage,0,2);
	}

	#endif
}

void CMiningSystem::CGMiningSuccessRecv(PMSG_MINING_SUCCESS_RECV* lpMsg,int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	if(lpMsg->index != aIndex)
	{
		return;
	}

	if(gObjIsConnected(lpMsg->MonsterIndex) == 0)
	{
		return;
	}

	if(lpObj->Interface.use == 0 || lpObj->Interface.type != INTERFACE_MINING)
	{
		return;
	}

	if(OBJECT_RANGE(lpObj->MiningIndex) == 0 || lpObj->MiningIndex != lpMsg->MonsterIndex)
	{
		return;
	}

	if(OBJECT_RANGE(gObj[lpMsg->MonsterIndex].MiningIndex) == 0 || gObj[lpMsg->MonsterIndex].MiningIndex != aIndex)
	{
		return;
	}

	MINING_INFO MiningInfo;

	if(this->GetMiningInfo(lpMsg->MonsterClass,lpObj->MiningStage,&MiningInfo) == 0)
	{
		this->GCMiningSuccessSend(aIndex,lpMsg->MonsterIndex,lpMsg->MonsterClass,lpMsg->map,lpMsg->stage,0);
		return;
	}

	if(lpObj->MiningStage == MAX_MINING_STAGE && lpObj->MiningStage != (lpMsg->stage+1))
	{
		this->GCMiningSuccessSend(aIndex,lpMsg->MonsterIndex,lpMsg->MonsterClass,lpMsg->map,lpMsg->stage,0);
		return;
	}

	if(lpObj->MiningStage != MAX_MINING_STAGE && lpObj->MiningStage != (lpMsg->stage+0))
	{
		this->GCMiningSuccessSend(aIndex,lpMsg->MonsterIndex,lpMsg->MonsterClass,lpMsg->map,lpMsg->stage,0);
		return;
	}

	for(int n=0;n < MAX_JEWEL_MIX_BUNDLE;n++)
	{
		if(gJewelMix.GetJewelSimpleIndex(n) == MiningInfo.ItemIndex)
		{
			if(lpObj->MiningStage == MAX_MINING_STAGE){GCFireworksSend(lpObj,lpObj->X,lpObj->Y);}
			this->CreateMiningReward(aIndex,MiningInfo.ItemIndex,gJewelMix.GetJewelBundleIndex(n),MiningInfo.ItemCount);
			break;
		}
	}

	if(MiningInfo.BonusItemRate >= 0 && (GetLargeRand()%10000) < MiningInfo.BonusItemRate)
	{
		for(int n=0;n < MAX_JEWEL_MIX_BUNDLE;n++)
		{
			if(gJewelMix.GetJewelSimpleIndex(n) == MiningInfo.BonusItemIndex)
			{
				gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(491),lpObj->Name,MiningInfo.BonusItemCount);
				this->CreateMiningReward(aIndex,MiningInfo.BonusItemIndex,gJewelMix.GetJewelBundleIndex(n),MiningInfo.BonusItemCount);
				break;
			}
		}
	}

	lpObj->Interface.use = 0;

	lpObj->Interface.type = INTERFACE_NONE;

	lpObj->Interface.state = 0;

	lpObj->MiningStage = 0;

	lpObj->MiningIndex = -1;

	gObj[lpMsg->MonsterIndex].Life = 0;

	gObj[lpMsg->MonsterIndex].MiningStage = 0;

	gObj[lpMsg->MonsterIndex].MiningIndex = -1;

	gObjectManager.CharacterLifeCheck(lpObj,&gObj[lpMsg->MonsterIndex],0,0,0,0,0,0);

	this->GCMiningSuccessSend(aIndex,lpMsg->MonsterIndex,lpMsg->MonsterClass,lpMsg->map,lpMsg->stage,3);

	#endif
}

void CMiningSystem::CGMiningFailureRecv(PMSG_MINING_FAILURE_RECV* lpMsg,int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	if(lpMsg->index != aIndex)
	{
		return;
	}

	if(gObjIsConnected(lpMsg->MonsterIndex) == 0)
	{
		return;
	}

	if(lpObj->Interface.use == 0 || lpObj->Interface.type != INTERFACE_MINING)
	{
		return;
	}

	if(OBJECT_RANGE(lpObj->MiningIndex) == 0 || lpObj->MiningIndex != lpMsg->MonsterIndex)
	{
		return;
	}

	if(OBJECT_RANGE(gObj[lpMsg->MonsterIndex].MiningIndex) == 0 || gObj[lpMsg->MonsterIndex].MiningIndex != aIndex)
	{
		return;
	}

	lpObj->Interface.use = 0;

	lpObj->Interface.type = INTERFACE_NONE;

	lpObj->Interface.state = 0;

	lpObj->MiningStage = 0;

	lpObj->MiningIndex = -1;

	gObj[lpMsg->MonsterIndex].Life = 0;

	gObj[lpMsg->MonsterIndex].MiningStage = 0;

	gObj[lpMsg->MonsterIndex].MiningIndex = -1;

	gObjectManager.CharacterLifeCheck(lpObj,&gObj[lpMsg->MonsterIndex],0,0,0,0,0,0);

	#endif
}

void CMiningSystem::GCMiningStartSend(int aIndex,int MonsterIndex,int MonsterClass,int map,int stage,int value,int result) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	PMSG_MINING_START_SEND pMsg;

	pMsg.header.set(0x4C,0x00,sizeof(pMsg));

	pMsg.index = aIndex;

	pMsg.MonsterIndex = MonsterIndex;

	pMsg.MonsterClass = MonsterClass;

	pMsg.map = map;

	pMsg.stage = stage;

	pMsg.value = value;

	pMsg.result = result;

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CMiningSystem::GCMiningSuccessSend(int aIndex,int MonsterIndex,int MonsterClass,int map,int stage,int result) // OK
{
	#if(GAMESERVER_UPDATE>=801)

	PMSG_MINING_SUCCESS_SEND pMsg;

	pMsg.header.set(0x4C,0x01,sizeof(pMsg));

	pMsg.index = aIndex;

	pMsg.MonsterIndex = MonsterIndex;

	pMsg.MonsterClass = MonsterClass;

	pMsg.map = map;

	pMsg.stage = stage;

	pMsg.result = result;

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);

	#endif
}
