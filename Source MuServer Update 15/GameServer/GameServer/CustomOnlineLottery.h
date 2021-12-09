// CustomOnlineLottery.h: interface for the CCustomOnlineLottery class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#define MAX_CUSTOM_ONLINE_LOTTERY 24

enum eCustomOnlineLotteryState
{
	CUSTOM_ONLINE_LOTTERY_STATE_BLANK = 0,
	CUSTOM_ONLINE_LOTTERY_STATE_EMPTY = 1,
	CUSTOM_ONLINE_LOTTERY_STATE_START = 2,
};

struct CUSTOM_ONLINE_LOTTERY_START_TIME
{
	int Year;
	int Month;
	int Day;
	int DayOfWeek;
	int Hour;
	int Minute;
	int Second;
};

struct CUSTOM_ONLINE_LOTTERY_ITEM_INFO
{
	int Index;
	int MinLevel;
	int MaxLevel;
	int Option1;
	int Option2;
	int Option3;
	int NewOption;
	int SetOption;
	int SocketOption;
};

struct CUSTOM_ONLINE_LOTTERY_RULE_INFO
{
	chr Name[32];
	int Map;
	int AlarmTime;
	int EventTime;
	std::vector<CUSTOM_ONLINE_LOTTERY_ITEM_INFO> DropItem;
};

struct CUSTOM_ONLINE_LOTTERY_REWARD
{
	int Coin1;
	int Coin2;
	int Coin3;
};

struct CUSTOM_ONLINE_LOTTERY_INFO
{
	int Index;
	int State;
	int RemainTime;
	int TargetTime;
	int TickCount;
	int AlarmMinSave;
	int AlarmMinLeft;
	CUSTOM_ONLINE_LOTTERY_RULE_INFO RuleInfo;
	std::vector<CUSTOM_ONLINE_LOTTERY_START_TIME> StartTime;
	CUSTOM_ONLINE_LOTTERY_REWARD Reward;
};

class CCustomOnlineLottery
{
public:
	CCustomOnlineLottery();
	virtual ~CCustomOnlineLottery();
	void Init();
	void ReadCustomOnlineLotteryInfo(char* section,char* path);
	void Load(char* path);
	void MainProc();
	void ProcState_BLANK(CUSTOM_ONLINE_LOTTERY_INFO* lpInfo);
	void ProcState_EMPTY(CUSTOM_ONLINE_LOTTERY_INFO* lpInfo);
	void ProcState_START(CUSTOM_ONLINE_LOTTERY_INFO* lpInfo);
	void SetState(CUSTOM_ONLINE_LOTTERY_INFO* lpInfo,int state);
	void SetState_BLANK(CUSTOM_ONLINE_LOTTERY_INFO* lpInfo);
	void SetState_EMPTY(CUSTOM_ONLINE_LOTTERY_INFO* lpInfo);
	void SetState_START(CUSTOM_ONLINE_LOTTERY_INFO* lpInfo);
	void CheckSync(CUSTOM_ONLINE_LOTTERY_INFO* lpInfo);
	int GetIndexSorted(int Map);
	void StartNow(int map);
	CUSTOM_ONLINE_LOTTERY_ITEM_INFO* GetInfo(int index);
private:
	int m_CustomOnlineLotterySwitch;
	char m_CustomOnlineLotteryText1[128];
	char m_CustomOnlineLotteryText2[128];
	char m_CustomOnlineLotteryText3[128];
	int m_CustomOnlineLotteryNoOff;
	CUSTOM_ONLINE_LOTTERY_INFO m_CustomOnlineLotteryInfo[MAX_CUSTOM_ONLINE_LOTTERY];

};

extern CCustomOnlineLottery gCustomOnlineLottery;
