#pragma once
// Flow Code: VS1-NET-CONFIG-SERVER
// Story: 3.4.2 - Server Connection Configuration
//
// Testable validation helpers for GameConfig server connection parameters.
// Extracted from GameConfig.cpp so MuTests (linked against MUCore) can
// validate port/IP logic without requiring the full MUData/MUGame dependency
// chain or a config.ini file on disk.
//
// Pattern: mirrors DotNetMessageFormat.h from story 3.4.1.

#include <string>

// Namespace is distinct from the GameConfig class to avoid naming collision.
// See MEDIUM-3 fix (code review 3-4-2).
namespace GameConfigValidation
{

// AC-4: Validate a ServerPort value read from config.ini.
// Returns value if valid (1..65535); otherwise logs a warning via
// g_ErrorReport.Write() and returns defaultValue.
[[nodiscard]] int ValidateServerPort(int value, int defaultValue);

// AC-5: Validate a ServerIP value read from config.ini.
// Trims leading/trailing whitespace. Returns trimmed value if non-empty;
// otherwise logs a warning via g_ErrorReport.Write() and returns defaultValue.
[[nodiscard]] std::wstring ValidateServerIP(const std::wstring& value, const std::wstring& defaultValue);

} // namespace GameConfigValidation
