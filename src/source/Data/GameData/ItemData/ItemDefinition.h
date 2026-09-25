#pragma once

#include "Core/Platform/WinCompat.h"
#include "Core/Globals/_define.h"
#include "Data/GameData/Common/LocalizedString.h"

#include <array>
#include <string>

namespace Data::Items
{
// Item slot value of items that cannot be equipped.
constexpr BYTE ItemSlotNone = 255;

struct ItemRequirements
{
    WORD level = 0;
    WORD strength = 0;
    WORD dexterity = 0;
    WORD energy = 0;
    WORD vitality = 0;
    WORD leadership = 0;

    bool operator==(const ItemRequirements& other) const = default;
};

// One item as the client knows it. Filled once at startup and read-only
// afterwards, except for the item editor; see ItemDatabase.
struct ItemDefinition
{
    int group = 0;
    int number = 0;

    // All names of the item, English first. Logs always use the English
    // name (names.GetNeutral()), never a translation.
    LocalizedString names;
    // The name in the current UI locale, shown to the player. Filled by
    // ItemDatabase from names.
    std::wstring name;

    BYTE width = 0;
    BYTE height = 0;
    BYTE slot = ItemSlotNone;
    bool twoHanded = false;
    WORD skill = 0;
    WORD level = 0;

    BYTE durability = 0;
    BYTE magicDurability = 0;
    BYTE damageMin = 0;
    BYTE damageMax = 0;
    BYTE blockRate = 0;
    BYTE defense = 0;
    BYTE magicDefense = 0;
    BYTE attackSpeed = 0;
    BYTE walkSpeed = 0;
    BYTE magicPower = 0;
    BYTE attackType = 0;

    ItemRequirements requirements;
    std::array<BYTE, MAX_CLASS> classRequirements{};
    std::array<BYTE, MAX_RESISTANCE + 1> resistances{};

    BYTE sellValue = 0;
    int buyPrice = 0;

    bool Exists() const
    {
        return !names.IsEmpty();
    }
};
} // namespace Data::Items
