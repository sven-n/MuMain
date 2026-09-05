//*****************************************************************************
// File: MsgWin.h
//*****************************************************************************
#pragma once

#include "UI/Core/NewUIBase.h"
#include "UI/Widgets/Button.h"
#include "UI/RmlBridge/RmlModelBinder.h"

#define MW_MSG_LINE_MAX 2
#define MW_MSG_ROW_MAX 52

namespace Rml { class ElementDocument; }

// RmlUi migration: the generic message/confirm dialog, following the same hybrid CWin+RmlUi
// pattern as CLoginWin/CCharMakeWin/CCharSelMainWin. Shown from both LOG_IN_SCENE and
// CHARACTER_SCENE (CSceneUICoordinator::CreateLoginScene()/CreateCharacterScene() both Create() it;
// CreateMainScene() never does -- a pre-existing characteristic, not something this port changes
// or needs to cover).
//
// 2026-08-31, built under docs/rmlui-ui-system/layout-and-scaling.md from day one: the panel is
// centered via base.rcss's `.center-both` utility class with a fixed `dp` size, not a C++-pushed
// rect.
//
// CUIMng/CNewUIManager merger (docs/newui-legacy-merger.md) Phase 2: migrated off CWin onto
// mu::ui::window::CNewUIObj. Previously, CWin::Create() spanning the full screen was what made
// CUIMng::IsCursorOnUI() report true for any cursor position while this dialog was shown --
// UpdateMouseEvent() below now does that job directly (unconditionally claims the click while
// shown, no rect check needed), and CSceneUICoordinator::Update() folds new-style claims into m_bCursorOnUI
// the same way it already folds them into the legacy click-walk skip. A modal message box
// genuinely swallows every click no matter how imprecisely the legacy CButton bookkeeping below
// lines up with the RmlUi visuals -- unlike CCharSelMainWin, this window has no legitimate
// "world click" competing for input that a hit-test mismatch could wrongly let through.
class CMsgWin : public mu::ui::window::CNewUIObj
{
protected:
    enum MSG_WIN_TYPE
    {
        MWT_NON,
        MWT_BTN_CANCEL,
        MWT_BTN_OK,
        MWT_BTN_BOTH,
        MWT_STR_INPUT,
    };

    CSprite m_sprBack;
    CSprite m_sprInput;
    CButton m_aBtn[2];
    wchar_t m_aszMsg[MW_MSG_LINE_MAX][MW_MSG_ROW_MAX];
    int m_nMsgLine;
    int m_nMsgCode;
    MSG_WIN_TYPE m_eType;
    short m_nGameExit;
    double m_dDeltaTickSum;

public:
    CMsgWin();
    ~CMsgWin() override;
    void Create();
    void Release(); // was CWin::PreRelease() (an override hook CWin::Release() called
                     // automatically) -- called explicitly now, same as CCreditWin's own Release().
    void SetPosition(int nXCoord, int nYCoord);
    void Show(bool bShow) override;
    void PopUp(int nMsgCode, wchar_t* pszMsg = nullptr);

    // Invoked from the RmlUi document's data-event-click bindings (see Create()). Polled-and-
    // cleared exactly like every other migrated window's RmlClickX() pattern.
    void RmlClickOk() { m_bRmlOkClicked = true; }
    void RmlClickCancel() { m_bRmlCancelClicked = true; }

    // Draws the resident-password (MWT_STR_INPUT) live text on top of RmlUi's input-frame
    // background -- called from Winmain.cpp's SetPostRmlUiCallback (already registered for
    // LOG_IN_SCENE/CHARACTER_SCENE), same pattern as CLoginWin::RenderTextOnTop(). A no-op outside
    // MWT_STR_INPUT.
    void RenderTextOnTop();

    // mu::ui::window::INewUIBase
    bool Render() override;
    bool Update() override;
    // Was CWin::Create()'s full-screen bounding rect + CWin::CursorInWin(WA_ALL) -- see this
    // class's header comment. Unconditionally claims while shown; no rect check needed.
    bool UpdateMouseEvent() override
    {
        return !IsVisible();
    }
    bool UpdateKeyEvent() override
    {
        return true;
    }
    // Below CCreditWin's full-screen-exclusive 100.0f -- the two are not known to ever coexist in
    // practice (CCreditWin is LOG_IN_SCENE-only decorative content; nothing pops a message box
    // while it's shown), but if they ever did, CCreditWin winning is the safer default.
    float GetLayerDepth() override
    {
        return 50.0f;
    }

protected:
    void SetCtrlPosition();
    void SetMsg(MSG_WIN_TYPE eType, std::wstring lpszMsg, std::wstring lpszMsg2 = L"");
    void ManageOKClick();
    void ManageCancelClick();
    void InitResidentNumInput();
    void RequestDeleteCharacter();

private:
    struct MsgWinRmlModel
    {
        Rml::String line1, line2;
        bool line2Hidden = true;
        bool noButtons = true;
        // Mutually exclusive -- mirror MSG_WIN_TYPE 1:1 (MWT_NON needs none of these set). Drive
        // both button visibility and their per-mode left offset in msg_win.rcss; kept as discrete
        // C++-reported state flags rather than a computed pixel position, consistent with
        // docs/rmlui-ui-system/layout-and-scaling.md's "C++ manages state, RCSS manages layout".
        bool modeCancelOnly = false;
        bool modeOkOnly = false;
        bool modeBoth = false;
        bool modeInput = false;
        Rml::String okLabel, cancelLabel;
    };
    RmlModelBinder<MsgWinRmlModel> m_RmlBinder;
    Rml::ElementDocument* m_pRmlDoc = nullptr;

    bool m_bRmlOkClicked = false;
    bool m_bRmlCancelClicked = false;

    void SyncRmlModel();
};

// Replaces CUIMng's old `CMsgWin m_MsgWin;` member, same convention as g_CreditWin.
extern CMsgWin g_MsgWin;
