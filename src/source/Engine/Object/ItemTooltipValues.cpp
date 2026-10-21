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
