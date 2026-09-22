#include "stdafx.h"
#include "App/Control/ControlServer.h"

#include "App/Control/ControlCommands.h"
#include "App/Control/ControlEvents.h"
#include "App/Control/ControlObjects.h"
#include "App/Control/ControlTaps.h"
#include "Core/Utilities/Log/MuLogger.h"
#include "World/MapInfra/MapManager.h"

#include <cstdlib>
#include <utility>

namespace
{
constexpr const char* LogChannel = "control";
}

// Non-zero while the client is still loading a world (ZzzInterface.cpp).
extern int LoadingWorld;

namespace App::Control
{
ControlServer& ControlServer::Instance()
{
    static ControlServer server;
    return server;
}

bool ControlServer::Start(const std::string& buildIdentifier)
{
    const char* path = std::getenv(SocketPathVariable);
    if (path == nullptr || path[0] == '\0')
    {
        // Not asked for: no socket, no log line, no recording.
        return false;
    }

    std::string error;
    if (!m_listener.Listen(path, error))
    {
        mu::log::Get(LogChannel)->error("control socket not available: {}", error);
        return false;
    }

    Commands::SetBuildIdentifier(buildIdentifier);
    Events::SetObjectResolver(&DescribeGameObject);
    Events::SetEnabled(true);

    mu::log::Get(LogChannel)->info("control socket listening on {}", m_listener.Path());
    return true;
}

void ControlServer::Stop()
{
    if (!IsRunning())
    {
        return;
    }

    // The dispatcher keeps per-connection state (a follower, the act a
    // connection owns); dropping the sockets alone would leave it queuing
    // answers for ids that no longer exist.
    for (const Connection& connection : m_connections)
    {
        m_dispatcher.AbandonConnection(connection.id);
    }
    m_connections.clear();
    // The next session starts from nothing known: without this, re-entering
    // the scene and map the last one ended on emits no `scene` or `map`
    // event and a follower never learns where the character is.
    m_lastScene.clear();
    m_lastMap = -1;
    m_listener.Close();
    Events::SetEnabled(false);
    Events::SetObjectResolver(nullptr);
}

void ControlServer::Poll()
{
    if (!IsRunning())
    {
        return;
    }

    RecordSceneTransition();
    AcceptNewConnections();
    ServeRequests();
    m_dispatcher.Tick();
    DeliverResponses();
    CloseFinishedPeers();
    DropClosedConnections();
}

void ControlServer::RecordSceneTransition()
{
    const std::string_view scene = Commands::CurrentSceneName();
    if (scene != m_lastScene)
    {
        m_lastScene = scene;
        Events::RecordScene(scene);
    }

    if (scene != "world")
    {
        // Leaving the world forgets the map, so entering the same one again
        // is a change: without this, a `map` event is missing after every
        // return to the map the character logged out on.
        m_lastMap = -1;
    }

    // The map is read here rather than tapped in the packet handler: the
    // world is only loaded some frames after the map-change packet, so the
    // packet itself still names the map being left, and the map index is
    // not settled until the loading handshake is over.
    const int map = gMapManager.WorldActive;
    if (map >= 0 && map != m_lastMap && scene == "world" && LoadingWorld == 0)
    {
        m_lastMap = map;
        Events::RecordMapChange();
    }
}

void ControlServer::AcceptNewConnections()
{
    for (std::size_t accepted = 0; accepted < MaxAcceptsPerFrame; ++accepted)
    {
        auto socket = m_listener.Accept();
        if (!socket)
        {
            return;
        }

        if (m_connections.size() >= MaxConnections)
        {
            // Refused rather than queued: the caller is told now, and the
            // list this server walks twice a frame stays bounded.
            mu::log::Get(LogChannel)
                ->warn("control socket refused a connection: {} are already open", m_connections.size());
            socket->Close();
            return;
        }

        ++m_nextConnectionId;
        m_connections.push_back({m_nextConnectionId, std::move(socket)});
    }
}

void ControlServer::ServeRequests()
{
    for (Connection& connection : m_connections)
    {
        if (!connection.socket->IsOpen())
        {
            continue;
        }

        if (!connection.socket->ReadAvailable())
        {
            // Closed under us: the peer hung up, the recv failed, or the
            // input caps dropped it for flooding. Whatever is still in the
            // inbox has nobody left to answer to, and running it would
            // move the character for a connection that is already gone.
            continue;
        }

        // The budget is per connection: a chatty driver must not starve a
        // second connection sitting on `events --follow`, whose unread bytes
        // would otherwise pile up until the input cap closes it.
        std::size_t served = 0;
        std::string line;
        while (served < MaxRequestsPerFrame && connection.socket->TakeLine(line))
        {
            // Counted before the line is judged: a peer writing nothing but
            // blank lines would otherwise drain its whole inbox in one
            // frame, which is the stall this budget exists to prevent.
            ++served;
            const Request request = Request::Parse(line);
            if (request.IsEmpty())
            {
                continue;
            }

            m_dispatcher.Handle(request, connection.id);
        }
    }
}

void ControlServer::CloseFinishedPeers()
{
    for (Connection& connection : m_connections)
    {
        // A peer that shut its end down is answered and then let go: it can
        // send nothing more, and holding the slot open would keep one of the
        // sixteen connections for a script that has already left. An act it
        // started still owes it an answer, and a one-shot `move` is as much
        // a question as a `ping`.
        if (connection.socket->IsOpen() && connection.socket->PeerClosed() && !connection.socket->HasLine() &&
            !connection.socket->HasPendingOutput() && !m_dispatcher.OwesAnswer(connection.id))
        {
            connection.socket->Close();
        }
    }
}

void ControlServer::DeliverResponses()
{
    for (const OutgoingResponse& response : m_dispatcher.TakeResponses())
    {
        for (Connection& connection : m_connections)
        {
            if (connection.id == response.connection && connection.socket->IsOpen())
            {
                connection.socket->Write(response.line);
                break;
            }
        }
    }

    for (Connection& connection : m_connections)
    {
        if (connection.socket->IsOpen() && connection.socket->HasPendingOutput())
        {
            connection.socket->Flush();
        }
    }
}

void ControlServer::DropClosedConnections()
{
    for (auto connection = m_connections.begin(); connection != m_connections.end();)
    {
        if (connection->socket->IsOpen())
        {
            ++connection;
            continue;
        }

        m_dispatcher.AbandonConnection(connection->id);
        connection = m_connections.erase(connection);
    }
}
} // namespace App::Control
