#include "doctest.h"

#include "Core/Platform/WinCompat.h"

TEST_CASE("Narrow paths normalize Windows separators [core][platform]")
{
    CHECK(mu_narrow_path(L"data\\shop\\list.txt") == "data/shop/list.txt");
    CHECK(mu_narrow_path("data\\shop\\list.txt") == "data/shop/list.txt");
    CHECK(mu_narrow_path(static_cast<const wchar_t*>(nullptr)).empty());
}
