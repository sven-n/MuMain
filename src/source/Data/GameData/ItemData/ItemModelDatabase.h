#pragma once

#include "Data/GameData/EffectData/GlowColorList.h"
#include "Data/GameData/ItemData/ItemModelDefinition.h"
#include "Data/GameData/ItemData/ItemType.h"

#include <span>
#include <vector>

namespace Data::Items
{
// In-memory table of the item models (Data/Items/Models), indexed by item
// type like the item database. Built once at startup, before the models are
// opened.
class ItemModelDatabase
{
public:
    static ItemModelDatabase& GetInstance();

    ItemModelDatabase();

    // Replaces all models. Each model goes to its (group, number); models
    // with invalid ids are skipped (validation reports them). The glow colors
    // of the models are looked up in `glowColors`.
    void Build(std::span<const ItemModelDefinition> models, const Effects::GlowColorList& glowColors);

    // Returns nullptr for invalid ids and for items without a model.
    const ItemModelDefinition* Find(int itemType) const
    {
        if (!IsValidItemType(itemType))
        {
            return nullptr;
        }

        const ItemModelDefinition& model = m_models[itemType];
        return model.Exists() ? &model : nullptr;
    }

    // The glow colors of the model; nullptr for invalid ids and for items
    // without a model.
    const ItemGlowColors* FindGlowColors(int itemType) const
    {
        return Find(itemType) != nullptr ? &m_glowColors[itemType] : nullptr;
    }

    // All MAX_ITEM slots, indexed by item type, including empty ones.
    std::span<const ItemModelDefinition> GetAllSlots() const
    {
        return m_models;
    }

    int GetModelCount() const
    {
        return m_modelCount;
    }

private:
    std::vector<ItemModelDefinition> m_models;
    std::vector<ItemGlowColors> m_glowColors;
    int m_modelCount = 0;
};
} // namespace Data::Items

#define g_ItemModelDatabase Data::Items::ItemModelDatabase::GetInstance()
