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
    public:
        enum IMAGE_LIST
        {
            // Frame/title/exit button moved to RmlUi (inventory_extension.rcss).
            // Numbered lock glyphs (formerly IMAGE_EXTENSION_NO1..4) moved to RmlUi too (see
            // LockedExtPageEntry below) -- only the locked page's table/empty-slot backing art stays native.
            IMAGE_EXTENSION_EMPTY = BITMAP_INTERFACE_NEW_INVENTORY_EXT_BEGIN,
            IMAGE_EXTENSION_TABLE,
        };

    private:
        // WIDTH/HEIGHT below are a pre-layout fallback only -- WindowGeometry's real hit-box comes
        // from #panel's own live RCSS size (UI::RmlBridge::RefreshLogicalPanelSize(), read at each
        // of UpdateMouseEvent()/InventoryProcess()'s own call sites), seeded with these only for the
        // first frame before RmlUi's layout has run. Distinct from HEIGHT_PER_EXT/EXT_BORDER below,
        // which size the native per-page item grid/table art and are NOT part of this fallback --
        // those stay native until the grid itself is separately migrated.
        static constexpr float WIDTH = 190.0f;
        static constexpr float HEIGHT = 429.0f;
        static constexpr float HEIGHT_PER_EXT = 87.0f;
        static constexpr float EXT_BORDER = 3.0f;

        CManager* m_pNewUIMng;
        CInventoryCtrl* m_extensions[MAX_INVENTORY_EXT_COUNT];
        POINT m_Pos;

        // Window frame/title/exit button are RmlUi; the extension grids stay native since their
        // icons are live 3D model renders (same reasoning as CMyInventory/CStorageInventoryExt).
        //
        // The locked (not-yet-purchased) pages' numbered lock glyph is a flat, decorative 2D
        // overlay with no live 3D content underneath (an unpurchased page holds no items), so it
        // moves here too as a data-for list -- one entry per locked page. Rebuilt unconditionally
        // whenever CharacterAttribute->InventoryExtensions could have changed (SyncRmlModel(),
        // same "rebuild every call" convention as CBuffStrip's buff list). The locked page's
        // table/empty-slot backing art (IMAGE_EXTENSION_TABLE/IMAGE_EXTENSION_EMPTY) stays native,
        // same reasoning as CInventoryCtrl's own per-cell chrome staying native everywhere else.
        struct LockedExtPageEntry
        {
            int number = 0;        // 1-based locked page number (matches former IMAGE_EXTENSION_NOn) -- modern theme renders this as a vector badge.
        };

        struct InventoryExtensionRmlModel
        {
            float rootX = 0.f, rootY = 0.f, rootScale = 1.f;
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
        UI::Items::ItemCameraTarget m_ItemTarget{[this](const Rml::Vector2f&, const Rml::Vector2f&) { RenderItems(); },
                                                 this};

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

        void LoadImages();
        void UnloadImages();

        bool InventoryProcess();

        void RenderFrame() const;
    };
}
