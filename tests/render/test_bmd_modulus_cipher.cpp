#include <doctest.h>

#include "BmdModulusTestEncryption.h"
#include "Render/Models/BmdModulusCipher.h"

#include <array>
#include <cstdint>
#include <memory>
#include <vector>

namespace
{
using namespace BmdModulusTest;
using Render::Models::BmdModulusCipher::DecryptInPlace;
using Render::Models::BmdModulusCipher::ModelHeaderSize;

std::vector<std::uint8_t> Decrypt(std::vector<std::uint8_t> payload)
{
    REQUIRE(DecryptInPlace(payload.data(), payload.size()));
    return {payload.begin() + BodyOffset, payload.end()};
}
} // namespace

TEST_CASE("version 14 models decrypt to the original data with every pair of ciphers [render][bmd]")
{
    // Sizes around the first stage's chunk rules: none, the ends only (overlapping, then not), the middle too,
    // each with a tail shorter than a cipher block.
    constexpr std::array<std::size_t, 5> BodySizes = {100, 1500, 3000, 4097, 9001};
    for (const std::size_t size : BodySizes)
    {
        const auto body = MakeBody(size);
        for (std::size_t second = 0; second < AlgorithmCount; ++second)
        {
            for (std::size_t first = 0; first < AlgorithmCount; ++first)
            {
                CAPTURE(size);
                CAPTURE(second);
                CAPTURE(first);
                CHECK(Decrypt(Encrypt(second, first, body)) == body);
            }
        }
    }
}

TEST_CASE("the first payload byte picks the second cipher, the second byte the first [render][bmd]")
{
    // Computed with independent TEA and RC5 implementations (checked against the ciphers' published test
    // vectors): payload[0] = 0 (TEA, second stage), payload[1] = 3 (RC5, first stage).
    constexpr std::array<std::uint8_t, 48> ExpectedHead = {
        0x00, 0x03, 0xC9, 0xEF, 0x3E, 0x1D, 0x1F, 0x33, 0x8B, 0x13, 0xC9, 0xB3, 0x0C, 0xB4, 0x78, 0x0A,
        0xB2, 0x6B, 0x69, 0xC8, 0xC7, 0x23, 0x2B, 0x46, 0xC9, 0x2F, 0x61, 0xD7, 0x47, 0xBA, 0x37, 0x60,
        0x9C, 0xFD, 0x1D, 0x06, 0x57, 0xA6, 0xBD, 0x9F, 0x01, 0x15, 0x6B, 0xE9, 0xB0, 0x42, 0xAE, 0xBD};
    constexpr std::array<std::uint8_t, 16> ExpectedTail = {0x98, 0x7B, 0xC9, 0xE6, 0xFB, 0x53, 0xB6, 0xD2,
                                                           0xF5, 0xAF, 0xD4, 0xE5, 0x31, 0xA1, 0x67, 0xA7};
    constexpr std::size_t BodySize = 1500;
    constexpr std::size_t Tea = 0;
    constexpr std::size_t Rc5 = 3;

    const auto payload = Encrypt(Tea, Rc5, MakeBody(BodySize));

    CHECK(std::vector<std::uint8_t>(payload.begin(), payload.begin() + ExpectedHead.size()) ==
          std::vector<std::uint8_t>(ExpectedHead.begin(), ExpectedHead.end()));
    CHECK(std::vector<std::uint8_t>(payload.end() - ExpectedTail.size(), payload.end()) ==
          std::vector<std::uint8_t>(ExpectedTail.begin(), ExpectedTail.end()));
    CHECK(Decrypt(payload) == MakeBody(BodySize));
}

TEST_CASE("a payload too short for a model is rejected [render][bmd]")
{
    // The keys and a model header (name and counts) at the least: one byte short is refused, exactly enough is not.
    std::vector<std::uint8_t> payload = MakeBody(BodyOffset + ModelHeaderSize - 1);
    CHECK_FALSE(DecryptInPlace(payload.data(), payload.size()));
    payload = MakeBody(BodyOffset + ModelHeaderSize);
    CHECK(DecryptInPlace(payload.data(), payload.size()));
    CHECK_FALSE(DecryptInPlace(nullptr, 0));
}

TEST_CASE("a corrupt version 14 payload decrypts without throwing, whichever ciphers it picks [render][bmd]")
{
    // The algorithm bytes are the file's, so values with the bits the cipher choice ignores set too. Both strides are
    // odd, so their low three bits (the cipher) run through all eight values within eight steps: 29 for the first
    // cipher, 13 for the second (37 left RC5 out of the second stage). The key and data bytes are whatever a damaged
    // file holds.
    constexpr int FirstStride = 29;
    constexpr int SecondStride = 13;
    // The smallest payload, then sizes just past the first stage's thresholds: more than 1024 data bytes (1020 for
    // 3-Way's 12-byte blocks) runs its end chunks, more than 4 chunks its middle one.
    constexpr std::array<std::size_t, 5> PayloadSizes = {BodyOffset + ModelHeaderSize, BodyOffset + 1025,
                                                         BodyOffset + 4097, 4200, 9001};
    for (const std::size_t size : PayloadSizes)
    {
        for (int second = 0; second < 256; second += SecondStride)
        {
            for (int first = 0; first < 256; first += FirstStride)
            {
                CAPTURE(size);
                CAPTURE(second);
                CAPTURE(first);
                std::vector<std::uint8_t> payload = MakeBody(size);
                payload[0] = static_cast<std::uint8_t>(second);
                payload[1] = static_cast<std::uint8_t>(first);
                bool decrypted = false;
                CHECK_NOTHROW(decrypted = DecryptInPlace(payload.data(), payload.size()));
                CHECK(decrypted);
            }
        }
    }
}
