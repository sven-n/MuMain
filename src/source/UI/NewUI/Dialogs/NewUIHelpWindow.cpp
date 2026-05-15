// NewUIHelpWindow.cpp: implementation of the CNewUIHelpWindow class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UI/NewUI/Dialogs/NewUIHelpWindow.h"
#include "UI/NewUI/NewUISystem.h"
#include "Engine/Object/ZzzInventory.h"
#include "Audio/DSPlaySound.h"

using namespace SEASON3B;

namespace
{
    // Engine-only help-text entries registered programmatically via
    // SEASON3B::RegisterCustomHelpText(). Keys are above MAX_NUMBER_OF_TEXTS
    // (9999) so they can't collide with shipped GlobalText.bmd entries.
    constexpr int HELP_KEY_F9_TOGGLE_CAMERA   = 10000;
    constexpr int HELP_KEY_F10_TOGGLE_ZOOM    = 10001;
    constexpr int HELP_KEY_F11_RESET_VIEW     = 10002;
}

void SEASON3B::RegisterCustomHelpText()
{
    GlobalText.Add(HELP_KEY_F9_TOGGLE_CAMERA, L"F9 - Toggle 3D Camera");
    GlobalText.Add(HELP_KEY_F10_TOGGLE_ZOOM,  L"F10 - Lock / Unlock Camera Zoom");
    GlobalText.Add(HELP_KEY_F11_RESET_VIEW,   L"F11 - Reset Camera View");
}

SEASON3B::CNewUIHelpWindow::CNewUIHelpWindow()
{
    m_pNewUIMng = NULL;
    m_Pos.x = 0;
    m_Pos.y = 0;

    m_iIndex = 0;
}

SEASON3B::CNewUIHelpWindow::~CNewUIHelpWindow()
{
    Release();
}

bool SEASON3B::CNewUIHelpWindow::Create(CNewUIManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_HELP, this);

    SetPos(x, y);

    Show(false);

    return true;
}

void SEASON3B::CNewUIHelpWindow::Release()
{
    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void SEASON3B::CNewUIHelpWindow::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

bool SEASON3B::CNewUIHelpWindow::UpdateMouseEvent()
{
    return true;
}

bool SEASON3B::CNewUIHelpWindow::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_HELP))
    {
        if (IsPress(VK_F1) == true)
        {
            if (++m_iIndex > 1)
            {
                g_pNewUISystem->Hide(SEASON3B::INTERFACE_HELP);
                PlayBuffer(SOUND_CLICK01);
            }

            return false;
        }

        if (IsPress(VK_ESCAPE) == true)
        {
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_HELP);
            PlayBuffer(SOUND_CLICK01);

            return false;
        }
    }

    return true;
}

bool SEASON3B::CNewUIHelpWindow::Update()
{
    return true;
}

bool SEASON3B::CNewUIHelpWindow::Render()
{
    EnableAlphaTest();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    // Reference-bind to the global arrays in ZzzInventory.cpp. A naive
    // `extern wchar_t TextList[50][100];` here would resolve to
    // SEASON3B::TextList (the reference defined in UIManager.cpp) because
    // this function is in the SEASON3B namespace -- and the linker stores
    // that reference as a 4-byte read-only pointer, so writing to it crashes.
    wchar_t (&TextList)[50][100] = ::TextList;
    int (&TextListColor)[50] = ::TextListColor;
    int (&TextBold)[50] = ::TextBold;

    if (m_iIndex == 0)
    {
        int iTextNum = 0;

        mu_swprintf(TextList[iTextNum], L"\n");
        iTextNum++;

        wcscpy(TextList[iTextNum], GlobalText[120]);
        TextListColor[iTextNum] = TEXT_COLOR_BLUE;
        TextBold[iTextNum] = true;
        iTextNum++;

        mu_swprintf(TextList[iTextNum], L"\n");
        iTextNum++;

        // Render F1-F4 entries (GlobalText[121..124]) first.
        for (int i = 0; i < 4; ++i)
        {
            wcscpy(TextList[iTextNum], GlobalText[121 + i]);
            TextListColor[iTextNum] = TEXT_COLOR_WHITE;
            TextBold[iTextNum] = false;
            iTextNum++;
        }

        // Insert engine-added F9 / F10 / F11 entries between F4 and the rest.
        // wcsncpy + explicit terminator: these GlobalText entries can be
        // overridden by .bmd translations of arbitrary length, so guard
        // the 100-wchar TextList row against overflow.
        const int kCustomKeys[] = {
            HELP_KEY_F9_TOGGLE_CAMERA,
            HELP_KEY_F10_TOGGLE_ZOOM,
            HELP_KEY_F11_RESET_VIEW,
        };
        for (int key : kCustomKeys)
        {
            wcsncpy_s(TextList[iTextNum], GlobalText[key], 99);
            TextListColor[iTextNum] = TEXT_COLOR_WHITE;
            TextBold[iTextNum] = false;
            iTextNum++;
        }

        // Render the remaining shipped entries (GlobalText[125..139]).
        for (int i = 4; i < 19; ++i)
        {
            wcscpy(TextList[iTextNum], GlobalText[121 + i]);
            TextListColor[iTextNum] = TEXT_COLOR_WHITE;
            TextBold[iTextNum] = false;
            iTextNum++;
        }

        mu_swprintf(TextList[iTextNum], L"\n");
        iTextNum++;

        RenderTipTextList(1, 1, iTextNum, 0, RT3_SORT_CENTER);
    }
    else if (m_iIndex == 1)
    {
        int iTextNum = 0;

        mu_swprintf(TextList[iTextNum], L"\n");
        iTextNum++;

        wcscpy(TextList[iTextNum], GlobalText[140]);
        TextListColor[iTextNum] = TEXT_COLOR_BLUE;
        TextBold[iTextNum] = true;
        iTextNum++;

        mu_swprintf(TextList[iTextNum], L"\n");
        iTextNum++;

        for (int i = 0; i < 16; ++i)
        {
            wcscpy(TextList[iTextNum], GlobalText[141 + i]);
            TextListColor[iTextNum] = TEXT_COLOR_WHITE;
            TextBold[iTextNum] = false;
            iTextNum++;
        }

        mu_swprintf(TextList[iTextNum], L"\n");
        iTextNum++;

        RenderTipTextList(1, 1, iTextNum, 0, RT3_SORT_CENTER);
    }
    else if (m_iIndex == 2)
    {
        int iTextNum = 0;

        mu_swprintf(TextList[iTextNum], L"\n");
        iTextNum++;

        wcscpy(TextList[iTextNum], GlobalText[2421]);
        TextListColor[iTextNum] = TEXT_COLOR_BLUE;
        TextBold[iTextNum] = true;
        iTextNum++;

        for (int i = 0; i < 24; ++i)
        {
            wcscpy(TextList[iTextNum], GlobalText[2422 + i]);
            TextListColor[iTextNum] = TEXT_COLOR_WHITE;
            TextBold[iTextNum] = false;
            iTextNum++;
        }

        mu_swprintf(TextList[iTextNum], L"\n");
        iTextNum++;

        RenderTipTextList(1, 1, iTextNum, 0, RT3_SORT_LEFT);
    }
    else if (m_iIndex == 3)
    {
        int iTextNum = 0;

        mu_swprintf(TextList[iTextNum], L"\n");
        iTextNum++;

        wcscpy(TextList[iTextNum], GlobalText[2446]);
        TextListColor[iTextNum] = TEXT_COLOR_BLUE;
        TextBold[iTextNum] = true;
        iTextNum++;

        for (int i = 0; i < 18; ++i)
        {
            wcscpy(TextList[iTextNum], GlobalText[2447 + i]);
            if (i == 0 || i == 8 || i == 9)
            {
                TextListColor[iTextNum] = TEXT_COLOR_BLUE;
                TextBold[iTextNum] = true;
                iTextNum++;
            }
            else
            {
                TextListColor[iTextNum] = TEXT_COLOR_WHITE;
                TextBold[iTextNum] = false;
                iTextNum++;
            }
        }

        mu_swprintf(TextList[iTextNum], L"\n");
        iTextNum++;

        RenderTipTextList(1, 1, iTextNum, 0, RT3_SORT_LEFT);
    }

    DisableAlphaBlend();
    return true;
}

float SEASON3B::CNewUIHelpWindow::GetLayerDepth()
{
    return 8.3f;
}

float SEASON3B::CNewUIHelpWindow::GetKeyEventOrder()
{
    return 10.f;
}

void SEASON3B::CNewUIHelpWindow::OpenningProcess()
{
    m_iIndex = 0;
}

void SEASON3B::CNewUIHelpWindow::ClosingProcess()
{
}

void SEASON3B::CNewUIHelpWindow::AutoUpdateIndex()
{
    if (++m_iIndex > 1) {
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_HELP);
    }
}