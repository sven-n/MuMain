// Profiler_Clock.cpp: implementation of the CProfiler_Clock class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#ifdef LDS_ADD_DEBUGINGMODULES_FOR_PROFILING

#include "Profiler_Clock.h"

CProfiler_Clock::CProfiler_Clock()
{
	Clear();
}

CProfiler_Clock::~CProfiler_Clock()
{

}

void CProfiler_Clock::Clear( void)
{
	m_bCustomTickMeasured = FALSE;
	m_llCustomTickPerMillisecond = 1000;

	m_llLatestFrameTick = 0;
	m_dwLatestFrameMoveTick = 0;
	m_dwLatestRenderTick = 0;
	m_iFramePerSecond = 40;
	m_bEnableRenderOnly = TRUE;
	m_lFrameCount = 0;
	m_iMinFramePerSecond = 10;
	m_bFixedFrame = TRUE;
	m_dwBaseTick = 0;
	m_bUpdateClock = FALSE;
}

void CProfiler_Clock::BeginMeasureTick( void)
{
	// 시작시간 얻기
	/*DWORD dwTrashTick = ::GetTickCount();
	while ( dwTrashTick == ( m_dwMeasureTick = ::GetTickCount()))
	{	// 정확한 결과를 위해서 Tick 이 갱신된 직후의 값을 사용한다.
	}*/
	m_llMeasureCustomTick = GetCustomTick();
}

void CProfiler_Clock::EndMeasureTick( void)
{
	// 현재 시간 얻기
	/*DWORD dwTrashTick = ::GetTickCount();
	DWORD dwCurrentTick;
	while ( dwTrashTick == ( dwCurrentTick = ::GetTickCount()))
	{	// 정확한 결과를 위해서 Tick 이 갱신된 직후의 값을 사용한다.
	}*/
	LONGLONG llCurrentCustomTick = GetCustomTick();

	// 비율계산
	//m_llCustomTickPerMillisecond = ( llCurrentCustomTick - m_llMeasureCustomTick) / ( dwCurrentTick - m_dwMeasureTick);
	LARGE_INTEGER liFrequency;
	QueryPerformanceFrequency( &liFrequency);
	m_llCustomTickPerMillisecond = liFrequency.QuadPart / 1000;
	m_bCustomTickMeasured = TRUE;

	// 최근 그린 틱을 현재 시간으로
	m_llLatestFrameTick = llCurrentCustomTick;
	m_dwLatestFrameMoveTick = GetCurrentTick();
	m_dwBaseTick = GetCurrentTick();
}

void CProfiler_Clock::AutoMeasure( DWORD dwMillisecond)
{
	// 0.1 초 동안 측정 ( dwMillisecond 무시)
	BeginMeasureTick();
	//Sleep( 100);
	EndMeasureTick();
}


BOOL CProfiler_Clock::SetFramePerSecond( int iMaxFramePerSecond, BOOL bEnableRenderOnly, 
							   int iMinFramePerSecond, BOOL bFixedFrame)
{
	assert( 0 < iMaxFramePerSecond && iMaxFramePerSecond < 1000);

	m_iFramePerSecond = iMaxFramePerSecond;
	m_iMinFramePerSecond = ( iMinFramePerSecond > 0) ? iMinFramePerSecond : iMaxFramePerSecond;
	m_bEnableRenderOnly = bEnableRenderOnly;
	m_bFixedFrame = bFixedFrame;
	return ( TRUE);
}


WORD CProfiler_Clock::CheckFrameSkipping( DWORD *pdwTick)
{
	m_bUpdateClock = TRUE;

	//return ( CFS_MOVE | CFS_RENDER);
	LONGLONG llTickPerFrame = m_llCustomTickPerMillisecond * 1000 / m_iFramePerSecond;
	::Sleep( 0);	// CPU 를 과도하게 점유하는 것 방지

	LONGLONG llCurrentCustomTick = GetCustomTick();
	// 로딩 등으로 인해 프레임 간격이 클 때의 처리
	LONGLONG llElapsedTick = llCurrentCustomTick - m_llLatestFrameTick;
	if ( llElapsedTick > llTickPerFrame * 100)
	{
		llElapsedTick = llTickPerFrame;
		m_llLatestFrameTick = llCurrentCustomTick - llTickPerFrame;
	}
	// 새로운 프레임인가?
	if ( llElapsedTick >= llTickPerFrame)
	{
		//{
		// add 2006.01.16
		if( !m_bFixedFrame)
		{
			LONGLONG llMinTickPerFrame = m_llCustomTickPerMillisecond * 1000 / m_iMinFramePerSecond;
			m_llLatestFrameTick += ( llElapsedTick < llMinTickPerFrame) ? llElapsedTick : llMinTickPerFrame;
		}
		else
			//}
		{
			m_llLatestFrameTick += llTickPerFrame;
		}
		++m_lFrameCount;

		// DWORD 틱 갱신
		DWORD dwCurrentTick = GetCurrentTick();
		*pdwTick = GetPassTick( dwCurrentTick, m_dwLatestFrameMoveTick) /*dwCurrentTick - m_dwLatestFrameMoveTick*/;
		m_dwLatestFrameMoveTick = dwCurrentTick;

		// 그리기도 하는지 체크
		//if ( llCurrentCustomTick - m_llLatestFrameTick <= llTickPerFrame)
		if ( llCurrentCustomTick - m_llLatestFrameTick <= llTickPerFrame ||
			( dwCurrentTick - m_dwLatestRenderTick > 5000))
		{
			m_dwLatestRenderTick = dwCurrentTick;
			return ( CFS_MOVE | CFS_RENDER);
		}
		return ( CFS_MOVE);
	}

	if ( m_bEnableRenderOnly)
	{
		*pdwTick = 1;
		m_dwLatestRenderTick = GetCurrentTick();
		if ( m_lFrameCount == 0)
		{
			return ( CFS_MOVE | CFS_RENDER);
		}
		// 그리기만
		return ( CFS_RENDER);
	}

	return ( 0);
}




void CProfiler_Clock::Reset( void)
{
	m_llLatestFrameTick = GetCustomTick();
}

#endif // LDS_ADD_DEBUGINGMODULES_FOR_PROFILING