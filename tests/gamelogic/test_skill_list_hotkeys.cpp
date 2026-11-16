// Tests of the skill list's hotkey bindings, which decide when the key configuration is saved.

#include "stdafx.h"
#include "doctest.h"

#include "UI/HUD/SkillList.h"

TEST_CASE("binding a skill to a hotkey reports whether the binding changed [hotkeys]")
{
    mu::ui::window::CSkillList skillList;

    CHECK(skillList.SetHotKey(3, 7));
    CHECK(skillList.GetHotKey(3) == 7);

    SUBCASE("the same skill on the same hotkey changes nothing")
    {
        CHECK_FALSE(skillList.SetHotKey(3, 7));
        CHECK(skillList.GetHotKey(3) == 7);
    }

    SUBCASE("moving the skill to another hotkey frees the old one")
    {
        CHECK(skillList.SetHotKey(5, 7));
        CHECK(skillList.GetHotKey(5) == 7);
        CHECK(skillList.GetHotKey(3) == -1);
    }

    SUBCASE("another skill on the hotkey replaces it")
    {
        CHECK(skillList.SetHotKey(3, 8));
        CHECK(skillList.GetHotKey(3) == 8);
    }
}
