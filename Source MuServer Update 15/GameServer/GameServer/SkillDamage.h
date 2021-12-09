// Move.h: interface for the CMove class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Protocol.h"
#include "User.h"

struct SKILLDAMAGE_INFO
{
	int Index;
	int Skill;
	int DamageRateUser;
	int DamageRateMonster;
};

class CSkillDamage
{
public:
	CSkillDamage();
	virtual ~CSkillDamage();
	void Load(char* path);
    bool GetInfo(int index,SKILLDAMAGE_INFO* lpInfo);
	int GetSkillDamage(LPOBJ lpObj,int index,int type);
private:
	std::map<int,SKILLDAMAGE_INFO> m_SkillDamageInfo;
};

extern CSkillDamage gSkillDamage;
