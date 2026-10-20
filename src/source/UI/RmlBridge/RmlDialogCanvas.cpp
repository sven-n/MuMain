#include "stdafx.h"
#include "RmlDialogCanvas.h"
#include "UI/Scaling/UITransform.h"

#include <RmlUi/Core/Context.h>

namespace UI::RmlBridge
{
float DialogCanvasTop(const Rml::Context* context)
{
    if (!context)
        return 0.f;

    const Rml::Vector2i window = context->GetDimensions();
    const auto transform = UI::Scaling::PanelTransform(window.x, window.y);
    const float dpRatio = context->GetDensityIndependentPixelRatio();
    return dpRatio > 0.f ? transform.offsetY / dpRatio : 0.f;
}
} // namespace UI::RmlBridge
