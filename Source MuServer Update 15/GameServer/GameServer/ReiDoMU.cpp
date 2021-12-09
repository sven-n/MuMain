// ReiDoMu.cpp: Evento rei do MU by louis || www.jogandomu.com.br
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DSProtocol.h"
#include "GuildClass.h"
#include "Map.h"
#include "MapServerManager.h"
#include "MemScript.h"
#include "Message.h"
#include "ServerInfo.h"
#include "SkillManager.h"
#include "ScheduleManager.h"
#include "Notice.h"
#include "Util.h"
#include "CommandManager.h"
#include "DSProtocol.h"
#include "GameMain.h"
#include "Guild.h"
#include "ReiDoMU.h"

CReiDoMU gReiDoMU;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CReiDoMU::CReiDoMU() // OK
{
	this->m_State = REI_STATE_BLANK;
	this->m_RemainTime = 0;
	this->m_TargetTime = 0;
	this->m_TickCount = GetTickCount();
	this->MinutesLeft = -1;

	this->GRewardValue1 = 0;
	this->GRewardValue2 = 0;
	this->GRewardValue3 = 0;
	this->CRewardValue1 = 0;
	this->CRewardValue2 = 0;
	this->CRewardValue3 = 0;

	this->m_ReiDoMUStartTime.clear();

	this->Init();
}

CReiDoMU::~CReiDoMU() // OK
{

}

void CReiDoMU::Init() // OK
{   
	this->Clear();

	if(gServerInfo.m_ReiDoMUEvent == 0)
	{
		this->SetState(REI_STATE_BLANK);
	}
	else
	{
		this->SetState(REI_STATE_EMPTY);
	}


}

void CReiDoMU::Load(char* path) // OK
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

	this->m_ReiDoMUStartTime.clear();

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

					this->m_EventTime = lpMemScript->GetAsNumber();

				}
				else if(section == 1)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					REI_START_TIME info;

					info.Year = lpMemScript->GetNumber();

					info.Month = lpMemScript->GetAsNumber();

					info.Day = lpMemScript->GetAsNumber();

					info.DayOfWeek = lpMemScript->GetAsNumber();

					info.Hour = lpMemScript->GetAsNumber();

					info.Minute = lpMemScript->GetAsNumber();

					info.Second = lpMemScript->GetAsNumber();

					this->m_ReiDoMUStartTime.push_back(info);
				}
				else if(section == 2)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					this->GRewardValue1 = lpMemScript->GetNumber();

					this->GRewardValue2 = lpMemScript->GetAsNumber();

					this->GRewardValue3 = lpMemScript->GetAsNumber();

				}
				else if(section == 3)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					this->CRewardValue1 = lpMemScript->GetNumber();

					this->CRewardValue2 = lpMemScript->GetAsNumber();

					this->CRewardValue3 = lpMemScript->GetAsNumber();

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


void CReiDoMU::MainProc() // OK
{
	DWORD elapsed = GetTickCount()-this->m_TickCount;

	if(elapsed < 1000)
	{
		return;
	}

	this->m_TickCount = GetTickCount();

	this->m_RemainTime = (int)difftime(this->m_TargetTime,time(0));

	if(gServerInfo.m_ReiDoMUEvent == 0)
	{
		if (gServerDisplayer.EventKing != -1)
		{
			gServerDisplayer.EventKing = -1;
		}
	}
	else 
	{
		if (this->m_State == REI_STATE_EMPTY)
		{
			gServerDisplayer.EventKing = this->m_RemainTime;
		}
		else 
		{
			if (gServerDisplayer.EventKing != 0)
			{
				gServerDisplayer.EventKing = 0;
			}
		}
	}

	switch(this->m_State)
	{
		case REI_STATE_BLANK:
			this->ProcState_BLANK();
			break;
		case REI_STATE_EMPTY:
			this->ProcState_EMPTY();
			break;
		case REI_STATE_START:
			this->ProcState_START();
			break;
	}

	if(this->m_RemainTimeWinner > 0 )
	{ 
		if (this->WinnerNumber >= 0)
		{

			if(this->m_RemainTimeWinner == 40)
			{
				gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(718));
			}

			if(this->m_RemainTimeWinner == 38)
			{
				gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(718));
			}

			if(this->m_RemainTimeWinner == 28)
			{
				GUILD_INFO_STRUCT* lpGuildInfo = gGuildClass.SearchGuild_Number(this->WinnerNumber);

				if (lpGuildInfo == 0)
				{

				}
				else{
					LogAdd(LOG_EVENT,"[KING OF MU] Guild Winner: %s", lpGuildInfo->Name);
					gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(719),lpGuildInfo->Name);
					gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(720),this->WinnerTime);
					gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(721));
				    gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(722));

					this->GDRankingKingGuildSaveSend(lpGuildInfo->Name,this->WinnerTime);

					for(int n=OBJECT_START_USER;n < MAX_OBJECT;n++)
					{
						if(gObjIsConnectedGP(n) != 0 && gObj[n].Type == OBJECT_USER)
						{
							if (strcmp(gObj[n].GuildName,lpGuildInfo->Name) == 0)
							{
								LogAdd(LOG_EVENT,"[KING OF MU] Member Reward: %s", gObj[n].Name);
								GDSetCoinSend(gObj[n].Index, this->GRewardValue1, this->GRewardValue2, this->GRewardValue3,"KingOfMu");
							}
						}
					}

				}
				
			}

			if(this->m_RemainTimeWinner == 20)
			{
				gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(723));
			}
			if(this->m_RemainTimeWinner == 16)
			{
				if (this->rank1 >= 0)
				{
					LPOBJ lpObj = &gObj[this->rank1];
					gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(724),1,lpObj->Name,lpObj->GuildName);
					//Reward
					LogAdd(LOG_EVENT,"[KING OF MU] 1° %s (Guild: %s)", lpObj->Name, lpObj->GuildName);
					GDSetCoinSend(lpObj->Index, this->CRewardValue1, this->CRewardValue2, this->CRewardValue3,"KingOfMu");
					
				}
			}
			if(this->m_RemainTimeWinner == 14)
			{
				if (this->rank2 >= 0)
				{
					LPOBJ lpObj = &gObj[this->rank2];
					gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(724),2,lpObj->Name,lpObj->GuildName);
					//Reward
					LogAdd(LOG_EVENT,"[KING OF MU] 2° %s (Guild: %s)", lpObj->Name, lpObj->GuildName);
					GDSetCoinSend(lpObj->Index, this->CRewardValue1, this->CRewardValue2, this->CRewardValue3,"KingOfMu");
				}
			}
			if(this->m_RemainTimeWinner == 12)
			{
				if (this->rank3 >= 0)
				{
					LPOBJ lpObj = &gObj[this->rank3];
					gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(724),3,lpObj->Name,lpObj->GuildName);
					//Reward
					LogAdd(LOG_EVENT,"[KING OF MU] 3° %s (Guild: %s)", lpObj->Name, lpObj->GuildName);
					GDSetCoinSend(lpObj->Index, this->CRewardValue1, this->CRewardValue2, this->CRewardValue3,"KingOfMu");
				}
			}
			if(this->m_RemainTimeWinner == 10)
			{
				if (this->rank4 >= 0)
				{
					LPOBJ lpObj = &gObj[this->rank4];
					gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(724),4,lpObj->Name,lpObj->GuildName);
					//Reward
					LogAdd(LOG_EVENT,"[KING OF MU] 4° %s (Guild: %s)", lpObj->Name, lpObj->GuildName);
					GDSetCoinSend(lpObj->Index, this->CRewardValue1, this->CRewardValue2, this->CRewardValue3,"KingOfMu");
				}
			}
			if(this->m_RemainTimeWinner == 8)
			{
				if (this->rank5 >= 0)
				{
					LPOBJ lpObj = &gObj[this->rank5];
					gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(724),5,lpObj->Name,lpObj->GuildName);
					//Reward
					LogAdd(LOG_EVENT,"[KING OF MU] 5° %s (Guild: %s)", lpObj->Name, lpObj->GuildName);
					GDSetCoinSend(lpObj->Index, this->CRewardValue1, this->CRewardValue2, this->CRewardValue3,"KingOfMu");
				}
			}

			if(this->m_RemainTimeWinner == 2)
			{
				gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(725));
			}

			this->m_RemainTimeWinner--;

			if(this->m_RemainTimeWinner <= 0)
			{
				gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(501));
				LogAdd(LOG_EVENT,"[KING OF MU] Finish");

				this->Clear();
			}
		}
		else
		{
			LogAdd(LOG_EVENT,"[KING OF MU] Finish - No Winner");
			gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(726));
			this->m_RemainTimeWinner = 0;
			this->Clear();
		}


	}

}

void CReiDoMU::ProcState_BLANK() // OK
{

}

void CReiDoMU::ProcState_EMPTY() // OK
{

	if(this->m_RemainTime > 0 && this->m_RemainTime <= (this->m_WarningTime*60))
	{
		int minutes = this->m_RemainTime/60;

		if((this->m_RemainTime%60) == 0)
		{
			minutes--;
		}

		if(this->MinutesLeft != minutes)
		{
			this->MinutesLeft = minutes;

				gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(502),(MinutesLeft+1));
		}
	}

	if(this->m_RemainTime > 0 && this->m_RemainTime <= 5)
	{
		gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(504),m_RemainTime);
	}

	if(this->m_RemainTime <= 0)
	{
		gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(500));

		this->SetState(REI_STATE_START);
	}
}

void CReiDoMU::ProcState_START() // OK
{
	if(this->m_RemainTime > 0 && this->m_RemainTime <= 300)
	{
		int minutes = this->m_RemainTime/60;

		if((this->m_RemainTime%60) == 0)
		{
			minutes--;
			this->AddTime();
		}

		if(this->MinutesLeft != minutes)
		{
			this->MinutesLeft = minutes;

				gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(511),(MinutesLeft+1));
		}
	}

	if(this->m_RemainTime <= 0)
	{
		this->CheckChar();
		gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(501));
		this->CalcUserRank();
		this->SetGuildTop();
		this->m_RemainTimeWinner = 40;
		this->m_Active = 0;
		this->SetState(REI_STATE_EMPTY);

	}
}

void CReiDoMU::SetState(int state) // OK
{
	this->m_State = state;

	switch(this->m_State)
	{
		case REI_STATE_BLANK:
			this->SetState_BLANK();
			break;
		case REI_STATE_EMPTY:
			this->SetState_EMPTY();
			break;
		case REI_STATE_START:
			this->SetState_START();
			break;
	}
}

void CReiDoMU::SetState_BLANK() // OK
{

}

void CReiDoMU::SetState_EMPTY() // OK
{
	this->CheckSync();
}

void CReiDoMU::SetState_START() // OK
{
	this->m_Active = 1;

	this->m_RemainTime = this->m_EventTime*60;

	this->m_TargetTime = (int)(time(0)+this->m_RemainTime);

	LogAdd(LOG_EVENT,"[KING OF MU] Start");

}

void CReiDoMU::CheckSync() // OK
{

	if(this->m_ReiDoMUStartTime.empty() != 0)
	{
		this->SetState(REI_STATE_BLANK);
		return;
	}

	CTime ScheduleTime;

	CScheduleManager ScheduleManager;

	for(std::vector<REI_START_TIME>::iterator it=this->m_ReiDoMUStartTime.begin();it != this->m_ReiDoMUStartTime.end();it++)
	{
		ScheduleManager.AddSchedule(it->Year,it->Month,it->Day,it->Hour,it->Minute,it->Second,it->DayOfWeek);
	}

	if(ScheduleManager.GetSchedule(&ScheduleTime) == 0)
	{
		this->SetState(REI_STATE_BLANK);
		return;
	}

	this->m_RemainTime = (int)difftime(ScheduleTime.GetTime(),time(0));

	this->m_TargetTime = (int)ScheduleTime.GetTime();

}

void CReiDoMU::Clear()
{
	for(int n=0;n < MAX_REI_GUILD;n++)
	{
			this->Guild[n].Reset();
	}

	for(int n=0;n < MAX_REI_CHAR;n++)
	{
			this->Char[n].Reset();
	}

	this->m_RemainTimeWinner = 0;

	this->WinnerNumber	= -1;
	this->WinnerTime	= 0;
	this->rank1	= -1;
	this->rank2	= -1;
	this->rank3	= -1;
	this->rank4	= -1;
	this->rank5	= -1;
}

void CReiDoMU::CommandReiDoMU(LPOBJ lpObj,char* arg) // OK
{

	GUILD_INFO_STRUCT * lpGuildInfo	= lpObj->Guild;

	if(gServerInfo.m_ReiDoMUEvent == 0) //Verifica se o evento está ativo
	{
		return;
	}

	if(this->m_Active == 0) //Verifica se o evento está em andamento
	{
		return;
	}

	if( lpGuildInfo == NULL )
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(506));
		return;
	}

	if(lpObj->GuildNumber == 0) //Verifica se o personagem faz parte de uma guild
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(506));
		return;
	}

	if(strcmp(lpObj->GuildName,this->m_GuildOwner) == 0) //Verifica se a guild do personagem já é dona do castelo
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(507));
		return;
	}

	if(lpGuildInfo->TotalCount < gServerInfo.m_ReiDoMUGuildMinMembers) //Verifica quantidade de membros na guild
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(509),gServerInfo.m_ReiDoMUGuildMinMembers);
		return;
	}

	if(lpObj->Map != gServerInfo.m_ReiDoMUMap || lpObj->X != gServerInfo.m_ReiDoMUCordX || lpObj->Y != gServerInfo.m_ReiDoMUCordY) //Verifica se o personagem está na coordenada correta
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(508));
		return;	
	}

	this->SetGuild(lpObj->GuildNumber);

	this->SetChar(lpObj->Index);

	gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(503),lpObj->GuildName);

    memcpy(this->m_GuildOwner,lpObj->GuildName,sizeof(this->m_GuildOwner));

	gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(510));

	LogAdd(LOG_EVENT,"[KING OF MU] Conquered by %s",lpObj->Name);
}

void CReiDoMU::SetGuild(int gIndex) // OK
{

	if(this->GetGuild(gIndex) == 0)
	{
		this->AddGuild(gIndex);
	}

	for(int n=0;n < MAX_REI_GUILD;n++)
	{
		if(Guild[n].Number == gIndex)
		{
			this->Guild[n].Owner = 1;
		}
		else 
		{
			this->Guild[n].Owner = 0;
		}
	}
}

void CReiDoMU::AddGuild(int gIndex) // OK
{
	for(int n=0;n < MAX_REI_GUILD;n++)
	{
		if(this->Guild[n].Number >= 0)
		{
			continue;
		}

		this->Guild[n].Number	= gIndex;
		this->Guild[n].Owner	= 0;
		this->Guild[n].Time		= 0;
		return;
	}
}

bool CReiDoMU::GetGuild(int gIndex) // OK
{
	for(int n=0;n < MAX_REI_GUILD;n++)
	{
		if(this->Guild[n].Number == gIndex)
		{
			return 1;
		}
	}
	return 0;
}

void CReiDoMU::SetGuildTop() // OK
{
	for(int n=0;n < MAX_REI_GUILD;n++)
	{
		if(this->Guild[n].Time > this->WinnerTime)
		{
			this->WinnerNumber	= this->Guild[n].Number;
			this->WinnerTime	= this->Guild[n].Time;
		}
	}
}

void CReiDoMU::AddTime() // OK
{
	for(int n=0;n < MAX_REI_GUILD;n++)
	{
		if(this->Guild[n].Owner == 1)
		{
			this->Guild[n].Time++;
		}
	}
}

void CReiDoMU::SetChar(int cIndex) // OK
{

	if(this->GetChar(cIndex) == 0)
	{
		this->AddChar(cIndex);
	}

	for(int n=0;n < MAX_REI_CHAR;n++)
	{
		if(Char[n].Index == cIndex)
		{
			this->Char[n].Times++;
		}
	}
}

void CReiDoMU::AddChar(int cIndex) // OK
{
	for(int n=0;n < MAX_REI_CHAR;n++)
	{
		if(this->Char[n].Index >= 0)
		{
			continue;
		}

		this->Char[n].Index	= cIndex;
		memcpy(this->Char[n].Name,gObj[cIndex].Name,sizeof(gObj[cIndex].Name));
		this->Char[n].Times	= 0;
		this->Char[n].Rank	= 0;
		return;
	}
}

bool CReiDoMU::GetChar(int cIndex) // OK
{
	for(int n=0;n < MAX_REI_CHAR;n++)
	{
		if(this->Char[n].Index == cIndex)
		{
			return 1;
		}
	}
	return 0;
}

void CReiDoMU::CheckChar() // OK
{
	for(int n=0;n < MAX_REI_CHAR;n++)
	{
		if(OBJECT_RANGE(this->Char[n].Index) == 0)
		{
			continue;
		}
		if(gObjIsConnected(this->Char[n].Index) == 0)
		{
			this->Char[n].Reset();
			continue;
		}
		//if (gObj[this->Char[n].Index].Map != gServerInfo.m_ReiDoMUMap)
		//{
		//	this->Char[n].Reset();
		//	continue;
		//}
		if(strcmp(this->Char[n].Name, gObj[this->Char[n].Index].Name) != 0 )
		{
			this->Char[n].Reset();
			continue;
		}
	}
	return;
}

void CReiDoMU::CalcUserRank() // OK
{
	for(int n=0;n < MAX_REI_CHAR;n++)
	{
		if(this->Char[n].Index < 0)
		{
			continue;
		}

		int rank = MAX_REI_CHAR;

		for(int i=0;i < MAX_REI_CHAR;i++)
		{
			if(this->Char[i].Index < 0)
			{
				rank--;
				continue;
			}

			if(this->Char[n].Index == this->Char[i].Index)
			{
				rank--;
				continue;
			}

			if(this->Char[n].Times > this->Char[i].Times)
			{
				rank--;
				continue;
			}

			if(this->Char[n].Times == this->Char[i].Times && n < i)
			{
				rank--;
				continue;
			}
		}

		this->Char[n].Rank = rank;
	}

	for(int n=0;n < MAX_REI_CHAR;n++)
	{
		if (this->Char[n].Index >= 0 )
		{
			this->Char[n].Rank++;

			if (this->Char[n].Rank == 1) 
			{
				this->rank1 = this->Char[n].Index;
				GDRankingKingPlayerSaveSend(this->Char[n].Index,this->Char[n].Times);
			}
			if (this->Char[n].Rank == 2)
			{
				this->rank2 = this->Char[n].Index;
				GDRankingKingPlayerSaveSend(this->Char[n].Index,this->Char[n].Times);
			}
			if (this->Char[n].Rank == 3)
			{
				this->rank3 = this->Char[n].Index;
				GDRankingKingPlayerSaveSend(this->Char[n].Index,this->Char[n].Times);
			}
			if (this->Char[n].Rank == 4)
			{
				this->rank4 = this->Char[n].Index;
				GDRankingKingPlayerSaveSend(this->Char[n].Index,this->Char[n].Times);
			}
			if (this->Char[n].Rank == 5)
			{
				this->rank5 = this->Char[n].Index;
				GDRankingKingPlayerSaveSend(this->Char[n].Index,this->Char[n].Times);
			}
		}
	}
}

void CReiDoMU::GDRankingKingGuildSaveSend(char* name,DWORD Score) // OK
{
	SDHP_RANKING_KING_GUILD_SAVE_SEND pMsg;

	pMsg.header.set(0x74,sizeof(pMsg));

	memcpy(pMsg.name,name,sizeof(pMsg.name));

	pMsg.score = Score;

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);
}

void CReiDoMU::GDRankingKingPlayerSaveSend(int aIndex,DWORD Score) // OK
{

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	LPOBJ lpObj = &gObj[aIndex];

	SDHP_RANKING_KING_PLAYER_SAVE_SEND pMsg;

	pMsg.header.set(0x75,sizeof(pMsg));

	pMsg.index = aIndex;

	memcpy(pMsg.account,lpObj->Account,sizeof(pMsg.account));

	memcpy(pMsg.name,lpObj->Name,sizeof(pMsg.name));

	pMsg.score = Score;

	gDataServerConnection.DataSend((BYTE*)&pMsg,pMsg.header.size);
}

void CReiDoMU::StartKing()
{
	CTime CurrentTime = CTime::GetTickCount();

	int hour	= (int)CurrentTime.GetHour();
	int minute	= (int)CurrentTime.GetMinute()+2;

	if (minute >= 60)
	{
		hour++;
		minute = minute-60; 
	}

	REI_START_TIME info;

	info.Year = (int)CurrentTime.GetYear();

	info.Month = (int)CurrentTime.GetMonth();

	info.Day = (int)CurrentTime.GetDay();

	info.DayOfWeek = -1;

	info.Hour = hour;

	info.Minute = minute;

	info.Second = 0;

	this->m_ReiDoMUStartTime.push_back(info);

	LogAdd(LOG_EVENT,"[Set King of Mu Start] At %02d:%02d:00",hour,minute);

	this->Init();
}


