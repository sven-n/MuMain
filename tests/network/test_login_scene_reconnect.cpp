// doctest unit tests for the login screen's quiet reconnect.
//
// LoginSceneReconnect only decides; a fake link stands in for the connection,
// the background check, the screens and the clock, so each case walks the
// state machine through what a server drop on the login screen looks like.
//
// Run: ctest --test-dir <build directory> --build-config Release -R "Login scene reconnect"

#include "stdafx.h"

#include "doctest.h"

#include "Network/Reconnect/LoginSceneReconnect.h"

#include <optional>
#include <vector>

namespace
{
constexpr double GreetingTimeoutMs = 10000.0;   // LoginSceneReconnect.cpp
constexpr double ServerListRefreshMs = 30000.0; // LoginSceneReconnect.cpp
constexpr int ListRequestsPerConnection = 20;   // LoginSceneReconnect.cpp

class FakeLink : public LoginSceneReconnectLink
{
public:
    bool connected = false;
    LoginServer connectedServer = LoginServer::Connect;
    int connectionId = 1;
    std::optional<LoginServer> awaited;
    bool serverListShown = false;
    bool probeCanStart = true;
    ProbeResult probeResult = ProbeResult::Pending;
    bool connectSucceeds = true;
    bool failureShown = false;
    double now = 0.0;

    std::vector<LoginServer> probes;
    std::vector<LoginServer> connects;
    int probeStops = 0;
    int listRequests = 0;
    int connectingShown = 0;
    int failures = 0;

    bool IsConnected() const override
    {
        return connected;
    }
    LoginServer ConnectedServer() const override
    {
        return connectedServer;
    }
    int ConnectionId() const override
    {
        return connectionId;
    }
    std::optional<LoginServer> AwaitedServer() const override
    {
        return awaited;
    }
    bool IsServerListShown() const override
    {
        return serverListShown;
    }

    bool StartProbe(LoginServer server) override
    {
        probes.push_back(server);
        return probeCanStart;
    }

    ProbeResult PollProbe() override
    {
        return probeResult;
    }
    void StopProbe() override
    {
        ++probeStops;
    }

    bool Connect(LoginServer server) override
    {
        connects.push_back(server);
        connected = connectSucceeds;
        connectedServer = server;
        ++connectionId;
        return connectSucceeds;
    }

    void SendServerListRequest() override
    {
        ++listRequests;
    }
    void ShowConnecting() override
    {
        ++connectingShown;
    }
    void ShowFailure() override
    {
        ++failures;
        failureShown = true;
    }
    bool IsFailureShown() const override
    {
        return failureShown;
    }
    void HideFailure() override
    {
        failureShown = false;
    }
    double NowMs() const override
    {
        return now;
    }
};

// A login screen whose connection the server closed while the player was idle.
struct DroppedScreen
{
    FakeLink link;
    LoginSceneReconnect reconnect{link};
    int resumed = 0;

    std::function<void()> CountResume()
    {
        return [this] { ++resumed; };
    }

    // The server accepts connections again and the client connects.
    void ServerComesBack()
    {
        link.probeResult = LoginSceneReconnectLink::ProbeResult::Reachable;
        reconnect.Update();
    }

    // The next refresh of an open server list is due.
    void RefreshDue()
    {
        link.now += ServerListRefreshMs;
        reconnect.Update();
    }

    // An open server list whose connection has used up the lists the connect server sends
    // over one connection: the first came with the connection, the refreshes asked for the rest.
    void UseUpListRequests()
    {
        link.connected = true;
        link.serverListShown = true;
        reconnect.OnGreeting(LoginServer::Connect);
        for (int refresh = 1; refresh < ListRequestsPerConnection; ++refresh)
        {
            RefreshDue();
        }
    }
};
} // namespace

TEST_SUITE("Login scene reconnect")
{
    TEST_CASE("a live connection sends right away")
    {
        DroppedScreen screen;
        screen.link.connected = true;

        CHECK(screen.reconnect.EnsureConnected(LoginServer::Connect, screen.CountResume()));
        CHECK(screen.link.probes.empty());
        CHECK(screen.resumed == 0);
    }

    TEST_CASE("a server pick on a dropped list reconnects and picks again")
    {
        DroppedScreen screen;

        CHECK_FALSE(screen.reconnect.EnsureConnected(LoginServer::Connect, screen.CountResume()));
        CHECK(screen.link.probes == std::vector{LoginServer::Connect});
        CHECK(screen.reconnect.IsReconnecting());

        screen.reconnect.Update(); // the server hasn't answered the check yet
        CHECK(screen.link.connects.empty());

        screen.ServerComesBack();
        CHECK(screen.link.connects == std::vector{LoginServer::Connect});
        CHECK(screen.resumed == 0); // not before the server's list

        screen.reconnect.OnGreeting(LoginServer::Connect);
        CHECK(screen.resumed == 1);
        CHECK_FALSE(screen.reconnect.IsReconnecting());

        screen.reconnect.OnGreeting(LoginServer::Connect); // a later list refresh
        CHECK(screen.resumed == 1);
        CHECK(screen.link.failures == 0);
    }

    TEST_CASE("a login on a dropped game server reconnects to it and logs in")
    {
        DroppedScreen screen;

        CHECK_FALSE(screen.reconnect.EnsureConnected(LoginServer::Game, screen.CountResume()));
        CHECK(screen.link.probes == std::vector{LoginServer::Game});

        screen.ServerComesBack();
        CHECK(screen.link.connects == std::vector{LoginServer::Game});

        screen.reconnect.OnGreeting(LoginServer::Game);
        CHECK(screen.resumed == 1);
    }

    TEST_CASE("a login doesn't go out over the connect server's connection")
    {
        DroppedScreen screen;
        screen.link.connected = true; // reconnected to the connect server meanwhile

        CHECK_FALSE(screen.reconnect.EnsureConnected(LoginServer::Game, screen.CountResume()));
        CHECK(screen.link.probes == std::vector{LoginServer::Game});

        screen.ServerComesBack();
        CHECK(screen.link.connects == std::vector{LoginServer::Game});
        screen.reconnect.OnGreeting(LoginServer::Game);
        CHECK(screen.resumed == 1);
    }

    TEST_CASE("a server pick doesn't go out over a game server's connection")
    {
        DroppedScreen screen;
        screen.link.connected = true;
        screen.link.connectedServer = LoginServer::Game;

        CHECK_FALSE(screen.reconnect.EnsureConnected(LoginServer::Connect, screen.CountResume()));
        CHECK(screen.link.probes == std::vector{LoginServer::Connect});
    }

    TEST_CASE("the login window asks for no list, connected or dropped")
    {
        // The credits open and close over the login window.
        DroppedScreen screen;
        screen.link.connectedServer = LoginServer::Game;
        screen.link.connected = true;
        screen.reconnect.RequestServerList();
        screen.link.connected = false;
        screen.reconnect.RequestServerList();

        CHECK(screen.link.listRequests == 0);
        CHECK(screen.link.probes.empty());
        CHECK_FALSE(screen.reconnect.IsReconnecting());
    }

    TEST_CASE("a join answer that ends the login drops the waiting login")
    {
        // The game server refused the client, or wants a newer version.
        DroppedScreen screen;
        screen.reconnect.EnsureConnected(LoginServer::Game, screen.CountResume());
        screen.ServerComesBack();

        screen.reconnect.OnGreeting(LoginServer::Game, false);
        CHECK(screen.resumed == 0);
        CHECK_FALSE(screen.reconnect.IsReconnecting());
        CHECK(screen.link.failures == 0);
    }

    TEST_CASE("the player is told that the client connects again")
    {
        DroppedScreen screen;
        screen.reconnect.EnsureConnected(LoginServer::Connect, screen.CountResume());
        CHECK(screen.link.connectingShown == 1);

        screen.reconnect.EnsureConnected(LoginServer::Connect, screen.CountResume()); // a second click
        screen.ServerComesBack();
        screen.reconnect.OnGreeting(LoginServer::Connect);
        CHECK(screen.link.connectingShown == 1);
    }

    TEST_CASE("a drop while the player can go on stays quiet")
    {
        DroppedScreen screen;

        for (int frame = 0; frame < 3; ++frame)
        {
            screen.reconnect.OnConnectionLost();
            screen.reconnect.Update();
        }

        CHECK(screen.link.probes.empty());
        CHECK(screen.link.failures == 0);
        CHECK_FALSE(screen.reconnect.IsReconnecting());
    }

    TEST_CASE("a drop while the screen waits for a server reconnects at once")
    {
        DroppedScreen screen;
        screen.link.awaited = LoginServer::Game;

        screen.reconnect.OnConnectionLost();
        CHECK(screen.link.probes == std::vector{LoginServer::Game});

        screen.reconnect.OnConnectionLost(); // the next frame, still probing
        CHECK(screen.link.probes.size() == 1);

        screen.ServerComesBack();
        screen.reconnect.OnGreeting(LoginServer::Game);
        CHECK_FALSE(screen.reconnect.IsReconnecting());
        CHECK(screen.link.failures == 0);
    }

    TEST_CASE("an unreachable server shows the failure once")
    {
        DroppedScreen screen;
        screen.reconnect.EnsureConnected(LoginServer::Connect, screen.CountResume());

        screen.link.probeResult = LoginSceneReconnectLink::ProbeResult::Unreachable;
        screen.reconnect.Update();
        CHECK(screen.link.failures == 1);
        CHECK_FALSE(screen.reconnect.IsReconnecting());

        screen.reconnect.OnConnectionLost();
        screen.reconnect.Update();
        CHECK(screen.link.failures == 1);
        CHECK(screen.link.probes.size() == 1);
    }

    TEST_CASE("a step taken past the failure message starts over")
    {
        // The message doesn't hold the screen; the server list under it still takes clicks.
        DroppedScreen screen;
        int firstPick = 0;
        screen.reconnect.EnsureConnected(LoginServer::Connect, [&firstPick] { ++firstPick; });
        screen.link.probeResult = LoginSceneReconnectLink::ProbeResult::Unreachable;
        screen.reconnect.Update();
        REQUIRE(screen.link.failureShown);

        screen.link.probeResult = LoginSceneReconnectLink::ProbeResult::Pending;
        CHECK_FALSE(screen.reconnect.EnsureConnected(LoginServer::Connect, screen.CountResume()));
        CHECK_FALSE(screen.link.failureShown);
        CHECK(screen.link.probes.size() == 2);
        CHECK(screen.reconnect.IsReconnecting());

        screen.ServerComesBack();
        screen.reconnect.OnGreeting(LoginServer::Connect);
        CHECK(firstPick == 0);
        CHECK(screen.resumed == 1);
    }

    TEST_CASE("a failure message another message replaced doesn't hold the screen")
    {
        DroppedScreen screen;
        screen.reconnect.EnsureConnected(LoginServer::Connect, screen.CountResume());
        screen.link.probeResult = LoginSceneReconnectLink::ProbeResult::Unreachable;
        screen.reconnect.Update();

        screen.link.failureShown = false; // e.g. "the server is busy" from a click under it
        screen.reconnect.Update();
        screen.reconnect.Retry(); // its button is gone with it
        CHECK(screen.link.probes.size() == 1);

        // That message is closed too and the screen shows nothing to click.
        screen.link.awaited = LoginServer::Connect;
        screen.reconnect.OnConnectionLost();
        CHECK(screen.link.probes.size() == 2);

        screen.ServerComesBack();
        screen.reconnect.OnGreeting(LoginServer::Connect);
        CHECK(screen.resumed == 0); // the pick waiting before the failure is dropped
        CHECK(screen.link.failures == 1);
    }

    TEST_CASE("a check that can't start fails at once")
    {
        DroppedScreen screen;
        screen.link.probeCanStart = false;

        CHECK_FALSE(screen.reconnect.EnsureConnected(LoginServer::Game, screen.CountResume()));
        CHECK(screen.link.failures == 1);
    }

    TEST_CASE("a connect that fails after the check shows the failure")
    {
        DroppedScreen screen;
        screen.link.connectSucceeds = false;
        screen.reconnect.EnsureConnected(LoginServer::Connect, screen.CountResume());

        screen.ServerComesBack();
        CHECK(screen.link.failures == 1);
    }

    TEST_CASE("Reconnect after a failed pick keeps the pick")
    {
        DroppedScreen screen;
        screen.reconnect.EnsureConnected(LoginServer::Connect, screen.CountResume());
        screen.link.probeResult = LoginSceneReconnectLink::ProbeResult::Unreachable;
        screen.reconnect.Update();

        screen.reconnect.Retry();
        CHECK(screen.link.probes.back() == LoginServer::Connect);
        screen.ServerComesBack();
        screen.reconnect.OnGreeting(LoginServer::Connect);
        CHECK(screen.resumed == 1);
    }

    TEST_CASE("Reconnect after a failed login starts over from the server list")
    {
        DroppedScreen screen;
        screen.reconnect.EnsureConnected(LoginServer::Game, screen.CountResume());
        screen.link.probeResult = LoginSceneReconnectLink::ProbeResult::Unreachable;
        screen.reconnect.Update();

        screen.reconnect.Retry();
        CHECK(screen.link.probes == std::vector{LoginServer::Game, LoginServer::Connect});
        screen.ServerComesBack();
        CHECK(screen.link.connects == std::vector{LoginServer::Connect});
        screen.reconnect.OnGreeting(LoginServer::Connect);
        CHECK(screen.resumed == 0); // the player picks a server and logs in again
    }

    TEST_CASE("Cancel and three failed logins go back to the server list")
    {
        // The login window's Cancel, or OK after the game server closed the login for
        // the third wrong password (maybe only after the player answered its message).
        DroppedScreen screen;
        screen.link.connected = true;

        screen.reconnect.ReturnToServerList();
        CHECK(screen.link.probes == std::vector{LoginServer::Connect});
        screen.ServerComesBack();
        CHECK(screen.link.connects == std::vector{LoginServer::Connect});

        screen.reconnect.OnGreeting(LoginServer::Connect);
        CHECK_FALSE(screen.reconnect.IsReconnecting());
        CHECK(screen.link.failures == 0);
    }

    TEST_CASE("Reconnect does nothing while no failure is shown")
    {
        DroppedScreen screen;
        screen.reconnect.Retry();
        CHECK(screen.link.probes.empty());
    }

    TEST_CASE("a server that doesn't answer in time fails")
    {
        DroppedScreen screen;
        screen.reconnect.EnsureConnected(LoginServer::Connect, screen.CountResume());
        screen.ServerComesBack();

        screen.link.now += GreetingTimeoutMs;
        screen.reconnect.Update();
        CHECK(screen.link.failures == 0);

        screen.link.now += 1.0;
        screen.reconnect.Update();
        CHECK(screen.link.failures == 1);
        screen.reconnect.OnGreeting(LoginServer::Connect); // too late
        CHECK(screen.resumed == 0);
    }

    TEST_CASE("the new connection closing before the answer fails")
    {
        DroppedScreen screen;
        screen.reconnect.EnsureConnected(LoginServer::Game, screen.CountResume());
        screen.ServerComesBack();

        screen.link.connected = false;
        screen.reconnect.OnConnectionLost();
        CHECK(screen.link.failures == 1);
    }

    TEST_CASE("the player's latest step replaces the one waiting")
    {
        DroppedScreen screen;
        int firstPick = 0;
        screen.reconnect.EnsureConnected(LoginServer::Connect, [&firstPick] { ++firstPick; });
        CHECK_FALSE(screen.reconnect.EnsureConnected(LoginServer::Connect, screen.CountResume()));
        CHECK(screen.link.probes.size() == 1);

        screen.ServerComesBack();
        screen.reconnect.OnGreeting(LoginServer::Connect);
        CHECK(firstPick == 0);
        CHECK(screen.resumed == 1);
    }

    TEST_CASE("the credits opened during a reconnect keep the waiting pick")
    {
        DroppedScreen screen;
        screen.reconnect.EnsureConnected(LoginServer::Connect, screen.CountResume());

        screen.reconnect.RequestServerList(); // the credits open, and close again
        screen.reconnect.RequestServerList();
        CHECK(screen.link.probes.size() == 1);
        CHECK(screen.link.listRequests == 0);

        screen.ServerComesBack();
        screen.reconnect.OnGreeting(LoginServer::Connect);
        CHECK(screen.resumed == 1);
    }

    TEST_CASE("a greeting from the other server resumes nothing")
    {
        // A client started straight on a game server gets the login window back
        // where a server list was expected.
        DroppedScreen screen;
        screen.reconnect.EnsureConnected(LoginServer::Connect, screen.CountResume());
        screen.ServerComesBack();

        screen.reconnect.OnGreeting(LoginServer::Game);
        CHECK(screen.resumed == 0);
        CHECK_FALSE(screen.reconnect.IsReconnecting());
        CHECK(screen.link.failures == 0);
    }

    TEST_CASE("the list request goes out at once or comes with the reconnect")
    {
        DroppedScreen screen;
        screen.link.connected = true;
        screen.reconnect.RequestServerList();
        CHECK(screen.link.listRequests == 1);

        screen.link.connected = false;
        screen.reconnect.RequestServerList();
        CHECK(screen.link.listRequests == 1);
        CHECK(screen.link.probes == std::vector{LoginServer::Connect});

        screen.ServerComesBack();
        screen.reconnect.OnGreeting(LoginServer::Connect);
        CHECK(screen.link.listRequests == 1);
    }

    TEST_CASE("an open server list asks for a fresh one before the server drops it")
    {
        DroppedScreen screen;
        screen.link.connected = true;
        screen.link.serverListShown = true;
        screen.reconnect.OnGreeting(LoginServer::Connect); // the list arrived

        screen.link.now += ServerListRefreshMs - 1.0;
        screen.reconnect.Update();
        CHECK(screen.link.listRequests == 0);

        screen.link.now += 1.0;
        screen.reconnect.Update();
        CHECK(screen.link.listRequests == 1);
        screen.reconnect.Update(); // its answer is on the way
        CHECK(screen.link.listRequests == 1);

        screen.reconnect.OnGreeting(LoginServer::Connect);
        screen.link.serverListShown = false; // a server was picked
        screen.link.now += ServerListRefreshMs;
        screen.reconnect.Update();
        CHECK(screen.link.listRequests == 1);
    }

    TEST_CASE("a dropped server list waits for the player's next step")
    {
        DroppedScreen screen;
        screen.link.serverListShown = true;
        screen.link.now += ServerListRefreshMs;

        screen.reconnect.Update();
        CHECK(screen.link.listRequests == 0);
        CHECK(screen.link.probes.empty());
    }

    TEST_CASE("an open server list connects anew before the connect server's limit")
    {
        DroppedScreen screen;
        screen.UseUpListRequests();
        CHECK(screen.link.listRequests == ListRequestsPerConnection - 1);
        CHECK(screen.link.probes.empty());

        screen.RefreshDue(); // one more request would end the connection
        CHECK(screen.link.listRequests == ListRequestsPerConnection - 1);
        CHECK(screen.link.probes == std::vector{LoginServer::Connect});
        CHECK(screen.link.connectingShown == 0); // nobody waits for it

        screen.ServerComesBack();
        screen.reconnect.OnGreeting(LoginServer::Connect); // the new connection's list
        CHECK_FALSE(screen.reconnect.IsReconnecting());

        screen.RefreshDue(); // the new connection has its own requests
        CHECK(screen.link.listRequests == ListRequestsPerConnection);
        CHECK(screen.link.failures == 0);
    }

    TEST_CASE("a renewal that can't reach the server counts as a drop")
    {
        DroppedScreen screen;
        screen.UseUpListRequests();
        screen.RefreshDue();

        screen.link.probeResult = LoginSceneReconnectLink::ProbeResult::Unreachable;
        screen.reconnect.Update();
        CHECK(screen.link.failures == 0);
        CHECK(screen.link.connectingShown == 0);
        CHECK_FALSE(screen.reconnect.IsReconnecting());

        screen.RefreshDue(); // the old connection still has no request left
        CHECK(screen.link.probes.size() == 2);
        CHECK(screen.link.listRequests == ListRequestsPerConnection - 1);
    }

    TEST_CASE("a step during a renewal waits for it and is told so")
    {
        DroppedScreen screen;
        screen.UseUpListRequests();
        screen.RefreshDue();

        CHECK_FALSE(screen.reconnect.EnsureConnected(LoginServer::Connect, screen.CountResume()));
        CHECK(screen.link.connectingShown == 1);

        screen.link.probeResult = LoginSceneReconnectLink::ProbeResult::Unreachable;
        screen.reconnect.Update();
        CHECK(screen.link.failures == 1); // the player waits for this one
    }

    TEST_CASE("a group click with no list request left connects anew for its list")
    {
        DroppedScreen screen;
        screen.link.connected = true;
        for (int request = 1; request < ListRequestsPerConnection; ++request)
        {
            screen.reconnect.RequestServerList();
        }
        CHECK(screen.link.listRequests == ListRequestsPerConnection - 1);

        screen.reconnect.RequestServerList();
        CHECK(screen.link.listRequests == ListRequestsPerConnection - 1);
        CHECK(screen.link.probes == std::vector{LoginServer::Connect});
        CHECK(screen.link.connectingShown == 1);
    }

    TEST_CASE("leaving the login scene drops the waiting step")
    {
        DroppedScreen screen;
        screen.reconnect.EnsureConnected(LoginServer::Connect, screen.CountResume());

        screen.reconnect.Reset();
        CHECK(screen.link.probeStops == 1);
        CHECK_FALSE(screen.reconnect.IsReconnecting());

        screen.reconnect.OnGreeting(LoginServer::Connect);
        CHECK(screen.resumed == 0);
    }
}
