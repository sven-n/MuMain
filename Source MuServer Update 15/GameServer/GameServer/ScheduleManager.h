// ScheduleManager.h: interface for the CScheduleManager class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#define ADD_MONTH(month,add_month) ((((month)+(add_month))>12)?((add_month)-(12-(month))):((month)+(add_month)))
#define ADD_YEAR(year,add_year,month,add_month) ((((month)+(add_month))>12)?((year)+((add_year)+1)):((year)+(add_year)))

class CScheduleManager
{
public:
	CScheduleManager();
	virtual ~CScheduleManager();
	bool GetSchedule(CTime* lpScheduleTime);
	bool AddSchedule(int Year,int Month,int Day,int Hour,int Minute,int Second,int DayOfWeek);
	bool AdjustSchedule(CTime* lpScheduleTime,bool IsYear,bool IsMonth,bool IsDay,bool IsHour,bool IsMinute,bool IsSecond);
	bool AdjustScheduleDayOfWeek(CTime* lpScheduleTime,bool IsYear,bool IsMonth,bool IsDay,bool IsHour,bool IsMinute,bool IsSecond,int DayOfWeek);
private:
	CTime m_StartTime;
	std::vector<CTime> m_ScheduleTimeInfo;
};
