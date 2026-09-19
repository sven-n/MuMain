#include "doctest.h"

#include "Core/Platform/PlatformCrypto.h"

#include <array>

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
