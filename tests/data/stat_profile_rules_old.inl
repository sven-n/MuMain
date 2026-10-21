// A copy of the code that gave an item the stat profile its data gives before the
// profile rules of the item stat file (src/source/Data/GameData/ItemData/
// ItemAttributeConversion.cpp: GetWingStatProfile, DeriveStatProfile and
// FollowStatProfile), with the profile names as text. Only for the one-time check in
// test_stat_profile_rules_equivalence.cpp.

#pragma once

#include "Data/GameData/ItemData/ItemDefinition.h"

#include <string>

namespace OldStatProfiles
{
using namespace Data::Items;

inline std::string OldGetWingStatProfile(const ItemDefinition& definition)
{
    switch (definition.wingTier)
    {
    case WingTier::Small:
        return "smallWing";
    case WingTier::First:
        return "firstTierWing";
    case WingTier::Second:
        return definition.tags.Has(ItemTag::Cape) ? "secondTierCape" : "secondTierWing";
    case WingTier::Third:
        return "thirdTierWing";
    default:
        return "";
    }
}

inline std::string OldDeriveStatProfile(const ItemDefinition& definition)
{
    constexpr int HelperGroup = 13;
    constexpr int PotionGroup = 14;
    constexpr int EtcGroup = 15;

    if (definition.tags.Has(ItemTag::Ammunition))
    {
        return "";
    }
    if (definition.tags.Has(ItemTag::Sceptre))
    {
        return "sceptre";
    }
    if (definition.tags.Has(ItemTag::SummonerBook))
    {
        return "book";
    }
    if (definition.weaponKind == WeaponKind::Shield)
    {
        return "shield";
    }
    if (definition.weaponKind != WeaponKind::None)
    {
        return "weapon";
    }
    switch (definition.slot)
    {
    case ItemSlot::Helm:
    case ItemSlot::Armor:
    case ItemSlot::Pants:
    case ItemSlot::Gloves:
    case ItemSlot::Boots:
        return "armor";
    default:
        break;
    }
    if (definition.slot == ItemSlot::Wings)
    {
        return OldGetWingStatProfile(definition);
    }
    if (definition.group == EtcGroup && definition.requirements != ItemRequirements{})
    {
        return "scroll";
    }
    if ((definition.group == HelperGroup || definition.group == PotionGroup) && definition.requirements.level > 0)
    {
        return "helper";
    }
    return "";
}

inline std::string OldFollowStatProfile(const ItemDefinition* previous, const ItemDefinition& updated)
{
    if (previous != nullptr && !previous->statProfile.empty() &&
        previous->statProfile != OldDeriveStatProfile(*previous))
    {
        return previous->statProfile;
    }
    return OldDeriveStatProfile(updated);
}
} // namespace OldStatProfiles
