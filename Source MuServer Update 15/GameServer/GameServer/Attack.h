// Attack.h: interface for the CAttack class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Protocol.h"

//**********************************************//
//************ Client -> GameServer ************//
//**********************************************//

struct PMSG_ATTACK_RECV
{
	PBMSG_HEAD header; // C1:[PROTOCOL_CODE2]
	BYTE index[2];
	BYTE action;
	BYTE dir;
};

//**********************************************//
//**********************************************//
//**********************************************//

class CAttack
{
public:
	CAttack();
	virtual ~CAttack();
	bool Attack(LPOBJ lpObj,LPOBJ lpTarget,CSkill* lpSkill,bool send,BYTE flag,int damage,int count,bool combo);
	bool AttackElemental(LPOBJ lpObj,LPOBJ lpTarget,CSkill* lpSkill,bool send,BYTE flag,int damage,int count,bool combo);
	bool DecreaseArrow(LPOBJ lpObj);
	void WingSprite(LPOBJ lpObj,LPOBJ lpTarget,int* damage);
	void HelperSprite(LPOBJ lpObj,LPOBJ lpTarget,int* damage);
	void DamageSprite(LPOBJ lpObj,int damage);
	bool DarkHorseSprite(LPOBJ lpObj,int damage);
	bool FenrirSprite(LPOBJ lpObj,int damage);
	void WeaponDurabilityDown(LPOBJ lpObj,LPOBJ lpTarget);
	void ArmorDurabilityDown(LPOBJ lpObj,LPOBJ lpTarget);
	bool CheckPlayerTarget(LPOBJ lpObj,LPOBJ lpTarget);
	void MissSend(LPOBJ lpObj,LPOBJ lpTarget,CSkill* lpSkill,int send,int count);
	bool MissCheck(LPOBJ lpObj,LPOBJ lpTarget,CSkill* lpSkill,int send,int count,BYTE* miss);
	bool MissCheckPvP(LPOBJ lpObj,LPOBJ lpTarget,CSkill* lpSkill,int send,int count,BYTE* miss);
	bool MissCheckElemental(LPOBJ lpObj,LPOBJ lpTarget,CSkill* lpSkill,int send,int count,BYTE* miss);
	bool ApplySkillEffect(LPOBJ lpObj,LPOBJ lpTarget,CSkill* lpSkill,int damage);
	int GetTargetDefense(LPOBJ lpObj,LPOBJ lpTarget,WORD* effect);
	int GetTargetElementalDefense(LPOBJ lpObj,LPOBJ lpTarget,WORD* effect);
	int GetAttackDamage(LPOBJ lpObj,LPOBJ lpTarget,CSkill* lpSkill,WORD* effect,int TargetDefense);
	int GetAttackDamageWizard(LPOBJ lpObj,LPOBJ lpTarget,CSkill* lpSkill,WORD* effect,int TargetDefense);
	int GetAttackDamageCursed(LPOBJ lpObj,LPOBJ lpTarget,CSkill* lpSkill,WORD* effect,int TargetDefense);
	int GetAttackDamageFenrir(LPOBJ lpObj,LPOBJ lpTarget,CSkill* lpSkill,WORD* effect,int TargetDefense);
	int GetAttackDamageElemental(LPOBJ lpObj,LPOBJ lpTarget,CSkill* lpSkill,WORD* effect,int AttackDamage,int TargetDefense);
	int GetShieldDamage(LPOBJ lpObj,LPOBJ lpTarget,int damage);
	void GetPreviewDefense(LPOBJ lpObj,DWORD* defense);
	void GetPreviewPhysiDamage(LPOBJ lpObj,DWORD* DamageMin,DWORD* DamageMax,DWORD* MulDamage,DWORD* DivDamage);
	void GetPreviewMagicDamage(LPOBJ lpObj,DWORD* DamageMin,DWORD* DamageMax,DWORD* MulDamage,DWORD* DivDamage,DWORD* DamageRate);
	void GetPreviewCurseDamage(LPOBJ lpObj,DWORD* DamageMin,DWORD* DamageMax,DWORD* MulDamage,DWORD* DivDamage,DWORD* DamageRate);
	void GetPreviewDamageMultiplier(LPOBJ lpObj,DWORD* DamageMultiplier,DWORD* RFDamageMultiplierA,DWORD* RFDamageMultiplierB,DWORD* RFDamageMultiplierC);
	void CGAttackRecv(PMSG_ATTACK_RECV* lpMsg,int aIndex);
};

extern CAttack gAttack;
