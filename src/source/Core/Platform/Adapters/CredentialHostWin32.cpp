// Saved logins on Windows: the key comes from the computer name, and older logins were protected with DPAPI.
#include "stdafx.h"

#include "Core/Platform/Adapters/OpenSslCredentials.h"
#include "Core/Platform/PlatformCrypto.h"

#include <climits>

#include <openssl/crypto.h>
#include <wincrypt.h>

bool CredentialHostKeyMaterial(std::string& material)
{
    char hostname[256] = {};
    DWORD hostnameSize = sizeof(hostname);
    if (!GetComputerNameA(hostname, &hostnameSize))
    {
        return false;
    }
    material.assign(hostname, hostnameSize);
    OPENSSL_cleanse(hostname, sizeof(hostname));
    return true;
}

bool mu_decrypt_legacy_blob(const void* input, std::size_t inputSize, std::vector<std::uint8_t>& output)
{
    output.clear();
    if (!input || inputSize == 0 || inputSize > static_cast<std::size_t>(UINT_MAX))
    {
        return false;
    }
    DATA_BLOB dataIn = {static_cast<DWORD>(inputSize), static_cast<BYTE*>(const_cast<void*>(input))};
    DATA_BLOB dataOut = {};
    if (!CryptUnprotectData(&dataIn, nullptr, nullptr, nullptr, nullptr, 0, &dataOut))
    {
        return false;
    }
    output.assign(dataOut.pbData, dataOut.pbData + dataOut.cbData);
    LocalFree(dataOut.pbData);
    return true;
}
