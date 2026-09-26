#include "stdafx.h"

#include "ItemAttributeConversion.h"
#include "ItemType.h"
#include "Core/Text/Utf8.h"

#include <algorithm>

namespace Data::Items
{
namespace
{
void CopyStats(const ITEM_ATTRIBUTE& attribute, ItemDefinition& definition)
{
    definition.width = attribute.Width;
    definition.height = attribute.Height;
    definition.slot = static_cast<ItemSlot>(attribute.m_byItemSlot);
    definition.twoHanded = attribute.TwoHand;
    definition.skill = attribute.m_wSkillIndex;
    definition.level = attribute.Level;
    definition.durability = attribute.Durability;
    definition.magicDurability = attribute.MagicDur;
    definition.damageMin = attribute.DamageMin;
    definition.damageMax = attribute.DamageMax;
    definition.blockRate = attribute.SuccessfulBlocking;
    definition.defense = attribute.Defense;
    definition.magicDefense = attribute.MagicDefense;
    definition.attackSpeed = attribute.WeaponSpeed;
    definition.walkSpeed = attribute.WalkSpeed;
    definition.magicPower = attribute.MagicPower;
    definition.attackType = attribute.AttType;
    definition.sellValue = attribute.Value;
    definition.buyPrice = attribute.iZen;
}

void CopyRequirements(const ITEM_ATTRIBUTE& attribute, ItemDefinition& definition)
{
    definition.requirements.level = attribute.RequireLevel;
    definition.requirements.strength = attribute.RequireStrength;
    definition.requirements.dexterity = attribute.RequireDexterity;
    definition.requirements.energy = attribute.RequireEnergy;
    definition.requirements.vitality = attribute.RequireVitality;
    definition.requirements.leadership = attribute.RequireCharisma;
    std::copy(std::begin(attribute.RequireClass), std::end(attribute.RequireClass), definition.classRequirements.begin());
    std::copy(std::begin(attribute.Resistance), std::end(attribute.Resistance), definition.resistances.begin());
}

void CopyStats(const ItemDefinition& definition, ITEM_ATTRIBUTE& attribute)
{
    attribute.Width = definition.width;
    attribute.Height = definition.height;
    attribute.m_byItemSlot = static_cast<BYTE>(definition.slot);
    attribute.TwoHand = definition.twoHanded;
    attribute.m_wSkillIndex = definition.skill;
    attribute.Level = definition.level;
    attribute.Durability = definition.durability;
    attribute.MagicDur = definition.magicDurability;
    attribute.DamageMin = definition.damageMin;
    attribute.DamageMax = definition.damageMax;
    attribute.SuccessfulBlocking = definition.blockRate;
    attribute.Defense = definition.defense;
    attribute.MagicDefense = definition.magicDefense;
    attribute.WeaponSpeed = definition.attackSpeed;
    attribute.WalkSpeed = definition.walkSpeed;
    attribute.MagicPower = definition.magicPower;
    attribute.AttType = definition.attackType;
    attribute.Value = definition.sellValue;
    attribute.iZen = definition.buyPrice;
}

void CopyRequirements(const ItemDefinition& definition, ITEM_ATTRIBUTE& attribute)
{
    attribute.RequireLevel = definition.requirements.level;
    attribute.RequireStrength = definition.requirements.strength;
    attribute.RequireDexterity = definition.requirements.dexterity;
    attribute.RequireEnergy = definition.requirements.energy;
    attribute.RequireVitality = definition.requirements.vitality;
    attribute.RequireCharisma = definition.requirements.leadership;
    std::copy(definition.classRequirements.begin(), definition.classRequirements.end(), std::begin(attribute.RequireClass));
    std::copy(definition.resistances.begin(), definition.resistances.end(), std::begin(attribute.Resistance));
}

void CopyName(const std::wstring& name, ITEM_ATTRIBUTE& attribute)
{
    const std::wstring cutName = CutToItemAttributeName(name);
    std::fill(std::begin(attribute.Name), std::end(attribute.Name), L'\0');
    cutName.copy(attribute.Name, cutName.size());
}
} // namespace

std::wstring CutToItemAttributeName(const std::wstring& name)
{
    return name.substr(0, static_cast<size_t>(MAX_ITEM_NAME - 1));
}

std::wstring ReadItemAttributeName(const ITEM_ATTRIBUTE& attribute)
{
    const wchar_t* nameEnd = std::find(std::begin(attribute.Name), std::end(attribute.Name), L'\0');
    return std::wstring(std::begin(attribute.Name), nameEnd);
}

void CopyItemAttributeStats(const ITEM_ATTRIBUTE& attribute, ItemDefinition& definition)
{
    CopyStats(attribute, definition);
    CopyRequirements(attribute, definition);
}

void CopyFieldsNotInItemAttribute(const ItemDefinition& source, ItemDefinition& target)
{
    target.tags = source.tags;
    target.wingTier = source.wingTier;
    target.tradable = source.tradable;
    target.droppable = source.droppable;
    target.storable = source.storable;
    target.sellable = source.sellable;
    target.personalShopSellable = source.personalShopSellable;
    target.repairable = source.repairable;
    target.droppableWhileRented = source.droppableWhileRented;
    target.personalShopSellableWhileRented = source.personalShopSellableWhileRented;
    target.sellableWhenRentalExpired = source.sellableWhenRentalExpired;
}

ItemDefinition ToItemDefinition(const ITEM_ATTRIBUTE& attribute, int itemType)
{
    ItemDefinition definition;
    definition.group = GetItemGroup(itemType);
    definition.number = GetItemNumber(itemType);
    definition.name = ReadItemAttributeName(attribute);
    definition.names.Set(LocalizedString::NeutralLocale, Core::Text::ToUtf8(definition.name.c_str()));
    CopyItemAttributeStats(attribute, definition);
    return definition;
}

void ToItemAttribute(const ItemDefinition& definition, ITEM_ATTRIBUTE& attribute)
{
    CopyName(definition.name, attribute);
    CopyStats(definition, attribute);
    CopyRequirements(definition, attribute);
}
} // namespace Data::Items
