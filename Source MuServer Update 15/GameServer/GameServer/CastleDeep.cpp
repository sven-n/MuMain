// CastleDeep.cpp: implementation of the CCastleDeep class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CastleDeep.h"
#include "DSProtocol.h"
#include "Map.h"
#include "MapServerManager.h"
#include "MemScript.h"
#include "Message.h"
#include "Monster.h"
#include "Notice.h"
#include "ScheduleManager.h"
#include "ServerInfo.h"
#include "Util.h"

CCastleDeep gCastleDeep;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCastleDeep::CCastleDeep() // OK
{
	this->m_State = CD_STATE_EMPTY;
	this->m_RemainTime = 0;
	this->m_TargetTime = 0;
	this->m_TickCount = GetTickCount();
	this->m_TimeNotify = 0;
	this->m_Stage = 0;
	this->m_Group = 0;

	this->m_CastleDeepStartTime.clear();

	this->m_CastleDeepGroup.clear();

	this->m_CastleDeepSpawn.clear();

	this->m_CastleDeepMonster.clear();
}

CCastleDeep::~CCastleDeep() // OK
{

}

void CCastleDeep::Init() // OK
{
	if(gServerInfo.m_CastleDeepEvent == 0)
	{
		this->SetState(CD_STATE_BLANK);
	}
	else
	{
		this->SetState(CD_STATE_EMPTY);
	}
}

void CCastleDeep::Load(char* path) // OK
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

	this->m_CastleDeepStartTime.clear();

	this->m_CastleDeepGroup.clear();

	this->m_CastleDeepSpawn.clear();

	this->m_CastleDeepMonster.clear();

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

					CASTLE_DEEP_START_TIME info;

					info.Year = lpMemScript->GetNumber();

					info.Month = lpMemScript->GetAsNumber();

					info.Day = lpMemScript->GetAsNumber();

					info.DayOfWeek = lpMemScript->GetAsNumber();

					info.Hour = lpMemScript->GetAsNumber();

					info.Minute = lpMemScript->GetAsNumber();

					info.Second = lpMemScript->GetAsNumber();

					this->m_CastleDeepStartTime.push_back(info);
				}
				else if(section == 1)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					CASTLE_DEEP_GROUP info;

					info.Index = lpMemScript->GetNumber();

					info.Rate = lpMemScript->GetAsNumber();

					this->m_CastleDeepGroup.push_back(info);
				}
				else if(section == 2)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					CASTLE_DEEP_SPAWN info;

					info.Index = lpMemScript->GetNumber();

					info.Group = lpMemScript->GetAsNumber();

					info.Minute = lpMemScript->GetAsNumber();

					this->m_CastleDeepSpawn.push_back(info);
				}
				else if(section == 3)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					CASTLE_DEEP_MONSTER info;

					info.Index = lpMemScript->GetNumber();

					info.Group = lpMemScript->GetAsNumber();

					info.Class = lpMemScript->GetAsNumber();

					info.Count = lpMemScript->GetAsNumber();

					info.X = lpMemScript->GetAsNumber();

					info.Y = lpMemScript->GetAsNumber();

					info.TX = lpMemScript->GetAsNumber();

					info.TY = lpMemScript->GetAsNumber();

					this->m_CastleDeepMonster.push_back(info);
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
}

void CCastleDeep::MainProc() // OK
{
	DWORD elapsed = GetTickCount()-this->m_TickCount;

	if(elapsed < 1000)
	{
		return;
	}

	if(gServerInfo.m_CastleDeepEvent == 0)
	{
		if (gServerDisplayer.EventCastleDeep != -1)
		{
			gServerDisplayer.EventCastleDeep = -1;
		}
	}
	else 
	{
		if (this->m_State == CD_STATE_EMPTY)
		{
			gServerDisplayer.EventCastleDeep = this->m_RemainTime;
		}
		else 
		{
			if (gServerDisplayer.EventCastleDeep != 0)
			{
				gServerDisplayer.EventCastleDeep = 0;
			}
		}
	}

	if(gServerDisplayer.EventCastleDeep > 0)
	{
		if (this->m_State != CD_STATE_EMPTY && this->m_State != CD_STATE_BLANK)
		{
			gServerDisplayer.EventCastleDeep = 0;
		}
	}

	this->m_TickCount = GetTickCount();

	this->m_RemainTime = (int)difftime(this->m_TargetTime,time(0));

	switch(this->m_State)
	{
		case CD_STATE_BLANK:
			this->ProcState_BLANK();
			break;
		case CD_STATE_EMPTY:
			this->ProcState_EMPTY();
			break;
		case CD_STATE_START:
			this->ProcState_START();
			break;
	}
}

void CCastleDeep::ProcState_BLANK() // OK
{

}

void CCastleDeep::ProcState_EMPTY() // OK
{
	if(this->m_RemainTime > 0 && this->m_RemainTime <= 300 && this->m_TimeNotify == 0)
	{
		this->m_TimeNotify = 1;

		GDGlobalNoticeSend(gMapServerManager.GetMapServerGroup(),0,0,0,0,0,0,gMessage.GetMessage(352));
	}

	if(this->m_RemainTime <= 0)
	{
		GDGlobalNoticeSend(gMapServerManager.GetMapServerGroup(),0,0,0,0,0,0,gMessage.GetMessage(353));

		this->SetState(CD_STATE_START);
	}
}

void CCastleDeep::ProcState_START() // OK
{
	for(std::vector<CASTLE_DEEP_SPAWN>::iterator it=this->m_CastleDeepSpawn.begin();it != this->m_CastleDeepSpawn.end();it++)
	{
		if(this->m_Group == it->Index && this->m_Stage == it->Group && (((gServerInfo.m_CastleDeepEventTime*60)-this->m_RemainTime)/60) == it->Minute)
		{
			this->AddMonster(it->Index,it->Group);
			this->m_Stage++;
			break;
		}
	}

	if(this->m_RemainTime <= 0)
	{
		gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(354));

		this->SetState(CD_STATE_EMPTY);
	}
}

void CCastleDeep::SetState(int state) // OK
{
	this->m_State = state;

	switch(this->m_State)
	{
		case CD_STATE_BLANK:
			this->SetState_BLANK();
			break;
		case CD_STATE_EMPTY:
			this->SetState_EMPTY();
			break;
		case CD_STATE_START:
			this->SetState_START();
			break;
	}
}

void CCastleDeep::SetState_BLANK() // OK
{
	//LogAdd(LOG_BLACK,"[Castle Deep] SetState BLANK");
}

void CCastleDeep::SetState_EMPTY() // OK
{
	this->m_TimeNotify = 0;
	this->m_Stage = 1;
	this->m_Group = 0;

	this->ClearMonster();

	this->CheckSync();

	//LogAdd(LOG_BLACK,"[Castle Deep] SetState EMPTY");
}

void CCastleDeep::SetState_START() // OK
{
	this->m_RemainTime = gServerInfo.m_CastleDeepEventTime*60;

	this->m_TargetTime = (int)(time(0)+this->m_RemainTime);

	while(this->m_Group == 0)
	{
		for(std::vector<CASTLE_DEEP_GROUP>::iterator it=this->m_CastleDeepGroup.begin();it != this->m_CastleDeepGroup.end();it++)
		{
			if((GetLargeRand()%100) < it->Rate)
			{
				this->m_Group = it->Index;
				break;
			}
		}
	}

	//LogAdd(LOG_BLACK,"[Castle Deep] SetState START");
}

void CCastleDeep::CheckSync() // OK
{
	if(this->m_CastleDeepStartTime.empty() != 0)
	{
		this->SetState(CD_STATE_BLANK);
		return;
	}

	CTime ScheduleTime;

	CScheduleManager ScheduleManager;

	for(std::vector<CASTLE_DEEP_START_TIME>::iterator it=this->m_CastleDeepStartTime.begin();it != this->m_CastleDeepStartTime.end();it++)
	{
		ScheduleManager.AddSchedule(it->Year,it->Month,it->Day,it->Hour,it->Minute,it->Second,it->DayOfWeek);
	}

	if(ScheduleManager.GetSchedule(&ScheduleTime) == 0)
	{
		this->SetState(CD_STATE_BLANK);
		return;
	}

	this->m_RemainTime = (int)difftime(ScheduleTime.GetTime(),time(0));

	this->m_TargetTime = (int)ScheduleTime.GetTime();

	//LogAdd(LOG_BLACK,"[Castle Deep] Sync Start Time. [%d] min remain",(this->m_RemainTime/60));
}

void CCastleDeep::ClearMonster() // OK
{
	for(int n=0;n < MAX_OBJECT_MONSTER;n++)
	{
		if(gObjIsConnected(n) != 0 && gObj[n].Attribute == 62)
		{
			gObjDel(n);
		}
	}
}

void CCastleDeep::AddMonster(int stage,int group) // OK 
{
	for(std::vector<CASTLE_DEEP_MONSTER>::iterator it=this->m_CastleDeepMonster.begin();it != this->m_CastleDeepMonster.end();it++)
	{
		if(it->Index != stage || it->Group != group)
		{
			continue;
		}

		for(int n=0;n < it->Count;n++)
		{
			int index = gObjAddMonster(MAP_CASTLE_SIEGE);

			if(OBJECT_RANGE(index) == 0)
			{
				continue;
			}

			LPOBJ lpObj = &gObj[index];

			int px = it->X;
			int py = it->Y;

			if(gObjGetRandomFreeLocation(MAP_CASTLE_SIEGE,&px,&py,(it->TX-it->X),(it->TY-it->Y),10) == 0)
			{
				px = it->X;
				py = it->Y;
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
			lpObj->Dir = GetLargeRand()%8;
			lpObj->Map = MAP_CASTLE_SIEGE;

			if(gObjSetMonster(index,it->Class) == 0)
			{
				gObjDel(index);
				continue;
			}

			lpObj->DieRegen = 0;
			lpObj->RegenTime = 1;
			lpObj->Attribute = 62;
			lpObj->MaxRegenTime = 1000;
			lpObj->LastCheckTick = GetTickCount();
		}
	}
}

void CCastleDeep::MonsterDieProc(LPOBJ lpObj,LPOBJ lpTarget) // OK
{
	int aIndex = gObjMonsterGetTopHitDamageUser(lpObj);

	if(OBJECT_RANGE(aIndex) != 0)
	{
		lpTarget = &gObj[aIndex];
	}

	if(lpObj->Attribute == 62 && lpObj->Class == 295)
	{
		gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(355),lpTarget->Name);
	}

	if(lpObj->Attribute == 62 && lpObj->Class >= 300 && lpObj->Class <= 303)
	{
		gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(356),lpObj->Name,lpTarget->Name);
	}
}

void CCastleDeep::StartLD()
{
#if(GAMESERVER_TYPE==1)
	CTime CurrentTime = CTime::GetTickCount();

	int hour	= (int)CurrentTime.GetHour();
	int minute	= (int)CurrentTime.GetMinute()+2;

	if (minute >= 60)
	{
		hour++;
		minute = minute-60; 
	}

	CASTLE_DEEP_START_TIME info;

	info.Year = (int)CurrentTime.GetYear();

	info.Month = (int)CurrentTime.GetMonth();

	info.Day = (int)CurrentTime.GetDay();

	info.DayOfWeek = -1;

	info.Hour = hour;

	info.Minute = minute;

	info.Second = 0;

	this->m_CastleDeepStartTime.push_back(info);

	LogAdd(LOG_EVENT,"[Set Loren Deep Start] At %2d:%2d:00",hour,minute);

	this->Init();
	
#endif
}
