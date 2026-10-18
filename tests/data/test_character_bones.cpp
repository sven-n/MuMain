#include "stdafx.h"

#include "doctest.h"

#include "TestModelSlots.h"

#include "Engine/Object/ZzzCharacter.h"

#include <cmath>
#include <cstring>
#include <filesystem>

namespace
{
const std::filesystem::path PlayerFolder = std::filesystem::path(MU_TEST_DATA_DIR) / "Player";

void PlaceCharacter(OBJECT& o, float x, float scale, float bones[MAX_BONES][3][4])
{
    o.Type = MODEL_PLAYER;
    o.BoneTransform = bones;
    o.CurrentAction = 1;
    o.PriorAction = 0;
    o.AnimationFrame = 2.5f;
    o.PriorAnimationFrame = 1.f;
    o.Scale = scale;
    Vector(x, 300.f, 0.f, o.Position);
    Vector(0.f, 0.f, 45.f, o.Angle);
    Vector(0.f, 0.f, 0.f, o.HeadAngle);
}
} // namespace

TEST_CASE("Character bones from the pre-pass do not depend on the model's last drawn object [data][animation]")
{
    TestModelSlots slots;
    BMD& model = Models[MODEL_PLAYER];
    const std::wstring folder = PlayerFolder.wstring() + L"/";
    REQUIRE(model.Open2(folder.c_str(), L"player.bmd"));
    REQUIRE(model.NumBones > 0);

    static float nearBones[MAX_BONES][3][4];
    static float farBones[MAX_BONES][3][4];
    static float alone[MAX_BONES][3][4];
    OBJECT a;
    OBJECT b;
    PlaceCharacter(a, 100.f, 0.9f, nearBones);
    PlaceCharacter(b, 9000.f, 1.4f, farBones);

    // Another object of the same model was drawn last somewhere else.
    Vector(5000.f, 5000.f, 500.f, model.BodyOrigin);
    model.BodyScale = 3.f;
    AnimateCharacterBones(&b);
    AnimateCharacterBones(&a);

    // Render's own bones for a: model space, after setting a's origin and scale.
    VectorCopy(a.Position, model.BodyOrigin);
    model.BodyScale = a.Scale;
    model.Animation(alone, a.AnimationFrame, a.PriorAnimationFrame, a.PriorAction, a.Angle, a.HeadAngle, false, false,
                    nullptr, a.CurrentAction);

    CHECK(a.EnableBoneMatrix);
    CHECK(std::memcmp(nearBones, alone, sizeof(float) * 12 * model.NumBones) == 0);
    // The root stays near the model origin rather than at a world position.
    CHECK(std::abs(nearBones[0][0][3]) < 1000.f);
    CHECK(std::abs(farBones[0][0][3]) < 1000.f);
}
