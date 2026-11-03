// doctest unit tests for BMD::Animation under the playback rule
// (Render/Models/AnimationSampling.h), on a small synthetic skeleton: where the
// rule keeps MU's pose the bone matrices equal MU's bit for bit (MU's pose code
// is kept below as the reference); skipped keys show the action sampled
// continuously; an action change fades from the pose on screen; a locked root
// keeps key 0's x and y; the head angle turns every sampled key.
//
// Run: ctest --test-dir <build directory> -R "animation_playback_rule"

#include "App/stdafx.h"

#include <doctest.h>

#include "Render/Models/ZzzBMD.h"

#include <cmath>
#include <cstddef>
#include <cstring>
#include <memory>
#include <random>
#include <vector>

namespace
{
constexpr int BoneCount = 5;
constexpr int HeadBone = 3;

enum TestAction : unsigned short
{
    Walk = 0,   // repeating, 6 keys
    Run = 1,    // repeating and locked, 7 keys
    Die = 2,    // held, 5 keys
    Stand = 3,  // repeating, does not play (speed 0)
    Count = 4,
};

struct Matrices
{
    float bones[MAX_BONES][3][4];
};

// A model whose skeleton is a chain of BoneCount bones with random keys, owning
// its key arrays (the BMD only points at them).
struct SyntheticModel
{
    BMD model;
    std::vector<Action_t> actions;
    std::vector<Bone_t> bones;
    std::vector<BoneMatrix_t> boneMatrixes;
    std::vector<std::unique_ptr<vec3_t[]>> positions;
    std::vector<std::unique_ptr<vec3_t[]>> rotations;
    std::vector<std::unique_ptr<vec4_t[]>> quaternions;

    SyntheticModel()
    {
        actions = {
            {false, 0.3f, 6, false, nullptr},
            {false, 0.45f, 7, true, nullptr},
            {true, 0.25f, 5, false, nullptr},
            {false, 0.f, 4, false, nullptr},
        };
        std::mt19937 random(1234);
        std::uniform_real_distribution<float> angle(-1.2f, 1.2f);
        std::uniform_real_distribution<float> offset(-20.f, 20.f);

        constexpr std::size_t Slots = static_cast<std::size_t>(BoneCount) * Count;
        bones.resize(BoneCount);
        boneMatrixes.resize(Slots);
        positions.resize(Slots);
        rotations.resize(Slots);
        quaternions.resize(Slots);
        for (int bone = 0; bone < BoneCount; ++bone)
        {
            Bone_t& b = bones[bone];
            std::memset(&b, 0, sizeof(b));
            b.Parent = static_cast<short>(bone - 1);
            b.Dummy = 0;
            b.BoneMatrixes = &boneMatrixes[static_cast<std::size_t>(bone) * Count];
            for (int action = 0; action < Count; ++action)
            {
                const std::size_t slot = static_cast<std::size_t>(bone) * Count + action;
                const int keys = actions[action].NumAnimationKeys;
                positions[slot] = std::make_unique<vec3_t[]>(keys);
                rotations[slot] = std::make_unique<vec3_t[]>(keys);
                quaternions[slot] = std::make_unique<vec4_t[]>(keys);
                for (int key = 0; key < keys; ++key)
                {
                    Vector(offset(random), offset(random), offset(random), positions[slot][key]);
                    Vector(angle(random), angle(random), angle(random), rotations[slot][key]);
                    AngleQuaternion(rotations[slot][key], quaternions[slot][key]);
                }
                boneMatrixes[slot] = {positions[slot].get(), rotations[slot].get(), quaternions[slot].get()};
            }
        }

        model.NumActions = Count;
        model.Actions = actions.data();
        model.NumBones = BoneCount;
        model.Bones = bones.data();
        model.BoneHead = HeadBone;
        model.BodyScale = 1.25f;
        Vector(100.f, -50.f, 30.f, model.BodyOrigin);
        model.BodyHeight = 7.f;
        model.CurrentAction = Walk;
    }

    ~SyntheticModel()
    {
        model.Actions = nullptr;
        model.Bones = nullptr;
        model.NumActions = 0;
        model.NumBones = 0;
    }

    SyntheticModel(const SyntheticModel&) = delete;
    SyntheticModel& operator=(const SyntheticModel&) = delete;
};

// MU's pose code as it was before the playback rule (BMD::Animation with
// Parent = false and Translate = true), the reference for the cases the rule
// keeps.
void MuAnimation(const BMD& m, float (*BoneMatrix)[3][4], float AnimationFrame, float PriorFrame,
                 unsigned short PriorAction, unsigned short currentAction, vec3_t Angle, vec3_t HeadAngle)
{
    vec4_t boneQuaternion[MAX_BONES] = {};
    float currentAnimation = AnimationFrame;
    int currentAnimationFrame = (int)AnimationFrame;
    float s1 = (currentAnimation - currentAnimationFrame);
    float s2 = 1.f - s1;
    auto PriorAnimationFrame = (int)PriorFrame;
    if (PriorAnimationFrame < 0)
        PriorAnimationFrame = 0;
    if (currentAnimationFrame < 0)
        currentAnimationFrame = 0;
    if (PriorAnimationFrame >= m.Actions[PriorAction].NumAnimationKeys)
        PriorAnimationFrame = 0;
    if (currentAnimationFrame >= m.Actions[currentAction].NumAnimationKeys)
        currentAnimationFrame = 0;

    float localParentMatrix[3][4];
    AngleMatrix(Angle, localParentMatrix);
    for (auto& y : localParentMatrix)
        for (int x = 0; x < 3; ++x)
            y[x] *= m.BodyScale;
    localParentMatrix[0][3] = m.BodyOrigin[0];
    localParentMatrix[1][3] = m.BodyOrigin[1];
    localParentMatrix[2][3] = m.BodyOrigin[2];

    vec4_t headQ1, headQ2;
    {
        const Bone_t* hb = &m.Bones[m.BoneHead];
        const BoneMatrix_t* hbm1 = &hb->BoneMatrixes[PriorAction];
        const BoneMatrix_t* hbm2 = &hb->BoneMatrixes[currentAction];
        vec3_t Angle1, Angle2;
        VectorCopy(hbm1->Rotation[PriorAnimationFrame], Angle1);
        VectorCopy(hbm2->Rotation[currentAnimationFrame], Angle2);
        constexpr float radFactor = 1.0f / (180.f / Q_PI);
        const float HeadAngleX = HeadAngle[0] * radFactor;
        const float HeadAngleY = HeadAngle[1] * radFactor;
        Angle1[0] -= HeadAngleX;
        Angle2[0] -= HeadAngleX;
        Angle1[2] -= HeadAngleY;
        Angle2[2] -= HeadAngleY;
        AngleQuaternion(Angle1, headQ1);
        AngleQuaternion(Angle2, headQ2);
    }

    const bool bLockPositions = (m.Actions[PriorAction].LockPositions || m.Actions[currentAction].LockPositions);
    for (int i = 0; i < m.NumBones; i++)
    {
        const Bone_t* b = &m.Bones[i];
        const BoneMatrix_t* bm1 = &b->BoneMatrixes[PriorAction];
        const BoneMatrix_t* bm2 = &b->BoneMatrixes[currentAction];
        const float* q1 = i == m.BoneHead ? headQ1 : bm1->Quaternion[PriorAnimationFrame];
        const float* q2 = i == m.BoneHead ? headQ2 : bm2->Quaternion[currentAnimationFrame];
        if (!QuaternionCompare(q1, q2))
        {
            QuaternionNLERP(q1, q2, s1, boneQuaternion[i]);
        }
        else
        {
            QuaternionCopy(q1, boneQuaternion[i]);
        }
        float Matrix[3][4];
        QuaternionMatrix(boneQuaternion[i], Matrix);
        const float* Position1 = bm1->Position[PriorAnimationFrame];
        const float* Position2 = bm2->Position[currentAnimationFrame];
        if (i == 0 && bLockPositions)
        {
            Matrix[0][3] = bm2->Position[0][0];
            Matrix[1][3] = bm2->Position[0][1];
            Matrix[2][3] = Position1[2] * s2 + Position2[2] * s1 + m.BodyHeight;
        }
        else
        {
            Matrix[0][3] = Position1[0] * s2 + Position2[0] * s1;
            Matrix[1][3] = Position1[1] * s2 + Position2[1] * s1;
            Matrix[2][3] = Position1[2] * s2 + Position2[2] * s1;
        }
        if (b->Parent == -1)
            R_ConcatTransforms(localParentMatrix, Matrix, BoneMatrix[i]);
        else
            R_ConcatTransforms(BoneMatrix[b->Parent], Matrix, BoneMatrix[i]);
    }
}

struct Pose
{
    float frame;
    float priorFrame;
    unsigned short priorAction;
    unsigned short action;
};

vec3_t TurnedHead = {12.f, -25.f, 0.f};
vec3_t Facing = {0.f, 0.f, 37.f};

void RuleAnimation(SyntheticModel& s, Matrices& out, const Pose& pose)
{
    s.model.CurrentAction = pose.action;
    s.model.Animation(out.bones, pose.frame, pose.priorFrame, pose.priorAction, Facing, TurnedHead, false, true,
                      nullptr, -1, MAX_BONES);
}

bool BitEqual(const Matrices& a, const Matrices& b)
{
    return std::memcmp(a.bones, b.bones, sizeof(float) * 12 * BoneCount) == 0;
}

void CheckClose(const Matrices& a, const Matrices& b, float tolerance)
{
    for (int bone = 0; bone < BoneCount; ++bone)
        for (int row = 0; row < 3; ++row)
            for (int column = 0; column < 4; ++column)
                CHECK(std::fabs(a.bones[bone][row][column] - b.bones[bone][row][column]) <= tolerance);
}
} // namespace

TEST_CASE("a held pose, one key per step and the wrap are MU's bone matrices bit for bit [render][animation_playback_rule]")
{
    SyntheticModel s;
    auto rule = std::make_unique<Matrices>();
    auto mu = std::make_unique<Matrices>();
    const Pose poses[] = {
        {3.f, 3.f, Walk, Walk},      // held, no fraction
        {3.6f, 3.f, Walk, Walk},     // held with a fraction
        {4.35f, 3.f, Walk, Walk},    // one key per step
        {0.5f, 5.f, Walk, Walk},     // the wrap of a repeating action
        {0.75f, 5.f, Run, Run},      // the wrap of a locked action
        {2.2f, 1.f, Run, Run},       // locked root
        {4.99f, 3.f, Die, Die},      // held at its end
        {0.f, 0.f, Walk, Walk},      // a fixed render
        {0.4f, 9.f, Walk, Walk},     // a prior frame beyond the keys
        {0.3f, 0.6f, Walk, Run},     // crossfade from a prior frame below 1
        {0.3f, 3.5f, Stand, Walk},   // crossfade from an action that does not play
    };
    for (const Pose& pose : poses)
    {
        CAPTURE(pose.frame);
        CAPTURE(pose.priorFrame);
        CAPTURE(pose.priorAction);
        CAPTURE(pose.action);
        RuleAnimation(s, *rule, pose);
        MuAnimation(s.model, mu->bones, pose.frame, pose.priorFrame, pose.priorAction, pose.action, Facing,
                    TurnedHead);
        CHECK(BitEqual(*rule, *mu));
    }
}

TEST_CASE("skipped keys show the action sampled continuously [render][animation_playback_rule]")
{
    SyntheticModel s;
    auto skipped = std::make_unique<Matrices>();
    auto continuous = std::make_unique<Matrices>();
    auto mu = std::make_unique<Matrices>();

    // MU's ordinary playback at frame f shows the action sampled at f - 1.
    RuleAnimation(s, *skipped, {4.35f, 2.f, Walk, Walk});
    RuleAnimation(s, *continuous, {4.35f, 3.f, Walk, Walk});
    CheckClose(*skipped, *continuous, 1e-4f);
    MuAnimation(s.model, mu->bones, 4.35f, 2.f, Walk, Walk, Facing, TurnedHead);
    CHECK_FALSE(BitEqual(*skipped, *mu));

    // Across the wrap of a locked action (its key n - 1 is never shown).
    RuleAnimation(s, *skipped, {0.5f, 4.f, Run, Run});
    RuleAnimation(s, *continuous, {0.5f, 5.f, Run, Run});
    CheckClose(*skipped, *continuous, 1e-4f);
}

TEST_CASE("an action change fades from the pose on screen [render][animation_playback_rule]")
{
    SyntheticModel s;
    auto onScreen = std::make_unique<Matrices>();
    auto fade = std::make_unique<Matrices>();
    auto target = std::make_unique<Matrices>();

    // Walk was shown at frame 3.4 (its prior key 2) when the action changed to Die.
    RuleAnimation(s, *onScreen, {3.4f, 2.f, Walk, Walk});
    s.model.CurrentAction = Die;
    s.model.Animation(fade->bones, 0.f, 3.4f, Walk, Facing, TurnedHead, false, true, nullptr, -1, MAX_BONES);
    CheckClose(*fade, *onScreen, 2e-3f);

    // The fade ends on Die's first key.
    s.model.Animation(fade->bones, 0.99999f, 3.4f, Walk, Facing, TurnedHead, false, true, nullptr, -1, MAX_BONES);
    RuleAnimation(s, *target, {0.f, 0.f, Die, Die});
    CheckClose(*fade, *target, 2e-2f);
}

TEST_CASE("a locked root keeps the current action's key 0 in x and y [render][animation_playback_rule]")
{
    SyntheticModel s;
    auto skipped = std::make_unique<Matrices>();
    vec3_t straight = {0.f, 0.f, 0.f};
    s.model.CurrentAction = Run;
    s.model.BodyScale = 1.f;
    Vector(0.f, 0.f, 0.f, s.model.BodyOrigin);
    s.model.Animation(skipped->bones, 4.5f, 1.f, Run, straight, straight, false, true, nullptr, -1, MAX_BONES);
    const float* key0 = s.bones[0].BoneMatrixes[Run].Position[0];
    const float* key3 = s.bones[0].BoneMatrixes[Run].Position[3];
    const float* key4 = s.bones[0].BoneMatrixes[Run].Position[4];
    CHECK(skipped->bones[0][0][3] == doctest::Approx(key0[0]));
    CHECK(skipped->bones[0][1][3] == doctest::Approx(key0[1]));
    CHECK(skipped->bones[0][2][3] == doctest::Approx(key3[2] * 0.5f + key4[2] * 0.5f + s.model.BodyHeight));
}

TEST_CASE("the head angle turns every sampled key [render][animation_playback_rule]")
{
    SyntheticModel s;
    auto skipped = std::make_unique<Matrices>();
    auto continuous = std::make_unique<Matrices>();
    auto straight = std::make_unique<Matrices>();
    RuleAnimation(s, *skipped, {2.5f, 0.f, Walk, Walk});
    RuleAnimation(s, *continuous, {2.5f, 1.f, Walk, Walk});
    CheckClose(*skipped, *continuous, 1e-4f);

    vec3_t ahead = {0.f, 0.f, 0.f};
    s.model.CurrentAction = Walk;
    s.model.Animation(straight->bones, 2.5f, 0.f, Walk, Facing, ahead, false, true, nullptr, -1, MAX_BONES);
    CHECK(std::fabs(straight->bones[HeadBone][0][0] - skipped->bones[HeadBone][0][0]) > 1e-3f);
    // The bones above the head do not turn.
    CHECK(std::memcmp(straight->bones, skipped->bones, sizeof(float) * 12 * HeadBone) == 0);
}
