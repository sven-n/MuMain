#include "stdafx.h"
#include "Engine/Object/Movement/Wings.h"

#include "Engine/Object/EnhancedMovement.h"
#include "Render/Models/HeadTurn.h"
#include "Render/Models/ZzzBMD.h"

namespace Engine::Characters
{
namespace
{
// On a Uniria or a Dinorant the riding actions turn the wings' bone askew on the spine, and the wings tilt to a side
// (it's the actions' own keys): there they sit on the spine as they do standing, their bone's turn under the spine
// taken from the standing action, in their bone's place.
void StraightenOnRidingSpine(const CHARACTER* c, int link, float parentMatrix[3][4])
{
    const OBJECT* o = &c->Object;
    const BMD& player = Models[MODEL_PLAYER];
    if ((c->Helper.Type == MODEL_HORN_OF_UNIRIA || c->Helper.Type == MODEL_HORN_OF_DINORANT) && link >= 0 &&
        link < player.NumBones && player.Bones[link].Parent >= 0 && !player.Bones[link].Dummy &&
        player.Bones[link].BoneMatrixes[PLAYER_STOP_MALE].Quaternion != nullptr)
    {
        float standing[3][4];
        QuaternionMatrix(player.Bones[link].BoneMatrixes[PLAYER_STOP_MALE].Quaternion[0], standing);
        standing[0][3] = standing[1][3] = standing[2][3] = 0.f;
        float onSpine[3][4];
        R_ConcatTransforms(o->BoneTransform[player.Bones[link].Parent], standing, onSpine);
        for (int r = 0; r < 3; ++r)
        {
            for (int column = 0; column < 3; ++column)
            {
                parentMatrix[r][column] = onSpine[r][column];
            }
        }
    }
}
} // namespace

void PlaceWingsOnBack(const CHARACTER* c, int linkBone, float parentMatrix[3][4])
{
    const OBJECT* o = &c->Object;
    static const int spine1Bone = Models[MODEL_PLAYER].FindBone("Bip01 Spine1");
    StraightenOnRidingSpine(c, linkBone, parentMatrix);
    // The two spine bones below the neck, where they sit (HeadTurn.h, as BMD::Animation turns the bones), and the turn
    // lag. A bone's turn about its X axis is the opposite way round from the world's upright, hence the minus.
    namespace HeadTurn = Render::Models::HeadTurn;
    const float back = EnhancedMovementEnabled()
                           ? HeadTurn::BackDegrees(HeadTurn::SignedYaw(o->HeadAngle[0])) + c->Motion.TurnLag
                           : 0.f;
    if (spine1Bone >= 0 && back != 0.f)
    {
        HeadTurn::TurnAboutUpright(parentMatrix, -back * (Q_PI / 180.f), o->BoneTransform[spine1Bone][0][3],
                                   o->BoneTransform[spine1Bone][1][3]);
    }
}
} // namespace Engine::Characters
