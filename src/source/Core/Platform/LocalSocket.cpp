#include "Core/Platform/LocalSocket.h"

#include <cstring>

#ifdef _WIN32
#include <afunix.h>
#include <io.h>
#else
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>
#endif

namespace
{
// Owner read/write only: the socket is developer tooling for the launching
// user, never a service other accounts may reach.
constexpr unsigned int SocketFileMode = 0600;
constexpr char LineTerminator = '\n';

bool SetNonBlocking(SOCKET handle)
{
    u_long nonBlocking = 1;
    return ioctlsocket(handle, FIONBIO, &nonBlocking) != SOCKET_ERROR;
}

bool WouldBlock(int error)
{
#ifdef _WIN32
    return error == WSAEWOULDBLOCK;
#else
    return error == EWOULDBLOCK || error == EAGAIN || error == EINTR;
#endif
}

void ApplyOwnerOnlyMode(SOCKET handle, const std::string& path)
{
#ifdef _WIN32
    // Windows has no POSIX mode bits on the socket file; the containing
    // directory's ACL is the access boundary there.
    (void)handle;
    (void)path;
#else
    // The umask guard around bind() already created the file with the right
    // mode; these calls make it explicit for the odd umask/filesystem, and
    // fchmod on a socket descriptor is not portable enough to rely on alone.
    if (::fchmod(handle, SocketFileMode) != 0)
    {
        ::chmod(path.c_str(), SocketFileMode);
    }
#endif
}

// Winsock must be started before the first socket call. The client's network
// code starts it too, but the control socket opens earlier than that and
// WSAStartup is reference counted, so doing it here removes the ordering
// assumption. Both calls are no-ops on POSIX.
void EnsureSocketLibrary()
{
    static const bool started = []
    {
        WSADATA data{};
        return WSAStartup(MAKEWORD(2, 2), &data) == 0;
    }();
    (void)started;
}

std::string DescribeLastSocketError()
{
    const int error = WSAGetLastError();
#ifdef _WIN32
    return "error " + std::to_string(error);
#else
    return std::strerror(error);
#endif
}
} // namespace

namespace Core::Platform
{
LocalSocketConnection::LocalSocketConnection(SOCKET handle) : m_handle(handle) {}

LocalSocketConnection::~LocalSocketConnection()
{
    Close();
}

void LocalSocketConnection::Close()
{
    if (m_handle == INVALID_SOCKET)
    {
        return;
    }

    closesocket(m_handle);
    m_handle = INVALID_SOCKET;
}

bool LocalSocketConnection::ReadAvailable()
{
    if (m_handle == INVALID_SOCKET)
    {
        return false;
    }

    char chunk[ReadChunkBytes];
    std::size_t taken = 0;
    while (taken < MaxBytesPerRead)
    {
        const auto received = ::recv(m_handle, chunk, static_cast<int>(sizeof(chunk)), 0);
        if (received > 0)
        {
            taken += static_cast<std::size_t>(received);
            Buffer(chunk, static_cast<std::size_t>(received));
            if (PendingLineBytes() > MaxPendingInputBytes || m_inbox.size() > MaxTotalInputBytes)
            {
                Close();
                return false;
            }
            continue;
        }

        if (received == 0)
        {
            // Orderly shutdown by the peer.
            Close();
            return false;
        }

        if (WouldBlock(WSAGetLastError()))
        {
            return true;
        }

        Close();
        return false;
    }

    return true;
}

void LocalSocketConnection::Buffer(const char* data, std::size_t size)
{
    const std::string_view chunk(data, size);
    const std::size_t lastTerminator = chunk.find_last_of(LineTerminator);
    if (lastTerminator == std::string_view::npos)
    {
        m_pendingLineBytes += size;
    }
    else
    {
        m_pendingLineBytes = size - lastTerminator - 1;
    }
    m_inbox.append(chunk);
}

bool LocalSocketConnection::TakeLine(std::string& line)
{
    const std::size_t end = m_inbox.find(LineTerminator);
    if (end == std::string::npos)
    {
        return false;
    }

    line.assign(m_inbox, 0, end);
    m_inbox.erase(0, end + 1);

    // Tolerate CRLF from tools that send Windows line endings.
    if (!line.empty() && line.back() == '\r')
    {
        line.pop_back();
    }
    return true;
}

bool LocalSocketConnection::Write(std::string_view payload)
{
    if (m_handle == INVALID_SOCKET)
    {
        return false;
    }

    m_outbox.append(payload);
    return Flush();
}

bool LocalSocketConnection::Flush()
{
    if (m_handle == INVALID_SOCKET)
    {
        return false;
    }

    while (!m_outbox.empty())
    {
        const auto sent = ::send(m_handle, m_outbox.data(), static_cast<int>(m_outbox.size()), 0);
        if (sent > 0)
        {
            m_outbox.erase(0, static_cast<std::size_t>(sent));
            continue;
        }

        if (sent < 0 && WouldBlock(WSAGetLastError()))
        {
            // Peer is not reading yet; the rest goes out on a later poll.
            return true;
        }

        Close();
        return false;
    }

    return true;
}

LocalSocketListener::~LocalSocketListener()
{
    Close();
}

std::size_t LocalSocketListener::MaxPathLength()
{
    sockaddr_un address{};
    return sizeof(address.sun_path) - 1;
}

void LocalSocketListener::Unlink(const std::string& path)
{
    if (path.empty())
    {
        return;
    }

#ifdef _WIN32
    ::_unlink(path.c_str());
#else
    ::unlink(path.c_str());
#endif
}

namespace
{
// Whether a socket file is a live listener rather than one left behind: the
// only portable answer is to knock on it.
bool SomethingIsListening(const std::string& path)
{
    if (path.size() > Core::Platform::LocalSocketListener::MaxPathLength())
    {
        return false;
    }

    const SOCKET probe = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (probe == INVALID_SOCKET)
    {
        return false;
    }

    sockaddr_un address{};
    address.sun_family = AF_UNIX;
    std::memcpy(address.sun_path, path.c_str(), path.size());
    const bool connected =
        ::connect(probe, reinterpret_cast<const sockaddr*>(&address), sizeof(address)) != SOCKET_ERROR;
    closesocket(probe);
    return connected;
}
} // namespace

bool LocalSocketListener::Listen(const std::string& path, std::string& error)
{
    Close();

    if (path.empty())
    {
        error = "control socket path is empty";
        return false;
    }

    if (path.size() > MaxPathLength())
    {
        error = "control socket path is longer than " + std::to_string(MaxPathLength()) + " bytes";
        return false;
    }

    EnsureSocketLibrary();

    const SOCKET handle = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (handle == INVALID_SOCKET)
    {
        error = "socket() failed: " + DescribeLastSocketError();
        return false;
    }

    // A file left behind by a crashed client would make bind() fail with
    // EADDRINUSE even though nothing is listening on it — but a path a live
    // client is serving must not be taken from it, which unlinking blindly
    // would do silently: the first client keeps its socket open and never
    // hears from anyone again. A connection that is accepted says somebody
    // is there.
    if (SomethingIsListening(path))
    {
        error = "another client is already listening on " + path;
        closesocket(handle);
        return false;
    }

    Unlink(path);

    sockaddr_un address{};
    address.sun_family = AF_UNIX;
    std::memcpy(address.sun_path, path.c_str(), path.size());

#ifndef _WIN32
    const mode_t previousMask = ::umask(~SocketFileMode & 0777);
#endif
    const int bound = ::bind(handle, reinterpret_cast<const sockaddr*>(&address), sizeof(address));
#ifndef _WIN32
    ::umask(previousMask);
#endif

    if (bound == SOCKET_ERROR)
    {
        error = "bind(" + path + ") failed: " + DescribeLastSocketError();
        closesocket(handle);
        return false;
    }

    ApplyOwnerOnlyMode(handle, path);

    if (!SetNonBlocking(handle))
    {
        error = "setting the control socket non-blocking failed: " + DescribeLastSocketError();
        closesocket(handle);
        Unlink(path);
        return false;
    }

    if (::listen(handle, PendingConnectionBacklog) == SOCKET_ERROR)
    {
        error = "listen() failed: " + DescribeLastSocketError();
        closesocket(handle);
        Unlink(path);
        return false;
    }

    m_handle = handle;
    m_path = path;
    return true;
}

std::unique_ptr<LocalSocketConnection> LocalSocketListener::Accept()
{
    if (m_handle == INVALID_SOCKET)
    {
        return nullptr;
    }

    const SOCKET accepted = ::accept(m_handle, nullptr, nullptr);
    if (accepted == INVALID_SOCKET)
    {
        return nullptr;
    }

    if (!SetNonBlocking(accepted))
    {
        closesocket(accepted);
        return nullptr;
    }

    return std::make_unique<LocalSocketConnection>(accepted);
}

void LocalSocketListener::Close()
{
    if (m_handle != INVALID_SOCKET)
    {
        closesocket(m_handle);
        m_handle = INVALID_SOCKET;
    }

    if (!m_path.empty())
    {
        Unlink(m_path);
        m_path.clear();
    }
}
} // namespace Core::Platform
