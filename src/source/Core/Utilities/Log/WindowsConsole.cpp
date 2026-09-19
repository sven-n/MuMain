#include "stdafx.h"

#include "WindowsConsole.h"

#include <cstdio>

#ifdef fwprintf
#undef fwprintf
#endif

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

namespace
{
bool IsTerminal()
{
#ifdef _WIN32
    return _isatty(_fileno(stdout)) != 0;
#else
    return isatty(fileno(stdout)) != 0;
#endif
}

int ToAnsiForeground(WORD color)
{
    static constexpr int codes[] = {30, 34, 32, 36, 31, 35, 33, 37, 90, 94, 92, 96, 91, 95, 93, 97};
    return color < 16 ? codes[color] : 37;
}

int ToAnsiBackground(WORD color)
{
    return ToAnsiForeground(color) + 10;
}
} // namespace

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
    return nullptr;
}
bool leaf::IsConsoleVisible()
{
    return CConsoleWindow::GetInstance()->IsVisible();
}
void leaf::ShowConsole(bool show)
{
    CConsoleWindow::GetInstance()->Show(show);
}
void leaf::ClearConsoleScreen()
{
    CConsoleWindow::GetInstance()->ClearScreen();
}
WORD leaf::GetConsoleTextColorIndex(WORD* background)
{
    return CConsoleWindow::GetInstance()->GetTextColorIndex(background);
}
void leaf::SetConsoleTextColor(WORD text, WORD background)
{
    CConsoleWindow::GetInstance()->SetTextColor(text, background);
}
void leaf::ActivateCloseButton(bool active)
{
    CConsoleWindow::GetInstance()->ActivateCloseButton(active);
}
bool leaf::IsActiveCloseButton()
{
    return CConsoleWindow::GetInstance()->IsActiveCloseButton();
}
bool leaf::SaveConsoleScreenBuffer(const std::wstring& filename)
{
    return CConsoleWindow::GetInstance()->SaveScreenBuffer(filename);
}

leaf::CConsoleWindow::CConsoleWindow()
{
    m_LimitTimer.SetTimer(12000);
}
leaf::CConsoleWindow::~CConsoleWindow() = default;

bool leaf::CConsoleWindow::Open(const std::wstring& title)
{
    if (m_started)
    {
        return true;
    }
    m_started = true;
    m_visible = true;
    m_bActiveCloseButton = true;
    return SetTitle(title);
}

void leaf::CConsoleWindow::Close()
{
    if (IsTerminal())
    {
        std::fputs("\033[0m", stdout);
        std::fflush(stdout);
    }
    m_started = false;
    m_visible = false;
    m_textColor = COLOR_WHITE;
    m_backgroundColor = COLOR_BLACK;
}

bool leaf::CConsoleWindow::SetTitle(const std::wstring& title)
{
    m_title = title;
    if (IsTerminal())
    {
        std::fwprintf(stdout, L"\033]0;%ls\007", title.c_str());
        std::fflush(stdout);
    }
    return true;
}

const std::wstring& leaf::CConsoleWindow::GetTitle()
{
    return m_title;
}
HWND leaf::CConsoleWindow::GetWndHandle()
{
    return nullptr;
}
bool leaf::CConsoleWindow::IsVisible()
{
    return m_visible;
}
void leaf::CConsoleWindow::Show(bool show)
{
    m_visible = show;
}

void leaf::CConsoleWindow::ClearScreen()
{
    if (IsTerminal())
    {
        std::fputs("\033[2J\033[H", stdout);
        std::fflush(stdout);
    }
}

WORD leaf::CConsoleWindow::GetTextColorIndex(WORD* background)
{
    if (background != nullptr)
    {
        *background = m_backgroundColor;
    }
    return m_textColor;
}

void leaf::CConsoleWindow::SetTextColor(WORD text, WORD background)
{
    m_textColor = text;
    m_backgroundColor = background;
    if (IsTerminal())
    {
        std::fprintf(stdout, "\033[%d;%dm", ToAnsiForeground(text), ToAnsiBackground(background));
        std::fflush(stdout);
    }
}

void leaf::CConsoleWindow::ActivateCloseButton(bool active)
{
    m_bActiveCloseButton = active;
}
bool leaf::CConsoleWindow::IsActiveCloseButton() const
{
    return m_bActiveCloseButton;
}
bool leaf::CConsoleWindow::SaveScreenBuffer(const std::wstring&)
{
    return false;
}

leaf::CConsoleWindow* leaf::CConsoleWindow::GetInstance()
{
    static CConsoleWindow console;
    return &console;
}
