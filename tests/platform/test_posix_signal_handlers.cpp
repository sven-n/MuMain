// Story 7.1.2: POSIX Signal Handlers for Crash Diagnostics — ATDD RED Phase
// [VS0-QUAL-SIGNAL-HANDLERS]
//
// AC-STD-2: Catch2 test verifying InstallSignalHandlers() installs SA_SIGINFO
// handlers for SIGSEGV, SIGABRT, SIGBUS. Compile-time guarded with #ifndef _WIN32
// so this is a no-op on MinGW/Windows cross-compile (CI remains green).
//
// AC-VAL-2: sigaction() query after InstallSignalHandlers() confirms SA_SIGINFO flag
// set for all three signals — this test validates that assertion.

#include <catch2/catch_test_macros.hpp>

#ifndef _WIN32

#include <csignal>
#include <signal.h>

#include "posix/PosixSignalHandlers.h"

// Helper: query the current sigaction for a signal and return its sa_flags
static int QuerySigFlags(int signum)
{
    struct sigaction act = {};
    sigaction(signum, nullptr, &act);
    return act.sa_flags;
}

// Helper: query the current sa_sigaction handler pointer for a signal
static void* QuerySigHandler(int signum)
{
    struct sigaction act = {};
    sigaction(signum, nullptr, &act);
    // On platforms where SA_SIGINFO is set, the handler is in sa_sigaction
    return reinterpret_cast<void*>(act.sa_sigaction);
}

TEST_CASE(
    "AC-STD-2,AC-VAL-2: InstallSignalHandlers installs SA_SIGINFO for SIGSEGV/SIGABRT/SIGBUS",
    "[platform][posix][signal]")
{
    // Save old handlers before install so we can restore them after the test
    struct sigaction oldSIGSEGV = {};
    struct sigaction oldSIGABRT = {};
    struct sigaction oldSIGBUS = {};
    sigaction(SIGSEGV, nullptr, &oldSIGSEGV);
    sigaction(SIGABRT, nullptr, &oldSIGABRT);
    sigaction(SIGBUS, nullptr, &oldSIGBUS);

    // AC-1: Install signal handlers (function under test)
    mu::platform::InstallSignalHandlers();

    SECTION("AC-STD-2: SIGSEGV handler has SA_SIGINFO flag set")
    {
        int flags = QuerySigFlags(SIGSEGV);
        REQUIRE((flags & SA_SIGINFO) != 0);
    }

    SECTION("AC-STD-2: SIGABRT handler has SA_SIGINFO flag set")
    {
        int flags = QuerySigFlags(SIGABRT);
        REQUIRE((flags & SA_SIGINFO) != 0);
    }

    SECTION("AC-STD-2: SIGBUS handler has SA_SIGINFO flag set")
    {
        int flags = QuerySigFlags(SIGBUS);
        REQUIRE((flags & SA_SIGINFO) != 0);
    }

    SECTION("AC-STD-2: SIGSEGV sa_sigaction handler pointer is non-null")
    {
        void* handler = QuerySigHandler(SIGSEGV);
        REQUIRE(handler != nullptr);
    }

    SECTION("AC-STD-2: SIGABRT sa_sigaction handler pointer is non-null")
    {
        void* handler = QuerySigHandler(SIGABRT);
        REQUIRE(handler != nullptr);
    }

    SECTION("AC-STD-2: SIGBUS sa_sigaction handler pointer is non-null")
    {
        void* handler = QuerySigHandler(SIGBUS);
        REQUIRE(handler != nullptr);
    }

    SECTION("AC-3: Handlers have SA_RESETHAND flag set (prevents re-entrant crash)")
    {
        // SA_RESETHAND resets to SIG_DFL after first invocation — required per AC-3 design
        // to prevent handler re-entry (e.g., SIGABRT from _exit() path)
        int flagsSEGV = QuerySigFlags(SIGSEGV);
        int flagsABRT = QuerySigFlags(SIGABRT);
        int flagsBUS = QuerySigFlags(SIGBUS);
        REQUIRE((flagsSEGV & SA_RESETHAND) != 0);
        REQUIRE((flagsABRT & SA_RESETHAND) != 0);
        REQUIRE((flagsBUS & SA_RESETHAND) != 0);
    }

    // Restore original handlers after test to avoid interfering with other test cases
    sigaction(SIGSEGV, &oldSIGSEGV, nullptr);
    sigaction(SIGABRT, &oldSIGABRT, nullptr);
    sigaction(SIGBUS, &oldSIGBUS, nullptr);
}

TEST_CASE(
    "AC-5: InstallSignalHandlers is in mu::platform namespace",
    "[platform][posix][signal]")
{
    // Compile-time verification: the function resolves in the mu::platform namespace.
    // If PosixSignalHandlers.h is missing or declares it elsewhere, this won't compile.
    [[maybe_unused]] auto fnPtr = &mu::platform::InstallSignalHandlers;
    SUCCEED("mu::platform::InstallSignalHandlers is accessible");
}

TEST_CASE(
    "AC-STD-NFR-2: InstallSignalHandlers can be called multiple times without crashing",
    "[platform][posix][signal]")
{
    // Idempotency check: calling install twice should not crash.
    // (Handlers are simply re-installed over themselves.)
    struct sigaction oldSIGSEGV = {};
    struct sigaction oldSIGABRT = {};
    struct sigaction oldSIGBUS = {};
    sigaction(SIGSEGV, nullptr, &oldSIGSEGV);
    sigaction(SIGABRT, nullptr, &oldSIGABRT);
    sigaction(SIGBUS, nullptr, &oldSIGBUS);

    mu::platform::InstallSignalHandlers();
    mu::platform::InstallSignalHandlers(); // second call — must not crash

    SUCCEED("Double-install completed without crash");

    // Restore
    sigaction(SIGSEGV, &oldSIGSEGV, nullptr);
    sigaction(SIGABRT, &oldSIGABRT, nullptr);
    sigaction(SIGBUS, &oldSIGBUS, nullptr);
}

#else // _WIN32

// Windows (MinGW CI): stub test body — compiles and trivially passes.
// This keeps CI green while POSIX-only signal handler code is excluded.

TEST_CASE(
    "AC-STD-2,AC-VAL-2: InstallSignalHandlers — POSIX only (Windows stub)",
    "[platform][posix][signal]")
{
    // On Windows, PosixSignalHandlers.h is excluded entirely. This stub ensures
    // the test TU compiles and the test case registers but trivially passes.
    SUCCEED("POSIX signal handler tests skipped on Windows (expected)");
}

#endif // _WIN32
