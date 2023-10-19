// NewUIHelpWindow.cpp: implementation of the CNewUIHelpWindow class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUIHelpWindow.h"
#include "NewUISystem.h"
#include "ZzzInventory.h"
#include "DSPlaySound.h"

using namespace SEASON3B;

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

    extern wchar_t TextList[50][100];
    extern int TextListColor[50];
    extern int TextBold[50];

    if (m_iIndex == 0)
    {
        int iTextNum = 0;

        swprintf(TextList[iTextNum], L"\n");
        iTextNum++;

        wcscpy(TextList[iTextNum], GlobalText[120]);
        TextListColor[iTextNum] = TEXT_COLOR_BLUE;
        TextBold[iTextNum] = true;
        iTextNum++;

        swprintf(TextList[iTextNum], L"\n");
        iTextNum++;

        for (int i = 0; i < 19; ++i)
        {
            wcscpy(TextList[iTextNum], GlobalText[121 + i]);
            TextListColor[iTextNum] = TEXT_COLOR_WHITE;
            TextBold[iTextNum] = false;
            iTextNum++;
        }

        swprintf(TextList[iTextNum], L"\n");
        iTextNum++;

        RenderTipTextList(1, 1, iTextNum, 0, RT3_SORT_CENTER);
    }
    else if (m_iIndex == 1)
    {
        int iTextNum = 0;

        swprintf(TextList[iTextNum], L"\n");
        iTextNum++;

        wcscpy(TextList[iTextNum], GlobalText[140]);
        TextListColor[iTextNum] = TEXT_COLOR_BLUE;
        TextBold[iTextNum] = true;
        iTextNum++;

        swprintf(TextList[iTextNum], L"\n");
        iTextNum++;

        for (int i = 0; i < 16; ++i)
        {
            wcscpy(TextList[iTextNum], GlobalText[141 + i]);
            TextListColor[iTextNum] = TEXT_COLOR_WHITE;
            TextBold[iTextNum] = false;
            iTextNum++;
        }

        swprintf(TextList[iTextNum], L"\n");
        iTextNum++;

        RenderTipTextList(1, 1, iTextNum, 0, RT3_SORT_CENTER);
    }
    else if (m_iIndex == 2)
    {
        int iTextNum = 0;

        swprintf(TextList[iTextNum], L"\n");
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

        swprintf(TextList[iTextNum], L"\n");
        iTextNum++;

        RenderTipTextList(1, 1, iTextNum, 0, RT3_SORT_LEFT);
    }
    else if (m_iIndex == 3)
    {
        int iTextNum = 0;

        swprintf(TextList[iTextNum], L"\n");
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

        swprintf(TextList[iTextNum], L"\n");
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