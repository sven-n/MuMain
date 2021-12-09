// CustomOnlineLottery.cpp: implementation of the CCustomOnlineLottery class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CustomOnlineLottery.h"
#include "DSProtocol.h"
#include "ItemBagManager.h"
#include "Map.h"
#include "MapManager.h"
#include "MemScript.h"
#include "Notice.h"
#include "ScheduleManager.h"
#include "User.h"
#include "Util.h"
#include "Viewport.h"

CCustomOnlineLottery gCustomOnlineLottery;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCustomOnlineLottery::CCustomOnlineLottery() // OK
{
	for(int n=0;n < MAX_CUSTOM_ONLINE_LOTTERY;n++)
	{
		CUSTOM_ONLINE_LOTTERY_INFO* lpInfo = &this->m_CustomOnlineLotteryInfo[n];

		lpInfo->Index = n;
		lpInfo->State = CUSTOM_ONLINE_LOTTERY_STATE_BLANK;
		lpInfo->RemainTime = 0;
		lpInfo->TargetTime = 0;
		lpInfo->TickCount = GetTickCount();
		lpInfo->AlarmMinSave = -1;
		lpInfo->AlarmMinLeft = -1;
	}
}

CCustomOnlineLottery::~CCustomOnlineLottery() // OK
{

}

void CCustomOnlineLottery::Init() // OK
{
	for(int n=0;n < MAX_CUSTOM_ONLINE_LOTTERY;n++)
	{
		if(this->m_CustomOnlineLotterySwitch == 0)
		{
			this->SetState(&this->m_CustomOnlineLotteryInfo[n],CUSTOM_ONLINE_LOTTERY_STATE_BLANK);
		}
		else
		{
			this->SetState(&this->m_CustomOnlineLotteryInfo[n],CUSTOM_ONLINE_LOTTERY_STATE_EMPTY);
		}
	}
}

void CCustomOnlineLottery::ReadCustomOnlineLotteryInfo(char* section,char* path) // OK
{
	this->m_CustomOnlineLotterySwitch = GetPrivateProfileInt(section,"CustomOnlineLotterySwitch",0,path);

	GetPrivateProfileString(section,"CustomOnlineLotteryText1","",this->m_CustomOnlineLotteryText1,sizeof(this->m_CustomOnlineLotteryText1),path);

	GetPrivateProfileString(section,"CustomOnlineLotteryText2","",this->m_CustomOnlineLotteryText2,sizeof(this->m_CustomOnlineLotteryText2),path);

	GetPrivateProfileString(section,"CustomOnlineLotteryText3","",this->m_CustomOnlineLotteryText3,sizeof(this->m_CustomOnlineLotteryText3),path);
}

void CCustomOnlineLottery::Load(char* path) // OK
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

	for(int n=0;n < MAX_CUSTOM_ONLINE_LOTTERY;n++)
	{
		this->m_CustomOnlineLotteryInfo[n].StartTime.clear();
		this->m_CustomOnlineLotteryInfo[n].RuleInfo.DropItem.clear();
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

					CUSTOM_ONLINE_LOTTERY_START_TIME info;

					int index = lpMemScript->GetNumber();

					info.Year = lpMemScript->GetAsNumber();

					info.Month = lpMemScript->GetAsNumber();

					info.Day = lpMemScript->GetAsNumber();

					info.DayOfWeek = lpMemScript->GetAsNumber();

					info.Hour = lpMemScript->GetAsNumber();

					info.Minute = lpMemScript->GetAsNumber();

					info.Second = lpMemScript->GetAsNumber();

					this->m_CustomOnlineLotteryInfo[index].StartTime.push_back(info);
				}
				else if(section == 1)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					int index = lpMemScript->GetNumber();

					strcpy_s(this->m_CustomOnlineLotteryInfo[index].RuleInfo.Name,lpMemScript->GetAsString());

					this->m_CustomOnlineLotteryInfo[index].RuleInfo.Map = lpMemScript->GetAsNumber();

					this->m_CustomOnlineLotteryInfo[index].RuleInfo.AlarmTime = lpMemScript->GetAsNumber();

					this->m_CustomOnlineLotteryInfo[index].RuleInfo.EventTime = 0;
				}
				else if(section == 2)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					int index = lpMemScript->GetNumber();

					this->m_CustomOnlineLotteryInfo[index].Reward.Coin1 = lpMemScript->GetAsNumber();

					this->m_CustomOnlineLotteryInfo[index].Reward.Coin2 = lpMemScript->GetAsNumber();

					this->m_CustomOnlineLotteryInfo[index].Reward.Coin3 = lpMemScript->GetAsNumber();

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

void CCustomOnlineLottery::MainProc() // OK
{
	for(int n=0;n < MAX_CUSTOM_ONLINE_LOTTERY;n++)
	{
		CUSTOM_ONLINE_LOTTERY_INFO* lpInfo = &this->m_CustomOnlineLotteryInfo[n];

		if((GetTickCount()-lpInfo->TickCount) >= 1000)
		{
			lpInfo->TickCount = GetTickCount();

			lpInfo->RemainTime = (int)difftime(lpInfo->TargetTime,time(0));

			if(this->m_CustomOnlineLotterySwitch == 0)
			{
				if (gServerDisplayer.EventCustomLottery != -1)
				{
					gServerDisplayer.EventCustomLottery = -1;
				}
			}
			else 
			{
				if (lpInfo->RemainTime >= 0 && (lpInfo->RemainTime < gServerDisplayer.EventCustomLottery || gServerDisplayer.EventCustomLottery <= 0))
				{
					if (lpInfo->State == CUSTOM_ONLINE_LOTTERY_STATE_EMPTY)
					{
						gServerDisplayer.EventCustomLottery = lpInfo->RemainTime;
					}
					else 
					{
						if (gServerDisplayer.EventCustomLottery != 0)
						{
							gServerDisplayer.EventCustomLottery = 0;
						}
					}
				}
			}
			

			switch(lpInfo->State)
			{
				case CUSTOM_ONLINE_LOTTERY_STATE_BLANK:
					this->ProcState_BLANK(lpInfo);
					break;
				case CUSTOM_ONLINE_LOTTERY_STATE_EMPTY:
					this->ProcState_EMPTY(lpInfo);
					break;
				case CUSTOM_ONLINE_LOTTERY_STATE_START:
					this->ProcState_START(lpInfo);
					break;
			}
		}
	}
}

void CCustomOnlineLottery::ProcState_BLANK(CUSTOM_ONLINE_LOTTERY_INFO* lpInfo) // OK
{

}

void CCustomOnlineLottery::ProcState_EMPTY(CUSTOM_ONLINE_LOTTERY_INFO* lpInfo) // OK
{
	if(lpInfo->RemainTime > 0 && lpInfo->RemainTime <= (lpInfo->RuleInfo.AlarmTime*60))
	{
		if((lpInfo->AlarmMinSave=(((lpInfo->RemainTime%60)==0)?((lpInfo->RemainTime/60)-1):(lpInfo->RemainTime/60))) != lpInfo->AlarmMinLeft)
		{
			lpInfo->AlarmMinLeft = lpInfo->AlarmMinSave;

			gNotice.GCNoticeSendToAll(0,0,0,0,0,0,this->m_CustomOnlineLotteryText1,lpInfo->RuleInfo.Name,(lpInfo->AlarmMinLeft+1));
		}
	}

	if(lpInfo->RemainTime <= 0)
	{
		this->SetState(lpInfo,CUSTOM_ONLINE_LOTTERY_STATE_START);
	}
}

void CCustomOnlineLottery::ProcState_START(CUSTOM_ONLINE_LOTTERY_INFO* lpInfo) // OK
{
	int Map = lpInfo->RuleInfo.Map;

	if(lpInfo->RemainTime <= 0)
	{
		
					int index = this->GetIndexSorted(Map);

					if (index > 0 )
					{
						
						LPOBJ lpObj = &gObj[index];

						gItemBagManager.DropItemBySpecialValue(ITEM_BAG_LOTTERY,lpObj,lpObj->Map,lpObj->X,lpObj->Y);

						GCFireworksSend(lpObj,lpObj->X,lpObj->Y);

						gNotice.GCNoticeSendToAll(0,0,0,0,0,0,this->m_CustomOnlineLotteryText2,lpObj->Name);

						LogAdd(LOG_EVENT,"[OnlineLoterry] Winner (%s)",lpObj->Name);

						int Coin1 = this->m_CustomOnlineLotteryInfo[lpInfo->Index].Reward.Coin1;

						int Coin2 = this->m_CustomOnlineLotteryInfo[lpInfo->Index].Reward.Coin2;

						int Coin3 = this->m_CustomOnlineLotteryInfo[lpInfo->Index].Reward.Coin3;

						if (Coin1 > 0 || Coin2 > 0 || Coin3 > 0)
						{	
							GDSetCoinSend(lpObj->Index, Coin1, Coin2, Coin3, "Lottery");
						}
					}
					else
					{
						gNotice.GCNoticeSendToAll(0,0,0,0,0,0,this->m_CustomOnlineLotteryText3,lpInfo->RuleInfo.Name);
						LogAdd(LOG_EVENT,"[OnlineLoterry] No Winner");
					}


		this->SetState(lpInfo,CUSTOM_ONLINE_LOTTERY_STATE_EMPTY);
	}
}

void CCustomOnlineLottery::SetState(CUSTOM_ONLINE_LOTTERY_INFO* lpInfo,int state) // OK
{
	switch((lpInfo->State=state))
	{
		case CUSTOM_ONLINE_LOTTERY_STATE_BLANK:
			this->SetState_BLANK(lpInfo);
			break;
		case CUSTOM_ONLINE_LOTTERY_STATE_EMPTY:
			this->SetState_EMPTY(lpInfo);
			break;
		case CUSTOM_ONLINE_LOTTERY_STATE_START:
			this->SetState_START(lpInfo);
			break;
	}
}

void CCustomOnlineLottery::SetState_BLANK(CUSTOM_ONLINE_LOTTERY_INFO* lpInfo) // OK
{

}

void CCustomOnlineLottery::SetState_EMPTY(CUSTOM_ONLINE_LOTTERY_INFO* lpInfo) // OK
{
	lpInfo->AlarmMinSave = -1;
	lpInfo->AlarmMinLeft = -1;

	this->CheckSync(lpInfo);
}

void CCustomOnlineLottery::SetState_START(CUSTOM_ONLINE_LOTTERY_INFO* lpInfo) // OK
{
	lpInfo->AlarmMinSave = -1;
	lpInfo->AlarmMinLeft = -1;

	lpInfo->RemainTime = lpInfo->RuleInfo.EventTime*60;

	lpInfo->TargetTime = (int)(time(0)+lpInfo->RemainTime);
}

void CCustomOnlineLottery::CheckSync(CUSTOM_ONLINE_LOTTERY_INFO* lpInfo) // OK
{
	if(lpInfo->StartTime.empty() != 0)
	{
		this->SetState(lpInfo,CUSTOM_ONLINE_LOTTERY_STATE_BLANK);
		return;
	}

	CTime ScheduleTime;

	CScheduleManager ScheduleManager;

	for(std::vector<CUSTOM_ONLINE_LOTTERY_START_TIME>::iterator it=lpInfo->StartTime.begin();it != lpInfo->StartTime.end();it++)
	{
		ScheduleManager.AddSchedule(it->Year,it->Month,it->Day,it->Hour,it->Minute,it->Second,it->DayOfWeek);
	}

	if(ScheduleManager.GetSchedule(&ScheduleTime) == 0)
	{
		this->SetState(lpInfo,CUSTOM_ONLINE_LOTTERY_STATE_BLANK);
		return;
	}

	lpInfo->RemainTime = (int)difftime(ScheduleTime.GetTime(),time(0));

	lpInfo->TargetTime = (int)ScheduleTime.GetTime();
}

int CCustomOnlineLottery::GetIndexSorted(int Map)
{
	std::vector< int > arr;

	for(int n=OBJECT_START_USER;n < MAX_OBJECT;n++)
	{
		if(gObjIsConnectedGP(n) != 0 && gObj[n].Type == OBJECT_USER)
		{
			if (gObj[n].Map == Map || Map < 0)
			{
				if (this->m_CustomOnlineLotteryNoOff == 1)
				{
					if (gObj[n].PShopCustomOffline == 0 && gObj[n].AttackCustomOffline == 0)
					{
						arr.push_back(n);
						LogAdd(LOG_EVENT,"[OnlineLoterry] Add Member (%s)",gObj[n].Name);
					}
				}
				else
				{
					arr.push_back(n);
					LogAdd(LOG_EVENT,"[OnlineLoterry] Add Member (%s)",gObj[n].Name);
				}
			}			
		}
	}

	if (arr.size() <= 0)
	{
		return 0;
	}

	int randomIndex = rand() % arr.size();

	if (arr[randomIndex] > 0)
	{
		return arr[randomIndex];
	}
	
	return 0;
}

void CCustomOnlineLottery::StartNow(int map) // OK
{
	int Map = map;

					int index = this->GetIndexSorted(Map);

					if (index > 0 )
					{
						
						LPOBJ lpObj = &gObj[index];

						gItemBagManager.DropItemBySpecialValue(ITEM_BAG_LOTTERY,lpObj,lpObj->Map,lpObj->X,lpObj->Y);

						GCFireworksSend(lpObj,lpObj->X,lpObj->Y);

						gNotice.GCNoticeSendToAll(0,0,0,0,0,0,this->m_CustomOnlineLotteryText2,lpObj->Name);

						LogAdd(LOG_EVENT,"[OnlineLoterry] Winner (%s)",lpObj->Name);

						int Coin1 = this->m_CustomOnlineLotteryInfo[0].Reward.Coin1;

						int Coin2 = this->m_CustomOnlineLotteryInfo[0].Reward.Coin2;

						int Coin3 = this->m_CustomOnlineLotteryInfo[0].Reward.Coin3;

						if (Coin1 > 0 || Coin2 > 0 || Coin3 > 0)
						{	
							GDSetCoinSend(lpObj->Index, Coin1, Coin2, Coin3, "Lottery");
						}
					}
					else
					{
						gNotice.GCNoticeSendToAll(0,0,0,0,0,0,this->m_CustomOnlineLotteryText3,"Sorteio Online");
						LogAdd(LOG_EVENT,"[OnlineLoterry] No Winner");
					}

}
