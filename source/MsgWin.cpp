//*****************************************************************************
// File: MsgWin.cpp
//
// Desc: implementation of the CMsgWin class.
//
// producer: Ahn Sang-Kyu
//*****************************************************************************

#include "stdafx.h"
#include "MsgWin.h"

#include "Input.h"
#include "UIMng.h"
#include <crtdbg.h>		// _ASSERT() 사용.

// 텍스트 랜더를 위한 #include. ㅜㅜ
#include "ZzzBMD.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzInterface.h"

#include "GOBoid.h"
#include "ZzzScene.h"
#include "DSPlaySound.h"
#include "wsclientinline.h"
#include "UIControls.h"
#include "ZzzOpenglUtil.h"
#ifdef LJH_ADD_ONETIME_PASSWORD
// web browser를 켜기 위해 include
	#include "iexplorer.h"
#endif //LJH_ADD_ONETIME_PASSWORD

#define	MW_OK		0	// OK 버튼, (Localized)예,확인 버튼
#define	MW_CANCEL	1	// Cancel 버튼, (Localized)아니오,취소 버튼

extern float g_fScreenRate_x;
extern float g_fScreenRate_y;
extern int g_iChatInputType;
extern CUITextInputBox* g_pSinglePasswdInputBox;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMsgWin::CMsgWin()
{

}

CMsgWin::~CMsgWin()
{

}

//*****************************************************************************
// 함수 이름 : Create()
// 함수 설명 : 메시지 창 생성.
//			   (메시지가 여러개라도 단 하나만 생성) 
// 변경 된 내용: YES/NO 선택 가능한 기능 추가
//				  -> p_iBtnType 값이 없거나 0(default):					OK/CANCEL 버튼을 포함 기본 버튼
//				  -> p_iBtnType 값이 MWT_BTN_LOCALIZED_YESNO:			(localized)예/아니오버튼 (10.01.22)
//				  -> p_iBtnType 값이 MWT_BTN_LOCALIZED_CONFIRM_CANCEL:	(localized)확인/취소 버튼 (10.01.27)
//*****************************************************************************
#ifdef LJH_ADD_LOCALIZED_BTNS
void CMsgWin::Create(int p_iBtnType)
#else  //LJH_ADD_LOCALIZED_BTNS
void CMsgWin::Create()
#endif //LJH_ADD_LOCALIZED_BTNS
{
#ifdef LJH_ADD_LOCALIZED_BTNS
	// YES/NO 버튼 글자 색
	DWORD adwBtnColor[3] = { CLRDW_BR_GRAY, CLRDW_BR_GRAY, CLRDW_WHITE};
#endif //LJH_ADD_LOCALIZED_BTNS

	CInput rInput = CInput::Instance();
	// 기본 창 생성.(화면 전체 크기. 검은색 반투명임.)
	CWin::Create(rInput.GetScreenWidth(), rInput.GetScreenHeight());

	// 창 모양의 스프라이트.
	m_sprBack.Create(352, 113, BITMAP_MESSAGE_WIN);
	// 텍스트 입력 박스 스프라이트.
	m_sprInput.Create(171, 23, BITMAP_MSG_WIN_INPUT);

	// 확인, 취소 버튼.
	// 또는 예, 아니오 버튼.
	for (int i = 0; i < 2; ++i)
	{
#ifdef LJH_ADD_LOCALIZED_BTNS
		if (p_iBtnType == 0)
			m_aBtn[i].Create(54, 30, BITMAP_BUTTON + i, 3, 2, 1);
		else if (p_iBtnType == MWT_BTN_LOCALIZED_YESNO)
		{
			m_aBtn[i].Create(64, 29, SEASON3B::CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL, 3, 2, 1);
			m_aBtn[i].SetText(GlobalText[1037+i], adwBtnColor);
		}
		else if (p_iBtnType == MWT_BTN_LOCALIZED_CONFIRM_CANCEL || p_iBtnType == MWT_BTN_LOCALIZED_CONFIRM_ONLY)
		{
			m_aBtn[i].Create(64, 29, SEASON3B::CNewUIMessageBoxMng::IMAGE_MSGBOX_BTN_EMPTY_SMALL, 3, 2, 1);
			m_aBtn[i].SetText(GlobalText[228+i*2886], adwBtnColor);
		}
#else  //LJH_ADD_LOCALIZED_BTNS
		m_aBtn[i].Create(54, 30, BITMAP_BUTTON + i, 3, 2, 1);
#endif //LJH_ADD_LOCALIZED_BTNS
		CWin::RegisterButton(&m_aBtn[i]);
	}

	::memset(m_aszMsg[0], 0 ,sizeof(char) * MW_MSG_LINE_MAX * MW_MSG_ROW_MAX);

	m_eType = MWT_NON;
	m_nMsgLine = 0;
	m_nMsgCode = -1;
	m_nGameExit = -1;
	m_dDeltaTickSum = 0.0;
}

//*****************************************************************************
// 함수 이름 : PreRelease()
// 함수 설명 : 모든 컨트롤 릴리즈.(버튼은 자동 삭제)
//*****************************************************************************
void CMsgWin::PreRelease()
{
	m_sprInput.Release();
	m_sprBack.Release();
}

//*****************************************************************************
// 함수 이름 : SetPosition()
// 함수 설명 : 창 위치 지정.
//			   (화면 상에는 창이 이동하는것처럼 보이지만 기본 창은 이동이 없음)
// 매개 변수 : nXCoord	: 스크린 X좌표.
//			   nYCoord	: 스크린 Y좌표.
//*****************************************************************************
void CMsgWin::SetPosition(int nXCoord, int nYCoord)
{
	m_sprBack.SetPosition(nXCoord, nYCoord);
	SetCtrlPosition();
}

//*****************************************************************************
// 함수 이름 : SetCtrlPosition()
// 함수 설명 : 컨트롤 위치 지정.
//*****************************************************************************
void CMsgWin::SetCtrlPosition()
{
	int nBaseXPos = m_sprBack.GetXPos();
	int nBtnYPos = m_sprBack.GetYPos() + 72;
// 메시지 박스 타잎에 따라 컨트롤 위치가 변함.
	switch (m_eType)
	{
	case MWT_BTN_CANCEL:
		m_aBtn[MW_CANCEL].SetPosition(nBaseXPos + 149, nBtnYPos);
		break;
	case MWT_BTN_OK:
#ifdef LJH_ADD_LOCALIZED_BTNS
	case MWT_BTN_LOCALIZED_CONFIRM_ONLY:
#endif //LJH_ADD_LOCALIZED_BTNS
		m_aBtn[MW_OK].SetPosition(nBaseXPos + 149, nBtnYPos);
		break;
	case MWT_BTN_BOTH:
#ifdef LJH_ADD_LOCALIZED_BTNS
	case MWT_BTN_LOCALIZED_YESNO:
	case MWT_BTN_LOCALIZED_CONFIRM_CANCEL:
#endif //LJH_ADD_LOCALIZED_BTNS
		m_aBtn[MW_OK].SetPosition(nBaseXPos + 98, nBtnYPos);
		m_aBtn[MW_CANCEL].SetPosition(nBaseXPos + 200, nBtnYPos);
		break;
	case MWT_STR_INPUT:
		m_sprInput.SetPosition(nBaseXPos + 32, nBtnYPos + 4);
		m_aBtn[MW_OK].SetPosition(nBaseXPos + 209, nBtnYPos);
		m_aBtn[MW_CANCEL].SetPosition(nBaseXPos + 264, nBtnYPos);
		// 입력 텍스트 위치 지정.
		if (m_nMsgCode == MESSAGE_DELETE_CHARACTER_RESIDENT)
			if (g_iChatInputType == 1)
				g_pSinglePasswdInputBox->SetPosition(
					int((m_sprInput.GetXPos() + 10) / g_fScreenRate_x),
					int((m_sprInput.GetYPos() + 8) / g_fScreenRate_y));
		break;
	}
}

//*****************************************************************************
// 함수 이름 : Show()
// 함수 설명 : 창을 보여 주거나 안보이게함.
// 매개 변수 : bShow	: true이면 보여줌.
//*****************************************************************************
void CMsgWin::Show(bool bShow)
{
	CWin::Show(bShow);

	m_sprBack.Show(bShow);

	switch (m_eType)
	{
	case MWT_BTN_CANCEL:
		m_aBtn[MW_OK].Show(false);
		m_aBtn[MW_CANCEL].Show(bShow);
		m_sprInput.Show(false);
		break;
	case MWT_BTN_OK:
#ifdef LJH_ADD_LOCALIZED_BTNS
	case MWT_BTN_LOCALIZED_CONFIRM_ONLY:
#endif //LJH_ADD_LOCALIZED_BTNS
		m_aBtn[MW_OK].Show(bShow);
		m_aBtn[MW_CANCEL].Show(false);
		m_sprInput.Show(false);
		break;
	case MWT_BTN_BOTH:
#ifdef LJH_ADD_LOCALIZED_BTNS
	case MWT_BTN_LOCALIZED_YESNO:
	case MWT_BTN_LOCALIZED_CONFIRM_CANCEL:
#endif //LJH_ADD_LOCALIZED_BTNS
		m_aBtn[MW_OK].Show(bShow);
		m_aBtn[MW_CANCEL].Show(bShow);
		m_sprInput.Show(false);
		break;
	case MWT_STR_INPUT:
		m_aBtn[MW_OK].Show(bShow);
		m_aBtn[MW_CANCEL].Show(bShow);
		m_sprInput.Show(bShow);
		break;
	default:
		m_aBtn[MW_OK].Show(false);
		m_aBtn[MW_CANCEL].Show(false);
		m_sprInput.Show(false);
	}
		
}

//*****************************************************************************
// 함수 이름 : CursorInWin()
// 함수 설명 : 윈도우 영역 안에 마우스 커서가 위치하는가?
// 매개 변수 : eArea	: 검사할 영역.(win.h의 #define 참조)
//*****************************************************************************
bool CMsgWin::CursorInWin(int nArea)
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
void CMsgWin::UpdateWhileActive(double dDeltaTick)
{
	CInput& rInput = CInput::Instance();

	if (rInput.IsKeyDown(VK_RETURN))	// 엔터키를 눌렀는가?
	{
		if (m_eType > MWT_BTN_CANCEL)	// OK 버튼을 사용하고 있다면.
		{
			::PlayBuffer(SOUND_CLICK01);// 클릭 사운드.
			ManageOKClick();
		}
		else if (m_eType == MWT_BTN_CANCEL)	// Cancel 버튼만 있다면.
		{
			::PlayBuffer(SOUND_CLICK01);// 클릭 사운드.
			ManageCancelClick();
		}
	}
	else if (rInput.IsKeyDown(VK_ESCAPE))// esc키를 눌렀는가?
	{
#ifdef LJH_ADD_LOCALIZED_BTNS
		if (m_eType == MWT_BTN_OK  || m_eType == MWT_BTN_LOCALIZED_CONFIRM_ONLY) // OK 버튼, 혹은 확인버튼만 있다면.
#else  //LJH_ADD_LOCALIZED_BTNS
		if (m_eType == MWT_BTN_OK)		// OK 버튼만 있다면.
#endif //LJH_ADD_LOCALIZED_BTNS
		{
			::PlayBuffer(SOUND_CLICK01);// 클릭 사운드.
			ManageOKClick();
		}
		else if (m_eType > MWT_NON)		// Cancel버튼을 사용하고 있다면.
		{
			::PlayBuffer(SOUND_CLICK01);// 클릭 사운드.
			ManageCancelClick();
		}
		CUIMng::Instance().SetSysMenuWinShow(false);
	}
	else if (m_aBtn[MW_OK].IsClick())		// OK 버튼을 클릭했는가?
		ManageOKClick();
	else if (m_aBtn[MW_CANCEL].IsClick())	// Cancel 버튼을 클릭했는가?
		ManageCancelClick();
	else if (m_nMsgCode == MESSAGE_GAME_END_COUNTDOWN)	// 게임종료 카운트.
	{
		if (m_nGameExit != -1) 
		{
			m_dDeltaTickSum += dDeltaTick;
			if (m_dDeltaTickSum > 1000.0)
			{
				m_dDeltaTickSum = 0.0;
				if (--m_nGameExit == 0)
				{
					g_ErrorReport.Write("> Menu - Exit game.");
					g_ErrorReport.WriteCurrentTime();
	//				SocketClient.Close();	// 어차피 WM_DESTROY에서 호출 됨.
					::PostMessage(g_hWnd, WM_CLOSE, 0, 0);
				}
				else
				{
					char szMsg[64];
					::sprintf(szMsg, GlobalText[380], m_nGameExit);
					SetMsg(m_eType, szMsg);
				}
			}
		}
	}
}

//*****************************************************************************
// 함수 이름 : RenderControls()
// 함수 설명 : 각종 컨트롤 렌더.
//*****************************************************************************
void CMsgWin::RenderControls()
{
	m_sprBack.Render();			// 창 모양의 스프라이트.

	int nTextPosX, nTextPosY;

	// 텍스트 색상 지정.
	g_pRenderText->SetFont(g_hFixFont);
	g_pRenderText->SetTextColor(CLRDW_WHITE);
	g_pRenderText->SetBgColor(0);

	if (1 == m_nMsgLine)		// 메시지가 1줄일때.
	{
		nTextPosX = int(m_sprBack.GetXPos() / g_fScreenRate_x);
		if (MWT_NON != m_eType)	// 버튼이 있다면.
			nTextPosY = int((m_sprBack.GetYPos() + 38) / g_fScreenRate_y);
		else
			nTextPosY = int((m_sprBack.GetYPos() + 54) / g_fScreenRate_y);
		g_pRenderText->RenderText(nTextPosX, nTextPosY, m_aszMsg[0],
			m_sprBack.GetWidth() / g_fScreenRate_x, 0, RT3_SORT_CENTER);
	}
	else if (2 == m_nMsgLine)	// 메시지가 2줄일때.
	{
		nTextPosX = int((m_sprBack.GetXPos() + 25) / g_fScreenRate_x);
		if (MWT_NON != m_eType)	// 버튼이 있다면.
			nTextPosY = int((m_sprBack.GetYPos() + 32) / g_fScreenRate_y);
		else
			nTextPosY = int((m_sprBack.GetYPos() + 44) / g_fScreenRate_y);
		g_pRenderText->RenderText(nTextPosX, nTextPosY, m_aszMsg[0]);

		if (MWT_NON != m_eType)	// 버튼이 있다면.
			nTextPosY = int((m_sprBack.GetYPos() + 51) / g_fScreenRate_y);
		else
			nTextPosY = int((m_sprBack.GetYPos() + 66) / g_fScreenRate_y);
		g_pRenderText->RenderText(nTextPosX, nTextPosY, m_aszMsg[1]);
	}

	m_sprInput.Render();		// 텍스트 입력 박스 스프라이트.
	// 텍스트 입력 처리.
	if (m_nMsgCode == MESSAGE_DELETE_CHARACTER_RESIDENT)
	{
		if (g_iChatInputType == 1)
			g_pSinglePasswdInputBox->Render();
		else if (g_iChatInputType == 0)
		{
			InputTextWidth = 100;
			::RenderInputText(
				int((m_sprInput.GetXPos() + 10) / g_fScreenRate_x),
				int((m_sprInput.GetYPos() + 8) / g_fScreenRate_y), 0, 0);
			InputTextWidth = 256;
		}
	}

	CWin::RenderButtons();
}

//*****************************************************************************
// 함수 이름 : SetMsg()
// 함수 설명 : 메시지와 버튼 셋팅.
// 매개 변수 : eType	: 메시지 윈도우 타잎.
//			   lpszMsg	: 메시지 문자열.
//			   lpszMsg2	: 메시지 문자열 2번째 라인.(기본값 NULL)
// lpszMsg만 있는 경우 자동으로 줄을 나누어 입력 되고, lpszMsg2도 있으면 첫째
//줄에 lpszMsg가 둘째 줄에는 lpszMsg2가 입력 된다.
//*****************************************************************************
void CMsgWin::SetMsg(MSG_WIN_TYPE eType, LPCTSTR lpszMsg, LPCTSTR lpszMsg2)
{
	_ASSERT(lpszMsg);

	m_eType = eType;

	SetCtrlPosition();	// m_eType에 따라 컨트롤 위치가 변하므로.

	if (NULL == lpszMsg2)
	{
		// 자동으로 줄을 나눔.
		m_nMsgLine = ::SeparateTextIntoLines(
			(char*)lpszMsg, m_aszMsg[0], MW_MSG_LINE_MAX, MW_MSG_ROW_MAX);
	}
	else
	{
		::strcpy(m_aszMsg[0], lpszMsg);
		::strcpy(m_aszMsg[1], lpszMsg2);

		m_nMsgLine = 2;
	}
}

//*****************************************************************************
// 함수 이름 : PopUp()
// 함수 설명 : 메시지 코드에 맞게 창을 띄움.
// 매개 변수 : nMsgCode	: 메시지 코드.
//*****************************************************************************
void CMsgWin::PopUp(int nMsgCode, char* pszMsg)
{
	CUIMng& rUIMng = CUIMng::Instance();
	LPCTSTR lpszMsg = NULL, lpszMsg2 = NULL;
	MSG_WIN_TYPE eType = MWT_BTN_OK;
	m_nMsgCode = nMsgCode;
	char szTempMsg[128];

	switch (m_nMsgCode)
	{
	case MESSAGE_FREE_MSG_NOT_BTN:			// 자유로운 메시지, 버튼 없음.(현재, 캐릭터 씬에서 서버 씬으로 카운트다운 시 사용)
		lpszMsg = pszMsg;
		eType = MWT_NON;
		break;
	case MESSAGE_GAME_END_COUNTDOWN:		// 게임 종료 카운트 다운.
		m_nGameExit = 5;
		::sprintf(szTempMsg, GlobalText[380], m_nGameExit);
		lpszMsg = szTempMsg;
		eType = MWT_NON;
		break;
	case MESSAGE_WAIT:						// 잠시만 기다려 주세요.
		lpszMsg = GlobalText[471];
		eType = MWT_NON;
		break;
	case MESSAGE_SERVER_BUSY:				// 게임 서버가 포화 상태.
	case RECEIVE_LOG_IN_FAIL_SERVER_BUSY:	// 게임 서버가 포화 상태.
		lpszMsg = GlobalText[416];
		break;
	case RECEIVE_JOIN_SERVER_WAITING:		// 인증서버가 바쁨.(현재 사용 안함.)
		rUIMng.ShowWin(&rUIMng.m_ServerSelWin);
		lpszMsg = GlobalText[416];
		break;
	case MESSAGE_SERVER_LOST:				// 서버 접속 실패.
		lpszMsg = GlobalText[402];
		break;
	case MESSAGE_VERSION:					// 버전이 틀림.
	case RECEIVE_LOG_IN_FAIL_VERSION:		// 버전이 틀림.
		lpszMsg = GlobalText[405];
		lpszMsg2 = GlobalText[406];
		break;
	case MESSAGE_INPUT_ID:					// 계정 입력 안함.
		lpszMsg = GlobalText[403];
		break;
	case MESSAGE_INPUT_PASSWORD:			// 암호 입력 안함.
		lpszMsg = GlobalText[404];
		break;
	case RECEIVE_LOG_IN_FAIL_ID:			// 계정 틀림.
		lpszMsg = GlobalText[414];
		break;
	case RECEIVE_LOG_IN_FAIL_PASSWORD:		// 암호 틀림.
		lpszMsg = GlobalText[407];
		break;
	case RECEIVE_LOG_IN_FAIL_ID_CONNECTED:	// 접속중인 계정임.
		lpszMsg = GlobalText[415];
		break;
	case RECEIVE_LOG_IN_FAIL_ID_BLOCK:		// 블럭된 계정임.
	case MESSAGE_DELETE_CHARACTER_ID_BLOCK:	// 블럭된 계정임.
		lpszMsg = GlobalText[417];
		break;
	case RECEIVE_LOG_IN_FAIL_CONNECT:		// 접속 오류.
		lpszMsg = GlobalText[408];
		break;
	case RECEIVE_LOG_IN_FAIL_ERROR:			// 로그인 입력 3번 틀림.
		lpszMsg = GlobalText[409];
		break;
	case RECEIVE_LOG_IN_FAIL_NO_PAYMENT_INFO:// 과금정보 없음.
		lpszMsg = GlobalText[433];
		break;
	case RECEIVE_LOG_IN_FAIL_USER_TIME1:	// 개인 정액 시간 만료.
		lpszMsg = GlobalText[410];
		break;
	case RECEIVE_LOG_IN_FAIL_USER_TIME2:	// 개인 정량 시간 만료.
		lpszMsg = GlobalText[411];
		break;
	case RECEIVE_LOG_IN_FAIL_PC_TIME1:		// IP 정액 시간 만료.
		lpszMsg = GlobalText[412];
		break;
	case RECEIVE_LOG_IN_FAIL_PC_TIME2:		// IP 정량 시간 만료.
		lpszMsg = GlobalText[413];
		break;
	case RECEIVE_LOG_IN_FAIL_ONLY_OVER_15:	// 15세 이상 이용가 서버.
		lpszMsg = GlobalText[435];
		break;
#ifdef ASG_ADD_CHARGED_CHANNEL_TICKET
	case RECEIVE_LOG_IN_FAIL_CHARGED_CHANNEL:// 유료채널 입장 불가. (글로벌 전용)
		lpszMsg = GlobalText[3118];
		break;
#endif	// ASG_ADD_CHARGED_CHANNEL_TICKET
	case RECEIVE_LOG_IN_FAIL_POINT_DATE:	// 포인트 날짜제 사용기간 만료. (대만 전용)
		lpszMsg = GlobalText[597];
		break;
	case RECEIVE_LOG_IN_FAIL_POINT_HOUR:	// 포인트 시간제 사용기간 만료. (대만 전용)
		lpszMsg = GlobalText[598];
		break;
	case RECEIVE_LOG_IN_FAIL_INVALID_IP:	// 접속 허가되지 않은 IP. (대만 전용)
		lpszMsg = GlobalText[599];
		break;
	case MESSAGE_DELETE_CHARACTER_GUILDWARNING:// 길드에 가입된 캐릭터 삭제 불가.
		lpszMsg = GlobalText[1654];
		break;
	case MESSAGE_DELETE_CHARACTER_WARNING:	// 제한 레벨 이상 삭제 불가.
//		lpszMsg = GlobalText[1711];
		::sprintf(szTempMsg, GlobalText[1711], CHAR_DEL_LIMIT_LV);
		lpszMsg = szTempMsg;
		break;
	case MESSAGE_DELETE_CHARACTER_CONFIRM:	// 캐릭터를 삭제하시겠습니까?
		::sprintf(szTempMsg, GlobalText[1712], CharactersClient[SelectedHero].ID);
		lpszMsg = szTempMsg;
		eType = MWT_BTN_BOTH;
		break;
	case MESSAGE_DELETE_CHARACTER_RESIDENT:	// 주민등록 번호 뒷자리 입력.
		lpszMsg = GlobalText[1713];
		eType = MWT_STR_INPUT;
		InitResidentNumInput();	// 입력 부분 초기화.		
		break;
	case MESSAGE_DELETE_CHARACTER_ITEM_BLOCK:// 아이템 블럭된 캐릭터.
		lpszMsg = GlobalText[439];
		break;
	case MESSAGE_STORAGE_RESIDENTWRONG:		// 주민등록번호 불일치.
		lpszMsg = GlobalText[401];
		break;
	case MESSAGE_DELETE_CHARACTER_SUCCESS:	// 캐릭터 삭제 성공.
		CharactersClient[SelectedHero].Object.Live = false;	// 캐릭터 삭제.
		DeleteBug(&CharactersClient[SelectedHero].Object);	// 유니리아, 디노란트, 펜릴등 삭제
		SelectedHero = -1;
		rUIMng.m_CharSelMainWin.UpdateDisplay();
		rUIMng.m_CharInfoBalloonMng.UpdateDisplay();
		lpszMsg = GlobalText[1714];
		break;
	case MESSAGE_BLOCKED_CHARACTER:			// 블럭된 캐릭터.
		lpszMsg = GlobalText[434];
		break;
	case MESSAGE_MIN_LENGTH:				// 입력된 글자수가 적음.
		lpszMsg = GlobalText[390];
		break;
	case MESSAGE_ID_SPACE_ERROR:			// 사용할 수 없는 단어가 포함.
		lpszMsg = GlobalText[1715];
		break;
	case MESSAGE_SPECIAL_NAME:				// 특수문자 사용 불가.
		lpszMsg = GlobalText[391];
		break;
	case RECEIVE_CREATE_CHARACTER_FAIL:		// 동일한 이름 존재.
		rUIMng.ShowWin(&rUIMng.m_CharMakeWin);
#ifdef PJH_CHARACTER_RENAME
		rUIMng.m_CharMakeWin.Set_State(rUIMng.m_CharMakeWin.ReName_Inter);
#endif //PJH_CHARACTER_RENAME

		lpszMsg = GlobalText[1716];
		break;
	case RECEIVE_CREATE_CHARACTER_FAIL2:	// 캐릭터를 더 만들 수 없음.
		rUIMng.ShowWin(&rUIMng.m_CharMakeWin);
#ifdef PJH_CHARACTER_RENAME
		rUIMng.m_CharMakeWin.Set_State();
#endif //PJH_CHARACTER_RENAME

		lpszMsg = GlobalText[396];
		break;
#ifdef ADD_EXPERIENCE_SERVER
	case MESSAGE_PCROOM_POSSIBLE:			// PC방에서만 접속 가능.
		lpszMsg = GlobalText[1698];
		break;
#endif	// ADD_EXPERIENCE_SERVER
		
#ifdef LDK_ADD_GLOBAL_PORTAL_SYSTEM
	case MESSAGE_NOT_EXECUTION_WEBSTARTER:	// 웹에서 로그인해주세요. (웹스타터 미실행)
#ifdef ASG_FIX_NOT_EXECUTION_WEBSTARTER_MSG
		lpszMsg = GlobalText[3117];
#else	// ASG_FIX_NOT_EXECUTION_WEBSTARTER_MSG
		lpszMsg = GlobalText[2743];
#endif	// ASG_FIX_NOT_EXECUTION_WEBSTARTER_MSG
		break;
#endif //LDK_ADD_GLOBAL_PORTAL_SYSTEM
#ifdef LJH_ADD_ONETIME_PASSWORD
	case MESSAGE_OTP_NOT_REGISTERED:		// OneTime Password 미등록
		lpszMsg = GlobalText[3108];			// "보다 안전한 원타임 패스워드의 이용을 추천합니다."
		lpszMsg2 = GlobalText[3109];		// "지금 바로 원타임 패스워드를 등록하시겠습니까?"
		eType = MWT_BTN_LOCALIZED_YESNO;
		break;
	case MESSAGE_OTP_INCORRECT:				// OneTime Password 불일치 
		lpszMsg = GlobalText[3111];			// "원타임 패스워드가 일치하지 않습니다."
		lpszMsg2 = GlobalText[3112];		// "다시 한번 확인바랍니다."
		break;
	case MESSAGE_OTP_NOT_WRITTEN:			// OneTime Password를 입력하지 않고 ok 클릭
		lpszMsg = GlobalText[3111];			// "원타임 패스워드가 일치하지 않습니다."
		lpszMsg2 = GlobalText[3112];		// "다시 한번 확인바랍니다."
		break;
	case MESSAGE_OTP_INVALID_INFO:
		lpszMsg = GlobalText[3113];			// "입력 정보가 정상적이지 않습니다."
		lpszMsg2 = GlobalText[3112];		// "다시 한번 확인바랍니다."
		eType = MWT_BTN_LOCALIZED_CONFIRM_ONLY;
		break;
#endif //LJH_ADD_ONETIME_PASSWORD

	default:
		m_nMsgCode = -1;
		return;
	}

	SetMsg(eType, lpszMsg, lpszMsg2);		// 메시지와 버튼 셋팅.
	rUIMng.ShowWin(this);
}

//*****************************************************************************
// 함수 이름 : ManageOKClick()
// 함수 설명 : 확인 버튼을 클릭했을 때의 처리.
//*****************************************************************************
void CMsgWin::ManageOKClick()
{
	CUIMng& rUIMng = CUIMng::Instance();
	rUIMng.HideWin(this);

	switch (m_nMsgCode)
	{
	case MESSAGE_SERVER_LOST:				// 서버 접속 실패.
	case MESSAGE_VERSION:					// 버전이 틀림.
	case RECEIVE_LOG_IN_FAIL_ERROR:			// 계정 3번 틀림.
	case RECEIVE_LOG_IN_FAIL_VERSION:		// 버전이 틀림.
#ifdef LDK_ADD_GLOBAL_PORTAL_SYSTEM
	case MESSAGE_NOT_EXECUTION_WEBSTARTER:	// 웹에서 로그인해주세요. (웹스타터 미실행)
#endif //LDK_ADD_GLOBAL_PORTAL_SYSTEM
		::PostMessage(g_hWnd, WM_CLOSE, 0, 0);
		break;
	case MESSAGE_INPUT_ID:					// 계정 입력 안함.
	case RECEIVE_LOG_IN_FAIL_ID:			// 계정 틀림.
	case RECEIVE_LOG_IN_FAIL_ID_CONNECTED:	// 접속 중인 계정임.
	case RECEIVE_LOG_IN_FAIL_SERVER_BUSY:	// 게임 서버가 포화 상태.
	case RECEIVE_LOG_IN_FAIL_ID_BLOCK:		// 블럭된 계정임.
	case RECEIVE_LOG_IN_FAIL_CONNECT:		// 접속 오류.
	case RECEIVE_LOG_IN_FAIL_NO_PAYMENT_INFO:// 과금정보 없음.
	case RECEIVE_LOG_IN_FAIL_USER_TIME1:	// 개인 정액 시간 만료.
	case RECEIVE_LOG_IN_FAIL_USER_TIME2:	// 개인 정량 시간 만료.
	case RECEIVE_LOG_IN_FAIL_PC_TIME1:		// IP 정액 시간 만료.
	case RECEIVE_LOG_IN_FAIL_PC_TIME2:		// IP 정량 시간 만료.
	case RECEIVE_LOG_IN_FAIL_ONLY_OVER_15:	// 15세 이상 이용가 서버.
	case RECEIVE_LOG_IN_FAIL_POINT_DATE:	// 포인트 날짜제 사용기간 만료.
	case RECEIVE_LOG_IN_FAIL_POINT_HOUR:	// 포인트 시간제 사용기간 만료.
	case RECEIVE_LOG_IN_FAIL_INVALID_IP:	// 접속 허가되지 않은 IP.
#ifdef ASG_ADD_CHARGED_CHANNEL_TICKET
	case RECEIVE_LOG_IN_FAIL_CHARGED_CHANNEL:// 유료채널 입장 불가. (글로벌 전용)
#endif	// ASG_ADD_CHARGED_CHANNEL_TICKET
		rUIMng.ShowWin(&rUIMng.m_LoginWin);
		CUIMng::Instance().m_LoginWin.GetIDInputBox()->GiveFocus(TRUE);
		CurrentProtocolState = RECEIVE_JOIN_SERVER_SUCCESS;
		break;
	case MESSAGE_INPUT_PASSWORD:			// 암호 입력 안함.
	case RECEIVE_LOG_IN_FAIL_PASSWORD:		// 암호 틀림.
		rUIMng.ShowWin(&rUIMng.m_LoginWin);
		CUIMng::Instance().m_LoginWin.GetPassInputBox()->GiveFocus(TRUE);
		CurrentProtocolState = RECEIVE_JOIN_SERVER_SUCCESS;
		break;
#ifdef ADD_EXPERIENCE_SERVER
	case MESSAGE_PCROOM_POSSIBLE:			// PC방에서만 접속 가능.
		CUIMng::Instance().m_LoginWin.ConnectConnectionServer();
		break;
#endif	// ADD_EXPERIENCE_SERVER
	case MESSAGE_DELETE_CHARACTER_CONFIRM:	// 캐릭터를 삭제하시겠습니까?
		PopUp(MESSAGE_DELETE_CHARACTER_RESIDENT);
		break;
	case MESSAGE_DELETE_CHARACTER_RESIDENT:	// 주민등록 번호 뒷자리 입력.
		RequestDeleteCharacter();
		PopUp(MESSAGE_WAIT);
		break;
#ifdef LJH_ADD_ONETIME_PASSWORD
	case MESSAGE_OTP_NOT_REGISTERED:
		// OneTime Password를 등록 할 수 있게 
		// 기본으로 설정되어 있는 web browser를 켜고 
		//채널 선택창으로 돌아간다.
		leaf::OpenExplorer("https://secure.muonline.jp/member/otk/index.asp");
		ReturnToConnServerWin();
		break;
	case MESSAGE_OTP_INCORRECT:				// OneTime Password가 틀림.
	case MESSAGE_OTP_NOT_WRITTEN:			// OneTime Password를 입력하지 않음.
		rUIMng.ShowWin(&rUIMng.m_OneTimePasswordWin);
		CUIMng::Instance().m_OneTimePasswordWin.GetOTPInputBox()->SetText(NULL);
		CUIMng::Instance().m_OneTimePasswordWin.GetOTPInputBox()->GiveFocus(TRUE);
		break;
	case MESSAGE_OTP_INVALID_INFO:
		ReturnToConnServerWin();
		break;
#endif //LJH_ADD_ONETIME_PASSWORD
	}
}

//*****************************************************************************
// 함수 이름 : ManageCancelClick()
// 함수 설명 : 취소 버튼을 클릭했을 때의 처리.
//*****************************************************************************
void CMsgWin::ManageCancelClick()
{
	CUIMng& rUIMng = CUIMng::Instance();

#ifdef LJH_ADD_ONETIME_PASSWORD
	switch (m_nMsgCode)
	{
		// 유저가 OneTime Password등록을 거절하였기 때문에 그냥 로그인 
		case MESSAGE_OTP_NOT_REGISTERED:
			SendingLoginInfoWOOPT();
			break;
	}
#endif //LJH_ADD_ONETIME_PASSWORD

	m_nMsgCode = -1;
	rUIMng.HideWin(this);
}

//*****************************************************************************
// 함수 이름 : InitResidentNumInput()
// 함수 설명 : 주민등록번호 입력 초기화.
//*****************************************************************************
void CMsgWin::InitResidentNumInput()
{
	::ClearInput();
	InputEnable = true;
	InputNumber = 1;
	InputTextMax[0] = g_iLengthAuthorityCode;
	InputTextHide[0] = 1;

	if (g_iChatInputType == 1)
	{
		g_pSinglePasswdInputBox->SetState(UISTATE_NORMAL);

#ifdef KJH_MOD_NATION_LANGUAGE_REDEFINE
	g_pSinglePasswdInputBox->SetBackColor(0, 0, 0, 0);
	
//----------------------------------------------------------------------------- Kor
// Kor
#ifdef _LANGUAGE_KOR
	g_pSinglePasswdInputBox->SetOption(UIOPTION_NUMBERONLY);
	g_pSinglePasswdInputBox->SetTextLimit(7);

//----------------------------------------------------------------------------- Eng
// Eng
#elif _LANGUAGE_ENG
	g_pSinglePasswdInputBox->SetOption(UIOPTION_NULL);
#ifdef LDK_MOD_PASSWORD_LENGTH_20
	g_pSinglePasswdInputBox->SetTextLimit(10);
#else // LDK_MOD_PASSWORD_LENGTH_20
	g_pSinglePasswdInputBox->SetTextLimit(7);
#endif // LDK_MOD_PASSWORD_LENGTH_20

//----------------------------------------------------------------------------- Tai
// Tai
#elif _LANGUAGE_TAI
	g_pSinglePasswdInputBox->SetOption(UIOPTION_SERIALNUMBER);
	g_pSinglePasswdInputBox->SetTextLimit(10);

//----------------------------------------------------------------------------- Chs
// Chs
#elif _LANGUAGE_CHS
	g_pSinglePasswdInputBox->SetOption(UIOPTION_SERIALNUMBER);
	g_pSinglePasswdInputBox->SetTextLimit(7);

//----------------------------------------------------------------------------- Jpn
// Jpn
#elif _LANGUAGE_JPN
	g_pSinglePasswdInputBox->SetOption(UIOPTION_NUMBERONLY);
	g_pSinglePasswdInputBox->SetTextLimit(8);

//----------------------------------------------------------------------------- Tha
// Tha
#elif _LANGUAGE_THA
	g_pSinglePasswdInputBox->SetOption(UIOPTION_NUMBERONLY);
	g_pSinglePasswdInputBox->SetTextLimit(7);

//----------------------------------------------------------------------------- Phi
// Phi
#elif _LANGUAGE_PHI
	g_pSinglePasswdInputBox->SetOption(UIOPTION_NUMBERONLY);
	g_pSinglePasswdInputBox->SetTextLimit(7);

//----------------------------------------------------------------------------- Vie
// Vie
#elif _LANGUAGE_VIE
	g_pSinglePasswdInputBox->SetOption(UIOPTION_NUMBERONLY);
	g_pSinglePasswdInputBox->SetTextLimit(7);

#endif
//-----------------------------------------------------------------------------

#else // KJH_MOD_NATION_LANGUAGE_REDEFINE
#if (SELECTED_LANGUAGE == LANGUAGE_TAIWANESE) || ( SELECTED_LANGUAGE==LANGUAGE_CHINESE )
		// 대만, 중국
		g_pSinglePasswdInputBox->SetOption(UIOPTION_SERIALNUMBER);
#elif ( SELECTED_LANGUAGE == LANGUAGE_ENGLISH )
		// 미국
		g_pSinglePasswdInputBox->SetOption(UIOPTION_NULL);
#else // (SELECTED_LANGUAGE == LANGUAGE_TAIWANESE) || ( SELECTED_LANGUAGE==LANGUAGE_CHINESE )
		// 한국
		g_pSinglePasswdInputBox->SetOption(UIOPTION_NUMBERONLY);
#endif // (SELECTED_LANGUAGE == LANGUAGE_TAIWANESE) || ( SELECTED_LANGUAGE==LANGUAGE_CHINESE )

		g_pSinglePasswdInputBox->SetBackColor(0, 0, 0, 0);

#if (SELECTED_LANGUAGE == LANGUAGE_JAPANESE)
		// 일본
		g_pSinglePasswdInputBox->SetTextLimit(8);
#elif (SELECTED_LANGUAGE == LANGUAGE_TAIWANESE)
		// 대만
		g_pSinglePasswdInputBox->SetTextLimit(10);

#elif defined( LDK_MOD_PASSWORD_LENGTH_20 ) && ( SELECTED_LANGUAGE == LANGUAGE_ENGLISH )
		// 미국
		g_pSinglePasswdInputBox->SetTextLimit(20);
#else // (SELECTED_LANGUAGE == LANGUAGE_JAPANESE)
		// 한국
		g_pSinglePasswdInputBox->SetTextLimit(7);
#endif	// (SELECTED_LANGUAGE == LANGUAGE_JAPANESE)
#endif // KJH_MOD_NATION_LANGUAGE_REDEFINE

		g_pSinglePasswdInputBox->GiveFocus();
	}
}

//*****************************************************************************
// 함수 이름 : RequestDeleteCharacter()
// 함수 설명 : 캐릭터 삭제 요청.
//*****************************************************************************
void CMsgWin::RequestDeleteCharacter()
{
	if (g_iChatInputType == 1)
	{
		g_pSinglePasswdInputBox->GetText(InputText[0]);
		g_pSinglePasswdInputBox->SetText(NULL);
		g_pSinglePasswdInputBox->SetState(UISTATE_HIDE);
	}
	InputEnable = false;
	SendRequestDeleteCharacter(CharactersClient[SelectedHero].ID, InputText[0]);
}

#ifdef LJH_ADD_ONETIME_PASSWORD
//*****************************************************************************
// 함수 이름 :	ReturnToConnServerWin()
// 함수 설명 :	OPT 미등록 유저가 등록(YES)버튼을 클릭.
//				게임 서버 접속을 끊고 접속 서버에 다시 연결.
//*****************************************************************************
void CMsgWin::ReturnToConnServerWin()
{
	SocketClient.Close();		// 게임 서버 접속을 끊음.
	LogIn = 0;
	CurrentProtocolState = REQUEST_JOIN_SERVER;

	// 접속 서버에 다시 연결.
    ::CreateSocket(szServerIpAddress, g_ServerPort);

	ClearLoginInfoFromInputBox();
}

//*****************************************************************************
// 함수 이름 : SendingLoginInfoWOOPT()
// 함수 설명 : 서버에 로그인에 필요한 정보들 전송 W/O OneTime Password.
//*****************************************************************************
void CMsgWin::SendingLoginInfoWOOPT()
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
	
		SendRequestOTP(szID, szPW, szOTP, 0x02);
	}
}

//*****************************************************************************
// 함수 이름 : ClearLoginInfoFromInputBox()
// 함수 설명 : for better security, remove the password and OneTime Password from the inputbox.
//*****************************************************************************
void CMsgWin::ClearLoginInfoFromInputBox()
{
		CUIMng::Instance().m_LoginWin.GetPassInputBox()->SetText(NULL);
		CUIMng::Instance().m_OneTimePasswordWin.GetOTPInputBox()->SetText(NULL);
}
#endif //LJH_ADD_ONETIME_PASSWORD