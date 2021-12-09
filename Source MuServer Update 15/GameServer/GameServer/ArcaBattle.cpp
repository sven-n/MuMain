// ArcaBattle.cpp: implementation of the CArcaBattle class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ArcaBattle.h"
#include "GameMain.h"
#include "Guild.h"
#include "Map.h"
#include "MemScript.h"
#include "Monster.h"
#include "Notice.h"
#include "NpcTalk.h"
#include "ScheduleManager.h"
#include "Util.h"

CArcaBattle gArcaBattle;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CArcaBattle::CArcaBattle() // OK
{

}

CArcaBattle::~CArcaBattle() // OK
{

}

void CArcaBattle::Init() // OK
{
	#if(GAMESERVER_UPDATE>=702)

	this->SetState(AB_STATE_BLANK);

	#endif
}

void CArcaBattle::Load(char* path) // OK
{
	#if(GAMESERVER_UPDATE>=702)

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

	this->m_ArcaBattleStartTime.clear();

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

					ARCA_BATTLE_START_TIME info;

					info.State = lpMemScript->GetNumber();

					info.Year = lpMemScript->GetAsNumber();

					info.Month = lpMemScript->GetAsNumber();

					info.Day = lpMemScript->GetAsNumber();

					info.DayOfWeek = lpMemScript->GetAsNumber();

					info.Hour = lpMemScript->GetAsNumber();

					info.Minute = lpMemScript->GetAsNumber();

					info.Second = lpMemScript->GetAsNumber();

					this->m_ArcaBattleStartTime.push_back(info);
				}
				else if(section == 1)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					ARCA_BATTLE_OBELISK_INFO info;

					info.Group = lpMemScript->GetNumber();

					info.MonsterClass = lpMemScript->GetAsNumber();

					info.Map = lpMemScript->GetAsNumber();

					info.X = lpMemScript->GetAsNumber();

					info.Y = lpMemScript->GetAsNumber();

					info.Life = lpMemScript->GetAsNumber();

					info.OccupyLife = lpMemScript->GetAsNumber();

					this->m_ObeliskInfo[info.Group] = info;
				}
				else if(section == 2)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					ARCA_BATTLE_OBELISK_AURA_INFO info;

					info.Group = lpMemScript->GetNumber();

					info.Index = lpMemScript->GetAsNumber();

					info.Map = lpMemScript->GetAsNumber();

					info.X = lpMemScript->GetAsNumber();

					info.Y = lpMemScript->GetAsNumber();

					this->m_ObeliskInfo[info.Group].AuraInfo[info.Index] = info;
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

void CArcaBattle::MainProc() // OK
{
	#if(GAMESERVER_UPDATE>=702)

	if((GetTickCount()-this->m_TickCount) >= 1000)
	{
		this->m_TickCount = GetTickCount();

		this->m_RemainTime = (int)difftime(this->m_TargetTime,time(0));

		switch(this->m_State)
		{
			case AB_STATE_BLANK:
				this->ProcState_BLANK();
				break;
			case AB_STATE_EMPTY:
				this->ProcState_EMPTY();
				break;
			case AB_STATE_SIGN1:
				this->ProcState_SIGN1();
				break;
			case AB_STATE_SIGN2:
				this->ProcState_SIGN2();
				break;
			case AB_STATE_READY:
				this->ProcState_READY();
				break;
			case AB_STATE_PARTY:
				this->ProcState_PARTY();
				break;
			case AB_STATE_START:
				this->ProcState_START();
				break;
			case AB_STATE_CLOSE:
				this->ProcState_CLOSE();
				break;
			case AB_STATE_CLEAN:
				this->ProcState_CLEAN();
				break;
			case AB_STATE_CLEAR:
				this->ProcState_CLEAR();
				break;
		}
	}

	#endif
}

void CArcaBattle::ProcState_BLANK() // OK
{

}

void CArcaBattle::ProcState_EMPTY() // OK
{
	#if(GAMESERVER_UPDATE>=702)

	if(this->m_RemainTime <= 0)
	{
		gNotice.GCNoticeSendToAll(0,0,0,0,0,0,"[ArcaBattle] SIGN1");

		this->SetState(AB_STATE_SIGN1);
	}

	#endif
}

void CArcaBattle::ProcState_SIGN1() // OK
{
	#if(GAMESERVER_UPDATE>=702)

	//REGISTER COUNTDOWN MESSAGE

	if(this->m_RemainTime <= 0)
	{
		gNotice.GCNoticeSendToAll(0,0,0,0,0,0,"[ArcaBattle] SIGN2");

		this->SetState(AB_STATE_SIGN2);
	}

	#endif
}

void CArcaBattle::ProcState_SIGN2() // OK
{
	#if(GAMESERVER_UPDATE>=702)

	//REGISTER COUNTDOWN MESSAGE

	if(this->m_RemainTime <= 0)
	{
		gNotice.GCNoticeSendToAll(0,0,0,0,0,0,"[ArcaBattle] READY");

		this->SetState(AB_STATE_READY);
	}

	#endif
}

void CArcaBattle::ProcState_READY() // OK
{
	#if(GAMESERVER_UPDATE>=702)

	//NOTIFY SHIT

	if(this->m_RemainTime <= 0)
	{
		gNotice.GCNoticeSendToAll(0,0,0,0,0,0,"[ArcaBattle] PARTY");

		this->SetState(AB_STATE_PARTY);
	}

	#endif
}

void CArcaBattle::ProcState_PARTY() // OK
{
	#if(GAMESERVER_UPDATE>=702)

	//NOTIFY SHIT

	if(this->m_RemainTime <= 0)
	{
		gNotice.GCNoticeSendToAll(0,0,0,0,0,0,"[ArcaBattle] START");

		this->SetState(AB_STATE_START);
	}

	#endif
}

void CArcaBattle::ProcState_START() // OK
{
	#if(GAMESERVER_UPDATE>=702)

	this->ChkAuraUserHover();
	//this->RegenMonsterRun();
	this->GCArcaBattleObeliskLifeSend();
	this->GCArcaBattleObeliskStateSend();
	this->RegenObelisk();

	if(this->m_RemainTime <= 0)
	{
		gNotice.GCNoticeSendToAll(0,0,0,0,0,0,"[ArcaBattle] CLOSE");

		this->SetState(AB_STATE_CLOSE);
	}

	#endif
}

void CArcaBattle::ProcState_CLOSE() // OK
{
	#if(GAMESERVER_UPDATE>=702)

	//NOTIFY SHIT

	if(this->m_RemainTime <= 0)
	{
		gNotice.GCNoticeSendToAll(0,0,0,0,0,0,"[ArcaBattle] CLEAN");

		this->SetState(AB_STATE_CLEAN);
	}

	#endif
}

void CArcaBattle::ProcState_CLEAN() // OK
{
	//NOTIFY SHIT

	#if(GAMESERVER_UPDATE>=702)

	if(this->m_RemainTime <= 0)
	{
		gNotice.GCNoticeSendToAll(0,0,0,0,0,0,"[ArcaBattle] CLEAR");

		this->SetState(AB_STATE_CLEAR);
	}

	#endif
}

void CArcaBattle::ProcState_CLEAR() // OK
{
	#if(GAMESERVER_UPDATE>=702)

	if(this->m_RemainTime <= 0)
	{
		gNotice.GCNoticeSendToAll(0,0,0,0,0,0,"[ArcaBattle] EMPTY");

		this->SetState(AB_STATE_EMPTY);
	}

	#endif
}

void CArcaBattle::SetState(int state) // OK
{
	#if(GAMESERVER_UPDATE>=702)

	switch((this->m_State=state))
	{
		case AB_STATE_BLANK:
			this->SetState_BLANK();
			break;
		case AB_STATE_EMPTY:
			this->SetState_EMPTY();
			break;
		case AB_STATE_SIGN1:
			this->SetState_SIGN1();
			break;
		case AB_STATE_SIGN2:
			this->SetState_SIGN2();
			break;
		case AB_STATE_READY:
			this->SetState_READY();
			break;
		case AB_STATE_PARTY:
			this->SetState_PARTY();
			break;
		case AB_STATE_START:
			this->SetState_START();
			break;
		case AB_STATE_CLOSE:
			this->SetState_CLOSE();
			break;
		case AB_STATE_CLEAN:
			this->SetState_CLEAN();
			break;
		case AB_STATE_CLEAR:
			this->SetState_CLEAR();
			break;
	}

	#endif
}

void CArcaBattle::SetState_BLANK() // OK
{

}

void CArcaBattle::SetState_EMPTY() // OK
{
	#if(GAMESERVER_UPDATE>=702)

	this->GCArcaBattleStateSend((this->m_State+1),-1);

	this->CheckSync();

	#endif
}

void CArcaBattle::SetState_SIGN1() // OK
{
	#if(GAMESERVER_UPDATE>=702)


	//this->GDReqDeleteArcaBattleInfo();
	//this->ReqRemoveRewardBuffWinningGuild();

	this->GCArcaBattleStateSend((this->m_State+1),-1);

	this->CheckSync();

	#endif
}

void CArcaBattle::SetState_SIGN2() // OK
{
	#if(GAMESERVER_UPDATE>=702)

	this->GCArcaBattleStateSend((this->m_State+1),-1);

	this->CheckSync();

	#endif
}

void CArcaBattle::SetState_READY() // OK
{
	#if(GAMESERVER_UPDATE>=702)

	this->GCArcaBattleStateSend((this->m_State+1),-1);

	this->CheckSync();

	#endif
}

void CArcaBattle::SetState_PARTY() // OK
{
	#if(GAMESERVER_UPDATE>=702)

	this->GCArcaBattleStateSend((this->m_State+1),-1);

	this->CheckSync();

	#endif
}

void CArcaBattle::SetState_START() // OK
{
	#if(GAMESERVER_UPDATE>=702)

	this->SetObelisk();

	this->GCArcaBattleStateSend((this->m_State+1),-1);

	this->CheckSync();

	#endif
}

void CArcaBattle::SetState_CLOSE() // OK
{
	#if(GAMESERVER_UPDATE>=702)

	//this->NotifyArcaBattle(4);	//#DEFINE
	//this->DeleteArcaBattleAllMonster();

	this->GCArcaBattleStateSend((this->m_State+1),-1);

	this->CheckSync();

	#endif
}

void CArcaBattle::SetState_CLEAN() // OK
{
	#if(GAMESERVER_UPDATE>=702)

	/*
	// ---- Only for position:
			int iMapNumber;	//ebp-11c
			short sX;	//ebp-120
			BYTE btDIR;	//ebp-121
			short sLVL;	//ebp-128
			int iGateNum;	//ebp-12c
			short sY;	//ebp-130
			BYTE btMAPNUM;	//ebp-131
			// ----
			iGateNum	= 426;
			btMAPNUM	= 92;
			//iMapNumber	= gGateC.GetGate(iGateNum, (short&)sX, (short&)sY, (BYTE&)btMAPNUM, (BYTE&)btDIR, (short&)sLVL);
			iMapNumber	= gGateC.GetGate(iGateNum, (BYTE&)sX, (BYTE&)sY, (BYTE&)btMAPNUM, (BYTE&)btDIR, (short&)sLVL);
			// ----
			if( iMapNumber >= 0 )
			{
				for( int n = OBJ_STARTUSERINDEX; n < OBJMAX; n++ )
				{
					if( gObj[n].Connected == PLAYER_PLAYING )
					{
						if( gObj[n].Type == OBJ_USER )
						{
							gObjTeleport(n, btMAPNUM, sX, sY);
						}
					}
				}
			}
			// ----
			this->GDReqWinGuildInfoInsert();
			this->SendPlayResult();
			this->GiveRewardBuffWinningGuild();
		*/

	this->GCArcaBattleStateSend((this->m_State+1),-1);

	this->CheckSync();

	#endif
}

void CArcaBattle::SetState_CLEAR() // OK
{
	#if(GAMESERVER_UPDATE>=702)

	/*
	this->Clear();
			// ----
			for( int n = 9000; n < 10000; n++ )
			{
				if( gObj[n].Connected == PLAYER_PLAYING )
				{
					if( gObj[n].Type == OBJ_USER )
					{
						gObjMoveGate(n, 27);	//Lorencia / Devias?
					}
				}
			}
			// ----
			*/

	this->GCArcaBattleStateSend((this->m_State+1),-1);

	this->CheckSync();

	#endif
}

void CArcaBattle::CheckSync() // OK
{
	#if(GAMESERVER_UPDATE>=702)

	if(this->m_ArcaBattleStartTime.empty() != 0)
	{
		this->SetState(AB_STATE_BLANK);
		return;
	}

	CTime ScheduleTime;

	CScheduleManager ScheduleManager;

	for(std::vector<ARCA_BATTLE_START_TIME>::iterator it=this->m_ArcaBattleStartTime.begin();it != this->m_ArcaBattleStartTime.end();it++)
	{
		ScheduleManager.AddSchedule(it->Year,it->Month,it->Day,it->Hour,it->Minute,it->Second,it->DayOfWeek);
	}

	if(ScheduleManager.GetSchedule(&ScheduleTime) == 0)
	{
		this->SetState(AB_STATE_BLANK);
		return;
	}

	this->m_RemainTime = (int)difftime(ScheduleTime.GetTime(),time(0));

	this->m_TargetTime = (int)ScheduleTime.GetTime();

	#endif
}

void CArcaBattle::SetObelisk() // OK
{
	#if(GAMESERVER_UPDATE>=702)

	for(int n=0;n < MAX_ARCA_BATTLE_OBELISK;n++)
	{
		if(CHECK_RANGE(this->m_ObeliskInfo[n].Group,MAX_ARCA_BATTLE_OBELISK) != 0)
		{
			int index = gObjAddMonster(this->m_ObeliskInfo[n].Map);
	
			if(OBJECT_RANGE(index) == 0)
			{
				continue;
			}

			LPOBJ lpObj = &gObj[index];

			int px = this->m_ObeliskInfo[n].X;
			int py = this->m_ObeliskInfo[n].Y;

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
			lpObj->Map = this->m_ObeliskInfo[n].Map;

			if(gObjSetMonster(index,this->m_ObeliskInfo[n].MonsterClass) == 0)
			{
				gObjDel(index);
				continue;
			}

			lpObj->Life = (float)this->m_ObeliskInfo[n].Life;

			lpObj->MaxLife = (float)this->m_ObeliskInfo[n].Life;

			this->m_ObeliskStateInfo[n].MonsterIndex = index;

			this->SetObeliskAura(n);
		}
	}

	#endif
}

void CArcaBattle::SetObeliskAura(int group) // OK
{
	#if(GAMESERVER_UPDATE>=702)

	for(int n=0;n < MAX_ARCA_BATTLE_OBELISK_AURA;n++)
	{
		if(CHECK_RANGE(this->m_ObeliskInfo[group].AuraInfo[n].Index,MAX_ARCA_BATTLE_OBELISK_AURA) != 0)
		{
			int index = gObjAddMonster(this->m_ObeliskInfo[group].AuraInfo[n].Map);

			if(OBJECT_RANGE(index) == 0)
			{
				continue;
			}

			LPOBJ lpObj = &gObj[index];

			int px = this->m_ObeliskInfo[group].AuraInfo[n].X;
			int py = this->m_ObeliskInfo[group].AuraInfo[n].Y;

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
			lpObj->Map = this->m_ObeliskInfo[group].AuraInfo[n].Map;

			if(gObjSetMonster(index,603) == 0)
			{
				gObjDel(index);
				continue;
			}

			lpObj->Life = (float)1.0f;

			lpObj->MaxLife = (float)1.0f;

			this->m_ObeliskStateInfo[group].AuraStateInfo[n].Index = index;
		}
	}

	#endif
}

void CArcaBattle::ChkAuraUserHover() // OK
{
	int iHoverUserIndex_2; //ebp-24
	int iAurReleaseCnt; //ebp-28
	int iHoverUserIndex_1; //ebp-2c
	int iUserIndex; //ebp-30
	int iObeliskIndex; //ebp-34

	for(int n=0;n < MAX_ARCA_BATTLE_OBELISK;n++)
	{
		//LOOP CHECK

		this->m_ObeliskStateInfo[n].AuraState = 0;

		for(int i=0;i < MAX_ARCA_BATTLE_OBELISK_AURA;i++)
		{
			this->m_ObeliskStateInfo[n].AuraStateInfo[i].State = 0;
			this->m_ObeliskStateInfo[n].AuraStateInfo[i].UserIndex = -1;
		}
	}
	// ----
	for(int n=OBJECT_START_USER;n < MAX_OBJECT;n++)
	{
		if(gObjIsConnectedGP(n) != 0)
		{
			if(gObj[n].Map == MAP_ACHERON2 || gObj[n].Map == MAP_DEVENTER2)
			{
				for(int i=0;i < MAX_ARCA_BATTLE_OBELISK;i++)
				{
					for(int o=0;o < MAX_ARCA_BATTLE_OBELISK_AURA;o++)
					{
						if(this->m_ObeliskInfo[i].AuraInfo[o].X == gObj[n].X && this->m_ObeliskInfo[i].AuraInfo[o].Y == gObj[n].Y)
						{
							this->m_ObeliskStateInfo[i].AuraStateInfo[o].State = 1;
							this->m_ObeliskStateInfo[i].AuraStateInfo[o].UserIndex = n;
						}
					}
				}
			}
		}
	}
	// ----
	for(int n=0;n < MAX_ARCA_BATTLE_OBELISK;n++)
	{
		iAurReleaseCnt		= 0;
		iHoverUserIndex_1	= 0;
		iHoverUserIndex_2	= 0;
		// ----
		for(int i=0;i < MAX_ARCA_BATTLE_OBELISK_AURA;i++)
		{
			if(this->m_ObeliskStateInfo[n].AuraStateInfo[i].State == 1)
			{
				iAurReleaseCnt++;
			}
		}
		// ----
		if( iAurReleaseCnt == 3 )
		{
			this->m_ObeliskStateInfo[n].AuraState = 2; //#DEFINE
			// ----
			for(int i=0;i < MAX_ARCA_BATTLE_OBELISK_AURA;i++)
			{
				iHoverUserIndex_1 = this->m_ObeliskStateInfo[n].AuraStateInfo[i+0].UserIndex;
				iHoverUserIndex_2 = this->m_ObeliskStateInfo[n].AuraStateInfo[i+1].UserIndex;
				// ----
				if( gObj[iHoverUserIndex_1].GuildNumber != gObj[iHoverUserIndex_2].GuildNumber )
				{
					this->m_ObeliskStateInfo[n].AuraState = 1;
				}
			}
			// ----
			if(this->m_ObeliskStateInfo[n].AuraState == 2)
			{
				iUserIndex = this->m_ObeliskStateInfo[n].AuraStateInfo[0].UserIndex;
				// ----
				if(this->m_ObeliskStateInfo[n].AuraReleaseGuildNum != gObj[iUserIndex].GuildNumber)
				{
					iObeliskIndex = this->m_ObeliskStateInfo[n].MonsterIndex;
					// ----
					if( gObj[iObeliskIndex].Live == 1 )
					{
						if( this->m_ObeliskStateInfo[n].ObeliskState == 1 )
						{
							gObj[iObeliskIndex].Life = (float)this->m_ObeliskInfo[n].OccupyLife;
						}
						else
						{
							gObj[iObeliskIndex].Life = (float)this->m_ObeliskInfo[n].Life;
						}
					}
				}
				// ----
				this->m_ObeliskStateInfo[n].AuraReleaseGuildNum = gObj[iUserIndex].GuildNumber;
				// ----
				for(int i=0;i < MAX_ARCA_BATTLE_OBELISK_AURA;i++)
				{
					this->m_ObeliskStateInfo[n].AuraStateInfo[i].State = 2;
				}
			}
		}
		else
		{
			if( iAurReleaseCnt > 0 )
			{
				this->m_ObeliskStateInfo[n].AuraState = 1;
			}
		}
	}

	PMSG_ARCA_BATTLE_OBELISK_INFO_SEND pMsg;

	pMsg.header.set(0xF8,0x37,sizeof(pMsg));

	for(int n=0;n < MAX_ARCA_BATTLE_OBELISK;n++)
	{
		for(int i=0;i < MAX_ARCA_BATTLE_OBELISK_AURA;i++)
		{
			int iIndex	= this->m_ObeliskStateInfo[n].AuraStateInfo[i].Index;
			LPOBJ obj			= &gObj[iIndex];
			// ----
			for( int n = 0; n < MAX_VIEWPORT; n++ )
			{
				if( obj->VpPlayer2[n].type == OBJECT_USER && obj->VpPlayer2[n].state != 0 )
				{
					int iTarObjIndex			= obj->VpPlayer2[n].index;
					pMsg.ObeliskAttribute	= (BYTE)this->m_ObeliskInfo[n].Attr;
					// ----
					for(int o=0;o < MAX_ARCA_BATTLE_OBELISK_AURA;o++)
					{
						pMsg.ObeliskAuraState[o] = this->m_ObeliskStateInfo[n].AuraStateInfo[o].State;
					}
					// ----
					DataSend(gObj[iTarObjIndex].Index, (LPBYTE)&pMsg, sizeof(PMSG_ARCA_BATTLE_OBELISK_INFO_SEND));
				}
			}
		}
	}
}

bool CArcaBattle::RegenObelisk()
{
	for( int i = 0; i < MAX_ARCA_BATTLE_OBELISK; i++ )
	{
		int iObeliskIndex = this->m_ObeliskStateInfo[i].MonsterIndex;
		// ----
		if( !OBJECT_RANGE(iObeliskIndex) )
		{
			return false;
		}
		// ----
		if( gObj[iObeliskIndex].Live == 1 )
		{
			this->m_ObeliskInfo[i].CreatedTime = GetTickCount();
		}
		else
		{
			if( GetTickCount() - (DWORD)this->m_ObeliskInfo[i].CreatedTime >= (DWORD)this->m_ObeliskInfo[i].RespawnTime )
			{
				int result = gObjAddMonster(this->m_ObeliskInfo[i].Map);
				// ----
				if( result >= 0 )
				{
					gObjSetMonster(result, this->m_ObeliskInfo[i].MonsterClass);
					// ----
					gObj[result].Class			= (WORD)this->m_ObeliskInfo[i].MonsterClass;
					gObj[result].Map		= (BYTE)this->m_ObeliskInfo[i].Map;
					gObj[result].X				= (WORD)this->m_ObeliskInfo[i].X;
					gObj[result].Y				= (WORD)this->m_ObeliskInfo[i].Y;
					gObj[result].TX				= (WORD)this->m_ObeliskInfo[i].X;
					gObj[result].TY				= (WORD)this->m_ObeliskInfo[i].Y;
					gObj[result].OldX			= (WORD)this->m_ObeliskInfo[i].X;
					gObj[result].OldY			= (WORD)this->m_ObeliskInfo[i].Y;
					gObj[result].StartX			= (BYTE)this->m_ObeliskInfo[i].X;
					gObj[result].StartY			= (BYTE)this->m_ObeliskInfo[i].Y;
					gObj[result].Dir			= 1;
					gObj[result].Live			= true;
					gObj[result].DieRegen		= 0;
					gObj[result].MaxRegenTime	= 0;
					gObj[result].MaxLife		= (float)this->m_ObeliskInfo[i].OccupyLife;
					gObj[result].Life			= (float)this->m_ObeliskInfo[i].OccupyLife;
					// ----
					this->m_ObeliskStateInfo[i].MonsterIndex = result;
				}
			}
		}
	}
	// ----
	return true;
}

bool CArcaBattle::IsEnableAttackObelisk(LPOBJ lpObj,int MonsterClass)
{
	int i;
	ARCA_BATTLE_OBELISK_STATE_INFO * pObelskState = NULL;
	// ----
	for( i = 0; i < MAX_ARCA_BATTLE_OBELISK; i++ )
	{
		if( this->m_ObeliskInfo[i].MonsterClass == MonsterClass )
		{
			pObelskState = &this->m_ObeliskStateInfo[i];
		}
	}
	// ----
	if( pObelskState == NULL )
	{
		return true;
	}
	// ----
	if( this->m_State != AB_STATE_START )
	{
		return false;
	}
	// ----
	if( pObelskState->OccupyGuildNum == lpObj->GuildNumber )
	{
		return false;
	}
	// ----
	if( pObelskState->AuraState == 2 && pObelskState->AuraReleaseGuildNum == lpObj->GuildNumber )
	{
		return true;
	}
	// ----
	return false;
}

bool CArcaBattle::IsPkEnable(LPOBJ lpObj,LPOBJ lpTarget)
{
	GUILD_INFO_STRUCT * lpGuildInfo	= lpObj->Guild;
	GUILD_INFO_STRUCT * lpTarGuildInfo	= lpTarget->Guild;
	// ----
	if( this->m_State != AB_STATE_START )
	{
		return true;
	}
	// ----
	if( lpGuildInfo == NULL )
	{
		return false;
	}
	// ----
	if( lpTarGuildInfo == NULL )
	{
		return false;
	}

	if((lpObj->Map != MAP_ACHERON2 && lpObj->Map != MAP_DEVENTER2) || (lpObj->Map != MAP_ACHERON2 && lpObj->Map != MAP_DEVENTER2))
	{
		return false;
	}
	// ----
	if( lpGuildInfo->Number != lpTarGuildInfo->Number )
	{
		return false;
	}
	// ----
	return true;
}

void CArcaBattle::NpcLesnar(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	#if(GAMESERVER_UPDATE>=702)

	PMSG_NPC_TALK_SEND pMsg;

	pMsg.header.set(0x30,sizeof(pMsg));

	pMsg.result = 40;

	DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CArcaBattle::UserDieProc(LPOBJ lpObj,LPOBJ lpTarget) // OK
{

}

void CArcaBattle::MonsterDieProc(LPOBJ lpObj,LPOBJ lpTarget) // OK
{
	#if(GAMESERVER_UPDATE>=702)

	int aIndex = gObjMonsterGetTopHitDamageUser(lpObj);

	if(OBJECT_RANGE(aIndex) != 0)
	{
		lpTarget = &gObj[aIndex];
	}

	#endif
}

void CArcaBattle::NoticeSendToAll(int type,char* message,...) // OK
{
	#if(GAMESERVER_UPDATE>=702)

	char buff[256];

	va_list arg;
	va_start(arg,message);
	vsprintf_s(buff,message,arg);
	va_end(arg);

	for(int n=OBJECT_START_USER;n < MAX_OBJECT;n++)
	{
		if(gObjIsConnectedGP(n) != 0)
		{
			if(gObj[n].Map == MAP_ACHERON2 || gObj[n].Map == MAP_DEVENTER2)
			{
				gNotice.GCNoticeSend(n,type,0,0,0,0,0,buff);
			}
		}
	}

	#endif
}

void CArcaBattle::DataSendToAll(BYTE* lpMsg,int size) // OK
{
	#if(GAMESERVER_UPDATE>=702)

	for(int n=OBJECT_START_USER;n < MAX_OBJECT;n++)
	{
		if(gObjIsConnectedGP(n) != 0)
		{
			if(gObj[n].Map == MAP_ACHERON2 || gObj[n].Map == MAP_DEVENTER2)
			{
				DataSend(n,lpMsg,size);
			}
		}
	}

	#endif
}

void CArcaBattle::CGArcaBattleGuildMasterRegisterRecv(int aIndex) // OK
{
	//register guild

	//send to DS and back

	//CHECL GUILD EXISTES = 1

	//CHECK ENTER STATE = 5

	//CHECK GUILD mASTER = 1

	//CHECK GUILD COUNT WITH G JOIN COUNT = 2

	//REGISTER

	//0 = join
	//1 = Not Guild Master
	//2 = Below Guild Member
	//3 = Guild Over
	//4 = Guild Name Same
	//5 = Time Over
	//6 = Need Item

	/*
	SEND ON DS CALLBACK
	        _tagPMSG_ANS_ARCA_BATTLE_JOIN::_tagPMSG_ANS_ARCA_BATTLE_JOIN(&pMsg);
        pMsg.h.c = -63;
        pMsg.h.size = 5;
        pMsg.h.headcode = -8;
        pMsg.h.subcode = 49;
        pMsg.btResult = iResult;
        DataSend(aIndex, &pMsg.h.c, 5u);
	*/

	SDHP_ARCA_BATTLE_GUILD_MASTER_REGISTER_SEND pMsg;

	pMsg.header.set(0x1B,0x00,sizeof(pMsg));

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);
}

void CArcaBattle::CGArcaBattleGuildMemberRegisterRecv(int aIndex) // OK
{
	//0 = join
	//7 = not guild member
	//8 = already reg
	//9 = member count over
	//10 = time over
	//13 = g master NOT
	//14 = g master not reg

	/*
	SEND ON DS CALLBACK
	        _tagPMSG_ANS_ARCA_BATTLE_JOIN::_tagPMSG_ANS_ARCA_BATTLE_JOIN(&pMsg);
        pMsg.h.c = -63;
        pMsg.h.size = 5;
        pMsg.h.headcode = -8;
        pMsg.h.subcode = 51;
        pMsg.btResult = lpMsg->btResult;
        DataSend(aIndex, &pMsg.h.c, 5u);
		*/

	SDHP_ARCA_BATTLE_GUILD_MEMBER_REGISTER_SEND pMsg;

	pMsg.header.set(0x1B,0x00,sizeof(pMsg));

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);
}

void CArcaBattle::CGArcaBattleEnterRecv(int aIndex) // OK
{
	//DGAnsArcaBattleEnter

	//11 = gmaster not join
	//12 = not play time
	//

	/*
	SEND ON DS CALLBACK
	    _tagPMSG_ANS_ARCA_BATTLE_ENTER::_tagPMSG_ANS_ARCA_BATTLE_ENTER(&pMsg);
    pMsg.h.c = -63;
    pMsg.h.size = 5;
    pMsg.h.headcode = -8;
    pMsg.h.subcode = 53;
    pMsg.btResult = lpMsg->btResult;
    DataSend(aIndex, &pMsg.h.c, 5u);
	*/
}

void CArcaBattle::CGArcaBattleRegisterMarkRecv(int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	PMSG_ARCA_BATTLE_REGISTER_MARK_ERROR_SEND pMsg;

	pMsg.header.set(0xF8,0x47,sizeof(pMsg));

	int iArcaBattleState = this->m_State;

	if ( iArcaBattleState < 3 || iArcaBattleState > 9 )
	{
		if(lpObj->Guild == 0)
		{
			pMsg.result = 15; //NO AUTHORITHA
			DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
			return;
		}

		if(lpObj->GuildStatus != 32 && lpObj->GuildStatus != 64 && lpObj->GuildStatus != 128)
		{
			pMsg.result = 15; //NO AUTHORITHA
			DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
			return;
		}

		if(lpObj->Guild->Count < 10) //MIN GUILD NUM
		{
			pMsg.result = 16; //NOT ENOUGH MEMBAS
			DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
			return;
		}

		//REQUEST COUNT FROM DATASETVER

		//error 17 = OVER MAX MARK (250)

		//SEND COUNT = *(_DWORD *)(lpObj + 1392)
	}
	else
	{
		pMsg.result = 5; //NO TIME
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
	}
}

void CArcaBattle::GCArcaBattleStateSend(int state,int aIndex) // OK
{
	#if(GAMESERVER_UPDATE>=702)

	PMSG_ARCA_BATTLE_STATE_SEND pMsg;

	pMsg.header.set(0xF8,0x38,sizeof(pMsg));

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

void CArcaBattle::GCArcaBattleInfoSend(int aIndex,int GuildGroupNum) // OK
{
	#if(GAMESERVER_UPDATE>=702)

	PMSG_ARCA_BATTLE_INFO_SEND pMsg;

	pMsg.header.set(0xF8,0x39,sizeof(pMsg));

	pMsg.GuildGroupNum = GuildGroupNum;

	for(int n=0;n < MAX_ARCA_BATTLE_OBELISK;n++)
	{
		pMsg.ObeliskInfo[n].GroupNumber = this->m_ObeliskInfo[n].Group;

		pMsg.ObeliskInfo[n].AttrKind = this->m_ObeliskInfo[n].Attr;

		pMsg.ObeliskInfo[n].MapNumber = this->m_ObeliskInfo[n].Map;

		pMsg.ObeliskInfo[n].MapX = this->m_ObeliskInfo[n].X;

		pMsg.ObeliskInfo[n].MapY = this->m_ObeliskInfo[n].Y;

		for(int i=0;i < MAX_ARCA_BATTLE_OBELISK_AURA;i++)
		{
			pMsg.ObeliskInfo[n].AuraInfo[i].AuraX = this->m_ObeliskInfo[n].AuraInfo[i].X;

			pMsg.ObeliskInfo[n].AuraInfo[i].AuraY = this->m_ObeliskInfo[n].AuraInfo[i].Y;
		}
	}

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);

	#endif
}

void CArcaBattle::GCArcaBattleObeliskStateSend() // OK
{
	#if(GAMESERVER_UPDATE>=702)

	PMSG_ARCA_BATTLE_OBELISK_STATE_SEND pMsg;

	pMsg.header.set(0xF8,0x3A,sizeof(pMsg));

	for(int n=0;n < MAX_ARCA_BATTLE_OBELISK;n++)
	{
		BYTE AuraRelease;
		LPOBJ obj;
		int iObeliskIndex = this->m_ObeliskStateInfo[n].MonsterIndex;
		// ----
		if ( this->m_ObeliskStateInfo[n].MonsterIndex >= 0 )
		{
			obj = &gObj[iObeliskIndex];
			pMsg.ObeliskState		= gObj[iObeliskIndex].Life > 0.0;
			pMsg.ObeliskAttribute = (BYTE)this->m_ObeliskInfo[n].Attr;
			AuraRelease				= 2;	//#DEFINE
			// ----
			for( int i = 0; i < MAX_ARCA_BATTLE_OBELISK_AURA; i++ )
			{
				if( this->m_ObeliskStateInfo[n].AuraStateInfo[i].State != 2 ) //#DEFINE
				{
					AuraRelease = 0;
					break;
				}
			}
			// ----
			pMsg.ObeliskAuraState = AuraRelease;
			// ----
			for( int i = 0; i < MAX_VIEWPORT; i++ )
			{
				if( obj->VpPlayer2[i].type == OBJECT_USER && obj->VpPlayer2[i].state != 0 )
				{
					int iTarObjIndex = gObj[obj->VpPlayer2[i].index].Index;
					DataSend(iTarObjIndex, (LPBYTE)&pMsg, sizeof(PMSG_ARCA_BATTLE_OBELISK_STATE_SEND));
				}
			}
		}
	}

	#endif
}

void CArcaBattle::GCArcaBattleOccupyGuildSend(LPOBJ lpObj,int gate) // OK
{
	PMSG_ARCA_BATTLE_OCCUPY_GUILD_SEND pMsg;

	pMsg.header.set(0xF8,0x3D,sizeof(pMsg));

	if( gate == 418 || gate == 422 )
	{
		for( int i = 0; i < MAX_ARCA_BATTLE_OBELISK; i++ )
		{
			if(gate == 418 && this->m_GuildInfo[i].ObeliskGroup == 1  ||	gate == 422 && this->m_GuildInfo[i].ObeliskGroup == 2 )	//Maybe it obelist attr (dark, earth...)
			{
				memcpy(pMsg.GuildName, this->m_GuildInfo[i].GuildName, 9); //MAX_GUILD_NAME define
				DataSend(lpObj->Index, (LPBYTE)&pMsg, sizeof(PMSG_ARCA_BATTLE_OCCUPY_GUILD_SEND));
				break;
			}
		}
	}
}

void CArcaBattle::GCArcaBattleObeliskLifeSend() // OK
{
	#if(GAMESERVER_UPDATE>=702)

	PMSG_ARCA_BATTLE_OBELISK_LIFE_SEND pMsg;

	pMsg.header.set(0xF8,0x3F,sizeof(pMsg));

	for(int n=0;n < MAX_ARCA_BATTLE_OBELISK;n++)
	{
		int iObeliskIndex = this->m_ObeliskStateInfo[n].MonsterIndex;
		// ----
		if( !OBJECT_RANGE(iObeliskIndex) )
		{
			return;
		}

		if( !gObj[iObeliskIndex].Live )
		{
			pMsg.ObeliskLife[n].ObeliskLife = 0;
			pMsg.ObeliskLife[n].ObeliskAttribute = (BYTE)this->m_ObeliskInfo[n].Attr;
		}
		else
		{
			pMsg.ObeliskLife[n].ObeliskLife = (int)ceil(gObj[iObeliskIndex].Life / (gObj[iObeliskIndex].MaxLife / 10.0)) & 0xF;	//Need check
			pMsg.ObeliskLife[n].ObeliskAttribute = (BYTE)this->m_ObeliskInfo[n].Attr;
		}
	}

	this->DataSendToAll((BYTE*)&pMsg,pMsg.header.size);

	#endif
}
