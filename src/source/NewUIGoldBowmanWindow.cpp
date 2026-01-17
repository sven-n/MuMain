// NewUIGoldBowmanWindow.cpp: implementation of the CNewUIGoldBowmanWindow class.
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "NewUIGoldBowmanWindow.h"
#include "NewUISystem.h"

#include "MixMgr.h"

#define MAXGOLDBOWMANSESERIAL 12

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

CNewUIGoldBowmanWindow::CNewUIGoldBowmanWindow()
{
    m_pNewUIMng = NULL;
    m_Pos.x = m_Pos.y = 0;
    ZeroMemory(g_strGiftName, sizeof(char) * 64);
}

CNewUIGoldBowmanWindow::~CNewUIGoldBowmanWindow()
{
    Release();
}

bool CNewUIGoldBowmanWindow::Create(CNewUIManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_GOLD_BOWMAN, this);

    SetPos(x, y);

    LoadImages();

    // Input Box
    m_EditBox = new CUITextInputBox;
    m_EditBox->Init(g_hWnd, 200, 14, MAXGOLDBOWMANSESERIAL);
    m_EditBox->SetPosition(m_Pos.x + 50, m_Pos.y + 260);
    m_EditBox->SetTextColor(255, 255, 230, 230);
    m_EditBox->SetBackColor(0, 0, 0, 25);
    m_EditBox->SetFont(g_hFont);
    m_EditBox->SetState(UISTATE_DISABLE);

    // Serial Button
    m_BtnSerial.ChangeButtonImgState(true, IMAGE_GB_BTN_SERIAL, false);
    m_BtnSerial.ChangeButtonInfo(m_Pos.x + 45, m_Pos.y + 285, 108, 29);
    m_BtnSerial.ChangeText(GlobalText[895]);

    // Exit Button
    m_BtnExit.ChangeButtonImgState(true, IMAGE_GB_BTN_EXIT, false);
    m_BtnExit.ChangeButtonInfo(m_Pos.x + 13, m_Pos.y + 392, 36, 29);
    m_BtnExit.ChangeToolTipText(GlobalText[1002], true);

    Show(false);

    return true;
}

void CNewUIGoldBowmanWindow::Release()
{
    UnloadImages();

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
    SAFE_DELETE(m_EditBox);
}

void CNewUIGoldBowmanWindow::OpeningProcess()
{
    ZeroMemory(g_strGiftName, sizeof(char) * 64);
    ChangeEditBox(UISTATE_NORMAL);
}

void CNewUIGoldBowmanWindow::ClosingProcess()
{
    ZeroMemory(g_strGiftName, sizeof(char) * 64);
    ChangeEditBox(UISTATE_HIDE);
    SocketClient->ToGameServer()->SendEventChipExitDialog();
}

void CNewUIGoldBowmanWindow::ChangeEditBox(const UISTATES type)
{
    m_EditBox->SetState(type);

    if (type == UISTATE_NORMAL)
    {
        m_EditBox->GiveFocus();
    }

    m_EditBox->SetText(NULL);
}

bool CNewUIGoldBowmanWindow::UpdateMouseEvent()
{
    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_GOLD_BOWMAN) == false) {
        return true;
    }

    if (m_EditBox) {
        m_EditBox->DoAction();
    }

    POINT ptExitBtn1 = { m_Pos.x + 169, m_Pos.y + 7 };

    if (SEASON3B::IsPress(VK_LBUTTON) && CheckMouseIn(ptExitBtn1.x, ptExitBtn1.y, 13, 12))
    {
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_GOLD_BOWMAN);
        return false;
    }

    if (m_BtnExit.UpdateMouseEvent())
    {
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_GOLD_BOWMAN);
        return false;
    }

    if (m_EditBox && m_BtnSerial.UpdateMouseEvent())
    {
        SEASON3B::CNewUIInventoryCtrl* pNewInventoryCtrl = g_pMyInventory->GetInventoryCtrl();
        if (pNewInventoryCtrl->FindEmptySlot(2, 4) == -1)
        {
            SEASON3B::CreateOkMessageBox(GlobalText[896]);
        }
        else
        {
            wchar_t strSerial[12];
            memset(&strSerial, 0, sizeof(char) * 12);
            m_EditBox->GetText(strSerial);

            wchar_t strSerial1[5] = L"0,";
            memcpy(strSerial1, strSerial, sizeof(char) * 4); strSerial1[4] = 0;

            wchar_t strSerial2[5] = L"0,";
            memcpy(strSerial2, strSerial + 4, sizeof(char) * 4); strSerial2[4] = 0;

            wchar_t strSerial3[5] = L"0,";
            memcpy(strSerial3, strSerial + 8, sizeof(char) * 4); strSerial3[4] = 0;

            SocketClient->ToGameServer()->SendLuckyNumberRequest(strSerial1, strSerial2, strSerial3);
        }
    }

    if (CheckMouseIn(m_Pos.x, m_Pos.y, INVENTORY_WIDTH, INVENTORY_HEIGHT))
    {
        if (SEASON3B::IsPress(VK_RBUTTON))
        {
            MouseRButton = false;
            MouseRButtonPop = false;
            MouseRButtonPush = false;
            return false;
        }

        if (SEASON3B::IsNone(VK_LBUTTON) == false)
        {
            return false;
        }
    }
    else
    {
        if (SEASON3B::IsNone(VK_LBUTTON) == false)
        {
            return false;
        }
    }

    return true;
}

bool CNewUIGoldBowmanWindow::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_GOLD_BOWMAN) == false) {
        return true;
    }

    if (SEASON3B::IsPress(VK_ESCAPE) == true)
    {
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_GOLD_BOWMAN);
        return false;
    }

    return true;
}

bool CNewUIGoldBowmanWindow::Update()
{
    if (!IsVisible())
        return true;

    if (m_EditBox)
    {
        if (m_EditBox->HaveFocus() && GetRelatedWnd() != m_EditBox->GetHandle())
        {
            SetRelatedWnd(m_EditBox->GetHandle());
        }
        if (!m_EditBox->HaveFocus() && GetRelatedWnd() != g_hWnd)
        {
            SetRelatedWnd(g_hWnd);
        }
    }
    return true;
}

void CNewUIGoldBowmanWindow::RenderFrame()
{
    // frame
    RenderImage(IMAGE_GB_BACK, m_Pos.x, m_Pos.y, 190.f, 429.f);
    RenderImage(IMAGE_GB_TOP, m_Pos.x, m_Pos.y, 190.f, 64.f);
    RenderImage(IMAGE_GB_LEFT, m_Pos.x, m_Pos.y + 64, 21.f, 320.f);
    RenderImage(IMAGE_GB_RIGHT, m_Pos.x + INVENTORY_WIDTH - 21, m_Pos.y + 64, 21.f, 320.f);
    RenderImage(IMAGE_GB_BOTTOM, m_Pos.x, m_Pos.y + INVENTORY_HEIGHT - 45, 190.f, 45.f);

    // edit box
    RenderImage(IMAGE_GB_EDITBOX, m_Pos.x + 45, m_Pos.y + 255, 108.f, 23.f);
}

void CNewUIGoldBowmanWindow::RenderTexts()
{
    wchar_t Text[100];

    auto name = getMonsterName(236); // npc Name file
    RenderText(name, m_Pos.x, m_Pos.y + 15, 190, 0, 0xFFFFFFFF, 0x00000000, RT3_SORT_CENTER);

    memset(&Text, 0, sizeof(char) * 100);
    swprintf(Text, GlobalText[891]); //"100%%
    RenderText(Text, m_Pos.x, m_Pos.y + 80, 190, 0, 0xFFFFFFFF, 0x00000000, RT3_SORT_CENTER);

    memset(&Text, 0, sizeof(char) * 100);
    swprintf(Text, GlobalText[892]);
    RenderText(Text, m_Pos.x, m_Pos.y + 95, 190, 0, 0xFFFFFFFF, 0x00000000, RT3_SORT_CENTER);

    memset(&Text, 0, sizeof(char) * 100);
    swprintf(Text, GlobalText[897]);
    RenderText(Text, m_Pos.x, m_Pos.y + 110, 190, 0, 0xFFFFFFFF, 0x00000000, RT3_SORT_CENTER);

    memset(&Text, 0, sizeof(char) * 100);
    swprintf(Text, GlobalText[898]);
    RenderText(Text, m_Pos.x, m_Pos.y + 125, 190, 0, 0xFFFFFFFF, 0x00000000, RT3_SORT_CENTER);

    /////////////////////////////// bottom text /////////////////////////////////////////////////////

    memset(&Text, 0, sizeof(char) * 100);
    swprintf(Text, GlobalText[893]);
    RenderText(Text, m_Pos.x, m_Pos.y + 180, 190, 0, 0xFFFFFFFF, 0x00000000, RT3_SORT_CENTER);

    memset(&Text, 0, sizeof(char) * 100);
    swprintf(Text, GlobalText[894]);
    RenderText(Text, m_Pos.x, m_Pos.y + 195, 190, 0, 0xFF18FF00, 0x00000000, RT3_SORT_CENTER);

    memset(&Text, 0, sizeof(char) * 100);
    swprintf(Text, GlobalText[916]);
    RenderText(Text, m_Pos.x, m_Pos.y + 210, 190, 0, 0xFFFFFFFF, 0x00000000, RT3_SORT_CENTER);

    memset(&Text, 0, sizeof(char) * 100);
    swprintf(Text, GlobalText[917]);
    RenderText(Text, m_Pos.x, m_Pos.y + 225, 190, 0, 0xFFFFFFFF, 0x00000000, RT3_SORT_CENTER);

    if (wcscmp(g_strGiftName, L""))
    {
        RenderText(g_strGiftName, m_Pos.x, m_Pos.y + 330, 190, 0, 0xFFFFD200, 0x00000000, RT3_SORT_CENTER);
    }

    if (m_EditBox) {
        m_EditBox->Render();
    }
}

void CNewUIGoldBowmanWindow::RendeerButton()
{
    m_BtnSerial.Render();

    m_BtnExit.Render();
}

bool CNewUIGoldBowmanWindow::Render()
{
    EnableAlphaTest();
    glColor4f(1.f, 1.f, 1.f, 1.f);
    RenderFrame();
    RenderTexts();
    RendeerButton();
    DisableAlphaBlend();
    return true;
}

float CNewUIGoldBowmanWindow::GetLayerDepth()
{
    return 3.4f;
}

void CNewUIGoldBowmanWindow::LoadImages()
{
    LoadBitmap(L"Interface\\newui_msgbox_back.jpg", IMAGE_GB_BACK, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back04.tga", IMAGE_GB_TOP, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back02-L.tga", IMAGE_GB_LEFT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back02-R.tga", IMAGE_GB_RIGHT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back03.tga", IMAGE_GB_BOTTOM, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_btn_empty.tga", IMAGE_GB_EXCHANGEBTN, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_btn_empty.tga", IMAGE_GB_BTN_SERIAL, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_exit_00.tga", IMAGE_GB_BTN_EXIT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_guildmakeeditbox.tga", IMAGE_GB_EDITBOX, GL_LINEAR);
}

void CNewUIGoldBowmanWindow::UnloadImages()
{
    DeleteBitmap(IMAGE_GB_EDITBOX);
    DeleteBitmap(IMAGE_GB_BTN_EXIT);
    DeleteBitmap(IMAGE_GB_BTN_SERIAL);
    DeleteBitmap(IMAGE_GB_EXCHANGEBTN);
    DeleteBitmap(IMAGE_GB_BOTTOM);
    DeleteBitmap(IMAGE_GB_RIGHT);
    DeleteBitmap(IMAGE_GB_LEFT);
    DeleteBitmap(IMAGE_GB_TOP);
    DeleteBitmap(IMAGE_GB_BACK);
}