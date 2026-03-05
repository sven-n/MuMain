// Story 1.2.1: Platform Abstraction Headers — Type Size Assertions
// RED PHASE: These tests will FAIL until PlatformTypes.h is implemented.

#include "PlatformTypes.h"
#include <catch2/catch_test_macros.hpp>
#include <cstdint>

TEST_CASE("AC-2: PlatformTypes type size assertions", "[platform][types]")
{
    SECTION("DWORD is 4 bytes")
    {
        REQUIRE(sizeof(DWORD) == 4);
    }

    SECTION("BOOL is same size as int")
    {
        REQUIRE(sizeof(BOOL) == sizeof(int));
    }

    SECTION("BYTE is 1 byte")
    {
        REQUIRE(sizeof(BYTE) == 1);
    }

    SECTION("WORD is 2 bytes")
    {
        REQUIRE(sizeof(WORD) == 2);
    }
}

TEST_CASE("AC-2: PlatformTypes constants and macros", "[platform][types]")
{
    SECTION("TRUE and FALSE are defined correctly")
    {
        REQUIRE(TRUE == 1);
        REQUIRE(FALSE == 0);
    }

    SECTION("MAX_PATH is 260")
    {
        REQUIRE(MAX_PATH == 260);
    }

    SECTION("LOWORD extracts low 16 bits")
    {
        DWORD_PTR val = 0x12345678;
        REQUIRE(LOWORD(val) == 0x5678);
    }

    SECTION("HIWORD extracts high 16 bits")
    {
        DWORD_PTR val = 0x12345678;
        REQUIRE(HIWORD(val) == 0x1234);
    }

    SECTION("ZeroMemory zeroes a buffer")
    {
        char buf[16];
        for (int i = 0; i < 16; ++i)
        {
            buf[i] = static_cast<char>(0xFF);
        }
        ZeroMemory(buf, 16);
        for (int i = 0; i < 16; ++i)
        {
            REQUIRE(buf[i] == 0);
        }
    }
}

TEST_CASE("AC-2: PlatformTypes handle types are pointer-sized", "[platform][types]")
{
    SECTION("HANDLE is pointer-sized")
    {
        REQUIRE(sizeof(HANDLE) == sizeof(void*));
    }

    SECTION("HWND is pointer-sized")
    {
        REQUIRE(sizeof(HWND) == sizeof(void*));
    }

    SECTION("HDC is pointer-sized")
    {
        REQUIRE(sizeof(HDC) == sizeof(void*));
    }

    SECTION("HINSTANCE is pointer-sized")
    {
        REQUIRE(sizeof(HINSTANCE) == sizeof(void*));
    }

    SECTION("HFONT is pointer-sized")
    {
        REQUIRE(sizeof(HFONT) == sizeof(void*));
    }

    SECTION("HGLRC is pointer-sized")
    {
        REQUIRE(sizeof(HGLRC) == sizeof(void*));
    }
}

TEST_CASE("AC-2: PlatformTypes integral parameter types", "[platform][types]")
{
    SECTION("LPARAM is intptr_t-sized")
    {
        REQUIRE(sizeof(LPARAM) == sizeof(intptr_t));
    }

    SECTION("WPARAM is uintptr_t-sized")
    {
        REQUIRE(sizeof(WPARAM) == sizeof(uintptr_t));
    }

    SECTION("LRESULT is intptr_t-sized")
    {
        REQUIRE(sizeof(LRESULT) == sizeof(intptr_t));
    }

    SECTION("HRESULT is long-sized")
    {
        REQUIRE(sizeof(HRESULT) == sizeof(long));
    }
}
