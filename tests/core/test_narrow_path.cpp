#include "doctest.h"

#include "Core/Platform/WinCompat.h"

TEST_CASE("Narrow paths normalize Windows separators [core][platform]")
{
    CHECK(mu_narrow_path(L"data\\shop\\list.txt") == "data/shop/list.txt");
    CHECK(mu_narrow_path("data\\shop\\list.txt") == "data/shop/list.txt");
    CHECK(mu_narrow_path(static_cast<const wchar_t*>(nullptr)).empty());
}

TEST_CASE("Wide paths preserve Unicode as UTF-8 [core][platform]")
{
    const std::string expected = "data/m" "\xC3\xBA" "sica/" "\xE6\x97\xA5\xE6\x9C\xAC" ".txt";
    CHECK(mu_narrow_path(L"data\\m\u00FAsica\\\u65E5\u672C.txt") == expected);
}

TEST_CASE("Wide-to-UTF-8 output handles text and null [core][platform]")
{
    std::string result = "stale";
    mu_wchar_to_utf8(L"m\u00FAsica\\\u65E5\u672C.txt", result);
    CHECK(result == "m" "\xC3\xBA" "sica\\" "\xE6\x97\xA5\xE6\x9C\xAC" ".txt");

    mu_wchar_to_utf8(nullptr, result);
    CHECK(result.empty());
}

TEST_CASE("Wide-to-UTF-8 validates edge scalars [core][platform]")
{
    CHECK(mu_wchar_to_utf8(L"").empty());
    CHECK(mu_wchar_to_utf8(L"\U0001F642") == "\xF0\x9F\x99\x82");

    std::string result = "stale";
    mu_wchar_to_utf8(L"\xD800", result);
    CHECK(result.empty());
}
