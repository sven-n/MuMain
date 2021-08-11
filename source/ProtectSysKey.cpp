#include "stdafx.h"

#include "ProtectSysKey.h"

#if defined USER_WINDOW_MODE || (defined WINDOWMODE)
extern BOOL g_bUseWindowMode;
#endif

bool ProtectSysKey::AttachProtectSysKey(HINSTANCE hInst, HWND hWnd)
{ return ProtectSysKey::CProtectSysKey::GetObjPtr()->AttachProtectSysKey(hInst, hWnd); }

void ProtectSysKey::DetachProtectSysKey()
{ ProtectSysKey::CProtectSysKey::GetObjPtr()->DetachProtectSysKey(); }


using namespace ProtectSysKey;

CProtectSysKey::CProtectSysKey() : m_hKeyboardHook(NULL), m_hWnd(NULL)
{}
CProtectSysKey::~CProtectSysKey() 
{
	DetachProtectSysKey();
}

bool CProtectSysKey::AttachProtectSysKey(HINSTANCE hInst, HWND hWnd)
{
	if(!m_hKeyboardHook) {
		// for Windows2000 and WindowsXP
		// WH_KEYBOARD_LL = 13
		m_hKeyboardHook = SetWindowsHookEx(13, CProtectSysKey::LowLevelKeyHookProc, hInst, 0);
		
		// for Window98 9 (lol by louis)
		if(RegisterHotKey( hWnd, 0, MOD_ALT, VK_TAB))
			m_hWnd = hWnd;

		return true;
	}
	return false;
}
void CProtectSysKey::DetachProtectSysKey()
{
	if(m_hKeyboardHook) {
		UnhookWindowsHookEx(m_hKeyboardHook);
		m_hKeyboardHook = NULL;
	}
	if(m_hWnd) {
		UnregisterHotKey(m_hWnd, 0);
		m_hWnd = NULL;
	}
}

HHOOK CProtectSysKey::GetHookHandle() const
{ return m_hKeyboardHook; }

CProtectSysKey* CProtectSysKey::GetObjPtr() 
{
	static CProtectSysKey s_Instance;
	return &s_Instance;
}

LRESULT CALLBACK CProtectSysKey::LowLevelKeyHookProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if(nCode == HC_ACTION) {
		switch(wParam)
		{
		case WM_KEYUP:
		case WM_KEYDOWN:
			{
				PKBDLLHOOKSTRUCT pKBHookStruct = (PKBDLLHOOKSTRUCT)lParam;
				switch(pKBHookStruct->vkCode)
				{
				case VK_ESCAPE:
					if(GetAsyncKeyState( VK_CONTROL))
						return 1;
					break;
				case VK_LWIN:
				case VK_RWIN:
#ifdef WINDOWMODE
					if (g_bUseWindowMode == FALSE)
					return 1;
#else
					return 1;
#endif
				}
			}
			break;
		}
	}
	return CallNextHookEx(CProtectSysKey::GetObjPtr()->GetHookHandle(), nCode, wParam, lParam);
}