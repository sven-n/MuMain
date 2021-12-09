// ScheduleManager.cpp: implementation of the CScheduleManager class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ScheduleManager.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CScheduleManager::CScheduleManager() // OK
{
	this->m_StartTime = CTime::GetTickCount();

	this->m_ScheduleTimeInfo.clear();
}

CScheduleManager::~CScheduleManager() // OK
{

}

bool CScheduleManager::GetSchedule(CTime* lpScheduleTime) // OK
{
	int count = 0;

	for(std::vector<CTime>::iterator it=this->m_ScheduleTimeInfo.begin();it != this->m_ScheduleTimeInfo.end();it++)
	{
		if(count == 0 || (*it) < (*lpScheduleTime))
		{
			(*lpScheduleTime) = (*it);
			count++;
		}
	}

	return ((count==0)?0:1);
}

bool CScheduleManager::AddSchedule(int Year,int Month,int Day,int Hour,int Minute,int Second,int DayOfWeek) // OK
{
	CTime ScheduleTime(((Year==-1)?this->m_StartTime.GetYear():Year),((Month==-1)?1:Month),((Day==-1)?1:Day),((Hour==-1)?0:Hour),((Minute==-1)?0:Minute),((Second==-1)?0:Second),-1); //Correção do horário de verão no ultimo campo, Padrão: -1

	while(this->m_StartTime > ScheduleTime)
	{
		if(this->m_StartTime.GetYear() > ScheduleTime.GetYear())
		{
			if(this->AdjustSchedule(&ScheduleTime,((Year==-1)?1:0),0,0,0,0,0)==0){return 0;}
		}
		else if(this->m_StartTime.GetYear() == ScheduleTime.GetYear() && this->m_StartTime.GetMonth() > ScheduleTime.GetMonth())
		{
			if(this->AdjustSchedule(&ScheduleTime,((Year==-1)?1:0),((Month==-1)?1:0),0,0,0,0)==0){return 0;}
		}
		else if(this->m_StartTime.GetYear() == ScheduleTime.GetYear() && this->m_StartTime.GetMonth() == ScheduleTime.GetMonth() && this->m_StartTime.GetDay() > ScheduleTime.GetDay())
		{
			if(this->AdjustSchedule(&ScheduleTime,((Year==-1)?1:0),((Month==-1)?1:0),((Day==-1)?1:0),0,0,0)==0){return 0;}
		}
		else if(this->m_StartTime.GetYear() == ScheduleTime.GetYear() && this->m_StartTime.GetMonth() == ScheduleTime.GetMonth() && this->m_StartTime.GetDay() == ScheduleTime.GetDay() && this->m_StartTime.GetHour() > ScheduleTime.GetHour())
		{
			if(this->AdjustSchedule(&ScheduleTime,((Year==-1)?1:0),((Month==-1)?1:0),((Day==-1)?1:0),((Hour==-1)?1:0),0,0)==0){return 0;}
		}
		else if(this->m_StartTime.GetYear() == ScheduleTime.GetYear() && this->m_StartTime.GetMonth() == ScheduleTime.GetMonth() && this->m_StartTime.GetDay() == ScheduleTime.GetDay() && this->m_StartTime.GetHour() == ScheduleTime.GetHour() && this->m_StartTime.GetMinute() > ScheduleTime.GetMinute())
		{
			if(this->AdjustSchedule(&ScheduleTime,((Year==-1)?1:0),((Month==-1)?1:0),((Day==-1)?1:0),((Hour==-1)?1:0),((Minute==-1)?1:0),0)==0){return 0;}
		}
		else if(this->m_StartTime.GetYear() == ScheduleTime.GetYear() && this->m_StartTime.GetMonth() == ScheduleTime.GetMonth() && this->m_StartTime.GetDay() == ScheduleTime.GetDay() && this->m_StartTime.GetHour() == ScheduleTime.GetHour() && this->m_StartTime.GetMinute() == ScheduleTime.GetMinute() && this->m_StartTime.GetSecond() > ScheduleTime.GetSecond())
		{
			if(this->AdjustSchedule(&ScheduleTime,((Year==-1)?1:0),((Month==-1)?1:0),((Day==-1)?1:0),((Hour==-1)?1:0),((Minute==-1)?1:0),((Second==-1)?1:0))==0){return 0;}
		}
	}

	if(this->AdjustScheduleDayOfWeek(&ScheduleTime,((Year==-1)?1:0),((Month==-1)?1:0),((Day==-1)?1:0),0,0,0,DayOfWeek)==0){return 0;}

	this->m_ScheduleTimeInfo.push_back(ScheduleTime);

	return 1;
}

bool CScheduleManager::AdjustSchedule(CTime* lpScheduleTime,bool IsYear,bool IsMonth,bool IsDay,bool IsHour,bool IsMinute,bool IsSecond) // OK
{
	int AdjustYear = 0;
	int AdjustMonth = 0;
	int AdjustDay = 0;
	int AdjustHour = 0;
	int AdjustMinute = 0;
	int AdjustSecond = 0;

	if(IsSecond != 0)
	{
		AdjustSecond++;
	}
	else if(IsMinute != 0)
	{
		AdjustMinute++;
	}
	else if(IsHour != 0)
	{
		AdjustHour++;
	}
	else if(IsDay != 0)
	{
		AdjustDay++;
	}
	else if(IsMonth != 0)
	{
		AdjustMonth++;
	}
	else if(IsYear != 0)
	{
		AdjustYear++;
	}
	else
	{
		return 0;
	}

	CTime AdjustTime(ADD_YEAR(lpScheduleTime->GetYear(),AdjustYear,lpScheduleTime->GetMonth(),AdjustMonth),ADD_MONTH(lpScheduleTime->GetMonth(),AdjustMonth),lpScheduleTime->GetDay(),lpScheduleTime->GetHour(),lpScheduleTime->GetMinute(),lpScheduleTime->GetSecond(),-1);

	CTimeSpan AdjustTimeSpan(AdjustDay,AdjustHour,AdjustMinute,AdjustSecond);

	AdjustTime += AdjustTimeSpan;

	if(IsSecond == 0 && AdjustTime.GetSecond() != lpScheduleTime->GetSecond())
	{
		AdjustTime = CTime(lpScheduleTime->GetYear(),lpScheduleTime->GetMonth(),lpScheduleTime->GetDay(),lpScheduleTime->GetHour(),lpScheduleTime->GetMinute(),lpScheduleTime->GetSecond(),-1)+CTimeSpan(0,0,1,0);
	}

	if(IsMinute == 0 && AdjustTime.GetMinute() != lpScheduleTime->GetMinute())
	{
		AdjustTime = CTime(lpScheduleTime->GetYear(),lpScheduleTime->GetMonth(),lpScheduleTime->GetDay(),lpScheduleTime->GetHour(),lpScheduleTime->GetMinute(),AdjustTime.GetSecond(),-1)+CTimeSpan(0,1,0,0);
	}

	if(IsHour == 0 && AdjustTime.GetHour() != lpScheduleTime->GetHour())
	{
		AdjustTime = CTime(lpScheduleTime->GetYear(),lpScheduleTime->GetMonth(),lpScheduleTime->GetDay(),lpScheduleTime->GetHour(),AdjustTime.GetMinute(),AdjustTime.GetSecond(),-1)+CTimeSpan(1,0,0,0);
	}

	if(IsDay == 0 && AdjustTime.GetDay() != lpScheduleTime->GetDay())
	{
		AdjustTime = CTime(ADD_YEAR(lpScheduleTime->GetYear(),0,lpScheduleTime->GetMonth(),1),ADD_MONTH(lpScheduleTime->GetMonth(),1),lpScheduleTime->GetDay(),AdjustTime.GetHour(),AdjustTime.GetMinute(),AdjustTime.GetSecond(),-1);
	}

	if(IsMonth == 0 && AdjustTime.GetMonth() != lpScheduleTime->GetMonth())
	{
		AdjustTime = CTime(ADD_YEAR(lpScheduleTime->GetYear(),1,lpScheduleTime->GetMonth(),0),lpScheduleTime->GetMonth(),AdjustTime.GetDay(),AdjustTime.GetHour(),AdjustTime.GetMinute(),AdjustTime.GetSecond(),-1);
	}

	if(IsYear == 0 && AdjustTime.GetYear() != lpScheduleTime->GetYear())
	{
		return 0;
	}

	(*lpScheduleTime) = AdjustTime;

	return 1;
}

bool CScheduleManager::AdjustScheduleDayOfWeek(CTime* lpScheduleTime,bool IsYear,bool IsMonth,bool IsDay,bool IsHour,bool IsMinute,bool IsSecond,int DayOfWeek) // OK
{
	if(DayOfWeek != -1)
	{
		while(lpScheduleTime->GetDayOfWeek() != DayOfWeek)
		{
			if(this->AdjustSchedule(lpScheduleTime,IsYear,IsMonth,IsDay,IsHour,IsMinute,IsSecond) == 0)
			{
				return 0;
			}
		}
	}

	return 1;
}
