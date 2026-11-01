#pragma once

#include <functional>

#include "Network/Reconnect/LoginSceneReconnectLink.h"

// Reconnects quietly when a server drops the client on the login screen.
//
// The connect server closes connections that stay idle (OpenMU: after one minute
// by default), and a router can cut an idle game-server connection the same way.
// An open server list asks for a fresh one now and then, which keeps its connection
// up; when the connect server's limit of lists per connection is near, the list
// connects anew in the background. A drop can still come (a shorter timeout, a
// server restart). Nothing is lost
// before the login, so the client doesn't tell the player. It reconnects when the
// player next needs the server (picking a server, changing
// the server group, opening the credits, logging in) and then carries out that
// step. When the screen itself waits for an answer, it reconnects at once. Only
// when the server can't be reached does the player get a choice: try again from
// the server list, or leave the game.
class LoginSceneReconnect
{
public:
    explicit LoginSceneReconnect(LoginSceneReconnectLink& link);

    // The one the game uses, wired to the game's connection (GameLoginSceneLink.cpp).
    static LoginSceneReconnect& Instance();

    // True when the connection to `server` is up, so the caller sends right away.
    // Otherwise (no connection, or one to the other server) connects to `server`,
    // runs `resume` (if any) once it greeted the client, and returns false.
    bool EnsureConnected(LoginServer server, std::function<void()> resume);

    // Asks the connect server for its list. After a reconnect the client asks on
    // its own, so nothing is left to resume then; a connection that has no list
    // request left connects anew for the same reason. Nothing while a reconnect runs
    // (it brings its own answer, and a server pick waiting for it stays), nor while
    // the screen is on a game server (the login window): only the connect server
    // sends a list, and switching would end the login window's connection.
    void RequestServerList();

    // The login scene found its connection closed; called every frame until it is back.
    void OnConnectionLost();

    // A server greeted the client: the connect server with its list, the game
    // server with its join answer. `carryOn` is false when that answer ends the
    // step itself (the game server refused the client, or wants a newer version);
    // the step waiting for it is dropped then.
    void OnGreeting(LoginServer server, bool carryOn = true);

    // The server the client was sent to can't be reached: a server pick's game
    // server, which the connect server still lists. The player gets the failure
    // message, and Reconnect starts over at the server list.
    void OnServerUnreachable(LoginServer server);

    // Goes back to the server list over a new connection to the connect server: the
    // login window's Cancel, and OK after the game server closed the login for three
    // wrong passwords (rather than logging in again over a new connection to it).
    void ReturnToServerList();

    void Update(); // per frame while the login scene runs
    void Retry();  // the failure message's Reconnect button
    void Reset();  // the login scene was left

    bool IsReconnecting() const;

private:
    enum class Phase
    {
        Idle,
        Probing,          // waiting for the server to accept connections
        AwaitingGreeting, // connected, waiting for the server's first answer
        Failed,           // the failure message is up
    };

    // `quiet`: a renewal nobody waits for. It tells the player nothing, and when it
    // fails that counts as a plain drop.
    void Begin(LoginServer server, std::function<void()> resume, bool quiet = false);
    void PlayerWaits();
    void RefreshServerList();
    bool TakeListRequest();
    void UpdateProbing();
    void Fail();
    void EnterPhase(Phase phase);

    LoginSceneReconnectLink& m_link;
    Phase m_phase = Phase::Idle;
    LoginServer m_server = LoginServer::Connect;
    std::function<void()> m_resume;
    bool m_quiet = false;
    double m_phaseStartMs = 0.0;
    double m_lastListMs = 0.0; // the last server list asked for or received
    int m_listConnection = 0;  // the connection m_listRequests counts for
    int m_listRequests = 0;    // list requests the client sent over it
    bool m_lossLogged = false;
};
