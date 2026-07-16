#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

inline constexpr std::uint8_t MU_CRYPTO_VERSION_AES256GCM = 0x01;

bool mu_encrypt_blob(const void* input, std::size_t inputSize, std::vector<std::uint8_t>& output);
bool mu_decrypt_blob(const void* input, std::size_t inputSize, std::vector<std::uint8_t>& output);
