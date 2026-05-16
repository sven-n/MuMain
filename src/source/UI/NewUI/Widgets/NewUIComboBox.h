//////////////////////////////////////////////////////////////////////
// NewUIComboBox.h: minimal click-to-open dropdown widget.
//////////////////////////////////////////////////////////////////////

#pragma once

namespace SEASON3B
{
    /**
     * @brief Minimal click-to-open dropdown combo box.
     *
     * Renders using the same raw GL / RenderText primitives as the rest of the
     * game UI (no ImGui dependency) so it works in release builds. Designed to
     * be self-contained so the upcoming UI rewrite can drop or replace it
     * without ripple changes elsewhere.
     *
     * Typical usage:
     *   - Call Setup() once after the owning window is placed.
     *   - Each frame, call UpdateMouseEvent() -- returns true if a new item
     *     was selected this frame. Then call Render().
     *   - Owner should also treat IsMouseOverWidget() as "click consumed" so
     *     the expanded dropdown (which can overflow the owner's hit box)
     *     doesn't leak clicks to the game world.
     *
     * The combo does NOT take ownership of the label array -- the caller
     * must keep it alive for the combo's lifetime.
     */
    class CNewUIComboBox
    {
    public:
        CNewUIComboBox() = default;
        ~CNewUIComboBox() = default;

        /**
         * @brief Configures the combo. Call once after the owning window is placed.
         *
         * @param x,y              Top-left of the closed combo field (UI coordinates)
         * @param width            Combo width
         * @param itemHeight       Row height (used for both the closed field and list items)
         * @param labels           Array of item labels; caller owns the memory
         * @param itemCount        Number of labels
         * @param initialIdx       Initial selected index (clamped to [0, itemCount))
         * @param maxVisibleItems  Max rows shown in the expanded dropdown at once. When
         *                         `itemCount` exceeds this, the list becomes scrollable
         *                         (mouse wheel + scrollbar indicator). Pass 0 (default)
         *                         to always show the full list.
         */
        void Setup(int x, int y, int width, int itemHeight,
                   const wchar_t* const* labels, int itemCount, int initialIdx,
                   int maxVisibleItems = 0);

        void SetPos(int x, int y) { m_X = x; m_Y = y; }
        void SetSelectedIndex(int idx);
        int  GetSelectedIndex() const { return m_SelectedIndex; }

        bool IsOpen() const { return m_bOpen; }
        void Close() { m_bOpen = false; }

        /**
         * @brief True if the mouse is over the closed combo or (when open) over any
         * part of the expanded dropdown list. The owner should consume clicks in
         * this region so the overflowing dropdown doesn't leak to other UI or the
         * game world.
         */
        bool IsMouseOverWidget() const;

        /**
         * @brief Processes one frame of mouse input.
         * @return true if the user picked a new item this frame (different index).
         */
        bool UpdateMouseEvent();

        /**
         * @brief Draws the closed field always, and the dropdown list if open.
         * Call this last in the owner's render flow so the dropdown sits on top.
         */
        void Render();

    private:
        int m_X = 0;
        int m_Y = 0;
        int m_Width = 0;
        int m_ItemHeight = 0;

        const wchar_t* const* m_Labels = nullptr;
        int m_ItemCount = 0;
        int m_SelectedIndex = 0;
        bool m_bOpen = false;

        // Scrolling
        int m_MaxVisibleItems = 0;  // 0 = show all, no scrollbar
        int m_ScrollOffset = 0;     // index of the first visible row

        // Geometry helpers -- all use UI-space coordinates.
        int  GetVisibleCount() const;       // Rows actually rendered in the open list
        int  GetMaxScrollOffset() const;
        bool IsScrollable() const;
        int  GetListY() const { return m_Y + m_ItemHeight; }
        int  GetListHeight() const { return GetVisibleCount() * m_ItemHeight; }
        int  GetItemIndexAtMouse() const;   // -1 if no visible item hit
        void ClampScrollOffset();
        void ScrollToShowIndex(int idx);    // Ensure `idx` is in the visible window
    };
}
