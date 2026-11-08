#include "doctest.h"

#include "Core/Text/NumberParse.h"

using Core::Text::TrimWhitespace;
using Core::Text::TryParseDouble;
using Core::Text::TryParseInt;

TEST_CASE("Whitespace is trimmed on both ends, whatever kind it is")
{
    CHECK(TrimWhitespace(L"  60  ") == L"60");
    CHECK(TrimWhitespace(L"\t60\r\n") == L"60");
    CHECK(TrimWhitespace(L"60\r") == L"60");
    CHECK(TrimWhitespace(L"\n 1 2 \n") == L"1 2");
    CHECK(TrimWhitespace(L"") == L"");
    CHECK(TrimWhitespace(L" \t\r\n") == L"");
    CHECK(TrimWhitespace(L"fps") == L"fps");
}

TEST_CASE("A double argument is one number and nothing else")
{
    CHECK(TryParseDouble(L"60") == doctest::Approx(60.0));
    CHECK(TryParseDouble(L" 60 ") == doctest::Approx(60.0));
    CHECK(TryParseDouble(L"60\r\n") == doctest::Approx(60.0));
    CHECK(TryParseDouble(L"-1") == doctest::Approx(-1.0));
    CHECK(TryParseDouble(L"0.5") == doctest::Approx(0.5));

    CHECK_FALSE(TryParseDouble(L""));
    CHECK_FALSE(TryParseDouble(L"   "));
    CHECK_FALSE(TryParseDouble(L"fps"));
    CHECK_FALSE(TryParseDouble(L"60abc"));
    CHECK_FALSE(TryParseDouble(L"abc60"));
    CHECK_FALSE(TryParseDouble(L"1e400"));
}

TEST_CASE("An integer argument is one base-ten integer and nothing else")
{
    CHECK(TryParseInt(L"30") == 30);
    CHECK(TryParseInt(L" 30\n") == 30);
    CHECK(TryParseInt(L"-1") == -1);

    CHECK_FALSE(TryParseInt(L""));
    CHECK_FALSE(TryParseInt(L"2.9"));
    CHECK_FALSE(TryParseInt(L"3e9"));
    CHECK_FALSE(TryParseInt(L"30x"));
    CHECK_FALSE(TryParseInt(L"3000000000"));
    CHECK_FALSE(TryParseInt(L"-3000000000"));
}
