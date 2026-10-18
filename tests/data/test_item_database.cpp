#include "doctest.h"

#include "Data/GameData/ItemData/ItemAttributeConversion.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemType.h"

#include "Core/Globals/_enum.h"

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

TEST_CASE("A weapon's group gives its weapon kind [data][items]")
{
    const auto groupWeaponKind = [](int group, ItemSlot slot, bool twoHanded)
    {
        ItemDefinition definition = MakeItem(group, 0, "Weapon");
        definition.slot = slot;
        definition.twoHanded = twoHanded;
        return GetGroupWeaponKind(definition);
    };

    CHECK(groupWeaponKind(3, ItemSlot::MainHand, true) == WeaponKind::Spear);
    CHECK(groupWeaponKind(6, ItemSlot::OffHand, false) == WeaponKind::Shield);
    CHECK(groupWeaponKind(4, ItemSlot::OffHand, true) == WeaponKind::Bow);
    CHECK(groupWeaponKind(4, ItemSlot::MainHand, true) == WeaponKind::Crossbow);
    // Arrows and bolts are one-handed.
    CHECK(groupWeaponKind(4, ItemSlot::MainHand, false) == WeaponKind::None);
    CHECK(groupWeaponKind(4, ItemSlot::OffHand, false) == WeaponKind::None);
    CHECK(groupWeaponKind(SwordGroup, ItemSlot::MainHand, false) == WeaponKind::Sword);
    CHECK(groupWeaponKind(1, ItemSlot::MainHand, true) == WeaponKind::Axe);
    CHECK(groupWeaponKind(2, ItemSlot::MainHand, false) == WeaponKind::Mace);
    // Group 5: staffs and sticks on mainHand, books on offHand.
    CHECK(groupWeaponKind(5, ItemSlot::MainHand, false) == WeaponKind::Staff);
    CHECK(groupWeaponKind(5, ItemSlot::OffHand, false) == WeaponKind::Book);
    CHECK(groupWeaponKind(5, ItemSlot::None, false) == WeaponKind::None);
    CHECK(groupWeaponKind(PotionGroup, ItemSlot::None, false) == WeaponKind::None);
    // Only on the slot of the kind.
    CHECK(groupWeaponKind(SwordGroup, ItemSlot::None, false) == WeaponKind::None);
    CHECK(groupWeaponKind(1, ItemSlot::OffHand, false) == WeaponKind::None);
    CHECK(groupWeaponKind(3, ItemSlot::None, true) == WeaponKind::None);
    CHECK(groupWeaponKind(6, ItemSlot::MainHand, false) == WeaponKind::None);

    // In group 0, an item only the Rage Fighter may use is one of his gloves.
    ItemDefinition glove = MakeItem(SwordGroup, 32, "Glove");
    glove.slot = ItemSlot::MainHand;
    glove.classRequirements[CLASS_RAGEFIGHTER] = 1;
    CHECK(GetGroupWeaponKind(glove) == WeaponKind::Glove);
    glove.classRequirements[CLASS_KNIGHT] = 1;
    CHECK(GetGroupWeaponKind(glove) == WeaponKind::Sword);
}

TEST_CASE("An edited weapon's kind follows its group while it is the group's [data][items]")
{
    ItemDefinition bolt = MakeItem(4, 7, "Bolt");
    bolt.slot = ItemSlot::OffHand;

    // A new item gets its group's kind.
    ItemDefinition spear = MakeItem(3, 100, "Spear");
    spear.slot = ItemSlot::MainHand;
    CHECK(FollowGroupWeaponKind(nullptr, spear) == WeaponKind::Spear);

    // Made two-handed, the bolt is a bow; one-handed again, it is none again.
    ItemDefinition twoHandedBolt = bolt;
    twoHandedBolt.twoHanded = true;
    twoHandedBolt.weaponKind = FollowGroupWeaponKind(&bolt, twoHandedBolt);
    CHECK(twoHandedBolt.weaponKind == WeaponKind::Bow);
    CHECK(FollowGroupWeaponKind(&twoHandedBolt, bolt) == WeaponKind::None);

    // A kind set in the item file that the group does not give stays.
    ItemDefinition shieldWithoutKind = MakeItem(6, 100, "Not a shield");
    shieldWithoutKind.slot = ItemSlot::OffHand;
    CHECK(FollowGroupWeaponKind(&shieldWithoutKind, shieldWithoutKind) == WeaponKind::None);
    ItemDefinition swordAsSpear = MakeItem(SwordGroup, 100, "Sword");
    swordAsSpear.weaponKind = WeaponKind::Spear;
    CHECK(FollowGroupWeaponKind(&swordAsSpear, swordAsSpear) == WeaponKind::Spear);
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

TEST_CASE("The item database counts its changes [data][items]")
{
    ItemDatabase database;
    database.Build(std::vector<ItemDefinition>{MakeItem(SwordGroup, 0, "Kris")});
    int version = database.GetVersion();

    // Lookups change nothing.
    CHECK(database.Find(SwordGroup, 0) != nullptr);
    CHECK(database.GetLogName(MakeItemType(SwordGroup, 0)) == "Kris (0,0)");
    CHECK(database.GetVersion() == version);

    // Every editor change and every build does.
    ItemDefinition changed = *database.Find(SwordGroup, 0);
    changed.names = MakeItem(SwordGroup, 0, "Dagger").names;
    database.Set(changed);
    CHECK(database.GetVersion() != version);
    version = database.GetVersion();
    database.Swap(MakeItemType(SwordGroup, 0), MakeItemType(SwordGroup, 5));
    CHECK(database.GetVersion() != version);
    version = database.GetVersion();
    database.Build({});
    CHECK(database.GetVersion() != version);
}

TEST_CASE("Item database answers tag, slot and rule questions [data][items]")
{
    ItemDefinition wing = MakeItem(12, 0, "Wings of Elf");
    wing.slot = ItemSlot::Wings;
    wing.wingTier = WingTier::First;
    wing.tags.Set(ItemTag::Valuable);
    wing.tradable = false;
    ItemDefinition shield = MakeItem(6, 0, "Small Shield");
    shield.slot = ItemSlot::OffHand;
    shield.weaponKind = WeaponKind::Shield;

    ItemDatabase database;
    database.Build(std::vector<ItemDefinition>{wing, shield});
    const int wingType = MakeItemType(12, 0);

    CHECK(database.HasTag(wingType, ItemTag::Valuable));
    CHECK_FALSE(database.HasTag(wingType, ItemTag::Jewel));
    CHECK(database.GetSlot(wingType) == ItemSlot::Wings);
    CHECK(database.GetWingTier(wingType) == WingTier::First);
    CHECK(database.GetWeaponKind(wingType) == WeaponKind::None);
    CHECK(database.GetWeaponKind(MakeItemType(6, 0)) == WeaponKind::Shield);
    CHECK_FALSE(database.IsAllowed(wingType, ItemAction::Trade));
    CHECK(database.IsAllowed(wingType, ItemAction::Drop));

    // Empty slots and invalid ids have no tags and allow no action.
    for (int itemType : {MakeItemType(12, 1), -1, MAX_ITEM})
    {
        CHECK_FALSE(database.HasTag(itemType, ItemTag::Valuable));
        CHECK(database.GetSlot(itemType) == ItemSlot::None);
        CHECK(database.GetWeaponKind(itemType) == WeaponKind::None);
        CHECK_FALSE(database.IsAllowed(itemType, ItemAction::Trade));
        CHECK_FALSE(database.IsAllowed(itemType, ItemAction::Repair));
    }
}

TEST_CASE("The item types with a tag come in use order: higher potion priority, then higher type [data][items]")
{
    std::vector<ItemDefinition> potions;
    for (const auto& [number, priority] : {std::pair{0, 2}, std::pair{1, 0}, std::pair{2, 2}, std::pair{3, 5}})
    {
        ItemDefinition potion = MakeItem(PotionGroup, number, "Potion");
        potion.tags.Set(ItemTag::HealingPotion);
        potion.potionPriority = static_cast<BYTE>(priority);
        potions.push_back(potion);
    }

    ItemDatabase database;
    database.Build(potions);
    const auto inUseOrder = [&database]
    {
        const auto itemTypes = database.GetItemTypesWithTagInUseOrder(ItemTag::HealingPotion);
        return std::vector<int>(itemTypes.begin(), itemTypes.end());
    };
    CHECK(inUseOrder() == std::vector<int>{MakeItemType(PotionGroup, 3), MakeItemType(PotionGroup, 2),
                                           MakeItemType(PotionGroup, 0), MakeItemType(PotionGroup, 1)});
    CHECK(database.GetItemTypesWithTagInUseOrder(ItemTag::ManaPotion).empty());
    CHECK(database.GetItemTypesWithTagInUseOrder(ItemTag::Count).empty());

    // Editor changes reorder the list.
    ItemDefinition changed = *database.Find(MakeItemType(PotionGroup, 1));
    changed.potionPriority = 9;
    database.Set(changed);
    CHECK(inUseOrder().front() == MakeItemType(PotionGroup, 1));
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

    const auto valuableTypes = [&database]
    {
        const auto itemTypes = database.GetItemTypesWithTag(ItemTag::Valuable);
        return std::vector<int>(itemTypes.begin(), itemTypes.end());
    };
    CHECK(valuableTypes().empty());

    ItemDefinition changed = *database.Find(krisType);
    changed.tags.Set(ItemTag::Valuable);
    changed.droppable = false;
    database.Set(changed);
    CHECK(database.HasTag(krisType, ItemTag::Valuable));
    CHECK_FALSE(database.IsAllowed(krisType, ItemAction::Drop));
    CHECK(valuableTypes() == std::vector<int>{krisType});

    database.Swap(krisType, MakeItemType(SwordGroup, 5));
    CHECK_FALSE(database.HasTag(krisType, ItemTag::Valuable));
    CHECK(database.IsAllowed(MakeItemType(SwordGroup, 5), ItemAction::Trade));
    CHECK(database.HasTag(MakeItemType(SwordGroup, 5), ItemTag::Valuable));
    CHECK_FALSE(database.IsAllowed(MakeItemType(SwordGroup, 5), ItemAction::Drop));
    CHECK(valuableTypes() == std::vector<int>{MakeItemType(SwordGroup, 5)});

    database.Build(std::vector<ItemDefinition>{MakeItem(SwordGroup, 0, "Kris")});
    CHECK(valuableTypes().empty());
}
