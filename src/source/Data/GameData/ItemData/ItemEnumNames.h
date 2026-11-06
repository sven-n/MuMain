#pragma once

#include "Data/GameData/ItemData/ItemDefinition.h"

#include <span>
#include <string>
#include <string_view>

// The names of the item enums in the item files (and later in the editors).
namespace Data::Items
{
template <typename TEnum> struct EnumName
{
    TEnum value;
    const char* name;
};

std::span<const EnumName<ItemSlot>> GetEnumNames(ItemSlot);
std::span<const EnumName<WingTier>> GetEnumNames(WingTier);
std::span<const EnumName<WeaponKind>> GetEnumNames(WeaponKind);
std::span<const EnumName<ItemTag>> GetEnumNames(ItemTag);
std::span<const EnumName<ItemTextColor>> GetEnumNames(ItemTextColor);
// The keys of "requirements" in the item files.
std::span<const EnumName<ItemRequirement>> GetEnumNames(ItemRequirement);

// nullptr when the value has no name.
template <typename TEnum> const char* FindEnumName(TEnum value)
{
    for (const EnumName<TEnum>& entry : GetEnumNames(TEnum{}))
    {
        if (entry.value == value)
        {
            return entry.name;
        }
    }
    return nullptr;
}

// Lists the names of an enum for error messages: "a", "b" or "c".
template <typename TEnum> std::string JoinEnumNames()
{
    std::string text;
    const auto names = GetEnumNames(TEnum{});
    for (size_t i = 0; i < names.size(); ++i)
    {
        text += i == 0 ? "" : (i + 1 == names.size() ? " or " : ", ");
        text += std::string("\"") + names[i].name + "\"";
    }
    return text;
}

template <typename TEnum> bool FindEnumValue(std::string_view name, TEnum& value)
{
    for (const EnumName<TEnum>& entry : GetEnumNames(TEnum{}))
    {
        if (name == entry.name)
        {
            value = entry.value;
            return true;
        }
    }
    return false;
}
} // namespace Data::Items
