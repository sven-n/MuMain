// NewUIUnitedMarketPlaceWindow.cpp: implementation of the CNewUIUnitedMarketPlaceWindow class.
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "NewUIUnitedMarketPlaceWindow.h"
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
#include "MapManager.h"

using namespace SEASON3B;

CNewUIUnitedMarketPlaceWindow::CNewUIUnitedMarketPlaceWindow()
{
    m_pNewUIMng = NULL;
    m_pNewUI3DRenderMng = NULL;
    m_Pos.x = m_Pos.y = 0;
    m_iRemainTime = 0;
    m_bIsEnterButtonLocked = FALSE;
}

CNewUIUnitedMarketPlaceWindow::~CNewUIUnitedMarketPlaceWindow()
{
    Release();
}

bool CNewUIUnitedMarketPlaceWindow::Create(CNewUIManager* pNewUIMng, CNewUI3DRenderMng* pNewUI3DRenderMng, int x, int y)
{
    if (NULL == pNewUIMng || NULL == pNewUI3DRenderMng || NULL == g_pNewItemMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_UNITEDMARKETPLACE_NPC_JULIA, this);

    m_pNewUI3DRenderMng = pNewUI3DRenderMng;
    m_pNewUI3DRenderMng->Add3DRenderObj(this, INVENTORY_CAMERA_Z_ORDER);

    SetPos(x, y);

    LoadImages();

    InitButton(&m_BtnEnter, m_Pos.x + INVENTORY_WIDTH / 2 - 27, m_Pos.y + 230, GlobalText[3016]);

    m_BtnClose.ChangeButtonImgState(true, IMAGE_UNITEDMARKETPLACEWINDOW_BTN_CLOSE);
    m_BtnClose.ChangeButtonInfo(x + 13, y + 392, 36, 29);
    m_BtnClose.ChangeToolTipText(GlobalText[1002], true);

    Show(false);

    return true;
}

void CNewUIUnitedMarketPlaceWindow::InitButton(CNewUIButton* pNewUIButton, int iPos_x, int iPos_y, const wchar_t* pCaption)
{
    pNewUIButton->ChangeText(pCaption);
    pNewUIButton->ChangeTextBackColor(RGBA(255, 255, 255, 0));
    pNewUIButton->ChangeButtonImgState(true, IMAGE_UNITEDMARKETPLACEWINDOW_BUTTON, true);
    pNewUIButton->ChangeButtonInfo(iPos_x, iPos_y, 53, 23);
    pNewUIButton->ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
    pNewUIButton->ChangeImgColor(BUTTON_STATE_DOWN, RGBA(255, 255, 255, 255));
}

void CNewUIUnitedMarketPlaceWindow::Release()
{
    UnloadImages();

    if (m_pNewUI3DRenderMng)
    {
        m_pNewUI3DRenderMng->Remove3DRenderObj(this);
        m_pNewUI3DRenderMng = NULL;
    }

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void CNewUIUnitedMarketPlaceWindow::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

bool CNewUIUnitedMarketPlaceWindow::UpdateMouseEvent()
{
    if (true == BtnProcess())
        return false;

    if (CheckMouseIn(m_Pos.x, m_Pos.y, INVENTORY_WIDTH, INVENTORY_HEIGHT))
        return false;

    return true;
}

bool CNewUIUnitedMarketPlaceWindow::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_UNITEDMARKETPLACE_NPC_JULIA) == true)
    {
        if (SEASON3B::IsPress(VK_ESCAPE) == true)
        {
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_UNITEDMARKETPLACE_NPC_JULIA);
            PlayBuffer(SOUND_CLICK01);
            return false;
        }
    }
    return true;
}

bool CNewUIUnitedMarketPlaceWindow::Update()
{
    if (IsVisible())
    {
        // 		for (int i = 0; i < 4; ++i)
        // 		{
        // 			if (g_DuelMgr.IsDuelChannelEnabled(i))
        // 				m_bChannelEnable[i] = TRUE;
        // 			else
        // 				m_bChannelEnable[i] = FALSE;
        //
        // 			// 버튼 잠금
        // 			if (m_bChannelEnable[i] == TRUE && g_DuelMgr.IsDuelChannelJoinable(i))
        // 			{
        // 				m_BtnChannel[i].UnLock();
        // 				m_BtnChannel[i].ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
        // 				m_BtnChannel[i].ChangeTextColor(RGBA(255, 255, 255, 255));
        // 			}
        // 			else
        // 			{
        // 				m_BtnChannel[i].Lock();
        // 				m_BtnChannel[i].ChangeImgColor(BUTTON_STATE_UP, RGBA(100, 100, 100, 255));
        // 				m_BtnChannel[i].ChangeTextColor(RGBA(100, 100, 100, 255));
        // 			}
        // 		}
    }
    return true;
}

bool CNewUIUnitedMarketPlaceWindow::IsVisible() const
{
    return CNewUIObj::IsVisible();
}

bool CNewUIUnitedMarketPlaceWindow::Render()
{
    EnableAlphaTest();
    glColor4f(1.f, 1.f, 1.f, 1.f);

    RenderFrame();

    POINT ptOrigin = { m_Pos.x, m_Pos.y + 50 };
    //char szText[256];

    if (gMapManager.WorldActive == WD_79UNITEDMARKETPLACE)
    {
        g_pRenderText->SetFont(g_hFont);
        g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y + 10, GlobalText[3010], 190, 0, RT3_SORT_CENTER);
        g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y + 30, GlobalText[3011], 190, 0, RT3_SORT_CENTER);
        g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y + 50, GlobalText[3012], 190, 0, RT3_SORT_CENTER);

        g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y + 130, GlobalText[3013], 190, 0, RT3_SORT_CENTER);
    }
    else
    {
        g_pRenderText->SetFont(g_hFont);
        g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y + 10, GlobalText[3003], 190, 0, RT3_SORT_CENTER);
        g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y + 30, GlobalText[3004], 190, 0, RT3_SORT_CENTER);
        g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y + 50, GlobalText[3005], 190, 0, RT3_SORT_CENTER);
        g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y + 70, GlobalText[3006], 190, 0, RT3_SORT_CENTER);
        g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y + 90, GlobalText[3007], 190, 0, RT3_SORT_CENTER);
        g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y + 110, GlobalText[3008], 190, 0, RT3_SORT_CENTER);
        g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y + 130, GlobalText[3009], 190, 0, RT3_SORT_CENTER);
    }

    if (m_bIsEnterButtonLocked == TRUE)
    {
        m_BtnEnter.Lock();
        m_BtnEnter.ChangeImgColor(BUTTON_STATE_UP, RGBA(100, 100, 100, 255));
        m_BtnEnter.ChangeImgColor(BUTTON_STATE_DOWN, RGBA(100, 100, 100, 255));
        m_BtnEnter.ChangeImgColor(BUTTON_STATE_OVER, RGBA(100, 100, 100, 255));
        m_BtnEnter.ChangeTextColor(RGBA(100, 100, 100, 255));
    }
    else
    {
        m_BtnEnter.UnLock();
        m_BtnEnter.ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
        m_BtnEnter.ChangeImgColor(BUTTON_STATE_DOWN, RGBA(255, 255, 255, 255));
        m_BtnEnter.ChangeImgColor(BUTTON_STATE_OVER, RGBA(255, 255, 255, 255));
        m_BtnEnter.ChangeTextColor(RGBA(255, 255, 255, 255));
    }
    m_BtnEnter.Render();
    m_BtnClose.Render();

    DisableAlphaBlend();
    return true;
}

void CNewUIUnitedMarketPlaceWindow::Render3D()
{
    //RenderItem3D();
}

void CNewUIUnitedMarketPlaceWindow::RenderItem3D()
{
    // 	POINT ptOrigin = { m_Pos.x, m_Pos.y+50 };
    //
    // 	int nItemType = (14*MAX_ITEM_INDEX)+111;
    //     int nItemLevel = 0;
    //
    // 	::RenderItem3D(ptOrigin.x+(190-20)/2, ptOrigin.y+75, 20.f, 27, nItemType, nItemLevel<<3, 0, 0, false);
}

void CNewUIUnitedMarketPlaceWindow::OpeningProcess()
{
    LockEnterButton(FALSE);
}

void CNewUIUnitedMarketPlaceWindow::ClosingProcess()
{
    // 	SocketClient->ToGameServer()->SendCloseNpcRequest();
}

float CNewUIUnitedMarketPlaceWindow::GetLayerDepth()
{
    return 5.0f;
}

void CNewUIUnitedMarketPlaceWindow::LoadImages()
{
    LoadBitmap(L"Interface\\newui_msgbox_back.jpg", IMAGE_UNITEDMARKETPLACEWINDOW_BACK, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back01.tga", IMAGE_UNITEDMARKETPLACEWINDOW_TOP, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back02-L.tga", IMAGE_UNITEDMARKETPLACEWINDOW_LEFT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back02-R.tga", IMAGE_UNITEDMARKETPLACEWINDOW_RIGHT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back03.tga", IMAGE_UNITEDMARKETPLACEWINDOW_BOTTOM, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_btn_empty_very_small.tga", IMAGE_UNITEDMARKETPLACEWINDOW_BUTTON, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_myquest_Line.tga", IMAGE_UNITEDMARKETPLACEWINDOW_LINE, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_exit_00.tga", IMAGE_UNITEDMARKETPLACEWINDOW_BTN_CLOSE, GL_LINEAR);
}

void CNewUIUnitedMarketPlaceWindow::UnloadImages()
{
    DeleteBitmap(IMAGE_UNITEDMARKETPLACEWINDOW_BOTTOM);
    DeleteBitmap(IMAGE_UNITEDMARKETPLACEWINDOW_RIGHT);
    DeleteBitmap(IMAGE_UNITEDMARKETPLACEWINDOW_LEFT);
    DeleteBitmap(IMAGE_UNITEDMARKETPLACEWINDOW_TOP);
    DeleteBitmap(IMAGE_UNITEDMARKETPLACEWINDOW_BACK);
    DeleteBitmap(IMAGE_UNITEDMARKETPLACEWINDOW_BUTTON);
    DeleteBitmap(IMAGE_UNITEDMARKETPLACEWINDOW_LINE);
}

void CNewUIUnitedMarketPlaceWindow::RenderFrame()
{
    RenderImage(IMAGE_UNITEDMARKETPLACEWINDOW_BACK, m_Pos.x, m_Pos.y, 190.f, 429.f);
    RenderImage(IMAGE_UNITEDMARKETPLACEWINDOW_TOP, m_Pos.x, m_Pos.y, 190.f, 64.f);
    RenderImage(IMAGE_UNITEDMARKETPLACEWINDOW_LEFT, m_Pos.x, m_Pos.y + 64, 21.f, 320.f);
    RenderImage(IMAGE_UNITEDMARKETPLACEWINDOW_RIGHT, m_Pos.x + INVENTORY_WIDTH - 21, m_Pos.y + 64, 21.f, 320.f);
    RenderImage(IMAGE_UNITEDMARKETPLACEWINDOW_BOTTOM, m_Pos.x, m_Pos.y + INVENTORY_HEIGHT - 45, 190.f, 45.f);

    wchar_t szText[256] = { 0, };
    float fPos_x = m_Pos.x + 15.0f, fPos_y = m_Pos.y;
    float fLine_y = 13.0f;

    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetTextColor(220, 220, 220, 255);
    g_pRenderText->SetBgColor(0, 0, 0, 0);

    swprintf(szText, L"%s", GlobalText[3000]);
    g_pRenderText->RenderText(fPos_x, fPos_y + fLine_y, szText, 160.0f, 0, RT3_SORT_CENTER);
}

bool CNewUIUnitedMarketPlaceWindow::BtnProcess()
{
    POINT ptExitBtn1 = { m_Pos.x + 169, m_Pos.y + 7 };

    if (SEASON3B::IsPress(VK_LBUTTON) && CheckMouseIn(ptExitBtn1.x, ptExitBtn1.y, 13, 12))
    {
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_UNITEDMARKETPLACE_NPC_JULIA);
    }

    if (m_BtnEnter.UpdateMouseEvent() == true)
    {
        LoadingWorld = 9999999;

        SocketClient->ToGameServer()->SendEnterMarketPlaceRequest();
        m_bIsEnterButtonLocked = true;

        return true;
    }

    if (m_BtnClose.UpdateMouseEvent() == true)
    {
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_UNITEDMARKETPLACE_NPC_JULIA);
    }

    return false;
}

void CNewUIUnitedMarketPlaceWindow::SetRemainTime(int iTime)
{
    m_iRemainTime = iTime;
    if (iTime != 0)
    {
        LockEnterButton(TRUE);
    }
}

void CNewUIUnitedMarketPlaceWindow::LockEnterButton(BOOL bLock)
{
    m_bIsEnterButtonLocked = bLock;
}