#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers

#include "stdafx.h"

#include "WindowsConsole.h"

bool leaf::OpenConsoleWindow(const std::wstring& title)
{
    return CConsoleWindow::GetInstance()->Open(title);
}
void leaf::CloseConsoleWindow()
{
    CConsoleWindow::GetInstance()->Close();
}

bool leaf::SetConsoleTitle(const std::wstring& title)
{
    return CConsoleWindow::GetInstance()->SetTitle(title);
}
const std::wstring& leaf::GetConsoleTitle()
{
    return CConsoleWindow::GetInstance()->GetTitle();
}

HWND leaf::GetConsoleWndHandle()
{
    return CConsoleWindow::GetInstance()->GetWndHandle();
}

bool leaf::IsConsoleVisible()
{
    return CConsoleWindow::GetInstance()->IsVisible();
}
void leaf::ShowConsole(bool bShow)
{
    CConsoleWindow::GetInstance()->Show(bShow);
}

void leaf::ClearConsoleScreen()
{
    CConsoleWindow::GetInstance()->ClearScreen();
}

WORD leaf::GetConsoleTextColorIndex(WORD* pwBgColorIndex)
{
    return CConsoleWindow::GetInstance()->GetTextColorIndex(pwBgColorIndex);
}
void leaf::SetConsoleTextColor(WORD wTextColorIndex, WORD wBgColorIndex)
{
    CConsoleWindow::GetInstance()->SetTextColor(wTextColorIndex, wBgColorIndex);
}

void leaf::ActivateCloseButton(bool bActive)
{
    CConsoleWindow::GetInstance()->ActivateCloseButton(bActive);
}
bool leaf::IsActiveCloseButton()
{
    return CConsoleWindow::GetInstance()->IsActiveCloseButton();
}

bool leaf::SaveConsoleScreenBuffer(const std::wstring& filename)
{
    return CConsoleWindow::GetInstance()->SaveScreenBuffer(filename);
}

//. class CConsoleWindow
using namespace leaf;

CConsoleWindow::CConsoleWindow()
{
    m_hWnd = NULL;
    m_bActiveCloseButton = false;
    m_started = false;

    m_LimitTimer.SetTimer(12000);	//. 12ÃÊ
}
CConsoleWindow::~CConsoleWindow() {}

bool CConsoleWindow::Open(const std::wstring& title)
{
    Close();
    if (m_started)
        return true;
    m_started = true;

    if (FALSE == ::AllocConsole())
        return false;

    freopen("CONIN$", "r", stdin);
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);

    ::SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE),
        ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_PROCESSED_INPUT);
    ::SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE),
        ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT);
    ::SetConsoleMode(GetStdHandle(STD_ERROR_HANDLE),
        ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT);

    while (!GetWndHandle())
    {
        m_LimitTimer.UpdateTime();
        if (m_LimitTimer.IsTime() || FALSE == ::EnumChildWindows(NULL, EnumChildProc, (LPARAM)this))
            break;
        ::Sleep(500);
    }

    m_bActiveCloseButton = true;

    return true;
}
void CConsoleWindow::Close()
{
    fclose(stdin);
    fclose(stdout);
    fclose(stderr);

    FreeConsole();

    m_hWnd = NULL;
    m_started = false;
}

bool CConsoleWindow::SetTitle(const std::wstring& title)
{
    if (FALSE == ::SetConsoleTitle(title.c_str()))
        return false;
    return true;
}
const std::wstring& CConsoleWindow::GetTitle()
{
    wchar_t szConsoleTile[1024] = { 0, };
    ::GetConsoleTitle(szConsoleTile, 1024);

    static std::wstring s_title = szConsoleTile;
    return s_title;
}

HWND CConsoleWindow::GetWndHandle() { return m_hWnd; }

bool CConsoleWindow::IsVisible() { return GetWndHandle() && ::IsWindowVisible(GetWndHandle()) ? true : false; }
void CConsoleWindow::Show(bool bShow)
{
    if (m_hWnd)
    {
        BOOL bResult = FALSE;
        m_LimitTimer.ResetTimer();
        while (!bResult)
        {
            ::SetLastError(0);
            ::ShowWindow(m_hWnd, bShow ? SW_SHOW : SW_HIDE);
            ::UpdateWindow(m_hWnd);
            bResult = ::GetLastError() ? FALSE : TRUE;
            Sleep(10);
        }
    }
}

void CConsoleWindow::ClearScreen()
{
    /***************************************/
    // This code is from one of Microsoft's
    // knowledge base articles, you can find it at
    // http://support.microsoft.com/default.aspx?scid=KB;EN-US;q99261&
    /***************************************/

    COORD coordScreen = { 0, 0 };

    DWORD cCharsWritten;
    CONSOLE_SCREEN_BUFFER_INFO csbi; /* to get buffer info */
    DWORD dwConSize;

    /* get the number of character cells in the current buffer */
    ::GetConsoleScreenBufferInfo(::GetStdHandle(STD_OUTPUT_HANDLE), &csbi);

    dwConSize = csbi.dwSize.X * csbi.dwSize.Y;

    /* fill the entire screen with blanks */
    ::FillConsoleOutputCharacter(::GetStdHandle(STD_OUTPUT_HANDLE), (TCHAR)' ', dwConSize, coordScreen, &cCharsWritten);

    /* get the current text attribute */
    ::GetConsoleScreenBufferInfo(::GetStdHandle(STD_OUTPUT_HANDLE), &csbi);

    /* now set the buffer's attributes accordingly */
    ::FillConsoleOutputAttribute(::GetStdHandle(STD_OUTPUT_HANDLE), csbi.wAttributes, dwConSize, coordScreen, &cCharsWritten);

    /* put the cursor at (0, 0) */
    ::SetConsoleCursorPosition(::GetStdHandle(STD_OUTPUT_HANDLE), coordScreen);
}

WORD CConsoleWindow::GetTextColorIndex(WORD* pwBgColorIndex)
{
    CONSOLE_SCREEN_BUFFER_INFO ConScreenBufInfo;
    if (FALSE == ::GetConsoleScreenBufferInfo(::GetStdHandle(STD_OUTPUT_HANDLE), &ConScreenBufInfo))
        return 0xFFFF;

    WORD wTextColorIndex = ConScreenBufInfo.wAttributes & 0x000F;
    if (pwBgColorIndex)
    {
        *pwBgColorIndex = wTextColorIndex >> 4;
    }
    return wTextColorIndex;
}
void CConsoleWindow::SetTextColor(WORD wTextColorIndex, WORD wBgColorIndex)
{
    WORD wColorAttr = (wBgColorIndex << 4) & 0xF0;
    wColorAttr |= wTextColorIndex;

    ::SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), wColorAttr);
}

void CConsoleWindow::ActivateCloseButton(bool bActive)
{
    if (bActive && (m_hWnd != NULL))
    {
        // enable the [x] button if we found our console
        ::GetSystemMenu(m_hWnd, TRUE);
        ::DrawMenuBar(m_hWnd);
        m_bActiveCloseButton = true;
    }
    else if (m_hWnd != NULL)
    {
        // disable the [x] button if we found our console
        HMENU hMenu = ::GetSystemMenu(m_hWnd, FALSE);
        if (hMenu != NULL) {
            ::RemoveMenu(hMenu, SC_CLOSE, MF_BYCOMMAND);
            ::DrawMenuBar(m_hWnd);
            m_bActiveCloseButton = false;
        }
    }
}
bool CConsoleWindow::IsActiveCloseButton() const { return m_bActiveCloseButton; }

bool CConsoleWindow::SaveScreenBuffer(const std::wstring& filename)
{
    CONSOLE_SCREEN_BUFFER_INFO csbi; /* to get buffer info */

    /* get the number of character cells in the current buffer */
    ::GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);

    COORD BufferSize;
    BufferSize.X = csbi.dwSize.X;
    BufferSize.Y = csbi.dwCursorPosition.Y + 1;

    auto* pbyCharBuffer = new CHAR_INFO[BufferSize.X * BufferSize.Y];

    COORD StartPointToWrite = { 0, 0 };
    SMALL_RECT RectToRead = { 0, 0, BufferSize.X, BufferSize.Y };
    if (ReadConsoleOutput(GetStdHandle(STD_OUTPUT_HANDLE), pbyCharBuffer, BufferSize, StartPointToWrite, &RectToRead))
    {
    }

    delete[] pbyCharBuffer;

    return true;
}

CConsoleWindow* CConsoleWindow::GetInstance()
{
    static CConsoleWindow s_ConsoleWindow;
    return &s_ConsoleWindow;
}

void CConsoleWindow::SetWndHandle(HWND hWnd)
{
    m_hWnd = hWnd;
}
DWORD CConsoleWindow::Get32ColorFromColorIndex(WORD wColorIndex)
{
    DWORD dw32Color = 0;
    if ((wColorIndex & leaf::COLOR_RED) == leaf::COLOR_RED)
        dw32Color |= 0x007F0000;
    if ((wColorIndex & leaf::COLOR_GREEN) == leaf::COLOR_GREEN)
        dw32Color |= 0x00007F00;
    if ((wColorIndex & leaf::COLOR_BLUE) == leaf::COLOR_BLUE)
        dw32Color |= 0x0000007F;
    if ((wColorIndex & leaf::COLOR_INTENSITY) == leaf::COLOR_INTENSITY)
    {
        if ((dw32Color & 0x7F0000) == 0x7F0000)
            dw32Color |= 0x800000;
        if ((dw32Color & 0x007F00) == 0x007F00)
            dw32Color |= 0x008000;
        if ((dw32Color & 0x00007F) == 0x00007F)
            dw32Color |= 0x000080;
    }
    return dw32Color;
}
BOOL CALLBACK CConsoleWindow::EnumChildProc(HWND hWnd, LPARAM lParam)
{
    auto* pConsoleWnd = (CConsoleWindow*)(lParam);
    DWORD dwProcessId = 0;
    if (GetWindowThreadProcessId(hWnd, &dwProcessId) == GetCurrentThreadId()
        && dwProcessId == GetCurrentProcessId())
    {
        wchar_t szClassName[512] = { 0, };
        GetClassName(hWnd, szClassName, 512);
        if (0 == wcscmp(szClassName, L"ConsoleWindowClass"))
        {
            if (!pConsoleWnd->GetWndHandle())
                pConsoleWnd->SetWndHandle(hWnd);
            return FALSE;
        }
    }
    return TRUE;
}