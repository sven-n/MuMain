#pragma once

#ifdef _EDITOR

#include "EffectBrowserModel.h"
#include "EffectStages.h"
#include "EffectTypeAsset.h"

#include "Data/GameData/EffectData/EffectKind.h"

#include <string>

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
const char* Assets(AssetFilter filter);
// The name of a world; the login and character screens, which GetMapName
// has no names for, by the editor's scene names.
std::string MapName(int world);
} // namespace MuEditor::Effects::Labels

#endif // _EDITOR
