#if !defined(AFX_NEWUIINGAMESHOP_H__AE3CE531_70BE_4CBB_9938_0D80B26F21A8__INCLUDED_)
#define AFX_NEWUIINGAMESHOP_H__AE3CE531_70BE_4CBB_9938_0D80B26F21A8__INCLUDED_

#pragma once

#ifdef PBG_ADD_INGAMESHOP_UI_ITEMSHOP

#include "UI/Core/WindowObject.h"
#include "UI/Core/WindowManager.h"
#include "UI/Dialogs/MessageBox.h"
#include "UI/Dialogs/CommonMessageBox.h"
#include "Engine/Object/ZzzInventory.h"
#include "InGameShopSystem.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/Inventory/ItemCameraTarget.h"
#include "UI/RmlBridge/RmlElementTooltip.h"
#include "UI/RmlBridge/RmlThemedView.h"
#include "GameShop/StorageItemSelection.h"

#include <RmlUi/Core/Types.h>

#include <functional>
#include <string>
#include <vector>

namespace Rml
{
class ElementDocument;
}

namespace mu::ui::window
{
// The cash shop. in_game_shop.rml draws it; C++ keeps its state (the selected zone, category and
// storage box, the storage pages) and every request it sends, and draws the page's live 3D
// package items into the document.
class CInGameShop : public CObject
{
public:
    enum LISTBOX_INDEX
    {
        IGS_SAFEKEEPING_LISTBOX = 0,
        IGS_PRESENTBOX_LISTBOX,
        IGS_TOTAL_LISTBOX,
    };

private:
    enum
    {
        IGS_PANEL_WIDTH = 640,
        IGS_PANEL_HEIGHT = 429,
        IGS_NUM_ITEMS_WIDTH = 3,
        IGS_PACKAGE_PITCH_X = 122,
        IGS_PACKAGE_PITCH_Y = 121,
        IGS_ITEMRENDER_POS_X = 102,
        IGS_ITEMRENDER_POS_Y = 51,
        IGS_ITEMRENDER_WIDTH = 108,
        IGS_ITEMRENDER_HEIGHT = 58,
        IGS_STORAGE_TOTAL_ITEM_PER_PAGE = 9,
    };

public:
    CInGameShop();
    virtual ~CInGameShop();

    bool Create(CManager* pNewUIMng, int x, int y);

    void SetPos(int x, int y);
    const POINT& GetPos()
    {
        return m_Pos;
    }

    bool Render();
    bool Update();
    bool UpdateMouseEvent();
    bool UpdateKeyEvent();
    float GetLayerDepth()
    {
        return 10.08f;
    }

    void OpeningProcess();
    void ClosingProcess();

    void Release();

    bool IsInGameShopOpen();
    bool IsInGameShop();

    void InitZoneBtn();
    void InitCategoryBtn();

    void AddStorageItem(int iStorageSeq, int iStorageItemSeq, int iStorageGroupCode, int iProductSeq, int iPriceSeq,
                        int iCashPoint, wchar_t chItemType, wchar_t* pszUserName = NULL, wchar_t* pszMessage = NULL);

    void ClearAllStorageItem();

    void InitStorage(int iTotalItemCnt, int iCurrentPageItemCnt, int iTotalPage, int iCurrentPage);
    char GetCurrentStorageCode();

    void StoragePrevPage();
    void StorageNextPage();
    void UpdateStorageItemList();

    void InitBanner(wchar_t* pszFileName, wchar_t* pszBannerURL);
    void ReleaseBanner();

private:
    void Init();

    // The document's controls. Each runs on the next Update(), where the native buttons ran.
    void SelectZone(int index);
    void SelectCategory(int index);
    void SelectStorageBox(int index);
    void BuyPackage(int index);
    void UseStorageItem();
    void Close();
    // A previewed shop ($preview igs) sends nothing.
    bool SendsRequests() const;

    CManager* m_pNewUIMng;
    POINT m_Pos;

    int m_SelectedZone = 0;
    int m_SelectedCategory = 0;
    int m_StorageBox = IGS_SAFEKEEPING_LISTBOX;

    // The banner as the document loads it (relative to it), and where a click on it goes.
    std::string m_BannerSource;
    bool m_bBannerLink;
    wchar_t m_szBannerURL[INTERNET_MAX_URL_LENGTH];

    int m_iStorageTotalItemCnt;
    int m_iStorageCurrentPageItemCnt;
    int m_iStorageTotalPage;
    int m_iStorageCurrentPage;
    int m_iSelectedStorageItemIndex;

    int m_iStorageCurrentPageReceiveItemCnt;
    bool m_bRequestCurrentPage;

    GameShop::StorageItemSelection m_StorageItems;

    std::vector<std::function<void()>> m_PendingActions;

    // The page's packages, live 3D, into #igs_items under the 2-degree camera the shop always drew
    // them with.
    void RenderItems();
    UI::Items::ItemCameraTarget m_ItemTarget{[this](const Rml::Vector2f&, const Rml::Vector2f&) { RenderItems(); }, this};

    // The gift, charge, refresh and close buttons' hints.
    UI::RmlBridge::ElementTooltip m_Hint;
    void SyncHint();

    struct RadioEntry
    {
        Rml::String name;
        bool selected = false;
        bool last = false;

        bool operator==(const RadioEntry&) const = default;
    };
    struct PackageEntry
    {
        Rml::String name;
        Rml::String price;
        bool shown = false;

        bool operator==(const PackageEntry&) const = default;
    };
    struct WalletEntry
    {
        Rml::String label;
        Rml::String value;

        bool operator==(const WalletEntry&) const = default;
    };
    // One row of the storage / gift list. Where it sits is the theme's; only what it says and
    // whether it is the picked row travel through the model.
    struct StorageRow
    {
        Rml::String name;
        Rml::String period;
        bool selected = false;

        bool operator==(const StorageRow&) const = default;
    };
    struct InGameShopRmlModel
    {
        float rootX = 0.f, rootY = 0.f, rootScale = 1.f;
        float textPx = 0.f;
        Rml::String characterName;
        std::vector<WalletEntry> wallet;
        std::vector<RadioEntry> zones;
        std::vector<RadioEntry> categories;
        std::vector<PackageEntry> packages;
        Rml::String page, totalPages;
        std::vector<RadioEntry> storageTabs;
        std::vector<StorageRow> storageRows;
        Rml::String storagePage, storageTotalPages;
        Rml::String buyLabel, useLabel, itemNameLabel, durationLabel;
        Rml::String bannerSrc;
        bool bannerLinked = false;
        Rml::String scriptVersion, bannerVersion;
    };
    void BindRmlModel(Rml::DataModelConstructor& c, InGameShopRmlModel& model);
    UI::RmlBridge::ThemedView<InGameShopRmlModel> m_RmlView{"in_game_shop",
        [this](Rml::DataModelConstructor& c, InGameShopRmlModel& model) { BindRmlModel(c, model); },
        {{"Data/Interface/RmlUi/in_game_shop.rml"}}};
    bool m_StorageRowsDirty = true;

    void SyncRmlModel();
    void SyncStorageRows();
};
} // namespace mu::ui::window

#endif // PBG_ADD_INGAMESHOP_UI_ITEMSHOP
#endif // !defined(AFX_NEWUIINGAMESHOP_H__AE3CE531_70BE_4CBB_9938_0D80B26F21A8__INCLUDED_)
