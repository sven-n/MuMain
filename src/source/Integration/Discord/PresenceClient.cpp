#include "Integration/Discord/PresenceClient.h"

#include "Integration/Discord/RpcCommands.h"
#include "Core/Utilities/Log/MuLogger.h"

#include <utility>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace
{
constexpr const char* LoggerName = "discord";

// Discord ties the activity to the process, so it disappears when the game
// exits even if the pipe outlives it.
std::uint32_t CurrentProcessId()
{
#ifdef _WIN32
    return static_cast<std::uint32_t>(GetCurrentProcessId());
#else
    return static_cast<std::uint32_t>(::getpid());
#endif
}
} // namespace

namespace Integration::Discord
{
PresenceClient::PresenceClient(std::string applicationId)
    : m_applicationId(std::move(applicationId)), m_worker([this] { Run(); })
{
}

PresenceClient::~PresenceClient()
{
    {
        std::lock_guard lock(m_mutex);
        m_stopping = true;
    }
    m_wake.notify_all();
    if (m_worker.joinable())
    {
        m_worker.join();
    }
    m_connection.Close();
}

void PresenceClient::Publish(std::optional<Activity> activity)
{
    {
        std::lock_guard lock(m_mutex);
        m_desired = std::move(activity);
        ++m_desiredVersion;
    }
    m_wake.notify_all();
}

void PresenceClient::Run()
{
    while (!m_stopping)
    {
        if (!m_connection.IsOpen())
        {
            if (!Connect())
            {
                WaitForStop(ReconnectDelay);
                continue;
            }
            m_sentVersion = 0;
        }

        if (HasUnsentActivity())
        {
            if (SendLatest())
            {
                WaitForStop(MinPublishInterval);
            }
            continue;
        }

        WaitForChange(IdleWake);
        DrainIncoming();
    }
}

bool PresenceClient::Connect()
{
    if (!m_connection.Open())
    {
        return false;
    }

    IpcConnection::Frame answer;
    const bool sent = m_connection.Send(Ipc::Opcode::Handshake, Rpc::Handshake(m_applicationId));
    const bool answered =
        sent && m_connection.Read(answer, HandshakeTimeout, m_stopping) == IpcConnection::ReadStatus::Frame;
    if (!answered || answer.opcode != Ipc::Opcode::Frame || !Rpc::IsReadyEvent(answer.payload))
    {
        if (answered)
        {
            ReportRejectedHandshake(answer);
        }
        m_connection.Close();
        return false;
    }

    mu::log::Get(LoggerName)->info("Connected to Discord, showing the rich presence.");
    m_rejectionReported = false;
    return true;
}

// A wrong application id is refused on every attempt; saying so once per
// configuration is enough.
void PresenceClient::ReportRejectedHandshake(const IpcConnection::Frame& frame)
{
    if (m_rejectionReported)
    {
        return;
    }
    m_rejectionReported = true;
    mu::log::Get(LoggerName)
        ->warn("Discord refused the rich presence handshake (check [Discord] ApplicationId): {}", frame.payload);
}

bool PresenceClient::SendLatest()
{
    std::optional<Activity> activity;
    std::uint64_t version = 0;
    {
        std::lock_guard lock(m_mutex);
        activity = m_desired;
        version = m_desiredVersion;
    }

    if (!m_connection.Send(Ipc::Opcode::Frame, Rpc::SetActivity(activity, CurrentProcessId(), ++m_nonce)))
    {
        return false;
    }
    m_sentVersion = version;
    return true;
}

void PresenceClient::DrainIncoming()
{
    constexpr std::chrono::milliseconds NoWait{0};
    IpcConnection::Frame frame;
    while (m_connection.Read(frame, NoWait, m_stopping) == IpcConnection::ReadStatus::Frame)
    {
        HandleFrame(frame);
    }
}

void PresenceClient::HandleFrame(const IpcConnection::Frame& frame)
{
    switch (frame.opcode)
    {
    case Ipc::Opcode::Ping:
        m_connection.Send(Ipc::Opcode::Pong, frame.payload);
        return;
    case Ipc::Opcode::Close:
        mu::log::Get(LoggerName)->info("Discord closed the rich presence connection: {}", frame.payload);
        m_connection.Close();
        return;
    default:
        break;
    }

    const std::string error = Rpc::ErrorMessage(frame.payload);
    if (!error.empty())
    {
        mu::log::Get(LoggerName)->warn("Discord rejected the rich presence update: {}", error);
    }
}

bool PresenceClient::HasUnsentActivity()
{
    std::lock_guard lock(m_mutex);
    return m_desiredVersion != m_sentVersion;
}

void PresenceClient::WaitForStop(std::chrono::milliseconds duration)
{
    std::unique_lock lock(m_mutex);
    m_wake.wait_for(lock, duration, [this] { return m_stopping.load(); });
}

void PresenceClient::WaitForChange(std::chrono::milliseconds duration)
{
    std::unique_lock lock(m_mutex);
    m_wake.wait_for(lock, duration, [this] { return m_stopping.load() || m_desiredVersion != m_sentVersion; });
}
} // namespace Integration::Discord
