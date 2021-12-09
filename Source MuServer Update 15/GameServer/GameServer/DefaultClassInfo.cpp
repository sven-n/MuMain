// DefaultClassInfo.cpp: implementation of the CDefaultClassInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DefaultClassInfo.h"
#include "MemScript.h"
#include "Util.h"

CDefaultClassInfo gDefaultClassInfo;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDefaultClassInfo::CDefaultClassInfo() // OK
{
	this->Init();
}

CDefaultClassInfo::~CDefaultClassInfo() // OK
{

}

void CDefaultClassInfo::Init() // OK
{
	memset(this->m_DefaultClassInfo,0,sizeof(this->m_DefaultClassInfo));
}

void CDefaultClassInfo::Load(char* path) // OK
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

			DEFAULT_CLASS_INFO info;

			info.Class = lpMemScript->GetNumber();

			info.Strength = lpMemScript->GetAsNumber();

			info.Dexterity = lpMemScript->GetAsNumber();

			info.Vitality = lpMemScript->GetAsNumber();

			info.Energy = lpMemScript->GetAsNumber();

			info.Leadership = lpMemScript->GetAsNumber();

			info.MaxLife = lpMemScript->GetAsFloatNumber();

			info.MaxMana = lpMemScript->GetAsFloatNumber();

			info.LevelLife = lpMemScript->GetAsFloatNumber();

			info.LevelMana = lpMemScript->GetAsFloatNumber();

			info.VitalityToLife = lpMemScript->GetAsFloatNumber();

			info.EnergyToMana = lpMemScript->GetAsFloatNumber();

			this->SetInfo(info);
		}
	}
	catch(...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;
}

void CDefaultClassInfo::SetInfo(DEFAULT_CLASS_INFO info) // OK
{
	if(CHECK_RANGE(info.Class,MAX_CLASS) == 0)
	{
		return;
	}

	this->m_DefaultClassInfo[info.Class] = info;
}

int CDefaultClassInfo::GetCharacterDefaultStat(int Class,int stat) // OK
{
	if(CHECK_RANGE(Class,MAX_CLASS) == 0)
	{
		return 0;
	}

	switch(stat)
	{
		case 0:
			return this->m_DefaultClassInfo[Class].Strength;
		case 1:
			return this->m_DefaultClassInfo[Class].Dexterity;
		case 2:
			return this->m_DefaultClassInfo[Class].Vitality;
		case 3:
			return this->m_DefaultClassInfo[Class].Energy;
		case 4:
			return this->m_DefaultClassInfo[Class].Leadership;
	}

	return 0;
}
