#include "stdafx.h"

#include "UI/Widgets/Window/Tooltip.h"
#include "UI/Core/WindowCommon.h"
#include "UI/Widgets/UIControls.h"
#include "UI/RmlBridge/RmlTooltip.h"
#include "UI/Scaling/UITransform.h"
#include "Core/Utilities/StringUtils.h"
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
        UI::RmlBridge::Tooltip::Hide(this);
        return;
    }
    if (!CheckMouseIn(x, y, width, height))
    {
        UI::RmlBridge::Tooltip::Hide(this);
        return;
    }

    // m_font/m_textColor are unused here -- zero of this class's 38 real call sites across the
    // codebase ever call SetTextColor()/SetFont() with anything but the class's own defaults, so
    // the shared tooltip's own unified white/themed styling already reproduces every actual use.
    // Both setters are kept for API compatibility, not because anything currently reads them.
    // x/y/width/height/offsetX/offsetY are reference-pixel, in the caller's own ambient scope --
    // same convention as CharacterInfoWindow's root_x/root_y conversion. Tooltip::Show() takes
    // already-converted real screen pixels (see RmlTooltip.h's own comment for why), so convert here.
    const UI::Scaling::Transform activeTransform = UI::Scaling::GetActiveTransform();

    UI::RmlBridge::Tooltip::Config config;
    UI::RmlBridge::Tooltip::Line line;
    line.text = StringUtils::WideToNarrow(m_text.c_str());
    config.lines.push_back(std::move(line));

    config.anchorX = UI::Scaling::PositionX(activeTransform, static_cast<float>(x + width / 2 + offsetX));
    config.centerHorizontally = true;
    config.anchor = m_anchorAbove ? UI::RmlBridge::Tooltip::AnchorPoint::AboveLeft
                                   : UI::RmlBridge::Tooltip::AnchorPoint::BelowLeft;
    config.textAlign = UI::RmlBridge::Tooltip::Config::TextAlign::Center; // original RenderTextWithColors() call always passed RT3_SORT_CENTER.
    config.anchorY = UI::Scaling::PositionY(activeTransform, static_cast<float>((m_anchorAbove ? y : (y + height + 2)) + offsetY));

    UI::RmlBridge::Tooltip::Show(config, this);
}
