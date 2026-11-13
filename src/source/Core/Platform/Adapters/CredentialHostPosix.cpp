// Saved logins on POSIX systems: the key comes from the host name and the user id, and older logins were protected
// with the DPAPI stand-in (Dpapi.h).
#include "Core/Platform/Adapters/OpenSslCredentials.h"
#include "Core/Platform/Dpapi.h"
#include "Core/Platform/PlatformCrypto.h"

#include <climits>

#include <openssl/crypto.h>
#include <unistd.h>

bool CredentialHostKeyMaterial(std::string& material)
{
    char hostname[256] = {};
    if (gethostname(hostname, sizeof(hostname)) != 0)
    {
        return false;
    }
    hostname[sizeof(hostname) - 1] = '\0';
    material = std::string(hostname) + ':' + std::to_string(getuid());
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
