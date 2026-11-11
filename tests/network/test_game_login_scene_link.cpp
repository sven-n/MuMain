// doctest unit tests for the game's side of the login screen's reconnect and login-window input gate,
// and for the music the login screen plays.
//
// GameLoginSceneLink reads the login scene's windows and protocol state to tell
// LoginSceneReconnect which server the screen waits for, and whether the server
// list is up. These cases set those globals the way the screen leaves them.
//
// Run: ctest --test-dir <build directory> --build-config Release -R "Game login scene link"

#include "stdafx.h"

#include "doctest.h"

#include "Character/CharMakeWin.h"
#include "Core/Globals/_enum.h"
#include "Core/Input/Input.h"
#include "Core/Platform/IPlatformAudio.h"
#include "Network/Reconnect/GameLoginSceneLink.h"
#include "Network/Server/WSclient.h" // CurrentProtocolState and its states
#include "Scenes/LoginScene.h"
#include "UI/Core/SceneUICoordinator.h"
#include "UI/Core/WindowCommon.h"
#include "UI/Windows/CreditWin.h"
#include "UI/Windows/LoginMainWin.h"
#include "UI/Windows/LoginWin.h"
#include "UI/Windows/MsgWin.h"
#include "UI/Windows/RememberPasswordPrompt.h"
#include "UI/Windows/ServerSelWin.h"
#include "UI/Windows/SysMenuWin.h"

#include <array>
#include <string>
#include <vector>

extern BOOL g_bGameServerConnected;
extern bool g_bWndActive;

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

// An audio backend that keeps one music track the way the real one does: asking for the
// track that plays starts nothing, any other track replaces it from its start. Logs each start.
class MusicLog final : public mu::IPlatformAudio
{
public:
    std::vector<std::string> starts;

    bool Initialize() override
    {
        return true;
    }
    void Shutdown() override {}
    void LoadSound(ESound, const wchar_t*, int, bool) override {}
    bool PlaySound(ESound, const void*, bool) override
    {
        return true;
    }
    void StopSound(ESound, bool) override {}
    void AllStopSound() override {}
    void ReleaseSound(ESound) override {}
    void Set3DSoundPosition() override {}
    void SetVolume(ESound, long) override {}
    void SetMasterVolume(long) override {}
    void PlayMusic(const char* name, bool enforce) override
    {
        if (!enforce && m_track == name)
            return;
        m_track = name;
        starts.emplace_back(name);
    }
    void StopMusic(const char* name, bool enforce) override
    {
        if (enforce || name == nullptr || m_track == name)
            m_track.clear();
    }
    bool IsEndMusic() override
    {
        return m_track.empty();
    }
    int GetMusicPosition() override
    {
        return 0;
    }
    void SetBGMVolume(float) override {}
    void SetSFXVolume(float) override {}
    float GetBGMVolume() const override
    {
        return 1.0f;
    }
    float GetSFXVolume() const override
    {
        return 1.0f;
    }

private:
    std::string m_track;
};

// Puts a MusicLog in as the audio backend, with the music on; puts both back afterwards.
class MusicScope
{
public:
    MusicScope()
    {
        g_platformAudio = &log;
        m_MusicOnOff = 1;
    }

    ~MusicScope()
    {
        g_platformAudio = m_backend;
        m_MusicOnOff = m_musicOnOff;
    }

    MusicScope(const MusicScope&) = delete;
    MusicScope& operator=(const MusicScope&) = delete;

    MusicLog log;

private:
    mu::IPlatformAudio* m_backend = g_platformAudio;
    int m_musicOnOff = m_MusicOnOff;
};
} // namespace

TEST_SUITE("Game login scene link")
{
    TEST_CASE("hidden and released login windows do not handle keys")
    {
        UI::Login::ClearRememberPasswordChoice();
        REQUIRE_FALSE(CSceneUICoordinator::Instance().WasSysMenuToggledByEscThisFrame());

        // Only change visibility flags: opening real dialogs also changes input and preview state.
        struct Modals
        {
            std::array<mu::ui::window::CObject*, 4> windows{&g_CharMakeWin, &g_CreditWin, &g_MsgWin, &g_SysMenuWin};
            std::array<bool, 4> visible{};

            Modals()
            {
                for (std::size_t i = 0; i < windows.size(); ++i)
                {
                    visible[i] = windows[i]->IsVisible();
                    windows[i]->mu::ui::window::CObject::Show(false);
                }
            }

            ~Modals()
            {
                for (std::size_t i = 0; i < windows.size(); ++i)
                    windows[i]->mu::ui::window::CObject::Show(visible[i]);
            }
        } modals;

        class LoginWindow : public CLoginWin
        {
        public:
            int keyUpdates = 0;

        protected:
            bool UpdateWhileActive() override
            {
                ++keyUpdates;
                return true;
            }
        } window;

        window.Show(false);
        window.Update();
        CHECK(window.keyUpdates == 0);

        window.Show(true);
        window.Update();
        CHECK(window.keyUpdates == 1);

        int expectedKeyUpdates = 1;
        for (mu::ui::window::CObject* modal : modals.windows)
        {
            modal->mu::ui::window::CObject::Show(true);
            // Character creation owns a different scene, where login itself must be hidden.
            window.Show(modal != &g_CharMakeWin);
            window.Update();
            CHECK(window.keyUpdates == expectedKeyUpdates);

            modal->mu::ui::window::CObject::Show(false);
            window.Show(true);
            window.Update();
            ++expectedKeyUpdates;
            CHECK(window.keyUpdates == expectedKeyUpdates);
        }

        window.Show(false);
        window.Update();
        CHECK(window.keyUpdates == expectedKeyUpdates);

        window.Show(true);
        window.Release();
        CHECK_FALSE(window.IsVisible());
        window.Update();
        CHECK(window.keyUpdates == expectedKeyUpdates);
    }

    TEST_CASE("scene-window release clears the visible flag")
    {
        const bool loginMainWasVisible = g_LoginMainWin.IsVisible();
        const bool serverListWasVisible = g_ServerSelWin.IsVisible();
        const bool creditsWereVisible = g_CreditWin.IsVisible();

        g_LoginMainWin.mu::ui::window::CObject::Show(true);
        g_ServerSelWin.mu::ui::window::CObject::Show(true);
        g_CreditWin.mu::ui::window::CObject::Show(true);

        g_LoginMainWin.Release();
        g_ServerSelWin.Release();
        g_CreditWin.Release();

        CHECK_FALSE(g_LoginMainWin.IsVisible());
        CHECK_FALSE(g_ServerSelWin.IsVisible());
        CHECK_FALSE(g_CreditWin.IsVisible());

        g_LoginMainWin.mu::ui::window::CObject::Show(loginMainWasVisible);
        g_ServerSelWin.mu::ui::window::CObject::Show(serverListWasVisible);
        g_CreditWin.mu::ui::window::CObject::Show(creditsWereVisible);
    }

    TEST_CASE("hidden credits do not close on Escape")
    {
        struct CloseObserved
        {
        };

        class CreditWindow : public CCreditWin
        {
        public:
            void Show(bool show) override
            {
                if (!show)
                    throw CloseObserved{};
                CCreditWin::Show(show);
            }
        } window;

        struct InputStateGuard
        {
            using KeyState = mu::ui::window::CNewKeyInput::KEY_STATE;

            bool wasActive = g_bWndActive;
            bool wasTextEditMode = false;
            KeyState previousEscapeState = KeyState::KEY_NONE;

            InputStateGuard()
            {
                g_bWndActive = true;
                wasTextEditMode = CInput::Instance().IsTextEditMode();
                CInput::Instance().SetTextEditMode(false);
                auto* input = g_pNewKeyInput;
                if (input->IsPress(VK_ESCAPE))
                    previousEscapeState = KeyState::KEY_PRESS;
                else if (input->IsRepeat(VK_ESCAPE))
                    previousEscapeState = KeyState::KEY_REPEAT;
                else if (input->IsRelease(VK_ESCAPE))
                    previousEscapeState = KeyState::KEY_RELEASE;
                input->SetKeyState(VK_ESCAPE, KeyState::KEY_PRESS);
            }

            ~InputStateGuard()
            {
                g_pNewKeyInput->SetKeyState(VK_ESCAPE, previousEscapeState);
                CInput::Instance().SetTextEditMode(wasTextEditMode);
                g_bWndActive = wasActive;
            }
        } inputState;

        window.mu::ui::window::CObject::Show(false);
        REQUIRE_FALSE(window.IsVisible());
        REQUIRE(CInput::Instance().IsKeyDown(VK_ESCAPE));

        // The override throws before CloseWin can reach reconnect or music-switch side effects.
        bool updateResult = false;
        CHECK_NOTHROW(updateResult = window.Update());
        CHECK(updateResult);
        CHECK_FALSE(window.IsVisible());
        CHECK(CInput::Instance().IsKeyDown(VK_ESCAPE));
    }

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

    TEST_CASE("the connection goes to the server the last connect named")
    {
        LoginScreen screen;
        g_bGameServerConnected = FALSE;
        CHECK(screen.link.ConnectedServer() == LoginServer::Connect);

        g_bGameServerConnected = TRUE; // a server pick or ConnectForLogin to the game server
        CHECK(screen.link.ConnectedServer() == LoginServer::Game);
    }

    TEST_CASE("a game server is told by its port")
    {
        // ConnectForLogin names the server it connected to by the port, also for a client
        // started straight on a game server.
        CHECK_FALSE(IsGameServerPort(44405)); // OpenMU's connect server
        CHECK_FALSE(IsGameServerPort(0xAD00));
        CHECK_FALSE(IsGameServerPort(0xADFF));
        CHECK(IsGameServerPort(0xAD00 - 1));
        CHECK(IsGameServerPort(0xADFF + 1));
        CHECK(IsGameServerPort(55901)); // OpenMU's first game server
    }

    TEST_CASE("no failure message is up on a fresh screen")
    {
        LoginScreen screen;
        CHECK_FALSE(screen.link.IsFailureShown());
        screen.link.HideFailure(); // nothing to close
        CHECK_FALSE(g_MsgWin.IsVisible());
    }

    TEST_CASE("the credits play their own theme, and the login theme comes back when they close")
    {
        LoginScreen screen;
        MusicScope music;

        // The scene asks for its track every frame; two frames per screen.
        PlayLogInSceneMusic();
        PlayLogInSceneMusic();
        g_CreditWin.mu::ui::window::CObject::Show(true);
        PlayLogInSceneMusic();
        PlayLogInSceneMusic();
        g_CreditWin.mu::ui::window::CObject::Show(false);
        PlayLogInSceneMusic();
        PlayLogInSceneMusic();

        // One start per screen: no track starts over while its screen is up.
        const std::vector<std::string> expected{MUSIC_LOGIN_THEME, MUSIC_MUTHEME, MUSIC_LOGIN_THEME};
        CHECK(music.log.starts == expected);
    }

    TEST_CASE("a login that goes through with the credits open takes the login theme to the character select")
    {
        LoginScreen screen;
        MusicScope music;

        // The login was sent, and the credits were opened before its answer came.
        CurrentProtocolState = REQUEST_LOG_IN;
        g_CreditWin.mu::ui::window::CObject::Show(true);
        PlayLogInSceneMusic();
        PlayLogInSceneMusic();

        // The answer: this frame leaves for the character select, which asks for no music of its own and
        // hides the credits. The loading screen after it stops only the login theme.
        CurrentProtocolState = RECEIVE_LOG_IN_SUCCESS;
        PlayLogInSceneMusic();

        const std::vector<std::string> expected{MUSIC_MUTHEME, MUSIC_LOGIN_THEME};
        CHECK(music.log.starts == expected);
    }
}
