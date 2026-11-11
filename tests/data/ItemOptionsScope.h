#pragma once

#include "Data/GameData/ItemData/ItemDatabase.h"

// Puts the item database's option definitions back when it goes, so a test that sets its own does not leave them to
// the tests after it, also when one of its checks stops it.
class ItemOptionsScope
{
public:
    ItemOptionsScope() : m_saved(g_ItemDatabase.GetOptions()) {}

    ~ItemOptionsScope()
    {
        g_ItemDatabase.SetOptions(m_saved);
    }

    ItemOptionsScope(const ItemOptionsScope&) = delete;
    ItemOptionsScope& operator=(const ItemOptionsScope&) = delete;

    const Data::Items::ItemOptionDatabase& Saved() const
    {
        return m_saved;
    }

private:
    Data::Items::ItemOptionDatabase m_saved;
};
