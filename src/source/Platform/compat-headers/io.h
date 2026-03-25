// Story 7.6.1: Stub for Windows <io.h> on non-Windows platforms.
// Windows <io.h> provides low-level I/O (_open, _read, _write, _close).
// POSIX equivalents are in <unistd.h> and <fcntl.h>.
// [VS0-QUAL-BUILDCOMP-MACOS]
#pragma once
#ifndef _WIN32
#include <unistd.h>
#include <fcntl.h>
#endif
