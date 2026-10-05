#pragma once

#include <cstdint>
#include <memory>

#include "UI/Core/WindowObject.h"
#include "UI/RmlBridge/RmlRenderTarget.h"

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
        bool GetHotKeyCommonItem(IN int iHotKey, OUT int& iStart, OUT int& iEnd);
        int GetHotKeyItemCount(int iType);
        ITEM* GetSlotItem(int iSlotIndex);
        // A target's drawer: one frame of the slot's item, framed for `width` x `height`.
        void RenderSlot(int iSlotIndex, std::uint32_t width, std::uint32_t height);

        int m_iHotKeyItemType[HOTKEY_COUNT];
        int m_iHotKeyItemLevel[HOTKEY_COUNT];
        std::unique_ptr<UI::RmlBridge::RenderTarget> m_SlotTargets[HOTKEY_COUNT];
        bool m_bSlotIconsShown = false;

        // -1 = nothing hovered; set by OnHotkeySlotHover(), cleared by OnUnhover().
        int m_iHoveredSlot = -1;
    };
}
