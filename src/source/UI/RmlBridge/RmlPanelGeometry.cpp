#include "stdafx.h"
#include "RmlPanelGeometry.h"

#include "UI/Scaling/UITransform.h"

#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Types.h>

bool UI::RmlBridge::RefreshLogicalPanelSize(Rml::ElementDocument* doc, const char* panelId, float& width, float& height)
{
    if (!doc)
        return false;

    Rml::Element* panel = doc->GetElementById(panelId);
    if (!panel)
        return false;

    const Rml::Vector2f size = panel->GetBox().GetSize(Rml::BoxArea::Border);
    if (size.x <= 0.0f || size.y <= 0.0f)
        return false;

    const auto transform = UI::Scaling::GetActiveTransform();
    if (transform.scaleX <= 0.0f || transform.scaleY <= 0.0f)
        return false;

    width = size.x / transform.scaleX;
    height = size.y / transform.scaleY;
    return true;
}

bool UI::RmlBridge::RefreshLogicalAnchorPosition(Rml::ElementDocument* doc, const char* anchorId,
    UI::Scaling::LayoutMode layoutMode, float& x, float& y)
{
    if (!doc)
        return false;

    Rml::Element* anchor = doc->GetElementById(anchorId);
    if (!anchor)
        return false;

    const auto transform = UI::Scaling::TransformForLayout(layoutMode, WindowWidth, WindowHeight);
    if (transform.scaleX <= 0.0f || transform.scaleY <= 0.0f)
        return false;

    const Rml::Vector2f offset = anchor->GetAbsoluteOffset();
    x = UI::Scaling::LogicalX(transform, offset.x);
    y = UI::Scaling::LogicalY(transform, offset.y);
    return true;
}
