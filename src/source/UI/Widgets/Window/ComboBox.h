//////////////////////////////////////////////////////////////////////
// ComboBox.h: minimal click-to-open dropdown widget.
//////////////////////////////////////////////////////////////////////

#pragma once

namespace mu::ui::window
{
    /**
     * @brief Minimal click-to-open dropdown combo box, drawn with raw GL/RenderText (no ImGui).
     *
     * Does NOT take ownership of the label array -- the caller must keep it alive for the combo's lifetime.
     */
    class CComboBox
    {
    public:
        CComboBox() = default;
        ~CComboBox() = default;

        /**
         * @brief Configures the combo. Call once after the owning window is placed.
         * @param maxVisibleItems Max rows shown before the list becomes scrollable; 0 shows all.
         */
        void Setup(int x, int y, int width, int itemHeight,
                   const wchar_t* const* labels, int itemCount, int initialIdx,
                   int maxVisibleItems = 0);

        void SetPos(int x, int y) { m_X = x; m_Y = y; }
        void SetSelectedIndex(int idx);
        int  GetSelectedIndex() const { return m_SelectedIndex; }

        bool IsOpen() const { return m_bOpen; }
        void Close() { m_bOpen = false; }

        /** @brief True if the mouse is over the closed combo, or over the expanded dropdown when open. */
        bool IsMouseOverWidget() const;

        /**
         * @brief Processes one frame of mouse input.
         * @return true if the user picked a new item this frame (different index).
         */
        bool UpdateMouseEvent();

        /** @brief Draws the closed field, and the dropdown list if open. Call last so the dropdown sits on top. */
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
