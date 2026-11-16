// For a target without a credential store: logins are never saved.
#include "Core/Platform/PlatformCrypto.h"

bool mu_credentials_available()
{
    return false;
}

bool mu_encrypt_blob(const void*, std::size_t, std::vector<std::uint8_t>& output)
{
    output.clear();
    return false;
}

bool mu_decrypt_blob(const void*, std::size_t, std::vector<std::uint8_t>& output)
{
    output.clear();
    return false;
}

bool mu_decrypt_legacy_blob(const void*, std::size_t, std::vector<std::uint8_t>& output)
{
    output.clear();
    return false;
}
