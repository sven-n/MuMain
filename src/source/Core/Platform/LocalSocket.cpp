#include "Core/Platform/LocalSocket.h"

#include <cstring>

#ifdef _WIN32
#include <afunix.h>
#include <io.h>
#else
#include <poll.h>
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

// send() flags: on Linux a write to a peer that has closed raises SIGPIPE,
// whose default action would end the game process; MSG_NOSIGNAL asks for the
// error return instead. Winsock has neither the signal nor the flag.
#ifdef _WIN32
constexpr int SendFlags = 0;
#else
constexpr int SendFlags = MSG_NOSIGNAL;
#endif

bool SetNonBlocking(SOCKET handle)
{
    u_long nonBlocking = 1;
    return ioctlsocket(handle, FIONBIO, &nonBlocking) != SOCKET_ERROR;
}

// Whether a non-blocking connect has not finished yet, rather than failed.
bool ConnectPending(int error)
{
#ifdef _WIN32
    return error == WSAEWOULDBLOCK || error == WSAEINPROGRESS;
#else
    return error == EINPROGRESS || error == EAGAIN || error == EWOULDBLOCK;
#endif
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
    if (m_peerClosed)
    {
        // Nothing more will arrive, and reading again would only rediscover
        // the shutdown; what is buffered is still served, and the server
        // closes the connection once it owes the peer nothing.
        return IsOpen();
    }

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
            // Orderly shutdown by the peer. Whatever it managed to send
            // before closing is still a request: a one-shot script writes
            // its line and shuts its end down in the same breath, and
            // dropping the line here is dropping the only thing it asked.
            m_peerClosed = true;
            if (HasLine())
            {
                return true;
            }
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

bool LocalSocketConnection::HasLine() const
{
    return m_inbox.find(LineTerminator) != std::string::npos;
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
    if (!Flush())
    {
        return false;
    }

    // Judged on what the kernel would not take: a large line is fine as long
    // as it goes out, and only a peer that has stopped reading leaves this
    // much behind. Dropping it is the same answer the inbox caps give.
    if (m_outbox.size() > MaxPendingOutputBytes)
    {
        Close();
        return false;
    }
    return true;
}

bool LocalSocketConnection::Flush()
{
    if (m_handle == INVALID_SOCKET)
    {
        return false;
    }

    while (!m_outbox.empty())
    {
        const auto sent = ::send(m_handle, m_outbox.data(), static_cast<int>(m_outbox.size()), SendFlags);
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
// Whether a failed connect means something is still there. Windows' AF_UNIX
// answers a path with no listener through more than one code depending on the
// build, so all of them count as stale.
bool ProbeFailureIsLive(int failure)
{
#ifdef _WIN32
    return failure != WSAECONNREFUSED && failure != WSAENOENT && failure != WSAEINVAL && failure != WSAEFAULT;
#else
    return failure != ECONNREFUSED && failure != ENOENT;
#endif
}

// How long the probe below waits for a connect it could not answer at once.
constexpr int ProbeSettleMilliseconds = 100;

// Whether a connect that returned "in flight" ends up accepted. Without this
// the probe would have to guess, and guessing "alive" makes a stale file stop
// the client from starting.
bool ProbeSettled(SOCKET probe)
{
#ifdef _WIN32
    // Winsock's fd_set holds handles, not indices, so one of them is one.
    fd_set writable;
    FD_ZERO(&writable);
    FD_SET(probe, &writable);
    timeval wait{};
    wait.tv_sec = 0;
    wait.tv_usec = ProbeSettleMilliseconds * 1000;
    const int ready = ::select(0, nullptr, &writable, nullptr, &wait);
#else
    // poll(), not select(): a descriptor at or past FD_SETSIZE cannot be put
    // into an fd_set at all, and a client with many files open would write
    // past it.
    pollfd waiting{};
    waiting.fd = probe;
    waiting.events = POLLOUT;
    const int ready = ::poll(&waiting, 1, ProbeSettleMilliseconds);
#endif
    if (ready <= 0)
    {
        // Still undecided after the wait: a listener that never answers is
        // one we must not take the path from.
        return true;
    }

    int error = 0;
#ifdef _WIN32
    int size = static_cast<int>(sizeof(error));
#else
    socklen_t size = sizeof(error);
#endif
    if (::getsockopt(probe, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&error), &size) != 0)
    {
        return true;
    }

    return error == 0 || ProbeFailureIsLive(error);
}

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
        // Could not ask — out of descriptors, most likely. That is not an
        // answer of "nothing is listening", and acting on it would unlink a
        // running client's socket.
        return true;
    }

    // Non-blocking, because a listener whose accept backlog is full makes a
    // blocking connect() wait for it and that would stall start-up on the
    // very case this probe detects. If the mode cannot be changed the probe
    // is abandoned rather than run blocking: not detecting a second client is
    // better than refusing to start.
    if (!SetNonBlocking(probe))
    {
        closesocket(probe);
        return true;
    }

    sockaddr_un address{};
    address.sun_family = AF_UNIX;
    std::memcpy(address.sun_path, path.c_str(), path.size());
    const bool connected =
        ::connect(probe, reinterpret_cast<const sockaddr*>(&address), sizeof(address)) != SOCKET_ERROR;
    const int failure = connected ? 0 : WSAGetLastError();

    // Only a refusal says the file is stale. A full accept backlog and a
    // socket we may not talk to (EACCES, EPERM) are live sockets, and
    // unlinking one would take it from its owner; a connect still in flight
    // is waited on rather than assumed either way.
    const bool listening = connected                 ? true
                           : ConnectPending(failure) ? ProbeSettled(probe)
                                                     : ProbeFailureIsLive(failure);
    closesocket(probe);
    return listening;
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
#ifndef _WIN32
    // Only a socket is ever ours to remove: a connect to a regular file
    // answers ECONNREFUSED as well, and a mistyped MU_CONTROL_SOCKET must
    // not delete the file it points at.
    struct stat entry{};
    if (::stat(path.c_str(), &entry) == 0 && !S_ISSOCK(entry.st_mode))
    {
        error = path + " exists and is not a socket";
        closesocket(handle);
        return false;
    }
#endif

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
