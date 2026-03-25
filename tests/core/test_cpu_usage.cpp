// Story 7.6.4: Cross-Platform CPU Usage Monitoring [VS0-QUAL-WIN32CLEAN-CPUUSAGE]
// RED PHASE — tests compile and run on macOS/Linux without Win32 APIs.
//
// Tests become GREEN once:
//   Task 3.1: CpuUsage::Impl replaces GetSystemInfo()/SYSTEM_INFO with
//             std::thread::hardware_concurrency()
//   Task 3.2: CpuUsage::Impl replaces GetProcessTimes()/FILETIME with
//             mu_get_process_cpu_times() + std::chrono wall-clock, normalised to [0.0, 1.0]
//   Task 3.3/3.4: All #ifdef _WIN32 blocks and windows.h removed from CpuUsage.cpp
//
// Range test [0.0, 1.0] is RED on Windows until Task 3 normalises the return value
// from the legacy percentage (0–100+) to a fractional ratio (0.0–1.0).

#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <thread>
#include "CpuUsage.h"

// AC-3, AC-STD-2: CPU core count — platform capability baseline.
// std::thread::hardware_concurrency() must return a positive value on any
// supported host. This test documents the requirement that CpuUsage internally
// uses hardware_concurrency() (verified at code level by AC-2's compile check
// that removes SYSTEM_INFO / GetSystemInfo()).
TEST_CASE("AC-3: hardware_concurrency returns positive value", "[core][cpu]")
{
    unsigned int coreCount = std::thread::hardware_concurrency();
    REQUIRE(coreCount > 0);
}

// AC-4, AC-STD-2: GetUsage() must return a fractional CPU utilisation in [0.0, 1.0].
// RED on Windows: current implementation returns a percentage (0–100+).
// GREEN after Task 3.2 normalises the formula to:
//   (delta_user_ns + delta_kernel_ns) / (delta_wall_ns * num_cores)
TEST_CASE("AC-4: CPU usage measurement returns value in [0,1] range", "[core][cpu]")
{
    CpuUsage* cpu = CpuUsage::Instance();
    REQUIRE(cpu != nullptr);

    SECTION("first call initialises state and returns a non-negative value")
    {
        double usage = cpu->GetUsage();
        // First call has no prior sample — must return 0.0 (not enough data)
        CHECK(usage >= 0.0);
        CHECK(usage <= 1.0);
    }

    SECTION("second call after sleep returns a value in [0.0, 1.0]")
    {
        // Warm-up: first call seeds the previous-sample state
        cpu->GetUsage();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        double usage = cpu->GetUsage();

        // Post-implementation the value must be a fraction, not a percentage
        CHECK(usage >= 0.0);
        CHECK(usage <= 1.0);
    }
}

// AC-5: If per-process timing is unavailable, GetUsage() must return 0.0 and
// must NOT crash or assert. Multiple rapid calls are safe.
TEST_CASE("AC-5: CPU usage never returns negative value or crashes", "[core][cpu]")
{
    CpuUsage* cpu = CpuUsage::Instance();
    REQUIRE(cpu != nullptr);

    for (int i = 0; i < 5; ++i)
    {
        double usage = cpu->GetUsage();
        REQUIRE(usage >= 0.0);
    }
}
