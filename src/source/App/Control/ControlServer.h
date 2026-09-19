// Lifecycle of the control socket: opened only when the launcher asks for it,
// served from the main loop, closed and unlinked on shutdown.
//
// Off by default. Without the environment variable nothing is created, nothing
// is logged, and the client behaves exactly as it did before this existed.
#pragma once

#if MU_ENABLE_CONTROL_SOCKET

#include "App/Control/ControlDispatcher.h"
#include "Core/Platform/LocalSocket.h"

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

namespace App::Control
{
class ControlServer
{
public:
    // Environment variable holding the socket path. An option would not do:
    // the client's command line parser splits on spaces, so a path with a
    // space in it could not be passed (design.md, D1).
    static constexpr const char* SocketPathVariable = "MU_CONTROL_SOCKET";

    // Connections accepted per frame, and requests served per frame *per
    // connection*. Bounded so a busy caller cannot stall rendering; counted
    // per connection so a busy one cannot starve the others either.
    static constexpr std::size_t MaxAcceptsPerFrame = 4;
    static constexpr std::size_t MaxRequestsPerFrame = 16;

    [[nodiscard]] static ControlServer& Instance();

    // Starts listening when the environment variable is set. Returns false
    // when it is not — that is the ordinary case, and it logs nothing.
    bool Start(const std::string& buildIdentifier);

    // Accepts, reads, dispatches and flushes. Called once per frame from
    // the main loop, after the packet drain.
    void Poll();

    void Stop();

    [[nodiscard]] bool IsRunning() const
    {
        return m_listener.IsListening();
    }
    [[nodiscard]] const std::string& SocketPath() const
    {
        return m_listener.Path();
    }

private:
    struct Connection
    {
        std::size_t id = 0;
        std::unique_ptr<Core::Platform::LocalSocketConnection> socket;
    };

    // Emits a `scene` event whenever the client changes screen, so login,
    // character list and world transitions are in the stream without the
    // scenes knowing about the control socket.
    void RecordSceneTransition();

    void AcceptNewConnections();
    void ServeRequests();
    void DeliverResponses();
    void DropClosedConnections();

    Core::Platform::LocalSocketListener m_listener;
    std::vector<Connection> m_connections;
    Dispatcher m_dispatcher;
    std::size_t m_nextConnectionId = 0;
    std::string m_lastScene;
    int m_lastMap = -1;
};
} // namespace App::Control

#else

#include <string>

namespace App::Control
{
// Player build: the same entry points the main loop calls, doing nothing.
class ControlServer
{
public:
    [[nodiscard]] static ControlServer& Instance()
    {
        static ControlServer instance;
        return instance;
    }

    bool Start(const std::string&)
    {
        return false;
    }
    void Poll() {}
    void Stop() {}
};
} // namespace App::Control

#endif // MU_ENABLE_CONTROL_SOCKET
