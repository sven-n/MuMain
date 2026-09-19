#include "doctest.h"

#include "Scenes/SceneCommon.h"
#include "Scenes/SceneManager.h"

TEST_CASE("Scene state APIs preserve validated legacy behavior [core][scenes]")
{
    CharacterSelectionState character;
    CHECK_FALSE(character.HasSelection());
    character.SelectCharacter(MAX_CHARACTERS_PER_ACCOUNT - 1);
    CHECK(character.GetSelectedIndex() == MAX_CHARACTERS_PER_ACCOUNT - 1);
    character.SelectCharacter(MAX_CHARACTERS_PER_ACCOUNT);
    CHECK(character.GetSelectedIndex() == MAX_CHARACTERS_PER_ACCOUNT - 1);

    SceneInitializationState initialization;
    initialization.SetInitLogIn(true);
    initialization.SetInitLoading(true);
    initialization.ResetForDisconnect();
    CHECK_FALSE(initialization.GetInitLogIn());
    CHECK(initialization.GetInitLoading());

    FrameTimingState timing;
    timing.UpdateCurrentTime(25.0);
    timing.SetLastWaterChange(10.0);
    CHECK(timing.GetCurrentTickCount() == 25.0);
    CHECK(timing.GetLastWaterChange() == 10.0);
    timing.MarkFrameRendered();
    CHECK(timing.GetLastRenderTickCount() == 25.0);
}
