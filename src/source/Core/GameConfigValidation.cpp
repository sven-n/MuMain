// Flow Code: VS1-NET-CONFIG-SERVER
// Story: 3.4.2 - Server Connection Configuration
//
// Definitions of GameConfig validation helpers.
// Auto-discovered by file(GLOB MU_CORE_SOURCES Core/*.cpp) in CMakeLists.txt.

#include "GameConfigValidation.h"
#include "MuLogger.h"

namespace GameConfigValidation
{

int ValidateServerPort(int value, int defaultValue)
{
    if (value <= 0 || value > 65535)
    {
        mu::log::Get("core")->warn("NET: Invalid ServerPort {} in config.ini -- using default {}", value, defaultValue);
        return defaultValue;
    }
    return value;
}

std::wstring ValidateServerIP(const std::wstring& value, const std::wstring& defaultValue)
{
    std::wstring trimmed = value;
    // erase(0, npos) when string is all-whitespace: find_first_not_of returns npos,
    // and erase(0, npos) clears the entire string (standard behaviour). The
    // subsequent !trimmed.empty() guard handles this case correctly.
    trimmed.erase(0, trimmed.find_first_not_of(L" \t\r\n"));
    if (!trimmed.empty())
    {
        trimmed.erase(trimmed.find_last_not_of(L" \t\r\n") + 1);
    }
    if (trimmed.empty())
    {
        mu::log::Get("core")->warn("NET: Empty ServerIP in config.ini -- using default {}",
                                   mu_wchar_to_utf8(defaultValue.c_str()));
        return defaultValue;
    }
    return trimmed;
}

} // namespace GameConfigValidation
