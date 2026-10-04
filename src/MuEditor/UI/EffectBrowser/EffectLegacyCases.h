#pragma once

#ifdef _EDITOR

#include <cstdint>
#include <span>

namespace MuEditor::Effects
{
// The switches of ZzzEffect.cpp that have a case for an effect type.
enum LegacyCase : std::uint8_t
{
    CreateCase = 1 << 0, // CreateEffect
    MoveCase = 1 << 1,   // MoveEffect
    RenderCase = 1 << 2, // RenderEffects
};

struct EffectLegacyCases
{
    int type = 0;
    std::uint8_t cases = 0;
};

// The effect types with a case in at least one of the switches, sorted by
// number. At runtime a type the registry does not handle looks the same
// whether its switch has a case or not, so this list is written from the code;
// a test reads the switches and fails when it differs.
std::span<const EffectLegacyCases> GetEffectLegacyCases();

// The LegacyCase flags of `type`, 0 when no switch has a case for it.
std::uint8_t FindEffectLegacyCases(int type);
} // namespace MuEditor::Effects

#endif // _EDITOR
