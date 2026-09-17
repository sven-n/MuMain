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

    m_connections.clear();
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

        ++m_nextConnectionId;
        m_connections.push_back({m_nextConnectionId, std::move(socket)});
    }
}

void ControlServer::ServeRequests()
{
    std::size_t served = 0;

    for (Connection& connection : m_connections)
    {
        if (!connection.socket->IsOpen())
        {
            continue;
        }

        connection.socket->ReadAvailable();

        std::string line;
        while (served < MaxRequestsPerFrame && connection.socket->TakeLine(line))
        {
            const Request request = Request::Parse(line);
            if (request.IsEmpty())
            {
                continue;
            }

            ++served;
            m_dispatcher.Handle(request, connection.id);
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
