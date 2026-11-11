#include "stdafx.h"
#include "UI/RmlBridge/RmlMarquee.h"

#include "UI/RmlBridge/RmlPointer.h"

#include <RmlUi/Core.h>

#include <algorithm>
#include <cmath>
#include <unordered_map>

namespace UI::RmlBridge
{
namespace
{
constexpr const char* kMarqueeClass = "marquee";
constexpr const char* kOverflowClass = "marquee-overflow";
constexpr const char* kEllipsis = "..";
// Scroll speed in the element's own px per second; a short wait before it starts, a longer one
// at each end.
constexpr double kSpeed = 50.0;
constexpr double kStartDelay = 0.15;
constexpr double kEndPause = 0.8;

struct MarqueeState
{
    Rml::ObserverPtr<Rml::Element> element;
    Rml::String fullText;
    Rml::String shownText;
    double hoverStart = -1.0;
    // The measurements, for the text, box width and font size they were taken at.
    float measuredWidth = -1.f;
    float measuredFontSize = -1.f;
    float overflow = 0.f;
    Rml::String truncated;
};

std::unordered_map<Rml::Element*, MarqueeState>& States()
{
    static std::unordered_map<Rml::Element*, MarqueeState> states;
    return states;
}

// Where the text sits `elapsed` seconds into a hover: a short wait, the run to the end, a pause
// there, the run back, a pause at the start, repeating.
float Offset(double elapsed, float overflow)
{
    const double run = overflow / kSpeed;
    if (elapsed < kStartDelay)
        return 0.f;
    double t = std::fmod(elapsed - kStartDelay, 2.0 * (run + kEndPause));
    if (t < run)
        return static_cast<float>(t * kSpeed);
    t -= run;
    if (t < kEndPause)
        return overflow;
    t -= kEndPause;
    if (t < run)
        return static_cast<float>(std::max(0.0, overflow - t * kSpeed));
    return 0.f;
}

// The longest start of `text`, cut between UTF-8 characters, that fits `width` with the ellipsis.
Rml::String Truncate(Rml::Element* element, const Rml::String& text, float width)
{
    size_t fit = 0;
    for (size_t end = 1; end <= text.size(); ++end)
    {
        if (end < text.size() && (static_cast<unsigned char>(text[end]) & 0xC0) == 0x80)
            continue;
        if (Rml::ElementUtilities::GetStringWidth(element, text.substr(0, end) + kEllipsis) > width)
            break;
        fit = end;
    }
    while (fit > 0 && text[fit - 1] == ' ')
        --fit;
    return text.substr(0, fit) + kEllipsis;
}

void Show(Rml::ElementText* text, MarqueeState& state, const Rml::String& value)
{
    if (state.shownText != value)
        text->SetText(value);
    state.shownText = value;
}

void UpdateElement(Rml::Element* element, double now)
{
    auto* text = element->GetNumChildren() > 0 ? rmlui_dynamic_cast<Rml::ElementText*>(element->GetChild(0)) : nullptr;
    if (text == nullptr)
        return;

    auto& states = States();
    auto it = states.find(element);
    if (it == states.end())
        it = states.emplace(element, MarqueeState{element->GetObserverPtr()}).first;
    MarqueeState& state = it->second;

    // Text other than what this pass last showed came from the document or its model: the full text.
    if (text->GetText() != state.shownText)
    {
        state.fullText = text->GetText();
        state.shownText = state.fullText;
        state.measuredWidth = -1.f;
    }

    const float width = element->GetClientWidth();
    const float fontSize = element->GetComputedValues().font_size();
    if (width != state.measuredWidth || fontSize != state.measuredFontSize)
    {
        state.measuredWidth = width;
        state.measuredFontSize = fontSize;
        state.overflow = static_cast<float>(Rml::ElementUtilities::GetStringWidth(element, state.fullText)) - width;
        state.truncated = state.overflow > 0.5f ? Truncate(element, state.fullText, width) : state.fullText;
    }
    const float overflow = state.overflow;
    const bool hovered = overflow > 0.5f && IsPointerWithin(element);
    element->SetClass(kOverflowClass, hovered);

    if (!hovered)
    {
        Show(text, state, state.truncated);
        if (state.hoverStart >= 0.0)
            element->SetScrollLeft(0.f);
        state.hoverStart = -1.0;
        return;
    }
    Show(text, state, state.fullText);
    if (state.hoverStart < 0.0)
        state.hoverStart = now;
    element->SetScrollLeft(Offset(now - state.hoverStart, overflow));
}

void UpdateSubtree(Rml::Element* element, double now)
{
    if (element->GetComputedValues().display() == Rml::Style::Display::None)
        return;
    if (element->IsClassSet(kMarqueeClass))
        UpdateElement(element, now);
    for (int i = 0; i < element->GetNumChildren(); ++i)
        UpdateSubtree(element->GetChild(i), now);
}
} // namespace

void UpdateMarquees(Rml::Context* context)
{
    if (!context)
        return;

    const double now = Rml::GetSystemInterface()->GetElapsedTime();
    for (int i = 0; i < context->GetNumDocuments(); ++i)
    {
        Rml::ElementDocument* document = context->GetDocument(i);
        if (document->IsVisible())
            UpdateSubtree(document, now);
    }
    std::erase_if(States(), [](const auto& entry) { return entry.second.element.get() != entry.first; });
}
} // namespace UI::RmlBridge
