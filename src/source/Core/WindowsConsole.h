#pragma once

#include "Core/Timer.h"

#include <string>

namespace leaf
{
// Console colour indices — self-contained, no Win32 FOREGROUND_* dependency.
// Numeric values match the original Win32 colour attribute layout (4-bit RGBI).
enum COLOR_INDEX
{
    COLOR_BLACK = 0,

    COLOR_DARKBLUE = 1,
    COLOR_DARKGREEN = 2,
    COLOR_TEAL = 3,
    COLOR_DARKRED = 4,
    COLOR_PURPLE = 5,
    COLOR_OLIVE = 6,
    COLOR_GRAY = 7,
    COLOR_INTENSITY = 8,

    COLOR_BLUE = 9,
    COLOR_GREEN = 10,
    COLOR_AQUA = 11,
    COLOR_RED = 12,
    COLOR_FUCHSIA = 13,
    COLOR_YELLOW = 14,
    COLOR_WHITE = 15,

    COLOR_ERROR = 0xFFFF
};

/* Public Console Functions */

bool OpenConsoleWindow(const std::wstring& title);
void CloseConsoleWindow();

bool SetConsoleTitle(const std::wstring& title);
const std::wstring& GetConsoleTitle();

bool IsConsoleVisible();
void ShowConsole(bool bShow = true);

void ClearConsoleScreen();

int GetConsoleTextColorIndex(int* pBgColorIndex = nullptr);
void SetConsoleTextColor(int textColorIndex = COLOR_WHITE, int bgColorIndex = COLOR_BLACK);

void ActivateCloseButton(bool bActive = true);
bool IsActiveCloseButton();

bool SaveConsoleScreenBuffer(const std::wstring& filename);

/* CConsoleWindow singleton class */

class CConsoleWindow
{
    CTimer2 m_LimitTimer;
    bool m_bActiveCloseButton;
    bool m_bVisible;
    bool m_started;
    int m_currentTextColor;
    int m_currentBgColor;
    std::wstring m_title;

public:
    ~CConsoleWindow();

    bool Open(const std::wstring& title);
    void Close();

    bool SetTitle(const std::wstring& title);
    const std::wstring& GetTitle();

    bool IsVisible();
    void Show(bool bShow = true);

    void ClearScreen();

    int GetTextColorIndex(int* pBgColorIndex = nullptr);
    void SetTextColor(int textColorIndex = COLOR_WHITE, int bgColorIndex = COLOR_BLACK);

    void ActivateCloseButton(bool bActive = true);
    bool IsActiveCloseButton() const;

    bool SaveScreenBuffer(const std::wstring& filename);

    static CConsoleWindow* GetInstance();

protected:
    CConsoleWindow();
};
} // namespace leaf
