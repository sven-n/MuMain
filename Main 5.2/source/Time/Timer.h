// Timer.h: interface for the CTimer class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TIMER_H__DF94D50D_ED17_4DF1_9BD1_FA13662C8B74__INCLUDED_)
#define AFX_TIMER_H__DF94D50D_ED17_4DF1_9BD1_FA13662C8B74__INCLUDED_

#pragma once

#include <windows.h>

class CTimer  
{
	BOOL	m_bUsePerformanceCounter;	// use performance counter or multimedia timer?
	DWORD	m_mmTimerStart;		// Multimedia timer variables for start times
	DWORD	m_mmAbsTimerStart;	// 최초로 MM Timer가 생성된 시간
	float	m_resolution;		// Timer resolution for performance counter timer
	__int64	m_pcTimerStart;		// Performance counter timer variables for start times
	__int64	m_pcAbsTimerStart;	// 최초로 PC Timer가 생성된 시간
	__int64	m_frequency;		// Time frequency for performance counter timer

public:
	CTimer();
	virtual ~CTimer();

	double GetAbsTime();
	void ResetTimer();
	double GetTimeElapsed();
};

class CTimer2
{
	DWORD m_dwStartTickCount, m_dwDelay;
	bool m_bTime;

public:
	CTimer2() : m_dwStartTickCount(0), m_dwDelay(0), m_bTime(false) {}
	~CTimer2() {}

	void SetTimer(DWORD dwDelay)
	{
		m_dwDelay = dwDelay;
		m_dwStartTickCount = 0;
	}
	DWORD GetDelay() const { return m_dwDelay; }
	void ResetTimer()
	{
		m_dwStartTickCount = 0;
	}
	void UpdateTime()
	{
		if(m_dwDelay == 0)
			m_bTime = true;
		else
		{
			m_bTime = false;
			if(m_dwStartTickCount == 0)
			{
				m_dwStartTickCount = ::GetTickCount();
				return;
			}
			if(::GetTickCount() - m_dwStartTickCount > m_dwDelay)
			{
				m_dwStartTickCount = ::GetTickCount();
				m_bTime = true;
			}
		}
	}

	bool IsTime() const { return m_bTime; }
};

#endif // !defined(AFX_TIMER_H__DF94D50D_ED17_4DF1_9BD1_FA13662C8B74__INCLUDED_)
