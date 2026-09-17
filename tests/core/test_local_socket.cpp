// doctest unit tests for the local stream socket transport, on Windows and
// Linux alike.
//
// The listener is exercised against a real socket file in a temporary
// directory; the "client" is a plain blocking socket created by the test, so
// nothing here needs a window, a renderer or the game's globals.
//
// Run: ctest --test-dir <build directory> --build-config Release -R local_socket

#include "doctest.h"

#include "Core/Platform/LocalSocket.h"

#include "Core/Platform/WinSock.h" // SOCKET, closesocket, WSAStartup (no-ops on POSIX)

#include <chrono>
#include <cstring>
#include <filesystem>
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

bool ReadLineWithin(Core::Platform::LocalSocketConnection& connection, std::string& line,
                    std::chrono::milliseconds timeout)
{
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    while (std::chrono::steady_clock::now() < deadline)
    {
        connection.ReadAvailable();
        if (connection.TakeLine(line))
        {
            return true;
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

    char received[64] = {};
    const int count = ::recv(client, received, static_cast<int>(sizeof(received) - 1), 0);
    REQUIRE(count > 0);
    CHECK(std::string(received, static_cast<std::size_t>(count)) == R"({"ok":true})"
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
    REQUIRE(connection->TakeLine(line));
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

    // A file left behind by a crashed client.
    {
        std::filesystem::permissions(directory, std::filesystem::perms::owner_all);
        FILE* stale = std::fopen(path.c_str(), "wb");
        REQUIRE(stale != nullptr);
        std::fclose(stale);
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
    Core::Platform::LocalSocketListener::Unlink(path);
    CHECK_FALSE(SocketFileExists(path));

    listener.Close();
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

    const auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(500);
    while (connection->IsOpen() && std::chrono::steady_clock::now() < deadline)
    {
        connection->ReadAvailable();
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }
    CHECK_FALSE(connection->IsOpen());

    listener.Close();
    std::filesystem::remove_all(directory);
}
