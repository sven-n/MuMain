#pragma once

#include "EffectDef.h"

// Registry mapping an effect type (a MODEL_* / BITMAP_* enum value) to its
// data-driven EffectDescriptor. The table is built once on first use and looked
// up by CreateEffect / MoveEffect / RenderEffects. Types with no entry fall back
// to the legacy switch statements, so migration can proceed effect by effect.
namespace Render::Effects
{
    // Returns the descriptor for an effect type, or nullptr if the type has not
    // been migrated to the registry yet. Lookup is O(1) (type-indexed table).
    const EffectDescriptor* Lookup(int type);
}
