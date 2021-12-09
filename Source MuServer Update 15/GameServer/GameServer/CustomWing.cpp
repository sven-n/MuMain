// CustomWing.cpp: implementation of the CCustomWing class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CustomWing.h"
#include "MemScript.h"
#include "Util.h"

CCustomWing gCustomWing;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCustomWing::CCustomWing() // OK
{
	this->Init();
}

CCustomWing::~CCustomWing() // OK
{

}

void CCustomWing::Init() // OK
{
	for(int n=0;n < MAX_CUSTOM_WING;n++)
	{
		this->m_CustomWingInfo[n].Index = -1;
	}
}

void CCustomWing::Load(char* path) // OK
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

			CUSTOM_WING_INFO info;

			memset(&info,0,sizeof(info));

			info.Index = lpMemScript->GetNumber();

			info.ItemIndex = lpMemScript->GetAsNumber();

			info.DefenseConstA = lpMemScript->GetAsNumber();

			info.IncDamageConstA = lpMemScript->GetAsNumber();

			info.IncDamageConstB = lpMemScript->GetAsNumber();

			info.DecDamageConstA = lpMemScript->GetAsNumber();

			info.DecDamageConstB = lpMemScript->GetAsNumber();

			info.OptionIndex1 = lpMemScript->GetAsNumber();

			info.OptionValue1 = lpMemScript->GetAsNumber();

			info.OptionIndex2 = lpMemScript->GetAsNumber();

			info.OptionValue2 = lpMemScript->GetAsNumber();

			info.OptionIndex3 = lpMemScript->GetAsNumber();

			info.OptionValue3 = lpMemScript->GetAsNumber();

			info.NewOptionIndex1 = lpMemScript->GetAsNumber();

			info.NewOptionValue1 = lpMemScript->GetAsNumber();

			info.NewOptionIndex2 = lpMemScript->GetAsNumber();

			info.NewOptionValue2 = lpMemScript->GetAsNumber();

			info.NewOptionIndex3 = lpMemScript->GetAsNumber();

			info.NewOptionValue3 = lpMemScript->GetAsNumber();

			info.NewOptionIndex4 = lpMemScript->GetAsNumber();

			info.NewOptionValue4 = lpMemScript->GetAsNumber();

			info.ModelType = lpMemScript->GetAsNumber();

			strcpy_s(info.ModelName,lpMemScript->GetAsString());

			this->SetInfo(info);
		}
	}
	catch(...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;
}

void CCustomWing::SetInfo(CUSTOM_WING_INFO info) // OK
{
	if(info.Index < 0 || info.Index >= MAX_CUSTOM_WING)
	{
		return;
	}

	this->m_CustomWingInfo[info.Index] = info;
}

CUSTOM_WING_INFO* CCustomWing::GetInfo(int index) // OK
{
	if(index < 0 || index >= MAX_CUSTOM_WING)
	{
		return 0;
	}

	if(this->m_CustomWingInfo[index].Index != index)
	{
		return 0;
	}

	return &this->m_CustomWingInfo[index];
}

CUSTOM_WING_INFO* CCustomWing::GetInfoByItem(int ItemIndex) // OK
{
	for(int n=0;n < MAX_CUSTOM_WING;n++)
	{
		CUSTOM_WING_INFO* lpInfo = this->GetInfo(n);

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

bool CCustomWing::CheckCustomWing(int index) // OK
{
	if(this->GetInfo(index) != 0)
	{
		return 1;
	}

	return 0;
}

bool CCustomWing::CheckCustomWingByItem(int ItemIndex) // OK
{
	if(this->GetInfoByItem(ItemIndex) != 0)
	{
		return 1;
	}

	return 0;
}

int CCustomWing::GetCustomWingIndex(int ItemIndex) // OK
{
	CUSTOM_WING_INFO* lpInfo = this->GetInfoByItem(ItemIndex);

	if(lpInfo == 0)
	{
		return 0;
	}

	return lpInfo->Index;
}

int CCustomWing::GetCustomWingDefense(int ItemIndex,int ItemLevel) // OK
{
	CUSTOM_WING_INFO* lpInfo = this->GetInfoByItem(ItemIndex);

	if(lpInfo == 0)
	{
		return 0;
	}

	return (lpInfo->DefenseConstA*ItemLevel);
}

int CCustomWing::GetCustomWingIncDamage(int ItemIndex,int ItemLevel) // OK
{
	CUSTOM_WING_INFO* lpInfo = this->GetInfoByItem(ItemIndex);

	if(lpInfo == 0)
	{
		return 100;
	}

	return (lpInfo->IncDamageConstA+(ItemLevel*lpInfo->IncDamageConstB));
}

int CCustomWing::GetCustomWingDecDamage(int ItemIndex,int ItemLevel) // OK
{
	CUSTOM_WING_INFO* lpInfo = this->GetInfoByItem(ItemIndex);

	if(lpInfo == 0)
	{
		return 100;
	}

	return (lpInfo->DecDamageConstA-(ItemLevel*lpInfo->DecDamageConstB));
}

int CCustomWing::GetCustomWingOptionIndex(int ItemIndex,int OptionNumber) // OK
{
	CUSTOM_WING_INFO* lpInfo = this->GetInfoByItem(ItemIndex);

	if(lpInfo == 0)
	{
		return 0;
	}

	switch(OptionNumber)
	{
		case 0:
			return lpInfo->OptionIndex1;
		case 1:
			return lpInfo->OptionIndex2;
		case 2:
			return lpInfo->OptionIndex3;
	}

	return 0;
}

int CCustomWing::GetCustomWingOptionValue(int ItemIndex,int OptionNumber) // OK
{
	CUSTOM_WING_INFO* lpInfo = this->GetInfoByItem(ItemIndex);

	if(lpInfo == 0)
	{
		return 0;
	}

	switch(OptionNumber)
	{
		case 0:
			return lpInfo->OptionValue1;
		case 1:
			return lpInfo->OptionValue2;
		case 2:
			return lpInfo->OptionValue3;
	}

	return 0;
}

int CCustomWing::GetCustomWingNewOptionIndex(int ItemIndex,int OptionNumber) // OK
{
	CUSTOM_WING_INFO* lpInfo = this->GetInfoByItem(ItemIndex);

	if(lpInfo == 0)
	{
		return 0;
	}

	switch(OptionNumber)
	{
		case 0:
			return lpInfo->NewOptionIndex1;
		case 1:
			return lpInfo->NewOptionIndex2;
		case 2:
			return lpInfo->NewOptionIndex3;
		case 3:
			return lpInfo->NewOptionIndex4;
	}

	return 0;
}

int CCustomWing::GetCustomWingNewOptionValue(int ItemIndex,int OptionNumber) // OK
{
	CUSTOM_WING_INFO* lpInfo = this->GetInfoByItem(ItemIndex);

	if(lpInfo == 0)
	{
		return 0;
	}

	switch(OptionNumber)
	{
		case 0:
			return lpInfo->NewOptionValue1;
		case 1:
			return lpInfo->NewOptionValue2;
		case 2:
			return lpInfo->NewOptionValue3;
		case 3:
			return lpInfo->NewOptionValue4;
	}

	return 0;
}
