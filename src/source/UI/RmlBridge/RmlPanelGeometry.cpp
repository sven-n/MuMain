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

    // No transform conversion: the box is already in logical/reference units. Every #panel this
    // reads is sized in plain `px` and scaled at paint time by `transform: scale(root_scale)`
    // (SyncRootTransform), and RmlUi's layout box does not reflect a render-time transform -- so
    // size.x/y are exactly the reference-space extents WindowGeometry wants.
    width = size.x;
    height = size.y;
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
