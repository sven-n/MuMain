#pragma once

#include <RmlUi/Core/Box.h>
#include <RmlUi/Core/Types.h>

namespace Rml
{
class Element;
}

namespace UI::RmlBridge
{
// `element`'s content box where it is drawn, in window pixels: through every transform on it and
// its ancestors, which GetAbsoluteOffset() leaves out. False while it has no area.
bool DrawnContentBox(Rml::Element& element, Rml::Vector2f& offset, Rml::Vector2f& size);
// The same for any of its boxes.
bool DrawnBox(Rml::Element& element, Rml::BoxArea area, Rml::Vector2f& offset, Rml::Vector2f& size);
// Its border box's drawn top-left, also for an element with no area (a point the theme places).
bool DrawnTopLeft(Rml::Element& element, Rml::Vector2f& point);
// Its drawn width over its laid-out width: the scale its transforms give it; 1 while it has none.
float DrawnScale(Rml::Element& element);
} // namespace UI::RmlBridge
