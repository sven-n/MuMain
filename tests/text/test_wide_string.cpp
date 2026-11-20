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

TEST_CASE("CutWideStringToUtf8Bytes keeps text that fits")
{
    wchar_t text[16] = L"Hello";

    CHECK_FALSE(Core::Text::CutWideStringToUtf8Bytes(text, 5));
    CHECK(std::wcscmp(text, L"Hello") == 0);
}

TEST_CASE("CutWideStringToUtf8Bytes counts letters outside ASCII in their UTF-8 bytes")
{
    // Georgian letters (U+10D0..) are three bytes each: seven bytes hold two of them, not three.
    wchar_t text[16] = L"\u10D0\u10D1\u10D2";

    CHECK(Core::Text::CutWideStringToUtf8Bytes(text, 7));
    CHECK(std::wcscmp(text, L"\u10D0\u10D1") == 0);
}

TEST_CASE("CutWideStringToUtf8Bytes mixes one, two and three byte characters")
{
    // 'a' (1) + U+00E3 (2) + U+20AC (3) = 6 bytes.
    wchar_t text[16] = L"a\u00E3\u20ACb";

    CHECK(Core::Text::CutWideStringToUtf8Bytes(text, 6));
    CHECK(std::wcscmp(text, L"a\u00E3\u20AC") == 0);
}

TEST_CASE("CutWideStringToUtf8Bytes never splits a character outside the BMP")
{
    // An emoji is four UTF-8 bytes (a surrogate pair where wchar_t is 16 bits).
    wchar_t text[16] = L"ab\U0001F409";

    CHECK(Core::Text::CutWideStringToUtf8Bytes(text, 5));
    CHECK(std::wcscmp(text, L"ab") == 0);

    wchar_t whole[16] = L"ab\U0001F409";
    CHECK_FALSE(Core::Text::CutWideStringToUtf8Bytes(whole, 6));
    CHECK(std::wcscmp(whole, L"ab\U0001F409") == 0);
}

TEST_CASE("CutWideStringToUtf8Bytes accepts a null text")
{
    CHECK_FALSE(Core::Text::CutWideStringToUtf8Bytes(nullptr, 4));
}
