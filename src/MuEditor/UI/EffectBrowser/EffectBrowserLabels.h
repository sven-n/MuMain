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
const char* Slot(EffectAssetSlot slot);
} // namespace MuEditor::Effects::Labels

#endif // _EDITOR
