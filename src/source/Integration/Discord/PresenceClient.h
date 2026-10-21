// Keeps the player's Discord presence in step with what the game publishes.
//
// Talking to Discord happens on a worker thread of its own: connecting,
// waiting for answers and reconnecting after Discord restarts must never
// cost the frame loop anything. The game only hands over the newest
// activity; the worker sends the latest one, at the pace Discord accepts.
#pragma once

#include "Integration/Discord/Activity.h"
#include "Integration/Discord/IpcConnection.h"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdint>
#include <mutex>
#include <optional>
#include <string>
#include <thread>

namespace Integration::Discord
{
class PresenceClient
{
public:
    // How often the worker looks for a Discord app that was not running.
    static constexpr std::chrono::seconds ReconnectDelay{15};

    // Discord takes five activity updates per 20 seconds; a faster stream
    // is throttled on its side. Changes in between are coalesced to the
    // newest one.
    static constexpr std::chrono::seconds MinPublishInterval{4};

    // How long Discord may take to accept the handshake.
    static constexpr std::chrono::seconds HandshakeTimeout{5};

    // How often an idle worker reads what Discord sent meanwhile.
    static constexpr std::chrono::seconds IdleWake{1};

    explicit PresenceClient(std::string applicationId);

    // Stops the worker and closes the connection, which makes Discord
    // remove the presence.
    ~PresenceClient();

    PresenceClient(const PresenceClient&) = delete;
    PresenceClient& operator=(const PresenceClient&) = delete;

    [[nodiscard]] const std::string& ApplicationId() const
    {
        return m_applicationId;
    }

    // Replaces what Discord should show; no activity clears it.
    void Publish(std::optional<Activity> activity);

private:
    void Run();

    [[nodiscard]] bool Connect();
    void ReportRejectedHandshake(const IpcConnection::Frame& frame);

    // Sends the newest published activity. False when the connection broke.
    [[nodiscard]] bool SendLatest();

    // Handles whatever Discord sent without waiting for more.
    void DrainIncoming();
    void HandleFrame(const IpcConnection::Frame& frame);

    [[nodiscard]] bool HasUnsentActivity();
    void WaitForStop(std::chrono::milliseconds duration);
    void WaitForChange(std::chrono::milliseconds duration);

    const std::string m_applicationId;

    // Worker thread only.
    IpcConnection m_connection;
    std::uint64_t m_sentVersion = 0;
    std::uint64_t m_nonce = 0;
    bool m_rejectionReported = false;

    // Shared with the game thread, guarded by m_mutex. Versions start at 1,
    // so a sent version of 0 always means "send again".
    std::mutex m_mutex;
    std::condition_variable m_wake;
    std::optional<Activity> m_desired;
    std::uint64_t m_desiredVersion = 0;
    std::atomic<bool> m_stopping{false};

    // Last, so everything the worker touches exists before it starts.
    std::thread m_worker;
};
} // namespace Integration::Discord
