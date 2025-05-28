// NewUIMyShopInventory.cpp: implementation of the CNewUIMyShopInventory class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUIMyShopInventory.h"
#include "DSPlaySound.h"
#include "NewUISystem.h"
#include "NewUICustomMessageBox.h"
#include "PersonalShopTitleImp.h"

const int iMAX_SHOPTITLE_MULTI = 26;

namespace
{
    void RenderText(const wchar_t* text, int x, int y, int sx, int sy, DWORD color, DWORD backcolor, int sort, HFONT hFont = g_hFont)
    {
        g_pRenderText->SetFont(hFont);

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

SEASON3B::CNewUIMyShopInventory::CNewUIMyShopInventory() : m_SourceIndex(-1), m_TargetIndex(-1), m_EnablePersonalShop(false)
{
    m_pNewUIMng = NULL;
    m_pNewInventoryCtrl = NULL;
    m_Pos.x = m_Pos.y = 0;
    m_EditBox = NULL;
    m_Button = NULL;
    m_bIsEnableInputValueTextBox = false;
}

SEASON3B::CNewUIMyShopInventory::~CNewUIMyShopInventory()
{
    Release();
}

bool SEASON3B::CNewUIMyShopInventory::Create(CNewUIManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng || NULL == g_pNewUI3DRenderMng || NULL == g_pNewItemMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_MYSHOP_INVENTORY, this);

    SetPos(x, y);

    LoadImages();

    m_pNewInventoryCtrl = new CNewUIInventoryCtrl;
    if (false == m_pNewInventoryCtrl->Create(STORAGE_TYPE::MYSHOP, g_pNewUI3DRenderMng, g_pNewItemMng, this, m_Pos.x + 16, m_Pos.y + 90, 8, 4, MAX_MY_INVENTORY_EX_INDEX))
    {
        SAFE_DELETE(m_pNewInventoryCtrl);
        return false;
    }

    m_pNewInventoryCtrl->SetToolTipType(TOOLTIP_TYPE_MY_SHOP);

    m_Button = new CNewUIButton[MYSHOPINVENTORY_MAXBUTTONCOUNT];

    m_Button[MYSHOPINVENTORY_EXIT].ChangeButtonImgState(true, IMAGE_MYSHOPINVENTORY_EXIT_BTN, false);
    m_Button[MYSHOPINVENTORY_EXIT].ChangeButtonInfo(m_Pos.x + 13, m_Pos.y + 391, 36, 29);
    m_Button[MYSHOPINVENTORY_EXIT].ChangeToolTipText(GlobalText[1002], true);

    m_Button[MYSHOPINVENTORY_OPEN].ChangeButtonImgState(true, IMAGE_MYSHOPINVENTORY_OPEN, false);
    m_Button[MYSHOPINVENTORY_OPEN].ChangeButtonInfo(m_Pos.x + 53, m_Pos.y + 391, 36, 29);
    m_Button[MYSHOPINVENTORY_OPEN].ChangeToolTipText(GlobalText[1107], true);

    m_Button[MYSHOPINVENTORY_CLOSE].ChangeButtonImgState(true, IMAGE_MYSHOPINVENTORY_CLOSE, false);
    m_Button[MYSHOPINVENTORY_CLOSE].ChangeButtonInfo(m_Pos.x + 93, m_Pos.y + 391, 36, 29);
    m_Button[MYSHOPINVENTORY_CLOSE].ChangeToolTipText(GlobalText[1108], true);

    m_EditBox = new CUITextInputBox;

    m_EditBox->Init(g_hWnd, 200, 14, iMAX_SHOPTITLE_MULTI - 1);
    m_EditBox->SetPosition(m_Pos.x + 50, m_Pos.y + 55);
    m_EditBox->SetTextColor(255, 255, 230, 210);
    m_EditBox->SetBackColor(0, 0, 0, 25);
    m_EditBox->SetFont(g_hFont);

    ChangeEditBox(UISTATE_NORMAL);
    ChangePersonal(m_EnablePersonalShop);

    Show(false);

    return true;
}

void SEASON3B::CNewUIMyShopInventory::Release()
{
    SAFE_DELETE(m_pNewInventoryCtrl);
    SAFE_DELETE_ARRAY(m_Button);
    SAFE_DELETE(m_EditBox);

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }

    UnloadImages();
}

void SEASON3B::CNewUIMyShopInventory::LoadImages()
{
    LoadBitmap(L"Interface\\newui_msgbox_back.jpg", IMAGE_MYSHOPINVENTORY_BACK, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back01.tga", IMAGE_MYSHOPINVENTORY_TOP, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back02-L.tga", IMAGE_MYSHOPINVENTORY_LEFT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back02-R.tga", IMAGE_MYSHOPINVENTORY_RIGHT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back03.tga", IMAGE_MYSHOPINVENTORY_BOTTOM, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_exit_00.tga", IMAGE_MYSHOPINVENTORY_EXIT_BTN, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_Box_openTitle.tga", IMAGE_MYSHOPINVENTORY_EDIT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_Bt_openshop.tga", IMAGE_MYSHOPINVENTORY_OPEN, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_Bt_closeshop.tga", IMAGE_MYSHOPINVENTORY_CLOSE, GL_LINEAR);
}

void SEASON3B::CNewUIMyShopInventory::UnloadImages()
{
    DeleteBitmap(IMAGE_MYSHOPINVENTORY_CLOSE);
    DeleteBitmap(IMAGE_MYSHOPINVENTORY_OPEN);
    DeleteBitmap(IMAGE_MYSHOPINVENTORY_EDIT);
    DeleteBitmap(IMAGE_MYSHOPINVENTORY_EXIT_BTN);
    DeleteBitmap(IMAGE_MYSHOPINVENTORY_BOTTOM);
    DeleteBitmap(IMAGE_MYSHOPINVENTORY_RIGHT);
    DeleteBitmap(IMAGE_MYSHOPINVENTORY_LEFT);
    DeleteBitmap(IMAGE_MYSHOPINVENTORY_TOP);
    DeleteBitmap(IMAGE_MYSHOPINVENTORY_BACK);
}

void SEASON3B::CNewUIMyShopInventory::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;

    if (m_pNewInventoryCtrl)
    {
        m_pNewInventoryCtrl->SetPos(m_Pos.x + 16, m_Pos.y + 90);
    }
    if (m_Button)
    {
        m_Button[MYSHOPINVENTORY_EXIT].SetPos(m_Pos.x + 13, m_Pos.y + 391);
        m_Button[MYSHOPINVENTORY_OPEN].SetPos(m_Pos.x + 53, m_Pos.y + 391);
        m_Button[MYSHOPINVENTORY_CLOSE].SetPos(m_Pos.x + 93, m_Pos.y + 391);
    }
}

void SEASON3B::CNewUIMyShopInventory::GetTitle(wchar_t* titletext)
{
     m_EditBox->GetText(titletext, iMAX_SHOPTITLE_MULTI);
}

void SEASON3B::CNewUIMyShopInventory::SetTitle(wchar_t* titletext)
{
    m_EditBox->SetText(titletext);
}

bool SEASON3B::CNewUIMyShopInventory::InsertItem(int iIndex, std::span<const BYTE> pbyItemPacket)
{
    if (m_pNewInventoryCtrl)
    {
        return m_pNewInventoryCtrl->AddItem(iIndex, pbyItemPacket);
    }

    return false;
}

void SEASON3B::CNewUIMyShopInventory::DeleteItem(int iIndex)
{
    if (m_pNewInventoryCtrl)
    {
        ITEM* pItem = m_pNewInventoryCtrl->FindItem(iIndex);
        if (pItem != NULL)
            m_pNewInventoryCtrl->RemoveItem(pItem);
    }
}

void SEASON3B::CNewUIMyShopInventory::DeleteAllItems()
{
    if (m_pNewInventoryCtrl)
    {
        m_pNewInventoryCtrl->RemoveAllItems();
    }
}

ITEM* SEASON3B::CNewUIMyShopInventory::FindItem(int iLinealPos)
{
    if (m_pNewInventoryCtrl)
        return m_pNewInventoryCtrl->FindItem(iLinealPos);
    return NULL;
}

void SEASON3B::CNewUIMyShopInventory::ChangePersonal(bool state)
{
    m_EnablePersonalShop = state;

    if (m_EnablePersonalShop)
    {
        m_Button[MYSHOPINVENTORY_OPEN].ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
        m_Button[MYSHOPINVENTORY_OPEN].ChangeTextColor(RGBA(255, 255, 255, 255));
        m_Button[MYSHOPINVENTORY_OPEN].UnLock();
        m_Button[MYSHOPINVENTORY_OPEN].ChangeToolTipText(GlobalText[1106], true);
        m_Button[MYSHOPINVENTORY_CLOSE].ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
        m_Button[MYSHOPINVENTORY_CLOSE].ChangeTextColor(RGBA(255, 255, 255, 255));
        m_Button[MYSHOPINVENTORY_CLOSE].UnLock();
    }
    else
    {
        m_Button[MYSHOPINVENTORY_CLOSE].ChangeImgColor(BUTTON_STATE_UP, RGBA(100, 100, 100, 255));
        m_Button[MYSHOPINVENTORY_CLOSE].ChangeTextColor(RGBA(100, 100, 100, 255));
        m_Button[MYSHOPINVENTORY_CLOSE].Lock();
        m_Button[MYSHOPINVENTORY_OPEN].ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
        m_Button[MYSHOPINVENTORY_OPEN].ChangeTextColor(RGBA(255, 255, 255, 255));
        m_Button[MYSHOPINVENTORY_OPEN].UnLock();
        m_Button[MYSHOPINVENTORY_OPEN].ChangeToolTipText(GlobalText[1107], true);
    }
}

void SEASON3B::CNewUIMyShopInventory::OpenButtonLock()
{
    m_Button[MYSHOPINVENTORY_OPEN].ChangeImgColor(BUTTON_STATE_UP, RGBA(100, 100, 100, 255));
    m_Button[MYSHOPINVENTORY_OPEN].ChangeTextColor(RGBA(100, 100, 100, 255));
    m_Button[MYSHOPINVENTORY_OPEN].Lock();
    m_Button[MYSHOPINVENTORY_OPEN].ChangeToolTipText(GlobalText[1107], true);
}

void SEASON3B::CNewUIMyShopInventory::OpenButtonUnLock()
{
    m_Button[MYSHOPINVENTORY_OPEN].ChangeImgColor(BUTTON_STATE_UP, RGBA(255, 255, 255, 255));
    m_Button[MYSHOPINVENTORY_OPEN].ChangeTextColor(RGBA(255, 255, 255, 255));
    m_Button[MYSHOPINVENTORY_OPEN].UnLock();
    m_Button[MYSHOPINVENTORY_OPEN].ChangeToolTipText(GlobalText[1106], true);
}

const bool SEASON3B::CNewUIMyShopInventory::IsEnablePersonalShop() const
{
    return m_EnablePersonalShop;
}

void SEASON3B::CNewUIMyShopInventory::ChangeEditBox(const UISTATES type)
{
    m_EditBox->SetState(type);

    if (type == UISTATE_NORMAL)
    {
        m_EditBox->GiveFocus();
    }

}

bool SEASON3B::CNewUIMyShopInventory::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MYSHOP_INVENTORY) == true)
    {
        if (SEASON3B::IsPress(VK_ESCAPE) == true)
        {
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_MYSHOP_INVENTORY);
            PlayBuffer(SOUND_CLICK01);
            return false;
        }
    }

    return true;
}

bool SEASON3B::CNewUIMyShopInventory::MyShopInventoryProcess()
{
    if (CheckMouseIn(m_Pos.x, m_Pos.y, INVENTORY_WIDTH, INVENTORY_HEIGHT) == false)
    {
        return false;
    }

    CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();

    if (m_pNewInventoryCtrl && pPickedItem && IsRelease(VK_LBUTTON))
    {
        ITEM* pItemObj = pPickedItem->GetItem();
        int iSourceIndex = pPickedItem->GetSourceLinealPos();
        int iTargetIndex = pPickedItem->GetTargetLinealPos(m_pNewInventoryCtrl);

#ifndef KJH_FIX_CHANGE_ITEM_PRICE_IN_PERSONAL_SHOP				// #ifndef
        if (IsPersonalShopBan(pItemObj))
            m_pNewInventoryCtrl->SetSquareColorNormal(1.0f, 0.0f, 0.0f);
        else
            m_pNewInventoryCtrl->SetSquareColorNormal(0.1f, 0.4f, 0.8f);
#endif // KJH_FIX_CHANGE_ITEM_PRICE_IN_PERSONAL_SHOP

        if (iTargetIndex == -1)
        {
            return true;
        }

        if (pPickedItem->GetOwnerInventory() == g_pMyInventory->GetInventoryCtrl())
        {
            if (IsPersonalShopBan(pItemObj) == true)
            {
                g_pSystemLogBox->AddText(GlobalText[2226], SEASON3B::TYPE_ERROR_MESSAGE);
                return true;
            }

            if (m_pNewInventoryCtrl->CanMove(iTargetIndex, pItemObj))
            {
                ChangeSourceIndex(iSourceIndex);
                ChangeTargetIndex(iTargetIndex);

                CreateMessageBox(MSGBOX_LAYOUT_CLASS(CPersonalShopItemValueMsgBoxLayout));
                SetInputValueTextBox(true);

                pPickedItem->HidePickedItem();
                return true;
            }
        }
        else if (pPickedItem->GetOwnerInventory() == NULL)
        {
            if (IsPersonalShopBan(pItemObj) == true)
            {
                g_pSystemLogBox->AddText(GlobalText[2226], SEASON3B::TYPE_ERROR_MESSAGE);
                return true;
            }

            if (m_pNewInventoryCtrl->CanMove(iTargetIndex, pItemObj))
            {
                ChangeSourceIndex(iSourceIndex);
                ChangeTargetIndex(iTargetIndex);

                CreateMessageBox(MSGBOX_LAYOUT_CLASS(CPersonalShopItemValueMsgBoxLayout));
                SetInputValueTextBox(true);

                pPickedItem->HidePickedItem();
                return true;
            }
        }
        else if (pPickedItem->GetOwnerInventory() == m_pNewInventoryCtrl)
        {
            if (m_pNewInventoryCtrl->CanMove(iTargetIndex, pItemObj))
            {
                ChangeSourceIndex(iSourceIndex);
                ChangeTargetIndex(iTargetIndex);
                SendRequestEquipmentItem(STORAGE_TYPE::MYSHOP, iSourceIndex, pItemObj,
                    STORAGE_TYPE::MYSHOP, iTargetIndex);
                return true;
            }
        }
    }
    else if (m_pNewInventoryCtrl && !pPickedItem && IsPress(VK_RBUTTON))
    {
        MouseRButton = false;
        MouseRButtonPop = false;
        MouseRButtonPush = false;

        int iCurSquareIndex = m_pNewInventoryCtrl->GetIndexAtPt(MouseX, MouseY);

        if (iCurSquareIndex != -1)
        {
            ITEM* pItem = g_pMyShopInventory->FindItem(iCurSquareIndex);

            if(pItem)
            {
                ChangeSourceIndex(iCurSquareIndex);
                ChangeTargetIndex(-1);
                CreateMessageBox(MSGBOX_LAYOUT_CLASS(CPersonalShopItemValueMsgBoxLayout));
                SetInputValueTextBox(true);
            }
            return true;
        }
    }

    return false;
}

bool SEASON3B::CNewUIMyShopInventory::UpdateMouseEvent()
{
    if (m_pNewInventoryCtrl && false == m_pNewInventoryCtrl->UpdateMouseEvent())
    {
        return false;
    }

    if (CheckMouseIn(m_Pos.x, m_Pos.y, INVENTORY_WIDTH, INVENTORY_HEIGHT))
    {
        if (MyShopInventoryProcess() == true)
        {
            return false;
        }

        POINT ptExitBtn1 = { m_Pos.x + 169, m_Pos.y + 7 };

        if (SEASON3B::IsPress(VK_LBUTTON) && CheckMouseIn(ptExitBtn1.x, ptExitBtn1.y, 13, 12))
        {
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_MYSHOP_INVENTORY);
            return false;
        }

        if (SEASON3B::IsRelease(VK_LBUTTON)
            && CheckMouseIn(m_EditBox->GetPosition_x(), m_EditBox->GetPosition_y(), m_EditBox->GetWidth(), m_EditBox->GetHeight()))
        {
            ChangeEditBox(UISTATE_NORMAL);
        }

        if (SEASON3B::IsRelease(VK_LBUTTON)
            && CheckMouseIn(m_EditBox->GetPosition_x(), m_EditBox->GetPosition_y(), m_EditBox->GetWidth(), m_EditBox->GetHeight()) == false)
        {
            SetFocus(g_hWnd);
        }
    }

    m_EditBox->DoAction();

    for (int i = 0; i < MYSHOPINVENTORY_MAXBUTTONCOUNT; ++i)
    {
        if (m_Button[i].UpdateMouseEvent())
        {
            switch (i)
            {
            case 0:
            {
                g_pNewUISystem->Hide(SEASON3B::INTERFACE_MYSHOP_INVENTORY);
            }
            return false;
            case 1:
            {
                wchar_t shopTitle[MAX_SHOPTITLE + 1]{};
                g_pMyShopInventory->GetTitle(shopTitle);
                if (IsExistUndecidedPrice() == false && wcslen(shopTitle) > 0)
                {
                    if (m_EnablePersonalShop == false)
                    {
                        SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CPersonalshopCreateMsgBoxLayout));
                    }
                    else
                    {
                        wcscpy(g_szPersonalShopTitle, shopTitle);
                        SocketClient->ToGameServer()->SendPlayerShopOpen(shopTitle);

                        g_pNewUISystem->Hide(SEASON3B::INTERFACE_MYSHOP_INVENTORY);
                        g_pNewUISystem->Hide(SEASON3B::INTERFACE_INVENTORY);
                        g_pNewUISystem->Hide(SEASON3B::INTERFACE_INVENTORY_EXT);
                    }
                }
                else
                {
                    g_pSystemLogBox->AddText(GlobalText[1119], SEASON3B::TYPE_ERROR_MESSAGE);
                }
            }
            return false;
            case 2:
            {
                SocketClient->ToGameServer()->SendPlayerShopClose();

                g_pNewUISystem->Hide(SEASON3B::INTERFACE_MYSHOP_INVENTORY);
                g_pNewUISystem->Hide(SEASON3B::INTERFACE_INVENTORY);
                g_pNewUISystem->Hide(SEASON3B::INTERFACE_INVENTORY_EXT);
            }
            return false;
            }
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

    return true;
}

bool SEASON3B::CNewUIMyShopInventory::Update()
{
    if (m_pNewInventoryCtrl && false == m_pNewInventoryCtrl->Update())
    {
        return false;
    }

    return true;
}

void SEASON3B::CNewUIMyShopInventory::RenderFrame()
{
    RenderImage(IMAGE_MYSHOPINVENTORY_BACK, m_Pos.x, m_Pos.y, INVENTORY_WIDTH, INVENTORY_HEIGHT);
    RenderImage(IMAGE_MYSHOPINVENTORY_TOP, m_Pos.x, m_Pos.y, INVENTORY_WIDTH, 64.f);
    RenderImage(IMAGE_MYSHOPINVENTORY_LEFT, m_Pos.x, m_Pos.y + 64, 21.f, 320.f);
    RenderImage(IMAGE_MYSHOPINVENTORY_RIGHT, m_Pos.x + INVENTORY_WIDTH - 21, m_Pos.y + 64, 21.f, 320.f);
    RenderImage(IMAGE_MYSHOPINVENTORY_BOTTOM, m_Pos.x, m_Pos.y + INVENTORY_HEIGHT - 45, INVENTORY_WIDTH, 45.f);
    RenderImage(IMAGE_MYSHOPINVENTORY_EDIT, m_Pos.x + 12, m_Pos.y + 49, 169.f, 26.f);

    wchar_t Text[100] = {};
    swprintf(Text, GlobalText[1102]);
    RenderText(Text, m_Pos.x, m_Pos.y + 15, INVENTORY_WIDTH, 0, 0xFF49B0FF, 0x00000000, RT3_SORT_CENTER);
}

void SEASON3B::CNewUIMyShopInventory::RenderTextInfo()
{
    wchar_t Text[100];

    if (m_EnablePersonalShop)
    {
        RenderText(GlobalText[1103], m_Pos.x, m_Pos.y + 200, INVENTORY_WIDTH, 0, RGBA(215, 138, 0, 255), 0x00000000, RT3_SORT_CENTER, g_hFontBold);
    }

    memset(&Text, 0, sizeof(wchar_t) * 100);
    swprintf(Text, GlobalText[370]);
    RenderText(Text, m_Pos.x + 30, m_Pos.y + 230, 0, 0, RGBA(255, 45, 47, 255), 0x00000000, RT3_SORT_LEFT, g_hFontBold);

    memset(&Text, 0, sizeof(wchar_t) * 100);
    swprintf(Text, GlobalText[1109]);
    RenderText(Text, m_Pos.x + 30, m_Pos.y + 250, 0, 0, RGBA(247, 206, 77, 255), 0x00000000, RT3_SORT_LEFT);

    memset(&Text, 0, sizeof(wchar_t) * 100);
    swprintf(Text, GlobalText[1111]);
    RenderText(Text, m_Pos.x + 30, m_Pos.y + 262, 0, 0, RGBA(247, 206, 77, 255), 0x00000000, RT3_SORT_LEFT);

    memset(&Text, 0, sizeof(wchar_t) * 100);
    swprintf(Text, GlobalText[1112]);
    RenderText(Text, m_Pos.x + 30, m_Pos.y + 274, 0, 0, RGBA(247, 206, 77, 255), 0x00000000, RT3_SORT_LEFT);

    memset(&Text, 0, sizeof(wchar_t) * 100);
    swprintf(Text, GlobalText[1113]);
    RenderText(Text, m_Pos.x + 30, m_Pos.y + 286, 0, 0, RGBA(247, 206, 77, 255), 0x00000000, RT3_SORT_LEFT);

    memset(&Text, 0, sizeof(wchar_t) * 100);
    swprintf(Text, GlobalText[1115]);
    RenderText(Text, m_Pos.x + 30, m_Pos.y + 298, 0, 0, RGBA(247, 206, 77, 255), 0x00000000, RT3_SORT_LEFT);

    memset(&Text, 0, sizeof(wchar_t) * 100);
    swprintf(Text, GlobalText[1134]);
    RenderText(Text, m_Pos.x + 30, m_Pos.y + 320, 0, 0, RGBA(255, 45, 47, 255), 0x00000000, RT3_SORT_LEFT, g_hFontBold);

    memset(&Text, 0, sizeof(wchar_t) * 100);
    swprintf(Text, GlobalText[1135]);
    RenderText(Text, m_Pos.x + 30, m_Pos.y + 332, 0, 0, RGBA(255, 45, 47, 255), 0x00000000, RT3_SORT_LEFT, g_hFontBold);
}

bool SEASON3B::CNewUIMyShopInventory::Render()
{
    EnableAlphaTest();
    glColor4f(1.f, 1.f, 1.f, 1.f);

    RenderFrame();

    RenderTextInfo();

    if (m_EditBox)
    {
        m_EditBox->Render();
    }

    if (m_pNewInventoryCtrl)
    {
        m_pNewInventoryCtrl->Render();
    }

    for (int i = 0; i < MYSHOPINVENTORY_MAXBUTTONCOUNT; ++i)
    {
        m_Button[i].Render();
    }

    DisableAlphaBlend();

    return true;
}

void SEASON3B::CNewUIMyShopInventory::ClosingProcess()
{
    CNewUIInventoryCtrl::BackupPickedItem();
    g_pMyInventory->ChangeMyShopButtonStateOpen();
    SetFocus(g_hWnd);
}

int SEASON3B::CNewUIMyShopInventory::GetPointedItemIndex()
{
    return m_pNewInventoryCtrl->GetPointedSquareIndex();
}

int SEASON3B::CNewUIMyShopInventory::GetItemInventoryIndex(ITEM* pItem)
{
    return m_pNewInventoryCtrl->GetIndexByItem(pItem);
}

void SEASON3B::CNewUIMyShopInventory::ResetSubject()
{
    if (m_EditBox)
    {
        m_EditBox->SetText(NULL);
    }
}

bool SEASON3B::CNewUIMyShopInventory::IsEnableInputValueTextBox()
{
    return m_bIsEnableInputValueTextBox;
}

void SEASON3B::CNewUIMyShopInventory::SetInputValueTextBox(bool bIsEnable)
{
    m_bIsEnableInputValueTextBox = bIsEnable;
}