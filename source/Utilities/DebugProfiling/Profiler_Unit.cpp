
#include "stdafx.h"

#ifdef LDS_ADD_DEBUGINGMODULES_FOR_PROFILING

#include ".\Profiler_Unit.h"

#include <stdio.h>

#include "Profiler_util_.h"


CProfiler_Unit::CProfiler_Unit()
{
	memset( m_lpszName, 0, sizeof(char) * MAX_PROFILESAMPLENAME );	
	ClearUnit();

	SetSortingType( EPROFILESORTING_DEFAULT );
}

CProfiler_Unit::CProfiler_Unit( unsigned int ikey, 
								char *szProfileUnitName, 
								EPROFILESORTING_TYPE eProfileSorting_Type,
								BOOL bLive, 
								unsigned int uiDegree )
{
	ClearUnit();

	m_bLive = bLive;

	key = ikey;

	//m_funcSortModule_Ascending = &CProfiler_Unit::sort_key_ascending;

	memset( m_lpszName, 0, sizeof(char) * MAX_PROFILESAMPLENAME );	

	if( szProfileUnitName )
	{
		if( strlen( szProfileUnitName ) < MAX_PROFILESAMPLENAME )
		{
			strcpy( m_lpszName, szProfileUnitName );
		}
		else
		{
			memcpy( m_lpszName, szProfileUnitName, sizeof(char) * MAX_PROFILESAMPLENAME );
		}
	}
	else
	{
		char	szTempProfileName[32];

		memset( szTempProfileName, 0, sizeof(char) * 32 );

		sprintf( szTempProfileName, "%d", ikey );

		strcpy( m_lpszName, szTempProfileName );
	}

	SetSortingType( eProfileSorting_Type );
}

CProfiler_Unit::~CProfiler_Unit()
{

}

void CProfiler_Unit::ClearUnit( void)
{
	ZeroMemory( m_lpszName, MAX_PROFILESAMPLENAME);
	m_llSumTime = 0;
	m_llSumTime_GrandTotal = 0;

	m_fLatestPercentage = 0.0f;
	m_fMaxPercentage = 0.0f;
	m_fMinPercentage = 100000000.0f;
	m_fTotalPercentage = 0.0f;

	m_ulCountInFrame = 0;
	m_ulLatestCount = 0;

	m_fAve_ms = 0.0f;
	m_fAve_ms_GrandTotal = 0.0f;
	m_ulCountInFrame_Grand = 0;

	m_ulAve_tick = 0;
	m_ulAve_tick_GrandTotal = 0;

	m_bLive = TRUE;
	m_bSuccessStart = FALSE;
	m_eRequestType = EREQUEST_NULL;

	m_uiDegree = 0;
}

void CProfiler_Unit::SetSortingType( EPROFILESORTING_TYPE eProfileSorting_Type )
{
	switch( eProfileSorting_Type )
	{
	case EPROFILESORTING_WEIGHT:
		{
			m_funcSortModule_Ascending = &CProfiler_Unit::sort_percentage_ascending;
			m_funcSortModule_Descending = &CProfiler_Unit::sort_percentage_descending;
		}
		break;
	case EPROFILESORTING_INDEX:	
		{
			m_funcSortModule_Ascending = &CProfiler_Unit::sort_key_ascending;
			m_funcSortModule_Descending = &CProfiler_Unit::sort_key_descending;
		}
		break;
	default:
		{
			m_funcSortModule_Ascending = &CProfiler_Unit::sort_key_ascending;
			m_funcSortModule_Descending = &CProfiler_Unit::sort_key_descending;
		}
		break;
	}
}

BOOL CProfiler_Unit::CheckName( char *lpszName)
{
	return ( 0 == strcmp( m_lpszName, lpszName));
}

void CProfiler_Unit::BeginUnit_( LONGLONG llTick )
{
	SetLivePseudo();
	if( FALSE == m_bLive )
	{
		m_bSuccessStart = FALSE;
		return;
	}

	m_llStartTick = llTick;
	m_bSuccessStart = TRUE;
}

void CProfiler_Unit::EndUnit_( LONGLONG llTick)
{
	if( FALSE == m_bSuccessStart )
	{
		return;
	}
	m_llSumTime += ( llTick - m_llStartTick);
	++m_ulCountInFrame;
}

void CProfiler_Unit::SetLivePseudo()
{
	if( EREQUEST_NULL == m_eRequestType )
	{
		return;
	}

	switch( m_eRequestType )
	{
	case EREQUEST_LIVE:
		{
			m_eRequestType = EREQUEST_NULL;
			m_bLive = TRUE;
		}
		break;
	case EREQUEST_KILL:
		{
			m_eRequestType = EREQUEST_NULL;
			m_bLive = FALSE;
		}
		break;
	}
}

void CProfiler_Unit::GenerateResult( const LONGLONG &llLastestTotalTime, const LONGLONG &llGrandTotalTime, const LONGLONG &llTickPerMS )
{
	if ( m_llSumTime <= 0)
	{	// 한번도 호출 안한경우 그대로 유지
		return;
	}

	// 시간 계산
	m_llSumTime_GrandTotal += m_llSumTime;

	// 연산 점유율 계산
	double dLatestPercentage = 0.0, dMaxPercentage = 0.0, dMinPercentage = 0.0, dTotalPercentage = 0.0;
	
	/*
	m_fLatestPercentage	= 100.0f * ( double)m_llSumTime / ( double)llLastestTotalTime;
	m_fMaxPercentage	= max ( m_fLatestPercentage, m_fMaxPercentage);
	m_fMinPercentage	= min ( m_fMinPercentage, m_fLatestPercentage);
	m_fTotalPercentage	= 100.0f * ( double)m_llSumTime_GrandTotal / ( double)llGrandTotalTime;
	*/
	
	dLatestPercentage	= m_fLatestPercentage;
	dMaxPercentage		= m_fMaxPercentage;
	dMinPercentage		= m_fMinPercentage;
	dTotalPercentage	= m_fTotalPercentage;

	dLatestPercentage	= 100.0f * ( double)m_llSumTime / ( double)llLastestTotalTime;
	dMaxPercentage		= max ( dLatestPercentage, dMaxPercentage);
	dMinPercentage		= min ( dMinPercentage, dLatestPercentage);
	dTotalPercentage	= 100.0f * ( double)m_llSumTime_GrandTotal / ( double)llGrandTotalTime;

	m_fLatestPercentage	= (float)dLatestPercentage;
	m_fMaxPercentage	= (float)dMaxPercentage;
	m_fMinPercentage	= (float)dMinPercentage;
	m_fTotalPercentage	= (float)dTotalPercentage;	

	// 호출 회수
	m_ulLatestCount = m_ulCountInFrame;
	m_ulCountInFrame_Grand += m_ulCountInFrame;

	// 호출 시간 평균
	if( llTickPerMS != 1 && m_ulCountInFrame != 0 && m_ulCountInFrame_Grand != 0 )
	{
		m_fAve_ms = (float)(m_llSumTime / m_ulCountInFrame) / (float)llTickPerMS;
		m_ulAve_tick = m_llSumTime / m_ulCountInFrame;

		m_fAve_ms_GrandTotal = (float)(m_llSumTime_GrandTotal / m_ulCountInFrame_Grand) / (float)llTickPerMS;
		m_ulAve_tick_GrandTotal = m_llSumTime_GrandTotal / m_ulCountInFrame_Grand;
	}

	m_llSumTime = 0;
	m_ulCountInFrame = 0;
}

void CProfiler_Unit::GetResult_Percentage( float *pfLatestPercentage, 
								   float *pfMaxPercentage, 
								   float *pfMinPercentage, 
								   float *pfTotalPercentage )
{
	*pfLatestPercentage = m_fLatestPercentage;
	*pfMaxPercentage = m_fMaxPercentage;
	*pfMinPercentage = m_fMinPercentage;
	*pfTotalPercentage = m_fTotalPercentage;
}

void CProfiler_Unit::GetResult_StatisticAve( float *pfAve_ms, 
											 unsigned long *pulAve_tick, 
											 float *pfAve_ms_GrandTotal,
											 unsigned long *pulAve_tick_GrandTotal )
{
	*pfAve_ms = m_fAve_ms;
	*pulAve_tick = m_ulAve_tick;
	*pfAve_ms_GrandTotal = m_fAve_ms_GrandTotal;
	*pulAve_tick_GrandTotal = m_ulAve_tick_GrandTotal;
}

void CProfiler_Unit::GetResult_AllProperty( unsigned int uiKey, 
						   float *pfLatestPercentage, float *pfMaxPercentage, float *pfMinPercentage, float *pfTotalPercentage,
						   float *pfAve_ms, unsigned long *pulAve_tick, float *pfAve_ms_GrandTotal, unsigned long *pulAve_tick_GrandTotal )
{
	*pfLatestPercentage = m_fLatestPercentage;
	*pfMaxPercentage = m_fMaxPercentage;
	*pfMinPercentage = m_fMinPercentage;
	*pfTotalPercentage = m_fTotalPercentage;

	*pfAve_ms = m_fAve_ms;
	*pulAve_tick = m_ulAve_tick;
	*pfAve_ms_GrandTotal = m_fAve_ms_GrandTotal;
	*pulAve_tick_GrandTotal = m_ulAve_tick_GrandTotal;
}

void CProfiler_Unit::GetResultText_( char *lpszText)
{
	sprintf( lpszText, "Cur : %3.2f%% (%d CallTime), Max : %3.2f%%, Min : %3.2f%%, Avg : %3.2f%%", 
			m_fLatestPercentage, m_ulLatestCount, m_fMaxPercentage, m_fMinPercentage, m_fTotalPercentage);
}

void CProfiler_Unit::GetResultText_Average( char *lpszText )
{
	/*sprintf( lpszText, "Call Time: %d, Total Call Time: %d, ProcessSec_Ave: %10.8fms, ProcessSec_TotalAve: %10.8fms",
	m_ulLatestCount, m_fAve_ms, m_fAve_ms_GrandTotal );*/
	sprintf( lpszText, "Call Time: %d, Total Call Time: %d, ProcessSec_Ave: %f ms, ProcessSec_TotalAve: %f ms",
		m_ulLatestCount, m_ulCountInFrame_Grand, m_fAve_ms, m_fAve_ms_GrandTotal );
}

void CProfiler_Unit::FillBlankBuffer( char *szSrcBuffer, const unsigned int uiMaxBuffer )
{
	unsigned int uiCurrentBuf_ = 0, uiBlankBuf = 0;
	
	uiCurrentBuf_ = strlen( szSrcBuffer );

	if( uiMaxBuffer < uiCurrentBuf_ )
	{
		szSrcBuffer[uiMaxBuffer] = 0;
		return;
	}
	
	uiBlankBuf = uiMaxBuffer - uiCurrentBuf_;
	if( 0 > uiBlankBuf )
	{
		return;
	}
	
	unsigned int uiBuf = 0;
	for( unsigned int uiBlank = 0; uiBlank < uiBlankBuf; ++uiBlank )
	{
		uiCurrentBuf_ = uiCurrentBuf_ + sprintf( szSrcBuffer + uiCurrentBuf_ , " " );
	}
	
	szSrcBuffer[uiMaxBuffer] = 0;
}


#define LENTITLETEXT_	512
#define LENUNITTEXT_	32

void CProfiler_Unit::GetResultText_Average_Total( char *lpszText, OUTPUT_OPTION iRequestOutputOption )
{
	unsigned int uiBuf = 0, uiTitleBuf = 0;
	const unsigned int uiTitleBufMax = 40;
	long lTitleAdditionalBlankBuf = 0;
	unsigned int uiBufOffset = 0;
	
	char	szTitleText[LENTITLETEXT_], 
			szTitleText_Blank[LENTITLETEXT_], 
			szTitleText_Ticks[LENTITLETEXT_],
			szTitleText_CallCount[LENTITLETEXT_],
			szTitleText_MS[LENTITLETEXT_],
			szTitleText_Result[LENTITLETEXT_];
	
	memset( szTitleText, 0, sizeof( char ) * LENTITLETEXT_ );
	memset( szTitleText_Blank, 0, sizeof( char ) * LENTITLETEXT_ );
	memset( szTitleText_Ticks, 0, sizeof( char ) * LENTITLETEXT_ );
	memset( szTitleText_CallCount, 0, sizeof( char ) * LENTITLETEXT_ );
	memset( szTitleText_MS, 0, sizeof( char ) * LENTITLETEXT_ );
	memset( szTitleText_Result, 0, sizeof( char ) * LENTITLETEXT_ );
	
	uiTitleBuf = uiTitleBuf + sprintf( uiTitleBuf + szTitleText , "  " );
	for( unsigned int uiBlank = 0; uiBlank < m_uiDegree; ++uiBlank )
	{
		uiTitleBuf = uiTitleBuf + sprintf( uiTitleBuf + szTitleText, "-" );
	}
	
	uiTitleBuf = uiTitleBuf + sprintf( uiTitleBuf + szTitleText, "%d. %s ", key, m_lpszName );
	
	unsigned int uiCurrentBuf_ = 0;
	
	FillBlankBuffer( szTitleText, uiTitleBufMax );
	FillBlankBuffer( szTitleText_Blank, uiTitleBufMax );
	
	
	char		szResultUnit_column1[LENUNITTEXT_];
	char		szResultUnit_column2[LENUNITTEXT_];
	char		szResultUnit_column3[LENUNITTEXT_];
	char		szResultUnit_column4[LENUNITTEXT_];
	
	memset( szResultUnit_column1, 0, sizeof( char ) * LENUNITTEXT_ );
	memset( szResultUnit_column2, 0, sizeof( char ) * LENUNITTEXT_ );
	memset( szResultUnit_column3, 0, sizeof( char ) * LENUNITTEXT_ );
	memset( szResultUnit_column4, 0, sizeof( char ) * LENUNITTEXT_ );
	
	sprintf( szTitleText_Result,	"%s *Rates     ", szTitleText );		// Rates Title 
	sprintf( szTitleText_MS,		"%s *MS        ", szTitleText_Blank );	// MS Title 
	sprintf( szTitleText_Ticks,		"%s *MegaTicks ", szTitleText_Blank );	// Ticks Title 
	sprintf( szTitleText_CallCount,	"%s *CallCount ", szTitleText_Blank );	// CallCount Title 
	


	if( OUTPUT_OPTION_RATES & iRequestOutputOption )
	{
		// Unit 1
		sprintf( szResultUnit_column1, "(%7.2f)%2s", m_fLatestPercentage, "%% " );
		sprintf( szResultUnit_column2, "%3s(%7.2f)%2s", "Max", m_fMaxPercentage, "%% " );
		sprintf( szResultUnit_column3, "%3s(%7.2f)%2s", "Min", m_fMinPercentage, "%% " );
		sprintf( szResultUnit_column4, "%3s(%7.2f)%2s", "Tot", m_fTotalPercentage, "%% " );
		
		uiBuf = uiBuf + sprintf( lpszText + uiBuf, "%s:%s,%s,%s,%s\n", 
			szTitleText_Result,
			szResultUnit_column1, szResultUnit_column2, szResultUnit_column3, szResultUnit_column4 );
	// / Unit 1
	}
	

	if( OUTPUT_OPTION_MEGATICKS & iRequestOutputOption )
	{
		// Unit 2
		sprintf( szResultUnit_column1, "(%7d)%2s", (unsigned int)(m_ulAve_tick*0.000001), "MT" );
		sprintf( szResultUnit_column2, "%3s(%7d)%2s", "Tot", 
			(unsigned int)(m_ulAve_tick_GrandTotal*0.000001), "MT" );
		
		uiBuf = uiBuf + sprintf( lpszText + uiBuf, "%s:%s,%s \n", 
			szTitleText_Ticks, szResultUnit_column1, szResultUnit_column2 );
		// / Unit 2
	}


	if( OUTPUT_OPTION_CALLCOUNT & iRequestOutputOption )
	{
		// Unit 3
		sprintf( szResultUnit_column1, "(%7d)%2s", m_ulLatestCount," " );
		sprintf( szResultUnit_column2, "%3s(%7d)%2s", "Tot", m_ulCountInFrame_Grand, " " );
		
		uiBuf = uiBuf + sprintf( lpszText + uiBuf, "%s:%s,%s \n", 
			szTitleText_CallCount, szResultUnit_column1, szResultUnit_column2 );
		// / Unit 3
	}
	

	if( OUTPUT_OPTION_MS & iRequestOutputOption )
	{
		// Unit 4
		sprintf( szResultUnit_column1, "(%7.1f)%2s", m_fAve_ms, "ms" );
		sprintf( szResultUnit_column2, "%3s(%7.1f)%2s", "Tot", m_fAve_ms_GrandTotal, "ms" );
		
		uiBuf = uiBuf + sprintf( lpszText + uiBuf, "%s:%s,%s\n", 
			szTitleText_MS, szResultUnit_column1, szResultUnit_column2 );
		// / Unit 4
	}
}

#endif // LDS_ADD_DEBUGINGMODULES_FOR_PROFILING