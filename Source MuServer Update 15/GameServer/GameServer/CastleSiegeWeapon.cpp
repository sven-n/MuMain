// CastleSiegeWeapon.cpp: implementation of the CCastleSiegeWeapon class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CastleSiegeWeapon.h"
#include "Kalima.h"
#include "ObjectManager.h"
#include "ServerInfo.h"
#include "Util.h"
#include "Viewport.h"

CCastleSiegeWeapon gCastleSiegeWeapon;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCastleSiegeWeapon::CCastleSiegeWeapon() // OK
{
	for(int n=0;n < MAX_CS_WEAPON_DAMAGED_TARGET;n++)
	{
		this->m_WeaponDamagedTargetInfo[n].Reset();
	}

	for(int n=0;n < MAX_CS_WEAPON_CAL_DAMAGER;n++)
	{
		this->m_WeaponCalDamageInfo[n].Reset();
	}
}

CCastleSiegeWeapon::~CCastleSiegeWeapon() // OK
{

}

void CCastleSiegeWeapon::MainProc() // OK
{
	for(int n=0;n < MAX_CS_WEAPON_DAMAGED_TARGET;n++)
	{
		if(this->m_WeaponDamagedTargetInfo[n].m_IsUsed == 0)
		{
			continue;
		}

		if(((DWORD)this->m_WeaponDamagedTargetInfo[n].m_iDamageDealingTime) > GetTickCount())
		{
			continue;
		}

		if(gObjIsConnected(this->m_WeaponDamagedTargetInfo[n].m_iWeaponObjIndex) == 0)
		{
			this->m_WeaponDamagedTargetInfo[n].Reset();
			continue;
		}

		if(gObjIsConnected(this->m_WeaponDamagedTargetInfo[n].m_iTargetObjIndex) == 0)
		{
			this->m_WeaponDamagedTargetInfo[n].Reset();
			continue;
		}

		if(this->Attack(&gObj[this->m_WeaponDamagedTargetInfo[n].m_iWeaponObjIndex],&gObj[this->m_WeaponDamagedTargetInfo[n].m_iTargetObjIndex]) == 0)
		{
			this->m_WeaponDamagedTargetInfo[n].Reset();
			continue;
		}

		this->m_WeaponDamagedTargetInfo[n].Reset();
	}
}

void CCastleSiegeWeapon::CastleSiegeWeaponAct(int aIndex) // OK
{
	for(int n=0;n < MAX_CS_WEAPON_CAL_DAMAGER;n++)
	{
		if(this->m_WeaponCalDamageInfo[n].m_IsUsed != 0 && this->m_WeaponCalDamageInfo[n].m_iWeaponObjIndex == aIndex)
		{
			if(((DWORD)this->m_WeaponCalDamageInfo[n].m_iLimitTime) < GetTickCount())
			{
				this->m_WeaponCalDamageInfo[n].Reset();
			}
		}
	}
}

bool CCastleSiegeWeapon::AddWeaponDamagedTargetInfo(int aIndex,int bIndex,int DelayTime) // OK
{
	for(int n=0;n < MAX_CS_WEAPON_DAMAGED_TARGET;n++)
	{
		if(this->m_WeaponDamagedTargetInfo[n].m_IsUsed == 0)
		{
			this->m_WeaponDamagedTargetInfo[n].m_iWeaponObjIndex = aIndex;
			this->m_WeaponDamagedTargetInfo[n].m_iTargetObjIndex = bIndex;
			this->m_WeaponDamagedTargetInfo[n].m_iDamageDealingTime = GetTickCount()+DelayTime;
			this->m_WeaponDamagedTargetInfo[n].m_IsUsed = 1;
			return 1;
		}
	}

	return 0;
}

bool CCastleSiegeWeapon::AddWeaponCalDamageInfo(int aIndex,BYTE tx,BYTE ty,int DelayTime) // OK
{
	for(int n=0;n < MAX_CS_WEAPON_CAL_DAMAGER;n++)
	{
		if(this->m_WeaponCalDamageInfo[n].m_IsUsed == 0)
		{
			this->m_WeaponCalDamageInfo[n].m_iWeaponObjIndex = aIndex;
			this->m_WeaponCalDamageInfo[n].m_iTargetX = tx;
			this->m_WeaponCalDamageInfo[n].m_iTargetY = ty;
			this->m_WeaponCalDamageInfo[n].m_iLimitTime = GetTickCount()+DelayTime;
			this->m_WeaponCalDamageInfo[n].m_IsUsed = 1;
			return 1;
		}
	}

	return 0;
}

bool CCastleSiegeWeapon::GetWeaponCalDamageInfo(int aIndex,BYTE* ox,BYTE* oy) // OK
{
	for(int n=0;n < MAX_CS_WEAPON_CAL_DAMAGER;n++)
	{
		if(this->m_WeaponCalDamageInfo[n].m_IsUsed != 0 && this->m_WeaponCalDamageInfo[n].m_iWeaponObjIndex == aIndex)
		{
			(*ox) = this->m_WeaponCalDamageInfo[n].m_iTargetX;
			(*oy) = this->m_WeaponCalDamageInfo[n].m_iTargetY;
			this->m_WeaponCalDamageInfo[n].Reset();
			return 1;
		}
	}

	return 0;
}

void CCastleSiegeWeapon::GetTargetPointXY(int type,int PointIndex,BYTE* ox,BYTE* oy) // OK
{
	if(PointIndex < 0 || PointIndex >= MAX_CS_WEAPON_ZONE)
	{
		return;
	}

	if(type == 0)
	{
		(*ox) = gCastleSiegeWeaponOffenseZone[PointIndex][0]+(GetLargeRand()%(gCastleSiegeWeaponOffenseZone[PointIndex][2]-gCastleSiegeWeaponOffenseZone[PointIndex][0]));
		(*oy) = gCastleSiegeWeaponOffenseZone[PointIndex][1]+(GetLargeRand()%(gCastleSiegeWeaponOffenseZone[PointIndex][3]-gCastleSiegeWeaponOffenseZone[PointIndex][1]));
	}

	if(type == 1)
	{
		(*ox) = gCastleSiegeWeaponDefenseZone[PointIndex][0]+(GetLargeRand()%(gCastleSiegeWeaponDefenseZone[PointIndex][2]-gCastleSiegeWeaponDefenseZone[PointIndex][0]));
		(*oy) = gCastleSiegeWeaponDefenseZone[PointIndex][1]+(GetLargeRand()%(gCastleSiegeWeaponDefenseZone[PointIndex][3]-gCastleSiegeWeaponDefenseZone[PointIndex][1]));
	}
}

bool CCastleSiegeWeapon::Attack(LPOBJ lpObj,LPOBJ lpTarget) // OK
{
	if(lpObj->Index == lpTarget->Index)
	{
		return 0;
	}

	if(lpObj->Type == OBJECT_USER && gObjIsConnectedGP(lpObj->Index) == 0)
	{
		return 0;
	}

	if(lpTarget->Type == OBJECT_USER && gObjIsConnectedGP(lpTarget->Index) == 0)
	{
		return 0;
	}

	if(lpObj->Map != lpTarget->Map)
	{
		return 0;
	}

	if(lpTarget->Live == 0 || lpTarget->State != OBJECT_PLAYING || lpTarget->Teleport != 0)
	{
		return 0;
	}

	if(lpTarget->Type == OBJECT_USER)
	{
		if(lpTarget->Authority == 32)
		{
			return 0;
		}
	}

	if(lpTarget->Type == OBJECT_MONSTER)
	{
		if(KALIMA_ATTRIBUTE_RANGE(lpTarget->Attribute) != 0)
		{
			return 0;
		}

		if((lpTarget->Class >= 100 && lpTarget->Class <= 110) || lpTarget->Class == 523) // Trap
		{
			return 0;
		}

		if(lpTarget->Class == 221 || lpTarget->Class == 222) // Siege
		{
			return 0;
		}
	}

	int damage = 0;

	if(lpObj->Class == 221)
	{
		damage = gServerInfo.m_CastleSiegeOffensiveWeaponDamage;
	}

	if(lpObj->Class == 222)
	{
		damage = gServerInfo.m_CastleSiegeDefensiveWeaponDamage;
	}

	if((lpTarget->Life-damage) < 0)
	{
		lpTarget->Life = 0;
	}
	else
	{
		lpTarget->Life -= damage;
	}

	gObjectManager.CharacterLifeCheck(lpObj,lpTarget,damage,0,0,0,0,0);
	return 1;
}

void CCastleSiegeWeapon::NpcCastleSiegeWeaponOffense(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	lpObj->Interface.use = 1;
	lpObj->Interface.type = INTERFACE_CASTLE_SIEGE_WEAPON_OFFENSE;
	lpObj->Interface.state = 0;

	PMSG_CASTLE_SIEGE_WEAPON_SEND pMsg;

	pMsg.header.set(0xB7,0x00,sizeof(pMsg));

	pMsg.result = 1;

	pMsg.type = 1;

	pMsg.index[0] = SET_NUMBERHB(lpNpc->Index);

	pMsg.index[1] = SET_NUMBERLB(lpNpc->Index);

	DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);
}

void CCastleSiegeWeapon::NpcCastleSiegeWeaponDefense(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	lpObj->Interface.use = 1;
	lpObj->Interface.type = INTERFACE_CASTLE_SIEGE_WEAPON_DEFENSE;
	lpObj->Interface.state = 0;

	PMSG_CASTLE_SIEGE_WEAPON_SEND pMsg;

	pMsg.header.set(0xB7,0x00,sizeof(pMsg));

	pMsg.result = 1;

	pMsg.type = 2;

	pMsg.index[0] = SET_NUMBERHB(lpNpc->Index);

	pMsg.index[1] = SET_NUMBERLB(lpNpc->Index);

	DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);
}

void CCastleSiegeWeapon::CGCastleSiegeWeaponUseRecv(PMSG_CASTLE_SIEGE_WEAPON_USE_RECV* lpMsg,int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(lpObj->Interface.use == 0 || (lpObj->Interface.type != INTERFACE_CASTLE_SIEGE_WEAPON_OFFENSE && lpObj->Interface.type != INTERFACE_CASTLE_SIEGE_WEAPON_DEFENSE))
	{
		return;
	}

	int bIndex = MAKE_NUMBERW(lpMsg->index[0],lpMsg->index[1]);

	if(OBJECT_RANGE(bIndex) == 0)
	{
		return;
	}

	LPOBJ lpTarget = &gObj[bIndex];

	if(lpTarget->Type != OBJECT_NPC || (lpTarget->Class != 221 && lpTarget->Class != 222))
	{
		return;
	}

	PMSG_CASTLE_SIEGE_WEAPON_USE_SEND pMsg;

	pMsg.header.set(0xB7,0x01,sizeof(pMsg));

	pMsg.result = 1;

	pMsg.index[0] = SET_NUMBERHB(bIndex);

	pMsg.index[1] = SET_NUMBERLB(bIndex);

	pMsg.type = lpTarget->Class-220;

	this->GetTargetPointXY((pMsg.type-1),(lpMsg->PointIndex-1),&pMsg.x,&pMsg.y);

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);

	pMsg.result = 2;

	for(int n=0;n < MAX_VIEWPORT;n++)
	{
		if(lpTarget->VpPlayer2[n].state != VIEWPORT_NONE && lpTarget->VpPlayer2[n].index != aIndex && lpTarget->VpPlayer2[n].type == OBJECT_USER)
		{
			DataSend(lpTarget->VpPlayer2[n].index,(BYTE*)&pMsg,pMsg.header.size);
		}
	}

	gViewport.GCViewportCastleSiegeWeaponSend(aIndex,pMsg.x,pMsg.y);

	this->GCCastleSiegeWeaponAttackRegionSend(aIndex,(lpTarget->Class-220),pMsg.x,pMsg.y);

	this->AddWeaponCalDamageInfo(bIndex,pMsg.x,pMsg.y,10000);
}

void CCastleSiegeWeapon::CGCastleSiegeWeaponDamageRecv(PMSG_CASTLE_SIEGE_WEAPON_DAMAGE_RECV* lpMsg,int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	if(lpObj->Interface.use == 0 || (lpObj->Interface.type != INTERFACE_CASTLE_SIEGE_WEAPON_OFFENSE && lpObj->Interface.type != INTERFACE_CASTLE_SIEGE_WEAPON_DEFENSE))
	{
		return;
	}

	int bIndex = MAKE_NUMBERW(lpMsg->index[0],lpMsg->index[1]);

	if(OBJECT_RANGE(bIndex) == 0)
	{
		return;
	}

	LPOBJ lpTarget = &gObj[bIndex];

	if(lpTarget->Type != OBJECT_NPC || (lpTarget->Class != 221 && lpTarget->Class != 222))
	{
		return;
	}

	BYTE tx,ty;

	if(this->GetWeaponCalDamageInfo(bIndex,&tx,&ty) == 0)
	{
		return;
	}

	for(int n=0;n < MAX_OBJECT;n++)
	{
		if(gObjIsConnected(n) == 0)
		{
			continue;
		}

		if(gObj[n].Map != lpTarget->Map)
		{
			continue;
		}

		if(gObj[n].X < (tx-3) || gObj[n].X > (tx+3) || gObj[n].Y < (ty-3) || gObj[n].Y > (ty+3))
		{
			continue;
		}

		this->AddWeaponDamagedTargetInfo(bIndex,n,100);
	}
}

void CCastleSiegeWeapon::GCCastleSiegeWeaponAttackRegionSend(int aIndex,int type,BYTE x,BYTE y) // OK
{
	PMSG_CASTLE_SIEGE_WEAPON_ATTACK_REGION_SEND pMsg;

	pMsg.header.set(0xB7,0x02,sizeof(pMsg));

	pMsg.type = type;

	pMsg.x = x;

	pMsg.y = y;

	for(int n=0;n < MAX_OBJECT;n++)
	{
		if(gObjIsConnected(n) == 0)
		{
			continue;
		}

		if(gObj[n].Map != gObj[aIndex].Map)
		{
			continue;
		}

		if(gObj[n].X < (x-6) || gObj[n].X > (x+6) || gObj[n].Y < (y-6) || gObj[n].Y > (y+6))
		{
			continue;
		}

		DataSend(n,(BYTE*)&pMsg,pMsg.header.size);
	}
}
