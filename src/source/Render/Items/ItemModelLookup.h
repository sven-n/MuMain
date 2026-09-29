#pragma once

#include "Core/Globals/_enum.h"
#include "Data/GameData/ItemData/ItemModelDatabase.h"

#include <span>
#include <vector>

namespace Render::Items
{
// The item model entry of a model slot (MODEL_ITEM + item type), or nullptr
// for slots that are not item models.
inline const Data::Items::ItemModelDefinition* FindItemModel(int modelType)
{
    return g_ItemModelDatabase.Find(modelType - MODEL_ITEM);
}

// The item a model slot is drawn like: its own item type, or for the second
// models of the Rage Fighter gloves (one per hand, drawn on the character)
// their glove. -1 for models that are not items.
inline int GetItemTypeOfModel(int modelType)
{
    static_assert(MODEL_SWORD_35_RIGHT - MODEL_SWORD_32_LEFT == 7 && ITEM_PHOENIX_SOUL_STAR - ITEM_SACRED_GLOVE == 3,
                  "a left and a right model for each of the four gloves");
    if (modelType >= MODEL_SWORD_32_LEFT && modelType <= MODEL_SWORD_35_RIGHT)
    {
        return ITEM_SACRED_GLOVE + (modelType - MODEL_SWORD_32_LEFT) / 2;
    }
    const int itemType = modelType - MODEL_ITEM;
    return Data::Items::IsValidItemType(itemType) ? itemType : -1;
}

// One value per item type, taken from the item model database after each
// build of it (the database counts its builds), e.g. the render style of
// every item. Drawing looks values up by model slot.
template <typename T> class ItemModelTable
{
public:
    using Take = T (*)(const Data::Items::ItemModelDefinition& model);

    explicit ItemModelTable(Take take) : m_take(take) {}

    // The value of the item a model slot is drawn like (GetItemTypeOfModel);
    // T{} for models that are not items and for slots without a model.
    T Find(int modelType)
    {
        const int itemType = GetItemTypeOfModel(modelType);
        return itemType >= 0 ? Values()[itemType] : T{};
    }

private:
    const std::vector<T>& Values()
    {
        const int databaseVersion = g_ItemModelDatabase.GetVersion();
        if (m_databaseVersion != databaseVersion)
        {
            m_databaseVersion = databaseVersion;
            const std::span<const Data::Items::ItemModelDefinition> models = g_ItemModelDatabase.GetAllSlots();
            m_values.assign(models.size(), T{});
            for (size_t itemType = 0; itemType < models.size(); ++itemType)
            {
                if (models[itemType].Exists())
                {
                    m_values[itemType] = m_take(models[itemType]);
                }
            }
        }
        return m_values;
    }

    Take m_take;
    int m_databaseVersion = -1;
    std::vector<T> m_values;
};
} // namespace Render::Items
