#include "stdafx.h"
#include "Core/Utilities/FrameProfiler.h"
#include "RmlPanelGeometry.h"

#include "UI/Scaling/UITransform.h"

#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Types.h>

#include <string>

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

bool UI::RmlBridge::FillPlacementSize::Set(float newWidth, float newHeight)
{
    if (width == newWidth && height == newHeight)
        return false;
    width = newWidth;
    height = newHeight;
    return true;
}

void UI::RmlBridge::FillPlacementSize::Apply(Rml::ElementDocument* doc, const char* panelId) const
{
    Rml::Element* panel = doc != nullptr ? doc->GetElementById(panelId) : nullptr;
    if (panel == nullptr)
        return;

    const bool fill = width > 0.f && height > 0.f;
    panel->SetClass("fill-placement", fill);
    if (fill)
    {
        panel->SetProperty(Rml::PropertyId::Width, Rml::Property(width, Rml::Unit::PX));
        panel->SetProperty(Rml::PropertyId::Height, Rml::Property(height, Rml::Unit::PX));
    }
    else
    {
        panel->RemoveProperty(Rml::PropertyId::Width);
        panel->RemoveProperty(Rml::PropertyId::Height);
    }
    {
        FRAME_PROFILE(UILayout);
        doc->UpdateDocument();
    }
}

void UI::RmlBridge::FillPlacementSize::Sync(Rml::ElementDocument* doc, const char* panelId) const
{
    if (width <= 0.f || height <= 0.f || doc == nullptr)
        return;
    Rml::Element* panel = doc->GetElementById(panelId);
    if (panel != nullptr && !panel->IsClassSet("fill-placement"))
        Apply(doc, panelId);
}

bool UI::RmlBridge::SlotPlacement::Set(float newLeft, float newTop, float newScale)
{
    if (left == newLeft && top == newTop && scale == newScale)
        return false;
    left = newLeft;
    top = newTop;
    scale = newScale;
    return true;
}

void UI::RmlBridge::SlotPlacement::Apply(Rml::ElementDocument* doc, const char* panelId) const
{
    Rml::Element* panel = doc != nullptr ? doc->GetElementById(panelId) : nullptr;
    if (panel == nullptr)
        return;

    const bool placed = scale > 0.f;
    panel->SetClass("slot-placed", placed);
    if (placed)
    {
        const std::string scaleText = std::to_string(scale);
        panel->SetProperty(Rml::PropertyId::Left, Rml::Property(left, Rml::Unit::PX));
        panel->SetProperty(Rml::PropertyId::Top, Rml::Property(top, Rml::Unit::PX));
        panel->SetProperty("transform", "scale(" + scaleText + ")");
        panel->SetProperty("--root-scale", scaleText);
    }
    else
    {
        panel->RemoveProperty(Rml::PropertyId::Left);
        panel->RemoveProperty(Rml::PropertyId::Top);
        panel->RemoveProperty("transform");
        panel->RemoveProperty("--root-scale");
    }
}

void UI::RmlBridge::SlotPlacement::Sync(Rml::ElementDocument* doc, const char* panelId) const
{
    if (scale <= 0.f || doc == nullptr)
        return;
    Rml::Element* panel = doc->GetElementById(panelId);
    if (panel != nullptr && !panel->IsClassSet("slot-placed"))
        Apply(doc, panelId);
}

bool UI::RmlBridge::RefreshLogicalAnchorRect(Rml::ElementDocument* doc, const char* panelId, const char* anchorId,
    const POINT& panelPos, float& x, float& y, float& width, float& height)
{
    float w = width;
    float h = height;
    if (!RefreshLogicalPanelSize(doc, anchorId, w, h))
        return false;
    if (!RefreshLogicalAnchorPosition(doc, panelId, anchorId, panelPos, x, y))
        return false;
    width = w;
    height = h;
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
