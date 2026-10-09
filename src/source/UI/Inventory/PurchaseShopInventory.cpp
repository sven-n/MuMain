
#include "stdafx.h"
#include "UI/Inventory/PurchaseShopInventory.h"
#include "UI/Core/WindowSystem.h"
#include "UI/RmlBridge/RmlPointer.h"
#include "UI/RmlBridge/RmlWindowClose.h"
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
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlNativeTextSize.h"
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
    if (NULL == pNewUIMng || NULL == g_pNewItemMng)
        return false;

    SetPos(x, y);

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_PURCHASESHOP_INVENTORY, this);

    m_pNewInventoryCtrl = new CInventoryCtrl;
    if (false == m_pNewInventoryCtrl->Create(STORAGE_TYPE::UNDEFINED, g_pNewItemMng, this, m_Pos.x + 16, m_Pos.y + 90, 8, 4, MAX_MY_INVENTORY_EX_INDEX))
    {
        SAFE_DELETE(m_pNewInventoryCtrl);
        return false;
    }

    m_pNewInventoryCtrl->SetToolTipType(TOOLTIP_TYPE_PURCHASE_SHOP);
    m_pNewInventoryCtrl->LockInventory();

    BuildRmlUi();

    Show(false);

    return true;
}

void mu::ui::window::CPurchaseShopInventory::BindRmlModel(Rml::DataModelConstructor& c, PurchaseShopRmlModel& model)
{
    UI::RmlBridge::BindWindowClose(c, mu::ui::window::INTERFACE_PURCHASESHOP_INVENTORY);
    UI::Items::RegisterItemGridCells(c);
    c.Bind("grid_cells", &model.gridCells);
    c.Bind("text_px", &model.textPx);

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
}

void mu::ui::window::CPurchaseShopInventory::BuildRmlUi()
{
    m_RmlView.Ensure();
}

void mu::ui::window::CPurchaseShopInventory::Release()
{
    m_ItemTarget.Disable();
    SAFE_DELETE(m_pNewInventoryCtrl);

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }

    m_RmlView.Release();
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

bool mu::ui::window::CPurchaseShopInventory::IsPointerOverPanel()
{
    // #panel takes no pointer events, so the grid's clicks stay native; its drawn box still holds
    // the pointer.
    Rml::ElementDocument* document = m_RmlView.Document();
    return UI::RmlBridge::IsPointerWithin(document != nullptr ? document->GetElementById("panel") : nullptr);
}

bool mu::ui::window::CPurchaseShopInventory::UpdateMouseEvent()
{

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
    if (IsPointerOverPanel() == false)
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
        int iCurSquareIndex = m_pNewInventoryCtrl->GetIndexAtPointer();
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
    m_ItemTarget.Sync(m_RmlView.Document() ? m_RmlView.Document()->GetElementById("item_view") : nullptr, IsVisible());
    if (!m_RmlView.Document()) return;
    UI::RmlBridge::SyncDocumentVisibility(m_RmlView.Document(), IsVisible());

    if (m_pNewInventoryCtrl)
        m_pNewInventoryCtrl->FollowGridPx(m_RmlView.Document(), "item_grid");
    if (m_pNewInventoryCtrl && m_RmlView.GetModel().gridCells != m_pNewInventoryCtrl->Cells())
    {
        m_RmlView.GetModel().gridCells = m_pNewInventoryCtrl->Cells();
        m_RmlView.MarkDirty("grid_cells");
    }
    UI::RmlBridge::SyncNativeTextSize(m_RmlView.Binder());

    auto& model = m_RmlView.GetModel();
    auto syncWide = [&](Rml::String PurchaseShopRmlModel::* field, const char* boundName, const wchar_t* text)
    {
        const Rml::String value = StringUtils::WideToNarrow(text);
        if (model.*field != value) { model.*field = value; m_RmlView.MarkDirty(boundName); }
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

// Into #item_view (m_ItemTarget), in window pixels (the grids' FollowGridPx()).
void mu::ui::window::CPurchaseShopInventory::RenderItems()
{
    if (m_pNewInventoryCtrl && m_pNewInventoryCtrl->IsVisible())
        m_pNewInventoryCtrl->Render3D();
}
