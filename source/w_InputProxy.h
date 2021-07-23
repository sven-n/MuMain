// w_InputProxy.h: interface for the InputProxy class.
//
//////////////////////////////////////////////////////////////////////

#ifdef NEW_USER_INTERFACE_PROXY

#if !defined(AFX_W_INPUTPROXY_H__F2C9B24C_EC4C_4042_A956_5A9FF8D3E2CD__INCLUDED_)
#define AFX_W_INPUTPROXY_H__F2C9B24C_EC4C_4042_A956_5A9FF8D3E2CD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "w_ServerProxy.h"

//Handler에 Virtual 함수만 추가 하자.
//순수 가장 함수로 만들지 말자.--;;;
struct InputProxyHandler 
{
	virtual ~InputProxyHandler() = 0 {}
	virtual void OnHandleMouseMessage( eKeyType key, eKeyStateType type, int value = 0 ){}
	virtual void OnHandleIMEMessage( eIMEMessageType type, const string& str = string() ){}
	virtual void OnHandleKeyboardMessage( eKeyType type, eKeyStateType state ){}
};

BoostSmartPointer( InputProxy );
class InputProxy : public LocalServerProxy<InputProxyHandler>
{
public:
	static InputProxyPtr Make( BoostWeak_Ptr(ServerProxy) proxy );
	virtual ~InputProxy();

public:
	void HandleMouseMessage( eKeyType key, eKeyStateType type, int value = 0 );
	void HandleIMEMessage( eIMEMessageType type, const string& str = string() );
	void HandleKeyboardMessage( eKeyType type, eKeyStateType state );

private:
	void Initialize( BoostWeak_Ptr(InputProxy) proxy );
	void Destroy();
	InputProxy();
};

#endif // !defined(AFX_W_INPUTPROXY_H__F2C9B24C_EC4C_4042_A956_5A9FF8D3E2CD__INCLUDED_)

#endif //NEW_USER_INTERFACE_PROXY