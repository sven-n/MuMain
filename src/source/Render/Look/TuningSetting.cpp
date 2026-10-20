#include "stdafx.h"

#include "Render/Look/TuningSetting.h"

#include "Core/Utilities/Log/MuLogger.h"
#include "Data/GameConfig/GameConfig.h"
#include "Render/Look/LookPresetSetting.h"
#include "Render/Look/LookProfileSetting.h"

#include <memory>

namespace Render::Look
{
namespace
{
Data::Config::PersonalTuningSaveResult Persist(const Data::Config::PersonalTuning& saved,
                                               const Data::Config::PersonalTuning& tuning)
{
    Data::Config::PersonalTuningSaveResult result =
        Data::Config::SavePersonalTuningFile(GameConfig::GetInstance().GetConfigPath(), saved, tuning);
    if (!result.saved)
    {
        mu::log::Get("render")->error("Enhanced tuning not saved: {}", result.error);
    }
    return result;
}

TuningSession::Hooks SessionHooks()
{
    return {Persist, SetPersonalLookOverrides, ChooseLightMoodStrength};
}

std::unique_ptr<TuningSession>& Session()
{
    static std::unique_ptr<TuningSession> session;
    return session;
}
} // namespace

void LoadPersonalTuning()
{
    std::vector<std::string> issues;
    Data::Config::PersonalTuning tuning =
        Data::Config::LoadPersonalTuningFile(GameConfig::GetInstance().GetConfigPath(), issues);
    for (const std::string& issue : issues)
    {
        mu::log::Get("render")->warn("{}", issue);
    }
    SetPersonalLookOverrides(tuning.look);
    ChooseLightMoodStrength(tuning.moodStrengthPercent);
    Session() = std::make_unique<TuningSession>(SessionHooks(), tuning);
}

TuningSession& PersonalTuningSession()
{
    std::unique_ptr<TuningSession>& session = Session();
    if (!session)
    {
        // Not loaded (a tool or test without start-up): nothing is tuned yet. A save writes only
        // what this session changes, so it cannot drop what the file holds.
        session = std::make_unique<TuningSession>(SessionHooks(), Data::Config::PersonalTuning{});
    }
    return *session;
}

void ApplyPendingTuning()
{
    if (Session())
    {
        Session()->ApplyPending();
    }
}

} // namespace Render::Look
