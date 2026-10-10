#include "stdafx.h"

#include "UI/Core/WindowGeometry.h"

mu::ui::window::WindowGeometry::WindowGeometry(int x, int y, int width, int height)
{
    SetBounds(x, y, width, height);
}

void mu::ui::window::WindowGeometry::SetPosition(int x, int y)
{
    m_pos.x = x;
    m_pos.y = y;
}

void mu::ui::window::WindowGeometry::SetSize(int width, int height)
{
    m_size.cx = width;
    m_size.cy = height;
}

void mu::ui::window::WindowGeometry::SetBounds(int x, int y, int width, int height)
{
    SetPosition(x, y);
    SetSize(width, height);
}

RECT mu::ui::window::WindowGeometry::Bounds() const
{
    RECT rect;
    rect.left = m_pos.x;
    rect.top = m_pos.y;
    rect.right = m_pos.x + m_size.cx;
    rect.bottom = m_pos.y + m_size.cy;
    return rect;
}

bool mu::ui::window::WindowGeometry::Contains(int windowX, int windowY) const
{
    return windowX >= m_pos.x && windowX < m_pos.x + m_size.cx &&
           windowY >= m_pos.y && windowY < m_pos.y + m_size.cy;
}
