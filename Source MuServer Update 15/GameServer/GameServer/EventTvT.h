#pragma once

#include "User.h"

#define MAX_TVTEVENT_USER 40

enum eTvTEventState
{
	TVT_EVENT_STATE_BLANK = 0,
	TVT_EVENT_STATE_EMPTY = 1,
	TVT_EVENT_STATE_STAND = 2,
	TVT_EVENT_STATE_START = 3,
	TVT_EVENT_STATE_CLEAN = 4,
};

enum eTvTTeams
{
	TVT_BLUETEAM = 1,
	TVT_REDTEAM = 2,
};

struct TVT_EVENT_START_TIME
{
	int Year;
	int Month;
	int Day;
	int DayOfWeek;
	int Hour;
	int Minute;
	int Second;
};

struct TVT_EVENT_USER
{
	void Reset() // OK
	{
		this->Index = -1;
		this->Available = false;
		this->Team = -1;
		this->Kills = 0;
		this->Deaths = 0;
	}

	int Index;
	bool Available;
	int Team;
	int Kills;
	int Deaths;
};

struct TVT_TIME
{
	int Year;
	int Month;
	int Day;
	int DayOfWeek;
	int Hour;
	int Minute;
	int Second;
};


class CTvTEvent
{
public:
	CTvTEvent();
	virtual ~CTvTEvent();
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
	bool CheckPlayerJoined(LPOBJ lpObj,LPOBJ lpTarget);
	bool CheckSelfTeam(LPOBJ lpObj,LPOBJ lpTarget);
	bool CheckAttack(LPOBJ lpObj,LPOBJ lpTarget);
	bool AddUser(int aIndex);
	bool DelUser(int aIndex);
	TVT_EVENT_USER* GetUser(int aIndex);
	void CleanUser();
	void ClearUser();
	void CheckUser();
	int GetUserCount();
	bool GetUserRespawnLocation(LPOBJ lpObj,int* gate,int* map,int* x,int* y,int* dir,int* level);
	void DivisionTeam();
	int ReturnMaxReset();
	void UserDieProc(LPOBJ lpObj,LPOBJ lpTarget);
	void NoticeSendToAll(int type,char* message,...);
	void StartTvT();
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

	int TotalPlayer;
	int ScoreBlue;
	int ScoreRed;

	int ReqItemCount;
	int ReqItemIndex;
	int ReqItemLevel;
	int EventMap;
	int WaitingGate;
	int BluTeamGate;
	int RedTeamGate;
	int SetSkin;
	int MinLevel;
	int MaxLevel;
	int MinReset;
	int MaxReset;
	int MinMasterReset;
	int MaxMasterReset;

	int Coin1;
	int Coin2;
	int Coin3;

	TVT_EVENT_USER User[MAX_TVTEVENT_USER];
	std::vector<TVT_TIME> m_TVTStartTime;
}; 
extern CTvTEvent gTvTEvent;