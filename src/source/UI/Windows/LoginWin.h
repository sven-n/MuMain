//*****************************************************************************
// File: LoginWin.h
//*****************************************************************************
#pragma once

#include "UI/Core/WindowObject.h"

#include "UI/RmlBridge/RmlModelBinder.h"

#include <atomic>
#include <thread>

class CUITextInputBox;

namespace Rml { class ElementDocument; }

// The login dialog: RmlUi (login.rml) renders the panel chrome, checkboxes, buttons, labels, and
// trust warning. Username/password text entry deliberately stays on legacy CUITextInputBox objects
// (m_pUsernameInputBox/m_pPasswordInputBox) rather than native RmlUi <input> elements, since
// external code calls GetUsernameInputBox()/GetPasswordInputBox()->GiveFocus() directly for
// error-recovery focus redirection.
class CLoginWin : public mu::ui::window::CObject
{
protected:
    bool m_bRememberMeChecked = false;
    bool m_bSavePasswordChecked = false;
    CUITextInputBox* m_pUsernameInputBox;
    CUITextInputBox* m_pPasswordInputBox;

    // Snapshot used to detect that the player edited the username/password, so stored
    // credentials can be dropped.
    wchar_t m_prevUsername[MAX_USERNAME_SIZE + 1] = {};
    wchar_t m_prevPassword[MAX_PASSWORD_SIZE + 1] = {};

    // This window's own bounding box (no shared rect facility on the CObject side).
    POINT m_ptPos = {};
    SIZE m_Size = {};

public:
    CLoginWin();
    ~CLoginWin() override;
    void Create();
    void Release();
    void SetPosition(int nXCoord, int nYCoord);
    void Show(bool bShow) override;

    void ConnectConnectionServer();

    // CancelLogin() runs ConnectConnectionServer() on a background thread since it blocks on a
    // real socket connect; CSceneUICoordinator::Update() polls this each frame and calls
    // ProcessPendingConnectionReconnect() to join once done. m_bConnectionReconnectDone (atomic)
    // is the happens-before point that makes the worker thread's writes visible to the main thread.
    bool HasPendingConnectionReconnect() const
    {
        return m_bConnectionReconnectDone.load();
    }
    void ProcessPendingConnectionReconnect();

    // Sends a login without touching the text boxes or the remembered
    // credentials in config.ini. The manual path calls it after validating
    // what the player typed; automation calls it directly.
    void SubmitCredentials(const wchar_t* pszUsername, const wchar_t* pszPassword);

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

    // Draws the actual username/password text, called from Winmain.cpp's SetPostRmlUiCallback and
    // also inline from Render() for the legacy theme's transparent panel.
    void RenderTextOnTop();

    // Bound to the RmlUi login document's click callbacks. Act immediately rather than setting a
    // flag for UpdateWhileActive() to consume later: this fires from the SDL event pump, always
    // before CSceneUICoordinator::Update() runs the same frame. Each re-checks the "remember
    // password" prompt's Pending state directly since these run before UpdateWhileShown()'s Tick().
    void RmlClickOk();
    void RmlClickCancel();
    void RmlToggleRememberMe();
    void RmlToggleSavePassword();

    // mu::ui::window::IObject
    bool Render() override;
    // Rebuilds this window's RmlUi document/model for the active theme. No-op if never opened.
    void ReloadRmlTheme();
    // Claims clicks within its own bounding box only -- not modal, world/credits/system-menu stay
    // reachable around it. Depth 20.0f is below the full-screen-claiming overlays (CSysMenuWin/
    // CMsgWin/CCreditWin), so no explicit modal check is needed here.
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
    // UpdateWhileShown() always runs while shown: ticks text-input state and the Remember-Password
    // sub-dialog even when something else has taken over input, and computes IsActive() via
    // SetActive() -- false while a higher-depth modal covers this dialog or the Remember-Password
    // prompt is pending.
    bool UpdateWhileShown() override;
    // Runs only while also active -- OK/Cancel/Enter/Esc submit handling.
    bool UpdateWhileActive() override;

    void RequestLogin();
    void CancelLogin();

    // "Remember me" credential handling, split out of the update loop.
    void ApplyRememberPasswordChoice();
    void RevokeSavedCredentialsIfEdited();

private:
    int FirstLoad = 0;
    std::atomic<bool> m_bConnectionReconnectInFlight{false};
    std::atomic<bool> m_bConnectionReconnectDone{false};
    std::thread m_ConnectionReconnectThread;

    // Shared by the immediate RmlUi callbacks and UpdateWhileActive()'s keyboard polling; each
    // re-checks the "remember password" prompt's live Pending state itself.
    void SubmitLogin();
    void SubmitCancel();
    void ApplyRememberMeChange();
    void ApplySavePasswordChange();

    struct LoginRmlModel
    {
        bool rememberMeChecked = false;
        bool savePasswordChecked = false;
        Rml::String serverName;
        // Synced from I18N::Game::* slots to keep these localization-correct.
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
    // Factored out of Create() so ReloadRmlTheme() can re-run it after tearing down the old document.
    void BuildRmlUi();
    // Repositions the two native CUITextInputBox overlays from login.rml's
    // #input_account_anchor/#input_password_anchor every frame -- see this method's own comment.
    void SyncInputBoxPositions();
};

extern CLoginWin g_LoginWin;
