#include "stdafx.h"
#include "ReconnectManager.h"

#include <ws2tcpip.h>  // GetAddrInfoW for the reachability probe (IPs + hostnames)

#include <cmath>
#include <cwchar>

#include "Network/Server/WSclient.h"   // SocketClient, CreateSocket, DeleteSocket,
                                       // ResetClientToLoginScene, protocol states
#include "Scenes/SceneCore.h"          // SceneFlag, szServerIpAddress, g_ServerPort
#include "Scenes/SceneCommon.h"        // SelectedHero, MAX_CHARACTERS_PER_ACCOUNT
#include "Scenes/CharacterScene.h"     // StartGame
#include "Engine/Object/ZzzCharacter.h"// CharactersClient
#include "UI/Legacy/UIMng.h"           // CUIMng, m_LoginWin
#include "MUHelper/MuHelper.h"         // MUHelper::g_MuHelper

// Timing (milliseconds). WorldTime is the wall-clock ms timer the rest of the
// client uses for timeouts (see ZzzAI.cpp).
namespace
{
    constexpr double PROBE_INTERVAL_MS = 5000.0;    // gap between reachability probes
    constexpr double PROBE_TIMEOUT_MS = 2000.0;     // allow a probe this long to connect
    constexpr double CONNECT_TIMEOUT_MS = 10000.0;  // wait for the server hello
    constexpr double LOGIN_TIMEOUT_MS = 10000.0;    // login -> character list
    constexpr double RETRY_INTERVAL_MS = 5000.0;    // gap between re-login retries
    constexpr int STEP_COUNT = 4;                   // progress-bar steps

    const uintptr_t NO_PROBE = static_cast<uintptr_t>(~0ull);  // == INVALID_SOCKET
}

extern double WorldTime;
extern int LogIn;
extern wchar_t LogInID[MAX_USERNAME_SIZE + 1];
extern BYTE Version[SIZE_PROTOCOLVERSION];
extern BYTE Serial[SIZE_PROTOCOLSERIAL + 1];
extern BOOL g_bGameServerConnected;

ReconnectManager& ReconnectManager::Instance()
{
    static ReconnectManager instance;
    return instance;
}

void ReconnectManager::CacheServer(const wchar_t* ip, unsigned short port)
{
    if (ip == nullptr)
    {
        return;
    }

    wcscpy_s(m_serverIp, _countof(m_serverIp), ip);
    m_serverPort = port;
}

void ReconnectManager::CacheCredentials(const wchar_t* username, const wchar_t* password)
{
    if (username == nullptr || password == nullptr)
    {
        return;
    }

    wcscpy_s(m_username, _countof(m_username), username);
    wcscpy_s(m_password, _countof(m_password), password);
}

void ReconnectManager::CacheCharacter(const wchar_t* characterName)
{
    if (characterName == nullptr)
    {
        return;
    }

    wcscpy_s(m_characterName, _countof(m_characterName), characterName);

    // A full session is only usable once we know the server, the account, and
    // the character to resume.
    m_hasSession = m_serverIp[0] != L'\0' && m_username[0] != L'\0' && m_characterName[0] != L'\0';
}

void ReconnectManager::ClearSession()
{
    m_hasSession = false;
    m_serverIp[0] = L'\0';
    m_username[0] = L'\0';
    m_password[0] = L'\0';
    m_characterName[0] = L'\0';
}

void ReconnectManager::RequestBegin()
{
    if (m_active || m_beginPending || !m_hasSession)
    {
        return;
    }

    // Capture the MU Helper state now (at the moment of disconnect), before
    // anything in the reconnect can touch it, so it can be restored afterwards.
    m_muHelperWasActive = MUHelper::g_MuHelper.IsActive();

    // Don't tear the game down yet: keep the main scene rendering (frozen, since
    // no server data arrives) and probe the server in the background. Only once
    // it answers do we tear down and re-login. This just sets state, so it's safe
    // to call from the render path.
    m_active = true;
    EnterPhase(Phase::Probing);
    // Probe straight away instead of waiting out the first interval, so a brief
    // blip recovers quickly.
    m_phaseStartTime = WorldTime - PROBE_INTERVAL_MS;
}

void ReconnectManager::Begin()
{
    // Runs in the window-proc context once a probe found the server alive. The
    // teardown (ReleaseMainData etc.) must run here, not mid-render.
    if (!m_beginPending)
    {
        return;
    }

    m_beginPending = false;

    if (!m_active)
    {
        return;
    }

    // Tear the live session down to a clean login state.
    ResetClientToLoginScene();

    // Cancel pressed while probing: stop here at the login screen.
    if (m_abortAfterTeardown)
    {
        m_abortAfterTeardown = false;
        Abort();
        return;
    }

    // Connect (the probe already confirmed the server is up).
    DeleteSocket();
    CreateSocket(m_serverIp, m_serverPort);

    if (IsSocketAlive())
    {
        g_bGameServerConnected = TRUE;
        EnterPhase(Phase::Connecting);
        return;
    }

    // Rare: the server went down again between the probe and the connect. Retry
    // (we've already torn down, so this just re-opens the socket).
    EnterPhase(Phase::Retrying);
}

void ReconnectManager::RequestCancel()
{
    if (!m_active)
    {
        return;
    }

    // Processed by Update() (scene-update phase) so the teardown never runs from
    // the dialog's render call.
    m_cancelRequested = true;
}

void ReconnectManager::Update()
{
    if (m_cancelRequested)
    {
        m_cancelRequested = false;
        if (m_active)
        {
            if (m_phase == Phase::Probing && SceneFlag == MAIN_SCENE)
            {
                // The game world is still loaded; its teardown must run between
                // frames (not mid-render), after which we land on the login
                // screen. m_beginPending defers it: the main loop calls Begin()
                // each frame and it fires once the flag is set.
                m_abortAfterTeardown = true;
                m_beginPending = true;
            }
            else
            {
                Abort();
            }
        }
        return;
    }

    if (!m_active)
    {
        return;
    }

    switch (m_phase)
    {
    case Phase::Probing:        UpdateProbing();       break;
    case Phase::Connecting:     UpdateConnecting();    break;
    case Phase::LoggingIn:      UpdateLoggingIn();     break;
    case Phase::SelectingChar:  UpdateSelectingChar(); break;
    case Phase::Joining:        UpdateJoining();       break;
    case Phase::Retrying:       UpdateRetrying();      break;
    default:                                           break;
    }
}

void ReconnectManager::UpdateProbing()
{
    // A probe found the server; waiting for Begin() to run in the window proc.
    if (m_beginPending)
    {
        return;
    }

    if (m_probeSocket == NO_PROBE)
    {
        if (ElapsedMs() >= PROBE_INTERVAL_MS)
        {
            StartProbe();
        }
        return;
    }

    PollProbe();
}

void ReconnectManager::StartProbe()
{
    static bool s_wsaInitialised = false;
    if (!s_wsaInitialised)
    {
        WSADATA wsaData;
        WSAStartup(MAKEWORD(2, 2), &wsaData);
        s_wsaInitialised = true;
    }

    const SOCKET probe = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (probe == INVALID_SOCKET)
    {
        EnterPhase(Phase::Probing);  // reset the interval timer and retry later
        return;
    }

    // Resolve the cached address - works for both an IPv4 literal and a hostname.
    addrinfoW hints = {};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;

    addrinfoW* resolved = nullptr;
    if (GetAddrInfoW(m_serverIp, nullptr, &hints, &resolved) != 0 || resolved == nullptr)
    {
        closesocket(probe);
        EnterPhase(Phase::Probing);  // unresolvable right now; try again later
        return;
    }

    sockaddr_in addr = *reinterpret_cast<sockaddr_in*>(resolved->ai_addr);
    addr.sin_port = htons(m_serverPort);
    FreeAddrInfoW(resolved);

    u_long nonBlocking = 1;
    ioctlsocket(probe, FIONBIO, &nonBlocking);

    connect(probe, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));  // WSAEWOULDBLOCK expected

    m_probeSocket = static_cast<uintptr_t>(probe);
    m_probeStartTime = WorldTime;
}

void ReconnectManager::PollProbe()
{
    const SOCKET probe = static_cast<SOCKET>(m_probeSocket);

    fd_set writeSet;
    fd_set errorSet;
    FD_ZERO(&writeSet);
    FD_ZERO(&errorSet);
    FD_SET(probe, &writeSet);
    FD_SET(probe, &errorSet);

    timeval immediate = {};
    select(0, nullptr, &writeSet, &errorSet, &immediate);

    if (FD_ISSET(probe, &writeSet))
    {
        int soError = 0;
        int len = sizeof(soError);
        getsockopt(probe, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&soError), &len);
        CloseProbe();

        if (soError == 0)
        {
            // Server is up - defer to Begin() (called by the main loop each
            // frame) to tear down and re-login between frames.
            m_beginPending = true;
        }
        else
        {
            EnterPhase(Phase::Probing);
        }
        return;
    }

    if (FD_ISSET(probe, &errorSet) || (WorldTime - m_probeStartTime) > PROBE_TIMEOUT_MS)
    {
        CloseProbe();
        EnterPhase(Phase::Probing);
    }
}

void ReconnectManager::CloseProbe()
{
    if (m_probeSocket != NO_PROBE)
    {
        closesocket(static_cast<SOCKET>(m_probeSocket));
        m_probeSocket = NO_PROBE;
    }
}

void ReconnectManager::UpdateConnecting()
{
    if (!IsSocketAlive())
    {
        EnterPhase(Phase::Retrying);
        return;
    }

    // The game server answers a fresh connection with its hello packet, which
    // ReceiveJoinServer turns into RECEIVE_JOIN_SERVER_SUCCESS (LogIn was reset
    // to 0 by the teardown, so it takes the login branch).
    if (CurrentProtocolState == RECEIVE_JOIN_SERVER_SUCCESS)
    {
        CUIMng& uiMng = CUIMng::Instance();
        uiMng.HideWin(&uiMng.m_LoginWin);

        LogIn = 1;
        wcscpy_s(LogInID, _countof(LogInID), m_username);
        CurrentProtocolState = REQUEST_LOG_IN;
        SocketClient->ToGameServer()->SendLogin(m_username, m_password, Version, Serial);

        EnterPhase(Phase::LoggingIn);
        return;
    }

    if (ElapsedMs() > CONNECT_TIMEOUT_MS)
    {
        EnterPhase(Phase::Retrying);
    }
}

void ReconnectManager::UpdateLoggingIn()
{
    if (!IsSocketAlive())
    {
        EnterPhase(Phase::Retrying);
        return;
    }

    // On login success the login-scene loop (NewMoveLogInScene) requests the
    // character list and switches to the character scene on its own. Wait for
    // the list to arrive.
    if (SceneFlag == CHARACTER_SCENE && CurrentProtocolState == RECEIVE_CHARACTERS_LIST)
    {
        EnterPhase(Phase::SelectingChar);
        return;
    }

    // No progress: most often the server still has the old session marked
    // "logged in" right after the crash and rejects the re-login. Retry - it
    // clears within a few seconds. (The player can Cancel if credentials are
    // genuinely wrong.)
    if (ElapsedMs() > LOGIN_TIMEOUT_MS)
    {
        EnterPhase(Phase::Retrying);
    }
}

void ReconnectManager::UpdateSelectingChar()
{
    if (!IsSocketAlive())
    {
        EnterPhase(Phase::Retrying);
        return;
    }

    if (TrySelectCachedCharacter())
    {
        EnterPhase(Phase::Joining);
        return;
    }

    // The character is gone (deleted on another client?) - fall back to manual
    // selection rather than looping forever.
    Abort();
}

void ReconnectManager::UpdateJoining()
{
    if (SceneFlag == MAIN_SCENE)
    {
        // Back in the world exactly where we left off.
        if (m_muHelperWasActive)
        {
            // Resume the client-side MU Helper so its state matches what the
            // player left running (and so the stop toggle works again).
            MUHelper::g_MuHelper.Start();
            m_muHelperWasActive = false;
        }

        m_active = false;
        m_phase = Phase::Idle;
        return;
    }

    // Loading can be slow, so only retry if the socket actually died.
    if (!IsSocketAlive())
    {
        EnterPhase(Phase::Retrying);
    }
}

void ReconnectManager::UpdateRetrying()
{
    if (ElapsedMs() < RETRY_INTERVAL_MS)
    {
        return;
    }

    // Already torn down (Begin ran once); just re-open the socket and let the
    // Connecting phase re-drive the login. Reset the protocol state so the
    // server hello is treated as a fresh login rather than a map change.
    LogIn = 0;
    CurrentProtocolState = REQUEST_JOIN_SERVER;
    DeleteSocket();
    CreateSocket(m_serverIp, m_serverPort);

    if (IsSocketAlive())
    {
        g_bGameServerConnected = TRUE;
        EnterPhase(Phase::Connecting);
        return;
    }

    EnterPhase(Phase::Retrying);  // server not reachable this time; wait and retry
}

bool ReconnectManager::TrySelectCachedCharacter()
{
    if (m_characterName[0] == L'\0' || CharactersClient == nullptr)
    {
        return false;
    }

    for (int i = 0; i < MAX_CHARACTERS_PER_ACCOUNT; ++i)
    {
        if (wcscmp(CharactersClient[i].ID, m_characterName) == 0)
        {
            SelectedHero = i;
            StartGame();   // -> LOADING_SCENE -> MAIN_SCENE (sends SelectCharacter)
            return true;
        }
    }

    return false;
}

void ReconnectManager::Abort()
{
    m_active = false;
    m_beginPending = false;
    m_cancelRequested = false;
    m_abortAfterTeardown = false;
    m_phase = Phase::Idle;
    CloseProbe();

    // Begin() already released the in-game data, so this only needs to land on a
    // usable login screen and reconnect to the server for a manual login. It
    // runs from Update() (scene-update phase), so the light scene reset is safe.
    DeleteSocket();
    g_sceneInit.ResetForDisconnect();
    SceneFlag = LOG_IN_SCENE;
    LogIn = 0;
    CurrentProtocolState = REQUEST_JOIN_SERVER;
    CreateSocket(szServerIpAddress, g_ServerPort);
}

void ReconnectManager::EnterPhase(Phase phase)
{
    m_phase = phase;
    m_phaseStartTime = WorldTime;
}

double ReconnectManager::ElapsedMs() const
{
    return WorldTime - m_phaseStartTime;
}

bool ReconnectManager::IsSocketAlive() const
{
    return SocketClient != nullptr && SocketClient->IsConnected();
}

int ReconnectManager::GetStepCount()
{
    return STEP_COUNT;
}

int ReconnectManager::GetStepIndex() const
{
    switch (m_phase)
    {
    // Probing/Retrying/Connecting share a step so the bar doesn't jump forward
    // and snap back when a connect attempt fails and retries.
    case Phase::Probing:        return 1;
    case Phase::Retrying:       return 1;
    case Phase::Connecting:     return 1;
    case Phase::LoggingIn:      return 2;
    case Phase::SelectingChar:  return 3;
    case Phase::Joining:        return 4;
    default:                    return 0;
    }
}

int ReconnectManager::GetCountdownSeconds() const
{
    // Seconds until the next attempt, for the dialog's "retrying in N" text.
    double interval = 0.0;
    if (m_phase == Phase::Probing && m_probeSocket == NO_PROBE)
    {
        interval = PROBE_INTERVAL_MS;
    }
    else if (m_phase == Phase::Retrying)
    {
        interval = RETRY_INTERVAL_MS;
    }
    else
    {
        return 0;
    }

    const double remainingMs = interval - ElapsedMs();
    if (remainingMs <= 0.0)
    {
        return 0;
    }

    return static_cast<int>(std::ceil(remainingMs / 1000.0));
}
