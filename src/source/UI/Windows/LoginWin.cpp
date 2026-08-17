//*****************************************************************************
// File: LoginWin.cpp
//*****************************************************************************

#include "stdafx.h"
#include "UI/Windows/LoginWin.h"
#include "Core/Input/Input.h"
#include "UI/Legacy/UIMng.h"
#include "Render/Models/ZzzBMD.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInterface.h"
#include "Network/Reconnect/ReconnectManager.h"
#include "UI/Legacy/UIControls.h"
#include "Scenes/SceneCore.h"
#include "I18N/All.h"

#include "Audio/DSPlaySound.h"
#include "UI/NewUI/NewUISystem.h"
#include "UI/NewUI/Dialogs/NewUIMessageBox.h"
#include "UI/Windows/RememberPasswordPrompt.h"


#include "Network/Server/ServerListManager.h"
#ifdef _WIN32
#include <dpapi.h>
#endif

#include "Data/GameConfig/GameConfig.h"
#include "Data/GameConfig/GameConfigConstants.h"

#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "Core/Utilities/StringUtils.h"
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/Event.h>

#define LIW_OK			0
#define LIW_CANCEL		1



extern int g_iChatInputType;
extern int  LogIn;
extern wchar_t LogInID[MAX_USERNAME_SIZE + 1];
extern BYTE Version[SIZE_PROTOCOLVERSION];
extern BYTE Serial[SIZE_PROTOCOLSERIAL + 1];

CLoginWin::CLoginWin()
{
    m_pUsernameInputBox = NULL;
    m_pPasswordInputBox = NULL;
}

CLoginWin::~CLoginWin()
{
    SAFE_DELETE(m_pUsernameInputBox);
    SAFE_DELETE(m_pPasswordInputBox);
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

    // The login background is fixed artwork and does not scale with the window
    // size, so keep the original height. The credential-consent controls fit on
    // the panel and the trust warning is drawn just below it (issue #462).
    // CWin never draws anything for this window -- every theme's #panel/.input-frame renders the
    // background/input-box-frame artwork itself (see themes/legacy/login.rcss and
    // themes/modern/login.rcss). CWin::Create's own nTexID<=-2 sentinel (confirmed precedent:
    // ServerSelWin.cpp's CWin::Create(0,0,-2)) leaves m_psprBg null, and CWin::Render()'s existing
    // `if (m_psprBg)` guard then draws nothing.
    CWin::Create(329, 245, -2);
    SetMovable(false);

    for (int i = 0; i < 2; ++i)
    {
        m_aBtn[i].Create(54, 30, BITMAP_BUTTON + i, 3, 2, 1);
        CWin::RegisterButton(&m_aBtn[i]);
    }

    m_aBtnRememberMe.Create(16, 16, BITMAP_CHECK_BTN, 2, 0, 0, -1, 1, 1, 1);
    CWin::RegisterButton(&m_aBtnRememberMe);

    m_aBtnSavePassword.Create(16, 16, BITMAP_CHECK_BTN, 2, 0, 0, -1, 1, 1, 1);
    CWin::RegisterButton(&m_aBtnSavePassword);

    SAFE_DELETE(m_pUsernameInputBox);

    m_pUsernameInputBox = new CUITextInputBox;
    m_pUsernameInputBox->Init(g_hWnd, 140, 14, MAX_USERNAME_SIZE);
    m_pUsernameInputBox->SetBackColor(0, 0, 0, 25);
    m_pUsernameInputBox->SetTextColor(255, 255, 230, 210);
    m_pUsernameInputBox->SetFont(g_hFixFont);
    m_pUsernameInputBox->SetState(UISTATE_NORMAL);
    if (m_RememberMe) {
        m_pUsernameInputBox->SetText(m_Username);
        m_aBtnRememberMe.SetCheck(true);
    }

    SAFE_DELETE(m_pPasswordInputBox);

    m_pPasswordInputBox = new CUITextInputBox;
    m_pPasswordInputBox->Init(g_hWnd, 140, 14, MAX_PASSWORD_SIZE, TRUE);
    m_pPasswordInputBox->SetBackColor(0, 0, 0, 25);
    m_pPasswordInputBox->SetTextColor(255, 255, 230, 210);
    m_pPasswordInputBox->SetFont(g_hFixFont);
    m_pPasswordInputBox->SetState(UISTATE_NORMAL);

    m_pUsernameInputBox->SetTabTarget(m_pPasswordInputBox);
    m_pPasswordInputBox->SetTabTarget(m_pUsernameInputBox);

    if (m_RememberMe) {
        m_pPasswordInputBox->SetText(m_Password);
        m_aBtnRememberMe.SetCheck(true);
    }

    // The password is only pre-filled and re-saved when the player previously
    // opted in on a trusted machine.
    m_aBtnSavePassword.SetCheck(m_RememberMe && GameConfig::GetInstance().GetSavePassword());

    // Seed the edit-detection snapshot with what we just loaded so filling the
    // boxes here is not mistaken for the player editing them.
    m_pUsernameInputBox->GetText(m_prevUsername, _countof(m_prevUsername));
    m_pPasswordInputBox->GetText(m_prevPassword, _countof(m_prevPassword));

    this->FirstLoad = 1;

    // RmlUi migration plan Phase 1 pilot -- see this class's header comment. Guarded on
    // m_pRmlDoc rather than unconditionally: CUIMng::RepositionSceneUI() re-runs
    // CreateLoginScene() (and so this Create()) on every resolution change, to refresh the
    // legacy sprites' stale screen-height-dependent Y-flip cache -- a problem RmlUi's own
    // layout doesn't have (it already re-flows against the Context's current dimensions), so
    // the document/data-model are set up once, ever, and only repositioned afterward (see
    // SetPosition() below), not recreated.
    if (!m_pRmlDoc && RmlUiRuntime::Instance().IsCreated())
    {
        // The data model must exist BEFORE the document referencing it (via data-model="login")
        // is loaded -- RmlUi resolves data-model/{{bindings}} while PARSING the RML, so a model
        // created after LoadDocument() is too late: every {{...}} in the document falls back to
        // rendering its own literal source text instead of the bound value (confirmed from a
        // real screenshot: "{{account_label}}", "{{server_name}}" etc. rendered verbatim). Create
        // the model first, then load the document.
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

                c.BindEventCallback("login_ok_click",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlClickOk(); });
                c.BindEventCallback("login_cancel_click",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlClickCancel(); });
                c.BindEventCallback("login_toggle_remember_me",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlToggleRememberMe(); });
                c.BindEventCallback("login_toggle_save_password",
                    [this](Rml::DataModelHandle, Rml::Event&, const Rml::VariantList&) { RmlToggleSavePassword(); });
            });

        // Routed through UI::RmlBridge::LoadThemedDocument (not Context::LoadDocument directly)
        // so this document resolves against the active theme's stylesheet -- see RmlTheme.h.
        if (modelCreated)
            m_pRmlDoc = UI::RmlBridge::LoadThemedDocument(RmlUiRuntime::Instance().GetContext(), "Data/Interface/RmlUi/login.rml");

        // Deliberately NOT calling UI::RmlBridge::MakeDraggable() here -- the login screen is
        // meant to stay static (see this class's CursorInWin(WA_MOVE) override, hardcoded false).
        // MakeDraggable() itself was built and verified against this exact document during
        // development (see docs/rmlui-ui-system/architecture.md) but isn't wired up for real use
        // on this window.
    }
}

void CLoginWin::PreRelease()
{
    // CUIMng::RemoveWinList() (run on every scene transition) calls Release() on every window
    // in its list unconditionally -- CWin's own Release()/PreRelease() has no knowledge of
    // m_pRmlDoc, so without this it can keep rendering into whatever scene comes next if this
    // window is ever Released() while still shown. In today's real login/cancel flow that
    // never happens (RequestLogin()/CancelLogin() both call CUIMng::HideWin(this) first), but
    // that's an incidental property of those two call sites, not something this class's own
    // lifecycle actually guarantees -- explicit here rather than relying on it staying true.
    // Hide(), not unload -- the document/model are meant to be created once and reused (see
    // Create()'s own guard comment above).
    if (m_pRmlDoc)
        m_pRmlDoc->Hide();
}

void CLoginWin::SetPosition(int x, int y)
{
	CWin::SetPosition(x, y);

	if (g_iChatInputType == 1)
	{
		const int boxX = int((x + 115) / g_fScreenRate_x);
		m_pUsernameInputBox->SetPosition(boxX, int((y + 112) / g_fScreenRate_y));
		m_pPasswordInputBox->SetPosition(boxX, int((y + 137) / g_fScreenRate_y));
	}

	// "Remember Username" (row 1) and "Remember Password" (row 2) stack
	// vertically; the OK/Cancel buttons move down to make room. These pixel
	// offsets are eyeballed against the login background and may need tuning.
	m_aBtnRememberMe.SetPosition(x + 109, y + 156);
	m_aBtnSavePassword.SetPosition(x + 109, y + 176);
	m_aBtn[LIW_OK].SetPosition(x + 150, y + 200);
	m_aBtn[LIW_CANCEL].SetPosition(x + 211, y + 200);

	// RmlUi panel overlay: positioned at the same real window-pixel origin CWin's own
	// background sprite (m_psprBg) uses -- unlike the legacy g_pRenderText calls this replaces
	// (which divided by g_fScreenRate_x/y to convert into the old 640x480 reference space),
	// RmlUi's Context operates directly in real window pixels, so no scale conversion is
	// needed here; the panel's children use fixed-pixel RCSS offsets relative to this container,
	// matching "the login background is fixed artwork and does not scale" above.
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
    CWin::Show(bShow);

    for (int i = 0; i < 2; ++i)
        m_aBtn[i].Show(bShow);
    m_aBtnRememberMe.Show(bShow);
    m_aBtnSavePassword.Show(bShow);

    // Drive the text fields' state so a hidden login screen releases keyboard
    // focus (portable fields stop SDL text input when hidden, #447).
    const int iState = bShow ? UISTATE_NORMAL : UISTATE_HIDE;
    if (m_pUsernameInputBox) m_pUsernameInputBox->SetState(iState);
    if (m_pPasswordInputBox) m_pPasswordInputBox->SetState(iState);

    if (m_pRmlDoc)
    {
        if (bShow) { SyncRmlModel(); m_pRmlDoc->Show(); }
        else       m_pRmlDoc->Hide();
    }
}

void CLoginWin::UpdateWhileActive(double)
{
	// While the "Remember Password" confirmation dialog is open, let it own the
	// input so Enter/Esc/clicks don't also drive the login screen behind it. The
	// dialog's outcome is applied on the next frame, once it has closed. Scoped to this
	// dialog's own Pending state (not the old shared g_MessageBox->IsEmpty() stack-wide check --
	// this dialog no longer registers with that engine, see RememberPasswordPrompt.cpp).
	if (UI::Login::RememberPasswordChoiceState() == UI::Login::RememberPasswordChoice::Pending)
		return;

	if (m_aBtn[LIW_OK].IsClick() || m_bRmlOkClicked || CInput::Instance().IsKeyDown(VK_RETURN))
	{
		m_bRmlOkClicked = false;
		PlayBuffer(SOUND_CLICK01);
		RequestLogin();
		return;
	}

	if (m_aBtn[LIW_CANCEL].IsClick() || m_bRmlCancelClicked || CInput::Instance().IsKeyDown(VK_ESCAPE))
	{
		m_bRmlCancelClicked = false;
		PlayBuffer(SOUND_CLICK01);
		CancelLogin();
		CUIMng::Instance().SetSysMenuWinShow(false);
		return;
	}

	UpdateRememberCheckboxes();
}

void CLoginWin::UpdateRememberCheckboxes()
{
	GameConfig& config = GameConfig::GetInstance();

	if (m_aBtnRememberMe.IsClick() || m_bRmlRememberMeClicked)
	{
		m_bRmlRememberMeClicked = false;
		m_RememberMe = m_aBtnRememberMe.IsCheck();
		config.SetRememberMe(m_RememberMe != 0);

		// Switching off "remember me" revokes everything: drop the stored
		// credentials from config.ini now so they can't linger if the game is
		// closed before the next login.
		if (!m_RememberMe)
		{
			m_aBtnSavePassword.SetCheck(false);
			config.ClearCredentials();
		}
	}

	if (!m_aBtnSavePassword.IsClick() && !m_bRmlSavePasswordClicked)
		return;
	m_bRmlSavePasswordClicked = false;

	if (!m_aBtnSavePassword.IsCheck())
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
	m_aBtnSavePassword.SetCheck(false);

	// Storing the password implies remembering the account.
	if (!m_RememberMe)
	{
		m_RememberMe = 1;
		m_aBtnRememberMe.SetCheck(true);
		config.SetRememberMe(true);
	}
	UI::Login::OpenRememberPasswordPrompt();
}

void CLoginWin::UpdateWhileShow(double dDeltaTick)
{
    m_pUsernameInputBox->DoAction();
    m_pPasswordInputBox->DoAction();

    // Polls the "Remember Password" dialog's own Enter/Esc while it's open -- ticked from here
    // (not UpdateWhileActive) for the same reason ApplyRememberPasswordChoice() is, right below.
    UI::Login::Tick();

    ApplyRememberPasswordChoice();
    RevokeSavedCredentialsIfEdited();
}

void CLoginWin::ApplyRememberPasswordChoice()
{
    // Applied here rather than in UpdateWhileActive because the modal message box
    // leaves the login window inactive (so UpdateWhileActive stops running),
    // while UpdateWhileShow keeps being called.
    const UI::Login::RememberPasswordChoice choice = UI::Login::RememberPasswordChoiceState();
    if (choice != UI::Login::RememberPasswordChoice::Ok
        && choice != UI::Login::RememberPasswordChoice::Cancel)
        return;

    UI::Login::ClearRememberPasswordChoice();

    const bool bAccepted = (choice == UI::Login::RememberPasswordChoice::Ok);
    GameConfig::GetInstance().SetSavePassword(bAccepted);
    m_aBtnSavePassword.SetCheck(bAccepted);
}

void CLoginWin::RevokeSavedCredentialsIfEdited()
{
    // Editing the account or password drops any stored credentials and revokes
    // the save-password consent, so an out-of-date password never lingers in
    // config.ini for the next person on this machine.
    wchar_t curUser[MAX_USERNAME_SIZE + 1] = {};
    wchar_t curPass[MAX_PASSWORD_SIZE + 1] = {};
    m_pUsernameInputBox->GetText(curUser, _countof(curUser));
    m_pPasswordInputBox->GetText(curPass, _countof(curPass));

    if (wcscmp(curUser, m_prevUsername) == 0 && wcscmp(curPass, m_prevPassword) == 0)
        return;

    GameConfig& config = GameConfig::GetInstance();
    const bool bHadStored = m_aBtnSavePassword.IsCheck()
        || !config.GetEncryptedUsername().empty()
        || !config.GetEncryptedPassword().empty();
    if (bHadStored)
    {
        m_aBtnSavePassword.SetCheck(false);
        config.ClearCredentials();
    }

    wcscpy_s(m_prevUsername, _countof(m_prevUsername), curUser);
    wcscpy_s(m_prevPassword, _countof(m_prevPassword), curPass);
}

void CLoginWin::RenderControls()
{
    if (FirstLoad)
    {
        (wcslen(m_Username) > 0 ? m_pPasswordInputBox : m_pUsernameInputBox)->GiveFocus();
        FirstLoad = 0;
    }

    // All panel chrome (background, input-box frames, checkboxes, OK/Cancel buttons, labels,
    // trust-warning text) now renders via the RmlUi overlay -- see this class's header comment
    // and login.rml/.rcss. Nothing legacy left to draw here. The actual CUITextInputBox text is
    // NOT rendered here -- see RenderTextOnTop()'s comment for why it moved to a separate, later
    // call.
    SyncRmlModel();
}

void CLoginWin::RenderTextOnTop()
{
    m_pUsernameInputBox->Render();
    m_pPasswordInputBox->Render();
}

void CLoginWin::SyncRmlModel()
{
    if (!m_pRmlDoc) return;

    const bool rememberChecked = m_aBtnRememberMe.IsCheck();
    if (m_RmlBinder.GetModel().rememberMeChecked != rememberChecked)
    {
        m_RmlBinder.GetModel().rememberMeChecked = rememberChecked;
        m_RmlBinder.MarkDirty("remember_me_checked");
    }

    const bool saveChecked = m_aBtnSavePassword.IsCheck();
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

    // Static (per-locale) labels, synced from the same I18N::Game::* slots the legacy
    // g_pRenderText calls used directly -- cheap to re-check every call via the string
    // comparison; only actually dirties the model on an active locale change, which is rare.
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

void CLoginWin::RequestLogin()
{
    if (CurrentProtocolState == REQUEST_JOIN_SERVER)
        return;

    CUIMng::Instance().HideWin(this);

    m_pUsernameInputBox->GetText(m_Username, _countof(m_Username));
    m_pPasswordInputBox->GetText(m_Password, _countof(m_Password));

    // Handle credentials saving. The username is remembered when "remember me"
    // is set; the password is only stored on top of that with explicit consent.
    if (m_aBtnRememberMe.IsCheck())
    {
        GameConfig::GetInstance().SetSavePassword(m_aBtnSavePassword.IsCheck());
        GameConfig::GetInstance().EncryptAndSaveCredentials(m_Username, m_Password);
    }
    else
    {
        // Clear saved credentials if user unchecked "Remember Me"
        GameConfig::GetInstance().ClearCredentials();
    }

    if (wcslen(m_Username) <= 0)
        CUIMng::Instance().PopUpMsgWin(MESSAGE_INPUT_ID);
    else if (wcslen(m_Password) <= 0)
        CUIMng::Instance().PopUpMsgWin(MESSAGE_INPUT_PASSWORD);
    else
    {
        if (CurrentProtocolState == RECEIVE_JOIN_SERVER_SUCCESS)
        {
            g_ConsoleDebug->Write(MCD_NORMAL, L"Login with the following account: %ls", m_Username);

            g_ErrorReport.Write(L"> Login Request.\r\n");
            g_ErrorReport.Write(L"> Try to Login \"%ls\"\r\n", m_Username);

            LogIn = 1;
            wcscpy(LogInID, (m_Username));
            CurrentProtocolState = REQUEST_LOG_IN;

            SocketClient->ToGameServer()->SendLogin(m_Username, m_Password, Version, Serial);

            // Keep the credentials in memory so auto-reconnect can re-login
            // without prompting after an in-game disconnect.
            ReconnectManager::Instance().CacheCredentials(m_Username, m_Password);

            g_pSystemLogBox->AddText(I18N::Game::VerifyingYourAccount, SEASON3B::TYPE_SYSTEM_MESSAGE);
            g_pSystemLogBox->AddText(I18N::Game::PleaseWait, SEASON3B::TYPE_SYSTEM_MESSAGE);
        }
    }
}

void CLoginWin::CancelLogin()
{
    ConnectConnectionServer();
    CUIMng::Instance().HideWin(this);
}

void CLoginWin::ConnectConnectionServer()
{
    LogIn = 0;
    CurrentProtocolState = REQUEST_JOIN_SERVER;
    CreateSocket(szServerIpAddress, g_ServerPort);
}