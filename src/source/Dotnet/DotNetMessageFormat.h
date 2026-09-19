#pragma once

#include <string>

namespace DotNetBridge
{

std::string FormatLibraryNotFoundMessage(const std::string& path, const char* platform);
std::string FormatSymbolNotFoundMessage(const char* name);

} // namespace DotNetBridge
