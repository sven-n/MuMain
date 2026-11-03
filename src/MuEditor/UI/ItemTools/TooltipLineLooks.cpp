#include "stdafx.h"

#ifdef _EDITOR

#include "TooltipLineLooks.h"

#include "UI/RmlBridge/RmlTheme.h"
#include "UI/RmlBridge/RmlTooltip.h"

#include <RmlUi/Core/ComputedValues.h>
#include <RmlUi/Core/Element.h>
#include <RmlUi/Core/ElementDocument.h>

#include <optional>
#include <string>

namespace MuEditor::Items
{
namespace
{
using Data::Items::GroundLabelColor;

// The line classes of the tooltip document (tooltip.rml) by TEXT_COLOR_* (Core/Globals/_define.h): every line has
// "tt-line", the others add their class.
constexpr std::string_view LineClass = "tt-line";
constexpr std::array<std::string_view, TooltipLineLookCount> ClassNames = {
    LineClass,        "tt-blue",          "tt-red",          "tt-yellow", "tt-green",     "tt-hl-darkred", "tt-purple",
    "tt-hl-darkblue", "tt-hl-darkyellow", "tt-hl-greenblue", "tt-gray",   "tt-redpurple", "tt-violet",     "tt-orange",
};
static_assert(TEXT_COLOR_WHITE == 0 && static_cast<size_t>(TEXT_COLOR_ORANGE) + 1 == TooltipLineLookCount,
              "ClassNames is indexed by TEXT_COLOR_*");

constexpr const char* PanelId = "tooltip_panel";
constexpr const char* ProbeTag = "div";

GroundLabelColor ToColor(const Rml::Colourb& colour)
{
    return {colour.red, colour.green, colour.blue};
}

// The looks before anything is computed: white lines without bars on a black box.
TooltipLooks DefaultLooks()
{
    TooltipLooks looks;
    for (size_t index = 0; index < looks.lines.size(); ++index)
    {
        looks.lines[index].className = ClassNames[index];
    }
    return looks;
}

// The classes of a line of TEXT_COLOR_* `index`: "tt-line", and its own after it.
std::string LineClasses(size_t index)
{
    std::string classes(LineClass);
    if (index != 0)
    {
        classes += " " + std::string(ClassNames[index]);
    }
    return classes;
}
} // namespace

TooltipLooks ComputeTooltipLooks(Rml::ElementDocument& document)
{
    TooltipLooks looks = DefaultLooks();
    Rml::Element* panel = document.GetElementById(PanelId);
    Rml::Element& parent = panel != nullptr ? *panel : document;
    std::array<Rml::Element*, TooltipLineLookCount> probes{};
    for (size_t index = 0; index < probes.size(); ++index)
    {
        Rml::ElementPtr probe = document.CreateElement(ProbeTag);
        probe->SetClassNames(LineClasses(index));
        probes[index] = parent.AppendChild(std::move(probe));
    }
    // Their styles, as the theme's rules and the cascade give them.
    document.UpdateDocument();
    for (size_t index = 0; index < probes.size(); ++index)
    {
        const Rml::ComputedValues& values = probes[index]->GetComputedValues();
        TooltipLineLook& look = looks.lines[index];
        look.text = ToColor(values.color());
        look.background = ToColor(values.background_color());
        look.backgroundAlpha = values.background_color().alpha;
        parent.RemoveChild(probes[index]);
    }
    if (panel != nullptr)
    {
        const Rml::Colourb box = panel->GetComputedValues().background_color();
        looks.box = ToColor(box);
        looks.boxAlpha = box.alpha;
    }
    return looks;
}

const TooltipLooks& GetTooltipLooks()
{
    static TooltipLooks looks = DefaultLooks();
    static std::optional<std::string> lookTheme;
    static const Rml::ElementDocument* lookDocument = nullptr;
    const std::string& theme = UI::RmlBridge::GetActiveThemeName();
    Rml::ElementDocument* document = UI::RmlBridge::Tooltip::GetDocument();
    // A theme switch builds the document again.
    if (lookTheme == theme && lookDocument == document)
    {
        return looks;
    }
    looks = document != nullptr ? ComputeTooltipLooks(*document) : DefaultLooks();
    lookTheme = theme;
    lookDocument = document;
    return looks;
}

const TooltipLineLook& GetTooltipLineLook(const TooltipLooks& looks, int color)
{
    const bool known = color >= 0 && color < static_cast<int>(looks.lines.size());
    return looks.lines[known ? static_cast<size_t>(color) : 0];
}
} // namespace MuEditor::Items

#endif // _EDITOR
