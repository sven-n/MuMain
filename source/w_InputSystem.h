// w_InputSystem.h: interface for the InputSystem class.
//
//////////////////////////////////////////////////////////////////////

#ifdef NEW_USER_INTERFACE_INPUTSYSTEM

#if !defined(AFX_W_INPUTSYSTEM_H__27D210BC_DDDD_4A76_972E_AB6E6B3F4C01__INCLUDED_)
#define AFX_W_INPUTSYSTEM_H__27D210BC_DDDD_4A76_972E_AB6E6B3F4C01__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "w_MouseDevice.h"
#include "w_KeyboardDevice.h"
#include "w_IMEDevice.h"
#include "w_WindowMessageHandler.h"

namespace input
{
	BoostSmartPointer( InputSystem );
	class InputSystem : public WindowMessageHandler, public InputProxyHandler
	{
	public:
		static InputSystemPtr Make( HWND windowHandle );
		virtual ~InputSystem();

	public:
		//WindowMessageHandler
		virtual bool HandleWindowMessage( UINT message, WPARAM wParam, LPARAM lParam, LRESULT& result );
		//InputProxyHandler
		virtual void OnHandleMouseMessage( eKeyType key, eKeyStateType type, int value = 0 );
		virtual void OnHandleIMEMessage( eIMEMessageType type, const string& str = string() );
		virtual void OnHandleKeyboardMessage( eKeyType type, eKeyStateType state );

	public:
		void RegisterInputMessageHandler( BoostWeak_Ptr(InputMessageHandler) handler, bool first = false );
		void RegisterIMEMessageHandler( BoostWeak_Ptr(IMEMessageHandler) handler );
		void ResetIME();
		void ActiveIME( bool b );
		void ActiveMouse( bool b );
		void SetWindowSize( const Dim& size );
		void Process( int delta );

	private:
		void Initialize( HWND windowHandle );
		void Destroy();
		InputSystem();

	private:
		typedef list< BoostWeak_Ptr(InputMessageHandler) >   HandlerList;

	private:
		//dxinput
		IDirectInput8*							m_pDI;
		//여기 역역 안에서만 활용할 수 있다.
		//scoped_ptr로 만들어 주자.
		BoostSmart_Ptr(MouseDevice)				m_MouseDevice;
		BoostSmart_Ptr(KeyboardDevice)			m_KeyboardDevice;
		BoostSmart_Ptr(IMEDevice)				m_IMEDevice;
		//handler
		HandlerList								m_HandlerList;
		BoostWeak_Ptr(IMEMessageHandler)		m_IMEHandler;
	};

inline
void InputSystem::SetWindowSize( const Dim& size )
{
	m_MouseDevice->SetWindowSize( size );
}

};

#endif // !defined(AFX_W_INPUTSYSTEM_H__27D210BC_DDDD_4A76_972E_AB6E6B3F4C01__INCLUDED_)

#endif //NEW_USER_INTERFACE_INPUTSYSTEM