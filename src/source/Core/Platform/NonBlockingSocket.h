// Non-blocking socket calls, for Windows and POSIX alike: the local socket
// transport and its tests share them, so both treat "try again later" the
// same way.
#pragma once

#include "Core/Platform/WinSock.h"

namespace Core::Platform::NonBlockingSocket
{
// Switches a socket to non-blocking mode. Returns false when the mode could
// not be changed.
[[nodiscard]] inline bool Enable(SOCKET handle)
{
    u_long nonBlocking = 1;
    return ioctlsocket(handle, FIONBIO, &nonBlocking) != SOCKET_ERROR;
}

// Whether a call on a non-blocking socket failed only because it would have
// had to wait, and can simply be tried again later.
[[nodiscard]] inline bool WouldBlock(int error)
{
#ifdef _WIN32
    return error == WSAEWOULDBLOCK;
#else
    return error == EWOULDBLOCK || error == EAGAIN || error == EINTR;
#endif
}
} // namespace Core::Platform::NonBlockingSocket
