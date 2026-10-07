#include "stdafx.h"
#include "UI/RmlBridge/RmlElementTooltip.h"

#include "UI/RmlBridge/RmlElementBox.h"

#include <RmlUi/Core/Event.h>

namespace UI::RmlBridge
{
ElementTooltip::~ElementTooltip()
{
    Tooltip::Hide(this);
}

void ElementTooltip::Enter(Rml::Event& event, int key)
{
    Rml::Element* element = event.GetCurrentElement();
    m_Element = element ? element->GetObserverPtr() : Rml::ObserverPtr<Rml::Element>{};
    m_Key = element ? key : -1;
}

void ElementTooltip::Leave(Rml::Event& event)
{
    // A child's mouseout bubbles here too, while the element itself is still under the pointer;
    // the element's own mouseout comes only when the pointer leaves it.
    if (event.GetTargetElement() != event.GetCurrentElement())
        return;
    m_Element.reset();
    m_Key = -1;
    Hide();
}

int ElementTooltip::Hovered() const
{
    return m_Element ? m_Key : -1;
}

void ElementTooltip::Show(std::vector<Tooltip::Line> lines, const Placement& placement)
{
    Rml::Vector2f offset, size;
    if (lines.empty() || !m_Element || !m_Element->IsVisible(true) ||
        !DrawnContentBox(*m_Element.get(), offset, size))
    {
        Hide();
        return;
    }

    Tooltip::Config config;
    config.lines = std::move(lines);
    config.box = placement.box;
    config.anchor = placement.anchor;
    config.anchorX = offset.x + size.x / 2.f;
    config.anchorY = offset.y + size.y * placement.anchorAt;
    config.centerHorizontally = true;
    config.textAlign = Tooltip::Config::TextAlign::Center;
    Tooltip::Show(config, this);
}

void ElementTooltip::Hide()
{
    Tooltip::Hide(this);
}
} // namespace UI::RmlBridge
