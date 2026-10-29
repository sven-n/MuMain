#include "stdafx.h"
#include "UI/RmlBridge/ThemeFonts.h"
#include "UI/RmlBridge/ThemeInheritance.h"
#include "UI/RmlBridge/RmlTheme.h"
#include "Core/Platform/BundledFonts.h"
#include "Core/Utilities/StringUtils.h"

#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/FontEngineInterface.h>
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <string_view>
#include <unordered_set>

namespace UI::RmlBridge
{
namespace
{
std::unordered_set<std::string>& LoadedFiles()
{
    static std::unordered_set<std::string> paths;
    return paths;
}

std::string& SelectedFontFamily()
{
    static std::string family;
    return family;
}

// The tokens a selected font replaces. Other font-* tokens keep the theme's family.
bool FollowsSelectedFont(const std::string& token)
{
    return token == "font-body" || token == "font-title";
}

// Names the family the native text renderer measures and draws with, for text drawn in a box the
// native renderer sized (the world labels).
constexpr const char* kNativeFontToken = "font-native";

std::optional<std::string> EnvironmentValue(const std::string& name)
{
#ifdef _WIN32
    const auto wide = StringUtils::NarrowToWide(name);
    const DWORD size = GetEnvironmentVariableW(wide.c_str(), nullptr, 0);
    if (size == 0)
        return std::nullopt;
    std::wstring value(size, L'\0');
    if (GetEnvironmentVariableW(wide.c_str(), value.data(), size) >= size)
        return std::nullopt;
    return StringUtils::WideToNarrow(value.c_str());
#else
    const char* value = std::getenv(name.c_str());
    return value ? std::optional<std::string>(value) : std::nullopt;
#endif
}

std::optional<std::string> ExpandEnvironment(const std::string& source)
{
    std::string result;
    for (size_t at = 0; at < source.size();)
    {
        if (source[at] != '%')
        {
            result += source[at++];
            continue;
        }
        const auto end = source.find('%', at + 1);
        if (end == std::string::npos)
            return std::nullopt;
        const auto variable = source.substr(at + 1, end - at - 1);
        const auto value = EnvironmentValue(variable);
        if (!value)
            return std::nullopt;
        result += *value;
        at = end + 1;
    }
    std::replace(result.begin(), result.end(), '\\', '/');
    return result;
}

bool FamilyAvailable(std::string family, Rml::Style::FontStyle style = Rml::Style::FontStyle::Normal)
{
    auto* fonts = Rml::GetFontEngineInterface();
    if (!fonts)
        return false;
    std::transform(family.begin(), family.end(), family.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    constexpr int probeSize = 16;
    return fonts->GetFontFaceHandle(family, style, Rml::Style::FontWeight::Normal, probeSize) != 0;
}

struct ResolvedFont
{
    std::string family;
    Rml::Style::FontStyle style = Rml::Style::FontStyle::Normal;
};

std::optional<Rml::Style::FontStyle> ConfiguredStyle(const ThemeInheritance& theme, const std::string& familyToken)
{
    const auto setting = theme.GetSetting("Tokens", familyToken + "-style");
    if (!setting)
        return std::nullopt;
    if (setting->value == "italic")
        return Rml::Style::FontStyle::Italic;
    if (setting->value == "normal")
        return Rml::Style::FontStyle::Normal;
    return std::nullopt;
}

// A family token whose theme names its style (`<token>-style`) may resolve to an italic-only family:
// the style token resolves with it. Other tokens need a regular face.
std::optional<ResolvedFont> ResolveFont(const ThemeInheritance& theme, const std::string& familyToken)
{
    const auto configured = ConfiguredStyle(theme, familyToken);
    const bool allowItalicOnly = configured.has_value();
    for (const auto& directory : theme.Directories())
    {
        const auto family = ThemeInheritance::ReadSetting(directory, "Tokens", familyToken);
        if (!family)
            continue;
        if (configured && FamilyAvailable(*family, *configured))
            return ResolvedFont{*family, *configured};
        if (FamilyAvailable(*family))
            return ResolvedFont{*family, Rml::Style::FontStyle::Normal};
        if (allowItalicOnly && FamilyAvailable(*family, Rml::Style::FontStyle::Italic))
            return ResolvedFont{*family, Rml::Style::FontStyle::Italic};
    }
    return std::nullopt;
}

// How many `-fallback` tokens one resolution follows; a longer chain (or a loop) ends at the bundled face.
constexpr int kMaxFallbackDepth = 4;

std::optional<std::string> ResolveToken(const ThemeInheritance& theme, const std::string& token, int fallbackDepth);

// The family token the theme names as `<token>-fallback`, resolved with its own style, for when none of
// the token's own families is available (the map banner falls back to the title font).
std::optional<ResolvedFont> ResolveFallbackToken(const ThemeInheritance& theme, const std::string& familyToken,
                                                 int fallbackDepth)
{
    const auto fallback = theme.GetSetting("Tokens", familyToken + "-fallback");
    if (!fallback || fallbackDepth >= kMaxFallbackDepth)
        return std::nullopt;
    const auto family = ResolveToken(theme, fallback->value, fallbackDepth + 1);
    if (!family)
        return std::nullopt;
    const auto style = ResolveToken(theme, fallback->value + "-style", fallbackDepth + 1);
    return ResolvedFont{*family, style == "italic" ? Rml::Style::FontStyle::Italic : Rml::Style::FontStyle::Normal};
}

std::optional<std::string> ResolveToken(const ThemeInheritance& theme, const std::string& token, int fallbackDepth)
{
    if (!token.starts_with("font-") || !theme.IsValid())
        return std::nullopt;
    const std::string& selected = SelectedFontFamily();
    if (token == kNativeFontToken)
    {
        const std::string native = ResolveBundledFont(selected).family;
        if (FamilyAvailable(native))
            return native;
    }
    else if (FollowsSelectedFont(token) && !selected.empty() && FamilyAvailable(selected))
    {
        return selected;
    }

    constexpr std::string_view styleSuffix = "-style";
    const bool isStyle = token.ends_with(styleSuffix);
    const std::string familyToken = isStyle ? token.substr(0, token.size() - styleSuffix.size()) : token;
    if (isStyle && FollowsSelectedFont(familyToken) && !selected.empty() && FamilyAvailable(selected))
    {
        const auto configured = ConfiguredStyle(theme, familyToken);
        if (configured && FamilyAvailable(selected, *configured))
            return *configured == Rml::Style::FontStyle::Italic ? "italic" : "normal";
        for (const auto style : {Rml::Style::FontStyle::Normal, Rml::Style::FontStyle::Italic})
            if (FamilyAvailable(selected, style))
                return style == Rml::Style::FontStyle::Italic ? "italic" : "normal";
    }
    auto resolved = ResolveFont(theme, familyToken);
    if (!resolved)
        resolved = ResolveFallbackToken(theme, familyToken, fallbackDepth);
    if (resolved)
    {
        if (isStyle)
            return resolved->style == Rml::Style::FontStyle::Italic ? "italic" : "normal";
        return resolved->family;
    }

    constexpr const char* bundledFamily = "Liberation Sans";
    if (isStyle)
        return "normal";
    return FamilyAvailable(bundledFamily) ? std::optional<std::string>(bundledFamily) : std::nullopt;
}
} // namespace

void ClearThemeFontCache()
{
    LoadedFiles().clear();
}

void LoadThemeFonts(const ThemeInheritance& theme)
{
    for (const auto& [key, setting] : theme.Section("Fonts"))
    {
        if (setting.value.empty())
            continue;
        const auto expanded = ExpandEnvironment(setting.value);
        if (!expanded)
            continue;
        auto path = std::filesystem::u8path(*expanded);
        if (!path.is_absolute())
            path = setting.directory / path;
        const std::string name = ThemePathUtf8(std::filesystem::absolute(path).lexically_normal());
        std::error_code error;
        if (LoadedFiles().contains(name) || !std::filesystem::is_regular_file(path, error))
            continue;
        LoadedFiles().insert(name);
        if (!Rml::LoadFontFace(name))
            g_ErrorReport.Write(L"> [RmlTheme] Unable to load optional font '%hs'.\r\n", name.c_str());
    }
}

void LoadActiveThemeFonts()
{
    LoadThemeFonts(ThemeInheritance(std::filesystem::path("Data/Interface/RmlUi/themes") / GetActiveThemeName()));
}

void SetSelectedFontFamily(std::string family)
{
    SelectedFontFamily() = std::move(family);
}

std::optional<std::string> ResolveThemeFontToken(const ThemeInheritance& theme, const std::string& token)
{
    return ResolveToken(theme, token, 0);
}
} // namespace UI::RmlBridge
