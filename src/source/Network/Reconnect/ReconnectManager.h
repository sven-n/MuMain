#pragma once

#include <cstdint>

#include "Core/Globals/_define.h"  // MAX_USERNAME_SIZE, MAX_PASSWORD_SIZE

// Drives automatic reconnection after an in-game disconnect (issue #338).
//
// When the connection to the game server drops while playing, the client caches
// the data needed to resume the session (game-server address, account
// credentials, selected character) and replays the normal
// connect -> login -> character-list -> select-character -> join sequence,
// retrying the probe every few seconds until the server is back. Because the
// server persists character state, re-entering the game restores the player
// exactly where they left off.
//
// The manager only injects the two steps the scene loops cannot do on their own
// (sending the login and picking the character). Everything else flows through
// the existing per-frame scene state machine (NewMoveLogInScene /
// NewMoveCharacterScene / StartGame), which advances on CurrentProtocolState.
class ReconnectManager
{
public:
    enum class Phase
    {
        Idle,           // not reconnecting
        Probing,        // in-game, world still rendered; probing the server for life
        Connecting,     // socket opened, waiting for the server hello
        LoggingIn,      // login sent, login/character scene loops are driving
        SelectingChar,  // character list arrived, selecting the cached character
        Joining,        // loading into the world
        Retrying,       // post-teardown: re-connect + re-login attempt failed, retry
        Done,           // back in game; dialog dismissed next frame
    };

    static ReconnectManager& Instance();

    // Session cache, populated during normal play.
    void CacheServer(const wchar_t* ip, unsigned short port);
    void CacheCredentials(const wchar_t* username, const wchar_t* password);
    void CacheCharacter(const wchar_t* characterName);
    void ClearSession();
    bool HasSession() const { return m_hasSession; }

    // Lifecycle.
    void RequestBegin();  // detector asks to start reconnect (begins probing)
    void Begin();         // performs the teardown between frames; the main loop calls it once m_beginPending is set
    void Update();        // per-frame driver (runs in the scene-update phase)
    void RequestCancel(); // dialog's Cancel button; processed by Update()

    // Read by ReconnectDialog.
    bool IsActive() const { return m_active || m_beginPending; }
    Phase GetPhase() const { return m_phase; }
    int GetStepIndex() const;        // 1-based progress step for the bar
    static int GetStepCount();       // total steps the bar represents
    int GetCountdownSeconds() const; // seconds until the next probe (Waiting only)

private:
    ReconnectManager() = default;

    void EnterPhase(Phase phase);
    void Abort();                     // give up -> back to the login screen
    double ElapsedMs() const;         // time spent in the current phase
    bool IsSocketAlive() const;
    bool TrySelectCachedCharacter();

    void UpdateProbing();
    void UpdateConnecting();
    void UpdateLoggingIn();
    void UpdateSelectingChar();
    void UpdateJoining();
    void UpdateRetrying();

    // Background TCP reachability probe (keeps the game rendered while waiting).
    void StartProbe();
    void PollProbe();
    void CloseProbe();

    bool m_active = false;
    bool m_beginPending = false;       // RequestBegin posted, Begin not yet run
    bool m_cancelRequested = false;    // Cancel clicked, Abort not yet run
    bool m_abortAfterTeardown = false; // cancel during Probing: tear down, then abort
    bool m_hasSession = false;
    bool m_muHelperWasActive = false; // MU Helper state to restore after reconnect
    Phase m_phase = Phase::Idle;
    double m_phaseStartTime = 0.0;

    // Background probe socket (uintptr_t holds a winsock SOCKET; ~0 = none).
    uintptr_t m_probeSocket = static_cast<uintptr_t>(~0ull);
    double m_probeStartTime = 0.0;

    wchar_t m_serverIp[256] = {};  // IPv4 literal or hostname
    unsigned short m_serverPort = 0;
    wchar_t m_username[MAX_USERNAME_SIZE + 1] = {};
    wchar_t m_password[MAX_PASSWORD_SIZE + 1] = {};
    wchar_t m_characterName[MAX_USERNAME_SIZE + 1] = {};
};
