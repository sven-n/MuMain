// NewUIDuelWatchWindow.cpp: implementation of the CNewUIDuelWatchWindow class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUIDuelWatchWindow.h"
#include "NewUISystem.h"
#include "NewUICustomMessageBox.h"
#include "ZzzBMD.h"
#include "ZzzEffect.h"
#include "ZzzObject.h"
#include "ZzzInventory.h"
#include "ZzzInterface.h"
#include "ZzzInfomation.h"
#include "ZzzCharacter.h"

#include "DSPlaySound.h"
#include "DuelMgr.h"

using namespace SEASON3B;

CNewUIDuelWatchWindow::CNewUIDuelWatchWindow()
{
    m_pNewUIMng = NULL;
    m_Pos.x = m_Pos.y = 0;
}

CNewUIDuelWatchWindow::~CNewUIDuelWatchWindow()
{
    Release();
}

bool CNewUIDuelWatchWindow::Create(CNewUIManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_DUELWATCH, this);

    SetPos(x, y);

    LoadImages();

    for (int i = 0; i < 4; ++i)
    {
        m_bChannelEnable[i] = FALSE;
        InitButton(m_BtnChannel + i, m_Pos.x + INVENTORY_WIDTH / 2 - 27, m_Pos.y + 100 + i * 90, GlobalText[2701]);
    }

    Show(false);

    return true;
}

void CNewUIDuelWatchWindow::InitButton(CNewUIButton* pNewUIButton, int iPos_x, int iPos_y, const wchar_t* pCaption)
{
    pNewUIButton->ChangeText(pCaption);
    pNewUIButton->ChangeTextBackColor(RGBA(255, 255, 255, 0));
    pNewUIButton->ChangeButtonImgState(true, IMAGE_DUELWATCHWINDOW_BUTTON, true);
    pNewUIButton->ChangeButtonInfo(iPos_x, iPos_y, 53, 23);
    pNewUIButton->ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
    pNewUIButton->ChangeImgColor(BUTTON_STATE_DOWN, RGBA(255, 255, 255, 255));
}

void CNewUIDuelWatchWindow::Release()
{
    UnloadImages();

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void CNewUIDuelWatchWindow::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

bool CNewUIDuelWatchWindow::UpdateMouseEvent()
{
    if (true == BtnProcess())
        return false;

    if (CheckMouseIn(m_Pos.x, m_Pos.y, INVENTORY_WIDTH, INVENTORY_HEIGHT))
        return false;

    return true;
}

bool CNewUIDuelWatchWindow::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_DUELWATCH) == true)
    {
        if (SEASON3B::IsPress(VK_ESCAPE) == true)
        {
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_DUELWATCH);
            PlayBuffer(SOUND_CLICK01);
            return false;
        }
    }
    return true;
}

bool CNewUIDuelWatchWindow::Update()
{
    if (IsVisible())
    {
        for (int i = 0; i < 4; ++i)
        {
            if (g_DuelMgr.IsDuelChannelEnabled(i))
                m_bChannelEnable[i] = TRUE;
            else
                m_bChannelEnable[i] = FALSE;

            if (m_bChannelEnable[i] == TRUE && g_DuelMgr.IsDuelChannelJoinable(i))
            {
                m_BtnChannel[i].UnLock();
                m_BtnChannel[i].ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
                m_BtnChannel[i].ChangeTextColor(RGBA(255, 255, 255, 255));
            }
            else
            {
                m_BtnChannel[i].Lock();
                m_BtnChannel[i].ChangeImgColor(BUTTON_STATE_UP, RGBA(100, 100, 100, 255));
                m_BtnChannel[i].ChangeTextColor(RGBA(100, 100, 100, 255));
            }
        }
    }
    return true;
}

bool CNewUIDuelWatchWindow::Render()
{
    EnableAlphaTest();
    glColor4f(1.f, 1.f, 1.f, 1.f);

    RenderFrame();

    POINT ptOrigin = { m_Pos.x, m_Pos.y + 50 };
    wchar_t szText[256];

    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, GlobalText[2699], 190, 0, RT3_SORT_CENTER);

    RenderImage(IMAGE_DUELWATCHWINDOW_LINE, m_Pos.x + 1, m_Pos.y + 130, 188.f, 21.f);
    RenderImage(IMAGE_DUELWATCHWINDOW_LINE, m_Pos.x + 1, m_Pos.y + 130 + 90, 188.f, 21.f);
    RenderImage(IMAGE_DUELWATCHWINDOW_LINE, m_Pos.x + 1, m_Pos.y + 130 + 90 + 90, 188.f, 21.f);

    int i = 0;
    g_pRenderText->SetFont(g_hFontBold);
    for (i = 0; i < 4; ++i)
    {
        g_pRenderText->SetTextColor(255, 255, 128, 255);
        swprintf(szText, GlobalText[2700], i + 1);
        g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y + 20, szText, 190, 0, RT3_SORT_CENTER);

        ptOrigin.y += 90;
    }

    ptOrigin.y = m_Pos.y + 50;
    g_pRenderText->SetFont(g_hFont);
    for (i = 0; i < 4; ++i)
    {
        if (m_bChannelEnable[i] == TRUE)
        {
            g_pRenderText->SetTextColor(255, 50, 50, 255);
            g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y + 35, L"VS", 190, 0, RT3_SORT_CENTER);
            g_pRenderText->SetTextColor(255, 255, 255, 255);
            g_pRenderText->RenderText(ptOrigin.x + 20, ptOrigin.y + 35, g_DuelMgr.GetDuelChannelUserID1(i), 70, 0, RT3_SORT_CENTER);
            g_pRenderText->RenderText(ptOrigin.x + 100, ptOrigin.y + 35, g_DuelMgr.GetDuelChannelUserID2(i), 70, 0, RT3_SORT_CENTER);
        }
        else
        {
            g_pRenderText->SetTextColor(255, 255, 255, 255);
            g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y + 35, GlobalText[2705], 190, 0, RT3_SORT_CENTER);
        }

        ptOrigin.y += 90;
    }

    for (i = 0; i < 4; ++i)
    {
        m_BtnChannel[i].Render();
    }

    DisableAlphaBlend();

    return true;
}

void CNewUIDuelWatchWindow::OpeningProcess()
{
}

void CNewUIDuelWatchWindow::ClosingProcess()
{
    SocketClient->ToGameServer()->SendCloseNpcRequest();
}

float CNewUIDuelWatchWindow::GetLayerDepth()
{
    return 5.0f;
}

void CNewUIDuelWatchWindow::LoadImages()
{
    LoadBitmap(L"Interface\\newui_msgbox_back.jpg", IMAGE_DUELWATCHWINDOW_BACK, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back01.tga", IMAGE_DUELWATCHWINDOW_TOP, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back02-L.tga", IMAGE_DUELWATCHWINDOW_LEFT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back02-R.tga", IMAGE_DUELWATCHWINDOW_RIGHT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back03.tga", IMAGE_DUELWATCHWINDOW_BOTTOM, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_btn_empty_very_small.tga", IMAGE_DUELWATCHWINDOW_BUTTON, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_myquest_Line.tga", IMAGE_DUELWATCHWINDOW_LINE, GL_LINEAR);
}

void CNewUIDuelWatchWindow::UnloadImages()
{
    DeleteBitmap(IMAGE_DUELWATCHWINDOW_BOTTOM);
    DeleteBitmap(IMAGE_DUELWATCHWINDOW_RIGHT);
    DeleteBitmap(IMAGE_DUELWATCHWINDOW_LEFT);
    DeleteBitmap(IMAGE_DUELWATCHWINDOW_TOP);
    DeleteBitmap(IMAGE_DUELWATCHWINDOW_BACK);
    DeleteBitmap(IMAGE_DUELWATCHWINDOW_BUTTON);
    DeleteBitmap(IMAGE_DUELWATCHWINDOW_LINE);
}

void CNewUIDuelWatchWindow::RenderFrame()
{
    RenderImage(IMAGE_DUELWATCHWINDOW_BACK, m_Pos.x, m_Pos.y, 190.f, 429.f);
    RenderImage(IMAGE_DUELWATCHWINDOW_TOP, m_Pos.x, m_Pos.y, 190.f, 64.f);
    RenderImage(IMAGE_DUELWATCHWINDOW_LEFT, m_Pos.x, m_Pos.y + 64, 21.f, 320.f);
    RenderImage(IMAGE_DUELWATCHWINDOW_RIGHT, m_Pos.x + INVENTORY_WIDTH - 21, m_Pos.y + 64, 21.f, 320.f);
    RenderImage(IMAGE_DUELWATCHWINDOW_BOTTOM, m_Pos.x, m_Pos.y + INVENTORY_HEIGHT - 45, 190.f, 45.f);

    wchar_t szText[256] = { 0, };
    float fPos_x = m_Pos.x + 15.0f, fPos_y = m_Pos.y;
    float fLine_y = 13.0f;

    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetTextColor(220, 220, 220, 255);
    g_pRenderText->SetBgColor(0, 0, 0, 0);

    swprintf(szText, L"%s", GlobalText[2698]);
    g_pRenderText->RenderText(fPos_x, fPos_y + fLine_y, szText, 160.0f, 0, RT3_SORT_CENTER);
}

bool CNewUIDuelWatchWindow::BtnProcess()
{
    POINT ptExitBtn1 = { m_Pos.x + 169, m_Pos.y + 7 };

    if (SEASON3B::IsPress(VK_LBUTTON) && CheckMouseIn(ptExitBtn1.x, ptExitBtn1.y, 13, 12))
    {
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_DUELWATCH);
    }

    for (BYTE i = 0; i < 4; ++i)
    {
        if (m_BtnChannel[i].UpdateMouseEvent() == true)
        {
            SocketClient->ToGameServer()->SendDuelChannelJoinRequest(i);
            return true;
        }
    }

    return false;
}