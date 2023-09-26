#include "stdafx.h"
#include "CBTMessageBox.h"

int leaf::CBTMessageBox(HWND hWnd, const std::wstring& text, const std::wstring& caption, UINT uType, bool bAlwaysOnTop)
{
    return CCBTMessageBox::GetInstance()->OpenMessageBox(hWnd, text.c_str(), caption.c_str(), uType, bAlwaysOnTop);
}

using namespace leaf;

CCBTMessageBox::CCBTMessageBox() : m_hParentWnd(NULL), m_hCBT(NULL), m_bAlwaysOnTop(false)
{}
CCBTMessageBox::~CCBTMessageBox()
{}

int CCBTMessageBox::OpenMessageBox(HWND hWnd, const wchar_t* lpText, const wchar_t* lpCaption, UINT uType, bool bAlwaysOnTop)
{
    if (hWnd != NULL) {
        m_hParentWnd = hWnd;
    }
    else {
        m_hParentWnd = GetDesktopWindow();
    }

    if (HookCBT()) {
        m_bAlwaysOnTop = bAlwaysOnTop;
        return MessageBox(hWnd, lpText, lpCaption, uType);
    }
    return 0;
}

HWND CCBTMessageBox::GetParentWndHandle() const
{
    return m_hParentWnd;
}
bool CCBTMessageBox::IsAlwaysOnTop() const
{
    return m_bAlwaysOnTop;
}

HHOOK CCBTMessageBox::GetHookHandle() const
{
    return m_hCBT;
}
bool CCBTMessageBox::HookCBT()
{
    if (m_hCBT)
        return false;
    m_hCBT = SetWindowsHookEx(WH_CBT, &CBTProc, 0, GetCurrentThreadId());
    if (m_hCBT == NULL)
        return false;
    return true;
}
void CCBTMessageBox::UnhookCBT()
{
    if (m_hCBT) {
        UnhookWindowsHookEx(m_hCBT);
        m_hCBT = NULL;
    }
}

CCBTMessageBox* CCBTMessageBox::GetInstance()
{
    static CCBTMessageBox s_Instance;
    return &s_Instance;
}

LRESULT CALLBACK CCBTMessageBox::CBTProc(INT nCode, WPARAM wParam, LPARAM lParam)
{
    // notification that a window is about to be activated
    // window handle is wParam
    if (nCode == HCBT_ACTIVATE && GetInstance()->GetHookHandle())
    {
        HWND	hChildWnd = (HWND)wParam;    // msgbox is "child"
        HWND	hParentWnd = GetInstance()->GetParentWndHandle();
        RECT  rParent, rChild, rDesktop;
        POINT pCenter, pStart;
        INT   nWidth, nHeight;

        // exit CBT hook
        GetInstance()->UnhookCBT();

        if ((hParentWnd != NULL) &&
            (hChildWnd != NULL) &&
            (GetWindowRect(GetDesktopWindow(), &rDesktop) != 0) &&
            (GetWindowRect(hParentWnd, &rParent) != 0) &&
            (GetWindowRect(hChildWnd, &rChild) != 0))
        {
            // calculate message box dimensions
            nWidth = (rChild.right - rChild.left);
            nHeight = (rChild.bottom - rChild.top);

            // calculate parent window center point
            pCenter.x = rParent.left + ((rParent.right
                - rParent.left) / 2);
            pCenter.y = rParent.top + ((rParent.bottom
                - rParent.top) / 2);

            // calculate message box starting point
            pStart.x = (pCenter.x - (nWidth / 2));
            pStart.y = (pCenter.y - (nHeight / 2));

            // adjust if message box is off desktop
            if (pStart.x < 0) pStart.x = 0;
            if (pStart.y < 0) pStart.y = 0;
            if (pStart.x + nWidth > rDesktop.right)
                pStart.x = rDesktop.right - nWidth;
            if (pStart.y + nHeight > rDesktop.bottom)
                pStart.y = rDesktop.bottom - nHeight;

            // move message box
            HWND hWndInsertAfter = NULL;
            if (GetInstance()->IsAlwaysOnTop())
                hWndInsertAfter = HWND_TOPMOST;
            SetWindowPos(hChildWnd, hWndInsertAfter, pStart.x, pStart.y, 0, 0, SWP_HIDEWINDOW | SWP_NOSIZE);
        }
    }
    else if (nCode == HCBT_DESTROYWND) {
        // exit CBT hook
        GetInstance()->UnhookCBT();
    }
    // otherwise, continue with any possible chained hooks
    else CallNextHookEx(GetInstance()->GetHookHandle(), nCode, wParam, lParam);

    return 0;
}