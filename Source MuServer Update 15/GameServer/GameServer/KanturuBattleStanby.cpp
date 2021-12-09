// KanturuBattleStanby.cpp: implementation of the CKanturuBattleStanby class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "KanturuBattleStanby.h"
#include "Kanturu.h"
#include "KanturuBattleUserMng.h"
#include "KanturuMonsterMng.h"
#include "KanturuUtil.h"
#include "MemScript.h"
#include "Message.h"
#include "Notice.h"
#include "Util.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CKanturuBattleStanby::CKanturuBattleStanby() // OK
{
	this->SetBattleStanbyState(KANTURU_STANBY_NONE);

	this->SetSuccessValue(0);

	this->ResetAllData();
}

CKanturuBattleStanby::~CKanturuBattleStanby() // OK
{

}

void CKanturuBattleStanby::ResetAllData() // OK
{
	this->m_FileDataLoad = 0;

	for(int n=0;n < KANTURU_STANBY_STATE_INFO;n++)
	{
		this->m_StateInfo[n].ResetTimeInfo();
	}

	this->m_StateInfoCount = 0;
}

void CKanturuBattleStanby::Load(char* path) // OK
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
				if(section == 1)
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

void CKanturuBattleStanby::MainProc() // OK
{
	this->CheckStateTime();

	switch(this->m_BattleStanbyState)
	{
		case KANTURU_STANBY_NONE:
			this->ProcState_NONE();
			break;
		case KANTURU_STANBY_START:
			this->ProcState_START();
			break;
		case KANTURU_STANBY_NOTIFY:
			this->ProcState_NOTIFY();
			break;
		case KANTURU_STANBY_END:
			this->ProcState_END();
			break;
	}
}

void CKanturuBattleStanby::ProcState_NONE() // OK
{

}

void CKanturuBattleStanby::ProcState_START() // OK
{

}

void CKanturuBattleStanby::ProcState_NOTIFY() // OK
{

}

void CKanturuBattleStanby::ProcState_END() // OK
{

}

void CKanturuBattleStanby::SetState(int state) // OK
{
	this->m_StateInfo[state].SetConditionAppliedTime();

	gKanturuMonsterMng.ResetRegenMonsterObjData();

	switch(state)
	{
		case KANTURU_STANBY_NONE:
			this->SetState_NONE();
			break;
		case KANTURU_STANBY_START:
			this->SetState_START();
			break;
		case KANTURU_STANBY_NOTIFY:
			this->SetState_NOTIFY();
			break;
		case KANTURU_STANBY_END:
			this->SetState_END();
			break;
	}

	gKanturuUtil.NotifyKanturuChangeState(KANTURU_STATE_BATTLE_STANTBY,this->m_BattleStanbyState);
}

void CKanturuBattleStanby::SetState_NONE() // OK
{
	//LogAdd(LOG_BLACK,"[ KANTURU ][ BattleStanby ] State(%d) -> NONE",this->m_BattleStanbyState);

	this->SetBattleStanbyState(KANTURU_STANBY_NONE);
}

void CKanturuBattleStanby::SetState_START() // OK
{
	////LogAdd(LOG_BLACK,"[ KANTURU ][ BattleStanby ] State(%d) -> START",this->m_BattleStanbyState);

	this->SetBattleStanbyState(KANTURU_STANBY_START);
}

void CKanturuBattleStanby::SetState_NOTIFY() // OK
{
	//LogAdd(LOG_BLACK,"[ KANTURU ][ BattleStanby ] State(%d) -> NOTIFY",this->m_BattleStanbyState);

	this->SetBattleStanbyState(KANTURU_STANBY_NOTIFY);

	gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(338));
}

void CKanturuBattleStanby::SetState_END() // OK
{
	//LogAdd(LOG_BLACK,"[ KANTURU ][ BattleStanby ] State(%d) -> END",this->m_BattleStanbyState);

	this->SetBattleStanbyState(KANTURU_STANBY_END);
}

void CKanturuBattleStanby::CheckStateTime() // OK
{
	if(this->GetBattleStanbyState() != KANTURU_STANBY_NONE && this->m_StateInfo[this->GetBattleStanbyState()].GetCondition() == 1 && this->m_StateInfo[this->GetBattleStanbyState()].IsTimeOut() != 0)
	{
		this->SetState(((this->GetBattleStanbyState()==KANTURU_STANBY_END)?KANTURU_STANBY_NONE:(this->GetBattleStanbyState()+1)));
	}
}

void CKanturuBattleStanby::SetBattleStanbyState(int state) // OK
{
	this->m_BattleStanbyState = state;
}

void CKanturuBattleStanby::SetSuccessValue(int success) // OK
{
	this->m_IsSucccess = success;
}

int CKanturuBattleStanby::GetBattleStanbyState() // OK
{
	return this->m_BattleStanbyState;
}

int CKanturuBattleStanby::GetRemainTime() // OK
{
	return this->m_StateInfo[this->GetBattleStanbyState()].GetRemainTime();
}
