//////////////////////////////////////////////////////////////////////
// NewUIWindowMenu.cpp: implementation of the CNewUIWindowMenu class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUIWindowMenu.h"
#include "NewUISystem.h"
#include "NewUICustomMessageBox.h"
#include "ZzzTexture.h"
#include "UIControls.h"
#include "DSPlaySound.h"

using namespace SEASON3B;

SEASON3B::CNewUIWindowMenu::CNewUIWindowMenu()
{
    m_pNewUIMng = NULL;
    m_Pos.x = m_Pos.y = 0;
    m_iSelectedIndex = -1;
}

SEASON3B::CNewUIWindowMenu::~CNewUIWindowMenu()
{
    Release();
}

bool SEASON3B::CNewUIWindowMenu::Create(CNewUIManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_WINDOW_MENU, this);

    SetPos(x, y);

    LoadImages();

    Show(false);

    return true;
}

void SEASON3B::CNewUIWindowMenu::Release()
{
    UnloadImages();

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void SEASON3B::CNewUIWindowMenu::SetPos(int x, int y)
{
    m_Pos.x = STANDARD_POS_X;
    m_Pos.y = STANDARD_POS_Y - (20 * (MENU_MAX_INDEX - 4));
}

bool SEASON3B::CNewUIWindowMenu::UpdateMouseEvent()
{
    POINT pt = { m_Pos.x, m_Pos.y + 20 };

    for (int i = 0; i < MENU_MAX_INDEX; ++i)
    {
        if (CheckMouseIn(pt.x, pt.y, 112, 20) == true)
        {
            m_iSelectedIndex = i;
            break;
        }

        pt.y += 20.f;
    }

    if (m_iSelectedIndex > -1 && SEASON3B::IsRelease(VK_LBUTTON))
    {
        switch (m_iSelectedIndex)
        {
        case 0:
        {
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_WINDOW_MENU);
            SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CSystemMenuMsgBoxLayout));
            return false;
        }
        break;
        case 1:
        {
            if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_HELP))
            {
                g_pHelp->AutoUpdateIndex();
            }
            else
            {
                g_pNewUISystem->Show(SEASON3B::INTERFACE_HELP);
            }
            return false;
        }
        break;
        case 2:
        {
            g_pNewUISystem->Show(SEASON3B::INTERFACE_GUILDINFO);
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_WINDOW_MENU);
            return false;
        }
        break;
        case 3:
        {
            g_pNewUISystem->Toggle(SEASON3B::INTERFACE_MOVEMAP);
            return false;
        }
        break;
        case 4:
        {
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_WINDOW_MENU);
            if (g_pNewUIMiniMap->m_bSuccess == false)
            {
                g_pNewUISystem->Hide(SEASON3B::INTERFACE_MINI_MAP);
            }
            else
                g_pNewUISystem->Toggle(SEASON3B::INTERFACE_MINI_MAP);
            return false;
        }
        break;
        case 5:
        {
            if (g_pNewUIGensRanking->SetGensInfo())
            {
                g_pNewUISystem->Show(SEASON3B::INTERFACE_GENSRANKING);
                g_pNewUISystem->Hide(SEASON3B::INTERFACE_WINDOW_MENU);
            }
            return false;
        }
        break;
        }
    }

    if (CheckMouseIn(m_Pos.x, m_Pos.y + 20, 112, 95 + 20 * (MENU_MAX_INDEX - 5)) == false)
    {
        m_iSelectedIndex = -1;
    }
    else
    {
        return false;
    }

    return true;
}

bool SEASON3B::CNewUIWindowMenu::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_WINDOW_MENU) == true)
    {
        if (SEASON3B::IsPress(VK_ESCAPE) == true)
        {
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_WINDOW_MENU);
            PlayBuffer(SOUND_CLICK01);

            return false;
        }
    }

    return true;
}

bool SEASON3B::CNewUIWindowMenu::Update()
{
    return true;
}

bool SEASON3B::CNewUIWindowMenu::Render()
{
    EnableAlphaTest();

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    RenderFrame();
    RenderTexts();
    RenderArrow();

    DisableAlphaBlend();

    return true;
}

void SEASON3B::CNewUIWindowMenu::RenderFrame()
{
    float x, y, width, height;

    x = m_Pos.x; y = m_Pos.y; width = 112.f; height = 105 + (MENU_MAX_INDEX - 4) * 20;

    RenderBitmap(IMAGE_WINDOW_MENU_BACK, x, y, width, height, 0.f, 0.f, width / 256.f, height / 512.f);

    y = m_Pos.y;
    RenderImage(IMAGE_WINDOW_MENU_FRAME_UP, m_Pos.x, y, 112.f, 45.f);
    y += 45.f;

    float Middle_TempCnt = (int)((height - 90) / 15) + 1;

    for (int i = 0; i < Middle_TempCnt; ++i)
    {
        RenderImage(IMAGE_WINDOW_MENU_FRAME_MIDDLE, m_Pos.x, y, 112.f, 15.f);
        y += 15.f;
    }

    y = 480 - 50 - 45;

    RenderImage(IMAGE_WINDOW_MENU_FRAME_DOWN, m_Pos.x, y, 112.f, 45.f);

    y = m_Pos.y + 33.f;

    for (int i = 0; i < MENU_MAX_INDEX - 1; ++i)
    {
        RenderImage(IMAGE_WINDOW_MENU_LINE, m_Pos.x + 15.f, y, 82.f, 2.f);
        y += 20.f;
    }
}

void SEASON3B::CNewUIWindowMenu::RenderTexts()
{
    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->SetBgColor(0);
    g_pRenderText->SetTextColor(255, 255, 255, 255);

    int iTextNumber[] = { 1741, 1742, 364, 1743,3055,3103 };
    float y = m_Pos.y + 20.f;
    for (int i = 0; i < MENU_MAX_INDEX; ++i)
    {
        if (m_iSelectedIndex == i)
        {
            g_pRenderText->SetTextColor(255, 255, 0, 255);
        }
        else
        {
            g_pRenderText->SetTextColor(255, 255, 255, 255);
        }
        g_pRenderText->RenderText(m_Pos.x, y, GlobalText[iTextNumber[i]], 112, 0, RT3_SORT_CENTER);
        y += 20.f;
    }
}

void SEASON3B::CNewUIWindowMenu::RenderArrow()
{
    if (m_iSelectedIndex < 0)
    {
        return;
    }

    float x, y;
    x = m_Pos.x + 16.f;
    y = m_Pos.y + 20.f + (m_iSelectedIndex * 20);

    RenderImage(IMAGE_WINDOW_MENU_ARROWL, x, y, 6.f, 9.f);
    x = m_Pos.x + 90.f;
    RenderImage(IMAGE_WINDOW_MENU_ARROWR, x, y, 6.f, 9.f);
}

float SEASON3B::CNewUIWindowMenu::GetLayerDepth()
{
    return 10.4f;
}

float SEASON3B::CNewUIWindowMenu::GetKeyEventOrder()
{
    return 10.f;
}

void SEASON3B::CNewUIWindowMenu::OpenningProcess()
{
    m_iSelectedIndex = -1;
}

void SEASON3B::CNewUIWindowMenu::ClosingProcess()
{
}

void SEASON3B::CNewUIWindowMenu::LoadImages()
{
    LoadBitmap(L"Interface\\newui_msgbox_back.jpg", IMAGE_WINDOW_MENU_BACK, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_commamd01.tga", IMAGE_WINDOW_MENU_FRAME_UP, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_commamd02.tga", IMAGE_WINDOW_MENU_FRAME_MIDDLE, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_commamd03.tga", IMAGE_WINDOW_MENU_FRAME_DOWN, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_commamd_Line.jpg", IMAGE_WINDOW_MENU_LINE, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_arrow(L).tga", IMAGE_WINDOW_MENU_ARROWL, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_arrow(R).tga", IMAGE_WINDOW_MENU_ARROWR, GL_LINEAR);
}

void SEASON3B::CNewUIWindowMenu::UnloadImages()
{
    DeleteBitmap(IMAGE_WINDOW_MENU_BACK);
    DeleteBitmap(IMAGE_WINDOW_MENU_FRAME_UP);
    DeleteBitmap(IMAGE_WINDOW_MENU_FRAME_MIDDLE);
    DeleteBitmap(IMAGE_WINDOW_MENU_FRAME_DOWN);
    DeleteBitmap(IMAGE_WINDOW_MENU_LINE);
    DeleteBitmap(IMAGE_WINDOW_MENU_ARROWL);
    DeleteBitmap(IMAGE_WINDOW_MENU_ARROWR);
}