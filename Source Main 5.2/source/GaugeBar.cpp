//*****************************************************************************
// File: GaugeBar.cpp
//
// Desc: implementation of the CGaugeBar class.
//
// producer: Ahn Sang-Kyu
//*****************************************************************************

#include "stdafx.h"

#include <algorithm>
#include <cstdint>

#include "GaugeBar.h"
#include "Input.h"
#include "UsefulDef.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGaugeBar::CGaugeBar()
    : m_gaugeRect{0, 0, 0, 0}
{
}

CGaugeBar::~CGaugeBar()
{
    Release();
}

void CGaugeBar::Create(int nGaugeWidth, int nGaugeHeight, int nGaugeTexID, const RECT* prcGauge, int nBackWidth, int nBackHeight, int nBackTexID, bool bShortenLeft, float fScaleX, float fScaleY)
{
    Release();

    const int nSizingDatums = bShortenLeft ? SPR_SIZING_DATUMS_LT : SPR_SIZING_DATUMS_RT;

    m_sprGauge.Create(nGaugeWidth, nGaugeHeight, nGaugeTexID, 0, nullptr, 0, 0,
        true, nSizingDatums, fScaleX, fScaleY);

    if (prcGauge == nullptr)
    {
        m_gaugeRect = Rect{0, 0, nGaugeWidth, nGaugeHeight};
    }
    else
    {
        m_gaugeRect = ConvertRect(*prcGauge);
        m_sprGauge.SetSize(0, m_gaugeRect.bottom - m_gaugeRect.top, Y);
    }

    if (nBackTexID >= 0)
    {
        m_backgroundSprite = std::make_unique<CSprite>();
        m_backgroundSprite->Create(nBackWidth, nBackHeight, nBackTexID, 0, nullptr, 0, 0, false, SPR_SIZING_DATUMS_LT, fScaleX, fScaleY);
        m_responseSize.reset();
    }
    else if (nBackWidth > 0 && nBackHeight > 0)
    {
        m_responseSize = GaugeSize{nBackWidth, nBackHeight};
        m_backgroundSprite.reset();
    }
    else
    {
        m_responseSize.reset();
        m_backgroundSprite.reset();
    }

    m_nXPos = 0;
    m_nYPos = 0;
}

void CGaugeBar::Release()
{
    m_sprGauge.Release();
    m_backgroundSprite.reset();
    m_responseSize.reset();
}

void CGaugeBar::SetPosition(int nXCoord, int nYCoord)
{
    if (m_backgroundSprite)
    {
        m_backgroundSprite->SetPosition(nXCoord, nYCoord);
    }

    m_sprGauge.SetPosition(nXCoord + m_gaugeRect.left, nYCoord + m_gaugeRect.top);

    m_nXPos = nXCoord;
    m_nYPos = nYCoord;
}

int CGaugeBar::GetWidth() const
{
    if (m_backgroundSprite)
    {
        return m_backgroundSprite->GetWidth();
    }

    if (m_responseSize)
    {
        return m_responseSize->width;
    }

    return m_gaugeRect.right - m_gaugeRect.left;
}

int CGaugeBar::GetHeight() const
{
    if (m_backgroundSprite)
    {
        return m_backgroundSprite->GetHeight();
    }

    if (m_responseSize)
    {
        return m_responseSize->height;
    }

    return m_gaugeRect.bottom - m_gaugeRect.top;
}

void CGaugeBar::SetValue(std::uint32_t dwNow, std::uint32_t dwTotal)
{
    const std::uint32_t safeTotal = std::max<std::uint32_t>(dwTotal, 1u);
    const std::uint32_t clampedNow = std::min<std::uint32_t>(dwNow, safeTotal);
    const int gaugeWidth = m_gaugeRect.right - m_gaugeRect.left;

    int nNowSize = static_cast<int>((static_cast<std::uint64_t>(clampedNow) * gaugeWidth) / safeTotal);

    if (IS_SIZING_DATUMS_R(m_sprGauge.GetSizingDatums()))
    {
        nNowSize
            += m_sprGauge.GetTexWidth() - gaugeWidth;
    }

    m_sprGauge.SetSize(nNowSize, 0, X);
}

bool CGaugeBar::CursorInObject()
{
    if (!IsShow())
    {
        return false;
    }

    if (m_backgroundSprite)
    {
        return m_backgroundSprite->CursorInObject() != FALSE;
    }

    float fScaleX = m_sprGauge.GetScaleX();
    float fScaleY = m_sprGauge.GetScaleY();

    CInput& rInput = CInput::Instance();

    if (m_responseSize)
    {
        Rect scaledRect{0, 0, m_responseSize->width, m_responseSize->height};
        scaledRect = ScaleRect(scaledRect, fScaleX, fScaleY);
        scaledRect = OffsetRect(scaledRect, m_nXPos, m_nYPos);
        const auto cursorPos = rInput.GetCursorPos();
        return Contains(scaledRect, cursorPos.x, cursorPos.y);
    }

    Rect scaledRect = ScaleRect(m_gaugeRect, fScaleX, fScaleY);
    scaledRect = OffsetRect(scaledRect, m_nXPos, m_nYPos);
    const auto cursorPos = rInput.GetCursorPos();
    return Contains(scaledRect, cursorPos.x, cursorPos.y);
}

void CGaugeBar::SetAlpha(std::uint8_t byAlpha)
{
    if (m_backgroundSprite)
    {
        m_backgroundSprite->SetAlpha(static_cast<BYTE>(byAlpha));
    }

    m_sprGauge.SetAlpha(static_cast<BYTE>(byAlpha));
}

void CGaugeBar::SetColor(std::uint8_t byRed, std::uint8_t byGreen, std::uint8_t byBlue)
{
    m_sprGauge.SetColor(static_cast<BYTE>(byRed), static_cast<BYTE>(byGreen), static_cast<BYTE>(byBlue));
}

void CGaugeBar::Show(bool bShow)
{
    if (m_backgroundSprite)
    {
        m_backgroundSprite->Show(bShow);
    }
    m_sprGauge.Show(bShow);
}

void CGaugeBar::Render()
{
    if (m_backgroundSprite)
    {
        m_backgroundSprite->Render();
    }
    m_sprGauge.Render();
}

CGaugeBar::Rect CGaugeBar::ConvertRect(const RECT& source)
{
    return Rect{source.left, source.top, source.right, source.bottom};
}

CGaugeBar::Rect CGaugeBar::ScaleRect(const Rect& rect, float scaleX, float scaleY)
{
    return Rect{
        static_cast<int>(rect.left * scaleX),
        static_cast<int>(rect.top * scaleY),
        static_cast<int>(rect.right * scaleX),
        static_cast<int>(rect.bottom * scaleY)};
}

CGaugeBar::Rect CGaugeBar::OffsetRect(const Rect& rect, int offsetX, int offsetY)
{
    return Rect{
        rect.left + offsetX,
        rect.top + offsetY,
        rect.right + offsetX,
        rect.bottom + offsetY};
}

bool CGaugeBar::Contains(const Rect& rect, long x, long y)
{
    return (x >= rect.left && x < rect.right && y >= rect.top && y < rect.bottom);
}