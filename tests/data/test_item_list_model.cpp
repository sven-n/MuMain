#include "stdafx.h"

#include "doctest.h"

#ifdef _EDITOR
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "UI/ItemEditor/ItemListModel.h"

#include <vector>

using namespace Data::Items;

namespace
{
ItemDefinition MakeNamedItem(int group, int number, const char* names)
{
    ItemDefinition item;
    item.group = group;
    item.number = number;
    item.names = Data::LocalizedString::Parse(names);
    return item;
}

// Fills the global item database for a test and puts the items it had back.
class GlobalItemsScope
{
public:
    explicit GlobalItemsScope(const std::vector<ItemDefinition>& items)
    {
        for (const ItemDefinition& slot : g_ItemDatabase.GetAllSlots())
        {
            if (slot.Exists())
            {
                m_previous.push_back(slot);
            }
        }
        g_ItemDatabase.Build(items);
    }

    ~GlobalItemsScope()
    {
        g_ItemDatabase.Build(m_previous);
    }

    GlobalItemsScope(const GlobalItemsScope&) = delete;
    GlobalItemsScope& operator=(const GlobalItemsScope&) = delete;

private:
    std::vector<ItemDefinition> m_previous;
};

const std::vector<ItemDefinition> Items = {
    MakeNamedItem(0, 0, "Kris||de=Kris||pt=Cris"),
    MakeNamedItem(0, 5, "Blade||de=Klinge||pt=L\xC3\xA2mina"),
    MakeNamedItem(14, 13, "Jewel of Bless||de=Juwel des Segens"),
};
} // namespace

TEST_CASE("The item list finds items by number, item type and any of their names [editor][items]")
{
    CHECK(MatchesItemSearch(Items[1], ""));
    CHECK(MatchesItemSearch(Items[1], "0,5"));
    CHECK_FALSE(MatchesItemSearch(Items[1], "0,4"));
    CHECK(MatchesItemSearch(Items[1], "5"));
    CHECK(MatchesItemSearch(Items[1], "klinge"));
    CHECK(MatchesItemSearch(Items[1], "BLADE"));
    CHECK(MatchesItemSearch(Items[1], "L\xC3\xA2min"));
    CHECK_FALSE(MatchesItemSearch(Items[1], "kris"));
}

TEST_CASE("The item list lists the items without a name in a language [editor][items]")
{
    GlobalItemsScope scope(Items);
    CItemListModel list;

    CHECK(list.GetItemTypes({}) == std::vector<int>{MakeItemType(0, 0), MakeItemType(0, 5), MakeItemType(14, 13)});
    CHECK(list.GetItemTypes({"", "pt"}) == std::vector<int>{MakeItemType(14, 13)});
    CHECK(list.GetItemTypes({"jewel", "de"}).empty());

    // A name filled through the list keeps its row until the list is refreshed.
    REQUIRE(list.GetItemTypes({"", "pt"}) == std::vector<int>{MakeItemType(14, 13)});
    ItemDefinition jewel = *g_ItemDatabase.Find(14, 13);
    jewel.names.Set("pt", "J\xC3\xB3ia da B\xC3\xAAn\xC3\xA7\xC3\xA3o");
    g_ItemDatabase.Set(jewel);
    list.KeepRowsAfterOwnEdit();
    CHECK(list.GetItemTypes({"", "pt"}) == std::vector<int>{MakeItemType(14, 13)});
    list.Refresh();
    CHECK(list.GetItemTypes({"", "pt"}).empty());

    // A change from elsewhere builds the list again.
    ItemDefinition kris = *g_ItemDatabase.Find(0, 0);
    kris.names.Set("pt", "");
    g_ItemDatabase.Set(kris);
    CHECK(list.GetItemTypes({"", "pt"}) == std::vector<int>{MakeItemType(0, 0)});
}
#endif // _EDITOR
