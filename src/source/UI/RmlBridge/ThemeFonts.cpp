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

bool FamilyAvailable(std::string family)
{
    auto* fonts = Rml::GetFontEngineInterface();
    if (!fonts)
        return false;
    std::transform(family.begin(), family.end(), family.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    constexpr int probeSize = 16;
    return fonts->GetFontFaceHandle(family, Rml::Style::FontStyle::Normal,
                                    Rml::Style::FontWeight::Normal, probeSize) != 0;
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
    for (const auto& directory : theme.Directories())
    {
        const auto family = ThemeInheritance::ReadSetting(directory, "Tokens", token);
        if (family && FamilyAvailable(*family))
            return family;
    }
    constexpr const char* bundledFamily = "Liberation Sans";
    return FamilyAvailable(bundledFamily) ? std::optional<std::string>(bundledFamily) : std::nullopt;
}
} // namespace UI::RmlBridge
