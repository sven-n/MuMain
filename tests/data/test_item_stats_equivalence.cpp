// One-time check for phase 3c-1 of the data-driven items plan: what the + level, excellent
// options and ancient sets add to damage, magic power, defense rate, defense and magic defense.
// This file keeps a copy of the old code (item_stats_old.inl) and checks that ComputeItemStats,
// the item SetItemAttributes fills and the item help table give the same values for every item,
// + level, excellent and ancient state, also for values the item editor can give. It is not
// meant to stay: once the data is the source of truth, a data change would make it fail.

#include "stdafx.h"

#include "doctest.h"

#include "ClientDataScope.h"

#include "Data/DataHandler/ItemData/ItemDataHandler.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Engine/Object/ZzzInventory.h"
#include "GameLogic/Items/ItemCategories.h"
#include "GameLogic/Items/ItemStats.h"
#include "I18N/All.h"

#include <algorithm>
#include <filesystem>
#include <functional>
#include <string>
#include <vector>

#include "item_stats_old.inl"

using namespace Data::Items;

namespace
{
const std::filesystem::path ClientDirectory = std::filesystem::path(MU_TEST_DATA_DIR).parent_path();

// The + levels 0 to 15 and higher ones: the item level is a byte in the item data packet.
constexpr int Levels[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 20, 31, 127, 255};
// No options, each excellent option (on wings the wing options) alone, and all six.
constexpr BYTE ExcellentFlags[] = {0, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x3F};
constexpr BYTE AncientDiscriminators[] = {0, 1, 2};
constexpr int MaxReportedDifferences = 20;

// Loads the shipped items, and gives SetItemAttributes the hero it reads for the excellent
// option values and the Dark Horse's defense.
class ShippedItems
{
public:
    ShippedItems() : m_client(ClientDirectory), m_previousHero(CharacterAttribute)
    {
        I18N::SetLocale("en");
        std::string errorMessage;
        REQUIRE(g_ItemDataHandler.Load(errorMessage));
        m_hero.Level = 400;
        m_hero.Dexterity = 100;
        CharacterAttribute = &m_hero;
    }

    ~ShippedItems()
    {
        CharacterAttribute = m_previousHero;
    }

    ShippedItems(const ShippedItems&) = delete;
    ShippedItems& operator=(const ShippedItems&) = delete;

private:
    ClientDataScope m_client;
    CHARACTER_ATTRIBUTE m_hero{};
    CHARACTER_ATTRIBUTE* m_previousHero;
};

struct Stats
{
    int damageMin;
    int damageMax;
    int magicPower;
    int blockRate;
    int defense;
    int magicDefense;

    bool operator==(const Stats& other) const = default;
};

std::string Describe(const Stats& stats)
{
    return std::to_string(stats.damageMin) + "-" + std::to_string(stats.damageMax) + " magic " +
           std::to_string(stats.magicPower) + " rate " + std::to_string(stats.blockRate) + " defense " +
           std::to_string(stats.defense) + " magic defense " + std::to_string(stats.magicDefense);
}

ITEM MakeItem(int itemType, int level, BYTE excellentFlags, BYTE ancientDiscriminator)
{
    ITEM item{};
    item.Type = static_cast<short>(itemType);
    item.Level = level;
    item.ExcellentFlags = excellentFlags;
    item.AncientDiscriminator = ancientDiscriminator;
    return item;
}

// The old SetItemAttributes up to its stat functions.
Stats OldStats(int itemType, int level, BYTE excellentFlags, BYTE ancientDiscriminator)
{
    ITEM item = MakeItem(itemType, level, excellentFlags, ancientDiscriminator);
    ITEM_ATTRIBUTE* p = &ItemAttribute[itemType];
    const int excelAddValue = OldGetExcellentAddValue(&item);
    item.DamageMin = p->DamageMin;
    item.DamageMax = p->DamageMax;
    item.SuccessfulBlocking = p->SuccessfulBlocking;
    item.Defense = p->Defense;
    item.MagicDefense = p->MagicDefense;
    item.MagicPower = p->MagicPower;
    OldCalcDamageMin(&item, p, excelAddValue);
    OldCalcDamageMax(&item, p, excelAddValue);
    OldCalcMagicPower(&item, p, excelAddValue);
    OldCalcSuccessfulBlocking(&item, p);
    OldCalcDefense(&item, p);
    return {item.DamageMin, item.DamageMax, item.MagicPower, item.SuccessfulBlocking, item.Defense, item.MagicDefense};
}

Stats ComputedStats(int itemType, int level, BYTE excellentFlags, BYTE ancientDiscriminator)
{
    const GameLogic::Items::ItemStatValues values = GameLogic::Items::ComputeItemStats(
        ItemAttribute[itemType], {itemType, level, excellentFlags, ancientDiscriminator});
    return {values.damageMin, values.damageMax, values.magicPower,
            values.blockRate, values.defense,   values.magicDefense};
}

Stats ItemFieldStats(int itemType, int level, BYTE excellentFlags, BYTE ancientDiscriminator)
{
    ITEM item = MakeItem(itemType, level, excellentFlags, ancientDiscriminator);
    SetItemAttributes(&item);
    return {item.DamageMin, item.DamageMax, item.MagicPower, item.SuccessfulBlocking, item.Defense, item.MagicDefense};
}

// Compares the old values with ComputeItemStats and the item for every level, excellent and
// ancient state of the item types; returns the number of cases and adds the differences.
int CompareItems(const std::vector<int>& itemTypes, std::vector<std::string>& differences)
{
    int cases = 0;
    for (const int itemType : itemTypes)
    {
        for (const int level : Levels)
        {
            for (const BYTE excellentFlags : ExcellentFlags)
            {
                for (const BYTE ancient : AncientDiscriminators)
                {
                    ++cases;
                    const Stats old = OldStats(itemType, level, excellentFlags, ancient);
                    const Stats computed = ComputedStats(itemType, level, excellentFlags, ancient);
                    const Stats item = ItemFieldStats(itemType, level, excellentFlags, ancient);
                    if (old == computed && old == item)
                    {
                        continue;
                    }
                    differences.push_back(g_ItemDatabase.GetLogName(itemType) + " +" + std::to_string(level) +
                                          " excellent " + std::to_string(excellentFlags) + " ancient " +
                                          std::to_string(ancient) + ": old " + Describe(old) + ", computed " +
                                          Describe(computed) + ", item " + Describe(item));
                }
            }
        }
    }
    return cases;
}

std::vector<int> ItemTypesWithData(const std::function<bool(const ITEM_ATTRIBUTE&)>& condition)
{
    std::vector<int> itemTypes;
    for (int itemType = 0; itemType < MAX_ITEM; ++itemType)
    {
        if (g_ItemDatabase.Find(itemType) != nullptr && condition(ItemAttribute[itemType]))
        {
            itemTypes.push_back(itemType);
        }
    }
    return itemTypes;
}

void ReportDifferences(const std::vector<std::string>& differences)
{
    if (!differences.empty())
    {
        const std::string count = std::to_string(differences.size()) + " differences, the first ones:";
        MESSAGE(count);
    }
    for (size_t index = 0; index < differences.size() && index < MaxReportedDifferences; ++index)
    {
        MESSAGE(differences[index]);
    }
    CHECK(differences.empty());
}
} // namespace

TEST_CASE("3c-1: the item stats are the same for every item, level, excellent and ancient state [data][items][3c]")
{
    ShippedItems items;
    std::vector<std::string> differences;
    const int cases = CompareItems(ItemTypesWithData([](const ITEM_ATTRIBUTE&) { return true; }), differences);
    CHECK(cases > 400000);
    ReportDifferences(differences);
}

TEST_CASE("3c-1: item types without data have no stats [data][items][3c]")
{
    ShippedItems items;
    std::vector<std::string> differences;
    for (int itemType = 0; itemType < MAX_ITEM; ++itemType)
    {
        if (g_ItemDatabase.Find(itemType) != nullptr)
        {
            continue;
        }
        for (const int level : {0, 15})
        {
            for (const BYTE excellentFlags : {BYTE{0}, BYTE{0x3F}})
            {
                const Stats old = OldStats(itemType, level, excellentFlags, 1);
                if (!(old == Stats{}) || !(ComputedStats(itemType, level, excellentFlags, 1) == Stats{}))
                {
                    differences.push_back(std::to_string(itemType) + ": " + Describe(old));
                }
            }
        }
    }
    ReportDifferences(differences);
}

// Values the item editor can give a kind, which no shipped item has.
TEST_CASE("3c-1: edited values give the same item stats [data][items][3c]")
{
    ShippedItems items;
    std::vector<std::string> differences;

    SUBCASE("magic defense")
    {
        const std::vector<int> itemTypes = ItemTypesWithData([](const ITEM_ATTRIBUTE& p) { return p.Defense > 0; });
        for (const int itemType : itemTypes)
        {
            ItemAttribute[itemType].MagicDefense = 10;
        }
        CompareItems(itemTypes, differences);
    }

    SUBCASE("a drop level of 0")
    {
        const std::vector<int> itemTypes = ItemTypesWithData(
            [](const ITEM_ATTRIBUTE& p) { return p.DamageMin > 0 || p.MagicPower > 0 || p.Defense > 0; });
        for (const int itemType : itemTypes)
        {
            ItemAttribute[itemType].Level = 0;
        }
        CompareItems(itemTypes, differences);
    }

    SUBCASE("a defense rate and magic power that pass 255")
    {
        const std::vector<int> itemTypes =
            ItemTypesWithData([](const ITEM_ATTRIBUTE& p) { return p.SuccessfulBlocking > 0 || p.MagicPower > 0; });
        for (const int itemType : itemTypes)
        {
            ITEM_ATTRIBUTE& attribute = ItemAttribute[itemType];
            attribute.SuccessfulBlocking = attribute.SuccessfulBlocking > 0 ? 250 : 0;
            attribute.MagicPower = attribute.MagicPower > 0 ? 250 : 0;
        }
        CompareItems(itemTypes, differences);
    }

    SUBCASE("maximum damage without minimum damage")
    {
        const std::vector<int> itemTypes = ItemTypesWithData([](const ITEM_ATTRIBUTE& p) { return p.DamageMin > 0; });
        for (const int itemType : itemTypes)
        {
            ItemAttribute[itemType].DamageMin = 0;
        }
        CompareItems(itemTypes, differences);
    }

    ReportDifferences(differences);
}

// The items the help window shows: groups 0 to 11 without the arrows and bolts, and group 15.
TEST_CASE("3c-1: the item help table shows the same stats [data][items][3c]")
{
    ShippedItems items;
    CHARACTER heroCharacter;
    CHARACTER* previousHeroCharacter = ::Hero;
    ::Hero = &heroCharacter;

    std::vector<std::string> differences;
    for (int itemType = 0; itemType < MAX_ITEM; ++itemType)
    {
        const bool shown = (itemType < ITEM_WING || (itemType >= ITEM_ETC && itemType < ITEM_ETC + MAX_ITEM_INDEX)) &&
                           !GameLogic::Items::IsAmmunitionType(itemType);
        if (!shown || g_ItemDatabase.Find(itemType) == nullptr)
        {
            continue;
        }
        ComputeItemInfo(itemType);
        for (int level = 0; level <= iMaxLevel; ++level)
        {
            const OldHelpTableStats old = OldComputeItemInfoStats(itemType, level);
            const int* row = g_iItemInfo[level];
            const int magic = row[_COLUMN_TYPE_MAGIC] + row[_COLUMN_TYPE_CURSE] + row[_COLUMN_TYPE_PET_ATTACK];
            if (old.DamageMin != row[_COLUMN_TYPE_ATTMIN] || old.DamageMax != row[_COLUMN_TYPE_ATTMAX] ||
                old.Magic != magic || old.Defense != row[_COLUMN_TYPE_DEFENCE] ||
                old.Blocking != row[_COLUMN_TYPE_DEFRATE])
            {
                differences.push_back(g_ItemDatabase.GetLogName(itemType) + " +" + std::to_string(level));
            }
        }
    }
    ::Hero = previousHeroCharacter;
    ReportDifferences(differences);
}
