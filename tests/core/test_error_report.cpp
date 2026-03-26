// Story 7.1.1: Cross-Platform Error Reporting — ATDD Acceptance Tests
// Flow Code: VS0-QUAL-ERRORREPORT-XPLAT
//
// RED PHASE:  These tests FAIL before story 7.1.1 is implemented.
//             On macOS, the current ErrorReport.cpp calls CreateFile/WriteFile/ReadFile
//             which are Win32 APIs unavailable outside Windows.
//
// GREEN PHASE: All tests PASS after ErrorReport.cpp is refactored to use
//              std::ofstream + std::filesystem::path + std::chrono.
//
// AC coverage:
//   AC-1  — file created at specified path
//   AC-2  — Write() produces readable UTF-8 content
//   AC-4  — WriteCurrentTime() uses std::chrono, output matches YYYY/MM/DD HH:MM
//   AC-STD-2 — Catch2 test suite exists and passes on macOS Clang
//   AC-NFR-1  — Write() overhead < 1ms per typical 256-char message
//   AC-NFR-2  — invalid path does not crash; emits stderr, game continues

#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <regex>
#include <string>

#include "PlatformTypes.h"
#include "ErrorReport.h"

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

namespace
{

/// Return a unique per-test temp file path under the system temp directory.
std::filesystem::path TempLogPath(const char* suffix)
{
    return std::filesystem::temp_directory_path() / (std::string("mu_test_7_1_1_") + suffix + ".log");
}

/// Open the log file at path and read its full UTF-8 content.
std::string ReadLogContent(const std::filesystem::path& path)
{
    std::ifstream f(path);
    return std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
}

/// Teardown helper — ignore errors (file may not exist if test aborted early).
void RemoveQuiet(const std::filesystem::path& path)
{
    std::error_code ec;
    std::filesystem::remove(path, ec);
}

} // namespace

// ---------------------------------------------------------------------------
// AC-1 / AC-2: File creation and UTF-8 write
// ---------------------------------------------------------------------------

TEST_CASE("AC-1/AC-2 [7-1-1]: ErrorReport creates file and writes UTF-8 text",
          "[core][error_report][7-1-1]")
{
    // VS0-QUAL-ERRORREPORT-XPLAT
    auto tmpPath = TempLogPath("write");
    RemoveQuiet(tmpPath);

    {
        CErrorReport report;
        report.Destroy(); // close the auto-created MuError.log from constructor
        report.Create(tmpPath.wstring().c_str());
        report.Write(L"TEST: hello %ls\r\n", L"world");
        report.Destroy();
    }

    // AC-1: file must exist at the specified path
    REQUIRE(std::filesystem::exists(tmpPath));

    // AC-2: Write() produces human-readable UTF-8 text (not raw wchar_t bytes)
    std::string content = ReadLogContent(tmpPath);
    REQUIRE_FALSE(content.empty());
    REQUIRE(content.find("TEST: hello world") != std::string::npos);

    RemoveQuiet(tmpPath);
}

// ---------------------------------------------------------------------------
// AC-2: HexWrite produces readable hex output
// ---------------------------------------------------------------------------

TEST_CASE("AC-2 [7-1-1]: HexWrite produces ASCII hex output readable as UTF-8",
          "[core][error_report][7-1-1]")
{
    // VS0-QUAL-ERRORREPORT-XPLAT
    auto tmpPath = TempLogPath("hexwrite");
    RemoveQuiet(tmpPath);

    {
        CErrorReport report;
        report.Destroy();
        report.Create(tmpPath.wstring().c_str());
        uint8_t buf[] = {0xDE, 0xAD, 0xBE, 0xEF};
        report.HexWrite(buf, static_cast<int>(sizeof(buf)));
        report.Destroy();
    }

    REQUIRE(std::filesystem::exists(tmpPath));
    std::string content = ReadLogContent(tmpPath);
    // HexWrite output must be readable ASCII (no raw wchar_t bytes written)
    REQUIRE(content.find("DE") != std::string::npos);
    REQUIRE(content.find("AD") != std::string::npos);

    RemoveQuiet(tmpPath);
}

// ---------------------------------------------------------------------------
// AC-4: WriteCurrentTime produces YYYY/MM/DD HH:MM format
// ---------------------------------------------------------------------------

TEST_CASE("AC-4 [7-1-1]: WriteCurrentTime formats timestamp as YYYY/MM/DD HH:MM",
          "[core][error_report][7-1-1]")
{
    // VS0-QUAL-ERRORREPORT-XPLAT
    auto tmpPath = TempLogPath("time");
    RemoveQuiet(tmpPath);

    {
        CErrorReport report;
        report.Destroy();
        report.Create(tmpPath.wstring().c_str());
        report.WriteCurrentTime(FALSE); // no trailing newline
        report.Destroy();
    }

    REQUIRE(std::filesystem::exists(tmpPath));
    std::string content = ReadLogContent(tmpPath);
    REQUIRE_FALSE(content.empty());

    // AC-4: must match exactly YYYY/MM/DD HH:MM (std::chrono implementation)
    std::regex tsRegex(R"(\d{4}/\d{2}/\d{2} \d{2}:\d{2})");
    REQUIRE(std::regex_search(content, tsRegex));

    RemoveQuiet(tmpPath);
}

// ---------------------------------------------------------------------------
// AC-1 / CutHead: Log trimmed to 4 sessions when 5+ present
// ---------------------------------------------------------------------------

TEST_CASE("AC-1/CutHead [7-1-1]: CutHead trims log to 4 sessions when 5+ present",
          "[core][error_report][7-1-1]")
{
    // VS0-QUAL-ERRORREPORT-XPLAT
    auto tmpPath = TempLogPath("cuthead");
    RemoveQuiet(tmpPath);

    // Phase 1: write 5 session markers
    {
        CErrorReport report;
        report.Destroy();
        report.Create(tmpPath.wstring().c_str());
        for (int i = 0; i < 5; ++i)
        {
            report.WriteLogBegin();
            report.Write(L"Session content %d\r\n", i);
        }
        report.Destroy();
    }

    // Phase 2: re-opening triggers CutHead — should trim to 4 sessions
    {
        CErrorReport report;
        report.Destroy();
        report.Create(tmpPath.wstring().c_str());
        report.Destroy();
    }

    // Phase 3: verify exactly 4 session markers remain
    std::string content = ReadLogContent(tmpPath);
    const std::string marker = "###### Log Begin ######";
    int count = 0;
    size_t pos = 0;
    while ((pos = content.find(marker, pos)) != std::string::npos)
    {
        ++count;
        pos += marker.size();
    }
    REQUIRE(count == 4);

    RemoveQuiet(tmpPath);
}

// ---------------------------------------------------------------------------
// AC-NFR-2: Invalid path must not crash — graceful failure
// ---------------------------------------------------------------------------

TEST_CASE("AC-NFR-2 [7-1-1]: ErrorReport does not crash on invalid file path",
          "[core][error_report][7-1-1]")
{
    // VS0-QUAL-ERRORREPORT-XPLAT
    // /nonexistent_mu_71_test_dir/ does not exist — Create() must fail gracefully
    // by emitting to stderr and continuing; Write() must silently no-op.
    CErrorReport report;
    report.Destroy();

    REQUIRE_NOTHROW(report.Create(L"/nonexistent_mu_71_test_dir/mu_test_error.log"));
    REQUIRE_NOTHROW(report.Write(L"silent no-op when file not open\r\n"));
    REQUIRE_NOTHROW(report.Destroy());
}

// ---------------------------------------------------------------------------
// ===========================================================================
// Story 7.6.7: ErrorReport Cross-Platform Crash Diagnostics
// Flow Code: VS0-QUAL-WIN32CLEAN-ERRDIAG
//
// RED PHASE:  Tests FAIL (or fail to compile) before 7-6-7 is implemented:
//   - AC-8 test:  compile error — m_lpszGpuBackend field does not yet exist
//                 (currently named m_lpszDxVersion)
//   - AC-3 test:  FAIL — WriteSystemInfo is an empty stub on non-Windows;
//                 OS/CPU fields remain empty and iMemorySize == 0
//   - AC-5 test:  compile error on Windows — WriteImeInfo takes HWND, not SDL_Window*
//   - AC-4 test:  PASS (stub doesn't throw) — behavioral intent documented
//   - AC-6 test:  PASS (stub doesn't throw) — behavioral intent documented
//
// GREEN PHASE: All tests PASS after cross-platform implementation is complete.
//   - AC-8:  m_lpszGpuBackend field exists in ER_SystemInfo
//   - AC-3:  WriteSystemInfo uses uname()/sysctlbyname//proc on POSIX
//   - AC-5:  WriteImeInfo(SDL_Window*) accepted — SDL_TextInputActive used
//   - AC-4:  WriteOpenGLInfo callable without crash on all platforms
//   - AC-6:  WriteSoundCardInfo uses mu::GetAudioDeviceNames() (miniaudio)
// ===========================================================================

#include <SDL3/SDL.h>

// ---------------------------------------------------------------------------
// AC-8: ER_SystemInfo.m_lpszGpuBackend field exists (renamed from m_lpszDxVersion)
// Compile error in RED phase: no member 'm_lpszGpuBackend' in ER_SystemInfo.
// ---------------------------------------------------------------------------

TEST_CASE("AC-8 [7-6-7]: ER_SystemInfo has m_lpszGpuBackend field",
          "[core][error_report][7-6-7]")
{
    // VS0-QUAL-WIN32CLEAN-ERRDIAG
    ER_SystemInfo si{};
    // Accessing m_lpszGpuBackend causes a compile error in RED phase
    // (field is still named m_lpszDxVersion until AC-8 is implemented).
    REQUIRE(sizeof(si.m_lpszGpuBackend) == MAX_GPU_BACKEND_LEN * sizeof(wchar_t));
}

// ---------------------------------------------------------------------------
// AC-3 / AC-STD-2: WriteSystemInfo populates OS name, CPU model, and RAM size.
// Fails in RED phase: empty stub leaves all fields at zero/empty.
// ---------------------------------------------------------------------------

TEST_CASE("AC-3/AC-STD-2 [7-6-7]: WriteSystemInfo populates OS, CPU, and RAM fields",
          "[core][error_report][7-6-7]")
{
    // VS0-QUAL-WIN32CLEAN-ERRDIAG
    ER_SystemInfo si{};

    // MuGetSystemInfo populates the struct fields using cross-platform APIs
    MuGetSystemInfo(&si);

    // Then WriteSystemInfo logs those fields to the error report
    g_ErrorReport.WriteSystemInfo(&si);

    // AC-3: OS field must be non-empty — uname() provides sysname + release on POSIX
    REQUIRE(si.m_lpszOS[0] != L'\0');

    // AC-3: CPU field must be non-empty — sysctlbyname on macOS / /proc/cpuinfo on Linux
    REQUIRE(si.m_lpszCPU[0] != L'\0');

    // AC-3: RAM must be positive — sysctlbyname hw.memsize macOS / /proc/meminfo Linux
    REQUIRE(si.m_iMemorySize > 0);
}

// ---------------------------------------------------------------------------
// AC-5: WriteImeInfo signature changed to WriteImeInfo(SDL_Window*).
// On Windows in RED phase: HWND and SDL_Window* are incompatible — compile error.
// On non-Windows in RED phase: HWND stub may accept nullptr implicitly.
// In GREEN phase: SDL_Window* overload accepted; nullptr handled gracefully.
// ---------------------------------------------------------------------------

TEST_CASE("AC-5 [7-6-7]: WriteImeInfo accepts SDL_Window* and does not crash",
          "[core][error_report][7-6-7]")
{
    // VS0-QUAL-WIN32CLEAN-ERRDIAG
    // No SDL window in unit test context — implementation must handle nullptr.
    // static_cast to SDL_Window* enforces the new signature at compile time.
    REQUIRE_NOTHROW(g_ErrorReport.WriteImeInfo(static_cast<SDL_Window*>(nullptr)));
}

// ---------------------------------------------------------------------------
// AC-4: WriteOpenGLInfo callable on all platforms without crash.
// Without an active GL context glGetString returns nullptr — implementation
// must handle this gracefully (not dereference nullptr).
// In RED phase this test passes (empty stub), but documents expected behavior.
// ---------------------------------------------------------------------------

TEST_CASE("AC-4 [7-6-7]: WriteOpenGLInfo callable on all platforms without crash",
          "[core][error_report][7-6-7]")
{
    // VS0-QUAL-WIN32CLEAN-ERRDIAG
    // Primary verification is ./ctl check (no #ifdef _WIN32 guard wrapping the body).
    // This test documents the no-crash guarantee for headless/unit-test environments.
    REQUIRE_NOTHROW(g_ErrorReport.WriteOpenGLInfo());
}

// ---------------------------------------------------------------------------
// AC-6: WriteSoundCardInfo uses miniaudio (mu::GetAudioDeviceNames) — no
// DirectSoundEnumerate or Win32 types in the implementation.
// In RED phase this test passes (empty stub), but documents expected behavior.
// In GREEN phase ma_context_get_devices may return empty in CI/headless
// environments but must not crash.
// ---------------------------------------------------------------------------

TEST_CASE("AC-6 [7-6-7]: WriteSoundCardInfo uses miniaudio and does not crash",
          "[core][error_report][7-6-7]")
{
    // VS0-QUAL-WIN32CLEAN-ERRDIAG
    REQUIRE_NOTHROW(g_ErrorReport.WriteSoundCardInfo());
}

// ---------------------------------------------------------------------------
// AC-NFR-1: Write() overhead < 1ms per call for a 256-char message
// ---------------------------------------------------------------------------

TEST_CASE("AC-NFR-1 [7-1-1]: Write() overhead is under 1ms per call",
          "[core][error_report][7-1-1]")
{
    // VS0-QUAL-ERRORREPORT-XPLAT
    auto tmpPath = TempLogPath("perf");
    RemoveQuiet(tmpPath);

    {
        CErrorReport report;
        report.Destroy();
        report.Create(tmpPath.wstring().c_str());

        // Warm-up call
        report.Write(L"PERF WARMUP\r\n");

        constexpr int kIterations = 100;
        auto t0 = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < kIterations; ++i)
        {
            // ~256 chars: typical error log line
            report.Write(
                L"PERF: session=%d msg=aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                L"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"
                L"aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\r\n",
                i);
        }
        auto t1 = std::chrono::high_resolution_clock::now();

        report.Destroy();

        auto totalUs = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
        double avgMs = static_cast<double>(totalUs) / (kIterations * 1000.0);

        // AC-NFR-1: must be < 1ms per call on average
        REQUIRE(avgMs < 1.0);
    }

    RemoveQuiet(tmpPath);
}
