//*****************************************************************************
// File: Slider.cpp
//
// Desc: implementation of the CSlider class.
//
// producer: Ahn Sang-Kyu
//*****************************************************************************

#include "stdafx.h"
#include "Slider.h"

#include "GaugeBar.h"
#include "Input.h"

// 썸 좌측 또는 우측 영역을 누루고 있을 때 사용.
#define SLD_FIRST_SLIDE_DELAY_TIME	500		// 첫 슬라이드 지연시간.
#define SLD_SLIDE_DELAY_TIME		50		// 슬라이드 지연시간.

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSlider::CSlider() : m_pGaugeBar(NULL), m_psprBack(NULL)
{

}

CSlider::~CSlider()
{
	SAFE_DELETE(m_pGaugeBar);
	SAFE_DELETE(m_psprBack);
}

//*****************************************************************************
// 함수 이름 : Create()
// 함수 설명 : 슬라이더 생성.
// 매개 변수 : piiThumb	: 썸 이미지 정보.
//			   piiBack	: 배경 이미지 정보.
//						  배경 이미지를 따로 사용하지 않는다면 piiBack->nTexID
//						를 -1로 놓고 배경 이미지를 대신할 이미지의 너비와 높이
//						를 piiBack->nWidth, piiBack->nHeight에 채움.
//			   piiGauge	: 게이지 이미지 정보.(기본값 NULL)
//						  현재 게이지바는 가로 슬라이더에만 적용.
//			   prcGauge	: 배경 이미지 기준으로 게이지 영역.(기본값 NULL)
//			   bVertical: 세로 슬라이더 생성이면 true. 가로면 false.
//						(기본값: false)
//*****************************************************************************
void CSlider::Create(SImgInfo* piiThumb, SImgInfo* piiBack, SImgInfo* piiGauge,
					 RECT* prcGauge, bool bVertical)
{
	m_btnThumb.Create(piiThumb->nWidth, piiThumb->nHeight, piiThumb->nTexID);

	SAFE_DELETE(m_pGaugeBar);
	SAFE_DELETE(m_psprBack);

	if (piiGauge)
	{
		m_pGaugeBar = new CGaugeBar;
		m_pGaugeBar->Create(
			piiGauge->nWidth, piiGauge->nHeight, piiGauge->nTexID, prcGauge,
			piiBack->nWidth, piiBack->nHeight, piiBack->nTexID);
	}
	else if (-1 < piiBack->nTexID)
	{
		m_psprBack->Create(piiBack);
	}

	m_bVertical = bVertical;
	m_byState = SLD_STATE_IDLE;
	m_nSlidePos = 0;
	m_nSlideRange = 1;
	m_ptPos.x = 0;
	m_ptPos.y = 0;
	m_dThumbMoveTime = 0.0;

	int nThumbRange;
	if (m_bVertical)	// 세로 슬라이더인가?
	{
		// Thumb이 움직일 수 있는 범위 = 전체 길이 - 썸 버튼 크기.
		nThumbRange = piiBack->nHeight - m_btnThumb.GetHeight();

		m_Size.cx = m_btnThumb.GetWidth();
		m_Size.cy = piiBack->nHeight;
	}
	else
	{
		nThumbRange = piiBack->nWidth - m_btnThumb.GetWidth();

		m_Size.cx = piiBack->nWidth;
		m_Size.cy = m_btnThumb.GetHeight();
	}
	// Thumb이 움직일 수 있는 범위가 음수면 0, 아니면 그대로.
	m_nThumbRange = nThumbRange < 0 ? 0 : nThumbRange;
}

//*****************************************************************************
// 함수 이름 : Create()
// 함수 설명 : 슬라이더를 메모리에서 삭제.
//*****************************************************************************
void CSlider::Release()
{
	m_btnThumb.Release();
	SAFE_DELETE(m_pGaugeBar);
	SAFE_DELETE(m_psprBack);
}

//*****************************************************************************
// 함수 이름 : SetThumbPosition()
// 함수 설명 : 현재의 슬라이드 위치(m_nSlidePos)에 따라 Thumb을 위치시킴.
//*****************************************************************************
void CSlider::SetThumbPosition()
{
	float fThumbPos;
	if (m_bVertical)	// 세로 슬라이더인가?
	{
		// Thumb의 위치 = 슬라이더의 위치
		//+ Thumb의 한 칸 이동 범위 * 슬라이드 위치.
		fThumbPos = float(m_ptPos.y)
			+ (float)m_nThumbRange / m_nSlideRange * m_nSlidePos;

		m_btnThumb.SetPosition(m_ptPos.x, (int)fThumbPos);
	}
	else				// 가로 슬라이더이면.
	{
		fThumbPos = float(m_ptPos.x)
			+ (float)m_nThumbRange / m_nSlideRange * m_nSlidePos;

		m_btnThumb.SetPosition((int)fThumbPos, m_ptPos.y);

		m_pGaugeBar->SetValue(m_nSlidePos, m_nSlideRange);
	}
}

//*****************************************************************************
// 함수 이름 : SetPosition()
// 함수 설명 : 슬라이더 위치 지정.
// 매개 변수 : nXCoord	: 스크롤바의 X좌표.
//			   nYCoord	: 스크롤바의 Y좌표.
//*****************************************************************************
void CSlider::SetPosition(int nXCoord, int nYCoord)
{
	m_ptPos.x = nXCoord;
	m_ptPos.y = nYCoord;

	SetThumbPosition();

	if (m_pGaugeBar)
		m_pGaugeBar->SetPosition(nXCoord, nYCoord);
	else if (m_psprBack)
		m_psprBack->SetPosition(nXCoord, nYCoord);
}

//*****************************************************************************
// 함수 이름 : SetSlideRange()
// 함수 설명 : 표현할 전체 범위 변경.
// 매개 변수 : nSlideRange	: 표현할 전체 범위.
//*****************************************************************************
void CSlider::SetSlideRange(int nSlideRange)
{
	m_nSlideRange = MAX(nSlideRange, 1);	// 1보다 작아서는 안됨.

	SetThumbPosition();
}

//*****************************************************************************
// 함수 이름 : LineUp()
// 함수 설명 : 슬라이드 위치를 하나 감소시키고 이에 따른 Thumb위치 변경.
//*****************************************************************************
void CSlider::LineUp()
{
	if (m_bVertical)	// 세로 슬라이더인가?
	{
		// Thumb의 y위치 = 슬라이더의 위치
		float fThumbYPos = float(m_ptPos.y)
			// + Thumb이 한 칸 이동하는 범위.
			+ (float)m_nThumbRange / m_nSlideRange
			* --m_nSlidePos;	// * --슬라이드 위치.

		m_btnThumb.SetPosition(m_ptPos.x, (int)fThumbYPos);
	}
	else				// 가로 슬라이더이면.
	{
		// Thumb의 x위치 = 슬라이더의 위치
		float fThumbXPos = float(m_ptPos.x)
			// + Thumb이 한 칸 이동하는 범위.
			+ (float)m_nThumbRange / m_nSlideRange
			* --m_nSlidePos;	// * --슬라이드 위치.

		m_btnThumb.SetPosition((int)fThumbXPos, m_ptPos.y);
		m_pGaugeBar->SetValue(m_nSlidePos, m_nSlideRange);
	}
}

//*****************************************************************************
// 함수 이름 : LineDown()
// 함수 설명 : 슬라이드 위치를 하나 증가시키고 이에 따른 Thumb위치 변경.
//*****************************************************************************
void CSlider::LineDown()
{
	float fThumbPos;

	if (m_bVertical)	// 세로 슬라이더인가?
	{
		// 하나 증가 시킨 썸이 마지막 위치면.
		if (++m_nSlidePos == m_nSlideRange)
			fThumbPos = float(m_ptPos.y + m_nThumbRange);
		else
			// Thumb의 y위치 = 슬라이더의 위치
			fThumbPos = float(m_ptPos.y)
			// + Thumb이 한 칸 이동하는 범위.
				+ (float)m_nThumbRange / m_nSlideRange
				* m_nSlidePos;	// * 슬라이드 위치.
		// 위와 같이 하는 이유는 썸이 마지막 위치에 도달하면 갭이 생기는 것
		//을 방지 해준다.

		m_btnThumb.SetPosition(m_ptPos.x, (int)fThumbPos);
	}
	else				// 가로 슬라이더이면.
	{
		// 하나 증가 시킨 썸이 마지막 위치면.
		if (++m_nSlidePos == m_nSlideRange)
			fThumbPos = float(m_ptPos.x + m_nThumbRange);
		else
			// Thumb의 y위치 = 슬라이더의 위치
			fThumbPos = float(m_ptPos.x)
			// + Thumb이 한 칸 이동하는 범위.
				+ (float)m_nThumbRange / m_nSlideRange
				* m_nSlidePos;	// * 슬라이드 위치.
		// 위와 같이 하는 이유는 썸이 마지막 위치에 도달하면 갭이 생기는 것
		//을 방지 해준다.

		m_btnThumb.SetPosition((int)fThumbPos, m_ptPos.y);
		m_pGaugeBar->SetValue(m_nSlidePos, m_nSlideRange);
	}
}

//*****************************************************************************
// 함수 이름 : Update()
// 함수 설명 : 슬라이더의 자동 처리.
// 매개 변수 : dDeltaTick	: 이전 Update()호출 후부터 지금 Update()까지 시간.
//*****************************************************************************
void CSlider::Update(double dDeltaTick)
{
	if (!m_btnThumb.IsShow())	// 보이지 않으면 리턴.
		return;

#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
	m_btnThumb.Update();
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
	m_btnThumb.Update(dDeltaTick);
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING

	if (!m_btnThumb.IsEnable())	// 사용할 수 없으면 리턴.
		return;

	CInput& rInput = CInput::Instance();
	if (rInput.IsLBtnUp())	// 마우스 버튼을 놓았는가?
	{
		SetThumbPosition();
		m_byState = SLD_STATE_IDLE;
	}

// 썸의 좌측 또는 우측 영역 누르고 있을 때의 루틴.
	RECT rcUpperThumb, rcUnderThumb;
	if (m_bVertical)
	{
		// Thumb 위쪽 사각 영역.
		::SetRect(&rcUpperThumb, m_ptPos.x, m_ptPos.y,
			m_ptPos.x + m_Size.cx, m_btnThumb.GetYPos());
		// Thumb 아랫쪽 사각 영역.
		::SetRect(&rcUnderThumb, m_ptPos.x,
			m_btnThumb.GetYPos() + m_btnThumb.GetHeight(),
			m_ptPos.x + m_Size.cx, m_ptPos.y + m_Size.cy);
	}
	else
	{
		// Thumb 왼쪽 사각 영역.
		::SetRect(&rcUpperThumb, m_ptPos.x, m_ptPos.y,
			m_btnThumb.GetXPos(), m_ptPos.y + m_Size.cy);
		// Thumb 오른쪽 사각 영역.
		::SetRect(&rcUnderThumb, m_btnThumb.GetXPos() + m_btnThumb.GetWidth(),
			m_ptPos.y, m_ptPos.x + m_Size.cx, m_ptPos.y + m_Size.cy);
	}

	// 현재 커서의 위치.
	POINT ptCursor = rInput.GetCursorPos();
	// 커서가 Thumb 위쪽 사각 영역에 위치하는가?
	if (::PtInRect(&rcUpperThumb, ptCursor))
	{
		m_dThumbMoveStartTime += dDeltaTick;
		if (rInput.IsLBtnHeldDn())	// 버튼을 누루기 시작했는가?
		{
			LineUp();
			m_byState |= SLD_STATE_UP;
			m_dThumbMoveStartTime = 0.0;
		}

		// Thumb이 이동하기 시작한 후로 SLD_FIRST_SLIDE_DELAY_TIME 만큼 지났는
		//가?
		if (m_dThumbMoveStartTime >= SLD_FIRST_SLIDE_DELAY_TIME
			&& m_byState & SLD_STATE_UP)
		{
			m_dThumbMoveTime += dDeltaTick;

			// Up 버튼을 누루고 있을 때 Thumb의 이동을 지연 시키기 위해
			//SLD_SLIDE_DELAY_TIME 만큼 시간이 지날 때 마다 슬라이드.
			// 이 지연시간을 체크하지 않으면 순간적으로 최상단까지 이동해 버림.
			if (m_dThumbMoveTime >= SLD_SLIDE_DELAY_TIME)
			{
				LineUp();
				m_dThumbMoveTime = 0.0;
			}
		}
	}
	// 커서가 Thumb 아랫쪽 사각 영역에 위치하는가?
	else if (::PtInRect(&rcUnderThumb, ptCursor))
	{
		m_dThumbMoveStartTime += dDeltaTick;
		if (rInput.IsLBtnHeldDn())	// 버튼을 누루기 시작했는가?
		{
			LineDown();	// 우선 페이지를 하나 증가시킴.
			m_byState |= SLD_STATE_DN;
			m_dThumbMoveStartTime = 0.0;
		}

		// Thumb이 이동하기 시작한 후로 SLD_FIRST_SLIDE_DELAY_TIME 만큼 지났는
		//가?
		if (m_dThumbMoveStartTime >= SLD_FIRST_SLIDE_DELAY_TIME
			&& m_byState & SLD_STATE_DN)
		{
			m_dThumbMoveTime += dDeltaTick;

			// Up 버튼을 누루고 있을 때 Thumb의 이동을 지연 시키기 위해
			//SLD_SLIDE_DELAY_TIME 만큼 시간이 지날 때 마다 슬라이드.
			// 이 지연시간을 체크하지 않으면 순간적으로 최하단까지 이동해 버림.
			if (m_dThumbMoveTime >= SLD_SLIDE_DELAY_TIME)
			{
				LineDown();
				m_dThumbMoveTime = 0.0;
			}
		}
	}

// Thumb 드래그 처리.
	if (m_btnThumb.CursorInObject() && rInput.IsLBtnHeldDn())
	{
		if (m_bVertical)	// 세로 슬라이더인가?
		{
			// 현재 커서 위치 기억.
			m_nCapturePos = ptCursor.y;
			// Thumb이 가장 위에 있을 때의 마우스 커서 위치.
			// 가장 밑에 있을 때의 위치는 이 값에 m_nThumbRange를 더한 값.
			m_nLimitPos = m_ptPos.y + m_nCapturePos - m_btnThumb.GetYPos();
		}
		else				// 가로 슬라이더이면.
		{
			// 현재 커서 위치 기억.
			m_nCapturePos = ptCursor.x;
			// Thumb이 가장 왼쪽에 있을 때의 마우스 커서 위치.
			// 가장 오른쪽에 있을 때의 위치는 이 값에 m_nThumbRange를 더한 값.
			m_nLimitPos = m_ptPos.x + m_nCapturePos - m_btnThumb.GetXPos();
		}
		m_byState |= SLD_STATE_THUMB_DRG;
	}

	if (m_byState & SLD_STATE_THUMB_DRG)	// Thumb 드래그 중인가?
	{
		int nThumbPos;
		if (m_bVertical)	// 세로 슬라이더인가?
		{
		// Thumb의 화면 좌표 바꾸기 및 Thumb의 이동.
			// 현재 Thumb 위치가 Thumb 이동 구역 내에 있나?
			if (m_nLimitPos < ptCursor.y
				&& m_nLimitPos + m_nThumbRange > ptCursor.y)
			{
				// Thumb의 위치는 마우스 커서가 이동한 만큼 변경.
				nThumbPos = m_btnThumb.GetYPos()
					+ rInput.GetCursorY() - m_nCapturePos;
				// 현재 커서 위치를 이전 커서 위치에 기억.
				m_nCapturePos = ptCursor.y;
			}
			// 현재 Thumb 위치가 Thumb 이동 구역 위에 있는가?
			else if (m_nLimitPos >= ptCursor.y)
			{
				// Thumb의 위치는 최상단.
				nThumbPos = m_ptPos.y;
				m_nCapturePos = m_nLimitPos;
			}
			else	// 현재 Thumb 위치가 Thumb 이동 구역 아래에 있는가?
			{
				nThumbPos = m_ptPos.y + m_nThumbRange;
				m_nCapturePos = m_nLimitPos + m_nThumbRange;
			}
			m_btnThumb.SetPosition(m_btnThumb.GetXPos(), nThumbPos);
			// Thumb을 눌린 이미지로 바꿈.
//			m_btnThumb.SetAction(BTN_HIGHLIGHT_DOWN, BTN_HIGHLIGHT_DOWN);

		// m_nSlidePos 구하기.
			// Thumb이 한 칸 이동하는 범위.
			float fPixelPerPos = (float)m_nThumbRange / m_nSlideRange;
			m_nSlidePos = int((float(m_btnThumb.GetYPos() - m_ptPos.y)
				+ (fPixelPerPos / 2)) / fPixelPerPos);
		}
		else	// 가로 슬라이더이면.
		{
		// Thumb의 화면 좌표 바꾸기 및 Thumb의 이동.
			// 현재 Thumb 위치가 Thumb 이동 구역 내에 있나?
			if (m_nLimitPos < ptCursor.x
				&& m_nLimitPos + m_nThumbRange > ptCursor.x)
			{
				nThumbPos = m_btnThumb.GetXPos() + ptCursor.x - m_nCapturePos;
				// 현재 커서 위치를 이전 커서 위치에 기억.
				m_nCapturePos = ptCursor.x;
			}
			// 현재 Thumb 위치가 Thumb 이동 영역에서 가장 왼쪽에 있는가?
			else if (m_nLimitPos >= ptCursor.x)
			{
				// Thumb의 위치는 가장 왼쪽.
				nThumbPos = m_ptPos.x;
				m_nCapturePos = m_nLimitPos;
			}
			else	// 현재 Thumb 위치가 Thumb 이동 영역 가장 오른쪽에 있는가?
			{
				nThumbPos = m_ptPos.x + m_nThumbRange;
				m_nCapturePos = m_nLimitPos + m_nThumbRange;
			}
			m_btnThumb.SetPosition(nThumbPos, m_btnThumb.GetYPos());
			// Thumb을 눌린 이미지로 바꿈.
//			m_btnThumb.SetAction(BTN_HIGHLIGHT_DOWN, BTN_HIGHLIGHT_DOWN);

		// m_nSlidePos 구하기.
			// Up이나 Down버튼을 한 번 누를 때 Thumb이 이동하는 범위.
			float fPixelPerPos = (float)m_nThumbRange / m_nSlideRange;
			m_nSlidePos = int((float(m_btnThumb.GetXPos() - m_ptPos.x)
				+ (fPixelPerPos / 2)) / fPixelPerPos);

			m_pGaugeBar->SetValue(m_nSlidePos, m_nSlideRange);
		}	// if (m_bVertical) else문 끝.
	}
}

//*****************************************************************************
// 함수 이름 : Render()
// 함수 설명 : 랜더링.
//*****************************************************************************
void CSlider::Render()
{
	if (!m_btnThumb.IsShow())
		return;

	if (m_pGaugeBar)
		m_pGaugeBar->Render();
	else if (m_psprBack)
		m_psprBack->Render();
	m_btnThumb.Render();
}

//*****************************************************************************
// 함수 이름 : SetEnable()
// 함수 설명 : 슬라이더 사용 가능/불가능 선택.
// 매개 변수 : bEnable	: true이면 사용가능.
//*****************************************************************************
void CSlider::SetEnable(bool bEnable)
{
	m_btnThumb.SetEnable(bEnable);

	if (!bEnable)
		m_byState = SLD_STATE_IDLE;
}

//*****************************************************************************
// 함수 이름 : Show()
// 함수 설명 : 슬라이더 보임/안보임 선택.
// 매개 변수 : bEnable	: true이면 보여줌.
//*****************************************************************************
void CSlider::Show(bool bShow)
{
	m_btnThumb.Show(bShow);
	if (m_pGaugeBar)
		m_pGaugeBar->Show(bShow);
	else if (m_psprBack)
		m_psprBack->Show(bShow);
}

//*****************************************************************************
// 함수 이름 : CursorInSlider()
// 함수 설명 : 마우스 커서가 슬라이더 위에 있으면 TRUE 리턴.
//*****************************************************************************
BOOL CSlider::CursorInObject()
{
	if (m_btnThumb.IsShow())
	{
		RECT rcSlider = { m_ptPos.x, m_ptPos.y,
			m_ptPos.x + m_Size.cx, m_ptPos.y + m_Size.cy };
		return ::PtInRect(&rcSlider, CInput::Instance().GetCursorPos());
	}

	return FALSE;
}

//*****************************************************************************
// 함수 이름 : SetSlidePos()
// 함수 설명 : 슬라이더 값을 세팅.
// 매개 변수 : nSlidePos	: 슬라이더 값.
//*****************************************************************************
void CSlider::SetSlidePos(int nSlidePos)
{
	m_nSlidePos = LIMIT(nSlidePos, 0, m_nSlideRange);
	SetThumbPosition();
}