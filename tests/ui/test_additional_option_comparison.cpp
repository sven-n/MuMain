#include <doctest.h>
#include "Engine/Object/AdditionalOptionComparison.h"
#include "Engine/Object/CompareItemResult.h"

using Engine::Items::CompareAdditionalOption;
namespace CompareResult = Engine::Items::CompareResult;

namespace
{
constexpr int HighValue = 16;
constexpr int LowValue = 4;
} // namespace

TEST_CASE("a remembered copy with a higher additional option is better than the offered one")
{
    CHECK(CompareAdditionalOption(true, HighValue, true, LowValue) == CompareResult::FirstIsBetter);
}

TEST_CASE("an offered copy with a higher additional option is better than the remembered one")
{
    CHECK(CompareAdditionalOption(true, LowValue, true, HighValue) == CompareResult::SecondIsBetter);
}

TEST_CASE("a copy that lost its additional option is worse")
{
    CHECK(CompareAdditionalOption(true, LowValue, false, 0) == CompareResult::FirstIsBetter);
    CHECK(CompareAdditionalOption(false, 0, true, LowValue) == CompareResult::SecondIsBetter);
}

TEST_CASE("copies with the same additional option, or none, are the same")
{
    CHECK(CompareAdditionalOption(true, LowValue, true, LowValue) == CompareResult::Same);
    CHECK(CompareAdditionalOption(false, 0, false, 0) == CompareResult::Same);
}
