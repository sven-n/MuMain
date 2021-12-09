#include "stdafx.h"
#include "EventTvT.h"
#include "Guild.h"
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

CTvTEvent gTvTEvent;

CTvTEvent::CTvTEvent()
{
	this->m_TickCount = GetTickCount();
	this->m_State = 0;
	this->m_RemainTime = 0;
	this->m_StandTime = 0;
	this->m_CloseTime = 0;
	this->m_TickCount = 0;
	this->m_WarningTime = 0;
	this->m_EventTime = 0;


	this->ScoreBlue = 0;
	this->ScoreRed = 0;
	this->TotalPlayer = 0;

	this->Coin1 = 0;
	this->Coin2 = 0;
	this->Coin3 = 0;

	this->CleanUser();

}

CTvTEvent::~CTvTEvent()
{
	
}

void CTvTEvent::Init()
{
	if(gServerInfo.m_TvTEventSwitch == 0)
	{
		this->SetState(TVT_EVENT_STATE_BLANK);
	}
	else
	{
		this->SetState(TVT_EVENT_STATE_EMPTY);
	}
}

void CTvTEvent::Clear()
{
	this->ScoreBlue = 0;
	this->ScoreRed = 0;
	this->TotalPlayer = 0;

	this->CleanUser();
}

void CTvTEvent::Load(char* path)
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

	this->m_TVTStartTime.clear();

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

					TVT_TIME info;

					info.Year = lpMemScript->GetNumber();

					info.Month = lpMemScript->GetAsNumber();

					info.Day = lpMemScript->GetAsNumber();

					info.DayOfWeek = lpMemScript->GetAsNumber();

					info.Hour = lpMemScript->GetAsNumber();

					info.Minute = lpMemScript->GetAsNumber();

					info.Second = lpMemScript->GetAsNumber();

					this->m_TVTStartTime.push_back(info);

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

					this->BluTeamGate = lpMemScript->GetAsNumber();

					this->RedTeamGate = lpMemScript->GetAsNumber();

					this->SetSkin = lpMemScript->GetAsNumber();

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

void CTvTEvent::MainProc()
{
	if((GetTickCount()-this->m_TickCount) >= 1000)
	{
		this->m_TickCount = GetTickCount();

		this->m_RemainTime = (int)difftime(this->TargetTime,time(0));

	if(gServerInfo.m_TvTEventSwitch == 0)
	{
		if (gServerDisplayer.EventTvT != -1)
		{
			gServerDisplayer.EventTvT = -1;
		}
	}
	else 
	{
		if (this->m_State == TVT_EVENT_STATE_EMPTY)
		{
			gServerDisplayer.EventTvT = this->m_RemainTime;
		}
		else 
		{
			if (gServerDisplayer.EventTvT != 0)
			{
				gServerDisplayer.EventTvT = 0;
			}
		}
	}

		switch(this->m_State)
		{
			case TVT_EVENT_STATE_BLANK:
				this->ProcState_BLANK();
				break;
			case TVT_EVENT_STATE_EMPTY:
				this->ProcState_EMPTY();
				break;
			case TVT_EVENT_STATE_STAND:
				this->ProcState_STAND();
				break;
			case TVT_EVENT_STATE_START:
				this->ProcState_START();
				break;
			case TVT_EVENT_STATE_CLEAN:
				this->ProcState_CLEAN();
				break;
		}
	}
}

void CTvTEvent::ProcState_BLANK() // OK
{

}

void CTvTEvent::ProcState_EMPTY() // OK
{
	if(this->m_RemainTime > 0 && this->m_RemainTime <= (this->m_WarningTime*60))
	{
		this->CheckUser();

		this->EnterEnabled = 1;

		if((this->AlarmMinSave=(((this->m_RemainTime%60)==0)?((this->m_RemainTime/60)-1):(this->m_RemainTime/60))) != this->AlarmMinLeft)
		{
			this->AlarmMinLeft = this->AlarmMinSave;

			gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(825),(this->AlarmMinLeft+1));
			//gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(826));
		}
	}

	if(this->m_RemainTime <= 0)
	{
		gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(827));
		this->NoticeSendToAll(1,gMessage.GetMessage(828),this->m_StandTime);
		this->SetState(TVT_EVENT_STATE_STAND);
	}
}

void CTvTEvent::ProcState_STAND() // OK
{
	this->CheckUser();

	if(this->GetUserCount() < gServerInfo.m_TvTEventMinUsers)
	{
		this->NoticeSendToAll(0,gMessage.GetMessage(830));
		this->SetState(TVT_EVENT_STATE_EMPTY);
		return;
	}

	if(this->m_RemainTime <= 0)
	{
		if (this->TotalPlayer >= gServerInfo.m_TvTEventMinUsers)
		{
			this->DivisionTeam();
			this->NoticeSendToAll(0,gMessage.GetMessage(829));
			this->SetState(TVT_EVENT_STATE_START);
		}
		else
		{
			this->NoticeSendToAll(0,gMessage.GetMessage(830));
			this->SetState(TVT_EVENT_STATE_EMPTY);
		}
	}
}

void CTvTEvent::ProcState_START() // OK
{
	this->CheckUser();

	if(this->GetUserCount() == 0)
	{
		this->SetState(TVT_EVENT_STATE_EMPTY);
		return;
	}

	if(this->GetUserCount() == 1)
	{
		this->SetState(TVT_EVENT_STATE_CLEAN);
		return;
	}

	if(this->m_RemainTime <= 0)
	{
		this->NoticeSendToAll(0,gMessage.GetMessage(830));
		this->SetState(TVT_EVENT_STATE_CLEAN);
	}
}

void CTvTEvent::ProcState_CLEAN() // OK
{
	this->CheckUser();

	if(this->m_RemainTime <= 0)
	{
		this->SetState(TVT_EVENT_STATE_EMPTY);
	}
}

void CTvTEvent::SetState(int state) // OK
{
	switch((this->m_State=state))
	{
		case TVT_EVENT_STATE_BLANK:
			this->SetState_BLANK();
			break;
		case TVT_EVENT_STATE_EMPTY:
			this->SetState_EMPTY();
			break;
		case TVT_EVENT_STATE_STAND:
			this->SetState_STAND();
			break;
		case TVT_EVENT_STATE_START:
			this->SetState_START();
			break;
		case TVT_EVENT_STATE_CLEAN:
			this->SetState_CLEAN();
			break;
	}
}

void CTvTEvent::SetState_BLANK() // OK
{

}

void CTvTEvent::SetState_EMPTY() // OK
{
	this->EnterEnabled = 0;
	this->AlarmMinSave = -1;
	this->AlarmMinLeft = -1;

	this->ClearUser();
	this->ScoreBlue = 0;
	this->ScoreRed = 0;

	this->CheckSync();
}

void CTvTEvent::SetState_STAND() // OK
{
	this->EnterEnabled = 0;
	this->AlarmMinSave = -1;
	this->AlarmMinLeft = -1;

	this->m_RemainTime = this->m_StandTime*60;

	this->TargetTime = (int)(time(0)+this->m_RemainTime);
}

void CTvTEvent::SetState_START() // OK
{
	this->EnterEnabled = 0;
	this->AlarmMinSave = -1;
	this->AlarmMinLeft = -1;

	this->m_RemainTime = this->m_EventTime*60;

	this->TargetTime = (int)(time(0)+this->m_RemainTime);
}

void CTvTEvent::SetState_CLEAN() // OK
{
	this->EnterEnabled = 0;
	this->AlarmMinSave = -1;
	this->AlarmMinLeft = -1;

	int TeamReward = 0;

	if(this->ScoreBlue > this->ScoreRed)
	{
		TeamReward = TVT_BLUETEAM;
		this->NoticeSendToAll(0,"Win BLUE Team!");
	}
	else if(this->ScoreRed > this->ScoreBlue)
	{
		TeamReward = TVT_REDTEAM;
		this->NoticeSendToAll(0,"Win RED Team!");
	}


	for(int n=0;n < MAX_TVTEVENT_USER;n++)
	{
		if(OBJECT_RANGE(this->User[n].Index) != 0)
		{
			gObjViewportListProtocolCreate(&gObj[this->User[n].Index]);
			gObjectManager.CharacterUpdateMapEffect(&gObj[this->User[n].Index]);

			if(this->User[n].Team == TeamReward)
			{
				LPOBJ lpObj = &gObj[this->User[n].Index];

				if (this->Coin1 > 0 || this->Coin2 > 0 || this->Coin3 > 0)
				{	
					GDSetCoinSend(lpObj->Index, this->Coin1, this->Coin2, this->Coin3,"TvTEvent");
				}

				GDRankingTvTEventSaveSend(lpObj->Index, this->User[n].Kills, this->User[n].Deaths);
			}
			//else
			//{
				//GDRankingCustomArenaSaveSend(lpInfo->User[n].Index,lpInfo->Index,lpInfo->User[n].Score,lpInfo->User[n].Rank);

				//gNotice.GCNoticeSend(lpInfo->User[n].Index,1,0,0,0,0,0,this->m_CustomArenaText7,lpInfo->RuleInfo.Name,lpInfo->User[n].Rank,lpInfo->User[n].Score);
			//}
		}
	}

	this->m_RemainTime = this->m_CloseTime*60;

	this->TargetTime = (int)(time(0)+this->m_RemainTime);
}

void CTvTEvent::CheckSync() // OK
{
	if(this->m_TVTStartTime.empty() != 0)
	{
		this->SetState(TVT_EVENT_STATE_BLANK);
		return;
	}

	CTime ScheduleTime;

	CScheduleManager ScheduleManager;

	for(std::vector<TVT_TIME>::iterator it=this->m_TVTStartTime.begin();it != this->m_TVTStartTime.end();it++)
	{
		ScheduleManager.AddSchedule(it->Year,it->Month,it->Day,it->Hour,it->Minute,it->Second,it->DayOfWeek);
	}

	if(ScheduleManager.GetSchedule(&ScheduleTime) == 0)
	{
		this->SetState(TVT_EVENT_STATE_BLANK);
		return;
	}

	this->m_RemainTime = (int)difftime(ScheduleTime.GetTime(),time(0));

	this->TargetTime = (int)ScheduleTime.GetTime();
}

int CTvTEvent::GetState() // OK
{
	return this->m_State;
}

bool CTvTEvent::Dialog(LPOBJ lpObj, LPOBJ lpNpc)
{
	if (!gServerInfo.m_TvTEventSwitch) return false;

	if (lpNpc->Class == gServerInfo.m_TvTEventNPC &&
		lpNpc->Map == gServerInfo.m_TvTEventNPCMap &&
		lpNpc->X == gServerInfo.m_TvTEventNPCX &&
		lpNpc->Y == gServerInfo.m_TvTEventNPCY)
	{
		GCChatTargetSend(lpObj, lpNpc->Index, gMessage.GetMessage(826));
		this->CheckEnterEnabled(lpObj);
		return 1;
	}

	return false;
}

bool CTvTEvent::CheckEnterEnabled(LPOBJ lpObj)
{
	
	if(OBJECT_RANGE(lpObj->PartyNumber) != 0)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(831));
		return 0;
	}

	if(lpObj->Guild != 0 && lpObj->Guild->WarState == 1)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(831));
		return 0;
	}

	if(OBJECT_RANGE(lpObj->DuelUser) != 0)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(831));
		return 0;
	}

	if(lpObj->PShopOpen != 0)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(831));
		return 0;
	}


	if(this->GetEnterEnabled() == 0)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(832));
		return 0;
	}

	if (this->CheckReqItems(lpObj) == 0)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(833));
		return 0;
	}

	if (this->GetUserCount() >= gServerInfo.m_TvTEventMaxUsers)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(831));
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

	gEffectManager.ClearAllEffect(lpObj);

	return this->AddUser(lpObj->Index);
}

bool CTvTEvent::CheckReqItems(LPOBJ lpObj)
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

int CTvTEvent::GetEnterEnabled() // OK
{
	return this->EnterEnabled;
}

int CTvTEvent::GetEnteredUserCount() // OK
{
	return this->GetUserCount();
}

bool CTvTEvent::CheckEnteredUser(int aIndex) // OK
{
	return ((this->GetUser(aIndex)==0)?0:1);
}

bool CTvTEvent::CheckPlayerTarget(LPOBJ lpObj) // OK
{
		if(this->GetState() == TVT_EVENT_STATE_START)
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

bool CTvTEvent::CheckPlayerJoined(LPOBJ lpObj,LPOBJ lpTarget) // OK
{
	if(this->GetState() != TVT_EVENT_STATE_BLANK)
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

bool CTvTEvent::CheckSelfTeam(LPOBJ lpObj,LPOBJ lpTarget)
{
	if(this->GetState() == TVT_EVENT_STATE_START)
	{
		TVT_EVENT_USER* lpUserA = this->GetUser(lpObj->Index);
		TVT_EVENT_USER* lpUserB = this->GetUser(lpTarget->Index);

		if (lpUserA == 0 || lpUserB == 0)
		{
			return 1;
		}

		if (lpUserA->Team == lpUserB->Team)
		{
			return 1;
		}
	}
	return 0;
}

bool CTvTEvent::AddUser(int aIndex) // OK
{
	if(OBJECT_RANGE(aIndex) == 0)
	{
		return 0;
	}

	if(this->GetUser(aIndex) != 0)
	{
		return 0;
	}

	for(int n=0;n < MAX_TVTEVENT_USER;n++)
	{
		if(OBJECT_RANGE(this->User[n].Index) != 0)
		{
			continue;
		}

		this->User[n].Index = aIndex;
		this->User[n].Available = 1;
		this->TotalPlayer++;
		gObjMoveGate(aIndex, this->WaitingGate);
		return 1;
	}
	return 0;
}

bool CTvTEvent::DelUser(int aIndex) // OK
{
	if(OBJECT_RANGE(aIndex) == 0)
	{
		return 0;
	}

	TVT_EVENT_USER* lpUser = this->GetUser(aIndex);

	if(lpUser == 0)
	{
		return 0;
	}

	lpUser->Reset();
	this->TotalPlayer--;
	return 1;
}

TVT_EVENT_USER* CTvTEvent::GetUser(int aIndex) // OK
{
	if(OBJECT_RANGE(aIndex) == 0)
	{
		return 0;
	}

	for(int n=0;n < MAX_TVTEVENT_USER;n++)
	{
		if(this->User[n].Index == aIndex)
		{
			return &this->User[n];
		}
	}
	return 0;
}

void CTvTEvent::CleanUser() // OK
{
	for(int n=0;n < MAX_TVTEVENT_USER;n++)
	{
		this->User[n].Reset();
		this->TotalPlayer = 0;
	}
}

void CTvTEvent::ClearUser() // OK
{
	for(int n=0;n < MAX_TVTEVENT_USER;n++)
	{
		if(OBJECT_RANGE(this->User[n].Index) == 0)
		{
			continue;
		}

		gObjMoveGate(this->User[n].Index,17);

		if (this->SetSkin == 1)
		{
			gObj[this->User[n].Index].Change = -1;
		}

		gObjViewportListProtocolCreate(&gObj[this->User[n].Index]);
		gObjectManager.CharacterUpdateMapEffect(&gObj[this->User[n].Index]);

		this->User[n].Reset();

		this->TotalPlayer--;
	}
}

void CTvTEvent::CheckUser() // OK
{
	for(int n=0;n < MAX_TVTEVENT_USER;n++)
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

int CTvTEvent::GetUserCount() // OK
{
	int count = 0;

	for(int n=0;n < MAX_TVTEVENT_USER;n++)
	{
		if(OBJECT_RANGE(this->User[n].Index) != 0)
		{
			count++;
		}
	}

	return count;
}

bool CTvTEvent::GetUserRespawnLocation(LPOBJ lpObj,int* gate,int* map,int* x,int* y,int* dir,int* level) // OK
{
	if(gServerInfo.m_TvTEventSwitch == 0)
	{
		return 0;
	}

	if(this->GetState() != TVT_EVENT_STATE_START)
	{
		return 0;
	}

	TVT_EVENT_USER* lpUser = this->GetUser(lpObj->Index);

	if(lpUser == 0)
	{
		return 0;
	}

	if (lpUser->Team == TVT_BLUETEAM)
	{
		if(gGate.GetGate(this->BluTeamGate,gate,map,x,y,dir,level) != 0)
		{
			return 1;
		}
	}
	else if (lpUser->Team == TVT_REDTEAM)
	{
		if(gGate.GetGate(this->RedTeamGate,gate,map,x,y,dir,level) != 0)
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

void CTvTEvent::DivisionTeam()
{
	int iTeam = 1;
	int Index = 0;

	for(int i=0;i<MAX_TVTEVENT_USER;i++)
	{
		Index = this->ReturnMaxReset();
		this->User[i].Team = iTeam;
		if(iTeam == 1) iTeam = 2;
		else iTeam = 1;
	}

	for(int n=0;n<MAX_TVTEVENT_USER;n++)
	{
		if (this->User[n].Team == 1)
		{
			gObjMoveGate(this->User[n].Index, this->BluTeamGate);

			if (this->SetSkin == 1)
			{
				gObj[this->User[n].Index].Change = 405;
			}

			gObjViewportListProtocolCreate(&gObj[this->User[n].Index]);
			gObjectManager.CharacterUpdateMapEffect(&gObj[this->User[n].Index]);

			gNotice.GCNoticeSend(this->User[n].Index,0,0,0,0,0,0,"[BlueTeam]: %d Vs [RedTeam]: %d",this->ScoreBlue,this->ScoreRed);
		}
		else if (this->User[n].Team == 2)
		{
			gObjMoveGate(this->User[n].Index, this->RedTeamGate);

			if (this->SetSkin == 1)
			{
				gObj[this->User[n].Index].Change = 404;
			}

			gObjViewportListProtocolCreate(&gObj[this->User[n].Index]);
			gObjectManager.CharacterUpdateMapEffect(&gObj[this->User[n].Index]);

			gNotice.GCNoticeSend(this->User[n].Index,0,0,0,0,0,0,"[RedTeam]: %d Vs [BlueTeam]: %d",this->ScoreRed,this->ScoreBlue);
		}
	}
}

int CTvTEvent::ReturnMaxReset()
{
	int MaxRes = 0;
	int Index = 0;
	int UserIndex = 0;

	for(int n=0;n < MAX_TVTEVENT_USER;n++)
	{
		LPOBJ lpObj = &gObj[this->User[n].Index];
		if(OBJECT_RANGE(lpObj->Index) == 0)
		{
			continue;
		}

		if (!this->User[n].Available)
		{
			continue;
		}

		if(lpObj->Reset >= MaxRes)
		{
			MaxRes = lpObj->Reset;
			Index = lpObj->Index;
			UserIndex = n;
		}
	}
	this->User[UserIndex].Available = 0;
	return Index;
}

void CTvTEvent::UserDieProc(LPOBJ lpObj,LPOBJ lpTarget) // OK
{
	if (gServerInfo.m_TvTEventSwitch == 0)
	{
		return;
	}

	if(this->GetState() != TVT_EVENT_STATE_START)
	{
		return;
	}

	TVT_EVENT_USER* lpUserA = this->GetUser(lpObj->Index);

	if(lpUserA == 0)
	{
		return;
	}

	TVT_EVENT_USER* lpUserB = this->GetUser(lpTarget->Index);

	if(lpUserB == 0)
	{
		return;
	}

	if(lpUserA->Team == TVT_BLUETEAM && lpUserB->Team == TVT_REDTEAM)
	{
		this->ScoreRed++;
	}
	else if(lpUserA->Team == TVT_REDTEAM && lpUserB->Team == TVT_BLUETEAM) 
	{
		this->ScoreBlue++;
	}

	lpUserA->Deaths++;
	lpUserB->Kills++;

	gNotice.GCNoticeSend(lpUserA->Index, 1, 0, 0, 0, 0, 0, "You died %d times", lpUserA->Deaths);
	gNotice.GCNoticeSend(lpUserB->Index, 1, 0, 0, 0, 0, 0, "You kill %d users", lpUserB->Kills);

	for (int i=0;i<MAX_TVTEVENT_USER;i++)
	{

		if(OBJECT_RANGE(this->User[i].Index) == 0)
		{
			continue;
		}

		if(this->User[i].Team == TVT_BLUETEAM)
		{
			gNotice.GCNoticeSend(this->User[i].Index,0,0,0,0,0,0,"[BlueTeam]: %d Vs [RedTeam]: %d",this->ScoreBlue,this->ScoreRed);

		}
		else if(this->User[i].Team == TVT_REDTEAM)
		{
			gNotice.GCNoticeSend(this->User[i].Index,0,0,0,0,0,0,"[RedTeam]: %d Vs [BlueTeam]: %d",this->ScoreRed,this->ScoreBlue);
		}
	}
}

void CTvTEvent::NoticeSendToAll(int type,char* message,...) // OK
{
	char buff[256];

	va_list arg;
	va_start(arg,message);
	vsprintf_s(buff,message,arg);
	va_end(arg);

	for(int n=0;n < MAX_TVTEVENT_USER;n++)
	{
		if(OBJECT_RANGE(this->User[n].Index) != 0)
		{
			gNotice.GCNoticeSend(this->User[n].Index,type,0,0,0,0,0,buff);
		}
	}
}

void CTvTEvent::StartTvT()
{
	CTime CurrentTime = CTime::GetTickCount();

	int hour	= (int)CurrentTime.GetHour();
	int minute	= (int)CurrentTime.GetMinute()+2;

	if (minute >= 60)
	{
		hour++;
		minute = minute-60; 
	}

	TVT_TIME info;

	info.Year = (int)CurrentTime.GetYear();

	info.Month = (int)CurrentTime.GetMonth();

	info.Day = (int)CurrentTime.GetDay();

	info.DayOfWeek = -1;

	info.Hour = hour;

	info.Minute = minute;

	info.Second = 0;

	this->m_TVTStartTime.push_back(info);

	LogAdd(LOG_EVENT,"[Set TVT Start] At %02d:%02d:00",hour,minute);

	this->Init();
}