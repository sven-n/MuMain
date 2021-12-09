// DarkSpirit.h: interface for the CDarkSpirit class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Item.h"
#include "Protocol.h"

//**********************************************//
//************ GameServer -> Client ************//
//**********************************************//

struct PMSG_DARK_SPIRIT_MODE_SEND
{
	PBMSG_HEAD header; // C1:A7
	BYTE type;
	BYTE mode;
	BYTE index[2];
};

struct PMSG_DARK_SPIRIT_ATTACK_SEND
{
	PBMSG_HEAD header; // C1:A8
	BYTE type;
	BYTE action;
	BYTE index[2];
	BYTE target[2];
};

//**********************************************//
//**********************************************//
//**********************************************//

enum eDarkSpiritMode
{
	DARK_SPIRIT_MODE_NORMAL = 0,
	DARK_SPIRIT_MODE_ATTACK_RANDOM = 1,
	DARK_SPIRIT_MODE_ATTACK_WITH_MASTER = 2,
	DARK_SPIRIT_MODE_ATTACK_TARGET = 3,
};

class CDarkSpirit
{
public:
	CDarkSpirit();
	virtual ~CDarkSpirit();
	void Init();
	void MainProc();
	void ModeNormal();
	void ModeAttackRandom();
	void ModeAttackWithMaster();
	void ModeAttakTarget();
	void RangeAttack(int aIndex,int bIndex);
	void SendAttackMsg(int aIndex,int bIndex,bool send,BYTE action);
	void Set(int aIndex,CItem* lpItem);
	void SetMode(eDarkSpiritMode mode,int aIndex);
	void SetTarget(int aIndex);
	void ResetTarget(int aIndex);
	void ChangeCommand(int command,int aIndex);
	bool Attack(LPOBJ lpObj,LPOBJ lpTarget,CSkill* lpSkill,bool send,BYTE flag,BYTE action);
	void DarkSpiritSprite(LPOBJ lpObj,int damage);
	bool MissCheck(LPOBJ lpObj,LPOBJ lpTarget,CSkill* lpSkill,int send,BYTE* miss);
	int GetTargetDefense(LPOBJ lpObj,LPOBJ lpTarget,WORD* effect);
	int GetAttackDamage(LPOBJ lpObj,LPOBJ lpTarget,CSkill* lpSkill,WORD* effect,int TargetDefense);
	void GetPreviewAttackDamage(LPOBJ lpObj,DWORD* DamageMin,DWORD* DamageMax,DWORD* AttackSpeed,DWORD* AttackSuccessRate);
public:
	int m_AttackDamageMin;
	int m_AttackDamageMax;
	int m_AttackSpeed;
	int m_AttackSuccessRate;
	int m_MasterIndex;
	int m_TargetIndex;
	DWORD m_LastAttackTime;
	eDarkSpiritMode m_ActionMode;
	CItem* m_Item;
};

extern CDarkSpirit gDarkSpirit[MAX_OBJECT];
