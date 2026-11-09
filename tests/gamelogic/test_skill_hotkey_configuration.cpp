// Tests of reading the skill hotkeys from the key configuration the server stores per character.

#include "stdafx.h"

#include "doctest.h"

#include "GameLogic/Skills/SkillHotkeyConfiguration.h"

namespace Hotkeys = GameLogic::SkillHotkeyConfiguration;

TEST_CASE("skill hotkey numbers are stored big-endian")
{
    CHECK(Hotkeys::SkillNumber(0x00, 0x18) == 24);
    CHECK(Hotkeys::SkillNumber(0x01, 0x2C) == 300);
    CHECK(Hotkeys::SkillNumber(0xFF, 0xFF) == Hotkeys::FreeHotkey);
}

TEST_CASE("free and zeroed skill hotkeys are not assigned")
{
    CHECK_FALSE(Hotkeys::IsAssigned(Hotkeys::FreeHotkey));
    CHECK_FALSE(Hotkeys::IsAssigned(0));
    CHECK(Hotkeys::IsAssigned(1));
    CHECK(Hotkeys::IsAssigned(24));
    CHECK(Hotkeys::IsAssigned(300));
}
