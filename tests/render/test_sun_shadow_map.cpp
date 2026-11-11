#include "stdafx.h"

#include <doctest.h>

#include "Render/Renderer/SdlGpuSunShadowMap.h"

// Without a device every allocation fails, as it does when graphics memory runs out.
TEST_CASE("a sun shadow map that fails to allocate gives way until another size is wanted [render][sun_shadow]")
{
    mu::SdlGpuSunShadowMap map;
    map.Want(2048u);
    CHECK_FALSE(map.WantedSizeFailed());

    map.Request(2048u);
    CHECK(map.PrepareMap() == nullptr);
    CHECK(map.WantedSizeFailed());
    CHECK_FALSE(map.IsAvailable());

    // The same size stays failed from frame to frame ...
    map.Cancel();
    map.Want(2048u);
    CHECK(map.WantedSizeFailed());

    // ... and another size may be tried.
    map.Want(1024u);
    CHECK_FALSE(map.WantedSizeFailed());
}

TEST_CASE("without a request the map's failure stands and nothing is allocated [render][sun_shadow]")
{
    mu::SdlGpuSunShadowMap map;
    map.Want(4096u);
    CHECK(map.PrepareMap() == nullptr);
    CHECK_FALSE(map.WantedSizeFailed());
    CHECK(map.Bound() == nullptr);
}
