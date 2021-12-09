// KanturuBattleUserMng.cpp: implementation of the CKanturuBattleUserMng class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "KanturuBattleUserMng.h"
#include "Map.h"
#include "User.h"
#include "Util.h"

CKanturuBattleUserMng gKanturuBattleUserMng;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CKanturuBattleUserMng::CKanturuBattleUserMng() // OK
{
	this->ResetAllData();
}

CKanturuBattleUserMng::~CKanturuBattleUserMng() // OK
{

}

void CKanturuBattleUserMng::ResetAllData() // OK
{
	for(int n=0;n < MAX_KANTURU_BATTLE_USER;n++)
	{
		this->m_BattleUser[n].ResetData();
	}

	this->m_BattleUserCount = 0;

	this->m_BattleMaxUser = MAX_KANTURU_BATTLE_USER;
}

bool CKanturuBattleUserMng::AddUserData(int aIndex) // OK
{
	if(gObjIsConnectedGP(aIndex) == 0)
	{
		//LogAdd(LOG_BLACK,"[ KANTURU ][ BattleUser ] Add User Fail - Disconnect User [%s][%s]",gObj[aIndex].Account,gObj[aIndex].Name);
		return 0;
	}

	if(this->IsOverMaxUser() != 0)
	{
		//LogAdd(LOG_BLACK,"[ KANTURU ][ BattleUser ] Add User Fail - Over Max User [%s][%s]",gObj[aIndex].Account,gObj[aIndex].Name);
		return 0;
	}

	for(int n=0;n < MAX_KANTURU_BATTLE_USER;n++)
	{
		if(this->m_BattleUser[n].IsUseData() == 0)
		{
			this->m_BattleUser[n].SetIndex(aIndex);
			this->m_BattleUserCount++;
			return 1;
		}
	}

	return 0;
}

bool CKanturuBattleUserMng::DelUserData(int aIndex) // OK
{
	if(OBJECT_RANGE(aIndex) == 0)
	{
		//LogAdd(LOG_BLACK,"[ KANTURU ][ BattleUser ] Delete User Fail - Invalid Index:%d",aIndex);
		return 0;
	}

	for(int n=0;n < MAX_KANTURU_BATTLE_USER;n++)
	{
		if(this->m_BattleUser[n].IsUseData() != 0 && this->m_BattleUser[n].GetIndex() == aIndex)
		{
			this->m_BattleUser[n].ResetData();
			this->m_BattleUserCount--;
			//LogAdd(LOG_BLACK,"[ KANTURU ][ BattleUser ] Delete User - [%s][%s] Current Battle User:%d",gObj[aIndex].Account,gObj[aIndex].Name,this->m_BattleUserCount);
			return 1;
		}
	}

	return 0;
}

void CKanturuBattleUserMng::CheckUserState() // OK
{
	for(int n=0;n < MAX_KANTURU_BATTLE_USER;n++)
	{
		if(this->m_BattleUser[n].IsUseData() == 0)
		{
			continue;
		}

		if(gObjIsConnectedGP(this->m_BattleUser[n].GetIndex()) == 0)
		{
			this->DelUserData(this->m_BattleUser[n].GetIndex());
			//LogAdd(LOG_BLACK,"[ KANTURU ][ BattleUser ] Delete User - Disconnect [%s][%s]",gObj[this->m_BattleUser[n].GetIndex()].Account,gObj[this->m_BattleUser[n].GetIndex()].Name);
			continue;
		}

		if(gObj[this->m_BattleUser[n].GetIndex()].Map != MAP_KANTURU3)
		{
			this->DelUserData(this->m_BattleUser[n].GetIndex());
			//LogAdd(LOG_BLACK,"[ KANTURU ][ BattleUser ] Delete User - Map Move [%s][%s]",gObj[this->m_BattleUser[n].GetIndex()].Account,gObj[this->m_BattleUser[n].GetIndex()].Name);
			continue;
		}
	}
}

void CKanturuBattleUserMng::MoveAllUser(int gate) // OK
{
	for(int n=0;n < MAX_KANTURU_BATTLE_USER;n++)
	{
		if(this->m_BattleUser[n].IsUseData() != 0)
		{
			gObjMoveGate(this->m_BattleUser[n].GetIndex(),gate);
			//LogAdd(LOG_BLACK,"[ KANTURU ][ BattleUser ] [%s][%s] MoveGate(%d)",gObj[this->m_BattleUser[n].GetIndex()].Account,gObj[this->m_BattleUser[n].GetIndex()].Name,gate);
		}
	}
}

int CKanturuBattleUserMng::GetUserCount() // OK
{
	return this->m_BattleUserCount;
}

int CKanturuBattleUserMng::GetMaxUser() // OK
{
	return this->m_BattleMaxUser;
}

int CKanturuBattleUserMng::IsEmpty() // OK
{
	return ((this->m_BattleUserCount==0)?1:0);
}

int CKanturuBattleUserMng::IsOverMaxUser() // OK
{
	return ((this->m_BattleUserCount>=MAX_KANTURU_BATTLE_USER)?1:0);
}
