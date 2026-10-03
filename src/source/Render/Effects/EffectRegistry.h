#pragma once

#include "EffectDef.h"
#include "Data/GameData/EffectData/EffectCreateParams.h"

#include <span>

// Registry mapping an effect type (a MODEL_* / BITMAP_* enum value) to its
// data-driven EffectDescriptor, looked up by CreateEffect / MoveEffect /
// RenderEffects. Types with no entry fall back to the legacy switch statements,
// so migration can proceed effect by effect.
namespace Render::Effects
{
// Builds the table from the creation values of the effect catalogue and the
// handlers of the code. Called once on the loading screen, after the
// catalogue is loaded (OpenBasicData). A lookup before that builds the
// handlers alone, which are code, and logs that the creation values are
// missing.
// Descriptors returned by Lookup stay valid until the next build.
void BuildRegistry(std::span<const Data::Effects::EffectTypeCreateParams> createParams);

// Returns the descriptor for an effect type, or nullptr if the type has not
// been migrated to the registry yet. A bounds check and one array read.
const EffectDescriptor* Lookup(int type);
} // namespace Render::Effects
