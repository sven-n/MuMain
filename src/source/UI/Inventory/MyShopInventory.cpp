
#include "stdafx.h"
#include "UI/Inventory/MyShopInventory.h"
#include "Audio/DSPlaySound.h"
#include "UI/Core/WindowSystem.h"
#include "UI/RmlBridge/RmlPointer.h"
#include "UI/RmlBridge/RmlWindowClose.h"
#include "UI/Dialogs/GenericConfirmDialog.h"
#include "UI/Dialogs/GenericMenuDialog.h"
#include "UI/Core/WindowCommon.h" // g_IsPurchaseShop
#include "UI/Inventory/ShopUpdates.h"
#include "GameLogic/Items/PersonalShopCurrency.h"
#include "GameLogic/Items/PersonalShopTitleImp.h"
#include "I18N/All.h"
#include "GameLogic/Items/ShopRestrictions.h"

// RmlUi migration -- see this class's header comment.
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlNativeTextSize.h"
#include "Core/Utilities/StringUtils.h"
#include <RmlUi/Core/ElementDocument.h>
#include <optional>
#include <vector>

const int iMAX_SHOPTITLE_MULTI = 26;

using namespace SEASON3B;
using namespace mu::ui::window;

namespace
{
// Sends the price of an item of the own personal shop. Zen prices use the original packet, so
// they work with every server.
void SendItemPrice(int itemSlot, const GameLogic::PersonalShop::ItemPrice& price)
{
    if (price.Kind == GameLogic::PersonalShop::PriceKind::Zen)
    {
        SocketClient->ToGameServer()->SendPlayerShopSetItemPrice(itemSlot, price.Amount);
        return;
    }

    const auto encoded = GameLogic::PersonalShop::EncodeExtendedPrice(price);
    SocketClient->ToGameServer()->SendPlayerShopSetItemPriceExtended(encoded.MoneyPrice, encoded.PriceItemType,
                                                                     encoded.RequiredItemAmount, itemSlot);
}

// Sends the price of the item which is moved into or within the own shop, or whose price is
// changed, and remembers it for the tooltip. sourceIndex is the shop slot of the item whose price
// is changed when no item is picked; a negative one changes nothing.
void ApplyPersonalShopItemPrice(const GameLogic::PersonalShop::ItemPrice& price, int sourceIndex)
{
    if (g_pMyShopInventory->IsEnablePersonalShop() == true)
    {
        SocketClient->ToGameServer()->SendPlayerShopClose();
    }

    CPickedItem* pPickedItem = CInventoryCtrl::GetPickedItem();
    if (pPickedItem == nullptr)
    {
        if (sourceIndex >= 0)
        {
            UI::Shop::RememberReplacedOwnShopPrice(sourceIndex);
            SendItemPrice(sourceIndex, price);
            AddPersonalItemPrice(sourceIndex, price, g_IsPurchaseShop);
        }

        // The slot is consumed: a stale source index would poison TransferSourceIndex()'s
        // fallback for later move responses without a picked item (e.g. vault auto-moves).
        g_pMyShopInventory->ChangeSourceIndex(-1);
        return;
    }

    ITEM* pItemObj = pPickedItem->GetItem();
    const int iSourceIndex = pPickedItem->GetSourceLinealPos();
    const int iTargetIndex = g_pMyShopInventory->GetTargetIndex();
    const auto* pOwnerInventory = pPickedItem->GetOwnerInventory();
    const bool isFromInventory = pOwnerInventory == g_pMyInventory->GetInventoryCtrl() || pOwnerInventory == nullptr;
    const bool isFromShop = pOwnerInventory == g_pMyShopInventory->GetInventoryCtrl();
    if (isFromInventory || isFromShop)
    {
        SendItemPrice(iSourceIndex, price);
        const STORAGE_TYPE sourceStorage = isFromShop ? STORAGE_TYPE::MYSHOP : STORAGE_TYPE::INVENTORY;
        SendRequestEquipmentItem(sourceStorage, iSourceIndex, pItemObj, STORAGE_TYPE::MYSHOP, iTargetIndex);
    }

    AddPersonalItemPrice(iTargetIndex, price, g_IsPurchaseShop);
    g_pMyShopInventory->ChangeSourceIndex(-1);
}

ITEM* FindPricedItem()
{
    CPickedItem* pPickedItem = CInventoryCtrl::GetPickedItem();
    return pPickedItem ? pPickedItem->GetItem() : g_pMyShopInventory->FindItem(g_pMyShopInventory->GetSourceIndex());
}

// A zen price below the value of the item has to be confirmed by the player.
void ShowZenPriceConfirmation(const ITEM& item, const wchar_t* strPrice,
                              const GameLogic::PersonalShop::ItemPrice& price)
{
    // Plain item3D + OkCancel confirm, same shape as NPCShop.cpp's own IsHighValueItem() sell
    // confirm. The price is just captured by value here instead of needing a GenericDialogConfig
    // field of its own.
    wchar_t strText[MAX_TEXT_LENGTH] = {};
    mu_swprintf(strText, I18N::Game::SellingPriceSZen, strPrice);

    GenericDialogConfig cfg;
    cfg.showCancel = true;
    cfg.item3D = item;
    cfg.lines = {
        {strText, true},
        {I18N::Game::DoYouWantToSellItemAtThisPrice, false},
    };
    cfg.onPrimary = [price]
    {
        int sourceIndex = -1;
        if (ITEM* pItem = g_pMyShopInventory->FindItem(g_pMyShopInventory->GetSourceIndex()))
        {
            sourceIndex = g_pMyShopInventory->GetItemInventoryIndex(pItem);
        }

        ApplyPersonalShopItemPrice(price, sourceIndex);
    };
    cfg.onCancel = []
    {
        CInventoryCtrl::BackupPickedItem();
        g_pMyShopInventory->ChangeSourceIndex(-1);
    };
    // Chained from inside the price dialog's own onPrimary -- proven-safe pattern, see
    // CGenericConfirmDialog::Resolve()'s own comment on why this doesn't stomp the dialog that's
    // still resolving.
    g_pGenericConfirmDialog->Show(std::move(cfg));
}

void ShowItemPriceDialog(const GameLogic::PersonalShop::ItemPrice& priceType, const std::wstring& initialText);

// A menu of currencies to choose the one of the price from. Choosing one, or closing the menu,
// shows the price dialog again with the typed amount. A long list takes two columns, so the
// menu stays on the screen.
void ShowPriceTypeMenu(const std::wstring& title, const std::vector<GameLogic::PersonalShop::ItemPrice>& priceTypes,
                       std::optional<std::vector<GameLogic::PersonalShop::ItemPrice>> otherItems,
                       const GameLogic::PersonalShop::ItemPrice& current, const std::wstring& typedText)
{
    constexpr size_t MaximumSingleColumnButtons = 6;

    GenericMenuConfig cfg;
    cfg.id = "personal-shop-price-type";
    cfg.title = title;
    for (const auto& priceType : priceTypes)
    {
        GenericMenuConfig::MenuButton button;
        button.label = GameLogic::PersonalShop::GetCurrencyName(priceType);
        button.onClick = [priceType, typedText] { ShowItemPriceDialog(priceType, typedText); };
        cfg.buttons.push_back(std::move(button));
    }

    if (otherItems)
    {
        GenericMenuConfig::MenuButton button;
        button.label = I18N::Game::OtherItems;
        button.onClick = [items = *otherItems, current, typedText]
        { ShowPriceTypeMenu(I18N::Game::OtherItems, items, std::nullopt, current, typedText); };
        cfg.buttons.push_back(std::move(button));
    }

    if (cfg.buttons.size() > MaximumSingleColumnButtons)
    {
        cfg.columns = 2;
    }

    GenericMenuConfig::MenuButton cancel;
    cancel.label = I18N::Game::Cancel;
    cancel.compact = true;
    cancel.dismiss = true;
    cancel.onClick = [current, typedText] { ShowItemPriceDialog(current, typedText); };
    cfg.buttons.push_back(std::move(cancel));
    cfg.onCancel = [current, typedText] { ShowItemPriceDialog(current, typedText); };

    g_pGenericMenuDialog->Show(std::move(cfg));
}

// The currencies the server allows: the common ones (zen, the account coins, the jewels), and
// the other items in a second menu behind an "Other items" button.
void ShowPriceTypeMenu(const GameLogic::PersonalShop::ItemPrice& current, const std::wstring& typedText)
{
    std::vector<GameLogic::PersonalShop::ItemPrice> common;
    std::vector<GameLogic::PersonalShop::ItemPrice> others;
    for (const auto& priceType : GameLogic::PersonalShop::GetAllowedPriceTypes())
    {
        (GameLogic::PersonalShop::IsCommonPriceType(priceType) ? common : others).push_back(priceType);
    }

    ShowPriceTypeMenu(I18N::Game::Currency, common, others.empty() ? std::nullopt : std::optional(std::move(others)),
                      current, typedText);
}

// The price dialog with the given currency. When the server allows other prices than zen, it
// names the currency in its own line, and its secondary button opens the menu of the currencies.
void ShowItemPriceDialog(const GameLogic::PersonalShop::ItemPrice& priceType, const std::wstring& initialText)
{
    GenericDialogConfig cfg;
    cfg.showCancel = true;
    cfg.lines = {{I18N::Game::EnterSellingPrice, false}};
    cfg.input = GenericDialogConfig::InputField{};
    cfg.input->mode = GenericDialogConfig::InputField::Mode::Text;
    cfg.input->maxLength = 8;
    cfg.input->numericOnly = true;
    cfg.input->initialText = initialText;

    if (GameLogic::PersonalShop::AreOtherPricesThanZenAllowed())
    {
        wchar_t currencyLine[MAX_TEXT_LENGTH] = {};
        mu_swprintf(currencyLine, I18N::Game::CurrencyLs, GameLogic::PersonalShop::GetCurrencyName(priceType));
        cfg.lines.push_back({currencyLine, true, RGBA(255, 210, 60, 255)});
        cfg.secondaryLabel = I18N::Game::Currency;
        cfg.onSecondary = [priceType] { ShowPriceTypeMenu(priceType, g_pGenericConfirmDialog->GetInputText()); };
    }

    cfg.onPrimary = [priceType]
    {
        const std::wstring strTextW = g_pGenericConfirmDialog->GetInputText();
        GameLogic::PersonalShop::ItemPrice price = priceType;
        price.Amount = strTextW.empty() ? 0 : _wtoi(strTextW.c_str());
        if (price.Amount == 0)
        {
            g_pGenericConfirmDialog->KeepOpen();
            return;
        }

        ITEM* pItem = FindPricedItem();
        if (pItem != nullptr && price.Kind == GameLogic::PersonalShop::PriceKind::Zen &&
            price.Amount < static_cast<int>(ItemValue(pItem, 2)))
        {
            ShowZenPriceConfirmation(*pItem, strTextW.c_str(), price);
        }
        else
        {
            ApplyPersonalShopItemPrice(price, g_pMyShopInventory->GetSourceIndex());
        }

        g_pMyShopInventory->SetInputValueTextBox(false);
    };
    cfg.onCancel = []
    {
        CInventoryCtrl::BackupPickedItem();
        g_pMyShopInventory->SetInputValueTextBox(false);
        g_pMyShopInventory->ChangeSourceIndex(-1);
    };

    g_pGenericConfirmDialog->Show(std::move(cfg));
}
} // namespace

void mu::ui::window::ShowPersonalShopItemValueDialog()
{
    // Numeric Mode::Text price entry. See MyShopInventory.h's own declaration comment for why
    // this is a free function shared across all 4 call sites instead of duplicated per site.
    ShowItemPriceDialog({}, {});
}

mu::ui::window::CMyShopInventory::CMyShopInventory() : m_SourceIndex(-1), m_TargetIndex(-1), m_EnablePersonalShop(false)
{
    m_pNewUIMng = NULL;
    m_pNewInventoryCtrl = NULL;
    m_bIsEnableInputValueTextBox = false;
    m_bOpenLocked = false;
    m_bOpenApplyTooltip = false;
}

mu::ui::window::CMyShopInventory::~CMyShopInventory()
{
    Release();
}

bool mu::ui::window::CMyShopInventory::Create(CManager* pNewUIMng)
{
    if (NULL == pNewUIMng || NULL == g_pNewItemMng)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_MYSHOP_INVENTORY, this);

    m_pNewInventoryCtrl = new CInventoryCtrl;
    if (false == m_pNewInventoryCtrl->Create(STORAGE_TYPE::MYSHOP, g_pNewItemMng, this, 8, 4, MAX_MY_INVENTORY_EX_INDEX))
    {
        SAFE_DELETE(m_pNewInventoryCtrl);
        return false;
    }

    m_pNewInventoryCtrl->SetToolTipType(TOOLTIP_TYPE_MY_SHOP);

    ChangePersonal(m_EnablePersonalShop);

    BuildRmlUi();

    Show(false);

    return true;
}

void mu::ui::window::CMyShopInventory::BindRmlModel(Rml::DataModelConstructor& c, MyShopRmlModel& model)
{
    UI::RmlBridge::BindWindowClose(c, mu::ui::window::INTERFACE_MYSHOP_INVENTORY);
    UI::Items::RegisterItemGridCells(c);
    c.Bind("grid_cells", &model.gridCells);
    c.Bind("text_px", &model.textPx);

    c.Bind("title", &model.title);
    c.Bind("shop_title", &model.shopTitle);
    c.Bind("exit_tooltip", &model.exitTooltip);

    c.Bind("open_locked", &model.openLocked);
    c.Bind("open_tooltip", &model.openTooltip);

    c.Bind("close_locked", &model.closeLocked);
    c.Bind("close_tooltip", &model.closeTooltip);

    c.Bind("show_still_opening", &model.showStillOpening);
    c.Bind("still_opening_text", &model.stillOpeningText);
    c.Bind("warning_text", &model.warningText);
    c.Bind("selling_price_text", &model.sellingPriceText);
    c.Bind("please_verify_text", &model.pleaseVerifyText);
    c.Bind("already_in_store_text", &model.alreadyInStoreText);
    c.Bind("cancel_sold_text", &model.cancelSoldText);
    c.Bind("cant_be_returned_text", &model.cantBeReturnedText);
    c.Bind("all_item_trading_text", &model.allItemTradingText);
    c.Bind("can_only_be_done_using_zen_text", &model.canOnlyBeDoneUsingZenText);

    c.BindEventCallback("my_shop_exit_click",
        [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
        {
            g_pNewUISystem->Hide(mu::ui::window::INTERFACE_MYSHOP_INVENTORY);
        });
    c.BindEventCallback(
        "my_shop_open_click",
        [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
        {
            if (m_bOpenLocked) // a locked CNewUIButton shows its hint but takes no click
                return;
            wchar_t shopTitle[MAX_SHOPTITLE + 1]{};
            GetTitle(shopTitle);
            if (IsExistUndecidedPrice() == false && wcslen(shopTitle) > 0)
            {
                if (m_EnablePersonalShop == false)
                {
                    mu::ui::window::GenericDialogConfig cfg;
                    cfg.showCancel = true;
                    cfg.lines.push_back({ I18N::Game::DoYouWantToOpenAStore, false });
                    cfg.onPrimary = [this]
                    {
                        wchar_t confirmedTitle[MAX_SHOPTITLE]{};
                        GetTitle(confirmedTitle);
                        wcscpy(g_szPersonalShopTitle, confirmedTitle);
                        SocketClient->ToGameServer()->SendPlayerShopOpen(MU_C16(confirmedTitle));

                        g_pNewUISystem->Hide(mu::ui::window::INTERFACE_MYSHOP_INVENTORY);
                        g_pNewUISystem->Hide(mu::ui::window::INTERFACE_INVENTORY);
                    };
                    mu::ui::window::g_pGenericConfirmDialog->Show(std::move(cfg));
                }
                else
                {
                    wcscpy(g_szPersonalShopTitle, shopTitle);
                    SocketClient->ToGameServer()->SendPlayerShopOpen(MU_C16(shopTitle));

                    g_pNewUISystem->Hide(mu::ui::window::INTERFACE_MYSHOP_INVENTORY);
                    g_pNewUISystem->Hide(mu::ui::window::INTERFACE_INVENTORY);
                    g_pNewUISystem->Hide(mu::ui::window::INTERFACE_INVENTORY_EXT);
                }
            }
            else
            {
                g_pSystemLogBox->AddText(I18N::Game::ThereSNoStoreNameOrItemPrice, mu::ui::window::TYPE_ERROR_MESSAGE);
            }
        });
    c.BindEventCallback("my_shop_close_click",
                        [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                        {
                            if (!m_EnablePersonalShop) // locked until the shop is open
                                return;
                            SocketClient->ToGameServer()->SendPlayerShopClose();

                            g_pNewUISystem->Hide(mu::ui::window::INTERFACE_MYSHOP_INVENTORY);
                            g_pNewUISystem->Hide(mu::ui::window::INTERFACE_INVENTORY);
                            g_pNewUISystem->Hide(mu::ui::window::INTERFACE_INVENTORY_EXT);
                        });

    // Former RenderTextInfo()'s static instructional lines -- set once here, not
    // re-synced every frame, since none of this text ever changes at runtime (same
    // convention CCharacterInfoWindow's own static labels use). Only show_still_opening
    // (SyncRmlModel()) reflects live state.
    model.warningText = StringUtils::WideToNarrow(I18N::Game::Warning);
    model.sellingPriceText = StringUtils::WideToNarrow(I18N::Game::SellingPriceWhenOpeningTheStore);
    model.pleaseVerifyText = StringUtils::WideToNarrow(I18N::Game::PleaseVerify);
    model.alreadyInStoreText = StringUtils::WideToNarrow(I18N::Game::AlreadyInThePersonalStore);
    model.cancelSoldText = StringUtils::WideToNarrow(I18N::Game::CancelSoldItem);
    model.cantBeReturnedText = StringUtils::WideToNarrow(I18N::Game::CanTBeReturned);
    model.allItemTradingText = StringUtils::WideToNarrow(I18N::Game::AllItemTrading);
    model.canOnlyBeDoneUsingZenText = StringUtils::WideToNarrow(I18N::Game::CanOnlyBeDoneUsingZen);
    model.stillOpeningText = StringUtils::WideToNarrow(I18N::Game::StillOpening);
}

void mu::ui::window::CMyShopInventory::BuildRmlUi()
{
    m_RmlView.Ensure();
}

void mu::ui::window::CMyShopInventory::Release()
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

// Caps the <input>'s own edit buffer at the same length the native CUITextInputBox was given
// (Init()'s iMAX_SHOPTITLE_MULTI - 1). Set from here rather than written into each theme's .rml so
// the limit stays a single C++ rule that can't drift per theme.
void mu::ui::window::CMyShopInventory::ApplyShopTitleLimit()
{
    if (!m_RmlView.Document()) return;

    if (Rml::Element* field = m_RmlView.Document()->GetElementById("shop_title"))
        field->SetAttribute("maxlength", iMAX_SHOPTITLE_MULTI - 1);
}

// Drops keyboard focus from the shop-title <input> unless the cursor is actually over it. Uses
// RmlUi's own hover hit-test rather than comparing the mouse against a rect derived from
// GetAbsoluteOffset()/GetBox(): #panel carries a CSS transform: scale(), which those two report
// inconsistently, so a hand-rolled rect blurred the field even on clicks inside it.
void mu::ui::window::CMyShopInventory::BlurShopTitleOnOutsideClick()
{
    if (!m_RmlView.Document()) return;

    Rml::Element* field = m_RmlView.Document()->GetElementById("shop_title");
    if (field == nullptr)
        return;

    if (field->IsPseudoClassSet("focus") && !field->IsPseudoClassSet("hover"))
        field->Blur();
}

void mu::ui::window::CMyShopInventory::GetTitle(wchar_t* titletext)
{
    if (titletext == nullptr) return;

    const std::wstring title = StringUtils::NarrowToWide(m_RmlView.GetModel().shopTitle);
    wcsncpy(titletext, title.c_str(), iMAX_SHOPTITLE_MULTI - 1);
    titletext[iMAX_SHOPTITLE_MULTI - 1] = L'\0';
}

void mu::ui::window::CMyShopInventory::SetTitle(wchar_t* titletext)
{
    m_RmlView.GetModel().shopTitle = (titletext != nullptr) ? StringUtils::WideToNarrow(titletext) : Rml::String();
    m_RmlView.MarkDirty("shop_title");
}

bool mu::ui::window::CMyShopInventory::InsertItem(int iIndex, std::span<const BYTE> pbyItemPacket)
{
    if (m_pNewInventoryCtrl)
    {
        return m_pNewInventoryCtrl->AddItem(iIndex, pbyItemPacket);
    }

    return false;
}

void mu::ui::window::CMyShopInventory::DeleteItem(int iIndex)
{
    if (m_pNewInventoryCtrl)
    {
        ITEM* pItem = m_pNewInventoryCtrl->FindItem(iIndex);
        if (pItem != NULL)
            m_pNewInventoryCtrl->RemoveItem(pItem);
    }
}

void mu::ui::window::CMyShopInventory::DeleteAllItems()
{
    if (m_pNewInventoryCtrl)
    {
        m_pNewInventoryCtrl->RemoveAllItems();
    }
}

ITEM* mu::ui::window::CMyShopInventory::FindItem(int iLinealPos)
{
    if (m_pNewInventoryCtrl)
        return m_pNewInventoryCtrl->FindItem(iLinealPos);
    return NULL;
}

void mu::ui::window::CMyShopInventory::ChangePersonal(bool state)
{
    m_EnablePersonalShop = state;

    // Close's lock state is purely a function of m_EnablePersonalShop (see SyncRmlModel()); only
    // Open's lock/tooltip need their own state, since OpenButtonLock()/UnLock() can also drive them
    // independently of this call.
    m_bOpenLocked = false;
    m_bOpenApplyTooltip = m_EnablePersonalShop;
}

void mu::ui::window::CMyShopInventory::OpenButtonLock()
{
    m_bOpenLocked = true;
    m_bOpenApplyTooltip = false;
}

void mu::ui::window::CMyShopInventory::OpenButtonUnLock()
{
    m_bOpenLocked = false;
    m_bOpenApplyTooltip = true;
}

const bool mu::ui::window::CMyShopInventory::IsEnablePersonalShop() const
{
    return m_EnablePersonalShop;
}

bool mu::ui::window::CMyShopInventory::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_MYSHOP_INVENTORY) == true)
    {
        if (mu::ui::window::IsPress(VK_ESCAPE) == true)
        {
            g_pNewUISystem->Hide(mu::ui::window::INTERFACE_MYSHOP_INVENTORY);
            PlayBuffer(SOUND_CLICK01);
            return false;
        }
    }

    return true;
}

bool mu::ui::window::CMyShopInventory::IsPointerOverPanel()
{
    // #panel takes no pointer events, so the grid's clicks stay native; its drawn box still holds
    // the pointer.
    Rml::ElementDocument* document = m_RmlView.Document();
    return UI::RmlBridge::IsPointerWithin(document != nullptr ? document->GetElementById("panel") : nullptr);
}

bool mu::ui::window::CMyShopInventory::MyShopInventoryProcess()
{
    if (IsPointerOverPanel() == false)
    {
        return false;
    }

    CPickedItem* pPickedItem = CInventoryCtrl::GetPickedItem();

    if (m_pNewInventoryCtrl && pPickedItem && IsRelease(VK_LBUTTON))
    {
        ITEM* pItemObj = pPickedItem->GetItem();
        int iSourceIndex = pPickedItem->GetSourceLinealPos();
        int iTargetIndex = pPickedItem->GetTargetLinealPos(m_pNewInventoryCtrl);

#ifndef KJH_FIX_CHANGE_ITEM_PRICE_IN_PERSONAL_SHOP				// #ifndef
        if (GameLogic::Items::IsPersonalShopBan(pItemObj))
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
            if (GameLogic::Items::IsPersonalShopBan(pItemObj) == true)
            {
                g_pSystemLogBox->AddText(I18N::Game::ThisItemIsNotAllowedToUseThePrivateStore, mu::ui::window::TYPE_ERROR_MESSAGE);
                return true;
            }

            if (m_pNewInventoryCtrl->CanMove(iTargetIndex, pItemObj))
            {
                ChangeSourceIndex(iSourceIndex);
                ChangeTargetIndex(iTargetIndex);

                ShowPersonalShopItemValueDialog();
                SetInputValueTextBox(true);

                pPickedItem->HidePickedItem();
                return true;
            }
        }
        else if (pPickedItem->GetOwnerInventory() == NULL)
        {
            if (GameLogic::Items::IsPersonalShopBan(pItemObj) == true)
            {
                g_pSystemLogBox->AddText(I18N::Game::ThisItemIsNotAllowedToUseThePrivateStore, mu::ui::window::TYPE_ERROR_MESSAGE);
                return true;
            }

            if (m_pNewInventoryCtrl->CanMove(iTargetIndex, pItemObj))
            {
                ChangeSourceIndex(iSourceIndex);
                ChangeTargetIndex(iTargetIndex);

                ShowPersonalShopItemValueDialog();
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
                // No price dialog follows, so reset right away: see ApplyPersonalShopItemPrice
                // for why no stale source index may stay behind.
                ChangeSourceIndex(-1);
                return true;
            }
        }
    }
    else if (m_pNewInventoryCtrl && !pPickedItem && IsPress(VK_RBUTTON))
    {
        MouseRButton = false;
        MouseRButtonPop = false;
        MouseRButtonPush = false;

        int iCurSquareIndex = m_pNewInventoryCtrl->GetIndexAtPointer();

        if (iCurSquareIndex != -1)
        {
            ITEM* pItem = g_pMyShopInventory->FindItem(iCurSquareIndex);

            if(pItem)
            {
                // The price table, the tooltip and the server all key the price by the item's
                // origin cell; the hovered cell can be any covered cell of a multi-cell item.
                ChangeSourceIndex(m_pNewInventoryCtrl->GetIndexByItem(pItem));
                ChangeTargetIndex(-1);
                ShowPersonalShopItemValueDialog();
                SetInputValueTextBox(true);
            }
            return true;
        }
    }

    return false;
}

bool mu::ui::window::CMyShopInventory::UpdateMouseEvent()
{
    if (m_pNewInventoryCtrl && false == m_pNewInventoryCtrl->UpdateMouseEvent())
    {
        return false;
    }

    if (IsPointerOverPanel())
    {
        if (MyShopInventoryProcess() == true)
        {
            return false;
        }

        // Click-to-focus is RmlUi's own (the <input> is the hovered element, so
        // Context::ProcessMouseButtonDown focuses it). Only the release half needs help: RmlUi
        // leaves focus alone when the click lands on a non-focusable element, so clicking the world
        // or the panel chrome would otherwise keep the field focused -- and with it every hotkey
        // suppressed (CHotKey::CanUpdateKeyEvent()). Mirror the native box's own blur-on-
        // outside-click using the field's live RCSS rect, so no offset is duplicated here.
        if (mu::ui::window::IsRelease(VK_LBUTTON))
            BlurShopTitleOnOutsideClick();
    }

    // The 3 real buttons (Exit/Open/Close) are handled by RmlUi's data-event-click (see Create()).

    if (WindowProcess())
        return false;

    return true;
}

bool mu::ui::window::CMyShopInventory::WindowProcess()
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

bool mu::ui::window::CMyShopInventory::Update()
{
    if (m_pNewInventoryCtrl && false == m_pNewInventoryCtrl->Update())
    {
        return false;
    }

    SyncRmlModel();
    return true;
}

void mu::ui::window::CMyShopInventory::SyncRmlModel()
{
    m_ItemTarget.Sync(m_RmlView.Document() ? m_RmlView.Document()->GetElementById("item_view") : nullptr, IsVisible());
    if (!m_RmlView.Document()) return;
    UI::RmlBridge::SyncDocumentVisibility(m_RmlView.Document(), IsVisible());
    if (!IsVisible())
        return;

    if (m_pNewInventoryCtrl)
        m_pNewInventoryCtrl->FollowGridPx(m_RmlView.Document(), "item_grid");
    if (m_pNewInventoryCtrl && m_RmlView.GetModel().gridCells != m_pNewInventoryCtrl->Cells())
    {
        m_RmlView.GetModel().gridCells = m_pNewInventoryCtrl->Cells();
        m_RmlView.MarkDirty("grid_cells");
    }
    UI::RmlBridge::SyncNativeTextSize(m_RmlView.Binder());

    auto& model = m_RmlView.GetModel();
    auto syncBool = [&](bool MyShopRmlModel::* field, const char* boundName, bool value)
    {
        if (model.*field != value) { model.*field = value; m_RmlView.MarkDirty(boundName); }
    };
    auto syncWide = [&](Rml::String MyShopRmlModel::* field, const char* boundName, const wchar_t* text)
    {
        const Rml::String value = StringUtils::WideToNarrow(text);
        if (model.*field != value) { model.*field = value; m_RmlView.MarkDirty(boundName); }
    };

    syncWide(&MyShopRmlModel::title, "title", I18N::Game::PersonalStore);
    syncWide(&MyShopRmlModel::exitTooltip, "exit_tooltip", I18N::Game::Close388);

    syncBool(&MyShopRmlModel::openLocked, "open_locked", m_bOpenLocked);
    syncWide(&MyShopRmlModel::openTooltip, "open_tooltip", m_bOpenApplyTooltip ? I18N::Game::Apply : I18N::Game::Open1107);

    syncBool(&MyShopRmlModel::closeLocked, "close_locked", !m_EnablePersonalShop);
    syncWide(&MyShopRmlModel::closeTooltip, "close_tooltip", I18N::Game::Closed);

    syncBool(&MyShopRmlModel::showStillOpening, "show_still_opening", m_EnablePersonalShop);
}

bool mu::ui::window::CMyShopInventory::Render()
{
    EnableAlphaTest();

    // text is RmlUi now too (MyShopRmlModel), driven by SyncRmlModel()/my_shop.rml. The shop-title
    // field is a stock RmlUi <input> in that same document, so it needs no native render pass here.

    if (m_pNewInventoryCtrl)
    {
        m_pNewInventoryCtrl->Render();
    }

    DisableAlphaBlend();

    return true;
}

void mu::ui::window::CMyShopInventory::ClosingProcess()
{
    CInventoryCtrl::BackupPickedItem();
    g_pMyInventory->ChangeMyShopButtonStateOpen();
    // The shop-title field is blurred by m_RmlView.Document()->Hide() itself (ElementDocument::Hide() calls
    // Context::UnfocusDocument()), which is what releases SDL text input -- no explicit release
    // here, and notably not CUITextInputBox::ReleaseFocus(), which would now blur some other
    // window's still-native field rather than this one's.
}

int mu::ui::window::CMyShopInventory::GetPointedItemIndex()
{
    return m_pNewInventoryCtrl->GetPointedSquareIndex();
}

int mu::ui::window::CMyShopInventory::GetItemInventoryIndex(ITEM* pItem)
{
    return m_pNewInventoryCtrl->GetIndexByItem(pItem);
}

void mu::ui::window::CMyShopInventory::ResetSubject()
{
    SetTitle(nullptr);
}

bool mu::ui::window::CMyShopInventory::IsEnableInputValueTextBox()
{
    return m_bIsEnableInputValueTextBox;
}

void mu::ui::window::CMyShopInventory::SetInputValueTextBox(bool bIsEnable)
{
    m_bIsEnableInputValueTextBox = bIsEnable;
}

// Into #item_view (m_ItemTarget), in window pixels (the grid's FollowGridPx()).
void mu::ui::window::CMyShopInventory::RenderItems()
{
    if (m_pNewInventoryCtrl && m_pNewInventoryCtrl->IsVisible())
        m_pNewInventoryCtrl->Render3D();
}
