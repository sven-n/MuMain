#include "stdafx.h"

#include "Scenes/FixedTick.h"

#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzObject.h"
#include "GameLogic/Pets/w_PetProcess.h"
#include "GameLogic/Skills/SkillEffectMgr.h"
#include "Render/Effects/ZzzEffect.h"
#include "Scenes/SceneCore.h"

#include <array>
#include <cmath>
#include <unordered_map>
#include <vector>

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
    int type = -1;
    bool captured = false;
    bool applied = false;
};

// A particle only has a position to interpolate.
struct PointSlot
{
    vec3_t previous{};
    vec3_t current{};
    int type = -1;
    bool captured = false;
    bool applied = false;
};

struct SpriteSlot
{
    int index = 0;
    vec3_t current{};
};

std::vector<Slot> s_slots;
std::vector<PointSlot> s_pointSlots;
std::vector<SpriteSlot> s_movedSprites;
std::unordered_map<const OBJECT*, std::array<float, 3>> s_offsets;
const OBJECT* s_spriteCarrier = nullptr;
std::array<const OBJECT*, MAX_SPRITES> s_spriteCarriers{};
int s_liveSprites = 0;
int s_spritesFollowing = 0;
bool s_on = false;
Clock s_clock;
double s_lastMs = -1.0;
int s_ticksThisFrame = 0;
std::uint64_t s_ticksRun = 0;

template <typename Visit> void ForEachMoved(Visit visit)
{
    std::size_t next = 0;
    const auto each = [&](OBJECT& o)
    {
        if (next == s_slots.size())
            s_slots.emplace_back();
        visit(s_slots[next++], o);
    };
    if (CharactersClient != nullptr)
    {
        for (int i = 0; i < MAX_CHARACTERS_CLIENT; ++i)
            each(CharactersClient[i].Object);
    }
    for (int i = 0; i < MAX_MOUNTS; ++i)
        each(Mounts[i]);
    for (int i = 0; i < MAX_EFFECTS; ++i)
        each(Effects[i]);
    for (int i = 0; i < g_SkillEffects.GetSize(); ++i)
        each(*g_SkillEffects.GetEffect(i));
    for (int i = 0; i < MAX_BOIDS; ++i)
        each(Boids[i]);
    for (int i = 0; i < MAX_FISHS; ++i)
        each(Fishs[i]);
    for (int i = 0; i < MAX_ITEMS; ++i)
        each(Items[i].Object);
    if (g_petProcess)
        ThePetProcess().ForEachObject(each);
}

template <typename Visit> void ForEachPoint(Visit visit)
{
    std::size_t next = 0;
    const auto pool = [&](PARTICLE* particles, int count)
    {
        for (int i = 0; i < count; ++i)
        {
            if (next == s_pointSlots.size())
                s_pointSlots.emplace_back();
            visit(s_pointSlots[next++], particles[i]);
        }
    };
    pool(Particles, MAX_PARTICLES);
    pool(Leaves, MAX_LEAVES);
    pool(Points, MAX_POINTS);
    pool(Pointers, MAX_POINTERS);
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
    for (PointSlot& slot : s_pointSlots)
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
            slot.type = o.Type;
            Copy(o, slot.previous);
        });
    ForEachPoint(
        [](PointSlot& slot, const PARTICLE& p)
        {
            slot.captured = p.Live;
            slot.type = p.Type;
            VectorCopy(p.Position, slot.previous);
        });
}

void ApplyInterpolation()
{
    const float alpha = s_clock.Alpha();
    ForEachMoved(
        [alpha](Slot& slot, OBJECT& o)
        {
            if (!slot.captured || !o.Live || slot.applied || slot.type != o.Type)
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
            if (!VectorCompare(o.Position, slot.current.position))
            {
                s_offsets[&o] = {o.Position[0] - slot.current.position[0], o.Position[1] - slot.current.position[1],
                                 o.Position[2] - slot.current.position[2]};
            }
        });
    ForEachPoint(
        [alpha](PointSlot& slot, PARTICLE& p)
        {
            if (!slot.captured || !p.Live || slot.applied || slot.type != p.Type)
                return;
            VectorCopy(p.Position, slot.current);
            Interpolate(slot.previous, slot.current, alpha, p.Position);
            slot.applied = true;
        });
    // A sprite lasts one tick, so it has no earlier position: it moves with its owner.
    s_liveSprites = 0;
    for (int i = 0; i < MAX_SPRITES; ++i)
    {
        OBJECT& sprite = Sprites[i];
        if (!sprite.Live)
            continue;
        ++s_liveSprites;
        auto offset = s_offsets.find(s_spriteCarriers[i]);
        if (offset == s_offsets.end())
            offset = s_offsets.find(sprite.Owner);
        if (offset == s_offsets.end())
            continue;
        SpriteSlot& slot = s_movedSprites.emplace_back();
        slot.index = i;
        VectorCopy(sprite.Position, slot.current);
        for (int axis = 0; axis < 3; ++axis)
            sprite.Position[axis] += offset->second[axis];
    }
    s_spritesFollowing = static_cast<int>(s_movedSprites.size());
}

void Restore()
{
    if (!s_on && s_slots.empty())
        return;
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
    ForEachPoint(
        [](PointSlot& slot, PARTICLE& p)
        {
            if (!slot.applied)
                return;
            VectorCopy(slot.current, p.Position);
            slot.applied = false;
        });
    for (const SpriteSlot& slot : s_movedSprites)
        VectorCopy(slot.current, Sprites[slot.index].Position);
    s_movedSprites.clear();
    s_offsets.clear();
}

SpriteCarrierScope::SpriteCarrierScope(const OBJECT* carrier) : m_previous(s_spriteCarrier)
{
    s_spriteCarrier = carrier;
}

SpriteCarrierScope::~SpriteCarrierScope()
{
    s_spriteCarrier = m_previous;
}

void NoteSpriteCreated(int sprite)
{
    s_spriteCarriers[sprite] = s_spriteCarrier;
}

void DrawOffset(const OBJECT* o, vec3_t out)
{
    const auto offset = s_offsets.find(o);
    for (int axis = 0; axis < 3; ++axis)
        out[axis] = offset != s_offsets.end() ? offset->second[axis] : 0.f;
}

int LiveSprites()
{
    return s_liveSprites;
}

int SpritesFollowing()
{
    return s_spritesFollowing;
}
} // namespace Scenes::FixedTick
