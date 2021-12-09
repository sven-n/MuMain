#include "stdafx.h"
#include "EventGvG.h"
#include "Guild.h"
#include "GuildClass.h"
#include "EffectManager.h"
#include "Notice.h"
#include "ObjectManager.h"
#include "CashShop.h"
#include "Gate.h"
#include "ScheduleManager.h"
#include "MemScript.h"
#include "DSProtocol.h"
#include "ItemManager.h"
#include "Message.h"
#include "ServerInfo.h"
#include "Util.h"

CGvGEvent gGvGEvent;

CGvGEvent::CGvGEvent()
{
	this->m_TickCount = GetTickCount();
	this->m_State = 0;
	this->m_RemainTime = 0;
	this->m_StandTime = 0;
	this->m_CloseTime = 0;
	this->m_TickCount = 0;
	this->m_WarningTime = 0;
	this->m_EventTime = 0;

	this->TotalPlayer = 0;

	this->Coin1 = 0;
	this->Coin2 = 0;
	this->Coin3 = 0;

	this->Winner = -1;

	this->CleanUser();
	this->CleanGuild();

}

CGvGEvent::~CGvGEvent()
{
	
}

void CGvGEvent::Init()
{
	if(gServerInfo.m_GvGEventSwitch == 0)
	{
		this->SetState(GVG_EVENT_STATE_BLANK);
	}
	else
	{
		this->SetState(GVG_EVENT_STATE_EMPTY);
	}
}

void CGvGEvent::Clear()
{
	this->TotalPlayer = 0;

	this->CleanUser();
	this->CleanGuild();

	this->Winner = -1;
}

void CGvGEvent::Load(char* path)
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

	this->m_GVGStartTime.clear();

	this->Clear();

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
					this->m_WarningTime = lpMemScript->GetNumber();

					this->m_StandTime = lpMemScript->GetAsNumber();

					this->m_EventTime = lpMemScript->GetAsNumber();

					this->m_CloseTime = lpMemScript->GetAsNumber();

				}
				else if(section == 1)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					GVG_TIME info;

					info.Year = lpMemScript->GetNumber();

					info.Month = lpMemScript->GetAsNumber();

					info.Day = lpMemScript->GetAsNumber();

					info.DayOfWeek = lpMemScript->GetAsNumber();

					info.Hour = lpMemScript->GetAsNumber();

					info.Minute = lpMemScript->GetAsNumber();

					info.Second = lpMemScript->GetAsNumber();

					this->m_GVGStartTime.push_back(info);

				}
				else if(section == 2)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}
					this->ReqItemCount = lpMemScript->GetNumber();

					this->ReqItemIndex = lpMemScript->GetAsNumber();

					this->ReqItemLevel = lpMemScript->GetAsNumber();

					this->EventMap = lpMemScript->GetAsNumber();

					this->WaitingGate = lpMemScript->GetAsNumber();

					this->StartGate = lpMemScript->GetAsNumber();

					this->MinLevel = lpMemScript->GetAsNumber();

					this->MaxLevel = lpMemScript->GetAsNumber();

					this->MinReset = lpMemScript->GetAsNumber();

					this->MaxReset = lpMemScript->GetAsNumber();

					this->MinMasterReset = lpMemScript->GetAsNumber();

					this->MaxMasterReset = lpMemScript->GetAsNumber();

				}
				else if(section == 3)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}
					this->Coin1 = lpMemScript->GetNumber();

					this->Coin2 = lpMemScript->GetAsNumber();

					this->Coin3 = lpMemScript->GetAsNumber();
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

void CGvGEvent::MainProc()
{
	if((GetTickCount()-this->m_TickCount) >= 1000)
	{
		this->m_TickCount = GetTickCount();

		this->m_RemainTime = (int)difftime(this->TargetTime,time(0));

	if(gServerInfo.m_GvGEventSwitch == 0)
	{
		if (gServerDisplayer.EventGvG != -1)
		{
			gServerDisplayer.EventGvG = -1;
		}
	}
	else 
	{
		if (this->m_State == GVG_EVENT_STATE_EMPTY)
		{
			gServerDisplayer.EventGvG = this->m_RemainTime;
		}
		else 
		{
			if (gServerDisplayer.EventGvG != 0)
			{
				gServerDisplayer.EventGvG = 0;
			}
		}
	}

		switch(this->m_State)
		{
			case GVG_EVENT_STATE_BLANK:
				this->ProcState_BLANK();
				break;
			case GVG_EVENT_STATE_EMPTY:
				this->ProcState_EMPTY();
				break;
			case GVG_EVENT_STATE_STAND:
				this->ProcState_STAND();
				break;
			case GVG_EVENT_STATE_START:
				this->ProcState_START();
				break;
			case GVG_EVENT_STATE_CLEAN:
				this->ProcState_CLEAN();
				break;
		}
	}
}

void CGvGEvent::ProcState_BLANK() // OK
{

}

void CGvGEvent::ProcState_EMPTY() // OK
{
	if(this->m_RemainTime > 0 && this->m_RemainTime <= (this->m_WarningTime*60))
	{
		this->CheckUser();

		this->EnterEnabled = 1;

		if((this->AlarmMinSave=(((this->m_RemainTime%60)==0)?((this->m_RemainTime/60)-1):(this->m_RemainTime/60))) != this->AlarmMinLeft)
		{
			this->AlarmMinLeft = this->AlarmMinSave;

			gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(863),(this->AlarmMinLeft+1));
		}
	}

	if(this->m_RemainTime <= 0)
	{
		gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(865));
		this->NoticeSendToAll(1,gMessage.GetMessage(866),this->m_StandTime);
		this->SetState(GVG_EVENT_STATE_STAND);
	}
}

void CGvGEvent::ProcState_STAND() // OK
{
	this->CheckUser();

	if(this->GetUserCount() < gServerInfo.m_GvGEventMinUsers)
	{
		this->NoticeSendToAll(0,gMessage.GetMessage(875));
		this->SetState(GVG_EVENT_STATE_EMPTY);
		return;
	}

	if(this->GetGuildCount() < 2)
	{
		this->NoticeSendToAll(0,gMessage.GetMessage(875));
		this->SetState(GVG_EVENT_STATE_EMPTY);
		return;
	}

	if(this->m_RemainTime <= 0)
	{
		if (this->TotalPlayer >= gServerInfo.m_GvGEventMinUsers)
		{
			for(int n=0;n<MAX_GVGEVENT_USER;n++)
			{
				gObjMoveGate(this->User[n].Index, this->User[n].Gate);
			}
			this->NoticeSendToAll(0,gMessage.GetMessage(867));
			this->SetState(GVG_EVENT_STATE_START);
		}
		else
		{
			this->NoticeSendToAll(0,gMessage.GetMessage(868));
			this->SetState(GVG_EVENT_STATE_EMPTY);
		}
	}
}

void CGvGEvent::ProcState_START() // OK
{
	this->CheckUser();

	if(this->GetUserCount() == 0)
	{
		this->SetState(GVG_EVENT_STATE_EMPTY);
		this->NoticeSendToAll(0,gMessage.GetMessage(875));
		return;
	}

	if(this->GetUserCount() == 1)
	{
		this->NoticeSendToAll(0,gMessage.GetMessage(875));
		this->SetState(GVG_EVENT_STATE_CLEAN);
		return;
	}

	if(this->m_RemainTime <= 0)
	{
		this->NoticeSendToAll(0,gMessage.GetMessage(868));
		this->SetState(GVG_EVENT_STATE_CLEAN);
	}
}

void CGvGEvent::ProcState_CLEAN() // OK
{
	this->CheckUser();

	if(this->m_RemainTime <= 0)
	{
		this->SetState(GVG_EVENT_STATE_EMPTY);
	}
}

void CGvGEvent::SetState(int state) // OK
{
	switch((this->m_State=state))
	{
		case GVG_EVENT_STATE_BLANK:
			this->SetState_BLANK();
			break;
		case GVG_EVENT_STATE_EMPTY:
			this->SetState_EMPTY();
			break;
		case GVG_EVENT_STATE_STAND:
			this->SetState_STAND();
			break;
		case GVG_EVENT_STATE_START:
			this->SetState_START();
			break;
		case GVG_EVENT_STATE_CLEAN:
			this->SetState_CLEAN();
			break;
	}
}

void CGvGEvent::SetState_BLANK() // OK
{

}

void CGvGEvent::SetState_EMPTY() // OK
{
	this->EnterEnabled = 0;
	this->AlarmMinSave = -1;
	this->AlarmMinLeft = -1;

	this->ClearUser();
	this->ClearGuild();

	this->Winner = -1;

	this->CheckSync();
}

void CGvGEvent::SetState_STAND() // OK
{
	this->EnterEnabled = 0;
	this->AlarmMinSave = -1;
	this->AlarmMinLeft = -1;

	this->m_RemainTime = this->m_StandTime*60;

	this->TargetTime = (int)(time(0)+this->m_RemainTime);
}

void CGvGEvent::SetState_START() // OK
{
	this->EnterEnabled = 0;
	this->AlarmMinSave = -1;
	this->AlarmMinLeft = -1;

	this->m_RemainTime = this->m_EventTime*60;

	this->TargetTime = (int)(time(0)+this->m_RemainTime);
}

void CGvGEvent::SetState_CLEAN() // OK
{
	this->EnterEnabled = 0;
	this->AlarmMinSave = -1;
	this->AlarmMinLeft = -1;

	this->CalcRank();

	for(int n=0;n < MAX_GVGEVENT_USER;n++)
	{
		if(OBJECT_RANGE(this->User[n].Index) != 0)
		{
			//gObjViewportListProtocolCreate(&gObj[this->User[n].Index]);
			gObjectManager.CharacterUpdateMapEffect(&gObj[this->User[n].Index]);

			if(this->User[n].Guild == this->Winner)
			{
				LPOBJ lpObj = &gObj[this->User[n].Index];

				if (this->Coin1 > 0 || this->Coin2 > 0 || this->Coin3 > 0)
				{	
					GDSetCoinSend(lpObj->Index, this->Coin1, this->Coin2, this->Coin3,"GvGEvent");
				}

				//GDRankingTvTEventSaveSend(lpObj->Index, this->User[n].Kills, this->User[n].Deaths);
			}
		}
	}

	this->m_RemainTime = this->m_CloseTime*60;

	this->TargetTime = (int)(time(0)+this->m_RemainTime);
}

void CGvGEvent::CheckSync() // OK
{
	if(this->m_GVGStartTime.empty() != 0)
	{
		this->SetState(GVG_EVENT_STATE_BLANK);
		return;
	}

	CTime ScheduleTime;

	CScheduleManager ScheduleManager;

	for(std::vector<GVG_TIME>::iterator it=this->m_GVGStartTime.begin();it != this->m_GVGStartTime.end();it++)
	{
		ScheduleManager.AddSchedule(it->Year,it->Month,it->Day,it->Hour,it->Minute,it->Second,it->DayOfWeek);
	}

	if(ScheduleManager.GetSchedule(&ScheduleTime) == 0)
	{
		this->SetState(GVG_EVENT_STATE_BLANK);
		return;
	}

	this->m_RemainTime = (int)difftime(ScheduleTime.GetTime(),time(0));

	this->TargetTime = (int)ScheduleTime.GetTime();
}

int CGvGEvent::GetState() // OK
{
	return this->m_State;
}

bool CGvGEvent::Dialog(LPOBJ lpObj, LPOBJ lpNpc)
{
	if (!gServerInfo.m_GvGEventSwitch) return false;

	if (lpNpc->Class == gServerInfo.m_GvGEventNPC &&
		lpNpc->Map == gServerInfo.m_GvGEventNPCMap &&
		lpNpc->X == gServerInfo.m_GvGEventNPCX &&
		lpNpc->Y == gServerInfo.m_GvGEventNPCY)
	{
		GCChatTargetSend(lpObj, lpNpc->Index, gMessage.GetMessage(864));
		this->CheckEnterEnabled(lpObj);
		return true;
	}

	return false;
}

bool CGvGEvent::CheckEnterEnabled(LPOBJ lpObj)
{
	if(OBJECT_RANGE(lpObj->PartyNumber) != 0)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(869));
		return 0;
	}

	if(lpObj->Guild != 0 && lpObj->Guild->WarState == 1)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(869));
		return 0;
	}

	if(lpObj->Guild == 0)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(872));
		return 0;
	}

	if(OBJECT_RANGE(lpObj->DuelUser) != 0)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(869));
		return 0;
	}

	if(lpObj->PShopOpen != 0)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(872));
		return 0;
	}


	if(this->GetEnterEnabled() == 0)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(870));
		return 0;
	}

	if (lpObj->GuildStatus == G_MASTER && this->CheckReqItems(lpObj) == 0)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(871));
		return 0;
	}

	if (this->GetUserCount() >= gServerInfo.m_GvGEventMaxUsers)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(869));
		return 0;
	}

	if (this->GetGuildCount() >= MAX_GVGEVENT_GUILD)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(869));
		return 0;
	}

	if(this->MinLevel != -1 && this->MinLevel > lpObj->Level)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(224),this->MinLevel);
		return 0;
	}

	if(this->MaxLevel != -1 && this->MaxLevel < lpObj->Level)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(230),this->MaxLevel);
		return 0;
	}

	if(this->MinReset != -1 && this->MinReset > lpObj->Reset)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(231),this->MinReset);
		return 0;
	}

	if(this->MaxReset != -1 && this->MaxReset < lpObj->Reset)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(232),this->MaxReset);
		return 0;
	}

	if(this->MinMasterReset != -1 && this->MinMasterReset > lpObj->MasterReset)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(818),this->MinMasterReset);
		return 0;
	}

	if(this->MaxMasterReset != -1 && this->MaxMasterReset < lpObj->MasterReset)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(819),this->MaxMasterReset);
		return 0;
	}

	if(lpObj->GuildStatus != G_MASTER && this->CheckGuildMaster(lpObj->GuildNumber) == 0)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(873),this->MaxMasterReset);
		return 0;
	}

	if (lpObj->GuildStatus == G_MASTER && this->AddGuild(lpObj->GuildNumber) == 0)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(869),this->MaxMasterReset);
		return 0;
	}

	gEffectManager.ClearAllEffect(lpObj);

	return this->AddUser(lpObj->Index);
}

bool CGvGEvent::CheckReqItems(LPOBJ lpObj)
{
	lpObj->ChaosLock = 1;
	int count = gItemManager.GetInventoryItemCount(lpObj,this->ReqItemIndex, this->ReqItemLevel);

	if (count < this->ReqItemCount)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(833));
		return false;
	}

	gItemManager.DeleteInventoryItemCount(lpObj,this->ReqItemIndex,this->ReqItemLevel,this->ReqItemCount);

	lpObj->ChaosLock = 0;
	return true;
}

int CGvGEvent::GetEnterEnabled() // OK
{
	return this->EnterEnabled;
}

int CGvGEvent::GetEnteredUserCount() // OK
{
	return this->GetUserCount();
}

bool CGvGEvent::CheckEnteredUser(int aIndex) // OK
{
	return ((this->GetUser(aIndex)==0)?0:1);
}

bool CGvGEvent::CheckPlayerTarget(LPOBJ lpObj) // OK
{
	if(this->GetState() == GVG_EVENT_STATE_START)
	{
		if (this->EventMap == lpObj->Map)
		{
			if(this->CheckEnteredUser(lpObj->Index) != 0)
			{
				return 1;
			}
		}
	}
	return 0;
}

bool CGvGEvent::CheckStandTarget(LPOBJ lpObj) // OK
{
	if(this->GetState() == GVG_EVENT_STATE_STAND || this->EnterEnabled == 1)
	{
		if (this->EventMap == lpObj->Map)
		{
			if(this->CheckEnteredUser(lpObj->Index) != 0)
			{
				return 1;
			}
		}
	}
	return 0;
}

bool CGvGEvent::CheckPlayerJoined(LPOBJ lpObj,LPOBJ lpTarget) // OK
{
	if(this->GetState() != GVG_EVENT_STATE_BLANK)
	{
		if (this->EventMap == lpObj->Map)
		{
			if(this->CheckEnteredUser(lpObj->Index) != 0)
			{
				return 1;
			}
		}
	}
	return 0;
}

bool CGvGEvent::CheckSelfTeam(LPOBJ lpObj,LPOBJ lpTarget)
{
	if(this->GetState() == GVG_EVENT_STATE_START)
	{
		GVG_EVENT_USER* lpUserA = this->GetUser(lpObj->Index);
		GVG_EVENT_USER* lpUserB = this->GetUser(lpTarget->Index);

		if (lpUserA == 0 || lpUserB == 0)
		{
			return 1;
		}

		if (lpUserA->Guild == lpUserB->Guild)
		{
			return 1;
		}
	}
	return 0;
}

bool CGvGEvent::AddUser(int aIndex) // OK
{
	if(OBJECT_RANGE(aIndex) == 0)
	{
		return 0;
	}

	if(this->GetUser(aIndex) != 0)
	{
		return 0;
	}

	GVG_EVENT_GUILD* lpGuild = this->GetGuild(gObj[aIndex].GuildNumber);

	if(lpGuild == 0)
	{
		return 0;
	}

	for(int n=0;n < MAX_GVGEVENT_USER;n++)
	{
		if(OBJECT_RANGE(this->User[n].Index) != 0)
		{
			continue;
		}

		this->User[n].Index = aIndex;
		this->User[n].Gate = lpGuild->Gate;
		this->User[n].Guild = lpGuild->Guild;
		this->TotalPlayer++;
		gObjMoveGate(aIndex, this->WaitingGate);
		return 1;
	}
	return 0;
}

bool CGvGEvent::AddGuild(int index) // OK
{
	if(index <= 0)
	{
		return 0;
	}

	if(this->GetGuild(index) != 0)
	{
		return 0;
	}

	for(int n=0;n < MAX_GVGEVENT_GUILD;n++)
	{
		if(this->Guild[n].Guild != -1)
		{
			continue;
		}

		this->Guild[n].Guild = index;
		this->Guild[n].Gate = this->StartGate+n;
		return 1;
	}
	return 0;
}

bool CGvGEvent::DelUser(int aIndex) // OK
{
	if(OBJECT_RANGE(aIndex) == 0)
	{
		return 0;
	}

	GVG_EVENT_USER* lpUser = this->GetUser(aIndex);

	if(lpUser == 0)
	{
		return 0;
	}

	lpUser->Reset();
	this->TotalPlayer--;
	return 1;
}

GVG_EVENT_USER* CGvGEvent::GetUser(int aIndex) // OK
{
	if(OBJECT_RANGE(aIndex) == 0)
	{
		return 0;
	}

	for(int n=0;n < MAX_GVGEVENT_USER;n++)
	{
		if(this->User[n].Index == aIndex)
		{
			return &this->User[n];
		}
	}
	return 0;
}

GVG_EVENT_GUILD* CGvGEvent::GetGuild(int aIndex) // OK
{
	if(OBJECT_RANGE(aIndex) == 0)
	{
		return 0;
	}

	for(int n=0;n < MAX_GVGEVENT_GUILD;n++)
	{
		if(this->Guild[n].Guild == aIndex)
		{
			return &this->Guild[n];
		}
	}
	return 0;
}

bool CGvGEvent::CheckGuildMaster(int index) // OK
{
	if(index <= 0)
	{
		return 0;
	}

	for(int n=0;n < MAX_GVGEVENT_GUILD;n++)
	{
		if(this->Guild[n].Guild == index)
		{
			return 1;
		}
	}
	return 0;
}

void CGvGEvent::CleanUser() // OK
{
	for(int n=0;n < MAX_GVGEVENT_USER;n++)
	{
		this->User[n].Reset();
		this->TotalPlayer = 0;
	}
}

void CGvGEvent::CleanGuild() // OK 
{
	for(int n=0;n < MAX_GVGEVENT_GUILD;n++)
	{
		this->Guild[n].Reset();
	}
}

void CGvGEvent::ClearUser() // OK
{
	for(int n=0;n < MAX_GVGEVENT_USER;n++)
	{
		if(OBJECT_RANGE(this->User[n].Index) == 0)
		{
			continue;
		}

		gObjMoveGate(this->User[n].Index,17);

		//gObjViewportListProtocolCreate(&gObj[this->User[n].Index]);
		gObjectManager.CharacterUpdateMapEffect(&gObj[this->User[n].Index]);

		this->User[n].Reset();

		this->TotalPlayer--;
	}
}

void CGvGEvent::ClearGuild() // OK
{
	for(int n=0;n < MAX_GVGEVENT_GUILD;n++)
	{
		if(this->Guild[n].Guild == -1)
		{
			continue;
		}
		this->Guild[n].Reset();
	}
}

void CGvGEvent::CheckUser() // OK
{
	for(int n=0;n < MAX_GVGEVENT_USER;n++)
	{
		if(OBJECT_RANGE(this->User[n].Index) == 0)
		{
			continue;
		}

		if(gObjIsConnected(this->User[n].Index) == 0)
		{
			this->DelUser(this->User[n].Index);
			continue;
		}

		if (gObj[this->User[n].Index].Map != this->EventMap)
		{
			this->DelUser(this->User[n].Index);
			continue;
		}
	}
}

int CGvGEvent::GetUserCount() // OK
{
	int count = 0;

	for(int n=0;n < MAX_GVGEVENT_USER;n++)
	{
		if(OBJECT_RANGE(this->User[n].Index) != 0)
		{
			count++;
		}
	}

	return count;
}

int CGvGEvent::GetGuildCount() // OK
{
	int count = 0;

	for(int n=0;n < MAX_GVGEVENT_GUILD;n++)
	{
		if(this->Guild[n].Guild != -1)
		{
			count++;
		}
	}

	return count;
}

bool CGvGEvent::GetUserRespawnLocation(LPOBJ lpObj,int* gate,int* map,int* x,int* y,int* dir,int* level) // OK
{
	if(gServerInfo.m_GvGEventSwitch == 0)
	{
		return 0;
	}

	if(this->GetState() != GVG_EVENT_STATE_START)
	{
		return 0;
	}

	GVG_EVENT_USER* lpUser = this->GetUser(lpObj->Index);

	if(lpUser == 0)
	{
		return 0;
	}

	if (lpUser->Gate != -1)
	{
		if(gGate.GetGate(lpUser->Gate,gate,map,x,y,dir,level) != 0)
		{
			return 1;
		}
	}
	else
	{
		if(gGate.GetGate(this->WaitingGate,gate,map,x,y,dir,level) != 0)
		{
			return 1;
		}
	}

	return 0;
}

void CGvGEvent::UserDieProc(LPOBJ lpObj,LPOBJ lpTarget) // OK
{
	if (gServerInfo.m_GvGEventSwitch == 0)
	{
		return;
	}

	if(this->GetState() != GVG_EVENT_STATE_START)
	{
		return;
	}

	GVG_EVENT_USER* lpUserA = this->GetUser(lpObj->Index);

	if(lpUserA == 0)
	{
		return;
	}

	GVG_EVENT_USER* lpUserB = this->GetUser(lpTarget->Index);

	if(lpUserB == 0)
	{
		return;
	}

	GVG_EVENT_GUILD* lpGuild = this->GetGuild(lpUserB->Guild);

	if(lpGuild == 0)
	{
		return;
	}

	lpGuild->Point++;

	//GUILD_INFO_STRUCT* lpGuildInfo = gGuildClass.SearchGuild_Number(lpUserB->Guild);

	this->NoticeSendToAll(0,gMessage.GetMessage(874),lpGuild->Point,lpTarget->GuildName);
}

void CGvGEvent::NoticeSendToAll(int type,char* message,...) // OK
{
	char buff[256];

	va_list arg;
	va_start(arg,message);
	vsprintf_s(buff,message,arg);
	va_end(arg);

	for(int n=0;n < MAX_GVGEVENT_USER;n++)
	{
		if(OBJECT_RANGE(this->User[n].Index) != 0)
		{
			gNotice.GCNoticeSend(this->User[n].Index,type,0,0,0,0,0,buff);
		}
	}
}

void CGvGEvent::CalcRank() // OK
{
	for(int n=0;n < MAX_GVGEVENT_GUILD;n++)
	{
		if(this->Guild[n].Guild < 0)
		{
			continue;
		}

		int rank = MAX_GVGEVENT_GUILD;

		for(int i=0;i < MAX_GVGEVENT_GUILD;i++)
		{
			if(this->Guild[i].Guild < 0)
			{
				rank--;
				continue;
			}

			if(this->Guild[n].Guild == this->Guild[i].Guild)
			{
				rank--;
				continue;
			}

			if(this->Guild[n].Point > this->Guild[i].Point)
			{
				rank--;
				continue;
			}

			if(this->Guild[n].Point == this->Guild[i].Point && n < i)
			{
				rank--;
				continue;
			}
		}

		this->Guild[n].Rank = rank;
	}

	for(int n=0;n < MAX_GVGEVENT_GUILD;n++)
	{
		if (this->Guild[n].Guild >= 0 )
		{
			this->Guild[n].Rank++;

			if (this->Guild[n].Rank == 1) 
			{
				this->Winner = this->Guild[n].Guild;

				GUILD_INFO_STRUCT* lpGuildInfo = gGuildClass.SearchGuild_Number(this->Winner);

				if (lpGuildInfo != NULL)
				{
					this->NoticeSendToAll(0,gMessage.GetMessage(876),lpGuildInfo->Name);
				}
				//GDRankingKingPlayerSaveSend(this->Char[n].Index,this->Char[n].Times);
			}
		}
	}
}

void CGvGEvent::StartGvG()
{
	CTime CurrentTime = CTime::GetTickCount();

	int hour	= (int)CurrentTime.GetHour();
	int minute	= (int)CurrentTime.GetMinute()+2;

	if (minute >= 60)
	{
		hour++;
		minute = minute-60; 
	}

	GVG_TIME info;

	info.Year = (int)CurrentTime.GetYear();

	info.Month = (int)CurrentTime.GetMonth();

	info.Day = (int)CurrentTime.GetDay();

	info.DayOfWeek = -1;

	info.Hour = hour;

	info.Minute = minute;

	info.Second = 0;

	this->m_GVGStartTime.push_back(info);

	LogAdd(LOG_EVENT,"[Set GVG Start] At %02d:%02d:00",hour,minute);

	this->Init();
}

