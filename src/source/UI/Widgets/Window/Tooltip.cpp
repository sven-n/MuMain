#include "stdafx.h"

#include "UI/Widgets/Window/Tooltip.h"
#include "UI/Core/WindowCommon.h"
#include "UI/Widgets/UIControls.h"
#include "I18N/All.h"

mu::ui::window::CTooltip::~CTooltip()
{
    if (m_localeObserverRegistered)
    {
        I18N::UnregisterLocaleObserver(&CTooltip::OnLocaleChanged, this);
    }
}

void mu::ui::window::CTooltip::SetText(std::wstring text)
{
    // Literal text overrides any prior slot binding.
    m_textSlot = nullptr;
    m_text = std::move(text);
}

void mu::ui::window::CTooltip::SetText(const wchar_t* const* textSlot)
{
    m_textSlot = textSlot;
    m_text = (textSlot != nullptr && *textSlot != nullptr) ? *textSlot : L"";
    EnsureLocaleObserver();
}

void mu::ui::window::CTooltip::SetTextColor(unsigned int color)
{
    m_textColor = color;
}

void mu::ui::window::CTooltip::SetFont(HFONT font)
{
    m_font = font;
}

void mu::ui::window::CTooltip::SetAnchorAbove(bool above)
{
    m_anchorAbove = above;
}

void mu::ui::window::CTooltip::EnsureLocaleObserver()
{
    if (m_localeObserverRegistered) return;
    I18N::RegisterLocaleObserver(&CTooltip::OnLocaleChanged, this);
    m_localeObserverRegistered = true;
}

void mu::ui::window::CTooltip::OnLocaleChanged(void* ctx) noexcept
{
    auto* self = static_cast<CTooltip*>(ctx);
    if (self->m_textSlot != nullptr && *self->m_textSlot != nullptr)
    {
        self->m_text = *self->m_textSlot;
    }
}

void mu::ui::window::CTooltip::Render(int x, int y, int width, int height, int offsetX, int offsetY) const
{
    if (m_text.empty())
    {
        return;
    }
    if (!CheckMouseIn(x, y, width, height))
    {
        return;
    }

    g_pRenderText->SetFont(m_font);
    const SIZE fontSize = g_pRenderText->MeasureText(m_text.c_str(), static_cast<int>(m_text.size()));

    int tipX = x + ((width / 2) - (fontSize.cx / 2));
    int tipY = y + height + 2;

    // Clamp so the tooltip's right edge never runs past the reference-resolution screen width.
    const int rightEdge = tipX + fontSize.cx + 6;
    if (rightEdge > REFERENCE_WIDTH)
    {
        tipX -= (rightEdge - REFERENCE_WIDTH);
    }

    if (m_anchorAbove)
    {
        tipY = y - (fontSize.cy + 2);
    }

    RenderTextWithColors(m_text.c_str(), tipX + offsetX, tipY + offsetY, fontSize.cx + 6, 0, m_font,
                         m_textColor, RGBA(0, 0, 0, 180), RT3_SORT_CENTER);
}
