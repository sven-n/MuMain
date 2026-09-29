#include <doctest.h>

#include "UI/Party/PartyListLayout.h"

TEST_CASE("party list health bar follows the ten health steps [ui][party]")
{
    using namespace UI::Party::List;
    CHECK(HealthBarLength(0) == doctest::Approx(0.f));
    CHECK(HealthBarLength(5) == doctest::Approx(34.5f));
    CHECK(HealthBarLength(10) == doctest::Approx(69.f));
    CHECK(HealthBarLength(13) == doctest::Approx(69.f));
    CHECK(HealthBarLength(-1) == doctest::Approx(0.f));
}
