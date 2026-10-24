#pragma once

// Builds version 14 BMD payloads the way the files are made, for the tests of the decryption
// (Render/Models/BmdModulusCipher): the reverse of the decryption, in the reverse order.

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

namespace BmdModulusTest
{
using Render::Models::BmdModulusCipher::BodyOffset;

constexpr std::size_t KeySize = 32;
constexpr std::size_t SecondKeyOffset = 2;
constexpr std::size_t MiddleChunkThreshold = 4;
constexpr std::size_t ChunkSize = 1024;
constexpr std::size_t AlgorithmCount = 8;
constexpr std::uint8_t FixedKey[] = "webzen#@!01webzen#@!01webzen#@!0";

template <class TCipher> inline std::unique_ptr<CryptoPP::BlockTransformation> CreateEncryption(const std::uint8_t* key)
{
    return std::make_unique<typename TCipher::Encryption>(key, TCipher::DEFAULT_KEYLENGTH);
}

// The same numbering as the files use.
inline std::unique_ptr<CryptoPP::BlockTransformation> CreateEncryption(std::size_t algorithm, const std::uint8_t* key)
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

inline void EncryptBlocks(const CryptoPP::BlockTransformation& cipher, std::uint8_t* data, std::size_t length)
{
    const std::size_t blockSize = cipher.BlockSize();
    for (std::size_t offset = 0; offset + blockSize <= length; offset += blockSize)
    {
        cipher.ProcessBlock(data + offset);
    }
}

inline std::vector<std::uint8_t> MakeBody(std::size_t size)
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

inline std::array<std::uint8_t, KeySize> MakeSecondKey()
{
    std::array<std::uint8_t, KeySize> key{};
    for (std::size_t i = 0; i < key.size(); ++i)
    {
        key[i] = static_cast<std::uint8_t>(i);
    }
    return key;
}

// Builds a payload the way the files are made: the reverse of the decryption, in the reverse order.
inline std::vector<std::uint8_t> Encrypt(std::size_t secondAlgorithm, std::size_t firstAlgorithm,
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

} // namespace BmdModulusTest
