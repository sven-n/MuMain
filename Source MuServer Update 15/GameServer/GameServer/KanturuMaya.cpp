// KanturuMaya.cpp: implementation of the CKanturuMaya class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "KanturuMaya.h"
#include "KanturuUtil.h"
#include "Map.h"
#include "MonsterSkillManager.h"
#include "ObjectManager.h"
#include "User.h"
#include "Util.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CKanturuMaya::CKanturuMaya() // OK
{
	this->Init();
}

CKanturuMaya::~CKanturuMaya() // OK
{

}

void CKanturuMaya::Init() // OK
{
	this->m_MayaObjIndex = -1;
	this->m_MayaSkillTime = 0;
	this->m_IceStormCount = 0;
}

void CKanturuMaya::KanturuMayaAct_IceStorm(int rate) // OK
{
	if((GetLargeRand()%10000) > rate)
	{
		return;
	}

	if(OBJECT_RANGE(this->m_MayaObjIndex) == 0)
	{
		return;
	}

	for(int n=OBJECT_START_USER;n < MAX_OBJECT;n++)
	{
		if(gObjIsConnectedGP(n) != 0 && gObj[n].Map == MAP_KANTURU3)
		{
			gKanturuUtil.NotifyKanturuAreaAttack(this->m_MayaObjIndex,n,0);
			CMonsterSkillManager::UseMonsterSkill(this->m_MayaObjIndex,n,31,-1,0);
			this->m_IceStormCount++;
		}
	}

	//LogAdd(LOG_BLACK,"[ KANTURU ][ IceStorm ] Skill Using(%d) : Index(%d) %s",this->m_IceStormCount,this->m_MayaObjIndex,gObj[this->m_MayaObjIndex].Name);
}

void CKanturuMaya::KanturuMayaAct_Hands() // OK
{
	if((GetTickCount()-this->m_MayaSkillTime) < 20000)
	{
		return;
	}

	if(OBJECT_RANGE(this->m_MayaObjIndex) == 0)
	{
		return;
	}

	this->m_MayaSkillTime = GetTickCount();

	for(int n=OBJECT_START_USER;n < MAX_OBJECT;n++)
	{
		if(gObjIsConnectedGP(n) != 0 && gObj[n].Map == MAP_KANTURU3)
		{
			gKanturuUtil.NotifyKanturuAreaAttack(this->m_MayaObjIndex,n,1);

			CMonsterSkillManager::UseMonsterSkill(this->m_MayaObjIndex,n,1,-1,0);

			if((gObj[n].Inventory[10].m_Index == GET_ITEM(13,38) && gObj[n].Inventory[10].m_Durability == 0) || (gObj[n].Inventory[11].m_Index == GET_ITEM(13,38) && gObj[n].Inventory[11].m_Durability == 0))
			{
				gObj[n].Life = 0;
				gObjectManager.CharacterLifeCheck(&gObj[this->m_MayaObjIndex],&gObj[n],0,0,0,0,0,0);
				//LogAdd(LOG_BLACK,"[ KANTURU ][ BrokenShower ] [%s][%s] User Dying cause NOT wearing MoonStone Pandent",gObj[n].Account,gObj[n].Name);
			}
		}
	}

	//LogAdd(LOG_BLACK,"[ KANTURU ][ BrokenShower ] Skill Using : Index(%d) %s",this->m_MayaObjIndex,gObj[this->m_MayaObjIndex].Name);
}

void CKanturuMaya::SetMayaObjIndex(int aIndex) // OK
{
	this->m_MayaObjIndex = aIndex;
}
