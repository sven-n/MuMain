#include "stdafx.h"
#include "Network/Reconnect/LoginSceneReconnect.h"

#include <utility>

#include "Core/Utilities/Log/MuLogger.h"

namespace
{
// How long a reconnected server may take for its first answer (the connect
// server's list, the game server's join answer) before the attempt counts as failed.
constexpr double GreetingTimeoutMs = 10000.0;

// How often an open server list asks for a fresh one. Well within OpenMU's connect
// server Timeout (one minute by default), so the list's connection stays up and its
// load figures current.
constexpr double ServerListRefreshMs = 30000.0;

// How many list requests the connect server answers over one connection: OpenMU
// closes the connection on the next one (MaxServerListRequests, 20 by default).
constexpr int ListRequestsPerConnection = 20;

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

    if (IsReconnecting())
    {
        // Already on the way back; the player's latest step is the one to carry out.
        if (server == m_server)
        {
            m_resume = std::move(resume);
            PlayerWaits();
        }
        return false;
    }

    if (m_link.IsConnected() && m_link.ConnectedServer() == server)
    {
        return true;
    }

    Begin(server, std::move(resume));
    return false;
}

void LoginSceneReconnect::RequestServerList()
{
    if (IsReconnecting())
    {
        // The reconnect brings a list (or the login window) itself. A server pick
        // waiting for it stays: opening the credits isn't a step that replaces it.
        if (m_server == LoginServer::Connect)
        {
            PlayerWaits();
        }
        return;
    }

    if (m_phase == Phase::Idle && m_link.ConnectedServer() == LoginServer::Game)
    {
        return;
    }

    if (!EnsureConnected(LoginServer::Connect, nullptr))
    {
        return;
    }

    if (TakeListRequest())
    {
        m_lastListMs = m_link.NowMs();
        m_link.SendServerListRequest();
        return;
    }

    Begin(LoginServer::Connect, nullptr); // the new connection's greeting is a list
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

void LoginSceneReconnect::OnGreeting(LoginServer server, bool carryOn)
{
    if (server == LoginServer::Connect)
    {
        m_lastListMs = m_link.NowMs();
    }

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
    if (resume && carryOn && server == m_server)
    {
        resume();
    }
}

void LoginSceneReconnect::ReturnToServerList()
{
    Begin(LoginServer::Connect, nullptr);
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
        RefreshServerList();
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

void LoginSceneReconnect::Begin(LoginServer server, std::function<void()> resume, bool quiet)
{
    m_server = server;
    m_resume = std::move(resume);
    m_quiet = quiet;
    mu::log::Get("network")->info("NET: reconnecting the login screen to the {}", NameOf(server));
    if (!m_quiet)
    {
        m_link.ShowConnecting();
    }

    if (!m_link.StartProbe(server))
    {
        Fail();
        return;
    }

    EnterPhase(Phase::Probing);
}

void LoginSceneReconnect::PlayerWaits()
{
    // A renewal went on quietly so far; the player now waits for it.
    if (!m_quiet)
    {
        return;
    }

    m_quiet = false;
    m_link.ShowConnecting();
}

void LoginSceneReconnect::RefreshServerList()
{
    if (!m_link.IsServerListShown() || !m_link.IsConnected())
    {
        return;
    }

    const double now = m_link.NowMs();
    if (now - m_lastListMs < ServerListRefreshMs)
    {
        return;
    }

    m_lastListMs = now;
    if (TakeListRequest())
    {
        m_link.SendServerListRequest();
        return;
    }

    // The next request would end the connection; a new one comes with a list.
    Begin(LoginServer::Connect, nullptr, /*quiet=*/true);
}

bool LoginSceneReconnect::TakeListRequest()
{
    const int connection = m_link.ConnectionId();
    if (connection != m_listConnection)
    {
        // The client asked for the first list as soon as the connect server greeted it.
        m_listConnection = connection;
        m_listRequests = 1;
    }

    if (m_listRequests >= ListRequestsPerConnection)
    {
        return false;
    }

    ++m_listRequests;
    return true;
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
    if (m_quiet)
    {
        // Nobody waited for it: as after any drop, the next step reconnects.
        mu::log::Get("network")->info("NET: renewing the login screen's {} connection failed", NameOf(m_server));
        m_resume = nullptr;
        EnterPhase(Phase::Idle);
        return;
    }

    EnterPhase(Phase::Failed);
    mu::log::Get("network")->warn("NET: the {} can't be reached from the login screen", NameOf(m_server));
    m_link.ShowFailure();
}

void LoginSceneReconnect::EnterPhase(Phase phase)
{
    m_phase = phase;
    m_phaseStartMs = m_link.NowMs();
}
