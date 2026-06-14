// Portable DPAPI shim (issue #462, Phase 3).
//
// The config layer encrypts saved credentials with the Windows Data Protection
// API. On Windows the types come from the platform SDK; elsewhere this provides
// a reversible, local-scope substitute so the "remember me" feature works.
//
// Windows DPAPI ties the ciphertext to the local user account. Without a system
// secret store wired up (libsecret / Keychain is a follow-up), this derives a
// key from stable per-machine/per-user material (/etc/machine-id + uid) and
// applies a keystream XOR. That is obfuscation, not strong cryptography - it
// keeps the saved password out of config.ini in plaintext and makes it
// decryptable only on the same machine/user, matching DPAPI's local scope and
// threat model (a casual reader of the config file), not a determined attacker.
#pragma once

#ifndef _WIN32

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <unistd.h>
#include "Core/Platform/WinCompat.h"  // BOOL, BYTE, DWORD, PVOID, LPCWSTR, LPWSTR

typedef struct _CRYPTOAPI_BLOB {
    DWORD cbData;
    BYTE* pbData;
} DATA_BLOB;

namespace mu_dpapi_detail
{
    // A stable 64-bit seed for this machine+user. FNV-1a over an app salt, the
    // uid, and /etc/machine-id, so the same user on the same install always
    // derives the same key (and a different user/machine cannot decrypt).
    inline std::uint64_t KeySeed()
    {
        std::uint64_t h = 1469598103934665603ull;  // FNV offset basis
        auto mix = [&](const void* p, std::size_t n) {
            const auto* b = static_cast<const unsigned char*>(p);
            for (std::size_t i = 0; i < n; ++i) { h ^= b[i]; h *= 1099511628211ull; }
        };

        static const char kSalt[] = "MuMain-credential-store-v1";
        mix(kSalt, sizeof(kSalt));
        const uid_t uid = ::getuid();
        mix(&uid, sizeof(uid));
        if (FILE* f = std::fopen("/etc/machine-id", "rb"))
        {
            char buf[64];
            const std::size_t n = std::fread(buf, 1, sizeof(buf), f);
            std::fclose(f);
            if (n > 0) mix(buf, n);
        }
        return h ? h : 0x9E3779B97F4A7C15ull;
    }

    // XOR the buffer in place with a SplitMix64 keystream seeded from KeySeed().
    // Symmetric: applying it twice restores the original, so the same routine
    // both encrypts and decrypts.
    inline void KeystreamXor(BYTE* data, DWORD len)
    {
        std::uint64_t s = KeySeed();
        for (DWORD i = 0; i < len; )
        {
            s += 0x9E3779B97F4A7C15ull;
            std::uint64_t z = s;
            z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
            z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
            z = z ^ (z >> 31);
            for (int b = 0; b < 8 && i < len; ++b, ++i)
                data[i] ^= static_cast<BYTE>(z >> (b * 8));
        }
    }

    // Encrypt == decrypt for a keystream XOR: copy, transform, hand back a
    // freshly allocated buffer (freed by LocalFree, like the Win32 contract).
    inline BOOL Transform(const DATA_BLOB* in, DATA_BLOB* out)
    {
        if (!in || !out || (!in->pbData && in->cbData != 0)) return FALSE;
        BYTE* buf = static_cast<BYTE*>(std::malloc(in->cbData ? in->cbData : 1));
        if (!buf) return FALSE;
        if (in->cbData) std::memcpy(buf, in->pbData, in->cbData);
        KeystreamXor(buf, in->cbData);
        out->pbData = buf;
        out->cbData = in->cbData;
        return TRUE;
    }
}

inline BOOL CryptProtectData(DATA_BLOB* pDataIn, LPCWSTR, DATA_BLOB*, PVOID, void*, DWORD, DATA_BLOB* pDataOut)
{
    return mu_dpapi_detail::Transform(pDataIn, pDataOut);
}
inline BOOL CryptUnprotectData(DATA_BLOB* pDataIn, LPWSTR*, DATA_BLOB*, PVOID, void*, DWORD, DATA_BLOB* pDataOut)
{
    return mu_dpapi_detail::Transform(pDataIn, pDataOut);
}

// Frees the buffer allocated by Crypt*Data above (Win32 LocalFree returns NULL
// on success).
inline void* LocalFree(void* hMem) { std::free(hMem); return nullptr; }

#endif // !_WIN32
