// NewUIPartyInfo.cpp: implementation of the CNewUIPartyInfo class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "NewUIPartyInfoWindow.h"
#include "NewUISystem.h"
#include "CSChaosCastle.h"
#include "DSPlaySound.h"
#include "w_CursedTemple.h"
#include "MapManager.h"

using namespace SEASON3B;

CNewUIPartyInfoWindow::CNewUIPartyInfoWindow()
{
    m_pNewUIMng = NULL;
    m_Pos.x = m_Pos.y = 0;
    m_bParty = false;
}

CNewUIPartyInfoWindow::~CNewUIPartyInfoWindow()
{
    Release();
}

bool CNewUIPartyInfoWindow::Create(CNewUIManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_PARTY, this);

    SetPos(x, y);

    LoadImages();

    InitButtons();

    Show(false);

    return true;
}

void CNewUIPartyInfoWindow::Release()
{
    UnloadImages();

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void CNewUIPartyInfoWindow::InitButtons()
{
    m_BtnExit.ChangeButtonImgState(true, IMAGE_PARTY_BASE_WINDOW_BTN_EXIT);
    m_BtnExit.ChangeButtonInfo(m_Pos.x + 13, m_Pos.y + 392, 36, 29);
    m_BtnExit.ChangeToolTipText(GlobalText[221], true);

    for (int i = 0; i < MAX_PARTYS; i++)
    {
        int iVal = i * 71;
        m_BtnPartyExit[i].ChangeButtonImgState(true, IMAGE_PARTY_EXIT);
        m_BtnPartyExit[i].ChangeButtonInfo(m_Pos.x + 159, m_Pos.y + 63 + iVal, 13, 13);
    }
}

void CNewUIPartyInfoWindow::OpenningProcess()
{
    SocketClient->ToGameServer()->SendPartyListRequest();
}

void CNewUIPartyInfoWindow::ClosingProcess()
{
}

bool CNewUIPartyInfoWindow::BtnProcess()
{
    POINT ptExitBtn1 = { m_Pos.x + 169, m_Pos.y + 7 };

    if (SEASON3B::IsPress(VK_LBUTTON) && CheckMouseIn(ptExitBtn1.x, ptExitBtn1.y, 13, 12))
    {
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_PARTY);
        return true;
    }

    if (m_BtnExit.UpdateMouseEvent() == true)
    {
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_PARTY);
        return true;
    }

    if (IsVisible())
    {
        for (int i = 0; i < PartyNumber; i++)
        {
            if (!wcscmp(Party[0].Name, Hero->ID) || !wcscmp(Party[i].Name, Hero->ID))
            {
                if (m_BtnPartyExit[i].UpdateMouseEvent())
                {
                    LeaveParty(i);
                    return true;
                }
            }
        }
    }

    return false;
}

bool CNewUIPartyInfoWindow::UpdateMouseEvent()
{
    if (true == BtnProcess())
        return false;

    if (CheckMouseIn(m_Pos.x, m_Pos.y, PARTY_INFO_WINDOW_WIDTH, PARTY_INFO_WINDOW_HEIGHT))
        return false;

    return true;
}

bool CNewUIPartyInfoWindow::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_PARTY) == true)
    {
        if (SEASON3B::IsPress(VK_ESCAPE) == true)
        {
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_PARTY);
            PlayBuffer(SOUND_CLICK01);

            return false;
        }
    }

    return true;
}

bool CNewUIPartyInfoWindow::Update()
{
    if (IsVisible())
    {
        if (PartyNumber > 0)
            SetParty(true);
        else
            SetParty(false);
    }

    return true;
}

bool CNewUIPartyInfoWindow::Render()
{
    EnableAlphaTest();
    glColor4f(1.f, 1.f, 1.f, 1.f);

    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->SetTextColor(0xFFFFFFFF);

    DWORD dwPreBGColor = g_pRenderText->GetBgColor();
    g_pRenderText->SetBgColor(RGBA(0, 0, 0, 0));

    RenderImage(IMAGE_PARTY_BASE_WINDOW_BACK, m_Pos.x, m_Pos.y, float(PARTY_INFO_WINDOW_WIDTH), float(PARTY_INFO_WINDOW_HEIGHT));
    RenderImage(IMAGE_PARTY_BASE_WINDOW_TOP, m_Pos.x, m_Pos.y, float(PARTY_INFO_WINDOW_WIDTH), 64.f);
    RenderImage(IMAGE_PARTY_BASE_WINDOW_LEFT, m_Pos.x, m_Pos.y + 64.f, 21.f, float(PARTY_INFO_WINDOW_HEIGHT) - 64.f - 45.f);
    RenderImage(IMAGE_PARTY_BASE_WINDOW_RIGHT, m_Pos.x + float(PARTY_INFO_WINDOW_WIDTH) - 21.f, m_Pos.y + 64.f, 21.f, float(PARTY_INFO_WINDOW_HEIGHT) - 64.f - 45.f);
    RenderImage(IMAGE_PARTY_BASE_WINDOW_BOTTOM, m_Pos.x, m_Pos.y + float(PARTY_INFO_WINDOW_HEIGHT) - 45.f, float(PARTY_INFO_WINDOW_WIDTH), 45.f);

    m_BtnExit.Render();
    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->RenderText(m_Pos.x + 60, m_Pos.y + 12, GlobalText[190], 72, 0, RT3_SORT_CENTER);

    g_pRenderText->SetFont(g_hFont);

    if (m_bParty)
    {
        for (int i = 0; i < PartyNumber; i++)
        {
            PARTY_t* pMember = &Party[i];

            bool bExitBtnRender = false;

            if (!wcscmp(Party[0].Name, Hero->ID) || !wcscmp(Party[i].Name, Hero->ID))
            {
                bExitBtnRender = true;
            }

            RenderMemberStatue(i, pMember, bExitBtnRender);
        }
    }
    else
    {
        int iStartHeight = 60;
        g_pRenderText->RenderText(m_Pos.x + 20, m_Pos.y + iStartHeight, GlobalText[191], 0, 0, RT3_SORT_CENTER);
        g_pRenderText->RenderText(m_Pos.x + 20, m_Pos.y + iStartHeight + 15, GlobalText[192], 0, 0, RT3_SORT_CENTER);
        g_pRenderText->RenderText(m_Pos.x + 20, m_Pos.y + iStartHeight + 30, GlobalText[193], 0, 0, RT3_SORT_CENTER);
        g_pRenderText->RenderText(m_Pos.x + 20, m_Pos.y + iStartHeight + 50, GlobalText[194], 0, 0, RT3_SORT_CENTER);
        g_pRenderText->RenderText(m_Pos.x + 20, m_Pos.y + iStartHeight + 65, GlobalText[195], 0, 0, RT3_SORT_CENTER);
        g_pRenderText->RenderText(m_Pos.x + 20, m_Pos.y + iStartHeight + 80, GlobalText[196], 0, 0, RT3_SORT_CENTER);
        g_pRenderText->RenderText(m_Pos.x + 20, m_Pos.y + iStartHeight + 95, GlobalText[197], 0, 0, RT3_SORT_CENTER);
    }

    g_pRenderText->SetBgColor(dwPreBGColor);

    DisableAlphaBlend();

    return true;
}

void CNewUIPartyInfoWindow::RenderGroupBox(int iPosX, int iPosY, int iWidth, int iHeight, int iTitleWidth/* =60 */, int iTitleHeight/* =20  */)
{
    EnableAlphaTest();

    glColor4f(0.f, 0.f, 0.f, 0.9f);
    RenderColor(float(iPosX + 3), float(iPosY + 2), float(iTitleWidth - 8), float(iTitleHeight));
    glColor4f(0.f, 0.f, 0.f, 0.6f);
    RenderColor(float(iPosX + 3), float(iPosY + 2 + iTitleHeight), float(iWidth - 7), float(iHeight - iTitleHeight - 7));

    EndRenderColor();

    RenderImage(IMAGE_PARTY_TABLE_TOP_LEFT, iPosX, iPosY, 14, 14);
    RenderImage(IMAGE_PARTY_TABLE_TOP_RIGHT, iPosX + iTitleWidth - 14, iPosY, 14, 14);
    RenderImage(IMAGE_PARTY_TABLE_TOP_RIGHT, iPosX + iWidth - 14, iPosY + iTitleHeight, 14, 14);
    RenderImage(IMAGE_PARTY_TABLE_BOTTOM_LEFT, iPosX, iPosY + iHeight - 14, 14, 14);
    RenderImage(IMAGE_PARTY_TABLE_BOTTOM_RIGHT, iPosX + iWidth - 14, iPosY + iHeight - 14, 14, 14);

    RenderImage(IMAGE_PARTY_TABLE_TOP_PIXEL, iPosX + 6, iPosY, iTitleWidth - 12, 14);
    RenderImage(IMAGE_PARTY_TABLE_RIGHT_PIXEL, iPosX + iTitleWidth - 14, iPosY + 6, 14, iTitleHeight - 6);
    RenderImage(IMAGE_PARTY_TABLE_TOP_PIXEL, iPosX + iTitleWidth - 5, iPosY + iTitleHeight, iWidth - iTitleWidth - 6, 14);
    RenderImage(IMAGE_PARTY_TABLE_RIGHT_PIXEL, iPosX + iWidth - 14, iPosY + iTitleHeight + 6, 14, iHeight - iTitleHeight - 14);
    RenderImage(IMAGE_PARTY_TABLE_BOTTOM_PIXEL, iPosX + 6, iPosY + iHeight - 14, iWidth - 12, 14);
    RenderImage(IMAGE_PARTY_TABLE_LEFT_PIXEL, iPosX, iPosY + 6, 14, iHeight - 14);
}

void CNewUIPartyInfoWindow::RenderMemberStatue(int iIndex, PARTY_t* pMember, bool bExitBtnRender /*= false*/)
{
    wchar_t szText[256] = { 0, };

    int iVal = iIndex * 71;
    int iPosX = m_Pos.x + 10;
    int iPosY = m_Pos.y + 40 + iVal;

    RenderGroupBox(iPosX, iPosY, 170, 70, 70, 20);

    g_pRenderText->SetFont(g_hFontBold);
    if (iIndex == 0)
    {
        g_pRenderText->SetFont(g_hFontBold);
        g_pRenderText->SetTextColor(0, 255, 0, 255);
        g_pRenderText->RenderText(iPosX, iPosY + 8, pMember->Name, 70, 0, RT3_SORT_CENTER);
        RenderImage(IMAGE_PARTY_FLAG, m_Pos.x + 146, iPosY + 23, 10, 12);
    }
    else
    {
        g_pRenderText->SetFont(g_hFontBold);
        g_pRenderText->SetTextColor(255, 255, 255, 255);
        g_pRenderText->RenderText(iPosX, iPosY + 8, pMember->Name, 70, 0, RT3_SORT_CENTER);
    }

    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->SetTextColor(RGBA(194, 194, 194, 255));

    g_pRenderText->RenderText(iPosX + 10, iPosY + 26, gMapManager.GetMapName(pMember->Map), 70, 0, RT3_SORT_LEFT);

    swprintf(szText, L"(%d,%d)", pMember->x, pMember->y);
    g_pRenderText->RenderText(iPosX + 85, iPosY + 26, szText, 60, 0, RT3_SORT_LEFT);

    int iHP = (pMember->currHP * 147) / pMember->maxHP;
    RenderImage(IMAGE_PARTY_HPBAR_BACK, iPosX + 8, iPosY + 39, 151, 8);
    RenderImage(IMAGE_PARTY_HPBAR, iPosX + 10, iPosY + 41, iHP, 4);

    swprintf(szText, L"%d %s %d", pMember->currHP, GlobalText[2374], pMember->maxHP);
    g_pRenderText->RenderText(iPosX + 88, iPosY + 51, szText, 70, 0, RT3_SORT_RIGHT);

    if (bExitBtnRender)
    {
        m_BtnPartyExit[iIndex].Render();
    }
}

bool CNewUIPartyInfoWindow::LeaveParty(const int iIndex)
{
    if (!gMapManager.IsCursedTemple())
    {
        PlayBuffer(SOUND_CLICK01);
        SocketClient->ToGameServer()->SendPartyPlayerKickRequest(Party[iIndex].Number);
    }

    SetParty(false);

    return true;
}

void CNewUIPartyInfoWindow::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;

    m_BtnExit.ChangeButtonInfo(m_Pos.x + 13, m_Pos.y + 392, 36, 29);

    for (int i = 0; i < MAX_PARTYS; i++)
    {
        int iVal = i * 71;
        m_BtnPartyExit[i].ChangeButtonInfo(m_Pos.x + 159, m_Pos.y + 63 + iVal, 13, 13);
    }
}

void CNewUIPartyInfoWindow::SetParty(bool bParty)
{
    m_bParty = bParty;
}

float CNewUIPartyInfoWindow::GetLayerDepth()
{
    return 2.4f;
}

void CNewUIPartyInfoWindow::LoadImages()
{
    LoadBitmap(L"Interface\\newui_msgbox_back.jpg", IMAGE_PARTY_BASE_WINDOW_BACK, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back01.tga", IMAGE_PARTY_BASE_WINDOW_TOP, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back02-L.tga", IMAGE_PARTY_BASE_WINDOW_LEFT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back02-R.tga", IMAGE_PARTY_BASE_WINDOW_RIGHT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back03.tga", IMAGE_PARTY_BASE_WINDOW_BOTTOM, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_exit_00.tga", IMAGE_PARTY_BASE_WINDOW_BTN_EXIT, GL_LINEAR);		// Exit Button

    LoadBitmap(L"Interface\\newui_item_table01(L).tga", IMAGE_PARTY_TABLE_TOP_LEFT);
    LoadBitmap(L"Interface\\newui_item_table01(R).tga", IMAGE_PARTY_TABLE_TOP_RIGHT);
    LoadBitmap(L"Interface\\newui_item_table02(L).tga", IMAGE_PARTY_TABLE_BOTTOM_LEFT);
    LoadBitmap(L"Interface\\newui_item_table02(R).tga", IMAGE_PARTY_TABLE_BOTTOM_RIGHT);
    LoadBitmap(L"Interface\\newui_item_table03(Up).tga", IMAGE_PARTY_TABLE_TOP_PIXEL);
    LoadBitmap(L"Interface\\newui_item_table03(Dw).tga", IMAGE_PARTY_TABLE_BOTTOM_PIXEL);
    LoadBitmap(L"Interface\\newui_item_table03(L).tga", IMAGE_PARTY_TABLE_LEFT_PIXEL);
    LoadBitmap(L"Interface\\newui_item_table03(R).tga", IMAGE_PARTY_TABLE_RIGHT_PIXEL);

    LoadBitmap(L"Interface\\newui_party_lifebar01.jpg", IMAGE_PARTY_HPBAR_BACK, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_party_lifebar02.jpg", IMAGE_PARTY_HPBAR, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_party_flag.tga", IMAGE_PARTY_FLAG, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_party_x.tga", IMAGE_PARTY_EXIT, GL_LINEAR);
}

void CNewUIPartyInfoWindow::UnloadImages()
{
    DeleteBitmap(IMAGE_PARTY_BASE_WINDOW_BACK);
    DeleteBitmap(IMAGE_PARTY_BASE_WINDOW_TOP);
    DeleteBitmap(IMAGE_PARTY_BASE_WINDOW_LEFT);
    DeleteBitmap(IMAGE_PARTY_BASE_WINDOW_RIGHT);
    DeleteBitmap(IMAGE_PARTY_BASE_WINDOW_BOTTOM);
    DeleteBitmap(IMAGE_PARTY_BASE_WINDOW_BTN_EXIT);

    DeleteBitmap(IMAGE_PARTY_TABLE_RIGHT_PIXEL);
    DeleteBitmap(IMAGE_PARTY_TABLE_LEFT_PIXEL);
    DeleteBitmap(IMAGE_PARTY_TABLE_BOTTOM_PIXEL);
    DeleteBitmap(IMAGE_PARTY_TABLE_TOP_PIXEL);
    DeleteBitmap(IMAGE_PARTY_TABLE_BOTTOM_RIGHT);
    DeleteBitmap(IMAGE_PARTY_TABLE_BOTTOM_LEFT);
    DeleteBitmap(IMAGE_PARTY_TABLE_TOP_RIGHT);
    DeleteBitmap(IMAGE_PARTY_TABLE_TOP_LEFT);

    DeleteBitmap(IMAGE_PARTY_HPBAR_BACK);
    DeleteBitmap(IMAGE_PARTY_HPBAR);
    DeleteBitmap(IMAGE_PARTY_FLAG);
    DeleteBitmap(IMAGE_PARTY_EXIT);
}