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

#ifndef SA_SIGACTION
#define SA_SIGACTION 0x0040
#endif
#ifndef SA_RESETHAND
#define SA_RESETHAND 0x0004
#endif

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

    // Compute name length at compile-time via sizeof trick; "SIGSEGV" is longest at 7 chars
    // Use a loop to find length — strlen() is async-signal-safe per POSIX 2008
    int nameLen = 0;
    {
        const char* p = name;
        while (*p != '\0')
        {
            ++nameLen;
            ++p;
        }
    }

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
    // Dump backtrace symbols to stderr and MuError.log
    // backtrace() and backtrace_symbols_fd() are async-signal-safe on glibc/macOS
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
    if ((oldact->sa_flags & SA_SIGACTION) != 0 && oldact->sa_sigaction != nullptr)
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
    struct sigaction act = {};
    act.sa_sigaction = CrashHandler;
    // SA_SIGACTION: use sa_sigaction (not sa_handler) — receives siginfo_t*
    // SA_RESETHAND: reset to SIG_DFL after first invocation (prevents re-entrant crash)
    act.sa_flags = SA_SIGACTION | SA_RESETHAND;

    sigaction(SIGSEGV, &act, &s_oldSIGSEGV);
    sigaction(SIGABRT, &act, &s_oldSIGABRT);
    sigaction(SIGBUS, &act, &s_oldSIGBUS);

    // Log install confirmation at install time (safe — not inside the handler)
    // AC-STD-5: PLAT: prefix for platform diagnostic messages
    g_ErrorReport.Write(L"PLAT: signal handler -- installed for SIGSEGV, SIGABRT, SIGBUS\r\n");
}

} // namespace mu::platform
