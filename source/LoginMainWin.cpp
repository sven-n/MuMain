//*****************************************************************************
// File: LoginMainWin.cpp
//
// Desc: implementation of the CLoginMainWin class.
//
// producer: Ahn Sang-Kyu
//*****************************************************************************

#include "stdafx.h"
#include "LoginMainWin.h"

#include "Input.h"
#include "UIMng.h"

// ::PlayBuffer(SOUND_CLICK01) 클릭 사운드 때문에 아래 Include ㅜㅜ.
#include "ZzzBMD.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "DSPlaySound.h"

// SendRequestServerList() 때문에 Include.
#include "ZzzCharacter.h"
#include "wsclientinline.h"

#ifdef MOVIE_DIRECTSHOW
#include <dshow.h>
#include "MovieScene.h"

extern CMovieScene* g_pMovieScene;
#endif // MOVIE_DIRECTSHOW

extern char* g_lpszMp3[NUM_MUSIC];

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLoginMainWin::CLoginMainWin()
{

}

CLoginMainWin::~CLoginMainWin()
{

}

//*****************************************************************************
// 함수 이름 : Create()
// 함수 설명 : 로그인씬 메인 창 생성.
//*****************************************************************************
// 버튼은 등록할 것.
void CLoginMainWin::Create()
{
	// 버튼 생성.
	for (int i = 0; i <= LMW_BTN_CREDIT; ++i)
		m_aBtn[i].Create(54, 30, BITMAP_LOG_IN+4 + i, 3, 2, 1);
#ifdef MOVIE_DIRECTSHOW
	m_aBtn[LMW_BTN_MOVIE].Create(54, 30, BITMAP_LOG_IN+15, 3, 2, 1);
#endif	// MOVIE_DIRECTSHOW

	// 기초 윈도우 생성.
	CWin::Create(CInput::Instance().GetScreenWidth() - 30 * 2,
		m_aBtn[0].GetHeight(), -2);

	// 버튼 등록.(기초 윈도우 생성되어야 버튼 등록 가능)
#ifdef _VS2008PORTING
	for (int i = 0; i < LMW_BTN_MAX; ++i)
#else // _VS2008PORTING
	for (i = 0; i < LMW_BTN_MAX; ++i)
#endif // _VS2008PORTING
		CWin::RegisterButton(&m_aBtn[i]);

	// 우측 하단 장식 스프라이트 생성.
	m_sprDeco.Create(189, 103, BITMAP_LOG_IN+6, 0, NULL, 105, 59);
}

//*****************************************************************************
// 함수 이름 : PreRelease()
// 함수 설명 : 모든 컨트롤 릴리즈.(버튼은 자동 삭제)
//*****************************************************************************
// 등록된 버튼과 기초 윈도우를 제외한 나머지 Release() 및 메모리 해제.
void CLoginMainWin::PreRelease()
{
	m_sprDeco.Release();
}

//*****************************************************************************
// 함수 이름 : SetPosition()
// 함수 설명 : 창 위치 지정.
// 매개 변수 : nXCoord	: 스크린 X좌표.
//			   nYCoord	: 스크린 Y좌표.
//*****************************************************************************
void CLoginMainWin::SetPosition(int nXCoord, int nYCoord)
{
	CWin::SetPosition(nXCoord, nYCoord);

	m_aBtn[LMW_BTN_MENU].SetPosition(nXCoord, nYCoord);
	m_aBtn[LMW_BTN_CREDIT].SetPosition(
		nXCoord + CWin::GetWidth() - m_aBtn[LMW_BTN_CREDIT].GetWidth(),
		nYCoord);
#ifdef MOVIE_DIRECTSHOW
	m_aBtn[LMW_BTN_MOVIE].SetPosition(m_aBtn[LMW_BTN_CREDIT].GetXPos()
		- 10 - m_aBtn[LMW_BTN_MOVIE].GetWidth(), nYCoord);
#endif	// MOVIE_DIRECTSHOW
	m_sprDeco.SetPosition(
		m_aBtn[LMW_BTN_CREDIT].GetXPos(), m_aBtn[LMW_BTN_CREDIT].GetYPos());
}

//*****************************************************************************
// 함수 이름 : Show()
// 함수 설명 : 창을 보여 주거나 안보이게함.
// 매개 변수 : bShow	: true이면 보여줌.
//*****************************************************************************
void CLoginMainWin::Show(bool bShow)
{
	CWin::Show(bShow);

	for (int i = 0; i < LMW_BTN_MAX; ++i)
		m_aBtn[i].Show(bShow);
	m_sprDeco.Show(bShow);
}

//*****************************************************************************
// 함수 이름 : CursorInWin()
// 함수 설명 : 윈도우 영역 안에 마우스 커서가 위치하는가?
// 매개 변수 : eArea	: 검사할 영역.(win.h의 #define 참조)
//*****************************************************************************
// 등록된 버튼 영역은 설정할 필요 없음.
// 단, 다른 컨트롤이나 영역을 버튼과 같은 효과로 처리하려면 기초 클래스의
//CursorInButtonlike() 함수를 재정의만 하면 된다.
bool CLoginMainWin::CursorInWin(int nArea)
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
void CLoginMainWin::UpdateWhileActive()
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
void CLoginMainWin::UpdateWhileActive(double dDeltaTick)
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
{
	if (m_aBtn[LMW_BTN_MENU].IsClick())			// 메뉴 버튼 클릭.
	{
		CUIMng& rUIMng = CUIMng::Instance();
		rUIMng.ShowWin(&rUIMng.m_SysMenuWin);
		rUIMng.SetSysMenuWinShow(true);
	}
	else if (m_aBtn[LMW_BTN_CREDIT].IsClick())	// 크레딧 버튼 클릭.
	{
		// 5분 동안 아무짓 안하면 서버에서 끊어버리기 때문.
#ifdef PKD_ADD_ENHANCED_ENCRYPTION
		SendRequestServerList2();
#else
		SendRequestServerList();
#endif // PKD_ADD_ENHANCED_ENCRYPTION

		CUIMng& rUIMng = CUIMng::Instance();
		rUIMng.ShowWin(&rUIMng.m_CreditWin);

		::StopMp3(g_lpszMp3[MUSIC_MAIN_THEME]);
		::PlayMp3(g_lpszMp3[MUSIC_MUTHEME]);
	}
#ifdef MOVIE_DIRECTSHOW
	else if (m_aBtn[LMW_BTN_MOVIE].IsClick())	// 동영상 버튼 클릭.
	{
		g_pMovieScene = new CMovieScene;
		g_pMovieScene->Initialize_DirectShow(g_hWnd, MOVIE_FILE_WMV);
		if(g_pMovieScene->IsFile() == FALSE || g_pMovieScene->IsFailDirectShow() == TRUE)
		{
			g_pMovieScene->Destroy();
			SAFE_DELETE(g_pMovieScene);
			return;	
		}
		::StopMp3(g_lpszMp3[MUSIC_MAIN_THEME]);
		CUIMng& rUIMng = CUIMng::Instance();
		rUIMng.SetMoving(true);
	}
#endif	// MOVIE_DIRECTSHOW
}

//*****************************************************************************
// 함수 이름 : RenderControls()
// 함수 설명 : 각종 컨트롤 렌더.
//*****************************************************************************
// CWin::RenderButtons()를 사용해서 버튼 랜더 순서를 정할 수 있다.
// 윈도우 배경은 설정할 필요 없다.
void CLoginMainWin::RenderControls()
{
	m_sprDeco.Render();
	CWin::RenderButtons();
}