#pragma once

#include "User.h"

#define MAX_GVGEVENT_GUILD 20
#define MAX_GVGEVENT_USER  100

enum eGvGEventState
{
	GVG_EVENT_STATE_BLANK = 0,
	GVG_EVENT_STATE_EMPTY = 1,
	GVG_EVENT_STATE_STAND = 2,
	GVG_EVENT_STATE_START = 3,
	GVG_EVENT_STATE_CLEAN = 4,
};

struct GVG_EVENT_START_TIME
{
	int Year;
	int Month;
	int Day;
	int DayOfWeek;
	int Hour;
	int Minute;
	int Second;
};

struct GVG_EVENT_GUILD
{
	void Reset() // OK
	{
		this->Guild = -1;
		this->Gate = -1;
		this->Point = 0;
		this->Rank = 0;
	}

	int Guild;
	int Gate;
	int Point;
	int Rank;
};

struct GVG_EVENT_USER
{
	void Reset() // OK
	{
		this->Index = -1;
		this->Guild = -1;
		this->Gate = -1;
	}

	int Index;
	int Guild;
	int Gate;
};

struct GVG_TIME
{
	int Year;
	int Month;
	int Day;
	int DayOfWeek;
	int Hour;
	int Minute;
	int Second;
};


class CGvGEvent
{
public:
	CGvGEvent();
	virtual ~CGvGEvent();
	void Init();
	void Clear();
	void Load(char * path);
	void MainProc();
	void ProcState_BLANK();
	void ProcState_EMPTY();
	void ProcState_STAND();
	void ProcState_START();
	void ProcState_CLEAN();
	void SetState(int state);
	void SetState_BLANK();
	void SetState_EMPTY();
	void SetState_STAND();
	void SetState_START();
	void SetState_CLEAN();
	void CheckSync();
	int GetState();
	bool Dialog(LPOBJ lpObj, LPOBJ lpNpc);
	bool CheckEnterEnabled(LPOBJ lpObj);
	bool CheckReqItems(LPOBJ lpObj);
	int GetEnterEnabled();
	int GetEnteredUserCount();
	bool CheckEnteredUser(int aIndex);
	bool CheckPlayerTarget(LPOBJ lpObj);
	bool CheckStandTarget(LPOBJ lpObj); 
	bool CheckPlayerJoined(LPOBJ lpObj,LPOBJ lpTarget);
	bool CheckSelfTeam(LPOBJ lpObj,LPOBJ lpTarget);
	bool CheckAttack(LPOBJ lpObj,LPOBJ lpTarget);
	bool CheckGuildMaster(int index);
	bool AddUser(int aIndex);
	bool AddGuild(int index);
	bool DelUser(int aIndex);
	GVG_EVENT_USER* GetUser(int aIndex);
	GVG_EVENT_GUILD* GetGuild(int aIndex);
	void CleanUser();
	void ClearUser();
	void CleanGuild();
	void ClearGuild();
	void CheckUser();
	int GetUserCount();
	int GetGuildCount();
	bool GetUserRespawnLocation(LPOBJ lpObj,int* gate,int* map,int* x,int* y,int* dir,int* level);
	void UserDieProc(LPOBJ lpObj,LPOBJ lpTarget);
	void NoticeSendToAll(int type,char* message,...);
	void CalcRank();
	void StartGvG();
private:
	int m_State;
	int m_RemainTime;
	int m_StandTime;
	int m_CloseTime;
	int m_TickCount;
	int m_WarningTime;
	int m_EventTime;
	int EnterEnabled;
	int AlarmMinSave;
	int AlarmMinLeft;
	int TargetTime;

	int Winner;
	int TotalPlayer;

	int ReqItemCount;
	int ReqItemIndex;
	int ReqItemLevel;
	int EventMap;
	int WaitingGate;
	int StartGate;
	int MinLevel;
	int MaxLevel;
	int MinReset;
	int MaxReset;
	int MinMasterReset;
	int MaxMasterReset;

	int Coin1;
	int Coin2;
	int Coin3;

	GVG_EVENT_GUILD Guild[MAX_GVGEVENT_GUILD];
	GVG_EVENT_USER User[MAX_GVGEVENT_USER];
	std::vector<GVG_TIME> m_GVGStartTime;
}; 
extern CGvGEvent gGvGEvent;