#include "GameConfigValidation.h"

#include "GameConfigConstants.h"

#include <algorithm>
#include <cwctype>

namespace GameConfigValidation
{
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

std::optional<LookPreset> ParseLook(std::wstring_view text)
{
    const auto first = text.find_first_not_of(L" \t");
    if (first == std::wstring_view::npos)
    {
        return std::nullopt;
    }
    text = text.substr(first, text.find_last_not_of(L" \t") - first + 1);
    const auto equals = [text](std::wstring_view name)
    {
        return text.size() == name.size() && std::equal(text.begin(), text.end(), name.begin(), [](wchar_t a, wchar_t b)
                                                        { return std::towlower(a) == std::towlower(b); });
    };
    if (equals(L"Classic"))
    {
        return LookPreset::Classic;
    }
    if (equals(L"Enhanced"))
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
}
