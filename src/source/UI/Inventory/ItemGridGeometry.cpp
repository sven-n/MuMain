#include "ItemGridGeometry.h"

#include <cmath>

namespace UI::Items
{
GridGeometry::GridGeometry(float left, float top, float pitchX, float pitchY, int columns, int rows)
    : m_Left(left), m_Top(top), m_PitchX(pitchX > 0.f ? pitchX : DefaultPitch),
      m_PitchY(pitchY > 0.f ? pitchY : DefaultPitch), m_Columns(columns), m_Rows(rows)
{
}

bool GridGeometry::Contains(float x, float y) const
{
    const GridRect box = Bounds();
    return x >= box.x && x < box.x + box.width && y >= box.y && y < box.y + box.height;
}

bool GridGeometry::CellAt(float x, float y, int& column, int& row) const
{
    if (!Contains(x, y))
        return false;
    column = static_cast<int>((x - m_Left) / m_PitchX);
    row = static_cast<int>((y - m_Top) / m_PitchY);
    if (column >= m_Columns)
        column = m_Columns - 1;
    if (row >= m_Rows)
        row = m_Rows - 1;
    return true;
}

void GridGeometry::CellOf(float x, float y, int& column, int& row) const
{
    if (CellAt(x, y, column, row))
        return;
    const float dx = x - m_Left;
    const float dy = y - m_Top;
    column = static_cast<int>(dx / m_PitchX) - (dx < 0.f ? 1 : 0);
    row = static_cast<int>(dy / m_PitchY) - (dy < 0.f ? 1 : 0);
}

GridRect GridGeometry::CellsRect(int column, int row, int width, int height) const
{
    return {m_Left + column * m_PitchX, m_Top + row * m_PitchY, width * m_PitchX, height * m_PitchY};
}

void AnchoredTopLeft(float pointerX, float pointerY, const PickupAnchor& anchor, float pitchX, float pitchY,
                     int& left, int& top)
{
    left = static_cast<int>(std::lround(pointerX - anchor.column * pitchX));
    top = static_cast<int>(std::lround(pointerY - anchor.row * pitchY));
}
} // namespace UI::Items
