//////////////////////////////////////////////////////////////////////
// OneTimePassword.cpp: implementation of the COneTimePassword class.
// 
// 작성자: 이주흥
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#ifdef LJH_ADD_ONETIME_PASSWORD	
#include "OneTimePassword.h"
#include "Input.h"
#include "UIMng.h"

// 텍스트 랜더를 위한 #include.
#include "ZzzBMD.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzInterface.h"

#include "UIControls.h"
#include "ZzzScene.h"

#include "wsclientinline.h"
#include "DSPlaySound.h"

#include "Local.h"

#define LIW_OK			0	// 확인(localized) 버튼.
#define LIW_CANCEL		1	// 취소(localized) 버튼.

extern float g_fScreenRate_x;
extern float g_fScreenRate_y;
extern int g_iChatInputType;
#ifndef KJH_ADD_SERVER_LIST_SYSTEM			// #ifndef
extern int ServerSelectHi;
extern int ServerLocalSelect;
#endif // KJH_ADD_SERVER_LIST_SYSTEM

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

COneTimePassword::COneTimePassword()
{
	
}

COneTimePassword::~COneTimePassword()
{
	SAFE_DELETE(m_pOTPInputBox);
}

//*****************************************************************************
// 함수 이름 : Create()
// 함수 설명 : 로그인 창 생성.
//*****************************************************************************
void COneTimePassword::Create()
{
// 기본 창 생성.
	CWin::Create(329, 245, BITMAP_LOG_IN+7);

// 텍스트 입력 박스(OTP용) 스프라이트.
	m_sprInputBox.Create(156, 23, BITMAP_LOG_IN+8);

	DWORD adwBtnColor[3] = { CLRDW_BR_GRAY, CLRDW_BR_GRAY, CLRDW_WHITE};
//	버튼 생성.
	for (int i = 0; i < 2; ++i)
	{
		m_aBtn[i].Create(64, 29, SEASON3B::CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL, 3, 2, 1);
		//GlobalText[228]:"확인", GlobalText[3114]:"취소"
		m_aBtn[i].SetText(GlobalText[228+i*2886], adwBtnColor);
		CWin::RegisterButton(&m_aBtn[i]);
		
		/*
		m_aBtn[i].Create(54, 30, BITMAP_BUTTON + i, 3, 2, 1);
		*/
	}

// 텍스트 입력 초기화.
	m_pOTPInputBox = new CUITextInputBox;
	m_pOTPInputBox->Init(g_hWnd, 140, 14, MAX_ONETIME_PASSWORD_SIZE, TRUE);
	m_pOTPInputBox->SetBackColor(255, 0, 0, 0);
	m_pOTPInputBox->SetTextColor(255, 255, 230, 210);
	m_pOTPInputBox->SetFont(g_hFixFont);
	m_pOTPInputBox->SetState(UISTATE_NORMAL);
	m_pOTPInputBox->SetOption(UIOPTION_NUMBERONLY);
}

//*****************************************************************************
// 함수 이름 : SetPosition()
// 함수 설명 : 창 위치 지정.
// 매개 변수 : nXCoord	: 스크린 X좌표.
//			   nYCoord	: 스크린 Y좌표.
//*****************************************************************************
void COneTimePassword::SetPosition(int nXCoord, int nYCoord)
{
	CWin::SetPosition(nXCoord, nYCoord);

	m_sprInputBox.SetPosition(nXCoord + 85, nYCoord + 130);
	
	if (g_iChatInputType == 1)
	{
		// 계정 입력 텍스트.
		m_pOTPInputBox->SetPosition(int((m_sprInputBox.GetXPos() + 6) / g_fScreenRate_x),
			int((m_sprInputBox.GetYPos() + 6) / g_fScreenRate_y));
	}

	m_aBtn[LIW_OK].SetPosition(nXCoord + 90, nYCoord + 167);
	m_aBtn[LIW_CANCEL].SetPosition(nXCoord + 175, nYCoord + 167);
}

//*****************************************************************************
// 함수 이름 : Show()
// 함수 설명 : 창을 보여 주거나 안보이게함.
// 매개 변수 : bShow	: true이면 보여줌.
//*****************************************************************************
void COneTimePassword::Show(bool bShow)
{
	CWin::Show(bShow);

	m_sprInputBox.Show(bShow);
	for (int i = 0; i < 2; ++i)
	{
		m_aBtn[i].Show(bShow);
	}
}

//*****************************************************************************
// 함수 이름 : CursorInWin()
// 함수 설명 : 윈도우 영역 안에 마우스 커서가 위치하는가?
// 매개 변수 : eArea	: 검사할 영역.(win.h의 #define 참조)
//*****************************************************************************
bool COneTimePassword::CursorInWin(int nArea)
{
	if (!CWin::m_bShow)		// 보이지 않는다면 처리하지 않음.
		return false;

	switch (nArea)
	{
	case WA_MOVE:
		return false;	// 이동 영역은 없음.(이동을 막음)
	}

	return CWin::CursorInWin(nArea);
}


//*****************************************************************************
// 함수 이름 : PreRelease()
// 함수 설명 : 모든 컨트롤 릴리즈.(버튼은 자동 삭제)
//*****************************************************************************
void COneTimePassword::PreRelease()
{
	m_sprInputBox.Release();
}

//*****************************************************************************
// 함수 이름 : UpdateWhileActive()
// 함수 설명 : 액티브일 때의 업데이트.
// 매개 변수 : dDeltaTick	: 이전 Update()호출 후부터 지금 Update()까지 시간.
//*****************************************************************************
void COneTimePassword::UpdateWhileActive(double dDeltaTick)
{
	CInput& rInput = CInput::Instance();

	if (m_aBtn[LIW_OK].IsClick())			// OK 버튼을 클릭했는가?
	{
		// match user's input with one's otp in server 
		SendingOTP();
	}
	else if (m_aBtn[LIW_CANCEL].IsClick())	// Cancel 버튼을 클릭했는가?
	{
		// return to the login screen
		CancelInputOTP();
	}
	else if (CInput::Instance().IsKeyDown(VK_RETURN))// 엔터키를 눌렀는가?
	{
		::PlayBuffer(SOUND_CLICK01);	// 클릭 사운드.
		SendingOTP();
	}
	else if (CInput::Instance().IsKeyDown(VK_ESCAPE))// esc키를 눌렀는가?
	{
		::PlayBuffer(SOUND_CLICK01);	// 클릭 사운드.
		CancelInputOTP();
		CUIMng::Instance().SetSysMenuWinShow(false);
	}
}

//*****************************************************************************
// 함수 이름 : UpdateWhileShow()
// 함수 설명 : 보일때 업데이트.
// 매개 변수 : dDeltaTick	: 이전 Update()호출 후부터 지금 Update()까지 시간.
//*****************************************************************************
void COneTimePassword::UpdateWhileShow(double dDeltaTick)
{
	// OTP 입력 거부
	m_pOTPInputBox->GiveFocus();
	m_pOTPInputBox->DoAction();
}

//*****************************************************************************
// 함수 이름 : RenderControls()
// 함수 설명 : 각종 컨트롤 렌더.
//*****************************************************************************
void COneTimePassword::RenderControls()
{
	CWin::RenderButtons();

	m_sprInputBox.Render();

// 텍스트 입력 부분.
	m_pOTPInputBox->Render();
	
	// '계정', '암호' 텍스트 부분.
	// 텍스트 색상 지정.
	g_pRenderText->SetFont(g_hFixFont);
	g_pRenderText->SetBgColor(0);
	g_pRenderText->SetTextColor(CLRDW_WHITE);
	g_pRenderText->RenderText(int((CWin::GetXPos()+75) / g_fScreenRate_x),
		int((CWin::GetYPos() + 97)/ g_fScreenRate_y), GlobalText[3110]);
}


//*****************************************************************************
// 함수 이름 : SendingOTP()
// 함수 설명 : 서버에 OneTime Password 전송.
//*****************************************************************************
void COneTimePassword::SendingOTP()
{
	if (CurrentProtocolState == REQUEST_JOIN_SERVER)
		return;

	CUIMng::Instance().HideWin(this);	// 로그인 창을 닫음.

	char	szID[MAX_ID_SIZE+1];
	char	szPW[MAX_ID_SIZE+1];
	char	szOTP[MAX_ONETIME_PASSWORD_SIZE+1];
	memset( szID, 0, sizeof(char) * MAX_ID_SIZE+1 );
	memset( szPW, 0, sizeof(char) * MAX_ID_SIZE+1 );
	memset( szOTP, 0, sizeof(char) * MAX_ONETIME_PASSWORD_SIZE+1 );
	
	m_pOTPInputBox->GetText(szOTP, MAX_ONETIME_PASSWORD_SIZE+1);
	
	if (unicode::_strlen(szOTP) <= 0)	// OneTime Password 입력을 하지 않고 OK 클릭.
	{
		CUIMng::Instance().PopUpMsgWin(MESSAGE_OTP_NOT_WRITTEN);
	}
	else	// 계정을 로그인 한 상태에서 OTP 입력완료.
	{
		if (CurrentProtocolState == REQUEST_LOG_IN)
		{
#ifdef CONSOLE_DEBUG
			g_ConsoleDebug->Write( MCD_NORMAL, "Sending OTP to Server: (ID)%s", szID);
#endif //CONSOLE_DEBUG
			// 서버에 OneTime Password 전송.
			g_ErrorReport.Write("> Sending OTP \"%s\"\r\n", szID);
			// SendRequestOTP() 안에서 LogIn = 1을 해줌.
			CUIMng::Instance().m_LoginWin.GetIDInputBox()->GetText(szID, MAX_ID_SIZE+1);
			CUIMng::Instance().m_LoginWin.GetPassInputBox()->GetText(szPW, MAX_ID_SIZE+1);
	
			SendRequestOTP(szID, szPW, szOTP, 0x01);
		}
	}
}


//*****************************************************************************
// 함수 이름 :	CancelInputOTP()
// 함수 설명 :	OneTime Password 입력 취소.
//				OneTime Password 입력창을 닫고 
//				게임 서버 접속을 끊고 접속 서버에 다시 연결.
//*****************************************************************************
void COneTimePassword::CancelInputOTP()
{
	ConnectConnectionServer();
	// OneTime Password 입력창을 닫음.
	CUIMng::Instance().HideWin(this);

	ClearLoginInfoFromInputBox();
}

//*****************************************************************************
// 함수 이름 : ConnectConnectionServer()
// 함수 설명 : 접속 서버에 다시 연결.
//*****************************************************************************
void COneTimePassword::ConnectConnectionServer()
{
	SocketClient.Close();		// 게임 서버 접속을 끊음.

	LogIn = 0;
	CurrentProtocolState = REQUEST_JOIN_SERVER;
	// 접속 서버에 다시 연결.
    ::CreateSocket(szServerIpAddress, g_ServerPort);
}

//*****************************************************************************
// 함수 이름 : ClearLoginInfoFromInputBox()
// 함수 설명 : for better security, remove the password and OneTime Password from the inputbox.
//*****************************************************************************
void COneTimePassword::ClearLoginInfoFromInputBox()
{
		CUIMng::Instance().m_LoginWin.GetPassInputBox()->SetText(NULL);
		CUIMng::Instance().m_OneTimePasswordWin.GetOTPInputBox()->SetText(NULL);
}
#endif //LJH_ADD_ONETIME_PASSWORD	