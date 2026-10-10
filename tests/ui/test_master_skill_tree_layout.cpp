#include <doctest.h>

#include "UI/HUD/Skills/MasterSkillTreeLayout.h"

using namespace UI::Skills::MasterTree;

TEST_CASE("master tree nodes sit on the original's column and rank grid [ui][skills]")
{
    const NodePosition first = NodeBoxPosition(0, 0, 1);
    CHECK(first.left == 11);
    CHECK(first.top == 55);

    const NodePosition middle = NodeBoxPosition(1, 2, 3);
    CHECK(middle.left == 221 + 2 * 49);
    CHECK(middle.top == 55 + 2 * 41);

    const NodePosition last = NodeBoxPosition(2, 3, 5);
    CHECK(last.left == 431 + 3 * 49);
    CHECK(last.top == 55 + 4 * 41);
}

TEST_CASE("a node's slot repeats every four tree indices [ui][skills]")
{
    CHECK(SlotInRank(1) == 0);
    CHECK(SlotInRank(4) == 3);
    CHECK(SlotInRank(5) == 0);
    CHECK(SlotInRank(10) == 1);
}

TEST_CASE("usable nodes draw from the lit atlas, the others from the grey one [ui][skills]")
{
    CHECK(IconSpriteName(0, true) == "master-icon-lit-0");
    CHECK(IconSpriteName(137, true) == "master-icon-lit-137");
    CHECK(IconSpriteName(137, false) == "master-icon-grey-137");
}

TEST_CASE("master experience percent counts from the start of the current master level [ui][skills]")
{
    // Master level 100 is total level 500; its range starts at 6,313,300,000 experience.
    constexpr std::int64_t levelStart = 6313300000;
    constexpr std::int64_t nextLevel = levelStart + 200000000;

    CHECK(ExperiencePercent({100, levelStart, nextLevel}) == doctest::Approx(0.0));
    CHECK(ExperiencePercent({100, levelStart + 100000000, nextLevel}) == doctest::Approx(50.0));
    CHECK(ExperiencePercent({100, nextLevel, nextLevel}) == doctest::Approx(100.0));
}
