#pragma once

#include <cstdint>

// The skill hotkeys in the key configuration the server stores per character: each hotkey holds a
// skill number, big-endian. 0xFFFF marks a free hotkey; OpenMU seeds new characters with zeros,
// which aren't a skill either, so they must not bind the hotkey to an empty skill slot.
namespace GameLogic::SkillHotkeyConfiguration
{
inline constexpr uint16_t FreeHotkey = 0xFFFF;

inline constexpr uint16_t SkillNumber(uint8_t highByte, uint8_t lowByte)
{
    return static_cast<uint16_t>((highByte << 8) | lowByte);
}

inline constexpr bool IsAssigned(uint16_t skillNumber)
{
    return skillNumber != FreeHotkey && skillNumber != 0;
}
} // namespace GameLogic::SkillHotkeyConfiguration
