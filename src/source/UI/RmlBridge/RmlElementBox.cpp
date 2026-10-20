#include "stdafx.h"
#include "UI/RmlBridge/RmlElementBox.h"

#include <cmath>

#include <RmlUi/Core/Element.h>

namespace
{
// RmlUi exposes only the inverse (Project), so the affine map is read from three projected points
// and inverted.
bool LayoutToScreen(Rml::Element& element, Rml::Vector2f layoutOffset, Rml::Vector2f layoutSize,
    Rml::Vector2f& offset, Rml::Vector2f& size)
{
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
    offset = topLeft;
    size = toScreen(layoutOffset + layoutSize) - topLeft;
    return true;
}
} // namespace

bool UI::RmlBridge::DrawnBox(Rml::Element& element, Rml::BoxArea area, Rml::Vector2f& offset, Rml::Vector2f& size)
{
    return LayoutToScreen(element, element.GetAbsoluteOffset(area), element.GetBox().GetSize(area), offset, size) &&
           size.x > 0.f && size.y > 0.f;
}

bool UI::RmlBridge::DrawnContentBox(Rml::Element& element, Rml::Vector2f& offset, Rml::Vector2f& size)
{
    return DrawnBox(element, Rml::BoxArea::Content, offset, size);
}

bool UI::RmlBridge::DrawnTopLeft(Rml::Element& element, Rml::Vector2f& point)
{
    Rml::Vector2f size;
    return LayoutToScreen(element, element.GetAbsoluteOffset(Rml::BoxArea::Border), Rml::Vector2f(0.f, 0.f), point, size);
}

float UI::RmlBridge::DrawnScale(Rml::Element& element)
{
    const float layoutWidth = element.GetBox().GetSize(Rml::BoxArea::Border).x;
    Rml::Vector2f offset;
    Rml::Vector2f size;
    if (layoutWidth <= 0.f || !DrawnBox(element, Rml::BoxArea::Border, offset, size))
        return 1.f;
    return size.x / layoutWidth;
}
