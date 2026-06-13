#pragma once

#include <string>

namespace Core::Platform
{
    // Human-readable operating system name and version for diagnostic overlays
    // and logs, e.g. "Windows 10.0" or "Linux 6.18". Resolved at runtime.
    std::wstring GetOSVersionString();
}
