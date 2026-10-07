#pragma once

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
} // namespace UI::RmlBridge
