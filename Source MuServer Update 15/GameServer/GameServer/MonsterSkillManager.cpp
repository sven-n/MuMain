// MonsterSkillManager.cpp: implementation of the CMonsterSkillManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MonsterSkillManager.h"
#include "Attack.h"
#include "Map.h"
#include "MemScript.h"
#include "MonsterSkillUnit.h"
#include "Protocol.h"
#include "SkillHitBox.h"
#include "SkillManager.h"
#include "Util.h"
#include "Viewport.h"

BOOL CMonsterSkillManager::m_DataLoad;
CMonsterSkillInfo CMonsterSkillManager::m_MonsterSkillInfoArray[MAX_MONSTER_SKILL_INFO];
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMonsterSkillManager::CMonsterSkillManager() // OK
{
	CMonsterSkillManager::DelAllSkillManagerInfo();
}

CMonsterSkillManager::~CMonsterSkillManager() // OK
{

}

void CMonsterSkillManager::LoadData(char* path) // OK
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

	CMonsterSkillManager::DelAllSkillManagerInfo();

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

					CMonsterSkillInfo info;

					info.m_MonsterIndex = lpMemScript->GetNumber();

					lpMemScript->GetToken();

					info.m_SkillUnitCount = 0;

					for(int n=0;n < MAX_MONSTER_SKILL_UNIT;n++)
					{
						info.m_SkillUnitTypeArray[n] = lpMemScript->GetAsNumber();

						info.m_SkillUnitArray[n] = CMonsterSkillUnit::FindSkillUnitInfo(lpMemScript->GetAsNumber());

						info.m_SkillUnitCount = ((info.m_SkillUnitArray[n]==0)?info.m_SkillUnitCount:(info.m_SkillUnitCount+1));
					}

					if(info.m_MonsterIndex >= 0 && info.m_MonsterIndex < MAX_MONSTER_SKILL_INFO)
					{
						CMonsterSkillManager::m_MonsterSkillInfoArray[info.m_MonsterIndex] = info;
					}
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

	CMonsterSkillManager::m_DataLoad = 1;

	delete lpMemScript;
}

void CMonsterSkillManager::DelAllSkillManagerInfo() // OK
{
	CMonsterSkillManager::m_DataLoad = 0;

	for(int n=0;n < MAX_MONSTER_SKILL_INFO;n++)
	{
		CMonsterSkillManager::m_MonsterSkillInfoArray[n].Reset();
	}
}

BOOL CMonsterSkillManager::CheckMonsterSkill(int MonsterClass,int MonsterSkillUnitType) // OK
{
	if(MonsterClass < 0 || MonsterClass >= MAX_MONSTER_SKILL_INFO)
	{
		return 0;
	}

	if(CMonsterSkillManager::m_MonsterSkillInfoArray[MonsterClass].IsValid() == 0)
	{
		return 0;
	}

	for(int n=0;n < MAX_MONSTER_SKILL_UNIT;n++)
	{
		if(CMonsterSkillManager::m_MonsterSkillInfoArray[MonsterClass].m_SkillUnitTypeArray[n] == MonsterSkillUnitType)
		{
			return 1;
		}
	}

	return 0;
}

CMonsterSkillUnitInfo* CMonsterSkillManager::FindMonsterSkillUnitInfo(int aIndex,int MonsterSkillUnitType) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(CMonsterSkillManager::m_MonsterSkillInfoArray[lpObj->Class].IsValid() == 0)
	{
		return 0;
	}

	int MonsterSkillUnitTypeTable[MAX_MONSTER_SKILL_UNIT];
	int MonsterSkillUnitTypeCount = 0;

	for(int n=0;n < MAX_MONSTER_SKILL_UNIT;n++)
	{
		if(CMonsterSkillManager::m_MonsterSkillInfoArray[lpObj->Class].m_SkillUnitTypeArray[n] == MonsterSkillUnitType)
		{
			MonsterSkillUnitTypeTable[MonsterSkillUnitTypeCount++] = n;
		}
	}

	return ((MonsterSkillUnitTypeCount==0)?0:CMonsterSkillManager::m_MonsterSkillInfoArray[lpObj->Class].m_SkillUnitArray[MonsterSkillUnitTypeTable[GetLargeRand()%MonsterSkillUnitTypeCount]]);
}

void CMonsterSkillManager::UseMonsterSkill(int aIndex,int bIndex,int MonsterSkillUnitType,int MonsterSkillUnit,CSkill* lpSkill) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(CMonsterSkillManager::m_MonsterSkillInfoArray[lpObj->Class].IsValid() == 0)
	{
		return;
	}

	CMonsterSkillUnitInfo* lpMonsterSkillUnitInfo = ((MonsterSkillUnit==-1)?CMonsterSkillManager::FindMonsterSkillUnitInfo(aIndex,MonsterSkillUnitType):CMonsterSkillManager::m_MonsterSkillInfoArray[lpObj->Class].m_SkillUnitArray[MonsterSkillUnit]);

	if(lpMonsterSkillUnitInfo == 0)
	{
		return;
	}

	switch(lpMonsterSkillUnitInfo->m_UnitTargetType)
	{
		case MSU_TARGET_TYPE_COMMON:
			bIndex = bIndex;
			break;
		case MSU_TARGET_TYPE_SELF:
			bIndex = aIndex;
			break;
	}

	GCMonsterSkillSend(lpObj,&gObj[bIndex],lpMonsterSkillUnitInfo->m_UnitNumber);

	if(lpMonsterSkillUnitInfo->m_UnitScopeType == MSU_SCOPE_TYPE_NONE)
	{
		lpMonsterSkillUnitInfo->RunSkill(aIndex,bIndex);
		CMonsterSkillManager::SpecialMonsterSkillAttack(lpObj,&gObj[bIndex],lpSkill,lpMonsterSkillUnitInfo);
	}
	else
	{
		for(int n=0;n < MAX_VIEWPORT;n++)
		{
			if(lpObj->VpPlayer2[n].state == VIEWPORT_NONE)
			{
				continue;
			}

			int index = lpObj->VpPlayer2[n].index;

			if(gObj[index].Type != OBJECT_USER || gObj[index].Live == 0 || gObj[index].State != OBJECT_PLAYING)
			{
				continue;
			}

			switch(lpMonsterSkillUnitInfo->m_UnitScopeType)
			{
				case MSU_SCOPE_TYPE_AREA:
					if(gObjCalcDistance(lpObj,&gObj[index]) < lpMonsterSkillUnitInfo->m_UnitScopeValue)
					{
						lpMonsterSkillUnitInfo->RunSkill(aIndex,index);
						CMonsterSkillManager::SpecialMonsterSkillAttack(lpObj,&gObj[index],lpSkill,lpMonsterSkillUnitInfo);
					}
					break;
				case MSU_SCOPE_TYPE_HITBOX:
					if(gSkillHitBox.HitCheck(gSkillManager.GetSkillAngle(lpObj->X,lpObj->Y,gObj[bIndex].X,gObj[bIndex].Y),lpObj->X,lpObj->Y,gObj[index].X,gObj[index].Y) != 0)
					{
						lpMonsterSkillUnitInfo->RunSkill(aIndex,index);
						CMonsterSkillManager::SpecialMonsterSkillAttack(lpObj,&gObj[index],lpSkill,lpMonsterSkillUnitInfo);
					}
					break;
			}
		}
	}
}

void CMonsterSkillManager::SpecialMonsterSkillAttack(LPOBJ lpObj,LPOBJ lpTarget,CSkill* lpSkill,CMonsterSkillUnitInfo* lpMonsterSkillUnitInfo) // OK
{
	if(lpMonsterSkillUnitInfo->m_UnitTargetType != MSU_TARGET_TYPE_COMMON)
	{
		return;
	}

	switch(lpObj->Class)
	{
		case 454:
			gAttack.Attack(lpObj,lpTarget,lpSkill,0,0,0,0,0);
			break;
		case 455:
			gAttack.Attack(lpObj,lpTarget,lpSkill,0,0,0,0,0);
			break;
		case 456:
			gAttack.Attack(lpObj,lpTarget,lpSkill,0,0,0,0,0);
			break;
		case 457:
			gAttack.Attack(lpObj,lpTarget,lpSkill,0,0,0,0,0);
			break;
		case 458:
			gAttack.Attack(lpObj,lpTarget,lpSkill,0,0,0,0,0);
			break;
		case 459:
			gAttack.Attack(lpObj,lpTarget,lpSkill,0,0,0,0,0);
			break;
		case 504:
			gAttack.Attack(lpObj,lpTarget,lpSkill,0,0,0,0,0);
			break;
		case 505:
			gAttack.Attack(lpObj,lpTarget,lpSkill,0,0,0,0,0);
			break;
		case 506:
			gAttack.Attack(lpObj,lpTarget,lpSkill,0,0,0,0,0);
			break;
		case 507:
			gAttack.Attack(lpObj,lpTarget,lpSkill,0,0,0,0,0);
			break;
		case 508:
			gAttack.Attack(lpObj,lpTarget,lpSkill,0,0,0,0,0);
			break;
		case 509:
			gAttack.Attack(lpObj,lpTarget,lpSkill,0,0,0,0,0);
			break;
		case 510:
			gAttack.Attack(lpObj,lpTarget,lpSkill,0,0,0,0,0);
			break;
		case 511:
			gAttack.Attack(lpObj,lpTarget,lpSkill,0,0,0,0,0);
			break;
		case 512:
			gAttack.Attack(lpObj,lpTarget,lpSkill,0,0,0,0,0);
			break;
		case 513:
			gAttack.Attack(lpObj,lpTarget,lpSkill,0,0,0,0,0);
			break;
		case 514:
			gAttack.Attack(lpObj,lpTarget,lpSkill,0,0,0,0,0);
			break;
		case 515:
			gAttack.Attack(lpObj,lpTarget,lpSkill,0,0,0,0,0);
			break;
		case 516:
			gAttack.Attack(lpObj,lpTarget,lpSkill,0,0,0,0,0);
			break;
		case 517:
			gAttack.Attack(lpObj,lpTarget,lpSkill,0,0,0,0,0);
			break;
		case 518:
			gAttack.Attack(lpObj,lpTarget,lpSkill,0,0,0,0,0);
			break;
		case 519:
			gAttack.Attack(lpObj,lpTarget,lpSkill,0,0,0,0,0);
			break;
		case 520:
			gAttack.Attack(lpObj,lpTarget,lpSkill,0,0,0,0,0);
			break;
		case 521:
			gAttack.Attack(lpObj,lpTarget,lpSkill,0,0,0,0,0);
			break;
		case 562:
			gAttack.Attack(lpObj,lpTarget,lpSkill,0,0,0,0,0);
			break;
		case 563:
			gAttack.Attack(lpObj,lpTarget,lpSkill,0,0,0,0,0);
			break;
		case 565:
			gAttack.Attack(lpObj,lpTarget,lpSkill,0,0,0,0,0);
			break;
	}
}
