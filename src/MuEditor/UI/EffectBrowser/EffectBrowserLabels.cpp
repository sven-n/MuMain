#include "stdafx.h"

#ifdef _EDITOR

#include "EffectBrowserLabels.h"

#include "I18N/All.h"

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
} // namespace MuEditor::Effects::Labels

#endif // _EDITOR
