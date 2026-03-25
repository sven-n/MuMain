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

    // Chain to previous handler (AC-STD-NFR-2 — R8 mitigation: preserves .NET AOT handler)
    // Only chain if the previous handler is not SIG_DFL or SIG_IGN
    if ((oldact->sa_flags & SA_SIGINFO) != 0 && oldact->sa_sigaction != nullptr)
    {
        oldact->sa_sigaction(signum, info, context);
    }
    else if (oldact->sa_handler != SIG_DFL && oldact->sa_handler != SIG_IGN)
    {
        oldact->sa_handler(signum);
    }

    // AC-3: Call _exit(1) — not exit() (which runs atexit handlers and may deadlock)
    // SA_RESETHAND (set in InstallSignalHandlers) ensures this signal reverts to SIG_DFL
    // after the first invocation, preventing handler re-entry.
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
    // SA_RESETHAND: reset to SIG_DFL after first invocation (prevents re-entrant crash)
    act.sa_flags = SA_SIGINFO | SA_RESETHAND;

    sigaction(SIGSEGV, &act, &s_oldSIGSEGV);
    sigaction(SIGABRT, &act, &s_oldSIGABRT);
    sigaction(SIGBUS, &act, &s_oldSIGBUS);

    // Log install confirmation at install time (safe — not inside the handler)
    // AC-STD-5: PLAT: prefix for platform diagnostic messages
    g_ErrorReport.Write(L"PLAT: signal handler -- installed for SIGSEGV, SIGABRT, SIGBUS\r\n");
}

} // namespace mu::platform
