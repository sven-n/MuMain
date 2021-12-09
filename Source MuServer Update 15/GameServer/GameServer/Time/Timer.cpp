// Timer.cpp: implementation of the CTimer class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "Timer.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//*****************************************************************************
// constructor
// 어떤 timer를 이용할 것인지 check하고 start time울 저장한다
//*****************************************************************************
CTimer::CTimer()
{
	if (::QueryPerformanceFrequency((LARGE_INTEGER*)&m_frequency) == FALSE)
	{
		// performance counter 사용 불가능.
		m_bUsePerformanceCounter = FALSE;

		// Windows NT/2000인 경우, 다음과 같이 함으로써 timeGetTime의 precision
		//을 변경할 수 있다
		TIMECAPS Caps;
		::timeGetDevCaps(&Caps, sizeof(Caps)); // resolution를 결정하기 위해서
		//timer device의 Caps를 묻는다.
		
		if (::timeBeginPeriod(Caps.wPeriodMin) == TIMERR_NOCANDO)
		{
			// 사실 이 error는 나도 상관 없다
			// 만약 이런 error가 난다면 default timeGetTime()이 쓰이게 될것!
#ifdef __TIMER_DEBUG
			__TraceF(TEXT("timeBeginPeriod(...) Error\n"));
			//CDebug::OutputDebugString("timeBeginPeriod(...) Error");
#endif //__TIMER_DEBUG 
		}

		// multimedia timer로 현재 시간을 얻는다
		m_mmAbsTimerStart = m_mmTimerStart = ::timeGetTime();
	}
	else
	{
		// Performance Counter 사용가능.
		m_bUsePerformanceCounter = TRUE;

		// Performance Counter를 이용해 현재 시간을 얻는다 
		::QueryPerformanceCounter((LARGE_INTEGER*)&m_pcTimerStart);

		// 절대 시간을 저장한다
		m_pcAbsTimerStart = m_pcTimerStart;
		
		// QueryPerformanceFrequency로 얻는 frequency로부터 timer resolution을
		//계산한다.
		// timer resolution(주기)이란, 가능한 최소 시간 간격을 말한다.
		// frequency의 역수는 주기이므로 역수를 취해서 구한다.
		//1000.0을 곱하는 것은 sec unit을 millisecond unit으로 바꾸기 위해서이다
		m_resolution = (float)(1.0 / (double)m_frequency) * 1000.0f;
	}
}

CTimer::~CTimer()
{
	if (!m_bUsePerformanceCounter) // timeGetTime을 이용하는 경우
	{
		// multimedia timer를 닫는다
		TIMECAPS Caps;
		::timeGetDevCaps(&Caps, sizeof(Caps));
		// 이전의 mimimum timer resoltion을 clear한다
		::timeEndPeriod(Caps.wPeriodMin);
	}
}

//*****************************************************************************
// ResetTimer()를 부를 이후로 흐른 시간을 얻는다
//*****************************************************************************
double CTimer::GetTimeElapsed()
{
	__int64 timeElapsed;

	if (m_bUsePerformanceCounter)	// if using Performance Counter
	{
		// 현재 시간을 얻어 이전 시간을 뺀다
		::QueryPerformanceCounter((LARGE_INTEGER*)&timeElapsed);
		timeElapsed -= m_pcTimerStart;
		return (double)timeElapsed * (double)m_resolution;
	}
	else	// if not
	{
		timeElapsed = ::timeGetTime() - m_mmTimerStart;
		return (double)timeElapsed;
	}
}

//*****************************************************************************
// 절대 시간을 얻는다
//*****************************************************************************
double CTimer::GetAbsTime()
{
	__int64 absTime;

	if (m_bUsePerformanceCounter)	// if using Performance Counter
	{
		::QueryPerformanceCounter((LARGE_INTEGER*)&absTime);
		return (double)absTime * (double)m_resolution;
	}
	else	// if not
	{
		absTime = ::timeGetTime();
		return (double)absTime;
	}
}

//*****************************************************************************
// Timer를 reset한다
//*****************************************************************************
void CTimer::ResetTimer()
{
	// start time을 다시 얻는다
	if (m_bUsePerformanceCounter)	// if using Performance Counter
		::QueryPerformanceCounter((LARGE_INTEGER*)&m_pcTimerStart);
	else	// if not
		m_mmTimerStart = ::timeGetTime();
}
