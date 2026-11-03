#include "stdafx.h"

#ifdef _EDITOR

#include "TooltipLineLooks.h"

#include "Data/GameData/ItemData/ItemJsonCommon.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "UI/RmlBridge/ThemeInheritance.h"

#include <charconv>
#include <filesystem>
#include <string>

namespace MuEditor::Items
{
namespace
{
using Data::Items::GroundLabelColor;

// The line classes of the tooltip document (tooltip.rml) by TEXT_COLOR_* (Core/Globals/_define.h): a plain line is
// white, the others add their class.
constexpr std::array<std::string_view, TooltipLineLookCount> ClassNames = {
    "tt-line",        "tt-blue",          "tt-red",          "tt-yellow", "tt-green",     "tt-hl-darkred", "tt-purple",
    "tt-hl-darkblue", "tt-hl-darkyellow", "tt-hl-greenblue", "tt-gray",   "tt-redpurple", "tt-violet",     "tt-orange",
};
static_assert(TEXT_COLOR_WHITE == 0 && static_cast<size_t>(TEXT_COLOR_ORANGE) + 1 == TooltipLineLookCount,
              "ClassNames is indexed by TEXT_COLOR_*");

constexpr std::string_view LineSelector = ".tt-line";
constexpr const char* TooltipStyleSheet = "tooltip.rcss";
constexpr int MaxColorChannel = 255;

std::string_view Trim(std::string_view text)
{
    constexpr std::string_view Spaces = " \t\r\n";
    const size_t first = text.find_first_not_of(Spaces);
    if (first == std::string_view::npos)
    {
        return {};
    }
    return text.substr(first, text.find_last_not_of(Spaces) - first + 1);
}

// The style sheet without its /* comments */.
std::string WithoutComments(std::string_view text)
{
    std::string result;
    size_t at = 0;
    while (at < text.size())
    {
        const size_t open = text.find("/*", at);
        result += text.substr(at, open == std::string_view::npos ? std::string_view::npos : open - at);
        if (open == std::string_view::npos)
        {
            break;
        }
        const size_t close = text.find("*/", open + 2);
        at = close == std::string_view::npos ? text.size() : close + 2;
    }
    return result;
}

// "#rrggbb", "rgb(r, g, b)" or "rgba(r, g, b, a)", its alpha left out; nothing for another value.
std::optional<GroundLabelColor> ReadCssColor(std::string_view value)
{
    value = Trim(value);
    GroundLabelColor color{};
    if (value.starts_with('#'))
    {
        return Data::Items::ReadGroundLabelColor(value, color) ? std::optional(color) : std::nullopt;
    }
    const size_t open = value.find('(');
    if (open == std::string_view::npos || !value.ends_with(')'))
    {
        return std::nullopt;
    }
    const std::string_view function = Trim(value.substr(0, open));
    if (function != "rgb" && function != "rgba")
    {
        return std::nullopt;
    }
    std::string_view arguments = value.substr(open + 1, value.size() - open - 2);
    for (size_t channel = 0; channel < color.size(); ++channel)
    {
        const size_t comma = arguments.find(',');
        if (comma == std::string_view::npos && channel + 1 < color.size())
        {
            return std::nullopt;
        }
        const std::string_view number = Trim(arguments.substr(0, comma));
        int read = 0;
        const auto [end, error] = std::from_chars(number.data(), number.data() + number.size(), read);
        if (error != std::errc() || end != number.data() + number.size() || read < 0 || read > MaxColorChannel)
        {
            return std::nullopt;
        }
        color[channel] = static_cast<BYTE>(read);
        arguments = comma == std::string_view::npos ? std::string_view() : arguments.substr(comma + 1);
    }
    return color;
}

// The TEXT_COLOR_* whose line a selector is the rule of (".tt-line" white, ".tt-line.tt-blue" blue, ...); nothing for
// another selector.
std::optional<size_t> FindLineClass(std::string_view selector)
{
    if (selector == LineSelector)
    {
        return 0;
    }
    if (!selector.starts_with(LineSelector) || selector.size() <= LineSelector.size() + 1 ||
        selector[LineSelector.size()] != '.')
    {
        return std::nullopt;
    }
    const std::string_view lineClass = selector.substr(LineSelector.size() + 1);
    for (size_t index = 1; index < ClassNames.size(); ++index)
    {
        if (ClassNames[index] == lineClass)
        {
            return index;
        }
    }
    return std::nullopt;
}

// The "color" and "background-color" declarations of a rule's body into the look.
void ApplyDeclarations(std::string_view body, TooltipLineLook& look)
{
    while (!body.empty())
    {
        const size_t end = body.find(';');
        const std::string_view declaration = body.substr(0, end);
        body = end == std::string_view::npos ? std::string_view() : body.substr(end + 1);
        const size_t colon = declaration.find(':');
        if (colon == std::string_view::npos)
        {
            continue;
        }
        const std::string_view property = Trim(declaration.substr(0, colon));
        const std::optional<GroundLabelColor> color = ReadCssColor(declaration.substr(colon + 1));
        if (!color)
        {
            continue;
        }
        if (property == "color")
        {
            look.text = *color;
        }
        else if (property == "background-color")
        {
            look.background = *color;
        }
    }
}
} // namespace

std::array<TooltipLineLook, TooltipLineLookCount> ReadTooltipLineLooks(std::string_view styleSheet)
{
    std::array<TooltipLineLook, TooltipLineLookCount> looks{};
    for (size_t index = 0; index < looks.size(); ++index)
    {
        looks[index].className = ClassNames[index];
    }
    const std::string sheet = WithoutComments(styleSheet);
    const std::string_view text(sheet);
    size_t at = 0;
    while (true)
    {
        const size_t open = text.find('{', at);
        const size_t close = open == std::string_view::npos ? open : text.find('}', open);
        if (close == std::string_view::npos)
        {
            break;
        }
        std::string_view selectors = text.substr(at, open - at);
        const std::string_view body = text.substr(open + 1, close - open - 1);
        at = close + 1;
        while (!selectors.empty())
        {
            const size_t comma = selectors.find(',');
            if (const std::optional<size_t> line = FindLineClass(Trim(selectors.substr(0, comma))))
            {
                ApplyDeclarations(body, looks[*line]);
            }
            selectors = comma == std::string_view::npos ? std::string_view() : selectors.substr(comma + 1);
        }
    }
    return looks;
}

const TooltipLineLook& GetTooltipLineLook(int color)
{
    static std::array<TooltipLineLook, TooltipLineLookCount> looks = ReadTooltipLineLooks({});
    static std::optional<std::string> lookTheme;
    const std::string& theme = UI::RmlBridge::GetActiveThemeName();
    if (lookTheme != theme)
    {
        // The theme's own tooltip.rcss, or the one of the theme it inherits it from.
        const std::string path =
            UI::RmlBridge::ResolveThemeResource(UI::RmlBridge::ThemedDocumentSourceUrl(TooltipStyleSheet, theme));
        const std::optional<std::string> styleSheet = Data::Items::Json::ReadTextFile(std::filesystem::u8path(path));
        looks = ReadTooltipLineLooks(styleSheet.value_or(std::string()));
        lookTheme = theme;
    }
    const bool known = color >= 0 && color < static_cast<int>(looks.size());
    return looks[known ? static_cast<size_t>(color) : 0];
}
} // namespace MuEditor::Items

#endif // _EDITOR
