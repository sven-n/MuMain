//////////////////////////////////////////////////////////////////////////
//
//  GOBoid.cpp
//
//////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "zzzInfomation.h"
#include "ZzzOpenglUtil.h"
#include "zzzBmd.h"
#include "GOBoid.h"
#include "zzzCharacter.h"
#include "ZzzLodTerrain.h"
#include "ZzzTexture.h"
#include "ZzzAI.h"
#include "ZzzEffect.h"
#include "zzzOpenData.h"
#include "CSChaosCastle.h"
#include "CDirection.h"
#include "DSPlaySound.h"
#include "MapManager.h"
#include "CameraMove.h"
#include "NewUISystem.h"

int EnableEvent = 0;

static  const   BYTE    BOID_FLY = 0;
static  const   BYTE    BOID_DOWN = 1;
static  const   BYTE    BOID_GROUND = 2;
static  const   BYTE    BOID_UP = 3;

void DeleteMount(OBJECT* Owner)
{
    for (int i = 0; i < MAX_MOUNTS; i++)
    {
        OBJECT* o = &Mounts[i];
        if (o->Live)
        {
            if (o->Owner == Owner)
                o->Live = false;
        }
    }
}

bool IsMount(ITEM* pItem)
{
    if (pItem == NULL)
    {
        return false;
    }

    if (pItem->Type == ITEM_GUARDIAN_ANGEL
        || pItem->Type == ITEM_IMP
        || pItem->Type == ITEM_HORN_OF_UNIRIA
        || pItem->Type == ITEM_HORN_OF_DINORANT
        || pItem->Type == ITEM_DARK_HORSE_ITEM
        || pItem->Type == ITEM_DARK_RAVEN_ITEM
        || pItem->Type == ITEM_HORN_OF_FENRIR
        )
    {
        return true;
    }

    return false;
}

bool CreateMountSub(int Type, vec3_t Position, OBJECT* Owner, OBJECT* o, int SubType, int LinkBone)
{
    if (gMapManager.InChaosCastle() == true)
    {
        return false;
    }

    if (!o->Live)
    {
        o->Type = Type;
        o->Live = true;
        o->Visible = false;
        o->LightEnable = true;
        o->ContrastEnable = false;
        o->AlphaEnable = false;
        o->EnableBoneMatrix = false;
        o->Owner = Owner;
        o->SubType = SubType;
        o->HiddenMesh = -1;
        o->BlendMesh = -1;
        o->BlendMeshLight = 1.f;
        o->Scale = 0.7f;
        o->LifeTime = 30;
        o->Alpha = 0.f;
        o->AlphaTarget = 1.f;
        VectorCopy(Position, o->Position);
        VectorCopy(Owner->Angle, o->Angle);
        Vector(3.f, 3.f, 3.f, o->Light);

        //int AnimationFrame = Models[o->Type].NumAnimationKeys[Models[o->Type].CurrentAction];
        o->PriorAnimationFrame = 0.f;
        o->AnimationFrame = 0.f;
        o->Velocity = 0.5f;
        switch (o->Type)
        {
        case MODEL_FENRIR_BLACK:
        case MODEL_FENRIR_BLUE:
        case MODEL_FENRIR_RED:
        case MODEL_FENRIR_GOLD:
            o->Scale = 0.9f;
            break;
        case MODEL_DARK_HORSE:
            o->Scale = 1.f;
            break;
        case MODEL_PEGASUS:
        case MODEL_UNICON:
            o->Scale = 0.9f;
            break;
        case MODEL_HELPER:
            o->BlendMesh = 1;
            Vector(Owner->Position[0] + (float)(rand() % 512 - 256),
                Owner->Position[1] + (float)(rand() % 512 - 256),
                Owner->Position[2] + (float)(rand() % 128 + 128), o->Position);
            break;
        case MODEL_IMP:
            Vector(Owner->Position[0] + (float)(rand() % 128 - 64),
                Owner->Position[1] + (float)(rand() % 128 - 64),
                Owner->Position[2], o->Position);
            o->Position[2] = RequestTerrainHeight(o->Position[0], o->Position[1]) + (float)(rand() % 100);
            break;
        }

        return FALSE;
    }

    return TRUE;
}

void CreateMount(int Type, vec3_t Position, OBJECT* Owner, int SubType, int LinkBone)
{
    if (gMapManager.InChaosCastle() == true) return;

    if (Owner->Type != MODEL_PLAYER && Type != MODEL_HELPER)
        return;

    for (int i = 0; i < MAX_MOUNTS; i++)
    {
        OBJECT* o = &Mounts[i];
        if (CreateMountSub(Type, Position, Owner, o, SubType, LinkBone) == FALSE)
        {
            // False means, it has been successful ...
            return;
        }
    }
}

bool MoveMount(OBJECT* o, bool bForceRender)
{
    if (o->Live)
    {
        if (SceneFlag == MAIN_SCENE)
        {
            if (gMapManager.WorldActive >= WD_65DOPPLEGANGER1 && gMapManager.WorldActive <= WD_68DOPPLEGANGER4);
            else
                if (!o->Owner->Live || o->Owner->Kind != KIND_PLAYER)
                {
                    o->Live = false;
                    return TRUE;
                }
        }

        Alpha(o);

        int     j;
        float   FlyRange = 0.0f;
        vec3_t  Light, Position;
        vec3_t  TargetPosition;
        BMD* b = &Models[o->Type];

        VectorCopy(o->Owner->Position, TargetPosition);
        switch (o->Type)
        {
        case MODEL_FENRIR_BLACK:
        case MODEL_FENRIR_BLUE:
        case MODEL_FENRIR_RED:
        case MODEL_FENRIR_GOLD:
            if ((TerrainWall[TERRAIN_INDEX_REPEAT((int)(o->Owner->Position[0] / TERRAIN_SCALE), (int)(o->Owner->Position[1] / TERRAIN_SCALE))] & TW_SAFEZONE) == TW_SAFEZONE && bForceRender == FALSE)
            {
                o->Alpha = 0.f;
                break;
            }

            if (o->Owner->Teleport == TELEPORT_BEGIN || o->Owner->Teleport == TELEPORT)
            {
                o->Alpha -= 0.1f;
                if (o->Alpha < 0) o->Alpha = 0.f;
            }
            else
            {
                o->Alpha = 1.f;
            }

            VectorCopy(o->Owner->HeadAngle, o->HeadAngle);
            VectorCopy(o->Owner->Position, o->Position);
            VectorCopy(o->Owner->Angle, o->Angle);

            if ((o->Owner->CurrentAction >= PLAYER_FENRIR_ATTACK && o->Owner->CurrentAction <= PLAYER_FENRIR_ATTACK_BOW)
                || IsAliceRideAction_Fenrir(o->Owner->CurrentAction) == true
                || o->Owner->CurrentAction == PLAYER_RAGE_FENRIR_ATTACK_RIGHT)
            {
                SetAction(o, FENRIR_ATTACK);
                o->Velocity = 0.4f;
            }
            else if (o->Owner->CurrentAction >= PLAYER_FENRIR_SKILL && o->Owner->CurrentAction <= PLAYER_FENRIR_SKILL_ONE_LEFT)
            {
                SetAction(o, FENRIR_ATTACK_SKILL);
                o->Velocity = 0.4f;
            }
            else if (o->Owner->CurrentAction >= PLAYER_FENRIR_DAMAGE && o->Owner->CurrentAction <= PLAYER_FENRIR_DAMAGE_ONE_LEFT)
            {
                SetAction(o, FENRIR_DAMAGE);
                o->Velocity = 0.4f;
            }
            else if (o->Owner->CurrentAction >= PLAYER_FENRIR_STAND && o->Owner->CurrentAction <= PLAYER_FENRIR_STAND_ONE_LEFT)
            {
                SetAction(o, FENRIR_STAND);
                o->Velocity = 0.4f;
            }
            else if (o->Owner->CurrentAction == PLAYER_DIE1)
            {
                SetAction(o, FENRIR_STAND);
                o->Velocity = 0.4f;
            }
            else if (o->Owner->CurrentAction >= PLAYER_RAGE_FENRIR_DAMAGE && o->Owner->CurrentAction <= PLAYER_RAGE_FENRIR_DAMAGE_ONE_LEFT)
            {
                SetAction(o, FENRIR_DAMAGE);
                o->Velocity = 0.4f;
            }
            else if (o->Owner->CurrentAction >= PLAYER_RAGE_FENRIR && o->Owner->CurrentAction <= PLAYER_RAGE_FENRIR_ONE_LEFT)
            {
                SetAction(o, FENRIR_ATTACK_SKILL);
                o->Velocity = 0.4f;
            }
            else if (o->Owner->CurrentAction >= PLAYER_RAGE_FENRIR_STAND && o->Owner->CurrentAction <= PLAYER_RAGE_FENRIR_STAND_ONE_LEFT)
            {
                SetAction(o, FENRIR_STAND);
                o->Velocity = 0.4f;
            }
            else if (o->Owner->CurrentAction >= PLAYER_SKILL_THRUST && o->Owner->CurrentAction <= PLAYER_SKILL_HP_UP_OURFORCES)
            {
                SetAction(o, FENRIR_STAND);
                o->Velocity = 0.4f;
            }
            else
            {
                if (o->Owner->CurrentAction >= PLAYER_FENRIR_WALK && o->Owner->CurrentAction <= PLAYER_FENRIR_WALK_ONE_LEFT)
                {
                    SetAction(o, FENRIR_WALK);
                    o->Velocity = 1.0f;
                }
                else if (o->Owner->CurrentAction >= PLAYER_FENRIR_RUN && o->Owner->CurrentAction <= PLAYER_FENRIR_RUN_ONE_LEFT_ELF)
                {
                    SetAction(o, FENRIR_RUN);
                    o->Velocity = 0.6f;
                }
                else if (o->Owner->CurrentAction >= PLAYER_RAGE_FENRIR_RUN && o->Owner->CurrentAction <= PLAYER_RAGE_FENRIR_RUN_ONE_LEFT)
                {
                    SetAction(o, FENRIR_RUN);
                    o->Velocity = 0.6f;
                }
                else if (o->Owner->CurrentAction >= PLAYER_RAGE_FENRIR_WALK && o->Owner->CurrentAction <= PLAYER_RAGE_FENRIR_WALK_TWO_SWORD)
                {
                    SetAction(o, FENRIR_WALK);
                    o->Velocity = 1.0f;
                }

                Vector(1.f, 1.f, 1.f, Light);
                if (gMapManager.WorldActive == WD_10HEAVEN)
                {
                    bool   bWave = false;
                    vec3_t p = { 120.f, 0.f, 32.f };

                    if (o->AnimationFrame > 1.f && o->AnimationFrame < 1.2f)
                    {
                        b->TransformPosition(BoneTransform[22], p, Position);	// R Hand
                        Position[0] += 40.f;
                        bWave = true;
                    }
                    else if (o->AnimationFrame > 4.8f && o->AnimationFrame <= 5.0f)
                    {
                        b->TransformPosition(BoneTransform[44], p, Position);	// R Foot
                        Position[0] += 40.f;
                        Position[2] += 700.f;
                        bWave = true;
                    }

                    if (bWave && rand_fps_check(1))
                    {
                        Vector(Position[0], Position[1], Position[2], p);
                        CreateEffect(BITMAP_SHOCK_WAVE, p, o->Angle, Light, 1);

                        for (int i = 0; i < rand() % 5 + 5; ++i)
                        {
                            Vector(Position[0] + (float)(rand() % 50 - 25),
                                Position[1] + (float)(rand() % 50 - 25),
                                Position[2] + (float)(rand() % 16 - 8) - 10, p);
                            CreateParticle(BITMAP_SMOKE, p, o->Angle, Light);
                        }
                    }
                }
                else if (rand_fps_check(3) && !gMapManager.InHellas())
                {
                    if (o->Owner && !g_isCharacterBuff(o->Owner, eBuff_Cloaking))
                    {
                        Vector(o->Position[0] + (float)(rand() % 64 - 32),
                            o->Position[1] + (float)(rand() % 64 - 32),
                            o->Position[2] + (float)(rand() % 32 - 16), Position);
                        if (gMapManager.WorldActive == WD_2DEVIAS)
                            CreateParticle(BITMAP_SMOKE, Position, o->Angle, Light);
                        else if (gMapManager.WorldActive != WD_10HEAVEN)
                            if (!g_Direction.m_CKanturu.IsMayaScene())
                                CreateParticle(BITMAP_SMOKE + 1, Position, o->Angle, Light);
                    }
                }
            }
            break;
        case MODEL_DARK_HORSE:
            if ((TerrainWall[TERRAIN_INDEX_REPEAT((int)(o->Owner->Position[0] / TERRAIN_SCALE), (int)(o->Owner->Position[1] / TERRAIN_SCALE))] & TW_SAFEZONE) == TW_SAFEZONE
                && bForceRender == FALSE)
            {
                o->Alpha = 0.f;
                break;
            }

            b->BoneHead = 7;

            // Take riders position and angle:
            VectorCopy(o->Owner->HeadAngle, o->HeadAngle);
            VectorCopy(o->Owner->Position, o->Position);
            VectorCopy(o->Owner->Angle, o->Angle);

            if (o->Owner->CurrentAction == PLAYER_ATTACK_DARKHORSE)
            {
                SetAction(o, 3);
                o->Velocity = 0.34f;
            }
            else if (o->Owner->CurrentAction == PLAYER_RUN_RIDE_HORSE)
            {
                Vector(1.f, 1.f, 1.f, Light);

                SetAction(o, 1);
                if (gMapManager.WorldActive == WD_10HEAVEN || g_Direction.m_CKanturu.IsMayaScene())
                {
                    bool   bWave = false;
                    vec3_t p = { 120.f, 0.f, rand() % 64 - 32.f };

                    if (o->AnimationFrame > 1.f && o->AnimationFrame < 1.2f)
                    {
                        b->TransformPosition(BoneTransform[19], p, Position);
                        bWave = true;
                    }
                    else if (o->AnimationFrame > 1.1f && o->AnimationFrame < 1.4f)
                    {
                        b->TransformPosition(BoneTransform[25], p, Position);
                        bWave = true;
                    }
                    else if (o->AnimationFrame > 1.3f && o->AnimationFrame < 1.6f)
                    {
                        b->TransformPosition(BoneTransform[32], p, Position);
                        bWave = true;
                    }
                    else if (o->AnimationFrame > 1.5f && o->AnimationFrame <= 1.8f)
                    {
                        b->TransformPosition(BoneTransform[38], p, Position);
                        bWave = true;
                    }

                    if (bWave)
                    {
                        Vector(Position[0] + (float)(rand() % 16 - 8),
                            Position[1] + (float)(rand() % 16 - 8),
                            Position[2], p);
                        CreateEffect(BITMAP_SHOCK_WAVE, p, o->Angle, Light, 1);

                        for (int i = 0; i < rand() % 5 + 5; ++i)
                        {
                            Vector(Position[0] + (float)(rand() % 50 - 25),
                                Position[1] + (float)(rand() % 50 - 25),
                                Position[2] + (float)(rand() % 16 - 8) - 10, p);
                            CreateParticle(BITMAP_SMOKE, p, o->Angle, Light);
                        }
                    }
                }
                else if (rand_fps_check(2) && !gMapManager.InHellas())
                {
                    if (o->Owner && !g_isCharacterBuff(o->Owner, eBuff_Cloaking))
                    {
                        // Smoke at the back feet of the horse.
                        Vector(o->Position[0] + (float)(rand() % 64 - 32),
                            o->Position[1] + (float)(rand() % 64 - 32),
                            o->Position[2] + (float)(rand() % 32 - 16), Position);

                        if (gMapManager.WorldActive == WD_2DEVIAS)
                            CreateParticle(BITMAP_SMOKE, Position, o->Angle, Light);
                        else if (gMapManager.WorldActive != WD_10HEAVEN && !g_Direction.m_CKanturu.IsMayaScene())
                            CreateParticle(BITMAP_SMOKE + 1, Position, o->Angle, Light);
                    }
                }

                o->Velocity = 0.34f;
            }
            else if (o->Owner->CurrentAction >= PLAYER_ATTACK_RIDE_STRIKE && o->Owner->CurrentAction <= PLAYER_ATTACK_RIDE_ATTACK_MAGIC)
            {
                SetAction(o, 2);
                o->Velocity = 0.34f;
            }
            else if (o->Owner->CurrentAction == PLAYER_IDLE1_DARKHORSE)
            {
                SetAction(o, 5);
                o->Velocity = 1.0f;
            }
            else if (o->Owner->CurrentAction == PLAYER_IDLE2_DARKHORSE)
            {
                SetAction(o, 6);
                o->Velocity = 1.0f;
            }
            else
            {
                SetAction(o, 0);
                o->WeaponLevel = 0;
                o->Velocity = 0.3f;
            }

            // Breathing smoke/bubbles:
            //if (o->CurrentAction != 1)
            {
                if (o->Owner && !g_isCharacterBuff(o->Owner, eBuff_Cloaking))
                {
                    if (rand_fps_check(3))
                    {
                        vec3_t p = { 50.f, -4.f, 0.f };
                        b->TransformPosition(BoneTransform[27], p, Position);

                        if (gMapManager.WorldActive == WD_7ATLANSE || gMapManager.WorldActive == WD_67DOPPLEGANGER3)
                        {
                            CreateParticle(BITMAP_BUBBLE, Position, o->Angle, o->Light);
                        }
                        else
                        {
                            CreateParticle(BITMAP_SMOKE, Position, o->Angle, o->Light);
                        }
                    }
                }
            }

            if (o->CurrentAction == 3)
            {
                vec3_t Pos1, Pos2, p, p2;

                Vector(60.f, 0.f, 0.f, Pos1);
                Vector(0.f, 0.f, 0.f, Pos2);
                Vector(1.f, 1.f, 1.f, Light);
                b->TransformPosition(BoneTransform[9], Pos1, p);
                b->TransformPosition(BoneTransform[9], Pos2, p2);
                CreateBlur(Hero, p, p2, Light, 0);
            }

            if (o->CurrentAction == 3)
            {
                RenderDarkHorseSkill(o, b);
            }

            if (o->Owner->ExtState == 1)
            {
                vec3_t p;
                vec3_t Angle = { 0.f, 0.f, 0.f };
                float  Matrix[3][4];

                Vector(0.f, 50, 0.f, p);
                for (int i = 0; i < 4; ++i)
                {
                    Angle[2] += 90.f;
                    AngleMatrix(Angle, Matrix);
                    VectorRotate(p, Matrix, Position);
                    VectorAdd(Position, o->Position, Position);

                    for (int j = 0; j < 3; ++j)
                    {
                        CreateJoint(BITMAP_FLARE, Position, Position, Angle, 0, o, 40, 2);
                    }
                }
            }
            else if (o->Owner->ExtState == 2)
            {
            }
            o->Owner->ExtState = 0;

            o->Live = o->Owner->Live;
            break;
        case MODEL_PEGASUS:
        case MODEL_UNICON:
            if ((TerrainWall[TERRAIN_INDEX_REPEAT((int)(o->Owner->Position[0] / TERRAIN_SCALE), (int)(o->Owner->Position[1] / TERRAIN_SCALE))] & TW_SAFEZONE) == TW_SAFEZONE
                && bForceRender == FALSE)
            {
                o->Alpha = 0.f;
                break;
            }

            if (o->Owner->Teleport == TELEPORT_BEGIN || o->Owner->Teleport == TELEPORT)
            {
                o->Alpha -= 0.1f;
                if (o->Alpha < 0) o->Alpha = 0.f;
            }
            else
            {
                o->Alpha = 1.f;
            }

            VectorCopy(o->Owner->Position, o->Position);

            if (o->Type == MODEL_PEGASUS)
            {
                if (gMapManager.WorldActive == WD_8TARKAN || gMapManager.WorldActive == WD_10HEAVEN || g_Direction.m_CKanturu.IsMayaScene())
                    o->Position[2] -= 10.f;
                else if (gMapManager.WorldActive != -1)
                    o->Position[2] -= 30.f;
            }
            VectorCopy(o->Owner->Angle, o->Angle);
            if (o->Owner->CurrentAction >= PLAYER_WALK_MALE && o->Owner->CurrentAction <= PLAYER_RUN_RIDE_WEAPON
                || o->Owner->CurrentAction == PLAYER_FLY_RIDE || o->Owner->CurrentAction == PLAYER_FLY_RIDE_WEAPON
                || o->Owner->CurrentAction == PLAYER_RAGE_UNI_RUN || o->Owner->CurrentAction == PLAYER_RAGE_UNI_RUN_ONE_RIGHT)
            {
                //  페가수스.
                if (o->Type == MODEL_PEGASUS)
                {
                    if (gMapManager.WorldActive == WD_8TARKAN || gMapManager.WorldActive == WD_10HEAVEN || g_Direction.m_CKanturu.IsMayaScene())
                        SetAction(o, 3);
                    else
                        SetAction(o, 2);
                }
                else
                {
                    SetAction(o, 2);
                }

                if (rand_fps_check(2) && gMapManager.WorldActive != WD_10HEAVEN)
                {
                    if (!g_Direction.m_CKanturu.IsMayaScene())

                        if (o->Owner && !g_isCharacterBuff(o->Owner, eBuff_Cloaking))
                        {
                            Vector(1.f, 1.f, 1.f, Light);
                            Vector(o->Position[0] + (float)(rand() % 64 - 32),
                                o->Position[1] + (float)(rand() % 64 - 32),
                                o->Position[2] + (float)(rand() % 32 - 16), Position);
                            if (gMapManager.WorldActive == 2)
                                CreateParticle(BITMAP_SMOKE, Position, o->Angle, Light);
                            else
                                CreateParticle(BITMAP_SMOKE + 1, Position, o->Angle, Light);
                        }
                }
            }
            else if (o->Owner->CurrentAction == PLAYER_SKILL_RIDER || o->Owner->CurrentAction == PLAYER_SKILL_RIDER_FLY)
            {
                if (gMapManager.WorldActive == WD_8TARKAN || gMapManager.WorldActive == WD_10HEAVEN || g_Direction.m_CKanturu.IsMayaScene())
                    SetAction(o, 7);
                else
                    SetAction(o, 6);
            }
            else if ((o->Owner->CurrentAction >= PLAYER_ATTACK_FIST && o->Owner->CurrentAction <= PLAYER_ATTACK_RIDE_CROSSBOW)
                || IsAliceRideAction_UniDino(o->Owner->CurrentAction) == true
                )
            {
                if (o->Type == MODEL_PEGASUS)
                {
                    if (gMapManager.WorldActive == WD_8TARKAN || gMapManager.WorldActive == WD_10HEAVEN || g_Direction.m_CKanturu.IsMayaScene())
                        SetAction(o, 5);
                    else
                        SetAction(o, 4);
                }
                else
                {
                    SetAction(o, 3);
                }
            }
            else
            {
                if (o->Type == MODEL_PEGASUS)
                {
                    if (gMapManager.WorldActive == WD_8TARKAN || gMapManager.WorldActive == WD_10HEAVEN || g_Direction.m_CKanturu.IsMayaScene())
                        SetAction(o, 1);
                    else
                        SetAction(o, 0);
                }
                else
                {
                    SetAction(o, 0);
                }
            }
            o->Velocity = 0.34f;
            o->Live = o->Owner->Live;
            break;
        case MODEL_BUTTERFLY01:
            FlyRange = 100.f;
            Vector(0.4f, 0.6f, 1.f, Light);
            if (rand_fps_check(2))
                CreateParticle(BITMAP_SMOKE, o->Position, o->Angle, Light, 1);
            break;
        case MODEL_HELPER:
            if (o->Owner && !g_isCharacterBuff(o->Owner, eBuff_Cloaking))
            {
                FlyRange = 150.f;
                vec3_t Position, Light;
                Vector(0.4f, 0.4f, 0.4f, Light);
                if (rand_fps_check(1))
                {
                    for (j = 0; j < 4; j++)
                    {
                        Vector((float)(rand() % 16 - 8), (float)(rand() % 16 - 8), (float)(rand() % 16 - 8), Position);
                        VectorAdd(Position, o->Position, Position);
                        CreateParticle(BITMAP_SPARK, Position, o->Angle, Light, 1);
                    }
                }
            }
        case MODEL_IMP:
            FlyRange = 150.f;
            break;
        }
        b->CurrentAction = o->CurrentAction;

        b->PlayAnimation(&o->AnimationFrame, &o->PriorAnimationFrame, &o->PriorAction, o->Velocity, o->Position, o->Angle);

        if (o->Type == MODEL_HELPER || o->Type == MODEL_IMP)
        {
            vec3_t Range;
            VectorSubtract(TargetPosition, o->Position, Range);
            float Distance = Range[0] * Range[0] + Range[1] * Range[1];
            if (Distance >= FlyRange * FlyRange)
            {
                float Angle = CreateAngle(o->Position[0], o->Position[1], TargetPosition[0], TargetPosition[1]);
                o->Angle[2] = TurnAngle2(o->Angle[2], Angle, 20.f);
            }
            AngleMatrix(o->Angle, o->Matrix);
            vec3_t Direction;
            VectorRotate(o->Direction, o->Matrix, Direction);
            VectorAddScaled(o->Position, Direction, o->Position, FPS_ANIMATION_FACTOR);
            o->Position[2] += (float)(rand() % 16 - 8);
            if (rand_fps_check(32))
            {
                float Speed = 0;
                if (Distance >= FlyRange * FlyRange)
                    Speed = -(float)(rand() % 64 + 128) * 0.1f;
                else
                {
                    Speed = -(float)(rand() % 64 + 16) * 0.1f;
                    o->Angle[2] = (float)(rand() % 360);
                }
                o->Direction[0] = 0.f;
                o->Direction[1] = Speed;
                o->Direction[2] = (float)(rand() % 64 - 32) * 0.1f;
            }
            if (o->Position[2] < o->Owner->Position[2] + 100.f) o->Direction[2] += 1.5f;
            if (o->Position[2] > o->Owner->Position[2] + 200.f) o->Direction[2] -= 1.5f;
        }
    }
    return TRUE;
}
void MoveMounts()
{
    for (int i = 0; i < MAX_MOUNTS; i++)
    {
        OBJECT* o = &Mounts[i];
        if (MoveMount(o) == FALSE) return;
    }
}

bool RenderMount(OBJECT* o, bool bForceRender)
{
    if (o->Live)
    {
        o->Visible = (bForceRender == FALSE ? TestFrustrum2D(o->Position[0] * 0.01f, o->Position[1] * 0.01f, -20.f) : true);

        if (o->Visible)
        {
            if (o->Owner->Type != MODEL_PLAYER && o->Type != MODEL_HELPER)
                return TRUE;

            if (bForceRender)
                o->Scale = o->Scale;
            else if (SceneFlag == CHARACTER_SCENE)
                o->Scale = 1.2f;
            else if (o->Type != MODEL_FENRIR_BLACK && o->Type != MODEL_FENRIR_BLUE && o->Type != MODEL_FENRIR_RED
                && o->Type != MODEL_FENRIR_GOLD
                )
                o->Scale = 1.0f;

            int State = 0;
            if (g_isCharacterBuff(o->Owner, eBuff_Cloaking))
            {
                State = 10;
            }

            RenderObject(o, false, 0, State);

            vec3_t Light;
            float  Luminosity = (float)(rand() % 30 + 70) * 0.01f;
            switch (o->Type)
            {
            case MODEL_HELPER:
                Vector(Luminosity * 0.5f, Luminosity * 0.8f, Luminosity * 0.6f, Light);
                CreateSprite(BITMAP_LIGHT, o->Position, 1.f, Light, o);
                break;
            }
        }
    }
    return TRUE;
}

void RenderMount()
{
    for (int i = 0; i < MAX_MOUNTS; i++)
    {
        OBJECT* o = &Mounts[i];
        if (RenderMount(o) == FALSE)
        {
            return;
        }
    }
}

constexpr float HorseEffectInterval = 10 * (1000.f / 25.f); // every 10 frames on a 25fps basis (400 ms)

void RenderDarkHorseSkill(OBJECT* o, BMD* b)
{
    if (o == NULL)	return;
    if (b == NULL)	return;

    // The weapon level is misused here to count how many frames have been rendered
    // for this effect...
    o->WeaponLevel++;

    if (o->LastHorseWaveEffect < WorldTime - HorseEffectInterval)
    {
        CreateEffect(BITMAP_SHOCK_WAVE, o->Position, o->Angle, o->Light);
        o->LastHorseWaveEffect = WorldTime;
    }

    if (o->AnimationFrame >= 8.f && o->AnimationFrame <= 9.5f)
    {
        if (rand_fps_check(2))
        {
            float  Matrix[3][4];
            vec3_t Angle, p, Position;
            Vector(0.f, 150.f * (o->WeaponLevel / 2) * FPS_ANIMATION_FACTOR, 0.f, p);
            Vector(0.f, 0.f, (float)(rand() % 360), Angle);
            for (int i = 0; i < 6; ++i)
            {
                Angle[2] += 60.f;
                AngleMatrix(Angle, Matrix);
                VectorRotate(p, Matrix, Position);
                VectorAdd(o->Position, Position, Position);

                CreateEffect(MODEL_GROUND_STONE + rand() % 2, Position, o->Angle, o->Light);
            }
        }
        EarthQuake = (rand() % 3 - 3) * 0.7f;
    }
    else if (o->WeaponLevel == (BYTE)(19.f / FPS_ANIMATION_FACTOR))
    {
        CreateEffect(MODEL_SKILL_FURY_STRIKE, o->Position, o->Angle, o->Light, 0, o, -1, 0, 2);
        o->WeaponLevel = -3;
    }
}

void RenderSkillEarthQuake(CHARACTER* c, OBJECT* o, BMD* b, int iMaxSkill)
{
    if (c == NULL)	return;
    if (o == NULL)	return;
    if (b == NULL)	return;

    float  Matrix[3][4];
    vec3_t Angle, p, Position;
    OBJECT& TargetO = CharactersClient[c->TargetCharacter].Object;

    o->WeaponLevel++;

    //if ( o->WeaponLevel >= 6 && o->WeaponLevel <= 8 )
    if (o->WeaponLevel == iMaxSkill - 2 ||
        o->WeaponLevel == iMaxSkill - 1 ||
        o->WeaponLevel == iMaxSkill - 0)
    {
        Vector(0.f, 40.f * (o->WeaponLevel / 2) * FPS_ANIMATION_FACTOR, 0.f, p);
        Vector(0.f, 0.f, (float)(rand() % 360), Angle);
        for (int i = 0; i < 6; ++i)
        {
            Angle[2] += 60.f;
            AngleMatrix(Angle, Matrix);
            VectorRotate(p, Matrix, Position);
            VectorAdd(TargetO.Position, Position, Position);

            int iCurrentStone = MODEL_GROUND_STONE;

            CreateEffect(iCurrentStone + 1, Position, TargetO.Angle, TargetO.Light);
        }
    }

    if (o->WeaponLevel == iMaxSkill - 1)
    {
        EarthQuake = (rand() % 3 - 3) * 0.7f;
        CreateEffect(MODEL_SKILL_FURY_STRIKE, TargetO.Position, TargetO.Angle, TargetO.Light, 0, o, -1, 0, 2);
    }

    if (o->WeaponLevel > iMaxSkill)
    {
        o->WeaponLevel = 0;
    }
}

void DeleteBoids()
{
    for (int i = 0; i < MAX_BOIDS; i++)
    {
        OBJECT* o = &Boids[i];
        o->Live = false;
    }
}

int CreateDragon(OBJECT* o, int index)
{
    if (gMapManager.WorldActive != WD_10HEAVEN) return 0;

    if (index < 3)
    {
        o->Live = true;
        OpenMonsterModel(MONSTER_MODEL_DRAGON);
        o->Type = MODEL_DRAGON_;
        o->Scale = (float)(rand() % 3 + 6) * 0.05f;
        o->Alpha = 1.f;
        o->AlphaTarget = o->Alpha;
        o->Velocity = (float)(rand() % 10 + 10) * 0.02f;
        o->Gravity = (float)(rand() % 10 + 10) * 0.05f;
        o->LightEnable = true;
        o->AlphaEnable = false;
        o->SubType = 0;
        o->HiddenMesh = -1;
        o->BlendMesh = -1;
        //o->LifeTime    = 128+rand()%128;
        //o->Timer       = (float)(rand()%10)*0.1f;
        o->CurrentAction = MONSTER01_DIE + 1;
        SetAction(o, o->CurrentAction);
        Vector(0.f, 0.f, (float)(rand() % 360), o->Angle);
        Vector(Hero->Object.Position[0] + (float)(rand() % 4000 - 2000),
            Hero->Object.Position[1] + (float)(rand() % 4000 - 2000),
            Hero->Object.Position[2] - 600.f, o->Position);
    }
    else
    {
        o->Type = MODEL_SPEARSKILL;
        o->Velocity = 2.2f;
        o->LightEnable = false;
        o->LifeTime = 240 * 40;
        o->AI = BOID_GROUND;
        Vector(1.f, 1.f, 1.f, o->Light);

        o->AlphaEnable = true;
        o->Scale = 0.8f;
        o->ShadowScale = 10.f;
        o->HiddenMesh = -1;
        o->BlendMesh = -1;
        o->Timer = (float)(rand() % 314) * 0.01f;
        Vector(Hero->Object.Position[0] + (float)(rand() % 1024 - 512),
            Hero->Object.Position[1] + (float)(rand() % 1024 - 512),
            Hero->Object.Position[2], o->Position);

        o->Position[2] = Hero->Object.Position[2];
        if (gMapManager.InBloodCastle() == true)
        {
            CreateJoint(MODEL_SPEARSKILL, o->Position, o->Position, o->Angle, 3, o, 25.0f);
        }
        else
        {
            CreateJoint(MODEL_SPEARSKILL, o->Position, o->Position, o->Angle, 1, o, 25.0f);
        }
        Vector(0.f, 0.f, 0.f, o->Angle);
        o->Angle[2] = (float)(rand() % 360);
    }
    return 1;
}

int CreateAtlanseFish(OBJECT* o)
{
    if (gMapManager.WorldActive != WD_7ATLANSE && gMapManager.WorldActive != WD_67DOPPLEGANGER3)
        return 0;

    if (Hero->Object.Position[1] * 0.01f < 128)
    {
        o->Type = MODEL_FISH01 + 1 + rand() % 2;
        o->Gravity = 15;
        o->AlphaEnable = true;
        o->Scale = 0.8f;
        o->ShadowScale = 10.f;
        o->HiddenMesh = -1;
        o->BlendMesh = -1;
        o->Timer = (float)(rand() % 314) * 0.01f;
        if (rand() % 100 < 90)
        {
            o->Velocity = 0.3f;
        }
        else
        {
            o->Velocity = 0.25f;
        }
        o->LightEnable = false;
        Vector(0.f, 0.f, 0.f, o->Angle);
        Vector(1.f, 1.f, 1.f, o->Light);
        Vector(Hero->Object.Position[0] + (float)(rand() % 1024 - 512),
            Hero->Object.Position[1] + (float)(rand() % 1024 - 512),
            Hero->Object.Position[2], o->Position);

        o->Position[2] = RequestTerrainHeight(o->Position[0], o->Position[1]) + (float)(rand() % 200 + 150);
    }
    else
    {
        o->Live = false;
    }
    return 1;
}

void MoveBat(OBJECT* o)
{
    o->Position[2] = RequestTerrainHeight(o->Position[0], o->Position[1]);
    o->Position[2] += ( - absf(sinf(o->Timer)) * 150.f + 350.f) * FPS_ANIMATION_FACTOR;
    o->Timer += 0.2f * FPS_ANIMATION_FACTOR;
}

void MoveButterFly(OBJECT* o)
{
    if (rand_fps_check(32))
    {
        o->Angle[2] = (float)(rand() % 360);
        o->Direction[2] = (float)(rand() % 15 - 7) * 1.f;
    }
    o->Direction[2] += (float)(rand() % 15 - 7) * 0.2f * FPS_ANIMATION_FACTOR;
    float Height = RequestTerrainHeight(o->Position[0], o->Position[1]);
    if (o->Position[2] < Height + 50.f)
    {
        o->Direction[2] *= pow(0.8f, FPS_ANIMATION_FACTOR);
        o->Direction[2] += 1.f * FPS_ANIMATION_FACTOR;
    }
    if (o->Position[2] > Height + 300.f)
    {
        o->Direction[2] *= pow(0.8f, FPS_ANIMATION_FACTOR);
        o->Direction[2] -= 1.f * FPS_ANIMATION_FACTOR;
    }
    o->Position[2] += (float)(rand() % 15 - 7) * 0.3f;
}

void MoveBird(OBJECT* o)
{
    if (o->AI == BOID_FLY)
    {
        if ((int)WorldTime % 8192 < 2048)
        {
            vec3_t Range;
            VectorSubtract(o->Position, Hero->Object.Position, Range);
            float Distance = sqrtf(Range[0] * Range[0] + Range[1] * Range[1]);
            if (Distance >= 200.f && Distance <= 400.f)
            {
                //int Index = TERRAIN_INDEX_REPEAT((int)(o->Position[0]/TERRAIN_SCALE),(int)(o->Position[1]/TERRAIN_SCALE));
                //if(TerrainMappingLayer1[Index]==0)
                o->AI = BOID_DOWN;
            }
        }
        o->Velocity = 1.f;
        o->Position[2] += (float)(rand() % 16 - 8) * FPS_ANIMATION_FACTOR;
        if (o->Position[2] < 200.f) o->Direction[2] = 10.f;
        else if (o->Position[2] > 600.f) o->Direction[2] = -10.f;
    }
    if (o->AI == BOID_DOWN)
    {
        //o->Velocity *= pow(0.95f, FPS_ANIMATION_FACTOR);
        o->Direction[2] = -20.f;
        float Height = RequestTerrainHeight(o->Position[0], o->Position[1]);
        if (o->Position[2] < Height)
        {
            o->AI = BOID_UP;
            o->Velocity = 1.1f;
            o->Direction[2] = 20.f;
            o->CurrentAction = 0;
        }
    }
    if (o->AI == BOID_GROUND)
    {
        if (Hero->Object.CurrentAction >= PLAYER_WALK_MALE || rand_fps_check(256))
        {
            o->AI = BOID_UP;
            o->Velocity = 1.1f;
            o->Direction[2] = 20.f;
            o->CurrentAction = 0;
        }
    }
    if (o->AI == BOID_UP)
    {
        o->Position[2] += (float)(rand() % 16 - 8) * FPS_ANIMATION_FACTOR;
        o->Velocity -= 0.005f * FPS_ANIMATION_FACTOR;
        if (o->Velocity <= 1.f)
        {
            o->AI = BOID_FLY;
        }
    }
}

void MoveHeavenBug(OBJECT* o, int index)
{
    const float iFrame = WorldTime / 40.0f;

    o->Position[0] += o->Velocity * (float)sinf(o->Angle[2]) * FPS_ANIMATION_FACTOR;
    o->Position[1] -= o->Velocity * (float)cosf(o->Angle[2]) * FPS_ANIMATION_FACTOR;
    o->Angle[2] += 0.01f * cosf((float)(34571 + iFrame + index * 41273) * 0.0003f) * sinf((float)(17732 + iFrame + index * 5161) * 0.0003f) * FPS_ANIMATION_FACTOR;

    float dx = o->Position[0] - Hero->Object.Position[0];
    float dy = o->Position[1] - Hero->Object.Position[1];
    float Range = sqrtf(dx * dx + dy * dy);
    if (Range >= 1500.f)
        o->Live = false;
    if (rand_fps_check(5120))
        o->Live = false;

    if (gMapManager.InBloodCastle())
    {
        if (o->LifeTime <= 0)
        {
            o->Live = false;
        }
    }
}

void MoveEagle(OBJECT* o)
{
    if (o->SubType == 0 && rand_fps_check(120))
    {
        o->SubType = 1;
        o->AnimationFrame = 0;
    }
    else if (o->SubType == 1 && o->AnimationFrame == 24)
    {
        o->SubType = 2;
    }
    else if (o->SubType == 2 && o->AnimationFrame == 0)
    {
        o->SubType = 0;
    }
    else if (o->SubType == 0)
    {
        o->AnimationFrame = 0;
    }

    float fSeedAngle = WorldTime * 0.001f;
    float fFlyRange = o->Gravity * FPS_ANIMATION_FACTOR;
    float fAngle = 0;
    if (o->AI == BOID_FLY)
    {
        o->HeadAngle[0] = cosf(fSeedAngle) * fFlyRange;
        o->HeadAngle[1] = sinf(fSeedAngle) * fFlyRange;
        fAngle = CreateAngle(o->Position[0], o->Position[1], o->Position[0] + o->HeadAngle[0], o->Position[1] + o->HeadAngle[1]);

        if (o->HeadAngle[2] == 0 && o->HeadAngle[0] > o->HeadAngle[1])
        {
            o->HeadAngle[2] = 1;
        }
        else if (o->HeadAngle[2] == 1 && o->HeadAngle[0] < o->HeadAngle[1])
        {
            o->HeadAngle[2] = 2;
        }
        else if (o->HeadAngle[2] == 2 && o->HeadAngle[0] > o->HeadAngle[1])
        {
            o->AI = BOID_GROUND;
            o->HeadAngle[2] = 0;

            o->HeadAngle[0] = sinf(fSeedAngle) * fFlyRange;
            o->HeadAngle[1] = cosf(fSeedAngle) * fFlyRange;
            fAngle = CreateAngle(o->Position[0], o->Position[1], o->Position[0] + o->HeadAngle[0], o->Position[1] + o->HeadAngle[1]);
        }
    }
    else if (o->AI == BOID_GROUND)
    {
        o->HeadAngle[0] = sinf(fSeedAngle) * fFlyRange;
        o->HeadAngle[1] = cosf(fSeedAngle) * fFlyRange;
        fAngle = CreateAngle(o->Position[0], o->Position[1], o->Position[0] + o->HeadAngle[0], o->Position[1] + o->HeadAngle[1]);

        if (o->HeadAngle[2] == 0 && o->HeadAngle[0] < o->HeadAngle[1])
        {
            o->HeadAngle[2] = 1;
        }
        else if (o->HeadAngle[2] == 1 && o->HeadAngle[0] > o->HeadAngle[1])
        {
            o->HeadAngle[2] = 2;
        }
        else if (o->HeadAngle[2] == 2 && o->HeadAngle[0] < o->HeadAngle[1])
        {
            o->AI = BOID_FLY;
            o->HeadAngle[2] = 0;

            o->HeadAngle[0] = cosf(fSeedAngle) * fFlyRange * FPS_ANIMATION_FACTOR;
            o->HeadAngle[1] = sinf(fSeedAngle) * fFlyRange * FPS_ANIMATION_FACTOR;
            fAngle = CreateAngle(o->Position[0], o->Position[1], o->Position[0] + o->HeadAngle[0], o->Position[1] + o->HeadAngle[1]);
        }
    }

    o->Position[0] += o->HeadAngle[0];
    o->Position[1] += o->HeadAngle[1];
    o->Position[2] += sinf(WorldTime * 0.0005f) * 1.0f;
    o->Angle[1] += sinf(WorldTime * 0.001f) * 0.4f;
    o->Angle[2] = fAngle + 270;
}

void MoveTornado(OBJECT* o)
{
    o->Scale = 1.0f;
    if (rand_fps_check(500))
    {
        o->HeadAngle[0] = (rand() % 314) / 100.0f;
    }
    o->Position[0] += sinf(o->HeadAngle[0]) * 2.0f * FPS_ANIMATION_FACTOR;
    o->Position[1] += cosf(o->HeadAngle[0]) * 2.0f * FPS_ANIMATION_FACTOR;
    o->Angle[2] = 0;
    if (o->BlendMeshLight < 1.0f) o->BlendMeshLight += 0.1f * FPS_ANIMATION_FACTOR;
}

void MoveBoidGroup(OBJECT* o, int index)
{
    if (!g_pOption->GetRenderAllEffects())
    {
        return;
    }

    if (o->AI != BOID_GROUND)
    {
        if (o->Type != MODEL_BUTTERFLY01 || rand_fps_check(4))
        {
            MoveBoid(o, index, Boids, MAX_BOIDS);
        }

        AngleMatrix(o->Angle, o->Matrix);
        vec3_t p, Direction;
        if (gMapManager.WorldActive == WD_7ATLANSE || gMapManager.WorldActive == WD_67DOPPLEGANGER3)
        {
            if (o->Timer < 5.f)
            {
                if (index < 35)
                {
                    Vector(o->Velocity * (float)(rand() % 16 + 8), 0.f, o->Direction[2], Direction);
                }
                else
                {
                    Vector(o->Velocity * (float)(rand() % 16 + 16), 0.f, o->Direction[2], Direction);
                }
                o->Gravity = 15;
            }
            else
            {
                Vector(o->Velocity * (float)(rand() % 32 + 32), 0.f, o->Direction[2], Direction);
                o->Gravity = 5;
            }
            o->Timer += 0.1f * FPS_ANIMATION_FACTOR;
            if (o->Timer >= 10)
            {
                o->Timer = 0.f;
            }
        }
        else
        {
            Vector(o->Velocity * 25.f, 0.f, o->Direction[2], Direction);
        }
        VectorRotate(Direction, o->Matrix, p);
        VectorAddScaled(o->Position, p, o->Position, FPS_ANIMATION_FACTOR);
        o->Direction[0] = o->Position[0] + 3.f * p[0];
        o->Direction[1] = o->Position[1] + 3.f * p[1];

        float dx = o->Position[0] - Hero->Object.Position[0];
        float dy = o->Position[1] - Hero->Object.Position[1];
        float Range = sqrtf(dx * dx + dy * dy);
        float FlyDistance = 1500.f;
        if (o->Type == MODEL_DRAGON_)
        {
            FlyDistance = 4000.f;
        }
        else if (o->Type == MODEL_BAHAMUT)
        {
            FlyDistance = 3000.f;
        }
        else if (gMapManager.WorldActive == WD_51HOME_6TH_CHAR
            );
        else
        {
            if (rand_fps_check(512))
                o->Live = false;
        }
            if (Range >= FlyDistance)
                o->Live = false;
    }
}

void MoveBoids()
{
    if (!g_pOption->GetRenderAllEffects())
    {
        return;
    }

    if (EnableEvent != 0)
    {
        OBJECT* o = &Hero->Object;
        vec3_t Position, Angle, Light;
        if (rand_fps_check(40))
        {
            Vector(Hero->Object.Position[0] + (float)(rand() % 600 - 200) * FPS_ANIMATION_FACTOR,
                Hero->Object.Position[1] + (float)(rand() % 400 + 200) * FPS_ANIMATION_FACTOR,
                Hero->Object.Position[2] + 300.f * FPS_ANIMATION_FACTOR,
                Position);
            Vector(0.f, 0.f, 0.f, Angle);
            Vector(1.f, 1.f, 1.f, Light);
            CreateEffect(MODEL_FIRE, Position, Angle, Light, 3);
            PlayBuffer(SOUND_METEORITE01);
        }
        Vector(-0.3f, -0.3f, -0.2f, Light);
        AddTerrainLight(o->Position[0], o->Position[1], Light, 16, PrimaryTerrainLight);
    }

    int Index = TERRAIN_INDEX_REPEAT((int)(Hero->Object.Position[0] / TERRAIN_SCALE), (int)(Hero->Object.Position[1] / TERRAIN_SCALE));
    for (int i = 0; i < MAX_BOIDS; i++)
    {
        bool bOut = false;
        bool bCreate = true;
        switch (gMapManager.WorldActive)
        {
        case WD_7ATLANSE:
        case WD_67DOPPLEGANGER3:
            break;
        case WD_10HEAVEN:
            if (i >= 13)
            {
                bOut = true;
            }
            break;
        case WD_51HOME_6TH_CHAR:
            if (i >= 2)
            {
                bOut = TRUE;
            }
            break;
        default:
            if (gMapManager.InHellas() == true)
            {
                if (i > 1)
                {
                    bOut = true;
                }
                if ((rand() % 10))
                {
                    bCreate = false;
                }
                break;
            }

            if (i >= 5)
            {
                bOut = true;
            }
            break;
        }
        if (bOut)
        {
            break;
        }
        OBJECT* o = &Boids[i];
        if (!o->Live && bCreate)
        {
            if (EnableEvent != 0)
            {
                if (rand_fps_check(300))
                {
                    o->Live = true;
                    OpenMonsterModel(MONSTER_MODEL_DRAGON);
                    o->Type = MODEL_DRAGON_;
                    o->Scale = (float)(rand() % 3 + 6) * 0.1f;
                    o->Alpha = 1.f;
                    o->AlphaTarget = 1.f;
                    o->Velocity = 0.5f;
                    o->LightEnable = true;
                    o->AlphaEnable = false;
                    o->SubType = 0;
                    o->HiddenMesh = -1;
                    o->BlendMesh = -1;
                    o->LifeTime = 128 + rand() % 128;
                    o->Timer = (float)(rand() % 10) * 0.1f;
                    Vector(0.f, 0.f, -90.f, o->Angle);
                    if (EnableEvent == 3)
                    {
                        o->SubType = 1;
                    }
                    Vector(Hero->Object.Position[0] + (float)(rand() % 600 - 100),
                        Hero->Object.Position[1] + (float)(rand() % 400 + 200),
                        Hero->Object.Position[2] + 300.f,
                        o->Position);
                }
            }
            else if (gMapManager.WorldActive == WD_0LORENCIA
                || gMapManager.WorldActive == WD_1DUNGEON
                || gMapManager.WorldActive == WD_3NORIA
                || gMapManager.WorldActive == WD_4LOSTTOWER
                || gMapManager.WorldActive == WD_10HEAVEN
                || ((gMapManager.WorldActive == WD_7ATLANSE || gMapManager.WorldActive == WD_67DOPPLEGANGER3) && (TerrainWall[Index] == 0 || TerrainWall[Index] == TW_CHARACTER))
                || gMapManager.InBloodCastle()
                || gMapManager.InHellas()
                || (gMapManager.WorldActive == WD_51HOME_6TH_CHAR && i < 1 && rand_fps_check(500) && Hero->SafeZone != true)
                )
            {
                int iCreateBoid = 0;

                o->Initialize();

                o->Live = true;
                o->Velocity = 1.f;
                o->LightEnable = true;
                o->LifeTime = 0;
                o->SubType = 0;
                Vector(0.5f, 0.5f, 0.5f, o->Light);
                o->Alpha = 0.f;
                o->AlphaTarget = 1.f;
                o->Gravity = 13;
                o->AI = 0;
                o->CurrentAction = 0;

                if (gMapManager.WorldActive == WD_0LORENCIA)
                    o->Type = MODEL_BIRD01;
                else if (gMapManager.WorldActive == WD_1DUNGEON || gMapManager.WorldActive == WD_4LOSTTOWER)
                    o->Type = MODEL_BAT01;
                else if (gMapManager.WorldActive == WD_3NORIA)
                {
                    o->Type = MODEL_BUTTERFLY01;
                    o->Velocity = 0.3f;
                    o->LightEnable = false;
                    Vector(1.f, 1.f, 1.f, o->Light);
                }
                else if (gMapManager.InBloodCastle() == true)
                {
                    o->Type = MODEL_CROW;
                }
                else if (gMapManager.WorldActive == WD_51HOME_6TH_CHAR)
                {
                    o->Type = MODEL_MAP_TORNADO;
                    o->Velocity = 0.0f;
                    o->BlendMeshLight = 0.0f;
                    o->HeadAngle[0] = (rand() % 314) / 100.0f;
                    o->Position[1] += 5.0f;
                    o->m_bRenderAfterCharacter = true;
                }
                else
                {
                    if (iCreateBoid == 0)
                    {
                        iCreateBoid = CreateDragon(o, i);
                    }
                    if (iCreateBoid == 0)
                    {
                        iCreateBoid = CreateAtlanseFish(o);
                    }
                    if (iCreateBoid == 0)
                    {
                        iCreateBoid = CreateBigMon(o);
                    }
                }
                if (iCreateBoid != 1)
                {
                    o->AlphaEnable = true;
                    o->Scale = 0.8f;
                    o->ShadowScale = 10.f;
                    o->HiddenMesh = -1;
                    o->BlendMesh = -1;
                    o->Timer = (float)(rand() % 314) * 0.01f;
                    Vector(Hero->Object.Position[0] + (float)(rand() % 1024 - 512),
                        Hero->Object.Position[1] + (float)(rand() % 1024 - 512),
                        Hero->Object.Position[2], o->Position);
                    o->Position[2] = RequestTerrainHeight(o->Position[0], o->Position[1]) + (float)(rand() % 200 + 150);
                    Vector(0.f, 0.f, 0.f, o->Angle);
                }
            }
        }

        if (o->Live)
        {
            BMD* b = &Models[o->Type];
            float PlaySpeed = 1.f;
            if (o->Type == MODEL_DRAGON_ || o->Type == MODEL_BAHAMUT)
            {
                PlaySpeed = 0.5f;
            }

            if (EnableEvent != 0 && o->Type == MODEL_DRAGON_)
            {
                SetAction(o, MONSTER01_DIE + 1);
                b->CurrentAction = o->CurrentAction;
                b->PlayAnimation(&o->AnimationFrame, &o->PriorAnimationFrame, &o->PriorAction, PlaySpeed, o->Position, o->Angle);
                AngleMatrix(o->Angle, o->Matrix);
                vec3_t Position, Direction;
                Vector(o->Scale * 40.f, 0.f, 0.f, Position);
                VectorRotate(Position, o->Matrix, Direction);
                VectorAddScaled(o->Position, Direction, o->Position, FPS_ANIMATION_FACTOR);
                o->Position[2] = RequestTerrainHeight(o->Position[0], o->Position[1]) + 300.f;
                o->Position[2] += -absf(sinf(o->Timer)) * 100.f + 100.f;
                o->Timer += o->Scale * 0.05f * FPS_ANIMATION_FACTOR;
                o->LifeTime -= FPS_ANIMATION_FACTOR;
                if (o->LifeTime <= 0)
                    o->Live = false;
                if (rand_fps_check(128))
                    PlayBuffer(SOUND_MONSTER_BULLATTACK1);
            }
            else
            {
                b->CurrentAction = o->CurrentAction;

                if (gMapManager.WorldActive == WD_51HOME_6TH_CHAR
                    )
                {
                    PlaySpeed = b->Actions[b->CurrentAction].PlaySpeed;
                }
                b->PlayAnimation(&o->AnimationFrame, &o->PriorAnimationFrame, &o->PriorAction, PlaySpeed, o->Position, o->Angle);

                switch (o->Type)
                {
                case MODEL_BAT01:
                    MoveBat(o);
                    break;

                case MODEL_BUTTERFLY01:
                    MoveButterFly(o);
                    break;

                case MODEL_BIRD01:
                case MODEL_CROW:
                    MoveBird(o);
                    break;

                case MODEL_SPEARSKILL:
                    MoveHeavenBug(o, i);
                    break;

                case MODEL_BAHAMUT:
                    MoveBigMon(o);
                    break;

                case MODEL_EAGLE:
                    MoveEagle(o);
                    break;
                case MODEL_MAP_TORNADO:
                    MoveTornado(o);
                    break;
                }

                MoveBoidGroup(o, i);

                if (o->LifeTime <= 0 && (gMapManager.WorldActive == WD_7ATLANSE || gMapManager.WorldActive == WD_67DOPPLEGANGER3) && TerrainWall[Index] == TW_SAFEZONE)
                {
                    o->Angle[2] += 180.f;
                    if (o->Angle[2] >= 360.f) o->Angle[2] -= 360.f;
                    o->LifeTime = 10;
                    o->SubType++;
                }

                if (o->Type == MODEL_EAGLE || o->Type == MODEL_MAP_TORNADO)
                {
                    // do nothing?
                }
                else if (o->SubType >= 2)
                {
                    o->Live = false;
                }

                o->LifeTime -= FPS_ANIMATION_FACTOR;

                float dx = o->Position[0] - Hero->Object.Position[0];
                float dy = o->Position[1] - Hero->Object.Position[1];
                float Range = sqrtf(dx * dx + dy * dy);
                if (Range < 600)
                {
                    if (o->Type == MODEL_BIRD01)
                    {
                        if (rand_fps_check(512))
                            PlayBuffer(SOUND_BIRD01, o);
                        if (rand_fps_check(512))
                            PlayBuffer(SOUND_BIRD02, o);
                    }
                    else if (o->Type == MODEL_BAT01)
                    {
                        if (rand_fps_check(256))
                            PlayBuffer(SOUND_BAT01, o);
                    }
                    else if (o->Type == MODEL_CROW)
                    {
                        if (TerrainWall[Index] == TW_SAFEZONE)
                        {
                            if (rand_fps_check(128))
                                PlayBuffer(SOUND_CROW, o);
                        }
                    }
                }
            }
            Alpha(o);
        }
    }
}

void RenderBoids(bool bAfterCharacter)
{
    if (!g_pOption->GetRenderAllEffects())
    {
        return;
    }

    for (int i = 0; i < MAX_BOIDS; i++)
    {
        OBJECT* o = &Boids[i];
        if (o->m_bRenderAfterCharacter != bAfterCharacter) continue;
        if (MODEL_SPEARSKILL != o->Type)
        {
            o->Angle[2] += 90.f;
        }
        if (o->Live)
        {
            o->Visible = TestFrustrum2D(o->Position[0] * 0.01f, o->Position[1] * 0.01f, -20.f);
            if (o->Visible)
            {
                if (MODEL_SPEARSKILL == o->Type)
                {
                    continue;
                }
                RenderObject(o, true);

                BMD* b = &Models[o->Type];
                vec3_t p, Position, Light;
                switch (o->Type)
                {
                case MODEL_DRAGON_:
                    if (o->SubType == 1)
                    {
                        float Bright = 1.0f;
                        RenderPartObjectBodyColor(&Models[o->Type], o, o->Type, o->Alpha, RENDER_METAL | RENDER_BRIGHT, Bright);
                        RenderPartObjectBodyColor(&Models[o->Type], o, o->Type, o->Alpha, RENDER_CHROME | RENDER_BRIGHT, Bright);
                    }
                    if (EnableEvent != 0)
                    {
                        Vector(0.f, -50.f, 0.f, p);
                        b->TransformPosition(BoneTransform[11], p, Position);
                        Vector(1.f, 0.f, 0.f, Light);
                        CreateSprite(BITMAP_LIGHTNING + 1, Position, 1.f, Light, o);
                        Vector(1.f, 1.f, 1.f, Light);
                        CreateParticleFpsChecked(BITMAP_FIRE, Position, o->Angle, Light);
                    }
                    break;

                case MODEL_BUTTERFLY01:
                {
                    float  Luminosity = (float)(rand() % 32 + 64) * 0.01f;

                    Vector(Luminosity * 0.2f, Luminosity * 0.4f, Luminosity * 0.4f, Light);
                    CreateSprite(BITMAP_LIGHT, o->Position, 1.f, Light, o);
                }
                break;

                case MODEL_MAP_TORNADO:
                {
                    CreateParticleFpsChecked(BITMAP_CLOUD, o->Position, o->Angle, o->Light, 18, o->Scale, o);
                }
                break;

                case MODEL_CROW:
                    Vector(-5.f, 0.f, 0.f, p);
                    b->TransformPosition(BoneTransform[1], p, Position, true);

                    float Luminosity = (float)(rand() % 32 + 128) * 0.01f;
                    Vector(Luminosity * 1.f, Luminosity * 0.2f, 0.f, Light);
                    CreateSprite(BITMAP_LIGHT, Position, 0.1f, Light, o);

                    Vector(5.f, 0.f, 0.f, p);
                    b->TransformPosition(BoneTransform[1], p, Position, true);
                    CreateSprite(BITMAP_LIGHT, Position, 0.1f, Light, o);
                    break;
                }
                if (gMapManager.WorldActive != WD_10HEAVEN)
                {
                    EnableAlphaTest();
                    if (o->Type == MODEL_EAGLE)
                    {
                        if (o->ShadowScale == 0)
                            glColor4f(0.f, 0.f, 0.f, 0.0f);
                        else
                            glColor4f(0.f, 0.f, 0.f, 1.0f);
                    }
                    else
                        glColor4f(0.f, 0.f, 0.f, 0.2f);

                    if (gMapManager.WorldActive == WD_51HOME_6TH_CHAR && o->Type == MODEL_MAP_TORNADO);
                    else
                    {
                        VectorCopy(o->Position, Position);
                        Position[2] = RequestTerrainHeight(o->Position[0], o->Position[1]);
                        VectorCopy(Position, b->BodyOrigin);

                        b->RenderBodyShadow();
                    }
                }
            }
        }
        if (MODEL_SPEARSKILL != o->Type)
        {
            o->Angle[2] -= 90.f;
        }
    }
}

void RenderFishs()
{
    if (!g_pOption->GetRenderAllEffects())
    {
        return;
    }

    for (int i = 0; i < MAX_FISHS; i++)
    {
        OBJECT* o = &Fishs[i];
        o->Angle[2] += 90.f;
        if (o->Live)
        {
            o->Visible = TestFrustrum2D(o->Position[0] * 0.01f, o->Position[1] * 0.01f, -20.f);
            if (o->Visible && o->Type != -1)
            {
                RenderObject(o);

                if (o->Type == MODEL_FISH01 + 7 || o->Type == MODEL_FISH01 + 8)
                {
                }
                else
                {
                    if (gMapManager.WorldActive != WD_10HEAVEN)
                    {
                        EnableAlphaTest();
                        glColor4f(0.f, 0.f, 0.f, 0.2f);
                        BMD* b = &Models[o->Type];
                        vec3_t Position;
                        VectorCopy(o->Position, Position);
                        Position[2] = RequestTerrainHeight(o->Position[0], o->Position[1]);
                        VectorCopy(Position, b->BodyOrigin);
                        b->RenderBodyShadow();
                    }
                }
            }
        }
        o->Angle[2] -= 90.f;
    }
}

void MoveFishs()
{
    if (!g_pOption->GetRenderAllEffects())
    {
        return;
    }

    for (int i = 0; i < MAX_FISHS; i++)
    {
        if (gMapManager.WorldActive != WD_7ATLANSE && gMapManager.WorldActive != WD_8TARKAN
            && gMapManager.InHellas() == false
            && gMapManager.WorldActive != WD_34CRYWOLF_1ST
            && gMapManager.WorldActive != WD_67DOPPLEGANGER3
            )
        {
            if (i >= 3) continue;
        }

        OBJECT* o = &Fishs[i];
        if (!o->Live)
        {
            Vector(Hero->Object.Position[0] + (float)(rand() % 1024 - 512),
                Hero->Object.Position[1] + (float)(rand() % 1024 - 512),
                Hero->Object.Position[2], o->Position);
            int Index = TERRAIN_INDEX_REPEAT((int)(o->Position[0] / TERRAIN_SCALE), (int)(o->Position[1] / TERRAIN_SCALE));
            if ((gMapManager.WorldActive == WD_0LORENCIA && TerrainMappingLayer1[Index] == 5) ||
                (gMapManager.WorldActive == WD_1DUNGEON && TerrainWall[Index] < TW_NOGROUND) ||
                (gMapManager.WorldActive == WD_6STADIUM && TerrainWall[Index] < TW_NOGROUND) ||
                ((gMapManager.WorldActive == WD_7ATLANSE
                    || gMapManager.WorldActive == WD_67DOPPLEGANGER3
                    ) && (TerrainWall[Index] == 0 || TerrainWall[Index] == TW_CHARACTER)) ||
                (gMapManager.InHellas() && (TerrainWall[Index] == 0 || TerrainWall[Index] == TW_CHARACTER)) ||
                (M33Aida::IsInAida() && (TerrainWall[Index] == 0 || TerrainWall[Index] == TW_CHARACTER)) ||
                ((gMapManager.WorldActive == WD_8TARKAN
                    || gMapManager.WorldActive == WD_34CRYWOLF_1ST
                    ) && (TerrainWall[Index] == 0 || TerrainWall[Index] == TW_CHARACTER))
                )
            {
                o->Live = true;
                o->Alpha = 0.f;
                o->AlphaTarget = 1.f;
                o->BlendMesh = -1;
                Vector(0.5f, 0.5f, 0.5f, o->Light);
                Vector(0.f, 0.f, 0.f, o->Angle);
                o->LightEnable = true;
                o->AlphaEnable = true;
                o->SubType = 0;
                o->HiddenMesh = -1;
                o->LifeTime = rand() % 128;
                o->Scale = (float)(rand() % 4 + 4) * 0.1f;
                o->Gravity = 13;
                o->bBillBoard = true;
                switch (gMapManager.WorldActive)
                {
                case WD_0LORENCIA:
                    o->Type = MODEL_FISH01;
                    o->AlphaTarget = (float)(rand() % 2 + 2) * 0.1f;
                    o->Velocity = 0.6f / o->Scale;
                    break;
                case WD_1DUNGEON:
                    o->Type = MODEL_RAT01;
                    o->Velocity = 0.6f / o->Scale;
                    break;
                case WD_6STADIUM:
                    o->Type = MODEL_BUG01;
                    o->Velocity = 0.6f / o->Scale;
                    break;
                case WD_33AIDA:
                case WD_8TARKAN:
                    o->Type = MODEL_BUG01 + 1;
                    o->Scale = (float)(rand() % 4 + 8) * 0.1f;
                    o->Velocity = 2.5f / o->Scale;
                    o->Gravity = 9;
                    o->LifeTime = 100;
                    VectorCopy(o->Position, o->EyeLeft);
                    CreateJointFpsChecked(BITMAP_JOINT_ENERGY, o->Position, o->Position, o->Angle, 4, o, 30.f);
                    break;
                case WD_34CRYWOLF_1ST:
                    if (Hero->SafeZone != true)
                    {
                        o->Type = MODEL_SCOLPION;
                        o->Scale = (float)(rand() % 4 + 8) * 0.1f;
                        o->Velocity = (((rand() % 8 + 1)) * 0.1f) / o->Scale;
                        VectorCopy(o->Position, o->EyeLeft);
                        o->Gravity = 1;
                        o->LifeTime = 100;
                        CreateJointFpsChecked(BITMAP_SCOLPION_TAIL, o->Position, o->Position, o->Angle, 0, o, 30.f);
                    }
                    else
                        o->Live = false;
                    break;
                case WD_7ATLANSE:
                case WD_67DOPPLEGANGER3:
                    o->Scale = (float)(rand() % 2 + 8) * 0.1f;
                    if (Hero->Object.Position[1] * 0.01f < 128)
                    {
                        o->Type = MODEL_FISH01 + 1 + 2 + rand() % 4;
                        o->Velocity = 1.f / o->Scale;
                        if (rand_fps_check(2))
                            o->Gravity = 2;
                        else
                            o->Gravity = 3;
                    }
                    else
                    {
                        o->Type = MODEL_FISH01 + 1 + 6 + rand() % 2;
                        if (o->Type == MODEL_FISH01 + 7 || o->Type == MODEL_FISH01 + 8)
                        {
                            o->BlendMesh = 0;
                            o->BlendMeshLight = 1.f;
                        }
                        o->Velocity = 0.5f / o->Scale;
                        if (rand_fps_check(2))
                            o->Gravity = 1;
                        else
                            o->Gravity = 2;
                    }
                    o->Timer = (float)(rand() % 32) * 0.1f;
                    o->Position[2] = RequestTerrainHeight(o->Position[0], o->Position[1]) + (float)(rand() % 150 + 50);
                    break;
                }

                if (gMapManager.InHellas())
                {
                    o->Type = -1;
                    o->Scale = (float)(rand() % 4 + 8) * 0.1f;
                    o->Velocity = 2.5f / o->Scale;
                    o->Gravity = 9;
                    o->LifeTime = 70;
                    CreateJointFpsChecked(BITMAP_FLARE + 1, o->Position, o->Position, o->Angle, 8, o, 50.f);
                }
            }
        }
        if (o->Live)
        {
            if (o->Type != -1)
            {
                BMD* b = &Models[o->Type];
                b->CurrentAction = o->CurrentAction;

                b->PlayAnimation(&o->AnimationFrame, &o->PriorAnimationFrame, &o->PriorAction, o->Velocity * 0.5f, o->Position, o->Angle);
            }
            if (o->Type == MODEL_FISH01 + 7 || o->Type == MODEL_FISH01 + 8)
            {
                o->BlendMeshLight = sinf(o->Timer) * 0.4f + 0.5f;
                o->Timer += 0.1f * FPS_ANIMATION_FACTOR;
            }

            if ((o->Type >= MODEL_FISH01 && o->Type <= MODEL_FISH01 + 10) ||
                o->LifeTime > 0)
            {
                if (o->Type == MODEL_BUG01
                    || o->Type == MODEL_SCOLPION
                    )
                    SetAction(o, 1);
                else
                    SetAction(o, 0);
                MoveBoid(o, i, Fishs, MAX_FISHS);
                AngleMatrix(o->Angle, o->Matrix);
                vec3_t Position, Direction;
                Vector(o->Velocity * (float)(rand() % 4 + 6), 0.f, 0.f, Position);
                VectorRotate(Position, o->Matrix, Direction);
                VectorAddScaled(o->Position, Direction, o->Position, FPS_ANIMATION_FACTOR);
                if (gMapManager.WorldActive != 7 || gMapManager.InHellas() == false || gMapManager.WorldActive != WD_67DOPPLEGANGER3)
                {
                    o->Position[2] = RequestTerrainHeight(o->Position[0], o->Position[1]);
                }
                o->Direction[0] = o->Position[0] + 3.f * Direction[0];
                o->Direction[1] = o->Position[1] + 3.f * Direction[1];
                int Index = TERRAIN_INDEX_REPEAT((int)(o->Position[0] / TERRAIN_SCALE), (int)(o->Position[1] / TERRAIN_SCALE));

                if (gMapManager.WorldActive == WD_8TARKAN || gMapManager.InHellas() || M33Aida::IsInAida())
                {
                    if (TerrainWall[Index] == 1 || TerrainWall[Index] >= TW_NOGROUND)
                    {
                        o->Angle[2] += 180.f;
                        if (o->Angle[2] >= 360.f) o->Angle[2] -= 360.f;
                        o->SubType++;
                    }
                    else
                    {
                        if (o->SubType > 0) o->SubType--;
                    }

                    if (gMapManager.InHellas() && o->LifeTime <= 1)
                    {
                        o->Live = false;
                    }
                }
                else if ((o->Type == MODEL_FISH01 && TerrainMappingLayer1[Index] != 5) ||
                    (o->Type == MODEL_RAT01 && TerrainWall[Index] >= TW_NOGROUND) ||
                    ((gMapManager.WorldActive == WD_7ATLANSE || gMapManager.WorldActive == WD_67DOPPLEGANGER3) && (TerrainWall[Index] == 1 || TerrainWall[Index] >= TW_NOGROUND)))
                {
                    o->Angle[2] += 180.f;
                    if (o->Angle[2] >= 360.f) o->Angle[2] -= 360.f;
                    o->SubType++;
                }
                else
                {
                    if (o->SubType > 0) o->SubType--;
                }
                if (o->SubType >= 2) o->Live = false;

                if (o->Type == MODEL_BUG01 + 1
                    || o->Type == MODEL_SCOLPION
                    )
                {
                    VectorCopy(o->Position, o->EyeLeft);
                }

                float dx = o->Position[0] - Hero->Object.Position[0];
                float dy = o->Position[1] - Hero->Object.Position[1];
                float Range = sqrtf(dx * dx + dy * dy);
                if (Range >= 1500.f)
                    o->Live = false;

                if (Range < 600.f)
                    if (o->Type == MODEL_RAT01 && rand_fps_check(256))
                        PlayBuffer(SOUND_RAT01, o);
            }
            else
            {
                SetAction(o, 0);
            }

            o->LifeTime -= FPS_ANIMATION_FACTOR;
            if (o->LifeTime <= 0)
            {
                if (o->Type == MODEL_BUG01)
                {
                }
                else
                {
                    if (rand_fps_check(64))
                        o->LifeTime = rand() % 128;
                }
            }
            Alpha(o);
        }
    }
}