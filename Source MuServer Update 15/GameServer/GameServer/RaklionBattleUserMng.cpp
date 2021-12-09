// Raklion.cpp: implementation of the CRaklion class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RaklionBattleUserMng.h"
#include "Map.h"
#include "Raklion.h"
#include "RaklionUtil.h"
#include "User.h"
#include "Util.h"

CRaklionBattleUserMng gRaklionBattleUserMng;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRaklionBattleUserMng::CRaklionBattleUserMng() // OK
{
	this->ResetAllData();
}

CRaklionBattleUserMng::~CRaklionBattleUserMng() // OK
{

}

void CRaklionBattleUserMng::ResetAllData() // OK
{
	this->m_UserInfo.clear();
}

void CRaklionBattleUserMng::AddUserData(int aIndex) // OK
{
	if(gObjIsConnectedGP(aIndex) == 0)
	{
		//LogAdd(LOG_BLACK,"[ RAKLION ][ BattleUser ] Add User Fail - Disconnect User [%s][%s]",gObj[aIndex].Account,gObj[aIndex].Name);
		return;
	}

	this->m_UserInfo.push_back(aIndex);

	gRaklionUtil.NotifyRaklionState(aIndex,gRaklion.GetRaklionState(),gRaklion.GetRaklionDetailState());

	//LogAdd(LOG_BLACK,"[ RAKLION ][ Battle User ] Add User [%s][%s] Current Battle User:%d",gObj[aIndex].Account,gObj[aIndex].Name,this->m_UserInfo.size());
}

void CRaklionBattleUserMng::DelUserData(int aIndex) // OK
{
	if(OBJECT_RANGE(aIndex) == 0)
	{
		//LogAdd(LOG_BLACK,"[ RAKLION ][ BattleUser ] Delete User Fail - Invalid Index:%d",aIndex);
		return;
	}

	for(int n=0;n < this->GetBattleUserCount();n++)
	{
		if(this->m_UserInfo[n] == aIndex)
		{
			this->m_UserInfo.erase((this->m_UserInfo.begin()+n));
			//LogAdd(LOG_BLACK,"[ RAKLION ][ BattleUser ] Delete User - [%s][%s] Current Battle User:%d",gObj[aIndex].Account,gObj[aIndex].Name,this->m_UserInfo.size());
			return;
		}
	}
}

void CRaklionBattleUserMng::CheckUserState() // OK
{
	for(int n=0;n < this->GetBattleUserCount();n++)
	{
		if(gObjIsConnected(this->m_UserInfo[n]) == 0)
		{
			this->DelUserData(this->m_UserInfo[n]);
			continue;
		}

		if(gObj[this->m_UserInfo[n]].Map != MAP_RAKLION2)
		{
			this->DelUserData(this->m_UserInfo[n]);
			continue;
		}
	}
}

void CRaklionBattleUserMng::CheckUserOnRaklionBossMap() // OK
{
	for(int n=OBJECT_START_USER;n < MAX_OBJECT;n++)
	{
		if(gObjIsConnectedGP(n) != 0 && gObj[n].Map == MAP_RAKLION2 && (gObj[n].Authority & 32) == 0)
		{
			if(this->IsBattleUser(n) == 0)
			{
				gObjMoveGate(n,286);
				//LogAdd(LOG_RED,"[ RAKLION ][ Invalid User ] Invalid Raklion Boss Map User[%s][%s]",gObj[n].Account,gObj[n].Name);
			}
		}
	}
}

bool CRaklionBattleUserMng::IsBattleUser(int aIndex) // OK
{
	for(int n=0;n < this->GetBattleUserCount();n++)
	{
		if(this->m_UserInfo[n] == aIndex)
		{
			return 1;
		}
	}

	return 0;
}

bool CRaklionBattleUserMng::CheckBattleUsers() // OK
{
	return this->m_UserInfo.empty();
}

int CRaklionBattleUserMng::GetBattleUserCount() // OK
{
	return this->m_UserInfo.size();
}

int CRaklionBattleUserMng::GetBattleUserIndex(int index) // OK
{
	return this->m_UserInfo[index];
}
