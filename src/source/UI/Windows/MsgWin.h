//*****************************************************************************
// File: MsgWin.h
//*****************************************************************************
#pragma once

#include "UI/Widgets/Win.h"
#include "UI/Widgets/Button.h"

#define MW_MSG_LINE_MAX 2
#define MW_MSG_ROW_MAX 52

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
    bool CursorInWin(int nArea);
    void PopUp(int nMsgCode, wchar_t* pszMsg = nullptr);

    // The message code currently on screen, or -1 when the window is hidden.
    // Automation reads it because several results — a refused login above all —
    // reach the player only as a message box.
    [[nodiscard]] int PendingMessageCode() const;

    // Confirms the message the way the OK button does, so the game's own state
    // restoration runs.
    void DismissMessage();

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
};
