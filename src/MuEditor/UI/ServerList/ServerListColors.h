#pragma once

#ifdef _EDITOR

#include "Core/Color/Rgb.h"
#include "Data/GameData/ServerListData/ButtonColors.h"
#include "Data/GameData/ServerListData/ServerListData.h"
#include "imgui.h"

#include <algorithm>
#include <functional>
#include <optional>
#include <string>
#include <string_view>

// The server list's colors in the Server List window: swatches in the shades the themes draw
// them in, their names, and the list to pick one from.
namespace MuEditor::ServerList::Colors
{
// The theme the game draws in now.
std::string_view GetGameTheme();
// A token of a theme folder's tokens.ini or its ancestors' ("#FFB400"), read once.
std::optional<std::string> ReadThemeToken(std::string_view theme, const char* token);
// The tokens are read again on their next use: a theme's tokens.ini may have changed.
void ForgetThemeTokens();
// "#RRGGBB" as a color; nothing for any other text (Core::Color::ParseRgb).
std::optional<ImVec4> ParseShade(std::string_view shade);
// A color of 8 bits a channel as ImGui's, and ImGui's red, green and blue rounded to 8 bits a channel.
ImVec4 ToSwatch(Core::Color::Rgb color);
Core::Color::Rgb ToRgb(const float rgb[3]);
// The theme folder a token's value comes from: the theme itself, or one it inherits from;
// nothing when none of them has it.
std::optional<std::string> FindThemeTokenSource(std::string_view theme, const char* token);
// Sets the token in the theme folder's own tokens.ini, in its [Tokens], replacing the lines that
// have it or adding one after the section's last. The lines count as the themes read them: section
// and key in any case, spaces around them, ';' and '#' lines as comments. False when the file can't
// be read or written; it is written through a temporary file (Data::WriteTextFile), so a write that
// fails leaves it as it was. The tokens are read again afterwards (ForgetThemeTokens).
bool WriteThemeToken(std::string_view theme, const char* token, const std::string& value);
// tokens.ini's text with the token set as WriteThemeToken sets it; the text keeps its line ends.
std::string SetTokenInText(std::string_view text, std::string_view token, const std::string& value);
// A token's color as tokens.ini writes it: "#rrggbb", or "rgba(r, g, b, a)" with an alpha.
struct TokenColor
{
    float rgb[3] = {1.0f, 1.0f, 1.0f};
    int alpha = 255;

    bool operator==(const TokenColor& other) const
    {
        return std::equal(rgb, rgb + 3, other.rgb) && alpha == other.alpha;
    }
};
std::optional<TokenColor> ParseTokenColor(std::string_view value);
std::string FormatTokenColor(const TokenColor& color);
// The shade of a token of a theme ("server-load-base"); white when it has none.
ImVec4 GetTokenSwatch(const char* token, std::string_view theme);
// The shade a theme draws a theme color in: the token the server select screen's stylesheets
// use (Data::ServerList::GetThemeColorToken).
ImVec4 GetThemeColorSwatch(Data::ServerList::ThemeColor color, std::string_view theme);
ImVec4 GetButtonColorSwatch(const Data::ServerList::ButtonColor& color, std::string_view theme);
void RenderSwatch(const ImVec4& color);

const char* GetThemeColorLabel(Data::ServerList::ThemeColor color);
// A theme color's label, or a color of the list's own by its name.
std::string GetColorLabel(const Data::ServerList::ServerListData& list, std::string_view color);

// A selectable row with `text` drawn as written. As a Selectable's label, a "##" in a
// name would start its ImGui ID and hide the rest; `id` tells the rows apart.
bool SelectableText(const char* id, const std::string& text, bool selected,
                    ImGuiSelectableFlags flags = ImGuiSelectableFlags_None);

// What a color of the list is drawn in, for its swatch.
using ColorSwatch = std::function<ImVec4(std::string_view color)>;

// A list of the theme colors and the list's own, each with its swatch, `width` wide before the
// editor UI scale. True when another color was picked; it is in `color` then.
bool PickColor(const char* id, const Data::ServerList::ServerListData& list, std::string& color, float width,
               const ColorSwatch& swatch);
} // namespace MuEditor::ServerList::Colors

#endif // _EDITOR
