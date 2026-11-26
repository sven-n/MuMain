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

namespace
{
constexpr int Free = Hotkeys::NoSkillIndex;
using SkillIndexOfHotkey = std::array<int, Hotkeys::HotkeyCount>;
} // namespace

TEST_CASE("the skill on key 1 is selected first, whatever the server's list order (#1166)")
{
    // Blade Master from #1166: Rageful Blow is at index 0 of the server's list and sits on key 6.
    const SkillIndexOfHotkey hotkeys{Free, 4, 2, 3, 5, 6, 0, Free, Free, Free};
    CHECK(Hotkeys::InitialSkillIndex(hotkeys) == 4);
}

TEST_CASE("the lowest bound key from 2 to 5 is selected when key 1 is free")
{
    const SkillIndexOfHotkey hotkeys{Free, Free, Free, 7, 2, Free, 0, Free, Free, Free};
    CHECK(Hotkeys::InitialSkillIndex(hotkeys) == 7);
}

TEST_CASE("nothing is selected when keys 1 to 5 are free")
{
    // Keys 0 and 6 to 9 are the other row: selecting them would flip the HUD again.
    const SkillIndexOfHotkey hotkeys{3, Free, Free, Free, Free, Free, 0, 1, 2, 4};
    CHECK(Hotkeys::InitialSkillIndex(hotkeys) == Hotkeys::NoSkillIndex);
}

TEST_CASE("skill-list index 0 on key 1 is selected")
{
    const SkillIndexOfHotkey hotkeys{Free, 0, Free, Free, Free, Free, Free, Free, Free, Free};
    CHECK(Hotkeys::InitialSkillIndex(hotkeys) == 0);
}
