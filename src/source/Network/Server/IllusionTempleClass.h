#pragma once

#include <cstdint>

namespace Network::Packets
{
// BF04 uses evolution:line nibbles; ordinary server class numbers use line * 4 + step.
constexpr std::uint8_t DecodeIllusionTempleClass(std::uint8_t wireClass)
{
    constexpr std::uint8_t LineMask = 0x0F;
    constexpr int EvolutionShift = 4;
    constexpr int ServerClassStride = 4;
    constexpr int SecondEvolution = 2;
    constexpr int MasterEvolution = 3;
    constexpr int TwoTierMasterStep = 1;
    constexpr int GladiatorLine = 3;
    constexpr int LordLine = 4;
    constexpr int FighterLine = 6;
    const int line = wireClass & LineMask;
    const int evolution = wireClass >> EvolutionShift;
    const bool twoTierLine = line == GladiatorLine || line == LordLine || line == FighterLine;
    if (line > FighterLine || (evolution != 0 && evolution != SecondEvolution && evolution != MasterEvolution) ||
        (twoTierLine && evolution == SecondEvolution))
    {
        return 0;
    }
    const int step = twoTierLine && evolution == MasterEvolution ? TwoTierMasterStep : evolution;
    return static_cast<std::uint8_t>(line * ServerClassStride + step);
}
} // namespace Network::Packets
