// NewUIGoldBowmanLena.cpp: implementation of the NewUIGoldBowmanLena class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUIGoldBowmanLena.h"
#include "NewUISystem.h"

#include "MixMgr.h"

namespace
{
    void RenderText(wchar_t* text, int x, int y, int sx, int sy, DWORD color, DWORD backcolor, int sort)
    {
        g_pRenderText->SetFont(g_hFont);

        DWORD backuptextcolor = g_pRenderText->GetTextColor();
        DWORD backuptextbackcolor = g_pRenderText->GetBgColor();

        g_pRenderText->SetTextColor(color);
        g_pRenderText->SetBgColor(backcolor);
        g_pRenderText->RenderText(x, y, text, sx, sy, sort);

        g_pRenderText->SetTextColor(backuptextcolor);
        g_pRenderText->SetBgColor(backuptextbackcolor);
    }
};

using namespace SEASON3B;

CNewUIGoldBowmanLena::CNewUIGoldBowmanLena()
{
}

CNewUIGoldBowmanLena::~CNewUIGoldBowmanLena()
{
    Release();
}

bool CNewUIGoldBowmanLena::Create(CNewUIManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng) {
        return false;
    }

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_GOLD_BOWMAN_LENA, this);

    SetPos(x, y);

    LoadImages();

    // Register Button
    m_BtnRegister.ChangeButtonImgState(true, IMAGE_GBL_BTN_SERIAL, false);
    m_BtnRegister.ChangeButtonInfo(m_Pos.x + 45, m_Pos.y + 285, 108, 29);
    m_BtnRegister.ChangeText(GlobalText[243]);
    m_BtnRegister.ChangeToolTipText(GlobalText[243], true);

    // Exit Button
    m_BtnExit.ChangeButtonImgState(true, IMAGE_GBL_BTN_EXIT, false);
    m_BtnExit.ChangeButtonInfo(m_Pos.x + 13, m_Pos.y + 392, 36, 29);
    m_BtnExit.ChangeToolTipText(GlobalText[1002], true);	// 1002 "닫기"

    Show(false);

    return true;
}

void CNewUIGoldBowmanLena::Release()
{
    UnloadImages();
}

void CNewUIGoldBowmanLena::LoadImages()
{
    LoadBitmap(L"Interface\\newui_msgbox_back.jpg", IMAGE_GBL_BACK, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back04.tga", IMAGE_GBL_TOP, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back02-L.tga", IMAGE_GBL_LEFT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back02-R.tga", IMAGE_GBL_RIGHT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back03.tga", IMAGE_GBL_BOTTOM, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_btn_empty.tga", IMAGE_GBL_EXCHANGEBTN, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_btn_empty.tga", IMAGE_GBL_BTN_SERIAL, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_exit_00.tga", IMAGE_GBL_BTN_EXIT, GL_LINEAR);
}

void CNewUIGoldBowmanLena::UnloadImages()
{
    DeleteBitmap(IMAGE_GBL_BTN_EXIT);
    DeleteBitmap(IMAGE_GBL_BTN_SERIAL);
    DeleteBitmap(IMAGE_GBL_EXCHANGEBTN);
    DeleteBitmap(IMAGE_GBL_BOTTOM);
    DeleteBitmap(IMAGE_GBL_RIGHT);
    DeleteBitmap(IMAGE_GBL_LEFT);
    DeleteBitmap(IMAGE_GBL_TOP);
    DeleteBitmap(IMAGE_GBL_BACK);
}

void CNewUIGoldBowmanLena::OpeningProcess()
{
}

void CNewUIGoldBowmanLena::ClosingProcess()
{
    g_bEventChipDialogEnable = 0;
    g_shEventChipCount = 0;
    SocketClient->ToGameServer()->SendEventChipExitDialog();
}

bool CNewUIGoldBowmanLena::UpdateMouseEvent()
{
    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_GOLD_BOWMAN_LENA) == false) {
        return true;
    }

    if (m_BtnRegister.UpdateMouseEvent()) {
        int registerItem = g_pMyInventory->GetInventoryCtrl()->GetItemCount(ITEM_POTION + 21, 0);

        if (registerItem != 0) {
            int index = g_pMyInventory->GetInventoryCtrl()->FindItemIndex(ITEM_POTION + 21, 0);

            if (index != -1) {
                SocketClient->ToGameServer()->SendEventChipRegistrationRequest(0, index);
            }
        }
    }

    POINT ptExitBtn1 = { m_Pos.x + 169, m_Pos.y + 7 };

    if (SEASON3B::IsPress(VK_LBUTTON) && CheckMouseIn(ptExitBtn1.x, ptExitBtn1.y, 13, 12)) {
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_GOLD_BOWMAN_LENA);
        return false;
    }

    if (m_BtnExit.UpdateMouseEvent()) {
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_GOLD_BOWMAN_LENA);
        return false;
    }

    if (CheckMouseIn(m_Pos.x, m_Pos.y, INVENTORY_WIDTH, INVENTORY_HEIGHT))
    {
        if (SEASON3B::IsPress(VK_RBUTTON)) {
            MouseRButton = false;
            MouseRButtonPop = false;
            MouseRButtonPush = false;
            return false;
        }

        if (SEASON3B::IsNone(VK_LBUTTON) == false) {
            return false;
        }
        return false;
    }
    else
    {
        if (SEASON3B::IsNone(VK_LBUTTON) == false) {
            return false;
        }
        return false;
    }
    return true;
}

bool CNewUIGoldBowmanLena::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_GOLD_BOWMAN_LENA) == false) {
        return true;
    }

    if (SEASON3B::IsPress(VK_ESCAPE) == true) {
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_GOLD_BOWMAN_LENA);
        return false;
    }

    return true;
}

bool CNewUIGoldBowmanLena::Update()
{
    return true;
}

bool CNewUIGoldBowmanLena::Render()
{
    EnableAlphaTest();
    glColor4f(1.f, 1.f, 1.f, 1.f);

    RenderFrame();

    RenderTexts();

    RendeerButton();

    DisableAlphaBlend();

    Render3D();

    return true;
}

void CNewUIGoldBowmanLena::RenderFrame()
{
    // frame
    RenderImage(IMAGE_GBL_BACK, m_Pos.x, m_Pos.y, 190.f, 429.f);
    RenderImage(IMAGE_GBL_TOP, m_Pos.x, m_Pos.y, 190.f, 64.f);
    RenderImage(IMAGE_GBL_LEFT, m_Pos.x, m_Pos.y + 64, 21.f, 320.f);
    RenderImage(IMAGE_GBL_RIGHT, m_Pos.x + INVENTORY_WIDTH - 21, m_Pos.y + 64, 21.f, 320.f);
    RenderImage(IMAGE_GBL_BOTTOM, m_Pos.x, m_Pos.y + INVENTORY_HEIGHT - 45, 190.f, 45.f);
}

void CNewUIGoldBowmanLena::RenderTexts()
{
    wchar_t* name = getMonsterName(236);
    RenderText(name, m_Pos.x, m_Pos.y + 15, 190, 0, 0xFFFFFFFF, 0x00000000, RT3_SORT_CENTER);

    wchar_t Text[100];
    memset(&Text, 0, sizeof(wchar_t) * 100);
    for (int i = 0; i < 3; ++i) {
        memset(&Text, 0, sizeof(wchar_t) * 100);
        swprintf(Text, GlobalText[700 + i]);
        RenderText(Text, m_Pos.x, m_Pos.y + 100 + (i * 15), 190, 0, 0xFFFFFFFF, 0x00000000, RT3_SORT_CENTER);
    }

    int registerItem = g_pMyInventory->GetInventoryCtrl()->GetItemCount(ITEM_POTION + 21, 0);

    memset(&Text, 0, sizeof(wchar_t) * 100);
    swprintf(Text, L"%s", GlobalText[245]);
    RenderText(Text, m_Pos.x + 20, m_Pos.y + 180, 190, 0, 0xFF47DFFA, 0x00000000, RT3_SORT_LEFT);

    memset(&Text, 0, sizeof(wchar_t) * 100);
    swprintf(Text, L"    X    %d", registerItem);
    RenderText(Text, m_Pos.x + 5, m_Pos.y + 202, 190, 0, 0xFFFFFFFF, 0x00000000, RT3_SORT_CENTER);

    memset(&Text, 0, sizeof(wchar_t) * 100);
    swprintf(Text, L"%s", GlobalText[246]);
    RenderText(Text, m_Pos.x + 20, m_Pos.y + 225, 190, 0, 0xFF47DFFA, 0x00000000, RT3_SORT_LEFT);

    memset(&Text, 0, sizeof(wchar_t) * 100);
    swprintf(Text, L"    X    %d", g_shEventChipCount);
    RenderText(Text, m_Pos.x + 5, m_Pos.y + 245, 190, 0, 0xFFFFFFFF, 0x00000000, RT3_SORT_CENTER);

    for (int j = 0; j < 2; ++j) {
        memset(&Text, 0, sizeof(wchar_t) * 100);
        swprintf(Text, GlobalText[703 + j]);
        RenderText(Text, m_Pos.x, m_Pos.y + 350 + (j * 15), 190, 0, 0xFFFA47D6, 0x00000000, RT3_SORT_CENTER);
    }
}

void CNewUIGoldBowmanLena::RendeerButton()
{
    m_BtnRegister.Render();
    m_BtnExit.Render();
}

float CNewUIGoldBowmanLena::GetLayerDepth()	// 3.4f
{
    return 3.4f;
}

void CNewUIGoldBowmanLena::Render3D()
{
    EndBitmap();

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glViewport2(0, 0, WindowWidth, WindowHeight);
    gluPerspective2(1.f, (float)(WindowWidth) / (float)(WindowHeight), RENDER_ITEMVIEW_NEAR, RENDER_ITEMVIEW_FAR);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    GetOpenGLMatrix(CameraMatrix);
    EnableDepthTest();
    EnableDepthMask();

    int Type = ITEM_POTION + 21;
    int Level = 0;
    float x = 640.f - 120.f;
    float y = 200.f;
    float Width = (float)ItemAttribute[Type].Width * INVENTORY_SCALE;
    float Height = (float)ItemAttribute[Type].Height * INVENTORY_SCALE;
    RenderItem3D(x, y, Width, Height, Type, Level, 0, 0, false);
    RenderItem3D(x, y + 42, Width, Height, Type, Level, 0, 0, false);

    UpdateMousePositionn();

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    BeginBitmap();
}