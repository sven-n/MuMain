// Client end of the local Discord app's RPC channel: the named pipe
// `\\.\pipe\discord-ipc-N` on Windows, the Unix socket `discord-ipc-N` in the
// runtime directory elsewhere.
//
// Blocking by design, with bounded waits: it is driven by the presence
// worker thread, never by the frame loop.
#pragma once

#include "Integration/Discord/IpcFrame.h"

#include <atomic>
#include <chrono>
#include <cstddef>
#include <string>
#include <string_view>

namespace Integration::Discord
{
class IpcConnection
{
public:
    // Discord numbers its endpoints from 0 and moves to the next number when
    // one is taken (several Discord builds running side by side).
    static constexpr int MaxEndpointIndex = 9;

    // Once a frame has started arriving, the rest of it must follow within
    // this time; a peer that stalls mid-frame has lost the framing.
    static constexpr std::chrono::milliseconds FrameCompletionTimeout{1000};

    // Longest single wait between two checks of the cancel flag.
    static constexpr std::chrono::milliseconds PollSlice{50};

    enum class ReadStatus
    {
        Frame,
        Timeout,
        Closed,
    };

    struct Frame
    {
        Ipc::Opcode opcode = Ipc::Opcode::Frame;
        std::string payload;
    };

    IpcConnection() = default;
    ~IpcConnection();

    IpcConnection(const IpcConnection&) = delete;
    IpcConnection& operator=(const IpcConnection&) = delete;

    // Connects to the first endpoint the Discord app is listening on.
    // False when Discord is not running.
    [[nodiscard]] bool Open();

    [[nodiscard]] bool IsOpen() const;

    void Close();

    // Writes one frame. False (and closed) when the peer is gone.
    bool Send(Ipc::Opcode opcode, std::string_view payload);

    // Waits up to `timeout` for the next frame. Closed means the connection
    // failed and has been closed; Timeout means nothing arrived in time, or
    // `cancelled` was raised while waiting.
    [[nodiscard]] ReadStatus Read(Frame& frame, std::chrono::milliseconds timeout, const std::atomic<bool>& cancelled);

private:
    using Clock = std::chrono::steady_clock;

    [[nodiscard]] bool OpenEndpoint(const std::string& path);

    // Reads what is there, waiting at most `wait` for something to arrive.
    // Returns the byte count, 0 when nothing came, a negative value when the
    // connection failed.
    [[nodiscard]] long ReadSome(char* buffer, std::size_t size, std::chrono::milliseconds wait);

    [[nodiscard]] bool WriteAll(std::string_view bytes);

    // Fills `buffer` completely unless the deadline passes or the wait is
    // cancelled; `received` says how far it got.
    [[nodiscard]] ReadStatus ReadExact(char* buffer, std::size_t size, Clock::time_point deadline,
                                       const std::atomic<bool>& cancelled, std::size_t& received);

#ifdef _WIN32
    void* m_handle = nullptr; // HANDLE of the pipe
#else
    int m_handle = -1; // socket descriptor
#endif
};
} // namespace Integration::Discord
