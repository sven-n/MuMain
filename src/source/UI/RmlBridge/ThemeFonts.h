#pragma once

#include <optional>
#include <string>

namespace UI::RmlBridge
{
class ThemeInheritance;
void LoadThemeFonts(const ThemeInheritance& theme);
void LoadActiveThemeFonts();
void ClearThemeFontCache();
std::optional<std::string> ResolveThemeFontToken(const ThemeInheritance& theme, const std::string& token);
} // namespace UI::RmlBridge
