// Flow Code: VS1-NET-ERROR-MESSAGING
// Story: 3.4.1 - Connection Error Messaging & Graceful Degradation
//
// Testable formatting helpers for DotNet connection error messages.
// Compiled into MUCore so MuTests can validate message text without
// requiring the full MUGame/Connection.cpp dependency chain.
// Connection.cpp includes DotNetMessageFormat.h and calls these helpers.

#include "DotNetMessageFormat.h"

namespace DotNetBridge
{

// AC-1: Format library-not-found message.
// message = "Network library not found at {path}. Build ClientLibrary for {platform} or check build docs."
std::string FormatLibraryNotFoundMessage(const std::string& path, const char* platform)
{
    return "Network library not found at " + path + ". Build ClientLibrary for " +
           std::string(platform ? platform : "Unknown") + " or check build docs.";
}

// AC-2: Format symbol-not-found message.
// message = "Network library loaded but function {name} not found. Version mismatch?"
std::string FormatSymbolNotFoundMessage(const char* name)
{
    return std::string("Network library loaded but function ") + (name ? name : "unknown") +
           " not found. Version mismatch?";
}

} // namespace DotNetBridge
