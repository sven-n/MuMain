//*****************************************************************************
// File: Slider.h
//
// Desc: interface for the CSlider class.
//		 슬라이더 컨트롤 클래스.
//
// producer: Ahn Sang-Kyu
//*****************************************************************************

#if !defined(AFX_SLIDER_H__51DCB080_BB6D_4486_BD3E_6A0CA739DD85__INCLUDED_)
#define AFX_SLIDER_H__51DCB080_BB6D_4486_BD3E_6A0CA739DD85__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Button.h"

// m_byState멤버에 적용되는 값.
#define SLD_STATE_IDLE				0x00	// 아무것도 안함.
#define SLD_STATE_UP				0x01	// 위로.
#define SLD_STATE_DN				0x02	// 밑으로.
#define SLD_STATE_THUMB_DRG			0x10	// 썸 드래그.

class CGaugeBar;

class CSlider  
{
protected:
	bool		m_bVertical;		// 수직 슬라이더인지 여부.
	BYTE		m_byState;			// 작동 상태.

	POINT		m_ptPos;
	SIZE		m_Size;
	int			m_nSlideRange;		// 슬라이드 범위.
	int			m_nSlidePos;		// 현재 슬라이드 위치.
	int			m_nThumbRange;		// Thumb이 움직일 수 있는 범위.(픽셀 단위.)
	CButton		m_btnThumb;			// Thumb.
	CGaugeBar*	m_pGaugeBar;		// 게이지 바.(배경 이미지도 포함 됨.)
	CSprite*	m_psprBack;			// 배경 스프라이트.(게이지가 없고 배경 이미지가 따로 있는 경우)

	double		m_dThumbMoveStartTime;	// Thumb이 이동하기 시작한 시간.
	double		m_dThumbMoveTime;	// Thumb이 이동했을 때의 시간.
	int			m_nCapturePos;		// Thumb을 눌렀을 때 마우스 커서 위치.
	int			m_nLimitPos;		// Thumb이 가장 위에 있을 때 마우스 커서 위치.

public:
	CSlider();
	virtual ~CSlider();

	void Create(SImgInfo* piiThumb, SImgInfo* piiBack, SImgInfo* piiGauge = NULL,
		RECT* prcGauge = NULL, bool bVertical = false);
	void Release();
	void SetPosition(int nXCoord, int nYCoord);
	int GetXPos() { return m_ptPos.x; }
	int GetYPos() { return m_ptPos.y; }
	int GetWidth() { return m_Size.cx; }
	int GetHeight() { return m_Size.cy; }
	void SetSlideRange(int nSlideRange);
	void Update(double dDeltaTick);
	void Render();
	void SetEnable(bool bEnable);
	bool IsEnable() { return m_btnThumb.IsEnable(); }
	void Show(bool bShow);
	bool IsShow() { return m_btnThumb.IsShow(); }
	BOOL CursorInObject();

	void SetSlidePos(int nSlidePos);
	int GetSlidePos() { return m_nSlidePos; }
	BYTE GetState() { return m_byState; }

protected:
	void SetThumbPosition();
	void LineUp();
	void LineDown();
};

#endif // !defined(AFX_SLIDER_H__51DCB080_BB6D_4486_BD3E_6A0CA739DD85__INCLUDED_)
