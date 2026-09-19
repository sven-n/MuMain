#include "doctest.h"

#include "Core/Utilities/CpuUsage.h"

TEST_CASE("Windows CPU usage converts FILETIME ticks to microseconds [core][cpu]")
{
    constexpr unsigned long long processTicks100ns = 50'000;
    constexpr long long wallMicroseconds = 10'000;

    CHECK(Core::Utilities::CalculateCpuUsageFromFileTime(processTicks100ns, wallMicroseconds, 1) ==
          doctest::Approx(0.5));
}
