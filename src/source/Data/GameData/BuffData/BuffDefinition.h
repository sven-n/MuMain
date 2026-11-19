#pragma once

#include "Data/GameData/Common/LocalizedString.h"

#include <cstdint>
#include <optional>
#include <string>

// One buff or debuff of Data/Buffs/Buffs.json (buffs design, B1, B10, B12): what the game shows for the effect number
// the server sends. See docs/superpowers/specs/2026-11-08-buffs-design.md.
namespace Data::Buffs
{
// The server sends an effect's number as a byte.
constexpr int MaxBuffNumber = 255;
// The highest tier a buff of a family can have; the buff strip shows only the highest tier of a family that is on.
constexpr int MaxBuffTier = 9;
// The longest display name the game shows, in letters; a longer one is cut.
constexpr int MaxDisplayNameLength = 49;

enum class BuffKind : std::uint8_t
{
    Buff,
    Debuff,
};

// The item that gives the buff, by its group and number.
struct BuffItemLink
{
    int group = 0;
    int number = 0;

    bool operator==(const BuffItemLink& other) const = default;
};

struct BuffDefinition
{
    int number = 0;
    // The name other data uses for it: letters and digits.
    std::string name;
    BuffKind kind = BuffKind::Buff;
    // Buffs of one group are the same kind of effect; the cash shop asks whether one of the group is active.
    int group = 0;
    LocalizedString displayName;
    // Its tooltip lines, separated by '\n'; empty for none.
    LocalizedString description;
    std::optional<BuffItemLink> item;
    // The buffs of one family are tiers of the same buff; the strip shows only the highest tier on. Empty for none.
    std::string family;
    int tier = 0;
    // No icon on the buff strip.
    bool hidden = false;
    // The skill a right-click on its icon asks to cancel; 0 when it can't be cancelled.
    int cancelSkill = 0;

    bool operator==(const BuffDefinition& other) const = default;
};
} // namespace Data::Buffs
