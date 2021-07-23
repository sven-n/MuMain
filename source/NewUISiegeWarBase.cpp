// NewUISiegeWarBase.cpp: implementation of the CNewUISiegeWarBase class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

using namespace SEASON3B;

#include "NewUISiegeWarBase.h"
#include "wsclientinline.h"
#include "GMBattleCastle.h"
#include "UIGuildInfo.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

SEASON3B::CNewUISiegeWarBase::CNewUISiegeWarBase()
{
	m_iMiniMapScale = 1;
	m_fMiniMapAlpha = 1.f;
	
	m_bSecond = true;
	m_fTime = 0.f;
	m_iHour = 0;
	m_iMinute = 0;
	m_bRenderSkillUI = false;
	m_bRenderToolTip = false;
	
	memset( &m_MiniMapFramePos, 0, sizeof(POINT) );
	memset( &m_MiniMapPos, 0, sizeof(POINT) );
	memset( &m_TimeUIPos, 0, sizeof(POINT) );
	memset( &m_SkillFramePos, 0, sizeof(POINT) );
	memset( &m_BtnSkillScrollUpPos, 0, sizeof(POINT) );
	memset( &m_BtnSkillScrollDnPos, 0, sizeof(POINT) );
	memset( &m_SkillIconPos , 0, sizeof(POINT) );
	memset( &m_UseSkillDestKillPos, 0, sizeof(POINT) );
	memset( &m_CurKillCountPos, 0, sizeof(POINT) );
	memset( &m_BtnAlphaPos, 0, sizeof(POINT) );
	memset( &m_SkillTooltipPos, 0 , sizeof(POINT) );

	memset( &m_HeroPosInWorld, 0, sizeof(POINT) );
	memset( &m_HeroPosInMiniMap, 0, sizeof(POINT) );
	memset( &m_MiniMapScaleOffset, 0, sizeof(POINT) );
}

SEASON3B::CNewUISiegeWarBase::~CNewUISiegeWarBase()
{

}

//---------------------------------------------------------------------------------------------
// Create
bool SEASON3B::CNewUISiegeWarBase::Create( int x, int y )
{
	SetPos( x, y );

	if( !OnCreate( x, y ) )
		return false;

	// 알파버튼 초기화
	unicode::t_char szText[256] = {NULL, };
	unicode::_sprintf( szText, "%d", (int)(m_fMiniMapAlpha*100.5f) );
	m_BtnAlpha.ChangeText( szText );
	m_BtnAlpha.ChangeButtonImgState( true, IMAGE_BTN_ALPHA, true );
	m_BtnAlpha.ChangeButtonInfo( m_BtnAlphaPos.x, m_BtnAlphaPos.y, BTN_ALPHA_WIDTH, BTN_ALPHA_HEIGHT );

	if( battleCastle::IsBattleCastleStart() == true )	
	{
// 		if( InitBattleSkill() == true )
// 		{
// 			m_bRenderSkillUI = true;
// 		}
// 		else
// 		{
// 			m_bRenderSkillUI = false;
// 		}

		InitBattleSkill();
	}

	return true;
}

//---------------------------------------------------------------------------------------------
// Release
void SEASON3B::CNewUISiegeWarBase::Release()
{
	ReleaseBattleSkill();

	OnRelease();
}

//---------------------------------------------------------------------------------------------
// Update
bool SEASON3B::CNewUISiegeWarBase::Update()
{
	UpdateBuffState();
	UpdateHeroPos();

	OnUpdate();

	return true;
}

//---------------------------------------------------------------------------------------------
// Render
bool SEASON3B::CNewUISiegeWarBase::Render()
{	
	unicode::t_char szText[256] = {0, };

	EnableAlphaTest();
	glColor4f ( 1.f, 1.f, 1.f, m_fMiniMapAlpha );
	g_pRenderText->SetFont( g_hFontBold );
	g_pRenderText->SetTextColor( 255, 255, 255, 255 );
	g_pRenderText->SetBgColor( 0, 0, 0, 0 );
	
	// 미니맵
	RenderBitmap ( IMAGE_MINIMAP, (float)(m_MiniMapPos.x), (float)(m_MiniMapPos.y), 128.f, 128.f, m_fMiniMapTexU, m_fMiniMapTexV, 0.5f*m_iMiniMapScale, 0.5f*m_iMiniMapScale );

	// 미니맵 UI Frame
	RenderImage( IMAGE_MINIMAP_FRAME, m_MiniMapFramePos.x, m_MiniMapFramePos.y, MINIMAP_FRAME_WIDTH, MINIMAP_FRAME_HEIGHT );
	RenderImage( IMAGE_TIME_FRAME, m_TimeUIPos.x, m_TimeUIPos.y, TIME_FRAME_WIDTH, TIME_FRAME_HEIGHT );	
	
	// Time
	glColor4f ( 1.f, 1.f, 1.f, m_fMiniMapAlpha );
	if( battleCastle::IsBattleCastleStart() )
	{
		g_pRenderText->SetFont( g_hFontBig );

		if ( ( WorldTime-m_fTime ) > 500 )
		{
			m_fTime = WorldTime;
			m_bSecond = true;
		}

		if ( m_bSecond )
		{
			if( m_iMinute < 10 )
			{
				unicode::_sprintf( szText, "%d:0%d", m_iHour, m_iMinute );
			}
			else
			{
				unicode::_sprintf( szText, "%d:%d", m_iHour, m_iMinute );
			}		
		}
		else
		{
			if( m_iMinute < 10 )
			{
				unicode::_sprintf( szText, "%d 0%d", m_iHour, m_iMinute );
			}
			else
			{
				unicode::_sprintf( szText, "%d %d", m_iHour, m_iMinute );
			}
		}
		g_pRenderText->RenderText( m_TimeUIPos.x, m_TimeUIPos.y+10, szText, 134, 0, RT3_SORT_CENTER );
 	}

	OnRender();

	// 미니맵상 주인공의 위치
    glColor4f( 1.f, 1.f, 0.f, m_fMiniMapAlpha );
    RenderColor( (float)(m_HeroPosInMiniMap.x), (float)(m_HeroPosInMiniMap.y), 3, 3 );

	DisableAlphaBlend();

	EnableAlphaTest();
	glColor4f ( 1.f, 1.f, 1.f, m_fMiniMapAlpha );
	
	if( m_bRenderSkillUI == true )
	{
		// 스킬창
		RenderImage( IMAGE_BATTLESKILL_FRAME, m_SkillFramePos.x, m_SkillFramePos.y, 
			BATTLESKILL_FRAME_WIDTH, BATTLESKILL_FRAME_HEIGHT );
		
		// 스킬 아이콘 렌더
		RenderSkillIcon();
		
		// 스킬 스크롤 버튼
		m_BtnSkillScroll[0].ChangeAlpha( m_fMiniMapAlpha );
		m_BtnSkillScroll[1].ChangeAlpha( m_fMiniMapAlpha );
		m_BtnSkillScroll[0].Render();	
		m_BtnSkillScroll[1].Render();
	}
	
	// Alpha버튼
	m_BtnAlpha.SetFont( g_hFontBold );
	m_BtnAlpha.ChangeAlpha( m_fMiniMapAlpha ); 
	m_BtnAlpha.Render();

	DisableAlphaBlend();



	return true;
}

//---------------------------------------------------------------------------------------------
// InitBattleSkill
bool SEASON3B::CNewUISiegeWarBase::InitBattleSkill()
{
	ReleaseBattleSkill();
	
	// 길드장이거나 배틀마스터가 아닐때 
	if ( !(Hero->EtcPart == PARTS_ATTACK_TEAM_MARK 
		|| Hero->EtcPart == PARTS_ATTACK_TEAM_MARK2
		|| Hero->EtcPart == PARTS_ATTACK_TEAM_MARK3
		|| Hero->EtcPart == PARTS_DEFENSE_TEAM_MARK) 
		|| Hero->GuildStatus == G_PERSON)
	{
		return false;
	}

	// 스킬 스크롤버튼 초기화
	m_BtnSkillScroll[0].ChangeButtonImgState( true, IMAGE_SKILL_BTN_SCROLL_UP, true );
	m_BtnSkillScroll[0].ChangeButtonInfo( m_BtnSkillScrollUpPos.x, m_BtnSkillScrollUpPos.y, SKILL_BTN_SCROLL_WIDTH, SKILL_BTN_SCROLL_HEIGHT );
	m_BtnSkillScroll[1].ChangeButtonImgState( true, IMAGE_SKILL_BTN_SCROLL_DN, true );
	m_BtnSkillScroll[1].ChangeButtonInfo( m_BtnSkillScrollDnPos.x, m_BtnSkillScrollDnPos.y, SKILL_BTN_SCROLL_WIDTH, SKILL_BTN_SCROLL_HEIGHT );
	
	switch( Hero->GuildStatus )
	{
	case G_MASTER:
		{
			m_listBattleSkill.push_back( AT_SKILL_INVISIBLE );
			m_listBattleSkill.push_back( AT_SKILL_REMOVAL_INVISIBLE );
			if( GetBaseClass ( Hero->Class ) == CLASS_DARK_LORD )
			{
				m_listBattleSkill.push_back( AT_SKILL_REMOVAL_BUFF );
			}
			m_bRenderSkillUI = true;
		}break;
	case G_SUB_MASTER:
		{
			m_listBattleSkill.push_back( AT_SKILL_INVISIBLE );
			m_listBattleSkill.push_back( AT_SKILL_REMOVAL_INVISIBLE );
			m_bRenderSkillUI = true;
		}break;
	case G_BATTLE_MASTER:
		{
			m_listBattleSkill.push_back( AT_SKILL_STUN );
			m_listBattleSkill.push_back( AT_SKILL_REMOVAL_STUN );
			m_listBattleSkill.push_back( AT_SKILL_MANA );
			m_bRenderSkillUI = true;
		}break;
	}
	
	m_iterCurBattleSkill = m_listBattleSkill.begin();

	Hero->GuildSkill = (*m_iterCurBattleSkill);
				
	return true;
}

//---------------------------------------------------------------------------------------------
// ReleaseBattleSkill
void SEASON3B::CNewUISiegeWarBase::ReleaseBattleSkill()
{
	m_listBattleSkill.clear();

	m_bRenderSkillUI = false;
}


//---------------------------------------------------------------------------------------------
// SetSkillScrollUp
void SEASON3B::CNewUISiegeWarBase::SetSkillScrollUp()
{
	if( m_listBattleSkill.begin() == m_iterCurBattleSkill )
		return;

	m_iterCurBattleSkill--;

	Hero->GuildSkill = (*m_iterCurBattleSkill);
}

//---------------------------------------------------------------------------------------------
// SetSkillScrollDn
void SEASON3B::CNewUISiegeWarBase::SetSkillScrollDn()
{
	if( m_listBattleSkill.end() == ++m_iterCurBattleSkill )
	{
		m_iterCurBattleSkill--;
		return;
	}

	Hero->GuildSkill = (*m_iterCurBattleSkill);
}

//---------------------------------------------------------------------------------------------
// UpdateMouseEvent
bool SEASON3B::CNewUISiegeWarBase::UpdateMouseEvent()
{	
	if( !OnUpdateMouseEvent() )
		return false;	
	
	if( BtnProcess() )
		return false;
	
	if( CheckMouseIn(m_MiniMapFramePos.x, m_MiniMapFramePos.y, MINIMAP_FRAME_WIDTH, MINIMAP_FRAME_HEIGHT)
		|| CheckMouseIn(m_TimeUIPos.x, m_TimeUIPos.y, TIME_FRAME_WIDTH, TIME_FRAME_HEIGHT) )
		return false;	
	
	if( m_bRenderSkillUI == true )
	{
		// 스킬아이콘 툴팁 렌더 이벤트 발생
		if( CheckMouseIn( m_SkillIconPos.x, m_SkillIconPos.y, SKILL_ICON_WIDTH, SKILL_ICON_HEIGHT ) )
		{
			m_bRenderToolTip = true;
			return false;
		}
		else
		{
			m_bRenderToolTip = false;
		}	
		
		if( CheckMouseIn(m_SkillFramePos.x, m_SkillFramePos.y, BATTLESKILL_FRAME_WIDTH, BATTLESKILL_FRAME_HEIGHT) )
			return false;
	}

	return true;
}

//---------------------------------------------------------------------------------------------
// UpdateKeyEvent
bool SEASON3B::CNewUISiegeWarBase::UpdateKeyEvent()
{
	if( !OnUpdateKeyEvent() )
		return false;

	return true;
}

//---------------------------------------------------------------------------------------------
// BtnProcess
bool SEASON3B::CNewUISiegeWarBase::BtnProcess()
{
	POINT ptScaleBtn = { m_MiniMapFramePos.x+134, m_MiniMapFramePos.y+7 };
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	POINT ptAlphaBtn = { m_MiniMapFramePos.x+59, m_MiniMapFramePos.y+5 };
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX

	// 미니맵 알파값 변경 버튼
	if( m_BtnAlpha.UpdateMouseEvent() )
	{		
		// 알파값을 바꾼다.
		if( m_fMiniMapAlpha <= 0.5f )
		{
			m_fMiniMapAlpha = 1.f;
		}
		else
		{
			m_fMiniMapAlpha = m_fMiniMapAlpha - 0.1f;
		}
		
		unicode::t_char szText[256] = {NULL, };
		unicode::_sprintf( szText, "%d", (int)(m_fMiniMapAlpha*100.5f) );
		m_BtnAlpha.ChangeText( szText );
		m_BtnAlpha.ChangeAlpha( m_fMiniMapAlpha );

		return true;
	}
	
	// 스케일 변경 버튼
	if(SEASON3B::IsPress(VK_LBUTTON) && CheckMouseIn(ptScaleBtn.x, ptScaleBtn.y, 13, 12))
	{
		if( m_iMiniMapScale == 1)
			m_iMiniMapScale = 2;
		else
			m_iMiniMapScale = 1;

		return true;
	}
	
	if( m_bRenderSkillUI == true )
	{
		// 스킬 스크롤 버튼
		if( m_BtnSkillScroll[0].UpdateMouseEvent() )
		{
			SetSkillScrollUp();
			return true;
		}
		if( m_BtnSkillScroll[1].UpdateMouseEvent() )
		{
			SetSkillScrollDn();
			return true;
		}
		
		if ( MouseWheel > 0 )
		{
			SetSkillScrollUp();
			MouseWheel = 0;
			return true;
		}
		if ( MouseWheel < 0 )
		{
			SetSkillScrollDn();
			MouseWheel = 0;
			return true;
		}
	}

	return false;
}

//---------------------------------------------------------------------------------------------
// UpdateBuffState
void SEASON3B::CNewUISiegeWarBase::UpdateBuffState()
{
	DWORD m_dwBuffState = -1;

	OBJECT *o = &Hero->Object;

	if( g_isCharacterBuff( o, eBuff_CastleRegimentAttack1 ) )
	{
		m_dwBuffState = eBuff_CastleRegimentAttack1;
	}
	else if( g_isCharacterBuff( o, eBuff_CastleRegimentAttack2 ) )
	{
		m_dwBuffState = eBuff_CastleRegimentAttack2;
	}
	else if( g_isCharacterBuff( o, eBuff_CastleRegimentAttack3 ) )
	{
		m_dwBuffState = eBuff_CastleRegimentAttack3;
	}
	else if( g_isCharacterBuff( o, eBuff_CastleRegimentDefense ) )
	{
		m_dwBuffState = eBuff_CastleRegimentDefense;
	}
}

//---------------------------------------------------------------------------------------------
// UpdateHeroPos
// 주인공의 미니맵상 위치 Update
void SEASON3B::CNewUISiegeWarBase::UpdateHeroPos()
{	
    m_HeroPosInWorld.x = ( Hero->PositionX ) / m_iMiniMapScale;
    m_HeroPosInWorld.y = ( 256 - ( Hero->PositionY ) ) / m_iMiniMapScale;

    m_MiniMapScaleOffset.x = max( (m_HeroPosInWorld.x-(64*m_iMiniMapScale)), 0 );
    m_MiniMapScaleOffset.y = min( max( (m_HeroPosInWorld.y-(64*m_iMiniMapScale)), 0 ), 128 );    
	
	m_HeroPosInMiniMap.x = m_HeroPosInWorld.x-m_MiniMapScaleOffset.x+m_MiniMapPos.x;
    m_HeroPosInMiniMap.y = m_HeroPosInWorld.y-m_MiniMapScaleOffset.y+m_MiniMapPos.y;
    
    m_fMiniMapTexU = (float)(m_MiniMapScaleOffset.x)/(256.f/(float)m_iMiniMapScale);
    m_fMiniMapTexV = (float)(m_MiniMapScaleOffset.y)/(256.f/(float)m_iMiniMapScale);
}

//---------------------------------------------------------------------------------------------
// RenderCmdIconInMiniMap
// 미니맵상의 명령Icon 렌더
void SEASON3B::CNewUISiegeWarBase::RenderCmdIconInMiniMap()
{
	// 지도에 찍힌 명령 렌더
	int iWidth, iHeight;
	unicode::t_char szText[256] = {0, };
	POINT Pos;
	memset(&Pos, 0, sizeof(POINT));

#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	vec3_t Light = { 1.f, 1.f, 1.f };
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	for ( int i=0 ; i<MAX_COMMANDGROUP ; i++ )
	{
		int iBWidth;
		switch ( m_CmdBuffer[i].byCmd )
		{
			case 0: iWidth=COMMAND_ATTACK_WIDTH; iHeight=COMMAND_ATTACK_HEIGHT; iBWidth = 16; break;
			case 1: iWidth=COMMAND_DEFENCE_WIDTH; iHeight=COMMAND_DEFENCE_HEIGHT; iBWidth = 32; break;
			case 2: iWidth=COMMAND_WAIT_WIDTH; iHeight=COMMAND_WAIT_HEIGHT; iBWidth = 16; break;
		}
		
		if ( m_CmdBuffer[i].byCmd!=3 && m_CmdBuffer[i].byTeam>=0 && m_CmdBuffer[i].byTeam<=6 )
		{
			Pos.x = ( m_CmdBuffer[i].byX )/m_iMiniMapScale - m_MiniMapScaleOffset.x + m_MiniMapPos.x;
			Pos.y = ( 256 - m_CmdBuffer[i].byY )/m_iMiniMapScale - m_MiniMapScaleOffset.y + m_MiniMapPos.y;
			
			// 미니맵 밖에 있는건 렌더 하지 않음
			if( Pos.x<m_MiniMapPos.x || Pos.x>m_MiniMapPos.x+128 || Pos.y<m_MiniMapPos.y || Pos.y>m_MiniMapPos.y+128 )
				continue;
			
			if ( m_CmdBuffer[i].byLifeTime>0 )
			{
				glColor4f( 1.f, 1.f+sinf( m_CmdBuffer[i].byLifeTime*0.2f ), 1.f+sinf( m_CmdBuffer[i].byLifeTime*0.2f ), m_fMiniMapAlpha );
				m_CmdBuffer[i].byLifeTime--;
			}
			else
			{
				glColor4f( 1.f, 1.f, 1.f, m_fMiniMapAlpha );
			}
			unicode::_sprintf( szText, "%d", m_CmdBuffer[i].byTeam+1 );
			g_pRenderText->RenderText( Pos.x-12, Pos.y-5, szText );
			RenderBitmap( IMAGE_COMMAND_ATTACK+m_CmdBuffer[i].byCmd, Pos.x-7, Pos.y-7, 11.f, 11.f, 0.f, 0.f, 
							((float)iWidth-1.f)/(float)iBWidth, ((float)iHeight-1.f)/16.f );

		}	
	}
}

void SEASON3B::CNewUISiegeWarBase::RenderSkillIcon()
{
	int iUseSkillDestKill;		// 현재 선택된 스킬의 목표 킬수
	int iSelectSkill;			// 현재 선택된 스킬
	int iCurKillCount;			// 현재 킬수

	unicode::t_char szText[256] = {NULL, };

	// 현재 길드 스킬에 사용가능한 킬 카운트를 구해놓는다.
	iUseSkillDestKill = SkillAttribute[Hero->GuildSkill].KillCount;
	
	//  스킬 아이콘 Index
	iSelectSkill = (*m_iterCurBattleSkill);
	iCurKillCount = Hero->GuildMasterKillCount;

	// 길드마스터의 킬 카운트가 스킬에 사용가능한 킬 카운트 이상이면
	// 색깔 활성화 아니면 비활성화
    if( Hero->GuildMasterKillCount < iUseSkillDestKill )
	{
		glColor4f( 1.f, 0.5f, 0.5f, m_fMiniMapAlpha );
	}

	int src_x, src_y;
	src_x = ((iSelectSkill - 57) % 8) * 20.f;
	src_y = ((iSelectSkill - 57) / 8) * 28.f;

	RenderImage( IMAGE_SKILL_ICON, m_SkillIconPos.x+1, m_SkillIconPos.y, (float)SKILL_ICON_WIDTH, (float)SKILL_ICON_HEIGHT, src_x, src_y);

	glColor4f( 1.f, 1.f, 1.f, m_fMiniMapAlpha );

	g_pRenderText->SetFont( g_hFontBig );
	unicode::_sprintf( szText, "%d", iUseSkillDestKill );
	g_pRenderText->RenderText( m_UseSkillDestKillPos.x, m_UseSkillDestKillPos.y, szText );
	unicode::_sprintf( szText, "%d", iCurKillCount );
	g_pRenderText->RenderText( m_CurKillCountPos.x, m_CurKillCountPos.y, szText );
	

	// 스킬 툴팁정보 렌더
	if ( m_bRenderToolTip == true )
    {
		RenderSkillInfo( m_SkillTooltipPos.x, m_SkillTooltipPos.y, FindHotKey( Hero->GuildSkill ), Hero->GuildSkill, STRP_BOTTOMCENTER);		
		// 사용안함
		//RenderTipText( m_SkillFramePos.x-60, m_SkillFramePos.y-10, GlobalText[1474] );		// 1474 : "공성전 에서만 스킬을 변경할 수 있습니다."
    }
}

//---------------------------------------------------------------------------------------------
// SetPos
void SEASON3B::CNewUISiegeWarBase::SetPos( int x, int y )
{
	m_MiniMapFramePos.x = x;
	m_MiniMapFramePos.y = y;
	m_MiniMapPos.x = m_MiniMapFramePos.x + 25;
	m_MiniMapPos.y = m_MiniMapFramePos.y + 28; 
	m_TimeUIPos.x = m_MiniMapFramePos.x + 20; 
	m_TimeUIPos.y = m_MiniMapFramePos.y + MINIMAP_FRAME_HEIGHT - 4;
	m_SkillFramePos.x = x + 26;
	m_SkillFramePos.y = y - BATTLESKILL_FRAME_HEIGHT;
	m_BtnSkillScrollUpPos.x = m_SkillFramePos.x + 48;
	m_BtnSkillScrollUpPos.y = m_SkillFramePos.y + 21;
	m_BtnSkillScrollDnPos.x = m_BtnSkillScrollUpPos.x;
	m_BtnSkillScrollDnPos.y = m_BtnSkillScrollUpPos.y + 15;
	m_SkillIconPos.x = m_SkillFramePos.x + 25;
	m_SkillIconPos.y = m_SkillFramePos.y + 21;
	m_UseSkillDestKillPos.x = m_SkillFramePos.x + 78;
	m_UseSkillDestKillPos.y = m_SkillFramePos.y + 28;
	m_CurKillCountPos.x = m_SkillFramePos.x + 97;
	m_CurKillCountPos.y = m_UseSkillDestKillPos.y;
	m_BtnAlphaPos.x = m_MiniMapFramePos.x + 58;
	m_BtnAlphaPos.y = m_MiniMapFramePos.y + 4;

	m_SkillTooltipPos.y = m_SkillFramePos.y + 16;
	m_SkillTooltipPos.x = m_SkillFramePos.x + 30; 

	OnSetPos( x, y );
}

//---------------------------------------------------------------------------------------------
// SetTime
// 서버에서 시간을 받는다.
void  SEASON3B::CNewUISiegeWarBase::SetTime( int iHour, int iMinute )
{
    m_iHour     = iHour;
    m_iMinute   = iMinute;
}

//---------------------------------------------------------------------------------------------
// SetMapInfo
void SEASON3B::CNewUISiegeWarBase::SetMapInfo( GuildCommander& data )
{
    m_CmdBuffer[data.byTeam].byCmd = data.byCmd;
    m_CmdBuffer[data.byTeam].byTeam = data.byTeam;
    m_CmdBuffer[data.byTeam].byX = data.byX;
    m_CmdBuffer[data.byTeam].byY = data.byY;
    m_CmdBuffer[data.byTeam].byLifeTime = 100;
}

//---------------------------------------------------------------------------------------------
// SetMapInfo
void SEASON3B::CNewUISiegeWarBase::SetRenderSkillUI( bool bRenderSkillUI )
{
	m_bRenderSkillUI = bRenderSkillUI;
}

//---------------------------------------------------------------------------------------------
// LoadImages
void SEASON3B::CNewUISiegeWarBase::LoadImages()
{
	LoadBitmap( "World31\\Map1.jpg" , IMAGE_MINIMAP, GL_LINEAR );			//  공성전 지도.
	LoadBitmap( "Interface\\newui_SW_Minimap_Frame.tga", IMAGE_MINIMAP_FRAME, GL_LINEAR );
	LoadBitmap( "Interface\\newui_SW_Time_Frame.tga", IMAGE_TIME_FRAME, GL_LINEAR );
	LoadBitmap( "Interface\\i_attack.tga", IMAGE_COMMAND_ATTACK );			//  공격 아이콘.
	LoadBitmap( "Interface\\i_defense.tga", IMAGE_COMMAND_DEFENCE );		//  방어 아이콘.
	LoadBitmap( "Interface\\i_wait.tga", IMAGE_COMMAND_WAIT );				//  대기 아이콘.
	LoadBitmap( "Interface\\newui_SW_BattleSkill_Frame.tga", IMAGE_BATTLESKILL_FRAME, GL_LINEAR );
	LoadBitmap( "Interface\\newui_Bt_skill_scroll_up.jpg", IMAGE_SKILL_BTN_SCROLL_UP, GL_LINEAR );
	LoadBitmap( "Interface\\newui_Bt_skill_scroll_dn.jpg", IMAGE_SKILL_BTN_SCROLL_DN, GL_LINEAR );
	LoadBitmap( "Interface\\newui_skill2.jpg", IMAGE_SKILL_ICON, GL_LINEAR );
	LoadBitmap( "Interface\\newui_SW_MiniMap_Bt_clearness.jpg", IMAGE_BTN_ALPHA, GL_LINEAR );

	OnLoadImages();
}

//---------------------------------------------------------------------------------------------
// UnLoadImages
void SEASON3B::CNewUISiegeWarBase::UnLoadImages()
{
	DeleteBitmap( IMAGE_MINIMAP );
	DeleteBitmap( IMAGE_MINIMAP_FRAME );
	DeleteBitmap( IMAGE_TIME_FRAME );
	DeleteBitmap( IMAGE_COMMAND_ATTACK );
	DeleteBitmap( IMAGE_COMMAND_DEFENCE );
	DeleteBitmap( IMAGE_COMMAND_WAIT );
	DeleteBitmap( IMAGE_BATTLESKILL_FRAME );
	DeleteBitmap( IMAGE_SKILL_BTN_SCROLL_UP );
	DeleteBitmap( IMAGE_SKILL_BTN_SCROLL_DN );
	DeleteBitmap( IMAGE_SKILL_ICON );
	DeleteBitmap( IMAGE_BTN_ALPHA );

	OnUnloadImages();
}
