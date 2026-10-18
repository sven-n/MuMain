#pragma once

#include <cstdint>
#include <memory>
#include <span>

#include "UI/Core/WindowObject.h"
#include "UI/Inventory/ItemCameraTarget.h"

namespace Rml { class ElementDocument; }

namespace mu::ui::window
{
    enum
    {
        HOTKEY_Q = 0,
        HOTKEY_W,
        HOTKEY_E,
        HOTKEY_R,
        HOTKEY_COUNT
    };

    // The item types a hotkey takes, in the order it looks for them in the inventory: one
    // item type, or a list of the item database (valid until the items change). Keeps no
    // pointer to itself, so it can be copied.
    class HotKeyItemTypes
    {
    public:
        explicit HotKeyItemTypes(std::span<const int> itemTypes) : m_itemTypes(itemTypes) {}
        explicit HotKeyItemTypes(int itemType) : m_singleType(itemType), m_isSingle(true) {}

        const int* begin() const
        {
            return m_isSingle ? &m_singleType : m_itemTypes.data();
        }
        const int* end() const
        {
            return begin() + (m_isSingle ? 1 : m_itemTypes.size());
        }

    private:
        std::span<const int> m_itemTypes;
        int m_singleType = 0;
        bool m_isSingle = false;
    };

    // The item types a hotkey bound to boundType (at the item level boundLevel) takes.
    HotKeyItemTypes GetHotKeyItemTypes(int hotKey, int boundType, int boundLevel);

    class CItemHotKey
    {
    public:
        CItemHotKey();
        virtual ~CItemHotKey();

        bool UpdateKeyEvent();

        void SetHotKey(int iHotKey, int iItemType, int iItemLevel);
        int GetHotKey(int iHotKey);
        int GetHotKeyLevel(int iHotKey);

        // Each icon is a render target its slot's .item-icon shows, so RCSS places it like the rest
        // of the slot. Once a frame from SyncRmlModel(): sizes each target to its icon's on-screen
        // box and points the icon at it.
        void SyncSlotIcons(Rml::ElementDocument* document);
        // From SyncDocVisibility(), which runs whether or not this window updates.
        void SetSlotIconsShown(bool shown);

        // RmlUi handles hit-testing/hover for the 4 item-hotkey slots; sets a member read by
        // SyncRmlModel() and by the icon's own drawer.
        void OnHotkeySlotHover(int iSlotIndex) { m_iHoveredSlot = iSlotIndex; }
        void OnUnhover() { m_iHoveredSlot = -1; }
        int GetHoveredSlot() const { return m_iHoveredSlot; }
        void OnHotkeySlotRightClick(int iSlotIndex);

        // Stack count for the slot's bound item (0 if unbound/non-stacking); feeds #item_slots' stack-label binding.
        int GetSlotItemCount(int iSlotIndex);

    private:
        int GetHotKeyItemIndex(int iType, bool bItemCount = false);
        int GetHotKeyItemCount(int iType);
        ITEM* GetSlotItem(int iSlotIndex);
        // A target's drawer: one frame of the slot's item, framed for `width` x `height`.
        void RenderSlot(int iSlotIndex, const Rml::Vector2f& offset, const Rml::Vector2f& size);

        int m_iHotKeyItemType[HOTKEY_COUNT];
        int m_iHotKeyItemLevel[HOTKEY_COUNT];
        std::unique_ptr<UI::Items::ItemCameraTarget> m_SlotTargets[HOTKEY_COUNT];
        bool m_bSlotIconsShown = false;

        // -1 = nothing hovered; set by OnHotkeySlotHover(), cleared by OnUnhover().
        int m_iHoveredSlot = -1;
    };
}
