// GMSwampOfQuiet.cpp: implementation of the GMSwampOfQuiet class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ZzzOpenglUtil.h"
#include "zzztexture.h"
#include "ZzzBMD.h"
#include "zzzlodterrain.h"
#include "ZzzScene.h"
#include "zzzEffect.h"
#include "zzzAi.h"
#include "zzzOpenData.h"
#include "ZzzCharacter.h"
#include "ZzzObject.h"
#include "DSPlaySound.h"
#include "BoneManager.h"
#include "GOBoid.h"
#include "GIPetManager.h"
#include "MapManager.h"
#include "SkillManager.h"

using namespace SEASON3C;

GMSwampOfQuiet::GMSwampOfQuiet()
{
}

GMSwampOfQuiet::~GMSwampOfQuiet()
{
}

bool GMSwampOfQuiet::IsCurrentMap()
{
    return (gMapManager.WorldActive == WD_56MAP_SWAMP_OF_QUIET);
}

void GMSwampOfQuiet::RenderBaseSmoke()
{
    if (!IsCurrentMap())
        return;

    EnableAlphaTest();
    glColor3f(0.4f, 0.4f, 0.45f);
    float WindX2 = (float)((int)WorldTime % 100000) * 0.0005f;
    RenderBitmapUV(BITMAP_CHROME + 3, 0.f, 0.f, 640.f, 480.f - 45.f, WindX2, 0.f, 3.f, 2.f);
    EnableAlphaBlend();
    float WindX = (float)((int)WorldTime % 100000) * 0.0002f;
    RenderBitmapUV(BITMAP_CHROME + 2, 0.f, 0.f, 640.f, 480.f - 45.f, WindX, 0.f, 0.3f, 0.3f);
}

void GMSwampOfQuiet::CreateObject(OBJECT* pObject)
{
    if (!IsCurrentMap())
        return;

    // 	switch(pObject->Type)
    // 	{
    // 	case 103:	// 의자 설정
    // 		{
    // 			CreateOperate(pObject);
    // 		}
    // 		break;
    // 	}
}

bool GMSwampOfQuiet::MoveObject(OBJECT* pObject)
{
    if (!IsCurrentMap())
        return false;

    PlayObjectSound(pObject);

    float Luminosity;
    vec3_t Light;

    switch (pObject->Type)
    {
    case 57:
        pObject->HiddenMesh = -2;
        Luminosity = (float)(rand() % 4 + 3) * 0.1f;
        Vector(Luminosity, Luminosity * 0.6f, Luminosity * 0.2f, Light);
        AddTerrainLight(pObject->Position[0], pObject->Position[1], Light, 3, PrimaryTerrainLight);
        break;
    case 71:
        pObject->HiddenMesh = -2;
        break;
    case 72:
        pObject->HiddenMesh = -2;
        break;
    case 73:
        pObject->HiddenMesh = -2;
        break;
    case 74:
        pObject->HiddenMesh = -2;
        break;
    case 77:
        pObject->HiddenMesh = -2;
        break;
    case 78:
        pObject->HiddenMesh = -2;
        break;
    }
    return true;
}

void GMSwampOfQuiet::PlayObjectSound(OBJECT* pObject)
{
    // 	float fDis_x, fDis_y;
    // 	fDis_x = pObject->Position[0] - Hero->Object.Position[0];
    // 	fDis_y = pObject->Position[1] - Hero->Object.Position[1];
    // 	float fDistance = sqrtf(fDis_x*fDis_x+fDis_y*fDis_y);

    // 	int Index = TERRAIN_INDEX_REPEAT(( Hero->PositionX),( Hero->PositionY));
    // 	BOOL bSafeZone = FALSE;
    // 	if((TerrainWall[Index]&TW_SAFEZONE)==TW_SAFEZONE)
    // 		bSafeZone = TRUE;

    // 	switch(pObject->Type)
    // 	{
    // 	case 2:		// 마을안 수로
    // 		if (!bSafeZone)
    // 			PlayBuffer(SOUND_ELBELAND_WATERSMALL01, pObject, false);
    // 		break;
    // 	}
}

bool GMSwampOfQuiet::RenderObjectVisual(OBJECT* pObject, BMD* pModel)
{
    if (!IsCurrentMap())
        return false;

    vec3_t Light;

    switch (pObject->Type)
    {
    case 57:
    {
        if (rand_fps_check(3))
        {
            Vector(1.f, 1.f, 1.f, Light);
            CreateParticle(BITMAP_TRUE_FIRE, pObject->Position, pObject->Angle, Light, 5, pObject->Scale);
            CreateParticle(BITMAP_SMOKE, pObject->Position, pObject->Angle, Light, 21, pObject->Scale);
        }
    }
    break;
    case 71:
    {
        if (rand_fps_check(3))
        {
            Vector(1.f, 1.f, 1.f, Light);
            CreateParticle(BITMAP_TRUE_FIRE, pObject->Position, pObject->Angle, Light, 5, pObject->Scale);
        }
    }
    break;
    case 72:
    {
        if (rand_fps_check(6))
        {
            vec3_t Position;
            VectorCopy(pObject->Position, Position);
            Position[2] += 50.0f;

            Vector(0.03f, 0.03f, 0.03f, Light);
            CreateParticle(BITMAP_SMOKE, Position, pObject->Angle, Light, 49, pObject->Scale);
        }
    }
    break;
    case 73:
        break;
    case 74:
    {
        if (rand_fps_check(3))
        {
            Vector(1.f, 1.f, 1.f, Light);
            CreateParticle(BITMAP_SMOKE, pObject->Position, pObject->Angle, Light, 21, pObject->Scale * 2.0f);
        }
    }
    break;
    case 77:
    {
        if (rand_fps_check(6))
        {
            Vector(0.04f, 0.06f, 0.03f, Light);
            CreateParticle(BITMAP_CLOUD, pObject->Position, pObject->Angle, Light, 20, pObject->Scale, pObject);
        }
    }
    break;
    case 78:
    {
        if (rand_fps_check(6))
        {
            Vector(0.03f, 0.03f, 0.05f, Light);
            CreateParticle(BITMAP_CLOUD, pObject->Position, pObject->Angle, Light, 20, pObject->Scale, pObject);
        }
    }
    break;
    }
    return true;
}

bool GMSwampOfQuiet::RenderObject(OBJECT* pObject, BMD* pModel, bool ExtraMon)
{
    if (pObject->Type >= MODEL_SHADOW_PAWN && pObject->Type <= MODEL_SHADOW_LOOK)
    {
        if (pObject->CurrentAction == MONSTER01_DIE)
        {
            float fLumi = 1.0f;
            if (pObject->AnimationFrame > 9.0f)
            {
                fLumi = (12.0f - pObject->AnimationFrame) / 3.0f;
            }

            switch (pObject->Type)
            {
            case MODEL_SHADOW_PAWN:
                Vector(1.0f, 0.2f, 0.2f, pModel->BodyLight);
                pModel->RenderMesh(3, RENDER_TEXTURE | RENDER_BRIGHT, pObject->Alpha, 3, fLumi);
                break;
            case MODEL_SHADOW_KNIGHT:
                Vector(0.5f, 0.8f, 1.0f, pModel->BodyLight);
                pModel->RenderMesh(3, RENDER_TEXTURE | RENDER_BRIGHT, pObject->Alpha, 3, fLumi);
                break;
            case MODEL_SHADOW_LOOK:
                Vector(0.5f, 1.0f, 0.5f, pModel->BodyLight);
                pModel->RenderMesh(4, RENDER_TEXTURE | RENDER_BRIGHT, pObject->Alpha, 4, fLumi);
                break;
            }
        }
        else
        {
            float fLumi = (sinf(WorldTime * 0.002f) + 1.2f) * 0.5f * 1.0f;
            switch (pObject->Type)
            {
            case MODEL_SHADOW_PAWN:
                pModel->RenderMesh(0, RENDER_TEXTURE, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight);
                pModel->RenderMesh(1, RENDER_TEXTURE, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight);
                pModel->RenderMesh(2, RENDER_TEXTURE, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight);
                pModel->RenderMesh(4, RENDER_TEXTURE, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight);
                pModel->RenderMesh(3, RENDER_TEXTURE, 0.7f, pObject->BlendMesh, pObject->BlendMeshLight);
                pModel->RenderMesh(4, RENDER_TEXTURE | RENDER_BRIGHT, pObject->Alpha, 4, fLumi, 0, 0, BITMAP_SHADOW_PAWN_RED);
                pModel->RenderMesh(4, RENDER_TEXTURE | RENDER_BRIGHT, pObject->Alpha, 4, fLumi, 0, 0, BITMAP_SHADOW_PAWN_RED);
                break;
            case MODEL_SHADOW_KNIGHT:
                pModel->RenderMesh(0, RENDER_TEXTURE, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight);
                pModel->RenderMesh(1, RENDER_TEXTURE, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight);
                pModel->RenderMesh(2, RENDER_TEXTURE, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight);
                pModel->RenderMesh(4, RENDER_TEXTURE, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight);
                pModel->RenderMesh(3, RENDER_TEXTURE, 0.7f, pObject->BlendMesh, pObject->BlendMeshLight);
                pModel->RenderMesh(4, RENDER_TEXTURE | RENDER_BRIGHT, pObject->Alpha, 4, fLumi, 0, 0, BITMAP_SHADOW_KINGHT_BLUE);
                pModel->RenderMesh(4, RENDER_TEXTURE | RENDER_BRIGHT, pObject->Alpha, 4, fLumi, 0, 0, BITMAP_SHADOW_KINGHT_BLUE);
                break;
            case MODEL_SHADOW_LOOK:
                pModel->RenderMesh(0, RENDER_TEXTURE, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight);
                pModel->RenderMesh(1, RENDER_TEXTURE, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight);
                pModel->RenderMesh(2, RENDER_TEXTURE, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight);
                pModel->RenderMesh(3, RENDER_TEXTURE, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight);
                pModel->RenderMesh(4, RENDER_TEXTURE, 0.7f, pObject->BlendMesh, pObject->BlendMeshLight);
                pModel->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, pObject->Alpha, 0, fLumi, 0, 0, BITMAP_SHADOW_ROOK_GREEN);
                pModel->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, pObject->Alpha, 0, fLumi, 0, 0, BITMAP_SHADOW_ROOK_GREEN);
                break;
            }
        }
    }
    else
        return false;

    return true;
}

void GMSwampOfQuiet::RenderObjectAfterCharacter(OBJECT* pObject, BMD* pModel, bool ExtraMon0)
{
    if (!IsCurrentMap())
        return;

    // 	if(pObject->Type == 2 || pObject->Type == 53 || pObject->Type == 55 || pObject->Type == 89 || pObject->Type == 125 || pObject->Type == 128)	// 폭포물1,2, 수로, 회오리, 빛
    // 	{
    // 		pModel->RenderBody(RENDER_TEXTURE,pObject->Alpha,pObject->BlendMesh,pObject->BlendMeshLight,pObject->BlendMeshTexCoordU,pObject->BlendMeshTexCoordV,pObject->HiddenMesh);
    // 	}
}

CHARACTER* GMSwampOfQuiet::CreateSwampOfQuietMonster(int iType, int PosX, int PosY, int Key)
{
    CHARACTER* pCharacter = NULL;

    switch (iType)
    {
    case MONSTER_SAPIUNUS:
        OpenMonsterModel(MONSTER_MODEL_SAPIUNUS);
        pCharacter = CreateCharacter(Key, MODEL_SAPIUNUS, PosX, PosY);
        pCharacter->Object.Scale = 1.0f;
        pCharacter->Weapon[0].Type = -1;
        pCharacter->Weapon[1].Type = -1;
        // 		{
        // 	  		BMD *b = &Models[MODEL_MONSTER01+136];
        // // 			b->Actions[MONSTER01_STOP1].PlaySpeed = 0.25f;
        // // 			b->Actions[MONSTER01_STOP2].PlaySpeed = 0.25f;
        // // 			b->Actions[MONSTER01_WALK ].PlaySpeed = 0.25f;
        // // 			b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.25f;
        // // 			b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.25f;
        // // 			b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.25f;
        // 			b->Actions[MONSTER01_DIE  ].PlaySpeed = 0.25f;
        // 		}
        break;
    case MONSTER_SAPIDUO:
        OpenMonsterModel(MONSTER_MODEL_SAPIDUO);
        pCharacter = CreateCharacter(Key, MODEL_SAPIDUO, PosX, PosY);
        pCharacter->Object.Scale = 1.0f;
        pCharacter->Weapon[0].Type = -1;
        pCharacter->Weapon[1].Type = -1;
        // 		{
        // 	  		BMD *b = &Models[MODEL_MONSTER01+137];
        // // 			b->Actions[MONSTER01_STOP1].PlaySpeed = 0.25f;
        // // 			b->Actions[MONSTER01_STOP2].PlaySpeed = 0.25f;
        // // 			b->Actions[MONSTER01_WALK ].PlaySpeed = 0.25f;
        // // 			b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.25f;
        // // 			b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.25f;
        // // 			b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.25f;
        // 			b->Actions[MONSTER01_DIE  ].PlaySpeed = 0.25f;
        // 		}
        break;
    case MONSTER_SAPITRES:
        OpenMonsterModel(MONSTER_MODEL_SAPITRES);
        pCharacter = CreateCharacter(Key, MODEL_SAPITRES, PosX, PosY);
        pCharacter->Object.Scale = 1.0f;
        pCharacter->Weapon[0].Type = -1;
        pCharacter->Weapon[1].Type = -1;
        // 		{
        // 	  		BMD *b = &Models[MODEL_MONSTER01+138];
        // // 			b->Actions[MONSTER01_STOP1].PlaySpeed = 0.25f;
        // // 			b->Actions[MONSTER01_STOP2].PlaySpeed = 0.25f;
        // // 			b->Actions[MONSTER01_WALK ].PlaySpeed = 0.25f;
        // // 			b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.25f;
        // // 			b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.25f;
        // // 			b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.25f;
        // 			b->Actions[MONSTER01_DIE  ].PlaySpeed = 0.25f;
        // 		}
        break;
    case MONSTER_SHADOW_PAWN:
        OpenMonsterModel(MONSTER_MODEL_SHADOW_PAWN);
        pCharacter = CreateCharacter(Key, MODEL_SHADOW_PAWN, PosX, PosY);
        pCharacter->Object.Scale = 1.1f;
        pCharacter->Weapon[0].Type = -1;
        pCharacter->Weapon[1].Type = -1;
        pCharacter->Object.LifeTime = 100;
        break;
    case MONSTER_SHADOW_KNIGHT:
        OpenMonsterModel(MONSTER_MODEL_SHADOW_KNIGHT);
        pCharacter = CreateCharacter(Key, MODEL_SHADOW_KNIGHT, PosX, PosY);
        pCharacter->Object.Scale = 1.1f;
        pCharacter->Weapon[0].Type = -1;
        pCharacter->Weapon[1].Type = -1;
        pCharacter->Object.LifeTime = 100;
        break;
    case MONSTER_SHADOW_LOOK:
        OpenMonsterModel(MONSTER_MODEL_SHADOW_LOOK);
        pCharacter = CreateCharacter(Key, MODEL_SHADOW_LOOK, PosX, PosY);
        pCharacter->Object.Scale = 1.3f;
        pCharacter->Weapon[0].Type = -1;
        pCharacter->Weapon[1].Type = -1;
        pCharacter->Object.LifeTime = 100;
        break;
    case MONSTER_THUNDER_NAPIN:
        OpenMonsterModel(MONSTER_MODEL_NAPIN);
        pCharacter = CreateCharacter(Key, MODEL_NAPIN, PosX, PosY);
        pCharacter->Object.Scale = 0.95f;
        pCharacter->Weapon[0].Type = -1;
        pCharacter->Weapon[1].Type = -1;
        break;
    case MONSTER_GHOST_NAPIN:
        OpenMonsterModel(MONSTER_MODEL_GHOST_NAPIN);
        pCharacter = CreateCharacter(Key, MODEL_GHOST_NAPIN, PosX, PosY);
        pCharacter->Object.Scale = 0.95f;
        pCharacter->Weapon[0].Type = -1;
        pCharacter->Weapon[1].Type = -1;
        break;
    case MONSTER_BLAZE_NAPIN:
        OpenMonsterModel(MONSTER_MODEL_BLAZE_NAPIN);
        pCharacter = CreateCharacter(Key, MODEL_BLAZE_NAPIN, PosX, PosY);
        pCharacter->Object.Scale = 0.95f;
        pCharacter->Weapon[0].Type = -1;
        pCharacter->Weapon[1].Type = -1;
        break;
    case MONSTER_MEDUSA:
        OpenMonsterModel(MONSTER_MODEL_MEDUSA);
        pCharacter = CreateCharacter(Key, MODEL_MEDUSA, PosX, PosY);
        pCharacter->Object.Scale = 1.5f;
        pCharacter->Object.LifeTime = 100;
        break;
    case MONSTER_SAPI_QUEEN:
    case MONSTER_SAPI_QUEEN2:
        OpenMonsterModel(MONSTER_MODEL_SAPI_QUEEN);
        pCharacter = CreateCharacter(Key, MODEL_SAPI_QUEEN, PosX, PosY);
        pCharacter->Object.Scale = 1.5f;
        pCharacter->Weapon[0].Type = -1;
        pCharacter->Weapon[1].Type = -1;
        break;
    case MONSTER_ICE_NAPIN:
        OpenMonsterModel(MONSTER_MODEL_ICE_NAPIN);
        pCharacter = CreateCharacter(Key, MODEL_ICE_NAPIN, PosX, PosY);
        pCharacter->Object.Scale = 1.1f;
        pCharacter->Weapon[0].Type = -1;
        pCharacter->Weapon[1].Type = -1;
        break;
    case MONSTER_SHADOW_MASTER:
        OpenMonsterModel(MONSTER_MODEL_SHADOW_MASTER);
        pCharacter = CreateCharacter(Key, MODEL_SHADOW_MASTER, PosX, PosY);
        pCharacter->Object.Scale = 1.56f;
        pCharacter->Weapon[0].Type = -1;
        pCharacter->Weapon[1].Type = -1;
        pCharacter->Object.LifeTime = 100;
        break;
    }

    return pCharacter;
}

bool GMSwampOfQuiet::MoveMonsterVisual(OBJECT* pObject, BMD* pModel)
{
    if (!IsCurrentMap())
        return false;

    switch (pObject->Type)
    {
    case MODEL_NAPIN:
        if (pObject->CurrentAction == MONSTER01_ATTACK2)
        {
            BMD* pModel = &Models[pObject->Type];
            float fActionSpeed = pModel->Actions[pObject->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);
            float Start_Frame = 7.2f;
            float End_Frame = Start_Frame + fActionSpeed;
            if (pObject->AnimationFrame >= Start_Frame && pObject->AnimationFrame < End_Frame)
            {
                vec3_t vLook, vPosition, vLight;
                float matRotate[3][4];
                Vector(1.f, 1.f, 1.f, vLight);
                Vector(0.0f, -250.0f, 0.0f, vPosition);
                AngleMatrix(pObject->Angle, matRotate);
                VectorRotate(vPosition, matRotate, vLook);
                VectorAdd(pObject->Position, vLook, vPosition);

                CreateEffectFpsChecked(BITMAP_CRATER, vPosition, pObject->Angle, vLight, 2, NULL, -1, 0, 0, 0, 1.5f);
                for (int iu = 0; iu < 20; iu++)
                {
                    //CreateEffect ( MODEL_BIG_STONE1, vPosition,pObject->Angle,pObject->Light,10);
                    CreateEffectFpsChecked(MODEL_STONE2, vPosition, pObject->Angle, pObject->Light);
                }
                Vector(0.7f, 0.7f, 1.f, vLight);
                CreateParticleFpsChecked(BITMAP_CLUD64, vPosition, pObject->Angle, vLight, 7, 2.0f);
                CreateParticleFpsChecked(BITMAP_CLUD64, vPosition, pObject->Angle, vLight, 7, 2.0f);

                Vector(0.3f, 0.2f, 1.f, vLight);
                CreateEffectFpsChecked(BITMAP_SHOCK_WAVE, vPosition, pObject->Angle, vLight, 11);
                CreateEffectFpsChecked(BITMAP_SHOCK_WAVE, vPosition, pObject->Angle, vLight, 11);

                vPosition[2] += 100.0f;
                Vector(0.0f, 0.2f, 1.0f, vLight);
                CreateEffectFpsChecked(MODEL_EFFECT_THUNDER_NAPIN_ATTACK_1, vPosition, pObject->Angle, vLight, 0);
            }
        }
        break;
    case MODEL_GHOST_NAPIN:
        if (pObject->CurrentAction == MONSTER01_ATTACK1 || pObject->CurrentAction == MONSTER01_ATTACK2)
        {
            float Start_Frame = 7.0f;
            float End_Frame = Start_Frame + 0.8f;
            if (pObject->AnimationFrame >= Start_Frame)
            {
                vec3_t vLook, vPosition, vLight;
                float matRotate[3][4];
                Vector(0.4f, 1.0f, 0.4f, vLight);
                Vector(0.0f, -150.0f, 100.0f, vPosition);
                AngleMatrix(pObject->Angle, matRotate);
                VectorRotate(vPosition, matRotate, vLook);
                VectorAdd(pObject->Position, vLook, vPosition);

                vec3_t vSmokePosition;
                for (int i = 0; i < 2; ++i)
                {
                    Vector(vPosition[0] + (rand() % 20 - 10) * 1.0f,
                        vPosition[1] + (rand() % 20 - 10) * 1.0f, vPosition[2] + (rand() % 20 - 10) * 1.0f, vSmokePosition);
                    CreateParticleFpsChecked(BITMAP_SMOKE, vSmokePosition, pObject->Angle, vLight, 51);
                }
                if (pObject->AnimationFrame < End_Frame)
                {
                    Vector(4.0f, 10.0f, 4.0f, vLight);
                    CreateParticleFpsChecked(BITMAP_SHOCK_WAVE, vPosition, pObject->Angle, vLight, 3, 0.5f);
                    CreateParticleFpsChecked(BITMAP_SHOCK_WAVE, vPosition, pObject->Angle, vLight, 3, 0.8f);
                }

                Vector(0.0f, -100.0f, 100.0f, vPosition);
                AngleMatrix(pObject->Angle, matRotate);
                VectorRotate(vPosition, matRotate, vLook);
                VectorAdd(pObject->Position, vLook, vPosition);
                CreateJointFpsChecked(BITMAP_JOINT_ENERGY, vPosition, pObject->Position, pObject->Angle, 46, pObject, 20.0f);
            }
        }
        break;
    case MODEL_BLAZE_NAPIN:
        if (pObject->CurrentAction == MONSTER01_ATTACK1 || pObject->CurrentAction == MONSTER01_ATTACK2)
        {
            BMD* pModel = &Models[pObject->Type];
            float fActionSpeed = pModel->Actions[pObject->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);
            float Start_Frame = 5.5f;
            float End_Frame = Start_Frame + fActionSpeed;
            if (pObject->AnimationFrame >= Start_Frame && pObject->AnimationFrame < End_Frame)
            {
                vec3_t vLook, vPosition, vLight, /*vFirePosition, */vLightFire;
                float matRotate[3][4];
                Vector(1.f, 1.f, 1.f, vLight);
                Vector(1.0f, 0.2f, 0.0f, vLightFire);
                Vector(0.0f, -150.0f, 0.0f, vPosition);
                AngleMatrix(pObject->Angle, matRotate);
                VectorRotate(vPosition, matRotate, vLook);
                VectorAdd(pObject->Position, vLook, vPosition);

                CreateEffectFpsChecked(BITMAP_FIRE_CURSEDLICH, vPosition, pObject->Angle, vLight, 1, pObject);

                CreateEffectFpsChecked(BITMAP_CRATER, vPosition, pObject->Angle, vLight, 2, NULL, -1, 0, 0, 0, 1.5f);
                for (int iu = 0; iu < 20; iu++)
                {
                    CreateEffectFpsChecked(MODEL_STONE2, vPosition, pObject->Angle, pObject->Light);
                }

                Vector(0.5f, 0.1f, 0.0f, vLight);
                CreateEffectFpsChecked(BITMAP_SHOCK_WAVE, vPosition, pObject->Angle, vLight, 12, NULL, -1, 0, 0, 0, 0.1f);
            }
        }
        break;
    case MODEL_MEDUSA:
    {
        vec3_t vPos, vColor;

        switch (pObject->CurrentAction)
        {
        case MONSTER01_WALK:
            Vector(pObject->Position[0] + rand() % 200 - 100, pObject->Position[1] + rand() % 200 - 100, pObject->Position[2], vPos);
            CreateParticleFpsChecked(BITMAP_SMOKE + 1, vPos, pObject->Angle, pObject->Light);
            break;

        case MONSTER01_ATTACK1:
        case MONSTER01_ATTACK2:
            if (2 < pModel->CurrentAnimationFrame)
            {
                vec3_t vAngle, vRel;
                VectorCopy(pObject->Angle, vAngle);
                Vector(10.0f, 5.0f, 0.0f, vRel);

                int temp[] = { 19, 31 };
                for (int i = 0; i < 2; i++)
                {
                    pModel->TransformByObjectBone(vPos, pObject, temp[i], vRel);

                    Vector(0.0f, 1.0f, 0.5f, vColor);
                    for (int i = 0; i < 2; ++i)
                    {
                        if (i == 1 && rand_fps_check(2)) continue;

                        switch (rand() % 3)
                        {
                        case 0:
                            CreateParticleFpsChecked(BITMAP_FIRE_HIK1_MONO, vPos, vAngle, vColor, 4, pObject->Scale, pObject);
                            break;
                        case 1:
                            CreateParticleFpsChecked(BITMAP_FIRE_HIK2_MONO, vPos, vAngle, vColor, 8, pObject->Scale, pObject);
                            break;
                        case 2:
                            CreateParticleFpsChecked(BITMAP_FIRE_HIK3_MONO, vPos, vAngle, vColor, 5, pObject->Scale, pObject);
                            break;
                        }
                    }

                    Vector(1.0f, 1.0f, 1.0f, vColor);
                    CreateSprite(BITMAP_HOLE, vPos, (sinf(WorldTime * 0.005f) + 1.0f) * 0.1f + 0.1f, vColor, pObject);
                }
            }
            break;
        }
    }
    break;
    case MODEL_ICE_NAPIN:
        if (pObject->CurrentAction == MONSTER01_ATTACK1 || pObject->CurrentAction == MONSTER01_ATTACK2)
        {
            BMD* pModel = &Models[pObject->Type];
            float fActionSpeed = pModel->Actions[pObject->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);
            float Start_Frame = 5.5f;
            float End_Frame = Start_Frame + fActionSpeed;
            if (pObject->AnimationFrame >= Start_Frame && pObject->AnimationFrame < End_Frame)
            {
                vec3_t vLook, vPosition, vLight, /*vFirePosition, */vLightFire;
                float matRotate[3][4];
                Vector(0.25f, 0.6f, 0.7f, vLight);
                Vector(1.0f, 1.0f, 1.0f, vLightFire);
                Vector(0.0f, -150.0f, 0.0f, vPosition);
                AngleMatrix(pObject->Angle, matRotate);
                VectorRotate(vPosition, matRotate, vLook);
                VectorAdd(pObject->Position, vLook, vPosition);

                CreateEffectFpsChecked(BITMAP_FIRE_CURSEDLICH, vPosition, pObject->Angle, vLight, 1, pObject);

                CreateEffectFpsChecked(BITMAP_CRATER, vPosition, pObject->Angle, vLight, 2, NULL, -1, 0, 0, 0, 1.5f);
                for (int iu = 0; iu < 20; iu++)
                {
                    CreateEffectFpsChecked(MODEL_STONE2, vPosition, pObject->Angle, pObject->Light);
                }

                Vector(0.5f, 0.1f, 0.0f, vLight);
                CreateEffectFpsChecked(BITMAP_SHOCK_WAVE, vPosition, pObject->Angle, vLight, 12, NULL, -1, 0, 0, 0, 0.1f);
            }
        }
        break;
    }
    return false;
}

void GMSwampOfQuiet::MoveBlurEffect(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel)
{
    switch (pObject->Type)
    {
    case MODEL_SAPIUNUS:
    {
        float Start_Frame = 6.f;
        float End_Frame = 7.6f;
        if ((pObject->AnimationFrame >= Start_Frame && pObject->AnimationFrame <= End_Frame && pObject->CurrentAction == MONSTER01_ATTACK1)
            || (pObject->AnimationFrame >= Start_Frame && pObject->AnimationFrame <= End_Frame && pObject->CurrentAction == MONSTER01_ATTACK2))
        {
            vec3_t  vLight;
            Vector(1.0f, 0.6f, 0.1f, vLight);

            vec3_t StartPos, StartRelative;
            vec3_t EndPos, EndRelative;

            float fActionSpeed = pModel->Actions[pObject->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);
            float fSpeedPerFrame = fActionSpeed / 10.f;
            float fAnimationFrame = pObject->AnimationFrame - fActionSpeed;
            for (int i = 0; i < 10; i++)
            {
                pModel->Animation(BoneTransform, fAnimationFrame, pObject->PriorAnimationFrame, pObject->PriorAction, pObject->Angle, pObject->HeadAngle);

                Vector(0.f, 0.f, 0.f, StartRelative);
                Vector(0.f, 0.f, 0.f, EndRelative);
                pModel->TransformPosition(BoneTransform[42], StartRelative, StartPos, false);

                pModel->TransformPosition(BoneTransform[34], EndRelative, EndPos, false);
                CreateBlur(pCharacter, StartPos, EndPos, vLight, 0, false, 0);
                pModel->TransformPosition(BoneTransform[35], EndRelative, EndPos, false);
                CreateBlur(pCharacter, StartPos, EndPos, vLight, 0, false, 1);
                pModel->TransformPosition(BoneTransform[38], EndRelative, EndPos, false);
                CreateBlur(pCharacter, StartPos, EndPos, vLight, 0, false, 2);
                pModel->TransformPosition(BoneTransform[39], EndRelative, EndPos, false);
                CreateBlur(pCharacter, StartPos, EndPos, vLight, 0, false, 3);
                pModel->TransformPosition(BoneTransform[43], EndRelative, EndPos, false);
                CreateBlur(pCharacter, StartPos, EndPos, vLight, 0, false, 4);

                fAnimationFrame += fSpeedPerFrame;
            }
        }
    }
    break;
    case MODEL_SAPIDUO:
    {
        float Start_Frame = 6.f;
        float End_Frame = 7.f;
        if ((pObject->AnimationFrame >= Start_Frame && pObject->AnimationFrame <= End_Frame && pObject->CurrentAction == MONSTER01_ATTACK1)
            || (pObject->AnimationFrame >= Start_Frame && pObject->AnimationFrame <= End_Frame && pObject->CurrentAction == MONSTER01_ATTACK2))
        {
            vec3_t  vLight;
            Vector(2.0f, 0.0f, 0.0f, vLight);

            vec3_t StartPos, StartRelative;
            vec3_t EndPos, EndRelative;

            float fActionSpeed = pModel->Actions[pObject->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);
            float fSpeedPerFrame = fActionSpeed / 10.f;
            float fAnimationFrame = pObject->AnimationFrame - fActionSpeed;
            for (int i = 0; i < 10; i++)
            {
                pModel->Animation(BoneTransform, fAnimationFrame, pObject->PriorAnimationFrame, pObject->PriorAction, pObject->Angle, pObject->HeadAngle);

                Vector(0.f, 0.f, 0.f, StartRelative);
                Vector(0.f, 0.f, 0.f, EndRelative);
                pModel->TransformPosition(BoneTransform[42], StartRelative, StartPos, false);

                pModel->TransformPosition(BoneTransform[34], EndRelative, EndPos, false);
                CreateBlur(pCharacter, StartPos, EndPos, vLight, 0, false, 0);
                pModel->TransformPosition(BoneTransform[35], EndRelative, EndPos, false);
                CreateBlur(pCharacter, StartPos, EndPos, vLight, 0, false, 1);
                pModel->TransformPosition(BoneTransform[38], EndRelative, EndPos, false);
                CreateBlur(pCharacter, StartPos, EndPos, vLight, 0, false, 2);
                pModel->TransformPosition(BoneTransform[39], EndRelative, EndPos, false);
                CreateBlur(pCharacter, StartPos, EndPos, vLight, 0, false, 3);
                pModel->TransformPosition(BoneTransform[43], EndRelative, EndPos, false);
                CreateBlur(pCharacter, StartPos, EndPos, vLight, 0, false, 4);

                fAnimationFrame += fSpeedPerFrame;
            }
        }
    }
    break;
    case MODEL_SAPITRES:
    {
        float Start_Frame = 5.f;
        float End_Frame = 10.f;
        if ((pObject->AnimationFrame >= Start_Frame && pObject->AnimationFrame <= End_Frame && pObject->CurrentAction == MONSTER01_ATTACK1)
            || (pObject->AnimationFrame >= Start_Frame && pObject->AnimationFrame <= End_Frame && pObject->CurrentAction == MONSTER01_ATTACK2))
        {
            vec3_t  vLight;
            Vector(0.3f, 0.7f, 1.0f, vLight);

            vec3_t StartPos, StartRelative;
            vec3_t EndPos, EndRelative;

            float fActionSpeed = pModel->Actions[pObject->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);
            float fSpeedPerFrame = fActionSpeed / 10.f;
            float fAnimationFrame = pObject->AnimationFrame - fActionSpeed;
            for (int i = 0; i < 10; i++)
            {
                pModel->Animation(BoneTransform, fAnimationFrame, pObject->PriorAnimationFrame, pObject->PriorAction, pObject->Angle, pObject->HeadAngle);
                Vector(0.f, 0.f, 0.f, StartRelative);
                Vector(0.f, 0.f, 0.f, EndRelative);

                // 왼손
                pModel->TransformPosition(BoneTransform[42], StartRelative, StartPos, false);

                pModel->TransformByBoneMatrix(EndPos, BoneTransform[34]);
                //					pModel->TransformPosition(BoneTransform[34], EndRelative, EndPos, false);
                CreateBlur(pCharacter, StartPos, EndPos, vLight, 0, false, 0);
                pModel->TransformPosition(BoneTransform[35], EndRelative, EndPos, false);
                CreateBlur(pCharacter, StartPos, EndPos, vLight, 0, false, 1);
                pModel->TransformPosition(BoneTransform[38], EndRelative, EndPos, false);
                CreateBlur(pCharacter, StartPos, EndPos, vLight, 0, false, 2);
                pModel->TransformPosition(BoneTransform[39], EndRelative, EndPos, false);
                CreateBlur(pCharacter, StartPos, EndPos, vLight, 0, false, 3);
                pModel->TransformPosition(BoneTransform[43], EndRelative, EndPos, false);
                CreateBlur(pCharacter, StartPos, EndPos, vLight, 0, false, 4);

                pModel->TransformPosition(BoneTransform[75], StartRelative, StartPos, false);

                pModel->TransformPosition(BoneTransform[76], EndRelative, EndPos, false);
                CreateBlur(pCharacter, StartPos, EndPos, vLight, 0, false, 5);
                pModel->TransformPosition(BoneTransform[79], EndRelative, EndPos, false);
                CreateBlur(pCharacter, StartPos, EndPos, vLight, 0, false, 6);
                pModel->TransformPosition(BoneTransform[80], EndRelative, EndPos, false);
                CreateBlur(pCharacter, StartPos, EndPos, vLight, 0, false, 7);
                pModel->TransformPosition(BoneTransform[83], EndRelative, EndPos, false);
                CreateBlur(pCharacter, StartPos, EndPos, vLight, 0, false, 8);
                pModel->TransformPosition(BoneTransform[84], EndRelative, EndPos, false);
                CreateBlur(pCharacter, StartPos, EndPos, vLight, 0, false, 9);

                fAnimationFrame += fSpeedPerFrame;
            }
        }
    }
    break;
    case MODEL_NAPIN:
        break;
    case MODEL_SAPI_QUEEN:
    case MODEL_WOLF_STATUS:
    {
        float Start_Frame = 6.f;
        float End_Frame = 7.6f;
        if ((pObject->AnimationFrame >= Start_Frame && pObject->AnimationFrame <= End_Frame && pObject->CurrentAction == MONSTER01_ATTACK1)
            || (pObject->AnimationFrame >= Start_Frame && pObject->AnimationFrame <= End_Frame && pObject->CurrentAction == MONSTER01_ATTACK2))
        {
            vec3_t  vLight;
            Vector(1.0f, 0.6f, 0.1f, vLight);

            vec3_t StartPos, StartRelative;
            vec3_t EndPos, EndRelative;

            float fActionSpeed = pModel->Actions[pObject->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);
            float fSpeedPerFrame = fActionSpeed / 10.f;
            float fAnimationFrame = pObject->AnimationFrame - fActionSpeed;
            for (int i = 0; i < 10; i++)
            {
                pModel->Animation(BoneTransform, fAnimationFrame, pObject->PriorAnimationFrame, pObject->PriorAction, pObject->Angle, pObject->HeadAngle);

                Vector(0.f, 0.f, 0.f, StartRelative);
                Vector(0.f, 0.f, 0.f, EndRelative);
                pModel->TransformPosition(BoneTransform[42], StartRelative, StartPos, false);

                pModel->TransformPosition(BoneTransform[34], EndRelative, EndPos, false);
                CreateBlur(pCharacter, StartPos, EndPos, vLight, 0, false, 0);
                pModel->TransformPosition(BoneTransform[35], EndRelative, EndPos, false);
                CreateBlur(pCharacter, StartPos, EndPos, vLight, 0, false, 1);
                pModel->TransformPosition(BoneTransform[38], EndRelative, EndPos, false);
                CreateBlur(pCharacter, StartPos, EndPos, vLight, 0, false, 2);
                pModel->TransformPosition(BoneTransform[39], EndRelative, EndPos, false);
                CreateBlur(pCharacter, StartPos, EndPos, vLight, 0, false, 3);
                pModel->TransformPosition(BoneTransform[43], EndRelative, EndPos, false);
                CreateBlur(pCharacter, StartPos, EndPos, vLight, 0, false, 4);

                fAnimationFrame += fSpeedPerFrame;
            }
        }
    }
    break;
    }
}

bool GMSwampOfQuiet::RenderMonsterVisual(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel)
{
    if (!IsCurrentMap())
        return false;

    vec3_t vPos, vRelative, vLight;

    switch (pObject->Type)
    {
    case MODEL_SAPIUNUS:
    {
        Vector(1.0f, 0.6f, 0.1f, vLight);
        Vector(0.0f, 0.0f, 0.0f, vRelative);
        pModel->TransformPosition(pObject->BoneTransform[15], vRelative, vPos, true);

        CreateSprite(BITMAP_LIGHT, vPos, 2.0f, vLight, pObject);
        CreateSprite(BITMAP_LIGHT, vPos, 2.0f, vLight, pObject);
    }
    break;
    case MODEL_SAPIDUO:
    {
        Vector(1.0f, 0.0f, 0.0f, vLight);
        Vector(0.0f, 0.0f, 0.0f, vRelative);
        pModel->TransformPosition(pObject->BoneTransform[15], vRelative, vPos, true);

        CreateSprite(BITMAP_LIGHT, vPos, 2.0f, vLight, pObject);
        CreateSprite(BITMAP_LIGHT, vPos, 2.0f, vLight, pObject);
    }
    break;
    case MODEL_SAPITRES:
    {
        Vector(0.2f, 0.7f, 1.0f, vLight);
        Vector(0.0f, 0.0f, 0.0f, vRelative);
        pModel->TransformPosition(pObject->BoneTransform[15], vRelative, vPos, true);

        CreateSprite(BITMAP_LIGHT, vPos, 2.0f, vLight, pObject);
        CreateSprite(BITMAP_LIGHT, vPos, 2.0f, vLight, pObject);
    }
    break;
    case MODEL_SHADOW_PAWN:
    {
        int iBones[] = { 11, 15, 34, 21, 25, 39 };
        Vector(1.0f, 0.1f, 0.1f, vLight);
        for (int i = 0; i < 6; ++i)
        {
            if (rand() % 6 > 0) continue;
            pModel->TransformByObjectBone(vPos, pObject, iBones[i]);
            CreateParticleFpsChecked(BITMAP_SMOKE, vPos, pObject->Angle, vLight, 50, 1.0f);
            CreateParticleFpsChecked(BITMAP_SMOKELINE1 + rand() % 3, vPos, pObject->Angle, vLight, 0, 1.0f);
        }
    }
    if (pObject->CurrentAction == MONSTER01_DIE)
    {
        if (pObject->LifeTime == 100)
        {
            pObject->LifeTime = 90;
            pObject->m_bRenderShadow = false;

            vec3_t vWorldPos, Light;
            Vector(1.0f, 1.0f, 1.0f, Light);

            pModel->TransformByObjectBone(vWorldPos, pObject, 34);
            CreateEffect(MODEL_SHADOW_PAWN_ANKLE_LEFT, vWorldPos, pObject->Angle, Light, 0, pObject, 0, 0);
            pModel->TransformByObjectBone(vWorldPos, pObject, 39);
            CreateEffect(MODEL_SHADOW_PAWN_ANKLE_RIGHT, vWorldPos, pObject->Angle, Light, 0, pObject, 0, 0);
            pModel->TransformByObjectBone(vWorldPos, pObject, 0);
            CreateEffect(MODEL_SHADOW_PAWN_BELT, vWorldPos, pObject->Angle, Light, 0, pObject, 0, 0);
            pModel->TransformByObjectBone(vWorldPos, pObject, 4);
            CreateEffect(MODEL_SHADOW_PAWN_CHEST, vWorldPos, pObject->Angle, Light, 0, pObject, 0, 0);
            pModel->TransformByObjectBone(vWorldPos, pObject, 6);
            CreateEffect(MODEL_SHADOW_PAWN_HELMET, vWorldPos, pObject->Angle, Light, 0, pObject, 0, 0);
            pModel->TransformByObjectBone(vWorldPos, pObject, 33);
            CreateEffect(MODEL_SHADOW_PAWN_KNEE_LEFT, vWorldPos, pObject->Angle, Light, 0, pObject, 0, 0);
            pModel->TransformByObjectBone(vWorldPos, pObject, 38);
            CreateEffect(MODEL_SHADOW_PAWN_KNEE_RIGHT, vWorldPos, pObject->Angle, Light, 0, pObject, 0, 0);
            pModel->TransformByObjectBone(vWorldPos, pObject, 13);
            CreateEffect(MODEL_SHADOW_PAWN_WRIST_LEFT, vWorldPos, pObject->Angle, Light, 0, pObject, 0, 0);
            pModel->TransformByObjectBone(vWorldPos, pObject, 23);
            CreateEffect(MODEL_SHADOW_PAWN_WRIST_RIGHT, vWorldPos, pObject->Angle, Light, 0, pObject, 0, 0);
        }
    }
    break;
    case MODEL_SHADOW_KNIGHT:
    {
        int iBones[] = { 11, 15, 34, 21, 25, 39 };
        Vector(0.3f, 0.6f, 1.0f, vLight);
        for (int i = 0; i < 6; ++i)
        {
            if (rand() % 6 > 0) continue;
            pModel->TransformByObjectBone(vPos, pObject, iBones[i]);
            CreateParticleFpsChecked(BITMAP_SMOKE, vPos, pObject->Angle, vLight, 50, 1.0f);
            CreateParticleFpsChecked(BITMAP_SMOKELINE1 + rand() % 3, vPos, pObject->Angle, vLight, 0, 1.0f);
        }
    }
    if (pObject->CurrentAction == MONSTER01_DIE)
    {
        if (pObject->LifeTime == 100)
        {
            pObject->LifeTime = 90;
            pObject->m_bRenderShadow = false;

            vec3_t vWorldPos, Light;
            Vector(1.0f, 1.0f, 1.0f, Light);

            pModel->TransformByObjectBone(vWorldPos, pObject, 34);
            CreateEffect(MODEL_SHADOW_KNIGHT_ANKLE_LEFT, vWorldPos, pObject->Angle, Light, 0, pObject, 0, 0);
            pModel->TransformByObjectBone(vWorldPos, pObject, 39);
            CreateEffect(MODEL_SHADOW_KNIGHT_ANKLE_RIGHT, vWorldPos, pObject->Angle, Light, 0, pObject, 0, 0);
            pModel->TransformByObjectBone(vWorldPos, pObject, 0);
            CreateEffect(MODEL_SHADOW_KNIGHT_BELT, vWorldPos, pObject->Angle, Light, 0, pObject, 0, 0);
            pModel->TransformByObjectBone(vWorldPos, pObject, 4);
            CreateEffect(MODEL_SHADOW_KNIGHT_CHEST, vWorldPos, pObject->Angle, Light, 0, pObject, 0, 0);
            pModel->TransformByObjectBone(vWorldPos, pObject, 6);
            CreateEffect(MODEL_SHADOW_KNIGHT_HELMET, vWorldPos, pObject->Angle, Light, 0, pObject, 0, 0);
            pModel->TransformByObjectBone(vWorldPos, pObject, 33);
            CreateEffect(MODEL_SHADOW_KNIGHT_KNEE_LEFT, vWorldPos, pObject->Angle, Light, 0, pObject, 0, 0);
            pModel->TransformByObjectBone(vWorldPos, pObject, 38);
            CreateEffect(MODEL_SHADOW_KNIGHT_KNEE_RIGHT, vWorldPos, pObject->Angle, Light, 0, pObject, 0, 0);
            pModel->TransformByObjectBone(vWorldPos, pObject, 13);
            CreateEffect(MODEL_SHADOW_KNIGHT_WRIST_LEFT, vWorldPos, pObject->Angle, Light, 0, pObject, 0, 0);
            pModel->TransformByObjectBone(vWorldPos, pObject, 23);
            CreateEffect(MODEL_SHADOW_KNIGHT_WRIST_RIGHT, vWorldPos, pObject->Angle, Light, 0, pObject, 0, 0);
        }
    }
    break;
    case MODEL_SHADOW_LOOK:
    {
        int iBones[] = { 11, 15, 34, 21, 25, 39 };
        Vector(0.5f, 1.0f, 0.5f, vLight);
        for (int i = 0; i < 6; ++i)
        {
            if (rand() % 6 > 0) continue;
            pModel->TransformByObjectBone(vPos, pObject, iBones[i]);
            CreateParticleFpsChecked(BITMAP_SMOKE, vPos, pObject->Angle, vLight, 50, 1.5f);
            CreateParticleFpsChecked(BITMAP_SMOKELINE1 + rand() % 3, vPos, pObject->Angle, vLight, 0, 1.1f);
        }
    }
    if (pObject->CurrentAction == MONSTER01_DIE)
    {
        if (pObject->LifeTime == 100)
        {
            pObject->LifeTime = 90;
            pObject->m_bRenderShadow = false;

            vec3_t vWorldPos, Light;
            Vector(1.0f, 1.0f, 1.0f, Light);

            pModel->TransformByObjectBone(vWorldPos, pObject, 34);
            CreateEffect(MODEL_SHADOW_ROOK_ANKLE_LEFT, vWorldPos, pObject->Angle, Light, 0, pObject, 0, 0);
            pModel->TransformByObjectBone(vWorldPos, pObject, 39);
            CreateEffect(MODEL_SHADOW_ROOK_ANKLE_RIGHT, vWorldPos, pObject->Angle, Light, 0, pObject, 0, 0);
            pModel->TransformByObjectBone(vWorldPos, pObject, 0);
            CreateEffect(MODEL_SHADOW_ROOK_BELT, vWorldPos, pObject->Angle, Light, 0, pObject, 0, 0);
            pModel->TransformByObjectBone(vWorldPos, pObject, 4);
            CreateEffect(MODEL_SHADOW_ROOK_CHEST, vWorldPos, pObject->Angle, Light, 0, pObject, 0, 0);
            pModel->TransformByObjectBone(vWorldPos, pObject, 6);
            CreateEffect(MODEL_SHADOW_ROOK_HELMET, vWorldPos, pObject->Angle, Light, 0, pObject, 0, 0);
            pModel->TransformByObjectBone(vWorldPos, pObject, 33);
            CreateEffect(MODEL_SHADOW_ROOK_KNEE_LEFT, vWorldPos, pObject->Angle, Light, 0, pObject, 0, 0);
            pModel->TransformByObjectBone(vWorldPos, pObject, 38);
            CreateEffect(MODEL_SHADOW_ROOK_KNEE_RIGHT, vWorldPos, pObject->Angle, Light, 0, pObject, 0, 0);
            pModel->TransformByObjectBone(vWorldPos, pObject, 12);
            CreateEffect(MODEL_SHADOW_ROOK_WRIST_LEFT, vWorldPos, pObject->Angle, Light, 0, pObject, 0, 0);
            pModel->TransformByObjectBone(vWorldPos, pObject, 23);
            CreateEffect(MODEL_SHADOW_ROOK_WRIST_RIGHT, vWorldPos, pObject->Angle, Light, 0, pObject, 0, 0);
        }
    }
    break;
    case MODEL_NAPIN:
    {
        int iBones[] = { 7, 4, 5, 10, 22, 11, 23, 12, 24, 34, 39 };
        vec3_t vLightFlare;
        float fScale;
        Vector(0.4f, 0.7f, 1.0f, vLight);
        Vector(0.1f, 0.2f, 1.0f, vLightFlare);
        for (int i = 0; i < 11; ++i)
        {
            pModel->TransformByObjectBone(vPos, pObject, iBones[i]);
            CreateSprite(BITMAP_LIGHT, vPos, 2.2f, vLightFlare, pObject);
            if (rand() % 3 > 0) continue;
            Vector((rand() % 30 - 15) * 1.0f, (rand() % 30 - 15) * 1.0f, (rand() % 30 - 15) * 1.0f, vRelative);
            pModel->TransformByObjectBone(vPos, pObject, iBones[i], vRelative);
            fScale = (float)(rand() % 80 + 32) * 0.01f * 1.0f;
            CreateParticleFpsChecked(BITMAP_LIGHTNING_MEGA1 + rand() % 3, vPos, pObject->Angle, vLight, 0, fScale);
        }
    }
    break;
    case MODEL_GHOST_NAPIN:
    {
        int iBones[] = { 21, 37, 65, 66, 77, 78, 79 };
        Vector(0.4f, 1.0f, 0.4f, vLight);
        for (int i = 0; i < 7; ++i)
        {
            pModel->TransformByObjectBone(vPos, pObject, iBones[i]);
            CreateParticleFpsChecked(BITMAP_SMOKELINE1 + rand() % 3, vPos, pObject->Angle, vLight, 1, 1.0f, pObject);
            CreateParticleFpsChecked(BITMAP_CLUD64, vPos, pObject->Angle, vLight, 6, 1.0f, pObject);
        }
    }
    break;
    case MODEL_BLAZE_NAPIN:
    {
        int iBones[] = { 10, 61, 72, 21, 122, 116 };
        Vector(1.0f, 1.0f, 1.0f, vLight);
        vec3_t vLightFire;
        Vector(1.0f, 0.2f, 0.0f, vLightFire);
        for (int i = 0; i < 6; ++i)
        {
            float fScale = 1.2f;
            if (i >= 4)
            {
                pModel->TransformByObjectBone(vPos, pObject, iBones[i]);
                CreateSprite(BITMAP_LIGHT, vPos, 1.0f, vLightFire, pObject);

                fScale = 0.7f;
                Vector((rand() % 10 - 5) * 1.0f, (rand() % 10 - 5) * 1.0f, (rand() % 10 - 5) * 1.0f, vRelative);
                pModel->TransformByObjectBone(vPos, pObject, iBones[i], vRelative);
            }
            else
            {
                pModel->TransformByObjectBone(vPos, pObject, iBones[i]);
                CreateSprite(BITMAP_LIGHT, vPos, 4.0f, vLightFire, pObject);

                Vector((rand() % 20 - 10) * 1.0f, (rand() % 20 - 10) * 1.0f, (rand() % 20 - 10) * 1.0f, vRelative);
                pModel->TransformByObjectBone(vPos, pObject, iBones[i], vRelative);
            }

            switch (rand() % 3)
            {
            case 0:
                CreateParticleFpsChecked(BITMAP_FIRE_HIK1, vPos, pObject->Angle, vLight, 0, fScale);
                break;
            case 1:
                CreateParticleFpsChecked(BITMAP_FIRE_CURSEDLICH, vPos, pObject->Angle, vLight, 4, fScale);
                break;
            case 2:
                CreateParticleFpsChecked(BITMAP_FIRE_HIK3, vPos, pObject->Angle, vLight, 0, fScale);
                break;
            }
        }
    }
    break;
    case MODEL_MEDUSA:
    {
        vec3_t vColor;
        Vector(1.0f, 1.0f, 1.0f, vColor);

        pModel->TransformByObjectBone(vPos, pObject, 5);
        Vector(0.9f, 0.8f, 0.3f, vColor);
        CreateSprite(BITMAP_LIGHTMARKS, vPos, 2.0f, vColor, pObject);
        Vector(0.1f, 1.0f, 0.0f, vColor);
        CreateSprite(BITMAP_LIGHT, vPos, 2.4f, vColor, pObject);

        pModel->TransformByObjectBone(vPos, pObject, 33);
        Vector(0.9f, 0.8f, 0.3f, vColor);
        CreateSprite(BITMAP_LIGHTMARKS, vPos, 1.2f, vColor, pObject);
        Vector(0.1f, 1.0f, 0.0f, vColor);
        CreateSprite(BITMAP_LIGHT, vPos, 1.4f, vColor, pObject);

        MoveEye(pObject, pModel, 34, 35);
        Vector(1.0f, 0.0f, 0.0f, vColor);
        CreateJointFpsChecked(BITMAP_JOINT_ENERGY, vPos, pObject->Position, pObject->Angle, 55, pObject, 6.0f, -1, 0, 0, -1, vColor);
        CreateJointFpsChecked(BITMAP_JOINT_ENERGY, vPos, pObject->Position, pObject->Angle, 56, pObject, 6.0f, -1, 0, 0, -1, vColor);
        Vector(1.0f, 1.0f, 1.0f, vColor);
        pModel->TransformByObjectBone(vPos, pObject, 34);
        vPos[1] -= 0.8f;
        CreateSprite(BITMAP_LIGHT_RED, vPos, 0.4f, vColor, pObject);
        pModel->TransformByObjectBone(vPos, pObject, 35);
        vPos[1] -= 0.8f;
        CreateSprite(BITMAP_LIGHT_RED, vPos, 0.4f, vColor, pObject);

        pModel->TransformByObjectBone(vPos, pObject, 68);
        Vector(0.1f, 1.0f, 0.0f, vColor);
        CreateSprite(BITMAP_LIGHT, vPos, 4.0f, vColor, pObject);
        Vector(0.9f, 1.0f, 0.1f, vColor);
        CreateSprite(BITMAP_SHOCK_WAVE, vPos, 0.35f, vColor, pObject);
        Vector(0.9f, 1.0f, 0.1f, vColor);
        CreateSprite(BITMAP_SHOCK_WAVE, vPos, 0.22f, vColor, pObject);
        Vector(0.9f, 1.0f, 0.9f, vColor);
        CreateSprite(BITMAP_SHINY + 1, vPos, 1.2f, vColor, pObject);
        if (rand_fps_check(2))
        {
            CreateEffect(BITMAP_WATERFALL_4, vPos, pObject->Angle, vColor, 0, pObject, 68);
        }

        Vector(0.1f, 0.6f, 0.3f, vColor);
        CreateJointFpsChecked(BITMAP_JOINT_ENERGY, vPos, pObject->Position, pObject->Angle, 57, pObject, 10.0f, 67, 0, 0, 15, vColor); //57 - pk:node , ichaindex:maxtail
        CreateJointFpsChecked(BITMAP_JOINT_ENERGY, vPos, pObject->Position, pObject->Angle, 57, pObject, 10.0f, 70, 0, 0, 15, vColor); //57 - pk:node , ichaindex:maxtail

        Vector(1.0f, 1.0f, 1.0f, vColor);
        pModel->TransformByObjectBone(vPos, pObject, 66);
        CreateSprite(BITMAP_LIGHT_RED, vPos, 0.4f, vColor, pObject);
        pModel->TransformByObjectBone(vPos, pObject, 67);
        CreateSprite(BITMAP_LIGHT_RED, vPos, 0.4f, vColor, pObject);
        pModel->TransformByObjectBone(vPos, pObject, 69);
        CreateSprite(BITMAP_LIGHT_RED, vPos, 0.4f, vColor, pObject);
        pModel->TransformByObjectBone(vPos, pObject, 70);
        CreateSprite(BITMAP_LIGHT_RED, vPos, 0.4f, vColor, pObject);

        int temp[] = { 5, 37, 52 };
        Vector(0.25f, 1.0f, 0.0f, vColor);
        for (int i = 0; i < 3; ++i)
        {
            if (rand() % 6 > 0) continue;
            if (i % 5 == 0)
            {
                pModel->TransformByObjectBone(vPos, pObject, temp[i]);
                CreateParticleFpsChecked(BITMAP_SMOKE, vPos, pObject->Angle, vColor, 50, 1.0f);
                CreateParticleFpsChecked(BITMAP_SMOKELINE1 + rand() % 3, vPos, pObject->Angle, vColor, 0, 1.0f);
            }
        }

        if (pObject->CurrentAction == MONSTER01_DIE)
        {
            pObject->m_bRenderShadow = false;

            if (rand_fps_check(3))
            {
                Vector(0.4f, 0.9f, 0.6f, vColor);
                pModel->TransformByObjectBone(vPos, pObject, 0);
                CreateParticle(BITMAP_SMOKE, vPos, pObject->Angle, vColor, 1);
                CreateParticle(BITMAP_SMOKE, vPos, pObject->Angle, vColor, 1);

                pModel->TransformByObjectBone(vPos, pObject, 5);
                CreateParticle(BITMAP_SMOKE, vPos, pObject->Angle, vColor, 1);
                CreateParticle(BITMAP_SMOKE, vPos, pObject->Angle, vColor, 1);

                pModel->TransformByObjectBone(vPos, pObject, 124);
                CreateParticle(BITMAP_SMOKE, vPos, pObject->Angle, vColor, 1);
                CreateParticle(BITMAP_SMOKE, vPos, pObject->Angle, vColor, 1);
            }
        }
    }
    break;
    case MODEL_SAPI_QUEEN:
    case MODEL_WOLF_STATUS:
    {
        Vector(1.0f, 0.6f, 0.1f, vLight);
        Vector(0.0f, 0.0f, 0.0f, vRelative);
        pModel->TransformPosition(pObject->BoneTransform[15], vRelative, vPos, true);

        CreateSprite(BITMAP_LIGHT, vPos, 2.0f, vLight, pObject);
        CreateSprite(BITMAP_LIGHT, vPos, 2.0f, vLight, pObject);
    }
    break;
    case MODEL_ICE_NAPIN:
    {
        int iBones[] = { 10, 61, 72, 21, 122, 116 };
        Vector(1.0f, 1.0f, 1.0f, vLight);
        vec3_t vLightFire;
        Vector(1.0f, 1.0f, 1.0f, vLightFire);
        for (int i = 0; i < 6; ++i)
        {
            float fScale = 1.2f;
            if (i >= 4)
            {
                pModel->TransformByObjectBone(vPos, pObject, iBones[i]);
                CreateSprite(BITMAP_LIGHT, vPos, 1.0f, vLightFire, pObject);

                fScale = 0.7f;
                Vector((rand() % 10 - 5) * 1.0f, (rand() % 10 - 5) * 1.0f, (rand() % 10 - 5) * 1.0f, vRelative);
                pModel->TransformByObjectBone(vPos, pObject, iBones[i], vRelative);
            }
            else
            {
                pModel->TransformByObjectBone(vPos, pObject, iBones[i]);
                CreateSprite(BITMAP_LIGHT, vPos, 4.0f, vLightFire, pObject);

                Vector((rand() % 20 - 10) * 1.0f, (rand() % 20 - 10) * 1.0f, (rand() % 20 - 10) * 1.0f, vRelative);
                pModel->TransformByObjectBone(vPos, pObject, iBones[i], vRelative);
            }
            Vector(0.25f, 0.6f, 0.7f, vLight);

            switch (rand() % 3)
            {
            case 0:
                CreateParticleFpsChecked(BITMAP_FIRE_HIK1, vPos, pObject->Angle, vLight, 0, fScale);
                break;
            case 1:
                CreateParticleFpsChecked(BITMAP_FIRE_CURSEDLICH, vPos, pObject->Angle, vLight, 4, fScale);
                break;
            case 2:
                CreateParticleFpsChecked(BITMAP_FIRE_HIK3, vPos, pObject->Angle, vLight, 0, fScale);
                break;
            }
        }
    }
    break;
    case MODEL_SHADOW_MASTER:
    {
        int iBones[] = { 11, 15, 34, 21, 25, 39 };
        Vector(1.0f, 1.0f, 1.f, vLight);
        for (int i = 0; i < 6; ++i)
        {
            if (rand() % 6 > 0) continue;
            pModel->TransformByObjectBone(vPos, pObject, iBones[i]);
            CreateParticleFpsChecked(BITMAP_SMOKE, vPos, pObject->Angle, vLight, 50, 1.5f);
            CreateParticleFpsChecked(BITMAP_SMOKELINE1 + rand() % 3, vPos, pObject->Angle, vLight, 0, 1.1f);
        }
    }
    if (pObject->CurrentAction == MONSTER01_DIE)
    {
        if (pObject->LifeTime == 100)
        {
            pObject->LifeTime = 90;
            pObject->m_bRenderShadow = false;

            vec3_t vWorldPos, Light;
            Vector(1.0f, 1.0f, 1.0f, Light);

            pModel->TransformByObjectBone(vWorldPos, pObject, 34);
            CreateEffect(MODEL_EX01_SHADOW_MASTER_ANKLE_LEFT, vWorldPos, pObject->Angle, Light, 0, pObject, 0, 0);
            pModel->TransformByObjectBone(vWorldPos, pObject, 39);
            CreateEffect(MODEL_EX01_SHADOW_MASTER_ANKLE_RIGHT, vWorldPos, pObject->Angle, Light, 0, pObject, 0, 0);
            pModel->TransformByObjectBone(vWorldPos, pObject, 0);
            CreateEffect(MODEL_EX01_SHADOW_MASTER_BELT, vWorldPos, pObject->Angle, Light, 0, pObject, 0, 0);
            pModel->TransformByObjectBone(vWorldPos, pObject, 4);
            CreateEffect(MODEL_EX01_SHADOW_MASTER_CHEST, vWorldPos, pObject->Angle, Light, 0, pObject, 0, 0);
            pModel->TransformByObjectBone(vWorldPos, pObject, 6);
            CreateEffect(MODEL_EX01_SHADOW_MASTER_HELMET, vWorldPos, pObject->Angle, Light, 0, pObject, 0, 0);
            pModel->TransformByObjectBone(vWorldPos, pObject, 33);
            CreateEffect(MODEL_EX01_SHADOW_MASTER_KNEE_LEFT, vWorldPos, pObject->Angle, Light, 0, pObject, 0, 0);
            pModel->TransformByObjectBone(vWorldPos, pObject, 38);
            CreateEffect(MODEL_EX01_SHADOW_MASTER_KNEE_RIGHT, vWorldPos, pObject->Angle, Light, 0, pObject, 0, 0);
            pModel->TransformByObjectBone(vWorldPos, pObject, 12);
            CreateEffect(MODEL_EX01_SHADOW_MASTER_WRIST_LEFT, vWorldPos, pObject->Angle, Light, 0, pObject, 0, 0);
            pModel->TransformByObjectBone(vWorldPos, pObject, 23);
            CreateEffect(MODEL_EX01_SHADOW_MASTER_WRIST_RIGHT, vWorldPos, pObject->Angle, Light, 0, pObject, 0, 0);
        }
    }
    break;
    }

    return false;
}

bool GMSwampOfQuiet::PlayMonsterSound(OBJECT* pObject)
{
    if (!IsCurrentMap())
        return false;

    float fDis_x, fDis_y;
    fDis_x = pObject->Position[0] - Hero->Object.Position[0];
    fDis_y = pObject->Position[1] - Hero->Object.Position[1];
    float fDistance = sqrtf(fDis_x * fDis_x + fDis_y * fDis_y);

    if (fDistance > 500.0f) return true;

    switch (pObject->Type)
    {
    case MODEL_SAPIUNUS:
    case MODEL_SAPIDUO:
        if (pObject->CurrentAction == MONSTER01_ATTACK1 || pObject->CurrentAction == MONSTER01_ATTACK2)
        {
            if (rand_fps_check(3))
            {
                PlayBuffer(SOUND_SWAMPOFQUIET_SAPI_UNUS_ATTACK01);
            }
        }
        else if (pObject->CurrentAction == MONSTER01_DIE)
        {
            if (rand_fps_check(3))
            {
                PlayBuffer(SOUND_SWAMPOFQUIET_SAPI_DEATH01);
            }
        }
        return true;
    case MODEL_SAPITRES:
        if (pObject->CurrentAction == MONSTER01_ATTACK1 || pObject->CurrentAction == MONSTER01_ATTACK2)
        {
            if (rand_fps_check(3))
            {
                PlayBuffer(SOUND_SWAMPOFQUIET_SAPI_TRES_ATTACK01);
            }
        }
        else if (pObject->CurrentAction == MONSTER01_DIE)
        {
            if (rand_fps_check(3))
            {
                PlayBuffer(SOUND_SWAMPOFQUIET_SAPI_DEATH01);
            }
        }
        return true;
    case MODEL_SHADOW_PAWN:
        if (pObject->CurrentAction == MONSTER01_ATTACK1 || pObject->CurrentAction == MONSTER01_ATTACK2)
        {
            PlayBuffer(SOUND_SWAMPOFQUIET_SHADOW_PAWN_ATTACK01);
        }
        else if (pObject->CurrentAction == MONSTER01_DIE)
        {
            PlayBuffer(SOUND_SWAMPOFQUIET_SHADOW_DEATH01);
        }
        return true;
    case MODEL_SHADOW_KNIGHT:
        if (pObject->CurrentAction == MONSTER01_ATTACK1 || pObject->CurrentAction == MONSTER01_ATTACK2)
        {
            PlayBuffer(SOUND_SWAMPOFQUIET_SHADOW_KNIGHT_ATTACK01);
        }
        else if (pObject->CurrentAction == MONSTER01_DIE)
        {
            PlayBuffer(SOUND_SWAMPOFQUIET_SHADOW_DEATH01);
        }
        return true;
    case MODEL_SHADOW_LOOK:
        if (pObject->CurrentAction == MONSTER01_ATTACK1 || pObject->CurrentAction == MONSTER01_ATTACK2)
        {
            PlayBuffer(SOUND_SWAMPOFQUIET_SHADOW_ROOK_ATTACK01);
        }
        else if (pObject->CurrentAction == MONSTER01_DIE)
        {
            PlayBuffer(SOUND_SWAMPOFQUIET_SHADOW_DEATH01);
        }
        return true;
    case MODEL_NAPIN:
        if (pObject->CurrentAction == MONSTER01_WALK)
        {
            if (rand_fps_check(100))
            {
                PlayBuffer(SOUND_SWAMPOFQUIET_THUNDER_NAIPIN_BREATH01);
            }
        }
        else if (pObject->CurrentAction == MONSTER01_ATTACK1 || pObject->CurrentAction == MONSTER01_ATTACK2)
        {
            PlayBuffer(SOUND_SWAMPOFQUIET_NAIPIN_ATTACK01);
        }
        else if (pObject->CurrentAction == MONSTER01_DIE)
        {
            PlayBuffer(SOUND_SWAMPOFQUIET_NAIPIN_DEATH01);
        }
        return true;
    case MODEL_GHOST_NAPIN:
        if (pObject->CurrentAction == MONSTER01_WALK)
        {
            if (rand_fps_check(100))
            {
                PlayBuffer(SOUND_SWAMPOFQUIET_GHOST_NAIPIN_BREATH01);
            }
        }
        else if (pObject->CurrentAction == MONSTER01_ATTACK1 || pObject->CurrentAction == MONSTER01_ATTACK2)
        {
            PlayBuffer(SOUND_SWAMPOFQUIET_NAIPIN_ATTACK01);
        }
        else if (pObject->CurrentAction == MONSTER01_DIE)
        {
            PlayBuffer(SOUND_SWAMPOFQUIET_NAIPIN_DEATH01);
        }
        return true;
    case MODEL_BLAZE_NAPIN:
        if (pObject->CurrentAction == MONSTER01_WALK)
        {
            if (rand_fps_check(100))
            {
                PlayBuffer(SOUND_SWAMPOFQUIET_BLAZE_NAIPIN_BREATH01);
            }
        }
        else if (pObject->CurrentAction == MONSTER01_ATTACK1 || pObject->CurrentAction == MONSTER01_ATTACK2)
        {
            PlayBuffer(SOUND_SWAMPOFQUIET_NAIPIN_ATTACK01);
        }
        else if (pObject->CurrentAction == MONSTER01_DIE)
        {
            PlayBuffer(SOUND_SWAMPOFQUIET_NAIPIN_DEATH01);
        }
        return true;
    case MODEL_MEDUSA:
    {
    }
    return true;
    case MODEL_SAPI_QUEEN:
    case MODEL_WOLF_STATUS:
        if (pObject->CurrentAction == MONSTER01_ATTACK1 || pObject->CurrentAction == MONSTER01_ATTACK2)
        {
            if (rand_fps_check(3))
            {
                PlayBuffer(SOUND_SWAMPOFQUIET_SAPI_UNUS_ATTACK01);
            }
        }
        else if (pObject->CurrentAction == MONSTER01_DIE)
        {
            if (rand_fps_check(3))
            {
                PlayBuffer(SOUND_SWAMPOFQUIET_SAPI_DEATH01);
            }
        }
        return true;
    case MODEL_ICE_NAPIN:
        if (pObject->CurrentAction == MONSTER01_WALK)
        {
            if (rand_fps_check(100))
            {
                PlayBuffer(SOUND_SWAMPOFQUIET_BLAZE_NAIPIN_BREATH01);
            }
        }
        else if (pObject->CurrentAction == MONSTER01_ATTACK1 || pObject->CurrentAction == MONSTER01_ATTACK2)
        {
            PlayBuffer(SOUND_SWAMPOFQUIET_NAIPIN_ATTACK01);
        }
        else if (pObject->CurrentAction == MONSTER01_DIE)
        {
            PlayBuffer(SOUND_SWAMPOFQUIET_NAIPIN_DEATH01);
        }
        return true;
    case MODEL_SHADOW_MASTER:
        if (pObject->CurrentAction == MONSTER01_ATTACK1 || pObject->CurrentAction == MONSTER01_ATTACK2)
        {
            PlayBuffer(SOUND_SWAMPOFQUIET_SHADOW_ROOK_ATTACK01);
        }
        else if (pObject->CurrentAction == MONSTER01_DIE)
        {
            PlayBuffer(SOUND_SWAMPOFQUIET_SHADOW_DEATH01);
        }
        return true;
    }

    return false;
}

bool GMSwampOfQuiet::AttackEffectMonster(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel)
{
    if (!IsCurrentMap())
        return false;

    return false;
}

bool GMSwampOfQuiet::SetCurrentActionMonster(CHARACTER* pCharacter, OBJECT* pObject)
{
    if (!IsCurrentMap())
        return false;

    switch (pObject->Type)
    {
    case MODEL_MEDUSA:
    {
        switch (pCharacter->Skill)
        {
        case AT_SKILL_DECAY:
        {
            SetAction(pObject, MONSTER01_ATTACK2);
            pCharacter->MonsterSkill = -1;
        } break;
        case AT_SKILL_GAOTIC:
        {
            SetAction(pObject, MONSTER01_ATTACK2);
            pCharacter->MonsterSkill = -1;
        } break;
        case AT_SKILL_GIGANTIC_STORM:
        {
            SetAction(pObject, MONSTER01_ATTACK3);
            pCharacter->MonsterSkill = -1;
        } break;
        case AT_SKILL_EVIL_SPIRIT:
        {
            SetAction(pObject, MONSTER01_ATTACK1);
            pCharacter->MonsterSkill = -1;
        } break;
        default:
        {
            SetAction(pObject, MONSTER01_ATTACK1);
            pCharacter->MonsterSkill = -1;
        } break;
        }
    }
    return true;
    case MODEL_SAPI_QUEEN:
    case MODEL_WOLF_STATUS:
    {
        if (pCharacter->MonsterSkill == ATMON_SKILL_EX_SAPIQUEEN_ATTACKSKILL)
        {
            SetAction(pObject, MONSTER01_ATTACK2);
            pCharacter->MonsterSkill = -1;
        }
        else
        {
            SetAction(pObject, MONSTER01_ATTACK1);
            pCharacter->MonsterSkill = -1;
        }
        return true;
    }
    return true;
    case MODEL_ICE_NAPIN:
    {
        if (pCharacter->MonsterSkill == ATMON_SKILL_EX_ICENAPIN_ATTACKSKILL)
        {
            SetAction(pObject, MONSTER01_ATTACK2);
            pCharacter->MonsterSkill = -1;
        }
        else
        {
            SetAction(pObject, MONSTER01_ATTACK1);
            pCharacter->MonsterSkill = -1;
        }
        return true;
    }
    return true;
    case MODEL_SHADOW_MASTER:
    {
        if (pCharacter->MonsterSkill == ATMON_SKILL_EX_SHADOWMASTER_ATTACKSKILL)
        {
            SetAction(pObject, MONSTER01_ATTACK2);
            pCharacter->MonsterSkill = -1;
        }
        else
        {
            SetAction(pObject, MONSTER01_ATTACK1);
            pCharacter->MonsterSkill = -1;
        }
        return true;
    }
    return true;
    default:
        return false;
    }
    return false;
}