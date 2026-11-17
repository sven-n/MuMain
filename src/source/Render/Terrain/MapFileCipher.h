#pragma once

#include <cstdint>

// The XOR cipher of classic .map, .att and .obj files and of version 12 BMD models. Called with a null destination,
// each returns the size of its output.
inline int MapFileEncrypt(unsigned char* pbyDst, const unsigned char* pbySrc, int iSize)
{
    if (!pbyDst)
    {
        return (iSize);
    }
    constexpr unsigned char byMapXorKey[16] = {0xD1, 0x73, 0x52, 0xF6, 0xD2, 0x9A, 0xCB, 0x27,
                                               0x3E, 0xAF, 0x59, 0x31, 0x37, 0xB3, 0xE7, 0xA2};

    std::uint16_t wMapKey = 0x5E;
    for (int i = 0; i < iSize; ++i)
    {
        pbyDst[i] = (pbySrc[i] + static_cast<unsigned char>(wMapKey)) ^ byMapXorKey[i % 16];

        wMapKey = pbyDst[i] + 0x3D;
        wMapKey = wMapKey & 0xFF;
    }

    return (iSize);
}

inline int MapFileDecrypt(unsigned char* pbyDst, const unsigned char* pbySrc, int iSize)
{
    if (!pbyDst)
    {
        return (iSize);
    }
    constexpr unsigned char byMapXorKey[16] = {0xD1, 0x73, 0x52, 0xF6, 0xD2, 0x9A, 0xCB, 0x27,
                                               0x3E, 0xAF, 0x59, 0x31, 0x37, 0xB3, 0xE7, 0xA2};

    std::uint16_t wMapKey = 0x5E;
    for (int i = 0; i < iSize; ++i)
    {
        pbyDst[i] = (pbySrc[i] ^ byMapXorKey[i % 16]) - static_cast<unsigned char>(wMapKey);
        wMapKey = pbySrc[i] + 0x3D;
        wMapKey = wMapKey & 0xFF;
    }
    return (iSize);
}
