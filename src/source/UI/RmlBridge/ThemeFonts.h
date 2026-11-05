#pragma once

#include <optional>
#include <string>

namespace UI::RmlBridge
{
class ThemeInheritance;
void LoadThemeFonts(const ThemeInheritance& theme);
void LoadActiveThemeFonts();
void ClearThemeFontCache();
// The player's [UI] Font choice (empty = Default). font-body and font-title follow it when it is a
// loaded family; font-native always names the family the native text renderer draws with; other
// font-* tokens and the Default choice keep the theme's family.
void SetSelectedFontFamily(std::string family);
std::optional<std::string> ResolveThemeFontToken(const ThemeInheritance& theme, const std::string& token);
// Whether the family the font token resolves to ("font-body") has an italic face to draw italic
// text in; without one, RmlUi draws no italic text at all.
bool ThemeFontHasItalic(const ThemeInheritance& theme, const std::string& token);
} // namespace UI::RmlBridge
