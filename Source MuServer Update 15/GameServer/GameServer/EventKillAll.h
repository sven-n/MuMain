// CEventKillAll.h: interface for the CEventKillAll class.
//
//////////////////////////////////////////////////////////////////////
#include "User.h"

#pragma once

#define MAX_KILLALL_USER 100

struct EVENT_KILLALL_USER
{
	void Reset() // OK
	{
		this->Index = -1;
		this->Score = 0;
		this->Rank = 0;
		this->UserKillCount = 0;
		this->UserKillStreak = 0;
		this->UserDeathCount = 0;
		this->UserDeathStreak = 0;
	}

	int Index;
	int Score;
	int Rank;
	int UserKillCount;
	int UserKillStreak;
	int UserDeathCount;
	int UserDeathStreak;
};

class CEventKillAll
{
public:
	CEventKillAll();
	virtual ~CEventKillAll();
	void MainProc();
	void Clear();
	bool CommandEventKillAll(LPOBJ lpObj,char* arg);
    void CommandEventKillAllJoin(LPOBJ lpObj,char* arg);
	int m_EventKillAllActive;
	void CalcUserRank();
	bool CheckPlayerTarget(LPOBJ lpObj,LPOBJ lpTarget);
	bool AddUser(int aIndex);
	void UserDieProc(LPOBJ lpObj,LPOBJ lpTarget);
	bool Respawn(LPOBJ lpObj,int* gate,int* map,int* x,int* y,int* dir,int* level);
	void CheckUser();
private:
	int m_RemainTime;
	int m_TickCount;
	int m_TotalTime;
	int m_Map;
	int m_MapX;
	int m_MapY;
	int m_Members;
	int m_EventKillAllEnter;
	EVENT_KILLALL_USER User[MAX_KILLALL_USER];
};

extern CEventKillAll gEventKillAll;
