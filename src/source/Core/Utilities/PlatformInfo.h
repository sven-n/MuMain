#pragma once

#include <string>

namespace Core::Platform
{
    // Human-readable operating system name and version for diagnostic overlays
    // and logs, e.g. "Windows 10.0", "Linux 6.18", "macOS 14.5", "iOS 17.5", or
    // "Android 14". Resolved once at runtime per platform and cached.
    std::wstring GetOSVersionString();
}
