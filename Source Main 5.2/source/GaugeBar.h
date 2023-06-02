//*****************************************************************************
// File: GaugeBar.h
//*****************************************************************************

#pragma once

#include "Sprite.h"

class CGaugeBar
{
protected:
    CSprite		m_sprGauge;
    RECT		m_rcGauge;
    CSprite* m_psprBack;
    SIZE* m_psizeResponse;
    int			m_nXPos;
    int			m_nYPos;

public:
    CGaugeBar();
    virtual ~CGaugeBar();

    void Create(int nGaugeWidth, int nGaugeHeight, int nGaugeTexID,
        RECT* prcGauge = NULL, int nBackWidth = 0, int nBackHeight = 0,
        int nBackTexID = -1, bool bShortenLeft = true, float fScaleX = 1.0f,
        float fScaleY = 1.0f);

    void Release();
    void SetPosition(int nXCoord, int nYCoord);
    int	GetXPos() { return m_nXPos; }
    int	GetYPos() { return m_nYPos; }
    int GetWidth();
    int GetHeight();
    void SetValue(DWORD dwNow, DWORD dwTotal);
    BOOL CursorInObject();
    void SetAlpha(BYTE dwAlpha);
    void SetColor(BYTE byRed, BYTE byGreen, BYTE byBlue);
    void Show(bool bShow = true);
    bool IsShow() { return m_sprGauge.IsShow(); };
    void Render();
};
