#include "stdafx.h"
#include "App/Control/ControlCommands.h"

#include "App/Control/ControlEvents.h"
#include "App/Platform/Windows/Winmain.h"
#include "Core/Text/Utf8.h"
#include "Network/Server/ServerListManager.h"
#include "Engine/Object/ZzzOpenData.h"
#include "MUHelper/MuHelper.h"
#include "Network/Server/WSclient.h"
#include "Scenes/CharacterScene.h"
#include "Scenes/SceneCore.h"
#include "UI/Legacy/UIMng.h"
#include "UI/NewUI/NewUISystem.h"

#include "json.hpp"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstddef>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

extern int LoadingWorld;
extern bool LogOut;

// Whether the game server connection is up; the client's own exit path
// tests it before saying goodbye (Winmain.cpp:145).
extern "C++" BOOL g_bGameServerConnected;

namespace
{
using App::Control::Act;
using App::Control::ErrorCode;
using App::Control::Request;
using nlohmann::json;

// The server list, the game-server handover and the account check are three
// round trips; a minute covers a slow stack without hanging a scenario.
constexpr std::chrono::milliseconds LoginDeadline{60000};
// Entering the world loads a map from disk.
constexpr std::chrono::milliseconds SelectCharacterDeadline{60000};
constexpr std::chrono::milliseconds LogoutDeadline{30000};

// Longest credential the client's own fields accept, in wide characters.
// A caller is told when it exceeds them rather than having its account
// silently truncated to something that will not log in.
bool CredentialFits(const std::string& text, std::size_t limit)
{
    return Core::Text::FromUtf8(text).size() <= limit;
}

// Seeded test accounts use the account name as the password; the spec makes
// that the default so `login test1` is enough.
// Empty when the field is present but not a string: the caller meant a
// password and must be told, not quietly logged in with the account name.
std::optional<std::string> PasswordOr(const Request& request, const std::string& account)
{
    if (!request.Has("password"))
    {
        return account;
    }

    std::string password;
    if (!request.GetString("password", password) || password.empty())
    {
        return std::nullopt;
    }
    return password;
}

// The character list as `login` and `select-char` report it.
json CharacterList()
{
    json characters = json::array();
    for (int slot = 0; slot < MAX_CHARACTERS_PER_ACCOUNT; ++slot)
    {
        const wchar_t* name = Scenes::CharacterNameInSlot(slot);
        if (name[0] == L'\0')
        {
            continue;
        }

        json character;
        // Slots are reported as `select-char --slot` counts them, from 1.
        character["slot"] = slot + 1;
        character["name"] = Core::Text::ToUtf8(name);
        character["level"] = Scenes::CharacterLevelInSlot(slot);
        characters.push_back(std::move(character));
    }
    return characters;
}

// Account names are matched case-insensitively, as the server matches them:
// a `login TEST1` while `test1` is signed in is the same session, not a
// different account that has to be logged out first.
bool SameAccount(std::string_view left, std::string_view right)
{
    return std::equal(left.begin(), left.end(), right.begin(), right.end(),
                      [](unsigned char a, unsigned char b) { return std::tolower(a) == std::tolower(b); });
}

// Login failures reach the client only as a message box (design.md, D5):
// read its code, dismiss it so the login screen is usable again, and turn
// it into an error response.
bool TakeLoginFailure(std::string& reason)
{
    CMsgWin& messageWindow = CUIMng::Instance().m_MsgWin;
    const int code = messageWindow.PendingMessageCode();
    if (!App::Control::IsLoginFailureCode(code))
    {
        return false;
    }

    reason = App::Control::LoginFailureReason(code);
    if (!messageWindow.DismissMessage())
    {
        // The box is one whose confirmation would close the client, so it
        // stays on screen; the caller still gets the reason.
        reason += " (the client cannot continue from this one)";
    }
    return true;
}

// Whether the connect server's list holds a group of that name.
bool ServerGroupExists(const std::wstring& name)
{
    CServerGroup* group = nullptr;
    g_ServerListManager->SetFirst();
    while (g_ServerListManager->GetNext(group))
    {
        if (group != nullptr && wcscmp(group->m_szName, name.c_str()) == 0)
        {
            return true;
        }
    }
    return false;
}

// How long a server selection that fails is retried before it is answered:
// long enough for the connect server's list to arrive, short enough not to
// spend the login's whole deadline on a selection that cannot succeed.
constexpr std::chrono::milliseconds SelectServerRetryWindow{5000};

// login: server list -> server -> credentials -> character list.
class LoginAct : public Act
{
public:
    LoginAct(std::string account, std::string password, std::wstring serverGroup)
        : m_account(std::move(account)), m_password(std::move(password)), m_serverGroup(std::move(serverGroup))
    {
    }

    [[nodiscard]] std::string_view Name() const override
    {
        return "login";
    }
    [[nodiscard]] bool ChangesScene() const override
    {
        return true;
    }
    [[nodiscard]] std::optional<std::chrono::milliseconds> Deadline() const override
    {
        return LoginDeadline;
    }

    [[nodiscard]] std::string ProgressObject() const override
    {
        json progress;
        progress["stage"] = StageName();
        progress["scene"] = App::Control::Commands::CurrentSceneName();
        return progress.dump();
    }

    [[nodiscard]] Status Tick(std::string& response) override
    {
        switch (m_stage)
        {
        case Stage::LeavingSession:
            return TickLeavingSession(response);
        case Stage::SelectingServer:
            return TickSelectingServer(response);
        case Stage::JoiningServer:
            return TickJoiningServer(response);
        case Stage::SubmittingCredentials:
            return TickSubmittingCredentials(response);
        }
        return Status::Running;
    }

private:
    enum class Stage : std::uint8_t
    {
        LeavingSession,
        SelectingServer,
        JoiningServer,
        SubmittingCredentials,
    };

    [[nodiscard]] std::string_view StageName() const
    {
        switch (m_stage)
        {
        case Stage::LeavingSession:
            return "leaving_session";
        case Stage::SelectingServer:
            return "selecting_server";
        case Stage::JoiningServer:
            return "joining_server";
        case Stage::SubmittingCredentials:
            return "submitting_credentials";
        }
        return "unknown";
    }

    Status Fail(std::string& response, ErrorCode code, const std::string& message)
    {
        App::Control::Events::RecordError("login", App::Control::ErrorCodeName(code), message);
        response = App::Control::EncodeError(EncodedId(), code, message, ProgressObject());
        return Status::Finished;
    }

    // Another account is already logged in on this client: leave that
    // session the way the in-game menu does before logging the new one in.
    Status TickLeavingSession(std::string& response)
    {
        if (!m_leaving)
        {
            if (SameAccount(Core::Text::ToUtf8(LogInID), m_account))
            {
                // The account asked for is the one already logged in.
                return Answer(response);
            }

            LogOut = true;
            SocketClient->ToGameServer()->SendLogOut(LogOutType::BackToServerSelection);
            m_leaving = true;
            return Status::Running;
        }

        if (CurrentProtocolState >= RECEIVE_CHARACTERS_LIST)
        {
            return Status::Running;
        }

        m_stage = Stage::SelectingServer;
        return Status::Running;
    }

    Status TickSelectingServer(std::string& response)
    {
        // A session of another account is still open on this client.
        if (CurrentProtocolState >= RECEIVE_CHARACTERS_LIST)
        {
            m_stage = Stage::LeavingSession;
            return Status::Running;
        }

        // Already past server selection (a previous login on this process).
        if (CurrentProtocolState >= RECEIVE_JOIN_SERVER_SUCCESS)
        {
            m_stage = Stage::JoiningServer;
            return Status::Running;
        }

        // A client that has just started is still asking the connect
        // server for its list; wait for it rather than refusing.
        if (g_ServerListManager->GetServerGroupSize() < 1)
        {
            return Status::Running;
        }

        // A group the caller named that the list does not hold will never
        // appear: that is an answer, not something to wait for.
        if (!m_serverGroup.empty() && !ServerGroupExists(m_serverGroup))
        {
            return Fail(response, ErrorCode::NotConnected, "no server group named as asked for");
        }

        CUIMng& uiManager = CUIMng::Instance();
        if (!uiManager.m_ServerSelWin.SelectServer(m_serverGroup.c_str(), m_serverIndex))
        {
            // Everything else may be a "not yet": the list may still be
            // filling in, and the connection this login had to tear down
            // first may still be coming back. Retried for a few seconds and
            // then answered — a selection that cannot succeed (a group the
            // display could not place, a server the group does not hold)
            // would otherwise sit out the whole login deadline.
            if (m_selectingSince == std::chrono::steady_clock::time_point{})
            {
                m_selectingSince = std::chrono::steady_clock::now();
            }
            if (std::chrono::steady_clock::now() - m_selectingSince < SelectServerRetryWindow)
            {
                return Status::Running;
            }
            return Fail(response, ErrorCode::NotConnected,
                        m_serverGroup.empty() ? "the server list holds no server this client may join"
                                              : "that server group holds no server this client may join");
        }

        m_stage = Stage::JoiningServer;
        return Status::Running;
    }

    Status TickJoiningServer(std::string& response)
    {
        std::string reason;
        if (TakeLoginFailure(reason))
        {
            return Fail(response, ErrorCode::LoginFailed, reason);
        }

        // ReceiveJoinServer sets this once the game server accepted the
        // connection and the login window is up, and it is the one state in
        // which CLoginWin::SubmitCredentials does anything (LoginWin.cpp:399)
        // — submitting in any other silently returns and the act would then
        // wait out its deadline. A state left behind by an earlier failed
        // login is answered as that failure instead of waited on.
        if (CurrentProtocolState != RECEIVE_JOIN_SERVER_SUCCESS)
        {
            // Every login failure arrives as a message box, which the stage
            // below reads; the protocol state itself is put back to
            // RECEIVE_JOIN_SERVER_SUCCESS by the box's OK handler
            // (MsgWin.cpp:495), so waiting here is waiting for the window
            // that accepts credentials.
            return Status::Running;
        }

        CUIMng::Instance().m_LoginWin.SubmitCredentials(Core::Text::FromUtf8(m_account).c_str(),
                                                        Core::Text::FromUtf8(m_password).c_str());
        m_stage = Stage::SubmittingCredentials;
        return Status::Running;
    }

    Status TickSubmittingCredentials(std::string& response)
    {
        std::string reason;
        if (TakeLoginFailure(reason))
        {
            return Fail(response, ErrorCode::LoginFailed, reason);
        }

        // At or past the list: the only state beyond it is entering the
        // world, and answering with the list the client holds is right
        // either way. Waiting for the exact value would hang if the client
        // moved on between two polls.
        if (CurrentProtocolState < RECEIVE_CHARACTERS_LIST)
        {
            return Status::Running;
        }

        return Answer(response);
    }

    Status Answer(std::string& response)
    {
        json result;
        result["account"] = Core::Text::ToUtf8(LogInID);
        result["scene"] = App::Control::Commands::CurrentSceneName();
        result["characters"] = CharacterList();
        response = App::Control::EncodeResult(EncodedId(), result.dump());
        return Status::Finished;
    }

    std::string m_account;
    std::string m_password;
    std::wstring m_serverGroup;
    // The first server of the group; MU's own list is one server per group
    // in this deployment.
    int m_serverIndex = 0;
    Stage m_stage = Stage::SelectingServer;
    bool m_leaving = false;
    std::chrono::steady_clock::time_point m_selectingSince{};
};

// select-char: enter the world with one character and wait for its map.
class SelectCharacterAct : public Act
{
public:
    explicit SelectCharacterAct(int slot) : m_slot(slot) {}

    [[nodiscard]] std::string_view Name() const override
    {
        return "select-char";
    }
    [[nodiscard]] bool ChangesScene() const override
    {
        return true;
    }
    [[nodiscard]] std::optional<std::chrono::milliseconds> Deadline() const override
    {
        return SelectCharacterDeadline;
    }

    [[nodiscard]] std::string ProgressObject() const override
    {
        json progress;
        progress["slot"] = m_slot + 1;
        progress["scene"] = App::Control::Commands::CurrentSceneName();
        return progress.dump();
    }

    [[nodiscard]] Status Tick(std::string& response) override
    {
        if (!m_started)
        {
            if (!Scenes::StartGameWithSlot(m_slot))
            {
                response = App::Control::EncodeError(EncodedId(), ErrorCode::NoSuchCharacter,
                                                     "slot " + std::to_string(m_slot + 1) + " holds no character",
                                                     ProgressObject());
                return Status::Finished;
            }
            m_started = true;
            return Status::Running;
        }

        // The world is entered once the main scene is up and the loading
        // handshake has finished.
        if (SceneFlag != MAIN_SCENE || LoadingWorld != 0 || Hero == nullptr)
        {
            return Status::Running;
        }

        json result;
        result["scene"] = App::Control::Commands::CurrentSceneName();
        result["slot"] = m_slot + 1;
        result["character"] = Core::Text::ToUtf8(Hero->ID);
        result["map"] = gMapManager.WorldActive;
        result["position"] = json::array({Hero->PositionX, Hero->PositionY});
        response = App::Control::EncodeResult(EncodedId(), result.dump());
        return Status::Finished;
    }

private:
    int m_slot;
    bool m_started = false;
};

// logout: the game's own "back to character selection".
class LogoutAct : public Act
{
public:
    [[nodiscard]] std::string_view Name() const override
    {
        return "logout";
    }
    [[nodiscard]] bool ChangesScene() const override
    {
        return true;
    }
    [[nodiscard]] std::optional<std::chrono::milliseconds> Deadline() const override
    {
        return LogoutDeadline;
    }

    [[nodiscard]] std::string ProgressObject() const override
    {
        json progress;
        progress["scene"] = App::Control::Commands::CurrentSceneName();
        return progress.dump();
    }

    [[nodiscard]] Status Tick(std::string& response) override
    {
        if (!m_sent)
        {
            // What the client's own "back to character select" does
            // (NewUICustomMessageBox.cpp:2397-2409): save what the session
            // changed, refuse while the chaos machine is open — it eats the
            // items in it — and stop the helper before leaving.
            if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MIXINVENTORY))
            {
                response = App::Control::EncodeError(EncodedId(), ErrorCode::NotAllowed,
                                                     "close the chaos machine window first", ProgressObject());
                return Status::Finished;
            }

            SaveOptions();
            SaveMacro(L"Data\\Macro.txt");
            MUHelper::g_MuHelper.TriggerStop();
            g_pNewUIMng->ResetActiveUIObj();

            LogOut = true;
            SocketClient->ToGameServer()->SendLogOut(LogOutType::BackToCharacterSelection);
            m_sent = true;
            return Status::Running;
        }

        // The character list is only trustworthy once the server has sent
        // it; before that the client's object table still holds the world.
        if (SceneFlag != CHARACTER_SCENE || CurrentProtocolState != RECEIVE_CHARACTERS_LIST)
        {
            return Status::Running;
        }

        json result;
        result["scene"] = App::Control::Commands::CurrentSceneName();
        result["characters"] = CharacterList();
        response = App::Control::EncodeResult(EncodedId(), result.dump());
        return Status::Finished;
    }

private:
    bool m_sent = false;
};
} // namespace

// Whether the game server connection is up; the client's own exit path
// tests it before saying goodbye (Winmain.cpp:145).
namespace App::Control::Commands
{
std::string Login(const Request& request, std::unique_ptr<Act>& act)
{
    std::string account;
    if (!request.GetString("account", account) || account.empty())
    {
        return EncodeError(request.EncodedId(), ErrorCode::BadRequest, "`login` needs an account");
    }

    if (!CredentialFits(account, MAX_USERNAME_SIZE))
    {
        return EncodeError(request.EncodedId(), ErrorCode::BadRequest,
                           "the account is longer than " + std::to_string(MAX_USERNAME_SIZE) + " characters");
    }

    const std::optional<std::string> given = PasswordOr(request, account);
    if (!given.has_value())
    {
        return EncodeError(request.EncodedId(), ErrorCode::BadRequest,
                           "`password` is a non-empty string; omit it to use the account name");
    }

    const std::string& password = *given;
    if (!CredentialFits(password, MAX_PASSWORD_SIZE))
    {
        return EncodeError(request.EncodedId(), ErrorCode::BadRequest,
                           "the password is longer than " + std::to_string(MAX_PASSWORD_SIZE) + " characters");
    }

    std::string serverGroup;
    if (request.Has("server") && !request.GetString("server", serverGroup))
    {
        return EncodeError(request.EncodedId(), ErrorCode::BadRequest,
                           "`server` is the name of a server group; omit it for the first one");
    }

    act = std::make_unique<LoginAct>(account, password, Core::Text::FromUtf8(serverGroup));
    return {};
}

std::string SelectCharacter(const Request& request, std::unique_ptr<Act>& act)
{
    if (CurrentProtocolState < RECEIVE_CHARACTERS_LIST)
    {
        return EncodeError(request.EncodedId(), ErrorCode::WrongScene, "no character list yet: log in first");
    }

    int slot = -1;
    std::string name;
    if (request.Has("slot"))
    {
        if (!request.GetInt("slot", slot))
        {
            return EncodeError(request.EncodedId(), ErrorCode::BadRequest, "`slot` is a whole number from 1");
        }

        // The spec counts slots as the list shows them, from 1. Checked
        // before the conversion: `slot` carries anything an int holds, and
        // decrementing its minimum first would be signed overflow.
        // Against the table's size, not the number of characters in it:
        // CharacterList() numbers slots by their table index, so a list with
        // a gap in it would otherwise refuse a slot it just advertised. The
        // emptiness of the slot is what the check below answers.
        if (slot < 1 || slot > MAX_CHARACTERS_PER_ACCOUNT)
        {
            json details;
            details["characters"] = CharacterList();
            return EncodeError(request.EncodedId(), ErrorCode::BadRequest, "no character in that slot", details.dump());
        }
        slot -= 1;
    }
    else if (request.GetString("name", name) && !name.empty())
    {
        slot = Scenes::FindCharacterSlot(Core::Text::FromUtf8(name).c_str());
    }
    else
    {
        return EncodeError(request.EncodedId(), ErrorCode::BadRequest,
                           "`select-char` needs a character name or a slot");
    }

    if (slot < 0 || Scenes::CharacterNameInSlot(slot)[0] == L'\0')
    {
        json details;
        details["characters"] = CharacterList();
        return EncodeError(request.EncodedId(), ErrorCode::NoSuchCharacter,
                           name.empty() ? "no character in that slot" : "no character named `" + name + "`",
                           details.dump());
    }

    act = std::make_unique<SelectCharacterAct>(slot);
    return {};
}

std::string Logout(const Request& request, std::unique_ptr<Act>& act)
{
    (void)request;
    act = std::make_unique<LogoutAct>();
    return {};
}

std::string Quit(const Request& request, std::unique_ptr<Act>&)
{
    // The exit handler's own steps (NewUICustomMessageBox.cpp:2334-2345):
    // save what the session changed, refuse while the chaos machine holds
    // items, stop the helper, and tell the server — so the account is free
    // at once instead of after its timeout.
    if (g_pNewUISystem->IsVisible(SEASON3B::INTERFACE_MIXINVENTORY))
    {
        return EncodeError(request.EncodedId(), ErrorCode::NotAllowed, "close the chaos machine window first");
    }

    // The save sends the key configuration to the server, so it belongs
    // inside the same test as the goodbye: `quit` is answered on every
    // scene, including those where there is no connection at all.
    if (SocketClient != nullptr && g_bGameServerConnected)
    {
        SaveOptions();
        SaveMacro(L"Data\\Macro.txt");
        MUHelper::g_MuHelper.TriggerStop();

        LogOut = true;
        SocketClient->ToGameServer()->SendLogOut(LogOutType::CloseGame);
    }

    // The main loop leaves on the next frame and ShutdownRuntime unlinks
    // the socket, so the caller gets its answer before the client goes.
    Destroy = true;

    json result;
    result["quitting"] = true;
    return EncodeResult(request.EncodedId(), result.dump());
}
} // namespace App::Control::Commands
