// PentagramSystem.cpp: implementation of the CPentagramSystem class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PentagramSystem.h"
#include "CastleSiegeSync.h"
#include "ChaosBox.h"
#include "DSProtocol.h"
#include "GameMain.h"
#include "ItemManager.h"
#include "MemScript.h"
#include "RandomManager.h"
#include "Util.h"

CPentagramSystem gPentagramSystem;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CPentagramSystem::CPentagramSystem() // OK
{
	#if(GAMESERVER_UPDATE>=701)

	this->m_PentagramTypeInfo.clear();

	this->m_PentagramOptionInfo.clear();

	this->m_PentagramJewelOptionInfo.clear();

	this->m_PentagramJewelRemoveInfo.clear();

	this->m_PentagramJewelUpgradeRankInfo.clear();

	this->m_PentagramJewelUpgradeLevelInfo.clear();

	#endif
}

CPentagramSystem::~CPentagramSystem() // OK
{

}

void CPentagramSystem::Load(char* path) // OK
{
	#if(GAMESERVER_UPDATE>=701)

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

	this->m_PentagramTypeInfo.clear();

	this->m_PentagramOptionInfo.clear();

	try
	{
		while(true)
		{
			if(lpMemScript->GetToken() == TOKEN_END)
			{
				break;
			}

			int section = lpMemScript->GetNumber();

			while(true)
			{
				if(section == 0)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					PENTAGRAM_TYPE_INFO info;

					info.Index = lpMemScript->GetNumber();

					info.OptionIndex1 = lpMemScript->GetAsNumber();

					info.OptionIndex1 = lpMemScript->GetAsNumber();

					this->m_PentagramTypeInfo.insert(std::pair<int,PENTAGRAM_TYPE_INFO>(info.Index,info));
				}
				else if(section == 1)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					PENTAGRAM_OPTION_INFO info;

					info.Index = lpMemScript->GetNumber();

					lpMemScript->GetToken();

					info.Value = lpMemScript->GetAsNumber();

					for(int n=0;n < MAX_PENTAGRAM_JEWEL_RANK;n++)
					{
						info.Type[n] = lpMemScript->GetAsNumber();

						info.Rank[n] = lpMemScript->GetAsNumber();

						info.Level[n] = lpMemScript->GetAsNumber();
					}

					this->m_PentagramOptionInfo.insert(std::pair<int,PENTAGRAM_OPTION_INFO>(info.Index,info));
				}
				else
				{
					break;
				}
			}
		}
	}
	catch(...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;

	#endif
}

void CPentagramSystem::LoadJewel(char* path) // OK
{
	#if(GAMESERVER_UPDATE>=701)

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

	this->m_PentagramJewelOptionInfo.clear();

	try
	{
		while(true)
		{
			if(lpMemScript->GetToken() == TOKEN_END)
			{
				break;
			}

			int section = lpMemScript->GetNumber();

			while(true)
			{
				if(section == 0)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					PENTAGRAM_JEWEL_OPTION_INFO info;

					info.Index = lpMemScript->GetNumber();

					info.ItemIndex = lpMemScript->GetAsNumber();

					info.RankNumber = lpMemScript->GetAsNumber();

					info.RankOption = lpMemScript->GetAsNumber();

					info.RankOptionRate = lpMemScript->GetAsNumber();

					lpMemScript->GetToken();

					for(int n=0;n < MAX_PENTAGRAM_JEWEL_LEVEL;n++)
					{
						info.RankOptionValue[n] = lpMemScript->GetAsNumber();
					}

					this->m_PentagramJewelOptionInfo.insert(std::pair<int,PENTAGRAM_JEWEL_OPTION_INFO>(info.Index,info));
				}
				else
				{
					break;
				}
			}
		}
	}
	catch(...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;

	#endif
}

void CPentagramSystem::LoadMixRate(char* path) // OK
{
	#if(GAMESERVER_UPDATE>=701)

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

	this->m_PentagramJewelRemoveInfo.clear();

	this->m_PentagramJewelUpgradeRankInfo.clear();

	this->m_PentagramJewelUpgradeLevelInfo.clear();

	try
	{
		while(true)
		{
			if(lpMemScript->GetToken() == TOKEN_END)
			{
				break;
			}

			int section = lpMemScript->GetNumber();

			while(true)
			{
				if(section == 0)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					PENTAGRAM_JEWEL_REMOVE_INFO info;

					info.Index = lpMemScript->GetNumber();

					for(int n=0;n < MAX_PENTAGRAM_JEWEL_RANK;n++)
					{
						info.MixRate[n] = lpMemScript->GetAsNumber();
					}

					this->m_PentagramJewelRemoveInfo.insert(std::pair<int,PENTAGRAM_JEWEL_REMOVE_INFO>(info.Index,info));
				}
				else if(section == 1)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					PENTAGRAM_JEWEL_UPGRADE_RANK_INFO info;

					info.Index = lpMemScript->GetNumber();

					info.MixMoney = lpMemScript->GetAsNumber();

					info.MixRate = lpMemScript->GetAsNumber();

					this->m_PentagramJewelUpgradeRankInfo.insert(std::pair<int,PENTAGRAM_JEWEL_UPGRADE_RANK_INFO>(info.Index,info));
				}
				else if(section == 2)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					PENTAGRAM_JEWEL_UPGRADE_LEVEL_INFO info;

					info.Index = lpMemScript->GetNumber();

					info.MixMoney = lpMemScript->GetAsNumber();

					for(int n=0;n < MAX_PENTAGRAM_JEWEL_RANK;n++)
					{
						info.MixRate[n] = lpMemScript->GetAsNumber();
					}

					this->m_PentagramJewelUpgradeLevelInfo.insert(std::pair<int,PENTAGRAM_JEWEL_UPGRADE_LEVEL_INFO>(info.Index,info));
				}
				else
				{
					break;
				}
			}
		}
	}
	catch(...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;

	#endif
}

bool CPentagramSystem::IsPentagramItem(int ItemIndex) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	if(ItemIndex >= GET_ITEM(12,200) && ItemIndex <= GET_ITEM(12,220))
	{
		return 1;
	}

	return 0;

	#else

	return 0;

	#endif
}

bool CPentagramSystem::IsPentagramItem(CItem* lpItem) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	if(lpItem->m_Index >= GET_ITEM(12,200) && lpItem->m_Index <= GET_ITEM(12,220))
	{
		return 1;
	}

	return 0;

	#else

	return 0;

	#endif
}

bool CPentagramSystem::IsPentagramJewel(int ItemIndex) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	if(ItemIndex >= GET_ITEM(12,221) && ItemIndex <= GET_ITEM(12,261))
	{
		return 1;
	}

	return 0;

	#else

	return 0;

	#endif
}

bool CPentagramSystem::IsPentagramJewel(CItem* lpItem) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	if(lpItem->m_Index >= GET_ITEM(12,221) && lpItem->m_Index <= GET_ITEM(12,261))
	{
		return 1;
	}

	return 0;

	#else

	return 0;

	#endif
}

bool CPentagramSystem::IsPentagramMithril(int ItemIndex) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	if(ItemIndex == GET_ITEM(12,144) || ItemIndex == GET_ITEM(12,145) || ItemIndex == GET_ITEM(12,148))
	{
		return 1;
	}

	return 0;

	#else

	return 0;

	#endif
}

bool CPentagramSystem::IsPentagramMithril(CItem* lpItem) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	if(lpItem->m_Index == GET_ITEM(12,144) || lpItem->m_Index == GET_ITEM(12,145) || lpItem->m_Index == GET_ITEM(12,148))
	{
		return 1;
	}

	return 0;

	#else

	return 0;

	#endif
}

PENTAGRAM_JEWEL_INFO* CPentagramSystem::GetPentagramJewelInfo(LPOBJ lpObj,int index,int type) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	if(CHECK_RANGE(index,MAX_PENTAGRAM_JEWEL_INFO) == 0)
	{
		return 0;
	}

	if(type == PENTAGRAM_JEWEL_TYPE_INVENTORY)
	{
		if(lpObj->PentagramJewelInfo_Inventory[index].Index == index)
		{
			return &lpObj->PentagramJewelInfo_Inventory[index];
		}
	}
	else
	{
		if(lpObj->PentagramJewelInfo_Warehouse[index].Index == index)
		{
			return &lpObj->PentagramJewelInfo_Warehouse[index];
		}
	}

	return 0;

	#else

	return 0;

	#endif
}

PENTAGRAM_JEWEL_INFO* CPentagramSystem::AddPentagramJewelInfo(LPOBJ lpObj,CItem* lpItem,int type) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	PENTAGRAM_JEWEL_INFO* lpPentagramJewelInfo = ((type==PENTAGRAM_JEWEL_TYPE_INVENTORY)?lpObj->PentagramJewelInfo_Inventory:lpObj->PentagramJewelInfo_Warehouse);

	for(int n=0;n < MAX_PENTAGRAM_JEWEL_INFO;n++)
	{
		if(lpPentagramJewelInfo[n].Index == 0xFF)
		{
			lpPentagramJewelInfo[n].Type = type;
			lpPentagramJewelInfo[n].Index = n;
			lpPentagramJewelInfo[n].Attribute = lpItem->m_SocketOptionBonus;
			lpPentagramJewelInfo[n].ItemSection = lpItem->m_Index/MAX_ITEM_TYPE;
			lpPentagramJewelInfo[n].ItemType = lpItem->m_Index%MAX_ITEM_TYPE;
			lpPentagramJewelInfo[n].ItemLevel = (BYTE)lpItem->m_Level;
			lpPentagramJewelInfo[n].OptionIndexRank1 = lpItem->m_SocketOption[0]%16;
			lpPentagramJewelInfo[n].OptionLevelRank1 = lpItem->m_SocketOption[0]/16;
			lpPentagramJewelInfo[n].OptionIndexRank2 = lpItem->m_SocketOption[1]%16;
			lpPentagramJewelInfo[n].OptionLevelRank2 = lpItem->m_SocketOption[1]/16;
			lpPentagramJewelInfo[n].OptionIndexRank3 = lpItem->m_SocketOption[2]%16;
			lpPentagramJewelInfo[n].OptionLevelRank3 = lpItem->m_SocketOption[2]/16;
			lpPentagramJewelInfo[n].OptionIndexRank4 = lpItem->m_SocketOption[3]%16;
			lpPentagramJewelInfo[n].OptionLevelRank4 = lpItem->m_SocketOption[3]/16;
			lpPentagramJewelInfo[n].OptionIndexRank5 = lpItem->m_SocketOption[4]%16;
			lpPentagramJewelInfo[n].OptionLevelRank5 = lpItem->m_SocketOption[4]/16;
			this->GDPentagramJewelInsertSaveSend(lpObj->Index,&lpPentagramJewelInfo[n]);
			return &lpPentagramJewelInfo[n];
		}
	}

	#endif

	return 0;
}

PENTAGRAM_JEWEL_INFO* CPentagramSystem::AddPentagramJewelInfo(LPOBJ lpObj,PENTAGRAM_JEWEL_INFO* lpInfo,int type) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	PENTAGRAM_JEWEL_INFO* lpPentagramJewelInfo = ((type==PENTAGRAM_JEWEL_TYPE_INVENTORY)?lpObj->PentagramJewelInfo_Inventory:lpObj->PentagramJewelInfo_Warehouse);

	for(int n=0;n < MAX_PENTAGRAM_JEWEL_INFO;n++)
	{
		if(lpPentagramJewelInfo[n].Index == 0xFF)
		{
			lpPentagramJewelInfo[n].Type = type;
			lpPentagramJewelInfo[n].Index = n;
			lpPentagramJewelInfo[n].Attribute = lpInfo->Attribute;
			lpPentagramJewelInfo[n].ItemSection = lpInfo->ItemSection;
			lpPentagramJewelInfo[n].ItemType = lpInfo->ItemType;
			lpPentagramJewelInfo[n].ItemLevel = lpInfo->ItemLevel;
			lpPentagramJewelInfo[n].OptionIndexRank1 = lpInfo->OptionIndexRank1;
			lpPentagramJewelInfo[n].OptionLevelRank1 = lpInfo->OptionLevelRank1;
			lpPentagramJewelInfo[n].OptionIndexRank2 = lpInfo->OptionIndexRank2;
			lpPentagramJewelInfo[n].OptionLevelRank2 = lpInfo->OptionLevelRank2;
			lpPentagramJewelInfo[n].OptionIndexRank3 = lpInfo->OptionIndexRank3;
			lpPentagramJewelInfo[n].OptionLevelRank3 = lpInfo->OptionLevelRank3;
			lpPentagramJewelInfo[n].OptionIndexRank4 = lpInfo->OptionIndexRank4;
			lpPentagramJewelInfo[n].OptionLevelRank4 = lpInfo->OptionLevelRank4;
			lpPentagramJewelInfo[n].OptionIndexRank5 = lpInfo->OptionIndexRank5;
			lpPentagramJewelInfo[n].OptionLevelRank5 = lpInfo->OptionLevelRank5;
			this->GDPentagramJewelInsertSaveSend(lpObj->Index,&lpPentagramJewelInfo[n]);
			return &lpPentagramJewelInfo[n];
		}
	}

	#endif

	return 0;
}

void CPentagramSystem::DelPentagramJewelInfo(LPOBJ lpObj,int index,int type) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	if(CHECK_RANGE(index,MAX_PENTAGRAM_JEWEL_INFO) == 0)
	{
		return;
	}

	if(type == PENTAGRAM_JEWEL_TYPE_INVENTORY)
	{
		if(lpObj->PentagramJewelInfo_Inventory[index].Index == index)
		{
			lpObj->PentagramJewelInfo_Inventory[index].Clear();
			this->GDPentagramJewelDeleteSaveSend(lpObj->Index,type,index);
		}
	}
	else
	{
		if(lpObj->PentagramJewelInfo_Warehouse[index].Index == index)
		{
			lpObj->PentagramJewelInfo_Warehouse[index].Clear();
			this->GDPentagramJewelDeleteSaveSend(lpObj->Index,type,index);
		}
	}

	#endif
}

void CPentagramSystem::DelAllPentagramJewelInfo(LPOBJ lpObj,CItem* lpItem,int type) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	if(lpItem->IsPentagramItem() != 0)
	{
		for(int n=0;n < MAX_SOCKET_OPTION;n++)
		{
			this->DelPentagramJewelInfo(lpObj,lpItem->m_SocketOption[n],type);
		}
	}

	#endif
}

bool CPentagramSystem::CheckExchangePentagramItem(LPOBJ lpObj) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	for(int n=0;n < TRADE_SIZE;n++)
	{
		if(lpObj->Trade[n].IsItem() != 0 && lpObj->Trade[n].IsPentagramItem() != 0)
		{
			if(lpObj->Trade[n].m_Durability == 0)
			{
				return 0;
			}
		}
	}

	return 1;

	#else

	return 1;

	#endif
}

bool CPentagramSystem::CheckExchangePentagramItem(LPOBJ lpObj,CItem* lpItem) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	if(lpItem->IsItem() != 0 && lpItem->IsPentagramItem() != 0)
	{
		if(lpItem->m_Durability == 0)
		{
			return 0;
		}
	}

	return 1;

	#else

	return 1;

	#endif
}

bool CPentagramSystem::CheckPentagramOption(LPOBJ lpObj,PENTAGRAM_OPTION_INFO* lpInfo) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	for(int n=0;n < MAX_SOCKET_OPTION;n++)
	{
		if(lpInfo->Type[n] != -1)
		{
			PENTAGRAM_JEWEL_INFO* lpPentagramJewelInfo = this->GetPentagramJewelInfo(lpObj,lpObj->Inventory[236].m_SocketOption[n],PENTAGRAM_JEWEL_TYPE_INVENTORY);

			if(lpPentagramJewelInfo == 0)
			{
				return 0;
			}

			if(lpPentagramJewelInfo->ItemLevel < lpInfo->Level[n])
			{
				return 0;
			}

			if((lpPentagramJewelInfo->Attribute/16) < lpInfo->Rank[n])
			{
				return 0;
			}
		}
	}

	return 1;

	#else

	return 0;

	#endif
}

void CPentagramSystem::MovePentagramToInventoryFromWarehouse(LPOBJ lpObj,CItem* lpItem) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	if(lpItem->IsPentagramItem() == 0)
	{
		return;
	}

	int count = 0;

	for(int n=0;n < MAX_SOCKET_OPTION;n++)
	{
		PENTAGRAM_JEWEL_INFO* lpPentagramJewelInfo_Warehouse = this->GetPentagramJewelInfo(lpObj,lpItem->m_SocketOption[n],PENTAGRAM_JEWEL_TYPE_WAREHOUSE);

		if(lpPentagramJewelInfo_Warehouse != 0)
		{
			PENTAGRAM_JEWEL_INFO* lpPentagramJewelInfo_Inventory = this->AddPentagramJewelInfo(lpObj,lpPentagramJewelInfo_Warehouse,PENTAGRAM_JEWEL_TYPE_INVENTORY);

			if(lpPentagramJewelInfo_Inventory != 0)
			{
				this->DelPentagramJewelInfo(lpObj,lpPentagramJewelInfo_Warehouse->Index,PENTAGRAM_JEWEL_TYPE_WAREHOUSE);

				lpItem->m_SocketOption[n] = lpPentagramJewelInfo_Inventory->Index;

				count++;
			}
		}
	}

	if(count > 0)
	{
		this->GCPentagramJewelInfoSend(lpObj->Index,PENTAGRAM_JEWEL_TYPE_INVENTORY);

		this->GCPentagramJewelInfoSend(lpObj->Index,PENTAGRAM_JEWEL_TYPE_WAREHOUSE);
	}

	#endif
}

void CPentagramSystem::MovePentagramToWarehouseFromInventory(LPOBJ lpObj,CItem* lpItem) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	if(lpItem->IsPentagramItem() == 0)
	{
		return;
	}

	int count = 0;

	for(int n=0;n < MAX_SOCKET_OPTION;n++)
	{
		PENTAGRAM_JEWEL_INFO* lpPentagramJewelInfo_Inventory = this->GetPentagramJewelInfo(lpObj,lpItem->m_SocketOption[n],PENTAGRAM_JEWEL_TYPE_INVENTORY);

		if(lpPentagramJewelInfo_Inventory != 0)
		{
			PENTAGRAM_JEWEL_INFO* lpPentagramJewelInfo_Warehouse = this->AddPentagramJewelInfo(lpObj,lpPentagramJewelInfo_Inventory,PENTAGRAM_JEWEL_TYPE_WAREHOUSE);

			if(lpPentagramJewelInfo_Warehouse != 0)
			{
				this->DelPentagramJewelInfo(lpObj,lpPentagramJewelInfo_Inventory->Index,PENTAGRAM_JEWEL_TYPE_INVENTORY);

				lpItem->m_SocketOption[n] = lpPentagramJewelInfo_Warehouse->Index;

				count++;
			}
		}
	}

	if(count > 0)
	{
		this->GCPentagramJewelInfoSend(lpObj->Index,PENTAGRAM_JEWEL_TYPE_INVENTORY);

		this->GCPentagramJewelInfoSend(lpObj->Index,PENTAGRAM_JEWEL_TYPE_WAREHOUSE);
	}

	#endif
}

void CPentagramSystem::ExchangePentagramItem(LPOBJ lpObj,LPOBJ lpTarget) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	int count = 0;

	for(int n=0;n < TRADE_SIZE;n++)
	{
		if(lpObj->Trade[n].IsItem() != 0 && lpObj->Trade[n].IsPentagramItem() != 0)
		{
			for(int i=0;i < MAX_SOCKET_OPTION;i++)
			{
				PENTAGRAM_JEWEL_INFO* lpPentagramJewelInfo_Source = this->GetPentagramJewelInfo(lpObj,lpObj->Trade[n].m_SocketOption[i],PENTAGRAM_JEWEL_TYPE_INVENTORY);

				if(lpPentagramJewelInfo_Source != 0)
				{
					PENTAGRAM_JEWEL_INFO* lpPentagramJewelInfo_Target = this->AddPentagramJewelInfo(lpTarget,lpPentagramJewelInfo_Source,PENTAGRAM_JEWEL_TYPE_INVENTORY);

					if(lpPentagramJewelInfo_Target != 0)
					{
						this->DelPentagramJewelInfo(lpObj,lpPentagramJewelInfo_Source->Index,PENTAGRAM_JEWEL_TYPE_INVENTORY);

						lpTarget->Inventory[lpObj->Trade[n].m_Slot].m_SocketOption[i] = lpPentagramJewelInfo_Target->Index;

						count++;
					}
				}
			}

			lpTarget->Inventory[lpObj->Trade[n].m_Slot].m_Durability--;
		}
	}

	if(count > 0)
	{
		this->GCPentagramJewelInfoSend(lpTarget->Index,PENTAGRAM_JEWEL_TYPE_INVENTORY);
	}

	#endif
}

void CPentagramSystem::ExchangePentagramItem(LPOBJ lpObj,LPOBJ lpTarget,CItem* lpItem) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	if(lpItem->IsPentagramItem() == 0)
	{
		return;
	}

	int count = 0;

	for(int n=0;n < MAX_SOCKET_OPTION;n++)
	{
		PENTAGRAM_JEWEL_INFO* lpPentagramJewelInfo_Source = this->GetPentagramJewelInfo(lpObj,lpItem->m_SocketOption[n],PENTAGRAM_JEWEL_TYPE_INVENTORY);

		if(lpPentagramJewelInfo_Source != 0)
		{
			PENTAGRAM_JEWEL_INFO* lpPentagramJewelInfo_Target = this->AddPentagramJewelInfo(lpTarget,lpPentagramJewelInfo_Source,PENTAGRAM_JEWEL_TYPE_INVENTORY);

			if(lpPentagramJewelInfo_Target != 0)
			{
				this->DelPentagramJewelInfo(lpObj,lpPentagramJewelInfo_Source->Index,PENTAGRAM_JEWEL_TYPE_INVENTORY);

				lpItem->m_SocketOption[n] = lpPentagramJewelInfo_Target->Index;

				count++;
			}
		}
	}

	lpItem->m_Durability--;

	if(count > 0)
	{
		this->GCPentagramJewelInfoSend(lpTarget->Index,PENTAGRAM_JEWEL_TYPE_INVENTORY);
	}

	#endif
}

bool CPentagramSystem::GetPentagramTypeInfo(int index,PENTAGRAM_TYPE_INFO* lpInfo) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	std::map<int,PENTAGRAM_TYPE_INFO>::iterator it = this->m_PentagramTypeInfo.find(index);

	if(it == this->m_PentagramTypeInfo.end())
	{
		return 0;
	}
	else
	{
		(*lpInfo) = it->second;
		return 1;
	}

	#else

	return 0;

	#endif
}

bool CPentagramSystem::GetPentagramOptionInfo(int index,PENTAGRAM_OPTION_INFO* lpInfo) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	std::map<int,PENTAGRAM_OPTION_INFO>::iterator it = this->m_PentagramOptionInfo.find(index);

	if(it == this->m_PentagramOptionInfo.end())
	{
		return 0;
	}
	else
	{
		(*lpInfo) = it->second;
		return 1;
	}

	#else

	return 0;

	#endif
}

bool CPentagramSystem::GetPentagramJewelOptionInfo(int ItemIndex,int RankNumber,int RankOption,PENTAGRAM_JEWEL_OPTION_INFO* lpInfo) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	for(std::map<int,PENTAGRAM_JEWEL_OPTION_INFO>::iterator it=this->m_PentagramJewelOptionInfo.begin();it != this->m_PentagramJewelOptionInfo.end();it++)
	{
		if(it->second.ItemIndex == ItemIndex)
		{
			if(it->second.RankNumber == RankNumber)
			{
				if(it->second.RankOption == RankOption)
				{
					(*lpInfo) = it->second;
					return 1;
				}
			}
		}
	}

	return 0;

	#else

	return 0;

	#endif
}

bool CPentagramSystem::GetPentagramJewelRemoveInfo(int index,PENTAGRAM_JEWEL_REMOVE_INFO* lpInfo) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	std::map<int,PENTAGRAM_JEWEL_REMOVE_INFO>::iterator it = this->m_PentagramJewelRemoveInfo.find(index);

	if(it == this->m_PentagramJewelRemoveInfo.end())
	{
		return 0;
	}
	else
	{
		(*lpInfo) = it->second;
		return 1;
	}

	#else

	return 0;

	#endif
}

bool CPentagramSystem::GetPentagramJewelUpgradeRankInfo(int index,PENTAGRAM_JEWEL_UPGRADE_RANK_INFO* lpInfo) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	std::map<int,PENTAGRAM_JEWEL_UPGRADE_RANK_INFO>::iterator it = this->m_PentagramJewelUpgradeRankInfo.find(index);

	if(it == this->m_PentagramJewelUpgradeRankInfo.end())
	{
		return 0;
	}
	else
	{
		(*lpInfo) = it->second;
		return 1;
	}

	#else

	return 0;

	#endif
}

bool CPentagramSystem::GetPentagramJewelUpgradeLevelInfo(int index,PENTAGRAM_JEWEL_UPGRADE_LEVEL_INFO* lpInfo) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	std::map<int,PENTAGRAM_JEWEL_UPGRADE_LEVEL_INFO>::iterator it = this->m_PentagramJewelUpgradeLevelInfo.find(index);

	if(it == this->m_PentagramJewelUpgradeLevelInfo.end())
	{
		return 0;
	}
	else
	{
		(*lpInfo) = it->second;
		return 1;
	}

	return 0;

	#else

	return 0;

	#endif
}

bool CPentagramSystem::GetPentagramRandomJewelOption(int ItemIndex,int RankNumber,BYTE* option) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	CRandomManager RandomManager;

	for(std::map<int,PENTAGRAM_JEWEL_OPTION_INFO>::iterator it=this->m_PentagramJewelOptionInfo.begin();it != this->m_PentagramJewelOptionInfo.end();it++)
	{
		if(it->second.ItemIndex == ItemIndex)
		{
			if(it->second.RankNumber == RankNumber)
			{
				RandomManager.AddElement(it->second.RankOption,it->second.RankOptionRate);
			}
		}
	}

	return RandomManager.GetRandomElement(option);

	#else

	return 0;

	#endif
}

void CPentagramSystem::GetPentagramRelationshipDamage(LPOBJ lpObj,LPOBJ lpTarget,int* damage) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	int rate = 100;

	if(lpTarget->ElementalAttribute == 0)
	{
		if(lpObj->Type == OBJECT_USER && lpTarget->Type == OBJECT_USER)
		{
			rate = 143;
		}

		if(lpObj->Type == OBJECT_MONSTER && lpTarget->Type == OBJECT_USER)
		{
			rate = 110;
		}

		if(lpObj->Type == OBJECT_USER && lpTarget->Type == OBJECT_MONSTER)
		{
			rate = 100;
		}
	}
	else
	{
		if(lpObj->Type == OBJECT_USER && lpTarget->Type == OBJECT_USER)
		{
			rate = gPentagramRelationshipDamageTablePvP[lpObj->ElementalAttribute][lpTarget->ElementalAttribute];
		}
		else
		{
			rate = gPentagramRelationshipDamageTablePvM[lpObj->ElementalAttribute][lpTarget->ElementalAttribute];
		}

		switch(lpObj->ElementalAttribute)
		{
			case ELEMENTAL_ATTRIBUTE_FIRE:
				rate += lpObj->PentagramJewelOption.MulElementalDamageVsFire;
				break;
			case ELEMENTAL_ATTRIBUTE_WATER:
				rate += lpObj->PentagramJewelOption.MulElementalDamageVsWater;
				break;
			case ELEMENTAL_ATTRIBUTE_EARTH:
				rate += lpObj->PentagramJewelOption.MulElementalDamageVsEarth;
				break;
			case ELEMENTAL_ATTRIBUTE_WIND:
				rate += lpObj->PentagramJewelOption.MulElementalDamageVsWind;
				break;
			case ELEMENTAL_ATTRIBUTE_DARK:
				rate += lpObj->PentagramJewelOption.MulElementalDamageVsDark;
				break;
		}
	}

	(*damage) = ((*damage)*rate)/100;

	#endif
}

void CPentagramSystem::GetPentagramRelationshipDefense(LPOBJ lpObj,LPOBJ lpTarget,int* defense) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	int rate = 100;

	if(lpTarget->ElementalAttribute == 0)
	{
		if(lpObj->Type == OBJECT_USER && lpTarget->Type == OBJECT_USER)
		{
			rate = 80;
		}

		if(lpObj->Type == OBJECT_MONSTER && lpTarget->Type == OBJECT_USER)
		{
			rate = 80;
		}

		if(lpObj->Type == OBJECT_USER && lpTarget->Type == OBJECT_MONSTER)
		{
			rate = 130;
		}
	}
	else
	{
		if(lpObj->Type == OBJECT_USER && lpTarget->Type == OBJECT_USER)
		{
			rate = gPentagramRelationshipDefenseTablePvP[lpObj->ElementalAttribute][lpTarget->ElementalAttribute];
		}
		else
		{
			rate = gPentagramRelationshipDefenseTablePvM[lpObj->ElementalAttribute][lpTarget->ElementalAttribute];
		}

		switch(lpObj->ElementalAttribute)
		{
			case ELEMENTAL_ATTRIBUTE_FIRE:
				rate += lpObj->PentagramJewelOption.MulElementalDefenseVsFire;
				break;
			case ELEMENTAL_ATTRIBUTE_WATER:
				rate += lpObj->PentagramJewelOption.MulElementalDefenseVsWater;
				break;
			case ELEMENTAL_ATTRIBUTE_EARTH:
				rate += lpObj->PentagramJewelOption.MulElementalDefenseVsEarth;
				break;
			case ELEMENTAL_ATTRIBUTE_WIND:
				rate += lpObj->PentagramJewelOption.MulElementalDefenseVsWind;
				break;
			case ELEMENTAL_ATTRIBUTE_DARK:
				rate += lpObj->PentagramJewelOption.MulElementalDefenseVsDark;
				break;
		}
	}

	(*defense) = ((*defense)*rate)/100;

	#endif
}

void CPentagramSystem::CalcPentagramOption(LPOBJ lpObj,bool flag) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	if(lpObj->Inventory[236].IsItem() == 0 || lpObj->Inventory[236].IsPentagramItem() == 0 || lpObj->Inventory[236].m_IsValidItem == 0)
	{
		return;
	}

	PENTAGRAM_TYPE_INFO PentagramTypeInfo;

	if(this->GetPentagramTypeInfo(lpObj->Inventory[236].m_Index,&PentagramTypeInfo) != 0)
	{
		PENTAGRAM_OPTION_INFO PentagramOptionInfo;

		if(this->GetPentagramOptionInfo(PentagramTypeInfo.OptionIndex1,&PentagramOptionInfo) != 0)
		{
			if(this->CheckPentagramOption(lpObj,&PentagramOptionInfo) != 0)
			{
				this->InsertOption(lpObj,PentagramOptionInfo.Index,PentagramOptionInfo.Value,flag);
			}
		}

		if(this->GetPentagramOptionInfo(PentagramTypeInfo.OptionIndex2,&PentagramOptionInfo) != 0)
		{
			if(this->CheckPentagramOption(lpObj,&PentagramOptionInfo) != 0)
			{
				this->InsertOption(lpObj,PentagramOptionInfo.Index,PentagramOptionInfo.Value,flag);
			}
		}
	}

	#endif
}

void CPentagramSystem::CalcPentagramJewelOption(LPOBJ lpObj,bool flag) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	if(lpObj->Inventory[236].IsItem() == 0 || lpObj->Inventory[236].IsPentagramItem() == 0 || lpObj->Inventory[236].m_IsValidItem == 0)
	{
		return;
	}

	for(int n=0;n < MAX_SOCKET_OPTION;n++)
	{
		PENTAGRAM_JEWEL_INFO* lpPentagramJewelInfo = this->GetPentagramJewelInfo(lpObj,lpObj->Inventory[236].m_SocketOption[n],PENTAGRAM_JEWEL_TYPE_INVENTORY);

		if(lpPentagramJewelInfo != 0)
		{
			PENTAGRAM_JEWEL_OPTION_INFO PentagramJewelOptionInfo;

			if(this->GetPentagramJewelOptionInfo(GET_ITEM(lpPentagramJewelInfo->ItemSection,lpPentagramJewelInfo->ItemType),1,lpPentagramJewelInfo->OptionIndexRank1,&PentagramJewelOptionInfo) != 0)
			{
				this->InsertJewelOption(lpObj,PentagramJewelOptionInfo.Index,PentagramJewelOptionInfo.RankOptionValue[lpPentagramJewelInfo->OptionLevelRank1],flag);
			}

			if(this->GetPentagramJewelOptionInfo(GET_ITEM(lpPentagramJewelInfo->ItemSection,lpPentagramJewelInfo->ItemType),2,lpPentagramJewelInfo->OptionIndexRank2,&PentagramJewelOptionInfo) != 0)
			{
				this->InsertJewelOption(lpObj,PentagramJewelOptionInfo.Index,PentagramJewelOptionInfo.RankOptionValue[lpPentagramJewelInfo->OptionLevelRank2],flag);
			}

			if(this->GetPentagramJewelOptionInfo(GET_ITEM(lpPentagramJewelInfo->ItemSection,lpPentagramJewelInfo->ItemType),3,lpPentagramJewelInfo->OptionIndexRank3,&PentagramJewelOptionInfo) != 0)
			{
				this->InsertJewelOption(lpObj,PentagramJewelOptionInfo.Index,PentagramJewelOptionInfo.RankOptionValue[lpPentagramJewelInfo->OptionLevelRank3],flag);
			}

			if(this->GetPentagramJewelOptionInfo(GET_ITEM(lpPentagramJewelInfo->ItemSection,lpPentagramJewelInfo->ItemType),4,lpPentagramJewelInfo->OptionIndexRank4,&PentagramJewelOptionInfo) != 0)
			{
				this->InsertJewelOption(lpObj,PentagramJewelOptionInfo.Index,PentagramJewelOptionInfo.RankOptionValue[lpPentagramJewelInfo->OptionLevelRank4],flag);
			}

			if(this->GetPentagramJewelOptionInfo(GET_ITEM(lpPentagramJewelInfo->ItemSection,lpPentagramJewelInfo->ItemType),5,lpPentagramJewelInfo->OptionIndexRank5,&PentagramJewelOptionInfo) != 0)
			{
				this->InsertJewelOption(lpObj,PentagramJewelOptionInfo.Index,PentagramJewelOptionInfo.RankOptionValue[lpPentagramJewelInfo->OptionLevelRank5],flag);
			}
		}
	}

	#endif
}

bool CPentagramSystem::CheckPentagramSocket(LPOBJ lpObj,CItem* lpItem) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	for(int n=0;n < MAX_SOCKET_OPTION;n++)
	{
		PENTAGRAM_JEWEL_INFO* lpPentagramJewelInfo = this->GetPentagramJewelInfo(lpObj,lpItem->m_SocketOption[n],PENTAGRAM_JEWEL_TYPE_INVENTORY);

		if(lpPentagramJewelInfo != 0)
		{
			PENTAGRAM_JEWEL_OPTION_INFO PentagramJewelOptionInfo;

			if(this->GetPentagramJewelOptionInfo(GET_ITEM(lpPentagramJewelInfo->ItemSection,lpPentagramJewelInfo->ItemType),1,lpPentagramJewelInfo->OptionIndexRank1,&PentagramJewelOptionInfo) != 0)
			{
				return 1;
			}

			if(this->GetPentagramJewelOptionInfo(GET_ITEM(lpPentagramJewelInfo->ItemSection,lpPentagramJewelInfo->ItemType),2,lpPentagramJewelInfo->OptionIndexRank2,&PentagramJewelOptionInfo) != 0)
			{
				return 1;
			}

			if(this->GetPentagramJewelOptionInfo(GET_ITEM(lpPentagramJewelInfo->ItemSection,lpPentagramJewelInfo->ItemType),3,lpPentagramJewelInfo->OptionIndexRank3,&PentagramJewelOptionInfo) != 0)
			{
				return 1;
			}

			if(this->GetPentagramJewelOptionInfo(GET_ITEM(lpPentagramJewelInfo->ItemSection,lpPentagramJewelInfo->ItemType),4,lpPentagramJewelInfo->OptionIndexRank4,&PentagramJewelOptionInfo) != 0)
			{
				return 1;
			}

			if(this->GetPentagramJewelOptionInfo(GET_ITEM(lpPentagramJewelInfo->ItemSection,lpPentagramJewelInfo->ItemType),5,lpPentagramJewelInfo->OptionIndexRank5,&PentagramJewelOptionInfo) != 0)
			{
				return 1;
			}
		}
	}


	#endif
	return 0;
}

void CPentagramSystem::InsertOption(LPOBJ lpObj,int index,int value,bool flag) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	if(flag != 0)
	{
		return;
	}

	switch(index)
	{
		case PENTAGRAM_OPTION_MUL_PENTAGRAM_DAMAGE:
			lpObj->PentagramOption.MulPentagramDamage += value;
			break;
		case PENTAGRAM_OPTION_MUL_PENTAGRAM_DEFENSE:
			lpObj->PentagramOption.MulPentagramDefense += value;
			break;
		case PENTAGRAM_OPTION_ADD_ELEMENTAL_CRITICAL_DAMAGE_RATE:
			lpObj->PentagramOption.AddElementalCriticalDamageRate += value;
			break;
		case PENTAGRAM_OPTION_ADD_ELEMENTAL_DEFENSE_TRANSFER_RATE:
			lpObj->PentagramOption.AddElementalDefenseTransferRate += value;
			break;
		case PENTAGRAM_OPTION_ADD_ELEMENTAL_ATTACK_TRANSFER_RATE:
			lpObj->PentagramOption.AddElementalAttackTransferRate += value;
			break;
		case PENTAGRAM_OPTION_MUL_ELEMENTAL_DEFENSE_SUCCESS_RATE:
			lpObj->PentagramOption.MulElementalDefenseSuccessRate += value;
			break;
		case PENTAGRAM_OPTION_MUL_ELEMENTAL_ATTACK_SUCCESS_RATE:
			lpObj->PentagramOption.MulElementalAttackSuccessRate += value;
			break;
	}

	#endif
}

void CPentagramSystem::InsertJewelOption(LPOBJ lpObj,int index,int value,bool flag) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	if(flag == 0)
	{
		if(index == PENTAGRAM_JEWEL_OF_GALE_OPTION_ADD_STRENGTH || index == PENTAGRAM_JEWEL_OF_GALE_OPTION_ADD_DEXTERITY || index == PENTAGRAM_JEWEL_OF_GALE_OPTION_ADD_VITALITY || index == PENTAGRAM_JEWEL_OF_GALE_OPTION_ADD_ENERGY)
		{
			return;
		}
	}
	else
	{
		if(index != PENTAGRAM_JEWEL_OF_GALE_OPTION_ADD_STRENGTH && index != PENTAGRAM_JEWEL_OF_GALE_OPTION_ADD_DEXTERITY && index != PENTAGRAM_JEWEL_OF_GALE_OPTION_ADD_VITALITY && index != PENTAGRAM_JEWEL_OF_GALE_OPTION_ADD_ENERGY)
		{
			return;
		}
	}

	switch(index)
	{
		case PENTAGRAM_JEWEL_OF_ANGER_OPTION_ADD_ELEMENTAL_DAMAGE:
			lpObj->PentagramJewelOption.AddElementalDamage += value;
			break;
		case PENTAGRAM_JEWEL_OF_ANGER_OPTION_MUL_ELEMENTAL_DAMAGE_VS_FIRE:
			lpObj->PentagramJewelOption.MulElementalDamageVsFire += value;
			break;
		case PENTAGRAM_JEWEL_OF_ANGER_OPTION_MUL_ELEMENTAL_DAMAGE_VS_WATER:
			lpObj->PentagramJewelOption.MulElementalDamageVsWater += value;
			break;
		case PENTAGRAM_JEWEL_OF_ANGER_OPTION_MUL_ELEMENTAL_DAMAGE_VS_EARTH:
			lpObj->PentagramJewelOption.MulElementalDamageVsEarth += value;
			break;
		case PENTAGRAM_JEWEL_OF_ANGER_OPTION_MUL_ELEMENTAL_DAMAGE_VS_WIND:
			lpObj->PentagramJewelOption.MulElementalDamageVsWind += value;
			break;
		case PENTAGRAM_JEWEL_OF_ANGER_OPTION_MUL_ELEMENTAL_DAMAGE_VS_DARK:
			lpObj->PentagramJewelOption.MulElementalDamageVsDark += value;
			break;
		case PENTAGRAM_JEWEL_OF_ANGER_OPTION_ADD_ELEMENTAL_DAMAGE_PVP:
			lpObj->PentagramJewelOption.AddElementalDamagePvP += value;
			break;
		case PENTAGRAM_JEWEL_OF_ANGER_OPTION_ADD_ELEMENTAL_DAMAGE_PVM:
			lpObj->PentagramJewelOption.AddElementalDamagePvM += value;
			break;
		case PENTAGRAM_JEWEL_OF_ANGER_OPTION_ADD_ELEMENTAL_DAMAGE_RANGE:
			lpObj->PentagramJewelOption.AddElementalDamageRange += value;
			break;
		case PENTAGRAM_JEWEL_OF_ANGER_OPTION_ADD_ELEMENTAL_DAMAGE_MELEE:
			lpObj->PentagramJewelOption.AddElementalDamageMelee += value;
			break;
		case PENTAGRAM_JEWEL_OF_ANGER_OPTION_ADD_ELEMENTAL_CRITICAL_DAMAGE_RATE_PVP:
			lpObj->PentagramJewelOption.AddElementalCriticalDamageRatePvP += value;
			break;
		case PENTAGRAM_JEWEL_OF_ANGER_OPTION_ADD_ELEMENTAL_CRITICAL_DAMAGE_RATE_PVM:
			lpObj->PentagramJewelOption.AddElementalCriticalDamageRatePvM += value;
			break;
		case PENTAGRAM_JEWEL_OF_BLESSING_OPTION_ADD_ELEMENTAL_DEFENSE:
			lpObj->PentagramJewelOption.AddElementalDefense += value;
			break;
		case PENTAGRAM_JEWEL_OF_BLESSING_OPTION_MUL_ELEMENTAL_DEFENSE_VS_FIRE:
			lpObj->PentagramJewelOption.MulElementalDefenseVsFire += value;
			break;
		case PENTAGRAM_JEWEL_OF_BLESSING_OPTION_MUL_ELEMENTAL_DEFENSE_VS_WATER:
			lpObj->PentagramJewelOption.MulElementalDefenseVsWater += value;
			break;
		case PENTAGRAM_JEWEL_OF_BLESSING_OPTION_MUL_ELEMENTAL_DEFENSE_VS_EARTH:
			lpObj->PentagramJewelOption.MulElementalDefenseVsEarth += value;
			break;
		case PENTAGRAM_JEWEL_OF_BLESSING_OPTION_MUL_ELEMENTAL_DEFENSE_VS_WIND:
			lpObj->PentagramJewelOption.MulElementalDefenseVsWind += value;
			break;
		case PENTAGRAM_JEWEL_OF_BLESSING_OPTION_MUL_ELEMENTAL_DEFENSE_VS_DARK:
			lpObj->PentagramJewelOption.MulElementalDefenseVsDark += value;
			break;
		case PENTAGRAM_JEWEL_OF_BLESSING_OPTION_ADD_ELEMENTAL_DEFENSE_PVP:
			lpObj->PentagramJewelOption.AddElementalDefensePvP += value;
			break;
		case PENTAGRAM_JEWEL_OF_BLESSING_OPTION_ADD_ELEMENTAL_DEFENSE_PVM:
			lpObj->PentagramJewelOption.AddElementalDefensePvM += value;
			break;
		case PENTAGRAM_JEWEL_OF_BLESSING_OPTION_ADD_ELEMENTAL_DEFENSE_RANGE:
			lpObj->PentagramJewelOption.AddElementalDefenseRange += value;
			break;
		case PENTAGRAM_JEWEL_OF_BLESSING_OPTION_ADD_ELEMENTAL_DEFENSE_MELEE:
			lpObj->PentagramJewelOption.AddElementalDefenseMelee += value;
			break;
		case PENTAGRAM_JEWEL_OF_BLESSING_OPTION_MUL_ELEMENTAL_DAMAGE_PVP:
			lpObj->PentagramJewelOption.MulElementalDamagePvP += value;
			break;
		case PENTAGRAM_JEWEL_OF_BLESSING_OPTION_MUL_ELEMENTAL_DAMAGE_PVM:
			lpObj->PentagramJewelOption.MulElementalDamagePvM += value;
			break;
		case PENTAGRAM_JEWEL_OF_INTEGRITY_OPTION_MUL_ELEMENTAL_ATTACK_SUCCESS_RATE:
			lpObj->PentagramJewelOption.MulElementalAttackSuccessRate += value;
			break;
		case PENTAGRAM_JEWEL_OF_INTEGRITY_OPTION_MUL_ELEMENTAL_DAMAGE_VS_FIRE:
			lpObj->PentagramJewelOption.MulElementalDamageVsFire += value;
			break;
		case PENTAGRAM_JEWEL_OF_INTEGRITY_OPTION_MUL_ELEMENTAL_DAMAGE_VS_WATER:
			lpObj->PentagramJewelOption.MulElementalDamageVsWater += value;
			break;
		case PENTAGRAM_JEWEL_OF_INTEGRITY_OPTION_MUL_ELEMENTAL_DAMAGE_VS_EARTH:
			lpObj->PentagramJewelOption.MulElementalDamageVsEarth += value;
			break;
		case PENTAGRAM_JEWEL_OF_INTEGRITY_OPTION_MUL_ELEMENTAL_DAMAGE_VS_WIND:
			lpObj->PentagramJewelOption.MulElementalDamageVsWind += value;
			break;
		case PENTAGRAM_JEWEL_OF_INTEGRITY_OPTION_MUL_ELEMENTAL_DAMAGE_VS_DARK:
			lpObj->PentagramJewelOption.MulElementalDamageVsDark += value;
			break;
		case PENTAGRAM_JEWEL_OF_INTEGRITY_OPTION_ADD_ELEMENTAL_DAMAGE_PVP:
			lpObj->PentagramJewelOption.AddElementalDamagePvP += value;
			break;
		case PENTAGRAM_JEWEL_OF_INTEGRITY_OPTION_ADD_ELEMENTAL_DAMAGE_PVM:
			lpObj->PentagramJewelOption.AddElementalDamagePvM += value;
			break;
		case PENTAGRAM_JEWEL_OF_INTEGRITY_OPTION_ADD_ELEMENTAL_DAMAGE_RANGE:
			lpObj->PentagramJewelOption.AddElementalDamageRange += value;
			break;
		case PENTAGRAM_JEWEL_OF_INTEGRITY_OPTION_ADD_ELEMENTAL_DAMAGE_MELEE:
			lpObj->PentagramJewelOption.AddElementalDamageMelee += value;
			break;
		case PENTAGRAM_JEWEL_OF_INTEGRITY_OPTION_MUL_ELEMENTAL_DAMAGE_PVP:
			lpObj->PentagramJewelOption.MulElementalDamagePvP += value;
			break;
		case PENTAGRAM_JEWEL_OF_INTEGRITY_OPTION_MUL_ELEMENTAL_DAMAGE_PVM:
			lpObj->PentagramJewelOption.MulElementalDamagePvM += value;
			break;
		case PENTAGRAM_JEWEL_OF_DIVINITY_OPTION_MUL_ELEMENTAL_DEFENSE_SUCCESS_RATE:
			lpObj->PentagramJewelOption.MulElementalDefenseSuccessRate += value;
			break;
		case PENTAGRAM_JEWEL_OF_DIVINITY_OPTION_MUL_ELEMENTAL_DEFENSE_VS_FIRE:
			lpObj->PentagramJewelOption.MulElementalDefenseVsFire += value;
			break;
		case PENTAGRAM_JEWEL_OF_DIVINITY_OPTION_MUL_ELEMENTAL_DEFENSE_VS_WATER:
			lpObj->PentagramJewelOption.MulElementalDefenseVsWater += value;
			break;
		case PENTAGRAM_JEWEL_OF_DIVINITY_OPTION_MUL_ELEMENTAL_DEFENSE_VS_EARTH:
			lpObj->PentagramJewelOption.MulElementalDefenseVsEarth += value;
			break;
		case PENTAGRAM_JEWEL_OF_DIVINITY_OPTION_MUL_ELEMENTAL_DEFENSE_VS_WIND:
			lpObj->PentagramJewelOption.MulElementalDefenseVsWind += value;
			break;
		case PENTAGRAM_JEWEL_OF_DIVINITY_OPTION_MUL_ELEMENTAL_DEFENSE_VS_DARK:
			lpObj->PentagramJewelOption.MulElementalDefenseVsDark += value;
			break;
		case PENTAGRAM_JEWEL_OF_DIVINITY_OPTION_ADD_ELEMENTAL_DEFENSE_PVP:
			lpObj->PentagramJewelOption.AddElementalDefensePvP += value;
			break;
		case PENTAGRAM_JEWEL_OF_DIVINITY_OPTION_ADD_ELEMENTAL_DEFENSE_PVM:
			lpObj->PentagramJewelOption.AddElementalDefensePvM += value;
			break;
		case PENTAGRAM_JEWEL_OF_DIVINITY_OPTION_ADD_ELEMENTAL_DEFENSE_RANGE:
			lpObj->PentagramJewelOption.AddElementalDefenseRange += value;
			break;
		case PENTAGRAM_JEWEL_OF_DIVINITY_OPTION_ADD_ELEMENTAL_DEFENSE_MELEE:
			lpObj->PentagramJewelOption.AddElementalDefenseMelee += value;
			break;
		case PENTAGRAM_JEWEL_OF_DIVINITY_OPTION_ADD_ELEMENTAL_DAMAGE_REDUCTION_PVP:
			lpObj->PentagramJewelOption.AddElementalDamageReductionPvP += value;
			break;
		case PENTAGRAM_JEWEL_OF_DIVINITY_OPTION_ADD_ELEMENTAL_DAMAGE_REDUCTION_PVM:
			lpObj->PentagramJewelOption.AddElementalDamageReductionPvM += value;
			break;
		case PENTAGRAM_JEWEL_OF_GALE_OPTION_ADD_ELEMENTAL_SLOW_RATE:
			lpObj->PentagramJewelOption.AddElementalSlowRate += value;
			break;
		case PENTAGRAM_JEWEL_OF_GALE_OPTION_ADD_STRENGTH:
			lpObj->AddStrength += value;
			break;
		case PENTAGRAM_JEWEL_OF_GALE_OPTION_ADD_DEXTERITY:
			lpObj->AddDexterity += value;
			break;
		case PENTAGRAM_JEWEL_OF_GALE_OPTION_ADD_ENERGY:
			lpObj->AddEnergy += value;
			break;
		case PENTAGRAM_JEWEL_OF_GALE_OPTION_ADD_VITALITY:
			lpObj->AddVitality += value;
			break;
		case PENTAGRAM_JEWEL_OF_GALE_OPTION_ADD_MAX_HP:
			lpObj->AddLife += value;
			break;
		case PENTAGRAM_JEWEL_OF_GALE_OPTION_ADD_MAX_MP:
			lpObj->AddMana += value;
			break;
		case PENTAGRAM_JEWEL_OF_GALE_OPTION_ADD_MAX_BP:
			lpObj->AddBP += value;
			break;
		case PENTAGRAM_JEWEL_OF_GALE_OPTION_ADD_MAX_SD:
			lpObj->AddShield += value;
			break;
		case PENTAGRAM_JEWEL_OF_GALE_OPTION_ADD_ELEMENTAL_EXCELLENT_DAMAGE_RATE_PVP:
			lpObj->PentagramJewelOption.AddElementalExcellentDamageRatePvP += value;
			break;
		case PENTAGRAM_JEWEL_OF_GALE_OPTION_ADD_ELEMENTAL_EXCELLENT_DAMAGE_RATE_PVM:
			lpObj->PentagramJewelOption.AddElementalExcellentDamageRatePvM += value;
			break;
		case PENTAGRAM_JEWEL_OF_GALE_OPTION_ADD_ELEMENTAL_DEBUFF_RATE:
			lpObj->PentagramJewelOption.AddElementalDebuffRate += value;
			break;
	}

	#endif
}

void CPentagramSystem::CGPentagramJewelInsertRecv(PMSG_PENTAGRAM_JEWEL_INSERT_RECV* lpMsg,int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	PMSG_PENTAGRAM_JEWEL_INSERT_SEND pMsg;

	pMsg.header.set(0xEC,0x00,sizeof(pMsg));

	pMsg.result = 0;

	if(INVENTORY_BASE_RANGE(lpMsg->TargetSlot) == 0)
	{
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(INVENTORY_BASE_RANGE(lpMsg->SourceSlot) == 0)
	{
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(lpObj->Inventory[lpMsg->TargetSlot].IsItem() == 0)
	{
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(lpObj->Inventory[lpMsg->SourceSlot].IsItem() == 0)
	{
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(lpObj->Inventory[lpMsg->TargetSlot].IsPentagramItem() == 0)
	{
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(lpObj->Inventory[lpMsg->SourceSlot].IsPentagramJewel() == 0)
	{
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	int SocketSlot = (lpObj->Inventory[lpMsg->SourceSlot].m_Index-GET_ITEM(12,221))/10;

	if(CHECK_RANGE(SocketSlot,MAX_SOCKET_OPTION) == 0)
	{
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(lpObj->Inventory[lpMsg->TargetSlot].m_SocketOption[SocketSlot] != 0xFE)
	{
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if((lpObj->Inventory[lpMsg->TargetSlot].m_SocketOptionBonus%16) != (lpObj->Inventory[lpMsg->SourceSlot].m_SocketOptionBonus%16))
	{
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	PENTAGRAM_JEWEL_INFO* lpPentagramJewelInfo = this->AddPentagramJewelInfo(lpObj,&lpObj->Inventory[lpMsg->SourceSlot],PENTAGRAM_JEWEL_TYPE_INVENTORY);

	if(lpPentagramJewelInfo == 0)
	{
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	pMsg.result = 1;

	memcpy(&pMsg.Type,lpPentagramJewelInfo,sizeof(PENTAGRAM_JEWEL_INFO));

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);

	lpObj->Inventory[lpMsg->TargetSlot].m_SocketOption[SocketSlot] = lpPentagramJewelInfo->Index;

	gItemManager.GCItemModifySend(aIndex,(BYTE)lpMsg->TargetSlot);

	gItemManager.InventoryDelItem(aIndex,(BYTE)lpMsg->SourceSlot);

	gItemManager.GCItemDeleteSend(aIndex,(BYTE)lpMsg->SourceSlot,0);

	this->GCPentagramJewelResultSend(aIndex,1);

	#endif
}

void CPentagramSystem::CGPentagramJewelRemoveRecv(PMSG_PENTAGRAM_JEWEL_REMOVE_RECV* lpMsg,int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	PMSG_PENTAGRAM_JEWEL_REMOVE_SEND pMsg;

	pMsg.header.set(0xEC,0x01,sizeof(pMsg));

	pMsg.result = 0;

	pMsg.Type = 0;

	pMsg.Index = 0;

	if(INVENTORY_BASE_RANGE(lpMsg->SourceSlot) == 0)
	{
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		this->GCPentagramJewelResultSend(aIndex,0);
		return;
	}

	if(lpObj->Inventory[lpMsg->SourceSlot].IsItem() == 0)
	{
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		this->GCPentagramJewelResultSend(aIndex,0);
		return;
	}

	if(lpObj->Inventory[lpMsg->SourceSlot].IsPentagramItem() == 0)
	{
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		this->GCPentagramJewelResultSend(aIndex,0);
		return;
	}

	if(CHECK_RANGE(lpMsg->SocketSlot,MAX_SOCKET_OPTION) == 0)
	{
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		this->GCPentagramJewelResultSend(aIndex,0);
		return;
	}

	PENTAGRAM_JEWEL_INFO* lpPentagramJewelInfo = this->GetPentagramJewelInfo(lpObj,lpObj->Inventory[lpMsg->SourceSlot].m_SocketOption[lpMsg->SocketSlot],PENTAGRAM_JEWEL_TYPE_INVENTORY);

	if(lpPentagramJewelInfo == 0)
	{
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		this->GCPentagramJewelResultSend(aIndex,0);
		return;
	}

	if(gItemManager.CheckItemInventorySpace(lpObj,GET_ITEM(lpPentagramJewelInfo->ItemSection,lpPentagramJewelInfo->ItemType)) == 0)
	{
		pMsg.result = 100;
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		this->GCPentagramJewelResultSend(aIndex,100);
		return;
	}

	PENTAGRAM_JEWEL_REMOVE_INFO PentagramJewelRamoveInfo;

	if(this->GetPentagramJewelRemoveInfo(lpPentagramJewelInfo->ItemLevel,&PentagramJewelRamoveInfo) == 0)
	{
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		this->GCPentagramJewelResultSend(aIndex,0);
		return;
	}

	if((GetLargeRand()%10000) < PentagramJewelRamoveInfo.MixRate[((lpPentagramJewelInfo->Attribute/16)-1)])
	{
		BYTE SocketOption[MAX_SOCKET_OPTION] = {0xFF,0xFF,0xFF,0xFF,0xFF};

		SocketOption[0] = lpPentagramJewelInfo->OptionIndexRank1 | (lpPentagramJewelInfo->OptionLevelRank1*16);
		SocketOption[1] = lpPentagramJewelInfo->OptionIndexRank2 | (lpPentagramJewelInfo->OptionLevelRank2*16);
		SocketOption[2] = lpPentagramJewelInfo->OptionIndexRank3 | (lpPentagramJewelInfo->OptionLevelRank3*16);
		SocketOption[3] = lpPentagramJewelInfo->OptionIndexRank4 | (lpPentagramJewelInfo->OptionLevelRank4*16);
		SocketOption[4] = lpPentagramJewelInfo->OptionIndexRank5 | (lpPentagramJewelInfo->OptionLevelRank5*16);

		GDCreateItemSend(aIndex,0xEB,0,0,GET_ITEM(lpPentagramJewelInfo->ItemSection,lpPentagramJewelInfo->ItemType),lpPentagramJewelInfo->ItemLevel,0,0,0,0,aIndex,0,0,0,0,SocketOption,lpPentagramJewelInfo->Attribute,0);

		pMsg.result = 1;

		pMsg.Type = lpPentagramJewelInfo->Type;

		pMsg.Index = lpPentagramJewelInfo->Index;

		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);

		lpObj->Inventory[lpMsg->SourceSlot].m_SocketOption[lpMsg->SocketSlot] = 0xFE;

		gItemManager.GCItemModifySend(aIndex,(BYTE)lpMsg->SourceSlot);

		this->DelPentagramJewelInfo(lpObj,lpPentagramJewelInfo->Index,PENTAGRAM_JEWEL_TYPE_INVENTORY);

		this->GCPentagramJewelResultSend(aIndex,1);
	}
	else
	{
		pMsg.result = 2;

		pMsg.Type = lpPentagramJewelInfo->Type;

		pMsg.Index = lpPentagramJewelInfo->Index;

		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);

		lpObj->Inventory[lpMsg->SourceSlot].m_SocketOption[lpMsg->SocketSlot] = 0xFE;

		gItemManager.GCItemModifySend(aIndex,(BYTE)lpMsg->SourceSlot);

		this->DelPentagramJewelInfo(lpObj,lpPentagramJewelInfo->Index,PENTAGRAM_JEWEL_TYPE_INVENTORY);

		this->GCPentagramJewelResultSend(aIndex,2);
	}

	#endif
}

void CPentagramSystem::CGPentagramJewelRefineRecv(PMSG_PENTAGRAM_JEWEL_REFINE_RECV* lpMsg,int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	if(lpObj->ChaosLock != 0)
	{
		return;
	}

	if(lpObj->PShopOpen != 0)
	{
		return;
	}

	lpObj->ChaosLock = 1;

	lpObj->ChaosMoney = 0;

	lpObj->ChaosSuccessRate = 0;

	lpObj->IsChaosMixCompleted = 1;

	switch(lpMsg->type)
	{
		case CHAOS_MIX_PENTAGRAM_MITHRIL:
			gChaosBox.PentagramMithrilMix(lpObj);
			break;
		case CHAOS_MIX_PENTAGRAM_ELIXIR:
			gChaosBox.PentagramElixirMix(lpObj);
			break;
		case CHAOS_MIX_PENTAGRAM_JEWEL:
			gChaosBox.PentagramJewelMix(lpObj);
			break;
		case CHAOS_MIX_PENTAGRAM_DECOMPOSITE1:
			gChaosBox.PentagramDecompositeMix(lpObj,0);
			break;
		case CHAOS_MIX_PENTAGRAM_DECOMPOSITE2:
			gChaosBox.PentagramDecompositeMix(lpObj,1);
			break;
		case CHAOS_MIX_PENTAGRAM_DECOMPOSITE3:
			gChaosBox.PentagramDecompositeMix(lpObj,2);
			break;
		case CHAOS_MIX_PENTAGRAM_DECOMPOSITE4:
			gChaosBox.PentagramDecompositeMix(lpObj,3);
			break;
		case CHAOS_MIX_PENTAGRAM_DECOMPOSITE5:
			gChaosBox.PentagramDecompositeMix(lpObj,4);
			break;
		case CHAOS_MIX_PENTAGRAM_DECOMPOSITE6:
			gChaosBox.PentagramDecompositeMix(lpObj,5);
			break;
		case CHAOS_MIX_PENTAGRAM_DECOMPOSITE7:
			gChaosBox.PentagramDecompositeMix(lpObj,6);
			break;
		case CHAOS_MIX_PENTAGRAM_DECOMPOSITE8:
			gChaosBox.PentagramDecompositeMix(lpObj,7);
			break;
		case CHAOS_MIX_PENTAGRAM_DECOMPOSITE9:
			gChaosBox.PentagramDecompositeMix(lpObj,8);
			break;
		case CHAOS_MIX_PENTAGRAM_DECOMPOSITE10:
			gChaosBox.PentagramDecompositeMix(lpObj,9);
			break;
		case CHAOS_MIX_PENTAGRAM_DECOMPOSITE11:
			gChaosBox.PentagramDecompositeMix(lpObj,10);
			break;
		case CHAOS_MIX_PENTAGRAM_DECOMPOSITE12:
			gChaosBox.PentagramDecompositeMix(lpObj,11);
			break;
		case CHAOS_MIX_PENTAGRAM_DECOMPOSITE13:
			gChaosBox.PentagramDecompositeMix(lpObj,12);
			break;
		case CHAOS_MIX_PENTAGRAM_DECOMPOSITE14:
			gChaosBox.PentagramDecompositeMix(lpObj,13);
			break;
		case CHAOS_MIX_PENTAGRAM_DECOMPOSITE15:
			gChaosBox.PentagramDecompositeMix(lpObj,14);
			break;
	}

	#endif
}

void CPentagramSystem::CGPentagramJewelUpgradeRecv(PMSG_PENTAGRAM_JEWEL_UPGRADE_RECV* lpMsg,int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	if(lpObj->ChaosLock != 0)
	{
		return;
	}

	if(lpObj->PShopOpen != 0)
	{
		return;
	}

	lpObj->ChaosLock = 1;

	lpObj->ChaosMoney = 0;

	lpObj->ChaosSuccessRate = 0;

	lpObj->IsChaosMixCompleted = 1;

	switch(lpMsg->type)
	{
		case CHAOS_MIX_PENTAGRAM_JEWEL_UPGRADE_LEVEL:
			gChaosBox.PentagramJewelUpgradeLevelMix(lpObj,lpMsg->info);
			break;
		case CHAOS_MIX_PENTAGRAM_JEWEL_UPGRADE_RANK:
			gChaosBox.PentagramJewelUpgradeRankMix(lpObj,lpMsg->info);
			break;
	}

	#endif
}

void CPentagramSystem::GCPentagramJewelResultSend(int aIndex,int result) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	PMSG_PENTAGRAM_JEWEL_RESULT_SEND pMsg;

	pMsg.header.set(0xEC,0x04,sizeof(pMsg));

	pMsg.result = result;

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CPentagramSystem::GCPentagramJewelInfoSend(int aIndex,int type) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	LPOBJ lpObj = &gObj[aIndex];

	PENTAGRAM_JEWEL_INFO* lpPentagramJewelInfo = ((type==PENTAGRAM_JEWEL_TYPE_INVENTORY)?lpObj->PentagramJewelInfo_Inventory:lpObj->PentagramJewelInfo_Warehouse);

	BYTE send[8192];

	PMSG_PENTAGRAM_JEWEL_INFO_SEND pMsg;

	pMsg.header.set(0xEE,0x01,0);

	int size = sizeof(pMsg);

	pMsg.result = 0;

	pMsg.count = 0;

	pMsg.type = type;

	PENTAGRAM_JEWEL_INFO info;

	for(int n=0;n < MAX_PENTAGRAM_JEWEL_INFO;n++)
	{
		if(lpPentagramJewelInfo[n].Index != 0xFF)
		{
			memcpy(&info,&lpPentagramJewelInfo[n],sizeof(info));

			memcpy(&send[size],&info,sizeof(info));
			size += sizeof(info);

			pMsg.count++;
		}
	}

	pMsg.result = ((pMsg.count>0)?1:0);

	pMsg.header.size[0] = SET_NUMBERHB(size);
	pMsg.header.size[1] = SET_NUMBERLB(size);

	memcpy(send,&pMsg,sizeof(pMsg));

	DataSend(aIndex,send,size);

	#endif
}

void CPentagramSystem::GCPentagramJewelTradeInfoSend(int aIndex,int bIndex,CItem* lpItem) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	if(lpItem->IsItem() == 0 || lpItem->IsPentagramItem() == 0)
	{
		return;
	}

	LPOBJ lpObj = &gObj[aIndex];

	BYTE send[8192];

	PMSG_PENTAGRAM_JEWEL_INFO_SEND pMsg;

	pMsg.header.set(0xEE,0x01,0);

	int size = sizeof(pMsg);

	pMsg.result = 1;

	pMsg.count = 0;

	pMsg.type = PENTAGRAM_JEWEL_TYPE_TRADE;

	PENTAGRAM_JEWEL_INFO info;

	for(int n=0;n < MAX_SOCKET_OPTION;n++)
	{
		PENTAGRAM_JEWEL_INFO* lpPentagramJewelInfo = this->GetPentagramJewelInfo(lpObj,lpItem->m_SocketOption[n],PENTAGRAM_JEWEL_TYPE_INVENTORY);

		if(lpPentagramJewelInfo != 0)
		{
			memcpy(&info,lpPentagramJewelInfo,sizeof(info));

			info.Type = pMsg.type;

			memcpy(&send[size],&info,sizeof(info));
			size += sizeof(info);

			pMsg.count++;
		}
	}

	if(pMsg.count > 0)
	{
		pMsg.header.size[0] = SET_NUMBERHB(size);
		pMsg.header.size[1] = SET_NUMBERLB(size);

		memcpy(send,&pMsg,sizeof(pMsg));

		DataSend(bIndex,send,size);
	}

	#endif
}

void CPentagramSystem::GCPentagramJewelPShopInfoSend(int aIndex,int bIndex) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	LPOBJ lpObj = &gObj[bIndex];

	BYTE send[8192];

	PMSG_PENTAGRAM_JEWEL_INFO_SEND pMsg;

	pMsg.header.set(0xEE,0x01,0);

	int size = sizeof(pMsg);

	pMsg.result = 1;

	pMsg.count = 0;

	pMsg.type = PENTAGRAM_JEWEL_TYPE_PSHOP;

	PENTAGRAM_JEWEL_INFO info;

	for(int n=INVENTORY_EXT4_SIZE;n < INVENTORY_FULL_SIZE;n++)
	{
		if(lpObj->Inventory[n].IsItem() != 0 && lpObj->Inventory[n].IsPentagramItem() != 0)
		{
			for(int i=0;i < MAX_SOCKET_OPTION;i++)
			{
				PENTAGRAM_JEWEL_INFO* lpPentagramJewelInfo = this->GetPentagramJewelInfo(lpObj,lpObj->Inventory[n].m_SocketOption[i],PENTAGRAM_JEWEL_TYPE_INVENTORY);

				if(lpPentagramJewelInfo != 0)
				{
					memcpy(&info,lpPentagramJewelInfo,sizeof(info));

					info.Type = pMsg.type;

					memcpy(&send[size],&info,sizeof(info));
					size += sizeof(info);

					pMsg.count++;
				}
			}
		}
	}

	if(pMsg.count > 0)
	{
		pMsg.header.size[0] = SET_NUMBERHB(size);
		pMsg.header.size[1] = SET_NUMBERLB(size);

		memcpy(send,&pMsg,sizeof(pMsg));

		DataSend(aIndex,send,size);
	}

	#endif
}

void CPentagramSystem::DGPentagramJewelInfoRecv(SDHP_PENTAGRAM_JEWEL_INFO_RECV* lpMsg) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	if(gObjIsAccountValid(lpMsg->index,lpMsg->account) == 0)
	{
		LogAdd(LOG_RED,"[DGPentagramJewelInfoRecv] Invalid Account [%d](%s)",lpMsg->index,lpMsg->account);
		CloseClient(lpMsg->index);
		return;
	}

	LPOBJ lpObj = &gObj[lpMsg->index];

	PENTAGRAM_JEWEL_INFO* lpPentagramJewelInfo = ((lpMsg->type==PENTAGRAM_JEWEL_TYPE_INVENTORY)?lpObj->PentagramJewelInfo_Inventory:lpObj->PentagramJewelInfo_Warehouse);

	for(int n=0;n < lpMsg->count;n++)
	{
		SDHP_PENTAGRAM_JEWEL_INFO* lpInfo = (SDHP_PENTAGRAM_JEWEL_INFO*)(((BYTE*)lpMsg)+sizeof(SDHP_PENTAGRAM_JEWEL_INFO_RECV)+(sizeof(SDHP_PENTAGRAM_JEWEL_INFO)*n));

		if(lpInfo->Type != lpMsg->type)
		{
			continue;
		}

		if(lpInfo->Index >= MAX_PENTAGRAM_JEWEL_INFO)
		{
			continue;
		}

		lpPentagramJewelInfo[lpInfo->Index].Type = lpInfo->Type;
		lpPentagramJewelInfo[lpInfo->Index].Index = lpInfo->Index;
		lpPentagramJewelInfo[lpInfo->Index].Attribute = lpInfo->Attribute;
		lpPentagramJewelInfo[lpInfo->Index].ItemSection = lpInfo->ItemSection;
		lpPentagramJewelInfo[lpInfo->Index].ItemType = lpInfo->ItemType;
		lpPentagramJewelInfo[lpInfo->Index].ItemLevel = lpInfo->ItemLevel;
		lpPentagramJewelInfo[lpInfo->Index].OptionIndexRank1 = lpInfo->OptionIndexRank1;
		lpPentagramJewelInfo[lpInfo->Index].OptionLevelRank1 = lpInfo->OptionLevelRank1;
		lpPentagramJewelInfo[lpInfo->Index].OptionIndexRank2 = lpInfo->OptionIndexRank2;
		lpPentagramJewelInfo[lpInfo->Index].OptionLevelRank2 = lpInfo->OptionLevelRank2;
		lpPentagramJewelInfo[lpInfo->Index].OptionIndexRank3 = lpInfo->OptionIndexRank3;
		lpPentagramJewelInfo[lpInfo->Index].OptionLevelRank3 = lpInfo->OptionLevelRank3;
		lpPentagramJewelInfo[lpInfo->Index].OptionIndexRank4 = lpInfo->OptionIndexRank4;
		lpPentagramJewelInfo[lpInfo->Index].OptionLevelRank4 = lpInfo->OptionLevelRank4;
		lpPentagramJewelInfo[lpInfo->Index].OptionIndexRank5 = lpInfo->OptionIndexRank5;
		lpPentagramJewelInfo[lpInfo->Index].OptionLevelRank5 = lpInfo->OptionLevelRank5;
	}

	this->GCPentagramJewelInfoSend(lpObj->Index,lpMsg->type);

	#endif
}

void CPentagramSystem::GDPentagramJewelInfoSend(int aIndex,int type) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	if(gObjIsAccountValid(aIndex,gObj[aIndex].Account) == 0)
	{
		return;
	}

	SDHP_PENTAGRAM_JEWEL_INFO_SEND pMsg;

	pMsg.header.set(0x23,0x00,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,gObj[aIndex].Account,sizeof(pMsg.account));

	memcpy(pMsg.name,gObj[aIndex].Name,sizeof(pMsg.name));

	pMsg.type = type;

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CPentagramSystem::GDPentagramJewelInfoSaveSend(int aIndex,int type) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	LPOBJ lpObj = &gObj[aIndex];

	PENTAGRAM_JEWEL_INFO* lpPentagramJewelInfo = ((type==PENTAGRAM_JEWEL_TYPE_INVENTORY)?lpObj->PentagramJewelInfo_Inventory:lpObj->PentagramJewelInfo_Warehouse);

	BYTE send[8192];

	SDHP_PENTAGRAM_JEWEL_INFO_SAVE_SEND pMsg;

	pMsg.header.set(0x23,0x30,0);

	int size = sizeof(pMsg);

	pMsg.index = aIndex;

	memcpy(pMsg.account,lpObj->Account,sizeof(pMsg.account));

	memcpy(pMsg.name,lpObj->Name,sizeof(pMsg.name));

	pMsg.count = 0;

	SDHP_PENTAGRAM_JEWEL_INFO_SAVE info;

	for(int n=0;n < MAX_PENTAGRAM_JEWEL_INFO;n++)
	{
		if(lpPentagramJewelInfo[n].Index != 0xFF)
		{
			info.Type = lpPentagramJewelInfo[n].Type;
			info.Index = lpPentagramJewelInfo[n].Index;
			info.Attribute = lpPentagramJewelInfo[n].Attribute;
			info.ItemSection = lpPentagramJewelInfo[n].ItemSection;
			info.ItemType = lpPentagramJewelInfo[n].ItemType;
			info.ItemLevel = lpPentagramJewelInfo[n].ItemLevel;
			info.OptionIndexRank1 = lpPentagramJewelInfo[n].OptionIndexRank1;
			info.OptionLevelRank1 = lpPentagramJewelInfo[n].OptionLevelRank1;
			info.OptionIndexRank2 = lpPentagramJewelInfo[n].OptionIndexRank2;
			info.OptionLevelRank2 = lpPentagramJewelInfo[n].OptionLevelRank2;
			info.OptionIndexRank3 = lpPentagramJewelInfo[n].OptionIndexRank3;
			info.OptionLevelRank3 = lpPentagramJewelInfo[n].OptionLevelRank3;
			info.OptionIndexRank4 = lpPentagramJewelInfo[n].OptionIndexRank4;
			info.OptionLevelRank4 = lpPentagramJewelInfo[n].OptionLevelRank4;
			info.OptionIndexRank5 = lpPentagramJewelInfo[n].OptionIndexRank5;
			info.OptionLevelRank5 = lpPentagramJewelInfo[n].OptionLevelRank5;

			memcpy(&send[size],&info,sizeof(info));
			size += sizeof(info);

			pMsg.count++;
		}
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

void CPentagramSystem::GDPentagramJewelInsertSaveSend(int aIndex,PENTAGRAM_JEWEL_INFO* lpInfo) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	SDHP_PENTAGRAM_JEWEL_INSERT_SAVE_SEND pMsg;

	pMsg.header.set(0x23,0x31,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,gObj[aIndex].Account,sizeof(pMsg.account));

	memcpy(pMsg.name,gObj[aIndex].Name,sizeof(pMsg.account));

	pMsg.Type = lpInfo->Type;

	pMsg.Index = lpInfo->Index;

	pMsg.Attribute = lpInfo->Attribute;

	pMsg.ItemSection = lpInfo->ItemSection;

	pMsg.ItemType = lpInfo->ItemType;

	pMsg.ItemLevel = lpInfo->ItemLevel;

	pMsg.OptionIndexRank1 = lpInfo->OptionIndexRank1;

	pMsg.OptionLevelRank1 = lpInfo->OptionLevelRank1;

	pMsg.OptionIndexRank2 = lpInfo->OptionIndexRank2;

	pMsg.OptionLevelRank2 = lpInfo->OptionLevelRank2;

	pMsg.OptionIndexRank3 = lpInfo->OptionIndexRank3;

	pMsg.OptionLevelRank3 = lpInfo->OptionLevelRank3;

	pMsg.OptionIndexRank4 = lpInfo->OptionIndexRank4;

	pMsg.OptionLevelRank4 = lpInfo->OptionLevelRank4;

	pMsg.OptionIndexRank5 = lpInfo->OptionIndexRank5;

	pMsg.OptionLevelRank5 = lpInfo->OptionLevelRank5;

	gDataServerConnection.DataSend((BYTE*)&pMsg,sizeof(pMsg));

	#endif
}

void CPentagramSystem::GDPentagramJewelDeleteSaveSend(int aIndex,int type,int index) // OK
{
	#if(GAMESERVER_UPDATE>=701)

	SDHP_PENTAGRAM_JEWEL_DELETE_SAVE_SEND pMsg;

	pMsg.header.set(0x23,0x32,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,gObj[aIndex].Account,sizeof(pMsg.account));

	memcpy(pMsg.name,gObj[aIndex].Name,sizeof(pMsg.account));

	pMsg.Type = type;

	pMsg.Index = index;

	gDataServerConnection.DataSend((BYTE*)&pMsg,sizeof(pMsg));

	#endif
}
