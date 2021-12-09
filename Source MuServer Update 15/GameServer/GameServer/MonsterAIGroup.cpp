// MonsterAIGroup.cpp: implementation of the CMonsterAIGroup class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MonsterAIGroup.h"
#include "MemScript.h"
#include "Monster.h"
#include "Util.h"

BOOL CMonsterAIGroup::m_DataLoad;
CMonsterAIGroupMember CMonsterAIGroup::m_MonsterAIGroupMemberArray[MAX_MONSTER_AI_GROUP][MAX_MONSTER_AI_GROUP_MEMBER];
int CMonsterAIGroup::m_MonsterAIGroupMemberCount[MAX_MONSTER_AI_GROUP];
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMonsterAIGroup::CMonsterAIGroup() // OK
{
	CMonsterAIGroup::DelAllGroupInfo();
}

CMonsterAIGroup::~CMonsterAIGroup() // OK
{

}

void CMonsterAIGroup::LoadData(char* path) // OK
{
	CMemScript* lpMemScript = new CMemScript;

	if(lpMemScript == 0)
	{
		ErrorMessageBox(MEM_SCRIPT_ALLOC_ERROR,path);
		return;
	}

	if(lpMemScript->SetBuffer(path) == 0)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
		delete lpMemScript;
		return;
	}

	CMonsterAIGroup::DelAllGroupInfo();

	try
	{
		while(true)
		{
			if(lpMemScript->GetToken() == TOKEN_END)
			{
				break;
			}

			int section = lpMemScript->GetNumber();

			while(true)
			{
				if(section == 0)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					CMonsterAIGroupMember info;

					info.m_GroupNumber = lpMemScript->GetNumber();

					info.m_Guid = lpMemScript->GetAsNumber();

					info.m_Class = lpMemScript->GetAsNumber();

					info.m_Rank = lpMemScript->GetAsNumber();

					info.m_StartAI = lpMemScript->GetAsNumber();

					info.m_AI01 = lpMemScript->GetAsNumber();

					info.m_AI02 = lpMemScript->GetAsNumber();

					info.m_AI03 = lpMemScript->GetAsNumber();

					info.m_CreateType = lpMemScript->GetAsNumber();

					info.m_MapNumber = lpMemScript->GetAsNumber();

					info.m_StartX = lpMemScript->GetAsNumber();

					info.m_StartY = lpMemScript->GetAsNumber();

					info.m_StartDir = lpMemScript->GetAsNumber();

					info.m_RegenType = lpMemScript->GetAsNumber();

					if((info.m_GroupNumber >= 0 && info.m_GroupNumber < MAX_MONSTER_AI_GROUP) && (info.m_Guid >= 0 && info.m_Guid < MAX_MONSTER_AI_GROUP_MEMBER))
					{
						CMonsterAIGroup::m_MonsterAIGroupMemberArray[info.m_GroupNumber][info.m_Guid] = info;
						CMonsterAIGroup::m_MonsterAIGroupMemberCount[info.m_GroupNumber]++;
					}
				}
				else
				{
					break;
				}
			}
		}
	}
	catch(...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	CMonsterAIGroup::m_DataLoad = 1;

	delete lpMemScript;
}

void CMonsterAIGroup::ChangeAIOrder(int GroupNumber,int AIOrder) // OK
{
	if(GroupNumber < 0 || GroupNumber >= MAX_MONSTER_AI_GROUP)
	{
		return;
	}

	for(int n=0;n < MAX_MONSTER_AI_GROUP_MEMBER;n++)
	{
		CMonsterAIGroupMember* lpMonsterAIGroupMember = &CMonsterAIGroup::m_MonsterAIGroupMemberArray[GroupNumber][n];

		if(lpMonsterAIGroupMember->m_Guid == -1)
		{
			continue;
		}

		if(AIOrder == 0)
		{
			gObj[lpMonsterAIGroupMember->m_ObjIndex].BasicAI = lpMonsterAIGroupMember->m_StartAI;
			gObj[lpMonsterAIGroupMember->m_ObjIndex].CurrentAI = lpMonsterAIGroupMember->m_StartAI;
			continue;
		}

		if(AIOrder == 1)
		{
			gObj[lpMonsterAIGroupMember->m_ObjIndex].BasicAI = lpMonsterAIGroupMember->m_AI01;
			gObj[lpMonsterAIGroupMember->m_ObjIndex].CurrentAI = lpMonsterAIGroupMember->m_AI01;
			continue;
		}

		if(AIOrder == 2)
		{
			gObj[lpMonsterAIGroupMember->m_ObjIndex].BasicAI = lpMonsterAIGroupMember->m_AI02;
			gObj[lpMonsterAIGroupMember->m_ObjIndex].CurrentAI = lpMonsterAIGroupMember->m_AI02;
			continue;
		}

		if(AIOrder == 3)
		{
			gObj[lpMonsterAIGroupMember->m_ObjIndex].BasicAI = lpMonsterAIGroupMember->m_AI03;
			gObj[lpMonsterAIGroupMember->m_ObjIndex].CurrentAI = lpMonsterAIGroupMember->m_AI03;
			continue;
		}
	}
}

BOOL CMonsterAIGroup::DelGroupInstance(int GroupNumber) // OK
{
	if(GroupNumber < 0 || GroupNumber >= MAX_MONSTER_AI_GROUP)
	{
		return 0;
	}

	for(int n=0;n < MAX_MONSTER_AI_GROUP_MEMBER;n++)
	{
		CMonsterAIGroupMember* lpMonsterAIGroupMember = &CMonsterAIGroup::m_MonsterAIGroupMemberArray[GroupNumber][n];

		if(lpMonsterAIGroupMember->m_Guid == -1)
		{
			continue;
		}

		if(OBJECT_RANGE(lpMonsterAIGroupMember->m_ObjIndex) != 0)
		{
			gObjDel(lpMonsterAIGroupMember->m_ObjIndex);
		}
	}

	return 1;
}

void CMonsterAIGroup::DelAllGroupInfo() // OK
{
	CMonsterAIGroup::m_DataLoad = 0;

	for(int n=0;n < MAX_MONSTER_AI_GROUP;n++)for(int i=0;i < MAX_MONSTER_AI_GROUP_MEMBER;i++)
	{
		CMonsterAIGroup::m_MonsterAIGroupMemberArray[n][i].Reset();
	}
}

int CMonsterAIGroup::FindGroupLeader(int GroupNumber) // OK
{
	if(GroupNumber < 0 || GroupNumber >= MAX_MONSTER_AI_GROUP)
	{
		return -1;
	}

	for(int n=0;n < MAX_MONSTER_AI_GROUP_MEMBER;n++)
	{
		CMonsterAIGroupMember* lpMonsterAIGroupMember = &CMonsterAIGroup::m_MonsterAIGroupMemberArray[GroupNumber][n];

		if(lpMonsterAIGroupMember->m_Guid == -1)
		{
			continue;
		}

		if(lpMonsterAIGroupMember->m_Rank == 0)
		{
			return lpMonsterAIGroupMember->m_ObjIndex;
		}
	}

	return -1;
}

int CMonsterAIGroup::FindGroupMemberObjectIndex(int GroupNumber,int guid) // OK
{
	if(GroupNumber < 0 || GroupNumber >= MAX_MONSTER_AI_GROUP)
	{
		return -1;
	}

	for(int n=0;n < MAX_MONSTER_AI_GROUP_MEMBER;n++)
	{
		CMonsterAIGroupMember* lpMonsterAIGroupMember = &CMonsterAIGroup::m_MonsterAIGroupMemberArray[GroupNumber][n];

		if(lpMonsterAIGroupMember->m_Guid == -1)
		{
			continue;
		}

		if(lpMonsterAIGroupMember->m_Guid == guid)
		{
			return lpMonsterAIGroupMember->m_ObjIndex;
		}
	}

	return -1;
}

CMonsterAIGroupMember* CMonsterAIGroup::FindGroupMember(int GroupNumber,int guid) // OK
{
	if(GroupNumber < 0 || GroupNumber >= MAX_MONSTER_AI_GROUP)
	{
		return 0;
	}

	for(int n=0;n < MAX_MONSTER_AI_GROUP_MEMBER;n++)
	{
		CMonsterAIGroupMember* lpMonsterAIGroupMember = &CMonsterAIGroup::m_MonsterAIGroupMemberArray[GroupNumber][n];

		if(lpMonsterAIGroupMember->m_Guid == -1)
		{
			continue;
		}

		if(lpMonsterAIGroupMember->m_Guid == guid)
		{
			return lpMonsterAIGroupMember;
		}
	}

	return 0;
}

CMonsterAIGroupMember* CMonsterAIGroup::FindGroupMemberToHeal(int ObjIndex,int GroupNumber,int guid,int distance,int rate) // OK
{
	for(int n=0;n < MAX_MONSTER_AI_GROUP_MEMBER;n++)
	{
		CMonsterAIGroupMember* lpMonsterAIGroupMember = &CMonsterAIGroup::m_MonsterAIGroupMemberArray[GroupNumber][n];

		if(lpMonsterAIGroupMember->m_Guid == -1)
		{
			continue;
		}

		if(lpMonsterAIGroupMember->m_ObjIndex == ObjIndex)
		{
			continue;
		}

		if(gObjCalcDistance(&gObj[ObjIndex],&gObj[lpMonsterAIGroupMember->m_ObjIndex]) > distance)
		{
			continue;
		}

		if(((gObj[lpMonsterAIGroupMember->m_ObjIndex].Life*100)/(gObj[lpMonsterAIGroupMember->m_ObjIndex].MaxLife+gObj[lpMonsterAIGroupMember->m_ObjIndex].AddLife)) < rate)
		{
			return lpMonsterAIGroupMember;
		}
	}

	return 0;
}

CMonsterAIGroupMember* CMonsterAIGroup::FindGroupMemberToSommon(int ObjIndex,int GroupNumber,int guid) // OK
{
	for(int n=0;n < MAX_MONSTER_AI_GROUP_MEMBER;n++)
	{
		CMonsterAIGroupMember* lpMonsterAIGroupMember = &CMonsterAIGroup::m_MonsterAIGroupMemberArray[GroupNumber][n];

		if(lpMonsterAIGroupMember->m_Guid == -1)
		{
			continue;
		}

		if(lpMonsterAIGroupMember->m_ObjIndex == ObjIndex)
		{
			continue;
		}

		if(gObj[lpMonsterAIGroupMember->m_ObjIndex].RegenType == -1)
		{
			continue;
		}

		if(gObj[lpMonsterAIGroupMember->m_ObjIndex].Live == 0)
		{
			return lpMonsterAIGroupMember;
		}
	}

	return 0;
}

void CMonsterAIGroup::InitAll() // OK
{
	for(int n=0;n < MAX_MONSTER_AI_GROUP;n++)
	{
		CMonsterAIGroup::Init(n);
	}
}

void CMonsterAIGroup::Init(int GroupNumber) // OK
{
	if(GroupNumber < 0 || GroupNumber >= MAX_MONSTER_AI_GROUP)
	{
		return;
	}

	CMonsterAIGroup::DelGroupInstance(GroupNumber);

	for(int n=0;n < MAX_MONSTER_AI_GROUP_MEMBER;n++)
	{
		CMonsterAIGroupMember* lpMonsterAIGroupMember = &CMonsterAIGroup::m_MonsterAIGroupMemberArray[GroupNumber][n];

		if(lpMonsterAIGroupMember->m_Guid == -1)
		{
			continue;
		}

		int index = gObjAddMonster(lpMonsterAIGroupMember->m_MapNumber);

		if(OBJECT_RANGE(index) == 0)
		{
			continue;
		}

		LPOBJ lpObj = &gObj[index];

		int px = lpMonsterAIGroupMember->m_StartX;
		int py = lpMonsterAIGroupMember->m_StartY;

		if(lpMonsterAIGroupMember->m_CreateType == 1 && gObjGetRandomFreeLocation(lpMonsterAIGroupMember->m_MapNumber,&px,&py,10,10,100) == 0)
		{
			px = lpMonsterAIGroupMember->m_StartX;
			py = lpMonsterAIGroupMember->m_StartY;
		}

		lpObj->PosNum = -1;
		lpObj->X = px;
		lpObj->Y = py;
		lpObj->TX = px;
		lpObj->TY = py;
		lpObj->OldX = px;
		lpObj->OldY = py;
		lpObj->StartX = px;
		lpObj->StartY = py;
		lpObj->Dir = lpMonsterAIGroupMember->m_StartDir;
		lpObj->Map = lpMonsterAIGroupMember->m_MapNumber;

		if(gObjSetMonster(index,lpMonsterAIGroupMember->m_Class) == 0)
		{
			gObjDel(index);
			continue;
		}

		lpObj->GroupNumber = lpMonsterAIGroupMember->m_GroupNumber;
		lpObj->GroupMemberGuid = lpMonsterAIGroupMember->m_Guid;
		lpObj->CurrentAI = lpMonsterAIGroupMember->m_StartAI;
		lpObj->BasicAI = lpMonsterAIGroupMember->m_StartAI;
		lpObj->RegenType = lpMonsterAIGroupMember->m_RegenType;

		lpMonsterAIGroupMember->m_ObjIndex = index;
	}
}
