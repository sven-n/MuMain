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
