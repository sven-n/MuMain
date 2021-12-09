// ItemOption.cpp: implementation of the CItemOption class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ItemOption.h"
#include "ItemManager.h"
#include "MemScript.h"
#include "Util.h"

CItemOption gItemOption;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CItemOption::CItemOption() // OK
{
	this->m_ItemOptionInfo.clear();
}

CItemOption::~CItemOption() // OK
{

}

void CItemOption::Load(char* path) // OK
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

	this->m_ItemOptionInfo.clear();

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

			ITEM_OPTION_INFO info;

			info.Index = lpMemScript->GetNumber();

			info.OptionIndex = lpMemScript->GetAsNumber();

			info.OptionValue = lpMemScript->GetAsNumber();

			info.ItemMinIndex = lpMemScript->GetAsNumber();

			info.ItemMaxIndex = lpMemScript->GetAsNumber();

			info.ItemOption1 = lpMemScript->GetAsNumber();

			info.ItemOption2 = lpMemScript->GetAsNumber();

			info.ItemOption3 = lpMemScript->GetAsNumber();

			info.ItemNewOption = lpMemScript->GetAsNumber();

			std::map<int,std::vector<ITEM_OPTION_INFO>>::iterator it = this->m_ItemOptionInfo.find(info.Index);

			if(it == this->m_ItemOptionInfo.end())
			{
				this->m_ItemOptionInfo.insert(std::pair<int,std::vector<ITEM_OPTION_INFO>>(info.Index,std::vector<ITEM_OPTION_INFO>(1,info)));
			}
			else
			{
				it->second.push_back(info);
			}
		}
	}
	catch(...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;
}

bool CItemOption::GetItemOption(int index,CItem* lpItem) // OK
{
	bool result = 0;

	std::map<int,std::vector<ITEM_OPTION_INFO>>::iterator ItemOptionInfo = this->m_ItemOptionInfo.find(index);

	if(ItemOptionInfo != this->m_ItemOptionInfo.end())
	{
		for(std::vector<ITEM_OPTION_INFO>::iterator it=ItemOptionInfo->second.begin();it != ItemOptionInfo->second.end();it++)
		{
			if(it->Index != index)
			{
				continue;
			}

			if(it->ItemMinIndex != -1 && it->ItemMinIndex > lpItem->m_Index)
			{
				continue;
			}

			if(it->ItemMaxIndex != -1 && it->ItemMaxIndex < lpItem->m_Index)
			{
				continue;
			}

			if(it->ItemOption1 != -1 && it->ItemOption1 > lpItem->m_Option1)
			{
				continue;
			}

			if(it->ItemOption2 != -1 && it->ItemOption2 > lpItem->m_Option2)
			{
				continue;
			}

			if(it->ItemOption3 != -1 && it->ItemOption3 > lpItem->m_Option3)
			{
				continue;
			}

			if(it->ItemNewOption != -1 && (lpItem->m_NewOption & it->ItemNewOption) == 0)
			{
				continue;
			}

			result = 1;

			lpItem->m_SpecialIndex[index] = it->OptionIndex;

			lpItem->m_SpecialValue[index] = it->OptionValue;
		}
	}

	return result;
}

void CItemOption::CalcItemCommonOption(LPOBJ lpObj,bool flag) // OK
{
	for(int n=0;n < INVENTORY_WEAR_SIZE;n++)
	{
		if(lpObj->Inventory[n].IsItem() != 0 && lpObj->Inventory[n].m_IsValidItem != 0 && lpObj->Inventory[n].m_Durability != 0)
		{
			this->InsertOption(lpObj,&lpObj->Inventory[n],flag);
		}
	}
}

void CItemOption::InsertOption(LPOBJ lpObj,CItem* lpItem,bool flag) // OK
{
	for(int n=0;n < MAX_SPECIAL;n++)
	{
		int index = lpItem->m_SpecialIndex[n];

		int value = lpItem->m_SpecialValue[n];

		if(flag == 0)
		{
			if(index == ITEM_OPTION_ADD_WING_LEADERSHIP)
			{
				continue;
			}
		}
		else
		{
			if(index != ITEM_OPTION_ADD_WING_LEADERSHIP)
			{
				continue;
			}
		}

		switch(index)
		{
			case ITEM_OPTION_ADD_PHYSI_DAMAGE:
				lpObj->PhysiDamageMinRight += value;
				lpObj->PhysiDamageMaxRight += value;
				lpObj->PhysiDamageMinLeft += value;
				lpObj->PhysiDamageMaxLeft += value;
				break;
			case ITEM_OPTION_ADD_MAGIC_DAMAGE:
				lpObj->MagicDamageMin += value;
				lpObj->MagicDamageMax += value;
				break;
			case ITEM_OPTION_ADD_DEFENSE_SUCCESS_RATE:
				lpObj->DefenseSuccessRate += value;
				break;
			case ITEM_OPTION_ADD_DEFENSE:
				lpObj->Defense += value;
				break;
			case ITEM_OPTION_ADD_CRITICAL_DAMAGE_RATE:
				lpObj->CriticalDamageRate += value;
				break;
			case ITEM_OPTION_ADD_HP_RECOVERY_RATE:
				lpObj->HPRecoveryRate += value;
				break;
			case ITEM_OPTION_ADD_MONEY_AMOUNT_DROP_RATE:
				lpObj->MoneyAmountDropRate += value;
				break;
			case ITEM_OPTION_MUL_DEFENSE_SUCCESS_RATE:
				lpObj->DefenseSuccessRate += (lpObj->DefenseSuccessRate*value)/100;
				break;
			case ITEM_OPTION_ADD_DAMAGE_REFLECT:
				lpObj->DamageReflect += value;
				break;
			case ITEM_OPTION_ADD_DAMAGE_REDUCTION:
				lpObj->DamageReduction[DAMAGE_REDUCTION_EXCELLENT_ITEM] += value;
				break;
			case ITEM_OPTION_MUL_MP:
				lpObj->AddMana += (int)((lpObj->MaxMana*value)/100);
				break;
			case ITEM_OPTION_MUL_HP:
				lpObj->AddLife += (int)((lpObj->MaxLife*value)/100);
				break;
			case ITEM_OPTION_ADD_EXCELLENT_DAMAGE_RATE:
				lpObj->ExcellentDamageRate += value;
				break;
			case ITEM_OPTION_ADD_PHYSI_DAMAGE_BY_LEVEL:
				lpObj->PhysiDamageMinRight += ((lpObj->Level+lpObj->MasterLevel)/value);
				lpObj->PhysiDamageMaxRight += ((lpObj->Level+lpObj->MasterLevel)/value);
				lpObj->PhysiDamageMinLeft += ((lpObj->Level+lpObj->MasterLevel)/value);
				lpObj->PhysiDamageMaxLeft += ((lpObj->Level+lpObj->MasterLevel)/value);
				break;
			case ITEM_OPTION_MUL_PHYSI_DAMAGE:
				lpObj->PhysiDamageMinRight += (lpObj->PhysiDamageMinRight*value)/100;
				lpObj->PhysiDamageMaxRight += (lpObj->PhysiDamageMaxRight*value)/100;
				lpObj->PhysiDamageMinLeft += (lpObj->PhysiDamageMinLeft*value)/100;
				lpObj->PhysiDamageMaxLeft += (lpObj->PhysiDamageMaxLeft*value)/100;
				break;
			case ITEM_OPTION_ADD_MAGIC_DAMAGE_BY_LEVEL:
				lpObj->MagicDamageMin += ((lpObj->Level+lpObj->MasterLevel)/value);
				lpObj->MagicDamageMax += ((lpObj->Level+lpObj->MasterLevel)/value);
				break;
			case ITEM_OPTION_MUL_MAGIC_DAMAGE:
				lpObj->MagicDamageMin += (lpObj->MagicDamageMin*value)/100;
				lpObj->MagicDamageMax += (lpObj->MagicDamageMax*value)/100;
				break;
			case ITEM_OPTION_ADD_SPEED:
				lpObj->PhysiSpeed += value;
				lpObj->MagicSpeed += value;
				break;
			case ITEM_OPTION_ADD_HUNT_HP:
				lpObj->HuntHP += value;
				break;
			case ITEM_OPTION_ADD_HUNT_MP:
				lpObj->HuntMP += value;
				break;
			case ITEM_OPTION_ADD_WING_HP:
				lpObj->AddLife += 50+(value*lpItem->m_Level);
				break;
			case ITEM_OPTION_ADD_WING_MP:
				lpObj->AddMana += 50+(value*lpItem->m_Level);
				break;
			case ITEM_OPTION_ADD_IGNORE_DEFENSE_RATE:
				lpObj->IgnoreDefenseRate += value;
				break;
			case ITEM_OPTION_ADD_BP:
				lpObj->AddBP += value;
				break;
			case ITEM_OPTION_MUL_BP:
				lpObj->AddBP += (int)((lpObj->MaxBP*value)/100);
				break;
			case ITEM_OPTION_ADD_WING_LEADERSHIP:
				lpObj->AddLeadership += 10+(value*lpItem->m_Level);
				break;
			case ITEM_OPTION_ADD_FENRIR1:
				break;
			case ITEM_OPTION_ADD_FENRIR2:
				break;
			case ITEM_OPTION_ADD_FENRIR3:
				break;
			case ITEM_OPTION_ADD_FULL_DAMAGE_REFLECT_RATE:
				lpObj->FullDamageReflectRate += value;
				break;
			case ITEM_OPTION_ADD_DEFENSIVE_FULL_HP_RESTORE_RATE:
				lpObj->DefensiveFullHPRestoreRate += value;
				break;
			case ITEM_OPTION_ADD_DEFENSIVE_FULL_MP_RESTORE_RATE:
				lpObj->DefensiveFullMPRestoreRate += value;
				break;
			case ITEM_OPTION_ADD_DINORANT1:
				break;
			case ITEM_OPTION_ADD_CURSE_DAMAGE:
				lpObj->CurseDamageMin += value;
				lpObj->CurseDamageMax += value;
				break;
			case ITEM_OPTION_ADD_CURSE_DAMAGE_BY_LEVEL:
				lpObj->CurseDamageMin += ((lpObj->Level+lpObj->MasterLevel)/value);
				lpObj->CurseDamageMax += ((lpObj->Level+lpObj->MasterLevel)/value);
				break;
			case ITEM_OPTION_MUL_CURSE_DAMAGE:
				lpObj->CurseDamageMin += (lpObj->CurseDamageMin*value)/100;
				lpObj->CurseDamageMax += (lpObj->CurseDamageMax*value)/100;
				break;
			case ITEM_OPTION_ADD_DOUBLE_DAMAGE_RATE:
				lpObj->DoubleDamageRate += value;
				break;
			case ITEM_OPTION_ADD_EXPERIENCE_RATE:
				lpObj->ExperienceRate += value;
				lpObj->MasterExperienceRate += value;
				break;
			case ITEM_OPTION_ADD_COMBO_EXPERIENCE_RATE:
				if(lpObj->Inventory[8].IsItem() != 0 && (lpObj->Inventory[8].m_Index == GET_ITEM(13,80) || lpObj->Inventory[8].m_Index == GET_ITEM(13,123)))
				{
					lpObj->ExperienceRate += value;
					lpObj->MasterExperienceRate += value;
				}
				break;
			case ITEM_OPTION_ADD_HP:
				lpObj->AddLife += value;
				break;
			case ITEM_OPTION_ADD_MP:
				lpObj->AddMana += value;
				break;
			case ITEM_OPTION_ADD_HP_BY_LEVEL:
				lpObj->AddLife += ((lpObj->Level+lpObj->MasterLevel)/value);
				break;
			case ITEM_OPTION_ADD_MP_BY_LEVEL:
				lpObj->AddMana += ((lpObj->Level+lpObj->MasterLevel)/value);
				break;
			case ITEM_OPTION_ADD_DAMAGE:
				lpObj->PhysiDamageMinRight += value;
				lpObj->PhysiDamageMaxRight += value;
				lpObj->PhysiDamageMinLeft += value;
				lpObj->PhysiDamageMaxLeft += value;
				lpObj->MagicDamageMin += value;
				lpObj->MagicDamageMax += value;
				lpObj->CurseDamageMin += value;
				lpObj->CurseDamageMax += value;
				break;
			case ITEM_OPTION_ADD_DAMAGE_BY_LEVEL:
				lpObj->PhysiDamageMinRight += ((lpObj->Level+lpObj->MasterLevel)/value);
				lpObj->PhysiDamageMaxRight += ((lpObj->Level+lpObj->MasterLevel)/value);
				lpObj->PhysiDamageMinLeft += ((lpObj->Level+lpObj->MasterLevel)/value);
				lpObj->PhysiDamageMaxLeft += ((lpObj->Level+lpObj->MasterLevel)/value);
				lpObj->MagicDamageMin += ((lpObj->Level+lpObj->MasterLevel)/value);
				lpObj->MagicDamageMax += ((lpObj->Level+lpObj->MasterLevel)/value);
				lpObj->CurseDamageMin += ((lpObj->Level+lpObj->MasterLevel)/value);
				lpObj->CurseDamageMax += ((lpObj->Level+lpObj->MasterLevel)/value);
				break;
			case ITEM_OPTION_MUL_DAMAGE:
				lpObj->PhysiDamageMinRight += (lpObj->PhysiDamageMinRight*value)/100;
				lpObj->PhysiDamageMaxRight += (lpObj->PhysiDamageMaxRight*value)/100;
				lpObj->PhysiDamageMinLeft += (lpObj->PhysiDamageMinLeft*value)/100;
				lpObj->PhysiDamageMaxLeft += (lpObj->PhysiDamageMaxLeft*value)/100;
				lpObj->MagicDamageMin += (lpObj->MagicDamageMin*value)/100;
				lpObj->MagicDamageMax += (lpObj->MagicDamageMax*value)/100;
				lpObj->CurseDamageMin += (lpObj->CurseDamageMin*value)/100;
				lpObj->CurseDamageMax += (lpObj->CurseDamageMax*value)/100;
				break;
			case ITEM_OPTION_ADD_DEFENSE_BY_LEVEL:
				lpObj->Defense += ((lpObj->Level+lpObj->MasterLevel)/value);
				break;
			case ITEM_OPTION_MUL_DEFENSE:
				lpObj->Defense += (lpObj->Defense*value)/100;
				break;
		}
	}
}
