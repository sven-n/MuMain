#pragma once

#include "Network/Reconnect/LoginSceneReconnectLink.h"
#include "Network/Reconnect/ReachabilityProbe.h"

// LoginSceneReconnect's link to the game: the client's connection (SocketClient),
// the login screen's windows and the addresses the client connected to.
class GameLoginSceneLink : public LoginSceneReconnectLink
{
public:
    bool IsConnected() const override;
    LoginServer ConnectedServer() const override;
    std::optional<LoginServer> AwaitedServer() const override;
    bool IsServerListShown() const override;
    bool StartProbe(LoginServer server) override;
    ProbeResult PollProbe() override;
    void StopProbe() override;
    bool Connect(LoginServer server) override;
    void SendServerListRequest() override;
    void ShowConnecting() override;
    void ShowFailure() override;
    bool IsFailureShown() const override;
    void HideFailure() override;
    double NowMs() const override;

private:
    struct Address
    {
        const wchar_t* host = nullptr;
        unsigned short port = 0;
    };

    static Address AddressOf(LoginServer server);

    ReachabilityProbe m_probe;
};
