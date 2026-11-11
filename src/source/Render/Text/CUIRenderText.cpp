#include "stdafx.h"
#include "Render/Text/CUIRenderText.h"

#include "Render/Text/CUIRenderTextSDLTtf.h"

CUIRenderText::CUIRenderText() = default;
CUIRenderText::~CUIRenderText() = default;

CUIRenderText* CUIRenderText::GetInstance()
{
    static CUIRenderText s_RenderText;
    return &s_RenderText;
}

bool CUIRenderText::Create(HDC hDC)
{
    if (m_pRenderText)
    {
        return true;
    }

    m_pRenderText = std::make_unique<CUIRenderTextSDLTtf>();
    if (!m_pRenderText->Create(hDC))
    {
        m_pRenderText.reset();
        return false;
    }
    return true;
}

void CUIRenderText::Release()
{
    m_pRenderText.reset();
}

DWORD CUIRenderText::GetTextColor() const
{
    if (m_pRenderText)
        return m_pRenderText->GetTextColor();
    return 0;
}
DWORD CUIRenderText::GetBgColor() const
{
    if (m_pRenderText)
        return m_pRenderText->GetBgColor();
    return 0;
}

void CUIRenderText::SetTextColor(BYTE byRed, BYTE byGreen, BYTE byBlue, BYTE byAlpha)
{
    if (m_pRenderText)
        m_pRenderText->SetTextColor(byRed, byGreen, byBlue, byAlpha);
}
void CUIRenderText::SetTextColor(DWORD dwColor)
{
    if (m_pRenderText)
        m_pRenderText->SetTextColor(dwColor);
}
void CUIRenderText::SetBgColor(BYTE byRed, BYTE byGreen, BYTE byBlue, BYTE byAlpha)
{
    if (m_pRenderText)
        m_pRenderText->SetBgColor(byRed, byGreen, byBlue, byAlpha);
}
void CUIRenderText::SetBgColor(DWORD dwColor)
{
    if (m_pRenderText)
        m_pRenderText->SetBgColor(dwColor);
}

void CUIRenderText::SetFont(HFONT hFont)
{
    if (m_pRenderText)
        m_pRenderText->SetFont(hFont);
}

SIZE CUIRenderText::MeasureText(const wchar_t* pszText, int iLength) const
{
    return m_pRenderText ? m_pRenderText->MeasureText(pszText, iLength) : SIZE{0, 0};
}

void CUIRenderText::RenderText(int iPos_x, int iPos_y, const wchar_t* pszText, int iBoxWidth /* = 0 */, int iBoxHeight /* = 0 */, int iSort /* = RT3_SORT_LEFT */, OUT SIZE* lpTextSize /* = NULL */)
{
    if (m_pRenderText)
    {
        m_pRenderText->RenderText(iPos_x, iPos_y, pszText, iBoxWidth, iBoxHeight, iSort, lpTextSize);
    }
}
