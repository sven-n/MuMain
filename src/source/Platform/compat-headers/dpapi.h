// Story 7.6.1: Stub for Windows <dpapi.h> on non-Windows platforms.
// Windows Data Protection API (CryptProtectData) is not available on macOS/Linux.
// [VS0-QUAL-BUILDCOMP-MACOS]
#pragma once
#ifndef _WIN32
// dpapi.h is empty on non-Windows — DPAPI calls are in Windows-only code paths.
#endif
