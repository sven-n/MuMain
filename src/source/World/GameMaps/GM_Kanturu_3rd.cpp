#include "stdafx.h"
#include "GameLogic/Monsters/MonsterAttackPlay.h"
#include "Core/Utilities/Random.h"
#include "Render/Textures/ZzzOpenglUtil.h"
#include "Render/Textures/ZzzTexture.h"
#include "Render/Models/ZzzBMD.h"
#include "Render/Terrain/ZzzLodTerrain.h"
#include "Scenes/SceneCore.h"
#include "Render/Effects/ZzzEffect.h"
#include "Engine/AI/ZzzAI.h"
#include "Engine/Object/ZzzOpenData.h"
#include "Engine/Object/ZzzCharacter.h"
#include "Engine/Object/ZzzInterface.h"
#include "GameLogic/Events/Cinematic/CDirection.h"
#include "Render/Models/BoneManager.h"
#include "Core/Time/CTimCheck.h"
#include "Audio/DSPlaySound.h"
#include "World/MapInfra/MapManager.h"

extern unsigned int WindowWidth;


bool KanturuSuccessMap = false;
bool KanturuSuccessMapBackup = false;
int iMayaAction = -1;
bool bMayaSkill2 = false;
int iMayaSkill2_Counter = 0;
int iMayaDie_Counter = 0;
int	iKanturuResult = -1;
float fAlpha = 0.1f;
int	UserCount = 0;
int	MonsterCount = 0;

bool M39Kanturu3rd::IsInKanturu3rd()
{
    return (gMapManager.WorldActive == WD_39KANTURU_3RD) ? true : false;
}

void M39Kanturu3rd::Kanturu3rdInit()
{
    KanturuSuccessMap = false;
    KanturuSuccessMapBackup = false;
    iMayaAction = -1;
    bMayaSkill2 = false;
    iMayaSkill2_Counter = 0;
    iMayaDie_Counter = 0;
    iKanturuResult = -1;
    fAlpha = 0.1f;
    UserCount = 0;
    MonsterCount = 0;
}

bool M39Kanturu3rd::IsSuccessBattle()
{
    if (KanturuSuccessMap)
        return true;
    return false;
}

void M39Kanturu3rd::CheckSuccessBattle(BYTE State, BYTE DetailState)
{
    if (State == KANTURU_STATE_TOWER && (DetailState == KANTURU_TOWER_REVITALIXATION || DetailState == KANTURU_TOWER_NOTIFY))
        KanturuSuccessMap = true;
    else
        KanturuSuccessMap = false;

    if (KanturuSuccessMap == KanturuSuccessMapBackup)
        return;

    KanturuSuccessMapBackup = KanturuSuccessMap;

    if (gMapManager.WorldActive != WD_39KANTURU_3RD)
        return;

    wchar_t FileName[64];
    wchar_t WorldName[32];
    mu_swprintf(WorldName, L"World%d", gMapManager.WorldActive + 1);

    //
    if (KanturuSuccessMap)
    {
        PlayBuffer(SOUND_KANTURU_3RD_MAP_SOUND02);
        mu_swprintf(FileName, L"Data\\%ls\\EncTerrain%d.att", WorldName, (gMapManager.WorldActive + 1) * 10 + 1);
    }
    else
    {
        mu_swprintf(FileName, L"Data\\%ls\\EncTerrain%d.att", WorldName, gMapManager.WorldActive + 1);
    }

    OpenTerrainAttribute(FileName);
}

bool M39Kanturu3rd::CreateKanturu3rdObject(OBJECT* o)
{
    if (!IsInKanturu3rd())
        return false;

    switch (o->Type)
    {
    case 0:
        o->Position[2] -= 2000.0f;
        break;
    case 32:
    case 47:
    case 51:
    case 52:
    case 53:
    case 54:
    case 57:
    case 58:
    case 70:
        o->HiddenMesh = -2;
        break;
    }

    return true;
}

bool M39Kanturu3rd::MoveKanturu3rdObject(OBJECT* o)
{
    if (!IsInKanturu3rd())
        return false;

    float Luminosity;
    vec3_t Light;

    switch (o->Type)
    {
    case 0:
    {
    }
    break;
    case 25:
    {
        PlayBuffer(SOUND_KANTURU_3RD_MAP_SOUND05);
    }
    break;
    case 40:
    case 41:
    case 42:
    {
        PlayBuffer(SOUND_KANTURU_3RD_MAP_SOUND01);
    }
    break;
    case 45:
    {
        if (rand_fps_check(3))
        {
            o->HiddenMesh = -2;
            Luminosity = (float)(Random::Rand() % 4 + 3) * 0.3f;
            Vector(Luminosity, Luminosity, Luminosity, Light);
            AddTerrainLight(o->Position[0], o->Position[1], Light, 1 + o->Scale / 2.0f, PrimaryTerrainLight);
        }
    }
    break;
    case 48:
    {
        o->HiddenMesh = -2;
    }
    break;
    case 50:
    {
        if (rand_fps_check(3))
        {
            Luminosity = (float)(Random::Rand() % 10) * 0.2f;
            Vector(Luminosity, Luminosity, Luminosity, Light);
            AddTerrainLight(o->Position[0], o->Position[1], Light, 1 + o->Scale, PrimaryTerrainLight);
        }
    }
    break;
    case 54:
    {
        if (o->HiddenMesh == -1)
        {
            o->HiddenMesh = -2;
            Luminosity = (float)(Random::Rand() % 4 + 3) * 0.3f;
            Vector(Luminosity, Luminosity, Luminosity, Light);
            AddTerrainLight(o->Position[0], o->Position[1], Light, 1 + o->Scale / 2.0f, PrimaryTerrainLight);
        }
    }
    break;
    case 71:
    {
        PlayBuffer(SOUND_KANTURU_3RD_MAP_SOUND04);
    }
    break;
    case 73:
    {
        PlayBuffer(SOUND_KANTURU_3RD_MAP_SOUND03);
    }
    break;
    }

    PlayBuffer(SOUND_KANTURU_3RD_AMBIENT);

    return true;
}

bool M39Kanturu3rd::UpdateKanturu3rdObjectEffects(OBJECT* o, BMD* b)
{
    if (!IsInKanturu3rd())
        return false;

    vec3_t Position, Light;

    switch (o->Type)
    {
    case 0:
    {
        vec3_t p, Pos, Light;

        // Maya darkens as she explodes, until only her outline is left.
        if (g_Direction.m_CKanturu.GetMayaExplotion())
        {
            o->Light[0] *= pow(1.0f / (1.02f), FPS_ANIMATION_FACTOR);
            o->Light[1] *= pow(1.0f / (1.02f), FPS_ANIMATION_FACTOR);
            o->Light[2] *= pow(1.0f / (1.02f), FPS_ANIMATION_FACTOR);
            if (o->Light[0] <= 0.05f)
            {
                o->Light[0] = 0.05f;
                o->Light[1] = 0.05f;
                o->Light[2] = 0.05f;
                o->HiddenMesh = -2;
            }
        }
        else
            Vector(1.0f, 1.0f, 1.0f, o->Light);

        if (!g_Direction.m_CKanturu.GetMayaExplotion())
            o->HiddenMesh = 0;

        // Her body's glow, kept in StartPosition: it fades as she explodes and pulses otherwise.
        if (g_Direction.m_CKanturu.GetMayaExplotion())
        {
            o->StartPosition[0] *= pow(1.0f / (1.02f), FPS_ANIMATION_FACTOR);
            o->StartPosition[1] *= pow(1.0f / (1.02f), FPS_ANIMATION_FACTOR);
            o->StartPosition[2] *= pow(1.0f / (1.02f), FPS_ANIMATION_FACTOR);
        }
        else
        {
            o->StartPosition[0] = sinf(WorldTime * 0.001f) * 0.12f + 0.05f;
            o->StartPosition[1] = sinf(WorldTime * 0.001f) * 0.12f + 0.3f;
            o->StartPosition[2] = sinf(WorldTime * 0.001f) * 0.12f + 0.21f;
        }

        if (g_Direction.m_CKanturu.m_iMayaState >= KANTURU_MAYA_DIRECTION_MONSTER1 && g_Direction.m_CKanturu.m_iMayaState <= KANTURU_MAYA_DIRECTION_ENDCYCLE)
            o->Position[2] = -749.5f;

        if (g_Direction.m_CKanturu.GetMayaAppear())
        {
            if (o->Position[2] >= -749.5f)
            {
                g_Direction.m_CKanturu.SetMayaAppear(false);
                o->Position[2] = -749.5f;
            }

            EarthQuake = (float)(Random::Rand() % 6 - 3) * 2.0f;
            o->Position[2] += 13.0f * FPS_ANIMATION_FACTOR;

            PlayBuffer(SOUND_KANTURU_3RD_MAYA_INTRO);
        }
        else
        {
            Vector(0.08f, 0.08f, 0.08f, Light);
            Vector(Random::Rand() % 20 - 30.0f, Random::Rand() % 20 - 30.0f, 0.0f, p);
            b->TransformPosition(Engine::Object::Pose::World(o, b)[34], p, Pos, false);
            if (o->AnimationFrame >= 5.0f && o->AnimationFrame < 12.5f)
                CreateParticleFpsChecked(BITMAP_SMOKE, Pos, o->Angle, Light, 43, 1.5f);
        }

        if (g_Direction.m_CKanturu.m_iMayaState == KANTURU_MAYA_DIRECTION_ENDCYCLE || g_Direction.m_CKanturu.m_iKanturuState == KANTURU_STATE_STANDBY)
        {
            //KanturuDirection.SetMayaAppear(false);
            o->Position[2] = -2749.5f;
        }

        MayaAction(o, b);
    }
    break;
    case 1:
    {
        if (o->HiddenMesh != -2)
        {
            vec3_t  Light;
            Vector(0.0f, 0.03f, 0.04f, Light);
            for (int i = 0; i < 10; ++i)
            {
                CreateParticle(BITMAP_CLOUD, o->Position, o->Angle, Light, 1, o->Scale, o);
            }
        }
        o->HiddenMesh = -2;
    }
    break;
    case 5:
    {
        float fLumi = (sinf(WorldTime * 0.002f) + 2.0f) * 0.5f;
        Vector(fLumi * 0.3f, fLumi * 0.5f, fLumi * 1.0f, Light);
        vec3_t vPos;
        Vector(-1.0f, 0.0f, 0.0f, vPos);
        b->TransformPosition(Engine::Object::Pose::World(o, b)[1], vPos, Position, false);
        CreateSprite(BITMAP_LIGHT, Position, fLumi / 2.4, Light, o);
        CreateSprite(BITMAP_SHINY + 1, Position, fLumi / 2.4, Light, o, (float)((int)(WorldTime / 2) % 360));
        CreateSprite(BITMAP_SHINY + 1, Position, fLumi / 2.4, Light, o, (float)((int)(WorldTime / 4) % 360));
    }
    break;
    case 11:
    {
        if (o->HiddenMesh != -2)
        {
            vec3_t  Light;
            Vector(0.1f, 0.1f, 0.1f, Light);
            for (int i = 0; i < 10; ++i)
            {
                CreateParticle(BITMAP_CLOUD, o->Position, o->Angle, Light, 3, o->Scale, o);
            }
        }
        o->HiddenMesh = -2;
    }
    break;
    case 32:
    {
        if (rand_fps_check(3))
        {
            Vector(0.4f, 0.6f, 0.7f, Light);
            CreateParticle(BITMAP_TWINTAIL_WATER, o->Position, o->Angle, Light, 1);
        }
    }
    break;
    case 46:
    {
        if (o->HiddenMesh != -2)
        {
            vec3_t  Light;
            Vector(0.25f, 0.25f, 0.25f, Light);
            for (int i = 0; i < 20; ++i)
            {
                CreateParticle(BITMAP_CLOUD, o->Position, o->Angle, Light, 7, o->Scale, o);
            }
        }
        o->HiddenMesh = -2;
    }
    break;
    case 74:
    {
        if (KanturuSuccessMap)
            o->HiddenMesh = -2;
        else
        {
            if (rand_fps_check(5))
                CreateJoint(BITMAP_JOINT_THUNDER + 1, o->Position, o->Position, o->Angle, 11, NULL, o->Scale * 15.0f);
        }
    }
    break;
    case 47:
    {
        if (rand_fps_check(5))
            CreateJoint(BITMAP_JOINT_THUNDER + 1, o->Position, o->Position, o->Angle, 11, NULL, o->Scale * 15.0f);
    }
    break;
    case 48:
    {
        if (rand_fps_check(3))
        {
            CreateParticle(BITMAP_CLOUD, o->Position, o->Angle, o->Light, 10, o->Scale, o);
        }
    }
    break;
    case 49:
    {
        if (o->HiddenMesh != -2)
        {
            vec3_t  Light;
            Vector(0.0f, 0.06f, 0.10f, Light);
            for (int i = 0; i < 10; ++i)
            {
                CreateParticle(BITMAP_CLOUD, o->Position, o->Angle, Light, 1, o->Scale, o);
            }
        }
        o->HiddenMesh = -2;
    }
    break;
    case 50:
    {
        if (o->HiddenMesh != -2)
        {
            vec3_t  Light;
            Vector(0.1f, 0.1f, 0.1f, Light);
            for (int i = 0; i < 10; ++i)
            {
                CreateParticle(BITMAP_CLOUD, o->Position, o->Angle, Light, 4, o->Scale, o);
            }
        }
        o->HiddenMesh = -2;
    }
    break;
    case 52:
    {
        if (rand_fps_check(3))
        {
            Vector(0.5f, 1.f, 0.7f, o->Light);
            CreateParticle(BITMAP_TRUE_BLUE, o->Position, o->Angle, o->Light, 1, o->Scale);
        }
    }
    break;
    case 53:
    {
        if (rand_fps_check(3))
        {
            Vector(0.3f, 0.3f, 0.3f, o->Light);
            CreateParticle(BITMAP_SMOKE, o->Position, o->Angle, o->Light, 46, o->Scale);
        }
    }
    break;
    case 54:
    {
        if (rand_fps_check(3))
        {
            o->HiddenMesh = -1;
            CreateJoint(BITMAP_JOINT_THUNDER, o->Position, o->Position, o->Angle, 19, NULL, o->Scale * 10.0f);
        }
    }
    break;
    }
    return true;
}

bool M39Kanturu3rd::RenderKanturu3rdObjectMesh(const OBJECT* o, DrawLook& look, BMD* b, bool ExtraMon)
{
    if (IsInKanturu3rd())
    {
        switch (o->Type)
        {
        case 0: // 마야
        {
            if (g_Direction.m_CKanturu.GetMayaExplotion())
                VectorCopy(o->Light, b->BodyLight);

            b->RenderMesh(0, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                          look.blendMeshV);
            b->RenderMesh(2, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                          look.blendMeshV);
            b->RenderMesh(3, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                          look.blendMeshV);
            b->RenderMesh(4, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                          look.blendMeshV);
            b->RenderMesh(5, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                          look.blendMeshV);
            b->RenderMesh(6, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                          look.blendMeshV);
            b->RenderMesh(7, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                          look.blendMeshV);
            b->RenderMesh(8, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                          look.blendMeshV);

            if (g_Direction.m_CKanturu.GetMayaExplotion())
            {
                VectorCopy(o->Light, b->BodyLight);
            }
            else
            {
                Vector(1.0f, 1.0f, 1.0f, b->BodyLight);
            }
            b->RenderMesh(1, RENDER_TEXTURE | RENDER_CHROME, look.alpha, look.blendMesh, look.blendMeshLight,
                          look.blendMeshU, look.blendMeshV, BITMAP_CHROME);

            // Her body's glow, which the update keeps in StartPosition.
            VectorCopy(o->StartPosition, b->BodyLight);

            b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, look.blendMesh, look.blendMeshLight,
                          look.blendMeshU, look.blendMeshV, BITMAP_MAYA_BODY);
            if (g_Direction.m_CKanturu.GetMayaExplotion())
            {
                VectorCopy(o->Light, b->BodyLight);
            }
            else
            {
                Vector(1.0f, 1.0f, 1.0f, b->BodyLight);
            }
            b->RenderMesh(1, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                          look.blendMeshV);
        }
        return true;
        case 2:
        {
            b->RenderMesh(0, RENDER_CHROME, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                          look.blendMeshV, BITMAP_CHROME2);
            Vector(0.3f, 0.3f, 0.3f, b->BodyLight);
            b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, look.blendMesh, look.blendMeshLight,
                          look.blendMeshU, look.blendMeshV);
        }
        return true;
        case 15:
        case 16:
        case 17:
        {
            b->RenderMesh(0, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                          look.blendMeshV);
            b->RenderMesh(1, RENDER_TEXTURE, look.alpha, 1, look.blendMeshLight, look.blendMeshU,
                          -(int)WorldTime % 10000 * 0.0005f);
        }
        return true;
        case 33:
        {
            b->RenderMesh(0, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                          look.blendMeshV);
            b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, 1, look.blendMeshLight, look.blendMeshU,
                          -(int)WorldTime % 2000 * 0.0005f);
        }
        return true;
        case 40:
        case 41:
        case 42:
        {
            Vector(0.6f, 0.7f, 1.f, b->BodyLight);
            b->RenderMesh(1, RENDER_CHROME, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                          look.blendMeshV);
            Vector(0.7f, 0.6, 0.7f, b->BodyLight);
            b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, 0, look.blendMeshLight, look.blendMeshU,
                          -(int)WorldTime % 200000 * 0.00001f);
        }
        return true;
        case 43:
        {
            vec3_t Light;
            VectorCopy(b->BodyLight, Light);
            b->RenderMesh(2, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                          look.blendMeshV);
            Vector(1.0f, 1.0f, 1.0f, b->BodyLight);
            b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, 0, look.blendMeshLight, look.blendMeshU,
                          -0.1f + (int)WorldTime % 5000 * 0.0001f, BITMAP_KANTURU3RD_OBJECT);
            VectorCopy(Light, b->BodyLight);
            b->RenderMesh(0, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                          look.blendMeshV);
            b->BodyLight[0] = sinf(WorldTime * 0.0015f) * 0.2f + 0.4f;
            b->BodyLight[1] = sinf(WorldTime * 0.0015f) * 0.2f + 0.4f;
            b->BodyLight[2] = sinf(WorldTime * 0.0015f) * 0.2f + 0.4f;
            b->RenderMesh(1, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                          look.blendMeshV);
        }
        return true;
        case MODEL_SMELTING_NPC:
        {
            Vector(1.f, 1.f, 1.f, b->BodyLight);
            b->RenderMesh(0, RENDER_CHROME, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                          look.blendMeshV);
            b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, look.blendMesh, look.blendMeshLight,
                          look.blendMeshU, look.blendMeshV);
            Vector(10.f, 10.f, 10.f, b->BodyLight);
            b->RenderMesh(0, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                          look.blendMeshV);
            b->RenderMesh(1, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                          look.blendMeshV);
            b->RenderMesh(2, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                          look.blendMeshV);
            b->RenderMesh(3, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, 3, look.blendMeshLight, look.blendMeshU,
                          (int)WorldTime % 2000 * 0.001f);
        }
        return true;
        case MODEL_MAYASTAR:
        {
            VectorCopy(o->Light, b->BodyLight);
            b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, look.blendMesh, look.blendMeshLight,
                          look.blendMeshU, look.blendMeshV);
            Vector(1.f, 1.f, 1.f, b->BodyLight);
        }
        return true;
        case MODEL_MAYASTONE1:
        {
            b->RenderMesh(1, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                          look.blendMeshV);
        }
        case MODEL_MAYASTONE2:
        case MODEL_MAYASTONE3:
        case MODEL_MAYASTONE4:
        case MODEL_MAYASTONE5:
        case MODEL_MAYASTONEFIRE:
        {
            b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, look.blendMesh, look.blendMeshLight,
                          look.blendMeshU, look.blendMeshV);
        }
        return true;
        case 8:
        case 10:
        case 19:
        case 20:
        case 21:
        case 24:
        case 25:
        case 73:
        {
            SendAfterCharacter(o);
        }
        return true;
        }
    }

    return RenderKanturu3rdMonsterObjectMesh(o, look, b, ExtraMon);
}

void M39Kanturu3rd::RenderKanturu3rdAfterObjectMesh(const OBJECT* o, DrawLook& look, BMD* b, bool ExtraMon)
{
    if (IsInKanturu3rd())
    {
        switch (o->Type)
        {
        case 8:
        {
            b->RenderMesh(1, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                          look.blendMeshV);
            b->RenderMesh(2, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                          look.blendMeshV);
            b->RenderMesh(3, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                          look.blendMeshV);
            b->RenderMesh(4, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                          look.blendMeshV);
            b->RenderMesh(5, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                          look.blendMeshV);
            b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, look.blendMesh, look.blendMeshLight,
                          look.blendMeshU, look.blendMeshV);
        }
        break;
        case 10:
        {
            b->RenderMesh(1, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                          look.blendMeshV);
            b->RenderMesh(2, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                          look.blendMeshV);
            b->RenderMesh(3, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                          look.blendMeshV);
            b->RenderMesh(4, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                          look.blendMeshV);
            b->RenderMesh(5, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                          look.blendMeshV);
            b->RenderMesh(6, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                          look.blendMeshV);
            b->RenderMesh(7, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                          look.blendMeshV);
            b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, look.blendMesh, look.blendMeshLight,
                          look.blendMeshU, look.blendMeshV);
        }
        break;
        case 19:
        {
            look.hiddenMesh = 7;
            b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                          look.blendMeshV, look.hiddenMesh);
            look.hiddenMesh = -1;

            b->RenderMesh(7, RENDER_TEXTURE, look.alpha, 7, look.blendMeshLight, look.blendMeshU,
                          -(int)WorldTime % 10000 * 0.0001f);
        }
        break;
        case 20:
        {
            look.hiddenMesh = 6;
            b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                          look.blendMeshV, look.hiddenMesh);
            look.hiddenMesh = -1;

            b->RenderMesh(6, RENDER_TEXTURE, look.alpha, 6, look.blendMeshLight, look.blendMeshU,
                          -(int)WorldTime % 10000 * 0.0001f);
        }
        break;
        case 21:
        {
            b->BodyLight[0] = sinf(WorldTime * 0.002f) * 0.2f + 0.5f;
            b->BodyLight[1] = sinf(WorldTime * 0.002f) * 0.2f + 0.5f;
            b->BodyLight[2] = sinf(WorldTime * 0.002f) * 0.2f + 0.5f;
            b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, look.blendMesh, look.blendMeshLight,
                          look.blendMeshU, look.blendMeshV);
        }
        break;
        case 24:
        case 25:
        {
            b->RenderMesh(0, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                          look.blendMeshV);
            b->RenderMesh(2, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                          look.blendMeshV);
            b->RenderMesh(3, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                          look.blendMeshV);
            b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, look.blendMesh, look.blendMeshLight,
                          look.blendMeshU, look.blendMeshV);
        }
        break;
        case 73:
        {
            if (!KanturuSuccessMap)
            {
                b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, 0, look.blendMeshLight, look.blendMeshU,
                              (int)WorldTime % 10000 * 0.0002f);
                b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, 0, look.blendMeshLight,
                              (int)WorldTime % 10000 * 0.0002f, look.blendMeshV);
            }
        }
        break;
        case MODEL_STORM3:
        {
            VectorCopy(o->Light, b->BodyLight);
            b->RenderMesh(0, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                          look.blendMeshV);
            b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, look.blendMesh, look.blendMeshLight,
                          look.blendMeshU, look.blendMeshV);
            b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, look.blendMesh, look.blendMeshLight,
                          look.blendMeshU, look.blendMeshV);
            Vector(1.f, 1.f, 1.f, b->BodyLight);
        }
        break;
        case MODEL_MAYAHANDSKILL:
        {
            VectorCopy(o->Light, b->BodyLight);
            b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, 0, look.blendMeshLight, look.blendMeshU,
                          -(int)WorldTime % 2000 * 0.001f);
            b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, 1, look.blendMeshLight,
                          -(int)WorldTime % 2000 * 0.001f, look.blendMeshV);
        }
        break;
        }
    }
}

bool M39Kanturu3rd::MoveKanturu3rdMonsterVisual(OBJECT* o, BMD* b)
{
    if (!IsInKanturu3rd())
        return false;

    vec3_t Position, Direction, Angle, Light;

    switch (o->Type)
    {
    case MODEL_DREADFEAR:
        // A dying Dreadfear fades out.
        if (o->CurrentAction == MONSTER01_DIE)
            o->Alpha = std::max(o->Alpha - 0.1f * static_cast<float>(FPS_ANIMATION_FACTOR), 0.f);
        break;
    case MODEL_DARK_SKULL_SOLDIER_5:
    {
        if (o->CurrentAction == MONSTER01_WALK || o->CurrentAction == MONSTER01_RUN)
        {
            if (rand_fps_check(15))
                PlayBuffer(static_cast<ESound>(SOUND_KANTURU_3RD_NIGHTMARE_IDLE1 + Random::Rand() % 2));
        }
        else if (o->CurrentAction == MONSTER01_ATTACK1)
        {
            if (o->SubType == FALSE) {
                o->SubType = TRUE;
                PlayBuffer(SOUND_KANTURU_3RD_NIGHTMARE_ATT1);
            }
        }
        else if (o->CurrentAction == MONSTER01_ATTACK2)
        {
            if (o->SubType == FALSE) {
                o->SubType = TRUE;
                PlayBuffer(SOUND_KANTURU_3RD_NIGHTMARE_ATT2);
            }
        }
        else if (o->CurrentAction == MONSTER01_ATTACK3)
        {
            if (o->SubType == FALSE) {
                o->SubType = TRUE;
                PlayBuffer(SOUND_KANTURU_3RD_NIGHTMARE_ATT3);
            }
        }
        else if (o->CurrentAction == MONSTER01_ATTACK4)
        {
            if (o->SubType == FALSE) {
                o->SubType = TRUE;
                PlayBuffer(SOUND_KANTURU_3RD_NIGHTMARE_ATT4);
            }
        }
        else if (o->CurrentAction == MONSTER01_DIE)
        {
            if (o->SubType == FALSE) {
                o->SubType = TRUE;
                PlayBuffer(SOUND_KANTURU_3RD_NIGHTMARE_DIE);
            }
        }

        if (o->CurrentAction == MONSTER01_ATTACK3)
        {
            Vector(0.f, 0.f, (float)(Random::Rand() % 360), Angle);

            for (int i = 0; i < 2; i++)
            {
                Position[0] = o->StartPosition[0] + Random::Rand() % 440 - 220;
                Position[1] = o->StartPosition[1] + Random::Rand() % 440 - 220;
                Position[2] = o->StartPosition[2];
                CreateJoint(BITMAP_FLARE, Position, Position, Angle, 2, NULL, 40);
            }
        }
        else if (o->CurrentAction == MONSTER01_ATTACK2 && (o->AnimationFrame >= 5.2f && o->AnimationFrame <= 5.8f))
        {
            float Matrix[3][4];
            Vector(0.0f, 0.0f, 0.0f, Angle);
            Vector(0.0f, -Random::Rand() % 20 - 40.0f, 0.0f, Direction);
            AngleMatrix(Angle, Matrix);
            VectorRotate(Direction, Matrix, Position);

            Vector(0.5f, 0.7f, 1.0f, Light);
            BoneManager::GetBonePosition(o, L"LHand_Bone", Position, Position);
            CreateEffect(MODEL_STORM2, Position, o->Angle, Light, 0);
            CreateEffect(BITMAP_BOSS_LASER, Position, o->Angle, Light, 2);
        }
        else if (o->CurrentAction == MONSTER01_DIE && o->AnimationFrame >= 3.0f && rand_fps_check(1))
        {
            vec3_t Position;
            BoneManager::GetBonePosition(o, L"Body_Bone13", Position);
            CreateParticle(BITMAP_SMOKE + 3, Position, o->Angle, Light, 3, 1.5f);
            CreateParticle(BITMAP_SMOKE + 3, Position, o->Angle, Light, 3, 1.5f);

            vec3_t Angle;
            Vector((float)(Random::Rand() % 100) - 50.0f, (float)(Random::Rand() % 100) - 50.0f,
                   (float)(Random::Rand() % 360), Angle);
            float Scale = (float)(Random::Rand() % 40) + 30.0f;
            CreateJoint(BITMAP_JOINT_SPIRIT2, Position, Position, Angle, 8, NULL, Scale, 0, 0);
        }
        else if (o->CurrentAction == MONSTER01_STOP1 || o->CurrentAction == MONSTER01_STOP2)
            o->SubType = FALSE;
    }
    break;
    case MODEL_MAYA_HAND_LEFT:
    {
        vec3_t Pos;

        if (o->CurrentAction == MONSTER01_DIE && g_Direction.m_CKanturu.m_iMayaState < KANTURU_MAYA_DIRECTION_MAYA3 && rand_fps_check(1))
        {
            o->BlendMesh = -2;
            Vector(0.0f, 0.0f, 0.0f, Position);
            Vector(0.4f, 0.6f, 1.0f, Light);

            for (int i = 0; i < 5; i++)
            {
                b->TransformPosition(o->BoneTransform[Random::Rand() % 45], Position, Pos, true);
                CreateParticle(BITMAP_WATERFALL_3, Pos, o->Angle, Light, 6, 2.f, o);
            }
        }
        else
            o->BlendMesh = -1;

        if (o->CurrentAction == MONSTER01_STOP2 && g_Direction.m_CKanturu.m_iMayaState >= KANTURU_MAYA_DIRECTION_MAYA3 && rand_fps_check(1))
        {
            Vector(1.0f, 1.0f, 1.0f, Light);
            Vector(0.0f, -50.0f, 0.0f, Position);
            b->TransformPosition(o->BoneTransform[24], Position, Pos, true);
            CreateParticle(BITMAP_SMOKE + 3, Pos, o->Angle, Light, 3, 1.5f);
            CreateParticle(BITMAP_SMOKE + 3, Pos, o->Angle, Light, 4, 0.8f);
        }
    }
    break;
    case MODEL_MAYA_HAND_RIGHT:
    {
        vec3_t Pos;

        if (o->CurrentAction == MONSTER01_DIE && g_Direction.m_CKanturu.m_iMayaState < KANTURU_MAYA_DIRECTION_MAYA3 && rand_fps_check(1))
        {
            o->BlendMesh = -2;
            Vector(0.0f, 0.0f, 0.0f, Position);
            Vector(0.4f, 0.6f, 1.0f, Light);

            for (int i = 0; i < 5; i++)
            {
                b->TransformPosition(o->BoneTransform[Random::Rand() % 45], Position, Pos, true);
                CreateParticle(BITMAP_WATERFALL_3, Pos, o->Angle, Light, 6, 2.f, o);
            }
        }
        else
            o->BlendMesh = -1;

        if (o->CurrentAction == MONSTER01_STOP2 && g_Direction.m_CKanturu.m_iMayaState >= KANTURU_MAYA_DIRECTION_MAYA3 && rand_fps_check(1))
        {
            Vector(1.0f, 1.0f, 1.0f, Light);
            Vector(50.0f, 0.0f, 0.0f, Position);
            b->TransformPosition(o->BoneTransform[12], Position, Pos, true);
            CreateParticle(BITMAP_SMOKE + 3, Pos, o->Angle, Light, 3, 1.5f);
            CreateParticle(BITMAP_SMOKE + 3, Pos, o->Angle, Light, 4, 0.8f);
        }
    }
    break;
    case MODEL_MAYA:
    {
    }
    break;
    case MODEL_SMELTING_NPC:
    {
        o->Angle[2] = 0.0f;
        o->Position[2] = -60.0f + sinf(WorldTime * 0.002f) * 5.8f;
    }
    break;
    }

    return false;
}

void M39Kanturu3rd::MoveKanturu3rdBlurEffect(CHARACTER* c, OBJECT* o, BMD* b)
{
    if (!IsInKanturu3rd())
        return;

    switch (o->Type)
    {
    case MODEL_DARK_SKULL_SOLDIER_5:
    {
        if ((o->CurrentAction == MONSTER01_ATTACK1 || o->CurrentAction == MONSTER01_ATTACK4) && o->AnimationFrame <= 5.9f)
        {
            vec3_t  Light;
            Vector(1.0f, 1.0f, 1.0f, Light);

            vec3_t StartPos, StartRelative;
            vec3_t EndPos, EndRelative;

            float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed * static_cast<float>(FPS_ANIMATION_FACTOR);
            float fSpeedPerFrame = fActionSpeed / 10.f;
            float fAnimationFrame = o->AnimationFrame - fActionSpeed;
            for (int i = 0; i < 10; i++)
            {
                b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);

                fAnimationFrame += fSpeedPerFrame;

                Vector(10.f, 0.f, 0.f, StartRelative);
                Vector(10.f, -10.f, 0.f, EndRelative);

                b->TransformPosition(BoneTransform[32], StartRelative, StartPos, false);
                b->TransformPosition(BoneTransform[38], EndRelative, EndPos, false);
                CreateBlur(c, StartPos, EndPos, Light, 3, true, 80);
            }
        }
    }
    break;
    case MODEL_MAYA_HAND_LEFT:
    {
    }
    break;
    case MODEL_MAYA_HAND_RIGHT:
    {
    }
    break;
    case MODEL_MAYA:
    {
    }
    break;
    }
}

bool M39Kanturu3rd::UpdateKanturu3rdMonsterEffects(CHARACTER* c, OBJECT* o, BMD* b)
{
    if (!IsInKanturu3rd())
        return false;

    vec3_t Position, Light;

    switch (o->Type)
    {
    case MODEL_DARK_SKULL_SOLDIER_5:
    {
        if (o->CurrentAction == MONSTER01_DIE)
            return true;

        MoveEye(o, b, 9, 10, 39, 40);

        Vector(1.0f, 1.0f, 1.0f, Light);
        Vector(0.0f, 0.0f, 0.0f, Position);

        BoneManager::GetBonePosition(o, L"Body_Bone1", Position);
        CreateSprite(BITMAP_LIGHT, Position, 0.3f, Light, o);
        CreateSprite(BITMAP_FLARE_BLUE, Position, 0.5f, Light, o);
        BoneManager::GetBonePosition(o, L"Body_Bone2", Position);
        CreateSprite(BITMAP_LIGHT, Position, 0.3f, Light, o);
        CreateSprite(BITMAP_FLARE_BLUE, Position, 0.5f, Light, o);
        BoneManager::GetBonePosition(o, L"Body_Bone3", Position);
        CreateSprite(BITMAP_LIGHT, Position, 0.3f, Light, o);
        CreateSprite(BITMAP_FLARE_BLUE, Position, 0.5f, Light, o);
        BoneManager::GetBonePosition(o, L"Body_Bone4", Position);
        CreateSprite(BITMAP_LIGHT, Position, 0.3f, Light, o);
        CreateSprite(BITMAP_FLARE_BLUE, Position, 0.5f, Light, o);
        Vector(5.0f, 0.0f, 0.0f, Position);
        BoneManager::GetBonePosition(o, L"Body_Bone5", Position, Position);
        CreateSprite(BITMAP_LIGHT, Position, 0.3f, Light, o);
        CreateSprite(BITMAP_FLARE_BLUE, Position, 0.5f, Light, o);
        Vector(9.0f, 0.0f, 0.0f, Position);
        BoneManager::GetBonePosition(o, L"Body_Bone6", Position, Position);
        CreateSprite(BITMAP_LIGHT, Position, 0.3f, Light, o);
        CreateSprite(BITMAP_FLARE_BLUE, Position, 0.5f, Light, o);
        Vector(6.0f, 0.0f, 0.0f, Position);
        BoneManager::GetBonePosition(o, L"Body_Bone7", Position, Position);
        CreateSprite(BITMAP_LIGHT, Position, 0.4f, Light, o);
        CreateSprite(BITMAP_FLARE_BLUE, Position, 0.7f, Light, o);
        BoneManager::GetBonePosition(o, L"Body_Bone8", Position);
        CreateSprite(BITMAP_LIGHT, Position, 0.4f, Light, o);
        CreateSprite(BITMAP_FLARE_BLUE, Position, 0.7f, Light, o);
        BoneManager::GetBonePosition(o, L"Body_Bone9", Position);
        CreateSprite(BITMAP_LIGHT, Position, 0.4f, Light, o);
        CreateSprite(BITMAP_FLARE_BLUE, Position, 0.7f, Light, o);
        BoneManager::GetBonePosition(o, L"Body_Bone10", Position);
        CreateSprite(BITMAP_LIGHT, Position, 0.4f, Light, o);
        CreateSprite(BITMAP_FLARE_BLUE, Position, 0.7f, Light, o);
        Vector(10.0f, 0.0f, 0.0f, Position);
        BoneManager::GetBonePosition(o, L"Body_Bone11", Position, Position);
        CreateSprite(BITMAP_LIGHT, Position, 0.4f, Light, o);
        CreateSprite(BITMAP_FLARE_BLUE, Position, 0.7f, Light, o);
        BoneManager::GetBonePosition(o, L"Body_Bone12", Position);
        CreateSprite(BITMAP_LIGHT, Position, 0.4f, Light, o);
        CreateSprite(BITMAP_FLARE_BLUE, Position, 0.7f, Light, o);

        BoneManager::GetBonePosition(o, L"Body_Bone13", Position);
        CreateParticleFpsChecked(BITMAP_FIRE + 1, Position, o->Angle, Light, 3, 1.7f);

        Vector(3.0f, 0.0f, 0.0f, Position);
        BoneManager::GetBonePosition(o, L"Sword_Bone1", Position, Position);
        CreateSprite(BITMAP_LIGHT, Position, 0.3f, Light, o);
        CreateSprite(BITMAP_FLARE_BLUE, Position, 0.6f, Light, o);
        Vector(3.0f, 0.0f, 0.0f, Position);
        BoneManager::GetBonePosition(o, L"Sword_Bone2", Position, Position);
        CreateSprite(BITMAP_LIGHT, Position, 0.3f, Light, o);
        CreateSprite(BITMAP_FLARE_BLUE, Position, 0.6f, Light, o);

        Vector(0.0f, -2.0f, 0.0f, Position);
        BoneManager::GetBonePosition(o, L"Eye_Bone1", Position, Position);
        CreateSprite(BITMAP_FLARE_BLUE, Position, 0.3f, Light, o);
        Vector(4.0f, -4.0f, 0.0f, Position);
        BoneManager::GetBonePosition(o, L"Eye_Bone2", Position, Position);
        CreateSprite(BITMAP_FLARE_BLUE, Position, 0.3f, Light, o);

        BoneManager::GetBonePosition(o, L"Windmill_Bone1", Position);
        if (rand_fps_check(2))
            CreateParticle(BITMAP_SPARK + 1, Position, o->Angle, Light, 16, 1.0f);
    }
    return true;
    case MODEL_MAYA_HAND_LEFT:
    {
        o->m_bRenderShadow = false;

        if (o->CurrentAction == MONSTER01_DIE)
            return true;

        MoveEye(o, b, 5, 11, 17, 29, 23);

        Vector(0.5f, 0.5f, 0.8f, Light);

        Vector(0.0f, 0.0f, 0.0f, Position);
        BoneManager::GetBonePosition(o, L"L_Hand01", Position, Position);
        CreateSprite(BITMAP_FLARE_BLUE, Position, 1.3f, Light, o);
        Vector(-10.0f, 0.0f, 3.0f, Position);
        BoneManager::GetBonePosition(o, L"L_Hand01", Position, Position);
        CreateSprite(BITMAP_FLARE_BLUE, Position, 0.7f, Light, o);

        Vector(0.0f, 0.0f, 0.0f, Position);
        BoneManager::GetBonePosition(o, L"L_Hand02", Position, Position);
        CreateSprite(BITMAP_FLARE_BLUE, Position, 1.3f, Light, o);
        Vector(-12.0f, 5.0f, 0.0f, Position);
        BoneManager::GetBonePosition(o, L"L_Hand02", Position, Position);
        CreateSprite(BITMAP_FLARE_BLUE, Position, 0.7f, Light, o);

        Vector(0.0f, 0.0f, 0.0f, Position);
        BoneManager::GetBonePosition(o, L"L_Hand03", Position, Position);
        CreateSprite(BITMAP_FLARE_BLUE, Position, 1.3f, Light, o);
        Vector(-12.0f, 5.0f, 0.0f, Position);
        BoneManager::GetBonePosition(o, L"L_Hand03", Position, Position);
        CreateSprite(BITMAP_FLARE_BLUE, Position, 0.7f, Light, o);

        Vector(0.0f, 0.0f, 0.0f, Position);
        BoneManager::GetBonePosition(o, L"L_Hand04", Position, Position);
        CreateSprite(BITMAP_FLARE_BLUE, Position, 1.3f, Light, o);
        Vector(-10.0f, 5.0f, 0.0f, Position);
        BoneManager::GetBonePosition(o, L"L_Hand04", Position, Position);
        CreateSprite(BITMAP_FLARE_BLUE, Position, 0.7f, Light, o);

        Vector(0.0f, 0.0f, 0.0f, Position);
        BoneManager::GetBonePosition(o, L"L_Hand05", Position, Position);
        CreateSprite(BITMAP_FLARE_BLUE, Position, 1.3f, Light, o);
        Vector(-8.0f, 9.0f, 0.0f, Position);
        BoneManager::GetBonePosition(o, L"L_Hand05", Position, Position);
        CreateSprite(BITMAP_FLARE_BLUE, Position, 0.7f, Light, o);

        Vector(0.0f, 0.0f, 0.0f, Position);
        BoneManager::GetBonePosition(o, L"L_Hand21", Position, Position);
        CreateSprite(BITMAP_FLARE_BLUE, Position, 1.0f, Light, o);
        Vector(0.0f, 0.0f, 0.0f, Position);
        BoneManager::GetBonePosition(o, L"L_Hand22", Position, Position);
        CreateSprite(BITMAP_FLARE_BLUE, Position, 1.0f, Light, o);
        Vector(0.0f, 0.0f, 0.0f, Position);
        BoneManager::GetBonePosition(o, L"L_Hand23", Position, Position);
        CreateSprite(BITMAP_FLARE_BLUE, Position, 1.0f, Light, o);
        Vector(0.0f, 0.0f, 0.0f, Position);
        BoneManager::GetBonePosition(o, L"L_Hand24", Position, Position);
        CreateSprite(BITMAP_FLARE_BLUE, Position, 1.0f, Light, o);
        Vector(0.0f, 0.0f, 0.0f, Position);
        BoneManager::GetBonePosition(o, L"L_Hand25", Position, Position);
        CreateSprite(BITMAP_FLARE_BLUE, Position, 1.0f, Light, o);
    }
    return true;
    case MODEL_MAYA_HAND_RIGHT:
    {
        o->m_bRenderShadow = false;

        if (o->CurrentAction == MONSTER01_DIE)
            return true;

        MoveEye(o, b, 5, 20, 31, 42, 53);

        Vector(0.5f, 0.3f, 0.3f, Light);

        Vector(-2.0f, 0.0f, 4.0f, Position);
        BoneManager::GetBonePosition(o, L"R_Hand01", Position, Position);
        CreateSprite(BITMAP_FLARE_RED, Position, 1.5f, Light, o);
        Vector(-10.0f, -3.0f, 3.0f, Position);
        BoneManager::GetBonePosition(o, L"R_Hand01", Position, Position);
        CreateSprite(BITMAP_FLARE_RED, Position, 0.7f, Light, o);

        Vector(-1.0f, -2.0f, 4.0f, Position);
        BoneManager::GetBonePosition(o, L"R_Hand02", Position, Position);
        CreateSprite(BITMAP_FLARE_RED, Position, 1.5f, Light, o);
        Vector(-10.0f, -3.0f, 5.0f, Position);
        BoneManager::GetBonePosition(o, L"R_Hand02", Position, Position);
        CreateSprite(BITMAP_FLARE_RED, Position, 0.7f, Light, o);

        Vector(0.0f, 0.0f, 4.0f, Position);
        BoneManager::GetBonePosition(o, L"R_Hand03", Position, Position);
        CreateSprite(BITMAP_FLARE_RED, Position, 1.3f, Light, o);
        Vector(-10.0f, 0.0f, 4.0f, Position);
        BoneManager::GetBonePosition(o, L"R_Hand03", Position, Position);
        CreateSprite(BITMAP_FLARE_RED, Position, 0.7f, Light, o);

        Vector(0.0f, 0.0f, 4.0f, Position);
        BoneManager::GetBonePosition(o, L"R_Hand04", Position, Position);
        CreateSprite(BITMAP_FLARE_RED, Position, 1.3f, Light, o);
        Vector(-10.0f, 1.0f, 4.0f, Position);
        BoneManager::GetBonePosition(o, L"R_Hand04", Position, Position);
        CreateSprite(BITMAP_FLARE_RED, Position, 0.7f, Light, o);

        Vector(6.0f, 3.0f, 3.0f, Position);
        BoneManager::GetBonePosition(o, L"R_Hand05", Position, Position);
        CreateSprite(BITMAP_FLARE_RED, Position, 1.3f, Light, o);
        Vector(-5.0f, 10.0f, 5.0f, Position);
        BoneManager::GetBonePosition(o, L"R_Hand05", Position, Position);
        CreateSprite(BITMAP_FLARE_RED, Position, 0.7f, Light, o);

        Vector(0.0f, 0.0f, -5.0f, Position);
        BoneManager::GetBonePosition(o, L"R_Hand11", Position, Position);
        CreateSprite(BITMAP_FLARE_RED, Position, 1.0f, Light, o);
        Vector(0.0f, 0.0f, -2.0f, Position);
        BoneManager::GetBonePosition(o, L"R_Hand12", Position, Position);
        CreateSprite(BITMAP_FLARE_RED, Position, 1.0f, Light, o);
        Vector(0.0f, 0.0f, -5.0f, Position);
        BoneManager::GetBonePosition(o, L"R_Hand13", Position, Position);
        CreateSprite(BITMAP_FLARE_RED, Position, 1.0f, Light, o);
        Vector(0.0f, 0.0f, -5.0f, Position);
        BoneManager::GetBonePosition(o, L"R_Hand14", Position, Position);
        CreateSprite(BITMAP_FLARE_RED, Position, 1.0f, Light, o);
        Vector(10.0f, -4.0f, 0.0f, Position);
        BoneManager::GetBonePosition(o, L"R_Hand15", Position, Position);
        CreateSprite(BITMAP_FLARE_RED, Position, 1.0f, Light, o);
    }
    return true;
    case MODEL_MAYA:
    {
    }
    return true;
    case MODEL_SMELTING_NPC:
    {
        o->Scale = 2.5f;
        Position[0] = o->Position[0] + (float)(Random::Rand() % 250 - 125);
        Position[1] = o->Position[1] + (float)(Random::Rand() % 250 - 125);
        Position[2] = o->Position[2] - (float)(Random::Rand() % 100) + 50.0f;

        if (rand_fps_check(10))
        {
            Vector(0.5f, 1.0f, 0.8f, Light);
            CreateParticle(BITMAP_SPARK + 1, Position, o->Angle, Light, 17, 0.6f);
        }
    }
    return true;
    }

    return false;
}

bool M39Kanturu3rd::RenderKanturu3rdMonsterObjectMesh(const OBJECT* o, DrawLook& look, BMD* b, bool ExtraMon)
{
    switch (o->Type)
    {
    case MODEL_PERSONA:
    {
        float fLumi2 = (sinf(WorldTime * 0.002f) + 1.f) * 0.5f;
        b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                      look.hiddenMesh);
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, 0, fLumi2, look.blendMeshU, look.blendMeshV,
                      BITMAP_PRSONA_EFFECT);
        b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, 2, fLumi2, look.blendMeshU, look.blendMeshV,
                      BITMAP_PRSONA_EFFECT2);
        return true;
    }
    break;
    case MODEL_TWIN_TAIL:
    {
        float fLumi = (sinf(WorldTime * 0.002f) + 1.f) * 0.5f;
        float fLumi2 = (sinf(WorldTime * 0.002f) + 1.f);

        if (o->CurrentAction != MONSTER01_DIE)
        {
            Vector(0.9f, 0.9f, 1.0f, b->BodyLight);
        }
        else
        {
            Vector(0.3f, 1.0f, 0.2f, b->BodyLight);
        }

        b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                      look.hiddenMesh);
        b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, 2, fLumi2, look.blendMeshU, look.blendMeshV,
                      BITMAP_TWINTAIL_EFFECT);
        b->RenderMesh(3, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, 3, fLumi);
        return true;
    }
    break;
    case MODEL_DREADFEAR:
    {
        b->RenderBody(RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU, look.blendMeshV,
                      look.hiddenMesh);
        return true;
    }
    break;
    case MODEL_DARK_SKULL_SOLDIER_5:
    {
        b->BeginRender(1.f);

        b->BodyLight[0] = 1.0f;
        b->BodyLight[1] = 1.0f;
        b->BodyLight[2] = 1.0f;

        b->RenderMesh(0, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                      look.blendMeshV);
        b->RenderMesh(1, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                      look.blendMeshV);
        b->RenderMesh(2, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                      look.blendMeshV);
        b->RenderMesh(3, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                      look.blendMeshV);

        float fLumi = (sinf(WorldTime * 0.003f) + 1.f) * 0.35f;

        b->BodyLight[0] = fLumi;
        b->BodyLight[1] = fLumi;
        b->BodyLight[2] = fLumi;

        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, look.blendMesh, look.blendMeshLight,
                      look.blendMeshU, look.blendMeshV, BITMAP_NIGHTMARE_EFFECT2);
        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, look.blendMesh, look.blendMeshLight,
                      look.blendMeshU, look.blendMeshV, BITMAP_NIGHTMARE_EFFECT1);

        if (o->CurrentAction == MONSTER01_ATTACK1 || o->CurrentAction == MONSTER01_ATTACK2)
        {
            b->BodyLight[0] = 0.7f;
            b->BodyLight[1] = 0.7f;
            b->BodyLight[2] = 1.0f;

            b->RenderMesh(2, RENDER_BRIGHT | RENDER_CHROME2, look.alpha, look.blendMesh, look.blendMeshLight,
                          look.blendMeshU, look.blendMeshV);
            b->RenderMesh(3, RENDER_BRIGHT | RENDER_CHROME2, look.alpha, look.blendMesh, look.blendMeshLight,
                          look.blendMeshU, look.blendMeshV);
        }

        b->EndRender();
    }
    return true;
    case MODEL_MAYA_HAND_LEFT:
    {
        b->BeginRender(1.f);

        b->RenderMesh(0, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                      look.blendMeshV);
        b->RenderMesh(1, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                      look.blendMeshV);
        b->RenderMesh(2, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                      look.blendMeshV);
        b->RenderMesh(2, RENDER_BRIGHT | RENDER_CHROME, look.alpha, look.blendMesh, look.blendMeshLight,
                      look.blendMeshU, look.blendMeshV, BITMAP_CHROME);

        b->EndRender();
    }
    return true;
    case MODEL_MAYA_HAND_RIGHT:
    {
        b->BeginRender(1.f);

        b->RenderMesh(0, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                      look.blendMeshV);
        b->RenderMesh(2, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                      look.blendMeshV);
        b->RenderMesh(1, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                      look.blendMeshV);
        b->RenderMesh(1, RENDER_BRIGHT | RENDER_CHROME, look.alpha, look.blendMesh, look.blendMeshLight,
                      look.blendMeshU, look.blendMeshV);

        b->EndRender();
    }
    return true;
    case MODEL_SUMMON:
    case MODEL_STORM2:
    {
        VectorCopy(o->Light, b->BodyLight);
        b->RenderMesh(0, RENDER_TEXTURE, look.alpha, look.blendMesh, look.blendMeshLight, look.blendMeshU,
                      look.blendMeshV);
        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, look.blendMesh, look.blendMeshLight,
                      look.blendMeshU, look.blendMeshV);
        b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT, look.alpha, look.blendMesh, look.blendMeshLight,
                      look.blendMeshU, look.blendMeshV);
        Vector(1.f, 1.f, 1.f, b->BodyLight);
    }
    return true;
    }
    return false;
}

bool M39Kanturu3rd::AttackEffectKanturu3rdMonster(CHARACTER* c, OBJECT* o, BMD* b)
{
    if (!IsInKanturu3rd())
        return false;

    vec3_t Light, Position, Direction, Angle;

    switch (c->MonsterIndex)
    {
    case MONSTER_DEATH_SPIRIT:
    {
        if (o->CurrentAction == MONSTER01_ATTACK1 || o->CurrentAction == MONSTER01_ATTACK2)
        {
            if (c->CheckAttackTime(14))
            {
                vec3_t vPos, vRelative, Light;
                Vector(140.f, 0.f, -30.f, vRelative);
                Vector(0.2f, 0.2f, 1.f, Light);
                BoneManager::GetBonePosition(o, L"BLADE_L_HAND", vRelative, vPos);

                CreateEffect(MODEL_BLADE_SKILL, vPos, o->Angle, Light, 0);

                CreateParticle(BITMAP_EXPLOTION + 1, vPos, o->Angle, o->Light, 0, 1.3f);
                CreateParticle(BITMAP_EXPLOTION + 1, vPos, o->Angle, o->Light, 0, 2.3f);
                CreateParticle(BITMAP_EXPLOTION + 1, vPos, o->Angle, o->Light, 0, 1.8f);
                CreateParticle(BITMAP_EXPLOTION + 1, vPos, o->Angle, o->Light, 0, 1.3f);
                CreateParticle(BITMAP_EXPLOTION + 1, vPos, o->Angle, o->Light, 0, 2.3f);
                CreateParticle(BITMAP_EXPLOTION + 1, vPos, o->Angle, o->Light, 0, 1.8f);
                c->SetLastAttackEffectTime();
            }
        }
    }
    return true;
    case MONSTER_NIGHTMARE:
    {
        if (o->CurrentAction == MONSTER01_ATTACK3 && c->AttackTime >= 14 && rand_fps_check(1))
        {
            Vector(0.3f, 0.2f, 0.1f, Light);
            CreateEffect(MODEL_SUMMON, o->Position, o->Angle, Light, 3);
        }
        else if (o->CurrentAction == MONSTER01_ATTACK4 && c->AttackTime >= 10 && rand_fps_check(1))
        {
            Vector(1.0f, 1.0f, 1.0f, Light);
            CreateInferno(o->Position);
            if (c->CheckAttackTime(10))
            {
                CreateEffect(MODEL_CIRCLE, o->Position, o->Angle, Light, 4, o);
                c->SetLastAttackEffectTime();
            }
            else if (c->CheckAttackTime(14))
            {
                CreateEffect(MODEL_CIRCLE, o->Position, o->Angle, Light, 4, o);
                c->SetLastAttackEffectTime();
            }
        }
    }
    return true;
    case MONSTER_MAYA_HAND_LEFT:
    {
        if (o->CurrentAction == MONSTER01_ATTACK1 && c->CheckAttackTime(14))
        {
            CreateInferno(o->Position, 2);
            Vector(0.0f, 0.5f, 1.0f, Light);
            CreateEffect(BITMAP_SHOCK_WAVE, o->Position, o->Angle, Light, 7);

            PlayBuffer(SOUND_KANTURU_3RD_MAYAHAND_ATTACK1);
            c->SetLastAttackEffectTime();
        }
        else if (o->CurrentAction == MONSTER01_ATTACK2 && c->CheckAttackTime(14))
        {
            float Matrix[3][4];
            Vector(0.0f, 0.0f, 0.0f, Angle);
            Vector(0.0f, -160.0f, 0.0f, Direction);
            AngleMatrix(Angle, Matrix);
            VectorRotate(Direction, Matrix, Position);
            VectorAdd(o->Position, Position, Position);

            Vector(0.3f, 0.5f, 1.0f, Light);
            CreateEffect(MODEL_MAYAHANDSKILL, Position, o->Angle, Light, 0, NULL, -1, 0, 0, 0, 1.0f);

            PlayBuffer(SOUND_KANTURU_3RD_MAYAHAND_ATTACK2);
            c->SetLastAttackEffectTime();
        }
    }
    return true;
    case MONSTER_MAYA_HAND_RIGHT:
    {
        if (o->CurrentAction == MONSTER01_ATTACK1 && c->CheckAttackTime(14))
        {
            CreateInferno(o->Position, 3);
            Vector(1.0f, 0.5f, 0.0f, Light);
            CreateEffect(BITMAP_SHOCK_WAVE, o->Position, o->Angle, Light, 7);

            PlayBuffer(SOUND_KANTURU_3RD_MAYAHAND_ATTACK1);
            c->SetLastAttackEffectTime();
        }
        else if (o->CurrentAction == MONSTER01_ATTACK2 && c->CheckAttackTime(14))
        {
            float Matrix[3][4];
            Vector(0.0f, 0.0f, 0.0f, Angle);
            Vector(0.0f, -160.0f, 0.0f, Direction);
            AngleMatrix(Angle, Matrix);
            VectorRotate(Direction, Matrix, Position);
            VectorAdd(o->Position, Position, Position);

            Vector(1.0f, 0.6f, 0.4f, Light);
            CreateEffect(MODEL_MAYAHANDSKILL, Position, o->Angle, Light, 0, NULL, -1, 0, 0, 0, 1.0f);

            PlayBuffer(SOUND_KANTURU_3RD_MAYAHAND_ATTACK2);
            c->SetLastAttackEffectTime();
        }
    }
    return true;
    case MONSTER_MAYA:
        return true;
    default:
        break;
    }

    return false;
}

bool M39Kanturu3rd::SetCurrentActionKanturu3rdMonster(CHARACTER* c, OBJECT* o)
{
    if (!IsInKanturu3rd())
        return false;

    // Maya's scene reads her skill before her attacks play (monsters design,
    // Q-MN64); the other monsters' attacks play as anywhere.
    if (c->MonsterIndex == MONSTER_MAYA)
    {
        MayaSceneMayaAction(c->MonsterSkill);
        GameLogic::Monsters::Attacks::Play(*c, *o);
        return true;
    }
    return false;
}

void M39Kanturu3rd::MayaSceneMayaAction(BYTE Skill)
{
    iMayaAction = Skill;

    switch (iMayaAction)
    {
    case 0:
        break;
    case 1:
        bMayaSkill2 = true;
        break;
    }
}

void M39Kanturu3rd::MayaAction(OBJECT* o, BMD* b)
{
    vec3_t Angle, Direction, Position, Pos, Light;

    if (g_Direction.m_CKanturu.GetMayaExplotion())
        iMayaAction = 2;

    switch (iMayaAction)
    {
    case 0:
    {
        float Matrix[3][4];

        Vector(0.0f, 0.0f, 0.0f, Angle);
        Vector(-200.0f + (Random::Rand() % 60 + 50.0f), 0.0f, 0.0f, Direction);
        AngleMatrix(Angle, Matrix);
        VectorRotate(Direction, Matrix, Position);

        Vector(0.2f, 0.2f, 0.4f, Light);
        b->TransformPosition(BoneTransform[34], Position, Pos, false);
        CreateEffect(MODEL_STORM3, Pos, o->Angle, Light);

        for (int i = 0; i < 100; i++)
        {
            Vector(1.0f, 1.0f, 1.0f, Light);
            Pos[0] = Hero->Object.Position[0] + (float)(Random::Rand() % 20 - 10) * 90.0f;
            Pos[1] = Hero->Object.Position[1] + (float)(Random::Rand() % 20 - 10) * 90.0f;
            Pos[2] = Hero->Object.Position[2] - (float)(Random::Rand() % 5) * 100.0f - 500.0f;
            CreateEffect(MODEL_STONE1 + Random::Rand() % 2, Pos, o->Angle, Light, 2);
        }
        iMayaAction = -1;

        PlayBuffer(SOUND_KANTURU_3RD_MAYA_STORM);
    }
    break;
    case 1:
    {
        if (bMayaSkill2)
        {
            if (iMayaSkill2_Counter == 0 || g_Time.GetTimeCheck(0, 1000))
            {
                for (int i = 0; i < 10; i++)
                {
                    Pos[0] = Hero->Object.Position[0] + (float)(Random::Rand() % 20 - 10) * 80.0f + 500.0f;
                    Pos[1] = Hero->Object.Position[1] + (float)(Random::Rand() % 20 - 10) * 80.0f;
                    Pos[2] = Hero->Object.Position[2] + 300.0f + (float)(Random::Rand() % 10) * 100.0f;
                    float Scale = 5.0f + Random::Rand() % 10 / 3.0f;
                    int index = MODEL_MAYASTONE1 + Random::Rand() % 3;
                    Vector(1.0f, 1.0f, 1.0f, Light);
                    CreateEffect(index, Pos, o->Angle, Light, 0, NULL, -1, 0, 0, 0, Scale);
                    CreateEffect(MODEL_MAYASTONEFIRE, Pos, o->Angle, Light, index, NULL, -1, 0, 0, 0, Scale);
                }

                iMayaSkill2_Counter++;
            }

            if (iMayaSkill2_Counter > 3)
            {
                iMayaAction = -1;
                bMayaSkill2 = false;
                iMayaSkill2_Counter = 0;
            }
        }
    }
    break;
    case 2:
    {
        if (g_Direction.m_CKanturu.GetMayaExplotion())
        {
            int Index[11] = { 3, 5, 18, 19, 23, 25, 26, 27, 28, 29, 30 };

            if (iMayaDie_Counter == 0)
            {
                if (rand_fps_check(5))
                {
                    for (int j = 0; j < 3; j++)
                    {
                        Vector(Random::Rand() % 20 - 10.0f, Random::Rand() % 20 - 10.0f, Random::Rand() % 20 - 10.0f,
                               Position);
                        Vector(0.0f, 0.0f, 0.0f, Position);
                        Vector(1.0f, 1.0f, 1.0f, Light);
                        b->TransformPosition(BoneTransform[33 + Random::Rand() % 42], Position, Pos, false);
                        CreateParticle(BITMAP_EXPLOTION, Pos, o->Angle, Light, 1, (1.5f + Random::Rand() % 10 / 20.0f));
                    }
                }

                if (g_Time.GetTimeCheck(1, 1200))
                    iMayaDie_Counter++;

                PlayBuffer(SOUND_KANTURU_3RD_MAYA_END);
            }
            else if (iMayaDie_Counter == 1 || iMayaDie_Counter == 3)
            {
                Vector(0.0f, -100.0f, 0.0f, Position);
                Vector(1.0f, 1.0f, 1.0f, Light);
                b->TransformPosition(BoneTransform[11], Position, Pos, false);
                CreateEffect(MODEL_MAYASTAR, Pos, o->Angle, Light);

                Vector(0.0f, -0.0f, 0.0f, Position);
                b->TransformPosition(BoneTransform[21], Position, Pos, false);
                CreateEffect(MODEL_MAYASTAR, Pos, o->Angle, Light);

                Vector(0.0f, -0.0f, 0.0f, Position);
                b->TransformPosition(BoneTransform[24], Position, Pos, false);
                CreateEffect(MODEL_MAYASTAR, Pos, o->Angle, Light);

                iMayaDie_Counter++;
            }
            else if (iMayaDie_Counter >= 5)
            {
                iMayaAction = -1;
                g_Direction.m_CKanturu.SetMayaExplotion(false);
                iMayaDie_Counter = 0;
            }
            else
            {
                for (int i = 0; i < 3; i++)
                {
                    Vector(1.0f, 1.0f, 1.0f, Light);
                    Vector(Random::Rand() % 100 - 50.0f, Random::Rand() % 100 - 50.0f, Random::Rand() % 100 - 50.0f,
                           Position);
                    b->TransformPosition(BoneTransform[Index[Random::Rand() % 11]], Position, Pos, false);
                    CreateParticle(BITMAP_EXPLOTION, Pos, o->Angle, Light, 1,
                                   1.0f + (i * 1.5f) + Random::Rand() % 10 / 10.0f);

                    Vector(1.0f, 0.5f, 0.3f, Light);
                    Vector(Random::Rand() % 200 - 100.0f, Random::Rand() % 200 - 100.0f, Random::Rand() % 200 - 100.0f,
                           Position);
                    b->TransformPosition(BoneTransform[Index[Random::Rand() % 11]], Position, Pos, false);
                    CreateParticle(BITMAP_SMOKE, Pos, o->Angle, Light, 47, 1.5f + Random::Rand() % 4);
                }

                if (g_Time.GetTimeCheck(2, 1200))
                    iMayaDie_Counter++;
            }
        }
    }
    break;
    }
}

void M39Kanturu3rd::Kanturu3rdState(BYTE State, BYTE DetailState)
{
    g_Direction.m_CKanturu.GetKanturuAllState(State, DetailState);
}

void M39Kanturu3rd::Kanturu3rdResult(BYTE Result)
{
    switch (Result)
    {
    case 0:
        iKanturuResult = 0;
        break;
    case 1:
    {
        if (g_Direction.m_CKanturu.m_iKanturuState != KANTURU_STATE_NIGHTMARE_BATTLE)
            break;

        iKanturuResult = 1;
    }
    break;
    }
}

void M39Kanturu3rd::Kanturu3rdUserandMonsterCount(int Count1, int Count2)
{
    UserCount = Count2;
    MonsterCount = Count1;
}

void M39Kanturu3rd::RenderKanturu3rdinterface()
{
    if (!IsInKanturu3rd() || iKanturuResult == -1)
    {
        return;
    }

    RenderKanturu3rdResultInterface();
}

void M39Kanturu3rd::RenderKanturu3rdResultInterface()
{
    if (iKanturuResult == 1)
        Kanturu3rdSuccess();
    else if (iKanturuResult == 0)
        Kanturu3rdFailed();
    else
    {
        iKanturuResult = -1;
        fAlpha = 0.1f;
    }
}

void M39Kanturu3rd::Kanturu3rdSuccess()
{
    float fPosX, fPosY, fWidth, fHeight, tu, tv;

    fWidth = 372.0f;
    fHeight = 99.0f;
    fPosX = ((float)REFERENCE_WIDTH - fWidth) / 2.0f;
    fPosY = ((float)REFERENCE_HEIGHT - fWidth) / 2.0f;
    tu = fWidth / 512.f;
    tv = fHeight / 128.f;

    if (fAlpha <= 1.0f)
        fAlpha += 0.01f;
    else if (fAlpha >= 0.99f && g_Time.GetTimeCheck(3, 5000))
    {
        fAlpha = 0.1f;
        iKanturuResult = -1;
    }

    EnableAlphaTest();
    RenderBitmap(BITMAP_KANTURU_SUCCESS, fPosX, fPosY, fWidth, fHeight, 0.f, 0.f, tu, tv, true, true, fAlpha);
}

void M39Kanturu3rd::Kanturu3rdFailed()
{
    float fPosX, fPosY, fWidth, fHeight, tu, tv;

    fWidth = 372.0f;
    fHeight = 99.0f;
    fPosX = ((float)REFERENCE_WIDTH - fWidth) / 2.0f;
    fPosY = ((float)REFERENCE_HEIGHT - fWidth) / 2.0f;
    tu = fWidth / 512.f;
    tv = fHeight / 128.f;

    if (fAlpha <= 1.0f)
        fAlpha += 0.01f;
    else if (fAlpha >= 0.99f && g_Time.GetTimeCheck(4, 5000))
    {
        fAlpha = 0.1f;
        iKanturuResult = -1;
    }

    EnableAlphaTest();
    RenderBitmap(BITMAP_KANTURU_FAILED, fPosX, fPosY, fWidth, fHeight, 0.f, 0.f, tu, tv, true, true, fAlpha);
}

void M39Kanturu3rd::ChangeBackGroundMusic(int World)
{
    if (World == WD_39KANTURU_3RD)
    {
        if (g_Direction.m_CKanturu.m_iKanturuState == KANTURU_STATE_MAYA_BATTLE || g_Direction.m_CKanturu.m_iKanturuState == KANTURU_STATE_STANDBY)
        {
            StopMp3(MUSIC_KANTURU_TOWER);
            PlayMp3(MUSIC_KANTURU_MAYA_BATTLE);

            if (IsEndMp3()) StopMp3(MUSIC_KANTURU_MAYA_BATTLE);
        }
        else if (g_Direction.m_CKanturu.m_iKanturuState == KANTURU_STATE_NIGHTMARE_BATTLE)
        {
            StopMp3(MUSIC_KANTURU_TOWER);
            StopMp3(MUSIC_KANTURU_MAYA_BATTLE);
            PlayMp3(MUSIC_KANTURU_NIGHTMARE_BATTLE);

            if (IsEndMp3()) StopMp3(MUSIC_KANTURU_NIGHTMARE_BATTLE);
        }
        else
        {
            StopMp3(MUSIC_KANTURU_MAYA_BATTLE);
            StopMp3(MUSIC_KANTURU_NIGHTMARE_BATTLE);
            PlayMp3(MUSIC_KANTURU_TOWER);

            if (IsEndMp3()) StopMp3(MUSIC_KANTURU_TOWER);
        }
    }
    else
    {
        StopMp3(MUSIC_KANTURU_MAYA_BATTLE);
        StopMp3(MUSIC_KANTURU_NIGHTMARE_BATTLE);
        StopMp3(MUSIC_KANTURU_TOWER);
    }
}