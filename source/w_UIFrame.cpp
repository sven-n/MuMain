// w_UIFrame.cpp: implementation of the UIFrame class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "w_UIFrame.h"

#ifdef NEW_USER_INTERFACE_UISYSTEM

#include "ZzzOpenglUtil.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

using namespace ui;

UIFramePtr UIFrame::Make( const string& uiwndName, bool isModal )
{
	UIFramePtr frame( new UIFrame( uiwndName, isModal ) );
	return frame;
}

UIFrame::UIFrame( const string& uiwndName, bool isModal ) : UIWnd( uiwndName ), m_IsModal( isModal ), m_UISystem( NULL )
{

}

UIFrame::~UIFrame()
{

}

bool UIFrame::IsModal() const
{
	return m_IsModal;
}

void UIFrame::Focus()
{
	assert( m_UISystem );
    m_UISystem->Focus( *this );
}

void UIFrame::SetSystem( UISystem& system )
{
	m_UISystem = &system;
}

UIWnd& UIFrame::QueryChild( vector<string> childname )
{
	UIWnd& wnd = GetChild( childname[0] );

	for( int i = 1;i < childname.size(); ++i )
	{
		if( i == childname.size() ) break;
		wnd = wnd.GetChild( childname[i] );
	}

	return wnd;
}

void UIFrame::Process( int delta )
{
	if( IsActivated() == false )
		return;
	
	for( UIWnd::ChildUIWnd_List::iterator iter = m_Childlist.begin()
		; iter != m_Childlist.end(); ++iter )
	{
		UIWnd& childWnd = *(*iter);
		Process( childWnd, delta );
	}
}

void UIFrame::Process( UIWnd& wnd, int delta )
{
	if( wnd.IsActivated() == false )
		return;

	wnd.Process( delta );

	for( UIWnd::ChildUIWnd_List::iterator iter = wnd.m_Childlist.begin()
		; iter != wnd.m_Childlist.end(); ++iter )
	{
		UIWnd& childWnd = *(*iter);
		Process( childWnd, delta );
	}
}

void UIFrame::Draw()
{
	if( IsActivated() == false ) {
		return;
	}

	//ㅜ..ㅜ 이렇게 꼭 해야 하나..--;;;
	//좀 더 명확하게 해 보자
	for( UIWnd::ChildUIWnd_List::iterator iter = m_Childlist.begin()
		; iter != m_Childlist.end(); ++iter ) {
		UIWnd& childWnd = *(*iter);

		BeginBitmap();
		Draw( childWnd, eDraw_Image );
		EndBitmap();

		BeginDrawMesh();
		Draw( childWnd, eDraw_Mesh );
		EndDrawMesh();

		Draw( childWnd, eDraw_ViewPortMesh );

		BeginBitmap();
		Draw( childWnd, eDraw_Rect );
		EndBitmap();
	}
}

void UIFrame::Draw( UIWnd& wnd, eDrawType drawtype )
{
	if( wnd.IsActivated() == false ) {
		return;
	}

	if( wnd.IsDrawEvent(drawtype) ) {
		switch( drawtype )
		{
		case eDraw_Image:            wnd.DrawImage();        break;
		case eDraw_Mesh:			 wnd.DrawMesh();		 break;
		case eDraw_ViewPortMesh:     wnd.DrawViewPortMesh(); break;
		case eDraw_Rect:			 wnd.DrawRect();         break;
		}
	}
	
#ifdef _DEBUG
	if( TheUISystem().IsDebugKey() ) {
		BeginBitmap();
		EnableAlphaTest();
		DisableTexture();
	
		DRAWRECT( Coord( wnd.GetScreenPosition().x, TheShell().GetWindowSize().y - wnd.GetScreenPosition().y ), 
					Coord( wnd.GetSize().x, wnd.GetSize().y), Color( 0xffff0000 ) );

		EndBitmap();

		if( drawtype == eDraw_Mesh )  {
			BeginDrawMesh();
		}
	}
#endif //_DEBUG	

	for( UIWnd::ChildUIWnd_List::iterator iter = wnd.m_Childlist.begin()
		; iter != wnd.m_Childlist.end(); ++iter ) {
		UIWnd& childWnd = *(*iter);
		Draw( childWnd, drawtype );
	}
}

#endif //NEW_USER_INTERFACE_UISYSTEM