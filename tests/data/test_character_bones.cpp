#include "stdafx.h"

#include "doctest.h"

#include "TestModelSlots.h"

#include "Engine/Object/ObjectPose.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzObject.h"

#include <cmath>
#include <cstring>
#include <filesystem>
#include <span>

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

TEST_CASE("The pre-pass animates characters at render's height, not one an item preview left [data][animation]")
{
    TestModelSlots slots;
    BMD& model = Models[MODEL_PLAYER];
    const std::wstring folder = PlayerFolder.wstring() + L"/";
    REQUIRE(model.Open2(folder.c_str(), L"player.bmd"));

    // The root height applies only to actions that lock positions.
    int lockedAction = -1;
    for (int action = 0; action < model.NumActions && lockedAction < 0; ++action)
    {
        if (model.Actions[action].LockPositions && model.Actions[action].NumAnimationKeys > 1) lockedAction = action;
    }
    REQUIRE(lockedAction >= 0);

    static float bones[MAX_BONES][3][4];
    static float rendered[MAX_BONES][3][4];
    static CHARACTER characters[1];
    OBJECT& o = characters[0].Object;
    PlaceCharacter(o, 100.f, 1.f, bones);
    o.CurrentAction = o.PriorAction = static_cast<unsigned short>(lockedAction);
    o.Live = true;
    o.Visible = true;

    model.BodyHeight = -160.f;
    UpdateCharactersAnimationParallel(std::span<CHARACTER>(characters, 1));

    model.BodyHeight = 0.f;
    model.Animation(rendered, o.AnimationFrame, o.PriorAnimationFrame, o.PriorAction, o.Angle, o.HeadAngle, false, false,
                    nullptr, o.CurrentAction);
    CHECK(bones[0][2][3] == rendered[0][2][3]);
    CHECK(std::memcmp(bones, rendered, sizeof(float) * 12 * model.NumBones) == 0);
}

TEST_CASE("A character's pose stays current until its animation state changes [data][animation]")
{
    TestModelSlots slots;
    BMD& model = Models[MODEL_PLAYER];
    const std::wstring folder = PlayerFolder.wstring() + L"/";
    REQUIRE(model.Open2(folder.c_str(), L"player.bmd"));

    static float bones[MAX_BONES][3][4];
    OBJECT o;
    PlaceCharacter(o, 100.f, 1.f, bones);
    CHECK_FALSE(Engine::Object::Pose::IsCurrent(&o));
    AnimateCharacterBones(&o);
    CHECK(Engine::Object::Pose::IsCurrent(&o));

    const auto changed = [&](auto change)
    {
        OBJECT copy = o;
        change(copy);
        return !Engine::Object::Pose::IsCurrent(&copy);
    };
    CHECK(changed([](OBJECT& c) { c.Type = MODEL_PLAYER + 1; }));
    CHECK(changed([](OBJECT& c) { c.CurrentAction = 2; }));
    CHECK(changed([](OBJECT& c) { c.PriorAction = 1; }));
    CHECK(changed([](OBJECT& c) { c.AnimationFrame += 0.25f; }));
    CHECK(changed([](OBJECT& c) { c.PriorAnimationFrame += 0.25f; }));
    CHECK(changed([](OBJECT& c) { c.Angle[2] += 1.f; }));
    CHECK(changed([](OBJECT& c) { c.HeadAngle[0] += 1.f; }));

    Engine::Object::Pose::Invalidate(&o);
    CHECK_FALSE(Engine::Object::Pose::IsCurrent(&o));
}

TEST_CASE("A draw reuses the pose the update built, and animates it after a change [data][animation]")
{
    TestModelSlots slots;
    BMD& model = Models[MODEL_PLAYER];
    const std::wstring folder = PlayerFolder.wstring() + L"/";
    REQUIRE(model.Open2(folder.c_str(), L"player.bmd"));

    static float bones[MAX_BONES][3][4];
    static float fresh[MAX_BONES][3][4];
    OBJECT o;
    PlaceCharacter(o, 100.f, 1.f, bones);
    o.Alpha = 1.f;
    AnimateCharacterBones(&o);

    // A marker the draw would overwrite if it animated the unchanged pose again.
    bones[0][0][3] = 12345.f;
    REQUIRE(Calc_ObjectAnimation(&o, true, 0));
    CHECK(bones[0][0][3] == 12345.f);

    // A change during the draw, like SetPlayerStop, gets its own pose.
    o.AnimationFrame += 0.5f;
    REQUIRE(Calc_ObjectAnimation(&o, true, 0));
    model.Animation(fresh, o.AnimationFrame, o.PriorAnimationFrame, o.PriorAction, o.Angle, o.HeadAngle, false, false,
                    nullptr, o.CurrentAction);
    CHECK(std::memcmp(bones, fresh, sizeof(float) * 12 * model.NumBones) == 0);
    CHECK(Engine::Object::Pose::IsCurrent(&o));
}

TEST_CASE("The pre-pass leaves a current pose alone [data][animation]")
{
    TestModelSlots slots;
    BMD& model = Models[MODEL_PLAYER];
    const std::wstring folder = PlayerFolder.wstring() + L"/";
    REQUIRE(model.Open2(folder.c_str(), L"player.bmd"));

    static float bones[MAX_BONES][3][4];
    OBJECT o;
    PlaceCharacter(o, 100.f, 1.f, bones);
    AnimateCharacterBones(&o);
    bones[0][0][3] = 12345.f;
    AnimateCharacterBones(&o);
    CHECK(bones[0][0][3] == 12345.f);

    o.AnimationFrame += 0.5f;
    AnimateCharacterBones(&o);
    CHECK(bones[0][0][3] != 12345.f);
}

#ifdef MU_ENABLE_CONTROL_SOCKET
TEST_CASE("The bones check counts a character whose bone differs [data][animation]")
{
    static float prepass[2][3][4];
    static float rendered[2][3][4];
    g_PrepassBoneCheck = PrepassBoneCheck{};
    RecordPrepassBones(prepass, rendered, 2, true);
    rendered[1][2][3] = -160.f;
    RecordPrepassBones(prepass, rendered, 2, false);

    CHECK(g_PrepassBoneCheck.checked == 2);
    CHECK(g_PrepassBoneCheck.players == 1);
    CHECK(g_PrepassBoneCheck.mismatched == 1);
    CHECK(g_PrepassBoneCheck.maxDelta == doctest::Approx(160.f));
    g_PrepassBoneCheck = PrepassBoneCheck{};
}
#endif
