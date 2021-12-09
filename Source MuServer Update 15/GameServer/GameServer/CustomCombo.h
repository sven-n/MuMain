// Move.h: interface for the CMove class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Protocol.h"
#include "User.h"

struct CUSTOMCOMBO_INFO
{
	int Index;
	int Skill;
};

class CCustomCombo
{
public:
	CCustomCombo();
	virtual ~CCustomCombo();
	void Load(char* path);
	bool CheckSkillPrimary(int Skill);
	bool CheckSkillSecundary(int Skill);
private:
	std::map<int,CUSTOMCOMBO_INFO> m_CustomComboSkillPrimary;
	std::map<int,CUSTOMCOMBO_INFO> m_CustomComboSkillSecundary;
};

extern CCustomCombo gCustomCombo;
