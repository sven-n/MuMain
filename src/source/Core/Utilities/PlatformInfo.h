#pragma once

#include <filesystem>
#include <string>

namespace Core::Platform
{
// Human-readable operating system name and version for diagnostic overlays
// and logs, e.g. "Ubuntu 24.04.3 LTS (Linux 6.18)", "Windows 10.0",
// "macOS 14.5", "iOS 17.5", or "Android 14". Resolved once and cached.
const std::wstring& GetOSVersionString();

// The OS line for logs: GetOSVersionString() plus what the overlay leaves out, which on Linux is the full
// kernel release, e.g. "Ubuntu 24.04.3 LTS (Linux 6.6.87.2-microsoft-standard-WSL2)". Resolved once and cached.
const std::wstring& GetOSVersionDetails();

// The Linux distribution name, from /etc/os-release PRETTY_NAME, e.g.
// "Ubuntu 24.04.3 LTS". Empty on platforms with no distro concept (Windows,
// macOS, Android) or when it cannot be read. Resolved once and cached.
std::wstring GetOSDistroName();

// The processor's brand name, e.g. "AMD Ryzen 7 5800X 8-Core Processor" or "Apple M2". Empty where the
// platform doesn't report one. Resolved once and cached.
const std::string& GetProcessorName();

// Installed system memory in MiB, or 0 if unknown.
int GetSystemMemoryMB();

// The running executable's file as an absolute path, e.g. "D:/MuMain/Main.exe"; empty when it can't be found.
std::filesystem::path GetExecutablePath();
}
