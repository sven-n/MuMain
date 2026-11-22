#include "stdafx.h"

#include "LookStore.h"

#include <algorithm>

namespace Data::Looks
{
LookStore g_LookStore;

namespace
{
const LookDefinition* FindIn(const std::vector<LookDefinition>& looks, std::string_view name)
{
    const auto found = std::ranges::lower_bound(looks, name, {}, &LookDefinition::name);
    return found != looks.end() && found->name == name ? &*found : nullptr;
}
} // namespace

void LookStore::SetRenderStyles(std::vector<LookDefinition> looks)
{
    SetLooks(LookKind::RenderStyle, std::move(looks));
}

void LookStore::SetEffects(std::vector<LookDefinition> looks)
{
    SetLooks(LookKind::Effect, std::move(looks));
}

void LookStore::SetLooks(LookKind kind, std::vector<LookDefinition> looks)
{
    std::ranges::sort(looks, {}, &LookDefinition::name);
    (kind == LookKind::Effect ? m_effects : m_renderStyles) = std::move(looks);
    ++m_version;
}

const LookDefinition* LookStore::FindRenderStyle(std::string_view name) const
{
    return FindIn(m_renderStyles, name);
}

const LookDefinition* LookStore::FindEffect(std::string_view name) const
{
    return FindIn(m_effects, name);
}

bool EffectRunsInUpdate(std::string_view name)
{
    const LookDefinition* effect = g_LookStore.FindEffect(name);
    return effect != nullptr && RunsInUpdate(*effect);
}
} // namespace Data::Looks
