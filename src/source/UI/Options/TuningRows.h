#pragma once

#include "Render/Look/TuningControls.h"
#include "Render/Look/TuningSession.h"

#include <array>
#include <optional>
#include <string>

// What the options window shows for the player's Enhanced tuning (Render/Look/TuningControls.h):
// one row per control, under the switch of the feature it tunes, and why a group cannot be changed
// right now. The texts are localized (I18N::Game); the window binds the result to its RmlUi model.
namespace UI::Options
{

// Why a group's sliders do not apply right now; reset stays available and the values are kept.
enum class TuningLock : std::uint8_t
{
    None,
    // The Classic look is active.
    ClassicLook,
    // The player switched the feature off.
    FeatureOff,
    // Dynamic lights act only with Enhanced lighting.
    NeedsLighting,
    // The renderer could not build the feature here.
    Unavailable,
    // A developer command switched the feature off for this session.
    SessionOff,
    // A developer command set the group's value for this session ($lightmood's strength).
    SessionSet,
};

// The state of one feature for its group's lock.
struct TuningFeatureState
{
    bool chosen = true;
    // Drawn this frame (chosen, available and not switched off for the session).
    bool active = true;
    bool sessionOff = false;
    bool sessionSet = false;
};

// `enhanced`: the Enhanced look is active; `lightingChosen`: the player keeps Enhanced lighting on
// (dynamic lights need it).
[[nodiscard]] TuningLock TuningGroupLock(Render::Look::TuningGroup group, bool enhanced, bool lightingChosen,
                                         const TuningFeatureState& feature);
[[nodiscard]] std::string TuningLockReason(TuningLock lock);

enum class TuningRowState : std::uint8_t
{
    // No player value: the look profile's and the map's.
    Inherited,
    Custom,
    // A player value the configuration does not hold yet (not saved, or the save failed).
    Unsaved,
};

struct TuningRow
{
    Render::Look::TuningControlId control = Render::Look::TuningControlId::DiffuseScale;
    std::string label;
    std::string help;
    // With its unit, or "No cap" / "Immediate" at zero where that is what zero means.
    std::string valueText;
    std::string minText;
    std::string maxText;
    std::string stateText;
    TuningRowState state = TuningRowState::Inherited;
    // Where the value is on the track, 0 to 100.
    float fillPercent = 0.f;

    bool operator==(const TuningRow&) const = default;
};

// A value of a control as the window shows it, with its unit.
[[nodiscard]] std::string FormatTuningText(const Render::Look::TuningControl& control, double value);

// The row of a control showing `value` (what applies: the player's value or the inherited one).
[[nodiscard]] TuningRow BuildTuningRow(const Render::Look::TuningControl& control, double value, TuningRowState state);
// The row's state from the session.
[[nodiscard]] TuningRowState TuningStateOf(const Render::Look::TuningSession& session,
                                           Render::Look::TuningControlId control);

// The label of each group's "More settings", reset and "Reset all tuning" actions.
[[nodiscard]] std::string TuningMoreSettingsLabel();
[[nodiscard]] std::string TuningResetLabel();
[[nodiscard]] std::string TuningResetGroupLabel();
[[nodiscard]] std::string TuningResetAllLabel();
// The visible failure of the last save, empty while what applies is saved or about to be.
[[nodiscard]] std::string TuningSaveMessage(const Render::Look::TuningSession& session);

} // namespace UI::Options
