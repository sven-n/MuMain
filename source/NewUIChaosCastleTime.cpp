// NewUIChaosCastleTime.cpp: implementation of the CNewUIChaosCastleTime class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "NewUIChaosCastleTime.h"
#include "NewUISystem.h"
#include "MatchEvent.h"

using namespace SEASON3B;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNewUIChaosCastleTime::CNewUIChaosCastleTime()
{
	m_pNewUIMng = NULL;
	m_Pos.x = m_Pos.y = 0;
	m_iTime = 0;
	m_iTimeState = CC_TIME_STATE_NORMAL;
	m_iMaxKillMonster = MAX_KILL_MONSTER;
	m_iKilledMonster = 0;
}

CNewUIChaosCastleTime::~CNewUIChaosCastleTime()
{
	Release( );
}

bool CNewUIChaosCastleTime::Create( CNewUIManager* pNewUIMng, int x, int y )
{
	if( NULL == pNewUIMng )
		return false;
	
	m_pNewUIMng = pNewUIMng;
	m_pNewUIMng->AddUIObj( SEASON3B::INTERFACE_CHAOSCASTLE_TIME, this );		// 인터페이스 오브젝트 등록
	
	SetPos(x, y);
	
	LoadImages();

	Show( false );
	
	return true;
}


void CNewUIChaosCastleTime::Release()
{
	UnloadImages();
	
	if(m_pNewUIMng)
	{
		m_pNewUIMng->RemoveUIObj( this );
		m_pNewUIMng = NULL;
	}
}

void CNewUIChaosCastleTime::SetPos(int x, int y)
{
	m_Pos.x = x;
	m_Pos.y = y;
}

bool CNewUIChaosCastleTime::UpdateMouseEvent()
{
	// 버튼 처리
	if( true == BtnProcess() )	// 처리가 완료 되었다면
		return false;
	
	// 파티 창 내 영역 클릭시 하위 UI처리 및 이동 불
	// ( #include "NewUICommon" )
	if( CheckMouseIn( m_Pos.x, m_Pos.y, CHAOSCASTLE_TIME_WINDOW_WIDTH, CHAOSCASTLE_TIME_WINDOW_HEIGHT) )
		return false;
	
	return true;
}

bool CNewUIChaosCastleTime::UpdateKeyEvent()
{
	return true;
}

bool CNewUIChaosCastleTime::Update()
{
	if ( !IsVisible() )
		return true;

#ifdef KJH_FIX_INIT_EVENT_MAP_AT_ABNORMAL_EXIT
	// 카오스 캐슬에서 벗어났을때 초기화
	if( InChaosCastle() == false )
	{
		Show(false);
	}
#endif // KJH_FIX_INIT_EVENT_MAP_AT_ABNORMAL_EXIT

// 	int iMinute = m_Time/60;
//     unicode::_sprintf ( m_szTime, " %.2d: %.2d: %.2d", iMinute, m_Time%60, WorldTime%60 );
// 	
// 	if ( iMinute < 5 )
//     {
// 		m_iTimeState = CC_TIME_STATE_NORMAL;
//     }
// 	else
// 	{
// 		m_iTimeState = CC_TIME_STATE_IMMINENCE;
// 	}


	return true;
}

bool CNewUIChaosCastleTime::Render()
{
	EnableAlphaTest();
	glColor4f(1.f, 1.f, 1.f, 1.f);

	unicode::t_char szText[256] = {NULL, };
	
	g_pRenderText->SetFont( g_hFont );
	g_pRenderText->SetBgColor( 0 );
	g_pRenderText->SetTextColor( 255, 150, 0, 255 );

	RenderImage(IMAGE_CHAOSCASTLE_TIME_WINDOW, m_Pos.x, m_Pos.y, 
					float(CHAOSCASTLE_TIME_WINDOW_WIDTH), float(CHAOSCASTLE_TIME_WINDOW_HEIGHT));

	if( m_iMaxKillMonster != MAX_KILL_MONSTER )
	{
		unicode::_sprintf ( szText, GlobalText[1161], m_iKilledMonster, m_iMaxKillMonster );		// "캐릭터 : ( %d/%d )"
		g_pRenderText->RenderText( m_Pos.x ,m_Pos.y+13, szText, CHAOSCASTLE_TIME_WINDOW_WIDTH, 0, RT3_SORT_CENTER);
	}

	g_pRenderText->RenderText( m_Pos.x ,m_Pos.y+38, GlobalText[865], CHAOSCASTLE_TIME_WINDOW_WIDTH, 0, RT3_SORT_CENTER);		// "남은 시간"
				
	if( m_iTimeState == CC_TIME_STATE_IMMINENCE )
		g_pRenderText->SetTextColor(255, 32, 32, 255);

	g_pRenderText->SetFont( g_hFontBig );
	g_pRenderText->RenderText(m_Pos.x, m_Pos.y+50, m_szTime, CHAOSCASTLE_TIME_WINDOW_WIDTH, 0, RT3_SORT_CENTER);
	
	DisableAlphaBlend();
	
	return true;
}

bool CNewUIChaosCastleTime::BtnProcess()
{
	return false;
}

float CNewUIChaosCastleTime::GetLayerDepth()
{
	return 1.3f;
}

void CNewUIChaosCastleTime::OpenningProcess()
{
	
}

void CNewUIChaosCastleTime::ClosingProcess()
{
	
}

void CNewUIChaosCastleTime::LoadImages()
{
	LoadBitmap("Interface\\newui_Figure_blood.tga", IMAGE_CHAOSCASTLE_TIME_WINDOW, GL_LINEAR);
}

void CNewUIChaosCastleTime::UnloadImages()
{
	DeleteBitmap( IMAGE_CHAOSCASTLE_TIME_WINDOW );
}

void CNewUIChaosCastleTime::SetTime( int iTime )
{
	m_iTime = iTime;

	int iMinute = m_iTime/60;
    unicode::_sprintf ( m_szTime, " %.2d:%.2d:%.2d", iMinute, m_iTime%60, (int)WorldTime%60 );
	
	if ( iMinute < 5 )
    {
		m_iTimeState = CC_TIME_STATE_IMMINENCE;
    }
	else
	{
		m_iTimeState = CC_TIME_STATE_NORMAL;
	}
}

void CNewUIChaosCastleTime::SetKillMonsterStatue(int iKilled, int iMaxKill )
{
	m_iKilledMonster = iKilled;
	m_iMaxKillMonster = iMaxKill;
}
