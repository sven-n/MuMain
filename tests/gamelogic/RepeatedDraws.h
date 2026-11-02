#pragma once

#include "Core/Utilities/DrawSpawnCheck.h"
#include "Core/Utilities/Random.h"
#include "GameLogic/Skills/SkillEffectMgr.h"
#include "Render/Effects/ZzzEffect.h"
#include "doctest.h"

#include <cstdlib>
#include <vector>

// A draw pass, run any number of times after an update, leaves what the next update reads as it was.
namespace RepeatedDraws
{
// The bytes of the shared pools the update reads.
inline std::vector<unsigned char> Pools()
{
    std::vector<unsigned char> bytes;
    const auto add = [&bytes](const void* data, std::size_t size)
    {
        const auto* first = static_cast<const unsigned char*>(data);
        bytes.insert(bytes.end(), first, first + size);
    };
    add(Effects, sizeof(OBJECT) * MAX_EFFECTS);
    for (int i = 0; i < g_SkillEffects.GetSize(); ++i)
        add(g_SkillEffects.GetEffect(i), sizeof(OBJECT));
    add(Sprites, sizeof(OBJECT) * MAX_SPRITES);
    add(Particles, sizeof(PARTICLE) * MAX_PARTICLES);
    add(Joints, sizeof(JOINT) * MAX_JOINTS);
    add(Leaves, sizeof(PARTICLE) * MAX_LEAVES);
    add(Points, sizeof(PARTICLE) * MAX_POINTS);
    add(Pointers, sizeof(PARTICLE) * MAX_POINTERS);
    return bytes;
}

// Runs `draw` `passes` times as the frame runs it, and checks it spawned nothing, left the pools as they were and
// took nothing from the random sequences the update draws from.
template <typename Draw> void CheckChangesNothing(Draw draw, int passes = 2)
{
    const std::vector<unsigned char> before = Pools();
    constexpr unsigned int seed = 914;
    Random::SeedRand(seed);
    const int next = Random::Rand();
    Random::SeedRand(seed);
    const std::mt19937 engine = Random::Snapshot();

    Diagnostics::DrawSpawnCheck::ResetViolations();
    for (int i = 0; i < passes; ++i)
    {
        const Diagnostics::DrawSpawnCheck::DrawScope drawing;
        draw();
    }

    CHECK(Diagnostics::DrawSpawnCheck::Violations() == 0);
    CHECK(Random::Rand() == next);
    CHECK(Random::Snapshot() == engine);
    CHECK(Pools() == before);
}
} // namespace RepeatedDraws
