#pragma once

#include <functional>

#include "Network/Reconnect/LoginSceneReconnectLink.h"

// Reconnects quietly when a server drops the client on the login screen.
//
// The connect server closes connections that stay idle (OpenMU: after one minute
// by default), and a router can cut an idle game-server connection the same way.
// Nothing is lost before the login, so the client doesn't tell the player. It
// reconnects when the player next needs the server (picking a server, changing
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

    // True when the connection is up, so the caller sends right away. Otherwise
    // reconnects to `server` and runs `resume` (if any) once the server greeted
    // the client again, and returns false.
    bool EnsureConnected(LoginServer server, std::function<void()> resume);

    // Asks the connect server for its list. After a reconnect the client asks on
    // its own, so nothing is left to resume then.
    void RequestServerList();

    // The login scene found its connection closed; called every frame until it is back.
    void OnConnectionLost();

    // A server greeted the client: the connect server with its list, the game
    // server with its join answer.
    void OnGreeting(LoginServer server);

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

    void Begin(LoginServer server, std::function<void()> resume);
    void UpdateProbing();
    void Fail();
    void EnterPhase(Phase phase);

    LoginSceneReconnectLink& m_link;
    Phase m_phase = Phase::Idle;
    LoginServer m_server = LoginServer::Connect;
    std::function<void()> m_resume;
    double m_phaseStartMs = 0.0;
    bool m_lossLogged = false;
};
