#include "stdafx.h"

#include "doctest.h"

#include "ClientDataScope.h"

#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemType.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Engine/Object/ZzzInventory.h"
#include "GameLogic/Items/ItemCategories.h"
#include "I18N/All.h"
#include "UI/Inventory/ItemHelpContentKey.h"

#include <algorithm>
#include <array>
#include <filesystem>
#include <iterator>
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

ITEM ComputeItem(int itemType, int level, BYTE excellentFlags = 0, BYTE ancientDiscriminator = 0)
{
    ITEM item{};
    item.Type = static_cast<short>(itemType);
    item.Level = level;
    item.ExcellentFlags = excellentFlags;
    item.AncientDiscriminator = ancientDiscriminator;
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

TEST_CASE("Magic power and defense rate past 255 are not wrapped around [data][items]")
{
    ShippedItemAttributes items;
    // Values the editor can give an item; no shipped item gets this high.
    items[ITEM_LEGENDARY_STAFF].MagicPower = 200;
    items[ITEM_LEGENDARY_STAFF].Level = 80;
    items[DragonShieldType].SuccessfulBlocking = 170;
    items[DragonShieldType].Level = 100;

    // 200 + 67 (excellent) + 66 (+15) = 333, halved, + 30.
    CHECK(ComputeItem(ITEM_LEGENDARY_STAFF, 15, 0x01).MagicPower == 196);
    // 170 + 47 (excellent) + 66 (+15) = 283.
    CHECK(ComputeItem(DragonShieldType, 15, 0x01).SuccessfulBlocking == 255);
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
        const int magicColumn = IsCepterItem(type) ? _COLUMN_TYPE_PET_ATTACK : _COLUMN_TYPE_MAGIC;
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

    I18N::SetLocale("pt");
    CHECK(GetItemHelpContentKey(ITEM_BONE_BLADE) != shown);
    I18N::SetLocale("en");
    CHECK(GetItemHelpContentKey(ITEM_BONE_BLADE) == shown);

    // An item editor change, which stores the item in the item database.
    g_ItemDatabase.Set(*g_ItemDatabase.Find(ITEM_BONE_BLADE));
    CHECK(GetItemHelpContentKey(ITEM_BONE_BLADE) != shown);
}
