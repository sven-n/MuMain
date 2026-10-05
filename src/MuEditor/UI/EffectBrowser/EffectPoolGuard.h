#pragma once

#ifdef _EDITOR

#include "Core/Globals/_define.h"

#include <array>

namespace MuEditor::Effects
{
// Removes the effects, sprites, particles and joints created while it lives.
// The item effect code that RenderPartObject runs creates them in the game's
// pools; the preview draws an item without them, so they do not show up in
// the world either.
class EffectPoolGuard
{
public:
    EffectPoolGuard();
    ~EffectPoolGuard();

    EffectPoolGuard(const EffectPoolGuard&) = delete;
    EffectPoolGuard& operator=(const EffectPoolGuard&) = delete;

private:
    std::array<bool, MAX_EFFECTS> m_effects{};
    std::array<bool, MAX_SPRITES> m_sprites{};
    std::array<bool, MAX_PARTICLES> m_particles{};
    std::array<bool, MAX_JOINTS> m_joints{};
};
} // namespace MuEditor::Effects

#endif // _EDITOR
