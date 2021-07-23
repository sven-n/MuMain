// CustomButton.cpp: implementation of the CustomButton class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "w_CustomButton.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#ifdef NEW_USER_INTERFACE_UISYSTEM

#include "w_Text.h"

using namespace ui;

CustomButton::CustomButton( const string& uiName ) : UIWnd( uiName )
{
	Initialize();
	RegisterDrawEvent( eDraw_Image );
}

CustomButton::~CustomButton()
{
	Destroy();
}

void CustomButton::Initialize()
{

}

void CustomButton::Destroy()
{
	m_ButtonImage.clear();
}

void CustomButton::MakeImage( const int bitmapindex )
{
	for ( int i = 0; i < eButton_StateCount; ++i )
	{
		BoostSmart_Ptr(ButtonImage) buttonima( new ButtonImage );
		m_ButtonImage.push_back( buttonima );
	}

	SetImageIndex( bitmapindex );

	ChangeFrame( static_cast<int>(eButton_Up) );
}

void CustomButton::SetImageIndex( const int bitmapindex )
{
	for ( int i = 0; i < m_ButtonImage.size(); ++i )
	{
		m_ButtonImage[i]->SetImageIndex( bitmapindex + i );
	}
}

void CustomButton::SetCustomButtonName( const string& buttonname, const Coord& pos, BYTE type )
{
	TextPtr custombuttonname( new Text( "custombuttonname" ) );
	custombuttonname->Make( FONTSIZE_LANGUAGE, buttonname.length() );
	custombuttonname->SetAlign( type );
	custombuttonname->SetContent( buttonname );

	Coord TextPos = Coord( (GetSize().x/2)-(custombuttonname->GetSize().x/2), 
						  (GetSize().y/2)-((custombuttonname->GetSize().y)/2) );

	if( pos != Coord( 0, 0 ) )
		TextPos.y = pos.y;

	custombuttonname->Move( TextPos );
	custombuttonname->EnableLButtonEvent( false );
	AddChild( custombuttonname );
}

void CustomButton::ChangeFrame( int index )
{
	for( int j = 0; j < m_ButtonImage.size(); ++j )
	{
		if( j == index )
			m_ButtonImage[j]->SetDraw( true );
		else
			m_ButtonImage[j]->SetDraw( false );
	}
}

void CustomButton::OnMove( const Coord& prevPos )
{
	UIWnd::OnMove( prevPos );

	for( int i = 0; i < m_ButtonImage.size(); ++i )
	{
		m_ButtonImage[i]->SetPosition( GetScreenPosition() );	
	}
}

void CustomButton::OnResize( const Dim& dim )
{
	UIWnd::OnResize( dim );

	for( int i = 0; i < m_ButtonImage.size(); ++i )
	{
		m_ButtonImage[i]->SetSize( m_Size );	
	}
}

void CustomButton::OnMouseOver()
{
	UIWnd::OnMouseOver();
	ChangeFrame( static_cast<int>(eButton_Over) );
}

void CustomButton::OnMouseOut()
{
	UIWnd::OnMouseOut();
	ChangeFrame( static_cast<int>(eButton_Up) );
}

void CustomButton::OnLButtonDown()
{
	UIWnd::OnLButtonDown();
	ChangeFrame( static_cast<int>(eButton_Down) );
}

void CustomButton::OnLButtonUp()
{
	UIWnd::OnLButtonUp();
	ChangeFrame( static_cast<int>(eButton_Up) );
}

void CustomButton::DrawImage()
{
	for( int j = 0; j < m_ButtonImage.size(); ++j )
	{
		m_ButtonImage[j]->Draw();
	}
}

#endif //NEW_USER_INTERFACE_UISYSTEM