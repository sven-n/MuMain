#pragma once

#include <cstdint>
#include <optional>

// The playback rule: which keys of which action a skeleton's pose is made of,
// given the frame of the current action and the frame the prior action was left
// at. BMD::Animation poses BMD models with it; a model loaded from another
// format samples its clips with the same plan, so both move alike.
//
// It is MU's rule with two changes:
//  - Skip: when the frame advanced two keys or more since the prior key (play
//    speed above one key per step), MU blends the prior key into the current one
//    and jumps over the keys between. The rule samples the action continuously
//    at frame - 1, where the pose of ordinary playback is.
//  - Crossfade: when the action changed (prior action != current action), MU
//    fades from the prior action's integer key floor(prior frame), not from the
//    pose that was on screen. The rule fades from the prior action sampled at
//    prior frame - 1 (the pose on screen when the action changed), when that
//    frame is at least 1 and the prior action plays at all; to the current
//    action sampled at max(frame - 1, 0).
// Every other case - a held pose, playback of at most one key per step (the
// wrap included), and a held action started over (its frame behind the prior
// key) - is MU's blend of key floor(prior frame) into key floor(frame), and
// BMD::Animation produces MU's bone matrices bit for bit.
namespace Render::Models::AnimationSampling
{
// The part of an action's metadata the rule reads.
struct ActionTimeline
{
    int numKeys = 0;
    bool lockPositions = false;
    float playSpeed = 0.f;
    // Action_t::Loop: the action stops on its last key instead of repeating.
    bool held = false;
};

// The key intervals MU plays an action over: n - 1 for a locked action (it
// wraps at n - 1 and never shows key n - 1), n otherwise (a repeating action
// blends key n - 1 into key 0; a held one stops before n). At least 1.
[[nodiscard]] int KeyIntervals(const ActionTimeline& timeline);

// A pose of one action: key0 blended into key1 by fraction (fraction 0 is key0
// exactly). time is the same point as a continuous key time in
// [0, KeyIntervals) - what a clip with its own keys samples.
struct KeySample
{
    unsigned short action = 0;
    float time = 0.f;
    int key0 = 0;
    int key1 = 0;
    float fraction = 0.f;
};

// Exactly key `key` of an action.
[[nodiscard]] KeySample KeyAt(unsigned short action, int key);

// The action sampled continuously at key time `time`, wrapped modulo
// KeyIntervals (so a locked action's key n - 1 is replaced by key 0).
[[nodiscard]] KeySample SampleAt(const ActionTimeline& timeline, unsigned short action, float time);

enum class PoseCase : std::uint8_t
{
    Mu,        // MU's blend: key floor(prior frame) of the prior action into key floor(frame)
    Skip,      // keys skipped: the current action sampled at frame - 1 (from == to)
    Crossfade, // action changed: the pose on screen faded into the current action
};

// pose = NLERP(from, to, weight); positions are lerped by the same weight.
struct PosePlan
{
    PoseCase kind = PoseCase::Mu;
    KeySample from;
    KeySample to;
    float weight = 0.f;
};

// The plan for one pose. priorAction and currentAction must be valid indices of
// the model's actions, and `prior`/`current` their timelines. The Mu plan holds
// MU's integer keys clamped as MU clamps them (below 0 or beyond the action's
// keys: key 0) and MU's weight frame - (int)frame.
[[nodiscard]] PosePlan PlanPose(const ActionTimeline& prior, unsigned short priorAction, float priorFrame,
                                const ActionTimeline& current, unsigned short currentAction, float frame);

// The continuous key time MU's blend in `plan` stands for, for a model that
// samples its own clips instead of MU's keys: when the plan is MU's blend of
// key p into the next key c of one action (an advance of one key, the wrap
// included), the action sampled at p + weight - between key p and key p + 1,
// which at the wrap is the closing key that equals key 0. Nothing for any
// other plan: weight 0 is key p, a held pose (p == c) is key c, Skip and
// Crossfade already sample continuously, and a held action started over
// blends its two keys as MU.
// `timeline` is the plan's action's.
[[nodiscard]] std::optional<KeySample> MuBlendPhase(const ActionTimeline& timeline, const PosePlan& plan);

// A crossfade of a set length (a model file's "crossfade", in ticks: steps of
// the action's play speed) instead of the rule's fade until frame 1. It still
// starts at the pose on screen when the action changed and fades into the
// current action sampled at max(frame - 1, 0), now over `fadeKeyTime` keys of
// the current action's frame; PlayAnimation keeps the prior action until the
// frame reaches it (CrossfadeContinues), so a fade may outlast frame 1.
//
// The key time a fade of `ticks` lasts in an action: ticks x playSpeed, at most
// the frame a held action stops on (its last key) or, for an action that
// repeats, one step of its play speed before the frame it wraps on
// (KeyIntervals - playSpeed): at the action's own speed or slower the fade
// has run, weight 1, on a frame before the wrap, so the wrap continues the
// current action without a jump. A faster step may pass over that frame; the
// fade then ends on the wrap, as PlayAnimation tests CrossfadeContinues before
// it wraps the frame.
[[nodiscard]] float CrossfadeKeyTime(const ActionTimeline& timeline, float ticks);

// `plan` (from PlanPose) with a Crossfade weighted frame / fadeKeyTime,
// clamped to [0, 1] (fadeKeyTime 0: the current action at once); other plans
// stay as they are.
void SetCrossfadeLength(PosePlan& plan, float frame, float fadeKeyTime);

// Whether a fade of `fadeKeyTime` keys still runs at `frame` of the new action.
[[nodiscard]] bool CrossfadeContinues(float frame, float fadeKeyTime);
} // namespace Render::Models::AnimationSampling
