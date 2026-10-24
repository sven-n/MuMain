#include <doctest.h>

#include "Render/Models/BmdModulusCipher.h"

#include <cryptopp/3way.h>
#include <cryptopp/cast.h>
#include <cryptopp/gost.h>
#include <cryptopp/idea.h>
#include <cryptopp/mars.h>
#include <cryptopp/rc5.h>
#include <cryptopp/rc6.h>
#include <cryptopp/tea.h>

#include <array>
#include <cstdint>
#include <memory>
#include <vector>

namespace
{
using Render::Models::BmdModulusCipher::BodyOffset;
using Render::Models::BmdModulusCipher::DecryptInPlace;

constexpr std::size_t KeySize = 32;
constexpr std::size_t SecondKeyOffset = 2;
constexpr std::size_t MiddleChunkThreshold = 4;
constexpr std::size_t ChunkSize = 1024;
constexpr std::size_t AlgorithmCount = 8;
constexpr std::uint8_t FixedKey[] = "webzen#@!01webzen#@!01webzen#@!0";

template <class TCipher> std::unique_ptr<CryptoPP::BlockTransformation> CreateEncryption(const std::uint8_t* key)
{
    return std::make_unique<typename TCipher::Encryption>(key, TCipher::DEFAULT_KEYLENGTH);
}

// The same numbering as the files use.
std::unique_ptr<CryptoPP::BlockTransformation> CreateEncryption(std::size_t algorithm, const std::uint8_t* key)
{
    switch (algorithm)
    {
    case 0:
        return CreateEncryption<CryptoPP::TEA>(key);
    case 1:
        return CreateEncryption<CryptoPP::ThreeWay>(key);
    case 2:
        return CreateEncryption<CryptoPP::CAST128>(key);
    case 3:
        return CreateEncryption<CryptoPP::RC5>(key);
    case 4:
        return CreateEncryption<CryptoPP::RC6>(key);
    case 5:
        return CreateEncryption<CryptoPP::MARS>(key);
    case 6:
        return CreateEncryption<CryptoPP::IDEA>(key);
    default:
        return CreateEncryption<CryptoPP::GOST>(key);
    }
}

void EncryptBlocks(const CryptoPP::BlockTransformation& cipher, std::uint8_t* data, std::size_t length)
{
    const std::size_t blockSize = cipher.BlockSize();
    for (std::size_t offset = 0; offset + blockSize <= length; offset += blockSize)
    {
        cipher.ProcessBlock(data + offset);
    }
}

std::vector<std::uint8_t> MakeBody(std::size_t size)
{
    constexpr std::size_t PatternStep = 7;
    constexpr std::size_t PatternModulus = 251;
    std::vector<std::uint8_t> body(size);
    for (std::size_t i = 0; i < size; ++i)
    {
        body[i] = static_cast<std::uint8_t>(i * PatternStep % PatternModulus);
    }
    return body;
}

std::array<std::uint8_t, KeySize> MakeSecondKey()
{
    std::array<std::uint8_t, KeySize> key{};
    for (std::size_t i = 0; i < key.size(); ++i)
    {
        key[i] = static_cast<std::uint8_t>(i);
    }
    return key;
}

// Builds a payload the way the files are made: the reverse of the decryption, in the reverse order.
std::vector<std::uint8_t> Encrypt(std::size_t secondAlgorithm, std::size_t firstAlgorithm,
                                  const std::vector<std::uint8_t>& body)
{
    const auto secondKey = MakeSecondKey();
    std::vector<std::uint8_t> payload{static_cast<std::uint8_t>(secondAlgorithm),
                                      static_cast<std::uint8_t>(firstAlgorithm)};
    payload.insert(payload.end(), secondKey.begin(), secondKey.end());
    payload.insert(payload.end(), body.begin(), body.end());

    EncryptBlocks(*CreateEncryption(secondAlgorithm, secondKey.data()), payload.data() + BodyOffset, body.size());

    const auto first = CreateEncryption(firstAlgorithm, FixedKey);
    const std::size_t chunk = ChunkSize - ChunkSize % first->BlockSize();
    const std::size_t dataSize = body.size();
    if (dataSize > chunk)
    {
        EncryptBlocks(*first, payload.data() + SecondKeyOffset, chunk);
        EncryptBlocks(*first, payload.data() + payload.size() - chunk, chunk);
    }

    if (dataSize > MiddleChunkThreshold * chunk)
    {
        EncryptBlocks(*first, payload.data() + SecondKeyOffset + dataSize / 2, chunk);
    }

    return payload;
}

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
    std::array<std::uint8_t, BodyOffset> payload{};
    CHECK_FALSE(DecryptInPlace(payload.data(), payload.size()));
    CHECK_FALSE(DecryptInPlace(nullptr, 0));
}

TEST_CASE("a corrupt version 14 payload decrypts without throwing, whichever ciphers it picks [render][bmd]")
{
    // The algorithm bytes are the file's, so every value (also with the bits the cipher choice ignores) and payload
    // sizes around the chunk rules; the key and data bytes are whatever a damaged file holds.
    constexpr std::array<std::size_t, 6> PayloadSizes = {BodyOffset + 1, BodyOffset + 7, 1024, 1025, 4200, 9001};
    for (const std::size_t size : PayloadSizes)
    {
        for (int second = 0; second < 256; second += 37)
        {
            for (int first = 0; first < 256; first += 29)
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
