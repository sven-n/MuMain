#pragma once

#include "Data/GameConfig/PersonalTuning.h"
#include "Render/Look/TuningControls.h"

#include <functional>
#include <optional>
#include <string>

// The player's tuning while it is being changed: what applies (the preview), what is saved, and
// how the two meet. A preview applies from the next frame on; the light mood's strength rebuilds
// the map's static light, so its changes (a reset's too) wait for ApplyPending(), which only the
// frame loop calls: however many interactions end within a frame, the map light is rebuilt once.
// Commit() saves at the end of an interaction (a released slider, a key, leaving the control,
// closing the window), never per frame, only what changed, and never applies anything itself. A
// failed save keeps the preview and the file as they were and says so (SaveState::Failed) until a
// later save succeeds.
namespace Render::Look
{

enum class TuningSaveState : std::uint8_t
{
    // What applies is what the file holds.
    Saved,
    // A preview not yet saved.
    Unsaved,
    // The last save failed; the preview applies, the file holds what it held before.
    Failed,
};

class TuningSession
{
public:
    struct Hooks
    {
        // Writes the change from what was saved to the tuning ([EnhancedTuning] of config.ini).
        std::function<Data::Config::PersonalTuningSaveResult(const Data::Config::PersonalTuning& saved,
                                                             const Data::Config::PersonalTuning& tuning)>
            persist;
        // The player's look values changed (the renderer reads them from the next frame on).
        std::function<void(const Data::Maps::LookOverrides&)> applyLook;
        // The player's light mood strength changed (rebuilds the map's static light).
        std::function<void(std::optional<int>)> applyMoodStrength;
    };

    // `persisted`: the tuning the file holds, already applied by the caller.
    TuningSession(Hooks hooks, Data::Config::PersonalTuning persisted);

    // The player's value of a control; nothing while it follows the look profile and the map.
    [[nodiscard]] std::optional<double> Override(TuningControlId id) const;
    // True when the control's preview differs from what the file holds.
    [[nodiscard]] bool IsUnsaved(TuningControlId id) const;

    // Previews a value; false (nothing changes) for a value outside the control's range. The value
    // is kept as config.ini would hold it, so a restart applies exactly the same.
    bool Preview(TuningControlId id, double value);
    // Back to the look profile and the map: one control, a group or every control. Saved at once;
    // a reset light mood strength waits for ApplyPending() like a preview.
    void Reset(TuningControlId id);
    void ResetGroup(TuningGroup group);
    void ResetAll();

    // Hands a changed light mood strength on. Called once per rendered frame (ApplyPendingTuning()).
    void ApplyPending();
    // Saves the preview when it differs from the file; false when that failed.
    bool Commit();

    [[nodiscard]] const Data::Config::PersonalTuning& Current() const
    {
        return m_current;
    }
    [[nodiscard]] const Data::Config::PersonalTuning& Persisted() const
    {
        return m_persisted;
    }
    [[nodiscard]] TuningSaveState SaveState() const;
    // Why the last save failed; empty unless SaveState() is Failed.
    [[nodiscard]] const std::string& SaveError() const
    {
        return m_saveError;
    }

private:
    void Remove(TuningControlId id);
    void Changed(const TuningControl& control);

    Hooks m_hooks;
    Data::Config::PersonalTuning m_persisted;
    Data::Config::PersonalTuning m_current;
    bool m_moodPending = false;
    std::string m_saveError;
};

} // namespace Render::Look
