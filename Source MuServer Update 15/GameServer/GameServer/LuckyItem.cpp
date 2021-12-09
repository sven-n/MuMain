// LuckyItem.cpp: implementation of the CLuckyItem class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LuckyItem.h"
#include "GameMain.h"
#include "ItemManager.h"
#include "ItemOptionRate.h"
#include "JewelOfHarmonyOption.h"
#include "MemScript.h"
#include "ObjectManager.h"
#include "ServerInfo.h"
#include "Util.h"

CLuckyItem gLuckyItem;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLuckyItem::CLuckyItem() // OK
{

}

CLuckyItem::~CLuckyItem() // OK
{

}

void CLuckyItem::Load(char* path) // OK
{
	#if(GAMESERVER_UPDATE>=602)

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

	this->m_LuckyItemInfo.clear();

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

			LUCKY_ITEM_INFO info;

			info.Index = lpMemScript->GetNumber();

			info.Group = lpMemScript->GetAsNumber();

			info.Decay = lpMemScript->GetAsNumber();

			info.Option0 = lpMemScript->GetAsNumber();

			info.Option1 = lpMemScript->GetAsNumber();

			info.Option2 = lpMemScript->GetAsNumber();

			info.Option3 = lpMemScript->GetAsNumber();

			info.Option4 = lpMemScript->GetAsNumber();

			info.Option5 = lpMemScript->GetAsNumber();

			info.Option6 = lpMemScript->GetAsNumber();

			this->m_LuckyItemInfo.insert(std::pair<int,LUCKY_ITEM_INFO>(info.Index,info));
		}
	}
	catch(...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;

	#endif
}

bool CLuckyItem::IsLuckyItem(CItem* lpItem) // OK
{
	#if(GAMESERVER_UPDATE>=602)

	std::map<int,LUCKY_ITEM_INFO>::iterator it = this->m_LuckyItemInfo.find(lpItem->m_Index);

	if(it == this->m_LuckyItemInfo.end())
	{
		return 0;
	}
	else
	{
		return 1;
	}

	#else

	return 0;

	#endif
}

bool CLuckyItem::GetLuckyItemIndex(LPOBJ lpObj,int section,int group,WORD* index) // OK
{
	#if(GAMESERVER_UPDATE>=602)

	for(std::map<int,LUCKY_ITEM_INFO>::iterator it=this->m_LuckyItemInfo.begin();it != this->m_LuckyItemInfo.end();it++)
	{
		if((it->second.Index/MAX_ITEM_TYPE) == section && it->second.Group == group && gItemManager.CheckItemRequireClass(lpObj,it->second.Index) != 0)
		{
			(*index) = it->second.Index;
			return 1;
		}
	}

	return 0;

	#else

	return 0;

	#endif
}

bool CLuckyItem::GetLuckyItemDecay(int index,int* decay) // OK
{
	#if(GAMESERVER_UPDATE>=602)

	std::map<int,LUCKY_ITEM_INFO>::iterator it = this->m_LuckyItemInfo.find(index);

	if(it == this->m_LuckyItemInfo.end())
	{
		return 0;
	}
	else
	{
		(*decay) = it->second.Decay;
		return 1;
	}

	#else

	return 0;

	#endif
}

bool CLuckyItem::GetLuckyItemOption0(int index,BYTE* option) // OK
{
	#if(GAMESERVER_UPDATE>=602)

	std::map<int,LUCKY_ITEM_INFO>::iterator it = this->m_LuckyItemInfo.find(index);

	if(it == this->m_LuckyItemInfo.end())
	{
		return 0;
	}
	else
	{
		return gItemOptionRate.GetItemOption0(it->second.Option0,option);
	}

	#else

	return 0;

	#endif
}

bool CLuckyItem::GetLuckyItemOption1(int index,BYTE* option) // OK
{
	#if(GAMESERVER_UPDATE>=602)

	std::map<int,LUCKY_ITEM_INFO>::iterator it = this->m_LuckyItemInfo.find(index);

	if(it == this->m_LuckyItemInfo.end())
	{
		return 0;
	}
	else
	{
		return gItemOptionRate.GetItemOption1(it->second.Option1,option);
	}

	#else

	return 0;

	#endif
}

bool CLuckyItem::GetLuckyItemOption2(int index,BYTE* option) // OK
{
	#if(GAMESERVER_UPDATE>=602)

	std::map<int,LUCKY_ITEM_INFO>::iterator it = this->m_LuckyItemInfo.find(index);

	if(it == this->m_LuckyItemInfo.end())
	{
		return 0;
	}
	else
	{
		return gItemOptionRate.GetItemOption2(it->second.Option2,option);
	}

	#else

	return 0;

	#endif
}

bool CLuckyItem::GetLuckyItemOption3(int index,BYTE* option) // OK
{
	#if(GAMESERVER_UPDATE>=602)

	std::map<int,LUCKY_ITEM_INFO>::iterator it = this->m_LuckyItemInfo.find(index);

	if(it == this->m_LuckyItemInfo.end())
	{
		return 0;
	}
	else
	{
		return gItemOptionRate.GetItemOption3(it->second.Option3,option);
	}

	#else

	return 0;

	#endif
}

bool CLuckyItem::GetLuckyItemOption4(int index,BYTE* option) // OK
{
	#if(GAMESERVER_UPDATE>=602)

	std::map<int,LUCKY_ITEM_INFO>::iterator it = this->m_LuckyItemInfo.find(index);

	if(it == this->m_LuckyItemInfo.end())
	{
		return 0;
	}
	else
	{
		return gItemOptionRate.GetItemOption4(it->second.Option4,option);
	}

	#else

	return 0;

	#endif
}

bool CLuckyItem::GetLuckyItemOption5(int index,BYTE* option) // OK
{
	#if(GAMESERVER_UPDATE>=602)

	std::map<int,LUCKY_ITEM_INFO>::iterator it = this->m_LuckyItemInfo.find(index);

	if(it == this->m_LuckyItemInfo.end())
	{
		return 0;
	}
	else
	{
		return gItemOptionRate.GetItemOption5(it->second.Option5,option);
	}

	#else

	return 0;

	#endif
}

bool CLuckyItem::GetLuckyItemOption6(int index,BYTE* option) // OK
{
	#if(GAMESERVER_UPDATE>=602)

	std::map<int,LUCKY_ITEM_INFO>::iterator it = this->m_LuckyItemInfo.find(index);

	if(it == this->m_LuckyItemInfo.end())
	{
		return 0;
	}
	else
	{
		return gItemOptionRate.GetItemOption6(it->second.Option6,option);
	}

	#else

	return 0;

	#endif
}

bool CLuckyItem::CharacterUseJewelOfExtension(LPOBJ lpObj,int SourceSlot,int TargetSlot) // OK
{
	#if(GAMESERVER_UPDATE>=602)

	if(INVENTORY_FULL_RANGE(SourceSlot) == 0)
	{
		return 0;
	}

	if(INVENTORY_FULL_RANGE(TargetSlot) == 0)
	{
		return 0;
	}

	if(lpObj->Inventory[SourceSlot].IsItem() == 0)
	{
		return 0;
	}

	if(lpObj->Inventory[TargetSlot].IsItem() == 0)
	{
		return 0;
	}

	CItem* lpItem = &lpObj->Inventory[TargetSlot];

	if(lpItem->IsLuckyItem() == 0)
	{
		return 0;
	}

	if(lpItem->m_Durability == 0 || lpItem->m_Durability == 255)
	{
		return 0;
	}

	lpItem->m_Durability = 255;

	lpItem->Convert(lpItem->m_Index,lpItem->m_Option1,lpItem->m_Option2,lpItem->m_Option3,lpItem->m_NewOption,lpItem->m_SetOption,lpItem->m_JewelOfHarmonyOption,lpItem->m_ItemOptionEx,lpItem->m_SocketOption,lpItem->m_SocketOptionBonus);

	gObjectManager.CharacterMakePreviewCharSet(lpObj->Index);
	return 1;

	#else

	return 0;

	#endif
}

bool CLuckyItem::CharacterUseJewelOfElevation(LPOBJ lpObj,int SourceSlot,int TargetSlot) // OK
{
	#if(GAMESERVER_UPDATE>=602)

	return gJewelOfHarmonyOption.AddJewelOfElevationOption(lpObj,SourceSlot,TargetSlot);

	#else

	return 0;

	#endif
}

void CLuckyItem::DGLuckyItemRecv(SDHP_LUCKY_ITEM_RECV* lpMsg) // OK
{
	#if(GAMESERVER_UPDATE>=602)

	if(gObjIsAccountValid(lpMsg->index,lpMsg->account) == 0)
	{
		LogAdd(LOG_RED,"[DGLuckyItemRecv] Invalid Account [%d](%s)",lpMsg->index,lpMsg->account);
		CloseClient(lpMsg->index);
		return;
	}

	LPOBJ lpObj = &gObj[lpMsg->index];

	for(int n=0;n < lpMsg->count;n++)
	{
		SDHP_LUCKY_ITEM1* lpInfo = (SDHP_LUCKY_ITEM1*)(((BYTE*)lpMsg)+sizeof(SDHP_LUCKY_ITEM_RECV)+(sizeof(SDHP_LUCKY_ITEM1)*n));

		if(INVENTORY_RANGE(lpInfo->slot) == 0)
		{
			continue;
		}

		if(lpObj->Inventory[lpInfo->slot].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->Inventory[lpInfo->slot].IsLuckyItem() == 0)
		{
			continue;
		}

		if(lpObj->Inventory[lpInfo->slot].m_Serial != lpInfo->serial)
		{
			continue;
		}

		lpObj->Inventory[lpInfo->slot].m_DurabilitySmall = (WORD)lpInfo->DurabilitySmall;
	}

	#endif
}

void CLuckyItem::GDLuckyItemSend(int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=602)

	if(gObjIsAccountValid(aIndex,gObj[aIndex].Account) == 0)
	{
		return;
	}

	LPOBJ lpObj = &gObj[aIndex];

	BYTE send[4096];

	SDHP_LUCKY_ITEM_SEND pMsg;

	pMsg.header.set(0x22,0x00,0);

	int size = sizeof(pMsg);

	pMsg.index = aIndex;

	memcpy(pMsg.account,lpObj->Account,sizeof(pMsg.account));

	pMsg.count = 0;

	SDHP_LUCKY_ITEM2 info;

	for(int n=0;n < INVENTORY_SIZE;n++)
	{
		if(lpObj->Inventory[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->Inventory[n].IsLuckyItem() == 0)
		{
			continue;
		}

		info.slot = n;

		info.serial = lpObj->Inventory[n].m_Serial;

		memcpy(&send[size],&info,sizeof(info));
		size += sizeof(info);

		pMsg.count++;
	}

	if(pMsg.count > 0)
	{
		pMsg.header.size[0] = SET_NUMBERHB(size);
		pMsg.header.size[1] = SET_NUMBERLB(size);

		memcpy(send,&pMsg,sizeof(pMsg));

		gDataServerConnection.DataSend(send,size);
	}

	#endif
}

void CLuckyItem::GDLuckyItemSaveSend(int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=602)

	LPOBJ lpObj = &gObj[aIndex];

	BYTE send[4096];

	SDHP_LUCKY_ITEM_SAVE_SEND pMsg;

	pMsg.header.set(0x22,0x30,0);

	int size = sizeof(pMsg);

	pMsg.index = aIndex;

	memcpy(pMsg.account,lpObj->Account,sizeof(pMsg.account));

	pMsg.count = 0;

	SDHP_LUCKY_ITEM_SAVE info;

	for(int n=0;n < INVENTORY_SIZE;n++)
	{
		if(lpObj->Inventory[n].IsItem() == 0)
		{
			continue;
		}

		if(lpObj->Inventory[n].IsLuckyItem() == 0)
		{
			continue;
		}

		info.serial = lpObj->Inventory[n].m_Serial;

		info.DurabilitySmall = lpObj->Inventory[n].m_DurabilitySmall;

		memcpy(&send[size],&info,sizeof(info));
		size += sizeof(info);

		pMsg.count++;
	}

	if(pMsg.count > 0)
	{
		pMsg.header.size[0] = SET_NUMBERHB(size);
		pMsg.header.size[1] = SET_NUMBERLB(size);

		memcpy(send,&pMsg,sizeof(pMsg));

		gDataServerConnection.DataSend(send,size);
	}

	#endif
}
