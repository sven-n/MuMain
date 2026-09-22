#pragma once

#include "UI/Core/WindowObject.h"
#include "UI/Inventory/InventoryCtrl.h"
#include "UI/RmlBridge/RmlModelBinder.h"
#include <vector>

namespace Rml { class ElementDocument; }

namespace mu::ui::window
{
    class CInventoryExtension : public CObject
    {
    public:
        enum IMAGE_LIST
        {
            // Frame/title/exit button moved to RmlUi (inventory_extension.rcss/inventory_extension_bg.rcss).
            // Numbered lock glyphs (formerly IMAGE_EXTENSION_NO1..4) moved to RmlUi too (see
            // LockedExtPageEntry below) -- only the locked page's table/empty-slot backing art stays native.
            IMAGE_EXTENSION_EMPTY = BITMAP_INTERFACE_NEW_INVENTORY_EXT_BEGIN,
            IMAGE_EXTENSION_TABLE,
        };

    private:
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
            float top = 0.f;
            int number = 0;        // 1-based locked page number (matches former IMAGE_EXTENSION_NOn) -- modern theme renders this as a vector badge.
            Rml::String decorator; // "image(ext-lock-N)" -- legacy theme's raster sprite reference, built from `number`.
        };

        struct InventoryExtensionRmlModel
        {
            float rootX = 0.f, rootY = 0.f, rootScale = 1.f;
            Rml::String title;
            Rml::String exitTooltip;
            std::vector<LockedExtPageEntry> lockedPages;
        };
        RmlModelBinder<InventoryExtensionRmlModel> m_RmlBinder;
        Rml::ElementDocument* m_pRmlDoc = nullptr;

        // Frame background panel must render behind the grids' live 3D icons, but RmlUi's main
        // context always renders last -- so it goes through RmlUiRuntime's background context
        // instead (see CMyInventory/CStorageInventoryExt's identical *BgRmlModel for the mechanism).
        struct InventoryExtensionBgRmlModel
        {
            float rootX = 0.f, rootY = 0.f, rootScale = 1.f;
        };
        RmlModelBinder<InventoryExtensionBgRmlModel> m_BgRmlBinder;
        Rml::ElementDocument* m_pRmlBgDoc = nullptr;

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

        void ReloadRmlTheme();

        float GetLayerDepth();	//. 2.5f
        ITEM* FindItem(int iIndex) const;
        bool InsertItem(int iIndex, std::span<const BYTE> pbyItemPacket) const;
        void DeleteItem(int iIndex) const;
        void DeleteAllItems() const;
        int FindEmptySlot(int cx, int cy, const CInventoryCtrl* excluded = nullptr) const;
        CInventoryCtrl* GetOwnerOf(const CPickedItem* pPickedItem) const;
    private:
        void Init();

        void LoadImages();
        void UnloadImages();
        CInventoryCtrl* TryGetExtensionByInventoryIndex(int iIndex) const;

        bool InventoryProcess();

        void RenderFrame() const;
    };
}
