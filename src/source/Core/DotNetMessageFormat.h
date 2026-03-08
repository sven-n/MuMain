#pragma once
// Flow Code: VS1-NET-ERROR-MESSAGING
// Story: 3.4.1 - Connection Error Messaging & Graceful Degradation
//
// Testable formatting helpers for DotNet connection error messages.
// Extracted from Connection.cpp so MuTests (linked against MUCore) can
// validate message text without requiring the full MUGame dependency chain.
//
// Declarations mirror what Connection.cpp exposes in the DotNetBridge namespace.

#include <string>

namespace DotNetBridge
{

// AC-1: Format library-not-found message.
// Returns: "Network library not found at {path}. Build ClientLibrary for {platform} or check build docs."
std::string FormatLibraryNotFoundMessage(const std::string& path, const char* platform);

// AC-2: Format symbol-not-found message.
// Returns: "Network library loaded but function {name} not found. Version mismatch?"
std::string FormatSymbolNotFoundMessage(const char* name);

} // namespace DotNetBridge
