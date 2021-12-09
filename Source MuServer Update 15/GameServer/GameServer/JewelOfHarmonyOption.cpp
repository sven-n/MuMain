// JewelOfHarmonyOption.cpp: implementation of the CJewelOfHarmonyOption class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "JewelOfHarmonyOption.h"
#include "ItemManager.h"
#include "MemScript.h"
#include "ObjectManager.h"
#include "RandomManager.h"
#include "ServerInfo.h"
#include "Util.h"

CJewelOfHarmonyOption gJewelOfHarmonyOption;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CJewelOfHarmonyOption::CJewelOfHarmonyOption() // OK
{
	this->Init();
}

CJewelOfHarmonyOption::~CJewelOfHarmonyOption() // OK
{

}

void CJewelOfHarmonyOption::Init() // OK
{
	for(int n=0;n < MAX_JEWEL_OF_HARMONY_OPTION_TYPE;n++)for(int i=0;i < MAX_JEWEL_OF_HARMONY_OPTION;i++)
	{
		this->m_JewelOfHarmonyOptionInfo[n][i].Index = -1;
	}
}

void CJewelOfHarmonyOption::Load(char* path) // OK
{
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

			int section = lpMemScript->GetNumber();

			while(true)
			{
				if(section >= 0 && section < MAX_JEWEL_OF_HARMONY_OPTION_TYPE)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}
			
					JEWEL_OF_HARMONY_OPTION_INFO info;

					memset(&info,0,sizeof(info));

					info.Index = lpMemScript->GetNumber();

					strcpy_s(info.Name,lpMemScript->GetAsString());

					info.Rate = lpMemScript->GetAsNumber();

					info.Level = lpMemScript->GetAsNumber();

					for(int n=0;n < MAX_JEWEL_OF_HARMONY_OPTION_TABLE;n++)
					{
						info.ValueTable[n] = lpMemScript->GetAsNumber();

						info.MoneyTable[n] = lpMemScript->GetAsNumber();
					}

					this->SetInfo(section,info);
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
}

void CJewelOfHarmonyOption::SetInfo(int type,JEWEL_OF_HARMONY_OPTION_INFO info) // OK
{
	if(type < 0 || type >= MAX_JEWEL_OF_HARMONY_OPTION_TYPE)
	{
		return;
	}

	if(info.Index < 0 || info.Index >= MAX_JEWEL_OF_HARMONY_OPTION)
	{
		return;
	}

	this->m_JewelOfHarmonyOptionInfo[type][info.Index] = info;
}

JEWEL_OF_HARMONY_OPTION_INFO* CJewelOfHarmonyOption::GetInfo(int type,int index) // OK
{
	if(type < 0 || type >= MAX_JEWEL_OF_HARMONY_OPTION_TYPE)
	{
		return 0;
	}

	if(index < 0 || index >= MAX_JEWEL_OF_HARMONY_OPTION)
	{
		return 0;
	}

	if(this->m_JewelOfHarmonyOptionInfo[type][index].Index != index)
	{
		return 0;
	}

	return &this->m_JewelOfHarmonyOptionInfo[type][index];
}

bool CJewelOfHarmonyOption::IsJewelOfHarmonyItem(CItem* lpItem) // OK
{
	if(lpItem->IsSocketItem() != 0)
	{
		return 0;
	}

	if(((lpItem->m_JewelOfHarmonyOption & 0xF0) >> 4) != JEWEL_OF_HARMONY_OPTION_NONE)
	{
		return 1;
	}

	return 0;
}

int CJewelOfHarmonyOption::GetJewelOfHarmonyItemOptionType(CItem* lpItem) // OK
{
	int type = JEWEL_OF_HARMONY_ITEM_OPTION_TYPE_NONE;

	if(lpItem->m_Index >= GET_ITEM(0,0) && lpItem->m_Index < GET_ITEM(5,0) && lpItem->m_Index != GET_ITEM(4,7) && lpItem->m_Index != GET_ITEM(4,15))
	{
		type = JEWEL_OF_HARMONY_ITEM_OPTION_TYPE_WEAPON;
	}
	else if(lpItem->m_Index >= GET_ITEM(5,0) && lpItem->m_Index < GET_ITEM(6,0))
	{
		type = JEWEL_OF_HARMONY_ITEM_OPTION_TYPE_STAFF;
	}
	else if(lpItem->m_Index >= GET_ITEM(6,0) && lpItem->m_Index < GET_ITEM(12,0))
	{
		type = JEWEL_OF_HARMONY_ITEM_OPTION_TYPE_ARMOR;
	}

	return type;
}

int CJewelOfHarmonyOption::GetJewelOfHarmonyItemRestoreMoney(CItem* lpItem) // OK
{
	int ItemOptionType = this->GetJewelOfHarmonyItemOptionType(lpItem);

	if(ItemOptionType == JEWEL_OF_HARMONY_ITEM_OPTION_TYPE_NONE)
	{
		return -1;
	}

	int ItemOption = (lpItem->m_JewelOfHarmonyOption & 0xF0) >> 4;

	if(ItemOption == JEWEL_OF_HARMONY_OPTION_NONE)
	{
		return -1;
	}

	JEWEL_OF_HARMONY_OPTION_INFO* lpInfo = this->GetInfo(ItemOptionType,ItemOption);

	if(lpInfo == 0)
	{
		return -1;
	}

	return lpInfo->MoneyTable[(lpItem->m_JewelOfHarmonyOption & 0x0F)];
}

int CJewelOfHarmonyOption::GetJewelOfHarmonyRandomOption(CItem* lpItem) // OK
{
	int ItemOptionType = this->GetJewelOfHarmonyItemOptionType(lpItem);

	if(ItemOptionType == JEWEL_OF_HARMONY_ITEM_OPTION_TYPE_NONE)
	{
		return 0;
	}

	int JewelOfHarmonyOption = 0;

	CRandomManager RandomManager;

	for(int n=0;n < MAX_JEWEL_OF_HARMONY_OPTION;n++)
	{
		JEWEL_OF_HARMONY_OPTION_INFO* lpInfo = this->GetInfo(ItemOptionType,n);

		if(lpInfo == 0)
		{
			continue;
		}

		if(lpInfo->Level > lpItem->m_Level)
		{
			continue;
		}

		if(lpItem->m_RequireStrength == 0 && ItemOptionType == JEWEL_OF_HARMONY_ITEM_OPTION_TYPE_WEAPON && lpInfo->Index == JEWEL_OF_HARMONY_WEAPON_OPTION_SUB_REQUIRE_STRENGTH)
		{
			continue;
		}

		if(lpItem->m_RequireStrength == 0 && ItemOptionType == JEWEL_OF_HARMONY_ITEM_OPTION_TYPE_STAFF && lpInfo->Index == JEWEL_OF_HARMONY_STAFF_OPTION_SUB_REQUIRE_STRENGTH)
		{
			continue;
		}

		if(lpItem->m_RequireDexterity == 0 && ItemOptionType == JEWEL_OF_HARMONY_ITEM_OPTION_TYPE_WEAPON && lpInfo->Index == JEWEL_OF_HARMONY_WEAPON_OPTION_SUB_REQUIRE_DEXTERITY)
		{
			continue;
		}

		if(lpItem->m_RequireDexterity == 0 && ItemOptionType == JEWEL_OF_HARMONY_ITEM_OPTION_TYPE_STAFF && lpInfo->Index == JEWEL_OF_HARMONY_STAFF_OPTION_SUB_REQUIRE_DEXTERITY)
		{
			continue;
		}

		RandomManager.AddElement(lpInfo->Index,lpInfo->Rate);
	}

	if(RandomManager.GetRandomElement(&JewelOfHarmonyOption) != 0)
	{
		return JewelOfHarmonyOption;
	}
	else
	{
		return 0;
	}
}

int CJewelOfHarmonyOption::GetJewelOfHarmonySubRequireStrOption(CItem* lpItem) // OK
{
	if(this->IsJewelOfHarmonyItem(lpItem) == 0)
	{
		return 0;
	}

	int ItemOptionType = this->GetJewelOfHarmonyItemOptionType(lpItem);

	if(ItemOptionType == JEWEL_OF_HARMONY_ITEM_OPTION_TYPE_NONE)
	{
		return 0;
	}

	int ItemOption = (lpItem->m_JewelOfHarmonyOption & 0xF0) >> 4;

	if(ItemOption == JEWEL_OF_HARMONY_OPTION_NONE)
	{
		return 0;
	}

	JEWEL_OF_HARMONY_OPTION_INFO* lpInfo = this->GetInfo(ItemOptionType,ItemOption);

	if(lpInfo == 0)
	{
		return 0;
	}

	int ItemOptionLevel = (lpItem->m_JewelOfHarmonyOption & 0x0F);

	if(ItemOptionLevel > lpItem->m_Level)
	{
		return 0;
	}

	if(ItemOptionType == JEWEL_OF_HARMONY_ITEM_OPTION_TYPE_WEAPON && ItemOption == JEWEL_OF_HARMONY_WEAPON_OPTION_SUB_REQUIRE_STRENGTH)
	{
		return lpInfo->ValueTable[ItemOptionLevel];
	}

	if(ItemOptionType == JEWEL_OF_HARMONY_ITEM_OPTION_TYPE_STAFF && ItemOption == JEWEL_OF_HARMONY_STAFF_OPTION_SUB_REQUIRE_STRENGTH)
	{
		return lpInfo->ValueTable[ItemOptionLevel];
	}

	return 0;
}

int CJewelOfHarmonyOption::GetJewelOfHarmonySubRequireDexOption(CItem* lpItem) // OK
{
	if(this->IsJewelOfHarmonyItem(lpItem) == 0)
	{
		return 0;
	}

	int ItemOptionType = this->GetJewelOfHarmonyItemOptionType(lpItem);

	if(ItemOptionType == JEWEL_OF_HARMONY_ITEM_OPTION_TYPE_NONE)
	{
		return 0;
	}

	int ItemOption = (lpItem->m_JewelOfHarmonyOption & 0xF0) >> 4;

	if(ItemOption == JEWEL_OF_HARMONY_OPTION_NONE)
	{
		return 0;
	}

	JEWEL_OF_HARMONY_OPTION_INFO* lpInfo = this->GetInfo(ItemOptionType,ItemOption);

	if(lpInfo == 0)
	{
		return 0;
	}

	int ItemOptionLevel = (lpItem->m_JewelOfHarmonyOption & 0x0F);

	if(ItemOptionLevel > lpItem->m_Level)
	{
		return 0;
	}

	if(ItemOptionType == JEWEL_OF_HARMONY_ITEM_OPTION_TYPE_WEAPON && ItemOption == JEWEL_OF_HARMONY_WEAPON_OPTION_SUB_REQUIRE_DEXTERITY)
	{
		return lpInfo->ValueTable[ItemOptionLevel];
	}

	if(ItemOptionType == JEWEL_OF_HARMONY_ITEM_OPTION_TYPE_STAFF && ItemOption == JEWEL_OF_HARMONY_STAFF_OPTION_SUB_REQUIRE_DEXTERITY)
	{
		return lpInfo->ValueTable[ItemOptionLevel];
	}

	return 0;
}

bool CJewelOfHarmonyOption::AddJewelOfHarmonyOption(LPOBJ lpObj,int SourceSlot,int TargetSlot) // OK
{
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

	if(lpItem->IsSetItem() != 0 && gServerInfo.m_SetItemAcceptHarmonySwitch == 0)
	{
		return 0;
	}

	if(lpItem->IsJewelOfHarmonyItem() != 0)
	{
		return 0;
	}

	if(lpItem->IsSocketItem() != 0)
	{
		return 0;
	}

	if((lpItem->m_Index/MAX_ITEM_TYPE) >= 7 && (lpItem->m_Index/MAX_ITEM_TYPE) <= 11 && (lpItem->m_Index%MAX_ITEM_TYPE) >= 62 && (lpItem->m_Index%MAX_ITEM_TYPE) <= 72) // Lucky Item
	{
		return 0;
	}

	int ItemOptionType = this->GetJewelOfHarmonyItemOptionType(lpItem);

	if(ItemOptionType == JEWEL_OF_HARMONY_ITEM_OPTION_TYPE_NONE)
	{
		return 0;
	}

	int ItemOption = this->GetJewelOfHarmonyRandomOption(lpItem);
	
	if(ItemOption == JEWEL_OF_HARMONY_OPTION_NONE)
	{
		return 0;
	}

	JEWEL_OF_HARMONY_OPTION_INFO* lpInfo = this->GetInfo(ItemOptionType,ItemOption);

	if(lpInfo == 0)
	{
		return 0;
	}

	int ItemOptionLevel = lpInfo->Level;

	if((GetLargeRand()%100) < gServerInfo.m_HarmonySuccessRate[lpObj->AccountLevel])
	{
		lpItem->m_JewelOfHarmonyOption = (ItemOption << 4) | (ItemOptionLevel & 0x0F);

		lpItem->Convert(lpItem->m_Index,lpItem->m_Option1,lpItem->m_Option2,lpItem->m_Option3,lpItem->m_NewOption,lpItem->m_SetOption,lpItem->m_JewelOfHarmonyOption,lpItem->m_ItemOptionEx,lpItem->m_SocketOption,lpItem->m_SocketOptionBonus);

		gObjectManager.CharacterMakePreviewCharSet(lpObj->Index);
	}

	return 1;
}

bool CJewelOfHarmonyOption::AddSmeltStoneOption(LPOBJ lpObj,int SourceSlot,int TargetSlot) // OK
{
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

	if(lpItem->IsJewelOfHarmonyItem() == 0)
	{
		return 0;
	}

	int ItemOptionType = this->GetJewelOfHarmonyItemOptionType(lpItem);

	if(ItemOptionType == JEWEL_OF_HARMONY_ITEM_OPTION_TYPE_NONE)
	{
		return 0;
	}

	int ItemOption = (lpItem->m_JewelOfHarmonyOption & 0xF0) >> 4;

	if(ItemOption == JEWEL_OF_HARMONY_OPTION_NONE)
	{
		return 0;
	}

	JEWEL_OF_HARMONY_OPTION_INFO* lpInfo = this->GetInfo(ItemOptionType,ItemOption);

	if(lpItem == 0)
	{
		return 0;
	}

	int ItemOptionLevel = (lpItem->m_JewelOfHarmonyOption & 0x0F);

	if(ItemOptionLevel >= lpItem->m_Level || ItemOptionLevel >= 13)
	{
		return 0;
	}

	int rate = gServerInfo.m_SmeltStoneSuccessRate1[lpObj->AccountLevel];

	if(lpObj->Inventory[SourceSlot].m_Index == GET_ITEM(14,44))
	{
		rate = gServerInfo.m_SmeltStoneSuccessRate2[lpObj->AccountLevel];
	}

	if((GetLargeRand()%100) < rate)
	{
		ItemOptionLevel++;
	}
	else
	{
		ItemOptionLevel = lpInfo->Level;
	}

	lpItem->m_JewelOfHarmonyOption = (ItemOption << 4) | (ItemOptionLevel & 0x0F);

	lpItem->Convert(lpItem->m_Index,lpItem->m_Option1,lpItem->m_Option2,lpItem->m_Option3,lpItem->m_NewOption,lpItem->m_SetOption,lpItem->m_JewelOfHarmonyOption,lpItem->m_ItemOptionEx,lpItem->m_SocketOption,lpItem->m_SocketOptionBonus);

	gObjectManager.CharacterMakePreviewCharSet(lpObj->Index);
	return 1;
}

bool CJewelOfHarmonyOption::AddJewelOfElevationOption(LPOBJ lpObj,int SourceSlot,int TargetSlot) // OK
{
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

	/*
	if(lpItem->IsSetItem() != 0 && gServerInfo.m_SetItemAcceptHarmonySwitch == 0)
	{
		return 0;
	}
	*/ // CHECK LATER

	if(lpItem->IsJewelOfHarmonyItem() != 0)
	{
		return 0;
	}

	if(lpItem->IsSocketItem() != 0)
	{
		return 0;
	}

	if(lpItem->IsLuckyItem() == 0)
	{
		return 0;
	}

	int ItemOptionType = this->GetJewelOfHarmonyItemOptionType(lpItem);

	if(ItemOptionType == JEWEL_OF_HARMONY_ITEM_OPTION_TYPE_NONE)
	{
		return 0;
	}

	int ItemOption = this->GetJewelOfHarmonyRandomOption(lpItem);
	
	if(ItemOption == JEWEL_OF_HARMONY_OPTION_NONE)
	{
		return 0;
	}

	JEWEL_OF_HARMONY_OPTION_INFO* lpInfo = this->GetInfo(ItemOptionType,ItemOption);

	if(lpInfo == 0)
	{
		return 0;
	}

	int ItemOptionLevel = ((lpItem->m_Level>13)?13:lpItem->m_Level);

	if((GetLargeRand()%100) < gServerInfo.m_HarmonySuccessRate[lpObj->AccountLevel])
	{
		lpItem->m_JewelOfHarmonyOption = (ItemOption << 4) | (ItemOptionLevel & 0x0F);

		lpItem->Convert(lpItem->m_Index,lpItem->m_Option1,lpItem->m_Option2,lpItem->m_Option3,lpItem->m_NewOption,lpItem->m_SetOption,lpItem->m_JewelOfHarmonyOption,lpItem->m_ItemOptionEx,lpItem->m_SocketOption,lpItem->m_SocketOptionBonus);

		gObjectManager.CharacterMakePreviewCharSet(lpObj->Index);
	}

	return 1;
}

void CJewelOfHarmonyOption::CalcJewelOfHarmonyOption(LPOBJ lpObj,bool flag) // OK
{
	for(int n=0;n < INVENTORY_WEAR_SIZE;n++)
	{
		if(lpObj->Inventory[n].IsItem() == 0 || lpObj->Inventory[n].m_IsValidItem == 0 || lpObj->Inventory[n].IsJewelOfHarmonyItem() == 0)
		{
			continue;
		}

		int ItemOptionType = this->GetJewelOfHarmonyItemOptionType(&lpObj->Inventory[n]);

		if(ItemOptionType == JEWEL_OF_HARMONY_ITEM_OPTION_TYPE_NONE)
		{
			continue;
		}

		int ItemOption = (lpObj->Inventory[n].m_JewelOfHarmonyOption & 0xF0) >> 4;

		if(ItemOption == JEWEL_OF_HARMONY_OPTION_NONE)
		{
			continue;
		}

		JEWEL_OF_HARMONY_OPTION_INFO* lpInfo = this->GetInfo(ItemOptionType,ItemOption);

		if(lpInfo == 0)
		{
			continue;
		}

		int ItemOptionLevel = (lpObj->Inventory[n].m_JewelOfHarmonyOption & 0x0F);

		if(ItemOptionLevel > lpObj->Inventory[n].m_Level)
		{
			continue;
		}

		this->InsertOption(lpObj,ItemOptionType,ItemOption,lpInfo->ValueTable[ItemOptionLevel],flag);
	}
}

void CJewelOfHarmonyOption::InsertOption(LPOBJ lpObj,int type,int index,int value,bool flag) // OK
{
	if(flag == 0)
	{
		if((type == JEWEL_OF_HARMONY_ITEM_OPTION_TYPE_WEAPON && (index == JEWEL_OF_HARMONY_WEAPON_OPTION_SUB_REQUIRE_STRENGTH || index == JEWEL_OF_HARMONY_WEAPON_OPTION_SUB_REQUIRE_DEXTERITY)) || (type == JEWEL_OF_HARMONY_ITEM_OPTION_TYPE_STAFF && (index == JEWEL_OF_HARMONY_STAFF_OPTION_SUB_REQUIRE_STRENGTH || index == JEWEL_OF_HARMONY_STAFF_OPTION_SUB_REQUIRE_DEXTERITY)))
		{
			return;
		}
	}
	else
	{
		if((type != JEWEL_OF_HARMONY_ITEM_OPTION_TYPE_WEAPON || (index != JEWEL_OF_HARMONY_WEAPON_OPTION_SUB_REQUIRE_STRENGTH && index != JEWEL_OF_HARMONY_WEAPON_OPTION_SUB_REQUIRE_DEXTERITY)) && (type != JEWEL_OF_HARMONY_ITEM_OPTION_TYPE_STAFF || (index != JEWEL_OF_HARMONY_STAFF_OPTION_SUB_REQUIRE_STRENGTH && index != JEWEL_OF_HARMONY_STAFF_OPTION_SUB_REQUIRE_DEXTERITY)))
		{
			return;
		}
	}

	switch(type)
	{
		case JEWEL_OF_HARMONY_ITEM_OPTION_TYPE_WEAPON:
			switch(index)
			{
				case JEWEL_OF_HARMONY_WEAPON_OPTION_ADD_MIN_PHYSI_DAMAGE:
					lpObj->PhysiDamageMinLeft += value;
					lpObj->PhysiDamageMinRight += value;
					break;
				case JEWEL_OF_HARMONY_WEAPON_OPTION_ADD_MAX_PHYSI_DAMAGE:
					lpObj->PhysiDamageMaxLeft += value;
					lpObj->PhysiDamageMaxRight += value;
					break;
				case JEWEL_OF_HARMONY_WEAPON_OPTION_SUB_REQUIRE_STRENGTH:
					break;
				case JEWEL_OF_HARMONY_WEAPON_OPTION_SUB_REQUIRE_DEXTERITY:
					break;
				case JEWEL_OF_HARMONY_WEAPON_OPTION_ADD_PHYSI_DAMAGE:
					lpObj->PhysiDamageMinLeft += value;
					lpObj->PhysiDamageMinRight += value;
					lpObj->PhysiDamageMaxLeft += value;
					lpObj->PhysiDamageMaxRight += value;
					break;
				case JEWEL_OF_HARMONY_WEAPON_OPTION_ADD_CRITICAL_DAMAGE:
					lpObj->CriticalDamage += value;
					break;
				case JEWEL_OF_HARMONY_WEAPON_OPTION_ADD_SKILL_DAMAGE:
					lpObj->SkillDamageBonus += value;
					break;
				case JEWEL_OF_HARMONY_WEAPON_OPTION_ADD_ATTACK_SUCCESS_RATE_PVP:
					lpObj->AttackSuccessRatePvP += value;
					break;
				case JEWEL_OF_HARMONY_WEAPON_OPTION_SUB_SD_RATE:
					lpObj->DecreaseShieldGaugeRate += value;
					break;
				case JEWEL_OF_HARMONY_WEAPON_OPTION_ADD_SD_IGNORE_RATE:
					lpObj->IgnoreShieldGaugeRate += value;
					break;
			}
			break;
		case JEWEL_OF_HARMONY_ITEM_OPTION_TYPE_STAFF:
			switch(index)
			{
				case JEWEL_OF_HARMONY_STAFF_OPTION_ADD_MAGIC_DAMAGE:
					lpObj->MagicDamageMin += value;
					lpObj->MagicDamageMax += value;
					break;
				case JEWEL_OF_HARMONY_STAFF_OPTION_SUB_REQUIRE_STRENGTH:
					break;
				case JEWEL_OF_HARMONY_STAFF_OPTION_SUB_REQUIRE_DEXTERITY:
					break;
				case JEWEL_OF_HARMONY_STAFF_OPTION_ADD_SKILL_DAMAGE:
					lpObj->SkillDamageBonus += value;
					break;
				case JEWEL_OF_HARMONY_STAFF_OPTION_ADD_CRITICAL_DAMAGE:
					lpObj->CriticalDamage += value;
					break;
				case JEWEL_OF_HARMONY_STAFF_OPTION_SUB_SD_RATE:
					lpObj->DecreaseShieldGaugeRate += value;
					break;
				case JEWEL_OF_HARMONY_STAFF_OPTION_ADD_ATTACK_SUCCESS_RATE_PVP:
					lpObj->AttackSuccessRatePvP += value;
					break;
				case JEWEL_OF_HARMONY_STAFF_OPTION_ADD_SD_IGNORE_RATE:
					lpObj->IgnoreShieldGaugeRate += value;
					break;
			}
			break;
		case JEWEL_OF_HARMONY_ITEM_OPTION_TYPE_ARMOR:
			switch(index)
			{
				case JEWEL_OF_HARMONY_ARMOR_OPTION_ADD_DEFENSE:
					lpObj->Defense += value;
					break;
				case JEWEL_OF_HARMONY_ARMOR_OPTION_ADD_MAX_BP:
					lpObj->AddBP += value;
					break;
				case JEWEL_OF_HARMONY_ARMOR_OPTION_ADD_MAX_HP:
					lpObj->AddLife += value;
					break;
				case JEWEL_OF_HARMONY_ARMOR_OPTION_ADD_HP_RECOVERY:
					lpObj->HPRecovery += value;
					break;
				case JEWEL_OF_HARMONY_ARMOR_OPTION_ADD_MP_RECOVERY:
					lpObj->MPRecovery += value;
					break;
				case JEWEL_OF_HARMONY_ARMOR_OPTION_ADD_DEFENSE_SUCCESS_RATE_PVP:
					lpObj->DefenseSuccessRatePvP += value;
					break;
				case JEWEL_OF_HARMONY_ARMOR_OPTION_ADD_DAMAGE_REDUCTION:
					lpObj->DamageReduction[DAMAGE_REDUCTION_JOH_ITEM] += value;
					break;
				case JEWEL_OF_HARMONY_ARMOR_OPTION_ADD_SD_RATE:
					lpObj->ShieldGaugeRate += value;
					break;
			}
			break;
	}
}
