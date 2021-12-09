// MonsterAIGroupMember.h: interface for the CMonsterAIGroupMember class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

class CMonsterAIGroupMember
{
public:
	CMonsterAIGroupMember();
	virtual ~CMonsterAIGroupMember();
	void Reset();
public:
	int m_GroupNumber;
	int m_Guid;
	int m_Class;
	int m_Rank;
	int m_StartAI;
	int m_AI01;
	int m_AI02;
	int m_AI03;
	int m_CreateType;
	int m_MapNumber;
	int m_StartX;
	int m_StartY;
	int m_StartDir;
	int m_RegenType;
	int m_ObjIndex;
};
