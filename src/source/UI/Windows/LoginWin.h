//*****************************************************************************
// File: LoginWin.h
//*****************************************************************************
#pragma once

#include "UI/Widgets/Win.h"

#include "UI/Widgets/Button.h"
#include "UI/RmlBridge/RmlModelBinder.h"

class CUITextInputBox;

namespace Rml { class ElementDocument; }

// RmlUi migration plan Phase 1 pilot (login dialog): the legacy CButton/CSprite objects below
// remain the actual state-holders (IsCheck/SetCheck, position/size bookkeeping via CWin) --
// nothing about their internal semantics changed. What changed is how they're driven: an RmlUi
// document (Data/Interface/RmlUi/login.rml) now renders the checkboxes/buttons/labels/trust
// warning as an overlay positioned on top of CWin::Render()'s existing background sprite
// (m_psprBg, still drawn exactly as before -- not reproduced in RmlUi, to avoid re-authoring a
// working panel graphic and to minimize regression risk with no compiler available to verify
// against). Username/password text entry deliberately stays on the legacy CUITextInputBox
// objects (m_pUsernameInputBox/m_pPasswordInputBox) rather than moving to native RmlUi <input>
// elements -- external code (WSclient.cpp, MsgWin.cpp) calls GetUsernameInputBox()/
// GetPasswordInputBox()->GiveFocus() directly for error-recovery focus redirection, and
// duplicating credential-entry/focus logic into a second, independent text-input system this
// session can't test is a real regression risk not worth taking for this pass.
class CLoginWin : public CWin
{
protected:
    CSprite		m_asprInputBox[2];
    CButton		m_aBtn[2];
    CButton     m_aBtnRememberMe;
    CButton     m_aBtnSavePassword;
    CUITextInputBox* m_pUsernameInputBox, * m_pPasswordInputBox;

    // Snapshot of the field contents, used to detect that the player edited the
    // username or password so the stored credentials can be dropped.
    wchar_t     m_prevUsername[MAX_USERNAME_SIZE + 1] = {};
    wchar_t     m_prevPassword[MAX_PASSWORD_SIZE + 1] = {};

public:
    CLoginWin();
    virtual ~CLoginWin();
    void Create();
    void SetPosition(int nXCoord, int nYCoord);
    void Show(bool bShow);
    bool CursorInWin(int nArea);

    void ConnectConnectionServer();

    CUITextInputBox* GetUsernameInputBox() const { return m_pUsernameInputBox; }
    CUITextInputBox* GetPasswordInputBox() const { return m_pPasswordInputBox; }

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
