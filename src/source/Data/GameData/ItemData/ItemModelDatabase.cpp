#include "stdafx.h"

#include "ItemModelDatabase.h"

#include <algorithm>

namespace Data::Items
{
ItemModelDatabase& ItemModelDatabase::GetInstance()
{
    static ItemModelDatabase instance;
    return instance;
}

ItemModelDatabase::ItemModelDatabase() : m_models(MAX_ITEM) {}

void ItemModelDatabase::Build(std::span<const ItemModelDefinition> models)
{
    std::fill(m_models.begin(), m_models.end(), ItemModelDefinition{});
    m_modelCount = 0;
    for (const ItemModelDefinition& model : models)
    {
        if (!IsValidItemId(model.group, model.number))
        {
            continue;
        }

        ItemModelDefinition& slot = m_models[MakeItemType(model.group, model.number)];
        m_modelCount += slot.Exists() ? 0 : 1;
        slot = model;
    }
    ++m_version;
}
} // namespace Data::Items
