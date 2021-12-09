// 380ItemOption.cpp: implementation of the C380ItemOption class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "380ItemOption.h"
#include "380ItemType.h"
#include "ItemManager.h"
#include "MemScript.h"
#include "Util.h"

C380ItemOption g380ItemOption;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

C380ItemOption::C380ItemOption() // OK
{
	this->Init();
}

C380ItemOption::~C380ItemOption() // OK
{

}

void C380ItemOption::Init() // OK
{
	for(int n=0;n < MAX_380_ITEM_OPTION;n++)
	{
		this->m_380ItemOptionInfo[n].Index = -1;
	}
}

void C380ItemOption::Load(char* path) // OK
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

			if(strcmp("end",lpMemScript->GetString()) == 0)
			{
				break;
			}
			
			ITEM_380_OPTION_INFO info;

			memset(&info,0,sizeof(info));

			info.Index = lpMemScript->GetNumber();

			strcpy_s(info.Name,lpMemScript->GetAsString());

			info.Value = lpMemScript->GetAsNumber();

			this->SetInfo(info);
		}
	}
	catch(...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;
}

void C380ItemOption::SetInfo(ITEM_380_OPTION_INFO info) // OK
{
	if(info.Index < 0 || info.Index >= MAX_380_ITEM_OPTION)
	{
		return;
	}

	this->m_380ItemOptionInfo[info.Index] = info;
}

ITEM_380_OPTION_INFO* C380ItemOption::GetInfo(int index) // OK
{
	if(index < 0 || index >= MAX_380_ITEM_OPTION)
	{
		return 0;
	}

	if(this->m_380ItemOptionInfo[index].Index != index)
	{
		return 0;
	}

	return &this->m_380ItemOptionInfo[index];
}

bool C380ItemOption::Is380Item(CItem* lpItem) // OK
{
	if((lpItem->m_ItemOptionEx & 0x80) == 0)
	{
		return 0;
	}

	return 1;
}

void C380ItemOption::Calc380ItemOption(LPOBJ lpObj,bool flag) // OK
{
	for(int n=0;n < INVENTORY_WEAR_SIZE;n++)
	{
		if(lpObj->Inventory[n].IsItem() == 0 || lpObj->Inventory[n].m_IsValidItem == 0 || lpObj->Inventory[n].Is380Item() == 0)
		{
			continue;
		}

		for(int i=0;i < MAX_380_ITEM_OPTION_INDEX;i++)
		{
			int ItemOption = g380ItemType.Get380ItemOptionIndex(lpObj->Inventory[n].m_Index,i);

			if(ItemOption == ITEM_380_OPTION_NONE)
			{
				continue;
			}

			ITEM_380_OPTION_INFO* lpInfo = this->GetInfo(ItemOption);

			if(lpInfo == 0)
			{
				continue;
			}

			int OptionValue = g380ItemType.Get380ItemOptionValue(lpObj->Inventory[n].m_Index,i);

			OptionValue = ((OptionValue==-1)?lpInfo->Value:OptionValue);

			this->InsertOption(lpObj,lpInfo->Index,OptionValue,flag);
		}
	}
}

void C380ItemOption::InsertOption(LPOBJ lpObj,int index,int value,bool flag) // OK
{
	if(flag != 0)
	{
		return;
	}

	switch(index)
	{
		case ITEM_380_OPTION_ADD_ATTACK_SUCCESS_RATE_PVP:
			lpObj->AttackSuccessRatePvP += value;
			break;
		case ITEM_380_OPTION_ADD_DAMAGE_PVP:
			lpObj->DamagePvP += value;
			break;
		case ITEM_380_OPTION_ADD_DEFENSE_SUCCESS_RATE_PVP:
			lpObj->DefenseSuccessRatePvP += value;
			break;
		case ITEM_380_OPTION_ADD_DEFENSE_PVP:
			lpObj->DefensePvP += value;
			break;
		case ITEM_380_OPTION_ADD_MAX_HP:
			lpObj->AddLife += value;
			break;
		case ITEM_380_OPTION_ADD_MAX_SD:
			lpObj->AddShield += value;
			break;
		case ITEM_380_OPTION_SET_SD_RECOVERY_TYPE:
			lpObj->SDRecoveryType = value;
			break;
		case ITEM_380_OPTION_ADD_SD_RECOVERY_RATE:
			lpObj->SDRecoveryRate += value;
			break;
	}
}
