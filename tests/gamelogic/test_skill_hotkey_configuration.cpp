// Tests of the skill hotkeys in the key configuration the server stores per character.

#include "doctest.h"

#include "GameLogic/Skills/SkillHotkeyConfiguration.h"

#include <cstdint>

namespace Hotkeys = GameLogic::Skills::HotkeyConfiguration;

TEST_CASE("skill hotkey numbers are stored big-endian")
{
    CHECK(Hotkeys::SkillNumber(0x00, 0x18) == 24);
    CHECK(Hotkeys::SkillNumber(0x01, 0x2C) == 300);
    CHECK(Hotkeys::SkillNumber(0xFF, 0xFF) == Hotkeys::FreeHotkey);
}

TEST_CASE("an encoded skill hotkey reads back as the same skill number")
{
    for (const std::uint16_t skillNumber :
         {std::uint16_t{0}, std::uint16_t{24}, std::uint16_t{300}, Hotkeys::FreeHotkey})
    {
        const auto bytes = Hotkeys::Encode(skillNumber);
        CHECK(Hotkeys::SkillNumber(bytes[0], bytes[1]) == skillNumber);
    }
    CHECK(Hotkeys::Encode(300) == std::array<std::uint8_t, Hotkeys::BytesPerHotkey>{0x01, 0x2C});
}

TEST_CASE("free and zeroed skill hotkeys are not assigned")
{
    CHECK_FALSE(Hotkeys::IsAssigned(Hotkeys::FreeHotkey));
    CHECK_FALSE(Hotkeys::IsAssigned(0));
    CHECK(Hotkeys::IsAssigned(1));
    CHECK(Hotkeys::IsAssigned(24));
    CHECK(Hotkeys::IsAssigned(300));
}
