//*****************************************************************************
// File: LoginWin.h
//*****************************************************************************
#pragma once

#include "UI/NewUI/NewUIBase.h"

#include "UI/Widgets/Button.h"
#include "UI/RmlBridge/RmlModelBinder.h"

class CUITextInputBox;

namespace Rml { class ElementDocument; }

// RmlUi migration plan Phase 1 pilot (login dialog): this window draws nothing of its own chrome
// -- the RmlUi document (Data/Interface/RmlUi/login.rml) renders the panel background, input-box
// frames, checkboxes, buttons, labels, and trust warning as an overlay -- the "legacy" theme
// reproduces the original look by pointing its RCSS decorators at the same art files
// (Interface/login_back.tga, Interface/login_me.tga) the old CWin sprites drew, so this is a
// renderer swap, not a visual change. The legacy CButton objects below remain the actual
// state-holders (IsCheck/SetCheck) for the checkboxes/OK/Cancel buttons -- nothing about their
// internal semantics changed, only what draws them. Username/password text entry deliberately
// stays on the legacy CUITextInputBox objects (m_pUsernameInputBox/m_pPasswordInputBox) rather
// than moving to native RmlUi <input> elements -- external code (WSclient.cpp, MsgWin.cpp) calls
// GetUsernameInputBox()/GetPasswordInputBox()->GiveFocus() directly for error-recovery focus
// redirection, and duplicating credential-entry/focus logic into a second, independent text-input
// system this session can't test is a real regression risk not worth taking for this pass.
//
// CUIMng/CNewUIManager merger (docs/newui-legacy-merger.md), Phase 3 -- the last CWin subclass to
// migrate, and the first window in the whole series that actually needs CNewUIObj's shown-vs-
// active split (UpdateWhileShown()/UpdateWhileActive(), added in Phase 0 but unused until now).
class CLoginWin : public SEASON3B::CNewUIObj
{
protected:
    CButton m_aBtn[2];
    CButton m_aBtnRememberMe;
    CButton m_aBtnSavePassword;
    CUITextInputBox* m_pUsernameInputBox;
    CUITextInputBox* m_pPasswordInputBox;

    // Snapshot of the field contents, used to detect that the player edited the
    // username or password so the stored credentials can be dropped.
    wchar_t m_prevUsername[MAX_USERNAME_SIZE + 1] = {};
    wchar_t m_prevPassword[MAX_PASSWORD_SIZE + 1] = {};

    // Replaces CWin::m_ptPos/m_Size -- no shared rect facility on the CNewUIObj side (matching
    // every pre-existing CNewUIObj window), so this window keeps its own bounding box, same as
    // CServerSelWin/CLoginMainWin's established pattern.
    POINT m_ptPos = {};
    SIZE m_Size = {};

public:
    CLoginWin();
    ~CLoginWin() override;
    void Create();
    // Was PreRelease() (a CWin override hook Release() called automatically) -- called explicitly
    // now, same as every other migrated window's Release().
    void Release();
    void SetPosition(int nXCoord, int nYCoord);
    void Show(bool bShow) override;

    void ConnectConnectionServer();

    CUITextInputBox* GetUsernameInputBox() const
    {
        return m_pUsernameInputBox;
    }
    CUITextInputBox* GetPasswordInputBox() const
    {
        return m_pPasswordInputBox;
    }
    int GetWidth() const
    {
        return m_Size.cx;
    }
    int GetHeight() const
    {
        return m_Size.cy;
    }

    // Draws the actual username/password text -- called explicitly, AFTER RmlUi has rendered for
    // the frame, from Winmain.cpp's SetPostRmlUiCallback (the theme-independent, canonical call
    // site) and also inline from Render() below (the legacy-theme-only shortcut, since that
    // theme's panel is transparent and draw order doesn't matter there -- see Render()'s own
    // comment).
    void RenderTextOnTop();

    // Called from the RmlUi login document's data-event-click callbacks (see Create()'s
    // DataModelConstructor::BindEventCallback registrations). 2026-09-03: act immediately here
    // instead of setting a flag for UpdateWhileActive() to consume later -- see
    // CLoginMainWin::RmlClickMenu()'s header comment for why: UpdateWhileActive() is gated behind
    // IsActive(), which this class now computes dynamically every frame (see UpdateWhileShown()'s
    // comment) rather than granting synchronously on click, so a click could still sit unconsumed
    // for a frame if it went through that path instead. Confirmed safe to call straight into
    // RequestLogin()/CancelLogin()/etc. here for the same reason as CLoginMainWin's fix: this
    // fires from RmlUiRuntime::ProcessSdlEvent(), called from Winmain's SDL event pump, always
    // before CUIMng::Update() runs the same frame. Each still re-checks the "remember password"
    // prompt's Pending state directly (the same guard UpdateWhileShown()'s SetActive() computation
    // applies) since these callbacks run earlier in the frame, before UpdateWhileShown()'s Tick()
    // call has had a chance to resolve anything.
    void RmlClickOk();
    void RmlClickCancel();
    void RmlToggleRememberMe();
    void RmlToggleSavePassword();

    // SEASON3B::INewUIBase
    bool Render() override;
    // Was CWin::CursorInWin(WA_ALL) -- claims (consumes) any click within its own bounding box,
    // same template CServerSelWin/CLoginMainWin already established. Not modal: this floating
    // dialog must leave the world/credits/system-menu reachable around it. Depth 20.0f (below
    // CSysMenuWin's 40.0f/CMsgWin's 50.0f/CCreditWin's 100.0f) means the descending-depth mouse
    // dispatch already stops at any of those windows' own full-screen claims before ever reaching
    // this rect check while one of them is shown, so no explicit modal check is needed here.
    bool UpdateMouseEvent() override;
    bool UpdateKeyEvent() override
    {
        return true;
    }
    float GetLayerDepth() override
    {
        return 20.0f;
    }

protected:
    // The shown-vs-active split (CNewUIObj/NewUIBase.h, added in Phase 0 -- first real use here).
    // UpdateWhileShown() always runs while shown: keeps ticking text-input state and the
    // Remember-Password sub-dialog even while something else has taken over input. Its first
    // statement computes and pushes this frame's IsActive() via SetActive() -- true unless
    // CCreditWin/CMsgWin/CSysMenuWin is currently covering this dialog (same "a higher-depth
    // modal claiming UpdateMouseEvent() doesn't stop a lower window's own Update()" gap already
    // fixed once for CCharSelMainWin/CCharMakeWin) or the Remember-Password prompt is pending --
    // read BEFORE Tick() (below) can resolve it, so UpdateWhileActive() (which CNewUIObj::Update()
    // only calls if this frame's IsActive() came back true) sees the same "was pending this frame"
    // snapshot the prompt needs, without a separate member to hold it.
    bool UpdateWhileShown() override;
    // Runs only while also active (see above) -- OK/Cancel/Enter/Esc submit handling.
    bool UpdateWhileActive() override;

    void RequestLogin();
    void CancelLogin();

    // "Remember me" credential handling, split out of the update loop.
    void UpdateRememberCheckboxes();
    void ApplyRememberPasswordChoice();
    void RevokeSavedCredentialsIfEdited();

private:
    int FirstLoad = 0;

    // Shared by the immediate RmlUi callbacks above and UpdateWhileActive()'s legacy polling
    // (CButton::IsClick(), keyboard). Each re-checks the "remember password" prompt's live Pending
    // state itself, correct from both call sites since UpdateWhileActive() never even runs while
    // pending (see UpdateWhileShown()'s comment) and the immediate RmlUi callbacks haven't had a
    // chance to observe that via the same path.
    void SubmitLogin();
    void SubmitCancel();
    void ApplyRememberMeChange();
    void ApplySavePasswordChange();

    struct LoginRmlModel
    {
        bool rememberMeChecked = false;
        bool savePasswordChecked = false;
        Rml::String serverName;
        // Synced from the same I18N::Game::* slots RenderControls() used to feed directly into
        // g_pRenderText -- keeps this migration localization-correct rather than hardcoding
        // English strings into the RML/RCSS.
        Rml::String accountLabel;
        Rml::String passwordLabel;
        Rml::String rememberMeLabel;
        Rml::String savePasswordLabel;
        Rml::String trustWarning;
        Rml::String okLabel;
        Rml::String cancelLabel;
    };
    RmlModelBinder<LoginRmlModel> m_RmlBinder;
    Rml::ElementDocument* m_pRmlDoc = nullptr;

    void SyncRmlModel();
};

// Replaces CUIMng's old `CLoginWin m_LoginWin;` member, same convention as g_CreditWin.
extern CLoginWin g_LoginWin;
