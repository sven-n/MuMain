//*****************************************************************************
// File: LoginWin.h
//*****************************************************************************
#pragma once

#include "UI/Widgets/Win.h"

#include "UI/Widgets/Button.h"
#include "UI/RmlBridge/RmlModelBinder.h"

class CUITextInputBox;

namespace Rml { class ElementDocument; }

// RmlUi migration plan Phase 1 pilot (login dialog): CWin no longer draws anything for this
// window -- Create() always passes nTexID=-2, so m_psprBg is never created, and the input-box
// frame sprites this class used to own (m_asprInputBox) are gone entirely. The RmlUi document
// (Data/Interface/RmlUi/login.rml) renders the panel background, input-box frames, checkboxes,
// buttons, labels, and trust warning as an overlay -- the "legacy" theme reproduces the original
// look by pointing its RCSS decorators at the same art files (Interface/login_back.tga,
// Interface/login_me.tga) the old CWin sprites drew, so this is a renderer swap, not a visual
// change. The legacy CButton objects below remain the actual state-holders (IsCheck/SetCheck) for
// the checkboxes/OK/Cancel buttons -- nothing about their internal semantics changed, only what
// draws them. Username/password text entry deliberately stays on the legacy CUITextInputBox
// objects (m_pUsernameInputBox/m_pPasswordInputBox) rather than moving to native RmlUi <input>
// elements -- external code (WSclient.cpp, MsgWin.cpp) calls GetUsernameInputBox()/
// GetPasswordInputBox()->GiveFocus() directly for error-recovery focus redirection, and
// duplicating credential-entry/focus logic into a second, independent text-input system this
// session can't test is a real regression risk not worth taking for this pass.
class CLoginWin : public CWin
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

    // Snapshot of the "Remember Password" dialog's Pending state, taken in UpdateWhileShow()
    // BEFORE UI::Login::Tick() can resolve it -- see UpdateWhileActive()'s own comment for why
    // this is needed instead of just re-checking RememberPasswordChoiceState() live there.
    bool m_bRememberPasswordPromptWasPending = false;

public:
    CLoginWin();
    virtual ~CLoginWin();
    void Create();
    void SetPosition(int nXCoord, int nYCoord);
    void Show(bool bShow);

    void ConnectConnectionServer();

    CUITextInputBox* GetUsernameInputBox() const
    {
        return m_pUsernameInputBox;
    }
    CUITextInputBox* GetPasswordInputBox() const
    {
        return m_pPasswordInputBox;
    }

    // Draws the actual username/password text -- called explicitly, AFTER RmlUi has rendered
    // for the frame (see RenderControls()'s own comment for why -- deferred here rather than
    // wired to a real "after RmlUi" call site yet since this branch's single-pre-submit-callback
    // architecture doesn't have one exposed for scene-specific overlays; see
    // .ai-os/memory/tasks/rmlui-sdl-gpu-port.md). Under the default "legacy" theme (transparent
    // panel) the ordering doesn't matter and RenderControls() calls this directly today.
    void RenderTextOnTop();

    // Called from the RmlUi login document's data-event-click callbacks (see Create()'s
    // DataModelConstructor::BindEventCallback registrations). Polled-and-cleared exactly like
    // the legacy CButton::IsClick() edge triggers they supplement in UpdateWhileActive()/
    // UpdateRememberCheckboxes() -- kept as separate flags rather than faking IsClick() so the
    // legacy CButton objects themselves (m_aBtn/m_aBtnRememberMe/m_aBtnSavePassword) are
    // untouched and still own real check-state for every existing internal call site
    // (RequestLogin, ApplyRememberPasswordChoice, etc).
    void RmlClickOk() { m_bRmlOkClicked = true; }
    void RmlClickCancel() { m_bRmlCancelClicked = true; }
    void RmlToggleRememberMe()
    {
        m_aBtnRememberMe.SetCheck(!m_aBtnRememberMe.IsCheck());
        m_bRmlRememberMeClicked = true;
    }
    void RmlToggleSavePassword()
    {
        m_aBtnSavePassword.SetCheck(!m_aBtnSavePassword.IsCheck());
        m_bRmlSavePasswordClicked = true;
    }

private:
    int FirstLoad = 0;

    bool m_bRmlOkClicked = false;
    bool m_bRmlCancelClicked = false;
    bool m_bRmlRememberMeClicked = false;
    bool m_bRmlSavePasswordClicked = false;

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

protected:
    void PreRelease();
    void UpdateWhileActive(double dDeltaTick);
    void UpdateWhileShow(double dDeltaTick);
    void RenderControls();
    void RequestLogin();
    void CancelLogin();

    // "Remember me" credential handling, split out of the update loop.
    void UpdateRememberCheckboxes();
    void ApplyRememberPasswordChoice();
    void RevokeSavedCredentialsIfEdited();
};
