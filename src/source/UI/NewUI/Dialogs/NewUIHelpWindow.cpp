// NewUIHelpWindow.cpp: implementation of the CNewUIHelpWindow class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UI/NewUI/Dialogs/NewUIHelpWindow.h"
#include "UI/NewUI/NewUISystem.h"
#include "Engine/Object/ZzzInventory.h"
#include "Audio/DSPlaySound.h"
#include "I18N/All.h"

using namespace SEASON3B;

namespace
{
    // Engine-only help-text entries. These three lines describe debug camera
    // hotkeys that the original game didn't ship with, so they live outside
    // the resx-driven translation pipeline; they're rendered as-is in the
    // F1 help panel.
    constexpr const wchar_t* kCustomHelpLines[] = {
        L"F9 - Toggle 3D Camera",
        L"F10 - Lock / Unlock Camera Zoom",
        L"F11 - Reset Camera View",
    };
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

        wcscpy(TextList[iTextNum], I18N::Game::KeyFunction);
        TextListColor[iTextNum] = TEXT_COLOR_BLUE;
        TextBold[iTextNum] = true;
        iTextNum++;

        mu_swprintf(TextList[iTextNum], L"\n");
        iTextNum++;

        // Render F1-F4 entries (I18N::Game::Lookup(121..124)) first.
        for (int i = 0; i < 4; ++i)
        {
            wcscpy(TextList[iTextNum], I18N::Game::Lookup(121 + i));
            TextListColor[iTextNum] = TEXT_COLOR_WHITE;
            TextBold[iTextNum] = false;
            iTextNum++;
        }

        // Insert engine-added F9 / F10 / F11 entries between F4 and the rest.
        for (const wchar_t* line : kCustomHelpLines)
        {
            wcsncpy_s(TextList[iTextNum], line, 99);
            TextListColor[iTextNum] = TEXT_COLOR_WHITE;
            TextBold[iTextNum] = false;
            iTextNum++;
        }

        // Render the remaining shipped entries (I18N::Game::Lookup(125..139)).
        for (int i = 4; i < 19; ++i)
        {
            wcscpy(TextList[iTextNum], I18N::Game::Lookup(121 + i));
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

        wcscpy(TextList[iTextNum], I18N::Game::ChattingInstructions);
        TextListColor[iTextNum] = TEXT_COLOR_BLUE;
        TextBold[iTextNum] = true;
        iTextNum++;

        mu_swprintf(TextList[iTextNum], L"\n");
        iTextNum++;

        for (int i = 0; i < 16; ++i)
        {
            wcscpy(TextList[iTextNum], I18N::Game::Lookup(141 + i));
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

        wcscpy(TextList[iTextNum], I18N::Game::AMPM);
        TextListColor[iTextNum] = TEXT_COLOR_BLUE;
        TextBold[iTextNum] = true;
        iTextNum++;

        for (int i = 0; i < 24; ++i)
        {
            wcscpy(TextList[iTextNum], I18N::Game::Lookup(2422 + i));
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

        wcscpy(TextList[iTextNum], I18N::Game::ChaosCastleDevilSSquare);
        TextListColor[iTextNum] = TEXT_COLOR_BLUE;
        TextBold[iTextNum] = true;
        iTextNum++;

        for (int i = 0; i < 18; ++i)
        {
            wcscpy(TextList[iTextNum], I18N::Game::Lookup(2447 + i));
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