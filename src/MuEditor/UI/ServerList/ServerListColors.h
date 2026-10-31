#pragma once

#ifdef _EDITOR

#include "Data/GameData/ServerListData/ButtonColors.h"
#include "Data/GameData/ServerListData/ServerListData.h"
#include "imgui.h"

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
// "#RRGGBB" as a color; nothing for any other text.
std::optional<ImVec4> ParseShade(std::string_view shade);
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
