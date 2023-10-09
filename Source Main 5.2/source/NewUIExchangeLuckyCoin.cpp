// NewUIExchangeLuckyCoin.cpp: implementation of the CNewUIExchangeLuckyCoin class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUIExchangeLuckyCoin.h"

#include "DSPlaySound.h"
#include "NewUISystem.h"
#include "NewUICommon.h"
#include "NewUICommonMessageBox.h"

using namespace SEASON3B;

CNewUIExchangeLuckyCoin::CNewUIExchangeLuckyCoin()
{
    m_pNewUIMng = NULL;
    memset(&m_Pos, 0, sizeof(POINT));
    memset(&m_TextPos, 0, sizeof(POINT));
    memset(&m_FirstBtnPos, 0, sizeof(POINT));
}

CNewUIExchangeLuckyCoin::~CNewUIExchangeLuckyCoin()
{
    Release();
}

bool CNewUIExchangeLuckyCoin::Create(CNewUIManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_EXCHANGE_LUCKYCOIN, this);

    SetPos(x, y);

    LoadImages();

    // Exit Button
    m_BtnExit.ChangeButtonImgState(true, IMAGE_EXCHANGE_LUCKYCOIN_WINDOW_BTN_EXIT, true);
    m_BtnExit.ChangeButtonInfo(m_Pos.x + ((EXCHANGE_LUCKYCOIN_WINDOW_WIDTH / 2) - (MSGBOX_BTN_EMPTY_SMALL_WIDTH / 2)), m_Pos.y + 360,
        MSGBOX_BTN_EMPTY_SMALL_WIDTH, MSGBOX_BTN_EMPTY_HEIGHT);
    m_BtnExit.ChangeText(GlobalText[1002]);

    // Exchange Button
    m_BtnExchange[0].ChangeButtonImgState(true, IMAGE_EXCHANGE_LUCKYCOIN_EXCHANGE_BTN, true);
    m_BtnExchange[0].SetFont(g_hFontBold);
    m_BtnExchange[0].ChangeText(GlobalText[1896]);

    m_BtnExchange[1].ChangeButtonImgState(true, IMAGE_EXCHANGE_LUCKYCOIN_EXCHANGE_BTN, true);
    m_BtnExchange[1].SetFont(g_hFontBold);
    m_BtnExchange[1].ChangeText(GlobalText[1897]);

    m_BtnExchange[2].ChangeButtonImgState(true, IMAGE_EXCHANGE_LUCKYCOIN_EXCHANGE_BTN, true);
    m_BtnExchange[2].SetFont(g_hFontBold);
    m_BtnExchange[2].ChangeText(GlobalText[1898]);

    Show(false);

    return true;
}

void CNewUIExchangeLuckyCoin::Release()
{
    UnloadImages();

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void CNewUIExchangeLuckyCoin::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
    m_TextPos.x = m_Pos.x;
    m_TextPos.y = m_Pos.y + 80;

    SetBtnPos(m_Pos.x + ((EXCHANGE_LUCKYCOIN_WINDOW_WIDTH / 2) - (MSGBOX_BTN_EMPTY_WIDTH / 2)), m_Pos.y + 220);

    m_BtnExit.ChangeButtonInfo(m_Pos.x + 13, m_Pos.y + 392, 36, 29);

    for (int i = 0; i < MAX_EXCHANGE_BTN; i++)
    {
        int iVal = EXCHANGE_BTN_VAL * i;
        m_BtnExchange[i].ChangeButtonInfo(m_FirstBtnPos.x, m_FirstBtnPos.y + iVal, MSGBOX_BTN_EMPTY_WIDTH, MSGBOX_BTN_EMPTY_HEIGHT);
    }
}

void CNewUIExchangeLuckyCoin::SetBtnPos(int x, int y)
{
    m_FirstBtnPos.x = x;
    m_FirstBtnPos.y = y;
}

bool CNewUIExchangeLuckyCoin::UpdateMouseEvent()
{
    if (true == BtnProcess())
        return false;

    if (SEASON3B::CheckMouseIn(m_Pos.x, m_Pos.y, EXCHANGE_LUCKYCOIN_WINDOW_WIDTH, EXCHANGE_LUCKYCOIN_WINDOW_HEIGHT))
        return false;

    return true;
}

bool CNewUIExchangeLuckyCoin::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_EXCHANGE_LUCKYCOIN) == true)
    {
        if (SEASON3B::IsPress(VK_ESCAPE) == true)
        {
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_EXCHANGE_LUCKYCOIN);
            return false;
        }
    }

    return true;
}

bool CNewUIExchangeLuckyCoin::Update()
{
    if (!IsVisible())
        return true;

    return true;
}

bool CNewUIExchangeLuckyCoin::Render()
{
    EnableAlphaTest();
    glColor4f(1.f, 1.f, 1.f, 1.f);

    RenderFrame();
    RenderTexts();
    RenderBtn();

    DisableAlphaBlend();

    return true;
}

void CNewUIExchangeLuckyCoin::RenderFrame()
{
    RenderImage(IMAGE_EXCHANGE_LUCKYCOIN_WINDOW_BACK, m_Pos.x, m_Pos.y, float(EXCHANGE_LUCKYCOIN_WINDOW_WIDTH), float(EXCHANGE_LUCKYCOIN_WINDOW_HEIGHT));
    RenderImage(IMAGE_EXCHANGE_LUCKYCOIN_WINDOW_TOP, m_Pos.x, m_Pos.y, float(EXCHANGE_LUCKYCOIN_WINDOW_WIDTH), 64.f);
    RenderImage(IMAGE_EXCHANGE_LUCKYCOIN_WINDOW_LEFT, m_Pos.x, m_Pos.y + 64.f, 21.f, float(EXCHANGE_LUCKYCOIN_WINDOW_HEIGHT) - 64.f - 45.f);
    RenderImage(IMAGE_EXCHANGE_LUCKYCOIN_WINDOW_RIGHT, m_Pos.x + float(EXCHANGE_LUCKYCOIN_WINDOW_WIDTH) - 21.f, m_Pos.y + 64.f, 21.f, float(EXCHANGE_LUCKYCOIN_WINDOW_HEIGHT) - 64.f - 45.f);
    RenderImage(IMAGE_EXCHANGE_LUCKYCOIN_WINDOW_BOTTOM, m_Pos.x, m_Pos.y + float(EXCHANGE_LUCKYCOIN_WINDOW_HEIGHT) - 45.f, float(EXCHANGE_LUCKYCOIN_WINDOW_WIDTH), 45.f);
}

void CNewUIExchangeLuckyCoin::RenderTexts()
{
    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->SetBgColor(0, 0, 0, 0);

    g_pRenderText->RenderText(m_Pos.x, m_Pos.y + 25, GlobalText[1892], 190, 0, RT3_SORT_CENTER);

    g_pRenderText->RenderText(m_TextPos.x, m_Pos.y + 200, GlobalText[1940], EXCHANGE_LUCKYCOIN_WINDOW_WIDTH, 0, RT3_SORT_CENTER);

    g_pRenderText->SetTextColor(255, 255, 0, 255);
    g_pRenderText->RenderText(m_TextPos.x, m_TextPos.y, GlobalText[1895], EXCHANGE_LUCKYCOIN_WINDOW_WIDTH, 0, RT3_SORT_CENTER);

    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    int iTextPosy = m_TextPos.y + (EXCHANGE_TEXT_VAL * 2);
    g_pRenderText->RenderText(m_TextPos.x, iTextPosy, GlobalText[1938], EXCHANGE_LUCKYCOIN_WINDOW_WIDTH, 0, RT3_SORT_CENTER);
    iTextPosy += EXCHANGE_TEXT_VAL;
    g_pRenderText->RenderText(m_TextPos.x, iTextPosy, GlobalText[1939], EXCHANGE_LUCKYCOIN_WINDOW_WIDTH, 0, RT3_SORT_CENTER);
}

void CNewUIExchangeLuckyCoin::RenderBtn()
{
    for (int i = 0; i < MAX_EXCHANGE_BTN; i++)
    {
        m_BtnExchange[i].Render();
    }
    m_BtnExit.Render();
}

bool CNewUIExchangeLuckyCoin::BtnProcess()
{
    POINT ptExitBtn1 = { m_Pos.x + 169, m_Pos.y + 7 };

    if (SEASON3B::IsPress(VK_LBUTTON) && SEASON3B::CheckMouseIn(ptExitBtn1.x, ptExitBtn1.y, 13, 12))
    {
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_EXCHANGE_LUCKYCOIN);
        return true;
    }

    if (m_BtnExit.UpdateMouseEvent() == true)
    {
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_EXCHANGE_LUCKYCOIN);
        return true;
    }

    if (m_BtnExchange[0].UpdateMouseEvent() == true)
    {
        LockExchangeBtn();
        SocketClient->ToGameServer()->SendLuckyCoinExchangeRequest(10);
    }

    if (m_BtnExchange[1].UpdateMouseEvent() == true)
    {
        LockExchangeBtn();
        SocketClient->ToGameServer()->SendLuckyCoinExchangeRequest(20);
    }

    if (m_BtnExchange[2].UpdateMouseEvent() == true)
    {
        LockExchangeBtn();
        SocketClient->ToGameServer()->SendLuckyCoinExchangeRequest(30);
    }

    return false;
}

float CNewUIExchangeLuckyCoin::GetLayerDepth()
{
    return 4.2f;
}

void CNewUIExchangeLuckyCoin::OpenningProcess()
{
    g_pNewUISystem->Show(SEASON3B::INTERFACE_INVENTORY);
    UnLockExchangeBtn();
    g_pMyInventory->GetInventoryCtrl()->LockInventory();
    PlayBuffer(SOUND_CLICK01);
}

void CNewUIExchangeLuckyCoin::ClosingProcess()
{
    PlayBuffer(SOUND_CLICK01);
    g_pMyInventory->GetInventoryCtrl()->UnlockInventory();
    SocketClient->ToGameServer()->SendCraftingDialogCloseRequest();
}

void CNewUIExchangeLuckyCoin::LockExchangeBtn()
{
    for (int i = 0; i < 3; i++)
    {
        m_BtnExchange[i].Lock();
        m_BtnExchange[i].ChangeTextColor(0xff808080);
    }
}

void CNewUIExchangeLuckyCoin::UnLockExchangeBtn()
{
    for (int i = 0; i < 3; i++)
    {
        m_BtnExchange[i].UnLock();
        m_BtnExchange[i].ChangeTextColor(0xffffffff);
    }
}

void CNewUIExchangeLuckyCoin::LoadImages()
{
    LoadBitmap(L"Interface\\newui_msgbox_back.jpg", IMAGE_EXCHANGE_LUCKYCOIN_WINDOW_BACK, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back04.tga", IMAGE_EXCHANGE_LUCKYCOIN_WINDOW_TOP, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back02-L.tga", IMAGE_EXCHANGE_LUCKYCOIN_WINDOW_LEFT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back02-R.tga", IMAGE_EXCHANGE_LUCKYCOIN_WINDOW_RIGHT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back03.tga", IMAGE_EXCHANGE_LUCKYCOIN_WINDOW_BOTTOM, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_btn_empty_small.tga", IMAGE_EXCHANGE_LUCKYCOIN_WINDOW_BTN_EXIT, GL_LINEAR);	// Exit Button
    LoadBitmap(L"Interface\\newui_btn_empty.tga", IMAGE_EXCHANGE_LUCKYCOIN_EXCHANGE_BTN, GL_LINEAR);				// Exchange Button
}

void CNewUIExchangeLuckyCoin::UnloadImages()
{
    DeleteBitmap(IMAGE_EXCHANGE_LUCKYCOIN_WINDOW_BACK);
    DeleteBitmap(IMAGE_EXCHANGE_LUCKYCOIN_WINDOW_TOP);
    DeleteBitmap(IMAGE_EXCHANGE_LUCKYCOIN_WINDOW_LEFT);
    DeleteBitmap(IMAGE_EXCHANGE_LUCKYCOIN_WINDOW_RIGHT);
    DeleteBitmap(IMAGE_EXCHANGE_LUCKYCOIN_WINDOW_BOTTOM);
    DeleteBitmap(IMAGE_EXCHANGE_LUCKYCOIN_WINDOW_BTN_EXIT);		// Exit Button
    DeleteBitmap(IMAGE_EXCHANGE_LUCKYCOIN_EXCHANGE_BTN);			// Exchange Button
}