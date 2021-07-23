// w_CustomRadioButton.cpp: implementation of the CustomRadioButton class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "w_CustomRadioButton.h"

#ifdef NEW_USER_INTERFACE_UISYSTEM

#include "w_Text.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

using namespace ui;

CustomRadioButton::CustomRadioButton( const string& name ) : RadioButton( name )
{
	Initialize();
	RegisterDrawEvent( eDraw_Image );
}

CustomRadioButton::~CustomRadioButton()
{
	Destroy();
}

void CustomRadioButton::Initialize()
{

}

void CustomRadioButton::Destroy()
{
	m_RadioButtonImage.clear();
}

void CustomRadioButton::MakeImage( const int bitmapindex )
{
	for ( int i = 0; i < eRadioButton_StateCount; ++i )
	{
		BoostSmart_Ptr(RadioButtonImage) buttonima( new RadioButtonImage );
		m_RadioButtonImage.push_back( buttonima );
	}

	SetImageIndex( bitmapindex );

	ChangeFrame( static_cast<int>(eRadioButton_Up) );
}

void CustomRadioButton::SetImageIndex( const int bitmapindex )
{
	for ( int i = 0; i < m_RadioButtonImage.size(); ++i )
	{
		m_RadioButtonImage[i]->SetImageIndex( bitmapindex + i );
	}
}

void CustomRadioButton::SetRadioButtonName( const string& buttonname, const Coord& pos, BYTE type )
{
	TextPtr radiobuttonname( new Text( "radiobuttonname" ) );
	radiobuttonname->Make( FONTSIZE_LANGUAGE, buttonname.length() );
	radiobuttonname->SetAlign( type );
	radiobuttonname->SetContent( buttonname );

	Coord TextPos = Coord( (GetSize().x/2)-(radiobuttonname->GetSize().x/2), 
						  (GetSize().y/2)-(radiobuttonname->GetSize().y/2) );

	if( pos != Coord( 0, 0 ) )
		TextPos.y = pos.y;

	radiobuttonname->Move( TextPos );
	radiobuttonname->EnableLButtonEvent( false );
	AddChild( radiobuttonname );
}

void CustomRadioButton::BasicChangeFrame()
{
	RadioButton::Select();
	ChangeFrame( static_cast<int>(eRadioButton_Down) );
}

void CustomRadioButton::UpChangeFrame()
{
	RadioButton::OnDeselect();
	ChangeFrame( static_cast<int>(eRadioButton_Up) );
}

void CustomRadioButton::ChangeFrame( int index )
{
	for( int j = 0; j < m_RadioButtonImage.size(); ++j )
	{
		if( j == index )
			m_RadioButtonImage[j]->SetDraw( true );
		else
			m_RadioButtonImage[j]->SetDraw( false );
	}
}

void CustomRadioButton::DrawImage()
{
	for( int j = 0; j < m_RadioButtonImage.size(); ++j )
	{
		m_RadioButtonImage[j]->Draw();
	}
}

void CustomRadioButton::OnMouseOver()
{
	RadioButton::OnMouseOver();
}

void CustomRadioButton::OnMouseOut()
{
	RadioButton::OnMouseOut();
}

void CustomRadioButton::OnMove( const Coord& pos )
{
	RadioButton::OnMove( pos );

	for( int i = 0; i < m_RadioButtonImage.size(); ++i )
	{
		m_RadioButtonImage[i]->SetPosition( GetScreenPosition() );
	}
}

void CustomRadioButton::OnResize( const Dim& dim )
{
	RadioButton::OnResize( dim );

	for( int i = 0; i < m_RadioButtonImage.size(); ++i )
	{
		m_RadioButtonImage[i]->SetSize( m_Size );
	}
}

void CustomRadioButton::OnSelect()
{
	RadioButton::OnSelect();
	ChangeFrame( static_cast<int>(eRadioButton_Down) );
}

void CustomRadioButton::OnDeselect()
{
	RadioButton::OnDeselect();
	ChangeFrame( static_cast<int>(eRadioButton_Up) );
}

#endif //NEW_USER_INTERFACE_UISYSTEM
