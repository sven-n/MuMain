#pragma once

#include "Data/GameData/ItemData/ItemDatabase.h"

// Puts the item database's stat profiles back when it goes, so a test that sets its own does
// not leave them to the tests after it (ItemDatabase::Build keeps the profiles it has).
class ItemStatProfilesScope
{
public:
    ItemStatProfilesScope() : m_saved(g_ItemDatabase.GetStatProfiles()) {}

    ~ItemStatProfilesScope()
    {
        g_ItemDatabase.SetStatProfiles(m_saved);
    }

    ItemStatProfilesScope(const ItemStatProfilesScope&) = delete;
    ItemStatProfilesScope& operator=(const ItemStatProfilesScope&) = delete;

private:
    Data::Items::ItemStatProfiles m_saved;
};
