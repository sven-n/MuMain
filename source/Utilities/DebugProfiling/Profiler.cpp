// Profiler.cpp: implementation of the CProfiler class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#ifdef LDS_ADD_DEBUGINGMODULES_FOR_PROFILING

#include "profiler.h"

CProfiler::CProfiler()
{
	Clear();
	InitClock_();
}

CProfiler::~CProfiler()
{
	Release_();
}

void CProfiler::Clear( void)
{
	m_iNumSample = 0;

	m_llTotalProcessTick = 0;
	m_llTotalProcessTick_Grand = 0;

	m_ulCountInFrame = 0;
	m_ulCountInFrame_Grand = 0;

	m_bProfiling = FALSE;

	m_bRequestLive = FALSE;
	m_bRequestKill = FALSE;

	m_bRequestReset = FALSE;

	m_bRequestGenerateResult = FALSE;

	m_bGenerateResultLaststFrame = FALSE;

	m_bOutputResultsWhenGenerate = FALSE;

	m_uiGenerateCnt = 0;

	memset( m_szDefaultOutputFileName, 0, sizeof(char) * 512 );
	strcpy( m_szDefaultOutputFileName, DEFAULT_PROFILINGNAME );
	
	memset( m_szDefaultOutputResult_, 0, sizeof(char) * 4096 );
}

void CProfiler::InitClock_( void)
{
	m_pClock = m_pClock->GetThis_();

	m_pClock->AutoMeasure( 3000 );
	m_llTickPerMS = m_pClock->GetCustomTickPerMillisecond();
}

void CProfiler::Initialize_()
{
	Clear();
	for( unsigned int i = 0; i < m_arrProfileUnits.size(); ++i)
	{
		CProfiler_Unit &CurrentUnit = m_arrProfileUnits[i];

		CurrentUnit.ClearUnit();
	}
	
	m_arrProfileUnits.clear();
	
	m_bRequestLive	= FALSE;
	m_bRequestKill	= FALSE;

	m_bRequestReset = FALSE;

	m_bRequestGenerateResult = FALSE;
}

void CProfiler::Release_( void )
{
	unsigned int iCnt = m_arrProfileUnits.size();

	if( iCnt < 1 ) return;

	for( unsigned int uiCur = 0; uiCur < iCnt; ++uiCur )
	{
		CProfiler_Unit &Unit = m_arrProfileUnits[uiCur];
	}

	m_arrProfileUnits.clear();
}

void CProfiler::Reset( void)
{
	Initialize_();
}


void CProfiler::Initialize( BOOL bOutputResultsWhenGenerate, 
							char *szOutput, 
							EPROFILESORTING_TYPE eSortingtypeforOutputResultData, 
							EPROFILESORTING_DIRECTION eSortingDirectionforOutputResultData,
							BOOL bPauseWithStart )
{
	Initialize_();

	m_bOutputResultsWhenGenerate = bOutputResultsWhenGenerate;

	m_eProfileUnitsSortingtypeForOutputResultText = eSortingtypeforOutputResultData;
	m_eProfileUnitsSortingDirectionForOutputResultText = eSortingDirectionforOutputResultData;

	strcpy( m_szDefaultOutputFileName, szOutput );

	if( TRUE == bPauseWithStart )
	{
		m_bProfiling = FALSE;
	}
	else
	{
		m_bProfiling = TRUE;
	}
}


void CProfiler::Release( void )
{
	Release_();
}

void CProfiler::BeginTotal( BOOL bClearAll )
{
	if( TRUE == bClearAll ||
		( TRUE == m_bRequestReset && TRUE == m_bProfiling )
	)
	{
		// @@ All Clear
		Reset();
	}
	else
	{
		if( TRUE == m_bRequestKill )
		{
			RequestKillAllUnits();
			m_bProfiling = FALSE;
			m_bRequestKill = FALSE;
		}

		if( TRUE == m_bRequestLive )
		{
			RequestLiveAllUnits();
			m_bProfiling = TRUE;
			m_bRequestLive = FALSE;
		}
	}

	SetSortingTypeKey();
	
	if( TRUE == m_bProfiling )
	{
		if( TRUE == m_bGenerateResultLaststFrame )
		{
			m_bGenerateResultLaststFrame = FALSE;
		}
		
		m_llTotalTickStart = m_pClock->GetCustomTick();
	}
}

void CProfiler::EndTotal()
{
	if( TRUE == m_bProfiling )
	{
		m_llTotalTickEnd = m_pClock->GetCustomTick();
		LONGLONG llLastestTotalTime = m_llTotalTickEnd - m_llTotalTickStart;

		m_llTotalProcessTick += llLastestTotalTime;
		
		++m_ulCountInFrame;

		if( TRUE == m_bRequestGenerateResult )
		{
			m_ulCountInFrame_Grand += m_ulCountInFrame;
			m_llTotalProcessTick_Grand += m_llTotalProcessTick;

			GenerateResultAll( m_llTotalProcessTick, m_llTotalProcessTick_Grand, m_llTickPerMS );

			m_ulCountInFrame = 0;
			m_llTotalProcessTick = 0;

			if( TRUE == m_bOutputResultsWhenGenerate )
			{
				OutputDefaultText( m_szDefaultOutputFileName );
			}

			m_bRequestGenerateResult = FALSE;
		}
	}
}

void CProfiler::RequestLiveAllUnits( void )
{
	for( unsigned int ui_ = 0; ui_ < m_arrProfileUnits.size(); ++ui_ )
	{
		CProfiler_Unit &Profile = m_arrProfileUnits[ui_];
		
		Profile.SetRequest( EREQUEST_LIVE );
	}
}

void CProfiler::RequestKillAllUnits( void )
{
	for( unsigned int ui_ = 0; ui_ < m_arrProfileUnits.size(); ++ui_ )
	{
		CProfiler_Unit &Profile = m_arrProfileUnits[ui_];

		Profile.SetRequest( EREQUEST_KILL );
	}
}

void CProfiler::BeginUnit( unsigned int ikey, char *lpszName, BOOL bLive, unsigned int uiDegree )
{
	if( FALSE == m_bProfiling )
	{
		return;
	}
	
	LONGLONG llTick = m_pClock->GetCustomTick();
	
	int	iIdx = -1;

	m_SearchKey.key = ikey;
	iIdx = m_arrProfileUnits.binary_search( m_SearchKey );

	if( iIdx != -1 )
	{
		CProfiler_Unit &ProfileUnit = m_arrProfileUnits[iIdx];
		ProfileUnit.BeginUnit_( llTick );
	}
	else
	{
		CProfiler_Unit	ProfileUnit(	ikey, 
										lpszName, 
										EPROFILESORTING_INDEX, 
										m_bProfiling, 
										uiDegree );
		ProfileUnit.BeginUnit_( llTick );

		m_arrProfileUnits.push_back( ProfileUnit );
	}
}

void CProfiler::EndUnit( unsigned int uiKey )
{
	if( FALSE == m_bProfiling )
	{
		return;
	}
	
	LONGLONG llTick = m_pClock->GetCustomTick();

	m_SearchKey.key = uiKey;
	int iIdx = m_arrProfileUnits.binary_search( m_SearchKey );

	CProfiler_Unit	&ProfileUnit = m_arrProfileUnits[iIdx];
	ProfileUnit.EndUnit_( llTick );
}

void CProfiler::GenerateResultAll( const LONGLONG &llTotalTime, 
									const LONGLONG &llGrandTotalTime, 
									const LONGLONG &llTickPerMS )
{
	for( unsigned int iCur = 0; iCur < m_arrProfileUnits.size(); ++iCur )
	{
		m_arrProfileUnits[iCur].GenerateResult( llTotalTime, llGrandTotalTime, llTickPerMS );
	}

	++m_uiGenerateCnt;
}

void CProfiler::GetResult_Percentage( unsigned int uiKey, 
									float *pfLatestPercentage, 
									float *pfMaxPercentage, 
									float *pfMinPercentage, 
									float *pfTotalPercentage)
{
	m_SearchKey.key = uiKey;
	int iIdx = m_arrProfileUnits.binary_search( m_SearchKey );

	CProfiler_Unit &ProfileUnit = m_arrProfileUnits[iIdx];
	ProfileUnit.GetResult_Percentage( pfLatestPercentage, pfMaxPercentage, pfMinPercentage, pfTotalPercentage);
}

void CProfiler::GetResult_StatisticAve( unsigned int uiKey, 
										float *pfAve_ms, 
										unsigned long *pulAve_tick, 
										float *pfAve_ms_GrandTotal,
										unsigned long *pulAve_tick_GrandTotal )
{
	m_SearchKey.key = uiKey;
	int iIdx = m_arrProfileUnits.binary_search( m_SearchKey );

	CProfiler_Unit &ProfileUnit = m_arrProfileUnits[iIdx];
	ProfileUnit.GetResult_StatisticAve( pfAve_ms, pulAve_tick, pfAve_ms_GrandTotal, pulAve_tick_GrandTotal);
}

void CProfiler::GetResultText( unsigned int uiKey, char *lpszText )
{
	m_SearchKey.key = uiKey;
	int iIdx = m_arrProfileUnits.binary_search( m_SearchKey );

	CProfiler_Unit &ProfileUnit = m_arrProfileUnits[iIdx];
	ProfileUnit.GetResultText_( lpszText );
}

char *CProfiler::GetResultText_All( )
{
	return m_szDefaultOutputResult_;
}

void CProfiler::OutputDefaultText(	char *szDefaultFileName,
									BOOL bOutputRates, 
									BOOL bOutputProcessTicks,
									BOOL bOutputCallCounts,
									BOOL bOutputMS )
{
	if( !szDefaultFileName ) 
	{
		return;
	}

	float fLatestPercentage = 0.0f; 
	float fMaxPercentage = 0.0f;
	float fMinPercentage = 0.0f; 
	float fTotalPercentage = 0.0f;

	float fAve_ms = 0.0f;
	unsigned long ulAve_tick = 0;
	float fAve_ms_GrandTotal = 0.0f;
	unsigned long ulAve_tick_GrandTotal = 0;

	unsigned int uiBuff = 0;
	
	char		szOutput[1024];
	double		dProfilingSec = 0.0;
	float		fProfilingSec = 0.0f;

	OUTPUT_OPTION 
				iRequestOption = 0;
	
	// Set Output Option
	if( TRUE == bOutputRates ) iRequestOption = iRequestOption | OUTPUT_OPTION_RATES;
	if( TRUE == bOutputProcessTicks ) iRequestOption = iRequestOption | OUTPUT_OPTION_RATES;
	if( TRUE == bOutputCallCounts ) iRequestOption = iRequestOption | OUTPUT_OPTION_CALLCOUNT;
	if( TRUE == bOutputMS ) iRequestOption = iRequestOption | OUTPUT_OPTION_MS;
	// Set Output Option

	memset( szOutput, 0, sizeof(char) * 1024 );
	//memset( m_szDefaultOutputResult_, 0, sizeof(char) * 8192 );

    dProfilingSec = (double)m_llTotalProcessTick_Grand / (double)m_llTickPerMS * 0.001;
	fProfilingSec = (float)dProfilingSec;
	
#ifdef KWAK_ADD_TRACE_FUNC
	__TraceF(TEXT("Profiling Statstic Results (%f) (ClockTicks) ========================================================\n"), fProfilingSec);
#endif // KWAK_ADD_TRACE_FUNC

	m_FileIO._file_save( szDefaultFileName, szOutput, 0, ESAVEFILE_ACCORD );

	//uiBuff = uiBuff + sprintf( uiBuff + m_szDefaultOutputResult_, szOutput );		// Default Output String

	// 출력 형식 Key, Percentage 가중치 
	SetAllUnitsSortingType( m_eProfileUnitsSortingtypeForOutputResultText );
	
	// 출력의 Ascending, Descending
	m_arrProfileUnits.forcingsort( 
			m_eProfileUnitsSortingDirectionForOutputResultText==EPROFILESORTING_DIRECTION_ASCENDING );

	for( unsigned int iCur = 0; iCur < m_arrProfileUnits.size(); ++iCur )
	{
		CProfiler_Unit &ProfileUnit = m_arrProfileUnits[iCur];

		memset( szOutput, 0, sizeof(char) * 1024 );
		
		ProfileUnit.GetResultText_Average_Total( szOutput, iRequestOption );
		m_FileIO._file_save( szDefaultFileName, szOutput, 0, ESAVEFILE_ACCORD );

		//uiBuff = uiBuff + sprintf( uiBuff + m_szDefaultOutputResult_, szOutput );	// Default Output String
	}

	sprintf( szOutput,"================================================================================================= \n" );
	m_FileIO._file_save( szDefaultFileName, szOutput, 0, ESAVEFILE_ACCORD );

	//uiBuff = uiBuff + sprintf( uiBuff + m_szDefaultOutputResult_, szOutput );		// Default Output String

	m_bGenerateResultLaststFrame = TRUE;
}

#endif // LDS_ADD_DEBUGINGMODULES_FOR_PROFILING
