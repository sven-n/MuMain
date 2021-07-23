// w_MouseDevice.h: interface for the MouseDevice class.
//
//////////////////////////////////////////////////////////////////////

#ifdef NEW_USER_INTERFACE_INPUTSYSTEM

#if !defined(AFX_W_MOUSEDEVICE_H__063B37AB_D4F1_412C_B583_02142C62013B__INCLUDED_)
#define AFX_W_MOUSEDEVICE_H__063B37AB_D4F1_412C_B583_02142C62013B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "w_typedevice.h"
#include "w_InputProxy.h"
#include <dinput.h>

//#define DBCLK

namespace input
{
	BoostSmartPointer( MouseDevice );
	class MouseDevice : public TypedDevice, public InputProxyHandler
	{
	public:
		static MouseDevicePtr Make( IDirectInput8& dxinput, HWND windowHandle );
		virtual ~MouseDevice();

	public:
		void Acquire();
		void Unacquire();
		void SetWindowSize( const Dim& size );
		const Coord& GetPosition() const;

	public:
		virtual void Reset();
		virtual void Process( int delta );   
		virtual void Active( bool b );
		virtual bool HandleWindowMessage( UINT message, WPARAM wParam, LPARAM lParam, LRESULT& result );

	private:
		void Initialize( HWND windowHandle );
		void Destroy();
		MouseDevice( IDirectInput8& dxinput, HWND windowHandle );

	private:
		HWND					m_WindowHandle;
		Coord					m_Position;
		Dim						m_WindowSize;
		bool					m_IsActive;
		bool					m_Acquired;
		int						m_PosValue;
#ifdef DBCLK
		bool					m_IsLbuttonDown;
		bool					m_IsRbuttonDown;
		bool                    m_IsLbuttonDownUp;
		bool                    m_IsRbuttonDownUp;
		bool					m_IsLbuttonDBclk;
		bool					m_IsRbuttonDBclk;
		DWORD					m_StatrtTime;
#endif //DBCLK
	};

inline
void MouseDevice::SetWindowSize( const Dim& size )
{
	m_WindowSize = size;
}

inline
const Coord& MouseDevice::GetPosition() const
{
	return m_Position;
}

};

#endif // !defined(AFX_W_MOUSEDEVICE_H__063B37AB_D4F1_412C_B583_02142C62013B__INCLUDED_)

#endif //NEW_USER_INTERFACE_INPUTSYSTEM