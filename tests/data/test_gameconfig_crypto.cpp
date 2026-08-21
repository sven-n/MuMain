// Story 7.6.3: Data Layer Win32 Removal
// RED PHASE: Tests verify mu_encrypt_blob / mu_decrypt_blob round-trip contract.
//
// AC-3: CryptProtectData / CryptUnprotectData replaced with mu_encrypt_blob / mu_decrypt_blob
//       (AES-256-GCM via OpenSSL, or identity no-op if OpenSSL is unavailable)
// AC-STD-2: Unit test in tests/data/test_gameconfig_crypto.cpp
//
// Tests compile but FAIL until:
//   Task 2.1: PlatformCompat.h gains mu_encrypt_blob() and mu_decrypt_blob() implementations
//   Task 3.2: CMake links OpenSSL::Crypto to MUCore (or provides no-op fallback)
//
// No Win32 APIs, no DPAPI, no game loop — compiles and runs on macOS/Linux/Windows (MinGW CI).

#include "PlatformCompat.h"

#include <catch2/catch_test_macros.hpp>
#include <cstring>
#include <string>
#include <vector>

// ---------------------------------------------------------------------------
// AC-3 / AC-STD-2: mu_encrypt_blob / mu_decrypt_blob round-trip
// ---------------------------------------------------------------------------

TEST_CASE("AC-3: mu_encrypt_blob round-trips plaintext bytes", "[data][crypto]")
{
    SECTION("short ASCII payload round-trips correctly")
    {
        const char plaintext[] = "Hello, World!";
        const size_t plaintextLen = sizeof(plaintext) - 1; // exclude null terminator

        std::vector<uint8_t> encrypted;
        bool encryptOk = mu_encrypt_blob(plaintext, plaintextLen, encrypted);

        REQUIRE(encryptOk);
        REQUIRE_FALSE(encrypted.empty());

        std::vector<uint8_t> decrypted;
        bool decryptOk = mu_decrypt_blob(encrypted.data(), encrypted.size(), decrypted);

        REQUIRE(decryptOk);
        REQUIRE(decrypted.size() == plaintextLen);
        REQUIRE(std::memcmp(decrypted.data(), plaintext, plaintextLen) == 0);
    }

    SECTION("binary payload with embedded null bytes round-trips correctly")
    {
        // Credential data can contain null bytes (wchar_t UTF-16 fields)
        const uint8_t payload[] = {0x61, 0x00, 0x62, 0x00, 0x63, 0x00}; // L"abc" UTF-16LE
        const size_t payloadLen = sizeof(payload);

        std::vector<uint8_t> encrypted;
        bool encryptOk = mu_encrypt_blob(payload, payloadLen, encrypted);

        REQUIRE(encryptOk);
        REQUIRE_FALSE(encrypted.empty());

        std::vector<uint8_t> decrypted;
        bool decryptOk = mu_decrypt_blob(encrypted.data(), encrypted.size(), decrypted);

        REQUIRE(decryptOk);
        REQUIRE(decrypted.size() == payloadLen);
        REQUIRE(std::memcmp(decrypted.data(), payload, payloadLen) == 0);
    }

    SECTION("unicode credential string (wstring bytes) round-trips correctly")
    {
        // Simulate wchar_t credential storage as raw bytes
        const std::wstring cred = L"TestPass123!";
        const size_t byteLen = (cred.size() + 1) * sizeof(wchar_t); // include null terminator

        std::vector<uint8_t> encrypted;
        bool encryptOk = mu_encrypt_blob(cred.c_str(), byteLen, encrypted);

        REQUIRE(encryptOk);
        REQUIRE_FALSE(encrypted.empty());

        std::vector<uint8_t> decrypted;
        bool decryptOk = mu_decrypt_blob(encrypted.data(), encrypted.size(), decrypted);

        REQUIRE(decryptOk);
        REQUIRE(decrypted.size() == byteLen);
        REQUIRE(std::memcmp(decrypted.data(), cred.c_str(), byteLen) == 0);
    }

    SECTION("empty input is handled without crash")
    {
        // nullptr / 0-byte input: implementation may return false or true with empty output
        std::vector<uint8_t> encrypted;
        bool encryptOk = mu_encrypt_blob(nullptr, 0, encrypted);

        // If encrypt succeeds, decrypt must also succeed and return 0 bytes
        if (encryptOk && !encrypted.empty())
        {
            std::vector<uint8_t> decrypted;
            bool decryptOk = mu_decrypt_blob(encrypted.data(), encrypted.size(), decrypted);
            REQUIRE(decryptOk);
            REQUIRE(decrypted.empty());
        }
        // If encrypt returns false for empty input, that is also acceptable
    }
}

TEST_CASE("AC-3: mu_decrypt_blob rejects tampered ciphertext", "[data][crypto]")
{
    SECTION("flipping a ciphertext byte causes authentication tag verification failure")
    {
        const char plaintext[] = "SecretPassword";
        const size_t plaintextLen = sizeof(plaintext) - 1;

        std::vector<uint8_t> encrypted;
        bool encryptOk = mu_encrypt_blob(plaintext, plaintextLen, encrypted);

        REQUIRE(encryptOk);
        REQUIRE(encrypted.size() >= 2);

        // Tamper with the last byte (typically within the GCM authentication tag)
        encrypted.back() ^= 0xFF;

        std::vector<uint8_t> decrypted;
        bool decryptOk = mu_decrypt_blob(encrypted.data(), encrypted.size(), decrypted);

        // AES-256-GCM MUST detect tampering — decryption must fail
        REQUIRE_FALSE(decryptOk);
    }
}

TEST_CASE("AC-3: mu_encrypt_blob uses a random IV/nonce per call", "[data][crypto]")
{
    SECTION("same plaintext encrypted twice produces different ciphertext blobs")
    {
        const char plaintext[] = "SameInput";
        const size_t plaintextLen = sizeof(plaintext) - 1;

        std::vector<uint8_t> enc1, enc2;
        bool ok1 = mu_encrypt_blob(plaintext, plaintextLen, enc1);
        bool ok2 = mu_encrypt_blob(plaintext, plaintextLen, enc2);

        REQUIRE(ok1);
        REQUIRE(ok2);

        // A random IV/nonce per encryption ensures ciphertext blobs always differ
        REQUIRE(enc1 != enc2);
    }
}
