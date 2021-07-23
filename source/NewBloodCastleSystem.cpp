// NewBloodCastleSystem.cpp: implementation of the CNewBloodCastleSystem class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "NewBloodCastleSystem.h"
#include "NewUICustomMessageBox.h"
#include "NewUISystem.h"
#include "DSPlaySound.h"
#include "CSChaosCastle.h"

using namespace SEASON3B;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CNewBloodCastleSystem::CNewBloodCastleSystem()
{

}

CNewBloodCastleSystem::~CNewBloodCastleSystem()
{

}

//////////////////////////////////////////////////////////////////////////
//  경기 결과에 필요한 값을 저장한다.
//////////////////////////////////////////////////////////////////////////
void CNewBloodCastleSystem::SetMatchResult ( const int iNumDevilRank, const int iMyRank, const MatchResult *pMatchResult, const int Success )
{
    if(iNumDevilRank != 255)
	{
		return;
	}

	m_iNumResult = Success;
	memcpy( m_MatchResult, pMatchResult, sizeof(MatchResult));
	SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CBloodCastleResultMsgBoxLayout));
}


//////////////////////////////////////////////////////////////////////////
//  블러드 캐슬 게임 커멘드 처리.
//////////////////////////////////////////////////////////////////////////
void CNewBloodCastleSystem::SetMatchGameCommand ( const LPPRECEIVE_MATCH_GAME_STATE data )
{                        
    //  블러드 캐슬.
    switch ( data->m_byPlayState )
    {
    case 0: //  게임 시작.
        SetAllAction ( PLAYER_RUSH1 );
        PlayBuffer( SOUND_BLOODCASTLE, NULL, true );

    case 1: //  게임 진행중.
        SetMatchInfo ( data->m_byPlayState+1, 15*60, data->m_wRemainSec, data->m_wMaxKillMonster, data->m_wCurKillMonster );

        //  절대 퀘스트 아이템.
        if ( data->m_wIndex!=65535 && data->m_byItemType!=255 && data->m_byItemType!=0 )
        {
            WORD Key = data->m_wIndex; 
	        Key &= 0x7FFF;
	        
            int  index = HangerBloodCastleQuestItem ( Key );
	        CHARACTER* c = &CharactersClient[index];

            if ( c!=NULL )
            {
                c->EtcPart = data->m_byItemType;
            }
        }
        break;

    case 2: //  게임 종료.
        clearMatchInfo ();
        StopBuffer ( SOUND_BLOODCASTLE, true );

        break;

    case 3: //  성문을 닫는다.
        //  성문을 연다. ( 테스트 ).
        SetActionObject ( World, 36, 20, 1.f );
        break;
    case 4: //  블러드 캐슬, 보스 몬스터를 제거, 게임 진행중.
        SetMatchInfo ( data->m_byPlayState+1, 15*60, data->m_wRemainSec, data->m_wMaxKillMonster, data->m_wCurKillMonster );

        //  절대 퀘스트 아이템.
        if ( data->m_wIndex!=65535 && data->m_byItemType!=255 && data->m_byItemType!=0 )
        {
            WORD Key = data->m_wIndex; 
	        Key &= 0x7FFF;
	        
            int  index = HangerBloodCastleQuestItem ( Key );
	        CHARACTER* c = &CharactersClient[index];

            if ( c!=NULL )
            {
                c->EtcPart = data->m_byItemType;
            }
        }
        break;
    }
}




//////////////////////////////////////////////////////////////////////////
//  경기에 필요한 정보를 표시한다.
//////////////////////////////////////////////////////////////////////////
void CNewBloodCastleSystem::RenderMatchTimes ( void )
{
	//  카운트 다운
	if ( m_byMatchType > 0 && InBloodCastle() == true )
	{
        switch ( m_byMatchType )
        {
            //  블러드 캐슬.
        case 0:
        case 1:
        case 2: //  몬스터수.
        case 5: //  보스 몬스터 수.
            if ( m_iMatchTime>0 )
            {
                if( !g_pNewUISystem->IsVisible( SEASON3B::INTERFACE_BLOODCASTLE_TIME ) )
				{
					g_pNewUISystem->Show( SEASON3B::INTERFACE_BLOODCASTLE_TIME );
				}
				
				g_pBloodCastle->SetTime( m_iMatchTime );
				g_pBloodCastle->SetKillMonsterStatue( m_iKillMonster, m_iMaxKillMonster );	
				
            }
            break;

        default :
            break;
        }
	}
	else
	{
		if( g_pNewUISystem->IsVisible( SEASON3B::INTERFACE_BLOODCASTLE_TIME ) )
		{
			g_pNewUISystem->Hide( SEASON3B::INTERFACE_BLOODCASTLE_TIME );
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//  경기 결과를 화면에 출력한다.
//////////////////////////////////////////////////////////////////////////
void CNewBloodCastleSystem::RenderMatchResult ( void )
{
	int x = 640/2;
	int yPos = m_PosResult.y + 40;

	EnableAlphaTest();

	g_pRenderText->SetFont( g_hFont );
	g_pRenderText->SetTextColor(128, 255, 128, 255);
	g_pRenderText->SetBgColor( 0, 0, 0, 0 );

	unicode::t_char lpszStr[256] = {NULL, };

	// 축하 메시지
    if ( m_iNumResult )
    {
		g_pRenderText->RenderText(x,yPos,GlobalText[857], 0, 0, RT3_WRITE_CENTER); 
		yPos += 16;
	    g_pRenderText->RenderText(x,yPos,GlobalText[858], 0, 0, RT3_WRITE_CENTER);
    }
    else
    {
		g_pRenderText->RenderText(x,yPos,GlobalText[859], 0, 0, RT3_WRITE_CENTER); 
		yPos += 16;
	    g_pRenderText->RenderText(x,yPos,GlobalText[860], 0, 0, RT3_WRITE_CENTER);
    }

	yPos += 30;

	MatchResult *pResult = &m_MatchResult[0];

	//  보상 경험치
	g_pRenderText->SetFont(g_hFontBold);
	g_pRenderText->SetTextColor(210, 255, 210, 255);
	unicode::_sprintf( lpszStr, GlobalText[861], pResult->m_dwExp);
	g_pRenderText->RenderText(x, yPos, lpszStr, 0, 0, RT3_WRITE_CENTER); 
	yPos += 24;
	
    if ( m_iNumResult )
    {
		//  보상 젠
		g_pRenderText->SetTextColor(255, 210, 210, 255);
		unicode::_sprintf( lpszStr, GlobalText[862], pResult->m_iZen);
		g_pRenderText->RenderText(x, yPos, lpszStr, 0, 0, RT3_WRITE_CENTER); 
		yPos += 24;
    }

    //  이벤트 점수
	g_pRenderText->SetTextColor(210, 210, 255, 255);
	unicode::_sprintf( lpszStr, GlobalText[863], pResult->m_iScore );
	g_pRenderText->RenderText(x, yPos, lpszStr, 0, 0, RT3_WRITE_CENTER);

	DisableAlphaBlend();
}

