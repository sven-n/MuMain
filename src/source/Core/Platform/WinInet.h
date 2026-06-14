// Portable WinINet type shim (issue #462, Phase 3).
//
// The in-game-shop headers reference a few WinINet types/constants in their
// declarations (a handle, an FTP port, URL/credential buffer sizes). On Windows
// these come from <wininet.h>; elsewhere this provides just those declarations so
// the headers parse. The shop's actual WinINet/urlmon downloader lives in the
// GameShop .cpp files, which are excluded from the non-Windows build.
#pragma once

#ifdef _WIN32

#include <wininet.h>

#else  // ---- non-Windows ----------------------------------------------------

#include "Core/Platform/WinCompat.h"  // WORD

typedef void* HINTERNET;
typedef WORD  INTERNET_PORT;

#ifndef INTERNET_MAX_URL_LENGTH
#define INTERNET_MAX_URL_LENGTH 2084
#endif
#ifndef INTERNET_MAX_USER_NAME_LENGTH
#define INTERNET_MAX_USER_NAME_LENGTH 128
#endif
#ifndef INTERNET_MAX_PASSWORD_LENGTH
#define INTERNET_MAX_PASSWORD_LENGTH 128
#endif
#ifndef INTERNET_DEFAULT_FTP_PORT
#define INTERNET_DEFAULT_FTP_PORT 21
#endif

#endif // _WIN32
