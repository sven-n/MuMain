// w_KeyboardDevice.h: interface for the KeyboardDevice class.
//
//////////////////////////////////////////////////////////////////////

#ifdef NEW_USER_INTERFACE_INPUTSYSTEM

#if !defined(AFX_W_KEYBOARDDEVICE_H__A1498391_959C_4304_AC01_58CE93D03254__INCLUDED_)
#define AFX_W_KEYBOARDDEVICE_H__A1498391_959C_4304_AC01_58CE93D03254__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <dinput.h>
#include "w_typedevice.h"
#include "w_InputProxy.h"

#define KEYBOARDCOUNT 256

namespace input
{
	BoostSmartPointer( KeyboardDevice );
	class KeyboardDevice : public TypedDevice, public InputProxyHandler
	{
	public:
		static KeyboardDevicePtr Make( IDirectInput8& dxinput, HWND windowHandle );	
		virtual ~KeyboardDevice();

	public:
		virtual void Active( bool b );
		virtual void Reset();
		virtual void Process( int delta );
		virtual bool HandleWindowMessage( UINT message, WPARAM wParam, LPARAM lParam, LRESULT& result );

	private:
		void Initialize( IDirectInput8& input );
		void InitKeyMap();
		bool Acquire();
		void Destroy();
		void Unacquire();
		void GetDeviceData();
		KeyboardDevice( IDirectInput8& dxinput, HWND windowHandle );

	private:
		HWND									m_WindowHandle;
		IDirectInputDevice8*					m_pDevice;
		array<eKeyType, KEYBOARDCOUNT>			m_KeyMap;
		bool									m_Acquired;
	};
};

#endif // !defined(AFX_W_KEYBOARDDEVICE_H__A1498391_959C_4304_AC01_58CE93D03254__INCLUDED_)

#endif //NEW_USER_INTERFACE_INPUTSYSTEM