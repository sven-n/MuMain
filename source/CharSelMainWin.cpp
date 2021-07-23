//*****************************************************************************
// File: CharSelMainWin.cpp
//
// Desc: implementation of the CCharSelMainWin class.
//
// producer: Ahn Sang-Kyu
//*****************************************************************************

#include "stdafx.h"
#include "CharSelMainWin.h"

#include "Input.h"
#include "UIMng.h"

// 텍스트 랜더를 위한 #include. ㅜㅜ
#include "ZzzBMD.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzInterface.h"

#include "UIGuildInfo.h"
#include "ZzzOpenData.h"
#include "ZzzOpenglUtil.h"

#ifdef KJH_ADD_SERVER_LIST_SYSTEM
#include "ServerListManager.h"
#endif // KJH_ADD_SERVER_LIST_SYSTEM
#ifdef PBG_ADD_CHARACTERSLOT
#include "Slotlimit.h"
#endif //PBG_ADD_CHARACTERSLOT

extern float g_fScreenRate_x;
extern float g_fScreenRate_y;
#ifndef KJH_ADD_SERVER_LIST_SYSTEM			// #ifndef
extern int ServerSelectHi;
#endif // KJH_ADD_SERVER_LIST_SYSTEM

#ifdef KJH_MOD_NATION_LANGUAGE_REDEFINE
#ifdef TEENAGER_REGULATION
#define	CSMW_WARNING_TEXT_MAX	3	// 경고 문구 개수.
#endif // TEENAGER_REGULATION
#else // KJH_MOD_NATION_LANGUAGE_REDEFINE
#if (SELECTED_LANGUAGE == LANGUAGE_KOREAN) && defined TEENAGER_REGULATION
#define	CSMW_WARNING_TEXT_MAX	3	// 경고 문구 개수.
#endif	// (SELECTED_LANGUAGE == LANGUAGE_KOREAN) && defined TEENAGER_REGULATION
#endif // KJH_MOD_NATION_LANGUAGE_REDEFINE

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCharSelMainWin::CCharSelMainWin()
{

}

CCharSelMainWin::~CCharSelMainWin()
{

}

//*****************************************************************************
// 함수 이름 : Create()
// 함수 설명 : 캐릭터 선택씬 메인 창 생성.
//*****************************************************************************
// 버튼은 등록할 것.
void CCharSelMainWin::Create()
{
// 스프라이트 생성.
	m_asprBack[CSMW_SPR_DECO].Create(189, 103, BITMAP_LOG_IN+2);
	m_asprBack[CSMW_SPR_INFO].Create(
	CInput::Instance().GetScreenWidth() - 266, 21);
	m_asprBack[CSMW_SPR_INFO].SetColor(0, 0, 0);
	m_asprBack[CSMW_SPR_INFO].SetAlpha(143);

// 버튼 생성.
	m_aBtn[CSMW_BTN_CREATE].Create(54, 30, BITMAP_LOG_IN+3, 4, 2, 1, 3);
	m_aBtn[CSMW_BTN_MENU].Create(54, 30, BITMAP_LOG_IN+4, 3, 2, 1);
	m_aBtn[CSMW_BTN_CONNECT].Create(54, 30, BITMAP_LOG_IN+5, 4, 2, 1, 3);
	m_aBtn[CSMW_BTN_DELETE].Create(54, 30, BITMAP_LOG_IN+6, 4, 2, 1, 3);

// 기초 윈도우 생성.
	CWin::Create(
		m_aBtn[0].GetWidth() * CSMW_BTN_MAX + m_asprBack[CSMW_SPR_INFO].GetWidth() + 6,
		m_aBtn[0].GetHeight(), -2);

// 버튼 등록.(기초 윈도우 생성되어야 버튼 등록 가능)
	for (int i = 0; i < CSMW_BTN_MAX; ++i)
		CWin::RegisterButton(&m_aBtn[i]);

// 좌측 서비스 정보 문자열은 이 윈도우가 생성되기 전에 채워지므로 초기화하지 않음.
/*	m_bInfoSet = false;
	::memset(m_szInfo, 0, sizeof(char) * 50);
*/

// 캐릭터 리스트 받은 후 계정 블럭 아이템(?)인지 검사.
	m_bAccountBlockItem = false;
#ifdef _VS2008PORTING
	for (int i = 0; i < 5; ++i)
#else // _VS2008PORTING
	for (i = 0; i < 5; ++i)
#endif // _VS2008PORTING
	{
		if (CharactersClient[i].Object.Live)
		{
			if (CharactersClient[i].CtlCode & CTLCODE_10ACCOUNT_BLOCKITEM)
			{
				m_bAccountBlockItem = true;
				break;
			}
		}
	}

#ifdef KJH_MOD_NATION_LANGUAGE_REDEFINE
#ifdef TEENAGER_REGULATION
	// 경고 문구를 CSMW_WARNING_TEXT_MAX개 중에 하나를 랜덤으로 선택함.
	short anWarningText[CSMW_WARNING_TEXT_MAX] = { 1710, 1739, 1740 };
	m_nWarningText = anWarningText[::rand() % CSMW_WARNING_TEXT_MAX];
#endif // TEENAGER_REGULATION
#else // KJH_MOD_NATION_LANGUAGE_REDEFINE
#if (SELECTED_LANGUAGE == LANGUAGE_KOREAN) && defined TEENAGER_REGULATION
	// 경고 문구를 CSMW_WARNING_TEXT_MAX개 중에 하나를 랜덤으로 선택함.
	short anWarningText[CSMW_WARNING_TEXT_MAX] = { 1710, 1739, 1740 };
	m_nWarningText = anWarningText[::rand() % CSMW_WARNING_TEXT_MAX];
#endif	// (SELECTED_LANGUAGE == LANGUAGE_KOREAN) && defined TEENAGER_REGULATION
#endif // KJH_MOD_NATION_LANGUAGE_REDEFINE
}

//*****************************************************************************
// 함수 이름 : PreRelease()
// 함수 설명 : 모든 컨트롤 릴리즈.(버튼은 자동 삭제)
//*****************************************************************************
// 등록된 버튼과 기초 윈도우를 제외한 나머지 Release() 및 메모리 해제.
void CCharSelMainWin::PreRelease()
{
	for (int i = 0; i < CSMW_SPR_MAX; ++i)
		m_asprBack[i].Release();
}

//*****************************************************************************
// 함수 이름 : SetPosition()
// 함수 설명 : 창 위치 지정.
// 매개 변수 : nXCoord	: 스크린 X좌표.
//			   nYCoord	: 스크린 Y좌표.
//*****************************************************************************
void CCharSelMainWin::SetPosition(int nXCoord, int nYCoord)
{
	CWin::SetPosition(nXCoord, nYCoord);

	int nBtnWidth = m_aBtn[0].GetWidth();
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING
	int nBtnHeight = m_aBtn[0].GetHeight();
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING

// 좌측 배치.
	m_aBtn[CSMW_BTN_CREATE].SetPosition(nXCoord, nYCoord);
	m_aBtn[CSMW_BTN_MENU].SetPosition(nXCoord + nBtnWidth + 1, nYCoord);
	m_asprBack[CSMW_SPR_INFO].SetPosition(
		m_aBtn[CSMW_BTN_MENU].GetXPos() + nBtnWidth + 2, nYCoord + 5);

// 우측 배치.
	int nWinRPosX = nXCoord + CWin::GetWidth();	// 윈도우 오른쪽 끝 X위치.
	m_asprBack[CSMW_SPR_DECO].SetPosition(
		nWinRPosX - (m_asprBack[CSMW_SPR_DECO].GetWidth() - 22), nYCoord - 59);
	m_aBtn[CSMW_BTN_DELETE].SetPosition(nWinRPosX - nBtnWidth, nYCoord);
	m_aBtn[CSMW_BTN_CONNECT].SetPosition(
		nWinRPosX - (nBtnWidth * 2 + 1), nYCoord);
}

//*****************************************************************************
// 함수 이름 : Show()
// 함수 설명 : 창을 보여 주거나 안보이게함.
// 매개 변수 : bShow	: true이면 보여줌.
//*****************************************************************************
void CCharSelMainWin::Show(bool bShow)
{
	CWin::Show(bShow);

	int i;
	for (i = 0; i < CSMW_SPR_MAX; ++i)
		m_asprBack[i].Show(bShow);
	for (i = 0; i < CSMW_BTN_MAX; ++i)
		m_aBtn[i].Show(bShow);
}

//*****************************************************************************
// 함수 이름 : CursorInWin()
// 함수 설명 : 윈도우 영역 안에 마우스 커서가 위치하는가?
// 매개 변수 : eArea	: 검사할 영역.(win.h의 #define 참조)
//*****************************************************************************
// 등록된 버튼 영역은 설정할 필요 없음.
// 단, 다른 컨트롤이나 영역을 버튼과 같은 효과로 처리하려면 기초 클래스의
//CursorInButtonlike() 함수를 재정의만 하면 된다.
bool CCharSelMainWin::CursorInWin(int nArea)
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
// 함수 이름 : UpdateDisplay()
// 함수 설명 : 현재의 정보를 창에 반영함.
//*****************************************************************************
void CCharSelMainWin::UpdateDisplay()
{
	// 캐릭터가 5명 미만이면 m_aBtn[CSMW_BTN_CREATE] 활성화.
	m_aBtn[CSMW_BTN_CREATE].SetEnable(false);
	int i=0;
#ifdef PBG_ADD_CHARACTERSLOT
	// 캐릭터 생성 가능수 만큼 캐릭이 생성 되어 있다면 비 활성화 하자
	if(BLUE_MU::IsBlueMuServer())
	{
		for(i = 0; i < g_SlotLimit->GetCharacterCnt(); ++i)
		{
			if (!CharactersClient[i].Object.Live)
			{
				m_aBtn[CSMW_BTN_CREATE].SetEnable(true);
				break;
			}
		}
	}
	else
#endif //PBG_ADD_CHARACTERSLOT
	for (i = 0; i < 5; ++i)
	{
		if (!CharactersClient[i].Object.Live)
		{
			m_aBtn[CSMW_BTN_CREATE].SetEnable(true);
			break;
		}
	}
	// 캐릭터를 선택 했으면 m_aBtn[CSMW_BTN_CONNECT], m_aBtn[CSMW_BTN_DELETE]활성화.
	if (SelectedHero > -1)
	{
		m_aBtn[CSMW_BTN_CONNECT].SetEnable(true);
		m_aBtn[CSMW_BTN_DELETE].SetEnable(true);
	}
	else
	{
		m_aBtn[CSMW_BTN_CONNECT].SetEnable(false);
		m_aBtn[CSMW_BTN_DELETE].SetEnable(false);
	}

	// 캐릭터가 한명도 생성이 되어 있지 않으면 캐릭성성창이 떠있게 수정
	bool bNobodyCharacter = true;
#ifdef _VS2008PORTING
	for (int i = 0; i < 5; ++i)
#else // _VS2008PORTING
	for (i = 0; i < 5; ++i)
#endif // _VS2008PORTING
	{
		if (CharactersClient[i].Object.Live == true)
		{
			bNobodyCharacter = false;
			break;
		}
	}
	
	if (bNobodyCharacter == true)
	{
		CUIMng& rUIMng = CUIMng::Instance();
		rUIMng.ShowWin(&rUIMng.m_CharMakeWin);
#ifdef PJH_CHARACTER_RENAME
		rUIMng.m_CharMakeWin.Set_State();
#endif //PJH_CHARACTER_RENAME
	}
}

//*****************************************************************************
// 함수 이름 : UpdateWhileActive()
// 함수 설명 : 액티브일 때의 업데이트.
// 매개 변수 : dDeltaTick	: 이전 Update()호출 후부터 지금 Update()까지 시간.
//*****************************************************************************
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
void CCharSelMainWin::UpdateWhileActive()
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
void CCharSelMainWin::UpdateWhileActive(double dDeltaTick)
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
{
	//PJH_DEBUG_SERVER_ADD
	if (m_aBtn[CSMW_BTN_CONNECT].IsClick())
		::StartGame();
	else if (m_aBtn[CSMW_BTN_MENU].IsClick())
	{
		CUIMng& rUIMng = CUIMng::Instance();
		rUIMng.ShowWin(&rUIMng.m_SysMenuWin);
		rUIMng.SetSysMenuWinShow(true);
	}
	else if (m_aBtn[CSMW_BTN_CREATE].IsClick())
	{
		CUIMng& rUIMng = CUIMng::Instance();
		rUIMng.ShowWin(&rUIMng.m_CharMakeWin);
#ifdef PJH_CHARACTER_RENAME
		rUIMng.m_CharMakeWin.Set_State();
#endif //PJH_CHARACTER_RENAME

	}
	else if (m_aBtn[CSMW_BTN_DELETE].IsClick())
		DeleteCharacter();
}

//*****************************************************************************
// 함수 이름 : RenderControls()
// 함수 설명 : 각종 컨트롤 렌더.
//*****************************************************************************
// CWin::RenderButtons()를 사용해서 버튼 랜더 순서를 정할 수 있다.
// 윈도우 배경은 설정할 필요 없다.
void CCharSelMainWin::RenderControls()
{
	for (int i = 0; i < CSMW_SPR_MAX; ++i)
		m_asprBack[i].Render();

	::EnableAlphaTest();
	::glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	
	g_pRenderText->SetFont(g_hFixFont);
	g_pRenderText->SetTextColor(CLRDW_WHITE);
	g_pRenderText->SetBgColor(0);

#ifdef KJH_MOD_NATION_LANGUAGE_REDEFINE
#ifdef _LANGUAGE_KOR
	// 경고 문구 텍스트.
	g_pRenderText->SetTextColor(CLRDW_BR_ORANGE);
	g_pRenderText->RenderText(
		int(m_asprBack[CSMW_SPR_INFO].GetXPos() / g_fScreenRate_x),
		int((m_asprBack[CSMW_SPR_INFO].GetYPos() + 7) / g_fScreenRate_y),
		GlobalText[m_nWarningText],
		int(m_asprBack[CSMW_SPR_INFO].GetWidth() / g_fScreenRate_x), 0,
		RT3_SORT_CENTER);
#endif // _LANGUAGE_KOR
#else // KJH_MOD_NATION_LANGUAGE_REDEFINE
#if SELECTED_LANGUAGE == LANGUAGE_KOREAN

	// 경고 문구 텍스트.
	g_pRenderText->SetTextColor(CLRDW_BR_ORANGE);
	g_pRenderText->RenderText(
		int(m_asprBack[CSMW_SPR_INFO].GetXPos() / g_fScreenRate_x),
		int((m_asprBack[CSMW_SPR_INFO].GetYPos() + 7) / g_fScreenRate_y),
		GlobalText[m_nWarningText],
		int(m_asprBack[CSMW_SPR_INFO].GetWidth() / g_fScreenRate_x), 0,
		RT3_SORT_CENTER);

#endif	// SELECTED_LANGUAGE == LANGUAGE_KOREAN
#endif // KJH_MOD_NATION_LANGUAGE_REDEFINE

	if (m_bAccountBlockItem)
	{
		g_pRenderText->SetTextColor(0, 0, 0, 255);
		g_pRenderText->SetBgColor(255, 255, 0, 128);
		g_pRenderText->RenderText(320, 330, GlobalText[436], 0, 0,
			RT3_WRITE_CENTER);
		g_pRenderText->RenderText(320, 348, GlobalText[437], 0, 0,
			RT3_WRITE_CENTER);
	}

	CWin::RenderButtons();
}

//*****************************************************************************
// 함수 이름 : DeleteCharacter()
// 함수 설명 : 삭제 불가능 캐릭터 검사 후 삭제할 것인가 하는 메시지 박스 출력.
//*****************************************************************************
void CCharSelMainWin::DeleteCharacter()
{
    // 길드에 가입된 캐릭터 삭제 불가.
    if (CharactersClient[SelectedHero].GuildStatus != G_NONE)
		CUIMng::Instance().PopUpMsgWin(MESSAGE_DELETE_CHARACTER_GUILDWARNING);
#if !defined FOR_WORK && !defined EXP_SERVER_PATCH	// 사내버전과 체험서버에서 삭제 가능하게 고침.
	// 본섭에서만 레벨 CHAR_DEL_LIMIT_LV 이상 삭제 불가.
#ifdef KJH_ADD_SERVER_LIST_SYSTEM
	else if((g_ServerListManager->IsTestServer() == false)
#else // KJH_ADD_SERVER_LIST_SYSTEM
	else if (!::IsTestServer()
#endif // KJH_ADD_SERVER_LIST_SYSTEM
		&& CharactersClient[SelectedHero].Level >= CHAR_DEL_LIMIT_LV)
		CUIMng::Instance().PopUpMsgWin(MESSAGE_DELETE_CHARACTER_WARNING);
#endif	// FOR_WORK
	// 블럭된 계정 삭제 불가.
	else if (CharactersClient[SelectedHero].CtlCode
		& (CTLCODE_02BLOCKITEM | CTLCODE_10ACCOUNT_BLOCKITEM))
		CUIMng::Instance().PopUpMsgWin(MESSAGE_DELETE_CHARACTER_ID_BLOCK);
	else
		CUIMng::Instance().PopUpMsgWin(MESSAGE_DELETE_CHARACTER_CONFIRM);
}
