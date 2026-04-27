#include "doctest.h"

#include "Text/TextLineWrap.h"

#include <cwchar>

TEST_CASE("guards: null or non-positive args return 0")
{
    wchar_t out[4][16] = {};
    CHECK(SeparateTextIntoLines(nullptr, out[0], 4, 16) == 0);
    CHECK(SeparateTextIntoLines(L"x", nullptr, 4, 16) == 0);
    CHECK(SeparateTextIntoLines(L"x", out[0], 0, 16) == 0);
    CHECK(SeparateTextIntoLines(L"x", out[0], 4, 0) == 0);
}

TEST_CASE("empty string produces one empty line")
{
    wchar_t out[4][16] = {};
    CHECK(SeparateTextIntoLines(L"", out[0], 4, 16) == 1);
    CHECK(out[0][0] == L'\0');
}

TEST_CASE("english: short text fits on one line")
{
    wchar_t out[4][16] = {};
    int n = SeparateTextIntoLines(L"Hello", out[0], 4, 16);
    CHECK(n == 1);
    CHECK(wcscmp(out[0], L"Hello") == 0);
}

TEST_CASE("english: wraps at space when line overflows")
{
    wchar_t out[4][10] = {};
    int n = SeparateTextIntoLines(L"Hello brave world", out[0], 4, 10);
    CHECK(n == 3);
    CHECK(wcscmp(out[0], L"Hello") == 0);
    CHECK(wcscmp(out[1], L"brave") == 0);
    CHECK(wcscmp(out[2], L"world") == 0);
}

TEST_CASE("chinese: hard-breaks at character (no spaces)")
{
    // 6 CJK ideographs, 2 visual columns each. iLineSize=5 → 4-col limit → 2 per line.
    wchar_t out[4][5] = {};
    int n = SeparateTextIntoLines(L"你好世界你好", out[0], 4, 5);
    CHECK(n == 3);
    CHECK(wcscmp(out[0], L"你好") == 0);
    CHECK(wcscmp(out[1], L"世界") == 0);
    CHECK(wcscmp(out[2], L"你好") == 0);
}

TEST_CASE("russian: cyrillic counts as single-width and fits without wrap")
{
    // "Привет всем" — 11 chars incl. space. Pre-fix bug counted Cyrillic as 2 cols
    // and would wrap this at iLineSize=12 (limit=11). With the fix it fits on one line.
    wchar_t out[4][12] = {};
    int n = SeparateTextIntoLines(L"Привет всем", out[0], 4, 12);
    CHECK(n == 1);
    CHECK(wcscmp(out[0], L"Привет всем") == 0);
}

TEST_CASE("russian: wraps at space at the right column count")
{
    // Same script, longer text. iLineSize=8 → 7-col limit. Should break at the
    // first space (col 7), not in the middle of a Cyrillic word.
    wchar_t out[4][8] = {};
    int n = SeparateTextIntoLines(L"Привет всем", out[0], 4, 8);
    CHECK(n == 2);
    CHECK(wcscmp(out[0], L"Привет") == 0);
    CHECK(wcscmp(out[1], L"всем") == 0);
}

TEST_CASE("greek: counts as single-width")
{
    // "Καλημέρα κόσμε" — 14 chars incl. space. Single-width each. iLineSize=16 → fits.
    wchar_t out[4][16] = {};
    int n = SeparateTextIntoLines(L"Καλημέρα κόσμε", out[0], 4, 16);
    CHECK(n == 1);
}

TEST_CASE("hebrew: counts as single-width")
{
    // "שלום עולם" — 9 chars incl. space.
    wchar_t out[4][12] = {};
    int n = SeparateTextIntoLines(L"שלום עולם", out[0], 4, 12);
    CHECK(n == 1);
}

TEST_CASE("mixed: latin then cjk hard-breaks at the cjk overflow")
{
    // "ab你好" — visual cols 1+1+2+2 = 6. iLineSize=5 → limit=4 → "ab你" (4 cols),
    // hard break, "好". No space on the line, so a hard break is the only option.
    wchar_t out[4][5] = {};
    int n = SeparateTextIntoLines(L"ab你好", out[0], 4, 5);
    CHECK(n == 2);
    CHECK(wcscmp(out[0], L"ab你") == 0);
    CHECK(wcscmp(out[1], L"好") == 0);
}

TEST_CASE("mixed: latin word then cjk wraps at the preceding space")
{
    // "Hello 你好" — visual cols 1+1+1+1+1+1+2+2 = 10. iLineSize=8 → limit=7.
    // Overflow happens at 你 (cur=6 + 2 > 7). A space was seen at col 6, so we
    // wrap there: row 0 = "Hello" (no trailing space), row 1 = "你好".
    wchar_t out[4][8] = {};
    int n = SeparateTextIntoLines(L"Hello 你好", out[0], 4, 8);
    CHECK(n == 2);
    CHECK(wcscmp(out[0], L"Hello") == 0);
    CHECK(wcscmp(out[1], L"你好") == 0);
}

TEST_CASE("long unbroken word hard-breaks")
{
    wchar_t out[4][5] = {};
    int n = SeparateTextIntoLines(L"abcdefghij", out[0], 4, 5);
    CHECK(n == 3);
    CHECK(wcscmp(out[0], L"abcd") == 0);
    CHECK(wcscmp(out[1], L"efgh") == 0);
    CHECK(wcscmp(out[2], L"ij") == 0);
}

TEST_CASE("iMaxLine cap is honoured")
{
    wchar_t out[2][5] = {};
    int n = SeparateTextIntoLines(L"abcdefghij", out[0], 2, 5);
    CHECK(n == 2);
    CHECK(wcscmp(out[0], L"abcd") == 0);
    CHECK(wcscmp(out[1], L"efgh") == 0);
}
