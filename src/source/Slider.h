//*****************************************************************************
// File: Slider.h
//*****************************************************************************

#pragma once

#include "Button.h"

#define SLD_STATE_IDLE				0x00
#define SLD_STATE_UP				0x01
#define SLD_STATE_DN				0x02
#define SLD_STATE_THUMB_DRG			0x10

class CGaugeBar;

class CSlider
{
protected:
    bool		m_bVertical;
    BYTE		m_byState;

    POINT		m_ptPos;
    SIZE		m_Size;
    int			m_nSlideRange;
    int			m_nSlidePos;
    int			m_nThumbRange;
    CButton		m_btnThumb;
    CGaugeBar* m_pGaugeBar;
    CSprite* m_psprBack;

    double		m_dThumbMoveStartTime;
    double		m_dThumbMoveTime;
    int			m_nCapturePos;
    int			m_nLimitPos;

public:
    CSlider();
    virtual ~CSlider();

    void Create(SImgInfo* piiThumb, SImgInfo* piiBack, SImgInfo* piiGauge = NULL, RECT* prcGauge = NULL, bool bVertical = false);
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
