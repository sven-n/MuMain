#pragma once

// Compile-time identity of the running binary: which configuration, compiler and architecture
// produced it, and when. Read by the $details overlay and by the benchmark exports, which both
// need to state what was measured -- a frame time without a build configuration next to it is
// not a result anybody can act on.
//
// The git fields are optional: the build system does not compute them, so a build script that
// wants them in the exports defines MU_GIT_DESCRIBE (and MU_GIT_DIRTY when the tree was not
// clean). Absent, the exports say "unknown" rather than inventing a commit.

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

    inline constexpr const char* kArchitecture =
#if defined(_WIN64) || defined(__x86_64__) || defined(__aarch64__)
        "x64";
#else
        "x86";
#endif

    inline constexpr const char* kDate = __DATE__;
    inline constexpr const char* kTime = __TIME__;

#ifdef MU_GIT_DESCRIBE
    inline constexpr const char* kGitDescribe = MU_GIT_DESCRIBE;
#else
    inline constexpr const char* kGitDescribe = "";
#endif

#ifdef MU_GIT_DIRTY
    inline constexpr bool kGitDirty = true;
#else
    inline constexpr bool kGitDirty = false;
#endif
}
