// doctest unit tests for the non-blocking server check both reconnects use.
//
// Runs against real sockets on the loopback interface: a listener the probe
// reaches, and a port nobody listens on.
//
// Run: ctest --test-dir <build directory> --build-config Release -R "Reachability probe"

#include "stdafx.h"

#include "doctest.h"

#include "Core/Platform/WinSock.h"
#include "Network/Reconnect/ReachabilityProbe.h"

#include <chrono>
#include <thread>

namespace
{
constexpr double TimeoutMs = 5000.0;

double NowMs()
{
    using namespace std::chrono;
    return duration<double, std::milli>(steady_clock::now().time_since_epoch()).count();
}

// Polls like the game does once per frame, until the probe has an answer.
ReachabilityProbe::Result PollUntilAnswered(ReachabilityProbe& probe)
{
    constexpr auto Frame = std::chrono::milliseconds(10);
    ReachabilityProbe::Result result = probe.Poll(NowMs());
    while (result == ReachabilityProbe::Result::Pending)
    {
        std::this_thread::sleep_for(Frame);
        result = probe.Poll(NowMs());
    }
    return result;
}

// A TCP listener on 127.0.0.1 at a port the system picks.
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
        if (m_socket != INVALID_SOCKET)
        {
            closesocket(m_socket);
            m_socket = INVALID_SOCKET;
        }
    }

    unsigned short Port() const
    {
        return m_port;
    }

private:
    SOCKET m_socket = INVALID_SOCKET;
    unsigned short m_port = 0;
};
} // namespace

TEST_SUITE("Reachability probe")
{
    TEST_CASE("a listening server is reachable")
    {
        LoopbackListener listener;
        ReachabilityProbe probe;

        REQUIRE(probe.Start(L"127.0.0.1", listener.Port(), NowMs(), TimeoutMs));
        CHECK(probe.IsRunning());
        CHECK(PollUntilAnswered(probe) == ReachabilityProbe::Result::Reachable);
        CHECK_FALSE(probe.IsRunning());
    }

    TEST_CASE("a port nobody listens on is unreachable")
    {
        LoopbackListener listener;
        const unsigned short port = listener.Port();
        listener.Close();

        ReachabilityProbe probe;
        REQUIRE(probe.Start(L"127.0.0.1", port, NowMs(), TimeoutMs));
        CHECK(PollUntilAnswered(probe) == ReachabilityProbe::Result::Unreachable);
        CHECK_FALSE(probe.IsRunning());
    }

    TEST_CASE("a poll past the timeout ends the probe")
    {
        LoopbackListener listener;
        ReachabilityProbe probe;
        const double start = NowMs();

        REQUIRE(probe.Start(L"127.0.0.1", listener.Port(), start, 0.0));
        // Polled as if the frame came late: an answer that has arrived meanwhile
        // still counts, so only check that the probe ends either way.
        const ReachabilityProbe::Result result = probe.Poll(start + 1.0);
        CHECK(result != ReachabilityProbe::Result::Pending);
        CHECK_FALSE(probe.IsRunning());
    }

    TEST_CASE("no host starts no probe")
    {
        ReachabilityProbe probe;
        CHECK_FALSE(probe.Start(nullptr, 1, NowMs(), TimeoutMs));
        CHECK_FALSE(probe.IsRunning());
        CHECK(probe.Poll(NowMs()) == ReachabilityProbe::Result::Unreachable);
    }

    TEST_CASE("stopping ends a running probe")
    {
        LoopbackListener listener;
        ReachabilityProbe probe;

        REQUIRE(probe.Start(L"127.0.0.1", listener.Port(), NowMs(), TimeoutMs));
        probe.Stop();
        CHECK_FALSE(probe.IsRunning());
    }
}
