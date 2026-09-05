// NewUINPCShop.cpp: implementation of the CNPCShop class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UI/NPCs/NewUINPCShop.h"
#include "I18N/All.h"

#include "Audio/DSPlaySound.h"
#include "UI/Core/NewUISystem.h"
#include "UI/Dialogs/NewUICommonMessageBox.h"
#include "Engine/Object/ZzzInventory.h"

#include "GameLogic/Social/GambleSystem.h"

using namespace SEASON3B;
using namespace mu::ui::window;

extern int BuyCost;

mu::ui::window::CNPCShop::CNPCShop()
{
    Init();
}

mu::ui::window::CNPCShop::~CNPCShop()
{
    Release();
}

void mu::ui::window::CNPCShop::Init()
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

bool mu::ui::window::CNPCShop::Create(CManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng || NULL == g_pNewItemMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_NPCSHOP, this);

    m_pNewInventoryCtrl = new CInventoryCtrl;
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

void mu::ui::window::CNPCShop::Release()
{
    UnloadImages();

    SAFE_DELETE(m_pNewInventoryCtrl);

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }
}

void mu::ui::window::CNPCShop::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;

    m_pNewInventoryCtrl->SetPos(x + 15, y + 50);
}

bool mu::ui::window::CNPCShop::UpdateMouseEvent()
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

            if ((m_bIsNPCShopOpen == true) && (pItem) && (mu::ui::window::IsRelease(VK_LBUTTON)))
            {
                int iIndex = (pItem->y * m_pNewInventoryCtrl->GetNumberOfColumn()) + pItem->x;
                GambleSystem& _gambleSys = GambleSystem::Instance();

                if (_gambleSys.IsGambleShop())
                {
                    _gambleSys.SetBuyItemInfo(iIndex, ItemValue(pItem, 0));
                    g_pNPCShop->SetStandbyItemKey(pItem->Key);

                    mu::ui::window::CreateMessageBox(MSGBOX_LAYOUT_CLASS(mu::ui::window::CGambleBuyMsgBoxLayout));

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
            if (mu::ui::window::IsRelease(VK_LBUTTON))
            {
                m_bIsNPCShopOpen = true;
                return false;
            }
            if (mu::ui::window::IsPress(VK_LBUTTON))
            {
                return false;
            }
        }
    }

    if (BtnProcess() == true)
    {
        return false;
    }

    if (WindowProcess())
        return false;

    return true;
}

bool mu::ui::window::CNPCShop::WindowProcess()
{
    return CheckMouseIn(m_Pos.x, m_Pos.y, NPCSHOP_WIDTH, NPCSHOP_HEIGHT);
}

bool mu::ui::window::CNPCShop::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_NPCSHOP) == false)
    {
        return true;
    }

    if (mu::ui::window::IsRepeat(VK_SHIFT) && mu::ui::window::IsPress('L'))
    {
        SocketClient->ToGameServer()->SendRepairItemRequest(0xFF, 0);
        return false;
    }
    if (mu::ui::window::IsPress('L'))
    {
        if (m_bRepairShop && CInventoryCtrl::GetPickedItem() == NULL)
        {
            ToggleState();
            return false;
        }
    }

    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_NPCSHOP) == true)
    {
        if (mu::ui::window::IsPress(VK_ESCAPE) == true && m_bSellingItem == false)
        {
            g_pNewUISystem->Hide(mu::ui::window::INTERFACE_NPCSHOP);
            PlayBuffer(SOUND_CLICK01);
            return false;
        }
    }
    return true;
}

bool mu::ui::window::CNPCShop::Update()
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

bool mu::ui::window::CNPCShop::Render()
{
    EnableAlphaTest();
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

void mu::ui::window::CNPCShop::RenderFrame()
{
    RenderImage(IMAGE_NPCSHOP_BACK, m_Pos.x, m_Pos.y, 190.f, 429.f);
    RenderImage(IMAGE_NPCSHOP_TOP, m_Pos.x, m_Pos.y, 190.f, 64.f);
    RenderImage(IMAGE_NPCSHOP_LEFT, m_Pos.x, m_Pos.y + 64, 21.f, 320.f);
    RenderImage(IMAGE_NPCSHOP_RIGHT, m_Pos.x + 190 - 21, m_Pos.y + 64, 21.f, 320.f);
    RenderImage(IMAGE_NPCSHOP_BOTTOM, m_Pos.x, m_Pos.y + 429 - 45, 190.f, 45.f);
}

void mu::ui::window::CNPCShop::RenderTexts()
{
    g_pRenderText->SetFont(g_hFontBold);
    g_pRenderText->SetBgColor(0);
    g_pRenderText->SetTextColor(220, 220, 220, 255);

    g_pRenderText->RenderText(m_Pos.x, m_Pos.y + 12, I18N::Game::Merchant, NPCSHOP_WIDTH, 0, RT3_SORT_CENTER);

    wchar_t strText[256];
    mu_swprintf(strText, I18N::Game::TaxRateDChangedInRealTime, m_iTaxRate);
    g_pRenderText->RenderText(m_Pos.x, m_Pos.y + 27, strText, NPCSHOP_WIDTH, 0, RT3_SORT_CENTER);
}

void mu::ui::window::CNPCShop::RenderButton()
{
    if (m_bRepairShop)
    {
        m_BtnRepair.Render();
        m_BtnRepairAll.Render();
    }
}

void mu::ui::window::CNPCShop::RenderRepairMoney()
{
    if (m_bRepairShop)
    {
        RenderImage(IMAGE_NPCSHOP_REPAIR_MONEY, m_Pos.x + 10, m_Pos.y + 355, 170.f, 24.f);
        g_pRenderText->SetBgColor(255, 255, 255, 0);
        g_pRenderText->SetTextColor(255, 220, 150, 255);
        wchar_t strText[256];
        ConvertGold(AllRepairGold, strText);
        g_pRenderText->SetFont(g_hFontBold);
        g_pRenderText->RenderText(m_Pos.x + 20, m_Pos.y + 362, I18N::Game::RepairAll);
        g_pRenderText->SetTextColor(getGoldColor(AllRepairGold));
        g_pRenderText->RenderText(m_Pos.x + 100, m_Pos.y + 362, strText);
    }
}

float mu::ui::window::CNPCShop::GetLayerDepth()
{
    return 4.55;
}

void mu::ui::window::CNPCShop::LoadImages()
{
    LoadBitmap(L"Interface\\newui_msgbox_back.jpg", IMAGE_NPCSHOP_BACK, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back04.tga", IMAGE_NPCSHOP_TOP, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back02-L.tga", IMAGE_NPCSHOP_LEFT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back02-R.tga", IMAGE_NPCSHOP_RIGHT, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_back03.tga", IMAGE_NPCSHOP_BOTTOM, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_repair_00.tga", IMAGE_NPCSHOP_BTN_REPAIR, GL_LINEAR);
    LoadBitmap(L"Interface\\newui_item_money2.tga", IMAGE_NPCSHOP_REPAIR_MONEY, GL_LINEAR);
}

void mu::ui::window::CNPCShop::UnloadImages()
{
    DeleteBitmap(IMAGE_NPCSHOP_BACK);
    DeleteBitmap(IMAGE_NPCSHOP_TOP);
    DeleteBitmap(IMAGE_NPCSHOP_LEFT);
    DeleteBitmap(IMAGE_NPCSHOP_LEFT);
    DeleteBitmap(IMAGE_NPCSHOP_BOTTOM);
    DeleteBitmap(IMAGE_NPCSHOP_BTN_REPAIR);
    DeleteBitmap(IMAGE_NPCSHOP_REPAIR_MONEY);
}

void mu::ui::window::CNPCShop::SetTaxRate(int iTaxRate)
{
    m_iTaxRate = iTaxRate;
}

int mu::ui::window::CNPCShop::GetTaxRate()
{
    return m_iTaxRate;
}

bool mu::ui::window::CNPCShop::InsertItem(int iIndex, std::span<const BYTE> pbyItemPacket)
{
    if (m_pNewInventoryCtrl)
    {
        return m_pNewInventoryCtrl->AddItem(iIndex, pbyItemPacket);
    }

    return false;
}

bool mu::ui::window::CNPCShop::InventoryProcess()
{
    CPickedItem* pPickedItem = CInventoryCtrl::GetPickedItem();

    if (!m_pNewInventoryCtrl)	return false;
    if (!pPickedItem)			return false;
    ITEM* pItem = pPickedItem->GetItem();

    if (IsSellingBan(pItem))	m_pNewInventoryCtrl->SetSquareColorNormal(1.0f, 0.0f, 0.0f);
    else	m_pNewInventoryCtrl->SetSquareColorNormal(0.1f, 0.4f, 0.8f);

    if (mu::ui::window::IsRelease(VK_LBUTTON) == true && m_pNewInventoryCtrl->CheckPtInRect(MouseX, MouseY) == true && m_bSellingItem == false)
    {
        if (CharacterMachine->Gold + ItemValue(pItem) > 2000000000)
        {
            g_pSystemLogBox->AddText(I18N::Game::ExceededMaximumAmountOfZenYouCanPossess, mu::ui::window::TYPE_SYSTEM_MESSAGE);

            return true;
        }

        if (pItem && pItem->Jewel_Of_Harmony_Option != 0)
        {
            g_pSystemLogBox->AddText(I18N::Game::ReinforcedItemCanTBeSold, mu::ui::window::TYPE_ERROR_MESSAGE);

            return true;
        }
        if (pItem && IsSellingBan(pItem) == true)
        {
            g_pSystemLogBox->AddText(I18N::Game::TheseItemsCannotBeTraded, mu::ui::window::TYPE_ERROR_MESSAGE);
            m_pNewInventoryCtrl->BackupPickedItem();

            return true;
        }
        if (pItem && IsHighValueItem(pItem) == true)
        {
            mu::ui::window::CreateMessageBox(MSGBOX_LAYOUT_CLASS(mu::ui::window::CHighValueItemCheckMsgBoxLayout));
            pPickedItem->HidePickedItem();

            return true;
        }

        if (pPickedItem->GetSourceStorageType() == STORAGE_TYPE::INVENTORY)
        {
            const int iSourceIndex = pPickedItem->GetSourceLinealPos();
            if (iSourceIndex >= MAX_EQUIPMENT_INDEX && iSourceIndex < MAX_MY_INVENTORY_EX_INDEX)
            {
                SocketClient->ToGameServer()->SendSellItemToNpcRequest(iSourceIndex);
                g_pNPCShop->SetSellingItem(true);
                return true;
            }
        }
    }

    return false;
}

bool mu::ui::window::CNPCShop::BtnProcess()
{
    // Top-right corner close "X" (shared frame): hides + swallows the click.
    if (m_bSellingItem == false && g_pNewUISystem->HandleFrameCornerClose(m_Pos, mu::ui::window::INTERFACE_NPCSHOP))
    {
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

void mu::ui::window::CNPCShop::DeleteAllItems()
{
    if (m_pNewInventoryCtrl)
        m_pNewInventoryCtrl->RemoveAllItems();
}

void mu::ui::window::CNPCShop::OpenningProcess()
{
    if (mu::ui::window::IsRepeat(VK_LBUTTON))
    {
        m_bIsNPCShopOpen = false;
    }
    else
    {
        m_bIsNPCShopOpen = true;
    }
}

void mu::ui::window::CNPCShop::ClosingProcess()
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

void mu::ui::window::CNPCShop::SetButtonInfo()
{
    m_BtnRepair.ChangeButtonImgState(true, IMAGE_NPCSHOP_BTN_REPAIR, false);
    m_BtnRepair.ChangeButtonInfo(m_Pos.x + 54, m_Pos.y + 390, 36, 29);
    m_BtnRepair.ChangeToolTipText(&I18N::Game::RepairL, true);

    m_BtnRepairAll.ChangeButtonImgState(true, IMAGE_NPCSHOP_BTN_REPAIR, false);
    m_BtnRepairAll.ChangeButtonInfo(m_Pos.x + 98, m_Pos.y + 390, 36, 29);
    m_BtnRepairAll.ChangeToolTipText(&I18N::Game::RepairAllA, true);
}

void mu::ui::window::CNPCShop::SetRepairShop(bool bRepair)
{
    m_bRepairShop = bRepair;
}

bool mu::ui::window::CNPCShop::IsRepairShop()
{
    return m_bRepairShop;
}

void mu::ui::window::CNPCShop::ToggleState()
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

DWORD mu::ui::window::CNPCShop::GetShopState()
{
    return m_dwShopState;
}

int mu::ui::window::CNPCShop::GetPointedItemIndex()
{
    return m_pNewInventoryCtrl->GetPointedSquareIndex();
}

void mu::ui::window::CNPCShop::SetStandbyItemKey(DWORD dwItemKey)
{
    m_dwStandbyItemKey = dwItemKey;
}

DWORD mu::ui::window::CNPCShop::GetStandbyItemKey() const
{
    return m_dwStandbyItemKey;
}

int mu::ui::window::CNPCShop::GetStandbyItemIndex()
{
    ITEM* pItem = GetStandbyItem();
    if (pItem)
        return pItem->y * m_pNewInventoryCtrl->GetNumberOfColumn() + pItem->x;
    return -1;
}

ITEM* mu::ui::window::CNPCShop::GetStandbyItem()
{
    if (m_pNewInventoryCtrl)
        return m_pNewInventoryCtrl->FindItemByKey(m_dwStandbyItemKey);
    return NULL;
}

void mu::ui::window::CNPCShop::SetSellingItem(bool bFlag)
{
    m_bSellingItem = bFlag;
}

bool mu::ui::window::CNPCShop::IsSellingItem()
{
    return m_bSellingItem;
}
