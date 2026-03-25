// Story 7.6.1: Stub for Windows <imm.h> on non-Windows platforms.
// Windows Input Method Editor APIs are not available on macOS/Linux.
// SDL3 handles text input natively on all platforms.
// [VS0-QUAL-BUILDCOMP-MACOS]
#pragma once
#ifndef _WIN32
#ifndef IME_CMODE_NATIVE
#define IME_CMODE_NATIVE 0x0001
#endif
#endif
