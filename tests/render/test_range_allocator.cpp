#include <doctest.h>

#include "Render/Renderer/RangeAllocator.h"

TEST_CASE("ranges are handed out first-fit until the buffer is full [render][range_allocator]")
{
    Render::RangeAllocator ranges(100);

    CHECK(ranges.Allocate(40) == 0u);
    CHECK(ranges.Allocate(40) == 40u);
    CHECK_FALSE(ranges.Allocate(30).has_value());
    CHECK(ranges.Allocate(20) == 80u);
    CHECK_FALSE(ranges.Allocate(1).has_value());
    CHECK_FALSE(ranges.Allocate(0).has_value());
}

TEST_CASE("a freed range is reused from the lowest offset that fits [render][range_allocator]")
{
    Render::RangeAllocator ranges(100);
    REQUIRE(ranges.Allocate(10) == 0u);
    REQUIRE(ranges.Allocate(30) == 10u);
    REQUIRE(ranges.Allocate(60) == 40u);

    ranges.Free(0, 10);
    ranges.Free(40, 60);
    CHECK(ranges.Allocate(20) == 40u);
    CHECK(ranges.Allocate(10) == 0u);
}

TEST_CASE("freed neighbours merge into one range [render][range_allocator]")
{
    Render::RangeAllocator ranges(90);
    REQUIRE(ranges.Allocate(30) == 0u);
    REQUIRE(ranges.Allocate(30) == 30u);
    REQUIRE(ranges.Allocate(30) == 60u);

    ranges.Free(0, 30);
    ranges.Free(60, 30);
    CHECK_FALSE(ranges.Allocate(60).has_value());

    ranges.Free(30, 30);
    CHECK(ranges.Allocate(90) == 0u);
}
