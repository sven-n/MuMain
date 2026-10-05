#pragma once

#ifdef _EDITOR

#include "EffectTypeAsset.h"

namespace MuEditor::Effects
{
// What the slot holds in the game right now: the model and the file
// AccessModel opened into it, or the texture loaded with that number. Loads
// nothing.
EffectAsset ProbeLoadedAsset(EffectAssetSlot slot, int type);
} // namespace MuEditor::Effects

#endif // _EDITOR
