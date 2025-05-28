//////////////////////////////////////////////////////////////////////////
//  GMBattleCastle.cpp
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UIWindows.h"
#include "UIManager.h"
#include "dsplaysound.h"
#include "ZzzOpenglUtil.h"
#include "zzztexture.h"
#include "ZzzBMD.h"
#include "zzzlodterrain.h"
#include "ZzzScene.h"
#include "zzzEffect.h"
#include "zzzAi.h"
#include "zzzOpenData.h"
#include "PhysicsManager.h"
#include "CSParts.h"

#include "MapManager.h"
#include "GIPetManager.h"
#include "BoneManager.h"

#include "GuildCache.h"
#include "ZzzInterface.h"


extern  int     WaterTextureNumber;
extern  char* NUM_MUSIC;

namespace battleCastle
{
    typedef struct
    {
        vec3_t  m_vPosition;
        BYTE    m_byBuildTime;
    } BuildTime;

    static  BYTE    g_byGuardAI = 0;
    static  bool    g_bBeGate = false;
    static  BYTE    g_byGateLocation[6][2] = { { 67, 114 }, {  93, 114 }, { 119, 114 }, {  81, 161 }, { 107, 161 }, { 93, 204 } };
    static  float   g_fGuardStoneLocation[4][2] = { { 8200, 13000 }, { 10700, 13000 }, {  9400, 18200 }, {  9400, 22700 } };
    static  float   g_fLifeStoneLocation[2] = { 0.f, 0.f };
    static  bool    g_isCrownState = false;
    static  DWORD   g_dwCrownBackState = 0x00000000;
    static  bool    g_bBattleCastleStart = false;
    static  bool    g_bBattleCastleStartBackup = false;
    static  float   g_iMp3PlayTime = 0.f;

    static float LastHealingParticle = 0.0f;
    static constexpr float HealingParticleInterval = 400; // every 400 ms

    static float LastStoneFlyEffect = 0.0f;
    static constexpr float StoneFlyEffectInterval = 800; // every 800 ms

    static float LastArrowEffectOnBattlefield = 0.0f;
    static constexpr float ArrowEffectOnBattlefieldInterval = 2400; // every 2400 ms

    static float LastArrowEffectInStampRoom = 0.0f;
    static constexpr float ArrowEffectInStampRoomInterval = 400; // every 400 ms

    std::queue<BuildTime>   g_qBuildTimeLocation;

    enum
    {
        GUARD_STOP = 0,
        GUARD_READY,
        GUARD_ATTACK_READY,
        GUARD_ATTACK
    };

    bool IsBattleCastleStart(void)
    {
        return g_bBattleCastleStart;
    }
    void SetBattleCastleStart(bool bResult)
    {
        g_bBattleCastleStartBackup = g_bBattleCastleStart;
        g_bBattleCastleStart = bResult;

        if ( /*g_bBattleCastleStart!=g_bBattleCastleStartBackup && */ gMapManager.WorldActive != -1 && gMapManager.InBattleCastle())
        {
            wchar_t FileName[64];
            wchar_t WorldName[32];

            swprintf(WorldName, L"World%d", gMapManager.WorldActive + 1);
            if (g_bBattleCastleStart)
            {
                swprintf(FileName, L"Data\\%s\\EncTerrain%d.att", WorldName, (gMapManager.WorldActive + 1) * 10 + 2);
                OpenTerrainAttribute(FileName);

                swprintf(FileName, L"%s\\TerrainLight2.jpg", WorldName);
                OpenTerrainLight(FileName);
            }
            else
            {
                swprintf(FileName, L"Data\\%s\\EncTerrain%d.att", WorldName, gMapManager.WorldActive + 1);
                OpenTerrainAttribute(FileName);

                swprintf(FileName, L"%s\\TerrainLight.jpg", WorldName);
            }

            g_iMp3PlayTime = 0;
        }
    }

    bool InArea(float x, float y, vec3_t Position, float Range)
    {
        float dx = x - Position[0];
        float dy = y - Position[1];
        float Distance = sqrtf(dx * dx + dy * dy);
        if (Distance <= Range)
            return true;
        return false;
    }

    void CollisionHeroCharacter(vec3_t Position, float Range, int AniType)
    {
        OBJECT* o = &Hero->Object;

        if (InArea(Position[0], Position[1], o->Position, Range) && o->CurrentAction != AniType)
        {
            //            if ( Range<=350.f && rand()%(int)(100-(Range-350.f)/10) )
            if (Hero->Helper.Type == MODEL_HORN_OF_FENRIR && AniType == PLAYER_HIGH_SHOCK)
            {
                SetAction_Fenrir_Damage(Hero, o);
                SendRequestAction(Hero->Object, AniType);
            }
            else
            {
                SetAction(o, AniType);
                SendRequestAction(Hero->Object, AniType);
            }
        }
    }

    void    CollisionTempCharacter(vec3_t Position, float Range, int AniType)
    {
        for (int i = 0; i < MAX_CHARACTERS_CLIENT; ++i)
        {
            OBJECT* o = &CharactersClient[i].Object;
            if (o->Live && (o->Kind == KIND_TMP) && o->Type == MODEL_PLAYER && o->CurrentAction != AniType)
            {
                if (InArea(Position[0], Position[1], o->Position, Range))
                {
                    //                    if ( Range<=350.f && rand()%(int)(100-(Range-350.f)/10) )
                    {
                        SetAction(o, AniType);
                    }
                }
            }
        }
    }

    bool CollisionEffectToObject(OBJECT* eff, float Range, float RangeZ, bool bCollisionGround, bool bRealCollision)
    {
        int i = (int)(eff->Position[0] / (16 * TERRAIN_SCALE));
        int j = (int)(eff->Position[1] / (16 * TERRAIN_SCALE));
        if (i < 0 || j < 0 || i >= 16 || j >= 16) return false;

        BYTE Block = i * 16 + j;
        OBJECT_BLOCK* ob = &ObjectBlock[Block];
        OBJECT* o = ob->Head;
        while (1)
        {
            if (o != NULL)
            {
                if (o->Live && o->HiddenMesh == -1 && o->m_bCollisionCheck)
                {
                    float dx = eff->Position[0] - o->Position[0];
                    float dy = eff->Position[1] - o->Position[1];
                    float Distance = sqrtf(dx * dx + dy * dy);
                    if (Distance <= Range)
                    {
                        if (bCollisionGround)
                        {
                            if (o->m_bCollisionCheck && o->ExtState == 0)
                            {
                                o->Timer += 0.3f * FPS_ANIMATION_FACTOR;
                                if (o->Timer >= 5)
                                {
                                    o->ExtState = 99;
                                }
                            }
                            return true;
                        }
                        else
                            if (fabs(eff->Position[2] - o->Position[2]) < RangeZ)
                            {
                                if (o->m_bCollisionCheck)
                                {
                                    if (o->ExtState == 0)
                                    {
                                        o->Timer += FPS_ANIMATION_FACTOR;
                                        if (o->Timer >= 5)
                                        {
                                            o->ExtState = 99;
                                        }
                                    }
                                }
                                eff->LifeTime = 15;
                                eff->SubType = 99;
                                eff->HiddenMesh = 99;
                                return true;
                            }
                    }
                }
                if (o->Next == NULL) break;
                o = o->Next;
            }
            else break;
        }
        return false;
    }

    bool CalcDistanceChrToChr(OBJECT* o, BYTE Type, float fRange)
    {
        for (int i = 0; i < MAX_CHARACTERS_CLIENT; i++)
        {
            CHARACTER* tc = &CharactersClient[i];
            OBJECT* to = &tc->Object;
            if (to->Kind == KIND_PLAYER && to->Type == MODEL_PLAYER && to->Visible == true)
            {
                if (Type == 0)
                {
                    float dx = o->Position[0] - to->Position[0];
                    float dy = o->Position[1] - to->Position[1];
                    if (dx < fRange && dy < 100) return true;
                }
                else if (Type == 1)
                {
                    float dx = fabs(o->Position[0] - to->Position[0]);
                    float dy = o->Position[1] - to->Position[1];
                    if (dx < 1 && dy>0 && dy < fRange)
                    {
                        return true;
                    }
                }
                else if (Type == 2)
                {
                    float dx = o->Position[0] - to->Position[0];
                    float dy = o->Position[1] - to->Position[1];
                    float Distance = sqrtf(dx * dx + dy * dy);
                    if (Distance < fRange) return true;
                }
            }
        }
        return false;
    }

    void SetCastleGate_Attribute(int x, int y, BYTE Operator, bool bAllClear)
    {
        for (int i = 0; i < 6; ++i)
        {
            if (bAllClear == true)
            {
                DWORD wall = TerrainWall[(g_byGateLocation[i][1] + 1) * TERRAIN_SIZE + (g_byGateLocation[i][0])];
                if ((wall & TW_NOMOVE) == TW_NOMOVE)
                {
                    AddTerrainAttributeRange(g_byGateLocation[i][0] - 1, g_byGateLocation[i][1] + 1, 4, 1, TW_NOMOVE, Operator);
                }
            }
            else
            {
                if (x == g_byGateLocation[i][0] && y == g_byGateLocation[i][1])
                {
                    AddTerrainAttributeRange(g_byGateLocation[i][0] - 1, g_byGateLocation[i][1] + 1, 4, 1, TW_NOMOVE, Operator);
                    return;
                }
            }
        }
    }

    void RenderAurora(int Type, int RenderType, float x, float y, float sx, float sy, vec3_t Light)
    {
        float   Luminosity = sinf(WorldTime * 0.0015f) * 0.3f + 0.7f;

        if (RenderType == RENDER_DARK)
        {
            EnableAlphaBlendMinus();
            Vector(Luminosity, Luminosity, Luminosity, Light);
        }
        else
        {
            EnableAlphaBlend();
            Vector(Luminosity * Light[0], Luminosity * Light[1], Luminosity * Light[2], Light);
        }

        RenderTerrainAlphaBitmap(Type, x, y, sx, sy, Light, WorldTime * 0.01f);

        if (RenderType == RENDER_DARK)
        {
            Luminosity = 1.f - Luminosity;
            Vector(Luminosity * Light[0], Luminosity * Light[1], Luminosity * Light[2], Light);
        }
        RenderTerrainAlphaBitmap(Type, x, y, sx, sy, Light, -WorldTime * 0.01f);
    }

    void SetBuildTimeLocation(OBJECT* o)
    {
        if (o->Type == MODEL_LIFE_STONE && o->m_byBuildTime < 5)
        {
            BuildTime bt;

            VectorCopy(o->Position, bt.m_vPosition);
            bt.m_vPosition[2] += o->BoundingBoxMax[2] + 100.f;
            bt.m_byBuildTime = o->m_byBuildTime;
            g_qBuildTimeLocation.push(bt);
        }
    }

    void RenderBuildTimes(void)
    {
        BuildTime bt;
        for (int i = 0; i < (int)g_qBuildTimeLocation.size(); ++i)
        {
            bt = g_qBuildTimeLocation.front();

            int ScreenX, ScreenY;
            int Width = 50, Height = 2;

            Projection(bt.m_vPosition, &ScreenX, &ScreenY);
            ScreenX -= (int)(Width / 2);

            int Time = bt.m_byBuildTime / 5.f * (int)Width;
            RenderBar(ScreenX, ScreenY + 12, Width, Height, (float)Time);

            g_qBuildTimeLocation.pop();
        }
    }

    void Init(void)
    {
        if (gMapManager.InBattleCastle() == false) return;

        g_byGuardAI = 0;
        g_iMp3PlayTime = 0;
        //        SetBattleCastleStart ( false );

        vec3_t Angle, Position;
        Vector(0.f, 0.f, 0.f, Angle);
        Vector(0.f, 0.f, 0.f, Position);

        SocketClient->ToGameServer()->SendGuildLogoOfCastleOwnerRequest();

        OpenMonsterModel(MONSTER_MODEL_BATTLE_GUARD2);

        constexpr float zOffset = 80.f;

        Position[0] = 65 * TERRAIN_SCALE;
        Position[1] = 113 * TERRAIN_SCALE;
        Position[2] = RequestTerrainHeight(Position[0], Position[1]) + zOffset;
        CreateObject(MODEL_BATTLE_GUARD2, Position, Angle);

        Position[0] = 71 * TERRAIN_SCALE;
        Position[1] = 113 * TERRAIN_SCALE;
        Position[2] = RequestTerrainHeight(Position[0], Position[1]) + zOffset;
        CreateObject(MODEL_BATTLE_GUARD2, Position, Angle);

        Position[0] = 91 * TERRAIN_SCALE;
        Position[1] = 113 * TERRAIN_SCALE;
        Position[2] = RequestTerrainHeight(Position[0], Position[1]) + zOffset;
        CreateObject(MODEL_BATTLE_GUARD2, Position, Angle);

        Position[0] = 97 * TERRAIN_SCALE;
        Position[1] = 113 * TERRAIN_SCALE;
        Position[2] = RequestTerrainHeight(Position[0], Position[1]) + zOffset;
        CreateObject(MODEL_BATTLE_GUARD2, Position, Angle);

        Position[0] = 117 * TERRAIN_SCALE;
        Position[1] = 113 * TERRAIN_SCALE;
        Position[2] = RequestTerrainHeight(Position[0], Position[1]) + zOffset;
        CreateObject(MODEL_BATTLE_GUARD2, Position, Angle);

        Position[0] = 123 * TERRAIN_SCALE;
        Position[1] = 113 * TERRAIN_SCALE;
        Position[2] = RequestTerrainHeight(Position[0], Position[1]) + zOffset;
        CreateObject(MODEL_BATTLE_GUARD2, Position, Angle);
    }

    bool    SettingBattleFormation(CHARACTER* c, eBuffState state)
    {
        if (gMapManager.InBattleCastle() == false)  return false;

        if (state == eBuff_CastleRegimentAttack1)
        {
            if (c->EtcPart == PARTS_DEFENSE_TEAM_MARK)
            {
                DeleteParts(c);
            }
            c->EtcPart = PARTS_ATTACK_TEAM_MARK;
        }

        else if (state == eBuff_CastleRegimentAttack2)
        {
            if (c->EtcPart == PARTS_DEFENSE_TEAM_MARK)
            {
                DeleteParts(c);
            }
            c->EtcPart = PARTS_ATTACK_TEAM_MARK2;
        }
        else if (state == eBuff_CastleRegimentAttack3)
        {
            if (c->EtcPart == PARTS_DEFENSE_TEAM_MARK)
            {
                DeleteParts(c);
            }
            c->EtcPart = PARTS_ATTACK_TEAM_MARK3;
        }
        else if (state == eBuff_CastleRegimentDefense)
        {
            if (c->EtcPart == PARTS_ATTACK_TEAM_MARK
                || c->EtcPart == PARTS_ATTACK_TEAM_MARK2
                || c->EtcPart == PARTS_ATTACK_TEAM_MARK3
                )
            {
                DeleteParts(c);
            }
            c->EtcPart = PARTS_DEFENSE_TEAM_MARK;
        }
        else if (c->EtcPart == PARTS_ATTACK_TEAM_MARK || c->EtcPart == PARTS_DEFENSE_TEAM_MARK
            || c->EtcPart == PARTS_ATTACK_TEAM_MARK2
            || c->EtcPart == PARTS_ATTACK_TEAM_MARK3
            )
        {
            DeleteParts(c);
            return false;
        }
        return true;
    }

    bool    GetGuildMaster(CHARACTER* c)
    {
        if (wcscmp(GuildMark[c->GuildMarkIndex].GuildName, L"") == NULL) return false;
        if (wcscmp(GuildMark[c->GuildMarkIndex].UnionName, L"") == NULL && c->GuildStatus != G_MASTER) return false;
        if (wcscmp(GuildMark[c->GuildMarkIndex].UnionName, GuildMark[c->GuildMarkIndex].GuildName) == NULL && c->GuildStatus != G_MASTER) return false;

        return true;
    }

    void SettingBattleKing(CHARACTER* c)
    {
        OBJECT* o = &c->Object;

        if (GetGuildMaster(c) == false) return;

        if (g_isCharacterBuff(o, eBuff_CastleRegimentAttack1))
        {
            DeleteParts(c);
            c->EtcPart = PARTS_ATTACK_KING_TEAM_MARK;
        }
        else if (g_isCharacterBuff(o, eBuff_CastleRegimentAttack2))
        {
            DeleteParts(c);
            c->EtcPart = PARTS_ATTACK_KING_TEAM_MARK2;
        }
        else if (g_isCharacterBuff(o, eBuff_CastleRegimentAttack3))
        {
            DeleteParts(c);
            c->EtcPart = PARTS_ATTACK_KING_TEAM_MARK3;
        }
        else if (g_isCharacterBuff(o, eBuff_CastleRegimentDefense))
        {
            DeleteParts(c);
            c->EtcPart = PARTS_DEFENSE_KING_TEAM_MARK;
        }
    }

    void    DeleteBattleFormation(CHARACTER* c, eBuffState state)
    {
        if (gMapManager.InBattleCastle() == false)  return;

        if (eBuff_CastleRegimentAttack1 != state || eBuff_CastleRegimentAttack2 != state
            || eBuff_CastleRegimentAttack3 != state || eBuff_CastleRegimentDefense != state)
        {
            return;
        }

        OBJECT* o = &c->Object;

        if (g_isCharacterBuff(o, eBuff_CastleRegimentAttack1) || g_isCharacterBuff(o, eBuff_CastleRegimentAttack2)
            || g_isCharacterBuff(o, eBuff_CastleRegimentAttack3) || g_isCharacterBuff(o, eBuff_CastleRegimentDefense))
        {
            if (c->EtcPart == PARTS_DEFENSE_TEAM_MARK || c->EtcPart == PARTS_ATTACK_TEAM_MARK
                || c->EtcPart == PARTS_ATTACK_TEAM_MARK2 || c->EtcPart == PARTS_ATTACK_TEAM_MARK3)
            {
                DeleteParts(c);
            }
        }
    }

    void ChangeBattleFormation(wchar_t* GuildName, bool bEffect)
    {
        for (int i = 0; i < MAX_CHARACTERS_CLIENT; ++i)
        {
            CHARACTER* c = &CharactersClient[i];
            OBJECT* o = &c->Object;
            if (o->Live && o->Visible)
            {
                DeleteParts(c);
                if (wcscmp(GuildMark[c->GuildMarkIndex].UnionName, GuildName) == NULL)
                {
                    // _buffwani_
                    g_TokenCharacterBuff(o, eBuff_CastleRegimentDefense);
                    c->EtcPart = PARTS_DEFENSE_TEAM_MARK;
                }
                // _buffwani_
                else if (g_isCharacterBuff(o, eBuff_CastleRegimentAttack1)
                    || g_isCharacterBuff(o, eBuff_CastleRegimentDefense))
                {
                    if (g_isCharacterBuff(o, eBuff_CastleRegimentDefense))
                        g_CharacterUnRegisterBuff(o, eBuff_CastleRegimentDefense);

                    g_CharacterRegisterBuff(o, eBuff_CastleRegimentAttack1);
                    c->EtcPart = PARTS_ATTACK_TEAM_MARK;
                }
                else if (g_isCharacterBuff(o, eBuff_CastleRegimentAttack2)
                    || g_isCharacterBuff(o, eBuff_CastleRegimentDefense))
                {
                    if (g_isCharacterBuff(o, eBuff_CastleRegimentDefense))
                        g_CharacterUnRegisterBuff(o, eBuff_CastleRegimentDefense);

                    g_CharacterRegisterBuff(o, eBuff_CastleRegimentAttack2);
                    c->EtcPart = PARTS_ATTACK_TEAM_MARK2;
                }
                else if (g_isCharacterBuff(o, eBuff_CastleRegimentAttack3)
                    || g_isCharacterBuff(o, eBuff_CastleRegimentDefense))
                {
                    if (g_isCharacterBuff(o, eBuff_CastleRegimentDefense))
                        g_CharacterUnRegisterBuff(o, eBuff_CastleRegimentDefense);

                    g_CharacterRegisterBuff(o, eBuff_CastleRegimentAttack3);
                    c->EtcPart = PARTS_ATTACK_TEAM_MARK3;
                }
            }
        }
    }

    void    CreateBattleCastleCharacter_Visual(CHARACTER* c, OBJECT* o)
    {
        if (gMapManager.InBattleCastle() == false)  return;
        if (IsBattleCastleStart() == false) return;

        if (o->Visible == false) return;

        CreateGuardStoneHealingVisual(c, 380.f);
    }

    void    DeleteTmpCharacter(void)
    {
        for (int i = 0; i < MAX_CHARACTERS_CLIENT; ++i)
        {
            CHARACTER* c = &CharactersClient[i];
            OBJECT* o = &c->Object;
            if (o->Live && o->Kind == KIND_TMP)
            {
                o->Live = false;
                BoneManager::UnregisterBone(c);

                for (int j = 0; j < MAX_MOUNTS; j++)
                {
                    OBJECT* b = &Mounts[j];
                    if (b->Live && b->Owner == o)
                        b->Live = false;
                }
                DeletePet(c);
                DeleteCloth(c, o);
                DeleteParts(c);
            }
        }
    }

    void    StartFog(vec3_t Color)
    {
        glEnable(GL_FOG);

        glFogfv(GL_FOG_COLOR, Color);
        glFogf(GL_FOG_MODE, GL_LINEAR);
        glFogf(GL_FOG_START, 2000.f);
        glFogf(GL_FOG_END, 2700.f);
    }

    void    EndFog(void)
    {
        glDisable(GL_FOG);
    }

    void    RenderBaseSmoke(void)
    {
        if (gMapManager.InBattleCastle() == false) return;
        if (IsBattleCastleStart() == false) return;

        EnableAlphaTest();

        glColor3f(0.3f, 0.3f, 0.25f);
        float WindX2 = (float)((int)WorldTime % 100000) * 0.0005f;
        RenderBitmapUV(BITMAP_CHROME + 3, 0.f, 0.f, 640.f, 480.f - 45.f, WindX2, 0.f, 3.f, 2.f);
        EnableAlphaBlend();
        float WindX = (float)((int)WorldTime % 100000) * 0.0002f;
        RenderBitmapUV(BITMAP_CHROME + 2, 0.f, 0.f, 640.f, 480.f - 45.f, WindX, 0.f, 0.3f, 0.3f);
    }

    bool CreateFireSnuff(PARTICLE* o)
    {
        if (gMapManager.WorldActive != WD_55LOGINSCENE)
        {
            if (gMapManager.InBattleCastle() == false) return false;
            if (IsBattleCastleStart() == false) return false;
            if (InBattleCastle3(Hero->Object.Position) == false) return false;
        }

        o->Type = BITMAP_FIRE_SNUFF;
        o->Scale = rand() % 50 / 100.f + 0.5f;
        vec3_t Position;

        if (gMapManager.WorldActive == WD_55LOGINSCENE)
        {
            Vector(Hero->Object.Position[0] + (float)(rand() % 1600 - 800), Hero->Object.Position[1] + (float)(rand() % 1400 - 500), Hero->Object.Position[2] + (float)(rand() % 1000 + 50), Position);
        }
        else
        {
            Vector(Hero->Object.Position[0] + (float)(rand() % 1600 - 800), Hero->Object.Position[1] + (float)(rand() % 1400 - 500), Hero->Object.Position[2] + (float)(rand() % 300 + 50), Position);
        }

        VectorCopy(Position, o->Position);
        VectorCopy(Position, o->StartPosition);
        o->Velocity[0] = -(float)(rand() % 64 + 64) * 0.1f;
        if (Position[1] < CameraPosition[1] + 400.f)
        {
            o->Velocity[0] = -o->Velocity[0] + 3.2f;
        }
        o->Velocity[1] = (float)(rand() % 32 - 16) * 0.1f;
        o->Velocity[2] = (float)(rand() % 32 - 16) * 0.1f;
        o->TurningForce[0] = (float)(rand() % 16 - 8) * 0.1f;
        o->TurningForce[1] = (float)(rand() % 64 - 32) * 0.1f;
        o->TurningForce[2] = (float)(rand() % 16 - 8) * 0.1f;

        Vector(1.f, 0.f, 0.f, o->Light);

        return true;
    }

    void SetAttackDefenseObjectType(OBJECT* o)
    {
        switch (o->Type)
        {
        case 16:
        case 38:
        case 39:
        case 40:
        case 50:
        case 58:
        case 59:
        case 78:
        case 80:
        case 81:
        case 82:
        case 83:
        case 84:
        case 85:
        case 86:
        case 87:
            if (IsBattleCastleStart())
            {
                o->HiddenMesh = -2;
            }
            else
            {
                o->HiddenMesh = -1;
            }
            break;

        case 4:
        case 11:
        case 37:
        case 41:
        case 43:
        case 44:
        case 45:
        case 46:
        case 47:
        case 48:
            //        case 66:
        case 67:
        case 68:
        case 69:
        case 71:
        case 72:
        case 73:
        case 74:
        case 75:
        case 76:
        case 88:
            if (IsBattleCastleStart())
            {
                o->HiddenMesh = -1;
            }
            else
            {
                o->HiddenMesh = -2;
            }
            break;
        }
    }

    bool MoveBattleCastleObjectSetting(int& objCount, int object)
    {
        if (gMapManager.InBattleCastle() == false) return false;

        DWORD  current = timeGetTime();
        double dif = (double)(current - g_iMp3PlayTime) / CLOCKS_PER_SEC;
        if (dif > 1800 || g_iMp3PlayTime == 0)
        {
            g_iMp3PlayTime = current;
            if (IsBattleCastleStart())
            {
                StopMp3(MUSIC_CASTLE_PEACE);
                PlayMp3(MUSIC_CASTLE_BATTLE_START);
                //            PlayMp3 ( MUSIC_CASTLE_BATTLE_ING );
            }
            else
            {
                StopMp3(MUSIC_CASTLE_BATTLE_START);
                //            StopMp3 ( MUSIC_CASTLE_BATTLE_ING );
                PlayMp3(MUSIC_CASTLE_PEACE);
            }
        }

        PlayBuffer(SOUND_BC_AMBIENT);
        if (IsBattleCastleStart() && rand_fps_check(10))
        {
            if (rand_fps_check(100))
            {
                PlayBuffer(SOUND_BC_AMBIENT_BATTLE1);
            }
            if (rand_fps_check(100))
            {
                PlayBuffer(SOUND_BC_AMBIENT_BATTLE2);
            }
            if (rand_fps_check(100))
            {
                PlayBuffer(SOUND_BC_AMBIENT_BATTLE3);
            }
            if (rand_fps_check(100))
            {
                PlayBuffer(SOUND_BC_AMBIENT_BATTLE4);
            }
            if (rand_fps_check(100))
            {
                PlayBuffer(SOUND_BC_AMBIENT_BATTLE5);
            }
        }

        if (IsBattleCastleStart())
        {
            if (LastStoneFlyEffect < WorldTime - StoneFlyEffectInterval)
            {
                LastStoneFlyEffect = WorldTime;
                int HeroY = (Hero->PositionY);
                if (rand() % 3)
                {
                    if (HeroY > 50 && HeroY < 131)
                    {
                        int dx = rand() % 1000 - 500;
                        vec3_t Position;
                        vec3_t Angle = { 0.f, 0.f, 0.f };
                        vec3_t Light = { 1.f, 1.f, 1.f };

                        if (dx < 30 && dx>0) dx = 100 + rand() % 30;
                        else if (dx > -30 && dx < 0) dx -= 100 + rand() % 30;

                        Position[0] = Hero->Object.Position[0] + dx;
                        Position[2] = 500.f;

                        int SiegePositionY;
                        if (HeroY > 55)
                        {
                            SiegePositionY = 45;
                        }
                        else
                        {
                            SiegePositionY = HeroY - (rand() % 10 + 10);
                        }
                        Position[1] = SiegePositionY * TERRAIN_SCALE + (rand() % 40 - 20);
                        CreateEffect(MODEL_FLY_BIG_STONE1, Position, Angle, Light, 0);
                    }
                }
                else
                {
                    if (HeroY > 70 && HeroY < 177)
                    {
                        int dx = rand() % 1000 - 500;
                        vec3_t Position;
                        vec3_t Angle = { 0.f, 0.f, 0.f };
                        vec3_t Light = { 1.f, 1.f, 1.f };

                        if (dx < 100 && dx>0) dx = 100 + rand() % 30;
                        else if (dx > -100 && dx < 0) dx -= 100 + rand() % 30;

                        Position[0] = Hero->Object.Position[0] + dx;
                        Position[2] = 500.f;

                        int SiegePositionY;
                        if (HeroY < 179)
                        {
                            SiegePositionY = 179;
                        }
                        else
                        {
                            SiegePositionY = HeroY - (rand() % 10 + 10);
                        }
                        Position[1] = SiegePositionY * TERRAIN_SCALE - (rand() % 40 - 20);
                        CreateEffect(MODEL_FLY_BIG_STONE2, Position, Angle, Light, 0);
                    }
                }
            }

            int HeroY = (Hero->PositionY);
            if ((HeroY > 58 && HeroY < 113) || (HeroY > 117 && HeroY < 159))
            {
                if (LastArrowEffectOnBattlefield < WorldTime - ArrowEffectOnBattlefieldInterval)
                {
                    LastArrowEffectOnBattlefield = WorldTime;
                    int length = (rand() % 4 + 2) / 2;
                    int dx = (rand() % 1400 - 700) - 60.f * length;

                    if (dx < 100 && dx>0) dx = 100 + rand() % 30;
                    else if (dx > -100 && dx < 0) dx -= 100 + rand() % 30;

                    vec3_t Position;
                    vec3_t Angle = { 0.f, 0.f, 0.f };
                    vec3_t Light = { 1.f, 1.f, 1.f };
                    for (int i = -length; i < length; ++i)
                    {
                        dx += 60.f;

                        Position[0] = Hero->Object.Position[0] + dx;
                        Position[2] = 350.f;

                        bool attArrow = rand() % 2;
                        if (HeroY < 113)
                        {
                            attArrow = true;
                        }
                        else if (HeroY > 117)
                        {
                            attArrow = false;
                        }

                        if (attArrow)
                        {
                            int SiegePositionY = HeroY - (rand() % 1 + 10);
                            Position[1] = SiegePositionY * TERRAIN_SCALE/*+(rand()%60-30)*/;

                            Angle[2] = 180.f;
                            CreateEffect(MODEL_ARROW, Position, Angle, Light, 4);
                        }
                        else
                        {
                            int SiegePositionY = HeroY + (rand() % 1 + 10);
                            Position[1] = SiegePositionY * TERRAIN_SCALE/*-(rand()%60-30)*/;

                            CreateEffect(MODEL_ARROW, Position, Angle, Light, 4);
                        }
                    }
                }
            }

            if (LastArrowEffectInStampRoom < WorldTime - ArrowEffectInStampRoomInterval)
            {
                LastArrowEffectInStampRoom = WorldTime;
                if (HeroY > 58 && HeroY < 159)
                {
                    vec3_t Position;
                    vec3_t Angle = { 0.f, 0.f, 0.f };
                    vec3_t Light = { 1.f, 1.f, 1.f };

                    int dx = rand() % 1400 - 700;
                    if (dx < 100 && dx>0) dx = 100 + rand() % 30;
                    else if (dx > -100 && dx < 0) dx -= 100 + rand() % 30;

                    Position[0] = Hero->Object.Position[0] + dx;
                    Position[2] = 350.f;

                    BYTE subtype = 3;
                    if (HeroY < 82 || HeroY>112 || rand_fps_check(10))
                    {
                        subtype = 4;
                        if (rand_fps_check(10))
                        {
                            subtype = 3;
                        }
                    }

                    bool attArrow = rand() % 2;
                    if (HeroY < 82)
                    {
                        attArrow = true;
                    }
                    else if (HeroY > 112)
                    {
                        attArrow = false;
                    }

                    if (attArrow)
                    {
                        int SiegePositionY = HeroY - (rand() % 3 + 10);
                        Position[1] = SiegePositionY * TERRAIN_SCALE + (rand() % 60 - 30);

                        Angle[2] = 180.f;
                        CreateEffect(MODEL_ARROW, Position, Angle, Light, subtype);
                    }
                    else
                    {
                        int SiegePositionY = HeroY + (rand() % 3 + 10);
                        Position[1] = SiegePositionY * TERRAIN_SCALE - (rand() % 60 - 30);

                        CreateEffect(MODEL_ARROW, Position, Angle, Light, subtype);
                    }
                }
            }
        }
        return true;
    }

    bool MoveBattleCastleObject(OBJECT* o, int& object, int& visibleObject)
    {
        if (gMapManager.InBattleCastle() == false)  return false;

        switch (o->Type)
        {
        case 81:
            o->BlendMesh = 1;
            o->BlendMeshLight = 1.f;
            o->BlendMeshTexCoordV = WorldTime * 0.0002f;
            break;

        case 83:
            o->BlendMesh = 1;
            o->BlendMeshLight = 1.f;
            o->BlendMeshTexCoordV = -WorldTime * 0.0004f;
            break;

        case BATTLE_CASTLE_WALL1:
        case BATTLE_CASTLE_WALL2:
            if (IsBattleCastleStart() == false) break;

            if (o->ExtState == 0)
            {
                o->HiddenMesh = -1;
            }
            else if (o->ExtState == 99 && o->Visible)
            {
                if (o->HiddenMesh == -1)
                {
                    CreateEffect(o->Type, o->Position, o->Angle, o->Light, 1);
                    PlayBuffer(SOUND_BC_WALL_HIT);
                }
                o->HiddenMesh = 0;
            }
            break;
        case BATTLE_CASTLE_WALL3:
        case BATTLE_CASTLE_WALL4:
            if (IsBattleCastleStart() == false) break;

            if (o->ExtState == 0)
            {
                o->HiddenMesh = -1;
            }
            else if (o->ExtState == 99 && o->Visible)
            {
                if (o->HiddenMesh == -1)
                {
                    CreateEffect(o->Type, o->Position, o->Angle, o->Light, 1);
                    PlayBuffer(SOUND_BC_WALL_HIT);
                }
                o->HiddenMesh = 0;
            }
            break;
        }

        SetAttackDefenseObjectType(o);
        return false;
    }

    bool CreateBattleCastleObject(OBJECT* o)
    {
        if (gMapManager.InBattleCastle() == false) return false;

        switch (o->Type)
        {
        case 8:
        case 9:
            o->ExtState = 0;
            break;

        case 7:
        case 10:
        case 13:
        case 14:
            //        case    18 :
            o->ExtState = 0;
            o->m_bCollisionCheck = true;
            break;

        case    BATTLE_CASTLE_WALL1:
        case    BATTLE_CASTLE_WALL2:
        case    BATTLE_CASTLE_WALL3:
        case    BATTLE_CASTLE_WALL4:
            o->ExtState = 0;
            o->m_bCollisionCheck = true;
            break;

        case    19:
            o->Scale = 1.f;
            break;

        case    MODEL_BATTLE_GUARD2:
            o->Scale = 1.f;
            if (IsBattleCastleStart())
            {
                o->HiddenMesh = -2;
            }
            else
            {
                o->HiddenMesh = -1;
            }
            break;

        case 39:
            o->HiddenMesh = -2;
            break;

        case 41:
            o->Timer = float(rand() % 1000) * 0.01f;
            break;

        case 77:
        case 84:
            CreateOperate(o);
            break;

        case 79:
            CreateEffect(MODEL_TOWER_GATE_PLANE, o->Position, o->Angle, o->Light, 0, o);
            break;
        }

        SetAttackDefenseObjectType(o);
        return true;
    }

    bool    MoveBattleCastleVisual(OBJECT* o)
    {
        if (gMapManager.InBattleCastle() == false) return false;

        float Luminosity;
        vec3_t Light;

        switch (o->Type)
        {
        case 4:
            o->HiddenMesh = -1;
            o->BlendMeshTexCoordU = WorldTime * 0.0001f;
            break;

        case 19:
            break;

        case 42:
        case 52:
        case 54:
            o->HiddenMesh = -2;
            break;

        case    0:
            if (IsBattleCastleStart() == false)
            {
                Luminosity = (float)(rand() % 4 + 3) * 0.1f;
                Vector(Luminosity, Luminosity * 0.6f, Luminosity * 0.2f, Light);
                AddTerrainLight(o->Position[0], o->Position[1], Light, 3, PrimaryTerrainLight);
            }
            o->HiddenMesh = -2;
            break;

        case 53:
            if (IsBattleCastleStart())
            {
                Luminosity = (float)(rand() % 4 + 3) * 0.1f;
                Vector(Luminosity, Luminosity * 0.6f, Luminosity * 0.2f, Light);
                AddTerrainLight(o->Position[0], o->Position[1], Light, 3, PrimaryTerrainLight);
            }
            o->HiddenMesh = -2;
            break;

        case 66:
            o->HiddenMesh = -2;
            if (IsBattleCastleStart())
            {
                if (g_isCrownState == false)
                {
                    o->HiddenMesh = -1;
                }
                o->BlendMeshTexCoordV = WorldTime * 0.001f;
            }
            break;

        case    BATTLE_CASTLE_WALL1:
        case    BATTLE_CASTLE_WALL2:
        case    BATTLE_CASTLE_WALL3:
        case    BATTLE_CASTLE_WALL4:
            break;

        case    MODEL_BATTLE_GUARD2:
            if (IsBattleCastleStart())
            {
                o->HiddenMesh = -2;
            }
            else
            {
                o->HiddenMesh = -1;
            }
            break;
        }
        return true;
    }

    bool RenderBattleCastleVisual(OBJECT* o, BMD* b)
    {
        if (gMapManager.InBattleCastle() == false) return false;

        vec3_t Light;

        switch (o->Type)
        {
        case 0:
            if (IsBattleCastleStart() == false && rand_fps_check(3))
            {
                Vector(1.f, 1.f, 1.f, Light);
                CreateParticle(BITMAP_TRUE_FIRE, o->Position, o->Angle, Light, 0, o->Scale);
            }
            break;

        case 41:
            o->BlendMeshLight = sinf(WorldTime * 0.002f) * 0.3f + 0.7f;
            break;

        case 42:
            if (IsBattleCastleStart() && rand_fps_check(3))
            {
                Vector(1.f, 1.f, 1.f, Light);
                CreateParticle(BITMAP_SMOKE, o->Position, o->Angle, Light, 21, o->Scale);
            }
            break;
        case 52:
            if (IsBattleCastleStart() == false)
            {
                Vector(1.f, 1.f, 1.f, Light);
                CreateParticleFpsChecked(BITMAP_WATERFALL_5, o->Position, o->Angle, Light, 6, o->Scale);
            }
            break;
        case 53:
            if (IsBattleCastleStart() && rand_fps_check(3))
            {
                Vector(1.f, 1.f, 1.f, Light);
                CreateParticle(BITMAP_TRUE_FIRE, o->Position, o->Angle, Light, 0, o->Scale);
            }
            break;
        case 54:
            if (IsBattleCastleStart() == false)
            {
                Vector(1.f, 1.f, 1.f, Light);
                CreateParticleFpsChecked(BITMAP_WATERFALL_3 + (rand() % 2), o->Position, o->Angle, Light, 0);
            }
            break;
        }

        return true;
    }

    bool    RenderBattleCastleObjectMesh(OBJECT* o, BMD* b)
    {
        if (gMapManager.InBattleCastle() == false)  return false;

        if (o->Type == 12)
        {
            Vector(1.f, 1.f, 1.f, b->BodyLight);
            b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
            b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_INTERFACE_MAP);
            return true;
        }
        else if (o->Type == 17)
        {
            b->StreamMesh = 0;
            Vector(0.45f, 0.45f, 0.45f, b->BodyLight);
            b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, (int)WorldTime % 10000 * 0.0002f, o->HiddenMesh);
            b->StreamMesh = -1;
            return true;
        }
        else if (o->Type == 51)
        {
            b->StreamMesh = 0;
            b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, -(int)WorldTime % 10000 * 0.0002f, o->BlendMeshTexCoordV, o->HiddenMesh);
            b->StreamMesh = -1;
            return true;
        }
        else if (o->Type == 66)
        {
            Vector(0.1f, 0.4f, 0.6f, b->BodyLight);
            b->RenderBody(RENDER_CHROME | RENDER_BRIGHT, o->Alpha, o->BlendMesh, sinf(WorldTime * 0.001f) * 0.2f + 0.3f, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
            return true;
        }
        else if (o->Type == 50 || o->Type == 83 || o->Type == 84 || o->Type == 85)
        {
            b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);

            if (IsBattleCastleStart() == false)
            {
                DisableAlphaBlend();
                glColor3f(0.f, 0.f, 0.f);
                b->RenderBodyShadow(o->BlendMesh, o->HiddenMesh);
            }
            return true;
        }
        else if (o->Type == 19)
        {
            if (IsBattleCastleStart())
            {
                b->BeginRender(o->Alpha);
                glColor3fv(b->BodyLight);
                b->RenderMesh(3, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
                Vector(0.3f, 0.3f, 0.3f, b->BodyLight);
                b->RenderMesh(3, RENDER_BRIGHT | RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_CHROME);
                Vector(0.2f, 0.2f, 0.5f, b->BodyLight);
                b->RenderMesh(3, RENDER_BRIGHT | RENDER_METAL, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_CHROME);
                Vector(1.f, 1.f, 1.f, b->BodyLight);
                b->EndRender();
            }
            else
            {
                b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                Vector(0.3f, 0.3f, 0.3f, b->BodyLight);
                b->RenderBody(RENDER_BRIGHT | RENDER_CHROME, o->Alpha, 0.5f, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh, BITMAP_CHROME);
                Vector(0.2f, 0.2f, 0.5f, b->BodyLight);
                b->RenderBody(RENDER_BRIGHT | RENDER_METAL, o->Alpha, 0.5f, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh, BITMAP_CHROME);
                Vector(1.f, 1.f, 1.f, b->BodyLight);
            }
            return true;
        }
        else if (o->Type >= BATTLE_CASTLE_WALL1 && o->Type <= BATTLE_CASTLE_WALL4)
        {
            if (o->SubType == 0)
            {
                b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
            }
            else
            {
                b->RenderMeshEffect(0, MODEL_WALL_PART1);
            }
            return true;
        }
        else
        {
            return RenderBattleCastleMonsterObjectMesh(o, b);
        }

        return false;
    }

    void    MoveFlyBigStone(OBJECT* o)
    {
    }

    CHARACTER* CreateBattleCastleMonster(EMonsterType Type, int PositionX, int PositionY, int Key)
    {
        if (gMapManager.InBattleCastle() == false) return NULL;

        CHARACTER* c = NULL;
        switch (Type)
        {
        case MONSTER_TRAP:
            c = CreateCharacter(Key, 11, PositionX, PositionY);
            c->NotRotateOnMagicHit = true;
            c->Object.m_bRenderShadow = false;
            break;

        case MONSTER_SHIELD:
            OpenNpc(MODEL_NPC_BARRIER);      //  MODEL_NPC_BARRIER
            c = CreateCharacter(Key, MODEL_NPC_BARRIER, PositionX, PositionY);
            c->NotRotateOnMagicHit = true;
            c->m_bIsSelected = false;
            c->Object.m_bRenderShadow = false;
            c->Object.Scale = 1.52f;
            c->Object.LifeTime = 0;
            c->Weapon[0].Type = -1;
            c->Weapon[1].Type = -1;
            break;

        case MONSTER_CROWN:
            OpenNpc(MODEL_NPC_CROWN);      //  MODEL_NPC_CROWN
            c = CreateCharacter(Key, MODEL_NPC_CROWN, PositionX, PositionY);
            c->NotRotateOnMagicHit = true;
            c->Object.m_bRenderShadow = false;
            c->Weapon[0].Type = -1;
            c->Weapon[1].Type = -1;
            break;

        case MONSTER_CROWN_SWITCH1:
            OpenNpc(MODEL_NPC_CHECK_FLOOR);      //  MODEL_NPC_CHECK_FLOOR
            c = CreateCharacter(Key, MODEL_NPC_CHECK_FLOOR, PositionX, PositionY);
            c->NotRotateOnMagicHit = true;
            c->Weapon[0].Type = -1;
            c->Weapon[1].Type = -1;
            c->Object.Velocity = c->Object.Position[2];
            if (IsBattleCastleStart() == false)
                c->Object.Position[2] -= 100.f;
            break;

        case MONSTER_CROWN_SWITCH2:
            OpenNpc(MODEL_NPC_CHECK_FLOOR);      //  MODEL_NPC_CHECK_FLOOR
            c = CreateCharacter(Key, MODEL_NPC_CHECK_FLOOR, PositionX, PositionY);
            c->NotRotateOnMagicHit = true;
            c->Weapon[0].Type = -1;
            c->Weapon[1].Type = -1;
            c->Object.Velocity = c->Object.Position[2];
            if (IsBattleCastleStart() == false)
                c->Object.Position[2] -= 100.f;
            break;

        case MONSTER_CASTLE_GATE_SWITCH:
            OpenNpc(MODEL_NPC_GATE_SWITCH);      //  MODEL_NPC_GATE_SWITCH
            c = CreateCharacter(Key, MODEL_NPC_GATE_SWITCH, PositionX, PositionY);
            c->NotRotateOnMagicHit = true;
            c->Object.Scale = 1.1f;
            c->Weapon[0].Type = -1;
            c->Weapon[1].Type = -1;
            break;

        case MONSTER_GUARD:
            OpenNpc(77);      //  MODEL_MONSTER01+77
            c = CreateCharacter(Key, MODEL_BATTLE_GUARD2, PositionX, PositionY);
            c->NotRotateOnMagicHit = true;
            c->Object.Scale = 1.1f;
            c->Weapon[0].Type = -1;
            c->Weapon[1].Type = -1;
            break;

        case MONSTER_SLINGSHOT_ATTACK:
            OpenNpc(MODEL_NPC_CAPATULT_ATT);
            c = CreateCharacter(Key, MODEL_NPC_CAPATULT_ATT, PositionX, PositionY);
            c->NotRotateOnMagicHit = true;
            c->Object.m_bRenderShadow = false;
            c->Object.Scale = 0.8f;
            c->Object.m_fEdgeScale = 1.03f;
            c->Weapon[0].Type = -1;
            c->Weapon[1].Type = -1;
            break;

        case MONSTER_SLINGSHOT_DEFENSE:
            OpenNpc(MODEL_NPC_CAPATULT_DEF);   //  MODEL_NPC_CATAPULT_DEF
            c = CreateCharacter(Key, MODEL_NPC_CAPATULT_DEF, PositionX, PositionY);
            c->NotRotateOnMagicHit = true;
            c->Object.m_bRenderShadow = false;
            c->Object.Scale = 0.8f;
            c->Object.m_fEdgeScale = 1.03f;
            c->Weapon[0].Type = -1;
            c->Weapon[1].Type = -1;
            break;

        case MONSTER_SENIOR:
            OpenNpc(MODEL_NPC_SENATUS);      //  MODEL_NPC_SENATUS
            c = CreateCharacter(Key, MODEL_NPC_SENATUS, PositionX, PositionY);
            c->NotRotateOnMagicHit = true;
            c->Object.Scale = 1.1f;
            c->Weapon[0].Type = -1;
            c->Weapon[1].Type = -1;
            break;

        case MONSTER_GUARDSMAN:
            OpenNpc(MODEL_NPC_CLERK);        //
            c = CreateCharacter(Key, MODEL_NPC_CLERK, PositionX, PositionY);
            c->NotRotateOnMagicHit = true;
            c->Object.Scale = 1.f;
            c->Object.SubType = rand() % 2 + 10;
            c->Weapon[0].Type = -1;
            c->Weapon[1].Type = -1;
            break;

        case MONSTER_CASTLE_GATE1:
            OpenMonsterModel(MONSTER_MODEL_CASTLE_GATE1);
            c = CreateCharacter(Key, MODEL_CASTLE_GATE1, PositionX, PositionY);
            c->NotRotateOnMagicHit = true;
            c->Object.Scale = 1.0f;
            c->Object.m_bRenderShadow = false;
            c->Weapon[0].Type = -1;
            c->Weapon[1].Type = -1;
            break;

        case MONSTER_LIFE_STONE:
            OpenMonsterModel(MONSTER_MODEL_LIFE_STONE);
            c = CreateCharacter(Key, MODEL_LIFE_STONE, PositionX, PositionY);
            c->NotRotateOnMagicHit = true;
            c->Object.Scale = 1.0f;
            c->Object.m_bRenderShadow = false;
            c->Object.BlendMesh = 3;
            c->Weapon[0].Type = -1;
            c->Weapon[1].Type = -1;

            CreateEffect(MODEL_AURORA, c->Object.Position, c->Object.Angle, c->Object.Light, 0, &c->Object, 120);
            break;

        case MONSTER_GUARDIAN_STATUE:
            OpenMonsterModel(MONSTER_MODEL_GUARDIAN_STATUE);
            c = CreateCharacter(Key, MODEL_GUARDIAN_STATUE, PositionX, PositionY);
            c->NotRotateOnMagicHit = true;
            c->Object.Scale = 1.f;
            c->Object.m_fEdgeScale = 1.03f;
            c->Object.m_bRenderShadow = false;
            c->Weapon[0].Type = -1;
            c->Weapon[1].Type = -1;

            if (IsBattleCastleStart())
            {
                c->Object.HiddenMesh = -1;
            }
            else
            {
                c->Object.HiddenMesh = -2;
            }
            CreateEffect(MODEL_AURORA, c->Object.Position, c->Object.Angle, c->Object.Light, 0, &c->Object, 120);
            break;

        case MONSTER_GUARDIAN:
            OpenMonsterModel(MONSTER_MODEL_GUARDIAN_STATUE);
            c = CreateCharacter(Key, MODEL_GUARDIAN_STATUE, PositionX, PositionY);
            c->NotRotateOnMagicHit = true;
            c->Object.Scale = 1.f;
            c->Object.m_bRenderShadow = false;
            c->Weapon[0].Type = -1;
            c->Weapon[1].Type = -1;
            break;

        case MONSTER_BATTLE_GUARD1:
            OpenMonsterModel(MONSTER_MODEL_BATTLE_GUARD1);
            c = CreateCharacter(Key, MODEL_BATTLE_GUARD1, PositionX, PositionY);
            c->NotRotateOnMagicHit = true;
            c->Object.Scale = 1.f;
            c->Weapon[0].Type = -1;
            c->Weapon[1].Type = -1;
            break;

        case MONSTER_BATTLE_GUARD2:
            OpenMonsterModel(MONSTER_MODEL_BATTLE_GUARD2);
            c = CreateCharacter(Key, MODEL_BATTLE_GUARD2, PositionX, PositionY);
            c->NotRotateOnMagicHit = true;
            c->Object.Scale = 1.f;
            c->Object.SubType = 30;
            c->Weapon[0].Type = -1;
            c->Weapon[1].Type = -1;
            break;

        case MONSTER_CANON_TOWER:
            OpenMonsterModel(MONSTER_MODEL_CANON_TOWER);
            c = CreateCharacter(Key, MODEL_CANON_TOWER, PositionX, PositionY);
            c->NotRotateOnMagicHit = true;
            c->Object.Scale = 1.f;
            c->Object.m_fEdgeScale = 1.04f;
            c->Object.m_bRenderShadow = false;
            c->Weapon[0].Type = -1;
            c->Weapon[1].Type = -1;

            if (IsBattleCastleStart())
            {
                c->Object.HiddenMesh = -1;
            }
            else
            {
                c->Object.HiddenMesh = -2;
            }
            c->Object.HiddenMesh = -1;
            break;
        }

        return c;
    }

    bool    SettingBattleCastleMonsterLinkBone(CHARACTER* c, int Type)
    {
        switch (Type)
        {
        case MODEL_NPC_CAPATULT_ATT:
            Vector(-200.f, -50.f, -100.f, c->Object.BoundingBoxMin);
            Vector(200.f, 50.f, 100.f, c->Object.BoundingBoxMax);
            break;

        case MODEL_CASTLE_GATE1:
            Vector(-140.f, -140.f, 0.f, c->Object.BoundingBoxMin);
            Vector(140.f, 140.f, 300.f, c->Object.BoundingBoxMax);
            c->SwordCount = 0;
            return true;
        }

        return false;
    }

    bool    StopBattleCastleMonster(CHARACTER* c, OBJECT* o)
    {
        if (gMapManager.InBattleCastle() == false)  return false;

        switch (c->MonsterIndex)
        {
        case MONSTER_BATTLE_GUARD1:
        case MONSTER_BATTLE_GUARD2:
            if (o->CurrentAction == 5)
            {
                o->PriorAction = 4;
                o->PriorAnimationFrame = 0.f;
                o->CurrentAction = 4;
                o->AnimationFrame = 0.f;

                g_byGuardAI = GUARD_ATTACK_READY;
                o->AI = GUARD_READY;
                o->AttackPoint[0] = 300;
                return true;
            }
            else if (o->CurrentAction == 4)
            {
                o->PriorAction = 4;
                o->PriorAnimationFrame = 0.f;
                o->CurrentAction = 4;
                o->AnimationFrame = 0.f;
                return true;
            }
            else if (o->CurrentAction == 2)
            {
                o->PriorAction = 4;
                o->PriorAnimationFrame = 0.f;
                o->CurrentAction = 4;
                o->AnimationFrame = 0.f;
                return true;
            }
            else if (o->CurrentAction == 0)
            {
                o->PriorAction = 0;
                o->PriorAnimationFrame = 0.f;
                o->CurrentAction = 0;
                o->AnimationFrame = 0.f;
                return true;
            }
            return false;

        case MONSTER_SLINGSHOT_ATTACK:
        case MONSTER_SLINGSHOT_DEFENSE:
            SetAction(o, 0);
            return true;

        case MONSTER_CASTLE_GATE1:
        {
            if (g_isCharacterBuff(o, eBuff_CastleGateIsOpen))
            {
                c->m_bIsSelected = false;
                SetAction(o, 2);
            }
            else
            {
                c->m_bIsSelected = true;
                SetAction(o, 0);
            }
        }
        return true;
        }
        return false;
    }

    void    InitGateAttribute(void)
    {
        if (g_bBeGate == false)
        {
            SetCastleGate_Attribute(0, 0, 0, true);
        }
    }

    void    InitEtcSetting(void)
    {
        g_fLifeStoneLocation[0] = 0.f;
        g_fLifeStoneLocation[1] = 0.f;

        InitGateAttribute();
        g_bBeGate = false;
    }

    bool MoveBattleCastleMonster(CHARACTER* c, OBJECT* o)
    {
        if (gMapManager.InBattleCastle() == false)  return false;

        switch (c->MonsterIndex)
        {
        case MONSTER_CROWN:
            o->Position[2] = RequestTerrainHeight(o->Position[0], o->Position[1]) + 200.f;
            g_isCrownState = g_isCharacterBuff(o, eBuff_CastleCrown);
            c->m_bIsSelected = (g_isCrownState ? true : false);
            break;

        case MONSTER_CASTLE_GATE1:
            g_bBeGate = true;
            break;

        case MONSTER_GUARDIAN_STATUE:
            o->HiddenMesh = -1;
            c->m_bIsSelected = true;
            if (IsBattleCastleStart() == false)
            {
                o->HiddenMesh = -2;
                c->m_bIsSelected = false;
            }
            break;

        case MONSTER_BATTLE_GUARD1:
        case MONSTER_BATTLE_GUARD2:
        {
            switch (o->AI)
            {
            case GUARD_STOP:
            {
                float fRange = 500.f;

                o->AttackPoint[0] = 0;

                if (CalcDistanceChrToChr(o, 1, fRange) || g_byGuardAI == GUARD_ATTACK_READY)
                {
                    SetAction(o, 4);

                    g_byGuardAI = GUARD_ATTACK_READY;
                    o->AI = GUARD_READY;
                    o->AttackPoint[0] = 300;
                }
            }
            break;

            case GUARD_READY:
                break;

            case GUARD_ATTACK_READY:
                break;

            case GUARD_ATTACK:
                break;
            }
            if (o->CurrentAction != MONSTER01_SHOCK && o->AI != GUARD_STOP && (o->AttackPoint[0] < 0 || g_byGuardAI == GUARD_STOP))
            {
                float fRange = 500.f;

                if (CalcDistanceChrToChr(o, 1, fRange))//|| g_byGuardAI==GUARD_ATTACK_READY )
                {
                    o->AttackPoint[0] = 300;
                    g_byGuardAI = GUARD_ATTACK_READY;
                }
                else
                {
                    SetAction(o, 0);
                    o->AI = GUARD_STOP;
                    g_byGuardAI = GUARD_STOP;
                }
            }
            o->AttackPoint[0]--;
        }
        break;

        case MONSTER_LIFE_STONE:
            if (c->m_byFriend == 128)
            {
                g_fLifeStoneLocation[0] = o->Position[0];
                g_fLifeStoneLocation[1] = o->Position[1];
            }
            break;
        }
        return false;
    }

    bool SetCurrentAction_BattleCastleMonster(CHARACTER* c, OBJECT* o)
    {
        switch (o->Type)
        {
        case 11:
            break;

        case MODEL_BATTLE_GUARD1:
        case MODEL_BATTLE_GUARD2:
            o->AI = GUARD_ATTACK;
            SetAction(o, 5);
            return true;
        }
        return false;
    }

    bool AttackEffect_BattleCastleMonster(CHARACTER* c, OBJECT* o, BMD* b)
    {
        if (gMapManager.InBattleCastle() == false) return false;

        CHARACTER* tc = NULL;
        OBJECT* to = NULL;
        vec3_t      Light;
        vec3_t      p, Position;

        Vector(0.f, 0.f, 0.f, p);
        Vector(1.f, 1.f, 1.f, Light);
        if (c->TargetCharacter >= 0 && c->TargetCharacter < MAX_CHARACTERS_CLIENT)
        {
            tc = &CharactersClient[c->TargetCharacter];
            to = &tc->Object;
        }

        switch (c->MonsterIndex)
        {
        case MONSTER_TRAP:
            if (c->CheckAttackTime(5))
            {
                VectorCopy(o->Position, Position);
                Position[2] += 500.f;
                CreateEffect(MODEL_BATTLE_GUARD2, Position, o->Angle, o->Light, 0);
                c->SetLastAttackEffectTime();
            }
            return true;

        case MONSTER_CANON_TOWER:
            if (rand_fps_check(1))
            {
                CreateEffect(BITMAP_JOINT_FORCE, o->Position, o->Angle, o->Light);
            }
            return true;
        }
        return false;
    }

    void CreateGuardStoneHealingVisual(CHARACTER* c, float Range)
    {
        OBJECT* o = &c->Object;
        bool bHealing = false;

        if (g_isCharacterBuff(o, eBuff_CastleRegimentDefense))
        {
            for (int i = 0; i < 4; ++i)
            {
                if (InArea(g_fGuardStoneLocation[i][0], g_fGuardStoneLocation[i][1], o->Position, Range))
                {
                    bHealing = true;
                    break;
                }
            }
        }
        else //if ( (o->State&STATE_REGIMENT_ATTACK)==STATE_REGIMENT_ATTACK )
        {
            if (g_fLifeStoneLocation[0] != 0.f && g_fLifeStoneLocation[1] != 0.f && Hero->GuildMarkIndex == c->GuildMarkIndex)
            {
                if (InArea(g_fLifeStoneLocation[0], g_fLifeStoneLocation[1], o->Position, Range))
                {
                    bHealing = true;
                }
            }
        }

        if (bHealing && LastHealingParticle < WorldTime - HealingParticleInterval)
        {
            LastHealingParticle = WorldTime;
            CreateParticle(BITMAP_PLUS, o->Position, o->Angle, o->Light);
        }
    }

    bool MoveBattleCastleMonsterVisual(OBJECT* o, BMD* b)
    {
        if (gMapManager.InBattleCastle() == false) return false;

        float  Luminosity;
        vec3_t Light;

        switch (o->Type)
        {
        case 11:
            //            o->HiddenMesh = -2;
            break;

        case MODEL_NPC_CROWN:
            o->HiddenMesh = -1;
            if (IsBattleCastleStart())
            {
                if (g_isCrownState == false)
                {
                    o->HiddenMesh = -2;
                }
            }
            break;

        case MODEL_CASTLE_GATE1:
            o->Angle[2] = 0.f;
            return true;

        case MODEL_GUARDIAN_STATUE:
            o->Angle[2] = 0.f;

            Luminosity = sinf(WorldTime * 0.0005) * 0.58f + 0.42f;
            Vector(Luminosity * 0.2f, Luminosity * 0.7f, Luminosity * 1.f, Light);
            AddTerrainLight(o->Position[0], o->Position[1], Light, 2, PrimaryTerrainLight);
            return true;

        case MODEL_GREAT_DRAKAN:
            o->Angle[2] = 0.f;
            return true;

        case MODEL_BATTLE_GUARD1:
            o->Angle[2] = 0.f;
            return true;

        case MODEL_BATTLE_GUARD2:
            o->Angle[2] = 0.f;
            return true;

        case MODEL_GOLDEN_GOBLIN:
            o->Angle[2] = 0.f;
            return true;

        case MODEL_CANON_TOWER:
            o->Angle[2] = 45.f;
            return true;
        }

        return false;
    }

    bool    RenderBattleCastleMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b)
    {
        if (gMapManager.InBattleCastle() == false) return false;

        vec3_t Position, Light, p;

        switch (o->Type)
        {
        case MODEL_CASTLE_GATE1:
            break;

        case MODEL_GUARDIAN_STATUE:
            if (IsBattleCastleStart())
            {
                Vector(0.3f, 0.2f, 0.f, Light);
                RenderAurora(BITMAP_MAGIC + 1, RENDER_BRIGHT, o->Position[0], o->Position[1], 9.f, 9.f, Light);
            }
            else
            {
                RenderAurora(BITMAP_MAGIC + 1, RENDER_DARK, o->Position[0], o->Position[1], 5.5f, 5.5f, o->Light);
            }
            return true;

        case MODEL_LIFE_STONE:
            if (o->m_byBuildTime >= 5)
            {
                o->BlendMeshLight = sinf(WorldTime * 0.001f) * 0.3 + 0.3f;

                Vector(0.3f, 0.2f, 0.f, Light);
                RenderAurora(BITMAP_MAGIC + 1, RENDER_BRIGHT, o->Position[0], o->Position[1], 9.f, 9.f, Light);
            }
            else
            {
                Vector(0.8f, 0.6f, 0.3f, Light);
                VectorCopy(o->Position, Position);
                Position[2] += 50.f;
                CreateSprite(BITMAP_LIGHT, Position, 4.f, Light, NULL);
                CreateSprite(BITMAP_SHINY + 1, Position, 2.5f, Light, NULL, rand() % 360);
            }
            return true;

        case MODEL_CANON_TOWER:
            if (IsBattleCastleStart())
            {
                c->m_bIsSelected = true;

                Vector(0.f, 0.f, 0.f, p);
                Vector(0.8f, 0.6f, 0.3f, Light);
                b->TransformPosition(o->BoneTransform[1], p, Position, true);

                Position[2] += sinf(WorldTime * 0.001f) * 60.f + 60.f;

                CreateSprite(BITMAP_LIGHT, Position, 3.f, Light, NULL);
                CreateSprite(BITMAP_SHINY + 1, Position, 1.5f, Light, NULL, rand() % 360);

                b->TransformPosition(o->BoneTransform[2], p, Position, true);
                CreateSprite(BITMAP_LIGHT, Position, 4.f, Light, NULL);

                RenderAurora(BITMAP_MAGIC + 1, RENDER_DARK, o->Position[0], o->Position[1], 5.5f, 5.5f, o->Light);
            }
            else
            {
                c->m_bIsSelected = false;
                o->HiddenMesh = -2;
            }
            return true;

        case MODEL_NPC_CHECK_FLOOR:
            if (IsBattleCastleStart())
            {
                o->Position[2] = o->Velocity;
                c->m_bIsSelected = true;
            }
            else
            {
                o->Position[2] = o->Velocity - 100.f;
                c->m_bIsSelected = false;
            }
            o->BlendMeshLight = sinf(WorldTime * 0.001f) * 0.3f + 0.5f;
            return true;
        }

        return false;
    }

    bool    RenderBattleCastleMonsterCloth(CHARACTER* c, OBJECT* o, bool Translate, int Select)
    {
        return false;
    }

    bool    RenderBattleCastleMonsterObjectMesh(OBJECT* o, BMD* b)
    {
        if (gMapManager.InBattleCastle() == false) return false;

        bool    success = false;

        switch (o->Type)
        {
        case MODEL_NPC_CAPATULT_ATT:
        {
            b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);

            OBB_t   OBB;
            vec3_t  Temp, p, Position;
            BMD* linkBmd = &Models[MODEL_FLY_BIG_STONE1];
            linkBmd->BodyHeight = 0.f;
            linkBmd->ContrastEnable = o->ContrastEnable;
            BodyLight(o, linkBmd);
            linkBmd->BodyScale = 1.3f;
            linkBmd->CurrentAction = 0;

            if (o->CurrentAction == 0 || o->AnimationFrame < 2.f || o->AnimationFrame >= 22.f)
            {
                Vector(0.f, 0.f, 0.f, p);
                b->TransformPosition(o->BoneTransform[12], p, Position, true);
                VectorCopy(Position, linkBmd->BodyOrigin);
                linkBmd->Animation(BoneTransform, 0.f, 0.f, 0, o->Angle, o->Angle, true);
                linkBmd->Transform(BoneTransform, Temp, Temp, &OBB, true);
                linkBmd->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
            }

            if (o->CurrentAction == 1 && o->AnimationFrame > 13.f && o->AnimationFrame < 22.f)
            {
                Vector(0.f, 0.f, 0.f, p);
                b->TransformPosition(o->BoneTransform[44], p, Position, true);
                VectorCopy(Position, linkBmd->BodyOrigin);
                linkBmd->Animation(BoneTransform, 0.f, 0.f, 0, o->Angle, o->Angle, true);
                linkBmd->Transform(BoneTransform, Temp, Temp, &OBB, true);
                linkBmd->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
            }
        }
        success = true;
        break;

        case MODEL_NPC_BARRIER:
            Vector(0.3f, 0.3f, 1.f, b->BodyLight);
            b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh, BITMAP_WATER + WaterTextureNumber);
            Vector(0.3f, 1.f, 0.3f, b->BodyLight);
            b->RenderBody(RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh, BITMAP_WATER + ((int)(WorldTime * 0.01f) % 32));
            success = true;
            break;

        case MODEL_CASTLE_GATE1:
            if (o->CurrentAction == MONSTER01_DIE)
            {
                if (o->CurrentAction == MONSTER01_DIE)
                {
                    o->Live = false;
                }
                PlayBuffer(SOUND_HIT_GATE2);
                b->RenderMeshEffect(0, MODEL_GATE_PART1);
            }
            else
            {
                b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
            }
            break;

        case MODEL_GUARDIAN_STATUE:
            if (IsBattleCastleStart())
            {
                if (o->CurrentAction != MONSTER01_DIE)
                    o->HiddenMesh = -1;

                if (o->CurrentAction == MONSTER01_DIE)
                {
                    if (o->CurrentAction == MONSTER01_DIE)
                    {
                        o->Live = false;
                    }

                    PlayBuffer(SOUND_BC_GUARD_STONE_DIS);
                    b->RenderMeshEffect(0, MODEL_STONE_COFFIN);
                }
                else
                {
                    b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
                    Vector(0.3f, 0.3f, 0.3f, b->BodyLight);
                    b->RenderBody(RENDER_BRIGHT | RENDER_CHROME, o->Alpha, 0.5f, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh, BITMAP_CHROME);
                    Vector(0.2f, 0.2f, 0.5f, b->BodyLight);
                    b->RenderBody(RENDER_BRIGHT | RENDER_METAL, o->Alpha, 0.5f, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh, BITMAP_CHROME);
                    Vector(1.f, 1.f, 1.f, b->BodyLight);
                }
            }
            success = true;
            break;

        case MODEL_LIFE_STONE:
            o->BlendMeshLight = o->m_byBuildTime / 10.f;
            Vector(o->BlendMeshLight, o->BlendMeshLight, o->BlendMeshLight, b->BodyLight);
            if (o->m_byBuildTime >= 2)
            {
                b->RenderMesh(0, RENDER_TEXTURE);
            }
            if (o->m_byBuildTime >= 3)
            {
                b->RenderMesh(2, RENDER_TEXTURE | RENDER_BRIGHT);
            }
            if (o->m_byBuildTime >= 4)
            {
                Vector(o->BlendMeshLight * 0.5f, o->BlendMeshLight * 0.5f, o->BlendMeshLight * 0.5f, b->BodyLight);
                b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT);
            }
            if (o->m_byBuildTime >= 5)
            {
                Vector(0.5f, 0.5f, 0.5f, b->BodyLight);
                b->RenderMesh(3, RENDER_TEXTURE | RENDER_BRIGHT, 1.f, 3, 1.f, WorldTime * 0.0001f);

                Vector(0.2f, 0.2f, 0.2f, b->BodyLight);
                b->RenderMesh(4, RENDER_TEXTURE | RENDER_BRIGHT, 1.f, 3, 1.f, WorldTime * 0.0001f);
            }
            success = true;
            break;
        }

        return success;
    }

    void RenderMonsterHitEffect(OBJECT* o)
    {
        if (gMapManager.InBattleCastle() == false) return;

        switch (o->Type)
        {
        case MODEL_CASTLE_GATE1:
        {
            vec3_t Position;
            VectorCopy(o->Position, Position);

            Position[0] += 60.f;
            Position[1] -= 100.f;
            Position[2] += 150.f;

            if (!g_isCharacterBuff(o, eBuff_CastleGateIsOpen))
            {
                if (!g_isCharacterBuff((&Hero->Object), eBuff_BlessPotion) && !g_isCharacterBuff((&Hero->Object), eBuff_SoulPotion))
                {
                    CreateEffect(MODEL_WAVES, Position, o->Angle, o->Light, 6, NULL, 0);
                }

                for (int i = 0; i < 5; ++i)
                {
                    Position[0] = o->Position[0];
                    Position[1] = o->Position[1];
                    Position[2] = o->Position[2] + 200 + rand() % 30;

                    CreateEffect(MODEL_GATE_PART1 + rand() % 3, Position, o->Angle, o->Light, rand() % 2 + 1);
                }
            }
        }
        break;

        case MODEL_GUARDIAN_STATUE:
        {
            vec3_t Position;
            for (int i = 0; i < 5; i++)
            {
                if (rand_fps_check(2))
                {
                    Position[0] = o->Position[0];
                    Position[1] = o->Position[1];
                    Position[2] = o->Position[2] + 200 + rand() % 30;

                    CreateEffect(MODEL_STONE_COFFIN + 1, Position, o->Angle, o->Light);
                }
            }
        }
        break;
        }
    }
}