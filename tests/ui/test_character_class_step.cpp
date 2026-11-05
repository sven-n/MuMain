#include <doctest.h>

#include "Character/ClassStep.h"

#include <array>

using UI::CharacterCreation::StepToEnabledClass;

namespace
{
// DW, DK, Elf, MG, DL, Summoner, RF, with MG and Summoner locked.
constexpr std::array<bool, 7> Enabled{true, true, true, false, true, false, true};
} // namespace

TEST_CASE("class arrows step over locked classes [ui][char-make]")
{
    CHECK(StepToEnabledClass(Enabled, 2, +1) == 4);
    CHECK(StepToEnabledClass(Enabled, 4, -1) == 2);
    CHECK(StepToEnabledClass(Enabled, 4, +1) == 6);
    CHECK(StepToEnabledClass(Enabled, 0, -1) == -1);
    CHECK(StepToEnabledClass(Enabled, 6, +1) == -1);
}

TEST_CASE("class arrows reach every enabled class, whatever the theme shows [ui][char-make]")
{
    int reached = 0;
    for (int index = 0; index >= 0; index = StepToEnabledClass(Enabled, index, +1))
        ++reached;
    CHECK(reached == 5);

    reached = 0;
    for (int index = 6; index >= 0; index = StepToEnabledClass(Enabled, index, -1))
        ++reached;
    CHECK(reached == 5);
}
