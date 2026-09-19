#include "doctest.h"

#include "UI/Legacy/TextSearch.h"

TEST_CASE("wide text search checks every character boundary")
{
    CHECK(UI::TextSearch::Contains(L"alpha \u03A9 beta", L"\u03A9"));
    CHECK(UI::TextSearch::Contains(L"\u6F22\u5B57 suffix", L"\u5B57"));
    CHECK_FALSE(UI::TextSearch::Contains(L"alpha", L"omega"));
}

TEST_CASE("first-only wide text search does not scan later positions")
{
    CHECK(UI::TextSearch::Contains(L"prefix value", L"prefix", true));
    CHECK_FALSE(UI::TextSearch::Contains(L"prefix value", L"value", true));
}
