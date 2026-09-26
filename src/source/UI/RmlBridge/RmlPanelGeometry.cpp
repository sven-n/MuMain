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

bool UI::RmlBridge::RefreshLogicalAnchorPosition(Rml::ElementDocument* doc, const char* panelId,
    const char* anchorId, const POINT& panelPos, float& x, float& y)
{
    if (!doc)
        return false;

    Rml::Element* panel = doc->GetElementById(panelId);
    Rml::Element* anchor = doc->GetElementById(anchorId);
    if (!panel || !anchor)
        return false;

    // The anchor's offset *within* #panel is plain reference px -- the root transform's scale is a
    // paint-time `transform`, which layout doesn't see. So the anchor's logical position is the
    // window's own position plus that raw delta; no conversion enters anywhere. Taking the delta is
    // also what keeps the panel's own pre-multiplied root_x/root_y out of the result.
    const Rml::Vector2f local = anchor->GetAbsoluteOffset() - panel->GetAbsoluteOffset();
    x = static_cast<float>(panelPos.x) + local.x;
    y = static_cast<float>(panelPos.y) + local.y;
    return true;
}
