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
#include <RmlUi/Core/Utilities.h>

#include <cmath>
#include <functional>
#include <string_view>
#include <unordered_map>

namespace UI::RmlBridge
{
namespace
{
const Rml::String kNativeFitClass = "native-fit";
const Rml::String kTextFitClass = "text-fit";

// What an element was last fitted for; unchanged inputs skip the measurement.
struct FitState
{
    Rml::ObserverPtr<Rml::Element> element; // expires with the element; guards a reused address
    size_t textHash = 0;
    float boxWidth = 0.f;
    float nativePx = 0.f;
    float scale = 1.f; // the element's own font-size in em; 1 = none set
    size_t typographyHash = 0;
    bool textFit = false;
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

size_t HashTypography(Rml::Element* element)
{
    const auto& values = element->GetComputedValues();
    size_t hash = 0;
    const auto add = [&hash](size_t value) { Rml::Utilities::HashCombine(hash, value); };
    if (const auto* family = element->GetProperty(Rml::PropertyId::FontFamily))
        add(std::hash<std::string_view>{}(family->value.GetReference<Rml::String>()));
    add(std::hash<std::string_view>{}(values.language()));
    add(std::hash<float>{}(values.letter_spacing()));
    add(static_cast<size_t>(values.font_style()));
    add(static_cast<size_t>(values.font_weight()));
    add(static_cast<size_t>(values.font_kerning()));
    add(static_cast<size_t>(values.direction()));
    add(static_cast<size_t>(values.text_transform()));
    return hash;
}

Rml::String TextForMeasurement(Rml::Element* element, Rml::Style::TextTransform transform)
{
    Rml::String text;
    for (int i = 0; i < element->GetNumChildren(); ++i)
    {
        if (const auto* child = dynamic_cast<const Rml::ElementText*>(element->GetChild(i)))
            text += child->GetText();
    }
    if (transform == Rml::Style::TextTransform::Uppercase || transform == Rml::Style::TextTransform::Lowercase)
    {
        for (char& character : text)
        {
            if (transform == Rml::Style::TextTransform::Uppercase && character >= 'a' && character <= 'z')
                character += 'A' - 'a';
            else if (transform == Rml::Style::TextTransform::Lowercase && character >= 'A' && character <= 'Z')
                character -= 'A' - 'a';
        }
    }
    return text;
}

// Update relative spacing through RmlUi itself when probing a text-fit font size.
float MeasureTextWidth(Rml::Element* element, float nativePx, bool textFit)
{
    if (textFit)
    {
        element->SetProperty(Rml::PropertyId::FontSize, Rml::Property(nativePx, Rml::Unit::PX));
        element->GetOwnerDocument()->UpdateDocument();
    }
    const auto& values = element->GetComputedValues();
    Rml::FontEngineInterface* fonts = Rml::GetFontEngineInterface();
    const Rml::FontFaceHandle face = fonts->GetFontFaceHandle(values.font_family(), values.font_style(),
                                                              values.font_weight(), static_cast<int>(nativePx));
    if (face == 0)
        return 0.f;

    const Rml::TextShapingContext shaping{values.language(), values.direction(), values.font_kerning(),
                                          values.letter_spacing()};
    return static_cast<float>(
        fonts->GetStringWidth(face, TextForMeasurement(element, values.text_transform()), shaping));
}

// Returns whether the element's font-size changed.
bool FitElement(Rml::Element* element, bool textFit)
{
    const Rml::Element* parent = element->GetParentNode();
    const float boxWidth = element->GetBox().GetSize(Rml::BoxArea::Content).x;
    if (parent == nullptr || boxWidth <= 0.f)
        return false;

    const float nativePx = parent->GetComputedValues().font_size();
    const size_t textHash = HashText(element);
    const size_t typographyHash = textFit ? HashTypography(element) : 0;
    FitState& state = FitStates()[element];
    const bool sameElement = state.element.get() == element;
    if (sameElement && state.textHash == textHash && state.boxWidth == boxWidth && state.nativePx == nativePx &&
        state.typographyHash == typographyHash && state.textFit == textFit)
        return false;

    const float previousScale = sameElement ? state.scale : 1.f;
    const float measuredWidth = MeasureTextWidth(element, nativePx, textFit);
    float fittedPx = UI::Scaling::FitTextPixelSizeToWidth(
        nativePx, measuredWidth, boxWidth,
        textFit ? 1.f : UI::Scaling::MinimumTextPixelSize(UI::Scaling::FontRole::Normal));
    if (textFit && measuredWidth > boxWidth + 1.f)
    {
        float lowerPx = 1.f;
        float upperPx = std::ceil(nativePx);
        // Search actual raster sizes: fixed spacing and hinting do not scale with glyph width.
        while (lowerPx + 1.f < upperPx)
        {
            const float candidatePx = std::floor((lowerPx + upperPx) * 0.5f);
            if (MeasureTextWidth(element, candidatePx, true) <= boxWidth + 1.f)
                lowerPx = candidatePx;
            else
                upperPx = candidatePx;
        }
        fittedPx = std::min(lowerPx, nativePx);
    }
    const float scale = nativePx > 0.f ? fittedPx / nativePx : 1.f;
    if (textFit)
    {
        if (scale >= 1.f)
            element->RemoveProperty(Rml::PropertyId::FontSize);
        else
            element->SetProperty(Rml::PropertyId::FontSize, Rml::Property(scale, Rml::Unit::EM));
        element->GetOwnerDocument()->UpdateDocument();
        state = {element->GetObserverPtr(), textHash, boxWidth, nativePx, scale, HashTypography(element), textFit};
        return scale != previousScale;
    }
    state = {element->GetObserverPtr(), textHash, boxWidth, nativePx, scale, typographyHash, textFit};
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
bool FitSubtree(Rml::Element* element, bool allowNativeFit)
{
    if (element->GetComputedValues().display() == Rml::Style::Display::None)
        return false;

    const bool textFit = element->IsClassSet(kTextFitClass);
    const bool nativeFit = allowNativeFit && element->IsClassSet(kNativeFitClass);
    bool changed = (textFit || nativeFit) && FitElement(element, textFit);
    for (int i = 0; i < element->GetNumChildren(); ++i)
        changed |= FitSubtree(element->GetChild(i), allowNativeFit);
    return changed;
}

void ForgetRemovedElements()
{
    std::erase_if(FitStates(), [](const auto& entry) { return entry.second.element.get() != entry.first; });
}
} // namespace

void FitNativeTextToBoxes(Rml::Context* context)
{
    if (!context)
        return;

    const bool allowNativeFit = ThemeUsesNativeTextSize();

    for (int i = 0; i < context->GetNumDocuments(); ++i)
    {
        Rml::ElementDocument* document = context->GetDocument(i);
        if (document->IsVisible() && FitSubtree(document, allowNativeFit))
            document->UpdateDocument();
    }
    ForgetRemovedElements();
}
} // namespace UI::RmlBridge
