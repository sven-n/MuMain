// Story 7.6.5: Cross-platform terminal/console using ANSI escape sequences.
// No Win32 console APIs — all platforms use the same ANSI code path.
// [VS0-QUAL-WIN32CLEAN-CONSOLE]

#include "stdafx.h"

#include "WindowsConsole.h"

#include "PlatformCompat.h"

// ---- Win32 COLOR_INDEX → ANSI SGR code mapping ----
// Maps the 4-bit RGBI colour index (0–15) to an ANSI SGR foreground code.
// Dark colours: 30–37, bright colours: 90–97.
static int ColorIndexToAnsiFg(int colorIndex)
{
    // clang-format off
    static const int kAnsiTable[16] = {
        30,  // 0  COLOR_BLACK
        34,  // 1  COLOR_DARKBLUE
        32,  // 2  COLOR_DARKGREEN
        36,  // 3  COLOR_TEAL (dark cyan)
        31,  // 4  COLOR_DARKRED
        35,  // 5  COLOR_PURPLE (dark magenta)
        33,  // 6  COLOR_OLIVE (dark yellow)
        37,  // 7  COLOR_GRAY (light gray)
        90,  // 8  COLOR_INTENSITY (dark gray / bright black)
        94,  // 9  COLOR_BLUE (bright blue)
        92,  // 10 COLOR_GREEN (bright green)
        96,  // 11 COLOR_AQUA (bright cyan)
        91,  // 12 COLOR_RED (bright red)
        95,  // 13 COLOR_FUCHSIA (bright magenta)
        93,  // 14 COLOR_YELLOW (bright yellow)
        97,  // 15 COLOR_WHITE (bright white)
    };
    // clang-format on
    if (colorIndex < 0 || colorIndex > 15)
    {
        return 37; // fallback: light gray
    }
    return kAnsiTable[colorIndex];
}

// Maps the 4-bit RGBI colour index (0–15) to an ANSI SGR background code.
static int ColorIndexToAnsiBg(int colorIndex)
{
    // Background codes are foreground + 10
    return ColorIndexToAnsiFg(colorIndex) + 10;
}

//. Public functions — delegate to singleton

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

int leaf::GetConsoleTextColorIndex(int* pBgColorIndex)
{
    return CConsoleWindow::GetInstance()->GetTextColorIndex(pBgColorIndex);
}
void leaf::SetConsoleTextColor(int textColorIndex, int bgColorIndex)
{
    CConsoleWindow::GetInstance()->SetTextColor(textColorIndex, bgColorIndex);
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
    m_bActiveCloseButton = false;
    m_bVisible = false;
    m_started = false;
    m_currentTextColor = COLOR_WHITE;
    m_currentBgColor = COLOR_BLACK;

    m_LimitTimer.SetTimer(12000);
}
CConsoleWindow::~CConsoleWindow() {}

bool CConsoleWindow::Open(const std::wstring& title)
{
    if (m_started)
    {
        return true; // Already open
    }
    Close();
    m_started = true;

    mu_console_init();

    m_bActiveCloseButton = true;
    m_bVisible = true;
    m_title = title;
    mu_set_console_title(title.c_str());

    return true;
}

void CConsoleWindow::Close()
{
    m_bVisible = false;
    m_started = false;
}

bool CConsoleWindow::SetTitle(const std::wstring& title)
{
    m_title = title;
    mu_set_console_title(title.c_str());
    return true;
}

const std::wstring& CConsoleWindow::GetTitle()
{
    return m_title;
}

bool CConsoleWindow::IsVisible()
{
    return m_bVisible;
}

void CConsoleWindow::Show(bool bShow)
{
    m_bVisible = bShow;
}

void CConsoleWindow::ClearScreen()
{
    mu_console_clear();
}

int CConsoleWindow::GetTextColorIndex(int* pBgColorIndex)
{
    if (pBgColorIndex != nullptr)
    {
        *pBgColorIndex = m_currentBgColor;
    }
    return m_currentTextColor;
}

void CConsoleWindow::SetTextColor(int textColorIndex, int bgColorIndex)
{
    m_currentTextColor = textColorIndex;
    m_currentBgColor = bgColorIndex;

    if (bgColorIndex == COLOR_BLACK)
    {
        // Foreground only — simpler escape
        mu_set_console_text_color(ColorIndexToAnsiFg(textColorIndex));
    }
    else
    {
        // Foreground + background — routed through platform abstraction
        mu_set_console_text_color_with_bg(ColorIndexToAnsiFg(textColorIndex), ColorIndexToAnsiBg(bgColorIndex));
    }
}

void CConsoleWindow::ActivateCloseButton(bool bActive)
{
    m_bActiveCloseButton = bActive;
}

bool CConsoleWindow::IsActiveCloseButton() const
{
    return m_bActiveCloseButton;
}

bool CConsoleWindow::SaveScreenBuffer(const std::wstring& /*filename*/)
{
    // Screen buffer capture has no cross-platform ANSI equivalent.
    // The Win32 ReadConsoleOutput API was removed; this is now a no-op.
    // Return false to indicate the operation is unsupported.
    return false;
}

CConsoleWindow* CConsoleWindow::GetInstance()
{
    static CConsoleWindow s_ConsoleWindow;
    return &s_ConsoleWindow;
}
