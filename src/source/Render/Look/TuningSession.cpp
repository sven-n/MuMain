#include "stdafx.h"

#include "Render/Look/TuningSession.h"

#include <cmath>
#include <utility>

namespace Render::Look
{
namespace
{
std::optional<double> Get(const Data::Config::PersonalTuning& tuning, const TuningControl& control)
{
    if (control.lookValue)
    {
        return tuning.look[*control.lookValue];
    }
    if (tuning.moodStrengthPercent)
    {
        return static_cast<double>(*tuning.moodStrengthPercent);
    }
    return std::nullopt;
}
} // namespace

TuningSession::TuningSession(Hooks hooks, Data::Config::PersonalTuning persisted)
    : m_hooks(std::move(hooks)), m_persisted(persisted), m_current(persisted)
{
}

std::optional<double> TuningSession::Override(TuningControlId id) const
{
    return Get(m_current, Control(id));
}

bool TuningSession::IsUnsaved(TuningControlId id) const
{
    const TuningControl& control = Control(id);
    return Get(m_current, control) != Get(m_persisted, control);
}

bool TuningSession::Preview(TuningControlId id, double value)
{
    const TuningControl& control = Control(id);
    const std::optional<double> valid = ValidTuningValue(control, value);
    if (!valid)
    {
        return false;
    }
    if (control.lookValue)
    {
        const double kept = Data::Config::NormalizeTuningNumber(*valid);
        if (m_current.look[*control.lookValue] == kept)
        {
            return true;
        }
        m_current.look[*control.lookValue] = kept;
    }
    else
    {
        const int percent = static_cast<int>(std::lround(*valid));
        if (m_current.moodStrengthPercent == percent)
        {
            return true;
        }
        m_current.moodStrengthPercent = percent;
    }
    Changed(control);
    return true;
}

void TuningSession::Remove(TuningControlId id)
{
    const TuningControl& control = Control(id);
    if (!Get(m_current, control))
    {
        return;
    }
    if (control.lookValue)
    {
        m_current.look[*control.lookValue].reset();
    }
    else
    {
        m_current.moodStrengthPercent.reset();
    }
    Changed(control);
}

void TuningSession::Reset(TuningControlId id)
{
    Remove(id);
    (void)Commit();
}

void TuningSession::ResetGroup(TuningGroup group)
{
    for (const TuningControl& control : TuningControls())
    {
        if (control.group == group)
        {
            Remove(control.id);
        }
    }
    (void)Commit();
}

void TuningSession::ResetAll()
{
    for (const TuningControl& control : TuningControls())
    {
        Remove(control.id);
    }
    (void)Commit();
}

void TuningSession::Changed(const TuningControl& control)
{
    if (!control.lookValue)
    {
        m_moodPending = true;
        return;
    }
    if (m_hooks.applyLook)
    {
        m_hooks.applyLook(m_current.look);
    }
}

void TuningSession::ApplyPending()
{
    if (!m_moodPending)
    {
        return;
    }
    m_moodPending = false;
    if (m_hooks.applyMoodStrength)
    {
        m_hooks.applyMoodStrength(m_current.moodStrengthPercent);
    }
}

bool TuningSession::Commit()
{
    if (m_current == m_persisted)
    {
        m_saveError.clear();
        return true;
    }
    const Data::Config::PersonalTuningSaveResult result =
        m_hooks.persist ? m_hooks.persist(m_persisted, m_current)
                        : Data::Config::PersonalTuningSaveResult{false, "no configuration file"};
    if (!result.saved)
    {
        m_saveError = result.error.empty() ? std::string("the configuration could not be saved") : result.error;
        return false;
    }
    m_persisted = m_current;
    m_saveError.clear();
    return true;
}

TuningSaveState TuningSession::SaveState() const
{
    if (!m_saveError.empty())
    {
        return TuningSaveState::Failed;
    }
    return m_current == m_persisted ? TuningSaveState::Saved : TuningSaveState::Unsaved;
}

} // namespace Render::Look
