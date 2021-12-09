// RaklionBattleUser.h: interface for the CRaklionBattleUser class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

class CRaklionBattleUser
{
public:
	CRaklionBattleUser();
	virtual ~CRaklionBattleUser();
	void ResetData();
	void SetIndex(int aIndex);
	BOOL IsUseData();
	int GetIndex();
private:
	int m_Index;
	int m_IsUse;
};
