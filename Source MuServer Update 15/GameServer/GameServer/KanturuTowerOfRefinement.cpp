// KanturuTowerOfRefinement.cpp: implementation of the CKanturuTowerOfRefinement class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "KanturuTowerOfRefinement.h"
#include "Kanturu.h"
#include "KanturuBattleUserMng.h"
#include "KanturuUtil.h"
#include "Map.h"
#include "MemScript.h"
#include "Message.h"
#include "Util.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CKanturuTowerOfRefinement::CKanturuTowerOfRefinement() // OK
{
	this->SetTowerOfRefinementState(KANTURU_TOWER_OF_REFINEMENT_NONE);

	this->SetEntrancePermit(0);

	this->SetUseTowerOfRefinement(0);

	this->ResetAllData();
}

CKanturuTowerOfRefinement::~CKanturuTowerOfRefinement() // OK
{

}

void CKanturuTowerOfRefinement::ResetAllData() // OK
{
	this->m_FileDataLoad = 0;

	for(int n=0;n < KANTURU_TOWER_OF_REFINEMENT_STATE_INFO;n++)
	{
		this->m_StateInfo[n].ResetTimeInfo();
	}

	this->m_StateInfoCount = 0;
}

void CKanturuTowerOfRefinement::Load(char* path) // OK
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
				if(section == 9)
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

void CKanturuTowerOfRefinement::MainProc() // OK
{
	this->CheckStateTime();

	switch(this->m_TowerOfRefinementState)
	{
		case KANTURU_TOWER_OF_REFINEMENT_NONE:
			this->ProcState_NONE();
			break;
		case KANTURU_TOWER_OF_REFINEMENT_REVITALIZATION:
			this->ProcState_REVITALIZATION();
			break;
		case KANTURU_TOWER_OF_REFINEMENT_NOTIFY1:
			this->ProcState_NOTIFY1();
			break;
		case KANTURU_TOWER_OF_REFINEMENT_CLOSE:
			this->ProcState_CLOSE();
			break;
		case KANTURU_TOWER_OF_REFINEMENT_NOTIFY2:
			this->ProcState_NOTIFY2();
			break;
		case KANTURU_TOWER_OF_REFINEMENT_END:
			this->ProcState_END();
			break;
	}
}

void CKanturuTowerOfRefinement::ProcState_NONE() // OK
{

}

void CKanturuTowerOfRefinement::ProcState_REVITALIZATION() // OK
{

}

void CKanturuTowerOfRefinement::ProcState_NOTIFY1() // OK
{

}

void CKanturuTowerOfRefinement::ProcState_CLOSE() // OK
{

}

void CKanturuTowerOfRefinement::ProcState_NOTIFY2() // OK
{

}

void CKanturuTowerOfRefinement::ProcState_END() // OK
{

}

void CKanturuTowerOfRefinement::SetState(int state) // OK
{
	this->m_StateInfo[state].SetConditionAppliedTime();

	switch(state)
	{
		case KANTURU_TOWER_OF_REFINEMENT_NONE:
			this->SetState_NONE();
			break;
		case KANTURU_TOWER_OF_REFINEMENT_REVITALIZATION:
			this->SetState_REVITALIZATION();
			break;
		case KANTURU_TOWER_OF_REFINEMENT_NOTIFY1:
			this->SetState_NOTIFY1();
			break;
		case KANTURU_TOWER_OF_REFINEMENT_CLOSE:
			this->SetState_CLOSE();
			break;
		case KANTURU_TOWER_OF_REFINEMENT_NOTIFY2:
			this->SetState_NOTIFY2();
			break;
		case KANTURU_TOWER_OF_REFINEMENT_END:
			this->SetState_END();
			break;
	}

	gKanturuUtil.NotifyKanturuChangeState(KANTURU_STATE_TOWER_OF_REFINEMENT,this->m_TowerOfRefinementState);
}

void CKanturuTowerOfRefinement::SetState_NONE() // OK
{
	//LogAdd(LOG_BLACK,"[ KANTURU ][ TowerOfRefinement ] State(%d) -> NONE",this->m_TowerOfRefinementState);

	this->SetTowerOfRefinementState(KANTURU_TOWER_OF_REFINEMENT_NONE);

	this->SetEntrancePermit(0);

	this->SetUseTowerOfRefinement(0);

	gKanturuBattleUserMng.ResetAllData();
}

void CKanturuTowerOfRefinement::SetState_REVITALIZATION() // OK
{
	//LogAdd(LOG_BLACK,"[ KANTURU ][ TowerOfRefinement ] State(%d) -> REVITALIZATION",this->m_TowerOfRefinementState);

	this->SetTowerOfRefinementState(KANTURU_TOWER_OF_REFINEMENT_REVITALIZATION);

	this->SetEntrancePermit(1);

	this->SetUseTowerOfRefinement(1);
}

void CKanturuTowerOfRefinement::SetState_NOTIFY1() // OK
{
	//LogAdd(LOG_BLACK,"[ KANTURU ][ TowerOfRefinement ] State(%d) -> NOTIFY1",this->m_TowerOfRefinementState);

	this->SetTowerOfRefinementState(KANTURU_TOWER_OF_REFINEMENT_NOTIFY1);

	this->SetEntrancePermit(1);

	this->SetUseTowerOfRefinement(1);

	gKanturuUtil.SendMsgKanturuBossMapUser(gMessage.GetMessage(336));
}

void CKanturuTowerOfRefinement::SetState_CLOSE() // OK
{
	//LogAdd(LOG_BLACK,"[ KANTURU ][ TowerOfRefinement ] State(%d) -> CLOSE",this->m_TowerOfRefinementState);

	this->SetTowerOfRefinementState(KANTURU_TOWER_OF_REFINEMENT_CLOSE);

	this->SetEntrancePermit(0);

	this->SetUseTowerOfRefinement(1);

	gKanturuUtil.SendMsgKanturuBossMapUser(gMessage.GetMessage(337));
}

void CKanturuTowerOfRefinement::SetState_NOTIFY2() // OK
{
	//LogAdd(LOG_BLACK,"[ KANTURU ][ TowerOfRefinement ] State(%d) -> NOTIFY2",this->m_TowerOfRefinementState);

	this->SetTowerOfRefinementState(KANTURU_TOWER_OF_REFINEMENT_NOTIFY2);

	this->SetEntrancePermit(0);

	this->SetUseTowerOfRefinement(1);
}

void CKanturuTowerOfRefinement::SetState_END() // OK
{
	//LogAdd(LOG_BLACK,"[ KANTURU ][ TowerOfRefinement ] State(%d) -> END",this->m_TowerOfRefinementState);

	this->SetTowerOfRefinementState(KANTURU_TOWER_OF_REFINEMENT_END);

	this->SetEntrancePermit(0);

	this->SetUseTowerOfRefinement(0);

	for(int n=OBJECT_START_USER;n < MAX_OBJECT;n++)
	{
		if(gObjIsConnectedGP(n) != 0 && gObj[n].Map == MAP_KANTURU3)
		{
			gObjMoveGate(n,136);
			//LogAdd(LOG_BLACK,"[ KANTURU ][ TowerOfRefinement ] State End:Kick Out [%s][%s]",gObj[n].Account,gObj[n].Name);
		}
	}
}

void CKanturuTowerOfRefinement::CheckStateTime() // OK
{
	if(this->GetTowerOfRefinementState() != KANTURU_TOWER_OF_REFINEMENT_NONE && this->m_StateInfo[this->GetTowerOfRefinementState()].GetCondition() == 1 && this->m_StateInfo[this->GetTowerOfRefinementState()].IsTimeOut() != 0)
	{
		this->SetState(((this->GetTowerOfRefinementState()==KANTURU_TOWER_OF_REFINEMENT_END)?KANTURU_TOWER_OF_REFINEMENT_NONE:(this->GetTowerOfRefinementState()+1)));
	}
}

void CKanturuTowerOfRefinement::SetTowerOfRefinementState(int state) // OK
{
	this->m_TowerOfRefinementState = state;
}

void CKanturuTowerOfRefinement::SetEntrancePermit(int permit) // OK
{
	this->m_EntrancePermit = permit;
}

void CKanturuTowerOfRefinement::SetUseTowerOfRefinement(int use) // OK
{
	this->m_IsUseTowerOfRefinement = use;
}

int CKanturuTowerOfRefinement::GetTowerOfRefinementState() // OK
{
	return this->m_TowerOfRefinementState;
}

int CKanturuTowerOfRefinement::GetEntrancePermit() // OK
{
	return this->m_EntrancePermit;
}

int CKanturuTowerOfRefinement::GetUseTowerOfRefinement() // OK
{
	return this->m_IsUseTowerOfRefinement;
}

int CKanturuTowerOfRefinement::GetRemainTime() // OK
{
	return this->m_StateInfo[this->GetTowerOfRefinementState()].GetRemainTime();
}
