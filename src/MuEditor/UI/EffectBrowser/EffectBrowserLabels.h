#pragma once

#ifdef _EDITOR

#include "EffectStages.h"
#include "EffectTypeAsset.h"

#include "Data/GameData/EffectData/EffectKind.h"

// The translated texts the effect browser shows for kinds, stages and slots.
namespace MuEditor::Effects::Labels
{
// Joints are "Lightning and trails" in the editor (D29).
const char* Kind(Data::Effects::EffectKind kind);
const char* Stage(CreateStage stage);
const char* Stage(MoveStage stage);
const char* Stage(RenderStage stage);
// Follow the drawing stage when RenderEffectShadows draws the effect on the
// ground besides it, or RenderAfterEffects again after the characters; empty
// otherwise.
const char* GroundSuffix(const EffectStages& stages);
const char* AfterCharactersSuffix(const EffectStages& stages);
const char* Slot(EffectAssetSlot slot);
} // namespace MuEditor::Effects::Labels

#endif // _EDITOR
