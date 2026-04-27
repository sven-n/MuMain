#include "stdafx.h"
#include "CBTMessageBox.h"

#include <algorithm>

int leaf::CBTMessageBox(HWND hWnd, const std::wstring& text, const std::wstring& caption, UINT uType, bool bAlwaysOnTop)
{
    return CCBTMessageBox::GetInstance()->OpenMessageBox(hWnd, text.c_str(), caption.c_str(), uType, bAlwaysOnTop);
}

namespace
{
    int ClampInt(int value, int minValue, int maxValue)
    {
        return std::min<int>(std::max<int>(value, minValue), maxValue);
    }

    POINT ComputeCenteredTopLeft(const RECT& parentRect, const RECT& childRect, const RECT& desktopRect)
    {
        const int width = (childRect.right - childRect.left);
        const int height = (childRect.bottom - childRect.top);

        const int centerX = parentRect.left + ((parentRect.right - parentRect.left) / 2);
        const int centerY = parentRect.top + ((parentRect.bottom - parentRect.top) / 2);

        POINT result{};
        result.x = centerX - (width / 2);
        result.y = centerY - (height / 2);

        const int desktopRight = static_cast<int>(desktopRect.right);
        const int desktopBottom = static_cast<int>(desktopRect.bottom);
        const int maxX = std::max<int>(0, desktopRight - width);
        const int maxY = std::max<int>(0, desktopBottom - height);

        result.x = ClampInt(result.x, 0, maxX);
        result.y = ClampInt(result.y, 0, maxY);
        return result;
    }

    struct ScopedCbtUnhook
    {
        leaf::CCBTMessageBox* owner{nullptr};

        explicit ScopedCbtUnhook(leaf::CCBTMessageBox* p) : owner(p) {}

        ScopedCbtUnhook(const ScopedCbtUnhook&) = delete;
        ScopedCbtUnhook& operator=(const ScopedCbtUnhook&) = delete;
        ScopedCbtUnhook(ScopedCbtUnhook&&) = delete;
        ScopedCbtUnhook& operator=(ScopedCbtUnhook&&) = delete;
        ~ScopedCbtUnhook()
        {
            if (owner)
            {
                owner->UnhookCBT();
            }
        }
    };
}

leaf::CCBTMessageBox::CCBTMessageBox() : m_hParentWnd(nullptr), m_hCBT(nullptr), m_bAlwaysOnTop(false)
{}

leaf::CCBTMessageBox::~CCBTMessageBox()
{
    UnhookCBT();
}

int leaf::CCBTMessageBox::OpenMessageBox(HWND hWnd, const wchar_t* lpText, const wchar_t* lpCaption, UINT uType, bool bAlwaysOnTop)
{
    m_hParentWnd = (hWnd != nullptr) ? hWnd : GetDesktopWindow();
    m_bAlwaysOnTop = bAlwaysOnTop;

    if (!HookCBT())
    {
        return 0;
    }

    ScopedCbtUnhook unhookGuard{this};
    return MessageBox(m_hParentWnd, lpText, lpCaption, uType);
}

HWND leaf::CCBTMessageBox::GetParentWndHandle() const
{
    return m_hParentWnd;
}
bool leaf::CCBTMessageBox::IsAlwaysOnTop() const
{
    return m_bAlwaysOnTop;
}

HHOOK leaf::CCBTMessageBox::GetHookHandle() const
{
    return m_hCBT;
}
bool leaf::CCBTMessageBox::HookCBT()
{
    if (m_hCBT)
        return false;
    m_hCBT = SetWindowsHookEx(WH_CBT, &CBTProc, 0, GetCurrentThreadId());
    if (m_hCBT == nullptr)
        return false;
    return true;
}
void leaf::CCBTMessageBox::UnhookCBT()
{
    if (m_hCBT) {
        UnhookWindowsHookEx(m_hCBT);
        m_hCBT = nullptr;
    }
}

leaf::CCBTMessageBox* leaf::CCBTMessageBox::GetInstance()
{
    static CCBTMessageBox s_Instance;
    return &s_Instance;
}

LRESULT CALLBACK leaf::CCBTMessageBox::CBTProc(INT nCode, WPARAM wParam, LPARAM lParam)
{
    // notification that a window is about to be activated
    // window handle is wParam
    if (nCode == HCBT_ACTIVATE && GetInstance()->GetHookHandle())
    {
        HWND hChildWnd = reinterpret_cast<HWND>(wParam);    // msgbox is "child"
        HWND hParentWnd = GetInstance()->GetParentWndHandle();
        RECT rParent{};
        RECT rChild{};
        RECT rDesktop{};

        // exit CBT hook
        GetInstance()->UnhookCBT();

        if ((hParentWnd != nullptr) &&
            (hChildWnd != nullptr) &&
            (GetWindowRect(GetDesktopWindow(), &rDesktop) != 0) &&
            (GetWindowRect(hParentWnd, &rParent) != 0) &&
            (GetWindowRect(hChildWnd, &rChild) != 0))
        {
            const POINT pStart = ComputeCenteredTopLeft(rParent, rChild, rDesktop);

            // move message box
            HWND hWndInsertAfter = nullptr;
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