#pragma once

#include <array>
#include <cstdint>
#include <span>

// The skill hotkeys in the key configuration the server stores per character: each hotkey holds a
// skill number, big-endian. 0xFFFF marks a free hotkey; OpenMU seeds new characters with zeros,
// which aren't a skill either, so they must not bind the hotkey to an empty skill slot.
namespace GameLogic::Skills::HotkeyConfiguration
{
inline constexpr std::uint16_t FreeHotkey = 0xFFFF;
inline constexpr int HotkeyCount = 10;
inline constexpr int BytesPerHotkey = 2;
// The whole key configuration the client sends: the skill hotkeys first, then the options, Q/W/E/R and the chat box.
inline constexpr int KeyConfigurationSize = 30;
static_assert(HotkeyCount * BytesPerHotkey <= KeyConfigurationSize);

inline constexpr std::uint16_t SkillNumber(std::uint8_t highByte, std::uint8_t lowByte)
{
    return static_cast<std::uint16_t>((highByte << 8) | lowByte);
}

// The bytes of one hotkey as SkillNumber reads them.
inline constexpr std::array<std::uint8_t, BytesPerHotkey> Encode(std::uint16_t skillNumber)
{
    return {static_cast<std::uint8_t>(skillNumber >> 8), static_cast<std::uint8_t>(skillNumber & 0xFF)};
}

inline constexpr bool IsAssigned(std::uint16_t skillNumber)
{
    return skillNumber != FreeHotkey && skillNumber != 0;
}

// The skill-list index of a hotkey without a skill.
inline constexpr int NoSkillIndex = -1;
// The HUD shows the skills of keys 1 to 5 or those of keys 6 to 9 and 0, one row at a time.
inline constexpr int FirstRowFirstHotkey = 1;
inline constexpr int FirstRowLastHotkey = 5;

// The skill selected once the key configuration arrived: the one on the lowest bound key from 1 to 5, so the
// HUD opens on that row with a skill the player put there, whatever order the server lists the skills in.
// `skillIndexOfHotkey` holds each hotkey's skill-list index. NoSkillIndex when keys 1 to 5 are all free.
inline constexpr int InitialSkillIndex(std::span<const int, HotkeyCount> skillIndexOfHotkey)
{
    for (int hotkey = FirstRowFirstHotkey; hotkey <= FirstRowLastHotkey; ++hotkey)
    {
        if (skillIndexOfHotkey[hotkey] != NoSkillIndex)
        {
            return skillIndexOfHotkey[hotkey];
        }
    }

    return NoSkillIndex;
}
} // namespace GameLogic::Skills::HotkeyConfiguration
