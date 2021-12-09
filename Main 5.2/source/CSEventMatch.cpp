//////////////////////////////////////////////////////////////////////////
//  CSEventMatch.cpp
//////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "UIWindows.h"
#include "ZzzOpenglUtil.h"
#include "ZzzBMD.h"
#include "zzzlodterrain.h"
#include "ZzzScene.h"
#include "zzzAi.h"
#include "CSEventMatch.h"
#include "wsclientinline.h"
#include "NewUICustomMessageBox.h"
#include "NewUISystem.h"

extern int g_iCustomMessageBoxButton[NUM_BUTTON_CMB][NUM_PAR_BUTTON_CMB];

void CSBaseMatch::clearMatchInfo ( void )
{
    m_byMatchType   = 0;
    m_iMatchTimeMax = -1;
    m_iMatchTime    = -1;
    m_iMaxKillMonster = -1;
    m_iKillMonster  = -1;
	SetPosition( 640-230/2, 100 ); 
}

bool CSBaseMatch::getEqualMonster ( int addV )
{
    if ( m_iKillMonster<=(m_iMaxKillMonster+addV) ) return  true;

    return false;
}

void CSBaseMatch::StartMatchCountDown ( int iType )
{
	if (m_iMatchCountDownType>=TYPE_MATCH_DOPPELGANGER_ENTER_CLOSE && m_iMatchCountDownType<=TYPE_MATCH_DOPPELGANGER_CLOSE)
	{
		if (!(iType>=TYPE_MATCH_DOPPELGANGER_ENTER_CLOSE && iType<=TYPE_MATCH_DOPPELGANGER_CLOSE) && iType != TYPE_MATCH_NONE)
		{
			return;
		}
	}
	m_iMatchCountDownType = (MATCH_TYPE)iType;
	m_dwMatchCountDownStart = GetTickCount();
}
void CSBaseMatch::SetMatchInfo ( const BYTE byType, const int iMaxTime, const int iTime, const int iMaxMonster, const int iKillMonster )
{
    m_byMatchType   = byType;
    m_iMatchTimeMax = iMaxTime;
    m_iMatchTime    = iTime;
    m_iMaxKillMonster=iMaxMonster;
    m_iKillMonster  = iKillMonster;
	SetPosition( 640-230/2, 100 ); 
}

void CSBaseMatch::RenderTime ( void )
{
	float x, y;

	if ( m_iMatchCountDownType>TYPE_MATCH_NONE && m_iMatchCountDownType<TYPE_MATCH_END )
	{
		DWORD dwCurrent = GetTickCount ();
		if ( dwCurrent - m_dwMatchCountDownStart>30000 )
		{
			m_iMatchCountDownType = TYPE_MATCH_NONE;
		}
		else
		{
			DisableAlphaBlend ();
			EnableAlphaTest ( false );

			x=10;
			y = 480-70;
			g_pRenderText->SetTextColor(128, 128, 255, 255);
			g_pRenderText->SetBgColor(0, 0, 0, 128);
			
			char lpszStr[256];
			int iTime = ( dwCurrent - m_dwMatchCountDownStart) / 1000;
            if ( m_iMatchCountDownType>=TYPE_MATCH_CASTLE_ENTER_CLOSE && m_iMatchCountDownType<=TYPE_MATCH_CASTLE_END )
            {
                int textNum = 824+m_iMatchCountDownType-TYPE_MATCH_CASTLE_ENTER_CLOSE;
                wsprintf ( lpszStr, GlobalText[textNum], GlobalText[1146], 30-iTime );
            }
            else if ( m_iMatchCountDownType>=TYPE_MATCH_CHAOS_ENTER_START && m_iMatchCountDownType<=TYPE_MATCH_CHAOS_END )
            {
                int textNum = 824+m_iMatchCountDownType-TYPE_MATCH_CHAOS_ENTER_START;
                if ( textNum==825 )
                {
                    textNum = 828;
				}
			    wsprintf ( lpszStr, GlobalText[textNum], GlobalText[1147], 30-iTime );
            }
			else if( m_iMatchCountDownType == TYPE_MATCH_CURSEDTEMPLE_ENTER_CLOSE
				  || m_iMatchCountDownType == TYPE_MATCH_CURSEDTEMPLE_GAME_START )
			{
				int textNum = 2384;

				if( m_iMatchCountDownType == TYPE_MATCH_CURSEDTEMPLE_GAME_START ) textNum = 2386;

				wsprintf ( lpszStr, GlobalText[textNum], 30-iTime );
			}
			else if ( m_iMatchCountDownType>=TYPE_MATCH_DOPPELGANGER_ENTER_CLOSE && m_iMatchCountDownType<=TYPE_MATCH_DOPPELGANGER_CLOSE )
			{
				int textNum = 2860+m_iMatchCountDownType-TYPE_MATCH_DOPPELGANGER_ENTER_CLOSE;
				wsprintf ( lpszStr, GlobalText[textNum], 30-iTime );
			}
            else
            {
			    wsprintf ( lpszStr, GlobalText[640+m_iMatchCountDownType-TYPE_MATCH_DEVIL_ENTER_START], 30-iTime );
            }		
			g_pRenderText->RenderText( 640/2, (int)y, lpszStr, 0, 0, RT3_WRITE_CENTER );
		}
	}
}

void CSBaseMatch::renderOnlyTime ( float x, float y, int MatchTime )
{
	char lpszStr[256];
	char lpszStrS[10];
    int iMinute = MatchTime/60;

    wsprintf ( lpszStr, " %.2d :", iMinute );

    int iSecondTime = MatchTime-(iMinute*60);
    if ( iSecondTime>=0 )
    {
    	wsprintf ( lpszStrS, " %.2d", iSecondTime );
        strcat ( lpszStr, lpszStrS );
    }

    if ( iMinute<5 )
    {
		g_pRenderText->SetTextColor(255, 32, 32, 255);
    }
    if ( iMinute<15 )
    {
    	wsprintf ( lpszStrS, ": %.2d", ((int)WorldTime%60) );
        strcat ( lpszStr, lpszStrS );
    }
	g_pRenderText->SetFont(g_hFontBig);
	g_pRenderText->RenderText((int)x,(int)y,lpszStr, 0, 0, RT3_WRITE_CENTER);
}

void CSBaseMatch::SetPosition( int ix, int iy )
{
	m_PosResult.x = ix;
	m_PosResult.y = iy;
}

void CSDevilSquareMatch::SetMatchResult ( const int iNumDevilRank, const int iMyRank, const MatchResult *pMatchResult, const int Success )
{
    if(iNumDevilRank >= 200) 
	{
		return;
	}

    m_iNumResult = iNumDevilRank;
	m_iMyResult = iMyRank;

	memcpy( m_MatchResult, pMatchResult, m_iNumResult * sizeof ( MatchResult));

	SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CDevilSquareRankMsgBoxLayout));
}

void CSDevilSquareMatch::RenderMatchTimes ( void )
{
    return;
}

void CSDevilSquareMatch::SetMatchGameCommand ( const LPPRECEIVE_MATCH_GAME_STATE data )
{
    return;
}

void CSDevilSquareMatch::RenderMatchResult ( void )
{
	int xPos[6] = {m_PosResult.x, };
	xPos[1] = xPos[0] + 15;
	xPos[2] = xPos[1] + 15;
	xPos[3] = xPos[2] + 60;
	xPos[4] = xPos[3] + 50;
	xPos[5] = xPos[4] + 38;

	int yPos = m_PosResult.y + 40;
	
	g_pRenderText->SetBgColor(0);

	char lpszStr[256];

	g_pRenderText->SetTextColor(255, 255, 255, 255);
	g_pRenderText->RenderText(xPos[2],yPos,GlobalText[647]); 
	yPos += 16;
	wsprintf( lpszStr, GlobalText[648], Hero->ID);
	g_pRenderText->RenderText(( xPos[2]),yPos,lpszStr); 
	yPos += 24;

	g_pRenderText->SetTextColor(0, 255, 0, 255);
	g_pRenderText->RenderText( xPos[2], yPos, GlobalText[680], xPos[3]-xPos[1], RT3_SORT_CENTER );
	g_pRenderText->RenderText( xPos[3], yPos, GlobalText[682], xPos[4]-xPos[3], RT3_SORT_CENTER );
	g_pRenderText->RenderText( xPos[4], yPos, GlobalText[683], xPos[5]-xPos[4], RT3_SORT_CENTER );
	g_pRenderText->RenderText( xPos[5], yPos, GlobalText[684], (640-230)/2+210-xPos[5], RT3_SORT_CENTER);
	yPos += 20;
	
	int yStartPos = yPos;

	for ( int i = 0; i < m_iNumResult; ++i)
	{
		MatchResult *pResult = &m_MatchResult[(i + 1) % m_iNumResult];

		if ( i == m_iNumResult - 1)
		{
			yPos = yStartPos + 16 * 10;
		
			g_pRenderText->SetTextColor(200, 120, 0, 255);
			g_pRenderText->RenderText(xPos[0], yPos, GlobalText[685], 230, 0, RT3_SORT_CENTER );
			yPos += 20;
			wsprintf( lpszStr, "%2d", m_iMyResult);
		}
		else
		{
			g_pRenderText->SetTextColor(255, 255, 0, 255);
			wsprintf( lpszStr, "%2d", i+1);
		}
		g_pRenderText->RenderText(xPos[1],yPos,lpszStr);

		ZeroMemory( lpszStr, MAX_ID_SIZE + 1);
		memcpy( lpszStr, pResult->m_lpID, MAX_ID_SIZE);
		g_pRenderText->RenderText(xPos[2],yPos,lpszStr);

		wsprintf( lpszStr, "%10d", pResult->m_iScore);
		g_pRenderText->RenderText(xPos[3],yPos,lpszStr);

		wsprintf( lpszStr, "%6d", pResult->m_dwExp);
		g_pRenderText->RenderText(xPos[4],yPos,lpszStr);

		wsprintf( lpszStr, "%6d", pResult->m_iZen);
		g_pRenderText->RenderText(xPos[5],yPos,lpszStr); yPos += 16;
	}
}

void CCursedTempleMatch::SetMatchGameCommand ( const LPPRECEIVE_MATCH_GAME_STATE data )
{
	return;
}

void CCursedTempleMatch::SetMatchResult ( const int iNumDevilRank, const int iMyRank, const MatchResult *pMatchResult, const int Success )
{
	return;
}

void CCursedTempleMatch::RenderMatchTimes ()
{
	return;
}

void CCursedTempleMatch::RenderMatchResult ()
{
	return;
}

