#include "doctest.h"

#include "Data/GameData/ItemData/ItemAttributeConversion.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemFileFit.h"
#include "Data/GameData/ItemData/ItemStatFile.h"
#include "Data/GameData/ItemData/ItemTooltipConditions.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "Data/GameData/ItemData/StatProfileRule.h"

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

// The profiles `names` (without terms of their own) and the profile rules `rules`.
ItemStatProfiles MakeStatProfiles(const std::vector<std::string>& names, std::vector<StatProfileRule> rules)
{
    ItemStatFile file;
    for (const std::string& name : names)
    {
        WrittenStatProfile profile;
        profile.name = name;
        file.profiles.push_back(profile);
    }
    file.profileRules = std::move(rules);
    return ItemStatProfiles(file);
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

// The editor rebuilds an item from its bmd attribute, which has no tooltip data: the fields the attribute lacks come
// from the item as it was, its description lines, level values and name lines whole.
TEST_CASE("Converting from the item attribute keeps the client-only fields [data][items]")
{
    ItemDefinition source = MakeItem(SwordGroup, 0, "Kris");
    ItemDescriptionLine written;
    written.text.Set("en", "Increase {percent}% of Damage");
    written.text.Set("de", "Erhöht den Schaden um {percent}%");
    written.values = {{"percent", "wingDamageIncrease1st"}};
    TooltipCondition condition;
    std::string error;
    REQUIRE(ReadTooltipCondition("level>=3", condition, error));
    written.when = {condition};
    written.color = ItemTextColor::Blue;
    written.minLevel = 3;
    written.maxLevel = 9;
    ItemDescriptionLine shared;
    shared.textName = "cannotRepair";
    shared.color = ItemTextColor::Red;
    source.description = {written, ItemDescriptionLine{}, shared};
    ItemLevelValue stat;
    stat.text.Set("en", "ENG");
    ItemLevelValue sharedStat;
    sharedStat.textName = "stamina";
    source.levelValues = {{"stat", {stat, sharedStat}}};
    ItemNameLine olive;
    olive.text.Set("en", "Olive of love");
    olive.hasText = true;
    olive.color = ItemTextColor::Yellow;
    olive.hasColor = true;
    olive.minLevel = 4;
    olive.maxLevel = 4;
    source.nameLines = {olive};
    source.statProfile = "weapon";

    // As the bmd import makes it: the attribute's fields, nothing of the tooltip.
    ItemDefinition target = MakeItem(SwordGroup, 0, "Kris of the bmd");
    target.damageMin = 99;
    CopyFieldsNotInItemAttribute(source, target);

    CHECK(target.description == source.description);
    CHECK(target.levelValues == source.levelValues);
    CHECK(target.nameLines == source.nameLines);
    CHECK(target.statProfile == "weapon");
    // The attribute's own fields stay the imported ones.
    CHECK(target.names.GetNeutral() == "Kris of the bmd");
    CHECK(target.damageMin == 99);
}

TEST_CASE("Damage and magic power past 255 are kept, but do not fit the bmd item file [data][items]")
{
    ItemDefinition definition = MakeItem(SwordGroup, 51, "Later Sword");
    definition.damageMin = 345;
    definition.damageMax = 380;
    definition.magicPower = 276;

    ITEM_ATTRIBUTE attribute{};
    ToItemAttribute(definition, attribute);
    CHECK(attribute.DamageMin == 345);
    CHECK(attribute.DamageMax == 380);
    CHECK(attribute.MagicPower == 276);
    CHECK(ToItemDefinition(attribute, MakeItemType(SwordGroup, 51)).damageMax == 380);

    // Each value past 255 is named, with its item and the file's limit.
    std::vector<ITEM_ATTRIBUTE> items(3);
    items[1] = attribute;
    const std::vector<ItemFileMisfit> misfits = FindItemFileMisfits(items.data(), static_cast<int>(items.size()));
    REQUIRE(misfits.size() == 3);
    CHECK(misfits[0].itemType == 1);
    CHECK(std::string(misfits[0].field) == "DamageMin");
    CHECK(misfits[0].value == 345);
    CHECK(misfits[0].limit == 255);
    CHECK(std::string(misfits[1].field) == "DamageMax");
    CHECK(std::string(misfits[2].field) == "MagicPower");
    CHECK(DescribeItemFileMisfits({misfits[0]}) == "Item (0,1) DamageMin is 345; a .bmd item file holds up to 255\n");

    // Each field on its own.
    for (WORD ITEM_ATTRIBUTE::* field :
         {&ITEM_ATTRIBUTE::DamageMin, &ITEM_ATTRIBUTE::DamageMax, &ITEM_ATTRIBUTE::MagicPower})
    {
        ITEM_ATTRIBUTE one{};
        one.*field = 256;
        const std::vector<ItemFileMisfit> alone = FindItemFileMisfits(&one, 1);
        REQUIRE(alone.size() == 1);
        CHECK(alone[0].value == 256);
        one.*field = 255;
        CHECK(FindItemFileMisfits(&one, 1).empty());
    }

    attribute.DamageMin = 255;
    attribute.DamageMax = 255;
    attribute.MagicPower = 255;
    CHECK(FindItemFileMisfits(&attribute, 1).empty());
    ITEM_ATTRIBUTE_FILE file{};
    CopyItemAttributeToDestination(file, attribute);
    CHECK(file.DamageMax == 255);
    CHECK(sizeof(file.DamageMax) == 1);
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

TEST_CASE("Another language of the item names raises the name version but not the item version [data][items]")
{
    ItemDatabase database;
    database.Build(std::vector<ItemDefinition>{MakeItem(SwordGroup, 0, "Kris||pt=Cris")});
    const int version = database.GetVersion();
    const int nameVersion = database.GetNameVersion();

    database.SetDisplayLocale("en");
    CHECK(database.GetNameVersion() == nameVersion);

    database.SetDisplayLocale("pt");
    CHECK(database.GetNameVersion() != nameVersion);
    CHECK(database.GetVersion() == version);

    const int ptNameVersion = database.GetNameVersion();
    database.Set(*database.Find(SwordGroup, 0));
    CHECK(database.GetNameVersion() != ptNameVersion);
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

TEST_CASE("A requirement given by level can be taken back [data][items]")
{
    RequirementsByLevel byLevel;
    RequirementByLevel energy{};
    energy.fill(30);
    byLevel.Set(ItemRequirement::Energy, energy);
    byLevel.Set(ItemRequirement::Level, energy);
    byLevel.Remove(ItemRequirement::Energy);
    CHECK(byLevel.Find(ItemRequirement::Energy) == nullptr);
    REQUIRE(byLevel.Find(ItemRequirement::Level) != nullptr);
    byLevel.Remove(ItemRequirement::Energy);
    byLevel.Remove(ItemRequirement::Level);
    CHECK(byLevel == RequirementsByLevel{});
}

TEST_CASE("Editor changes to many items at once update them and the tag lists [data][items]")
{
    ItemDatabase database;
    database.Build(
        std::vector<ItemDefinition>{MakeItem(SwordGroup, 0, "Kris"), MakeItem(SwordGroup, 1, "Short Sword")});
    const int version = database.GetVersion();

    std::vector<ItemDefinition> changed = {*database.Find(SwordGroup, 0), *database.Find(SwordGroup, 1),
                                           MakeItem(SwordGroup, 2, "Rapier")};
    for (ItemDefinition& definition : changed)
    {
        definition.tags.Set(ItemTag::Valuable);
    }
    changed[1].names = {};
    database.SetMany(changed);

    CHECK(database.HasTag(MakeItemType(SwordGroup, 0), ItemTag::Valuable));
    CHECK(database.Find(SwordGroup, 1) == nullptr);
    CHECK(database.Find(SwordGroup, 2) != nullptr);
    CHECK(database.GetExistingItemCount() == 2);
    const auto valuable = database.GetItemTypesWithTag(ItemTag::Valuable);
    CHECK(std::vector<int>(valuable.begin(), valuable.end()) ==
          std::vector<int>{MakeItemType(SwordGroup, 0), MakeItemType(SwordGroup, 2)});
    CHECK(database.GetVersion() != version);
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

TEST_CASE("Item database resolves the stat profile of every item [data][items]")
{
    ItemStatFile file;
    file.tables.push_back(LevelBonusTable{"linear", {}, {}, 1});
    WrittenStatProfile shield{"shield", {}};
    shield.stats[static_cast<size_t>(ItemStat::Defense)].table = "linear";
    file.profiles.push_back(shield);

    ItemDefinition smallShield = MakeItem(6, 0, "Small Shield");
    smallShield.statProfile = "shield";
    ItemDefinition shortSword = MakeItem(0, 1, "Short Sword");
    shortSword.statProfile = "missing";
    const int smallShieldType = MakeItemType(6, 0);
    const int krisType = MakeItemType(0, 0);
    const int shortSwordType = MakeItemType(0, 1);

    ItemDatabase database;
    database.Build(std::vector<ItemDefinition>{smallShield, MakeItem(0, 0, "Kris"), shortSword});
    const auto defenseBonus = [&database](int itemType, int level)
    {
        const ItemStatProfiles& profiles = database.GetStatProfiles();
        return profiles.GetLevelBonus(database.GetStatProfile(itemType).Get(ItemStat::Defense), level);
    };
    // Without a stat file every item has the empty default.
    CHECK(defenseBonus(smallShieldType, 7) == 0);

    database.SetStatProfiles(ItemStatProfiles(file));
    CHECK(defenseBonus(smallShieldType, 7) == 7);
    CHECK(defenseBonus(krisType, 7) == 0);
    // A profile the file does not have, an empty slot and an invalid id get the default.
    CHECK(defenseBonus(shortSwordType, 7) == 0);
    CHECK(defenseBonus(MakeItemType(6, 1), 7) == 0);
    CHECK(defenseBonus(-1, 7) == 0);

    // Moves and edits resolve the profile again.
    database.Swap(smallShieldType, MakeItemType(6, 5));
    CHECK(defenseBonus(MakeItemType(6, 5), 7) == 7);
    CHECK(defenseBonus(smallShieldType, 7) == 0);
    ItemDefinition kris = *database.Find(krisType);
    kris.statProfile = "shield";
    database.Set(kris);
    CHECK(defenseBonus(krisType, 7) == 7);
}

TEST_CASE("The stat profile an item's data gives is the one of the first profile rule it matches [data][items]")
{
    StatProfileRule ammunition{.profile = "default"};
    ammunition.tags.Set(ItemTag::Ammunition);
    StatProfileRule bows{.profile = "bow"};
    bows.groups.set(4);
    StatProfileRule staffs{.profile = "missing"};
    staffs.groups.set(5);
    const ItemStatProfiles profiles = MakeStatProfiles({"bow"}, {ammunition, bows, staffs});

    ItemDefinition bolt = MakeItem(4, 7, "Bolt");
    bolt.tags.Set(ItemTag::Ammunition);
    // A rule for the default gives no profile, as an item that has the default names none.
    CHECK(DeriveStatProfile(profiles, bolt).empty());
    const ItemDefinition bow = MakeItem(4, 0, "Short Bow");
    CHECK(DeriveStatProfile(profiles, bow) == "bow");
    // A profile the file does not have gives the default.
    CHECK(DeriveStatProfile(profiles, MakeItem(5, 0, "Skull Staff")).empty());
    // No rule matches.
    CHECK(DeriveStatProfile(profiles, MakeItem(14, 0, "Apple")).empty());
    CHECK(DeriveStatProfile(ItemStatProfiles(), bow).empty());
}

TEST_CASE("An edited item's stat profile follows the profile rules while it is the one they give [data][items]")
{
    StatProfileRule shields{.profile = "shield"};
    shields.weaponKinds.set(static_cast<size_t>(WeaponKind::Shield));
    const ItemStatProfiles profiles = MakeStatProfiles({"custom", "shield"}, {shields});

    ItemDefinition shield = MakeItem(6, 0, "Small Shield");
    shield.slot = ItemSlot::OffHand;
    shield.weaponKind = WeaponKind::Shield;
    shield.statProfile = "shield";

    ItemDefinition edited = shield;
    edited.weaponKind = WeaponKind::None;
    CHECK(FollowStatProfile(profiles, &shield, edited).empty());
    CHECK(FollowStatProfile(profiles, nullptr, shield) == "shield");

    // A profile the file sets that differs from the one the rules give stays, also "default".
    shield.statProfile = "custom";
    CHECK(FollowStatProfile(profiles, &shield, edited) == "custom");
    shield.statProfile = "default";
    CHECK(FollowStatProfile(profiles, &shield, edited) == "default");

    // An item without a profile, as in item files saved before the field, takes the rules' one.
    shield.statProfile = "";
    CHECK(FollowStatProfile(profiles, &shield, shield) == "shield");
}

TEST_CASE("A stat profile rule matches an item that has one of the values of each condition it has [data][items]")
{
    ItemDefinition helm = MakeItem(7, 0, "Bronze Helm");
    helm.slot = ItemSlot::Helm;

    StatProfileRule rule{.profile = "armor"};
    // Without conditions it matches every item.
    CHECK(Matches(rule, helm));

    rule.slots.set(static_cast<size_t>(ItemSlot::Helm));
    rule.slots.set(static_cast<size_t>(ItemSlot::Boots));
    CHECK(Matches(rule, helm));
    helm.slot = ItemSlot::Pants;
    CHECK_FALSE(Matches(rule, helm));
    // An item without a slot meets no slot condition.
    helm.slot = ItemSlot::None;
    CHECK_FALSE(Matches(rule, helm));
    helm.slot = ItemSlot::Boots;

    // Every condition the rule has must hold.
    rule.groups.set(7);
    CHECK(Matches(rule, helm));
    helm.group = 8;
    CHECK_FALSE(Matches(rule, helm));
    helm.group = 7;

    rule.tags.Set(ItemTag::Cape);
    rule.tags.Set(ItemTag::LuckyItem);
    CHECK_FALSE(Matches(rule, helm));
    helm.tags.Set(ItemTag::LuckyItem);
    CHECK(Matches(rule, helm));

    rule.wingTiers.set(static_cast<size_t>(WingTier::Second));
    CHECK_FALSE(Matches(rule, helm));
    helm.wingTier = WingTier::Second;
    CHECK(Matches(rule, helm));

    rule.weaponKinds.set(static_cast<size_t>(WeaponKind::Shield));
    CHECK_FALSE(Matches(rule, helm));
    helm.weaponKind = WeaponKind::Shield;
    CHECK(Matches(rule, helm));
}

TEST_CASE("A stat profile rule's requirement condition counts base values above 0 only [data][items]")
{
    ItemDefinition orb = MakeItem(12, 11, "Orb of Summoning");
    StatProfileRule rule{.profile = "scroll"};
    rule.requirements.set(static_cast<size_t>(ItemRequirement::Energy));
    rule.requirements.set(static_cast<size_t>(ItemRequirement::Leadership));
    CHECK_FALSE(Matches(rule, orb));

    // A requirement given by level is no base value.
    RequirementByLevel energy{};
    energy.fill(100);
    orb.requirementsByLevel.Set(ItemRequirement::Energy, energy);
    CHECK_FALSE(Matches(rule, orb));

    orb.requirements.level = 10;
    CHECK_FALSE(Matches(rule, orb));
    orb.requirements.leadership = 1;
    CHECK(Matches(rule, orb));
}

TEST_CASE("The first stat profile rule an item matches is the one it follows [data][items]")
{
    const std::vector<StatProfileRule> rules = []
    {
        std::vector<StatProfileRule> list(3);
        list[0].profile = "default";
        list[0].tags.Set(ItemTag::Ammunition);
        list[1].profile = "weapon";
        list[1].groups.set(4);
        list[2].profile = "other";
        return list;
    }();

    ItemDefinition bolt = MakeItem(4, 7, "Bolt");
    bolt.tags.Set(ItemTag::Ammunition);
    CHECK(FindMatchingRule(rules, bolt) == 0);
    ItemDefinition bow = MakeItem(4, 0, "Short Bow");
    CHECK(FindMatchingRule(rules, bow) == 1);
    ItemDefinition potion = MakeItem(14, 0, "Apple");
    CHECK(FindMatchingRule(rules, potion) == 2);

    CHECK(FindMatchingRule(std::span(rules).first(2), potion) == NoStatProfileRule);
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
