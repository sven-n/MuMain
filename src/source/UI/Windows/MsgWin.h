//*****************************************************************************
// File: MsgWin.h
//*****************************************************************************
#pragma once

#include "UI/Core/WindowObject.h"
#include "Render/Sprites/Sprite.h"
#include "UI/RmlBridge/RmlModelBinder.h"

#define MW_MSG_LINE_MAX 2
#define MW_MSG_ROW_MAX 52

namespace Rml { class ElementDocument; }

// Generic message/confirm dialog (hybrid CWin+RmlUi), shown from the login and character scenes.
// The panel is centered via base.rcss's `.center-both` utility class, not a C++-pushed rect.
class CMsgWin : public mu::ui::window::CObject
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
    void Release(); // Called explicitly at scene transitions.
    void SetPosition(int nXCoord, int nYCoord);
    void Show(bool bShow) override;
    void PopUp(int nMsgCode, wchar_t* pszMsg = nullptr);

    // Set by RmlUi click bindings; polled and cleared in Update().
    void RmlClickOk() { m_bRmlOkClicked = true; }
    void RmlClickCancel() { m_bRmlCancelClicked = true; }

    // Draws the resident-password (MWT_STR_INPUT) live text over RmlUi's input-frame background.
    // No-op outside MWT_STR_INPUT.
    void RenderTextOnTop();

    // mu::ui::window::IObject
    bool Render() override;
    bool Update() override;
    // Unconditionally claims clicks while shown; no rect check needed.
    bool UpdateMouseEvent() override
    {
        return !IsVisible();
    }
    bool UpdateKeyEvent() override
    {
        return true;
    }
    // Below CCreditWin's full-screen-exclusive layer; the two are not expected to coexist.
    float GetLayerDepth() override
    {
        return 50.0f;
    }
    void ReloadRmlTheme() override;

protected:
    void BuildRmlUi();
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
        // Mutually exclusive; mirror MSG_WIN_TYPE. Drive button visibility/layout in msg_win.rcss.
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

extern CMsgWin g_MsgWin;
