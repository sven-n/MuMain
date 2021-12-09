// CrywolfStateTimeInfo.cpp: implementation of the CCrywolfStateTimeInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CrywolfStateTimeInfo.h"
#include "ServerInfo.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCrywolfStateTimeInfo::CCrywolfStateTimeInfo() // OK
{
	this->Reset();
}

bool CCrywolfStateTimeInfo::CheckScheduleTime() // OK
{
	CTime CurrentTime = CTime::GetTickCount();

	if(this->m_DayOfWeek != -1 && this->m_DayOfWeek != CurrentTime.GetDayOfWeek())
	{
		return 0;
	}

	int Year = CurrentTime.GetYear();
	int Month = CurrentTime.GetMonth();
	int Day = CurrentTime.GetDay();
	int Hour = CurrentTime.GetHour();
	int Minute = CurrentTime.GetMinute();

	CTimeSpan ValidRange(0,0,0,this->m_ContinuanceTime);

	if(this->m_Month != -1 && this->m_Month != Month)
	{
		CTime ConditionTime(Year,this->m_Month,0,0,0,0,-1);

		if(CurrentTime < ConditionTime || CurrentTime > (ConditionTime+ValidRange))
		{
			return 0;
		}
	}

	if(this->m_Day != -1 && this->m_Day != Day)
	{
		CTime ConditionTime(Year,((this->m_Month==-1)?Month:this->m_Month),this->m_Day,0,0,0,-1);

		if(CurrentTime < ConditionTime || CurrentTime > (ConditionTime+ValidRange))
		{
			return 0;
		}
	}

	if(this->m_Hour != -1 && this->m_Hour != Hour)
	{
		CTime ConditionTime(Year,((this->m_Month==-1)?Month:this->m_Month),((this->m_Day==-1)?Day:this->m_Day),this->m_Hour,0,0,-1);

		if(CurrentTime < ConditionTime || CurrentTime > (ConditionTime+ValidRange))
		{
			return 0;
		}
	}

	if(this->m_Minute != -1 && this->m_Minute != Minute)
	{
		CTime ConditionTime(Year,((this->m_Month==-1)?Month:this->m_Month),((this->m_Day==-1)?Day:this->m_Day),((this->m_Hour==-1)?Hour:this->m_Hour),this->m_Minute,0,-1);

		if(CurrentTime < ConditionTime || CurrentTime > (ConditionTime+ValidRange))
		{
			return 0;
		}
	}

	return 1;
}

bool CCrywolfStateTimeInfo::CheckContinuanceTime() // OK
{
	if((GetTickCount()-this->m_AppliedTickCount) > ((DWORD)this->m_ContinuanceTime))
	{
		return 0;
	}

	return 1;
}

void CCrywolfStateTimeInfo::Reset() // OK
{
	this->m_Used = 0;
	this->m_StateNumber = -1;
	this->m_Year = -1;
	this->m_Month = -1;
	this->m_Day = -1;
	this->m_DayOfWeek = -1;
	this->m_Hour = -1;
	this->m_Minute = -1;
	this->m_Second = -1;
	this->m_ContinuanceTime = 0;
	this->m_AppliedTickCount = 0;
}

void CCrywolfStateTimeInfo::ResetAppliedTime() // OK
{
	this->m_AppliedTickCount = 0;
}

void CCrywolfStateTimeInfo::SetAppliedTime() // OK
{
	this->m_AppliedTickCount = GetTickCount();
}

int CCrywolfStateTimeInfo::GetLeftTime() // OK
{
	if(this->CheckContinuanceTime() == 0)
	{
		return 0;
	}

	int LeftTime = this->m_ContinuanceTime-(GetTickCount()-this->m_AppliedTickCount);

	LeftTime = ((LeftTime<0)?0:LeftTime);

	return LeftTime;
}
