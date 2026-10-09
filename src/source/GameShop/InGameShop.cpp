
#include "stdafx.h"
#include "I18N/All.h"

#ifdef PBG_ADD_INGAMESHOP_UI_ITEMSHOP
#include "App/Platform/Windows/iexplorer.h"
#include "UI/Core/WindowSystem.h"
#include "UI/RmlBridge/RmlElementBox.h"
#include "UI/RmlBridge/RmlPointer.h"
#include "InGameShop.h"
#include "MsgBoxIGSBuyPackageItem.h"
#include "MsgBoxIGSBuySelectItem.h"
#include "MsgBoxIGSStorageItemInfo.h"
#include "MsgBoxIGSGiftStorageItemInfo.h"
#include "World/MapInfra/MapManager.h"
#include "Audio/DSPlaySound.h"
#include "UI/Core/WindowCommon.h"
#include "UI/Events/EventPreview.h"
#include "Core/Utilities/StringUtils.h"
#include "UI/RmlBridge/RmlSyncField.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlRootTransform.h"
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Event.h>

#include <algorithm>
#include <cwctype>
#include <filesystem>

static_assert(GameShop::kStorageTextLength == MAX_TEXT_LENGTH);
static_assert(GameShop::kStorageUserNameSize == MAX_USERNAME_SIZE);
static_assert(GameShop::kStorageMessageSize == MAX_GIFT_MESSAGE_SIZE);

using namespace SEASON3B;
using namespace mu::ui::window;

namespace
{
Rml::String Narrow(const wchar_t* text)
{
    return StringUtils::WideToNarrow(text ? text : L"");
}

std::vector<Rml::String> Names(const type_listName& names)
{
    std::vector<Rml::String> result;
    for (const std::wstring& name : names)
        result.push_back(Narrow(name.c_str()));
    return result;
}

// The banner as an <img> source: RmlUi reads a leading '/' as the working directory, which the
// banners download under (Data/InGameShopBanner), whichever theme folder the document came from.
std::string BannerSource(const wchar_t* path)
{
    std::wstring generic = std::filesystem::path(path).generic_wstring();
    std::wstring lower = generic;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::towlower);
    const size_t folder = lower.find(L"/ingameshopbanner/");
    if (folder == std::wstring::npos)
        return {};
    return "/Data" + StringUtils::WideToNarrow(generic.substr(folder).c_str());
}
} // namespace

CInGameShop::CInGameShop()
{
    Init();
}

CInGameShop::~CInGameShop()
{
    Release();
}

void CInGameShop::Init()
{
    m_pNewUIMng = nullptr;
    m_bBannerLink = false;
    m_szBannerURL[0] = L'\0';
    m_iStorageTotalItemCnt = 0;
    m_iStorageCurrentPageItemCnt = 0;
    m_iStorageTotalPage = 0;
    m_iStorageCurrentPage = 0;
    m_iSelectedStorageItemIndex = 0;
    m_iStorageCurrentPageReceiveItemCnt = 0;
    m_bRequestCurrentPage = false;
}

void CInGameShop::Release()
{
    m_ItemTarget.Disable();
    m_RmlView.Release();

    ReleaseBanner();

    if (m_pNewUIMng)
    {
        m_pNewUIMng->RemoveUIObj(this);
        m_pNewUIMng = NULL;
    }

    ClearAllStorageItem();
}

bool CInGameShop::Create(CManager* pNewUIMng, int x, int y)
{
    if (pNewUIMng == NULL)
        return false;

    m_pNewUIMng = pNewUIMng;
    m_pNewUIMng->AddUIObj(mu::ui::window::INTERFACE_INGAMESHOP, this);

    SetPos(x, y);
    m_ItemTarget.SetFieldOfView(2.f);
    m_RmlView.Ensure();
    Show(false);

    return true;
}

void CInGameShop::BindRmlModel(Rml::DataModelConstructor& c, InGameShopRmlModel& model)
{
    c.Bind("text_px", &model.textPx);

    auto radio = c.RegisterStruct<RadioEntry>();
    radio.RegisterMember("name", &RadioEntry::name);
    radio.RegisterMember("selected", &RadioEntry::selected);
    radio.RegisterMember("last", &RadioEntry::last);
    c.RegisterArray<std::vector<RadioEntry>>();
    auto package = c.RegisterStruct<PackageEntry>();
    package.RegisterMember("name", &PackageEntry::name);
    package.RegisterMember("price", &PackageEntry::price);
    package.RegisterMember("shown", &PackageEntry::shown);
    c.RegisterArray<std::vector<PackageEntry>>();
    auto wallet = c.RegisterStruct<WalletEntry>();
    wallet.RegisterMember("label", &WalletEntry::label);
    wallet.RegisterMember("value", &WalletEntry::value);
    c.RegisterArray<std::vector<WalletEntry>>();
    auto row = c.RegisterStruct<StorageRow>();
    row.RegisterMember("name", &StorageRow::name);
    row.RegisterMember("period", &StorageRow::period);
    row.RegisterMember("selected", &StorageRow::selected);
    c.RegisterArray<std::vector<StorageRow>>();

    c.Bind("character_name", &model.characterName);
    c.Bind("wallet", &model.wallet);
    c.Bind("zones", &model.zones);
    c.Bind("categories", &model.categories);
    c.Bind("packages", &model.packages);
    c.Bind("page", &model.page);
    c.Bind("total_pages", &model.totalPages);
    c.Bind("storage_tabs", &model.storageTabs);
    c.Bind("storage_rows", &model.storageRows);
    c.Bind("storage_page", &model.storagePage);
    c.Bind("storage_total_pages", &model.storageTotalPages);
    c.Bind("buy_label", &model.buyLabel);
    c.Bind("gift_hint", &model.giftHint);
    c.Bind("charge_hint", &model.chargeHint);
    c.Bind("refresh_hint", &model.refreshHint);
    c.Bind("close_hint", &model.closeHint);
    c.Bind("use_label", &model.useLabel);
    c.Bind("item_name_label", &model.itemNameLabel);
    c.Bind("duration_label", &model.durationLabel);
    c.Bind("banner_src", &model.bannerSrc);
    c.Bind("banner_linked", &model.bannerLinked);
    c.Bind("script_version", &model.scriptVersion);
    c.Bind("banner_version", &model.bannerVersion);

    const auto queue = [this](std::function<void()> action) { m_PendingActions.push_back(std::move(action)); };
    const auto indexed = [queue](void (CInGameShop::*method)(int), CInGameShop* self)
    {
        return [queue, method, self](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& args)
        {
            if (args.size() == 1)
                queue([self, method, index = args[0].Get<int>(-1)] { (self->*method)(index); });
        };
    };
    c.BindEventCallback("igs_zone", indexed(&CInGameShop::SelectZone, this));
    c.BindEventCallback("igs_category", indexed(&CInGameShop::SelectCategory, this));
    c.BindEventCallback("igs_storage_tab", indexed(&CInGameShop::SelectStorageBox, this));
    c.BindEventCallback("igs_buy", indexed(&CInGameShop::BuyPackage, this));
    c.BindEventCallback("igs_page",
                        [queue](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& args)
                        {
                            if (args.size() != 1)
                                return;
                            const bool next = args[0].Get<int>(0) > 0;
                            queue([next] { next ? g_InGameShopSystem->NextPage() : g_InGameShopSystem->PrePage(); });
                        });
    c.BindEventCallback("igs_storage_page",
                        [this, queue](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& args)
                        {
                            if (args.size() != 1)
                                return;
                            const bool next = args[0].Get<int>(0) > 0;
                            queue([this, next] { next ? StorageNextPage() : StoragePrevPage(); });
                        });
    c.BindEventCallback("igs_gift",
                        [queue](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                        {
                            queue([] { CreateOkMessageBoxWithTitle(I18N::Game::RestrictedFunction, I18N::Game::ThisFunctionIsNotSupportedIn); });
                        });
    c.BindEventCallback("igs_charge",
                        [queue](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                        {
                            queue([] { CreateOkMessageBoxWithTitle(I18N::Game::RestrictedFunction, I18N::Game::ThisFunctionIsNotSupportedIn); });
                        });
    c.BindEventCallback("igs_refresh",
                        [this, queue](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                        {
                            queue([this]
                                  {
                                      if (SendsRequests())
                                          SocketClient->ToGameServer()->SendCashShopPointInfoRequest();
                                  });
                        });
    c.BindEventCallback("igs_use",
                        [this, queue](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { queue([this] { UseStorageItem(); }); });
    c.BindEventCallback("igs_close",
                        [this, queue](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { queue([this] { Close(); }); });
    c.BindEventCallback("igs_banner",
                        [this, queue](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&)
                        {
                            queue([this]
                                  {
                                      if (m_bBannerLink && !m_BannerSource.empty())
                                          leaf::OpenExplorer(m_szBannerURL);
                                  });
                        });
    c.BindEventCallback("igs_select_storage",
                        [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList& args)
                        {
                            if (args.size() == 1 && m_StorageItems.SelectRow(args[0].Get<int>(-1)))
                                m_StorageRowsDirty = true;
                        });
}

void CInGameShop::SyncRmlModel()
{
    m_ItemTarget.Sync(m_RmlView.Document() ? m_RmlView.Document()->GetElementById("igs_items") : nullptr, IsVisible());
    if (!m_RmlView.Document())
        return;
    UI::RmlBridge::SyncDocumentVisibility(m_RmlView.Document(), IsVisible());
    if (!IsVisible())
        return;
    auto& binder = m_RmlView.Binder();
    UI::RmlBridge::SyncNativeTextSize(binder);

    SyncField(binder, &InGameShopRmlModel::characterName, "character_name", Narrow(Hero->ID));

    // RenderTexts(): "My W Coin :" and the rest, each with its balance.
    std::vector<WalletEntry> wallet;
    const auto addBalance = [&wallet](const wchar_t* format, double amount, int decimals)
    {
        wchar_t label[MAX_TEXT_LENGTH] = {};
        wchar_t value[MAX_TEXT_LENGTH] = {};
        mu_swprintf(label, format, L"");
        ConvertGold(amount, value, decimals);
        wallet.push_back({Narrow(label), Narrow(value)});
    };
    addBalance(I18N::Game::MyWCoinS, g_InGameShopSystem->GetCashCreditCard(), 0);
    addBalance(I18N::Game::MyWCoinPS, g_InGameShopSystem->GetCashPrepaid(), 0);
    addBalance(I18N::Game::GoblinPointsS, g_InGameShopSystem->GetTotalMileage(), 1);
    SyncField(binder, &InGameShopRmlModel::wallet, "wallet", std::move(wallet));

    const auto radio = [](const std::vector<Rml::String>& names, int selected)
    {
        std::vector<RadioEntry> entries;
        for (size_t i = 0; i < names.size(); ++i)
            entries.push_back({names[i], static_cast<int>(i) == selected, i + 1 == names.size()});
        return entries;
    };
    std::vector<Rml::String> zoneNames;
    if (g_InGameShopSystem->GetSizeZones() > 0)
        zoneNames = Names(g_InGameShopSystem->GetZoneName());
    SyncField(binder, &InGameShopRmlModel::zones, "zones", radio(zoneNames, m_SelectedZone));
    std::vector<Rml::String> categoryNames;
    if (g_InGameShopSystem->GetSizeCategoriesAsSelectedZone() > 0)
        categoryNames = Names(g_InGameShopSystem->GetCategoryName());
    SyncField(binder, &InGameShopRmlModel::categories, "categories", radio(categoryNames, m_SelectedCategory));

    std::vector<PackageEntry> packages(INGAMESHOP_DISPLAY_ITEMLIST_SIZE);
    for (int i = 0; i < g_InGameShopSystem->GetSizePackageAsDisplayPackage() && i < INGAMESHOP_DISPLAY_ITEMLIST_SIZE; ++i)
    {
        CShopPackage* pPackage = g_InGameShopSystem->GetDisplayPackage(i);
        wchar_t value[MAX_TEXT_LENGTH] = {};
        wchar_t price[MAX_TEXT_LENGTH] = {};
        ConvertGold(pPackage->Price, value);
        mu_swprintf(price, L"%ls %ls", value, pPackage->PricUnitName);
        packages[i] = {Narrow(pPackage->PackageProductName), Narrow(price), true};
    }
    SyncField(binder, &InGameShopRmlModel::packages, "packages", std::move(packages));
    SyncField(binder, &InGameShopRmlModel::page, "page", std::to_string(g_InGameShopSystem->GetSelectPage()));
    SyncField(binder, &InGameShopRmlModel::totalPages, "total_pages", std::to_string(g_InGameShopSystem->GetTotalPages()));

    SyncField(binder, &InGameShopRmlModel::storageTabs, "storage_tabs",
              radio({Narrow(I18N::Game::Storage), Narrow(I18N::Game::GiftInventory)}, m_StorageBox));
    SyncStorageRows();
    SyncField(binder, &InGameShopRmlModel::storagePage, "storage_page", std::to_string(m_iStorageCurrentPage));
    SyncField(binder, &InGameShopRmlModel::storageTotalPages, "storage_total_pages", std::to_string(m_iStorageTotalPage));

    SyncField(binder, &InGameShopRmlModel::buyLabel, "buy_label", Narrow(I18N::Game::Buy1124));
    SyncField(binder, &InGameShopRmlModel::giftHint, "gift_hint", Narrow(I18N::Game::SendWCoin));
    SyncField(binder, &InGameShopRmlModel::chargeHint, "charge_hint", Narrow(I18N::Game::RechargeWCoin));
    SyncField(binder, &InGameShopRmlModel::refreshHint, "refresh_hint", Narrow(I18N::Game::UpdateInformation));
    SyncField(binder, &InGameShopRmlModel::closeHint, "close_hint", Narrow(I18N::Game::Close388));
    SyncField(binder, &InGameShopRmlModel::useLabel, "use_label", Narrow(I18N::Game::Use));
    SyncField(binder, &InGameShopRmlModel::itemNameLabel, "item_name_label", Narrow(I18N::Game::ItemName));
    SyncField(binder, &InGameShopRmlModel::durationLabel, "duration_label", Narrow(I18N::Game::Duration));
    SyncField(binder, &InGameShopRmlModel::bannerSrc, "banner_src", Rml::String(m_BannerSource));
    SyncField(binder, &InGameShopRmlModel::bannerLinked, "banner_linked", m_bBannerLink);

#ifdef FOR_WORK
    wchar_t text[MAX_TEXT_LENGTH] = {};
    CListVersionInfo version = g_InGameShopSystem->GetCurrentScriptVer();
    mu_swprintf(text, L"Script Ver. %d.%d.%d", version.Zone, version.year, version.yearId);
    SyncField(binder, &InGameShopRmlModel::scriptVersion, "script_version", Narrow(text));
    version = g_InGameShopSystem->GetCurrentBannerVer();
    mu_swprintf(text, L"Banner Ver. %d.%d.%d", version.Zone, version.year, version.yearId);
    SyncField(binder, &InGameShopRmlModel::bannerVersion, "banner_version", Narrow(text));
#endif // FOR_WORK
}

void CInGameShop::SyncStorageRows()
{
    if (!m_StorageRowsDirty)
        return;
    m_StorageRowsDirty = false;

    std::vector<StorageRow> rows;
    rows.reserve(m_StorageItems.Items().size());
    const int selected = m_StorageItems.SelectedRow();
    int index = 0;
    for (const GameShop::StorageItem& item : m_StorageItems.Items())
    {
        StorageRow row;
        // RenderDataLine() appended the count to the name when there was more than one.
        wchar_t name[MAX_TEXT_LENGTH] = {};
        if (item.m_iNum > 1)
            mu_swprintf(name, L"%ls(%d)", item.m_szName, item.m_iNum);
        else
            wcsncpy_s(name, item.m_szName, _TRUNCATE);
        row.name = StringUtils::WideToNarrow(name);
        row.period = StringUtils::WideToNarrow(item.m_szPeriod);
        row.selected = (index == selected);
        rows.push_back(std::move(row));
        ++index;
    }
    SyncField(m_RmlView.Binder(), &InGameShopRmlModel::storageRows, "storage_rows", std::move(rows));
}

void CInGameShop::SetPos(int x, int y)
{
    m_Pos.x = x;
    m_Pos.y = y;
}

bool CInGameShop::Render()
{
    return true;
}

// Into #igs_items, in window pixels: each shown package's item in its card's .igs-package-item.
void CInGameShop::RenderItems()
{
    Rml::ElementDocument* document = m_RmlView.Document();
    if (document == nullptr)
        return;
    Rml::ElementList boxes;
    document->QuerySelectorAll(boxes, ".igs-package-item");
    const int count = std::min({g_InGameShopSystem->GetSizePackageAsDisplayPackage(),
                                static_cast<int>(INGAMESHOP_DISPLAY_ITEMLIST_SIZE), static_cast<int>(boxes.size())});
    for (int i = 0; i < count; i++)
    {
        Rml::Vector2f offset;
        Rml::Vector2f size;
        if (UI::RmlBridge::DrawnBox(*boxes[i], Rml::BoxArea::Border, offset, size))
            RenderItem3D(offset.x, offset.y, size.x, size.y, g_InGameShopSystem->GetPackageItemCode(i), 0, 0, 0, true);
    }
}

bool CInGameShop::SendsRequests() const
{
    return !UI::EventPreview::IsShowing(UI::EventPreview::Event::CashShop);
}

void CInGameShop::SelectZone(int index)
{
    if (!g_InGameShopSystem->IsRequestEventPackge() || index < 0 || index >= g_InGameShopSystem->GetSizeZones() ||
        index == m_SelectedZone)
        return;
    m_SelectedZone = index;
    g_InGameShopSystem->SelectZone(index);
    InitCategoryBtn();
    g_InGameShopSystem->SelectCategory(m_SelectedCategory);
}

void CInGameShop::SelectCategory(int index)
{
    if (!g_InGameShopSystem->IsRequestEventPackge() || index < 0 ||
        index >= g_InGameShopSystem->GetSizeCategoriesAsSelectedZone() || index == m_SelectedCategory)
        return;
    m_SelectedCategory = index;
    g_InGameShopSystem->SelectCategory(index);
}

void CInGameShop::SelectStorageBox(int index)
{
    if (index < 0 || index >= IGS_TOTAL_LISTBOX || index == m_StorageBox)
        return;
    m_StorageBox = index;
    m_iSelectedStorageItemIndex = 0;
    m_bRequestCurrentPage = true;
    if (SendsRequests())
        SocketClient->ToGameServer()->SendCashShopStorageListRequest(1, GetCurrentStorageCode());
}

void CInGameShop::BuyPackage(int index)
{
    if (index < 0 || index >= g_InGameShopSystem->GetSizePackageAsDisplayPackage())
        return;
    CShopPackage* pPackage = g_InGameShopSystem->GetDisplayPackage(index);
    if (pPackage->PriceCount == 1)
    {
        CMsgBoxIGSBuyPackageItem* pMsgBox = NULL;
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxBuyPackageItemLayout), &pMsgBox);
        pMsgBox->Initialize(pPackage);
    }
    else if (pPackage->PriceCount > 1)
    {
        CMsgBoxIGSBuySelectItem* pMsgBox = NULL;
        CreateMessageBox(MSGBOX_LAYOUT_CLASS(CMsgBoxIGSBuySelectItemLayout), &pMsgBox);
        pMsgBox->Initialize(pPackage);
    }
}

void CInGameShop::UseStorageItem()
{
    if (m_StorageItems.Empty())
    {
        CreateOkMessageBoxWithTitle(I18N::Game::Error, I18N::Game::ThereIsNoUsableItem);
        return;
    }

    const GameShop::StorageItem* pSelectItem = m_StorageItems.Selected();
    if (pSelectItem == nullptr)
        return;

    if (m_StorageBox == IGS_SAFEKEEPING_LISTBOX)
    {
        ShowIGSStorageItemInfoDialog(pSelectItem->m_iStorageSeq, pSelectItem->m_iStorageItemSeq, pSelectItem->m_wItemCode, static_cast<char>(pSelectItem->m_szType),
            pSelectItem->m_szName, pSelectItem->m_szNum, pSelectItem->m_szPeriod);
    }
    else if (m_StorageBox == IGS_PRESENTBOX_LISTBOX)
    {
        ShowIGSGiftStorageItemInfoDialog(pSelectItem->m_iStorageSeq, pSelectItem->m_iStorageItemSeq, pSelectItem->m_wItemCode,
            pSelectItem->m_szType, pSelectItem->m_szSendUserName, pSelectItem->m_szMessage,
            pSelectItem->m_szName, pSelectItem->m_szNum, pSelectItem->m_szPeriod);
    }
}

void CInGameShop::Close()
{
    if (!g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_INGAMESHOP))
        return;
    if (SendsRequests())
        SocketClient->ToGameServer()->SendCashShopOpenState(1);
    g_pNewUISystem->Hide(mu::ui::window::INTERFACE_INGAMESHOP);
}

bool CInGameShop::Update()
{
    // The document's clicks, in the order they came, where the native buttons ran.
    std::vector<std::function<void()>> actions;
    actions.swap(m_PendingActions);
    if (IsVisible())
    {
        for (const auto& action : actions)
            action();
    }

    SyncRmlModel();
    return true;
}

bool CInGameShop::UpdateMouseEvent()
{
    if (IsVisible() == false)
        return true;

    // Nothing under the shop takes the pointer: its drawn panel holds it.
    Rml::ElementDocument* document = m_RmlView.Document();
    if (UI::RmlBridge::IsPointerWithin(document != nullptr ? document->GetElementById("panel") : nullptr))
    {
        if (mu::ui::window::IsPress(VK_RBUTTON))
        {
            MouseRButton = false;
            MouseRButtonPop = false;
            MouseRButtonPush = false;
        }
        return false;
    }

    return true;
}

bool CInGameShop::UpdateKeyEvent()
{
    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_INGAMESHOP) == true)
    {
        if (mu::ui::window::IsPress(VK_ESCAPE) == true)
        {
            Close();
            return false;
        }
    }
    return true;
}

bool CInGameShop::IsInGameShopOpen()
{
    g_ConsoleDebug->Write(MCD_NORMAL, L"InGameShopStatue.Txt CallStack - CInGameShop::IsInGameShopOpen()");
    if (Hero->Movement)
        return false;

    if (!(Hero->SafeZone) && !(WD_0LORENCIA == gMapManager.WorldActive && WD_3NORIA == gMapManager.WorldActive && WD_2DEVIAS == gMapManager.WorldActive && WD_51HOME_6TH_CHAR == gMapManager.WorldActive))
    {
        CreateOkMessageBoxWithTitle(I18N::Game::Error, I18N::Game::YouCanOnlyOpenMUItemShopInATownOrSafeZone);
        g_ConsoleDebug->Write(MCD_NORMAL, L"InGameShopStatue.Txt Return - false <%ls>", I18N::Game::YouCanOnlyOpenMUItemShopInATownOrSafeZone);
        return false;
    }

    if (g_InGameShopSystem->IsShopOpen() == false)
    {
        CreateOkMessageBoxWithTitle(I18N::Game::Error, I18N::Game::CannotOpenMUItemShopPleaseReconnectToTheGame);
        g_ConsoleDebug->Write(MCD_NORMAL, L"InGameShopStatue.Txt Return - false <%ls>", I18N::Game::CannotOpenMUItemShopPleaseReconnectToTheGame);
        return false;
    }
    g_ConsoleDebug->Write(MCD_NORMAL, L"InGameShopStatue.Txt Return - true");
    return true;
}

bool CInGameShop::IsInGameShop()
{
    if (g_pNewUISystem->IsVisible(mu::ui::window::INTERFACE_INGAMESHOP))
        return true;
    else
        return false;
}

void CInGameShop::InitBanner(wchar_t* pszFileName, wchar_t* pszBannerURL)
{
    ReleaseBanner();

    if (pszFileName == NULL)
        return;

    m_bBannerLink = pszBannerURL != NULL && pszBannerURL[0] != '#';

    // The downloaded .jpg becomes the .OZJ the game's image loader reads.
    if (Bitmaps.Convert_Format(pszFileName) == false)
        return;

    m_BannerSource = BannerSource(pszFileName);
    if (pszBannerURL != NULL)
        wcsncpy_s(m_szBannerURL, pszBannerURL, _TRUNCATE);
}

void CInGameShop::ReleaseBanner()
{
    m_BannerSource.clear();
    m_bBannerLink = false;
}

void CInGameShop::OpeningProcess()
{
    g_ConsoleDebug->Write(MCD_NORMAL, L"InGameShopStatue.Txt CallStack - CInGameShop::OpeningProcess()");
    PlayBuffer(SOUND_CLICK01);
    g_InGameShopSystem->Initalize();
    g_InGameShopSystem->SelectZone(0);
    InitZoneBtn();
    g_InGameShopSystem->SelectCategory(0);
    InitCategoryBtn();
    g_InGameShopSystem->SetRequestEventPackge();
}

void CInGameShop::ClosingProcess()
{
    PlayBuffer(SOUND_CLICK01);
    m_StorageBox = IGS_SAFEKEEPING_LISTBOX;
    m_PendingActions.clear();
    ClearAllStorageItem();
}

// The zone tabs and the category column follow the shop system's lists (SyncRmlModel()); these
// reset the selection to the first, as the native radio groups did when rebuilt.
void CInGameShop::InitZoneBtn()
{
    m_SelectedZone = 0;
}

void CInGameShop::InitCategoryBtn()
{
    m_SelectedCategory = 0;
}

void CInGameShop::AddStorageItem(int iStorageSeq, int iStorageItemSeq, int iStorageGroupCode, int iProductSeq, int iPriceSeq, int iCashPoint, wchar_t chItemType, wchar_t* pszUserName /* = NULL */, wchar_t* pszMessage /* = NULL */)
{
    int iValue = -1;
    wchar_t szText[MAX_TEXT_LENGTH] = { '\0', };
    GameShop::StorageItem Item;

    Item.m_iStorageSeq = iStorageSeq;
    Item.m_iStorageItemSeq = iStorageItemSeq;
    Item.m_iStorageGroupCode = iStorageGroupCode;
    Item.m_iProductSeq = iProductSeq;
    Item.m_iPriceSeq = iPriceSeq;
    Item.m_iCashPoint = iCashPoint;
    Item.m_iNum = 1;
    Item.m_szType = chItemType;
    Item.m_wItemCode = -1;

    if (pszUserName == NULL)
    {
        Item.m_szSendUserName[0] = '\0';
    }
    else
    {
        wcscpy(Item.m_szSendUserName, pszUserName);
    }

    if (pszMessage == NULL)
    {
        Item.m_szMessage[0] = '\0';
    }
    else
    {
        wcscpy(Item.m_szMessage, pszMessage);
    }

    if (chItemType == 'C' || chItemType == 'c')
    {
        wchar_t szValue[MAX_TEXT_LENGTH] = { '\0', };
        ConvertGold(iCashPoint, szValue);
        // Name
        mu_swprintf(Item.m_szName, I18N::Game::WCoinSCoins, szValue);

        // Num
        mu_swprintf(Item.m_szNum, I18N::Game::SWCoin, szValue);
        Item.m_iNum = iCashPoint;

        // Period
        mu_swprintf(Item.m_szPeriod, L"-");
    }
    else if (chItemType == 'P' || chItemType == 'p')
    {
        if (iPriceSeq > 0)
        {
            // Name
            if (g_InGameShopSystem->GetProductInfoFromPriceSeq(iProductSeq, iPriceSeq, CInGameShopSystem::IGS_PRODUCT_ATT_TYPE_ITEMNAME, iValue, Item.m_szName) == false)
            {
                mu_swprintf(Item.m_szName, L"aaa");
            }

            g_InGameShopSystem->GetProductInfoFromPriceSeq(iProductSeq, iPriceSeq, CInGameShopSystem::IGS_PRODUCT_ATT_TYPE_NUM, iValue, szText);
            if (iValue > 0)
            {
                mu_swprintf(Item.m_szNum, L"%d %ls", iValue, szText);
                Item.m_iNum = iValue;
            }
            else
            {
                mu_swprintf(Item.m_szNum, L"-");
            }

            // Period
            g_InGameShopSystem->GetProductInfoFromPriceSeq(iProductSeq, iPriceSeq, CInGameShopSystem::IGS_PRODUCT_ATT_TYPE_USE_LIMIT_PERIOD, iValue, szText);
            if (iValue > 0)
            {
                mu_swprintf(Item.m_szPeriod, L"%d %ls", iValue, szText);
            }
            else
            {
                mu_swprintf(Item.m_szPeriod, L"-");
            }

            g_InGameShopSystem->GetProductInfoFromPriceSeq(iProductSeq, iPriceSeq, CInGameShopSystem::IGS_PRODUCT_ATT_TYPE_ITEMCODE, iValue, szText);
            Item.m_wItemCode = iValue;
        }
        else
        {
            if (g_InGameShopSystem->GetProductInfoFromProductSeq(iProductSeq, CInGameShopSystem::IGS_PRODUCT_ATT_TYPE_ITEMNAME, iValue, Item.m_szName) == false)
                return;

            // Num
            g_InGameShopSystem->GetProductInfoFromProductSeq(iProductSeq, CInGameShopSystem::IGS_PRODUCT_ATT_TYPE_NUM, iValue, szText);
            if (iValue > 0)
            {
                mu_swprintf(Item.m_szNum, L"%d %ls", iValue, szText);
                Item.m_iNum = iValue;
            }
            else
            {
                mu_swprintf(Item.m_szNum, L"-");
            }

            // Period
            g_InGameShopSystem->GetProductInfoFromProductSeq(iProductSeq, CInGameShopSystem::IGS_PRODUCT_ATT_TYPE_USE_LIMIT_PERIOD, iValue, szText);
            if (iValue > 0)
            {
                mu_swprintf(Item.m_szPeriod, L"%d %ls", iValue, szText);
            }
            else
            {
                mu_swprintf(Item.m_szPeriod, L"-");
            }

            g_InGameShopSystem->GetProductInfoFromProductSeq(iProductSeq, CInGameShopSystem::IGS_PRODUCT_ATT_TYPE_ITEMCODE, iValue, szText);
            Item.m_wItemCode = iValue;
        }
    }
    else
    {
        return;
    }

    m_iStorageCurrentPageReceiveItemCnt++;

    m_StorageItems.Add(Item);
    m_StorageRowsDirty = true;
    // The native list box selected each row as it arrived; the pair of SLSetSelectLine calls
    // below then settled on the remembered one once the page was complete.
    m_StorageItems.SelectLast();
    m_StorageRowsDirty = true;

    if (m_iStorageCurrentPageReceiveItemCnt >= m_iStorageCurrentPageItemCnt)
    {
        if (m_iSelectedStorageItemIndex > m_iStorageCurrentPageItemCnt)
        {
            m_StorageItems.SelectRow(m_iStorageCurrentPageItemCnt - 1);
            m_StorageRowsDirty = true;
        }
        else
        {
            m_StorageItems.SelectRow(m_iSelectedStorageItemIndex - 1);
            m_StorageRowsDirty = true;
        }
    }
}

void CInGameShop::ClearAllStorageItem()
{
    m_iStorageTotalItemCnt = 0;
    m_iStorageCurrentPageItemCnt = 0;
    m_iStorageTotalPage = 0;
    m_iStorageCurrentPage = 0;
    m_iStorageCurrentPageReceiveItemCnt = 0;
    m_StorageItems.Clear();
    m_StorageRowsDirty = true;
}

void CInGameShop::InitStorage(int iTotalItemCnt, int iCurrentPageItemCnt, int iTotalPage, int iCurrentPage)
{
    ClearAllStorageItem();

    m_iStorageTotalItemCnt = iTotalItemCnt;
    m_iStorageCurrentPageItemCnt = iCurrentPageItemCnt;
    m_iStorageTotalPage = iTotalPage;

    if (m_iStorageTotalPage > 0)
    {
        m_iStorageCurrentPage = iCurrentPage;
    }
    else
    {
        m_iStorageCurrentPage = 0;
    }

    if (m_iSelectedStorageItemIndex == 0 || m_bRequestCurrentPage == false)
    {
        m_iSelectedStorageItemIndex = iCurrentPageItemCnt;
    }

    m_bRequestCurrentPage = false;
}

char CInGameShop::GetCurrentStorageCode()
{
    char szCode;
    switch (m_StorageBox)
    {
    case IGS_SAFEKEEPING_LISTBOX:
        szCode = 'S';
        break;
    case IGS_PRESENTBOX_LISTBOX:
        szCode = 'G';
        break;
    default:
        szCode = 'Z';
        break;
    }
    return szCode;
}

void CInGameShop::StoragePrevPage()
{
    if (m_iStorageCurrentPage > 1)
    {
        char szCode = GetCurrentStorageCode();
        m_iSelectedStorageItemIndex = 0;
        m_bRequestCurrentPage = true;
        SocketClient->ToGameServer()->SendCashShopStorageListRequest(m_iStorageCurrentPage - 1, szCode);
    }
}

void CInGameShop::StorageNextPage()
{
    if (m_iStorageCurrentPage < m_iStorageTotalPage)
    {
        char szCode = GetCurrentStorageCode();
        m_iSelectedStorageItemIndex = 0;
        m_bRequestCurrentPage = true;
        SocketClient->ToGameServer()->SendCashShopStorageListRequest(m_iStorageCurrentPage + 1, szCode);
    }
}

void CInGameShop::UpdateStorageItemList()
{
    char szCode = GetCurrentStorageCode();
    // SLGetSelectLineNum() was 1-based, and the arithmetic below still reads that way.
    const int iSelectLineIndex = m_StorageItems.SelectedRow() + 1;
    m_bRequestCurrentPage = true;

    if ((m_iStorageCurrentPageItemCnt == 1) && (m_iStorageTotalPage > 1))
    {
        m_iSelectedStorageItemIndex = 1;
        SocketClient->ToGameServer()->SendCashShopStorageListRequest(m_iStorageCurrentPage - 1, szCode);
    }
    else if (iSelectLineIndex == 1)
    {
        m_iSelectedStorageItemIndex = iSelectLineIndex;
        SocketClient->ToGameServer()->SendCashShopStorageListRequest(m_iStorageCurrentPage, szCode);
    }
    else if (m_iStorageCurrentPageItemCnt < IGS_STORAGE_TOTAL_ITEM_PER_PAGE)
    {
        m_iSelectedStorageItemIndex = (iSelectLineIndex - 1);
        SocketClient->ToGameServer()->SendCashShopStorageListRequest(m_iStorageCurrentPage, szCode);
    }
    else
    {
        m_iSelectedStorageItemIndex = iSelectLineIndex;
        SocketClient->ToGameServer()->SendCashShopStorageListRequest(m_iStorageCurrentPage, szCode);
    }
}

#endif //PBG_ADD_INGAMESHOP_UI_ITEMSHOP
