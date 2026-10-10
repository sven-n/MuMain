#pragma once

#include "Data/GameData/ItemData/ItemDefinition.h"

#include <span>
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
std::span<const EnumName<ItemTag>> GetEnumNames(ItemTag);

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
