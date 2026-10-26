#include "stdafx.h"

#include "Engine/Object/ItemTooltipValues.h"

#include "Character/CharacterManager.h"
#include "Core/Text/Utf8.h"
#include "Core/Utilities/_GlobalFunctions.h"
#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemDescriptionPlaceholders.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Engine/Object/ZzzInventory.h"
#include "GameLogic/Items/ItemAddOptioninfo.h"
#include "GameLogic/Items/ItemCategories.h"
#include "Network/Server/SocketSystem.h"
#include "UI/Core/UIManager.h"
#include "UI/Inventory/UIJewelHarmony.h"

#include <algorithm>
#include <string_view>

namespace
{
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
    if (values.skillBook)
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
    requirement.asked = required != 0;
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
    values.skillBook = item.Type >= ITEM_ETC && item.Type < ITEM_ETC + MAX_ITEM_INDEX;
    values.twoHanded = attribute->TwoHand;
    values.armour = item.Type >= ITEM_HELM && item.Type < ITEM_BOOTS + MAX_ITEM_INDEX;
    values.wizardryWeapon =
        GameLogic::Items::IsStaffOrBookType(item.Type) || GameLogic::Items::IsMagicSwordType(item.Type);
    const int type = item.Type;
    values.hasDurability = ((attribute->Durability || attribute->MagicDur) &&
                            (type < ITEM_WING || type >= ITEM_HELPER) && type < ITEM_POTION) ||
                           (type >= ITEM_WING && type <= ITEM_WINGS_OF_DARKNESS) ||
                           (type >= ITEM_WING_OF_STORM && type <= ITEM_WING_OF_DIMENSION) ||
                           (type >= ITEM_CAPE_OF_FIGHTER && type <= ITEM_CAPE_OF_OVERRULE);
    values.timed = item.bPeriodItem;
    values.hasOptions = item.SpecialNum > 0;
    // No hero yet (the character screen, a tooltip built before the world loads): level 0.
    values.heroLevel = CharacterAttribute != nullptr ? CharacterAttribute->Level : 0;
    values.name = attribute->Name;
    values.levelPlusOne = item.Level + 1;
    values.excellentFlags = item.ExcellentFlags & 63;
    if (g_pItemAddOptioninfo != nullptr)
    {
        const ITEM_ADD_OPTION& addOption = g_pItemAddOptioninfo->GetItemAddOtioninfo(item.Type);
        values.addOptionValue1 = addOption.m_byValue1;
        values.addOptionValue2 = addOption.m_byValue2;
        values.addOptionTime = addOption.m_Time;
    }
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
    {"damageMin", &ItemTooltipValues::damageMin},
    {"damageMax", &ItemTooltipValues::damageMax},
    {"defense", &ItemTooltipValues::defense},
    {"magicDefense", &ItemTooltipValues::magicDefense},
    {"blockRate", &ItemTooltipValues::blockRate},
    {"attackSpeed", &ItemTooltipValues::attackSpeed},
    {"walkSpeed", &ItemTooltipValues::walkSpeed},
    {"magicPower", &ItemTooltipValues::magicPower},
    {"durability", &ItemTooltipValues::durability},
    {"maxDurability", &ItemTooltipValues::maxDurability},
    {"heroLevel", &ItemTooltipValues::heroLevel},
    {"levelPlusOne", &ItemTooltipValues::levelPlusOne},
    {"excellentFlags", &ItemTooltipValues::excellentFlags},
    {"addOptionValue1", &ItemTooltipValues::addOptionValue1},
    {"addOptionValue2", &ItemTooltipValues::addOptionValue2},
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

std::optional<std::string> GetItemTooltipText(const ItemTooltipValues& values, std::string_view name)
{
    if (name == "name")
    {
        return values.name != nullptr ? Core::Text::ToUtf8(values.name) : std::string();
    }
    if (name == "addOptionDuration")
    {
        // Written only here, for the line that shows it: g_StringTime needs the buff system, and every tooltip
        // computes the values each frame. Without the buff system (the tests) it's an empty text.
        if (!g_BuffSystem)
        {
            return std::string();
        }
        std::wstring duration;
        g_StringTime(values.addOptionTime, duration, true);
        return Core::Text::ToUtf8(duration.c_str());
    }
    return std::nullopt;
}

bool GetItemTooltipState(const ItemTooltipValues& values, std::string_view name)
{
    struct Named
    {
        std::string_view name;
        bool ItemTooltipValues::* state;
    };
    static constexpr Named States[] = {
        {"hasDamage", &ItemTooltipValues::hasDamage},
        {"skillBook", &ItemTooltipValues::skillBook},
        {"twoHanded", &ItemTooltipValues::twoHanded},
        {"excellent", &ItemTooltipValues::excellent},
        {"ancient", &ItemTooltipValues::ancient},
        {"damageHarmonyBonus", &ItemTooltipValues::damageHarmonyBonus},
        {"defenseHarmonyBonus", &ItemTooltipValues::defenseHarmonyBonus},
        {"armour", &ItemTooltipValues::armour},
        {"wizardryWeapon", &ItemTooltipValues::wizardryWeapon},
        {"hasDurability", &ItemTooltipValues::hasDurability},
        {"timed", &ItemTooltipValues::timed},
        {"hasOptions", &ItemTooltipValues::hasOptions},
    };
    for (const Named& named : States)
    {
        if (named.name == name)
        {
            return values.*named.state;
        }
    }
    if (name == "strengthReduced")
    {
        return values.strength.reduced;
    }
    if (name == "dexterityReduced")
    {
        return values.dexterity.reduced;
    }
    if (name == "strengthRequired")
    {
        return values.strength.asked;
    }
    if (name == "dexterityRequired")
    {
        return values.dexterity.asked;
    }
    return false;
}

bool AreTooltipConditionsMet(std::span<const Data::Items::TooltipCondition> conditions, int type, int level,
                             const ItemTooltipValues& values)
{
    using Data::Items::TooltipCondition;
    const auto holds = [&](const TooltipCondition& condition)
    {
        switch (condition.kind)
        {
        case TooltipCondition::Kind::Value:
        {
            std::optional<int> value = GetItemTooltipValue(values, condition.name);
            if (!value)
            {
                value = Data::Items::GetPlaceholderValue(condition.name, level, g_ItemDatabase.GetStatProfiles());
            }
            return value && Data::Items::CompareTooltipValue(condition, *value);
        }
        case TooltipCondition::Kind::State:
            return GetItemTooltipState(values, condition.name);
        case TooltipCondition::Kind::Tag:
            return g_ItemDatabase.HasTag(type, condition.tag);
        case TooltipCondition::Kind::Class:
            return Hero != nullptr && gCharacterManager.GetBaseClass(Hero->Class) == condition.number;
        }
        return false;
    };
    return std::all_of(conditions.begin(), conditions.end(),
                       [&](const TooltipCondition& condition) { return holds(condition) != condition.negated; });
}

Data::Items::ItemTextColor GetTooltipColor(std::span<const Data::Items::TooltipColorRule> rules,
                                           Data::Items::ItemTextColor color, int type, int level,
                                           const ItemTooltipValues& values)
{
    for (const Data::Items::TooltipColorRule& rule : rules)
    {
        if (AreTooltipConditionsMet(rule.when, type, level, values))
        {
            return rule.color;
        }
    }
    return color;
}

Data::Items::ItemTextColor GetTooltipLineColor(const Data::Items::TooltipLayoutLine& line, int type, int level,
                                               const ItemTooltipValues& values)
{
    return GetTooltipColor(line.colors, line.color, type, level, values);
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
