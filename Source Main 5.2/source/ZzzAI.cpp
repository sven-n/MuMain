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

#include <random>

#include "ZzzTexture.h"
#include "ZzzOpenglUtil.h"
#include "ZzzInterface.h"
#include "ZzzEffect.h"
#include "zzzScene.h"
#include "DSPlaySound.h"
#include "ZzzPath.h"
#include "CharacterManager.h"
#include "SkillManager.h"

float CreateAngle2D(const vec3_t from, const vec2_t to)
{
    return CreateAngle(from[0], from[1], to[0], to[1]);
}

float CreateAngle(float x1, float y1, float x2, float y2)
{
    float nx2 = x2 - x1, ny2 = y2 - y1;
    float r, angle;
    if (absf(nx2) < 0.0001f)
    {
        if (ny2 < 0.f) return 0.f;   //s
        else        return 180.f; //n
    }
    else if (absf(ny2) < 0.0001f)
    {
        if (nx2 < 0.f) return 270.f; //e
        else        return 90.f;  //w
    }
    else
    {
        angle = (float)atan(ny2 / nx2) / Q_PI * 180.f + 90.f;
        if (nx2 < 0.f && ny2 >= 0.f) r = angle + 180.f;
        else if (nx2 < 0.f && ny2 < 0.f) r = angle + 180.f;
        else if (nx2 >= 0.f && ny2 < 0.f) r = angle;
        else                          r = angle;
    }
    if(r<0.f) r+=360.f;else if(r>=360.f) r-=360.f;
    return r;
}

int TurnAngle(int iTheta, int iHeading, int maxTURN)
{
    int iChange = 0;

    int Delta = abs(iTheta - iHeading);
    if (iTheta > iHeading)
    {
        if (Delta < abs((iHeading + 360) - iTheta)) iChange = -min(maxTURN, Delta);
        else iChange = min(maxTURN, Delta);
    }
    if (iTheta < iHeading)
    {
        if (Delta < abs((iTheta + 360) - iHeading)) iChange = min(maxTURN, Delta);
        else iChange = -min(maxTURN, Delta);
    }
    iTheta += iChange + 360;
    iTheta %= 360;
    return iTheta;
}

float TurnAngle2(float angle, float a, float d)
{
    if (angle < 0.f) angle += 360.f;
    if (a < 0.f) a += 360.f;
    float aa;
    if (angle < 180.f)
    {
        aa = angle - d;
        if (a >= angle + d && (a < angle + 180.f)) angle += d;
        else if (aa >= 0 && (a >= angle + 180.f || a < aa)) angle -= d;
        else if (aa < 0 && (a >= angle + 180.f && a < aa + 360.f)) { angle = angle - d + 360.f; }
        else                                                  angle = a;
    }
    else
    {
        aa = angle + d;
        if (a < angle - d && (a >= angle - 180.f)) angle -= d;
        else if (aa < 360.f && (a < angle - 180.f || a >= aa)) angle += d;
        else if (aa >= 360.f && (a < angle - 180.f && a >= aa - 360.f)) { angle = angle + d - 360.f; }
        else                                                  angle = a;
    }
    return angle;
}

float FarAngle(float angle1, float angle2, bool abs)
{
    if (angle1 < 0.f) angle1 += 360.f;
    if (angle2 < 0.f) angle2 += 360.f;
    float d = angle2 - angle1;
    if (angle1 < 180.f)
    {
        if (angle2 >= angle1 + 180.f) d = (360.f - angle2 + angle1);
    }
    else
    {
        if (angle2 < angle1 - 180.f) d = (360.f - angle1 + angle2);
    }
    if (abs == true) if (d < 0.f) d = -d;
    return d;
}

int CalcAngle(float PositionX, float PositionY, float TargetX, float TargetY)
{
    float Grad;
    if (TargetX - PositionX == 0.f)
        Grad = 0.f;
    else
        Grad = (PositionY - TargetY) / (TargetX - PositionX);
    int TargetTheta = (int)(atanf(Grad) * (180.f / Q_PI));
    if (TargetX < PositionX) TargetTheta += 180;
    if (TargetTheta < 0) TargetTheta += 360;
    TargetTheta = 360 - TargetTheta;
    TargetTheta %= 360;
    return TargetTheta;
}

float MoveHumming(vec3_t Position, vec3_t Angle, vec3_t TargetPosition, float Turn)
{
    float scaledTurn = Turn * FPS_ANIMATION_FACTOR;
    float targetAngle = CreateAngle2D(Position, TargetPosition);
    Angle[2] = TurnAngle2(Angle[2], targetAngle, scaledTurn);
    vec3_t Range;
    VectorSubtract(Position, TargetPosition, Range);
    float distance = sqrtf(Range[0] * Range[0] + Range[1] * Range[1]);
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
    VectorScale(Velocity, FPS_ANIMATION_FACTOR, Velocity)
    VectorAdd(Position, Velocity, Position);
}

BYTE CalcTargetPos(float x, float y, int Tx, int Ty)
{
    BYTE position;
    int PositionX = (int)(x / TERRAIN_SCALE);
    int PositionY = (int)(y / TERRAIN_SCALE);
    int TargetX = (int)(Tx / TERRAIN_SCALE);
    int TargetY = (int)(Ty / TERRAIN_SCALE);

    BYTE dx = 8 + TargetX - PositionX;
    BYTE dy = 8 + TargetY - PositionY;

    position = ((BYTE)((BYTE)dx | (BYTE)(dy << 4)));

    return position;
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
                float pdist = sqrtf(xdist * xdist + ydist * ydist);
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
                o->HeadTargetAngle[0] = (float)(rand() % 128 - 64);
                o->HeadTargetAngle[1] = (float)(rand() % 48 - 16);
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
            Vector(-(float)(rand() % 60 + 30), 0.f, (float)(rand() % 360), Angle);
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

    p->Lock.lock();

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
        float Distance = sqrtf(dx * dx + dy * dy);
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

    p->Lock.unlock();

    return Success;
}

void InitPath()
{
    path->SetMapDimensions(256, 256, TerrainWall);
}

bool PathFinding2(int sx, int sy, int tx, int ty, PATH_t* a, float fDistance, int iDefaultWall)
{
    a->Lock.lock();

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

    a->Lock.unlock();

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

std::random_device rd;  // a seed source for the random number engine
std::mt19937 gen(rd()); // mersenne_twister_engine seeded with rd()
std::uniform_real_distribution<> distrib(0.0, 1.0);

bool rand_fps_check(int reference_frames)
{
    // return rand() % reference_frames == 0;
    const auto animation_factor = min(1.0, static_cast<double>(FPS_ANIMATION_FACTOR));
    const auto rand_value = distrib(gen);// *1.5;
    const auto chance = reference_frames == 1
        ? animation_factor
        : (1.0 / reference_frames) * animation_factor;

    return rand_value <= chance;
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

    const double differenceMs = WorldTime - last;
    if (differenceMs <= 0)
    {
        FPS = 0.01;
    }
    else
    {
        FPS = 1000 / differenceMs;
    }

    // animate with no less than 25 fps, otherwise some animations don't work correctly
    FPS_ANIMATION_FACTOR = minf(static_cast<float>(REFERENCE_FPS / FPS), 1.f);

    // Calculate average fps every 2 seconds or 25 frames
    const double diffSinceStart = WorldTime - start;
    if (diffSinceStart > 2000.0 || frame > 25)
    {
        FPS_AVG = (1000 * frame) / diffSinceStart;
        start = WorldTime;
        frame = 0;
    }

    last = WorldTime;

    if (SceneFlag == MAIN_SCENE)
    {
        gSkillManager.CalcSkillDelay(static_cast<int>(differenceMs));
    }
}