#include "stdafx.h"

#include "doctest.h"

#include "Data/GameData/ItemData/ItemDataValidation.h"
#include "Data/GameData/ItemData/ItemNames.h"

#include <string>
#include <vector>

using namespace Data::Items;

namespace
{
ItemDefinition MakeNamedItem(int number, const char* names)
{
    ItemDefinition item;
    item.group = 0;
    item.number = number;
    item.names = Data::LocalizedString::Parse(names);
    return item;
}
} // namespace

TEST_CASE("An item has a name in a language when it has a translation, also one like the English name [data][items]")
{
    const ItemDefinition kris = MakeNamedItem(0, "Kris||pt=Kris||es=Daga");

    CHECK(HasNameIn(kris, "en"));
    CHECK(HasNameIn(kris, "es"));
    CHECK(HasNameIn(kris, "pt"));
    CHECK_FALSE(HasNameIn(kris, "de"));
    CHECK_FALSE(HasNameIn(MakeNamedItem(1, "||de=Klinge"), "en"));
    CHECK(GetOwnName(kris, "es") == "Daga");
    CHECK(GetOwnName(kris, "en") == "Kris");
    CHECK(GetOwnName(kris, "de").empty());
}

TEST_CASE("Item names are shown in the UI language, or in English while translated item names are off [data][items]")
{
    CHECK(GetItemNameLocale("de", true) == "de");
    CHECK(GetItemNameLocale("de", false) == "en");
    CHECK(GetItemNameLocale("en", false) == "en");
}

TEST_CASE("Item names are counted by language over the items that exist [data][items]")
{
    const std::vector<ItemDefinition> items = {MakeNamedItem(0, "Kris||pt=Kris"),
                                               MakeNamedItem(1, "Short Sword||de=Kurzschwert||pt=Espada curta"),
                                               MakeNamedItem(2, "")};
    const char* locales[] = {"en", "de", "pt", "pl"};

    const std::vector<ItemNameCount> counts = CountItemNames(items, locales);
    REQUIRE(counts.size() == 4);
    CHECK(counts[0].locale == "en");
    CHECK(counts[0].named == 2);
    CHECK(counts[0].missing == 0);
    CHECK(counts[1].named == 1);
    CHECK(counts[1].missing == 1);
    CHECK(counts[2].named == 2);
    CHECK(counts[2].missing == 0);
    CHECK(counts[3].named == 0);
    CHECK(counts[3].missing == 2);
}

TEST_CASE("An item name in a language the game has no texts for is a warning [data][items]")
{
    const std::vector<ItemDefinition> items = {MakeNamedItem(0, "Kris||de=Kris||fr=Kriss")};
    const char* uiLocales[] = {"en", "de"};

    std::vector<ItemDataIssue> issues;
    ValidateNameLocales(items, uiLocales, issues);
    REQUIRE(issues.size() == 1);
    CHECK(issues.front().severity == ItemDataIssueSeverity::Warning);
    CHECK(issues.front().message.find("fr") != std::string::npos);
}
