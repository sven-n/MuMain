// Cross-platform credential encryption implementation.
// Story 7.6.3: mu_encrypt_blob / mu_decrypt_blob replace Windows DPAPI.
// AES-256-GCM via OpenSSL when MU_HAS_OPENSSL is defined; identity no-op fallback otherwise.
// [VS0-QUAL-WIN32CLEAN-DATALAYER]

#include "stdafx.h"

#include "Platform/PlatformCrypto.h"

#include <atomic>
#include <climits>
#include <cstdio>
#include <cstring>

#ifndef _WIN32
#include <unistd.h>
#endif

#if defined(MU_HAS_OPENSSL)

#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

// AES-256-GCM constants
static constexpr int MU_CRYPTO_KEY_LEN = 32; // 256-bit key
static constexpr int MU_CRYPTO_IV_LEN = 12;  // 96-bit IV (GCM recommended)
static constexpr int MU_CRYPTO_TAG_LEN = 16; // 128-bit auth tag
static constexpr int MU_CRYPTO_SALT_LEN = 16;
static constexpr int MU_CRYPTO_PBKDF2_ITERATIONS = 100000;

// Fixed salt for PBKDF2 key derivation (not secret — machine binding comes from hostname)
static constexpr uint8_t MU_CRYPTO_FIXED_SALT[MU_CRYPTO_SALT_LEN] = {0x4D, 0x75, 0x4F, 0x6E, 0x6C, 0x69, 0x6E, 0x65,
                                                                     0x43, 0x6F, 0x6E, 0x66, 0x69, 0x67, 0x53, 0x61};

namespace mu_crypto_detail
{

static bool DeriveKey(uint8_t* keyOut)
{
    char hostname[256] = {};
#ifdef _WIN32
    DWORD size = sizeof(hostname);
    if (!GetComputerNameA(hostname, &size))
    {
        strncpy(hostname, "MuOnlineDefault", sizeof(hostname) - 1);
    }
#else
    if (gethostname(hostname, sizeof(hostname)) != 0)
    {
        strncpy(hostname, "MuOnlineDefault", sizeof(hostname) - 1);
    }
#endif
    hostname[sizeof(hostname) - 1] = '\0';

    bool ok = PKCS5_PBKDF2_HMAC(hostname, static_cast<int>(strlen(hostname)), MU_CRYPTO_FIXED_SALT, MU_CRYPTO_SALT_LEN,
                                MU_CRYPTO_PBKDF2_ITERATIONS, EVP_sha256(), MU_CRYPTO_KEY_LEN, keyOut) == 1;

    // Zero hostname after use for crypto hygiene (LOW-2 fix)
    OPENSSL_cleanse(hostname, sizeof(hostname));

    return ok;
}

// Cache the derived key — hostname and salt are fixed for process lifetime, so the key
// never changes. Avoids 100k PBKDF2 iterations on every encrypt/decrypt call. (HIGH-1 fix)
static const uint8_t* GetCachedKey()
{
    static uint8_t cachedKey[MU_CRYPTO_KEY_LEN] = {};
    static bool derived = false;
    if (!derived)
    {
        if (!DeriveKey(cachedKey))
        {
            return nullptr;
        }
        derived = true;
    }
    return cachedKey;
}

} // namespace mu_crypto_detail

bool mu_encrypt_blob(const void* pIn, size_t cbIn, std::vector<uint8_t>& out)
{
    out.clear();

    if (pIn == nullptr || cbIn == 0)
    {
        return false;
    }

    // EVP_EncryptUpdate takes int for length; guard against overflow
    if (cbIn > static_cast<size_t>(INT_MAX))
    {
        return false;
    }

    const uint8_t* key = mu_crypto_detail::GetCachedKey();
    if (key == nullptr)
    {
        return false;
    }

    uint8_t iv[MU_CRYPTO_IV_LEN];
    if (RAND_bytes(iv, MU_CRYPTO_IV_LEN) != 1)
    {
        return false;
    }

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (ctx == nullptr)
    {
        return false;
    }

    bool ok = false;
    do
    {
        if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1)
            break;
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, MU_CRYPTO_IV_LEN, nullptr) != 1)
            break;
        if (EVP_EncryptInit_ex(ctx, nullptr, nullptr, key, iv) != 1)
            break;

        // Reserve: version_byte + IV + ciphertext (same size as plaintext) + tag
        out.resize(1 + MU_CRYPTO_IV_LEN + cbIn + MU_CRYPTO_TAG_LEN);

        // Write version byte (HIGH-2 fix: format versioning for cross-build compatibility)
        out[0] = MU_CRYPTO_VERSION_AES256GCM;

        // Write IV after version byte
        memcpy(out.data() + 1, iv, MU_CRYPTO_IV_LEN);

        int outLen = 0;
        if (EVP_EncryptUpdate(ctx, out.data() + 1 + MU_CRYPTO_IV_LEN, &outLen, static_cast<const uint8_t*>(pIn),
                              static_cast<int>(cbIn)) != 1)
            break;

        int finalLen = 0;
        if (EVP_EncryptFinal_ex(ctx, out.data() + 1 + MU_CRYPTO_IV_LEN + outLen, &finalLen) != 1)
            break;

        outLen += finalLen;

        // Get auth tag
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, MU_CRYPTO_TAG_LEN,
                                out.data() + 1 + MU_CRYPTO_IV_LEN + outLen) != 1)
            break;

        out.resize(1 + MU_CRYPTO_IV_LEN + outLen + MU_CRYPTO_TAG_LEN);
        ok = true;
    } while (false);

    EVP_CIPHER_CTX_free(ctx);

    if (!ok)
    {
        out.clear();
    }

    return ok;
}

bool mu_decrypt_blob(const void* pIn, size_t cbIn, std::vector<uint8_t>& out)
{
    out.clear();

    if (pIn == nullptr || cbIn == 0)
    {
        return false;
    }

    const auto* data = static_cast<const uint8_t*>(pIn);

    // Detect format via version byte (HIGH-2 fix)
    if (data[0] == MU_CRYPTO_VERSION_NOOP)
    {
        // No-op encrypted data: strip version byte, return raw bytes
        out.assign(data + 1, data + cbIn);
        return true;
    }

    // For non-versioned legacy blobs (first byte != 0x00 and != 0x01),
    // attempt AES decryption on the entire blob (pre-version-byte format)
    const uint8_t* payload = data;
    size_t payloadLen = cbIn;

    if (data[0] == MU_CRYPTO_VERSION_AES256GCM)
    {
        // New versioned format: strip version byte
        payload = data + 1;
        payloadLen = cbIn - 1;
    }

    // Minimum size: IV + tag
    if (payloadLen < static_cast<size_t>(MU_CRYPTO_IV_LEN + MU_CRYPTO_TAG_LEN))
    {
        return false;
    }

    // EVP_DecryptUpdate takes int for length; guard against overflow
    if (payloadLen > static_cast<size_t>(INT_MAX))
    {
        return false;
    }

    const uint8_t* key = mu_crypto_detail::GetCachedKey();
    if (key == nullptr)
    {
        return false;
    }

    const uint8_t* iv = payload;
    const uint8_t* ciphertext = payload + MU_CRYPTO_IV_LEN;
    size_t ciphertextLen = payloadLen - MU_CRYPTO_IV_LEN - MU_CRYPTO_TAG_LEN;
    const uint8_t* tag = payload + MU_CRYPTO_IV_LEN + ciphertextLen;

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    if (ctx == nullptr)
    {
        return false;
    }

    bool ok = false;
    do
    {
        if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1)
            break;
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, MU_CRYPTO_IV_LEN, nullptr) != 1)
            break;
        if (EVP_DecryptInit_ex(ctx, nullptr, nullptr, key, iv) != 1)
            break;

        out.resize(ciphertextLen);

        int outLen = 0;
        if (EVP_DecryptUpdate(ctx, out.data(), &outLen, ciphertext, static_cast<int>(ciphertextLen)) != 1)
            break;

        // Set expected auth tag before finalize
        // Note: const_cast is needed because OpenSSL API requires non-const void*, but does not modify the tag
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, MU_CRYPTO_TAG_LEN, const_cast<uint8_t*>(tag)) != 1)
            break;

        int finalLen = 0;
        // EVP_DecryptFinal_ex returns 0 if GCM auth tag verification fails
        if (EVP_DecryptFinal_ex(ctx, out.data() + outLen, &finalLen) != 1)
            break;

        out.resize(outLen + finalLen);
        ok = true;
    } while (false);

    EVP_CIPHER_CTX_free(ctx);

    if (!ok)
    {
        out.clear();
    }

    return ok;
}

#else // !MU_HAS_OPENSSL

// No-op fallback: passes bytes through unencrypted with version byte prefix.
// Game config encryption is a convenience feature, not a security boundary.
bool mu_encrypt_blob(const void* pIn, size_t cbIn, std::vector<uint8_t>& out)
{
    out.clear();
    if (pIn == nullptr || cbIn == 0)
    {
        return false;
    }

    // Warn once (not per-call) that encryption is unavailable (LOW-1 fix: atomic for thread safety)
    static std::atomic<bool> warned{false};
    if (!warned.exchange(true))
    {
        fprintf(stderr, "[GameConfig] WARN: OpenSSL unavailable, config not encrypted\n");
    }

    const auto* bytes = static_cast<const uint8_t*>(pIn);
    out.reserve(1 + cbIn);
    out.push_back(MU_CRYPTO_VERSION_NOOP); // Version byte prefix (HIGH-2 fix)
    out.insert(out.end(), bytes, bytes + cbIn);
    return true;
}

bool mu_decrypt_blob(const void* pIn, size_t cbIn, std::vector<uint8_t>& out)
{
    out.clear();
    if (pIn == nullptr || cbIn == 0)
    {
        return false;
    }
    const auto* bytes = static_cast<const uint8_t*>(pIn);

    // Detect format via version byte (HIGH-2 fix)
    if (bytes[0] == MU_CRYPTO_VERSION_NOOP)
    {
        // New no-op format: strip version byte
        out.assign(bytes + 1, bytes + cbIn);
        return true;
    }

    if (bytes[0] == MU_CRYPTO_VERSION_AES256GCM)
    {
        // OpenSSL-encrypted data but we don't have OpenSSL — cannot decrypt
        fprintf(stderr, "[GameConfig] WARN: Cannot decrypt AES-256-GCM data without OpenSSL\n");
        return false;
    }

    // Legacy format (no version byte): return as-is
    out.assign(bytes, bytes + cbIn);
    return true;
}

#endif // MU_HAS_OPENSSL
