#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>

// The chat box byte of the key configuration the server stores per character (its skill hotkeys: see
// GameLogic/Skills/SkillHotkeyConfiguration.h): the shown lines / 3 in the high 4 bits, the background alpha in
// tenths in the low 4 bits.
namespace GameLogic::KeyConfiguration::ChatBox
{
inline constexpr int MaxLines = 15;
// The box a character starts with (a new one has no stored configuration); the byte must store it as it is, or a
// new character's first save would change it.
inline constexpr int DefaultLines = 6;
inline constexpr float DefaultAlpha = 0.6f;

inline std::uint8_t Encode(int shownLines, float backAlpha)
{
    const int size = std::clamp(shownLines / 3, 0, MaxLines / 3);
    const int alpha = std::clamp(static_cast<int>(std::lround(backAlpha * 10.f)), 0, 10);
    return static_cast<std::uint8_t>((size << 4) | alpha);
}

// The shown lines the byte stores; 0 when it stores none (a zeroed byte, or the original client's chat box off).
inline constexpr int Lines(std::uint8_t chatBox)
{
    const int size = chatBox >> 4;
    return size >= 1 && size <= MaxLines / 3 ? size * 3 : 0;
}

// The background alpha the byte stores; 0 when it stores none.
inline constexpr float Alpha(std::uint8_t chatBox)
{
    const int alpha = chatBox & 0x0F;
    return alpha >= 1 && alpha <= 10 ? static_cast<float>(alpha) / 10.f : 0.f;
}
} // namespace GameLogic::KeyConfiguration::ChatBox
