#include "stdafx.h"
#include "RmlNativeText.h"
#include "RmlTheme.h"
#include "UI/Scaling/UITransform.h"

#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/Property.h>

namespace UI::RmlBridge
{
namespace
{
constexpr const char* kNativeTextClass = "native-text";
}

void ApplyNativeTextSize(Rml::ElementDocument* document)
{
    if (!ThemeUsesNativeTextSize() || !document || !document->GetContext() || !document->IsClassSet(kNativeTextClass))
        return;

    const Rml::Vector2i window = document->GetContext()->GetDimensions();
    const auto transform = UI::Scaling::TransformForLayout(UI::Scaling::LayoutMode::Dialog, window.x, window.y);
    const float textPx = UI::Scaling::NativeTextPixelSize(UI::Scaling::FontRole::Normal, transform);
    document->SetProperty(Rml::PropertyId::FontSize, Rml::Property(textPx, Rml::Unit::PX));
}

void ApplyNativeTextSize(Rml::Context* context)
{
    if (!context)
        return;

    for (int i = 0; i < context->GetNumDocuments(); ++i)
        ApplyNativeTextSize(context->GetDocument(i));
}
} // namespace UI::RmlBridge
