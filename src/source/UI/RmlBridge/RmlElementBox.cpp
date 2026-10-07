#include "stdafx.h"
#include "UI/RmlBridge/RmlElementBox.h"

#include <cmath>

#include <RmlUi/Core/Element.h>

// RmlUi exposes only the inverse (Project), so the affine map is read from three projected points
// and inverted.
bool UI::RmlBridge::DrawnContentBox(Rml::Element& element, Rml::Vector2f& offset, Rml::Vector2f& size)
{
    const Rml::Vector2f layoutOffset = element.GetAbsoluteOffset(Rml::BoxArea::Content);
    const Rml::Vector2f layoutSize = element.GetBox().GetSize(Rml::BoxArea::Content);
    Rml::Vector2f origin(0.f, 0.f), unitX(1.f, 0.f), unitY(0.f, 1.f);
    if (!element.Project(origin) || !element.Project(unitX) || !element.Project(unitY))
        return false;
    // local = origin + a * screen.x + c * screen.y
    const Rml::Vector2f a = unitX - origin;
    const Rml::Vector2f c = unitY - origin;
    const float determinant = a.x * c.y - c.x * a.y;
    if (std::abs(determinant) < 1e-6f)
        return false;
    const auto toScreen = [&](Rml::Vector2f local) {
        local -= origin;
        return Rml::Vector2f((c.y * local.x - c.x * local.y) / determinant, (a.x * local.y - a.y * local.x) / determinant);
    };
    const Rml::Vector2f topLeft = toScreen(layoutOffset);
    const Rml::Vector2f bottomRight = toScreen(layoutOffset + layoutSize);
    offset = topLeft;
    size = bottomRight - topLeft;
    return size.x > 0.f && size.y > 0.f;
}
