// w_MouseDevice.cpp: implementation of the MouseDevice class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "w_MouseDevice.h"
#include <windowsx.h>
#include <zmouse.h>

#ifdef NEW_USER_INTERFACE_INPUTSYSTEM

using namespace input;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

MouseDevicePtr MouseDevice::Make( IDirectInput8& dxinput, HWND windowHandle )
{
	MouseDevicePtr device( new MouseDevice( dxinput, windowHandle ) );
	return device;
}

MouseDevice::MouseDevice( IDirectInput8& dxinput, HWND windowHandle ) : m_WindowHandle( windowHandle ), m_IsActive( true )
#ifdef DBCLK
,m_IsLbuttonDown( false ), m_IsRbuttonDown( false ), m_IsLbuttonDownUp( false ), m_IsRbuttonDownUp( false )
,m_IsLbuttonDBclk( false ), m_IsRbuttonDBclk( false ), m_StatrtTime( 0 )
#endif //DBCLK
{
	Initialize( windowHandle );
}

MouseDevice::~MouseDevice()
{
	Destroy();
}

void MouseDevice::Initialize( HWND windowHandle )
{

}

void MouseDevice::Destroy()
{

}

void MouseDevice::Reset()
{

}

void MouseDevice::Process( int delta )
{
	if( !m_Acquired ) return;

#ifdef DBCLK

	if( m_IsLbuttonDownUp )
	{
		if( m_IsLbuttonDBclk )
		{
			TheInputProxy().HandleMouseMessage( eKey_LButton, eKeyState_DblClk, m_PosValue );
			m_IsLbuttonDBclk = false;
		}
		else if( 100 <= timeGetTime()-m_StatrtTime )
		{
			TheInputProxy().HandleMouseMessage( eKey_LButton, eKeyState_Up, m_PosValue );
			m_IsLbuttonDBclk = m_IsLbuttonDownUp = m_IsLbuttonDown = false;
			m_StatrtTime = 0;
		}
	}

	if( m_IsRbuttonDownUp )
	{
		if( m_IsRbuttonDBclk )
		{
			TheInputProxy().HandleMouseMessage( eKey_RButton, eKeyState_DblClk, m_PosValue );
			m_IsRbuttonDBclk = false;
		}
		else if( 100 <= timeGetTime()-m_StatrtTime )
		{
			TheInputProxy().HandleMouseMessage( eKey_RButton, eKeyState_Up, m_PosValue );
			m_IsRbuttonDBclk = m_IsRbuttonDownUp = m_IsRbuttonDown = false;
			m_StatrtTime = 0;
		}
	}

#endif //DBCLK

    POINT screenPt;
    GetCursorPos( &screenPt );
    POINT wndPt = screenPt;
    ScreenToClient( m_WindowHandle, &wndPt );

    const int limit = 8;
/*
    if( wndPt.x < limit )
    {
        wndPt.x = limit;
    }
    else if( wndPt.x > m_WindowSize.x - limit )
    {
        wndPt.x = m_WindowSize.x - limit;
    }

    if( wndPt.y < limit )
    {
        wndPt.y = limit;
    }
    else if( wndPt.y > m_WindowSize.y - limit )
    {
        wndPt.y = m_WindowSize.y - limit;
    }
*/
    screenPt = wndPt;
    ClientToScreen( m_WindowHandle, &screenPt );

    SetCursorPos( screenPt.x, screenPt.y );
    m_Position.x = screenPt.x;
    m_Position.y = screenPt.x;
}

void MouseDevice::Active( bool b )
{
	m_IsActive = b;
}

bool MouseDevice::HandleWindowMessage( UINT message, WPARAM wParam, LPARAM lParam, LRESULT& result )
{
    if( m_IsActive == false )
        return false;

    switch( message )
    {
    case WM_LBUTTONDOWN:
		{
#ifdef DBCLK
			m_IsLbuttonDown = true;
#endif //DBCLK
			TheInputProxy().HandleMouseMessage( eKey_LButton, eKeyState_Down, m_PosValue );
		}
        return true;
    
    case WM_LBUTTONUP:
		{
#ifdef DBCLK
			m_StatrtTime    = timeGetTime();
			if( m_IsLbuttonDown ) m_IsLbuttonDownUp = true;
			m_IsLbuttonDown = false;
#else //DBCLK
			TheInputProxy().HandleMouseMessage( eKey_LButton, eKeyState_Up, m_PosValue );
#endif //DBCLK
		}
        return true;

    case WM_LBUTTONDBLCLK:
        {
#ifdef DBCLK
			m_IsLbuttonDBclk = true;
#else //DBCLK
			TheInputProxy().HandleMouseMessage( eKey_LButton, eKeyState_DblClk, m_PosValue );
#endif //DBCLK
		}   
        return true;

    case WM_RBUTTONDOWN:
		{
#ifdef DBCLK
			m_IsRbuttonDown = true;
#endif //DBCLK
			TheInputProxy().HandleMouseMessage( eKey_RButton, eKeyState_Down, m_PosValue );
		}
        return true;

    case WM_RBUTTONUP:
		{
#ifdef DBCLK
			m_StatrtTime    = timeGetTime();
			if( m_IsRbuttonDown ) m_IsRbuttonDownUp = true;
			m_IsRbuttonDown = false;
#else //DBCLK
			TheInputProxy().HandleMouseMessage( eKey_RButton, eKeyState_Up, m_PosValue );
#endif //DBCLK
		}
        return true;

    case WM_RBUTTONDBLCLK:
		{
#ifdef DBCLK
			m_IsRbuttonDBclk = true;
#else //DBCLK
			TheInputProxy().HandleMouseMessage( eKey_RButton, eKeyState_DblClk, m_PosValue );
#endif //DBCLK
		}
        return true;

    case WM_MBUTTONDOWN:
        TheInputProxy().HandleMouseMessage( eKey_MButton, eKeyState_Down, m_PosValue );
        return true;

    case WM_MBUTTONUP:
        TheInputProxy().HandleMouseMessage( eKey_MButton, eKeyState_Up, m_PosValue );
        return true;

    case WM_MBUTTONDBLCLK:
        TheInputProxy().HandleMouseMessage( eKey_MButton, eKeyState_DblClk, m_PosValue );
        return true;

    case WM_MOUSEWHEEL:
        TheInputProxy().HandleMouseMessage( eKey_Wheel, eKeyState_None, (int)(short)HIWORD( wParam ) );
        return true;

    case WM_MOUSEMOVE:
        {
            m_Position.x = GET_X_LPARAM( lParam );
            m_Position.y = GET_Y_LPARAM( lParam );
			
            const int limit = 8;

            bool changed = false;
/*
            if( m_Position.x < limit )
            {
                changed = true;
                m_Position.x = limit;
            }
            else if( m_Position.x > m_WindowSize.x - limit )
            {
                changed = true;
                m_Position.x = m_WindowSize.x - limit;
            }

            if( m_Position.y < limit )
            {
                changed = true;
                m_Position.y = limit;
            }
            else if( m_Position.y > m_WindowSize.y - limit )
            {
                changed = true;
                m_Position.y = m_WindowSize.y - limit;
            }
*/
            m_PosValue = m_Position.y | ( m_Position.x << 16 );
            TheInputProxy().HandleMouseMessage( eKey_MouseMove, eKeyState_None, m_PosValue );
        }
        return true;

    case WM_NCACTIVATE:
        if( m_Acquired == false )
        {
            Acquire();
        }
        return false;

    case WM_ACTIVATE:
        if( wParam != WA_INACTIVE )
        {
            Acquire();
        }
        else
        {
            Unacquire();
        }
        return false;
    }
    return false;
}

void MouseDevice::Acquire()
{
    m_Acquired = true;
}

void MouseDevice::Unacquire()
{
    m_Acquired = false;
}

#endif //NEW_USER_INTERFACE_INPUTSYSTEM
