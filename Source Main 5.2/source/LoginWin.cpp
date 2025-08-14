//*****************************************************************************
// File: LoginWin.cpp
//*****************************************************************************

#include "stdafx.h"
#include "LoginWin.h"
#include "Input.h"
#include "UIMng.h"
#include "ZzzBMD.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzInterface.h"
#include "UIControls.h"
#include "ZzzScene.h"

#include "DSPlaySound.h"
#include "NewUISystem.h"


#include "ServerListManager.h"
#include <dpapi.h>

#define	LIW_ACCOUNT		0
#define	LIW_PASSWORD	1

#define LIW_OK			0
#define LIW_CANCEL		1



extern int g_iChatInputType;
extern int  LogIn;
extern wchar_t LogInID[MAX_ID_SIZE + 1];
extern BYTE Version[SIZE_PROTOCOLVERSION];
extern BYTE Serial[SIZE_PROTOCOLSERIAL + 1];

CLoginWin::CLoginWin()
{
    m_pIDInputBox = NULL;
    m_pPassInputBox = NULL;
}

CLoginWin::~CLoginWin()
{
    SAFE_DELETE(m_pIDInputBox);
    SAFE_DELETE(m_pPassInputBox);
}

void CLoginWin::Create()
{
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

    SAFE_DELETE(m_pIDInputBox);

    m_pIDInputBox = new CUITextInputBox;
    m_pIDInputBox->Init(g_hWnd, 140, 14, MAX_ID_SIZE);
    m_pIDInputBox->SetBackColor(0, 0, 0, 25);
    m_pIDInputBox->SetTextColor(255, 255, 230, 210);
    m_pIDInputBox->SetFont(g_hFixFont);
    m_pIDInputBox->SetState(UISTATE_NORMAL);
    m_pIDInputBox->SetText(m_ID);

    SAFE_DELETE(m_pPassInputBox);

    m_pPassInputBox = new CUITextInputBox;
    m_pPassInputBox->Init(g_hWnd, 140, 14, MAX_PASSWORD_SIZE, TRUE);
    m_pPassInputBox->SetBackColor(0, 0, 0, 25);
    m_pPassInputBox->SetTextColor(255, 255, 230, 210);
    m_pPassInputBox->SetFont(g_hFixFont);
    m_pPassInputBox->SetState(UISTATE_NORMAL);

    m_pIDInputBox->SetTabTarget(m_pPassInputBox);
    m_pPassInputBox->SetTabTarget(m_pIDInputBox);

    if (m_RememberMe) {
        m_pPassInputBox->SetText(m_Password);
        m_aBtnRememberMe.SetCheck(true);
    }

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
		m_pIDInputBox->SetPosition(boxX, int((y + 112) / g_fScreenRate_y));
		m_pPassInputBox->SetPosition(boxX, int((y + 137) / g_fScreenRate_y));
	}

	m_aBtn[LIW_OK].SetPosition(x + 150, y + 178);
	m_aBtn[LIW_CANCEL].SetPosition(x + 211, y + 178);
	m_aBtnRememberMe.SetPosition(x + 109, y + 156);
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

	if (m_aBtnRememberMe.IsClick())
		m_RememberMe = m_aBtnRememberMe.IsCheck();
}

void CLoginWin::UpdateWhileShow(double dDeltaTick)
{
    m_pIDInputBox->DoAction();
    m_pPassInputBox->DoAction();
}

void CLoginWin::RenderControls()
{
    if (FirstLoad)
    {
        (wcslen(m_ID) > 0 ? m_pPassInputBox : m_pIDInputBox)->GiveFocus();
        FirstLoad = 0;
    }

    CWin::RenderButtons();
    m_asprInputBox[LIW_ACCOUNT].Render();
    m_asprInputBox[LIW_PASSWORD].Render();
    m_pIDInputBox->Render();
    m_pPassInputBox->Render();

    g_pRenderText->SetFont(g_hFixFont);
    g_pRenderText->SetBgColor(0);
    g_pRenderText->SetTextColor(CLRDW_WHITE);

    const int baseX = GetXPos();
    const int baseY = GetYPos();

    g_pRenderText->RenderText(int((baseX + 30) / g_fScreenRate_x), int((baseY + 113) / g_fScreenRate_y), GlobalText[450]);
    g_pRenderText->RenderText(int((baseX + 30) / g_fScreenRate_x), int((baseY + 139) / g_fScreenRate_y), GlobalText[451]);

    wchar_t szServerName[MAX_TEXT_LENGTH] = {};
    const wchar_t* pServerStatus = g_ServerListManager->GetNonPVPInfo() ? GlobalText[461] : GlobalText[460];
    swprintf(szServerName, pServerStatus, g_ServerListManager->GetSelectServerName(), g_ServerListManager->GetSelectServerIndex());
    g_pRenderText->RenderText(int((baseX + 111) / g_fScreenRate_x), int((baseY + 80) / g_fScreenRate_y), szServerName);

    g_pRenderText->RenderText(int((baseX + 130) / g_fScreenRate_x), int((baseY + 159) / g_fScreenRate_y), L"Remember me?");
}

void CLoginWin::RequestLogin()
{
    if (CurrentProtocolState == REQUEST_JOIN_SERVER)
        return;

    CUIMng::Instance().HideWin(this);

    m_pIDInputBox->GetText(m_ID, MAX_ID_SIZE + 1);
    m_pPassInputBox->GetText(m_Password, MAX_PASSWORD_SIZE + 1);

    // Start save account info to registry
    HKEY hKey;
    RegCreateKeyEx(HKEY_CURRENT_USER, L"SOFTWARE\\Webzen\\Mu\\Config", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
    RegSetValueEx(hKey, L"ID", 0, REG_SZ, (BYTE*)m_ID, sizeof(m_ID));

    DATA_BLOB dataIn;
    DATA_BLOB dataOut;
    dataIn.cbData = sizeof(m_Password);
    dataIn.pbData = m_aBtnRememberMe.IsCheck() ? (BYTE*)m_Password : (BYTE*)"";

    if (CryptProtectData(&dataIn, NULL, NULL, NULL, NULL, 0, &dataOut))
    {
        RegSetValueEx(hKey, L"Password", 0, REG_BINARY, dataOut.pbData, dataOut.cbData);
        LocalFree(dataOut.pbData);
    }

    RegSetValueEx(hKey, L"RememberMe", 0, REG_DWORD, (BYTE*)&m_RememberMe, sizeof(m_RememberMe));
    // End save account info to registry

    if (wcslen(m_ID) <= 0)
        CUIMng::Instance().PopUpMsgWin(MESSAGE_INPUT_ID);
    else if (wcslen(m_Password) <= 0)
        CUIMng::Instance().PopUpMsgWin(MESSAGE_INPUT_PASSWORD);
    else
    {
        if (CurrentProtocolState == RECEIVE_JOIN_SERVER_SUCCESS)
        {
            g_ConsoleDebug->Write(MCD_NORMAL, L"Login with the following account: %s", m_ID);

            g_ErrorReport.Write(L"> Login Request.\r\n");
            g_ErrorReport.Write(L"> Try to Login \"%s\"\r\n", m_ID);

            LogIn = 1;
            wcscpy(LogInID, (m_ID));
            CurrentProtocolState = REQUEST_LOG_IN;

            SocketClient->ToGameServer()->SendLogin(m_ID, m_Password, Version, Serial);

            g_pSystemLogBox->AddText(GlobalText[472], SEASON3B::TYPE_SYSTEM_MESSAGE);
            g_pSystemLogBox->AddText(GlobalText[473], SEASON3B::TYPE_SYSTEM_MESSAGE);
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