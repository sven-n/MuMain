// MonsterAIGroup.h: interface for the CMonsterAIGroup class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "MonsterAIGroupMember.h"

#define MAX_MONSTER_AI_GROUP 50
#define MAX_MONSTER_AI_GROUP_MEMBER 100

class CMonsterAIGroup
{
public:
	CMonsterAIGroup();
	virtual ~CMonsterAIGroup();
	static void LoadData(char* path);
	static void ChangeAIOrder(int GroupNumber,int AIOrder);
	static BOOL DelGroupInstance(int GroupNumber);
	static void DelAllGroupInfo();
	static int FindGroupLeader(int GroupNumber);
	static int FindGroupMemberObjectIndex(int GroupNumber,int guid);
	static CMonsterAIGroupMember* FindGroupMember(int GroupNumber,int guid);
	static CMonsterAIGroupMember* FindGroupMemberToHeal(int ObjIndex,int GroupNumber,int guid,int distance,int rate);
	static CMonsterAIGroupMember* FindGroupMemberToSommon(int ObjIndex,int GroupNumber,int guid);
	static void InitAll();
	static void Init(int GroupNumber);
public:
	static BOOL m_DataLoad;
	static CMonsterAIGroupMember m_MonsterAIGroupMemberArray[MAX_MONSTER_AI_GROUP][MAX_MONSTER_AI_GROUP_MEMBER];
	static int m_MonsterAIGroupMemberCount[MAX_MONSTER_AI_GROUP];
};
