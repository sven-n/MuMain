#pragma once

#include "Core/Platform/WinCompat.h"
#include "Core/Globals/_enum.h"

#include <array>
#include <cstdint>
#include <optional>

namespace Network::Packets
{
// OpenMU's GameServer/RemoteView/MiniGames/Extensions.cs::ToIllusionTempleCharacterClass
// encodes BF04 classes as evolution:line nibbles. Plain server class numbers are not this format
// and cannot be auto-detected: their values overlap valid packed base classes.
constexpr std::optional<SERVER_CLASS_TYPE> DecodeIllusionTempleClass(std::uint8_t wireClass)
{
    struct WireClass
    {
        std::uint8_t wire;
        SERVER_CLASS_TYPE serverClass;
    };
    constexpr std::array<WireClass, 18> Classes{{
        {0x00, DarkWizard},
        {0x20, SoulMaster},
        {0x30, GrandMaster},
        {0x01, DarkKnight},
        {0x21, BladeKnight},
        {0x31, BladeMaster},
        {0x02, FairyElf},
        {0x22, MuseElf},
        {0x32, HighElf},
        {0x03, MagicGladiator},
        {0x33, DuelMaster},
        {0x04, DarkLord},
        {0x34, LordEmperor},
        {0x05, Summoner},
        {0x25, BloodySummoner},
        {0x35, DimensionMaster},
        {0x06, RageFighter},
        {0x36, FistMaster},
    }};
    for (const auto& entry : Classes)
    {
        if (entry.wire == wireClass)
            return entry.serverClass;
    }
    return std::nullopt;
}
} // namespace Network::Packets
