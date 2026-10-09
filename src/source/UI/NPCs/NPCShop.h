
#if !defined(AFX_NEWUINPCSHOP_H__EEE639A8_C89E_47B3_8DBA_22560F102D98__INCLUDED_)
#define AFX_NEWUINPCSHOP_H__EEE639A8_C89E_47B3_8DBA_22560F102D98__INCLUDED_

#pragma once

#include "UI/Inventory/ItemCameraTarget.h"
#include "UI/Inventory/ItemGridModel.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/Core/WindowObject.h"
#include "UI/Inventory/InventoryCtrl.h"
#include "UI/Inventory/MyInventory.h"
#include "UI/RmlBridge/RmlThemedView.h"
#include <span>

namespace Rml { class ElementDocument; }

namespace mu::ui::window
{
    class CNPCShop : public CObject
    {
    public:
        enum
        {
            NPCSHOP_POS_X = 260,
            NPCSHOP_POS_Y = 0,
            SHOP_STATE_BUYNSELL = 1,
            SHOP_STATE_REPAIR = 2,
        };

    private:

        CManager* m_pNewUIMng;
        CInventoryCtrl* m_pNewInventoryCtrl;
        POINT m_Pos;

        DWORD m_dwShopState;
        int m_iTaxRate;
        bool m_bRepairShop;
        bool m_bIsNPCShopOpen;

        DWORD m_dwStandbyItemKey;

        bool m_bSellingItem;

        // Window frame/title/tax-rate line/repair buttons/repair-money strip are RmlUi; the
        // inventory grid stays native since its icons are live 3D model renders (same reasoning
        // as CMyInventory/CStorageInventoryExt).
        struct NPCShopRmlModel
        {
            float textPx = 0.f; // native text size in physical px (RmlRootTransform.h)

            Rml::String title;
            Rml::String taxRateText;

            bool repairVisible = false;
            Rml::String repairTooltip;
            Rml::String repairAllTooltip;

            // Repair-money strip: wallet-style like CMyInventory's gold strip, but showing
            // AllRepairGold (the cost to repair everything) instead of the character's own gold.
            Rml::String repairAllLabel;
            Rml::String repairGoldText;
            Rml::String repairGoldTier; // UI::RmlBridge::GoldTierKey() of the amount
            // The grids as their documents draw them (CInventoryCtrl::Cells()).
            UI::Items::ItemGridCells gridCells;
        };
        void BindRmlModel(Rml::DataModelConstructor& c, NPCShopRmlModel& model);
        UI::RmlBridge::ThemedView<NPCShopRmlModel> m_RmlView{"npc_shop",
            [this](Rml::DataModelConstructor& c, NPCShopRmlModel& model) { BindRmlModel(c, model); },
            {{"Data/Interface/RmlUi/npc_shop.rml"}}};

        // The grids' items, into the document's #item_view.
        void RenderItems();
        UI::Items::ItemCameraTarget m_ItemTarget{[this](const Rml::Vector2f&, const Rml::Vector2f&) { RenderItems(); }};

        void BuildRmlUi();
        void SyncRmlModel();

    public:
        CNPCShop();
        virtual ~CNPCShop();

        bool Create(CManager* pNewUIMng, int x, int y);
        void Release();

        void SetPos(int x, int y);

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();
        Rml::ElementDocument* GetPlacedDocument() const override { return m_RmlView.Document(); }

        float GetLayerDepth();	//. 2.5f

        void SetTaxRate(int iTaxRate);
        int GetTaxRate();

        bool InsertItem(int iIndex, std::span<const BYTE> pbyItemPacket);

        void OpenningProcess();
        void DeleteAllItems();

        void ClosingProcess();
        void SetRepairShop(bool bRepair);
        bool IsRepairShop();
        void ToggleState();
        DWORD GetShopState();

        int GetPointedItemIndex();

        //. Exporting Functions
        void SetStandbyItemKey(DWORD dwItemKey);
        DWORD GetStandbyItemKey() const;
        int GetStandbyItemIndex();
        ITEM* GetStandbyItem();

        void SetSellingItem(bool bFlag);
        bool IsSellingItem();

    private:
        void Init();

        bool InventoryProcess();
        bool WindowProcess();
    };
}

#endif // !defined(AFX_NEWUINPCSHOP_H__EEE639A8_C89E_47B3_8DBA_22560F102D98__INCLUDED_)
