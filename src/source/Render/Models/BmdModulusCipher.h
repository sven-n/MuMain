#pragma once

#include <cstddef>
#include <cstdint>

// The encryption of version 14 (0x0E) BMD model files, used by the models of later seasons.
//
// The payload (what follows "BMD", the version byte and the int32 payload size) is encrypted in two
// stages, each with one of eight block ciphers in ECB mode:
//   1. payload[1] picks the first cipher, keyed with a fixed key. It covers a few 1 KB chunks only:
//      at offset 2 and at the end, and in the middle of large payloads.
//   2. payload[2..34) is then the key of the second cipher, picked by payload[0], which covers the
//      model data from offset 34 on (whole cipher blocks; a shorter tail is stored as is).
// After decryption, payload[34..) is a regular BMD body, read like the one of a version 10 file.
namespace Render::Models::BmdModulusCipher
{
// Where the model data starts in a decrypted payload.
constexpr std::size_t BodyOffset = 34;
// The smallest model body: its 32-byte name and its mesh, bone and action counts (three int16).
constexpr std::size_t ModelHeaderSize = 32 + 3 * 2;

// Decrypts a version 14 payload in place. False when it is too short to hold a model (the keys and a model header).
bool DecryptInPlace(std::uint8_t* payload, std::size_t size);
} // namespace Render::Models::BmdModulusCipher
