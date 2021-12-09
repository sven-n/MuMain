// CustomJewel.cpp: implementation of the CCustomJewel class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CustomJewel.h"
#include "CustomWing.h"
#include "ItemManager.h"
#include "ItemExcellentOptionRate.h"
#include "MemScript.h"
#include "ObjectManager.h"
#include "ServerInfo.h"
#include "SetItemType.h"
#include "SocketItemType.h"
#include "Util.h"

CCustomJewel gCustomJewel;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCustomJewel::CCustomJewel() // OK
{
	this->Init();
}

CCustomJewel::~CCustomJewel() // OK
{

}

void CCustomJewel::Init() // OK
{
	for(int n=0;n < MAX_CUSTOM_JEWEL;n++)
	{
		this->m_CustomJewelInfo[n].Index = -1;
	}
}

void CCustomJewel::Load(char* path) // OK
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
				if(section == 0)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					CUSTOM_JEWEL_INFO info;

					memset(&info,0,sizeof(info));

					info.Index = lpMemScript->GetNumber();

					info.ItemIndex = lpMemScript->GetAsNumber();

					info.MinItemLevel = lpMemScript->GetAsNumber();

					info.MaxItemLevel = lpMemScript->GetAsNumber();

					info.MaxItemOption1 = lpMemScript->GetAsNumber();

					info.MaxItemOption2 = lpMemScript->GetAsNumber();

					info.MinItemOption3 = lpMemScript->GetAsNumber();

					info.MaxItemOption3 = lpMemScript->GetAsNumber();

					info.MinItemNewOption = lpMemScript->GetAsNumber();

					info.MaxItemNewOption = lpMemScript->GetAsNumber();

					info.MaxItemSetOption = lpMemScript->GetAsNumber();

					info.MinItemSocketOption = lpMemScript->GetAsNumber();

					info.MaxItemSocketOption = lpMemScript->GetAsNumber();

					info.EnableSlotWeapon = lpMemScript->GetAsNumber();

					info.EnableSlotArmor = lpMemScript->GetAsNumber();

					info.EnableSlotWing = lpMemScript->GetAsNumber();

					info.SuccessRate[0] = lpMemScript->GetAsNumber();

					info.SuccessRate[1] = lpMemScript->GetAsNumber();

					info.SuccessRate[2] = lpMemScript->GetAsNumber();

					info.SuccessRate[3] = lpMemScript->GetAsNumber();

					info.SalePrice = lpMemScript->GetAsNumber();

					strcpy_s(info.ModelName,lpMemScript->GetAsString());

					this->SetInfo(info);
				}
				else if(section == 1)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					CUSTOM_JEWEL_SUCCESS_INFO info;

					memset(&info,0,sizeof(info));

					info.Index = lpMemScript->GetNumber();

					info.Level = lpMemScript->GetAsNumber();

					info.Option1 = lpMemScript->GetAsNumber();

					info.Option2 = lpMemScript->GetAsNumber();

					info.Option3 = lpMemScript->GetAsNumber();

					info.NewOption = lpMemScript->GetAsNumber();

					info.SetOption = lpMemScript->GetAsNumber();

					info.SocketOption = lpMemScript->GetAsNumber();

					this->SetSuccessInfo(info);
				}
				else if(section == 2)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					CUSTOM_JEWEL_FAILURE_INFO info;

					memset(&info,0,sizeof(info));

					info.Index = lpMemScript->GetNumber();

					info.Level = lpMemScript->GetAsNumber();

					info.Option1 = lpMemScript->GetAsNumber();

					info.Option2 = lpMemScript->GetAsNumber();

					info.Option3 = lpMemScript->GetAsNumber();

					info.NewOption = lpMemScript->GetAsNumber();

					info.SetOption = lpMemScript->GetAsNumber();

					info.SocketOption = lpMemScript->GetAsNumber();

					this->SetFailureInfo(info);
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

void CCustomJewel::SetInfo(CUSTOM_JEWEL_INFO info) // OK
{
	if(info.Index < 0 || info.Index >= MAX_CUSTOM_JEWEL)
	{
		return;
	}

	this->m_CustomJewelInfo[info.Index] = info;
}

void CCustomJewel::SetSuccessInfo(CUSTOM_JEWEL_SUCCESS_INFO info) // OK
{
	if(info.Index < 0 || info.Index >= MAX_CUSTOM_JEWEL)
	{
		return;
	}

	this->m_CustomJewelInfo[info.Index].SuccessInfo = info;
}

void CCustomJewel::SetFailureInfo(CUSTOM_JEWEL_FAILURE_INFO info) // OK
{
	if(info.Index < 0 || info.Index >= MAX_CUSTOM_JEWEL)
	{
		return;
	}

	this->m_CustomJewelInfo[info.Index].FailureInfo = info;
}

CUSTOM_JEWEL_INFO* CCustomJewel::GetInfo(int index) // OK
{
	if(index < 0 || index >= MAX_CUSTOM_JEWEL)
	{
		return 0;
	}

	if(this->m_CustomJewelInfo[index].Index != index)
	{
		return 0;
	}

	return &this->m_CustomJewelInfo[index];
}

CUSTOM_JEWEL_INFO* CCustomJewel::GetInfoByItem(int ItemIndex) // OK
{
	for(int n=0;n < MAX_CUSTOM_JEWEL;n++)
	{
		CUSTOM_JEWEL_INFO* lpInfo = this->GetInfo(n);

		if(lpInfo == 0)
		{
			continue;
		}

		if(lpInfo->ItemIndex == ItemIndex)
		{
			return lpInfo;
		}
	}

	return 0;
}

CUSTOM_JEWEL_SUCCESS_INFO* CCustomJewel::GetSuccessInfo(int ItemIndex) // OK
{
	CUSTOM_JEWEL_INFO* lpInfo = this->GetInfoByItem(ItemIndex);

	if(lpInfo == 0)
	{
		return 0;
	}

	return &lpInfo->SuccessInfo;
}

CUSTOM_JEWEL_FAILURE_INFO* CCustomJewel::GetFailureInfo(int ItemIndex) // OK
{
	CUSTOM_JEWEL_INFO* lpInfo = this->GetInfoByItem(ItemIndex);

	if(lpInfo == 0)
	{
		return 0;
	}

	return &lpInfo->FailureInfo;
}

bool CCustomJewel::CheckCustomJewel(int index) // OK
{
	if(this->GetInfo(index) != 0)
	{
		return 1;
	}

	return 0;
}

bool CCustomJewel::CheckCustomJewelByItem(int ItemIndex) // OK
{
	if(this->GetInfoByItem(ItemIndex) != 0)
	{
		return 1;
	}

	return 0;
}

bool CCustomJewel::CheckCustomJewelApplyItem(int ItemIndex,CItem* lpItem) // OK
{
	CUSTOM_JEWEL_INFO* lpInfo = this->GetInfoByItem(ItemIndex);

	if(lpInfo == 0)
	{
		return 0;
	}

	if(lpItem->IsLuckyItem() != 0)
	{
		return 0;
	}

	//if(lpItem->IsJewelOfHarmonyItem() == 0 && (lpInfo->MinItemSocketOption > 0 || lpInfo->MaxItemSocketOption > 0))
	//{
	//	return 0;
	//}

	if(lpItem->m_Index > GET_ITEM(12,6) && (lpItem->m_Index < GET_ITEM(12,36) || lpItem->m_Index > GET_ITEM(12,43)) && lpItem->m_Index != GET_ITEM(12,49) && lpItem->m_Index != GET_ITEM(12,50) && (lpItem->m_Index < GET_ITEM(12,200) || lpItem->m_Index > GET_ITEM(12,265)) && lpItem->m_Index != GET_ITEM(13,30) && gCustomWing.CheckCustomWingByItem(lpItem->m_Index) == 0)
	{
		return 0;
	}

	if(lpInfo->EnableSlotWeapon == 0 && (lpItem->m_Index >= GET_ITEM(0,0) && lpItem->m_Index < GET_ITEM(6,0)))
	{
		return 0;
	}

	if(lpInfo->EnableSlotArmor == 0 && (lpItem->m_Index >= GET_ITEM(6,0) && lpItem->m_Index < GET_ITEM(12,0)))
	{
		return 0;
	}

	if(lpInfo->EnableSlotWing == 0 && ((lpItem->m_Index >= GET_ITEM(12,0) && lpItem->m_Index <= GET_ITEM(12,6)) || (lpItem->m_Index >= GET_ITEM(12,36) && lpItem->m_Index <= GET_ITEM(12,43)) || lpItem->m_Index == GET_ITEM(12,49) || lpItem->m_Index == GET_ITEM(12,50) || (lpItem->m_Index >= GET_ITEM(12,262) && lpItem->m_Index <= GET_ITEM(12,265)) || lpItem->m_Index == GET_ITEM(13,30) || gCustomWing.CheckCustomWingByItem(lpItem->m_Index) != 0))
	{
		return 0;
	}

	if(lpInfo->MinItemLevel != -1 && lpItem->m_Level < lpInfo->MinItemLevel)
	{
		return 0;
	}

	if(lpInfo->MaxItemLevel != -1 && lpItem->m_Level > lpInfo->MaxItemLevel)
	{
		return 0;
	}

	ITEM_INFO ItemInfo;

	if(gItemManager.GetInfo(lpItem->m_Index,&ItemInfo) == 0)
	{
		return 0;
	}

	if(lpInfo->MaxItemOption1 != -1 && (lpItem->m_Option1 > lpInfo->MaxItemOption1 || ItemInfo.Skill == 0))
	{
		return 0;
	}

	if(lpInfo->MaxItemOption2 != -1 && lpItem->m_Option2 > lpInfo->MaxItemOption2)
	{
		return 0;
	}

	if(lpInfo->MinItemOption3 != -1 && lpItem->m_Option3 < lpInfo->MinItemOption3)
	{
		return 0;
	}

	if(lpInfo->MaxItemOption3 != -1 && lpItem->m_Option3 > lpInfo->MaxItemOption3)
	{
		return 0;
	}

	if(this->CheckCustomJewelApplyItemNewOption(lpInfo,lpItem) == 0)
	{
		return 0;
	}

	if(this->CheckCustomJewelApplyItemSetOption(lpInfo,lpItem) == 0)
	{
		return 0;
	}

	if(this->CheckCustomJewelApplyItemSocketOption(lpInfo,lpItem) == 0)
	{
		return 0;
	}

	return 1;
}

bool CCustomJewel::CheckCustomJewelApplyItemNewOption(CUSTOM_JEWEL_INFO* lpInfo,CItem* lpItem) // OK
{
	if(lpInfo->MinItemNewOption != -1 && GetNewOptionCount(lpItem->m_NewOption) < lpInfo->MinItemNewOption)
	{
		return 0;
	}

	if(lpInfo->MaxItemNewOption != -1 && lpInfo->MaxItemNewOption > 10 && (lpItem->m_NewOption & (lpInfo->MaxItemNewOption-10)) != 0)
	{
		return 0;
	}

	if(lpInfo->MaxItemNewOption != -1 && lpInfo->MaxItemNewOption < 10 && GetNewOptionCount(lpItem->m_NewOption) > lpInfo->MaxItemNewOption)
	{
		return 0;
	}

	return 1;
}

bool CCustomJewel::CheckCustomJewelApplyItemSetOption(CUSTOM_JEWEL_INFO* lpInfo,CItem* lpItem) // OK
{
	if(lpInfo->MaxItemSetOption != -1 && (lpItem->IsSetItem() != 0 || ((lpInfo->MaxItemSetOption & 1) != 0 && gSetItemType.GetSetItemOptionIndex(lpItem->m_Index,0) <= 0)))
	{
		return 0;
	}

	if(lpInfo->MaxItemSetOption != -1 && (lpItem->IsSetItem() != 0 || ((lpInfo->MaxItemSetOption & 2) != 0 && gSetItemType.GetSetItemOptionIndex(lpItem->m_Index,1) <= 0)))
	{
		return 0;
	}

	return 1;
}

bool CCustomJewel::CheckCustomJewelApplyItemSocketOption(CUSTOM_JEWEL_INFO* lpInfo,CItem* lpItem) // OK
{
	if(gServerInfo.m_JewelSocketPentagram == 1 && (lpItem->m_Index >= GET_ITEM(12,200) && lpItem->m_Index <= GET_ITEM(12,214)) && GetSocketOptionCount(lpItem->m_SocketOption) < 5)
	{
		return 1;
	}
	if(lpInfo->MinItemSocketOption != -1 && ((lpInfo->MinItemSocketOption == 0 && gSocketItemType.CheckSocketItemType(lpItem->m_Index) == 0) || (lpInfo->MinItemSocketOption != 0 && (gSocketItemType.CheckSocketItemType(lpItem->m_Index) == 0 || GetSocketOptionCount(lpItem->m_SocketOption) < lpInfo->MinItemSocketOption))))
	{
		return 0;
	}

	if(lpInfo->MaxItemSocketOption != -1 && ((lpInfo->MaxItemSocketOption == 0 && gSocketItemType.CheckSocketItemType(lpItem->m_Index) == 0) || (lpInfo->MaxItemSocketOption != 0 && (gSocketItemType.CheckSocketItemType(lpItem->m_Index) == 0 || GetSocketOptionCount(lpItem->m_SocketOption) > lpInfo->MaxItemSocketOption || GetSocketOptionCount(lpItem->m_SocketOption) >= gSocketItemType.GetSocketItemMaxSocket(lpItem->m_Index)))))
	{
		return 0;
	}

	return 1;
}

int CCustomJewel::GetCustomJewelSuccessRate(int ItemIndex,int AccountLevel) // OK
{
	CUSTOM_JEWEL_INFO* lpInfo = this->GetInfoByItem(ItemIndex);

	if(lpInfo == 0)
	{
		return 0;
	}

	return lpInfo->SuccessRate[AccountLevel];
}

int CCustomJewel::GetCustomJewelSalePrice(int ItemIndex) // OK
{
	CUSTOM_JEWEL_INFO* lpInfo = this->GetInfoByItem(ItemIndex);

	if(lpInfo == 0)
	{
		return 0;
	}

	return lpInfo->SalePrice;
}

int CCustomJewel::GetCustomJewelNewOption(CItem* lpItem,int value) // OK
{
	if(value >= 10)
	{
		return (lpItem->m_NewOption=(lpItem->m_NewOption|(value-10)));
	}

	int count = 0;

	int MaxNewOption = MAX_EXC_OPTION;

	if((lpItem->m_Index >= GET_ITEM(12,3) && lpItem->m_Index <= GET_ITEM(12,6)) || lpItem->m_Index == GET_ITEM(12,42) || lpItem->m_Index == GET_ITEM(12,49) || lpItem->m_Index == GET_ITEM(13,30)) // 2sd Wings
	{
		MaxNewOption = ((lpItem->m_Index==GET_ITEM(13,30))?4:3);
	}

	if((lpItem->m_Index >= GET_ITEM(12,36) && lpItem->m_Index <= GET_ITEM(12,40)) || lpItem->m_Index == GET_ITEM(12,43) || lpItem->m_Index == GET_ITEM(12,50)) // 3rd Wings
	{
		MaxNewOption = 4;
	}

	if(lpItem->m_Index >= GET_ITEM(12,262) && lpItem->m_Index <= GET_ITEM(12,265)) // Monster Wings
	{
		MaxNewOption = 2;
	}

	if(lpItem->m_Index == GET_ITEM(12,266)) // Wings of Conqueror
	{
		MaxNewOption = 3;
	}

	if(lpItem->m_Index == GET_ITEM(12,267)) // Wings of Angel and Devil
	{
		MaxNewOption = 3;
	}

	if(gCustomWing.CheckCustomWingByItem(lpItem->m_Index) != 0)
	{
		MaxNewOption = 4;
	}

	if(lpItem->m_Index == GET_ITEM(13,3)) // Dinorant
	{
		MaxNewOption = 3;
	}

	if(lpItem->m_Index == GET_ITEM(13,37)) // Fenrir
	{
		MaxNewOption = 3;
	}

	while(true)
	{
		if(count >= value || GetNewOptionCount(lpItem->m_NewOption) >= MaxNewOption)
		{
			break;
		}

		//int option = 1 << (GetLargeRand()%MaxNewOption);

		BYTE ExcOption;

		if(gItemExOptionRate.GetExcellentOptionByRate(lpItem->m_Index,lpItem->m_NewOption,&ExcOption) == 0)
		{
			break;
		}

		if((lpItem->m_NewOption & ExcOption) == 0)
		{
			lpItem->m_NewOption |= ExcOption;
			count++;
		}
	}

	return count;
}

int CCustomJewel::GetCustomJewelSetOption(CItem* lpItem,int value) // OK
{
	BYTE SetIndex = lpItem->m_SetOption & 3;
	BYTE SetValue = lpItem->m_SetOption & 12;

	if(SetIndex == 0)
	{
		for(int n=0;n < MAX_SET_ITEM_OPTION_INDEX;n++)
		{
			if((value & (1 << n)) != 0 && gSetItemType.GetSetItemOptionIndex(lpItem->m_Index,n) > 0)
			{
				SetIndex = ((SetIndex==0)?(n+1):(((GetLargeRand()%2)==0)?(n+1):SetIndex));
			}
		}
	}

	if(SetValue == 0 && SetIndex > 0 && (value & 4) != 0)
	{
		SetValue = 4;
	}

	if(SetValue == 4 && SetIndex > 0 && (value & 8) != 0)
	{
		SetValue = 8;
	}

	return ((lpItem->m_SetOption)=(SetIndex+SetValue));
}

int CCustomJewel::GetCustomJewelSocketOption(CItem* lpItem,int value) // OK
{
	int count = 0;

	for(int n=0;n < MAX_SOCKET_OPTION;n++)
	{
		if(count < value && lpItem->m_SocketOption[n] == 0xFF)
		{
			lpItem->m_SocketOption[n] = 0xFE;
			count++;
		}
	}

	return count;
}
