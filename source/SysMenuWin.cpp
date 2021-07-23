//*****************************************************************************
// File: SysMenuWin.cpp
//
// Desc: implementation of the CSysMenuWin class.
//
// producer: Ahn Sang-Kyu
//*****************************************************************************

#include "stdafx.h"
#include "SysMenuWin.h"

#include "Input.h"
#include "UIMng.h"
#include "ZzzInfomation.h"
#include "ZzzScene.h"

// SendRequestLogOut() 사용하기 위해.ㅜㅜ
#include "ZzzBMD.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "wsclientinline.h"

#include "GMCryWolf1st.h"

#define	SMW_BTN_GAP		4		// 버튼 간 높이 갭.

extern int  SceneFlag;
extern CWsctlc SocketClient;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSysMenuWin::CSysMenuWin()
{

}

CSysMenuWin::~CSysMenuWin()
{

}

//*****************************************************************************
// 함수 이름 : Create()
// 함수 설명 : 시스템 메뉴 창 생성.
//*****************************************************************************
void CSysMenuWin::Create()
{
	CInput rInput = CInput::Instance();
	CWin::Create(rInput.GetScreenWidth(), rInput.GetScreenHeight());

// 배경 윈도우 생성.
	SImgInfo aiiBack[WE_BG_MAX] = 
	{
		{ BITMAP_SYS_WIN, 0, 0, 128, 128 },	// 중앙
		{ BITMAP_SYS_WIN+1, 0, 0, 213, 64 },// 상
		{ BITMAP_SYS_WIN+2, 0, 0, 213, 43 },// 하
		{ BITMAP_SYS_WIN+3, 0, 0, 5, 8 },	// 좌
		{ BITMAP_SYS_WIN+4, 0, 0, 5, 8 }	// 우
	};
	m_winBack.Create(aiiBack, 1, 10);

// 버튼 생성.
	const char* apszBtnText[SMW_BTN_MAX] =
	{ GlobalText[381], GlobalText[382], GlobalText[385], GlobalText[388] };
	DWORD adwBtnClr[4] =
	{ CLRDW_BR_GRAY, CLRDW_BR_GRAY, CLRDW_WHITE, 0 };
	for (int i = 0; i < SMW_BTN_MAX; ++i)
	{
		m_aBtn[i].Create(108, 30, BITMAP_TEXT_BTN, 4, 2, 1);
		m_aBtn[i].SetText(apszBtnText[i], adwBtnClr);
		CWin::RegisterButton(&m_aBtn[i]);
	}

// 씬마다 버튼 수나 배경 창 크기가 바뀌므로.
	switch (SceneFlag)
	{
	case LOG_IN_SCENE:
		m_aBtn[SMW_BTN_SERVER_SEL].SetEnable(false);
		m_winBack.SetLine(6);
		break;
	case CHARACTER_SCENE:
		m_aBtn[SMW_BTN_SERVER_SEL].SetEnable(true);
		m_winBack.SetLine(10);
		break;
	}

	SetPosition((rInput.GetScreenWidth() - m_winBack.GetWidth()) / 2,
		(rInput.GetScreenHeight() - m_winBack.GetHeight()) / 2);
}

//*****************************************************************************
// 함수 이름 : PreRelease()
// 함수 설명 : 모든 컨트롤 릴리즈.(버튼은 자동 삭제)
//*****************************************************************************
void CSysMenuWin::PreRelease()
{
	m_winBack.Release();
}

//*****************************************************************************
// 함수 이름 : SetPosition()
// 함수 설명 : 창 위치 지정.
//			   (화면 상에는 창이 이동하는것처럼 보이지만 기본 창은 이동이 없음)
// 매개 변수 : nXCoord	: 스크린 X좌표.
//			   nYCoord	: 스크린 Y좌표.
//*****************************************************************************
void CSysMenuWin::SetPosition(int nXCoord, int nYCoord)
{
	m_winBack.SetPosition(nXCoord, nYCoord);

	// 버튼은 항상 중앙.
	int nBtnPosX = m_winBack.GetXPos()
		+ (m_winBack.GetWidth() - m_aBtn[0].GetWidth()) / 2;
	int nBtnGap = SMW_BTN_GAP + m_aBtn[0].GetHeight();
	// 게임 종료 버튼부터 옵션 버튼 전 버튼 까지는 위를 기준으로 위치 시킴.
	int nBtnPosBaseTop = m_winBack.GetYPos() + 33;
	for (int i = 0; i < SMW_BTN_OPTION; ++i)
		m_aBtn[i].SetPosition(nBtnPosX, nBtnPosBaseTop + i * nBtnGap);

	// 옵션 버튼과 닫기 버튼은 밑을 기준으로 위치시킴.
	int nCloseBtnPosY = m_winBack.GetYPos() + m_winBack.GetHeight() - 52;
	m_aBtn[SMW_BTN_CLOSE].SetPosition(nBtnPosX, nCloseBtnPosY);
	m_aBtn[SMW_BTN_OPTION].SetPosition(nBtnPosX, nCloseBtnPosY - nBtnGap);
}

//*****************************************************************************
// 함수 이름 : Show()
// 함수 설명 : 창을 보여 주거나 안보이게함.
// 매개 변수 : bShow	: true이면 보여줌.
//*****************************************************************************
void CSysMenuWin::Show(bool bShow)
{
	CWin::Show(bShow);

	m_winBack.Show(bShow);
	for (int i = 0; i < SMW_BTN_MAX; ++i)
		m_aBtn[i].Show(bShow);
}

//*****************************************************************************
// 함수 이름 : CursorInWin()
// 함수 설명 : 윈도우 영역 안에 마우스 커서가 위치하는가?
// 매개 변수 : eArea	: 검사할 영역.(win.h의 #define 참조)
//*****************************************************************************
bool CSysMenuWin::CursorInWin(int nArea)
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
void CSysMenuWin::UpdateWhileActive()
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
void CSysMenuWin::UpdateWhileActive(double dDeltaTick)
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
{
	if (m_aBtn[SMW_BTN_GAME_END].IsClick())	// 게임 종료.
	{
		CUIMng::Instance().PopUpMsgWin(MESSAGE_GAME_END_COUNTDOWN);
	}
	else if (m_aBtn[SMW_BTN_SERVER_SEL].IsClick())	// 서버 선택(로그인) 씬으로.
	{
		g_ErrorReport.Write("> Menu - Join another server.");
		g_ErrorReport.WriteCurrentTime();
		SendRequestLogOut(2);

		// 자신과 캐릭터 선택씬 메인창을 닫음.
		CUIMng& rUIMng = CUIMng::Instance();
		rUIMng.HideWin(this);
		rUIMng.HideWin(&rUIMng.m_CharSelMainWin);
	}
	else if (m_aBtn[SMW_BTN_OPTION].IsClick())
	{
		CUIMng& rUIMng = CUIMng::Instance();
		rUIMng.HideWin(this);
		rUIMng.ShowWin(&rUIMng.m_OptionWin);
	}
	else if (m_aBtn[SMW_BTN_CLOSE].IsClick())		// 닫기.
	{
		CUIMng::Instance().SetSysMenuWinShow(false);
		CUIMng::Instance().HideWin(this);
	}
	else if (CInput::Instance().IsKeyDown(VK_ESCAPE)
		&& !CUIMng::Instance().IsSysMenuWinShow())// 닫기.
	{
		::PlayBuffer(SOUND_CLICK01);	// 클릭 사운드.
		CUIMng::Instance().HideWin(this);
	}		
}

//*****************************************************************************
// 함수 이름 : RenderControls()
// 함수 설명 : 각종 컨트롤 렌더.
//*****************************************************************************
void CSysMenuWin::RenderControls()
{
	m_winBack.Render();
	CWin::RenderButtons();
}
