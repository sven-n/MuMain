// w_KeyboardDevice.cpp: implementation of the KeyboardDevice class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "w_KeyboardDevice.h"

#ifdef NEW_USER_INTERFACE_INPUTSYSTEM

using namespace input;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

KeyboardDevicePtr KeyboardDevice::Make( IDirectInput8& dxinput, HWND windowHandle )
{
	KeyboardDevicePtr keydevice( new KeyboardDevice( dxinput, windowHandle ) );
	return keydevice;
}

KeyboardDevice::KeyboardDevice( IDirectInput8& dxinput, HWND windowHandle ) : m_pDevice( NULL ), 
m_WindowHandle( windowHandle ), m_Acquired( false )
{
	Initialize( dxinput );
}

KeyboardDevice::~KeyboardDevice()
{
	Destroy();
}

void KeyboardDevice::Initialize( IDirectInput8& dxinput )
{
	try 
	{
        InitKeyMap();

        if( dxinput.CreateDevice( GUID_SysKeyboard, &m_pDevice, NULL ) != DI_OK )
            throw;

        if( m_pDevice->SetDataFormat( &c_dfDIKeyboard ) != DI_OK )
            throw;

        if( m_pDevice->SetCooperativeLevel( m_WindowHandle, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE ) != DI_OK )
            throw;

        DIPROPDWORD  dipdw;

        dipdw.diph.dwSize = sizeof(DIPROPDWORD);
        dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
        dipdw.diph.dwHow = DIPH_DEVICE;
        dipdw.diph.dwObj = 0;
        dipdw.dwData = 64;

        if( m_pDevice->SetProperty( DIPROP_BUFFERSIZE, &dipdw.diph ) != DI_OK )
            throw;

        Acquire();
    }
    catch( ... )
    {
        Destroy();
        throw;
    }
}

void KeyboardDevice::InitKeyMap()
{
	for( unsigned int i = 0; i < m_KeyMap.size(); i++ )
    {
        m_KeyMap[i] = eKey_None;
    }

    m_KeyMap[DIK_ESCAPE]    = eKey_Escape;
    m_KeyMap[DIK_SPACE]     = eKey_Space;
    m_KeyMap[DIK_A]         = eKey_A;
    m_KeyMap[DIK_S]         = eKey_S;
    m_KeyMap[DIK_D]         = eKey_D;
    m_KeyMap[DIK_F]         = eKey_F;
    m_KeyMap[DIK_G]         = eKey_G;
    m_KeyMap[DIK_P]         = eKey_P;
    m_KeyMap[DIK_R]         = eKey_R;
	m_KeyMap[DIK_Q]			= eKey_Q;
    m_KeyMap[DIK_W]         = eKey_W;
    m_KeyMap[DIK_E]         = eKey_E;
    m_KeyMap[DIK_M]         = eKey_M;
	m_KeyMap[DIK_U]         = eKey_U;
    m_KeyMap[DIK_I]         = eKey_I;
    m_KeyMap[DIK_X]         = eKey_X;
    m_KeyMap[DIK_F1]        = eKey_F1;
    m_KeyMap[DIK_F2]        = eKey_F2;
    m_KeyMap[DIK_F3]        = eKey_F3;
    m_KeyMap[DIK_F4]        = eKey_F4;
    m_KeyMap[DIK_F5]        = eKey_F5;
    m_KeyMap[DIK_F6]        = eKey_F6;
    m_KeyMap[DIK_F7]        = eKey_F7;
    m_KeyMap[DIK_F8]        = eKey_F8;
    m_KeyMap[DIK_F9]        = eKey_F9;
    m_KeyMap[DIK_F10]       = eKey_F10;
	m_KeyMap[DIK_F11]       = eKey_F11;
    m_KeyMap[DIK_F12]       = eKey_F12;
    m_KeyMap[DIK_1]         = eKey_1;
    m_KeyMap[DIK_2]         = eKey_2;
    m_KeyMap[DIK_3]         = eKey_3;
	m_KeyMap[DIK_4]         = eKey_4;
	m_KeyMap[DIK_5]         = eKey_5;
	m_KeyMap[DIK_6]         = eKey_6;
	m_KeyMap[DIK_7]         = eKey_7;
	m_KeyMap[DIK_8]         = eKey_8;
	m_KeyMap[DIK_9]         = eKey_9;
	m_KeyMap[DIK_0]         = eKey_0;
    m_KeyMap[DIK_UP]        = eKey_Up;
    m_KeyMap[DIK_DOWN]      = eKey_Down;
    m_KeyMap[DIK_LEFT]      = eKey_Left;
    m_KeyMap[DIK_RIGHT]     = eKey_Right;
	m_KeyMap[DIK_RETURN]	= eKey_Enter;
	m_KeyMap[DIK_TAB]		= eKey_Tab;
	m_KeyMap[DIK_DELETE]    = eKey_Delete;
}

void KeyboardDevice::Destroy()
{
	Unacquire();
    SAFE_RELEASE( m_pDevice );
}

void KeyboardDevice::Active( bool b )
{
	assert( 0 );
}

void KeyboardDevice::Reset()
{
	assert( 0 );
}

void KeyboardDevice::Process( int delta )
{
    if( m_Acquired == false )
	{
		return;
	}

    GetDeviceData();
}

bool KeyboardDevice::Acquire()
{
	if( m_pDevice->Acquire() == DI_OK )
    {
        m_Acquired = true;
        return true;
    }
    else
    {
        m_Acquired = false;
        return false;
    }
}

void KeyboardDevice::Unacquire()
{
	if( m_pDevice )
	{
		m_Acquired = false;
		m_pDevice->Unacquire();
	}
}

void KeyboardDevice::GetDeviceData()
{
	DIDEVICEOBJECTDATA  data[64];
	HRESULT result;

	do 
	{
		DWORD itemCnt = sizeof(data)/sizeof(*data);
		result = m_pDevice->GetDeviceData( sizeof( DIDEVICEOBJECTDATA )
			, data, &itemCnt, 0 );

		if( SUCCEEDED( result ) )
		{
			for( unsigned int i = 0; i < itemCnt; i++ )
			{
				eKeyType type = m_KeyMap[data[i].dwOfs];
				if( type != eKey_None )
				{
					eKeyStateType state = data[i].dwData ? eKeyState_Down : eKeyState_Up;
					TheInputProxy().HandleKeyboardMessage( type, state );
				}
			}
		}
	} while( result == DI_BUFFEROVERFLOW );
}

bool KeyboardDevice::HandleWindowMessage( UINT message, WPARAM wParam, LPARAM lParam, LRESULT& result )
{
    switch( message )
    {
    case WM_NCACTIVATE:
        if( m_Acquired == false )
        {
            Unacquire();
            Acquire();
        }
        return false;

    case WM_ACTIVATE:
        if( wParam != WA_INACTIVE )
        {
            if( Acquire() == true )
            {
                GetDeviceData();
            }
        }
        else
        {
            Unacquire();
        }
        return false;
    }
    return false;
}

#endif //NEW_USER_INTERFACE_INPUTSYSTEM