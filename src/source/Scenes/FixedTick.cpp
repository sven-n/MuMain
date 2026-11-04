#include "stdafx.h"

#include "Scenes/FixedTick.h"

#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzObject.h"
#include "Scenes/SceneCore.h"

#include <array>
#include <cmath>

extern float FPS_ANIMATION_FACTOR;
extern int REFERENCE_FRAME_STEP;
extern unsigned int RENDERED_FRAME;

namespace Scenes::FixedTick
{
namespace
{
constexpr float MaxInterpolatedDistance = 200.f;

// What the object's draw shows, kept to give it back.
struct Transform
{
    vec3_t position{};
    vec3_t angle{};
    float frame = 0.f;
    float priorFrame = 0.f;
    unsigned short action = 0;
    unsigned short priorAction = 0;
};

void Copy(const OBJECT& from, Transform& to)
{
    VectorCopy(from.Position, to.position);
    VectorCopy(from.Angle, to.angle);
    to.frame = from.AnimationFrame;
    to.priorFrame = from.PriorAnimationFrame;
    to.action = from.CurrentAction;
    to.priorAction = from.PriorAction;
}

struct Slot
{
    Transform previous;
    Transform current;
    bool captured = false;
    bool applied = false;
};

std::array<Slot, MAX_CHARACTERS_CLIENT + MAX_MOUNTS> s_slots{};
bool s_on = false;
Clock s_clock;
double s_lastMs = -1.0;
int s_ticksThisFrame = 0;
std::uint64_t s_ticksRun = 0;

template <typename Visit> void ForEachMoved(Visit visit)
{
    if (CharactersClient != nullptr)
    {
        for (int i = 0; i < MAX_CHARACTERS_CLIENT; ++i)
            visit(s_slots[i], CharactersClient[i].Object);
    }
    for (int i = 0; i < MAX_MOUNTS; ++i)
        visit(s_slots[MAX_CHARACTERS_CLIENT + i], Mounts[i]);
}
} // namespace

int Clock::Advance(double elapsedMs)
{
    m_pendingMs += elapsedMs;
    const int ticks = static_cast<int>(m_pendingMs / TickMs);
    m_pendingMs -= ticks * TickMs;
    return ticks < MaxTicksPerFrame ? ticks : MaxTicksPerFrame;
}

float Clock::Alpha() const
{
    return static_cast<float>(m_pendingMs / TickMs);
}

void Interpolate(const vec3_t previous, const vec3_t current, float alpha, vec3_t out)
{
    float distanceSquared = 0.f;
    for (int i = 0; i < 3; ++i)
        distanceSquared += (current[i] - previous[i]) * (current[i] - previous[i]);
    const bool jumped = distanceSquared > MaxInterpolatedDistance * MaxInterpolatedDistance;
    for (int i = 0; i < 3; ++i)
        out[i] = jumped ? current[i] : previous[i] + (current[i] - previous[i]) * alpha;
}

float InterpolateAngle(float previous, float current, float alpha)
{
    float difference = std::fmod(current - previous, 360.f);
    if (difference > 180.f)
        difference -= 360.f;
    else if (difference < -180.f)
        difference += 360.f;
    return previous + difference * alpha;
}

float InterpolateFrame(float previous, float current, float alpha, bool sameAction)
{
    if (!sameAction || current < previous)
        return current;
    return previous + (current - previous) * alpha;
}

bool IsOn()
{
    return s_on;
}

void SetOn(bool on)
{
    s_on = on;
    s_lastMs = -1.0;
    s_ticksThisFrame = 0;
    s_clock = Clock{};
    for (Slot& slot : s_slots)
        slot.captured = false;
}

bool RunsWorld()
{
    return s_on && SceneFlag == MAIN_SCENE;
}

void BeginFrame(double nowMs)
{
    s_ticksThisFrame = 0;
    if (!RunsWorld())
    {
        s_lastMs = -1.0;
        return;
    }
    if (s_lastMs >= 0.0)
        s_ticksThisFrame = s_clock.Advance(nowMs - s_lastMs);
    s_lastMs = nowMs;
}

int TicksThisFrame()
{
    return s_ticksThisFrame;
}

std::uint64_t TicksRun()
{
    return s_ticksRun;
}

TickScope::TickScope() : m_factor(FPS_ANIMATION_FACTOR), m_step(REFERENCE_FRAME_STEP)
{
    FPS_ANIMATION_FACTOR = 1.f;
    REFERENCE_FRAME_STEP = 1;
    ++RENDERED_FRAME;
    ++s_ticksRun;
}

TickScope::~TickScope()
{
    FPS_ANIMATION_FACTOR = m_factor;
    REFERENCE_FRAME_STEP = m_step;
}

void CaptureTransforms()
{
    ForEachMoved(
        [](Slot& slot, const OBJECT& o)
        {
            slot.captured = o.Live;
            Copy(o, slot.previous);
        });
}

void ApplyInterpolation()
{
    const float alpha = s_clock.Alpha();
    ForEachMoved(
        [alpha](Slot& slot, OBJECT& o)
        {
            if (!slot.captured || !o.Live || slot.applied)
                return;
            Copy(o, slot.current);
            Interpolate(slot.previous.position, slot.current.position, alpha, o.Position);
            for (int i = 0; i < 3; ++i)
                o.Angle[i] = InterpolateAngle(slot.previous.angle[i], slot.current.angle[i], alpha);
            o.AnimationFrame = InterpolateFrame(slot.previous.frame, slot.current.frame, alpha,
                                                slot.previous.action == slot.current.action);
            // Still on the previous tick's key: it blends from what that tick blended from.
            if (static_cast<int>(o.AnimationFrame) == static_cast<int>(slot.previous.frame) &&
                slot.previous.action == slot.current.action)
            {
                o.PriorAnimationFrame = slot.previous.priorFrame;
                o.PriorAction = slot.previous.priorAction;
            }
            slot.applied = true;
        });
}

void Restore()
{
    ForEachMoved(
        [](Slot& slot, OBJECT& o)
        {
            if (!slot.applied)
                return;
            VectorCopy(slot.current.position, o.Position);
            VectorCopy(slot.current.angle, o.Angle);
            o.AnimationFrame = slot.current.frame;
            o.PriorAnimationFrame = slot.current.priorFrame;
            o.PriorAction = slot.current.priorAction;
            slot.applied = false;
        });
}
} // namespace Scenes::FixedTick
