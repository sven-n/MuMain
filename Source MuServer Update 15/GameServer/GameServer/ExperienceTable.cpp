// ExperienceTable.cpp: implementation of the CExperienceTable class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ExperienceTable.h"
#include "MemScript.h"
#include "Util.h"

CExperienceTable gExperienceTable;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CExperienceTable::CExperienceTable() // OK
{
	this->m_ExperienceTableInfo.clear();
}

CExperienceTable::~CExperienceTable() // OK
{

}

void CExperienceTable::Load(char* path) // OK
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

	this->m_ExperienceTableInfo.clear();

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

			EXPERIENCE_TABLE_INFO info;

			info.MinLevel = lpMemScript->GetNumber();

			info.MaxLevel = lpMemScript->GetAsNumber();

			info.MinMasterLevel = lpMemScript->GetAsNumber();

			info.MaxMasterLevel = lpMemScript->GetAsNumber();

			info.MinReset = lpMemScript->GetAsNumber();

			info.MaxReset = lpMemScript->GetAsNumber();

			info.MinMasterReset = lpMemScript->GetAsNumber();

			info.MaxMasterReset = lpMemScript->GetAsNumber();

			info.ExperienceRate = lpMemScript->GetAsNumber();

			this->m_ExperienceTableInfo.push_back(info);
		}
	}
	catch(...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;
}

int CExperienceTable::GetExperienceRate(LPOBJ lpObj) // OK
{
	for(std::vector<EXPERIENCE_TABLE_INFO>::iterator it=this->m_ExperienceTableInfo.begin();it != this->m_ExperienceTableInfo.end();it++)
	{
		if(it->MinLevel != -1 && it->MinLevel > lpObj->Level)
		{
			continue;
		}

		if(it->MaxLevel != -1 && it->MaxLevel < lpObj->Level)
		{
			continue;
		}

		if(it->MinReset != -1 && it->MinReset > lpObj->Reset)
		{
			continue;
		}

		if(it->MaxReset != -1 && it->MaxReset < lpObj->Reset)
		{
			continue;
		}

		if(it->MinMasterReset != -1 && it->MinMasterReset > lpObj->MasterReset)
		{
			continue;
		}

		if(it->MaxMasterReset != -1 && it->MaxMasterReset < lpObj->MasterReset)
		{
			continue;
		}

		return it->ExperienceRate;
	}

	return 100;
}

int CExperienceTable::GetMasterExperienceRate(LPOBJ lpObj) // OK
{
	for(std::vector<EXPERIENCE_TABLE_INFO>::iterator it=this->m_ExperienceTableInfo.begin();it != this->m_ExperienceTableInfo.end();it++)
	{
		if(it->MinMasterLevel != -1 && it->MinMasterLevel > lpObj->MasterLevel)
		{
			continue;
		}

		if(it->MaxMasterLevel != -1 && it->MaxMasterLevel < lpObj->MasterLevel)
		{
			continue;
		}

		if(it->MinReset != -1 && it->MinReset > lpObj->Reset)
		{
			continue;
		}

		if(it->MaxReset != -1 && it->MaxReset < lpObj->Reset)
		{
			continue;
		}

		if(it->MinMasterReset != -1 && it->MinMasterReset > lpObj->MasterReset)
		{
			continue;
		}

		if(it->MaxMasterReset != -1 && it->MaxMasterReset < lpObj->MasterReset)
		{
			continue;
		}

		return it->ExperienceRate;
	}

	return 100;
}
