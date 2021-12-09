// ItemOption.cpp: implementation of the CItemOption class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ItemExcellentOptionRate.h"
#include "ItemManager.h"
#include "MemScript.h"
#include "RandomManager.h"
#include "Util.h"

CItemExOptionRate gItemExOptionRate;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CItemExOptionRate::CItemExOptionRate() // OK
{
	this->m_ItemExOptionRateInfo.clear();
}

CItemExOptionRate::~CItemExOptionRate() // OK
{

}

void CItemExOptionRate::Load(char* path) // OK
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

	this->m_ItemExOptionRateInfo.clear();

	int count = 0;

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

			ITEM_EX_OPTION_RATE_INFO info;

			info.Index = count++;

			info.OptionIndex = lpMemScript->GetNumber();

			info.ItemMinIndex = lpMemScript->GetAsNumber();

			info.ItemMaxIndex = lpMemScript->GetAsNumber();

			strcpy_s(info.Name,lpMemScript->GetAsString());

			info.OptionRate = lpMemScript->GetAsNumber();

			this->m_ItemExOptionRateInfo.insert(std::pair<int,ITEM_EX_OPTION_RATE_INFO>(info.Index,info));
		}
	}
	catch(...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;
}

bool CItemExOptionRate::GetExcellentOptionByRate(int ItemIndex, int NewOption, BYTE* OptionIndex) // OK
{
	CRandomManager RandomManager;

	for(std::map<int,ITEM_EX_OPTION_RATE_INFO>::iterator it=this->m_ItemExOptionRateInfo.begin();it != this->m_ItemExOptionRateInfo.end();it++)
	{
		if(ItemIndex < it->second.ItemMinIndex)
		{
			continue;
		}

		if(ItemIndex > it->second.ItemMaxIndex)
		{
			continue;
		}

		if((NewOption & it->second.OptionIndex) != 0)
		{
			continue;
		}

		RandomManager.AddElement(it->second.OptionIndex,it->second.OptionRate);
	}
	return RandomManager.GetRandomElement(OptionIndex);
}