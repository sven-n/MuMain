#pragma once

#include <string>

namespace Core::Platform
{
    // Human-readable operating system name and version for diagnostic overlays
    // and logs, e.g. "Ubuntu 24.04.3 LTS (Linux 6.18)", "Windows 10.0",
    // "macOS 14.5", "iOS 17.5", or "Android 14". Resolved once and cached.
    std::wstring GetOSVersionString();

    // The Linux distribution name, from /etc/os-release PRETTY_NAME, e.g.
    // "Ubuntu 24.04.3 LTS". Empty on platforms with no distro concept (Windows,
    // macOS, Android) or when it cannot be read. Resolved once and cached.
    std::wstring GetOSDistroName();
}
