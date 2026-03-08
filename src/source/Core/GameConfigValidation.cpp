// Flow Code: VS1-NET-CONFIG-SERVER
// Story: 3.4.2 - Server Connection Configuration
//
// Definitions of GameConfig validation helpers.
// Auto-discovered by file(GLOB MU_CORE_SOURCES Core/*.cpp) in CMakeLists.txt.
//
// RED PHASE: Stub implementations that compile.
// GREEN PHASE: Real implementations wire in g_ErrorReport.Write() calls
//              (per AC-4 and AC-5 in story.md) during dev-story workflow.

#include "GameConfigValidation.h"
#include "ErrorReport.h"

namespace GameConfig
{

int ValidateServerPort(int value, int defaultValue)
{
    if (value <= 0 || value > 65535)
    {
        g_ErrorReport.Write(L"NET: Invalid ServerPort %d in config.ini \xe2\x80\x94 using default %d\r\n", value,
                            defaultValue);
        return defaultValue;
    }
    return value;
}

std::wstring ValidateServerIP(const std::wstring& value, const std::wstring& defaultValue)
{
    std::wstring trimmed = value;
    trimmed.erase(0, trimmed.find_first_not_of(L" \t\r\n"));
    if (!trimmed.empty())
    {
        trimmed.erase(trimmed.find_last_not_of(L" \t\r\n") + 1);
    }
    if (trimmed.empty())
    {
        g_ErrorReport.Write(L"NET: Empty ServerIP in config.ini \xe2\x80\x94 using default %ls\r\n",
                            defaultValue.c_str());
        return defaultValue;
    }
    return trimmed;
}

} // namespace GameConfig
