#include "stdafx.h"

#include "doctest.h"

// The animation values of the Monster Model tool and what the game's code
// adds on top (monsters design, section 9), without ImGui.
#ifdef _EDITOR

#include "Core/Globals/_define.h"
#include "Core/Globals/_enum.h"
#include "UI/MonsterTools/MonsterAnimationFacts.h"
#include "UI/MonsterTools/MonsterModelValues.h"

#include <vector>

using Data::Monsters::MonsterModelDefinition;
namespace ModelValues = MuEditor::Monsters::ModelValues;
namespace AnimationFacts = MuEditor::Monsters::AnimationFacts;

TEST_CASE("Monster model values: a speed is the entry's, else the range's, and the entry keeps only other speeds "
          "[editor][monsters]")
{
    MonsterModelDefinition model;
    // The monster range walks at 0.34, an NPC model plays all at 0.25.
    CHECK(ModelValues::GetSpeed(model, MODEL_HOUND, MONSTER01_WALK) == 0.34f);
    CHECK(ModelValues::GetSpeed(model, MODEL_SMITH, MONSTER01_WALK) == 0.25f);

    ModelValues::SetSpeed(model, MODEL_HOUND, MONSTER01_DIE, 0.4f);
    ModelValues::SetSpeed(model, MODEL_HOUND, MONSTER01_WALK, 0.5f);
    CHECK(model.load.speeds ==
          std::vector<Data::Monsters::MonsterAnimationSpeed>{{MONSTER01_WALK, 0.5f}, {MONSTER01_DIE, 0.4f}});
    CHECK(ModelValues::HasOwnSpeed(model, MONSTER01_WALK));
    CHECK(ModelValues::GetSpeed(model, MODEL_HOUND, MONSTER01_WALK) == 0.5f);

    // The range's speed again: the entry lets it go.
    ModelValues::SetSpeed(model, MODEL_HOUND, MONSTER01_WALK, 0.34f);
    CHECK_FALSE(ModelValues::HasOwnSpeed(model, MONSTER01_WALK));
    CHECK(model.load.speeds.size() == 1);
}

TEST_CASE("Monster model values: the held last frames list none while they are the range's [editor][monsters]")
{
    MonsterModelDefinition model;
    // The monster range holds die, the NPC range nothing.
    CHECK(ModelValues::HoldsLastFrame(model, MODEL_HOUND, MONSTER01_DIE));
    CHECK_FALSE(ModelValues::HoldsLastFrame(model, MODEL_HOUND, MONSTER01_STOP2));
    CHECK_FALSE(ModelValues::HoldsLastFrame(model, MODEL_SMITH, MONSTER01_DIE));

    constexpr int Animations = 8;
    ModelValues::SetHold(model, MODEL_HOUND, Animations, MONSTER01_STOP2, true);
    REQUIRE(model.load.holdLastFrame);
    CHECK(*model.load.holdLastFrame == std::vector<int>{MONSTER01_STOP2, MONSTER01_DIE});

    ModelValues::SetHold(model, MODEL_HOUND, Animations, MONSTER01_STOP2, false);
    CHECK_FALSE(model.load.holdLastFrame);

    // Die let go: an empty list, which differs from the range's.
    ModelValues::SetHold(model, MODEL_HOUND, Animations, MONSTER01_DIE, false);
    REQUIRE(model.load.holdLastFrame);
    CHECK(model.load.holdLastFrame->empty());
    CHECK_FALSE(ModelValues::HoldsLastFrame(model, MODEL_HOUND, MONSTER01_DIE));
}

TEST_CASE("Monster animation facts: the impact frame and the next animation are the monster range's [editor][monsters]")
{
    CHECK(AnimationFacts::GetImpactFrame(MODEL_HOUND, MONSTER01_ATTACK1) == 5.0f);
    // The pose blends from the key before (BMD::PlayAnimation keeps it), in
    // the first key from the last one of a looping animation.
    CHECK(AnimationFacts::GetPriorFrame(3.5f, 10, false) == 2.0f);
    CHECK(AnimationFacts::GetPriorFrame(1.0f, 10, false) == 0.0f);
    CHECK(AnimationFacts::GetPriorFrame(0.5f, 10, false) == 9.0f);
    CHECK(AnimationFacts::GetPriorFrame(0.5f, 10, true) == 0.0f);
    CHECK(AnimationFacts::GetPriorFrame(9.99f, 10, true) == 8.0f);
    CHECK(AnimationFacts::GetImpactFrame(MODEL_HOUND, MONSTER01_ATTACK2) == 5.0f);
    CHECK_FALSE(AnimationFacts::GetImpactFrame(MODEL_HOUND, MONSTER01_ATTACK3));
    CHECK_FALSE(AnimationFacts::GetImpactFrame(MODEL_SMITH, MONSTER01_ATTACK1));

    CHECK(AnimationFacts::GetNextAnimation(MODEL_HOUND, MONSTER01_ATTACK1) == MONSTER01_STOP1);
    CHECK(AnimationFacts::GetNextAnimation(MODEL_HOUND, MONSTER01_SHOCK) == MONSTER01_STOP1);
    CHECK_FALSE(AnimationFacts::GetNextAnimation(MODEL_HOUND, MONSTER01_STOP1));
    CHECK_FALSE(AnimationFacts::GetNextAnimation(MODEL_HOUND, MONSTER01_DIE));
    // Appear goes on to stop 1 only for Maya's hands and Selupan.
    CHECK_FALSE(AnimationFacts::GetNextAnimation(MODEL_HOUND, MONSTER01_APEAR));
    CHECK(AnimationFacts::GetNextAnimation(MODEL_SELUPAN, MONSTER01_APEAR) == MONSTER01_STOP1);
    CHECK_FALSE(AnimationFacts::GetNextAnimation(MODEL_SMITH, MONSTER01_ATTACK1));
}

TEST_CASE("Monster animation facts: the game's speed factors [editor][monsters]")
{
    using When = AnimationFacts::SpeedFactor::When;
    const std::vector<AnimationFacts::SpeedFactor> attack =
        AnimationFacts::GetSpeedFactors(MODEL_HOUND, MONSTER01_ATTACK2);
    REQUIRE(attack.size() == 1);
    CHECK(attack[0].factor == 1.5f);
    CHECK(attack[0].when == When::Transformed);
    CHECK(AnimationFacts::GetSpeedFactors(MODEL_HOUND, MONSTER01_DIE).empty());

    const std::vector<AnimationFacts::SpeedFactor> kundun =
        AnimationFacts::GetSpeedFactors(MODEL_ILLUSION_OF_KUNDUN, MONSTER01_DIE);
    REQUIRE(kundun.size() == 1);
    CHECK(kundun[0].factor == 4.0f);
    CHECK(kundun[0].when == When::AfterFrame6);

    const std::vector<AnimationFacts::SpeedFactor> erohim =
        AnimationFacts::GetSpeedFactors(MODEL_EROHIM, MONSTER01_DIE);
    REQUIRE(erohim.size() == 1);
    CHECK(erohim[0].factor == 0.5f);
    CHECK(erohim[0].when == When::Always);
}

#endif // _EDITOR
