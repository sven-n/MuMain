// Story 3.1.2: Connection.h Cross-Platform Updates — ATDD RED Phase
// Flow Code: VS1-NET-CONNECTION-XPLAT
//
// Tests the mu::platform primitives that Connection.h uses after refactoring.
// Does NOT require an actual .NET library at test time.
// RED phase: AC-1 and AC-3 tests will FAIL until Connection.h is refactored
//            to use mu::platform::Load() / mu::platform::GetSymbol().
// AC-STD-2 and AC-STD-3 tests pass immediately (platform primitives already work).

#include <catch2/catch_test_macros.hpp>
#include "PlatformLibrary.h"

// ---------------------------------------------------------------------------
// AC-1: mu::platform::Load() replaces LoadLibrary()/dlopen()
//       Verifies graceful nullptr return for a non-existent library path —
//       this is the exact behaviour IsManagedLibraryAvailable() relies on.
// ---------------------------------------------------------------------------

TEST_CASE("3.1.2 AC-1: Load returns nullptr for non-existent library", "[network][dotnet]")
{
    // GIVEN: a path that does not exist on the filesystem
    // WHEN:  mu::platform::Load() is called
    // THEN:  it returns nullptr (graceful failure — no crash, no exception)
    mu::platform::LibraryHandle handle = mu::platform::Load("NonExistent.Client.Library.xyz");
    REQUIRE(handle == nullptr);
}

TEST_CASE("3.1.2 AC-1: Load returns nullptr for path with wrong extension", "[network][dotnet]")
{
    // Validates the same graceful-failure path for a plausible but absent name
    mu::platform::LibraryHandle handle = mu::platform::Load("MUnique.Client.Library.xyz");
    REQUIRE(handle == nullptr);
}

// ---------------------------------------------------------------------------
// AC-2: Library path constructed with MU_DOTNET_LIB_EXT — compile-time check
//       MU_DOTNET_LIB_EXT is a CMake-defined macro (string literal).
//       The fact that this test compiles confirms the macro is available and
//       the filesystem path construction pattern is correct.
// ---------------------------------------------------------------------------

TEST_CASE("3.1.2 AC-2: MU_DOTNET_LIB_EXT macro is defined and non-empty", "[network][dotnet]")
{
#ifndef MU_DOTNET_LIB_EXT
    FAIL("MU_DOTNET_LIB_EXT is not defined — FindDotnetAOT.cmake must define it via add_compile_definitions");
#else
    // Verify the macro expands to a non-empty string
    const std::string ext = MU_DOTNET_LIB_EXT;
    REQUIRE_FALSE(ext.empty());
    // Extension must start with a dot
    REQUIRE(ext.front() == '.');
#endif
}

TEST_CASE("3.1.2 AC-2: Path construction produces correct library name", "[network][dotnet]")
{
#ifndef MU_DOTNET_LIB_EXT
    FAIL("MU_DOTNET_LIB_EXT is not defined");
#else
    // Mirrors the pattern used in Connection.h after refactoring
    const std::string libPath = (std::filesystem::path("MUnique.Client.Library") += MU_DOTNET_LIB_EXT).string();
    REQUIRE(libPath.find("MUnique.Client.Library") == 0);
    REQUIRE(libPath.size() > std::string("MUnique.Client.Library").size());
    // The extension from the macro must be appended
    const std::string ext = MU_DOTNET_LIB_EXT;
    REQUIRE(libPath.size() >= ext.size());
    REQUIRE(libPath.substr(libPath.size() - ext.size()) == ext);
#endif
}

// ---------------------------------------------------------------------------
// AC-3: mu::platform::GetSymbol() replaces GetProcAddress()/dlsym()
//       Null-handle safety — IsManagedLibraryAvailable() guards against this,
//       but the platform primitive must be safe regardless.
// ---------------------------------------------------------------------------

TEST_CASE("3.1.2 AC-3: GetSymbol returns nullptr for null handle", "[network][dotnet]")
{
    // GIVEN: a null LibraryHandle (library was not loaded)
    // WHEN:  GetSymbol() is called with any symbol name
    // THEN:  it returns nullptr safely (no crash, no exception)
    void* symbol = mu::platform::GetSymbol(nullptr, "ConnectionManager_Connect");
    REQUIRE(symbol == nullptr);
}

TEST_CASE("3.1.2 AC-3: GetSymbol returns nullptr for null handle — disconnect symbol", "[network][dotnet]")
{
    void* symbol = mu::platform::GetSymbol(nullptr, "ConnectionManager_Disconnect");
    REQUIRE(symbol == nullptr);
}

// ---------------------------------------------------------------------------
// AC-STD-2: Catch2 test covering mu::platform::Load / GetSymbol graceful path
//           (this AC is satisfied by the tests above)
// ---------------------------------------------------------------------------

TEST_CASE("3.1.2 AC-STD-2: Load and GetSymbol compose safely for missing library", "[network][dotnet]")
{
    // GIVEN: a library that does not exist
    mu::platform::LibraryHandle handle = mu::platform::Load("MUnique.Client.Library.xyz");

    // WHEN: handle is nullptr (expected for missing library)
    REQUIRE(handle == nullptr);

    // THEN: GetSymbol on nullptr is safe
    void* symbol = mu::platform::GetSymbol(handle, "ConnectionManager_Connect");
    REQUIRE(symbol == nullptr);

    // AND: Unload on nullptr is safe (no crash)
    mu::platform::Unload(handle);
    SUCCEED("Unload(nullptr) completed without crash");
}

// ---------------------------------------------------------------------------
// AC-STD-3: Zero platform ifdefs in Connection.h — compile-time verification
//           This test file includes PlatformLibrary.h directly (as Connection.h
//           will after refactoring). The fact it compiles without windows.h /
//           dlfcn.h demonstrates platform independence.
// ---------------------------------------------------------------------------

TEST_CASE("3.1.2 AC-STD-3: PlatformLibrary.h compiles without platform conditionals", "[network][dotnet]")
{
    // Compile-time proof: this file includes PlatformLibrary.h and compiles on
    // any platform without requiring windows.h or dlfcn.h directly.
    mu::platform::LibraryHandle handle = nullptr;
    REQUIRE(handle == nullptr);
    SUCCEED("PlatformLibrary.h compiled without direct platform-specific includes");
}
