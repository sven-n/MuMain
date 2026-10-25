#include "stdafx.h"

#include "Engine/Object/ItemTooltipValues.h"

#include "Engine/Object/ZzzInfomation.h"
#include "Engine/Object/ZzzInventory.h"
#include "Network/Server/SocketSystem.h"
#include "UI/Core/UIManager.h"
#include "UI/Inventory/UIJewelHarmony.h"

#include <algorithm>
#include <string_view>

namespace
{
// The teleport scrolls have damage in their data but no damage line.
bool HasNoDamageLine(int type)
{
    return type == ITEM_SCROLL_OF_TELEPORT || type == ITEM_SCROLL_OF_TELEPORT_ALLY ||
           type == ITEM_SCROLL_OF_SOUL_BARRIER;
}

// The Jewel of Harmony's bonus of the item, of one kind (StrengthenCapability: 0 requirements, 1 attack, 2 defense).
StrengthenCapability HarmonyBonus(const ITEM& item, int kind)
{
    StrengthenCapability capability{};
    if (item.Level >= item.Jewel_Of_Harmony_OptionLevel)
    {
        g_pUIJewelHarmonyinfo->GetStrengthenCapability(&capability, &item, kind);
    }
    return capability;
}

void ComputeDamage(const ITEM& item, ItemTooltipValues& values)
{
    if (item.DamageMin == 0)
    {
        return;
    }
    int minBonus = 0;
    int maxBonus = 0;
    const StrengthenCapability harmony = HarmonyBonus(item, 1);
    if (harmony.SI_isSP)
    {
        minBonus = harmony.SI_SP.SI_minattackpower;
        maxBonus = harmony.SI_SP.SI_maxattackpower;
    }
    int damageMin = item.DamageMin;
    int damageMax = item.DamageMax;
    values.damageHarmonyBonus = minBonus != 0 || maxBonus != 0;
    values.damageMin = damageMin;
    values.damageMax = damageMax;
    if (item.Type >> 4 == 15 || HasNoDamageLine(item.Type))
    {
        values.hasDamage = damageMin > 0;
        return;
    }
    if (item.Type >= ITEM_ETC && item.Type < ITEM_ETC + MAX_ITEM_INDEX)
    {
        // A skill book shows its skill's wizardry damage.
        const ActionSkillType skillIndex = GetSkillByBook(item.Type);
        if (SkillAttribute != nullptr && skillIndex != AT_SKILL_UNDEFINED &&
            IsValidateSkillIdx(static_cast<INT>(skillIndex)))
        {
            const SKILL_ATTRIBUTE& skill = SkillAttribute[skillIndex];
            damageMin = skill.Damage;
            damageMax = skill.Damage + skill.Damage / 2;
        }
        values.damageMin = damageMin;
        values.damageMax = damageMax;
    }
    else
    {
        // A harmony bonus that lifts the minimum to the maximum shows the maximum twice.
        values.damageMax = damageMax + maxBonus;
        values.damageMin = damageMin + minBonus >= damageMax + maxBonus ? damageMax + maxBonus : damageMin + minBonus;
    }
    values.hasDamage = damageMin > 0;
}

// `heroValue` is nothing without a hero: then nothing is missing.
ItemTooltipValues::Requirement HeroRequirement(int required, std::optional<int> heroValue, int reduction)
{
    ItemTooltipValues::Requirement requirement;
    requirement.required = required - reduction;
    requirement.missing = heroValue && *heroValue < requirement.required ? requirement.required - *heroValue : 0;
    requirement.reduced = reduction != 0;
    return requirement;
}

void ComputeRequirements(const ITEM& item, ItemTooltipValues& values)
{
    // The Jewel of Harmony's and the sockets' strength and agility reductions.
    int strengthReduction = 0;
    int dexterityReduction = 0;
    const StrengthenCapability harmony = HarmonyBonus(item, 0);
    if (harmony.SI_isNB)
    {
        strengthReduction = harmony.SI_NB.SI_force;
        dexterityReduction = harmony.SI_NB.SI_activity;
    }
    // The count comes from the server; a bad one must not read past the item's sockets.
    const int socketCount = std::min<int>(item.SocketCount, MAX_SOCKETS);
    for (int socket = 0; socket < socketCount; ++socket)
    {
        if (item.SocketSeedID[socket] == 38)
        {
            strengthReduction += g_SocketItemMgr.GetSocketOptionValue(&item, socket);
        }
        else if (item.SocketSeedID[socket] == 39)
        {
            dexterityReduction += g_SocketItemMgr.GetSocketOptionValue(&item, socket);
        }
    }

    // The hero's stats as the old code read them (WORD sums); none without a hero.
    const CHARACTER_ATTRIBUTE* hero = CharacterAttribute;
    const auto stat = [hero](auto value) -> std::optional<int>
    { return hero != nullptr ? std::optional<int>(value(*hero)) : std::nullopt; };
    values.level =
        HeroRequirement(item.RequireLevel, stat([](const CHARACTER_ATTRIBUTE& h) { return int(h.Level); }), 0);
    values.strength = HeroRequirement(
        item.RequireStrength,
        stat([](const CHARACTER_ATTRIBUTE& h) { return int(static_cast<WORD>(h.Strength + h.AddStrength)); }),
        strengthReduction);
    values.dexterity = HeroRequirement(
        item.RequireDexterity,
        stat([](const CHARACTER_ATTRIBUTE& h) { return int(static_cast<WORD>(h.Dexterity + h.AddDexterity)); }),
        dexterityReduction);
    values.vitality = HeroRequirement(
        item.RequireVitality,
        stat([](const CHARACTER_ATTRIBUTE& h) { return int(static_cast<WORD>(h.Vitality + h.AddVitality)); }), 0);
    values.energy = HeroRequirement(
        item.RequireEnergy,
        stat([](const CHARACTER_ATTRIBUTE& h) { return int(static_cast<WORD>(h.Energy + h.AddEnergy)); }), 0);
    values.leadership = HeroRequirement(
        item.RequireCharisma,
        stat([](const CHARACTER_ATTRIBUTE& h) { return int(static_cast<WORD>(h.Charisma + h.AddCharisma)); }), 0);
}
} // namespace

namespace Engine::Items::TooltipValues
{
ItemTooltipValues ComputeItemTooltipValues(const ITEM& item)
{
    ITEM_ATTRIBUTE* attribute = &ItemAttribute[item.Type];
    ItemTooltipValues values;
    ComputeDamage(item, values);

    const StrengthenCapability harmony = HarmonyBonus(item, 2);
    const int defenseBonus = harmony.SI_isSD ? harmony.SI_SD.SI_defense : 0;
    values.defense = item.Defense + defenseBonus;
    values.defenseHarmonyBonus = defenseBonus != 0;
    values.magicDefense = item.MagicDefense;
    values.blockRate = item.SuccessfulBlocking;
    values.attackSpeed = attribute->WeaponSpeed;
    values.walkSpeed = attribute->WalkSpeed;
    values.magicPower = item.MagicPower;
    values.durability = item.Durability;
    values.maxDurability = CalcMaxDurability(&item, attribute, item.Level);
    for (size_t resistance = 0; resistance < values.resistances.size(); ++resistance)
    {
        values.resistances[resistance] = attribute->Resistance[resistance] ? item.Level + 1 : 0;
    }
    ComputeRequirements(item, values);
    values.excellent = item.ExcellentFlags > 0;
    values.ancient = item.AncientDiscriminator > 0;
    return values;
}

namespace
{
struct NamedValue
{
    std::string_view name;
    int ItemTooltipValues::* value;
};
constexpr NamedValue PlainValues[] = {
    {"damageMin", &ItemTooltipValues::damageMin},   {"damageMax", &ItemTooltipValues::damageMax},
    {"defense", &ItemTooltipValues::defense},       {"magicDefense", &ItemTooltipValues::magicDefense},
    {"blockRate", &ItemTooltipValues::blockRate},   {"attackSpeed", &ItemTooltipValues::attackSpeed},
    {"walkSpeed", &ItemTooltipValues::walkSpeed},   {"magicPower", &ItemTooltipValues::magicPower},
    {"durability", &ItemTooltipValues::durability}, {"maxDurability", &ItemTooltipValues::maxDurability},
};

constexpr std::string_view ResistanceNames[] = {"iceResistance",  "poisonResistance", "lightningResistance",
                                                "fireResistance", "earthResistance",  "windResistance",
                                                "waterResistance"};

// "required<Stat>" and "missing<Stat>" for each.
struct NamedRequirement
{
    std::string_view requiredName;
    std::string_view missingName;
    ItemTooltipValues::Requirement ItemTooltipValues::* requirement;
};
constexpr NamedRequirement Requirements[] = {
    {"requiredLevel", "missingLevel", &ItemTooltipValues::level},
    {"requiredStrength", "missingStrength", &ItemTooltipValues::strength},
    {"requiredDexterity", "missingDexterity", &ItemTooltipValues::dexterity},
    {"requiredVitality", "missingVitality", &ItemTooltipValues::vitality},
    {"requiredEnergy", "missingEnergy", &ItemTooltipValues::energy},
    {"requiredLeadership", "missingLeadership", &ItemTooltipValues::leadership},
};
} // namespace

std::optional<int> GetItemTooltipValue(const ItemTooltipValues& values, std::string_view name)
{
    for (const NamedValue& named : PlainValues)
    {
        if (named.name == name)
        {
            return values.*named.value;
        }
    }
    for (size_t index = 0; index < std::size(ResistanceNames); ++index)
    {
        if (ResistanceNames[index] == name)
        {
            return values.resistances[index];
        }
    }
    for (const NamedRequirement& named : Requirements)
    {
        if (named.requiredName == name)
        {
            return (values.*named.requirement).required;
        }
        if (named.missingName == name)
        {
            return (values.*named.requirement).missing;
        }
    }
    return std::nullopt;
}

std::vector<std::string_view> GetItemTooltipValueNames()
{
    std::vector<std::string_view> names;
    for (const NamedValue& named : PlainValues)
    {
        names.push_back(named.name);
    }
    names.insert(names.end(), std::begin(ResistanceNames), std::end(ResistanceNames));
    for (const NamedRequirement& named : Requirements)
    {
        names.push_back(named.requiredName);
        names.push_back(named.missingName);
    }
    return names;
}
} // namespace Engine::Items::TooltipValues
