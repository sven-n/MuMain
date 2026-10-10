#include "stdafx.h"

#ifdef _EDITOR

#include "EffectBrowserLabels.h"

#include "Core/Text/Utf8.h"
#include "I18N/All.h"
#include "World/MapInfra/MapManager.h"

namespace MuEditor::Effects::Labels
{
using Data::Effects::EffectKind;

const char* Kind(EffectKind kind)
{
    switch (kind)
    {
    case EffectKind::Effect:
        return I18N::Editor::Effects;
    case EffectKind::Particle:
        return I18N::Editor::Particles;
    case EffectKind::Joint:
        return I18N::Editor::LightningAndTrails;
    case EffectKind::Sprite:
        break;
    }
    return I18N::Editor::Sprites;
}

const char* Stage(CreateStage stage)
{
    switch (stage)
    {
    case CreateStage::Data:
        return I18N::Editor::StageData;
    case CreateStage::Hook:
        return I18N::Editor::StageHook;
    case CreateStage::DataThenHook:
        return I18N::Editor::StageDataThenHook;
    case CreateStage::Switch:
        return I18N::Editor::StageSwitch;
    case CreateStage::SetupOnly:
        break;
    }
    return I18N::Editor::StageSetupOnly;
}

const char* Stage(MoveStage stage)
{
    switch (stage)
    {
    case MoveStage::Handler:
        return I18N::Editor::StageHandler;
    case MoveStage::Switch:
        return I18N::Editor::StageSwitch;
    case MoveStage::SharedCodeOnly:
        break;
    }
    return I18N::Editor::StageSharedCodeOnly;
}

const char* Stage(RenderStage stage)
{
    switch (stage)
    {
    case RenderStage::Handler:
        return I18N::Editor::StageHandler;
    case RenderStage::Switch:
        return I18N::Editor::StageSwitch;
    case RenderStage::DrawnAsModel:
        return I18N::Editor::StageDrawnAsModel;
    case RenderStage::OnGround:
        return I18N::Editor::StageOnTheGround;
    case RenderStage::NotDrawn:
        break;
    }
    return I18N::Editor::StageNotDrawn;
}

const char* GroundSuffix(const EffectStages& stages)
{
    return stages.drawnOnGround && stages.render != RenderStage::OnGround ? I18N::Editor::AlsoOnTheGround : "";
}

const char* AfterCharactersSuffix(const EffectStages& stages)
{
    return stages.drawnAfterCharacters ? I18N::Editor::AlsoAfterTheCharacters : "";
}

const char* Slot(EffectAssetSlot slot)
{
    switch (slot)
    {
    case EffectAssetSlot::Model:
        return I18N::Editor::SlotModel;
    case EffectAssetSlot::Texture:
    case EffectAssetSlot::DefaultTexture:
        return I18N::Editor::SlotTexture;
    case EffectAssetSlot::TextureChosenInCode:
        break;
    }
    return I18N::Editor::SlotTextureChosenInCode;
}
const char* Assets(AssetFilter filter)
{
    switch (filter)
    {
    case AssetFilter::LoadedNow:
        return I18N::Editor::LoadedNow;
    case AssetFilter::LoadedAtStart:
        return I18N::Editor::LoadedAtStart;
    case AssetFilter::LoadedByThisMap:
        return I18N::Editor::LoadedByThisMap;
    case AssetFilter::All:
        break;
    }
    return I18N::Editor::AssetsAll;
}

std::string MapName(int world)
{
    if (world == WD_73NEW_LOGIN_SCENE)
        return I18N::Editor::LoginScene;
    if (world == WD_74NEW_CHARACTER_SCENE)
        return I18N::Editor::CharacterScene;
    return world >= 0 ? Core::Text::ToUtf8(gMapManager.GetMapName(world)) : std::string("-");
}
} // namespace MuEditor::Effects::Labels

#endif // _EDITOR
