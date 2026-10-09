#pragma once

#include "UI/Inventory/ItemCameraTarget.h"
#include "UI/Inventory/ItemGridModel.h"
#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/Core/WindowObject.h"
#include "UI/Inventory/InventoryCtrl.h"
#include "UI/RmlBridge/RmlThemedView.h"
#include <vector>

namespace Rml { class ElementDocument; }

namespace mu::ui::window
{
    class CInventoryExtension : public CObject
    {
    private:
        // The original's spacing between pages, for the grids' first placement.
        static constexpr float HEIGHT_PER_EXT = 87.0f;

        CManager* m_pNewUIMng;
        CInventoryCtrl* m_extensions[MAX_INVENTORY_EXT_COUNT];
        POINT m_Pos;

        // The grids stay native (live 3D items); a locked (not-yet-purchased) page is flat art the
        // theme draws, one entry per locked page.
        struct LockedExtPageEntry
        {
            int number = 0; // 1-based page number
        };

        struct InventoryExtensionRmlModel
        {
            float textPx = 0.f; // native text size in physical px (RmlRootTransform.h)
            Rml::String title;
            Rml::String exitTooltip;
            std::vector<LockedExtPageEntry> lockedPages;
            // The grids as their documents draw them (CInventoryCtrl::Cells()).
            UI::Items::ItemGridCells gridCells1;
            UI::Items::ItemGridCells gridCells2;
            UI::Items::ItemGridCells gridCells3;
            UI::Items::ItemGridCells gridCells4;
        };
        void BindRmlModel(Rml::DataModelConstructor& c, InventoryExtensionRmlModel& model);
        UI::RmlBridge::ThemedView<InventoryExtensionRmlModel> m_RmlView{"inventory_extension",
            [this](Rml::DataModelConstructor& c, InventoryExtensionRmlModel& model) { BindRmlModel(c, model); },
            {{"Data/Interface/RmlUi/inventory_extension.rml"}}};

        // The grids' items, into the document's #item_view.
        void RenderItems();
        UI::Items::ItemCameraTarget m_ItemTarget{[this](const Rml::Vector2f&, const Rml::Vector2f&) { RenderItems(); }};

        void BuildRmlUi();
        void SyncRmlModel();

    public:
        CInventoryExtension();
        virtual ~CInventoryExtension();

        bool Create(CManager* pNewUIMng, int x, int y);
        void Release();

        void SetPos(int x, int y);

        bool UpdateMouseEvent();
        bool UpdateKeyEvent();
        bool Update();
        bool Render();
        Rml::ElementDocument* GetPlacedDocument() const override { return m_RmlView.Document(); }

        float GetLayerDepth();	//. 2.5f
        ITEM* FindItem(int iIndex) const;
        bool InsertItem(int iIndex, std::span<const BYTE> pbyItemPacket) const;
        void DeleteItem(int iIndex) const;
        void DeleteAllItems() const;
        int FindEmptySlot(int cx, int cy, const CInventoryCtrl* excluded = nullptr) const;
        CInventoryCtrl* GetOwnerOf(const CPickedItem* pPickedItem) const;
        // The control holding that inventory index, or null: which extension
        // a slot belongs to is not derivable from the index alone, and
        // picking an item up needs the control it sits in.
        CInventoryCtrl* TryGetExtensionByInventoryIndex(int iIndex) const;

    private:
        void Init();

        // The pointer over the drawn panel.
        bool IsPointerOverPanel();
        bool InventoryProcess();
    };
}
