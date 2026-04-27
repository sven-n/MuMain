//////////////////////////////////////////////////////////////////////
// NewUIComboBox.cpp: minimal click-to-open dropdown widget.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewUIComboBox.h"
#include "UIControls.h"  // for g_pRenderText macro

// From ZzzOpenglUtil.cpp -- viewport scaling helpers.
float ConvertX(float x);
float ConvertY(float y);

extern unsigned int WindowWidth, WindowHeight;
extern int MouseWheel;

using namespace SEASON3B;

namespace
{
    // Visual style for the combo widget. Tweaked to match the option window's
    // existing arrow-button look (see NewUIOptionWindow::RenderContents).
    constexpr float BG_BRIGHTNESS_IDLE     = 0.05f;
    constexpr float BG_BRIGHTNESS_HOVER    = 0.15f;
    constexpr float BG_BRIGHTNESS_OPEN     = 0.10f;  // closed field shade while dropdown is open
    constexpr float SCROLLBAR_TRACK_BRIGHT = 0.02f;
    constexpr float SCROLLBAR_THUMB_BRIGHT = 0.35f;

    constexpr int TEXT_PAD_X     = 6;   // left padding inside rows
    constexpr int TEXT_PAD_Y     = 4;   // top  padding inside rows
    constexpr int ARROW_WIDTH    = 14;  // width reserved for the dropdown-arrow glyph
    constexpr int SCROLLBAR_WIDTH = 6;  // vertical scrollbar width inside the list

    // Draws a solid-color axis-aligned rectangle in UI coordinates.
    // Matches the raw-GL pattern used by NewUIOptionWindow.
    void DrawSolidRect(int x, int y, int w, int h, float brightness)
    {
        glDisable(GL_TEXTURE_2D);

        float gx = ConvertX((float)x);
        float gy = ConvertY((float)y);
        float gw = ConvertX((float)w);
        float gh = ConvertY((float)h);
        gy = (float)WindowHeight - gy;

        glColor4f(brightness, brightness, brightness, 1.0f);
        glBegin(GL_QUADS);
            glVertex2f(gx,      gy);
            glVertex2f(gx + gw, gy);
            glVertex2f(gx + gw, gy - gh);
            glVertex2f(gx,      gy - gh);
        glEnd();

        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        glEnable(GL_TEXTURE_2D);
    }
}

void CNewUIComboBox::Setup(int x, int y, int width, int itemHeight,
                            const wchar_t* const* labels, int itemCount, int initialIdx,
                            int maxVisibleItems)
{
    m_X = x;
    m_Y = y;
    m_Width = width;
    m_ItemHeight = itemHeight;
    m_Labels = labels;
    m_ItemCount = itemCount;
    m_MaxVisibleItems = maxVisibleItems;
    m_ScrollOffset = 0;
    SetSelectedIndex(initialIdx);
    m_bOpen = false;
}

void CNewUIComboBox::SetSelectedIndex(int idx)
{
    if (m_ItemCount <= 0)
    {
        m_SelectedIndex = 0;
        return;
    }
    if (idx < 0) idx = 0;
    if (idx >= m_ItemCount) idx = m_ItemCount - 1;
    m_SelectedIndex = idx;
    ScrollToShowIndex(m_SelectedIndex);
}

int CNewUIComboBox::GetVisibleCount() const
{
    if (m_MaxVisibleItems <= 0 || m_MaxVisibleItems >= m_ItemCount)
        return m_ItemCount;
    return m_MaxVisibleItems;
}

int CNewUIComboBox::GetMaxScrollOffset() const
{
    const int visible = GetVisibleCount();
    return (m_ItemCount > visible) ? (m_ItemCount - visible) : 0;
}

bool CNewUIComboBox::IsScrollable() const
{
    return m_MaxVisibleItems > 0 && m_ItemCount > m_MaxVisibleItems;
}

void CNewUIComboBox::ClampScrollOffset()
{
    const int maxOffset = GetMaxScrollOffset();
    if (m_ScrollOffset < 0)            m_ScrollOffset = 0;
    if (m_ScrollOffset > maxOffset)    m_ScrollOffset = maxOffset;
}

void CNewUIComboBox::ScrollToShowIndex(int idx)
{
    if (!IsScrollable())
    {
        m_ScrollOffset = 0;
        return;
    }
    if (idx < m_ScrollOffset)
        m_ScrollOffset = idx;
    else if (idx >= m_ScrollOffset + m_MaxVisibleItems)
        m_ScrollOffset = idx - m_MaxVisibleItems + 1;
    ClampScrollOffset();
}

int CNewUIComboBox::GetItemIndexAtMouse() const
{
    if (!m_bOpen)
        return -1;

    const int listY = GetListY();
    const int visible = GetVisibleCount();
    // Scrollbar eats the right edge when present
    const int rowWidth = IsScrollable() ? (m_Width - SCROLLBAR_WIDTH) : m_Width;

    for (int row = 0; row < visible; row++)
    {
        const int itemY = listY + row * m_ItemHeight;
        if (CheckMouseIn(m_X, itemY, rowWidth, m_ItemHeight))
            return m_ScrollOffset + row;
    }
    return -1;
}

bool CNewUIComboBox::IsMouseOverWidget() const
{
    if (CheckMouseIn(m_X, m_Y, m_Width, m_ItemHeight))
        return true;
    if (m_bOpen && CheckMouseIn(m_X, GetListY(), m_Width, GetListHeight()))
        return true;
    return false;
}

bool CNewUIComboBox::UpdateMouseEvent()
{
    // Mouse-wheel scrolling inside the open dropdown (consumed so it doesn't
    // leak to other handlers like the volume sliders).
    if (m_bOpen && IsScrollable() && MouseWheel != 0 &&
        CheckMouseIn(m_X, GetListY(), m_Width, GetListHeight()))
    {
        // MouseWheel sign convention matches NewUIOptionWindow sliders:
        // positive = wheel up = scroll toward earlier items.
        if (MouseWheel > 0) m_ScrollOffset--;
        else                m_ScrollOffset++;
        ClampScrollOffset();
        MouseWheel = 0;
    }

    if (!SEASON3B::IsPress(VK_LBUTTON))
        return false;

    const bool clickedClosedField = CheckMouseIn(m_X, m_Y, m_Width, m_ItemHeight);

    // Clicking the closed field toggles the dropdown.
    if (clickedClosedField)
    {
        m_bOpen = !m_bOpen;
        if (m_bOpen)
            ScrollToShowIndex(m_SelectedIndex);
        return false;
    }

    if (!m_bOpen)
        return false;

    // Dropdown open: did the click land on an item?
    const int hitIdx = GetItemIndexAtMouse();
    if (hitIdx < 0)
    {
        // Click landed outside the list (or on the scrollbar column) -- close
        // without changing selection.
        m_bOpen = false;
        return false;
    }

    const bool changed = (hitIdx != m_SelectedIndex);
    m_SelectedIndex = hitIdx;
    m_bOpen = false;
    return changed;
}

void CNewUIComboBox::Render()
{
    if (m_ItemCount <= 0 || m_Labels == nullptr)
        return;

    // --- Closed field ---
    const bool hoverClosed = CheckMouseIn(m_X, m_Y, m_Width, m_ItemHeight);
    const float closedBrightness = m_bOpen ? BG_BRIGHTNESS_OPEN
                                            : (hoverClosed ? BG_BRIGHTNESS_HOVER
                                                           : BG_BRIGHTNESS_IDLE);
    DrawSolidRect(m_X, m_Y, m_Width, m_ItemHeight, closedBrightness);

    g_pRenderText->SetFont(g_hFont);
    g_pRenderText->SetBgColor(0);
    g_pRenderText->SetTextColor(255, 255, 255, 255);
    g_pRenderText->RenderText(m_X + TEXT_PAD_X, m_Y + TEXT_PAD_Y, m_Labels[m_SelectedIndex]);

    // Dropdown arrow glyph (plain text -- no asset needed)
    g_pRenderText->SetTextColor(255, 230, 200, 255);
    g_pRenderText->RenderText(m_X + m_Width - ARROW_WIDTH, m_Y + TEXT_PAD_Y, m_bOpen ? L"^" : L"v");

    // --- Expanded list (drawn on top of anything below) ---
    if (!m_bOpen)
        return;

    const int listY = GetListY();
    const int visible = GetVisibleCount();
    const bool hasScrollbar = IsScrollable();
    const int rowWidth = hasScrollbar ? (m_Width - SCROLLBAR_WIDTH) : m_Width;

    g_pRenderText->SetTextColor(255, 255, 255, 255);

    for (int row = 0; row < visible; row++)
    {
        const int absIdx = m_ScrollOffset + row;
        const int itemY = listY + row * m_ItemHeight;
        const bool hoverItem = CheckMouseIn(m_X, itemY, rowWidth, m_ItemHeight);
        const bool isSelected = (absIdx == m_SelectedIndex);

        float bright = BG_BRIGHTNESS_IDLE;
        if (hoverItem)      bright = BG_BRIGHTNESS_HOVER;
        else if (isSelected) bright = BG_BRIGHTNESS_OPEN;

        DrawSolidRect(m_X, itemY, rowWidth, m_ItemHeight, bright);
        g_pRenderText->RenderText(m_X + TEXT_PAD_X, itemY + TEXT_PAD_Y, m_Labels[absIdx]);
    }

    // --- Scrollbar (track + proportional thumb) ---
    if (hasScrollbar)
    {
        const int listHeight = GetListHeight();
        const int barX = m_X + m_Width - SCROLLBAR_WIDTH;

        // Track
        DrawSolidRect(barX, listY, SCROLLBAR_WIDTH, listHeight, SCROLLBAR_TRACK_BRIGHT);

        // Thumb: size proportional to (visible / total), position to (offset / maxOffset).
        int thumbHeight = (listHeight * visible) / m_ItemCount;
        if (thumbHeight < m_ItemHeight / 2)
            thumbHeight = m_ItemHeight / 2;

        const int maxOffset = GetMaxScrollOffset();
        const int thumbTravel = listHeight - thumbHeight;
        const int thumbY = listY + ((maxOffset > 0) ? (thumbTravel * m_ScrollOffset / maxOffset) : 0);

        DrawSolidRect(barX, thumbY, SCROLLBAR_WIDTH, thumbHeight, SCROLLBAR_THUMB_BRIGHT);
    }
}
