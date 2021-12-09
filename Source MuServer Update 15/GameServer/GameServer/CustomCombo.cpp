// CustomCombo.cpp: implementation of the CCustomCombo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CommandManager.h"
#include "CustomCombo.h"
#include "GensSystem.h"
#include "Log.h"
#include "Map.h"
#include "MapManager.h"
#include "MemScript.h"
#include "Message.h"
#include "Notice.h"
#include "Util.h"

CCustomCombo gCustomCombo;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCustomCombo::CCustomCombo() // OK
{
	this->m_CustomComboSkillPrimary.clear();
	this->m_CustomComboSkillSecundary.clear();
}

CCustomCombo::~CCustomCombo() // OK
{

}

void CCustomCombo::Load(char* path) // OK
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

	this->m_CustomComboSkillPrimary.clear();
	this->m_CustomComboSkillSecundary.clear();

	int count=0;
	int count2=0;

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

					CUSTOMCOMBO_INFO info;

					info.Index = count;

					info.Skill = lpMemScript->GetNumber();

					this->m_CustomComboSkillPrimary.insert(std::pair<int,CUSTOMCOMBO_INFO>(info.Index,info));

					count++;
				}
				else if(section == 1)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}
					CUSTOMCOMBO_INFO info2;

					info2.Index = count2;

					info2.Skill = lpMemScript->GetNumber();

					this->m_CustomComboSkillSecundary.insert(std::pair<int,CUSTOMCOMBO_INFO>(info2.Index,info2));

					count2++;
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

bool CCustomCombo::CheckSkillPrimary(int Skill) // OK
{
	//LogAdd(LOG_BLACK,"%d",Skill);
	for(std::map<int,CUSTOMCOMBO_INFO>::iterator it=this->m_CustomComboSkillPrimary.begin();it != this->m_CustomComboSkillPrimary.end();it++)
	{
		if(it->second.Skill == Skill)
		{
			//LogAdd(LOG_BLACK,"Primary");
			return 1;
		}
	}
	return 0;
}

bool CCustomCombo::CheckSkillSecundary(int Skill) // OK
{
	//LogAdd(LOG_BLACK,"%d",Skill);
	for(std::map<int,CUSTOMCOMBO_INFO>::iterator it=this->m_CustomComboSkillSecundary.begin();it != this->m_CustomComboSkillSecundary.end();it++)
	{
		if(it->second.Skill == Skill)
		{
			//LogAdd(LOG_BLACK,"Secundary");
			return 1;
		}
	}
	return 0;
}



