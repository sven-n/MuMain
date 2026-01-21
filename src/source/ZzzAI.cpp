///////////////////////////////////////////////////////////////////////////////
// AI관련 함수
// 타켓방향으로 방향 틀기, 길찾기, fps구하기 등등
//
// *** 함수 레벨: 2
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ZzzBMD.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzLodTerrain.h"
#include "ZzzAI.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <mutex>

#include "Random.h"
#include "ZzzTexture.h"
#include "ZzzOpenglUtil.h"
#include "ZzzInterface.h"
#include "ZzzEffect.h"
#include "Scenes/SceneCore.h"
#include "DSPlaySound.h"
#include "ZzzPath.h"
#include "CharacterManager.h"
#include "SkillManager.h"

namespace
{
constexpr float FULL_ROTATION_DEGREES = 360.f;
constexpr float HALF_ROTATION_DEGREES = 180.f;
constexpr float RAD_TO_DEG = 180.f / static_cast<float>(Q_PI);
constexpr double MIN_FRAME_TIME_MS = 0.001;
constexpr double MILLISECONDS_IN_SECOND = 1000.0;

float NormalizeAngleDegrees(float angle)
{
    angle = std::fmod(angle, FULL_ROTATION_DEGREES);
    if (angle < 0.f)
    {
        angle += FULL_ROTATION_DEGREES;
    }

    return angle;
}

int NormalizeAngleInt(int angle)
{
    angle %= static_cast<int>(FULL_ROTATION_DEGREES);
    if (angle < 0)
    {
        angle += static_cast<int>(FULL_ROTATION_DEGREES);
    }
    return angle;
}

float SignedAngleDelta(float from, float to)
{
    float delta = NormalizeAngleDegrees(to) - NormalizeAngleDegrees(from);
    if (delta > HALF_ROTATION_DEGREES)
    {
        delta -= FULL_ROTATION_DEGREES;
    }
    else if (delta < -HALF_ROTATION_DEGREES)
    {
        delta += FULL_ROTATION_DEGREES;
    }
    return delta;
}

float StepTowardsAngle(float current, float target, float maxDelta)
{
    const float clampedDelta = std::clamp(SignedAngleDelta(current, target), -maxDelta, maxDelta);
    return NormalizeAngleDegrees(current + clampedDelta);
}
} // namespace

float CreateAngle2D(const vec3_t from, const vec2_t to)
{
    return CreateAngle(from[0], from[1], to[0], to[1]);
}

float CreateAngle(float x1, float y1, float x2, float y2)
{
    const float dx = x2 - x1;
    const float dy = y2 - y1;

    if (std::fabs(dx) < std::numeric_limits<float>::epsilon() && std::fabs(dy) < std::numeric_limits<float>::epsilon())
    {
        return 0.f;
    }

    const float angle = std::atan2(dx, -dy) * RAD_TO_DEG;
    return NormalizeAngleDegrees(angle);
}

int TurnAngle(int iTheta, int iHeading, int maxTURN)
{
    if (maxTURN <= 0)
    {
        return NormalizeAngleInt(iTheta);
    }

    const float updated = StepTowardsAngle(static_cast<float>(iTheta), static_cast<float>(iHeading), static_cast<float>(maxTURN));
    return NormalizeAngleInt(static_cast<int>(std::lround(updated)));
}

float TurnAngle2(float angle, float a, float d)
{
    if (d <= 0.f)
    {
        return NormalizeAngleDegrees(angle);
    }

    return StepTowardsAngle(angle, a, d);
}

float FarAngle(float angle1, float angle2, bool absolute)
{
    const float delta = SignedAngleDelta(angle1, angle2);
    return absolute ? std::fabs(delta) : delta;
}

int CalcAngle(float PositionX, float PositionY, float TargetX, float TargetY)
{
    const float targetAngle = CreateAngle(PositionX, PositionY, TargetX, TargetY);
    return NormalizeAngleInt(static_cast<int>(std::lround(targetAngle)));
}

float MoveHumming(vec3_t Position, vec3_t Angle, vec3_t TargetPosition, float Turn)
{
    float scaledTurn = Turn * FPS_ANIMATION_FACTOR;
    float targetAngle = CreateAngle2D(Position, TargetPosition);
    Angle[2] = TurnAngle2(Angle[2], targetAngle, scaledTurn);
    vec3_t Range;
    VectorSubtract(Position, TargetPosition, Range);
    float distance = std::sqrt(Range[0] * Range[0] + Range[1] * Range[1]);
    targetAngle = 360.f - CreateAngle(Position[2], distance, TargetPosition[2], 0.f);
    Angle[0] = TurnAngle2(Angle[0], targetAngle, scaledTurn);
    return VectorLength(Range);
}

void MovePosition(vec3_t Position, vec3_t Angle, vec3_t Speed)
{
    float Matrix[3][4];
    AngleMatrix(Angle, Matrix);

    vec3_t Velocity;
    VectorRotate(Speed, Matrix, Velocity);
    VectorScale(Velocity, FPS_ANIMATION_FACTOR, Velocity);
    VectorAdd(Position, Velocity, Position);
}

std::uint8_t CalcTargetPos(float x, float y, int Tx, int Ty)
{
    const int PositionX = static_cast<int>(x / TERRAIN_SCALE);
    const int PositionY = static_cast<int>(y / TERRAIN_SCALE);
    const int TargetX = static_cast<int>(Tx / TERRAIN_SCALE);
    const int TargetY = static_cast<int>(Ty / TERRAIN_SCALE);

    const std::uint8_t dx = static_cast<std::uint8_t>(8 + TargetX - PositionX);
    const std::uint8_t dy = static_cast<std::uint8_t>(8 + TargetY - PositionY);

    return static_cast<std::uint8_t>((dx & 0x0F) | ((dy & 0x0F) << 4));
}

void Alpha(OBJECT* o)
{
    if (o->AlphaEnable)
    {
        if (o->AlphaTarget > o->Alpha)
        {
            o->Alpha += 0.05f;
            if (o->Alpha > 1.f) o->Alpha = 1.f;
        }
        else if (o->AlphaTarget < o->Alpha)
        {
            o->Alpha -= 0.05f;
            if (o->Alpha < 0.f) o->Alpha = 0.f;
        }
    }
    else
        o->Alpha += (o->AlphaTarget - o->Alpha) * 0.1f;
    if (o->BlendMeshLight > o->Alpha)
        o->BlendMeshLight = o->Alpha;
}

void MoveBoid(OBJECT* o, int i, OBJECT* Boids, int MAX)
{
    int NumBirds = 0;
    float TargetX = 0.f;
    float TargetY = 0.f;
    for (int j = 0; j < MAX; j++)
    {
        OBJECT* t = &Boids[j];
        if (t->Live && j != i)
        {
            vec3_t Range;
            VectorSubtract(o->Position, t->Position, Range);
            const auto distance = VectorLength(Range);
            if (distance < 400.f)
            {
                float xdist = t->Direction[0] - t->Position[0];
                float ydist = t->Direction[1] - t->Position[1];
                if (distance < 80.f)
                {
                    xdist -= t->Direction[0] - o->Position[0];
                    ydist -= t->Direction[1] - o->Position[1];
                }
                else
                {
                    xdist += t->Direction[0] - o->Position[0];
                    ydist += t->Direction[1] - o->Position[1];
                }

                xdist *= FPS_ANIMATION_FACTOR;
                ydist *= FPS_ANIMATION_FACTOR;
                float pdist = std::sqrt(xdist * xdist + ydist * ydist);
                TargetX += xdist / pdist;
                TargetY += ydist / pdist;
                NumBirds++;
            }
        }
    }
    if (NumBirds > 0)
    {
        TargetX = o->Position[0] + TargetX / NumBirds;
        TargetY = o->Position[1] + TargetY / NumBirds;

        o->Angle[2] = (float)TurnAngle((int)o->Angle[2], CalcAngle(o->Position[0], o->Position[1], TargetX, TargetY), (int)o->Gravity);
    }
}

void PushObject(vec3_t PushPosition, vec3_t Position, float Power, vec3_t Angle)
{
    Vector(0.f, 0.f, 0.f, Angle);
    Angle[2] = CreateAngle2D(PushPosition, Position) + 180.f;
    if (Angle[2] >= 360.f) Angle[2] -= 360.f;

    float Matrix[3][4];
    AngleMatrix(Angle, Matrix);
    vec3_t p1, p2;
    Vector(0.f, -Power, 0.f, p1);
    VectorRotate(p1, Matrix, p2);
    Position[2] = RequestTerrainHeight(Position[0], Position[1]);
}

void SetAction_Fenrir_Skill(CHARACTER* c, OBJECT* o)
{
    if (gCharacterManager.GetBaseClass(c->Class) == CLASS_RAGEFIGHTER)
    {
        if (c->Weapon[0].Type != -1 && c->Weapon[1].Type != -1)
            SetAction(&c->Object, PLAYER_RAGE_FENRIR_TWO_SWORD);
        else if (c->Weapon[0].Type != -1 && c->Weapon[1].Type == -1)
            SetAction(&c->Object, PLAYER_RAGE_FENRIR_ONE_RIGHT);
        else if (c->Weapon[0].Type == -1 && c->Weapon[1].Type != -1)
            SetAction(&c->Object, PLAYER_RAGE_FENRIR_ONE_LEFT);
        else
            SetAction(&c->Object, PLAYER_RAGE_FENRIR);
    }
    else
    {
        if (c->Weapon[0].Type != -1 && c->Weapon[1].Type != -1)
            SetAction(o, PLAYER_FENRIR_SKILL_TWO_SWORD);
        else if (c->Weapon[0].Type != -1 && c->Weapon[1].Type == -1)
            SetAction(o, PLAYER_FENRIR_SKILL_ONE_RIGHT);
        else if (c->Weapon[0].Type == -1 && c->Weapon[1].Type != -1)
            SetAction(o, PLAYER_FENRIR_SKILL_ONE_LEFT);
        else
            SetAction(o, PLAYER_FENRIR_SKILL);
    }
}

void SetAction_Fenrir_Damage(CHARACTER* c, OBJECT* o)
{
    if (gCharacterManager.GetBaseClass(c->Class) == CLASS_RAGEFIGHTER)
    {
        if (c->Weapon[0].Type != -1 && c->Weapon[1].Type != -1)
            SetAction(o, PLAYER_RAGE_FENRIR_DAMAGE_TWO_SWORD);
        else if (c->Weapon[0].Type != -1 && c->Weapon[1].Type == -1)
            SetAction(o, PLAYER_RAGE_FENRIR_DAMAGE_ONE_RIGHT);
        else if (c->Weapon[0].Type == -1 && c->Weapon[1].Type != -1)
            SetAction(o, PLAYER_RAGE_FENRIR_DAMAGE_ONE_LEFT);
        else
            SetAction(o, PLAYER_RAGE_FENRIR_DAMAGE);
    }
    else
    {
        if (c->Weapon[0].Type != -1 && c->Weapon[1].Type != -1)
            SetAction(o, PLAYER_FENRIR_DAMAGE_TWO_SWORD);
        else if (c->Weapon[0].Type != -1 && c->Weapon[1].Type == -1)
            SetAction(o, PLAYER_FENRIR_DAMAGE_ONE_RIGHT);
        else if (c->Weapon[0].Type != -1 && c->Weapon[1].Type != -1 && c->Weapon[1].Type == MODEL_BOLT)
            SetAction(o, PLAYER_FENRIR_DAMAGE_ONE_RIGHT);
        else if (c->Weapon[0].Type == -1 && c->Weapon[1].Type != -1)
            SetAction(o, PLAYER_FENRIR_DAMAGE_ONE_LEFT);
        else if (c->Weapon[0].Type != -1 && c->Weapon[1].Type != -1 && c->Weapon[0].Type == MODEL_ARROWS)
            SetAction(o, PLAYER_FENRIR_DAMAGE_ONE_LEFT);
        else	// 맨손
            SetAction(o, PLAYER_FENRIR_DAMAGE);
    }
}

void SetAction_Fenrir_Run(CHARACTER* c, OBJECT* o)
{
    if (c->Weapon[0].Type != -1 && c->Weapon[1].Type != -1 && c->Weapon[0].Type != MODEL_ARROWS && c->Weapon[1].Type != MODEL_BOLT)
    {
        if (gCharacterManager.GetBaseClass(c->Class) == CLASS_ELF)
            SetAction(o, PLAYER_FENRIR_RUN_TWO_SWORD_ELF);
        else if (gCharacterManager.GetBaseClass(c->Class) == CLASS_DARK)
            SetAction(o, PLAYER_FENRIR_RUN_TWO_SWORD_MAGOM);
        else if (gCharacterManager.GetBaseClass(c->Class) == CLASS_RAGEFIGHTER)
            SetAction(o, PLAYER_RAGE_FENRIR_RUN_TWO_SWORD);
        else
            SetAction(o, PLAYER_FENRIR_RUN_TWO_SWORD);
    }
    else if (c->Weapon[0].Type != -1 && c->Weapon[1].Type == -1)
    {
        if (gCharacterManager.GetBaseClass(c->Class) == CLASS_ELF)
            SetAction(o, PLAYER_FENRIR_RUN_ONE_RIGHT_ELF);
        else if (gCharacterManager.GetBaseClass(c->Class) == CLASS_DARK)
            SetAction(o, PLAYER_FENRIR_RUN_ONE_RIGHT_MAGOM);
        else if (gCharacterManager.GetBaseClass(c->Class) == CLASS_RAGEFIGHTER)
            SetAction(o, PLAYER_RAGE_FENRIR_RUN_ONE_RIGHT);
        else
            SetAction(o, PLAYER_FENRIR_RUN_ONE_RIGHT);
    }
    else if (c->Weapon[0].Type != -1 && c->Weapon[1].Type != -1 && c->Weapon[1].Type == MODEL_BOLT)
        SetAction(o, PLAYER_FENRIR_RUN_ONE_RIGHT_ELF);
    else if (c->Weapon[0].Type == -1 && c->Weapon[1].Type != -1)
    {
        if (gCharacterManager.GetBaseClass(c->Class) == CLASS_ELF)
            SetAction(o, PLAYER_FENRIR_RUN_ONE_LEFT_ELF);
        else if (gCharacterManager.GetBaseClass(c->Class) == CLASS_DARK)
            SetAction(o, PLAYER_FENRIR_RUN_ONE_LEFT_MAGOM);
        else if (gCharacterManager.GetBaseClass(c->Class) == CLASS_RAGEFIGHTER)
            SetAction(o, PLAYER_RAGE_FENRIR_RUN_ONE_LEFT);
        else
            SetAction(o, PLAYER_FENRIR_RUN_ONE_LEFT);
    }
    else if (c->Weapon[0].Type != -1 && c->Weapon[1].Type != -1 && c->Weapon[0].Type == MODEL_ARROWS)
        SetAction(o, PLAYER_FENRIR_RUN_ONE_LEFT_ELF);
    else
    {
        if (gCharacterManager.GetBaseClass(c->Class) == CLASS_ELF)
            SetAction(o, PLAYER_FENRIR_RUN_ELF);
        else if (gCharacterManager.GetBaseClass(c->Class) == CLASS_DARK)
            SetAction(o, PLAYER_FENRIR_RUN_MAGOM);
        else if (gCharacterManager.GetBaseClass(c->Class) == CLASS_RAGEFIGHTER)
            SetAction(o, PLAYER_RAGE_FENRIR_RUN);
        else
            SetAction(o, PLAYER_FENRIR_RUN);
    }
}

void SetAction_Fenrir_Walk(CHARACTER* c, OBJECT* o)
{
    if (gCharacterManager.GetBaseClass(c->Class) == CLASS_RAGEFIGHTER)
    {
        if (c->Weapon[0].Type != -1 && c->Weapon[1].Type != -1)
            SetAction(o, PLAYER_RAGE_FENRIR_WALK_TWO_SWORD);
        else if (c->Weapon[0].Type != -1 && c->Weapon[1].Type == -1)
            SetAction(o, PLAYER_RAGE_FENRIR_WALK_ONE_RIGHT);
        else if (c->Weapon[0].Type == -1 && c->Weapon[1].Type != -1)
            SetAction(o, PLAYER_RAGE_FENRIR_WALK_ONE_LEFT);
        else
            SetAction(o, PLAYER_RAGE_FENRIR_WALK);
    }
    else
    {
        if (c->Weapon[0].Type != -1 && c->Weapon[1].Type != -1 && c->Weapon[0].Type != MODEL_ARROWS && c->Weapon[1].Type != MODEL_BOLT)
            SetAction(o, PLAYER_FENRIR_WALK_TWO_SWORD);
        else if (c->Weapon[0].Type != -1 && c->Weapon[1].Type == -1)
            SetAction(o, PLAYER_FENRIR_WALK_ONE_RIGHT);
        else if (c->Weapon[0].Type != -1 && c->Weapon[1].Type != -1 && c->Weapon[1].Type == MODEL_BOLT)
            SetAction(o, PLAYER_FENRIR_WALK_ONE_RIGHT);
        else if (c->Weapon[0].Type == -1 && c->Weapon[1].Type != -1)
            SetAction(o, PLAYER_FENRIR_WALK_ONE_LEFT);
        else if (c->Weapon[0].Type != -1 && c->Weapon[1].Type != -1 && c->Weapon[0].Type == MODEL_ARROWS)
            SetAction(o, PLAYER_FENRIR_WALK_ONE_LEFT);
        else
            SetAction(o, PLAYER_FENRIR_WALK);
    }
}

bool IsAliceRideAction_UniDino(unsigned short byAction)
{
    if (
        byAction == PLAYER_SKILL_CHAIN_LIGHTNING_UNI
        || byAction == PLAYER_SKILL_CHAIN_LIGHTNING_DINO
        || byAction == PLAYER_SKILL_LIGHTNING_ORB_UNI
        || byAction == PLAYER_SKILL_LIGHTNING_ORB_DINO
        || byAction == PLAYER_SKILL_DRAIN_LIFE_UNI
        || byAction == PLAYER_SKILL_DRAIN_LIFE_DINO
        )
    {
        return true;
    }

    return false;
}

bool IsAliceRideAction_Fenrir(unsigned short byAction)
{
    if (
        byAction == PLAYER_SKILL_CHAIN_LIGHTNING_FENRIR
        || byAction == PLAYER_SKILL_LIGHTNING_ORB_FENRIR
        || byAction == PLAYER_SKILL_DRAIN_LIFE_FENRIR
        )
    {
        return true;
    }

    return false;
}

void SetAction(OBJECT* o, int Action, bool bBlending)
{
    BMD* b = &Models[o->Type];
    if (Action >= b->NumActions) return;
    if (o->CurrentAction != Action)
    {
        o->PriorAction = o->CurrentAction;
        o->PriorAnimationFrame = o->AnimationFrame;
        o->CurrentAction = Action;
        o->AnimationFrame = 0;
        if (bBlending == false)
        {
            o->PriorAnimationFrame = 0;
        }
    }
}

bool TestDistance(CHARACTER* c, vec3_t TargetPosition, float Range)
{
    vec3_t Range2;
    VectorSubtract(c->Object.Position, TargetPosition, Range2);
    float Distance = Range2[0] * Range2[0] + Range2[1] * Range2[1];
    float ZoneRange = Range;
    if (Distance <= ZoneRange * ZoneRange) return true;
    return false;
}

void LookAtTarget(OBJECT* o, const CHARACTER* targetCharacter)
{
    if (targetCharacter == nullptr)
    {
        return;
    }

    const auto targetObject = &targetCharacter->Object;
    const auto angle = CreateAngle2D(o->Position, targetObject->Position);
    const auto deltaAngle = FarAngle(o->Angle[2], angle);

    if (deltaAngle < 90.f)
    {
        o->HeadTargetAngle[0] = o->Angle[2] - angle;
        o->HeadTargetAngle[1] = (targetObject->Position[2] - (o->Position[2] + 50.f)) * 0.2f;
        if (o->HeadTargetAngle[0] < 0)
        {
            o->HeadTargetAngle[0] += 360.f;
        }

        if (o->HeadTargetAngle[1] < 0)
        {
            o->HeadTargetAngle[1] += 360.f;
        }
    }
    else
    {
        o->HeadTargetAngle[0] = 0.f;
        o->HeadTargetAngle[1] = 0.f;
    }
}

void MoveHead(CHARACTER* c)
{
    OBJECT* o = &c->Object;
    if (o->CurrentAction != MONSTER01_DIE)
    {
        if (o->CurrentAction == MONSTER01_STOP1)
        {
            if (rand_fps_check(32))
            {
                o->HeadTargetAngle[0] = Random::RangeFloat(-64, 63);
                o->HeadTargetAngle[1] = Random::RangeFloat(-16, 31);
                for (int i = 0; i < 2; i++)
                    if (o->HeadTargetAngle[i] < 0) o->HeadTargetAngle[i] += 360.f;
            }
        }
        else
            if (o->CurrentAction == MONSTER01_WALK && c->TargetCharacter != -1)
            {
                LookAtTarget(o, &CharactersClient[c->TargetCharacter]);
            }
            else
            {
                o->HeadTargetAngle[0] = 0.f;
                o->HeadTargetAngle[1] = 0.f;
            }
    }
}

void Damage(vec3_t soPosition, CHARACTER* tc, float AttackRange, int AttackPoint, bool Hit)
{
    return;
    if (tc == NULL) return;

    OBJECT* to = &tc->Object;

    vec3_t Position;
    if (AttackPoint > 0)
    {
        vec3_t Range;
        VectorSubtract(soPosition, to->Position, Range);
        VectorMA(to->Position, 0.3f, Range, Position);
        Position[2] += 80.f;
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);
        for (int i = 0; i < 40; i++)
        {
            CreateParticle(BITMAP_SPARK, Position, to->Angle, Light);
            vec3_t Angle;
            Vector(-Random::RangeFloat(30, 89), 0.f, Random::RangeFloat(0, 359), Angle);
            CreateJoint(BITMAP_JOINT_SPARK, Position, Position, Angle);
        }
        CreateParticle(BITMAP_SPARK + 1, Position, to->Angle, Light);
    }

    //point
    VectorCopy(to->Position, Position);
    Position[2] += 130.f + to->CollisionRange * 0.5f;
    vec3_t Color;
    Vector(1.f, 0.1f, 0.1f, Color);
    CreatePoint(Position, AttackPoint, Color);
}

PATH _path;
PATH* path = &_path;

bool MovePath(CHARACTER* c, bool Turn)
{
    bool Success = false;
    PATH_t* p = &c->Path;

    std::lock_guard<SpinLock> guard(p->Lock);

    if (p->CurrentPath < p->PathNum)
    {
        if (p->CurrentPathFloat == 0)
        {
            int Path = p->CurrentPath + 1;
            if (Path > p->PathNum - 1)
                Path = p->PathNum - 1;
            c->PositionX = p->PathX[Path];
            c->PositionY = p->PathY[Path];
        }
        float x[4], y[4];
        for (int i = 0; i < 4; i++)
        {
            int Path = p->CurrentPath + i - 1;
            if (Path < 0)
                Path = 0;
            else if (Path > p->PathNum - 1)
                Path = p->PathNum - 1;
            x[i] = ((float)p->PathX[Path] + 0.5f) * TERRAIN_SCALE;
            y[i] = ((float)p->PathY[Path] + 0.5f) * TERRAIN_SCALE;
        }
        float cx, cy;
        switch (p->CurrentPathFloat)
        {
        case 0:
            cx = x[0] * -0.0703125f + x[1] * 0.8671875f + x[2] * 0.2265625f + x[3] * -0.0234375f;
            cy = y[0] * -0.0703125f + y[1] * 0.8671875f + y[2] * 0.2265625f + y[3] * -0.0234375f;
            break;
        case 1:
            cx = x[0] * -0.0625f + x[1] * 0.5625f + x[2] * 0.5625f + x[3] * -0.0625f;
            cy = y[0] * -0.0625f + y[1] * 0.5625f + y[2] * 0.5625f + y[3] * -0.0625f;
            break;
        case 2:
            cx = x[0] * -0.0234375f + x[1] * 0.2265625f + x[2] * 0.8671875f + x[3] * -0.0703125f;
            cy = y[0] * -0.0234375f + y[1] * 0.2265625f + y[2] * 0.8671875f + y[3] * -0.0703125f;
            break;
        case 3:
            cx = x[2];
            cy = y[2];
            break;
        }
        OBJECT* o = &c->Object;
        float dx = o->Position[0] - cx;
        float dy = o->Position[1] - cy;
        float Distance = std::sqrt(dx * dx + dy * dy);
        if (Distance <= 20.f)
        {
            p->CurrentPathFloat++;
            if (p->CurrentPathFloat > 3)
            {
                p->CurrentPathFloat = 0;
                p->CurrentPath++;
                if (p->CurrentPath >= p->PathNum - 1)
                {
                    p->CurrentPath = p->PathNum - 1;
                    c->PositionX = p->PathX[p->CurrentPath];
                    c->PositionY = p->PathY[p->CurrentPath];
                    o->Position[0] = cx;
                    o->Position[1] = cy;
                    Success = true;
                }
            }
        }
        if (!Success && Turn)
        {
            float Angle = CreateAngle(o->Position[0], o->Position[1], cx, cy);
            float TargetAngle = FarAngle(o->Angle[2], Angle);
            if (TargetAngle >= 45.f)
            {
                o->Angle[2] = Angle;
            }
            else
            {
                o->Angle[2] = TurnAngle2(o->Angle[2], Angle, TargetAngle * 0.5f);
            }
        }
    }

    return Success;
}

void InitPath()
{
    path->SetMapDimensions(256, 256, TerrainWall);
}

bool PathFinding2(int sx, int sy, int tx, int ty, PATH_t* a, float fDistance, int iDefaultWall)
{
    std::lock_guard<SpinLock> guard(a->Lock);

    bool Success = false;
    bool Value = false;

    if (M34CryWolf1st::Get_State_Only_Elf() == true && M34CryWolf1st::IsCyrWolf1st() == true)
    {
        if ((CharactersClient[TargetNpc].Object.Type >= MODEL_CRYWOLF_ALTAR1 && CharactersClient[TargetNpc].Object.Type <= MODEL_CRYWOLF_ALTAR5))
        {
            Value = true;
        }
    }

    int Wall = iDefaultWall;

    bool PathFound = path->FindPath(sx, sy, tx, ty, true, Wall, Value, fDistance);
    if (!PathFound)
    {
        if (((TerrainWall[TERRAIN_INDEX_REPEAT(sx, sy)] & TW_SAFEZONE) == TW_SAFEZONE || (TerrainWall[TERRAIN_INDEX_REPEAT(tx, ty)] & TW_SAFEZONE) == TW_SAFEZONE) && (TerrainWall[TERRAIN_INDEX_REPEAT(tx, ty)] & TW_CHARACTER) != TW_CHARACTER)
        {
            Wall = TW_NOMOVE;
        }

        PathFound = path->FindPath(sx, sy, tx, ty, false, Wall, Value, fDistance);
    }

    if (PathFound)
    {
        int PathNum = path->GetPath();
        if (PathNum > 1)
        {
            a->PathNum = PathNum;
            unsigned char* x = path->GetPathX();
            unsigned char* y = path->GetPathY();

            for (int i = 0; i < a->PathNum; i++)
            {
                a->PathX[i] = x[i];
                a->PathY[i] = y[i];
            }

            a->CurrentPath = 0;
            a->CurrentPathFloat = 0;

            Success = true;
        }
    }

    return Success;
}

CTimer* g_WorldTime = new CTimer();

double   FPS;

/**
 * \brief A factor which should applied to all values which get an added offset, frame-by-frame.
 * E.g. you have an object which moves by 10 x positions at every frame on a 25fps basis,
 * you'll simply multiply this 10 positions with this factor. If you have a current
 * frame rate of 50 fps, this factor is 0.5f, so it moves just 5 positions in this frame.
 * Therefore, the speed of the game is maintained even when the FPS change dynamically.
 */
float   FPS_ANIMATION_FACTOR;
double   FPS_AVG;
double   WorldTime = 0.0;

bool rand_fps_check(int reference_frames)
{
    return Random::FpsCheck(reference_frames, static_cast<double>(FPS_ANIMATION_FACTOR));
}

void CalcFPS()
{
    static int timeinit = 0;
    static double start, last;
    static int frame = 0;
    if (!timeinit)
    {
        start = g_WorldTime->GetTimeElapsed();
        timeinit = 1;
    }

    frame++;
    WorldTime = g_WorldTime->GetTimeElapsed();

    const double differenceMs = std::max<double>(WorldTime - last, MIN_FRAME_TIME_MS);
    FPS = MILLISECONDS_IN_SECOND / differenceMs;

    // animate with no less than REFERENCE_FPS, otherwise some animations don't work correctly
    const double fpsRatio = (FPS <= 0.0) ? 0.0 : REFERENCE_FPS / FPS;
    FPS_ANIMATION_FACTOR = std::clamp(static_cast<float>(fpsRatio), 0.f, 1.f);

    // Calculate average fps every 2 seconds or 25 frames
    const double diffSinceStart = WorldTime - start;
    if (diffSinceStart > 2000.0 || frame > 25)
    {
        FPS_AVG = (MILLISECONDS_IN_SECOND * frame) / diffSinceStart;
        start = WorldTime;
        frame = 0;
    }

    last = WorldTime;

    if (SceneFlag == MAIN_SCENE)
    {
        gSkillManager.CalcSkillDelay(static_cast<int>(differenceMs));
    }
}