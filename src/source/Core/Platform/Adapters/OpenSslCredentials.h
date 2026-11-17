#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

// What this host's key is derived from: the user and machine a saved login belongs to. The target compiles one of
// CredentialHostWin32.cpp or CredentialHostPosix.cpp beside OpenSslCredentials.cpp; it also defines
// mu_decrypt_legacy_blob (PlatformCrypto.h) for the logins the host saved before AES-GCM.
bool CredentialHostKeyMaterial(std::string& material);

// The blob functions with the key derived from the given material instead of this host's, so a test can pin the
// format and the key derivation that saved logins depend on.
bool mu_encrypt_blob_with_key_material(std::string_view material, const void* input, std::size_t inputSize,
                                       std::vector<std::uint8_t>& output);
bool mu_decrypt_blob_with_key_material(std::string_view material, const void* input, std::size_t inputSize,
                                       std::vector<std::uint8_t>& output);
