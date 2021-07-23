//*****************************************************************************
// File: GaugeBar.cpp
//
// Desc: implementation of the CGaugeBar class.
//
// producer: Ahn Sang-Kyu
//*****************************************************************************

#include "stdafx.h"
#include "GaugeBar.h"
#include "UsefulDef.h"
#include "Input.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGaugeBar::CGaugeBar()
{
	m_psprBack = NULL;
	m_psizeResponse = NULL;
}

CGaugeBar::~CGaugeBar()
{
	Release();
}

//*****************************************************************************
// 함수 이름 : Create()
// 함수 설명 : 게이지바 생성.
//			   배경 스프라이트 없이 생성 가능.
//			   nBackTexID가 -1일경우 nBackWidth와 nBackHeight에 값이 있다면
//			  nBackWidth와 nBackHeight는 반응영역 값으로 인식됨. nBackWidth와
//			  nBackHeight가 0일 경우 prcGauge가 반응영역으로 인식됨.
// 매개 변수 : nGaugeWidth	: 게이지 스프라이트 픽셀 너비.
//			   nGaugeHeight	: 게이지 스프라이트 픽셀 높비.
//			   nGaugeTexID	: 게이지 텍스처 ID.
//			   prcGauge		: 게이지 영역 RECT 값의 주소.(기본값 NULL)
//			   nBackWidth	: 배경 스프라이트 너비나 배경 반응 영역.(기본값 0)
//			   nBackHeight	: 배경 스프라이트 높비나 배경 반응 영역.(기본값 0)
//			   nBackTexID	: 배경 텍스처 ID.(기본값 -1)
//			   bShortenLeft	: 게이지가 왼쪽으로 줄어드는가?(기본값 true)
//			   fScaleX		: X 확대, 축소 비율.(기본값 1.0f)
//			   fScaleY		: Y 확대, 축소 비율.(기본값 1.0f)
//*****************************************************************************
void CGaugeBar::Create(int nGaugeWidth, int nGaugeHeight, int nGaugeTexID,
					   RECT* prcGauge, int nBackWidth, int nBackHeight,
					   int nBackTexID, bool bShortenLeft, float fScaleX,
					   float fScaleY)
{
	Release();

	// 게이지바 스프라이트 생성.
	int nSizingDatums
		= bShortenLeft ? SPR_SIZING_DATUMS_LT : SPR_SIZING_DATUMS_RT;

	m_sprGauge.Create(nGaugeWidth, nGaugeHeight, nGaugeTexID, 0, NULL, 0, 0,
		true, nSizingDatums, fScaleX, fScaleY);

	// 게이지 영역이 NULL 이면 스프라이트 크기로 게이지 영역을 정함.
	if (NULL == prcGauge)
	{
		RECT rc = { 0, 0, nGaugeWidth, nGaugeHeight };
		m_rcGauge = rc;
	}
	else
	{
		m_rcGauge = *prcGauge;
		// 게이지바 스프라이트 높이 조정.
		m_sprGauge.SetSize(0, m_rcGauge.bottom - m_rcGauge.top, Y);
	}

	if (-1 < nBackTexID)	// 배경 텍스처가 있는가?
	{
		// 이경우엔 this->m_prcResponse이 NULL임.
		m_psprBack = new CSprite;
		m_psprBack->Create(nBackWidth, nBackHeight, nBackTexID, 0, NULL, 0, 0,
			false, SPR_SIZING_DATUMS_LT, fScaleX, fScaleY);
	}
	// 배경 텍스처가 없다면 배경 반응 영역은 있는가?
	else if (0 < nBackWidth && 0 < nBackHeight)
	{
		// 이 경우엔 배경 텍스처 대용으로 m_prcResponse가 사용됨.
		m_psizeResponse = new SIZE;
		m_psizeResponse->cx = nBackWidth;
		m_psizeResponse->cy = nBackHeight;
	}

	m_nXPos = m_nYPos = 0;
}

//*****************************************************************************
// 함수 이름 : Release()
// 함수 설명 : 게이지 바 Release.
//*****************************************************************************
void CGaugeBar::Release()
{
	m_sprGauge.Release();
	if (m_psprBack)
	{ delete m_psprBack;	m_psprBack = NULL; }
	else if (m_psizeResponse)
	{ delete m_psizeResponse;	m_psizeResponse = NULL; }
}

//*****************************************************************************
// 함수 이름 : SetPosition()
// 함수 설명 : 위치 지정.
// 매개 변수 : nXCoord		: X좌표.(스크린 좌표.)
//			   nYCoord		: Y좌표.(스크린 좌표.)
//*****************************************************************************
void CGaugeBar::SetPosition(int nXCoord, int nYCoord)
{
	if (m_psprBack)
		m_psprBack->SetPosition(nXCoord, nYCoord);

	m_sprGauge.SetPosition(nXCoord + m_rcGauge.left, nYCoord + m_rcGauge.top);

	m_nXPos = nXCoord;
	m_nYPos = nYCoord;
}

//*****************************************************************************
// 함수 이름 : GetWidth()
// 함수 설명 : 너비를 얻음.
//*****************************************************************************
int CGaugeBar::GetWidth()
{
	if (m_psprBack)		// 배경 스프라이트가 있다면.
		return m_psprBack->GetWidth();

	if (m_psizeResponse)	// 반응 영역이 있다면.
		return m_psizeResponse->cx;

	// 둘 다 값이 없다면 게이지바 영역 너비 리턴.
	return m_rcGauge.right - m_rcGauge.left;
}

//*****************************************************************************
// 함수 이름 : GetWidth()
// 함수 설명 : 높이를 얻음.
//*****************************************************************************
int CGaugeBar::GetHeight()
{
	if (m_psprBack)		// 배경 스프라이트가 있다면.
		return m_psprBack->GetHeight();

	if (m_psizeResponse)	// 반응 영역이 있다면.
		return m_psizeResponse->cy;

	// 둘 다 값이 없다면 게이지바 영역 높이 리턴.
	return m_rcGauge.bottom - m_rcGauge.top;
}

//*****************************************************************************
// 함수 이름 : SetValue()
// 함수 설명 : 게이지바의 수치를 정함.
// 매개 변수 : dwNow	: 현재 값.
//			   dwTotal	: 전체 값.
//*****************************************************************************
void CGaugeBar::SetValue(DWORD dwNow, DWORD dwTotal)
{
	dwTotal = (dwTotal < 1) ? 1 : dwTotal;
	dwNow = LIMIT(dwNow, 0, dwTotal);

	// 우선 dwNow의 크기를 픽셀로 계산.
	int nNowSize = dwNow * (m_rcGauge.right - m_rcGauge.left) / dwTotal;

	// 오른쪽으로 줄어드는 타잎인가?
	if (IS_SIZING_DATUMS_R(m_sprGauge.GetSizingDatums()))
	{
		// 원본 그림 파일에서 오른쪽에 여백이 있을 수 있으므로.
		nNowSize
			+= m_sprGauge.GetTexWidth() - (m_rcGauge.right - m_rcGauge.left);
	}

	m_sprGauge.SetSize(nNowSize, 0, X);
}

//*****************************************************************************
// 함수 이름 : CursorInObject()
// 함수 설명 : 게이지바 안에 커서가 위치하면 TRUE 리턴.
//*****************************************************************************
BOOL CGaugeBar::CursorInObject()
{
	if (!IsShow())
		return FALSE;

	if (m_psprBack)		// 배경 스프라이트가 있다면.
		return m_psprBack->CursorInObject();

	float fScaleX = m_sprGauge.GetScaleX();
	float fScaleY = m_sprGauge.GetScaleY();

	CInput& rInput = CInput::Instance();

	if (m_psizeResponse)	// 반응 영역이 있다면.
	{
		RECT rc = 
		{
			long(m_nXPos * fScaleX),
			long(m_nYPos * fScaleY),
			long((m_nXPos + m_psizeResponse->cx) * fScaleX),
			long((m_nYPos + m_psizeResponse->cy) * fScaleY)
		};
		return ::PtInRect(&rc, rInput.GetCursorPos());
	}

	// 둘 다 값이 없다면 게이지바 영역으로 판단.
	RECT rc = 
	{
		long(m_rcGauge.left * fScaleX),
		long(m_rcGauge.top * fScaleY),
		long(m_rcGauge.right * fScaleX),
		long(m_rcGauge.bottom * fScaleY)
	};
	::OffsetRect(&rc, m_nXPos, m_nYPos);
	return ::PtInRect(&rc, rInput.GetCursorPos());
}

//*****************************************************************************
// 함수 이름 : SetAlpha()
// 함수 설명 : 알파값 지정.
// 매개 변수 : byAlpha	: 알파 값.
//*****************************************************************************
void CGaugeBar::SetAlpha(BYTE byAlpha)
{
	if (m_psprBack)
		m_psprBack->SetAlpha(byAlpha);

	m_sprGauge.SetAlpha(byAlpha);
}

//*****************************************************************************
// 함수 이름 : SetColor()
// 함수 설명 : 게이지바 스프라이트의 칼라값 지정.
// 매개 변수 : dwColor	: 칼라 값.
//*****************************************************************************
void CGaugeBar::SetColor(BYTE byRed, BYTE byGreen, BYTE byBlue)
{
	m_sprGauge.SetColor(byRed, byGreen, byBlue);
}

//*****************************************************************************
// 함수 이름 : Show()
// 함수 설명 : 게이지바를 보이게 하거나 안보이게 함.
// 매개 변수 : bShow	: true이면 보임.(기본값 true)
//*****************************************************************************
void CGaugeBar::Show(bool bShow)
{
	if (m_psprBack)
		m_psprBack->Show(bShow);
	m_sprGauge.Show(bShow);
}

//*****************************************************************************
// 함수 이름 : Render()
// 함수 설명 : 렌더.
//*****************************************************************************
void CGaugeBar::Render()
{
	if (m_psprBack)
		m_psprBack->Render();
	m_sprGauge.Render();
}