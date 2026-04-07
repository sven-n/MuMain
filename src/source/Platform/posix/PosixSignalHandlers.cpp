#include "stdafx.h"
#include "PosixSignalHandlers.h"

// [VS0-QUAL-SIGNAL-HANDLERS]
// Story 7.1.2: POSIX Signal Handlers for Crash Diagnostics
//
// AC-STD-NFR-1: Only async-signal-safe functions called inside the handler:
//   write(), backtrace(), backtrace_symbols_fd(), _exit()
// AC-STD-NFR-2: Previous handlers preserved for chaining (R8 mitigation)
// AC-STD-5: PLAT: prefix used in install-time log message

#include <csignal>
#include <cstdlib>
#include <unistd.h>

// Note: SA_SIGINFO (not SA_SIGACTION) is the correct POSIX flag to enable the 3-arg
// signal handler form (siginfo_t* parameter). SA_SIGINFO is defined on both macOS and
// Linux. SA_SIGACTION does not exist on macOS and is not part of the POSIX standard.

#if defined(__APPLE__) || defined(__GLIBC__)
#include <execinfo.h>
#define MU_HAS_BACKTRACE 1
#endif

#include "ErrorReport.h"

namespace mu::platform
{

// Previous handlers stored for chaining (AC-STD-NFR-2 — R8 mitigation)
static struct sigaction s_oldSIGSEGV; // NOLINT
static struct sigaction s_oldSIGABRT; // NOLINT
static struct sigaction s_oldSIGBUS;  // NOLINT

// Async-signal-safe crash handler.
// Only write(), backtrace_symbols_fd(), and _exit() are called here.
// g_ErrorReport.Write() is NOT safe inside a signal handler — do not call it here.
static void CrashHandler(int signum, siginfo_t* info, void* context)
{
    // Re-entry guard: .NET runtime uses SIGSEGV internally — if the previous handler
    // returns (handling a managed null-ref), we must not print a crash report for it.
    // Only print diagnostics on the first genuine crash, not on .NET-internal signals.
    static volatile sig_atomic_t s_inHandler = 0;
    if (s_inHandler)
    {
        // Re-entrant call — chain to previous handler without diagnostics.
        const struct sigaction* old = (signum == SIGABRT)  ? &s_oldSIGABRT
                                      : (signum == SIGBUS) ? &s_oldSIGBUS
                                                           : &s_oldSIGSEGV;
        if ((old->sa_flags & SA_SIGINFO) != 0 && old->sa_sigaction != nullptr)
        {
            old->sa_sigaction(signum, info, context);
            return;
        }
        _exit(1);
    }
    s_inHandler = 1;

    // Determine signal name using compile-time constants (no malloc, no strlen ambiguity)
    const char* name = "SIGSEGV";
    const struct sigaction* oldact = &s_oldSIGSEGV;
    if (signum == SIGABRT)
    {
        name = "SIGABRT";
        oldact = &s_oldSIGABRT;
    }
    else if (signum == SIGBUS)
    {
        name = "SIGBUS";
        oldact = &s_oldSIGBUS;
    }

    // FIX L-1: Use compile-time sizeof() trick to compute name length — avoids a runtime
    // loop in an async-signal context and matches the dev notes recommendation.
    // "SIGSEGV" = 7, "SIGABRT" = 7, "SIGBUS" = 6
    int nameLen = (signum == SIGBUS) ? static_cast<int>(sizeof("SIGBUS") - 1) : static_cast<int>(sizeof("SIGSEGV") - 1);

    // Fixed prefix string (no allocation)
    static const char prefix[] = "PLAT: signal handler -- caught ";
    static const char newline[] = "\n";
    static const int prefixLen = static_cast<int>(sizeof(prefix) - 1);

    // Write to stderr (always available, async-signal-safe)
    write(STDERR_FILENO, prefix, prefixLen);
    write(STDERR_FILENO, name, nameLen);
    write(STDERR_FILENO, newline, 1);

    // Write to MuError.log fd if available (async-signal-safe: write() on raw fd)
    int fd = g_errorReportFd;
    if (fd >= 0)
    {
        write(fd, prefix, prefixLen);
        write(fd, name, nameLen);
        write(fd, newline, 1);
    }

#ifdef MU_HAS_BACKTRACE
    // FIX M-3: backtrace()/backtrace_symbols_fd() are not strictly POSIX async-signal-safe,
    // but are documented as signal-safe by glibc and work in practice on macOS.
    // Risk: may deadlock on macOS if crash occurs inside dyld lock. Accepted trade-off
    // for diagnostic value. (Story 7.1.2 Dev Notes §Async-Signal-Safety)
    void* frames[32];
    int count = backtrace(frames, 32);
    if (count > 0)
    {
        backtrace_symbols_fd(frames, count, STDERR_FILENO);
        if (fd >= 0)
        {
            backtrace_symbols_fd(frames, count, fd);
        }
    }
#endif

    // Chain to previous handler (AC-STD-NFR-2 — R8 mitigation: preserves .NET AOT handler).
    // .NET Native AOT uses SIGSEGV for null-reference checks and GC write barriers on arm64.
    // Its handler modifies the signal context (ucontext_t) to resume at managed exception code,
    // then returns. We must return as well so the kernel applies the modified context.
    // If no previous handler exists, fall through to _exit(1).
    if ((oldact->sa_flags & SA_SIGINFO) != 0 && oldact->sa_sigaction != nullptr)
    {
        s_inHandler = 0; // Reset guard — if the chained handler resumes, we're done.
        oldact->sa_sigaction(signum, info, context);
        return; // Let kernel resume at the modified context (e.g., .NET exception handler)
    }
    else if (oldact->sa_handler != SIG_DFL && oldact->sa_handler != SIG_IGN)
    {
        s_inHandler = 0;
        oldact->sa_handler(signum);
        return;
    }

    // No previous handler handled the signal — terminate.
    // AC-3: Call _exit(1) — not exit() (which runs atexit handlers and may deadlock).
    _exit(1);
}

void InstallSignalHandlers()
{
    // FIX H-1: Guard against double-install. A second call would overwrite s_old* with
    // the CrashHandler pointer itself (the handler installed by the first call), causing
    // recursive CrashHandler invocation on signal delivery (stack overflow).
    // MuPlatform::Initialize() calls this once; the guard makes it safe regardless.
    static bool s_installed = false;
    if (s_installed)
    {
        return;
    }
    s_installed = true;

    struct sigaction act = {};
    // FIX M-2: POSIX specifies sigset_t as an opaque type. All-zeros is not guaranteed
    // to represent an empty signal set. sigemptyset() is the correct POSIX idiom.
    sigemptyset(&act.sa_mask);
    act.sa_sigaction = CrashHandler;
    // SA_SIGINFO: use sa_sigaction (not sa_handler) — receives siginfo_t*
    // NOTE: SA_RESETHAND removed — .NET Native AOT uses SIGSEGV internally for managed
    // null-reference checks and GC write barriers. SA_RESETHAND resets to SIG_DFL after
    // the first signal, preventing .NET's handler from functioning on subsequent signals.
    // Re-entry is prevented by the s_inHandler guard below instead.
    act.sa_flags = SA_SIGINFO;

    // .NET Native AOT uses SIGSEGV internally for managed null-reference checks and GC
    // write barriers (HardwareExceptions.cpp). Installing our own SIGSEGV handler conflicts
    // with .NET's signal handling — our handler fires first, prints diagnostics, and
    // interferes with .NET's context modification needed to resume execution.
    // Skip SIGSEGV when .NET is present; .NET handles it via its own SIGSEGVHandler.
#ifndef MU_ENABLE_SDL3
    sigaction(SIGSEGV, &act, &s_oldSIGSEGV);
#endif
    sigaction(SIGABRT, &act, &s_oldSIGABRT);
    sigaction(SIGBUS, &act, &s_oldSIGBUS);

    // Log install confirmation at install time (safe — not inside the handler)
    // AC-STD-5: PLAT: prefix for platform diagnostic messages
#ifdef MU_ENABLE_SDL3
    g_ErrorReport.Write(L"PLAT: signal handler -- installed for SIGABRT, SIGBUS (SIGSEGV left to .NET AOT)\r\n");
#else
    g_ErrorReport.Write(L"PLAT: signal handler -- installed for SIGSEGV, SIGABRT, SIGBUS\r\n");
#endif
}

} // namespace mu::platform
