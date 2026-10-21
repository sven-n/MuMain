// One-time check for phase 6 of the data-driven items plan: the profile rules of the item
// stat file replace the code that gave an item the stat profile its data gives
// (DeriveStatProfile). This file keeps a copy of that code (stat_profile_rules_old.inl)
// and checks that the shipped rules give the same profile for every shipped item and for
// every combination of the item fields the code read. It is not meant to stay: once the
// rules are the source of truth, a change of them would make it fail.

#include "stdafx.h"

#include "doctest.h"

#include "Data/DataHandler/ItemData/ItemJsonStorage.h"
#include "Data/GameData/ItemData/ItemAttributeConversion.h"
#include "Data/GameData/ItemData/ItemDataValidation.h"
#include "Data/GameData/ItemData/ItemStatProfiles.h"

#include <algorithm>
#include <array>
#include <filesystem>
#include <string>
#include <tuple>
#include <vector>

#include "stat_profile_rules_old.inl"

using namespace Data::Items;
using OldStatProfiles::OldDeriveStatProfile;
using OldStatProfiles::OldFollowStatProfile;

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

TEST_CASE("Phase 6 one-time: an edited item follows the profile rules as it followed the code [data][items]")
{
    const ItemStatProfiles profiles = LoadShippedProfiles();
    const ItemDataLoadResult items = LoadItemDataDirectory(DataDirectory / "Items");
    size_t compared = 0;
    for (const ItemDefinition& shipped : items.items)
    {
        CAPTURE(shipped.group);
        CAPTURE(shipped.number);
        for (const char* profile : {"", "default", "custom"})
        {
            // The item with its shipped profile is the previous item of the second check below.
            ItemDefinition previous = shipped;
            previous.statProfile = profile;
            std::vector<ItemDefinition> updates(5, previous);
            updates[1].weaponKind = WeaponKind::None;
            updates[2].slot = ItemSlot::Helm;
            updates[3].tags = ItemTagSet{};
            updates[4].group = (previous.group + 1) % MAX_ITEM_TYPE;
            for (const ItemDefinition& updated : updates)
            {
                ++compared;
                CHECK(DeriveStatProfile(profiles, updated) == OldDeriveStatProfile(updated));
                CHECK(FollowStatProfile(profiles, &previous, updated) == OldFollowStatProfile(&previous, updated));
                CHECK(FollowStatProfile(profiles, &shipped, updated) == OldFollowStatProfile(&shipped, updated));
                CHECK(FollowStatProfile(profiles, nullptr, updated) == OldFollowStatProfile(nullptr, updated));
            }
        }
    }
    CHECK(compared == 911 * 3 * 5);
}

TEST_CASE("Phase 6 one-time: the warning about an item without the profile its data gives is the same [data][items]")
{
    const ItemStatProfiles profiles = LoadShippedProfiles();
    const ItemDataLoadResult shipped = LoadItemDataDirectory(DataDirectory / "Items");
    std::vector<ItemDefinition> cleared = shipped.items;
    for (ItemDefinition& item : cleared)
    {
        item.statProfile.clear();
    }

    using Warning = std::tuple<int, int, std::string>;
    const std::vector<ItemDefinition>* const lists[] = {&shipped.items, &cleared};
    for (const std::vector<ItemDefinition>* items : lists)
    {
        std::vector<Warning> expected;
        for (const ItemDefinition& item : *items)
        {
            const std::string derived = OldDeriveStatProfile(item);
            if (item.statProfile.empty() && !derived.empty())
            {
                expected.emplace_back(item.group, item.number,
                                      "has no stat profile, but its data gives \"" + derived +
                                          "\" (\"default\" keeps the default)");
            }
        }
        std::vector<ItemDataIssue> issues;
        ValidateStatProfiles(*items, profiles, "ItemStats.json", issues);
        std::vector<Warning> actual;
        for (const ItemDataIssue& issue : issues)
        {
            if (issue.field == "statProfile" && issue.severity == ItemDataIssueSeverity::Warning)
            {
                actual.emplace_back(issue.group, issue.number, issue.message);
            }
        }
        std::sort(expected.begin(), expected.end());
        std::sort(actual.begin(), actual.end());
        CHECK(actual == expected);
        // Cleared: the 566 items with a profile and the Horn of Fenrir, whose data gives "helper".
        CHECK(expected.size() == (items == &cleared ? 567 : 0));
    }
}
