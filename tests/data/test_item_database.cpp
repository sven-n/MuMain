#include "doctest.h"

#include "Data/GameData/ItemData/ItemAttributeConversion.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemType.h"

#include <string>
#include <vector>

using namespace Data::Items;

namespace
{
constexpr int SwordGroup = 0;
constexpr int PotionGroup = 14;
constexpr int LargeHealingPotionNumber = 3;

std::vector<ITEM_ATTRIBUTE> MakeEmptyAttributes()
{
    return std::vector<ITEM_ATTRIBUTE>(MAX_ITEM, ITEM_ATTRIBUTE{});
}

void SetName(ITEM_ATTRIBUTE& attribute, const std::wstring& name)
{
    name.copy(attribute.Name, name.size());
}

// Gives every field a different value, so a field the conversion forgets
// shows up as a mismatch.
ITEM_ATTRIBUTE MakeAttributeWithDistinctValues()
{
    ITEM_ATTRIBUTE attribute{};
    SetName(attribute, L"Kris");
    int value = 1;
#define SET_DISTINCT_VALUE(name, type, arraySize, width, i18nName) \
    attribute.name = static_cast<FIELD_TYPE_##type>(value++);
    ITEM_FIELDS_SIMPLE(SET_DISTINCT_VALUE)
#undef SET_DISTINCT_VALUE
    for (BYTE& requireClass : attribute.RequireClass)
    {
        requireClass = static_cast<BYTE>(value++);
    }
    for (BYTE& resistance : attribute.Resistance)
    {
        resistance = static_cast<BYTE>(value++);
    }
    return attribute;
}
} // namespace

TEST_CASE("Item types split into group and number [data][items]")
{
    const int itemType = MakeItemType(PotionGroup, LargeHealingPotionNumber);
    CHECK(itemType == PotionGroup * MAX_ITEM_INDEX + LargeHealingPotionNumber);
    CHECK(GetItemGroup(itemType) == PotionGroup);
    CHECK(GetItemNumber(itemType) == LargeHealingPotionNumber);

    CHECK(IsValidItemType(0));
    CHECK(IsValidItemType(MAX_ITEM - 1));
    CHECK_FALSE(IsValidItemType(-1));
    CHECK_FALSE(IsValidItemType(MAX_ITEM));

    CHECK(IsValidItemId(MAX_ITEM_TYPE - 1, MAX_ITEM_INDEX - 1));
    CHECK_FALSE(IsValidItemId(MAX_ITEM_TYPE, 0));
    CHECK_FALSE(IsValidItemId(0, MAX_ITEM_INDEX));
    CHECK_FALSE(IsValidItemId(-1, 0));
}

TEST_CASE("Item attribute conversion keeps every field [data][items]")
{
    const ITEM_ATTRIBUTE original = MakeAttributeWithDistinctValues();
    const int itemType = MakeItemType(PotionGroup, LargeHealingPotionNumber);

    const ItemDefinition definition = ToItemDefinition(original, itemType);
    CHECK(definition.group == PotionGroup);
    CHECK(definition.number == LargeHealingPotionNumber);
    CHECK(definition.name == L"Kris");

    ITEM_ATTRIBUTE converted{};
    ToItemAttribute(definition, converted);

    CHECK(std::wstring(converted.Name) == L"Kris");
#define CHECK_SAME_FIELD(name, type, arraySize, width, i18nName) CHECK(converted.name == original.name);
    ITEM_FIELDS_SIMPLE(CHECK_SAME_FIELD)
#undef CHECK_SAME_FIELD
    for (int i = 0; i < MAX_CLASS; ++i)
    {
        CHECK(converted.RequireClass[i] == original.RequireClass[i]);
    }
    for (int i = 0; i < MAX_RESISTANCE + 1; ++i)
    {
        CHECK(converted.Resistance[i] == original.Resistance[i]);
    }
}

TEST_CASE("Item names longer than the bmd field are cut [data][items]")
{
    ItemDefinition definition;
    definition.name = std::wstring(MAX_ITEM_NAME + 10, L'x');

    ITEM_ATTRIBUTE attribute{};
    ToItemAttribute(definition, attribute);

    CHECK(std::wstring(attribute.Name).size() == MAX_ITEM_NAME - 1);
}

TEST_CASE("Item database finds existing items only [data][items]")
{
    auto attributes = MakeEmptyAttributes();
    SetName(attributes[MakeItemType(SwordGroup, 0)], L"Kris");
    SetName(attributes[MakeItemType(PotionGroup, LargeHealingPotionNumber)], L"Large Healing Potion");

    ItemDatabase database;
    database.Build(attributes);

    CHECK(database.GetExistingItemCount() == 2);

    const ItemDefinition* potion = database.Find(PotionGroup, LargeHealingPotionNumber);
    REQUIRE(potion != nullptr);
    CHECK(potion->name == L"Large Healing Potion");
    CHECK(potion == database.Find(MakeItemType(PotionGroup, LargeHealingPotionNumber)));

    CHECK(database.Find(SwordGroup, 1) == nullptr);
    CHECK(database.Find(-1) == nullptr);
    CHECK(database.Find(MAX_ITEM) == nullptr);
    CHECK(database.Find(SwordGroup, MAX_ITEM_INDEX) == nullptr);
    CHECK(database.Find(MAX_ITEM_TYPE, 0) == nullptr);
}

TEST_CASE("Item log names use English names and the item id [data][items]")
{
    auto attributes = MakeEmptyAttributes();
    const int potionType = MakeItemType(PotionGroup, LargeHealingPotionNumber);
    SetName(attributes[MakeItemType(SwordGroup, 0)], L"Kris");
    SetName(attributes[potionType], L"Großer Heiltrank");

    std::vector<std::string> englishNames(MAX_ITEM);
    englishNames[potionType] = "Large Healing Potion";

    ItemDatabase database;
    database.Build(attributes, englishNames);

    CHECK(database.GetLogName(potionType) == "Large Healing Potion (14,3)");
    CHECK(database.Find(potionType)->name == L"Großer Heiltrank");

    // No English name for this item: the loaded name is used instead.
    CHECK(database.GetLogName(MakeItemType(SwordGroup, 0)) == "Kris (0,0)");

    CHECK(database.GetLogName(MakeItemType(SwordGroup, 1)) == "<unknown item> (0,1)");
    CHECK(database.GetLogName(-1) == "<invalid item type -1>");
}

TEST_CASE("Item log names fall back to the loaded names as UTF-8 [data][items]")
{
    auto attributes = MakeEmptyAttributes();
    const int potionType = MakeItemType(PotionGroup, LargeHealingPotionNumber);
    SetName(attributes[potionType], L"Großer Heiltrank");

    ItemDatabase database;
    database.Build(attributes);

    CHECK(database.GetLogName(potionType) == "Gro" "\xC3\x9F" "er Heiltrank (14,3)");
}

TEST_CASE("Rebuilding the item database replaces old items [data][items]")
{
    auto attributes = MakeEmptyAttributes();
    SetName(attributes[MakeItemType(SwordGroup, 0)], L"Kris");

    ItemDatabase database;
    database.Build(attributes);
    REQUIRE(database.Find(SwordGroup, 0) != nullptr);

    database.Build(MakeEmptyAttributes());
    CHECK(database.Find(SwordGroup, 0) == nullptr);
    CHECK(database.GetExistingItemCount() == 0);
}
