// CustomQuiz.h: interface for the CCustomQuiz class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "User.h"

#define MAX_CUSTOM_QUIZ 24

enum eCustomQuizState
{
	CUSTOM_QUIZ_STATE_BLANK = 0,
	CUSTOM_QUIZ_STATE_EMPTY = 1,
	CUSTOM_QUIZ_STATE_START = 2,
};

struct CUSTOM_QUIZ_START_TIME
{
	int Year;
	int Month;
	int Day;
	int DayOfWeek;
	int Hour;
	int Minute;
	int Second;
};

struct CUSTOM_QUIZ_RULE_INFO
{
	chr Name[32];
	int AlarmTime;
	int EventTime;
};

struct CUSTOM_QUIZ_INFO
{
	int Index;
	int State;
	int RemainTime;
	int TargetTime;
	int TickCount;
	int AlarmMinSave;
	int AlarmMinLeft;
	CUSTOM_QUIZ_RULE_INFO RuleInfo;
	std::vector<CUSTOM_QUIZ_START_TIME> StartTime;
};

struct CUSTOMQUIZ_QUESTION
{
	int Index;
	int Value1;
	int Value2;
	int Value3;
	char Question[128];
	char Answer[128];
};

class CCustomQuiz
{
public:
	CCustomQuiz();
	virtual ~CCustomQuiz();
	void Init();
	void Load(char* path);
	void MainProc();
	void ProcState_BLANK(CUSTOM_QUIZ_INFO* lpInfo);
	void ProcState_EMPTY(CUSTOM_QUIZ_INFO* lpInfo);
	void ProcState_START(CUSTOM_QUIZ_INFO* lpInfo);
	void SetState(CUSTOM_QUIZ_INFO* lpInfo,int state);
	void SetState_BLANK(CUSTOM_QUIZ_INFO* lpInfo);
	void SetState_EMPTY(CUSTOM_QUIZ_INFO* lpInfo);
	void SetState_START(CUSTOM_QUIZ_INFO* lpInfo);
	void CheckSync(CUSTOM_QUIZ_INFO* lpInfo);
	void StartQuiz();
	bool GetInfo(int index,CUSTOMQUIZ_QUESTION* lpInfo);
	void CommandQuiz(LPOBJ lpObj,char* arg);
private:
	std::map<int,CUSTOMQUIZ_QUESTION> m_CustomQuiz;
	CUSTOM_QUIZ_INFO m_CustomQuizInfo[MAX_CUSTOM_QUIZ];
	int Active;
	int IndexSelected;
	int IndexInfo;

};

extern CCustomQuiz gCustomQuiz;
