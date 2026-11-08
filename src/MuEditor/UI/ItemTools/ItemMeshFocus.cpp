#include "stdafx.h"

#ifdef _EDITOR

#include "ItemMeshFocus.h"

namespace MuEditor::Items
{
CItemMeshFocus& CItemMeshFocus::GetInstance()
{
    static CItemMeshFocus instance;
    return instance;
}

void CItemMeshFocus::BeginFrame(int itemType, std::optional<int> frame)
{
    if (itemType != m_itemType)
    {
        *this = CItemMeshFocus();
        m_itemType = itemType;
        m_frame = frame;
        return;
    }
    if (frame && frame == m_frame)
    {
        return;
    }
    m_frame = frame;
    m_pointedBefore = m_pointedNow;
    m_pointedNow.reset();
}

void CItemMeshFocus::Point(int mesh)
{
    m_pointedNow = mesh;
}

void CItemMeshFocus::ToggleKept(int mesh)
{
    m_kept = m_kept == mesh ? std::nullopt : std::optional<int>(mesh);
    m_keptBeforePick.reset();
}

void CItemMeshFocus::Pick(std::optional<int> mesh)
{
    m_keptBeforePick.emplace(m_kept);
    m_kept = mesh;
    m_picked = mesh.has_value();
}

void CItemMeshFocus::TakeBackPick()
{
    if (!m_keptBeforePick)
    {
        return;
    }
    m_kept = *m_keptBeforePick;
    m_keptBeforePick.reset();
    // Its row is scrolled to again.
    m_picked = m_kept.has_value();
}

void CItemMeshFocus::ForgetMeshesFrom(int meshCount)
{
    const auto forget = [meshCount](std::optional<int>& mesh)
    {
        if (mesh && *mesh >= meshCount)
        {
            mesh.reset();
        }
    };
    forget(m_pointedNow);
    forget(m_pointedBefore);
    forget(m_kept);
    if (m_keptBeforePick)
    {
        forget(*m_keptBeforePick);
    }
}

std::optional<int> CItemMeshFocus::Shown() const
{
    return m_pointedBefore ? m_pointedBefore : m_kept;
}

bool CItemMeshFocus::TakePicked()
{
    const bool picked = m_picked;
    m_picked = false;
    return picked;
}
} // namespace MuEditor::Items

#endif // _EDITOR
