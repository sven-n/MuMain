// One-time check for phase 3c of the data-driven items plan: what the + level, excellent
// options, ancient sets and option level add to damage, magic power, defense rate, defense,
// magic defense and the requirements. This file keeps a copy of the old code
// (item_stats_old.inl) and checks that ComputeItemStats, the item SetItemAttributes fills and
// the item help table give the same values for every item, + level, excellent, ancient and
// option state, also for values the item editor can give. It is not meant to stay: once the
// data is the source of truth, a data change would make it fail.

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
// The option level is 4 bits in the item data packet.
constexpr BYTE OptionLevels[] = {0, 1, 4, 15};
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
    int requireLevel;
    int requireStrength;
    int requireDexterity;
    int requireEnergy;
    int requireVitality;
    int requireLeadership;

    bool operator==(const Stats& other) const = default;
};

std::string Describe(const Stats& stats)
{
    return std::to_string(stats.damageMin) + "-" + std::to_string(stats.damageMax) + " magic " +
           std::to_string(stats.magicPower) + " rate " + std::to_string(stats.blockRate) + " defense " +
           std::to_string(stats.defense) + " magic defense " + std::to_string(stats.magicDefense) + " level " +
           std::to_string(stats.requireLevel) + " str " + std::to_string(stats.requireStrength) + " dex " +
           std::to_string(stats.requireDexterity) + " ene " + std::to_string(stats.requireEnergy) + " vit " +
           std::to_string(stats.requireVitality) + " cmd " + std::to_string(stats.requireLeadership);
}

ITEM MakeItem(int itemType, int level, BYTE excellentFlags, BYTE ancientDiscriminator, BYTE optionLevel)
{
    ITEM item{};
    item.Type = static_cast<short>(itemType);
    item.Level = level;
    item.ExcellentFlags = excellentFlags;
    item.AncientDiscriminator = ancientDiscriminator;
    item.OptionLevel = optionLevel;
    return item;
}

Stats FieldsOf(const ITEM& item)
{
    return {item.DamageMin,        item.DamageMax,     item.MagicPower,      item.SuccessfulBlocking,
            item.Defense,          item.MagicDefense,  item.RequireLevel,    item.RequireStrength,
            item.RequireDexterity, item.RequireEnergy, item.RequireVitality, item.RequireCharisma};
}

// The old SetItemAttributes up to its requirement function, and the strength the option adds.
Stats OldStats(int itemType, int level, BYTE excellentFlags, BYTE ancientDiscriminator, BYTE optionLevel)
{
    ITEM item = MakeItem(itemType, level, excellentFlags, ancientDiscriminator, optionLevel);
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
    OldCalcRequirements(&item, p);
    OldAddOptionStrength(&item);
    return FieldsOf(item);
}

Stats ComputedStats(int itemType, int level, BYTE excellentFlags, BYTE ancientDiscriminator, BYTE optionLevel)
{
    const GameLogic::Items::ItemStatValues values = GameLogic::Items::ComputeItemStats(
        ItemAttribute[itemType], {itemType, level, excellentFlags, ancientDiscriminator, optionLevel});
    return {values.damageMin,        values.damageMax,     values.magicPower,      values.blockRate,
            values.defense,          values.magicDefense,  values.requireLevel,    values.requireStrength,
            values.requireDexterity, values.requireEnergy, values.requireVitality, values.requireLeadership};
}

Stats ItemFieldStats(int itemType, int level, BYTE excellentFlags, BYTE ancientDiscriminator, BYTE optionLevel)
{
    ITEM item = MakeItem(itemType, level, excellentFlags, ancientDiscriminator, optionLevel);
    SetItemAttributes(&item);
    return FieldsOf(item);
}

// Compares the old values with ComputeItemStats and the item for every level, excellent,
// ancient and option state of the item types; returns the number of cases and adds the
// differences.
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
                    for (const BYTE optionLevel : OptionLevels)
                    {
                        ++cases;
                        const Stats old = OldStats(itemType, level, excellentFlags, ancient, optionLevel);
                        const Stats computed = ComputedStats(itemType, level, excellentFlags, ancient, optionLevel);
                        const Stats item = ItemFieldStats(itemType, level, excellentFlags, ancient, optionLevel);
                        if (old == computed && old == item)
                        {
                            continue;
                        }
                        differences.push_back(g_ItemDatabase.GetLogName(itemType) + " +" + std::to_string(level) +
                                              " excellent " + std::to_string(excellentFlags) + " ancient " +
                                              std::to_string(ancient) + " option " + std::to_string(optionLevel) +
                                              ": old " + Describe(old) + ", computed " + Describe(computed) +
                                              ", item " + Describe(item));
                    }
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

TEST_CASE("3c: the item stats and requirements are the same for every item and state [data][items][3c]")
{
    ShippedItems items;
    std::vector<std::string> differences;
    const int cases = CompareItems(ItemTypesWithData([](const ITEM_ATTRIBUTE&) { return true; }), differences);
    CHECK(cases > 1500000);
    ReportDifferences(differences);
}

// The only difference: for the numbers of groups 0 to 11 without item data the old code added
// the option's strength (by group range); the new code has no profile for them.
TEST_CASE("3c: item types without data have no stats or requirements [data][items][3c]")
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
                for (const BYTE optionLevel : {BYTE{0}, BYTE{4}})
                {
                    Stats expectedOld{};
                    expectedOld.requireStrength = itemType < ITEM_WING ? optionLevel * 5 : 0;
                    const Stats old = OldStats(itemType, level, excellentFlags, 1, optionLevel);
                    const Stats computed = ComputedStats(itemType, level, excellentFlags, 1, optionLevel);
                    if (!(old == expectedOld) || !(computed == Stats{}))
                    {
                        differences.push_back(std::to_string(itemType) + ": old " + Describe(old) + ", computed " +
                                              Describe(computed));
                    }
                }
            }
        }
    }
    ReportDifferences(differences);
}

// Values the item editor can give a kind, which no shipped item has. The stat profile does not
// change with them (the editor would let a profile follow the data otherwise).
TEST_CASE("3c: edited values give the same item stats and requirements [data][items][3c]")
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

    SUBCASE("a command and vitality requirement")
    {
        const std::vector<int> itemTypes =
            ItemTypesWithData([](const ITEM_ATTRIBUTE& p) { return p.RequireStrength > 0; });
        for (const int itemType : itemTypes)
        {
            ItemAttribute[itemType].RequireCharisma = 100;
            ItemAttribute[itemType].RequireVitality = 50;
        }
        CompareItems(itemTypes, differences);
    }

    ReportDifferences(differences);
}

// The items the help window shows: groups 0 to 11 without the arrows and bolts, and group 15.
// The window shows only the +0 row of group 15; the old table's other rows showed the base level
// requirement where the item's grows with the + level, so their requirements are left out.
TEST_CASE("3c: the item help table shows the same stats and requirements [data][items][3c]")
{
    ShippedItems items;
    CHARACTER heroCharacter;
    CHARACTER* previousHeroCharacter = ::Hero;
    ::Hero = &heroCharacter;

    std::vector<std::string> differences;
    for (int itemType = 0; itemType < MAX_ITEM; ++itemType)
    {
        const bool etc = itemType >= ITEM_ETC && itemType < ITEM_ETC + MAX_ITEM_INDEX;
        const bool shown = (itemType < ITEM_WING || etc) && !GameLogic::Items::IsAmmunitionType(itemType);
        if (!shown || g_ItemDatabase.Find(itemType) == nullptr)
        {
            continue;
        }
        ComputeItemInfo(itemType);
        for (int level = 0; level <= iMaxLevel; ++level)
        {
            const OldHelpTableStats old = OldComputeItemInfoStats(itemType, level);
            const OldHelpTableRequirements oldRequirements = OldComputeItemInfoRequirements(itemType, level);
            const int* row = g_iItemInfo[level];
            const int magic = row[_COLUMN_TYPE_MAGIC] + row[_COLUMN_TYPE_CURSE] + row[_COLUMN_TYPE_PET_ATTACK];
            const bool sameStats = old.DamageMin == row[_COLUMN_TYPE_ATTMIN] &&
                                   old.DamageMax == row[_COLUMN_TYPE_ATTMAX] && old.Magic == magic &&
                                   old.Defense == row[_COLUMN_TYPE_DEFENCE] &&
                                   old.Blocking == row[_COLUMN_TYPE_DEFRATE];
            const bool sameRequirements =
                (etc && level > 0) || (oldRequirements.RequireLevel == row[_COLUMN_TYPE_REQNLV] &&
                                       oldRequirements.RequireStrength == row[_COLUMN_TYPE_REQSTR] &&
                                       oldRequirements.RequireDexterity == row[_COLUMN_TYPE_REQDEX] &&
                                       oldRequirements.RequireEnergy == row[_COLUMN_TYPE_REQENG] &&
                                       oldRequirements.RequireVitality == row[_COLUMN_TYPE_REQVIT] &&
                                       oldRequirements.RequireCharisma == row[_COLUMN_TYPE_REQCHA]);
            if (!sameStats || !sameRequirements)
            {
                differences.push_back(g_ItemDatabase.GetLogName(itemType) + " +" + std::to_string(level));
            }
        }
    }
    ::Hero = previousHeroCharacter;
    ReportDifferences(differences);
}
