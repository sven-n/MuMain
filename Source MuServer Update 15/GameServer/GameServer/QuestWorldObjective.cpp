// QuestWorldObjective.cpp: implementation of the CQuestWorldObjective class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "QuestWorldObjective.h"
#include "DSProtocol.h"
#include "MemScript.h"
#include "Monster.h"
#include "Notice.h"
#include "Party.h"
#include "QuestWorld.h"
#include "SkillManager.h"
#include "Util.h"

CQuestWorldObjective gQuestWorldObjective;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CQuestWorldObjective::CQuestWorldObjective() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	this->m_count = 0;

	#endif
}

CQuestWorldObjective::~CQuestWorldObjective() // OK
{

}

void CQuestWorldObjective::Load(char* path) // OK
{
	#if(GAMESERVER_UPDATE>=501)

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

	this->m_count = 0;

	try
	{
		while(true)
		{
			if(lpMemScript->GetToken() == TOKEN_END)
			{
				break;
			}

			if(strcmp("end",lpMemScript->GetString()) == 0)
			{
				break;
			}

			QUEST_WORLD_OBJECTIVE_INFO info;

			memset(&info,0,sizeof(info));

			info.Number = lpMemScript->GetNumber();

			info.Type = lpMemScript->GetAsNumber();

			info.Index = lpMemScript->GetAsNumber();

			info.Value = lpMemScript->GetAsNumber();

			info.Level = lpMemScript->GetAsNumber();

			info.Option1 = lpMemScript->GetAsNumber();

			info.Option2 = lpMemScript->GetAsNumber();

			info.Option3 = lpMemScript->GetAsNumber();

			info.NewOption = lpMemScript->GetAsNumber();

			info.RequireIndex = lpMemScript->GetAsNumber();

			info.RequireGroup = lpMemScript->GetAsNumber();

			info.RequireState = lpMemScript->GetAsNumber();

			for(int n=0;n < MAX_CLASS;n++){info.RequireClass[n] = lpMemScript->GetAsNumber();}

			this->SetInfo(info);
		}
	}
	catch(...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	delete lpMemScript;

	#endif
}

void CQuestWorldObjective::SetInfo(QUEST_WORLD_OBJECTIVE_INFO info) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	if(this->m_count < 0 || this->m_count >= MAX_QUEST_WORLD_OBJECTIVE)
	{
		return;
	}

	this->m_QuestWorldObjectiveInfo[this->m_count++] = info;

	#endif
}

QUEST_WORLD_OBJECTIVE_INFO* CQuestWorldObjective::GetInfo(int index) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	if(index < 0 || index >= this->m_count)
	{
		return 0;
	}

	return &this->m_QuestWorldObjectiveInfo[index];

	#else

	return 0;

	#endif
}

int CQuestWorldObjective::GetQuestWorldObjectiveCount(LPOBJ lpObj,QUEST_WORLD_OBJECTIVE_INFO* lpInfo) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	if(lpInfo->Type == QUEST_WORLD_OBJECTIVE_NONE)
	{
		return 0;
	}

	if(lpInfo->Type == QUEST_WORLD_OBJECTIVE_MONSTER)
	{
		return this->GetQuestWorldObjectiveKillCount(lpObj,lpInfo);
	}

	if(lpInfo->Type == QUEST_WORLD_OBJECTIVE_SKILL)
	{
		return ((gSkillManager.GetSkill(lpObj,lpInfo->Index)==0)?0:1);
	}

	if(lpInfo->Type == QUEST_WORLD_OBJECTIVE_ITEM)
	{
		return gItemManager.GetInventoryItemCount(lpObj,lpInfo->Index,lpInfo->Level);
	}

	if(lpInfo->Type == QUEST_WORLD_OBJECTIVE_LEVEL)
	{
		return lpObj->Level;
	}

	if(lpInfo->Type == QUEST_WORLD_OBJECTIVE_SPECIAL)
	{
		return this->GetQuestWorldObjectiveKillCount(lpObj,lpInfo);
	}

	return 0;

	#else

	return 0;

	#endif
}

int CQuestWorldObjective::GetQuestWorldObjectiveKillCount(LPOBJ lpObj,QUEST_WORLD_OBJECTIVE_INFO* lpInfo) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	QUEST_WORLD_LIST* lpQuestWorldList = gQuestWorld.GetQuestWorldList(lpObj,lpInfo->RequireIndex,lpInfo->RequireGroup);

	if(lpQuestWorldList == 0)
	{
		return 0;
	}

	return lpQuestWorldList->ObjectiveCount[lpInfo->Number];

	#else

	return 0;

	#endif
}

bool CQuestWorldObjective::GetQuestWorldObjective(LPOBJ lpObj,int QuestIndex,int QuestGroup,QUEST_WORLD_OBJECTIVE* lpObjective,int ObjectiveNumber) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	for(int n=0;n < this->m_count;n++)
	{
		QUEST_WORLD_OBJECTIVE_INFO* lpInfo = this->GetInfo(n);

		if(lpInfo == 0)
		{
			continue;
		}

		if(lpInfo->RequireIndex != QuestIndex)
		{
			continue;
		}

		if(lpInfo->RequireGroup != QuestGroup)
		{
			continue;
		}

		if(this->CheckQuestWorldObjectiveRequisite(lpObj,lpInfo) == 0)
		{
			continue;
		}

		if(lpInfo->Number != ObjectiveNumber)
		{
			continue;
		}

		lpObjective->type = lpInfo->Type;

		lpObjective->index = lpInfo->Index;

		lpObjective->value = lpInfo->Value;

		lpObjective->count = this->GetQuestWorldObjectiveCount(lpObj,lpInfo);

		if(lpInfo->Type == QUEST_WORLD_OBJECTIVE_ITEM)
		{
			CItem item;

			item.m_Level = lpInfo->Level;

			item.Convert(lpInfo->Index,lpInfo->Option1,lpInfo->Option2,lpInfo->Option3,lpInfo->NewOption,0,0,0,0,0xFF);

			gItemManager.ItemByteConvert(lpObjective->ItemInfo,item);
		}

		return 1;
	}

	return 0;

	#else

	return 0;

	#endif
}

bool CQuestWorldObjective::CheckQuestWorldObjectiveRequisite(LPOBJ lpObj,QUEST_WORLD_OBJECTIVE_INFO* lpInfo) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	if(lpInfo->RequireIndex != -1 && gQuestWorld.CheckQuestWorldListState(lpObj,lpInfo->RequireIndex,lpInfo->RequireGroup,lpInfo->RequireState) == 0)
	{
		return 0;
	}

	if(lpInfo->RequireClass[lpObj->Class] == 0 || lpInfo->RequireClass[lpObj->Class] > (lpObj->ChangeUp+1))
	{
		return 0;
	}

	return 1;

	#else

	return 0;

	#endif
}

bool CQuestWorldObjective::CheckQuestWorldObjective(LPOBJ lpObj,int QuestIndex,int QuestGroup) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	for(int n=0;n < this->m_count;n++)
	{
		QUEST_WORLD_OBJECTIVE_INFO* lpInfo = this->GetInfo(n);

		if(lpInfo == 0)
		{
			continue;
		}

		if(lpInfo->RequireIndex != QuestIndex)
		{
			continue;
		}

		if(lpInfo->RequireGroup != QuestGroup)
		{
			continue;
		}

		if(this->CheckQuestWorldObjectiveRequisite(lpObj,lpInfo) == 0)
		{
			continue;
		}

		if(this->GetQuestWorldObjectiveCount(lpObj,lpInfo) < lpInfo->Value)
		{
			return 0;
		}
	}

	return 1;

	#else

	return 0;

	#endif
}

void CQuestWorldObjective::AddQuestWorldObjectiveKillCount(LPOBJ lpObj,QUEST_WORLD_OBJECTIVE_INFO* lpInfo) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	QUEST_WORLD_LIST* lpQuestWorldList = gQuestWorld.GetQuestWorldList(lpObj,lpInfo->RequireIndex,lpInfo->RequireGroup);

	if(lpQuestWorldList == 0)
	{
		return;
	}

	lpQuestWorldList->ObjectiveCount[lpInfo->Number]++;

	#endif
}

void CQuestWorldObjective::RemoveQuestWorldObjective(LPOBJ lpObj,int QuestIndex,int QuestGroup) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	for(int n=0;n < this->m_count;n++)
	{
		QUEST_WORLD_OBJECTIVE_INFO* lpInfo = this->GetInfo(n);

		if(lpInfo == 0)
		{
			continue;
		}

		if(lpInfo->RequireIndex != QuestIndex)
		{
			continue;
		}

		if(lpInfo->RequireGroup != QuestGroup)
		{
			continue;
		}

		if(this->CheckQuestWorldObjectiveRequisite(lpObj,lpInfo) == 0)
		{
			continue;
		}

		if(lpInfo->Type == QUEST_WORLD_OBJECTIVE_NONE)
		{
			continue;
		}

		if(lpInfo->Type == QUEST_WORLD_OBJECTIVE_MONSTER)
		{
			continue;
		}

		if(lpInfo->Type == QUEST_WORLD_OBJECTIVE_SKILL)
		{
			continue;
		}

		if(lpInfo->Type == QUEST_WORLD_OBJECTIVE_ITEM)
		{
			continue;
		}

		if(lpInfo->Type == QUEST_WORLD_OBJECTIVE_LEVEL)
		{
			continue;
		}

		if(lpInfo->Type == QUEST_WORLD_OBJECTIVE_SPECIAL)
		{
			continue;
		}
	}

	#endif
}

void CQuestWorldObjective::PressButton(LPOBJ lpObj,int QuestIndex,int QuestGroup) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	for(int n=0;n < this->m_count;n++)
	{
		QUEST_WORLD_OBJECTIVE_INFO* lpInfo = this->GetInfo(n);

		if(lpInfo == 0)
		{
			continue;
		}

		if(lpInfo->RequireIndex != QuestIndex)
		{
			continue;
		}

		if(lpInfo->RequireGroup != QuestGroup)
		{
			continue;
		}

		if(this->CheckQuestWorldObjectiveRequisite(lpObj,lpInfo) == 0)
		{
			continue;
		}

		if(lpInfo->Type != QUEST_WORLD_OBJECTIVE_SPECIAL)
		{
			continue;
		}

		if(lpInfo->Value > this->GetQuestWorldObjectiveCount(lpObj,lpInfo))
		{
			this->AddQuestWorldObjectiveKillCount(lpObj,lpInfo);
			return;
		}
	}

	#endif
}

void CQuestWorldObjective::MonsterKill(LPOBJ lpMonster) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	int aIndex = gObjMonsterGetTopHitDamageUser(lpMonster);

	if(OBJECT_RANGE(aIndex) == 0)
	{
		return;
	}

	LPOBJ lpObj = &gObj[aIndex];

	if(OBJECT_RANGE(lpObj->PartyNumber) != 0)
	{
		return this->MonsterKillParty(lpMonster,lpObj->PartyNumber);
	}

	for(int n=0;n < this->m_count;n++)
	{
		QUEST_WORLD_OBJECTIVE_INFO* lpInfo = this->GetInfo(n);

		if(lpInfo == 0)
		{
			continue;
		}

		if(this->CheckQuestWorldObjectiveRequisite(lpObj,lpInfo) == 0)
		{
			continue;
		}

		if(lpInfo->Type != QUEST_WORLD_OBJECTIVE_MONSTER)
		{
			continue;
		}

		if(lpInfo->Index == lpMonster->Class && lpInfo->Value > this->GetQuestWorldObjectiveCount(lpObj,lpInfo))
		{
			this->AddQuestWorldObjectiveKillCount(lpObj,lpInfo);
			gNotice.GCNoticeSend(aIndex,1,0,0,0,0,0,"%s %d/%d",lpMonster->Name,this->GetQuestWorldObjectiveCount(lpObj,lpInfo),lpInfo->Value);
			return;
		}
	}

	#endif
}

void CQuestWorldObjective::MonsterKillParty(LPOBJ lpMonster,int PartyNumber) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	for(int n=0;n < MAX_PARTY_USER;n++)
	{
		int aIndex = gParty.m_PartyInfo[PartyNumber].Index[n];

		if(OBJECT_RANGE(aIndex) == 0)
		{
			continue;
		}

		LPOBJ lpObj = &gObj[aIndex];

		for(int i=0;i < this->m_count;i++)
		{
			QUEST_WORLD_OBJECTIVE_INFO* lpInfo = this->GetInfo(i);

			if(lpInfo == 0)
			{
				continue;
			}

			if(this->CheckQuestWorldObjectiveRequisite(lpObj,lpInfo) == 0)
			{
				continue;
			}

			if(lpInfo->Type != QUEST_WORLD_OBJECTIVE_MONSTER)
			{
				continue;
			}

			if(lpInfo->Index == lpMonster->Class && lpInfo->Value > this->GetQuestWorldObjectiveCount(lpObj,lpInfo))
			{
				this->AddQuestWorldObjectiveKillCount(lpObj,lpInfo);
				gNotice.GCNoticeSend(aIndex,1,0,0,0,0,0,"%s %d/%d",lpMonster->Name,this->GetQuestWorldObjectiveCount(lpObj,lpInfo),lpInfo->Value);
				return;
			}
		}
	}

	#endif
}
