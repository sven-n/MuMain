// doctest unit tests for the local stream socket transport, on Windows and
// Linux alike.
//
// The listener is exercised against a real socket file in a temporary
// directory; the "client" is a plain blocking socket created by the test, so
// nothing here needs a window, a renderer or the game's globals.
//
// Run: ctest --test-dir <build directory> --build-config Release -R "\[core\]\[local-socket\]"

#include "doctest.h"

#include "Core/Platform/LocalSocket.h"

#include "Core/Platform/WinSock.h" // SOCKET, closesocket, WSAStartup (no-ops on POSIX)

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <thread>

#ifdef _WIN32
#include <afunix.h> // sockaddr_un
#include <process.h>
#else
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>
#endif

namespace
{
// Winsock has to be started before the first socket call; the shim makes both
// calls no-ops on POSIX.
void EnsureSocketLibrary()
{
    static const bool started = []
    {
        WSADATA data{};
        return WSAStartup(MAKEWORD(2, 2), &data) == 0;
    }();
    (void)started;
}

int CurrentProcessId()
{
#ifdef _WIN32
    return ::_getpid();
#else
    return ::getpid();
#endif
}

// A bound AF_UNIX socket file is a reparse point on Windows, which
// std::filesystem::exists() cannot stat; ask the file system for the
// attributes instead. Elsewhere the plain query is the right one.
bool SocketFileExists(const std::string& path)
{
#ifdef _WIN32
    return ::GetFileAttributesA(path.c_str()) != INVALID_FILE_ATTRIBUTES;
#else
    return std::filesystem::exists(path);
#endif
}

// A socket path unique to the running test binary, short enough for
// sockaddr_un on every platform.
std::filesystem::path MakeSocketDirectory()
{
    const auto directory =
        std::filesystem::temp_directory_path() / ("mu-local-socket-" + std::to_string(CurrentProcessId()));
    std::filesystem::remove_all(directory);
    std::filesystem::create_directories(directory);
    return directory;
}

// Blocking client side, standing in for a test script.
SOCKET ConnectTo(const std::string& path)
{
    EnsureSocketLibrary();

    const SOCKET handle = ::socket(AF_UNIX, SOCK_STREAM, 0);
    if (handle == INVALID_SOCKET)
    {
        return INVALID_SOCKET;
    }

    sockaddr_un address{};
    address.sun_family = AF_UNIX;
    // sun_path is 108 bytes on Linux and 104 with afunix.h: a longer path
    // would be copied past the end of the address, and the test would be
    // reporting a corrupted stack rather than the socket's behaviour.
    REQUIRE(path.size() < sizeof(address.sun_path));
    std::memcpy(address.sun_path, path.c_str(), path.size());
    if (::connect(handle, reinterpret_cast<const sockaddr*>(&address), sizeof(address)) != 0)
    {
        closesocket(handle);
        return INVALID_SOCKET;
    }
    return handle;
}

// send()/recv() take an int length on Winsock and a size_t on POSIX.
int SendAll(SOCKET handle, std::string_view payload)
{
    return ::send(handle, payload.data(), static_cast<int>(payload.size()), 0);
}

// The listener is non-blocking, so a connection may not be queued yet when
// Accept() is first called; poll briefly instead of sleeping a fixed time.
std::unique_ptr<Core::Platform::LocalSocketConnection> AcceptWithin(Core::Platform::LocalSocketListener& listener,
                                                                    std::chrono::milliseconds timeout)
{
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    while (std::chrono::steady_clock::now() < deadline)
    {
        if (auto connection = listener.Accept())
        {
            return connection;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    return nullptr;
}

// Pushes a payload in through chunks, buffering each one on the connection
// without draining lines: how a pipelining script and the frame loop that
// only serves a few requests per frame interleave.
bool BufferInto(SOCKET client, Core::Platform::LocalSocketConnection& connection, std::string_view payload)
{
    constexpr std::size_t ChunkBytes = 16 * 1024;
    std::size_t offset = 0;
    while (offset < payload.size())
    {
        const std::size_t size = std::min(ChunkBytes, payload.size() - offset);
        const int sent = SendAll(client, payload.substr(offset, size));
        if (sent <= 0)
        {
            return false;
        }
        offset += static_cast<std::size_t>(sent);
        if (!connection.ReadAvailable())
        {
            return false;
        }
    }
    return true;
}

bool ReadLineWithin(Core::Platform::LocalSocketConnection& connection, std::string& line,
                    std::chrono::milliseconds timeout)
{
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    while (std::chrono::steady_clock::now() < deadline)
    {
        const bool open = connection.ReadAvailable();
        if (connection.TakeLine(line))
        {
            return true;
        }
        if (!open)
        {
            // The peer is gone: waiting out the timeout would report "no
            // line yet" for a connection that can never produce one.
            return false;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    return false;
}
} // namespace

TEST_CASE("Local socket serves a line round-trip [core][local-socket]")
{
    const auto directory = MakeSocketDirectory();
    const std::string path = (directory / "round-trip.sock").string();

    Core::Platform::LocalSocketListener listener;
    std::string error;
    REQUIRE(listener.Listen(path, error));
    REQUIRE(error.empty());
    REQUIRE(listener.IsListening());
    REQUIRE(SocketFileExists(path));

    const SOCKET client = ConnectTo(path);
    REQUIRE(client != INVALID_SOCKET);

    auto connection = AcceptWithin(listener, std::chrono::milliseconds(500));
    REQUIRE(connection != nullptr);
    REQUIRE(connection->IsOpen());

    const std::string request = R"({"cmd":"ping"})"
                                "\n";
    REQUIRE(SendAll(client, request) == static_cast<int>(request.size()));

    std::string line;
    REQUIRE(ReadLineWithin(*connection, line, std::chrono::milliseconds(500)));
    CHECK(line == R"({"cmd":"ping"})");

    REQUIRE(connection->Write(R"({"ok":true})"
                              "\n"));

    // A stream, not a message queue: read until the terminator rather than
    // assuming one recv() carries the whole line.
    std::string received;
    while (received.find('\n') == std::string::npos)
    {
        char chunk[64] = {};
        const int count = ::recv(client, chunk, static_cast<int>(sizeof(chunk)), 0);
        REQUIRE(count > 0);
        received.append(chunk, static_cast<std::size_t>(count));
    }
    CHECK(received == R"({"ok":true})"
                      "\n");

    closesocket(client);
    listener.Close();
    std::filesystem::remove_all(directory);
}

TEST_CASE("Local socket splits and preserves partial lines [core][local-socket]")
{
    const auto directory = MakeSocketDirectory();
    const std::string path = (directory / "framing.sock").string();

    Core::Platform::LocalSocketListener listener;
    std::string error;
    REQUIRE(listener.Listen(path, error));

    const SOCKET client = ConnectTo(path);
    REQUIRE(client != INVALID_SOCKET);
    auto connection = AcceptWithin(listener, std::chrono::milliseconds(500));
    REQUIRE(connection != nullptr);

    // Two complete lines (one with a CRLF terminator) plus an unterminated tail.
    const std::string payload = "first\r\nsecond\nthi";
    REQUIRE(SendAll(client, payload) == static_cast<int>(payload.size()));

    std::string line;
    REQUIRE(ReadLineWithin(*connection, line, std::chrono::milliseconds(500)));
    CHECK(line == "first");
    // The payload is one send(), but a stream may still deliver it in
    // pieces: wait for the second line rather than assume it is already
    // buffered, and only then assert that the tail is not a line yet.
    REQUIRE(ReadLineWithin(*connection, line, std::chrono::milliseconds(500)));
    CHECK(line == "second");
    CHECK_FALSE(connection->TakeLine(line));

    const std::string rest = "rd\n";
    REQUIRE(SendAll(client, rest) == static_cast<int>(rest.size()));
    REQUIRE(ReadLineWithin(*connection, line, std::chrono::milliseconds(500)));
    CHECK(line == "third");

    closesocket(client);
    listener.Close();
    std::filesystem::remove_all(directory);
}

TEST_CASE("Local socket file is owner-only and replaces a stale file [core][local-socket]")
{
    const auto directory = MakeSocketDirectory();
    const std::string path = (directory / "stale.sock").string();

    // A socket file left behind by a crashed client: bound by a socket that
    // then went away without unlinking it, which is what a crash leaves.
    {
        const SOCKET stale = ::socket(AF_UNIX, SOCK_STREAM, 0);
        REQUIRE(stale != INVALID_SOCKET);
        sockaddr_un address{};
        address.sun_family = AF_UNIX;
        REQUIRE(path.size() < sizeof(address.sun_path));
        std::memcpy(address.sun_path, path.c_str(), path.size());
        REQUIRE(::bind(stale, reinterpret_cast<const sockaddr*>(&address), sizeof(address)) == 0);
        closesocket(stale);
    }
    REQUIRE(SocketFileExists(path));

    Core::Platform::LocalSocketListener listener;
    std::string error;
    REQUIRE(listener.Listen(path, error));
    REQUIRE(error.empty());

#ifndef _WIN32
    struct stat status = {};
    REQUIRE(::stat(path.c_str(), &status) == 0);
    CHECK((status.st_mode & 07777) == 0600);
    CHECK(S_ISSOCK(status.st_mode));
#endif

    // The replaced file still serves connections.
    const SOCKET client = ConnectTo(path);
    REQUIRE(client != INVALID_SOCKET);
    CHECK(AcceptWithin(listener, std::chrono::milliseconds(500)) != nullptr);
    closesocket(client);

    listener.Close();
    CHECK_FALSE(SocketFileExists(path));

    std::filesystem::remove_all(directory);
}

TEST_CASE("Local socket leaves a path that is not a socket alone [core][local-socket]")
{
    const auto directory = MakeSocketDirectory();
    const std::string path = (directory / "notasocket").string();

    {
        FILE* file = std::fopen(path.c_str(), "wb");
        REQUIRE(file != nullptr);
        std::fputs("not a socket", file);
        std::fclose(file);
    }

    Core::Platform::LocalSocketListener listener;
    std::string error;
    CHECK_FALSE(listener.Listen(path, error));
    CHECK(error.find("not a socket") != std::string::npos);
    // The file is still there: a mistyped path costs a refusal, not the file.
    CHECK(SocketFileExists(path));

    std::filesystem::remove_all(directory);
}

TEST_CASE("Local socket rejects an impossible path and unlinks on demand [core][local-socket]")
{
    const auto directory = MakeSocketDirectory();

    Core::Platform::LocalSocketListener listener;
    std::string error;

    const std::string tooLong =
        (directory / std::string(Core::Platform::LocalSocketListener::MaxPathLength(), 'x')).string();
    CHECK_FALSE(listener.Listen(tooLong, error));
    CHECK_FALSE(error.empty());
    CHECK_FALSE(listener.IsListening());

    error.clear();
    const std::string missingDirectory = (directory / "no-such-directory" / "x.sock").string();
    CHECK_FALSE(listener.Listen(missingDirectory, error));
    CHECK_FALSE(error.empty());
    CHECK_FALSE(SocketFileExists(missingDirectory));

    // Unlink removes the file of a listener that is no longer running.
    const std::string path = (directory / "unlink.sock").string();
    REQUIRE(listener.Listen(path, error));
    REQUIRE(SocketFileExists(path));
    listener.Close();
    Core::Platform::LocalSocketListener::Unlink(path);
    CHECK_FALSE(SocketFileExists(path));
    std::filesystem::remove_all(directory);
}

TEST_CASE("Local socket refuses a path another listener is serving [core][local-socket]")
{
    const auto directory = MakeSocketDirectory();
    const std::string path = (directory / "taken.sock").string();

    Core::Platform::LocalSocketListener first;
    std::string error;
    REQUIRE(first.Listen(path, error));

    // A second client of the same name must not unlink the live socket and
    // bind its own: the first would keep an open socket nobody can reach.
    Core::Platform::LocalSocketListener second;
    CHECK_FALSE(second.Listen(path, error));
    CHECK(error.find("already listening") != std::string::npos);
    CHECK_FALSE(second.IsListening());

    // The first listener still serves — and serves *this* connection, not
    // the refused listener's probe: the accepted peer has to answer with
    // the line this test sends, which a probe never would.
    CHECK(first.IsListening());
    const SOCKET client = ConnectTo(path);
    REQUIRE(client != INVALID_SOCKET);
    const std::string probe = "{\"cmd\":\"ping\"}\n";
    REQUIRE(SendAll(client, probe) == static_cast<int>(probe.size()));

    std::string line;
    std::unique_ptr<Core::Platform::LocalSocketConnection> served;
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(500);
    while (std::chrono::steady_clock::now() < deadline)
    {
        auto accepted = AcceptWithin(first, std::chrono::milliseconds(100));
        if (accepted != nullptr && ReadLineWithin(*accepted, line, std::chrono::milliseconds(200)))
        {
            served = std::move(accepted);
            break;
        }
    }
    REQUIRE(served != nullptr);
    CHECK(line == "{\"cmd\":\"ping\"}");
    closesocket(client);

    // Once it is gone the path is free again, stale file and all.
    first.Close();
    Core::Platform::LocalSocketListener third;
    CHECK(third.Listen(path, error));
    third.Close();
    std::filesystem::remove_all(directory);
}

TEST_CASE("Local socket reports a closed peer [core][local-socket]")
{
    const auto directory = MakeSocketDirectory();
    const std::string path = (directory / "closed.sock").string();

    Core::Platform::LocalSocketListener listener;
    std::string error;
    REQUIRE(listener.Listen(path, error));

    const SOCKET client = ConnectTo(path);
    REQUIRE(client != INVALID_SOCKET);
    auto connection = AcceptWithin(listener, std::chrono::milliseconds(500));
    REQUIRE(connection != nullptr);

    // Nothing to read yet, but the connection is healthy.
    CHECK(connection->ReadAvailable());
    CHECK(connection->IsOpen());

    closesocket(client);

    // The shutdown is reported, not acted on: whoever owns the connection
    // decides when to let go, because an act the peer started may still owe
    // it an answer.
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(500);
    while (!connection->PeerClosed() && std::chrono::steady_clock::now() < deadline)
    {
        connection->ReadAvailable();
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    CHECK(connection->PeerClosed());
    CHECK_FALSE(connection->HasLine());
    CHECK(connection->IsOpen());

    connection->Close();
    CHECK_FALSE(connection->IsOpen());

    listener.Close();
    std::filesystem::remove_all(directory);
}

TEST_CASE("Local socket bounds the unterminated tail, not a pipelined batch [core][local-socket]")
{
    const auto directory = MakeSocketDirectory();
    const std::string path = (directory / "cap.sock").string();

    Core::Platform::LocalSocketListener listener;
    std::string error;
    REQUIRE(listener.Listen(path, error));

    const SOCKET client = ConnectTo(path);
    REQUIRE(client != INVALID_SOCKET);
    auto connection = AcceptWithin(listener, std::chrono::milliseconds(500));
    REQUIRE(connection != nullptr);

    // More complete lines than the cap holds in bytes, queued before any of
    // them is served. A batch of valid commands is not abuse: the connection
    // stays open and every line survives in order.
    const std::string command = "{\"cmd\":\"ping\"}\n";
    const std::size_t lines = (Core::Platform::LocalSocketConnection::MaxPendingInputBytes / command.size()) + 64;
    std::string batch;
    batch.reserve(command.size() * lines);
    for (std::size_t index = 0; index < lines; ++index)
    {
        batch += command;
    }
    REQUIRE(batch.size() > Core::Platform::LocalSocketConnection::MaxPendingInputBytes);

    // Served as it arrives, the way the frame loop does: reading pauses
    // once a frame's worth of complete lines is waiting, so the batch is
    // taken in several passes rather than all at once.
    constexpr std::size_t ChunkBytes = 16 * 1024;
    std::size_t offset = 0;
    std::size_t taken = 0;
    std::string line;
    while (offset < batch.size())
    {
        const std::size_t size = std::min(ChunkBytes, batch.size() - offset);
        const int sent = SendAll(client, batch.substr(offset, size));
        REQUIRE(sent > 0);
        offset += static_cast<std::size_t>(sent);
        REQUIRE(connection->ReadAvailable());
        while (connection->TakeLine(line))
        {
            ++taken;
        }
    }
    CHECK(connection->IsOpen());
    CHECK(taken == lines);

    // A peer that never terminates its line is still cut off.
    // Comfortably past the cap: crossing it on the last byte of the payload
    // would depend on that byte having arrived before ReadAvailable() runs.
    const std::string blob(Core::Platform::LocalSocketConnection::MaxPendingInputBytes + (64 * 1024), 'x');
    CHECK_FALSE(BufferInto(client, *connection, blob));
    CHECK_FALSE(connection->IsOpen());

    closesocket(client);
    listener.Close();
    std::filesystem::remove_all(directory);
}

TEST_CASE("Local socket paces a peer that outruns the drain rate [core][local-socket]")
{
    const auto directory = MakeSocketDirectory();
    const std::string path = (directory / "flood.sock").string();

    Core::Platform::LocalSocketListener listener;
    std::string error;
    REQUIRE(listener.Listen(path, error));

    const SOCKET client = ConnectTo(path);
    REQUIRE(client != INVALID_SOCKET);
    auto connection = AcceptWithin(listener, std::chrono::milliseconds(500));
    REQUIRE(connection != nullptr);

    // Valid commands arriving faster than they are served are not abuse:
    // reading pauses once more than a frame's worth is waiting, so the
    // buffer stays bounded and the rest of the batch waits in the kernel
    // instead of the connection being dropped for it.
    const std::string command = "{\"cmd\":\"ping\"}\n";
    std::string batch;
    while (batch.size() <= Core::Platform::LocalSocketConnection::ReadPauseBytes * 2)
    {
        batch += command;
    }

    // Written in chunks with the reader and a consumer interleaved, the way
    // the frame loop does it: the writer is never blocked out and nothing
    // is dropped, while the buffer stays at the pause mark.
    constexpr std::size_t ChunkBytes = 16 * 1024;
    std::size_t offset = 0;
    std::size_t taken = 0;
    std::string line;
    while (offset < batch.size())
    {
        const std::size_t size = std::min(ChunkBytes, batch.size() - offset);
        const int sent = SendAll(client, batch.substr(offset, size));
        REQUIRE(sent > 0);
        offset += static_cast<std::size_t>(sent);

        REQUIRE(connection->ReadAvailable());
        REQUIRE(connection->IsOpen());
        while (connection->TakeLine(line))
        {
            ++taken;
        }
    }

    // Everything sent arrived, in order, and the connection is still open:
    // a fast writer is paced rather than dropped.
    CHECK(taken == batch.size() / command.size());
    CHECK(connection->IsOpen());

    // Draining makes room, and the connection is still there to read more.
    REQUIRE(connection->ReadAvailable());
    CHECK(connection->IsOpen());

    closesocket(client);
    listener.Close();
    std::filesystem::remove_all(directory);
}
