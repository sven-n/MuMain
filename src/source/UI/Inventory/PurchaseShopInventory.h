
#if !defined(AFX_NEWUIPURCHASESHOPINVENTORY_H__5D417396_5ACE_46AF_9477_102810B6A1B8__INCLUDED_)
#define AFX_NEWUIPURCHASESHOPINVENTORY_H__5D417396_5ACE_46AF_9477_102810B6A1B8__INCLUDED_

#pragma once

#include "UI/Inventory/ItemCameraTarget.h"
#include "UI/Inventory/ItemGridModel.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/Core/WindowObject.h"
#include "UI/Inventory/InventoryCtrl.h"
#include "UI/RmlBridge/RmlThemedView.h"

namespace Rml { class ElementDocument; }

namespace mu::ui::window
{
    class CPurchaseShopInventory : public CObject
    {
    private:
        // INVENTORY_WIDTH/HEIGHT below are a pre-layout fallback only -- WindowGeometry's real
        // hit-box comes from #panel's own live RCSS size (UI::RmlBridge::RefreshLogicalPanelSize(),
        // read at WindowProcess()'s call site), seeded with these only for the first frame before
        // RmlUi's layout has run. Never referenced by the native purchase-grid rendering, which has
        // its own separate offset.
        enum
        {
            INVENTORY_WIDTH = 190,
            INVENTORY_HEIGHT = 429,
        };

    public:
        CPurchaseShopInventory();
        virtual ~CPurchaseShopInventory();

        bool Create(CManager* pNewUIMng, int x, int y);
        void Release();
        void SetPos(int x, int y);
        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();

        void ClosingProcess();

    public:
        float GetLayerDepth();	//. 3.2f
        CInventoryCtrl* GetInventoryCtrl() const;
        const int GetShopCharacterIndex();
        const std::wstring& GetTitleText();
        const int GetSourceIndex();
        int GetPointedItemIndex();

    public:
        void ChangeShopCharacterIndex(int index);
        void ChangeTitleText(wchar_t* text);
        void ChangeSourceIndex(int sindex);

    public:
        bool InsertItem(int iIndex, std::span<const BYTE> pbyItemPacket);
        void DeleteItem(int iIndex);
        ITEM* FindItem(int iLinealPos);
        int GetItemInventoryIndex(ITEM* pItem);

    private:
        bool PurchaseShopInventoryProcess();
        bool WindowProcess();

    private:
        CManager* m_pNewUIMng;
        CInventoryCtrl* m_pNewInventoryCtrl;
        POINT					m_Pos;
        int						m_ShopCharacterIndex;
       std::wstring		m_TitleText;
        int						m_SourceIndex;

        // Window frame/title/subtitle strip/warning text/exit button are RmlUi; the inventory grid
        // stays native since its icons are live 3D model renders (same reasoning as
        // CMyShopInventory/CStorageInventoryExt).
        struct PurchaseShopRmlModel
        {
            float rootX = 0.f, rootY = 0.f, rootScale = 1.f;
            float panelWidth = 190.f;
            float textPx = 0.f;         // native text size in physical px (RmlRootTransform.h)
            Rml::String title;			// static "Personal Store" label
            Rml::String shopOwnerText;	// dynamic shop-owner name (m_TitleText, via ChangeTitleText())
            Rml::String warningLabel;
            Rml::String sellingPriceLine;
            Rml::String verifyLine;
            Rml::String alreadyInStoreLine;
            Rml::String cancelPurchasedLine;
            Rml::String cantBeReturnedLine;
            Rml::String allItemTradingLine;
            Rml::String zenOnlyLine;
            // The grids as their documents draw them (CInventoryCtrl::Cells()).
            UI::Items::ItemGridCells gridCells;
        };
        void BindRmlModel(Rml::DataModelConstructor& c, PurchaseShopRmlModel& model);
        UI::RmlBridge::ThemedView<PurchaseShopRmlModel> m_RmlView{"purchase_shop",
            [this](Rml::DataModelConstructor& c, PurchaseShopRmlModel& model) { BindRmlModel(c, model); },
            {{"Data/Interface/RmlUi/purchase_shop.rml"}}};

        // The grids' items, into the document's #item_view.
        void RenderItems();
        UI::Items::ItemCameraTarget m_ItemTarget{[this](const Rml::Vector2f&, const Rml::Vector2f&) { RenderItems(); },
                                                 this};

        void BuildRmlUi();
        void SyncRmlModel();
    };

    inline
        void CPurchaseShopInventory::SetPos(int x, int y)
    {
        m_Pos.x = x; m_Pos.y = y;
    }

    inline
        void CPurchaseShopInventory::ChangeShopCharacterIndex(int index)
    {
        m_ShopCharacterIndex = index;
    }

    inline
        void CPurchaseShopInventory::ChangeTitleText(wchar_t* text)
    {
        m_TitleText = text;
    }

    inline
        void CPurchaseShopInventory::ChangeSourceIndex(int sindex)
    {
        m_SourceIndex = sindex;
    }

    inline
        CInventoryCtrl* CPurchaseShopInventory::GetInventoryCtrl() const
    {
        return m_pNewInventoryCtrl;
    }

    inline
        float CPurchaseShopInventory::GetLayerDepth()
    {
        return 3.2f;
    }

    inline
        const int CPurchaseShopInventory::GetShopCharacterIndex()
    {
        return m_ShopCharacterIndex;
    }

    inline
        const std::wstring& CPurchaseShopInventory::GetTitleText()
    {
        return m_TitleText;
    }

    inline
        const int CPurchaseShopInventory::GetSourceIndex()
    {
        return m_SourceIndex;
    }
};

#endif // !defined(AFX_NEWUIPURCHASESHOPINVENTORY_H__5D417396_5ACE_46AF_9477_102810B6A1B8__INCLUDED_)
