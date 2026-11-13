#include "doctest.h"

#include "Core/Platform/PlatformCrypto.h"
#include "Core/Platform/Adapters/OpenSslCredentials.h"

#include <array>
#include <string>
#include <string_view>

TEST_CASE("Platform crypto authenticates encrypted blobs [core][crypto]")
{
    constexpr std::array<std::uint8_t, 8> plaintext = { 0x4D, 0x75, 0x4D, 0x61, 0x69, 0x6E, 0x00, 0x01 };
    std::vector<std::uint8_t> first;
    std::vector<std::uint8_t> second;

    REQUIRE(mu_encrypt_blob(plaintext.data(), plaintext.size(), first));
    REQUIRE(mu_encrypt_blob(plaintext.data(), plaintext.size(), second));
    CHECK(first.front() == MU_CRYPTO_VERSION_AES256GCM);
    CHECK(first != second);

    std::vector<std::uint8_t> decrypted;
    REQUIRE(mu_decrypt_blob(first.data(), first.size(), decrypted));
    CHECK(decrypted == std::vector<std::uint8_t>(plaintext.begin(), plaintext.end()));

    first.back() ^= 0x01;
    CHECK_FALSE(mu_decrypt_blob(first.data(), first.size(), decrypted));
    CHECK(decrypted.empty());
}

TEST_CASE("Saved credential blobs keep their format and key derivation [core][crypto]")
{
    // "MuMain saved login" encrypted under the key from the material "host:1000"; a saved login stops loading if
    // this stops decrypting.
    constexpr std::string_view goldenHex =
        "01cd42fc2f89ea78ceb8a46d6b970d5280bd12dec74d4e6c7bcb10aa3175d9064cdd4d5613d81719c4a68cd4140367";
    std::vector<std::uint8_t> blob;
    for (std::size_t i = 0; i < goldenHex.size(); i += 2)
    {
        blob.push_back(static_cast<std::uint8_t>(std::stoi(std::string(goldenHex.substr(i, 2)), nullptr, 16)));
    }

    std::vector<std::uint8_t> decrypted;
    REQUIRE(mu_decrypt_blob_with_key_material("host:1000", blob.data(), blob.size(), decrypted));
    CHECK(std::string(decrypted.begin(), decrypted.end()) == "MuMain saved login");
    CHECK_FALSE(mu_decrypt_blob_with_key_material("host:1001", blob.data(), blob.size(), decrypted));
}
