#pragma once

#include "Data/GameData/BuffData/BuffAttributes.h"
#include "Data/GameData/Common/LocalizedString.h"
#include "Data/GameData/ItemData/ItemOptionDefinition.h"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

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

// What the buff raises: an attribute, by OpenMU's Stats name in camelCase ("defenseBase"), or by a name of Webzen's
// item add options where OpenMU has none ("mobility"), and the value it adds, in the item options' value format. The
// value is the one of Webzen's Season 6 data: a rate in percent ("experienceRate" 150 is 150 %).
struct BuffPowerUp
{
    std::string attribute;
    Items::ItemOptionAdd add = Items::ItemOptionAdd::Add;
    Items::ItemOptionValue value;
    // The attribute's index in BuffAttributes, which the buff table resolves when it loads (BuffDatabase::Build), so
    // the character stats summed every frame never compare a name; NoBuffAttribute until then.
    int attributeIndex = NoBuffAttribute;

    // The data only: attributeIndex follows from the name, so a loaded power-up equals an edited one with the same
    // data.
    bool operator==(const BuffPowerUp& other) const
    {
        return attribute == other.attribute && add == other.add && value == other.value;
    }
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
    // How long the buff lasts, in seconds, when the server sends no time; none when only the server knows it (an item
    // of the cash shop lasts as long as it was bought for).
    std::optional<Items::ItemOptionValue> duration;
    // What it raises, in their order: an item's tooltip shows the first as {addOptionValue1}, the second as
    // {addOptionValue2}.
    std::vector<BuffPowerUp> powerUps;
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
