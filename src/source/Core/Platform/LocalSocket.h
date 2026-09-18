// Local stream socket transport (AF_UNIX), for Windows and Linux alike.
//
// A listener bound to a filesystem path, serving line-oriented text to local
// processes of the same user. Everything is non-blocking: the owner polls it
// from the frame loop, so no call here waits on the network.
//
// The platform-specific parts sit behind the Winsock shim; the transport and
// its tests run on both platforms.
#pragma once

#include "Core/Platform/WinSock.h"

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>

namespace Core::Platform
{
// One accepted connection. Reads are buffered until a whole line arrives;
// writes are buffered until the peer's receive window takes them.
class LocalSocketConnection
{
public:
    // Bytes read from the socket per ReadAvailable() pass.
    static constexpr std::size_t ReadChunkBytes = 4096;
    // A single request line longer than this is treated as abuse and closes
    // the connection instead of growing the buffer without bound.
    static constexpr std::size_t MaxPendingInputBytes = std::size_t{256} * 1024;

    explicit LocalSocketConnection(SOCKET handle);
    ~LocalSocketConnection();

    LocalSocketConnection(const LocalSocketConnection&) = delete;
    LocalSocketConnection& operator=(const LocalSocketConnection&) = delete;

    [[nodiscard]] bool IsOpen() const
    {
        return m_handle != INVALID_SOCKET;
    }

    // Moves whatever the socket has ready into the line buffer. Returns
    // false when the peer closed the connection or the socket failed; the
    // connection is closed in that case.
    bool ReadAvailable();

    // Pops one complete line (without its terminator) from the line buffer.
    [[nodiscard]] bool TakeLine(std::string& line);

    // Queues a payload and tries to send it right away. Returns false when
    // the connection is gone.
    bool Write(std::string_view payload);

    // Pushes queued bytes out as far as the socket accepts them.
    bool Flush();

    [[nodiscard]] bool HasPendingOutput() const
    {
        return !m_outbox.empty();
    }

    void Close();

private:
    SOCKET m_handle;
    std::string m_inbox;
    std::string m_outbox;
};

// Listening endpoint. Owns the socket file: binding replaces a stale file
// at the path, and closing removes it.
class LocalSocketListener
{
public:
    // Pending connections the kernel may queue before Accept() runs.
    static constexpr int PendingConnectionBacklog = 8;

    LocalSocketListener() = default;
    ~LocalSocketListener();

    LocalSocketListener(const LocalSocketListener&) = delete;
    LocalSocketListener& operator=(const LocalSocketListener&) = delete;

    // Longest path that fits a sockaddr_un, terminator excluded.
    [[nodiscard]] static std::size_t MaxPathLength();

    // Removes a socket file, whether or not anything is listening on it.
    static void Unlink(const std::string& path);

    // Binds and listens. Any existing file at the path is removed first, so
    // a crashed client's socket does not block start-up. On failure the
    // reason is in `error` and nothing is left behind.
    [[nodiscard]] bool Listen(const std::string& path, std::string& error);

    [[nodiscard]] bool IsListening() const
    {
        return m_handle != INVALID_SOCKET;
    }
    [[nodiscard]] const std::string& Path() const
    {
        return m_path;
    }

    // Returns the next queued connection, or nullptr when none is waiting.
    [[nodiscard]] std::unique_ptr<LocalSocketConnection> Accept();

    // Closes the listening socket and removes its file.
    void Close();

private:
    SOCKET m_handle = INVALID_SOCKET;
    std::string m_path;
};
} // namespace Core::Platform
