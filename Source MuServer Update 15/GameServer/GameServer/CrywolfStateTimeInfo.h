// CrywolfStateTimeInfo.h: interface for the CCrywolfStateTimeInfo class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

class CCrywolfStateTimeInfo
{
public:
	CCrywolfStateTimeInfo();
	bool CheckScheduleTime();
	bool CheckContinuanceTime();
	void Reset();
	void ResetAppliedTime();
	void SetAppliedTime();
	int GetLeftTime();
public:
	int m_Used;
	int m_StateNumber;
	int m_Year;
	int m_Month;
	int m_Day;
	int m_DayOfWeek;
	int m_Hour;
	int m_Minute;
	int m_Second;
	int m_ContinuanceTime;
	DWORD m_AppliedTickCount;
	CTime TesteTime2;
};
