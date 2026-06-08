// Portable DPAPI shim (issue #462, Phase 3).
//
// The config layer encrypts saved credentials with the Windows Data Protection
// API. There is no portable equivalent, and on Windows the types come from the
// platform SDK (pulled in via <imagehlp.h> where the config code uses them), so
// this header only provides non-Windows stubs.
//
// The stubs report failure: CryptProtectData/CryptUnprotectData return FALSE, so
// the caller never stores or reads credentials -- nothing is written in
// plaintext. Wiring up a portable secret store (libsecret / Keychain) is a
// follow-up.
#pragma once

#ifndef _WIN32

#include "Core/Platform/WinCompat.h"  // BOOL, BYTE, DWORD, PVOID, LPCWSTR, LPWSTR

typedef struct _CRYPTOAPI_BLOB {
    DWORD cbData;
    BYTE* pbData;
} DATA_BLOB;

inline BOOL CryptProtectData(DATA_BLOB*, LPCWSTR, DATA_BLOB*, PVOID, void*, DWORD, DATA_BLOB*)
{
    return FALSE;
}
inline BOOL CryptUnprotectData(DATA_BLOB*, LPWSTR*, DATA_BLOB*, PVOID, void*, DWORD, DATA_BLOB*)
{
    return FALSE;
}

// Never reached on Linux (only called on the success path of the Crypt* calls,
// which always fail here), but needed so the call sites compile.
inline void* LocalFree(void* hMem) { return hMem; }

#endif // !_WIN32
