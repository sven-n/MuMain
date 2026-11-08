#pragma once

// Compile-time identity of the running binary: which configuration, compiler and architecture
// produced it, and when. Read by the $details overlay and by the benchmark exports, which both
// need to state what was measured -- a frame time without a build configuration next to it is
// not a result anybody can act on.
//
// The commit comes from MuGitCommit.h, which cmake/WriteGitCommit.cmake writes for every build.

namespace Core::Build
{
inline constexpr const char* kConfiguration =
#if defined(_DEBUG) || defined(DEBUG)
    "Debug";
#else
    "Release";
#endif

inline constexpr const char* kEditor =
#ifdef _EDITOR
    "Editor";
#else
    "NoEditor";
#endif

inline constexpr const char* kCompiler =
#if defined(__MINGW32__) || defined(__MINGW64__)
    "MinGW";
#elif defined(__clang__)
    "Clang";
#elif defined(_MSC_VER)
            "MSVC";
#elif defined(__GNUC__)
            "GCC";
#else
            "Unknown";
#endif

// By the compiler's CPU macros (_WIN64 is also defined on Windows on ARM); others say "unknown", not a wrong name.
// ARM64EC first: MSVC defines _M_X64 for it, and not _M_ARM64.
inline constexpr const char* kArchitecture =
#if defined(_M_ARM64EC)
    "arm64ec";
#elif defined(_M_ARM64) || defined(__aarch64__)
    "arm64";
#elif defined(_M_X64) || defined(__x86_64__)
    "x64";
#elif defined(_M_IX86) || defined(__i386__)
    "x86";
#elif defined(_M_ARM) || defined(__arm__)
    "arm";
#else
        "unknown";
#endif

inline constexpr const char* kDate = __DATE__;
inline constexpr const char* kTime = __TIME__;
} // namespace Core::Build
