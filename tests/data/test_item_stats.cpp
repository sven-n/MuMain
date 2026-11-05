#include "stdafx.h"

#include "doctest.h"

#include "ClientDataScope.h"
#include "ItemStatProfilesScope.h"
#include "LevelBasisScrollProfile.h"
#include "TestFiles.h"

#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "Data/DataHandler/ItemData/ItemJsonStorage.h"
#include "Data/GameData/ItemData/ItemAttributeConversion.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemStatFile.h"
#include "Data/GameData/ItemData/ItemStatsJson.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Engine/Object/ZzzInventory.h"
#include "GameLogic/Items/ItemCategories.h"
#include "GameLogic/Items/ItemStats.h"
#include "I18N/All.h"
#include "UI/Inventory/ItemHelpContentKey.h"

#include <algorithm>
#include <array>
#include <filesystem>
#include <iterator>
#include <optional>
#include <string>

using namespace Data::Items;

// What SetItemAttributes computes from the shipped item data: the values the item
// tooltip and the character window show. The values the fixes of #683 changed are
// the ones the client before 815b8828 and OpenMU compute; the "Unchanged" checks
// pin the client's other values.
namespace
{
const std::filesystem::path ClientDirectory = std::filesystem::path(MU_TEST_DATA_DIR).parent_path();

constexpr int MorningStarType = MakeItemType(2, 1);
constexpr int SmallShieldType = MakeItemType(6, 0);
constexpr int DragonShieldType = MakeItemType(6, 13);
constexpr int BronzeArmorType = MakeItemType(8, 0);
constexpr int LeatherArmorType = MakeItemType(8, 5);
constexpr int StormCrowArmorType = MakeItemType(8, 15);
constexpr int FirstSmallWingType = MakeItemType(12, 130);
constexpr int LastSmallWingType = MakeItemType(12, 135);

// Loads the shipped item data into a test-owned ItemAttribute, as the game does at
// startup; the client data scope restores the previous one, also when loading fails.
class ShippedItemAttributes
{
public:
    ShippedItemAttributes() : m_client(ClientDirectory)
    {
        I18N::SetLocale("en");
        std::string errorMessage;
        REQUIRE(g_ItemDataHandler.Load(errorMessage));
    }

    ITEM_ATTRIBUTE& operator[](int itemType)
    {
        return ItemAttribute[itemType];
    }

private:
    ClientDataScope m_client;
};

using ItemInfoTable = std::array<std::array<int, _COLUMN_TYPE_COUNT>, iMaxLevel + 1>;

// Points the item help window at a test-owned hero: ComputeItemInfo marks the levels its
// attributes can equip, and the "Can be equipped by" lines compare its class.
class ItemHelpScope
{
public:
    ItemHelpScope() : m_previousHero(CharacterAttribute), m_previousHeroCharacter(::Hero)
    {
        CharacterAttribute = &m_hero;
        ::Hero = &m_heroCharacter;
    }

    ~ItemHelpScope()
    {
        CharacterAttribute = m_previousHero;
        ::Hero = m_previousHeroCharacter;
    }

    CHARACTER_ATTRIBUTE& Hero()
    {
        return m_hero;
    }

    CHARACTER& HeroCharacter()
    {
        return m_heroCharacter;
    }

private:
    CHARACTER_ATTRIBUTE m_hero{};
    CHARACTER m_heroCharacter;
    CHARACTER_ATTRIBUTE* m_previousHero;
    CHARACTER* m_previousHeroCharacter;
};

// The table of the first item the window shows, while the table is still empty.
ItemInfoTable ComputeFirstItemInfo(int itemType)
{
    for (auto& row : g_iItemInfo)
    {
        std::ranges::fill(row, 0);
    }
    ComputeItemInfo(itemType);

    ItemInfoTable table{};
    for (int level = 0; level <= iMaxLevel; ++level)
    {
        std::ranges::copy(g_iItemInfo[level], table[level].begin());
    }
    return table;
}

constexpr int NoMagicColumn = -1;

struct HelpTableCase
{
    int itemType;
    int magicColumn;
};

// The column an item's magic power goes into: Curse for a summoner book, Attack for a sceptre,
// WIZDmg for a staff, none for an item without magic power.
constexpr HelpTableCase HelpTableCases[] = {
    {ITEM_BOOK_OF_SAHAMUTT, _COLUMN_TYPE_CURSE},
    {ITEM_GREAT_LORD_SCEPTER, _COLUMN_TYPE_PET_ATTACK},
    {ITEM_LEGENDARY_STAFF, _COLUMN_TYPE_MAGIC},
    {ITEM_KRIS, NoMagicColumn},
};

constexpr int MagicColumns[] = {_COLUMN_TYPE_MAGIC, _COLUMN_TYPE_CURSE, _COLUMN_TYPE_PET_ATTACK};

ITEM ComputeItem(int itemType, int level, BYTE excellentFlags = 0, BYTE ancientDiscriminator = 0, BYTE optionLevel = 0)
{
    ITEM item{};
    item.Type = static_cast<short>(itemType);
    item.Level = level;
    item.ExcellentFlags = excellentFlags;
    item.AncientDiscriminator = ancientDiscriminator;
    item.OptionLevel = optionLevel;
    SetItemAttributes(&item);
    return item;
}

// The + levels the item help table shows in white.
int CountEquippableLevels()
{
    int count = 0;
    for (const auto& row : g_iItemInfo)
    {
        if (row[_COLUMN_TYPE_CAN_EQUIP] == TRUE)
        {
            ++count;
        }
    }
    return count;
}

// Gives the hero the stats the help table's item requires at the + level, no more.
void GiveRequiredStats(CHARACTER_ATTRIBUTE& hero, int level)
{
    hero.Strength = static_cast<WORD>(g_iItemInfo[level][_COLUMN_TYPE_REQSTR]);
    hero.Dexterity = static_cast<WORD>(g_iItemInfo[level][_COLUMN_TYPE_REQDEX]);
    hero.Energy = static_cast<WORD>(g_iItemInfo[level][_COLUMN_TYPE_REQENG]);
    hero.Vitality = static_cast<WORD>(g_iItemInfo[level][_COLUMN_TYPE_REQVIT]);
    hero.Charisma = static_cast<WORD>(g_iItemInfo[level][_COLUMN_TYPE_REQCHA]);
}
} // namespace

TEST_CASE("The shipped item stat file loads without problems and is in the written format [data][items]")
{
    const ItemStatsLoadResult result = LoadItemStatsFile(ClientDirectory / GetItemStatsFile());
    for (const ItemDataIssue& issue : result.issues)
    {
        INFO(issue.ToString());
        CHECK(false);
    }
    CHECK(WriteItemStatsJson(result.file) == TestFiles::ReadWholeFile(ClientDirectory / GetItemStatsFile()));
}

// The bmd import and the item editor keep a profile the profile rules give; the shipped data
// follows the same rule, so nothing changes when an item is saved again. The Horn of Fenrir is
// no helper: its level requirement does not grow, so it names the default.
TEST_CASE("Every shipped item names the stat profile the profile rules give [data][items]")
{
    ShippedItemAttributes items;
    int itemsWithProfile = 0;
    for (const ItemDefinition& definition : g_ItemDatabase.GetAllSlots())
    {
        if (!definition.Exists())
        {
            continue;
        }
        CAPTURE(g_ItemDatabase.GetLogName(MakeItemType(definition.group, definition.number)));
        const bool hornOfFenrir = MakeItemType(definition.group, definition.number) == ITEM_HORN_OF_FENRIR;
        CHECK(definition.statProfile ==
              (hornOfFenrir ? "default" : DeriveStatProfile(g_ItemDatabase.GetStatProfiles(), definition)));
        itemsWithProfile += definition.statProfile.empty() || hornOfFenrir ? 0 : 1;
    }
    CHECK(DeriveStatProfile(g_ItemDatabase.GetStatProfiles(), *g_ItemDatabase.Find(ITEM_HORN_OF_FENRIR)) == "helper");
    CHECK(itemsWithProfile == 578);
}

TEST_CASE("The shipped profile rules give each kind of item its profile [data][items]")
{
    ShippedItemAttributes items;
    const ItemStatProfiles& profiles = g_ItemDatabase.GetStatProfiles();
    const auto make = [](int group, int number)
    {
        ItemDefinition definition;
        definition.group = group;
        definition.number = number;
        return definition;
    };

    ItemDefinition sceptre = make(2, 8);
    sceptre.weaponKind = WeaponKind::Mace;
    CHECK(DeriveStatProfile(profiles, sceptre) == "weapon");
    sceptre.tags.Set(ItemTag::Sceptre);
    CHECK(DeriveStatProfile(profiles, sceptre) == "sceptre");

    ItemDefinition shield = make(6, 0);
    shield.slot = ItemSlot::OffHand;
    shield.weaponKind = WeaponKind::Shield;
    CHECK(DeriveStatProfile(profiles, shield) == "shield");

    ItemDefinition wing = make(12, 0);
    wing.slot = ItemSlot::Wings;
    const std::pair<WingTier, const char*> tiers[] = {{WingTier::None, ""},
                                                      {WingTier::Small, "smallWing"},
                                                      {WingTier::First, "firstTierWing"},
                                                      {WingTier::Second, "secondTierWing"},
                                                      {WingTier::Third, "thirdTierWing"}};
    for (const auto& [tier, profile] : tiers)
    {
        wing.wingTier = tier;
        CHECK(DeriveStatProfile(profiles, wing) == profile);
    }
    wing.wingTier = WingTier::Second;
    wing.tags.Set(ItemTag::Cape);
    CHECK(DeriveStatProfile(profiles, wing) == "secondTierCape");
    // A wing tier counts only on the wings slot.
    wing.slot = ItemSlot::Helm;
    CHECK(DeriveStatProfile(profiles, wing) == "armor");

    ItemDefinition kris = make(0, 0);
    kris.slot = ItemSlot::MainHand;
    kris.weaponKind = WeaponKind::Sword;
    CHECK(DeriveStatProfile(profiles, kris) == "weapon");

    ItemDefinition book = make(5, 21);
    book.weaponKind = WeaponKind::Book;
    book.tags.Set(ItemTag::SummonerBook);
    CHECK(DeriveStatProfile(profiles, book) == "book");

    ItemDefinition bolt = make(4, 7);
    bolt.tags.Set(ItemTag::Ammunition);
    CHECK(DeriveStatProfile(profiles, bolt).empty());

    ItemDefinition ring = make(13, 8);
    ring.slot = ItemSlot::Ring;
    CHECK(DeriveStatProfile(profiles, ring).empty());
    ring.requirements.level = 20;
    CHECK(DeriveStatProfile(profiles, ring) == "helper");

    ItemDefinition scroll = make(15, 0);
    CHECK(DeriveStatProfile(profiles, scroll).empty());
    scroll.requirements.energy = 100;
    CHECK(DeriveStatProfile(profiles, scroll) == "scroll");
}

// The shipped profile rules give the profiles that the code before them gave; how many
// items each rule reaches pins them.
TEST_CASE("The items each shipped profile rule gives its profile [data][items]")
{
    ShippedItemAttributes items;
    const ItemStatProfiles& profiles = g_ItemDatabase.GetStatProfiles();
    const std::vector<int> expected = {2, 10, 3, 22, 113, 327, 6, 4, 2, 5, 7, 0, 12, 36, 32};
    REQUIRE(profiles.GetRules().size() == expected.size());
    std::vector<int> counts(expected.size());
    int withoutRule = 0;
    for (const ItemDefinition& definition : g_ItemDatabase.GetAllSlots())
    {
        if (!definition.Exists())
        {
            continue;
        }
        const int rule = profiles.FindRule(definition);
        if (rule == NoStatProfileRule)
        {
            ++withoutRule;
            continue;
        }
        ++counts[static_cast<size_t>(rule)];
    }
    CHECK(counts == expected);
    CHECK(withoutRule == 330);
}

// The fixed excellent bonus of the three chaos weapons is item data.
TEST_CASE("The chaos weapons' fixed excellent bonus is item data [data][items]")
{
    ShippedItemAttributes items;
    CHECK(g_ItemDatabase.Find(ITEM_CHAOS_DRAGON_AXE)->excellentBonus == 15);
    CHECK(g_ItemDatabase.Find(ITEM_CHAOS_NATURE_BOW)->excellentBonus == 30);
    CHECK(g_ItemDatabase.Find(ITEM_CHAOS_LIGHTNING_STAFF)->excellentBonus == 25);
    int itemsWithBonus = 0;
    for (const ItemDefinition& definition : g_ItemDatabase.GetAllSlots())
    {
        itemsWithBonus += definition.excellentBonus != 0 ? 1 : 0;
    }
    CHECK(itemsWithBonus == 3);
}

// ComputeItemStats takes the terms from the item stat file and the item's profile.
TEST_CASE("Item stats come from the item's stat profile [data][items]")
{
    ShippedItemAttributes items;
    const auto stats = [&items](int itemType, int level, BYTE excellentFlags = 0, BYTE ancientDiscriminator = 0)
    {
        return GameLogic::Items::ComputeItemStats(items[itemType],
                                                  {itemType, level, excellentFlags, ancientDiscriminator});
    };

    // Shield: block rate from the default table, defense +1 per level and no excellent part;
    // the ancient part counts the level bonus: 29 + 29 * 20 / 90 + 2.
    CHECK(stats(DragonShieldType, 15).blockRate == 126);
    CHECK(stats(DragonShieldType, 15).defense == 29);
    CHECK(stats(DragonShieldType, 15, 0x01).blockRate == 156);
    CHECK(stats(DragonShieldType, 15, 0x01).defense == 29);
    CHECK(stats(DragonShieldType, 15, 0, 1).defense == 37);

    // Second tier wing: 2 per level up to +9; a wing option is no excellent option.
    CHECK(stats(ITEM_WINGS_OF_DRAGON, 9).defense == 63);
    CHECK(stats(ITEM_WINGS_OF_DRAGON, 15).defense == 102);
    CHECK(stats(ITEM_WINGS_OF_DRAGON, 15, 0x01).defense == 102);
    CHECK(stats(ITEM_CAPE_OF_LORD, 15).defense == 72);

    // The fixed excellent bonus of the Chaos Dragon Axe instead of 102 * 25 / 75 + 5.
    CHECK(stats(ITEM_CHAOS_DRAGON_AXE, 0, 0x01).damageMin == 117);
    CHECK(stats(ITEM_CHAOS_DRAGON_AXE, 0, 0x01).damageMax == 145);

    // Magic power: halved, then +2 per level except for sceptres.
    CHECK(stats(ITEM_DRAGON_SOUL_STAFF, 15).magicPower == 109);
    CHECK(stats(ITEM_BATTLE_SCEPTER, 15).magicPower == 34);

    // A kind without a stat keeps 0.
    CHECK(stats(ITEM_KRIS, 15).defense == 0);
    CHECK(stats(ITEM_KRIS, 15).magicPower == 0);
}

// The requirements come from the profile's requirement terms and the item's own data.
TEST_CASE("Item requirements come from the item's stat profile [data][items]")
{
    ShippedItemAttributes items;
    const auto stats =
        [&items](int itemType, int level, BYTE excellentFlags = 0, BYTE ancientDiscriminator = 0, BYTE optionLevel = 0)
    {
        return GameLogic::Items::ComputeItemStats(items[itemType],
                                                  {itemType, level, excellentFlags, ancientDiscriminator, optionLevel});
    };
    // The checks against ComputeItem (the item SetItemAttributes fills) hold before and after it uses the profiles.
    ItemHelpScope hero;

    // Weapon: 20 + base * (drop level + 3 * level) * 3 / 100; excellent drop level + 25; 5 strength per option level.
    CHECK(stats(ITEM_BLADE, 0).requireStrength == 106);
    CHECK(stats(ITEM_BLADE, 0).requireDexterity == 74);
    CHECK(stats(ITEM_BLADE, 9, 0x01).requireStrength == 231);
    CHECK(stats(ITEM_BLADE, 15, 0, 1, 4).requireStrength == ComputeItem(ITEM_BLADE, 15, 0, 1, 4).RequireStrength);
    CHECK(stats(ITEM_BLADE, 15, 0, 1, 4).requireStrength == 306);
    // Excellent and ancient: the excellent drop level bonus counts.
    CHECK(stats(ITEM_DRAGON_ARMOR, 9, 0x01, 1).requireStrength == 419);
    CHECK(stats(ITEM_DRAGON_ARMOR, 9, 0, 1).requireStrength == 437);

    // Level requirement growth: second tier wings 5 per level, capes and group 13 4; no +20 for wings.
    CHECK(stats(ITEM_WINGS_OF_DRAGON, 15, 0x01).requireLevel == 290);
    CHECK(stats(ITEM_CAPE_OF_LORD, 15).requireLevel == 240);
    CHECK(stats(ITEM_RING_OF_ICE, 4).requireLevel == 36);
    CHECK(stats(ITEM_RING_OF_ICE, 4, 0x01).requireLevel == 56);
    // excellentRaisesLevel false: the Dragon Knight Helm needs 380 also when excellent, and the
    // Horn of Fenrir keeps 300 at every level.
    CHECK(stats(ITEM_DRAGON_KNIGHT_HELM, 0, 0x01).requireLevel == 380);
    CHECK(stats(ITEM_HORN_OF_FENRIR, 15, 0x01).requireLevel == 300);

    // Requirements by level, and the +20 on top of the ring's list.
    CHECK(stats(ITEM_ORB_OF_SUMMONING, 3).requireEnergy == 130);
    CHECK(stats(ITEM_ORB_OF_SUMMONING, 7).requireEnergy == 500);
    CHECK(stats(ITEM_ORB_OF_SUMMONING, 8).requireEnergy == 0);
    CHECK(stats(ITEM_TRANSFORMATION_RING, 2).requireLevel == 20);
    CHECK(stats(ITEM_TRANSFORMATION_RING, 3, 0x01).requireLevel == 70);

    // Books: energy with one per level; their strength grows with the option even from 0.
    CHECK(stats(ITEM_BOOK_OF_SAHAMUTT, 0).requireEnergy == ComputeItem(ITEM_BOOK_OF_SAHAMUTT, 0).RequireEnergy);
    CHECK(stats(ITEM_BOOK_OF_SAHAMUTT, 15, 0, 0, 12).requireStrength == 60);
    CHECK(stats(ITEM_BOOK_OF_SAHAMUTT, 15, 0, 0, 12).requireStrength ==
          ComputeItem(ITEM_BOOK_OF_SAHAMUTT, 15, 0, 0, 12).RequireStrength);

    // Scrolls: the energy as stored, the level requirement + 4 per level.
    const int wizardryEnhance = MakeItemType(15, 28);
    CHECK(stats(wizardryEnhance, 5).requireEnergy == 1058);
    CHECK(stats(wizardryEnhance, 5).requireLevel == 240);

    // The "levelRequirement" basis, which no shipped profile uses any more: the energy comes
    // from the level requirement, also one given by level.
    const LevelBasisScrollProfile levelBasis(ClientDirectory);
    constexpr int energyWeight = 118;
    ItemDefinition givenByLevel = *g_ItemDatabase.Find(wizardryEnhance);
    RequirementByLevel levels{};
    levels.fill(100);
    givenByLevel.requirements.level = 0;
    givenByLevel.requirements.energy = energyWeight;
    givenByLevel.requirementsByLevel.Set(ItemRequirement::Level, levels);
    g_ItemDatabase.Set(givenByLevel);
    items[wizardryEnhance].RequireLevel = 0;
    items[wizardryEnhance].RequireEnergy = energyWeight;
    CHECK(stats(wizardryEnhance, 5).requireLevel == 100);
    CHECK(stats(wizardryEnhance, 5).requireEnergy == 20 + energyWeight * 100 * 4 / 100);
}

// The item editor computes an item it has not stored from its definition: for every
// shipped item that gives what the game computes from the item database.
TEST_CASE("Item stats from an item's definition are those from the item database [data][items]")
{
    ShippedItemAttributes items;
    const ItemStatProfiles& profiles = g_ItemDatabase.GetStatProfiles();
    int compared = 0;
    for (const ItemDefinition& definition : g_ItemDatabase.GetAllSlots())
    {
        if (!definition.Exists())
        {
            continue;
        }
        const int itemType = MakeItemType(definition.group, definition.number);
        ITEM_ATTRIBUTE attribute{};
        ToItemAttribute(definition, attribute);
        const int profileIndex = profiles.Find(definition.statProfile);
        const ItemStatProfile& profile = profiles.Get(
            profileIndex == ItemStatProfiles::UnknownProfile ? ItemStatProfiles::DefaultProfile : profileIndex);
        const GameLogic::Items::ItemStatSources sources{attribute, &definition, profile,
                                                        definition.slot == ItemSlot::Wings};
        for (int level = 0; level < ItemLevelCount; ++level)
        {
            for (const BYTE excellentFlags : {BYTE{0}, BYTE{0x01}})
            {
                for (const BYTE ancientDiscriminator : {BYTE{0}, BYTE{1}})
                {
                    for (const BYTE optionLevel : {BYTE{0}, BYTE{4}})
                    {
                        const GameLogic::Items::ItemStatInput input{itemType, level, excellentFlags,
                                                                    ancientDiscriminator, optionLevel};
                        const GameLogic::Items::ItemStatValues fromDatabase =
                            GameLogic::Items::ComputeItemStats(items[itemType], input);
                        const GameLogic::Items::ItemStatValues fromDefinition =
                            GameLogic::Items::ComputeItemStats(sources, input);
                        INFO("(" << definition.group << "," << definition.number << ") +" << level);
                        REQUIRE(fromDefinition.damageMin == fromDatabase.damageMin);
                        REQUIRE(fromDefinition.damageMax == fromDatabase.damageMax);
                        REQUIRE(fromDefinition.magicPower == fromDatabase.magicPower);
                        REQUIRE(fromDefinition.blockRate == fromDatabase.blockRate);
                        REQUIRE(fromDefinition.defense == fromDatabase.defense);
                        REQUIRE(fromDefinition.magicDefense == fromDatabase.magicDefense);
                        REQUIRE(fromDefinition.requireLevel == fromDatabase.requireLevel);
                        REQUIRE(fromDefinition.requireStrength == fromDatabase.requireStrength);
                        REQUIRE(fromDefinition.requireDexterity == fromDatabase.requireDexterity);
                        REQUIRE(fromDefinition.requireEnergy == fromDatabase.requireEnergy);
                        REQUIRE(fromDefinition.requireVitality == fromDatabase.requireVitality);
                        REQUIRE(fromDefinition.requireLeadership == fromDatabase.requireLeadership);
                    }
                }
            }
        }
        ++compared;
    }
    CHECK(compared > 900);
}

// The items whose level requirement excellent options do not raise are item data: the
// Dragon Knight to Sunlight and Storm Blitz armour, six weapons, the lucky items and the Horn
// of Fenrir.
TEST_CASE("The items excellent options do not raise the level requirement of [data][items]")
{
    ShippedItemAttributes items;
    int itemsWithoutRaise = 0;
    for (const ItemDefinition& definition : g_ItemDatabase.GetAllSlots())
    {
        itemsWithoutRaise += definition.Exists() && !definition.excellentRaisesLevel ? 1 : 0;
    }
    CHECK(itemsWithoutRaise == 89);
    CHECK_FALSE(g_ItemDatabase.Find(ITEM_HORN_OF_FENRIR)->excellentRaisesLevel);
    CHECK_FALSE(g_ItemDatabase.Find(ITEM_DRAGON_KNIGHT_HELM)->excellentRaisesLevel);
    CHECK_FALSE(g_ItemDatabase.Find(ITEM_BONE_BLADE)->excellentRaisesLevel);
    CHECK(g_ItemDatabase.Find(ITEM_KRIS)->excellentRaisesLevel);
}

// The Orb of Summoning's energy and the Transformation Ring's level requirement by level are
// item data.
TEST_CASE("The requirements given by level are item data [data][items]")
{
    ShippedItemAttributes items;
    const RequirementByLevel* orbEnergy =
        g_ItemDatabase.Find(ITEM_ORB_OF_SUMMONING)->requirementsByLevel.Find(ItemRequirement::Energy);
    REQUIRE(orbEnergy != nullptr);
    CHECK((*orbEnergy)[7] == 500);
    const RequirementByLevel* ringLevel =
        g_ItemDatabase.Find(ITEM_TRANSFORMATION_RING)->requirementsByLevel.Find(ItemRequirement::Level);
    REQUIRE(ringLevel != nullptr);
    CHECK((*ringLevel)[2] == 20);
    CHECK((*ringLevel)[3] == 50);
}

TEST_CASE("A shield's defense rate grows with its + level [data][items]")
{
    ShippedItemAttributes items;

    CHECK(ComputeItem(DragonShieldType, 0).SuccessfulBlocking == 60);
    CHECK(ComputeItem(DragonShieldType, 9).SuccessfulBlocking == 87);

    for (int number = 0; number < MAX_ITEM_INDEX; ++number)
    {
        const int shieldType = MakeItemType(6, number);
        if (items[shieldType].SuccessfulBlocking == 0)
        {
            continue;
        }
        CAPTURE(number);
        for (int level = 0; level < 15; ++level)
        {
            CAPTURE(level);
            CHECK(ComputeItem(shieldType, level + 1).SuccessfulBlocking >
                  ComputeItem(shieldType, level).SuccessfulBlocking);
        }
    }
}

TEST_CASE("Magic defense grows with the + level [data][items]")
{
    ShippedItemAttributes items;
    // No shipped item has magic defense; the editor can give one some.
    items[BronzeArmorType].MagicDefense = 10;

    CHECK(ComputeItem(BronzeArmorType, 0).MagicDefense == 10);
    CHECK(ComputeItem(BronzeArmorType, 9).MagicDefense == 37);
    CHECK(ComputeItem(BronzeArmorType, 15).MagicDefense == 76);
}

TEST_CASE("The late wings, crystals and small wings keep their level requirement [data][items]")
{
    ShippedItemAttributes items;

    for (int level = 0; level <= 15; ++level)
    {
        CAPTURE(level);
        CHECK(ComputeItem(ITEM_WING_OF_DIMENSION, level).RequireLevel == 400);
        CHECK(ComputeItem(ITEM_CAPE_OF_OVERRULE, level).RequireLevel == 400);
        for (int type = ITEM_CRYSTAL_OF_DESTRUCTION; type <= ITEM_SCROLL_OF_CHAOTIC_DISEIER; ++type)
        {
            CAPTURE(type);
            CHECK(ComputeItem(type, level).RequireLevel == 220);
        }
        for (int type = FirstSmallWingType; type <= LastSmallWingType; ++type)
        {
            CAPTURE(type);
            CHECK(ComputeItem(type, level).RequireLevel == 1);
        }

        // Unchanged: the other third tier wings keep theirs, the Cape of Fighter's grows.
        CHECK(ComputeItem(ITEM_WING_OF_STORM, level).RequireLevel == 400);
        CHECK(ComputeItem(ITEM_CAPE_OF_EMPEROR, level).RequireLevel == 400);
        CHECK(ComputeItem(ITEM_CAPE_OF_FIGHTER, level).RequireLevel == 180 + level * 4);
    }
}

// The Cape of Lord's defense of 15 is item data: the item table has it from the start, and
// creating a cape writes nothing into the table.
TEST_CASE("The Cape of Lord's base defense is item data [data][items]")
{
    ShippedItemAttributes items;
    CHECK(items[ITEM_CAPE_OF_LORD].Defense == 15);

    CHECK(ComputeItem(ITEM_CAPE_OF_LORD, 0).Defense == 15);
    items[ITEM_CAPE_OF_LORD].Defense = 20;
    CHECK(ComputeItem(ITEM_CAPE_OF_LORD, 0).Defense == 20);
    CHECK(items[ITEM_CAPE_OF_LORD].Defense == 20);
}

TEST_CASE("A wing's option does not count as excellent [data][items]")
{
    ShippedItemAttributes items;

    CHECK(ComputeItem(ITEM_WING_OF_STORM, 0, 0x01).Defense == 60);
    CHECK(ComputeItem(ITEM_CAPE_OF_LORD, 0, 0x01).Defense == 15);

    for (int type = MakeItemType(12, 0); type < MakeItemType(14, 0); ++type)
    {
        ITEM wing{};
        wing.Type = static_cast<short>(type);
        if (items[type].Name[0] == L'\0' || !GameLogic::Items::IsWingItem(&wing))
        {
            continue;
        }
        CAPTURE(type);
        for (int level = 0; level <= 15; ++level)
        {
            CAPTURE(level);
            CHECK(ComputeItem(type, level, 0x01).Defense == ComputeItem(type, level).Defense);
        }
    }
}

TEST_CASE("Ancient items get the excellent bonus and their base defense once [data][items]")
{
    ShippedItemAttributes items;

    CHECK(ComputeItem(LeatherArmorType, 0, 0, 1).Defense == 33);
    CHECK(ComputeItem(StormCrowArmorType, 0, 0, 1).Defense == 76);
    CHECK(ComputeItem(SmallShieldType, 0, 0, 1).SuccessfulBlocking == 33);
    CHECK(ComputeItem(MorningStarType, 0, 0, 1).DamageMin == 49);

    // Unchanged: excellent armor, and the stat requirements of ancient items.
    CHECK(ComputeItem(LeatherArmorType, 0, 0x01).Defense == 28);
    CHECK(ComputeItem(BronzeArmorType, 0, 0, 1).RequireStrength == 135);
}

TEST_CASE("The excellent bonus adds the same to minimum and maximum damage [data][items]")
{
    ShippedItemAttributes items;

    CHECK(ComputeItem(MorningStarType, 0, 0, 1).DamageMax == 58);

    for (int type = MakeItemType(0, 0); type < MakeItemType(6, 0); ++type)
    {
        if (items[type].DamageMin <= 0 || items[type].DamageMax <= 0 || items[type].Level == 0)
        {
            continue;
        }
        CAPTURE(type);
        const ITEM normal = ComputeItem(type, 0);
        const ITEM excellent = ComputeItem(type, 0, 0x01);
        CHECK(excellent.DamageMax - normal.DamageMax == excellent.DamageMin - normal.DamageMin);
    }
}

TEST_CASE("The + level bonus adds up above +9 [data][items]")
{
    ShippedItemAttributes items;
    // The Season 6 client and OpenMU: 27 at +9, then 31, 36, 42, 49, 57 and 66 at +10 to +15.
    constexpr int FirstLevelAboveNine = 10;
    constexpr int BonusFromTen[] = {31, 36, 42, 49, 57, 66};

    for (int i = 0; i < static_cast<int>(std::size(BonusFromTen)); ++i)
    {
        const int level = FirstLevelAboveNine + i;
        CAPTURE(level);
        CHECK(ComputeItem(ITEM_DOUBLE_BLADE, level).DamageMin == items[ITEM_DOUBLE_BLADE].DamageMin + BonusFromTen[i]);
        CHECK(ComputeItem(ITEM_DOUBLE_BLADE, level).DamageMax == items[ITEM_DOUBLE_BLADE].DamageMax + BonusFromTen[i]);
        CHECK(ComputeItem(ITEM_DRAGON_ARMOR, level).Defense == items[ITEM_DRAGON_ARMOR].Defense + BonusFromTen[i]);
        CHECK(ComputeItem(DragonShieldType, level).SuccessfulBlocking ==
              items[DragonShieldType].SuccessfulBlocking + BonusFromTen[i]);
    }

    // Magic power is halved after the bonus; staffs add 2 per level, sceptres don't (OpenMU's
    // staff and sceptre rise tables).
    CHECK(ComputeItem(ITEM_LEGENDARY_STAFF, 15).MagicPower == 92);
    CHECK(ComputeItem(ITEM_GREAT_LORD_SCEPTER, 15).MagicPower == 66);
    CHECK(ComputeItem(ITEM_DIVINE_STAFF_OF_ARCHANGEL, 15).MagicPower == 141);
    // With the excellent bonus the sum passes 255 before the halving.
    CHECK(ComputeItem(ITEM_DIVINE_STAFF_OF_ARCHANGEL, 15, 0x01).MagicPower == 162);
}

TEST_CASE("Wing defense adds up above +9, from 5 for the third tier wings [data][items]")
{
    ShippedItemAttributes items;

    CHECK(ComputeItem(ITEM_WING_OF_STORM, 10).Defense == 101);
    CHECK(ComputeItem(ITEM_WINGS_OF_DRAGON, 10).Defense == 67);

    CHECK(ComputeItem(ITEM_WING_OF_STORM, 15).Defense == 141);
    CHECK(ComputeItem(ITEM_WINGS_OF_DRAGON, 15).Defense == 102);
    CHECK(ComputeItem(ITEM_WINGS_OF_SATAN, 15).Defense == 86);
    CHECK(ComputeItem(ITEM_CAPE_OF_LORD, 15).Defense == 72);
}

TEST_CASE("Magic power passes 255 and defense rate stops there [data][items]")
{
    ShippedItemAttributes items;
    // Values the editor can give an item; no shipped item gets this high.
    items[ITEM_LEGENDARY_STAFF].MagicPower = 200;
    items[ITEM_LEGENDARY_STAFF].Level = 80;
    items[DragonShieldType].SuccessfulBlocking = 170;
    items[DragonShieldType].Level = 100;

    // 200 + 67 (excellent) + 66 (+15) = 333, halved, + 30.
    CHECK(ComputeItem(ITEM_LEGENDARY_STAFF, 15, 0x01).MagicPower == 196);
    // A later staff: 345 + 66 (excellent: 345 * 25 / 140 + 5) + 66 (+15) = 477, halved, + 30.
    items[ITEM_LEGENDARY_STAFF].MagicPower = 345;
    items[ITEM_LEGENDARY_STAFF].Level = 140;
    CHECK(ComputeItem(ITEM_LEGENDARY_STAFF, 15, 0x01).MagicPower == 268);
    // 170 + 47 (excellent) + 66 (+15) = 283.
    CHECK(ComputeItem(DragonShieldType, 15, 0x01).SuccessfulBlocking == 255);

    // The help table shows the same: 200 + 66 (+15) = 266.
    items[DragonShieldType].SuccessfulBlocking = 200;
    ItemHelpScope help;
    ComputeItemInfo(DragonShieldType);
    CHECK(ComputeItem(DragonShieldType, 15).SuccessfulBlocking == 255);
    CHECK(g_iItemInfo[15][_COLUMN_TYPE_DEFRATE] == 255);
}

// The numbers of the item stat file go up to 65535; with the largest ones the sums pass what
// an int holds.
TEST_CASE("Item stats and requirements stop at 65535 [data][items]")
{
    ShippedItemAttributes items;
    ItemStatsLoadResult stats = LoadItemStatsFile(ClientDirectory / GetItemStatsFile());
    REQUIRE_FALSE(HasFatal(stats.issues));
    WrittenStatSection& damage = stats.file.defaults[static_cast<size_t>(ItemStat::Damage)];
    damage.excellent.values.times = 65535;
    damage.ancient.values = {65535, 65535, 1, true};
    std::optional<RequirementFormula>& strength =
        stats.file.defaultRequirements.stats[static_cast<size_t>(ItemRequirementStat::Strength)];
    REQUIRE(strength);
    strength->levelWeight = 65535;
    strength->percent = 65535;
    stats.file.defaultRequirements.excellentLevelAdd = 65535;
    ItemStatProfilesScope restoreProfiles;
    g_ItemDatabase.SetStatProfiles(ItemStatProfiles(stats.file));
    items[ITEM_BLADE].DamageMin = 255;
    items[ITEM_BLADE].Level = 1;
    items[ITEM_BLADE].RequireStrength = 65535;
    items[ITEM_BLADE].RequireLevel = 400;

    const GameLogic::Items::ItemStatValues values =
        GameLogic::Items::ComputeItemStats(items[ITEM_BLADE], {ITEM_BLADE, 255, 0x01, 1, 15});
    CHECK(values.damageMin == 65535);
    CHECK(values.requireStrength == 65535);
    CHECK(values.requireLevel == 65535);
    const ITEM item = ComputeItem(ITEM_BLADE, 255, 0x01, 1);
    CHECK(item.DamageMin == 65535);
    CHECK(item.RequireStrength == 65535);
}

TEST_CASE("The item help table shows the stats the item has at each level [data][items]")
{
    ShippedItemAttributes items;
    ItemHelpScope help;

    ComputeItemInfo(ITEM_DOUBLE_BLADE);
    CHECK(g_iItemInfo[15][_COLUMN_TYPE_ATTMIN] == items[ITEM_DOUBLE_BLADE].DamageMin + 66);

    for (const int type :
         {static_cast<int>(ITEM_DOUBLE_BLADE), static_cast<int>(ITEM_LEGENDARY_STAFF),
          static_cast<int>(ITEM_GREAT_LORD_SCEPTER), static_cast<int>(ITEM_DRAGON_ARMOR), DragonShieldType})
    {
        CAPTURE(type);
        ComputeItemInfo(type);
        const int magicColumn = GameLogic::Items::IsSceptreType(type) ? _COLUMN_TYPE_PET_ATTACK : _COLUMN_TYPE_MAGIC;
        for (int level = 0; level <= 15; ++level)
        {
            CAPTURE(level);
            const ITEM item = ComputeItem(type, level);
            CHECK(g_iItemInfo[level][_COLUMN_TYPE_ATTMIN] == item.DamageMin);
            CHECK(g_iItemInfo[level][_COLUMN_TYPE_ATTMAX] == item.DamageMax);
            CHECK(g_iItemInfo[level][magicColumn] == item.MagicPower);
            CHECK(g_iItemInfo[level][_COLUMN_TYPE_DEFENCE] == item.Defense);
            CHECK(g_iItemInfo[level][_COLUMN_TYPE_DEFRATE] == item.SuccessfulBlocking);
        }
    }
}

// The item keeps its command requirement whatever its level; the help table shows the same.
TEST_CASE("The item help table shows the command requirement the item has [data][items]")
{
    ShippedItemAttributes items;
    ItemHelpScope help;
    // No shipped item the window shows has a command requirement; the editor can give one.
    items[ITEM_DRAGON_ARMOR].RequireCharisma = 100;
    ComputeItemInfo(ITEM_DRAGON_ARMOR);
    for (const int level : {0, 9, 15})
    {
        CAPTURE(level);
        CHECK(g_iItemInfo[level][_COLUMN_TYPE_REQCHA] == ComputeItem(ITEM_DRAGON_ARMOR, level).RequireCharisma);
        CHECK(g_iItemInfo[level][_COLUMN_TYPE_REQCHA] == 100);
    }
}

TEST_CASE("The item help table shows the magic power in the item's own column [data][items]")
{
    ShippedItemAttributes items;
    ItemHelpScope help;

    for (const HelpTableCase& item : HelpTableCases)
    {
        CAPTURE(item.itemType);
        const ItemInfoTable table = ComputeFirstItemInfo(item.itemType);
        for (int level = 0; level <= iMaxLevel; ++level)
        {
            CAPTURE(level);
            for (const int column : MagicColumns)
            {
                CAPTURE(column);
                if (column == item.magicColumn)
                {
                    CHECK(table[level][column] > 0);
                }
                else
                {
                    CHECK(table[level][column] == 0);
                }
            }
        }
    }
}

TEST_CASE("The item help table keeps nothing of the item shown before [data][items]")
{
    ShippedItemAttributes items;
    ItemHelpScope help;

    // The table is global: /Book of Samut and then /Legendary Staff showed a Curse column under
    // the staff, and /Book of Samut and then /Kris one under the Kris.
    for (const HelpTableCase& shown : HelpTableCases)
    {
        const ItemInfoTable first = ComputeFirstItemInfo(shown.itemType);
        for (const HelpTableCase& before : HelpTableCases)
        {
            if (before.itemType == shown.itemType)
            {
                continue;
            }

            CAPTURE(before.itemType);
            CAPTURE(shown.itemType);
            ComputeItemInfo(before.itemType);
            ComputeItemInfo(shown.itemType);
            for (int level = 0; level <= iMaxLevel; ++level)
            {
                CAPTURE(level);
                for (int column = 0; column < _COLUMN_TYPE_COUNT; ++column)
                {
                    CAPTURE(column);
                    CHECK(g_iItemInfo[level][column] == first[level][column]);
                }
            }
        }
    }
}

TEST_CASE("The item help table follows the hero's level and stats while it shows the same item [data][items]")
{
    ShippedItemAttributes items;
    ItemHelpScope help;
    CHARACTER_ATTRIBUTE& hero = help.Hero();
    const int requiredLevel = items[ITEM_BONE_BLADE].RequireLevel;
    REQUIRE(requiredLevel > 0);

    ComputeItemInfo(ITEM_BONE_BLADE);
    CHECK(CountEquippableLevels() == 0);

    // Stat points for every + level, one level short.
    GiveRequiredStats(hero, iMaxLevel);
    hero.Level = static_cast<WORD>(requiredLevel - 1);
    ComputeItemInfo(ITEM_BONE_BLADE);
    CHECK(CountEquippableLevels() == 0);

    hero.Level = static_cast<WORD>(requiredLevel);
    ComputeItemInfo(ITEM_BONE_BLADE);
    CHECK(CountEquippableLevels() == iMaxLevel + 1);

    // Another character, with the stats for +0 only.
    GiveRequiredStats(hero, 0);
    ComputeItemInfo(ITEM_BONE_BLADE);
    CHECK(g_iItemInfo[0][_COLUMN_TYPE_CAN_EQUIP] == TRUE);
    CHECK(g_iItemInfo[iMaxLevel][_COLUMN_TYPE_CAN_EQUIP] == FALSE);
}

TEST_CASE("The item help table shows the item editor's changes to the item it shows [data][items]")
{
    ShippedItemAttributes items;
    ItemHelpScope help;
    constexpr int AddedDamage = 10;

    ComputeItemInfo(ITEM_DOUBLE_BLADE);
    const int shippedDamageMin = g_iItemInfo[0][_COLUMN_TYPE_ATTMIN];

    // The item editor's fields change ItemAttribute first, which the table reads.
    items[ITEM_DOUBLE_BLADE].DamageMin += AddedDamage;

    ComputeItemInfo(ITEM_DOUBLE_BLADE);
    CHECK(g_iItemInfo[0][_COLUMN_TYPE_ATTMIN] == shippedDamageMin + AddedDamage);
    for (int level = 0; level <= iMaxLevel; ++level)
    {
        CAPTURE(level);
        CHECK(g_iItemInfo[level][_COLUMN_TYPE_ATTMIN] == ComputeItem(ITEM_DOUBLE_BLADE, level).DamageMin);
    }
}

TEST_CASE("The item help window builds its content again only when what it shows changes [data][items]")
{
    using mu::ui::window::GetItemHelpContentKey;

    ShippedItemAttributes items;
    ItemHelpScope help;
    help.HeroCharacter().Class = CLASS_WIZARD;
    const auto shown = GetItemHelpContentKey(ITEM_BONE_BLADE);

    CHECK(GetItemHelpContentKey(ITEM_BONE_BLADE) == shown);
    CHECK(GetItemHelpContentKey(ITEM_DOUBLE_BLADE) != shown);

    help.Hero().Level = 1;
    CHECK(GetItemHelpContentKey(ITEM_BONE_BLADE) != shown);
    help.Hero().Level = 0;

    help.Hero().AddStrength = 1;
    CHECK(GetItemHelpContentKey(ITEM_BONE_BLADE) != shown);
    help.Hero().AddStrength = 0;

    help.HeroCharacter().Class = CLASS_KNIGHT;
    CHECK(GetItemHelpContentKey(ITEM_BONE_BLADE) != shown);
    help.HeroCharacter().Class = CLASS_WIZARD;

    // Another language changes the texts and the item names. Back in English
    // the names were taken again; the content then stays until something changes.
    I18N::SetLocale("pt");
    CHECK(GetItemHelpContentKey(ITEM_BONE_BLADE) != shown);
    I18N::SetLocale("en");
    const auto backInEnglish = GetItemHelpContentKey(ITEM_BONE_BLADE);
    CHECK(GetItemHelpContentKey(ITEM_BONE_BLADE) == backInEnglish);

    // An item editor change, which stores the item in the item database.
    g_ItemDatabase.Set(*g_ItemDatabase.Find(ITEM_BONE_BLADE));
    CHECK(GetItemHelpContentKey(ITEM_BONE_BLADE) != backInEnglish);

    // Translated item names turned on or off: only the item names change language.
    const auto namesInEnglish = GetItemHelpContentKey(ITEM_BONE_BLADE);
    g_ItemDatabase.SetDisplayLocale("pt");
    CHECK(GetItemHelpContentKey(ITEM_BONE_BLADE) != namesInEnglish);
    g_ItemDatabase.SetDisplayLocale("en");
}

// Scrolls and orbs keep their stat requirements as the values the game asks for (their profiles
// keep the base value), as OpenMU stores them. These are the values the drop level formula gave
// them before; +0 is the only level they have.
TEST_CASE("Scrolls and orbs ask for their requirements as stored [data][items]")
{
    struct StoredRequirements
    {
        int group;
        int number;
        int strength;
        int dexterity;
        int energy;
        int vitality;
        int leadership;
    };
    constexpr StoredRequirements expected[] = {
        {12, 8, 0, 0, 52, 0, 0},     {12, 9, 0, 0, 72, 0, 0},     {12, 10, 0, 0, 92, 0, 0},
        {12, 16, 596, 0, 0, 0, 0},   {12, 18, 0, 646, 0, 0, 0},   {12, 21, 0, 0, 79, 0, 0},
        {12, 22, 0, 0, 153, 0, 400}, {12, 23, 0, 0, 102, 0, 300}, {12, 24, 0, 0, 126, 0, 340},
        {12, 35, 0, 0, 150, 0, 70},  {12, 46, 0, 0, 168, 0, 0},   {12, 48, 0, 0, 84, 0, 0},
        {15, 0, 0, 0, 140, 0, 0},    {15, 1, 0, 0, 104, 0, 0},    {15, 2, 0, 0, 72, 0, 0},
        {15, 3, 0, 0, 40, 0, 0},     {15, 4, 0, 0, 160, 0, 0},    {15, 5, 0, 0, 88, 0, 0},
        {15, 6, 0, 0, 120, 0, 0},    {15, 7, 0, 0, 180, 0, 0},    {15, 8, 0, 0, 220, 0, 0},
        {15, 9, 0, 0, 260, 0, 0},    {15, 10, 0, 0, 56, 0, 0},    {15, 11, 0, 0, 345, 0, 0},
        {15, 12, 0, 0, 500, 0, 0},   {15, 13, 0, 0, 724, 0, 0},   {15, 14, 0, 0, 644, 0, 0},
        {15, 15, 0, 0, 408, 0, 0},   {15, 16, 0, 0, 953, 0, 0},   {15, 17, 0, 0, 849, 0, 0},
        {15, 18, 0, 0, 1052, 0, 0},  {15, 19, 0, 0, 245, 0, 0},   {15, 20, 0, 0, 150, 0, 0},
        {15, 21, 0, 0, 823, 0, 0},   {15, 22, 0, 0, 375, 0, 0},   {15, 23, 0, 0, 620, 0, 0},
        {15, 24, 0, 0, 180, 0, 0},   {15, 26, 0, 0, 663, 0, 0},   {15, 27, 0, 0, 912, 0, 0},
        {15, 28, 0, 0, 1058, 0, 0},  {15, 29, 0, 0, 1058, 0, 0},  {15, 34, 0, 0, 404, 0, 0},
        {15, 35, 0, 0, 132, 0, 0},   {15, 36, 0, 0, 80, 0, 0},
    };
    ShippedItemAttributes items;
    for (const StoredRequirements& row : expected)
    {
        CAPTURE(row.group);
        CAPTURE(row.number);
        const int itemType = MakeItemType(row.group, row.number);
        const ItemDefinition* item = g_ItemDatabase.Find(itemType);
        REQUIRE(item != nullptr);
        CHECK(item->requirements.strength == row.strength);
        CHECK(item->requirements.dexterity == row.dexterity);
        CHECK(item->requirements.energy == row.energy);
        CHECK(item->requirements.vitality == row.vitality);
        CHECK(item->requirements.leadership == row.leadership);
        const auto values = GameLogic::Items::ComputeItemStats(items[itemType], {itemType, 0, 0, 0, 0});
        CHECK(values.requireStrength == row.strength);
        CHECK(values.requireDexterity == row.dexterity);
        CHECK(values.requireEnergy == row.energy);
        CHECK(values.requireVitality == row.vitality);
        CHECK(values.requireLeadership == row.leadership);
    }
}
