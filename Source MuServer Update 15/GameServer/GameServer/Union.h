// Union.h: interface for the CUnion class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "UnionInfo.h"

class CUnion
{
public:
	CUnion();
	virtual ~CUnion();
	int GetGuildRelationShipCount(int iGuildNumber, int iRelationShipType);
	int GetGuildRelationShip(int iGuildNumber, int iTargetGuildNumber);
	int SetGuildUnionMemberList(int iMasterGuildNumber, int iCount, int const * iList);
	int SetGuildRivalMemberList(int iMasterGuildNumber, int iCount, int const * iList);
	int GetGuildUnionMemberList(int iMasterGuildNumber, int& iCount, int * iList);
	int GetUnionName(int iMasterGuildNumber, char* szMasterGuildName);
	int AddUnion(int iMasterGuildNumber, char* szUnionMasterGuildName);
	int DelUnion(int iMasterGuildNumber);
	int DelAllUnion();
	CUnionInfo* SearchUnion(int iMasterGuildNumber);
private:
	std::map<int,CUnionInfo*> m_mpUnion; // 4
	TSync m_Sync; // 14
};

extern CUnion gUnionManager;
