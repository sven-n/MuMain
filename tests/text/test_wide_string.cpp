#include <cwchar>
#include <iterator>

#include <doctest.h>

#include "Core/Text/WideString.h"

TEST_CASE("AppendWideString adds the whole text when it fits")
{
    wchar_t destination[32] = L"Screenshot saved.";

    CHECK(Core::Text::AppendWideString(destination, std::size(destination), L" [Valhalla]"));
    CHECK(std::wcscmp(destination, L"Screenshot saved. [Valhalla]") == 0);
}

TEST_CASE("AppendWideString cuts the text to what fits and keeps it terminated")
{
    wchar_t destination[8] = L"abc";

    CHECK_FALSE(Core::Text::AppendWideString(destination, std::size(destination), L"defghijk"));
    CHECK(std::wcscmp(destination, L"abcdefg") == 0);
    CHECK(destination[7] == L'\0');
}

TEST_CASE("AppendWideString adds nothing to a full text")
{
    wchar_t destination[4] = L"abc";

    CHECK_FALSE(Core::Text::AppendWideString(destination, std::size(destination), L"d"));
    CHECK(std::wcscmp(destination, L"abc") == 0);
    CHECK(Core::Text::AppendWideString(destination, std::size(destination), L""));
    CHECK(Core::Text::AppendWideString(destination, std::size(destination), nullptr));
    CHECK(std::wcscmp(destination, L"abc") == 0);
}

TEST_CASE("AppendWideString ends a text that has no end")
{
    wchar_t destination[4] = {L'a', L'b', L'c', L'd'};

    CHECK_FALSE(Core::Text::AppendWideString(destination, std::size(destination), L"e"));
    CHECK(std::wcscmp(destination, L"abc") == 0);
}

TEST_CASE("AppendWideString leaves a buffer without room alone")
{
    wchar_t destination = L'x';

    CHECK_FALSE(Core::Text::AppendWideString(&destination, 0, L"abc"));
    CHECK(destination == L'x');
    CHECK_FALSE(Core::Text::AppendWideString(nullptr, 4, L"abc"));
}
