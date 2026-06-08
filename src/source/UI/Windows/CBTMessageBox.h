#include <string>

// Centers/raises the native Win32 MessageBox via a CBT hook. Windows-only and
// currently unused; portable code uses the SDL-backed MessageBox (WinUser.h).
#ifdef _WIN32

namespace leaf {
    int CBTMessageBox(HWND hWnd, const std::wstring& text, const std::wstring& caption, UINT uType, bool bAlwaysOnTop = false);

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

#endif // _WIN32
