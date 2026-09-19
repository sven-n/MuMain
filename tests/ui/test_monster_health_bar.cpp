#include <doctest.h>

#include "UI/Combat/MonsterHealthBar.h"

TEST_CASE("selected monster health eligibility [ui][health_bar]")
{
    CHECK(UI::Combat::HealthBar::ShouldRenderSelected(1.0f));
    CHECK(UI::Combat::HealthBar::ShouldRenderSelected(0.25f));
    CHECK(UI::Combat::HealthBar::ShouldRenderSelected(-1.0f));
    CHECK_FALSE(UI::Combat::HealthBar::ShouldRenderSelected(0.0f));
}
