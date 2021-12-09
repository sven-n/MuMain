// DoubleGoer.cpp: implementation of the CDoubleGoer class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DoubleGoer.h"
#include "EffectManager.h"
#include "ItemBagManager.h"
#include "ItemManager.h"
#include "Map.h"
#include "MapServerManager.h"
#include "MemScript.h"
#include "Message.h"
#include "Monster.h"
#include "Notice.h"
#include "NpcTalk.h"
#include "ServerInfo.h"
#include "SkillManager.h"
#include "Util.h"

CDoubleGoer gDoubleGoer;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDoubleGoer::CDoubleGoer() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	this->m_Level = 0;
	this->m_State = DG_STATE_EMPTY;
	this->m_Map = 0;
	this->m_RemainTime = 0;
	this->m_TargetTime = 0;
	this->m_TickCount = GetTickCount();
	this->m_TimeCount = 0;
	this->m_CurMonster = 0;
	this->m_MaxMonster = 0;
	this->m_PartyNumber = -1;
	this->m_PartyCount = 0;
	this->m_HighestLevel = 0;
	this->m_CreateMonsterTime = GetTickCount();

	this->CleanUser();

	this->m_SlaughtererIndex = -1;
	this->m_MadSlaughtererIndex = -1;
	this->m_IceWalkerIndex = -1;
	this->m_SilverRewardChestIndex = -1;
	this->m_GoldenRewardChestIndex = -1;

	this->CleanMonster();

	#endif
}

CDoubleGoer::~CDoubleGoer() // OK
{

}

void CDoubleGoer::Load(char* path) // OK
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

					this->m_NotifyTime = lpMemScript->GetNumber();

					this->m_EventTime = lpMemScript->GetAsNumber();

					this->m_CloseTime = lpMemScript->GetAsNumber();
				}
				else if(section == 1)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					this->m_MonsterAIUnitNumber[0] = lpMemScript->GetNumber();

					this->m_MonsterAIUnitNumber[1] = lpMemScript->GetAsNumber();

					this->m_MonsterAIUnitNumber[2] = lpMemScript->GetAsNumber();

					this->m_MonsterAIUnitNumber[3] = lpMemScript->GetAsNumber();
				}
				else if(section == 2)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					DOUBLE_GOER_MONSTER_DIFFICULT_INFO info;

					memset(&info,0,sizeof(info));

					info.Index = lpMemScript->GetNumber();

					info.Level[0] = lpMemScript->GetAsFloatNumber();

					info.Level[1] = lpMemScript->GetAsFloatNumber();

					info.Level[2] = lpMemScript->GetAsFloatNumber();

					info.Level[3] = lpMemScript->GetAsFloatNumber();

					info.Level[4] = lpMemScript->GetAsFloatNumber();

					info.Life[0] = lpMemScript->GetAsFloatNumber();

					info.Life[1] = lpMemScript->GetAsFloatNumber();

					info.Life[2] = lpMemScript->GetAsFloatNumber();

					info.Life[3] = lpMemScript->GetAsFloatNumber();

					info.Life[4] = lpMemScript->GetAsFloatNumber();

					info.Damage[0] = lpMemScript->GetAsFloatNumber();

					info.Damage[1] = lpMemScript->GetAsFloatNumber();

					info.Damage[2] = lpMemScript->GetAsFloatNumber();

					info.Damage[3] = lpMemScript->GetAsFloatNumber();

					info.Damage[4] = lpMemScript->GetAsFloatNumber();

					info.Defense[0] = lpMemScript->GetAsFloatNumber();

					info.Defense[1] = lpMemScript->GetAsFloatNumber();

					info.Defense[2] = lpMemScript->GetAsFloatNumber();

					info.Defense[3] = lpMemScript->GetAsFloatNumber();

					info.Defense[4] = lpMemScript->GetAsFloatNumber();

					if(info.Index >= 0 && info.Index < MAX_DG_MONSTER_DIFFICULT)
					{
						this->m_MonsterDifficultInfo[info.Index] = info;
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

	delete lpMemScript;

	#endif
}

void CDoubleGoer::MainProc() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	DWORD elapsed = GetTickCount()-this->m_TickCount;

	if(elapsed < 1000)
	{
		return;
	}

	this->m_TickCount = GetTickCount();

	this->m_RemainTime = (int)(difftime(this->m_TargetTime,time(0))*1000);

	switch(this->m_State)
	{
		case DG_STATE_EMPTY:
			this->ProcState_EMPTY();
			break;
		case DG_STATE_STAND:
			this->ProcState_STAND();
			break;
		case DG_STATE_START:
			this->ProcState_START();
			break;
		case DG_STATE_CLEAN:
			this->ProcState_CLEAN();
			break;
	}

	#endif
}

void CDoubleGoer::ProcState_EMPTY() // OK
{

}

void CDoubleGoer::ProcState_STAND() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	this->CheckUser();

	if(this->GetUserCount() == 0)
	{
		this->SetState(DG_STATE_EMPTY);
		return;
	}

	if(this->m_RemainTime > 0 && this->m_RemainTime <= 30000 && this->m_TimeCount == 0)
	{
		PMSG_TIME_COUNT_SEND pMsg;

		pMsg.header.set(0x92,sizeof(pMsg));

		pMsg.type = 17;

		this->DataSendToAll((BYTE*)&pMsg,pMsg.header.size);

		this->m_TimeCount = 1;
	}

	if(this->m_RemainTime <= 0)
	{
		this->SetState(DG_STATE_START);
	}

	#endif
}

void CDoubleGoer::ProcState_START() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	this->CheckUser();

	if(this->GetUserCount() == 0)
	{
		this->SetState(DG_STATE_EMPTY);
		return;
	}

	switch(this->m_Stage)
	{
		case 0:
			this->SetStage0();
			break;
		case 1:
			this->SetStage1();
			break;
		case 2:
			this->SetStage2();
			break;
		case 3:
			this->SetStage3();
			break;
	}

	if((GetTickCount()-this->m_CreateMonsterTime) > 5000)
	{
		this->m_CreateMonsterTime = GetTickCount();

		this->SetMonster(533+(GetLargeRand()%7));
	}

	this->GCDoubleGoerMonsterPositionSend();

	this->GCDoubleGoerInterfaceSend();

	this->GCDoubleGoerBossPositionSend();

	this->CheckMonsterPosition();

	if(this->m_CurMonster >= this->m_MaxMonster)
	{
		this->SetState(DG_STATE_CLEAN);
		return;
	}

	if(this->m_RemainTime > 0 && this->m_RemainTime <= 30000 && this->m_TimeCount == 0 && OBJECT_RANGE(this->m_IceWalkerIndex) != 0)
	{
		PMSG_TIME_COUNT_SEND pMsg;

		pMsg.header.set(0x92,sizeof(pMsg));

		pMsg.type = 18;

		this->DataSendToAll((BYTE*)&pMsg,pMsg.header.size);

		this->m_TimeCount = 1;
	}

	if(this->m_RemainTime <= 0)
	{
		this->SetState(DG_STATE_CLEAN);
	}

	#endif
}

void CDoubleGoer::ProcState_CLEAN() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	this->CheckUser();

	if(this->m_RemainTime > 0 && this->m_RemainTime <= 30000 && this->m_TimeCount == 0)
	{
		PMSG_TIME_COUNT_SEND pMsg;

		pMsg.header.set(0x92,sizeof(pMsg));

		pMsg.type = 19;

		this->DataSendToAll((BYTE*)&pMsg,pMsg.header.size);

		this->m_TimeCount = 1;
	}

	if(this->m_RemainTime <= 0)
	{
		this->SetState(DG_STATE_EMPTY);
	}

	#endif
}

void CDoubleGoer::SetState(int state) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	this->m_State = state;

	switch(this->m_State)
	{
		case DG_STATE_EMPTY:
			this->SetState_EMPTY();
			break;
		case DG_STATE_STAND:
			this->SetState_STAND();
			break;
		case DG_STATE_START:
			this->SetState_START();
			break;
		case DG_STATE_CLEAN:
			this->SetState_CLEAN();
			break;
	}

	#endif
}

void CDoubleGoer::SetState_EMPTY() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	this->m_RemainTime = 0;
	this->m_TargetTime = 0;
	this->m_TickCount = GetTickCount();
	this->m_TimeCount = 0;
	this->m_CurMonster = 0;
	this->m_MaxMonster = 0;
	this->m_PartyCount = 0;
	this->m_HighestLevel = 0;
	this->m_CreateMonsterTime = GetTickCount();

	if(OBJECT_RANGE(this->m_PartyNumber) != 0)
	{
		gParty.Destroy(this->m_PartyNumber);
		this->m_PartyNumber = -1;
	}

	this->GCDoubleGoerStateSend(0,-1);

	this->ClearUser();

	if(OBJECT_RANGE(this->m_SlaughtererIndex) != 0)
	{
		gObjDel(this->m_SlaughtererIndex);
		this->m_SlaughtererIndex = -1;
	}

	if(OBJECT_RANGE(this->m_MadSlaughtererIndex) != 0)
	{
		gObjDel(this->m_MadSlaughtererIndex);
		this->m_MadSlaughtererIndex = -1;
	}

	if(OBJECT_RANGE(this->m_IceWalkerIndex) != 0)
	{
		gObjDel(this->m_IceWalkerIndex);
		this->m_IceWalkerIndex = -1;
	}

	if(OBJECT_RANGE(this->m_SilverRewardChestIndex) != 0)
	{
		gObjDel(this->m_SilverRewardChestIndex);
		this->m_SilverRewardChestIndex = -1;
	}

	if(OBJECT_RANGE(this->m_GoldenRewardChestIndex) != 0)
	{
		gObjDel(this->m_GoldenRewardChestIndex);
		this->m_GoldenRewardChestIndex = -1;
	}

	this->ClearMonster();

	this->DelEntranceZone();

	#endif
}

void CDoubleGoer::SetState_STAND() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	this->m_TimeCount = 0;
	this->m_CurMonster = 0;
	this->m_MaxMonster = 0;
	this->m_CreateMonsterTime = GetTickCount();

	this->SetEntranceZone();

	this->GCDoubleGoerStateSend(1,-1);

	this->m_RemainTime = this->m_NotifyTime*60000;

	this->m_TargetTime = (int)(time(0)+(this->m_RemainTime/1000));

	#endif
}

void CDoubleGoer::SetState_START() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	this->m_TimeCount = 0;
	this->m_CurMonster = 0;
	this->m_MaxMonster = 3;
	this->m_CreateMonsterTime = GetTickCount();

	this->DelEntranceZone();

	this->GCDoubleGoerStateSend(2,-1);

	this->GCDoubleGoerMonsterInfoSend();

	this->m_RemainTime = this->m_EventTime*60000;

	this->m_TargetTime = (int)(time(0)+(this->m_RemainTime/1000));

	#endif
}

void CDoubleGoer::SetState_CLEAN() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	this->m_TimeCount = 0;
	this->m_CreateMonsterTime = GetTickCount();

	for(int n=0;n < MAX_DG_USER;n++)
	{
		if(OBJECT_RANGE(this->m_User[n].Index) != 0)
		{
			this->GCDoubleGoerResultSend(this->m_User[n].Index,((this->m_CurMonster<this->m_MaxMonster&&OBJECT_RANGE(this->m_IceWalkerIndex)==0)?0:1));
		}
	}

	if(this->m_CurMonster < this->m_MaxMonster && OBJECT_RANGE(this->m_IceWalkerIndex) == 0)
	{
		for(int n=0;n < MAX_DG_USER;n++)
		{
			if(OBJECT_RANGE(this->m_User[n].Index) != 0)
			{
				this->SetGoldenRewardChest(gObj[this->m_User[n].Index].X,gObj[this->m_User[n].Index].Y);
				break;
			}
		}
	}

	if(OBJECT_RANGE(this->m_SlaughtererIndex) != 0)
	{
		gObjDel(this->m_SlaughtererIndex);
		this->m_SlaughtererIndex = -1;
	}

	if(OBJECT_RANGE(this->m_MadSlaughtererIndex) != 0)
	{
		gObjDel(this->m_MadSlaughtererIndex);
		this->m_MadSlaughtererIndex = -1;
	}

	if(OBJECT_RANGE(this->m_IceWalkerIndex) != 0)
	{
		gObjDel(this->m_IceWalkerIndex);
		this->m_IceWalkerIndex = -1;
	}

	this->ClearMonster();

	this->GCDoubleGoerStateSend(3,-1);

	this->m_RemainTime = this->m_CloseTime*60000;

	this->m_TargetTime = (int)(time(0)+(this->m_RemainTime/1000));

	#endif
}

void CDoubleGoer::SetStage0() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	if(((this->m_RemainTime*100)/(this->m_EventTime*60000)) > 75)
	{
		return;
	}

	this->m_Stage = 1;

	this->SetSlaughterer();

	#endif
}

void CDoubleGoer::SetStage1() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	if(((this->m_RemainTime*100)/(this->m_EventTime*60000)) > 50)
	{
		return;
	}

	this->m_Stage = 2;

	this->SetMadSlaughterer();

	#endif
}

void CDoubleGoer::SetStage2() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	if(((this->m_RemainTime*100)/(this->m_EventTime*60000)) > 25)
	{
		return;
	}

	this->m_Stage = 3;

	this->SetIceWalker();

	this->NoticeSendToAll(0,gMessage.GetMessage(304));

	#endif
}

void CDoubleGoer::SetStage3() // OK
{

}

int CDoubleGoer::GetState() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	return this->m_State;

	#else

	return 0;

	#endif
}

bool CDoubleGoer::AddUser(int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	if(OBJECT_RANGE(aIndex) == 0)
	{
		return 0;
	}

	if(this->GetUser(aIndex) != 0)
	{
		return 0;
	}

	for(int n=0;n < MAX_DG_USER;n++)
	{
		if(OBJECT_RANGE(this->m_User[n].Index) != 0)
		{
			continue;
		}

		this->m_User[n].Index = aIndex;
		return 1;
	}

	return 0;

	#else

	return 0;

	#endif
}

bool CDoubleGoer::DelUser(int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	if(OBJECT_RANGE(aIndex) == 0)
	{
		return 0;
	}

	DOUBLE_GOER_USER* lpUser = this->GetUser(aIndex);

	if(lpUser == 0)
	{
		return 0;
	}

	if(OBJECT_RANGE(this->m_PartyNumber) != 0)
	{
		gParty.DelMember(this->m_PartyNumber,aIndex);
		this->m_PartyNumber = ((gParty.IsParty(this->m_PartyNumber)==0)?-1:this->m_PartyNumber);
	}

	lpUser->Index = -1;
	return 1;

	#else

	return 0;

	#endif
}

DOUBLE_GOER_USER* CDoubleGoer::GetUser(int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	if(OBJECT_RANGE(aIndex) == 0)
	{
		return 0;
	}

	for(int n=0;n < MAX_DG_USER;n++)
	{
		if(this->m_User[n].Index == aIndex)
		{
			return &this->m_User[n];
		}
	}

	return 0;

	#else

	return 0;

	#endif
}

void CDoubleGoer::CleanUser() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	for(int n=0;n < MAX_DG_USER;n++)
	{
		this->m_User[n].Index = -1;
	}

	#endif
}

void CDoubleGoer::ClearUser() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	for(int n=0;n < MAX_DG_USER;n++)
	{
		if(OBJECT_RANGE(this->m_User[n].Index) == 0)
		{
			continue;
		}

		gObjMoveGate(this->m_User[n].Index,267);

		this->m_User[n].Index = -1;
	}

	#endif
}

void CDoubleGoer::CheckUser() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	for(int n=0;n < MAX_DG_USER;n++)
	{
		if(OBJECT_RANGE(this->m_User[n].Index) == 0)
		{
			continue;
		}

		if(gObjIsConnected(this->m_User[n].Index) == 0)
		{
			this->DelUser(this->m_User[n].Index);
			continue;
		}

		if(gObj[this->m_User[n].Index].Map != this->m_Map)
		{
			this->DelUser(this->m_User[n].Index);
			continue;
		}
	}

	#endif
}

int CDoubleGoer::GetUserCount() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	int count = 0;

	for(int n=0;n < MAX_DG_USER;n++)
	{
		if(OBJECT_RANGE(this->m_User[n].Index) != 0)
		{
			count++;
		}
	}

	return count;

	#else

	return 0;

	#endif
}

bool CDoubleGoer::AddMonster(int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	if(OBJECT_RANGE(aIndex) == 0)
	{
		return 0;
	}

	if(this->GetMonster(aIndex) != 0)
	{
		return 0;
	}

	for(int n=0;n < MAX_DG_MONSTER;n++)
	{
		if(OBJECT_RANGE(this->m_MonsterIndex[n]) != 0)
		{
			continue;
		}

		this->m_MonsterIndex[n] = aIndex;
		return 1;
	}

	return 0;

	#else

	return 0;

	#endif
}

bool CDoubleGoer::DelMonster(int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	if(OBJECT_RANGE(aIndex) == 0)
	{
		return 0;
	}

	int* index = this->GetMonster(aIndex);

	if(index == 0)
	{
		return 0;
	}

	(*index) = -1;
	return 1;

	#else

	return 0;

	#endif
}

int* CDoubleGoer::GetMonster(int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	if(OBJECT_RANGE(aIndex) == 0)
	{
		return 0;
	}

	for(int n=0;n < MAX_DG_MONSTER;n++)
	{
		if(this->m_MonsterIndex[n] == aIndex)
		{
			return &this->m_MonsterIndex[n];
		}
	}

	return 0;

	#else

	return 0;

	#endif
}

void CDoubleGoer::CleanMonster() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	for(int n=0;n < MAX_DG_MONSTER;n++)
	{
		this->m_MonsterIndex[n] = -1;
	}

	#endif
}

void CDoubleGoer::ClearMonster() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	for(int n=0;n < MAX_DG_MONSTER;n++)
	{
		if(OBJECT_RANGE(this->m_MonsterIndex[n]) != 0)
		{
			gObjDel(this->m_MonsterIndex[n]);
			this->m_MonsterIndex[n] = -1;
		}
	}

	#endif
}

int CDoubleGoer::GetMonsterCount() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	int count = 0;

	for(int n=0;n < MAX_DG_MONSTER;n++)
	{
		if(OBJECT_RANGE(this->m_MonsterIndex[n]) != 0)
		{
			count++;
		}
	}

	return count;

	#else

	return 0;

	#endif
}

void CDoubleGoer::SetSlaughterer() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	if(OBJECT_RANGE(this->m_SlaughtererIndex) != 0)
	{
		return;
	}

	int index = gObjAddMonster(this->m_Map);

	if(OBJECT_RANGE(index) == 0)
	{
		return;
	}

	LPOBJ lpObj = &gObj[index];

	int px = gDoubleGoerRespawnZone[this->m_Level][0];
	int py = gDoubleGoerRespawnZone[this->m_Level][1];

	if(gObjGetRandomFreeLocation(this->m_Map,&px,&py,2,2,10) == 0)
	{
		px = gDoubleGoerRespawnZone[this->m_Level][0];
		py = gDoubleGoerRespawnZone[this->m_Level][1];
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
	lpObj->Dir = 1;
	lpObj->Map = this->m_Map;

	if(gObjSetMonster(index,530) == 0)
	{
		gObjDel(index);
		return;
	}

	lpObj->Level = (int)this->GetMonsterLevel((float)lpObj->Level);
	lpObj->Life = (float)this->GetMonsterLife((float)lpObj->Life);
	lpObj->MaxLife = (float)this->GetMonsterLife((float)lpObj->MaxLife);
	lpObj->ScriptMaxLife = (float)this->GetMonsterLife((float)lpObj->ScriptMaxLife);
	lpObj->PhysiDamageMin = (int)this->GetMonsterDamageMin((float)lpObj->PhysiDamageMin);
	lpObj->PhysiDamageMax = (int)this->GetMonsterDamageMax((float)lpObj->PhysiDamageMax);
	lpObj->Defense = (int)this->GetMonsterDefense((float)lpObj->Defense);
	lpObj->BasicAI = this->m_MonsterAIUnitNumber[this->m_Level];
	lpObj->CurrentAI = this->m_MonsterAIUnitNumber[this->m_Level];
	lpObj->CurrentAIState = 0;
	lpObj->LastAIRunTime = 0;
	lpObj->GroupNumber = 0;
	lpObj->SubGroupNumber = 0;
	lpObj->GroupMemberGuid = -1;
	lpObj->RegenType = 0;

	this->m_SlaughtererIndex = index;

	#endif
}

void CDoubleGoer::SetMadSlaughterer() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	if(OBJECT_RANGE(this->m_MadSlaughtererIndex) != 0)
	{
		return;
	}

	int index = gObjAddMonster(this->m_Map);

	if(OBJECT_RANGE(index) == 0)
	{
		return;
	}

	LPOBJ lpObj = &gObj[index];

	int px = gDoubleGoerRespawnZone[this->m_Level][0];
	int py = gDoubleGoerRespawnZone[this->m_Level][1];

	if(gObjGetRandomFreeLocation(this->m_Map,&px,&py,2,2,10) == 0)
	{
		px = gDoubleGoerRespawnZone[this->m_Level][0];
		py = gDoubleGoerRespawnZone[this->m_Level][1];
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
	lpObj->Dir = 1;
	lpObj->Map = this->m_Map;

	if(gObjSetMonster(index,529) == 0)
	{
		gObjDel(index);
		return;
	}

	lpObj->Level = (int)this->GetMonsterLevel((float)lpObj->Level);
	lpObj->Life = (float)this->GetMonsterLife((float)lpObj->Life);
	lpObj->MaxLife = (float)this->GetMonsterLife((float)lpObj->MaxLife);
	lpObj->ScriptMaxLife = (float)this->GetMonsterLife((float)lpObj->ScriptMaxLife);
	lpObj->PhysiDamageMin = (int)this->GetMonsterDamageMin((float)lpObj->PhysiDamageMin);
	lpObj->PhysiDamageMax = (int)this->GetMonsterDamageMax((float)lpObj->PhysiDamageMax);
	lpObj->Defense = (int)this->GetMonsterDefense((float)lpObj->Defense);
	lpObj->BasicAI = this->m_MonsterAIUnitNumber[this->m_Level];
	lpObj->CurrentAI = this->m_MonsterAIUnitNumber[this->m_Level];
	lpObj->CurrentAIState = 0;
	lpObj->LastAIRunTime = 0;
	lpObj->GroupNumber = 0;
	lpObj->SubGroupNumber = 0;
	lpObj->GroupMemberGuid = -1;
	lpObj->RegenType = 0;

	this->m_MadSlaughtererIndex = index;

	#endif
}

void CDoubleGoer::SetIceWalker() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	if(OBJECT_RANGE(this->m_IceWalkerIndex) != 0)
	{
		return;
	}

	int index = gObjAddMonster(this->m_Map);

	if(OBJECT_RANGE(index) == 0)
	{
		return;
	}

	LPOBJ lpObj = &gObj[index];

	int px = gDoubleGoerRespawnZone[this->m_Level][0];
	int py = gDoubleGoerRespawnZone[this->m_Level][1];

	if(gObjGetRandomFreeLocation(this->m_Map,&px,&py,2,2,10) == 0)
	{
		px = gDoubleGoerRespawnZone[this->m_Level][0];
		py = gDoubleGoerRespawnZone[this->m_Level][1];
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
	lpObj->Dir = 1;
	lpObj->Map = this->m_Map;

	if(gObjSetMonster(index,531) == 0)
	{
		gObjDel(index);
		return;
	}

	lpObj->Level = (int)this->GetMonsterLevel((float)lpObj->Level);
	lpObj->Life = (float)this->GetMonsterLife((float)lpObj->Life);
	lpObj->MaxLife = (float)this->GetMonsterLife((float)lpObj->MaxLife);
	lpObj->ScriptMaxLife = (float)this->GetMonsterLife((float)lpObj->ScriptMaxLife);
	lpObj->PhysiDamageMin = (int)this->GetMonsterDamageMin((float)lpObj->PhysiDamageMin);
	lpObj->PhysiDamageMax = (int)this->GetMonsterDamageMax((float)lpObj->PhysiDamageMax);
	lpObj->Defense = (int)this->GetMonsterDefense((float)lpObj->Defense);
	lpObj->BasicAI = this->m_MonsterAIUnitNumber[this->m_Level];
	lpObj->CurrentAI = this->m_MonsterAIUnitNumber[this->m_Level];
	lpObj->CurrentAIState = 0;
	lpObj->LastAIRunTime = 0;
	lpObj->GroupNumber = 0;
	lpObj->SubGroupNumber = 0;
	lpObj->GroupMemberGuid = -1;
	lpObj->RegenType = 0;

	this->m_IceWalkerIndex = index;

	#endif
}

void CDoubleGoer::SetSilverRewardChest(int px,int py) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	if(OBJECT_RANGE(this->m_SilverRewardChestIndex) != 0)
	{
		return;
	}

	int index = gObjAddMonster(this->m_Map);

	if(OBJECT_RANGE(index) == 0)
	{
		return;
	}

	LPOBJ lpObj = &gObj[index];

	lpObj->PosNum = -1;
	lpObj->X = px;
	lpObj->Y = py;
	lpObj->TX = px;
	lpObj->TY = py;
	lpObj->OldX = px;
	lpObj->OldY = py;
	lpObj->StartX = px;
	lpObj->StartY = py;
	lpObj->Dir = 1;
	lpObj->Map = this->m_Map;

	if(gObjSetMonster(index,541) == 0)
	{
		gObjDel(index);
		return;
	}

	this->m_SilverRewardChestIndex = index;

	#endif
}

void CDoubleGoer::SetGoldenRewardChest(int px,int py) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	if(OBJECT_RANGE(this->m_GoldenRewardChestIndex) != 0)
	{
		return;
	}

	int index = gObjAddMonster(this->m_Map);

	if(OBJECT_RANGE(index) == 0)
	{
		return;
	}

	LPOBJ lpObj = &gObj[index];

	lpObj->PosNum = -1;
	lpObj->X = px;
	lpObj->Y = py;
	lpObj->TX = px;
	lpObj->TY = py;
	lpObj->OldX = px;
	lpObj->OldY = py;
	lpObj->StartX = px;
	lpObj->StartY = py;
	lpObj->Dir = 1;
	lpObj->Map = this->m_Map;

	if(gObjSetMonster(index,542) == 0)
	{
		gObjDel(index);
		return;
	}

	this->m_GoldenRewardChestIndex = index;

	#endif
}

void CDoubleGoer::SetMonster(int MonsterClass) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	int index = gObjAddMonster(this->m_Map);

	if(OBJECT_RANGE(index) == 0)
	{
		return;
	}

	LPOBJ lpObj = &gObj[index];

	int px = gDoubleGoerRespawnZone[this->m_Level][0];
	int py = gDoubleGoerRespawnZone[this->m_Level][1];

	if(gObjGetRandomFreeLocation(this->m_Map,&px,&py,2,2,10) == 0)
	{
		px = gDoubleGoerRespawnZone[this->m_Level][0];
		py = gDoubleGoerRespawnZone[this->m_Level][1];
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
	lpObj->Dir = 1;
	lpObj->Map = this->m_Map;

	if(gObjSetMonster(index,MonsterClass) == 0)
	{
		gObjDel(index);
		return;
	}

	lpObj->Level = (int)this->GetMonsterLevel((float)lpObj->Level);
	lpObj->Life = (float)this->GetMonsterLife((float)lpObj->Life);
	lpObj->MaxLife = (float)this->GetMonsterLife((float)lpObj->MaxLife);
	lpObj->ScriptMaxLife = (float)this->GetMonsterLife((float)lpObj->ScriptMaxLife);
	lpObj->PhysiDamageMin = (int)this->GetMonsterDamageMin((float)lpObj->PhysiDamageMin);
	lpObj->PhysiDamageMax = (int)this->GetMonsterDamageMax((float)lpObj->PhysiDamageMax);
	lpObj->Defense = (int)this->GetMonsterDefense((float)lpObj->Defense);
	lpObj->BasicAI = this->m_MonsterAIUnitNumber[this->m_Level];
	lpObj->CurrentAI = this->m_MonsterAIUnitNumber[this->m_Level];
	lpObj->CurrentAIState = 0;
	lpObj->LastAIRunTime = 0;
	lpObj->GroupNumber = 0;
	lpObj->SubGroupNumber = 0;
	lpObj->GroupMemberGuid = -1;
	lpObj->RegenType = 0;

	if(this->AddMonster(index) == 0)
	{
		gObjDel(index);
		return;
	}

	#endif
}

void CDoubleGoer::NpcSartiganTheAngel(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	if(gServerInfo.m_DoubleGoerEvent == 0)
	{
		return;
	}

	if(lpObj->Level < 10)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(305));
		return;
	}

	PMSG_NPC_TALK_SEND pMsg;

	pMsg.header.set(0x30,sizeof(pMsg));

	pMsg.result = 35;

	switch(this->m_State)
	{
		case DG_STATE_EMPTY:
			pMsg.level[0] = 0;
			break;
		case DG_STATE_STAND:
			pMsg.level[0] = (((this->m_RemainTime%60000)==0)?(this->m_RemainTime/60000):((this->m_RemainTime/60000)+1))+this->m_EventTime+this->m_CloseTime;
			break;
		case DG_STATE_START:
			pMsg.level[0] = (((this->m_RemainTime%60000)==0)?(this->m_RemainTime/60000):((this->m_RemainTime/60000)+1))+this->m_CloseTime;
			break;
		case DG_STATE_CLEAN:
			pMsg.level[0] = (((this->m_RemainTime%60000)==0)?(this->m_RemainTime/60000):((this->m_RemainTime/60000)+1));
			break;
	}

	pMsg.level[1] = pMsg.level[0]%60;

	pMsg.level[0] = pMsg.level[0]/60;

	DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CDoubleGoer::NpcSilverRewardChest(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	if(gServerInfo.m_DoubleGoerEvent == 0)
	{
		return;
	}

	if(this->m_State != DG_STATE_START && this->m_State != DG_STATE_CLEAN)
	{
		return;
	}

	if(this->m_Map != lpNpc->Map)
	{
		return;
	}

	if(this->m_SilverRewardChestIndex != lpNpc->Index)
	{
		return;
	}

	switch(((this->m_HighestLevel-1)/100))
	{
		case 0:
			gItemBagManager.DropItemBySpecialValue(ITEM_BAG_DOUBLE_GOER_SILVER_CHEST1,lpObj,lpNpc->Map,lpNpc->X,lpNpc->Y);
			break;
		case 1:
			gItemBagManager.DropItemBySpecialValue(ITEM_BAG_DOUBLE_GOER_SILVER_CHEST2,lpObj,lpNpc->Map,lpNpc->X,lpNpc->Y);
			break;
		case 2:
			gItemBagManager.DropItemBySpecialValue(ITEM_BAG_DOUBLE_GOER_SILVER_CHEST3,lpObj,lpNpc->Map,lpNpc->X,lpNpc->Y);
			break;
		case 3:
			gItemBagManager.DropItemBySpecialValue(ITEM_BAG_DOUBLE_GOER_SILVER_CHEST4,lpObj,lpNpc->Map,lpNpc->X,lpNpc->Y);
			break;
		default:
			gItemBagManager.DropItemBySpecialValue(ITEM_BAG_DOUBLE_GOER_SILVER_CHEST5,lpObj,lpNpc->Map,lpNpc->X,lpNpc->Y);
			break;
	}

	GCUserDieSend(&gObj[this->m_SilverRewardChestIndex],this->m_SilverRewardChestIndex,0,0);

	gObjDel(this->m_SilverRewardChestIndex);

	this->m_SilverRewardChestIndex = -1;

	#endif
}

void CDoubleGoer::NpcGoldenRewardChest(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	if(gServerInfo.m_DoubleGoerEvent == 0)
	{
		return;
	}

	if(this->m_State != DG_STATE_START && this->m_State != DG_STATE_CLEAN)
	{
		return;
	}

	if(this->m_Map != lpNpc->Map)
	{
		return;
	}

	if(this->m_GoldenRewardChestIndex != lpNpc->Index)
	{
		return;
	}

	switch(((this->m_HighestLevel-1)/100))
	{
		case 0:
			gItemBagManager.DropItemBySpecialValue(ITEM_BAG_DOUBLE_GOER_GOLDEN_CHEST1,lpObj,lpNpc->Map,lpNpc->X,lpNpc->Y);
			break;
		case 1:
			gItemBagManager.DropItemBySpecialValue(ITEM_BAG_DOUBLE_GOER_GOLDEN_CHEST2,lpObj,lpNpc->Map,lpNpc->X,lpNpc->Y);
			break;
		case 2:
			gItemBagManager.DropItemBySpecialValue(ITEM_BAG_DOUBLE_GOER_GOLDEN_CHEST3,lpObj,lpNpc->Map,lpNpc->X,lpNpc->Y);
			break;
		case 3:
			gItemBagManager.DropItemBySpecialValue(ITEM_BAG_DOUBLE_GOER_GOLDEN_CHEST4,lpObj,lpNpc->Map,lpNpc->X,lpNpc->Y);
			break;
		default:
			gItemBagManager.DropItemBySpecialValue(ITEM_BAG_DOUBLE_GOER_GOLDEN_CHEST5,lpObj,lpNpc->Map,lpNpc->X,lpNpc->Y);
			break;
	}

	GCUserDieSend(&gObj[this->m_GoldenRewardChestIndex],this->m_GoldenRewardChestIndex,0,0);

	gObjDel(this->m_GoldenRewardChestIndex);

	this->m_GoldenRewardChestIndex = -1;

	#endif
}

void CDoubleGoer::CheckMonsterPosition() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	if(OBJECT_RANGE(this->m_SlaughtererIndex) != 0)
	{
		if((BYTE)((gDoubleGoerFinalPoint[this->m_Level]-gObj[this->m_SlaughtererIndex].Y)/gDoubleGoerPointValue[this->m_Level]) == 22)
		{
			this->m_CurMonster++;

			this->m_CurMonster = ((this->m_CurMonster>this->m_MaxMonster)?this->m_MaxMonster:this->m_CurMonster);

			this->GCDoubleGoerMonsterInfoSend();

			gEffectManager.AddEffect(&gObj[this->m_SlaughtererIndex],0,EFFECT_DOUBLE_GOER_DELETE,1,0,0,0,0);

			gObjDel(this->m_SlaughtererIndex);

			this->m_SlaughtererIndex = -1;
		}
	}

	if(OBJECT_RANGE(this->m_MadSlaughtererIndex) != 0)
	{
		if((BYTE)((gDoubleGoerFinalPoint[this->m_Level]-gObj[this->m_MadSlaughtererIndex].Y)/gDoubleGoerPointValue[this->m_Level]) == 22)
		{
			this->m_CurMonster++;

			this->m_CurMonster = ((this->m_CurMonster>this->m_MaxMonster)?this->m_MaxMonster:this->m_CurMonster);

			this->GCDoubleGoerMonsterInfoSend();

			gEffectManager.AddEffect(&gObj[this->m_MadSlaughtererIndex],0,EFFECT_DOUBLE_GOER_DELETE,1,0,0,0,0);

			gObjDel(this->m_MadSlaughtererIndex);

			this->m_MadSlaughtererIndex = -1;
		}
	}

	if(OBJECT_RANGE(this->m_IceWalkerIndex) != 0)
	{
		if((BYTE)((gDoubleGoerFinalPoint[this->m_Level]-gObj[this->m_IceWalkerIndex].Y)/gDoubleGoerPointValue[this->m_Level]) == 22)
		{
			this->m_CurMonster++;

			this->m_CurMonster = ((this->m_CurMonster>this->m_MaxMonster)?this->m_MaxMonster:this->m_CurMonster);

			this->GCDoubleGoerMonsterInfoSend();

			gEffectManager.AddEffect(&gObj[this->m_IceWalkerIndex],0,EFFECT_DOUBLE_GOER_DELETE,1,0,0,0,0);

			gObjDel(this->m_IceWalkerIndex);

			this->m_IceWalkerIndex = -1;
		}
	}

	for(int n=0;n < MAX_DG_MONSTER;n++)
	{
		if(OBJECT_RANGE(this->m_MonsterIndex[n]) == 0)
		{
			continue;
		}

		if((BYTE)((gDoubleGoerFinalPoint[this->m_Level]-gObj[this->m_MonsterIndex[n]].Y)/gDoubleGoerPointValue[this->m_Level]) == 22)
		{
			this->m_CurMonster++;

			this->m_CurMonster = ((this->m_CurMonster>this->m_MaxMonster)?this->m_MaxMonster:this->m_CurMonster);

			this->GCDoubleGoerMonsterInfoSend();

			gEffectManager.AddEffect(&gObj[this->m_MonsterIndex[n]],0,EFFECT_DOUBLE_GOER_DELETE,1,0,0,0,0);

			gObjDel(this->m_MonsterIndex[n]);

			this->DelMonster(this->m_MonsterIndex[n]);
		}
	}

	#endif
}

float CDoubleGoer::GetMonsterLevel(float value) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	float MonsterLevel = value*this->m_MonsterDifficultInfo[((this->m_HighestLevel-1)/10)].Level[(this->m_PartyCount-1)];

	return MonsterLevel;

	#else

	return 0;

	#endif
}

float CDoubleGoer::GetMonsterLife(float value) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	float MonsterLife = value*this->m_MonsterDifficultInfo[((this->m_HighestLevel-1)/10)].Life[(this->m_PartyCount-1)];

	return ((MonsterLife*gServerInfo.m_DoubleGoerDifficultRate)/100);

	#else

	return 0;

	#endif
}

float CDoubleGoer::GetMonsterDamageMin(float value) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	float MonsterDamageMin = value*this->m_MonsterDifficultInfo[((this->m_HighestLevel-1)/10)].Damage[(this->m_PartyCount-1)];

	return ((MonsterDamageMin*gServerInfo.m_DoubleGoerDifficultRate)/100);

	#else

	return 0;

	#endif
}

float CDoubleGoer::GetMonsterDamageMax(float value) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	float MonsterDamageMax = value*this->m_MonsterDifficultInfo[((this->m_HighestLevel-1)/10)].Damage[(this->m_PartyCount-1)];

	return ((MonsterDamageMax*gServerInfo.m_DoubleGoerDifficultRate)/100);

	#else

	return 0;

	#endif
}

float CDoubleGoer::GetMonsterDefense(float value) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	float MonsterDefense = value*this->m_MonsterDifficultInfo[((this->m_HighestLevel-1)/10)].Defense[(this->m_PartyCount-1)];

	return ((MonsterDefense*gServerInfo.m_DoubleGoerDifficultRate)/100);

	#else

	return 0;

	#endif
}

void CDoubleGoer::SetEntranceZone() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	for(int x=gDoubleGoerEntranceZone[this->m_Level][0];x <= gDoubleGoerEntranceZone[this->m_Level][2];x++)
	{
		for(int y=gDoubleGoerEntranceZone[this->m_Level][1];y <= gDoubleGoerEntranceZone[this->m_Level][3];y++)
		{
			gMap[this->m_Map].SetAttr(x,y,4);
		}
	}

	for(int n=0;n < MAX_DG_USER;n++)
	{
		if(OBJECT_RANGE(this->m_User[n].Index) != 0)
		{
			GCMapAttrSend(this->m_User[n].Index,0,4,0,1,(PMSG_MAP_ATTR*)gDoubleGoerEntranceZone[this->m_Level]);
		}
	}

	#endif
}

void CDoubleGoer::DelEntranceZone() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	for(int x=gDoubleGoerEntranceZone[this->m_Level][0];x <= gDoubleGoerEntranceZone[this->m_Level][2];x++)
	{
		for(int y=gDoubleGoerEntranceZone[this->m_Level][1];y <= gDoubleGoerEntranceZone[this->m_Level][3];y++)
		{
			gMap[this->m_Map].DelAttr(x,y,4);
		}
	}

	for(int n=0;n < MAX_DG_USER;n++)
	{
		if(OBJECT_RANGE(this->m_User[n].Index) != 0)
		{
			GCMapAttrSend(this->m_User[n].Index,0,4,1,1,(PMSG_MAP_ATTR*)gDoubleGoerEntranceZone[this->m_Level]);
		}
	}

	#endif
}

void CDoubleGoer::UserDieProc(LPOBJ lpObj,LPOBJ lpTarget) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	if(this->m_State != DG_STATE_START)
	{
		return;
	}

	if(this->m_Map != lpObj->Map)
	{
		return;
	}

	if(this->GetUser(lpObj->Index) == 0)
	{
		return;
	}

	this->GCDoubleGoerResultSend(lpObj->Index,1);

	this->DelUser(lpObj->Index);

	#endif
}

void CDoubleGoer::MonsterDieProc(LPOBJ lpObj,LPOBJ lpTarget) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	int aIndex = gObjMonsterGetTopHitDamageUser(lpObj);

	if(OBJECT_RANGE(aIndex) != 0)
	{
		lpTarget = &gObj[aIndex];
	}

	if(this->m_State != DG_STATE_START)
	{
		return;
	}

	if(this->m_Map != lpObj->Map)
	{
		return;
	}

	if(lpObj->Class == 530 && lpObj->Index == this->m_SlaughtererIndex)
	{
		this->m_SlaughtererIndex = -1;
		this->SetSilverRewardChest(lpObj->X,lpObj->Y);
		return;
	}

	if(lpObj->Class == 529 && lpObj->Index == this->m_MadSlaughtererIndex)
	{
		this->m_MadSlaughtererIndex = -1;
		this->SetSilverRewardChest(lpObj->X,lpObj->Y);
		return;
	}

	if(lpObj->Class == 531 && lpObj->Index == this->m_IceWalkerIndex)
	{
		this->m_IceWalkerIndex = -1;
		this->NoticeSendToAll(0,gMessage.GetMessage(306));
		return;
	}

	if(this->GetMonster(lpObj->Index) == 0)
	{
		return;
	}

	this->DelMonster(lpObj->Index);

	if(lpTarget->Type != OBJECT_USER)
	{
		return;
	}

	DOUBLE_GOER_USER* lpUser = this->GetUser(lpTarget->Index);

	if(lpUser == 0)
	{
		return;
	}

	#endif
}

void CDoubleGoer::NoticeSendToAll(int type,char* message,...) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	char buff[256];

	va_list arg;
	va_start(arg,message);
	vsprintf_s(buff,message,arg);
	va_end(arg);

	for(int n=0;n < MAX_DG_USER;n++)
	{
		if(OBJECT_RANGE(this->m_User[n].Index) != 0)
		{
			gNotice.GCNoticeSend(this->m_User[n].Index,type,0,0,0,0,0,buff);
		}
	}

	#endif
}

void CDoubleGoer::DataSendToAll(BYTE* lpMsg,int size) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	for(int n=0;n < MAX_DG_USER;n++)
	{
		if(OBJECT_RANGE(this->m_User[n].Index) != 0)
		{
			DataSend(this->m_User[n].Index,lpMsg,size);
		}
	}

	#endif
}

void CDoubleGoer::CGDoubleGoerEnterRecv(PMSG_DOUBLE_GOER_ENTER_RECV* lpMsg,int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	if(gServerInfo.m_DoubleGoerEvent == 0)
	{
		return;
	}

	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	PMSG_DOUBLE_GOER_ENTER_SEND pMsg;

	pMsg.header.set(0xBF,0x0E,sizeof(pMsg));

	pMsg.result = 0;

	lpMsg->slot = lpMsg->slot+INVENTORY_WEAR_SIZE;

	if(INVENTORY_FULL_RANGE(lpMsg->slot) == 0)
	{
		pMsg.result = 1;
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(lpObj->Inventory[lpMsg->slot].m_Index != GET_ITEM(14,111) && lpObj->Inventory[lpMsg->slot].m_Index != GET_ITEM(13,125))
	{
		pMsg.result = 1;
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(this->m_State != DG_STATE_EMPTY)
	{
		pMsg.result = 2;
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(gMapServerManager.CheckMapServer(MAP_DOUBLE_GOER1) == 0)
	{
		pMsg.result = 2;
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(gMapServerManager.CheckMapServer(MAP_DOUBLE_GOER2) == 0)
	{
		pMsg.result = 2;
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(gMapServerManager.CheckMapServer(MAP_DOUBLE_GOER3) == 0)
	{
		pMsg.result = 2;
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(gMapServerManager.CheckMapServer(MAP_DOUBLE_GOER4) == 0)
	{
		pMsg.result = 2;
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(gServerInfo.m_PKLimitFree == 0 && lpObj->PKLevel >= 4)
	{
		pMsg.result = 3;
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	gItemManager.DecreaseItemDur(lpObj,lpMsg->slot,1);

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);

	this->m_Level = GetLargeRand()%4;

	this->m_Map = MAP_DOUBLE_GOER1+this->m_Level;

	this->m_PartyNumber = lpObj->PartyNumber;

	if(OBJECT_RANGE(this->m_PartyNumber) == 0)
	{
		if(this->AddUser(aIndex) != 0)
		{
			lpObj->Interface.use = 0;
			lpObj->Interface.type = INTERFACE_NONE;
			lpObj->Interface.state = 0;

			gObjMoveGate(aIndex,329+this->m_Level);

			this->GCDoubleGoerStateSend(0,aIndex);

			this->m_PartyCount++;

			this->m_HighestLevel = lpObj->Level+lpObj->MasterLevel;
		}
	}
	else
	{
		for(int n=0;n < MAX_PARTY_USER;n++)
		{
			if(OBJECT_RANGE(gParty.m_PartyInfo[this->m_PartyNumber].Index[n]) != 0)
			{
				if(this->AddUser(gParty.m_PartyInfo[this->m_PartyNumber].Index[n]) != 0)
				{
					gObj[gParty.m_PartyInfo[this->m_PartyNumber].Index[n]].Interface.use = 0;
					gObj[gParty.m_PartyInfo[this->m_PartyNumber].Index[n]].Interface.type = INTERFACE_NONE;
					gObj[gParty.m_PartyInfo[this->m_PartyNumber].Index[n]].Interface.state = 0;

					gObjMoveGate(gParty.m_PartyInfo[this->m_PartyNumber].Index[n],329+this->m_Level);

					this->GCDoubleGoerStateSend(0,gParty.m_PartyInfo[this->m_PartyNumber].Index[n]);

					this->m_PartyCount++;

					if((gObj[gParty.m_PartyInfo[this->m_PartyNumber].Index[n]].Level+gObj[gParty.m_PartyInfo[this->m_PartyNumber].Index[n]].MasterLevel) > this->m_HighestLevel)
					{
						this->m_HighestLevel = gObj[gParty.m_PartyInfo[this->m_PartyNumber].Index[n]].Level+gObj[gParty.m_PartyInfo[this->m_PartyNumber].Index[n]].MasterLevel;
					}
				}
			}
		}
	}

	this->m_HighestLevel = ((this->m_HighestLevel>600)?600:this->m_HighestLevel);

	this->SetState(DG_STATE_STAND);

	#endif
}

void CDoubleGoer::GCDoubleGoerMonsterPositionSend() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	PMSG_DOUBLE_GOER_MONSTER_POSITION_SEND pMsg;

	pMsg.header.set(0xBF,0x0F,sizeof(pMsg));

	pMsg.position = 0;

	if(OBJECT_RANGE(this->m_SlaughtererIndex) != 0)
	{
		if((BYTE)((gDoubleGoerFinalPoint[this->m_Level]-gObj[this->m_SlaughtererIndex].Y)/gDoubleGoerPointValue[this->m_Level]) > pMsg.position)
		{
			pMsg.position = (BYTE)((gDoubleGoerFinalPoint[this->m_Level]-gObj[this->m_SlaughtererIndex].Y)/gDoubleGoerPointValue[this->m_Level]);
		}
	}

	if(OBJECT_RANGE(this->m_MadSlaughtererIndex) != 0)
	{
		if((BYTE)((gDoubleGoerFinalPoint[this->m_Level]-gObj[this->m_MadSlaughtererIndex].Y)/gDoubleGoerPointValue[this->m_Level]) > pMsg.position)
		{
			pMsg.position = (BYTE)((gDoubleGoerFinalPoint[this->m_Level]-gObj[this->m_MadSlaughtererIndex].Y)/gDoubleGoerPointValue[this->m_Level]);
		}
	}

	if(OBJECT_RANGE(this->m_IceWalkerIndex) != 0)
	{
		if((BYTE)((gDoubleGoerFinalPoint[this->m_Level]-gObj[this->m_IceWalkerIndex].Y)/gDoubleGoerPointValue[this->m_Level]) > pMsg.position)
		{
			pMsg.position = (BYTE)((gDoubleGoerFinalPoint[this->m_Level]-gObj[this->m_IceWalkerIndex].Y)/gDoubleGoerPointValue[this->m_Level]);
		}
	}

	for(int n=0;n < MAX_DG_MONSTER;n++)
	{
		if(OBJECT_RANGE(this->m_MonsterIndex[n]) != 0)
		{
			if((BYTE)((gDoubleGoerFinalPoint[this->m_Level]-gObj[this->m_MonsterIndex[n]].Y)/gDoubleGoerPointValue[this->m_Level]) > pMsg.position)
			{
				pMsg.position = (BYTE)((gDoubleGoerFinalPoint[this->m_Level]-gObj[this->m_MonsterIndex[n]].Y)/gDoubleGoerPointValue[this->m_Level]);
			}
		}
	}

	this->DataSendToAll((BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CDoubleGoer::GCDoubleGoerStateSend(int state,int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	PMSG_DOUBLE_GOER_STATE_SEND pMsg;

	pMsg.header.set(0xBF,0x10,sizeof(pMsg));

	pMsg.state = state;

	if(OBJECT_RANGE(aIndex) == 0)
	{
		this->DataSendToAll((BYTE*)&pMsg,pMsg.header.size);
	}
	else
	{
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
	}

	#endif
}

void CDoubleGoer::GCDoubleGoerBossPositionSend() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	if(OBJECT_RANGE(this->m_IceWalkerIndex) == 0)
	{
		return;
	}

	PMSG_DOUBLE_GOER_BOSS_POSITION_SEND pMsg;

	pMsg.header.set(0xBF,0x11,sizeof(pMsg));

	pMsg.type = 0;

	pMsg.position = (BYTE)((gDoubleGoerFinalPoint[this->m_Level]-gObj[this->m_IceWalkerIndex].Y)/gDoubleGoerPointValue[this->m_Level]);

	this->DataSendToAll((BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CDoubleGoer::GCDoubleGoerInterfaceSend() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	BYTE send[256];

	PMSG_DOUBLE_GOER_INTERFACE_SEND pMsg;

	pMsg.header.set(0xBF,0x12,0);

	int size = sizeof(pMsg);

	pMsg.time = this->m_RemainTime/1000;

	pMsg.count = 0;

	PMSG_DOUBLE_GOER_INTERFACE info;

	for(int n=0;n < MAX_DG_USER;n++)
	{
		if(OBJECT_RANGE(this->m_User[n].Index) == 0)
		{
			continue;
		}

		info.index = this->m_User[n].Index;

		info.number = n+1;

		info.position = (BYTE)((gDoubleGoerFinalPoint[this->m_Level]-gObj[this->m_User[n].Index].Y)/gDoubleGoerPointValue[this->m_Level]);

		memcpy(&send[size],&info,sizeof(info));
		size += sizeof(info);

		pMsg.count++;
	}

	pMsg.header.size = size;

	memcpy(send,&pMsg,sizeof(pMsg));

	this->DataSendToAll(send,size);

	#endif
}

void CDoubleGoer::GCDoubleGoerResultSend(int aIndex,int result) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	PMSG_DOUBLE_GOER_RESULT_SEND pMsg;

	pMsg.header.set(0xBF,0x13,sizeof(pMsg));

	pMsg.result = result;

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CDoubleGoer::GCDoubleGoerMonsterInfoSend() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	PMSG_DOUBLE_GOER_MONSTER_INFO_SEND pMsg;

	pMsg.header.set(0xBF,0x14,sizeof(pMsg));

	pMsg.CurMonster = this->m_CurMonster;

	pMsg.MaxMonster = this->m_MaxMonster;

	this->DataSendToAll((BYTE*)&pMsg,pMsg.header.size);

	#endif
}
