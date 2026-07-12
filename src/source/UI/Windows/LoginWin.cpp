//*****************************************************************************
// File: LoginWin.cpp
//*****************************************************************************

#include "stdafx.h"
#include "UI/Windows/LoginWin.h"
#include "Core/Input/Input.h"
#include "UI/Legacy/UIMng.h"
#include "Render/Models/ZzzBMD.h"
#include "Engine/Object/ZzzInfomation.h"
#include "Engine/Object/ZzzObject.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInterface.h"
#include "Network/Reconnect/ReconnectManager.h"
#include "UI/Legacy/UIControls.h"
#include "Scenes/SceneCore.h"
#include "I18N/All.h"

#include "Audio/DSPlaySound.h"
#include "UI/NewUI/NewUISystem.h"
#include "UI/NewUI/Dialogs/NewUIMessageBox.h"
#include "UI/Windows/RememberPasswordPrompt.h"


#include "Network/Server/ServerListManager.h"
#ifdef _WIN32
#include <dpapi.h>
#endif

#include "Data/GameConfig/GameConfig.h"
#include "Data/GameConfig/GameConfigConstants.h"

#define	LIW_ACCOUNT		0
#define	LIW_PASSWORD	1

#define LIW_OK			0
#define LIW_CANCEL		1



extern int g_iChatInputType;
extern int  LogIn;
extern wchar_t LogInID[MAX_USERNAME_SIZE + 1];
extern BYTE Version[SIZE_PROTOCOLVERSION];
extern BYTE Serial[SIZE_PROTOCOLSERIAL + 1];

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

    // The login background is fixed artwork and does not scale with the window
    // size, so keep the original height. The credential-consent controls fit on
    // the panel and the trust warning is drawn just below it (issue #462).
    CWin::Create(329, 245, BITMAP_LOG_IN + 7);

    m_asprInputBox[LIW_ACCOUNT].Create(156, 23, BITMAP_LOG_IN + 8);
    m_asprInputBox[LIW_PASSWORD].Create(156, 23, BITMAP_LOG_IN + 8);

    for (int i = 0; i < 2; ++i)
    {
        m_aBtn[i].Create(54, 30, BITMAP_BUTTON + i, 3, 2, 1);
        CWin::RegisterButton(&m_aBtn[i]);
    }

    m_aBtnRememberMe.Create(16, 16, BITMAP_CHECK_BTN, 2, 0, 0, -1, 1, 1, 1);
    CWin::RegisterButton(&m_aBtnRememberMe);

    m_aBtnSavePassword.Create(16, 16, BITMAP_CHECK_BTN, 2, 0, 0, -1, 1, 1, 1);
    CWin::RegisterButton(&m_aBtnSavePassword);

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
}

void CLoginWin::PreRelease()
{
    for (int i = 0; i < 2; ++i)
        m_asprInputBox[i].Release();
}

void CLoginWin::SetPosition(int x, int y)
{
	CWin::SetPosition(x, y);

	const int boxOffsetX = x + 109;
	m_asprInputBox[LIW_ACCOUNT].SetPosition(boxOffsetX, y + 106);
	m_asprInputBox[LIW_PASSWORD].SetPosition(boxOffsetX, y + 131);

	if (g_iChatInputType == 1)
	{
		const int boxX = int((x + 115) / g_fScreenRate_x);
		m_pUsernameInputBox->SetPosition(boxX, int((y + 112) / g_fScreenRate_y));
		m_pPasswordInputBox->SetPosition(boxX, int((y + 137) / g_fScreenRate_y));
	}

	// "Remember Username" (row 1) and "Remember Password" (row 2) stack
	// vertically; the OK/Cancel buttons move down to make room. These pixel
	// offsets are eyeballed against the login background and may need tuning.
	m_aBtnRememberMe.SetPosition(x + 109, y + 156);
	m_aBtnSavePassword.SetPosition(x + 109, y + 176);
	m_aBtn[LIW_OK].SetPosition(x + 150, y + 200);
	m_aBtn[LIW_CANCEL].SetPosition(x + 211, y + 200);
}

void CLoginWin::Show(bool bShow)
{
    CWin::Show(bShow);

    for (int i = 0; i < 2; ++i)
    {
        m_asprInputBox[i].Show(bShow);
        m_aBtn[i].Show(bShow);
    }
    m_aBtnRememberMe.Show(bShow);
    m_aBtnSavePassword.Show(bShow);

    // Drive the text fields' state so a hidden login screen releases keyboard
    // focus (portable fields stop SDL text input when hidden, #447).
    const int iState = bShow ? UISTATE_NORMAL : UISTATE_HIDE;
    if (m_pUsernameInputBox) m_pUsernameInputBox->SetState(iState);
    if (m_pPasswordInputBox) m_pPasswordInputBox->SetState(iState);
}

bool CLoginWin::CursorInWin(int nArea)
{
    if (!CWin::m_bShow)
        return false;

    switch (nArea)
    {
    case WA_MOVE:
        return false;
    }

    return CWin::CursorInWin(nArea);
}

void CLoginWin::UpdateWhileActive(double)
{
	// While the "Remember Password" confirmation dialog is open, let it own the
	// input so Enter/Esc/clicks don't also drive the login screen behind it. The
	// dialog's outcome is applied on the next frame, once it has closed.
	if (!g_MessageBox->IsEmpty())
		return;

	if (m_aBtn[LIW_OK].IsClick() || CInput::Instance().IsKeyDown(VK_RETURN))
	{
		PlayBuffer(SOUND_CLICK01);
		RequestLogin();
		return;
	}

	if (m_aBtn[LIW_CANCEL].IsClick() || CInput::Instance().IsKeyDown(VK_ESCAPE))
	{
		PlayBuffer(SOUND_CLICK01);
		CancelLogin();
		CUIMng::Instance().SetSysMenuWinShow(false);
		return;
	}

	UpdateRememberCheckboxes();
}

void CLoginWin::UpdateRememberCheckboxes()
{
	GameConfig& config = GameConfig::GetInstance();

	if (m_aBtnRememberMe.IsClick())
	{
		m_RememberMe = m_aBtnRememberMe.IsCheck();
		config.SetRememberMe(m_RememberMe != 0);

		// Saving the password requires remembering the account, so drop the
		// password consent when "remember me" is switched off.
		if (!m_RememberMe && m_aBtnSavePassword.IsCheck())
		{
			m_aBtnSavePassword.SetCheck(false);
			config.SetSavePassword(false);
		}
	}

	if (!m_aBtnSavePassword.IsClick())
		return;

	if (!m_aBtnSavePassword.IsCheck())
	{
		// Player unticked it: revoke the stored password immediately.
		config.SetSavePassword(false);
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

void CLoginWin::UpdateWhileShow(double dDeltaTick)
{
    m_pUsernameInputBox->DoAction();
    m_pPasswordInputBox->DoAction();

    ApplyRememberPasswordChoice();
    RevokeSavedCredentialsIfEdited();
}

void CLoginWin::ApplyRememberPasswordChoice()
{
    // Applied here rather than in UpdateWhileActive because the modal message box
    // leaves the login window inactive (so UpdateWhileActive stops running),
    // while UpdateWhileShow keeps being called.
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

void CLoginWin::RenderControls()
{
    if (FirstLoad)
    {
        (wcslen(m_Username) > 0 ? m_pPasswordInputBox : m_pUsernameInputBox)->GiveFocus();
        FirstLoad = 0;
    }

    CWin::RenderButtons();
    m_asprInputBox[LIW_ACCOUNT].Render();
    m_asprInputBox[LIW_PASSWORD].Render();
    m_pUsernameInputBox->Render();
    m_pPasswordInputBox->Render();

    g_pRenderText->SetFont(g_hFixFont);
    g_pRenderText->SetBgColor(0);
    g_pRenderText->SetTextColor(CLRDW_WHITE);

    const int baseX = GetXPos();
    const int baseY = GetYPos();

    g_pRenderText->RenderText(int((baseX + 30) / g_fScreenRate_x), int((baseY + 113) / g_fScreenRate_y), I18N::Game::Account);
    g_pRenderText->RenderText(int((baseX + 30) / g_fScreenRate_x), int((baseY + 139) / g_fScreenRate_y), I18N::Game::Password);

    wchar_t szServerName[MAX_TEXT_LENGTH] = {};
    const wchar_t* pServerStatus = g_ServerListManager->GetNonPVPInfo() ? I18N::Game::SDServer : I18N::Game::SDNonPvPServer;
    mu_swprintf(szServerName, pServerStatus, g_ServerListManager->GetSelectServerName(), g_ServerListManager->GetSelectServerIndex());
    g_pRenderText->RenderText(int((baseX + 111) / g_fScreenRate_x), int((baseY + 80) / g_fScreenRate_y), szServerName);

    g_pRenderText->RenderText(int((baseX + 130) / g_fScreenRate_x), int((baseY + 159) / g_fScreenRate_y), L"Remember Username");
    g_pRenderText->RenderText(int((baseX + 130) / g_fScreenRate_x), int((baseY + 179) / g_fScreenRate_y), L"Remember Password");

    // Trust warning rendered just below the login panel (the panel artwork is a
    // fixed size, so there is no room for this long line inside it). Position is
    // eyeballed against the background and may need tuning.
    g_pRenderText->SetTextColor(255, 210, 60, 255);
    g_pRenderText->RenderText(int((baseX + 30) / g_fScreenRate_x), int((baseY + 252) / g_fScreenRate_y), L"Only save the password on a PC you trust.");
    g_pRenderText->SetTextColor(CLRDW_WHITE);
}

void CLoginWin::RequestLogin()
{
    if (CurrentProtocolState == REQUEST_JOIN_SERVER)
        return;

    CUIMng::Instance().HideWin(this);

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
        CUIMng::Instance().PopUpMsgWin(MESSAGE_INPUT_ID);
    else if (wcslen(m_Password) <= 0)
        CUIMng::Instance().PopUpMsgWin(MESSAGE_INPUT_PASSWORD);
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

            g_pSystemLogBox->AddText(I18N::Game::VerifyingYourAccount, SEASON3B::TYPE_SYSTEM_MESSAGE);
            g_pSystemLogBox->AddText(I18N::Game::PleaseWait, SEASON3B::TYPE_SYSTEM_MESSAGE);
        }
    }
}

void CLoginWin::CancelLogin()
{
    ConnectConnectionServer();
    CUIMng::Instance().HideWin(this);
}

void CLoginWin::ConnectConnectionServer()
{
    LogIn = 0;
    CurrentProtocolState = REQUEST_JOIN_SERVER;
    CreateSocket(szServerIpAddress, g_ServerPort);
}