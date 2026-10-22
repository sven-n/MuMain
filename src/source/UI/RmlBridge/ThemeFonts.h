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
} // namespace UI::RmlBridge
