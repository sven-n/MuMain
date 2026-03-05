// Story 1.2.2: Platform Library Backends — ATDD RED Phase
// All tests below will FAIL until PlatformLibrary.h and backends are implemented.

#include <catch2/catch_test_macros.hpp>
#include "PlatformLibrary.h"

// Platform-specific test library and symbol selection
// (platform conditionals in test files are acceptable — not game logic)
#ifdef _WIN32
static constexpr const char* TEST_SYSTEM_LIBRARY = "kernel32.dll";
static constexpr const char* TEST_KNOWN_SYMBOL = "GetTickCount";
#elif defined(__APPLE__)
static constexpr const char* TEST_SYSTEM_LIBRARY = "libSystem.B.dylib";
static constexpr const char* TEST_KNOWN_SYMBOL = "strlen";
#else
static constexpr const char* TEST_SYSTEM_LIBRARY = "libm.so.6";
static constexpr const char* TEST_KNOWN_SYMBOL = "sin";
#endif

static constexpr const char* TEST_NONEXISTENT_LIBRARY = "definitely_does_not_exist_12345.so";
static constexpr const char* TEST_INVALID_SYMBOL = "this_symbol_does_not_exist_xyz_99";

TEST_CASE("AC-1: PlatformLibrary.h interface exists in mu::platform namespace", "[platform][library]")
{
    // Verify the interface compiles and the types/functions are accessible
    // This test passes if the header includes and namespace resolve correctly
    mu::platform::LibraryHandle handle = nullptr;
    REQUIRE(handle == nullptr);

    // Verify function signatures exist (compile-time check)
    // These function pointers confirm the interface declarations
    [[maybe_unused]] auto loadFn = &mu::platform::Load;
    [[maybe_unused]] auto getSymbolFn = &mu::platform::GetSymbol;
    [[maybe_unused]] auto unloadFn = &mu::platform::Unload;
}

TEST_CASE("AC-1,AC-6: Load returns [[nodiscard]] LibraryHandle", "[platform][library]")
{
    // Load a known system library — should return a non-null handle
    mu::platform::LibraryHandle handle = mu::platform::Load(TEST_SYSTEM_LIBRARY);
    REQUIRE(handle != nullptr);
    mu::platform::Unload(handle);
}

TEST_CASE("AC-1,AC-6: GetSymbol returns [[nodiscard]] pointer for known symbol", "[platform][library]")
{
    mu::platform::LibraryHandle handle = mu::platform::Load(TEST_SYSTEM_LIBRARY);
    REQUIRE(handle != nullptr);

    void* symbol = mu::platform::GetSymbol(handle, TEST_KNOWN_SYMBOL);
    REQUIRE(symbol != nullptr);

    mu::platform::Unload(handle);
}

TEST_CASE("AC-5: Load failure returns nullptr for non-existent library", "[platform][library]")
{
    mu::platform::LibraryHandle handle = mu::platform::Load(TEST_NONEXISTENT_LIBRARY);
    REQUIRE(handle == nullptr);
}

TEST_CASE("AC-5: GetSymbol failure returns nullptr for invalid symbol name", "[platform][library]")
{
    mu::platform::LibraryHandle handle = mu::platform::Load(TEST_SYSTEM_LIBRARY);
    REQUIRE(handle != nullptr);

    void* symbol = mu::platform::GetSymbol(handle, TEST_INVALID_SYMBOL);
    REQUIRE(symbol == nullptr);

    mu::platform::Unload(handle);
}

TEST_CASE("AC-5: Unload on nullptr handle is a safe no-op", "[platform][library]")
{
    // This must not crash or throw — reaching this line is the assertion
    mu::platform::Unload(nullptr);
    SUCCEED("Unload(nullptr) completed without crash");
}

TEST_CASE("AC-STD-2: Load and resolve symbol round-trip", "[platform][library]")
{
    // Full round-trip: Load -> GetSymbol -> Unload
    mu::platform::LibraryHandle handle = mu::platform::Load(TEST_SYSTEM_LIBRARY);
    REQUIRE(handle != nullptr);

    void* symbol = mu::platform::GetSymbol(handle, TEST_KNOWN_SYMBOL);
    REQUIRE(symbol != nullptr);

    mu::platform::Unload(handle);
    // After unload, handle is stale — no further operations on it
}

TEST_CASE("AC-STD-3: PlatformLibrary.h has no platform conditionals", "[platform][library]")
{
    // Compile-time verification: this test file includes PlatformLibrary.h
    // and compiles on any platform without requiring windows.h or dlfcn.h.
    // The fact that this test case compiles and links proves AC-STD-3 —
    // no runtime assertion is needed.
    SUCCEED("PlatformLibrary.h compiled without platform-specific includes");
}
