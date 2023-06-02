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

void CGaugeBar::Create(int nGaugeWidth, int nGaugeHeight, int nGaugeTexID, RECT* prcGauge, int nBackWidth, int nBackHeight, int nBackTexID, bool bShortenLeft, float fScaleX, float fScaleY)
{
    Release();

    int nSizingDatums = bShortenLeft ? SPR_SIZING_DATUMS_LT : SPR_SIZING_DATUMS_RT;

    m_sprGauge.Create(nGaugeWidth, nGaugeHeight, nGaugeTexID, 0, NULL, 0, 0,
        true, nSizingDatums, fScaleX, fScaleY);

    if (NULL == prcGauge)
    {
        RECT rc = { 0, 0, nGaugeWidth, nGaugeHeight };
        m_rcGauge = rc;
    }
    else
    {
        m_rcGauge = *prcGauge;
        m_sprGauge.SetSize(0, m_rcGauge.bottom - m_rcGauge.top, Y);
    }

    if (-1 < nBackTexID)
    {
        m_psprBack = new CSprite;
        m_psprBack->Create(nBackWidth, nBackHeight, nBackTexID, 0, NULL, 0, 0, false, SPR_SIZING_DATUMS_LT, fScaleX, fScaleY);
    }
    else if (0 < nBackWidth && 0 < nBackHeight)
    {
        m_psizeResponse = new SIZE;
        m_psizeResponse->cx = nBackWidth;
        m_psizeResponse->cy = nBackHeight;
    }

    m_nXPos = m_nYPos = 0;
}

void CGaugeBar::Release()
{
    m_sprGauge.Release();
    if (m_psprBack) { delete m_psprBack;	m_psprBack = NULL; }
    else if (m_psizeResponse) { delete m_psizeResponse;	m_psizeResponse = NULL; }
}

void CGaugeBar::SetPosition(int nXCoord, int nYCoord)
{
    if (m_psprBack)
        m_psprBack->SetPosition(nXCoord, nYCoord);

    m_sprGauge.SetPosition(nXCoord + m_rcGauge.left, nYCoord + m_rcGauge.top);

    m_nXPos = nXCoord;
    m_nYPos = nYCoord;
}

int CGaugeBar::GetWidth()
{
    if (m_psprBack)
        return m_psprBack->GetWidth();

    if (m_psizeResponse)
        return m_psizeResponse->cx;

    return m_rcGauge.right - m_rcGauge.left;
}

int CGaugeBar::GetHeight()
{
    if (m_psprBack)
        return m_psprBack->GetHeight();

    if (m_psizeResponse)
        return m_psizeResponse->cy;

    return m_rcGauge.bottom - m_rcGauge.top;
}

void CGaugeBar::SetValue(DWORD dwNow, DWORD dwTotal)
{
    dwTotal = (dwTotal < 1) ? 1 : dwTotal;
    dwNow = LIMIT(dwNow, 0, dwTotal);

    int nNowSize = dwNow * (m_rcGauge.right - m_rcGauge.left) / dwTotal;

    if (IS_SIZING_DATUMS_R(m_sprGauge.GetSizingDatums()))
    {
        nNowSize
            += m_sprGauge.GetTexWidth() - (m_rcGauge.right - m_rcGauge.left);
    }

    m_sprGauge.SetSize(nNowSize, 0, X);
}

BOOL CGaugeBar::CursorInObject()
{
    if (!IsShow())
        return FALSE;

    if (m_psprBack)
        return m_psprBack->CursorInObject();

    float fScaleX = m_sprGauge.GetScaleX();
    float fScaleY = m_sprGauge.GetScaleY();

    CInput& rInput = CInput::Instance();

    if (m_psizeResponse)
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

void CGaugeBar::SetAlpha(BYTE byAlpha)
{
    if (m_psprBack)
        m_psprBack->SetAlpha(byAlpha);

    m_sprGauge.SetAlpha(byAlpha);
}

void CGaugeBar::SetColor(BYTE byRed, BYTE byGreen, BYTE byBlue)
{
    m_sprGauge.SetColor(byRed, byGreen, byBlue);
}

void CGaugeBar::Show(bool bShow)
{
    if (m_psprBack)
        m_psprBack->Show(bShow);
    m_sprGauge.Show(bShow);
}

void CGaugeBar::Render()
{
    if (m_psprBack)
        m_psprBack->Render();
    m_sprGauge.Render();
}