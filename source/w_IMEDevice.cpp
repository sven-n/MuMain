// w_IMEDevice.cpp: implementation of the IMEDevice class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "w_IMEDevice.h"
#include <ime.h>
#include <imm.h>

#ifdef NEW_USER_INTERFACE_INPUTSYSTEM

using namespace input;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IMEDevicePtr IMEDevice::Make( IDirectInput8& dxinput, HWND windowHandle )
{
	IMEDevicePtr device( new IMEDevice( dxinput, windowHandle ) );
	return device;
}

IMEDevice::IMEDevice( IDirectInput8& dxinput, HWND windowHandle ) : m_WindowHandle( windowHandle ), 
m_IsActive( true ), m_IsCompositing( false ), m_Shvsleri( false ), m_Property( 0 )
{
	Initialize();
}

IMEDevice::~IMEDevice()
{
	Destroy();
}

void IMEDevice::Initialize()
{	
	m_KeyLayout = GetKeyboardLayout(0);
	m_Property  = ImmGetProperty( m_KeyLayout, IGP_PROPERTY );

	Reset();
}

void IMEDevice::Destroy()
{

}

void IMEDevice::Active( bool b )
{
	m_IsActive = b;
	Reset();
}

void IMEDevice::Reset()
{
	m_IsCompositing = false;

	if( CaptureContext() )
    {
        ImmNotifyIME( m_ImeContext, NI_COMPOSITIONSTR, CPS_CANCEL, 0 );
        ReleaseContext();
    }
}

void IMEDevice::Process( int delta )
{

}

bool IMEDevice::HandleWindowMessage( UINT message, WPARAM wParam, LPARAM lParam, LRESULT& result )
{
    if( m_IsActive == false ) return false;

    switch( message )
    {
    case WM_IME_STARTCOMPOSITION:
        OnStartComposition( wParam, lParam );
        return true;

    case WM_IME_ENDCOMPOSITION:
        OnEndComposition( wParam, lParam );
        return true;

    case WM_IME_COMPOSITION:
        OnComposition( wParam, lParam );
        return true;

    case WM_IME_SETCONTEXT:
        OnSetContext( wParam, lParam, result );
        return true;

    case WM_INPUTLANGCHANGE:
        OnInputLangChange( wParam, lParam );
        return true;

    case WM_KEYDOWN:
            OnChar( static_cast<UINT>( wParam | 128 ) );
        return true;
	case WM_KEYUP:
		{
			OnCharUP(static_cast<UINT>( wParam | 128 ) );
		}
		return true;
    case WM_CHAR:
        OnChar( static_cast<UINT>( wParam ) );
        return true;
    }
    return false;
}

bool IMEDevice::MayDoComposition() const
{
	if( m_Property & IME_PROP_SPECIAL_UI )
		return false;

	if( m_Property & IME_PROP_AT_CARET )
		return true;

	return false;
}

void IMEDevice::OnStartComposition( WPARAM command, LPARAM data )
{
	if( MayDoComposition() )
	{
		m_IsCompositing = true;
		TheInputProxy().HandleIMEMessage( eIMEMessage_StartComposition );
	}
}

void IMEDevice::OnEndComposition( WPARAM command, LPARAM data )
{
    if( MayDoComposition() )
    {
        m_IsCompositing = false;
        TheInputProxy().HandleIMEMessage( eIMEMessage_EndComposition );
    }
}

void IMEDevice::ReportResultString()
{
    if( CaptureContext() == false )
        return;

    DWORD len = ImmGetCompositionString( m_ImeContext, GCS_RESULTSTR, NULL, 0 );
    if( len > 0 )
    {
        assert( len < 1024 );
        char buffer[1024];
        ImmGetCompositionString( m_ImeContext, GCS_RESULTSTR, buffer, len );
        buffer[len] = NULL;
        TheInputProxy().HandleIMEMessage( eIMEMessage_Complete, buffer );
    }

    ReleaseContext();
}

void IMEDevice::ReportCompositionString( LONG flag )
{
    if( CaptureContext() == false )
        return;

    DWORD len = ImmGetCompositionString( m_ImeContext, GCS_COMPSTR, NULL, 0 );
    if( len > 0 )
    {
        assert( len < 1024 );
        char buffer[1024];
        ImmGetCompositionString( m_ImeContext, GCS_COMPSTR, buffer, len );
        buffer[len] = NULL;
        TheInputProxy().HandleIMEMessage( eIMEMessage_Composition, buffer );
    }
    ReleaseContext();
}

void IMEDevice::OnComposition( WPARAM command, LPARAM data )
{
    if( MayDoComposition() )
    {
        if( data & GCS_RESULTSTR )
        {
            ReportResultString();
        }
        else if( data & GCS_COMPSTR )
        {
            ReportCompositionString( static_cast<LONG>( data ) );
        }
    }
}

void IMEDevice::OnSetContext( WPARAM command, LPARAM data, LRESULT& result )
{
    if( m_Property & IME_PROP_AT_CARET )
    {
        data &= ~ISC_SHOWUICOMPOSITIONWINDOW;
        data |= ISC_SHOWUIGUIDELINE | ISC_SHOWUIALLCANDIDATEWINDOW;
    }
    result = DefWindowProc( m_WindowHandle, WM_IME_SETCONTEXT, command, data );
}

void IMEDevice::OnInputLangChange( WPARAM command, LPARAM data )
{
    //assert( 0 );
}

void IMEDevice::OnCharUP( UINT inputedChar )
{
	switch( inputedChar ) 
	{
	case 165:/*LEFT*/
		{
			if(m_Shvsleri)
			{
				TheInputProxy().HandleIMEMessage( eIMEMessage_ShiftUp );
			}
		}
		break;

	case 167:/*RIGHT*/
		{
			if(m_Shvsleri)
			{
				TheInputProxy().HandleIMEMessage( eIMEMessage_ShiftUp );
			}
		}
		break;
	case 144:
		if( m_Shvsleri )
		{
			TheInputProxy().HandleIMEMessage( eIMEMessage_ShiftUp );
			m_Shvsleri = false;
		}
		break;
	}
}

void IMEDevice::OnChar( UINT inputedChar )
{
    switch( inputedChar )
    {
    case 27:/*ESCAPE*/
        TheInputProxy().HandleIMEMessage( eIMEMessage_Escape );
        break;
    case 8:/*BACK*/
        if( m_IsCompositing == false )
            TheInputProxy().HandleIMEMessage( eIMEMessage_Back );
		break;

    case 165:/*LEFT*/
		{
			if( m_IsCompositing == false )
			{
				if(!m_Shvsleri)
				{
					TheInputProxy().HandleIMEMessage( eIMEMessage_Left );
				}
				else
				{
					TheInputProxy().HandleIMEMessage( eIMEMessage_LShift );
				}
			}
		}
        break;

    case 167:/*RIGHT*/
		{
			if( m_IsCompositing == false )
			{
				if(!m_Shvsleri)
				{
					TheInputProxy().HandleIMEMessage( eIMEMessage_Right );
				}
				else
				{
					TheInputProxy().HandleIMEMessage( eIMEMessage_RShift );
				}
			}
		}
        break;

    case 13:/*ENTER*/
        if( m_IsCompositing == false )
            TheInputProxy().HandleIMEMessage( eIMEMessage_Enter );
        break;

	case 174/*DELETE*/:
		if( m_IsCompositing == false )
			TheInputProxy().HandleIMEMessage( eIMEMessage_Delete );
		break;

	case 144/*SHIFT*/:
		if( m_IsCompositing == false )
		{
			m_Shvsleri = true;
		}
		break;
	}

	if( inputedChar >= ' ' && inputedChar <= 126 )
	{
		char inputedString[2];
		inputedString[0] = inputedChar & 0xff;
		inputedString[1] = NULL;
		TheInputProxy().HandleIMEMessage( eIMEMessage_Char, inputedString );
	}
}

bool IMEDevice::CaptureContext()
{
    m_ImeContext = ImmGetContext( m_WindowHandle );

    if( m_ImeContext )
        return true;
    else
        return false;
}

void IMEDevice::ReleaseContext()
{
    if( m_ImeContext )
        ImmReleaseContext( m_WindowHandle, m_ImeContext );
    
    m_ImeContext = NULL;
}


#endif //NEW_USER_INTERFACE_INPUTSYSTEM