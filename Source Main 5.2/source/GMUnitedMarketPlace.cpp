// GMUnitedMarketPlace.cpp: implementation of the GMUnitedMarketPlace class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ZzzBMD.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzAI.h"
#include "ZzzEffect.h"
#include "w_MapHeaders.h"
#include "DSPlaySound.h"
#include "ZzzOpenData.h"
#include "ZzzLodTerrain.h"
#include "BoneManager.h"
#include "PhysicsManager.h"

#include "GOBoid.h"



GMUnitedMarketPlacePtr GMUnitedMarketPlace::Make()
{
    GMUnitedMarketPlacePtr empire(new GMUnitedMarketPlace);
    empire->Init();
    return empire;
}

GMUnitedMarketPlace::GMUnitedMarketPlace()
{
}

GMUnitedMarketPlace::~GMUnitedMarketPlace()
{
    Destroy();
}

void GMUnitedMarketPlace::Init()
{
}

void GMUnitedMarketPlace::Destroy()
{
}

bool GMUnitedMarketPlace::CreateObject(OBJECT* o)
{
    switch (o->Type)
    {
    case 67:	// 기대기 박스
    {
        CreateOperate(o);
        Vector(100.f, 100.f, 160.f, o->BoundingBoxMax);
        o->HiddenMesh = -2;
    }
    break;
    }

    return false;
}

CHARACTER* GMUnitedMarketPlace::CreateMonster(int iType, int PosX, int PosY, int Key)
{
    CHARACTER* pCharacter = NULL;

    //	switch (iType)
    // 	{
    // 	case 526:
    // 		{
    // 			OpenMonsterModel(MONSTER_MODEL_AEGIS_3);
    // 			pCharacter = CreateCharacter(Key, MODEL_MONSTER01+185, PosX, PosY);
    // 			wcscpy(pCharacter->ID, L"석상");
    // 			pCharacter->Object.m_bRenderShadow = false;
    // 			pCharacter->Object.Scale = 0.6f;
    // 			pCharacter->Object.LifeTime = 100;
    // 		}
    // 		break;
    // 	}

    return pCharacter;
}

bool GMUnitedMarketPlace::MoveObject(OBJECT* o)
{
    if (IsUnitedMarketPlace() == false) return false;

    Alpha(o);
    if (o->Alpha < 0.01f) return false;

    BMD* b = &Models[o->Type];

    switch (o->Type)
    {
    case 8:	// chofountain01 폭포물 표면의 Animation 속도 처리 약간더 빠르게.
    {
        o->Velocity = 0.2f;
    }
    return true;
    case 30:	// 가로등
    {
        VectorCopy(o->Position, b->BodyOrigin);
        b->BodyScale = o->Scale;
        b->Animation(BoneTransform, o->AnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle, false, true);

        vec3_t	vLightPosition, vRelativePos;
        Vector(0.0f, 0.0f, 0.0f, vRelativePos);
        b->TransformPosition(BoneTransform[1], vRelativePos, vLightPosition, false);

        float fLumi = (sinf(WorldTime * 0.002f) + 1.0f) * 0.2f + 0.6f;
        vec3_t vLightFire;
        Vector(fLumi * 0.7f, fLumi * 0.5f, fLumi * 0.2f, vLightFire);

        AddTerrainLight(vLightPosition[0], vLightPosition[1], vLightFire, 3, PrimaryTerrainLight);
    }
    return true;
    case 35:	// 벽가로등
    {
        VectorCopy(o->Position, b->BodyOrigin);
        b->BodyScale = o->Scale;
        b->Animation(BoneTransform, o->AnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle, false, true);

        vec3_t	vLightPosition, vRelativePos;
        Vector(0.0f, 0.0f, 0.0f, vRelativePos);
        b->TransformPosition(BoneTransform[2], vRelativePos, vLightPosition, false);

        float fLumi = (sinf(WorldTime * 0.039f) + 1.0f) * 0.2f + 0.6f;
        vec3_t vLightFire;
        Vector(fLumi * 0.7f, fLumi * 0.5f, fLumi * 0.2f, vLightFire);

        AddTerrainLight(vLightPosition[0], vLightPosition[1], vLightFire, 1, PrimaryTerrainLight);
    }
    return true;
    case 54:
    case 55:
    case 56:
    case 57:
    case 58:
    {
        o->HiddenMesh = -2;
    }
    return true;
    }

    return false;
}

bool GMUnitedMarketPlace::MoveMonsterVisual(OBJECT* o, BMD* b)
{
    if (o == NULL)	return false;
    if (b == NULL)	return false;

    if (IsUnitedMarketPlace() == false)
    {
        return false;
    }

    switch (o->Type)
    {
    case MODEL_UNITEDMARKETPLACE_JULIA:
    {
    }
    return true;
    }

    return false;
}

bool GMUnitedMarketPlace::MoveMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b)
{
    if (IsUnitedMarketPlace() == false)
    {
        return false;
    }

    // 	switch(o->Type)
    // 	{
    // 	case MODEL_MONSTER01+168:	// 데슬러
    // 		{
    // 			switch( o->CurrentAction )
    // 			{
    // 			case MONSTER01_STOP1:
    // 			case MONSTER01_STOP2:
    // 				break;
    // 			case MONSTER01_WALK:
    // 				break;
    // 			case MONSTER01_DIE:
    // 				{
    //
    // 				}
    // 				break;
    // 			case MONSTER01_ATTACK1:
    // 				{
    //
    // 				}
    // 				break;
    // 			}
    // 		}
    // 	break;
    // 	} //switch end

    return false;
}

void GMUnitedMarketPlace::MoveBlurEffect(CHARACTER* c, OBJECT* o, BMD* b)
{
    vec3_t  Light;
    vec3_t StartPos, StartRelative;
    vec3_t EndPos, EndRelative;
    Vector(1.0f, 1.0f, 1.0f, Light);
    Vector(0.0f, 0.0f, 0.0f, StartPos);
    Vector(0.0f, 0.0f, 0.0f, StartRelative);
    Vector(0.0f, 0.0f, 0.0f, EndPos);
    Vector(0.0f, 0.0f, 0.0f, EndRelative);

    // 	switch(o->Type)
    // 	{
    // 		default : return;
    // 	} //switch end
}

bool GMUnitedMarketPlace::RenderObjectMesh(OBJECT* o, BMD* b, bool ExtraMon)
{
    if (IsUnitedMarketPlace() == false)
    {
        return false;
    }

    switch (o->Type)
    {
    case 8:
    case 30:
        o->m_bRenderAfterCharacter = true;
        return true;
    }

    return false;
}

bool GMUnitedMarketPlace::RenderObjectVisual(OBJECT* o, BMD* b)
{
    if (IsUnitedMarketPlace() == false) return false;

    vec3_t p, Position, Light;
    Vector(0.f, 30.f, 0.f, Position);
    Vector(0.f, 0.f, 0.f, p);
    Vector(0.f, 0.f, 0.f, Light);

    switch (o->Type)
    {
    case 30:
    {
        for (int i = 1; i <= 3; ++i)
        {
            vec3_t	vLightPosition, vRelativePos;
            Vector(0.0f, 0.0f, 0.0f, vRelativePos);
            b->TransformPosition(BoneTransform[i], vRelativePos, vLightPosition, false);

            float fLumi = (sinf(WorldTime * 0.002f) + 1.0f) * 0.2f + 0.6f;
            vec3_t vLightFire;
            Vector(fLumi * 0.5f, fLumi * 0.5f, fLumi * 0.5f, vLightFire);
            CreateSprite(BITMAP_FLARE, vLightPosition, 1.5f * o->Scale, vLightFire, o);
            //CreateSprite(BITMAP_LIGHT, vLightPosition, fLumi/2, vLightFire, o);
        }
    }
    return true;
    case 35:
    {
        vec3_t	vLightPosition, vRelativePos;
        Vector(0.0f, 0.0f, 0.0f, vRelativePos);
        b->TransformPosition(BoneTransform[2], vRelativePos, vLightPosition, false);

        float fLumi = (sinf(WorldTime * 0.039f) + 1.0f) * 0.2f + 0.6f;
        vec3_t vLightFire;
        Vector(fLumi * 0.7f, fLumi * 0.7f, fLumi * 0.7f, vLightFire);
        CreateSprite(BITMAP_FLARE, vLightPosition, 1.5f * o->Scale, vLightFire, o);
    }
    return true;
    case 54:
    {
        CreateParticleFpsChecked(BITMAP_WATERFALL_5, o->Position, o->Angle, Light, 0);
    }
    return true;
    case 55:
    {
        Vector(1.f, 1.f, 1.f, Light);
        CreateParticleFpsChecked(BITMAP_WATERFALL_3, o->Position, o->Angle, Light, 8, o->Scale);
    }
    return true;
    case 56:
    {
        Vector(1.f, 1.f, 1.f, Light);
        if (rand_fps_check(8))
        {
            CreateParticle(BITMAP_WATERFALL_2, o->Position, o->Angle, Light, 4, o->Scale);
        }
    }
    return true;
    case 57:
    {
        vec3_t vLightFire;
        Vector(1.0f, 0.2f, 0.0f, vLightFire);
        CreateSprite(BITMAP_LIGHT, o->Position, 2.0f * o->Scale, vLightFire, o);

        vec3_t vLight;
        Vector(1.0f, 1.0f, 1.0f, vLight);

        switch (rand() % 3)
        {
        case 0:
            CreateParticleFpsChecked(BITMAP_FIRE_HIK1, o->Position, o->Angle, vLight, 0, o->Scale);
            break;
        case 1:
            CreateParticleFpsChecked(BITMAP_FIRE_CURSEDLICH, o->Position, o->Angle, vLight, 4, o->Scale);
            break;
        case 2:
            CreateParticleFpsChecked(BITMAP_FIRE_HIK3, o->Position, o->Angle, vLight, 0, o->Scale);
            break;
        }
    }
    return true;
    case 58:
    {
        float fLumi;
        fLumi = (sinf(WorldTime * 0.005f) + 1.0f) * 0.3f + 0.5f;
        vec3_t vLightFire;
        Vector(fLumi * 0.4f, fLumi * 0.4f, fLumi * 0.2f, vLightFire);
        CreateSprite(BITMAP_LIGHT, o->Position, 20.0f * o->Scale, vLightFire, o);
    }
    return true;
    }

    return false;
}

bool GMUnitedMarketPlace::RenderMonster(OBJECT* o, BMD* b, bool ExtraMon)
{
    switch (o->Type)
    {
    case MODEL_STATUE:
    {
        if (o->CurrentAction != MONSTER01_DIE)
        {
            b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        }
    }
    return true;
    }

    return false;
}

bool GMUnitedMarketPlace::RenderMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b)
{
    switch (o->Type)
    {
    case MODEL_UNITEDMARKETPLACE_CHRISTIN:
    {
    }
    return true;
    case MODEL_UNITEDMARKETPLACE_RAUL:
    {
        vec3_t vRelativePos, vWorldPos, Light, vLightPosition;
        Vector(0.f, 0.f, 0.f, vRelativePos);

        Vector(0.8f, 0.8f, 0.8f, Light);
        vec3_t vAngle;
        Vector(10.0f, 0.0f, 0.0f, vAngle);
        b->TransformPosition(o->BoneTransform[43], vRelativePos, vWorldPos, true);
        CreateParticleFpsChecked(BITMAP_SMOKELINE1 + rand() % 3, vWorldPos, o->Angle, Light, 1, 0.6f, o);
        CreateParticleFpsChecked(BITMAP_CLUD64, vWorldPos, o->Angle, Light, 6, 0.6f, o);

        Vector(1.0f, 1.0f, 1.0f, Light);
        Vector(0.0f, 0.0f, 0.0f, vRelativePos);
        b->TransformPosition(o->BoneTransform[43], vRelativePos, vLightPosition, true);
        CreateSprite(BITMAP_FLARE_RED, vLightPosition, 0.24f, Light, o);

        vec3_t	v3Angle;
        Vector(0.0f, 0.0f, 0.0f, v3Angle);
        Vector(0.f, 0.f, 0.f, vRelativePos);
        Vector(1.0f, 0.7f, 0.3f, Light);
        b->TransformPosition(o->BoneTransform[79], vRelativePos, vWorldPos, true);
        CreateSprite(BITMAP_LIGHT, vWorldPos, 0.6f, Light, o);
        CreateSprite(BITMAP_LIGHT, vWorldPos, 0.6f, Light, o);

        Vector(0.0f, 0.0f, 0.0f, vRelativePos);
        b->TransformPosition(o->BoneTransform[78], vRelativePos, vLightPosition, true);

        float fLumi = (sinf(WorldTime * 0.030f) + 1.0f) * 0.4f + 0.6f;
        vec3_t vLightFire;
        Vector(fLumi * 1.0f, fLumi * 0.7f, fLumi * 0.3f, vLightFire);
        CreateSprite(BITMAP_FLARE, vLightPosition, 0.5f * o->Scale, vLightFire, o);

        Vector(1.0f, 1.0f, 1.0f, Light);
        Vector(0.0f, 0.0f, 0.0f, vRelativePos);
        b->TransformPosition(o->BoneTransform[77], vRelativePos, vLightPosition, true);
        CreateSprite(BITMAP_FLARE_RED, vLightPosition, 0.5f, Light, o);

        Vector(1.0f, 0.7f, 0.3f, Light);
        Vector(0.0f, 0.0f, 0.0f, vRelativePos);
        b->TransformPosition(o->BoneTransform[76], vRelativePos, vLightPosition, true);
        CreateSprite(BITMAP_FLARE, vLightPosition, 0.4f, Light, o);
    }
    return true;
    }

    return false;
}

void GMUnitedMarketPlace::RenderAfterObjectMesh(OBJECT* o, BMD* b, bool ExtraMon)
{
    if (IsUnitedMarketPlace() == false) return;

    switch (o->Type)
    {
    case 8:
    {
        b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight);
        b->RenderMesh(3, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight);
        b->RenderMesh(4, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight);
        float fFlow_u = sinf(WorldTime * 0.0007f) * 0.05f;
        float fFlow_v = sinf(WorldTime * 0.001f) * 0.05f;
        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 1, 0.3f, o->BlendMeshTexCoordU, -(int)WorldTime % 10000 * 0.001f);
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, 0.3f, fFlow_u, fFlow_v);
    }
    break;
    case 30:
    {
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
    }
    break;
    }
}

void GMUnitedMarketPlace::RenderFrontSideVisual()
{
}

bool GMUnitedMarketPlace::SetCurrentActionMonster(CHARACTER* c, OBJECT* o)
{
    if (IsUnitedMarketPlace() == false)
    {
        return false;
    }

    switch (c->MonsterIndex)
    {
    case MONSTER_LUCAS:
    {
        // 			switch(c->MonsterSkill)
        // 			{
        // 			default:
        // 				{
        // 					SetAction(o, MONSTER01_ATTACK1);
        // 					c->MonsterSkill = -1;
        // 				}
        // 				break;
        // 			}
    }
    return true;
    }

    return false;
}

bool GMUnitedMarketPlace::AttackEffectMonster(CHARACTER* c, OBJECT* o, BMD* b)
{
    if (IsUnitedMarketPlace() == false)
        return false;

    //  switch(c->MonsterIndex)
    //  {
    //  }

    return false;
}

bool GMUnitedMarketPlace::PlayMonsterSound(OBJECT* o)
{
    if (IsUnitedMarketPlace() == false)
    {
        return false;
    }

    float fDis_x, fDis_y;
    fDis_x = o->Position[0] - Hero->Object.Position[0];
    fDis_y = o->Position[1] - Hero->Object.Position[1];
    float fDistance = sqrtf(fDis_x * fDis_x + fDis_y * fDis_y);

    if (fDistance > 500.0f)
        return true;

    return false;
}

void GMUnitedMarketPlace::PlayObjectSound(OBJECT* o)
{
    if (IsUnitedMarketPlace() == false)
    {
        return;
    }
}

void GMUnitedMarketPlace::PlayBGM()
{
    // 	if (IsUnitedMarketPlace())
    // 	{
    // 		PlayMp3(MUSIC_EMPIREGUARDIAN1);
    // 	}
}

bool GMUnitedMarketPlace::CreateRain(PARTICLE* o)
{
    if (IsUnitedMarketPlace() == false)
    {
        return false;
    }

    o->Type = BITMAP_RAIN;
    Vector(Hero->Object.Position[0] + (float)(rand() % 1600 - 800), Hero->Object.Position[1] + (float)(rand() % 1400 - 500), Hero->Object.Position[2] + (float)(rand() % 200 + 200), o->Position);
    Vector(-30.f, 0.f, 0.f, o->Angle);
    vec3_t Velocity;
    Vector(0.f, 0.f, -(float)(rand() % 24 + 20), Velocity);
    float Matrix[3][4];
    AngleMatrix(o->Angle, Matrix);
    VectorRotate(Velocity, Matrix, o->Velocity);

    //o->Scale = o->Scale * 1.5f;

    return true;
}

bool GMUnitedMarketPlace::MoveRain(PARTICLE* o)
{
    if (IsUnitedMarketPlace() == false)
    {
        return false;
    }

    if (o->Type == BITMAP_RAIN)
    {
        VectorAddScaled(o->Position, o->Velocity, o->Position, FPS_ANIMATION_FACTOR);
        float Height = RequestTerrainHeight(o->Position[0], o->Position[1]);
        if (o->Position[2] < Height)
        {
            o->Live = false;
            o->Position[2] = Height + 10.f;
            if (rand_fps_check(4))
                CreateParticle(BITMAP_RAIN_CIRCLE, o->Position, o->Angle, o->Light, 2);
            else if (rand_fps_check(1))
                CreateParticle(BITMAP_RAIN_CIRCLE + 1, o->Position, o->Angle, o->Light, 2);
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

bool IsUnitedMarketPlace()
{
    if (gMapManager.WorldActive == WD_79UNITEDMARKETPLACE)
    {
        return true;
    }
    return false;
}