#include "GameConfigValidation.h"

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
