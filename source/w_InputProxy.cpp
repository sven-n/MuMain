// w_InputProxy.cpp: implementation of the InputProxy class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "w_InputProxy.h"

#ifdef NEW_USER_INTERFACE_PROXY

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

InputProxyPtr InputProxy::Make( BoostWeak_Ptr(ServerProxy) proxy )
{
	InputProxyPtr inputproxy( new InputProxy() );
	inputproxy->Initialize( inputproxy );
	inputproxy->RegisterMainServerProxy( proxy );
	return inputproxy;
}

InputProxy::InputProxy()
{

}

InputProxy::~InputProxy()
{
	Destroy();
}

void InputProxy::Initialize( BoostWeak_Ptr(InputProxy) proxy )
{

}

void InputProxy::Destroy()
{

}

void InputProxy::HandleMouseMessage( eKeyType key, eKeyStateType type, int value )
{
	NotifyToHandler( &InputProxyHandler::OnHandleMouseMessage, key, type, value );
}

void InputProxy::HandleIMEMessage( eIMEMessageType type, const string& str )
{
	NotifyToHandler( &InputProxyHandler::OnHandleIMEMessage, type, str );
}

void InputProxy::HandleKeyboardMessage( eKeyType type, eKeyStateType state )
{
	NotifyToHandler( &InputProxyHandler::OnHandleKeyboardMessage, type, state );
}

#endif //NEW_USER_INTERFACE_PROXY
