#include "stdafx.h"
#include "RmlNativeTextFit.h"
#include "RmlTheme.h"
#include "UI/Scaling/UITransform.h"

#include <RmlUi/Core/ComputedValues.h>
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Core/ElementText.h>
#include <RmlUi/Core/FontEngineInterface.h>
#include <RmlUi/Core/Property.h>
#include <RmlUi/Core/TextShapingContext.h>

#include <functional>
#include <string_view>
#include <unordered_map>

namespace UI::RmlBridge
{
namespace
{
const Rml::String kNativeFitClass = "native-fit";

// What an element was last fitted for; unchanged inputs skip the measurement.
struct FitState
{
    Rml::ObserverPtr<Rml::Element> element; // expires with the element; guards a reused address
    size_t textHash = 0;
    float boxWidth = 0.f;
    float nativePx = 0.f;
    float scale = 1.f; // the element's own font-size in em; 1 = none set
};

std::unordered_map<Rml::Element*, FitState>& FitStates()
{
    static std::unordered_map<Rml::Element*, FitState> states;
    return states;
}

size_t HashText(Rml::Element* element)
{
    size_t hash = 0;
    for (int i = 0; i < element->GetNumChildren(); ++i)
    {
        if (const auto* text = dynamic_cast<const Rml::ElementText*>(element->GetChild(i)))
            hash = hash * 31 + std::hash<std::string_view>{}(text->GetText());
    }
    return hash;
}

// The width of the element's text at `nativePx`, its size without the fit.
float MeasureTextWidth(Rml::Element* element, float nativePx)
{
    const auto& values = element->GetComputedValues();
    Rml::FontEngineInterface* fonts = Rml::GetFontEngineInterface();
    const Rml::FontFaceHandle face = fonts->GetFontFaceHandle(values.font_family(), values.font_style(),
                                                              values.font_weight(), static_cast<int>(nativePx));
    if (face == 0)
        return 0.f;

    const Rml::TextShapingContext shaping{values.language(), values.direction(), values.font_kerning(),
                                          values.letter_spacing()};
    int width = 0;
    for (int i = 0; i < element->GetNumChildren(); ++i)
    {
        if (const auto* text = dynamic_cast<const Rml::ElementText*>(element->GetChild(i)))
            width += fonts->GetStringWidth(face, text->GetText(), shaping);
    }
    return static_cast<float>(width);
}

// Returns whether the element's font-size changed.
bool FitElement(Rml::Element* element)
{
    const Rml::Element* parent = element->GetParentNode();
    const float boxWidth = element->GetBox().GetSize(Rml::BoxArea::Content).x;
    if (parent == nullptr || boxWidth <= 0.f)
        return false;

    const float nativePx = parent->GetComputedValues().font_size();
    const size_t textHash = HashText(element);
    FitState& state = FitStates()[element];
    const bool sameElement = state.element.get() == element;
    if (sameElement && state.textHash == textHash && state.boxWidth == boxWidth && state.nativePx == nativePx)
        return false;

    const float previousScale = sameElement ? state.scale : 1.f;
    const float fittedPx =
        UI::Scaling::FitTextPixelSizeToWidth(nativePx, MeasureTextWidth(element, nativePx), boxWidth,
                                             UI::Scaling::MinimumTextPixelSize(UI::Scaling::FontRole::Normal));
    const float scale = nativePx > 0.f ? fittedPx / nativePx : 1.f;
    state = {element->GetObserverPtr(), textHash, boxWidth, nativePx, scale};

    if (scale >= 1.f)
    {
        if (previousScale >= 1.f)
            return false;
        element->RemoveProperty(Rml::PropertyId::FontSize);
        return true;
    }
    if (scale == previousScale)
        return false;
    element->SetProperty(Rml::PropertyId::FontSize, Rml::Property(scale, Rml::Unit::EM));
    return true;
}

// Returns whether any font-size under `element` changed; skips hidden subtrees.
bool FitSubtree(Rml::Element* element)
{
    if (element->GetComputedValues().display() == Rml::Style::Display::None)
        return false;

    bool changed = element->IsClassSet(kNativeFitClass) && FitElement(element);
    for (int i = 0; i < element->GetNumChildren(); ++i)
        changed |= FitSubtree(element->GetChild(i));
    return changed;
}

void ForgetRemovedElements()
{
    std::erase_if(FitStates(), [](const auto& entry) { return entry.second.element.get() != entry.first; });
}
} // namespace

void FitNativeTextToBoxes(Rml::Context* context)
{
    if (!context || !ThemeUsesNativeTextSize())
        return;

    for (int i = 0; i < context->GetNumDocuments(); ++i)
    {
        Rml::ElementDocument* document = context->GetDocument(i);
        if (document->IsVisible() && FitSubtree(document))
            document->UpdateDocument();
    }
    ForgetRemovedElements();
}
} // namespace UI::RmlBridge
