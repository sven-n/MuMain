#pragma once

#include "ItemModelLookup.h"

#include "Data/GameData/ItemData/ItemModelDatabase.h"
#include "Data/GameData/LookData/LookStore.h"

#include <memory>
#include <span>
#include <unordered_map>
#include <utility>
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

// What a table of the looks takes for some items and keeps for them: a look made for one item, as an item that sets
// values of its look draws it (design L12). Kept until the item models or the looks change, when the tables take
// their values again.
template <typename T> class ItemLooks
{
public:
    // Keeps a value made of `args` for the item, in its place (a look's style points at the look), and gives it.
    template <typename... Args> const T* Keep(int itemType, Args&&... args)
    {
        const std::pair<int, int> versions{g_ItemModelDatabase.GetVersion(), Data::Looks::g_LookStore.GetVersion()};
        if (m_builtFor != versions)
        {
            m_values.clear();
            m_builtFor = versions;
        }
        std::unique_ptr<T>& kept = m_values[itemType];
        kept = std::make_unique<T>(std::forward<Args>(args)...);
        return kept.get();
    }

private:
    std::unordered_map<int, std::unique_ptr<T>> m_values;
    std::pair<int, int> m_builtFor{-1, -1};
};
} // namespace Render::Items
