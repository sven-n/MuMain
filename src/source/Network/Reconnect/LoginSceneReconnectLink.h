#pragma once

#include <optional>

// The two servers the login screen talks to: the connect server sends the server
// list, the chosen game server takes the login.
enum class LoginServer
{
    Connect,
    Game,
};

// What LoginSceneReconnect needs from the client around it. GameLoginSceneLink
// works on the game's connection and screens; the tests drive a fake.
class LoginSceneReconnectLink
{
public:
    enum class ProbeResult
    {
        Pending,
        Reachable,
        Unreachable,
    };

    virtual ~LoginSceneReconnectLink() = default;

    virtual bool IsConnected() const = 0;

    // The server the screen waits for while it shows nothing to click (after a server
    // was picked, after the login was sent, before the first list); none while the
    // player can go on by themselves.
    virtual std::optional<LoginServer> AwaitedServer() const = 0;

    // Checks in the background whether the server accepts connections again. False
    // when no check could be started.
    virtual bool StartProbe(LoginServer server) = 0;
    virtual ProbeResult PollProbe() = 0;
    virtual void StopProbe() = 0;

    // Opens the connection; the server's greeting arrives later.
    virtual bool Connect(LoginServer server) = 0;

    virtual void SendServerListRequest() = 0;

    // Tells the player that the client connects again, since the screen shows nothing
    // else while it does.
    virtual void ShowConnecting() = 0;

    // Tells the player the server can't be reached and offers to try again or leave.
    virtual void ShowFailure() = 0;
    // Whether that message is still up; another message can take its place.
    virtual bool IsFailureShown() const = 0;
    virtual void HideFailure() = 0;

    virtual double NowMs() const = 0;
};
