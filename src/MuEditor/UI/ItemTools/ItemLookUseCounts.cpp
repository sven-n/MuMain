#include "stdafx.h"

#ifdef _EDITOR

#include "ItemLookUseCounts.h"

#include "Data/GameData/ItemData/ItemModelDatabase.h"

namespace MuEditor::Items
{
int ItemLookUseCounts::SharedModel(std::string_view name)
{
    Update();
    return Find(m_sharedModels, name);
}

int ItemLookUseCounts::RenderStyle(std::string_view name)
{
    Update();
    return Find(m_renderStyles, name);
}

int ItemLookUseCounts::ItemEffect(std::string_view name)
{
    Update();
    return Find(m_itemEffects, name);
}

int ItemLookUseCounts::GlowColor(std::string_view name)
{
    Update();
    return Find(m_glowColors, name);
}

void ItemLookUseCounts::Update()
{
    if (m_version == g_ItemModelDatabase.GetVersion())
    {
        return;
    }
    m_version = g_ItemModelDatabase.GetVersion();
    m_sharedModels.clear();
    m_renderStyles.clear();
    m_itemEffects.clear();
    m_glowColors.clear();
    for (const Data::Items::ItemModelDefinition& model : g_ItemModelDatabase.GetAllSlots())
    {
        if (!model.Exists())
        {
            continue;
        }
        ++m_sharedModels[model.model];
        ++m_renderStyles[model.renderStyle];
        ++m_itemEffects[model.itemEffect];
        ++m_glowColors[model.glow.color];
        // A model with the same color in several places counts once for it.
        if (model.glow.shineColor != model.glow.color)
        {
            ++m_glowColors[model.glow.shineColor];
        }
        if (model.glow.ancientColor != model.glow.color && model.glow.ancientColor != model.glow.shineColor)
        {
            ++m_glowColors[model.glow.ancientColor];
        }
    }
}

int ItemLookUseCounts::Find(const Counts& counts, std::string_view name)
{
    const auto found = counts.find(name);
    return found != counts.end() ? found->second : 0;
}
} // namespace MuEditor::Items

#endif // _EDITOR
