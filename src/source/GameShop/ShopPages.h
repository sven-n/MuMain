#pragma once

#include <algorithm>

namespace GameShop
{
// The pages a list of `items` fills at `pageSize` per page: rounded up, and at least one page so an
// empty list still shows "1 / 1".
[[nodiscard]] constexpr int CountPages(int items, int pageSize)
{
    return std::max(1, (items + pageSize - 1) / pageSize);
}
} // namespace GameShop
