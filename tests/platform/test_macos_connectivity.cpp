// Story 3.3.1: macOS Server Connectivity Validation
// Flow Code: VS1-NET-VALIDATE-MACOS
// Tests dylib loading and symbol resolution via mu::platform (PlatformLibrary.h)
// Does NOT require a running OpenMU server at test time.
//
// RED PHASE: Tests skip gracefully when MUnique.Client.Library.dylib is absent.
//   On macOS with dylib present: AC-1 and AC-2 tests exercise mu::platform::Load/GetSymbol.
//   On non-Apple (MinGW/Linux CI): all tests SUCCEED() as no-ops — CI stays green.
//
// AC Coverage:
//   AC-1:       ClientLibrary.dylib loads via mu::platform::Load on macOS arm64
//   AC-2:       All four ConnectionManager exports resolve to non-null pointers
//   AC-STD-2:   Catch2 smoke test for dylib loading path resolution
//   AC-STD-11:  Flow Code VS1-NET-VALIDATE-MACOS in test file header
//   AC-VAL-3:   Catch2 smoke test passes on macOS arm64 build

#include <catch2/catch_test_macros.hpp>
#include <filesystem>
#include <string>

#include "PlatformLibrary.h"

#ifdef __APPLE__

// MU_TEST_LIBRARY_PATH is injected by CMakeLists.txt as the absolute path to
// MUnique.Client.Library.dylib in the build output directory.
// If not defined (e.g., dotnet SDK not available, non-macOS CI), tests skip gracefully.
#ifndef MU_TEST_LIBRARY_PATH
#define MU_TEST_LIBRARY_PATH ""
#endif

// ============================================================================
// AC-1 / AC-STD-2: ClientLibrary.dylib loads via mu::platform::Load on macOS
// ============================================================================

TEST_CASE("3.3.1 AC-1: ClientLibrary.dylib loads via mu::platform::Load on macOS", "[network][dotnet][macos]")
{
    // GIVEN: Path to MUnique.Client.Library.dylib (CMake-injected or absent)
    const std::string libPath = MU_TEST_LIBRARY_PATH;

    if (libPath.empty() || !std::filesystem::exists(libPath))
    {
        SKIP("MUnique.Client.Library.dylib not found — build ClientLibrary with dotnet publish --runtime osx-arm64");
    }

    // WHEN: mu::platform::Load() is called with the dylib path
    mu::platform::LibraryHandle handle = mu::platform::Load(libPath.c_str());

    // THEN: Handle is non-null (dylib loaded successfully)
    REQUIRE(handle != nullptr);

    // CLEANUP:
    mu::platform::Unload(handle);
}

// ============================================================================
// AC-2 / AC-STD-2: All four ConnectionManager exports resolve
// ============================================================================

TEST_CASE("3.3.1 AC-2: All four ConnectionManager exports resolve", "[network][dotnet][macos]")
{
    // GIVEN: Path to MUnique.Client.Library.dylib
    const std::string libPath = MU_TEST_LIBRARY_PATH;

    if (libPath.empty() || !std::filesystem::exists(libPath))
    {
        SKIP("MUnique.Client.Library.dylib not found — build ClientLibrary with dotnet publish --runtime osx-arm64");
    }

    // WHEN: Library is loaded
    // LOW-1 fix (Story 3.3.1): use a RAII guard so Unload() is always called even if
    // REQUIRE or CHECK aborts the test via Catch2 exception. Without this, a REQUIRE
    // failure would skip the Unload() call, leaving the handle open in sanitizer runs.
    mu::platform::LibraryHandle handle = mu::platform::Load(libPath.c_str());
    struct HandleGuard
    {
        mu::platform::LibraryHandle h;
        ~HandleGuard()
        {
            if (h)
            {
                mu::platform::Unload(h);
            }
        }
    } guard{handle};

    REQUIRE(handle != nullptr);

    // THEN: All four ConnectionManager exports are resolvable (non-null)
    CHECK(mu::platform::GetSymbol(handle, "ConnectionManager_Connect") != nullptr);
    CHECK(mu::platform::GetSymbol(handle, "ConnectionManager_Disconnect") != nullptr);
    CHECK(mu::platform::GetSymbol(handle, "ConnectionManager_BeginReceive") != nullptr);
    CHECK(mu::platform::GetSymbol(handle, "ConnectionManager_Send") != nullptr);

    // CLEANUP: handled by HandleGuard RAII destructor above.
}

#else // non-Apple platforms

// ============================================================================
// Non-Apple CI: no-op tests keep CI green (MinGW/Linux do not have .dylib)
// ============================================================================

TEST_CASE("3.3.1: macOS dylib tests skipped on non-Apple platform", "[network][dotnet][macos]")
{
    // No-op: macOS-specific tests only run on __APPLE__ builds.
    // MinGW CI uses MU_ENABLE_DOTNET=OFF and does not have ClientLibrary.dylib.
    SUCCEED("macOS-only tests skipped on this platform");
}

#endif // __APPLE__
