///////////////////////////////////////////////////////////////////////////////
// 3D 특수효과 관련 함수
//
// *** 함수 레벨: 3
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ZzzOpenglUtil.h"
#include "ZzzBMD.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzLodTerrain.h"
#include "ZzzTexture.h"
#include "ZzzAi.h"
#include "ZzzEffect.h"
#include "DSPlaySound.h"
#include "WSClient.h"
#include "CSChaosCastle.h"
#include "MapManager.h"
#include "w_MapHeaders.h"
#include "NewUISystem.h"

float RainTarget = 0;
float RainCurrent = 0.f;

static  int RainSpeed = 30;
static  int RainAngle = 0;
static  int RainPosition = 0;

void CreateBonfire(vec3_t Position, vec3_t Angle)
{
    Position[0] += (float)(rand() % 16 - 8);
    Position[1] += (float)(rand() % 16 - 8);
    Position[2] += (float)(rand() % 16 - 8);
    vec3_t Light;
    Vector(1.f, 1.f, 1.f, Light);
    CreateParticle(BITMAP_FIRE, Position, Angle, Light, rand() % 4);
    if (rand_fps_check(4))
    {
        CreateParticle(BITMAP_SPARK, Position, Angle, Light);
        vec3_t a;
        Vector(-(float)(rand() % 60 + 30), 0.f, (float)(rand() % 360), a);
        CreateJoint(BITMAP_JOINT_SPARK, Position, Position, a);
    }
    float Luminosity = (float)(rand() % 6 + 6) * 0.1f;
    Vector(Luminosity, Luminosity * 0.6f, Luminosity * 0.4f, Light);
    AddTerrainLight(Position[0], Position[1], Light, 4, PrimaryTerrainLight);
}

void CreateFire(int Type, OBJECT* o, float x, float y, float z)
{
    vec3_t Light;
    vec3_t p, Position;
    float Luminosity;
    float Matrix[3][4];
    AngleMatrix(o->Angle, Matrix);
    Vector(x, y, z, p);
    VectorRotate(p, Matrix, Position);
    VectorAdd(Position, o->Position, Position)
        Position[0] += (float)(rand() % 16 - 8);
    Position[1] += (float)(rand() % 16 - 8);
    Position[2] += (float)(rand() % 16 - 8);
    switch (Type)
    {
    case 0:
        Luminosity = (float)(rand() % 6 + 6) * 0.1f;
        Vector(Luminosity, Luminosity * 0.6f, Luminosity * 0.4f, Light);
        if (rand_fps_check(2))
            CreateParticle(BITMAP_FIRE, Position, o->Angle, Light, rand() % 4);
        AddTerrainLight(Position[0], Position[1], Light, 4, PrimaryTerrainLight);
        break;
    case 1:
        if (rand_fps_check(2))
            CreateParticle(BITMAP_SMOKE, Position, o->Angle, o->Light);
        break;
    case 2:
        if (rand_fps_check(2))
            CreateParticle(BITMAP_SMOKE, Position, o->Angle, o->Light, 2);
        break;
    }
}

void CheckSkull(OBJECT* o)
{
    vec3_t Position;
    VectorCopy(Hero->Object.Position, Position);
    if (Hero->Object.CurrentAction >= PLAYER_WALK_MALE && Hero->Object.CurrentAction <= PLAYER_RUN_RIDE_WEAPON
        || (Hero->Object.CurrentAction == PLAYER_RAGE_UNI_RUN || Hero->Object.CurrentAction == PLAYER_RAGE_UNI_RUN_ONE_RIGHT))
    {
        if (o->Direction[0] < 0.1f)
        {
            float dx = Position[0] - o->Position[0];
            float dy = Position[1] - o->Position[1];
            float Distance = sqrtf(dx * dx + dy * dy);
            if (Distance < 50.f)
            {
                Vector(-dx * 0.4f, -dy * 0.4f, 0.f, o->Direction);
                o->HeadAngle[1] = -dx * 4.f;
                o->HeadAngle[0] = -dy * 4.f;
                PlayBuffer(SOUND_BONE2, o);
            }
        }
    }
    VectorScale(o->Direction, 0.6f, o->Direction);
    VectorScale(o->HeadAngle, 0.6f, o->HeadAngle);
    VectorAddScaled(o->Position, o->Direction, o->Position, FPS_ANIMATION_FACTOR);
    VectorAdd(o->Angle, o->HeadAngle, o->Angle);
}

bool CreateDevilSquareRain(PARTICLE* o, int Index)
{
    if (gMapManager.InDevilSquare() == false
        && gMapManager.WorldActive != WD_34CRYWOLF_1ST
        )
    {
        return false;
    }

    if (gMapManager.WorldActive == WD_34CRYWOLF_1ST && weather != 1)
        return false;

    o->Type = BITMAP_RAIN;
    if (Index < 300)
    {
        Vector(
            Hero->Object.Position[0] + (float)(rand() % 1600 - 800),
            Hero->Object.Position[1] + (float)(rand() % 1400 - 500),
            Hero->Object.Position[2] + (float)(rand() % 200 + 300),
            o->Position);
    }
    else
    {
        Vector(
            Hero->Object.Position[0] + (float)(rand() % 1600 - 800),
            Hero->Object.Position[1] + (float)(rand() % 300 + 1000) - RainPosition,
            Hero->Object.Position[2] + (float)(rand() % 200 + 300),
            o->Position);
    }

    if (rand_fps_check(2))
    {
        Vector((float)(-(rand() % 20 + 20)), 0.f, 0.f, o->Angle);
    }
    else
    {
        Vector((float)(-(rand() % 20 + 30 + RainAngle)), 0.f, 0.f, o->Angle);
    }

    vec3_t Velocity;
    Vector(0.f, 0.f, -(float)((rand() % 40 + RainSpeed) * FPS_ANIMATION_FACTOR), Velocity);
    float Matrix[3][4];
    AngleMatrix(o->Angle, Matrix);
    VectorRotate(Velocity, Matrix, o->Velocity);

    return true;
}

bool CreateChaosCastleRain(PARTICLE* o, int Index)
{
    if (gMapManager.InChaosCastle() == false) return false;

    o->Type = BITMAP_RAIN;
    o->TurningForce[0] = 1.f;
    o->TurningForce[1] = 30.f + rand() % 10;
    if (Index < 300)
    {
        Vector(
            Hero->Object.Position[0] + (float)(rand() % 1600 - 800),
            Hero->Object.Position[1] + (float)(rand() % 1400 - 500),
            Hero->Object.Position[2] + (float)(rand() % 200 + 300),
            o->Position);
    }
    else
    {
        Vector(
            Hero->Object.Position[0] + (float)(rand() % 1600 - 800),
            Hero->Object.Position[1] + (float)(rand() % 300 + 1000) - RainPosition,
            Hero->Object.Position[2] + (float)(rand() % 200 + 300),
            o->Position);
    }
    if (rand_fps_check(2))
    {
        Vector((float)(-(rand() % 20 + 20)), 0.f, 0.f, o->Angle);
    }
    else
    {
        Vector((float)(-(rand() % 20 + 30 + RainAngle)), 0.f, 0.f, o->Angle);
    }
    vec3_t Velocity;
    Vector(0.f, 0.f, -(float)((rand() % 40 + RainSpeed + 20) * FPS_ANIMATION_FACTOR), Velocity);
    float Matrix[3][4];
    AngleMatrix(o->Angle, Matrix);
    VectorRotate(Velocity, Matrix, o->Velocity);

    return true;
}

bool CreateLorenciaLeaf(PARTICLE* o)
{
    if (gMapManager.WorldActive != WD_0LORENCIA) return false;

    o->Type = BITMAP_LEAF1;
    vec3_t Position;
    Vector(Hero->Object.Position[0] + (float)(rand() % 1600 - 800), Hero->Object.Position[1] + (float)(rand() % 1400 - 500),
        Hero->Object.Position[2] + (float)(rand() % 300 + 50),
        Position);
    VectorCopy(Position, o->Position);
    VectorCopy(Position, o->StartPosition);
    o->Velocity[0] = -(float)(rand() % 64 + 64) * 0.1f;
    if (Position[1] < CameraPosition[1] + 400.f)
    {
        o->Velocity[0] = -o->Velocity[0] + 3.2f;
    }

    o->Velocity[0] *= FPS_ANIMATION_FACTOR;
    o->Velocity[1] = (float)(rand() % 32 - 16) * 0.1f * FPS_ANIMATION_FACTOR;
    o->Velocity[2] = (float)(rand() % 32 - 16) * 0.1f * FPS_ANIMATION_FACTOR;
    o->TurningForce[0] = (float)(rand() % 16 - 8) * 0.1f;
    o->TurningForce[1] = (float)(rand() % 64 - 32) * 0.1f;
    o->TurningForce[2] = (float)(rand() % 16 - 8) * 0.1f;

    return true;
}

bool CreateHeavenRain(PARTICLE* o, int index)
{
    if (!g_pOption->GetRenderAllEffects())
    {
        return false;
    }

    if (gMapManager.WorldActive != WD_10HEAVEN) return false;

    int Rainly = RainCurrent * MAX_LEAVES / 100;
    if (index < Rainly)
    {
        o->Type = BITMAP_RAIN;
        Vector(
            Hero->Object.Position[0] + (float)(rand() % 1600 - 800),
            Hero->Object.Position[1] + (float)(rand() % 1400 - 500),
            Hero->Object.Position[2] + (float)(rand() % 200 + 200),
            o->Position);
        Vector(-30.f, 0.f, 0.f, o->Angle);
        vec3_t Velocity;
        Vector(0.f, 0.f, -(float)(rand() % 24 + 20), Velocity);
        float Matrix[3][4];
        AngleMatrix(o->Angle, Matrix);
        VectorRotate(Velocity, Matrix, o->Velocity);
    }
    return true;
}

bool CreateDeviasSnow(PARTICLE* o)
{
    if (gMapManager.WorldActive != WD_2DEVIAS) return false;

    o->Type = BITMAP_LEAF1;
    o->Scale = 5.f;
    if (rand_fps_check(10))
    {
        o->Type = BITMAP_LEAF2;
        o->Scale = 10.f;
    }
    Vector(Hero->Object.Position[0] + (float)(rand() % 1600 - 800),
        Hero->Object.Position[1] + (float)(rand() % 1400 - 500),
        Hero->Object.Position[2] + (float)(rand() % 200 + 200),
        o->Position);
    Vector(-30.f, 0.f, 0.f, o->Angle);
    vec3_t Velocity;
    Vector(0.f, 0.f, -(float)(rand() % 16 + 8), Velocity);
    float Matrix[3][4];
    AngleMatrix(o->Angle, Matrix);
    VectorRotate(Velocity, Matrix, o->Velocity);

    return true;
}

bool CreateAtlanseLeaf(PARTICLE* o)
{
    if (gMapManager.WorldActive != WD_3NORIA && gMapManager.WorldActive != WD_7ATLANSE) return false;

    o->Type = BITMAP_LEAF1;
    vec3_t Position;
    Vector(Hero->Object.Position[0] + (float)(rand() % 1600 - 800),
        Hero->Object.Position[1] + (float)(rand() % 1400 - 500),
        Hero->Object.Position[2] + (float)(rand() % 300 + 50),
        Position);
    VectorCopy(Position, o->Position);
    VectorCopy(Position, o->StartPosition);
    o->Velocity[0] = -(float)(rand() % 64 + 64) * 0.1f;
    if (Position[1] < CameraPosition[1] + 400.f) o->Velocity[0] = -o->Velocity[0] + 3.2f;
    o->Velocity[1] = (float)(rand() % 32 - 16) * 0.1f;
    o->Velocity[2] = (float)(rand() % 32 - 16) * 0.1f;
    o->TurningForce[0] = (float)(rand() % 16 - 8) * 0.1f;
    o->TurningForce[1] = (float)(rand() % 64 - 32) * 0.1f;
    o->TurningForce[2] = (float)(rand() % 16 - 8) * 0.1f;

    return true;
}

bool MoveDevilSquareRain(PARTICLE* o)
{
    if (gMapManager.InDevilSquare() == false
        && gMapManager.WorldActive != WD_34CRYWOLF_1ST
        ) return false;

    //	if(o->Type != BITMAP_RAIN)
    if (gMapManager.WorldActive == WD_34CRYWOLF_1ST && weather != 1)
        return false;

    VectorAddScaled(o->Position, o->Velocity, o->Position, FPS_ANIMATION_FACTOR);
    float Height = RequestTerrainHeight(o->Position[0], o->Position[1]);
    if (o->Position[2] < Height)
    {
        o->Live = false;
        o->Position[2] = Height + 10.f;
        if (rand_fps_check(4))
            CreateParticle(BITMAP_RAIN_CIRCLE, o->Position, o->Angle, o->Light);
        else
            CreateParticle(BITMAP_RAIN_CIRCLE + 1, o->Position, o->Angle, o->Light);
    }
    return true;
}

bool MoveChaosCastleRain(PARTICLE* o)
{
    if (gMapManager.InChaosCastle() == false) return false;

    VectorAddScaled(o->Position, o->Velocity, o->Position, FPS_ANIMATION_FACTOR);
    float Height = RequestTerrainHeight(o->Position[0], o->Position[1]);
    if (o->Position[2] < Height && (TERRAIN_ATTRIBUTE(o->Position[0], o->Position[1]) & TW_NOGROUND) != TW_NOGROUND)
    {
        o->Live = false;
        o->Position[2] = Height + 10.f;
        if (rand_fps_check(4))
            CreateParticle(BITMAP_RAIN_CIRCLE, o->Position, o->Angle, o->Light);
        else
            CreateParticle(BITMAP_RAIN_CIRCLE + 1, o->Position, o->Angle, o->Light);
    }
    return true;
}

bool MoveHeavenRain(PARTICLE* o)
{
    if (gMapManager.WorldActive != WD_0LORENCIA && gMapManager.WorldActive != WD_10HEAVEN) return false;

    if (o->Type == BITMAP_RAIN)
    {
        VectorAddScaled(o->Position, o->Velocity, o->Position, FPS_ANIMATION_FACTOR);
        float Height = RequestTerrainHeight(o->Position[0], o->Position[1]);
        if (o->Position[2] < Height)
        {
            o->Live = false;
            o->Position[2] = Height + 10.f;
            if (gMapManager.WorldActive != WD_10HEAVEN)
                CreateParticle(BITMAP_RAIN_CIRCLE, o->Position, o->Angle, o->Light);
        }
    }
    else
    {
        o->Velocity[0] += (float)(rand() % 16 - 8) * 0.1f * FPS_ANIMATION_FACTOR;
        o->Velocity[1] += (float)(rand() % 16 - 8) * 0.1f * FPS_ANIMATION_FACTOR;
        o->Velocity[2] += (float)(rand() % 16 - 8) * 0.1f * FPS_ANIMATION_FACTOR;
        VectorAddScaled(o->Position, o->Velocity, o->Position, FPS_ANIMATION_FACTOR);

        o->TurningForce[0] += (float)(rand() % 8 - 4) * 0.02f * FPS_ANIMATION_FACTOR;
        o->TurningForce[1] += (float)(rand() % 16 - 8) * 0.02f * FPS_ANIMATION_FACTOR;
        o->TurningForce[2] += (float)(rand() % 8 - 4) * 0.02f * FPS_ANIMATION_FACTOR;
        VectorAdd(o->Angle, o->TurningForce, o->Angle);

        vec3_t Range;
        VectorSubtract(o->StartPosition, o->Position, Range);
        float Length = Range[0] * Range[0] + Range[1] * Range[1] + Range[2] * Range[2];
        if (Length >= 200000.f) o->Live = false;
    }

    return true;
}

void MoveEtcLeaf(PARTICLE* o)
{
    float Height = RequestTerrainHeight(o->Position[0], o->Position[1]);
    if (o->Position[2] <= Height)
    {
        o->Position[2] = Height;
        o->Light[0] -= 0.05f * FPS_ANIMATION_FACTOR;
        o->Light[1] -= 0.05f * FPS_ANIMATION_FACTOR;
        o->Light[2] -= 0.05f * FPS_ANIMATION_FACTOR;
        if (o->Light[0] <= 0.f)
            o->Live = false;
    }
    else
    {
        o->Velocity[0] += (float)(rand() % 16 - 8) * 0.1f * FPS_ANIMATION_FACTOR;
        o->Velocity[1] += (float)(rand() % 16 - 8) * 0.1f * FPS_ANIMATION_FACTOR;
        o->Velocity[2] += (float)(rand() % 16 - 8) * 0.1f * FPS_ANIMATION_FACTOR;
        VectorAddScaled(o->Position, o->Velocity, o->Position, FPS_ANIMATION_FACTOR);
    }
}

bool MoveLeaves()
{
    if (!g_pOption->GetRenderAllEffects())
    {
        return false;
    }

    int iMaxLeaves = (gMapManager.InDevilSquare() == true) ? MAX_LEAVES : 80;

    if (gMapManager.WorldActive == WD_10HEAVEN)
    {
        RainTarget = MAX_LEAVES / 2;
    }
    else if (gMapManager.InChaosCastle() == true)
    {
        RainTarget = MAX_LEAVES / 2;
        iMaxLeaves = 80;
    }
    else if (gMapManager.InBattleCastle())
    {
        iMaxLeaves = 40;
    }
    else if (gMapManager.WorldActive == WD_55LOGINSCENE)
    {
        iMaxLeaves = 80;
    }
    else if (M34CryWolf1st::IsCyrWolf1st())
    {
        if (weather == 1)
            iMaxLeaves = 60;
        else
            if (weather == 2)
                iMaxLeaves = 50;
    }
    if (RainCurrent > RainTarget)
        RainCurrent -= FPS_ANIMATION_FACTOR;
    else if (RainCurrent < RainTarget)
        RainCurrent += FPS_ANIMATION_FACTOR;

    RainSpeed = ((int)sinf(WorldTime * 0.001f) * 10 + 30) * FPS_ANIMATION_FACTOR;
    RainAngle = (int)sinf(WorldTime * 0.0005f + 50.f) * 20 * FPS_ANIMATION_FACTOR;
    RainPosition += 20 * FPS_ANIMATION_FACTOR;
    RainPosition %= 2000;

    for (int i = 0; i < iMaxLeaves; i++)
    {
        PARTICLE* o = &Leaves[i];
        if (!o->Live)
        {
            Vector(1.f, 1.f, 1.f, o->Light);
            o->Live = true;

            if (CreateDevilSquareRain(o, i))continue;
            if (CreateChaosCastleRain(o, i))continue;
            if (CreateLorenciaLeaf(o))      continue;
            if (CreateHeavenRain(o, i))     continue;
            if (CreateDeviasSnow(o))        continue;
            if (CreateAtlanseLeaf(o))       continue;
            if (battleCastle::CreateFireSnuff(o)) continue;
            if (M31HuntingGround::CreateMist(o)) continue;
            if (M33Aida::CreateMist(o)) continue;
            if (weather == 2)
                if (M34CryWolf1st::CreateMist(o)) continue;
            if (SEASON3A::CGM3rdChangeUp::Instance().CreateFireSnuff(o)) continue;
            if (g_Raklion.CreateSnow(o)) continue;
            if (g_SantaTown.CreateSnow(o)) continue;
            if (g_PKField.CreateFireSpark(o)) continue;
            if (g_DoppelGanger2.CreateFireSpark(o)) continue;
            if (g_EmpireGuardian1.CreateRain(o)) continue;
            if (g_EmpireGuardian2.CreateRain(o)) continue;
            if (g_EmpireGuardian3.CreateRain(o)) continue;
            if (g_UnitedMarketPlace.CreateRain(o)) continue;
        }
        else
        {
            if (MoveDevilSquareRain(o)) continue;
            if (MoveChaosCastleRain(o)) continue;
            if (MoveHeavenRain(o))      continue;
            if (g_UnitedMarketPlace.MoveRain(o)) continue;
            MoveEtcLeaf(o);
        }
    }
    return true;
}

void RenderLeaves()
{
    if (!g_pOption->GetRenderAllEffects())
    {
        return;
    }

    if (gMapManager.WorldActive == WD_2DEVIAS || gMapManager.WorldActive == WD_7ATLANSE || gMapManager.WorldActive == WD_10HEAVEN
        || IsIceCity()
        || IsSantaTown()
        || gMapManager.IsPKField()
        || IsDoppelGanger2()
        || gMapManager.IsEmpireGuardian1()
        || gMapManager.IsEmpireGuardian2()
        || gMapManager.IsEmpireGuardian3()
        || IsUnitedMarketPlace()
        )
        EnableAlphaBlend();
    else if (gMapManager.InChaosCastle() == true)
        EnableAlphaTest();
    else if (gMapManager.InBattleCastle() == true)
        EnableAlphaBlend();
    else if (gMapManager.WorldActive == WD_55LOGINSCENE)
        EnableAlphaBlend();
    else if (gMapManager.WorldActive == WD_42CHANGEUP3RD_2ND)
        EnableAlphaBlend();
    else
    {
        EnableAlphaTest();
    }

    glColor3f(1.f, 1.f, 1.f);
#ifdef DEVIAS_XMAS_EVENT
    int iMaxLeaves;
    if (World == WD_2DEVIAS)
        iMaxLeaves = MAX_LEAVES_DOUBLE;
    else
        iMaxLeaves = MAX_LEAVES;

    for (int i = 0; i < iMaxLeaves; i++)
#else // DEVIAS_XMAS_EVENT
    for (int i = 0; i < MAX_LEAVES; i++)
#endif // DEVIAS_XMAS_EVENT
    {
        PARTICLE* o = &Leaves[i];
        if (o->Live
            && Bitmaps.FindTexture(o->Type)
            )
        {
            BindTexture(o->Type);
            if (gMapManager.WorldActive == WD_2DEVIAS || IsIceCity() || IsSantaTown())
            {
                RenderSprite(o->Type, o->Position, o->Scale, o->Scale, o->Light);
            }
            else
            {
                glPushMatrix();
                glTranslatef(o->Position[0], o->Position[1], o->Position[2]);
                float Matrix[3][4];
                AngleMatrix(o->Angle, Matrix);

                if (gMapManager.InChaosCastle() == true)
                    RenderPlane3D(o->TurningForce[0], o->TurningForce[1], Matrix);
                else
                {
                    if (o->Type == BITMAP_RAIN)
                    {
                        if (gMapManager.WorldActive == WD_34CRYWOLF_1ST)
                        {
                            if (weather == 1)
                                RenderPlane3D(1.f, 20.f, Matrix);
                        }
                        else
                            RenderPlane3D(1.f, 20.f, Matrix);
                    }
                    else if (o->Type == BITMAP_FIRE_SNUFF)
                        RenderPlane3D(o->Scale * 2.f, o->Scale * 4.f, Matrix);
                    else
                    {
                        RenderPlane3D(3.f, 3.f, Matrix);
                    }
                }
                glPopMatrix();
            }
        }
    }
}