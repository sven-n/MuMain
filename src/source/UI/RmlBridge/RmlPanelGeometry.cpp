#include "stdafx.h"
#include "Core/Utilities/FrameProfiler.h"
#include "RmlPanelGeometry.h"
#include "RmlDraggable.h"

#include "UI/Scaling/UITransform.h"

#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Types.h>

#include <string>

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

    // A new place takes over from where the player dragged it.
    ResetDrag(doc, panelId);
    const bool placed = scale > 0.f;
    panel->SetClass("slot-placed", placed);
    if (placed)
    {
        panel->SetProperty("--slot-left", std::to_string(left) + "px");
        panel->SetProperty("--slot-top", std::to_string(top) + "px");
        panel->SetProperty("--root-scale", std::to_string(scale));
    }
    else
    {
        panel->RemoveProperty("--slot-left");
        panel->RemoveProperty("--slot-top");
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

void UI::RmlBridge::SlotPlacement::ResetDrag(Rml::ElementDocument* doc, const char* panelId)
{
    Rml::Element* panel = doc != nullptr ? doc->GetElementById(panelId) : nullptr;
    if (panel != nullptr && panel->IsClassSet("dragged"))
        ResetDraggedPosition(panel);
}
