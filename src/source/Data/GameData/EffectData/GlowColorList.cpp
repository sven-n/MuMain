#include "stdafx.h"

#include "GlowColorList.h"

#include <algorithm>

namespace Data::Effects
{
GlowColorList& GlowColorList::GetInstance()
{
    static GlowColorList instance;
    return instance;
}

void GlowColorList::Build(std::span<const GlowColor> colors)
{
    m_colors.assign(colors.begin(), colors.end());
}

const GlowColorValue* GlowColorList::Find(std::string_view name) const
{
    const auto found =
        std::find_if(m_colors.begin(), m_colors.end(), [name](const GlowColor& color) { return color.name == name; });
    return found != m_colors.end() ? &found->value : nullptr;
}
} // namespace Data::Effects
