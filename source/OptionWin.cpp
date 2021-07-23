//*****************************************************************************
// File: OptionWin.cpp
//
// Desc: implementation of the COptionWin class.
//
// producer: Ahn Sang-Kyu
//*****************************************************************************

#include "stdafx.h"

#include "OptionWin.h"
#include "Input.h"
#include "UIMng.h"

// 텍스트 랜더를 위한 #include. ㅜㅜ
#include "ZzzBMD.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzInterface.h"
#include "ZzzScene.h"
#include "DSPlaySound.h"
#include "UIControls.h"
#include "NewUISystem.h"


#define	OW_BTN_GAP		25		// 버튼 간 높이 갭.
#define	OW_SLD_GAP		48		// 슬라이더 간 높이 갭.

extern float g_fScreenRate_x;
extern float g_fScreenRate_y;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

COptionWin::COptionWin()
{

}

COptionWin::~COptionWin()
{

}

//*****************************************************************************
// 함수 이름 : Create()
// 함수 설명 : 옵션창 생성.
//*****************************************************************************
void COptionWin::Create()
{
	CInput rInput = CInput::Instance();
	// 화면에 꽉차는 검은색 반투명 기본 윈도우 생성.
	CWin::Create(rInput.GetScreenWidth(), rInput.GetScreenHeight());

// 배경 윈도우 생성.(단지 이미지로만 사용됨.)
	SImgInfo aiiBack[WE_BG_MAX] = 
	{
		{ BITMAP_SYS_WIN, 0, 0, 128, 128 },		// 중앙
		{ BITMAP_OPTION_WIN, 0, 0, 213, 65 },	// 상
		{ BITMAP_SYS_WIN+2, 0, 0, 213, 43 },	// 하
		{ BITMAP_SYS_WIN+3, 0, 0, 5, 8 },		// 좌
		{ BITMAP_SYS_WIN+4, 0, 0, 5, 8 }		// 우
	};
	m_winBack.Create(aiiBack, 1, 30);
	m_winBack.SetLine(30);	// 높이 설정.

// 버튼 생성.
	// 닫기 버튼 제외한 나머지 버튼.
	for (int i = 0; i <= OW_BTN_SLIDE_HELP; ++i)
	{
		m_aBtn[i].Create(16, 16, BITMAP_CHECK_BTN, 2, 0, 0, -1, 1, 1, 1);
		CWin::RegisterButton(&m_aBtn[i]);
	}

	// 닫기 버튼.
	DWORD adwBtnClr[4] = { CLRDW_BR_GRAY, CLRDW_BR_GRAY, CLRDW_WHITE, 0 };
	m_aBtn[OW_BTN_CLOSE].Create(108, 30, BITMAP_TEXT_BTN, 4, 2, 1);
	m_aBtn[OW_BTN_CLOSE].SetText(GlobalText[388], adwBtnClr);
	CWin::RegisterButton(&m_aBtn[OW_BTN_CLOSE]);

// 슬라이더 생성.
	SImgInfo iiThumb = { BITMAP_SLIDER, 0, 0, 13, 13 };
	SImgInfo iiBack = { BITMAP_SLIDER+2, 0, 0, 98, 13 };
	SImgInfo iiGauge = { BITMAP_SLIDER+1, 0, 0, 4, 7 };
	RECT rcGauge = { 3, 3, 95, 10 };	// 게이지 영역.
#ifdef _VS2008PORTING
	for (int i = 0; i < OW_SLD_MAX; ++i)
#else // _VS2008PORTING
	for (i = 0; i < OW_SLD_MAX; ++i)
#endif // _VS2008PORTING
		m_aSlider[i].Create(&iiThumb, &iiBack, &iiGauge, &rcGauge);
	m_aSlider[OW_SLD_EFFECT_VOL].SetSlideRange(9);
	m_aSlider[OW_SLD_RENDER_LV].SetSlideRange(4);

	// 위치 선정.
	SetPosition((rInput.GetScreenWidth() - m_winBack.GetWidth()) / 2,
		(rInput.GetScreenHeight() - m_winBack.GetHeight()) / 2);

	UpdateDisplay();	// 옵션 데이터 반영.
}

//*****************************************************************************
// 함수 이름 : PreRelease()
// 함수 설명 : 모든 컨트롤 릴리즈.(버튼은 자동 삭제)
//*****************************************************************************
void COptionWin::PreRelease()
{
	m_winBack.Release();
	for (int i = 0; i < OW_SLD_MAX; ++i)
		m_aSlider[i].Release();
}

//*****************************************************************************
// 함수 이름 : SetPosition()
// 함수 설명 : 창 위치 지정.
//			   (화면 상에는 창이 이동하는것처럼 보이지만 기본 창은 이동이 없음)
// 매개 변수 : nXCoord	: 스크린 X좌표.
//			   nYCoord	: 스크린 Y좌표.
//*****************************************************************************
void COptionWin::SetPosition(int nXCoord, int nYCoord)
{
	m_winBack.SetPosition(nXCoord, nYCoord);

	int nBtnPosX = m_winBack.GetXPos() + 52;
	int nBtnGap = OW_BTN_GAP + m_aBtn[0].GetHeight();
	int nBtnPosBaseTop = m_winBack.GetYPos() + 52;
	for (int i = 0; i <= OW_BTN_SLIDE_HELP; ++i)
		m_aBtn[i].SetPosition(nBtnPosX, nBtnPosBaseTop + i * nBtnGap);

	m_aBtn[OW_BTN_CLOSE].SetPosition(m_winBack.GetXPos()
		+ (m_winBack.GetWidth() - m_aBtn[OW_BTN_CLOSE].GetWidth()) / 2,
		m_winBack.GetYPos() + 301);

//	int nSldPosX = m_winBack.GetXPos()
//		+ (m_winBack.GetWidth() - m_aSlider[0].GetWidth()) / 2;
	int nSldGap = OW_SLD_GAP + m_aSlider[0].GetHeight();
	int nSldPosBaseTop = m_aBtn[OW_BTN_SLIDE_HELP].GetYPos()
		+ m_aBtn[0].GetHeight() + OW_SLD_GAP;
#ifdef _VS2008PORTING
	for (int i = 0; i < OW_SLD_MAX; ++i)
#else // _VS2008PORTING
	for (i = 0; i < OW_SLD_MAX; ++i)
#endif // _VS2008PORTING
		m_aSlider[i].SetPosition(nBtnPosX, nSldPosBaseTop + i * nSldGap);
}

//*****************************************************************************
// 함수 이름 : Show()
// 함수 설명 : 창을 보여 주거나 안보이게함.
// 매개 변수 : bShow	: true이면 보여줌.
//*****************************************************************************
void COptionWin::Show(bool bShow)
{
	CWin::Show(bShow);

	m_winBack.Show(bShow);
	for (int i = 0; i < OW_BTN_MAX; ++i)
		m_aBtn[i].Show(bShow);
#ifdef _VS2008PORTING
	for (int i = 0; i < OW_SLD_MAX; ++i)
#else // _VS2008PORTING
	for (i = 0; i < OW_SLD_MAX; ++i)
#endif // _VS2008PORTING
		m_aSlider[i].Show(bShow);
}

//*****************************************************************************
// 함수 이름 : CursorInWin()
// 함수 설명 : 윈도우 영역 안에 마우스 커서가 위치하는가?
// 매개 변수 : eArea	: 검사할 영역.(win.h의 #define 참조)
//*****************************************************************************
bool COptionWin::CursorInWin(int nArea)
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
void COptionWin::UpdateDisplay()
{
	m_aBtn[OW_BTN_AUTO_ATTACK].SetCheck(g_pOption->IsAutoAttack());
	m_aBtn[OW_BTN_WHISPER_ALARM].SetCheck(g_pOption->IsWhisperSound());
	m_aBtn[OW_BTN_SLIDE_HELP].SetCheck(g_pOption->IsSlideHelp());
	m_aSlider[OW_SLD_EFFECT_VOL].SetSlidePos(g_pOption->GetVolumeLevel());
	m_aSlider[OW_SLD_RENDER_LV].SetSlidePos(g_pOption->GetRenderLevel());
}

//*****************************************************************************
// 함수 이름 : UpdateWhileActive()
// 함수 설명 : 액티브일 때의 업데이트.
// 매개 변수 : dDeltaTick	: 이전 Update()호출 후부터 지금 Update()까지 시간.
//*****************************************************************************
void COptionWin::UpdateWhileActive(double dDeltaTick)
{
	for (int i = 0; i < OW_SLD_MAX; ++i)
		m_aSlider[i].Update(dDeltaTick);

	if (m_aBtn[OW_BTN_AUTO_ATTACK].IsClick())
	{
		g_pOption->SetAutoAttack(m_aBtn[OW_BTN_AUTO_ATTACK].IsCheck());
	}
	else if (m_aBtn[OW_BTN_WHISPER_ALARM].IsClick())
	{
		g_pOption->SetWhisperSound( m_aBtn[OW_BTN_WHISPER_ALARM].IsCheck() );
	}
	else if (m_aBtn[OW_BTN_SLIDE_HELP].IsClick())
	{
		g_pOption->SetSlideHelp( m_aBtn[OW_BTN_SLIDE_HELP].IsCheck() );
	}
	else if (m_aBtn[OW_BTN_CLOSE].IsClick())
	{
		CUIMng::Instance().HideWin(this);
		CUIMng::Instance().SetSysMenuWinShow(false);
	}
	// 효과음 슬라이더에 변화가 있는가?
	else if (m_aSlider[OW_SLD_EFFECT_VOL].GetState())
	{
		int nSlidePos = m_aSlider[OW_SLD_EFFECT_VOL].GetSlidePos();

		if(g_pOption->GetVolumeLevel() != nSlidePos)
		{
			g_pOption->SetVolumeLevel(nSlidePos);
			::SetEffectVolumeLevel(g_pOption->GetVolumeLevel());
		}
	}
	// 렌더레벨 슬라이더에 변화가 있는가?
	else if (m_aSlider[OW_SLD_RENDER_LV].GetState())
	{
		int nSlidePos = m_aSlider[OW_SLD_RENDER_LV].GetSlidePos();
		if(g_pOption->GetRenderLevel() != nSlidePos)
		{
			g_pOption->SetRenderLevel(nSlidePos);
		}
	}
	else if (CInput::Instance().IsKeyDown(VK_ESCAPE))
	{
		::PlayBuffer(SOUND_CLICK01);	// 클릭 사운드.
		CUIMng::Instance().HideWin(this);
		CUIMng::Instance().SetSysMenuWinShow(false);
	}
}

//*****************************************************************************
// 함수 이름 : RenderControls()
// 함수 설명 : 각종 컨트롤 렌더.
//*****************************************************************************
void COptionWin::RenderControls()
{
	m_winBack.Render();	

	// 제목.
	g_pRenderText->SetFont(g_hFixFont);
	g_pRenderText->SetTextColor(CLRDW_WHITE);
	g_pRenderText->SetBgColor(0);
	g_pRenderText->RenderText(int(m_winBack.GetXPos() / g_fScreenRate_x),
		int((m_winBack.GetYPos() + 10) / g_fScreenRate_y),
		GlobalText[385], m_winBack.GetWidth() / g_fScreenRate_x, 0, RT3_SORT_CENTER);	

	// 체크 버튼 텍스트.
	const char* apszBtnText[3] =
	{ GlobalText[386], GlobalText[387], GlobalText[919] };
	for (int i = 0; i <= OW_BTN_SLIDE_HELP; ++i)
	{
		g_pRenderText->RenderText(int((m_aBtn[i].GetXPos() + 24) / g_fScreenRate_x),
			int((m_aBtn[i].GetYPos() + 4) / g_fScreenRate_y), apszBtnText[i]);
	}

	// 슬라이더.
	int nTextPosY;
	const char* apszSldText[OW_SLD_MAX] = { GlobalText[389], GlobalText[1840] };
	int anVal[OW_SLD_MAX] = { g_pOption->GetVolumeLevel(), g_pOption->GetRenderLevel() * 2 + 5};
	
	char szVal[3];
#ifdef _VS2008PORTING
	for (int i = 0; i < OW_SLD_MAX; ++i)	
#else // _VS2008PORTING
	for (i = 0; i < OW_SLD_MAX; ++i)
#endif // _VS2008PORTING
	{
		nTextPosY = int((m_aSlider[i].GetYPos() - 18) / g_fScreenRate_y);
		g_pRenderText->RenderText(int(m_aSlider[i].GetXPos() / g_fScreenRate_x),
			nTextPosY, apszSldText[i]);

		::_itoa(anVal[i], szVal, 10);
		g_pRenderText->RenderText(int((m_aSlider[i].GetXPos() + 85)/ g_fScreenRate_x),
			nTextPosY, szVal);
		
		m_aSlider[i].Render();
	}

	CWin::RenderButtons();
}