#include "stdafx.h"
#include "doctest.h"

#include "Engine/AI/ZzzAI.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Render/Effects/ZzzEffect.h"

#include <algorithm>
#include <array>
#include <memory>

TEST_CASE("Integer frame counters step once per reference frame at every FPS [gamelogic][timing]")
{
    for (const double fps : std::array{25.0, 30.0, 60.0, 144.0, 250.0, 1000.0})
    {
        CAPTURE(fps);
        const float factor = static_cast<float>(REFERENCE_FPS / fps);
        float referenceFrames = 0.f;
        int steps = 0;
        int largestStep = 0;
        const int frames = static_cast<int>(fps * 10.0);
        for (int frame = 0; frame < frames; ++frame)
        {
            const int step = TakeReferenceFrameSteps(referenceFrames, factor);
            largestStep = std::max(largestStep, step);
            steps += step;
        }
        CHECK(steps >= 249);
        CHECK(steps <= 250);
        CHECK(largestStep == 1);
    }
}

TEST_CASE("Below the reference rate each frame is one step [gamelogic][timing]")
{
    float referenceFrames = 0.f;
    CHECK(TakeReferenceFrameSteps(referenceFrames, 1.f) == 1);
    CHECK(TakeReferenceFrameSteps(referenceFrames, 1.f) == 1);
    CHECK(referenceFrames == 0.f);
}

TEST_CASE("Monster lightning collects at most one target per TargetIndex entry [render]")
{
    for (int i = 0; i < MAX_JOINTS; ++i)
    {
        Joints[i].Live = false;
    }
    static OBJECT caster;
    caster.Live = true;
    const auto characters = std::make_unique<CHARACTER[]>(MAX_CHARACTERS_CLIENT);
    CHARACTER* const savedCharacters = CharactersClient;
    CharactersClient = characters.get();
    vec3_t position = {1000.f, 1000.f, 0.f};
    vec3_t angle = {0.f, 0.f, 0.f};
    for (int i = 0; i < 6; ++i)
    {
        CHARACTER& monster = CharactersClient[i];
        monster.Object.Live = true;
        monster.Object.Visible = true;
        monster.Object.Kind = KIND_MONSTER;
        monster.Dead = 0;
        Vector(1000.f + 10.f * i, 1000.f, 0.f, monster.Object.Position);
    }
    CreateJoint(BITMAP_JOINT_THUNDER, position, position, angle, 15, &caster, 50.f, -1, 0, 77);
    JOINT* joint = std::find_if(Joints, Joints + MAX_JOINTS, [](const JOINT& j) { return j.Live; });
    REQUIRE(joint != Joints + MAX_JOINTS);
    CHECK(joint->Weapon == 5 * 15);
    CHECK(joint->m_bySkillSerialNum == 77);
    joint->Live = false;
    CharactersClient = savedCharacters;
}
