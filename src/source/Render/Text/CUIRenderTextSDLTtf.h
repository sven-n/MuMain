#pragma once


#include "UI/Legacy/UIControls.h"

#include <string>

struct TTF_Font;
struct TTF_Text;

class CUIRenderTextSDLTtf final : public IUIRenderText
{
public:
    CUIRenderTextSDLTtf();
    ~CUIRenderTextSDLTtf() override;

    bool Create(HDC hDC) override;
    void Release() override;

    DWORD GetTextColor() const override;
    DWORD GetBgColor() const override;

    void SetTextColor(BYTE red, BYTE green, BYTE blue, BYTE alpha) override;
    void SetTextColor(DWORD color) override;
    void SetBgColor(BYTE red, BYTE green, BYTE blue, BYTE alpha) override;
    void SetBgColor(DWORD color) override;
    void SetFont(HFONT font) override;
    SIZE MeasureText(const wchar_t* text, int length) const override;

    void RenderText(int x, int y, const wchar_t* text, int boxWidth = 0, int boxHeight = 0, int sort = RT3_SORT_LEFT,
                    OUT SIZE* textSize = nullptr) override;

private:
    DWORD m_textColor = 0xFFFFFFFFu;
    DWORD m_backColor = 0u;
    TTF_Font* m_activeFont = nullptr;
    TTF_Text* m_ttfText = nullptr;
    std::string m_utf8Scratch;
    mutable std::wstring m_measureWideScratch;
    mutable std::string m_measureUtf8Scratch;
};
