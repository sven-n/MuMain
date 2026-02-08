#pragma once


inline static const BYTE bBuxCode[3] = { 0xFC, 0xCF, 0xAB };

inline static void BuxConvert(BYTE* Buffer, int Size)
{
    for (int i = 0; i < Size; ++i)
        Buffer[i] ^= bBuxCode[i % 3];
}