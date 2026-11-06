// doctest unit tests for the playback rule's plan (Render::Models::AnimationSampling):
// which keys of which action a pose is made of, per case.
//
// Run: ctest --test-dir <build directory> -R "animation_sampling"

#include <doctest.h>

#include "Render/Models/AnimationSampling.h"

#include <limits>

namespace Sampling = Render::Models::AnimationSampling;

namespace
{
constexpr Sampling::ActionTimeline Repeating{8, false, 0.3f};
constexpr Sampling::ActionTimeline Locked{7, true, 0.4f};
constexpr Sampling::ActionTimeline Still{8, false, 0.f};
constexpr Sampling::ActionTimeline OneKey{1, false, 0.3f};
// Action_t::Loop: stops on key 15.
constexpr Sampling::ActionTimeline Held{16, false, 0.25f, true};

void CheckKeys(const Sampling::KeySample& sample, unsigned short action, int key0, int key1, float fraction)
{
    CHECK(sample.action == action);
    CHECK(sample.key0 == key0);
    CHECK(sample.key1 == key1);
    CHECK(sample.fraction == doctest::Approx(fraction).epsilon(1e-5));
}
} // namespace

TEST_CASE("key intervals follow MU's wrap [render][animation_sampling]")
{
    CHECK(Sampling::KeyIntervals(Repeating) == 8);
    CHECK(Sampling::KeyIntervals(Locked) == 6);
    CHECK(Sampling::KeyIntervals(OneKey) == 1);
    CHECK(Sampling::KeyIntervals({1, true, 0.f}) == 1);
    CHECK(Sampling::KeyIntervals({0, false, 0.f}) == 1);
}

TEST_CASE("continuous sampling wraps modulo the key intervals [render][animation_sampling]")
{
    CheckKeys(Sampling::SampleAt(Repeating, 3, 2.25f), 3, 2, 3, 0.25f);
    // A repeating action blends its last key into key 0.
    CheckKeys(Sampling::SampleAt(Repeating, 3, 7.5f), 3, 7, 0, 0.5f);
    CheckKeys(Sampling::SampleAt(Repeating, 3, 9.5f), 3, 1, 2, 0.5f);
    CheckKeys(Sampling::SampleAt(Repeating, 3, -0.25f), 3, 7, 0, 0.75f);
    // A locked action never shows key n - 1: key n - 2 blends into key 0.
    CheckKeys(Sampling::SampleAt(Locked, 4, 5.5f), 4, 5, 0, 0.5f);
    CheckKeys(Sampling::SampleAt(Locked, 4, 6.25f), 4, 0, 1, 0.25f);
    CheckKeys(Sampling::SampleAt(OneKey, 1, 0.5f), 1, 0, 0, 0.5f);
    // Rounding at the period and NaN end on key 0.
    CheckKeys(Sampling::SampleAt(Repeating, 3, -1e-9f), 3, 0, 1, 0.f);
    CheckKeys(Sampling::SampleAt(Repeating, 3, std::numeric_limits<float>::quiet_NaN()), 3, 0, 1, 0.f);
    CHECK(Sampling::SampleAt(Repeating, 3, 9.5f).time == doctest::Approx(1.5f));
}

TEST_CASE("a held pose and one key per step are MU's plan [render][animation_sampling]")
{
    SUBCASE("held pose: prior key equals the current key")
    {
        const auto plan = Sampling::PlanPose(Repeating, 2, 3.f, Repeating, 2, 3.6f);
        CHECK(plan.kind == Sampling::PoseCase::Mu);
        CheckKeys(plan.from, 2, 3, 3, 0.f);
        CheckKeys(plan.to, 2, 3, 3, 0.f);
        CHECK(plan.weight == 3.6f - 3.f);
    }
    SUBCASE("one key per step")
    {
        const auto plan = Sampling::PlanPose(Repeating, 2, 4.f, Repeating, 2, 5.25f);
        CHECK(plan.kind == Sampling::PoseCase::Mu);
        CheckKeys(plan.from, 2, 4, 4, 0.f);
        CheckKeys(plan.to, 2, 5, 5, 0.f);
        CHECK(plan.weight == 0.25f);
    }
    SUBCASE("the wrap of a repeating action")
    {
        const auto plan = Sampling::PlanPose(Repeating, 2, 7.f, Repeating, 2, 0.5f);
        CHECK(plan.kind == Sampling::PoseCase::Mu);
        CheckKeys(plan.from, 2, 7, 7, 0.f);
        CheckKeys(plan.to, 2, 0, 0, 0.f);
    }
    SUBCASE("the wrap of a locked action")
    {
        const auto plan = Sampling::PlanPose(Locked, 1, 5.f, Locked, 1, 0.5f);
        CHECK(plan.kind == Sampling::PoseCase::Mu);
        CheckKeys(plan.from, 1, 5, 5, 0.f);
        CheckKeys(plan.to, 1, 0, 0, 0.f);
    }
    SUBCASE("frames outside the keys clamp to key 0 as MU does")
    {
        const auto plan = Sampling::PlanPose(Repeating, 2, 12.f, Repeating, 2, 0.5f);
        CHECK(plan.kind == Sampling::PoseCase::Mu);
        CheckKeys(plan.from, 2, 0, 0, 0.f);
    }
}

TEST_CASE("skipped keys sample the action at frame - 1 [render][animation_sampling]")
{
    SUBCASE("forwards")
    {
        const auto plan = Sampling::PlanPose(Repeating, 2, 3.f, Repeating, 2, 5.25f);
        CHECK(plan.kind == Sampling::PoseCase::Skip);
        CheckKeys(plan.from, 2, 4, 5, 0.25f);
        CheckKeys(plan.to, 2, 4, 5, 0.25f);
        CHECK(plan.weight == 0.f);
    }
    SUBCASE("across the wrap")
    {
        const auto plan = Sampling::PlanPose(Repeating, 2, 6.f, Repeating, 2, 0.5f);
        CHECK(plan.kind == Sampling::PoseCase::Skip);
        CheckKeys(plan.from, 2, 7, 0, 0.5f);
    }
    SUBCASE("a locked action skips over its wrap without key n - 1")
    {
        const auto plan = Sampling::PlanPose(Locked, 1, 4.f, Locked, 1, 0.5f);
        CHECK(plan.kind == Sampling::PoseCase::Skip);
        CheckKeys(plan.from, 1, 5, 0, 0.5f);
    }
    SUBCASE("a frame behind the prior key counts forwards")
    {
        const auto plan = Sampling::PlanPose(Repeating, 2, 4.f, Repeating, 2, 3.5f);
        CHECK(plan.kind == Sampling::PoseCase::Skip);
        CheckKeys(plan.from, 2, 2, 3, 0.5f);
    }
    SUBCASE("an action with one key has nothing to skip")
    {
        const auto plan = Sampling::PlanPose(OneKey, 0, 0.f, OneKey, 0, 0.5f);
        CHECK(plan.kind == Sampling::PoseCase::Mu);
    }
}

TEST_CASE("an action change fades from the pose on screen [render][animation_sampling]")
{
    SUBCASE("prior frame at least 1: the prior action sampled at prior frame - 1")
    {
        const auto plan = Sampling::PlanPose(Repeating, 2, 4.5f, Locked, 1, 0.25f);
        CHECK(plan.kind == Sampling::PoseCase::Crossfade);
        CheckKeys(plan.from, 2, 3, 4, 0.5f);
        CheckKeys(plan.to, 1, 0, 1, 0.f);
        CHECK(plan.weight == 0.25f);
    }
    SUBCASE("prior frame below 1: MU's prior key")
    {
        const auto plan = Sampling::PlanPose(Repeating, 2, 0.5f, Locked, 1, 0.25f);
        CHECK(plan.kind == Sampling::PoseCase::Crossfade);
        CheckKeys(plan.from, 2, 0, 0, 0.f);
        CheckKeys(plan.to, 1, 0, 1, 0.f);
    }
    SUBCASE("a prior action that does not play: MU's prior key")
    {
        const auto plan = Sampling::PlanPose(Still, 5, 4.5f, Locked, 1, 0.25f);
        CHECK(plan.kind == Sampling::PoseCase::Crossfade);
        CheckKeys(plan.from, 5, 4, 4, 0.f);
    }
    SUBCASE("a current frame beyond 1 samples the current action at frame - 1")
    {
        const auto plan = Sampling::PlanPose(Repeating, 2, 4.5f, Locked, 1, 2.75f);
        CHECK(plan.kind == Sampling::PoseCase::Crossfade);
        CheckKeys(plan.to, 1, 1, 2, 0.75f);
        CHECK(plan.weight == 0.75f);
    }
}

TEST_CASE("a held action started over is MU's blend, never beyond its last key [render][animation_sampling]")
{
    SUBCASE("from its end state: key 14 into key 0, as MU")
    {
        const auto plan = Sampling::PlanPose(Held, 6, 14.f, Held, 6, 0.25f);
        CHECK(plan.kind == Sampling::PoseCase::Mu);
        CheckKeys(plan.from, 6, 14, 14, 0.f);
        CheckKeys(plan.to, 6, 0, 0, 0.f);
        CHECK(plan.weight == 0.25f);
    }
    SUBCASE("from an earlier key")
    {
        const auto plan = Sampling::PlanPose(Held, 6, 5.f, Held, 6, 0.5f);
        CHECK(plan.kind == Sampling::PoseCase::Mu);
        CheckKeys(plan.from, 6, 5, 5, 0.f);
        CheckKeys(plan.to, 6, 0, 0, 0.f);
    }
    SUBCASE("forwards a held action still skips keys")
    {
        const auto plan = Sampling::PlanPose(Held, 6, 3.f, Held, 6, 5.25f);
        CHECK(plan.kind == Sampling::PoseCase::Skip);
        CheckKeys(plan.from, 6, 4, 5, 0.25f);
    }
    SUBCASE("a repeating action behind its prior key still counts forwards across the wrap")
    {
        const auto plan = Sampling::PlanPose(Repeating, 2, 4.f, Repeating, 2, 0.5f);
        CHECK(plan.kind == Sampling::PoseCase::Skip);
        CheckKeys(plan.from, 2, 7, 0, 0.5f);
    }
}

TEST_CASE("MU's blend of neighbouring keys stands for the continuous key time prior key + weight [render][animation_sampling]")
{
    const auto phaseOf = [](const Sampling::ActionTimeline& timeline, float priorFrame, float frame) {
        return Sampling::MuBlendPhase(timeline, Sampling::PlanPose(timeline, 2, priorFrame, timeline, 2, frame));
    };
    SUBCASE("one key per step")
    {
        const auto phase = phaseOf(Repeating, 0.75f, 1.5f);
        REQUIRE(phase);
        CheckKeys(*phase, 2, 0, 1, 0.5f);
        CHECK(phase->time == doctest::Approx(0.5f));
    }
    SUBCASE("the wrap of a repeating action: key 7 towards the closing key")
    {
        const auto phase = phaseOf(Repeating, 7.9f, 0.25f);
        REQUIRE(phase);
        CheckKeys(*phase, 2, 7, 0, 0.25f);
        CHECK(phase->time == doctest::Approx(7.25f));
    }
    SUBCASE("the wrap of a locked action: key n - 2 towards the closing key")
    {
        const auto phase = phaseOf(Locked, 5.8f, 0.5f);
        REQUIRE(phase);
        CheckKeys(*phase, 2, 5, 0, 0.5f);
        CHECK(phase->time == doctest::Approx(5.5f));
    }
    SUBCASE("a held action approaching its last key")
    {
        const auto phase = phaseOf(Held, 14.f, 15.5f);
        REQUIRE(phase);
        CHECK(phase->time == doctest::Approx(14.5f));
    }
    SUBCASE("no phase: a held pose, a key itself, skipped keys, a held action started over, an action change")
    {
        CHECK_FALSE(phaseOf(Repeating, 3.f, 3.6f));
        CHECK_FALSE(phaseOf(Repeating, 3.75f, 4.f));
        CHECK_FALSE(phaseOf(Held, 15.f, 15.99f));
        CHECK_FALSE(phaseOf(Repeating, 3.f, 5.25f));
        CHECK_FALSE(phaseOf(Held, 15.f, 0.25f));
        CHECK_FALSE(Sampling::MuBlendPhase(Repeating, Sampling::PlanPose(Repeating, 2, 4.5f, Locked, 1, 0.25f)));
        CHECK_FALSE(phaseOf(OneKey, 0.f, 0.5f));
    }
}
