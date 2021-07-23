// Profiler_Clock.h: interface for the CProfiler_Clock class.
//
//////////////////////////////////////////////////////////////////////

#ifdef LDS_ADD_DEBUGINGMODULES_FOR_PROFILING

#if !defined(AFX_PROFILER_CLOCK_H__D0EAE5F8_6D19_4487_AF5A_6C764507B973__INCLUDED_)
#define AFX_PROFILER_CLOCK_H__D0EAE5F8_6D19_4487_AF5A_6C764507B973__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <Wtypes.h>

#include <assert.h>

#include "debugSingleton_.h"


#define CFS_MOVE		( 0x0001)
#define CFS_RENDER		( 0x0002)


class CProfiler_Clock : public CdebugSingleton_<CProfiler_Clock>
{
public:
	CProfiler_Clock();
	virtual ~CProfiler_Clock();
	virtual void Clear( void);

	/// @name 고밀도 tick 얻기
public:
	virtual LONGLONG GetCustomTick() const
	{
		//DWORD dwLow, dwHigh;
		//_asm
		//{
		//	rdtsc
		//	mov dwLow, eax
		//	mov dwHigh, edx
		//}
		//return ( ( ( unsigned __int64)dwHigh << 32) | ( unsigned __int64)dwLow);
		LARGE_INTEGER liCounter;
		QueryPerformanceCounter( &liCounter);
		return ( ( UINT64)liCounter.QuadPart);
	}

	/// @name 고밀도 tick 과 실제 tick 의 관계 측정
protected:
	LONGLONG m_llMeasureCustomTick;
	DWORD m_dwMeasureTick;
public:
	virtual void BeginMeasureTick( void);
	virtual void EndMeasureTick( void);
	virtual void AutoMeasure( DWORD dwMillisecond);	///< EndMeasure 없이, 일정 시간이 지나면 자동으로 계산

	/// @name 고밀도 tick 이용하기
protected:
	BOOL m_bCustomTickMeasured;	///< measure 완료
	LONGLONG m_llCustomTickPerMillisecond;	///< 1 ms 당 틱
public:
	virtual BOOL IsMeasureComplete() const { return ( m_bCustomTickMeasured); }
	virtual LONGLONG GetCustomTickPerMillisecond() const { return ( m_llCustomTickPerMillisecond); }
	virtual DWORD CustomTick2Millisecond( LONGLONG llTick) const
	{
		return ( ( DWORD)( llTick / m_llCustomTickPerMillisecond));
	}
	virtual LONGLONG Millisecond2CustomTick( DWORD dwTick) const
	{
		return ( dwTick * m_llCustomTickPerMillisecond);
	}
	virtual DWORD GetCurrentTick() const	///< GetTickCount 대신 이용 가능 (단, 같은 프레임 안에서는 변하지 않는다.)
	{
		return ( ( DWORD)( m_llLatestFrameTick / m_llCustomTickPerMillisecond));
	}
	virtual DWORD GetTickCount() const	///< GetTickCount 대신 이용 가능 (단, 같은 프레임 안에서도 변한다.)
	{
		return ( ( DWORD)( GetCustomTick() / m_llCustomTickPerMillisecond));
	}

	/// @name Frame skipping
protected:
	LONGLONG m_llLatestFrameTick;	///< 최근 프레임의 tick
	DWORD m_dwLatestFrameMoveTick;	///< CheckFrameSkipping 의 결과 move 할 경우 최근 tick 저장
	DWORD m_dwLatestRenderTick;		///< 최근에 render 한 tick 저장 (render 없이 move 만 되는 경우 방지)
	int m_iFramePerSecond;			///< 희망 초당 프레임
	BOOL m_bEnableRenderOnly;		///< Render 만 하는 경우도 있게 할 것인지 여부
	long m_lFrameCount;				///< 프레임 세기
	int m_iMinFramePerSecond;
	BOOL m_bFixedFrame;
	DWORD m_dwBaseTick;
	BOOL m_bUpdateClock;

public:
	// 초당 프레임 설정
	virtual BOOL SetFramePerSecond( int iMaxFramePerSecond, BOOL bEnableRenderOnly = TRUE, 
		int iMinFramePerSecond = 0, BOOL bFixedFrame = TRUE);	///< Move 관련 fps 지정

	//virtual BOOL New_SetFramePerSecond( int iMinFramePerSecond, int iMaxFramePerSecond, BOOL bEnableRenderOnly = TRUE);
	virtual int GetFramePerSecond() const	{ return ( m_iFramePerSecond); }
	virtual WORD CheckFrameSkipping( DWORD *pdwTick);	///< Move 와 Render 중 할 수 있는 것 얻기
	virtual void Reset( void);
	virtual long GetFrameCount() const	{ return ( m_lFrameCount); }

	// add 2006.6.9 by koma0
	//{
	virtual DWORD GetFrameMoveTick() const { return GetPassTick( m_dwLatestFrameMoveTick, m_dwBaseTick); }
	virtual DWORD GetRenderTick() const 
	{ 
		// fix 2007.8.17 by koma0
		// x_Clock을 사용하지 않을 시 RenderTick값은 GetCurrentTick
		return ( m_bUpdateClock) ? GetPassTick( m_dwLatestRenderTick, m_dwBaseTick) : GetTickCount();
	}
protected:
	enum { TICK_MAXVALUE = 0xffffffff };
	DWORD GetPassTick( DWORD dwTickTo, DWORD dwTickFrom) const
	{
		return ( dwTickTo < dwTickFrom) ? TICK_MAXVALUE - ( dwTickFrom - dwTickTo) : dwTickTo - dwTickFrom;
	}
	//}

};

#endif // !defined(AFX_PROFILER_CLOCK_H__D0EAE5F8_6D19_4487_AF5A_6C764507B973__INCLUDED_)

#endif // LDS_ADD_DEBUGINGMODULES_FOR_PROFILING