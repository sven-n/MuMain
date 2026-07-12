//*****************************************************************************
// File: LoginWin.h
//*****************************************************************************
#pragma once

#include "UI/Widgets/Win.h"

#include "UI/Widgets/Button.h"

class CUITextInputBox;

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

private:
    int FirstLoad = 0;

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
