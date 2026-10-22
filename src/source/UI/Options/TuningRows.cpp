#include "stdafx.h"

#include "UI/Options/TuningRows.h"

#include "Core/Utilities/StringUtils.h"
#include "I18N/All.h"

namespace UI::Options
{
namespace
{
using Render::Look::TuningControl;
using Render::Look::TuningControlId;
using Render::Look::TuningGroup;
using Render::Look::TuningUnit;
using Render::Look::TuningZero;

std::string Text(const wchar_t* text)
{
    return StringUtils::WideToNarrow(text);
}

struct ControlTexts
{
    const wchar_t* label;
    const wchar_t* help;
};

ControlTexts TextsOf(TuningControlId id)
{
    switch (id)
    {
    case TuningControlId::DiffuseScale:
        return {I18N::Game::TuningDirectionalLight, I18N::Game::TuningDirectionalLightHelp};
    case TuningControlId::RimStrength:
        return {I18N::Game::TuningRimIntensity, I18N::Game::TuningRimIntensityHelp};
    case TuningControlId::SpecularScale:
        return {I18N::Game::TuningMetallicShine, I18N::Game::TuningMetallicShineHelp};
    case TuningControlId::DiffuseAmbient:
        return {I18N::Game::TuningAmbientFill, I18N::Game::TuningAmbientFillHelp};
    case TuningControlId::DiffuseFloor:
        return {I18N::Game::TuningMinimumLight, I18N::Game::TuningMinimumLightHelp};
    case TuningControlId::HighlightKnee:
        return {I18N::Game::TuningHighlightKnee, I18N::Game::TuningHighlightKneeHelp};
    case TuningControlId::RimPower:
        return {I18N::Game::TuningRimConcentration, I18N::Game::TuningRimConcentrationHelp};
    case TuningControlId::SpecularPower:
        return {I18N::Game::TuningShineConcentration, I18N::Game::TuningShineConcentrationHelp};
    case TuningControlId::EffectGain:
        return {I18N::Game::TuningSkillIntensity, I18N::Game::TuningSkillIntensityHelp};
    case TuningControlId::EffectRange:
        return {I18N::Game::TuningSkillRadius, I18N::Game::TuningSkillRadiusHelp};
    case TuningControlId::EffectFade:
        return {I18N::Game::TuningSkillFade, I18N::Game::TuningSkillFadeHelp};
    case TuningControlId::EffectCap:
        return {I18N::Game::TuningOverlappingLightCap, I18N::Game::TuningOverlappingLightCapHelp};
    case TuningControlId::Wrap:
        return {I18N::Game::TuningLightWrap, I18N::Game::TuningLightWrapHelp};
    case TuningControlId::MapSmoothing:
        return {I18N::Game::TuningMapLightSmoothing, I18N::Game::TuningMapLightSmoothingHelp};
    case TuningControlId::EffectRangeMax:
        return {I18N::Game::TuningMaximumSkillRadius, I18N::Game::TuningMaximumSkillRadiusHelp};
    case TuningControlId::MoodStrength:
        return {I18N::Game::TuningMoodStrength, I18N::Game::TuningMoodStrengthHelp};
    case TuningControlId::BloomStrength:
        return {I18N::Game::TuningBloomIntensity, I18N::Game::TuningBloomIntensityHelp};
    case TuningControlId::BloomLevelFalloff:
        return {I18N::Game::TuningBloomSpread, I18N::Game::TuningBloomSpreadHelp};
    case TuningControlId::GradingStrength:
        return {I18N::Game::TuningColourGradingStrength, I18N::Game::TuningColourGradingStrengthHelp};
    case TuningControlId::Count:
        break;
    }
    return {L"", L""};
}

std::string StateText(TuningRowState state)
{
    switch (state)
    {
    case TuningRowState::Custom:
        return Text(I18N::Game::TuningCustom);
    case TuningRowState::Unsaved:
        return Text(I18N::Game::TuningNotSaved);
    case TuningRowState::Inherited:
        break;
    }
    return Text(I18N::Game::TuningMapDefault);
}
} // namespace

TuningLock TuningGroupLock(TuningGroup group, bool enhanced, bool lightingChosen, const TuningFeatureState& feature)
{
    if (!enhanced)
    {
        return TuningLock::ClassicLook;
    }
    if (group == TuningGroup::DynamicLights && !lightingChosen)
    {
        return TuningLock::NeedsLighting;
    }
    if (!feature.chosen)
    {
        return TuningLock::FeatureOff;
    }
    if (feature.sessionOff)
    {
        return TuningLock::SessionOff;
    }
    if (feature.sessionSet)
    {
        return TuningLock::SessionSet;
    }
    if (!feature.active)
    {
        return TuningLock::Unavailable;
    }
    return TuningLock::None;
}

std::string TuningLockReason(TuningLock lock)
{
    switch (lock)
    {
    case TuningLock::ClassicLook:
        return Text(I18N::Game::TuningLockedClassic);
    case TuningLock::FeatureOff:
        return Text(I18N::Game::TuningLockedOff);
    case TuningLock::NeedsLighting:
        return Text(I18N::Game::TuningLockedNeedsLighting);
    case TuningLock::Unavailable:
        return Text(I18N::Game::TuningLockedUnavailable);
    case TuningLock::SessionOff:
        return Text(I18N::Game::TuningLockedSession);
    case TuningLock::SessionSet:
        return Text(I18N::Game::TuningLockedSessionValue);
    case TuningLock::None:
        break;
    }
    return {};
}

std::string FormatTuningText(const TuningControl& control, double value)
{
    if (value == 0.0 && control.zero == TuningZero::NoCap)
    {
        return Text(I18N::Game::TuningNoCap);
    }
    if (value == 0.0 && control.zero == TuningZero::Immediate)
    {
        return Text(I18N::Game::TuningImmediate);
    }
    std::string number = Render::Look::FormatTuningValue(control, value);
    switch (control.unit)
    {
    case TuningUnit::Times:
        // The spelling of the Anti-aliasing row's "4x", which every bundled font has.
        return number + "x";
    case TuningUnit::Seconds:
        return number + " " + Text(I18N::Game::TuningSecondsUnit);
    case TuningUnit::Cells:
        return number + " " + Text(I18N::Game::TuningCellsUnit);
    case TuningUnit::Percent:
        // The spacing of the UI Scale row ("100 %").
        return number + " %";
    case TuningUnit::None:
        break;
    }
    return number;
}

TuningRow BuildTuningRow(const TuningControl& control, double value, TuningRowState state)
{
    const ControlTexts texts = TextsOf(control.id);
    TuningRow row;
    row.control = control.id;
    row.label = Text(texts.label);
    row.help = Text(texts.help);
    row.valueText = FormatTuningText(control, value);
    row.minText = FormatTuningText(control, control.min);
    row.maxText = FormatTuningText(control, control.max);
    row.state = state;
    row.stateText = StateText(state);
    row.fillPercent = static_cast<float>(100.0 * Render::Look::TuningFraction(control, value));
    return row;
}

TuningRowState TuningStateOf(const Render::Look::TuningSession& session, TuningControlId control)
{
    if (session.IsUnsaved(control))
    {
        return TuningRowState::Unsaved;
    }
    return session.Override(control) ? TuningRowState::Custom : TuningRowState::Inherited;
}

std::string TuningMoreSettingsLabel()
{
    return Text(I18N::Game::TuningMoreSettings);
}

std::string TuningResetLabel()
{
    return Text(I18N::Game::TuningReset);
}

std::string TuningResetGroupLabel()
{
    return Text(I18N::Game::TuningResetGroup);
}

std::string TuningResetAllLabel()
{
    return Text(I18N::Game::TuningResetAll);
}

std::string TuningSaveMessage(const Render::Look::TuningSession& session)
{
    if (session.SaveState() != Render::Look::TuningSaveState::Failed)
    {
        return {};
    }
    return Text(I18N::Game::TuningSaveFailed) + " " + session.SaveError();
}

} // namespace UI::Options
