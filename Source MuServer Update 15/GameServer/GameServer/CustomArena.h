// CustomArena.h: interface for the CCustomArena class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "DefaultClassInfo.h"
#include "User.h"

#define MAX_CUSTOM_ARENA 30
#define MAX_CUSTOM_ARENA_USER 100

enum eCustomArenaState
{
	CUSTOM_ARENA_STATE_BLANK = 0,
	CUSTOM_ARENA_STATE_EMPTY = 1,
	CUSTOM_ARENA_STATE_STAND = 2,
	CUSTOM_ARENA_STATE_START = 3,
	CUSTOM_ARENA_STATE_CLEAN = 4,
};

struct CUSTOM_ARENA_START_TIME
{
	int Year;
	int Month;
	int Day;
	int DayOfWeek;
	int Hour;
	int Minute;
	int Second;
};

struct CUSTOM_ARENA_USER
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

struct CUSTOM_ARENA_RULE_INFO
{
	char Name[32];
	int AlarmTime;
	int StandTime;
	int EventTime;
	int CloseTime;
	int StartGate;
	int FinalGate;
	int MinUser;
	int MaxUser;
	int MaxDeath;
	int MinLevel;
	int MaxLevel;
	int MinMasterLevel;
	int MaxMasterLevel;
	int MinReset;
	int MaxReset;
	int MinMasterReset;
	int MaxMasterReset;
	int RequireClass[MAX_CLASS];
};

struct CUSTOM_ARENA_INFO
{
	int Index;
	int State;
	int RemainTime;
	int TargetTime;
	int TickCount;
	int EnterEnabled;
	int AlarmMinSave;
	int AlarmMinLeft;
	CUSTOM_ARENA_RULE_INFO RuleInfo;
	CUSTOM_ARENA_USER User[MAX_CUSTOM_ARENA_USER];
	std::vector<CUSTOM_ARENA_START_TIME> StartTime;
};

class CCustomArena
{
public:
	CCustomArena();
	virtual ~CCustomArena();
	void Init();
	void ReadCustomArenaInfo(char* section,char* path);
	void Load(char* path);
	void MainProc();
	void ProcState_BLANK(CUSTOM_ARENA_INFO* lpInfo);
	void ProcState_EMPTY(CUSTOM_ARENA_INFO* lpInfo);
	void ProcState_STAND(CUSTOM_ARENA_INFO* lpInfo);
	void ProcState_START(CUSTOM_ARENA_INFO* lpInfo);
	void ProcState_CLEAN(CUSTOM_ARENA_INFO* lpInfo);
	void SetState(CUSTOM_ARENA_INFO* lpInfo,int state);
	void SetState_BLANK(CUSTOM_ARENA_INFO* lpInfo);
	void SetState_EMPTY(CUSTOM_ARENA_INFO* lpInfo);
	void SetState_STAND(CUSTOM_ARENA_INFO* lpInfo);
	void SetState_START(CUSTOM_ARENA_INFO* lpInfo);
	void SetState_CLEAN(CUSTOM_ARENA_INFO* lpInfo);
	void CheckSync(CUSTOM_ARENA_INFO* lpInfo);
	int GetState(int index);
	int GetRemainTime(int index);
	int GetEnterEnabled(int index);
	int GetEnteredUserCount(int index);
	bool CheckEnteredUser(int index,int aIndex);
	bool CheckEnterEnabled(LPOBJ lpObj,int gate);
	bool CheckPlayerJoined(LPOBJ lpObj,LPOBJ lpTarget);
	bool CheckPlayerTarget(LPOBJ lpObj,LPOBJ lpTarget);
	bool AddUser(CUSTOM_ARENA_INFO* lpInfo,int aIndex);
	bool DelUser(CUSTOM_ARENA_INFO* lpInfo,int aIndex);
	CUSTOM_ARENA_USER* GetUser(CUSTOM_ARENA_INFO* lpInfo,int aIndex);
	void CleanUser(CUSTOM_ARENA_INFO* lpInfo);
	void ClearUser(CUSTOM_ARENA_INFO* lpInfo);
	void CheckUser(CUSTOM_ARENA_INFO* lpInfo);
	int GetUserCount(CUSTOM_ARENA_INFO* lpInfo);
	void CalcUserRank(CUSTOM_ARENA_INFO* lpInfo);
	bool GetUserRespawnLocation(LPOBJ lpObj,int* gate,int* map,int* x,int* y,int* dir,int* level);
	void UserDieProc(LPOBJ lpObj,LPOBJ lpTarget);
	void NoticeSendToAll(CUSTOM_ARENA_INFO* lpInfo,int type,char* message,...);
	void DataSendToAll(CUSTOM_ARENA_INFO* lpInfo,BYTE* lpMsg,int size);
	void StartCustomArena(int index);
	char* GetArenaName(int index);
private:
	int m_CustomArenaSwitch;
	int m_CustomArenaVictimScoreDecrease;
	int m_CustomArenaKillerScoreIncrease;
	char m_CustomArenaText1[128];
	char m_CustomArenaText2[128];
	char m_CustomArenaText3[128];
	char m_CustomArenaText4[128];
	char m_CustomArenaText5[128];
	char m_CustomArenaText6[128];
	char m_CustomArenaText7[128];
	char m_CustomArenaText8[128];
	char m_CustomArenaText9[128];
	char m_CustomArenaText10[128];
	char m_CustomArenaText11[128];
	char m_CustomArenaText12[128];
	char m_CustomArenaText13[128];
	char m_CustomArenaText14[128];
	char m_CustomArenaText15[128];
	char m_CustomArenaText16[128];
	char m_CustomArenaText17[128];
	char m_CustomArenaText18[128];
	char m_CustomArenaText19[128];
	CUSTOM_ARENA_INFO m_CustomArenaInfo[MAX_CUSTOM_ARENA];
};

extern CCustomArena gCustomArena;
