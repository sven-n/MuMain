//*****************************************************************************
// File: CreditWin.cpp
//
// Desc: implementation of the CCreditWin class.
//
// producer: Ahn Sang-Kyu
//*****************************************************************************

#include "stdafx.h"
#include "CreditWin.h"
#include "Input.h"
#include "UIMng.h"

// PlayBuffer() 함수 사용하기 위한 include.
#include "ZzzInfomation.h"
#include "ZzzBMD.h"
#include "ZzzObject.h"
#include "DSPlaySound.h"

// RenderText3() 함수 사용하기 위한 include.
#include "ZzzCharacter.h"
#include "ZzzInterface.h"

#include "Local.h"
#include "./Utilities/Log/ErrorReport.h"
#include "wsclientinline.h"
#include "UIControls.h"

#define	CRW_ILLUST_FADE_TIME	2000.0	// 알파 증감값.
#define	CRW_ILLUST_SHOW_TIME	22000.0	// 보여 주는 시간.

#define	CRW_TEXT_FADE_TIME		300.0	// 텍스트가 나타나거나 사라지는 시간.
#define	CRW_NAME_SHOW_TIME		2300.0	// 보여 주는 시간.

// 크레딧 텍스트 정보 파일명.
#ifdef USE_CREDITTEST_BMD
#define	CRW_DATA_FILE		"Data\\Local\\credittest.bmd"
#else
#define	CRW_DATA_FILE		"Data\\Local\\credit.bmd"
#endif

extern float g_fScreenRate_x;
extern char* g_lpszMp3[NUM_MUSIC];
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCreditWin::CCreditWin() : m_hFont(NULL)
{

}

CCreditWin::~CCreditWin()
{

}

//*****************************************************************************
// 함수 이름 : Create()
// 함수 설명 : 크레딧 창 생성.
//*****************************************************************************
void CCreditWin::Create()
{
	CInput rInput = CInput::Instance();

// 기본 윈도우.
	CWin::Create(rInput.GetScreenWidth(), rInput.GetScreenHeight());
	CWin::SetBgAlpha(255);	// 불투명임.

// 각 스프라이트.
	// 800 * 600 가상 스크린 모드를 사용 함.
	float fScaleX = (float)rInput.GetScreenWidth() / 800.0f;
	float fScaleY = (float)rInput.GetScreenHeight() / 600.0f;

	m_aSpr[CRW_SPR_DECO].Create(189, 103, BITMAP_LOG_IN+6);
	m_aSpr[CRW_SPR_LOGO].Create(290, 41, BITMAP_LOG_IN+14, 0, NULL, 0, 0,
		false, SPR_SIZING_DATUMS_LT, fScaleX, fScaleY);

	for (int i = CRW_SPR_TXT_HIDE0; i <= CRW_SPR_TXT_HIDE2; ++i)
	{
		m_aSpr[i].Create(800, 42, -1, 0, NULL, 0, 0, false,
			SPR_SIZING_DATUMS_LT, fScaleX, fScaleY);
		m_aSpr[i].SetColor(0, 0, 0);
	}

// 닫기 버튼.
	m_btnClose.Create(54, 30, BITMAP_BUTTON+2, 3, 2, 1);
	CWin::RegisterButton(&m_btnClose);

// 일러스트.
	m_eIllustState = HIDE;

	m_apszIllustPath[0][0] = "Interface\\im1_1.jpg";
	m_apszIllustPath[0][1] = "Interface\\im1_2.jpg";
	m_apszIllustPath[1][0] = "Interface\\im2_1.jpg";
	m_apszIllustPath[1][1] = "Interface\\im2_2.jpg";
	m_apszIllustPath[2][0] = "Interface\\im3_1.jpg";
	m_apszIllustPath[2][1] = "Interface\\im3_2.jpg";
	m_apszIllustPath[3][0] = "Interface\\im4_1.jpg";
	m_apszIllustPath[3][1] = "Interface\\im4_2.jpg";
	m_apszIllustPath[4][0] = "Interface\\im5_1.jpg";
	m_apszIllustPath[4][1] = "Interface\\im5_2.jpg";
	m_apszIllustPath[5][0] = "Interface\\im6_1.jpg";
	m_apszIllustPath[5][1] = "Interface\\im6_2.jpg";
	m_apszIllustPath[6][0] = "Interface\\im7_1.jpg";
	m_apszIllustPath[6][1] = "Interface\\im7_2.jpg";
	m_apszIllustPath[7][0] = "Interface\\im8_1.jpg";
	m_apszIllustPath[7][1] = "Interface\\im8_2.jpg";

// 텍스트.
	int nFontSize = 10;
	switch(rInput.GetScreenWidth())
	{
	case 800 :	nFontSize = 14;	break;
	case 1024:	nFontSize = 18;	break;
	case 1280:	nFontSize = 24;	break;
	}
#ifdef KJH_MOD_NATION_LANGUAGE_REDEFINE
	m_hFont = ::CreateFont(nFontSize, 0, 0, 0, FW_BOLD, 0, 0, 0,
		g_dwCharSet, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		NONANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		GlobalText[0][0] ? GlobalText[0] : NULL);
#else // KJH_MOD_NATION_LANGUAGE_REDEFINE
	m_hFont = ::CreateFont(nFontSize, 0, 0, 0, FW_BOLD, 0, 0, 0,
		g_dwCharSet[SELECTED_LANGUAGE], OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		NONANTIALIASED_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		GlobalText[0][0] ? GlobalText[0] : NULL);
#endif // KJH_MOD_NATION_LANGUAGE_REDEFINE

	LoadText();

#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
	SetPosition();	// 각 컨트롤 위치를 잡아주는 의미에서 호출.
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
	SetPosition(0, 0);	// 각 컨트롤 위치를 잡아주는 의미에서 호출.
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
}

//*****************************************************************************
// 함수 이름 : PreRelease()
// 함수 설명 : 모든 컨트롤 릴리즈.(버튼은 자동 삭제)
//*****************************************************************************
void CCreditWin::PreRelease()
{
	for (int i = 0; i < CRW_SPR_MAX; ++i)
		m_aSpr[i].Release();

	if (m_hFont)
	{
		::DeleteObject((HGDIOBJ)m_hFont);
		m_hFont = NULL;
	}
}

//*****************************************************************************
// 함수 이름 : SetPosition()
// 함수 설명 : 창 위치 지정.
// 매개 변수 : nXCoord	: 스크린 X좌표.
//			   nYCoord	: 스크린 Y좌표.
//*****************************************************************************
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
void CCreditWin::SetPosition()
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
void CCreditWin::SetPosition(int nXCoord, int nYCoord)
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
{
	m_aSpr[CRW_SPR_PIC_L].SetPosition(0, 126);
	m_aSpr[CRW_SPR_PIC_R].SetPosition(400, 126);
	m_aSpr[CRW_SPR_LOGO].SetPosition(241, 549);

	// 좌측 하단 장식.
	CInput rInput = CInput::Instance();
	// 800*600 모드에서 일러스트 밑바닥 Y위치가 527임.
	int nBaseY = int(527.0f / 600.0f * (float)rInput.GetScreenHeight());
	m_aSpr[CRW_SPR_DECO].SetPosition(
		rInput.GetScreenWidth() - m_aSpr[CRW_SPR_DECO].GetWidth(),
		nBaseY - m_aSpr[CRW_SPR_DECO].GetHeight());

	// 텍스트 가리개.
	for (int i = CRW_SPR_TXT_HIDE0; i <= CRW_SPR_TXT_HIDE2; ++i)
		m_aSpr[i].SetPosition(0, 42 * (i - CRW_SPR_TXT_HIDE0));

	// 닫기 버튼.
	m_btnClose.SetPosition(m_aSpr[CRW_SPR_DECO].GetXPos() + 122,
		m_aSpr[CRW_SPR_DECO].GetYPos() + 63);
}

//*****************************************************************************
// 함수 이름 : Show()
// 함수 설명 : 창을 보여 주거나 안보이게함.
// 매개 변수 : bShow	: true이면 보여줌.
//*****************************************************************************
void CCreditWin::Show(bool bShow)
{
	CWin::Show(bShow);

	for (int i = 0; i < CRW_SPR_MAX; ++i)
		m_aSpr[i].Show(bShow);

	m_btnClose.Show(bShow);

	if (bShow)
		Init();
	else
		m_eIllustState = HIDE;
}

//*****************************************************************************
// 함수 이름 : CursorInWin()
// 함수 설명 : 윈도우 영역 안에 마우스 커서가 위치하는가?
// 매개 변수 : eArea	: 검사할 영역.(win.h의 #define 참조)
//*****************************************************************************
bool CCreditWin::CursorInWin(int nArea)
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
void CCreditWin::UpdateWhileActive(double dDeltaTick)
{
	if (m_btnClose.IsClick())
		CloseWin();
	else if (CInput::Instance().IsKeyDown(VK_ESCAPE))
	{
		::PlayBuffer(SOUND_CLICK01);	// 클릭 사운드.
		CloseWin();
		CUIMng::Instance().SetSysMenuWinShow(false);
	}

// 텍스트 및 일러스트 나타났다 사라지는 효과 애니메이션.
	for (int i = 0; i <= CRW_INDEX_NAME; ++i)
		AnimationText(i, dDeltaTick);
	AnimationIllust(dDeltaTick);
	
	/*
	if (::IsEndMp3())		// 음악 반복.
	{
		::StopMp3(g_lpszMp3[MUSIC_MUTHEME]);
		::PlayMp3(g_lpszMp3[MUSIC_MUTHEME]);
	}
	*/
}

//*****************************************************************************
// 함수 이름 : RenderControls()
// 함수 설명 : 각종 컨트롤 렌더.
//*****************************************************************************
void CCreditWin::RenderControls()
{
	::glDisable(GL_ALPHA_TEST);	// 일러스트가 알파값이 적을땐 아예 안보이므로.

	// 장식, 로고 스프라이트.
	for (int i = 0; i <= CRW_SPR_LOGO; ++i)
		m_aSpr[i].Render();

	long lScreenWidth = CInput::Instance().GetScreenWidth();
	int nTextBoxWidth;

	g_pRenderText->SetFont(m_hFont);
	g_pRenderText->SetTextColor(CLRDW_BR_GRAY);
	g_pRenderText->SetBgColor(0);
	
	// 1번째줄 텍스트.
	nTextBoxWidth = lScreenWidth / g_fScreenRate_x;

	g_pRenderText->RenderText(0, 20,
		m_aCredit[m_anTextIndex[CRW_INDEX_DEPARTMENT]].szName,
		nTextBoxWidth, 0, RT3_SORT_CENTER);
	
	// 2번째줄 텍스트.
	g_pRenderText->RenderText(0, 46, m_aCredit[m_anTextIndex[CRW_INDEX_TEAM]].szName,
		nTextBoxWidth, 0, RT3_SORT_CENTER);

	// 3번째줄 텍스트.
	g_pRenderText->SetTextColor(CLRDW_BR_YELLOW);
	
	switch (m_nNameCount)
	{
	case 1:
		g_pRenderText->RenderText(0, 72, m_aCredit[m_anTextIndex[CRW_INDEX_NAME0]].szName,
			nTextBoxWidth, 0, RT3_SORT_CENTER);
		break;
	case 2:
		nTextBoxWidth = lScreenWidth / 4 / g_fScreenRate_x;
		g_pRenderText->RenderText(160, 72, m_aCredit[m_anTextIndex[CRW_INDEX_NAME0]].szName,
			nTextBoxWidth, 0, RT3_SORT_CENTER);
		g_pRenderText->RenderText(320, 72, m_aCredit[m_anTextIndex[CRW_INDEX_NAME1]].szName,
			nTextBoxWidth, 0, RT3_SORT_CENTER);
		break;
	case 3:
		nTextBoxWidth = lScreenWidth / 3 / g_fScreenRate_x;
		g_pRenderText->RenderText(0, 72, m_aCredit[m_anTextIndex[CRW_INDEX_NAME0]].szName,
			nTextBoxWidth, 0, RT3_SORT_CENTER);
		g_pRenderText->RenderText(213, 72, m_aCredit[m_anTextIndex[CRW_INDEX_NAME1]].szName,
			nTextBoxWidth, 0, RT3_SORT_CENTER);
		g_pRenderText->RenderText(426, 72, m_aCredit[m_anTextIndex[CRW_INDEX_NAME2]].szName,
			nTextBoxWidth, 0, RT3_SORT_CENTER);
		break;
	case 4:
		nTextBoxWidth = lScreenWidth / 4 / g_fScreenRate_x;
		g_pRenderText->RenderText(0, 72, m_aCredit[m_anTextIndex[CRW_INDEX_NAME0]].szName,
			nTextBoxWidth, 0, RT3_SORT_CENTER);
		g_pRenderText->RenderText(160, 72, m_aCredit[m_anTextIndex[CRW_INDEX_NAME1]].szName,
			nTextBoxWidth, 0, RT3_SORT_CENTER);
		g_pRenderText->RenderText(320, 72, m_aCredit[m_anTextIndex[CRW_INDEX_NAME2]].szName,
			nTextBoxWidth, 0, RT3_SORT_CENTER);
		g_pRenderText->RenderText(480, 72, m_aCredit[m_anTextIndex[CRW_INDEX_NAME3]].szName,
			nTextBoxWidth, 0, RT3_SORT_CENTER);
		break;
	}

	// 텍스트 가리개 스프라이트.
#ifdef _VS2008PORTING
	for (int i = CRW_SPR_TXT_HIDE0; i <= CRW_SPR_TXT_HIDE2; ++i)
#else // _VS2008PORTING
	for (i = CRW_SPR_TXT_HIDE0; i <= CRW_SPR_TXT_HIDE2; ++i)
#endif // _VS2008PORTING
		m_aSpr[i].Render();

	::glEnable(GL_ALPHA_TEST);	// 알파 테스트 원상태로 돌려줌.

	CWin::RenderButtons();
}

//*****************************************************************************
// 함수 이름 : CloseWin()
// 함수 설명 : 크레딧 창을 닫음.
//*****************************************************************************
void CCreditWin::CloseWin()
{
	CUIMng::Instance().HideWin(this);

	// 5분 동안 아무짓 안하면 서버에서 끊어버리기 때문에 서버리스트를 요청함.
#ifdef PKD_ADD_ENHANCED_ENCRYPTION
	SendRequestServerList2();
#else // PKD_ADD_ENHANCED_ENCRYPTION
	SendRequestServerList();
#endif // PKD_ADD_ENHANCED_ENCRYPTION

	::StopMp3(g_lpszMp3[MUSIC_MUTHEME]);
	::PlayMp3(g_lpszMp3[MUSIC_MAIN_THEME]);
}

//*****************************************************************************
// 함수 이름 : Init()
// 함수 설명 : 일러스트, 텍스트 초기화.
//*****************************************************************************
void CCreditWin::Init()
{
	// 일러스트 애니 초기화.
	m_eIllustState = FADEIN;
	m_dIllustDeltaTickSum = 0.0;
	m_byIllust = 0;
	LoadIllust();

	// 텍스트 애니 초기화.
	for (int i = 0; i <= CRW_INDEX_NAME; ++i)
		m_aeTextState[i] = FADEIN;
	m_dTextDeltaTickSum = 0.0;
	m_nNowIndex = 0;
	m_nNameCount = 0;
	SetTextIndex();
}

//*****************************************************************************
// 함수 이름 : LoadIllust()
// 함수 설명 : 일러스트 파일을 읽어 스프라이트를 생성함.
//*****************************************************************************
void CCreditWin::LoadIllust()
{
	CInput rInput = CInput::Instance();
	// 800 * 600 가상 스크린 모드를 사용 함.
	float fScaleX = (float)rInput.GetScreenWidth() / 800.0f;
	float fScaleY = (float)rInput.GetScreenHeight() / 600.0f;
	
	for (int i = 0; i < 2; ++i)
	{
		LoadBitmap(m_apszIllustPath[m_byIllust][i], BITMAP_TEMP+i, GL_LINEAR);
		
		m_aSpr[i].Create(400, 400, BITMAP_TEMP+i, 0, NULL, 0, 0,
			false, SPR_SIZING_DATUMS_LT, fScaleX, fScaleY);
		m_aSpr[i].SetAlpha(0);
		m_aSpr[i].Show(true);
	}

	m_aSpr[CRW_SPR_PIC_L].SetPosition(0, 126);
	m_aSpr[CRW_SPR_PIC_R].SetPosition(400, 126);
}

//*****************************************************************************
// 함수 이름 : AnimationIllust()
// 함수 설명 : 일러스트의 사라지고 나타남과 일러스트 교체 처리.
// 매개 변수 : dDeltaTick	: 이전 Update()호출 후부터 지금 Update()까지 시간.
//*****************************************************************************
void CCreditWin::AnimationIllust(double dDeltaTick)
{
	short nAlpha;
	switch (m_eIllustState)
	{
	case FADEIN:	// 서서히 나타나는 경우.
		nAlpha = short(m_aSpr[CRW_SPR_PIC_L].GetAlpha());
		nAlpha += short(255.0 * dDeltaTick / CRW_ILLUST_FADE_TIME);
		if (255 <= nAlpha)
		{
			m_eIllustState = SHOW;
			nAlpha = 255;
		}
		m_aSpr[CRW_SPR_PIC_L].SetAlpha((BYTE)nAlpha);
		m_aSpr[CRW_SPR_PIC_R].SetAlpha((BYTE)nAlpha);
		break;

	case SHOW:		// 완전히 보이는 경우.
		m_dIllustDeltaTickSum += dDeltaTick;
		if (m_dIllustDeltaTickSum > CRW_ILLUST_SHOW_TIME)
		{
			m_eIllustState = FADEOUT;
			m_dIllustDeltaTickSum = 0.0;
		}
		break;

	case FADEOUT:	// 서서히 사라지는 경우.
		nAlpha = short(m_aSpr[CRW_SPR_PIC_L].GetAlpha());
		nAlpha -= short(255.0 * dDeltaTick / CRW_ILLUST_FADE_TIME);
		if (0 >= nAlpha)
		{
			m_eIllustState = FADEIN;
			nAlpha = 0;

			m_byIllust = ++m_byIllust == CRW_ILLUST_MAX ? 0 : m_byIllust;
			LoadIllust();
		}
		m_aSpr[CRW_SPR_PIC_L].SetAlpha((BYTE)nAlpha);
		m_aSpr[CRW_SPR_PIC_R].SetAlpha((BYTE)nAlpha);
		break;
	}
}

void BuxConvert(BYTE *Buffer, int Size);

//*****************************************************************************
// 함수 이름 : LoadText()
// 함수 설명 : 텍스트 정보 파일을 읽음.
//*****************************************************************************
void CCreditWin::LoadText()
{
	FILE *fp = ::fopen(CRW_DATA_FILE, "rb");
	if (fp == NULL)
	{
		char szMessage[256];
		::sprintf(szMessage, "%s file not found.\r\n", CRW_DATA_FILE);
		g_ErrorReport.Write(szMessage);
		::MessageBox(g_hWnd, szMessage, NULL, MB_OK);
		::PostMessage(g_hWnd, WM_DESTROY, 0, 0);
		return;
	}

	int nSize = sizeof(SCreditItem) * CRW_ITEM_MAX;
	::fread(m_aCredit, nSize, 1, fp);
	::BuxConvert((BYTE*)m_aCredit, nSize);

	::fclose(fp);
}

//*****************************************************************************
// 함수 이름 : SetTextIndex()
// 함수 설명 : 한번에 보여줄 텍스트의 정보 인덱스를 세팅.
//*****************************************************************************
void CCreditWin::SetTextIndex()
{
	if (0 == m_aCredit[m_nNowIndex].byClass)	// 0이면 끝임.
	{
		::PlayBuffer(SOUND_CLICK01);	// 클릭 사운드.
		CloseWin();
	}

	if (1 == m_aCredit[m_nNowIndex].byClass)	// 1째줄에 들어갈 내용인가?
	{
		m_anTextIndex[CRW_INDEX_DEPARTMENT] = m_nNowIndex;
		++m_nNowIndex;
	}
	if (2 == m_aCredit[m_nNowIndex].byClass)	// 2째줄에 들어갈 내용인가?
	{
		m_anTextIndex[CRW_INDEX_TEAM] = m_nNowIndex;
		++m_nNowIndex;
	}
#ifdef _VS2008PORTING
	int iNameCnt = 0;
	for (int i = 0; i < 4; ++i)		// 3번째줄은 한번에 최대 4개까지 표시.
	{
		iNameCnt = i;
		if (3 == m_aCredit[m_nNowIndex].byClass)// 3째줄에 들어갈 내용인가?
		{
			m_anTextIndex[CRW_INDEX_NAME0+i] = m_nNowIndex;
			++m_nNowIndex;
		}
		else
			break;
	}
	m_nNameCount = iNameCnt;	// 3째줄에 표시할 항목 개수.
#else // _VS2008PORTING
	for (int i = 0; i < 4; ++i)		// 3번째줄은 한번에 최대 4개까지 표시.
	{
		if (3 == m_aCredit[m_nNowIndex].byClass)// 3째줄에 들어갈 내용인가?
		{
			m_anTextIndex[CRW_INDEX_NAME0+i] = m_nNowIndex;
			++m_nNowIndex;
		}
		else
			break;
	}
	m_nNameCount = i;	// 3째줄에 표시할 항목 개수.
#endif // _VS2008PORTING

}

//*****************************************************************************
// 함수 이름 : AnimationText()
// 함수 설명 : 텍스트의 사라지고 나타남과 텍스트 교체 처리.
// 매개 변수 : nClass		: 애니메이션 시킬 줄 번호(1~3)
//			   dDeltaTick	: 이전 Update()호출 후부터 지금 Update()까지 시간.
//*****************************************************************************
void CCreditWin::AnimationText(int nClass, double dDeltaTick)
{
	SHOW_STATE* peTextState = &m_aeTextState[nClass];
	short nAlpha;
	// 텍스트를 가리고 있는 스프라이트.
	CSprite* psprHide = &m_aSpr[CRW_SPR_TXT_HIDE0 + nClass];

	switch (*peTextState)
	{
	case FADEIN:	// 서서히 나타나는 경우.
		nAlpha = short(psprHide->GetAlpha());
		nAlpha -= short(255.0 * dDeltaTick / CRW_TEXT_FADE_TIME);
		if (0 >= nAlpha)
		{
			*peTextState = SHOW;
			nAlpha = 0;
		}
		psprHide->SetAlpha((BYTE)nAlpha);
		break;

	case SHOW:		// 완전히 보이는 경우.
		if (nClass != CRW_INDEX_NAME)
			break;

		m_dTextDeltaTickSum += dDeltaTick;
		if (m_dTextDeltaTickSum > CRW_NAME_SHOW_TIME)
		{
			m_aeTextState[CRW_INDEX_NAME] = FADEOUT;
			m_dTextDeltaTickSum = 0.0;

			if (3 != m_aCredit[m_nNowIndex].byClass)	// 0 ~ 2면.
			{
				m_aeTextState[CRW_INDEX_TEAM] = FADEOUT;
				if (2 != m_aCredit[m_nNowIndex].byClass)// 0 ~ 1이면.
					m_aeTextState[CRW_INDEX_DEPARTMENT] = FADEOUT;
			}
		}
		break;

	case FADEOUT:	// 서서히 사라지는 경우.
		nAlpha = short(psprHide->GetAlpha());
		nAlpha += short(255.0 * dDeltaTick / CRW_TEXT_FADE_TIME);
		if (255 <= nAlpha)
		{
			*peTextState = FADEIN;
			nAlpha = 255;

			if (nClass == CRW_INDEX_NAME)
				SetTextIndex();
		}
		psprHide->SetAlpha((BYTE)nAlpha);
		break;
	}
}
