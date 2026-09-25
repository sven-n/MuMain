#pragma once

#include "Core/Platform/WinCompat.h"
#include "Core/Globals/_define.h"

#include <array>
#include <string>

namespace Data::Items
{
struct ItemRequirements
{
    WORD level = 0;
    WORD strength = 0;
    WORD dexterity = 0;
    WORD energy = 0;
    WORD vitality = 0;
    WORD leadership = 0;
};

// One item as the client knows it. Filled once at startup and read-only
// afterwards; see ItemDatabase.
struct ItemDefinition
{
    int group = 0;
    int number = 0;

    // Name in the selected language, shown to the player.
    std::wstring name;
    // English name as UTF-8. Logs always use this one, never the translation.
    std::string englishName;

    BYTE width = 0;
    BYTE height = 0;
    BYTE slot = 0;
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

    bool Exists() const { return !name.empty(); }
};
} // namespace Data::Items
