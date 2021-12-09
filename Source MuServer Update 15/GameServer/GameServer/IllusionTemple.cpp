// IllusionTemple.cpp: implementation of the CIllusionTemple class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IllusionTemple.h"
#include "DSProtocol.h"
#include "EffectManager.h"
#include "Gate.h"
#include "Guild.h"
#include "ItemBagManager.h"
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
#include "ScheduleManager.h"
#include "ServerInfo.h"
#include "SkillManager.h"
#include "Util.h"
#include "Viewport.h"

CIllusionTemple gIllusionTemple;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CIllusionTemple::CIllusionTemple() // OK
{
	for(int n=0;n < MAX_IT_LEVEL;n++)
	{
		ILLUSION_TEMPLE_LEVEL* lpLevel = &this->m_IllusionTempleLevel[n];

		lpLevel->Level = n;
		lpLevel->State = IT_STATE_BLANK;
		lpLevel->Map = MAP_ILLUSION_TEMPLE1+n;
		lpLevel->RemainTime = 0;
		lpLevel->TargetTime = 0;
		lpLevel->TickCount = GetTickCount();
		lpLevel->EnterEnabled = 0;
		lpLevel->MinutesLeft = -1;
		lpLevel->TimeCount = 0;
		lpLevel->Score[IT_TEAM_Y] = 0;
		lpLevel->Score[IT_TEAM_B] = 0;
		lpLevel->EventItemSerial = -1;
		lpLevel->EventItemNumber = -1;
		lpLevel->EventItemOwner = -1;
		lpLevel->EventItemLevel = -1;
		lpLevel->PartyNumber[IT_TEAM_Y] = -1;
		lpLevel->PartyNumber[IT_TEAM_B] = -1;
		lpLevel->WinnerTeam = IT_TEAM_NONE;
		lpLevel->TeamRewardExperience[IT_TEAM_Y] = 0;
		lpLevel->TeamRewardExperience[IT_TEAM_B] = 0;

		this->CleanUser(lpLevel);

		lpLevel->StoneStatueIndex = -1;

		this->CleanMonster(lpLevel);
	}

	memset(this->m_IllusionTempleExperienceTable,0,sizeof(this->m_IllusionTempleExperienceTable));
}

CIllusionTemple::~CIllusionTemple() // OK
{

}

void CIllusionTemple::Init() // OK
{
	for(int n=0;n < MAX_IT_LEVEL;n++)
	{
		if(gServerInfo.m_IllusionTempleEvent == 0)
		{
			this->SetState(&this->m_IllusionTempleLevel[n],IT_STATE_BLANK);
		}
		else
		{
			this->SetState(&this->m_IllusionTempleLevel[n],IT_STATE_EMPTY);
		}
	}
}

void CIllusionTemple::Load(char* path) // OK
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

	this->m_IllusionTempleStartTime.clear();

	memset(this->m_IllusionTempleExperienceTable,0,sizeof(this->m_IllusionTempleExperienceTable));

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

					this->m_NotifyTime = lpMemScript->GetAsNumber();

					this->m_EventTime = lpMemScript->GetAsNumber();

					this->m_CloseTime = lpMemScript->GetAsNumber();
				}
				else if(section == 1)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					ILLUSION_TEMPLE_START_TIME info;

					info.Year = lpMemScript->GetNumber();

					info.Month = lpMemScript->GetAsNumber();

					info.Day = lpMemScript->GetAsNumber();

					info.DayOfWeek = lpMemScript->GetAsNumber();

					info.Hour = lpMemScript->GetAsNumber();

					info.Minute = lpMemScript->GetAsNumber();

					info.Second = lpMemScript->GetAsNumber();

					this->m_IllusionTempleStartTime.push_back(info);
				}
				else if(section == 2)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					int level = lpMemScript->GetNumber();

					this->m_IllusionTempleExperienceTable[level][0] = lpMemScript->GetAsNumber();

					this->m_IllusionTempleExperienceTable[level][1] = lpMemScript->GetAsNumber();
				}
				else if(section == 3)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					int level = lpMemScript->GetNumber();

					this->m_IllusionTempleReward[level][0] = lpMemScript->GetAsNumber();

					this->m_IllusionTempleReward[level][1] = lpMemScript->GetAsNumber();

					this->m_IllusionTempleReward[level][2] = lpMemScript->GetAsNumber();
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

void CIllusionTemple::MainProc() // OK
{
	for(int n=0;n < MAX_IT_LEVEL;n++)
	{
		ILLUSION_TEMPLE_LEVEL* lpLevel = &this->m_IllusionTempleLevel[n];

		DWORD elapsed = GetTickCount()-lpLevel->TickCount;

		if(elapsed < 1000)
		{
			continue;
		}

		lpLevel->TickCount = GetTickCount();

		lpLevel->RemainTime = (int)difftime(lpLevel->TargetTime,time(0));

		if (n==0)
		{
			if(gServerInfo.m_IllusionTempleEvent == 0)
			{
				if (gServerDisplayer.EventIt != -1)
				{
					gServerDisplayer.EventIt = -1;
				}
			}
			else 
			{
				if (lpLevel->State == IT_STATE_EMPTY)
				{
					gServerDisplayer.EventIt = lpLevel->RemainTime;
				}
				else 
				{
					if (gServerDisplayer.EventIt != 0)
					{
						gServerDisplayer.EventIt = 0;
					}
				}
			}
		}

		if(gServerDisplayer.EventIt > 0)
		{
			if (lpLevel->State != IT_STATE_EMPTY && lpLevel->State != IT_STATE_BLANK)
			{
				gServerDisplayer.EventIt = 0;
			}
		}

		switch(lpLevel->State)
		{
			case IT_STATE_BLANK:
				this->ProcState_BLANK(lpLevel);
				break;
			case IT_STATE_EMPTY:
				this->ProcState_EMPTY(lpLevel);
				break;
			case IT_STATE_STAND:
				this->ProcState_STAND(lpLevel);
				break;
			case IT_STATE_START:
				this->ProcState_START(lpLevel);
				break;
			case IT_STATE_CLEAN:
				this->ProcState_CLEAN(lpLevel);
				break;
		}
	}
}

void CIllusionTemple::ProcState_BLANK(ILLUSION_TEMPLE_LEVEL* lpLevel) // OK
{

}

void CIllusionTemple::ProcState_EMPTY(ILLUSION_TEMPLE_LEVEL* lpLevel) // OK
{
	if(lpLevel->RemainTime > 0 && lpLevel->RemainTime <= (this->m_WarningTime*60))
	{
		this->CheckUser(lpLevel);

		int minutes = lpLevel->RemainTime/60;

		if((lpLevel->RemainTime%60) == 0)
		{
			minutes--;
		}

		if(lpLevel->MinutesLeft != minutes)
		{
			lpLevel->MinutesLeft = minutes;

			if(lpLevel->Level == 0)
			{
				gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(176),(lpLevel->MinutesLeft+1));
			}
		}

		lpLevel->EnterEnabled = 1;
	}

	if(lpLevel->RemainTime > 0 && lpLevel->RemainTime <= 30 && lpLevel->TimeCount == 0)
	{
		PMSG_TIME_COUNT_SEND pMsg;

		pMsg.header.set(0x92,sizeof(pMsg));

		pMsg.type = 14;

		DataSendAll((BYTE*)&pMsg,pMsg.header.size);

		lpLevel->TimeCount = 1;
	}

	if(lpLevel->RemainTime <= 0)
	{
		if(lpLevel->Level == 0){gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(177));}
		this->NoticeSendToAll(lpLevel,1,gMessage.GetMessage(178),(lpLevel->Level+1),this->m_NotifyTime);
		this->SetState(lpLevel,IT_STATE_STAND);
	}
}

void CIllusionTemple::ProcState_STAND(ILLUSION_TEMPLE_LEVEL* lpLevel) // OK
{
	this->CheckUser(lpLevel);

	if(this->GetUserCount(lpLevel,IT_TEAM_Y) < (gServerInfo.m_IllusionTempleMinUser/2) || this->GetUserCount(lpLevel,IT_TEAM_B) < (gServerInfo.m_IllusionTempleMinUser/2))
	{
		//LogAdd(LOG_BLACK,"[Illusion Temple] (%d) Not enough users",(lpLevel->Level+1));
		this->SetState(lpLevel,IT_STATE_EMPTY);
		return;
	}

	if(lpLevel->RemainTime > 0 && lpLevel->RemainTime <= 30 && lpLevel->TimeCount == 0)
	{
		PMSG_TIME_COUNT_SEND pMsg;

		pMsg.header.set(0x92,sizeof(pMsg));

		pMsg.type = 15;

		this->DataSendToAll(lpLevel,(BYTE*)&pMsg,pMsg.header.size);

		lpLevel->TimeCount = 1;
	}

	if(lpLevel->RemainTime <= 0)
	{
		this->NoticeSendToAll(lpLevel,0,gMessage.GetMessage(179));

		this->SetState(lpLevel,IT_STATE_START);
	}

	this->GCIllusionTempleBattleInfoSend(lpLevel);
}

void CIllusionTemple::ProcState_START(ILLUSION_TEMPLE_LEVEL* lpLevel) // OK
{
	this->CheckUser(lpLevel);

	if(this->GetUserCount(lpLevel,IT_TEAM_Y) == 0 || this->GetUserCount(lpLevel,IT_TEAM_B) == 0)
	{
		//LogAdd(LOG_BLACK,"[Illusion Temple] (%d) Not enough users",(lpLevel->Level+1));
		this->SetState(lpLevel,IT_STATE_CLEAN);
		return;
	}

#if (GAMESERVER_UPDATE >= 803)
	if (lpLevel->EventItemOwner != -1)
	{
		ILLUSION_TEMPLE_USER* lpUser = this->GetUser(lpLevel,lpLevel->EventItemOwner);

		if(lpUser == 0)
		{
			return;
		}
		lpLevel->Score[lpUser->Team]++;
	}

#endif

	this->GCIllusionTempleBattleInfoSend(lpLevel);

#if (GAMESERVER_UPDATE <= 603)
	if(lpLevel->Score[IT_TEAM_Y] >= 7 || lpLevel->Score[IT_TEAM_B] >= 7)
	{
		this->SetState(lpLevel,IT_STATE_CLEAN);
		return;
	}
#else
	if(lpLevel->Score[IT_TEAM_Y] >= 255 || lpLevel->Score[IT_TEAM_B] >= 255)
	{
		this->SetState(lpLevel,IT_STATE_CLEAN);
		return;
	}
#endif

	if(lpLevel->RemainTime <= 0)
	{
		this->NoticeSendToAll(lpLevel,0,gMessage.GetMessage(180));

		this->SetState(lpLevel,IT_STATE_CLEAN);
	}
}

void CIllusionTemple::ProcState_CLEAN(ILLUSION_TEMPLE_LEVEL* lpLevel) // OK
{
	this->CheckUser(lpLevel);

	if(lpLevel->RemainTime <= 0)
	{
		this->SetState(lpLevel,IT_STATE_EMPTY);
	}
}

void CIllusionTemple::SetState(ILLUSION_TEMPLE_LEVEL* lpLevel,int state) // OK
{
	lpLevel->State = state;

	switch(lpLevel->State)
	{
		case IT_STATE_BLANK:
			this->SetState_BLANK(lpLevel);
			break;
		case IT_STATE_EMPTY:
			this->SetState_EMPTY(lpLevel);
			break;
		case IT_STATE_STAND:
			this->SetState_STAND(lpLevel);
			break;
		case IT_STATE_START:
			this->SetState_START(lpLevel);
			break;
		case IT_STATE_CLEAN:
			this->SetState_CLEAN(lpLevel);
			break;
	}
}

void CIllusionTemple::SetState_BLANK(ILLUSION_TEMPLE_LEVEL* lpLevel) // OK
{
	//LogAdd(LOG_BLACK,"[Illusion Temple] (%d) SetState BLANK",(lpLevel->Level+1));
}

void CIllusionTemple::SetState_EMPTY(ILLUSION_TEMPLE_LEVEL* lpLevel) // OK
{
	lpLevel->EnterEnabled = 0;
	lpLevel->MinutesLeft = -1;
	lpLevel->TimeCount = 0;
	lpLevel->Score[IT_TEAM_Y] = 0;
	lpLevel->Score[IT_TEAM_B] = 0;
	lpLevel->EventItemSerial = -1;
	lpLevel->EventItemNumber = -1;
	lpLevel->EventItemOwner = -1;
	lpLevel->EventItemLevel = -1;
	lpLevel->WinnerTeam = IT_TEAM_NONE;
	lpLevel->TeamRewardExperience[IT_TEAM_Y] = 0;
	lpLevel->TeamRewardExperience[IT_TEAM_B] = 0;

	if(OBJECT_RANGE(lpLevel->PartyNumber[IT_TEAM_Y]) != 0)
	{
		gParty.Destroy(lpLevel->PartyNumber[IT_TEAM_Y]);
		lpLevel->PartyNumber[IT_TEAM_Y] = -1;
	}

	if(OBJECT_RANGE(lpLevel->PartyNumber[IT_TEAM_B]) != 0)
	{
		gParty.Destroy(lpLevel->PartyNumber[IT_TEAM_B]);
		lpLevel->PartyNumber[IT_TEAM_B] = -1;
	}

	this->GCIllusionTempleChangeScenarioSend(lpLevel,0,-1);

	this->ClearUser(lpLevel);

	if(OBJECT_RANGE(lpLevel->StoneStatueIndex) != 0)
	{
		gObjDel(lpLevel->StoneStatueIndex);
		lpLevel->StoneStatueIndex = -1;
	}

	this->ClearMonster(lpLevel);

	this->CheckSync(lpLevel);

	//LogAdd(LOG_BLACK,"[Illusion Temple] (%d) SetState EMPTY",(lpLevel->Level+1));
}

void CIllusionTemple::SetState_STAND(ILLUSION_TEMPLE_LEVEL* lpLevel) // OK
{
	lpLevel->EnterEnabled = 0;
	lpLevel->MinutesLeft = -1;
	lpLevel->TimeCount = 0;

	int team = GetLargeRand()%MAX_IT_TEAM;

	for(int n=0;n < MAX_IT_USER;n++)
	{
		if(OBJECT_RANGE(lpLevel->User[n].Index) != 0)
		{
			lpLevel->User[n].Team = team;
			gObjMoveGate(lpLevel->User[n].Index,148+(team*6)+lpLevel->Level);
			gObj[lpLevel->User[n].Index].Change = 404+team;
			gObjViewportListProtocolCreate(&gObj[lpLevel->User[n].Index]);

			if(OBJECT_RANGE(lpLevel->PartyNumber[team]) == 0)
			{
				if(gParty.Create(lpLevel->User[n].Index) != 0)
				{
					lpLevel->PartyNumber[team] = gObj[lpLevel->User[n].Index].PartyNumber;
				}
			}
			else
			{
				if(gParty.AddMember(lpLevel->PartyNumber[team],lpLevel->User[n].Index) != 0)
				{
					lpLevel->PartyNumber[team] = gObj[lpLevel->User[n].Index].PartyNumber;
				}
			}

			team ^= 1;
		}
	}

	this->GCIllusionTempleChangeScenarioSend(lpLevel,1,-1);

	lpLevel->RemainTime = this->m_NotifyTime*60;

	lpLevel->TargetTime = (int)(time(0)+lpLevel->RemainTime);

	//LogAdd(LOG_BLACK,"[Illusion Temple] (%d) SetState STAND",(lpLevel->Level+1));
}

void CIllusionTemple::SetState_START(ILLUSION_TEMPLE_LEVEL* lpLevel) // OK
{
	lpLevel->EnterEnabled = 0;
	lpLevel->MinutesLeft = -1;
	lpLevel->TimeCount = 0;

	this->SetStoneStatue(lpLevel);


	switch(lpLevel->Level)
	{
		case 0:
			this->SetMonster(lpLevel,386);
			this->SetMonster(lpLevel,387);
			this->SetMonster(lpLevel,388);
			break;
		case 1:
			this->SetMonster(lpLevel,389);
			this->SetMonster(lpLevel,390);
			this->SetMonster(lpLevel,391);
			break;
		case 2:
			this->SetMonster(lpLevel,392);
			this->SetMonster(lpLevel,393);
			this->SetMonster(lpLevel,394);
			break;
		case 3:
			this->SetMonster(lpLevel,395);
			this->SetMonster(lpLevel,396);
			this->SetMonster(lpLevel,397);
			break;
		case 4:
			this->SetMonster(lpLevel,398);
			this->SetMonster(lpLevel,399);
			this->SetMonster(lpLevel,400);
			break;
		case 5:
			this->SetMonster(lpLevel,401);
			this->SetMonster(lpLevel,402);
			this->SetMonster(lpLevel,403);
			break;
	}

	this->GCIllusionTempleChangeScenarioSend(lpLevel,2,-1);

	lpLevel->RemainTime = this->m_EventTime*60;

	lpLevel->TargetTime = (int)(time(0)+lpLevel->RemainTime);

	//LogAdd(LOG_BLACK,"[Illusion Temple] (%d) SetState START",(lpLevel->Level+1));
}

void CIllusionTemple::SetState_CLEAN(ILLUSION_TEMPLE_LEVEL* lpLevel) // OK
{
	lpLevel->EnterEnabled = 0;
	lpLevel->MinutesLeft = -1;
	lpLevel->TimeCount = 0;

	if(OBJECT_RANGE(lpLevel->EventItemOwner) != 0)
	{
		this->SearchUserDeleteEventItem(lpLevel->EventItemOwner);
	}

	for(int n=0;n < MAX_IT_USER;n++)
	{
		if(OBJECT_RANGE(lpLevel->User[n].Index) != 0)
		{
			gObjMoveGate(lpLevel->User[n].Index,148+(lpLevel->User[n].Team*6)+lpLevel->Level);
		}
	}

#if (GAMESERVER_UPDATE <=603)
	if(lpLevel->Score[IT_TEAM_Y] >= 2 && lpLevel->Score[IT_TEAM_Y] > lpLevel->Score[IT_TEAM_B])
	{
		lpLevel->WinnerTeam = IT_TEAM_Y;

		lpLevel->TeamRewardExperience[IT_TEAM_Y] = (this->m_IllusionTempleExperienceTable[lpLevel->Level][0]+((lpLevel->Score[IT_TEAM_Y]-lpLevel->Score[IT_TEAM_B])*this->m_IllusionTempleExperienceTable[lpLevel->Level][1]))*2;

		if(lpLevel->Score[IT_TEAM_B] == 0)
		{
			lpLevel->TeamRewardExperience[IT_TEAM_B] = this->m_IllusionTempleExperienceTable[lpLevel->Level][0];
		}
		else
		{
			lpLevel->TeamRewardExperience[IT_TEAM_B] = this->m_IllusionTempleExperienceTable[lpLevel->Level][0]+(lpLevel->Score[IT_TEAM_B]*this->m_IllusionTempleExperienceTable[lpLevel->Level][1]);
		}
	}
	else if(lpLevel->Score[IT_TEAM_B] >= 2 && lpLevel->Score[IT_TEAM_B] > lpLevel->Score[IT_TEAM_Y])
	{
		lpLevel->WinnerTeam = IT_TEAM_B;

		lpLevel->TeamRewardExperience[IT_TEAM_B] = (this->m_IllusionTempleExperienceTable[lpLevel->Level][0]+((lpLevel->Score[IT_TEAM_B]-lpLevel->Score[IT_TEAM_Y])*this->m_IllusionTempleExperienceTable[lpLevel->Level][1]))*2;

		if(lpLevel->Score[IT_TEAM_Y] == 0)
		{
			lpLevel->TeamRewardExperience[IT_TEAM_Y] = this->m_IllusionTempleExperienceTable[lpLevel->Level][0];
		}
		else
		{
			lpLevel->TeamRewardExperience[IT_TEAM_Y] = this->m_IllusionTempleExperienceTable[lpLevel->Level][0]+(lpLevel->Score[IT_TEAM_Y]*this->m_IllusionTempleExperienceTable[lpLevel->Level][1]);
		}
	}
	else
	{
		if(lpLevel->Score[IT_TEAM_Y] == 0)
		{
			lpLevel->TeamRewardExperience[IT_TEAM_Y] = this->m_IllusionTempleExperienceTable[lpLevel->Level][0];
		}
		else
		{
			lpLevel->TeamRewardExperience[IT_TEAM_Y] = this->m_IllusionTempleExperienceTable[lpLevel->Level][0]+(lpLevel->Score[IT_TEAM_Y]*this->m_IllusionTempleExperienceTable[lpLevel->Level][1]);
		}

		if(lpLevel->Score[IT_TEAM_B] == 0)
		{
			lpLevel->TeamRewardExperience[IT_TEAM_B] = this->m_IllusionTempleExperienceTable[lpLevel->Level][0];
		}
		else
		{
			lpLevel->TeamRewardExperience[IT_TEAM_B] = this->m_IllusionTempleExperienceTable[lpLevel->Level][0]+(lpLevel->Score[IT_TEAM_B]*this->m_IllusionTempleExperienceTable[lpLevel->Level][1]);
		}
	}

#else
	if (lpLevel->Score[IT_TEAM_Y] <= lpLevel->Score[IT_TEAM_B])
	{
		if (lpLevel->Score[IT_TEAM_Y] >= lpLevel->Score[IT_TEAM_B])
		{
			if (lpLevel->Score[IT_TEAM_Y] == lpLevel->Score[IT_TEAM_B])
			{
				BYTE byAlliedTeamKillSum = 0;
				BYTE byIllusionTeamKillSum = 0;

				for(int n=0;n < MAX_IT_USER;n++)
				{
					LPOBJ lpObj = &gObj[lpLevel->User[n].Index];

					if(OBJECT_RANGE(lpObj->Index) == 0)
					{
						continue;
					}
					if(lpLevel->User[n].Team == 0)
					{
						byAlliedTeamKillSum += lpLevel->User[n].Point;
					}
					if(lpLevel->User[n].Team == 1)
					{
						byIllusionTeamKillSum += lpLevel->User[n].Point;
					}
				}

				if (byAlliedTeamKillSum <= byIllusionTeamKillSum)
				{
					if (byAlliedTeamKillSum >= byIllusionTeamKillSum)
					{
						int nAlliedLevelSum = 0;
						int nIllusionLevelSum = 0;

						for(int n=0;n < MAX_IT_USER;n++)
						{
							LPOBJ lpObj = &gObj[lpLevel->User[n].Index];

							if(OBJECT_RANGE(lpObj->Index) == 0)
							{
								continue;
							}

							if(lpLevel->User[n].Team == 0)
							{
								nAlliedLevelSum += lpObj->Level+lpObj->MasterLevel;
							}
							if(lpLevel->User[n].Team == 1)
							{
								nIllusionLevelSum += lpObj->Level+lpObj->MasterLevel;
							}
						}

						if (nAlliedLevelSum <= nIllusionLevelSum)
						{
							if (nAlliedLevelSum < nIllusionLevelSum)
							{
								lpLevel->WinnerTeam = IT_TEAM_Y;
							}
						}

						else
						{
							lpLevel->WinnerTeam = IT_TEAM_B;
						}
					}

					else
					{
						lpLevel->WinnerTeam = IT_TEAM_B;
					}
				}

				else
				{
					lpLevel->WinnerTeam = IT_TEAM_Y;
				}
			}
		}

		else
		{
			lpLevel->WinnerTeam = IT_TEAM_B;
		}
	}

	else
	{
		lpLevel->WinnerTeam = IT_TEAM_Y;
	}

	if (lpLevel->WinnerTeam == 0)
	{
		this->NoticeSendToAll(lpLevel,0,gMessage.GetMessage(769));
	}
	else
	{
		this->NoticeSendToAll(lpLevel,0,gMessage.GetMessage(770));
	}

	for(int n=0;n < MAX_IT_USER;n++)
	{
		if(OBJECT_RANGE(lpLevel->User[n].Index) == 0)
		{
			continue;
		}
		if(OBJECT_RANGE(lpLevel->WinnerTeam) == 0)
		{
			continue;
		}
		if(lpLevel->User[n].Team == lpLevel->WinnerTeam)
		{

			LPOBJ lpObj = &gObj[lpLevel->User[n].Index];

			switch(lpLevel->Level)
			{
				case 0:
					gItemBagManager.DropItemBySpecialValue(ITEM_BAG_ILLUSION_TEMPLE1,lpObj,lpObj->Map,lpObj->X,lpObj->Y);
					break;
				case 1:
					gItemBagManager.DropItemBySpecialValue(ITEM_BAG_ILLUSION_TEMPLE2,lpObj,lpObj->Map,lpObj->X,lpObj->Y);
					break;
				case 2:
					gItemBagManager.DropItemBySpecialValue(ITEM_BAG_ILLUSION_TEMPLE3,lpObj,lpObj->Map,lpObj->X,lpObj->Y);
					break;
				case 3:
					gItemBagManager.DropItemBySpecialValue(ITEM_BAG_ILLUSION_TEMPLE4,lpObj,lpObj->Map,lpObj->X,lpObj->Y);
					break;
				case 4:
					gItemBagManager.DropItemBySpecialValue(ITEM_BAG_ILLUSION_TEMPLE5,lpObj,lpObj->Map,lpObj->X,lpObj->Y);
					break;
				case 5:
					gItemBagManager.DropItemBySpecialValue(ITEM_BAG_ILLUSION_TEMPLE6,lpObj,lpObj->Map,lpObj->X,lpObj->Y);
					break;
			}

			if (this->m_IllusionTempleReward[lpLevel->Level][0] > 0 || this->m_IllusionTempleReward[lpLevel->Level][1] > 0 || this->m_IllusionTempleReward[lpLevel->Level][2] > 0)
			{
				//Reward
				GDSetCoinSend(lpObj->Index, this->m_IllusionTempleReward[lpLevel->Level][0], this->m_IllusionTempleReward[lpLevel->Level][1], this->m_IllusionTempleReward[lpLevel->Level][2],"IT");
		
				GCFireworksSend(lpObj,lpObj->X,lpObj->Y);
			}
		}


		

	}
#endif



	if(OBJECT_RANGE(lpLevel->StoneStatueIndex) != 0)
	{
		gObjDel(lpLevel->StoneStatueIndex);
		lpLevel->StoneStatueIndex = -1;
	}
	

	this->GCIllusionTempleChangeScenarioSend(lpLevel,3,-1);

	this->ClearMonster(lpLevel);

#if (GAMESERVER_UPDATE <= 603)
	this->GCIllusionTempleRewardScoreSend(lpLevel);
#endif

	this->GiveUserRewardExperience(lpLevel);

	lpLevel->RemainTime = this->m_CloseTime*60;

	lpLevel->TargetTime = (int)(time(0)+lpLevel->RemainTime);

	//LogAdd(LOG_BLACK,"[Illusion Temple] (%d) SetState CLEAN",(lpLevel->Level+1));
}

void CIllusionTemple::CheckSync(ILLUSION_TEMPLE_LEVEL* lpLevel) // OK
{
	if(this->m_IllusionTempleStartTime.empty() != 0)
	{
		this->SetState(lpLevel,IT_STATE_BLANK);
		return;
	}

	CTime ScheduleTime;

	CScheduleManager ScheduleManager;

	for(std::vector<ILLUSION_TEMPLE_START_TIME>::iterator it=this->m_IllusionTempleStartTime.begin();it != this->m_IllusionTempleStartTime.end();it++)
	{
		ScheduleManager.AddSchedule(it->Year,it->Month,it->Day,it->Hour,it->Minute,it->Second,it->DayOfWeek);
	}

	if(ScheduleManager.GetSchedule(&ScheduleTime) == 0)
	{
		this->SetState(lpLevel,IT_STATE_BLANK);
		return;
	}

	lpLevel->RemainTime = (int)difftime(ScheduleTime.GetTime(),time(0));

	lpLevel->TargetTime = (int)ScheduleTime.GetTime();

	//LogAdd(LOG_BLACK,"[Illusion Temple] (%d) Sync Start Time. [%d] min remain",(lpLevel->Level+1),(lpLevel->RemainTime/60));
}

int CIllusionTemple::GetState(int level) // OK
{
	if(IT_LEVEL_RANGE(level) == 0)
	{
		return IT_STATE_BLANK;
	}

	return this->m_IllusionTempleLevel[level].State;
}

int CIllusionTemple::GetRemainTime(int level) // OK
{
	if(IT_LEVEL_RANGE(level) == 0)
	{
		return 0;
	}

	if(this->m_IllusionTempleStartTime.empty() != 0)
	{
		return 0;
	}

	CTime ScheduleTime;

	CScheduleManager ScheduleManager;

	for(std::vector<ILLUSION_TEMPLE_START_TIME>::iterator it=this->m_IllusionTempleStartTime.begin();it != this->m_IllusionTempleStartTime.end();it++)
	{
		ScheduleManager.AddSchedule(it->Year,it->Month,it->Day,it->Hour,it->Minute,it->Second,it->DayOfWeek);
	}

	if(ScheduleManager.GetSchedule(&ScheduleTime) == 0)
	{
		return 0;
	}

	int RemainTime = (int)difftime(ScheduleTime.GetTime(),time(0));

	return (((RemainTime%60)==0)?(RemainTime/60):((RemainTime/60)+1));
}

int CIllusionTemple::GetEnterEnabled(int level) // OK
{
	if(IT_LEVEL_RANGE(level) == 0)
	{
		return 0;
	}
	
	return this->m_IllusionTempleLevel[level].EnterEnabled;
}

int CIllusionTemple::GetEnteredUserCount(int level) // OK
{
	if(IT_LEVEL_RANGE(level) == 0)
	{
		return 0;
	}

	return this->GetUserCount(&this->m_IllusionTempleLevel[level],IT_TEAM_NONE);
}

bool CIllusionTemple::AddUser(ILLUSION_TEMPLE_LEVEL* lpLevel,int aIndex) // OK
{
	if(OBJECT_RANGE(aIndex) == 0)
	{
		return 0;
	}

	if(this->GetUser(lpLevel,aIndex) != 0)
	{
		return 0;
	}

	for(int n=0;n < MAX_IT_USER;n++)
	{
		if(OBJECT_RANGE(lpLevel->User[n].Index) != 0)
		{
			continue;
		}

		lpLevel->User[n].Index = aIndex;
		lpLevel->User[n].Team = IT_TEAM_NONE;
		lpLevel->User[n].Point = 0;
		lpLevel->User[n].Score = 0;
		lpLevel->User[n].RewardExperience = 0;
		return 1;
	}

	return 0;
}

bool CIllusionTemple::DelUser(ILLUSION_TEMPLE_LEVEL* lpLevel,int aIndex) // OK
{
	if(OBJECT_RANGE(aIndex) == 0)
	{
		return 0;
	}

	ILLUSION_TEMPLE_USER* lpUser = this->GetUser(lpLevel,aIndex);

	if(lpUser == 0)
	{
		return 0;
	}

	if(lpLevel->State == IT_STATE_START)
	{
		this->SearchUserDropEventItem(aIndex);
	}
	else
	{
		this->SearchUserDeleteEventItem(aIndex);
	}

	if(OBJECT_RANGE(lpLevel->PartyNumber[lpUser->Team]) != 0)
	{
		gParty.DelMember(lpLevel->PartyNumber[lpUser->Team],aIndex);
		lpLevel->PartyNumber[lpUser->Team] = ((gParty.IsParty(lpLevel->PartyNumber[lpUser->Team])==0)?-1:lpLevel->PartyNumber[lpUser->Team]);
	}

	if(gObj[aIndex].Change >= 0)
	{
		gObj[aIndex].Change = -1;
		gObjViewportListProtocolCreate(&gObj[aIndex]);
	}

	lpUser->Index = -1;
	lpUser->Team = IT_TEAM_NONE;
	lpUser->Point = 0;
	lpUser->Score = 0;
	lpUser->RewardExperience = 0;
	return 1;
}

ILLUSION_TEMPLE_USER* CIllusionTemple::GetUser(ILLUSION_TEMPLE_LEVEL* lpLevel,int aIndex) // OK
{
	if(OBJECT_RANGE(aIndex) == 0)
	{
		return 0;
	}

	for(int n=0;n < MAX_IT_USER;n++)
	{
		if(lpLevel->User[n].Index == aIndex)
		{
			return &lpLevel->User[n];
		}
	}

	return 0;
}

void CIllusionTemple::CleanUser(ILLUSION_TEMPLE_LEVEL* lpLevel) // OK
{
	for(int n=0;n < MAX_IT_USER;n++)
	{
		lpLevel->User[n].Index = -1;
		lpLevel->User[n].Team = IT_TEAM_NONE;
		lpLevel->User[n].Point = 0;
		lpLevel->User[n].Score = 0;
		lpLevel->User[n].RewardExperience = 0;
	}
}

void CIllusionTemple::ClearUser(ILLUSION_TEMPLE_LEVEL* lpLevel) // OK
{
	for(int n=0;n < MAX_IT_USER;n++)
	{
		if(OBJECT_RANGE(lpLevel->User[n].Index) == 0)
		{
			continue;
		}

		if(gObj[lpLevel->User[n].Index].Change >= 0)
		{
			gObj[lpLevel->User[n].Index].Change = -1;
			gObjViewportListProtocolCreate(&gObj[lpLevel->User[n].Index]);
		}

		gObjMoveGate(lpLevel->User[n].Index,267);

		lpLevel->User[n].Index = -1;
		lpLevel->User[n].Team = IT_TEAM_NONE;
		lpLevel->User[n].Point = 0;
		lpLevel->User[n].Score = 0;
		lpLevel->User[n].RewardExperience = 0;
	}
}

void CIllusionTemple::CheckUser(ILLUSION_TEMPLE_LEVEL* lpLevel) // OK
{
	for(int n=0;n < MAX_IT_USER;n++)
	{
		if(OBJECT_RANGE(lpLevel->User[n].Index) == 0)
		{
			continue;
		}

		if(gObjIsConnected(lpLevel->User[n].Index) == 0)
		{
			this->DelUser(lpLevel,lpLevel->User[n].Index);
			continue;
		}

		if(gObj[lpLevel->User[n].Index].Map != lpLevel->Map)
		{
			this->DelUser(lpLevel,lpLevel->User[n].Index);
			continue;
		}
	}
}

int CIllusionTemple::GetUserCount(ILLUSION_TEMPLE_LEVEL* lpLevel,int team) // OK
{
	int count = 0;

	for(int n=0;n < MAX_IT_USER;n++)
	{
		if(OBJECT_RANGE(lpLevel->User[n].Index) == 0)
		{
			continue;
		}

		if(team == IT_TEAM_NONE || lpLevel->User[n].Team == team)
		{
			count++;
		}
	}

	return count;
}

int CIllusionTemple::GetUserJoinTeam(LPOBJ lpObj) // OK
{
	int level = GET_IT_LEVEL(lpObj->Map);

	if(IT_LEVEL_RANGE(level) == 0)
	{
		return -1;
	}

	ILLUSION_TEMPLE_USER* lpUser = this->GetUser(&this->m_IllusionTempleLevel[level],lpObj->Index);

	if(lpUser == 0)
	{
		return -1;
	}

	return lpUser->Team;
}

int CIllusionTemple::GetUserAbleLevel(LPOBJ lpObj) // OK
{
	int level = -1;

	if(lpObj->ChangeUp < 2)
	{
		if(lpObj->Level >= 220 && lpObj->Level <= 270)
		{
			level = 0;
		}
		else if(lpObj->Level >= 271 && lpObj->Level <= 320)
		{
			level = 1;
		}
		else if(lpObj->Level >= 321 && lpObj->Level <= 350)
		{
			level = 2;
		}
		else if(lpObj->Level >= 351 && lpObj->Level <= 380)
		{
			level = 3;
		}
		else if(lpObj->Level >= 381 && lpObj->Level <= MAX_CHARACTER_LEVEL)
		{
			level = 4;
		}
	}
	else
	{
		level = 5;
	}

	return level;
}

bool CIllusionTemple::GetUserRespawnLocation(LPOBJ lpObj,int* gate,int* map,int* x,int* y,int* dir,int* level) // OK
{
	int IllusionTempleLevel = GET_IT_LEVEL(lpObj->Map);

	if(IT_LEVEL_RANGE(IllusionTempleLevel) == 0)
	{
		return 0;
	}

	ILLUSION_TEMPLE_USER* lpUser = this->GetUser(&this->m_IllusionTempleLevel[IllusionTempleLevel],lpObj->Index);

	if(lpUser != 0 && lpUser->Team != IT_TEAM_NONE)
	{
		if(gGate.GetGate(((148+(lpUser->Team*6))+IllusionTempleLevel),gate,map,x,y,dir,level) == 0)
		{
			return 0;
		}
	}
	else
	{
		if(gGate.GetGate(267,gate,map,x,y,dir,level) == 0)
		{
			return 0;
		}
	}

	return 1;
}

void CIllusionTemple::SearchUserDropEventItem(int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	int level = GET_IT_LEVEL(lpObj->Map);

	if(IT_LEVEL_RANGE(level) == 0)
	{
		return;
	}

	ILLUSION_TEMPLE_LEVEL* lpLevel = &this->m_IllusionTempleLevel[level];

	if(this->GetUser(lpLevel,aIndex) == 0)
	{
		return;
	}

	if(lpLevel->EventItemOwner != aIndex)
	{
		return;
	}

	int slot = gItemManager.GetInventoryItemSlot(lpObj,GET_ITEM(14,64),lpLevel->EventItemLevel);

	if(slot == -1)
	{
		return;
	}

	if(lpObj->Inventory[slot].m_Serial != lpLevel->EventItemSerial)
	{
		return;
	}

	PMSG_ITEM_DROP_SEND pMsg;

	pMsg.header.set(0x23,sizeof(pMsg));

	pMsg.result = 0;

	pMsg.slot = slot;

	CItem* lpItem = &gObj[aIndex].Inventory[slot];

	if(gMap[lpObj->Map].ItemDrop(lpItem->m_Index,lpItem->m_Level,lpItem->m_Durability,lpObj->X,lpObj->Y,lpItem->m_Option1,lpItem->m_Option2,lpItem->m_Option3,lpItem->m_NewOption,lpItem->m_SetOption,lpItem->m_Serial,aIndex,lpItem->m_PetItemLevel,lpItem->m_PetItemExp,lpItem->m_JewelOfHarmonyOption,lpItem->m_ItemOptionEx,lpItem->m_SocketOption,lpItem->m_SocketOptionBonus,lpItem->m_PeriodicItemTime) != 0)
	{
		pMsg.result = 1;
		gItemManager.InventoryDelItem(aIndex,slot);
		gItemManager.UpdateInventoryViewport(aIndex,slot);
	}

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);

	this->GCIllusionTempleEventItemOwnerS8Send(lpLevel, aIndex, 1);
}

void CIllusionTemple::SearchUserDeleteEventItem(int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	int level = GET_IT_LEVEL(lpObj->Map);

	if(IT_LEVEL_RANGE(level) == 0)
	{
		return;
	}

	ILLUSION_TEMPLE_LEVEL* lpLevel = &this->m_IllusionTempleLevel[level];

	if(this->GetUser(lpLevel,aIndex) == 0)
	{
		return;
	}

	if(lpLevel->EventItemOwner != aIndex)
	{
		return;
	}

	int slot = gItemManager.GetInventoryItemSlot(lpObj,GET_ITEM(14,64),lpLevel->EventItemLevel);

	if(slot == -1)
	{
		return;
	}

	if(lpObj->Inventory[slot].m_Serial != lpLevel->EventItemSerial)
	{
		return;
	}

	this->GCIllusionTempleEventItemOwnerS8Send(lpLevel, aIndex, 1);

	gItemManager.InventoryDelItem(lpObj->Index,slot);
	gItemManager.GCItemDeleteSend(lpObj->Index,slot,1);

	lpLevel->EventItemSerial = -1;
	lpLevel->EventItemNumber = -1;
	lpLevel->EventItemOwner = -1;
	lpLevel->EventItemLevel = -1;
}

void CIllusionTemple::GiveUserRewardExperience(ILLUSION_TEMPLE_LEVEL* lpLevel) // OK
{
	for(int n=0;n < MAX_IT_USER;n++)
	{
		if(OBJECT_RANGE(lpLevel->User[n].Index) == 0)
		{
			continue;
		}

		lpLevel->User[n].RewardExperience = lpLevel->TeamRewardExperience[lpLevel->User[n].Team];

		lpLevel->User[n].Score = lpLevel->User[n].RewardExperience/5;

		lpLevel->User[n].RewardExperience = lpLevel->User[n].RewardExperience*gServerInfo.m_AddEventExperienceRate[gObj[lpLevel->User[n].Index].AccountLevel];

		if(gEffectManager.CheckEffect(&gObj[lpLevel->User[n].Index],EFFECT_SEAL_OF_SUSTENANCE1) != 0 || gEffectManager.CheckEffect(&gObj[lpLevel->User[n].Index],EFFECT_SEAL_OF_SUSTENANCE2) != 0)
		{
			continue;
		}

		if(gObjectManager.CharacterLevelUp(&gObj[lpLevel->User[n].Index],lpLevel->User[n].RewardExperience,gServerInfo.m_MaxLevelUpEvent,EXPERIENCE_ILLUSION_TEMPLE) != 0)
		{
			continue;
		}

		GCRewardExperienceSend(lpLevel->User[n].Index,lpLevel->User[n].RewardExperience);
	}
}

bool CIllusionTemple::AddMonster(ILLUSION_TEMPLE_LEVEL* lpLevel,int aIndex) // OK
{
	if(OBJECT_RANGE(aIndex) == 0)
	{
		return 0;
	}

	if(this->GetMonster(lpLevel,aIndex) != 0)
	{
		return 0;
	}

	for(int n=0;n < MAX_IT_MONSTER;n++)
	{
		if(OBJECT_RANGE(lpLevel->MonsterIndex[n]) != 0)
		{
			continue;
		}

		lpLevel->MonsterIndex[n] = aIndex;
		return 1;
	}

	return 0;
}

bool CIllusionTemple::DelMonster(ILLUSION_TEMPLE_LEVEL* lpLevel,int aIndex) // OK
{
	if(OBJECT_RANGE(aIndex) == 0)
	{
		return 0;
	}

	int* index = this->GetMonster(lpLevel,aIndex);

	if(index == 0)
	{
		return 0;
	}

	(*index) = -1;
	return 1;
}

int* CIllusionTemple::GetMonster(ILLUSION_TEMPLE_LEVEL* lpLevel,int aIndex) // OK
{
	if(OBJECT_RANGE(aIndex) == 0)
	{
		return 0;
	}

	for(int n=0;n < MAX_IT_MONSTER;n++)
	{
		if(lpLevel->MonsterIndex[n] == aIndex)
		{
			return &lpLevel->MonsterIndex[n];
		}
	}

	return 0;
}

void CIllusionTemple::CleanMonster(ILLUSION_TEMPLE_LEVEL* lpLevel) // OK
{
	for(int n=0;n < MAX_IT_MONSTER;n++)
	{
		lpLevel->MonsterIndex[n] = -1;
	}
}

void CIllusionTemple::ClearMonster(ILLUSION_TEMPLE_LEVEL* lpLevel) // OK
{
	for(int n=0;n < MAX_IT_MONSTER;n++)
	{
		if(OBJECT_RANGE(lpLevel->MonsterIndex[n]) != 0)
		{
			gObjDel(lpLevel->MonsterIndex[n]);
			lpLevel->MonsterIndex[n] = -1;
		}
	}
}

int CIllusionTemple::GetMonsterCount(ILLUSION_TEMPLE_LEVEL* lpLevel) // OK
{
	int count = 0;

	for(int n=0;n < MAX_IT_MONSTER;n++)
	{
		if(OBJECT_RANGE(lpLevel->MonsterIndex[n]) != 0)
		{
			count++;
		}
	}

	return count;
}

void CIllusionTemple::SetStoneStatue(ILLUSION_TEMPLE_LEVEL* lpLevel) // OK
{
#if (GAMESERVER_UPDATE <= 603)
	if(OBJECT_RANGE(lpLevel->StoneStatueIndex) != 0)
	{
		return;
	}

	int index = gObjAddMonster(lpLevel->Map);
	
	if(OBJECT_RANGE(index) == 0)
	{
		return;
	}

	LPOBJ lpObj = &gObj[index];

	int pos = GetLargeRand()%2;
	int px = 134+(pos*73);
	int py = 121-(pos*74);

	lpObj->PosNum = -1;
	lpObj->X = px;
	lpObj->Y = py;
	lpObj->TX = px;
	lpObj->TY = py;
	lpObj->OldX = px;
	lpObj->OldY = py;
	lpObj->StartX = px;
	lpObj->StartY = py;
	lpObj->Dir = 2;
	lpObj->Map = lpLevel->Map;

	if(gObjSetMonster(index,380) == 0)
	{
		gObjDel(index);
		return;
	}

	lpLevel->StoneStatueIndex = index;

#else

	int pos = GetLargeRand()%2;
	int px = 134+(pos*73);
	int py = 121-(pos*74);

	GDCreateItemSend(9000,lpLevel->Map,(BYTE)px,(BYTE)py,GET_ITEM(14,64),0,255,0,0,0,-1,0,0,0,0,0,0xFF,0);

	lpLevel->StoneStatueIndex = -1;

#endif

	//S8
	//this->GCIllusionTempleStoneInfoSend(lpLevel,0,0);
}

void CIllusionTemple::SetMonster(ILLUSION_TEMPLE_LEVEL* lpLevel,int MonsterClass) // OK
{
	for(int n=0;n < gMonsterSetBase.m_count;n++)
	{
		MONSTER_SET_BASE_INFO* lpInfo = &gMonsterSetBase.m_MonsterSetBaseInfo[n];

		if(lpInfo->Type != 4 || lpInfo->MonsterClass != MonsterClass || lpInfo->Map != lpLevel->Map)
		{
			continue;
		}

		int index = gObjAddMonster(lpLevel->Map);

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

		lpObj->MaxRegenTime = 1000;

		if(this->AddMonster(lpLevel,index) == 0)
		{
			gObjDel(index);
			continue;
		}
	}
}

void CIllusionTemple::NpcStoneStatue(LPOBJ lpNpc,LPOBJ lpObj) // OK
{

	if(gServerInfo.m_IllusionTempleEvent == 0)
	{
		return;
	}

	int level = GET_IT_LEVEL(lpObj->Map);

	if(IT_LEVEL_RANGE(level) == 0)
	{
		return;
	}

	ILLUSION_TEMPLE_LEVEL* lpLevel = &this->m_IllusionTempleLevel[level];

	if(this->GetState(level) != IT_STATE_START)
	{
		return;
	}

	if(lpNpc->Index != lpLevel->StoneStatueIndex)
	{
		return;
	}

	if(lpLevel->EventItemSerial != -1)
	{
		return;
	}

	ILLUSION_TEMPLE_USER* lpUser = this->GetUser(lpLevel,lpObj->Index);

	if(lpUser == 0)
	{
		return;
	}

	GDCreateItemSend(lpObj->Index,lpNpc->Map,(BYTE)lpNpc->X,(BYTE)lpNpc->Y,GET_ITEM(14,64),0,255,0,0,0,lpObj->Index,0,0,0,0,0,0xFF,0);

	GCUserDieSend(&gObj[lpLevel->StoneStatueIndex],lpLevel->StoneStatueIndex,0,0);

	gObjDel(lpLevel->StoneStatueIndex);

	lpLevel->StoneStatueIndex = -1;
}

void CIllusionTemple::NpcYellowStorage(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	if(gServerInfo.m_IllusionTempleEvent == 0)
	{
		return;
	}

	int level = GET_IT_LEVEL(lpObj->Map);

	if(IT_LEVEL_RANGE(level) == 0)
	{
		return;
	}

	ILLUSION_TEMPLE_LEVEL* lpLevel = &this->m_IllusionTempleLevel[level];

	if(lpLevel->State != IT_STATE_START)
	{
		return;
	}

	if(lpLevel->EventItemOwner != lpObj->Index)
	{
		return;
	}

	ILLUSION_TEMPLE_USER* lpUser = this->GetUser(lpLevel,lpObj->Index);

	if(lpUser == 0)
	{
		return;
	}

	if(lpUser->Team != IT_TEAM_Y)
	{
		return;
	}

	int slot = gItemManager.GetInventoryItemSlot(lpObj,GET_ITEM(14,64),lpLevel->EventItemLevel);

	if(slot == -1)
	{
		return;
	}

	if(lpObj->Inventory[slot].m_Serial != lpLevel->EventItemSerial)
	{
		return;
	}

	gItemManager.InventoryDelItem(lpObj->Index,slot);
	gItemManager.GCItemDeleteSend(lpObj->Index,slot,1);

	lpLevel->Score[IT_TEAM_Y]++;
	lpLevel->EventItemSerial = -1;
	lpLevel->EventItemNumber = -1;
	lpLevel->EventItemOwner = -1;
	lpLevel->EventItemLevel = -1;

	this->SetStoneStatue(lpLevel);
}

void CIllusionTemple::NpcBlueStorage(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	if(gServerInfo.m_IllusionTempleEvent == 0)
	{
		return;
	}

	int level = GET_IT_LEVEL(lpObj->Map);

	if(IT_LEVEL_RANGE(level) == 0)
	{
		return;
	}

	ILLUSION_TEMPLE_LEVEL* lpLevel = &this->m_IllusionTempleLevel[level];

	if(lpLevel->State != IT_STATE_START)
	{
		return;
	}

	if(lpLevel->EventItemOwner != lpObj->Index)
	{
		return;
	}

	ILLUSION_TEMPLE_USER* lpUser = this->GetUser(lpLevel,lpObj->Index);

	if(lpUser == 0)
	{
		return;
	}

	if(lpUser->Team != IT_TEAM_B)
	{
		return;
	}

	int slot = gItemManager.GetInventoryItemSlot(lpObj,GET_ITEM(14,64),lpLevel->EventItemLevel);

	if(slot == -1)
	{
		return;
	}

	if(lpObj->Inventory[slot].m_Serial != lpLevel->EventItemSerial)
	{
		return;
	}

	gItemManager.InventoryDelItem(lpObj->Index,slot);
	gItemManager.GCItemDeleteSend(lpObj->Index,slot,1);

	lpLevel->Score[IT_TEAM_B]++;
	lpLevel->EventItemSerial = -1;
	lpLevel->EventItemNumber = -1;
	lpLevel->EventItemOwner = -1;
	lpLevel->EventItemLevel = -1;

	this->SetStoneStatue(lpLevel);

}

void CIllusionTemple::NpcMirageTheMummy(LPOBJ lpNpc,LPOBJ lpObj) // OK
{
	if(gServerInfo.m_IllusionTempleEvent == 0)
	{
		return;
	}

	int level = this->GetUserAbleLevel(lpObj);

	if(IT_LEVEL_RANGE(level) == 0)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(181));
		return;
	}

	if(gItemManager.GetInventoryItemCount(lpObj,GET_ITEM(13,51),(level+1)) == 0 && gItemManager.GetInventoryItemCount(lpObj,GET_ITEM(13,61),0) == 0)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(182));
		return;
	}

	PMSG_NPC_TALK_SEND pMsg;

	pMsg.header.set(0x30,sizeof(pMsg));

	pMsg.result = 20;

	pMsg.level[0] = this->GetEnterEnabled(level);

	pMsg.level[1] = this->GetUserCount(&this->m_IllusionTempleLevel[level],IT_TEAM_NONE);

	DataSend(lpObj->Index,(BYTE*)&pMsg,pMsg.header.size);
}

bool CIllusionTemple::CheckEventItemSerial(int map,CMapItem* lpItem) // OK
{
	int level = GET_IT_LEVEL(map);

	if(IT_LEVEL_RANGE(level) == 0)
	{
		return 0;
	}

	if(this->m_IllusionTempleLevel[level].EventItemSerial == -1)
	{
		return 0;
	}

	if(lpItem->m_Index == GET_ITEM(14,64) && lpItem->m_Serial == this->m_IllusionTempleLevel[level].EventItemSerial)
	{
		return 1;
	}

	return 0;
}

void CIllusionTemple::SetEventItemSerial(int map,int index,int serial) // OK
{
	int level = GET_IT_LEVEL(map);

	if(IT_LEVEL_RANGE(level) == 0)
	{
		return;
	}

	if(this->m_IllusionTempleLevel[level].EventItemSerial != -1)
	{
		return;
	}

	if(index == GET_ITEM(14,64))
	{
		this->m_IllusionTempleLevel[level].EventItemSerial = serial;
	}
}

void CIllusionTemple::DropEventItem(int map,int number,int aIndex) // OK
{
	int level = GET_IT_LEVEL(map);

	if(IT_LEVEL_RANGE(level) == 0)
	{
		return;
	}

	gMap[map].m_Item[number].m_Time = GetTickCount()+15000;

	gMap[map].m_Item[number].m_LootTime = GetTickCount();

	this->m_IllusionTempleLevel[level].EventItemNumber = number;

	this->m_IllusionTempleLevel[level].EventItemOwner = -1;

	this->m_IllusionTempleLevel[level].EventItemLevel = -1;

#if (GAMESERVER_UPDATE <= 603)
	this->NoticeSendToAll(&this->m_IllusionTempleLevel[level],0,gMessage.GetMessage(183),gObj[aIndex].Name);
#endif
}

void CIllusionTemple::GetEventItem(int map,int aIndex,CMapItem* lpItem) // OK
{
	int level = GET_IT_LEVEL(map);

	if(IT_LEVEL_RANGE(level) == 0)
	{
		return;
	}

	this->m_IllusionTempleLevel[level].EventItemNumber = -1;

	this->m_IllusionTempleLevel[level].EventItemOwner = aIndex;

	this->m_IllusionTempleLevel[level].EventItemLevel = lpItem->m_Level;

	this->GCIllusionTempleEventItemOwnerSend(&this->m_IllusionTempleLevel[level]);

	this->GCIllusionTempleEventItemOwnerS8Send(&this->m_IllusionTempleLevel[level], aIndex, 0);

	this->NoticeSendToAll(&this->m_IllusionTempleLevel[level],0,gMessage.GetMessage(184),gObj[aIndex].Name);
}

void CIllusionTemple::DestroyEventItem(int map,CMapItem* lpItem) // OK
{
	int level = GET_IT_LEVEL(map);

	if(IT_LEVEL_RANGE(level) == 0)
	{
		return;
	}

	lpItem->m_State = OBJECT_DIECMD;

	this->GCIllusionTempleEventItemOwnerS8Send(&this->m_IllusionTempleLevel[level], this->m_IllusionTempleLevel[level].EventItemOwner, 1);

	this->m_IllusionTempleLevel[level].EventItemSerial = -1;

	this->m_IllusionTempleLevel[level].EventItemNumber = -1;

	this->m_IllusionTempleLevel[level].EventItemOwner = -1;

	this->m_IllusionTempleLevel[level].EventItemLevel = -1;

	this->SetStoneStatue(&this->m_IllusionTempleLevel[level]);
}

void CIllusionTemple::UserDieProc(LPOBJ lpObj,LPOBJ lpTarget) // OK
{
	int level = GET_IT_LEVEL(lpObj->Map);

	if(IT_LEVEL_RANGE(level) == 0)
	{
		return;
	}

	if(this->GetState(level) != IT_STATE_START)
	{
		return;
	}

	if(this->GetUser(&this->m_IllusionTempleLevel[level],lpObj->Index) == 0)
	{
		return;
	}

	this->SearchUserDropEventItem(lpObj->Index);

	gEffectManager.DelEffect(lpObj,EFFECT_ORDER_OF_PROTECTION);

	gEffectManager.DelEffect(lpObj,EFFECT_ORDER_OF_RESTRAINT);

	if(lpTarget->Type != OBJECT_USER)
	{
		return;
	}

	ILLUSION_TEMPLE_USER* lpUser = this->GetUser(&this->m_IllusionTempleLevel[level],lpTarget->Index);

	if(lpUser == 0)
	{
		return;
	}

#if (GAMESERVER_UPDATE <= 603)
	if((lpUser->Point+5) > 90)
	{
		lpUser->Point = 90;
	}
	else
	{
		lpUser->Point += 5;
	}

	this->GCIllusionTempleKillPointSend(lpUser->Index,lpUser->Point);
#else
	if (lpUser->Point >= 255)
	{
		return;
	}
	else
	{
		lpUser->Point++;
	}
#endif
}

void CIllusionTemple::MonsterDieProc(LPOBJ lpObj,LPOBJ lpTarget) // OK
{
#if (GAMESERVER_UPDATE <= 603)
	int aIndex = gObjMonsterGetTopHitDamageUser(lpObj);

	if(OBJECT_RANGE(aIndex) != 0)
	{
		lpTarget = &gObj[aIndex];
	}

	int level = GET_IT_LEVEL(lpObj->Map);

	if(IT_LEVEL_RANGE(level) == 0)
	{
		return;
	}

	if(this->GetState(level) != IT_STATE_START)
	{
		return;
	}

	if(this->GetMonster(&this->m_IllusionTempleLevel[level],lpObj->Index) == 0)
	{
		return;
	}

	if(lpTarget->Type != OBJECT_USER)
	{
		return;
	}

	ILLUSION_TEMPLE_USER* lpUser = this->GetUser(&this->m_IllusionTempleLevel[level],lpTarget->Index);

	if(lpUser == 0)
	{
		return;
	}

	if((lpUser->Point+2) > 90)
	{
		lpUser->Point = 90;
	}
	else
	{
		lpUser->Point += 2;
	}

	this->GCIllusionTempleKillPointSend(lpUser->Index,lpUser->Point);
#endif
}

bool CIllusionTemple::CheckPlayerTarget(LPOBJ lpObj,LPOBJ lpTarget) // OK
{
	int level = GET_IT_LEVEL(lpObj->Map);

	if(IT_LEVEL_RANGE(level) == 0)
	{
		return 0;
	}

	if(this->GetState(level) != IT_STATE_START)
	{
		return 0;
	}

	ILLUSION_TEMPLE_USER* lpUser1 = this->GetUser(&this->m_IllusionTempleLevel[level],lpObj->Index);

	if(lpUser1 == 0)
	{
		return 0;
	}

	ILLUSION_TEMPLE_USER* lpUser2 = this->GetUser(&this->m_IllusionTempleLevel[level],lpTarget->Index);

	if(lpUser2 == 0)
	{
		return 0;
	}

	if(lpUser1->Team == lpUser2->Team)
	{
		return 0;
	}

	return 1;
}

void CIllusionTemple::NoticeSendToAll(ILLUSION_TEMPLE_LEVEL* lpLevel,int type,char* message,...) // OK
{
	char buff[256];

	va_list arg;
	va_start(arg,message);
	vsprintf_s(buff,message,arg);
	va_end(arg);

	for(int n=0;n < MAX_IT_USER;n++)
	{
		if(OBJECT_RANGE(lpLevel->User[n].Index) != 0)
		{
			gNotice.GCNoticeSend(lpLevel->User[n].Index,type,0,0,0,0,0,buff);
		}
	}
}

void CIllusionTemple::DataSendToAll(ILLUSION_TEMPLE_LEVEL* lpLevel,BYTE* lpMsg,int size) // OK
{
	for(int n=0;n < MAX_IT_USER;n++)
	{
		if(OBJECT_RANGE(lpLevel->User[n].Index) != 0)
		{
			DataSend(lpLevel->User[n].Index,lpMsg,size);
		}
	}
}

void CIllusionTemple::CGIllusionTempleEnterRecv(PMSG_ILLUSION_TEMPLE_ENTER_RECV* lpMsg,int aIndex) // OK
{
	if(gServerInfo.m_IllusionTempleEvent == 0)
	{
		return;
	}

	LPOBJ lpObj = &gObj[aIndex];

	if(gObjIsConnectedGP(aIndex) == 0)
	{
		return;
	}

	PMSG_ILLUSION_TEMPLE_ENTER_SEND pMsg;

	pMsg.header.set(0xBF,0x70,sizeof(pMsg));

	pMsg.result = 0;

	lpMsg->level = this->GetUserAbleLevel(lpObj);

	ILLUSION_TEMPLE_LEVEL* lpLevel = &this->m_IllusionTempleLevel[lpMsg->level];

	if(INVENTORY_FULL_RANGE(lpMsg->slot) == 0)
	{
		pMsg.result = 1;
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		//LogAdd(LOG_BLACK,"OK1");
		return;
	}

		int level = this->GetUserAbleLevel(lpObj);

	if(gItemManager.GetInventoryItemCount(lpObj,GET_ITEM(13,51),(level+1)) == 0 && gItemManager.GetInventoryItemCount(lpObj,GET_ITEM(13,61),0) == 0)
	{
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(182));
		return;
	}

	if(lpObj->Inventory[lpMsg->slot].m_Index == GET_ITEM(13,51) && lpObj->Inventory[lpMsg->slot].m_Level != (lpMsg->level+1))
	{
		pMsg.result = 3;
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		//LogAdd(LOG_BLACK,"OK3");
		return;
	}

	if(OBJECT_RANGE(lpObj->PartyNumber) != 0)
	{
		pMsg.result = 1;
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		//LogAdd(LOG_BLACK,"OK4");
		return;
	}

	if(lpObj->Guild != 0 && lpObj->Guild->WarState == 1)
	{
		pMsg.result = 1;
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		//LogAdd(LOG_BLACK,"OK5");
		return;
	}

	if(OBJECT_RANGE(lpObj->DuelUser) != 0)
	{
		pMsg.result = 1;
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		//LogAdd(LOG_BLACK,"OK6");
		return;
	}

	if(lpObj->PShopOpen != 0)
	{
		pMsg.result = 1;
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		//LogAdd(LOG_BLACK,"OK7");
		return;
	}

	if(this->GetEnterEnabled(lpMsg->level) == 0)
	{
		pMsg.result = 3;
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		gNotice.GCNoticeSend(lpObj->Index,1,0,0,0,0,0,gMessage.GetMessage(305));
		LogAdd(LOG_BLACK,"OK8");
		return;
	}

	if(gMapServerManager.CheckMapServer(lpLevel->Map) == 0)
	{
		pMsg.result = 2;
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		return;
	}

	if(this->GetUserAbleLevel(lpObj) != lpMsg->level)
	{
		pMsg.result = 3;
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		//LogAdd(LOG_BLACK,"OK10");
		return;
	}

	if(gServerInfo.m_PKLimitFree == 0 && lpObj->PKLevel >= 4)
	{
		pMsg.result = 7;
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		//LogAdd(LOG_BLACK,"OK11");
		return;
	}

	if(lpObj->Change != -1)
	{
		pMsg.result = 8;
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		//LogAdd(LOG_BLACK,"OK12");
		return;
	}

	if(this->AddUser(lpLevel,aIndex) == 0)
	{
		pMsg.result = 4;
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
		//LogAdd(LOG_BLACK,"OK13");
		return;
	}
	
	gItemManager.DecreaseItemDur(lpObj,lpMsg->slot,1);

	//fix ilusion invite
	if(lpObj->Inventory[lpMsg->slot].m_Index == GET_ITEM(13,19))
	{
		gItemManager.InventoryDelItem(lpObj->Index,lpMsg->slot);
		gItemManager.GCItemDeleteSend(lpObj->Index,lpMsg->slot,1);
	}

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);

	switch(lpMsg->level)
	{
		case 0:
			gObjMoveGate(aIndex,142);
			break;
		case 1:
			gObjMoveGate(aIndex,143);
			break;
		case 2:
			gObjMoveGate(aIndex,144);
			break;
		case 3:
			gObjMoveGate(aIndex,145);
			break;
		case 4:
			gObjMoveGate(aIndex,146);
			break;
		case 5:
			gObjMoveGate(aIndex,147);
			break;
	}

	this->GCIllusionTempleChangeScenarioSend(lpLevel,0,aIndex);
}

void CIllusionTemple::CGIllusionTempleSkillRecv(PMSG_ILLUSION_TEMPLE_SKILL_RECV* lpMsg,int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	int level = GET_IT_LEVEL(lpObj->Map);

	if(IT_LEVEL_RANGE(level) == 0)
	{
		return;
	}

	ILLUSION_TEMPLE_USER* lpUser = this->GetUser(&this->m_IllusionTempleLevel[level],aIndex);

	if(lpUser == 0)
	{
		return;
	}

	int bIndex = MAKE_NUMBERW(lpMsg->index[0],lpMsg->index[1]);

	if(OBJECT_RANGE(bIndex) == 0)
	{
		return;
	}

	int skill = MAKE_NUMBERW(lpMsg->skill[0],lpMsg->skill[1]);

	SKILL_INFO SkillInfo;

	if(gSkillManager.GetInfo(skill,&SkillInfo) == 0)
	{
		this->GCIllusionTempleAddSkillSend(aIndex,bIndex,0,skill);
		return;
	}

	if(lpUser->Point < SkillInfo.RequireKillCount)
	{
		this->GCIllusionTempleAddSkillSend(aIndex,bIndex,0,skill);
		return;
	}

	int result = 0;

	switch(skill)
	{
		case SKILL_ORDER_OF_PROTECTION:
		case SKILL_SPROTECTION:
			result = gSkillManager.SkillOrderOfProtection(aIndex,bIndex);
			break;
		case SKILL_ORDER_OF_RESTRAINT:
		case SKILL_SRESTRICTION:
			result = gSkillManager.SkillOrderOfRestraint(aIndex,bIndex);
			break;
		case SKILL_ORDER_OF_TRACKING:
		case SKILL_SPURSUIT:
			result = gSkillManager.SkillOrderOfTracking(aIndex,this->m_IllusionTempleLevel[level].EventItemOwner);
			break;
		case SKILL_ORDER_OF_WEAKEN:
		case SKILL_SSHIELD_BURN:
			result = gSkillManager.SkillOrderOfWeaken(aIndex,bIndex);
			break;
	}

	if(result == 0)
	{
		this->GCIllusionTempleAddSkillSend(aIndex,bIndex,0,skill);
	}
	else
	{
		lpUser->Point -= SkillInfo.RequireKillCount;
		this->GCIllusionTempleKillPointSend(aIndex,lpUser->Point);
		this->GCIllusionTempleAddSkillSend(aIndex,bIndex,1,skill);
	}
}

void CIllusionTemple::CGIllusionTempleButtonClickRecv(int aIndex) // OK
{
	LPOBJ lpObj = &gObj[aIndex];

	int level = GET_IT_LEVEL(lpObj->Map);

	if(IT_LEVEL_RANGE(level) == 0)
	{
		return;
	}

	if(this->GetState(level) != IT_STATE_CLEAN)
	{
		return;
	}

	ILLUSION_TEMPLE_USER* lpUser = this->GetUser(&this->m_IllusionTempleLevel[level],lpObj->Index);

	if(lpUser == 0)
	{
		return;
	}

	if(lpUser->Team != this->m_IllusionTempleLevel[level].WinnerTeam)
	{
		return;
	}

	if((GetLargeRand()%10000) < gServerInfo.m_IllusionTempleRewardFenrirRate)
	{
		GDCreateItemSend(lpObj->Index,lpObj->Map,(BYTE)lpObj->X,(BYTE)lpObj->Y,GET_ITEM(13,37),0,0,0,0,0,lpObj->Index,4,0,0,0,0,0xFF,0);
		return;
	}

	switch(level)
	{
		case 0:
			gItemBagManager.DropItemBySpecialValue(ITEM_BAG_ILLUSION_TEMPLE1,lpObj,lpObj->Map,lpObj->X,lpObj->Y);
			break;
		case 1:
			gItemBagManager.DropItemBySpecialValue(ITEM_BAG_ILLUSION_TEMPLE2,lpObj,lpObj->Map,lpObj->X,lpObj->Y);
			break;
		case 2:
			gItemBagManager.DropItemBySpecialValue(ITEM_BAG_ILLUSION_TEMPLE3,lpObj,lpObj->Map,lpObj->X,lpObj->Y);
			break;
		case 3:
			gItemBagManager.DropItemBySpecialValue(ITEM_BAG_ILLUSION_TEMPLE4,lpObj,lpObj->Map,lpObj->X,lpObj->Y);
			break;
		case 4:
			gItemBagManager.DropItemBySpecialValue(ITEM_BAG_ILLUSION_TEMPLE5,lpObj,lpObj->Map,lpObj->X,lpObj->Y);
			break;
		case 5:
			gItemBagManager.DropItemBySpecialValue(ITEM_BAG_ILLUSION_TEMPLE6,lpObj,lpObj->Map,lpObj->X,lpObj->Y);
			break;
	}

	if (this->m_IllusionTempleReward[level][0] > 0 || this->m_IllusionTempleReward[level][1] > 0 || this->m_IllusionTempleReward[level][2] > 0)
	{
		//Reward
		GDSetCoinSend(lpUser->Index, this->m_IllusionTempleReward[level][0], this->m_IllusionTempleReward[level][1], this->m_IllusionTempleReward[level][2],"IT");
		
		GCFireworksSend(&gObj[lpUser->Index],gObj[lpUser->Index].X,gObj[lpUser->Index].Y);
	}
}

void CIllusionTemple::GCIllusionTempleBattleInfoSend(ILLUSION_TEMPLE_LEVEL* lpLevel) // OK
{
#if GAMESERVER_UPDATE <= 603
	for(int n=0;n < MAX_IT_USER;n++)
	{
		ILLUSION_TEMPLE_USER* lpUser = &lpLevel->User[n];

		if(OBJECT_RANGE(lpUser->Index) == 0)
		{
			continue;
		}

		BYTE send[256];

		PMSG_ILLUSION_TEMPLE_BATTLE_INFO_SEND pMsg;

		pMsg.header.set(0xBF,0x01,0);

		int size = sizeof(pMsg);

		pMsg.time = lpLevel->RemainTime;

		if(OBJECT_RANGE(lpLevel->EventItemOwner) == 0)
		{
			pMsg.EventItemOwner = 0xFFFF;
			pMsg.EventItemOwnerX = 0xFF;
			pMsg.EventItemOwnerY = 0xFF;
		}
		else
		{
			pMsg.EventItemOwner = lpLevel->EventItemOwner;
			pMsg.EventItemOwnerX = (BYTE)gObj[lpLevel->EventItemOwner].X;
			pMsg.EventItemOwnerY = (BYTE)gObj[lpLevel->EventItemOwner].Y;
		}

		pMsg.score[IT_TEAM_Y] = lpLevel->Score[IT_TEAM_Y];

		pMsg.score[IT_TEAM_B] = lpLevel->Score[IT_TEAM_B];

		pMsg.team = lpUser->Team;

		pMsg.count = 0;
		
		PMSG_ILLUSION_TEMPLE_BATTLE_INFO info;
		
		for(int i=0;i < MAX_IT_USER;i++)
		{
			if(OBJECT_RANGE(lpLevel->User[i].Index) == 0)
			{
				continue;
			}

			if(lpLevel->User[i].Team != lpUser->Team)
			{
				continue;
			}

			info.index = lpLevel->User[i].Index;
			info.map = gObj[lpLevel->User[i].Index].Map;
			info.x = (BYTE)gObj[lpLevel->User[i].Index].X;
			info.y = (BYTE)gObj[lpLevel->User[i].Index].Y;

			memcpy(&send[size],&info,sizeof(info));
			size += sizeof(info);

			pMsg.count++;
		}

		pMsg.header.size = size;

		memcpy(send,&pMsg,sizeof(pMsg));

		DataSend(lpUser->Index,send,size);
	}
#else
	for(int n=0;n < MAX_IT_USER;n++)
	{
		ILLUSION_TEMPLE_USER* lpUser = &lpLevel->User[n];

		if(OBJECT_RANGE(lpUser->Index) == 0)
		{
			continue;
		}

		PMSG_ILLUSION_TEMPLE_BATTLE_INFOS8_SEND pMsg;

		pMsg.wRemainSec = lpLevel->RemainTime;
		pMsg.byAlliedPoint = lpLevel->Score[IT_TEAM_Y];
		pMsg.btIllusionPoint = lpLevel->Score[IT_TEAM_B];
		pMsg.btMyTeam = lpUser->Team;

		pMsg.header.set(0xBF,0x62, sizeof(pMsg));

		DataSend(lpUser->Index,(BYTE*)&pMsg,pMsg.header.size);
	}
#endif
}

void CIllusionTemple::GCIllusionTempleAddSkillSend(int aIndex,int result,int skill,int bIndex) // OK
{
	PMSG_ILLUSION_TEMPLE_ADD_SKILL_SEND pMsg;

	pMsg.header.set(0xBF,0x02,sizeof(pMsg));

	pMsg.result = result;

	pMsg.skill[0] = SET_NUMBERHB(skill);

	pMsg.skill[1] = SET_NUMBERLB(skill);

	pMsg.index = aIndex;

	pMsg.target = bIndex;

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);

	if(result != 0)
	{
		MsgSendV2(&gObj[aIndex],(BYTE*)&pMsg,pMsg.header.size);
	}
}

void CIllusionTemple::GCIllusionTempleRewardScoreSend(ILLUSION_TEMPLE_LEVEL* lpLevel) // OK
{
#if GAMESERVER_UPDATE <= 603
	BYTE send[256];

	PMSG_ILLUSION_TEMPLE_REWARD_SCORE_SEND pMsg;

	pMsg.header.set(0xBF,0x04,0);

	int size = sizeof(pMsg);

	pMsg.score[IT_TEAM_Y] = lpLevel->Score[IT_TEAM_Y];

	pMsg.score[IT_TEAM_B] = lpLevel->Score[IT_TEAM_B];

	pMsg.count = 0;

	PMSG_ILLUSION_TEMPLE_REWARD_SCORE info;

	for(int n=0;n < MAX_IT_USER;n++)
	{
		if(OBJECT_RANGE(lpLevel->User[n].Index) == 0)
		{
			continue;
		}

		memcpy(info.name,gObj[lpLevel->User[n].Index].Name,sizeof(info.name));

		info.map = lpLevel->Map;

		info.team = lpLevel->User[n].Team;

		info.Class = (BYTE)gObj[lpLevel->User[n].Index].Class;

		info.Experience = lpLevel->User[n].RewardExperience;

		GDRankingIllusionTempleSaveSend(lpLevel->User[n].Index,lpLevel->User[n].Score);

		memcpy(&send[size],&info,sizeof(info));
		size += sizeof(info);

		pMsg.count++;
	}

	pMsg.header.size = size;

	memcpy(send,&pMsg,sizeof(pMsg));

	this->DataSendToAll(lpLevel,send,size);
#else

	BYTE send[256];

	PMSG_ILLUSION_TEMPLE_REWARD_SCORES8_SEND pMsg;

	pMsg.header.set(0xBF,0x04,0);

	int size = sizeof(pMsg);

	pMsg.btUserCount = 0;
	pMsg.byWinTeamIndex = 0;
	pMsg.btAlliedPoint = lpLevel->Score[IT_TEAM_Y]+1;
	pMsg.btIllusionPoint = lpLevel->Score[IT_TEAM_B]+2;

	LogAdd(LOG_EVENT,"[1] winner %d, point 1 %d, point 2 %d",lpLevel->WinnerTeam,lpLevel->Score[IT_TEAM_Y],lpLevel->Score[IT_TEAM_B]);

	PMSG_ILLUSION_TEMPLE_REWARD_SCORES8 info;

	//for(int n=0;n < MAX_IT_USER;n++)
	//{
	//	if(OBJECT_RANGE(lpLevel->User[n].Index) == 0)
	//	{
	//		continue;
	//	}

	//	if(gObjIsConnected(lpLevel->User[n].Index) == 0)
	//	{
	//		continue;
	//	}

	//	if(gObj[lpLevel->User[n].Index].Map != lpLevel->Map)
	//	{
	//		continue;
	//	}

	//	memcpy(info.name,gObj[lpLevel->User[n].Index].Name,sizeof(info.name));

	//	info.map = lpLevel->Map;

	//	info.team = lpLevel->User[n].Team;

	//	info.Class = (BYTE)gObj[lpLevel->User[n].Index].Class;

	//	info.btUserKillCount = 2;

	//	GDRankingIllusionTempleSaveSend(lpLevel->User[n].Index,lpLevel->User[n].Score);

	//	memcpy(&send[size],&info,sizeof(info));
	//	size += sizeof(info);

	//	pMsg.btUserCount++;

	//	LogAdd(LOG_EVENT,"[1] count %d, map %d, team %d, class %d, kills %d",pMsg.btUserCount,lpLevel->Map,lpLevel->User[n].Team, gObj[lpLevel->User[n].Index].Class,lpLevel->User[n].Score);
	//}

	for(int n=0;n < MAX_IT_USER;n++)
	{

		char fulltext[11]; 
        wsprintf(fulltext,"teste%d",n);

		memcpy(info.name,fulltext,sizeof(info.name));

		info.map = lpLevel->Map;

		info.team = 0 + (n>4)?1:0;

		info.Class = 1;

		info.btUserKillCount = 2;

		memcpy(&send[size],&info,sizeof(info));
		size += sizeof(info);

		pMsg.btUserCount++;
	}

	pMsg.header.size = size;

	memcpy(send,&pMsg,sizeof(pMsg));

	this->DataSendToAll(lpLevel,send,size);

#endif
}

void CIllusionTemple::GCIllusionTempleKillPointSend(int aIndex,int point) // OK
{
#if GAMESERVER_UPDATE <= 603
	PMSG_ILLUSION_TEMPLE_KILL_POINT_SEND pMsg;

	pMsg.header.set(0xBF,0x06,sizeof(pMsg));

	pMsg.KillPoint = point;

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
#endif
}

void CIllusionTemple::GCIllusionTempleDelSkillSend(int aIndex,int skill) // OK
{
#if (GAMESERVER_UPDATE <= 603)
	PMSG_ILLUSION_TEMPLE_DEL_SKILL_SEND pMsg;

	pMsg.header.set(0xBF,0x07,sizeof(pMsg));

	pMsg.skill[0] = SET_NUMBERHB(skill);

	pMsg.skill[1] = SET_NUMBERLB(skill);

	pMsg.index = aIndex;

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
	MsgSendV2(&gObj[aIndex],(BYTE*)&pMsg,pMsg.header.size);
#else
	PMSG_ILLUSION_TEMPLE_DEL_SKILL_SEND pMsg;

	pMsg.header.set(0xBF,0x63,sizeof(pMsg));

	pMsg.skill[0] = SET_NUMBERHB(skill);

	pMsg.skill[1] = SET_NUMBERLB(skill);

	pMsg.index = aIndex;

	DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
	MsgSendV2(&gObj[aIndex],(BYTE*)&pMsg,pMsg.header.size);
#endif
}

void CIllusionTemple::GCIllusionTempleEventItemOwnerSend(ILLUSION_TEMPLE_LEVEL* lpLevel) // OK
{
#if (GAMESERVER_UPDATE <= 603)
	PMSG_ILLUSION_TEMPLE_EVENT_ITEM_OWNER_SEND pMsg;

	pMsg.header.set(0xBF,0x08,sizeof(pMsg));

	pMsg.index = lpLevel->EventItemOwner;

	memcpy(pMsg.name,gObj[lpLevel->EventItemOwner].Name,sizeof(pMsg.name));

	this->DataSendToAll(lpLevel,(BYTE*)&pMsg,pMsg.header.size);
#endif
}

void CIllusionTemple::GCIllusionTempleEventItemOwnerS8Send(ILLUSION_TEMPLE_LEVEL* lpLevel, int Index, int byGet) // OK
{
#if (GAMESERVER_UPDATE >= 803)
	PMSG_ILLUSION_TEMPLE_EVENT_ITEM_OWNER_SEND pMsg;

	pMsg.header.set(0xBF,0x61,sizeof(pMsg));

	pMsg.index = Index;

	memcpy(pMsg.name,gObj[Index].Name,sizeof(pMsg.name));

	pMsg.byGet = byGet;

	this->DataSendToAll(lpLevel,(BYTE*)&pMsg,pMsg.header.size);
#endif
}

void CIllusionTemple::GCIllusionTempleChangeScenarioSend(ILLUSION_TEMPLE_LEVEL* lpLevel,int state,int aIndex) // OK
{
#if GAMESERVER_UPDATE <= 603
	PMSG_ILLUSION_TEMPLE_CHANGE_SCENARIO_SEND pMsg;

	pMsg.header.set(0xBF,0x09,sizeof(pMsg));

	pMsg.level = lpLevel->Level;

	pMsg.state = state;

	if(OBJECT_RANGE(aIndex) == 0)
	{
		this->DataSendToAll(lpLevel,(BYTE*)&pMsg,pMsg.header.size);
	}
	else
	{
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
	}

#else
	PMSG_ILLUSION_TEMPLE_CHANGE_SCENARIO_SEND pMsg;

	pMsg.header.set(0xBF,0x60,sizeof(pMsg));

	pMsg.level = lpLevel->Level;

	pMsg.state = state;

	if(OBJECT_RANGE(aIndex) == 0)
	{
		this->DataSendToAll(lpLevel,(BYTE*)&pMsg,pMsg.header.size);
	}
	else
	{
		DataSend(aIndex,(BYTE*)&pMsg,pMsg.header.size);
	}
#endif
}

void CIllusionTemple::StartIT()
{
	CTime CurrentTime = CTime::GetTickCount();

	int hour	= (int)CurrentTime.GetHour();
	int minute	= (int)CurrentTime.GetMinute()+2;


	if (minute >= 60)
	{
		hour++;
		minute = minute-60; 
	}

	LogAdd(LOG_EVENT,"[Set IT Start] At %02d:%02d:00",hour,minute);

	ILLUSION_TEMPLE_START_TIME info;

	info.Year = (int)CurrentTime.GetYear();

	info.Month = (int)CurrentTime.GetMonth();

	info.Day = (int)CurrentTime.GetDay();

	info.DayOfWeek = -1;

	info.Hour = hour;

	info.Minute = minute;

	info.Second = 0;

	this->m_IllusionTempleStartTime.push_back(info);

	this->Init();
}

void CIllusionTemple::GCIllusionTempleStoneInfoSend(ILLUSION_TEMPLE_LEVEL* lpLevel,int type,int state) // OK
{
#if (GAMESERVER_UPDATE >= 803)
	for(int n=0;n < MAX_IT_USER;n++)
	{
		ILLUSION_TEMPLE_USER* lpUser = &lpLevel->User[n];

		if(OBJECT_RANGE(lpUser->Index) == 0)
		{
			continue;
		}

		PMSG_ILLUSION_TEMPLE_STONEINFO pMsg;

		pMsg.byMapTagIndex = lpLevel->Map;
		pMsg.wOccupiedStoneType = 1;
		pMsg.byOccupiedStoneState = 0;
		pMsg.nNpcIndex = lpLevel->StoneStatueIndex;

		pMsg.header.set(0xBF,0x6C, sizeof(pMsg));

		DataSend(lpUser->Index,(BYTE*)&pMsg,pMsg.header.size);
	}
#endif
}
