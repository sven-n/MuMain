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
    // Bytes read from the socket per recv() call.
    static constexpr std::size_t ReadChunkBytes = 4096;
    // Bytes taken from the socket in one ReadAvailable() pass. A local peer
    // can write faster than the frame loop reads, and the loop below only
    // stops when the socket runs dry, so the work of one pass is bounded and
    // the rest waits for the next frame.
    static constexpr std::size_t MaxBytesPerRead = std::size_t{256} * 1024;
    // A single request line longer than this is treated as abuse and closes
    // the connection instead of growing the buffer without bound.
    static constexpr std::size_t MaxPendingInputBytes = std::size_t{256} * 1024;
    // Ceiling on everything buffered, complete lines included. Generous
    // enough that an honest pipelined batch never meets it, while a peer
    // that writes faster than its requests are served still hits a wall
    // instead of growing the buffer for as long as it keeps writing.
    static constexpr std::size_t MaxTotalInputBytes = std::size_t{4} * 1024 * 1024;

    // And the same for what is waiting to go out. A follower that stops
    // reading (a stopped process, a full pipe) leaves every event queued
    // here; without a ceiling the client grows a send buffer until it dies
    // of it, for a peer that is no longer listening.
    static constexpr std::size_t MaxPendingOutputBytes = std::size_t{4} * 1024 * 1024;

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
    // Buffers a chunk just read from the socket, keeping the size of the
    // unterminated tail up to date: what the peer has sent since its last
    // newline. Counted as it arrives rather than searched for afterwards, so
    // filling the buffer costs what it appends and not a scan per chunk.
    void Buffer(const char* data, std::size_t size);

    // Bytes of the unterminated tail of the inbox. Complete lines are
    // counted separately: a pipelined batch of valid commands is not a
    // request line that never ends.
    [[nodiscard]] std::size_t PendingLineBytes() const
    {
        return m_pendingLineBytes;
    }

    SOCKET m_handle;
    std::string m_inbox;
    std::size_t m_pendingLineBytes = 0;
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

    // Binds and listens. A stale file left by a crashed client is removed
    // first; a path another client is still listening on is refused instead,
    // so two clients cannot end up sharing one name. On failure the reason is
    // in `error` and nothing is left behind.
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
