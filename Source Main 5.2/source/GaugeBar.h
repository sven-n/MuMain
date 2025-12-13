//*****************************************************************************
// File: GaugeBar.h
//*****************************************************************************

#pragma once

#include <cstdint>
#include <memory>
#include <optional>

#include "Sprite.h"

class CGaugeBar
{
protected:
    struct Rect
    {
        int left{0};
        int top{0};
        int right{0};
        int bottom{0};
    };

    struct GaugeSize
    {
        int width{0};
        int height{0};
    };

    CSprite		m_sprGauge;
    Rect        m_gaugeRect;
    std::unique_ptr<CSprite> m_backgroundSprite;
    std::optional<GaugeSize> m_responseSize;
    int			m_nXPos{0};
    int			m_nYPos{0};

public:
    CGaugeBar();
    virtual ~CGaugeBar();

    void Create(int nGaugeWidth, int nGaugeHeight, int nGaugeTexID,
        const RECT* prcGauge = nullptr, int nBackWidth = 0, int nBackHeight = 0,
        int nBackTexID = -1, bool bShortenLeft = true, float fScaleX = 1.0f,
        float fScaleY = 1.0f);

    void Release();
    void SetPosition(int nXCoord, int nYCoord);
    int	GetXPos() const { return m_nXPos; }
    int	GetYPos() const { return m_nYPos; }
    int GetWidth() const;
    int GetHeight() const;
    void SetValue(std::uint32_t dwNow, std::uint32_t dwTotal);
    bool CursorInObject();
    void SetAlpha(std::uint8_t dwAlpha);
    void SetColor(std::uint8_t byRed, std::uint8_t byGreen, std::uint8_t byBlue);
    void Show(bool bShow = true);
    bool IsShow() const { return m_sprGauge.IsShow(); };
    void Render();

private:
    static Rect ConvertRect(const RECT& source);
    static Rect ScaleRect(const Rect& rect, float scaleX, float scaleY);
    static Rect OffsetRect(const Rect& rect, int offsetX, int offsetY);
    static bool Contains(const Rect& rect, long x, long y);
};
