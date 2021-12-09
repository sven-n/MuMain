// SetItemType.cpp: implementation of the CSetItemType class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SetItemType.h"
#include "DSProtocol.h"
#include "ItemManager.h"
#include "ItemOptionRate.h"
#include "MemScript.h"
#include "ServerInfo.h"
#include "Util.h"

CSetItemType gSetItemType;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSetItemType::CSetItemType() // OK
{
	this->Init();
}

CSetItemType::~CSetItemType() // OK
{

}

void CSetItemType::Init() // OK
{
	for(int n=0;n < MAX_SET_ITEM_TYPE;n++)
	{
		this->m_SetItemTypeInfo[n].Index = -1;
	}

	this->m_count = 0;
}

void CSetItemType::Load(char* path) // OK
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

			SET_ITEM_TYPE_INFO info;

			memset(&info,0,sizeof(info));

			info.Index = SafeGetItem(GET_ITEM(lpMemScript->GetNumber(),lpMemScript->GetAsNumber()));

			info.StatType = lpMemScript->GetAsNumber();

			for(int n=0;n < MAX_SET_ITEM_OPTION_INDEX;n++)
			{
				info.OptionIndex[n] = lpMemScript->GetAsNumber();
			}

			this->SetInfo(info);
		}
	}
	catch(...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;
}

void CSetItemType::SetInfo(SET_ITEM_TYPE_INFO info) // OK
{
	if(this->m_count < 0 || this->m_count >= MAX_SET_ITEM_TYPE)
	{
		return;
	}

	this->m_SetItemTypeInfo[this->m_count++] = info;
}

SET_ITEM_TYPE_INFO* CSetItemType::GetInfo(int index) // OK
{
	for(int n=0;n < this->m_count;n++)
	{
		if(this->m_SetItemTypeInfo[n].Index == -1)
		{
			continue;
		}

		if(this->m_SetItemTypeInfo[n].Index == index)
		{
			return &this->m_SetItemTypeInfo[n];
		}
	}
	
	return 0;
}

bool CSetItemType::CheckSetItemType(int index) // OK
{
	return ((this->GetInfo(index)==0)?0:1);
}

int CSetItemType::GetSetItemOptionIndex(int index,int number) // OK
{
	SET_ITEM_TYPE_INFO* lpInfo = this->GetInfo(index);

	if(lpInfo == 0)
	{
		return -1;
	}

	if(number < 0 || number >= MAX_SET_ITEM_OPTION_INDEX)
	{
		return -1;
	}

	return lpInfo->OptionIndex[number];
}

int CSetItemType::GetSetItemStatType(int index) // OK
{
	SET_ITEM_TYPE_INFO* lpInfo = this->GetInfo(index);

	if(lpInfo == 0)
	{
		return -1;
	}

	return lpInfo->StatType;
}

int CSetItemType::GetRandomSetItem() // OK
{
	int IndexTable[MAX_SET_ITEM_TYPE],IndexTableCount=0;

	for(int n=0;n < this->m_count;n++)
	{
		ITEM_INFO ItemInfo;

		if(gItemManager.GetInfo(this->m_SetItemTypeInfo[n].Index,&ItemInfo) != 0 && ItemInfo.DropItem != 0)
		{
			IndexTable[IndexTableCount++] = ItemInfo.Index;
		}
	}

	return ((IndexTableCount==0)?-1:IndexTable[GetLargeRand()%IndexTableCount]);
}

void CSetItemType::MakeRandomSetItem(int aIndex,int map,int x,int y) // OK
{
	WORD ItemIndex = this->GetRandomSetItem();

	if(ItemIndex == -1)
	{
		return;
	}

	BYTE ItemLevel = 0;
	BYTE ItemOption1 = 0;
	BYTE ItemOption2 = 0;
	BYTE ItemOption3 = 0;
	BYTE ItemNewOption = 0;
	BYTE ItemSetOption = 0;
	BYTE ItemSocketOption[MAX_SOCKET_OPTION] = {0xFF,0xFF,0xFF,0xFF,0xFF};

	gItemOptionRate.GetItemOption0(2,&ItemLevel);

	gItemOptionRate.GetItemOption1(2,&ItemOption1);

	gItemOptionRate.GetItemOption2(2,&ItemOption2);

	gItemOptionRate.GetItemOption3(2,&ItemOption3);

	gItemOptionRate.GetItemOption4(2,&ItemNewOption);

	gItemOptionRate.GetItemOption5(2,&ItemSetOption);

	gItemOptionRate.GetItemOption6(2,&ItemSocketOption[0]);

	gItemOptionRate.MakeNewOption(ItemIndex,ItemNewOption,&ItemNewOption);

	gItemOptionRate.MakeSetOption(ItemIndex,ItemSetOption,&ItemSetOption);

	gItemOptionRate.MakeSocketOption(ItemIndex,ItemSocketOption[0],&ItemSocketOption[0]);

	GDCreateItemSend(aIndex,map,x,y,ItemIndex,ItemLevel,0,ItemOption1,ItemOption2,ItemOption3,aIndex,ItemNewOption,ItemSetOption,0,0,ItemSocketOption,0xFF,0);
}
