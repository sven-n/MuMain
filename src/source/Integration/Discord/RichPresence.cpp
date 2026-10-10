#include "stdafx.h"
#include "Integration/Discord/RichPresence.h"

#include "Core/Text/Utf8.h"
#include "Data/GameConfig/GameConfig.h"
#include "GameLogic/Discord/ServerIntegration.h"
#include "Integration/Discord/GameStateReader.h"
#include "Integration/Discord/PresenceMode.h"
#include "Integration/Discord/PresenceText.h"

namespace
{
std::int64_t UnixSecondsNow()
{
    const auto sinceEpoch = std::chrono::system_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::seconds>(sinceEpoch).count();
}
} // namespace

namespace Integration::Discord
{
RichPresence& RichPresence::Instance()
{
    static RichPresence instance;
    return instance;
}

void RichPresence::Update()
{
    const auto now = std::chrono::steady_clock::now();
    if (now < m_nextRefresh)
    {
        return;
    }
    m_nextRefresh = now + RefreshInterval;
    Refresh();
}

void RichPresence::Reconfigure()
{
    m_nextRefresh = {};
    Update();
}

void RichPresence::Stop()
{
    m_client.reset();
    m_published.reset();
}

void RichPresence::Refresh()
{
    if (!MatchConfiguration())
    {
        return;
    }

    const std::optional<PresenceSnapshot> snapshot = GameState::Read();
    if (!snapshot.has_value())
    {
        return;
    }

    const auto& server = GameLogic::Discord::ServerIntegration::Instance();
    const PresenceImages images{Core::Text::ToUtf8(server.RichPresenceLargeImageKey().c_str()),
                                Core::Text::ToUtf8(server.RichPresenceSmallImageKey().c_str())};
    const PresenceMode mode = ParsePresenceMode(GameConfig::GetInstance().GetDiscordPresence());
    Activity activity = DescribePresence(*snapshot, mode, images);
    activity.startTimestamp = SceneStartTimestamp(snapshot->scene);
    if (m_published == activity)
    {
        return;
    }

    m_client->Publish(activity);
    m_published = std::move(activity);
}

bool RichPresence::MatchConfiguration()
{
    // The server's application wins over the one in config.ini.
    const auto& server = GameLogic::Discord::ServerIntegration::Instance();
    const std::string applicationId = Core::Text::ToUtf8(server.RichPresenceApplicationId().c_str());
    const bool enabled = ParsePresenceMode(GameConfig::GetInstance().GetDiscordPresence()) != PresenceMode::Off;
    if (!enabled || applicationId.empty())
    {
        Stop();
        return false;
    }

    if (m_client == nullptr || m_client->ApplicationId() != applicationId)
    {
        m_client = std::make_unique<PresenceClient>(applicationId);
        m_published.reset();
    }
    return true;
}

// Discord's "elapsed" timer counts the time spent in the current scene: in
// the world since entering it, not since the last map change.
std::int64_t RichPresence::SceneStartTimestamp(PresenceSnapshot::Scene scene)
{
    if (m_scene != scene)
    {
        m_scene = scene;
        m_sceneStartedAt = UnixSecondsNow();
    }
    return m_sceneStartedAt;
}
} // namespace Integration::Discord
