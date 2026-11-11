#include "JewelUnmixSelection.h"

#include <algorithm>

namespace GameLogic::Items
{
bool JewelUnmixSelection::Update(std::span<const Entry> entries)
{
    if (std::equal(m_Entries.begin(), m_Entries.end(), entries.begin(), entries.end()))
        return false;
    m_Entries.assign(entries.begin(), entries.end());
    if (m_Selected && std::find(m_Entries.begin(), m_Entries.end(), *m_Selected) == m_Entries.end())
        m_Selected.reset();
    return true;
}

bool JewelUnmixSelection::Select(const Entry& entry)
{
    if (std::find(m_Entries.begin(), m_Entries.end(), entry) == m_Entries.end())
        return false;
    m_Selected = entry;
    return true;
}

void JewelUnmixSelection::Clear()
{
    m_Entries.clear();
    m_Selected.reset();
}
}
