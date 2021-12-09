// ResetTable.h: interface for the CResetTable class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "User.h"

struct RESET_TABLE_INFO
{
	int MinReset;
	int MaxReset;
	int Level[MAX_ACCOUNT_LEVEL];
	int Money[MAX_ACCOUNT_LEVEL];
	int Point[MAX_ACCOUNT_LEVEL];
};

class CResetTable
{
public:
	CResetTable();
	virtual ~CResetTable();
	void Load(char* path);
	int GetResetLevel(LPOBJ lpObj);
	int GetResetMoney(LPOBJ lpObj);
	int GetResetPoint(LPOBJ lpObj);
private:
	std::vector<RESET_TABLE_INFO> m_ResetTableInfo;
};

extern CResetTable gResetTable;
