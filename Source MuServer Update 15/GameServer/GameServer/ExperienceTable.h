// ExperienceTable.h: interface for the CExperienceTable class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "User.h"

struct EXPERIENCE_TABLE_INFO
{
	int MinLevel;
	int MaxLevel;
	int MinMasterLevel;
	int MaxMasterLevel;
	int MinReset;
	int MaxReset;
	int MinMasterReset;
	int MaxMasterReset;
	int ExperienceRate;
};

class CExperienceTable
{
public:
	CExperienceTable();
	virtual ~CExperienceTable();
	void Load(char* path);
	int GetExperienceRate(LPOBJ lpObj);
	int GetMasterExperienceRate(LPOBJ lpObj);
private:
	std::vector<EXPERIENCE_TABLE_INFO> m_ExperienceTableInfo;
};

extern CExperienceTable gExperienceTable;
