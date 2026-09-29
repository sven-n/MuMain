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
constexpr const char* kSceneWindowScaleClass = "scene-window-scale";
constexpr const char* kSceneBarScaleClass = "scene-bar-scale";
} // namespace

float SceneWindowRatio(int windowWidth, int windowHeight)
{
    return ThemeUsesNativeTextSize() ? UI::Scaling::SceneWindowScale(windowWidth, windowHeight)
                                     : UI::Scaling::CompanionRatio(windowWidth, windowHeight);
}

float SceneWindowPixelRatio(int windowWidth, int windowHeight)
{
    return ThemeUsesNativeTextSize() ? UI::Scaling::SceneWindowScale(windowWidth, windowHeight) : 1.0f;
}

void ApplyNativeTextSize(Rml::ElementDocument* document)
{
    if (!ThemeUsesNativeTextSize() || !document || !document->GetContext())
        return;

    const Rml::Vector2i window = document->GetContext()->GetDimensions();
    const auto transform = UI::Scaling::TransformForLayout(UI::Scaling::LayoutMode::Dialog, window.x, window.y);
    const float textPx = UI::Scaling::NativeTextPixelSize(UI::Scaling::FontRole::Normal, transform);
    const bool sceneWindow = document->IsClassSet(kSceneWindowScaleClass);
    if (sceneWindow || document->IsClassSet(kSceneBarScaleClass))
    {
        const float rootPx = sceneWindow ? UI::Scaling::SceneWindowScale(window.x, window.y)
                                         : UI::Scaling::SceneBarScale(window.x, window.y);
        document->SetProperty(Rml::PropertyId::FontSize, Rml::Property(rootPx, Rml::Unit::PX));
        Rml::ElementList textElements;
        document->GetElementsByClassName(textElements, kNativeTextClass);
        for (Rml::Element* element : textElements)
            element->SetProperty(Rml::PropertyId::FontSize, Rml::Property(textPx, Rml::Unit::PX));
        return;
    }

    if (document->IsClassSet(kNativeTextClass))
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
