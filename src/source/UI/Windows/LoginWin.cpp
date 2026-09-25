//*****************************************************************************
// File: LoginWin.cpp
//*****************************************************************************

#include "stdafx.h"
#include "UI/Windows/LoginWin.h"
#include "Core/Input/Input.h"
#include "UI/Core/SceneUICoordinator.h"
#include "UI/Windows/CreditWin.h"
#include "UI/Windows/SysMenuWin.h"
#include "UI/Windows/MsgWin.h"
#include "Character/CharMakeWin.h"
#include "Core/Globals/_enum.h"
#include "Render/Models/ZzzBMD.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInterface.h"
#include "Network/Reconnect/ReconnectManager.h"
#include "Scenes/SceneCore.h"
#include "I18N/All.h"

#include "Audio/DSPlaySound.h"
#include "UI/Core/WindowSystem.h"
#include "UI/Dialogs/MessageBox.h"
#include "UI/Windows/RememberPasswordPrompt.h"

#include "Network/Server/ServerListManager.h"
#ifdef _WIN32
#include <dpapi.h>
#endif

#include "Data/GameConfig/GameConfig.h"
#include "Data/GameConfig/GameConfigConstants.h"

#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlDocumentVisibility.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/Scaling/UITransform.h"
#include "Core/Utilities/StringUtils.h"
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Elements/ElementFormControlInput.h>
#include <RmlUi/Core/Event.h>
#include <cmath>

extern unsigned int WindowWidth, WindowHeight;

namespace
{
    // Scales fixed reference-pixel offsets (legacy bounding box, CUITextInputBox placement) to stay
    // aligned with login.rcss's dp values. Reads WindowWidth/WindowHeight, not
    // CInput::Instance().GetScreenWidth/Height() -- the latter doesn't reliably match the values
    // RmlUiRuntime::OnResize() uses, which caused the panel/input box to visibly drift off-position.
    float LoginUIScaleRatio()
    {
        return UI::Scaling::CompanionRatio(static_cast<int>(WindowWidth), static_cast<int>(WindowHeight));
    }

    int ScaledOffset(int value, float ratio)
    {
        return static_cast<int>(std::lround(value * ratio));
    }
}

extern int g_iChatInputType;
extern int  LogIn;
extern wchar_t LogInID[MAX_USERNAME_SIZE + 1];
extern BYTE Version[SIZE_PROTOCOLVERSION];
extern BYTE Serial[SIZE_PROTOCOLSERIAL + 1];

CLoginWin g_LoginWin;

CLoginWin::CLoginWin() = default;

CLoginWin::~CLoginWin()
{
    // Backstop join: std::thread's destructor calls std::terminate() if still joinable, and
    // there's no guarantee ProcessPendingConnectionReconnect() ran one last time before shutdown.
    if (m_ConnectionReconnectThread.joinable())
        m_ConnectionReconnectThread.join();
}

void CLoginWin::Create()
{
    m_RememberMe = GameConfig::GetInstance().GetRememberMe();
    if (m_RememberMe)
    {
        // Use the helper we built to fill m_Username[11] and m_Password[21]
        GameConfig::GetInstance().DecryptCredentials(m_Username, m_Password, _countof(m_Username), _countof(m_Password));
    }
    else
    {
        // Ensure they are empty if RememberMe is off
        m_Username[0] = L'\0';
        m_Password[0] = L'\0';
    }

    // Tracks login.rcss's #panel width/height (329dp/245dp) by the same ratio rather than a fixed
    // 329x245, so UpdateMouseEvent()'s hit-test rect (m_Size) doesn't go stale against RmlUi's
    // auto-fitting visuals.
    const float uiScale = LoginUIScaleRatio();
    m_Size.cx = ScaledOffset(329, uiScale);
    m_Size.cy = ScaledOffset(245, uiScale);
    m_ptPos.x = m_ptPos.y = 0;

    // Tab order between the two fields is RmlUi's own document-level navigation (ElementDocument
    // handles KI_TAB, and WidgetTextInput deliberately lets it bubble), replacing the native boxes'
    // reciprocal SetTabTarget() pair.
    auto& model = m_RmlBinder.GetModel();
    model.username.clear();
    model.password.clear();
    if (m_RememberMe) {
        model.username = StringUtils::WideToNarrow(m_Username);
        model.password = StringUtils::WideToNarrow(m_Password);
        m_bRememberMeChecked = true;
    }

    // The password is only pre-filled and re-saved when the player previously
    // opted in on a trusted machine.
    m_bSavePasswordChecked = (m_RememberMe != 0) && GameConfig::GetInstance().GetSavePassword();

    // Seed the edit-detection snapshot with what we just loaded so filling the
    // fields here is not mistaken for the player editing them.
    wcsncpy(m_prevUsername, m_RememberMe ? m_Username : L"", _countof(m_prevUsername) - 1);
    wcsncpy(m_prevPassword, m_RememberMe ? m_Password : L"", _countof(m_prevPassword) - 1);

    this->FirstLoad = 1;

    // Guarded on m_pRmlDoc: Create() re-runs on every resolution change, but the RmlUi
    // document/model are set up once and only repositioned afterward (see SetPosition()).
    if (!m_pRmlDoc && RmlUiRuntime::Instance().IsCreated())
    {
        BuildRmlUi();
        UI::RmlBridge::RegisterForThemeReload(this, [this] { ReloadRmlTheme(); });
    }

    CSceneUICoordinator::Instance().GetNewStyleMng().AddUIObj(mu::ui::window::INTERFACE_LOGIN, this);
    Show(false);
}

void CLoginWin::BuildRmlUi()
{
    // The data model must exist before the document is loaded -- RmlUi resolves data-model
    // bindings while parsing the RML, so a model created after LoadDocument() renders every
    // {{...}} as literal text instead of its bound value.
    const bool modelCreated = m_RmlBinder.Create(RmlUiRuntime::Instance().GetContext(), "login",
        [this](Rml::DataModelConstructor& c, LoginRmlModel& model)
        {
            c.Bind("remember_me_checked", &model.rememberMeChecked);
            c.Bind("save_password_checked", &model.savePasswordChecked);
            c.Bind("server_name", &model.serverName);
            c.Bind("account_label", &model.accountLabel);
            c.Bind("password_label", &model.passwordLabel);
            c.Bind("remember_me_label", &model.rememberMeLabel);
            c.Bind("save_password_label", &model.savePasswordLabel);
            c.Bind("trust_warning", &model.trustWarning);
            c.Bind("ok_label", &model.okLabel);
            c.Bind("cancel_label", &model.cancelLabel);
            c.Bind("username", &model.username);
            c.Bind("password", &model.password);

            c.BindEventCallback("login_ok_click",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlClickOk(); });
            c.BindEventCallback("login_cancel_click",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlClickCancel(); });
            c.BindEventCallback("login_toggle_remember_me",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlToggleRememberMe(); });
            c.BindEventCallback("login_toggle_save_password",
                [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlToggleSavePassword(); });
        });

    // Routed through LoadThemedDocument so this resolves against the active theme's stylesheet.
    if (modelCreated)
    {
        m_pRmlDoc = UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(), "Data/Interface/RmlUi/login.rml");
        ApplyCredentialLimits();
    }

    // Deliberately NOT calling UI::RmlBridge::MakeDraggable() here -- the login screen is
    // meant to stay static.
}

void CLoginWin::ReloadRmlTheme()
{
    if (!m_pRmlDoc) return; // never opened; BuildRmlUi() will pick up the new theme later

    // The document's own visibility, not CObject::IsVisible() -- Release() hides m_pRmlDoc
    // directly, so the latter goes stale across scene transitions.
    const bool wasVisible = m_pRmlDoc->IsVisible();
    Rml::Context* context = RmlUiRuntime::Instance().GetContext();
    m_RmlBinder.Destroy(context);
    context->UnloadDocument(m_pRmlDoc);
    m_pRmlDoc = nullptr;

    BuildRmlUi();
    SetPosition(m_ptPos.x, m_ptPos.y);
    if (wasVisible) { SyncRmlModel(); if (m_pRmlDoc) m_pRmlDoc->Show(); }
}

void CLoginWin::SetPosition(int x, int y)
{
	m_ptPos.x = x;
	m_ptPos.y = y;

	// No field repositioning here: both credential fields are RmlUi elements positioned by each
	// theme's RCSS, so moving #panel carries them along.

	// RmlUi panel origin: real window pixels, no scale conversion needed (RmlUi's Context already
	// operates in real pixels; only the panel's own size/children are dp, scaled by RmlUi itself).
	if (m_pRmlDoc)
	{
		if (Rml::Element* panel = m_pRmlDoc->GetElementById("panel"))
		{
			panel->SetProperty("left", std::to_string(x) + "px");
			panel->SetProperty("top", std::to_string(y) + "px");
		}
	}
}

void CLoginWin::Show(bool bShow)
{
    mu::ui::window::CObject::Show(bShow);

    // Hiding the document unfocuses it (Context::UnfocusDocument()), which is what releases SDL
    // text input now that the fields are RmlUi's -- no separate widget state to drive (#447).
    if (m_pRmlDoc)
    {
        if (bShow) { SyncRmlModel(); m_pRmlDoc->Show(); }
        else       m_pRmlDoc->Hide();
    }
}

void CLoginWin::Release()
{
    // Hide, not unload -- the document/model are created once and reused.
    if (m_pRmlDoc)
        m_pRmlDoc->Hide();
}

void CLoginWin::RmlClickOk() { SubmitLogin(); }
void CLoginWin::RmlClickCancel() { SubmitCancel(); }

void CLoginWin::RmlToggleRememberMe()
{
	m_bRememberMeChecked = !m_bRememberMeChecked;
	if (UI::Login::RememberPasswordChoiceState() != UI::Login::RememberPasswordChoice::Pending)
		ApplyRememberMeChange();
}

void CLoginWin::RmlToggleSavePassword()
{
	m_bSavePasswordChecked = !m_bSavePasswordChecked;
	if (UI::Login::RememberPasswordChoiceState() != UI::Login::RememberPasswordChoice::Pending)
		ApplySavePasswordChange();
}

bool CLoginWin::UpdateWhileActive()
{
	if (CInput::Instance().IsKeyDown(VK_RETURN))
	{
		SubmitLogin();
		return true;
	}

	if (CInput::Instance().IsKeyDown(VK_ESCAPE))
	{
		SubmitCancel();
		return true;
	}

	return true;
}

// Called both from the immediate RmlUi click callbacks and from UpdateWhileActive()'s keyboard
// polling, so each re-checks the "remember password" prompt's Pending state itself.
void CLoginWin::SubmitLogin()
{
	if (UI::Login::RememberPasswordChoiceState() == UI::Login::RememberPasswordChoice::Pending)
		return;
	PlayBuffer(SOUND_CLICK01);
	RequestLogin();
}

void CLoginWin::SubmitCancel()
{
	if (UI::Login::RememberPasswordChoiceState() == UI::Login::RememberPasswordChoice::Pending)
		return;
	PlayBuffer(SOUND_CLICK01);
	CancelLogin();
}

void CLoginWin::ApplyRememberMeChange()
{
	GameConfig& config = GameConfig::GetInstance();
	m_RememberMe = m_bRememberMeChecked;
	config.SetRememberMe(m_RememberMe != 0);

	// Switching off "remember me" revokes everything: drop the stored
	// credentials from config.ini now so they can't linger if the game is
	// closed before the next login.
	if (!m_RememberMe)
	{
		m_bSavePasswordChecked = false;
		config.ClearCredentials();
	}
}

void CLoginWin::ApplySavePasswordChange()
{
	GameConfig& config = GameConfig::GetInstance();

	if (!m_bSavePasswordChecked)
	{
		// Player unticked it: drop the stored password from config.ini now.
		config.SetSavePassword(false);
		config.SetEncryptedPassword(L"");
		config.Save();
		return;
	}

	// Enabling requires confirmation. Revert the tick immediately; it is
	// re-applied only if the dialog is accepted, so a cancel (however the dialog
	// closes) always leaves the box unchecked.
	m_bSavePasswordChecked = false;

	// Storing the password implies remembering the account.
	if (!m_RememberMe)
	{
		m_RememberMe = 1;
		m_bRememberMeChecked = true;
		config.SetRememberMe(true);
	}
	UI::Login::OpenRememberPasswordPrompt();
}

bool CLoginWin::UpdateWhileShown()
{
    // Prevents Enter/Esc from double-firing past an overlaying modal: a higher-depth modal
    // claiming UpdateMouseEvent() doesn't by itself stop this window's own VK_RETURN/VK_ESCAPE
    // polling in UpdateWhileActive(). WasSysMenuToggledByEscThisFrame() is needed in addition to
    // g_SysMenuWin.IsVisible() because an Esc that just closed the menu this frame already reads
    // back IsVisible()==false. g_CharMakeWin.IsVisible() covers the same gap: this window keeps
    // ticking on the character-select scene too, so without it, Esc closing Character Create also
    // fires this window's own SubmitCancel() and logs out unexpectedly.
    SetActive(!(g_CreditWin.IsVisible() || g_MsgWin.IsVisible() || g_SysMenuWin.IsVisible()
                || g_CharMakeWin.IsVisible()
                || CSceneUICoordinator::Instance().WasSysMenuToggledByEscThisFrame()
                || UI::Login::RememberPasswordChoiceState() == UI::Login::RememberPasswordChoice::Pending));

    // Polls the "Remember Password" dialog's own Enter/Esc while it's open.
    UI::Login::Tick();

    ApplyRememberPasswordChoice();
    RevokeSavedCredentialsIfEdited();
    return true;
}

void CLoginWin::ApplyRememberPasswordChoice()
{
    // Applied here, not UpdateWhileActive: the modal leaves this window inactive, but
    // UpdateWhileShown keeps being called.
    const UI::Login::RememberPasswordChoice choice = UI::Login::RememberPasswordChoiceState();
    if (choice != UI::Login::RememberPasswordChoice::Ok
        && choice != UI::Login::RememberPasswordChoice::Cancel)
        return;

    UI::Login::ClearRememberPasswordChoice();

    const bool bAccepted = (choice == UI::Login::RememberPasswordChoice::Ok);
    GameConfig::GetInstance().SetSavePassword(bAccepted);
    m_bSavePasswordChecked = bAccepted;
}

void CLoginWin::RevokeSavedCredentialsIfEdited()
{
    // Editing the account or password drops any stored credentials and revokes
    // the save-password consent, so an out-of-date password never lingers in
    // config.ini for the next person on this machine.
    const auto& model = m_RmlBinder.GetModel();
    const std::wstring curUser = StringUtils::NarrowToWide(model.username);
    const std::wstring curPass = StringUtils::NarrowToWide(model.password);

    if (curUser == m_prevUsername && curPass == m_prevPassword)
        return;

    GameConfig& config = GameConfig::GetInstance();
    const bool bHadStored = m_bSavePasswordChecked
        || !config.GetEncryptedUsername().empty()
        || !config.GetEncryptedPassword().empty();
    if (bHadStored)
    {
        m_bSavePasswordChecked = false;
        config.ClearCredentials();
    }

    wcsncpy(m_prevUsername, curUser.c_str(), _countof(m_prevUsername) - 1);
    m_prevUsername[_countof(m_prevUsername) - 1] = L'\0';
    wcsncpy(m_prevPassword, curPass.c_str(), _countof(m_prevPassword) - 1);
    m_prevPassword[_countof(m_prevPassword) - 1] = L'\0';
}

bool CLoginWin::Render()
{
    if (FirstLoad)
    {
        // Land on whichever field the player still has to fill in.
        if (wcslen(m_Username) > 0)
            FocusPassword();
        else
            FocusUsername();
        FirstLoad = 0;
    }

    // g_CreditWin renders via plain CSprite/g_pRenderText, strictly before RmlUi's frame-final
    // document render, so login.rml's panel would otherwise always paint over it. RmlUi always
    // renders last regardless of GetLayerDepth(), so this must be toggled every frame rather than
    // just on Show().
    const bool coveredByCredits = g_CreditWin.IsVisible();
    // Through the transition-only helper, not a bare Show(): this runs every frame, and
    // ElementDocument::Show() defaults to FocusFlag::Auto, which would hand focus back to the
    // document and blur #input_account/#input_password on every single frame.
    UI::RmlBridge::SyncDocumentVisibility(m_pRmlDoc, !coveredByCredits);


    // Everything renders via the RmlUi overlay now, the two credential fields included, so there's
    // no native text pass left here -- and no need for the old g_SysMenuWin guard, which existed
    // only because raw CUITextInputBox pixels ignored RmlUi's own document ordering.
    SyncRmlModel();

    return true;
}

void CLoginWin::ApplyCredentialLimits()
{
    if (!m_pRmlDoc) return;

    if (Rml::Element* account = m_pRmlDoc->GetElementById("input_account"))
        account->SetAttribute("maxlength", static_cast<int>(MAX_USERNAME_SIZE));
    if (Rml::Element* password = m_pRmlDoc->GetElementById("input_password"))
        password->SetAttribute("maxlength", static_cast<int>(MAX_PASSWORD_SIZE));
}

void CLoginWin::FocusCredentialField(const char* elementId, bool selectAll)
{
    if (!m_pRmlDoc) return;

    Rml::Element* field = m_pRmlDoc->GetElementById(elementId);
    if (field == nullptr || !field->Focus())
        return;

    // Reproduces the native GiveFocus(TRUE): pre-select the text so a retry after a failed login
    // overwrites it in one keystroke instead of appending to it.
    if (selectAll)
    {
        if (auto* input = rmlui_dynamic_cast<Rml::ElementFormControlInput*>(field))
            input->Select();
    }
}

void CLoginWin::FocusUsername(bool selectAll)
{
    FocusCredentialField("input_account", selectAll);
}

void CLoginWin::FocusPassword(bool selectAll)
{
    FocusCredentialField("input_password", selectAll);
}

void CLoginWin::SyncRmlModel()
{
    if (!m_pRmlDoc) return;

    const bool rememberChecked = m_bRememberMeChecked;
    if (m_RmlBinder.GetModel().rememberMeChecked != rememberChecked)
    {
        m_RmlBinder.GetModel().rememberMeChecked = rememberChecked;
        m_RmlBinder.MarkDirty("remember_me_checked");
    }

    const bool saveChecked = m_bSavePasswordChecked;
    if (m_RmlBinder.GetModel().savePasswordChecked != saveChecked)
    {
        m_RmlBinder.GetModel().savePasswordChecked = saveChecked;
        m_RmlBinder.MarkDirty("save_password_checked");
    }

    wchar_t szServerName[MAX_TEXT_LENGTH] = {};
    const wchar_t* pServerStatus = g_ServerListManager->GetNonPVPInfo() ? I18N::Game::SDServer : I18N::Game::SDNonPvPServer;
    mu_swprintf(szServerName, pServerStatus, g_ServerListManager->GetSelectServerName(), g_ServerListManager->GetSelectServerIndex());
    const std::string serverNameUtf8 = StringUtils::WideToNarrow(szServerName);
    if (m_RmlBinder.GetModel().serverName != serverNameUtf8)
    {
        m_RmlBinder.GetModel().serverName = serverNameUtf8;
        m_RmlBinder.MarkDirty("server_name");
    }

    // Static (per-locale) labels; only dirties the model on an active locale change.
    auto syncLabel = [this](Rml::String LoginRmlModel::* field, const char* boundName, const wchar_t* text)
    {
        const std::string utf8 = StringUtils::WideToNarrow(text);
        if (m_RmlBinder.GetModel().*field != utf8)
        {
            m_RmlBinder.GetModel().*field = utf8;
            m_RmlBinder.MarkDirty(boundName);
        }
    };
    syncLabel(&LoginRmlModel::accountLabel, "account_label", I18N::Game::Account);
    syncLabel(&LoginRmlModel::passwordLabel, "password_label", I18N::Game::Password);
    syncLabel(&LoginRmlModel::rememberMeLabel, "remember_me_label", I18N::Game::LoginRememberUsername);
    syncLabel(&LoginRmlModel::savePasswordLabel, "save_password_label", I18N::Game::LoginRememberPassword);
    syncLabel(&LoginRmlModel::trustWarning, "trust_warning", I18N::Game::LoginTrustWarning);
    syncLabel(&LoginRmlModel::okLabel, "ok_label", I18N::Game::OK);
    syncLabel(&LoginRmlModel::cancelLabel, "cancel_label", I18N::Game::Cancel);
}

bool CLoginWin::UpdateMouseEvent()
{
    if (!IsVisible())
        return true;

    // Not modal: only claims a click within its own bounding box.
    RECT rc;
    ::SetRect(&rc, m_ptPos.x, m_ptPos.y, m_ptPos.x + m_Size.cx, m_ptPos.y + m_Size.cy);
    if (::PtInRect(&rc, CInput::Instance().GetCursorPos()))
        return false;

    return true;
}

void CLoginWin::RequestLogin()
{
    if (CurrentProtocolState == REQUEST_JOIN_SERVER)
        return;

    Show(false);

    const auto& model = m_RmlBinder.GetModel();
    wcsncpy(m_Username, StringUtils::NarrowToWide(model.username).c_str(), _countof(m_Username) - 1);
    m_Username[_countof(m_Username) - 1] = L'\0';
    wcsncpy(m_Password, StringUtils::NarrowToWide(model.password).c_str(), _countof(m_Password) - 1);
    m_Password[_countof(m_Password) - 1] = L'\0';

    // Handle credentials saving. The username is remembered when "remember me"
    // is set; the password is only stored on top of that with explicit consent.
    if (m_bRememberMeChecked)
    {
        GameConfig::GetInstance().SetSavePassword(m_bSavePasswordChecked);
        GameConfig::GetInstance().EncryptAndSaveCredentials(m_Username, m_Password);
    }
    else
    {
        // Clear saved credentials if user unchecked "Remember Me"
        GameConfig::GetInstance().ClearCredentials();
    }

    if (wcslen(m_Username) <= 0)
        CSceneUICoordinator::Instance().PopUpMsgWin(MESSAGE_INPUT_ID);
    else if (wcslen(m_Password) <= 0)
        CSceneUICoordinator::Instance().PopUpMsgWin(MESSAGE_INPUT_PASSWORD);
    else
    {
        SubmitCredentials(m_Username, m_Password);
    }
}

void CLoginWin::SubmitCredentials(const wchar_t* pszUsername, const wchar_t* pszPassword)
{
    if (CurrentProtocolState != RECEIVE_JOIN_SERVER_SUCCESS)
        return;

    if (pszUsername == nullptr || pszPassword == nullptr)
        return;

    // Put the login screen away before the request goes out. The manual path
    // hides it on the click; a caller that submits credentials directly must
    // not leave it up, or its text fields keep the keyboard focus and no key
    // reaches the game afterwards. Hiding an already hidden window is a no-op.
    Show(false);

    g_ConsoleDebug->Write(MCD_NORMAL, L"Login with the following account: %ls", pszUsername);

    g_ErrorReport.Write(L"> Login Request.\r\n");
    g_ErrorReport.Write(L"> Try to Login \"%ls\"\r\n", pszUsername);

    LogIn = 1;
    // The destination is a fixed-size global. The click path arrives here with
    // a string the input box already bounded, but this entry point is public:
    // anything longer is truncated rather than written past the end.
    wcsncpy(LogInID, pszUsername, MAX_USERNAME_SIZE);
    LogInID[MAX_USERNAME_SIZE] = L'\0';
    CurrentProtocolState = REQUEST_LOG_IN;

    SocketClient->ToGameServer()->SendLogin(pszUsername, pszPassword, Version, Serial);

    // Keep the credentials in memory so auto-reconnect can re-login
    // without prompting after an in-game disconnect.
    ReconnectManager::Instance().CacheCredentials(pszUsername, pszPassword);

    g_pSystemLogBox->AddText(I18N::Game::VerifyingYourAccount, mu::ui::window::TYPE_SYSTEM_MESSAGE);
    g_pSystemLogBox->AddText(I18N::Game::PleaseWait, mu::ui::window::TYPE_SYSTEM_MESSAGE);
}

void CLoginWin::CancelLogin()
{
    // Hide immediately, run the blocking reconnect on a background thread. Guard against ESC-spam
    // stacking threads: if one is already in flight, this press is a no-op.
    Show(false);

    if (m_bConnectionReconnectInFlight.load())
        return;

    if (m_ConnectionReconnectThread.joinable())
        m_ConnectionReconnectThread.join(); // previous run already finished and was consumed

    m_bConnectionReconnectInFlight.store(true);
    m_bConnectionReconnectDone.store(false);
    m_ConnectionReconnectThread = std::thread([this]
    {
        ConnectConnectionServer();
        m_bConnectionReconnectDone.store(true);
    });
}

void CLoginWin::ProcessPendingConnectionReconnect()
{
    if (m_ConnectionReconnectThread.joinable())
        m_ConnectionReconnectThread.join();
    m_bConnectionReconnectDone.store(false);
    m_bConnectionReconnectInFlight.store(false);
}

void CLoginWin::ConnectConnectionServer()
{
    LogIn = 0;
    CurrentProtocolState = REQUEST_JOIN_SERVER;
    CreateSocket(szServerIpAddress, g_ServerPort);
}
