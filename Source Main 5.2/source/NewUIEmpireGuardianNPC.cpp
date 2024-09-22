// CNewUIEmpireGuardianNPC.cpp: implementation of the CNewUIEmpireGuardianNPC class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUISystem.h"
#include "NewUICommon.h"
#include "NewUIEmpireGuardianNPC.h"

#include "DSPlaySound.h"
#include "UIControls.h"

using namespace SEASON3B;

CNewUIEmpireGuardianNPC::CNewUIEmpireGuardianNPC()
{
    m_pNewUIMng = NULL;
    m_Pos.x = m_Pos.y = 0;
    m_bCanClick = true;
}

CNewUIEmpireGuardianNPC::~CNewUIEmpireGuardianNPC()
{
    Release();
}

bool CNewUIEmpireGuardianNPC::Create(CNewUIManager* pNewUIMng, CNewUI3DRenderMng* pNewUI3DRenderMng, int x, int y)
{
    if (NULL == pNewUIMng || NULL == pNewUI3DRenderMng || NULL == g_pNewItemMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_EMPIREGUARDIAN_NPC, this);

    m_pNewUI3DRenderMng = pNewUI3DRenderMng;
    m_pNewUI3DRenderMng->Add3DRenderObj(this, INVENTORY_CAMERA_Z_ORDER);

    SetPos(x, y);

    LoadImages();

    InitButton(&m_btPositive, x + (NPC_WINDOW_WIDTH / 2) - 27, y + 190, GlobalText[1593]);
    InitButton(&m_btNegative, x + (NPC_WINDOW_WIDTH / 2) - 27, y + 380, GlobalText[1002]);

    Show(false);

    return true;
}

void CNewUIEmpireGuardianNPC::InitButton(CNewUIButton* pNewUIButton, int iPos_x, int iPos_y, const wchar_t* pCaption)
{
    pNewUIButton->ChangeText(pCaption);
    pNewUIButton->ChangeTextBackColor(RGBA(255, 255, 255, 0));
    pNewUIButton->ChangeButtonImgState(true, IMAGE_EMPIREGUARDIAN_NPC_BTN, true);
    pNewUIButton->ChangeButtonInfo(iPos_x, iPos_y, 53, 23);
    pNewUIButton->ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
    pNewUIButton->ChangeImgColor(BUTTON_STATE_DOWN, RGBA(255, 255, 255, 255));
}

void CNewUIEmpireGuardianNPC::Release()
{
    UnloadImages();

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void CNewUIEmpireGuardianNPC::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

bool CNewUIEmpireGuardianNPC::UpdateMouseEvent()
{
    if (true == BtnProcess())
        return false;

    if (CheckMouseIn(m_Pos.x, m_Pos.y, NPC_WINDOW_WIDTH, NPC_WINDOW_HEIGHT))
        return false;

    return true;
}

bool CNewUIEmpireGuardianNPC::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_EMPIREGUARDIAN_NPC) == true)
    {
        if (SEASON3B::IsPress(VK_ESCAPE) == true)
        {
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_EMPIREGUARDIAN_NPC);
            PlayBuffer(SOUND_CLICK01);
            return false;
        }
    }
    return true;
}

bool CNewUIEmpireGuardianNPC::Update()
{
    if (!IsVisible())
        return true;

    return true;
}

bool CNewUIEmpireGuardianNPC::IsVisible() const
{
    return CNewUIObj::IsVisible();
}

bool CNewUIEmpireGuardianNPC::Render()
{
    EnableAlphaTest();
    glColor4f(1.f, 1.f, 1.f, 1.f);
    RenderFrame();

    POINT Position = { m_Pos.x + (NPC_WINDOW_WIDTH / 2), m_Pos.y };

    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->SetBgColor(0, 0, 0, 0);
    g_pRenderText->SetTextColor(220, 220, 220, 255);
    g_pRenderText->RenderText(m_Pos.x, Position.y + 50, GlobalText[2795], 190, 0, RT3_SORT_CENTER);
    wchar_t szTextOut[2][300];
    CutStr(GlobalText[2796], szTextOut[0], 150, 2, 300);
    g_pRenderText->RenderText(m_Pos.x, Position.y + 70, szTextOut[0], 190, 0, RT3_SORT_CENTER);
    g_pRenderText->RenderText(m_Pos.x, Position.y + 90, szTextOut[1], 190, 0, RT3_SORT_CENTER);
    g_pRenderText->RenderText(m_Pos.x, Position.y + 110, GlobalText[2797], 190, 0, RT3_SORT_CENTER);

    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetTextColor(255, 240, 0, 255);
    g_pRenderText->RenderText(Position.x - 55, Position.y + 170, GlobalText[2783], 110, 0, RT3_SORT_CENTER);

    m_btPositive.Render();

    RenderImage(IMAGE_EMPIREGUARDIAN_NPC_LINE, m_Pos.x + 1, m_Pos.y + 220, 188.f, 21.f);

    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetTextColor(255, 0, 0, 255);
    g_pRenderText->RenderText(Position.x - 55, Position.y + 260, GlobalText[2223], 110, 0, RT3_SORT_CENTER);

    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->SetTextColor(220, 220, 220, 255);
    g_pRenderText->RenderText(Position.x - 100, Position.y + 280, GlobalText[2835], 200, 0, RT3_SORT_CENTER);
    g_pRenderText->RenderText(Position.x - 100, Position.y + 300, GlobalText[2836], 200, 0, RT3_SORT_CENTER);
    g_pRenderText->RenderText(Position.x - 100, Position.y + 320, GlobalText[2837], 200, 0, RT3_SORT_CENTER);
    CutStr(GlobalText[2838], szTextOut[0], 155, 2, 300);
    g_pRenderText->RenderText(m_Pos.x, Position.y + 340, szTextOut[0], 200, 0, RT3_SORT_CENTER);
    g_pRenderText->RenderText(m_Pos.x, Position.y + 360, szTextOut[1], 200, 0, RT3_SORT_CENTER);

    m_btNegative.Render();
    DisableAlphaBlend();
    return true;
}

bool CNewUIEmpireGuardianNPC::BtnProcess()
{
    POINT ptExitBtn1 = { m_Pos.x + 169, m_Pos.y + 7 };

    if (SEASON3B::IsPress(VK_LBUTTON) && CheckMouseIn(ptExitBtn1.x, ptExitBtn1.y, 13, 12))
    {
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_EMPIREGUARDIAN_NPC);
    }

    if (m_btNegative.UpdateMouseEvent())
    {
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_EMPIREGUARDIAN_NPC);
    }

    if (m_btPositive.UpdateMouseEvent())
    {
        SocketClient->ToGameServer()->SendEnterEmpireGuardianEvent();
        ::PlayBuffer(SOUND_INTERFACE01);
        m_bCanClick = false;
        return true;
    }

    return false;
}

float CNewUIEmpireGuardianNPC::GetLayerDepth()
{
    return 1.2f;
}

void CNewUIEmpireGuardianNPC::OpenningProcess()
{
}

void CNewUIEmpireGuardianNPC::ClosingProcess()
{
}

void CNewUIEmpireGuardianNPC::LoadImages()
{
    LoadBitmap(L"Interface\\newui_msgbox_back.jpg", IMAGE_EMPIREGUARDIAN_NPC_BACK, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back01.tga", IMAGE_EMPIREGUARDIAN_NPC_TOP, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back02-L.tga", IMAGE_EMPIREGUARDIAN_NPC_LEFT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back02-R.tga", IMAGE_EMPIREGUARDIAN_NPC_RIGHT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back03.tga", IMAGE_EMPIREGUARDIAN_NPC_BOTTOM, GL_LINEAR);

    //btn
    LoadBitmap(L"Interface\\newui_btn_empty_very_small.tga", IMAGE_EMPIREGUARDIAN_NPC_BTN, GL_LINEAR);

    //line
    LoadBitmap(L"Interface\\newui_myquest_Line.tga", IMAGE_EMPIREGUARDIAN_NPC_LINE, GL_LINEAR);
}

void CNewUIEmpireGuardianNPC::UnloadImages()
{
    DeleteBitmap(IMAGE_EMPIREGUARDIAN_NPC_TOP);
    DeleteBitmap(IMAGE_EMPIREGUARDIAN_NPC_LEFT);
    DeleteBitmap(IMAGE_EMPIREGUARDIAN_NPC_RIGHT);
    DeleteBitmap(IMAGE_EMPIREGUARDIAN_NPC_BOTTOM);
    DeleteBitmap(IMAGE_EMPIREGUARDIAN_NPC_BACK);
    DeleteBitmap(IMAGE_EMPIREGUARDIAN_NPC_BTN);
    DeleteBitmap(IMAGE_EMPIREGUARDIAN_NPC_LINE);
}

void CNewUIEmpireGuardianNPC::RenderFrame()
{
    RenderImage(IMAGE_EMPIREGUARDIAN_NPC_BACK, m_Pos.x, m_Pos.y, float(NPC_WINDOW_WIDTH), float(NPC_WINDOW_HEIGHT));
    RenderImage(IMAGE_EMPIREGUARDIAN_NPC_TOP, m_Pos.x, m_Pos.y, float(NPC_WINDOW_WIDTH), 64.f);
    RenderImage(IMAGE_EMPIREGUARDIAN_NPC_LEFT, m_Pos.x, m_Pos.y + 64, 21.f, 320.f);
    RenderImage(IMAGE_EMPIREGUARDIAN_NPC_RIGHT, m_Pos.x + NPC_WINDOW_WIDTH - 21, m_Pos.y + 64, 21.f, 320.f);
    RenderImage(IMAGE_EMPIREGUARDIAN_NPC_BOTTOM, m_Pos.x, m_Pos.y + NPC_WINDOW_HEIGHT - 45, float(NPC_WINDOW_WIDTH), 45.f);

    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetTextColor(220, 220, 220, 255);
    g_pRenderText->SetBgColor(0, 0, 0, 0);
    g_pRenderText->RenderText(m_Pos.x + (NPC_WINDOW_WIDTH / 2) - 55, m_Pos.y + 13, GlobalText[2794], 110, 0, RT3_SORT_CENTER);
}

void CNewUIEmpireGuardianNPC::Render3D()
{
    RenderItem3D();
}

void CNewUIEmpireGuardianNPC::RenderItem3D()
{
    POINT ptOrigin = { m_Pos.x, m_Pos.y + 50 };

    int nItemType = ITEM_GAIONS_ORDER;
    int nItemLevel = 0;

    ::RenderItem3D(ptOrigin.x + (190 - 20) / 2, ptOrigin.y + 70, 20.0f, 27.0f, nItemType, nItemLevel, 0, 0, false);
}