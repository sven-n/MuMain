// w_CustomCheckButton.cpp: implementation of the CustomCheckButton class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "w_CustomCheckButton.h"

#ifdef NEW_USER_INTERFACE_UISYSTEM

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

using namespace ui;

namespace
{
	const int CHECKIMAGECOUNT = 2;
}

void CustomCheckButton::MakeUI( UISCRIPT& uiscriptdata, UIWnd& parentWnd )
{
	BoostSmart_Ptr(CustomCheckButton) checkbutton( new CustomCheckButton( uiscriptdata.s_UiName ) );
	checkbutton->MakeImage( uiscriptdata.s_UIImgIndex );
	checkbutton->SetID( uiscriptdata.s_UiID );
	checkbutton->Move( uiscriptdata.s_Position );
	checkbutton->Resize( uiscriptdata.s_Size );
	parentWnd.AddChild( checkbutton );
}

CustomCheckButton::CustomCheckButton( const string& uiName ) : CheckButton( uiName )
{
	Initialize();
	RegisterDrawEvent( eDraw_Image );
}

CustomCheckButton::~CustomCheckButton()
{
	Destroy();
}

void CustomCheckButton::Initialize()
{

}

void CustomCheckButton::Destroy()
{
	for ( int i = m_Image.size() - 1; i > -1; --i )
	{
		m_Image[i].reset();
	}

	m_Image.clear();
}

void CustomCheckButton::MakeImage( const int bitmapindex )
{
	for ( int i = 0; i < CHECKIMAGECOUNT; ++i )
	{
		m_Image.push_back( image::ImageUnit::Make() );
	}

	SetImageIndex( bitmapindex );

	ChangeFrame( static_cast<int>(eCheckButton_UnCheck) );
}

void CustomCheckButton::SetImageIndex( const int bitmapindex )
{
	for ( int i = 0; i < m_Image.size(); ++i )
	{
		m_Image[i]->SetImageIndex( bitmapindex + i );
	}
}

void CustomCheckButton::ChangeFrame( int index )
{
	for ( int i = 0; i < m_Image.size(); ++i )
	{
		if( i == index )
			m_Image[i]->SetDraw( true );
		else
			m_Image[i]->SetDraw( false );
	}
}

void CustomCheckButton::OnMove( const Coord& prevPos )
{
    CheckButton::OnMove( prevPos );

	for( int i = 0; i < m_Image.size(); ++i )
    {
        m_Image[i]->SetPosition( GetScreenPosition() );
    }
}

void CustomCheckButton::OnResize( const Dim& dim )
{
	CheckButton::OnResize( dim );

	for ( int i = 0; i < m_Image.size(); ++i )
	{
		m_Image[i]->SetSize( m_Size );
	}
}

void CustomCheckButton::OnMouseOver()
{
	CheckButton::OnMouseOver();
}

void CustomCheckButton::OnMouseOut()
{
	CheckButton::OnMouseOut();
}

void CustomCheckButton::OnCheck()
{
    CheckButton::OnCheck();
    ChangeFrame( static_cast<int>(eCheckButton_Check) );
}

void CustomCheckButton::OnUncheck()
{
    CheckButton::OnUncheck();
    ChangeFrame( static_cast<int>(eCheckButton_UnCheck) );
}

void CustomCheckButton::DrawImage()
{
	for ( int i = 0; i < m_Image.size(); ++i )
	{
		m_Image[i]->Draw();
	}
}

#endif //NEW_USER_INTERFACE_UISYSTEM