// doctest unit tests for the game's side of the login screen's reconnect.
//
// GameLoginSceneLink reads the login scene's windows and protocol state to tell
// LoginSceneReconnect which server the screen waits for, and whether the server
// list is up. These cases set those globals the way the screen leaves them.
//
// Run: ctest --test-dir <build directory> --build-config Release -R "Game login scene link"

#include "stdafx.h"

#include "doctest.h"

#include "Network/Reconnect/GameLoginSceneLink.h"
#include "Network/Server/WSclient.h" // CurrentProtocolState and its states
#include "UI/Windows/CreditWin.h"
#include "UI/Windows/LoginWin.h"
#include "UI/Windows/MsgWin.h"
#include "UI/Windows/ServerSelWin.h"

extern BOOL g_bGameServerConnected;

namespace
{
// A login screen with all its windows hidden (they start out shown); puts the
// globals back afterwards.
class LoginScreen
{
public:
    LoginScreen()
    {
        HideWindows();
    }

    ~LoginScreen()
    {
        HideWindows();
        CurrentProtocolState = m_protocolState;
        g_bGameServerConnected = m_gameServerConnected;
    }

    LoginScreen(const LoginScreen&) = delete;
    LoginScreen& operator=(const LoginScreen&) = delete;

    GameLoginSceneLink link;

private:
    static void HideWindows()
    {
        g_ServerSelWin.Show(false);
        g_CreditWin.Show(false);
        g_LoginWin.Show(false);
        g_MsgWin.Show(false);
    }

    int m_protocolState = CurrentProtocolState;
    BOOL m_gameServerConnected = g_bGameServerConnected;
};
} // namespace

TEST_SUITE("Game login scene link")
{
    TEST_CASE("an empty screen waits for the server of the step it took")
    {
        LoginScreen screen;

        CurrentProtocolState = REQUEST_JOIN_SERVER; // before the first list, or after a server pick
        CHECK(screen.link.AwaitedServer() == LoginServer::Connect);

        CurrentProtocolState = REQUEST_LOG_IN; // the login went out
        CHECK(screen.link.AwaitedServer() == LoginServer::Game);
    }

    TEST_CASE("a screen with something to click waits for no server")
    {
        LoginScreen screen;
        CurrentProtocolState = REQUEST_LOG_IN;

        g_ServerSelWin.Show(true);
        CHECK_FALSE(screen.link.AwaitedServer().has_value());
        g_ServerSelWin.Show(false);

        g_LoginWin.Show(true);
        CHECK_FALSE(screen.link.AwaitedServer().has_value());
        g_LoginWin.Show(false);

        g_MsgWin.Show(true);
        CHECK_FALSE(screen.link.AwaitedServer().has_value());
    }

    TEST_CASE("the server list counts as shown only over the connect server")
    {
        LoginScreen screen;
        g_bGameServerConnected = FALSE;
        CHECK_FALSE(screen.link.IsServerListShown());

        g_ServerSelWin.Show(true);
        CHECK(screen.link.IsServerListShown());

        // A list left up while a game server is connected doesn't get refreshed:
        // the game server doesn't answer a list request.
        g_bGameServerConnected = TRUE;
        CHECK_FALSE(screen.link.IsServerListShown());
    }

    TEST_CASE("no failure message is up on a fresh screen")
    {
        LoginScreen screen;
        CHECK_FALSE(screen.link.IsFailureShown());
        screen.link.HideFailure(); // nothing to close
        CHECK_FALSE(g_MsgWin.IsVisible());
    }
}
