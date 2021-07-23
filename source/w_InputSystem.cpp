// w_InputSystem.cpp: implementation of the InputSystem class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "w_InputSystem.h"

#ifdef NEW_USER_INTERFACE_INPUTSYSTEM

using namespace input;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

InputSystemPtr InputSystem::Make( HWND windowHandle )
{
	InputSystemPtr input( new InputSystem() );
	input->Initialize( windowHandle );
	return input;
}

InputSystem::InputSystem()
{

}

InputSystem::~InputSystem()
{
	Destroy();
}

void InputSystem::Initialize( HWND windowHandle )
{
	try 
	{
		//dxinput
		DirectInput8Create(::GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (VOID**)&m_pDI, NULL);
		//keyboard
		m_KeyboardDevice = KeyboardDevice::Make( *m_pDI,  windowHandle );
		//mouse
		m_MouseDevice = MouseDevice::Make( *m_pDI,  windowHandle );
		ActiveMouse( true );
		//ime
		m_IMEDevice = IMEDevice::Make( *m_pDI,  windowHandle );
		ActiveIME( false );
	}
	catch ( ... ) 
	{
		Destroy();
        throw;
	}
}

void InputSystem::Destroy()
{
	m_KeyboardDevice.reset();
    SAFE_RELEASE( m_pDI );
}

void InputSystem::OnHandleMouseMessage( eKeyType key, eKeyStateType type, int value )
{
    for( HandlerList::iterator iter = m_HandlerList.begin()
        ; iter != m_HandlerList.end(); )
    {
        HandlerList::iterator curIter = iter;
        ++iter;
        BoostWeak_Ptr(InputMessageHandler) handler = *curIter;
        if( handler.expired() == false )
        {
            if( handler.lock()->HandleInputMessage( key, type, value ) == true )
            {
                return;
            }
        }
        else
        {
            m_HandlerList.erase( curIter );
        }
    }
}

void InputSystem::OnHandleIMEMessage( eIMEMessageType type, const string& str )
{
	if( m_IMEHandler.expired() == false )
		m_IMEHandler.lock()->HandleIMEMessage( type, str );
}

void InputSystem::OnHandleKeyboardMessage( eKeyType type, eKeyStateType state )
{
	for( HandlerList::iterator iter = m_HandlerList.begin()
        ; iter != m_HandlerList.end(); )
    {
        HandlerList::iterator curIter = iter;
        ++iter;
        BoostWeak_Ptr(InputMessageHandler) handler = *curIter;
        if( handler.expired() == false )
        {
            if( handler.lock()->HandleInputMessage( type, state, 0 ) == true )
            {
                return;
            }
        }
        else
        {
            m_HandlerList.erase( curIter );
        }
    }
}

bool InputSystem::HandleWindowMessage( UINT message, WPARAM wParam, LPARAM lParam, LRESULT& result )
{
    if( m_KeyboardDevice->HandleWindowMessage( message, wParam, lParam, result ) == true )
    {
        return true;
    }

    if( m_MouseDevice->HandleWindowMessage( message, wParam, lParam, result ) == true )
    {
        return true;
    }

    if( m_IMEDevice->HandleWindowMessage( message, wParam, lParam, result ) == true )
    {
        return true;
    }

    return false;
}

void InputSystem::RegisterInputMessageHandler( BoostWeak_Ptr(InputMessageHandler) handler, bool first )
{
	if( first )
        m_HandlerList.push_front( handler );
    else
        m_HandlerList.push_back( handler );
}

void InputSystem::RegisterIMEMessageHandler( BoostWeak_Ptr(IMEMessageHandler) handler )
{
	m_IMEHandler = handler;
}

void InputSystem::ResetIME()
{
	if( m_IMEDevice )
		m_IMEDevice->Reset();
}

void InputSystem::ActiveIME( bool b )
{
    if( m_IMEDevice )
        m_IMEDevice->Active( b );
}

void InputSystem::ActiveMouse( bool b )
{
    if( m_MouseDevice )
        m_MouseDevice->Active( b );
}

void InputSystem::Process( int delta )
{
    if( m_MouseDevice ) m_MouseDevice->Process( delta );
    if( m_KeyboardDevice) m_KeyboardDevice->Process( delta );
	if( m_IMEDevice ) m_IMEDevice->Process( delta );
}


#endif //NEW_USER_INTERFACE_INPUTSYSTEM