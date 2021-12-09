// BonusManager.cpp: implementation of the CBonusManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BonusManager.h"
#include "DSProtocol.h"
#include "ItemDrop.h"
#include "ItemManager.h"
#include "ItemOptionRate.h"
#include "MemScript.h"
#include "Message.h"
#include "Monster.h"
#include "Notice.h"
#include "ObjectManager.h"
#include "ScheduleManager.h"
#include "ServerInfo.h"
#include "Util.h"

CBonusManager gBonusManager;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBonusManager::CBonusManager() // OK
{
	for(int n=0;n < MAX_BONUS;n++)
	{
		BONUS_INFO* lpInfo = &this->m_BonusInfo[n];

		lpInfo->Index = n;
		lpInfo->State = BONUS_STATE_BLANK;
		lpInfo->RemainTime = 0;
		lpInfo->TargetTime = 0;
		lpInfo->TickCount = GetTickCount();

		lpInfo->StartTime.clear();

		lpInfo->ValueInfo.clear();
	}
}

CBonusManager::~CBonusManager() // OK
{

}

void CBonusManager::Init() // OK
{
	for(int n=0;n < MAX_BONUS;n++)
	{
		if(gServerInfo.m_BonusManagerSwitch == 0)
		{
			this->SetState(&this->m_BonusInfo[n],BONUS_STATE_BLANK);
		}
		else
		{
			this->SetState(&this->m_BonusInfo[n],BONUS_STATE_EMPTY);
		}
	}
}

void CBonusManager::Load(char* path) // OK
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

	for(int n=0;n < MAX_BONUS;n++)
	{
		this->m_BonusInfo[n].StartMessage = -1;
		this->m_BonusInfo[n].FinalMessage = -1;
		this->m_BonusInfo[n].BonusTime = 0;
		this->m_BonusInfo[n].StartTime.clear();
		this->m_BonusInfo[n].ValueInfo.clear();
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

					BONUS_START_TIME info;

					int index = lpMemScript->GetNumber();

					info.Year = lpMemScript->GetAsNumber();

					info.Month = lpMemScript->GetAsNumber();

					info.Day = lpMemScript->GetAsNumber();

					info.DayOfWeek = lpMemScript->GetAsNumber();

					info.Hour = lpMemScript->GetAsNumber();

					info.Minute = lpMemScript->GetAsNumber();

					info.Second = lpMemScript->GetAsNumber();

					this->m_BonusInfo[index].StartTime.push_back(info);
				}
				else if(section == 1)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					int index = lpMemScript->GetNumber();

					this->m_BonusInfo[index].StartMessage = lpMemScript->GetAsNumber();

					this->m_BonusInfo[index].FinalMessage = lpMemScript->GetAsNumber();

					this->m_BonusInfo[index].BonusTime = lpMemScript->GetAsNumber();
				}
				else if(section == 2)
				{
					if(strcmp("end",lpMemScript->GetAsString()) == 0)
					{
						break;
					}

					BONUS_VALUE_INFO info;

					int index = lpMemScript->GetNumber();

					info.BonusIndex = lpMemScript->GetAsNumber();

					info.BonusValue[0] = lpMemScript->GetAsNumber();

					info.BonusValue[1] = lpMemScript->GetAsNumber();

					info.BonusValue[2] = lpMemScript->GetAsNumber();

					info.BonusValue[3] = lpMemScript->GetAsNumber();

					info.ItemIndex = lpMemScript->GetAsNumber();

					info.ItemLevel = lpMemScript->GetAsNumber();

					info.MapNumber = lpMemScript->GetAsNumber();

					info.MonsterClass = lpMemScript->GetAsNumber();

					info.MonsterLevelMin = lpMemScript->GetAsNumber();

					info.MonsterLevelMax = lpMemScript->GetAsNumber();

					this->m_BonusInfo[index].ValueInfo.push_back(info);
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

void CBonusManager::MainProc() // OK
{
	for(int n=0;n < MAX_BONUS;n++)
	{
		BONUS_INFO* lpInfo = &this->m_BonusInfo[n];

		DWORD elapsed = GetTickCount()-lpInfo->TickCount;

		if(elapsed >= 1000)
		{
			lpInfo->TickCount = GetTickCount();

			lpInfo->RemainTime = (int)difftime(lpInfo->TargetTime,time(0));

			if(gServerInfo.m_BonusManagerSwitch == 0)
			{
				if (gServerDisplayer.EventCustomBonus != -1)
				{
					gServerDisplayer.EventCustomBonus = -1;
				}
			}
			else 
			{
				if (lpInfo->RemainTime >= 0 && (lpInfo->RemainTime < gServerDisplayer.EventCustomBonus || gServerDisplayer.EventCustomBonus <= 0))
				{
					if (lpInfo->State == BONUS_STATE_EMPTY)
					{
						gServerDisplayer.EventCustomBonus = lpInfo->RemainTime;
					}
					else 
					{
						if (gServerDisplayer.EventCustomBonus != 0)
						{
							gServerDisplayer.EventCustomBonus = 0;
						}
					}
				}
			}


			switch(lpInfo->State)
			{
				case BONUS_STATE_BLANK:
					this->ProcState_BLANK(lpInfo);
					break;
				case BONUS_STATE_EMPTY:
					this->ProcState_EMPTY(lpInfo);
					break;
				case BONUS_STATE_START:
					this->ProcState_START(lpInfo);
					break;
			}
		}
	}
}

void CBonusManager::ProcState_BLANK(BONUS_INFO* lpInfo) // OK
{

}

void CBonusManager::ProcState_EMPTY(BONUS_INFO* lpInfo) // OK
{
	if(lpInfo->RemainTime <= 0)
	{
		if(lpInfo->StartMessage != -1){gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(lpInfo->StartMessage));}
		this->SetState(lpInfo,BONUS_STATE_START);
	}
}

void CBonusManager::ProcState_START(BONUS_INFO* lpInfo) // OK
{
	if(lpInfo->RemainTime <= 0)
	{
		if(lpInfo->FinalMessage != -1){gNotice.GCNoticeSendToAll(0,0,0,0,0,0,gMessage.GetMessage(lpInfo->FinalMessage));}
		this->SetState(lpInfo,BONUS_STATE_EMPTY);
	}
}

void CBonusManager::SetState(BONUS_INFO* lpInfo,int state) // OK
{
	switch((lpInfo->State=state))
	{
		case BONUS_STATE_BLANK:
			this->SetState_BLANK(lpInfo);
			break;
		case BONUS_STATE_EMPTY:
			this->SetState_EMPTY(lpInfo);
			break;
		case BONUS_STATE_START:
			this->SetState_START(lpInfo);
			break;
	}
}

void CBonusManager::SetState_BLANK(BONUS_INFO* lpInfo) // OK
{

}

void CBonusManager::SetState_EMPTY(BONUS_INFO* lpInfo) // OK
{
	this->CheckSync(lpInfo);
}

void CBonusManager::SetState_START(BONUS_INFO* lpInfo) // OK
{
	lpInfo->RemainTime = lpInfo->BonusTime;

	lpInfo->TargetTime = (int)(time(0)+lpInfo->RemainTime);
}

void CBonusManager::CheckSync(BONUS_INFO* lpInfo) // OK
{
	if(lpInfo->StartTime.empty() != 0)
	{
		this->SetState(lpInfo,BONUS_STATE_BLANK);
		return;
	}

	CTime ScheduleTime;

	CScheduleManager ScheduleManager;

	for(std::vector<BONUS_START_TIME>::iterator it=lpInfo->StartTime.begin();it != lpInfo->StartTime.end();it++)
	{
		ScheduleManager.AddSchedule(it->Year,it->Month,it->Day,it->Hour,it->Minute,it->Second,it->DayOfWeek);
	}

	if(ScheduleManager.GetSchedule(&ScheduleTime) == 0)
	{
		this->SetState(lpInfo,BONUS_STATE_BLANK);
		return;
	}

	lpInfo->RemainTime = (int)difftime(ScheduleTime.GetTime(),time(0));

	lpInfo->TargetTime = (int)ScheduleTime.GetTime();
}

int CBonusManager::GetState(int index) // OK
{
	if(CHECK_RANGE(index,MAX_BONUS) == 0)
	{
		return BONUS_STATE_BLANK;
	}
	else
	{
		return this->m_BonusInfo[index].State;
	}
}

int CBonusManager::GetRemainTime(int index) // OK
{
	if(CHECK_RANGE(index,MAX_BONUS) == 0)
	{
		return 0;
	}

	BONUS_INFO* lpInfo = &this->m_BonusInfo[index];

	if(lpInfo->StartTime.empty() != 0)
	{
		return 0;
	}

	CTime ScheduleTime;

	CScheduleManager ScheduleManager;

	for(std::vector<BONUS_START_TIME>::iterator it=lpInfo->StartTime.begin();it != lpInfo->StartTime.end();it++)
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

int CBonusManager::GetBonusValue(LPOBJ lpObj,int BonusIndex,int BonusValue,int ItemIndex,int ItemLevel,int MonsterClass,int MonsterLevel) // OK
{
	for(int n=0;n < MAX_BONUS;n++)
	{
		if(this->m_BonusInfo[n].State == BONUS_STATE_START)
		{
			for(std::vector<BONUS_VALUE_INFO>::iterator it=this->m_BonusInfo[n].ValueInfo.begin();it != this->m_BonusInfo[n].ValueInfo.end();it++)
			{
				if(it->BonusIndex == BonusIndex && (it->ItemIndex == -1 || it->ItemIndex == ItemIndex) && (it->ItemLevel == -1 || it->ItemLevel == ItemLevel) && (it->MapNumber == -1 || it->MapNumber == lpObj->Map) && (it->MonsterClass == -1 || it->MonsterClass == MonsterClass) && (it->MonsterLevelMin == -1 || it->MonsterLevelMin <= MonsterLevel) && (it->MonsterLevelMax == -1 || it->MonsterLevelMax >= MonsterLevel))
				{
					switch(it->BonusIndex)
					{
						case BONUS_INDEX_EXPERIENCE_RATE:
							BonusValue += (BonusValue*it->BonusValue[lpObj->AccountLevel])/100;
							break;
						case BONUS_INDEX_MASTER_EXPERIENCE_RATE:
							BonusValue += (BonusValue*it->BonusValue[lpObj->AccountLevel])/100;
							break;
						case BONUS_INDEX_ITEM_DROP_RATE:
							BonusValue += (BonusValue*it->BonusValue[lpObj->AccountLevel])/100;
							break;
						case BONUS_INDEX_CMN_ITEM_DROP_RATE:
							BonusValue += ((__int64)(1000000-BonusValue)*it->BonusValue[lpObj->AccountLevel])/1000000;
							break;
						case BONUS_INDEX_EXC_ITEM_DROP_RATE:
							BonusValue += ((__int64)(1000000-BonusValue)*it->BonusValue[lpObj->AccountLevel])/1000000;
							break;
						case BONUS_INDEX_SET_ITEM_DROP_RATE:
							BonusValue += ((__int64)(1000000-BonusValue)*it->BonusValue[lpObj->AccountLevel])/1000000;
							break;
						case BONUS_INDEX_GLOBIN_POINT:
							BonusValue += it->BonusValue[lpObj->AccountLevel];
							break;
						case BONUS_INDEX_RESET_AMOUNT:
							BonusValue += it->BonusValue[lpObj->AccountLevel];
							break;
						case BONUS_INDEX_MASTER_RESET_AMOUNT:
							BonusValue += it->BonusValue[lpObj->AccountLevel];
							break;
					}
				}
			}
		}
	}

	return BonusValue;
}
