// Story 3.3.2: Linux Server Connectivity Validation
// Flow Code: VS1-NET-VALIDATE-LINUX
// Tests .so loading and symbol resolution via mu::platform (PlatformLibrary.h)
// Does NOT require a running OpenMU server at test time.
//
// RED PHASE: Tests skip gracefully when MUnique.Client.Library.so is absent.
//   On Linux with .so present: AC-1 and AC-2 tests exercise mu::platform::Load/GetSymbol.
//   On non-Linux (MinGW/macOS CI): all tests SUCCEED() as no-ops — CI stays green.
//
// AC Coverage:
//   AC-1:       ClientLibrary.so loads via mu::platform::Load on Linux x64
//   AC-2:       All four ConnectionManager exports resolve to non-null pointers
//   AC-STD-2:   Catch2 smoke test for .so loading path resolution (Risk R6 mitigation)
//   AC-STD-11:  Flow Code VS1-NET-VALIDATE-LINUX in test file header
//   AC-STD-NFR-1: Library load via mu::platform::Load() is resilient to binary output dir layout
//   AC-STD-NFR-2: dotnet publish produces MUnique.Client.Library.so with four exports
//   AC-VAL-3:   Catch2 smoke test passes on Linux x64 build

#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <string>

#include "PlatformLibrary.h"

#ifdef __linux__

// MU_TEST_LIBRARY_PATH is injected by CMakeLists.txt as the absolute path to
// MUnique.Client.Library.so in the build output directory.
// If not defined (e.g., dotnet SDK not available, non-Linux CI), tests skip gracefully.
// Note: On Linux, dlopen() does NOT search the executable directory for bare filenames —
// MU_TEST_LIBRARY_PATH must be an absolute path (injected by CMake from CMAKE_RUNTIME_OUTPUT_DIRECTORY).
#ifndef MU_TEST_LIBRARY_PATH
#define MU_TEST_LIBRARY_PATH ""
#endif

// ============================================================================
// AC-1 / AC-STD-2 / AC-STD-NFR-1: ClientLibrary.so loads via mu::platform::Load on Linux
// ============================================================================

TEST_CASE("3.3.2 AC-1: ClientLibrary.so loads via mu::platform::Load on Linux", "[network][dotnet][linux]")
{
    // GIVEN: Absolute path to MUnique.Client.Library.so (CMake-injected or absent)
    // Note: MU_TEST_LIBRARY_PATH must be absolute on Linux — bare filenames do NOT
    // search the executable directory (unlike macOS @rpath). CMakeLists.txt uses
    // CMAKE_RUNTIME_OUTPUT_DIRECTORY to construct the absolute path. (Risk R6 mitigation)
    const std::string libPath = MU_TEST_LIBRARY_PATH;

    if (libPath.empty() || !std::filesystem::exists(libPath))
    {
        SKIP("MUnique.Client.Library.so not found — build ClientLibrary with dotnet publish --runtime linux-x64");
    }

    // WHEN: mu::platform::Load() is called with the absolute .so path
    mu::platform::LibraryHandle handle = mu::platform::Load(libPath.c_str());

    // THEN: Handle is non-null (.so loaded successfully via dlopen)
    REQUIRE(handle != nullptr);

    // CLEANUP:
    mu::platform::Unload(handle);
}

// ============================================================================
// AC-2 / AC-STD-2 / AC-STD-NFR-2: All four ConnectionManager exports resolve
// ============================================================================

TEST_CASE("3.3.2 AC-2: All four ConnectionManager exports resolve", "[network][dotnet][linux]")
{
    // GIVEN: Absolute path to MUnique.Client.Library.so
    const std::string libPath = MU_TEST_LIBRARY_PATH;

    if (libPath.empty() || !std::filesystem::exists(libPath))
    {
        SKIP("MUnique.Client.Library.so not found — build ClientLibrary with dotnet publish --runtime linux-x64");
    }

    // WHEN: Library is loaded
    mu::platform::LibraryHandle handle = mu::platform::Load(libPath.c_str());
    REQUIRE(handle != nullptr);

    // THEN: All four ConnectionManager exports are resolvable (non-null)
    // Note: Linux AOT exports symbols without underscore prefix (unlike Win32 __cdecl).
    // Verify with: nm -gD MUnique.Client.Library.so | grep ConnectionManager
    CHECK(mu::platform::GetSymbol(handle, "ConnectionManager_Connect") != nullptr);
    CHECK(mu::platform::GetSymbol(handle, "ConnectionManager_Disconnect") != nullptr);
    CHECK(mu::platform::GetSymbol(handle, "ConnectionManager_BeginReceive") != nullptr);
    CHECK(mu::platform::GetSymbol(handle, "ConnectionManager_Send") != nullptr);

    // CLEANUP:
    mu::platform::Unload(handle);
}

#else // non-Linux platforms

// ============================================================================
// Non-Linux CI: no-op tests keep CI green (MinGW/macOS do not have .so)
// ============================================================================

TEST_CASE("3.3.2: Linux .so tests skipped on non-Linux platform", "[network][dotnet][linux]")
{
    // No-op: Linux-specific tests only run on __linux__ builds.
    // MinGW CI uses MU_ENABLE_DOTNET=OFF and does not have ClientLibrary.so.
    // macOS uses ClientLibrary.dylib validated by story 3.3.1.
    SUCCEED("Linux-only tests skipped on this platform");
}

#endif // __linux__
