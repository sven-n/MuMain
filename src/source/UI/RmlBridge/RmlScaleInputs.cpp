#include "stdafx.h"
#include "UI/RmlBridge/RmlScaleInputs.h"

#include "UI/Scaling/UITransform.h"

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Element.h>

#include <string>

void UI::RmlBridge::ApplyScaleInputs(Rml::Context* context)
{
    Rml::Element* root = context != nullptr ? context->GetRootElement() : nullptr;
    if (root == nullptr)
        return;

    const Rml::Vector2i size = context->GetDimensions();
    const auto overlay = UI::Scaling::ScreenOverlayTransform(size.x, size.y);
    const auto set = [root](const char* name, float value) { root->SetProperty(name, std::to_string(value)); };
    set("--ui-scale", UI::Scaling::TypographyScale(size.x, size.y));
    set("--hud-scale", UI::Scaling::BottomHudScale(size.x, size.y));
    set("--dock-scale", UI::Scaling::DockRightTransform(size.x, size.y).scaleX);
    set("--overlay-scale-x", overlay.scaleX);
    set("--overlay-scale-y", overlay.scaleY);
}
