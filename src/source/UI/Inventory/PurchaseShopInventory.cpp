
#include "stdafx.h"
#include "UI/Inventory/PurchaseShopInventory.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Core/WindowGeometry.h"
#include "UI/RmlBridge/RmlPanelGeometry.h"
#include "UI/Dialogs/CustomMessageBox.h"
#include "UI/Dialogs/GenericConfirmDialog.h"
#include "UI/Inventory/MyInventory.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Network/Server/WSclient.h"
#include "I18N/All.h"

#include "GameLogic/Items/PersonalShopTitleImp.h"

// RmlUi migration -- see this class's header comment.
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/RmlBridge/RmlRootTransform.h"
#include "Core/Utilities/StringUtils.h"
#include <RmlUi/Core/ElementDocument.h>

using namespace SEASON3B;
using namespace mu::ui::window;

mu::ui::window::CPurchaseShopInventory::CPurchaseShopInventory() : m_pNewUIMng(NULL), m_pNewInventoryCtrl(NULL)
{
    m_Pos.x = m_Pos.y = 0;
    m_ShopCharacterIndex = -1;
}

mu::ui::window::CPurchaseShopInventory::~CPurchaseShopInventory()
{
    Release();
}

bool mu::ui::window::CPurchaseShopInventory::Create(CManager* pNewUIMng, int x, int y)
{
    if (NULL == pNewUIMng || NULL == g_pNewUI3DRenderMng || NULL == g_pNewItemMng)
        return false;

    SetPos(x, y);

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_PURCHASESHOP_INVENTORY, this);

    m_pNewInventoryCtrl = new CInventoryCtrl;
    if (false == m_pNewInventoryCtrl->Create(STORAGE_TYPE::UNDEFINED, g_pNewUI3DRenderMng, g_pNewItemMng, this, m_Pos.x + 16, m_Pos.y + 90, 8, 4, MAX_MY_INVENTORY_EX_INDEX))
    {
        SAFE_DELETE(m_pNewInventoryCtrl);
        return false;
    }

    m_pNewInventoryCtrl->SetToolTipType(TOOLTIP_TYPE_PURCHASE_SHOP);
    m_pNewInventoryCtrl->LockInventory();

    BuildRmlUi();
    UI::RmlBridge::RegisterForThemeReload(this, [this] { ReloadRmlTheme(); });

    Show(false);

    return true;
}

void mu::ui::window::CPurchaseShopInventory::BuildRmlUi()
{
    // Guarded so the document/model are created once, even if Create() re-runs on resolution change.
    if (!m_pRmlDoc && RmlUiRuntime::Instance().IsCreated())
    {
        const bool modelCreated = m_RmlBinder.Create(RmlUiRuntime::Instance().GetContext(), "purchase_shop",
            [this](Rml::DataModelConstructor& c, PurchaseShopRmlModel& model)
            {
                c.Bind("root_x", &model.rootX);
                c.Bind("root_y", &model.rootY);
                c.Bind("root_scale", &model.rootScale);

                c.Bind("title", &model.title);
                c.Bind("shop_owner_text", &model.shopOwnerText);
                c.Bind("warning_label", &model.warningLabel);
                c.Bind("selling_price_line", &model.sellingPriceLine);
                c.Bind("verify_line", &model.verifyLine);
                c.Bind("already_in_store_line", &model.alreadyInStoreLine);
                c.Bind("cancel_purchased_line", &model.cancelPurchasedLine);
                c.Bind("cant_be_returned_line", &model.cantBeReturnedLine);
                c.Bind("all_item_trading_line", &model.allItemTradingLine);
                c.Bind("zen_only_line", &model.zenOnlyLine);

                c.BindEventCallback("purchase_shop_exit_click",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                    {
                        g_pNewUISystem->Hide(mu::ui::window::INTERFACE_PURCHASESHOP_INVENTORY);
                    });
            });

        if (modelCreated)
            m_pRmlDoc = UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(), "Data/Interface/RmlUi/purchase_shop.rml");

        // Frame background panel uses the background context -- see PurchaseShopBgRmlModel (PurchaseShopInventory.h).
        if (Rml::Context* bgContext = RmlUiRuntime::Instance().GetBackgroundContext())
        {
            const bool bgModelCreated = m_BgRmlBinder.Create(bgContext, "purchase_shop_bg",
                [](Rml::DataModelConstructor& c, PurchaseShopBgRmlModel& model)
                {
                    c.Bind("root_x", &model.rootX);
                    c.Bind("root_y", &model.rootY);
                    c.Bind("root_scale", &model.rootScale);
                });
            if (bgModelCreated)
            {
                // Starts hidden -- CreateBackgroundDocument() no longer Show()s eagerly (see its
                // own comment, RmlTheme.h); SyncRmlModel() below is what shows/hides it.
                m_pRmlBgDoc = UI::RmlBridge::CreateBackgroundDocument("Data/Interface/RmlUi/purchase_shop_bg.rml");
            }
        }

        // Not Show()n here -- m_pRmlDoc's visibility follows this window's own Show()/Hide() via
        // SyncRmlModel(), not an eager Show() at Create() time.
    }
}

void mu::ui::window::CPurchaseShopInventory::ReloadRmlTheme()
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
    // Next frame's Update()/SyncRmlModel() self-corrects live state/visibility for both docs.
}

void mu::ui::window::CPurchaseShopInventory::Release()
{
    SAFE_DELETE(m_pNewInventoryCtrl);

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        UI::RmlBridge::UnregisterForThemeReload(this);
        m_pNewUIMng = NULL;
    }
}

bool mu::ui::window::CPurchaseShopInventory::InsertItem(int iIndex, std::span<const BYTE> pbyItemPacket)
{
    if (m_pNewInventoryCtrl)
    {
        return m_pNewInventoryCtrl->AddItem(iIndex, pbyItemPacket);
    }

    return false;
}

void mu::ui::window::CPurchaseShopInventory::DeleteItem(int iIndex)
{
    if (m_pNewInventoryCtrl)
    {
        ITEM* pItem = m_pNewInventoryCtrl->FindItem(iIndex);

        if (pItem != NULL)
        {
            m_pNewInventoryCtrl->RemoveItem(pItem);
        }
    }
}

ITEM* mu::ui::window::CPurchaseShopInventory::FindItem(int iLinealPos)
{
    if (m_pNewInventoryCtrl)
    {
        return m_pNewInventoryCtrl->FindItem(iLinealPos);
    }

    return NULL;
}

int mu::ui::window::CPurchaseShopInventory::GetItemInventoryIndex(ITEM* pItem)
{
    if (m_pNewInventoryCtrl && pItem)
    {
        return m_pNewInventoryCtrl->GetIndexByItem(pItem);
    }

    return -1;
}

bool mu::ui::window::CPurchaseShopInventory::UpdateMouseEvent()
{
    // Top-right corner close "X" (shared frame): hides + swallows the click.
    if (g_pNewUISystem->HandleFrameCornerClose(m_Pos, mu::ui::window::INTERFACE_PURCHASESHOP_INVENTORY))
    {
        return false;
    }

    // The exit button is handled by RmlUi's data-event-click (see Create()).
    if (m_pNewInventoryCtrl)
    {
        if (false == m_pNewInventoryCtrl->UpdateMouseEvent())
        {
            return false;
        }

        if (PurchaseShopInventoryProcess())
        {
            return false;
        }
    }

    if (WindowProcess())
        return false;

    return true;
}

bool mu::ui::window::CPurchaseShopInventory::WindowProcess()
{
    // #panel's own live RCSS size is the source of truth -- INVENTORY_WIDTH/HEIGHT only cover the
    // first frame after Create()/Show(true)/ReloadRmlTheme(), before RmlUi's next layout pass.
    float panelWidth = INVENTORY_WIDTH;
    float panelHeight = INVENTORY_HEIGHT;
    UI::RmlBridge::RefreshLogicalPanelSize(m_pRmlDoc, "panel", panelWidth, panelHeight);
    if (mu::ui::window::WindowGeometry(m_Pos.x, m_Pos.y, static_cast<int>(panelWidth), static_cast<int>(panelHeight)).Contains(MouseX, MouseY) == false)
    {
        return false;
    }

    if (mu::ui::window::IsPress(VK_RBUTTON))
    {
        MouseRButton = false;
        MouseRButtonPop = false;
        MouseRButtonPush = false;
    }

    return true;
}

bool mu::ui::window::CPurchaseShopInventory::UpdateKeyEvent()
{
    return true;
}

bool mu::ui::window::CPurchaseShopInventory::PurchaseShopInventoryProcess()
{
    if (m_pNewInventoryCtrl && IsPress(VK_LBUTTON))
    {
        int iCurSquareIndex = m_pNewInventoryCtrl->GetIndexAtPt(MouseX, MouseY);
        ITEM* pItem = (iCurSquareIndex != -1) ? m_pNewInventoryCtrl->FindItem(iCurSquareIndex) : nullptr;
        if (iCurSquareIndex != -1 && pItem != nullptr)
        {
            ChangeSourceIndex(iCurSquareIndex);

            GenericDialogConfig cfg;
            cfg.showCancel = true;
            cfg.item3D = *pItem;
            cfg.lines = { { I18N::Game::DoYouWantToBuyAnItem, false } };
            cfg.onPrimary = []
            {
                ITEM* pItem = g_pPurchaseShopInventory->FindItem(g_pPurchaseShopInventory->GetSourceIndex());
                CHARACTER* pCha = &CharactersClient[g_pPurchaseShopInventory->GetShopCharacterIndex()];
                if (pItem && pCha)
                {
                    int sourceIndex = g_pPurchaseShopInventory->GetItemInventoryIndex(pItem);
                    if (sourceIndex >= 0)
                    {
                        SocketClient->ToGameServer()->SendPlayerShopItemBuyRequest(pCha->Key, MU_C16(pCha->ID), sourceIndex);
                    }
                }
            };
            g_pGenericConfirmDialog->Show(std::move(cfg));
        }

        return true;
    }

    return false;
}

bool mu::ui::window::CPurchaseShopInventory::Update()
{
    if (m_pNewInventoryCtrl && false == m_pNewInventoryCtrl->Update())
    {
        return false;
    }

    SyncRmlModel();
    return true;
}

void mu::ui::window::CPurchaseShopInventory::SyncRmlModel()
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
    auto syncWide = [&](Rml::String PurchaseShopRmlModel::* field, const char* boundName, const wchar_t* text)
    {
        const Rml::String value = StringUtils::WideToNarrow(text);
        if (model.*field != value) { model.*field = value; m_RmlBinder.MarkDirty(boundName); }
    };

    syncWide(&PurchaseShopRmlModel::title, "title", I18N::Game::PersonalStore);
    // Dynamic: set per-shop-owner via ChangeTitleText() (see WSclient.cpp's shop-open packet handler).
    syncWide(&PurchaseShopRmlModel::shopOwnerText, "shop_owner_text", m_TitleText.c_str());

    syncWide(&PurchaseShopRmlModel::warningLabel, "warning_label", I18N::Game::Warning);
    syncWide(&PurchaseShopRmlModel::sellingPriceLine, "selling_price_line", I18N::Game::SellingPriceWhenOpeningTheStore);
    syncWide(&PurchaseShopRmlModel::verifyLine, "verify_line", I18N::Game::PleaseVerify);
    syncWide(&PurchaseShopRmlModel::alreadyInStoreLine, "already_in_store_line", I18N::Game::AlreadyInThePersonalStore);
    syncWide(&PurchaseShopRmlModel::cancelPurchasedLine, "cancel_purchased_line", I18N::Game::CancelPurchasedItem);
    syncWide(&PurchaseShopRmlModel::cantBeReturnedLine, "cant_be_returned_line", I18N::Game::CanTBeReturned);
    syncWide(&PurchaseShopRmlModel::allItemTradingLine, "all_item_trading_line", I18N::Game::AllItemTrading);
    syncWide(&PurchaseShopRmlModel::zenOnlyLine, "zen_only_line", I18N::Game::CanOnlyBeDoneUsingZen);
}

bool mu::ui::window::CPurchaseShopInventory::Render()
{
    EnableAlphaTest();

    // Frame background panel is RmlUi, routed through the background context (see
    // PurchaseShopBgRmlModel), painted by CManager::Render()'s centralized RenderBackgroundLayer()
    // call before this window's own Render()/Render3D() run.
    if (m_pNewInventoryCtrl)
    {
        m_pNewInventoryCtrl->Render();
    }

    DisableAlphaBlend();

    return true;
}

void mu::ui::window::CPurchaseShopInventory::ClosingProcess()
{
    if (m_pNewInventoryCtrl)
    {
        m_pNewInventoryCtrl->RemoveAllItems();
        g_ErrorReport.Write(L"@ [Notice] CPurchaseShopInventory::ClosingProcess():m_pNewInventoryCtrl->RemoveAllItems(); )\n");
    }

    m_ShopCharacterIndex = -1;

    g_pMyInventory->ChangeMyShopButtonStateOpen();
}

int mu::ui::window::CPurchaseShopInventory::GetPointedItemIndex()
{
    return m_pNewInventoryCtrl->GetPointedSquareIndex();
}
