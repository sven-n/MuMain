#pragma once

// [VS0-QUAL-SIGNAL-HANDLERS]
// Story 7.1.2: POSIX Signal Handlers for Crash Diagnostics
// AC-5: Signal handler code lives exclusively in Platform/posix/

#ifndef _WIN32

#include <csignal>

namespace mu::platform
{

// Install SIGSEGV, SIGABRT, and SIGBUS signal handlers for crash diagnostics.
// Handlers write signal name and backtrace to stderr and MuError.log (via g_errorReportFd),
// chain to the previous handler (R8 mitigation — preserves .NET AOT handler),
// then call _exit(1).
//
// Must be called after g_ErrorReport is open (i.e., after CErrorReport::Create()).
// Safe to call multiple times — handlers are simply re-installed.
void InstallSignalHandlers();

} // namespace mu::platform

#endif // _WIN32
