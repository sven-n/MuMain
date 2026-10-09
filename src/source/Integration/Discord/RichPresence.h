// The game's side of the Discord rich presence: once per refresh it reads
// the player's state, builds the activity and hands it to the worker that
// talks to Discord.
//
// Builds without ENABLE_DISCORD (and the mobile platforms) get the no-op
// below instead, so the frame loop and the options window call it without
// conditionals.
#pragma once

#if MU_ENABLE_DISCORD

#include "Integration/Discord/Activity.h"
#include "Integration/Discord/PresenceClient.h"
#include "Integration/Discord/PresenceSnapshot.h"

#include <chrono>
#include <cstdint>
#include <memory>
#include <optional>

namespace Integration::Discord
{
class RichPresence
{
public:
    static constexpr bool IsAvailable = true;

    // Game state changes (map, level, party) reach Discord within this time.
    // Reading the state is cheap, but not something to do every frame.
    static constexpr std::chrono::seconds RefreshInterval{1};

    [[nodiscard]] static RichPresence& Instance();

    // Called once per frame from the main loop.
    void Update();

    // Re-reads the configuration right away, after the options window
    // changed it, instead of waiting for the next refresh.
    void Reconfigure();

    void Stop();

private:
    void Refresh();

    // Starts, replaces or stops the worker to match config.ini. Returns
    // whether a presence should be shown.
    [[nodiscard]] bool MatchConfiguration();

    [[nodiscard]] std::int64_t SceneStartTimestamp(PresenceSnapshot::Scene scene);

    std::unique_ptr<PresenceClient> m_client;
    std::optional<Activity> m_published;
    std::chrono::steady_clock::time_point m_nextRefresh{};

    std::optional<PresenceSnapshot::Scene> m_scene;
    std::int64_t m_sceneStartedAt = 0;
};
} // namespace Integration::Discord

#else

namespace Integration::Discord
{
// Built without ENABLE_DISCORD: the same entry points, doing nothing.
class RichPresence
{
public:
    static constexpr bool IsAvailable = false;

    [[nodiscard]] static RichPresence& Instance()
    {
        static RichPresence instance;
        return instance;
    }

    void Update() {}
    void Reconfigure() {}
    void Stop() {}
};
} // namespace Integration::Discord

#endif // MU_ENABLE_DISCORD
