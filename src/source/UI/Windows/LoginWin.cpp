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
#include "Core/Globals/_enum.h"
#include "Render/Models/ZzzBMD.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInterface.h"
#include "Network/Reconnect/ReconnectManager.h"
#include "UI/Widgets/UIControls.h"
#include "Scenes/SceneCore.h"
#include "I18N/All.h"

#include "Audio/DSPlaySound.h"
#include "UI/Core/NewUISystem.h"
#include "UI/Dialogs/NewUIMessageBox.h"
#include "UI/Windows/RememberPasswordPrompt.h"


#include "Network/Server/ServerListManager.h"
#ifdef _WIN32
#include <dpapi.h>
#endif

#include "Data/GameConfig/GameConfig.h"
#include "Data/GameConfig/GameConfigConstants.h"

#include "Render/RmlUi/RmlUiRuntime.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/Scaling/UITransform.h"
#include "Core/Utilities/StringUtils.h"
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/Event.h>
#include <cmath>

extern unsigned int WindowWidth, WindowHeight;

namespace
{
    // Same combined ratio RmlUi's own dp unit uses (RmlUiRuntime.cpp's ApplyUIScale(),
    // UIScalePercent x UI::Scaling::ViewportFitScale()) -- every fixed reference-pixel offset
    // this window's C++ still computes (the panel's own legacy bounding-box size, the real
    // CUITextInputBox placement, the checkbox/OK/Cancel legacy CButton companions) must scale by
    // this to stay pixel-for-pixel aligned with login.rcss's own now-dp values, the same lockstep
    // requirement CharSelMainWin.cpp/LoginMainWin.cpp already established for their own windows.
    // UI::Scaling::CompanionRatio() (UITransform.cpp) is the single shared implementation of this
    // formula (extracted 2026-09-03 after this exact function, CharSelMainWin.cpp's
    // GetUIScaleRatio(), and LoginMainWin.cpp's inline version had each independently hand-copied
    // it) -- it's the reason to keep reading the WindowWidth/WindowHeight globals here rather than
    // CInput::Instance().GetScreenWidth()/GetScreenHeight(): a real bug, found via a screenshot
    // showing the panel rendering off-center and the username text/caret floating outside the
    // input box entirely, traced to CInput's own copy of the screen size not reliably matching
    // WindowWidth/WindowHeight (the exact values RmlUiRuntime::OnResize() uses).
    float LoginUIScaleRatio()
    {
        return UI::Scaling::CompanionRatio(static_cast<int>(WindowWidth), static_cast<int>(WindowHeight));
    }

    int ScaledOffset(int value, float ratio)
    {
        return static_cast<int>(std::lround(value * ratio));
    }
}

#define LIW_OK			0
#define LIW_CANCEL		1

extern int g_iChatInputType;
extern int  LogIn;
extern wchar_t LogInID[MAX_USERNAME_SIZE + 1];
extern BYTE Version[SIZE_PROTOCOLVERSION];
extern BYTE Serial[SIZE_PROTOCOLSERIAL + 1];

// Replaces CUIMng's old `CLoginWin m_LoginWin;` member, same convention as g_CreditWin.
CLoginWin g_LoginWin;

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

    // This window draws none of its own chrome -- every theme's #panel/.input-frame renders the
    // background/input-box-frame artwork itself (see this class's header comment).
    //
    // 2026-09-03: the legacy bounding-box size tracks login.rcss's own #panel width/height
    // (329dp/245dp, both themes) by the same ratio, instead of a fixed 329x245 this used to always
    // be -- UpdateMouseEvent() reads m_Size for its own hit-test rect, and letting it go stale
    // against the now-auto-fitting RmlUi visuals risks the same class of bug
    // CharSelMainWin.h/CalculateFixedAnchorLayout()'s own comment documents in detail (a correctly
    // rendered element whose legacy hit-test rect no longer matches it).
    const float uiScale = LoginUIScaleRatio();
    m_Size.cx = ScaledOffset(329, uiScale);
    m_Size.cy = ScaledOffset(245, uiScale);
    m_ptPos.x = m_ptPos.y = 0;

    for (int i = 0; i < 2; ++i)
    {
        m_aBtn[i].Create(54, 30, BITMAP_BUTTON + i, 3, 2, 1);
    }

    m_aBtnRememberMe.Create(16, 16, BITMAP_CHECK_BTN, 2, 0, 0, -1, 1, 1, 1);
    m_aBtnSavePassword.Create(16, 16, BITMAP_CHECK_BTN, 2, 0, 0, -1, 1, 1, 1);

    // CButton::Update() auto-toggles m_bCheck on its own input polling whenever HasCheckVisuals()
    // is true (both are 2-frame check-style art) -- independent of and in addition to
    // RmlToggleRememberMe()/RmlToggleSavePassword()'s own SetCheck() flip below, since legacy
    // input polling isn't gated by which UI tier's Context claimed the click. Left enabled, one
    // click produces two flips (net no change). SetEnable(false) makes these pure state
    // containers (SetCheck/IsCheck still work for every other call site); only the RmlUi handler
    // drives them now. Plain action buttons (OK/Cancel) don't need this -- HasCheckVisuals() is
    // false there, so redundant firing is harmless.
    m_aBtnRememberMe.SetEnable(false);
    m_aBtnSavePassword.SetEnable(false);

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
    // m_pRmlDoc rather than unconditionally: CSceneUICoordinator::RepositionSceneUI() re-runs
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
        // meant to stay static.
    }

    CSceneUICoordinator::Instance().GetNewStyleMng().AddUIObj(mu::ui::window::INTERFACE_LOGIN, this);
    Show(false);
}

void CLoginWin::SetPosition(int x, int y)
{
	m_ptPos.x = x;
	m_ptPos.y = y;

	// This class draws the real text/hit-testing for the two input boxes (RmlUi's own
	// .input-frame divs are border-only overlays with no background, so they never cover what's
	// typed here) plus the otherwise-inert checkbox/button state objects below (SetEnable(false)
	// elsewhere in this file; RmlUi owns their real click handling and visuals in both themes, so
	// these are kept in sync only to avoid a silently-stale coordinate set, not because anything
	// still renders or hit-tests through them). These offsets must track each theme's own RCSS
	// positions for the same elements (login.rcss's .input-account/.input-password/
	// .checkbox-remember-me/.checkbox-save-password/.btn-ok/.btn-cancel) -- legacy's are fixed to
	// match its real sprite art (login_back.tga/login_me.tga, pixel-faithful, never moves);
	// modern's are its own, currently more spaced-out layout. This function isn't itself
	// theme-aware anywhere else, but the two themes' positions genuinely diverge here, so branch
	// just for these offsets rather than picking one theme's numbers and silently misplacing the
	// real input text in the other.
	//
	// 2026-09-03: every offset below is now scaled by LoginUIScaleRatio() -- login.rcss's own
	// positions became dp (grow with UIScalePercent/window size) the same day, and without this
	// these real, functional CUITextInputBox/legacy-CButton placements would silently drift from
	// the now-auto-fitting RmlUi visuals at any ratio other than 1.0. Highest-stakes instance of
	// the lockstep requirement this whole migration pass has been applying (CharSelMainWin,
	// LoginMainWin, sys_menu) -- this window has real, functional text entry, not just redundant
	// click-detection companions, so a mismatch here is the most consequential.
	const bool bModernTheme = (UI::RmlBridge::GetActiveThemeName() == "modern");
	const float uiScale = LoginUIScaleRatio();
	const int usernameY = ScaledOffset(bModernTheme ? 72 : 112, uiScale);
	const int passwordY = ScaledOffset(bModernTheme ? 105 : 137, uiScale);
	const int rememberMeY = ScaledOffset(bModernTheme ? 134 : 156, uiScale);
	const int savePasswordY = ScaledOffset(bModernTheme ? 158 : 176, uiScale);
	const int buttonRowY = ScaledOffset(bModernTheme ? 196 : 200, uiScale);
	const int checkboxButtonX = ScaledOffset(109, uiScale);
	const int okButtonX = ScaledOffset(150, uiScale);
	const int cancelButtonX = ScaledOffset(211, uiScale);

	if (g_iChatInputType == 1)
	{
		// Real pixels, not divided by g_fScreenRate_x/y -- CUITextInputBox::Render() rescales the
		// position it's given via ConvertPositionX/Y using *whatever transform is active when
		// Render() runs* (see RenderTextOnTop()'s own comment), a fundamentally different contract
		// than CSprite's "store real pixels, ignore the transform entirely". Dividing here relied
		// on a later multiply landing under the exact same ambient transform to cancel it back
		// out -- true unconditionally only as long as this ran unscoped (CWin days); once this
		// window's own dispatch can run inside a ScopedActiveTransform(LayoutMode::Legacy) scope,
		// dividing by the ambient rate here while the identity scope is active would silently
		// store an un-descaled value. Same fix as CMsgWin's resident-password gotcha
		// (docs/newui-legacy-merger.md) -- store real pixels on both ends instead.
		const int boxX = x + ScaledOffset(115, uiScale);
		m_pUsernameInputBox->SetPosition(boxX, y + usernameY);
		m_pPasswordInputBox->SetPosition(boxX, y + passwordY);
	}

	// "Remember Username" (row 1) and "Remember Password" (row 2) stack vertically; the OK/Cancel
	// buttons move down to make room.
	m_aBtnRememberMe.SetPosition(x + checkboxButtonX, y + rememberMeY);
	m_aBtnSavePassword.SetPosition(x + checkboxButtonX, y + savePasswordY);
	m_aBtn[LIW_OK].SetPosition(x + okButtonX, y + buttonRowY);
	m_aBtn[LIW_CANCEL].SetPosition(x + cancelButtonX, y + buttonRowY);

	// RmlUi panel overlay: positioned at the same real window-pixel origin this window's own
	// bounding box (m_ptPos) uses -- RmlUi's Context operates directly in real window pixels, so
	// no scale conversion is needed for the panel's own origin here; it's the panel's own SIZE
	// (login.rcss's #panel width/height) and every child's position that are dp now, scaled
	// automatically by RmlUi itself -- this C++-pushed left/top is just the panel's screen
	// placement, not its layout.
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
    mu::ui::window::CNewUIObj::Show(bShow);

    for (int i = 0; i < 2; ++i)
    {
        m_aBtn[i].Show(bShow);
    }
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

void CLoginWin::Release()
{
    // Every scene transition calls Release() explicitly on this global now (CreateLoginScene()
    // etc.), same as every other migrated window -- this window no longer lives in any list that
    // would do it automatically. Hide(), not unload -- the document/model are meant to be created
    // once and reused (see Create()'s own guard comment above).
    if (m_pRmlDoc)
        m_pRmlDoc->Hide();
}

void CLoginWin::RmlClickOk() { SubmitLogin(); }
void CLoginWin::RmlClickCancel() { SubmitCancel(); }

void CLoginWin::RmlToggleRememberMe()
{
	m_aBtnRememberMe.SetCheck(!m_aBtnRememberMe.IsCheck());
	if (UI::Login::RememberPasswordChoiceState() != UI::Login::RememberPasswordChoice::Pending)
		ApplyRememberMeChange();
}

void CLoginWin::RmlToggleSavePassword()
{
	m_aBtnSavePassword.SetCheck(!m_aBtnSavePassword.IsCheck());
	if (UI::Login::RememberPasswordChoiceState() != UI::Login::RememberPasswordChoice::Pending)
		ApplySavePasswordChange();
}

bool CLoginWin::UpdateWhileActive()
{
	if (m_aBtn[LIW_OK].IsClick() || CInput::Instance().IsKeyDown(VK_RETURN))
	{
		SubmitLogin();
		return true;
	}

	if (m_aBtn[LIW_CANCEL].IsClick() || CInput::Instance().IsKeyDown(VK_ESCAPE))
	{
		SubmitCancel();
		return true;
	}

	UpdateRememberCheckboxes();
	return true;
}

// 2026-09-03: RmlClickOk()/RmlClickCancel() (see LoginWin.h's header comment) call these directly,
// bypassing the shown/active split entirely -- UpdateWhileActive()'s own CButton::IsClick()/
// keyboard branches above call the same functions, so there is exactly one place each action's
// logic lives regardless of which path triggered it. Each re-checks the "remember password"
// prompt's live Pending state rather than trusting the caller: cheap, and correct from both call
// sites (UpdateWhileActive() never even runs while pending, per UpdateWhileShown()'s SetActive()
// computation; the immediate RmlUi callbacks haven't had a chance to observe that yet without
// this).
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

void CLoginWin::UpdateRememberCheckboxes()
{
	if (m_aBtnRememberMe.IsClick())
		ApplyRememberMeChange();
	if (m_aBtnSavePassword.IsClick())
		ApplySavePasswordChange();
}

void CLoginWin::ApplyRememberMeChange()
{
	GameConfig& config = GameConfig::GetInstance();
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

void CLoginWin::ApplySavePasswordChange()
{
	GameConfig& config = GameConfig::GetInstance();

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

bool CLoginWin::UpdateWhileShown()
{
    // Computed BEFORE Tick() (below) can resolve a pending Remember-Password prompt -- see this
    // method's own header comment for why that ordering matters. Also folds in the "don't let
    // Enter/Esc double-fire past an overlaying modal" gate CCharSelMainWin/CCharMakeWin already
    // needed once: a higher-depth modal claiming UpdateMouseEvent() doesn't, by itself, stop this
    // window's own Update() from still polling VK_RETURN/VK_ESCAPE directly in UpdateWhileActive()
    // below -- without this, pressing Enter to dismiss a CMsgWin login-failure message could also
    // resubmit the login form behind it.
    //
    // WasSysMenuToggledByEscThisFrame() is needed IN ADDITION to the live g_SysMenuWin.IsVisible()
    // check -- found via live testing: CSceneUICoordinator::Update()'s ESC-toggle-system-menu block runs
    // entirely before this (see its own comment), so an ESC press that just CLOSED the menu this
    // same frame already reads back IsVisible()==false here, and without this flag this window
    // would treat that as "not covered" and ALSO fire SubmitCancel() off the very same keypress.
    SetActive(!(g_CreditWin.IsVisible() || g_MsgWin.IsVisible() || g_SysMenuWin.IsVisible()
                || CSceneUICoordinator::Instance().WasSysMenuToggledByEscThisFrame()
                || UI::Login::RememberPasswordChoiceState() == UI::Login::RememberPasswordChoice::Pending));

    m_pUsernameInputBox->DoAction();
    m_pPasswordInputBox->DoAction();

    // Polls the "Remember Password" dialog's own Enter/Esc while it's open -- ticked from here
    // (not UpdateWhileActive) for the same reason ApplyRememberPasswordChoice() is, right below.
    UI::Login::Tick();

    ApplyRememberPasswordChoice();
    RevokeSavedCredentialsIfEdited();
    return true;
}

void CLoginWin::ApplyRememberPasswordChoice()
{
    // Applied here rather than in UpdateWhileActive because the modal message box
    // leaves the login window inactive (so UpdateWhileActive stops running),
    // while UpdateWhileShown keeps being called.
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

bool CLoginWin::Render()
{
    if (FirstLoad)
    {
        (wcslen(m_Username) > 0 ? m_pPasswordInputBox : m_pUsernameInputBox)->GiveFocus();
        FirstLoad = 0;
    }

    // g_CreditWin's own visuals are plain CSprite/g_pRenderText content (CUIMng/CNewUIManager
    // merger, docs/newui-legacy-merger.md), drawn in a C++ pass strictly *before* RmlUi's own
    // frame-final document render (docs/rmlui-ui-system/README.md's "RmlUi renders last") --
    // login.rml's own panel would otherwise always paint over it regardless of which was opened
    // more recently, the same "RmlUi always wins" gap CCharInfoBalloonMng's own shouldHide check
    // exists to work around. This is a PERMANENT gap, not one this class's own migration onto
    // CNewUIObj retires: GetLayerDepth()'s sort only orders this manager's own dispatch, not
    // RmlUi's separate, always-last compositor pass, so no depth choice can substitute for this
    // toggle (docs/newui-legacy-merger.md). Toggled every frame here (not just on Show()), same
    // idempotent pattern as that check, since credits can open/close at any time while this
    // dialog is already showing.
    const bool coveredByCredits = g_CreditWin.IsVisible();
    if (m_pRmlDoc)
    {
        if (coveredByCredits) m_pRmlDoc->Hide();
        else                  m_pRmlDoc->Show();
    }

    // All panel chrome (background, input-box frames, checkboxes, OK/Cancel buttons, labels,
    // trust-warning text) now renders via the RmlUi overlay -- see this class's header comment
    // and login.rml/.rcss. Nothing legacy left to draw here. RenderTextOnTop() below draws the
    // actual CUITextInputBox text -- called directly here (not deferred to a later "after RmlUi"
    // call site) since the default "legacy" theme's panel is transparent, so draw order doesn't
    // matter yet; see RenderTextOnTop()'s own comment.
    SyncRmlModel();

    // Also skip while g_SysMenuWin is shown -- its own RmlUi panel already stacks correctly
    // against login.rml's (both are same-phase RmlUi documents; no fix needed there), but these
    // are raw CUITextInputBox pixels drawn directly, not RmlUi content, so RmlUi's own document
    // ordering has no effect on them at all. Same permanent-gap reasoning as coveredByCredits
    // above.
    if (!coveredByCredits && !g_SysMenuWin.IsVisible())
        RenderTextOnTop();

    return true;
}

void CLoginWin::RenderTextOnTop()
{
    // Force identity so this agrees with SetPosition()'s now-real-pixel values regardless of
    // which context runs this call (Render()'s own call above, under whatever transform
    // CNewUIManager::Render() applies for this window's LayoutMode::Legacy, or Winmain.cpp's
    // completely unscoped post-RmlUi callback) -- same fix as CMsgWin's resident-password gotcha
    // (docs/newui-legacy-merger.md).
    const auto transform = UI::Scaling::TransformForLayout(UI::Scaling::LayoutMode::Legacy, WindowWidth, WindowHeight);
    UI::Scaling::ScopedActiveTransform identity(transform);
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

bool CLoginWin::UpdateMouseEvent()
{
    if (!IsVisible())
        return true;

    // Was CWin::CursorInWin(WA_ALL) -- ported directly (see CServerSelWin/CLoginMainWin's
    // identical pattern). Not modal: only claims a click within its own bounding box, leaving the
    // world/credits/system-menu reachable around it.
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
        CSceneUICoordinator::Instance().PopUpMsgWin(MESSAGE_INPUT_ID);
    else if (wcslen(m_Password) <= 0)
        CSceneUICoordinator::Instance().PopUpMsgWin(MESSAGE_INPUT_PASSWORD);
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

            g_pSystemLogBox->AddText(I18N::Game::VerifyingYourAccount, mu::ui::window::TYPE_SYSTEM_MESSAGE);
            g_pSystemLogBox->AddText(I18N::Game::PleaseWait, mu::ui::window::TYPE_SYSTEM_MESSAGE);
        }
    }
}

void CLoginWin::CancelLogin()
{
    ConnectConnectionServer();
    Show(false);
}

void CLoginWin::ConnectConnectionServer()
{
    LogIn = 0;
    CurrentProtocolState = REQUEST_JOIN_SERVER;
    CreateSocket(szServerIpAddress, g_ServerPort);
}
