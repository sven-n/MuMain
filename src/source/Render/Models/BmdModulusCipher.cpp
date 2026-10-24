#include "BmdModulusCipher.h"

#include <cryptopp/3way.h>
#include <cryptopp/cryptlib.h>
#include <cryptopp/cast.h>
#include <cryptopp/gost.h>
#include <cryptopp/idea.h>
#include <cryptopp/mars.h>
#include <cryptopp/rc5.h>
#include <cryptopp/rc6.h>
#include <cryptopp/tea.h>

#include <memory>

namespace
{
constexpr std::size_t SecondKeyOffset = 2;
constexpr std::size_t FirstStageChunkSize = 1024;
// The middle chunk is only encrypted when the model data is larger than this many chunks.
constexpr std::size_t MiddleChunkThreshold = 4;
// The low three bits of the algorithm byte pick the cipher.
constexpr std::uint8_t AlgorithmMask = 0x07;
constexpr std::uint8_t FirstStageKey[] = "webzen#@!01webzen#@!01webzen#@!0";

enum class Algorithm : std::uint8_t
{
    Tea = 0,
    ThreeWay = 1,
    Cast128 = 2,
    Rc5 = 3,
    Rc6 = 4,
    Mars = 5,
    Idea = 6,
    Gost = 7,
};

// Every cipher is keyed with its default key length: the key material is longer than any of them needs.
template <class TCipher> std::unique_ptr<CryptoPP::BlockTransformation> CreateDecryption(const std::uint8_t* key)
{
    return std::make_unique<typename TCipher::Decryption>(key, TCipher::DEFAULT_KEYLENGTH);
}

std::unique_ptr<CryptoPP::BlockTransformation> CreateDecryption(std::uint8_t algorithmByte, const std::uint8_t* key)
{
    switch (static_cast<Algorithm>(algorithmByte & AlgorithmMask))
    {
    case Algorithm::Tea:
        return CreateDecryption<CryptoPP::TEA>(key);
    case Algorithm::ThreeWay:
        return CreateDecryption<CryptoPP::ThreeWay>(key);
    case Algorithm::Cast128:
        return CreateDecryption<CryptoPP::CAST128>(key);
    case Algorithm::Rc5:
        return CreateDecryption<CryptoPP::RC5>(key);
    case Algorithm::Rc6:
        return CreateDecryption<CryptoPP::RC6>(key);
    case Algorithm::Mars:
        return CreateDecryption<CryptoPP::MARS>(key);
    case Algorithm::Idea:
        return CreateDecryption<CryptoPP::IDEA>(key);
    case Algorithm::Gost:
        return CreateDecryption<CryptoPP::GOST>(key);
    }
    return nullptr;
}

// Decrypts the whole cipher blocks of the range; a shorter tail stays as is.
void DecryptBlocks(const CryptoPP::BlockTransformation& cipher, std::uint8_t* data, std::size_t length)
{
    const std::size_t blockSize = cipher.BlockSize();
    const std::size_t wholeLength = length - length % blockSize;
    for (std::size_t offset = 0; offset < wholeLength; offset += blockSize)
    {
        cipher.ProcessBlock(data + offset);
    }
}

// The first stage restores the second stage's key, among a few chunks of the model data; false when its cipher
// can't be made, as the second stage fails then.
bool DecryptFirstStage(std::uint8_t* payload, std::size_t size)
{
    const auto cipher = CreateDecryption(payload[1], FirstStageKey);
    if (cipher == nullptr)
    {
        return false;
    }
    const std::size_t dataSize = size - Render::Models::BmdModulusCipher::BodyOffset;
    const std::size_t chunkSize = FirstStageChunkSize - FirstStageChunkSize % cipher->BlockSize();
    if (dataSize > MiddleChunkThreshold * chunkSize)
    {
        DecryptBlocks(*cipher, payload + SecondKeyOffset + dataSize / 2, chunkSize);
    }

    if (dataSize > chunkSize)
    {
        DecryptBlocks(*cipher, payload + size - chunkSize, chunkSize);
        DecryptBlocks(*cipher, payload + SecondKeyOffset, chunkSize);
    }
    return true;
}
} // namespace

bool Render::Models::BmdModulusCipher::DecryptInPlace(std::uint8_t* payload, std::size_t size)
{
    if (payload == nullptr || size < BodyOffset + ModelHeaderSize)
    {
        return false;
    }

    // The file picks the ciphers and gives the second key, so a corrupt file must not throw out of model loading:
    // Crypto++ reports a bad key or state with an exception.
    try
    {
        if (!DecryptFirstStage(payload, size))
        {
            return false;
        }
        const auto cipher = CreateDecryption(payload[0], payload + SecondKeyOffset);
        if (cipher == nullptr)
        {
            return false;
        }
        DecryptBlocks(*cipher, payload + BodyOffset, size - BodyOffset);
    }
    catch (const CryptoPP::Exception&)
    {
        return false;
    }
    return true;
}
