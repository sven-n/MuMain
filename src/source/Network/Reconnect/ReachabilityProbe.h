#pragma once

#include <cstdint>

// Finds out whether a server accepts connections, without waiting for it.
//
// A plain connect blocks until the operating system gives up, which takes many
// seconds for a server that is down. The probe connects non-blocking instead and
// is polled once per frame, so the game keeps rendering while it waits. It only
// opens and closes a TCP connection; the real connection is made afterwards.
class ReachabilityProbe
{
public:
    enum class Result
    {
        Pending,     // still connecting
        Reachable,   // the server accepted the connection
        Unreachable, // refused, failed or timed out
    };

    ReachabilityProbe() = default;
    ~ReachabilityProbe();
    ReachabilityProbe(const ReachabilityProbe&) = delete;
    ReachabilityProbe& operator=(const ReachabilityProbe&) = delete;

    // Starts connecting to `host` (an IPv4 literal or a host name) at `port`. Returns false when
    // no probe could be started (no socket, or the name doesn't resolve right now).
    bool Start(const wchar_t* host, unsigned short port, double nowMs, double timeoutMs);

    // Checks the probe without waiting. Reachable and Unreachable end the probe.
    Result Poll(double nowMs);

    void Stop();
    bool IsRunning() const;

private:
    static constexpr uintptr_t NoSocket = static_cast<uintptr_t>(~0ull); // == INVALID_SOCKET

    uintptr_t m_socket = NoSocket; // holds a winsock SOCKET
    double m_startMs = 0.0;
    double m_timeoutMs = 0.0;
};
