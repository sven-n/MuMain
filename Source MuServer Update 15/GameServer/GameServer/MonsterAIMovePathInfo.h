// MonsterAIMovePathInfo.h: interface for the CMonsterAIMovePathInfo class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

class CMonsterAIMovePathInfo
{
public:
	CMonsterAIMovePathInfo();
	void Reset();
public:
	int m_Type;
	int m_MapNumber;
	int m_PathX;
	int m_PathY;
};
