#pragma once

#include "ItemModelLookup.h"

#include "Data/GameData/ItemData/ItemModelDatabase.h"

#include <span>
#include <vector>

namespace Render::Items
{
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
