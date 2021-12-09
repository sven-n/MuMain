// CustomEventDrop.h: interface for the CCustomEventDrop class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#define MAX_CUSTOM_EVENT_DROP 10

enum eCustomEventDropState
{
	CUSTOM_EVENT_DROP_STATE_BLANK = 0,
	CUSTOM_EVENT_DROP_STATE_EMPTY = 1,
	CUSTOM_EVENT_DROP_STATE_START = 2,
};

struct CUSTOM_EVENT_DROP_START_TIME
{
	int Year;
	int Month;
	int Day;
	int DayOfWeek;
	int Hour;
	int Minute;
	int Second;
};

struct CUSTOM_EVENT_DROP_ITEM_INFO
{
	int ItemIndex;
	int ItemLevel;
	int DropCount;
	int DropDelay;
	int DropState;
};

struct CUSTOM_EVENT_DROP_RULE_INFO
{
	chr Name[32];
	int Map;
	int X;
	int Y;
	int Range;
	int AlarmTime;
	int EventTime;
	std::vector<CUSTOM_EVENT_DROP_ITEM_INFO> DropItem;
};

struct CUSTOM_EVENT_DROP_INFO
{
	int Index;
	int State;
	int RemainTime;
	int TargetTime;
	int TickCount;
	int AlarmMinSave;
	int AlarmMinLeft;
	CUSTOM_EVENT_DROP_RULE_INFO RuleInfo;
	std::vector<CUSTOM_EVENT_DROP_START_TIME> StartTime;
};

class CCustomEventDrop
{
public:
	CCustomEventDrop();
	virtual ~CCustomEventDrop();
	void Init();
	void ReadCustomEventDropInfo(char* section,char* path);
	void Load(char* path);
	void MainProc();
	void ProcState_BLANK(CUSTOM_EVENT_DROP_INFO* lpInfo);
	void ProcState_EMPTY(CUSTOM_EVENT_DROP_INFO* lpInfo);
	void ProcState_START(CUSTOM_EVENT_DROP_INFO* lpInfo);
	void SetState(CUSTOM_EVENT_DROP_INFO* lpInfo,int state);
	void SetState_BLANK(CUSTOM_EVENT_DROP_INFO* lpInfo);
	void SetState_EMPTY(CUSTOM_EVENT_DROP_INFO* lpInfo);
	void SetState_START(CUSTOM_EVENT_DROP_INFO* lpInfo);
	void CheckSync(CUSTOM_EVENT_DROP_INFO* lpInfo);
	LONG GetDummyUserIndex();
	bool GetRandomItemDropLocation(int map,int* ox,int* oy,int tx,int ty,int count);
	void GCFireworksSendToNearUser(int map,int x,int y);
	void StartDrop();
private:
	int m_CustomEventDropSwitch;
	char m_CustomEventDropText1[128];
	char m_CustomEventDropText2[128];
	char m_CustomEventDropText3[128];
	CUSTOM_EVENT_DROP_INFO m_CustomEventDropInfo[MAX_CUSTOM_EVENT_DROP];
};

extern CCustomEventDrop gCustomEventDrop;
