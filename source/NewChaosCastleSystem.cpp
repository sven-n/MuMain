// NewUIChaosCastleSystem.cpp: implementation of the CNewUIChaosCastleSystem class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

using namespace SEASON3B;

#include "NewChaosCastleSystem.h"

#include "UIWindows.h"
#include "ZzzOpenglUtil.h"
#include "zzztexture.h"
#include "ZzzBMD.h"
#include "zzzlodterrain.h"
#include "ZzzScene.h"
#include "zzzEffect.h"
#include "zzzAi.h"
#include "UIWindows.h"
#include "UIManager.h"
#include "CSChaosCastle.h"
#include "wsclientinline.h"
#include "NewUICustomMessageBox.h"

//////////////////////////////////////////////////////////////////////////
//  EXTERN.
//////////////////////////////////////////////////////////////////////////

extern int g_iChatInputType;
extern int g_iCustomMessageBoxButton[NUM_BUTTON_CMB][NUM_PAR_BUTTON_CMB];// ok, cancel // 사용여부, x, y, width, height
extern  int g_iActionObjectType;
extern  int g_iActionWorld;
extern  int g_iActionTime;
extern  float g_fActionObjectVelocity;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
CNewChaosCastleSystem::CNewChaosCastleSystem()
{
	int iChaosCastleLimitArea1[16] = { 23, 75, 44, 76, 43, 77, 44, 108, 23, 107, 42, 108, 23, 77, 24, 106 };
	int iChaosCastleLimitArea2[16] = { 25, 77, 42, 78, 41, 79, 42, 106, 25, 105, 40, 106, 25, 79, 26, 104 };
	int iChaosCastleLimitArea3[16] = { 27, 79, 40, 80, 39, 81, 40, 104, 27, 103, 38, 104, 27, 81, 28, 102 };
	memcpy( m_iChaosCastleLimitArea1, iChaosCastleLimitArea1, sizeof(int)*16 );
	memcpy( m_iChaosCastleLimitArea2, iChaosCastleLimitArea2, sizeof(int)*16 );
	memcpy( m_iChaosCastleLimitArea3, iChaosCastleLimitArea3, sizeof(int)*16 );

	m_byCurrCastleLevel = 255;
	m_bActionMatch = true;
}

CNewChaosCastleSystem::~CNewChaosCastleSystem()
{
	
}



//////////////////////////////////////////////////////////////////////////
//  경기 결과에 필요한 값을 저장한다.
//////////////////////////////////////////////////////////////////////////
void CNewChaosCastleSystem::SetMatchResult ( const int iNumDevilRank, const int iMyRank, const MatchResult *pMatchResult, const int Success )
{
    if(iNumDevilRank != 254)
	{
		return;
	}
	
    m_iNumResult = Success;
	
	memcpy( m_MatchResult, pMatchResult, sizeof ( MatchResult));
	
	SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CChaosCastleResultMsgBoxLayout));
}


//////////////////////////////////////////////////////////////////////////
//  SetMatchCommand
//////////////////////////////////////////////////////////////////////////
void CNewChaosCastleSystem::SetMatchGameCommand ( const LPPRECEIVE_MATCH_GAME_STATE data )
{
    //  카오스 캐슬
    switch ( data->m_byPlayState )
    {
    case 5 :    //  경기 시작.
        m_bActionMatch = true;
        m_byCurrCastleLevel = 255;
		
        SetAllAction ( PLAYER_RUSH1 );
        StopBuffer ( SOUND_CHAOS_ENVIR, true );
        PlayBuffer ( SOUND_CHAOSCASTLE, NULL, true );
        break;
    case 6 :    //  게임 진행중.
        SetMatchInfo ( data->m_byPlayState+1, 15*60, data->m_wRemainSec, data->m_wMaxKillMonster, data->m_wCurKillMonster );
        break;
    case 7 :    //  게임 종료.
        clearMatchInfo ();
        StopBuffer ( SOUND_CHAOSCASTLE, true );
        if ( InChaosCastle()==true )
        {
            PlayBuffer ( SOUND_CHAOS_ENVIR, NULL, true );
            PlayBuffer ( SOUND_CHAOS_END );
        }
        break;
		
        //  처음 제한 레벨 돌파.
    case 8:
        if ( m_byCurrCastleLevel==0 )
        {
            m_byCurrCastleLevel = 1;
            SetActionObject ( World, 1, 40, 1 );
            AddTerrainAttributeRange ( 23,  75, 22,  2, TW_NOGROUND, 1 );
            AddTerrainAttributeRange ( 43,  77,  2, 32, TW_NOGROUND, 1 );
            AddTerrainAttributeRange ( 23, 107, 20,  2, TW_NOGROUND, 1 );
            AddTerrainAttributeRange ( 23,  77,  2, 30, TW_NOGROUND, 1 );
			
            PlayBuffer ( SOUND_CHAOS_FALLING_STONE );
        }
        break;
		
        //  둘째 제한 레벨 돌파.
    case 9:
        if ( m_byCurrCastleLevel==3 )
        {
            m_byCurrCastleLevel = 4;
            SetActionObject ( World, 1, 40, 1 );
            AddTerrainAttributeRange ( 25,  77, 18,  2, TW_NOGROUND, 1 );
            AddTerrainAttributeRange ( 41,  79,  2, 28, TW_NOGROUND, 1 );
            AddTerrainAttributeRange ( 25, 105, 16,  2, TW_NOGROUND, 1 );
            AddTerrainAttributeRange ( 25,  79,  2, 26, TW_NOGROUND, 1 );
			
            PlayBuffer ( SOUND_CHAOS_FALLING_STONE );
        }
        break;
		
        //  셋째 제한 레벨 돌파.
    case 10:
        if ( m_byCurrCastleLevel==6 )
        {
            m_byCurrCastleLevel = 7;
            SetActionObject ( World, 1, 40, 1 );
            AddTerrainAttributeRange ( 27,  79, 14,  2, TW_NOGROUND, 1 );
            AddTerrainAttributeRange ( 39,  81,  2, 24, TW_NOGROUND, 1 );
            AddTerrainAttributeRange ( 27, 103, 12,  2, TW_NOGROUND, 1 );
            AddTerrainAttributeRange ( 27,  81,  2, 22, TW_NOGROUND, 1 );
			
            PlayBuffer ( SOUND_CHAOS_FALLING_STONE );
        }
        break;
    }
}

//////////////////////////////////////////////////////////////////////////
//  경기에 필요한 정보를 표시한다.
//////////////////////////////////////////////////////////////////////////
void CNewChaosCastleSystem::RenderMatchTimes ( void )
{
	//  카운트 다운
	if ( m_byMatchType > 0 && InChaosCastle() )
	{
        switch ( m_byMatchType )
        {
            //  카오스캐슬.
        case 6:
        case 7:
        case 8: //  몬스터수.
            if ( m_iMatchTime>0 )
            {
				if( !g_pNewUISystem->IsVisible( SEASON3B::INTERFACE_CHAOSCASTLE_TIME ) )
				{
					// 카오스캐슬 이벤트가 시작하면 모든 창이 닫히고 이벤트정보창만 보인다.
					g_pNewUISystem->HideAll();
					g_pNewUISystem->Show( SEASON3B::INTERFACE_CHAOSCASTLE_TIME );
				}
			
				g_pChaosCastleTime->SetTime( m_iMatchTime );
				g_pChaosCastleTime->SetKillMonsterStatue( m_iKillMonster, m_iMaxKillMonster );	
				
				
                //  금지 구역 설정값.
                if ( m_iKillMonster<=46 && m_iKillMonster>40 )  //  1단계 경고.
                {
                    m_byCurrCastleLevel = 0;
                }
                else if ( m_iKillMonster<=36 && m_iKillMonster>30 )  //  2단계 경고.
                {
                    m_byCurrCastleLevel = 3;
                }
                else if ( m_iKillMonster<=26 && m_iKillMonster>20 )  //  3단계 경고.
                {
                    m_byCurrCastleLevel = 6;
                }
            }
            break;
			
        default :
            break;
        }
	}
	else
	{
		if( g_pNewUISystem->IsVisible( SEASON3B::INTERFACE_CHAOSCASTLE_TIME ) )
		{
			g_pNewUISystem->Hide( SEASON3B::INTERFACE_CHAOSCASTLE_TIME );
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//  경기 결과를 화면에 출력한다.
//////////////////////////////////////////////////////////////////////////
void    CNewChaosCastleSystem::RenderMatchResult ( void )
{
	int x = 640/2;
	int yPos = m_PosResult.y + 40;	
	unicode::t_char lpszStr[256] = {NULL, };

	EnableAlphaTest();
	
	g_pRenderText->SetFont( g_hFont );
	g_pRenderText->SetTextColor( 128, 255, 128, 255 );
	g_pRenderText->SetBgColor( 0, 0, 0, 0 );

	// 축하 메시지
    if ( m_iNumResult )
    {
		g_pRenderText->RenderText(x, yPos, GlobalText[1151], 0, 0, RT3_WRITE_CENTER);   //  근위병들의 영혼이 정화 되었습니다.
		yPos += 16;
        unicode::_sprintf(lpszStr, "%s %s", GlobalText[1152], GlobalText[858]);
		g_pRenderText->RenderText(x, yPos, lpszStr, 0, 0, RT3_WRITE_CENTER);            //  퀘스트를 성공하였습니다.
    }
    else
    {
		unicode::_sprintf(lpszStr, "%s %s", GlobalText[1152], GlobalText[860]);
		g_pRenderText->RenderText(x, yPos, lpszStr, 0, 0, RT3_WRITE_CENTER);            //  퀘스트를 실패하였습니다.
		yPos += 16;
		g_pRenderText->RenderText(x, yPos, GlobalText[1153], 0, 0, RT3_WRITE_CENTER);   //  다음 기회를 이용해 주십시요.
    }
	yPos += 30;
	
	MatchResult *pResult = &m_MatchResult[0];
	
	g_pRenderText->SetFont(g_hFontBold);	
	g_pRenderText->SetTextColor(210, 255, 210, 255);

	//  보상 경험치
	unicode::_sprintf( lpszStr, GlobalText[861], pResult->m_dwExp);
	g_pRenderText->RenderText(x, yPos, lpszStr, 0, 0, RT3_WRITE_CENTER); yPos += 20;
	
	// 점수
    unicode::_sprintf( lpszStr, GlobalText[1162], pResult->m_iScore );
	g_pRenderText->RenderText(x, yPos, lpszStr, 0, 0, RT3_WRITE_CENTER); yPos += 20;
	
	// 젠
    unicode::_sprintf( lpszStr, GlobalText[1163], pResult->m_iZen );
	g_pRenderText->RenderText(x, yPos, lpszStr, 0, 0, RT3_WRITE_CENTER); yPos += 24;

	DisableAlphaBlend();
}
