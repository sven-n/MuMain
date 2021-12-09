// CastleSiegeWeapon.h: interface for the CCastleSiegeWeapon class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Protocol.h"
#include "User.h"

#define MAX_CS_WEAPON_DAMAGED_TARGET 100
#define MAX_CS_WEAPON_CAL_DAMAGER 50
#define MAX_CS_WEAPON_ZONE 4

//**********************************************//
//************ Client -> GameServer ************//
//**********************************************//

struct PMSG_CASTLE_SIEGE_WEAPON_USE_RECV
{
	PSBMSG_HEAD header; // C1:B7:01
	BYTE index[2];
	BYTE PointIndex;
};

struct PMSG_CASTLE_SIEGE_WEAPON_DAMAGE_RECV
{
	PSBMSG_HEAD header; // C1:B7:04
	BYTE index[2];
};

//**********************************************//
//************ GameServer -> Client ************//
//**********************************************//

struct PMSG_CASTLE_SIEGE_WEAPON_SEND
{
	PSBMSG_HEAD header; // C1:B7:00
	BYTE result;
	BYTE type;
	BYTE index[2];
};

struct PMSG_CASTLE_SIEGE_WEAPON_USE_SEND
{
	PSBMSG_HEAD header; // C1:B7:01
	BYTE result;
	BYTE index[2];
	BYTE type;
	BYTE x;
	BYTE y;
};

struct PMSG_CASTLE_SIEGE_WEAPON_ATTACK_REGION_SEND
{
	PSBMSG_HEAD header; // C1:B7:02
	BYTE type;
	BYTE x;
	BYTE y;
};

//**********************************************//
//**********************************************//
//**********************************************//

struct CS_WEAPON_DAMAGED_TARGET_INFO
{
	int m_iWeaponObjIndex; // 0
	int m_iTargetObjIndex; // 4
	int m_iDamageDealingTime; // 8
	BOOL m_IsUsed; // C

	void Reset() // OK
	{
		this->m_iWeaponObjIndex = -1;
		this->m_iTargetObjIndex = -1;
		this->m_iDamageDealingTime = 0;
		this->m_IsUsed = 0;
	}
};

struct CS_WEAPON_CAL_DAMAGER_INFO
{
	int m_iWeaponObjIndex; // 0
	int m_iTargetX; // 4
	int m_iTargetY; // 8
	int m_iLimitTime; // C
	BOOL m_IsUsed; // 10

	void Reset() // OK
	{
		this->m_iWeaponObjIndex = -1;
		this->m_iTargetX = -1;
		this->m_iTargetY = -1;
		this->m_iLimitTime = -1;
		this->m_IsUsed = 0;
	}
};

class CCastleSiegeWeapon
{
public:
	CCastleSiegeWeapon();
	virtual ~CCastleSiegeWeapon();
	void MainProc();
	void CastleSiegeWeaponAct(int aIndex);
	bool AddWeaponDamagedTargetInfo(int aIndex,int bIndex,int DelayTime);
	bool AddWeaponCalDamageInfo(int aIndex,BYTE tx,BYTE ty,int DelayTime);
	bool GetWeaponCalDamageInfo(int aIndex,BYTE* ox,BYTE* oy);
	void GetTargetPointXY(int type,int PointIndex,BYTE* ox,BYTE* oy);
	bool Attack(LPOBJ lpObj,LPOBJ lpTarget);
	void NpcCastleSiegeWeaponOffense(LPOBJ lpNpc,LPOBJ lpObj);
	void NpcCastleSiegeWeaponDefense(LPOBJ lpNpc,LPOBJ lpObj);
	void CGCastleSiegeWeaponUseRecv(PMSG_CASTLE_SIEGE_WEAPON_USE_RECV* lpMsg,int aIndex);
	void CGCastleSiegeWeaponDamageRecv(PMSG_CASTLE_SIEGE_WEAPON_DAMAGE_RECV* lpMsg,int aIndex);
	void GCCastleSiegeWeaponAttackRegionSend(int aIndex,int type,BYTE x,BYTE y);
private:
	CS_WEAPON_DAMAGED_TARGET_INFO m_WeaponDamagedTargetInfo[MAX_CS_WEAPON_DAMAGED_TARGET]; // 4
	CS_WEAPON_CAL_DAMAGER_INFO m_WeaponCalDamageInfo[MAX_CS_WEAPON_CAL_DAMAGER]; // 644
};

extern CCastleSiegeWeapon gCastleSiegeWeapon;

static const BYTE gCastleSiegeWeaponOffenseZone[MAX_CS_WEAPON_ZONE][4] = {62,103,72,112,88,104,124,111,116,105,124,112,73,86,105,103};

static const BYTE gCastleSiegeWeaponDefenseZone[MAX_CS_WEAPON_ZONE][4] = {61,88,93,108,92,89,127,111,84,52,102,66,0,0,0,0};
