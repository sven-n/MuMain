//*****************************************************************************
// File: Slider.cpp
//*****************************************************************************

#include "stdafx.h"
#include "Slider.h"
#include "GaugeBar.h"
#include "Input.h"

#define SLD_FIRST_SLIDE_DELAY_TIME	500
#define SLD_SLIDE_DELAY_TIME		50

CSlider::CSlider() : m_pGaugeBar(NULL), m_psprBack(NULL)
{
}

CSlider::~CSlider()
{
    SAFE_DELETE(m_pGaugeBar);
    SAFE_DELETE(m_psprBack);
}

void CSlider::Create(SImgInfo* piiThumb, SImgInfo* piiBack, SImgInfo* piiGauge, RECT* prcGauge, bool bVertical)
{
    m_btnThumb.Create(piiThumb->nWidth, piiThumb->nHeight, piiThumb->nTexID);

    SAFE_DELETE(m_pGaugeBar);
    SAFE_DELETE(m_psprBack);

    if (piiGauge)
    {
        m_pGaugeBar = new CGaugeBar;
        m_pGaugeBar->Create(piiGauge->nWidth, piiGauge->nHeight, piiGauge->nTexID, prcGauge, piiBack->nWidth, piiBack->nHeight, piiBack->nTexID);
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
    if (m_bVertical)
    {
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
    m_nThumbRange = nThumbRange < 0 ? 0 : nThumbRange;
}

void CSlider::Release()
{
    m_btnThumb.Release();
    SAFE_DELETE(m_pGaugeBar);
    SAFE_DELETE(m_psprBack);
}

void CSlider::SetThumbPosition()
{
    float fThumbPos;
    if (m_bVertical)
    {
        fThumbPos = float(m_ptPos.y) + (float)m_nThumbRange / m_nSlideRange * m_nSlidePos;
        m_btnThumb.SetPosition(m_ptPos.x, (int)fThumbPos);
    }
    else
    {
        fThumbPos = float(m_ptPos.x) + (float)m_nThumbRange / m_nSlideRange * m_nSlidePos;
        m_btnThumb.SetPosition((int)fThumbPos, m_ptPos.y);
        m_pGaugeBar->SetValue(m_nSlidePos, m_nSlideRange);
    }
}

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

void CSlider::SetSlideRange(int nSlideRange)
{
    m_nSlideRange = MAX(nSlideRange, 1);

    SetThumbPosition();
}

void CSlider::LineUp()
{
    if (m_bVertical)
    {
        float fThumbYPos = float(m_ptPos.y) + (float)m_nThumbRange / m_nSlideRange * --m_nSlidePos;
        m_btnThumb.SetPosition(m_ptPos.x, (int)fThumbYPos);
    }
    else
    {
        float fThumbXPos = float(m_ptPos.x) + (float)m_nThumbRange / m_nSlideRange * --m_nSlidePos;
        m_btnThumb.SetPosition((int)fThumbXPos, m_ptPos.y);
        m_pGaugeBar->SetValue(m_nSlidePos, m_nSlideRange);
    }
}

void CSlider::LineDown()
{
    float fThumbPos;

    if (m_bVertical)
    {
        if (++m_nSlidePos == m_nSlideRange)
            fThumbPos = float(m_ptPos.y + m_nThumbRange);
        else
            fThumbPos = float(m_ptPos.y) + (float)m_nThumbRange / m_nSlideRange * m_nSlidePos;

        m_btnThumb.SetPosition(m_ptPos.x, (int)fThumbPos);
    }
    else
    {
        if (++m_nSlidePos == m_nSlideRange)	fThumbPos = float(m_ptPos.x + m_nThumbRange);
        else
            fThumbPos = float(m_ptPos.x) + (float)m_nThumbRange / m_nSlideRange * m_nSlidePos;

        m_btnThumb.SetPosition((int)fThumbPos, m_ptPos.y);
        m_pGaugeBar->SetValue(m_nSlidePos, m_nSlideRange);
    }
}

void CSlider::Update(double dDeltaTick)
{
    if (!m_btnThumb.IsShow())
        return;

    m_btnThumb.Update();

    if (!m_btnThumb.IsEnable())
        return;

    CInput& rInput = CInput::Instance();
    if (rInput.IsLBtnUp())
    {
        SetThumbPosition();
        m_byState = SLD_STATE_IDLE;
    }

    RECT rcUpperThumb, rcUnderThumb;
    if (m_bVertical)
    {
        ::SetRect(&rcUpperThumb, m_ptPos.x, m_ptPos.y, m_ptPos.x + m_Size.cx, m_btnThumb.GetYPos());
        ::SetRect(&rcUnderThumb, m_ptPos.x, m_btnThumb.GetYPos() + m_btnThumb.GetHeight(), m_ptPos.x + m_Size.cx, m_ptPos.y + m_Size.cy);
    }
    else
    {
        ::SetRect(&rcUpperThumb, m_ptPos.x, m_ptPos.y, m_btnThumb.GetXPos(), m_ptPos.y + m_Size.cy);
        ::SetRect(&rcUnderThumb, m_btnThumb.GetXPos() + m_btnThumb.GetWidth(), m_ptPos.y, m_ptPos.x + m_Size.cx, m_ptPos.y + m_Size.cy);
    }

    POINT ptCursor = rInput.GetCursorPos();

    if (::PtInRect(&rcUpperThumb, ptCursor))
    {
        m_dThumbMoveStartTime += dDeltaTick;
        if (rInput.IsLBtnHeldDn())
        {
            LineUp();
            m_byState |= SLD_STATE_UP;
            m_dThumbMoveStartTime = 0.0;
        }

        if (m_dThumbMoveStartTime >= SLD_FIRST_SLIDE_DELAY_TIME && m_byState & SLD_STATE_UP)
        {
            m_dThumbMoveTime += dDeltaTick;

            if (m_dThumbMoveTime >= SLD_SLIDE_DELAY_TIME)
            {
                LineUp();
                m_dThumbMoveTime = 0.0;
            }
        }
    }
    else if (::PtInRect(&rcUnderThumb, ptCursor))
    {
        m_dThumbMoveStartTime += dDeltaTick;
        if (rInput.IsLBtnHeldDn())
        {
            LineDown();
            m_byState |= SLD_STATE_DN;
            m_dThumbMoveStartTime = 0.0;
        }

        if (m_dThumbMoveStartTime >= SLD_FIRST_SLIDE_DELAY_TIME && m_byState & SLD_STATE_DN)
        {
            m_dThumbMoveTime += dDeltaTick;

            if (m_dThumbMoveTime >= SLD_SLIDE_DELAY_TIME)
            {
                LineDown();
                m_dThumbMoveTime = 0.0;
            }
        }
    }

    if (m_btnThumb.CursorInObject() && rInput.IsLBtnHeldDn())
    {
        if (m_bVertical)
        {
            m_nCapturePos = ptCursor.y;
            m_nLimitPos = m_ptPos.y + m_nCapturePos - m_btnThumb.GetYPos();
        }
        else
        {
            m_nCapturePos = ptCursor.x;
            m_nLimitPos = m_ptPos.x + m_nCapturePos - m_btnThumb.GetXPos();
        }
        m_byState |= SLD_STATE_THUMB_DRG;
    }

    if (m_byState & SLD_STATE_THUMB_DRG)
    {
        int nThumbPos;
        if (m_bVertical)
        {
            if (m_nLimitPos < ptCursor.y && m_nLimitPos + m_nThumbRange > ptCursor.y)
            {
                nThumbPos = m_btnThumb.GetYPos() + rInput.GetCursorY() - m_nCapturePos;
                m_nCapturePos = ptCursor.y;
            }
            else if (m_nLimitPos >= ptCursor.y)
            {
                nThumbPos = m_ptPos.y;
                m_nCapturePos = m_nLimitPos;
            }
            else
            {
                nThumbPos = m_ptPos.y + m_nThumbRange;
                m_nCapturePos = m_nLimitPos + m_nThumbRange;
            }
            m_btnThumb.SetPosition(m_btnThumb.GetXPos(), nThumbPos);
            //			m_btnThumb.SetAction(BTN_HIGHLIGHT_DOWN, BTN_HIGHLIGHT_DOWN);

                    // m_nSlidePos 구하기.
            float fPixelPerPos = (float)m_nThumbRange / m_nSlideRange;
            m_nSlidePos = int((float(m_btnThumb.GetYPos() - m_ptPos.y) + (fPixelPerPos / 2)) / fPixelPerPos);
        }
        else
        {
            if (m_nLimitPos < ptCursor.x && m_nLimitPos + m_nThumbRange > ptCursor.x)
            {
                nThumbPos = m_btnThumb.GetXPos() + ptCursor.x - m_nCapturePos;
                m_nCapturePos = ptCursor.x;
            }
            else if (m_nLimitPos >= ptCursor.x)
            {
                nThumbPos = m_ptPos.x;
                m_nCapturePos = m_nLimitPos;
            }
            else
            {
                nThumbPos = m_ptPos.x + m_nThumbRange;
                m_nCapturePos = m_nLimitPos + m_nThumbRange;
            }
            m_btnThumb.SetPosition(nThumbPos, m_btnThumb.GetYPos());
            //			m_btnThumb.SetAction(BTN_HIGHLIGHT_DOWN, BTN_HIGHLIGHT_DOWN);

            float fPixelPerPos = (float)m_nThumbRange / m_nSlideRange;
            m_nSlidePos = int((float(m_btnThumb.GetXPos() - m_ptPos.x) + (fPixelPerPos / 2)) / fPixelPerPos);
            m_pGaugeBar->SetValue(m_nSlidePos, m_nSlideRange);
        }	// if (m_bVertical) else문 끝.
    }
}

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

void CSlider::SetEnable(bool bEnable)
{
    m_btnThumb.SetEnable(bEnable);

    if (!bEnable)
        m_byState = SLD_STATE_IDLE;
}

void CSlider::Show(bool bShow)
{
    m_btnThumb.Show(bShow);
    if (m_pGaugeBar)
        m_pGaugeBar->Show(bShow);
    else if (m_psprBack)
        m_psprBack->Show(bShow);
}

BOOL CSlider::CursorInObject()
{
    if (m_btnThumb.IsShow())
    {
        RECT rcSlider = { m_ptPos.x, m_ptPos.y,	m_ptPos.x + m_Size.cx, m_ptPos.y + m_Size.cy };
        return ::PtInRect(&rcSlider, CInput::Instance().GetCursorPos());
    }

    return FALSE;
}

void CSlider::SetSlidePos(int nSlidePos)
{
    m_nSlidePos = LIMIT(nSlidePos, 0, m_nSlideRange);
    SetThumbPosition();
}