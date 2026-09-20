
#include "stdafx.h"
#include "UI/NPCs/NPCShop.h"
#include "I18N/All.h"

#include "Audio/DSPlaySound.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Core/WindowGeometry.h"
#include "UI/Dialogs/CommonMessageBox.h"
#include "UI/Dialogs/GenericConfirmDialog.h"
#include "Engine/Object/ZzzInventory.h"

#include "GameLogic/Social/GambleSystem.h"

// RmlUi migration -- see this class's header comment.
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/RmlBridge/RmlRootTransform.h"
#include "UI/Scaling/UITransform.h"
#include "Core/Utilities/StringUtils.h"
#include <RmlUi/Core/ElementDocument.h>

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

    // Guarded so the document/model are created once, even if Create() re-runs on resolution change.
    if (!m_pRmlDoc && RmlUiRuntime::Instance().IsCreated())
    {
        BuildRmlUi();
    }

    Show(false);

    return true;
}

void mu::ui::window::CNPCShop::BuildRmlUi()
{
    const bool modelCreated = m_RmlBinder.Create(RmlUiRuntime::Instance().GetContext(), "npc_shop",
            [this](Rml::DataModelConstructor& c, NPCShopRmlModel& model)
            {
                c.Bind("root_x", &model.rootX);
                c.Bind("root_y", &model.rootY);
                c.Bind("root_scale", &model.rootScale);

                c.Bind("title", &model.title);
                c.Bind("tax_rate_text", &model.taxRateText);

                c.Bind("repair_visible", &model.repairVisible);
                c.Bind("repair_tooltip", &model.repairTooltip);
                c.Bind("repair_all_tooltip", &model.repairAllTooltip);
                c.Bind("repair_all_label", &model.repairAllLabel);
                c.Bind("repair_gold_text", &model.repairGoldText);
                c.Bind("repair_gold_color", &model.repairGoldColor);

                c.BindEventCallback("npc_shop_repair_click",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { ToggleState(); });
                c.BindEventCallback("npc_shop_repair_all_click",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                    {
                        SocketClient->ToGameServer()->SendRepairItemRequest(0xFF, 0);
                    });
            });

        if (modelCreated)
            m_pRmlDoc = UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(), "Data/Interface/RmlUi/npc_shop.rml");

        // Frame background panel uses the background context -- see NPCShopBgRmlModel (NPCShop.h).
        if (Rml::Context* bgContext = RmlUiRuntime::Instance().GetBackgroundContext())
        {
            const bool bgModelCreated = m_BgRmlBinder.Create(bgContext, "npc_shop_bg",
                [](Rml::DataModelConstructor& c, NPCShopBgRmlModel& model)
                {
                    c.Bind("root_x", &model.rootX);
                    c.Bind("root_y", &model.rootY);
                    c.Bind("root_scale", &model.rootScale);
                });
            if (bgModelCreated)
            {
                // Starts hidden -- CreateBackgroundDocument() no longer Show()s eagerly (see its
                // own comment, RmlTheme.h); SyncRmlModel() below is what shows/hides it.
                m_pRmlBgDoc = UI::RmlBridge::CreateBackgroundDocument("Data/Interface/RmlUi/npc_shop_bg.rml");
            }
        }

    // Not Show()n here -- m_pRmlDoc's visibility follows this window's own Show()/Hide() via
    // SyncRmlModel(), not an eager Show() at Create() time.
}

void mu::ui::window::CNPCShop::ReloadRmlTheme()
{
    if (!m_pRmlDoc) return; // never opened -- BuildRmlUi() will simply pick up the new theme whenever it first is

    Rml::Context* context = RmlUiRuntime::Instance().GetContext();
    m_RmlBinder.Destroy(context);
    context->UnloadDocument(m_pRmlDoc);
    m_pRmlDoc = nullptr;

    if (m_pRmlBgDoc)
    {
        if (Rml::Context* bgContext = RmlUiRuntime::Instance().GetBackgroundContext())
        {
            m_BgRmlBinder.Destroy(bgContext);
            bgContext->UnloadDocument(m_pRmlBgDoc);
        }
        m_pRmlBgDoc = nullptr;
    }

    BuildRmlUi();
    // Next frame's SyncRmlModel() self-corrects visibility for both docs.
}

void mu::ui::window::CNPCShop::Release()
{
    SAFE_DELETE(m_pNewInventoryCtrl);

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }

    // Hide explicitly -- Release() has no other way to hide these once created.
    if (m_pRmlDoc)
        m_pRmlDoc->Hide();
    if (m_pRmlBgDoc)
        m_pRmlBgDoc->Hide();
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

                    mu::ui::window::GenericDialogConfig cfg;
                    cfg.showCancel = true;
                    cfg.item3D = *pItem;
                    cfg.lines = { { I18N::Game::WouldYouLikeToPurchase, false } };
                    cfg.onPrimary = []
                    {
                        GambleSystem& gambleSys = GambleSystem::Instance();
                        if (gambleSys.IsGambleShop() && BuyCost != 0)
                        {
                            const auto& itemInfo = gambleSys.GetBuyItemInfoConst();
                            SocketClient->ToGameServer()->SendBuyItemFromNpcRequest(itemInfo.ItemIndex);
                            BuyCost = itemInfo.ItemCost;
                            g_ConsoleDebug->Write(MCD_SEND, L"0x32 [SendRequestBuy(%d)]", itemInfo.ItemIndex);
                        }
                    };
                    mu::ui::window::g_pGenericConfirmDialog->Show(std::move(cfg));

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
    return mu::ui::window::WindowGeometry(m_Pos.x, m_Pos.y, NPCSHOP_WIDTH, NPCSHOP_HEIGHT).Contains(MouseX, MouseY);
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

    SyncRmlModel();
    return true;
}

bool mu::ui::window::CNPCShop::Render()
{
    EnableAlphaTest();

    // Frame background panel is RmlUi, routed through the background context (see
    // NPCShopBgRmlModel), painted by CManager::Render()'s centralized RenderBackgroundLayer() call
    // before this window's own Render()/Render3D() run.
    if (m_pNewInventoryCtrl)
    {
        m_pNewInventoryCtrl->Render();
    }

    DisableAlphaBlend();
    return true;
}

void mu::ui::window::CNPCShop::SyncRmlModel()
{
    if (m_pRmlBgDoc)
    {
        UI::RmlBridge::SyncRootTransform(m_BgRmlBinder, m_Pos);

        // RenderBackgroundLayer() renders whatever's shown in the shared background context
        // regardless of caller, so this Hide()/Show() is what keeps the bg panel hidden when closed.
        if (IsVisible()) m_pRmlBgDoc->Show(); else m_pRmlBgDoc->Hide();
    }

    if (!m_pRmlDoc) return;
    if (IsVisible()) m_pRmlDoc->Show(); else m_pRmlDoc->Hide();

    UI::RmlBridge::SyncRootTransform(m_RmlBinder, m_Pos);

    auto& model = m_RmlBinder.GetModel();
    auto syncBool = [&](bool NPCShopRmlModel::* field, const char* boundName, bool value)
    {
        if (model.*field != value) { model.*field = value; m_RmlBinder.MarkDirty(boundName); }
    };
    auto syncText = [&](Rml::String NPCShopRmlModel::* field, const char* boundName, const Rml::String& value)
    {
        if (model.*field != value) { model.*field = value; m_RmlBinder.MarkDirty(boundName); }
    };
    auto syncWide = [&](Rml::String NPCShopRmlModel::* field, const char* boundName, const wchar_t* text)
    {
        syncText(field, boundName, StringUtils::WideToNarrow(text));
    };

    syncWide(&NPCShopRmlModel::title, "title", I18N::Game::Merchant);

    wchar_t taxRateBuf[256];
    mu_swprintf(taxRateBuf, I18N::Game::TaxRateDChangedInRealTime, m_iTaxRate);
    syncWide(&NPCShopRmlModel::taxRateText, "tax_rate_text", taxRateBuf);

    syncBool(&NPCShopRmlModel::repairVisible, "repair_visible", m_bRepairShop);
    syncWide(&NPCShopRmlModel::repairTooltip, "repair_tooltip", I18N::Game::RepairL);
    syncWide(&NPCShopRmlModel::repairAllTooltip, "repair_all_tooltip", I18N::Game::RepairAllA);
    syncWide(&NPCShopRmlModel::repairAllLabel, "repair_all_label", I18N::Game::RepairAll);

    wchar_t goldBuf[256] = { 0, };
    ConvertGold(AllRepairGold, goldBuf);
    syncWide(&NPCShopRmlModel::repairGoldText, "repair_gold_text", goldBuf);

    // getGoldColor() packs (A<<24)+(R<<16)+(G<<8)+B -- unpack into an rgba() CSS string.
    const unsigned int goldArgb = getGoldColor(AllRepairGold);
    char goldColorBuf[32];
    snprintf(goldColorBuf, sizeof(goldColorBuf), "rgba(%u,%u,%u,%u)",
        (goldArgb >> 16) & 0xFF, (goldArgb >> 8) & 0xFF, goldArgb & 0xFF, (goldArgb >> 24) & 0xFF);
    syncText(&NPCShopRmlModel::repairGoldColor, "repair_gold_color", Rml::String(goldColorBuf));
}

float mu::ui::window::CNPCShop::GetLayerDepth()
{
    return 4.55;
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
            mu::ui::window::GenericDialogConfig cfg;
            cfg.showCancel = true;
            cfg.item3D = *pItem;
            cfg.lines = {
                { I18N::Game::AnExpensiveItem, true },
                { I18N::Game::CheckTheItemPlease, true },
                { I18N::Game::AreYouSureYouWantToSellIt, true },
            };
            cfg.onPrimary = []
            {
                CPickedItem* pPickedItem = mu::ui::window::CInventoryCtrl::GetPickedItem();
                int iSourceIndex = pPickedItem ? pPickedItem->GetSourceLinealPos() : -1;
                if (iSourceIndex >= MAX_EQUIPMENT_INDEX && iSourceIndex < MAX_MY_INVENTORY_EX_INDEX)
                {
                    SocketClient->ToGameServer()->SendSellItemToNpcRequest(iSourceIndex);
                    g_pNPCShop->SetSellingItem(true);
                }
                else
                {
                    mu::ui::window::CInventoryCtrl::BackupPickedItem();
                }
            };
            cfg.onSecondary = []
            {
                mu::ui::window::CInventoryCtrl::BackupPickedItem();
            };
            mu::ui::window::g_pGenericConfirmDialog->Show(std::move(cfg));
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
    // Top-right corner close "X" (shared frame): hides + swallows the click. The Repair/Repair-All
    // buttons are handled by RmlUi's data-event-click (see Create()).
    if (m_bSellingItem == false && g_pNewUISystem->HandleFrameCornerClose(m_Pos, mu::ui::window::INTERFACE_NPCSHOP))
    {
        return true;
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
