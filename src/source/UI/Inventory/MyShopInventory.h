
#if !defined(AFX_NEWUIMYSHOPINVENTORY_H__A0C3DD4A_C4D5_4CF2_9702_DF54540DB6FD__INCLUDED_)
#define AFX_NEWUIMYSHOPINVENTORY_H__A0C3DD4A_C4D5_4CF2_9702_DF54540DB6FD__INCLUDED_

#pragma once

#include "UI/Inventory/ItemCameraTarget.h"
#include "UI/Inventory/ItemGridModel.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/Core/WindowObject.h"
#include "UI/Inventory/InventoryCtrl.h"
#include "UI/Dialogs/MessageBox.h"
#include "UI/Inventory/MyInventory.h"
#include "UI/RmlBridge/RmlThemedView.h"

namespace Rml { class ElementDocument; }

namespace mu::ui::window
{
    class CMyShopInventory : public CObject
    {
    public:
        enum SHOPTYEP
        {
            PERSONALSHOPSALE = 0,
            PERSONALSHOPPURCHASE,
        };

    private:

        CManager* m_pNewUIMng;
        CInventoryCtrl* m_pNewInventoryCtrl;
        POINT m_Pos;

        // Window frame/title/shop-title field/Open-Close-Exit buttons/instructional text (former
        // RenderTextInfo()) are RmlUi; the inventory grid stays native since its icons are live 3D
        // model renders (same reasoning as CMyInventory/CStorageInventoryExt).
        struct MyShopRmlModel
        {
            float textPx = 0.f; // native text size in physical px (RmlRootTransform.h)
            Rml::String title;

            // The editable shop name, two-way bound to my_shop.rml's <input data-value="shop_title">.
            // RmlUi owns the edit buffer/caret/selection/IME; this is only the committed value, read
            // back by GetTitle() and written by SetTitle()/ResetSubject(). Length is capped by the
            // input's own maxlength, set from iMAX_SHOPTITLE_MULTI in BuildRmlUi().
            Rml::String shopTitle;

            Rml::String exitTooltip;

            bool openLocked = false;
            Rml::String openTooltip;

            bool closeLocked = true;
            Rml::String closeTooltip;

            // Former RenderTextInfo() -- only stillOpening's visibility ever changes at runtime
            // (m_EnablePersonalShop), so every other line here is set once (BuildRmlUi()) and never
            // re-synced, same convention CCharacterInfoWindow's own static labels use. Colors are
            // plain RCSS now (my_shop.rcss's .myshop-line-* rules), not data-bound -- these are
            // fixed per-line decorative constants, not a live gameplay signal the way e.g.
            // CMixInventory's success-rate color is.
            bool showStillOpening = false;
            Rml::String stillOpeningText;

            Rml::String warningText;
            Rml::String sellingPriceText;
            Rml::String pleaseVerifyText;
            Rml::String alreadyInStoreText;
            Rml::String cancelSoldText;
            Rml::String cantBeReturnedText;
            Rml::String allItemTradingText;
            Rml::String canOnlyBeDoneUsingZenText;
            // The grids as their documents draw them (CInventoryCtrl::Cells()).
            UI::Items::ItemGridCells gridCells;
        };
        void BindRmlModel(Rml::DataModelConstructor& c, MyShopRmlModel& model);
        UI::RmlBridge::ThemedView<MyShopRmlModel> m_RmlView{"my_shop",
            [this](Rml::DataModelConstructor& c, MyShopRmlModel& model) { BindRmlModel(c, model); },
            {{"Data/Interface/RmlUi/my_shop.rml"}},
            {.afterBuild = [this] { ApplyShopTitleLimit(); }}};

        // The grids' items, into the document's #item_view.
        void RenderItems();
        UI::Items::ItemCameraTarget m_ItemTarget{[this](const Rml::Vector2f&, const Rml::Vector2f&) { RenderItems(); },
                                                 };

        // Mirrors the old CButton array's Lock()/tooltip-text state (OpenButtonLock()/UnLock(),
        // ChangePersonal()) now that the buttons themselves are RmlUi-owned.
        bool m_bOpenLocked;
        bool m_bOpenApplyTooltip; // true => "Apply" tooltip, false => "Open" tooltip

        void BuildRmlUi();
        void SyncRmlModel();
        void ApplyShopTitleLimit();
        void BlurShopTitleOnOutsideClick();

    public:
        CMyShopInventory();
        virtual ~CMyShopInventory();
        bool Create(CManager* pNewUIMng, int x, int y);
        void Release();
        void SetPos(int x, int y);
        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();
        Rml::ElementDocument* GetPlacedDocument() const override { return m_RmlView.Document(); }
        void ClosingProcess();
        float GetLayerDepth();	//. 3.2f

        CInventoryCtrl* GetInventoryCtrl() const;

    public:
        void ChangeSourceIndex(int sindex);
        const int GetSourceIndex();
        void ChangeTargetIndex(int tindex);
        const int GetTargetIndex();
        ITEM* FindItem(int iLinealPos);
        int GetItemInventoryIndex(ITEM* pItem);
        void ChangeTitle(wchar_t* titletext);
        void GetTitle(wchar_t* titletext);
        void SetTitle(wchar_t* titletext);
        void ChangePersonal(bool state);
        const bool IsEnablePersonalShop() const;
        void OpenButtonLock();
        void OpenButtonUnLock();
        int GetPointedItemIndex();
        void ResetSubject();
        bool IsEnableInputValueTextBox();
        void SetInputValueTextBox(bool bIsEnable);

    public:
        bool InsertItem(int iIndex, std::span<const BYTE> pbyItemPacket);
        void DeleteItem(int iIndex);
        void DeleteAllItems();

    private:
        // The pointer over the drawn panel.
        bool IsPointerOverPanel();
        bool MyShopInventoryProcess();
        bool WindowProcess();

    private:
        int					m_TargetIndex;
        int					m_SourceIndex;
        bool				m_EnablePersonalShop;
        bool				m_bIsEnableInputValueTextBox;
    };

    inline
        void CMyShopInventory::ChangeTitle(wchar_t* titletext)
    {
        SetTitle(titletext);
    }

    inline
        void CMyShopInventory::ChangeSourceIndex(int sindex)
    {
        m_SourceIndex = sindex;
    }

    inline
        void CMyShopInventory::ChangeTargetIndex(int tindex)
    {
        m_TargetIndex = tindex;
    }

    inline
        float CMyShopInventory::GetLayerDepth()
    {
        return 3.2f;
    }

    inline
        CInventoryCtrl* CMyShopInventory::GetInventoryCtrl() const
    {
        return m_pNewInventoryCtrl;
    }

    inline
        const int CMyShopInventory::GetSourceIndex()
    {
        return m_SourceIndex;
    }

    inline
        const int CMyShopInventory::GetTargetIndex()
    {
        return m_TargetIndex;
    }

    // Shows the "enter selling price" GenericConfirmDialog. Factored out as a free function since
    // 3 of its 4 call sites already live in this file and 1 lives in ZzzInventory.cpp. Caller must
    // already have ChangeSourceIndex()/ChangeTargetIndex() set for the item being priced.
    void ShowPersonalShopItemValueDialog();
}

#endif // !defined(AFX_NEWUIMYSHOPINVENTORY_H__A0C3DD4A_C4D5_4CF2_9702_DF54540DB6FD__INCLUDED_)
