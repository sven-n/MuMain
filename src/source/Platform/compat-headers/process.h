// Story 7.6.1: Stub for Windows CRT <process.h> on non-Windows platforms.
// Windows process management APIs (_beginthread, _getpid) are not available on macOS/Linux.
// [VS0-QUAL-BUILDCOMP-MACOS]
#pragma once
#ifndef _WIN32
#include <unistd.h>
inline int _getpid() { return getpid(); }
#endif
