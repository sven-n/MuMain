// ImperialGuardian.cpp: implementation of the CImperialGuardian class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ImperialGuardian.h"
#include "DSProtocol.h"
#include "EffectManager.h"
#include "ItemBagManager.h"
#include "ItemManager.h"
#include "Map.h"
#include "MapServerManager.h"
#include "MemScript.h"
#include "Message.h"
#include "Monster.h"
#include "MonsterSetBase.h"
#include "Notice.h"
#include "NpcTalk.h"
#include "ObjectManager.h"
#include "Party.h"
#include "ServerInfo.h"
#include "Util.h"

CImperialGuardian gImperialGuardian;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CImperialGuardian::CImperialGuardian() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	this->m_DayOfWeek = 0;
	this->m_Level = 0;
	this->m_State = IG_STATE_EMPTY;
	this->m_Map = 0;
	this->m_RemainTime = 0;
	this->m_TargetTime = 0;
	this->m_TickCount = GetTickCount();
	this->m_TimeCount = 0;
	this->m_Stage = 0;
	this->m_CurMonster = 0;
	this->m_MaxMonster = 0;
	this->m_PartyNumber = -1;
	this->m_PartyCount = 0;
	this->m_HighestLevel = 0;

	this->CleanUser();

	this->m_SideGateIndex = -1;
	this->m_LockGateIndex = -1;
	this->m_MainGateIndex = -1;
	this->m_SideBossIndex = -1;
	this->m_MainBossIndex = -1;

	this->CleanMonster();

	#endif
}

CImperialGuardian::~CImperialGuardian() // OK
{

}

void CImperialGuardian::Load(char* path) // OK
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

					this->m_DefaultMonsterAIUnitNumber = lpMemScript->GetNumber();

					this->m_SupportMonsterAIUnitNumber = lpMemScript->GetAsNumber();
				}
				else if(section == 2)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					IMPERIAL_GUARDIAN_MONSTER_DIFFICULT_INFO info;

					memset(&info,0,sizeof(info));

					info.Index = lpMemScript->GetNumber();

					info.Level = lpMemScript->GetAsNumber();

					info.Life = lpMemScript->GetAsFloatNumber();

					info.DamageMin = lpMemScript->GetAsFloatNumber();

					info.DamageMax = lpMemScript->GetAsFloatNumber();

					info.Defense = lpMemScript->GetAsFloatNumber();

					info.AttackSuccessRate = lpMemScript->GetAsFloatNumber();

					info.DefenseSuccessRate = lpMemScript->GetAsFloatNumber();

					if(info.Index >= 0 && info.Index < MAX_IG_MONSTER_DIFFICULT)
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

void CImperialGuardian::MainProc() // OK
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
		case IG_STATE_EMPTY:
			this->ProcState_EMPTY();
			break;
		case IG_STATE_WAIT1:
			this->ProcState_WAIT1();
			break;
		case IG_STATE_PLAY1:
			this->ProcState_PLAY1();
			break;
		case IG_STATE_WAIT2:
			this->ProcState_WAIT2();
			break;
		case IG_STATE_PLAY2:
			this->ProcState_PLAY2();
			break;
		case IG_STATE_WAIT3:
			this->ProcState_WAIT3();
			break;
		case IG_STATE_PLAY3:
			this->ProcState_PLAY3();
			break;
		case IG_STATE_WAIT4:
			this->ProcState_WAIT4();
			break;
		case IG_STATE_PLAY4:
			this->ProcState_PLAY4();
			break;
		case IG_STATE_CLEAN:
			this->ProcState_CLEAN();
			break;
	}

	#endif
}

void CImperialGuardian::ProcState_EMPTY() // OK
{

}

void CImperialGuardian::ProcState_WAIT1() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	this->CheckUser();

	if(this->GetUserCount() == 0)
	{
		this->SetState(IG_STATE_EMPTY);
		return;
	}

	this->GCImperialGuardianInterfaceSend();

	if(this->m_RemainTime > 0 && this->m_RemainTime <= 30000 && this->m_TimeCount == 0)
	{
		PMSG_TIME_COUNT_SEND pMsg;

		pMsg.header.set(0x92,sizeof(pMsg));

		pMsg.type = 14;

		this->DataSendToAll((BYTE*)&pMsg,pMsg.header.size);

		this->m_TimeCount = 1;
	}

	if(this->m_RemainTime <= 0)
	{
		this->SetState(IG_STATE_PLAY1);
	}

	#endif
}

void CImperialGuardian::ProcState_PLAY1() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	this->CheckUser();

	if(this->GetUserCount() == 0)
	{
		this->SetState(IG_STATE_EMPTY);
		return;
	}

	this->GCImperialGuardianInterfaceSend();

	if(OBJECT_RANGE(this->m_LockGateIndex) != 0 && OBJECT_RANGE(this->m_MainGateIndex) != 0 && this->m_CurMonster >= this->m_MaxMonster)
	{
		gEffectManager.DelEffect(&gObj[this->m_LockGateIndex],EFFECT_MONSTER_PHYSI_DAMAGE_IMMUNITY);

		gEffectManager.DelEffect(&gObj[this->m_LockGateIndex],EFFECT_MONSTER_MAGIC_DAMAGE_IMMUNITY);

		gEffectManager.DelEffect(&gObj[this->m_MainGateIndex],EFFECT_MONSTER_PHYSI_DAMAGE_IMMUNITY);

		gEffectManager.DelEffect(&gObj[this->m_MainGateIndex],EFFECT_MONSTER_MAGIC_DAMAGE_IMMUNITY);
	}

	if(this->m_RemainTime > 0 && this->m_RemainTime <= 30000 && this->m_TimeCount == 0)
	{
		PMSG_TIME_COUNT_SEND pMsg;

		pMsg.header.set(0x92,sizeof(pMsg));

		pMsg.type = 15;

		this->DataSendToAll((BYTE*)&pMsg,pMsg.header.size);

		this->m_TimeCount = 1;
	}

	if(this->m_RemainTime <= 0)
	{
		this->SetState(IG_STATE_CLEAN);
	}

	#endif
}

void CImperialGuardian::ProcState_WAIT2() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	this->CheckUser();

	if(this->GetUserCount() == 0)
	{
		this->SetState(IG_STATE_EMPTY);
		return;
	}

	this->GCImperialGuardianInterfaceSend();

	if(this->m_RemainTime > 0 && this->m_RemainTime <= 30000 && this->m_TimeCount == 0)
	{
		PMSG_TIME_COUNT_SEND pMsg;

		pMsg.header.set(0x92,sizeof(pMsg));

		pMsg.type = 14;

		this->DataSendToAll((BYTE*)&pMsg,pMsg.header.size);

		this->m_TimeCount = 1;
	}

	if(this->m_RemainTime <= 0)
	{
		this->SetState(IG_STATE_PLAY2);
	}

	#endif
}

void CImperialGuardian::ProcState_PLAY2() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	this->CheckUser();

	if(this->GetUserCount() == 0)
	{
		this->SetState(IG_STATE_EMPTY);
		return;
	}

	this->GCImperialGuardianInterfaceSend();

	if(OBJECT_RANGE(this->m_LockGateIndex) != 0 && OBJECT_RANGE(this->m_MainGateIndex) != 0 && this->m_CurMonster >= this->m_MaxMonster)
	{
		gEffectManager.DelEffect(&gObj[this->m_LockGateIndex],EFFECT_MONSTER_PHYSI_DAMAGE_IMMUNITY);

		gEffectManager.DelEffect(&gObj[this->m_LockGateIndex],EFFECT_MONSTER_MAGIC_DAMAGE_IMMUNITY);

		gEffectManager.DelEffect(&gObj[this->m_MainGateIndex],EFFECT_MONSTER_PHYSI_DAMAGE_IMMUNITY);

		gEffectManager.DelEffect(&gObj[this->m_MainGateIndex],EFFECT_MONSTER_MAGIC_DAMAGE_IMMUNITY);
	}

	if(this->m_RemainTime > 0 && this->m_RemainTime <= 30000 && this->m_TimeCount == 0)
	{
		PMSG_TIME_COUNT_SEND pMsg;

		pMsg.header.set(0x92,sizeof(pMsg));

		pMsg.type = 15;

		this->DataSendToAll((BYTE*)&pMsg,pMsg.header.size);

		this->m_TimeCount = 1;
	}

	if(this->m_RemainTime <= 0)
	{
		this->SetState(IG_STATE_CLEAN);
	}

	#endif
}

void CImperialGuardian::ProcState_WAIT3() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	this->CheckUser();

	if(this->GetUserCount() == 0)
	{
		this->SetState(IG_STATE_EMPTY);
		return;
	}

	this->GCImperialGuardianInterfaceSend();

	if(this->m_RemainTime > 0 && this->m_RemainTime <= 30000 && this->m_TimeCount == 0)
	{
		PMSG_TIME_COUNT_SEND pMsg;

		pMsg.header.set(0x92,sizeof(pMsg));

		pMsg.type = 14;

		this->DataSendToAll((BYTE*)&pMsg,pMsg.header.size);

		this->m_TimeCount = 1;
	}

	if(this->m_RemainTime <= 0)
	{
		this->SetState(IG_STATE_PLAY3);
	}

	#endif
}

void CImperialGuardian::ProcState_PLAY3() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	this->CheckUser();

	if(this->GetUserCount() == 0)
	{
		this->SetState(IG_STATE_EMPTY);
		return;
	}

	this->GCImperialGuardianInterfaceSend();

	if(this->m_Level == 3 && OBJECT_RANGE(this->m_LockGateIndex) != 0 && OBJECT_RANGE(this->m_MainGateIndex) != 0 && this->m_CurMonster >= this->m_MaxMonster)
	{
		gEffectManager.DelEffect(&gObj[this->m_LockGateIndex],EFFECT_MONSTER_PHYSI_DAMAGE_IMMUNITY);

		gEffectManager.DelEffect(&gObj[this->m_LockGateIndex],EFFECT_MONSTER_MAGIC_DAMAGE_IMMUNITY);

		gEffectManager.DelEffect(&gObj[this->m_MainGateIndex],EFFECT_MONSTER_PHYSI_DAMAGE_IMMUNITY);

		gEffectManager.DelEffect(&gObj[this->m_MainGateIndex],EFFECT_MONSTER_MAGIC_DAMAGE_IMMUNITY);
	}

	if(this->m_Level != 3 && OBJECT_RANGE(this->m_SideGateIndex) == 0 && this->m_CurMonster >= this->m_MaxMonster)
	{
		this->SetState(IG_STATE_CLEAN);
		return;
	}

	if(this->m_RemainTime > 0 && this->m_RemainTime <= 30000 && this->m_TimeCount == 0)
	{
		PMSG_TIME_COUNT_SEND pMsg;

		pMsg.header.set(0x92,sizeof(pMsg));

		pMsg.type = 15;

		this->DataSendToAll((BYTE*)&pMsg,pMsg.header.size);

		this->m_TimeCount = 1;
	}

	if(this->m_RemainTime <= 0)
	{
		this->SetState(IG_STATE_CLEAN);
	}

	#endif
}

void CImperialGuardian::ProcState_WAIT4() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	this->CheckUser();

	if(this->GetUserCount() == 0)
	{
		this->SetState(IG_STATE_EMPTY);
		return;
	}

	this->GCImperialGuardianInterfaceSend();

	if(this->m_RemainTime > 0 && this->m_RemainTime <= 30000 && this->m_TimeCount == 0)
	{
		PMSG_TIME_COUNT_SEND pMsg;

		pMsg.header.set(0x92,sizeof(pMsg));

		pMsg.type = 14;

		this->DataSendToAll((BYTE*)&pMsg,pMsg.header.size);

		this->m_TimeCount = 1;
	}

	if(this->m_RemainTime <= 0)
	{
		this->SetState(IG_STATE_PLAY4);
	}

	#endif
}

void CImperialGuardian::ProcState_PLAY4() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	this->CheckUser();

	if(this->GetUserCount() == 0)
	{
		this->SetState(IG_STATE_EMPTY);
		return;
	}

	this->GCImperialGuardianInterfaceSend();

	if(this->m_Level == 3 && OBJECT_RANGE(this->m_SideGateIndex) == 0 && this->m_CurMonster >= this->m_MaxMonster)
	{
		this->SetState(IG_STATE_CLEAN);
		return;
	}

	if(this->m_RemainTime > 0 && this->m_RemainTime <= 30000 && this->m_TimeCount == 0)
	{
		PMSG_TIME_COUNT_SEND pMsg;

		pMsg.header.set(0x92,sizeof(pMsg));

		pMsg.type = 15;

		this->DataSendToAll((BYTE*)&pMsg,pMsg.header.size);

		this->m_TimeCount = 1;
	}

	if(this->m_RemainTime <= 0)
	{
		this->SetState(IG_STATE_CLEAN);
	}

	#endif
}

void CImperialGuardian::ProcState_CLEAN() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	this->CheckUser();

	this->GCImperialGuardianInterfaceSend();

	if(this->m_RemainTime > 0 && this->m_RemainTime <= 30000 && this->m_TimeCount == 0)
	{
		PMSG_TIME_COUNT_SEND pMsg;

		pMsg.header.set(0x92,sizeof(pMsg));

		pMsg.type = 16;

		this->DataSendToAll((BYTE*)&pMsg,pMsg.header.size);

		this->m_TimeCount = 1;
	}

	if(this->m_RemainTime <= 0)
	{
		this->SetState(IG_STATE_EMPTY);
	}

	#endif
}

void CImperialGuardian::SetState(int state) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	this->m_State = state;

	switch(this->m_State)
	{
		case IG_STATE_EMPTY:
			this->SetState_EMPTY();
			break;
		case IG_STATE_WAIT1:
			this->SetState_WAIT1();
			break;
		case IG_STATE_PLAY1:
			this->SetState_PLAY1();
			break;
		case IG_STATE_WAIT2:
			this->SetState_WAIT2();
			break;
		case IG_STATE_PLAY2:
			this->SetState_PLAY2();
			break;
		case IG_STATE_WAIT3:
			this->SetState_WAIT3();
			break;
		case IG_STATE_PLAY3:
			this->SetState_PLAY3();
			break;
		case IG_STATE_WAIT4:
			this->SetState_WAIT4();
			break;
		case IG_STATE_PLAY4:
			this->SetState_PLAY4();
			break;
		case IG_STATE_CLEAN:
			this->SetState_CLEAN();
			break;
	}

	#endif
}

void CImperialGuardian::SetState_EMPTY() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	this->m_RemainTime = 0;
	this->m_TargetTime = 0;
	this->m_TickCount = GetTickCount();
	this->m_TimeCount = 0;
	this->m_Stage = 0;
	this->m_CurMonster = 0;
	this->m_MaxMonster = 0;
	this->m_PartyCount = 0;
	this->m_HighestLevel = 0;

	if(OBJECT_RANGE(this->m_PartyNumber) != 0)
	{
		gParty.Destroy(this->m_PartyNumber);
		this->m_PartyNumber = -1;
	}

	this->ClearUser();

	if(OBJECT_RANGE(this->m_SideGateIndex) != 0)
	{
		gObjDel(this->m_SideGateIndex);
		this->m_SideGateIndex = -1;
	}

	if(OBJECT_RANGE(this->m_LockGateIndex) != 0)
	{
		gObjDel(this->m_LockGateIndex);
		this->m_LockGateIndex = -1;
	}

	if(OBJECT_RANGE(this->m_MainGateIndex) != 0)
	{
		gObjDel(this->m_MainGateIndex);
		this->m_MainGateIndex = -1;
	}

	if(OBJECT_RANGE(this->m_SideBossIndex) != 0)
	{
		gObjDel(this->m_SideBossIndex);
		this->m_SideBossIndex = -1;
	}

	if(OBJECT_RANGE(this->m_MainBossIndex) != 0)
	{
		gObjDel(this->m_MainBossIndex);
		this->m_MainBossIndex = -1;
	}

	this->ClearMonster();

	this->ClearMonsterTrap();

	#endif
}

void CImperialGuardian::SetState_WAIT1() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	this->m_TimeCount = 0;
	this->m_Stage = 0;
	this->m_CurMonster = 0;
	this->m_MaxMonster = 0;

	this->SetSideGate();

	this->SetLockGate();

	this->SetMainGate();

	this->SetMonsterTrap(523);

	this->GCImperialGuardianStateSend(0,-1);

	this->m_RemainTime = this->m_NotifyTime*60000;

	this->m_TargetTime = (int)(time(0)+(this->m_RemainTime/1000));

	#endif
}

void CImperialGuardian::SetState_PLAY1() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	this->m_TimeCount = 0;
	this->m_Stage = 0;
	this->m_CurMonster = 0;
	this->m_MaxMonster = 0;

	if(OBJECT_RANGE(this->m_SideGateIndex) != 0)
	{
		gEffectManager.DelEffect(&gObj[this->m_SideGateIndex],EFFECT_MONSTER_PHYSI_DAMAGE_IMMUNITY);

		gEffectManager.DelEffect(&gObj[this->m_SideGateIndex],EFFECT_MONSTER_MAGIC_DAMAGE_IMMUNITY);
	}

	if(this->m_Level == 3)
	{
		this->SetSideBoss(507);

		this->SetMonster(518);

		this->SetMonster(518);

		this->SetMonster(518);

		this->SetMonster(519);

		this->SetMonster(519);

		this->SetMonster(519);

		this->SetMonster(521);

		this->SetMonster(521);

		this->SetMonster(521);

		this->m_MaxMonster = 10;
	}
	else
	{
		this->SetMonster(512+(this->m_Level*2));

		this->SetMonster(519);

		this->SetMonster(519);

		this->SetMonster(519);

		this->SetMonster(520);

		this->SetMonster(520);

		this->SetMonster(520);

		this->m_MaxMonster = 7;
	}

	this->m_RemainTime = this->m_EventTime*60000;

	this->m_TargetTime = (int)(time(0)+(this->m_RemainTime/1000));

	#endif
}

void CImperialGuardian::SetState_WAIT2() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	this->m_TimeCount = 0;
	this->m_Stage = 1;
	this->m_CurMonster = 0;
	this->m_MaxMonster = 0;

	this->SetSideGate();

	this->SetLockGate();

	this->SetMainGate();

	this->GCImperialGuardianStateSend(0,-1);

	this->m_RemainTime = this->m_NotifyTime*60000;

	this->m_TargetTime = (int)(time(0)+(this->m_RemainTime/1000));

	#endif
}

void CImperialGuardian::SetState_PLAY2() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	this->m_TimeCount = 0;
	this->m_Stage = 1;
	this->m_CurMonster = 0;
	this->m_MaxMonster = 0;

	if(OBJECT_RANGE(this->m_SideGateIndex) != 0)
	{
		gEffectManager.DelEffect(&gObj[this->m_SideGateIndex],EFFECT_MONSTER_PHYSI_DAMAGE_IMMUNITY);

		gEffectManager.DelEffect(&gObj[this->m_SideGateIndex],EFFECT_MONSTER_MAGIC_DAMAGE_IMMUNITY);
	}

	if(this->m_Level == 3)
	{
		this->SetSideBoss(506);

		this->SetMonster(518);

		this->SetMonster(518);

		this->SetMonster(518);

		this->SetMonster(519);

		this->SetMonster(519);

		this->SetMonster(519);

		this->SetMonster(521);

		this->SetMonster(521);

		this->SetMonster(521);

		this->m_MaxMonster = 10;
	}
	else
	{
		this->SetMonster(513+(this->m_Level*2));

		this->SetMonster(518);

		this->SetMonster(518);

		this->SetMonster(518);

		this->SetMonster(519);

		this->SetMonster(519);

		this->SetMonster(519);

		this->m_MaxMonster = 7;
	}

	this->m_RemainTime = this->m_EventTime*60000;

	this->m_TargetTime = (int)(time(0)+(this->m_RemainTime/1000));

	#endif
}

void CImperialGuardian::SetState_WAIT3() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	this->m_TimeCount = 0;
	this->m_Stage = 2;
	this->m_CurMonster = 0;
	this->m_MaxMonster = 0;

	this->SetSideGate();

	this->SetLockGate();

	this->SetMainGate();

	this->GCImperialGuardianStateSend(0,-1);

	this->m_RemainTime = this->m_NotifyTime*60000;

	this->m_TargetTime = (int)(time(0)+(this->m_RemainTime/1000));

	#endif
}

void CImperialGuardian::SetState_PLAY3() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	this->m_TimeCount = 0;
	this->m_Stage = 2;
	this->m_CurMonster = 0;
	this->m_MaxMonster = 0;

	if(OBJECT_RANGE(this->m_SideGateIndex) != 0)
	{
		gEffectManager.DelEffect(&gObj[this->m_SideGateIndex],EFFECT_MONSTER_PHYSI_DAMAGE_IMMUNITY);

		gEffectManager.DelEffect(&gObj[this->m_SideGateIndex],EFFECT_MONSTER_MAGIC_DAMAGE_IMMUNITY);
	}

	if(this->m_Level == 3)
	{
		this->SetSideBoss(505);

		this->SetMonster(518);

		this->SetMonster(518);

		this->SetMonster(518);

		this->SetMonster(519);

		this->SetMonster(519);

		this->SetMonster(519);

		this->SetMonster(521);

		this->SetMonster(521);

		this->SetMonster(521);

		this->m_MaxMonster = 10;
	}
	else
	{
		switch(this->m_DayOfWeek)
		{
			case 1:
				this->SetSideBoss(508);
				break;
			case 2:
				this->SetSideBoss(509);
				break;
			case 3:
				this->SetSideBoss(510);
				break;
			case 4:
				this->SetSideBoss(511);
				break;
			case 5:
				this->SetSideBoss(507);
				break;
			case 6:
				this->SetSideBoss(506);
				break;
		}

		this->SetMonster(519);

		this->SetMonster(519);

		this->SetMonster(519);

		this->SetMonster(521);

		this->SetMonster(521);

		this->SetMonster(521);

		this->m_MaxMonster = 7;
	}

	this->m_RemainTime = this->m_EventTime*60000;

	this->m_TargetTime = (int)(time(0)+(this->m_RemainTime/1000));

	#endif
}

void CImperialGuardian::SetState_WAIT4() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	this->m_TimeCount = 0;
	this->m_Stage = 3;
	this->m_CurMonster = 0;
	this->m_MaxMonster = 0;

	this->SetSideGate();

	this->SetLockGate();

	this->SetMainGate();

	this->SetSideGateZone();

	this->SetLockGateZone();

	this->SetMainGateZone();

	this->GCImperialGuardianStateSend(0,-1);

	this->m_RemainTime = this->m_NotifyTime*60000;

	this->m_TargetTime = (int)(time(0)+(this->m_RemainTime/1000));

	#endif
}

void CImperialGuardian::SetState_PLAY4() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	this->m_TimeCount = 0;
	this->m_Stage = 3;
	this->m_CurMonster = 0;
	this->m_MaxMonster = 0;

	if(OBJECT_RANGE(this->m_SideGateIndex) != 0)
	{
		gEffectManager.DelEffect(&gObj[this->m_SideGateIndex],EFFECT_MONSTER_PHYSI_DAMAGE_IMMUNITY);

		gEffectManager.DelEffect(&gObj[this->m_SideGateIndex],EFFECT_MONSTER_MAGIC_DAMAGE_IMMUNITY);
	}

	if(this->m_Level == 3)
	{
		this->SetMainBoss(504);

		this->SetMonster(518);

		this->SetMonster(518);

		this->SetMonster(518);

		this->SetMonster(519);

		this->SetMonster(519);

		this->SetMonster(519);

		this->SetMonster(521);

		this->SetMonster(521);

		this->SetMonster(521);

		this->SetMonster(521);

		this->m_MaxMonster = 11;
	}

	this->m_RemainTime = this->m_EventTime*60000;

	this->m_TargetTime = (int)(time(0)+(this->m_RemainTime/1000));

	#endif
}

void CImperialGuardian::SetState_CLEAN() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	this->m_TimeCount = 0;

	for(int n=0;n < MAX_IG_USER;n++)
	{
		if(OBJECT_RANGE(this->m_User[n].Index) != 0)
		{
			if(this->m_Level == 3)
			{
				if(this->m_Stage == 3 && this->m_CurMonster >= this->m_MaxMonster)
				{
					this->GiveUserRewardExperience(this->m_User[n].Index);

					this->GCImperialGuardianResultSend(this->m_User[n].Index,2,this->m_User[n].RewardExperience);
				}
				else
				{
					this->GCImperialGuardianResultSend(this->m_User[n].Index,0,this->m_User[n].RewardExperience);
				}
			}
			else
			{
				if(this->m_Stage == 2 && this->m_CurMonster >= this->m_MaxMonster)
				{
					this->GiveUserRewardExperience(this->m_User[n].Index);

					this->GCImperialGuardianResultSend(this->m_User[n].Index,2,this->m_User[n].RewardExperience);
				}
				else
				{
					this->GCImperialGuardianResultSend(this->m_User[n].Index,0,this->m_User[n].RewardExperience);
				}
			}
		}
	}

	if(OBJECT_RANGE(this->m_SideGateIndex) != 0)
	{
		gObjDel(this->m_SideGateIndex);
		this->m_SideGateIndex = -1;
	}

	if(OBJECT_RANGE(this->m_LockGateIndex) != 0)
	{
		gObjDel(this->m_LockGateIndex);
		this->m_LockGateIndex = -1;
	}

	if(OBJECT_RANGE(this->m_MainGateIndex) != 0)
	{
		gObjDel(this->m_MainGateIndex);
		this->m_MainGateIndex = -1;
	}

	if(OBJECT_RANGE(this->m_SideBossIndex) != 0)
	{
		gObjDel(this->m_SideBossIndex);
		this->m_SideBossIndex = -1;
	}

	if(OBJECT_RANGE(this->m_MainBossIndex) != 0)
	{
		gObjDel(this->m_MainBossIndex);
		this->m_MainBossIndex = -1;
	}

	this->ClearMonster();

	this->ClearMonsterTrap();

	this->m_RemainTime = this->m_CloseTime*60000;

	this->m_TargetTime = (int)(time(0)+(this->m_RemainTime/1000));

	#endif
}

int CImperialGuardian::GetState() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	return this->m_State;

	#else

	return 0;

	#endif
}

bool CImperialGuardian::AddUser(int aIndex) // OK
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

	for(int n=0;n < MAX_IG_USER;n++)
	{
		if(OBJECT_RANGE(this->m_User[n].Index) != 0)
		{
			continue;
		}

		this->m_User[n].Index = aIndex;
		this->m_User[n].RewardExperience = 0;
		return 1;
	}

	return 0;

	#else

	return 0;

	#endif
}

bool CImperialGuardian::DelUser(int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	if(OBJECT_RANGE(aIndex) == 0)
	{
		return 0;
	}

	IMPERIAL_GUARDIAN_USER* lpUser = this->GetUser(aIndex);

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
	lpUser->RewardExperience = 0;
	return 1;

	#else

	return 0;

	#endif
}

IMPERIAL_GUARDIAN_USER* CImperialGuardian::GetUser(int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	if(OBJECT_RANGE(aIndex) == 0)
	{
		return 0;
	}

	for(int n=0;n < MAX_IG_USER;n++)
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

void CImperialGuardian::CleanUser() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	for(int n=0;n < MAX_IG_USER;n++)
	{
		this->m_User[n].Index = -1;
		this->m_User[n].RewardExperience = 0;
	}

	#endif
}

void CImperialGuardian::ClearUser() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	for(int n=0;n < MAX_IG_USER;n++)
	{
		if(OBJECT_RANGE(this->m_User[n].Index) == 0)
		{
			continue;
		}

		gObjMoveGate(this->m_User[n].Index,22);

		this->m_User[n].Index = -1;
		this->m_User[n].RewardExperience = 0;
	}

	#endif
}

void CImperialGuardian::CheckUser() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	for(int n=0;n < MAX_IG_USER;n++)
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

int CImperialGuardian::GetUserCount() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	int count = 0;

	for(int n=0;n < MAX_IG_USER;n++)
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

void CImperialGuardian::GiveUserRewardExperience(int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	IMPERIAL_GUARDIAN_USER* lpUser = this->GetUser(aIndex);

	if(lpUser == 0)
	{
		return;
	}

	if(this->m_Level == 3)
	{
		lpUser->RewardExperience = 15000*((this->m_HighestLevel-1)/10);
	}
	else
	{
		lpUser->RewardExperience = 10000*((this->m_HighestLevel-1)/10);
	}

	lpUser->RewardExperience = lpUser->RewardExperience*gServerInfo.m_AddEventExperienceRate[gObj[lpUser->Index].AccountLevel];

	if(gEffectManager.CheckEffect(&gObj[lpUser->Index],EFFECT_SEAL_OF_SUSTENANCE1) != 0 || gEffectManager.CheckEffect(&gObj[lpUser->Index],EFFECT_SEAL_OF_SUSTENANCE2) != 0)
	{
		return;
	}

	if(gObjectManager.CharacterLevelUp(&gObj[lpUser->Index],lpUser->RewardExperience,gServerInfo.m_MaxLevelUpEvent,EXPERIENCE_IMPERIAL_GUARDIAN) != 0)
	{
		return;
	}

	GCRewardExperienceSend(lpUser->Index,lpUser->RewardExperience);

	#endif
}

bool CImperialGuardian::AddMonster(int aIndex) // OK
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

	for(int n=0;n < MAX_IG_MONSTER;n++)
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

bool CImperialGuardian::DelMonster(int aIndex) // OK
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

int* CImperialGuardian::GetMonster(int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	if(OBJECT_RANGE(aIndex) == 0)
	{
		return 0;
	}

	for(int n=0;n < MAX_IG_MONSTER;n++)
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

void CImperialGuardian::CleanMonster() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	for(int n=0;n < MAX_IG_MONSTER;n++)
	{
		this->m_MonsterIndex[n] = -1;
	}

	#endif
}

void CImperialGuardian::ClearMonster() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	for(int n=0;n < MAX_IG_MONSTER;n++)
	{
		if(OBJECT_RANGE(this->m_MonsterIndex[n]) != 0)
		{
			gObjDel(this->m_MonsterIndex[n]);
			this->m_MonsterIndex[n] = -1;
		}
	}

	#endif
}

void CImperialGuardian::ClearMonsterTrap() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	for(int n=OBJECT_START_MONSTER;n < MAX_OBJECT_MONSTER;n++)
	{
		if(gObjIsConnected(n) != 0 && gObj[n].Map == this->m_Map && gObj[n].Class == 523)
		{
			gObjDel(n);
		}
	}

	#endif
}

int CImperialGuardian::GetMonsterCount() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	int count = 0;

	for(int n=0;n < MAX_IG_MONSTER;n++)
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

void CImperialGuardian::SetSideGate() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	if(OBJECT_RANGE(this->m_SideGateIndex) != 0)
	{
		return;
	}

	if(gImperialGuardianSideGatePosition[this->m_Level][this->m_Stage][0] == 0)
	{
		return;
	}

	int index = gObjAddMonster(this->m_Map);

	if(OBJECT_RANGE(index) == 0)
	{
		return;
	}

	LPOBJ lpObj = &gObj[index];

	int px = gImperialGuardianSideGatePosition[this->m_Level][this->m_Stage][0];
	int py = gImperialGuardianSideGatePosition[this->m_Level][this->m_Stage][1];

	lpObj->PosNum = -1;
	lpObj->X = px;
	lpObj->Y = py;
	lpObj->TX = px;
	lpObj->TY = py;
	lpObj->OldX = px;
	lpObj->OldY = py;
	lpObj->StartX = px;
	lpObj->StartY = py;
	lpObj->Dir = gImperialGuardianSideGatePosition[this->m_Level][this->m_Stage][2];
	lpObj->Map = this->m_Map;

	if(gObjSetMonster(index,((this->m_Level==3)?528:525)) == 0)
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
	lpObj->AttackSuccessRate = (int)this->GetMonsterAttackSuccessRate((float)lpObj->AttackSuccessRate);
	lpObj->DefenseSuccessRate = (int)this->GetMonsterDefenseSuccessRate((float)lpObj->DefenseSuccessRate);

	gEffectManager.AddEffect(lpObj,1,EFFECT_MONSTER_PHYSI_DAMAGE_IMMUNITY,0,0,0,0,0);

	gEffectManager.AddEffect(lpObj,1,EFFECT_MONSTER_MAGIC_DAMAGE_IMMUNITY,0,0,0,0,0);

	this->m_SideGateIndex = index;

	this->SetSideGateZone();

	#endif
}

void CImperialGuardian::SetLockGate() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	if(OBJECT_RANGE(this->m_LockGateIndex) != 0)
	{
		return;
	}

	if(gImperialGuardianLockGatePosition[this->m_Level][this->m_Stage][0] == 0)
	{
		return;
	}

	int index = gObjAddMonster(this->m_Map);

	if(OBJECT_RANGE(index) == 0)
	{
		return;
	}

	LPOBJ lpObj = &gObj[index];

	int px = gImperialGuardianLockGatePosition[this->m_Level][this->m_Stage][0];
	int py = gImperialGuardianLockGatePosition[this->m_Level][this->m_Stage][1];

	lpObj->PosNum = -1;
	lpObj->X = px;
	lpObj->Y = py;
	lpObj->TX = px;
	lpObj->TY = py;
	lpObj->OldX = px;
	lpObj->OldY = py;
	lpObj->StartX = px;
	lpObj->StartY = py;
	lpObj->Dir = gImperialGuardianLockGatePosition[this->m_Level][this->m_Stage][2];
	lpObj->Map = this->m_Map;

	if(gObjSetMonster(index,((this->m_Level==3)?527:524)) == 0)
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
	lpObj->AttackSuccessRate = (int)this->GetMonsterAttackSuccessRate((float)lpObj->AttackSuccessRate);
	lpObj->DefenseSuccessRate = (int)this->GetMonsterDefenseSuccessRate((float)lpObj->DefenseSuccessRate);

	gEffectManager.AddEffect(lpObj,1,EFFECT_MONSTER_PHYSI_DAMAGE_IMMUNITY,0,0,0,0,0);

	gEffectManager.AddEffect(lpObj,1,EFFECT_MONSTER_MAGIC_DAMAGE_IMMUNITY,0,0,0,0,0);

	this->m_LockGateIndex = index;

	this->SetLockGateZone();

	#endif
}

void CImperialGuardian::SetMainGate() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	if(OBJECT_RANGE(this->m_MainGateIndex) != 0)
	{
		return;
	}

	if(gImperialGuardianMainGatePosition[this->m_Level][this->m_Stage][0] == 0)
	{
		return;
	}

	int index = gObjAddMonster(this->m_Map);

	if(OBJECT_RANGE(index) == 0)
	{
		return;
	}

	LPOBJ lpObj = &gObj[index];

	int px = gImperialGuardianMainGatePosition[this->m_Level][this->m_Stage][0];
	int py = gImperialGuardianMainGatePosition[this->m_Level][this->m_Stage][1];

	lpObj->PosNum = -1;
	lpObj->X = px;
	lpObj->Y = py;
	lpObj->TX = px;
	lpObj->TY = py;
	lpObj->OldX = px;
	lpObj->OldY = py;
	lpObj->StartX = px;
	lpObj->StartY = py;
	lpObj->Dir = gImperialGuardianMainGatePosition[this->m_Level][this->m_Stage][2];
	lpObj->Map = this->m_Map;

	if(gObjSetMonster(index,((this->m_Level==3)?528:525)) == 0)
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
	lpObj->AttackSuccessRate = (int)this->GetMonsterAttackSuccessRate((float)lpObj->AttackSuccessRate);
	lpObj->DefenseSuccessRate = (int)this->GetMonsterDefenseSuccessRate((float)lpObj->DefenseSuccessRate);

	gEffectManager.AddEffect(lpObj,1,EFFECT_MONSTER_PHYSI_DAMAGE_IMMUNITY,0,0,0,0,0);

	gEffectManager.AddEffect(lpObj,1,EFFECT_MONSTER_MAGIC_DAMAGE_IMMUNITY,0,0,0,0,0);

	this->m_MainGateIndex = index;

	this->SetMainGateZone();

	#endif
}

void CImperialGuardian::SetSideBoss(int MonsterClass) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	if(OBJECT_RANGE(this->m_SideBossIndex) != 0)
	{
		return;
	}

	int index = gObjAddMonster(this->m_Map);

	if(OBJECT_RANGE(index) == 0)
	{
		return;
	}

	LPOBJ lpObj = &gObj[index];

	int px = gImperialGuardianRespawnZone[this->m_Level][this->m_Stage][0];
	int py = gImperialGuardianRespawnZone[this->m_Level][this->m_Stage][1];

	if(gObjGetRandomFreeLocation(this->m_Map,&px,&py,5,5,10) == 0)
	{
		px = gImperialGuardianRespawnZone[this->m_Level][this->m_Stage][0];
		py = gImperialGuardianRespawnZone[this->m_Level][this->m_Stage][1];
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
	lpObj->AttackSuccessRate = (int)this->GetMonsterAttackSuccessRate((float)lpObj->AttackSuccessRate);
	lpObj->DefenseSuccessRate = (int)this->GetMonsterDefenseSuccessRate((float)lpObj->DefenseSuccessRate);
	lpObj->BasicAI = ((lpObj->Class==519)?this->m_SupportMonsterAIUnitNumber:this->m_DefaultMonsterAIUnitNumber);
	lpObj->CurrentAI = ((lpObj->Class==519)?this->m_SupportMonsterAIUnitNumber:this->m_DefaultMonsterAIUnitNumber);
	lpObj->CurrentAIState = 0;
	lpObj->LastAIRunTime = 0;
	lpObj->GroupNumber = 0;
	lpObj->SubGroupNumber = 0;
	lpObj->GroupMemberGuid = -1;
	lpObj->RegenType = 0;

	this->m_SideBossIndex = index;

	#endif
}

void CImperialGuardian::SetMainBoss(int MonsterClass) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	if(OBJECT_RANGE(this->m_MainBossIndex) != 0)
	{
		return;
	}

	int index = gObjAddMonster(this->m_Map);

	if(OBJECT_RANGE(index) == 0)
	{
		return;
	}

	LPOBJ lpObj = &gObj[index];

	int px = gImperialGuardianRespawnZone[this->m_Level][this->m_Stage][0];
	int py = gImperialGuardianRespawnZone[this->m_Level][this->m_Stage][1];

	if(gObjGetRandomFreeLocation(this->m_Map,&px,&py,5,5,10) == 0)
	{
		px = gImperialGuardianRespawnZone[this->m_Level][this->m_Stage][0];
		py = gImperialGuardianRespawnZone[this->m_Level][this->m_Stage][1];
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
	lpObj->AttackSuccessRate = (int)this->GetMonsterAttackSuccessRate((float)lpObj->AttackSuccessRate);
	lpObj->DefenseSuccessRate = (int)this->GetMonsterDefenseSuccessRate((float)lpObj->DefenseSuccessRate);
	lpObj->BasicAI = ((lpObj->Class==519)?this->m_SupportMonsterAIUnitNumber:this->m_DefaultMonsterAIUnitNumber);
	lpObj->CurrentAI = ((lpObj->Class==519)?this->m_SupportMonsterAIUnitNumber:this->m_DefaultMonsterAIUnitNumber);
	lpObj->CurrentAIState = 0;
	lpObj->LastAIRunTime = 0;
	lpObj->GroupNumber = 0;
	lpObj->SubGroupNumber = 0;
	lpObj->GroupMemberGuid = -1;
	lpObj->RegenType = 0;

	this->m_MainBossIndex = index;

	#endif
}

void CImperialGuardian::SetMonster(int MonsterClass) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	int index = gObjAddMonster(this->m_Map);

	if(OBJECT_RANGE(index) == 0)
	{
		return;
	}

	LPOBJ lpObj = &gObj[index];

	int px = gImperialGuardianRespawnZone[this->m_Level][this->m_Stage][0];
	int py = gImperialGuardianRespawnZone[this->m_Level][this->m_Stage][1];

	if(gObjGetRandomFreeLocation(this->m_Map,&px,&py,5,5,10) == 0)
	{
		px = gImperialGuardianRespawnZone[this->m_Level][this->m_Stage][0];
		py = gImperialGuardianRespawnZone[this->m_Level][this->m_Stage][1];
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
	lpObj->AttackSuccessRate = (int)this->GetMonsterAttackSuccessRate((float)lpObj->AttackSuccessRate);
	lpObj->DefenseSuccessRate = (int)this->GetMonsterDefenseSuccessRate((float)lpObj->DefenseSuccessRate);
	lpObj->BasicAI = ((lpObj->Class==519)?this->m_SupportMonsterAIUnitNumber:this->m_DefaultMonsterAIUnitNumber);
	lpObj->CurrentAI = ((lpObj->Class==519)?this->m_SupportMonsterAIUnitNumber:this->m_DefaultMonsterAIUnitNumber);
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

void CImperialGuardian::SetMonsterTrap(int MonsterClass) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	for(int n=0;n < gMonsterSetBase.m_count;n++)
	{
		MONSTER_SET_BASE_INFO* lpInfo = &gMonsterSetBase.m_MonsterSetBaseInfo[n];

		if(lpInfo->Type != 4 || lpInfo->MonsterClass != MonsterClass || lpInfo->Map != this->m_Map)
		{
			continue;
		}

		int index = gObjAddMonster(this->m_Map);

		if(OBJECT_RANGE(index) == 0)
		{
			continue;
		}

		LPOBJ lpObj = &gObj[index];

		if(gObjSetPosMonster(index,n) == 0)
		{
			gObjDel(index);
			continue;
		}

		if(gObjSetMonster(index,lpInfo->MonsterClass) == 0)
		{
			gObjDel(index);
			continue;
		}

		lpObj->Level = (int)this->GetMonsterLevel((float)lpObj->Level);
		lpObj->Life = (float)this->GetMonsterLife((float)lpObj->Life);
		lpObj->MaxLife = (float)this->GetMonsterLife((float)lpObj->MaxLife);
		lpObj->ScriptMaxLife = (float)this->GetMonsterLife((float)lpObj->ScriptMaxLife);
		lpObj->PhysiDamageMin = (int)this->GetMonsterDamageMin((float)lpObj->PhysiDamageMin);
		lpObj->PhysiDamageMax = (int)this->GetMonsterDamageMax((float)lpObj->PhysiDamageMax);
		lpObj->Defense = (int)this->GetMonsterDefense((float)lpObj->Defense);
		lpObj->AttackSuccessRate = (int)this->GetMonsterAttackSuccessRate((float)lpObj->AttackSuccessRate);
		lpObj->DefenseSuccessRate = (int)this->GetMonsterDefenseSuccessRate((float)lpObj->DefenseSuccessRate);
	}

	#endif
}

void CImperialGuardian::NpcJerryTheAdviser(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	if(gServerInfo.m_ImperialGuardianEvent == 0)
	{
		return;
	}

	if(lpObj->Level < 15)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(305));
		return;
	}

	PMSG_NPC_TALK_SEND pMsg;

	pMsg.header.set(0x30,sizeof(pMsg));

	pMsg.result = 36;

	DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CImperialGuardian::MoveGate(LPOBJ lpObj,int gate) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	if(this->m_Map != lpObj->Map)
	{
		return;
	}

	if(this->GetUser(lpObj->Index) == 0)
	{
		return;
	}

	if(gate < (308+(this->m_Level*5)) || gate > (((this->m_Level==3)?313:311)+(this->m_Level*5)))
	{
		return;
	}

	lpObj->Interface.use = 0;
	lpObj->Interface.type = INTERFACE_NONE;
	lpObj->Interface.state = 0;

	gObjMoveGate(lpObj->Index,gate);

	if(((308+(this->m_Level*5))+(this->m_Stage*2)) == gate)
	{
		this->SetState(IG_STATE_WAIT2+(this->m_Stage*2));
		return;
	}

	#endif
}

void CImperialGuardian::DropSideBossItem(LPOBJ lpObj,LPOBJ lpTarget) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	if(gServerInfo.m_ImperialGuardianEvent == 0)
	{
		return;
	}

	if(this->m_Map != lpObj->Map)
	{
		return;
	}

	for(int n=0;n < MAX_IG_USER;n++)
	{
		if(OBJECT_RANGE(this->m_User[n].Index) != 0 && this->m_DayOfWeek != 0)
		{
			GDCreateItemSend(lpTarget->Index,lpObj->Map,(BYTE)lpObj->X,(BYTE)lpObj->Y,(GET_ITEM(14,102)+this->m_DayOfWeek),0,0,0,0,0,this->m_User[n].Index,0,0,0,0,0,0xFF,0);
		}
	}

	switch(((this->m_HighestLevel-1)/100))
	{
		case 0:
			gItemBagManager.DropItemBySpecialValue(ITEM_BAG_IMPERIAL_GUARDIAN_SIDE_BOSS1,lpTarget,lpObj->Map,lpObj->X,lpObj->Y);
			break;
		case 1:
			gItemBagManager.DropItemBySpecialValue(ITEM_BAG_IMPERIAL_GUARDIAN_SIDE_BOSS2,lpTarget,lpObj->Map,lpObj->X,lpObj->Y);
			break;
		case 2:
			gItemBagManager.DropItemBySpecialValue(ITEM_BAG_IMPERIAL_GUARDIAN_SIDE_BOSS3,lpTarget,lpObj->Map,lpObj->X,lpObj->Y);
			break;
		case 3:
			gItemBagManager.DropItemBySpecialValue(ITEM_BAG_IMPERIAL_GUARDIAN_SIDE_BOSS4,lpTarget,lpObj->Map,lpObj->X,lpObj->Y);
			break;
		default:
			gItemBagManager.DropItemBySpecialValue(ITEM_BAG_IMPERIAL_GUARDIAN_SIDE_BOSS5,lpTarget,lpObj->Map,lpObj->X,lpObj->Y);
			break;
	}

	#endif
}

void CImperialGuardian::DropMainBossItem(LPOBJ lpObj,LPOBJ lpTarget) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	if(gServerInfo.m_ImperialGuardianEvent == 0)
	{
		return;
	}

	if(this->m_Map != lpObj->Map)
	{
		return;
	}

	switch(((this->m_HighestLevel-1)/100))
	{
		case 0:
			gItemBagManager.DropItemBySpecialValue(ITEM_BAG_IMPERIAL_GUARDIAN_MAIN_BOSS1,lpTarget,lpObj->Map,lpObj->X,lpObj->Y);
			break;
		case 1:
			gItemBagManager.DropItemBySpecialValue(ITEM_BAG_IMPERIAL_GUARDIAN_MAIN_BOSS2,lpTarget,lpObj->Map,lpObj->X,lpObj->Y);
			break;
		case 2:
			gItemBagManager.DropItemBySpecialValue(ITEM_BAG_IMPERIAL_GUARDIAN_MAIN_BOSS3,lpTarget,lpObj->Map,lpObj->X,lpObj->Y);
			break;
		case 3:
			gItemBagManager.DropItemBySpecialValue(ITEM_BAG_IMPERIAL_GUARDIAN_MAIN_BOSS4,lpTarget,lpObj->Map,lpObj->X,lpObj->Y);
			break;
		default:
			gItemBagManager.DropItemBySpecialValue(ITEM_BAG_IMPERIAL_GUARDIAN_MAIN_BOSS5,lpTarget,lpObj->Map,lpObj->X,lpObj->Y);
			break;
	}

	#endif
}

float CImperialGuardian::GetMonsterLevel(float value) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	float MonsterLevel = (float)this->m_MonsterDifficultInfo[((this->m_HighestLevel-1)/10)].Level;

	return MonsterLevel;

	#else

	return 0;

	#endif
}

float CImperialGuardian::GetMonsterLife(float value) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	float MonsterLife = value*this->m_MonsterDifficultInfo[((this->m_HighestLevel-1)/10)].Life;

	return ((MonsterLife*gServerInfo.m_ImperialGuardianDifficultRate)/100);

	#else

	return 0;

	#endif
}

float CImperialGuardian::GetMonsterDamageMin(float value) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	float MonsterDamageMin = value*this->m_MonsterDifficultInfo[((this->m_HighestLevel-1)/10)].DamageMin;

	return ((MonsterDamageMin*gServerInfo.m_ImperialGuardianDifficultRate)/100);

	#else

	return 0;

	#endif
}

float CImperialGuardian::GetMonsterDamageMax(float value) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	float MonsterDamageMax = value*this->m_MonsterDifficultInfo[((this->m_HighestLevel-1)/10)].DamageMax;

	return ((MonsterDamageMax*gServerInfo.m_ImperialGuardianDifficultRate)/100);

	#else

	return 0;

	#endif
}

float CImperialGuardian::GetMonsterDefense(float value) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	float MonsterDefense = value*this->m_MonsterDifficultInfo[((this->m_HighestLevel-1)/10)].Defense;

	return ((MonsterDefense*gServerInfo.m_ImperialGuardianDifficultRate)/100);

	#else

	return 0;

	#endif
}

float CImperialGuardian::GetMonsterAttackSuccessRate(float value) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	float MonsterAttackSuccessRate = value*this->m_MonsterDifficultInfo[((this->m_HighestLevel-1)/10)].AttackSuccessRate;

	return ((MonsterAttackSuccessRate*gServerInfo.m_ImperialGuardianDifficultRate)/100);

	#else

	return 0;

	#endif
}

float CImperialGuardian::GetMonsterDefenseSuccessRate(float value) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	float MonsterDefenseSuccessRate = value*this->m_MonsterDifficultInfo[((this->m_HighestLevel-1)/10)].DefenseSuccessRate;

	return ((MonsterDefenseSuccessRate*gServerInfo.m_ImperialGuardianDifficultRate)/100);

	#else

	return 0;

	#endif
}

void CImperialGuardian::SetSideGateZone() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	for(int x=gImperialGuardianSideGateZone[this->m_Level][this->m_Stage][0];x <= gImperialGuardianSideGateZone[this->m_Level][this->m_Stage][2];x++)
	{
		for(int y=gImperialGuardianSideGateZone[this->m_Level][this->m_Stage][1];y <= gImperialGuardianSideGateZone[this->m_Level][this->m_Stage][3];y++)
		{
			gMap[this->m_Map].SetAttr(x,y,16);
		}
	}

	for(int n=0;n < MAX_IG_USER;n++)
	{
		if(OBJECT_RANGE(this->m_User[n].Index) != 0)
		{
			GCMapAttrSend(this->m_User[n].Index,0,16,0,1,(PMSG_MAP_ATTR*)gImperialGuardianSideGateZone[this->m_Level][this->m_Stage]);
		}
	}

	#endif
}

void CImperialGuardian::SetLockGateZone() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	for(int x=gImperialGuardianLockGateZone[this->m_Level][this->m_Stage][0];x <= gImperialGuardianLockGateZone[this->m_Level][this->m_Stage][2];x++)
	{
		for(int y=gImperialGuardianLockGateZone[this->m_Level][this->m_Stage][1];y <= gImperialGuardianLockGateZone[this->m_Level][this->m_Stage][3];y++)
		{
			gMap[this->m_Map].SetAttr(x,y,16);
		}
	}

	for(int n=0;n < MAX_IG_USER;n++)
	{
		if(OBJECT_RANGE(this->m_User[n].Index) != 0)
		{
			GCMapAttrSend(this->m_User[n].Index,0,16,0,1,(PMSG_MAP_ATTR*)gImperialGuardianLockGateZone[this->m_Level][this->m_Stage]);
		}
	}

	#endif
}

void CImperialGuardian::SetMainGateZone() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	for(int x=gImperialGuardianMainGateZone[this->m_Level][this->m_Stage][0];x <= gImperialGuardianMainGateZone[this->m_Level][this->m_Stage][2];x++)
	{
		for(int y=gImperialGuardianMainGateZone[this->m_Level][this->m_Stage][1];y <= gImperialGuardianMainGateZone[this->m_Level][this->m_Stage][3];y++)
		{
			gMap[this->m_Map].SetAttr(x,y,16);
		}
	}

	for(int n=0;n < MAX_IG_USER;n++)
	{
		if(OBJECT_RANGE(this->m_User[n].Index) != 0)
		{
			GCMapAttrSend(this->m_User[n].Index,0,16,0,1,(PMSG_MAP_ATTR*)gImperialGuardianMainGateZone[this->m_Level][this->m_Stage]);
		}
	}

	#endif
}

void CImperialGuardian::DelSideGateZone() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	for(int x=gImperialGuardianSideGateZone[this->m_Level][this->m_Stage][0];x <= gImperialGuardianSideGateZone[this->m_Level][this->m_Stage][2];x++)
	{
		for(int y=gImperialGuardianSideGateZone[this->m_Level][this->m_Stage][1];y <= gImperialGuardianSideGateZone[this->m_Level][this->m_Stage][3];y++)
		{
			gMap[this->m_Map].DelAttr(x,y,16);
		}
	}

	for(int n=0;n < MAX_IG_USER;n++)
	{
		if(OBJECT_RANGE(this->m_User[n].Index) != 0)
		{
			GCMapAttrSend(this->m_User[n].Index,0,16,1,1,(PMSG_MAP_ATTR*)gImperialGuardianSideGateZone[this->m_Level][this->m_Stage]);
		}
	}

	#endif
}

void CImperialGuardian::DelLockGateZone() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	for(int x=gImperialGuardianLockGateZone[this->m_Level][this->m_Stage][0];x <= gImperialGuardianLockGateZone[this->m_Level][this->m_Stage][2];x++)
	{
		for(int y=gImperialGuardianLockGateZone[this->m_Level][this->m_Stage][1];y <= gImperialGuardianLockGateZone[this->m_Level][this->m_Stage][3];y++)
		{
			gMap[this->m_Map].DelAttr(x,y,16);
		}
	}

	for(int n=0;n < MAX_IG_USER;n++)
	{
		if(OBJECT_RANGE(this->m_User[n].Index) != 0)
		{
			GCMapAttrSend(this->m_User[n].Index,0,16,1,1,(PMSG_MAP_ATTR*)gImperialGuardianLockGateZone[this->m_Level][this->m_Stage]);
		}
	}

	#endif
}

void CImperialGuardian::DelMainGateZone() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	for(int x=gImperialGuardianMainGateZone[this->m_Level][this->m_Stage][0];x <= gImperialGuardianMainGateZone[this->m_Level][this->m_Stage][2];x++)
	{
		for(int y=gImperialGuardianMainGateZone[this->m_Level][this->m_Stage][1];y <= gImperialGuardianMainGateZone[this->m_Level][this->m_Stage][3];y++)
		{
			gMap[this->m_Map].DelAttr(x,y,16);
		}
	}

	for(int n=0;n < MAX_IG_USER;n++)
	{
		if(OBJECT_RANGE(this->m_User[n].Index) != 0)
		{
			GCMapAttrSend(this->m_User[n].Index,0,16,1,1,(PMSG_MAP_ATTR*)gImperialGuardianMainGateZone[this->m_Level][this->m_Stage]);
		}
	}

	#endif
}

void CImperialGuardian::UserDieProc(LPOBJ lpObj,LPOBJ lpTarget) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	if(this->m_State != IG_STATE_PLAY1 && this->m_State != IG_STATE_PLAY2 && this->m_State != IG_STATE_PLAY3 && this->m_State != IG_STATE_PLAY4)
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

	this->GCImperialGuardianResultSend(lpObj->Index,0,0);

	this->DelUser(lpObj->Index);

	#endif
}

void CImperialGuardian::MonsterDieProc(LPOBJ lpObj,LPOBJ lpTarget) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	int aIndex = gObjMonsterGetTopHitDamageUser(lpObj);

	if(OBJECT_RANGE(aIndex) != 0)
	{
		lpTarget = &gObj[aIndex];
	}

	if(this->m_State != IG_STATE_PLAY1 && this->m_State != IG_STATE_PLAY2 && this->m_State != IG_STATE_PLAY3 && this->m_State != IG_STATE_PLAY4)
	{
		return;
	}

	if(this->m_Map != lpObj->Map)
	{
		return;
	}

	if(lpObj->Class == 504 && lpObj->Index == this->m_MainBossIndex)
	{
		this->m_CurMonster++;
		this->m_MainBossIndex = -1;
		return;
	}

	if(lpObj->Class == 505 && lpObj->Index == this->m_SideBossIndex)
	{
		this->m_CurMonster++;
		this->m_SideBossIndex = -1;
		return;
	}

	if(lpObj->Class == 506 && lpObj->Index == this->m_SideBossIndex)
	{
		this->m_CurMonster++;
		this->m_SideBossIndex = -1;
		return;
	}

	if(lpObj->Class == 507 && lpObj->Index == this->m_SideBossIndex)
	{
		this->m_CurMonster++;
		this->m_SideBossIndex = -1;
		return;
	}

	if(lpObj->Class == 508 && lpObj->Index == this->m_SideBossIndex)
	{
		this->m_CurMonster++;
		this->m_SideBossIndex = -1;
		return;
	}

	if(lpObj->Class == 509 && lpObj->Index == this->m_SideBossIndex)
	{
		this->m_CurMonster++;
		this->m_SideBossIndex = -1;
		return;
	}

	if(lpObj->Class == 510 && lpObj->Index == this->m_SideBossIndex)
	{
		this->m_CurMonster++;
		this->m_SideBossIndex = -1;
		return;
	}

	if(lpObj->Class == 511 && lpObj->Index == this->m_SideBossIndex)
	{
		this->m_CurMonster++;
		this->m_SideBossIndex = -1;
		return;
	}

	if(lpObj->Class == 524 && lpObj->Index == this->m_LockGateIndex)
	{
		this->DelLockGateZone();
		this->m_LockGateIndex = -1;
		return;
	}

	if(lpObj->Class == 525 && lpObj->Index == this->m_SideGateIndex)
	{
		this->DelSideGateZone();
		this->m_SideGateIndex = -1;
		return;
	}

	if(lpObj->Class == 525 && lpObj->Index == this->m_MainGateIndex)
	{
		this->DelMainGateZone();
		this->m_MainGateIndex = -1;
		return;
	}

	if(lpObj->Class == 527 && lpObj->Index == this->m_LockGateIndex)
	{
		this->DelLockGateZone();
		this->m_LockGateIndex = -1;
		return;
	}

	if(lpObj->Class == 528 && lpObj->Index == this->m_SideGateIndex)
	{
		this->DelSideGateZone();
		this->m_SideGateIndex = -1;
		return;
	}

	if(lpObj->Class == 528 && lpObj->Index == this->m_MainGateIndex)
	{
		this->DelMainGateZone();
		this->m_MainGateIndex = -1;
		return;
	}

	if(this->GetMonster(lpObj->Index) == 0)
	{
		return;
	}

	this->m_CurMonster++;

	this->DelMonster(lpObj->Index);

	#endif
}

void CImperialGuardian::NoticeSendToAll(int type,char* message,...) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	char buff[256];

	va_list arg;
	va_start(arg,message);
	vsprintf_s(buff,message,arg);
	va_end(arg);

	for(int n=0;n < MAX_IG_USER;n++)
	{
		if(OBJECT_RANGE(this->m_User[n].Index) != 0)
		{
			gNotice.GCNoticeSend(this->m_User[n].Index,type,0,0,0,0,0,buff);
		}
	}

	#endif
}

void CImperialGuardian::DataSendToAll(BYTE* lpMsg,int size) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	for(int n=0;n < MAX_IG_USER;n++)
	{
		if(OBJECT_RANGE(this->m_User[n].Index) != 0)
		{
			DataSend(this->m_User[n].Index,lpMsg,size);
		}
	}

	#endif
}

void CImperialGuardian::CGImperialGuardianEnterRecv(PMSG_IMPERIAL_GUARDIAN_ENTER_RECV* lpMsg,int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	if(gServerInfo.m_ImperialGuardianEvent == 0)
	{
		return;
	}

	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	SYSTEMTIME time;

	GetLocalTime(&time);

	lpMsg->slot = gItemManager.GetInventoryItemSlot(lpObj,((time.wDayOfWeek==0)?GET_ITEM(14,109):GET_ITEM(14,102)),0);

	if(INVENTORY_FULL_RANGE(lpMsg->slot) == 0)
	{
		lpMsg->slot = gItemManager.GetInventoryItemSlot(lpObj,((time.wDayOfWeek==0)?GET_ITEM(13,127):GET_ITEM(13,126)),0);
	}

	if(INVENTORY_FULL_RANGE(lpMsg->slot) == 0)
	{
		this->GCImperialGuardianStateSend(2,aIndex);
		return;
	}

	if(this->m_State != IG_STATE_EMPTY)
	{
		this->GCImperialGuardianStateSend(4,aIndex);
		return;
	}

	if(gMapServerManager.CheckMapServer(MAP_IMPERIAL_GUARDIAN1) == 0)
	{
		this->GCImperialGuardianStateSend(4,aIndex);
		return;
	}

	if(gMapServerManager.CheckMapServer(MAP_IMPERIAL_GUARDIAN2) == 0)
	{
		this->GCImperialGuardianStateSend(4,aIndex);
		return;
	}

	if(gMapServerManager.CheckMapServer(MAP_IMPERIAL_GUARDIAN3) == 0)
	{
		this->GCImperialGuardianStateSend(4,aIndex);
		return;
	}

	if(gMapServerManager.CheckMapServer(MAP_IMPERIAL_GUARDIAN4) == 0)
	{
		this->GCImperialGuardianStateSend(4,aIndex);
		return;
	}

	if(gServerInfo.m_PKLimitFree == 0 && lpObj->PKLevel >= 4)
	{
		this->GCImperialGuardianStateSend(6,aIndex);
		return;
	}

	gItemManager.DecreaseItemDur(lpObj,lpMsg->slot,1);

	this->m_DayOfWeek = time.wDayOfWeek;

	this->m_Level = ((this->m_DayOfWeek==0)?3:((this->m_DayOfWeek-1)%3));

	this->m_Map = MAP_IMPERIAL_GUARDIAN1+this->m_Level;

	this->m_PartyNumber = lpObj->PartyNumber;

	if(OBJECT_RANGE(this->m_PartyNumber) == 0)
	{
		if(this->AddUser(aIndex) != 0)
		{
			lpObj->Interface.use = 0;
			lpObj->Interface.type = INTERFACE_NONE;
			lpObj->Interface.state = 0;

			gObjMoveGate(aIndex,(307+(this->m_Level*5)));

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

					gObjMoveGate(gParty.m_PartyInfo[this->m_PartyNumber].Index[n],(307+(this->m_Level*5)));

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

	this->SetState(IG_STATE_WAIT1);

	#endif
}

void CImperialGuardian::GCImperialGuardianStateSend(int state,int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	PMSG_IMPERIAL_GUARDIAN_STATE_SEND pMsg;

	pMsg.header.set(0xF7,0x02,sizeof(pMsg));

	pMsg.state = state;

	pMsg.level = ((state==0)?((this->m_DayOfWeek==0)?7:this->m_DayOfWeek):0);

	pMsg.stage = ((state==0)?(this->m_Stage+1):0);

	pMsg.count = ((state==0)?(((this->m_MaxMonster-this->m_CurMonster)<0)?0:(this->m_MaxMonster-this->m_CurMonster)):0);

	pMsg.time = ((state==0)?((this->m_RemainTime<0)?0:this->m_RemainTime):0);

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

void CImperialGuardian::GCImperialGuardianInterfaceSend() // OK
{
	#if(GAMESERVER_UPDATE>=501)

	PMSG_IMPERIAL_GUARDIAN_INTERFACE_SEND pMsg;

	pMsg.header.set(0xF7,0x04,sizeof(pMsg));

	pMsg.state = ((this->m_State==IG_STATE_EMPTY||this->m_State==IG_STATE_CLEAN)?0:(((this->m_State-1)%2)+1));

	pMsg.level = ((this->m_DayOfWeek==0)?7:this->m_DayOfWeek);

	pMsg.stage = (this->m_Stage+1);

	pMsg.time = ((this->m_RemainTime<0)?0:this->m_RemainTime);

	pMsg.count = (((this->m_MaxMonster-this->m_CurMonster)<0)?0:(this->m_MaxMonster-this->m_CurMonster));

	this->DataSendToAll((BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CImperialGuardian::GCImperialGuardianResultSend(int aIndex,int result,int experience) // OK
{
	#if(GAMESERVER_UPDATE>=501)

	PMSG_IMPERIAL_GUARDIAN_RESULT_SEND pMsg;

	pMsg.header.set(0xF7,0x06,sizeof(pMsg));

	pMsg.result = result;

	pMsg.experience = experience;

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);

	#endif
}
