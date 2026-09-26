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

ItemDefinition MakeItem(int group, int number, const std::string& names)
{
    ItemDefinition definition;
    definition.group = group;
    definition.number = number;
    definition.names = Data::LocalizedString::Parse(names);
    return definition;
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
    CHECK(definition.names.GetNeutral() == "Kris");

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
    const std::vector<ItemDefinition> items = {
        MakeItem(SwordGroup, 0, "Kris"),
        MakeItem(PotionGroup, LargeHealingPotionNumber, "Large Healing Potion"),
    };

    ItemDatabase database;
    database.Build(items);

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

TEST_CASE("Item names follow the display locale with English fallback [data][items]")
{
    const std::vector<ItemDefinition> items = {
        MakeItem(PotionGroup, LargeHealingPotionNumber, "Large Healing Potion||pt=Po\xC3\xA7\xC3\xA3o de Cura Grande"),
    };

    ItemDatabase database;
    database.Build(items);
    const ItemDefinition* potion = database.Find(PotionGroup, LargeHealingPotionNumber);
    REQUIRE(potion != nullptr);
    CHECK(potion->name == L"Large Healing Potion");

    database.SetDisplayLocale("pt");
    CHECK(potion->name == L"Poção de Cura Grande");

    database.SetDisplayLocale("de");
    CHECK(potion->name == L"Large Healing Potion");
}

TEST_CASE("Item log names use English names and the item id [data][items]")
{
    const std::vector<ItemDefinition> items = {
        MakeItem(SwordGroup, 0, "Kris||pt=Cris"),
    };

    ItemDatabase database;
    database.SetDisplayLocale("pt");
    database.Build(items);

    CHECK(database.GetLogName(MakeItemType(SwordGroup, 0)) == "Kris (0,0)");
    CHECK(database.GetLogName(MakeItemType(SwordGroup, 1)) == "<unknown item> (0,1)");
    CHECK(database.GetLogName(-1) == "<invalid item type -1>");
}

TEST_CASE("Rebuilding the item database replaces old items [data][items]")
{
    ItemDatabase database;
    database.Build(std::vector<ItemDefinition>{MakeItem(SwordGroup, 0, "Kris")});
    REQUIRE(database.Find(SwordGroup, 0) != nullptr);

    database.Build(std::vector<ItemDefinition>{MakeItem(SwordGroup, 1, "Short Sword")});
    CHECK(database.Find(SwordGroup, 0) == nullptr);
    CHECK(database.Find(SwordGroup, 1) != nullptr);
    CHECK(database.GetExistingItemCount() == 1);
}

TEST_CASE("Editor changes update single items [data][items]")
{
    ItemDatabase database;
    database.Build(std::vector<ItemDefinition>{MakeItem(SwordGroup, 0, "Kris")});

    ItemDefinition changed = *database.Find(SwordGroup, 0);
    changed.width = 2;
    database.Set(changed);
    CHECK(database.Find(SwordGroup, 0)->width == 2);

    // An item without names does not exist, but keeps its values.
    changed.names = {};
    database.Set(changed);
    CHECK(database.Find(SwordGroup, 0) == nullptr);
    CHECK(database.GetAllSlots()[MakeItemType(SwordGroup, 0)].width == 2);
    CHECK(database.GetExistingItemCount() == 0);
}

TEST_CASE("Swapping items moves them with their ids [data][items]")
{
    ItemDatabase database;
    database.Build(std::vector<ItemDefinition>{MakeItem(SwordGroup, 0, "Kris")});

    database.Swap(MakeItemType(SwordGroup, 0), MakeItemType(SwordGroup, 5));

    CHECK(database.Find(SwordGroup, 0) == nullptr);
    const ItemDefinition* moved = database.Find(SwordGroup, 5);
    REQUIRE(moved != nullptr);
    CHECK(moved->number == 5);
    CHECK(moved->names.GetNeutral() == "Kris");
}

TEST_CASE("Item database answers tag, slot and rule questions [data][items]")
{
    ItemDefinition wing = MakeItem(12, 0, "Wings of Elf");
    wing.slot = ItemSlot::Wings;
    wing.wingTier = WingTier::First;
    wing.tags.Set(ItemTag::Valuable);
    wing.tradable = false;

    ItemDatabase database;
    database.Build(std::vector<ItemDefinition>{wing});
    const int wingType = MakeItemType(12, 0);

    CHECK(database.HasTag(wingType, ItemTag::Valuable));
    CHECK_FALSE(database.HasTag(wingType, ItemTag::Jewel));
    CHECK(database.GetSlot(wingType) == ItemSlot::Wings);
    CHECK(database.GetWingTier(wingType) == WingTier::First);
    CHECK_FALSE(database.IsAllowed(wingType, ItemAction::Trade));
    CHECK(database.IsAllowed(wingType, ItemAction::Drop));

    // Empty slots and invalid ids have no tags and allow no action.
    for (int itemType : {MakeItemType(12, 1), -1, MAX_ITEM})
    {
        CHECK_FALSE(database.HasTag(itemType, ItemTag::Valuable));
        CHECK(database.GetSlot(itemType) == ItemSlot::None);
        CHECK_FALSE(database.IsAllowed(itemType, ItemAction::Trade));
        CHECK_FALSE(database.IsAllowed(itemType, ItemAction::Repair));
    }
}

TEST_CASE("Item tag sets test several tags at once [data][items]")
{
    const ItemTagSet socketItems{ItemTag::SocketSeed, ItemTag::SocketSphere};
    ItemTagSet seed;
    seed.Set(ItemTag::SocketSeed);

    CHECK(seed.HasAny(socketItems));
    CHECK_FALSE(ItemTagSet{ItemTag::Jewel}.HasAny(socketItems));
    CHECK_FALSE(ItemTagSet{}.HasAny(socketItems));
}

TEST_CASE("Item database rule data follows editor changes [data][items]")
{
    ItemDatabase database;
    database.Build(std::vector<ItemDefinition>{MakeItem(SwordGroup, 0, "Kris")});
    const int krisType = MakeItemType(SwordGroup, 0);

    ItemDefinition changed = *database.Find(krisType);
    changed.tags.Set(ItemTag::Valuable);
    changed.droppable = false;
    database.Set(changed);
    CHECK(database.HasTag(krisType, ItemTag::Valuable));
    CHECK_FALSE(database.IsAllowed(krisType, ItemAction::Drop));

    database.Swap(krisType, MakeItemType(SwordGroup, 5));
    CHECK_FALSE(database.HasTag(krisType, ItemTag::Valuable));
    CHECK(database.IsAllowed(MakeItemType(SwordGroup, 5), ItemAction::Trade));
    CHECK(database.HasTag(MakeItemType(SwordGroup, 5), ItemTag::Valuable));
    CHECK_FALSE(database.IsAllowed(MakeItemType(SwordGroup, 5), ItemAction::Drop));
}
