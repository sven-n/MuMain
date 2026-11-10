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
    // Another count of changes the values follow, e.g. of the looks that are data.
    using Version = int (*)();

    explicit ItemModelTable(Take take, Version version = nullptr) : m_take(take), m_version(version) {}

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
        const int version = m_version != nullptr ? m_version() : 0;
        if (m_databaseVersion != databaseVersion || m_otherVersion != version)
        {
            m_databaseVersion = databaseVersion;
            m_otherVersion = version;
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
    Version m_version;
    int m_databaseVersion = -1;
    int m_otherVersion = -1;
    std::vector<T> m_values;
};
} // namespace Render::Items
