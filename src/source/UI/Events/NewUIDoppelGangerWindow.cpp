// NewUIDoppelGangerWindow.cpp: implementation of the CDoppelGangerWindow class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UI/Events/NewUIDoppelGangerWindow.h"
#include "UI/Core/NewUISystem.h"
#include "UI/Dialogs/NewUICustomMessageBox.h"
#include "Render/Models/ZzzBMD.h"
#include "Render/Effects/ZzzEffect.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Object/ZzzInventory.h"
#include "Engine/Object/ZzzInterface.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Engine/Object/ZzzCharacter.h"
#include "I18N/All.h"

#include "Audio/DSPlaySound.h"

using namespace SEASON3B;
using namespace mu::ui::window;

CDoppelGangerWindow::CDoppelGangerWindow()
{
    m_pNewUIMng = NULL;
    m_pNewUI3DRenderMng = NULL;
    m_Pos.x = m_Pos.y = 0;
    m_iRemainTime = 0;
    m_bIsEnterButtonLocked = FALSE;
}

CDoppelGangerWindow::~CDoppelGangerWindow()
{
    Release();
}

bool CDoppelGangerWindow::Create(CManager* pNewUIMng, C3DRenderMng* pNewUI3DRenderMng, int x, int y)
{
    if (NULL == pNewUIMng || NULL == pNewUI3DRenderMng || NULL == g_pNewItemMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_DOPPELGANGER_NPC, this);

    m_pNewUI3DRenderMng = pNewUI3DRenderMng;
    m_pNewUI3DRenderMng->Add3DRenderObj(this, INVENTORY_CAMERA_Z_ORDER);

    SetPos(x, y);

    LoadImages();

    InitButton(&m_BtnEnter, m_Pos.x + INVENTORY_WIDTH / 2 - 27, m_Pos.y + 190, I18N::Game::Enter);
    InitButton(&m_BtnClose, m_Pos.x + INVENTORY_WIDTH / 2 - 27, m_Pos.y + 360, I18N::Game::Close388);

    Show(false);

    return true;
}

void CDoppelGangerWindow::InitButton(CButton* pNewUIButton, int iPos_x, int iPos_y, const wchar_t* pCaption)
{
    pNewUIButton->ChangeText(pCaption);
    pNewUIButton->ChangeTextBackColor(RGBA(255, 255, 255, 0));
    pNewUIButton->ChangeButtonImgState(true, IMAGE_DOPPELGANGERWINDOW_BUTTON, true);
    pNewUIButton->ChangeButtonInfo(iPos_x, iPos_y, 53, 23);
    pNewUIButton->ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
    pNewUIButton->ChangeImgColor(BUTTON_STATE_DOWN, RGBA(255, 255, 255, 255));
}

void CDoppelGangerWindow::Release()
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

void CDoppelGangerWindow::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

bool CDoppelGangerWindow::UpdateMouseEvent()
{
    if (true == BtnProcess())
        return false;

    if (CheckMouseIn(m_Pos.x, m_Pos.y, INVENTORY_WIDTH, INVENTORY_HEIGHT))
        return false;

    return true;
}

bool CDoppelGangerWindow::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_DOPPELGANGER_NPC) == true)
    {
        if (mu::ui::window::IsPress(VK_ESCAPE) == true)
        {
            g_pNewUISystem->Hide(mu::ui::window::INTERFACE_DOPPELGANGER_NPC);
            PlayBuffer(SOUND_CLICK01);
            return false;
        }
    }
    return true;
}

bool CDoppelGangerWindow::Update()
{
    if (IsVisible())
    {
    }
    return true;
}

bool CDoppelGangerWindow::IsVisible() const
{
    return CObject::IsVisible();
}

bool CDoppelGangerWindow::Render()
{
    EnableAlphaTest();

    RenderFrame();

    POINT ptOrigin = { m_Pos.x, m_Pos.y + 50 };
    wchar_t szText[256];

    g_pRenderText->SetFont(g_hFont);
    wchar_t szTextOut[2][300];
    CutStr(I18N::Game::OnlyThoseInPossessionOfAMirrorOfDimensions, szTextOut[0], 140, 2, 300);
    g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y, szTextOut[0], 190, 0, RT3_SORT_CENTER);
    g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y + 15, szTextOut[1], 190, 0, RT3_SORT_CENTER);
    CutStr(I18N::Game::MayPassThroughTheDoppelgangerGate, szTextOut[0], 100, 2, 300);
    g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y + 30, szTextOut[0], 190, 0, RT3_SORT_CENTER);
    g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y + 45, szTextOut[1], 190, 0, RT3_SORT_CENTER);
    g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y + 60, I18N::Game::WillYouShowMeYourMirror, 190, 0, RT3_SORT_CENTER);

    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y + 120, I18N::Game::MirrorOfDimensions, 190, 0, RT3_SORT_CENTER);

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

    RenderImage(IMAGE_DOPPELGANGERWINDOW_LINE, m_Pos.x + 1, m_Pos.y + 130 + 90, 188.f, 21.f);

    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y + 210, I18N::Game::EntryTime, 190, 0, RT3_SORT_CENTER);
    if (m_iRemainTime == 0)
    {
        mu_swprintf(szText, I18N::Game::YouMayNowEnter);
    }
    else
    {
        mu_swprintf(szText, I18N::Game::EnterAfterDMinutes, m_iRemainTime);
    }
    g_pRenderText->RenderText(ptOrigin.x, ptOrigin.y + 230, szText, 190, 0, RT3_SORT_CENTER);

    m_BtnClose.Render();

    DisableAlphaBlend();

    return true;
}

void CDoppelGangerWindow::Render3D()
{
    RenderItem3D();
}

void CDoppelGangerWindow::RenderItem3D()
{
    POINT ptOrigin = { m_Pos.x, m_Pos.y + 50 };

    int nItemType = (14 * MAX_ITEM_INDEX) + 111;
    int nItemLevel = 0;

    ::RenderItem3D(ptOrigin.x + (190 - 20) / 2, ptOrigin.y + 75, 20.f, 27, nItemType, nItemLevel, 0, 0, false);
}

void CDoppelGangerWindow::OpeningProcess()
{
    LockEnterButton(FALSE);
}

void CDoppelGangerWindow::ClosingProcess()
{
    SocketClient->ToGameServer()->SendCloseNpcRequest();
}

float CDoppelGangerWindow::GetLayerDepth()
{
    return 5.0f;
}

void CDoppelGangerWindow::LoadImages()
{
    LoadBitmap(L"Interface\\newui_msgbox_back.jpg", IMAGE_DOPPELGANGERWINDOW_BACK, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back01.tga", IMAGE_DOPPELGANGERWINDOW_TOP, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back02-L.tga", IMAGE_DOPPELGANGERWINDOW_LEFT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back02-R.tga", IMAGE_DOPPELGANGERWINDOW_RIGHT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back03.tga", IMAGE_DOPPELGANGERWINDOW_BOTTOM, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_btn_empty_very_small.tga", IMAGE_DOPPELGANGERWINDOW_BUTTON, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_myquest_Line.tga", IMAGE_DOPPELGANGERWINDOW_LINE, GL_LINEAR);
}

void CDoppelGangerWindow::UnloadImages()
{
    DeleteBitmap(IMAGE_DOPPELGANGERWINDOW_BOTTOM);
    DeleteBitmap(IMAGE_DOPPELGANGERWINDOW_RIGHT);
    DeleteBitmap(IMAGE_DOPPELGANGERWINDOW_LEFT);
    DeleteBitmap(IMAGE_DOPPELGANGERWINDOW_TOP);
    DeleteBitmap(IMAGE_DOPPELGANGERWINDOW_BACK);
    DeleteBitmap(IMAGE_DOPPELGANGERWINDOW_BUTTON);
    DeleteBitmap(IMAGE_DOPPELGANGERWINDOW_LINE);
}

void CDoppelGangerWindow::RenderFrame()
{
    RenderImage(IMAGE_DOPPELGANGERWINDOW_BACK, m_Pos.x, m_Pos.y, 190.f, 429.f);
    RenderImage(IMAGE_DOPPELGANGERWINDOW_TOP, m_Pos.x, m_Pos.y, 190.f, 64.f);
    RenderImage(IMAGE_DOPPELGANGERWINDOW_LEFT, m_Pos.x, m_Pos.y + 64, 21.f, 320.f);
    RenderImage(IMAGE_DOPPELGANGERWINDOW_RIGHT, m_Pos.x + INVENTORY_WIDTH - 21, m_Pos.y + 64, 21.f, 320.f);
    RenderImage(IMAGE_DOPPELGANGERWINDOW_BOTTOM, m_Pos.x, m_Pos.y + INVENTORY_HEIGHT - 45, 190.f, 45.f);

    wchar_t szText[256] = { 0, };
    float fPos_x = m_Pos.x + 15.0f, fPos_y = m_Pos.y;
    float fLine_y = 13.0f;

    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetTextColor(220, 220, 220, 255);
    g_pRenderText->SetBgColor(0, 0, 0, 0);

    mu_swprintf(szText, L"%ls", I18N::Game::Lugard);
    g_pRenderText->RenderText(fPos_x, fPos_y + fLine_y, szText, 160.0f, 0, RT3_SORT_CENTER);
}

bool CDoppelGangerWindow::BtnProcess()
{
    // Top-right corner close "X" (shared frame): hides + swallows the click.
    g_pNewUISystem->HandleFrameCornerClose(m_Pos, mu::ui::window::INTERFACE_DOPPELGANGER_NPC);

    if (m_BtnEnter.UpdateMouseEvent() == true)
    {
        SocketClient->ToGameServer()->SendDoppelgangerEnterRequest(0xFF);
        return true;
    }

    if (m_BtnClose.UpdateMouseEvent() == true)
    {
        g_pNewUISystem->Hide(mu::ui::window::INTERFACE_DOPPELGANGER_NPC);
    }

    return false;
}

void CDoppelGangerWindow::SetRemainTime(int iTime)
{
    m_iRemainTime = iTime;
    if (iTime != 0)
    {
        LockEnterButton(TRUE);
    }
}

void CDoppelGangerWindow::LockEnterButton(BOOL bLock)
{
    m_bIsEnterButtonLocked = bLock;
}
