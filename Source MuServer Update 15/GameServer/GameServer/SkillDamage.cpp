// CustomMove.cpp: implementation of the CCustomMove class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SkillDamage.h"
#include "MemScript.h"
#include "Util.h"

CSkillDamage gSkillDamage;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSkillDamage::CSkillDamage() // OK
{
	this->m_SkillDamageInfo.clear();
}

CSkillDamage::~CSkillDamage() // OK
{

}

void CSkillDamage::Load(char* path) // OK
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

	this->m_SkillDamageInfo.clear();

	int m_count = 0;

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

			SKILLDAMAGE_INFO info;

			info.Index = m_count;

			info.Skill = lpMemScript->GetNumber();

			info.DamageRateUser = lpMemScript->GetAsNumber();

			info.DamageRateMonster = lpMemScript->GetAsNumber();

			this->m_SkillDamageInfo.insert(std::pair<int,SKILLDAMAGE_INFO>(info.Skill,info));

			m_count++;
		}
	}
	catch(...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;
}

bool CSkillDamage::GetInfo(int index,SKILLDAMAGE_INFO* lpInfo) // OK
{
	std::map<int,SKILLDAMAGE_INFO>::iterator it = this->m_SkillDamageInfo.find(index);

	if(it == this->m_SkillDamageInfo.end())
	{
		return 0;
	}
	else
	{
		(*lpInfo) = it->second;
		return 1;
	}
}

int CSkillDamage::GetSkillDamage(LPOBJ lpObj,int index,int type) // OK
{
	SKILLDAMAGE_INFO SkillDamageInfo;

	for(std::map<int,SKILLDAMAGE_INFO>::iterator it=this->m_SkillDamageInfo.begin();it != this->m_SkillDamageInfo.end();it++)
	{
		if(it->second.Skill == index)
		{
			if (type == OBJECT_USER)
			{
				return it->second.DamageRateUser;
			}
			else
			{
				return it->second.DamageRateMonster;
			}
		}
	}
	return 100;
}