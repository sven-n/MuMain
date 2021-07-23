// w_ToolTipPanel.cpp: implementation of the ToolTipPanel class.
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

#include "w_ToolTipPanel.h"

#ifdef NEW_USER_INTERFACE_UISYSTEM

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

namespace
{
	Color GetColor( const DWORD color )
	{
		switch( color )
		{
		case TEXT_COLOR_WHITE:
		case TEXT_COLOR_DARKRED:
		case TEXT_COLOR_DARKBLUE:
		case TEXT_COLOR_DARKYELLOW:
			return Color( 1.f, 1.f, 1.f, 1.f );
			break;
		case TEXT_COLOR_BLUE:
			return Color( 1.f, 0.5f, 0.7f, 1.f );
			break;
		case TEXT_COLOR_GRAY:
			return Color( 1.f, 0.4f, 0.4f, 0.4f );
			break;
		case TEXT_COLOR_GREEN_BLUE:
			return Color( 1.f, 1.f, 1.f, 1.f );
			break;
		case TEXT_COLOR_RED:
			return Color( 1.f, 1.f, 0.2f, 0.1f );
			break;
		case TEXT_COLOR_YELLOW:
			return Color( 1.f, 1.f, 0.8f, 0.1f );
			break;
		case TEXT_COLOR_GREEN:
			return Color( 1.f, 0.1f, 1.f, 0.5f );
			break;
		case TEXT_COLOR_PURPLE:
			return Color( 1.f, 1.f, 0.1f, 1.f );
			break;
		case TEXT_COLOR_REDPURPLE:
			return Color( 1.f, 0.8f, 0.5f, 0.8f );
			break;
#ifdef SOCKET_SYSTEM
		case TEXT_COLOR_VIOLET:
			return Color( 1.f, 0.7f, 0.4f, 1.0f );
			break;
#endif	// SOCKET_SYSTEM
#ifdef KJH_ADD_PERIOD_ITEM_SYSTEM
		case TEXT_COLOR_ORANGE:
			return Color( 1.f, 0.9f, 0.42f, 0.04f );
			break;
#endif // KJH_ADD_PERIOD_ITEM_SYSTEM
		}
		return Color( 1.f, 1.f, 1.f, 1.f );
	}

	void GetBackColor( Color& backcolor, DWORD color )
	{
		if ( TEXT_COLOR_DARKRED == color)
		{
			backcolor = 0xFFFF3745;
			backcolor.SetAlpha( static_cast<unsigned char>((float)(0xff) * 0.7f) );
		}
		else if ( TEXT_COLOR_DARKBLUE==color)
		{
			backcolor = 0xFF5B57FF;
			backcolor.SetAlpha( static_cast<unsigned char>((float)(0xff) * 0.7f) );
		}
		else if ( TEXT_COLOR_DARKYELLOW==color)
		{
			backcolor = 0xFFFFF600;
			backcolor.SetAlpha( static_cast<unsigned char>((float)(0xff) * 0.7f) );
		}
		else if ( TEXT_COLOR_GREEN_BLUE==color)
		{
			backcolor = 0xFF57FF77;
			backcolor.SetAlpha( static_cast<unsigned char>((float)(0xff) * 0.7f) );
		}
		else
		{
			backcolor = 0x00FFFFFF;
		}
	}

};

using namespace ui;

MAKE_OBJECT_POOL( ToolTipPanel );
ToolTipPanel::ToolTipPanel( const string& name ) : UIWnd( name ), m_AngleStat( false ), m_TempPosY( 0 )
{
	RegisterDrawEvent( eDraw_Rect );
	m_Image = image::ImageUnit::Make();
	m_Image->SetDrawRectLine( true );
}

ToolTipPanel::~ToolTipPanel()
{

}

void ToolTipPanel::ClearInfo()
{

}

void ToolTipPanel::SetInfo( const CASHSHOP_ITEMLIST& data )
{
	if( m_ToolTipInfoText.size() != 0 ) return;

	ITEM tooltipitem;
	int sizeX = 0, sizeY = 0;

	m_ToolTipItemInfo     = data;
	ChangeData( &tooltipitem, m_ToolTipItemInfo, m_ToolTipInfos, m_ToolTipInfoColors );

	if( (tooltipitem.Type >= ITEM_HELPER+43 && tooltipitem.Type <= ITEM_HELPER+45)
#ifdef PSW_SEAL_ITEM
		|| ( tooltipitem.Type == ITEM_HELPER+59 ) // 이동의 인장
#endif //PSW_SEAL_ITEM
#ifdef PSW_SCROLL_ITEM
		|| ( tooltipitem.Type >= ITEM_POTION+72 && tooltipitem.Type <= ITEM_POTION+77 ) // 스크롤
#endif //PSW_SCROLL_ITEM
#ifdef 	PSW_ADD_PC4_SEALITEM
		|| ( tooltipitem.Type >= ITEM_HELPER+62 && tooltipitem.Type <= ITEM_HELPER+63 )
#endif //PSW_ADD_PC4_SEALITEM
#ifdef PSW_ADD_PC4_SCROLLITEM
		|| ( tooltipitem.Type >= ITEM_POTION+97 && tooltipitem.Type <= ITEM_POTION+98 )
#endif //PSW_ADD_PC4_SCROLLITEM
#ifdef YDG_ADD_HEALING_SCROLL
		|| ( tooltipitem.Type == ITEM_POTION+140 ) // 치유의 스크롤
#endif	// YDG_ADD_HEALING_SCROLL
#ifdef ASG_ADD_CS6_GUARD_CHARM
		|| tooltipitem.Type == ITEM_HELPER+81	// 수호의부적
#endif	// ASG_ADD_CS6_GUARD_CHARM
#ifdef ASG_ADD_CS6_ITEM_GUARD_CHARM
		|| tooltipitem.Type == ITEM_HELPER+82	// 아이템보호부적
#endif	// ASG_ADD_CS6_ITEM_GUARD_CHARM
#ifdef ASG_ADD_CS6_ASCENSION_SEAL_MASTER
		|| tooltipitem.Type == ITEM_HELPER+93	// 상승의인장마스터
#endif	// ASG_ADD_CS6_ASCENSION_SEAL_MASTER
#ifdef ASG_ADD_CS6_WEALTH_SEAL_MASTER
		|| tooltipitem.Type == ITEM_HELPER+94	// 풍요의인장마스터
#endif	// ASG_ADD_CS6_WEALTH_SEAL_MASTER
#ifdef YDG_ADD_CS7_PARTY_EXP_BONUS_ITEM
		|| tooltipitem.Type == ITEM_HELPER+103	// 파티경험치추가
#endif	// YDG_ADD_CS7_PARTY_EXP_BONUS_ITEM			
#ifdef YDG_ADD_CS7_MAX_AG_AURA
		|| tooltipitem.Type == ITEM_HELPER+104	// AG오라
#endif	// YDG_ADD_CS7_MAX_AG_AURA			
#ifdef YDG_ADD_CS7_MAX_SD_AURA
		|| tooltipitem.Type == ITEM_HELPER+105	// SD오라
#endif	// YDG_ADD_CS7_MAX_SD_AURA			
		)
	{
		string timetext;
		GetTime( data.s_itemTime, timetext, false );

		m_ToolTipInfos.push_back( timetext );
		m_ToolTipInfos.push_back( GlobalText[2302] );

		m_ToolTipInfoColors.push_back( TEXT_COLOR_PURPLE );
		m_ToolTipInfoColors.push_back( TEXT_COLOR_WHITE );
	}

	for( int i = 0; i < m_ToolTipInfos.size(); ++i )
	{
		image::ImageUnitPtr textback = image::ImageUnit::Make();
		m_TextBack.push_back( textback );

		string Temp = m_ToolTipInfos[i];

		TextPtr tooltipinfos( new Text( ( format( "tooltipinfos%1%" ) % i ).str() ) );
		tooltipinfos->Make( FONTSIZE_LANGUAGE, Temp.length() );
		tooltipinfos->SetAlign( TA_LEFT );
		tooltipinfos->SetContent( Temp );

		sizeX = Max( sizeX, tooltipinfos->GetSize().x );

		m_ToolTipInfoText.push_back( tooltipinfos );
	}

	if( m_ToolTipInfoText.size() != 0 ) 
	{
		sizeY = m_ToolTipInfos.size()*m_ToolTipInfoText[0]->GetSize().y+2;
	}

	m_TempPosY = 8;
	
	Resize( Dim( sizeX+14, sizeY+35 ) );
}

void ToolTipPanel::SetInfo( const vector<string>& data )
{
	if( m_ToolTipInfoText.size() != 0 ) return;

	int sizeX = 0, sizeY = 0, index = 0;

	for( int i = 0; i < data.size(); ++i )
	{
		image::ImageUnitPtr textback = image::ImageUnit::Make();
		m_TextBack.push_back( textback );

		string Temp = data[i];
		sizeX = Max( sizeX, Temp.length() );

		TextPtr tooltipinfos( new Text( ( format( "tooltipinfos%1%" ) % i ).str() ) );
		tooltipinfos->Make( FONTSIZE_LANGUAGE-2, Temp.length() );
		tooltipinfos->SetAlign( TA_LEFT );
		tooltipinfos->SetContent( Temp );
		m_ToolTipInfoText.push_back( tooltipinfos );

		m_ToolTipInfoColors.push_back( 0xffffffff );
	}

	if( m_ToolTipInfoText.size() != 0 ) 
	{
		sizeY = m_ToolTipInfos.size()*(m_ToolTipInfoText[0]->GetSize().y+2);
	}

	m_Image->SetDrawRectLine( false );

	Resize( Dim( (sizeX*7)-20, sizeY+15 ) );
}

void ToolTipPanel::SetInfo( const vector<string>& data, const vector<DWORD>& colordata )
{
	int sizeX = 0, sizeY = 0;

	for( int i = 0; i < data.size(); ++i )
	{
		image::ImageUnitPtr textback = image::ImageUnit::Make();
		m_TextBack.push_back( textback );

		string Temp = data[i];

		TextPtr tooltipinfos( new Text( ( format( "tooltipinfos%1%" ) % i ).str() ) );
		tooltipinfos->Make( FONTSIZE_LANGUAGE, Temp.length() );
		tooltipinfos->SetAlign( TA_LEFT );
		tooltipinfos->SetContent( Temp );
		m_ToolTipInfoText.push_back( tooltipinfos );

		sizeX = Max( sizeX, tooltipinfos->GetSize().x );

		m_ToolTipInfoColors.push_back( colordata[i] );
	}

	if( m_ToolTipInfoText.size() != 0 ) 
	{
		int aaaa = m_ToolTipInfoText.size();
		int bbbb = m_ToolTipInfoText[0]->GetSize().y;
		sizeY = aaaa*bbbb+2;
	}

	m_TempPosY = 8;
	
	Resize( Dim( sizeX+14, sizeY+35 ) );
}

void ToolTipPanel::Process( int delta )
{

}

void ToolTipPanel::DrawRect()
{
	if( !m_AngleStat ) 
		return;

	Coord Position  = GetScreenPosition();
	Dim	  Size      = GetSize();
	Dim windowsize  = TheShell().GetWindowSize();

	if( windowsize.x < Position.x + Size.x )
	{
		Position.x -= ((Position.x + Size.x) - windowsize.x)+10;
	}

	//틀을 빼고 계산한다.
	int mainframepos = windowsize.y-((480-48)*windowsize.y/480);
	if( windowsize.y - mainframepos < Position.y + Size.y )
	{
		Position.y -= ((Position.y+Size.y)-(windowsize.y-mainframepos))+10;
	}

	m_Image->SetPosition( Coord( Position.x, Position.y ) );
	m_Image->SetSize( Dim( Size.x, Size.y ) );
	m_Image->DrawRect( 0x99000000 );

	int index = 0;
	
	for( int i = 0; i < m_ToolTipInfoText.size(); ++i )
	{
		if( 0 >= i )
			index = m_ToolTipInfoText[i]->GetSize().y*i;
		else
			index = m_ToolTipInfoText[i]->GetSize().y*(i+1);

		Coord   textpos   = Position + Coord( (Size.x/2)-(m_ToolTipInfoText[i]->GetSize().x/2), (index+2)+m_TempPosY );

		m_TextBack[i]->SetPosition( Coord( Position.x, textpos.y-1 ) );
		m_TextBack[i]->SetSize( Dim( Size.x, m_ToolTipInfoText[i]->GetSize().y-1 ) );

		Color backcolor;
		GetBackColor( backcolor, m_ToolTipInfoColors[i] );
		m_TextBack[i]->DrawRect( backcolor );

		Color textcolor = GetColor(m_ToolTipInfoColors[i]);
		if ( TEXT_COLOR_GREEN_BLUE==m_ToolTipInfoColors[i])
		{
			textcolor = (255<<24)+(0<<16)+(255<<8)+(0);
		}

		m_ToolTipInfoText[i]->SetColor( textcolor );
		m_ToolTipInfoText[i]->Move( textpos );
		m_ToolTipInfoText[i]->DrawImage();
	}
}


#endif //NEW_USER_INTERFACE_UISYSTEM