//*****************************************************************************
// File: UIMng.cpp
//
// Desc: implementation of the CUIMng class.
//
// producer: Ahn Sang-Kyu
//*****************************************************************************

#include "stdafx.h"
#include "UIMng.h"
#include "Input.h"
#include "Sprite.h"
#include "GaugeBar.h"
#include "ZzzOpenglUtil.h"
#include "Zzzinfomation.h"
#include "ZzzBMD.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzInterface.h"
#include "GameCensorship.h"
#include "UIControls.h"

#ifdef KJH_ADD_SERVER_LIST_SYSTEM
#include "ServerListManager.h"
#endif // KJH_ADD_SERVER_LIST_SYSTEM

#define	DOCK_EXTENT		10		///< 도킹 범위.

// 타이틀 씬 스프라이트(TS)
//#define	UIM_TS_BG_BLACK		0	// 검정 배경색.
#define	UIM_TS_BACK0		0	// 배경0
#define	UIM_TS_BACK1		1	// 배경1
#define	UIM_TS_MU			2	// 뮤 로고
#define	UIM_TS_121518		3	// 전체 이용가 마크
#define	UIM_TS_WEBZEN		4	// 웹젠 로고
#ifdef PBG_ENGLISHLOGO_CHANGE
	#define UIM_TS_WEBZEN_WITH	5	//글로벌 추가 로고
	#define	UIM_TS_MAX			6	// 스프라이트 최대 개수
#else //PBG_ENGLISHLOGO_CHANGE
#ifdef PBG_ADD_NEWLOGO_IMAGECHANGE
	#define UIM_TS_BACK2		5	// 배경2
	#define UIM_TS_BACK3		6	// 배경3
	#define UIM_TS_BACK4		7	// 배경4
	#define UIM_TS_BACK5		8	// 배경5
	#define UIM_TS_BACK6		9	// 배경6
	#define UIM_TS_BACK7		10	// 배경7
	#define UIM_TS_BACK8		11	// 배경8
	#define UIM_TS_BACK9		12	// 배경9
	#define	UIM_TS_MAX			13	// 스프라이트 최대 개수
#else //PBG_ADD_NEWLOGO_IMAGECHANGE
	#define	UIM_TS_MAX			5	// 스프라이트 최대 개수
#endif //PBG_ADD_NEWLOGO_IMAGECHANGE
#endif //PBG_ENGLISHLOGO_CHANGE

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUIMng::CUIMng()
{
	m_asprTitle = NULL;
	m_pgbLoding = NULL;
	m_pLoadingScene = NULL;
	
#ifdef MOVIE_DIRECTSHOW
	m_bMoving = false;
#endif // MOVIE_DIRECTSHOW
}

CUIMng::~CUIMng()
{

}

//*****************************************************************************
// 함수 이름 : Instance()
// 함수 설명 : 객체를 단 하나만 생성.
//			  이 클래스에 접근하기 위한 인터페이스임
//*****************************************************************************
CUIMng& CUIMng::Instance()
{
	static CUIMng s_UIMng;
    return s_UIMng;
}

//*****************************************************************************
// 함수 이름 : CreateTitleSceneUI()
// 함수 설명 : 타이틀 씬 UI 생성.
//*****************************************************************************
void CUIMng::CreateTitleSceneUI()
{
// 타이틀 씬 UI가 있다면 삭제.
	ReleaseTitleSceneUI();

	g_GameCensorship->SetVisible(true);
	g_GameCensorship->SetState(SEASON3A::CGameCensorship::STATE_LOADING);

	CInput& rInput = CInput::Instance();
// 타이틀 씬은 800 * 600 가상 스크린 모드를 사용 함.
	float fScaleX = (float)rInput.GetScreenWidth() / 800.0f;
	float fScaleY = (float)rInput.GetScreenHeight() / 600.0f;

// 모든 스프라이트 생성 및 배치.
	m_asprTitle = new CSprite[UIM_TS_MAX];

#ifdef PBG_ADD_NEWLOGO_IMAGECHANGE
	float _fScaleXTemp = (float)rInput.GetScreenWidth()/1280.0f;
	float _fScaleYTemp = (float)rInput.GetScreenHeight()/1024.0f;

	m_asprTitle[UIM_TS_BACK0].Create(400, 69, BITMAP_TITLE, 0, NULL, 0, 0,
		false, SPR_SIZING_DATUMS_LT, fScaleX, fScaleY);
	m_asprTitle[UIM_TS_BACK0].SetPosition(0, 0);

	m_asprTitle[UIM_TS_BACK1].Create(400, 69, BITMAP_TITLE+1, 0, NULL, 0, 0,
		false, SPR_SIZING_DATUMS_LT, fScaleX, fScaleY);
	m_asprTitle[UIM_TS_BACK1].SetPosition(400, 0);

	m_asprTitle[UIM_TS_BACK2].Create(400, 100, BITMAP_TITLE+6, 0, NULL, 0, 0,
		false, SPR_SIZING_DATUMS_LT, fScaleX, fScaleY);
	m_asprTitle[UIM_TS_BACK2].SetPosition(0, 500);

	m_asprTitle[UIM_TS_BACK3].Create(400, 100, BITMAP_TITLE+7, 0, NULL, 0, 0,
		false, SPR_SIZING_DATUMS_LT, fScaleX, fScaleY);
	m_asprTitle[UIM_TS_BACK3].SetPosition(400, 500);

	m_asprTitle[UIM_TS_BACK4].Create(512, 512, BITMAP_TITLE+8, 0, NULL, 0, 0,
		false, SPR_SIZING_DATUMS_LT, _fScaleXTemp, _fScaleYTemp);
	m_asprTitle[UIM_TS_BACK4].SetPosition(0, 119);

	m_asprTitle[UIM_TS_BACK5].Create(512, 512, BITMAP_TITLE+9, 0, NULL, 0, 0,
		false, SPR_SIZING_DATUMS_LT, _fScaleXTemp, _fScaleYTemp);
	m_asprTitle[UIM_TS_BACK5].SetPosition(512, 119);

	m_asprTitle[UIM_TS_BACK6].Create(256, 512, BITMAP_TITLE+10, 0, NULL, 0, 0,
		false, SPR_SIZING_DATUMS_LT, _fScaleXTemp, _fScaleYTemp);
	m_asprTitle[UIM_TS_BACK6].SetPosition(1024, 119);

	m_asprTitle[UIM_TS_BACK7].Create(512, 223, BITMAP_TITLE+11, 0, NULL, 0, 0,
		false, SPR_SIZING_DATUMS_LT, _fScaleXTemp, _fScaleYTemp);
	m_asprTitle[UIM_TS_BACK7].SetPosition(0, 512+119);

	m_asprTitle[UIM_TS_BACK8].Create(512, 223, BITMAP_TITLE+12, 0, NULL, 0, 0,
		false, SPR_SIZING_DATUMS_LT, _fScaleXTemp, _fScaleYTemp);
	m_asprTitle[UIM_TS_BACK8].SetPosition(512, 512+119);

	m_asprTitle[UIM_TS_BACK9].Create(256, 223, BITMAP_TITLE+13, 0, NULL, 0, 0,
		false, SPR_SIZING_DATUMS_LT, _fScaleXTemp, _fScaleYTemp);
	m_asprTitle[UIM_TS_BACK9].SetPosition(1024, 512+119);

#ifdef PBG_ADD_MU_LOGO
#ifdef PBG_ADD_MUBLUE_LOGO
	// 블루 뮤
	m_asprTitle[UIM_TS_MU].Create(167, 168, BITMAP_TITLE+2, 0, NULL, 0, 0,
		false, SPR_SIZING_DATUMS_LT, _fScaleXTemp, _fScaleXTemp);
#else //PBG_ADD_MUBLUE_LOGO
 	m_asprTitle[UIM_TS_MU].Create(216, 138, BITMAP_TITLE+2, 0, NULL, 0, 0,
		false, SPR_SIZING_DATUMS_LT, _fScaleXTemp, _fScaleXTemp);
#endif //PBG_ADD_MUBLUE_LOGO
#else //PBG_ADD_MU_LOGO
	m_asprTitle[UIM_TS_MU].Create(106, 49, BITMAP_TITLE+2, 0, NULL, 0, 0,
		false, SPR_SIZING_DATUMS_LT, _fScaleXTemp, _fScaleXTemp);
#endif //PBG_ADD_MU_LOGO
	if((float)rInput.GetScreenWidth() == 1280)
#ifdef PBG_ADD_MU_LOGO
#ifdef PBG_ADD_MUBLUE_LOGO
		//블루 뮤
		m_asprTitle[UIM_TS_MU].SetPosition(640-108, 633+53);
#else //PBG_ADD_MUBLUE_LOGO
		m_asprTitle[UIM_TS_MU].SetPosition(640-108, 663+53);
#endif //PBG_ADD_MUBLUE_LOGO
#else //PBG_ADD_MU_LOGO
		m_asprTitle[UIM_TS_MU].SetPosition(1150, 775);
#endif //PBG_ADD_MU_LOGO
	else
#ifdef PBG_ADD_MU_LOGO
#ifdef PBG_ADD_MUBLUE_LOGO
		m_asprTitle[UIM_TS_MU].SetPosition(640-83, 633);
#else //PBG_ADD_MUBLUE_LOGO
		m_asprTitle[UIM_TS_MU].SetPosition(640-108, 663);
#endif //PBG_ADD_MUBLUE_LOGO
#else //PBG_ADD_MU_LOGO
		m_asprTitle[UIM_TS_MU].SetPosition(1150, 725);
#endif //PBG_ADD_MU_LOGO
	
#else //PBG_ADD_NEWLOGO_IMAGECHANGE
	m_asprTitle[UIM_TS_BACK0].Create(512, 478, BITMAP_TITLE, 0, NULL, 0, 0,
		false, SPR_SIZING_DATUMS_LT, fScaleX, fScaleY);
	m_asprTitle[UIM_TS_BACK0].SetPosition(0, 60);

	m_asprTitle[UIM_TS_BACK1].Create(288, 478, BITMAP_TITLE+1, 0, NULL, 0, 0,
		false, SPR_SIZING_DATUMS_LT, fScaleX, fScaleY);
	m_asprTitle[UIM_TS_BACK1].SetPosition(512, 60);

	m_asprTitle[UIM_TS_MU].Create(93, 48, BITMAP_TITLE+2, 0, NULL, 0, 0,
		false, SPR_SIZING_DATUMS_LT, fScaleX, fScaleY);
	m_asprTitle[UIM_TS_MU].SetPosition(3, 80);
#endif //PBG_ADD_NEWLOGO_IMAGECHANGE

	m_asprTitle[UIM_TS_121518].Create(256, 206, BITMAP_TITLE+3, 0, NULL, 0, 0,
		false, SPR_SIZING_DATUMS_LT, fScaleX, fScaleY);
	m_asprTitle[UIM_TS_121518].SetPosition(544, 60);

#if SELECTED_LANGUAGE == LANGUAGE_ENGLISH
#ifdef PBG_ENGLISHLOGO_CHANGE
/*
	//로고 위아래로 찍히게
	//Webzen 로고
	m_asprTitle[UIM_TS_WEBZEN_WITH].Create(187, 151, BITMAP_TITLE+4, 0, NULL, 0, 0,
		false, SPR_SIZING_DATUMS_LT, fScaleX, fScaleY);
	m_asprTitle[UIM_TS_WEBZEN_WITH].SetPosition(307, 125);

	//글로벌 로고
	m_asprTitle[UIM_TS_WEBZEN].Create(512, 122, BITMAP_TITLE+6, 0, NULL, 0, 0,
		false, SPR_SIZING_DATUMS_LT, fScaleX, fScaleY);
	m_asprTitle[UIM_TS_WEBZEN].SetPosition(144, 255);
*/
	//로고 좌우로 찍히게..
	//Webzen 로고
	m_asprTitle[UIM_TS_WEBZEN_WITH].Create(187, 151, BITMAP_TITLE+4, 0, NULL, 0, 0,
		false, SPR_SIZING_DATUMS_LT, fScaleX, fScaleY);
	m_asprTitle[UIM_TS_WEBZEN_WITH].SetPosition(570, 205);
	
	//글로벌 로고
	m_asprTitle[UIM_TS_WEBZEN].Create(512, 122, BITMAP_TITLE+6, 0, NULL, 0, 0,
		false, SPR_SIZING_DATUMS_LT, fScaleX, fScaleY);
	m_asprTitle[UIM_TS_WEBZEN].SetPosition(44, 225);

#else //PBG_ENGLISHLOGO_CHANGE

#ifdef LDK_MOD_GLOBAL_PORTAL_LOGO
	//기본 로고로 재수정.
	m_asprTitle[UIM_TS_WEBZEN].Create(187, 151, BITMAP_TITLE+4, 0, NULL, 0, 0,
		false, SPR_SIZING_DATUMS_LT, fScaleX, fScaleY);
	m_asprTitle[UIM_TS_WEBZEN].SetPosition(307, 225);
#else //LDK_MOD_GLOBAL_PORTAL_LOGO
	m_asprTitle[UIM_TS_WEBZEN].Create(342, 152, BITMAP_TITLE+4, 0, NULL, 0, 0,
		false, SPR_SIZING_DATUMS_LT, fScaleX, fScaleY);
	m_asprTitle[UIM_TS_WEBZEN].SetPosition(307-77, 225);
#endif //LDK_MOD_GLOBAL_PORTAL_LOGO

#endif //PBG_ENGLISHLOGO_CHANGE
#else //SELECTED_LANGUAGE == LANGUAGE_ENGLISH
#ifdef PBG_ADD_NEWLOGO_IMAGECHANGE
	m_asprTitle[UIM_TS_WEBZEN].Create(53, 46, BITMAP_TITLE+4, 0, NULL, 0, 0,
		false, SPR_SIZING_DATUMS_LT, _fScaleXTemp*1.2f, _fScaleYTemp*1.2f);
	m_asprTitle[UIM_TS_WEBZEN].SetPosition(16, 8);
#else //PBG_ADD_NEWLOGO_IMAGECHANGE
	m_asprTitle[UIM_TS_WEBZEN].Create(187, 151, BITMAP_TITLE+4, 0, NULL, 0, 0,
		false, SPR_SIZING_DATUMS_LT, fScaleX, fScaleY);
	m_asprTitle[UIM_TS_WEBZEN].SetPosition(307, 225);
#endif //PBG_ADD_NEWLOGO_IMAGECHANGE
#endif //SELECTED_LANGUAGE == LANGUAGE_ENGLISH

// 기본 데이타 로딩 진행바 생성 및 배치.
	m_pgbLoding = new CGaugeBar;
	
	RECT rc = { 0, 0, 656, 15 };
	m_pgbLoding->Create(4, 15, BITMAP_TITLE+5, &rc, 0, 0, -1, true, fScaleX,
		fScaleY);
#ifdef PBG_ADD_NEWLOGO_IMAGECHANGE
	m_pgbLoding->SetPosition(72, 540);
#else //PBG_ADD_NEWLOGO_IMAGECHANGE
	m_pgbLoding->SetPosition(72, 482);
#endif //PBG_ADD_NEWLOGO_IMAGECHANGE
// 모든 걸 화면에 보여 줌.
	for (int i = 0; i < UIM_TS_MAX; ++i)
	{
		m_asprTitle[i].Show();
	}
	m_pgbLoding->Show();

	// 해외는 원래 등급을 안뿌리고, 한국은 새로운 등급제를 렌더링 하고 있으니깐 무조건 false로 임시로 한다.	
	m_asprTitle[UIM_TS_121518].Show(false);

	m_nScene = UIM_SCENE_TITLE;
}

//*****************************************************************************
// 함수 이름 : ReleaseTitleSceneUI()
// 함수 설명 : 타이틀 씬 UI Release.
//*****************************************************************************
void CUIMng::ReleaseTitleSceneUI()
{
	g_GameCensorship->SetVisible(false);

	SAFE_DELETE_ARRAY(m_asprTitle);
	SAFE_DELETE(m_pgbLoding);

	m_nScene = UIM_SCENE_NONE;
}

//*****************************************************************************
// 함수 이름 : RenderTitleSceneUI()
// 함수 설명 : 타이틀 씬 렌더.
// 매개 변수 : hDC		: DC 핸들.
//			   dwNow	: 현재 로딩 된 데이터 값.
//			   dwTotal	: 최대 로딩 데이터 값.
//*****************************************************************************
void CUIMng::RenderTitleSceneUI(HDC hDC, DWORD dwNow, DWORD dwTotal)
{
	::BeginOpengl();
/*
#if SELECTED_LANGUAGE == LANGUAGE_JAPANESE || SELECTED_LANGUAGE == LANGUAGE_CHINESE
	::glClearColor( 1.f, 1.f, 1.f, 1.f);
#endif
*/
#ifdef USE_SHADOWVOLUME
	::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
#else
	::glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
#endif
/*
#if SELECTED_LANGUAGE == LANGUAGE_JAPANESE || SELECTED_LANGUAGE == LANGUAGE_CHINESE
	::glClearColor( 0.f, 0.f, 0.f, 1.f);
#endif
*/
	::BeginBitmap();

	for (int i = 0; i < UIM_TS_MAX; ++i)
	{
#ifdef PBG_ADD_NEWLOGO_IMAGECHANGE
		if(i==2)
			continue;
#endif //PBG_ADD_NEWLOGO_IMAGECHANGE
		m_asprTitle[i].Render();
	}
#ifndef KJH_ADD_EVENT_LOADING_SCENE				// #ifndef
#ifdef PBG_ADD_NEWLOGO_IMAGECHANGE
	m_asprTitle[UIM_TS_MU].Render();
#endif //PBG_ADD_NEWLOGO_IMAGECHANGE
#endif // KJH_ADD_EVENT_LOADING_SCENE

	m_pgbLoding->SetValue(dwNow, dwTotal);
	m_pgbLoding->Render();

#if SELECTED_LANGUAGE == LANGUAGE_KOREAN	
	g_GameCensorship->Render();
#endif //SELECTED_LANGUAGE == LANGUAGE_KOREAN
	
#ifdef FOR_WORK
	g_pRenderText->SetFont(g_hFontBig);

#ifdef FOR_HACKING
	g_pRenderText->RenderText(0,0,"멀티 로긴 버젼");
#else // FOR_HACKING
	g_pRenderText->RenderText(0,0,"사내 작업용 버젼");
#endif // FOR_HACKING
#endif // FOR_WORK

	::EndBitmap();
	::EndOpengl();
	::glFlush();
	::SwapBuffers(hDC);
}

//*****************************************************************************
// 함수 이름 : Create()
// 함수 설명 : 공통 오브젝트 생성.
//*****************************************************************************
void CUIMng::Create()
{
	m_bCursorOnUI = false;
	m_bBlockCharMove = false;
	m_bWinActive = false;
	m_nScene = UIM_SCENE_NONE;

	return;
}

//*****************************************************************************
// 함수 이름 : RemoveWinList()
// 함수 설명 : 윈도우들 Release와 윈도우 리스트 비움.
//*****************************************************************************
void CUIMng::RemoveWinList()
{
	CWin* pWin;
	while (m_WinList.GetCount())
	{
		pWin = (CWin*)m_WinList.RemoveHead();
		pWin->Release();
	}
}

//*****************************************************************************
// 함수 이름 : Release()
// 함수 설명 : 윈도우 매니저 Release.
//*****************************************************************************
void CUIMng::Release()
{
	RemoveWinList();

	// 캐릭터 씬에서 사용되는 캐릭터 정보 말풍선을 지움.
	m_CharInfoBalloonMng.Release();

//	m_ToolTip.Release();

	m_nScene = UIM_SCENE_NONE;
}

//*****************************************************************************
// 함수 이름 : CreateLoginScene()
// 함수 설명 : 로그인 씬에서의 윈도우 생성.
//*****************************************************************************
void CUIMng::CreateLoginScene()
{
	g_GameCensorship->SetVisible(false);
	
	RemoveWinList();

	// 캐릭터 씬에서 사용되는 캐릭터 정보 말풍선을 지움.
	m_CharInfoBalloonMng.Release();

	CInput& rInput = CInput::Instance();
	
	// 메시지 창.
	m_MsgWin.Create();
	m_WinList.AddHead(&m_MsgWin);
	m_MsgWin.SetPosition((rInput.GetScreenWidth() - 352) / 2,
		(rInput.GetScreenHeight() - 113) / 2);

	// 시스템 메뉴 창.
	m_SysMenuWin.Create();
	m_WinList.AddHead(&m_SysMenuWin);

	// 옵션창.
	m_OptionWin.Create();
	m_WinList.AddHead(&m_OptionWin);

	// 메인 윈도우.
	m_LoginMainWin.Create();
	m_WinList.AddHead(&m_LoginMainWin);
	// 800*600 모드에서 3D화면 밑바닥 Y위치가 567임.
	int nBaseY = int(567.0f / 600.0f * (float)rInput.GetScreenHeight());
	m_LoginMainWin.SetPosition(30,  nBaseY - m_LoginMainWin.GetHeight() - 11);

	// 서버 선택 창.
	m_ServerSelWin.Create();
	m_WinList.AddHead(&m_ServerSelWin);
	m_ServerSelWin.SetPosition(
		(rInput.GetScreenWidth() - m_ServerSelWin.GetWidth()) / 2,
		(rInput.GetScreenHeight() - m_ServerSelWin.GetHeight()) / 2);

	// 로그인 창.
	m_LoginWin.Create();
	m_WinList.AddHead(&m_LoginWin);
	m_LoginWin.SetPosition(
		(rInput.GetScreenWidth() - m_LoginWin.GetWidth()) / 2,
		(rInput.GetScreenHeight() - m_LoginWin.GetHeight()) * 2 / 3);

	// 크레딧 창.
	m_CreditWin.Create();
	m_WinList.AddHead(&m_CreditWin);

#ifdef LJH_ADD_LOCALIZED_BTNS
	// 예, 아니오(localized) 버튼 메시지 창.
	m_LocalizedYESNOMsgWin.Create(MWT_BTN_LOCALIZED_YESNO);
	m_WinList.AddHead(&m_LocalizedYESNOMsgWin);
	m_LocalizedYESNOMsgWin.SetPosition((rInput.GetScreenWidth() - 352) / 2,
		(rInput.GetScreenHeight() - 113) / 2);

	// 예, 아니오(localized) 버튼 메시지 창.
	m_LocalizedConfirmCancelMsgWin.Create(MWT_BTN_LOCALIZED_CONFIRM_CANCEL);
	m_WinList.AddHead(&m_LocalizedConfirmCancelMsgWin);
	m_LocalizedConfirmCancelMsgWin.SetPosition((rInput.GetScreenWidth() - 352) / 2,
		(rInput.GetScreenHeight() - 113) / 2);

#endif //LJH_ADD_LOCALIZED_BTNS

#ifdef LJH_ADD_ONETIME_PASSWORD
	// 원타임패스워드 창.
	m_OneTimePasswordWin.Create();
	m_WinList.AddHead(&m_OneTimePasswordWin);
	m_OneTimePasswordWin.SetPosition(
		(rInput.GetScreenWidth() - m_OneTimePasswordWin.GetWidth()) / 2,
		(rInput.GetScreenHeight() - m_OneTimePasswordWin.GetHeight()) * 2 / 3);
#endif //LJH_ADD_ONETIME_PASSWORD

	
	
	m_bSysMenuWinShow = false;
	m_nScene = UIM_SCENE_LOGIN;
}

//*****************************************************************************
// 함수 이름 : CreateCharacterScene()
// 함수 설명 : 캐릭터 선택 씬에서의 윈도우 생성.
//*****************************************************************************
void CUIMng::CreateCharacterScene()
{
#ifdef KJH_ADD_SERVER_LIST_SYSTEM
	g_GameCensorship->SetState( g_ServerListManager->GetCensorshipIndex() );
#else // KJH_ADD_SERVER_LIST_SYSTEM
	g_GameCensorship->SetGameState();
#endif // KJH_ADD_SERVER_LIST_SYSTEM

	RemoveWinList();

	m_CharInfoBalloonMng.Create();

	CInput& rInput = CInput::Instance();

	// 메시지 창.
	m_MsgWin.Create();
	m_WinList.AddHead(&m_MsgWin);
	m_MsgWin.SetPosition((rInput.GetScreenWidth() - 352) / 2,
		(rInput.GetScreenHeight() - 113) / 2);

	// 서버 메시지 창.
	m_ServerMsgWin.Create();
	m_WinList.AddHead(&m_ServerMsgWin);
	// 800*600 모드에서 3D화면 Y위치가 31임.
	int nBaseY = int(31.0f / 600.0f * (float)rInput.GetScreenHeight());
	m_ServerMsgWin.SetPosition(10, nBaseY + 10);

	// 시스템 메뉴 창.
	m_SysMenuWin.Create();
	m_WinList.AddHead(&m_SysMenuWin);

	m_OptionWin.Create();
	m_WinList.AddHead(&m_OptionWin);

	// 메인 윈도우.
	m_CharSelMainWin.Create();
	m_WinList.AddHead(&m_CharSelMainWin);
	// 800*600 모드에서 3D화면 밑바닥 Y위치가 567임.
	nBaseY = int(567.0f / 600.0f * (float)rInput.GetScreenHeight());
	m_CharSelMainWin.SetPosition(22,  nBaseY - m_CharSelMainWin.GetHeight() - 11);

	// 캐릭터 만들기 창.
	m_CharMakeWin.Create();
	m_WinList.AddHead(&m_CharMakeWin);
#ifdef PJH_CHARACTER_RENAME
	m_CharMakeWin.Set_State();
#endif //PJH_CHARACTER_RENAME

	m_CharMakeWin.SetPosition((rInput.GetScreenWidth() - 454) / 2,
		(rInput.GetScreenHeight() - 406) / 2);

	m_CharSelMainWin.UpdateDisplay();
	m_CharInfoBalloonMng.UpdateDisplay();

	ShowWin(&m_CharSelMainWin);

	m_bSysMenuWinShow = false;
	m_nScene = UIM_SCENE_CHARACTER;
}

//*****************************************************************************
// 함수 이름 : CreateCharacterScene()
// 함수 설명 : 메인 씬에서의 윈도우 생성.
//*****************************************************************************
void CUIMng::CreateMainScene()
{
	RemoveWinList();

	// 캐릭터 씬에서 사용되는 캐릭터 정보 말풍선을 지움.
	m_CharInfoBalloonMng.Release();

	m_nScene = UIM_SCENE_MAIN;
}

//*****************************************************************************
// 함수 이름 : SetActiveWin()
// 함수 설명 : 윈도우의 활성화. 이전에 Active였던 window 주소 리턴.
//			   (list의 head를 비활성화 후, 활성화시킬 window를 활성화 시키고
//			  list의 head로 옮김.)
// 매개 변수 : pWin	: 활성화시킬 윈도우의 주소. (list에 등록된 윈도우)
//*****************************************************************************
CWin* CUIMng::SetActiveWin(CWin* pWin)
{
// Active였던 window 처리 부분.
	// Active였던 window가 있다면 그 window는 list의 head가 가리키는 window이므
	//로 일단 list의 head 값을 얻음.
	CWin* pBeforeActWin = (CWin*)m_WinList.GetHead();
	// list의 head가 없으면(list가 비어 있다면) NULL 리턴.
	if (pBeforeActWin == NULL)
		return NULL;

	if (pBeforeActWin->IsActive())	// list의 head인 window가 Active인가?
		pBeforeActWin->Active(FALSE);	// 비활성화.
	else							// list의 head인 window가 Active가 아니라면.
		pBeforeActWin = NULL;		// 이전에 Active였던 window는 없음.

// Active시킬 Window 처리 부분.
	// Active시킬 Window가 보이는가? (Update()에서 검사해주지만 이 함수가 단독
	//으로 호출될 경우를 위해 다시 검사함.)
	if (pWin->IsShow())
	{
	// Active시킬 Window는 list의 head로 이동.
		// 우선 list에서 찾아서 삭제.
		if (!m_WinList.RemoveAt(m_WinList.Find(pWin)))
			return NULL;			// 실패시 NULL 리턴.

		m_bWinActive = true;
		m_WinList.AddHead(pWin);	// list의 head에 추가.
	}

	return pBeforeActWin;			// 이전에 Active였던 window 포인터 리턴.
}
/**
//*****************************************************************************
// 함수 이름 : ShowWin()\n
// 함수 설명 : 윈도우를 보이게하고 활성화 시킴.\n
// 매개 변수 : pWin	: 보이게할 윈도우의 주소. (list에 등록된 윈도우)
//*****************************************************************************/
void CUIMng::ShowWin(CWin* pWin)
{
	pWin->Show(TRUE);
	SetActiveWin(pWin);
}
/**
//*****************************************************************************
// 함수 이름 : HideWin()\n
// 함수 설명 : 윈도우를 감춤.\n
// 매개 변수 : pWin	: 감출 윈도우의 주소. (list에 등록된 윈도우)
//*****************************************************************************/
void CUIMng::HideWin(CWin* pWin)
{
	// 그 윈도우를 리스트에서 삭제.
	if (!m_WinList.RemoveAt(m_WinList.Find(pWin)))
		return;					// 실패시 리턴.

	pWin->Show(FALSE);		// 보이지 않게 한다.
	pWin->Active(FALSE);	// 비활성화.
	m_WinList.AddTail(pWin);	// list의 tail에 추가.

	// list의 head가 보인다면 활성화.
	pWin = (CWin*)m_WinList.GetHead();
	if (pWin->IsShow())
		m_bWinActive = true;
}

//*****************************************************************************
// 함수 이름 : CheckDockWin()\n
// 함수 설명 : 윈도우 도킹 처리.
//*****************************************************************************/
void CUIMng::CheckDockWin()
{
	NODE* position = m_WinList.GetHeadPosition();
	if (NULL == position)
		return;

	// list의 head의 포인터를 얻고 position은 다음으로.
	// pMovWin: 이동 중인 윈도우.
	CWin* pMovWin = (CWin*)m_WinList.GetNext(position);

	if (pMovWin->GetState() != WS_MOVE)	// 이동 중이 아닌가?
		return;

	pMovWin->SetDocking(false);

	// 이동 중인 윈도우의 영역.
	RECT rcMovWin = { pMovWin->GetTempXPos(), pMovWin->GetTempYPos(),
		pMovWin->GetTempXPos() + pMovWin->GetWidth(),
		pMovWin->GetTempYPos() + pMovWin->GetHeight() };
	
	// 이동 중인 윈도우의 모서리 도킹 영역. 순서대로 윈도우 모서리를 중심으
	//로 한 좌상, 우상, 좌하, 우하 사각형 영역.
	RECT rcDock[4] =
	{
		{ rcMovWin.left - DOCK_EXTENT, rcMovWin.top - DOCK_EXTENT,
			rcMovWin.left + DOCK_EXTENT, rcMovWin.top + DOCK_EXTENT },
		{ rcMovWin.right - DOCK_EXTENT, rcMovWin.top - DOCK_EXTENT,
			rcMovWin.right + DOCK_EXTENT, rcMovWin.top + DOCK_EXTENT },
		{ rcMovWin.left - DOCK_EXTENT, rcMovWin.bottom - DOCK_EXTENT,
			rcMovWin.left + DOCK_EXTENT, rcMovWin.bottom + DOCK_EXTENT },
		{ rcMovWin.right - DOCK_EXTENT, rcMovWin.bottom - DOCK_EXTENT,
			rcMovWin.right + DOCK_EXTENT, rcMovWin.bottom + DOCK_EXTENT }
	};

// 화면 가장자리 도킹 검사 및 이동.
	CInput& rInput = CInput::Instance();
	// 화면의 모서리 4곳.
	POINT pt[4] = { { 0, 0 }, { rInput.GetScreenWidth(), 0 },
		{ 0, rInput.GetScreenHeight() },
		{ rInput.GetScreenWidth(), rInput.GetScreenHeight() } };

	if (::PtInRect(&rcDock[0], pt[0]))		// 화면의 좌상 모서리.
	{
		pMovWin->SetPosition(pt[0].x, pt[0].y);
		pMovWin->SetDocking(true);
	}
	else if (::PtInRect(&rcDock[1], pt[1]))	// 화면의 우상 모서리.
	{
		pMovWin->SetPosition(pt[1].x - pMovWin->GetWidth(), pt[1].y);
		pMovWin->SetDocking(true);
	}
	else if (::PtInRect(&rcDock[2], pt[2]))	// 화면의 좌하 모서리.
	{
		pMovWin->SetPosition(pt[2].x, pt[2].y - pMovWin->GetHeight());
		pMovWin->SetDocking(true);
	}
	else if (::PtInRect(&rcDock[3], pt[3]))	// 화면의 우하 모서리.
	{
		pMovWin->SetPosition(pt[3].x - pMovWin->GetWidth(),
			pt[3].y - pMovWin->GetHeight());
		pMovWin->SetDocking(true);
	}
	// 화면의 상단.
	else if (rcDock[0].top < 0 && rcDock[0].bottom > 0)
	{
		pMovWin->SetPosition(rcMovWin.left, 0);
		pMovWin->SetDocking(true);
	}
	// 화면의 하단.
	else if (rcDock[2].top < pt[2].y && rcDock[2].bottom > pt[2].y)
	{
		pMovWin->SetPosition(rcMovWin.left, pt[2].y - pMovWin->GetHeight());
		pMovWin->SetDocking(true);
	}
	// 화면의 우측.
	else if (rcDock[0].left < 0 && rcDock[0].right > 0)
	{
		pMovWin->SetPosition(0, rcMovWin.top);
		pMovWin->SetDocking(true);
	}
	// 화면의 좌측.
	else if (rcDock[1].left < pt[1].x && rcDock[1].right > pt[1].x)
	{
		pMovWin->SetPosition(pt[1].x - pMovWin->GetWidth(), rcMovWin.top);
		pMovWin->SetDocking(true);
	}

// 타 윈도우와의 도킹.
	BOOL bEdgeDocking = FALSE;
	int i, j, nXCoord, nYCoord;
	CWin* pWin;

	while (position)	// 2번째 윈도우부터 끝까지 체크.
	{
		// 체크할 윈도우 얻기.
		pWin = (CWin*)m_WinList.GetNext(position);
		if (!pWin->IsShow())	// 보이지 않으면 루프 탈출.
			continue;

		// 체크할 윈도우의 좌상, 우상, 좌하, 우하 지점.
		pt[0].x = pWin->GetXPos();
		pt[0].y = pWin->GetYPos();
		pt[1].x = pWin->GetXPos() + pWin->GetWidth();
		pt[1].y = pt[0].y;
		pt[2].x = pt[0].x;
		pt[2].y = pWin->GetYPos() + pWin->GetHeight();
		pt[3].x = pt[1].x;
		pt[3].y = pt[2].y;

		// 모서리 도킹 검사.
		for (i = 0; i < 4; i++)
		{
			for (j = 0; j < 4; j++)
			{
				if (i != j && ::PtInRect(&rcDock[i], pt[j]))
				{
					bEdgeDocking = TRUE;
					goto DOCKING;
				}
			}
		}

		// 각 변 도킹 검사 및 이동.
		// 위, 아랫변 검사.
		if (pt[0].x < rcDock[1].left && pt[1].x > rcDock[0].right)
		{
			nXCoord = rcMovWin.left;
			if (pt[2].y > rcDock[0].top && pt[2].y < rcDock[0].bottom)
			{
				if (SetDockWinPosition(pMovWin, nXCoord, pt[2].y))
					continue;
			}
			else if (pt[0].y > rcDock[2].top && pt[0].y < rcDock[2].bottom)
			{
				if (SetDockWinPosition(pMovWin,
					nXCoord, pt[0].y - pMovWin->GetHeight()))
					continue;
			}
		}
		// 좌,우변 검사.
		else if (pt[0].y < rcDock[2].top && pt[2].y > rcDock[0].bottom)
		{
			nYCoord = rcMovWin.top;
			if (pt[1].x > rcDock[0].left && pt[1].x < rcDock[0].right)
			{
				if (SetDockWinPosition(pMovWin, pt[1].x, nYCoord))
					continue;
			}
			else if (pt[0].x > rcDock[1].left && pt[0].x < rcDock[1].right)
			{
				if (SetDockWinPosition(pMovWin,
					pt[0].x - pMovWin->GetWidth(), nYCoord))
					continue;
			}
		}
	}	// while (position)문 끝.

DOCKING:	// 모서리 도킹 시 이동.
	if (bEdgeDocking)	//  모서리 도킹인가?
	{
		switch (j)		// j는 체크할 윈도우(pWin)의 모서리 지점.
		{
		case 0:
			switch (i)	// i는 도킹 시킬 윈도우(pMovWin)의 도킹 영역.
			{
			case 1:
				nXCoord = pWin->GetXPos() - pMovWin->GetWidth();
				nYCoord = pWin->GetYPos();
				break;
			case 2:
				nXCoord = pWin->GetXPos();
				nYCoord = pWin->GetYPos() - pMovWin->GetHeight();
				break;
			case 3:
				nXCoord = pWin->GetXPos() - pMovWin->GetWidth();
				nYCoord = pWin->GetYPos() - pMovWin->GetHeight();
			}
			break;
		
		case 1:
			switch (i)
			{
			case 0:
				nXCoord = pWin->GetXPos() + pWin->GetWidth();
				nYCoord = pWin->GetYPos();
				break;
			case 2:
				nXCoord = pWin->GetXPos() + pWin->GetWidth();
				nYCoord = pWin->GetYPos() - pMovWin->GetHeight();
				break;
			case 3:
				nXCoord = pWin->GetXPos() + pWin->GetWidth()
					- pMovWin->GetWidth();
				nYCoord = pWin->GetYPos() - pMovWin->GetHeight();
			}
			break;
		
		case 2:
			switch (i)
			{
			case 0:
				nXCoord = pWin->GetXPos();
				nYCoord = pWin->GetYPos() + pWin->GetHeight();
				break;
			case 1:
				nXCoord = pWin->GetXPos() - pMovWin->GetWidth();
				nYCoord = pWin->GetYPos() + pWin->GetHeight();
				break;
			case 3:
				nXCoord = pWin->GetXPos() - pMovWin->GetWidth();
				nYCoord = pWin->GetYPos() + pWin->GetHeight()
					- pMovWin->GetHeight();
			}
			break;
		
		case 3:
			switch (i)
			{
			case 0:
				nXCoord = pWin->GetXPos() + pWin->GetWidth();
				nYCoord = pWin->GetYPos() + pWin->GetHeight();
				break;
			case 1:
				nXCoord = pWin->GetXPos() + pWin->GetWidth()
					- pMovWin->GetWidth();
				nYCoord = pWin->GetYPos() + pWin->GetHeight();
				break;
			case 2:
				nXCoord = pWin->GetXPos() + pWin->GetWidth();
				nYCoord = pWin->GetYPos() + pWin->GetHeight()
					- pMovWin->GetHeight();
			} 
		}
		SetDockWinPosition(pMovWin, nXCoord, nYCoord);
	}	// if (bEdgeDocking)문 끝.
}

//*****************************************************************************
// 함수 이름 : SetDockWinPosition()
// 함수 설명 : 도킹 후 위도우 위치가 화면에서 보인다면 그 위치로 이동 후 TRUE
//			  리턴.(도킹시 윈도우가 화면 밖으로 나가는 것을 방지.)
// 매개 변수 : pMoveWin	: 이동 중인 윈도우 주소.
//			   nDockX	: 도킹 후 윈도우의 X 좌표.
//			   nDockY	: 도킹 후 윈도우의 Y 좌표.
//*****************************************************************************
bool CUIMng::SetDockWinPosition(CWin* pMoveWin, int nDockX, int nDockY)
{
	CInput& rInput = CInput::Instance();
	RECT rcDummy;
	RECT rcScreen = { 0, 0, rInput.GetScreenWidth(), rInput.GetScreenHeight() };
	RECT rcMoveWin = { nDockX, nDockY,
		nDockX + pMoveWin->GetWidth(), nDockY + pMoveWin->GetHeight() };

	if (::IntersectRect(&rcDummy, &rcScreen, &rcMoveWin))
	{
		pMoveWin->SetPosition(nDockX, nDockY);
		pMoveWin->SetDocking(true);
		return true;
	}

	return false;
}

//*****************************************************************************
// 함수 이름 : Update()
// 함수 설명 : 윈도우들의 자동 처리.(윈도우 클릭시 활성화, 윈도우 Update()실행,
//			  도킹, 마우스 커서 모양 바꾸기, 윈도우 닫았을 때의 처리, 윈도우 내
//			  버튼 처리, 윈도우 보여주기 버튼 처리.)
// 매개 변수 : dDeltaTick	: 이전 Update()호출 후부터 지금 Update()까지 시간.
//*****************************************************************************
void CUIMng::Update(double dDeltaTick)
{
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
	if(dDeltaTick < 0)		return;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
	// 생성되어 있지 않다면 처리하지 않음.
	if (UIM_SCENE_NONE == m_nScene || m_WinList.IsEmpty())
		return;

	if (m_bWinActive)
	{
		CWin* pWin = (CWin*)m_WinList.GetHead();
		if (pWin->IsShow())
		{
			pWin->Active(true);
			m_bWinActive = false;
		}
	}

	CInput& rInput = CInput::Instance();
	if (rInput.IsKeyDown(VK_ESCAPE))
	{
		m_bSysMenuWinShow = !m_bSysMenuWinShow;
	}
	
	CWin* pWin;
	NODE* position;

	m_bCursorOnUI = false;	// 윈도우 위에 커서가 있는지 여부 초기화.

// 1. 클릭 시 윈도우를 눌렀는지 여부에 따른 윈도우 활성화 처리.
	if (rInput.IsLBtnDn())	// 마우스 왼쪽 버튼을 눌렀는가?
	{
		bool bWinClick = false;
		position = m_WinList.GetHeadPosition();	// list의 Head부터 검사.
		while (position)	// position이 NULL일 때까지 반복.
		{
			// 이 position의 값을 얻고 이 position를 다음 position으로.
			pWin = (CWin*)m_WinList.GetNext(position);
			// 이 window가 보이는 상태에서 이 window를 클릭을 했는가?
			if (pWin->CursorInWin(WA_ALL))
			{
				SetActiveWin(pWin);		// 이 window는 Active.
				bWinClick = true;		// window를 클릭 했음.
				break;		// while 루프를 빠져나옴.
			}
		}

		if (!bWinClick)		// window를 클릭하지 않았는가?
		{
			// 우선 이전에 Active였던 window를 비활성화 시킴.
			// Active였던 window가 있었다면 list의 head이므로 무조건 비활성화.
			pWin = (CWin*)m_WinList.GetHead();
			pWin->Active(false);
		}
	}
	// 마우스 왼 버튼 업일 경우만 풀어줌.
	else if (rInput.IsLBtnUp())
	{
		m_bBlockCharMove = false;
	}
// 2. 윈도우 Update() 및 윈도우 내의 버튼 활성화 초기화.
	// 리스트 순서대로 배열에 저장 후 업데이트 및 버튼 활성화 초기화를 함.
	// 리스트 순서대로 업데이트할 경우 업데이트 도중 다른 창이 강제로 활성화 되
	//면 리스트 순서가 바뀌게 되어 치명적인 에러 발생.
	int nlist = m_WinList.GetCount();
	CWin** apTempWin = new (CWin*[nlist]);
 
	position = m_WinList.GetHeadPosition();
	for (int i = 0; i < nlist; ++i)
	{
		 apTempWin[i] = (CWin*)m_WinList.GetNext(position);
		 apTempWin[i]->ActiveBtns(false);
	}

	position = m_WinList.GetHeadPosition();
	while (position)	// position이 NULL일 때까지 반복.
	{
		// 이 position의 값을 얻고 이 position를 이전 position으로.
		pWin = (CWin*)m_WinList.GetNext(position);
		if (pWin->CursorInWin(WA_ALL))	// 커서가 이 윈도우 위에 있는가?
		{
			// 이 윈도우의 버튼들은 활성화.
			pWin->ActiveBtns(true);
			break;	// 커서가 윈도우 위에 있을 경우 다른 윈도우는 검사하지 않음.
		}
	}
 
#ifdef _VS2008PORTING
	for(int i = 0; i < nlist; ++i)
#else // _VS2008PORTING
	for(i = 0; i < nlist; ++i)
#endif // _VS2008PORTING
	{
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
		apTempWin[i]->Update();
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
		apTempWin[i]->Update(dDeltaTick);
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
	}
 
#ifdef KJH_FIX_ARRAY_DELETE
	SAFE_DELETE_ARRAY(apTempWin);
#else // KJH_FIX_ARRAY_DELETE
	delete apTempWin;
#endif // KJH_FIX_ARRAY_DELETE

//	CheckKey();
	CheckDockWin();		// 윈도우 도킹 체크.

// 3. 윈도우 업데이트 이후 처리.(커서 모양 바꾸기, 윈도우 내 버튼 처리.)
	position = m_WinList.GetHeadPosition();
	while (position)	// position이 NULL일 때까지 반복.
	{
		// 이 position의 값을 얻고 이 position를 이전 position으로.
		pWin = (CWin*)m_WinList.GetNext(position);

		switch (pWin->GetState())	// 이 윈도우의 상태는?
		{
		case WS_ETC:			// 기타 작동 중.
			m_bCursorOnUI = true;	// while (position)루프 탈출.
			break;

		case WS_MOVE:			// 이동 중.
//			eCursorActType = CURSOR_M;
			m_bCursorOnUI = true;	// while (position)루프 탈출.
			break;

		case WS_EXTEND_UP:		// 상단 크기 조절 중.
//			eCursorActType = CURSOR_V;
			m_bCursorOnUI = true;	// while (position)루프 탈출.
			break;

		case WS_EXTEND_DN:		// 하단 크기 조절 중.
//			eCursorActType = CURSOR_V;
			m_bCursorOnUI = true;	// while (position)루프 탈출.
			break;
		}

		if (m_bCursorOnUI)
			break;

		if (pWin->CursorInWin(WA_ALL))	// 커서가 이 윈도우 위에 있는가?
		{
			m_bCursorOnUI = true;
			break;	// 커서가 윈도우 위에 있을 경우 다른 윈도우는 검사하지 않음.
		}	// if (pWin->CursorInWin()) 끝.
	}	// while (position)	끝.
}

//*****************************************************************************
// 함수 이름 : Render()
// 함수 설명 : 윈도우 보여주기 버튼 및 윈도우 랜더링.
//*****************************************************************************
void CUIMng::Render()
{
	if (UIM_SCENE_NONE == m_nScene)	// 생성되어 있지 않다면 처리하지 않음.
		return;

	m_CharInfoBalloonMng.Render();

	CWin* pWin;
	// 렌더 순서는 list 순서의 역순. 따라서 tail 부터 렌더.
	NODE* position = m_WinList.GetTailPosition();
	while (position)	// position이 NULL일 때까지 반복.
	{
		// 이 position의 값을 얻고 이 position를 이전 position으로.
		pWin = (CWin*)m_WinList.GetPrev(position);
		pWin->Render();
	}
}

//*****************************************************************************
// 함수 이름 : PopUpMsgWin()
// 함수 설명 : 메시지 윈도우를 보여줌.
// 매개 변수 : nMsgCode	: 메시지 코드.
//*****************************************************************************
void CUIMng::PopUpMsgWin(int nMsgCode, char* pszMsg)
{
	if (UIM_SCENE_NONE == m_nScene || UIM_SCENE_TITLE == m_nScene
		|| UIM_SCENE_LOADING == m_nScene)
		return;

	if (UIM_SCENE_MAIN == m_nScene)	return;	// 현재 메인씬에서는 적용되지 않음.

#ifdef LJH_ADD_LOCALIZED_BTNS
	if (nMsgCode == MESSAGE_OTP_NOT_REGISTERED)	{
		m_LocalizedYESNOMsgWin.PopUp(nMsgCode, pszMsg);
	}
	else if (nMsgCode == MESSAGE_OTP_INCORRECT || nMsgCode == MESSAGE_OTP_INVALID_INFO) {
		m_LocalizedConfirmCancelMsgWin.PopUp(nMsgCode, pszMsg);
	}
	else {
		m_MsgWin.PopUp(nMsgCode, pszMsg);
	}
#else  //LJH_ADD_LOCALIZED_BTNS
	m_MsgWin.PopUp(nMsgCode, pszMsg);
#endif //LJH_ADD_LOCALIZED_BTNS
	
	
}

//*****************************************************************************
// 함수 이름 : PopUpMsgWin()
// 함수 설명 : 메시지 윈도우를 보여줌.
// 매개 변수 : nMsgCode	: 메시지 코드.
//*****************************************************************************
void CUIMng::AddServerMsg(char* pszMsg)
{
	if (UIM_SCENE_CHARACTER != m_nScene)	// 캐릭터 씬에서만 적용.
		return;

	m_ServerMsgWin.AddMsg(pszMsg);
}
