#include "stdafx.h"

#include "Core/Platform/PlatformCrypto.h"

#include <array>
#include <climits>
#include <cstring>
#include <string>

#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/rand.h>

#ifndef _WIN32
#include <unistd.h>
#endif

namespace
{
    constexpr int KeySize = 32;
    constexpr int IvSize = 12;
    constexpr int TagSize = 16;
    constexpr int Pbkdf2Iterations = 100000;
    constexpr std::array<std::uint8_t, 16> Salt = {
        0x4D, 0x75, 0x4D, 0x61, 0x69, 0x6E, 0x43, 0x72,
        0x65, 0x64, 0x65, 0x6E, 0x74, 0x69, 0x61, 0x6C
    };

    bool DeriveKey(std::array<std::uint8_t, KeySize>& key)
    {
        char hostname[256] = {};
#ifdef _WIN32
        DWORD hostnameSize = sizeof(hostname);
        if (!GetComputerNameA(hostname, &hostnameSize))
        {
            return false;
        }
        const std::string material(hostname, hostnameSize);
#else
        if (gethostname(hostname, sizeof(hostname)) != 0)
        {
            return false;
        }
        hostname[sizeof(hostname) - 1] = '\0';
        const std::string material = std::string(hostname) + ':' + std::to_string(getuid());
#endif
        const bool success = PKCS5_PBKDF2_HMAC(
            material.data(), static_cast<int>(material.size()), Salt.data(), static_cast<int>(Salt.size()),
            Pbkdf2Iterations, EVP_sha256(), static_cast<int>(key.size()), key.data()) == 1;
        OPENSSL_cleanse(hostname, sizeof(hostname));
        return success;
    }

    const std::array<std::uint8_t, KeySize>* GetKey()
    {
        static const std::array<std::uint8_t, KeySize> key = [] {
            std::array<std::uint8_t, KeySize> value = {};
            DeriveKey(value);
            return value;
        }();
        static const bool valid = std::any_of(key.begin(), key.end(), [](std::uint8_t value) { return value != 0; });
        return valid ? &key : nullptr;
    }
}

bool mu_encrypt_blob(const void* input, std::size_t inputSize, std::vector<std::uint8_t>& output)
{
    output.clear();
    if (!input || inputSize == 0 || inputSize > static_cast<std::size_t>(INT_MAX))
    {
        return false;
    }

    const auto* key = GetKey();
    std::array<std::uint8_t, IvSize> iv = {};
    if (!key || RAND_bytes(iv.data(), static_cast<int>(iv.size())) != 1)
    {
        return false;
    }

    EVP_CIPHER_CTX* context = EVP_CIPHER_CTX_new();
    if (!context)
    {
        return false;
    }

    output.resize(1 + IvSize + inputSize + TagSize);
    output[0] = MU_CRYPTO_VERSION_AES256GCM;
    std::memcpy(output.data() + 1, iv.data(), iv.size());

    int encryptedSize = 0;
    int finalSize = 0;
    const bool success =
        EVP_EncryptInit_ex(context, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) == 1
        && EVP_CIPHER_CTX_ctrl(context, EVP_CTRL_GCM_SET_IVLEN, IvSize, nullptr) == 1
        && EVP_EncryptInit_ex(context, nullptr, nullptr, key->data(), iv.data()) == 1
        && EVP_EncryptUpdate(context, output.data() + 1 + IvSize, &encryptedSize,
            static_cast<const std::uint8_t*>(input), static_cast<int>(inputSize)) == 1
        && EVP_EncryptFinal_ex(context, output.data() + 1 + IvSize + encryptedSize, &finalSize) == 1
        && EVP_CIPHER_CTX_ctrl(context, EVP_CTRL_GCM_GET_TAG, TagSize,
            output.data() + 1 + IvSize + encryptedSize + finalSize) == 1;
    EVP_CIPHER_CTX_free(context);

    if (!success)
    {
        output.clear();
        return false;
    }
    output.resize(1 + IvSize + encryptedSize + finalSize + TagSize);
    return true;
}

bool mu_decrypt_blob(const void* input, std::size_t inputSize, std::vector<std::uint8_t>& output)
{
    output.clear();
    constexpr std::size_t Overhead = 1 + IvSize + TagSize;
    if (!input || inputSize < Overhead || inputSize > static_cast<std::size_t>(INT_MAX))
    {
        return false;
    }

    const auto* data = static_cast<const std::uint8_t*>(input);
    const auto* key = GetKey();
    if (data[0] != MU_CRYPTO_VERSION_AES256GCM || !key)
    {
        return false;
    }

    const std::size_t ciphertextSize = inputSize - Overhead;
    const auto* iv = data + 1;
    const auto* ciphertext = iv + IvSize;
    const auto* tag = ciphertext + ciphertextSize;
    EVP_CIPHER_CTX* context = EVP_CIPHER_CTX_new();
    if (!context)
    {
        return false;
    }

    output.resize(ciphertextSize);
    int decryptedSize = 0;
    int finalSize = 0;
    const bool success =
        EVP_DecryptInit_ex(context, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) == 1
        && EVP_CIPHER_CTX_ctrl(context, EVP_CTRL_GCM_SET_IVLEN, IvSize, nullptr) == 1
        && EVP_DecryptInit_ex(context, nullptr, nullptr, key->data(), iv) == 1
        && EVP_DecryptUpdate(context, output.data(), &decryptedSize, ciphertext, static_cast<int>(ciphertextSize)) == 1
        && EVP_CIPHER_CTX_ctrl(context, EVP_CTRL_GCM_SET_TAG, TagSize, const_cast<std::uint8_t*>(tag)) == 1
        && EVP_DecryptFinal_ex(context, output.data() + decryptedSize, &finalSize) == 1;
    EVP_CIPHER_CTX_free(context);

    if (!success)
    {
        output.clear();
        return false;
    }
    output.resize(decryptedSize + finalSize);
    return true;
}
