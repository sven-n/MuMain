// KanturuBattleOfNightmare.cpp: implementation of the CKanturuBattleOfNightmare class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "KanturuBattleOfNightmare.h"
#include "Kanturu.h"
#include "KanturuBattleUserMng.h"
#include "KanturuMonsterMng.h"
#include "KanturuUtil.h"
#include "MemScript.h"
#include "Message.h"
#include "MonsterAIGroup.h"
#include "Util.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CKanturuBattleOfNightmare::CKanturuBattleOfNightmare() // OK
{
	this->SetBattleOfNightmareState(KANTURU_NIGHTMARE_NONE);

	this->SetSuccessValue(0);

	this->m_NightmareAIGroupNumber = 0;

	this->m_NightmareObjIndex = -1;

	this->ResetAllData();
}

CKanturuBattleOfNightmare::~CKanturuBattleOfNightmare() // OK
{

}

void CKanturuBattleOfNightmare::ResetAllData() // OK
{
	this->m_FileDataLoad = 0;

	for(int n=0;n < KANTURU_NIGHTMARE_STATE_INFO;n++)
	{
		this->m_StateInfo[n].ResetTimeInfo();
	}

	this->m_StateInfoCount = 0;
}

void CKanturuBattleOfNightmare::Load(char* path) // OK
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

	this->ResetAllData();

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
				if(section == 7)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					int state = lpMemScript->GetNumber();

					int condition = lpMemScript->GetAsNumber();

					int value = lpMemScript->GetAsNumber();

					this->m_StateInfo[this->m_StateInfoCount].SetStateInfo(state);

					this->m_StateInfo[this->m_StateInfoCount].SetCondition(condition);

					this->m_StateInfo[this->m_StateInfoCount].SetValue(((condition==1)?(value*1000):value));

					this->m_StateInfoCount++;
				}
				else if(section == 8)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					this->m_NightmareAIGroupNumber = lpMemScript->GetNumber();
				}
				else
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}
				}
			}
		}
	}
	catch(...)
	{
		ErrorMessageBox(lpMemScript->GetLastError());
	}

	this->m_FileDataLoad = 1;

	delete lpMemScript;
}

void CKanturuBattleOfNightmare::MainProc() // OK
{
	this->CheckStateTime();

	this->CheckNightmareBattleUser();

	gKanturuBattleUserMng.CheckUserState();

	switch(this->m_BattleOfNightmareState)
	{
		case KANTURU_NIGHTMARE_NONE:
			this->ProcState_NONE();
			break;
		case KANTURU_NIGHTMARE_IDLE:
			this->ProcState_IDLE();
			break;
		case KANTURU_NIGHTMARE_NOTIFY:
			this->ProcState_NOTIFY();
			break;
		case KANTURU_NIGHTMARE_START:
			this->ProcState_START();
			break;
		case KANTURU_NIGHTMARE_END:
			this->ProcState_END();
			break;
		case KANTURU_NIGHTMARE_ENDCYCLE:
			this->ProcState_ENDCYCLE();
			break;
	}
}

void CKanturuBattleOfNightmare::ProcState_NONE() // OK
{

}

void CKanturuBattleOfNightmare::ProcState_IDLE() // OK
{

}

void CKanturuBattleOfNightmare::ProcState_NOTIFY() // OK
{

}

void CKanturuBattleOfNightmare::ProcState_START() // OK
{
	gKanturuUtil.NotifyKanturuObjectCount(gKanturuMonsterMng.GetAliveMonsterCount(),gKanturuBattleUserMng.GetUserCount());

	if(OBJECT_RANGE(this->m_NightmareObjIndex) != 0 && gObj[this->m_NightmareObjIndex].Live == 0)
	{
		this->SetSuccessValue(1);

		this->SetState(KANTURU_NIGHTMARE_END);

		//LogAdd(LOG_BLACK,"[ KANTURU ][ BattleOfNightmare ] Success!! Nightmare Die");
	}
}

void CKanturuBattleOfNightmare::ProcState_END() // OK
{

}

void CKanturuBattleOfNightmare::ProcState_ENDCYCLE() // OK
{
	CMonsterAIGroup::DelGroupInstance(this->m_NightmareAIGroupNumber);
}

void CKanturuBattleOfNightmare::SetState(int state) // OK
{
	this->m_StateInfo[state].SetConditionAppliedTime();

	gKanturuMonsterMng.ResetRegenMonsterObjData();

	this->m_NightmareObjIndex = -1;

	switch(state)
	{
		case KANTURU_NIGHTMARE_NONE:
			this->SetState_NONE();
			break;
		case KANTURU_NIGHTMARE_IDLE:
			this->SetState_IDLE();
			break;
		case KANTURU_NIGHTMARE_NOTIFY:
			this->SetState_NOTIFY();
			break;
		case KANTURU_NIGHTMARE_START:
			this->SetState_START();
			break;
		case KANTURU_NIGHTMARE_END:
			this->SetState_END();
			break;
		case KANTURU_NIGHTMARE_ENDCYCLE:
			this->SetState_ENDCYCLE();
			break;
	}

	gKanturuUtil.NotifyKanturuChangeState(KANTURU_STATE_BATTLE_OF_NIGHTMARE,this->m_BattleOfNightmareState);
}

void CKanturuBattleOfNightmare::SetState_NONE() // OK
{
	//LogAdd(LOG_BLACK,"[ KANTURU ][ BattleOfNightmare ] State(%d) -> NONE",this->m_BattleOfNightmareState);

	this->SetBattleOfNightmareState(KANTURU_NIGHTMARE_NONE);

	CMonsterAIGroup::DelGroupInstance(this->m_NightmareAIGroupNumber);
}

void CKanturuBattleOfNightmare::SetState_IDLE() // OK
{
	//LogAdd(LOG_BLACK,"[ KANTURU ][ BattleOfNightmare ] State(%d) -> IDLE",this->m_BattleOfNightmareState);

	this->SetBattleOfNightmareState(KANTURU_NIGHTMARE_IDLE);

	this->SetSuccessValue(0);

	CMonsterAIGroup::DelGroupInstance(this->m_NightmareAIGroupNumber);

	gKanturuBattleUserMng.MoveAllUser(134);
}

void CKanturuBattleOfNightmare::SetState_NOTIFY() // OK
{
	//LogAdd(LOG_BLACK,"[ KANTURU ][ BattleOfNightmare ] State(%d) -> NOTIFY",this->m_BattleOfNightmareState);

	this->SetBattleOfNightmareState(KANTURU_NIGHTMARE_NOTIFY);
}

void CKanturuBattleOfNightmare::SetState_START() // OK
{
	//LogAdd(LOG_BLACK,"[ KANTURU ][ BattleOfNightmare ] State(%d) -> START",this->m_BattleOfNightmareState);

	this->SetBattleOfNightmareState(KANTURU_NIGHTMARE_START);

	if(this->m_StateInfo[this->m_BattleOfNightmareState].GetCondition() == 1)
	{
		gKanturuUtil.NotifyKanturuBattleTime(this->m_StateInfo[this->m_BattleOfNightmareState].GetValue());
	}

	gKanturuMonsterMng.SetKanturuMonster(2);

	CMonsterAIGroup::Init(this->m_NightmareAIGroupNumber);

	CMonsterAIGroup::ChangeAIOrder(this->m_NightmareAIGroupNumber,0);

	this->m_NightmareObjIndex = CMonsterAIGroup::FindGroupLeader(this->m_NightmareAIGroupNumber);
}

void CKanturuBattleOfNightmare::SetState_END() // OK
{
	//LogAdd(LOG_BLACK,"[ KANTURU ][ BattleOfNightmare ] State(%d) -> END",this->m_BattleOfNightmareState);

	this->SetBattleOfNightmareState(KANTURU_NIGHTMARE_END);

	gKanturuUtil.NotifyKanturuResult(this->GetSuccessValue());

	if(this->GetSuccessValue() == 0)
	{
		gKanturuUtil.SendMsgKanturuBossMapUser(gMessage.GetMessage(343));
		//LogAdd(LOG_BLACK,"[ KANTURU ][ BattleOfNightmare ] Fail!! TimeOut");
	}
}

void CKanturuBattleOfNightmare::SetState_ENDCYCLE() // OK
{
	//LogAdd(LOG_BLACK,"[ KANTURU ][ BattleOfNightmare ] State(%d) -> ENDCYCLE",this->m_BattleOfNightmareState);

	this->SetBattleOfNightmareState(KANTURU_NIGHTMARE_ENDCYCLE);

	if(this->GetSuccessValue() == 0)
	{
		gKanturuBattleUserMng.MoveAllUser(137);
		gKanturuBattleUserMng.ResetAllData();
	}
	else
	{
		gKanturuUtil.SendMsgKanturuBossMapUser(gMessage.GetMessage(342));
	}
}

void CKanturuBattleOfNightmare::CheckStateTime()
{
	if(this->GetBattleOfNightmareState() != KANTURU_NIGHTMARE_NONE && this->m_StateInfo[this->GetBattleOfNightmareState()].GetCondition() == 1 && this->m_StateInfo[this->GetBattleOfNightmareState()].IsTimeOut() != 0)
	{
		this->SetState(((this->GetBattleOfNightmareState()==KANTURU_NIGHTMARE_ENDCYCLE)?KANTURU_NIGHTMARE_NONE:(this->GetBattleOfNightmareState()+1)));
	}
}

void CKanturuBattleOfNightmare::CheckNightmareBattleUser() // OK
{
	if(this->GetBattleOfNightmareState() == KANTURU_NIGHTMARE_NOTIFY || this->GetBattleOfNightmareState() == KANTURU_NIGHTMARE_START)
	{
		if(gKanturuBattleUserMng.IsEmpty() != 0)
		{
			this->SetSuccessValue(0);

			this->SetState(KANTURU_NIGHTMARE_END);

			gKanturuBattleUserMng.ResetAllData();

			//LogAdd(LOG_BLACK,"[ KANTURU ][ BattleOfNightmare ] Fail!! All Battle User Die");
		}
	}
}

void CKanturuBattleOfNightmare::SetBattleOfNightmareState(int state) // OK
{
	this->m_BattleOfNightmareState = state;
}

void CKanturuBattleOfNightmare::SetSuccessValue(int success) // OK
{
	this->m_IsSucccess = success;
}

int CKanturuBattleOfNightmare::GetBattleOfNightmareState() // OK
{
	return this->m_BattleOfNightmareState;
}

int CKanturuBattleOfNightmare::GetSuccessValue() // OK
{
	return this->m_IsSucccess;
}

int CKanturuBattleOfNightmare::GetRemainTime() // OK
{
	return this->m_StateInfo[this->GetBattleOfNightmareState()].GetRemainTime();
}
