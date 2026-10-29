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
#include <RmlUi/Core/Plugin.h>
#include <RmlUi/Core/Property.h>
#include <RmlUi/Core/TextShapingContext.h>
#include <RmlUi/Core/Utilities.h>

#include <cmath>
#include <functional>
#include <optional>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace UI::RmlBridge
{
namespace
{
const Rml::String kNativeFitClass = "native-fit";
const Rml::String kTextFitClass = "text-fit";
// How far a fitted text may still overrun its box: glyph advances round to whole pixels.
constexpr float kOverflowTolerancePx = 1.f;
// The smallest raster size `text-fit` searches down to (`native-fit` keeps the role's minimum).
constexpr float kSmallestTextFitPx = 1.f;

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

// Appends the element's text and that of its shown inline children, in document order.
void AppendText(Rml::Element* element, Rml::String& text)
{
    for (int i = 0; i < element->GetNumChildren(); ++i)
    {
        Rml::Element* child = element->GetChild(i);
        if (const auto* textChild = dynamic_cast<const Rml::ElementText*>(child))
            text += textChild->GetText();
        else if (child->GetComputedValues().display() != Rml::Style::Display::None)
            AppendText(child, text);
    }
}

bool IsCollapsibleSpace(char character)
{
    return character == ' ' || character == '\t' || character == '\n' || character == '\r' || character == '\f';
}

// The text as RmlUi lays it out on one line: under white-space normal and nowrap a run of spaces is
// one space and the line drops its leading and trailing spaces.
Rml::String LaidOutText(Rml::Element* element)
{
    Rml::String raw;
    AppendText(element, raw);
    const auto whiteSpace = element->GetComputedValues().white_space();
    if (whiteSpace != Rml::Style::WhiteSpace::Normal && whiteSpace != Rml::Style::WhiteSpace::Nowrap)
        return raw;

    Rml::String text;
    bool pendingSpace = false;
    for (const char character : raw)
    {
        if (IsCollapsibleSpace(character))
        {
            pendingSpace = !text.empty();
            continue;
        }
        if (pendingSpace)
            text += ' ';
        pendingSpace = false;
        text += character;
    }
    return text;
}

size_t HashText(Rml::Element* element)
{
    return std::hash<std::string_view>{}(LaidOutText(element));
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

// Nested inline text is measured in the element's own face; a nested element with another font
// is not measured in its own.
Rml::String TextForMeasurement(Rml::Element* element, Rml::Style::TextTransform transform)
{
    Rml::String text = LaidOutText(element);
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

// The width of the element's text drawn at `fontPx` with `letterSpacingPx`.
float TextWidthAt(Rml::Element* element, float fontPx, float letterSpacingPx)
{
    const auto& values = element->GetComputedValues();
    Rml::FontEngineInterface* fonts = Rml::GetFontEngineInterface();
    const Rml::FontFaceHandle face = fonts->GetFontFaceHandle(values.font_family(), values.font_style(),
                                                              values.font_weight(), static_cast<int>(fontPx));
    if (face == 0)
        return 0.f;

    const Rml::TextShapingContext shaping{values.language(), values.direction(), values.font_kerning(),
                                          letterSpacingPx};
    return static_cast<float>(
        fonts->GetStringWidth(face, TextForMeasurement(element, values.text_transform()), shaping));
}

// The width of the element's text at `fontPx` with its current letter spacing.
float MeasureTextWidth(Rml::Element* element, float fontPx)
{
    return TextWidthAt(element, fontPx, element->GetComputedValues().letter_spacing());
}

// Letter spacing as a function of the font size. RmlUi resolves an em length, also inside calc(),
// against the element's own font size, so each probed size has its own spacing.
struct LetterSpacingModel
{
    float fixedPx = 0.f;
    float perEm = 0.f;

    float At(float fontPx) const
    {
        return fixedPx + perEm * fontPx;
    }
};

// Reads the model from the element's current spacing. Only a calc() mixing em and fixed lengths needs
// a second sample, taken with one layout at another size.
LetterSpacingModel ReadLetterSpacing(Rml::Element* element)
{
    const float currentPx = element->GetComputedValues().font_size();
    const float currentSpacing = element->GetComputedValues().letter_spacing();
    const Rml::Property* property = element->GetProperty(Rml::PropertyId::LetterSpacing);
    if (property == nullptr || property->unit == Rml::Unit::KEYWORD)
        return {};
    if (property->unit == Rml::Unit::EM)
        return {0.f, property->Get<float>()};
    if (property->unit != Rml::Unit::CALCULATION || currentPx <= 0.f)
        return {currentSpacing, 0.f};

    const float otherPx = currentPx * 2.f;
    element->SetProperty(Rml::PropertyId::FontSize, Rml::Property(otherPx, Rml::Unit::PX));
    element->GetOwnerDocument()->UpdateDocument();
    const float perEm = (element->GetComputedValues().letter_spacing() - currentSpacing) / currentPx;
    return {currentSpacing - perEm * currentPx, perEm};
}

// What a fit needs to know about an element, and the font-size scale it had before.
struct FitInputs
{
    size_t textHash = 0;
    float boxWidth = 0.f;
    float nativePx = 0.f;
    size_t typographyHash = 0;
    float previousScale = 1.f;
};

// The element's fit inputs, or nothing when it has no box yet or nothing changed since its last fit.
std::optional<FitInputs> ChangedFitInputs(Rml::Element* element, bool textFit)
{
    const Rml::Element* parent = element->GetParentNode();
    const float boxWidth = element->GetBox().GetSize(Rml::BoxArea::Content).x;
    if (parent == nullptr || boxWidth <= 0.f)
        return std::nullopt;

    FitInputs inputs{HashText(element), boxWidth, parent->GetComputedValues().font_size(),
                     textFit ? HashTypography(element) : 0};
    const FitState& state = FitStates()[element];
    const bool sameElement = state.element.get() == element;
    if (sameElement && state.textHash == inputs.textHash && state.boxWidth == boxWidth &&
        state.nativePx == inputs.nativePx && state.typographyHash == inputs.typographyHash && state.textFit == textFit)
        return std::nullopt;
    inputs.previousScale = sameElement ? state.scale : 1.f;
    return inputs;
}

// native-fit: one proportional step, as the original's RenderText() did, not below the role's minimum.
bool FitNativeElement(Rml::Element* element)
{
    const auto inputs = ChangedFitInputs(element, false);
    if (!inputs)
        return false;

    const float fittedPx = UI::Scaling::FitTextPixelSizeToWidth(
        inputs->nativePx, MeasureTextWidth(element, inputs->nativePx), inputs->boxWidth,
        UI::Scaling::MinimumTextPixelSize(UI::Scaling::FontRole::Normal));
    const float scale = inputs->nativePx > 0.f ? fittedPx / inputs->nativePx : 1.f;
    FitStates()[element] = {
        element->GetObserverPtr(), inputs->textHash, inputs->boxWidth, inputs->nativePx, scale, 0, false};
    if (scale >= 1.f)
    {
        if (inputs->previousScale >= 1.f)
            return false;
        element->RemoveProperty(Rml::PropertyId::FontSize);
        return true;
    }
    if (scale == inputs->previousScale)
        return false;
    element->SetProperty(Rml::PropertyId::FontSize, Rml::Property(scale, Rml::Unit::EM));
    return true;
}

// text-fit: the largest whole raster size whose text fits, found by search, since fixed spacing and
// hinting do not scale with the glyph width.
bool FitTextElement(Rml::Element* element)
{
    const auto inputs = ChangedFitInputs(element, true);
    if (!inputs)
        return false;

    const float nativePx = inputs->nativePx;
    const float boxWidth = inputs->boxWidth;
    const LetterSpacingModel spacing = ReadLetterSpacing(element);
    const auto widthAt = [&](float fontPx) { return TextWidthAt(element, fontPx, spacing.At(fontPx)); };
    const float measuredWidth = widthAt(nativePx);
    float fittedPx = UI::Scaling::FitTextPixelSizeToWidth(nativePx, measuredWidth, boxWidth, kSmallestTextFitPx);
    if (measuredWidth > boxWidth + kOverflowTolerancePx)
    {
        float lowerPx = kSmallestTextFitPx;
        float upperPx = std::ceil(nativePx);
        while (lowerPx + 1.f < upperPx)
        {
            const float candidatePx = std::floor((lowerPx + upperPx) * 0.5f);
            if (widthAt(candidatePx) <= boxWidth + kOverflowTolerancePx)
                lowerPx = candidatePx;
            else
                upperPx = candidatePx;
        }
        fittedPx = std::min(lowerPx, nativePx);
    }
    const auto applySize = [&](float fontPx)
    {
        if (fontPx >= nativePx)
            element->RemoveProperty(Rml::PropertyId::FontSize);
        else
            element->SetProperty(Rml::PropertyId::FontSize, Rml::Property(fontPx / nativePx, Rml::Unit::EM));
        element->GetOwnerDocument()->UpdateDocument();
    };
    applySize(fittedPx);
    // The model's spacing can round differently from RmlUi's own; check the size RmlUi laid out.
    while (fittedPx > kSmallestTextFitPx &&
           MeasureTextWidth(element, element->GetComputedValues().font_size()) > boxWidth + kOverflowTolerancePx)
    {
        fittedPx = std::max(kSmallestTextFitPx, std::floor(fittedPx) - 1.f);
        applySize(fittedPx);
    }
    const float scale = nativePx > 0.f ? fittedPx / nativePx : 1.f;
    FitStates()[element] = {
        element->GetObserverPtr(), inputs->textHash, boxWidth, nativePx, scale, HashTypography(element), true};
    return scale != inputs->previousScale;
}

// Returns whether any native-fit font-size under `element` changed; skips hidden subtrees. A
// text-fit element is fitted from the tracker instead, also when it carries both classes.
bool FitNativeSubtree(Rml::Element* element)
{
    if (element->GetComputedValues().display() == Rml::Style::Display::None)
        return false;

    bool changed =
        element->IsClassSet(kNativeFitClass) && !element->IsClassSet(kTextFitClass) && FitNativeElement(element);
    for (int i = 0; i < element->GetNumChildren(); ++i)
        changed |= FitNativeSubtree(element->GetChild(i));
    return changed;
}

void CollectTextFit(Rml::Element* element, std::unordered_set<Rml::Element*>& found)
{
    if (element->IsClassSet(kTextFitClass))
        found.insert(element);
    for (int i = 0; i < element->GetNumChildren(); ++i)
        CollectTextFit(element->GetChild(i), found);
}

// Keeps the text-fit elements, so the per-frame pass visits them and not every element of every
// document. RmlUi sets an element's classes after creating it, so a new element waits until the
// next pass checks its class; a class added later than its first frame is not seen.
class TextFitTracker final : public Rml::Plugin
{
public:
    // The current text-fit elements. The first call after Rml::Initialise() walks the existing
    // documents once and starts following element creation.
    std::vector<Rml::Element*> Elements()
    {
        if (!m_Following)
            Follow();
        for (const auto& created : m_Created)
        {
            if (created && created->IsClassSet(kTextFitClass))
                m_TextFit.insert(created.get());
        }
        m_Created.clear();
        return {m_TextFit.begin(), m_TextFit.end()};
    }

    int GetEventClasses() override
    {
        return EVT_BASIC | EVT_ELEMENT;
    }
    void OnElementCreate(Rml::Element* element) override
    {
        m_Created.push_back(element->GetObserverPtr());
    }
    void OnElementDestroy(Rml::Element* element) override
    {
        m_TextFit.erase(element);
    }
    void OnShutdown() override
    {
        m_Created.clear();
        m_TextFit.clear();
        m_Following = false;
    }

private:
    void Follow()
    {
        m_Following = true;
        m_TextFit.clear();
        for (int i = 0; i < Rml::GetNumContexts(); ++i)
        {
            Rml::Context* context = Rml::GetContext(i);
            for (int j = 0; j < context->GetNumDocuments(); ++j)
                CollectTextFit(context->GetDocument(j), m_TextFit);
        }
        Rml::RegisterPlugin(this);
    }

    std::vector<Rml::ObserverPtr<Rml::Element>> m_Created;
    std::unordered_set<Rml::Element*> m_TextFit;
    bool m_Following = false;
};

// Never destroyed: Rml::Shutdown() may still notify it at exit.
TextFitTracker& Tracker()
{
    static auto* tracker = new TextFitTracker();
    return *tracker;
}

// Whether the element and all its ancestors take part in layout.
bool IsDisplayed(Rml::Element* element)
{
    for (; element != nullptr; element = element->GetParentNode())
    {
        if (element->GetComputedValues().display() == Rml::Style::Display::None)
            return false;
    }
    return true;
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

    std::unordered_set<Rml::ElementDocument*> changed;
    for (Rml::Element* element : Tracker().Elements())
    {
        Rml::ElementDocument* document = element->GetOwnerDocument();
        if (element->GetContext() != context || document == nullptr || !document->IsVisible() ||
            !element->IsClassSet(kTextFitClass) || !IsDisplayed(element))
            continue;
        if (FitTextElement(element))
            changed.insert(document);
    }

    if (ThemeUsesNativeTextSize())
    {
        for (int i = 0; i < context->GetNumDocuments(); ++i)
        {
            Rml::ElementDocument* document = context->GetDocument(i);
            if (document->IsVisible() && FitNativeSubtree(document))
                changed.insert(document);
        }
    }
    for (Rml::ElementDocument* document : changed)
        document->UpdateDocument();
    ForgetRemovedElements();
}
} // namespace UI::RmlBridge
