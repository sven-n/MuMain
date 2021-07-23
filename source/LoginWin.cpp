//*****************************************************************************
// File: LoginWin.cpp
//
// Desc: implementation of the CLoginWin class.
//
// producer: Ahn Sang-Kyu
//*****************************************************************************

#include "stdafx.h"
#include "LoginWin.h"
#include "Input.h"
#include "UIMng.h"

// 텍스트 랜더를 위한 #include. ㅜㅜ
#include "ZzzBMD.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzInterface.h"

#include "UIControls.h"
#include "ZzzScene.h"

#include "wsclientinline.h"
#include "DSPlaySound.h"
#ifdef CONSOLE_DEBUG
#include "./Utilities/Log/muConsoleDebug.h"
#endif // CONSOLE_DEBUG

#ifdef LDK_ADD_GLOBAL_PORTAL_SYSTEM
	#include "GlobalPortalSystem.h"
#endif //LDK_ADD_GLOBAL_PORTAL_SYSTEM

#ifdef LEM_ADD_GAMECHU
	#include "GCCertification.h"
#endif // LEM_ADD_GAMECHU

#include "Local.h"

#ifdef KJH_ADD_SERVER_LIST_SYSTEM
#include "ServerListManager.h"
#endif // KJH_ADD_SERVER_LIST_SYSTEM

#define	LIW_ACCOUNT		0	// 계정 입력 박스 스프라이트.
#define	LIW_PASSWORD	1	// 암호 입력 박스 스프라이트.

#define LIW_OK			0	// OK 버튼.
#define LIW_CANCEL		1	// Cancel 버튼.

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

CLoginWin::CLoginWin()
{
#ifdef LDS_FIX_MEMORYLEAK_0910_LOGINWIN
	m_pIDInputBox = NULL;
	m_pPassInputBox = NULL;
#endif // LDS_FIX_MEMORYLEAK_0910_LOGINWIN
}

CLoginWin::~CLoginWin()
{
	SAFE_DELETE(m_pIDInputBox);
	SAFE_DELETE(m_pPassInputBox);
}

//*****************************************************************************
// 함수 이름 : Create()
// 함수 설명 : 로그인 창 생성.
//*****************************************************************************
void CLoginWin::Create()
{
// 기본 창 생성.
	CWin::Create(329, 245, BITMAP_LOG_IN+7);

// 텍스트 입력 박스 스프라이트.
	m_asprInputBox[LIW_ACCOUNT].Create(156, 23, BITMAP_LOG_IN+8);
	m_asprInputBox[LIW_PASSWORD].Create(156, 23, BITMAP_LOG_IN+8);

//	버튼 생성.
	for (int i = 0; i < 2; ++i)
	{
		m_aBtn[i].Create(54, 30, BITMAP_BUTTON + i, 3, 2, 1);
		CWin::RegisterButton(&m_aBtn[i]);
	}

#ifdef LDS_FIX_MEMORYLEAK_0910_LOGINWIN
	SAFE_DELETE( m_pIDInputBox );
#endif // LDS_FIX_MEMORYLEAK_0910_LOGINWIN

// 텍스트 입력 초기화.
	m_pIDInputBox = new CUITextInputBox;
#ifdef LDK_MOD_GLOBAL_PORTAL_ID_HIDE
	m_pIDInputBox->Init(g_hWnd, 140, 14, MAX_ID_SIZE, TRUE);
#else //LDK_MOD_GLOBAL_PORTAL_ID_HIDE
	m_pIDInputBox->Init(g_hWnd, 140, 14, MAX_ID_SIZE);
#endif //LDK_MOD_GLOBAL_PORTAL_ID_HIDE

#ifdef KJW_FIX_LOGIN_ID_BLACK_SPACE
	m_pIDInputBox->SetBackColor(0, 0, 0, 255);
#else // KJW_FIX_LOGIN_ID_BLACK_SPACE
	m_pIDInputBox->SetBackColor(255, 0, 0, 0);
#endif // KJW_FIX_LOGIN_ID_BLACK_SPACE
	m_pIDInputBox->SetTextColor(255, 255, 230, 210);
	m_pIDInputBox->SetFont(g_hFixFont);
	m_pIDInputBox->SetState(UISTATE_NORMAL);

#ifdef LDK_ADD_GLOBAL_PORTAL_SYSTEM
	//test
 	//strcat(m_ID, "1234512345");

	// 글로벌 포털 웹 인증 id 적용
	GlobalPortalSystem& portalSys = GlobalPortalSystem::Instance();
	
	if( portalSys.IsAuthSet() )
	{
		strcpy(m_ID, portalSys.GetAuthID());
	}
#endif //LDK_ADD_GLOBAL_PORTAL_SYSTEM
	//test
 	//strcat(m_ID, "webzendm15");
	m_pIDInputBox->SetText(m_ID);

#ifdef LDS_FIX_MEMORYLEAK_0910_LOGINWIN
	SAFE_DELETE( m_pPassInputBox );
#endif // LDS_FIX_MEMORYLEAK_0910_LOGINWIN
	
	m_pPassInputBox = new CUITextInputBox;

#ifdef LDS_MODIFY_CHAR_LENGTH_USERPASSWORD		// 비밀번호 자릿수 10->12로 변경 사항
	m_pPassInputBox->Init(g_hWnd, 140, 14, MAX_PASSWORD_SIZE, TRUE);
#else // LDS_MODIFY_CHAR_LENGTH_USERPASSWORD

#ifdef LDK_MOD_PASSWORD_LENGTH_20 // 글로벌 포털 비밀번호 20 자리로 변경
	m_pPassInputBox->Init(g_hWnd, 140, 14, MAX_PASSWORD_SIZE, TRUE);
#else //LDK_MOD_PASSWORD_LENGTH_20
	m_pPassInputBox->Init(g_hWnd, 140, 14, MAX_ID_SIZE, TRUE);
#endif //LDK_MOD_PASSWORD_LENGTH_20

#endif // LDS_MODIFY_CHAR_LENGTH_USERPASSWORD

	m_pPassInputBox->SetBackColor(0, 0, 0, 25);
	m_pPassInputBox->SetTextColor(255, 255, 230, 210);
	m_pPassInputBox->SetFont(g_hFixFont);
	m_pPassInputBox->SetState(UISTATE_NORMAL);


#ifdef LDK_MOD_GLOBAL_PORTAL_LOGIN_ID_INPUT_DENY
	//id 변경 불가
	m_pPassInputBox->GiveFocus();
#else //LDK_MOD_GLOBAL_PORTAL_LOGIN_ID_INPUT_DENY
	if (::strlen(m_ID) > 0)
		m_pPassInputBox->GiveFocus();
	else
		m_pIDInputBox->GiveFocus();

	m_pIDInputBox->SetTabTarget(m_pPassInputBox);
	m_pPassInputBox->SetTabTarget(m_pIDInputBox);
#endif //LDK_MOD_GLOBAL_PORTAL_LOGIN_ID_INPUT_DENY
}

//*****************************************************************************
// 함수 이름 : PreRelease()
// 함수 설명 : 모든 컨트롤 릴리즈.(버튼은 자동 삭제)
//*****************************************************************************
void CLoginWin::PreRelease()
{
	for (int i = 0; i < 2; ++i)
		m_asprInputBox[i].Release();
}

//*****************************************************************************
// 함수 이름 : SetPosition()
// 함수 설명 : 창 위치 지정.
// 매개 변수 : nXCoord	: 스크린 X좌표.
//			   nYCoord	: 스크린 Y좌표.
//*****************************************************************************
void CLoginWin::SetPosition(int nXCoord, int nYCoord)
{
	CWin::SetPosition(nXCoord, nYCoord);

	m_asprInputBox[LIW_ACCOUNT].SetPosition(nXCoord + 109, nYCoord + 106);
	m_asprInputBox[LIW_PASSWORD].SetPosition(nXCoord + 109, nYCoord + 131);

	if (g_iChatInputType == 1)
	{
		// 계정 입력 텍스트.
		m_pIDInputBox->SetPosition(int((m_asprInputBox[LIW_ACCOUNT].GetXPos() + 6) / g_fScreenRate_x),
			int((m_asprInputBox[LIW_ACCOUNT].GetYPos() + 6) / g_fScreenRate_y));
		// 암호 입력 텍스트.
		m_pPassInputBox->SetPosition(int((m_asprInputBox[LIW_PASSWORD].GetXPos() + 6) / g_fScreenRate_x),
			int((m_asprInputBox[LIW_PASSWORD].GetYPos() + 6) / g_fScreenRate_y));
	}

	m_aBtn[LIW_OK].SetPosition(nXCoord + 150, nYCoord + 178);
	m_aBtn[LIW_CANCEL].SetPosition(nXCoord + 211, nYCoord + 178);
}

//*****************************************************************************
// 함수 이름 : Show()
// 함수 설명 : 창을 보여 주거나 안보이게함.
// 매개 변수 : bShow	: true이면 보여줌.
//*****************************************************************************
void CLoginWin::Show(bool bShow)
{
	CWin::Show(bShow);

	for (int i = 0; i < 2; ++i)
	{
		m_asprInputBox[i].Show(bShow);
		m_aBtn[i].Show(bShow);
	}
}

//*****************************************************************************
// 함수 이름 : CursorInWin()
// 함수 설명 : 윈도우 영역 안에 마우스 커서가 위치하는가?
// 매개 변수 : eArea	: 검사할 영역.(win.h의 #define 참조)
//*****************************************************************************
bool CLoginWin::CursorInWin(int nArea)
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
// 함수 이름 : UpdateWhileActive()
// 함수 설명 : 액티브일 때의 업데이트.
// 매개 변수 : dDeltaTick	: 이전 Update()호출 후부터 지금 Update()까지 시간.
//*****************************************************************************
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
void CLoginWin::UpdateWhileActive()
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
void CLoginWin::UpdateWhileActive(double dDeltaTick)
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
{
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
	CInput& rInput = CInput::Instance();
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING

#ifdef LEM_ADD_GAMECHU	// 게임츄 자동 로그인
	if( GAMECHU->Get_GameChuLogin() )	RequestLogin();
#endif // LEM_ADD_GAMECH
	if (m_aBtn[LIW_OK].IsClick())			// OK 버튼을 클릭했는가?
		RequestLogin();
	else if (m_aBtn[LIW_CANCEL].IsClick())	// Cancel 버튼을 클릭했는가?
		CancelLogin();
	else if (CInput::Instance().IsKeyDown(VK_RETURN))// 엔터키를 눌렀는가?
	{
		::PlayBuffer(SOUND_CLICK01);	// 클릭 사운드.
		RequestLogin();
	}
	else if (CInput::Instance().IsKeyDown(VK_ESCAPE))// esc키를 눌렀는가?
	{
		::PlayBuffer(SOUND_CLICK01);	// 클릭 사운드.
		CancelLogin();
		CUIMng::Instance().SetSysMenuWinShow(false);
	}
}

//*****************************************************************************
// 함수 이름 : UpdateWhileShow()
// 함수 설명 : 보일때 업데이트.
// 매개 변수 : dDeltaTick	: 이전 Update()호출 후부터 지금 Update()까지 시간.
//*****************************************************************************
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
void CLoginWin::UpdateWhileShow()
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
void CLoginWin::UpdateWhileShow(double dDeltaTick)
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
{
#ifdef LDK_MOD_GLOBAL_PORTAL_LOGIN_ID_INPUT_DENY
	// id 입력 거부
	m_pPassInputBox->GiveFocus();
	m_pPassInputBox->DoAction();
#else //LDK_MOD_GLOBAL_PORTAL_LOGIN_ID_INPUT_DENY
	m_pIDInputBox->DoAction();
	m_pPassInputBox->DoAction();
#endif //LDK_MOD_GLOBAL_PORTAL_LOGIN_ID_INPUT_DENY
}

//*****************************************************************************
// 함수 이름 : RenderControls()
// 함수 설명 : 각종 컨트롤 렌더.
//*****************************************************************************
void CLoginWin::RenderControls()
{
	CWin::RenderButtons();

	for (int i = 0; i < 2; ++i)
		m_asprInputBox[i].Render();

// 텍스트 입력 부분.
	m_pIDInputBox->Render();
	m_pPassInputBox->Render();

#ifdef LEM_ADD_GAMECHU	// 임시 로그인 텍스트
	char	szTemp[50] = { 0, };
	sprintf( szTemp, "GAMECHU : %d\0", GAMECHU->Get_GameChuLogin() );
	g_pRenderText->SetFont(g_hFixFont);
	g_pRenderText->SetBgColor(0);
	g_pRenderText->SetTextColor(CLRDW_WHITE);
	g_pRenderText->RenderText(int((CWin::GetXPos() + 30) / g_fScreenRate_x) + 200,
		int((CWin::GetYPos() + 113) / g_fScreenRate_y), szTemp);

	sprintf( szTemp, "GAMECHU : %s\0", GAMECHU->Get_UserData().szUserSerialNumber );
	g_pRenderText->RenderText(int((CWin::GetXPos() + 30) / g_fScreenRate_x) + 200,
		int((CWin::GetYPos() + 113) / g_fScreenRate_y) + 20, szTemp);
#endif // LEM_ADD_GAMECHU

	// '계정', '암호' 텍스트 부분.
	// 텍스트 색상 지정.
	g_pRenderText->SetFont(g_hFixFont);
	g_pRenderText->SetBgColor(0);
	g_pRenderText->SetTextColor(CLRDW_WHITE);
	g_pRenderText->RenderText(int((CWin::GetXPos() + 30) / g_fScreenRate_x),
		int((CWin::GetYPos() + 113) / g_fScreenRate_y), GlobalText[450]);
	g_pRenderText->RenderText(int((CWin::GetXPos() + 30) / g_fScreenRate_x),
		int((CWin::GetYPos() + 139) / g_fScreenRate_y), GlobalText[451]);	

// 서버 이름.
#ifdef KJH_ADD_SERVER_LIST_SYSTEM
	unicode::t_char szServerName[MAX_TEXT_LENGTH];
#ifdef ASG_MOD_SERVER_LIST_ADD_CHARGED_CHANNEL
	const char* apszGlobalText[4]
		= { GlobalText[461], GlobalText[460], GlobalText[3130], GlobalText[3131] };
	sprintf(szServerName, apszGlobalText[g_ServerListManager->GetNonPVPInfo()],
		g_ServerListManager->GetSelectServerName(), g_ServerListManager->GetSelectServerIndex());
#else	// ASG_MOD_SERVER_LIST_ADD_CHARGED_CHANNEL
	if(g_ServerListManager->IsNonPvP() == true)		// NonPvP인가?
	{
		sprintf(szServerName, GlobalText[460], 
					g_ServerListManager->GetSelectServerName(), 
					g_ServerListManager->GetSelectServerIndex());
	}
	else
	{
		sprintf(szServerName, GlobalText[461], 
			g_ServerListManager->GetSelectServerName(), 
					g_ServerListManager->GetSelectServerIndex());
	}
#endif	// ASG_MOD_SERVER_LIST_ADD_CHARGED_CHANNEL
#else // KJH_ADD_SERVER_LIST_SYSTEM
	char szServerName[64];
#if defined	ADD_EXPERIENCE_SERVER
	// 테스트 서버이고, 서버 번호가 10 이상 이면.
	if (0 == ::strcmp(ServerList[ServerSelectHi].Name, GlobalText[559])
		&& ServerLocalSelect >= 10)
		::sprintf(szServerName, GlobalText[461], GlobalText[1699],
			ServerLocalSelect - 9);
	else
#elif defined ADD_EVENT_SERVER_NAME
	// 테스트 서버이고, 서버번호가 6이상이면.
	if (0 == ::strcmp(ServerList[ServerSelectHi].Name, GlobalText[559])
		&& ServerLocalSelect >= 6)
		::sprintf(szServerName, GlobalText[461], GlobalText[19],
			ServerLocalSelect - 5);
	else
#endif	// ADD_EXPERIENCE_SERVER	ADD_EVENT_SERVER_NAME
	{
		if (::IsNonPvpServer(ServerSelectHi, ServerLocalSelect))
			::sprintf(szServerName, GlobalText[460], ServerList[ServerSelectHi].Name, ServerLocalSelect);
		else
			::sprintf(szServerName, GlobalText[461], ServerList[ServerSelectHi].Name, ServerLocalSelect);
	}
#endif // KJH_ADD_SERVER_LIST_SYSTEM

	g_pRenderText->RenderText(int((CWin::GetXPos() + 111) / g_fScreenRate_x),
		int((CWin::GetYPos() + 80) / g_fScreenRate_y), szServerName);
}

//*****************************************************************************
// 함수 이름 : RequestLogin()
// 함수 설명 : 서버에 로그인 요구.
//*****************************************************************************
void CLoginWin::RequestLogin()
{
	if (CurrentProtocolState == REQUEST_JOIN_SERVER)
		return;

	CUIMng::Instance().HideWin(this);	// 로그인 창을 닫음.

#ifdef LEM_ADD_GAMECHU	// 게임츄 로그인 (2010.10.28)
	if( GAMECHU->Get_GameChuLogin() )
	{
		char*	szAuth	= GAMECHU->Get_AuthInfo_ToChar();
		char*	szStat	= GAMECHU->Get_StatInfo_ToChar();
		int		iAuthLen= strlen(szAuth);
		int		iResult = 1000;

#ifdef FOR_WORK
		g_ErrorReport.Write("[gamechu] = WebLogin, AuthInfo: %s, StatInfo: %s\n", szAuth, szStat );
#endif // FOR_WORK
		SendRequestLogIn_GameChu( szAuth, iAuthLen, szStat );
		return;
	}
#endif // LEM_ADD_GAMECHU

#ifdef LDS_MODIFY_CHAR_LENGTH_USERPASSWORD	// 비밀번호 자릿수 10->12로 변경 사항
	char	szID[MAX_ID_SIZE+1], szPass[MAX_PASSWORD_SIZE+1];
	memset( szID, 0, sizeof(char) * MAX_ID_SIZE+1 );
	memset( szPass, 0, sizeof(char) * MAX_PASSWORD_SIZE+1 );
	m_pIDInputBox->GetText(szID, MAX_ID_SIZE+1);
	m_pPassInputBox->GetText(szPass, MAX_PASSWORD_SIZE+1);
	
	if (unicode::_strlen(szID) <= 0)		// 계정 입력이 없다면.
	{
		CUIMng::Instance().PopUpMsgWin(MESSAGE_INPUT_ID);
	}
	else if (unicode::_strlen(szPass) <= 0)	// 암호 입력이 없다면.
	{
		CUIMng::Instance().PopUpMsgWin(MESSAGE_INPUT_PASSWORD);
	}
#else // LDS_MODIFY_CHAR_LENGTH_USERPASSWORD	


#ifdef LDK_MOD_PASSWORD_LENGTH_20
//글로벌 포털 
	char szID[MAX_ID_SIZE+1] = { 0, };
	char szPass[MAX_PASSWORD_SIZE+1] = {0, };
	m_pIDInputBox->GetText(szID, MAX_ID_SIZE+1);
	m_pPassInputBox->GetText(szPass, MAX_PASSWORD_SIZE+1);
#else //LDK_MOD_PASSWORD_LENGTH_20
	char szID[MAX_ID_SIZE] = { 0, };
	char szPass[MAX_ID_SIZE] = {0, };
	m_pIDInputBox->GetText(szID, MAX_ID_SIZE + 1);
	m_pPassInputBox->GetText(szPass, MAX_ID_SIZE + 1);
#endif //LDK_MOD_PASSWORD_LENGTH_20
	
	if (unicode::_strlen(szID) <= 0)		// 계정 입력이 없다면.
		CUIMng::Instance().PopUpMsgWin(MESSAGE_INPUT_ID);
	else if (unicode::_strlen(szPass) <= 0)	// 암호 입력이 없다면.
		CUIMng::Instance().PopUpMsgWin(MESSAGE_INPUT_PASSWORD);
#endif // LDS_MODIFY_CHAR_LENGTH_USERPASSWORD
	else	// 계정, 암호 모두 입력이 있다면.
	{
		if (CurrentProtocolState == RECEIVE_JOIN_SERVER_SUCCESS)
		{
#ifdef CONSOLE_DEBUG
			g_ConsoleDebug->Write( MCD_NORMAL, "Login with the following account: %s", szID);
#endif //CONSOLE_DEBUG
			// 서버에 로그인 요구.
			g_ErrorReport.Write("> Login Request.\r\n");
			g_ErrorReport.Write("> Try to Login \"%s\"\r\n", szID);
			// SendRequestLogIn() 안에서 LogIn = 1을 해줌.

#ifdef LEM_ADD_GAMECHU
			char*	szAuth	= GAMECHU->Get_AuthInfo_ToChar();
			char*	szStat	= GAMECHU->Get_StatInfo_ToChar();
			int		iAuthLen= strlen(szAuth);
			int		iResult = 1000;

		#ifdef FOR_WORK
			g_ErrorReport.Write("[gamechu] = NoDataLogin, AuthInfo: %s, StatInfo: %s\n", szAuth, szStat );					
		#endif // FOR_WORK
	
#endif // LEM_ADD_GAMECHU

 			SendRequestLogIn(szID, szPass);
		}
	}
}

//*****************************************************************************
// 함수 이름 : CancelLogin()
// 함수 설명 : 로그인 취소.
//*****************************************************************************
void CLoginWin::CancelLogin()
{
	ConnectConnectionServer();
	// 로그인 창을 닫음.
	CUIMng::Instance().HideWin(this);
}

//*****************************************************************************
// 함수 이름 : ConnectConnectionServer()
// 함수 설명 : 접속 서버에 다시 연결.
//*****************************************************************************
void CLoginWin::ConnectConnectionServer()
{
	SocketClient.Close();		// 게임 서버 접속을 끊음.

	LogIn = 0;
	CurrentProtocolState = REQUEST_JOIN_SERVER;
	// 접속 서버에 다시 연결.
    ::CreateSocket(szServerIpAddress, g_ServerPort);
}