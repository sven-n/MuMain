// w_ItemModuleView.cpp: implementation of the ItemModuleView class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "ZzzBMD.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzAI.h"
#include "ZzzInterface.h"
#include "ZzzInventory.h"

#include "w_ItemModuleView.h"

extern float g_fScreenRate_x;
extern float g_fScreenRate_y;

#ifdef NEW_USER_INTERFACE_UISYSTEM

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

//소스 리펙토링 꼭 할것..-.-;;;
//연습으로 해본건데..꼭 고쳐라
using namespace ui;

ItemModuleView::ItemModuleView( const string& name ) : UIWnd( name ), m_Type(-1), m_Level(-1), 
																	  m_AddOption(-1), m_ExOption(-1), m_AngleStat( false )
{
	Initialize();
	RegisterDrawEvent( eDraw_Mesh );
}

ItemModuleView::~ItemModuleView()
{
	Destroy();
}

void ItemModuleView::Initialize()
{

}

void ItemModuleView::Destroy()
{
	
}

void ItemModuleView::OnMove( const Coord& prevPos )
{
	UIWnd::OnMove( prevPos );
}

void ItemModuleView::OnResize( const Dim& prevSize )
{
	UIWnd::OnResize( prevSize );
}

void ItemModuleView::OnMouseOver()
{
	UIWnd::OnMouseOver();
	m_AngleStat = true;
	if( m_ToolTipPanel ) m_ToolTipPanel->SetAngleStat( m_AngleStat );
}

void ItemModuleView::OnMouseOut()
{
	UIWnd::OnMouseOut();
	m_AngleStat = false;
	if( m_ToolTipPanel ) m_ToolTipPanel->SetAngleStat( m_AngleStat );
}

void ItemModuleView::SetToolTipItem( const CASHSHOP_ITEMLIST& tooltipiteminfo )
{
	ITEM     tooltipitem;

	if( ChangeCodeItem( &tooltipitem, (BYTE*)tooltipiteminfo.s_btItemInfo ) )
	{
		m_Type			= tooltipitem.Type;
		m_Level			= tooltipitem.Level;
		m_AddOption		= tooltipitem.Option1;
		m_ExOption		= tooltipitem.ExtOption;
	}

	if( !m_ToolTipPanel )
	{
		ToolTipPanelPtr tooltip( new ToolTipPanel( "tooltip" ) );
		tooltip->SetInfo( tooltipiteminfo );
		tooltip->Move( Coord(GetPosition().x+(GetSize().x/2)+((GetSize().x/2)/2), GetPosition().y + 15) 
			+ Coord(  -10, -26 ) );
		tooltip->EnableLButtonEvent( false );
		AddChild( tooltip );

		m_ToolTipPanel = tooltip;
	}
}

void ItemModuleView::Process( int delta )
{

}

void ItemModuleView::DrawMesh()
{
	Coord pos  = GetScreenPosition();
	Dim	  size = GetSize();
	
	ITEM_ATTRIBUTE* pItemAttr = &ItemAttribute[m_Type];
	
	float width  = pItemAttr->Width;
	float height = pItemAttr->Height+20;
	
	pos = Coord( (pos.x+(size.x/2))-(width/2), (pos.y)+(height)+18 );

	if( m_Type == ITEM_HELPER+60 )
	{
		pos.y -= 20;
	}
#ifdef LDK_ADD_PC4_GUARDIAN
	else if( m_Type == ITEM_HELPER+64 || m_Type == ITEM_HELPER+65 )
	{
		pos.y -= 20;
	}
#endif //LDK_ADD_PC4_GUARDIAN
#ifdef YDG_ADD_CS7_CRITICAL_MAGIC_RING
	else if (m_Type == ITEM_HELPER+107)
	{
		// 치명마법반지
		pos.y -= 10;
	}
#endif	// YDG_ADD_CS7_CRITICAL_MAGIC_RING
#ifdef YDG_ADD_CS7_MAX_AG_AURA
	else if(m_Type == ITEM_HELPER+104)
	{
		// AG증가 오라
		pos.y -= 10;
	}
#endif	// YDG_ADD_CS7_MAX_AG_AURA
#ifdef YDG_ADD_CS7_MAX_SD_AURA
	else if(m_Type == ITEM_HELPER+105)
	{
		// SD증가 오라
		pos.y -= 10;
	}
#endif	// YDG_ADD_CS7_MAX_SD_AURA
#ifdef YDG_ADD_CS7_PARTY_EXP_BONUS_ITEM
	else if(m_Type == ITEM_HELPER+103)
	{
		// 파티 경험치 증가 아이템
		pos.y -= 5;
	}
#endif	// YDG_ADD_CS7_PARTY_EXP_BONUS_ITEM
#ifdef LDK_ADD_CS7_UNICORN_PET
	else if(m_Type == ITEM_HELPER+106)
	{
		// 유니콘펫
		pos.y -= 15;
	}
#endif	// LDK_ADD_CS7_UNICORN_PET
#ifdef YDG_ADD_CS7_ELITE_SD_POTION
	else if(m_Type == ITEM_POTION+133)
	{
		// 엘리트 SD회복 물약
		pos.y -= 5;
	}
#endif	// YDG_ADD_CS7_ELITE_SD_POTION

	Coord OldWindowSize = Coord( WindowWidth, WindowHeight );
	
	WindowWidth = 640;
	WindowHeight = 480;
	
	Coord OldMousePos = Coord( MouseX, MouseY );

	MouseX = MouseX*g_fScreenRate_x;
	MouseY = MouseY*g_fScreenRate_y;
	
	if( m_AngleStat )
	{
		MouseX = pos.x;
		MouseY = pos.y;
	}

	RenderItem3D( static_cast<float>(pos.x), static_cast<float>(pos.y), width, height, 
					 m_Type, m_Level, m_AddOption, m_ExOption, true );

	MouseX = OldMousePos.x;
	MouseY = OldMousePos.y;
	
	WindowWidth = OldWindowSize.x;
	WindowHeight = OldWindowSize.y;
}

#endif //NEW_USER_INTERFACE_UISYSTEM