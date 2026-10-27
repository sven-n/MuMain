#include "stdafx.h"
#include "Engine/Object/Movement/FootPlacement.h"

#include "Engine/Object/EnhancedMovement.h"
#include "Engine/Object/Movement/FlightFeel.h"
#include "Render/Models/BoneMath.h"
#include "Render/Models/ZzzBMD.h"
#include "Render/Terrain/ZzzLodTerrain.h"

#include <algorithm>
#include <array>
#include <cmath>

namespace Engine::Characters
{
namespace
{
namespace BoneMath = Render::Models::BoneMath;

// A foot goes at most this far up or down to the ground under it, and the body down at most this far for it (world
// units); a leg straightens to at most this share of its length, so a knee never locks.
constexpr float FootMaxReach = 40.f;
constexpr float FootMaxStretch = 0.995f;

struct Leg
{
    int thigh = -1;
    int calf = -1;
    int foot = -1;
};

// The player skeleton's legs, by name, found once.
const std::array<Leg, 2>& PlayerLegs()
{
    static const std::array<Leg, 2> legs = []
    {
        const BMD& player = Models[MODEL_PLAYER];
        return std::array<Leg, 2>{
            Leg{player.FindBone("Bip01 L Thigh"), player.FindBone("Bip01 L Calf"), player.FindBone("Bip01 L Foot")},
            Leg{player.FindBone("Bip01 R Thigh"), player.FindBone("Bip01 R Calf"), player.FindBone("Bip01 R Foot")}};
    }();
    return legs;
}

bool IsBelow(const BMD& model, int bone, int ancestor)
{
    for (int b = model.Bones[bone].Parent; b >= 0; b = model.Bones[b].Parent)
    {
        if (b == ancestor)
            return true;
    }
    return false;
}

// Bends a leg (as just built, model space) so its foot reaches `raise` higher than it stands (model units): the thigh
// turns onto the new knee, the calf onto the foot, and the foot keeps its own turn.
void BendLeg(float (*bones)[3][4], const Leg& leg, float raise)
{
    const BMD& player = Models[MODEL_PLAYER];
    float hip[3], knee[3], ankle[3];
    for (int r = 0; r < 3; ++r)
    {
        hip[r] = bones[leg.thigh][r][3];
        knee[r] = bones[leg.calf][r][3];
        ankle[r] = bones[leg.foot][r][3];
    }
    float target[3] = {ankle[0], ankle[1], ankle[2] + raise};
    float thighVector[3], shinVector[3], reach[3];
    BoneMath::Subtract(knee, hip, thighVector);
    BoneMath::Subtract(ankle, knee, shinVector);
    const float thighLength = BoneMath::Length(thighVector);
    const float shinLength = BoneMath::Length(shinVector);
    BoneMath::Subtract(target, hip, reach);
    float distance = BoneMath::Length(reach);
    if (thighLength <= 0.f || shinLength <= 0.f || distance <= 0.f)
        return;
    // Not further than the leg reaches (nearly straight), nor closer than it folds.
    const float most = (thighLength + shinLength) * FootMaxStretch;
    const float least = std::fabs(thighLength - shinLength) + 0.01f;
    const float clamped = std::clamp(distance, least, most);
    for (float& v : reach)
        v *= clamped / distance;
    distance = clamped;
    for (int r = 0; r < 3; ++r)
        target[r] = hip[r] + reach[r];
    // The new knee: in the plane of the hip, the target and where the knee pointed.
    float along[3] = {reach[0] / distance, reach[1] / distance, reach[2] / distance};
    const float kneeAlong = thighVector[0] * along[0] + thighVector[1] * along[1] + thighVector[2] * along[2];
    float bend[3] = {thighVector[0] - along[0] * kneeAlong, thighVector[1] - along[1] * kneeAlong,
                     thighVector[2] - along[2] * kneeAlong};
    const float bendLength = BoneMath::Length(bend);
    if (bendLength < 0.0001f)
        return;
    for (float& v : bend)
        v /= bendLength;
    const float a = (thighLength * thighLength - shinLength * shinLength + distance * distance) / (2.f * distance);
    const float h = std::sqrt(std::max(0.f, thighLength * thighLength - a * a));
    float newKnee[3];
    for (int r = 0; r < 3; ++r)
        newKnee[r] = hip[r] + along[r] * a + bend[r] * h;
    // The thigh turns onto the new knee; the calf, carried by it, turns onto the target.
    float newThigh[3];
    BoneMath::Subtract(newKnee, hip, newThigh);
    float thighTurn[3][3];
    BoneMath::RotationBetween(thighVector, newThigh, thighTurn);
    BoneMath::TurnRotation(bones[leg.thigh], thighTurn);
    float carriedShin[3];
    BoneMath::Rotate(thighTurn, shinVector, carriedShin);
    float newShin[3];
    BoneMath::Subtract(target, newKnee, newShin);
    float shinTurn[3][3];
    BoneMath::RotationBetween(carriedShin, newShin, shinTurn);
    BoneMath::TurnRotation(bones[leg.calf], thighTurn);
    BoneMath::TurnRotation(bones[leg.calf], shinTurn);
    for (int r = 0; r < 3; ++r)
    {
        bones[leg.calf][r][3] = newKnee[r];
    }
    // The foot keeps its own turn (as animated) and moves onto the target, its toes with it.
    float moved[3];
    BoneMath::Subtract(target, ankle, moved);
    for (int b = 0; b < player.NumBones; ++b)
    {
        if (b == leg.foot || IsBelow(player, b, leg.foot))
        {
            for (int r = 0; r < 3; ++r)
                bones[b][r][3] += moved[r];
        }
    }
}
} // namespace

void PlaceFeet(const CHARACTER* c, float (*bones)[3][4])
{
    const OBJECT* o = &c->Object;
    if (!EnhancedMovementEnabled() || o->Type != MODEL_PLAYER || c->Dead != 0 || IsRiding(c) || IsFlying(o) ||
        o->Scale <= 0.f)
    {
        return;
    }
    const std::array<Leg, 2>& legs = PlayerLegs();
    for (const Leg& leg : legs)
    {
        if (leg.thigh < 0 || leg.calf < 0 || leg.foot < 0)
            return;
    }
    const BMD& player = Models[MODEL_PLAYER];
    const float scale = o->Scale;
    // The animation stands on flat ground at the character's own ground; how far each foot's ground is from that.
    const float ground = RequestTerrainHeight(o->Position[0], o->Position[1]);
    float rise[2];
    for (int i = 0; i < 2; ++i)
    {
        const float x = o->Position[0] + bones[legs[i].foot][0][3] * scale;
        const float y = o->Position[1] + bones[legs[i].foot][1][3] * scale;
        rise[i] = std::clamp(RequestTerrainHeight(x, y) - ground, -FootMaxReach, FootMaxReach);
    }
    // The body goes down as far as the lower foot needs to reach (the higher one bends).
    const float drop = std::max(0.f, -std::min(rise[0], rise[1]));
    if (drop == 0.f && rise[0] == 0.f && rise[1] == 0.f)
    {
        return;
    }
    const float dropModel = drop / scale;
    for (int b = 0; b < player.NumBones; ++b)
    {
        bones[b][2][3] -= dropModel;
    }
    for (int i = 0; i < 2; ++i)
    {
        BendLeg(bones, legs[i], (rise[i] + drop) / scale);
    }
}
} // namespace Engine::Characters
