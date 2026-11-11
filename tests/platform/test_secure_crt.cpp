#include <doctest.h>

#include <cwchar>

#include "Core/Platform/SecureCrt.h"

#ifndef _WIN32

TEST_CASE("_snwprintf_s with _TRUNCATE terminates a truncated result [platform][securecrt]")
{
    wchar_t buffer[8];
    wmemset(buffer, L'X', 8);

    const int result = _snwprintf_s(buffer, 8, _TRUNCATE, L"%ls", L"abcdefghijkl");

    CHECK(result == -1);
    CHECK(std::wcscmp(buffer, L"abcdefg") == 0);
}

TEST_CASE("_snwprintf_s with _TRUNCATE keeps a result that fits [platform][securecrt]")
{
    wchar_t buffer[8];
    wmemset(buffer, L'X', 8);

    const int result = _snwprintf_s(buffer, 8, _TRUNCATE, L"%d-%ls", 7, L"ab");

    CHECK(result == 4);
    CHECK(std::wcscmp(buffer, L"7-ab") == 0);
}

#endif
