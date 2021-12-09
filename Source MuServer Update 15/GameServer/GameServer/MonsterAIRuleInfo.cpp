// MonsterAIRuleInfo.cpp: implementation of the CMonsterAIRuleInfo class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MonsterAIRuleInfo.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMonsterAIRuleInfo::CMonsterAIRuleInfo() // OK
{
	this->Reset();
}

CMonsterAIRuleInfo::~CMonsterAIRuleInfo() // OK
{

}

void CMonsterAIRuleInfo::Reset() // OK
{
	this->m_RuleNumber = -1;
	this->m_MonsterClass = -1;
	this->m_MonsterAIUnit = 0;
	this->m_RuleCondition = -1;
	this->m_WaitTime = -1;
	this->m_ContinuanceTime = -1;
	this->m_Month = -1;
	this->m_Day = -1;
	this->m_WeekDay = -1;
	this->m_Hour = -1;
	this->m_Minute = -1;
	this->m_AppliedTime = -1;
	this->m_ApplyingTime = -1;

	memset(this->m_RuleDesc,0,sizeof(this->m_RuleDesc));
}

BOOL CMonsterAIRuleInfo::IsValid() // OK
{
	BOOL IsValid = 0;

	switch(this->m_RuleCondition)
	{
		case 1:
			IsValid = this->CheckConditionSpecificDate();
			break;
		case 71:
			IsValid = this->CheckConditionCrywolfStart();
			break;
		case 72:
			IsValid = this->CheckConditionCrywolfEnd();
			break;
	}

	if(IsValid == 0)
	{
		this->m_AppliedTime = -1;
		this->m_ApplyingTime = -1;
		return 0;
	}

	if(this->m_AppliedTime == -1)
	{
		this->m_AppliedTime = GetTickCount();
	}

	if((DWORD)(this->m_AppliedTime+(this->m_WaitTime*1000)) > GetTickCount())
	{
		return 0;
	}

	if((DWORD)((this->m_AppliedTime+(this->m_WaitTime*1000))+(this->m_ContinuanceTime*1000)) < GetTickCount())
	{
		return 0;
	}

	return 1;
}

BOOL CMonsterAIRuleInfo::CheckConditionSpecificDate() // OK
{
	CTime CurrentTime = CTime::GetTickCount();

	if(this->m_WeekDay != -1 && this->m_WeekDay != CurrentTime.GetDayOfWeek())
	{
		return 0;
	}

	int Year = CurrentTime.GetYear();
	int Month = CurrentTime.GetMonth();
	int Day = CurrentTime.GetDay();
	int Hour = CurrentTime.GetHour();
	int Minute = CurrentTime.GetMinute();

	CTimeSpan ValidRange(0,0,0,(this->m_WaitTime+this->m_ContinuanceTime));

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

BOOL CMonsterAIRuleInfo::CheckConditionCrywolfStart() // OK
{
	return 0;
}

BOOL CMonsterAIRuleInfo::CheckConditionCrywolfEnd() // OK
{
	return 0;
}
