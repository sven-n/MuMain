#include "stdafx.h"
#include "Network/Reconnect/GameLoginSceneLink.h"

#include "I18N/All.h"
#include "Network/Reconnect/LoginSceneReconnect.h"
#include "Network/Reconnect/ReconnectManager.h" // the game server connected to last
#include "Network/Server/WSclient.h"            // SocketClient, CreateSocket, protocol states
#include "Scenes/SceneCore.h"                   // szServerIpAddress, g_ServerPort
#include "UI/Core/WindowSystem.h"               // g_pSystemLogBox
#include "UI/Windows/LoginSceneUpdates.h"

extern double WorldTime;
extern BOOL g_bGameServerConnected;

namespace
{
// How long the background check waits for a server to accept the connection.
constexpr double ProbeTimeoutMs = 5000.0;
} // namespace

// Defined here, with the game's link, so the state machine links without the game (tests).
LoginSceneReconnect& LoginSceneReconnect::Instance()
{
    static GameLoginSceneLink link;
    static LoginSceneReconnect instance(link);
    return instance;
}

bool GameLoginSceneLink::IsConnected() const
{
    return SocketClient != nullptr && SocketClient->IsConnected();
}

LoginServer GameLoginSceneLink::ConnectedServer() const
{
    // ConnectForLogin (by the port) and a server pick (ReceiveServerConnect) set it; a drop leaves it.
    return g_bGameServerConnected ? LoginServer::Game : LoginServer::Connect;
}

int GameLoginSceneLink::ConnectionId() const
{
    // The client library numbers its connections without reusing a number.
    return SocketClient != nullptr ? SocketClient->GetId() : 0;
}

std::optional<LoginServer> GameLoginSceneLink::AwaitedServer() const
{
    if (!UI::LoginScene::IsWaitingForServer())
    {
        return std::nullopt;
    }

    return CurrentProtocolState == REQUEST_LOG_IN ? LoginServer::Game : LoginServer::Connect;
}

bool GameLoginSceneLink::IsServerListShown() const
{
    // Only the connect server answers a list request.
    return UI::LoginScene::IsServerListShown() && !g_bGameServerConnected;
}

bool GameLoginSceneLink::StartProbe(LoginServer server)
{
    const Address address = AddressOf(server);
    if (address.host == nullptr || address.host[0] == L'\0' || address.port == 0)
    {
        return false;
    }

    return m_probe.Start(address.host, address.port, WorldTime, ProbeTimeoutMs);
}

LoginSceneReconnectLink::ProbeResult GameLoginSceneLink::PollProbe()
{
    return m_probe.Poll(WorldTime);
}

void GameLoginSceneLink::StopProbe()
{
    m_probe.Stop();
}

bool GameLoginSceneLink::Connect(LoginServer server)
{
    const Address address = AddressOf(server);
    return ConnectForLogin(address.host, address.port) != FALSE;
}

void GameLoginSceneLink::SendServerListRequest()
{
    SocketClient->ToConnectServer()->SendServerListRequest();
}

void GameLoginSceneLink::ShowConnecting()
{
    // The texts a server pick shows while it connects.
    g_pSystemLogBox->AddText(I18N::Game::ConnectingToTheServer, mu::ui::window::TYPE_SYSTEM_MESSAGE);
    g_pSystemLogBox->AddText(I18N::Game::PleaseWait, mu::ui::window::TYPE_SYSTEM_MESSAGE);
}

void GameLoginSceneLink::ShowFailure()
{
    UI::LoginScene::ShowMessage(MESSAGE_SERVER_LOST_RECONNECT);
}

bool GameLoginSceneLink::IsFailureShown() const
{
    return UI::LoginScene::IsShowingMessage(MESSAGE_SERVER_LOST_RECONNECT);
}

void GameLoginSceneLink::HideFailure()
{
    if (IsFailureShown())
    {
        UI::LoginScene::CloseMessage();
    }
}

double GameLoginSceneLink::NowMs() const
{
    return WorldTime;
}

GameLoginSceneLink::Address GameLoginSceneLink::AddressOf(LoginServer server)
{
    if (server == LoginServer::Game)
    {
        const ReconnectManager& cache = ReconnectManager::Instance();
        return {cache.GetServerIp(), cache.GetServerPort()};
    }

    return {szServerIpAddress, g_ServerPort};
}
