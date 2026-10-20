#pragma once

#include "UI/RmlBridge/RmlTooltip.h"

#include <RmlUi/Core/Element.h>

#include <vector>

namespace Rml
{
class Event;
}

namespace UI::RmlBridge
{
// The shared tooltip (RmlTooltip.h) for an element of a window's document. The document forwards
// the element's pointer events to the window's model callbacks (data-event-mouseover="x_hover(i)",
// data-event-mouseout="x_leave"), which call Enter() / Leave(); each frame the window passes the
// hovered element's lines to Show(). The tooltip anchors to the element's drawn box, so it follows
// the element through dp layout and transforms alike.
class ElementTooltip
{
public:
    struct Placement
    {
        Tooltip::AnchorPoint anchor = Tooltip::AnchorPoint::BelowLeft;
        // The anchor's height on the element, as a fraction of its drawn height from its top.
        float anchorAt = 1.f;
        Tooltip::Config::Box box = Tooltip::Config::Box::TipTextList;
    };

    ElementTooltip() = default;
    ElementTooltip(const ElementTooltip&) = delete;
    ElementTooltip& operator=(const ElementTooltip&) = delete;
    ~ElementTooltip();

    // `key` names the element for the window (a row index, a button id).
    void Enter(Rml::Event& event, int key = 0);
    void Leave(Rml::Event& event);
    // The hovered element's key, or -1.
    int Hovered() const;

    // Centred under (or over) the hovered element; hides when nothing is hovered or `lines` is
    // empty.
    void Show(std::vector<Tooltip::Line> lines, const Placement& placement);
    void Show(std::vector<Tooltip::Line> lines) { Show(std::move(lines), Placement{}); }
    void Hide();

private:
    Rml::ObserverPtr<Rml::Element> m_Element;
    int m_Key = -1;
};
} // namespace UI::RmlBridge
