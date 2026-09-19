#include "DotNetMessageFormat.h"

namespace DotNetBridge
{

std::string FormatLibraryNotFoundMessage(const std::string& path, const char* platform)
{
    return "Network library not found at " + path + ". Build ClientLibrary for " +
           std::string(platform ? platform : "Unknown") + " or check build docs.";
}

std::string FormatSymbolNotFoundMessage(const char* name)
{
    return std::string("Network library loaded but function ") + (name ? name : "unknown") +
           " not found. Version mismatch?";
}

} // namespace DotNetBridge
