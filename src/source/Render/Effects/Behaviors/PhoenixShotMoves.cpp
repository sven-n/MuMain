#include "stdafx.h"

#include "PhoenixShotMoves.h"

#include <algorithm>
#include <cmath>

#include "Audio/DSPlaySound.h"
#include "Core/Utilities/Random.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Render/Effects/ZzzEffect.h"
#include "Render/Models/ZzzBMD.h"
#include "Render/Textures/ZzzOpenglUtil.h"

namespace Render::Effects::Behaviors
{
namespace
{
// The rig (bones, no meshes) is created with lifeTime 70 and velocity 1.5 (EffectTypes.json):
// the common effect animation plays it 1.5 keys a tick. Its two arms push it on, each by its
// age times ArmPush keys a tick, while they live.
constexpr float RigLife = 70.f;
constexpr float ArmLife = 9.f;
constexpr float ArmPush = 0.06f;
constexpr int ArmCount = 2;
// The rig's bones each arm's trail runs between: the right arm's, the left arm's.
constexpr int ArmBones[ArmCount][2] = {{1, 2}, {5, 6}};
// While the rig is between these keys, each arm poses it TrailSamples times a tick over the
// TrailSpan keys behind its frame, and lays its trail (an object blur, SwordEffor2_blur) there.
constexpr float TrailFromKey = 3.f;
constexpr float TrailToKey = 18.f;
constexpr int TrailSamples = 10;
constexpr float TrailSpan = 2.f;
constexpr int TrailLife = 30;
constexpr int TrailBlurType = 14;
// Keeps the two arms' trails apart (CreateObjectBlur's SubType).
constexpr int TrailIdentity = 270;

// The rig lets go its blades on its eighth tick, BladeHeight over the caster's feet.
constexpr float LetGoLife = RigLife - 7.f;
constexpr float BladeTurns[] = {0.f, 45.f, 90.f};
constexpr float BladeScale = 0.8f;
constexpr float BladeHeight = 100.f;
// The first blade let go lays the flashes (and lands the blows).
constexpr int LeadBlade = 0;

// A blade is created with lifeTime 50: it flies until it's past its target, then swells and
// fades for StrikeLife ticks.
constexpr float BladeFlight = 50.f;
constexpr float StrikeLife = 30.f;
constexpr float BladeSpeed = 25.f;
constexpr float BladeAcceleration = 2.f;
constexpr float FlightSpin = -70.f;
constexpr float FlightFade = 0.95f;
constexpr float StrikeGrowth = 0.08f;
constexpr float StrikeDim = 0.97f;
constexpr float FadeGrowth = 0.04f;
constexpr float FadeSpin = -10.f;
constexpr float FadeDim = 0.85f;

// Under the first blade as it flies, every tick: two flashes (BITMAP_TWLIGHT subtype 5).
constexpr int FlashSubType = 5;
constexpr float FlashTurns[] = {40.f, 20.f};
constexpr float FlashTurnOffset = 90.f;

// At the strike: sparks (BITMAP_SPARK subtype 14) at one spot over the target, all flying one
// way, and spirals (BITMAP_TWLIGHT subtype 4) on the ground under it.
constexpr int SparkCount = 7;
constexpr int SparkSubType = 14;
constexpr int SparkSpread = 150;
constexpr int SparkRise = 130;
constexpr float SparkHeight = 230.f;
constexpr int SpiralCount = 8;
constexpr int SpiralSubType = 4;
constexpr float SpiralTurn = 30.f;
// Each spiral swells and turns, brightening until its last SpiralFadeLife ticks, then dimming.
constexpr float SpiralGrowth = 0.12f;
constexpr float SpiralSpin = -60.f;
constexpr float SpiralFadeLife = 18.f;
constexpr float SpiralBrighten = 1.05f;
constexpr float SpiralDim = 0.75f;

// Not the Season 6 client's: for BlowTicks ticks after the strike, a burst (Killing Blow's,
// BITMAP_SBUMB) lands each tick on one of the target's bones, a little off it, as clips of the
// skill being played show.
constexpr float BlowTicks = 14.f;
constexpr int BlowJitter = 40;
constexpr float BlowScale = 2.7f;
constexpr int BlowScaleSpread = 35;
constexpr float BlowLowest = 30.f;
// The bursts' subtype: drawn nearer the eye, so the body they land on doesn't hide them
// (ZzzEffectParticle's BITMAP_SBUMB draw).
constexpr int BlowSubType = 1;

// A random offset in [-span / 2, span / 2).
float Jitter(int span)
{
    return static_cast<float>(Random::Rand() % span - span / 2);
}

bool TargetSeen(const OBJECT* o)
{
    return o->m_sTargetIndex >= 0 && o->m_sTargetIndex < MAX_CHARACTERS_CLIENT &&
           CharactersClient[o->m_sTargetIndex].Object.Live;
}

// Where a bone of o's pose (in model space) stands in the world: scaled by o's own Scale, not the model's shared
// BodyScale, which belongs to whichever object set it last.
void BoneInWorld(const OBJECT* o, const float (*bone)[4], vec3_t out)
{
    for (int axis = 0; axis < 3; ++axis)
    {
        out[axis] = o->Position[axis] + bone[axis][3] * o->Scale;
    }
}

// Poses the rig into a pose of its own (not the shared BoneTransform) at each sample.
void LayArmTrails(OBJECT* rig)
{
    static vec34_t pose[MAX_BONES];
    BMD* model = &Models[rig->Type];
    const int bones = std::min<int>(model->NumBones, MAX_BONES);
    vec3_t light;
    Vector(1.f, 1.f, 1.f, light);
    for (int sample = 1; sample <= TrailSamples; ++sample)
    {
        const float frame = rig->AnimationFrame - TrailSpan + TrailSpan * sample / TrailSamples;
        model->Animation(pose, frame, frame, rig->PriorAction, rig->Angle, rig->HeadAngle, false, false, nullptr, -1,
                         bones);
        for (int arm = 0; arm < ArmCount; ++arm)
        {
            vec3_t inner, outer;
            BoneInWorld(rig, pose[ArmBones[arm][0]], inner);
            BoneInWorld(rig, pose[ArmBones[arm][1]], outer);
            CreateObjectBlur(rig, inner, outer, light, TrailBlurType, false, TrailIdentity + arm, TrailLife);
        }
    }
}

// The blades fly at the target the cast was at, which the rig keeps.
void LetGoBlades(OBJECT* rig)
{
    vec3_t position, light;
    VectorCopy(rig->Position, position);
    position[2] += BladeHeight;
    Vector(1.f, 1.f, 1.f, light);
    for (int blade = 0; blade < static_cast<int>(std::size(BladeTurns)); ++blade)
    {
        vec3_t angle;
        Vector(0.f, 0.f, BladeTurns[blade], angle);
        CreateEffect(MODEL_WINDSPIN03, position, angle, light, blade, rig->Owner, -1, 0, 0, 0, BladeScale,
                     rig->m_sTargetIndex);
    }
}

void LayFlashes(OBJECT* blade)
{
    vec3_t light;
    Vector(1.f, 0.7f, 0.3f, light);
    for (const float turn : FlashTurns)
    {
        vec3_t angle;
        Vector(0.f, 0.f, blade->LifeTime * turn + FlashTurnOffset, angle);
        CreateEffectFpsChecked(BITMAP_TWLIGHT, blade->Position, angle, light, FlashSubType, blade);
    }
}

void Strike(OBJECT* blade, const vec3_t target)
{
    vec3_t spot, light, angle;
    Vector(target[0] + Jitter(SparkSpread), target[1] + Jitter(SparkSpread),
           target[2] + static_cast<float>(Random::Rand() % SparkRise) + SparkHeight, spot);
    Vector(1.f, 1.f, 1.f, light);
    Vector(0.f, 0.f, static_cast<float>(Random::Rand() % 360), angle);
    for (int spark = 0; spark < SparkCount; ++spark)
    {
        CreateParticle(BITMAP_SPARK, spot, angle, light, SparkSubType);
    }

    vec3_t ground;
    VectorCopy(target, ground);
    Vector(0.2f, 0.12f, 0.04f, light);
    for (int spiral = 1; spiral <= SpiralCount; ++spiral)
    {
        Vector(0.f, 0.f, SpiralTurn * spiral, angle);
        CreateEffect(BITMAP_TWLIGHT, ground, angle, light, SpiralSubType, blade);
    }

    StopBuffer(SOUND_RAGESKILL_PHOENIXSHOT_ATTACK, true);
    PlayBuffer(SOUND_RAGESKILL_PHOENIXSHOT_ATTACK);

    blade->Position[0] = target[0];
    blade->Position[1] = target[1];
    blade->Scale += StrikeGrowth;
    VectorScale(blade->Light, StrikeDim, blade->Light);
    blade->LifeTime = StrikeLife;
}

// Flies the blade level along the ground from where it was let go toward `target`; true once
// it's farther from there than the target is.
bool FlyPast(OBJECT* blade, vec3_t target)
{
    const float speed = BladeSpeed + BladeAcceleration * (BladeFlight - blade->LifeTime);
    blade->Distance += speed * FPS_ANIMATION_FACTOR;

    const float wayX = target[0] - blade->StartPosition[0];
    const float wayY = target[1] - blade->StartPosition[1];
    const float targetDistance = VectorDistance2D(target, blade->StartPosition);
    if (targetDistance > 0.f)
    {
        blade->Position[0] = blade->StartPosition[0] + wayX / targetDistance * blade->Distance;
        blade->Position[1] = blade->StartPosition[1] + wayY / targetDistance * blade->Distance;
    }
    blade->Angle[2] += FlightSpin * FPS_ANIMATION_FACTOR;
    VectorScale(blade->Light, powf(FlightFade, FPS_ANIMATION_FACTOR), blade->Light);
    return blade->Distance > targetDistance;
}

// One of the target's bones, from the pose the update keeps for it.
bool BoneOf(const OBJECT* target, vec3_t out)
{
    const int bones = std::min<int>(Models[target->Type].NumBones, MAX_BONES);
    if (target->BoneTransform == nullptr || bones <= 0)
    {
        return false;
    }
    BoneInWorld(target, target->BoneTransform[Random::Rand() % bones], out);
    return true;
}

void LandBlow(const OBJECT* blade)
{
    OBJECT* target = &CharactersClient[blade->m_sTargetIndex].Object;
    vec3_t at, light, angle;
    if (!BoneOf(target, at))
    {
        return;
    }
    at[0] += Jitter(BlowJitter);
    at[1] += Jitter(BlowJitter);
    at[2] = std::max(at[2], target->Position[2] + BlowLowest);
    Vector(1.f, 1.f, 1.f, light);
    Vector(0.f, 0.f, 0.f, angle);
    const float scale = BlowScale + static_cast<float>(Random::Rand() % BlowScaleSpread) * 0.02f;
    CreateParticleFpsChecked(BITMAP_SBUMB, at, angle, light, BlowSubType, scale, target);
}

void SwellAndFade(OBJECT* blade)
{
    blade->Scale += FadeGrowth * FPS_ANIMATION_FACTOR;
    blade->Angle[2] += FadeSpin * FPS_ANIMATION_FACTOR;
    VectorScale(blade->Light, powf(FadeDim, FPS_ANIMATION_FACTOR), blade->Light);
}
} // namespace

bool Move_MODEL_PHOENIX_SHOT(OBJECT* o, int index, float Luminosity)
{
    const float age = RigLife - o->LifeTime;
    if (age <= ArmLife)
    {
        o->AnimationFrame += ArmCount * age * ArmPush * FPS_ANIMATION_FACTOR;
        if (o->AnimationFrame >= TrailFromKey && o->AnimationFrame < TrailToKey)
        {
            LayArmTrails(o);
        }
    }
    // Kind starts at 0 (the cast passes no skill) and marks the blades let go.
    if (o->Kind == 0 && o->LifeTime <= LetGoLife)
    {
        o->Kind = 1;
        LetGoBlades(o);
    }
    return true;
}

bool Move_MODEL_WINDSPIN03(OBJECT* o, int index, float Luminosity)
{
    if (o->LifeTime < StrikeLife)
    {
        SwellAndFade(o);
        if (o->SubType == LeadBlade && o->LifeTime >= StrikeLife - BlowTicks && TargetSeen(o))
        {
            LandBlow(o);
        }
        return true;
    }
    if (!TargetSeen(o))
    {
        o->Live = false;
        return false;
    }

    vec3_t target;
    VectorCopy(CharactersClient[o->m_sTargetIndex].Object.Position, target);
    if (o->SubType == LeadBlade)
    {
        LayFlashes(o);
    }
    if (FlyPast(o, target))
    {
        Strike(o, target);
    }
    return true;
}
void MoveStrikeSpiral(OBJECT* o)
{
    o->Scale += SpiralGrowth * FPS_ANIMATION_FACTOR;
    o->Angle[2] += SpiralSpin * FPS_ANIMATION_FACTOR;
    const float change = o->LifeTime < SpiralFadeLife ? SpiralDim : SpiralBrighten;
    VectorScale(o->Light, powf(change, FPS_ANIMATION_FACTOR), o->Light);
}
} // namespace Render::Effects::Behaviors
