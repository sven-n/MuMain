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
    virtual ~LoginSceneReconnectLink() = default;

    virtual bool IsConnected() const = 0;
    // The server the connection goes to, or went to before it closed.
    virtual LoginServer ConnectedServer() const = 0;
    // Tells the live connection from the ones before it; the connect server limits the
    // lists it sends per connection.
    virtual int ConnectionId() const = 0;

    // The server the screen waits for while it shows nothing to click (after a server
    // was picked, after the login was sent, before the first list); none while the
    // player can go on by themselves.
    virtual std::optional<LoginServer> AwaitedServer() const = 0;

    // The server list is on screen, over a connection to the connect server.
    virtual bool IsServerListShown() const = 0;

    // Starts connecting and returns at once: the server's greeting arrives later, or the
    // connection closes first when the server can't be reached. False when no connect
    // could start.
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
