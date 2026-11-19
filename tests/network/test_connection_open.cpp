// A Connection through the managed library against real loopback sockets: the constructor returns at once, and
// the state changes reach the game thread through the packet queue.

#include "stdafx.h"

#include "doctest.h"

#include "Core/Platform/WinSock.h"
#include "Dotnet/Connection.h"
#include "Dotnet/SymbolSources/ManagedLibrarySource.h"
#include "Network/IncomingPacketQueue.h"

#include <atomic>
#include <chrono>
#include <memory>
#include <thread>

namespace
{
using Network::ConnectionState;
using std::chrono::milliseconds;
using std::chrono::steady_clock;

void IgnorePacket(int32_t, const BYTE*, int32_t) {}

// Packets arrive on the library's threads.
std::atomic<int32_t> s_lastPacketSize = 0;
void RecordPacket(int32_t, const BYTE*, int32_t size)
{
    s_lastPacketSize = size;
}

void ApplyEvents(const PacketInfo* entry)
{
    if (entry->Event != Network::ConnectionEvent::None)
    {
        Connection::ApplyQueuedEvent(entry->ConnectionHandle, entry->Event);
    }
}

// Drains like the game does once per frame, until the connection is no longer connecting or 5 s passed.
ConnectionState WaitForAnswer(const Connection& connection)
{
    const auto deadline = steady_clock::now() + std::chrono::seconds(5);
    while (connection.State() == ConnectionState::Connecting && steady_clock::now() < deadline)
    {
        std::this_thread::sleep_for(milliseconds(10));
        Network::IncomingPacketQueue::Instance().DrainTo(&ApplyEvents);
    }
    return connection.State();
}

// A TCP listener on 127.0.0.1 at a port the system picks. The handshake completes without an accept.
class LoopbackListener
{
public:
    LoopbackListener()
    {
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);

        m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        sockaddr_in address = {};
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        address.sin_port = 0;
        bind(m_socket, reinterpret_cast<sockaddr*>(&address), sizeof(address));
        listen(m_socket, 1);

        socklen_t length = sizeof(address);
        getsockname(m_socket, reinterpret_cast<sockaddr*>(&address), &length);
        m_port = ntohs(address.sin_port);
    }

    ~LoopbackListener()
    {
        Close();
    }

    LoopbackListener(const LoopbackListener&) = delete;
    LoopbackListener& operator=(const LoopbackListener&) = delete;

    void Close()
    {
        if (m_accepted != INVALID_SOCKET)
        {
            closesocket(m_accepted);
            m_accepted = INVALID_SOCKET;
        }
        if (m_socket != INVALID_SOCKET)
        {
            closesocket(m_socket);
            m_socket = INVALID_SOCKET;
        }
    }

    // Takes the connection waiting in the backlog and sends it `bytes`, as a server greets a client.
    void AcceptAndSend(const char* bytes, int length)
    {
        m_accepted = accept(m_socket, nullptr, nullptr);
        send(m_accepted, bytes, length, 0);
    }

    unsigned short Port() const
    {
        return m_port;
    }

private:
    SOCKET m_socket = INVALID_SOCKET;
    SOCKET m_accepted = INVALID_SOCKET;
    unsigned short m_port = 0;
};

void RequireLibrary()
{
    InitializeDotNetBridge(DotNetBridge::TargetManagedSymbolSource());
    REQUIRE(munique_client_library_handle != nullptr);
    REQUIRE(DotNetBridge::ProcessManagedSymbols().State() == DotNetBridge::ManagedSymbolsState::Resolved);
}
} // namespace

TEST_CASE("A connection to a listening server opens [network][dotnet]")
{
    RequireLibrary();
    LoopbackListener listener;
    Connection connection(u"127.0.0.1", static_cast<int32_t>(listener.Port()), false, &IgnorePacket);
    CHECK(connection.State() == ConnectionState::Connecting);
    CHECK(WaitForAnswer(connection) == ConnectionState::Open);
    CHECK_FALSE(connection.FailedToOpen());
}

TEST_CASE("A refused connection closes without opening [network][dotnet]")
{
    RequireLibrary();
    LoopbackListener listener;
    const auto port = static_cast<int32_t>(listener.Port());
    listener.Close();
    Connection connection(u"127.0.0.1", port, false, &IgnorePacket);
    CHECK(WaitForAnswer(connection) == ConnectionState::Closed);
    CHECK(connection.FailedToOpen());
    CHECK(connection.GetHandle() == 0);
}

TEST_CASE("A connection opened without receiving keeps the greeting until it starts [network][dotnet]")
{
    // The in-game reconnect opens it while the world is still up and logs in over it after the teardown.
    RequireLibrary();
    LoopbackListener listener;
    s_lastPacketSize = 0;
    Connection connection(u"127.0.0.1", static_cast<int32_t>(listener.Port()), false, &RecordPacket,
                          /*startReceiving=*/false);
    REQUIRE(WaitForAnswer(connection) == ConnectionState::Open);
    const char greeting[] = {static_cast<char>(0xC1), 0x04, static_cast<char>(0xF1), 0x00};
    listener.AcceptAndSend(greeting, sizeof(greeting));
    std::this_thread::sleep_for(milliseconds(200));
    CHECK(s_lastPacketSize == 0);

    connection.StartReceiving();
    const auto deadline = steady_clock::now() + std::chrono::seconds(5);
    while (s_lastPacketSize == 0 && steady_clock::now() < deadline)
    {
        std::this_thread::sleep_for(milliseconds(10));
    }
    CHECK(s_lastPacketSize == static_cast<int32_t>(sizeof(greeting)));
}

TEST_CASE("Connecting to a server that never answers returns at once [network][dotnet]")
{
    RequireLibrary();
    const auto started = steady_clock::now();
    auto connection = std::make_unique<Connection>(u"10.255.255.1", 44405, false, &IgnorePacket);
    CHECK(steady_clock::now() - started < milliseconds(500));
    CHECK(connection->State() == ConnectionState::Connecting);

    // Destroyed while connecting: whatever the network reports later finds nothing.
    const int32_t handle = connection->GetHandle();
    connection.reset();
    CHECK_FALSE(Connection::ApplyQueuedEvent(handle, Network::ConnectionEvent::Opened));
    std::this_thread::sleep_for(milliseconds(100));
    Network::IncomingPacketQueue::Instance().DrainTo(&ApplyEvents);
}
