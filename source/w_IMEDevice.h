// w_IMEDevice.h: interface for the IMEDevice class.
//
//////////////////////////////////////////////////////////////////////

#ifdef NEW_USER_INTERFACE_INPUTSYSTEM

#if !defined(AFX_W_IMEDEVICE_H__0FF214B1_6A4B_49DD_8717_A6111B3F1ED3__INCLUDED_)
#define AFX_W_IMEDEVICE_H__0FF214B1_6A4B_49DD_8717_A6111B3F1ED3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "w_typedevice.h"
#include "w_InputProxy.h"
#include <dinput.h>

namespace input
{
	BoostSmartPointer( IMEDevice );
	class IMEDevice : public TypedDevice, public InputProxyHandler  
	{
	public:
		static IMEDevicePtr Make( IDirectInput8& dxinput, HWND windowHandle );
		virtual ~IMEDevice();

	public:
		virtual void Active( bool b );
		virtual void Reset();
		virtual void Process( int delta );
		virtual bool HandleWindowMessage( UINT message, WPARAM wParam, LPARAM lParam, LRESULT& result );

	private:
		void Initialize();
		void Destroy();
		IMEDevice( IDirectInput8& dxinput, HWND windowHandle );

	private:
		void OnStartComposition( WPARAM command, LPARAM data );
		void OnEndComposition( WPARAM command, LPARAM data );
		void OnComposition( WPARAM command, LPARAM data );
		void OnSetContext( WPARAM command, LPARAM data, LRESULT& result );
		void OnInputLangChange( WPARAM command, LPARAM data );
		void OnChar( UINT inputedChar );
		void OnCharUP( UINT inputedChar );

		bool CaptureContext();
		void ReleaseContext();
		bool MayDoComposition() const;
		void ReportResultString();
		void ReportCompositionString( LONG flag );

	private:
		HWND				m_WindowHandle;
		HKL					m_KeyLayout;
		HIMC				m_ImeContext;
		DWORD				m_Property;

		bool				m_IsActive;
		bool				m_IsCompositing;
		bool				m_Shvsleri;
	};
};

#endif // !defined(AFX_W_IMEDEVICE_H__0FF214B1_6A4B_49DD_8717_A6111B3F1ED3__INCLUDED_)

#endif //NEW_USER_INTERFACE_INPUTSYSTEM