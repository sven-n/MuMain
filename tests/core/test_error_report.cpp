// Story 7.6.7: Cross-Platform System Info Diagnostics
// Flow Code: VS0-QUAL-WIN32CLEAN-ERRDIAG
//
// Originally tested CErrorReport methods (Story 7.1.1) — those were removed when
// CErrorReport was replaced by spdlog/MuLogger (Story 7.10.1).
// Remaining tests verify MuSystemInfo (extracted from ErrorReport in 7.10.1).
//
// AC coverage (from 7-6-7):
//   AC-8  — MuSystemInfo.m_lpszGpuBackend field exists
//   AC-3  — MuGetSystemInfo populates OS, CPU, RAM

#include <catch2/catch_test_macros.hpp>

#include "MuSystemInfo.h"

// ---------------------------------------------------------------------------
// AC-8: MuSystemInfo.m_lpszGpuBackend field exists (renamed from ER_SystemInfo)
// ---------------------------------------------------------------------------

TEST_CASE("AC-8 [7-6-7]: MuSystemInfo has m_lpszGpuBackend field",
          "[core][system_info][7-6-7]")
{
    // VS0-QUAL-WIN32CLEAN-ERRDIAG
    MuSystemInfo si{};
    REQUIRE(sizeof(si.m_lpszGpuBackend) == MAX_GPU_BACKEND_LEN * sizeof(wchar_t));
}

// ---------------------------------------------------------------------------
// AC-3 / AC-STD-2: MuGetSystemInfo populates OS name, CPU model, and RAM size.
// ---------------------------------------------------------------------------

TEST_CASE("AC-3/AC-STD-2 [7-6-7]: MuGetSystemInfo populates OS, CPU, and RAM fields",
          "[core][system_info][7-6-7]")
{
    // VS0-QUAL-WIN32CLEAN-ERRDIAG
    MuSystemInfo si{};

    MuGetSystemInfo(&si);

    // AC-3: OS field must be non-empty
    REQUIRE(si.m_lpszOS[0] != L'\0');

    // AC-3: CPU field must be non-empty
    REQUIRE(si.m_lpszCPU[0] != L'\0');

    // AC-3: RAM must be positive
    REQUIRE(si.m_iMemorySize > 0);
}
