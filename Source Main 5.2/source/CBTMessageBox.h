#ifndef _CBTMESSAGEBOX_H_
#define _CBTMESSAGEBOX_H_

#include <windows.h>

#pragma warning(disable : 4786)
#include <string>

namespace leaf {
    //. Interface Declaration

    int CBTMessageBox(HWND hWnd, const std::wstring& text, const std::wstring& caption, UINT uType, bool bAlwaysOnTop = false);

    //. class CCBTMessageBox

    class CCBTMessageBox
    {
        HWND	m_hParentWnd;
        HHOOK	m_hCBT;
        bool	m_bAlwaysOnTop;

    public:
        CCBTMessageBox();
        ~CCBTMessageBox();

        int OpenMessageBox(HWND hWnd, const wchar_t* lpText, const wchar_t* lpCaption, UINT uType, bool bAlwaysOnTop);

        HWND GetParentWndHandle() const;
        bool IsAlwaysOnTop() const;

        bool HookCBT();
        void UnhookCBT();
        HHOOK GetHookHandle() const;

        static CCBTMessageBox* GetInstance();

        static LRESULT CALLBACK CBTProc(INT nCode, WPARAM wParam, LPARAM lParam);
    };
}

#endif // _CBTMESSAGEBOX_H_