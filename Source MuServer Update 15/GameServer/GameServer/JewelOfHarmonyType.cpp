// JewelOfHarmonyType.cpp: implementation of the CJewelOfHarmonyType class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "JewelOfHarmonyType.h"
#include "ItemManager.h."
#include "MemScript.h"
#include "Util.h"

CJewelOfHarmonyType gJewelOfHarmonyType;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CJewelOfHarmonyType::CJewelOfHarmonyType() // OK
{
	this->Init();
}

CJewelOfHarmonyType::~CJewelOfHarmonyType() // OK
{

}

void CJewelOfHarmonyType::Init() // OK
{
	for(int n=0;n < MAX_JEWEL_OF_HARMONY_TYPE;n++)
	{
		this->m_JewelOfHarmonyTypeInfo[n].Index = -1;
	}
	
	this->m_count = 0;
}

void CJewelOfHarmonyType::Load(char* path) // OK
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

			JEWEL_OF_HARMONY_TYPE_INFO info;

			info.Index = SafeGetItem(GET_ITEM(lpMemScript->GetNumber(),lpMemScript->GetAsNumber()));

			info.Level = lpMemScript->GetAsNumber();

			this->SetInfo(info);
		}
	}
	catch(...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;
}

void CJewelOfHarmonyType::SetInfo(JEWEL_OF_HARMONY_TYPE_INFO info) // OK
{
	if(this->m_count < 0 || this->m_count >= MAX_JEWEL_OF_HARMONY_TYPE)
	{
		return;
	}

	this->m_JewelOfHarmonyTypeInfo[this->m_count++] = info;
}

JEWEL_OF_HARMONY_TYPE_INFO* CJewelOfHarmonyType::GetInfo(int index) // OK
{
	for(int n=0;n < MAX_JEWEL_OF_HARMONY_TYPE;n++)
	{
		if(this->m_JewelOfHarmonyTypeInfo[n].Index == -1)
		{
			continue;
		}

		if(this->m_JewelOfHarmonyTypeInfo[n].Index == index)
		{
			return &this->m_JewelOfHarmonyTypeInfo[n];
		}
	}

	return 0;
};

bool CJewelOfHarmonyType::CheckJewelOfHarmonyItemType(CItem* lpItem) // OK
{
	JEWEL_OF_HARMONY_TYPE_INFO* lpInfo = this->GetInfo(lpItem->m_Index);

	if(lpInfo != 0)
	{
		if(lpInfo->Level > lpItem->m_Level)
		{
			return 0;
		}
	}

	return 1;
}
