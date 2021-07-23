// w_UIWnd.cpp: implementation of the UIWnd class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "w_UIType.h"
#include "w_UIWnd.h"

#ifdef NEW_USER_INTERFACE_UISYSTEM

using namespace ui;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

UIWnd::UIWnd( const string& uiName ) : m_Name( uiName ), m_ID( 0 ),
m_Opacity( 0xff ), m_LockedOpacity( false ), m_Activate( true ), m_MouseOvered( false ), m_Focused( false )
{
	Initialize();
}

UIWnd::~UIWnd()
{
	Destroy();
}

void UIWnd::Initialize()
{
	int i = 0;

	m_ScreenPosition = BoostSmart_Ptr(sScreenPosition)( new sScreenPosition );

	for( i = 0; i < m_EnableEvents.size(); i++ )
    {
        m_EnableEvents[i] = true;
    }

    for( i = 0; i < eEventTypeMax; i++ )
    {
        m_Eventsarray[i].reset();
    }
}

void UIWnd::Destroy()
{
	ClearDrawEvent();
    ClearChild();
    OnDestory();
}

void UIWnd::EnableEvent( eEventType type, bool b )
{
    m_EnableEvents[type] = b;
}

void UIWnd::EnableEvents( vector<eEventType> types, bool b )
{
    for( unsigned int i = 0; i < types.size(); i++ )
    {
        m_EnableEvents[ types[i] ] = b;
    }
}

void UIWnd::EnableLButtonEvent( bool b )
{
    m_EnableEvents[eEvent_LButtonUp] = b;
    m_EnableEvents[eEvent_LButtonDown] = b;
    m_EnableEvents[eEvent_LButtonClick] = b;
    m_EnableEvents[eEvent_LButtonDbClk] = b;
}

void UIWnd::EnableRButtonEvent( bool b )
{
    m_EnableEvents[eEvent_RButtonUp] = b;
    m_EnableEvents[eEvent_RButtonDown] = b;
    m_EnableEvents[eEvent_RButtonClick] = b;
    m_EnableEvents[eEvent_RButtonDbClk] = b;
}

void UIWnd::Activate( bool b )
{
    m_Activate = b;
}

bool UIWnd::IsActivated() const
{
    return m_Activate;
}

void UIWnd::ChangeOpacity( BYTE b, bool bRecursive )
{
	if( m_LockedOpacity == false )
	{
		BYTE prev = m_Opacity;
		m_Opacity = b;
		OnChangeOpacity( prev );
	}

	if( bRecursive )
	{
		for( ChildUIWnd_List::iterator iter = m_Childlist.begin()
			; iter != m_Childlist.end(); ++iter )
		{
			UIWnd& wnd = **iter;
			wnd.ChangeOpacity( b, bRecursive );
		}
	}
}

void UIWnd::ChangeOpacityByPercent( float f, bool bRecursive )
{
	BYTE b = (float)(0xff) * f;
    ChangeOpacity( b, bRecursive );
}

BYTE UIWnd::GetOpacity() const
{
	return m_Opacity;
}

void UIWnd::LockOpacity( bool b )
{
	m_LockedOpacity = b;
}

bool UIWnd::IsFocused() const
{
	return m_Focused;
}

bool UIWnd::IsMouseOvered() const
{
	return m_MouseOvered;
}

void UIWnd::Move( const Coord& pos )
{
    Coord prev = m_Position;
    m_Position = pos;
    CalculateScreenPosition();
    OnMove( prev );
}

void UIWnd::Resize( const Dim& size )
{
    Dim prev = m_Size;
    m_Size = size;
	CalculateScreenPosition();
    OnResize( prev );
}

void UIWnd::CalculateScreenPosition()
{
    if( !m_ScreenPosition->parentPos.expired() )
    {
        m_ScreenPosition->pos = m_ScreenPosition->parentPos.lock()->pos + m_Position;
        OnMove( m_Position );
    }
    else
        m_ScreenPosition->pos = m_Position;

    for( ChildUIWnd_List::iterator iter = m_Childlist.begin()
       ; iter != m_Childlist.end(); ++iter )
    {
        UIWnd& wnd = **iter;
        wnd.CalculateScreenPosition();
    }
}

void UIWnd::AddChild( BoostSmart_Ptr(UIWnd) childWnd )
{
    assert( childWnd );
    m_Childlist.push_back( childWnd );
    OnAddChild( childWnd->GetName() );

	childWnd->m_ScreenPosition->parentPos = m_ScreenPosition;
    childWnd->CalculateScreenPosition();
    OnAddChild( childWnd->GetName() );
}

void UIWnd::SudChild( const string& wndName )
{
	assert( IsChild( wndName ) );

	if( !IsChild( wndName ) ) return;
    OnSubChild( wndName );

    for( ChildUIWnd_List::iterator iter = m_Childlist.begin()
        ; iter != m_Childlist.end(); ++iter )
    {
        UIWnd& wnd = **iter;
        if( wnd.GetName() == wndName )
        {
            m_Childlist.erase( iter );
            return;
        }
    }
}

void UIWnd::SubChild( int id )
{
    OnSubChild( id );

    for( ChildUIWnd_List::iterator iter = m_Childlist.begin()
        ; iter != m_Childlist.end(); ++iter )
    {
        UIWnd& wnd = **iter;
        if( wnd.GetID() == id )
        {
            m_Childlist.erase( iter );
            return;
        }
    }
}

void UIWnd::ClearChild()
{
	OnClearChild();
    for( ChildUIWnd_List::iterator iter = m_Childlist.begin(); iter !=  m_Childlist.end(); ++iter )
    {
        BoostSmart_Ptr(UIWnd) wnd = *iter;
        wnd->Destroy();
    }
    m_Childlist.clear();
}

UIWnd& UIWnd::GetChild( const string& name )
{
	for( ChildUIWnd_List::iterator iter = m_Childlist.begin()
        ; iter != m_Childlist.end(); ++iter )
    {
        UIWnd& wnd = **iter;
        if( wnd.GetName() == name )
            return wnd;
    }
    assert( 0 );
    throw;

}

UIWnd& UIWnd::GetChild( int id )
{
    for( ChildUIWnd_List::iterator iter = m_Childlist.begin()
       ; iter != m_Childlist.end(); ++iter )
    {
        UIWnd& wnd = **iter;
        if( wnd.GetID() == id )
            return wnd;
    }
    assert( 0 );
    throw;
}

bool UIWnd::IsChild( const string& name ) const
{
	for( ChildUIWnd_List::const_iterator iter = m_Childlist.begin()
        ; iter != m_Childlist.end(); ++iter )
    {
        UIWnd& wnd = **iter;
        if( wnd.GetName() == name )
            return true;
    }
    return false;
}

bool UIWnd::IsChild( int id ) const
{
	for( ChildUIWnd_List::const_iterator iter = m_Childlist.begin()
        ; iter != m_Childlist.end(); ++iter )
    {
        UIWnd& wnd = **iter;
        if( wnd.GetID() == id )
            return true;
    }
    return false;
}

void UIWnd::NotifyToHandler( eEventType type )
{
	if( m_Eventsarray[type] )
	{
		//자기 자신 포인트도 같이 넘겨 준다.
		( *m_Eventsarray[type] )( *this );
	}
}

bool UIWnd::IsDrawEvent( eDrawType drawtype )
{
	for( DrawEvent_List::iterator iter = m_DrawEventslist.begin()
        ; iter != m_DrawEventslist.end(); ++iter )
    {
        eDrawType& type = *iter;
        if( type == drawtype )
        {
            return true;
        }
    }

	return false;
}

void UIWnd::UnRegisterDrawEvent( eDrawType drawtype )
{
	for( DrawEvent_List::iterator iter = m_DrawEventslist.begin()
        ; iter != m_DrawEventslist.end(); ++iter )
    {
        eDrawType& type = *iter;
        if( type == drawtype )
        {
            m_DrawEventslist.erase( iter );
            return;
        }
    }
}

/////////////////////////////////////////////////////Event Fun/////////////////////////////////////////////////////

void UIWnd::OnInitialize()
{
	//NotifyToHandler( eEvent_ScrollButtonDown );
}

void UIWnd::OnDestory()
{
	//NotifyToHandler( eEvent_ScrollButtonDown );
}

void UIWnd::OnFocus()
{
	NotifyToHandler( eEvent_Focus );
}

void UIWnd::OnKillFocus()
{
	NotifyToHandler( eEvent_KillFocus );
}

void UIWnd::OnMouseMove( const Coord& pos )
{
	//NotifyToHandler( eEvent_ScrollButtonDown );
}

void UIWnd::OnMouseOver()
{
	NotifyToHandler( eEvent_MouseOver );
}

void UIWnd::OnMouseOut()
{
	NotifyToHandler( eEvent_MouseOut );
}

void UIWnd::OnLButtonDown()
{
	NotifyToHandler( eEvent_LButtonDown );
}

void UIWnd::OnLButtonUp()
{
	NotifyToHandler( eEvent_LButtonUp );
}

void UIWnd::OnLButtonClk()
{
	NotifyToHandler( eEvent_LButtonClick );
}

void UIWnd::OnLButtonDBClk()
{
	NotifyToHandler( eEvent_LButtonDbClk );
}

void UIWnd::OnRButtonDown()
{
	NotifyToHandler( eEvent_RButtonDown );
}

void UIWnd::OnRButtonUp()
{
	NotifyToHandler( eEvent_RButtonUp );
}

void UIWnd::OnRButtonClk()
{
	NotifyToHandler( eEvent_RButtonClick );
}

void UIWnd::OnRButtonDBClk()
{
	NotifyToHandler( eEvent_RButtonDbClk );
}

void UIWnd::OnWheelScroll( int delta )
{
	NotifyToHandler( eEvent_Scroll );
}

void UIWnd::OnWheelButtonDown( int delta )
{
	//NotifyToHandler( eEvent_ScrollButtonDown );
}

void UIWnd::OnWheelButtonUp( int delta )
{
	//NotifyToHandler( eEvent_ScrollButtonDown );
}

void UIWnd::OnWheelButtonClk( int delta )
{
	//NotifyToHandler( eEvent_ScrollButtonDown );
}

void UIWnd::OnWheelButtonDBClk( int delta )
{
	//NotifyToHandler( eEvent_ScrollButtonDown );
}

void UIWnd::OnTab()
{
	NotifyToHandler( eEvent_Tab );
}

void UIWnd::OnEnter()
{
	NotifyToHandler( eEvent_Enter );
}

void UIWnd::OnEscape()
{
	NotifyToHandler( eEvent_Escape );
}

void UIWnd::OnAddChild( const string& childName )
{
	//NotifyToHandler( eEvent_ScrollButtonDown );
}

void UIWnd::OnSubChild( const string& childName )
{
	//NotifyToHandler( eEvent_ScrollButtonDown );
}

void UIWnd::OnSubChild( int id )
{
	//NotifyToHandler( eEvent_ScrollButtonDown );
}

void UIWnd::OnClearChild()
{
	//NotifyToHandler( eEvent_ScrollButtonDown );
}

void UIWnd::OnChangeOpacity( BYTE prevOp )
{
	//NotifyToHandler( eEvent_ScrollButtonDown );
}

void UIWnd::OnMove( const Coord& pos )
{
	NotifyToHandler( eEvent_Move );
}

void UIWnd::OnResize( const Dim& dim )
{
	NotifyToHandler( eEvent_Resize );
}

void UIWnd::DrawImage()
{

}

void UIWnd::DrawMesh()
{

}

void UIWnd::DrawViewPortMesh()
{

}

void UIWnd::DrawRect()
{

}

void UIWnd::Process( int delta )
{

}

#endif //NEW_USER_INTERFACE_UISYSTEM