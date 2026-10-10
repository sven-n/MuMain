#pragma once

#include <memory>

// The engine's native text renderer: the interface its backend implements, and the single
// instance every native draw reaches through g_pRenderText. The only backend is
// CUIRenderTextSDLTtf (Render/Text/CUIRenderTextSDLTtf.h).
class IUIRenderText
{
public:
    virtual ~IUIRenderText() = default;

    virtual bool Create(HDC hDC) = 0;
    virtual void Release() = 0;

    virtual DWORD GetTextColor() const = 0;
    virtual DWORD GetBgColor() const = 0;

    virtual void SetTextColor(BYTE byRed, BYTE byGreen, BYTE byBlue, BYTE byAlpha) = 0;
    virtual void SetTextColor(DWORD dwColor) = 0;
    virtual void SetBgColor(BYTE byRed, BYTE byGreen, BYTE byBlue, BYTE byAlpha) = 0;
    virtual void SetBgColor(DWORD dwColor) = 0;

    virtual void SetFont(HFONT hFont) = 0;
    virtual SIZE MeasureText(const wchar_t* pszText, int iLength) const = 0;

    virtual void RenderText(int iPos_x, int iPos_y, const wchar_t* pszText, int iBoxWidth = 0, int iBoxHeight = 0,
                            int iSort = RT3_SORT_LEFT, OUT SIZE* lpTextSize = NULL) = 0;
};

class CUIRenderText
{
    CUIRenderText();

    std::unique_ptr<IUIRenderText> m_pRenderText;

public:
    virtual ~CUIRenderText();

    static CUIRenderText* GetInstance();

    bool Create(HDC hDC);
    void Release();

    DWORD GetTextColor() const;
    DWORD GetBgColor() const;

    void SetTextColor(BYTE byRed, BYTE byGreen, BYTE byBlue, BYTE byAlpha);
    void SetTextColor(DWORD dwColor);
    void SetBgColor(BYTE byRed, BYTE byGreen, BYTE byBlue, BYTE byAlpha);
    void SetBgColor(DWORD dwColor);

    void SetFont(HFONT hFont);
    SIZE MeasureText(const wchar_t* pszText, int iLength) const;

    void RenderText(int iPos_x, int iPos_y, const wchar_t* pszText, int iBoxWidth = 0, int iBoxHeight = 0,
                    int iSort = RT3_SORT_LEFT, OUT SIZE* lpTextSize = NULL);
};

#define g_pRenderText CUIRenderText::GetInstance()
