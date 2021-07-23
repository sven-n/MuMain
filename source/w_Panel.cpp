// w_Panel.cpp: implementation of the Panel class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "w_Panel.h"

#ifdef NEW_USER_INTERFACE_UISYSTEM

using namespace ui;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

Panel::Panel( const string& name ) : UIWnd( name )
{
	m_Image = image::ImageUnit::Make();
	RegisterDrawEvent( eDraw_Image );
}

Panel::~Panel()
{
	Destroy();
}

void Panel::Initialize()
{

}

void Panel::Destroy()
{
	m_Image.reset();
}

void Panel::SetColor( const Color& color )
{
	m_Color             = color;
}

void Panel::DrawImage()
{
	if( m_Image ) m_Image->DrawRect( m_Color );
}

void Panel::OnChangeOpacity( BYTE prevOp )
{
	UIWnd::OnChangeOpacity( prevOp );
	m_Color.SetAlpha( GetOpacity() );
}

void Panel::OnMove( const Coord& prevPos )
{
	UIWnd::OnMove( prevPos );
	m_Image->SetPosition( GetScreenPosition() );
}

void Panel::OnResize( const Dim& prevSize )
{
	UIWnd::OnResize( prevSize );
	m_Image->SetSize( m_Size );
}


#endif //NEW_USER_INTERFACE_UISYSTEM

