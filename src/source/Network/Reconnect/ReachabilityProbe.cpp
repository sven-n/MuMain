#include "stdafx.h"
#include "Network/Reconnect/ReachabilityProbe.h"

#include "Core/Platform/WinSock.h" // GetAddrInfoW + BSD-socket shims

namespace
{
void InitialiseSocketsOnce()
{
    static bool s_initialised = false;
    if (s_initialised)
    {
        return;
    }

    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    s_initialised = true;
}

bool ResolveIPv4(const wchar_t* host, unsigned short port, sockaddr_in& address)
{
    addrinfoW hints = {};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    addrinfoW* resolved = nullptr;
    if (GetAddrInfoW(host, nullptr, &hints, &resolved) != 0 || resolved == nullptr)
    {
        return false;
    }

    address = *reinterpret_cast<sockaddr_in*>(resolved->ai_addr);
    address.sin_port = htons(port);
    FreeAddrInfoW(resolved);
    return true;
}
} // namespace

ReachabilityProbe::~ReachabilityProbe()
{
    Stop();
}

bool ReachabilityProbe::Start(const wchar_t* host, unsigned short port, double nowMs, double timeoutMs)
{
    Stop();
    InitialiseSocketsOnce();

    sockaddr_in address = {};
    if (host == nullptr || !ResolveIPv4(host, port, address))
    {
        return false;
    }

    const SOCKET probe = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (probe == INVALID_SOCKET)
    {
        return false;
    }

    u_long nonBlocking = 1;
    ioctlsocket(probe, FIONBIO, &nonBlocking);

    connect(probe, reinterpret_cast<sockaddr*>(&address), sizeof(address)); // WSAEWOULDBLOCK expected

    m_socket = static_cast<uintptr_t>(probe);
    m_startMs = nowMs;
    m_timeoutMs = timeoutMs;
    return true;
}

ReachabilityProbe::Result ReachabilityProbe::Poll(double nowMs)
{
    if (!IsRunning())
    {
        return Result::Unreachable;
    }

    const SOCKET probe = static_cast<SOCKET>(m_socket);

    fd_set writeSet;
    fd_set errorSet;
    FD_ZERO(&writeSet);
    FD_ZERO(&errorSet);
    FD_SET(probe, &writeSet);
    FD_SET(probe, &errorSet);

    timeval immediate = {};
    // POSIX needs nfds = highest fd + 1; Windows ignores the first argument.
    select(static_cast<int>(probe + 1), nullptr, &writeSet, &errorSet, &immediate);

    if (FD_ISSET(probe, &writeSet))
    {
        int soError = 0;
        socklen_t length = sizeof(soError);
        getsockopt(probe, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&soError), &length);
        Stop();
        return soError == 0 ? Result::Reachable : Result::Unreachable;
    }

    if (FD_ISSET(probe, &errorSet) || (nowMs - m_startMs) > m_timeoutMs)
    {
        Stop();
        return Result::Unreachable;
    }

    return Result::Pending;
}

void ReachabilityProbe::Stop()
{
    if (m_socket == NoSocket)
    {
        return;
    }

    closesocket(static_cast<SOCKET>(m_socket));
    m_socket = NoSocket;
}

bool ReachabilityProbe::IsRunning() const
{
    return m_socket != NoSocket;
}
