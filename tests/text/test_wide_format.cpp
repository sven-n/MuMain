#include <doctest.h>

#include <cwchar>

#include "Core/Text/WideFormat.h"

TEST_CASE("AppendFormatted appends to the text already in the buffer [text][format]")
{
    wchar_t buffer[32] = L"Rate 50%";

    Core::Text::AppendFormatted(buffer, L" + %d%%", 10);

    CHECK(std::wcscmp(buffer, L"Rate 50% + 10%") == 0);
}

TEST_CASE("AppendFormatted truncates and keeps the buffer terminated [text][format]")
{
    wchar_t buffer[8] = L"abc";

    Core::Text::AppendFormatted(buffer, L"%ls", L"defghijkl");

    CHECK(std::wcscmp(buffer, L"abcdefg") == 0);
}

TEST_CASE("AppendFormatted leaves a full buffer untouched [text][format]")
{
    wchar_t buffer[4] = L"abc";

    Core::Text::AppendFormatted(buffer, L"%d", 1);

    CHECK(std::wcscmp(buffer, L"abc") == 0);
}
