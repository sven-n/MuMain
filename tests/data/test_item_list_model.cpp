#include "stdafx.h"

#include "doctest.h"

#ifdef _EDITOR
#include "ItemModelDatabaseScope.h"

#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemModelDatabase.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "UI/Common/TextSearch.h"
#include "UI/ItemEditor/ItemListModel.h"

#include <vector>

using namespace Data::Items;
using MuEditor::TextSearch::ContainsIgnoringCase;

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
    // The group counts, and a number alone is the item type, not the number.
    CHECK(MatchesItemSearch(Items[2], "14,13"));
    CHECK_FALSE(MatchesItemSearch(Items[2], "0,13"));
    CHECK(MatchesItemSearch(Items[2], "7181"));
    CHECK_FALSE(MatchesItemSearch(Items[2], "13"));
    CHECK_FALSE(MatchesItemSearch(Items[2], "14,"));
    // Only A to Z ignore case: an upper-case letter with an accent is another letter.
    CHECK_FALSE(MatchesItemSearch(Items[1], "L\xC3\x82MINA"));
    CHECK(MatchesItemSearch(Items[1], "klinge"));
    CHECK(MatchesItemSearch(Items[1], "BLADE"));
    CHECK(MatchesItemSearch(Items[1], "L\xC3\xA2min"));
    CHECK_FALSE(MatchesItemSearch(Items[1], "kris"));
}

// The item table's search uses it too.
TEST_CASE("The item search ignores case only for the letters A to Z [editor][items]")
{
    CHECK(ContainsIgnoringCase("Short Sword", "SWORD"));
    CHECK(ContainsIgnoringCase("Short Sword", ""));
    CHECK_FALSE(ContainsIgnoringCase("Short Sword", "swords"));
    CHECK(ContainsIgnoringCase("L\xC3\xA2mina", "l\xC3\xA2MIN"));
    CHECK_FALSE(ContainsIgnoringCase("L\xC3\xA2mina", "L\xC3\x82MINA"));
}

TEST_CASE("The item list lists the items without a name in a language [editor][items]")
{
    GlobalItemsScope scope(Items);
    CItemListModel list;

    CHECK(list.GetItemTypes({}) == std::vector<int>{MakeItemType(0, 0), MakeItemType(0, 5), MakeItemType(14, 13)});
    CHECK(list.GetItemTypes({"", "pt"}) == std::vector<int>{MakeItemType(14, 13)});
    CHECK(list.GetItemTypes({"jewel", ""}) == std::vector<int>{MakeItemType(14, 13)});
    CHECK(list.GetItemTypes({"7181", ""}) == std::vector<int>{MakeItemType(14, 13)});
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
TEST_CASE("The item list filters by group, tag, rule flag and missing model [editor][items]")
{
    std::vector<ItemDefinition> items = Items;
    items[1].tags.Set(ItemTag::Valuable);
    items[2].tradable = false;
    GlobalItemsScope scope(items);
    ItemModelDatabaseScope restoreModels;
    ItemModelDefinition kris;
    kris.file = "Data/Item/Sword01.bmd";
    // A model at a number without an item.
    ItemModelDefinition orphan;
    orphan.number = 30;
    orphan.file = "Data/Item/Sword31.bmd";
    g_ItemModelDatabase.Build(std::vector<ItemModelDefinition>{kris, orphan}, Data::Effects::GlowColorList{});
    CItemListModel list;

    CItemListModel::Filter filter;
    filter.group = 14;
    CHECK(list.GetItemTypes(filter) == std::vector<int>{MakeItemType(14, 13)});
    filter = {};
    filter.tag = ItemTag::Valuable;
    CHECK(list.GetItemTypes(filter) == std::vector<int>{MakeItemType(0, 5)});
    filter = {};
    filter.notAllowed = ItemAction::Trade;
    CHECK(list.GetItemTypes(filter) == std::vector<int>{MakeItemType(14, 13)});
    filter = {};
    filter.withoutModel = true;
    CHECK(list.GetItemTypes(filter) == std::vector<int>{MakeItemType(0, 5), MakeItemType(14, 13)});
    filter = {};
    filter.modelsWithoutItem = true;
    CHECK(list.GetItemTypes(filter) == std::vector<int>{MakeItemType(0, 30)});
    filter.search = "0,30";
    CHECK(list.GetItemTypes(filter) == std::vector<int>{MakeItemType(0, 30)});

    // A model change counts as a change of the list.
    filter = {};
    filter.withoutModel = true;
    REQUIRE(list.GetItemTypes(filter).size() == 2);
    ItemModelDefinition blade;
    blade.number = 5;
    blade.file = "Data/Item/Sword06.bmd";
    g_ItemModelDatabase.Set(blade, Data::Effects::GlowColorList{});
    CHECK(list.GetItemTypes(filter) == std::vector<int>{MakeItemType(14, 13)});
}

TEST_CASE("The item browser's All items lists the numbers with only a model too [editor][items]")
{
    GlobalItemsScope scope(Items);
    ItemModelDatabaseScope restoreModels;
    ItemModelDefinition orphan;
    orphan.number = 30;
    orphan.file = "Data/Item/Sword31.bmd";
    g_ItemModelDatabase.Build(std::vector<ItemModelDefinition>{orphan}, Data::Effects::GlowColorList{});
    const int orphanType = MakeItemType(0, 30);
    CItemListModel browser(CItemListModel::NumbersWithoutItem::InAllItems);
    CItemListModel translations;

    CItemListModel::Filter filter;
    CHECK(browser.GetItemTypes(filter) ==
          std::vector<int>{MakeItemType(0, 0), MakeItemType(0, 5), orphanType, MakeItemType(14, 13)});
    CHECK(translations.GetItemTypes(filter) ==
          std::vector<int>{MakeItemType(0, 0), MakeItemType(0, 5), MakeItemType(14, 13)});
    // The search and the group still apply; it is found by its number.
    filter.search = "0,30";
    CHECK(browser.GetItemTypes(filter) == std::vector<int>{orphanType});
    filter.search = "Kris";
    CHECK(browser.GetItemTypes(filter) == std::vector<int>{MakeItemType(0, 0)});
    filter = {};
    filter.group = 14;
    CHECK(browser.GetItemTypes(filter) == std::vector<int>{MakeItemType(14, 13)});
    // An item filter lists items only.
    filter = {};
    filter.withoutModel = true;
    CHECK(browser.GetItemTypes(filter) ==
          std::vector<int>{MakeItemType(0, 0), MakeItemType(0, 5), MakeItemType(14, 13)});
    filter = {};
    filter.missingLocale = "pt";
    CHECK(browser.GetItemTypes(filter) == std::vector<int>{MakeItemType(14, 13)});
}

TEST_CASE("The item browser lists the entries without a name among the numbers without an item [editor][items]")
{
    std::vector<ItemDefinition> items = Items;
    for (const int number : {40, 41})
    {
        ItemDefinition entry;
        entry.number = number;
        entry.entryWithoutName = true;
        items.push_back(entry);
    }
    GlobalItemsScope scope(items);
    ItemModelDatabaseScope restoreModels;
    // 0,40 has a model, as Sphere 4 and 5 have; 0,41 has none.
    ItemModelDefinition model;
    model.number = 40;
    model.file = "Data/Item/Sword31.bmd";
    g_ItemModelDatabase.Build(std::vector<ItemModelDefinition>{model}, Data::Effects::GlowColorList{});
    const int withModel = MakeItemType(0, 40);
    const int withoutModel = MakeItemType(0, 41);
    CItemListModel browser(CItemListModel::NumbersWithoutItem::InAllItems);
    CItemListModel translations;

    CItemListModel::Filter filter;
    CHECK(browser.GetItemTypes(filter) ==
          std::vector<int>{MakeItemType(0, 0), MakeItemType(0, 5), withModel, withoutModel, MakeItemType(14, 13)});
    CHECK(translations.GetItemTypes(filter) ==
          std::vector<int>{MakeItemType(0, 0), MakeItemType(0, 5), MakeItemType(14, 13)});
    filter.search = "0,41";
    CHECK(browser.GetItemTypes(filter) == std::vector<int>{withoutModel});
    // Models without an item: the game knows no item at 0,40.
    filter = {};
    filter.modelsWithoutItem = true;
    CHECK(browser.GetItemTypes(filter) == std::vector<int>{withModel});
    filter = {};
    filter.missingLocale = "pt";
    CHECK(browser.GetItemTypes(filter) == std::vector<int>{MakeItemType(14, 13)});

    // An entry takes its number.
    const std::vector<int> free = GetFreeItemNumbers(0);
    CHECK(std::ranges::find(free, 40) == free.end());
    CHECK(std::ranges::find(free, 41) == free.end());
}

TEST_CASE("A new item can take a number with neither an item nor a model [editor][items]")
{
    GlobalItemsScope scope(Items);
    ItemModelDatabaseScope restoreModels;
    ItemModelDefinition orphan;
    orphan.number = 30;
    orphan.file = "Data/Item/Sword31.bmd";
    g_ItemModelDatabase.Build(std::vector<ItemModelDefinition>{orphan}, Data::Effects::GlowColorList{});

    const std::vector<int> free = GetFreeItemNumbers(0);
    CHECK(free.size() == MAX_ITEM_INDEX - 3);
    CHECK(std::ranges::find(free, 0) == free.end());
    CHECK(std::ranges::find(free, 5) == free.end());
    CHECK(std::ranges::find(free, 30) == free.end());
    CHECK(std::ranges::find(free, 1) != free.end());
}

#endif // _EDITOR
