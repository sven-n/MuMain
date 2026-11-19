#include "stdafx.h"
#include "Engine/Object/Movement/PoseFade.h"

#include "Engine/Object/EnhancedMovement.h"
#include "Engine/Object/Movement/FrameTime.h"
#include "Engine/Object/Movement/Stride.h"
#include "Render/Textures/ZzzOpenglUtil.h"

#include <algorithm>
#include <cmath>

namespace Engine::Characters
{
namespace
{
// A player's switch from one action to the next fades over this long, by where it goes: moving, emotes and rest fade
// gently; an attack, a skill or a hit fades just enough to lose the snap and still start at once; death and the
// special transformations don't fade.
constexpr float PoseFadeSeconds = 0.25f;
constexpr float PoseFadeBriefSeconds = 0.08f;

bool InRange(int action, int first, int last)
{
    return action >= first && action <= last;
}

float PoseFadeSecondsFor(const OBJECT* o, int from, int to)
{
    if (o->Type != MODEL_PLAYER)
    {
        return 0.f;
    }
    const auto still = [](int action)
    {
        return InRange(action, PLAYER_DIE1, PLAYER_DIE2) || action == PLAYER_COME_UP ||
               InRange(action, PLAYER_JACK_1, PLAYER_CHANGE_UP);
    };
    if (still(from) || still(to))
    {
        return 0.f;
    }
    const bool gentle =
        InRange(to, PLAYER_STOP_MALE, PLAYER_RUN_RIDE_WEAPON) || InRange(to, PLAYER_FLY_RIDE, PLAYER_RUN_RIDE_HORSE) ||
        InRange(to, PLAYER_IDLE1_DARKHORSE, PLAYER_IDLE2_DARKHORSE) ||
        InRange(to, PLAYER_FENRIR_RUN, PLAYER_FENRIR_WALK_ONE_LEFT) ||
        InRange(to, PLAYER_GREETING1, PLAYER_POINT_DANCE) || InRange(to, PLAYER_SIT1, PLAYER_POSE_FEMALE1);
    return gentle ? PoseFadeSeconds : PoseFadeBriefSeconds;
}
} // namespace

void UpdatePoseFade(CHARACTER* c, OBJECT* o, float playSpeed)
{
    if (!EnhancedMovementEnabled())
    {
        c->Motion.FadeLeft = 0.f;
        return;
    }
    BMD& b = Models[o->Type];
    const float seconds = FrameSeconds();
    const float fadeSeconds = c->Motion.FadeLastAction >= 0 && o->CurrentAction != c->Motion.FadeLastAction &&
                                      c->Motion.FadeLastAction < b.NumActions
                                  ? PoseFadeSecondsFor(o, c->Motion.FadeLastAction, o->CurrentAction)
                                  : 0.f;
    if (fadeSeconds > 0.f)
    {
        c->Motion.FadeAction = c->Motion.FadeLastAction;
        c->Motion.FadeFrame = c->Motion.FadeLastFrame;
        c->Motion.FadeLeft = 1.f;
        c->Motion.FadeSeconds = fadeSeconds;
        const int oldKeys = b.Actions[c->Motion.FadeAction].NumAnimationKeys;
        const int newKeys = b.Actions[o->CurrentAction].NumAnimationKeys;
        const Stride& from = MeasureStride(c->Motion.FadeAction);
        const Stride& to = MeasureStride(o->CurrentAction);
        if (from.stepPerKey > 0.f && to.stepPerKey > 0.f && oldKeys > 0 && newKeys > 0)
        {
            // The same point of the stride by the feet: as far past the first foot's furthest forward.
            const float phase = c->Motion.FadeLastFrame / static_cast<float>(oldKeys) - from.leftForwardAt;
            const float wrapped = phase - std::floor(phase);
            const float newPhase = wrapped + to.leftForwardAt;
            o->AnimationFrame = (newPhase - std::floor(newPhase)) * static_cast<float>(newKeys);
        }
        // The fade does the blending: no blend from the last action's key as well.
        o->PriorAction = o->CurrentAction;
        o->PriorAnimationFrame = static_cast<float>(static_cast<int>(o->AnimationFrame));
    }
    else if (o->CurrentAction != c->Motion.FadeLastAction)
    {
        // A switch that doesn't fade (death, a transformation) ends a fade under way.
        c->Motion.FadeLeft = 0.f;
    }
    else if (c->Motion.FadeLeft > 0.f && c->Motion.FadeAction >= 0 && c->Motion.FadeSeconds > 0.f)
    {
        c->Motion.FadeLeft = std::max(0.f, c->Motion.FadeLeft - seconds / c->Motion.FadeSeconds);
        const Action_t& faded = b.Actions[c->Motion.FadeAction];
        const int fadeKeys = faded.NumAnimationKeys;
        const int keys = b.Actions[o->CurrentAction].NumAnimationKeys;
        if (fadeKeys > 0 && keys > 0)
        {
            // Two strides keep one pace through the stride, as the new action plays; anything else plays on at its
            // own speed, and an action played once holds its end.
            const bool strides = StepPerKey(c->Motion.FadeAction) > 0.f && StepPerKey(o->CurrentAction) > 0.f;
            const float advance =
                strides ? playSpeed * static_cast<float>(fadeKeys) / static_cast<float>(keys) : faded.PlaySpeed;
            const float frame = c->Motion.FadeFrame + advance * FPS_ANIMATION_FACTOR;
            c->Motion.FadeFrame = faded.Loop ? std::min(frame, static_cast<float>(fadeKeys) - 0.01f)
                                             : std::fmod(frame, static_cast<float>(fadeKeys));
        }
    }
}

const PoseBlend* CharacterPoseFade(const CHARACTER* c, PoseBlend& out)
{
    if (c == nullptr || c->Motion.FadeLeft <= 0.f || c->Motion.FadeAction < 0)
    {
        return nullptr;
    }
    out.Action = static_cast<unsigned short>(c->Motion.FadeAction);
    out.Frame = c->Motion.FadeFrame;
    // Eased: it leaves gently and lets go gently.
    out.Weight = c->Motion.FadeLeft * c->Motion.FadeLeft * (3.f - 2.f * c->Motion.FadeLeft);
    return &out;
}
} // namespace Engine::Characters
