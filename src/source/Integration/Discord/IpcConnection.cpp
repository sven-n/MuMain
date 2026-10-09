#include "Integration/Discord/IpcConnection.h"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <thread>
#include <vector>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#endif

namespace
{
constexpr const char* EndpointName = "discord-ipc-";

#ifdef _WIN32
constexpr const char* PipePrefix = "\\\\.\\pipe\\";

std::vector<std::string> EndpointDirectories()
{
    return {PipePrefix};
}
#else
#ifdef MSG_NOSIGNAL
constexpr int SendFlags = MSG_NOSIGNAL;
#else
constexpr int SendFlags = 0; // macOS: SO_NOSIGPIPE is set on the socket instead
#endif

// Where the Discord app may have created its socket: the runtime and
// temporary directories it consults, plus the sandbox directories of the
// Flatpak and Snap packages, which a native client can still reach.
std::vector<std::string> EndpointDirectories()
{
    constexpr std::array<const char*, 4> BaseVariables = {"XDG_RUNTIME_DIR", "TMPDIR", "TMP", "TEMP"};
    constexpr std::array<const char*, 4> SandboxSubdirectories = {
        "", "app/com.discordapp.Discord/", ".flatpak/com.discordapp.Discord/xdg-run/", "snap.discord/"};
    constexpr const char* FallbackBase = "/tmp";

    std::vector<std::string> bases;
    for (const char* variable : BaseVariables)
    {
        const char* value = std::getenv(variable);
        if (value != nullptr && value[0] != '\0')
        {
            bases.emplace_back(value);
        }
    }
    bases.emplace_back(FallbackBase);

    std::vector<std::string> directories;
    for (std::string base : bases)
    {
        if (base.back() != '/')
        {
            base.push_back('/');
        }
        for (const char* subdirectory : SandboxSubdirectories)
        {
            std::string directory = base + subdirectory;
            if (std::find(directories.begin(), directories.end(), directory) == directories.end())
            {
                directories.push_back(std::move(directory));
            }
        }
    }
    return directories;
}
#endif
} // namespace

namespace Integration::Discord
{
IpcConnection::~IpcConnection()
{
    Close();
}

bool IpcConnection::Open()
{
    Close();
    for (const std::string& directory : EndpointDirectories())
    {
        for (int index = 0; index <= MaxEndpointIndex; ++index)
        {
            if (OpenEndpoint(directory + EndpointName + std::to_string(index)))
            {
                return true;
            }
        }
    }
    return false;
}

bool IpcConnection::Send(Ipc::Opcode opcode, std::string_view payload)
{
    if (!IsOpen())
    {
        return false;
    }
    if (!WriteAll(Ipc::EncodeFrame(opcode, payload)))
    {
        Close();
        return false;
    }
    return true;
}

IpcConnection::ReadStatus IpcConnection::Read(Frame& frame, std::chrono::milliseconds timeout,
                                              const std::atomic<bool>& cancelled)
{
    if (!IsOpen())
    {
        return ReadStatus::Closed;
    }

    std::array<std::uint8_t, Ipc::HeaderBytes> header{};
    std::size_t received = 0;
    const ReadStatus headerStatus =
        ReadExact(reinterpret_cast<char*>(header.data()), header.size(), Clock::now() + timeout, cancelled, received);
    if (headerStatus == ReadStatus::Timeout && received == 0)
    {
        return ReadStatus::Timeout;
    }

    const auto decoded = headerStatus == ReadStatus::Frame ? Ipc::DecodeHeader(header) : std::nullopt;
    if (!decoded.has_value())
    {
        Close();
        return ReadStatus::Closed;
    }

    frame.opcode = decoded->opcode;
    frame.payload.assign(decoded->length, '\0');
    const ReadStatus payloadStatus = ReadExact(frame.payload.data(), frame.payload.size(),
                                               Clock::now() + FrameCompletionTimeout, cancelled, received);
    if (payloadStatus != ReadStatus::Frame)
    {
        Close();
        return ReadStatus::Closed;
    }
    return ReadStatus::Frame;
}

IpcConnection::ReadStatus IpcConnection::ReadExact(char* buffer, std::size_t size, Clock::time_point deadline,
                                                   const std::atomic<bool>& cancelled, std::size_t& received)
{
    // Looks at the connection at least once, so a zero timeout still picks
    // up what has already arrived.
    received = 0;
    while (received < size)
    {
        if (cancelled.load())
        {
            return ReadStatus::Timeout;
        }

        const auto remaining = std::max(std::chrono::duration_cast<std::chrono::milliseconds>(deadline - Clock::now()),
                                        std::chrono::milliseconds{0});
        const long count = ReadSome(buffer + received, size - received, std::min(remaining, PollSlice));
        if (count < 0)
        {
            Close();
            return ReadStatus::Closed;
        }
        received += static_cast<std::size_t>(count);
        if (count == 0 && Clock::now() >= deadline)
        {
            return ReadStatus::Timeout;
        }
    }
    return ReadStatus::Frame;
}

#ifdef _WIN32

bool IpcConnection::IsOpen() const
{
    return m_handle != nullptr;
}

void IpcConnection::Close()
{
    if (m_handle != nullptr)
    {
        CloseHandle(static_cast<HANDLE>(m_handle));
        m_handle = nullptr;
    }
}

bool IpcConnection::OpenEndpoint(const std::string& path)
{
    const std::wstring widePath(path.begin(), path.end()); // the pipe path is ASCII
    HANDLE pipe = CreateFileW(widePath.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
    if (pipe == INVALID_HANDLE_VALUE)
    {
        return false;
    }
    m_handle = pipe;
    return true;
}

long IpcConnection::ReadSome(char* buffer, std::size_t size, std::chrono::milliseconds wait)
{
    // Synchronous ReadFile on a pipe waits without a timeout, so look first
    // and only read what has already arrived.
    DWORD available = 0;
    if (!PeekNamedPipe(static_cast<HANDLE>(m_handle), nullptr, 0, nullptr, &available, nullptr))
    {
        return -1;
    }
    if (available == 0)
    {
        std::this_thread::sleep_for(wait);
        return 0;
    }

    const DWORD wanted = static_cast<DWORD>(std::min<std::size_t>(size, available));
    DWORD read = 0;
    if (!ReadFile(static_cast<HANDLE>(m_handle), buffer, wanted, &read, nullptr))
    {
        return -1;
    }
    return static_cast<long>(read);
}

bool IpcConnection::WriteAll(std::string_view bytes)
{
    while (!bytes.empty())
    {
        DWORD written = 0;
        if (!WriteFile(static_cast<HANDLE>(m_handle), bytes.data(), static_cast<DWORD>(bytes.size()), &written,
                       nullptr) ||
            written == 0)
        {
            return false;
        }
        bytes.remove_prefix(written);
    }
    return true;
}

#else

bool IpcConnection::IsOpen() const
{
    return m_handle >= 0;
}

void IpcConnection::Close()
{
    if (m_handle >= 0)
    {
        ::close(m_handle);
        m_handle = -1;
    }
}

bool IpcConnection::OpenEndpoint(const std::string& path)
{
    sockaddr_un address{};
    if (path.size() >= sizeof(address.sun_path))
    {
        return false;
    }
    address.sun_family = AF_UNIX;
    std::memcpy(address.sun_path, path.c_str(), path.size() + 1);

    const int socketHandle = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (socketHandle < 0)
    {
        return false;
    }
    ::fcntl(socketHandle, F_SETFD, FD_CLOEXEC);
#ifdef SO_NOSIGPIPE
    const int enabled = 1;
    ::setsockopt(socketHandle, SOL_SOCKET, SO_NOSIGPIPE, &enabled, sizeof(enabled));
#endif

    if (::connect(socketHandle, reinterpret_cast<const sockaddr*>(&address), sizeof(address)) != 0)
    {
        ::close(socketHandle);
        return false;
    }
    m_handle = socketHandle;
    return true;
}

long IpcConnection::ReadSome(char* buffer, std::size_t size, std::chrono::milliseconds wait)
{
    pollfd watched{m_handle, POLLIN, 0};
    const int ready = ::poll(&watched, 1, static_cast<int>(wait.count()));
    if (ready < 0)
    {
        return errno == EINTR ? 0 : -1;
    }
    if (ready == 0)
    {
        return 0;
    }

    const ssize_t count = ::recv(m_handle, buffer, size, 0);
    if (count < 0 && errno == EINTR)
    {
        return 0;
    }
    // Zero is the peer's orderly shutdown: poll reported the socket readable
    // and nothing came.
    return count > 0 ? static_cast<long>(count) : -1;
}

bool IpcConnection::WriteAll(std::string_view bytes)
{
    while (!bytes.empty())
    {
        const ssize_t count = ::send(m_handle, bytes.data(), bytes.size(), SendFlags);
        if (count < 0 && errno == EINTR)
        {
            continue;
        }
        if (count <= 0)
        {
            return false;
        }
        bytes.remove_prefix(static_cast<std::size_t>(count));
    }
    return true;
}

#endif
} // namespace Integration::Discord
