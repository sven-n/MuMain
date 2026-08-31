//*****************************************************************************
// File: MsgWin.h
//*****************************************************************************
#pragma once

#include "UI/Widgets/Win.h"
#include "UI/Widgets/Button.h"
#include "UI/RmlBridge/RmlModelBinder.h"

#define MW_MSG_LINE_MAX 2
#define MW_MSG_ROW_MAX 52

namespace Rml { class ElementDocument; }

// RmlUi migration: the generic message/confirm dialog, following the same hybrid CWin+RmlUi
// pattern as CLoginWin/CCharMakeWin/CCharSelMainWin. Shown from both LOG_IN_SCENE and
// CHARACTER_SCENE (CUIMng::CreateLoginScene()/CreateCharacterScene() both Create() it;
// CreateMainScene() never does -- a pre-existing characteristic, not something this port changes
// or needs to cover).
//
// 2026-08-31, built under docs/rmlui-ui-system/layout-and-scaling.md from day one: the panel is
// centered via base.rcss's `.center-both` utility class with a fixed `dp` size, not a C++-pushed
// rect. CWin::Create() still spans the full screen exactly as before the port -- that is what
// makes CUIMng::IsCursorOnUI() report true for any cursor position while this dialog is shown,
// regardless of the RmlUi panel's own (much smaller, centered) footprint, so a modal message box
// genuinely swallows every click no matter how imprecisely the legacy CButton bookkeeping below
// lines up with the RmlUi visuals -- unlike CCharSelMainWin, this window has no legitimate
// "world click" competing for input that a hit-test mismatch could wrongly let through.
class CMsgWin : public CWin
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
    virtual ~CMsgWin();
    void Create();
    void SetPosition(int nXCoord, int nYCoord);
    void Show(bool bShow);
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

protected:
    void PreRelease();
    void UpdateWhileActive(double dDeltaTick);
    void RenderControls();
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
