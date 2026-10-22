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
    ++m_version;
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

#ifdef _EDITOR
void ItemModelDatabase::ResolveGlowColors(const Effects::GlowColorList& glowColors)
{
    ++m_version;
    for (size_t itemType = 0; itemType < m_models.size(); ++itemType)
    {
        if (m_models[itemType].Exists())
        {
            m_glowColors[itemType] = FindColors(glowColors, m_models[itemType].glow);
        }
    }
}
#endif

void ItemModelDatabase::Set(const ItemModelDefinition& model, const Effects::GlowColorList& glowColors)
{
    if (!IsValidItemId(model.group, model.number))
    {
        return;
    }
    if (!model.Exists())
    {
        Remove(MakeItemType(model.group, model.number));
        return;
    }

    const int itemType = MakeItemType(model.group, model.number);
    ++m_version;
    m_modelCount += m_models[itemType].Exists() ? 0 : 1;
    m_models[itemType] = model;
    m_glowColors[itemType] = FindColors(glowColors, model.glow);
}

void ItemModelDatabase::Remove(int itemType)
{
    if (!IsValidItemType(itemType) || !m_models[itemType].Exists())
    {
        return;
    }
    ++m_version;
    --m_modelCount;
    m_models[itemType] = ItemModelDefinition{};
    m_glowColors[itemType] = ItemGlowColors{};
}

void ItemModelDatabase::SetSharedModels(std::span<const SharedItemModel> sharedModels)
{
    ++m_version;
    m_sharedModels.assign(sharedModels.begin(), sharedModels.end());
}

const SharedItemModel* ItemModelDatabase::FindSharedModel(std::string_view name) const
{
    const auto found = std::ranges::find(m_sharedModels, name, &SharedItemModel::name);
    return found != m_sharedModels.end() ? &*found : nullptr;
}

void ItemModelDatabase::SetSharedModel(const SharedItemModel& sharedModel)
{
    ++m_version;
    const auto found = std::ranges::find(m_sharedModels, sharedModel.name, &SharedItemModel::name);
    if (found != m_sharedModels.end())
    {
        *found = sharedModel;
        return;
    }
    m_sharedModels.push_back(sharedModel);
}

void ItemModelDatabase::RemoveSharedModel(std::string_view name)
{
    if (std::erase_if(m_sharedModels, [name](const SharedItemModel& model) { return model.name == name; }) > 0)
    {
        ++m_version;
    }
}

void ItemModelDatabase::Swap(int firstItemType, int secondItemType)
{
    if (!IsValidItemType(firstItemType) || !IsValidItemType(secondItemType) || firstItemType == secondItemType)
    {
        return;
    }

    ++m_version;
    std::swap(m_models[firstItemType], m_models[secondItemType]);
    std::swap(m_glowColors[firstItemType], m_glowColors[secondItemType]);
    for (const int itemType : {firstItemType, secondItemType})
    {
        ItemModelDefinition& model = m_models[itemType];
        if (model.Exists())
        {
            model.group = GetItemGroup(itemType);
            model.number = GetItemNumber(itemType);
        }
    }
}
} // namespace Data::Items
