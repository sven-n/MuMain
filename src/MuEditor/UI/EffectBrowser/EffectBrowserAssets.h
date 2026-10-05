#pragma once

#ifdef _EDITOR

#include "EffectTypeAsset.h"

namespace MuEditor::Effects
{
// What the slot holds in the game right now: the model and the file
// AccessModel opened into it, or the texture loaded with that number, and
// the world that was active when it was loaded. Loads nothing.
EffectAsset ProbeLoadedAsset(EffectAssetSlot slot, int type);

// Whether the model slot holds a model with meshes now (BMD::Release clears
// them). Loads nothing.
bool IsModelLoaded(int model);
} // namespace MuEditor::Effects

#endif // _EDITOR
