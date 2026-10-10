#include "stdafx.h"
#include "Core/Input/ImeInput.h"

#include "Core/Platform/Imm.h"
#include "App/Platform/Windows/Winmain.h"   // g_hWnd, WindowWidth/Height
#include "Engine/Object/ZzzInterface.h" // LockInputStatus
#include "Render/Text/CUIRenderText.h"

// Defined in Winmain.cpp; declared per-TU across the codebase (no header).
extern int g_iChatInputType;

namespace
{
    DWORD g_dwOldConv = IME_CMODE_ALPHANUMERIC;
    DWORD g_dwOldSent = IME_SMODE_AUTOMATIC;
    DWORD g_dwCurrConv = 0;
}

namespace Input::IME
{
    void SetCompositionWindow(float x, float y)
    {
        float xRatio_Input = x / (float)REFERENCE_WIDTH;
        float yRatio_Input = y / (float)REFERENCE_HEIGHT;

        COMPOSITIONFORM comForm;
        comForm.dwStyle = CFS_POINT;
        comForm.ptCurrentPos.x = (long)(WindowWidth * xRatio_Input);
        comForm.ptCurrentPos.y = (long)(WindowHeight * yRatio_Input);
        SetRect(&comForm.rcArea, 0, 0, WindowWidth, WindowHeight);

        HWND hWnd = ImmGetDefaultIMEWnd(g_hWnd);
        ::SendMessage(hWnd, WM_IME_CONTROL, IMC_SETCOMPOSITIONWINDOW, (LPARAM)&comForm);
    }

    void SaveStatus()
    {
        HIMC data = ::ImmGetContext(g_hWnd);
        ::ImmGetConversionStatus(data, &g_dwOldConv, &g_dwOldSent);
        ::ImmReleaseContext(g_hWnd, data);
    }

    void SetStatus(bool halfShape)
    {
        if (g_iChatInputType == 1) return;

        HIMC data = ::ImmGetContext(g_hWnd);

        DWORD dwConv = g_dwOldConv;
        DWORD dwSent = g_dwOldSent;
        if (halfShape)
        {
            dwConv = IME_CMODE_ALPHANUMERIC;
            dwSent = IME_SMODE_NONE;
        }
        ::ImmSetConversionStatus(data, dwConv, dwSent);
        ::ImmReleaseContext(g_hWnd, data);
    }

    bool CheckStatus(bool change, int mode)
    {
        if (g_iChatInputType == 1) return false;

        HIMC data = ::ImmGetContext(g_hWnd);

        DWORD dwConv, dwSent;
        ::ImmGetConversionStatus(data, &dwConv, &dwSent);

        bool bIme = false;
        if (dwConv != IME_CMODE_ALPHANUMERIC || dwSent != IME_SMODE_NONE)
        {
            bIme = true;

            if ((mode & IME_CONVERSIONMODE) == IME_CONVERSIONMODE)
            {
                g_dwOldConv = dwConv;
            }
            if ((mode & IME_SENTENCEMODE) == IME_SENTENCEMODE)
            {
                g_dwOldSent = dwSent;
            }

            if (change)
            {
                dwConv = IME_CMODE_ALPHANUMERIC;
                dwSent = IME_SMODE_NONE;
                ::ImmSetConversionStatus(data, dwConv, dwSent);
            }
        }
        ::ImmReleaseContext(g_hWnd, data);

        g_dwCurrConv = dwConv;

        return bIme;
    }

    void RenderStatus()
    {
        wchar_t Text[100];
        if ((g_dwOldConv & IME_CMODE_NATIVE) == IME_CMODE_NATIVE)
        {
            mu_swprintf(Text, L"ENGLISH");
        }
        else
        {
            mu_swprintf(Text, L"ENGLISH");
        }

        g_pRenderText->SetTextColor(255, 230, 210, 255);
        g_pRenderText->SetBgColor(0);
        g_pRenderText->RenderText(100, 100, Text);

        HIMC data = ::ImmGetContext(g_hWnd);

        DWORD dwConv, dwSent;
        ::ImmGetConversionStatus(data, &dwConv, &dwSent);
        ::ImmReleaseContext(g_hWnd, data);

        mu_swprintf(Text, L"Sentence Mode = %d", dwSent);
        g_pRenderText->RenderText(100, 110, Text);

        mu_swprintf(Text, L"Old Sentence Mode = %d", g_dwOldSent);
        g_pRenderText->RenderText(100, 120, Text);

        mu_swprintf(Text, L"LockInputStatus=%d", LockInputStatus);
        g_pRenderText->RenderText(100, 130, Text);
    }
}

// The legacy text-input boxes keep their own conversion-mode backup, separate from the
// Input::IME stack above: the two save and restore independently, so they are not
// interchangeable. CharacterScene.cpp reads g_dwBKConv/g_dwBKSent directly instead of
// calling SaveIMEStatus().

DWORD g_dwBKConv = IME_CMODE_ALPHANUMERIC;
DWORD g_dwBKSent = IME_SMODE_NONE;
BOOL g_bForceIMEConv = FALSE;
BOOL g_bForceIMESent = FALSE;
BOOL g_bBKOpenState = TRUE;
BOOL g_bIMEBlock = FALSE;

void SaveIMEStatus()
{
    HIMC hIMC = ImmGetContext(g_hWnd);
    ImmGetConversionStatus(hIMC, &g_dwBKConv, &g_dwBKSent);
    ImmSetConversionStatus(hIMC, IME_CMODE_ALPHANUMERIC, IME_SMODE_NONE);
    ImmReleaseContext(g_hWnd, hIMC);
}

void RestoreIMEStatus()
{
    HIMC hIMC = ImmGetContext(g_hWnd);
    ImmSetConversionStatus(hIMC, g_dwBKConv, g_dwBKSent);
    ImmReleaseContext(g_hWnd, hIMC);
}

void CheckTextInputBoxIME(int iMode)
{
    if (g_bIMEBlock == FALSE) return;
    if (iMode & IME_CONVERSIONMODE)
    {
        if (/*InputEnable == false && */g_bForceIMEConv == TRUE)
        {
            g_bForceIMEConv = FALSE;
            return;
        }

        HIMC hIMC = ImmGetContext(g_hWnd);

        DWORD dwConv, dwSent;
        ImmGetConversionStatus(hIMC, &dwConv, &dwSent);
        if (dwConv != IME_CMODE_ALPHANUMERIC)
        {
            g_dwBKConv = dwConv;
            g_bForceIMEConv = TRUE;
            ImmSetConversionStatus(hIMC, IME_CMODE_ALPHANUMERIC, IME_SMODE_NONE);
        }

        ImmReleaseContext(g_hWnd, hIMC);
        //		g_bForceIMEConv = FALSE;
    }
    if (iMode & IME_SENTENCEMODE)
    {
        if (/*InputEnable == false && */g_bForceIMESent == TRUE)
        {
            g_bForceIMESent = FALSE;
            return;
        }

        HIMC hIMC = ImmGetContext(g_hWnd);
        DWORD dwConv, dwSent;
        ImmGetConversionStatus(hIMC, &dwConv, &dwSent);
        if (dwSent != IME_SMODE_NONE)
        {
            g_dwBKSent = dwSent;
            g_bForceIMESent = TRUE;
            ImmSetConversionStatus(hIMC, IME_CMODE_ALPHANUMERIC, IME_SMODE_NONE);
        }
        ImmReleaseContext(g_hWnd, hIMC);
        //		g_bForceIMESent = FALSE;
    }
}
