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
#include "GameLogic/Items/CComGem.h"
#include "GameLogic/Items/ItemAddOptioninfo.h"
#include "GameLogic/Items/ItemCategories.h"
#include "I18N/All.h"
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
    values.bundleCount = COMGEM::CalcCompiledCount(&item);
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

std::optional<int> GetItemTooltipValue(const ItemTooltipValues& values, Data::Items::TooltipValue value)
{
    using Data::Items::TooltipValue;
    switch (value)
    {
    case TooltipValue::DamageMin:
        return values.damageMin;
    case TooltipValue::DamageMax:
        return values.damageMax;
    case TooltipValue::Defense:
        return values.defense;
    case TooltipValue::MagicDefense:
        return values.magicDefense;
    case TooltipValue::BlockRate:
        return values.blockRate;
    case TooltipValue::AttackSpeed:
        return values.attackSpeed;
    case TooltipValue::WalkSpeed:
        return values.walkSpeed;
    case TooltipValue::MagicPower:
        return values.magicPower;
    case TooltipValue::Durability:
        return values.durability;
    case TooltipValue::MaxDurability:
        return values.maxDurability;
    case TooltipValue::IceResistance:
    case TooltipValue::PoisonResistance:
    case TooltipValue::LightningResistance:
    case TooltipValue::FireResistance:
    case TooltipValue::EarthResistance:
    case TooltipValue::WindResistance:
    case TooltipValue::WaterResistance:
        return values.resistances[static_cast<size_t>(value) - static_cast<size_t>(TooltipValue::IceResistance)];
    case TooltipValue::RequiredLevel:
        return values.level.required;
    case TooltipValue::RequiredStrength:
        return values.strength.required;
    case TooltipValue::RequiredDexterity:
        return values.dexterity.required;
    case TooltipValue::RequiredVitality:
        return values.vitality.required;
    case TooltipValue::RequiredEnergy:
        return values.energy.required;
    case TooltipValue::RequiredLeadership:
        return values.leadership.required;
    case TooltipValue::MissingLevel:
        return values.level.missing;
    case TooltipValue::MissingStrength:
        return values.strength.missing;
    case TooltipValue::MissingDexterity:
        return values.dexterity.missing;
    case TooltipValue::MissingVitality:
        return values.vitality.missing;
    case TooltipValue::MissingEnergy:
        return values.energy.missing;
    case TooltipValue::MissingLeadership:
        return values.leadership.missing;
    case TooltipValue::AddOptionValue1:
        return values.addOptionValue1;
    case TooltipValue::AddOptionValue2:
        return values.addOptionValue2;
    case TooltipValue::HeroLevel:
        return values.heroLevel;
    case TooltipValue::LevelPlusOne:
        return values.levelPlusOne;
    case TooltipValue::ExcellentFlags:
        return values.excellentFlags;
    case TooltipValue::BundleCount:
        return values.bundleCount;
    case TooltipValue::Level:
    case TooltipValue::AddOptionDuration:
    case TooltipValue::Name:
    case TooltipValue::Count:
        break;
    }
    return std::nullopt;
}

const wchar_t* GetItemTooltipText(const ItemTooltipValues& values, Data::Items::TooltipValue value)
{
    if (value == Data::Items::TooltipValue::Name)
    {
        return values.name != nullptr ? values.name : L"";
    }
    if (value == Data::Items::TooltipValue::AddOptionDuration)
    {
        // Written only here, for a line that shows it: g_StringTime needs the buff system. The text changes with the
        // time and with the UI's language (its words for hours, minutes and seconds), so it is written again only when
        // either does. Without the buff system it's an empty text.
        if (!g_BuffSystem)
        {
            return L"";
        }
        static DWORD durationTime = 0;
        static std::string durationLocale;
        static std::wstring duration;
        const char* const locale = I18N::GetCurrentLocale();
        if (duration.empty() || values.addOptionTime != durationTime || durationLocale != locale)
        {
            duration.clear();
            g_StringTime(values.addOptionTime, duration, true);
            durationTime = values.addOptionTime;
            durationLocale = locale;
        }
        return duration.c_str();
    }
    return nullptr;
}

bool GetItemTooltipState(const ItemTooltipValues& values, Data::Items::TooltipState state)
{
    using Data::Items::TooltipState;
    switch (state)
    {
    case TooltipState::HasDamage:
        return values.hasDamage;
    case TooltipState::SkillBook:
        return values.skillBook;
    case TooltipState::TwoHanded:
        return values.twoHanded;
    case TooltipState::Excellent:
        return values.excellent;
    case TooltipState::Ancient:
        return values.ancient;
    case TooltipState::DamageHarmonyBonus:
        return values.damageHarmonyBonus;
    case TooltipState::DefenseHarmonyBonus:
        return values.defenseHarmonyBonus;
    case TooltipState::Armour:
        return values.armour;
    case TooltipState::StrengthReduced:
        return values.strength.reduced;
    case TooltipState::DexterityReduced:
        return values.dexterity.reduced;
    case TooltipState::StrengthRequired:
        return values.strength.asked;
    case TooltipState::DexterityRequired:
        return values.dexterity.asked;
    case TooltipState::WizardryWeapon:
        return values.wizardryWeapon;
    case TooltipState::HasDurability:
        return values.hasDurability;
    case TooltipState::Timed:
        return values.timed;
    case TooltipState::HasOptions:
        return values.hasOptions;
    case TooltipState::Count:
        break;
    }
    return false;
}

std::optional<int> GetItemTooltipValue(const ItemTooltipValues& values, std::string_view name)
{
    const std::optional<Data::Items::TooltipValue> value = Data::Items::FindBuiltInPlaceholder(name);
    return value ? GetItemTooltipValue(values, *value) : std::nullopt;
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
    const std::optional<Data::Items::TooltipState> state = Data::Items::FindTooltipState(name);
    return state && GetItemTooltipState(values, *state);
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
            const std::optional<int> value = condition.value == Data::Items::TooltipValue::Level
                                                 ? std::optional<int>(level)
                                                 : GetItemTooltipValue(values, condition.value);
            return value && Data::Items::CompareTooltipValue(condition, *value);
        }
        case TooltipCondition::Kind::State:
            return GetItemTooltipState(values, condition.state);
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
    // The built-in values that are numbers, by the names the placeholder checks give them.
    const ItemTooltipValues values;
    std::vector<std::string_view> names;
    for (size_t index = 0; index < static_cast<size_t>(Data::Items::TooltipValue::Count); ++index)
    {
        const auto value = static_cast<Data::Items::TooltipValue>(index);
        if (GetItemTooltipValue(values, value).has_value())
        {
            names.push_back(Data::Items::GetBuiltInPlaceholderName(value));
        }
    }
    return names;
}
} // namespace Engine::Items::TooltipValues
