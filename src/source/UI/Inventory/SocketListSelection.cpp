#include "SocketListSelection.h"

#include <algorithm>

namespace UI::Inventory
{
bool SocketListSelection::Update(int recipe, std::span<const Option> options)
{
    if (m_Recipe == recipe && std::equal(m_Options.begin(), m_Options.end(), options.begin(), options.end()))
        return false;
    m_Recipe = recipe;
    m_Options.assign(options.begin(), options.end());
    ClearSelection();
    return true;
}

bool SocketListSelection::Select(int index)
{
    if (index < 0 || index >= static_cast<int>(m_Options.size()))
        return false;
    m_Selected = index;
    return true;
}
}
