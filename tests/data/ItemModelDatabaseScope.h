#pragma once

#include "Data/GameData/EffectData/GlowColorList.h"
#include "Data/GameData/ItemData/ItemModelDatabase.h"

// Empties the item model database, its shared models and its part models when the test ends,
// so the next test starts without the models this one built.
class ItemModelDatabaseScope
{
public:
    ItemModelDatabaseScope() = default;
    ~ItemModelDatabaseScope()
    {
        g_ItemModelDatabase.Build({}, Data::Effects::GlowColorList{});
        g_ItemModelDatabase.SetSharedModels({});
        g_ItemModelDatabase.SetPartModels({});
    }

    ItemModelDatabaseScope(const ItemModelDatabaseScope&) = delete;
    ItemModelDatabaseScope& operator=(const ItemModelDatabaseScope&) = delete;
};
