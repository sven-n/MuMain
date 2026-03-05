// Story 1.2.1: Platform Abstraction Headers — PlatformCompat Function Shims
// GREEN PHASE: PlatformCompat.h implemented — all assertions pass.

#include "PlatformCompat.h"
#include <catch2/catch_test_macros.hpp>
#include <cstring>
#include <thread>

TEST_CASE("AC-1: PlatformCompat timing shims", "[platform][compat]")
{
    SECTION("timeGetTime returns non-zero milliseconds")
    {
        uint32_t t = timeGetTime();
        REQUIRE(t > 0);
    }

    SECTION("GetTickCount returns non-zero and matches timeGetTime range")
    {
        uint32_t t1 = timeGetTime();
        uint32_t t2 = GetTickCount();
        // Both should be within a few ms of each other
        uint32_t diff = (t2 >= t1) ? (t2 - t1) : (t1 - t2);
        REQUIRE(diff < 100);
    }

    SECTION("timeGetTime advances over time")
    {
        uint32_t t1 = timeGetTime();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        uint32_t t2 = timeGetTime();
        REQUIRE(t2 > t1);
        REQUIRE((t2 - t1) >= 40);
    }
}

TEST_CASE("AC-1: PlatformCompat MessageBoxW stub", "[platform][compat]")
{
    SECTION("MessageBoxW returns IDOK")
    {
        int result = MessageBoxW(nullptr, L"Test", L"Title", MB_OK);
        REQUIRE(result == IDOK);
    }

    SECTION("MB_ constants are defined")
    {
        REQUIRE(MB_OK == 0x00);
        REQUIRE(MB_YESNO == 0x04);
        REQUIRE(MB_OKCANCEL == 0x01);
        REQUIRE(MB_ICONERROR == 0x10);
        REQUIRE(MB_ICONWARNING == 0x30);
    }

    SECTION("ID return value constants are defined")
    {
        REQUIRE(IDOK == 1);
        REQUIRE(IDCANCEL == 2);
        REQUIRE(IDYES == 6);
        REQUIRE(IDNO == 7);
    }
}

TEST_CASE("AC-1: PlatformCompat RtlSecureZeroMemory shim", "[platform][compat]")
{
    SECTION("RtlSecureZeroMemory zeroes a buffer")
    {
        char buf[32];
        std::memset(buf, 0xAB, sizeof(buf));
        RtlSecureZeroMemory(buf, sizeof(buf));
        for (int i = 0; i < 32; ++i)
        {
            REQUIRE(buf[i] == 0);
        }
    }
}

TEST_CASE("AC-1: PlatformCompat file I/O shims", "[platform][compat]")
{
    SECTION("_wfopen_s returns non-zero for non-existent file")
    {
        FILE* f = nullptr;
        errno_t err = _wfopen_s(&f, L"/tmp/nonexistent_test_file_121.txt", L"r");
        REQUIRE(err != 0);
        REQUIRE(f == nullptr);
    }
}
