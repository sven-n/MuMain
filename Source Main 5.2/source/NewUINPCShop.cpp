// NewUINPCShop.cpp: implementation of the CNewUINPCShop class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUINPCShop.h"

#include "DSPlaySound.h"
#include "NewUISystem.h"
#include "NewUICommonMessageBox.h"
#include "ZzzInventory.h"

#include "GambleSystem.h"

using namespace SEASON3B;

extern int BuyCost;

SEASON3B::CNewUINPCShop::CNewUINPCShop()
{
    Init();
}

SEASON3B::CNewUINPCShop::~CNewUINPCShop()
{
    Release();
}

void SEASON3B::CNewUINPCShop::Init()
{
    m_pNewUIMng = NULL;
    m_pNewInventoryCtrl = NULL;
    m_Pos.x = m_Pos.y = 0;
    m_dwShopState = SHOP_STATE_BUYNSELL;
    m_iTaxRate = 0;
    m_bRepairShop = false;
    m_bIsNPCShopOpen = false;
    m_dwStandbyItemKey = 0;
    m_bSellingItem = false;
}

bool SEASON3B::CNewUINPCShop::Create(CNewUIManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng || NULL == g_pNewItemMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(SEASON3B::INTERFACE_NPCSHOP, this);

    m_pNewInventoryCtrl = new CNewUIInventoryCtrl;
    if (false == m_pNewInventoryCtrl->Create(STORAGE_TYPE::UNDEFINED, g_pNewUI3DRenderMng, g_pNewItemMng, this, x + 15, y + 50, 8, 15))
    {
        SAFE_DELETE(m_pNewInventoryCtrl);
        return false;
    }

    if (m_pNewInventoryCtrl)
    {
        m_pNewInventoryCtrl->SetToolTipType(TOOLTIP_TYPE_NPC_SHOP);
    }

    SetPos(x, y);

    LoadImages();

    SetButtonInfo();

    Show(false);

    return true;
}

void SEASON3B::CNewUINPCShop::Release()
{
    UnloadImages();

    SAFE_DELETE(m_pNewInventoryCtrl);

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void SEASON3B::CNewUINPCShop::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;

    m_pNewInventoryCtrl->SetPos(x + 15, y + 50);
}

bool SEASON3B::CNewUINPCShop::UpdateMouseEvent()
{
    if (m_pNewInventoryCtrl)
    {
        if (false == m_pNewInventoryCtrl->UpdateMouseEvent())
        {
            return false;
        }

        if (InventoryProcess() == true)
        {
            return false;
        }

        if (m_pNewInventoryCtrl->CheckPtInRect(MouseX, MouseY) == true)
        {
            ITEM* pItem = m_pNewInventoryCtrl->FindItemAtPt(MouseX, MouseY);

            if ((m_bIsNPCShopOpen == true) && (pItem) && (SEASON3B::IsRelease(VK_LBUTTON)))
            {
                int iIndex = (pItem->y * m_pNewInventoryCtrl->GetNumberOfColumn()) + pItem->x;
                GambleSystem& _gambleSys = GambleSystem::Instance();

                if (_gambleSys.IsGambleShop())
                {
                    _gambleSys.SetBuyItemInfo(iIndex, ItemValue(pItem, 0));
                    g_pNPCShop->SetStandbyItemKey(pItem->Key);

                    SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CGambleBuyMsgBoxLayout));

                    return false;
                }
                if (BuyCost == 0)
                {
                    SocketClient->ToGameServer()->SendBuyItemFromNpcRequest(iIndex);
                    BuyCost = ItemValue(pItem, 0);
                    g_ConsoleDebug->Write(MCD_SEND, L"0x32 [SendRequestBuy(%d)]", iIndex);
                }

                return false;
            }
            if (SEASON3B::IsRelease(VK_LBUTTON))
            {
                m_bIsNPCShopOpen = true;
                return false;
            }
            if (SEASON3B::IsPress(VK_LBUTTON))
            {
                return false;
            }
        }
    }

    if (BtnProcess() == true)
    {
        return false;
    }

    if (CheckMouseIn(m_Pos.x, m_Pos.y, NPCSHOP_WIDTH, NPCSHOP_HEIGHT))
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

bool SEASON3B::CNewUINPCShop::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPCSHOP) == false)
    {
        return true;
    }

    if (SEASON3B::IsRepeat(VK_SHIFT) && SEASON3B::IsPress('L'))
    {
        SocketClient->ToGameServer()->SendRepairItemRequest(0xFF, 0);
        return false;
    }
    if (SEASON3B::IsPress('L'))
    {
        if (m_bRepairShop && CNewUIInventoryCtrl::GetPickedItem() == NULL)
        {
            ToggleState();
            return false;
        }
    }

    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_NPCSHOP) == true)
    {
        if (SEASON3B::IsPress(VK_ESCAPE) == true && m_bSellingItem == false)
        {
            g_pNewUISystem->Hide(SEASON3B::INTERFACE_NPCSHOP);
            PlayBuffer(SOUND_CLICK01);
            return false;
        }
    }
    return true;
}

bool SEASON3B::CNewUINPCShop::Update()
{
    if (m_bRepairShop)
    {
        RepairAllGold();
    }
    if (m_pNewInventoryCtrl && false == m_pNewInventoryCtrl->Update())
    {
        return false;
    }
    return true;
}

bool SEASON3B::CNewUINPCShop::Render()
{
    EnableAlphaTest();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    RenderFrame();
    RenderTexts();
    RenderButton();
    RenderRepairMoney();

    if (m_pNewInventoryCtrl)
    {
        m_pNewInventoryCtrl->Render();
    }

    DisableAlphaBlend();
    return true;
}

void SEASON3B::CNewUINPCShop::RenderFrame()
{
    RenderImage(IMAGE_NPCSHOP_BACK, m_Pos.x, m_Pos.y, 190.f, 429.f);
    RenderImage(IMAGE_NPCSHOP_TOP, m_Pos.x, m_Pos.y, 190.f, 64.f);
    RenderImage(IMAGE_NPCSHOP_LEFT, m_Pos.x, m_Pos.y + 64, 21.f, 320.f);
    RenderImage(IMAGE_NPCSHOP_RIGHT, m_Pos.x + 190 - 21, m_Pos.y + 64, 21.f, 320.f);
    RenderImage(IMAGE_NPCSHOP_BOTTOM, m_Pos.x, m_Pos.y + 429 - 45, 190.f, 45.f);
}

void SEASON3B::CNewUINPCShop::RenderTexts()
{
    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetBgColor(0);
    g_pRenderText->SetTextColor(220, 220, 220, 255);

    g_pRenderText->RenderText(m_Pos.x, m_Pos.y + 12, GlobalText[230], NPCSHOP_WIDTH, 0, RT3_SORT_CENTER);

    wchar_t strText[256];
    swprintf(strText, GlobalText[1623], m_iTaxRate);
    g_pRenderText->RenderText(m_Pos.x, m_Pos.y + 27, strText, NPCSHOP_WIDTH, 0, RT3_SORT_CENTER);
}

void SEASON3B::CNewUINPCShop::RenderButton()
{
    if (m_bRepairShop)
    {
        m_BtnRepair.Render();
        m_BtnRepairAll.Render();
    }
}

void SEASON3B::CNewUINPCShop::RenderRepairMoney()
{
    if (m_bRepairShop)
    {
        RenderImage(IMAGE_NPCSHOP_REPAIR_MONEY, m_Pos.x + 10, m_Pos.y + 355, 170.f, 24.f);
        g_pRenderText->SetBgColor(255, 255, 255, 0);
        g_pRenderText->SetTextColor(255, 220, 150, 255);
        wchar_t strText[256];
        ConvertGold(AllRepairGold, strText);
        g_pRenderText->SetFont(g_hFontBold);
        g_pRenderText->RenderText(m_Pos.x + 20, m_Pos.y + 362, GlobalText[239]);
        g_pRenderText->SetTextColor(getGoldColor(AllRepairGold));
        g_pRenderText->RenderText(m_Pos.x + 100, m_Pos.y + 362, strText);
    }
}

float SEASON3B::CNewUINPCShop::GetLayerDepth()
{
    return 4.55;
}

void SEASON3B::CNewUINPCShop::LoadImages()
{
    LoadBitmap(L"Interface\\newui_msgbox_back.jpg", IMAGE_NPCSHOP_BACK, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back04.tga", IMAGE_NPCSHOP_TOP, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back02-L.tga", IMAGE_NPCSHOP_LEFT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back02-R.tga", IMAGE_NPCSHOP_RIGHT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back03.tga", IMAGE_NPCSHOP_BOTTOM, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_repair_00.tga", IMAGE_NPCSHOP_BTN_REPAIR, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_money2.tga", IMAGE_NPCSHOP_REPAIR_MONEY, GL_LINEAR);
}

void SEASON3B::CNewUINPCShop::UnloadImages()
{
    DeleteBitmap(IMAGE_NPCSHOP_BACK);
    DeleteBitmap(IMAGE_NPCSHOP_TOP);
    DeleteBitmap(IMAGE_NPCSHOP_LEFT);
    DeleteBitmap(IMAGE_NPCSHOP_LEFT);
    DeleteBitmap(IMAGE_NPCSHOP_BOTTOM);
    DeleteBitmap(IMAGE_NPCSHOP_BTN_REPAIR);
    DeleteBitmap(IMAGE_NPCSHOP_REPAIR_MONEY);
}

void SEASON3B::CNewUINPCShop::SetTaxRate(int iTaxRate)
{
    m_iTaxRate = iTaxRate;
}

int SEASON3B::CNewUINPCShop::GetTaxRate()
{
    return m_iTaxRate;
}

bool SEASON3B::CNewUINPCShop::InsertItem(int iIndex, std::span<const BYTE> pbyItemPacket)
{
    if (m_pNewInventoryCtrl)
    {
        return m_pNewInventoryCtrl->AddItem(iIndex, pbyItemPacket);
    }

    return false;
}

bool SEASON3B::CNewUINPCShop::InventoryProcess()
{
    CNewUIPickedItem* pPickedItem = CNewUIInventoryCtrl::GetPickedItem();

    if (!m_pNewInventoryCtrl)	return false;
    if (!pPickedItem)			return false;
    ITEM* pItem = pPickedItem->GetItem();

    if (IsSellingBan(pItem))	m_pNewInventoryCtrl->SetSquareColorNormal(1.0f, 0.0f, 0.0f);
    else	m_pNewInventoryCtrl->SetSquareColorNormal(0.1f, 0.4f, 0.8f);

    if (SEASON3B::IsRelease(VK_LBUTTON) == true && m_pNewInventoryCtrl->CheckPtInRect(MouseX, MouseY) == true && m_bSellingItem == false)
    {
        if (CharacterMachine->Gold + ItemValue(pItem) > 2000000000)
        {
            g_pSystemLogBox->AddText(GlobalText[3148], SEASON3B::TYPE_SYSTEM_MESSAGE);

            return true;
        }

        if (pItem && pItem->Jewel_Of_Harmony_Option != 0)
        {
            g_pSystemLogBox->AddText(GlobalText[2211], SEASON3B::TYPE_ERROR_MESSAGE);

            return true;
        }
        if (pItem && IsSellingBan(pItem) == true)
        {
            g_pSystemLogBox->AddText(GlobalText[668], SEASON3B::TYPE_ERROR_MESSAGE);
            m_pNewInventoryCtrl->BackupPickedItem();

            return true;
        }
        if (pItem && IsHighValueItem(pItem) == true)
        {
            SEASON3B::CreateMessageBox(MSGBOX_LAYOUT_CLASS(SEASON3B::CHighValueItemCheckMsgBoxLayout));
            pPickedItem->HidePickedItem();

            return true;
        }

        if (pPickedItem->GetSourceStorageType() == STORAGE_TYPE::INVENTORY)
        {
            int iSourceIndex = pPickedItem->GetSourceLinealPos();
            SocketClient->ToGameServer()->SendSellItemToNpcRequest(iSourceIndex);
            g_pNPCShop->SetSellingItem(true);

            return true;
        }
    }

    return false;
}

bool SEASON3B::CNewUINPCShop::BtnProcess()
{
    POINT ptExitBtn1 = { m_Pos.x + 169, m_Pos.y + 7 };

    if (SEASON3B::IsPress(VK_LBUTTON) && CheckMouseIn(ptExitBtn1.x, ptExitBtn1.y, 13, 12) && m_bSellingItem == false)
    {
        g_pNewUISystem->Hide(SEASON3B::INTERFACE_NPCSHOP);

        return true;
    }

    if (m_bRepairShop)
    {
        if (m_BtnRepair.UpdateMouseEvent() == true)
        {
            ToggleState();

            return true;
        }
        if (m_BtnRepairAll.UpdateMouseEvent() == true)
        {
            SocketClient->ToGameServer()->SendRepairItemRequest(0xFF, 0);

            return true;
        }
    }

    return false;
}

void SEASON3B::CNewUINPCShop::DeleteAllItems()
{
    if (m_pNewInventoryCtrl)
        m_pNewInventoryCtrl->RemoveAllItems();
}

void SEASON3B::CNewUINPCShop::OpenningProcess()
{
    if (SEASON3B::IsRepeat(VK_LBUTTON))
    {
        m_bIsNPCShopOpen = false;
    }
    else
    {
        m_bIsNPCShopOpen = true;
    }
}

void SEASON3B::CNewUINPCShop::ClosingProcess()
{
    SocketClient->ToGameServer()->SendCloseNpcRequest();

    m_dwShopState = SHOP_STATE_BUYNSELL;
    m_iTaxRate = 0;
    m_bRepairShop = false;
    m_dwStandbyItemKey = 0;

    m_bIsNPCShopOpen = false;

    if (m_pNewInventoryCtrl)
    {
        m_pNewInventoryCtrl->RemoveAllItems();
    }

    GambleSystem::Instance().SetGambleShop(false);
    m_bSellingItem = false;
}

void SEASON3B::CNewUINPCShop::SetButtonInfo()
{
    m_BtnRepair.ChangeButtonImgState(true, IMAGE_NPCSHOP_BTN_REPAIR, false);
    m_BtnRepair.ChangeButtonInfo(m_Pos.x + 54, m_Pos.y + 390, 36, 29);
    m_BtnRepair.ChangeToolTipText(GlobalText[233], true);

    m_BtnRepairAll.ChangeButtonImgState(true, IMAGE_NPCSHOP_BTN_REPAIR, false);
    m_BtnRepairAll.ChangeButtonInfo(m_Pos.x + 98, m_Pos.y + 390, 36, 29);
    m_BtnRepairAll.ChangeToolTipText(GlobalText[237], true);
}

void SEASON3B::CNewUINPCShop::SetRepairShop(bool bRepair)
{
    m_bRepairShop = bRepair;
}

bool SEASON3B::CNewUINPCShop::IsRepairShop()
{
    return m_bRepairShop;
}

void SEASON3B::CNewUINPCShop::ToggleState()
{
    if (m_dwShopState == SHOP_STATE_BUYNSELL)
    {
        m_dwShopState = SHOP_STATE_REPAIR;

        g_pMyInventory->SetRepairMode(true);
    }
    else
    {
        m_dwShopState = SHOP_STATE_BUYNSELL;
        g_pMyInventory->SetRepairMode(false);
    }
}

DWORD SEASON3B::CNewUINPCShop::GetShopState()
{
    return m_dwShopState;
}

int SEASON3B::CNewUINPCShop::GetPointedItemIndex()
{
    return m_pNewInventoryCtrl->GetPointedSquareIndex();
}

void SEASON3B::CNewUINPCShop::SetStandbyItemKey(DWORD dwItemKey)
{
    m_dwStandbyItemKey = dwItemKey;
}

DWORD SEASON3B::CNewUINPCShop::GetStandbyItemKey() const
{
    return m_dwStandbyItemKey;
}

int SEASON3B::CNewUINPCShop::GetStandbyItemIndex()
{
    ITEM* pItem = GetStandbyItem();
    if (pItem)
        return pItem->y * m_pNewInventoryCtrl->GetNumberOfColumn() + pItem->x;
    return -1;
}

ITEM* SEASON3B::CNewUINPCShop::GetStandbyItem()
{
    if (m_pNewInventoryCtrl)
        return m_pNewInventoryCtrl->FindItemByKey(m_dwStandbyItemKey);
    return NULL;
}

void SEASON3B::CNewUINPCShop::SetSellingItem(bool bFlag)
{
    m_bSellingItem = bFlag;
}

bool SEASON3B::CNewUINPCShop::IsSellingItem()
{
    return m_bSellingItem;
}