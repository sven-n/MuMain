// Profiler_util_.h: interface for the CProfiler_util_ class.
//
//////////////////////////////////////////////////////////////////////

#ifdef LDS_ADD_DEBUGINGMODULES_FOR_PROFILING

#if !defined(AFX_PROFILER_UTIL__H__C131E066_5097_4C4E_B021_559B0F637D3A__INCLUDED_)
#define AFX_PROFILER_UTIL__H__C131E066_5097_4C4E_B021_559B0F637D3A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#pragma warning ( disable : 4305 4503 4786 4291 4312 4311 4267 4244 4355  )

#ifdef _DEBUG
#define	DEBUGOUTPUT(lpText)															\
{																						\
	static char szBuffer[512];															\
	sprintf(szBuffer, "[File: %s][Line: %d]\n[Note : %s]", __FILE__, __LINE__, lpText);	\
	OutputDebugString( szBuffer );												\
}																					
#else
#define DEBUGOUTPUT(lpText)
#endif


#ifdef _DEBUG
#define	DEBUGMSG(lpText)															\
{																						\
	static char szBuffer[512];															\
	sprintf(szBuffer, "[File: %s][Line: %d]\n[Note : %s]", __FILE__, __LINE__, lpText);	\
	MessageBox(NULL, szBuffer, "ERROR", MB_ICONERROR);									\
}																					

#else
#define DEBUGMSG(lpText)
#endif




#define	OUTPUT_NOTICE_MSG(lpText)															\
{																						\
	MessageBox(NULL, lpText, "Notice", MB_OK );									\
}

#define	OUTPUT_ERROR_MSG(lpText)															\
{																						\
	MessageBox(NULL, lpText, "Error", MB_OK );									\
}

#define	OUTPUT_NOTICE(lpText)															\
{																						\
	static char szBuffer[1024];															\
	sprintf(szBuffer, "%s", lpText);	\
	OutputDebugString( szBuffer );												\
}	

#define SAFE_DEL_ARR(T) { if(T){ delete [] T; } T = 0; }
#define SAFE_DEL(T) { if(T){ delete T; } T = 0; }

#include "Profiler_clock.h"

template <typename T>
void	_doshuffle( int iMaxCnt, int iShuffleCnt, T *pData )
{
	int iRndNum = 0;
	T tmp;
	
	CProfiler_Clock_	*pClock = m_pClock;
	
	for( int i = 0; i < iShuffleCnt; ++i )
	{
		for( int j = 0; j < iMaxCnt; ++j )
		{
			iRndNum			= pClock->GetRandNum( 0, iMaxCnt );
			tmp				= pData[j];
			pData[j]		= pData[iRndNum];
			pData[iRndNum]	= tmp;
		}
	}
}

#endif // !defined(AFX_PROFILER_UTIL__H__C131E066_5097_4C4E_B021_559B0F637D3A__INCLUDED_)

#endif // LDS_ADD_DEBUGINGMODULES_FOR_PROFILING
