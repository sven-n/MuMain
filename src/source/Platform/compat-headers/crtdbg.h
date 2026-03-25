// Story 7.6.1: Stub for Windows CRT <crtdbg.h> on non-Windows platforms.
// Windows CRT debug macros (_ASSERTE, _CrtSetReportMode) are not available on macOS/Linux.
// [VS0-QUAL-BUILDCOMP-MACOS]
#pragma once
#ifndef _WIN32
#include <assert.h>
#define _ASSERTE(expr) assert(expr)
#define _CrtSetReportMode(type, mode) ((int)0)
#define _CrtSetReportFile(type, file) ((_HFILE)0)
#define _CRTDBG_MODE_DEBUG 0
#define _CRTDBG_MODE_FILE 0
#define _CRTDBG_FILE_STDERR 0
typedef void* _HFILE;
#endif
