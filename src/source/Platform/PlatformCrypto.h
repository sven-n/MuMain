#pragma once

// Cross-platform credential encryption API.
// Story 7.6.3: Replaces Windows DPAPI with AES-256-GCM (OpenSSL) or identity no-op fallback.
// Key derived via PBKDF2(hostname, fixed_salt, 100000, SHA-256) — machine-bound.
// [VS0-QUAL-WIN32CLEAN-DATALAYER]

#include <cstddef>
#include <cstdint>
#include <vector>

// Format version bytes prepended to encrypted blobs for cross-build compatibility.
// Allows OpenSSL and no-op builds to correctly identify and handle each other's data.
inline constexpr uint8_t MU_CRYPTO_VERSION_AES256GCM = 0x01;
inline constexpr uint8_t MU_CRYPTO_VERSION_NOOP = 0x00;

// Encrypt plaintext bytes.
// Output format: [version_byte][payload] where payload depends on implementation.
// OpenSSL: [0x01][12-byte IV][ciphertext][16-byte GCM auth tag]
// No-op:   [0x00][raw bytes]
// Returns true on success.
bool mu_encrypt_blob(const void* pIn, size_t cbIn, std::vector<uint8_t>& out);

// Decrypt a blob produced by mu_encrypt_blob.
// Detects format via version byte prefix; falls back to legacy (no prefix) for compatibility.
// Returns true on success.
bool mu_decrypt_blob(const void* pIn, size_t cbIn, std::vector<uint8_t>& out);
