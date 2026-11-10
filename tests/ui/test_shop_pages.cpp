// Tests of how many pages the cash shop's package list fills.

#include "doctest.h"

#include "GameShop/ShopPages.h"

using GameShop::CountPages;

TEST_CASE("the cash shop's package pages round up and never drop below one [ui][cash-shop]")
{
    constexpr int PageSize = 9;
    CHECK(CountPages(0, PageSize) == 1);
    CHECK(CountPages(1, PageSize) == 1);
    CHECK(CountPages(9, PageSize) == 1);
    CHECK(CountPages(10, PageSize) == 2);
    CHECK(CountPages(18, PageSize) == 2);
    CHECK(CountPages(19, PageSize) == 3);
}
