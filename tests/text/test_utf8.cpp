#include "doctest.h"

#include "Core/Text/Utf8.h"

using Core::Text::IsValidUtf8;

TEST_CASE("IsValidUtf8 accepts ASCII and characters of two, three and four bytes [text][utf8]")
{
    CHECK(IsValidUtf8(""));
    CHECK(IsValidUtf8("Budge Dragon"));
    // U+0080, U+00E3 and U+07FF: the first, a letter and the last of two bytes.
    CHECK(IsValidUtf8("\xC2\x80"));
    CHECK(IsValidUtf8("Drag\xC3\xA3o"));
    CHECK(IsValidUtf8("\xDF\xBF"));
    // U+0800, U+20AC, U+D7FF and U+E000 (around the surrogates) and U+FFFF.
    CHECK(IsValidUtf8("\xE0\xA0\x80"));
    CHECK(IsValidUtf8("\xE2\x82\xAC"));
    CHECK(IsValidUtf8("\xED\x9F\xBF"));
    CHECK(IsValidUtf8("\xEE\x80\x80"));
    CHECK(IsValidUtf8("\xEF\xBF\xBF"));
    // U+10000, an emoji (U+1F409) and U+10FFFF, the last code point.
    CHECK(IsValidUtf8("\xF0\x90\x80\x80"));
    CHECK(IsValidUtf8("Dragon \xF0\x9F\x90\x89"));
    CHECK(IsValidUtf8("\xF4\x8F\xBF\xBF"));
}

TEST_CASE("IsValidUtf8 refuses a character in more bytes than it needs [text][utf8]")
{
    // C0 and C1 can only start overlong forms of ASCII.
    CHECK_FALSE(IsValidUtf8("\xC0\x80"));
    CHECK_FALSE(IsValidUtf8("\xC1\xBF"));
    // Below U+0800 in three bytes, below U+10000 in four.
    CHECK_FALSE(IsValidUtf8("\xE0\x80\x80"));
    CHECK_FALSE(IsValidUtf8("\xE0\x9F\xBF"));
    CHECK_FALSE(IsValidUtf8("\xF0\x80\x80\x80"));
    CHECK_FALSE(IsValidUtf8("\xF0\x8F\xBF\xBF"));
}

TEST_CASE("IsValidUtf8 refuses UTF-16 surrogates [text][utf8]")
{
    CHECK_FALSE(IsValidUtf8("\xED\xA0\x80"));
    CHECK_FALSE(IsValidUtf8("\xED\xBF\xBF"));
    // An emoji written as its two surrogates (CESU-8) instead of four bytes.
    CHECK_FALSE(IsValidUtf8("\xED\xA0\xBD\xED\xB8\x80"));
}

TEST_CASE("IsValidUtf8 refuses code points above U+10FFFF [text][utf8]")
{
    CHECK_FALSE(IsValidUtf8("\xF4\x90\x80\x80"));
    // F5 to F7 can only start code points above U+10FFFF.
    CHECK_FALSE(IsValidUtf8("\xF5\x80\x80\x80"));
    CHECK_FALSE(IsValidUtf8("\xF7\xBF\xBF\xBF"));
    // F8 to FF start no character at all.
    CHECK_FALSE(IsValidUtf8("\xF8\x88\x80\x80\x80"));
    CHECK_FALSE(IsValidUtf8("\xFF"));
}

TEST_CASE("IsValidUtf8 refuses a character cut short and a byte that continues nothing [text][utf8]")
{
    CHECK_FALSE(IsValidUtf8("\xC3"));
    CHECK_FALSE(IsValidUtf8("\xE2\x82"));
    CHECK_FALSE(IsValidUtf8("\xF0\x9F\x90"));
    CHECK_FALSE(IsValidUtf8("\xC3"
                            "A"));
    CHECK_FALSE(IsValidUtf8("\x80"));
    // "Dragão" in Windows-1252.
    CHECK_FALSE(IsValidUtf8("Drag\xE3o"));
    // One bad character in the middle of valid text.
    CHECK_FALSE(IsValidUtf8("Drag\xC3\xA3o \xC0\xAF"
                            " Dragon"));
}
