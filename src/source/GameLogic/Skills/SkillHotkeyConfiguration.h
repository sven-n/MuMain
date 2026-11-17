#pragma once

#include <array>
#include <cstdint>

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
} // namespace GameLogic::Skills::HotkeyConfiguration
