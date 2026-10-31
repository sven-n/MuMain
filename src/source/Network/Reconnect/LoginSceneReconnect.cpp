#include "stdafx.h"
#include "Network/Reconnect/LoginSceneReconnect.h"

#include <utility>

#include "Core/Utilities/Log/MuLogger.h"

namespace
{
// How long a reconnected server may take for its first answer (the connect
// server's list, the game server's join answer) before the attempt counts as failed.
constexpr double GreetingTimeoutMs = 10000.0;

const char* NameOf(LoginServer server)
{
    return server == LoginServer::Game ? "game server" : "connect server";
}
} // namespace

LoginSceneReconnect::LoginSceneReconnect(LoginSceneReconnectLink& link) : m_link(link) {}

bool LoginSceneReconnect::EnsureConnected(LoginServer server, std::function<void()> resume)
{
    if (m_phase == Phase::Failed)
    {
        // The failure message doesn't hold the screen: the player went on past it
        // (the server list under it still takes clicks), so their step starts over.
        m_link.HideFailure();
        m_resume = nullptr;
        EnterPhase(Phase::Idle);
    }

    switch (m_phase)
    {
    case Phase::Idle:
        if (m_link.IsConnected())
        {
            return true;
        }
        Begin(server, std::move(resume));
        return false;
    case Phase::Probing:
    case Phase::AwaitingGreeting:
        // Already on the way back; the player's latest step is the one to carry out.
        if (server == m_server)
        {
            m_resume = std::move(resume);
        }
        return false;
    case Phase::Failed:
        break;
    }
    return false;
}

void LoginSceneReconnect::RequestServerList()
{
    if (EnsureConnected(LoginServer::Connect, nullptr))
    {
        m_link.SendServerListRequest();
    }
}

void LoginSceneReconnect::OnConnectionLost()
{
    switch (m_phase)
    {
    case Phase::Probing:
    case Phase::Failed:
        return;
    case Phase::AwaitingGreeting:
        Fail(); // the new connection closed before the server answered
        return;
    case Phase::Idle:
        break;
    }

    if (!m_lossLogged)
    {
        m_lossLogged = true;
        mu::log::Get("network")->info("NET: the login screen lost its connection; it reconnects when needed");
    }

    const std::optional<LoginServer> awaited = m_link.AwaitedServer();
    if (awaited.has_value())
    {
        Begin(*awaited, nullptr);
    }
}

void LoginSceneReconnect::OnGreeting(LoginServer server)
{
    if (m_phase != Phase::AwaitingGreeting)
    {
        return;
    }

    mu::log::Get("network")->info("NET: reconnected to the {}", NameOf(server));
    EnterPhase(Phase::Idle);

    // A server can greet other than expected (a client started straight on a game
    // server gets the login window back instead of a list); the step waiting for
    // the other server then has nothing to resume.
    std::function<void()> resume = std::exchange(m_resume, nullptr);
    if (resume && server == m_server)
    {
        resume();
    }
}

void LoginSceneReconnect::Update()
{
    switch (m_phase)
    {
    case Phase::Idle:
        if (m_lossLogged && m_link.IsConnected())
        {
            m_lossLogged = false; // someone else reconnected (e.g. the login window's Cancel)
        }
        return;
    case Phase::Probing:
        UpdateProbing();
        return;
    case Phase::AwaitingGreeting:
        if (m_link.NowMs() - m_phaseStartMs > GreetingTimeoutMs)
        {
            Fail();
        }
        return;
    case Phase::Failed:
        // Another message took the failure message's place, or the screen was built
        // again: nothing answers it any more. The next step, or a screen left waiting,
        // starts over as after a plain drop.
        if (!m_link.IsFailureShown())
        {
            m_resume = nullptr;
            EnterPhase(Phase::Idle);
        }
        return;
    }
}

void LoginSceneReconnect::Retry()
{
    if (m_phase != Phase::Failed)
    {
        return;
    }

    // Start over from the server list, which shows the servers that are up. A
    // server pick waiting there is kept; a login waits for a game server that
    // didn't answer, so the player logs in again after picking a server.
    std::function<void()> resume = std::exchange(m_resume, nullptr);
    if (m_server != LoginServer::Connect)
    {
        resume = nullptr;
    }
    Begin(LoginServer::Connect, std::move(resume));
}

void LoginSceneReconnect::Reset()
{
    if (m_phase == Phase::Idle && !m_resume && !m_lossLogged)
    {
        return;
    }

    m_link.StopProbe();
    m_resume = nullptr;
    m_lossLogged = false;
    EnterPhase(Phase::Idle);
}

bool LoginSceneReconnect::IsReconnecting() const
{
    return m_phase == Phase::Probing || m_phase == Phase::AwaitingGreeting;
}

void LoginSceneReconnect::Begin(LoginServer server, std::function<void()> resume)
{
    m_server = server;
    m_resume = std::move(resume);
    mu::log::Get("network")->info("NET: reconnecting the login screen to the {}", NameOf(server));

    if (!m_link.StartProbe(server))
    {
        Fail();
        return;
    }

    EnterPhase(Phase::Probing);
}

void LoginSceneReconnect::UpdateProbing()
{
    switch (m_link.PollProbe())
    {
    case LoginSceneReconnectLink::ProbeResult::Pending:
        return;
    case LoginSceneReconnectLink::ProbeResult::Unreachable:
        Fail();
        return;
    case LoginSceneReconnectLink::ProbeResult::Reachable:
        break;
    }

    if (!m_link.Connect(m_server))
    {
        Fail();
        return;
    }

    EnterPhase(Phase::AwaitingGreeting);
}

void LoginSceneReconnect::Fail()
{
    m_link.StopProbe();
    EnterPhase(Phase::Failed);
    mu::log::Get("network")->warn("NET: the {} can't be reached from the login screen", NameOf(m_server));
    m_link.ShowFailure();
}

void LoginSceneReconnect::EnterPhase(Phase phase)
{
    m_phase = phase;
    m_phaseStartMs = m_link.NowMs();
}
