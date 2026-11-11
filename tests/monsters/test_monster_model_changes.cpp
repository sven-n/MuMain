#include "stdafx.h"

// A character of a model that the monster editor opened again stays inside
// the model's animations (monsters design, D55).

#include "Engine/Object/w_ObjectInfo.h"
#include "GameLogic/Monsters/MonsterModelChanges.h"
#include "Render/Models/ZzzBMD.h"

#include <doctest.h>

using GameLogic::Monsters::ModelChanges::ClampAnimation;

namespace
{
// A model with two animations of 10 and 4 frames; the BMD frees them.
void GiveTwoAnimations(BMD& model)
{
    model.NumActions = 2;
    model.Actions = new Action_t[2]{};
    model.Actions[0].NumAnimationKeys = 10;
    model.Actions[1].NumAnimationKeys = 4;
}
} // namespace

TEST_CASE("Monster models: an animation the model no longer has goes back to the first (D55) [monsters][editor]")
{
    BMD model;
    GiveTwoAnimations(model);
    OBJECT object;
    object.CurrentAction = 5;
    object.AnimationFrame = 7.5f;
    object.PriorAction = 1;
    object.PriorAnimationFrame = 6.0f;

    ClampAnimation(object, model);

    CHECK(object.CurrentAction == 0);
    CHECK(object.AnimationFrame == 0.0f);
    // An animation it has keeps its number; a frame past its end starts it
    // again.
    CHECK(object.PriorAction == 1);
    CHECK(object.PriorAnimationFrame == 0.0f);
}

TEST_CASE("Monster models: an animation inside the model stays as it is (D55) [monsters][editor]")
{
    BMD model;
    GiveTwoAnimations(model);
    OBJECT object;
    object.CurrentAction = 1;
    object.AnimationFrame = 3.5f;
    object.PriorAction = 0;
    object.PriorAnimationFrame = 9.0f;

    ClampAnimation(object, model);

    CHECK(object.CurrentAction == 1);
    CHECK(object.AnimationFrame == 3.5f);
    CHECK(object.PriorAction == 0);
    CHECK(object.PriorAnimationFrame == 9.0f);

    // A model without animations (its file is gone) leaves the first
    // animation's start.
    const BMD empty;
    ClampAnimation(object, empty);
    CHECK(object.CurrentAction == 0);
    CHECK(object.AnimationFrame == 0.0f);
}
