#include "GameConfigValidation.h"

#include "GameConfigConstants.h"
#include "Core/Text/Utf8Path.h"
#include "Render/PostProcessing/RenderScale.h"

#include <algorithm>
#include <array>
#include <cwctype>
#include <utility>

namespace GameConfigValidation
{
namespace
{
// `text`, surrounding blanks ignored, is `name` in any case.
bool EqualsSettingName(std::wstring_view text, std::wstring_view name)
{
    const auto first = text.find_first_not_of(L" 	");
    if (first == std::wstring_view::npos)
    {
        return false;
    }
    text = text.substr(first, text.find_last_not_of(L" 	") - first + 1);
    return text.size() == name.size() && std::equal(text.begin(), text.end(), name.begin(), [](wchar_t a, wchar_t b)
                                                    { return std::towlower(a) == std::towlower(b); });
}

constexpr std::array<std::pair<WindowMode, const wchar_t*>, 3> WindowModeNames{{
    {WindowMode::Windowed, L"Windowed"},
    {WindowMode::Borderless, L"Borderless"},
    {WindowMode::Fullscreen, L"Fullscreen"},
}};
} // namespace

int ValidateServerPort(int value, int defaultPort)
{
    return value > 0 && value <= 65535 ? value : defaultPort;
}

std::wstring ValidateServerIP(const std::wstring& value, const std::wstring& defaultIP)
{
    const auto first = value.find_first_not_of(L" \t\r\n");
    if (first == std::wstring::npos)
    {
        return defaultIP;
    }

    const auto last = value.find_last_not_of(L" \t\r\n");
    return value.substr(first, last - first + 1);
}

int ValidateAnisotropy(int value)
{
    return value < 1 ? 1 : value;
}

int ValidateAntialiasing(int value)
{
    if (value >= 8)
    {
        return 8;
    }
    if (value >= 4)
    {
        return 4;
    }
    if (value >= 2)
    {
        return 2;
    }
    return 0;
}

int ValidateRenderScale(int value)
{
    return std::clamp(value, Render::PostProcessing::MinRenderScalePercent,
                      Render::PostProcessing::MaxRenderScalePercent);
}

int ValidateSharpness(int value)
{
    return std::clamp(value, Render::PostProcessing::MinSharpnessPercent, Render::PostProcessing::MaxSharpnessPercent);
}

std::optional<LookPreset> ParseLook(std::wstring_view text)
{
    if (EqualsSettingName(text, L"Classic"))
    {
        return LookPreset::Classic;
    }
    if (EqualsSettingName(text, L"Enhanced"))
    {
        return LookPreset::Enhanced;
    }
    return std::nullopt;
}

const wchar_t* FormatLook(LookPreset preset)
{
    return preset == LookPreset::Enhanced ? L"Enhanced" : L"Classic";
}

LookPreset ValidateLook(std::wstring_view text)
{
    if (const std::optional<LookPreset> preset = ParseLook(text))
    {
        return *preset;
    }
    return *ParseLook(CfgDefaults::CfgDefaultLook);
}

std::optional<WindowMode> ParseWindowMode(std::wstring_view text)
{
    for (const auto& [mode, name] : WindowModeNames)
    {
        if (EqualsSettingName(text, name))
        {
            return mode;
        }
    }
    return std::nullopt;
}

const wchar_t* FormatWindowMode(WindowMode mode)
{
    for (const auto& [each, name] : WindowModeNames)
    {
        if (each == mode)
        {
            return name;
        }
    }
    return CfgDefaults::CfgDefaultWindowMode;
}

WindowMode ResolveWindowMode(std::wstring_view modeText, std::optional<bool> legacyWindowed)
{
    if (const std::optional<WindowMode> mode = ParseWindowMode(modeText))
    {
        return *mode;
    }
    if (legacyWindowed.has_value())
    {
        // Older clients asked for the monitor's mode when not windowed.
        return *legacyWindowed ? WindowMode::Windowed : WindowMode::Fullscreen;
    }
    return *ParseWindowMode(CfgDefaults::CfgDefaultWindowMode);
}

std::wstring ValidateRenderBackend(const std::wstring& value, const std::wstring& defaultValue)
{
    const auto first = value.find_first_not_of(L" \t\r\n");
    if (first == std::wstring::npos)
    {
        return defaultValue;
    }

    const auto last = value.find_last_not_of(L" \t\r\n");
    std::wstring trimmed = value.substr(first, last - first + 1);
    std::transform(trimmed.begin(), trimmed.end(), trimmed.begin(),
                    [](wchar_t c) { return static_cast<wchar_t>(std::towlower(c)); });

    if (trimmed == L"d3d12")
    {
        return L"direct3d12";
    }
    return trimmed;
}

std::filesystem::path ResolveConfigPath(const std::filesystem::path& userFolder, const char* overridePath)
{
    if (overridePath != nullptr && overridePath[0] != '\0')
    {
        return Core::Text::PathFromUtf8(overridePath);
    }
    return userFolder / CfgFiles::CfgFileName;
}
}
