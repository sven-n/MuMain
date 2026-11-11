#pragma once

#ifdef _EDITOR

#include "EffectPoolSnapshot.h"

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
    EffectPools m_pools;
    EffectPoolSnapshot m_before;
};
} // namespace MuEditor::Effects

#endif // _EDITOR
