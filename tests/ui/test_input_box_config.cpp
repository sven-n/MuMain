#include "App/stdafx.h"
#include "UI/Legacy/UIControls.h"

#include "doctest.h"
#include <limits>

TEST_CASE("input box configuration defaults preserve layout and reset shared state")
{
    const InputBoxConfig config;

    CHECK(config.pos.x == std::numeric_limits<int>::min());
    CHECK(config.pos.y == std::numeric_limits<int>::min());
    CHECK(config.size.cx == 0);
    CHECK(config.size.cy == 0);
    CHECK(config.textLimit == MAX_TEXT_LENGTH);
    CHECK(config.options == UIOPTION_NULL);
    CHECK_FALSE(config.password);
    CHECK(config.textAlpha == 255);
    CHECK(config.backAlpha == 0);
    CHECK(config.selectAlpha == 255);
    CHECK(config.state == UISTATE_NORMAL);
}
