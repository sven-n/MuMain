#include "stdafx.h"

#include "LookStore.h"

#include <algorithm>

namespace Data::Looks
{
LookStore g_LookStore;

void LookStore::SetRenderStyles(std::vector<LookDefinition> looks)
{
    std::ranges::sort(looks, {}, &LookDefinition::name);
    m_renderStyles = std::move(looks);
    ++m_version;
}

const LookDefinition* LookStore::FindRenderStyle(std::string_view name) const
{
    const auto found = std::ranges::lower_bound(m_renderStyles, name, {}, &LookDefinition::name);
    return found != m_renderStyles.end() && found->name == name ? &*found : nullptr;
}
} // namespace Data::Looks
