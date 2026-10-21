#include "stdafx.h"

#include "Engine/Object/ItemTooltipValues.h"

#include "Data/GameData/ItemData/ItemDatabase.h"
#include "Data/GameData/ItemData/ItemDescriptionPlaceholders.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Engine/Object/ZzzInventory.h"
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

ItemTooltipValues::Requirement HeroRequirement(int required, int heroValue, int reduction)
{
    ItemTooltipValues::Requirement requirement;
    requirement.required = required - reduction;
    requirement.missing = heroValue < requirement.required ? requirement.required - heroValue : 0;
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
    for (int socket = 0; socket < item.SocketCount; ++socket)
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

    const CHARACTER_ATTRIBUTE& hero = *CharacterAttribute;
    values.level = HeroRequirement(item.RequireLevel, hero.Level, 0);
    values.strength =
        HeroRequirement(item.RequireStrength, static_cast<WORD>(hero.Strength + hero.AddStrength), strengthReduction);
    values.dexterity = HeroRequirement(item.RequireDexterity, static_cast<WORD>(hero.Dexterity + hero.AddDexterity),
                                       dexterityReduction);
    values.vitality = HeroRequirement(item.RequireVitality, static_cast<WORD>(hero.Vitality + hero.AddVitality), 0);
    values.energy = HeroRequirement(item.RequireEnergy, static_cast<WORD>(hero.Energy + hero.AddEnergy), 0);
    values.leadership = HeroRequirement(item.RequireCharisma, static_cast<WORD>(hero.Charisma + hero.AddCharisma), 0);
}
} // namespace

ItemTooltipValues ComputeItemTooltipValues(const ITEM& item)
{
    ITEM_ATTRIBUTE* attribute = &ItemAttribute[item.Type];
    ItemTooltipValues values;
    values.skillBook = item.Type >= ITEM_ETC && item.Type < ITEM_ETC + MAX_ITEM_INDEX;
    values.twoHanded = attribute->TwoHand;
    values.armour = item.Type >= ITEM_HELM && item.Type < ITEM_BOOTS + MAX_ITEM_INDEX;
    values.wizardryWeapon =
        GameLogic::Items::IsStaffOrBookType(item.Type) || GameLogic::Items::IsMagicSwordType(item.Type);
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

std::optional<int> GetItemTooltipValue(const ItemTooltipValues& values, std::string_view name)
{
    struct Named
    {
        std::string_view name;
        int ItemTooltipValues::* value;
    };
    static constexpr Named Plain[] = {
        {"damageMin", &ItemTooltipValues::damageMin},   {"damageMax", &ItemTooltipValues::damageMax},
        {"defense", &ItemTooltipValues::defense},       {"magicDefense", &ItemTooltipValues::magicDefense},
        {"blockRate", &ItemTooltipValues::blockRate},   {"attackSpeed", &ItemTooltipValues::attackSpeed},
        {"walkSpeed", &ItemTooltipValues::walkSpeed},   {"magicPower", &ItemTooltipValues::magicPower},
        {"durability", &ItemTooltipValues::durability}, {"maxDurability", &ItemTooltipValues::maxDurability},
    };
    for (const Named& named : Plain)
    {
        if (named.name == name)
        {
            return values.*named.value;
        }
    }

    static constexpr std::string_view Resistances[] = {"iceResistance",  "poisonResistance", "lightningResistance",
                                                       "fireResistance", "earthResistance",  "windResistance",
                                                       "waterResistance"};
    for (size_t index = 0; index < std::size(Resistances); ++index)
    {
        if (Resistances[index] == name)
        {
            return values.resistances[index];
        }
    }

    struct NamedRequirement
    {
        std::string_view stat;
        ItemTooltipValues::Requirement ItemTooltipValues::* requirement;
    };
    static constexpr NamedRequirement Requirements[] = {
        {"Level", &ItemTooltipValues::level},         {"Strength", &ItemTooltipValues::strength},
        {"Dexterity", &ItemTooltipValues::dexterity}, {"Vitality", &ItemTooltipValues::vitality},
        {"Energy", &ItemTooltipValues::energy},       {"Leadership", &ItemTooltipValues::leadership},
    };
    for (const NamedRequirement& named : Requirements)
    {
        if (name.starts_with("required") && name.substr(8) == named.stat)
        {
            return (values.*named.requirement).required;
        }
        if (name.starts_with("missing") && name.substr(7) == named.stat)
        {
            return (values.*named.requirement).missing;
        }
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
            return value.value_or(0) != 0;
        }
        case TooltipCondition::Kind::State:
            return GetItemTooltipState(values, condition.name);
        case TooltipCondition::Kind::Tag:
            return g_ItemDatabase.HasTag(type, condition.tag);
        }
        return false;
    };
    return std::all_of(conditions.begin(), conditions.end(),
                       [&](const TooltipCondition& condition) { return holds(condition) != condition.negated; });
}

Data::Items::ItemTextColor GetTooltipLineColor(const Data::Items::TooltipLayoutLine& line, int type, int level,
                                               const ItemTooltipValues& values)
{
    for (const Data::Items::TooltipColorRule& rule : line.colors)
    {
        if (AreTooltipConditionsMet(rule.when, type, level, values))
        {
            return rule.color;
        }
    }
    return line.color;
}
