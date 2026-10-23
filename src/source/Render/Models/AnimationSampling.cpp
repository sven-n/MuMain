#include "Render/Models/AnimationSampling.h"

#include <algorithm>
#include <cmath>

namespace Render::Models::AnimationSampling
{
namespace
{
// MU's integer key for a frame: (int)frame, and key 0 when that is below 0 or
// beyond the action's keys.
int MuKey(float frame, int numKeys)
{
    const auto key = static_cast<int>(frame);
    if (key < 0 || key >= numKeys)
        return 0;
    return key;
}
} // namespace

int KeyIntervals(const ActionTimeline& timeline)
{
    const int intervals = timeline.lockPositions ? timeline.numKeys - 1 : timeline.numKeys;
    return intervals < 1 ? 1 : intervals;
}

KeySample KeyAt(unsigned short action, int key)
{
    return {action, static_cast<float>(key), key, key, 0.f};
}

KeySample SampleAt(const ActionTimeline& timeline, unsigned short action, float time)
{
    const int intervals = KeyIntervals(timeline);
    const auto period = static_cast<float>(intervals);
    float wrapped = std::fmod(time, period);
    if (wrapped < 0.f)
        wrapped += period;
    // A tiny negative time wraps to `period` itself after rounding; NaN ends here too.
    if (!(wrapped >= 0.f && wrapped < period))
        wrapped = 0.f;

    int key0 = static_cast<int>(wrapped);
    if (key0 >= intervals)
        key0 = intervals - 1;
    const int key1 = (key0 + 1) % intervals;
    return {action, wrapped, key0, key1, wrapped - static_cast<float>(key0)};
}

PosePlan PlanPose(const ActionTimeline& prior, unsigned short priorAction, float priorFrame,
                  const ActionTimeline& current, unsigned short currentAction, float frame)
{
    const int currentKey = MuKey(frame, current.numKeys);
    const int priorKey = MuKey(priorFrame, prior.numKeys);

    PosePlan plan;
    plan.kind = PoseCase::Mu;
    plan.from = KeyAt(priorAction, priorKey);
    plan.to = KeyAt(currentAction, currentKey);
    // BMD::Animation's s1, from the unclamped frame.
    plan.weight = frame - static_cast<float>(static_cast<int>(frame));

    // Fewer than two keys: there is nothing to sample between.
    if (current.numKeys < 2 || prior.numKeys < 1)
        return plan;

    if (priorAction == currentAction)
    {
        // A held action does not repeat: a frame behind the prior key is the action
        // started over (a second death), not keys skipped across a wrap. MU's blend
        // of the prior key into the current one, never key n - 1 into key 0.
        if (current.held && currentKey < priorKey)
            return plan;
        // Keys advanced since the prior key, forwards (no action plays backwards).
        const int intervals = KeyIntervals(current);
        const int advanced = ((currentKey - priorKey) % intervals + intervals) % intervals;
        if (advanced <= 1)
            return plan;

        plan.kind = PoseCase::Skip;
        plan.from = SampleAt(current, currentAction, frame - 1.f);
        plan.to = plan.from;
        plan.weight = 0.f;
        return plan;
    }

    plan.kind = PoseCase::Crossfade;
    if (priorFrame >= 1.f && prior.playSpeed > 0.f && prior.numKeys >= 2)
        plan.from = SampleAt(prior, priorAction, priorFrame - 1.f);
    plan.to = SampleAt(current, currentAction, frame > 1.f ? frame - 1.f : 0.f);
    return plan;
}

std::optional<KeySample> MuBlendPhase(const ActionTimeline& timeline, const PosePlan& plan)
{
    // Weight 0 is key p itself, which MU's plan already samples.
    if (plan.kind != PoseCase::Mu || plan.from.action != plan.to.action || timeline.numKeys < 2 || plan.weight == 0.f)
        return std::nullopt;
    const int priorKey = plan.from.key0;
    const int currentKey = plan.to.key0;
    if (timeline.held && currentKey < priorKey)
        return std::nullopt;
    const int intervals = KeyIntervals(timeline);
    const int advanced = ((currentKey - priorKey) % intervals + intervals) % intervals;
    if (advanced != 1)
        return std::nullopt;
    return SampleAt(timeline, plan.to.action, static_cast<float>(priorKey) + plan.weight);
}

float CrossfadeKeyTime(const ActionTimeline& timeline, float ticks)
{
    const float end = timeline.held ? static_cast<float>(std::max(timeline.numKeys - 1, 0))
                                    : std::max(static_cast<float>(KeyIntervals(timeline)) - timeline.playSpeed, 0.f);
    const float keyTime = ticks * timeline.playSpeed;
    if (!(keyTime > 0.f))
        return 0.f;
    return std::min(keyTime, end);
}

void SetCrossfadeLength(PosePlan& plan, float frame, float fadeKeyTime)
{
    if (plan.kind != PoseCase::Crossfade)
        return;
    plan.weight = fadeKeyTime > 0.f ? std::clamp(frame / fadeKeyTime, 0.f, 1.f) : 1.f;
}

bool CrossfadeContinues(float frame, float fadeKeyTime)
{
    return frame < fadeKeyTime;
}
} // namespace Render::Models::AnimationSampling
