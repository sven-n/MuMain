///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ZzzOpenglUtil.h"
#include "ZzzBMD.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzLodTerrain.h"
#include "ZzzTexture.h"
#include "ZzzAI.h"
#include "ZzzInterface.h"
#include "ZzzInventory.h"
#include "ZzzEffect.h"
#include "ZzzScene.h"
#include "ZzzOpenData.h"
#include "DSPlaySound.h"

#include "SideHair.h"
#include "PhysicsManager.h"
#include "GOBoid.h"
#include "CSparts.h"
#include "CSItemOption.h"
#include "CSChaosCastle.h"
#include "MapManager.h"
#include "UIManager.h"
#include "CDirection.h"
#include "CComGem.h"
#include "BoneManager.h"
#include "w_CursedTemple.h"
#include "CharacterManager.h"
#include "w_MapHeaders.h"
#include "MonkSystem.h"
#include "NewUISystem.h"

extern vec3_t VertexTransform[MAX_MESH][MAX_VERTICES];
extern vec3_t LightTransform[MAX_MESH][MAX_VERTICES];

int          g_iTotalObj = 0;
OBJECT_BLOCK ObjectBlock[256];
OBJECT       Boids[MAX_BOIDS];
OBJECT       Fishs[MAX_FISHS];
OBJECT       Mounts[MAX_MOUNTS];
OPERATE      Operates[MAX_OPERATES];
//int   World = -1;
float EarthQuake;

static  int g_iThunderTime = 0;
int g_iActionObjectType = -1;
int g_iActionWorld = -1;
int g_iActionTime = -1;
float g_fActionObjectVelocity = -1;

void ClearActionObject()
{
    g_iActionObjectType = -1;
    g_iActionWorld = -1;
    g_iActionTime = -1;
    g_fActionObjectVelocity = -1;
}

void SetActionObject(int iWorld, int iType, int iLifeTime, int iVel)
{
    g_iActionWorld = iWorld;
    g_iActionObjectType = iType;
    g_iActionTime = iLifeTime;
    g_fActionObjectVelocity = (float)iVel;
}

void ActionObject(OBJECT* o)
{
    if (g_iActionWorld < 0)     return;
    if (g_iActionObjectType < 0)return;
    if (g_iActionTime < 0)      return;

    if (gMapManager.WorldActive == g_iActionWorld)
    {
        if (MoveChaosCastleAllObject(o)) return;
        {
            vec3_t  Position, Light;

            Vector(1.f, 1.f, 1.f, Light);
            if (o->Type == g_iActionObjectType || o->Type == 9 || o->Type == 10)
            {
                if (o->Type == 9)
                {
                    if (g_iActionTime == 0)
                    {
                        o->HiddenMesh = -1;
                        o->PKKey = 4;
                    }
                }
                else if (o->Type == 10)
                {
                    if (g_iActionTime == 0)
                    {
                        o->HiddenMesh = -1;
                        o->PKKey = 4;
                    }
                }
                else if (o->Type == g_iActionObjectType)
                {
                    if (g_iActionTime == 20)
                    {
                        o->Angle[0] = 35.f;
                        o->HiddenMesh = -1;

                        PlayBuffer(SOUND_DOWN_GATE);
                    }

                    if (g_iActionTime >= 0)
                    {
                        o->Angle[0] += g_fActionObjectVelocity;
                        g_fActionObjectVelocity += 1.5f;

                        if (o->Angle[0] >= 90.f)
                        {
                            o->Angle[0] -= g_iActionTime;
                            g_fActionObjectVelocity = 2.f;

                            VectorCopy(o->Position, Position);

                            if (o->Angle[0] == 80)
                            {
                                for (int i = 0; i < 10; ++i)
                                {
                                    Position[0] = o->Position[0] + (rand() % 300 - 150.f);
                                    Position[1] = o->Position[1] - (rand() % 20 + 600.f);
                                    CreateParticleFpsChecked(BITMAP_SMOKE + 1, Position, o->Angle, o->Light);
                                }
                            }
                        }
                        if (g_iActionTime == 0)
                        {
                            o->HiddenMesh = -2;
                            o->Angle[0] = 90.f;
                            ClearActionObject();

                            AddTerrainAttributeRange(13, 70, 3, 6, TW_NOGROUND, false);
                        }
                    }
                }
            }
        }
    }
}

OBJECT* CollisionDetectObjects(OBJECT* PickObject)
{
    OBJECT* Object = NULL;
    InitCollisionDetectLineToFace();
    for (int i = 0; i < 16; i++)
    {
        for (int j = 0; j < 16; j++)
        {
            OBJECT_BLOCK* ob = &ObjectBlock[i * 16 + j];
            OBJECT* o = ob->Head;
            while (1)
            {
                if (o != NULL)
                {
                    if (o->Live && o->Visible && o->Alpha >= 0.01f)
                    {
                        //if(o != PickObject)
                        {
                            BMD* b = &Models[o->Type];
                            b->BodyScale = o->Scale;
                            b->CurrentAction = o->CurrentAction;
                            VectorCopy(o->Position, b->BodyOrigin);
                            b->Animation(BoneTransform, o->AnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle, false, false);
                            b->Transform(BoneTransform, o->BoundingBoxMin, o->BoundingBoxMax, &o->OBB, true);
                            if (CollisionDetectLineToOBB(MousePosition, MouseTarget, o->OBB))
                            {
                                if (b->CollisionDetectLineToMesh(MousePosition, MouseTarget))
                                {
                                    Object = o;
                                    //return Object;
                                }
                            }
                        }
                    }
                    if (o->Next == NULL) break;
                    o = o->Next;
                }
                else break;
            }
        }
    }
    return Object;
}

void BodyLight(OBJECT* o, BMD* b)
{
    if (o->Type == MODEL_DARK_PHEONIX_SHIELD)
    {
        Vector(.6f, .6f, .6f, b->BodyLight);
        return;
    }
    if (o->Type == MODEL_PROTECT)
    {
        float Luminosity = sinf(WorldTime * 0.003f) * 0.5f + 0.5f;
        Vector(Luminosity, Luminosity, Luminosity, b->BodyLight);
        return;
    }

    b->LightEnable = o->LightEnable;
    if (o->LightEnable)
    {
        vec3_t Light;
        RequestTerrainLight(o->Position[0], o->Position[1], Light);
        VectorAdd(Light, o->Light, b->BodyLight);
    }
    else
    {
        vec3_t Light;
        RequestTerrainLight(o->Position[0], o->Position[1], Light);
        VectorScale(Light, 0.1f, Light);
        VectorAdd(Light, o->Light, b->BodyLight);
    }
}

extern float BoneScale;

bool Calc_RenderObject(OBJECT* o, bool Translate, int Select, int ExtraMon)
{
    if (gMapManager.InChaosCastle() == true && Hero->Object.m_bActionStart == true)
    {
        o->Alpha -= 0.15f;
    }

    if (o->Alpha < 0.01f)
    {
        return false;
    }

    BMD* b = &Models[o->Type];
    b->BodyHeight = 0.f;
    b->ContrastEnable = o->ContrastEnable;
    BodyLight(o, b);
    b->BodyScale = o->Scale;
    b->CurrentAction = o->CurrentAction;
    VectorCopy(o->Position, b->BodyOrigin);

    if (o->Type == MODEL_CASTLE_GATE)
    {
        vec3_t Position;
        VectorCopy(o->Position, Position);

        Position[1] += 60.f;
        VectorCopy(Position, b->BodyOrigin);
    }
    else if (o->Type == MODEL_STATUE_OF_SAINT)
    {
        vec3_t Position;
        VectorCopy(o->Position, Position);

        Position[1] += 120.f;
        VectorCopy(Position, b->BodyOrigin);
    }

    if (o->Owner != NULL)
    {
        if (g_isCharacterBuff(o->Owner, eDeBuff_Stun) || g_isCharacterBuff(o->Owner, eDeBuff_Sleep))
        {
            o->AnimationFrame = 0.f;
        }
    }

    if (o->EnableBoneMatrix)
    {
        b->Animation(o->BoneTransform, o->AnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle, false, !Translate);
    }
    else
    {
        b->Animation(BoneTransform, o->AnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle, false, !Translate);
    }

    BoneScale = 1.f;
    if (3 == Select)
    {
        BoneScale = 1.4f;
    }
    else if (2 == Select)
    {
        BoneScale = 1.2f;
    }
    else if (1 == Select)
    {
        b->LightEnable = false;

        if (gMapManager.InChaosCastle() == true || o->Kind != KIND_NPC)
        {
            Vector(0.1f, 0.01f, 0.f, b->BodyLight);
            if (o->Type == MODEL_BALI)
            {
                BoneScale = 1.2f;
            }
            else
            {
                BoneScale = 1.f + (0.1f / o->Scale);
            }
            if (o->m_fEdgeScale != 1.2f)
            {
                BoneScale = o->m_fEdgeScale;
            }
        }
        else
        {
            Vector(0.02f, 0.1f, 0.f, b->BodyLight);
            BoneScale = 1.2f;
            BoneScale = o->m_fEdgeScale;
        }
        float Scale = BoneScale;
        RenderPartObjectEdge(b, o, RENDER_BRIGHT, Translate, Scale);

        if (gMapManager.InChaosCastle() == true || o->Kind != KIND_NPC)
        {
            Vector(0.7f, 0.07f, 0.f, b->BodyLight);
            if (o->Type == MODEL_BALI)
            {
                BoneScale = 1.08f;
            }
            else
            {
                BoneScale = 1.f + (0.04f / o->Scale);
            }
            if (o->m_fEdgeScale != 1.2f)
            {
                BoneScale = maxf(o->m_fEdgeScale - 0.04f, 1.01f);
            }
        }
        else
        {
            Vector(0.16f, 0.7f, 0.f, b->BodyLight);
            BoneScale = 1.08f;
            BoneScale = maxf(o->m_fEdgeScale - 0.12f, 1.01f);
        }

        Scale = BoneScale;
        RenderPartObjectEdge(b, o, RENDER_BRIGHT, Translate, Scale);
        BodyLight(o, b);
        BoneScale = 1.f;
    }

    if (o->EnableBoneMatrix)
    {
        b->Transform(o->BoneTransform, o->BoundingBoxMin, o->BoundingBoxMax, &o->OBB, Translate);
    }
    else
    {
        b->Transform(BoneTransform, o->BoundingBoxMin, o->BoundingBoxMax, &o->OBB, Translate);
    }

    return true;
}

bool Calc_ObjectAnimation(OBJECT* o, bool Translate, int Select)
{
    if (gMapManager.InChaosCastle() == true && Hero->Object.m_bActionStart)
    {
        o->Alpha -= 0.15f;
    }

    if (o->Alpha < 0.01f) return false;

    BMD* b = &Models[o->Type];
    b->BodyHeight = 0.f;
    b->ContrastEnable = o->ContrastEnable;
    BodyLight(o, b);
    b->BodyScale = o->Scale;
    b->CurrentAction = o->CurrentAction;
    VectorCopy(o->Position, b->BodyOrigin);

    if (o->EnableBoneMatrix)
    {
        b->Animation(o->BoneTransform, o->AnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle, false, !Translate);
    }
    else
    {
        b->Animation(BoneTransform, o->AnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle, false, !Translate);
    }
    return true;
}

void Draw_RenderObject(OBJECT* o, bool Translate, int Select, int ExtraMon)
{
    BMD* b = &Models[o->Type];
    bool View = true;

    if ((EditFlag != EDIT_NONE) || (EditFlag == EDIT_NONE && o->HiddenMesh != -2))
    {
        if (ExtraMon == 10)
        {
            float Alpha = 0.5f;

            for (int i = 0; i < MAX_CHARACTERS_CLIENT; ++i)
            {
                CHARACTER* c = &CharactersClient[i];
                OBJECT* oa = &c->Object;
                OBJECT* ob = o->Owner;

                if (oa == ob)
                {
                    if (c != Hero && battleCastle::IsBattleCastleStart() == true && g_isCharacterBuff(ob, eBuff_Cloaking))
                    {
                        if (Hero->EtcPart == PARTS_ATTACK_KING_TEAM_MARK || Hero->EtcPart == PARTS_ATTACK_TEAM_MARK)
                        {
                            if (!(c->EtcPart == PARTS_ATTACK_KING_TEAM_MARK || c->EtcPart == PARTS_ATTACK_TEAM_MARK))
                            {
                                View = false;
                                break;
                            }
                        }
                        else
                            if (Hero->EtcPart == PARTS_ATTACK_KING_TEAM_MARK2 || Hero->EtcPart == PARTS_ATTACK_TEAM_MARK2)
                            {
                                if (!(c->EtcPart == PARTS_ATTACK_KING_TEAM_MARK2 || c->EtcPart == PARTS_ATTACK_TEAM_MARK2))
                                {
                                    View = false;
                                    break;
                                }
                            }
                            else
                                if (Hero->EtcPart == PARTS_ATTACK_KING_TEAM_MARK3 || Hero->EtcPart == PARTS_ATTACK_TEAM_MARK3)
                                {
                                    if (!(c->EtcPart == PARTS_ATTACK_KING_TEAM_MARK3 || c->EtcPart == PARTS_ATTACK_TEAM_MARK3))
                                    {
                                        View = false;
                                        break;
                                    }
                                }
                                else
                                    if (Hero->EtcPart == PARTS_DEFENSE_KING_TEAM_MARK || Hero->EtcPart == PARTS_DEFENSE_TEAM_MARK)
                                    {
                                        if (!(c->EtcPart == PARTS_DEFENSE_KING_TEAM_MARK || c->EtcPart == PARTS_DEFENSE_TEAM_MARK))
                                        {
                                            View = false;
                                            break;
                                        }
                                    }
                    }
                }
            }

            if (View == true)
            {
                Vector(1.f, 1.f, 1.f, b->BodyLight);
                for (int i = 0; i < Models[o->Type].NumMeshs; i++)
                    b->RenderMesh(i, RENDER_BRIGHT | RENDER_CHROME5, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU * 8.f, o->BlendMeshTexCoordV * 2.f, BITMAP_CHROME2);
            }

            return;
        }
        if (gMapManager.WorldActive == WD_10HEAVEN)
        {
            if (o->Type == MODEL_DRAGON_)
            {
                Vector(0.02f, 0.05f, 0.15f, b->BodyLight);
            }
        }
        if (gMapManager.InDevilSquare())
        {
            if (o->Type == MODEL_ICE_QUEEN)
            {
                Vector(0.0f, 0.3f, 1.0f, b->BodyLight);
            }
        }
        if (ExtraMon && o->Type == MODEL_BALROG)
        {
            Vector(0.0f, 0.0f, 1.0f, b->BodyLight);
        }

        if (o->RenderType == RENDER_DARK)
        {
            b->RenderBody(RENDER_TEXTURE | RENDER_DARK, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        }
        else if (o->Type == MODEL_CHANGE_UP_EFF)
        {
            if (o->SubType == 0 || o->SubType == 1)
            {
                Vector(0.1f, 0.4f, 0.6f, b->BodyLight);
            }
            else
            {
                Vector(0.1f, 0.2f, 0.9f, b->BodyLight);
            }
            b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        }
        else if (o->Type == MODEL_CHANGE_UP_NASA)
        {
            Vector(0.5f, 0.5f, 0.9f, b->BodyLight);
            b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        }
        else if (o->Type == MODEL_CHANGE_UP_CYLINDER)
        {
            Vector(0.4f, 0.5f, 1.f, b->BodyLight);
            b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        }
        else if (o->Type == MODEL_SUMMON)
        {
            //			Vector(0.4f,0.5f,1.f,b->BodyLight);
            //			b->RenderBody(RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV);
            if (!M39Kanturu3rd::IsInKanturu3rd())
            {
                VectorCopy(o->Light, b->BodyLight)
                    b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                Vector(1.f, 1.f, 1.f, b->BodyLight);
            }
        }
        else if (o->Type == MODEL_DEASULER)
        {
            b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
            Vector(0.3f, 0.4f, 1.0f, b->BodyLight);
            b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        }
        else if (o->Type == MODEL_FRED)
        {
            MoveEye(o, b, 14, 15, 71, 72, 73, 74);

            if (!o->m_pCloth)
            {
                auto* pCloth = new CPhysicsCloth[1];
                pCloth[0].Create(o, 10, 0.0f, -10.0f, 0.0f, 5, 12, 150.0f, 190.0f, BITMAP_DEASULER_CLOTH, BITMAP_DEASULER_CLOTH, PCT_MASK_ALPHA | PCT_HEAVY | PCT_STICKED | PCT_SHORT_SHOULDER);
                pCloth[0].AddCollisionSphere(50.0f, -140.0f, -20.0f, 30.0f, 2);
                o->m_pCloth = (void*)pCloth;
                o->m_byNumCloth = 1;
            }
            auto* pCloth = (CPhysicsCloth*)o->m_pCloth;
            if (!pCloth[0].Move2(0.005f, 5))
            {
                CHARACTER* c = &CharactersClient[(int)o->PKKey];
                DeleteCloth(c, o);
            }
            else
            {
                pCloth[0].Render();
            }

            Vector(1.f, 1.f, 1.f, b->BodyLight);
            b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
            b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
            b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);

            if (o->CurrentAction != MONSTER01_ATTACK2 || o->AnimationFrame < 2.5f)
            {
                b->RenderMesh(3, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                Vector(0.4f, 0.5f, 1.0f, b->BodyLight);
                b->RenderMesh(4, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
            }
        }
        else
            if (o->Type == MODEL_BANSHEE)
            {
                float fLumi = (sinf(WorldTime * 0.0015f) + 1.f) * 0.35f;

                Vector(1.f, 1.f, 1.f, b->BodyLight);
                b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);

                Vector(fLumi, fLumi, fLumi, b->BodyLight);
                b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_ASSASSIN_EFFECT1);
            }
            else if (o->Type == MODEL_RAKLION_BOSS_CRACKEFFECT)
            {
                float fLumi = o->Alpha;
                Vector(o->Light[0] * fLumi, o->Light[1] * fLumi, o->Light[2] * fLumi, b->BodyLight);
                b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
            }
            else if (o->Type == MODEL_RAKLION_BOSS_MAGIC)
            {
                float fLumi = o->Alpha;
                Vector(o->Light[0] * fLumi, o->Light[1] * fLumi, o->Light[2] * fLumi, b->BodyLight);
                b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
            }
            else if (o->Type == MODEL_NIGHTWATER_01)
            {
                float fLumi = o->Alpha;
                Vector(o->Light[0] * fLumi, o->Light[1] * fLumi, o->Light[2] * fLumi, b->BodyLight);
                b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
            }
            else if (o->Type == MODEL_KNIGHT_PLANCRACK_A)
            {
                float fLumi = o->Alpha;
                Vector(o->Light[0] * fLumi, o->Light[1] * fLumi, o->Light[2] * fLumi, b->BodyLight);
                b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 1, o->BlendMeshLight, o->BlendMeshTexCoordU, -(int)WorldTime % 2000 * 0.0001f);
            }
            else if (o->Type == MODEL_KNIGHT_PLANCRACK_B)
            {
                float fLumi = o->Alpha;
                Vector(o->Light[0] * fLumi, o->Light[1] * fLumi, o->Light[2] * fLumi, b->BodyLight);
                b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 1, o->BlendMeshLight, o->BlendMeshTexCoordU, -(int)WorldTime % 2000 * 0.0001f);
            }
            else if (o->Type == MODEL_ALICE_BUFFSKILL_EFFECT || o->Type == MODEL_ALICE_BUFFSKILL_EFFECT2)
            {
                if (o->SubType == 1)
                {
                    b->RenderBody(RENDER_TEXTURE | RENDER_DARK, o->Alpha, 0, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                }
                else
                {
                    b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                }
            }
            else if (o->Type == MODEL_CURSEDTEMPLE_HOLYITEM)
            {
                vec3_t Light;
                float fLuminosity = (float)sinf((WorldTime) * 0.0002f) * 0.002f;
                Vector(0.9f, 0.4f, 0.4f, Light);
                CreateSprite(BITMAP_LIGHT, o->Position, 3.f, Light, o, 0.0f, 0);	// flare01.jpg
                Vector(0.9f, 0.6f, 0.6f, Light);
                CreateSprite(BITMAP_POUNDING_BALL, o->Position, 0.9f + fLuminosity, Light, NULL, (WorldTime / 10.0f));
                CreateParticleFpsChecked(BITMAP_POUNDING_BALL, o->Position, o->Angle, Light, 2, 0.1f, o);

                b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, 0);
                b->RenderMesh(0, RENDER_TEXTURE, 0.7f, 0, o->BlendMeshLight * 3.f, o->BlendMeshTexCoordU, -WorldTime * 0.004f);
            }
            else if (o->Type == MODEL_CURSEDTEMPLE_PRODECTION_SKILL)
            {
                Vector(0.3f, 0.3f, 1.0f, b->BodyLight);
                VectorCopy(o->Angle, b->BodyAngle);
                b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, 0, o->BlendMeshLight, o->BlendMeshTexCoordU, -WorldTime * 0.0004f);
            }
            else if (o->Type == MODEL_CURSEDTEMPLE_RESTRAINT_SKILL)
            {
                float fLuminosity = (float)sinf((WorldTime) * 0.0002f) * 0.002f;
                b->RenderMesh(0, RENDER_TEXTURE, 0.7f, 0, 0.35f + fLuminosity, -WorldTime * 0.0004f, WorldTime * 0.0004f);
            }
            else if (gMapManager.IsCursedTemple() == true
                && (o->Type == 32 || o->Type == 39 || o->Type == 41 || o->Type == 46 || o->Type == 62
                    || o->Type == 67 || o->Type == 68 || o->Type == 64 || o->Type == 65 || o->Type == 66 || o->Type == 80)
                )
            {
                if (o->Type == 32)
                {
                    b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight);
                    b->RenderMesh(1, RENDER_TEXTURE | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight);
                    b->StreamMesh = 2;
                    b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, -(int)WorldTime % 4000 * 0.00025f, o->BlendMeshTexCoordV);
                    b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, -(int)WorldTime % 5000 * 0.0002f, o->BlendMeshTexCoordV);
                    b->StreamMesh = -1;
                }
                else if (o->Type == 39 || o->Type == 41)
                {
                    b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight);
                    b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight);
                }
                else if (o->Type == 46)
                {
                    b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight);
                    b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight);
                    b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight);
                }
                else if (o->Type == 62)
                {
                    b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight);
                    b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight);
                    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight);
                    b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight);

                    vec3_t vPos, vRelativePos, vLight;
                    Vector(0.f, 0.f, 0.f, vPos);
                    Vector(6.f, 5.f, 2.f, vRelativePos);
                    Vector(1.f, 1.f, 1.f, vLight);
                    float fLumi = sinf(WorldTime * 0.001f) * 0.5f + 0.5f;
                    b->TransformPosition(BoneTransform[22], vRelativePos, vPos);
                    Vector(1.f, 0.5f, 0.5f, vLight);
                    CreateSprite(BITMAP_SHINY + 1, vPos, 1.5f + fLumi / 2.f, vLight, NULL);
                    Vector(6.f, -5.f, 2.f, vRelativePos);
                    b->TransformPosition(BoneTransform[23], vRelativePos, vPos);
                    Vector(1.f, 0.5f, 0.5f, vLight);
                    CreateSprite(BITMAP_SHINY + 1, vPos, 1.5f + fLumi / 2.f, vLight, NULL);

                    if (rand_fps_check(5))
                    {
                        Vector(0.f, 0.f, 0.f, vRelativePos);
                        b->TransformPosition(BoneTransform[20], vRelativePos, vPos);
                        Vector(1.f, 1.f, 1.f, vLight);
                        CreateParticleFpsChecked(BITMAP_SMOKE, vPos, o->Angle, vLight);
                    }
                }
                else if (o->Type == 67)
                {
                    b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight);
                    b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight);
                }
                else if (o->Type == 68)
                {
                    b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight);
                    b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight);
                }
                else if (o->Type == 64 || o->Type == 65 || o->Type == 66 || o->Type == 80)
                {
                    o->m_bRenderAfterCharacter = true;
                }
            }
            else if (o->Type == MODEL_EFFECT_SKURA_ITEM)
            {
                b->RenderBody(RENDER_COLOR, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
            }

            else if (o->Type == MODEL_DEVIL)
            {
                Vector(0.4f, 0.6f, 1.f, b->BodyLight);
                b->StreamMesh = 0;
                b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh, BITMAP_CHROME);
                Vector(1.f, 1.f, 1.f, b->BodyLight);
                b->StreamMesh = -1;
                b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
            }
            else if (o->Type == MODEL_PEGASUS)
            {
                b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
            }
            else if (o->Type == MODEL_SKIN_SHELL)
            {
                if (o->SubType == 0)
                {
                    VectorCopy(o->Light, b->BodyLight);
                    b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh, BITMAP_CHROME);
                }
                else if (o->SubType == 1)
                {
                    Vector(0.1f, 0.5f, 1.f, b->BodyLight);
                    b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh, BITMAP_CHROME);
                }
            }
            else if (o->Type == MODEL_STUN_STONE)
            {
                b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);

                Vector(1.f, 1.f, 1.f, b->BodyLight);
                b->RenderBody(RENDER_CHROME, 0.5f, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU * 5.f, o->BlendMeshTexCoordV * 2.f, -1, BITMAP_CHROME);
            }
            else if (o->Type == MODEL_DARK_HORSE && o->SubType == 1)
            {
                if (gMapManager.WorldActive == WD_65DOPPLEGANGER1)
                {
                    o->Alpha = 0.7f;
                    b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                }
                else
                {
                    Vector(0.2f, 0.2f, 0.2f, b->BodyLight);
                    b->RenderBody(RENDER_BRIGHT | RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                }
            }
            else if (o->Type == MODEL_DARK_HORSE)
            {
                Vector(1.f, 1.f, 1.f, b->BodyLight);
                b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);

                Vector(1.f, 0.8f, 0.3f, b->BodyLight);
                b->RenderMesh(12, RENDER_CHROME | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                b->RenderMesh(13, RENDER_CHROME | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                b->RenderMesh(14, RENDER_CHROME | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                b->RenderMesh(15, RENDER_CHROME | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);

                if (gMapManager.WorldActive != WD_10HEAVEN && gMapManager.InHellas() == false)
                {
                    if (!g_Direction.m_CKanturu.IsMayaScene())
                    {
                        EnableAlphaTest();

                        if (gMapManager.WorldActive == WD_7ATLANSE)
                        {
                            glColor4f(0.f, 0.f, 0.f, 0.2f);
                        }
                        else
                        {
                            glColor4f(0.f, 0.f, 0.f, 0.7f);
                        }
                        b->RenderBodyShadow(-1, -1, 8, 9);
                    }
                }
            }
            else if (o->Type == MODEL_FENRIR_BLACK || o->Type == MODEL_FENRIR_BLUE || o->Type == MODEL_FENRIR_RED || o->Type == MODEL_FENRIR_GOLD)
            {
                vec3_t vLight, vPos, vPosition;
                float fLuminosity = (float)sinf((WorldTime) * 0.002f) * 0.2f;

                b->BeginRender(1.f);

                b->BodyLight[0] = 1.0f;
                b->BodyLight[1] = 1.0f;
                b->BodyLight[2] = 1.0f;

                if (o->Type == MODEL_FENRIR_GOLD)
                {
                    b->StreamMesh = 0;

                    b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                    b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                    b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);

                    if (o->CurrentAction == FENRIR_ATTACK_SKILL)
                    {
                        Vector(1.0f, 0.0f, 0.0f, vLight);
                        Vector((float)(rand() % 10 - 10) * 0.5f, 0.f, (float)(rand() % 40 - 20) * 0.5f, vPos);
                        b->TransformPosition(BoneTransform[14], vPos, vPosition, false);
                        CreateParticleFpsChecked(BITMAP_SPARK + 1, vPosition, o->Angle, vLight, 15, 0.7f + (fLuminosity * 0.05f));
                    }

                    b->StreamMesh = -1;
                }
                else
                {
                    b->StreamMesh = 0;

                    b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                    b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);

                    // custom glow? :
                    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);

                    if (o->CurrentAction == FENRIR_ATTACK_SKILL)
                    {
                        b->BodyLight[0] = 1.0f;
                        b->BodyLight[1] = 1.0f;
                        b->BodyLight[2] = 1.0f;

                        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);

                        Vector(1.0f, 0.0f, 0.0f, vLight);
                        Vector((float)(rand() % 10 - 10) * 0.5f, 0.f, (float)(rand() % 40 - 20) * 0.5f, vPos);
                        b->TransformPosition(BoneTransform[14], vPos, vPosition, false);	// 턱
                        CreateParticleFpsChecked(BITMAP_SPARK + 1, vPosition, o->Angle, vLight, 15, 0.7f + (fLuminosity * 0.05f));
                    }
                    b->StreamMesh = -1;
                }

                b->EndRender();

                if (gMapManager.WorldActive != WD_10HEAVEN && gMapManager.InHellas() == FALSE)
                {
                    if (!g_Direction.m_CKanturu.IsMayaScene())
                    {
                        EnableAlphaTest();

                        if (gMapManager.WorldActive == WD_7ATLANSE)
                        {
                            glColor4f(0.f, 0.f, 0.f, 0.2f);
                        }
                        else
                        {
                            glColor4f(0.f, 0.f, 0.f, 0.7f);
                        }

                        b->RenderBodyShadow();
                    }
                }

                Vector(0.9f + fLuminosity, 0.2f + (fLuminosity * 0.5f), 0.1f + (fLuminosity * 0.5f), vLight);
                Vector(50.f, 2.f, 11.f, vPos);
                b->TransformPosition(BoneTransform[11], vPos, vPosition, false);
                CreateSprite(BITMAP_LIGHT, vPosition, 0.5f + (fLuminosity * 0.1f), vLight, o);
                CreateSprite(BITMAP_LIGHT, vPosition, 0.5f + (fLuminosity * 0.1f), vLight, o);
                Vector(50.f, 2.f, -11.f, vPos);
                b->TransformPosition(BoneTransform[11], vPos, vPosition, false);
                CreateSprite(BITMAP_LIGHT, vPosition, 0.5f + (fLuminosity * 0.1f), vLight, o);
                CreateSprite(BITMAP_LIGHT, vPosition, 0.5f + (fLuminosity * 0.1f), vLight, o);

                Vector(1.0f, 0.3f, 0.2f, vLight);
                Vector(40.f, 15.f, 0.f, vPos);
                b->TransformPosition(BoneTransform[13], vPos, vPosition, false);
                CreateSprite(BITMAP_LIGHT, vPosition, 1.5f, vLight, o);
                CreateSprite(BITMAP_LIGHT, vPosition, 1.0f, vLight, o);

                int iSubType = 0;
                if (o->Type == MODEL_FENRIR_RED)
                {
                    Vector(0.8f, 0.0f, 0.0f, vLight);
                    iSubType = 1;
                }
                else if (o->Type == MODEL_FENRIR_BLUE)
                {
                    Vector(0.1f, 0.1f, 0.8f, vLight);
                    iSubType = 2;
                }
                else if (o->Type == MODEL_FENRIR_GOLD)
                {
                    Vector(0.8f, 0.8f, 0.1f, vLight);
                    iSubType = 4;
                }
                else
                {
                    Vector(1.0f, 1.0f, 0.2f, vLight);
                    iSubType = 3;
                }
                CreateEffect(MODEL_FENRIR_THUNDER, o->Position, o->Angle, vLight, 0, o);
                CreateEffect(MODEL_FENRIR_THUNDER, o->Position, o->Angle, vLight, 0, o);

                Vector(1.0f, 1.0f, 1.0f, vLight);
                if (o->CurrentAction == FENRIR_WALK)
                {
                    if (o->AnimationFrame >= 0.0f && o->AnimationFrame <= 1.5f)
                    {
                        Vector(0.0f, 0.0f, 0.0f, vPos);
                        b->TransformPosition(BoneTransform[22], vPos, vPosition, false);
                        CreateEffect(MODEL_FENRIR_FOOT_THUNDER, vPosition, o->Angle, vLight, iSubType, o);
                        Vector(0.0f, 0.0f, 0.0f, vPos);
                        b->TransformPosition(BoneTransform[28], vPos, vPosition, false);
                        CreateEffect(MODEL_FENRIR_FOOT_THUNDER, vPosition, o->Angle, vLight, iSubType, o);
                        Vector(0.0f, 0.0f, 0.0f, vPos);
                        b->TransformPosition(BoneTransform[36], vPos, vPosition, false);
                        CreateEffect(MODEL_FENRIR_FOOT_THUNDER, vPosition, o->Angle, vLight, iSubType, o);
                        Vector(0.0f, 0.0f, 0.0f, vPos);
                        b->TransformPosition(BoneTransform[44], vPos, vPosition, false);
                        CreateEffect(MODEL_FENRIR_FOOT_THUNDER, vPosition, o->Angle, vLight, iSubType, o);
                    }
                }
                else if (o->CurrentAction == FENRIR_RUN)
                {
                    if (o->AnimationFrame > 1.f && o->AnimationFrame <= 1.4f)
                    {
                        Vector(0.0f, 0.0f, 0.0f, vPos);
                        b->TransformPosition(BoneTransform[22], vPos, vPosition, false);
                        CreateEffect(MODEL_FENRIR_FOOT_THUNDER, vPosition, o->Angle, vLight, iSubType, o);
                        Vector(0.0f, 0.0f, 0.0f, vPos);
                        b->TransformPosition(BoneTransform[28], vPos, vPosition, false);
                        CreateEffect(MODEL_FENRIR_FOOT_THUNDER, vPosition, o->Angle, vLight, iSubType, o);
                    }
                    else if (o->AnimationFrame > 4.8f && o->AnimationFrame <= 5.2f)
                    {
                        Vector(0.0f, 0.0f, 0.0f, vPos);
                        b->TransformPosition(BoneTransform[36], vPos, vPosition, false);
                        CreateEffect(MODEL_FENRIR_FOOT_THUNDER, vPosition, o->Angle, vLight, iSubType, o);
                        Vector(0.0f, 0.0f, 0.0f, vPos);
                        b->TransformPosition(BoneTransform[44], vPos, vPosition, false);
                        CreateEffect(MODEL_FENRIR_FOOT_THUNDER, vPosition, o->Angle, vLight, iSubType, o);
                    }
                }
            }
            else if (o->Type >= MODEL_FACE && o->Type <= MODEL_FACE + 6)
            {
                Vector(4.8f, 4.8f, 4.8f, b->BodyLight);
                b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                Vector(1.f, 1.f, 1.f, b->BodyLight);
            }
            else if (o->Type == MODEL_DARKLORD_SKILL)
            {
                VectorCopy(o->Light, b->BodyLight);
                b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                Vector(1.f, 1.f, 1.f, b->BodyLight);
            }
            else if (o->Type == MODEL_DARK_SPIRIT)
            {
                glColor3f(1.f, 1.f, 1.f);
                b->BeginRender(o->Alpha);
                b->RenderMesh(3, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                if (o->WeaponLevel >= 40)
                {
                    b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                    Vector(0.3f, 0.6f, 1.f, b->BodyLight);
                    b->RenderMesh(0, RENDER_CHROME | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                    Vector(1.f, 1.f, 1.f, b->BodyLight);
                    b->RenderMesh(3, RENDER_BRIGHT | RENDER_TEXTURE, o->Alpha, 3, sinf(WorldTime * 0.001f), o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_MONSTER_SKIN + 2);
                }
                else if (o->WeaponLevel >= 20)
                {
                    b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                }
                else
                {
                    b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                }
                b->EndRender();

                if (gMapManager.WorldActive != WD_10HEAVEN && gMapManager.InHellas() == false)
                {
                    if (!g_Direction.m_CKanturu.IsMayaScene())
                    {
                        vec3_t Position;
                        EnableAlphaTest();

                        if (gMapManager.WorldActive == WD_7ATLANSE)
                        {
                            glColor4f(0.f, 0.f, 0.f, 0.2f);
                        }
                        else
                        {
                            glColor4f(0.f, 0.f, 0.f, 1.f);
                        }
                        VectorCopy(o->Position, Position);
                        Position[2] = RequestTerrainHeight(o->Position[0], o->Position[1]);
                        VectorCopy(Position, b->BodyOrigin);
                        b->RenderBodyShadow();
                    }
                }
            }
            else if (gMapManager.WorldActive == WD_0LORENCIA && o->Type == MODEL_WATERSPOUT)
            {
                b->BeginRender(o->Alpha);
                b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight);
                b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight);
                b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight);
                b->RenderMesh(3, RENDER_TEXTURE, o->Alpha, 3, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                b->EndRender();
            }
            else if (gMapManager.WorldActive == WD_4LOSTTOWER && (o->Type == 23 || o->Type == 19 || o->Type == 20 || o->Type == 3 || o->Type == 4))
            {
                vec3_t Light, p;
                float Luminosity;
                Luminosity = (float)(rand() % 2 + 6) * 0.1f;
                Vector(Luminosity * 0.4f, Luminosity * 0.8f, Luminosity * 1.f, Light);
                Vector(0.f, 0.f, 0.f, p);
                if (o->Type == 23)
                {
                    b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                }
                else if (o->Type == 19 || o->Type == 20)
                {
                    VectorCopy(b->BodyLight, Light);
                    Vector(1.f, 0.2f, 0.1f, b->BodyLight);
                    b->StreamMesh = 2;
                    b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh, BITMAP_CHROME);
                    VectorCopy(Light, b->BodyLight);
                    b->StreamMesh = -1;
                    b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                }
                else if (o->Type == 3 || o->Type == 4)
                {
                    VectorCopy(b->BodyLight, Light);
                    Vector(1.f, 0.2f, 0.1f, b->BodyLight);
                    b->StreamMesh = 1;
                    b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh, BITMAP_CHROME);
                    VectorCopy(Light, b->BodyLight);
                    b->StreamMesh = -1;
                    b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                }
            }
            else if (gMapManager.WorldActive == WD_8TARKAN && (o->Type == 81))
            {
                b->BeginRender(o->Alpha);
                b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                b->RenderMesh(0, RENDER_CHROME | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_CHROME);
                b->EndRender();
            }
            else if (gMapManager.InBloodCastle() == true && (o->Type == 28 || o->Type == 29))
            {
                b->BeginRender(o->Alpha);
                b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                b->EndRender();

                EnableAlphaTest();

                vec3_t Position;

                glColor4f(0.f, 0.f, 0.f, 1.f);
                VectorCopy(o->Position, Position);
                Position[2] = RequestTerrainHeight(o->Position[0], o->Position[1]);
                VectorCopy(Position, b->BodyOrigin);
                o->HiddenMesh = 2;
                b->RenderBodyShadow(o->BlendMesh, o->HiddenMesh);
                o->HiddenMesh = -1;
            }
            else if (gMapManager.WorldActive == WD_3NORIA && o->Type == MODEL_WARP3)
            {
                b->BodyLight[0] = 0.8f;
                b->BodyLight[1] = 0.8f;
                b->BodyLight[2] = 0.8f;

                b->StreamMesh = 0;
                b->RenderMesh(0, RENDER_DARK, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                b->StreamMesh = -1;
            }
            else if (gMapManager.WorldActive == WD_55LOGINSCENE && o->Type == 90)
            {
                b->BodyLight[0] = 1.0f;
                b->BodyLight[1] = 1.0f;
                b->BodyLight[2] = 1.0f;

                o->Alpha = 1.0f;
                o->BlendMeshLight = 1.0f;
                b->StreamMesh = 0;
                b->RenderMesh(0, RENDER_DARK, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                b->StreamMesh = -1;
            }
            else
            {
                BOOL bIsRendered = TRUE;

                if (g_isCharacterBuff(o, eDeBuff_Poison) && g_isCharacterBuff(o, eDeBuff_Freeze))
                {
                    Vector(0.3f, 1.f, 0.8f, b->BodyLight);
                }
                else if (g_isCharacterBuff(o, eDeBuff_Poison))
                {
                    Vector(0.3f, 1.f, 0.5f, b->BodyLight);
                }
                else if (g_isCharacterBuff(o, eDeBuff_Freeze))
                {
                    Vector(0.3f, 0.5f, 1.f, b->BodyLight);
                }
                else if (g_isCharacterBuff(o, eDeBuff_BlowOfDestruction))
                {
                    Vector(0.3f, 0.5f, 1.f, b->BodyLight);
                }

                if (o->Type == MODEL_VALKYRIE || o->Type == MODEL_ICE_MONSTER || o->Type == MODEL_ALQUAMOS || o->Type == MODEL_QUEEN_RAINER)
                {
                    if (o->Alpha == 1.0f && o->BlendMeshLight == 0.05f)
                    {
                        o->BlendMeshLight = 1.0f;
                    }
                }

                if (RenderHellasObjectMesh(o, b)) {}
                else
                    if (battleCastle::RenderBattleCastleObjectMesh(o, b)) {}
                    else
                        if (M31HuntingGround::RenderHuntingGroundObjectMesh(o, b, ExtraMon)) {}
                        else
                            if (M34CryingWolf2nd::RenderCryingWolf2ndObjectMesh(o, b)) {}
                            else
                                if (M33Aida::RenderAidaObjectMesh(o, b, ExtraMon)) {}
                                else if (M34CryWolf1st::RenderCryWolf1stObjectMesh(o, b, ExtraMon)) {}
                                else if (M37Kanturu1st::RenderKanturu1stObjectMesh(o, b)) {}
                                else if (M38Kanturu2nd::Render_Kanturu2nd_ObjectMesh(o, b)) {}
                                else if (M39Kanturu3rd::RenderKanturu3rdObjectMesh(o, b, ExtraMon)) {}
                                else if (SEASON3A::CGM3rdChangeUp::Instance().RenderObjectMesh(o, b)) {}
                                else if (g_CursedTemple->RenderObjectMesh(o, b, ExtraMon)) {}
                                else if (SEASON3B::GMNewTown::RenderObject(o, b, ExtraMon)) {}
                                else if (SEASON3C::GMSwampOfQuiet::RenderObject(o, b, ExtraMon)) {}
                                else if (TheMapProcess().RenderObjectMesh(o, b)) {}
                                else
                                    if (o->Type == MODEL_KALIMA_SHOP)
                                    {
                                        Vector(1.f, 1.f, 1.f, b->BodyLight);
                                        float Luminosity = sinf(WorldTime * 0.002f) * 0.3f + 0.6f;
                                        b->BeginRender(o->Alpha);
                                        b->RenderMesh(4, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                                        b->RenderMesh(3, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                                        b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                                        b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                                        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                                        vec3_t Position, Light, p, Angle;
                                        Vector(0.4f, 0.2f, 0.4f, Light);
                                        Vector(30.f, 40.f, 0.f, p);
                                        b->TransformPosition(o->BoneTransform[3], p, Position);

                                        VectorAdd(Position, o->Position, Position);
                                        CreateSprite(BITMAP_LIGHT, Position, Luminosity + 3.5f, Light, o, 50.f);

                                        Vector(0.f, 0.f, 0.f, p);
                                        b->TransformPosition(o->BoneTransform[21], p, Position);
                                        VectorAdd(Position, o->Position, Position);
                                        CreateSprite(BITMAP_LIGHT, Position, Luminosity + 3.5f, Light, o, 50.f);

                                        Vector(0.f, 0.f, 0.f, Angle);
                                        Vector(1.f, 1.f, 1.f, Light);
                                        VectorCopy(o->Position, Position);
                                        Position[0] += 40.f;
                                        Position[1] += 30.f;
                                        Position[2] -= 90.f;
                                        CreateParticleFpsChecked(BITMAP_SMOKE, Position, Angle, Light, 11, 0.4f);

                                        VectorCopy(o->Position, Position);
                                        Position[0] -= 40.f;
                                        Position[1] -= 30.f;
                                        Position[2] -= 90.f;
                                        CreateParticleFpsChecked(BITMAP_SMOKE, Position, Angle, Light, 11, 0.4f);
                                        b->EndRender();
                                    }
                                    else
                                        if (o->Type == MODEL_NPC_QUARREL)
                                        {
                                            Vector(0.5f, 0.5f, 0.8f, b->BodyLight);
                                            b->BeginRender(o->Alpha);
                                            for (int i = 0; i < Models[o->Type].NumMeshs; i++)
                                            {
                                                b->RenderMesh(i, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                                                b->RenderMesh(i, RENDER_CHROME | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_CHROME);
                                            }
                                            b->EndRender();
                                        }
                                        else
                                            if (o->Type == MODEL_SEED_MASTER)
                                            {
                                                b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                float fLumi = (sinf(WorldTime * 0.001f) + 1.0f) * 0.2f + 0.0f;
                                                b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, fLumi, 1, fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                                            }
                                            else
                                                if (o->Type == MODEL_MUTANT)
                                                {
                                                    if (ExtraMon)
                                                    {
                                                        Vector(1.f, 1.f, 1.f, b->BodyLight);
                                                        b->BeginRender(o->Alpha);
                                                        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                                                        b->RenderMesh(0, RENDER_CHROME | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_CHROME);
                                                        b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                                                        b->RenderMesh(1, RENDER_CHROME | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_CHROME);
                                                        b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                                                        b->RenderMesh(2, RENDER_CHROME | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_CHROME);
                                                        b->EndRender();
                                                    }
                                                    else
                                                    {
                                                        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                    }
                                                }
                                                else
                                                    if (o->Type == MODEL_TANTALLOS && o->SubType == 1)
                                                    {
                                                        b->BeginRender(o->Alpha);
                                                        float Light = sinf(WorldTime * 0.002f) * 0.01f + 1.f;
                                                        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, Light, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_MONSTER_SKIN + 1);
                                                        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 1, Light, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_MONSTER_SKIN);
                                                        b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 1, Light, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                                                        b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 1, Light, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                                                        b->EndRender();
                                                    }
                                                    else if (o->Type == MODEL_MAGIC_SKELETON)
                                                    {
                                                        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                        b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, 2, 1.0f, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                                                    }
                                                    else if (o->Type == MODEL_MOLT)
                                                    {
                                                        b->BeginRender(o->Alpha);
                                                        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                                                        b->EndRender();
                                                        auto* pSideHair = new CSideHair;
                                                        pSideHair->Create(VertexTransform, b, o);
                                                        pSideHair->Render(VertexTransform, LightTransform);
                                                        pSideHair->Destroy();
                                                    }
                                                    else if (o->Type == MODEL_DRAKAN)
                                                    {
                                                        if (ExtraMon)
                                                        {
                                                            Vector(.1f, 0.1f, 0.1f, b->BodyLight);
                                                            b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, 1);
                                                            Vector(1.0f, 0.1f, 0.1f, b->BodyLight);
                                                            b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                                                        }
                                                        else
                                                        {
                                                            b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                        }
                                                    }
                                                    else if (o->Type == MODEL_ORC_ARCHER)
                                                    {
                                                        if (ExtraMon)
                                                        {
                                                            Vector(1.0f, 1.0f, 1.0f, b->BodyLight);
                                                            b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_DEST_ORC_WAR0);
                                                            //					b->RenderMesh(1,RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,BITMAP_DEST_ORC_WAR1);
                                                            b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_DEST_ORC_WAR2);
                                                        }
                                                        else
                                                        {
                                                            b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                        }
                                                    }
                                                    else if (o->Type == MODEL_ORC)
                                                    {
                                                        if (ExtraMon)
                                                        {
                                                            Vector(1.0f, 1.0f, 1.0f, b->BodyLight);
                                                            b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_DEST_ORC_WAR1);
                                                            b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_DEST_ORC_WAR0);
                                                            //b->RenderMesh(2,RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,BITMAP_DEST_ORC_WAR2);
                                                        }
                                                        else
                                                        {
                                                            b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                        }
                                                    }
                                                    else if (o->Type == MODEL_CURSED_KING)
                                                    {
                                                        if (ExtraMon)
                                                        {
                                                            Vector(1.0f, 1.0f, 1.0f, b->BodyLight);
                                                            b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, 1, BITMAP_WHITE_WIZARD);
                                                        }
                                                        else
                                                        {
                                                            b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                        }
                                                    }
                                                    else if (o->Type == MODEL_CRUST)
                                                    {
                                                        if (ExtraMon == 0)
                                                        {
                                                            float Luminosity = sinf(WorldTime * 0.002f) * 0.3f + 0.5f;
                                                            Vector(Luminosity + 0.5f, 0.3f - Luminosity * 0.5f, -Luminosity * 0.5f + 0.5f, b->BodyLight);
                                                            //Vector(1.f,1.f,1.f,b->BodyLight);
                                                            //if ( c->Dead == 0)
                                                            {
                                                                b->StreamMesh = 0;
                                                                Vector(.4f, .3f, .5f, b->BodyLight);
                                                                b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh, BITMAP_JANUSEXT);
                                                                Vector(.5f, .5f, .5f, b->BodyLight);
                                                                b->StreamMesh = -1;
                                                            }
                                                            b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                        }
                                                        else
                                                            if (ExtraMon == 301)
                                                            {
                                                                //					b->BodyScale     = o->Scale + (o->Scale/1.0f);
                                                                //					Vector(0.7f,0.5f,0.8f,b->BodyLight);
                                                                Vector(1.f, 1.f, 1.f, b->BodyLight);
                                                                b->BeginRender(o->Alpha);
                                                                b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                                                                b->RenderMesh(0, RENDER_CHROME | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_CHROME);
                                                                b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                                                                b->RenderMesh(1, RENDER_CHROME | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_CHROME);
                                                                b->EndRender();
                                                            }
                                                            else
                                                            {
                                                                Vector(0.1f, 1.0f, .8f, b->BodyLight);
                                                                b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                                //Vector( 0.1f, sinf(WorldTime*0.002f)*0.5f+0.5f, sinf(WorldTime*0.00173f)*0.5f+0.5f,b->BodyLight);
                                                            }
                                                    }
                                                    else if (o->Type == MODEL_PHANTOM_KNIGHT)
                                                    {
                                                        float Luminosity = sinf(WorldTime * 0.002f) * 0.3f + 0.5f;
                                                        Vector(Luminosity + 0.5f, 0.3f - Luminosity * 0.5f, -Luminosity * 0.5f + 0.5f, b->BodyLight);
                                                        //Vector(1.f,1.f,1.f,b->BodyLight);
                                                        Vector(.9f, .8f, 1.0f, b->BodyLight);
                                                        b->RenderBody(RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh, BITMAP_CHROME + 1);
                                                        Vector(1.f, 1.f, 1.f, b->BodyLight);
                                                        //Vector(.7f,.2f,.2f,b->BodyLight);
                                                        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                    }
                                                    else if (o->Type == MODEL_QUEEN_RAINER)
                                                    {
                                                        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, 1, -1);
                                                        //b->RenderMesh(1,RENDER_TEXTURE|RENDER_PONG|RENDER_WAVE,1.0f,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
                                                        b->RenderMesh(1, RENDER_TEXTURE | RENDER_WAVE, 0.5f, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                        b->RenderMesh(1, RENDER_TEXTURE | RENDER_WAVE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                    }
                                                    else if (o->Type == MODEL_LUNAR_RABBIT)
                                                    {
                                                        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                    }
                                                    else if (o->Type == MODEL_MOONHARVEST_MOON)
                                                    {
                                                        b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                    }
                                                    else if (o->Type == MODEL_MOONHARVEST_GAM)
                                                    {
                                                        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                    }
                                                    else if (o->Type == MODEL_MOONHARVEST_SONGPUEN1)
                                                    {
                                                        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                    }
                                                    else if (o->Type == MODEL_MOONHARVEST_SONGPUEN2)
                                                    {
                                                        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                    }
                                                    else if (o->Type == MODEL_NPC_CHERRYBLOSSOM)
                                                    {
                                                        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, 2.f, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                                                        b->RenderMesh(0, RENDER_TEXTURE | RENDER_CHROME7, 0.25f, o->BlendMesh, 1.f, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                                                        b->RenderBody(RENDER_TEXTURE, 0.8f, o->BlendMesh, 2.f, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, 0);
                                                    }
                                                    else if (o->Type == MODEL_NPC_CHERRYBLOSSOMTREE)
                                                    {
                                                        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                    }
                                                    else if (o->Type == MODEL_DARK_PHEONIX)
                                                    {
                                                        b->StreamMesh = 0;
                                                        Vector(1.f, 1.0f, 1.0f, b->BodyLight);
                                                        b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh, BITMAP_CHROME);
                                                        b->StreamMesh = -1;
                                                        Vector(.6f, .6f, .6f, b->BodyLight);
                                                        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                    }
                                                    else if (o->Type == MODEL_STATUE_OF_SAINT)
                                                    {
                                                        if (o->CurrentAction == MONSTER01_DIE ||
                                                            (WorldTime - o->InitialSceneTime) < 1000
                                                            )
                                                        {
                                                            if (o->CurrentAction == MONSTER01_DIE)
                                                            {
                                                                o->Live = false;
                                                            }

                                                            PlayBuffer(SOUND_HIT_GATE2);
                                                            b->RenderMeshEffect(0, MODEL_STONE_COFFIN);
                                                        }
                                                        else
                                                        {
                                                            b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                            Vector(1.f, 1.f, 1.f, b->BodyLight);
                                                            b->RenderBody(RENDER_BRIGHT | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh, BITMAP_CHROME);
                                                            Vector(0.3f, 0.3f, 1.f, b->BodyLight);
                                                            b->RenderBody(RENDER_BRIGHT | RENDER_METAL, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh, BITMAP_CHROME);
                                                        }
                                                    }
                                                    else if (o->Type == MODEL_CASTLE_GATE && o->CurrentAction == MONSTER01_DIE)
                                                    {
                                                        o->Live = false;

                                                        PlayBuffer(SOUND_HIT_GATE2);
                                                        b->RenderMeshEffect(0, MODEL_GATE);
                                                    }
                                                    else if (o->Type == MODEL_STONE_COFFIN || o->Type == MODEL_STONE_COFFIN + 1)
                                                    {
                                                        if (o->SubType == 2 || o->SubType == 3)
                                                        {
                                                            if (o->SubType == 2)
                                                            {
                                                                Vector(0.1f, 0.3f, 0.6f, b->BodyLight);
                                                            }
                                                            else
                                                            {
                                                                Vector(0.1f, 0.6f, 0.3f, b->BodyLight);
                                                            }
                                                            b->RenderBody(RENDER_BRIGHT | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh, BITMAP_CHROME);
                                                        }
                                                        else
                                                        {
                                                            b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                            Vector(1.f, 1.f, 1.f, b->BodyLight);
                                                            b->RenderBody(RENDER_BRIGHT | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh, BITMAP_CHROME);
                                                            Vector(0.3f, 0.3f, 1.f, b->BodyLight);
                                                            b->RenderBody(RENDER_BRIGHT | RENDER_METAL, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh, BITMAP_CHROME);
                                                        }
                                                    }
                                                    else if (o->Type == MODEL_FLY_BIG_STONE1)
                                                    {
                                                        if (o->HiddenMesh == 99 && o->Visible)
                                                        {
                                                            o->HiddenMesh = -2;
                                                            b->RenderMeshEffect(0, MODEL_BIG_STONE_PART1);
                                                        }
                                                        else
                                                        {
                                                            Vector(1.f, 1.f, 1.f, b->BodyLight);
                                                            b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                            if (o->SubType <= 1)
                                                            {
                                                                Vector(1.0f, 0.2f, 0.1f, b->BodyLight);
                                                                b->RenderBody(RENDER_CHROME | RENDER_BRIGHT, o->Alpha, o->BlendMesh, 1.f, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                            }
                                                        }
                                                    }
                                                    else if (o->Type == MODEL_FLY_BIG_STONE2)
                                                    {
                                                        if (o->HiddenMesh == 99 && o->Visible)
                                                        {
                                                            o->HiddenMesh = -2;
                                                            b->RenderMeshEffect(0, MODEL_BIG_STONE_PART2);
                                                        }
                                                        else
                                                        {
                                                            b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                        }
                                                    }
                                                    else if (o->Type == MODEL_BIG_STONE_PART1 || o->Type == MODEL_BIG_STONE_PART2)
                                                    {
                                                        if (o->SubType == 1)
                                                        {
                                                            Vector(1.f, 1.f, 1.f, b->BodyLight);
                                                            b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                            Vector(1.0f, 0.2f, 0.1f, b->BodyLight);
                                                            b->RenderBody(RENDER_CHROME | RENDER_BRIGHT, o->Alpha, o->BlendMesh, 1.f, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                            Vector(1.f, 1.f, 1.f, b->BodyLight);
                                                        }
                                                        else if (o->SubType == 2 || o->SubType == 3)
                                                        {
                                                            Vector(0.5f, 1.0f, 0.3f, b->BodyLight);
                                                            b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                            Vector(1.f, 1.f, 1.f, b->BodyLight);
                                                        }
                                                        else
                                                        {
                                                            b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                        }
                                                    }
                                                    else if (o->Type == MODEL_WALL_PART1 || o->Type == MODEL_WALL_PART2)
                                                    {
                                                        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                    }
                                                    else if (o->Type == MODEL_CHAOS_CASTLE_KNIGHT || o->Type == MODEL_CHAOS_CASTLE_ELF)
                                                    {
                                                        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                    }
                                                    else if (o->Type == MODEL_COMBO && o->SubType == 1)
                                                    {
                                                        b->RenderBody(RENDER_TEXTURE | o->RenderType, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                    }
                                                    else if (o->Type == MODEL_CIRCLE_LIGHT && (o->SubType == 3 || o->SubType == 4))
                                                    {
                                                        Vector(0.1f, 0.1f, 10.f, b->BodyLight);
                                                        b->RenderBody(RENDER_TEXTURE | o->RenderType, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                        Vector(1.f, 1.f, 1.f, b->BodyLight);
                                                    }
                                                    else if (o->Type == MODEL_CIRCLE && (o->SubType == 2 || o->SubType == 3))
                                                    {
                                                        Vector(0.5f, 0.5f, 1.f, b->BodyLight);
                                                        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh, BITMAP_MAGIC_EMBLEM);
                                                        Vector(1.f, 1.f, 1.f, b->BodyLight);
                                                    }
                                                    else if (o->Type == MODEL_MULTI_SHOT1 || o->Type == MODEL_MULTI_SHOT2 || o->Type == MODEL_MULTI_SHOT3)
                                                    {
                                                        VectorCopy(o->Light, b->BodyLight);
                                                        b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                    }
                                                    else if (o->Type == MODEL_DESAIR)
                                                    {
                                                        Vector(1.f, 1.f, 1.f, o->Light);
                                                        VectorCopy(o->Light, b->BodyLight);
                                                        b->RenderBody(RENDER_TEXTURE | RENDER_DARK, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                    }
                                                    else if (o->Type == MODEL_DARK_SCREAM)
                                                    {
                                                        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, 1.f, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                    }
                                                    else if (o->Type == MODEL_DARK_SCREAM_FIRE)
                                                    {
                                                        b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, 1.f, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                    }
                                                    else if (o->Type == MODEL_ARROW_SPARK)
                                                    {
                                                        b->RenderBody(RENDER_TEXTURE | RENDER_CHROME5 | RENDER_BRIGHT, o->Alpha, o->BlendMesh, 1.f, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                    }
                                                    else if (o->Type == MODEL_SKULL)
                                                    {
                                                        Vector(1.f, 0.6f, 0.3f, b->BodyLight);
                                                        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);

                                                        vec3_t Light, p;

                                                        Vector(0.f, 0.f, 0.f, p);
                                                        b->TransformPosition(BoneTransform[2], p, o->EyeLeft, false);
                                                        b->TransformPosition(BoneTransform[3], p, o->EyeRight, false);

                                                        Vector(1.f, 0.f, 0.f, Light);
                                                        CreateSprite(BITMAP_LIGHT, o->EyeLeft, 1.f, Light, o);
                                                        CreateSprite(BITMAP_LIGHT, o->EyeRight, 1.f, Light, o);
                                                        Vector(0.5f, 0.5f, 0.5f, Light);
                                                        CreateSprite(BITMAP_SHINY + 1, o->EyeLeft, 0.5f, Light, o, (float)(rand() % 360));
                                                        CreateSprite(BITMAP_SHINY + 1, o->EyeRight, 0.5f, Light, o, (float)(rand() % 360));
                                                    }
                                                    else if (o->Type == MODEL_WAVES)
                                                    {
                                                        if (o->SubType == 3)
                                                            b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, -1, BITMAP_PINK_WAVE);
                                                        else if (o->SubType == 4 || o->SubType == 5)
                                                            b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, -1, BITMAP_PINK_WAVE);
                                                        else
                                                            b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                    }
                                                    else if (o->Type == MODEL_PROTECTGUILD)
                                                    {
                                                        Vector(0.4f, 0.6f, 1.f, b->BodyLight);
                                                        b->RenderBody(RENDER_BRIGHT | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                        b->RenderBody(RENDER_BRIGHT | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                    }
                                                    else if (o->Type == MODEL_WEBZEN_MARK)
                                                    {
                                                        Vector(1.f, 1.f, 1.f, b->BodyLight);
                                                        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                        Vector(1.f, 0.5f, 0.f, b->BodyLight);
                                                        b->RenderBody(RENDER_BRIGHT | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                    }
                                                    else if (o->Type == MODEL_MANA_RUNE && o->SubType == 0)
                                                    {
                                                        Vector(0.3f, 0.6f, 1.f, b->BodyLight);
                                                        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                        b->RenderBody(RENDER_BRIGHT | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                    }
                                                    else if (o->Type == MODEL_SKILL_JAVELIN)
                                                    {
                                                        Vector(1.f, 0.6f, 0.3f, b->BodyLight);
                                                        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                        b->RenderBody(RENDER_BRIGHT | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, 1);
                                                    }
                                                    else if (o->Type == MODEL_MAGIC_CAPSULE2 && o->SubType == 1)
                                                    {
                                                        b->RenderBody(RENDER_BRIGHT | RENDER_CHROME4, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, 1);
                                                        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                    }
                                                    else if (o->Type == MODEL_ARROW_AUTOLOAD)
                                                    {
                                                        vec3_t Light, p1, p2;
                                                        VectorCopy(o->Light, b->BodyLight);
                                                        Vector(0.f, 0.f, 0.f, p1);
                                                        Vector(1.0f, 0.8f, 0.3f, Light);
                                                        b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);

                                                        if (o->LifeTime > 15)
                                                        {
                                                            b->TransformPosition(BoneTransform[1], p1, p2);
                                                            CreateParticleFpsChecked(BITMAP_LIGHT + 1, p2, o->Angle, Light, 5, 0.6f);
                                                            b->TransformPosition(BoneTransform[3], p1, p2);
                                                            CreateParticleFpsChecked(BITMAP_LIGHT + 1, p2, o->Angle, Light, 5, 0.8f);
                                                        }
                                                    }
                                                    else if (o->Type == MODEL_INFINITY_ARROW)
                                                    {
                                                        vec3_t p1, p2;
                                                        Vector(0.f, 0.f, 0.f, p1);

                                                        for (int idx = 1; idx <= 9; ++idx)
                                                        {
                                                            if (idx == 5) continue;
                                                            b->TransformPosition(BoneTransform[idx], p1, p2);
                                                            CreateJointFpsChecked(BITMAP_FLARE + 1, p2, o->Position, o->Angle, 16, o, 20.f);
                                                        }
                                                    }
                                                    else if (o->Type >= MODEL_INFINITY_ARROW && o->Type <= MODEL_INFINITY_ARROW4)
                                                    {
                                                        VectorCopy(o->Light, b->BodyLight);
                                                        b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                    }
                                                    else if (o->Type == MODEL_SHIELD_CRASH || o->Type == MODEL_SHIELD_CRASH2)
                                                    {
                                                        VectorCopy(o->Light, b->BodyLight);
                                                        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                    }
                                                    else if (o->Type == MODEL_IRON_RIDER_ARROW)
                                                    {
                                                        VectorCopy(o->Light, b->BodyLight);
                                                        b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                    }
                                                    else if (o->Type == MODEL_BLADE_SKILL)
                                                    {
                                                        VectorCopy(o->Light, b->BodyLight);
                                                        for (int abc = 0; abc < 3; abc++)
                                                            b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                    }
                                                    else if (o->Type == MODEL_KENTAUROS_ARROW)
                                                    {
                                                        VectorCopy(o->Light, b->BodyLight);
                                                        b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                    }
                                                    else if (o->Type == MODEL_XMAS_EVENT_EARRING)
                                                    {
                                                        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                        b->RenderBody(RENDER_BRIGHT | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, -WorldTime * 0.0002f, o->BlendMeshTexCoordV, BITMAP_CHROME);
                                                        vec3_t vPos, vRelativePos, vLight;
                                                        Vector(1.f, 1.f, 1.f, vLight);
                                                        Vector(18.f, 0.f, 6.f, vRelativePos);
                                                        b->TransformPosition(BoneTransform[0], vRelativePos, vPos, true);
                                                        float fLumi = (sinf(WorldTime * 0.004f) + 1.0f) * 0.05f;
                                                        Vector(0.8f + fLumi, 0.8f + fLumi, 0.3f + fLumi, o->Light);
                                                        CreateSprite(BITMAP_LIGHT, vPos, 0.4f, o->Light, o, 0.5f);
                                                        if (rand_fps_check(15))
                                                        {
                                                            CreateParticle(BITMAP_SHINY, vPos, o->Angle, vLight, 5, 0.5f);
                                                        }
                                                        Vector(-18.f, 0.f, 6.f, vRelativePos);
                                                        b->TransformPosition(BoneTransform[0], vRelativePos, vPos, true);
                                                        CreateSprite(BITMAP_LIGHT, vPos, 0.4f, o->Light, o, 0.5f);
                                                        if (rand_fps_check(15))
                                                        {
                                                            CreateParticle(BITMAP_SHINY, vPos, o->Angle, vLight, 5, 0.5f);
                                                        }
                                                    }
                                                    else if (o->Type == MODEL_XMAS_EVENT_ICEHEART)
                                                    {
                                                        b->StreamMesh = 0;
                                                        Vector(1.f, 0.4f, 0.4f, b->BodyLight);
                                                        b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, -(int)WorldTime % 2000 * 0.0005f, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                        b->StreamMesh = -1;
                                                    }
                                                    else if (o->Type == MODEL_ARROW_BEST_CROSSBOW)
                                                    {
                                                        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                    }
                                                    else if (o->Type == MODEL_NEWYEARSDAY_EVENT_PIG)
                                                    {
                                                        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                        Vector(1.0f, 0.4f, 0.2f, b->BodyLight);
                                                        b->RenderBody(RENDER_CHROME3 | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                        Vector(1.0f, 1.f, 1.f, b->BodyLight);
                                                    }
                                                    else if (o->Type == MODEL_FENRIR_THUNDER)
                                                    {
                                                        if (o->SubType == 1)
                                                        {
                                                            VectorCopy(o->Light, b->BodyLight);
                                                            b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                            Vector(1.f, 1.f, 1.f, b->BodyLight);
                                                        }
                                                        else
                                                        {
                                                            b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                        }
                                                    }
                                                    else if (o->Type == MODEL_MAP_TORNADO)
                                                    {
                                                        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, -1);
                                                    }
                                                    else if (o->Type >= MODEL_SUMMONER_CASTING_EFFECT1 && o->Type <= MODEL_SUMMONER_CASTING_EFFECT4)
                                                    {
                                                        b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, -1);
                                                    }
                                                    else if (o->Type == MODEL_SUMMONER_SUMMON_SAHAMUTT)
                                                    {
                                                        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, -1);
                                                        b->RenderBody(RENDER_BRIGHT | RENDER_CHROME7, o->Alpha, o->BlendMesh, o->Alpha, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, -1);
                                                    }
                                                    else if (o->Type == MODEL_SUMMONER_SUMMON_NEIL)
                                                    {
                                                        if (o->Alpha < 0.7f)
                                                        {
                                                            b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, o->Alpha, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, -1);
                                                            b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 2, o->Alpha, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, -1);
                                                        }
                                                        else
                                                        {
                                                            b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, -1);
                                                            b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, -1);
                                                        }
                                                        Vector(1.0f, 0.0f, 0.0f, b->BodyLight);
                                                        b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, 1, o->Alpha, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, -1);

                                                        vec3_t  vPos, vRelative, vLight;
                                                        Vector(0.0f, 0.0f, 0.0f, vRelative);
                                                        Vector(1.0f, 0.0f, 0.0f, vLight);
                                                        for (int i = 51; i <= 59; ++i)
                                                        {
                                                            b->TransformPosition(BoneTransform[i], vRelative, vPos, false);
                                                            CreateSprite(BITMAP_LIGHT, vPos, 1.0f, vLight, o);
                                                        }

                                                        float Start_Frame = 0.f;
                                                        float End_Frame = 10.0f;
                                                        if (o->AnimationFrame >= Start_Frame && o->AnimationFrame <= End_Frame && o->CurrentAction == 0)
                                                        {
                                                            vec3_t  Light;
                                                            Vector(1.0f, 0.0f, 0.0f, Light);

                                                            vec3_t StartPos, StartRelative;
                                                            vec3_t EndPos, EndRelative;

                                                            float fActionSpeed = o->Velocity * static_cast<float>(FPS_ANIMATION_FACTOR);
                                                            float fSpeedPerFrame = fActionSpeed / 10.f;
                                                            float fAnimationFrame = o->AnimationFrame - fActionSpeed;
                                                            for (int i = 0; i < 10; i++)
                                                            {
                                                                b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);

                                                                Vector(0.f, 0.f, 0.f, StartRelative);
                                                                Vector(0.f, 0.f, 0.f, EndRelative);

                                                                b->TransformPosition(BoneTransform[51], StartRelative, StartPos, false);
                                                                b->TransformPosition(BoneTransform[59], EndRelative, EndPos, false);
                                                                CreateObjectBlur(o, StartPos, EndPos, Light, 2);

                                                                fAnimationFrame += fSpeedPerFrame;
                                                            }
                                                        }
                                                    }
                                                    else if (o->Type >= MODEL_SUMMONER_SUMMON_NEIL_NIFE1 && o->Type <= MODEL_SUMMONER_SUMMON_NEIL_NIFE3)
                                                    {
                                                        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, -1);
                                                        b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, -1);
                                                        Vector(1.0f, 0.0f, 0.0f, b->BodyLight);
                                                        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 1, o->Alpha, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, -1);
                                                    }
                                                    else if (o->Type >= MODEL_SUMMONER_SUMMON_NEIL_GROUND1 && o->Type <= MODEL_SUMMONER_SUMMON_NEIL_GROUND3)
                                                    {
                                                        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, o->Alpha, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, -1);
                                                        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 1, o->Alpha, -WorldTime * 0.001f, o->BlendMeshTexCoordV, -1);
                                                    }
                                                    else if (o->Type == MODEL_SUMMONER_SUMMON_LAGUL)
                                                    {
                                                        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, o->Alpha, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, -1);
                                                        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 1, o->Alpha, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, -1);
                                                    }
                                                    else if (o->Type >= MODEL_EFFECT_BROKEN_ICE0 && o->Type <= MODEL_EFFECT_BROKEN_ICE3)
                                                    {
                                                        VectorCopy(o->Light, b->BodyLight);
                                                        b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                    }
                                                    else if (o->Type == MODEL_MOVE_TARGETPOSITION_EFFECT)
                                                    {
                                                        Vector(1.0f, 0.7f, 0.3f, b->BodyLight);
                                                        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight);
                                                    }
                                                    else if (o->Type == MODEL_ARROW_DARKSTINGER)
                                                    {
                                                        b->BodyLight[0] = 0.7f;
                                                        b->BodyLight[1] = 0.7f;
                                                        b->BodyLight[2] = 0.9f;

                                                        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, 0, 1.f, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);

                                                        b->BodyLight[0] = 0.3f;
                                                        b->BodyLight[1] = 0.4f;
                                                        b->BodyLight[2] = 0.9f;
                                                        b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, 1, 1.f, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                                                        b->BodyLight[0] = 1.0f;
                                                        b->BodyLight[1] = 1.0f;
                                                        b->BodyLight[2] = 1.0f;
                                                    }
                                                    else if (o->Type == MODEL_FEATHER)
                                                    {
                                                        if (o->SubType == 2 || o->SubType == 3)
                                                            b->RenderBody(RENDER_TEXTURE | RENDER_DARK, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                        else
                                                            b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, 0, o->Alpha, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                    }
                                                    else if (o->Type == MODEL_ARROW_GAMBLE)
                                                    {
                                                        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, o->Alpha, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                    }
                                                    else if (o->Type == MODEL_DEMON)
                                                    {
                                                        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                        //b->RenderMesh(0, RENDER_TEXTURE|RENDER_BRIGHT, o->Alpha, 0, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_DEMONWING_R);
                                                    }
                                                    else if (o->Type == MODEL_SPIRIT_OF_GUARDIAN)
                                                    {
                                                        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                                                    }
                                                    else
                                                    {
                                                        bIsRendered = FALSE;
                                                    }

                if (bIsRendered == TRUE)
                {
                }
                else if (o->Type == MODEL_XMAS2008_SNOWMAN)
                {
                    if (o->CurrentAction == MONSTER01_DIE)
                    {
                        if ((int)o->LifeTime == 100)
                        {
                            o->LifeTime = 90;
                            o->m_bRenderShadow = false;

                            vec3_t vRelativePos, vWorldPos, Light;
                            Vector(1.0f, 1.0f, 1.0f, Light);
                            Vector(0.f, 0.f, 0.f, vRelativePos);

                            b->TransformPosition(o->BoneTransform[7], vRelativePos, vWorldPos, true);
                            CreateEffect(MODEL_XMAS2008_SNOWMAN_HEAD, vWorldPos, o->Angle, Light, 0, o, 0, 0);

                            CreateEffect(MODEL_XMAS2008_SNOWMAN_BODY, o->Position, o->Angle, Light, 0, o, 0, 0);
                        }
                    }
                    else
                    {
                        vec3_t vRelativePos, vWorldPos, Light;
                        Vector(0.f, 0.f, 0.f, vRelativePos);
                        Vector(0.8f, 0.8f, 0.9f, Light);

                        b->TransformPosition(o->BoneTransform[7], vRelativePos, vWorldPos, true);
                        CreateSprite(BITMAP_LIGHT, vWorldPos, 6.0f, Light, o);
                        CreateSprite(BITMAP_LIGHT, vWorldPos, 4.0f, Light, o);

                        b->TransformPosition(o->BoneTransform[34], vRelativePos, vWorldPos, true);
                        CreateParticleFpsChecked(BITMAP_SHINY, vWorldPos, o->Angle, Light, 7);
                        Vector(1.0f, 0.8f, 0.2f, Light);
                        CreateSprite(BITMAP_LIGHT, vWorldPos, 2.0f, Light, o);

                        if (o->CurrentAction == MONSTER01_WALK)
                        {
                            vRelativePos[0] = o->Position[0] + sinf(((rand() % 360) * 6.12)) * 40.0f;
                            vRelativePos[1] = o->Position[1] + sinf(((rand() % 360) * 6.12)) * 40.0f;
                            vRelativePos[2] = o->Position[2];
                            CreateParticleFpsChecked(BITMAP_SMOKE, vRelativePos, o->Angle, o->Light);
                        }

                        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                        b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                        b->RenderMesh(3, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                        b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, 1, 0.5f, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                    }
                }
                else if (o->Type == MODEL_XMAS2008_SNOWMAN_BODY)
                {
                    b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                    b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                    b->RenderMesh(3, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                    b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, 1, 0.5f, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                }
                else if (o->Type == MODEL_FEATHER_FOREIGN)
                {
                    if (o->SubType == 2 || o->SubType == 3)
                        b->RenderBody(RENDER_TEXTURE | RENDER_DARK, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                    else
                        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, 0, o->Alpha, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                }
                else if (o->Type == MODEL_SWELL_OF_MAGICPOWER)
                {
                    o->BlendMesh = 0;
                    Vector(0.7f, 0.4f, 0.9f, b->BodyLight);

                    if (o->LifeTime <= 20)
                    {
                        o->BlendMeshLight *= pow(0.86f, FPS_ANIMATION_FACTOR);
                    }
                    b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                }
                else if (o->Type == MODEL_ARROWSRE06)
                {
                    o->BlendMesh = 0;
                    VectorCopy(o->Light, b->BodyLight);
                    //Vector(0.7f, 0.2f, 0.9f, b->BodyLight);
                    if (o->LifeTime <= 10)
                    {
                        o->BlendMeshLight *= pow(0.8f, FPS_ANIMATION_FACTOR);
                    }
                    b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                }
                else if (o->Type == MODEL_XMAS2008_SNOWMAN_NPC)
                {
                    vec3_t vRelativePos, vWorldPos, Light;
                    Vector(0.f, 0.f, 0.f, vRelativePos);
                    Vector(0.8f, 0.8f, 0.9f, Light);

                    b->TransformPosition(o->BoneTransform[7], vRelativePos, vWorldPos, true);
                    CreateSprite(BITMAP_LIGHT, vWorldPos, 6.0f, Light, o);
                    CreateSprite(BITMAP_LIGHT, vWorldPos, 4.0f, Light, o);

                    b->TransformPosition(o->BoneTransform[34], vRelativePos, vWorldPos, true);
                    CreateParticleFpsChecked(BITMAP_SHINY, vWorldPos, o->Angle, Light, 7);
                    Vector(1.0f, 0.8f, 0.2f, Light);
                    CreateSprite(BITMAP_LIGHT, vWorldPos, 2.0f, Light, o);

                    b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                    b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                    b->RenderMesh(3, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                    b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, 1, 0.5f, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                }
                else if (o->Type == MODEL_XMAS2008_SANTA_NPC)
                {
                    vec3_t vRelativePos, vWorldPos, Light;
                    Vector(0.f, 0.f, 0.f, vRelativePos);

                    b->TransformPosition(o->BoneTransform[4], vRelativePos, vWorldPos, true);
                    Vector(1.0f, 0.8f, 0.2f, Light);
                    CreateSprite(BITMAP_LIGHT, vWorldPos, 8.0f, Light, o);

                    b->TransformPosition(o->BoneTransform[13], vRelativePos, vWorldPos, true);
                    Vector(1.0f, 0.4f, 0.0f, Light);
                    CreateSprite(BITMAP_LIGHT, vWorldPos, 5.0f, Light, o);

                    b->TransformPosition(o->BoneTransform[38], vRelativePos, vWorldPos, true);
                    Vector(1.0f, 0.8f, 0.2f, Light);
                    CreateSprite(BITMAP_LIGHT, vWorldPos, 2.0f, Light, o);

                    vRelativePos[1] = 17.0f;
                    b->TransformPosition(o->BoneTransform[53], vRelativePos, vWorldPos, true);
                    Vector(1.0f, 0.4f, 0.0f, Light);
                    RenderAurora(BITMAP_LIGHTMARKS, RENDER_BRIGHT, vWorldPos[0], vWorldPos[1], 2.0f, 2.0f, Light);

                    b->TransformPosition(o->BoneTransform[58], vRelativePos, vWorldPos, true);
                    Vector(1.0f, 0.4f, 0.0f, Light);
                    RenderAurora(BITMAP_LIGHTMARKS, RENDER_BRIGHT, vWorldPos[0], vWorldPos[1], 2.0f, 2.0f, Light);

                    Vector(0.f, 0.f, 0.f, vRelativePos);
                    int temp[11] = { 39, 41, 43, 44, 46, 49, 40, 42, 45, 47, 48 };
                    float fCos1 = cosf(WorldTime * 0.002f);
                    float fCos2 = sinf(WorldTime * 0.002f);
                    float fSize = 0.0f;
                    for (int i = 0; i < 11; i++)
                    {
                        b->TransformPosition(o->BoneTransform[temp[i]], vRelativePos, vWorldPos, true);

                        if (i < 6)
                        {
                            Vector(1.0f, 0.9f, 0.3f, Light);
                            fSize = 0.7f * fCos1;
                        }
                        else
                        {
                            Vector(1.0f, 0.5f, 0.5f, Light);
                            fSize = 0.7f * fCos2;
                        }
                        CreateSprite(BITMAP_LIGHT, vWorldPos, fSize, Light, o);
                    }

                    float fScale = 0.0f;
                    switch (o->CurrentAction)
                    {
                    case 1:
                        vRelativePos[0] = 20 + (rand() % 500 - 250) * 0.1f;
                        vRelativePos[1] = (rand() % 300 - 150) * 0.1f;
                        b->TransformPosition(o->BoneTransform[16], vRelativePos, vWorldPos, true);

                        Vector((rand() % 90 + 10) * 0.01f, (rand() % 90 + 10) * 0.01f, (rand() % 90 + 10) * 0.01f, Light);
                        fScale = (rand() % 5 + 5) * 0.1f;
                        CreateParticleFpsChecked(BITMAP_LIGHT, vWorldPos, o->Angle, Light, 13, fScale, o);
                        CreateParticleFpsChecked(BITMAP_LIGHT, vWorldPos, o->Angle, Light, 13, fScale, o);
                        CreateParticleFpsChecked(BITMAP_LIGHT, vWorldPos, o->Angle, Light, 13, fScale, o);
                        CreateParticleFpsChecked(BITMAP_SHINY, vWorldPos, o->Angle, Light, 7);

                        Vector(1.0f, 0.8f, 0.2f, Light);
                        Vector(0.f, 0.f, 0.f, vRelativePos);
                        b->TransformPosition(o->BoneTransform[16], vRelativePos, vWorldPos, true);
                        CreateSprite(BITMAP_LIGHT, vWorldPos, 2.5f, Light, o);
                        break;
                    case 2:
                        Vector(150.0f, 0.0f, 0.0f, vRelativePos);
                        b->TransformPosition(o->BoneTransform[6], vRelativePos, vWorldPos, true);
                        Vector(0.8f, 0.8f, 0.9f, Light);

                        if (o->AnimationFrame < 1) o->AI = 0;
                        if (o->AI == 0 && o->AnimationFrame > 1)
                        {
                            o->AI = 1;
                            CreateSprite(BITMAP_LIGHT, vWorldPos, 0.5f, Light, o);
                            CreateSprite(BITMAP_DS_SHOCK, vWorldPos, 0.15f, Light, o);
                            CreateEffectFpsChecked(BITMAP_FIRECRACKER0002, vWorldPos, o->Angle, Light, o->Skill);
                        }
                        break;
                    }

                    b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                }
                else if (o->Type == MODEL_CURSED_SANTA)
                {
                    if (o->CurrentAction == MONSTER01_DIE)
                    {
                        vec3_t vLight;
                        Vector(1.0 * o->Alpha, 0.8 * o->Alpha, 0.5 * o->Alpha, vLight);
                        vec3_t vPosition;
                        Vector(o->Position[0], o->Position[1], o->Position[2] + 200, vPosition);

                        if (o->AnimationFrame >= 13)//11
                        {
                            o->Angle[2] = 45;
                            if (o->AI != 0)
                            {
                                o->AI -= 1;
                                vec3_t vPos, Position;
                                Vector(o->Position[0], o->Position[1], o->Position[2] + 250, vPos);

                                vec3_t vLight;
                                Vector(0.7f, 0.3f, 0.0f, vLight);
                                for (int i = 0; i < 30; i++)
                                {
                                    CreateParticleFpsChecked(BITMAP_SPARK + 1, vPos, o->Angle, vLight, 27);
                                }

                                for (int i = 0; i < 20; i++)
                                {
                                    CreateParticleFpsChecked(BITMAP_SPARK + 1, vPos, o->Angle, vLight, 28);
                                }

                                Vector(vPos[0] + (rand() % 100 - 50), vPos[1] + (rand() % 100 - 50),
                                    vPos[2], Position);
                                CreateSprite(BITMAP_DS_SHOCK, Position, rand() % 10 * 0.1f + 1.5f, o->Light, o);

                                for (int i = 0; i < 60; i++)
                                {
                                    Vector(0.3f + (rand() % 700) * 0.001f, 0.3f + (rand() % 700) * 0.001f, 0.3f + (rand() % 700) * 0.001f, vLight);
                                    CreateParticleFpsChecked(BITMAP_SHINY, vPos, o->Angle, vLight, 9);
                                }

                                vPos[2] += 50;

                                for (int i = 0; i < 3; ++i)
                                {
                                    CreateEffectFpsChecked(MODEL_HALLOWEEN_CANDY_STAR, vPos, o->Angle, vLight, 1);
                                    CreateEffectFpsChecked(rand() % 4 + MODEL_XMAS_EVENT_BOX, vPos, o->Angle, vLight, 0, o);
                                    CreateEffectFpsChecked(rand() % 4 + MODEL_XMAS_EVENT_BOX, vPos, o->Angle, vLight, 0, o);
                                }
                            }

                            b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_GOOD_SANTA);
                            b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_GOOD_SANTA_BAGGAGE);
                            b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);

                            CreateSprite(BITMAP_LIGHT, vPosition, 5.5f, vLight, o);
                        }
                        else if (o->AnimationFrame >= 9)//9
                        {
                            o->AI = 3;
                            o->Alpha = 1.0f;

                            o->Angle[2] += 40.0f;

                            float fFade = (13.0f - o->AnimationFrame) / (13.0f - 9.0f);
                            b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_GOOD_SANTA);
                            b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_GOOD_SANTA_BAGGAGE);
                            b->RenderMesh(2, RENDER_TEXTURE, 1.0f - fFade, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);

                            b->RenderMesh(0, RENDER_TEXTURE, fFade, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                            b->RenderMesh(1, RENDER_TEXTURE, fFade, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);

                            CreateSprite(BITMAP_LIGHT, vPosition, 5.5f, vLight, o);

                            if (o->WeaponLevel == 0)
                            {
                                o->WeaponLevel = 1;
                                o->Position[0] = o->StartPosition[0] + (rand() % 20 + 20) * 0.1f;
                                o->Position[1] = o->StartPosition[1] + (rand() % 20 + 20) * 0.1f;
                            }
                            else
                            {
                                o->WeaponLevel = 0;
                                o->Position[0] = o->StartPosition[0] - (rand() % 20 + 20) * 0.1f;
                                o->Position[1] = o->StartPosition[1] - (rand() % 20 + 20) * 0.1f;
                            }
                        }
                        else
                        {
                            if (o->AI == 4 && o->AnimationFrame >= 3)
                            {
                                vec3_t Angle = { 0.0f, 0.0f, 0.0f };
                                vec3_t Position;
                                for (int i = 0; i < 36; ++i)
                                {
                                    Vector(0.f, 0.f, (float)(rand() % 360), Angle);
                                    Position[0] = o->Position[0] + rand() % 200 - 100;
                                    Position[1] = o->Position[1] + rand() % 200 - 100;
                                    Position[2] = o->Position[2];

                                    CreateJoint(BITMAP_FLARE, Position, Position, Angle, 2, NULL, 40);
                                }
                                o->AI = 3;
                            }
                            b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                            b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);

                            if (o->WeaponLevel == 0)
                            {
                                o->WeaponLevel = 1;
                                o->Position[0] = o->StartPosition[0] + (rand() % 40 + 40) * 0.1f;
                                o->Position[1] = o->StartPosition[1] + (rand() % 40 + 40) * 0.1f;
                            }
                            else
                            {
                                o->WeaponLevel = 0;
                                o->Position[0] = o->StartPosition[0] - (rand() % 40 + 40) * 0.1f;
                                o->Position[1] = o->StartPosition[1] - (rand() % 40 + 40) * 0.1f;
                            }
                        }
                    }
                    else
                    {
                        VectorCopy(o->Position, o->StartPosition);
                        o->WeaponLevel = 0;
                        o->AI = 4;
                        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                        b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                    }
                }
                else if (o->Type >= MODEL_LITTLESANTA && o->Type <= MODEL_LITTLESANTA_END)
                {
                    float fLumi = (sinf(WorldTime * 0.004f) + 1.2f) * 0.5f + 0.1f;
                    auto Rotation = (float)((int)(WorldTime * 0.1f) % 360);
                    vec3_t vWorldPos, vLight;

                    switch (o->Type)
                    {
                    case MODEL_LITTLESANTA:
                        Vector(0.5f, 0.5f, 0.0f, vLight);
                        break;
                    case MODEL_LITTLESANTA + 1:
                        Vector(0.3f, 0.8f, 0.4f, vLight);
                        break;
                    case MODEL_LITTLESANTA + 2:
                        Vector(0.8f, 0.1f, 0.1f, vLight);
                        break;
                    case MODEL_LITTLESANTA + 3:
                        Vector(0.3f, 0.3f, 0.8f, vLight);
                        break;
                    case MODEL_LITTLESANTA + 4:
                        Vector(0.9f, 0.9f, 0.9f, vLight);
                        break;
                    case MODEL_LITTLESANTA + 5:
                        Vector(0.9f, 0.9f, 0.9f, vLight);
                        break;
                    case MODEL_LITTLESANTA + 6:
                        Vector(0.8f, 0.4f, 0.0f, vLight);
                        break;
                    case MODEL_LITTLESANTA + 7:
                        Vector(0.9f, 0.5f, 0.7f, vLight);
                        break;
                    default:
                        break;
                    }
                    b->TransformByObjectBone(vWorldPos, o, 17);
                    CreateSprite(BITMAP_LIGHTMARKS, vWorldPos, 0.7f, vLight, o, Rotation);	//scale 0.7
                    CreateSprite(BITMAP_LIGHTMARKS, vWorldPos, 1.3f, vLight, o, Rotation);	//scale 1.3

                    b->TransformByObjectBone(vWorldPos, o, 20);
                    CreateSprite(BITMAP_LIGHTMARKS, vWorldPos, 0.7f, vLight, o, Rotation);	//scale 0.7
                    CreateSprite(BITMAP_LIGHTMARKS, vWorldPos, 1.3f, vLight, o, Rotation);	//scale 1.3

                    RenderAurora(BITMAP_LIGHTMARKS, RENDER_BRIGHT, o->Position[0], o->Position[1], 2.0f + fLumi, 2.0f + fLumi, vLight);
                    RenderAurora(BITMAP_LIGHTMARKS, RENDER_BRIGHT, o->Position[0], o->Position[1], 0.5f + fLumi, 0.5 + fLumi, vLight);

                    if (rand() % 50 <= 5)
                    {
                        CreateParticle(BITMAP_LIGHT + 3, o->Position, o->Angle, vLight, 0, 1.0f);
                    }

                    b->RenderBody(RENDER_TEXTURE, 0.9f, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                    Vector(b->BodyLight[0] * 0.5f, b->BodyLight[0] * 0.5f, b->BodyLight[0] * 0.5f, b->BodyLight);
                    b->RenderBody(RENDER_BRIGHT | RENDER_TEXTURE, 0.9f, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                }
                else if (o->Type == MODEL_DUEL_NPC_TITUS)
                {
                    b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                    b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                    b->RenderMesh(3, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                    b->RenderMesh(4, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                    b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, o->BlendMeshLight, o->BlendMeshTexCoordU, WorldTime * 0.0001f);

                    int iBones[] = { 9, 45, 23, 11 };
                    vec3_t vLight, vPos, vRelative;
                    Vector(1.0f, 1.0f, 1.0f, vLight);
                    vec3_t vLightFire;
                    Vector(1.0f, 0.2f, 0.0f, vLightFire);
                    for (int i = 0; i < 4; ++i)
                    {
                        float fScale = 1.0f;
                        if (i == 0)
                        {
                            b->TransformByObjectBone(vPos, o, iBones[i]);
                            CreateSprite(BITMAP_LIGHT, vPos, 3.0f, vLightFire, o);

                            fScale = 0.8f;
                            Vector((rand() % 10 - 5) * 1.0f, (rand() % 10 - 5) * 1.0f, (rand() % 10 - 5) * 1.0f, vRelative);
                            b->TransformByObjectBone(vPos, o, iBones[i], vRelative);
                        }
                        else
                        {
                            b->TransformByObjectBone(vPos, o, iBones[i]);
                            CreateSprite(BITMAP_LIGHT, vPos, 2.0f, vLightFire, o);

                            fScale = 0.6f;
                            Vector((rand() % 10 - 5) * 1.0f, (rand() % 10 - 5) * 1.0f, (rand() % 10 - 5) * 1.0f, vRelative);
                            b->TransformByObjectBone(vPos, o, iBones[i], vRelative);
                        }
                        switch (rand() % 3)
                        {
                        case 0:
                            CreateParticleFpsChecked(BITMAP_FIRE_HIK1, vPos, o->Angle, vLight, 3, fScale);
                            break;
                        case 1:
                            CreateParticleFpsChecked(BITMAP_FIRE_CURSEDLICH, vPos, o->Angle, vLight, 7, fScale);
                            break;
                        case 2:
                            CreateParticleFpsChecked(BITMAP_FIRE_HIK3, vPos, o->Angle, vLight, 3, fScale);
                            break;
                        }
                    }
                }
#ifdef ASG_ADD_TIME_LIMIT_QUEST_NPC
                else if (o->Type == MODEL_GAMBLE_NPC_MOSS || o->Type == MODEL_TIME_LIMIT_QUEST_NPC_ZAIRO)
#else	// ASG_ADD_TIME_LIMIT_QUEST_NPC
                else if (o->Type == MODEL_GAMBLE_NPC_MOSS)
#endif	// ASG_ADD_TIME_LIMIT_QUEST_NPC
                {
                    vec3_t vRelativePos, vWorldPos, Light;
                    Vector(0.f, 0.f, 0.f, vRelativePos);
                    Vector(0.8f, 0.8f, 0.8f, Light);
                    b->TransformPosition(o->BoneTransform[41], vRelativePos, vWorldPos, true);
                    CreateParticleFpsChecked(BITMAP_SMOKELINE1 + rand() % 3, vWorldPos, o->Angle, Light, 1, 0.6f, o);
                    CreateParticleFpsChecked(BITMAP_CLUD64, vWorldPos, o->Angle, Light, 6, 0.6f, o);

                    Vector(0.5f, 0.5f, 0.5f, Light);
                    b->TransformPosition(o->BoneTransform[55], vRelativePos, vWorldPos, true);
                    CreateSprite(BITMAP_LIGHT, vWorldPos, 2.0f, Light, o);

                    b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                }
                else if (o->Type == MODEL_DOPPELGANGER_NPC_LUGARD)
                {
                    b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                    b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                    b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                    b->RenderMesh(3, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                    b->RenderMesh(4, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                    // 날개
                    Vector(1.0f, 1.0f, 1.0f, b->BodyLight);
                    b->RenderMesh(5, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                    b->RenderMesh(5, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 5, 0.1f, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                }
                else if (o->Type == MODEL_DOPPELGANGER_NPC_BOX || o->Type == MODEL_DOPPELGANGER_NPC_GOLDENBOX)
                {
                    if (o->CurrentAction == MONSTER01_DIE)
                    {
                        o->Alpha = (10 - o->AnimationFrame) * 0.1f;

                        if (o->AnimationFrame > 9)
                            o->AnimationFrame = 9;
                    }

                    if (o->Type == MODEL_DOPPELGANGER_NPC_BOX)
                    {
                        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                        b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                    }
                    else if (o->Type == MODEL_DOPPELGANGER_NPC_GOLDENBOX)
                    {
                        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_DOPPELGANGER_GOLDENBOX2);
                        b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_DOPPELGANGER_GOLDENBOX1);
                        b->RenderMesh(0, RENDER_BRIGHT | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                        b->RenderMesh(1, RENDER_BRIGHT | RENDER_CHROME7, 0.5f, 1, 0.3f, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                    }

                    if (o->CurrentAction == MONSTER01_DIE && o->AnimationFrame > 1 && o->AnimationFrame < 9)
                    {
                        vec3_t vPos, vLight;
                        for (int i = 0; i < 1; ++i)
                        {
                            vPos[0] = o->Position[0] + rand() % 120 - 60;
                            vPos[1] = o->Position[1] + rand() % 120 - 60;
                            vPos[2] = o->Position[2];
                            Vector(0.3f, 0.3f, 0.3f, vLight);
                            CreateParticleFpsChecked(BITMAP_SMOKE, vPos, o->Angle, vLight, 54, 2.0f);
                            Vector(0.3f, 0.3f, 0.3f, vLight);
                            CreateParticleFpsChecked(BITMAP_SMOKE, vPos, o->Angle, vLight, 66, 2.0f, o);
                        }

                        if (rand_fps_check(2))
                        {
                            vPos[0] = o->Position[0] + rand() % 80 - 40;
                            vPos[1] = o->Position[1] + rand() % 80 - 40;
                            vPos[2] = o->Position[2] + rand() % 50 + 10;
                            Vector(0.2f, 0.5f, 1.0f, vLight);
                            CreateParticle(BITMAP_PIN_LIGHT, vPos, o->Angle, vLight, 2, 1.0f, o);
                            vPos[2] = o->Position[2] + rand() % 50 + 10;
                            Vector(1.0f, 0.8f, 0.5f, vLight);
                            CreateParticle(BITMAP_PIN_LIGHT, vPos, o->Angle, vLight, 3, 0.2f, o);
                        }

                        Vector(0.9f, 0.7f, 0.0f, vLight);
                        CreateParticleFpsChecked(BITMAP_SHINY, vPos, o->Angle, vLight, 3, 0.5f, o);
                        CreateParticleFpsChecked(BITMAP_SHINY, vPos, o->Angle, vLight, 3, 0.5f, o);

                        vPos[0] = o->Position[0];
                        vPos[1] = o->Position[1];
                        if (o->Type == MODEL_DOPPELGANGER_NPC_GOLDENBOX)
                            vPos[2] = o->Position[2] + 80;
                        else
                            vPos[2] = o->Position[2] + 50;
                        Vector(1.0f, 1.0f, 1.0f, vLight);
                        float fRot = (WorldTime * 0.0006f) * 360.0f;
                        CreateSprite(BITMAP_DS_EFFECT, vPos, 2.5f, vLight, o, fRot);
                        Vector(0.9f, 0.7f, 0.0f, vLight);
                        CreateSprite(BITMAP_SHINY + 5, vPos, 0.8f, vLight, o, -fRot);
                        float fLight = (sinf(WorldTime * 0.01f) + 1.0f) * 0.5f * 0.9f + 0.1f;
                        Vector(1.0f * fLight, 1.0f * fLight, 1.0f * fLight, vLight);
                        CreateSprite(BITMAP_FLARE, vPos, 1.5f, vLight, o);
                    }
                }
                else if (o->Type == MODEL_DOPPELGANGER_SLIME_CHIP)
                {
                    b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                }
                else if (o->Type == MODEL_FIRE_FLAME_GHOST)
                {
                    if (o->CurrentAction == MONSTER01_DIE)
                    {
                        Vector(1.0f, 0.6f, 0.9f, b->BodyLight);
                        o->m_bRenderShadow = false;
                        b->RenderBody(RENDER_TEXTURE, o->Alpha, -2, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                        b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                    }
                    else
                    {
                        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                        vec3_t vOriBodyLight;
                        VectorCopy(b->BodyLight, vOriBodyLight);
                        Vector(1.0f, 0.6f, 0.5f, b->BodyLight);
                        b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                        VectorCopy(vOriBodyLight, b->BodyLight);
                    }
                }
                else if (o->Type == MODEL_EFFECT_UMBRELLA_GOLD)
                {
                    b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                    b->RenderMesh(0, RENDER_BRIGHT | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                }
                else if (o->Type == MODEL_EFFECT_SD_AURA)
                {
                    b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, o->BlendMeshLight, o->BlendMeshTexCoordU, WorldTime * 0.0006f);
                }
                else if (o->Type == MODEL_UNITEDMARKETPLACE_CHRISTIN)
                {
                    b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                }
                else if (o->Type == MODEL_UNITEDMARKETPLACE_RAUL)
                {
                    b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                    b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                    b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                    b->RenderMesh(3, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                    b->RenderMesh(4, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                    b->RenderMesh(5, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                    b->RenderMesh(6, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                    b->RenderMesh(7, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                    b->RenderMesh(8, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                    b->RenderMesh(9, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                    b->RenderMesh(10, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                    b->RenderMesh(11, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                    b->RenderMesh(12, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                    b->RenderMesh(13, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                    b->RenderMesh(14, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                }
                else if (o->Type == MODEL_UNITEDMARKETPLACE_JULIA)
                {
                    vec3_t vRelativePos, vWorldPos, Light;
                    Vector(0.f, 0.f, 0.f, vRelativePos);
                    float fScale = 0.0f;

                    if (o->CurrentAction == 0 && rand_fps_check(5))
                    {
                        Vector(0.f, 0.f, 0.f, vWorldPos);
                        Vector((rand() % 90 + 10) * 0.01f, (rand() % 90 + 10) * 0.01f, (rand() % 90 + 10) * 0.01f, Light);
                        fScale = (rand() % 5 + 5) * 0.1f;
                        Vector(0.f, 0.f, 0.f, vRelativePos);
                        vRelativePos[0] = -5 + (rand() % 1000 - 500) * 0.01f;
                        vRelativePos[1] = (rand() % 300 - 150) * 0.01f;
                        b->TransformPosition(o->BoneTransform[127], vRelativePos, vWorldPos, true);
                        CreateParticle(BITMAP_LIGHT, vWorldPos, o->Angle, Light, 13, fScale, o);
                        CreateParticle(BITMAP_LIGHT, vWorldPos, o->Angle, Light, 12, fScale * 0.1f, o);
                    }

                    Vector(0.f, 0.f, 0.f, vRelativePos);
                    Vector(0.f, 0.f, 0.f, vWorldPos);
                    Vector(1.0f, 1.0f, 1.0f, Light);
                    b->TransformPosition(o->BoneTransform[71], vRelativePos, vWorldPos, true);
                    CreateSprite(BITMAP_LIGHT, vWorldPos, 3.0f, Light, o);

                    vec3_t	vLight;
                    float fLight = (sinf(WorldTime * 0.001f) + 1.0f) * 0.5f * 0.9f + 0.4f;
                    Vector(1.0f * fLight, 1.0f * fLight, 1.0f * fLight, vLight);
                    CreateSprite(BITMAP_FLARE, vWorldPos, 1.5f, vLight, o);

                    b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                }
                else if (o->Type == MODEL_TERSIA)
                {
                    b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                    b->RenderMesh(0, RENDER_BRIGHT | RENDER_CHROME, o->Alpha * 0.4f, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                }
                else if (o->Type == MODEL_LUCKYITEM_NPC)
                {
                    b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                    b->RenderMesh(0, RENDER_BRIGHT | RENDER_CHROME, o->Alpha * 0.4f, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                }
                else if (o->Type == MODEL_KARUTAN_NPC_VOLVO)
                {
                    b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight,
                        o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                    float fLumi = (sinf(WorldTime * 0.001f) + 1.0f) * 0.45f + 0.1f;
                    b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, fLumi,
                        o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_VOLO_SKIN_EFFECT);
                }
                else if (o->Type == MODEL_WOLF_HEAD_EFFECT2)
                {
                    float _BlendLight = o->BlendMeshLight;
                    Vector(0.4f, 0.4f, 0.6f, o->Light);
                    VectorCopy(o->Light, b->BodyLight);
                    VectorScale(b->BodyLight, 0.3f, b->BodyLight);
                    b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, _BlendLight * 0.5f, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                }
                else if (o->Type == MODEL_SHOCKWAVE01)
                {
                    if (o->SubType == 1)
                    {
                        VectorCopy(o->Light, b->BodyLight);
                        VectorScale(b->BodyLight, 5.0f, b->BodyLight);
                        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_DAMAGE2);
                    }
                    else if (o->SubType == 2)
                    {
                        VectorCopy(o->Light, b->BodyLight);
                        VectorScale(b->BodyLight, 15.0f, b->BodyLight);
                        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                    }
                    else if (o->SubType == 3)
                    {
                        VectorCopy(o->Light, b->BodyLight);
                        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_DAMAGE2);
                        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_DAMAGE2);
                    }
                    else if (o->SubType == 4 || o->SubType == 5)
                    {
                        VectorCopy(o->Light, b->BodyLight);
                        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_DAMAGE2);
                    }
                    else
                    {
                        VectorCopy(o->Light, b->BodyLight);
                        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                    }
                }
                else if (o->Type == MODEL_SHOCKWAVE02)
                {
                    VectorCopy(o->Light, b->BodyLight);
                    b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                    b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                }
                else if (o->Type == MODEL_SHOCKWAVE_SPIN01)
                {
                    VectorCopy(o->Light, b->BodyLight);
                    b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh, BITMAP_DAMAGE1);
                }
                else if (o->Type == MODEL_DRAGON_KICK_DUMMY || o->Type == MODEL_DOWN_ATTACK_DUMMY_L
                    || o->Type == MODEL_DOWN_ATTACK_DUMMY_R || o->Type == MODEL_WOLF_HEAD_EFFECT)
                {
                    b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                }
                else if (o->Type == MODEL_WINDFOCE)
                {
                    VectorCopy(o->Light, b->BodyLight);
                    VectorScale(b->BodyLight, 6.0f, b->BodyLight);
                    b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                }
                else if (o->Type == MODEL_WINDFOCE_MIRROR)
                {
                    VectorCopy(o->Light, b->BodyLight);
                    VectorScale(b->BodyLight, 6.0f, b->BodyLight);
                    b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                }
                else if (o->Type == MODEL_SHOCKWAVE_GROUND01)
                {
                    if (o->SubType == 1)
                    {
                        VectorCopy(o->Light, b->BodyLight);
                        VectorScale(b->BodyLight, 6.0f, b->BodyLight);
                        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_KWAVE2);
                    }
                    else
                    {
                        VectorCopy(o->Light, b->BodyLight);
                        VectorScale(b->BodyLight, 6.0f, b->BodyLight);
                        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                    }
                }
                else if (o->Type == MODEL_DRAGON_LOWER_DUMMY)
                {
                    Vector(1.0f, 0.8f, 0.2f, o->Light);
                    VectorCopy(o->Light, b->BodyLight);
                    VectorScale(b->BodyLight, o->Alpha, b->BodyLight);
                    // grandmark2.jpg
                    b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, o->BlendMeshLight, o->BlendMeshTexCoordU, -(int)WorldTime % 2000 * 0.0001f);
                    b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 2, o->BlendMeshLight, o->BlendMeshTexCoordU, -(int)WorldTime % 2000 * 0.0001f);

                    Vector(1.0f, 0.2f, 0.1f, o->Light);
                    VectorCopy(o->Light, b->BodyLight);
                    VectorScale(b->BodyLight, o->Alpha, b->BodyLight);
                    // lines2.jpg
                    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 1, o->BlendMeshLight, o->BlendMeshTexCoordU, -(int)WorldTime % 2000 * 0.0001f);
                }
                else if (o->Type == MODEL_VOLCANO_OF_MONK)
                {
                    if (o->SubType == 1)
                    {
                        VectorCopy(o->Light, b->BodyLight);
                        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);

                        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_VOLCANO_CORE);
                    }
                }
                else if (o->Type == MODEL_VOLCANO_STONE)
                {
                    VectorCopy(o->Light, b->BodyLight);
                    b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                }
                else if (o->Type == MODEL_SWORD_FORCE)
                {
                    if (o->SubType == 2 || o->SubType == 3)
                    {
                        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh, BITMAP_KNIGHTST_BLUE);
                    }
                    else
                    {
                        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                    }
                }
                else
                {
                    b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                }
                if (g_isCharacterBuff(o, eDeBuff_Freeze))
                {
                    b->RenderBody(RENDER_TEXTURE, o->Alpha, -2, 1.f, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                }
                else if (g_isCharacterBuff(o, eDeBuff_BlowOfDestruction))
                {
                    Vector(0.3f, 0.5f, 1.f, b->BodyLight);
                    b->RenderBody(RENDER_TEXTURE, o->Alpha, -2, 1.f, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                }

                if (o->Type == MODEL_HYDRA)
                {
                    b->BeginRender(o->Alpha);
                    float Light = sinf(WorldTime * 0.002f) * 0.3f + 0.5f;
                    b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, 0, Light, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, b->IndexTexture[6]);
                    float TexCoord = (float)((int)WorldTime % 100) * 0.01f;
                    b->RenderMesh(3, RENDER_CHROME | RENDER_BRIGHT, o->Alpha, 3, Light, o->BlendMeshTexCoordU, -TexCoord);
                    b->EndRender();
                }
                if (o->Type == MODEL_NPC_ARCHANGEL_MESSENGER || o->Type == MODEL_NPC_ARCHANGEL)
                {
                    b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight);
                }
                if (o->Type == MODEL_ARROW_DRILL)
                {
                    vec3_t Position, p, Light;

                    Vector(1.f, 0.6f, 0.4f, Light);
                    Vector(0.f, 0.f, 0.f, p);
                    b->TransformPosition(BoneTransform[1], p, Position);
                    CreateSprite(BITMAP_SHINY + 1, Position, (float)(sinf(WorldTime * 0.002f) * 0.3f + 1.3f), Light, o, (float)(rand() % 360));
                    CreateSprite(BITMAP_LIGHT, Position, 1.5f, Light, o, 90.f);
                    CreateSprite(BITMAP_SHINY + 1, Position, (float)(sinf(WorldTime * 0.002f) * 0.3f + 1.3f), Light, o, (float)(rand() % 360));
                    CreateParticleFpsChecked(BITMAP_SPARK, Position, o->Angle, Light);
                }
                if (o->Type == MODEL_ARROW_TANKER_HIT || o->Type == MODEL_ARROW_TANKER)
                {
                    b->BodyLight[0] = 1.0f;
                    b->BodyLight[1] = 1.0f;
                    b->BodyLight[2] = 1.0f;

                    b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, -2, 1.f, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);

                    vec3_t p1, p2;
                    BMD* b = &Models[o->Type];

                    Vector(1.0f, 1.0f, 1.0f, o->Light);
                    Vector(0.0f, 1.5f, 0.0f, p1);
                    b->TransformPosition(BoneTransform[2], p1, p2);
                    CreateParticleFpsChecked(BITMAP_FIRE + 1, o->Position, o->Angle, o->Light, 8, o->Scale - 0.4f, o);
                    CreateParticleFpsChecked(BITMAP_FIRE + 1, o->Position, o->Angle, o->Light, 8, o->Scale - 0.4f, o);
                    CreateParticleFpsChecked(BITMAP_SMOKE, o->Position, o->Angle, o->Light, 38, o->Scale, o);
                    Vector(1.0f, 0.4f, 0.0f, o->Light);
                    CreateParticleFpsChecked(BITMAP_FIRE + 1, o->Position, o->Angle, o->Light, 9, o->Scale - 0.4f, o);

                    for (int i = 0; i < 10; i++)
                    {
                        Vector(1.0f - (i * 0.1f), 0.4f - (i * 0.04f), 0.0f, o->Light);
                        Vector(15.0f * i, 1.5f, 0.0f, p1);
                        b->TransformPosition(BoneTransform[1], p1, p2);
                        CreateSprite(BITMAP_SPARK + 1, p2, 3.0f + (i * 0.9f), o->Light, o);
                    }
                }
            }
    }
}

void RenderObject(OBJECT* o, bool Translate, int Select, int ExtraMon)
{
    if (Calc_RenderObject(o, Translate, Select, ExtraMon) == false)
    {
        return;
    }
    Draw_RenderObject(o, Translate, Select, ExtraMon);
}

void RenderObject_AfterImage(OBJECT* o, bool Translate, int Select, int ExtraMon)
{
    float fAnimationFrame = o->AnimationFrame;
    float fAni1, fAni2;

    fAni1 = fAnimationFrame - 1.4f;
    fAni2 = fAnimationFrame - 0.7f;
    if (fAni1 > 0.0f)
    {
        o->Alpha = 0.2f;
        o->AnimationFrame = fAni1;
        RenderObject(o, Translate, Select, 0);
    }
    if (fAni2 > 0.0f)
    {
        o->Alpha = 0.6f;
        o->AnimationFrame = fAni2;
        RenderObject(o, Translate, Select, 0);
    }

    o->Alpha = 1.0f;
    o->AnimationFrame = fAnimationFrame;
    RenderObject(o, Translate, Select, 0);
}

void RenderCharacter_AfterImage(CHARACTER* pCha, PART_t* pPart, bool Translate, int Select, float AniInterval1, float AniInterval2)
{
    OBJECT* pObj = &pCha->Object;
    float fAnimationFrame = pObj->AnimationFrame;
    float fAni1, fAni2;
    int Type = pPart->Type;

    fAni1 = (float)(fAnimationFrame - AniInterval1);
    fAni2 = (float)(fAnimationFrame - AniInterval2);

    vec3_t vPos, vStartPos, vOrgPos;
    VectorCopy(pObj->Position, vOrgPos);
    VectorCopy(pObj->Position, vPos);
    VectorCopy(pObj->StartPosition, vStartPos);
    VectorSubtract(vPos, vStartPos, vPos);
    float fDis = fAni1 / Models[pObj->Type].Actions[pObj->CurrentAction].NumAnimationKeys;

    if (fAni1 > 0.0f)
    {
        pObj->Alpha = 0.3f;
        pObj->AnimationFrame = fAni1;
        VectorScale(vPos, fDis, vPos);
        VectorAdd(vStartPos, vPos, vPos);
        VectorCopy(vPos, pObj->Position);
        Calc_ObjectAnimation(pObj, Translate, Select);
        RenderPartObject(pObj, Type, pPart, pCha->Light, pObj->Alpha, pPart->Level, pPart->ExcellentFlags, pPart->AncientDiscriminator, false, false, Translate, Select);
    }

    VectorCopy(vOrgPos, vPos);
    VectorCopy(pObj->StartPosition, vStartPos);
    VectorSubtract(vPos, vStartPos, vPos);
    if (fAni2 > 0.0f)
    {
        fDis = fAni2 / Models[pObj->Type].Actions[pObj->CurrentAction].NumAnimationKeys;
        pObj->Alpha = 0.5f;
        pObj->AnimationFrame = fAni2;
        VectorScale(vPos, fDis, vPos);
        VectorAdd(vStartPos, vPos, vPos);
        VectorCopy(vPos, pObj->Position);
        Calc_ObjectAnimation(pObj, Translate, Select);
        RenderPartObject(pObj, Type, pPart, pCha->Light, pObj->Alpha, pPart->Level, pPart->ExcellentFlags, pPart->AncientDiscriminator, false, false, Translate, Select);
    }

    VectorCopy(vOrgPos, pObj->Position);
    pObj->Alpha = 1.0f;
    pObj->AnimationFrame = fAnimationFrame;
    Calc_ObjectAnimation(pObj, Translate, Select);
    RenderPartObject(pObj, Type, pPart, pCha->Light, pObj->Alpha, pPart->Level, pPart->ExcellentFlags, pPart->AncientDiscriminator, false, false, Translate, Select);
}

void RenderObjectVisual(OBJECT* o)
{
    BMD* b = &Models[o->Type];
    vec3_t p, Position;
    vec3_t Light;
    float Luminosity = (float)(rand() % 30 + 70) * 0.01f;
    int Bitmap;
    float Scale;
    float Rotation;
    Vector(0.f, 0.f, 0.f, p);

    switch (gMapManager.WorldActive)
    {
    case WD_0LORENCIA:
        switch (o->Type)
        {
        case MODEL_WATERSPOUT:
            o->BlendMeshLight = 1.f;
            o->BlendMeshTexCoordV = -(int)WorldTime % 1000 * 0.001f;
            if (rand_fps_check(2))
            {
                Vector((float)(rand() % 32 - 16), -20.f, (float)(rand() % 32 - 16), p);
                b->TransformPosition(BoneTransform[1], p, Position);
                CreateParticle(BITMAP_SMOKE, Position, o->Angle, o->Light);
                Vector((float)(rand() % 32 - 16), -80.f, (float)(rand() % 32 - 16), p);
                b->TransformPosition(BoneTransform[4], p, Position);
                CreateParticle(BITMAP_SMOKE, Position, o->Angle, o->Light);
            }
            break;
        case MODEL_MERCHANT_ANIMAL01:
            Scale = Luminosity * 5.f;
            Vector(Luminosity * 0.6f, Luminosity * 0.3f, Luminosity * 0.1f, Light);
            b->TransformPosition(BoneTransform[48], Position, p);
            CreateSprite(BITMAP_LIGHT, p, Scale, Light, o);
            b->TransformPosition(BoneTransform[57], Position, p);
            CreateSprite(BITMAP_LIGHT, p, Scale, Light, o);
            /*b->TransformPosition(BoneTransform[51],Position,p);
            CreateSprite(BITMAP_LIGHT,p,Scale,Light,o);
            b->TransformPosition(BoneTransform[54],Position,p);
            CreateSprite(BITMAP_LIGHT,p,Scale,Light,o);*/
            break;
        }
        break;
    case WD_2DEVIAS:
        switch (o->Type)
        {
        case 100:
            Vector(Luminosity * 1.f, Luminosity * 1.f, Luminosity * 1.f, Light);
            Rotation = (float)((int)(WorldTime * 0.1f) % 360);
            Vector(0.f, 0.f, 150.f, p);
            b->TransformPosition(BoneTransform[0], p, Position);
            CreateSprite(BITMAP_LIGHTNING + 1, Position, 2.5f, Light, o, Rotation);
            CreateSprite(BITMAP_LIGHTNING + 1, Position, 2.5f, Light, o, -Rotation);
            break;
        case 103:		//. Sleddog
            if (b->CurrentAnimationFrame == b->Actions[o->CurrentAction].NumAnimationKeys - 1) {
                if (rand_fps_check(32))
                    SetAction(o, 1);
                else
                    SetAction(o, 0);
            }
            break;
#ifdef DEVIAS_XMAS_EVENT2007
        case 110:
        {
            b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
            Vector(1.f, 1.0f, 1.f, b->BodyLight);
            b->RenderMesh(0, RENDER_BRIGHT | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        }
        break;
#endif	// DEVIAS_XMAS_EVENT2007
#ifdef DEVIAS_XMAS_EVENT
        case 105:
        {
            Vector(1.f, 1.0f, 1.f, b->BodyLight);
            b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
            b->RenderMesh(0, RENDER_BRIGHT | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        }
        break;
        case 106:
        {
            Vector(1.0f, 1.0f, 1.0f, b->BodyLight);
            b->RenderMesh(3, RENDER_BRIGHT | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_CHROME8);
        }
        break;
#endif //DEVIAS_XMAS_EVENT
        }
        break;
    case WD_3NORIA:
        switch (o->Type)
        {
        case 9:
            Vector(Luminosity * 0.4f, Luminosity * 0.7f, Luminosity * 1.f, Light);
            b->TransformPosition(BoneTransform[1], p, Position);
            CreateSprite(BITMAP_LIGHT, Position, 1.5f, Light, o);
            break;
        case 35:
            Vector(Luminosity * 0.4f, Luminosity * 0.7f, Luminosity * 1.f, Light);
            b->TransformPosition(BoneTransform[3], p, Position);
            CreateSprite(BITMAP_LIGHT, Position, 1.5f, Light, o);
            break;
        case 1:
            Vector(Luminosity * 0.4f, Luminosity * 0.7f, Luminosity * 1.f, Light);
            b->TransformPosition(BoneTransform[2], p, Position);
            CreateSprite(BITMAP_LIGHT, Position, 0.5f, Light, o);
            b->TransformPosition(BoneTransform[4], p, Position);
            CreateSprite(BITMAP_LIGHT, Position, 0.5f, Light, o);
            b->TransformPosition(BoneTransform[6], p, Position);
            CreateSprite(BITMAP_LIGHT, Position, 0.5f, Light, o);
            break;
        case 17:
            Vector(Luminosity * 0.4f, Luminosity * 0.7f, Luminosity * 1.f, Light);
            b->TransformPosition(BoneTransform[4], p, Position);
            CreateSprite(BITMAP_LIGHT, Position, 1.f, Light, o);
            b->TransformPosition(BoneTransform[7], p, Position);
            CreateSprite(BITMAP_LIGHT, Position, 1.f, Light, o);
            b->TransformPosition(BoneTransform[10], p, Position);
            CreateSprite(BITMAP_LIGHT, Position, 1.f, Light, o);
            b->TransformPosition(BoneTransform[13], p, Position);
            CreateSprite(BITMAP_LIGHT, Position, 1.f, Light, o);
            break;
        case 39:
            Vector(Luminosity * 0.4f, Luminosity * 0.8f, Luminosity * 1.f, Light);
            Rotation = (float)((int)(WorldTime * 0.1f) % 360);
            b->TransformPosition(BoneTransform[57], p, Position);
            CreateSprite(BITMAP_LIGHTNING + 1, Position, 1.f, Light, o, Rotation);
            CreateSprite(BITMAP_LIGHTNING + 1, Position, 1.f, Light, o, -Rotation);

            Vector(1.f, 1.f, 1.f, Light);

            for (int i = 61; i <= 65; i++)
            {
                b->TransformPosition(BoneTransform[i], p, Position);
                CreateSprite(BITMAP_LIGHT, Position, 1.f, Light, o);
                if (rand_fps_check(32))
                {
                    CreateParticle(BITMAP_SHINY, Position, o->Angle, Light);
                    CreateParticle(BITMAP_SHINY, Position, o->Angle, Light, 1);
                }
            }
            b->TransformPosition(BoneTransform[58], p, Position);
            if (rand_fps_check(8))
            {
                vec3_t Angle;

                for (int i = 0; i < 8; i++)
                {
                    Vector((float)(rand() % 60 + 60), 90.f + 50.f, (float)(rand() % 30), Angle);
                    CreateJoint(BITMAP_JOINT_SPARK, Position, Position, Angle);
                    CreateParticle(BITMAP_SPARK, Position, Angle, Light);
                }
            }
            break;
        }
        break;
    case WD_4LOSTTOWER:
        switch (o->Type)
        {
        case 19:
        case 20:
            if (o->Type == 19)
            {
                Bitmap = BITMAP_MAGIC + 1;
                Vector(Luminosity * 1.f, Luminosity * 0.2f, Luminosity * 0.f, Light);
            }
            else
            {
                Bitmap = BITMAP_LIGHTNING + 1;
                Vector(Luminosity * 0.4f, Luminosity * 0.8f, Luminosity * 1.f, Light);
            }
            Rotation = (float)((int)(WorldTime * 0.1f) % 360);
            b->TransformPosition(BoneTransform[15], p, Position);
            CreateSprite(Bitmap, Position, 0.3f, Light, o, Rotation);
            CreateSprite(Bitmap, Position, 0.3f, Light, o, -Rotation);
            b->TransformPosition(BoneTransform[19], p, Position);
            CreateSprite(Bitmap, Position, 0.3f, Light, o, Rotation);
            CreateSprite(Bitmap, Position, 0.3f, Light, o, -Rotation);
            b->TransformPosition(BoneTransform[21], p, Position);
            CreateSprite(Bitmap, Position, 1.5f, Light, o, Rotation);
            CreateSprite(Bitmap, Position, 1.5f, Light, o, -Rotation);
            break;
        case 40:
            Vector(Luminosity * 1.f, Luminosity * 1.f, Luminosity * 1.f, Light);
            Rotation = (float)((int)(WorldTime * 0.1f) % 360);
            VectorCopy(o->Position, Position);
            Vector(0.f, 0.f, 260.f, p);
            VectorAdd(Position, p, Position);
            CreateSprite(BITMAP_LIGHTNING + 1, Position, 2.5f, Light, o, Rotation);
            CreateSprite(BITMAP_LIGHTNING + 1, Position, 2.5f, Light, o, -Rotation);
            /*{
                //Vector(1.f,1.f,1.f,Light);
                Vector(0.f,0.f,1.f,Light);
                for ( int j = 0; j < 2; ++j)
                {
                    for ( int i = 0; i < 3; ++i)
                    {
                        VectorCopy( o->Position, Position);
                        Position[0] += ( float)( 2 * i - 2) / 2.f * 100.f + ( float)( rand() % 31 - 15);
                        Position[1] += ( float)( 2 * j - 1) / 1.f * 70.f-80.f + ( float)( rand() % 31 - 15);
                        VectorCopy( Position, p);
                        p[2] += 300.f;
                        CreateJoint(BITMAP_JOINT_LASER+1,Position,p,o->Angle,0,o,50.f);
                    }
                }
            }*/
            break;
        }
    case WD_6STADIUM:
        switch (o->Type)
        {
        case 9:
            Scale = Luminosity * 5.f;
            Vector(Luminosity * 0.6f, Luminosity * 0.3f, Luminosity * 0.1f, Light);
            b->TransformPosition(BoneTransform[1], Position, p);
            CreateSprite(BITMAP_LIGHT, p, Scale, Light, o);
            break;
        }
        break;
    case WD_8TARKAN:
        switch (o->Type)
        {
        case 60:
            if (o->HiddenMesh != -2)
            {
                for (int i = 0; i < 20; ++i)
                {
                    CreateParticleFpsChecked(BITMAP_SMOKE, o->Position, o->Angle, o->Light, 6, o->Scale);
                }
            }
            o->HiddenMesh = -2;
            break;

        case 63:
            Luminosity = (float)sinf((WorldTime + (o->Angle[2] * 5)) * 0.002f) * 0.3f + 0.7f;

            Scale = Luminosity * 1.5f;
            Vector(Luminosity / 1.7f, Luminosity, Luminosity, Light);
            b->TransformPosition(BoneTransform[2], p, Position);
            CreateSprite(BITMAP_IMPACT, Position, Scale, Light, o);
            break;

        case 64:
            Luminosity = (float)sinf((WorldTime + (o->Angle[2] * 5)) * 0.002f) * 0.3f + 0.7f;

            Scale = Luminosity * 1.5f;
            Vector(Luminosity, Luminosity * 0.32f, Luminosity * 0.32f, Light);
            b->TransformPosition(BoneTransform[2], p, Position);
            CreateSprite(BITMAP_IMPACT, Position, Scale, Light, o);
            break;

        case 70:
            o->HiddenMesh = -2;
            if (rand_fps_check(5))
                CreateParticle(BITMAP_SMOKE, o->Position, o->Angle, o->Light, 7, o->Scale);
            break;

        case 76:
            o->HiddenMesh = -2;
            {
                bool Smoke = false;

                if (((int)WorldTime % 5000) > 4500) Smoke = true;
                if (Smoke)
                    CreateParticleFpsChecked(BITMAP_SMOKE, o->Position, o->Angle, o->Light, 4, o->Scale);
            }
            break;
        case 83:
            o->HiddenMesh = -2;
            {
                bool Smoke = false;
                int  inter = (int)o->Angle[2] * 10;
                int  timing = (int)WorldTime % 10000;

                if (timing > 3500 + inter && timing < 4000 + inter) Smoke = true;
                if (Smoke)
                {
                    Vector(1.f, 1.f, 1.f, Light);

                    CreateParticleFpsChecked(BITMAP_SMOKE, o->Position, o->Angle, o->Light, 8, o->Scale);
                    if (rand_fps_check(3))
                    {
                        Position[0] = o->Position[0] + (rand() % 128 - 64);
                        Position[1] = o->Position[1] + (rand() % 128 - 64);
                        Position[2] = o->Position[2];
                        CreateParticleFpsChecked(BITMAP_SMOKE, Position, o->Angle, o->Light, 4, o->Scale * 0.5f);
                        CreateEffectFpsChecked(MODEL_STONE1 + rand() % 2, o->Position, o->Angle, o->Light);
                    }
                }
            }
            break;
        }
        break;
    case WD_9DEVILSQUARE:
        switch (o->Type)
        {
        case 2:
            Vector(1.f, 1.f, 1.f, Light);
            Vector(-15.f, 0.f, 0.f, p);
            if (rand_fps_check(4))
            {
                b->TransformPosition(BoneTransform[23], p, Position);
                CreateParticle(BITMAP_RAIN_CIRCLE + 1, Position, o->Angle, Light);
            }
            if (rand_fps_check(4))
            {
                b->TransformPosition(BoneTransform[31], p, Position);
                CreateParticle(BITMAP_RAIN_CIRCLE + 1, Position, o->Angle, Light);
            }
            Vector(-15.f, 0.f, 0.f, p);
            b->TransformPosition(BoneTransform[23], p, Position);
            CreateParticleFpsChecked(BITMAP_RAIN_CIRCLE + 1, Position, o->Angle, Light);
            break;
        }
        break;
    case WD_10HEAVEN:
        switch (o->Type)
        {
        case    0:
            if (o->HiddenMesh != -2)
            {
                vec3_t  Light;
                Vector(0.1f, 0.1f, 0.1f, Light);
                for (int i = 0; i < 20; ++i)
                {
                    CreateParticleFpsChecked(BITMAP_CLOUD, o->Position, o->Angle, Light, 0, o->Scale, o);
                }
            }
            o->HiddenMesh = -2;
            break;

        case    1:
            if (o->HiddenMesh != -2)
            {
                vec3_t  Light;
                Vector(0.1f, 0.1f, 0.1f, Light);
                for (int i = 0; i < 20; ++i)
                {
                    CreateParticleFpsChecked(BITMAP_CLOUD, o->Position, o->Angle, Light, 1, o->Scale, o);
                }
            }
            o->HiddenMesh = -2;
            break;

        case    2:
            if (o->HiddenMesh != -2)
            {
                vec3_t  Light;
                Vector(0.1f, 0.1f, 0.1f, Light);
                for (int i = 0; i < 20; ++i)
                {
                    CreateParticleFpsChecked(BITMAP_CLOUD, o->Position, o->Angle, Light, 2, o->Scale, o);
                }
            }
            o->HiddenMesh = -2;
            break;

        case    3:
            if (o->HiddenMesh != -2)
            {
                vec3_t  Light;
                Vector(0.1f, 0.1f, 0.1f, Light);
                for (int i = 0; i < 10; ++i)
                {
                    CreateParticleFpsChecked(BITMAP_CLOUD, o->Position, o->Angle, Light, 3, o->Scale, o);
                }
            }
            o->HiddenMesh = -2;
            break;

        case    4:
            if (o->HiddenMesh != -2)
            {
                vec3_t  Light;
                Vector(0.1f, 0.1f, 0.1f, Light);
                for (int i = 0; i < 10; ++i)
                {
                    CreateParticleFpsChecked(BITMAP_CLOUD, o->Position, o->Angle, Light, 4, o->Scale, o);
                }
            }
            o->HiddenMesh = -2;
            break;

        case    5:
            if (o->HiddenMesh != -2)
            {
                vec3_t  Light;
                Vector(0.1f, 0.1f, 0.1f, Light);
                for (int i = 0; i < 10; ++i)
                {
                    CreateParticleFpsChecked(BITMAP_CLOUD, o->Position, o->Angle, Light, 5, o->Scale, o);
                }
            }
            o->HiddenMesh = -2;
            break;

        case    10:
        {
            Vector(0.f, 0.f, 0.f, p);
            b->TransformPosition(BoneTransform[3], p, Position);

            Vector(1.f, 1.f, 1.f, Light);
            CreateParticleFpsChecked(BITMAP_LIGHT, Position, o->Angle, Light, 0, 1.f);
        }
        case    6:
        case    7:
        case    8:
        case    9:
        case    11:
        case    12:
        case    13:
        case    14:
        case    15:
            //            o->BlendMeshLight = sinf(WorldTime*0.002f)*1.f;
            //            o->BlendMesh = -2;
            //            o->HiddenMesh= -99;
            break;
        }
        break;

    case WD_11BLOODCASTLE1:
    case WD_11BLOODCASTLE1 + 1:
    case WD_11BLOODCASTLE1 + 2:
    case WD_11BLOODCASTLE1 + 3:
    case WD_11BLOODCASTLE1 + 4:
    case WD_11BLOODCASTLE1 + 5:
    case WD_11BLOODCASTLE1 + 6:
    case WD_52BLOODCASTLE_MASTER_LEVEL:
        switch (o->Type)
        {
        case 11:
        {
            wchar_t indexLight[7] = { 1, 2, 4, 6, 9, 10, 11 };

            Luminosity = sinf((o->Angle[2] * 20 + WorldTime) * 0.001f) * 0.5f + 0.5f;
            Vector(Luminosity * 1.f, Luminosity * 0.5f, 0.f, Light);

            for (int i = 0; i < 7; ++i)
            {
                Vector(0.f, 0.f, 2.f, p);
                b->TransformPosition(BoneTransform[indexLight[i]], p, Position);
                CreateSprite(BITMAP_LIGHT, Position, 0.5f, Light, o);
            }
        }
        break;
        case 13:
            Luminosity = sinf(WorldTime * 0.001f) * 0.3f + 0.7f;
            Vector(Luminosity, Luminosity, Luminosity, Light);
            Vector(0.f, 0.f, 0.f, p);
            b->TransformPosition(BoneTransform[3], p, Position);
            CreateSprite(BITMAP_FLARE, Position, Luminosity + 0.5f, Light, o);
            break;
        case 37:
            Vector(1.f, 1.f, 1.f, Light);
            if (rand_fps_check(2))
                if ((int)((o->Timer++) + 2) % 4 == 0)
                {
                    CreateParticle(BITMAP_ADV_SMOKE + 1, o->Position, o->Angle, Light);
                    CreateParticle(BITMAP_ADV_SMOKE, o->Position, o->Angle, Light, 0);
                }
            if (rand_fps_check(2))
                if ((int)(o->Timer++) % 4 == 0)
                {
                    CreateParticle(BITMAP_CLOUD, o->Position, o->Angle, Light, 6);
                    CreateParticle(BITMAP_ADV_SMOKE, o->Position, o->Angle, Light, 1);

                    Vector(1.f, 0.8f, 0.8f, Light);
                    CreateParticle(BITMAP_FLARE, o->Position, o->Angle, Light, 4, 0.19f, NULL);
                }
            break;
        }
        break;
    case WD_55LOGINSCENE:
    {
        switch (o->Type)
        {
        case 84:
            if (rand_fps_check(5))
                CreateParticle(BITMAP_CLOUD, o->Position, o->Angle, Light, 10, o->Scale, o);
            break;
        case 90:
        case 86:
            break;
        case 89:
        {
            int iTimeCheck = (int)WorldTime % 8000;

            if (iTimeCheck >= 7950)
            {
                Vector(o->Position[0] + (rand() % 2048 - 1024) * FPS_ANIMATION_FACTOR,
                    o->Position[1] + (rand() % 2048 - 1024) * FPS_ANIMATION_FACTOR,
                    o->Position[2] + (3000 + rand() % 600) * FPS_ANIMATION_FACTOR,
                    Position);
                CreateEffectFpsChecked(MODEL_FIRE, Position, o->Angle, o->Light, 9);
            }
        }
        break;
        }
    }
    default:
        if (RenderChaosCastleVisual(o, b))							return;
        if (RenderHellasVisual(o, b))								return;
        if (battleCastle::RenderBattleCastleVisual(o, b))		return;
        if (M31HuntingGround::RenderHuntingGroundObjectVisual(o, b))	return;
        if (M34CryingWolf2nd::RenderCryingWolf2ndObjectVisual(o, b))	return;
        if (M33Aida::RenderAidaObjectVisual(o, b))						return;
        if (M34CryWolf1st::RenderCryWolf1stObjectVisual(o, b))			return;
        if (M37Kanturu1st::RenderKanturu1stObjectVisual(o, b))			return;
        if (M38Kanturu2nd::Render_Kanturu2nd_ObjectVisual(o, b))			return;
        if (M39Kanturu3rd::RenderKanturu3rdObjectVisual(o, b))			return;
        if (SEASON3A::CGM3rdChangeUp::Instance().RenderObjectVisual(o, b)) return;
        if (g_CursedTemple->RenderObjectVisual(o, b))				return;
        if (SEASON3B::GMNewTown::RenderObjectVisual(o, b))			return;
        if (SEASON3C::GMSwampOfQuiet::RenderObjectVisual(o, b))		return;
        if (TheMapProcess().RenderObjectVisual(o, b) == true)		return;
    }
}

void RenderObjects()
{
    float   range = 0.f;
    if (gMapManager.WorldActive == WD_10HEAVEN)
    {
        range = -10.f;
    }

    if (Time_Effect > 40)
        Time_Effect = 0;
    Time_Effect += FPS_ANIMATION_FACTOR;

    for (int i = 0; i < 16; i++)
    {
        for (int j = 0; j < 16; j++)
        {
            OBJECT_BLOCK* ob = &ObjectBlock[i * 16 + j];
            ob->Visible = TestFrustrum2D((float)(i * 16 + 8), (float)(j * 16 + 8), -180.f);
            if (g_Direction.m_CKanturu.IsMayaScene()
                || gMapManager.WorldActive == WD_51HOME_6TH_CHAR
                || gMapManager.WorldActive == WD_73NEW_LOGIN_SCENE
                || gMapManager.WorldActive == WD_74NEW_CHARACTER_SCENE
                || IsIceCity()
                || gMapManager.IsPKField()
                || IsDoppelGanger2()
                )
            {
                OBJECT* o = ob->Head;

                while (1)
                {
                    if (o != NULL)
                    {
                        if (g_Direction.m_CKanturu.IsMayaScene() && (o->Type == 0 || o->Type == MODEL_STORM2))
                        {
                            RenderObject(o);
                            RenderObjectVisual(o);
                        }
                        else
                            if (gMapManager.WorldActive == WD_73NEW_LOGIN_SCENE)
                            {
                                float fDistance_x = CameraPosition[0] - o->Position[0];
                                float fDistance_y = CameraPosition[1] - o->Position[1];
                                float fDistance = sqrtf(fDistance_x * fDistance_x + fDistance_y * fDistance_y);
                                float fDis = 2000.0f;

                                if (((o->Type >= 122 && o->Type <= 124) || (o->Type == 159) || (o->Type == 126) || (o->Type == 129) || (o->Type == 127)) &&
                                    TestFrustrum2D(o->Position[0] * 0.01f, o->Position[1] * 0.01f, -500.f) && fDistance < fDis * 2.0f)
                                {
                                    if (o->BlendMeshLight < 1.0f)
                                        o->BlendMeshLight += 0.03f;
                                    else
                                        o->BlendMeshLight = 1.0f;

                                    if (o->AlphaTarget < 1.0f)
                                        o->AlphaTarget += 0.03f;
                                    else
                                        o->AlphaTarget = 1.0f;

                                    RenderObject(o);
                                    RenderObjectVisual(o);
                                }
                                else if (TestFrustrum2D(o->Position[0] * 0.01f, o->Position[1] * 0.01f, -500.f) && fDistance < fDis && o->Type <= MAX_WORLD_OBJECTS)
                                {
                                    if (o->AlphaTarget < 1.0f)
                                        o->AlphaTarget += 0.03f;
                                    else
                                        o->AlphaTarget = 1.0f;

                                    RenderObject(o);
                                    RenderObjectVisual(o);
                                }
                                else if (o->AlphaTarget != 0 && fDistance > fDis)
                                {
                                    o->BlendMeshLight = 0.0f;
                                    o->Alpha = 0.0f;
                                    o->AlphaTarget = 0.0f;
                                }
                            }

                            else if ((gMapManager.WorldActive == WD_51HOME_6TH_CHAR
                                ) && ((o->Type >= 5 && o->Type <= 14) || (o->Type >= 87 && o->Type <= 88) || (o->Type == 4 || o->Type == 129)) && TestFrustrum2D(o->Position[0] * 0.01f, o->Position[1] * 0.01f, -400.f))
                            {
                                RenderObject(o);
                                RenderObjectVisual(o);
                            }
                            else if ((gMapManager.WorldActive == WD_74NEW_CHARACTER_SCENE) && (o->Type == 129 || o->Type == 98))
                            {
                                RenderObject(o);
                                RenderObjectVisual(o);
                            }
                            else if ((gMapManager.WorldActive == WD_57ICECITY || gMapManager.WorldActive == WD_58ICECITY_BOSS) && (o->Type == 30 || o->Type == 31) && TestFrustrum2D(o->Position[0] * 0.01f, o->Position[1] * 0.01f, -600.f))
                            {
                                RenderObject(o);
                                RenderObjectVisual(o);
                            }
                            else if (gMapManager.IsPKField() && (o->Type == 16 || o->Type == 67 || o->Type == 68) && TestFrustrum2D(o->Position[0] * 0.01f, o->Position[1] * 0.01f, -600.f))
                            {
                                RenderObject(o);
                                RenderObjectVisual(o);
                            }
                            else if (IsDoppelGanger2() && (o->Type == 16 || o->Type == 67 || o->Type == 68) && TestFrustrum2D(o->Position[0] * 0.01f, o->Position[1] * 0.01f, -600.f))
                            {
                                RenderObject(o);
                                RenderObjectVisual(o);
                            }

                        if (o->Next == NULL)
                            break;
                        else
                            o = o->Next;
                    }
                    else
                        break;
                }
            }

            if (ob->Visible || CameraTopViewEnable)
            {
                OBJECT* o = ob->Head;
                while (1)
                {
                    if (o != NULL)
                    {
                        if (o->Live)
                        {
                            o->Visible = TestFrustrum2D(o->Position[0] * 0.01f, o->Position[1] * 0.01f, o->CollisionRange + range);
                            if ((gMapManager.WorldActive == WD_51HOME_6TH_CHAR
                                ) &&
                                ((o->Type >= 5 && o->Type <= 14) || (o->Type >= 87 && o->Type <= 88) || (o->Type == 4 || o->Type == 129)));
                            else
                                if ((gMapManager.WorldActive == WD_57ICECITY || gMapManager.WorldActive == WD_58ICECITY_BOSS) && (o->Type == 30 || o->Type == 31 || o->Type == 76));
                                else
                                    if ((gMapManager.IsPKField() || IsDoppelGanger2()) && (o->Type == 16 || o->Type == 67 || o->Type == 68));
                                    else
                                        if (gMapManager.WorldActive == WD_73NEW_LOGIN_SCENE);
                                        else if ((gMapManager.WorldActive == WD_74NEW_CHARACTER_SCENE) && (o->Type == 129 || o->Type == 98));

                                        else
                                            if (o->Visible || CameraTopViewEnable)
                                            {
                                                bool Success = false;
                                                if (gMapManager.WorldActive == WD_2DEVIAS && o->Type == 100)
                                                {
                                                    int Level;
                                                    if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_DARK
                                                        || gCharacterManager.GetBaseClass(Hero->Class) == CLASS_DARK_LORD
                                                        || gCharacterManager.GetBaseClass(Hero->Class) == CLASS_RAGEFIGHTER)
                                                        Level = 50 * 2 / 3;
                                                    else
                                                        Level = 50;
                                                    if (CharacterAttribute->Level >= Level)
                                                        Success = true;
                                                }
                                                else
                                                    Success = true;
                                                if (Success)
                                                {
                                                    RenderObject(o);
                                                    RenderObjectVisual(o);
                                                }
                                            }
#ifdef CSK_DEBUG_RENDER_BOUNDINGBOX
                            if (o->Visible == true && g_bRenderBoundingBox == true)
                            {
                                RenderBoundingBox(o);
                            }
#endif // CSK_DEBUG_RENDER_BOUNDINGBOX
                        }

                        if (o->Next == NULL) break;
                        o = o->Next;
                    }
                    else break;
                }
            }
        }
    }
}

void RenderObject_AfterCharacter(OBJECT* o, bool Translate, int Select, int ExtraMon)
{
    if (Calc_RenderObject(o, Translate, Select, ExtraMon) == false)
    {
        return;
    }
    Draw_RenderObject_AfterCharacter(o, Translate, Select, ExtraMon);
}

void Draw_RenderObject_AfterCharacter(OBJECT* o, bool Translate, int Select, int ExtraMon)
{
    BMD* b = &Models[o->Type];

    if ((EditFlag != EDIT_NONE) || (EditFlag == EDIT_NONE && o->HiddenMesh != -2))
    {
        M37Kanturu1st::RenderKanturu1stAfterObjectMesh(o, b);
        M38Kanturu2nd::Render_Kanturu2nd_AfterObjectMesh(o, b);
        M39Kanturu3rd::RenderKanturu3rdAfterObjectMesh(o, b);
        SEASON3A::CGM3rdChangeUp::Instance().RenderAfterObjectMesh(o, b);
        g_CursedTemple->RenderObject_AfterCharacter(o, b);
        SEASON3B::GMNewTown::RenderObjectAfterCharacter(o, b);
        SEASON3C::GMSwampOfQuiet::RenderObjectAfterCharacter(o, b);

        TheMapProcess().RenderAfterObjectMesh(o, b);
    }
}

void RenderObjects_AfterCharacter()
{
    if (!(gMapManager.WorldActive == WD_37KANTURU_1ST || gMapManager.WorldActive == WD_38KANTURU_2ND || gMapManager.WorldActive == WD_39KANTURU_3RD
        || gMapManager.WorldActive == WD_40AREA_FOR_GM
        || gMapManager.WorldActive == WD_41CHANGEUP3RD_1ST
        || gMapManager.WorldActive == WD_42CHANGEUP3RD_2ND
        || gMapManager.IsCursedTemple()

        || gMapManager.WorldActive == WD_51HOME_6TH_CHAR
        || gMapManager.WorldActive == WD_56MAP_SWAMP_OF_QUIET
        || IsIceCity()
        || gMapManager.IsPKField()
        || IsDoppelGanger1()
        || IsDoppelGanger2()
        || IsDoppelGanger3()
        || IsDoppelGanger4()
        || IsUnitedMarketPlace()
#ifdef ASG_ADD_MAP_KARUTAN
        || IsKarutanMap()
#endif	// ASG_ADD_MAP_KARUTAN
        ))
        return;

    float   range = 0.f;
    if (gMapManager.WorldActive == WD_10HEAVEN)
    {
        range = -10.f;
    }

    if (Time_Effect > 40)
        Time_Effect = 0;
    Time_Effect += FPS_ANIMATION_FACTOR;

    for (int i = 0; i < 16; i++)
    {
        for (int j = 0; j < 16; j++)
        {
            OBJECT_BLOCK* ob = &ObjectBlock[i * 16 + j];
            ob->Visible = TestFrustrum2D((float)(i * 16 + 8), (float)(j * 16 + 8), -180.f);

            if (g_Direction.m_CKanturu.IsMayaScene())
            {
                OBJECT* o = ob->Head;

                while (1)
                {
                    if (o != NULL)
                    {
                        if (o->Type == MODEL_STORM3)
                        {
                            RenderObject_AfterCharacter(o);
                        }

                        if (o->Next == NULL)
                            break;
                        else
                            o = o->Next;
                    }
                    else
                        break;
                }
            }

            if (gMapManager.WorldActive == WD_51HOME_6TH_CHAR
                || IsIceCity()
                || gMapManager.IsPKField()
                || IsDoppelGanger2()
                )
            {
                OBJECT* o = ob->Head;

                while (1)
                {
                    if (o != NULL)
                    {
                        if (gMapManager.WorldActive == WD_51HOME_6TH_CHAR &&
                            (o->Type == 89)
                            && TestFrustrum2D(o->Position[0] * 0.01f, o->Position[1] * 0.01f, -400.f))
                        {
                            RenderObject_AfterCharacter(o);
                        }
                        else if ((gMapManager.WorldActive == WD_57ICECITY || gMapManager.WorldActive == WD_58ICECITY_BOSS) && o->Type == 76 && TestFrustrum2D(o->Position[0] * 0.01f, o->Position[1] * 0.01f, -600.f))
                        {
                            RenderObject_AfterCharacter(o);
                        }
                        else if (gMapManager.IsPKField() && (o->Type == 16 || o->Type == 67 || o->Type == 68) && TestFrustrum2D(o->Position[0] * 0.01f, o->Position[1] * 0.01f, -600.f))
                        {
                            RenderObject_AfterCharacter(o);
                        }
                        else if (IsDoppelGanger2() && (o->Type == 16 || o->Type == 67 || o->Type == 68) && TestFrustrum2D(o->Position[0] * 0.01f, o->Position[1] * 0.01f, -600.f))
                        {
                            RenderObject_AfterCharacter(o);
                        }
                        if (o->Next == NULL)
                            break;
                        else
                            o = o->Next;
                    }
                    else
                        break;
                }
            }

            if (ob->Visible || CameraTopViewEnable)
            {
                OBJECT* o = ob->Head;
                while (1)
                {
                    if (o != NULL)
                    {
                        if (o->Live && o->m_bRenderAfterCharacter == true)
                        {
                            o->Visible = TestFrustrum2D(o->Position[0] * 0.01f, o->Position[1] * 0.01f, o->CollisionRange + range);
                            if ((gMapManager.WorldActive == WD_51HOME_6TH_CHAR
                                ) && (o->Type == 89));
                            else
                                if (IsIceCity() && o->Type == 76);
                                else
                                    if (gMapManager.IsPKField() && (o->Type == 16 || o->Type == 67 || o->Type == 68));
                                    else
                                        if (IsDoppelGanger2() && (o->Type == 16 || o->Type == 67 || o->Type == 68));
                                        else
                                            if (o->Visible || CameraTopViewEnable)
                                            {
                                                bool Success = false;
                                                if (gMapManager.WorldActive == WD_2DEVIAS && o->Type == 100)
                                                {
                                                    int Level;

                                                    if (gCharacterManager.GetBaseClass(Hero->Class) == CLASS_DARK
                                                        || gCharacterManager.GetBaseClass(Hero->Class) == CLASS_DARK_LORD
                                                        || gCharacterManager.GetBaseClass(Hero->Class) == CLASS_RAGEFIGHTER)
                                                        Level = 80 * 2 / 3;
                                                    else
                                                        Level = 80;
                                                    if (CharacterAttribute->Level >= Level)
                                                        Success = true;
                                                }
                                                else
                                                    Success = true;
                                                if (Success)
                                                {
                                                    RenderObject_AfterCharacter(o);
                                                }
                                            }
                        }
                        if (o->Next == NULL) break;
                        o = o->Next;
                    }
                    else break;
                }
            }
        }
    }
}

extern int CameraWalkCut;
extern int CurrentCameraCount;

void MoveObject(OBJECT* o)
{
    if (gMapManager.WorldActive == 9)
    {
        if ((int)WorldTime % 4000 < 1000)
            if (rand_fps_check(100))
            {
                float Luminosity = (float)(rand() % 12 + 4) * 0.1f;
                vec3_t Light;
                Vector(Luminosity * 0.2f, Luminosity * 0.3f, Luminosity * 0.5f, Light);
                AddTerrainLight(Hero->Object.Position[0] + rand() % 1200 - 600,
                    Hero->Object.Position[1] + rand() % 1200 - 600,
                    Light, 12, PrimaryTerrainLight);
                //PlayBuffer(SOUND_THUNDER01);
            }
        PlayBuffer(SOUND_RAIN01, NULL, true);
    }
    if (gMapManager.WorldActive == 0)
    {
        if (o->Type == MODEL_HOUSE_WALL01 + 4 || o->Type == MODEL_HOUSE_WALL01 + 5)
        {
            if (HeroTile == 4)
                o->AlphaTarget = 0.f;
            else
                o->AlphaTarget = 1.f;
        }
#ifdef _PVP_MURDERER_HERO_ITEM
        else if (o->Type == MODEL_MURDERER_DOG)
        {
            if (o->PriorAnimationFrame > o->AnimationFrame)
            {
                if (rand_fps_check(10) && o->CurrentAction != MONSTER01_STOP2)
                    o->CurrentAction = MONSTER01_STOP2;
                else
                    o->CurrentAction = MONSTER01_STOP1;
            }
        }
#endif	// _PVP_MURDERER_HERO_ITEM
    }
    if (gMapManager.WorldActive == 2)
    {
        if (o->Type == 81 || o->Type == 82 || o->Type == 96 || o->Type == 98 || o->Type == 99)
        {
            if (HeroTile == 3 || HeroTile >= 10)
                o->AlphaTarget = 0.f;
            else
                o->AlphaTarget = 1.f;
        }
    }
    Alpha(o);
    if (o->Alpha < 0.01f) return;
    BMD* b = &Models[o->Type];
    b->CurrentAction = o->CurrentAction;

    float fSpeed = o->Velocity;
    if (gMapManager.WorldActive == WD_8TARKAN)
    {
        switch (o->Type)
        {
        case 8:
            fSpeed *= pow(4.0f, FPS_ANIMATION_FACTOR);
            break;
        }
    }
    else if (IsIceCity() && (o->Type == 16 || o->Type == 17 || o->Type == 68))
    {
        fSpeed = b->Actions[o->CurrentAction].PlaySpeed;
    }
#ifdef ASG_ADD_MAP_KARUTAN
    else if (IsKarutanMap() && (o->Type == 66 || o->Type == 107))
    {
        fSpeed = b->Actions[o->CurrentAction].PlaySpeed;
    }
#endif	// ASG_ADD_MAP_KARUTAN

    b->PlayAnimation(&o->AnimationFrame, &o->PriorAnimationFrame, &o->PriorAction, fSpeed, o->Position, o->Angle);

    vec3_t p;
    vec3_t Light;
    float Luminosity;
    Vector(0.f, 0.f, 0.f, p);
    if (SceneFlag == LOG_IN_SCENE || SceneFlag == CHARACTER_SCENE)
    {
        switch (o->Type)
        {
        case MODEL_LOGO:
            o->BlendMeshTexCoordV = -((int)WorldTime % 4000 * 0.00025f);
            break;
        case MODEL_WAVEBYSHIP:
            o->BlendMeshTexCoordV = -((int)WorldTime % 4000 * 0.00025f);
            break;
        case MODEL_MUGAME:
            if (CameraWalkCut == 0)
                Luminosity = CurrentCameraCount * 0.02f;
            else
            {
                Luminosity = 1.5f;
                /*if(CameraWalkCut==1 && CurrentCameraCount==0)
                {
                for(int i=0;i<100;i++)
                {
                Vector((float)(rand()%200-100),(float)(rand()%40-20),(float)(rand()%200-100),Position);
                VectorAdd(Position,o->Position,Position);
                Vector(1.f,1.f,1.f,Light);
                CreateParticle(BITMAP_SPARK,Position,o->Angle,Light);
                }
            }*/
            }
            Vector(Luminosity * 1.f, Luminosity * 1.f, Luminosity * 1.f, o->Light);
            o->BlendMeshLight = Luminosity;
            break;
        }
    }

    switch (gMapManager.WorldActive)
    {
    case WD_0LORENCIA:
        switch (o->Type)
        {
        case MODEL_HOUSE01 + 3:
        case MODEL_HOUSE01 + 4:
            o->BlendMeshTexCoordV = -(int)WorldTime % 1000 * 0.001f;
            break;
        case MODEL_HOUSE01 + 2:
        case MODEL_HOUSE_WALL01 + 1:
            o->BlendMeshLight = (float)(rand() % 4 + 4) * 0.1f;
            break;
        case MODEL_LIGHT01:
            CreateFire(0, o, 0.f, 0.f, 0.f);
            o->HiddenMesh = -2;
            break;
        case MODEL_LIGHT01 + 1:
            CreateFire(1, o, 0.f, 0.f, 0.f);
            o->HiddenMesh = -2;
            break;
        case MODEL_LIGHT01 + 2:
            CreateFire(2, o, 0.f, 0.f, 0.f);
            o->HiddenMesh = -2;
            break;
        case MODEL_BRIDGE:
            CreateFire(0, o, 90.f, -200.f, 30.f);
            CreateFire(0, o, 90.f, 200.f, 30.f);
            break;
        case MODEL_DUNGEON_GATE:
            CreateFire(0, o, -150.f, -150.f, 140.f);
            CreateFire(0, o, 150.f, -150.f, 140.f);
            break;
        case MODEL_FIRE_LIGHT01:
            CreateFire(0, o, 0.f, 0.f, 200.f);
            break;
        case MODEL_FIRE_LIGHT01 + 1:
            CreateFire(0, o, 0.f, -30.f, 60.f);
            break;
        case MODEL_STREET_LIGHT:
            Luminosity = (float)(rand() % 2 + 6) * 0.1f;
            Vector(Luminosity, Luminosity * 0.8f, Luminosity * 0.6f, Light);
            AddTerrainLight(o->Position[0], o->Position[1], Light, 3, PrimaryTerrainLight);
            break;
        case MODEL_BONFIRE:
            CreateFire(0, o, 0.f, 0.f, 60.f);
            o->BlendMeshLight = (float)(rand() % 6 + 4) * 0.1f;
            //CreateBonfire(o->Position,o->Angle);
            break;
            /*	case MODEL_FIREPLACE01:
            CreateFire(0,o,0.f,0.f,60.f);
            //CreateBonfire(o->Position,o->Angle);
            break;
            case MODEL_DOOR_WOOD01:
            case MODEL_DOOR_STEEL01:
            {
            vec3_t Range;
            VectorSubtract(Hero->Object.Position,o->Position,Range);
            float Distance = Range[0]*Range[0]+Range[1]*Range[1];
            if(Distance <= 120.f*120.f)
            {
            o->Direction[2] = (120.f+(Hero->Object.Position[1]-o->Position[1]))*0.6f;
            }
            o->Angle[2] = TurnAngle2(o->Angle[2],o->Direction[2],FarAngle(o->Angle[2],o->Direction[2])*0.1f);
            if(FarAngle(o->Angle[2],o->Direction[2]) < absf(o->Direction[2])*0.2f)
            o->Direction[2] = -o->Direction[2] * 0.2f;
            break;
        }*/
        case MODEL_CANDLE:
            Luminosity = (float)(rand() % 4 + 3) * 0.1f;
            Vector(Luminosity, Luminosity * 0.6f, Luminosity * 0.2f, Light);
            AddTerrainLight(o->Position[0], o->Position[1], Light, 3, PrimaryTerrainLight);
            break;
            /*case MODEL_GOLD01:
            Vector(0.f,-15.f,25.f,p);
            CreateShiny(o,p);
            break;*/
        case MODEL_SIGN01 + 1:
            Vector(50.f, -10.f, 120.f, p);
            //CreateShiny(o,p);
            break;
            /*case MODEL_STONE_WALL01+3:
            o->Live = false;
            break;*/
        }
        break;
    case WD_1DUNGEON:
        switch (o->Type)
        {
        case 52:
            if (rand_fps_check(3))
                CreateEffect(MODEL_DUNGEON_STONE01, o->Position, o->Angle, o->Light);
            //CreateEffect(MODEL_STONE1,o->Position,o->Angle,o->Light);
            o->HiddenMesh = -2;
            break;
        case 22:
        case 23:
        case 24:
            Models[o->Type].StreamMesh = 1;
            o->BlendMeshTexCoordV = -(float)((int)WorldTime % 1000) * 0.001f;
            break;
        case 41:
            CreateFire(0, o, 0.f, -30.f, 240.f);
            break;
        case 42:
            CreateFire(0, o, 0.f, 0.f, 190.f);
            break;
        case 39:
        case 40:
        case 51:
            o->HiddenMesh = -2;
            break;
        }
        break;
    case WD_2DEVIAS:
        switch (o->Type)
        {
        case 78:
            o->BlendMeshLight = (float)(rand() % 4 + 4) * 0.1f;
            break;
        case 30:
        {
            vec3_t Position;
            Position[0] = o->Position[0];
            Position[1] = o->Position[1];
            Position[2] = o->Position[2] + 160.f;
            CreateParticleFpsChecked(BITMAP_TRUE_FIRE, Position, o->Angle,
                o->Light, 0, o->Scale);
            CreateParticleFpsChecked(BITMAP_SMOKE, Position, o->Angle,
                o->Light, 21, 0.5f + ((rand() % 9) * 0.1f));
        }
        break;
        case 66:
            CreateFire(0, o, 0.f, 0.f, 50.f);
            break;
        case 86:
        case 20:
        case 65:
        case 88:
            if (EditFlag == EDIT_NONE)
            {
                float dx = Hero->Object.Position[0] - o->HeadTargetAngle[0];
                float dy = Hero->Object.Position[1] - o->HeadTargetAngle[1];
                float Distance = sqrtf(dx * dx + dy * dy);
                if (Distance < 200.f)
                {
                    if (o->Type == 86)
                    {
                        if (o->Angle[2] == 90.f)
                            o->Position[1] = o->HeadTargetAngle[1] + (200.f - Distance) * 2.f;
                        if (o->Angle[2] == 270.f)
                            o->Position[1] = o->HeadTargetAngle[1] - (200.f - Distance) * 2.f;
                        if (o->Angle[2] == 0.f)
                            o->Position[0] = o->HeadTargetAngle[0] + (200.f - Distance) * 2.f;
                        if (o->Angle[2] == 180.f)
                            o->Position[0] = o->HeadTargetAngle[0] - (200.f - Distance) * 2.f;
                        PlayBuffer(SOUND_DOOR02);
                    }
                    else
                    {
                        if (o->HeadAngle[2] == 90.f)
                            o->Angle[2] = 30.f - (200.f - Distance) * 0.5f;
                        if (o->HeadAngle[2] == 270.f)
                            o->Angle[2] = 330.f + (200.f - Distance) * 0.5f;
                        if (o->HeadAngle[2] == 0.f)
                            o->Angle[2] = 300.f - (200.f - Distance) * 0.5f;
                        if (o->HeadAngle[2] == 180.f)
                            o->Angle[2] = 240.f + (200.f - Distance) * 0.5f;
                        PlayBuffer(SOUND_DOOR01);
                    }
                }
                else
                {
                    o->Angle[2] = TurnAngle2(o->Angle[2], o->HeadAngle[2], 10.f);
                    o->Position[0] += (o->HeadTargetAngle[0] - o->Position[0]) * 0.2f;
                    o->Position[1] += (o->HeadTargetAngle[1] - o->Position[1]) * 0.2f;
                }
            }
            break;
        }
        break;
    case WD_3NORIA:
        switch (o->Type)
        {
        case MODEL_WARP:
            vec3_t Position;
            Vector(o->Position[0], o->Position[1] - 50.f, o->Position[2] + 350.f, Position);
            Vector(0.5f, 0.5f, 0.5f, Light);
            CreateParticleFpsChecked(BITMAP_SPARK + 1, Position, o->Angle, Light, 9, 1.4f);
            break;
        case 18:
            o->BlendMeshTexCoordV = (int)WorldTime % 1000 * 0.001f;
            break;
        case 39:
            o->BlendMesh = 1;
            break;
        case 41:
            o->BlendMesh = 0;
            o->BlendMeshTexCoordV = (int)WorldTime % 2000 * 0.0005f;
            break;
        case 42:
            Models[o->Type].StreamMesh = 0;
            o->BlendMeshTexCoordU = -(float)((int)WorldTime % 500) * 0.002f;
            break;
        case 43:
            Models[o->Type].StreamMesh = 0;
            o->BlendMeshTexCoordU = (float)((int)WorldTime % 500) * 0.002f;
            break;
        }
        break;
    case WD_4LOSTTOWER:
        switch (o->Type)
        {
        case 38:
        case 39:
            CheckSkull(o);
            break;
        case 3:
        case 4:
            o->BlendMeshTexCoordU = -(int)WorldTime % 1000 * 0.001f;
            break;
        case 19:
        case 20:
            o->BlendMesh = 4;
            o->BlendMeshTexCoordU = -(int)WorldTime % 1000 * 0.001f;
            break;
        case 18:
            o->BlendMesh = 1;
            break;
        case 23:
            o->BlendMesh = 1;
            //b->TransformPosition(BoneTransform[1],p,Position);
            //CreateSprite(BITMAP_LIGHT,Position,2.f,Light,o);
            break;
        case 24:
            o->HiddenMesh = -2;
            if (rand_fps_check(64))
                CreateEffect(BITMAP_FLAME, o->Position, o->Angle, o->Light);
            //o->BlendMeshTexCoordV = (int)WorldTime%1000 * 0.001f;
            break;
        case 25:
            o->HiddenMesh = -2;
            break;
        }
        break;
    case WD_5UNKNOWN:
        switch (o->Type)
        {
        case 2:
            o->BlendMesh = 0;
            break;
        case 3:
            o->BlendMesh = 0;
            o->BlendMeshLight = (float)(rand() % 4 + 6) * 0.1f;
            break;
        }
        break;
    case WD_6STADIUM:
        switch (o->Type)
        {
        case 21:
            o->BlendMesh = 3;
            o->BlendMeshTexCoordV = -(int)WorldTime % 1000 * 0.001f;
            break;
        case 38:
            o->HiddenMesh = -2;
            break;
        }
        break;
    case WD_7ATLANSE:
        switch (o->Type)
        {
        case 22:
            o->HiddenMesh = -2;
            o->Timer += 0.1f * FPS_ANIMATION_FACTOR;
            if (o->Timer > 10.f)
                o->Timer = 0.f;
            if (o->Timer > 5.f)
                CreateParticleFpsChecked(BITMAP_BUBBLE, o->Position, o->Angle, o->Light);
            break;
        case 23:
            o->BlendMesh = 0;
            //o->BlendMeshLight = o->Light[1]+1.f;
            o->BlendMeshLight = sinf(WorldTime * 0.002f) * 0.3f + 0.5f;
            break;
        case 32:
        case 34:
            o->BlendMesh = 1;
            o->BlendMeshLight = sinf(WorldTime * 0.004f) * 0.5f + 0.5f;
            break;
        case 38:
            o->BlendMesh = 0;
            //o->BlendMeshLight = sinf(WorldTime*0.004f)*0.3f+0.7f;
            break;
        case 40:
            o->BlendMesh = 0;
            o->BlendMeshLight = sinf(WorldTime * 0.004f) * 0.3f + 0.5f;
            o->Velocity = 0.05f;
            break;
        }
        break;
    case WD_8TARKAN:
        switch (o->Type)
        {
        case 2:
            o->BlendMesh = 0;
            o->BlendMeshTexCoordU = -(int)WorldTime % 1000 * 0.001f;
            break;

        case 4:
        {
            float   sine = (float)sinf(WorldTime * 0.002f) * 0.35f + 0.65f;
            o->BlendMesh = 0;
            o->BlendMeshLight = sine;
            o->BlendMeshTexCoordV = -(int)WorldTime % 10000 * 0.0001f;

            Luminosity = sine;
            Vector(Luminosity, Luminosity, Luminosity, Light);
            AddTerrainLight(o->Position[0], o->Position[1], Light, 3, PrimaryTerrainLight);
        }
        break;

        case 7:
        {
            float   sine = (float)sinf((WorldTime + (o->Angle[2] * 100)) * 0.002f) * 0.35f + 0.65f;

            o->BlendMesh = 0;
            o->BlendMeshLight = sine;

            Luminosity = sine;
            Vector(Luminosity, Luminosity * 0.6f, Luminosity * 0.2f, Light);
            AddTerrainLight(o->Position[0], o->Position[1], Light, 3, PrimaryTerrainLight);
        }
        break;

        case 11:
            o->BlendMeshTexCoordV = -(int)WorldTime % 10000 * 0.0002f;
            break;

        case 12:
            o->BlendMeshTexCoordU = -(int)WorldTime % 50000 * 0.00005f;
            o->BlendMeshTexCoordV = -(int)WorldTime % 50000 * 0.00005f;
            break;

        case 13:
            o->BlendMeshTexCoordV = -(int)WorldTime % 10000 * 0.0002f;
            break;

        case 61:
            o->BlendMesh = 1;
            o->BlendMeshTexCoordV = -(int)WorldTime % 1000 * 0.001f;

            Luminosity = (float)sinf(WorldTime * 0.002f) * 0.35f + 0.65f;
            Vector(Luminosity, Luminosity * 0.6f, Luminosity * 0.2f, Light);
            AddTerrainLight(o->Position[0], o->Position[1], Light, 2, PrimaryTerrainLight);
            break;

        case 63:
        case 64:
            o->HiddenMesh = -2;
            break;

        case 65:
        case 66:
            o->BlendMesh = 1;
            o->BlendMeshTexCoordV = -(int)WorldTime % 1000 * 0.001f;

            Luminosity = (float)sinf(WorldTime * 0.002f) * 0.35f + 0.65f;
            Vector(Luminosity, Luminosity * 0.6f, Luminosity * 0.2f, Light);
            AddTerrainLight(o->Position[0], o->Position[1], Light, 2, PrimaryTerrainLight);
            break;

        case 72:
            o->BlendMesh = 0;
            o->BlendMeshTexCoordV = -(int)WorldTime % 10000 * 0.0002f;
            break;

        case 73:
            o->BlendMeshTexCoordV = -(int)WorldTime % 10000 * 0.0002f;
            break;

        case 75:
            o->BlendMeshTexCoordV = -(int)WorldTime % 10000 * 0.0002f;
            break;

        case 79:
            o->BlendMeshTexCoordV = -(int)WorldTime % 10000 * 0.0002f;
            break;

        case 82:
            o->BlendMesh = 0;
            Vector(1.0f, 1.0f, 1.0f, o->Light);
            break;
        }
        break;

    case WD_10HEAVEN:
        break;

    case WD_11BLOODCASTLE1:
    case WD_11BLOODCASTLE1 + 1:
    case WD_11BLOODCASTLE1 + 2:
    case WD_11BLOODCASTLE1 + 3:
    case WD_11BLOODCASTLE1 + 4:
    case WD_11BLOODCASTLE1 + 5:
    case WD_11BLOODCASTLE1 + 6:
    case WD_52BLOODCASTLE_MASTER_LEVEL:
        switch (o->Type)
        {
        case 9:
        case 10:
            if (o->PKKey != 4)
                o->HiddenMesh = -2;
            break;
        }
        break;
    case WD_55LOGINSCENE:
    {
        switch (o->Type)
        {
        case 84:
        case 85:
        case 87:
        case 89:
            o->HiddenMesh = -2;
            break;
        case 90:
        case 86:
            if (o->Type == 90)
            {
                o->Alpha = 1.0f;
                Vector(1.0f, 1.0f, 1.0f, o->Light);
                o->Angle[2] -= 0.23f;
            }
            else
            {
                o->Alpha = 0.2f;
                o->Angle[2] -= 0.6f;
                Vector(1.0f, 0.0f, 0.0f, o->Light);
            }

            float fTemp1 = sinf(WorldTime * 0.001f) * 0.2 + 0.6f;
            o->BlendMesh = -2;
            break;
        }
        break;
    }
    default:
        if (MoveHellasVisual(o) == true) return;
        if (battleCastle::MoveBattleCastleVisual(o) == true) return;
        if (M31HuntingGround::MoveHuntingGroundObject(o)) return;
        if (M34CryingWolf2nd::MoveCryingWolf2ndObject(o)) return;
        if (M33Aida::MoveAidaObject(o)) return;
        if (M34CryWolf1st::MoveCryWolf1stObject(o)) return;
        if (M37Kanturu1st::MoveKanturu1stObject(o))	return;
        if (M38Kanturu2nd::Move_Kanturu2nd_Object(o)) return;
        if (M39Kanturu3rd::MoveKanturu3rdObject(o)) return;
        if (SEASON3A::CGM3rdChangeUp::Instance().MoveObject(o)) return;
        if (g_CursedTemple->MoveObject(o)) return;
        if (SEASON3B::GMNewTown::MoveObject(o)) return;
        if (SEASON3C::GMSwampOfQuiet::MoveObject(o)) return;
        if (TheMapProcess().MoveObject(o)) return;
        break;
    }
}

static  int visibleObject = 0;

int MoveHeavenThunder(void)
{
    int     objectCount = 0;
    float   Luminosity;
    vec3_t  Light;

    if (rand_fps_check(50))
    {
        vec3_t  position, Angle;

        position[0] = Hero->Object.Position[0] + rand() % 300 - 150.f;
        position[1] = Hero->Object.Position[1];
        position[2] = Hero->Object.Position[2] + rand() % 300 - 150.f;

        Luminosity = (float)(rand() % 4 + 4) * 0.05f;
        Vector(Luminosity * 0.3f, Luminosity * 0.3f, Luminosity * 0.081f, Light);
        AddTerrainLight(position[0], position[1], Light, 2, PrimaryTerrainLight);
        Vector(0.f, 0.f, 0.f, Angle);
        CreateEffect(MODEL_CLOUD, Hero->Object.Position, Angle, Light);

        if (visibleObject)
        {
            objectCount = rand() % visibleObject;
        }

        if (rand_fps_check(5))
        {
            float Matrix1[3][4];
            float Matrix2[3][4];
            vec3_t  pos, position, position2, angle;
            vec3_t vTempPos;

            Vector(0.f, 0.f, -45.f, angle);
            AngleMatrix(angle, Matrix1);

            switch (rand() % 4)
            {
            case 0:
                Vector(-400.f, -1000.f, 0.f, position);
                VectorCopy(position, vTempPos);
                VectorRotate(vTempPos, Matrix1, position);
                VectorAdd(Hero->Object.Position, position, pos);
                Vector(0.f, 0.f, 240.f, angle);
                AngleMatrix(angle, Matrix2);
                Vector(-200.f, -1000.f, 0.f, position);
                break;
            case 1:
                Vector(-300.f, -400.f, 0.f, position);
                VectorCopy(position, vTempPos);
                VectorRotate(vTempPos, Matrix1, position);
                VectorSubtract(Hero->Object.Position, position, pos);
                Vector(0.f, 0.f, 210.f, angle);
                AngleMatrix(angle, Matrix2);
                Vector(-500.f, -1000.f, 0.f, position);
                break;
            case 2:
                Vector(-200.f, -400.f, 0.f, position);
                VectorCopy(position, vTempPos);
                VectorRotate(vTempPos, Matrix1, position);
                VectorAdd(Hero->Object.Position, position, pos);
                Vector(0.f, 0.f, 235.f, angle);
                AngleMatrix(angle, Matrix2);
                Vector(-1000.f, -1500.f, 0.f, position);
                break;
            case 3:
                Vector(-200.f, 400.f, 0.f, position);
                VectorCopy(position, vTempPos);
                VectorRotate(vTempPos, Matrix1, position);
                VectorAdd(Hero->Object.Position, position, pos);
                Vector(0.f, 0.f, 200.f, angle);
                AngleMatrix(angle, Matrix2);
                Vector(-600.f, -1200.f, 0.f, position);
                break;
            }
            VectorCopy(position, vTempPos);
            VectorRotate(vTempPos, Matrix2, position);
            VectorSubtract(pos, position, position2);
            VectorAdd(pos, position, position);
            position[2] -= 300.f;
            position2[2] -= 300.f;
            angle[2] = 0;
            CreateJoint(BITMAP_JOINT_THUNDER + 1, position, position2, angle, 0, NULL, 40.f + rand() % 10);
            CreateJoint(BITMAP_JOINT_THUNDER + 1, position, position2, angle, 0, NULL, 40.f + rand() % 10);
        }
    }

    return objectCount;
}

void MoveObjectSetting(int& objCount)
{
    if (MoveChaosCastleObjectSetting(objCount, visibleObject))                  return;
    if (MoveHellasObjectSetting(objCount, visibleObject))                       return;
    if (battleCastle::MoveBattleCastleObjectSetting(objCount, visibleObject))   return;

    if (gMapManager.WorldActive == WD_10HEAVEN)
    {
        objCount = MoveHeavenThunder();

        if (0 == (rand() % 10))
        {
            vec3_t Position;
            Vector(Hero->Object.Position[0] + (float)(rand() % 5000 - 2500),
                Hero->Object.Position[1] + (float)(rand() % 5000 - 2500),
                Hero->Object.Position[2] - 1000.f, Position);
            vec3_t Light = { 1.f, 1.f, 1.f };
            vec3_t Angle = { 0.f, 0.f, 0.f };
            CreateEffect(BITMAP_LIGHT, Position, Angle, Light);
        }
        return;
    }
    else if (gMapManager.InBloodCastle() == true)
    {
        vec3_t Position, Angle;
        vec3_t  Light;

        Vector(0.f, 0.f, 0.f, Angle);
        Vector(1.f, 1.f, 1.f, Light);

        Vector(Hero->Object.Position[0] + (float)(rand() % 900 - 300),
            Hero->Object.Position[1] + (float)(rand() % 900 - 300),
            Hero->Object.Position[2] + rand() % 50 + 250.f, Position);

        if (rand_fps_check(4))
        {
            CreateParticle(BITMAP_FLARE, Position, Angle, Light, 3, 0.19f, NULL);
        }
        return;
    }
}

void MoveObjectOnEffect(OBJECT* o, int& objCount, int& visObject)
{
    vec3_t  Light;
    if (gMapManager.WorldActive == WD_10HEAVEN)
    {
        visObject++;
        if (objCount)
        {
            if (rand_fps_check(10) && o->Type >= 0 && o->Type <= 5)
            {
                Vector(rand() % 10 / 50.f, rand() % 10 / 50.f, rand() % 10 / 50.f, Light);
                CreateSprite(BITMAP_CLOUD + 1, o->Position, 0.5f, Light, &Hero->Object);
                CreateJoint(BITMAP_JOINT_THUNDER, o->Position, o->Position, o->Angle, 6, o, rand() % 20 + 10.f);
                CreateJoint(BITMAP_JOINT_THUNDER, o->Position, o->Position, o->Angle, 6, o, rand() % 20 + 10.f);
            }
            else
            {
                visObject--;
            }
        }
    }
    else
    {
        if (MoveChaosCastleObject(o, objCount, visObject))  return;
        if (battleCastle::MoveBattleCastleObject(o, objCount, visObject))   return;
    }
}

void MoveObjects()
{
    int     objectCount = 0;

    MoveObjectSetting(objectCount);

    visibleObject = 0;
    for (int i = 0; i < 16; i++)
    {
        for (int j = 0; j < 16; j++)
        {
            OBJECT_BLOCK* ob = &ObjectBlock[i * 16 + j];
            if (ob->Visible)
            {
                OBJECT* o = ob->Head;
                while (1)
                {
                    if (o != NULL)
                    {
                        if (o->Live && o->Visible)
                        {
                            MoveObject(o);
                            MoveObjectOnEffect(o, objectCount, visibleObject);
                            ActionObject(o);
                        }
                        if (o->Next == NULL) break;
                        o = o->Next;
                    }
                    else break;
                }
            }
            else
            {
                OBJECT* o = ob->Head;
                while (1)
                {
                    if (o != NULL)
                    {
                        if (o->Live)
                        {
                            ActionObject(o);
                        }
                        o->Visible = false;
                        if (o->Next == NULL) break;
                        o = o->Next;
                    }
                    else break;
                }
            }
        }
    }
    g_iActionTime--;
}

void CreateOperate(OBJECT* Owner)
{
    for (int i = 0; i < MAX_OPERATES; i++)
    {
        OPERATE* o = &Operates[i];
        if (!o->Live)
        {
            o->Live = true;
            o->Owner = Owner;
            return;
        }
    }
}

OBJECT* CreateObject(int Type, vec3_t Position, vec3_t Angle, float Scale)
{
    int i = (int)(Position[0] / (16 * TERRAIN_SCALE));
    int j = (int)(Position[1] / (16 * TERRAIN_SCALE));
    if (i < 0 || j < 0 || i >= 16 || j >= 16) return NULL;

    BYTE Block = i * 16 + j;
    OBJECT_BLOCK* ob = &ObjectBlock[Block];
    auto* o = new OBJECT;

    o->Initialize();

    if (ob->Head == NULL)
    {
        o->Prior = NULL;
        o->Next = NULL;
        ob->Head = o;
        ob->Tail = o;
    }
    else
    {
        ob->Tail->Next = o;
        o->Prior = ob->Tail;
        o->Next = NULL;
        ob->Tail = o;
    }
    o->Live = true;
    o->Visible = false;
    o->AlphaEnable = false;
    o->LightEnable = true;
    o->ContrastEnable = false;
    o->EnableBoneMatrix = false;

    o->m_bCollisionCheck = false;

    o->Type = Type;
    o->Scale = Scale;
    o->Alpha = 1.f;
    o->AlphaTarget = 1.f;
    o->Velocity = 0.16f;
    o->ShadowScale = 50.f;
    o->HiddenMesh = -1;
    o->CurrentAction = 0;
    o->PriorAction = 0;
    o->AnimationFrame = 0.f;
    o->PriorAnimationFrame = 0.f;
    o->Block = Block;
    o->BlendMesh = -1;
    o->BlendMeshLight = 1.f;
    o->BlendMeshTexCoordU = 0.f;
    o->BlendMeshTexCoordV = 0.f;
    g_CharacterClearBuff(o);
    o->CollisionRange = -30.f;
    o->Timer = 0.f;
    o->m_bpcroom = FALSE;
    o->m_bRenderAfterCharacter = false;

    VectorCopy(Position, o->Position);
    VectorCopy(Position, o->StartPosition);
    VectorCopy(Angle, o->Angle);
    Vector(0.f, 0.f, 0.f, o->Light);
    Vector(0.f, 0.f, 0.f, o->HeadAngle);
    Vector(0.f, 0.f, 0.f, o->Direction);
    Vector(-40.f, -40.f, 0.f, o->BoundingBoxMin);
    Vector(40.f, 40.f, 80.f, o->BoundingBoxMax);
    if (SceneFlag == LOG_IN_SCENE || SceneFlag == CHARACTER_SCENE)
    {
        switch (Type)
        {
        case MODEL_SHIP:
            o->Scale = 0.8f;
            Vector(0.2f, 0.2f, 0.2f, o->Light);
            o->LightEnable = true;
            break;
        case MODEL_WAVEBYSHIP:
            o->Scale = 0.8f;
            o->BlendMesh = 0;
            o->BlendMeshLight = 1.f;
            Vector(1.f, 1.f, 1.f, o->Light);
            break;
        case MODEL_MUGAME:
            o->Scale = 2.2f;
            o->BlendMesh = 1;
            o->LightEnable = false;
            break;
        case MODEL_LOGO:
            o->BlendMesh = 1;
            o->BlendMeshLight = 1.f;
            o->Scale = 5.f;
            o->Scale = 0.044f;
            Vector(1.f, 1.f, 1.f, o->Light);
            o->LightEnable = false;
            break;
        case MODEL_LOGOSUN:
            o->Scale = 3.f;
            o->BlendMesh = 0;
            Vector(0.5f, 0.5f, 0.5f, o->Light);
            break;
        case MODEL_LOGO + 4:
            o->BlendMesh = 10;
            Vector(1.f, 1.f, 1.f, o->Light);
            break;
        case MODEL_DRAGON:
            o->HiddenMesh = -2;
            Vector(1.f, 1.f, 1.f, o->Light);
            CreateEffect(MODEL_DRAGON, o->Position, o->Angle, o->Light, 0, o, -1, 0, 0, 0, 1.6f);
            break;
        }
    }
    switch (gMapManager.WorldActive)
    {
    case WD_0LORENCIA:
        switch (Type)
        {
        case MODEL_BRIDGE:
            o->CollisionRange = -50.f;
            break;
        case MODEL_HOUSE01 + 2:
            o->BlendMesh = 4;
            break;
        case MODEL_HOUSE01 + 3:
            o->BlendMesh = 8;
            break;
        case MODEL_HOUSE01 + 4:
            o->BlendMesh = 2;
            break;
        case MODEL_HOUSE_WALL01 + 1:
            o->BlendMesh = 4;
            break;
        case MODEL_WATERSPOUT:
            o->BlendMesh = 3;
            break;
        case MODEL_BONFIRE:
            o->BlendMesh = 1;
            break;
        case MODEL_CARRIAGE01:
            o->BlendMesh = 2;
            break;
        case MODEL_TREE01:
        case MODEL_TREE01 + 1:
            //case MODEL_TREE01+10:
            Vector(-150.f, -150.f, 0.f, o->BoundingBoxMin);
            Vector(150.f, 150.f, 500.f, o->BoundingBoxMax);
            o->Velocity = 1.f / o->Scale * 0.4f;
            //o->AlphaEnable = true;
            break;
        case MODEL_STREET_LIGHT:
            o->BlendMesh = 1;
            o->Velocity = 0.3f;
            break;
        case MODEL_CANDLE:
            o->BlendMesh = 1;
            o->Velocity = 0.3f;
            break;
        case MODEL_TREASURE_CHEST:
            o->Velocity = 0.f;
            break;
        case MODEL_SIGN01:
        case MODEL_SIGN01 + 1:
            o->Velocity = 0.3f;
            //CreateNpc(o);
            break;
        case MODEL_POSE_BOX:
            CreateOperate(o);
            Vector(40.f, 40.f, 160.f, o->BoundingBoxMax);
            o->HiddenMesh = -2;
            break;
        case MODEL_TREE01 + 6:
        case MODEL_FURNITURE01 + 5:
        case MODEL_FURNITURE01 + 6:
            CreateOperate(o);
            break;
        }
        break;
    case WD_1DUNGEON:
        switch (Type)
        {
        case 59:
            CreateOperate(o);
            break;
        case 60:
            CreateOperate(o);
            Vector(40.f, 40.f, 160.f, o->BoundingBoxMax);
            o->HiddenMesh = -2;
            break;
        }
        break;
    case WD_2DEVIAS:
        switch (Type)
        {
        case MODEL_WARP:
        {
            vec3_t Position;
            Vector(o->Position[0], o->Position[1], o->Position[2] + 360.f, Position);
            CreateEffect(MODEL_WARP, Position, o->Angle, o->Light, 1);

            Vector(o->Position[0], o->Position[1] + 4.0f, o->Position[2] + 360.f, Position);
            CreateEffect(MODEL_WARP2, Position, o->Angle, o->Light, 1);

            Vector(o->Position[0], o->Position[1] + 20.0f, o->Position[2] + 360.f, Position);
            CreateEffect(MODEL_WARP3, Position, o->Angle, o->Light, 1);
        }
        break;
        case 22:
        case 25:
        case 40:
        case 45:
        case 55:
        case 73:
            CreateOperate(o);
            break;
        case 91:
            CreateOperate(o);
            Vector(40.f, 40.f, 160.f, o->BoundingBoxMax);
            o->HiddenMesh = -2;
            break;
        case 19:
        case 92:
        case 93:
            o->BlendMesh = 0;
            break;
        case 54:
        case 56:
            o->BlendMesh = 1;
            break;
        case 78:
            o->BlendMesh = 3;
            break;
        case 20:
        case 65:
        case 88:
        case 86:
            o->Angle[2] = (float)((int)o->Angle[2] % 360);
            VectorCopy(o->Angle, o->HeadAngle);
            VectorCopy(o->Position, o->HeadTargetAngle);
            break;
        case 100:
            o->HiddenMesh = -2;
            break;
        }
        break;
    case WD_3NORIA:
        switch (Type)
        {
        case MODEL_WARP:
        {
            vec3_t Position;
            Vector(o->Position[0], o->Position[1], o->Position[2] + 350.f, Position);
            CreateEffect(MODEL_WARP, Position, o->Angle, o->Light);

            Vector(o->Position[0], o->Position[1] + 4.0f, o->Position[2] + 350.f, Position);
            CreateEffect(MODEL_WARP2, Position, o->Angle, o->Light);

            Vector(o->Position[0], o->Position[1] + 8.0f, o->Position[2] + 350.f, Position);
            CreateEffect(MODEL_WARP, Position, o->Angle, o->Light);

            Vector(o->Position[0], o->Position[1] + 12.0f, o->Position[2] + 350.f, Position);
            CreateEffect(MODEL_WARP2, Position, o->Angle, o->Light);

            Vector(o->Position[0], o->Position[1] + 20.0f, o->Position[2] + 350.f, Position);
            CreateEffect(MODEL_WARP3, Position, o->Angle, o->Light);
        }
        break;
        case 8:
            CreateOperate(o);
            break;
        case 1:
            o->BlendMesh = 1;
            break;
        case 9:
            o->BlendMesh = 3;
            break;
        case 38:
            CreateOperate(o);
            o->HiddenMesh = -2;
            break;
        case 17:
        case 37:
            o->BlendMesh = 0;
            break;
        case 19:
            o->BlendMesh = 0;
            break;
        case 18:
            o->BlendMesh = 2;
            break;
        }
        break;
    case WD_7ATLANSE:
        switch (Type)
        {
        case 39:
            CreateOperate(o);
            o->HiddenMesh = -2;
            break;
        }
        break;
    case WD_8TARKAN:
        switch (Type)
        {
        case 78:
            CreateOperate(o);
            //CreateJoint(BITMAP_JOINT_ENERGY,o->Position,o->Position,o->Angle,2,o,30.f);
            //CreateJoint(BITMAP_JOINT_ENERGY,o->Position,o->Position,o->Angle,3,o,30.f);
            break;
        }
        break;
    case WD_55LOGINSCENE:
    {
        switch (Type)
        {
        case 90:
        case 86:
            o->Position[0] = 8600.0f;
            o->Position[1] = 25000.0f;
            if (Type == 90)
            {
                o->Scale = 110.0f;
                o->Position[2] = 5000.0f;
            }
            else
            {
                o->Scale = 60.0f;
                o->Position[2] = 5000.0f;
            }
            Vector(0.0f, 0.0f, 0.0f, o->Angle);
            break;
        }
        break;
    }
    default:
    {
        if (CreateChaosCastleObject(o) == true) return o;
        if (CreateHellasObject(o) == true) return o;
        battleCastle::CreateBattleCastleObject(o);
        M31HuntingGround::CreateHuntingGroundObject(o);
        M34CryingWolf2nd::CreateCryingWolf2ndObject(o);
        M33Aida::CreateAidaObject(o);
        M34CryWolf1st::CreateCryWolf1stObject(o);
        M37Kanturu1st::CreateKanturu1stObject(o);
        M38Kanturu2nd::Create_Kanturu2nd_Object(o);
        M39Kanturu3rd::CreateKanturu3rdObject(o);
        SEASON3A::CGM3rdChangeUp::Instance().CreateBalgasBarrackObject(o);
        SEASON3A::CGM3rdChangeUp::Instance().CreateBalgasRefugeObject(o);
        g_CursedTemple->CreateObject(o);
        SEASON3B::GMNewTown::CreateObject(o);
        SEASON3C::GMSwampOfQuiet::CreateObject(o);
        TheMapProcess().CreateObject(o);
    }
    break;
    }
    return o;
}

void DeleteObject(OBJECT* o, OBJECT_BLOCK* ob)
{
    if (o != NULL)
    {
        OBJECT* Next = o->Next;
        OBJECT* Prior = o->Prior;
        if (Next != NULL)
        {
            if (Prior != NULL)
            {
                Prior->Next = o->Next;
                Next->Prior = o->Prior;
            }
            else
            {
                Next->Prior = NULL;
                ob->Head = Next;
            }
        }
        else
        {
            if (Prior != NULL)
            {
                Prior->Next = NULL;
                ob->Tail = Prior;
            }
            else
            {
                ob->Head = NULL;
                ob->Tail = NULL;
            }
        }
        SAFE_DELETE(o);
    }
}

typedef std::vector<OBJECT* > ObjectPtrVec_t;
typedef std::map<int, ObjectPtrVec_t> SortObj_t;
void SortInBlockByType()
{
    SortObj_t sortMap;
    for (int i = 0; i < 16; ++i)
    {
        for (int j = 0; j < 16; ++j)
        {
            OBJECT_BLOCK* ob = &ObjectBlock[i * 16 + j];
            OBJECT* o = ob->Head;
            while (1)
            {
                if (o == NULL) break;
                sortMap[o->Type].push_back(o);
                o = o->Next;
            }

            o = ob->Head;
            for (SortObj_t::iterator iter = sortMap.begin(); iter != sortMap.end(); ++iter)
            {
                ObjectPtrVec_t& rObjectVec = iter->second;
                for (ObjectPtrVec_t::iterator iter2 = rObjectVec.begin(); iter2 != rObjectVec.end(); ++iter2)
                {
                    o = *iter2;
                    o = o->Next;
                }
            }

            sortMap.clear();
        }
    }
}

void DeleteObjectTile(int x, int y)
{
    for (int i = 0; i < 16; i++)
    {
        for (int j = 0; j < 16; j++)
        {
            OBJECT_BLOCK* ob = &ObjectBlock[i * 16 + j];
            OBJECT* o = ob->Head;
            while (1)
            {
                if (o != NULL && o != (OBJECT*)0xdddddddd)
                {
                    if (o->Live && (int)(o->Position[0] / TERRAIN_SCALE) == x && (int)(o->Position[1] / TERRAIN_SCALE) == y)
                        DeleteObject(o, ob);
                    if (o->Next == NULL) break;
                    o = o->Next;
                }
                else break;
            }
        }
    }
}

int OpenObjects(wchar_t* FileName)
{
    FILE* fp = _wfopen(FileName, L"rb");
    if (fp == NULL)
    {
        wchar_t Text[256];
        swprintf(Text, L"%s file not found.", FileName);
        MessageBox(g_hWnd, Text, NULL, MB_OK);
        SendMessage(g_hWnd, WM_DESTROY, 0, 0);
        return (-1);
    }
    fseek(fp, 0, SEEK_END);
    int EncBytes = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    auto* EncData = new unsigned char[EncBytes];
    fread(EncData, 1, EncBytes, fp);
    fclose(fp);

    unsigned char* Data = EncData;
    int DataBytes = EncBytes;

    int DataPtr = 0;

    BYTE Version = *((BYTE*)(Data + DataPtr)); DataPtr += 1;

    int iMapNumber = 0;
    short Count = *((short*)(Data + DataPtr)); DataPtr += 2;
    for (int i = 0; i < Count; i++)
    {
        vec3_t Position;
        vec3_t Angle;
        short Type = *((short*)(Data + DataPtr)); DataPtr += 2;
        memcpy(Position, Data + DataPtr, sizeof(vec3_t)); DataPtr += sizeof(vec3_t);
        memcpy(Angle, Data + DataPtr, sizeof(vec3_t)); DataPtr += sizeof(vec3_t);
        float Scale = *((float*)(Data + DataPtr)); DataPtr += 4;
        CreateObject(Type, Position, Angle, Scale);
    }
    delete[] Data;

    fclose(fp);
    return iMapNumber;
}

int OpenObjectsEnc(wchar_t* FileName)
{
    FILE* fp = _wfopen(FileName, L"rb");
    if (fp == NULL)
    {
        wchar_t Text[256];
        swprintf(Text, L"%s file not found.", FileName);
        MessageBox(g_hWnd, Text, NULL, MB_OK);
        SendMessage(g_hWnd, WM_DESTROY, 0, 0);
        return (-1);
    }
    fseek(fp, 0, SEEK_END);
    int EncBytes = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    auto* EncData = new unsigned char[EncBytes];
    fread(EncData, 1, EncBytes, fp);
    fclose(fp);

    int DataBytes = MapFileDecrypt(NULL, EncData, EncBytes);
    auto* Data = new unsigned char[DataBytes];
    MapFileDecrypt(Data, EncData, EncBytes);
    delete[] EncData;

    int DataPtr = 0;
    DataPtr += 1;
    int iMapNumber = (int)*((BYTE*)(Data + DataPtr)); DataPtr += 1;
    short Count = *((short*)(Data + DataPtr)); DataPtr += 2;
    g_iTotalObj = Count;
    for (int i = 0; i < Count; i++)
    {
        vec3_t Position;
        vec3_t Angle;
        short Type = *((short*)(Data + DataPtr)); DataPtr += 2;
        memcpy(Position, Data + DataPtr, sizeof(vec3_t)); DataPtr += sizeof(vec3_t);
        memcpy(Angle, Data + DataPtr, sizeof(vec3_t)); DataPtr += sizeof(vec3_t);
        float Scale = *((float*)(Data + DataPtr)); DataPtr += 4;
        CreateObject(Type, Position, Angle, Scale);
    }
    delete[] Data;

    fclose(fp);
    return iMapNumber;
}

bool SaveObjects(wchar_t* FileName, int iMapNumber)
{
    FILE* fp = _wfopen(FileName, L"wb");

    short ObjectCount = 0;
    int CounterPoint = 3;
    BYTE Version = 0;
    fwrite(&Version, sizeof(BYTE), 1, fp);
    fwrite(&iMapNumber, 1, 1, fp);
    fseek(fp, 4, SEEK_SET);
    for (int i = 0; i < 16; i++)
    {
        for (int j = 0; j < 16; j++)
        {
            OBJECT_BLOCK* ob = &ObjectBlock[i * 16 + j];
            OBJECT* o = ob->Head;
            while (1)
            {
                if (o != NULL)
                {
                    if (o->Live)
                    {
                        fwrite(&o->Type, 2, 1, fp);
                        fwrite(o->Position, sizeof(vec3_t), 1, fp);
                        fwrite(o->Angle, sizeof(vec3_t), 1, fp);
                        fwrite(&o->Scale, sizeof(float), 1, fp);
                    }
                    ObjectCount++;
                    if (o->Next == NULL) break;
                    o = o->Next;
                }
                else break;
            }
        }
    }
    int EndPoint = ftell(fp);
    fseek(fp, 2, SEEK_SET);
    fwrite(&ObjectCount, 2, 1, fp);
    fseek(fp, EndPoint, SEEK_SET);

    fclose(fp);

    {
        fp = _wfopen(FileName, L"rb");
        fseek(fp, 0, SEEK_END);
        int EncBytes = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        auto* EncData = new unsigned char[EncBytes];
        fread(EncData, 1, EncBytes, fp);
        fclose(fp);

        int DataBytes = MapFileEncrypt(NULL, EncData, EncBytes);
        auto* Data = new unsigned char[DataBytes];
        MapFileEncrypt(Data, EncData, EncBytes);
        delete[] EncData;

        fp = _wfopen(FileName, L"wb");
        fwrite(Data, DataBytes, 1, fp);
        fclose(fp);
        delete[] Data;
    }
    return true;
}

void SaveTrapObjects(wchar_t* FileName)
{
    FILE* fp = _wfopen(FileName, L"wt");
    fwprintf(fp, L"0\n");
    for (int i = 0; i < 16; i++)
    {
        for (int j = 0; j < 16; j++)
        {
            OBJECT_BLOCK* ob = &ObjectBlock[i * 16 + j];
            OBJECT* o = ob->Head;
            while (1)
            {
                if (o != NULL)
                {
                    if ((gMapManager.WorldActive == WD_1DUNGEON && (o->Type == 39 || o->Type == 40 || o->Type == 51)) ||
                        (gMapManager.WorldActive == WD_4LOSTTOWER && (o->Type == 25)))
                    {
                        int Type = 0;
                        switch (o->Type)
                        {
                        case 39:Type = 100; break;
                        case 40:Type = 101; break;
                        case 51:Type = 102; break;
                        case 25:Type = 103; break;
                        }
                        fwprintf(fp, L"%4d %4d 0 %4d %4d %4d\n", Type, gMapManager.WorldActive, (BYTE)(o->Position[0] / TERRAIN_SCALE), (BYTE)(o->Position[1] / TERRAIN_SCALE), (BYTE)((o->Angle[2] + 22.5f) / 360.f * 8.f + 1.f) % 8);
                    }
                    if (o->Next == NULL) break;
                    o = o->Next;
                }
                else break;
            }
        }
    }
    fwprintf(fp, L"end\n");
    fclose(fp);
}

OBJECT  g_CloudsLow;
void    RenderCloudLowLevel(int index, int Type)
{
    OBJECT* o = &g_CloudsLow;

    o->Alpha = 1.f;
    o->Live = true;
    o->Type = Type;
    o->SubType = 0;
    o->Scale = 10.f;

    if (o->LifeTime != 10)
    {
        Vector(0.f, 0.f, 0.f, o->StartPosition);
        Vector(0.f, 0.f, 0.f, o->Position);
    }
    o->LifeTime = 10;

    Vector(0.f, 0.f, 0.f, o->Angle);
    Vector(0.f, 0.f, 0.f, o->HeadAngle);
    Vector(0.f, 0.f, 0.f, o->HeadTargetAngle);

    o->EnableBoneMatrix = false;
    o->CurrentAction = 0;
    o->PriorAction = 0;
    o->AnimationFrame = 1.f;
    o->PriorAnimationFrame = 1.f;

    o->BlendMeshLight = 1.f;
    o->LightEnable = true;

    o->HiddenMesh = -1;

    if (index != 0)
    {
        vec3_t  delPosition;
        delPosition[0] = (Hero->Object.Position[0] - o->Position[0]);
        delPosition[1] = (Hero->Object.Position[1] - o->Position[1]);
        delPosition[2] = 0.f;

        float Matrix[3][4];
        vec3_t angle;

        Vector(0.f, 0.f, -45.f, angle);
        AngleMatrix(angle, Matrix);
        VectorRotate(delPosition, Matrix, delPosition);

        if (index == 1)
        {
            o->StartPosition[0] -= sinf(WorldTime * 0.0002f) * 0.001f;
            o->StartPosition[1] += cosf(WorldTime * 0.0002f) * 0.001f;
        }
        else
        {
            o->StartPosition[0] += sinf(WorldTime * 0.0002f) * 0.001f;
            o->StartPosition[1] -= cosf(WorldTime * 0.0002f) * 0.001f;
        }

        if (index == 1)
        {
            o->BlendMesh = -1;
        }
        else
        {
            o->BlendMeshLight = 1.f;
            o->BlendMesh = 0;
        }
        o->BlendMeshTexCoordU = o->StartPosition[0];
        o->BlendMeshTexCoordV = o->StartPosition[1];
        Vector(Hero->Object.Position[0], Hero->Object.Position[1], Hero->Object.Position[2] - 150.f + (index * 20), o->Position);

        RenderObject(o);
    }
}

void ClearItems()
{
    for (int i = 0; i < MAX_ITEMS; i++)
    {
        OBJECT* o = &Items[i].Object;
        o->Live = false;
    }
}

void ItemObjectAttribute(OBJECT* o)
{
    Vector(0.3f, 0.3f, 0.3f, o->Light);
    o->LightEnable = true;
    o->AlphaEnable = false;
    o->EnableShadow = false;
    o->Velocity = 0.f;
    o->CollisionRange = 50.f;
    o->PriorAnimationFrame = 0.f;
    o->AnimationFrame = 0.f;
    o->PriorAction = 0;
    o->CurrentAction = 0;
    o->HiddenMesh = -1;
    o->Gravity = 0.f;
    o->Alpha = 1.f;
    o->BlendMesh = -1;
    o->BlendMeshLight = 1.f;
    o->BlendMeshTexCoordU = 0.f;
    o->BlendMeshTexCoordV = 0.f;
    o->HiddenMesh = -1;
    o->Scale = 0.8f;
    g_CharacterClearBuff(o);
    if (o->Type >= MODEL_SPEAR && o->Type <= MODEL_PLATINA_STAFF)
        o->Scale = 0.7f;
    switch (o->Type)
    {
    case MODEL_SAINT_CROSSBOW:
        o->BlendMesh = -2;
        o->BlendMeshLight = sinf(WorldTime * 0.004f) * 0.2f + 0.9f;
        break;
    case MODEL_STAFF_OF_DESTRUCTION:
        o->BlendMesh = -2;
        o->BlendMeshLight = sinf(WorldTime * 0.004f) * 0.2f + 0.9f;
        //o->BlendMeshTexCoordU = (float)((int)(WorldTime)%2000)*0.0005f;
        break;
    case MODEL_CHAOS_LIGHTNING_STAFF:
        o->BlendMesh = 1;
        o->BlendMeshLight = (float)(rand() % 11) * 0.1f;
        break;
    case MODEL_STAFF_OF_RESURRECTION:
        o->BlendMesh = -2;
        o->BlendMeshLight = sinf(WorldTime * 0.004f) * 0.3f + 0.7f;
        break;
    case MODEL_CHAOS_NATURE_BOW:
    case MODEL_BLUEWING_CROSSBOW:
    case MODEL_AQUAGOLD_CROSSBOW:
        o->BlendMesh = -2;
        o->BlendMeshLight = sinf(WorldTime * 0.004f) * 0.3f + 0.7f;
        break;
    case MODEL_RED_SPIRIT_PANTS:
    case MODEL_RED_SPIRIT_HELM:
        o->BlendMeshLight = sinf(WorldTime * 0.001f) * 0.4f + 0.6f;
        break;
    case MODEL_STAFF_OF_KUNDUN:
        o->BlendMesh = 2;
        o->BlendMeshLight = sinf(WorldTime * 0.004f) * 0.3f + 0.7f;
        break;
    case MODEL_CRYSTAL_MORNING_STAR:
        o->BlendMesh = 1;
        o->BlendMeshLight = sinf(WorldTime * 0.004f) * 0.2f + 0.8f;
        break;
    case MODEL_CRYSTAL_SWORD:
        o->BlendMesh = 0;
        break;
    case MODEL_CHAOS_DRAGON_AXE:
        o->BlendMesh = 1;
        o->BlendMeshLight = sinf(WorldTime * 0.004f) * 0.3f + 0.7f;
        break;
    case MODEL_BILL_OF_BALROG:
        o->BlendMeshTexCoordV = -(float)((int)(WorldTime) % 2000) * 0.0005f;
        break;
    case MODEL_LEGENDARY_SHIELD:
        o->BlendMesh = 1;
        o->BlendMeshTexCoordU = (float)(rand() % 10) * 0.1f;
        o->BlendMeshTexCoordV = (float)(rand() % 10) * 0.1f;
        break;
    case MODEL_ELEMENTAL_SHIELD:
        o->HiddenMesh = 2;
        break;
    case MODEL_ELEMENTAL_MACE:
        o->HiddenMesh = 2;
        break;
    case MODEL_RUNE_BLADE:
        o->HiddenMesh = 2;
        break;
    case MODEL_DRAGON_SPEAR:
        o->HiddenMesh = 1;
        break;
    case MODEL_DRAGON_SOUL_STAFF:
        o->BlendMesh = 1;
        break;
    case MODEL_LEGENDARY_STAFF:
        o->BlendMesh = 2;
        o->BlendMeshTexCoordU = (float)(rand() % 10) * 0.1f;
        o->BlendMeshTexCoordV = (float)(rand() % 10) * 0.1f;
        break;
    case MODEL_ETC:
        o->Scale = 0.7f;
        break;
    case MODEL_SMALL_HEALING_POTION:
        o->Scale = 1.f;
        break;
    case MODEL_POTION + 21:
        o->Scale = 0.5f;
        break;
    case MODEL_LIGHTING_SWORD:
        o->BlendMesh = 1;
        o->BlendMeshTexCoordU = (float)(rand() % 10) * 0.1f;
        o->BlendMeshLight = sinf(WorldTime * 0.004f) * 0.3f + 0.7f;
        break;
    case MODEL_LIGHT_SABER:
    case MODEL_SPEAR:
        o->BlendMesh = 1;
        o->BlendMeshLight = sinf(WorldTime * 0.004f) * 0.3f + 0.7f;
        break;
    case MODEL_BLADE:
    case MODEL_DOUBLE_BLADE:
        o->BlendMesh = 1;
        break;
    case MODEL_STAFF:
        o->BlendMesh = 2;
        break;
    case MODEL_HELPER:
        o->BlendMesh = 1;
        break;
    case MODEL_WINGS_OF_SPIRITS:
        o->Scale = 0.5f;
        o->BlendMesh = 0;
        break;
    case MODEL_WINGS_OF_SOUL:
    case MODEL_WINGS_OF_DRAGON:
    case MODEL_WINGS_OF_DARKNESS:
        break;
    case MODEL_WING:
    case MODEL_ORB_OF_HEALING:
    case MODEL_ORB_OF_GREATER_DEFENSE:
    case MODEL_ORB_OF_GREATER_DAMAGE:
    case MODEL_ORB_OF_SUMMONING:
    case MODEL_WING + 20:
    case MODEL_WING + 132:
        o->BlendMesh = 0;
        break;
    case MODEL_SERPENT_SHIELD:
    case MODEL_BRONZE_SHIELD:
    case MODEL_DRAGON_SHIELD:
        o->BlendMesh = 1;
        o->BlendMeshLight = sinf(WorldTime * 0.004f) * 0.3f + 0.7f;
        break;
    }
}

bool ItemAngleRF(OBJECT* o)
{
    if (o->Type == MODEL_CAPE_OF_FIGHTER)
    {
        o->Angle[0] = 270.0f;
        o->Angle[1] = 180.0f;
        o->Angle[2] = 45.0f;
        o->Scale = 0.7f;
        return true;
    }

    if (o->Type == MODEL_CAPE_OF_OVERRULE)
    {
        o->Angle[0] = 250.0f;
        o->Angle[1] = 180.0f;
        o->Angle[2] = 45.0f;
        return true;
    }

    if (o->Type >= MODEL_SACRED_HELM && o->Type <= MODEL_SACRED_HELM + 2)
    {
        o->Scale = 1.0f;
        o->Angle[2] = 45.0f;
        return true;
    }

    if (o->Type >= MODEL_CHAIN_DRIVE_PARCHMENT && o->Type <= MODEL_INCREASE_BLOCK_PARCHMENT)
    {
        o->Angle[0] = 270.f;
        o->Scale = 0.8f;
        return true;
    }

    return false;
}

void ItemAngle(OBJECT* o)
{
    Vector(0.f, 0.f, -45.f, o->Angle);

    o->Angle[0] = 0.f;

    if (o->Type >= MODEL_SWORD && o->Type < MODEL_AXE + MAX_ITEM_INDEX)
    {
        o->Angle[0] = 60.f;
        if (o->Type == MODEL_DIVINE_SWORD_OF_ARCHANGEL)
            o->Scale = 0.7f;
    }
    else if (o->Type == MODEL_ARROW_VIPER_BOW || o->Type == MODEL_SYLPH_WIND_BOW || o->Type == MODEL_ALBATROSS_BOW)
    {
        o->Angle[0] = 0.f;
        o->Angle[1] = 0.f;
    }
    else if ((o->Type >= MODEL_CROSSBOW && o->Type < MODEL_CELESTIAL_BOW) || (o->Type >= MODEL_DIVINE_CB_OF_ARCHANGEL && o->Type < MODEL_ARROW_VIPER_BOW))
    {
        o->Angle[0] = 90.f;
        o->Angle[1] = 0.f;
    }
    else if (o->Type >= MODEL_MACE && o->Type < MODEL_STAFF + MAX_ITEM_INDEX)
    {
        o->Angle[0] = 0.f;
        o->Angle[1] = 270.f;
    }
    else if (o->Type >= MODEL_SHIELD && o->Type < MODEL_SHIELD + MAX_ITEM_INDEX)
    {
        o->Angle[1] = 270.f;
        o->Angle[2] = 270.f - 45.f;
    }
    else if (MODEL_MISTERY_HELM <= o->Type && MODEL_LILIUM_HELM >= o->Type)
    {
        o->Scale = 1.5f;
        o->Angle[2] = 45.f;
    }
    else if (o->Type >= MODEL_ARMOR && o->Type < MODEL_GLOVES + MAX_ITEM_INDEX)
    {
        o->Angle[0] = 270.f;
    }
    else if (o->Type >= MODEL_RED_RIBBON_BOX && o->Type <= MODEL_BLUE_RIBBON_BOX)
    {
        o->Scale = 0.3f;
        o->Angle[0] = 0.f;
        o->Angle[2] = 90.f;
    }
    else if (o->Type >= MODEL_SEED_FIRE && o->Type <= MODEL_SEED_EARTH)
    {
        o->Angle[0] = 0.f;
        o->Scale = 0.6f;
    }
    else if (o->Type >= MODEL_SPHERE_MONO && o->Type <= MODEL_SPHERE_5)
    {
        o->Angle[0] = 0.f;
        o->Scale = 0.6f;
    }
    else if (o->Type >= MODEL_SEED_SPHERE_FIRE_1 && o->Type <= MODEL_SEED_SPHERE_EARTH_5)
    {
        o->Angle[0] = 0.f;
        o->Scale = 0.6f;
    }
    else if (o->Type == MODEL_PUMPKIN_OF_LUCK)
    {
        o->Scale = 0.9f;
        o->Angle[0] = 0.f;
        o->Angle[2] = 90.f;
    }
    else if (o->Type >= MODEL_JACK_OLANTERN_BLESSINGS && o->Type <= MODEL_JACK_OLANTERN_CRY)
    {
        o->Scale = 0.7f;
        o->Angle[0] = 90.f;
    }
    else if (o->Type == MODEL_JACK_OLANTERN_FOOD)
    {
        o->Scale = 0.9f;
        o->Angle[0] = 0.f;
        o->Angle[2] = 90.f;
    }
    else if (o->Type == MODEL_JACK_OLANTERN_DRINK)
    {
        o->Scale = 0.26f;
        o->Angle[0] = 0.f;
        o->Angle[2] = 90.f;
    }
    else if (o->Type >= MODEL_PINK_CHOCOLATE_BOX && o->Type <= MODEL_BLUE_CHOCOLATE_BOX)
    {
        o->Scale = 0.7f;
        o->Angle[0] = 0.f;
        o->Angle[2] = 90.f;
    }
    else if (o->Type >= MODEL_EVENT + 21 && o->Type <= MODEL_EVENT + 23)
    {
        o->Scale = 0.7f;
        o->Angle[0] = 0.f;
        o->Angle[2] = 90.f;
    }
    else if (o->Type >= MODEL_HELPER + 46 && o->Type <= MODEL_HELPER + 48)
    {
        o->Scale = 0.5f;
        o->Angle[0] = 90.f;
    }
    else if (o->Type == MODEL_POTION + 54)
    {
        o->Scale = 0.5f;
        o->Angle[0] = 90.f;
    }
    else if (o->Type == MODEL_POTION + 58)
    {
        o->Scale = 0.3f;
        o->Angle[2] = 90.f;
    }
    else if (o->Type == MODEL_POTION + 59 || o->Type == MODEL_POTION + 60)
    {
        o->Scale = 0.3f;
        o->Angle[0] = 90.f;
        o->Angle[1] = 90.f;
    }
    else if (o->Type == MODEL_POTION + 61 || o->Type == MODEL_POTION + 62)
    {
        o->Scale = 0.3f;
        o->Angle[0] = 90.f;
    }
    else if (o->Type == MODEL_POTION + 53)
    {
        o->Scale = 0.2f;
        o->Angle[2] = 90.f;
    }
    else if (o->Type == MODEL_HELPER + 43 || o->Type == MODEL_HELPER + 44 || o->Type == MODEL_HELPER + 45)
    {
        o->Scale = 0.5f;
        o->Angle[0] = 90.f;
    }
    else if (o->Type >= MODEL_POTION + 70 && o->Type <= MODEL_POTION + 71)
    {
        o->Scale = 0.6f;
        o->Angle[2] = 90.f;
    }
    else if (o->Type >= MODEL_POTION + 72 && o->Type <= MODEL_POTION + 77)
    {
        o->Scale = 0.5f;
        o->Angle[2] = 90.f;
    }
    else if (o->Type == MODEL_HELPER + 59)
    {
        o->Scale = 0.2f;
        o->Angle[2] = 90.f;
    }
    else if (o->Type >= MODEL_HELPER + 54 && o->Type <= MODEL_HELPER + 58)
    {
        o->Scale = 0.7f;
        o->Angle[2] = 90.f;
    }
    else if (o->Type >= MODEL_POTION + 78 && o->Type <= MODEL_POTION + 82)
    {
        o->Scale = 0.5f;
        o->Angle[2] = 90.f;
    }
    else if (o->Type == MODEL_HELPER + 60)
    {
        o->Scale = 1.5f;
        o->Angle[2] = 90.f;
    }
    else if (o->Type == MODEL_HELPER + 61)
    {
        o->Scale = 0.5f;
        o->Angle[0] = 90.f;
    }
    else if (o->Type == MODEL_POTION + 83)
    {
        o->Scale = 0.3f;
        o->Angle[0] = 90.f;
    }
    else if (o->Type >= MODEL_POTION + 145 && o->Type <= MODEL_POTION + 150)
    {
        o->Scale = 0.3f;
        o->Angle[0] = 90.f;
    }
    else if (o->Type >= MODEL_HELPER + 125 && o->Type <= MODEL_HELPER + 127)
    {
        o->Scale = 0.5f;
        o->Angle[0] = 90.f;
    }
    else if (o->Type == MODEL_POTION + 91)
    {
        o->Scale = 0.5f;
        o->Angle[0] = 90.f;
    }
    else if (o->Type == MODEL_POTION + 92)
    {
        o->Scale = 0.5f;
        o->Angle[0] = 90.f;
    }
    else if (o->Type == MODEL_POTION + 93)
    {
        o->Scale = 0.5f;
        o->Angle[0] = 90.f;
    }
    else if (o->Type == MODEL_POTION + 95)
    {
        o->Scale = 0.5f;
        o->Angle[0] = 90.f;
    }
    else if (o->Type == MODEL_POTION + 94)
    {
        o->Scale = 0.6f;
        o->Angle[2] = 90.f;
    }
    else if (o->Type == MODEL_CHERRY_BLOSSOM_PLAYBOX)
    {
        o->Scale = 0.8f;
        o->Angle[2] = 90.f;
    }
    else if (o->Type == MODEL_CHERRY_BLOSSOM_WINE)
    {
        o->Scale = 0.9f;
        o->Angle[2] = 90.f;
    }
    else if (o->Type == MODEL_CHERRY_BLOSSOM_RICE_CAKE)
    {
        o->Scale = 0.7f;
        o->Angle[2] = 90.f;
    }
    else if (o->Type == MODEL_CHERRY_BLOSSOM_FLOWER_PETAL)
    {
        o->Scale = 1.3f;
        o->Angle[2] = 90.f;
    }
    else if (o->Type == MODEL_POTION + 88)
    {
        o->Scale = 0.7f;
        o->Angle[0] = 180.f;
        o->Angle[1] = 180.f;
        //o->Angle[2] = 90.f;
    }
    else if (o->Type == MODEL_POTION + 89)
    {
        o->Scale = 0.7f;
        o->Angle[0] = 30.f;
        o->Angle[2] = 90.f;
    }
    else if (o->Type == MODEL_GOLDEN_CHERRY_BLOSSOM_BRANCH)
    {
        o->Scale = 0.7f;
        o->Angle[0] = 30.f;
        o->Angle[2] = 90.f;
    }
    else if (o->Type >= MODEL_HELPER + 62 && o->Type <= MODEL_HELPER + 63)
    {
        o->Scale = 0.5f;
        o->Angle[0] = 90.f;
    }
    else if (o->Type >= MODEL_POTION + 97 && o->Type <= MODEL_POTION + 98)
    {
        o->Scale = 0.5f;
        o->Angle[2] = 90.f;
    }
    else if (o->Type == MODEL_POTION + 140)
    {
        o->Scale = 0.5f;
        o->Angle[2] = 90.f;
    }
    else if (o->Type == MODEL_POTION + 96)
    {
        o->Scale = 0.2f;
        o->Angle[2] = 90.f;
    }
    else if (o->Type >= MODEL_DEMON && o->Type <= MODEL_SPIRIT_OF_GUARDIAN)
    {
        switch (o->Type)
        {
        case MODEL_DEMON: o->Scale = 0.21f; break;
        case MODEL_SPIRIT_OF_GUARDIAN: o->Scale = 0.5f; break;
        }
        o->Angle[2] = 70.f;
    }

    else if (o->Type == MODEL_OLD_SCROLL)
    {
        o->Angle[0] = 90.f;
        o->Angle[1] = 0.f;
        o->Scale = 0.3f;
    }
    else if (o->Type == MODEL_ILLUSION_SORCERER_COVENANT)
    {
        o->Angle[0] = 0.f;
        o->Scale = 0.6f;
    }
    else if (o->Type == MODEL_SCROLL_OF_BLOOD)
    {
        o->Angle[0] = 90.f;
        o->Scale = 0.45f;
    }
    else if (o->Type == MODEL_POTION + 64)
    {
        o->Angle[0] = 0.f;
        o->Scale = 0.8f;
    }
    else if (o->Type == MODEL_FLAME_OF_CONDOR)
    {
        o->Angle[0] = 0.f;
        o->Scale = 1.2f;
    }
    else if (o->Type == MODEL_FEATHER_OF_CONDOR)
    {
        o->Angle[0] = 0.f;
        o->Scale = 1.2f;
    }
    else if (o->Type == MODEL_FLAME_OF_DEATH_BEAM_KNIGHT)
    {
        o->Angle[0] = 90.f;
        o->Scale = 0.6f;
    }
    else if (o->Type == MODEL_HORN_OF_HELL_MAINE)
    {
        o->Angle[0] = 90.f;
        o->Scale = 0.8f;
    }
    else if (o->Type == MODEL_FEATHER_OF_DARK_PHOENIX)
    {
        o->Angle[0] = 270.f;
        o->Scale = 0.8f;
    }
    else if (o->Type == MODEL_EYE_OF_ABYSSAL)
    {
        o->Angle[2] = -135.f;
        o->Scale = 0.6f;
    }
    else if (o->Type == MODEL_EVENT + 4)
    {
        o->Angle[0] = 90.f;
    }
    else if (o->Type == MODEL_EVENT + 8 || o->Type == MODEL_EVENT + 9)
    {
        o->Angle[0] = 270.f;
        o->Angle[2] = 45.f;
    }
    else if (o->Type == MODEL_EVENT + 10)
    {
        o->Scale = .2f;
    }
    else if (o->Type == MODEL_EVENT + 5)
    {
        o->Angle[0] = 90.f;
    }
    else if (o->Type == MODEL_SCROLL_OF_EMPEROR_RING_OF_HONOR)
    {
        o->Angle[1] = 45.f;
        o->Angle[2] = 45.f;
    }
    else if (o->Type == MODEL_BROKEN_SWORD_DARK_STONE)
    {
        o->Angle[2] = 45.f;
    }
    else if ((o->Type >= MODEL_TEAR_OF_ELF && o->Type < MODEL_POTION + 27) || o->Type == MODEL_LOCHS_FEATHER)
    {
        o->Angle[2] = 45.f;
    }
    else if (o->Type == MODEL_DEVILS_EYE)
    {
        o->Angle[0] = 90.f;
    }
    else if (o->Type == MODEL_FIRECRACKER)
    {
        o->Angle[0] = 70.f;
        o->Scale = 1.5f;
    }
    else if (o->Type == MODEL_CHRISTMAS_FIRECRACKER)
    {
        o->Angle[0] = 70.f;
        o->Angle[2] = 0.f;
        o->Scale = 1.0f;
    }
    else if (o->Type == MODEL_GM_GIFT)
    {
        o->Angle[2] = -10.f;
        o->Scale = 0.4f;
    }
    else if (o->Type == MODEL_DEVILS_KEY)
    {
        o->Angle[0] = o->Angle[2] = 270.f;
    }
    else if (o->Type == MODEL_DEVILS_INVITATION)
    {
        o->Angle[0] = 270.f;
        o->Angle[2] = 90.f;
    }
    else if (o->Type == MODEL_SYMBOL_OF_KUNDUN)
    {
        o->Angle[0] = 90.f;
        o->Angle[2] = 70.f;
    }
    else if (o->Type == MODEL_EVENT + 11)
    {
        o->Angle[0] = 115.f;
        o->Angle[1] = 75.f;
        o->Angle[2] = 8.f;
        o->Scale = 0.4f;
    }
    else if (o->Type == MODEL_SCROLL_OF_ARCHANGEL || o->Type == MODEL_BLOOD_BONE)
    {
        o->Angle[0] = -45.f;
        o->Angle[1] = -5.f;
        o->Angle[2] = 18.f;
        o->Scale = 0.48f;
    }
    else if (o->Type == MODEL_INVISIBILITY_CLOAK)
    {
        o->Angle[0] = 165.f;
        o->Angle[1] = -168.f;
        o->Angle[2] = 198.f;
        o->Scale = 0.48f;
    }
    else if (o->Type == MODEL_CAPE_OF_LORD)
    {
        o->Angle[0] = -45.f;
        o->Angle[1] = 0.f;
        o->Angle[2] = 45.f;
        o->Scale = 0.5f;
    }
    else if (o->Type == MODEL_EVENT + 16)
    {
        o->Angle[2] = 45.f;
        o->Scale = 0.5f;
    }
    else if (o->Type == MODEL_EVENT + 12)
    {
        o->Angle[0] = 160.f;
        o->Angle[1] = -183.f;
        o->Angle[2] = 198.f;
        o->Scale = 0.38f;
    }
    else if (o->Type == MODEL_EVENT + 13)
    {
        o->Angle[0] = 160.f;
        o->Angle[1] = -183.f;
        o->Angle[2] = 198.f;
        o->Scale = 0.54f;
    }
    else if (o->Type == MODEL_POTION + 21)
    {
        o->Angle[0] = 270.f;
        o->Angle[2] = 90.f;
    }
    else if (o->Type == MODEL_EVENT + 7)
    {
        o->Angle[2] = 45.f;
    }
    else if (o->Type == MODEL_POTION + 20)
    {
        o->Angle[2] = 45.f;
    }
    else if (o->Type >= MODEL_RING_OF_FIRE && o->Type <= MODEL_RING_OF_MAGIC)
    {
        o->Angle[2] = 20.f;
    }
    else if (o->Type == MODEL_BLESS_OF_GUARDIAN)
    {
        o->Angle[2] = 45.f;
        o->Scale = 1.2f;
    }
    else if (o->Type == MODEL_CLAW_OF_BEAST)
    {
        o->Angle[0] = 90.f;
    }
    else if (o->Type == MODEL_FRAGMENT_OF_HORN)
    {
        o->Angle[2] = 90.f;
    }
    else if (o->Type == MODEL_BROKEN_HORN)
    {
        o->Angle[2] = 90.f;
        o->Scale = 1.3f;
    }
    else if (o->Type == MODEL_HORN_OF_FENRIR)
    {
        o->Angle[2] = 180.f;
    }
    else if (o->Type == MODEL_JEWEL_OF_LIFE)
    {
        o->Angle[0] = 270.f;
        o->Angle[2] = 90.f - 45.f;
    }
    else if (o->Type == MODEL_JEWEL_OF_HARMONY)
    {
        o->Angle[0] = 270.f;
        o->Angle[2] = -15.f;
        o->Scale = 1.3f;
    }
    else if (o->Type == MODEL_LOWER_REFINE_STONE || o->Type == MODEL_HIGHER_REFINE_STONE)
    {
        o->Angle[0] = 270.f;
        o->Angle[2] = -15.f;
        o->Scale = 1.0f;
    }
    else if (o->Type >= MODEL_CHAIN_LIGHTNING_PARCHMENT && o->Type <= MODEL_INNOVATION_PARCHMENT)
    {
        o->Angle[0] = 270.f;
        o->Scale = 0.8f;
    }
    else if (o->Type == MODEL_HELPER + 66)
    {
        o->Angle[0] = 270.0f;
        o->Scale = 1.0f;
    }
    else if (o->Type == MODEL_POTION + 100)
    {
        o->Angle[0] = 180.0f;
        //	o->Angle[2] = 0.0f;
        o->Scale = 1.0f;
    }
    else if (o->Type >= MODEL_TYPE_CHARM_MIXWING + EWS_BEGIN && o->Type <= MODEL_TYPE_CHARM_MIXWING + EWS_END)
    {
        o->Scale = 0.5f;
        o->Angle[2] = 90.f;
    }
    else if (o->Type == MODEL_HELPER + 97 || o->Type == MODEL_HELPER + 98 || o->Type == MODEL_POTION + 91)
    {
        o->Angle[0] = 270.0f;
        o->Scale = 1.0f;
    }
    else if (o->Type == MODEL_HELPER + 99)
    {
        o->Angle[0] = 270.0f;
        o->Scale = 1.0f;
    }
    else if (o->Type == MODEL_POTION + 110 || o->Type == MODEL_POTION + 111)
    {
        o->Angle[0] = 270.0f;
        o->Scale = 1.0f;
    }
    else if (o->Type == MODEL_HELPER + 107)
    {
        o->Angle[0] = 270.0f;
        o->Scale = 1.0f;
    }
    else if (o->Type == MODEL_HELPER + 104)
    {
        o->Angle[0] = 270.0f;
        o->Scale = 1.0f;
    }
    else if (o->Type == MODEL_HELPER + 105)
    {
        o->Angle[0] = 270.0f;
        o->Scale = 1.0f;
    }
    else if (o->Type == MODEL_HELPER + 103)
    {
        o->Angle[0] = 0.0f;
        o->Scale = 1.0f;
    }
    else if (o->Type == MODEL_POTION + 133)
    {
        o->Angle[0] = 270.0f;
        o->Scale = 1.0f;
    }
    else if (o->Type == MODEL_HELPER + 109)
    {
        o->Angle[0] = 270.0f;
        o->Scale = 1.0f;
    }
    else if (o->Type == MODEL_HELPER + 110)
    {
        o->Angle[0] = 270.0f;
        o->Scale = 1.0f;
    }
    else if (o->Type == MODEL_HELPER + 111)
    {
        o->Angle[0] = 270.0f;
        o->Scale = 1.0f;
    }
    else if (o->Type == MODEL_HELPER + 112)
    {
        o->Angle[0] = 270.0f;
        o->Scale = 1.0f;
    }
    else if (o->Type == MODEL_HELPER + 113)
    {
        o->Angle[0] = 270.0f;
        o->Scale = 1.0f;
    }
    else if (o->Type == MODEL_HELPER + 114)
    {
        o->Angle[0] = 270.0f;
        o->Scale = 1.0f;
    }
    else if (o->Type == MODEL_HELPER + 115)
    {
        o->Angle[0] = 270.0f;
        o->Scale = 1.0f;
    }
    else if (o->Type == MODEL_POTION + 112)
    {
        o->Angle[0] = 270.0f;
        o->Scale = 1.0f;
    }
    else if (o->Type == MODEL_POTION + 113)
    {
        o->Angle[0] = 270.0f;
        o->Scale = 1.0f;
    }
    else if (o->Type == MODEL_HELPER + 116)
    {
        o->Scale = 0.5f;
        o->Angle[0] = 90.f;
    }
    else if (o->Type == MODEL_HELPER + 121)
    {
        o->Scale = 0.5f;
        o->Angle[0] = 90.f;
    }
    else if (o->Type == MODEL_PET_SKELETON)
    {
        o->Scale = 0.4f;
        o->Angle[0] = 30.f;
    }
    else if (o->Type >= MODEL_WING && o->Type < MODEL_WING + MAX_ITEM_INDEX)
    {
        {
            o->Angle[0] = 270.f;
            o->Angle[2] = 90.f - 45.f;
        }
    }
    else if (o->Type >= MODEL_HELPER + 135 && o->Type <= MODEL_HELPER + 145)
    {
        o->Scale = 0.2f;
        o->Angle[0] = 90.f;
    }
    else if (o->Type == MODEL_POTION + 160 || o->Type == MODEL_POTION + 161)
    {
        o->Scale = 0.2f;
        o->Angle[0] = 90.f;
    }
    else if (Check_LuckyItem(o->Type - MODEL_ITEM))
    {
        o->Angle[0] = 250.0f;
        o->Angle[1] = 180.0f;
        o->Angle[2] = 45.0f;
    }
}

void CreateItemDrop(ITEM_t* ip, ItemCreationParams params, vec3_t position, bool isFreshDrop)
{
    int Type = params.Group * MAX_ITEM_INDEX + params.Number;
    ITEM* n = &ip->Item;
    n->Type = Type;
    n->Level = params.Level;
    n->HasSkill = params.WithSkill;
    n->HasLuck = params.WithLuck;
    n->OptionLevel = params.OptionLevel;
    n->OptionType = params.OptionType;
    n->Durability = params.Durability;
    n->ExcellentFlags = params.ExcellentFlags;
    n->AncientDiscriminator = params.AncientDiscriminator;
    n->AncientBonusOption = params.AncientBonusOption;
    n->option_380 = params.HasGuardianOption;

    if (isFreshDrop)
    {
        if (Type == ITEM_JEWEL_OF_BLESS || Type == ITEM_JEWEL_OF_SOUL || Type == ITEM_JEWEL_OF_LIFE || Type == ITEM_JEWEL_OF_CHAOS || Type == ITEM_JEWEL_OF_CREATION || Type == ITEM_JEWEL_OF_GUARDIAN)
            PlayBuffer(SOUND_JEWEL01, &ip->Object);
        else if (Type == ITEM_GEMSTONE)
            PlayBuffer(SOUND_JEWEL02, &ip->Object);
        else
            PlayBuffer(SOUND_DROP_ITEM01, &ip->Object);
    }
    

    OBJECT* o = &ip->Object;
    o->Live = true;
    o->Type = MODEL_ITEM + Type;
    o->SubType = 1;
    if (Type == (int)(ITEM_BOX_OF_LUCK))
    {
        switch (n->Level)
        {
        case 1:
            o->Type = MODEL_EVENT + 4;
            break;
        case 2:
            o->Type = MODEL_EVENT + 5;
            break;
        case 3:
            o->Type = MODEL_EVENT + 6;
            break;
        case 5:
            o->Type = MODEL_EVENT + 8;
            break;
        case 6:
            o->Type = MODEL_EVENT + 9;
            break;
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
            o->Type = MODEL_EVENT + 10;
            break;
        case 13:
            o->Type = MODEL_EVENT + 6;
            break;
        case 14:
        case 15:
            o->Type = MODEL_EVENT + 5;
        }
    }
    if (Type >= ITEM_JACK_OLANTERN_BLESSINGS && Type <= ITEM_JACK_OLANTERN_CRY)
    {
        o->Type = MODEL_ITEM + Type;
    }
    if (Type >= ITEM_PINK_CHOCOLATE_BOX && Type <= ITEM_BLUE_CHOCOLATE_BOX)
    {
        if (n->Level == 1)
        {
            int Num = Type - (ITEM_PINK_CHOCOLATE_BOX);
            o->Type = MODEL_EVENT + 21 + Num;
        }
    }
    else if (Type == (int)(ITEM_POTION + 21))
    {
        switch (n->Level)
        {
        case 1:
        case 2:
            o->Type = MODEL_EVENT + 11;
            break;
        }
    }
    else if (Type == (int)(ITEM_WEAPON_OF_ARCHANGEL))
    {
        switch (n->Level)
        {
        case 0:
            o->Type = MODEL_DIVINE_STAFF_OF_ARCHANGEL;
            n->Level = 0;
            break;
        case 1:
            o->Type = MODEL_DIVINE_SWORD_OF_ARCHANGEL;
            n->Level = 0;
            break;
        case 2:
            o->Type = MODEL_DIVINE_CB_OF_ARCHANGEL;
            n->Level = 0;
            break;
        }
    }
    else if (Type == ITEM_SCROLL_OF_EMPEROR_RING_OF_HONOR)
    {
        switch (n->Level)
        {
        case 1:
            o->Type = MODEL_EVENT + 12;
            break;
            break;
        }
    }
    else if (Type == ITEM_BROKEN_SWORD_DARK_STONE)
    {
        switch (n->Level)
        {
        case 1:
            o->Type = MODEL_EVENT + 13;
            break;
        }
    }
    else if (Type == ITEM_WIZARDS_RING)
    {
        switch (n->Level)
        {
        case 0:
            o->Type = MODEL_EVENT + 15;
            break;
        case 1:
        case 2:
        case 3:
            o->Type = MODEL_EVENT + 14;
            break;
        }
    }
    else if (Type == (int)(ITEM_ALE))
    {
        switch (n->Level)
        {
        case 1:
            o->Type = MODEL_EVENT + 7;
            break;
        }
    }
    else if (Type == ITEM_LOCHS_FEATHER && n->Level == 1)
    {
        o->Type = MODEL_EVENT + 16;
    }
    else if (Type == ITEM_LIFE_STONE_ITEM && n->Level == 1)
    {
        o->Type = MODEL_EVENT + 18;
    }
    else if (Type == ITEM_GEMSTONE)
    {
        o->Type = MODEL_GEMSTONE;
    }
    else if (Type == ITEM_JEWEL_OF_HARMONY)
    {
        o->Type = MODEL_JEWEL_OF_HARMONY;
    }
    else if (Type == ITEM_LOWER_REFINE_STONE)
    {
        o->Type = MODEL_LOWER_REFINE_STONE;
    }
    else if (Type == ITEM_HIGHER_REFINE_STONE)
    {
        o->Type = MODEL_HIGHER_REFINE_STONE;
    }
    ItemObjectAttribute(o);
    Vector(-30.f, -30.f, -30.f, o->BoundingBoxMin);
    Vector(30.f, 30.f, 30.f, o->BoundingBoxMax);
    VectorCopy(position, o->Position);
    if (isFreshDrop)
    {
        if (o->Type == MODEL_EVENT + 8 || o->Type == MODEL_EVENT + 9)
        {
            short  scale = 55;
            vec3_t Angle;
            vec3_t light;
            Vector(1.f, 1.f, 1.f, light);
            Vector(0.f, 0.f, 0.f, Angle);
            vec3_t NewPosition;
            VectorCopy(position, NewPosition);
            NewPosition[2] = RequestTerrainHeight(position[0], position[1]) + 3;
            CreateEffect(MODEL_SKILL_FURY_STRIKE + 6, NewPosition, Angle, light, 0, o, scale);
            CreateEffect(MODEL_SKILL_FURY_STRIKE + 4, NewPosition, Angle, light, 0, o, scale);
            //CreateEffect(MODEL_SKILL_FURY_STRIKE+5,NewPosition,Angle,light,0,o,scale);

            o->Position[2] = RequestTerrainHeight(o->Position[0], o->Position[1]) + 3.f;
            o->Gravity = 50.f;
        }
        else
        {
            o->Position[2] = RequestTerrainHeight(o->Position[0], o->Position[1]) + 180.f;
            o->Gravity = 20.f;
        }
    }
    else
    {
        o->Position[2] = RequestTerrainHeight(o->Position[0], o->Position[1]);
    }

    ItemAngle(o);
}

void CreateMoneyDrop(ITEM_t* ip, int amount, vec3_t position, bool isFreshDrop)
{
    int Type = ITEM_ZEN;
    ITEM* n = &ip->Item;
    n->Type = Type;
    n->Level = amount;
    n->Durability = 0;
    n->ExcellentFlags = 0;
    n->AncientDiscriminator = 0;
    if (isFreshDrop)
    {
        PlayBuffer(SOUND_DROP_GOLD01);
    }

    OBJECT* o = &ip->Object;
    o->Live = true;
    o->Type = MODEL_ITEM + Type;
    o->SubType = 1;
    
    ItemObjectAttribute(o);
    Vector(-30.f, -30.f, -30.f, o->BoundingBoxMin);
    Vector(30.f, 30.f, 30.f, o->BoundingBoxMax);
    VectorCopy(position, o->Position);
    if (isFreshDrop)
    {
        o->Position[2] = RequestTerrainHeight(o->Position[0], o->Position[1]) + 180.f;
        o->Gravity = 20.f;
    }
    else
    {
        o->Position[2] = RequestTerrainHeight(o->Position[0], o->Position[1]);
    }

    ItemAngle(o);
}

void CreateShiny(OBJECT* o)
{
    vec3_t p, Position;
    if (o->SubType++ % 48 == 0)
    {
        float Matrix[3][4];
        AngleMatrix(o->Angle, Matrix);
        Vector((float)(rand() % 32 + 16), 0.f, (float)(rand() % 32 + 16), p);
        VectorRotate(p, Matrix, Position);
        VectorAdd(o->Position, Position, Position);
        vec3_t Light;
        Vector(1.f, 1.f, 1.f, Light);

        CreateParticle(BITMAP_SHINY, Position, o->Angle, Light);
        CreateParticle(BITMAP_SHINY, Position, o->Angle, Light, 1);
    }
}

void MoveItems()
{
    for (int i = 0; i < MAX_ITEMS; i++)
    {
        OBJECT* o = &Items[i].Object;
        if (o->Live)
        {
            o->Position[2] += o->Gravity * FPS_ANIMATION_FACTOR;
            o->Gravity -= 6.f;
            float Height = RequestTerrainHeight(o->Position[0], o->Position[1]) + 30.f;
            if (o->Type >= MODEL_SWORD && o->Type < MODEL_STAFF + MAX_ITEM_INDEX)
                Height += 40.f;
            if (o->Position[2] <= Height)
            {
                o->Position[2] = Height;
                ItemAngle(o);
            }
            else
            {
                if (o->Type >= MODEL_SHIELD && o->Type < MODEL_SHIELD + MAX_ITEM_INDEX)
                    o->Angle[1] = -o->Gravity * 10.f * FPS_ANIMATION_FACTOR;
                else
                    o->Angle[0] = -o->Gravity * 10.f * FPS_ANIMATION_FACTOR;
            }

            if (rand_fps_check(1))
            {
                CreateShiny(o);
            }
        }
    }
}

void ItemHeight(int Type, BMD* b)
{
    if (Type >= MODEL_HELM && Type < MODEL_HELM + MAX_ITEM_INDEX)
        b->BodyHeight = -160.f;
    else if (Type >= MODEL_ARMOR && Type < MODEL_ARMOR + MAX_ITEM_INDEX)
        b->BodyHeight = -100.f;
    else if (Type >= MODEL_GLOVES && Type < MODEL_GLOVES + MAX_ITEM_INDEX)
        b->BodyHeight = -70.f;
    else if (Type >= MODEL_PANTS && Type < MODEL_PANTS + MAX_ITEM_INDEX)
        b->BodyHeight = -50.f;
    else if (Type >= MODEL_BOOTS && Type < MODEL_BOOTS + MAX_ITEM_INDEX)
        b->BodyHeight = 0.f;
    else
        b->BodyHeight = 0.f;
}

void RenderZen(int itemIndex, ITEM_t* item, vec3_t light)
{
    auto o = &item->Object;
    auto k = itemIndex;
    vec3_t tempPosition;
    VectorCopy(o->Position, tempPosition);

    int coinCount = static_cast<int>(sqrtf(static_cast<float>(Items[k].Item.Level))) / 2;

    coinCount = max(min(coinCount, 80), 3);

    vec3_t randomRadius;
    vec3_t randomAngle;
    vec3_t randomPosition;
    float angleMatrix[3][4];

    BMD* b = &Models[MODEL_ZEN];
    b->BodyScale = o->Scale;

    BoneScale = 1.f;
    BodyLight(o, b);

    constexpr auto alpha = 1.0f;
    b->BeginRender(alpha);
    b->BeginRenderCoinHeap();
    int target_vertex_index = -1;
    for (int i = 0; i < coinCount; ++i)
    {
        // Get a random angle
        Vector(0.f, 0.f, static_cast<float>(RandomTable[(k * 20 + i) % 100] % 360), randomAngle);

        // And a random radius
        const auto maxRadius = coinCount + 20;
        Vector(static_cast<float>(RandomTable[(k + i) % 100] % maxRadius), 0.f, 0.f, randomRadius);

        // Calculate the position based on the random angle and radius
        AngleMatrix(randomAngle, angleMatrix);
        VectorRotate(randomRadius, angleMatrix, randomPosition);

        VectorAdd(tempPosition, randomPosition, o->Position);
        VectorCopy(o->Position, b->BodyOrigin);
        b->Transform(BoneTransform, o->BoundingBoxMin, o->BoundingBoxMax, &o->OBB, true);

        target_vertex_index = b->AddToCoinHeap(i, target_vertex_index);
    }

    b->EndRenderCoinHeap(coinCount);
    b->EndRender();

    VectorCopy(tempPosition, o->Position);
}

void RenderItems()
{
    for (int i = 0; i < MAX_ITEMS; i++)
    {
        OBJECT* o = &Items[i].Object;
        if (o->Live)
        {
            o->Visible = TestFrustrum(o->Position, 400.f);

            if (o->Visible)
            {
                int Type = o->Type;
                if (o->Type >= MODEL_HELM && o->Type < MODEL_BOOTS + MAX_ITEM_INDEX)
                    Type = MODEL_PLAYER;
                else if (o->Type == MODEL_POTION + 12)
                {
                    int Level = Items[i].Item.Level;
                    if (Level == 0)
                        Type = MODEL_EVENT;
                    else if (Level == 2)
                        Type = MODEL_EVENT + 1;
                }
                BMD* b = &Models[Type];
                b->CurrentAction = 0;
                b->Skin = gCharacterManager.GetBaseClass(Hero->Class); // ???
                b->CurrentAction = o->CurrentAction;
                VectorCopy(o->Position, b->BodyOrigin);
                ItemHeight(o->Type, b);
                b->Animation(BoneTransform, o->AnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle, false, false);

                if (o->Type >= MODEL_HELM && o->Type < MODEL_BOOTS + MAX_ITEM_INDEX)
                    Type = o->Type;
                b = &Models[Type];
                vec3_t Light;
                RequestTerrainLight(o->Position[0], o->Position[1], Light);
                VectorAdd(Light, o->Light, Light);
                if (o->Type == MODEL_ZEN) // Zen
                {
                    RenderZen(i, &Items[i], Light);
                }
                else if (o->Type == MODEL_CAPE_OF_OVERRULE)
                {
                    Vector(1.0f, 1.0f, 1.0f, Light);
                }

                vec3_t vBackup;
                VectorCopy(o->Position, vBackup);
                if (gMapManager.WorldActive == WD_10HEAVEN)
                {
                    o->Position[2] += 10.0f * (float)sinf((float)(i * 1237 + WorldTime) * 0.002f);
                }
                else if (gMapManager.WorldActive == WD_39KANTURU_3RD && g_Direction.m_CKanturu.IsMayaScene())
                {
                    o->Position[2] += 10.0f * (float)sinf((float)(i * 1237 + WorldTime) * 0.002f);
                }
                else if (gMapManager.InHellas() == true)
                {
                    o->Position[2] = GetWaterTerrain(o->Position[0], o->Position[1]) + 180;
                }

                RenderPartObject(o, o->Type, NULL, Light, o->Alpha, Items[i].Item.Level, Items[i].Item.ExcellentFlags, Items[i].Item.AncientDiscriminator, true, true, true);
                VectorCopy(vBackup, o->Position);

                vec3_t Position;
                VectorCopy(o->Position, Position);
                Position[2] += 30.f;
                int ScreenX, ScreenY;
                Projection(Position, &ScreenX, &ScreenY);
                o->ScreenX = ScreenX;
                o->ScreenY = ScreenY;
            }
        }
    }
}

void PartObjectColor(int Type, float Alpha, float Bright, vec3_t Light, bool ExtraMon)
{
    int Color = 0;

    if (ExtraMon && (Type == MODEL_BALROG || Type == MODEL_BILL_OF_BALROG))
    {
        Color = 8;
    }
    else if (Type == MODEL_BALROG || Type == MODEL_BILL_OF_BALROG)
    {
        Color = 1;
    }
    else if (Type == MODEL_VALKYRIE || Type == MODEL_SILVER_BOW || Type == MODEL_BLUEWING_CROSSBOW)
    {
        Color = 5;
    }
    else if (Type == MODEL_LIGHTING_SWORD || Type == MODEL_LEGENDARY_STAFF || (Type >= MODEL_TEAR_OF_ELF && Type < MODEL_POTION + 27))
    {
        Color = 2;
    }
    else if (Type == MODEL_CELESTIAL_BOW || Type == MODEL_GREAT_REIGN_CROSSBOW)
    {
        Color = 9;
    }
    else if (Type == MODEL_ORB_OF_GREATER_FORTITUDE)
    {
        Color = 2;
    }
    else if (Type == MODEL_DIVINE_CB_OF_ARCHANGEL)
    {
        Color = 10;
    }
    else if (Type == MODEL_DRAGON_SOUL_STAFF)
    {
        Color = 5;
    }
    else if (Type == MODEL_RUNE_BLADE)
    {
        Color = 10;
    }
    else if (Type == MODEL_ELEMENTAL_SHIELD)
    {
        Color = 6;
    }
    else if (Type == MODEL_DRAGON_SPEAR)
    {
        Color = 9;
    }
    else if (Type == MODEL_BATTLE_SCEPTER)
    {
        Color = 9;
    }
    else if (Type == MODEL_MASTER_SCEPTER)
    {
        Color = 10;
    }
    else if (Type == MODEL_GREAT_SCEPTER)
    {
        Color = 12;
    }
    else if (Type == MODEL_KNIGHT_BLADE)
    {
        Color = 10;
    }
    else if (Type == MODEL_DARK_REIGN_BLADE)
    {
        Color = 5;
    }
    else if (Type == MODEL_ARROW_VIPER_BOW)
    {
        Color = 16;
    }
    else if (Type == MODEL_STAFF_OF_KUNDUN)
    {
        Color = 17;
    }
    else if (Type == MODEL_GREAT_LORD_SCEPTER)
    {
        Color = 16;
    }
    else if (Type == MODEL_BONE_BLADE)
    {
        Color = 18;
    }
    else if (Type == MODEL_GRAND_VIPER_STAFF)
    {
        Color = 19;
    }
    else if (Type == MODEL_SYLPH_WIND_BOW)
    {
        Color = 20;
    }
    else if (Type == MODEL_EXPLOSION_BLADE)
    {
        Color = 23;
    }
    else if (Type == MODEL_SOLEIL_SCEPTER)
    {
        Color = 22;
    }
    else if (Type == MODEL_DAYBREAK)
    {
        Color = 24;
    }
    else if (Type == MODEL_PLATINA_STAFF)
    {
        Color = 25;
    }
    else if (Type == MODEL_ALBATROSS_BOW)
    {
        Color = 26;
    }
    else if (Type == MODEL_SHINING_SCEPTER)
    {
        Color = 28;
    }
    else if (Type == MODEL_SWORD_DANCER)
    {
        Color = 27;
    }
    else if (Type == MODEL_MISTERY_STICK)
        Color = 24;
    else if (Type == MODEL_VIOLENT_WIND_STICK)
        Color = 15;
    else if (Type == MODEL_RED_WING_STICK)
        Color = 1;
    else if (Type == MODEL_ANCIENT_STICK)
        Color = 3;
    else if (Type == MODEL_DEMONIC_STICK)
        Color = 30;
    else if (Type == MODEL_STORM_BLITZ_STICK)
        Color = 21;
    else if (Type == MODEL_ETERNAL_WING_STICK)
        Color = 5;
    else if (Type == MODEL_BOOK_OF_NEIL)
        Color = 1;
    else if (Type == MODEL_IMPERIAL_SWORD)
        Color = 8;
    else if (Type == MODEL_DEADLY_STAFF)
        Color = 1;
    else if (Type == MODEL_IMPERIAL_STAFF)
        Color = 19;
    else if (Type == MODEL_ABSOLUTE_SCEPTER)
        Color = 40;
    else if (Type == MODEL_FROST_BARRIER)
        Color = 29;
    else if (Type == MODEL_STINGER_BOW)
        Color = 35;
    else if (Type == MODEL_GUARDIAN_SHILED)
        Color = 36;
    else if (Type == MODEL_CROSS_SHIELD)
        Color = 30;
    else if (Type == MODEL_BEUROBA)
        Color = 20;
    else if (Type == MODEL_CHROMATIC_STAFF)
        Color = 43;
    else if (Type == MODEL_RAVEN_STICK)
        Color = 5;
    else if (Type == MODEL_STRYKER_SCEPTER)
        Color = 5;
    else if (Type == MODEL_AIR_LYN_BOW)
        Color = 36;
    else if (g_CMonkSystem.EqualItemModelType(Type) == MODEL_SACRED_GLOVE)
        Color = 16;
    else if (g_CMonkSystem.EqualItemModelType(Type) == MODEL_STORM_HARD_GLOVE)
        Color = 42;
    else if (g_CMonkSystem.EqualItemModelType(Type) == MODEL_PIERCING_BLADE_GLOVE)
        Color = 18;
    else if (g_CMonkSystem.EqualItemModelType(Type) == MODEL_PHOENIX_SOUL_STAR)
        Color = 45;
    else if (Type == MODEL_ARMORINVEN_60)
        Color = 16;
    else if (Type == MODEL_ARMORINVEN_61)
        Color = 42;
    else if (Type == MODEL_ARMORINVEN_62)
        Color = 18;
    else if (Type == MODEL_ARMORINVEN_74)
        Color = 45;
    else//  if ( Type<MODEL_WING )
    {
        int ItemType = Type - MODEL_ITEM;
        if (ItemType / MAX_ITEM_INDEX >= 7 && ItemType / MAX_ITEM_INDEX <= 11)
        {
            switch (ItemType % MAX_ITEM_INDEX)
            {
            case 1:Color = 1; break;
            case 9:Color = 2; break;
            case 12:Color = 2; break;
            case 3:Color = 3; break;
            case 13:Color = 4; break;
            case 4:Color = 5; break;
            case 14:Color = 5; break;
            case 6:Color = 6; break;
            case 15:Color = 7; break;
            case 16:Color = 10; break;
            case 17:Color = 9; break;
            case 18:Color = 5; break;
            case 19:Color = 9; break;
            case 20:Color = 9; break;
            case 21:Color = 16; break;
            case 22:Color = 17; break;
            case 23:Color = 11; break;
            case 24:Color = 16; break;
            case 25:Color = 11; break;
            case 26:Color = 12; break;
            case 27:Color = 10; break;
            case 28:Color = 15; break;
            case 29:Color = 18; break;
            case 30:Color = 19; break;
            case 31:Color = 20; break;
            case 32:Color = 21; break;
            case 33:Color = 22; break;
            case 34:Color = 24; break;
            case 35:Color = 25; break;
            case 36:Color = 26; break;
            case 37:Color = 27; break;
            case 38:Color = 28; break;
            case 39:Color = 29; break;
            case 40:Color = 30; break;
            case 41:Color = 31; break;
            case 42:Color = 32; break;
            case 43:Color = 33; break;
            case 44:Color = 34; break;
            case 45:Color = 36; break;
            case 46:Color = 42; break;
            case 47:Color = 37; break;
            case 48:Color = 1; break;
            case 49:Color = 35; break;
            case 50:Color = 39; break;
            case 51:Color = 40; break;
            case 52:Color = 36; break;
            case 53:Color = 41; break;
            case 59:Color = 16; break;
            case 60:Color = 42; break;
            case 61:Color = 18; break;
            case 73:Color = 45; break;
            }
        }
    }
    Bright *= pow(Alpha, FPS_ANIMATION_FACTOR);
    switch (Color)
    {
    case 0:Vector(Bright * 1.0f, Bright * 0.5f, Bright * 0.0f, Light); break;
    case 1:Vector(Bright * 1.0f, Bright * 0.2f, Bright * 0.0f, Light); break;
    case 2:Vector(Bright * 0.0f, Bright * 0.5f, Bright * 1.0f, Light); break;
    case 3:Vector(Bright * 0.0f, Bright * 0.5f, Bright * 1.0f, Light); break;
    case 4:Vector(Bright * 0.0f, Bright * 0.8f, Bright * 0.4f, Light); break;
    case 5:Vector(Bright * 1.0f, Bright * 1.0f, Bright * 1.0f, Light); break;
    case 6:Vector(Bright * 0.6f, Bright * 0.8f, Bright * 0.4f, Light); break;
    case 7:Vector(Bright * 0.9f, Bright * 0.8f, Bright * 1.0f, Light); break;
    case 8:Vector(Bright * 0.8f, Bright * 0.8f, Bright * 1.0f, Light); break;
    case 9:Vector(Bright * 0.5f, Bright * 0.5f, Bright * 0.8f, Light); break;
    case 10:Vector(Bright * 0.75f, Bright * 0.65f, Bright * 0.5f, Light); break;
    case 11:Vector(Bright * 0.35f, Bright * 0.35f, Bright * 0.6f, Light); break;
    case 12:Vector(Bright * 0.47f, Bright * 0.67f, Bright * 0.6f, Light); break;
    case 13:Vector(Bright * 0.0f, Bright * 0.3f, Bright * 0.6f, Light); break;
    case 14:Vector(Bright * 0.65f, Bright * 0.65f, Bright * 0.55f, Light); break;
    case 15:Vector(Bright * 0.2f, Bright * 0.3f, Bright * 0.6f, Light); break;
    case 16:Vector(Bright * 0.8f, Bright * 0.46f, Bright * 0.25f, Light); break;
    case 17:Vector(Bright * 0.65f, Bright * 0.45f, Bright * 0.3f, Light); break;
    case 18:Vector(Bright * 0.5f, Bright * 0.4f, Bright * 0.3f, Light); break;
    case 19:Vector(Bright * 0.37f, Bright * 0.37f, Bright * 1.0f, Light); break;
    case 20:Vector(Bright * 0.3f, Bright * 0.7f, Bright * 0.3f, Light); break;
    case 21:Vector(Bright * 0.5f, Bright * 0.4f, Bright * 1.0f, Light); break;
    case 22:Vector(Bright * 0.45f, Bright * 0.45f, Bright * 0.23f, Light); break;
    case 23:Vector(Bright * 0.3f, Bright * 0.3f, Bright * 0.45f, Light); break;
    case 24:Vector(Bright * 0.6f, Bright * 0.5f, Bright * 0.2f, Light); break;
    case 25:Vector(Bright * 0.6f, Bright * 0.6f, Bright * 0.6f, Light); break;
    case 26:Vector(Bright * 0.3f, Bright * 0.7f, Bright * 0.3f, Light); break;
    case 27:Vector(Bright * 0.5f, Bright * 0.6f, Bright * 0.7f, Light); break;
    case 28:Vector(Bright * 0.45f, Bright * 0.45f, Bright * 0.23f, Light); break;
    case 29:Vector(Bright * 0.2f, Bright * 0.7f, Bright * 0.3f, Light); break;
    case 30:Vector(Bright * 0.7f, Bright * 0.3f, Bright * 0.3f, Light); break;
    case 31:Vector(Bright * 0.7f, Bright * 0.5f, Bright * 0.3f, Light); break;
    case 32:Vector(Bright * 0.5f, Bright * 0.2f, Bright * 0.7f, Light); break;
    case 33:Vector(Bright * 0.8f, Bright * 0.4f, Bright * 0.6f, Light); break;
    case 34:Vector(Bright * 0.6f, Bright * 0.4f, Bright * 0.8f, Light); break;
    case 35:Vector(Bright * 0.7f, Bright * 0.4f, Bright * 0.4f, Light); break;
    case 36:Vector(Bright * 0.5f, Bright * 0.5f, Bright * 0.7f, Light); break;
    case 37:Vector(Bright * 0.7f, Bright * 0.5f, Bright * 0.7f, Light); break;
    case 38:Vector(Bright * 0.2f, Bright * 0.4f, Bright * 0.7f, Light); break;
    case 39:Vector(Bright * 0.3f, Bright * 0.6f, Bright * 0.4f, Light); break;
    case 40:Vector(Bright * 0.7f, Bright * 0.2f, Bright * 0.2f, Light); break;
    case 41:Vector(Bright * 0.7f, Bright * 0.2f, Bright * 0.7f, Light); break;
    case 42:Vector(Bright * 0.8f, Bright * 0.4f, Bright * 0.0f, Light); break;
    case 43:Vector(Bright * 0.8f, Bright * 0.6f, Bright * 0.2f, Light); break;
    case 44:Vector(Bright * 0.8f, Bright * 0.7f, Bright * 0.4f, Light); break;
    case 45:Vector(Bright * 0.5f, Bright * 0.8f, Bright * 0.9f, Light); break;
    }
}

void PartObjectColor2(int Type, float Alpha, float Bright, vec3_t Light, bool ExtraMon)
{
    int Color = 0;
    if (Type == MODEL_SILVER_BOW || Type == MODEL_BLUEWING_CROSSBOW)
    {
        Color = 2;
    }
    else if (Type == MODEL_LIGHTING_SWORD || Type == MODEL_LEGENDARY_STAFF)
    {
        Color = 2;
    }
    else if (Type == MODEL_THUNDER_BLADE)
    {
        Color = 0;
    }
    else if (Type == MODEL_CELESTIAL_BOW)
    {
        Color = 0;
    }
    else if (Type == MODEL_DRAGON_SOUL_STAFF)
    {
        Color = 0;
    }
    else if (Type == MODEL_ARMORINVEN_60
        || Type == MODEL_ARMORINVEN_61
        || Type == MODEL_ARMORINVEN_62)
    {
        Color = 0;
    }
    else
    {
        int ItemType = Type - MODEL_ITEM;
        if (ItemType / MAX_ITEM_INDEX >= 7 && ItemType / MAX_ITEM_INDEX <= 11)
        {
            switch (ItemType % MAX_ITEM_INDEX)
            {
            case 0:Color = 0; break;
            case 1:Color = 0; break;
            case 2:Color = 0; break;
            case 3:Color = 0; break;
            case 4:Color = 1; break;
            case 5:Color = 0; break;
            case 6:Color = 0; break;
            case 7:Color = 0; break;
            case 8:Color = 0; break;
            case 9:Color = 0; break;
            case 10:Color = 0; break;
            case 11:Color = 0; break;
            case 12:Color = 0; break;
            case 13:Color = 0; break;
            case 14:Color = 1; break;
            case 15:Color = 1; break;
            case 16:Color = 0; break;
            case 17:Color = 1; break;
            case 18:Color = 2; break;
            case 19:Color = 0; break;
            case 21:Color = 3; break;
            case 39:Color = 1; break;
            case 40:Color = 1; break;
            case 41:Color = 1; break;
            case 42:Color = 1; break;
            case 43:Color = 2; break;
            case 44:Color = 3; break;
            case 45:Color = 0; break;
            case 59:Color = 0; break;
            case 60:Color = 0; break;
            case 61:Color = 0; break;
            }
        }
    }
    Bright *= pow(Alpha, FPS_ANIMATION_FACTOR);
    switch (Color)
    {
    case 0: Vector(Bright * 1.0f * Light[0], Bright * 1.0f * Light[1], Bright * 1.0f * Light[2], Light); break;
    case 1: Vector(Bright * 1.0f * Light[0], Bright * 0.5f * Light[1], Bright * 0.0f * Light[2], Light); break;
    case 2: Vector(Bright * 0.0f * Light[0], Bright * 0.5f * Light[1], Bright * 1.0f * Light[2], Light); break;
    case 3: Vector(1.f, 1.f, 1.f, Light);	//
    }
}

void PartObjectColor3(int Type, float Alpha, float Bright, vec3_t Light, bool ExtraMon)
{
    int Color = 0;
    int ItemType = Type - MODEL_ITEM;

    if (ItemType / MAX_ITEM_INDEX >= 7 && ItemType / MAX_ITEM_INDEX <= 11)
    {
        switch (ItemType % MAX_ITEM_INDEX)
        {
        case 0:Color = 0; break;
        case 1:Color = 0; break;
        case 2:Color = 0; break;
        case 3:Color = 1; break;
        case 4:Color = 0; break;
        case 5:Color = 0; break;
        case 6:Color = 0; break;
        case 7:Color = 0; break;
            //		case 7 :Color=44;break;
        case 8:Color = 0; break;
        case 9:Color = 1; break;
        case 10:Color = 0; break;
        case 11:Color = 0; break;
        case 12:Color = 0; break;
        case 13:Color = 0; break;
        case 14:Color = 0; break;
        case 15:Color = 0; break;
        case 16:Color = 0; break;
        case 17:Color = 1; break;
        case 18:Color = 0; break;
        case 19:Color = 0; break;
        }
    }
    switch (Color)
    {
    case 0: Vector(0.1f, 0.6f, 1.0f, Light); break;
    case 1: Vector(1.f, 0.7f, 0.2f, Light); break;
    }
}

void RenderPartObjectBody(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType)
{
    int		nIndex;
    int		nNum;

    nIndex = int((o->Type + 1 - MODEL_ITEM) / 512.0f);
    nNum = (o->Type - MODEL_ITEM) % 512;

    BOOL bIsNotRendered = FALSE;

    if ((Type == MODEL_STORM_CROW_ARMOR || Type == MODEL_STORM_CROW_GLOVES || Type == MODEL_STORM_CROW_PANTS || Type == MODEL_STORM_CROW_BOOTS))
    {
        b->RenderBody(RENDER_TEXTURE | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh, BITMAP_CHROME + 1);
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
    }
    else if (Type == MODEL_THUNDER_HAWK_ARMOR || Type == MODEL_THUNDER_HAWK_GLOVES || Type == MODEL_THUNDER_HAWK_PANTS || Type == MODEL_THUNDER_HAWK_BOOTS)
    {
        vec3_t Light;
        VectorCopy(b->BodyLight, Light);
        Vector(0.85f * Light[0], 0.85f * Light[1], 1.2f * Light[2], b->BodyLight);
        b->RenderBody(RENDER_TEXTURE | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh, BITMAP_CHROME + 1);
        VectorCopy(Light, b->BodyLight);
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
    }
    else if (Type == MODEL_WINGS_OF_DARKNESS)
    {
        Vector(0.8f, 0.6f, 1.f, b->BodyLight);
        b->RenderBody(RENDER_BRIGHT | RENDER_CHROME, o->Alpha, o->BlendMesh, 0.5f, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh, BITMAP_CHROME + 1);
        Vector(1.f, 1.f, 1.f, b->BodyLight);
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
    }
    else if (Type == MODEL_WING_OF_STORM)
    {
        Vector(1.f, 0.7f, 0.5f, b->BodyLight);
        glColor3fv(b->BodyLight);
        b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);

        static float s_iTexAni = 0;
        s_iTexAni += FPS_ANIMATION_FACTOR;
        if (s_iTexAni > 15)
            s_iTexAni = 0;
        float fU = ((int)s_iTexAni / 4) * 0.25f;
        Vector(0.9f, 0.6f, 0.3f, b->BodyLight);
        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 1, o->BlendMeshLight, fU, o->BlendMeshTexCoordV, o->HiddenMesh);
        Vector(1.f, 1.f, 1.f, b->BodyLight);
    }
    // 	else if( Type==MODEL_WING+37 )	// 시공날개(법사)
    //     {
    // 		Vector(1.f,1.f,1.f,b->BodyLight);
    // 		b->RenderBody(RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
    //     }
    else if (Type == MODEL_WING_OF_RUIN)
    {
        Vector(1.f, 1.f, 1.f, b->BodyLight);
        glColor3fv(b->BodyLight);
        b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        float Luminosity = absf(sinf(WorldTime * 0.001f)) * 0.3f;
        Vector(0.1f + Luminosity, 0.1f + Luminosity, 0.1f + Luminosity, b->BodyLight);
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        Luminosity = absf(sinf(WorldTime * 0.001f)) * 0.8f;
        Vector(0.0f + Luminosity, 0.0f + Luminosity, 0.0f + Luminosity, b->BodyLight);
        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_3RDWING_LAYER);
    }
    else if (Type == MODEL_CAPE_OF_EMPEROR)
    {
        if (b->BodyLight[0] == 1 && b->BodyLight[1] == 1 && b->BodyLight[2] == 1)
        {
            Vector(1.f, 1.f, 1.f, b->BodyLight);
            glColor3fv(b->BodyLight);
            b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        }
        else
        {
            Vector(1.f, 1.f, 1.f, b->BodyLight);
            glColor3fv(b->BodyLight);
            b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        }
    }
    else if (Type == MODEL_WINGS_OF_DESPAIR)
    {
        Vector(1.f, 1.f, 1.f, b->BodyLight);
        glColor3fv(b->BodyLight);
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT | RENDER_CHROME6, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_WING_OF_DIMENSION)
    {
        Vector(1.f, 1.f, 1.f, b->BodyLight);
        glColor3fv(b->BodyLight);
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(1, RENDER_BRIGHT | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_DIVINE_SWORD_OF_ARCHANGEL)
    {
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_METAL, Alpha, 0, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(0, RENDER_LIGHTMAP | RENDER_TEXTURE, Alpha, 0, o->BlendMeshLight, o->BlendMeshTexCoordU, WorldTime * 0.0001f, BITMAP_CHROME);
        b->RenderMesh(1, RENDER_TEXTURE, Alpha, 0, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_DIVINE_STAFF_OF_ARCHANGEL)
    {
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_METAL, Alpha, 0, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(0, RENDER_LIGHTMAP | RENDER_TEXTURE, 1.f, 0, o->BlendMeshLight, o->BlendMeshTexCoordU, -WorldTime * 0.0001f, BITMAP_CHROME);
        b->RenderMesh(1, RENDER_TEXTURE, Alpha, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_DIVINE_CB_OF_ARCHANGEL)
    {
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_METAL, Alpha, 0, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(0, RENDER_LIGHTMAP | RENDER_TEXTURE, 1.f, 0, o->BlendMeshLight, o->BlendMeshTexCoordU, -WorldTime * 0.0001f, BITMAP_CHROME);
        b->RenderMesh(1, RENDER_TEXTURE, Alpha, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_DIVINE_SCEPTER_OF_ARCHANGEL)
    {
        b->RenderMesh(0, RENDER_TEXTURE, Alpha, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(0, RENDER_LIGHTMAP | RENDER_TEXTURE, Alpha, 0, o->BlendMeshLight, o->BlendMeshTexCoordU, -WorldTime * 0.0001f, BITMAP_CHROME);
    }
    else if (Type == MODEL_GREAT_REIGN_CROSSBOW)
    {
        vec3_t Light;
        VectorCopy(b->BodyLight, Light);
        Vector(0.8f * Light[0], 0.f, 0.8f * Light[2], b->BodyLight);
        b->RenderMesh(0, RENDER_LIGHTMAP | RENDER_TEXTURE, 1.f, 0, o->BlendMeshLight, -WorldTime * 0.0002f, o->BlendMeshTexCoordV, BITMAP_CHROME);
        VectorCopy(Light, b->BodyLight);
        b->RenderMesh(0, RENDER_CHROME | RENDER_BRIGHT, 1.f, 0, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_CHROME + 1);
        VectorCopy(Light, b->BodyLight);
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
    }
    else if (Type == MODEL_PUMPKIN_OF_LUCK)
    {
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(1, RENDER_DARK | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, -WorldTime * 0.0002f, o->BlendMeshTexCoordV, BITMAP_CHROME);

        vec3_t vPos, vRelativePos;
        Vector(0.f, 0.f, 0.f, vRelativePos);
        b->TransformPosition(BoneTransform[8], vRelativePos, vPos, true);
        float fLumi = (sinf(WorldTime * 0.004f) + 1.0f) * 0.05f;
        Vector(0.8f + fLumi, 0.8f + fLumi, 0.3f + fLumi, o->Light);
        CreateSprite(BITMAP_LIGHT, vPos, 1.5f, o->Light, o, 0.5f);
        b->TransformPosition(BoneTransform[10], vRelativePos, vPos, true);
        CreateSprite(BITMAP_LIGHT, vPos, 0.5f, o->Light, o, 0.5f);
        b->TransformPosition(BoneTransform[11], vRelativePos, vPos, true);
        CreateSprite(BITMAP_LIGHT, vPos, 0.5f, o->Light, o, 0.5f);
    }
    else if (o->Type >= MODEL_CHAIN_LIGHTNING_PARCHMENT && o->Type <= MODEL_INNOVATION_PARCHMENT)
    {
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        float fLumi = (sinf(WorldTime * 0.0015f) + 1.0f) * 0.5f;
        b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh, BITMAP_ROOLOFPAPER_EFFECT_R);
    }
    else if (Type == MODEL_RUNE_BLADE && !(RenderType & RENDER_DOPPELGANGER))
    {
        vec3_t Light;
        VectorCopy(b->BodyLight, Light);
        b->BeginRender(1.f);
        glColor3f(b->BodyLight[0], b->BodyLight[1], b->BodyLight[2]);
        b->RenderMesh(3, RENDER_TEXTURE, 1.f, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(1, RENDER_TEXTURE, 1.f, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(1, RENDER_TEXTURE, sinf(WorldTime * 0.01f), 1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(0, RENDER_CHROME | RENDER_TEXTURE, 1.f, 0, o->BlendMeshLight, o->BlendMeshTexCoordU, WorldTime * 0.001f, BITMAP_CHROME);

        float Luminosity = sinf(WorldTime * 0.001f) * 0.5f + 0.5f;
        Vector(Light[0] * Luminosity, Light[0] * Luminosity, Light[0] * Luminosity, b->BodyLight);
        b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT, 1.f, 2, o->BlendMeshLight, WorldTime * 0.0001f, -WorldTime * 0.0005f);
        b->EndRender();
    }
    else if (Type == MODEL_DRAGON_SPEAR)
    {
        b->RenderBody(RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(1, RENDER_TEXTURE, 1.f, 1, o->BlendMeshLight, WorldTime * 0.0001f, WorldTime * 0.0005f);
    }
    else if (Type == MODEL_ELEMENTAL_MACE)
    {
        vec3_t Light;
        VectorCopy(b->BodyLight, Light);
        b->RenderBody(RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);

        float time = WorldTime * 0.001f;
        float Luminosity = sinf(time) * 0.5f + 0.3f;
        Vector(Light[0] * Luminosity, Light[0] * Luminosity, Light[0] * Luminosity, b->BodyLight);
        b->RenderMesh(2, RENDER_TEXTURE, 1.f, 2, o->BlendMeshLight, time, -WorldTime * 0.0005f);
    }
    else if (Type == MODEL_DARK_HORSE_ITEM)
    {
        b->RenderBody(RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        Vector(0.8f, 0.4f, 0.1f, b->BodyLight);
        b->RenderBody(RENDER_BRIGHT | RENDER_CHROME, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_DARK_RAVEN_ITEM)
    {
        b->RenderBody(RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        Vector(0.3f, 0.8f, 1.f, b->BodyLight);
        b->RenderMesh(0, RENDER_BRIGHT | RENDER_CHROME, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_BATTLE_SCEPTER)
    {
        o->BlendMeshLight = sinf(WorldTime * 0.001f) * 0.6f + 0.4f;
        b->BeginRender(1.f);
        b->RenderBody(RENDER_TEXTURE, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(0, RENDER_TEXTURE, Alpha, 0, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->EndRender();
    }
    else if (Type == MODEL_MASTER_SCEPTER)
    {
        b->RenderBody(RENDER_TEXTURE, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        Vector(0.1f, 0.3f, 1.f, b->BodyLight);
        o->BlendMesh = 0;
        o->BlendMeshLight = sinf(WorldTime * 0.001f) * 0.6f + 0.4f;
        b->RenderMesh(0, RENDER_TEXTURE, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        Vector(0.6f, 0.8f, 1.f, b->BodyLight);
        o->BlendMesh = 1;
        o->BlendMeshLight = 1.f;
        b->RenderMesh(1, RENDER_TEXTURE, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, WorldTime * 0.0003f, BITMAP_CHROME);
    }
    else if (Type == MODEL_FLAMBERGE)
    {
        //b->RenderBody( RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, 5 );

        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        o->BlendMesh = 1;
        Vector(1.f, 0.f, 0.2f, b->BodyLight);
        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        Vector(1.f, 1.f, 1.f, b->BodyLight);
        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(3, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(4, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        float fV;
        fV = (((int)(WorldTime * 0.05) % 16) / 4) * 0.25f;
        b->RenderMesh(5, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 5, o->BlendMeshLight, o->BlendMeshTexCoordU, fV);
    }
    else if (Type == MODEL_SWORD_BREAKER)
    {
        b->RenderBody(RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_IMPERIAL_SWORD)
    {
        b->RenderBody(RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        o->BlendMesh = 1;
        o->BlendMeshLight = sinf(WorldTime * 0.001f) * 0.6f + 0.4f;
        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_FROST_MACE)
    {
        b->RenderBody(RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, 1);
        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(3, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_DEADLY_STAFF)
    {
        b->RenderBody(RENDER_TEXTURE, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        o->BlendMesh = 1;
        o->BlendMeshLight = 1.f;
        Vector(1.f, 0.5f, 0.5f, b->BodyLight);
        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_IMPERIAL_STAFF)
    {
        b->RenderBody(RENDER_TEXTURE, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        o->BlendMesh = 0;
        o->BlendMeshLight = fabs(sinf(WorldTime * 0.001f)) + 0.1f;
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_SOCKETSTAFF);
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_SOCKETSTAFF);
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_SOCKETSTAFF);
    }
    else if (Type == MODEL_STAFF + 32)
    {
        b->RenderBody(RENDER_TEXTURE, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(3, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(3, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_CRIMSONGLORY)
    {
        b->RenderBody(RENDER_TEXTURE, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_SALAMANDER_SHIELD)
    {
        b->RenderBody(RENDER_TEXTURE, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, 1);
        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_FROST_BARRIER)
    {
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        o->BlendMesh = 2;
        o->BlendMeshLight = absf((sinf(WorldTime * 0.001f)));
        b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_GUARDIAN_SHILED)
    {
        b->RenderBody(RENDER_TEXTURE, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_CROSS_SHIELD)
    {
        glColor3f(1.f, 1.f, 1.f);
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->StreamMesh = 1;
        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, -(int)WorldTime % 2000 * 0.0005f);
        b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type >= MODEL_POTION + 55 && Type <= MODEL_POTION + 57)
    {
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderBody(RENDER_BRIGHT | RENDER_CHROME2, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
    }
    else if (o->Type == MODEL_OLD_SCROLL)
    {
        float sine = float(sinf(WorldTime * 0.002f) * 0.3f) + 0.7f;
        b->RenderBody(RenderType, 0.7f, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, 1);
        b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, 1.0f, 4, sine, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, 0);
    }
    else if (o->Type == MODEL_ILLUSION_SORCERER_COVENANT)
    {
        float sine = float(sinf(WorldTime * 0.00004f) * 0.15f) + 0.5f;
        b->RenderBody(RenderType, 1.f, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, 0);
        b->RenderBody(RenderType, 1.f, 0.5f, sine, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, 1);
    }
    else if (o->Type == MODEL_SCROLL_OF_BLOOD)
    {
        float sine = float(sinf(WorldTime * 0.002f) * 0.3f) + 0.7f;
        b->RenderBody(RenderType, 0.7f, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, 0);
        b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, 1.0f, 0, sine, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, 1);
    }
    else if (Type == MODEL_POTION + 64)
    {
        float fLumi = (sinf(WorldTime * 0.001f) + 1.5f) * 0.25f;

        float PlaySpeed = 0.f;
        PlaySpeed = b->Actions[b->CurrentAction].PlaySpeed;

        b->PlayAnimation(&o->AnimationFrame, &o->PriorAnimationFrame, &o->PriorAction, 2.f / 7.f, o->Position, o->Angle);
        b->RenderBody(RenderType, Alpha, o->BlendMesh, o->BlendMeshLight * 1.5f, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, Alpha, o->BlendMesh, o->BlendMeshLight / 4.0f, WorldTime * 0.005f, -WorldTime * 0.005f, 0);

        vec3_t Light;
        vec3_t vRelativePos, vWorldPos;
        Vector(0.f, 0.f, 0.f, vRelativePos);
        Vector(1.f, 0.f, 0.0f, Light);

        b->TransformPosition(BoneTransform[1], vRelativePos, vWorldPos, true);
        CreateSprite(BITMAP_LIGHT, vWorldPos, 3.f, Light, o, 0.f);
    }
    else if (o->Type == MODEL_FLAME_OF_CONDOR)
    {
        b->RenderBody(RENDER_TEXTURE, 0.9, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderBody(RENDER_BRIGHT | RENDER_CHROME, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (o->Type == MODEL_FEATHER_OF_CONDOR)
    {
        b->RenderBody(RENDER_TEXTURE, 0.9, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (o->Type == MODEL_FLAME_OF_DEATH_BEAM_KNIGHT)
    {
        float fLumi = (sinf(WorldTime * 0.0015f) + 1.5f) * 0.4f;
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight,
            o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, fLumi,
            o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_ITEM_EFFECT_DBSTONE_R);
    }
    else if (o->Type == MODEL_HORN_OF_HELL_MAINE)
    {
        float fLumi = (sinf(WorldTime * 0.0015f) + 1.5f) * 0.4f;
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight,
            o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, fLumi,
            o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_ITEM_EFFECT_HELLHORN_R);
    }
    else if (o->Type == MODEL_FEATHER_OF_DARK_PHOENIX)
    {
        float fLumi = (sinf(WorldTime * 0.0015f) + 1.5f) * 0.4f;
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight,
            o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, fLumi,
            o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_ITEM_EFFECT_PFEATHER_R);
    }
    else if (o->Type == MODEL_EYE_OF_ABYSSAL)
    {
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight,
            o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        float fLumi = (sinf(WorldTime * 0.0015f) + 1.5f) * 0.4f;
        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, Alpha, 1, fLumi,
            o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_ITEM_EFFECT_DEYE_R);
        b->RenderMesh(1, RENDER_CHROME | RENDER_BRIGHT, 0.2f, -1,
            o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type >= MODEL_HELPER + 46 && Type <= MODEL_HELPER + 48)
    {
        float fLumi = (sinf(WorldTime * 0.0015f) + 1.2f) * 0.3f;
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_FREETICKET_R);
    }
    else if (Type == MODEL_POTION + 54)
    {
        float fLumi = (sinf(WorldTime * 0.0015f) + 1.2f) * 0.4f;
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, 1.0f, 0, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, 1);
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_CHAOSCARD_R);
    }
    else if (Type == MODEL_POTION + 58)
    {
        float fLumi = (sinf(WorldTime * 0.0015f) + 1.f) * 0.5f;
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_RAREITEM1_R);
        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_POTION + 59)
    {
        float fLumi = (sinf(WorldTime * 0.0015f) + 1.f) * 0.5f;
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_RAREITEM2_R);
        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_POTION + 60)
    {
        float fLumi = (sinf(WorldTime * 0.0015f) + 1.f) * 0.5f;
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_RAREITEM3_R);
        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_POTION + 61)
    {
        float fLumi = (sinf(WorldTime * 0.0015f) + 1.f) * 0.5f;
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_RAREITEM4_R);
    }
    else if (Type == MODEL_POTION + 62)
    {
        float fLumi = (sinf(WorldTime * 0.0015f) + 1.f) * 0.5f;
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_RAREITEM5_R);
    }
    else if (o->Type == MODEL_POTION + 53)
    {
        float fLumi = (sinf(WorldTime * 0.0015f) + 1.5f) * 0.5f;
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_LUCKY_CHARM_EFFECT53);
    }
    else if (o->Type == MODEL_HELPER + 43
        || o->Type == MODEL_HELPER + 93
        )
    {
        float fLumi = (sinf(WorldTime * 0.001f) + 1.5f) * 0.25f;
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_LUCKY_SEAL_EFFECT43);
        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (o->Type == MODEL_HELPER + 44
        || o->Type == MODEL_HELPER + 94
        || o->Type == MODEL_HELPER + 116
        )
    {
        float fLumi = (sinf(WorldTime * 0.001f) + 1.5f) * 0.25f;
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_LUCKY_SEAL_EFFECT44);
        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (o->Type == MODEL_HELPER + 45)
    {
        float fLumi = (sinf(WorldTime * 0.001f) + 1.5f) * 0.25f;
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_LUCKY_SEAL_EFFECT45);
        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (o->Type >= MODEL_POTION + 70 && o->Type <= MODEL_POTION + 71)
    {
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(0, RENDER_CHROME4, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (o->Type >= MODEL_POTION + 72 && o->Type <= MODEL_POTION + 77)
    {
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
    }
    else if (o->Type == MODEL_HELPER + 59)
    {
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
    }
    else if (o->Type >= MODEL_HELPER + 54 && o->Type <= MODEL_HELPER + 58)
    {
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
    }
    else if (o->Type >= MODEL_POTION + 78 && o->Type <= MODEL_POTION + 82)
    {
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
    }
    else if (o->Type == MODEL_HELPER + 60)
    {
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
    }
    else if (o->Type == MODEL_HELPER + 61)
    {
        float fLumi = (sinf(WorldTime * 0.0015f) + 1.2f) * 0.3f;
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_FREETICKET_R);
    }
    else if (o->Type == MODEL_POTION + 83)
    {
        float fLumi = (sinf(WorldTime * 0.0015f) + 1.f) * 0.5f;
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_RAREITEM5_R);
    }
    else if (o->Type == MODEL_POTION + 145)
    {
        float fLumi = (sinf(WorldTime * 0.0015f) + 1.f) * 0.5f;
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_RAREITEM7);
    }
    else if (o->Type == MODEL_POTION + 146)
    {
        float fLumi = (sinf(WorldTime * 0.0015f) + 1.f) * 0.5f;
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_RAREITEM8);
    }
    else if (o->Type == MODEL_POTION + 147)
    {
        float fLumi = (sinf(WorldTime * 0.0015f) + 1.f) * 0.5f;
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_RAREITEM9);
    }
    else if (o->Type == MODEL_POTION + 148)
    {
        float fLumi = (sinf(WorldTime * 0.0015f) + 1.f) * 0.5f;
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_RAREITEM10);
    }
    else if (o->Type == MODEL_POTION + 149)
    {
        float fLumi = (sinf(WorldTime * 0.0015f) + 1.f) * 0.5f;
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_RAREITEM11);
    }
    else if (o->Type == MODEL_POTION + 150)
    {
        float fLumi = (sinf(WorldTime * 0.0015f) + 1.f) * 0.5f;
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_RAREITEM12);
    }
    else if (Type == MODEL_HELPER + 125)
    {
        float fLumi = (sinf(WorldTime * 0.0015f) + 1.2f) * 0.3f;
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_DOPPLEGANGGER_FREETICKET);
    }
    else if (Type == MODEL_HELPER + 126)
    {
        float fLumi = (sinf(WorldTime * 0.0015f) + 1.2f) * 0.3f;
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_BARCA_FREETICKET);
    }
    else if (Type == MODEL_HELPER + 127)
    {
        float fLumi = (sinf(WorldTime * 0.0015f) + 1.2f) * 0.3f;
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_BARCA7TH_FREETICKET);
    }
    else if (Type == MODEL_POTION + 91)
    {
        float fLumi = (sinf(WorldTime * 0.0015f) + 1.2f) * 0.4f;
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, 1.0f, 0, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, 1);
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_CHARACTERCARD_R);
    }
    else if (Type == MODEL_POTION + 92)
    {
        float fLumi = (sinf(WorldTime * 0.0015f) + 1.2f) * 0.4f;
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, 1.0f, 0, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, 1);
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_NEWCHAOSCARD_GOLD_R);
    }
    else if (Type == MODEL_POTION + 93)
    {
        float fLumi = (sinf(WorldTime * 0.0015f) + 1.2f) * 0.4f;
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, 1.0f, 0, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, 1);
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_NEWCHAOSCARD_RARE_R);
    }
    else if (Type == MODEL_POTION + 95)
    {
        float fLumi = (sinf(WorldTime * 0.0015f) + 1.2f) * 0.4f;
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, 1.0f, 0, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, 1);
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_NEWCHAOSCARD_MINI_R);
    }
    else if (Type == MODEL_POTION + 94)
    {
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_CHERRY_BLOSSOM_PLAYBOX)
    {
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_CHERRY_BLOSSOM_WINE)
    {
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_CHERRY_BLOSSOM_RICE_CAKE)
    {
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_CHERRY_BLOSSOM_FLOWER_PETAL)
    {
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_POTION + 88)
    {
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_POTION + 89)
    {
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_GOLDEN_CHERRY_BLOSSOM_BRANCH)
    {
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type >= MODEL_HELPER + 62 && Type <= MODEL_HELPER + 63)
    {
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
    }
    else if (Type >= MODEL_POTION + 97 && Type <= MODEL_POTION + 98)
    {
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
    }
    else if (Type == MODEL_POTION + 96)
    {
        float fLumi = (sinf(WorldTime * 0.0015f) + 1.5f) * 0.5f;

        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
    }
    else if (Type == MODEL_DEMON)
    {
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
    }
    else if (Type == MODEL_SPIRIT_OF_GUARDIAN)
    {
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
    }
    else if (Type == MODEL_GREAT_SCEPTER && !(RenderType & RENDER_DOPPELGANGER))
    {
        float Luminosity = WorldTime * 0.0005f;

        o->HiddenMesh = 2;
        o->BlendMesh = 2;
        o->BlendMeshLight = 1.f;
        b->BeginRender(1.f);
        b->RenderBody(RENDER_TEXTURE, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(2, RENDER_TEXTURE, Alpha, o->BlendMesh, o->BlendMeshLight, Luminosity, Luminosity);
        b->EndRender();
    }
    else if (Type == MODEL_LORD_SCEPTER)
    {
        o->BlendMeshLight = 1.f;
        o->BlendMeshTexCoordU = WorldTime * 0.0008f;
        b->RenderBody(RENDER_TEXTURE, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        o->BlendMeshLight = sinf(WorldTime * 0.001f) * 0.3f + 0.7f;
        b->RenderMesh(1, RENDER_BRIGHT | RENDER_CHROME, Alpha, 0, o->BlendMeshLight, 0.f, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_KNIGHT_BLADE)
    {
        b->RenderBody(RENDER_TEXTURE, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);

        float Luminosity = sinf(WorldTime * 0.0008f) * 0.7f + 0.5f;
        b->RenderMesh(2, RENDER_TEXTURE, Alpha, 2, Luminosity, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(1, RENDER_TEXTURE, Alpha, 1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        //. 날
        glColor3f(0.43f, 0.14f, 0.6f);

        b->RenderMesh(3, RENDER_BRIGHT | RENDER_CHROME, Alpha, 3, o->BlendMeshLight, WorldTime * 0.0001f, WorldTime * 0.0005f);
        glColor3f(1.f, 1.f, 1.f);
    }
    else if (Type == MODEL_DARK_REIGN_BLADE)
    {
        b->RenderBody(RENDER_TEXTURE, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(0, RENDER_TEXTURE, 1.f, 0, o->BlendMeshLight, WorldTime * 0.0005f, WorldTime * 0.0005f);
        o->HiddenMesh = 0;
        b->StreamMesh = 1;
        b->RenderMesh(1, RENDER_TEXTURE, 1.f, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, WorldTime * 0.0005f);
    }
    else if (Type == MODEL_HURRICANE_ARMOR || Type == MODEL_HURRICANE_GLOVES || Type == MODEL_HURRICANE_PANTS || Type == MODEL_HURRICANE_BOOTS)
    {
        float Luminosity = sinf(WorldTime * 0.002f) * 0.3f + 0.5f;
        vec3_t Light;
        VectorCopy(b->BodyLight, Light);
        Vector(Light[0] * 0.3f, Light[1] * 0.8f, Light[1] * 1.f, b->BodyLight);
        b->RenderBody(RENDER_TEXTURE | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh, BITMAP_CHROME + 1);
        VectorCopy(Light, b->BodyLight);
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
    }
    else if (Type == MODEL_SYLPH_WIND_BOW || Type == MODEL_GRAND_VIPER_STAFF || Type == MODEL_SOLEIL_SCEPTER || Type == MODEL_BONE_BLADE)
    {
        b->BeginRender(1.0f);

        b->StreamMesh = 0;
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        if (Type == MODEL_BONE_BLADE)
        {
            b->BodyLight[0] = 1.0f;
            b->BodyLight[1] = 0.7f;
            b->BodyLight[2] = 0.4f;
            b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, (int)WorldTime % 4000 * 0.0004f - 0.7f, o->BlendMeshTexCoordV, BITMAP_CHROME7);
            b->BodyLight[0] = 0.7f;
            b->BodyLight[1] = 0.7f;
            b->BodyLight[2] = 0.7f;
        }
        else if (Type == MODEL_GRAND_VIPER_STAFF)
            b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, (int)WorldTime % 2000 * 0.0002f - 0.3f, (int)WorldTime % 2000 * 0.0002f - 0.3f, BITMAP_CHROME_ENERGY);
        else
            b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, (int)WorldTime % 4000 * 0.0002f - 0.3f, (int)WorldTime % 4000 * 0.0002f - 0.3f, BITMAP_CHROME6);
        b->StreamMesh = -1;

        b->EndRender();
    }
    else if (Type == MODEL_EXPLOSION_BLADE)
    {
        b->RenderBody(RENDER_TEXTURE, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);

        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, 0, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordU);
        float Luminosity = sinf(WorldTime * 0.0015f) * 0.03f + 0.3f;
        Vector(Luminosity, Luminosity, Luminosity + 0.1f, b->BodyLight);
        b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, -2, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordU);

        o->Alpha = 0.5f;
        float Luminosity4 = sinf(WorldTime * 0.0025f) * 0.5f + 0.7f;
        Vector(0.4f, 0.4f, 0.8f, b->BodyLight);
        b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, -2, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordU);
    }
    else if (Type == MODEL_SYLPHID_RAY_ARMOR)
    {
        if (b->HideSkin == true)
        {
            b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
            b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        }
        else
        {
            b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        }
    }
    else if (Type == MODEL_SYLPHID_RAY_PANTS)
    {
        if (b->HideSkin == true)
        {
            b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
            b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        }
        else
        {
            b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        }
    }
    else if (Type == MODEL_SWORD_DANCER)
    {
        o->HiddenMesh = 1;
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(1, RENDER_TEXTURE, 0.5f, 0, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_LAVA);
        b->RenderMesh(1, RENDER_TEXTURE, 0.7f, 1, o->BlendMeshLight, o->BlendMeshTexCoordU, WorldTime * 0.0009f);
    }
    else if (Type == MODEL_SHINING_SCEPTER)
    {
        b->RenderBody(RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_ALBATROSS_BOW)
    {
        o->HiddenMesh = 1;
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(1, RENDER_TEXTURE, 1.0f, 1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT, 1.0f, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT | RENDER_CHROME5, 0.5f, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_PLATINA_STAFF)
    {
        o->HiddenMesh = 1;
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, 1.0f, 1, o->BlendMeshLight, WorldTime * 0.0009f, WorldTime * 0.0009f);
    }
    else if (Type == MODEL_IRIS_ARMOR)
    {
        if (b->HideSkin == true)
        {
            b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        }
        else
        {
            b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        }
    }
    else if (Type == MODEL_IRIS_PANTS)
    {
        if (b->HideSkin == true)
        {
            b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        }
        else
        {
            b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        }
    }
    else if (Type == MODEL_IRIS_GLOVES)
    {
        if (b->HideSkin == true)
        {
            b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        }
        else
        {
            b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        }
    }
    else if (MODEL_MISTERY_HELM <= Type && MODEL_LILIUM_HELM >= Type && !(RenderType & RENDER_DOPPELGANGER))
    {
        if (b->HideSkin)
        {
            ::glColor3fv(b->BodyLight);
            int anMesh[6] = { 2, 1, 0, 2, 1, 2 };
            b->RenderMesh(anMesh[Type - (MODEL_MISTERY_HELM)], RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        }
        else
            b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
    }
    else if (MODEL_MISTERY_ARMOR <= Type && MODEL_LILIUM_ARMOR >= Type && !(RenderType & RENDER_DOPPELGANGER))
    {
        if (b->HideSkin)
        {
            ::glColor3fv(b->BodyLight);
            int nTexture = Type - (MODEL_MISTERY_ARMOR);
            b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_INVEN_ARMOR + nTexture);
            for (int i = 1; i < b->NumMeshs; ++i)
                b->RenderMesh(i, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        }
        else
            b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
    }
    else if (MODEL_MISTERY_PANTS <= Type && MODEL_LILIUM_PANTS >= Type && !(RenderType & RENDER_DOPPELGANGER))
    {
        if (b->HideSkin)
        {
            ::glColor3fv(b->BodyLight);
            int nTexture = Type - (MODEL_MISTERY_PANTS);
            b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_INVEN_PANTS + nTexture);
            for (int i = 1; i < b->NumMeshs; ++i)
                b->RenderMesh(i, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        }
        else
            b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
    }
    else if (MODEL_LILIUM_GLOVES == Type)
    {
        if (b->HideSkin)
        {
            ::glColor3fv(b->BodyLight);
            b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        }
        else
            b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
    }
#ifdef PBG_ADD_CHARACTERCARD
    else if (MODEL_HELPER + 97 == Type || MODEL_HELPER + 98 == Type || MODEL_POTION + 91 == Type)
    {
        float fLumi = (sinf(WorldTime * 0.0015f) + 1.2f) * 0.4f;
        int _R_Type = 0;
        switch (Type)
        {
        case MODEL_HELPER + 97:
            _R_Type = BITMAP_CHARACTERCARD_R_MA;
            break;
        case MODEL_HELPER + 98:
            _R_Type = BITMAP_CHARACTERCARD_R_DA;
            break;
        case MODEL_POTION + 91:
            _R_Type = BITMAP_CHARACTERCARD_R;
            break;
        default:
            break;
        }
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, _R_Type);
    }
#endif //PBG_ADD_CHARACTERCARD
#ifdef PBG_ADD_CHARACTERSLOT
    else if (MODEL_HELPER + 99 == Type)
    {
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(0, RENDER_BRIGHT | RENDER_CHROME, o->Alpha, 0, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
#endif //PBG_ADD_CHARACTERSLOT
    else
    {
        bIsNotRendered = TRUE;
    }
    if (bIsNotRendered == FALSE);
    else if (Type == MODEL_FAITH_ARMOR || Type == MODEL_FAITH_PANTS || Type == MODEL_ARMOR + 53 || Type == MODEL_PANTS + 53)
    {
        int nTexture = 0;
        switch (Type)
        {
        case MODEL_FAITH_ARMOR:	nTexture = BITMAP_SKIN_ARMOR_DEVINE; break;
        case MODEL_FAITH_PANTS:	nTexture = BITMAP_SKIN_PANTS_DEVINE; break;
        case MODEL_ARMOR + 53:	nTexture = BITMAP_SKIN_ARMOR_SUCCUBUS; break;
        case MODEL_PANTS + 53:	nTexture = BITMAP_SKIN_PANTS_SUCCUBUS; break;
        }
        if (b->HideSkin)
        {
            ::glColor3fv(b->BodyLight);
            b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, nTexture);
            for (int i = 1; i < b->NumMeshs; ++i)
            {
                b->RenderMesh(i, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
            }
        }
        else
        {
            b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        }
    }
    else if (Type == MODEL_SERAPHIM_HELM)
    {
        if (b->HideSkin == true)
        {
            ::glColor3fv(b->BodyLight);
            b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        }
        else
        {
            b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        }
    }
    else if (Type == MODEL_SERAPHIM_ARMOR)
    {
        if (b->HideSkin == true)
        {
            ::glColor3fv(b->BodyLight);
            b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        }
        else
        {
            b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        }
    }
    else if (Type == MODEL_FAITH_HELM)
    {
        if (b->HideSkin == true)
        {
            ::glColor3fv(b->BodyLight);
            b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        }
        else
        {
            b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        }
    }
    else if (Type == MODEL_HELM + 53)
    {
        if (b->HideSkin == true)
        {
            ::glColor3fv(b->BodyLight);
            b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        }
        else
        {
            b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        }
    }
    else if (Type == MODEL_BEUROBA)
    {
        ::glColor3fv(b->BodyLight);
        b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(3, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(4, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(5, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, 0.4f, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 1, 0.8f, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_STRYKER_SCEPTER)
    {
        ::glColor3fv(b->BodyLight);
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(3, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(4, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(5, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(6, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 1, 0.5f, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_ARROW_VIPER_BOW)
    {
        float Luminosity = sinf(WorldTime * 0.002f) * 0.3f + 0.5f;
        b->BeginRender(1.f);
        glColor3f(b->BodyLight[0], b->BodyLight[1], b->BodyLight[2]);
        b->RenderMesh(0, RENDER_TEXTURE, 1.f, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(1, RENDER_TEXTURE, 1.f, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(2, RENDER_TEXTURE, 1.f, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(6, RENDER_TEXTURE, 1.f, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(3, RENDER_TEXTURE, 1.f, 3, Luminosity, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(4, RENDER_TEXTURE, 1.f, 4, Luminosity, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        glColor3f(1.f, 1.f, 1.f);
        b->RenderMesh(5, RENDER_TEXTURE, 1.f, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);

        b->EndRender();
    }
    else if (Type == MODEL_LOST_MAP)
    {
        glColor3f(1.f, 1.f, 1.f);
        Models[o->Type].StreamMesh = 1;
        b->RenderMesh(1, RENDER_TEXTURE, 1.f, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, WorldTime * 0.0005f);
        Models[o->Type].StreamMesh = -1;
        b->RenderMesh(0, RENDER_TEXTURE, 1.f, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_SYMBOL_OF_KUNDUN)
    {
        Vector(1.f, 1.f, 1.f, b->BodyLight);
        b->StreamMesh = 1;
        b->RenderMesh(1, RENDER_TEXTURE, 1.f, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, WorldTime * 0.0005f);
        b->StreamMesh = -1;

        Vector(1.f, 0.5f, 0.f, b->BodyLight);
        static float fMeshLight = 0.500f;
        static float fAdd = 0.01f;
        if (fMeshLight > 1.f) {
            fMeshLight = 1.00f;
            fAdd = -0.01f;
        }
        if (fMeshLight < 0.01f) {
            fMeshLight = 0.01f;
            fAdd = 0.01f;
        }
        b->RenderMesh(2, RENDER_TEXTURE, 1.0f, 2, fMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        fMeshLight += fAdd;

        glColor3f(1.f, 1.f, 1.f);
        Vector(1.f, 1.f, 1.f, b->BodyLight);
        b->RenderMesh(0, RENDER_TEXTURE, 1.f, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(0, RENDER_CHROME | RENDER_BRIGHT, 0.3f, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_STAFF_OF_KUNDUN)
    {
        b->RenderBody(RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(1, RENDER_CHROME | RENDER_BRIGHT, Alpha, 1, 0.2f, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);

        glColor3f(1.f, 1.f, 1.f);
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, Alpha, 0, sinf(WorldTime * 0.005f), o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(0, RENDER_CHROME4 | RENDER_BRIGHT, Alpha, 0, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_DEMONIC_STICK)
    {
        b->RenderBody(RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, Alpha, 1, 1.f, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_STORM_BLITZ_STICK)
    {
        b->RenderBody(RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        float fLumi = (sinf(WorldTime * 0.002f) + 0.5f) * 0.5f;
        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, Alpha, 1, fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_GREAT_LORD_SCEPTER)
    {
        b->RenderBody(RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, Alpha, 1, 1.f, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(3, RENDER_TEXTURE | RENDER_BRIGHT, Alpha, 3, 1.f, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_GRAND_SOUL_SHIELD && !(RenderType & RENDER_DOPPELGANGER))
    {
        b->BeginRender(1.f);

        vec3_t Light;
        VectorCopy(b->BodyLight, Light);
        Vector(Light[0] * 0.3f, Light[1] * 0.3f, Light[2] * 0.3f, b->BodyLight);
        glColor3f(b->BodyLight[0], b->BodyLight[1], b->BodyLight[2]);
        b->RenderMesh(2, RENDER_COLOR, 1.f, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);

        VectorCopy(Light, b->BodyLight);
        glColor3f(b->BodyLight[0], b->BodyLight[1], b->BodyLight[2]);
        b->RenderMesh(2, RENDER_CHROME | RENDER_BRIGHT, 1.f, 2, o->BlendMeshLight, o->BlendMeshTexCoordU, WorldTime * 0.01f, BITMAP_CHROME);
        b->RenderMesh(0, RENDER_TEXTURE, 1.f, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(1, RENDER_TEXTURE, 1.f, 1, o->BlendMeshLight, (float)(rand() % 10) * 0.1f, (float)(rand() % 10) * 0.1f);

        float Luminosity = sinf(WorldTime * 0.001f) * 0.4f + 0.6f;
        Vector(Light[0] * Luminosity, Light[0] * Luminosity, Light[0] * Luminosity, b->BodyLight);
        glColor3f(b->BodyLight[0], b->BodyLight[1], b->BodyLight[2]);
        b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT, 1.f, 2, o->BlendMeshLight, WorldTime * 0.0001f, WorldTime * 0.0005f);
        b->EndRender();
    }
    else if (Type == MODEL_ELEMENTAL_SHIELD)
    {
        b->BeginRender(1.f);
        glColor4f(b->BodyLight[0], b->BodyLight[1], b->BodyLight[2], 0.8f);
        b->RenderMesh(1, RENDER_TEXTURE, 0.8f, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        glColor4f(b->BodyLight[0], b->BodyLight[1], b->BodyLight[2], 0.5f);
        b->RenderMesh(3, RENDER_TEXTURE, 0.5f, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);

        glColor3f(b->BodyLight[0], b->BodyLight[1], b->BodyLight[2]);
        b->RenderMesh(0, RENDER_TEXTURE, 1.f, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(2, RENDER_TEXTURE, 1.f, 2, o->BlendMeshLight, WorldTime * 0.0005f, o->BlendMeshTexCoordV);
        b->RenderMesh(3, RENDER_TEXTURE, 1.f, 3, o->BlendMeshLight, (float)(rand() % 10) * 0.1f, (float)(rand() % 10) * 0.1f);
        b->EndRender();
    }
    else if (Type == MODEL_BATTLE_BOW && (RenderType & RENDER_EXTRA))
    {
        RenderType -= RENDER_EXTRA;
        Vector(0.1f, 0.1f, 0.1f, b->BodyLight);
        b->RenderBody(RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_ANGEL && b->NumMeshs)
    {
        b->RenderBody(RENDER_TEXTURE, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        if (RenderType & RENDER_CHROME)
        {
            Vector(0.75f, 0.55f, 0.5f, b->BodyLight);
            b->RenderMesh(0, RENDER_CHROME4 | RENDER_BRIGHT, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
            Vector(1.f, 1.f, 1.f, b->BodyLight);
            b->RenderMesh(0, RENDER_CHROME | RENDER_TEXTURE | RENDER_BRIGHT, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        }
    }
    else if (Type == MODEL_SIEGE_POTION)
    {
        b->BeginRender(1.f);
        if (o->HiddenMesh == 1)
        {
            glColor3f(1.f, 1.f, 1.f);
            Vector(1.f, 1.f, 1.f, b->BodyLight);
            b->RenderMesh(0, RENDER_TEXTURE, 1.f, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
            Vector(0.1f, 0.5f, 1.f, b->BodyLight);
            b->RenderMesh(0, RENDER_METAL | RENDER_BRIGHT, 1.f, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
            b->RenderMesh(0, RENDER_CHROME | RENDER_BRIGHT, 1.f, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        }
        else if (o->HiddenMesh == 0)
        {
            glColor3f(1.f, 1.f, 1.f);
            Vector(1.f, 1.f, 1.f, b->BodyLight);
            b->RenderMesh(1, RENDER_TEXTURE, 1.f, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
            Vector(0.1f, 0.5f, 1.f, b->BodyLight);
            b->RenderMesh(1, RENDER_METAL | RENDER_BRIGHT, 1.f, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
            b->RenderMesh(1, RENDER_CHROME | RENDER_BRIGHT, 1.f, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        }
        b->EndRender();
    }
    else if (Type == MODEL_HELPER + 7)
    {
        b->BeginRender(1.f);
        if (o->HiddenMesh == 1)
        {
            glColor3f(1.f, 1.f, 1.f);
            Vector(1.f, 1.f, 1.f, b->BodyLight);
            b->RenderMesh(0, RENDER_TEXTURE, 1.f, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        }
        else if (o->HiddenMesh == 0)
        {
            glColor3f(1.f, 1.f, 1.f);
            Vector(1.f, 1.f, 1.f, b->BodyLight);
            b->RenderMesh(1, RENDER_TEXTURE, 1.f, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        }
        b->EndRender();
    }
    else if (Type == MODEL_LIFE_STONE_ITEM)
    {
        b->BeginRender(1.f);
        glColor3f(1.f, 1.f, 1.f);
        Vector(1.f, 1.f, 1.f, b->BodyLight);
        b->RenderMesh(0, RENDER_TEXTURE, 1.f, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        Vector(0.f, 0.5f, 1.f, b->BodyLight);
        b->RenderMesh(1, RENDER_CHROME | RENDER_BRIGHT, 1.f, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->EndRender();
    }
    else if (Type == MODEL_BOLT || Type == MODEL_ARROWS)
    {
        if (g_isCharacterBuff(o, eBuff_InfinityArrow))
        {
            Vector(1.f, 0.8f, 0.2f, b->BodyLight);
            b->RenderBody(RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
            b->RenderBody(RENDER_CHROME | RENDER_BRIGHT, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        }
        else
        {
            b->RenderBody(RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        }
    }
    else if (o->m_bpcroom == TRUE && (Type == MODEL_PLATE_HELM || Type == MODEL_PLATE_ARMOR || Type == MODEL_PLATE_PANTS || Type == MODEL_PLATE_GLOVES || Type == MODEL_PLATE_BOOTS))
    {
        if (Type == MODEL_PLATE_ARMOR)
        {
            vec3_t EndRelative, EndPos;
            Vector(0.f, 0.f, 0.f, EndRelative);

            b->TransformPosition(o->BoneTransform[0], EndRelative, EndPos, true);

            Vector(0.4f, 0.6f, 0.8f, o->Light);
            CreateSprite(BITMAP_LIGHT, EndPos, 6.0f, o->Light, o, 0.5f);

            float Luminosity;
            Luminosity = sinf(WorldTime * 0.05f) * 0.4f + 0.9f;
            Vector(Luminosity * 0.3f, Luminosity * 0.5f, Luminosity * 0.8f, o->Light);
            CreateSprite(BITMAP_LIGHT, EndPos, 2.0f, o->Light, o);
        }

        vec3_t Light;
        VectorCopy(b->BodyLight, Light);
        Vector(0.9f, 0.7f, 1.0f, b->BodyLight);

        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(0, RENDER_BRIGHT | RENDER_CHROME, o->Alpha, 1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_FENRIR_THUNDER);
        b->RenderMesh(0, RENDER_BRIGHT | RENDER_METAL, o->Alpha, 1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_FENRIR_THUNDER);
        VectorCopy(Light, b->BodyLight);
    }
    else if (Type == MODEL_JEWEL_OF_HARMONY || Type == MODEL_LOWER_REFINE_STONE || Type == MODEL_HIGHER_REFINE_STONE)
    {
        b->RenderBody(RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type >= MODEL_SEED_FIRE && Type <= MODEL_SEED_EARTH)
    {
        int iCategoryIndex = Type - (MODEL_SEED_FIRE) + 1;
        switch (iCategoryIndex)
        {
        case 1:	// 0~9
            Vector(0.9f, 0.1f, 0.2f, b->BodyLight);
            break;
        case 2:	// 10~15
            Vector(0.4f, 0.5f, 1.0f, b->BodyLight);
            break;
        case 3:	// 16~20
            Vector(1.0f, 1.0f, 1.0f, b->BodyLight);
            break;
        case 4:	// 21~28
            Vector(0.4f, 1.0f, 0.6f, b->BodyLight);
            break;
        case 5:	// 29~33
            Vector(1.0f, 0.8f, 0.4f, b->BodyLight);
            break;
        case 6:	// 34~40
            Vector(1.0f, 0.4f, 1.0f, b->BodyLight);
            break;
        }
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderBody(RENDER_BRIGHT | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
    }
    else if (Type >= MODEL_SEED_SPHERE_FIRE_1 && Type <= MODEL_SEED_SPHERE_EARTH_5)
    {
        b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);

        int iCategoryIndex = (Type - (MODEL_SEED_SPHERE_FIRE_1)) % 6 + 1;
        switch (iCategoryIndex)
        {
        case 1:	// 0~9
            Vector(0.9f, 0.1f, 0.2f, b->BodyLight);
            break;
        case 2:	// 10~15
            Vector(0.4f, 0.5f, 1.0f, b->BodyLight);
            break;
        case 3:	// 16~20
            Vector(1.0f, 1.0f, 1.0f, b->BodyLight);
            break;
        case 4:	// 21~28
            Vector(0.4f, 1.0f, 0.6f, b->BodyLight);
            break;
        case 5:	// 29~33
            Vector(1.0f, 0.8f, 0.4f, b->BodyLight);
            break;
        case 6:	// 34~40
            Vector(1.0f, 0.4f, 1.0f, b->BodyLight);
            break;
        }
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, 1);
        b->RenderBody(RENDER_BRIGHT | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, 1);
    }
    else if (Type == MODEL_HELPER + 71 || Type == MODEL_HELPER + 72 || Type == MODEL_HELPER + 73 || Type == MODEL_HELPER + 74 || Type == MODEL_HELPER + 75)
    {
        int _angle = int(b->BodyAngle[1]) % 360;
        float _meshLight1;
        if (0 < _angle && _angle <= 180)
        {
            _meshLight1 = 0.2f - (sinf(Q_PI * (_angle) / 180.0f) * 0.2f);
        }
        else
        {
            _meshLight1 = 0.2f;
        }
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, 0.35f, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 1, _meshLight1, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, 2, _meshLight1, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (o->Kind == KIND_PLAYER && o->Type == MODEL_PLAYER && o->SubType == MODEL_SKELETON_PCBANG)
    {
        b->RenderBody(RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        vec3_t Light;
        VectorCopy(b->BodyLight, Light);
        Vector(0.9f, 0.8f, 1.0f, b->BodyLight);
        b->RenderBody(RENDER_BRIGHT | RENDER_CHROME, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_FENRIR_THUNDER);
        b->RenderBody(RENDER_BRIGHT | RENDER_METAL, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_FENRIR_THUNDER);
        VectorCopy(Light, b->BodyLight);
    }
    else if (o->Kind == KIND_PLAYER && o->Type == MODEL_PLAYER && o->SubType == MODEL_CURSEDTEMPLE_ALLIED_PLAYER)
    {
        b->RenderBody(RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, 1);

        if (g_isCharacterBuff(o, eDeBuff_CursedTempleRestraint) || g_isCharacterBuff(o, eBuff_CursedTempleProdection))
        {
            vec3_t vRelativePos, vtaWorldPos, Light;
            Vector(0.4f, 0.4f, 0.8f, Light);
            Vector(0.f, 0.f, 0.f, vRelativePos);

            if (g_isCharacterBuff(o, eDeBuff_CursedTempleRestraint))
            {
                float Luminosity = sinf(WorldTime * 0.002f) * 0.3f + 0.5f;
                b->RenderMesh(1, RENDER_TEXTURE, 0.4f + Luminosity, 0, o->BlendMeshLight * Luminosity, -WorldTime * 0.0005f, WorldTime * 0.0005f);
            }

            int boneindex[15] = { 1, 2, 17, 18, 19, 20, 44, 25, 27, 34, 37, 3, 5, 10, 12 };

            for (int j = 0; j < 15; ++j)
            {
                b->TransformPosition(o->BoneTransform[boneindex[j]], vRelativePos, vtaWorldPos, true);
                CreateSprite(BITMAP_LIGHT, vtaWorldPos, 1.3f, Light, o);
            }
        }

        if (o->m_pCloth == NULL)
        {
            int iTex = BITMAP_CURSEDTEMPLE_ALLIED_PHYSICSCLOTH;
            auto* pCloth = new CPhysicsCloth[1];
            pCloth[0].Create(o, 19, 0.0f, 8.0f, 0.0f, 10, 10, 140.0f, 140.0f, iTex, iTex, PCT_CURVED | PCT_SHORT_SHOULDER | PCT_MASK_ALPHA);
            o->m_pCloth = (void*)pCloth;
            o->m_byNumCloth = 1;
        }
        auto* pCloth = (CPhysicsCloth*)o->m_pCloth;
        if (pCloth)
        {
            float Flag = 0.005f;

            if (g_isCharacterBuff(o, eDeBuff_Stun) || g_isCharacterBuff(o, eDeBuff_Sleep))
            {
                Flag = 0.0f;
            }

            for (int k = 0; k < o->m_byNumCloth; ++k)
            {
                if (pCloth[k].Move2(Flag, 5) == FALSE)
                {
                    DeleteCloth(NULL, o);
                }
                else
                {
                    pCloth[k].Render();
                }
            }
        }
    }
    else if (o->Kind == KIND_PLAYER && o->Type == MODEL_PLAYER && o->SubType == MODEL_CURSEDTEMPLE_ILLUSION_PLAYER)
    {
        b->RenderBody(RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, 2);

        if (g_isCharacterBuff(o, eDeBuff_CursedTempleRestraint) || g_isCharacterBuff(o, eBuff_CursedTempleProdection))
        {
            vec3_t vRelativePos, vtaWorldPos, Light;
            Vector(0.4f, 0.4f, 0.8f, Light);
            Vector(0.f, 0.f, 0.f, vRelativePos);

            if (g_isCharacterBuff(o, eDeBuff_CursedTempleRestraint))
            {
                float Luminosity = sinf(WorldTime * 0.002f) * 0.3f + 0.5f;
                b->RenderMesh(2, RENDER_TEXTURE, 0.4f + Luminosity, 0, o->BlendMeshLight * Luminosity, -WorldTime * 0.0005f, WorldTime * 0.0005f);
            }

            int boneindex[15] = { 1, 2, 17, 18, 19, 20, 44, 25, 27, 34, 37, 3, 5, 10, 12 };

            for (int j = 0; j < 15; ++j)
            {
                b->TransformPosition(o->BoneTransform[boneindex[j]], vRelativePos, vtaWorldPos, true);
                CreateSprite(BITMAP_LIGHT, vtaWorldPos, 1.3f, Light, o);
            }
        }

        if (o->m_pCloth == NULL)
        {
            int iTex = BITMAP_CURSEDTEMPLE_ILLUSION_PHYSICSCLOTH;
            auto* pCloth = new CPhysicsCloth[2];
            pCloth[0].Create(o, 20, -4.0f, 5.0f, 0.0f, 10, 20, 17.0f, 100.0f, iTex, iTex, PCT_SHAPE_HALLOWEEN | PCT_ELASTIC_HALLOWEEN | PCT_MASK_ALPHA);
            o->m_pCloth = (void*)pCloth;
            o->m_byNumCloth = 1;
        }
        auto* pCloth = (CPhysicsCloth*)o->m_pCloth;
        if (pCloth)
        {
            float Flag = 0.005f;

            if (g_isCharacterBuff(o, eDeBuff_Stun) || g_isCharacterBuff(o, eDeBuff_Sleep))
            {
                Flag = 0.0f;
            }

            for (int k = 0; k < o->m_byNumCloth; ++k)
            {
                if (pCloth[k].Move2(Flag, 5) == FALSE)
                {
                    DeleteCloth(NULL, o);
                }
                else
                {
                    pCloth[k].Render();
                }
            }
        }
    }
    else if (o->Kind == KIND_PLAYER && o->Type == MODEL_PLAYER && o->SubType == MODEL_HALLOWEEN)
    {
        b->RenderBody(RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);

        vec3_t vLight;
        Vector(1.f, 1.f, 1.f, vLight);
        vec3_t vPos, vRelativePos;
        Vector(6.f, 6.f, 0.f, vRelativePos);
        VectorCopy(o->Position, b->BodyOrigin);
        b->TransformPosition(o->BoneTransform[20], vRelativePos, vPos, true);
        vPos[2] += 36.f;
        if (rand_fps_check(2))
        {
            CreateParticle(BITMAP_TRUE_FIRE, vPos, o->Angle, vLight, 8, 0.5f, o);
        }
        CreateParticleFpsChecked(BITMAP_TRUE_FIRE, vPos, o->Angle, vLight, 9, 0.5f, o);

        vPos[2] -= 5.0f;
        float fLumi;
        fLumi = (float)(rand() % 5 + 10) * 0.1f;
        Vector(fLumi + 0.5f, fLumi * 0.3f + 0.1f, 0.f, vLight);
        CreateSprite(BITMAP_LIGHT, vPos, 1.0f + fLumi * 0.1f, vLight, o);

        if (rand_fps_check(8))
        {
            vPos[2] -= 10.0f;
            fLumi = (float)(rand() % 100) * 0.01f;
            vLight[0] = fLumi;
            fLumi = (float)(rand() % 100) * 0.01f;
            vLight[1] = fLumi;
            fLumi = (float)(rand() % 100) * 0.01f;
            vLight[2] = fLumi;
            CreateParticle(BITMAP_SHINY, vPos, o->Angle, vLight, 4, 0.8f);
        }

        if (o->CurrentAction == PLAYER_JACK_1)
        {
            int iAnimationFrame = (int)o->AnimationFrame;

            if (iAnimationFrame == 2 || iAnimationFrame == 9)
            {
                Vector(1.f, 1.f, 1.f, vLight);
                vPos[2] -= 20.f;
                CreateParticleFpsChecked(BITMAP_SPARK + 2, vPos, o->Angle, vLight, 0, 0.6f);
                CreateParticleFpsChecked(BITMAP_SPARK + 2, vPos, o->Angle, vLight, 0, 0.6f);
                for (int i = 0; i < 20; ++i)
                {
                    fLumi = (float)(rand() % 100) * 0.01f;
                    vLight[0] = fLumi;
                    fLumi = (float)(rand() % 100) * 0.01f;
                    vLight[1] = fLumi;
                    fLumi = (float)(rand() % 100) * 0.01f;
                    vLight[2] = fLumi;
                    CreateParticleFpsChecked(BITMAP_SHINY, vPos, o->Angle, vLight, 4, 0.8f);
                }
                o->m_iAnimation++;
            }

            if (o->m_iAnimation >= 40 && iAnimationFrame == 9)
            {
                Vector(1.f, 1.f, 1.f, vLight);
                CreateEffect(MODEL_HALLOWEEN_EX, vPos, o->Angle, vLight, 0);
                vPos[2] -= 40.f;
                CreateParticleFpsChecked(BITMAP_SPARK + 2, vPos, o->Angle, vLight, 1);
                SetAction(o, PLAYER_SHOCK);
                SendRequestAction(*o, PLAYER_SHOCK);
                o->m_iAnimation = 0;
            }

            float fStageG, fStageB;
            fStageG = 0.8f - ((o->m_iAnimation / 40.f) * 0.8f);
            if (fStageG < 0.2f)
            {
                fStageG = 0.2f;
            }
            fStageB = 0.8f - ((o->m_iAnimation / 40.f) * 0.9f);
            if (fStageB < 0.1f)
            {
                fStageB = 0.1f;
            }
            VectorCopy(b->BodyLight, vLight);
            Vector(1.f, fStageG, fStageB, b->BodyLight);
            b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
            VectorCopy(vLight, b->BodyLight);
        }
        else if (o->CurrentAction == PLAYER_JACK_2)
        {
            int iAnimationFrame = (int)o->AnimationFrame;

            if (iAnimationFrame == 10 || iAnimationFrame == 19)
            {
                Vector(1.f, 1.f, 1.f, vLight);
                vPos[2] -= 20.f;
                CreateParticleFpsChecked(BITMAP_SPARK + 2, vPos, o->Angle, vLight, 0, 0.6f);
                CreateParticleFpsChecked(BITMAP_SPARK + 2, vPos, o->Angle, vLight, 0, 0.6f);
                for (int i = 0; i < 20; ++i)
                {
                    fLumi = (float)(rand() % 100) * 0.01f;
                    vLight[0] = fLumi;
                    fLumi = (float)(rand() % 100) * 0.01f;
                    vLight[1] = fLumi;
                    fLumi = (float)(rand() % 100) * 0.01f;
                    vLight[2] = fLumi;
                    CreateParticleFpsChecked(BITMAP_SHINY, vPos, o->Angle, vLight, 4, 0.8f);
                }
                o->m_iAnimation++;
            }

            if (o->m_iAnimation >= 40 && iAnimationFrame == 19)
            {
                Vector(1.f, 1.f, 1.f, vLight);
                CreateEffect(MODEL_HALLOWEEN_EX, vPos, o->Angle, vLight, 0);
                vPos[2] -= 40.f;
                CreateParticleFpsChecked(BITMAP_SPARK + 2, vPos, o->Angle, vLight, 1);
                SetAction(o, PLAYER_SHOCK);
                SendRequestAction(*o, PLAYER_SHOCK);
                o->m_iAnimation = 0;
            }

            float fStageG, fStageB;
            fStageG = 0.8f - ((o->m_iAnimation / 40.f) * 0.8f);
            if (fStageG < 0.2f)
            {
                fStageG = 0.2f;
            }
            fStageB = 0.8f - ((o->m_iAnimation / 40.f) * 0.9f);
            if (fStageB < 0.1f)
            {
                fStageB = 0.1f;
            }
            VectorCopy(b->BodyLight, vLight);
            Vector(1.f, fStageG, fStageB, b->BodyLight);
            b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
            VectorCopy(vLight, b->BodyLight);
        }

        if (o->m_pCloth == NULL)
        {
            int iTex = BITMAP_ROBE + 3;
            auto* pCloth = new CPhysicsCloth[1];
            pCloth[0].Create(o, 19, 0.0f, 10.0f, 0.0f, 10, 20, 30.0f, 200.0f, iTex, iTex, PCT_SHAPE_HALLOWEEN | PCT_ELASTIC_HALLOWEEN | PCT_MASK_ALPHA);
            o->m_pCloth = (void*)pCloth;
            o->m_byNumCloth = 1;
        }
        auto* pCloth = (CPhysicsCloth*)o->m_pCloth;
        if (pCloth)
        {
            if (pCloth[0].Move2(0.005f, 5) == FALSE)
            {
                DeleteCloth(NULL, o);
            }
            else
            {
                pCloth[0].Render();
            }
        }
    }
    else if (o->Kind == KIND_PLAYER && o->Type == MODEL_PLAYER && o->SubType == MODEL_PANDA)
    {
        vec3_t Light;
        vec3_t vPos;

        Vector(1.f, 0.6f, 0.2f, Light);
        if (rand_fps_check(100))
        {
            CreateEffect(MODEL_ARROWSRE06, vPos, o->Angle, Light, 2, o, 0, 0, 0, 0, 1.f);
            CreateEffect(MODEL_ARROWSRE06, vPos, o->Angle, Light, 2, o, 1, 0, 0, 0, 1.f);
        }

        float fLumi = (sinf(WorldTime * 0.002f) + 1.f) * 0.5f;
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 1, fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_PANDABODY_R);
        b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(3, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);

        int iAnimationFrame = (int)o->AnimationFrame;
        static int iPriorAnimationFrame = 0;
        iPriorAnimationFrame = iAnimationFrame;
    }
    else if (o->Kind == KIND_PLAYER && o->Type == MODEL_PLAYER && o->SubType == MODEL_XMAS_EVENT_CHANGE_GIRL)
    {
        b->RenderBody(RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);

        int iAnimationFrame = (int)o->AnimationFrame;
        static int iPriorAnimationFrame = 0;

        if (o->CurrentAction == PLAYER_SANTA_1)
        {
            if (iAnimationFrame == 4 || iAnimationFrame == 8 || iAnimationFrame == 12)
            {
                int iEffectType = rand() % 4 + MODEL_XMAS_EVENT_BOX;
                vec3_t vPos;
                vec3_t vLight;
                Vector(1.f, 1.f, 1.f, vLight);
                VectorCopy(o->Position, vPos);
                vPos[2] += 230.f;
                for (int i = 0; i < 2; ++i)
                {
                    CreateEffectFpsChecked(iEffectType, vPos, o->Angle, o->Light);
                    CreateParticleFpsChecked(BITMAP_SPARK + 1, vPos, o->Angle, o->Light, 20, 1.f);
                }
                for (int i = 0; i < 5; ++i)
                {
                    CreateParticleFpsChecked(BITMAP_SPARK + 1, vPos, o->Angle, vLight, 20, 1.f);
                }
            }
            if (iAnimationFrame >= 19 && iPriorAnimationFrame != iAnimationFrame)
            {
                o->m_iAnimation++;
            }
        }
        else if (o->CurrentAction == PLAYER_SANTA_2)
        {
            vec3_t vPos, vLight;
            VectorCopy(o->Position, vPos);
            vPos[2] += 230;
            Vector(1.f, 1.f, 1.f, vLight);
            CreateParticleFpsChecked(BITMAP_SPARK + 1, vPos, o->Angle, vLight, 20, 1.f);
            CreateParticleFpsChecked(BITMAP_SMOKE, vPos, o->Angle, vLight, 4, 2.0f);
            if (rand_fps_check(6))
            {
                CreateParticle(BITMAP_SNOW_EFFECT_1, vPos, o->Angle, vLight, 0, 0.3f);
            }
            if (rand_fps_check(3))
            {
                CreateParticle(BITMAP_SNOW_EFFECT_2, vPos, o->Angle, vLight, 0, 0.5f);
            }

            if (iAnimationFrame >= 14 && iPriorAnimationFrame != iAnimationFrame)
            {
                o->m_iAnimation++;
                DeleteEffect(MODEL_XMAS_EVENT_ICEHEART, NULL);
                DeleteParticle(BITMAP_DS_EFFECT);
                DeleteParticle(BITMAP_LIGHT);
            }
        }

        iPriorAnimationFrame = iAnimationFrame;
    }
    else if (o->SubType == MODEL_XMAS_EVENT_CHA_DEER)
    {
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        float fLumi = (sinf(WorldTime * 0.002f) + 1.f) * 0.5f;
        b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, 1, fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_BLOOD + 1);
        b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, 1, fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_BLOOD + 1);
        b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, 1, fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_BLOOD + 1);
    }
    else if (o->Kind == KIND_PLAYER && o->Type == MODEL_PLAYER && o->SubType == MODEL_GM_CHARACTER)
    {
        o->HiddenMesh = 2;
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);

        vec3_t vRelativePos, vPos, vLight, vLight2;
        float fLight;
        fLight = sinf(WorldTime * 0.08f) * 0.2f + 0.2f;

        Vector(0.f, 0.3f, 1.2f, vLight);
        Vector(0.3f + fLight, 0.f + fLight, 0.1f + fLight, vLight2);

        Vector(0.f, 0.f, -7.5f, vRelativePos);
        b->TransformPosition(o->BoneTransform[26], vRelativePos, vPos, true);
        CreateSprite(BITMAP_SHINY + 1, vPos, 0.9f, vLight, o, -WorldTime * 0.08f);
        CreateSprite(BITMAP_LIGHT + 3, vPos, 0.8f, vLight2, o, WorldTime * 0.3f);

        Vector(0.f, 0.f, 7.5f, vRelativePos);
        b->TransformPosition(o->BoneTransform[35], vRelativePos, vPos, true);
        CreateSprite(BITMAP_SHINY + 1, vPos, 0.9f, vLight, o, WorldTime * 0.08f);
        CreateSprite(BITMAP_LIGHT + 3, vPos, 0.8f, vLight2, o, -WorldTime * 0.3f);

        Vector(-5.f, 2.5f, 0.f, vRelativePos);
        b->TransformPosition(o->BoneTransform[5], vRelativePos, vPos, true);
        CreateSprite(BITMAP_SHINY + 1, vPos, 0.9f, vLight, o, -WorldTime * 0.08f);
        CreateSprite(BITMAP_LIGHT + 3, vPos, 0.8f, vLight2, o, WorldTime * 0.3f);

        b->TransformPosition(o->BoneTransform[12], vRelativePos, vPos, true);
        CreateSprite(BITMAP_SHINY + 1, vPos, 0.9f, vLight, o, -WorldTime * 0.08f);
        CreateSprite(BITMAP_LIGHT + 3, vPos, 0.8f, vLight2, o, WorldTime * 0.3f);
    }
    else if (Type == MODEL_HELPER + 69)
    {
        b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        float Luminosity = (sinf(WorldTime * 0.003f) + 1) * 0.3f + 0.3f;
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, Luminosity, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
    }
    else if (Type == MODEL_HELPER + 70)
    {
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(1, RENDER_BRIGHT | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
    }
    else if (Type == MODEL_HELPER + 81)
    {
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(0, RENDER_BRIGHT | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_HELPER + 82)
    {
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
    }
    else if (Type == MODEL_HELPER + 66)
    {
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(0, RENDER_BRIGHT | RENDER_CHROME4, o->Alpha, 0, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);

        float Luminosity = (sinf(WorldTime * 0.003f) + 1) * 0.3f + 0.6f;
        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 1, Luminosity, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_POTION + 100)
    {
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (o->Type >= MODEL_TYPE_CHARM_MIXWING + EWS_BEGIN && o->Type <= MODEL_TYPE_CHARM_MIXWING + EWS_END)
    {
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
    }
    else if (Type == MODEL_HELPER + 107)
    {
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(1, RENDER_BRIGHT | RENDER_CHROME, o->Alpha, 0, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_HELPER + 104)
    {
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(0, RENDER_BRIGHT | RENDER_CHROME, 0.2f, 0, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_HELPER + 105)
    {
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(0, RENDER_BRIGHT | RENDER_CHROME, 0.2f, 0, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (o->Type >= MODEL_HELPER + 109 && o->Type <= MODEL_HELPER + 112)	// InGameShop 장착 아이템 : 반지 (사파이어, 루비, 토파즈, 자수정)
    {
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(1, RENDER_BRIGHT | RENDER_CHROME, o->Alpha, 0, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (o->Type >= MODEL_HELPER + 113 && o->Type <= MODEL_HELPER + 115)// InGameShop 장착 아이템 : 목걸이 (사파이어, 루비, 에메랄드)
    {
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(1, RENDER_BRIGHT | RENDER_CHROME, o->Alpha, 0, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (o->Type >= MODEL_POTION + 112 && o->Type <= MODEL_POTION + 113)
    {
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(0, RENDER_BRIGHT | RENDER_CHROME, o->Alpha, 0, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (o->Type == MODEL_POTION + 120)
    {
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(0, RENDER_BRIGHT | RENDER_CHROME, o->Alpha, 0, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (o->Type == MODEL_POTION + 121 || o->Type == MODEL_POTION + 122)
    {
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(1, RENDER_BRIGHT | RENDER_CHROME, o->Alpha, 1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (o->Type == MODEL_POTION + 123 || o->Type == MODEL_POTION + 124)
    {
        b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(1, RENDER_BRIGHT | RENDER_CHROME, o->Alpha, 1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (o->Type == MODEL_WING + 130)
    {
        if (b->BodyLight[0] == 1 && b->BodyLight[1] == 1 && b->BodyLight[2] == 1)
        {
            Vector(1.f, 1.f, 1.f, b->BodyLight);
            glColor3fv(b->BodyLight);
            b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        }
    }
    else if (o->Type == MODEL_POTION + 134)
    {
        //b->RenderBody(RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight, o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_PACKAGEBOX_RED);
    }
    else if (o->Type == MODEL_POTION + 135)
    {
        //b->RenderBody(RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight, o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_PACKAGEBOX_BLUE);
    }
    else if (o->Type == MODEL_POTION + 136)
    {
        //b->RenderBody(RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight, o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_PACKAGEBOX_GOLD);
    }
    else if (o->Type == MODEL_POTION + 137)
    {
        //b->RenderBody(RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight, o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_PACKAGEBOX_GREEN);
    }
    else if (o->Type == MODEL_POTION + 138)
    {
        //b->RenderBody(RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight, o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_PACKAGEBOX_PUPLE);
    }
    else if (o->Type == MODEL_POTION + 139)
    {
        //b->RenderBody(RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight, o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_PACKAGEBOX_SKY);
    }
    else if (o->Type >= MODEL_POTION + 114 && o->Type <= MODEL_POTION + 119)
    {
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_INGAMESHOP_PRIMIUM6);
    }
    else if (o->Type >= MODEL_POTION + 126 && o->Type <= MODEL_POTION + 129)
    {
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_INGAMESHOP_COMMUTERTICKET4);
    }
    else if (o->Type >= MODEL_POTION + 130 && o->Type <= MODEL_POTION + 132)
    {
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_INGAMESHOP_SIZECOMMUTERTICKET3);
    }
    else if (o->Type == MODEL_HELPER + 121)
    {
        float fLumi = (sinf(WorldTime * 0.0015f) + 1.2f) * 0.3f;
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_FREETICKET_R);
    }
    else if (Type >= MODEL_POTION + 141 && Type <= MODEL_POTION + 144)
    {
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(0, RENDER_BRIGHT | RENDER_CHROME, o->Alpha, 0, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_SKELETON_TRANSFORMATION_RING)
    {
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderBody(RENDER_BRIGHT | RENDER_CHROME, 0.5f, 0, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
    }
    else if (o->Type == MODEL_15GRADE_ARMOR_OBJ_ARMLEFT ||
        o->Type == MODEL_15GRADE_ARMOR_OBJ_ARMRIGHT ||
        o->Type == MODEL_15GRADE_ARMOR_OBJ_BODYLEFT ||
        o->Type == MODEL_15GRADE_ARMOR_OBJ_BODYRIGHT ||
        o->Type == MODEL_15GRADE_ARMOR_OBJ_BOOTLEFT ||
        o->Type == MODEL_15GRADE_ARMOR_OBJ_BOOTRIGHT ||
        o->Type == MODEL_15GRADE_ARMOR_OBJ_HEAD ||
        o->Type == MODEL_15GRADE_ARMOR_OBJ_PANTLEFT ||
        o->Type == MODEL_15GRADE_ARMOR_OBJ_PANTRIGHT)
    {
        float fLight, texCoordU;

        fLight = 0.8f - absf(sinf(WorldTime * 0.0018f) * 0.5f);
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, fLight - 0.1f, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        texCoordU = absf(sinf(WorldTime * 0.0005f));
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, fLight - 0.3f, texCoordU, o->BlendMeshTexCoordV, BITMAP_RGB_MIX);
        b->RenderMesh(0, RENDER_TEXTURE | RENDER_CHROME4, o->Alpha, 0, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }

#ifdef LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM
    else if (Type == MODEL_HELPER + 128 || Type == MODEL_HELPER + 129)
    {
        float fEyeBlinkingTime = sinf(WorldTime * 0.005f) + 1;

        b->RenderBody(RENDER_TEXTURE, 1.0f, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(0, RENDER_BRIGHT | RENDER_CHROME, 0.3f, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 1, fEyeBlinkingTime, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
#endif //LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM
#ifdef LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM_PART_2
    else if (Type >= MODEL_HELPER + 130 && Type <= MODEL_HELPER + 133)
    {
        Vector(1.f, 1.f, 1.f, b->BodyLight);
        glColor3fv(b->BodyLight);
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        float Luminosity = absf(sinf(WorldTime * 0.001f)) * 0.3f;
        Vector(0.1f + Luminosity, 0.1f + Luminosity, 0.1f + Luminosity, b->BodyLight);
        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        Luminosity = absf(sinf(WorldTime * 0.001f)) * 0.8f;
        Vector(0.0f + Luminosity, 0.0f + Luminosity, 0.0f + Luminosity, b->BodyLight);

        switch (Type)
        {
        case MODEL_HELPER + 130:
            b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_ORK_CHAM_LAYER_R);
            break;
        case MODEL_HELPER + 131:
            b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_MAPLE_CHAM_LAYER_R);
            break;
        case MODEL_HELPER + 132:
            b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_GOLDEN_ORK_CHAM_LAYER_R);
            break;
        case MODEL_HELPER + 133:
            b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_GOLDEN_MAPLE_CHAM_LAYER_R);
            break;
        default:
            break;
        }
    }
#endif //LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM_PART_2

    else if (g_CMonkSystem.EqualItemModelType(Type) == MODEL_PHOENIX_SOUL_STAR) //ok
    {
        float fLumi = (sinf(WorldTime * 0.003) + 1.f) * 0.3f + 0.4f;
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(0, RENDER_BRIGHT, o->Alpha * fLumi, 0, o->BlendMeshLight * fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_PHOENIXSOULWING);

        Vector(.15f, 1.f, .25f, b->BodyLight);
        glColor3fv(b->BodyLight);
        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        Vector(1.f, 1.f, 1.f, b->BodyLight);
        b->RenderMesh(1, RENDER_BRIGHT | RENDER_CHROME3, o->Alpha, 1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        Vector(1.f, 1.f, 1.f, b->BodyLight);
    }
    else if (Type == MODEL_SWORD_35_WING)
    {
        float fLumi = (sinf(WorldTime * 0.003) + 1.f) * 0.3f + 0.4f;
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(0, RENDER_BRIGHT, o->Alpha * fLumi, 0, o->BlendMeshLight * fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_PHOENIXSOULWING);

        Vector(.15f, 1.f, .25f, b->BodyLight);
        glColor3fv(b->BodyLight);
        b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        Vector(1.f, 1.f, 1.f, b->BodyLight);
        b->RenderMesh(1, RENDER_CHROME3 | RENDER_BRIGHT, o->Alpha, 1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        Vector(1.f, 1.f, 1.f, b->BodyLight);
    }
    else if (Type == MODEL_PHOENIX_SOUL_HELMET)
    {
        float fLumi = (sinf(WorldTime * 0.003) + 1.f) * 0.3f + 0.4f;
        if (b->HideSkin == true)
        {
            glColor3fv(b->BodyLight);
            b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, -1);
            b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, -1);
            b->RenderMesh(2, RENDER_CHROME | RENDER_BRIGHT, o->Alpha * fLumi, 2, o->BlendMeshLight * fLumi, (double)(-int(WorldTime) % 1000) * 0.00009f, o->BlendMeshTexCoordV, -1);
        }
        else
        {
            b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
            b->RenderMesh(2, RENDER_CHROME | RENDER_BRIGHT, o->Alpha * fLumi, 2, o->BlendMeshLight * fLumi, (double)(-int(WorldTime) % 1000) * 0.00009f, o->BlendMeshTexCoordV, -1);
        }
    }
    else if (Type == MODEL_PHOENIX_SOUL_ARMOR)
    {
        float fLumi = (sinf(WorldTime * 0.003) + 1.f) * 0.3f + 0.4f;
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, -1);
        b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, (double)(-int(WorldTime) % 1000) * 0.00009f, -1);
        b->RenderMesh(1, RENDER_CHROME | RENDER_BRIGHT, o->Alpha * fLumi, 1, o->BlendMeshLight * fLumi, (double)(-int(WorldTime) % 1000) * 0.00009f, o->BlendMeshTexCoordV, -1);
        b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, -1);
    }
    else if (Type == MODEL_ARMORINVEN_74)
    {
        vec3_t Light;
        VectorCopy(b->BodyLight, Light);
        glColor3fv(b->BodyLight);
        float fLumi = (sinf(WorldTime * 0.003) + 1.f) * 0.3f + 0.4f;
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, -1);
        b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, (double)(-int(WorldTime) % 1000) * 0.00009f, o->BlendMeshTexCoordV, -1);
        b->RenderMesh(1, RENDER_CHROME | RENDER_BRIGHT, o->Alpha * fLumi, o->BlendMesh, o->BlendMeshLight * fLumi, (double)(-int(WorldTime) % 1000) * 0.00009f, o->BlendMeshTexCoordV, -1);
        VectorCopy(Light, b->BodyLight);
    }
    else if (Type == MODEL_PHOENIX_SOUL_BOOTS)
    {
        glColor3fv(b->BodyLight);
        float fLumi = (sinf(WorldTime * 0.003) + 1.f) * 0.3f + 0.4f;
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, -1);
        b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, (double)(-int(WorldTime) % 1000) * 0.00009f, o->BlendMeshTexCoordV, -1);
        b->RenderMesh(1, RENDER_CHROME | RENDER_BRIGHT, o->Alpha * fLumi, 1, o->BlendMeshLight * fLumi, (double)(-int(WorldTime) % 1000) * 0.00009f, o->BlendMeshTexCoordV, -1);
    }
    else if (Type == MODEL_CAPE_OF_FIGHTER || Type == MODEL_WING + 135)
    {
        b->RenderBody(RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_CAPE_OF_OVERRULE)
    {
        if (b->BodyLight[0] == 1 && b->BodyLight[1] == 1 && b->BodyLight[2] == 1)
        {
            Vector(1.f, 1.f, 1.f, b->BodyLight);
            glColor3fv(b->BodyLight);
            b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        }
        else
        {
            Vector(1.f, 1.f, 1.f, b->BodyLight);
            glColor3fv(b->BodyLight);
            b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        }
    }
    else if (Type == MODEL_SACRED_HELM || Type == MODEL_STORM_HARD_HELM)
    {
        if (b->HideSkin)
        {
            ::glColor3fv(b->BodyLight);
            b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        }
        else
            b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
    }
    else if (Type == MODEL_PIERCING_HELM)
    {
        if (b->HideSkin)
        {
            ::glColor3fv(b->BodyLight);
            b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        }
        else
            b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
    }
    else if ((g_CMonkSystem.EqualItemModelType(Type) == MODEL_SACRED_GLOVE)
        || (g_CMonkSystem.EqualItemModelType(Type) == MODEL_STORM_HARD_GLOVE)
        || (g_CMonkSystem.EqualItemModelType(Type) == MODEL_PIERCING_BLADE_GLOVE)
        || (g_CMonkSystem.EqualItemModelType(Type) == MODEL_PHOENIX_SOUL_STAR))
    {
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
    }
    else if (Type >= MODEL_CHAIN_DRIVE_PARCHMENT && Type <= MODEL_INCREASE_BLOCK_PARCHMENT)
    {
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        float fLumi = (sinf(WorldTime * 0.0015f) + 1.0f) * 0.5f;
        b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0, fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh, BITMAP_ROOLOFPAPER_EFFECT_R);
    }
    /*
    else if (Type == MODEL_BODY_ARMOR + (MAX_CLASS * 2) + CLASS_RAGEFIGHTER || Type == MODEL_BODY_PANTS + (MAX_CLASS * 2) + CLASS_RAGEFIGHTER
        || Type == MODEL_BODY_GLOVES + (MAX_CLASS * 2) + CLASS_RAGEFIGHTER || Type == MODEL_BODY_BOOTS + (MAX_CLASS * 2) + CLASS_RAGEFIGHTER)
    {
        const char* pSkinTextureName = "LevelClass207_1";
        int nLen = strlen(pSkinTextureName);
        for (int i = 0; i < b->NumMeshs; ++i)
        {
            Texture_t* pTexture = &b->Textures[i];
            int SkinTexture = (!strnicmp(pTexture->FileName, pSkinTextureName, nLen)) ? BITMAP_SKIN + 14 : -1;
            if (SkinTexture != -1)
            {
                b->RenderMesh(i, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, SkinTexture);
            }
        }
    }*/

    else if (Check_LuckyItem(Type, -MODEL_ITEM))
    {
        bool	bHide = false;
        int		nIndex = 0;
        if (Type == MODEL_HELM + 65)	nIndex = 2;
        else if (Type == MODEL_HELM + 70)	nIndex = 1;
        if (nIndex > 0)					bHide = true;

        if (bHide)
        {
            b->RenderMesh(nIndex, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        }
        else if (b->HideSkin)
        {
            if (Type == MODEL_ARMOR + 65)	nIndex = BITMAP_INVEN_ARMOR + 6;
            else if (Type == MODEL_ARMOR + 70)	nIndex = BITMAP_INVEN_ARMOR + 7;
            else if (Type == MODEL_PANTS + 65)	nIndex = BITMAP_INVEN_PANTS + 6;
            else if (Type == MODEL_PANTS + 70)	nIndex = BITMAP_INVEN_PANTS + 7;

            if (nIndex > 0)
            {
                b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, nIndex);
                for (int i = 1; i < b->NumMeshs; ++i)
                    b->RenderMesh(i, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
            }
            else	b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        }
        else
        {
            b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        }
    }
    else
    {
        b->RenderBody(RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
}

void RenderPartObjectBodyColor(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType, float Bright, int Texture, int iMonsterIndex)
{
    if (Type >= MODEL_HELM_MONK && Type <= MODEL_BOOTS_MONK + MODEL_ITEM_COMMONCNT_RAGEFIGHTER)
        Type = g_CMonkSystem.OrginalTypeCommonItemMonk(Type);
    if ((RenderType & RENDER_LIGHTMAP) == RENDER_LIGHTMAP)
    {
        Vector(1.f, 1.f, 1.f, b->BodyLight);
    }
    else if (Type == MODEL_DRAKAN)
    {
        if (RenderType & RENDER_EXTRA)
        {
            RenderType -= RENDER_EXTRA;
            Vector(1.f, 0.1f, 0.1f, b->BodyLight);
        }
        else
        {
            Vector(0.2f, 0.2f, 0.8f, b->BodyLight);
            //Vector(1.0f,0.5f,0.0f,b->BodyLight);
        }
    }
    else if (iMonsterIndex >= 493 && iMonsterIndex <= 502)
    {
        if (iMonsterIndex == 495)
        {
            Vector(1.0f, 0.8f, 0.0f, b->BodyLight);
        }
        else if (iMonsterIndex == 496)
        {
            Vector(1.0f, 0.6f, 0.0f, b->BodyLight);
        }
        else if (iMonsterIndex == 499)
        {
            Vector(1.0f, 0.5f, 0.0f, b->BodyLight);
        }
        else if (iMonsterIndex == 501)
        {
            Vector(1.0f, 0.5f, 0.0f, b->BodyLight);
        }
        else
        {
            Vector(1.f, 0.6f, 0.1f, b->BodyLight);
        }
    }
    else
    {
        PartObjectColor(Type, Alpha, Bright, b->BodyLight, (RenderType & RENDER_EXTRA) ? true : false);
    }

    if (Type == MODEL_ELEMENTAL_MACE)
    {
        o->HiddenMesh = 2;
    }

    if (Type == MODEL_LEGENDARY_STAFF)
        b->RenderBody(RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, 1, Texture);
    else if (Type == MODEL_KNIGHT_BLADE)
        b->RenderBody(RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, 1, Texture);
    else if (Type == MODEL_DARK_REIGN_BLADE)
        b->RenderBody(RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, 0, Texture);
    else if (Type == MODEL_RUNE_BLADE || Type == MODEL_DRAGON_SPEAR || Type == MODEL_ELEMENTAL_MACE || Type == MODEL_ELEMENTAL_SHIELD)
        b->RenderBody(RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh, Texture);
    else if (Type >= MODEL_BATTLE_SCEPTER && Type <= MODEL_DIVINE_SCEPTER_OF_ARCHANGEL)
        b->RenderBody(RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh, Texture);
    else if (Type == MODEL_FLAMBERGE)
    {
        b->RenderBody(RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, 5);
    }
    else if (Type == MODEL_SWORD_BREAKER)
    {
        b->RenderMesh(0, RenderType, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_IMPERIAL_SWORD)
    {
        b->RenderMesh(2, RenderType, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_DEADLY_STAFF)
    {
        b->RenderBody(RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, -1, Texture);
        o->BlendMesh = 1;
        Vector(1.f, 1.f, 1.f, b->BodyLight);
        b->RenderMesh(1, RenderType, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_STAFF + 32)
    {
        b->RenderMesh(1, RenderType, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_FROST_BARRIER)
    {
        b->RenderMesh(0, RenderType, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_GUARDIAN_SHILED)
    {
        b->RenderMesh(0, RenderType, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_SERAPHIM_HELM)
    {
        b->RenderMesh(0, RenderType, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_FAITH_HELM)
    {
        b->RenderMesh(1, RenderType, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_HELM + 53)
    {
        b->RenderMesh(2, RenderType, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_CROSS_SHIELD)
    {
        b->RenderMesh(0, RenderType, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_BEUROBA)
    {
        b->RenderMesh(0, RenderType, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(1, RenderType, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_CHROMATIC_STAFF)
    {
        b->RenderMesh(0, RenderType, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_RAVEN_STICK)
    {
        b->RenderMesh(0, RenderType, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_STRYKER_SCEPTER)
    {
        b->RenderMesh(0, RenderType, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_AIR_LYN_BOW)
    {
        b->RenderMesh(0, RenderType, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type >= MODEL_SACRED_GLOVE && Type <= MODEL_PHOENIX_SOUL_STAR)
    {
        b->RenderBody(RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_SACRED_HELM || Type == MODEL_STORM_HARD_HELM)
    {
        ::glColor3fv(b->BodyLight);
        b->RenderMesh(1, RenderType, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_PIERCING_HELM)
    {
        ::glColor3fv(b->BodyLight);
        b->RenderMesh(0, RenderType, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_SACRED_ARMOR)
    {
        b->RenderMesh(1, RenderType, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_STORM_HARD_ARMOR)
    {
        b->RenderMesh(1, RenderType, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_PIERCING_ARMOR)
    {
        b->RenderMesh(0, RenderType, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_PHOENIX_SOUL_HELMET)
    {
        b->RenderMesh(0, RenderType, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_PHOENIX_SOUL_ARMOR)
    {
        b->RenderMesh(2, RenderType, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_ARMORINVEN_74)
    {
        if (RenderType & RENDER_METAL)
        {
            b->RenderMesh(0, RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
            b->RenderMesh(0, RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        }
        b->RenderMesh(0, RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_PHOENIX_SOUL_BOOTS)
    {
        b->RenderMesh(0, RenderType, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else
    {
        b->RenderBody(RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, -1, Texture);
    }

    if (Type == MODEL_ELEMENTAL_MACE)
    {
        o->HiddenMesh = -1;
    }
}

void RenderPartObjectBodyColor2(BMD* b, OBJECT* o, int Type, float Alpha, int RenderType, float Bright, int Texture)
{
    if (Type >= MODEL_HELM_MONK && Type <= MODEL_BOOTS_MONK + MODEL_ITEM_COMMONCNT_RAGEFIGHTER)
        Type = g_CMonkSystem.OrginalTypeCommonItemMonk(Type);

    if ((RenderType & RENDER_LIGHTMAP) == RENDER_LIGHTMAP)
    {
        Vector(1.f, 1.f, 1.f, b->BodyLight);
    }
    else if ((RenderType & RENDER_CHROME3) == RENDER_CHROME3)
    {
        PartObjectColor3(Type, Alpha, Bright, b->BodyLight, (RenderType & RENDER_EXTRA) ? true : false);
    }
    else
    {
        PartObjectColor2(Type, Alpha, Bright, b->BodyLight, (RenderType & RENDER_EXTRA) ? true : false);
    }
    if (Type == MODEL_LEGENDARY_STAFF)
        b->RenderBody(RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, 1, Texture);
    else if (Type == MODEL_KNIGHT_BLADE)
        b->RenderBody(RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, 1, Texture);
    else if (Type == MODEL_DARK_REIGN_BLADE)
        b->RenderBody(RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, 0, Texture);
    else if (Type == MODEL_GRAND_SOUL_SHIELD || Type == MODEL_ELEMENTAL_SHIELD)
        b->RenderBody(RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, -1, Texture);
    else if (Type == MODEL_ILLUSION_SORCERER_COVENANT)
    {
        b->RenderBody(RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, 1, Texture);
    }
    else if (Type == MODEL_POTION + 64)
    {
        b->RenderBody(RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, 0, Texture);
    }
    else if (Type == MODEL_FLAMBERGE)
    {
        b->RenderBody(RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, 5, Texture);
    }
    else if (Type == MODEL_FROST_BARRIER)
    {
        b->RenderMesh(0, RenderType, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_SERAPHIM_HELM)
    {
        b->RenderMesh(0, RenderType, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_FAITH_HELM)
    {
        b->RenderMesh(1, RenderType, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_HELM + 53)
    {
        b->RenderMesh(2, RenderType, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type >= MODEL_SACRED_GLOVE && Type <= MODEL_PHOENIX_SOUL_STAR)
    {
        b->RenderBody(RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_SACRED_HELM || Type == MODEL_STORM_HARD_HELM)
    {
        ::glColor3fv(b->BodyLight);
        b->RenderMesh(1, RenderType, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_PIERCING_HELM)
    {
        ::glColor3fv(b->BodyLight);
        b->RenderMesh(0, RenderType, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_SACRED_ARMOR)
    {
        b->RenderMesh(1, RenderType, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_STORM_HARD_ARMOR)
    {
        b->RenderMesh(1, RenderType, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_PIERCING_ARMOR)
    {
        b->RenderMesh(0, RenderType, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_PHOENIX_SOUL_HELMET)
    {
        b->RenderMesh(0, RenderType, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_PHOENIX_SOUL_ARMOR)
    {
        b->RenderMesh(2, RenderType, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_ARMORINVEN_74)
    {
        b->RenderMesh(0, RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else if (Type == MODEL_PHOENIX_SOUL_BOOTS)
    {
        b->RenderMesh(0, RenderType, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
    }
    else
    {
        b->RenderBody(RenderType, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, -1, Texture);
    }
}

void NextGradeObjectRender(CHARACTER* c)
{
    int weaponIndex;
    int weaponIndex2;
    BMD* b = &Models[c->Object.Type];
    vec3_t vRelativePos, vPos, vLight;
    float fLight2, fScale;
    int Level;
    PART_t* w;

    for (int i = 0; i < 2; i++)
    {
        w = &c->Weapon[i];
        Level = w->Level;
        if (Level < 15 || w->Type == -1) continue;

        if (MODEL_BOW <= w->Type && w->Type < MODEL_STAFF)
        {
            weaponIndex = 27;
            weaponIndex2 = 28;
        }
        else
        {
            weaponIndex = (i == 0 ? 27 : 36);
            weaponIndex2 = (i == 0 ? 28 : 37);
        }

        switch (Level)
        {
        case 15:
        {
            Vector(0.0f, 0.0f, 0.0f, vRelativePos);
            b->TransformByObjectBone(vPos, &c->Object, weaponIndex, vRelativePos);
            Vector(1.0f, 0.6f, 0.0f, vLight);
            CreateSprite(BITMAP_LIGHT, vPos, 0.6f, vLight, &c->Object);

            Vector(10.0f, 0.0f, 0.0f, vRelativePos);
            b->TransformByObjectBone(vPos, &c->Object, weaponIndex, vRelativePos);
            Vector(1.0f, 0.6f, 0.0f, vLight);
            CreateSprite(BITMAP_LIGHT, vPos, 0.6f, vLight, &c->Object);

            Vector(20.0f, 0.0f, 0.0f, vRelativePos);
            b->TransformByObjectBone(vPos, &c->Object, weaponIndex, vRelativePos);
            Vector(1.0f, 0.6f, 0.0f, vLight);
            CreateSprite(BITMAP_LIGHT, vPos, 0.6f, vLight, &c->Object);

            Vector(10.0f, 0.0f, 0.0f, vRelativePos);
            b->TransformByObjectBone(vPos, &c->Object, weaponIndex2, vRelativePos);

            fLight2 = absf(sinf(WorldTime * 0.002f));
            Vector(0.7f * fLight2 + 0.3f, 0.1f * fLight2 + 0.1f, 0.0f, vLight);
            CreateSprite(BITMAP_MAGIC, vPos, 0.35f, vLight, &c->Object);

            Vector(1.0f, 1.0f, 1.0f, vLight);
            fScale = (float)(rand() % 60) * 0.01f;
            CreateSprite(BITMAP_FLARE_RED, vPos, 0.6f * fScale + 0.4f, vLight, &c->Object);

            Vector(1.0f, 0.2f, 0.0f, vLight);
            fScale = (float)(rand() % 80 + 10) * 0.01f * 1.0f;
            CreateParticleFpsChecked(BITMAP_LIGHTNING_MEGA1 + rand() % 3, vPos, c->Object.Angle, vLight, 0, fScale);
        }break;
        }//switch
    } //for

    int bornIndex[2]{}; // left, right;
    int gradeType[2]{}; // left, right;

    for (int k = 0; k < MAX_BODYPART; k++)
    {
        w = &c->BodyPart[k];
        Level = w->Level;

        if (k == 0) continue;
        if (Level < 15 || w->Type == -1) continue;

        switch (k)
        {
        case 1:
        {
            bornIndex[0] = 20;
            bornIndex[1] = -1;
            gradeType[0] = MODEL_15GRADE_ARMOR_OBJ_HEAD;
            gradeType[1] = -1;
        }break;
        case 2:
        {
            bornIndex[0] = 35;
            bornIndex[1] = 26;
            gradeType[0] = MODEL_15GRADE_ARMOR_OBJ_BODYLEFT;
            gradeType[1] = MODEL_15GRADE_ARMOR_OBJ_BODYRIGHT;
        }break;
        case 3:
        {
            bornIndex[0] = 3;
            bornIndex[1] = 10;
            gradeType[0] = MODEL_15GRADE_ARMOR_OBJ_PANTLEFT;
            gradeType[1] = MODEL_15GRADE_ARMOR_OBJ_PANTRIGHT;
        }break;
        case 4:
        {
            bornIndex[0] = 36;
            bornIndex[1] = 27;
            gradeType[0] = MODEL_15GRADE_ARMOR_OBJ_ARMLEFT;
            gradeType[1] = MODEL_15GRADE_ARMOR_OBJ_ARMRIGHT;
        }break;
        case 5:
        {
            bornIndex[0] = 4;
            bornIndex[1] = 11;
            gradeType[0] = MODEL_15GRADE_ARMOR_OBJ_BOOTLEFT;
            gradeType[1] = MODEL_15GRADE_ARMOR_OBJ_BOOTRIGHT;
        }break;
        default:
            bornIndex[0] = -1;
            bornIndex[1] = -1;
            gradeType[0] = -1;
            gradeType[1] = -1;
            break;
        }

        OBJECT* o = &c->Object;

        for (int m = 0; m < 2; m++)
        {
            if (gradeType[m] == -1) continue;

            switch (Level)
            {
            case 15: // +15
            {
                w->LinkBone = bornIndex[m];
                RenderLinkObject(0.f, 0.f, 0.f, c, w, gradeType[m], 0, 0, true, true);

                b->TransformByBoneMatrix(vPos, BoneTransform[0], o->Position);

                fLight2 = absf(sinf(WorldTime * 0.01f));
                Vector(0.2f * fLight2, 0.4f * fLight2, 1.0f * fLight2, vLight);
                CreateSprite(BITMAP_MAGIC, vPos, 0.12f, vLight, o);

                Vector(0.4f, 0.7f, 1.0f, vLight);
                CreateSprite(BITMAP_SHINY + 5, vPos, 0.4f, vLight, o);

                Vector(0.1f, 0.3f, 1.0f, vLight);
                CreateSprite(BITMAP_PIN_LIGHT, vPos, 0.6f, vLight, o, 90.0f);
            }break;
            }
        }
    } //for
}

extern float g_Luminosity;

void RenderPartObjectEffect(OBJECT* o, int Type, vec3_t Light, float Alpha, int ItemLevel, int ExcellentFlags, int ancientDiscriminator, int Select, int RenderType)
{
    int Level = ItemLevel;
    if (RenderType & RENDER_WAVE)
    {
        Level = 0;
    }
    BMD* b = &Models[Type];

    if (o->EnableShadow == true)
    {
        if (gMapManager.WorldActive == 7)
        {
            EnableAlphaTest();
            glColor4f(0.f, 0.f, 0.f, 0.2f);
        }
        else
        {
            DisableAlphaBlend();
            glColor3f(0.f, 0.f, 0.f);
        }
        bool bRenderShadow = true;

        if (gMapManager.InHellas())
        {
            bRenderShadow = false;
        }

        if (WD_10HEAVEN == gMapManager.WorldActive || o->m_bySkillCount == 3 || g_Direction.m_CKanturu.IsMayaScene())
        {
            bRenderShadow = false;
        }

        if (g_isCharacterBuff(o, eBuff_Cloaking))
        {
            bRenderShadow = false;
        }

        if (bRenderShadow)
        {
            if (o->m_bRenderShadow)
            {
                int iHiddenMesh = o->HiddenMesh;

                if (o->Type == MODEL_DREADFEAR)
                {
                    iHiddenMesh = 2;
                }
                else if (o->Type == MODEL_GAYION)
                {
                    iHiddenMesh = 0;
                }

                b->RenderBodyShadow(o->BlendMesh, iHiddenMesh);
            }
        }
        return;
    }

    switch (Type)
    {
    case MODEL_HORN_OF_DINORANT:Level = 8; break;
    case MODEL_ELITE_TRANSFER_SKELETON_RING:Level = 13; break;
    case MODEL_JACK_OLANTERN_TRANSFORMATION_RING:Level = 13; break;
    case MODEL_CHRISTMAS_TRANSFORMATION_RING:Level = 13; break;
    case MODEL_CHRISTMAS_STAR:Level = 13; break;
    case MODEL_GAME_MASTER_TRANSFORMATION_RING:Level = 13; break;
    case MODEL_TRANSFORMATION_RING:Level = 8; break;
    case MODEL_CAPE_OF_LORD:
    case MODEL_CAPE_OF_FIGHTER:
        Level = 0; break;
    case MODEL_EVENT + 16: Level = 0; break;
    case MODEL_JEWEL_OF_BLESS:
    case MODEL_JEWEL_OF_SOUL:
    case MODEL_JEWEL_OF_LIFE:
    case MODEL_JEWEL_OF_GUARDIAN:
    case MODEL_PACKED_JEWEL_OF_LIFE:
    case MODEL_PACKED_JEWEL_OF_GUARDIAN:
    case MODEL_PACKED_JEWEL_OF_CHAOS:
    case MODEL_COMPILED_CELE:
    case MODEL_COMPILED_SOUL:
    case MODEL_JEWEL_OF_CHAOS:Level = 8; break;
    case MODEL_WING_OF_STORM:
    case MODEL_WING_OF_ETERNAL:
    case MODEL_WING_OF_ILLUSION:
    case MODEL_WING_OF_RUIN:
    case MODEL_CAPE_OF_EMPEROR:
    case MODEL_WING_OF_CURSE:
    case MODEL_WINGS_OF_DESPAIR:
    case MODEL_WING_OF_DIMENSION:
    case MODEL_WINGS_OF_SPIRITS:
    case MODEL_WINGS_OF_SOUL:
    case MODEL_WINGS_OF_DRAGON:
    case MODEL_WINGS_OF_DARKNESS:
    case MODEL_WING:
    case MODEL_WINGS_OF_HEAVEN:
    case MODEL_CAPE_OF_OVERRULE:
    case MODEL_WINGS_OF_SATAN:Level = 0; break;
    case MODEL_ORB_OF_TWISTING_SLASH:Level = 9; break;
    case MODEL_ORB_OF_SUMMONING:Level = 0; break;
    case MODEL_ORB_OF_RAGEFUL_BLOW:
    case MODEL_ORB_OF_IMPALE:
    case MODEL_ORB_OF_FIRE_SLASH:
    case MODEL_ORB_OF_PENETRATION:
    case MODEL_ORB_OF_ICE_ARROW:
    case MODEL_ORB_OF_DEATH_STAB:
    case MODEL_CRYSTAL_OF_DESTRUCTION:
    case MODEL_CRYSTAL_OF_MULTI_SHOT:
    case MODEL_CRYSTAL_OF_RECOVERY:
    case MODEL_CRYSTAL_OF_FLAME_STRIKE:
    {
        Level = 9;
        break;
    }
    case MODEL_ORB_OF_GREATER_FORTITUDE:
        Level = 9;
        break;
    case MODEL_POTION + 12:
        Level = 8;
        break;
    case MODEL_DEVILS_EYE:
    case MODEL_DEVILS_KEY:
    case MODEL_DEVILS_INVITATION:
    {
        if (Level <= 6)
        {
            Level /= 2;
        }
        else
        {
            Level = 13;
        }
    }
    break;
    case MODEL_POTION + 20:Level = 9; break;
    case MODEL_JEWEL_OF_CREATION:Level = 8; break;
    case MODEL_PACKED_JEWEL_OF_CREATION:Level = 8; break;
    case MODEL_TEAR_OF_ELF:Level = 8; break;
    case MODEL_SOUL_SHARD_OF_WIZARD:Level = 8; break;
    case MODEL_PACKED_GEMSTONE:
    case MODEL_PACKED_JEWEL_OF_HARMONY:
    case MODEL_PACKED_LOWER_REFINE_STONE:
    case MODEL_PACKED_HIGHER_REFINE_STONE:
    case MODEL_GEMSTONE:
        Level = 0; break;
    case MODEL_JEWEL_OF_HARMONY:
        Level = 0; break;
    case MODEL_LOWER_REFINE_STONE:
        Level = 0; break;
    case MODEL_HIGHER_REFINE_STONE:
        Level = 0; break;
    case MODEL_ILLUSION_SORCERER_COVENANT:
    case MODEL_POTION + 64:
        Level = 0; break;
    case MODEL_FLAME_OF_CONDOR:
    case MODEL_FEATHER_OF_CONDOR:
        Level = 0; break;
    case MODEL_EVENT + 4:Level = 0; break;
    case MODEL_EVENT + 6:
        if (Level == 13)
        {
            Level = 13;
        }
        else
        {
            Level = 9;
        }
        break;
    case MODEL_EVENT + 7:Level = 0; break;
    case MODEL_EVENT + 8:Level = 0; break;
    case MODEL_EVENT + 9:Level = 8; break;
    case MODEL_EVENT + 5:
    {
        Level = 0;
    }
    break;
    case MODEL_EVENT + 10:
        Level = (Level - 8) * 2 + 1;
        break;
    case MODEL_EVENT + 11:
        Level--;
        break;
    case MODEL_EVENT + 12:
        Level = 0;
        break;
    case MODEL_EVENT + 13:
        Level = 0;
        break;
    case MODEL_EVENT + 14:
        Level += 7;
        break;
    case MODEL_EVENT + 15:
        Level = 8;
        break;
    case MODEL_EVENT:
    case MODEL_EVENT + 1:Level = 8; break;
    case MODEL_BOLT: Level >= 1 ? Level = Level * 2 + 1 : Level = 0; break;
    case MODEL_ARROWS:Level >= 1 ? Level = Level * 2 + 1 : Level = 0; break;
#ifdef LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM
    case MODEL_HELPER + 134:
        Level = 13;
        break;
#endif //LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM
    }

    if (g_pOption->GetRenderLevel() < 4)
    {
        Level = min(Level, g_pOption->GetRenderLevel() * 2 + 5);
    }

    if (o->Type == MODEL_BILL_OF_BALROG)
    {
        Vector(0.5f, 0.5f, 1.5f, b->BodyLight);
        b->StreamMesh = 0;
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh, BITMAP_CHROME);
        b->StreamMesh = -1;
    }
    else if (o->Type == MODEL_POTION + 27)
    {
        Vector(1.f, 1.f, 1.f, b->BodyLight);
        b->StreamMesh = 0;
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        if (Level == 1)
        {
        }
        else if (Level == 2)
        {
            b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
            Vector(0.75f, 0.65f, 0.5f, b->BodyLight);
            b->RenderMesh(1, RENDER_BRIGHT | RENDER_CHROME, o->Alpha, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_CHROME);
        }
        else if (Level == 3)
        {
            b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
            b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
            Vector(0.75f, 0.65f, 0.5f, b->BodyLight);
            b->RenderMesh(1, RENDER_BRIGHT | RENDER_CHROME, o->Alpha, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_CHROME);
            b->RenderMesh(2, RENDER_BRIGHT | RENDER_CHROME, o->Alpha, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_CHROME);
        }
        b->StreamMesh = -1;
        return;
    }
    else if (o->Type == MODEL_FIRECRACKER)
    {
        b->StreamMesh = 0;
        o->BlendMeshLight = 1.f;
        Vector(1.f, 1.f, 1.f, b->BodyLight);
        b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        Vector(1.f, 0.f, 0.f, b->BodyLight);
        b->LightEnable = true;
        b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderMesh(1, RENDER_BRIGHT | RENDER_CHROME, o->Alpha, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->StreamMesh = -1;
        return;
    }
    else if (o->Type == MODEL_GM_GIFT)
    {
        Vector(1.f, 1.f, 1.f, b->BodyLight);
        b->RenderBody(RENDER_TEXTURE, o->Alpha, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->LightEnable = true;
        Vector(0.1f, 0.6f, 0.4f, b->BodyLight);
        o->Alpha = 0.5f;
        b->RenderMesh(0, RENDER_BRIGHT | RENDER_CHROME, o->Alpha, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        return;
    }
    else if (o->Type == MODEL_EVENT + 14 && Level == 9)
    {
        Vector(0.3f, 0.8f, 1.f, b->BodyLight);
        b->RenderBody(RENDER_TEXTURE, o->Alpha, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        Vector(1.f, 0.8f, 0.3f, b->BodyLight);
        b->RenderBody(RENDER_BRIGHT | RENDER_CHROME, o->Alpha, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        return;
    }
    else if ((o->Type >= MODEL_SCROLL_OF_FIREBURST && o->Type <= MODEL_SCROLL_OF_ELECTRIC_SPARK)
        || o->Type == MODEL_SCROLL_OF_FIRE_SCREAM
        || (o->Type == MODEL_SCROLL_OF_CHAOTIC_DISEIER)
        )
    {
        b->BeginRender(o->Alpha);
        glColor3f(1.f, 1.f, 1.f);
        o->BlendMeshLight = 1.f;
        b->RenderMesh(0, RENDER_TEXTURE, Alpha, -1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        o->BlendMeshLight = sinf(WorldTime * 0.001f) * 0.5f + 0.5f;
        b->RenderMesh(1, RENDER_BRIGHT | RENDER_TEXTURE, Alpha, 1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(2, RENDER_BRIGHT | RENDER_TEXTURE, Alpha, 2, 1 - o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->EndRender();
        return;
    }
    else if (o->Type == MODEL_SPIRIT)
    {
        b->RenderBody(RENDER_TEXTURE, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        switch (Level)
        {
        case 0:
            b->RenderMesh(0, RENDER_BRIGHT | RENDER_TEXTURE, Alpha, 0, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
            break;

        case 1:
            Vector(0.3f, 0.8f, 1.f, b->BodyLight);
            b->RenderMesh(0, RENDER_BRIGHT | RENDER_TEXTURE, Alpha, 0, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
            break;
        }
        return;
    }
    else if (o->Type >= MODEL_POTION && o->Type <= MODEL_LARGE_MANA_POTION)
    {
        if (Level > 0)
            Level = 7;
    }
    else if ((o->Type >= MODEL_SEED_FIRE && o->Type <= MODEL_SEED_EARTH)
        || (o->Type >= MODEL_SPHERE_MONO && o->Type <= MODEL_SPHERE_5)
        || (o->Type >= MODEL_SEED_SPHERE_FIRE_1 && o->Type <= MODEL_SEED_SPHERE_EARTH_5))
    {
        Level = 0;
    }
    else if (o->Type == MODEL_FRUITS)
    {
        switch (Level)
        {
        case 0: Vector(0.0f, 0.5f, 1.0f, b->BodyLight); break;
        case 1: Vector(1.0f, 0.2f, 0.0f, b->BodyLight); break;
        case 2: Vector(1.0f, 0.8f, 0.0f, b->BodyLight); break;
        case 3: Vector(0.6f, 0.8f, 0.4f, b->BodyLight); break;
        }
        b->RenderBody(RENDER_METAL, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh, BITMAP_CHROME + 1);
        b->RenderBody(RENDER_BRIGHT | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh, BITMAP_CHROME + 1);
        return;
    }
    else if (o->Type == MODEL_EVENT + 11)
    {
        Vector(0.9f, 0.9f, 0.9f, b->BodyLight);
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderBody(RENDER_CHROME | RENDER_BRIGHT, 0.5f, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, -1, BITMAP_CHROME + 1);
        return;
    }
    else if (Type == MODEL_EVENT + 5 && ItemLevel == 14)
    {
        Vector(0.2f, 0.3f, 0.5f, b->BodyLight);
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        Vector(0.1f, 0.3f, 1.f, b->BodyLight);
        b->RenderBody(RENDER_CHROME | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderBody(RENDER_METAL | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        return;
    }
    else if (Type == MODEL_EVENT + 5 && ItemLevel == 15)
    {
        Vector(0.5f, 0.3f, 0.2f, b->BodyLight);
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        Vector(1.f, 0.3f, 0.1f, b->BodyLight);
        b->RenderBody(RENDER_CHROME | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        b->RenderBody(RENDER_METAL | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
        return;
    }
    else if (o->Type == MODEL_BLOOD_BONE)
    {
        Vector(.9f, .1f, .1f, b->BodyLight);
        o->BlendMeshTexCoordU = sinf(gMapManager.WorldActive * 0.0001f);
        o->BlendMeshTexCoordV = -WorldTime * 0.0005f;
        Models[o->Type].StreamMesh = 0;
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh, BITMAP_CHROME);
        Models[o->Type].StreamMesh = -1;
        Vector(.9f, .9f, .9f, b->BodyLight);
        b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        return;
    }
    else if (o->Type == MODEL_INVISIBILITY_CLOAK)
    {
        Vector(0.8f, 0.8f, 0.8f, b->BodyLight);
        float sine = float(sinf(WorldTime * 0.002f) * 0.3f) + 0.7f;

        b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, 1.0f, 0, sine, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        return;
    }
    else if (o->Type == MODEL_EVENT + 12)
    {
        float Luminosity = (float)sinf((WorldTime) * 0.002f) * 0.35f + 0.65f;
        vec3_t p, Position, EffLight;
        Vector(0.f, 0.f, 15.f, p);

        float Scale = Luminosity * 0.8f + 2.f;
        Vector(Luminosity * 0.32f, Luminosity * 0.32f, Luminosity * 2.f, EffLight);

        b->TransformPosition(BoneTransform[0], p, Position);
        VectorAdd(Position, o->Position, Position);

        CreateSprite(BITMAP_SPARK + 1, Position, Scale, EffLight, o);
    }
    else if (o->Type == MODEL_EVENT + 6 && Level == 13)
    {
        Vector(0.4f, 0.6f, 1.0f, b->BodyLight);
        b->RenderBody(RENDER_COLOR, 1.0f, 0, 1.0f, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderBody(RENDER_CHROME | RENDER_BRIGHT, 1.0f, 0, 1.0f, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        return;
    }
    else if (o->Type == MODEL_EVENT + 13)
    {
        float Luminosity = (float)sinf((WorldTime) * 0.002f) * 0.35f + 0.65f;
        vec3_t p, Position, EffLight;
        Vector(0.f, -5.f, -15.f, p);

        float Scale = Luminosity * 0.8f + 2.5f;
        Vector(Luminosity * 2.f, Luminosity * 0.32f, Luminosity * 0.32f, EffLight);

        b->StreamMesh = 0;
        o->BlendMeshTexCoordV = (int)-WorldTime % 4000 * 0.00025f;

        b->TransformPosition(BoneTransform[0], p, Position);
        VectorAdd(Position, o->Position, Position);

        CreateSprite(BITMAP_SPARK + 1, Position, Scale, EffLight, o);
    }
    else if (o->Type == MODEL_DEVILS_EYE)
    {
        float   sine = (float)sinf(WorldTime * 0.002f) * 10.f + 15.65f;

        o->BlendMesh = 1;
        o->BlendMeshLight = sine;
        o->BlendMeshTexCoordV = (int)WorldTime % 2000 * 0.0005f;
        o->Alpha = 2.0f;

        float Luminosity = sine;
        Vector(Luminosity / 5.0f, Luminosity / 5.0f, Luminosity / 5.0f, o->Light);
    }
    else if (o->Type == MODEL_DEVILS_KEY)
    {
        float Luminosity = (float)sinf((WorldTime) * 0.002f) * 0.35f + 0.65f;
        vec3_t p, Position, EffLight;
        Vector(0.f, 0.f, 0.f, p);

        float Scale = Luminosity * 0.8f;
        Vector(Luminosity * 2, Luminosity * 0.32f, Luminosity * 0.32f, EffLight);

        b->TransformPosition(BoneTransform[1], p, Position);
        VectorAdd(Position, o->Position, Position);
        CreateSprite(BITMAP_SPARK + 1, Position, Scale, EffLight, o);

        b->TransformPosition(BoneTransform[2], p, Position);
        VectorAdd(Position, o->Position, Position);
        CreateSprite(BITMAP_SPARK + 1, Position, Scale, EffLight, o);
    }
    else if (o->Type == MODEL_DEVILS_INVITATION)
    {
        float Luminosity = (float)sinf((WorldTime) * 0.002f) * 0.35f + 0.65f;
        vec3_t p, Position, EffLight;
        Vector(0.f, 0.f, 0.f, p);

        float Scale = Luminosity * 0.8f;
        Vector(Luminosity * 2, Luminosity * 0.32f, Luminosity * 0.32f, EffLight);

        b->TransformPosition(BoneTransform[9], p, Position);
        VectorAdd(Position, o->Position, Position);
        CreateSprite(BITMAP_SPARK + 1, Position, Scale, EffLight, o);

        b->TransformPosition(BoneTransform[10], p, Position);
        VectorAdd(Position, o->Position, Position);
        CreateSprite(BITMAP_SPARK + 1, Position, Scale, EffLight, o);
    }
    else if (o->Type == MODEL_POTION + 21)
    {
        float Luminosity = (float)sinf((WorldTime) * 0.002f) * 0.25f + 0.75f;
        vec3_t EffLight;

        Vector(Luminosity * 1.f, Luminosity * 0.5f, Luminosity * 0.f, EffLight);
        CreateSprite(BITMAP_SPARK + 1, o->Position, 2.5f, EffLight, o);
    }
    else if (o->Type == MODEL_WINGS_OF_DRAGON)
    {
        o->BlendMeshLight = (float)(sinf(WorldTime * 0.001f) + 1.f) / 4.f;
    }
    else if (o->Type == MODEL_WINGS_OF_SOUL)
    {
        o->BlendMeshLight = (float)sinf(WorldTime * 0.001f) + 1.1f;
    }
    else if (Type == MODEL_RED_SPIRIT_PANTS || Type == MODEL_RED_SPIRIT_HELM)
    {
        o->BlendMeshLight = sinf(WorldTime * 0.001f) * 0.4f + 0.6f;
    }
    else if (o->Type == MODEL_STAFF_OF_KUNDUN)
    {
        o->BlendMeshLight = sinf(WorldTime * 0.004f) * 0.3f + 0.7f;
    }
    else if (Type == MODEL_DIVINE_HELM || Type == MODEL_DIVINE_GLOVES || Type == MODEL_DIVINE_BOOTS)
    {
        o->BlendMeshLight = 1.f;
    }
    else if (Type == MODEL_SIEGE_POTION)
    {
        switch (Level)
        {
        case 0: o->HiddenMesh = 1; break;
        case 1: o->HiddenMesh = 0; break;
        }
    }
    else if (Type == MODEL_HELPER + 7)
    {
        switch (Level)
        {
        case 0: o->HiddenMesh = 1; break;
        case 1: o->HiddenMesh = 0; break;
        }
    }
    else if (Type == MODEL_LIFE_STONE_ITEM)
    {
        o->HiddenMesh = 1;
    }
    else if (Type == MODEL_EVENT + 18)
    {
        o->BlendMesh = 1;
    }
    else if (o->Type == MODEL_WINGS_OF_DARKNESS)
    {
        vec3_t  posCenter, p, Position, Light;
        float   Scale = sinf(WorldTime * 0.004f) * 0.3f + 0.3f;

        Scale = (Scale * 10.f) + 20.f;

        Vector(0.6f, 0.3f, 0.8f, Light);

        Vector(0.f, 0.f, 0.f, p);

        for (int i = 0; i < 5; ++i)
        {
            b->TransformPosition(BoneTransform[22 - i], p, posCenter, true);
            b->TransformPosition(BoneTransform[30 - i], p, Position, true);
            if (rand_fps_check(1))
            {
                CreateJoint(BITMAP_JOINT_THUNDER, Position, posCenter, o->Angle, 14, o, Scale);
                CreateJoint(BITMAP_JOINT_SPIRIT, posCenter, Position, o->Angle, 4, o, Scale + 5);
            }

            CreateSprite(BITMAP_FLARE_BLUE, posCenter, Scale / 28.f, Light, o);
        }

        for (int i = 0; i < 5; ++i)
        {
            b->TransformPosition(BoneTransform[7 - i], p, posCenter, true);
            b->TransformPosition(BoneTransform[11 + i], p, Position, true);
            if (rand_fps_check(1))
            {
                CreateJoint(BITMAP_JOINT_THUNDER, Position, posCenter, o->Angle, 14, o, Scale);
                CreateJoint(BITMAP_JOINT_SPIRIT, posCenter, Position, o->Angle, 4, o, Scale + 5);
            }

            CreateSprite(BITMAP_FLARE_BLUE, posCenter, Scale / 28.f, Light, o);
        }
    }
    else if (Type == MODEL_WING_OF_STORM)
    {
        vec3_t vRelativePos, vPos, vLight;
        Vector(0.f, 0.f, 0.f, vRelativePos);
        Vector(0.f, 0.f, 0.f, vPos);
        Vector(0.f, 0.f, 0.f, vLight);

        float fLuminosity = absf(sinf(WorldTime * 0.0004f)) * 0.4f;
        Vector(0.5f + fLuminosity, 0.5f + fLuminosity, 0.5f + fLuminosity, vLight);
        int iBone[] = { 9, 20, 19, 10, 18,
                        28, 27, 36, 35, 38,
                        37, 53, 48, 62, 70,
                        72, 71, 78, 79, 80,
                        87, 90, 91, 106, 102 };
        float fScale = 0.f;

        for (int i = 0; i < 25; ++i)
        {
            b->TransformPosition(BoneTransform[iBone[i]], vRelativePos, vPos, true);
            fScale = 0.5f;// (rand()%10) * 0.05f + 0.3f;
            CreateSprite(BITMAP_CLUD64, vPos, fScale, vLight, o, WorldTime * 0.01f, 1);
        }

        int iBoneThunder[] = { 11, 21, 29, 63, 81, 89 };
        if (rand_fps_check(2))
        {
            for (int i = 0; i < 6; ++i)
            {
                b->TransformPosition(BoneTransform[iBoneThunder[i]], vRelativePos, vPos, true);
                if (rand_fps_check(20))
                {
                    Vector(0.6f, 0.6f, 0.9f, vLight);
                    CreateEffect(MODEL_FENRIR_THUNDER, vPos, o->Angle, vLight, 1, o);
                }
            }
        }

        int iBoneLight[] = { 64, 61, 69, 77, 86,
                            98, 97, 99, 104, 103,
                            105, 12, 8, 17, 26,
                            34, 52, 44, 51, 50,
                            49, 45 };

        fScale = absf(sinf(WorldTime * 0.003f)) * 0.2f;

        for (int i = 0; i < 22; ++i)
        {
            b->TransformPosition(BoneTransform[iBoneLight[i]], vRelativePos, vPos, true);
            if (iBoneLight[i] == 12 || iBoneLight[i] == 64 || iBoneLight[i] == 98 || iBoneLight[i] == 52)
            {
                Vector(0.9f, 0.0f, 0.0f, vLight);
                CreateSprite(BITMAP_LIGHT, vPos, fScale + 1.4f, vLight, o);
            }
            else
            {
                Vector(0.8f, 0.5f, 0.2f, vLight);
                CreateSprite(BITMAP_LIGHT, vPos, fScale + 0.3f, vLight, o);
            }
        }
    }
    else if (Type == MODEL_WING_OF_ETERNAL)
    {
        vec3_t  p, Position, Light;
        Vector(0.f, 0.f, 0.f, p);
        float Scale = absf(sinf(WorldTime * 0.003f)) * 0.2f;
        float Luminosity = absf(sinf(WorldTime * 0.003f)) * 0.3f;

        Vector(0.5f + Luminosity, 0.5f + Luminosity, 0.6f + Luminosity, Light);
        //int iRedFlarePos[] = { 25, 32, 53, 15, 9, 35 };
        int iRedFlarePos[] = { 24, 31, 15, 8, 53, 35 };
        for (int i = 0; i < 6; ++i)
        {
            b->TransformPosition(BoneTransform[iRedFlarePos[i]], p, Position, true);
            CreateSprite(BITMAP_LIGHT, Position, Scale + 1.3f, Light, o);
        }

        Vector(0.1f, 0.1f, 0.9f, Light);
        //int iGreenFlarePos[] = { 23, 22, 24, 34, 5, 31, 14, 12, 27, 8, 6, 7, 16, 13, 56, 37, 58, 40, 39, 38 };
        int iGreenFlarePos[] = { 22, 23, 25, 29, 30, 28, 32, 13, 16, 14, 12, 9, 7, 6, 57, 58, 40, 39 };

        for (int i = 0; i < 18; ++i)
        {
            b->TransformPosition(BoneTransform[iGreenFlarePos[i]], p, Position, true);
            CreateSprite(BITMAP_LIGHT, Position, Scale + 1.5f, Light, o);
        }
        int iGreenFlarePos2[] = { 56, 38, 51, 45 };

        for (int i = 0; i < 4; ++i)
        {
            b->TransformPosition(BoneTransform[iGreenFlarePos2[i]], p, Position, true);
            CreateSprite(BITMAP_LIGHT, Position, Scale + 0.5f, Light, o);
        }
    }
    else if (Type == MODEL_WING_OF_ILLUSION)
    {
        vec3_t  p, Position, Light;
        Vector(0.f, 0.f, 0.f, p);
        float Scale = absf(sinf(WorldTime * 0.002f)) * 0.2f;
        float Luminosity = absf(sinf(WorldTime * 0.002f)) * 0.4f;

        Vector(0.5f + Luminosity, 0.0f + Luminosity, 0.0f + Luminosity, Light);
        int iRedFlarePos[] = { 5, 6, 7, 8, 18, 19, 23, 24, 25, 27, 37, 38 };
        for (int i = 0; i < 12; ++i)
        {
            b->TransformPosition(BoneTransform[iRedFlarePos[i]], p, Position, true);
            CreateSprite(BITMAP_FLARE, Position, Scale + 0.6f, Light, o);
        }

        Vector(0.0f + Luminosity, 0.5f + Luminosity, 0.0f + Luminosity, Light);
        int iGreenFlarePos[] = { 4, 9, 13, 14, 26, 32, 31, 33 };

        for (int i = 0; i < 8; ++i)
        {
            b->TransformPosition(BoneTransform[iGreenFlarePos[i]], p, Position, true);
            CreateSprite(BITMAP_LIGHT, Position, 1.3f, Light, o);
        }

        Vector(1.0f, 1.0f, 1.0f, Light);
        float fLumi = (sinf(WorldTime * 0.004f) + 1.0f) * 0.05f;
        Vector(0.8f + fLumi, 0.8f + fLumi, 0.3f + fLumi, Light);
        CreateSprite(BITMAP_LIGHT, Position, 0.4f, Light, o, 0.5f);
        if (rand_fps_check(2))
        {
            b->TransformPosition(BoneTransform[13], p, Position, true);
            CreateParticle(BITMAP_SHINY, Position, o->Angle, Light, 5, 0.5f);
            b->TransformPosition(BoneTransform[31], p, Position, true);
            CreateParticle(BITMAP_SHINY, Position, o->Angle, Light, 5, 0.5f);
        }
    }
    else if (Type == MODEL_WING_OF_RUIN)
    {
        vec3_t  p, Position, Light;
        Vector(0.f, 0.f, 0.f, p);
        float Scale = absf(sinf(WorldTime * 0.003f)) * 0.2f;
        float Luminosity = absf(sinf(WorldTime * 0.003f)) * 0.3f;

        Vector(0.7f + Luminosity, 0.5f + Luminosity, 0.8f + Luminosity, Light);
        int iRedFlarePos[] = { 6, 15, 24, 56, 47, 38 };
        for (int i = 0; i < 6; ++i)
        {
            b->TransformPosition(BoneTransform[iRedFlarePos[i]], p, Position, true);
            CreateSprite(BITMAP_LIGHT, Position, Scale + 1.5f, Light, o);
        }

        Vector(0.6f, 0.4f, 0.7f, Light);
        int iSparkPos[] = { 7, 16, 25, 57, 48, 39,
                            11, 22, 31, 63, 54, 40,
                            10, 21, 30, 62, 53, 41,
                            9, 20, 29, 61, 52, 42,
                            8, 19, 28, 60, 51, 43,
                            18, 27, 59, 50,
                            17, 26, 58, 49 };
        int iNumParticle = 1;

        for (int i = 0; i < 6; ++i)
        {
            b->TransformPosition(BoneTransform[iSparkPos[i]], p, Position, true);
            for (int j = 0; j < iNumParticle; ++j)
                if (rand_fps_check(1))
                    CreateParticle(BITMAP_CHROME_ENERGY2, Position, o->Angle, Light, 0, 0.1f);
        }

        for (int i = 6; i < 18; ++i)
        {
            b->TransformPosition(BoneTransform[iSparkPos[i]], p, Position, true);
            for (int j = 0; j < iNumParticle; ++j)
                if (rand_fps_check(1))
                    CreateParticle(BITMAP_CHROME_ENERGY2, Position, o->Angle, Light, 0, 0.3f);
        }

        for (int i = 18; i < 30; ++i)
        {
            b->TransformPosition(BoneTransform[iSparkPos[i]], p, Position, true);
            for (int j = 0; j < iNumParticle; ++j)
                if (rand_fps_check(1))
                    CreateParticle(BITMAP_CHROME_ENERGY2, Position, o->Angle, Light, 0, 0.5f);
        }

        for (int i = 30; i < 38; ++i)
        {
            b->TransformPosition(BoneTransform[iSparkPos[i]], p, Position, true);
            for (int j = 0; j < iNumParticle; ++j)
                if (rand_fps_check(1))
                    CreateParticle(BITMAP_CHROME_ENERGY2, Position, o->Angle, Light, 0, 0.7f);
        }
    }
    else if (Type == MODEL_WING_OF_DIMENSION)
    {
        vec3_t  p, Position, Light;
        Vector(0.f, 0.f, 0.f, p);
        float Scale = absf(sinf(WorldTime * 0.002f)) * 0.2f;
        float Luminosity = absf(sinf(WorldTime * 0.002f)) * 0.4f;

        Vector((1.0f + Luminosity) / 2.f, (0.7f + Luminosity) / 2.f, (0.2f + Luminosity) / 2.f, Light);
        int iFlarePos0[] = { 7, 30, 31, 43, 8, 20 };

        int icnt;
        for (icnt = 0; icnt < 2; ++icnt)
        {
            b->TransformPosition(BoneTransform[iFlarePos0[icnt]], p, Position, true);
            CreateSprite(BITMAP_FLARE, Position, Scale + 2.0f, Light, o);
        }
        Vector((1.0f + Luminosity) / 4.f, (0.7f + Luminosity) / 4.f, (0.2f + Luminosity) / 4.f, Light);
        for (; icnt < 6; ++icnt)
        {
            b->TransformPosition(BoneTransform[iFlarePos0[icnt]], p, Position, true);
            CreateSprite(BITMAP_FLARE, Position, Scale + 0.5f, Light, o);
        }

        Vector((0.5f + Luminosity) / 2.f, (0.1f + Luminosity) / 2.f, (0.4f + Luminosity) / 2.f, Light);
        int iGreenFlarePos[] = { 29, 38, 42, 19, 15, 6 };

        for (int i = 0; i < 6; ++i)
        {
            b->TransformPosition(BoneTransform[iGreenFlarePos[i]], p, Position, true);
            CreateSprite(BITMAP_FLARE, Position, Scale + 2.0f, Light, o);
        }
    }

    if (!o->EnableShadow)
    {
        float Luminosity = 1.f;

        if (g_isCharacterBuff(o, eBuff_Cloaking))
        {
            Alpha = 0.5f;
            Vector(1.f, 1.f, 1.f, b->BodyLight);
            b->RenderBody(RENDER_BRIGHT | RENDER_CHROME5, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU * 8.f, o->BlendMeshTexCoordV * 2.f, -1, BITMAP_CHROME2);
        }
        else if (g_isCharacterBuff(o, eDeBuff_Poison) && g_isCharacterBuff(o, eDeBuff_Freeze))
        {
            Vector(Luminosity * 0.3f, Luminosity * 1.f, Luminosity * 1.f, b->BodyLight);
            RenderPartObjectBody(b, o, Type, Alpha, RenderType);
        }
        else if (g_isCharacterBuff(o, eDeBuff_BlowOfDestruction))
        {
            Vector(Luminosity * 0.3f, Luminosity * 1.f, Luminosity * 1.f, b->BodyLight);
            RenderPartObjectBody(b, o, Type, Alpha, RenderType);
        }
        else if (g_isCharacterBuff(o, eDeBuff_Poison))
        {
            Vector(Luminosity * 0.3f, Luminosity * 1.f, Luminosity * 0.5f, b->BodyLight);
            RenderPartObjectBody(b, o, Type, Alpha, RenderType);
        }
        else if (g_isCharacterBuff(o, eDeBuff_Stun))
        {
            DeleteEffect(BITMAP_SKULL, o, 5);
            CreateEffect(BITMAP_SKULL, o->Position, o->Angle, Light, 5, o);
            Vector(Luminosity * 0.f, Luminosity * 0.f, Luminosity * 1.0f, b->BodyLight);
            RenderPartObjectBody(b, o, Type, Alpha, RenderType);
        }
        else if (g_isCharacterBuff(o, eDeBuff_Freeze))
        {
            Vector(Luminosity * 0.3f, Luminosity * 0.5f, Luminosity * 1.f, b->BodyLight);
            RenderPartObjectBody(b, o, Type, Alpha, RenderType);
        }
        else if (g_isCharacterBuff(o, eDeBuff_BlowOfDestruction))
        {
            Vector(Luminosity * 0.3f, Luminosity * 0.5f, Luminosity * 1.f, b->BodyLight);
            RenderPartObjectBody(b, o, Type, Alpha, RenderType);
        }
        else if (g_isCharacterBuff(o, eDeBuff_Harden))
        {
            Vector(Luminosity * 0.3f, Luminosity * 0.5f, Luminosity * 1.f, b->BodyLight);
            RenderPartObjectBody(b, o, Type, Alpha, RenderType);
        }
        else if (Level < 3 || o->Type == MODEL_ZEN)
        {
            if (o->Type == MODEL_POTION + 64)
            {
                RenderPartObjectBody(b, o, Type, Alpha, RenderType);
                RenderPartObjectBodyColor2(b, o, Type, 0.5f, RENDER_TEXTURE | RENDER_BRIGHT | (RenderType & RENDER_EXTRA), 0.5f);
                RenderPartObjectBodyColor2(b, o, Type, 1.f, RENDER_CHROME4 | RENDER_BRIGHT | (RenderType & RENDER_EXTRA), 1.f);
            }
            else if (o->Type == MODEL_ILLUSION_SORCERER_COVENANT)
            {
                VectorCopy(Light, b->BodyLight);
                RenderPartObjectBody(b, o, Type, Alpha, RenderType);
                RenderPartObjectBodyColor2(b, o, Type, 1.5f, RENDER_CHROME2 | RENDER_BRIGHT | (RenderType & RENDER_EXTRA), 1.5f);
                RenderPartObjectBodyColor2(b, o, Type, 1.f, RENDER_CHROME4 | RENDER_BRIGHT | (RenderType & RENDER_EXTRA), 1.f);
            }
            else if (o->Type == MODEL_JEWEL_OF_HARMONY || o->Type == MODEL_MOONSTONE_PENDANT)
            {
                VectorCopy(Light, b->BodyLight);
                RenderPartObjectBody(b, o, Type, Alpha, RenderType);

                RenderPartObjectBodyColor2(b, o, Type, 1.5f, RENDER_CHROME2 | RENDER_BRIGHT | (RenderType & RENDER_EXTRA), 1.5f);
                RenderPartObjectBodyColor2(b, o, Type, 1.f, RENDER_CHROME4 | RENDER_BRIGHT | (RenderType & RENDER_EXTRA), 1.f);
            }
            else
                if (o->Type == MODEL_HELPER + 43 || o->Type == MODEL_HELPER + 93)
                {
                    Vector(Light[0] * 0.9f, Light[1] * 0.9f, Light[2] * 0.9f, b->BodyLight);
                    RenderPartObjectBody(b, o, Type, Alpha, RenderType);
                    RenderPartObjectBodyColor2(b, o, Type, 1.5f, RENDER_CHROME2 | RENDER_BRIGHT | (RenderType & RENDER_EXTRA), 1.5f);
                    RenderPartObjectBodyColor2(b, o, Type, 1.f, RENDER_CHROME4 | RENDER_BRIGHT | (RenderType & RENDER_EXTRA), 1.f);
                }
                else if (o->Type == MODEL_HELPER + 44 || o->Type == MODEL_HELPER + 94 || o->Type == MODEL_HELPER + 116)
                {
                    Vector(Light[0] * 0.9f, Light[1] * 0.9f, Light[2] * 0.9f, b->BodyLight);
                    RenderPartObjectBody(b, o, Type, Alpha, RenderType);
                    RenderPartObjectBodyColor2(b, o, Type, 1.5f, RENDER_CHROME2 | RENDER_BRIGHT | (RenderType & RENDER_EXTRA), 1.5f);
                    RenderPartObjectBodyColor2(b, o, Type, 1.f, RENDER_CHROME4 | RENDER_BRIGHT | (RenderType & RENDER_EXTRA), 1.f);
                }
                else if (o->Type == MODEL_HELPER + 45)
                {
                    Vector(Light[0] * 0.9f, Light[1] * 0.9f, Light[2] * 0.9f, b->BodyLight);
                    RenderPartObjectBody(b, o, Type, Alpha, RenderType);
                    RenderPartObjectBodyColor2(b, o, Type, 1.5f, RENDER_CHROME2 | RENDER_BRIGHT | (RenderType & RENDER_EXTRA), 1.5f);
                    RenderPartObjectBodyColor2(b, o, Type, 1.f, RENDER_CHROME4 | RENDER_BRIGHT | (RenderType & RENDER_EXTRA), 1.f);
                }
                else
                {
                    VectorCopy(Light, b->BodyLight);
                    RenderPartObjectBody(b, o, Type, Alpha, RenderType);
                }
        }
        else if (Level < 5)
        {
            vec3_t l;
            Vector(g_Luminosity, g_Luminosity * 0.6f, g_Luminosity * 0.6f, l);
            VectorMul(l, Light, b->BodyLight);
            RenderPartObjectBody(b, o, Type, Alpha, RenderType);
        }
        else if (Level < 7)
        {
            vec3_t l;
            Vector(g_Luminosity * 0.5f, g_Luminosity * 0.7f, g_Luminosity, l);
            VectorMul(l, Light, b->BodyLight);
            RenderPartObjectBody(b, o, Type, Alpha, RenderType);
        }
        else if (g_pOption->GetRenderLevel())
        {
            if (Level < 8 && g_pOption->GetRenderLevel() >= 1)  //  +7
            {
                Vector(Light[0] * 0.8f, Light[1] * 0.8f, Light[2] * 0.8f, b->BodyLight);
                RenderPartObjectBody(b, o, Type, Alpha, RenderType);
                RenderPartObjectBodyColor(b, o, Type, Alpha, RENDER_CHROME | RENDER_BRIGHT, 1.f);
            }
            else if (Level < 9 && g_pOption->GetRenderLevel() >= 1)  //  +8
            {
                Vector(Light[0] * 0.8f, Light[1] * 0.8f, Light[2] * 0.8f, b->BodyLight);
                RenderPartObjectBody(b, o, Type, Alpha, RenderType);
                RenderPartObjectBodyColor(b, o, Type, Alpha, RENDER_CHROME | RENDER_BRIGHT, 1.f);
            }
            else if (Level < 10 && g_pOption->GetRenderLevel() >= 2) //  +9
            {
                Vector(Light[0] * 0.9f, Light[1] * 0.9f, Light[2] * 0.9f, b->BodyLight);
                RenderPartObjectBody(b, o, Type, Alpha, RenderType);
                RenderPartObjectBodyColor(b, o, Type, Alpha, RENDER_CHROME | RENDER_BRIGHT | (RenderType & RENDER_EXTRA), 1.f);
                RenderPartObjectBodyColor(b, o, Type, Alpha, RENDER_METAL | RENDER_BRIGHT | (RenderType & RENDER_EXTRA), 1.f);
            }
            else if (Level < 11 && g_pOption->GetRenderLevel() >= 2) //  +10
            {
                Vector(Light[0] * 0.9f, Light[1] * 0.9f, Light[2] * 0.9f, b->BodyLight);
                RenderPartObjectBody(b, o, Type, Alpha, RenderType);
                RenderPartObjectBodyColor(b, o, Type, Alpha, RENDER_CHROME | RENDER_BRIGHT | (RenderType & RENDER_EXTRA), 1.f);
                RenderPartObjectBodyColor(b, o, Type, Alpha, RENDER_METAL | RENDER_BRIGHT | (RenderType & RENDER_EXTRA), 1.f);
            }
            else if (Level < 12 && g_pOption->GetRenderLevel() >= 3) //  +11
            {
                Vector(Light[0] * 0.9f, Light[1] * 0.9f, Light[2] * 0.9f, b->BodyLight);
                RenderPartObjectBody(b, o, Type, Alpha, RenderType);
                RenderPartObjectBodyColor2(b, o, Type, 1.f, RENDER_CHROME2 | RENDER_BRIGHT | (RenderType & RENDER_EXTRA), 1.f);
                RenderPartObjectBodyColor(b, o, Type, Alpha, RENDER_METAL | RENDER_BRIGHT | (RenderType & RENDER_EXTRA), 1.f);
                RenderPartObjectBodyColor(b, o, Type, Alpha, RENDER_CHROME | RENDER_BRIGHT | (RenderType & RENDER_EXTRA), 1.f);
            }
            else if (Level < 13 && g_pOption->GetRenderLevel() >= 3) //  +12
            {
                Vector(Light[0] * 0.9f, Light[1] * 0.9f, Light[2] * 0.9f, b->BodyLight);
                RenderPartObjectBody(b, o, Type, Alpha, RenderType);
                RenderPartObjectBodyColor2(b, o, Type, 1.f, RENDER_CHROME2 | RENDER_BRIGHT | (RenderType & RENDER_EXTRA), 1.f);
                RenderPartObjectBodyColor(b, o, Type, Alpha, RENDER_METAL | RENDER_BRIGHT | (RenderType & RENDER_EXTRA), 1.f);
                RenderPartObjectBodyColor(b, o, Type, Alpha, RENDER_CHROME | RENDER_BRIGHT | (RenderType & RENDER_EXTRA), 1.f);
            }
            else if (Level < 14 && g_pOption->GetRenderLevel() >= 4) //  +13
            {
                Vector(Light[0] * 0.9f, Light[1] * 0.9f, Light[2] * 0.9f, b->BodyLight);
                RenderPartObjectBody(b, o, Type, Alpha, RenderType);
                RenderPartObjectBodyColor2(b, o, Type, 1.f, RENDER_CHROME4 | RENDER_BRIGHT | (RenderType & RENDER_EXTRA), 1.f);
                RenderPartObjectBodyColor(b, o, Type, Alpha, RENDER_METAL | RENDER_BRIGHT | (RenderType & RENDER_EXTRA), 1.f);
                RenderPartObjectBodyColor(b, o, Type, Alpha, RENDER_CHROME | RENDER_BRIGHT | (RenderType & RENDER_EXTRA), 1.f);
            }
            else if (Level < 15 && g_pOption->GetRenderLevel() >= 4) //  +14
            {
                Vector(Light[0] * 0.9f, Light[1] * 0.9f, Light[2] * 0.9f, b->BodyLight);
                RenderPartObjectBody(b, o, Type, Alpha, RenderType);
                RenderPartObjectBodyColor2(b, o, Type, 1.f, RENDER_CHROME4 | RENDER_BRIGHT | (RenderType & RENDER_EXTRA), 1.f);
                RenderPartObjectBodyColor(b, o, Type, Alpha, RENDER_METAL | RENDER_BRIGHT | (RenderType & RENDER_EXTRA), 1.f);
                RenderPartObjectBodyColor(b, o, Type, Alpha, RENDER_CHROME | RENDER_BRIGHT | (RenderType & RENDER_EXTRA), 1.f);
            }
            else if (Level < 16 && g_pOption->GetRenderLevel() >= 4) //  +15
            {
                Vector(Light[0] * 0.9f, Light[1] * 0.9f, Light[2] * 0.9f, b->BodyLight);
                RenderPartObjectBody(b, o, Type, Alpha, RenderType);
                RenderPartObjectBodyColor2(b, o, Type, 1.f, RENDER_CHROME4 | RENDER_BRIGHT | (RenderType & RENDER_EXTRA), 1.f);
                RenderPartObjectBodyColor(b, o, Type, Alpha, RENDER_METAL | RENDER_BRIGHT | (RenderType & RENDER_EXTRA), 1.f);
                RenderPartObjectBodyColor(b, o, Type, Alpha, RENDER_CHROME | RENDER_BRIGHT | (RenderType & RENDER_EXTRA), 1.f);
            }
            else
            {
                VectorCopy(Light, b->BodyLight);
                RenderPartObjectBody(b, o, Type, Alpha, RenderType);
            }
        }
        else
        {
            VectorCopy(Light, b->BodyLight);
            RenderPartObjectBody(b, o, Type, Alpha, RenderType);
        }

        if (g_pOption->GetRenderLevel() == 0)
        {
            return;
        }

        if (!g_isCharacterBuff(o, eDeBuff_Harden) && !g_isCharacterBuff(o, eBuff_Cloaking)
            && !g_isCharacterBuff(o, eDeBuff_CursedTempleRestraint)
            )
        {
            if ((ExcellentFlags & 63) > 0 && (o->Type<MODEL_WING || o->Type>MODEL_WINGS_OF_DARKNESS) && o->Type != MODEL_CAPE_OF_LORD
                && (o->Type<MODEL_WING_OF_STORM || o->Type>MODEL_WING_OF_DIMENSION)
                && (o->Type < MODEL_WING + 130 || MODEL_WING + 134 < o->Type)
                && !(o->Type >= MODEL_CAPE_OF_FIGHTER && o->Type <= MODEL_CAPE_OF_OVERRULE)
                && (o->Type != MODEL_WING + 135))
            {
                Luminosity = sinf(WorldTime * 0.002f) * 0.5f + 0.5f;
                Vector(Luminosity, Luminosity * 0.3f, 1.f - Luminosity, b->BodyLight);
                Alpha = 1.f;
                if (b->HideSkin && MODEL_MISTERY_HELM <= o->Type && MODEL_LILIUM_HELM >= o->Type)
                {
                    int anMesh[6] = { 2, 1, 0, 2, 1, 2 };
                    b->RenderMesh(anMesh[o->Type - (MODEL_MISTERY_HELM)], RENDER_TEXTURE | RENDER_BRIGHT, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                }

                else if (Type == MODEL_SACRED_HELM || Type == MODEL_STORM_HARD_HELM)
                {
                    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                }
                else if (Type == MODEL_PIERCING_HELM)
                {
                    b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                }
                else if (Type == MODEL_SACRED_ARMOR)
                {
                    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                }
                else if (Type == MODEL_STORM_HARD_ARMOR)
                {
                    b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                }
                else if (Type == MODEL_PIERCING_ARMOR)
                {
                    b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                }
                else if (Type == MODEL_PHOENIX_SOUL_HELMET)
                {
                    b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                }
                else if (Type == MODEL_PHOENIX_SOUL_ARMOR)
                {
                    b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                }
                else if (Type == MODEL_ARMORINVEN_74)
                {
                    b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                }
                else if (Type == MODEL_PHOENIX_SOUL_BOOTS)
                {
                    b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                }
                else
                {
                    b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                }
            }
            else if (ancientDiscriminator > 0)
            {
                Alpha = sinf(WorldTime * 0.001f) * 0.5f + 0.4f;
                if (Alpha <= 0.01f)
                {
                    b->renderCount = rand() % 100;
                }
                RenderPartObjectBodyColor2(b, o, Type, Alpha, RENDER_CHROME3 | RENDER_BRIGHT, 1.f);
            }
        }
    }
#ifndef CAMERA_TEST
    else
    {
        if (gMapManager.WorldActive == 7)
        {
            EnableAlphaTest();
            glColor4f(0.f, 0.f, 0.f, 0.2f);
        }
        else
        {
            DisableAlphaBlend();
            glColor3f(0.f, 0.f, 0.f);
        }
        bool bRenderShadow = true;

        if (gMapManager.InHellas())
        {
            bRenderShadow = false;
        }

        if (WD_10HEAVEN == gMapManager.WorldActive || o->m_bySkillCount == 3 || g_Direction.m_CKanturu.IsMayaScene())
        {
            bRenderShadow = false;
        }

        if (g_isCharacterBuff(o, eBuff_Cloaking))
        {
            bRenderShadow = false;
        }

        if (bRenderShadow)
        {
            bRenderShadow = o->m_bRenderShadow;
            if (bRenderShadow)
            {
                b->RenderBodyShadow(o->BlendMesh, o->HiddenMesh);
            }
        }
#endif
    }
}

extern float BoneScale;

void RenderPartObjectEdge(BMD* b, OBJECT* o, int Flag, bool Translate, float Scale)
{
    if (g_isCharacterBuff(o, eBuff_Cloaking))
    {
        return;
    }

    b->LightEnable = false;

    BoneScale = Scale;
    if (o->EnableBoneMatrix == 1)
    {
        b->Transform(o->BoneTransform, o->BoundingBoxMin, o->BoundingBoxMax, &o->OBB, Translate);
    }
    else
    {
        b->Transform(BoneTransform, o->BoundingBoxMin, o->BoundingBoxMax, &o->OBB);
    }

    if (o->Type == MODEL_WARCRAFT)
    {
        b->BeginRender(o->Alpha);
        if (o->Alpha >= 0.99f)
        {
            glColor3fv(b->BodyLight);
        }
        else
        {
            glColor4f(b->BodyLight[0], b->BodyLight[1], b->BodyLight[2], o->Alpha);
        }
        b->RenderMesh(0, Flag, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->EndRender();
    }
    else if (o->Type == MODEL_PERSONA)
    {
        glColor3fv(b->BodyLight);
        b->RenderMesh(0, Flag, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(1, Flag, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
    }
    else if (o->Type == MODEL_DREADFEAR)
    {
        glColor3fv(b->BodyLight);
        b->RenderMesh(0, Flag, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(1, Flag, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
    }
    else if (o->Type == MODEL_DARK_SKULL_SOLDIER_5)
    {
        glColor3fv(b->BodyLight);
        b->RenderMesh(0, Flag, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(2, Flag, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
        b->RenderMesh(3, Flag, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
    }
    else
    {
        b->RenderBody(Flag, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
    }

    BoneScale = 1.f;
}

void RenderPartObjectEdge2(BMD* b, OBJECT* o, int Flag, bool Translate, float Scale, OBB_t* OBB)
{
    vec3_t tmp;

    b->LightEnable = false;

    BoneScale = Scale;
    b->Transform(BoneTransform, tmp, tmp, OBB, Translate);
    b->RenderBody(Flag, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);

    BoneScale = 1.f;
}

void RenderPartObjectEdgeLight(BMD* b, OBJECT* o, int Flag, bool Translate, float Scale)
{
    float Luminosity = sinf(WorldTime * 0.001f) * 0.5f + 0.5f;
    Vector(Luminosity * 1.f, Luminosity * 0.8f, Luminosity * 0.3f, b->BodyLight);
    RenderPartObjectEdge(b, o, Flag, Translate, Scale);
}

void RenderPartObject(OBJECT* o, int Type, void* p2, vec3_t Light, float Alpha, int ItemLevel, int ExcellentFlags, int ancientDiscriminator, bool GlobalTransform, bool HideSkin, bool Translate, int Select, int RenderType)
{
    if (Alpha <= 0.01f)
    {
        return;
    }

    auto* p = (PART_t*)p2;

    if (Type == MODEL_POTION + 12)
    {
        int Level = ItemLevel;

        if (Level == 0)
        {
            Type = MODEL_EVENT;
        }
        else if (Level == 2)
        {
            Type = MODEL_EVENT + 1;
        }
    }

    BMD* b = &Models[Type];
    b->HideSkin = HideSkin;
    b->BodyScale = o->Scale;
    b->ContrastEnable = o->ContrastEnable;
    b->LightEnable = o->LightEnable;
    VectorCopy(o->Position, b->BodyOrigin);

    BoneScale = 1.f;
    if (3 == Select)
    {
        BoneScale = 1.4f;
    }
    else if (2 == Select)
    {
        BoneScale = 1.2f;
    }
    else if (1 == Select)
    {
        float Scale = 1.2f;
        Scale = o->m_fEdgeScale;
        if (o->Kind == KIND_NPC)
        {
            Vector(0.02f, 0.1f, 0.f, b->BodyLight);
        }
        else
        {
            Vector(0.1f, 0.03f, 0.f, b->BodyLight);
        }

        if (gMapManager.InChaosCastle())
        {
            Vector(0.1f, 0.01f, 0.f, b->BodyLight);
            Scale = 1.f + 0.1f / o->Scale;
        }

        RenderPartObjectEdge(b, o, RENDER_BRIGHT, Translate, Scale);
        if (o->Kind == KIND_NPC)
        {
            Vector(0.16f, 0.7f, 0.f, b->BodyLight);
        }
        else
        {
            Vector(0.7f, 0.2f, 0.f, b->BodyLight);
        }

        if (gMapManager.InChaosCastle())
        {
            Vector(0.7f, 0.07f, 0.f, b->BodyLight);
            Scale = 1.f + 0.04f / o->Scale + 0.02f;
        }
        RenderPartObjectEdge(b, o, RENDER_BRIGHT, Translate, Scale - 0.02f);
    }
    BodyLight(o, b);

    if (GlobalTransform)
    {
        b->Transform(BoneTransform, o->BoundingBoxMin, o->BoundingBoxMax, &o->OBB, Translate);
    }
    else
    {
        b->Transform(o->BoneTransform, o->BoundingBoxMin, o->BoundingBoxMax, &o->OBB, Translate);
    }

    if (p)
    {
        int iCloth = 0;
        switch (Type)
        {
        case MODEL_GRAND_SOUL_PANTS:
            iCloth = 1;
            break;

        case MODEL_DIVINE_PANTS:
            iCloth = 2;
            break;

        case MODEL_DARK_SOUL_PANTS:
            iCloth = 3;
            break;
        }

        if (!p->m_pCloth[0] && iCloth > 0)
        {
            int iCount = 1;
            CPhysicsClothMesh* pCloth[2] = { NULL, NULL };
            switch (iCloth)
            {
            case 1:
                pCloth[0] = new CPhysicsClothMesh;
                pCloth[0]->Create(o, 2, 17, 0.0f, 9.0f, 7.0f, 5, 8, 45.0f, 85.0f, BITMAP_PANTS_G_SOUL, BITMAP_PANTS_G_SOUL, PCT_MASK_ALPHA | PCT_HEAVY | PCT_STICKED | PCT_SHORT_SHOULDER, Type);
                pCloth[0]->AddCollisionSphere(0.0f, -15.0f, -20.0f, 30.0f, 2);
                break;
            case 2:
                iCount = 1;
                pCloth[0] = new CPhysicsClothMesh;
                pCloth[0]->Create(o, 3, 2, PCT_OPT_CORRECTEDFORCE | PCT_HEAVY, Type);
                pCloth[0]->AddCollisionSphere(0.0f, 0.0f, -15.0f, 22.0f, 2);
                pCloth[0]->AddCollisionSphere(0.0f, 0.0f, -27.0f, 23.0f, 2);
                pCloth[0]->AddCollisionSphere(0.0f, 0.0f, -40.0f, 24.0f, 2);
                pCloth[0]->AddCollisionSphere(0.0f, 0.0f, -54.0f, 25.0f, 2);
                pCloth[0]->AddCollisionSphere(0.0f, 0.0f, -69.0f, 26.0f, 2);
                break;
            case 3:
                pCloth[0] = new CPhysicsClothMesh;
                pCloth[0]->Create(o, 2, 17, 0.0f, 9.0f, 7.0f, 7, 5, 50.0f, 100.0f, b->IndexTexture[b->Meshs[2].Texture], b->IndexTexture[b->Meshs[2].Texture], PCT_MASK_ALPHA | PCT_HEAVY | PCT_STICKED | PCT_SHORT_SHOULDER, Type);
                pCloth[0]->AddCollisionSphere(0.0f, -15.0f, -20.0f, 30.0f, 2);
                break;
            }
            p->m_byNumCloth = iCount;
            p->m_pCloth[0] = (void*)pCloth[0];
            p->m_pCloth[1] = (void*)pCloth[1];
        }

        if (p->m_pCloth[0])
        {
            if (iCloth > 0)
            {
                for (int i = 0; i < p->m_byNumCloth; ++i)
                {
                    auto* pCloth = (CPhysicsCloth*)p->m_pCloth[i];
                    if (!pCloth->Move2(0.005f, 5))
                    {
                        DeleteCloth(NULL, o, p);
                    }
                    else
                    {
                        pCloth->Render();
                    }
                }
            }
            else if (iCloth == 0)
            {
                DeleteCloth(NULL, o, p);
            }
        }
    }

    if (!g_CMonkSystem.RageFighterEffect(o, Type))
        RenderPartObjectEffect(o, Type, Light, Alpha, ItemLevel, ExcellentFlags, ancientDiscriminator, Select, RenderType);
}

float AmbientShadowAngle = 180.f;

void CopyShadowAngle(OBJECT* o, BMD* b)
{
    /*VectorCopy(o->ShadowAngle,b->ShadowAngle);
    if(o->ShadowAlpha >= 0.5f)
    {
    }
    else
    {
        b->ShadowAngle[2] = AmbientShadowAngle;
    }*/
}

void InitShadowAngle()
{
    /*for(int i=0;i<16;i++)
    {
        for(int j=0;j<16;j++)
        {
            OBJECT_BLOCK *ob = &ObjectBlock[i*16+j];
            OBJECT       *o  = ob->Head;
            while(1)
            {
                if(o != NULL)
                {
                    if(o->Live && o->Visible)
                    {
                        o->Distance = 9999.f*9999.f;
                        o->ShadowAlpha = 0.f;
                    }
                    if(o->Next == NULL) break;
                    o = o->Next;
                }
                else break;
            }
        }
    }*/
}

void ShadowAngle(OBJECT* Owner)
{
    /*for(int i=0;i<16;i++)
    {
        for(int j=0;j<16;j++)
        {
            OBJECT_BLOCK *ob = &ObjectBlock[i*16+j];
            OBJECT       *o  = ob->Head;
            while(1)
            {
                if(o != NULL)
                {
                    if(o->Live && o->Visible)
                    {
                        vec3_t Range;
                        VectorSubtract(Owner->Position,o->Position,Range);
                        float Distance = Range[0]*Range[0]+Range[1]*Range[1];
                        if(Distance<=400.f*400.f && o->Distance>Distance)
                        {
                            o->Distance = Distance;
                            o->ShadowAlpha = (400.f*400.f-Distance)/(400.f*400.f);
                            o->ShadowAngle[2] = 360.f-CreateAngle2D(o->Position, Owner->Position);
                        }
                    }
                    if(o->Next == NULL) break;
                    o = o->Next;
                }
                else break;
            }
        }
    }*/
}

void CreateShadowAngle()
{
    for (int i = 0; i < 16; i++)
    {
        for (int j = 0; j < 16; j++)
        {
            OBJECT_BLOCK* ob = &ObjectBlock[i * 16 + j];
            OBJECT* o = ob->Head;
            while (1)
            {
                if (o != NULL)
                {
                    if (o->Live)
                    {
                        o->Visible = true;
                        ShadowAngle(o);
                    }
                    if (o->Next == NULL) break;
                    o = o->Next;
                }
                else break;
            }
        }
    }
}

void RenderObjectShadow()
{
}

bool IsPartyMemberBuff(int partyindex, eBuffState buffstate)
{
    return false;
}

bool isPartyMemberBuff(int partyindex)
{
    return false;
}

#ifdef CSK_DEBUG_RENDER_BOUNDINGBOX
void RenderBoundingBox(OBJECT* pObj)
{
    EnableAlphaBlend();
    glPushMatrix();

    float Matrix[3][4];
    AngleMatrix(pObj->Angle, Matrix);
    Matrix[0][3] = pObj->Position[0];
    Matrix[1][3] = pObj->Position[1];
    Matrix[2][3] = pObj->Position[2];

    vec3_t BoundingVertices[8];
    Vector(pObj->BoundingBoxMax[0], pObj->BoundingBoxMax[1], pObj->BoundingBoxMax[2], BoundingVertices[0]);
    Vector(pObj->BoundingBoxMax[0], pObj->BoundingBoxMax[1], pObj->BoundingBoxMin[2], BoundingVertices[1]);
    Vector(pObj->BoundingBoxMax[0], pObj->BoundingBoxMin[1], pObj->BoundingBoxMax[2], BoundingVertices[2]);
    Vector(pObj->BoundingBoxMax[0], pObj->BoundingBoxMin[1], pObj->BoundingBoxMin[2], BoundingVertices[3]);
    Vector(pObj->BoundingBoxMin[0], pObj->BoundingBoxMax[1], pObj->BoundingBoxMax[2], BoundingVertices[4]);
    Vector(pObj->BoundingBoxMin[0], pObj->BoundingBoxMax[1], pObj->BoundingBoxMin[2], BoundingVertices[5]);
    Vector(pObj->BoundingBoxMin[0], pObj->BoundingBoxMin[1], pObj->BoundingBoxMax[2], BoundingVertices[6]);
    Vector(pObj->BoundingBoxMin[0], pObj->BoundingBoxMin[1], pObj->BoundingBoxMin[2], BoundingVertices[7]);

    vec3_t TransformVertices[8];
    for (int j = 0; j < 8; j++)
    {
        VectorTransform(BoundingVertices[j], Matrix, TransformVertices[j]);
    }

    //glBegin(GL_QUADS);
    glBegin(GL_LINES);
    glColor3f(0.2f, 0.2f, 0.2f);
    glTexCoord2f(1.0F, 1.0F); glVertex3fv(TransformVertices[7]);
    glTexCoord2f(1.0F, 0.0F); glVertex3fv(TransformVertices[6]);
    glTexCoord2f(0.0F, 0.0F); glVertex3fv(TransformVertices[4]);
    glTexCoord2f(0.0F, 1.0F); glVertex3fv(TransformVertices[5]);

    glColor3f(0.2f, 0.2f, 0.2f);
    glTexCoord2f(0.0F, 1.0F); glVertex3fv(TransformVertices[0]);
    glTexCoord2f(1.0F, 1.0F); glVertex3fv(TransformVertices[2]);
    glTexCoord2f(1.0F, 0.0F); glVertex3fv(TransformVertices[3]);
    glTexCoord2f(0.0F, 0.0F); glVertex3fv(TransformVertices[1]);

    glColor3f(0.6f, 0.6f, 0.6f);
    glTexCoord2f(1.0F, 1.0F); glVertex3fv(TransformVertices[7]);
    glTexCoord2f(1.0F, 0.0F); glVertex3fv(TransformVertices[3]);
    glTexCoord2f(0.0F, 0.0F); glVertex3fv(TransformVertices[2]);
    glTexCoord2f(0.0F, 1.0F); glVertex3fv(TransformVertices[6]);

    glColor3f(0.6f, 0.6f, 0.6f);
    glTexCoord2f(0.0F, 1.0F); glVertex3fv(TransformVertices[0]);
    glTexCoord2f(1.0F, 1.0F); glVertex3fv(TransformVertices[1]);
    glTexCoord2f(1.0F, 0.0F); glVertex3fv(TransformVertices[5]);
    glTexCoord2f(0.0F, 0.0F); glVertex3fv(TransformVertices[4]);

    glColor3f(0.4f, 0.4f, 0.4f);
    glTexCoord2f(1.0F, 1.0F); glVertex3fv(TransformVertices[7]);
    glTexCoord2f(1.0F, 0.0F); glVertex3fv(TransformVertices[5]);
    glTexCoord2f(0.0F, 0.0F); glVertex3fv(TransformVertices[1]);
    glTexCoord2f(0.0F, 1.0F); glVertex3fv(TransformVertices[3]);

    glColor3f(0.4f, 0.4f, 0.4f);
    glTexCoord2f(0.0F, 1.0F); glVertex3fv(TransformVertices[0]);
    glTexCoord2f(1.0F, 1.0F); glVertex3fv(TransformVertices[4]);
    glTexCoord2f(1.0F, 0.0F); glVertex3fv(TransformVertices[6]);
    glTexCoord2f(0.0F, 0.0F); glVertex3fv(TransformVertices[2]);
    glEnd();

    glPopMatrix();
}
#endif // CSK_DEBUG_RENDER_BOUNDINGBOX