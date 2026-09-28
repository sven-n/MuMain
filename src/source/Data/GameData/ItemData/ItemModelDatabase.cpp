#include "stdafx.h"

#include "ItemModelDatabase.h"

#include <algorithm>

namespace Data::Items
{
namespace
{
// Names that are not in the list (loading stops at them) have no color.
std::array<float, 3> FindColor(const Effects::GlowColorList& glowColors, const std::string& name)
{
    const Effects::GlowColorValue* value = glowColors.Find(name);
    if (value == nullptr)
    {
        return {};
    }
    return {static_cast<float>((*value)[0]), static_cast<float>((*value)[1]), static_cast<float>((*value)[2])};
}

ItemGlowColors FindColors(const Effects::GlowColorList& glowColors, const ItemGlow& glow)
{
    return {FindColor(glowColors, glow.color), FindColor(glowColors, glow.shineColor), glow.shineWhite,
            FindColor(glowColors, glow.ancientColor)};
}
} // namespace

ItemModelDatabase& ItemModelDatabase::GetInstance()
{
    static ItemModelDatabase instance;
    return instance;
}

ItemModelDatabase::ItemModelDatabase() : m_models(MAX_ITEM), m_glowColors(MAX_ITEM) {}

void ItemModelDatabase::Build(std::span<const ItemModelDefinition> models, const Effects::GlowColorList& glowColors)
{
    std::fill(m_models.begin(), m_models.end(), ItemModelDefinition{});
    std::fill(m_glowColors.begin(), m_glowColors.end(), ItemGlowColors{});
    m_modelCount = 0;
    for (const ItemModelDefinition& model : models)
    {
        if (!IsValidItemId(model.group, model.number))
        {
            continue;
        }

        const int itemType = MakeItemType(model.group, model.number);
        ItemModelDefinition& slot = m_models[itemType];
        m_modelCount += slot.Exists() ? 0 : 1;
        slot = model;
        m_glowColors[itemType] = FindColors(glowColors, model.glow);
    }
}
} // namespace Data::Items
