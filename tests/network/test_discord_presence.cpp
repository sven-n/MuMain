// doctest unit tests for the Discord rich presence (docs/discord.md): the
// IPC framing, the RPC payloads, the presence text, and - on Linux and macOS,
// against a stand-in for the Discord app - the worker that ties them together.
//
// Run: ctest --test-dir <build directory> --build-config Release -R "Discord"

#include "doctest.h"

#include "Integration/Discord/Invite.h"
#include "Integration/Discord/IpcFrame.h"
#include "Integration/Discord/PresenceClient.h"
#include "Integration/Discord/PresenceMode.h"
#include "Integration/Discord/PresenceText.h"
#include "Integration/Discord/RpcCommands.h"
#include "Integration/Discord/WideFormat.h"

#include "json.hpp"

#include <array>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <string>
#include <thread>

#ifndef _WIN32
#include <cstdlib>
#include <poll.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#endif

using namespace Integration::Discord;
using nlohmann::json;

namespace
{
std::array<std::uint8_t, Ipc::HeaderBytes> HeaderOf(const std::string& frame)
{
    std::array<std::uint8_t, Ipc::HeaderBytes> header{};
    std::memcpy(header.data(), frame.data(), header.size());
    return header;
}

PresenceSnapshot WorldSnapshot()
{
    PresenceSnapshot snapshot;
    snapshot.scene = PresenceSnapshot::Scene::World;
    snapshot.className = L"Blade Knight";
    snapshot.level = 380;
    snapshot.masterLevel = 120;
    snapshot.location = L"Devias";
    snapshot.partyMembers = 3;
    snapshot.partyCapacity = 5;
    return snapshot;
}
} // namespace

TEST_CASE("Discord IPC frames carry opcode and length little-endian [discord]")
{
    const std::string frame = Ipc::EncodeFrame(Ipc::Opcode::Frame, "{}");

    REQUIRE(frame.size() == Ipc::HeaderBytes + 2);
    CHECK(frame.substr(0, Ipc::HeaderBytes) == std::string("\x01\x00\x00\x00\x02\x00\x00\x00", Ipc::HeaderBytes));
    CHECK(frame.substr(Ipc::HeaderBytes) == "{}");

    const auto header = Ipc::DecodeHeader(HeaderOf(frame));
    REQUIRE(header.has_value());
    CHECK(header->opcode == Ipc::Opcode::Frame);
    CHECK(header->length == 2);
}

TEST_CASE("Discord IPC refuses unknown opcodes and oversized frames [discord]")
{
    std::string unknownOpcode = Ipc::EncodeFrame(Ipc::Opcode::Frame, "");
    unknownOpcode[0] = 9;
    CHECK_FALSE(Ipc::DecodeHeader(HeaderOf(unknownOpcode)).has_value());

    const std::string oversized = Ipc::EncodeFrame(Ipc::Opcode::Frame, std::string(Ipc::MaxPayloadBytes + 1, ' '));
    CHECK_FALSE(Ipc::DecodeHeader(HeaderOf(oversized)).has_value());
}

TEST_CASE("Discord handshake names the application [discord]")
{
    const json handshake = json::parse(Rpc::Handshake("1234567890"));
    CHECK(handshake["v"] == 1);
    CHECK(handshake["client_id"] == "1234567890");
}

TEST_CASE("Discord SET_ACTIVITY carries text, timer and images [discord]")
{
    Activity activity;
    activity.details = "Lv 380 / ML 120 Blade Knight";
    activity.state = "Devias";
    activity.largeImageKey = "logo";
    activity.largeImageText = "Devias";
    activity.startTimestamp = 1700000000;

    const json command = json::parse(Rpc::SetActivity(activity, 42, 7));
    CHECK(command["cmd"] == "SET_ACTIVITY");
    CHECK(command["nonce"] == "7");
    CHECK(command["args"]["pid"] == 42);

    const json& described = command["args"]["activity"];
    CHECK(described["details"] == activity.details);
    CHECK(described["state"] == activity.state);
    CHECK(described["timestamps"]["start"] == activity.startTimestamp);
    CHECK(described["assets"]["large_image"] == "logo");
    CHECK(described["assets"]["large_text"] == "Devias");
    // No small image key configured: no small image at all.
    CHECK_FALSE(described["assets"].contains("small_image"));
    CHECK_FALSE(described["assets"].contains("small_text"));
}

TEST_CASE("Discord SET_ACTIVITY survives text that is not UTF-8 [discord]")
{
    Activity activity;
    activity.details = "Lv 1 \xFF\xFE";

    std::string payload;
    CHECK_NOTHROW(payload = Rpc::SetActivity(activity, 42, 1));
    CHECK(json::accept(payload));
}

TEST_CASE("Discord SET_ACTIVITY without activity clears the presence [discord]")
{
    const json command = json::parse(Rpc::SetActivity(std::nullopt, 42, 1));
    CHECK_FALSE(command["args"].contains("activity"));
}

TEST_CASE("Discord text is cut to its limit on a character boundary [discord]")
{
    // 127 ASCII bytes and a two-byte character straddling the limit.
    const std::string text = std::string(Rpc::MaxTextLength - 1, 'a') + "\xC3\xA9" + "tail";
    const std::string clamped = Rpc::ClampText(text);
    CHECK(clamped == std::string(Rpc::MaxTextLength - 1, 'a'));

    CHECK(Rpc::ClampText("short") == "short");
}

TEST_CASE("Discord READY and ERROR events are recognised [discord]")
{
    CHECK(Rpc::IsReadyEvent(R"({"cmd":"DISPATCH","evt":"READY","data":{}})"));
    CHECK_FALSE(Rpc::IsReadyEvent(R"({"cmd":"DISPATCH","evt":"ERROR"})"));
    CHECK_FALSE(Rpc::IsReadyEvent("not json"));

    CHECK(Rpc::ErrorMessage(R"({"evt":"ERROR","data":{"code":4000,"message":"Invalid Client ID"}})") ==
          "Invalid Client ID");
    // Discord's ordinary answers carry a null event.
    CHECK(Rpc::ErrorMessage(R"({"evt":null,"cmd":"SET_ACTIVITY"})").empty());
    CHECK_FALSE(Rpc::IsReadyEvent(R"({"evt":null,"cmd":"SET_ACTIVITY"})"));
    CHECK(Rpc::ErrorMessage(R"({"evt":"ERROR","data":null})") == "unknown error");
}

TEST_CASE("Discord presence mode round-trips through its config value [discord]")
{
    for (const PresenceMode mode : PresenceModes)
    {
        CHECK(ParsePresenceMode(PresenceModeName(mode)) == mode);
    }
    CHECK(ParsePresenceMode(L"typo") == DefaultPresenceMode);
}

TEST_CASE("Discord presence hides the character unless the player asks for it [discord]")
{
    CHECK(DefaultPresenceMode == PresenceMode::HideDetails);
}

TEST_CASE("Discord presence formats translated texts of any length [discord]")
{
    const std::wstring longName(300, L'x');
    const std::wstring text = FormatWide(L"In %ls", longName.c_str());
    CHECK(text == L"In " + longName);
    CHECK(FormatWide(L"Party %d/%d", 3, 5) == L"Party 3/5");
}

TEST_CASE("Discord presence shows character, location and party [discord]")
{
    const Activity activity = DescribePresence(WorldSnapshot(), PresenceMode::On, {"logo", "class"});

    CHECK(activity.details == "Lv 380 / ML 120 Blade Knight");
    CHECK(activity.state == "Devias \xC2\xB7 Party 3/5");
    CHECK(activity.largeImageKey == "logo");
    CHECK(activity.largeImageText == "Devias");
    CHECK(activity.smallImageKey == "class");
    CHECK(activity.smallImageText == "Blade Knight");
}

TEST_CASE("Discord presence leaves out the master level of a character without one [discord]")
{
    PresenceSnapshot snapshot = WorldSnapshot();
    snapshot.masterLevel = 0;
    snapshot.partyMembers = 0;

    const Activity activity = DescribePresence(snapshot, PresenceMode::On, {});
    CHECK(activity.details == "Lv 380 Blade Knight");
    CHECK(activity.state == "Devias");
}

TEST_CASE("Discord presence names the event the character is in [discord]")
{
    PresenceSnapshot snapshot = WorldSnapshot();
    snapshot.location = L"Blood Castle 5";
    snapshot.inEvent = true;

    const Activity activity = DescribePresence(snapshot, PresenceMode::On, {});
    CHECK(activity.state == "In Blood Castle 5 \xC2\xB7 Party 3/5");
}

TEST_CASE("Discord presence hides the character on request [discord]")
{
    const Activity activity = DescribePresence(WorldSnapshot(), PresenceMode::HideDetails, {"logo", "class"});

    CHECK(activity.details == "In game");
    CHECK(activity.details.find("Blade Knight") == std::string::npos);
    CHECK(activity.smallImageText.empty());
    CHECK(activity.state == "Devias \xC2\xB7 Party 3/5");
}

TEST_CASE("Discord presence outside the world names the screen [discord]")
{
    PresenceSnapshot snapshot;
    snapshot.scene = PresenceSnapshot::Scene::CharacterSelect;

    const Activity activity = DescribePresence(snapshot, PresenceMode::On, {});
    CHECK(activity.details == "Selecting a character");
    CHECK(activity.state.empty());
}

TEST_CASE("Discord invite links are recognised [discord]")
{
    CHECK(Invite::IsInviteUrl(L"https://discord.gg/abcDEF123"));
    CHECK(Invite::IsInviteUrl(L"https://discord.com/invite/my-server"));
    CHECK(Invite::IsInviteUrl(L"https://discordapp.com/invite/abc123"));
}

TEST_CASE("Discord invite links may carry a query or fragment [discord]")
{
    CHECK(Invite::IsInviteUrl(L"https://discord.gg/abc123?event=1234567890"));
    CHECK(Invite::IsInviteUrl(L"https://discord.gg/abc123?utm_source=web&utm_medium=copy"));
    CHECK(Invite::IsInviteUrl(L"https://discord.com/invite/abc123#top"));

    // The code still has to be one, and nothing may break out of the URL.
    CHECK_FALSE(Invite::IsInviteUrl(L"https://discord.gg/?event=1"));
    CHECK_FALSE(Invite::IsInviteUrl(L"https://discord.gg/abc123?x=\"a b\""));
    CHECK_FALSE(Invite::IsInviteUrl(L"https://discord.gg/abc123?x=" + std::wstring(300, L'a')));
}

TEST_CASE("An invite link which isn't one is not accepted [discord]")
{
    CHECK(Invite::Accept(L"https://discord.gg/abc123"));
    CHECK_FALSE(Invite::Accept(L"https://discord.gg/abc 123"));
    CHECK_FALSE(Invite::Accept(L""));
}

TEST_CASE("Anything but a Discord invite link is refused [discord]")
{
    CHECK_FALSE(Invite::IsInviteUrl(L""));
    CHECK_FALSE(Invite::IsInviteUrl(L"discord.gg/abc123"));
    CHECK_FALSE(Invite::IsInviteUrl(L"http://discord.gg/abc123"));
    CHECK_FALSE(Invite::IsInviteUrl(L"https://discord.gg/"));
    CHECK_FALSE(Invite::IsInviteUrl(L"https://discord.gg.example.com/abc123"));
    CHECK_FALSE(Invite::IsInviteUrl(L"https://example.com/?https://discord.gg/abc123"));
    CHECK_FALSE(Invite::IsInviteUrl(L"https://discord.gg/abc123/../../evil"));
    CHECK_FALSE(Invite::IsInviteUrl(L"https://discord.com/channels/123"));
    CHECK_FALSE(Invite::IsInviteUrl(L"file:///etc/passwd"));
    CHECK_FALSE(Invite::IsInviteUrl(L"https://discord.gg/" + std::wstring(40, L'a')));
}

TEST_CASE("Opening something that is not an invite does nothing [discord]")
{
    CHECK_FALSE(Invite::Open(L"https://example.com/"));
}

#ifndef _WIN32
namespace
{
// Plays the Discord app's side of one connection: accepts the client,
// answers its handshake with READY and records the first activity it sends.
class FakeDiscord
{
public:
    explicit FakeDiscord(const std::filesystem::path& directory) : m_path((directory / "discord-ipc-0").string())
    {
        m_listener = ::socket(AF_UNIX, SOCK_STREAM, 0);
        sockaddr_un address{};
        address.sun_family = AF_UNIX;
        std::strncpy(address.sun_path, m_path.c_str(), sizeof(address.sun_path) - 1);
        REQUIRE(::bind(m_listener, reinterpret_cast<const sockaddr*>(&address), sizeof(address)) == 0);
        REQUIRE(::listen(m_listener, 1) == 0);
    }

    ~FakeDiscord()
    {
        if (m_peer >= 0)
        {
            ::close(m_peer);
        }
        ::close(m_listener);
        ::unlink(m_path.c_str());
    }

    bool Accept(std::chrono::milliseconds timeout)
    {
        pollfd watched{m_listener, POLLIN, 0};
        if (::poll(&watched, 1, static_cast<int>(timeout.count())) <= 0)
        {
            return false;
        }
        m_peer = ::accept(m_listener, nullptr, nullptr);
        return m_peer >= 0;
    }

    bool ReadFrame(Ipc::Opcode& opcode, std::string& payload)
    {
        std::array<std::uint8_t, Ipc::HeaderBytes> header{};
        if (!ReadExact(reinterpret_cast<char*>(header.data()), header.size()))
        {
            return false;
        }
        const auto decoded = Ipc::DecodeHeader(header);
        if (!decoded.has_value())
        {
            return false;
        }
        opcode = decoded->opcode;
        payload.assign(decoded->length, '\0');
        return ReadExact(payload.data(), payload.size());
    }

    void Send(Ipc::Opcode opcode, const std::string& payload)
    {
        const std::string frame = Ipc::EncodeFrame(opcode, payload);
        REQUIRE(::send(m_peer, frame.data(), frame.size(), 0) == static_cast<ssize_t>(frame.size()));
    }

private:
    bool ReadExact(char* buffer, std::size_t size)
    {
        constexpr int TimeoutMilliseconds = 10000;
        std::size_t received = 0;
        while (received < size)
        {
            pollfd watched{m_peer, POLLIN, 0};
            if (::poll(&watched, 1, TimeoutMilliseconds) <= 0)
            {
                return false;
            }
            const ssize_t count = ::recv(m_peer, buffer + received, size - received, 0);
            if (count <= 0)
            {
                return false;
            }
            received += static_cast<std::size_t>(count);
        }
        return true;
    }

    std::string m_path;
    int m_listener = -1;
    int m_peer = -1;
};

// Points the client's endpoint search at a directory of the test's own.
class ScopedRuntimeDirectory
{
public:
    ScopedRuntimeDirectory()
        : m_directory(std::filesystem::temp_directory_path() / ("mu-discord-test-" + std::to_string(::getpid())))
    {
        std::filesystem::create_directories(m_directory);
        const char* previous = std::getenv("XDG_RUNTIME_DIR");
        m_previous = previous != nullptr ? previous : "";
        m_hadPrevious = previous != nullptr;
        ::setenv("XDG_RUNTIME_DIR", m_directory.c_str(), 1);
    }

    ~ScopedRuntimeDirectory()
    {
        if (m_hadPrevious)
        {
            ::setenv("XDG_RUNTIME_DIR", m_previous.c_str(), 1);
        }
        else
        {
            ::unsetenv("XDG_RUNTIME_DIR");
        }
        std::filesystem::remove_all(m_directory);
    }

    [[nodiscard]] const std::filesystem::path& Path() const
    {
        return m_directory;
    }

private:
    std::filesystem::path m_directory;
    std::string m_previous;
    bool m_hadPrevious = false;
};
} // namespace

TEST_CASE("Discord presence client handshakes and publishes the activity [discord]")
{
    const ScopedRuntimeDirectory runtimeDirectory;
    FakeDiscord discord(runtimeDirectory.Path());

    PresenceClient client("1234567890");
    Activity activity;
    activity.details = "Lv 380 Blade Knight";
    activity.state = "Devias";
    client.Publish(activity);

    REQUIRE(discord.Accept(std::chrono::seconds(5)));

    Ipc::Opcode opcode{};
    std::string payload;
    REQUIRE(discord.ReadFrame(opcode, payload));
    CHECK(opcode == Ipc::Opcode::Handshake);
    CHECK(json::parse(payload)["client_id"] == "1234567890");

    discord.Send(Ipc::Opcode::Frame, R"({"cmd":"DISPATCH","evt":"READY","data":{"v":1}})");

    REQUIRE(discord.ReadFrame(opcode, payload));
    CHECK(opcode == Ipc::Opcode::Frame);
    const json command = json::parse(payload);
    CHECK(command["cmd"] == "SET_ACTIVITY");
    CHECK(command["args"]["pid"] == static_cast<std::uint32_t>(::getpid()));
    CHECK(command["args"]["activity"]["details"] == "Lv 380 Blade Knight");
    CHECK(command["args"]["activity"]["state"] == "Devias");
}
TEST_CASE("Discord presence client without Discord running stops at once [discord]")
{
    const ScopedRuntimeDirectory runtimeDirectory;
    const auto started = std::chrono::steady_clock::now();
    {
        PresenceClient client("1234567890");
        client.Publish(Activity{});
        // Let the worker try to connect and settle into its reconnect wait.
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    CHECK(std::chrono::steady_clock::now() - started < PresenceClient::ReconnectDelay);
}
#endif
