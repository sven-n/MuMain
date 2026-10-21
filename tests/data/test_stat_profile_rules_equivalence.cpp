// One-time check for phase 6 of the data-driven items plan: the profile rules of the item
// stat file replace the code that gave an item the stat profile its data gives
// (DeriveStatProfile). This file keeps a copy of that code (stat_profile_rules_old.inl)
// and checks that the shipped rules give the same profile for every shipped item and for
// every combination of the item fields the code read. It is not meant to stay: once the
// rules are the source of truth, a change of them would make it fail.

#include "stdafx.h"

#include "doctest.h"

#include "Data/DataHandler/ItemData/ItemJsonStorage.h"
#include "Data/GameData/ItemData/ItemStatProfiles.h"

#include <array>
#include <filesystem>
#include <string>
#include <vector>

#include "stat_profile_rules_old.inl"

using namespace Data::Items;
using OldStatProfiles::OldDeriveStatProfile;

namespace
{
const std::filesystem::path DataDirectory = MU_TEST_DATA_DIR;
const std::filesystem::path ClientDirectory = DataDirectory.parent_path();

ItemStatProfiles LoadShippedProfiles()
{
    const ItemStatsLoadResult stats = LoadItemStatsFile(ClientDirectory / GetItemStatsFile());
    REQUIRE(stats.issues.empty());
    return ItemStatProfiles(stats.file);
}

std::string RuleProfile(const ItemStatProfiles& profiles, const ItemDefinition& item)
{
    return profiles.GetRuleProfile(profiles.FindRule(item));
}
} // namespace

TEST_CASE("Phase 6 one-time: the profile rules give every shipped item the profile the code gave it [data][items]")
{
    const ItemStatProfiles profiles = LoadShippedProfiles();
    const ItemDataLoadResult items = LoadItemDataDirectory(DataDirectory / "Items");
    REQUIRE(items.items.size() == 911);
    for (const ItemDefinition& item : items.items)
    {
        CAPTURE(item.group);
        CAPTURE(item.number);
        CHECK(RuleProfile(profiles, item) == OldDeriveStatProfile(item));
    }
}

TEST_CASE("Phase 6 one-time: the profile rules give the profile the code gave for every combination of the fields "
          "it read [data][items]")
{
    const ItemStatProfiles profiles = LoadShippedProfiles();

    std::vector<ItemSlot> slots = {ItemSlot::None};
    for (size_t slot = 0; slot < ItemSlotCount; ++slot)
    {
        slots.push_back(static_cast<ItemSlot>(slot));
    }
    const std::array<ItemTag, 4> tags = {ItemTag::Ammunition, ItemTag::Sceptre, ItemTag::SummonerBook, ItemTag::Cape};
    std::vector<ItemRequirements> requirements(5);
    requirements[1].level = 20;
    requirements[2].strength = 40;
    requirements[3].level = 1;
    requirements[3].energy = 65535;
    requirements[4].leadership = 5;

    size_t compared = 0;
    size_t mismatches = 0;
    const auto compare = [&](const ItemDefinition& item)
    {
        ++compared;
        if (RuleProfile(profiles, item) != OldDeriveStatProfile(item))
        {
            if (mismatches++ == 0)
            {
                INFO("first mismatch: group " << item.group << ", slot " << static_cast<int>(item.slot)
                                              << ", wing tier " << static_cast<int>(item.wingTier) << ", weapon kind "
                                              << static_cast<int>(item.weaponKind));
                CHECK(RuleProfile(profiles, item) == OldDeriveStatProfile(item));
            }
        }
    };

    ItemDefinition item;
    for (const ItemSlot slot : slots)
    {
        item.slot = slot;
        for (size_t tier = 0; tier < WingTierCount; ++tier)
        {
            item.wingTier = static_cast<WingTier>(tier);
            for (size_t kind = 0; kind < WeaponKindCount; ++kind)
            {
                item.weaponKind = static_cast<WeaponKind>(kind);
                for (unsigned subset = 0; subset < (1u << tags.size()); ++subset)
                {
                    item.tags = ItemTagSet{};
                    for (size_t tag = 0; tag < tags.size(); ++tag)
                    {
                        item.tags.Set(tags[tag], (subset & (1u << tag)) != 0);
                    }
                    for (int group = 0; group < MAX_ITEM_TYPE; ++group)
                    {
                        item.group = group;
                        for (const ItemRequirements& values : requirements)
                        {
                            item.requirements = values;
                            compare(item);
                        }
                    }
                }
            }
        }
    }
    CHECK(compared == 844800);

    // Each base requirement alone, at its lowest and highest value, in the groups the
    // helper and scroll checks read; and a tag the code did not read.
    ItemDefinition other;
    for (const int group : {13, 14, 15})
    {
        other.group = group;
        for (size_t requirement = 0; requirement < ItemRequirementCount; ++requirement)
        {
            for (const WORD value : {WORD{0}, WORD{1}, WORD{65535}})
            {
                other.requirements = ItemRequirements{};
                WORD* const fields[] = {&other.requirements.level,     &other.requirements.strength,
                                        &other.requirements.dexterity, &other.requirements.energy,
                                        &other.requirements.vitality,  &other.requirements.leadership};
                *fields[requirement] = value;
                other.tags = ItemTagSet{};
                compare(other);
                other.tags.Set(ItemTag::Jewel);
                compare(other);
            }
        }
    }
    CHECK(mismatches == 0);
}
