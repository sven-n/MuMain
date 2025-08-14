// MapManager.cpp: implementation of the CMapManager class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MapManager.h"
#include "CameraMove.h"
#include "CDirection.h"
#include "DSPlaySound.h"
#include "GlobalBitmap.h"
#include "LoadData.h"
#include "NewUISystem.h"
#include "PersonalShopTitleImp.h"
#include "ZzzBMD.h"
#include "ZzzCharacter.h"
#include "ZzzEffect.h"
#include "ZzzLodTerrain.h"
#include "ZzzObject.h"
#include "ZzzOpenData.h"
#include "ZzzTexture.h"
#include "w_CursedTemple.h"
#include "WSclient.h"


CMapManager gMapManager;

CMapManager::CMapManager() // OK
{
    this->WorldActive = -1;
}

CMapManager::~CMapManager() // OK
{
}

void CMapManager::Load() // OK
{
    int i;
    bool Temp = true;

    LoadBitmap(L"Object8\\drop01.jpg", BITMAP_BUBBLE);

    switch (gMapManager.WorldActive)
    {
    case WD_0LORENCIA:
        gLoadData.AccessModel(MODEL_BIRD01, L"Data\\Object1\\", L"Bird", 1);
        gLoadData.OpenTexture(MODEL_BIRD01, L"Object1\\");
        gLoadData.AccessModel(MODEL_FISH01, L"Data\\Object1\\", L"Fish", 1);
        gLoadData.OpenTexture(MODEL_FISH01, L"Object1\\");
        break;
    case WD_1DUNGEON:
    case WD_4LOSTTOWER:
        gLoadData.AccessModel(MODEL_DUNGEON_STONE01, L"Data\\Object2\\", L"DungeonStone", 1);
        gLoadData.OpenTexture(MODEL_DUNGEON_STONE01, L"Object2\\");
        gLoadData.AccessModel(MODEL_BAT01, L"Data\\Object2\\", L"Bat", 1);
        gLoadData.OpenTexture(MODEL_BAT01, L"Object2\\");
        gLoadData.AccessModel(MODEL_RAT01, L"Data\\Object2\\", L"Rat", 1);
        gLoadData.OpenTexture(MODEL_RAT01, L"Object2\\");
        break;
    case WD_2DEVIAS:
    {
        vec3_t Pos, Ang;
        gLoadData.AccessModel(MODEL_NPC_SERBIS_DONKEY, L"Data\\Npc\\", L"obj_donkey");
        gLoadData.OpenTexture(MODEL_NPC_SERBIS_DONKEY, L"Npc\\");
        gLoadData.AccessModel(MODEL_NPC_SERBIS_FLAG, L"Data\\Npc\\", L"obj_flag");
        gLoadData.OpenTexture(MODEL_NPC_SERBIS_FLAG, L"Npc\\");

        Vector(0.f, 0.f, 0.f, Ang);
        Vector(0.f, 0.f, 270.f, Pos);
        Pos[0] = 191 * TERRAIN_SCALE; Pos[1] = 16 * TERRAIN_SCALE;
        CreateObject(MODEL_NPC_SERBIS_DONKEY, Pos, Ang);
        Pos[0] = 191 * TERRAIN_SCALE; Pos[1] = 17 * TERRAIN_SCALE;
        CreateObject(MODEL_NPC_SERBIS_FLAG, Pos, Ang);

        gLoadData.AccessModel(MODEL_WARP, L"Data\\Npc\\", L"warp01");
        gLoadData.AccessModel(MODEL_WARP2, L"Data\\Npc\\", L"warp02");
        gLoadData.AccessModel(MODEL_WARP3, L"Data\\Npc\\", L"warp03");
        gLoadData.OpenTexture(MODEL_WARP, L"Npc\\");
        gLoadData.OpenTexture(MODEL_WARP2, L"Npc\\");
        gLoadData.OpenTexture(MODEL_WARP3, L"Npc\\");
        Vector(0.f, 0.f, 10.f, Ang);
        Vector(0.f, 0.f, 0.f, Pos);
        Pos[0] = 53 * TERRAIN_SCALE + 50.f;
        Pos[1] = 92 * TERRAIN_SCALE + 20.f;
        CreateObject(MODEL_WARP, Pos, Ang);
    }
    break;
    case WD_3NORIA:
        gLoadData.AccessModel(MODEL_BUTTERFLY01, L"Data\\Object1\\", L"Butterfly", 1);
        gLoadData.OpenTexture(MODEL_BUTTERFLY01, L"Object1\\");
        gLoadData.AccessModel(MODEL_WARP, L"Data\\Npc\\", L"warp01");
        gLoadData.AccessModel(MODEL_WARP2, L"Data\\Npc\\", L"warp02");
        gLoadData.AccessModel(MODEL_WARP3, L"Data\\Npc\\", L"warp03");

        gLoadData.OpenTexture(MODEL_WARP, L"Npc\\");
        gLoadData.OpenTexture(MODEL_WARP2, L"Npc\\");
        gLoadData.OpenTexture(MODEL_WARP3, L"Npc\\");

        vec3_t Pos, Ang;
        Vector(0.f, 0.f, 10.f, Ang);
        Vector(0.f, 0.f, 0.f, Pos);
        Pos[0] = 223 * TERRAIN_SCALE; Pos[1] = 30 * TERRAIN_SCALE;
        CreateObject(MODEL_WARP, Pos, Ang);
        break;
    case WD_5UNKNOWN:
        for (int i = 0; i < 5; i++)
            gLoadData.AccessModel(MODEL_BIG_METEO1 + i, L"Data\\Object6\\", L"Meteo", i + 1);

        gLoadData.AccessModel(MODEL_BOSS_HEAD, L"Data\\Object6\\", L"BossHead", 1);
        gLoadData.AccessModel(MODEL_PRINCESS, L"Data\\Object6\\", L"Princess", 1);
        for (int i = MODEL_BIG_METEO1; i <= MODEL_PRINCESS; i++)
            gLoadData.OpenTexture(i, L"Object6\\", GL_NEAREST);
        break;
    case WD_6STADIUM:
        gLoadData.AccessModel(MODEL_BUG01, L"Data\\Object7\\", L"Bug", 1);
        gLoadData.OpenTexture(MODEL_BUG01, L"Object7\\");
        break;
    case WD_7ATLANSE:
    case WD_67DOPPLEGANGER3:
        for (int i = 1; i < 9; i++)
        {
            gLoadData.AccessModel(MODEL_FISH01 + i, L"Data\\Object8\\", L"Fish", i + 1);
            gLoadData.OpenTexture(MODEL_FISH01 + i, L"Object8\\");
        }

        for (int i = 0; i < 32; i++)
        {
            wchar_t FileName[100];
            if (i < 10)
                swprintf(FileName, L"Object8\\wt0%d.jpg", i);
            else
                swprintf(FileName, L"Object8\\wt%d.jpg", i);

            LoadBitmap(FileName, BITMAP_WATER + i, GL_LINEAR, GL_REPEAT, false);

            if (i < 10)
                swprintf(FileName, L"wt0%d.jpg", i);
            else
                swprintf(FileName, L"wt%d.jpg", i);
            wcscpy(Bitmaps[BITMAP_WATER + i].FileName, FileName);
        }
        break;
    case WD_8TARKAN:
        LoadBitmap(L"Object9\\sand01.jpg", BITMAP_CHROME + 2, GL_LINEAR, GL_REPEAT);
        LoadBitmap(L"Object9\\sand02.jpg", BITMAP_CHROME + 3, GL_LINEAR, GL_REPEAT);
        LoadBitmap(L"Object9\\Impack03.jpg", BITMAP_IMPACT, GL_LINEAR, GL_CLAMP_TO_EDGE);
        gLoadData.AccessModel(MODEL_BUG01 + 1, L"Data\\Object9\\", L"Bug", 2);
        gLoadData.OpenTexture(MODEL_BUG01 + 1, L"Object9\\");
        break;
    case WD_10HEAVEN:
        LoadBitmap(L"Effect\\clouds.jpg", BITMAP_CLOUD, GL_LINEAR, GL_CLAMP_TO_EDGE);
        gLoadData.AccessModel(MODEL_CLOUD, L"Data\\Object11\\", L"cloud");
        gLoadData.OpenTexture(MODEL_CLOUD, L"Object11\\");

        LoadBitmap(L"Effect\\cloudLight.jpg", BITMAP_CLOUD + 1, GL_LINEAR, GL_CLAMP_TO_EDGE);
        break;

    case WD_11BLOODCASTLE1:
    case WD_11BLOODCASTLE1 + 1:
    case WD_11BLOODCASTLE1 + 2:
    case WD_11BLOODCASTLE1 + 3:
    case WD_11BLOODCASTLE1 + 4:
    case WD_11BLOODCASTLE1 + 5:
    case WD_11BLOODCASTLE1 + 6:
    case WD_52BLOODCASTLE_MASTER_LEVEL:
    {
        gLoadData.AccessModel(MODEL_CROW, L"Data\\Object12\\", L"Crow", 1);
        gLoadData.OpenTexture(MODEL_CROW, L"Object12\\");

        for (int i = 0; i < 2; ++i)
            gLoadData.OpenTexture(MODEL_GATE + i, L"Monster\\");

        LoadBitmap(L"Effect\\clouds.jpg", BITMAP_CLOUD, GL_LINEAR, GL_CLAMP_TO_EDGE);
        LoadWaveFile(SOUND_BLOODCASTLE, L"Data\\Sound\\iBloodCastle.wav", 1);
    }
    break;
    case WD_34CRYWOLF_1ST:
    {
        gLoadData.AccessModel(MODEL_SCOLPION, L"Data\\Object35\\", L"scorpion");
        gLoadData.OpenTexture(MODEL_SCOLPION, L"Object35\\");

        LoadBitmap(L"Effect\\Map_Smoke1.jpg", BITMAP_CHROME + 2, GL_LINEAR, GL_REPEAT);
        LoadBitmap(L"Effect\\Map_Smoke2.tga", BITMAP_CHROME + 3, GL_LINEAR, GL_REPEAT);
        LoadBitmap(L"Effect\\mhoujin_R.jpg", BITMAP_MAGIC_CIRCLE, GL_LINEAR, GL_CLAMP_TO_EDGE);

        gLoadData.AccessModel(MODEL_ARROW_TANKER, L"Data\\Monster\\", L"arrowstusk");
        gLoadData.OpenTexture(MODEL_ARROW_TANKER, L"Monster\\");
        gLoadData.AccessModel(MODEL_ARROW_TANKER_HIT, L"Data\\Monster\\", L"arrowstusk");
        gLoadData.OpenTexture(MODEL_ARROW_TANKER_HIT, L"Monster\\");

        LoadBitmap(L"Effect\\Impack03.jpg", BITMAP_EXT_LOG_IN + 2);
        LoadBitmap(L"Logo\\chasellight.jpg", BITMAP_EFFECT);

        LoadWaveFile(SOUND_CRY1ST_AMBIENT, L"Data\\Sound\\w35\\crywolf_ambi.wav", 1, true);
        LoadWaveFile(SOUND_CRY1ST_WWOLF_MOVE1, L"Data\\Sound\\w35\\ww_idle1.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_WWOLF_MOVE2, L"Data\\Sound\\w35\\ww_idle2.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_WWOLF_ATTACK1, L"Data\\Sound\\w35\\ww_attack1.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_WWOLF_ATTACK2, L"Data\\Sound\\w35\\ww_attack2.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_WWOLF_DIE, L"Data\\Sound\\w35\\ww_death.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_SCOUT1_MOVE1, L"Data\\Sound\\w35\\ww_s1_idle1.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_SCOUT1_MOVE2, L"Data\\Sound\\w35\\ww_s1_idle2.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_SCOUT1_ATTACK1, L"Data\\Sound\\w35\\ww_s1_attack1.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_SCOUT1_ATTACK2, L"Data\\Sound\\w35\\ww_s1_attack2.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_SCOUT1_DIE, L"Data\\Sound\\w35\\ww_s1_death.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_SCOUT2_MOVE1, L"Data\\Sound\\w35\\ww_s2_idle1.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_SCOUT2_MOVE2, L"Data\\Sound\\w35\\ww_s2_idle2.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_SCOUT2_ATTACK1, L"Data\\Sound\\w35\\ww_s2_attack1.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_SCOUT2_ATTACK2, L"Data\\Sound\\w35\\ww_s2_attack2.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_SCOUT2_DIE, L"Data\\Sound\\w35\\ww_s2_death.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_SCOUT3_MOVE1, L"Data\\Sound\\w35\\ww_s3_idle1.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_SCOUT3_MOVE2, L"Data\\Sound\\w35\\ww_s3_idle2.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_SCOUT3_ATTACK1, L"Data\\Sound\\w35\\ww_s3_attack1.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_SCOUT3_ATTACK2, L"Data\\Sound\\w35\\ww_s3_attack2.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_SCOUT3_DIE, L"Data\\Sound\\w35\\ww_s3_death.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_SORAM_MOVE1, L"Data\\Sound\\w35\\soram_idle1.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_SORAM_MOVE2, L"Data\\Sound\\w35\\soram_idle2.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_SORAM_ATTACK1, L"Data\\Sound\\w35\\soram_attack1.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_SORAM_ATTACK2, L"Data\\Sound\\w35\\soram_attack2.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_SORAM_DIE, L"Data\\Sound\\w35\\soram_death.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_BALRAM_MOVE1, L"Data\\Sound\\w35\\balram_idle1.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_BALRAM_MOVE2, L"Data\\Sound\\w35\\balram_idle2.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_BALRAM_ATTACK1, L"Data\\Sound\\w35\\balram_attack1.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_BALRAM_ATTACK2, L"Data\\Sound\\w35\\balram_attack2.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_BALRAM_DIE, L"Data\\Sound\\w35\\balram_death.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_BALGAS_MOVE1, L"Data\\Sound\\w35\\balga_idle1.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_BALGAS_MOVE2, L"Data\\Sound\\w35\\balga_idle2.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_BALGAS_ATTACK1, L"Data\\Sound\\w35\\balga_at1.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_BALGAS_ATTACK2, L"Data\\Sound\\w35\\balga_at2.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_BALGAS_DIE, L"Data\\Sound\\w35\\balga_death.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_BALGAS_SKILL1, L"Data\\Sound\\w35\\balga_skill1.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_BALGAS_SKILL2, L"Data\\Sound\\w35\\balga_skill2.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_DARKELF_MOVE1, L"Data\\Sound\\w35\\darkelf_idle1.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_DARKELF_MOVE2, L"Data\\Sound\\w35\\darkelf_idle2.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_DARKELF_ATTACK1, L"Data\\Sound\\w35\\darkelf_at1.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_DARKELF_ATTACK2, L"Data\\Sound\\w35\\darkelf_at2.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_DARKELF_DIE, L"Data\\Sound\\w35\\darkelf_death.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_DARKELF_SKILL1, L"Data\\Sound\\w35\\darkelf_skill1.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_DARKELF_SKILL2, L"Data\\Sound\\w35\\darkelf_skill2.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_DEATHSPIRIT_MOVE1, L"Data\\Sound\\w35\\dths_idle1.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_DEATHSPIRIT_MOVE2, L"Data\\Sound\\w35\\dths_idle2.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_DEATHSPIRIT_ATTACK1, L"Data\\Sound\\w35\\dths_at1.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_DEATHSPIRIT_ATTACK2, L"Data\\Sound\\w35\\dths_at2.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_DEATHSPIRIT_DIE, L"Data\\Sound\\w35\\dths_deat.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_TANKER_ATTACK1, L"Data\\Sound\\w35\\tanker_attack.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_TANKER_DIE, L"Data\\Sound\\w35\\tanker_death.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_SUMMON, L"Data\\Sound\\w35\\spawn_single.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_SUCCESS, L"Data\\Sound\\w35\\CW_win.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_FAILED, L"Data\\Sound\\w35\\CW_lose.wav", 1);
    }
    break;
    case WD_30BATTLECASTLE:
        LoadBitmap(L"Effect\\Map_Smoke1.jpg", BITMAP_CHROME + 2, GL_LINEAR, GL_REPEAT);
        LoadBitmap(L"Effect\\Map_Smoke2.tga", BITMAP_CHROME + 3, GL_LINEAR, GL_REPEAT);
        LoadBitmap(L"World31\\Map1.jpg", BITMAP_INTERFACE_MAP + 1, GL_LINEAR, GL_CLAMP_TO_EDGE);
        LoadBitmap(L"World31\\Map2.jpg", BITMAP_INTERFACE_MAP + 2, GL_LINEAR, GL_CLAMP_TO_EDGE);

        LoadWaveFile(SOUND_BC_AMBIENT, L"Data\\Sound\\BattleCastle\\aSiegeAmbi.wav", 1, true);
        LoadWaveFile(SOUND_BC_AMBIENT_BATTLE1, L"Data\\Sound\\BattleCastle\\RanAmbi1.wav", 1, true);
        LoadWaveFile(SOUND_BC_AMBIENT_BATTLE2, L"Data\\Sound\\BattleCastle\\RanAmbi2.wav", 1, true);
        LoadWaveFile(SOUND_BC_AMBIENT_BATTLE3, L"Data\\Sound\\BattleCastle\\RanAmbi3.wav", 1, true);
        LoadWaveFile(SOUND_BC_AMBIENT_BATTLE4, L"Data\\Sound\\BattleCastle\\RanAmbi4.wav", 1, true);
        LoadWaveFile(SOUND_BC_AMBIENT_BATTLE5, L"Data\\Sound\\BattleCastle\\RanAmbi5.wav", 1, true);
        LoadWaveFile(SOUND_BC_GUARD_STONE_DIS, L"Data\\Sound\\BattleCastle\\oGuardStoneDis.wav", 1, true);
        LoadWaveFile(SOUND_BC_SHIELD_SPACE_DIS, L"Data\\Sound\\BattleCastle\\oProtectionDis.wav", 1, true);
        LoadWaveFile(SOUND_BC_CATAPULT_ATTACK, L"Data\\Sound\\BattleCastle\\oSWFire.wav", 1, true);
        LoadWaveFile(SOUND_BC_CATAPULT_HIT, L"Data\\Sound\\BattleCastle\\oSWHitG.wav", MAX_CHANNEL, true);
        LoadWaveFile(SOUND_BC_WALL_HIT, L"Data\\Sound\\BattleCastle\\oSWHit.wav", MAX_CHANNEL, true);

        LoadWaveFile(SOUND_BC_GATE_OPEN, L"Data\\Sound\\BattleCastle\\oCDoorMove.wav", 1, true);
        LoadWaveFile(SOUND_BC_GUARDIAN_ATTACK, L"Data\\Sound\\BattleCastle\\mGMercAttack.wav", 1, true);
        LoadWaveFile(SOUND_BMS_STUN, L"Data\\Sound\\BattleCastle\\sDStun.wav", MAX_CHANNEL, true);
        LoadWaveFile(SOUND_BMS_STUN_REMOVAL, L"Data\\Sound\\BattleCastle\\sDStunCancel.wav", MAX_CHANNEL, true);
        LoadWaveFile(SOUND_BMS_MANA, L"Data\\Sound\\BattleCastle\\sDSwllMana.wav", MAX_CHANNEL, true);
        LoadWaveFile(SOUND_BMS_INVISIBLE, L"Data\\Sound\\BattleCastle\\sDTrans.wav", MAX_CHANNEL, true);
        LoadWaveFile(SOUND_BMS_VISIBLE, L"Data\\Sound\\BattleCastle\\sDStunCancel.wav", MAX_CHANNEL, true);
        LoadWaveFile(SOUND_BMS_MAGIC_REMOVAL, L"Data\\Sound\\BattleCastle\\sDMagicCancel.wav", MAX_CHANNEL, true);
        LoadWaveFile(SOUND_BCS_RUSH, L"Data\\Sound\\BattleCastle\\sCHaveyBlow.wav", MAX_CHANNEL, true);
        LoadWaveFile(SOUND_BCS_JAVELIN, L"Data\\Sound\\BattleCastle\\sCShockWave.wav", MAX_CHANNEL, true);
        LoadWaveFile(SOUND_BCS_DEEP_IMPACT, L"Data\\Sound\\BattleCastle\\sCFireArrow.wav", MAX_CHANNEL, true);
        LoadWaveFile(SOUND_BCS_DEATH_CANON, L"Data\\Sound\\BattleCastle\\sCMW.wav", MAX_CHANNEL, true);
        LoadWaveFile(SOUND_BCS_ONE_FLASH, L"Data\\Sound\\BattleCastle\\sCColdAttack.wav", MAX_CHANNEL, true);
        LoadWaveFile(SOUND_BCS_SPACE_SPLIT, L"Data\\Sound\\BattleCastle\\sCDarkAttack.wav", MAX_CHANNEL, true);
        LoadWaveFile(SOUND_BCS_BRAND_OF_SKILL, L"Data\\Sound\\BattleCastle\\sCDarkAssist.wav", 1, true);
        break;

    case WD_31HUNTING_GROUND:
        gLoadData.AccessModel(MODEL_BUTTERFLY01, L"Data\\Object1\\", L"Butterfly", 1);
        gLoadData.OpenTexture(MODEL_BUTTERFLY01, L"Object1\\");

        LoadBitmap(L"Effect\\clouds.jpg", BITMAP_CLOUD, GL_LINEAR, GL_CLAMP_TO_EDGE);
        LoadBitmap(L"Monster\\bossmap1_R.jpg", BITMAP_HGBOSS_PATTERN, GL_LINEAR, GL_CLAMP_TO_EDGE);
        LoadBitmap(L"Monster\\bosswing.tga", BITMAP_HGBOSS_WING, GL_NEAREST, GL_REPEAT);
        LoadBitmap(L"Skill\\bossrock1_R.JPG", BITMAP_FISSURE_FIRE, GL_LINEAR, GL_CLAMP_TO_EDGE);

        LoadWaveFile(SOUND_BC_HUNTINGGROUND_AMBIENT, L"Data\\Sound\\w31\\aW31.wav", 1, true);
        LoadWaveFile(SOUND_BC_AXEWARRIOR_MOVE1, L"Data\\Sound\\w31\\mAWidle1.wav", 1);
        LoadWaveFile(SOUND_BC_AXEWARRIOR_MOVE2, L"Data\\Sound\\w31\\mAWidle2.wav", 1);
        LoadWaveFile(SOUND_BC_AXEWARRIOR_ATTACK1, L"Data\\Sound\\w31\\mAWattack1.wav", 1);
        LoadWaveFile(SOUND_BC_AXEWARRIOR_ATTACK2, L"Data\\Sound\\w31\\mAWattack2.wav", 1);
        LoadWaveFile(SOUND_BC_AXEWARRIOR_DIE, L"Data\\Sound\\w31\\mAWdeath.wav", 1);
        LoadWaveFile(SOUND_BC_LIZARDWARRIOR_MOVE1, L"Data\\Sound\\w31\\mLWidle1.wav", 1);
        LoadWaveFile(SOUND_BC_LIZARDWARRIOR_MOVE2, L"Data\\Sound\\w31\\mLWidle2.wav", 1);
        LoadWaveFile(SOUND_BC_LIZARDWARRIOR_ATTACK1, L"Data\\Sound\\w31\\mLWattack1.wav", 1);
        LoadWaveFile(SOUND_BC_LIZARDWARRIOR_ATTACK2, L"Data\\Sound\\w31\\mLWattack2.wav", 1);
        LoadWaveFile(SOUND_BC_LIZARDWARRIOR_DIE, L"Data\\Sound\\w31\\mLWdeath.wav", 1);
        LoadWaveFile(SOUND_BC_POISONGOLEM_MOVE1, L"Data\\Sound\\w31\\mPGidle1.wav", 1);
        LoadWaveFile(SOUND_BC_POISONGOLEM_MOVE2, L"Data\\Sound\\w31\\mPGidle2.wav", 1);
        LoadWaveFile(SOUND_BC_POISONGOLEM_ATTACK1, L"Data\\Sound\\w31\\mPGattack1.wav", 1);
        LoadWaveFile(SOUND_BC_POISONGOLEM_ATTACK2, L"Data\\Sound\\w31\\mPGattack2.wav", 1);
        LoadWaveFile(SOUND_BC_POISONGOLEM_ATTACK3, L"Data\\Sound\\w31\\mPGeff1.wav", 1);
        LoadWaveFile(SOUND_BC_POISONGOLEM_DIE, L"Data\\Sound\\w31\\mPGdeath.wav", 1);
        LoadWaveFile(SOUND_BC_QUEENBEE_MOVE1, L"Data\\Sound\\w31\\mQBidle1.wav", 1);
        LoadWaveFile(SOUND_BC_QUEENBEE_MOVE2, L"Data\\Sound\\w31\\mQBidle2.wav", 1);
        LoadWaveFile(SOUND_BC_QUEENBEE_ATTACK1, L"Data\\Sound\\w31\\mQBattack1.wav", 1);
        LoadWaveFile(SOUND_BC_QUEENBEE_ATTACK2, L"Data\\Sound\\w31\\mQBattack2.wav", 1);
        LoadWaveFile(SOUND_BC_QUEENBEE_DIE, L"Data\\Sound\\w31\\mQBdeath.wav", 1);
        LoadWaveFile(SOUND_BC_FIREGOLEM_MOVE1, L"Data\\Sound\\w31\\mFGidle1.wav", 1);
        LoadWaveFile(SOUND_BC_FIREGOLEM_MOVE2, L"Data\\Sound\\w31\\mFGidle2.wav", 1);
        LoadWaveFile(SOUND_BC_FIREGOLEM_ATTACK1, L"Data\\Sound\\w31\\mFGattack1.wav", 1);
        LoadWaveFile(SOUND_BC_FIREGOLEM_ATTACK2, L"Data\\Sound\\w31\\mFGattack2.wav", 1);
        LoadWaveFile(SOUND_BC_FIREGOLEM_DIE, L"Data\\Sound\\w31\\mFGdeath.wav", 1);
        LoadWaveFile(SOUND_BC_EROHIM_ENTER, L"Data\\Sound\\w31\\mELOidle1.wav", 1);
        LoadWaveFile(SOUND_BC_EROHIM_ATTACK1, L"Data\\Sound\\w31\\mELOattack1.wav", 1);
        LoadWaveFile(SOUND_BC_EROHIM_ATTACK2, L"Data\\Sound\\w31\\mELOattack2.wav", 1);
        LoadWaveFile(SOUND_BC_EROHIM_ATTACK3, L"Data\\Sound\\w31\\mELOeff1.wav", 1);
        LoadWaveFile(SOUND_BC_EROHIM_DIE, L"Data\\Sound\\w31\\mELOdeath.wav", 1);
        break;
    case WD_33AIDA:
        gLoadData.AccessModel(MODEL_BUTTERFLY01, L"Data\\Object1\\", L"Butterfly", 1);
        gLoadData.OpenTexture(MODEL_BUTTERFLY01, L"Object1\\");
        gLoadData.AccessModel(MODEL_TREE_ATTACK, L"Data\\Object34\\", L"tree_eff");
        gLoadData.OpenTexture(MODEL_TREE_ATTACK, L"Object34\\");
        gLoadData.AccessModel(MODEL_BUG01 + 1, L"Data\\Object9\\", L"Bug", 2);
        gLoadData.OpenTexture(MODEL_BUG01 + 1, L"Object9\\");

        LoadBitmap(L"Effect\\clouds.jpg", BITMAP_CLOUD, GL_LINEAR, GL_CLAMP_TO_EDGE);

        LoadWaveFile(SOUND_AIDA_AMBIENT, L"Data\\Sound\\w34\\aida_ambi.wav", 1, true);
        LoadWaveFile(SOUND_AIDA_BLUEGOLEM_MOVE1, L"Data\\Sound\\w34\\bg_idle1.wav", 1);
        LoadWaveFile(SOUND_AIDA_BLUEGOLEM_MOVE2, L"Data\\Sound\\w34\\bg_idle2.wav", 1);
        LoadWaveFile(SOUND_AIDA_BLUEGOLEM_ATTACK1, L"Data\\Sound\\w34\\bg_attack1.wav", 1);
        LoadWaveFile(SOUND_AIDA_BLUEGOLEM_ATTACK2, L"Data\\Sound\\w34\\bg_attack2.wav", 1);
        LoadWaveFile(SOUND_AIDA_BLUEGOLEM_DIE, L"Data\\Sound\\w34\\bg_death.wav", 1);
        LoadWaveFile(SOUND_AIDA_DEATHRAIDER_MOVE1, L"Data\\Sound\\w34\\dr_idle1.wav", 1);
        LoadWaveFile(SOUND_AIDA_DEATHRAIDER_MOVE2, L"Data\\Sound\\w34\\dr_idle2.wav", 1);
        LoadWaveFile(SOUND_AIDA_DEATHRAIDER_ATTACK1, L"Data\\Sound\\w34\\dr_attack1.wav", 1);
        LoadWaveFile(SOUND_AIDA_DEATHRAIDER_ATTACK2, L"Data\\Sound\\w34\\dr_attack2.wav", 1);
        LoadWaveFile(SOUND_AIDA_DEATHRAIDER_DIE, L"Data\\Sound\\w34\\dr_death.wav", 1);
        LoadWaveFile(SOUND_AIDA_DEATHTREE_MOVE1, L"Data\\Sound\\w34\\dt_idle1.wav", 1);
        LoadWaveFile(SOUND_AIDA_DEATHTREE_MOVE2, L"Data\\Sound\\w34\\dt_idle2.wav", 1);
        LoadWaveFile(SOUND_AIDA_DEATHTREE_ATTACK1, L"Data\\Sound\\w34\\dt_attack1.wav", 1);
        LoadWaveFile(SOUND_AIDA_DEATHTREE_ATTACK2, L"Data\\Sound\\w34\\dt_attack2.wav", 1);
        LoadWaveFile(SOUND_AIDA_DEATHTREE_DIE, L"Data\\Sound\\w34\\dt_death.wav", 1);
        LoadWaveFile(SOUND_AIDA_FORESTORC_MOVE1, L"Data\\Sound\\w34\\fo_idle1.wav", 1);
        LoadWaveFile(SOUND_AIDA_FORESTORC_MOVE2, L"Data\\Sound\\w34\\fo_idle2.wav", 1);
        LoadWaveFile(SOUND_AIDA_FORESTORC_ATTACK1, L"Data\\Sound\\w34\\fo_attack1.wav", 1);
        LoadWaveFile(SOUND_AIDA_FORESTORC_ATTACK2, L"Data\\Sound\\w34\\fo_attack2.wav", 1);
        LoadWaveFile(SOUND_AIDA_FORESTORC_DIE, L"Data\\Sound\\w34\\fo_death.wav", 1);
        LoadWaveFile(SOUND_AIDA_HELL_MOVE1, L"Data\\Sound\\w34\\hm_idle1.wav", 1);
        LoadWaveFile(SOUND_AIDA_HELL_MOVE2, L"Data\\Sound\\w34\\hm_idle2.wav", 1);
        LoadWaveFile(SOUND_AIDA_HELL_ATTACK1, L"Data\\Sound\\w34\\hm_attack1.wav", 1);
        LoadWaveFile(SOUND_AIDA_HELL_ATTACK2, L"Data\\Sound\\w34\\hm_firelay.wav", 1);
        LoadWaveFile(SOUND_AIDA_HELL_ATTACK3, L"Data\\Sound\\w34\\hm_bloodywind.wav", 1);
        LoadWaveFile(SOUND_AIDA_HELL_DIE, L"Data\\Sound\\w34\\hm_death.wav", 1);
        LoadWaveFile(SOUND_AIDA_WITCHQUEEN_MOVE1, L"Data\\Sound\\w34\\wq_idle1.wav", 1);
        LoadWaveFile(SOUND_AIDA_WITCHQUEEN_MOVE2, L"Data\\Sound\\w34\\wq_idle2.wav", 1);
        LoadWaveFile(SOUND_AIDA_WITCHQUEEN_ATTACK1, L"Data\\Sound\\w34\\wq_attack1.wav", 1);
        LoadWaveFile(SOUND_AIDA_WITCHQUEEN_ATTACK2, L"Data\\Sound\\w34\\wq_attack2.wav", 1);
        LoadWaveFile(SOUND_AIDA_WITCHQUEEN_DIE, L"Data\\Sound\\w34\\wq_death.wav", 1);
        LoadWaveFile(SOUND_CHAOS_THUNDER01, L"Data\\Sound\\eElec1.wav", 1);
        LoadWaveFile(SOUND_CHAOS_THUNDER02, L"Data\\Sound\\eElec2.wav", 1);
        break;
    case WD_68DOPPLEGANGER4:
        LoadBitmap(L"Effect\\clouds.jpg", BITMAP_CLOUD, GL_LINEAR, GL_CLAMP_TO_EDGE);
        LoadBitmap(L"effect\\water.jpg", BITMAP_TWINTAIL_WATER, GL_LINEAR, GL_CLAMP_TO_EDGE);
    case WD_37KANTURU_1ST:
        gLoadData.AccessModel(MODEL_BUTTERFLY01, L"Data\\Object1\\", L"Butterfly", 1);
        gLoadData.OpenTexture(MODEL_BUTTERFLY01, L"Object1\\");

        LoadWaveFile(SOUND_KANTURU_1ST_BG_WATERFALL, L"Data\\Sound\\w37\\kan_ruin_waterfall.wav", 1);
        LoadWaveFile(SOUND_KANTURU_1ST_BG_ELEC, L"Data\\Sound\\w37\\kan_ruin_elec.wav", 1);
        LoadWaveFile(SOUND_KANTURU_1ST_BG_WHEEL, L"Data\\Sound\\w37\\kan_ruin_wheel.wav", 1);
        LoadWaveFile(SOUND_KANTURU_1ST_BG_PLANT, L"Data\\Sound\\w37\\kan_ruin_plant.wav", 1);
        LoadWaveFile(SOUND_KANTURU_1ST_BG_GLOBAL, L"Data\\Sound\\w37\\kan_ruin_global.wav", 1);
        LoadWaveFile(SOUND_KANTURU_1ST_BER_MOVE1, L"Data\\Sound\\w37\\ber_idle-01.wav", 1);
        LoadWaveFile(SOUND_KANTURU_1ST_BER_MOVE2, L"Data\\Sound\\w37\\ber_idle-02.wav", 1);
        LoadWaveFile(SOUND_KANTURU_1ST_BER_ATTACK1, L"Data\\Sound\\w37\\ber_attack-01.wav", 1);
        LoadWaveFile(SOUND_KANTURU_1ST_BER_ATTACK2, L"Data\\Sound\\w37\\ber_attack-02.wav", 1);
        LoadWaveFile(SOUND_KANTURU_1ST_BER_DIE, L"Data\\Sound\\w37\\ber_death.wav", 1);
        LoadWaveFile(SOUND_KANTURU_1ST_GIGAN_MOVE1, L"Data\\Sound\\w37\\gigan_idle-01.wav", 1);
        LoadWaveFile(SOUND_KANTURU_1ST_GIGAN_ATTACK1, L"Data\\Sound\\w37\\gigan_attack-01.wav", 1);
        LoadWaveFile(SOUND_KANTURU_1ST_GIGAN_ATTACK2, L"Data\\Sound\\w37\\gigan_attack-02.wav", 1);
        LoadWaveFile(SOUND_KANTURU_1ST_GIGAN_DIE, L"Data\\Sound\\w37\\gigan_death.wav", 1);
        LoadWaveFile(SOUND_KANTURU_1ST_GENO_MOVE1, L"Data\\Sound\\w37\\geno_idle-01.wav", 1);
        LoadWaveFile(SOUND_KANTURU_1ST_GENO_MOVE2, L"Data\\Sound\\w37\\geno_idle-02.wav", 1);
        LoadWaveFile(SOUND_KANTURU_1ST_GENO_ATTACK1, L"Data\\Sound\\w37\\geno_attack-01.wav", 1);
        LoadWaveFile(SOUND_KANTURU_1ST_GENO_ATTACK2, L"Data\\Sound\\w37\\geno_attack-02.wav", 1);
        LoadWaveFile(SOUND_KANTURU_1ST_GENO_DIE, L"Data\\Sound\\w37\\geno_death.wav", 1);
        LoadWaveFile(SOUND_KANTURU_1ST_KENTA_MOVE1, L"Data\\Sound\\w37\\kenta_idle-01.wav", 1);
        LoadWaveFile(SOUND_KANTURU_1ST_KENTA_MOVE2, L"Data\\Sound\\w37\\kenta_idle-02.wav", 1);
        LoadWaveFile(SOUND_KANTURU_1ST_KENTA_ATTACK1, L"Data\\Sound\\w37\\kenta_attack-01.wav", 1);
        LoadWaveFile(SOUND_KANTURU_1ST_KENTA_ATTACK2, L"Data\\Sound\\w37\\kenta_skill-01.wav", 1);
        LoadWaveFile(SOUND_KANTURU_1ST_KENTA_DIE, L"Data\\Sound\\w37\\kenta_death.wav", 1);
        LoadWaveFile(SOUND_KANTURU_1ST_BLADE_MOVE1, L"Data\\Sound\\w37\\blade_idle-01.wav", 1);
        LoadWaveFile(SOUND_KANTURU_1ST_BLADE_MOVE2, L"Data\\Sound\\w37\\blade_idle-02.wav", 1);
        LoadWaveFile(SOUND_KANTURU_1ST_BLADE_ATTACK1, L"Data\\Sound\\w37\\blade_attack-01.wav", 1);
        LoadWaveFile(SOUND_KANTURU_1ST_BLADE_ATTACK2, L"Data\\Sound\\w37\\blade_attack-02.wav", 1);
        LoadWaveFile(SOUND_KANTURU_1ST_BLADE_DIE, L"Data\\Sound\\w37\\blade_death.wav", 1);
        LoadWaveFile(SOUND_KANTURU_1ST_SATI_MOVE1, L"Data\\Sound\\w37\\sati_idle-01.wav", 1);
        LoadWaveFile(SOUND_KANTURU_1ST_SATI_MOVE2, L"Data\\Sound\\w37\\sati_idle-02.wav", 1);
        LoadWaveFile(SOUND_KANTURU_1ST_SATI_ATTACK1, L"Data\\Sound\\w37\\sati_attack-01.wav", 1);
        LoadWaveFile(SOUND_KANTURU_1ST_SATI_ATTACK2, L"Data\\Sound\\w37\\sati_attack-02.wav", 1);
        LoadWaveFile(SOUND_KANTURU_1ST_SATI_DIE, L"Data\\Sound\\w37\\sati_death.wav", 1);
        LoadWaveFile(SOUND_KANTURU_1ST_SWOLF_MOVE1, L"Data\\Sound\\w37\\swolf_idle-01.wav", 1);
        LoadWaveFile(SOUND_KANTURU_1ST_SWOLF_MOVE2, L"Data\\Sound\\w37\\swolf_idle-02.wav", 1);
        LoadWaveFile(SOUND_KANTURU_1ST_SWOLF_ATTACK1, L"Data\\Sound\\w37\\swolf_attack-01.wav", 1);
        LoadWaveFile(SOUND_KANTURU_1ST_SWOLF_ATTACK2, L"Data\\Sound\\w37\\swolf_attack-02.wav", 1);
        LoadWaveFile(SOUND_KANTURU_1ST_SWOLF_DIE, L"Data\\Sound\\w37\\swolf_death.wav", 1);
        LoadWaveFile(SOUND_KANTURU_1ST_IR_MOVE1, L"Data\\Sound\\w37\\ir_idle-01.wav", 1);
        LoadWaveFile(SOUND_KANTURU_1ST_IR_MOVE2, L"Data\\Sound\\w37\\ir_idle-02.wav", 1);
        LoadWaveFile(SOUND_KANTURU_1ST_IR_ATTACK1, L"Data\\Sound\\w37\\ir_attack-01.wav", 1);
        LoadWaveFile(SOUND_KANTURU_1ST_IR_ATTACK2, L"Data\\Sound\\w37\\ir_attack-02.wav", 1);
        LoadWaveFile(SOUND_KANTURU_1ST_IR_DIE, L"Data\\Sound\\w37\\ir_death.wav", 1);
        break;
    case WD_38KANTURU_2ND:
    {
        g_TrapCanon.Open_TrapCanon();

        gLoadData.AccessModel(MODEL_STORM2, L"Data\\SKill\\", L"boswind");
        gLoadData.OpenTexture(MODEL_STORM2, L"SKill\\");
        gLoadData.AccessModel(MODEL_STORM3, L"Data\\Skill\\", L"mayatonedo");
        gLoadData.OpenTexture(MODEL_STORM3, L"Skill\\");

        LoadBitmap(L"Object39\\k_effect_01.JPG", BITMAP_KANTURU_2ND_EFFECT1, GL_LINEAR, GL_CLAMP_TO_EDGE);
        LoadBitmap(L"Npc\\khs_kan2gate001.jpg", BITMAP_KANTURU_2ND_NPC1, GL_LINEAR, GL_CLAMP_TO_EDGE);
        LoadBitmap(L"Npc\\khs_kan2gate003.jpg", BITMAP_KANTURU_2ND_NPC2, GL_LINEAR, GL_CLAMP_TO_EDGE);
        LoadBitmap(L"Npc\\khs_kan2gate004.jpg", BITMAP_KANTURU_2ND_NPC3, GL_LINEAR, GL_CLAMP_TO_EDGE);

        LoadWaveFile(SOUND_KANTURU_2ND_MAPSOUND_GEAR, L"Data\\Sound\\w38\\kan_relic_gear.wav", 1);
        LoadWaveFile(SOUND_KANTURU_2ND_MAPSOUND_INCUBATOR, L"Data\\Sound\\w38\\kan_relic_incubator.wav", 1);
        LoadWaveFile(SOUND_KANTURU_2ND_MAPSOUND_HOLE, L"Data\\Sound\\w38\\kan_relic_hole.wav", 1);
        LoadWaveFile(SOUND_KANTURU_2ND_MAPSOUND_GLOBAL, L"Data\\Sound\\w38\\kan_relic_global.wav", 1);
        LoadWaveFile(SOUND_KANTURU_2ND_PERSO_MOVE1, L"Data\\Sound\\w38\\perso_idle-01.wav", 1);
        LoadWaveFile(SOUND_KANTURU_2ND_PERSO_MOVE2, L"Data\\Sound\\w38\\perso_idle-02.wav", 1);
        LoadWaveFile(SOUND_KANTURU_2ND_PERSO_ATTACK1, L"Data\\Sound\\w38\\perso_attack-01.wav", 1);
        LoadWaveFile(SOUND_KANTURU_2ND_PERSO_ATTACK2, L"Data\\Sound\\w38\\perso_attack-02.wav", 1);
        LoadWaveFile(SOUND_KANTURU_2ND_PERSO_DIE, L"Data\\Sound\\w38\\perso_death.wav", 1);
        LoadWaveFile(SOUND_KANTURU_2ND_TWIN_MOVE1, L"Data\\Sound\\w38\\twin_idle-01.wav", 1);
        LoadWaveFile(SOUND_KANTURU_2ND_TWIN_MOVE2, L"Data\\Sound\\w38\\twin_idle-02.wav", 1);
        LoadWaveFile(SOUND_KANTURU_2ND_TWIN_ATTACK1, L"Data\\Sound\\w38\\twin_attack-01.wav", 1);
        LoadWaveFile(SOUND_KANTURU_2ND_TWIN_ATTACK2, L"Data\\Sound\\w38\\twin_attack-02.wav", 1);
        LoadWaveFile(SOUND_KANTURU_2ND_TWIN_DIE, L"Data\\Sound\\w38\\twin_death.wav", 1);
        LoadWaveFile(SOUND_KANTURU_2ND_DRED_MOVE1, L"Data\\Sound\\w38\\dred_idle-01.wav", 1);
        LoadWaveFile(SOUND_KANTURU_2ND_DRED_MOVE2, L"Data\\Sound\\w38\\dred_idle-02.wav", 1);
        LoadWaveFile(SOUND_KANTURU_2ND_DRED_ATTACK1, L"Data\\Sound\\w38\\dred_attack-01.wav", 1);
        LoadWaveFile(SOUND_KANTURU_2ND_DRED_ATTACK2, L"Data\\Sound\\w38\\dred_attack-02.wav", 1);
        LoadWaveFile(SOUND_KANTURU_2ND_DRED_DIE, L"Data\\Sound\\w38\\dred_death.wav", 1);
    }
    break;
    case WD_39KANTURU_3RD:
    {
        LoadBitmap(L"Monster\\nightmare_R.jpg", BITMAP_NIGHTMARE_EFFECT1, GL_LINEAR, GL_CLAMP_TO_EDGE);
        LoadBitmap(L"Monster\\nightmaresward_R.jpg", BITMAP_NIGHTMARE_EFFECT2, GL_LINEAR, GL_CLAMP_TO_EDGE);
        LoadBitmap(L"Monster\\nightmare_cloth.tga", BITMAP_NIGHTMARE_ROBE);
        LoadBitmap(L"Object40\\maya01_R.jpg", BITMAP_MAYA_BODY, GL_LINEAR, GL_CLAMP_TO_EDGE);
        LoadBitmap(L"Object40\\Mtowereffe.JPG", BITMAP_KANTURU3RD_OBJECT, GL_LINEAR, GL_CLAMP_TO_EDGE);
        LoadBitmap(L"Effect\\bluering0001_R.jpg", BITMAP_ENERGY_RING, GL_LINEAR, GL_CLAMP_TO_EDGE);
        LoadBitmap(L"Effect\\bluewave0001_R.jpg", BITMAP_ENERGY_FIELD, GL_LINEAR, GL_CLAMP_TO_EDGE);

        gLoadData.AccessModel(MODEL_STORM2, L"Data\\SKill\\", L"boswind");
        gLoadData.OpenTexture(MODEL_STORM2, L"SKill\\");
        gLoadData.AccessModel(MODEL_STORM3, L"Data\\Skill\\", L"mayatonedo");
        gLoadData.OpenTexture(MODEL_STORM3, L"Skill\\");
        gLoadData.AccessModel(MODEL_MAYASTAR, L"Data\\Skill\\", L"arrowsre05");
        gLoadData.OpenTexture(MODEL_MAYASTAR, L"Skill\\");
        gLoadData.AccessModel(MODEL_MAYASTONE1, L"Data\\Skill\\", L"mayastone01");
        gLoadData.OpenTexture(MODEL_MAYASTONE1, L"Skill\\");
        gLoadData.AccessModel(MODEL_MAYASTONE2, L"Data\\Skill\\", L"mayastone02");
        gLoadData.OpenTexture(MODEL_MAYASTONE2, L"Skill\\");
        gLoadData.AccessModel(MODEL_MAYASTONE3, L"Data\\Skill\\", L"mayastone03");
        gLoadData.OpenTexture(MODEL_MAYASTONE3, L"Skill\\");
        gLoadData.AccessModel(MODEL_MAYASTONE4, L"Data\\Skill\\", L"mayastone04");
        gLoadData.OpenTexture(MODEL_MAYASTONE4, L"Skill\\");
        gLoadData.AccessModel(MODEL_MAYASTONE5, L"Data\\Skill\\", L"mayastone05");
        gLoadData.OpenTexture(MODEL_MAYASTONE5, L"Skill\\");
        gLoadData.AccessModel(MODEL_MAYASTONEFIRE, L"Data\\Skill\\", L"mayastonebluefire");
        gLoadData.OpenTexture(MODEL_MAYASTONEFIRE, L"Skill\\");
        gLoadData.AccessModel(MODEL_MAYAHANDSKILL, L"Data\\Skill\\", L"hendlight02");
        gLoadData.OpenTexture(MODEL_MAYAHANDSKILL, L"Skill\\");

        LoadWaveFile(SOUND_KANTURU_3RD_MAYA_INTRO, L"Data\\Sound\\w39\\maya_intro.wav", 1);
        LoadWaveFile(SOUND_KANTURU_3RD_MAYA_END, L"Data\\Sound\\w39\\maya_death.wav", 1);
        LoadWaveFile(SOUND_KANTURU_3RD_MAYA_STORM, L"Data\\Sound\\w39\\maya_storm.wav", 1);
        LoadWaveFile(SOUND_KANTURU_3RD_MAYAHAND_ATTACK1, L"Data\\Sound\\w39\\maya_hand_attack-01.wav", 1);
        LoadWaveFile(SOUND_KANTURU_3RD_MAYAHAND_ATTACK2, L"Data\\Sound\\w39\\maya_hand_attack-02.wav", 1);
        LoadWaveFile(SOUND_KANTURU_3RD_NIGHTMARE_INTRO, L"Data\\Sound\\w39\\nightmare_intro.wav", 1);
        LoadWaveFile(SOUND_KANTURU_3RD_NIGHTMARE_ATT1, L"Data\\Sound\\w39\\nightmare_attack-01.wav", 1);
        LoadWaveFile(SOUND_KANTURU_3RD_NIGHTMARE_ATT2, L"Data\\Sound\\w39\\nightmare_skill-01", 1);
        LoadWaveFile(SOUND_KANTURU_3RD_NIGHTMARE_ATT3, L"Data\\Sound\\w39\\nightmare_skill-02", 1);
        LoadWaveFile(SOUND_KANTURU_3RD_NIGHTMARE_ATT4, L"Data\\Sound\\w39\\nightmare_skill-03", 1);
        LoadWaveFile(SOUND_KANTURU_3RD_NIGHTMARE_DIE, L"Data\\Sound\\w39\\nightmare_death.wav", 1);
        LoadWaveFile(SOUND_KANTURU_3RD_NIGHTMARE_IDLE1, L"Data\\Sound\\w39\\nightmare_idle-01.wav", 1);
        LoadWaveFile(SOUND_KANTURU_3RD_NIGHTMARE_IDLE2, L"Data\\Sound\\w39\\nightmare_idle-02.wav", 1);
        LoadWaveFile(SOUND_KANTURU_3RD_NIGHTMARE_TELE, L"Data\\Sound\\w39\\nightmare_tele.wav", 1);
        LoadWaveFile(SOUND_KANTURU_3RD_MAP_SOUND01, L"Data\\Sound\\w39\\kan_boss_crystal.wav", 1);
        LoadWaveFile(SOUND_KANTURU_3RD_MAP_SOUND02, L"Data\\Sound\\w39\\kan_boss_disfield.wav", 1);
        LoadWaveFile(SOUND_KANTURU_3RD_MAP_SOUND03, L"Data\\Sound\\w39\\kan_boss_field.wav", 1);
        LoadWaveFile(SOUND_KANTURU_3RD_MAP_SOUND04, L"Data\\Sound\\w39\\kan_boss_gear.wav", 1);
        LoadWaveFile(SOUND_KANTURU_3RD_MAP_SOUND05, L"Data\\Sound\\w39\\kan_boss_incubator.wav", 1);
        LoadWaveFile(SOUND_KANTURU_3RD_AMBIENT, L"Data\\Sound\\w39\\kan_boss_global.wav", 1);
    }
    break;
    case WD_45CURSEDTEMPLE_LV1:
    case WD_45CURSEDTEMPLE_LV2:
    case WD_45CURSEDTEMPLE_LV3:
    case WD_45CURSEDTEMPLE_LV4:
    case WD_45CURSEDTEMPLE_LV5:
    case WD_45CURSEDTEMPLE_LV6:
    {
        LoadBitmap(L"Effect\\clouds.jpg", BITMAP_CLOUD, GL_LINEAR, GL_CLAMP_TO_EDGE);
        LoadBitmap(L"Effect\\clud64.jpg", BITMAP_CLUD64, GL_LINEAR, GL_CLAMP_TO_EDGE);
        LoadBitmap(L"Effect\\ghosteffect01.jpg", BITMAP_GHOST_CLOUD1, GL_LINEAR, GL_CLAMP_TO_EDGE);
        LoadBitmap(L"Effect\\ghosteffect02.jpg", BITMAP_GHOST_CLOUD2, GL_LINEAR, GL_CLAMP_TO_EDGE);
        LoadBitmap(L"Effect\\torchfire.jpg", BITMAP_TORCH_FIRE, GL_LINEAR, GL_CLAMP_TO_EDGE);
        LoadBitmap(L"Effect\\clouds2.jpg", BITMAP_EVENT_CLOUD, GL_LINEAR, GL_CLAMP_TO_EDGE);

        //EFFECT
        gLoadData.AccessModel(MODEL_FALL_STONE_EFFECT, L"Data\\Object47\\", L"Stoneeffec");
        gLoadData.OpenTexture(MODEL_FALL_STONE_EFFECT, L"Object47\\");
        //game system sound
        LoadWaveFile(SOUND_CURSEDTEMPLE_GAMESYSTEM1, L"Data\\Sound\\w47\\cursedtemple_start01.wav", 1);
        LoadWaveFile(SOUND_CURSEDTEMPLE_GAMESYSTEM2, L"Data\\Sound\\w47\\cursedtemple_statue01.wav", 1);
        LoadWaveFile(SOUND_CURSEDTEMPLE_GAMESYSTEM3, L"Data\\Sound\\w47\\cursedtemple_holy01.wav", 1);
        LoadWaveFile(SOUND_CURSEDTEMPLE_GAMESYSTEM4, L"Data\\Sound\\w47\\cursedtemple_score01.wav", 1);
        LoadWaveFile(SOUND_CURSEDTEMPLE_GAMESYSTEM5, L"Data\\Sound\\w47\\cursedtemple_end01.wav", 1);
        //moster 1 - 2 effect sound
        LoadWaveFile(SOUND_CURSEDTEMPLE_MONSTER1_IDLE, L"Data\\Sound\\w47\\cursedtemple_idle01.wav", 1);
        LoadWaveFile(SOUND_CURSEDTEMPLE_MONSTER_MOVE, L"Data\\Sound\\w47\\cursedtemple_move01.wav", 1);
        LoadWaveFile(SOUND_CURSEDTEMPLE_MONSTER1_DAMAGE, L"Data\\Sound\\w47\\cursedtemple_damage01.wav", 1);
        LoadWaveFile(SOUND_CURSEDTEMPLE_MONSTER1_DEATH, L"Data\\Sound\\w47\\cursedtemple_death01.wav", 1);
        //moster 3 effect sound
        LoadWaveFile(SOUND_CURSEDTEMPLE_MONSTER2_IDLE, L"Data\\Sound\\w47\\cursedtemple_idle02.wav", 1);
        LoadWaveFile(SOUND_CURSEDTEMPLE_MONSTER2_ATTACK, L"Data\\Sound\\w47\\cursedtemple_Attack01.wav", 1);
        LoadWaveFile(SOUND_CURSEDTEMPLE_MONSTER2_DAMAGE, L"Data\\Sound\\w47\\cursedtemple_damage02.wav", 1);
        LoadWaveFile(SOUND_CURSEDTEMPLE_MONSTER2_DEATH, L"Data\\Sound\\w47\\cursedtemple_death02.wav", 1);
    }
    break;
    case WD_41CHANGEUP3RD_1ST:
        LoadBitmap(L"Effect\\clouds.jpg", BITMAP_CLOUD, GL_LINEAR, GL_CLAMP_TO_EDGE);
        LoadBitmap(L"Effect\\firered.jpg", BITMAP_FIRE_RED, GL_LINEAR, GL_CLAMP_TO_EDGE);
        LoadBitmap(L"Effect\\FireSnuff.jpg", BITMAP_FIRE_SNUFF, GL_LINEAR, GL_CLAMP_TO_EDGE);      //  불씨.

        LoadWaveFile(SOUND_3RD_CHANGE_UP_BG_CAGE1, L"Data\\Sound\\w42\\cage01.wav", 1);
        LoadWaveFile(SOUND_3RD_CHANGE_UP_BG_CAGE2, L"Data\\Sound\\w42\\cage02.wav", 1);
        LoadWaveFile(SOUND_3RD_CHANGE_UP_BG_VOLCANO, L"Data\\Sound\\w42\\volcano.wav", 1);
        LoadWaveFile(SOUND_3RD_CHANGE_UP_BG_FIREPILLAR, L"Data\\Sound\\w42\\firepillar.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_BALRAM_MOVE1, L"Data\\Sound\\w35\\balram_idle1.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_BALRAM_MOVE2, L"Data\\Sound\\w35\\balram_idle2.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_BALRAM_ATTACK1, L"Data\\Sound\\w35\\balram_attack1.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_BALRAM_ATTACK2, L"Data\\Sound\\w35\\balram_attack2.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_BALRAM_DIE, L"Data\\Sound\\w35\\balram_death.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_DEATHSPIRIT_MOVE1, L"Data\\Sound\\w35\\dths_idle1.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_DEATHSPIRIT_MOVE2, L"Data\\Sound\\w35\\dths_idle2.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_DEATHSPIRIT_ATTACK1, L"Data\\Sound\\w35\\dths_at1.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_DEATHSPIRIT_ATTACK2, L"Data\\Sound\\w35\\dths_at2.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_DEATHSPIRIT_DIE, L"Data\\Sound\\w35\\dths_deat.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_SORAM_MOVE1, L"Data\\Sound\\w35\\soram_idle1.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_SORAM_MOVE2, L"Data\\Sound\\w35\\soram_idle2.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_SORAM_ATTACK1, L"Data\\Sound\\w35\\soram_attack1.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_SORAM_ATTACK2, L"Data\\Sound\\w35\\soram_attack2.wav", 1);
        LoadWaveFile(SOUND_CRY1ST_SORAM_DIE, L"Data\\Sound\\w35\\soram_death.wav", 1);
        break;
    case WD_42CHANGEUP3RD_2ND:
        LoadBitmap(L"Effect\\clouds.jpg", BITMAP_CLOUD, GL_LINEAR, GL_CLAMP_TO_EDGE);
        LoadBitmap(L"Effect\\firered.jpg", BITMAP_FIRE_RED, GL_LINEAR, GL_CLAMP_TO_EDGE);
        LoadBitmap(L"Effect\\FireSnuff.jpg", BITMAP_FIRE_SNUFF, GL_LINEAR, GL_CLAMP_TO_EDGE);

        LoadWaveFile(SOUND_3RD_CHANGE_UP_BG_CAGE1, L"Data\\Sound\\w42\\cage01.wav", 1);
        LoadWaveFile(SOUND_3RD_CHANGE_UP_BG_CAGE2, L"Data\\Sound\\w42\\cage02.wav", 1);
        LoadWaveFile(SOUND_3RD_CHANGE_UP_BG_VOLCANO, L"Data\\Sound\\w42\\volcano.wav", 1);
        LoadWaveFile(SOUND_3RD_CHANGE_UP_BG_FIREPILLAR, L"Data\\Sound\\w42\\firepillar.wav", 1);
        break;
    case WD_51HOME_6TH_CHAR:
        LoadBitmap(L"Effect\\clouds.jpg", BITMAP_CLOUD, GL_LINEAR, GL_CLAMP_TO_EDGE);
        LoadBitmap(L"Effect\\Map_Smoke1.jpg", BITMAP_CHROME + 2, GL_LINEAR, GL_REPEAT);
        gLoadData.AccessModel(MODEL_EAGLE, L"Data\\Object52\\", L"sos3bi01");
        gLoadData.OpenTexture(MODEL_EAGLE, L"Object52\\");
        if (gMapManager.WorldActive == WD_51HOME_6TH_CHAR)
        {
            gLoadData.AccessModel(MODEL_MAP_TORNADO, L"Data\\Object52\\", L"typhoonall");
            gLoadData.OpenTexture(MODEL_MAP_TORNADO, L"Object52\\");

            gLoadData.AccessModel(MODEL_TOTEMGOLEM_PART1, L"Data\\Monster\\", L"totemhead");
            gLoadData.OpenTexture(MODEL_TOTEMGOLEM_PART1, L"Monster\\");
            gLoadData.AccessModel(MODEL_TOTEMGOLEM_PART2, L"Data\\Monster\\", L"totembody");
            gLoadData.OpenTexture(MODEL_TOTEMGOLEM_PART2, L"Monster\\");
            gLoadData.AccessModel(MODEL_TOTEMGOLEM_PART3, L"Data\\Monster\\", L"totemleft");
            gLoadData.OpenTexture(MODEL_TOTEMGOLEM_PART3, L"Monster\\");
            gLoadData.AccessModel(MODEL_TOTEMGOLEM_PART4, L"Data\\Monster\\", L"totemright");
            gLoadData.OpenTexture(MODEL_TOTEMGOLEM_PART4, L"Monster\\");
            gLoadData.AccessModel(MODEL_TOTEMGOLEM_PART5, L"Data\\Monster\\", L"totemleg");
            gLoadData.OpenTexture(MODEL_TOTEMGOLEM_PART5, L"Monster\\");
            gLoadData.AccessModel(MODEL_TOTEMGOLEM_PART6, L"Data\\Monster\\", L"totemleg2");
            gLoadData.OpenTexture(MODEL_TOTEMGOLEM_PART6, L"Monster\\");

            LoadWaveFile(SOUND_ELBELAND_VILLAGEPROTECTION01, L"Data\\Sound\\w52\\SE_Obj_villageprotection01.wav", 1);
            LoadWaveFile(SOUND_ELBELAND_WATERFALLSMALL01, L"Data\\Sound\\w52\\SE_Obj_waterfallsmall01.wav", 1);
            LoadWaveFile(SOUND_ELBELAND_WATERWAY01, L"Data\\Sound\\w52\\SE_Obj_waterway01.wav", 1);
            LoadWaveFile(SOUND_ELBELAND_ENTERDEVIAS01, L"Data\\Sound\\w52\\SE_Obj_enterdevias01.wav", 1);
            LoadWaveFile(SOUND_ELBELAND_WATERSMALL01, L"Data\\Sound\\w52\\SE_Obj_watersmall01.wav", 1);
            LoadWaveFile(SOUND_ELBELAND_RAVINE01, L"Data\\Sound\\w52\\SE_Amb_ravine01.wav", 1);
            LoadWaveFile(SOUND_ELBELAND_ENTERATLANCE01, L"Data\\Sound\\w52\\SE_Amb_enteratlance01.wav", 1);
        }
        break;
    case WD_66DOPPLEGANGER2:
    {
        LoadBitmap(L"Effect\\clouds.jpg", BITMAP_CLOUD, GL_LINEAR, GL_CLAMP_TO_EDGE);
        LoadBitmap(L"Effect\\FireSnuff.jpg", BITMAP_FIRE_SNUFF, GL_LINEAR, GL_CLAMP_TO_EDGE);
        LoadBitmap(L"Effect\\WATERFALL2.jpg", BITMAP_CHROME3, GL_LINEAR, GL_CLAMP_TO_EDGE);
        LoadBitmap(L"Effect\\clud64.jpg", BITMAP_CLUD64, GL_LINEAR, GL_CLAMP_TO_EDGE);
    }
    break;
    case WD_63PK_FIELD:
    {
        gLoadData.AccessModel(MODEL_PKFIELD_ASSASSIN_EFFECT_GREEN_HEAD, L"Data\\Monster\\", L"pk_manhead_green");
        gLoadData.OpenTexture(MODEL_PKFIELD_ASSASSIN_EFFECT_GREEN_HEAD, L"Monster\\");

        gLoadData.AccessModel(MODEL_PKFIELD_ASSASSIN_EFFECT_RED_HEAD, L"Data\\Monster\\", L"pk_manhead_red");
        gLoadData.OpenTexture(MODEL_PKFIELD_ASSASSIN_EFFECT_RED_HEAD, L"Monster\\");

        gLoadData.AccessModel(MODEL_PKFIELD_ASSASSIN_EFFECT_GREEN_BODY, L"Data\\Monster\\", L"assassin_dieg");
        gLoadData.OpenTexture(MODEL_PKFIELD_ASSASSIN_EFFECT_GREEN_BODY, L"Monster\\");

        gLoadData.AccessModel(MODEL_PKFIELD_ASSASSIN_EFFECT_RED_BODY, L"Data\\Monster\\", L"assassin_dier");
        gLoadData.OpenTexture(MODEL_PKFIELD_ASSASSIN_EFFECT_RED_BODY, L"Monster\\");
    }
    break;
    case WD_56MAP_SWAMP_OF_QUIET:
        LoadBitmap(L"Effect\\Map_Smoke1.jpg", BITMAP_CHROME + 2, GL_LINEAR, GL_REPEAT);
        LoadBitmap(L"Effect\\Map_Smoke2.tga", BITMAP_CHROME + 3, GL_LINEAR, GL_REPEAT);
        LoadBitmap(L"Monster\\red_shadows.jpg", BITMAP_SHADOW_PAWN_RED, GL_LINEAR, GL_REPEAT);
        LoadBitmap(L"Monster\\blue_shadows.jpg", BITMAP_SHADOW_KINGHT_BLUE, GL_LINEAR, GL_REPEAT);
        LoadBitmap(L"Monster\\green_shadows.jpg", BITMAP_SHADOW_ROOK_GREEN, GL_LINEAR, GL_REPEAT);

        gLoadData.AccessModel(MODEL_SHADOW_PAWN_ANKLE_LEFT, L"Data\\Monster\\", L"shadow_pawn_7_ankle_left");
        gLoadData.OpenTexture(MODEL_SHADOW_PAWN_ANKLE_LEFT, L"Monster\\");
        gLoadData.AccessModel(MODEL_SHADOW_PAWN_ANKLE_RIGHT, L"Data\\Monster\\", L"shadow_pawn_7_ankle_right");
        gLoadData.OpenTexture(MODEL_SHADOW_PAWN_ANKLE_RIGHT, L"Monster\\");
        gLoadData.AccessModel(MODEL_SHADOW_PAWN_BELT, L"Data\\Monster\\", L"shadow_pawn_7_belt");
        gLoadData.OpenTexture(MODEL_SHADOW_PAWN_BELT, L"Monster\\");
        gLoadData.AccessModel(MODEL_SHADOW_PAWN_CHEST, L"Data\\Monster\\", L"shadow_pawn_7_chest");
        gLoadData.OpenTexture(MODEL_SHADOW_PAWN_CHEST, L"Monster\\");
        gLoadData.AccessModel(MODEL_SHADOW_PAWN_HELMET, L"Data\\Monster\\", L"shadow_pawn_7_helmet");
        gLoadData.OpenTexture(MODEL_SHADOW_PAWN_HELMET, L"Monster\\");
        gLoadData.AccessModel(MODEL_SHADOW_PAWN_KNEE_LEFT, L"Data\\Monster\\", L"shadow_pawn_7_knee_left");
        gLoadData.OpenTexture(MODEL_SHADOW_PAWN_KNEE_LEFT, L"Monster\\");
        gLoadData.AccessModel(MODEL_SHADOW_PAWN_KNEE_RIGHT, L"Data\\Monster\\", L"shadow_pawn_7_knee_right");
        gLoadData.OpenTexture(MODEL_SHADOW_PAWN_KNEE_RIGHT, L"Monster\\");
        gLoadData.AccessModel(MODEL_SHADOW_PAWN_WRIST_LEFT, L"Data\\Monster\\", L"shadow_pawn_7_wrist_left");
        gLoadData.OpenTexture(MODEL_SHADOW_PAWN_WRIST_LEFT, L"Monster\\");
        gLoadData.AccessModel(MODEL_SHADOW_PAWN_WRIST_RIGHT, L"Data\\Monster\\", L"shadow_pawn_7_wrist_right");
        gLoadData.OpenTexture(MODEL_SHADOW_PAWN_WRIST_RIGHT, L"Monster\\");
        gLoadData.AccessModel(MODEL_SHADOW_KNIGHT_ANKLE_LEFT, L"Data\\Monster\\", L"shadow_knight_7_ankle_left");
        gLoadData.OpenTexture(MODEL_SHADOW_KNIGHT_ANKLE_LEFT, L"Monster\\");
        gLoadData.AccessModel(MODEL_SHADOW_KNIGHT_ANKLE_RIGHT, L"Data\\Monster\\", L"shadow_knight_7_ankle_right");
        gLoadData.OpenTexture(MODEL_SHADOW_KNIGHT_ANKLE_RIGHT, L"Monster\\");
        gLoadData.AccessModel(MODEL_SHADOW_KNIGHT_BELT, L"Data\\Monster\\", L"shadow_knight_7_belt");
        gLoadData.OpenTexture(MODEL_SHADOW_KNIGHT_BELT, L"Monster\\");
        gLoadData.AccessModel(MODEL_SHADOW_KNIGHT_CHEST, L"Data\\Monster\\", L"shadow_knight_7_chest");
        gLoadData.OpenTexture(MODEL_SHADOW_KNIGHT_CHEST, L"Monster\\");
        gLoadData.AccessModel(MODEL_SHADOW_KNIGHT_HELMET, L"Data\\Monster\\", L"shadow_knight_7_helmet");
        gLoadData.OpenTexture(MODEL_SHADOW_KNIGHT_HELMET, L"Monster\\");
        gLoadData.AccessModel(MODEL_SHADOW_KNIGHT_KNEE_LEFT, L"Data\\Monster\\", L"shadow_knight_7_knee_left");
        gLoadData.OpenTexture(MODEL_SHADOW_KNIGHT_KNEE_LEFT, L"Monster\\");
        gLoadData.AccessModel(MODEL_SHADOW_KNIGHT_KNEE_RIGHT, L"Data\\Monster\\", L"shadow_knight_7_knee_right");
        gLoadData.OpenTexture(MODEL_SHADOW_KNIGHT_KNEE_RIGHT, L"Monster\\");
        gLoadData.AccessModel(MODEL_SHADOW_KNIGHT_WRIST_LEFT, L"Data\\Monster\\", L"shadow_knight_7_wrist_left");
        gLoadData.OpenTexture(MODEL_SHADOW_KNIGHT_WRIST_LEFT, L"Monster\\");
        gLoadData.AccessModel(MODEL_SHADOW_KNIGHT_WRIST_RIGHT, L"Data\\Monster\\", L"shadow_knight_7_wrist_right");
        gLoadData.OpenTexture(MODEL_SHADOW_KNIGHT_WRIST_RIGHT, L"Monster\\");
        gLoadData.AccessModel(MODEL_SHADOW_ROOK_ANKLE_LEFT, L"Data\\Monster\\", L"shadow_rock_7_ankle_left");
        gLoadData.OpenTexture(MODEL_SHADOW_ROOK_ANKLE_LEFT, L"Monster\\");
        gLoadData.AccessModel(MODEL_SHADOW_ROOK_ANKLE_RIGHT, L"Data\\Monster\\", L"shadow_rock_7_ankle_right");
        gLoadData.OpenTexture(MODEL_SHADOW_ROOK_ANKLE_RIGHT, L"Monster\\");
        gLoadData.AccessModel(MODEL_SHADOW_ROOK_BELT, L"Data\\Monster\\", L"shadow_rock_7_belt");
        gLoadData.OpenTexture(MODEL_SHADOW_ROOK_BELT, L"Monster\\");
        gLoadData.AccessModel(MODEL_SHADOW_ROOK_CHEST, L"Data\\Monster\\", L"shadow_rock_7_chest");
        gLoadData.OpenTexture(MODEL_SHADOW_ROOK_CHEST, L"Monster\\");
        gLoadData.AccessModel(MODEL_SHADOW_ROOK_HELMET, L"Data\\Monster\\", L"shadow_rock_7_helmet");
        gLoadData.OpenTexture(MODEL_SHADOW_ROOK_HELMET, L"Monster\\");
        gLoadData.AccessModel(MODEL_SHADOW_ROOK_KNEE_LEFT, L"Data\\Monster\\", L"shadow_rock_7_knee_left");
        gLoadData.OpenTexture(MODEL_SHADOW_ROOK_KNEE_LEFT, L"Monster\\");
        gLoadData.AccessModel(MODEL_SHADOW_ROOK_KNEE_RIGHT, L"Data\\Monster\\", L"shadow_rock_7_knee_right");
        gLoadData.OpenTexture(MODEL_SHADOW_ROOK_KNEE_RIGHT, L"Monster\\");
        gLoadData.AccessModel(MODEL_SHADOW_ROOK_WRIST_LEFT, L"Data\\Monster\\", L"shadow_rock_7_wrist_left");
        gLoadData.OpenTexture(MODEL_SHADOW_ROOK_WRIST_LEFT, L"Monster\\");
        gLoadData.AccessModel(MODEL_SHADOW_ROOK_WRIST_RIGHT, L"Data\\Monster\\", L"shadow_rock_7_wrist_right");
        gLoadData.OpenTexture(MODEL_SHADOW_ROOK_WRIST_RIGHT, L"Monster\\");
        gLoadData.AccessModel(MODEL_EX01_SHADOW_MASTER_ANKLE_LEFT, L"Data\\Monster\\", L"ex01shadow_rock_7_ankle_left");
        gLoadData.OpenTexture(MODEL_EX01_SHADOW_MASTER_ANKLE_LEFT, L"Monster\\");
        gLoadData.AccessModel(MODEL_EX01_SHADOW_MASTER_ANKLE_RIGHT, L"Data\\Monster\\", L"ex01shadow_rock_7_ankle_right");
        gLoadData.OpenTexture(MODEL_EX01_SHADOW_MASTER_ANKLE_RIGHT, L"Monster\\");
        gLoadData.AccessModel(MODEL_EX01_SHADOW_MASTER_BELT, L"Data\\Monster\\", L"ex01shadow_rock_7_belt");
        gLoadData.OpenTexture(MODEL_EX01_SHADOW_MASTER_BELT, L"Monster\\");
        gLoadData.AccessModel(MODEL_EX01_SHADOW_MASTER_CHEST, L"Data\\Monster\\", L"ex01shadow_rock_7_chest");
        gLoadData.OpenTexture(MODEL_EX01_SHADOW_MASTER_CHEST, L"Monster\\");
        gLoadData.AccessModel(MODEL_EX01_SHADOW_MASTER_HELMET, L"Data\\Monster\\", L"ex01shadow_rock_7_helmet");
        gLoadData.OpenTexture(MODEL_EX01_SHADOW_MASTER_HELMET, L"Monster\\");
        gLoadData.AccessModel(MODEL_EX01_SHADOW_MASTER_KNEE_LEFT, L"Data\\Monster\\", L"ex01shadow_rock_7_knee_left");
        gLoadData.OpenTexture(MODEL_EX01_SHADOW_MASTER_KNEE_LEFT, L"Monster\\");
        gLoadData.AccessModel(MODEL_EX01_SHADOW_MASTER_KNEE_RIGHT, L"Data\\Monster\\", L"ex01shadow_rock_7_knee_right");
        gLoadData.OpenTexture(MODEL_EX01_SHADOW_MASTER_KNEE_RIGHT, L"Monster\\");
        gLoadData.AccessModel(MODEL_EX01_SHADOW_MASTER_WRIST_LEFT, L"Data\\Monster\\", L"ex01shadow_rock_7_wrist_left");
        gLoadData.OpenTexture(MODEL_EX01_SHADOW_MASTER_WRIST_LEFT, L"Monster\\");
        gLoadData.AccessModel(MODEL_EX01_SHADOW_MASTER_WRIST_RIGHT, L"Data\\Monster\\", L"ex01shadow_rock_7_wrist_right");
        gLoadData.OpenTexture(MODEL_EX01_SHADOW_MASTER_WRIST_RIGHT, L"Monster\\");
        break;
    case WD_65DOPPLEGANGER1:
        LoadBitmap(L"Effect\\clouds.jpg", BITMAP_CLOUD, GL_LINEAR, GL_CLAMP_TO_EDGE);
        LoadBitmap(L"Effect\\Map_Smoke1.jpg", BITMAP_CHROME + 2, GL_LINEAR, GL_REPEAT);
        LoadBitmap(L"Object9\\sand02.jpg", BITMAP_CHROME + 3, GL_LINEAR, GL_REPEAT);
        LoadBitmap(L"Effect\\Chrome08.jpg", BITMAP_CHROME8, GL_LINEAR, GL_CLAMP_TO_EDGE);
        break;
    case WD_57ICECITY:
    case WD_58ICECITY_BOSS:
        LoadBitmap(L"Effect\\Map_Smoke1.jpg", BITMAP_CHROME + 2, GL_LINEAR, GL_REPEAT);
        LoadBitmap(L"Object9\\sand02.jpg", BITMAP_CHROME + 3, GL_LINEAR, GL_REPEAT);
        LoadBitmap(L"Effect\\Chrome08.jpg", BITMAP_CHROME8, GL_LINEAR, GL_CLAMP_TO_EDGE);

        gLoadData.AccessModel(MODEL_FALL_STONE_EFFECT, L"Data\\Object47\\", L"Stoneeffec");
        gLoadData.OpenTexture(MODEL_FALL_STONE_EFFECT, L"Object47\\");
        gLoadData.AccessModel(MODEL_WARP, L"Data\\Npc\\", L"warp01");
        gLoadData.AccessModel(MODEL_WARP2, L"Data\\Npc\\", L"warp02");
        gLoadData.AccessModel(MODEL_WARP3, L"Data\\Npc\\", L"warp03");
        gLoadData.OpenTexture(MODEL_WARP, L"Npc\\");
        gLoadData.OpenTexture(MODEL_WARP2, L"Npc\\");
        gLoadData.OpenTexture(MODEL_WARP3, L"Npc\\");
        gLoadData.AccessModel(MODEL_WARP4, L"Data\\Npc\\", L"warp01");
        gLoadData.AccessModel(MODEL_WARP5, L"Data\\Npc\\", L"warp02");
        gLoadData.AccessModel(MODEL_WARP6, L"Data\\Npc\\", L"warp03");
        gLoadData.OpenTexture(MODEL_WARP4, L"Npc\\");
        gLoadData.OpenTexture(MODEL_WARP5, L"Npc\\");
        gLoadData.OpenTexture(MODEL_WARP6, L"Npc\\");
        gLoadData.AccessModel(MODEL_SUMMON, L"Data\\SKill\\", L"nightmaresum");
        gLoadData.OpenTexture(MODEL_SUMMON, L"SKill\\");
        gLoadData.AccessModel(MODEL_STORM2, L"Data\\SKill\\", L"boswind");
        gLoadData.OpenTexture(MODEL_STORM2, L"SKill\\");

        LoadWaveFile(SOUND_KANTURU_3RD_MAYAHAND_ATTACK2, L"Data\\Sound\\w39\\maya_hand_attack-02.wav", 1);

        vec3_t vPos, vAngle;
        if (gMapManager.WorldActive == WD_57ICECITY)
        {
            Vector(0.f, 0.f, 35.f, vAngle);
            Vector(0.f, 0.f, 0.f, vPos);
            vPos[0] = 162 * TERRAIN_SCALE;
            vPos[1] = 83 * TERRAIN_SCALE;
            CreateObject(MODEL_WARP, vPos, vAngle);

            Vector(0.f, 0.f, 80.f, vAngle);
            Vector(0.f, 0.f, 0.f, vPos);
            vPos[0] = 171 * TERRAIN_SCALE;
            vPos[1] = 24 * TERRAIN_SCALE;
            CreateObject(MODEL_WARP4, vPos, vAngle);
        }
        else if (gMapManager.WorldActive == WD_58ICECITY_BOSS)
        {
            Vector(0.f, 0.f, 85.f, vAngle);
            Vector(0.f, 0.f, 0.f, vPos);
            vPos[0] = 169 * TERRAIN_SCALE;
            vPos[1] = 24 * TERRAIN_SCALE;
            CreateObject(MODEL_WARP4, vPos, vAngle);

            Vector(0.f, 0.f, 85.f, vAngle);
            Vector(0.f, 0.f, 0.f, vPos);
            vPos[0] = 170 * TERRAIN_SCALE;
            vPos[1] = 24 * TERRAIN_SCALE;
            CreateObject(MODEL_WARP4, vPos, vAngle);
        }
        break;

    case WD_69EMPIREGUARDIAN1:
    case WD_70EMPIREGUARDIAN2:
    case WD_71EMPIREGUARDIAN3:
    case WD_72EMPIREGUARDIAN4:
        gLoadData.AccessModel(MODEL_PROJECTILE, L"Data\\Effect\\", L"choarms_06");
        gLoadData.OpenTexture(MODEL_PROJECTILE, L"Effect\\");
        gLoadData.AccessModel(MODEL_DOOR_CRUSH_EFFECT_PIECE01, L"Data\\Effect\\", L"piece01_01");
        gLoadData.AccessModel(MODEL_DOOR_CRUSH_EFFECT_PIECE02, L"Data\\Effect\\", L"piece01_02");
        gLoadData.AccessModel(MODEL_DOOR_CRUSH_EFFECT_PIECE03, L"Data\\Effect\\", L"piece01_03");
        gLoadData.AccessModel(MODEL_DOOR_CRUSH_EFFECT_PIECE04, L"Data\\Effect\\", L"piece01_04");
        gLoadData.AccessModel(MODEL_DOOR_CRUSH_EFFECT_PIECE05, L"Data\\Effect\\", L"piece01_05");
        gLoadData.AccessModel(MODEL_DOOR_CRUSH_EFFECT_PIECE06, L"Data\\Effect\\", L"piece01_06");
        gLoadData.AccessModel(MODEL_DOOR_CRUSH_EFFECT_PIECE07, L"Data\\Effect\\", L"piece01_07");
        gLoadData.AccessModel(MODEL_DOOR_CRUSH_EFFECT_PIECE08, L"Data\\Effect\\", L"piece01_08");
        gLoadData.AccessModel(MODEL_DOOR_CRUSH_EFFECT_PIECE09, L"Data\\Effect\\", L"newdoor_break_01");
        gLoadData.OpenTexture(MODEL_DOOR_CRUSH_EFFECT_PIECE01, L"Effect\\");
        gLoadData.OpenTexture(MODEL_DOOR_CRUSH_EFFECT_PIECE02, L"Effect\\");
        gLoadData.OpenTexture(MODEL_DOOR_CRUSH_EFFECT_PIECE03, L"Effect\\");
        gLoadData.OpenTexture(MODEL_DOOR_CRUSH_EFFECT_PIECE04, L"Effect\\");
        gLoadData.OpenTexture(MODEL_DOOR_CRUSH_EFFECT_PIECE05, L"Effect\\");
        gLoadData.OpenTexture(MODEL_DOOR_CRUSH_EFFECT_PIECE06, L"Effect\\");
        gLoadData.OpenTexture(MODEL_DOOR_CRUSH_EFFECT_PIECE07, L"Effect\\");
        gLoadData.OpenTexture(MODEL_DOOR_CRUSH_EFFECT_PIECE08, L"Effect\\");
        gLoadData.OpenTexture(MODEL_DOOR_CRUSH_EFFECT_PIECE09, L"Effect\\");
        gLoadData.AccessModel(MODEL_STATUE_CRUSH_EFFECT_PIECE01, L"Data\\Effect\\", L"NpcGagoil_Crack01");
        gLoadData.AccessModel(MODEL_STATUE_CRUSH_EFFECT_PIECE02, L"Data\\Effect\\", L"NpcGagoil_Crack02");
        gLoadData.AccessModel(MODEL_STATUE_CRUSH_EFFECT_PIECE03, L"Data\\Effect\\", L"NpcGagoil_Crack03");
        gLoadData.AccessModel(MODEL_STATUE_CRUSH_EFFECT_PIECE04, L"Data\\Effect\\", L"NpcGagoil_Ruin");
        gLoadData.OpenTexture(MODEL_STATUE_CRUSH_EFFECT_PIECE01, L"Effect\\");
        gLoadData.OpenTexture(MODEL_STATUE_CRUSH_EFFECT_PIECE02, L"Effect\\");
        gLoadData.OpenTexture(MODEL_STATUE_CRUSH_EFFECT_PIECE03, L"Effect\\");
        gLoadData.OpenTexture(MODEL_STATUE_CRUSH_EFFECT_PIECE04, L"Effect\\");
        gLoadData.AccessModel(MODEL_DOOR_CRUSH_EFFECT_PIECE10, L"Data\\Effect\\", L"sojghmoon02");
        gLoadData.AccessModel(MODEL_DOOR_CRUSH_EFFECT_PIECE11, L"Data\\Effect\\", L"sojghmj01");
        gLoadData.AccessModel(MODEL_DOOR_CRUSH_EFFECT_PIECE12, L"Data\\Effect\\", L"sojghmj02");
        gLoadData.AccessModel(MODEL_DOOR_CRUSH_EFFECT_PIECE13, L"Data\\Effect\\", L"sojghmj03");
        gLoadData.OpenTexture(MODEL_DOOR_CRUSH_EFFECT_PIECE10, L"Effect\\");
        gLoadData.OpenTexture(MODEL_DOOR_CRUSH_EFFECT_PIECE11, L"Effect\\");
        gLoadData.OpenTexture(MODEL_DOOR_CRUSH_EFFECT_PIECE12, L"Effect\\");
        gLoadData.OpenTexture(MODEL_DOOR_CRUSH_EFFECT_PIECE13, L"Effect\\");

        LoadBitmap(L"Effect\\clouds.jpg", BITMAP_CLOUD, GL_LINEAR, GL_CLAMP_TO_EDGE);
        LoadBitmap(L"Effect\\flare.jpg", BITMAP_FLARE, GL_LINEAR, GL_CLAMP_TO_EDGE);
        LoadBitmap(L"Effect\\Map_Smoke1.jpg", BITMAP_CHROME + 2, GL_LINEAR, GL_REPEAT);

        LoadWaveFile(SOUND_EMPIREGUARDIAN_WEATHER_RAIN, L"Data\\Sound\\w69w70w71w72\\ImperialGuardianFort_out1.wav");
        LoadWaveFile(SOUND_EMPIREGUARDIAN_WEATHER_FOG, L"Data\\Sound\\w69w70w71w72\\ImperialGuardianFort_out2.wav");
        LoadWaveFile(SOUND_EMPIREGUARDIAN_WEATHER_STORM, L"Data\\Sound\\w69w70w71w72\\ImperialGuardianFort_out3.wav");
        LoadWaveFile(SOUND_EMPIREGUARDIAN_INDOOR_SOUND, L"Data\\Sound\\w69w70w71w72\\ImperialGuardianFort_in.wav");
        LoadWaveFile(SOUND_EMPIREGUARDIAN_BOSS_GAION_MONSTER_MOVE, L"Data\\Sound\\w69w70w71w72\\GaionKalein_move.wav");
        LoadWaveFile(SOUND_EMPIREGUARDIAN_BOSS_GAION_MONSTER_RAGE, L"Data\\Sound\\w69w70w71w72\\GaionKalein_rage.wav");
        LoadWaveFile(SOUND_EMPIREGUARDIAN_BOSS_GAION_MONSTER_DEATH, L"Data\\Sound\\w69w70w71w72\\GrandWizard_death.wav");
        LoadWaveFile(SOUND_EMPIREGUARDIAN_JERINT_MONSTER_ATTACK01, L"Data\\Sound\\w69w70w71w72\\Jelint_attack1.wav");
        LoadWaveFile(SOUND_EMPIREGUARDIAN_JERINT_MONSTER_ATTACK03, L"Data\\Sound\\w69w70w71w72\\Jelint_attack3.wav");
        LoadWaveFile(SOUND_EMPIREGUARDIAN_JERINT_MONSTER_MOVE01, L"Data\\Sound\\w69w70w71w72\\Jelint_move01.wav");
        LoadWaveFile(SOUND_EMPIREGUARDIAN_JERINT_MONSTER_MOVE02, L"Data\\Sound\\w69w70w71w72\\Jelint_move02.wav");
        LoadWaveFile(SOUND_EMPIREGUARDIAN_JERINT_MONSTER_RAGE, L"Data\\Sound\\w69w70w71w72\\Jelint_rage.wav");
        LoadWaveFile(SOUND_EMPIREGUARDIAN_JERINT_MONSTER_DEATH, L"Data\\Sound\\w69w70w71w72\\Jelint_death.wav");
        LoadWaveFile(SOUND_EMPIREGUARDIAN_RAYMOND_MONSTER_ATTACK02, L"Data\\Sound\\w69w70w71w72\\Raymond_attack2.wav");
        LoadWaveFile(SOUND_EMPIREGUARDIAN_RAYMOND_MONSTER_RAGE, L"Data\\Sound\\w69w70w71w72\\Raymond_rage.wav");
        LoadWaveFile(SOUND_EMPIREGUARDIAN_ERCANNE_MONSTER_ATTACK03, L"Data\\Sound\\w69w70w71w72\\Ercanne_attack3.wav");
        LoadWaveFile(SOUND_EMPIREGUARDIAN_1CORP_DEASULER_MONSTER_ATTACK02, L"Data\\Sound\\w69w70w71w72\\1Deasuler_attack2.wav");
        LoadWaveFile(SOUND_EMPIREGUARDIAN_1CORP_DEASULER_MONSTER_ATTACK03, L"Data\\Sound\\w69w70w71w72\\1Deasuler_attack3.wav");
        LoadWaveFile(SOUND_EMPIREGUARDIAN_2CORP_VERMONT_MONSTER_ATTACK01, L"Data\\Sound\\w69w70w71w72\\2Vermont_attack1.wav");
        LoadWaveFile(SOUND_EMPIREGUARDIAN_2CORP_VERMONT_MONSTER_ATTACK02, L"Data\\Sound\\w69w70w71w72\\2Vermont_attack2.wav");
        LoadWaveFile(SOUND_EMPIREGUARDIAN_2CORP_VERMONT_MONSTER_DEATH, L"Data\\Sound\\w69w70w71w72\\2Vermont_death.wav");
        LoadWaveFile(SOUND_EMPIREGUARDIAN_3CORP_CATO_ATTACK02, L"Data\\Sound\\w69w70w71w72\\3Cato_attack2.wav");
        LoadWaveFile(SOUND_EMPIREGUARDIAN_3CORP_CATO_MOVE, L"Data\\Sound\\w69w70w71w72\\3Cato_move.wav");
        LoadWaveFile(SOUND_EMPIREGUARDIAN_4CORP_GALLIA_ATTACK02, L"Data\\Sound\\w69w70w71w72\\4Gallia_attack2.wav");
        LoadWaveFile(SOUND_EMPIREGUARDIAN_QUATERMASTER_ATTACK02, L"Data\\Sound\\w69w70w71w72\\QuaterMaster_attack2.wav");
        LoadWaveFile(SOUND_EMPIREGUARDIAN_COMBATMASTER_ATTACK01, L"Data\\Sound\\w69w70w71w72\\CombatMaster_attack1.wav");
        LoadWaveFile(SOUND_EMPIREGUARDIAN_COMBATMASTER_ATTACK02, L"Data\\Sound\\w69w70w71w72\\CombatMaster_attack2.wav");
        LoadWaveFile(SOUND_EMPIREGUARDIAN_COMBATMASTER_ATTACK03, L"Data\\Sound\\w69w70w71w72\\CombatMaster_attack3.wav");
        LoadWaveFile(SOUND_EMPIREGUARDIAN_GRANDWIZARD_DEATH, L"Data\\Sound\\w69w70w71w72\\GrandWizard_death.wav");
        LoadWaveFile(SOUND_EMPIREGUARDIAN_ASSASSINMASTER_DEATH, L"Data\\Sound\\w69w70w71w72\\AssassinMaster_Death.wav");
        LoadWaveFile(SOUND_EMPIREGUARDIAN_CAVALRYLEADER_ATTACK01, L"Data\\Sound\\w69w70w71w72\\CavalryLeader_attack1.wav");
        LoadWaveFile(SOUND_EMPIREGUARDIAN_CAVALRYLEADER_ATTACK02, L"Data\\Sound\\w69w70w71w72\\CavalryLeader_attack2.wav");
        LoadWaveFile(SOUND_EMPIREGUARDIAN_CAVALRYLEADER_MOVE01, L"Data\\Sound\\w69w70w71w72\\CavalryLeader_move01.wav");
        LoadWaveFile(SOUND_EMPIREGUARDIAN_CAVALRYLEADER_MOVE02, L"Data\\Sound\\w69w70w71w72\\CavalryLeader_move02.wav");
        LoadWaveFile(SOUND_EMPIREGUARDIAN_DEFENDER_ATTACK02, L"Data\\Sound\\w69w70w71w72\\GrandWizard_death.wav");
        LoadWaveFile(SOUND_EMPIREGUARDIAN_PRIEST_STOP, L"Data\\Sound\\w69w70w71w72\\Priest_stay.wav");
        break;
    case WD_73NEW_LOGIN_SCENE:
    case WD_74NEW_CHARACTER_SCENE:
    {
        LoadBitmap(L"Logo\\MU-logo.tga", BITMAP_LOG_IN + 16, GL_LINEAR);
        LoadBitmap(L"Logo\\MU-logo_g.jpg", BITMAP_LOG_IN + 17, GL_LINEAR);
    }
    break;
    case WD_55LOGINSCENE:
        gLoadData.AccessModel(MODEL_DRAGON, L"Data\\Object56\\", L"Dragon");
        gLoadData.OpenTexture(MODEL_DRAGON, L"Object56\\");

        Vector(0.f, 0.f, 0.f, Ang);
        Vector(0.f, 0.f, 0.f, Pos);
        Pos[0] = 56 * TERRAIN_SCALE; Pos[1] = 230 * TERRAIN_SCALE;
        CreateObject(MODEL_DRAGON, Pos, Ang);
        break;

#ifdef ASG_ADD_KARUTAN_MONSTERS
    case WD_80KARUTAN1:
    case WD_81KARUTAN2:
        LoadBitmap(L"Object9\\sand02.jpg", BITMAP_CHROME + 3, GL_LINEAR, GL_REPEAT);
        LoadBitmap(L"Effect\\clouds.jpg", BITMAP_CLOUD, GL_LINEAR, GL_CLAMP_TO_EDGE);

        gLoadData.AccessModel(MODEL_CONDRA_ARM_L, L"Data\\Monster\\", L"condra_7_arm_left");
        gLoadData.OpenTexture(MODEL_CONDRA_ARM_L, L"Monster\\");
        gLoadData.AccessModel(MODEL_CONDRA_ARM_L2, L"Data\\Monster\\", L"condra_7_arm_left_2");
        gLoadData.OpenTexture(MODEL_CONDRA_ARM_L2, L"Monster\\");
        gLoadData.AccessModel(MODEL_CONDRA_SHOULDER, L"Data\\Monster\\", L"condra_7_shoulder_right");
        gLoadData.OpenTexture(MODEL_CONDRA_SHOULDER, L"Monster\\");
        gLoadData.AccessModel(MODEL_CONDRA_ARM_R, L"Data\\Monster\\", L"condra_7_arm_right");
        gLoadData.OpenTexture(MODEL_CONDRA_ARM_R, L"Monster\\");
        gLoadData.AccessModel(MODEL_CONDRA_ARM_R2, L"Data\\Monster\\", L"condra_7_arm_right_2");
        gLoadData.OpenTexture(MODEL_CONDRA_ARM_R2, L"Monster\\");
        gLoadData.AccessModel(MODEL_CONDRA_CONE_L, L"Data\\Monster\\", L"condra_7_cone_left");
        gLoadData.OpenTexture(MODEL_CONDRA_CONE_L, L"Monster\\");
        gLoadData.AccessModel(MODEL_CONDRA_CONE_R, L"Data\\Monster\\", L"condra_7_cone_right");
        gLoadData.OpenTexture(MODEL_CONDRA_CONE_R, L"Monster\\");
        gLoadData.AccessModel(MODEL_CONDRA_PELVIS, L"Data\\Monster\\", L"condra_7_pelvis");
        gLoadData.OpenTexture(MODEL_CONDRA_PELVIS, L"Monster\\");
        gLoadData.AccessModel(MODEL_CONDRA_STOMACH, L"Data\\Monster\\", L"condra_7_stomach");
        gLoadData.OpenTexture(MODEL_CONDRA_STOMACH, L"Monster\\");
        gLoadData.AccessModel(MODEL_CONDRA_NECK, L"Data\\Monster\\", L"condra_7_neck");
        gLoadData.OpenTexture(MODEL_CONDRA_NECK, L"Monster\\");
        gLoadData.AccessModel(MODEL_NARCONDRA_ARM_L, L"Data\\Monster\\", L"nar_condra_7_arm_left");
        gLoadData.OpenTexture(MODEL_NARCONDRA_ARM_L, L"Monster\\");
        gLoadData.AccessModel(MODEL_NARCONDRA_ARM_L2, L"Data\\Monster\\", L"nar_condra_7_arm_left_2");
        gLoadData.OpenTexture(MODEL_NARCONDRA_ARM_L2, L"Monster\\");
        gLoadData.AccessModel(MODEL_NARCONDRA_SHOULDER_L, L"Data\\Monster\\", L"nar_condra_7_shoulder_left");
        gLoadData.OpenTexture(MODEL_NARCONDRA_SHOULDER_L, L"Monster\\");
        gLoadData.AccessModel(MODEL_NARCONDRA_SHOULDER_R, L"Data\\Monster\\", L"nar_condra_7_shoulder_right");
        gLoadData.OpenTexture(MODEL_NARCONDRA_SHOULDER_R, L"Monster\\");
        gLoadData.AccessModel(MODEL_NARCONDRA_ARM_R, L"Data\\Monster\\", L"nar_condra_7_arm_right");
        gLoadData.OpenTexture(MODEL_NARCONDRA_ARM_R, L"Monster\\");
        gLoadData.AccessModel(MODEL_NARCONDRA_ARM_R2, L"Data\\Monster\\", L"nar_condra_7_arm_right_2");
        gLoadData.OpenTexture(MODEL_NARCONDRA_ARM_R2, L"Monster\\");
        gLoadData.AccessModel(MODEL_NARCONDRA_ARM_R3, L"Data\\Monster\\", L"nar_condra_7_arm_right_3");
        gLoadData.OpenTexture(MODEL_NARCONDRA_ARM_R3, L"Monster\\");
        gLoadData.AccessModel(MODEL_NARCONDRA_CONE_1, L"Data\\Monster\\", L"nar_condra_7_cone_1");
        gLoadData.OpenTexture(MODEL_NARCONDRA_CONE_1, L"Monster\\");
        gLoadData.AccessModel(MODEL_NARCONDRA_CONE_2, L"Data\\Monster\\", L"nar_condra_7_cone_2");
        gLoadData.OpenTexture(MODEL_NARCONDRA_CONE_2, L"Monster\\");
        gLoadData.AccessModel(MODEL_NARCONDRA_CONE_3, L"Data\\Monster\\", L"nar_condra_7_cone_3");
        gLoadData.OpenTexture(MODEL_NARCONDRA_CONE_3, L"Monster\\");
        gLoadData.AccessModel(MODEL_NARCONDRA_CONE_4, L"Data\\Monster\\", L"nar_condra_7_cone_4");
        gLoadData.OpenTexture(MODEL_NARCONDRA_CONE_4, L"Monster\\");
        gLoadData.AccessModel(MODEL_NARCONDRA_CONE_5, L"Data\\Monster\\", L"nar_condra_7_cone_5");
        gLoadData.OpenTexture(MODEL_NARCONDRA_CONE_5, L"Monster\\");
        gLoadData.AccessModel(MODEL_NARCONDRA_CONE_6, L"Data\\Monster\\", L"nar_condra_7_cone_6");
        gLoadData.OpenTexture(MODEL_NARCONDRA_CONE_6, L"Monster\\");
        gLoadData.AccessModel(MODEL_NARCONDRA_PELVIS, L"Data\\Monster\\", L"nar_condra_7_pelvis");
        gLoadData.OpenTexture(MODEL_NARCONDRA_PELVIS, L"Monster\\");
        gLoadData.AccessModel(MODEL_NARCONDRA_STOMACH, L"Data\\Monster\\", L"nar_condra_7_stomach");
        gLoadData.OpenTexture(MODEL_NARCONDRA_STOMACH, L"Monster\\");
        gLoadData.AccessModel(MODEL_NARCONDRA_NECK, L"Data\\Monster\\", L"nar_condra_7_neck");
        gLoadData.OpenTexture(MODEL_NARCONDRA_NECK, L"Monster\\");

        LoadWaveFile(SOUND_KARUTAN_DESERT_ENV, L"Data\\Sound\\Karutan\\Karutan_desert_env.wav", 1);
        LoadWaveFile(SOUND_KARUTAN_INSECT_ENV, L"Data\\Sound\\Karutan\\Karutan_insect_env.wav", 1);
        LoadWaveFile(SOUND_KARUTAN_KARDAMAHAL_ENV, L"Data\\Sound\\Karutan\\Kardamahal_entrance_env.wav", 1);
        break;
#endif	// ASG_ADD_KARUTAN_MONSTERS
    }

    if (this->InChaosCastle() == true)
    {
        gLoadData.AccessModel(MODEL_ANGEL, L"Data\\Player\\", L"Angel");
        gLoadData.OpenTexture(MODEL_ANGEL, L"Npc\\");

        LoadBitmap(L"Effect\\clouds.jpg", BITMAP_CLOUD, GL_LINEAR, GL_CLAMP_TO_EDGE);

        LoadWaveFile(SOUND_CHAOSCASTLE, L"Data\\Sound\\iChaosCastle.wav", 1);
    }

    if (this->InHellas())
    {
        LoadBitmap(L"Object25\\water1.tga", BITMAP_MAPTILE, GL_LINEAR, GL_REPEAT, false);
        LoadBitmap(L"Object25\\water2.jpg", BITMAP_MAPTILE + 1, GL_NEAREST, GL_CLAMP_TO_EDGE);

        gLoadData.AccessModel(MODEL_CUNDUN_PART1, L"Data\\Monster\\", L"cd71a", -1);
        gLoadData.OpenTexture(MODEL_CUNDUN_PART1, L"Monster\\");
        gLoadData.AccessModel(MODEL_CUNDUN_PART2, L"Data\\Monster\\", L"cd71b", -1);
        gLoadData.OpenTexture(MODEL_CUNDUN_PART2, L"Monster\\");
        gLoadData.AccessModel(MODEL_CUNDUN_PART3, L"Data\\Monster\\", L"cd71c", -1);
        gLoadData.OpenTexture(MODEL_CUNDUN_PART3, L"Monster\\");
        gLoadData.AccessModel(MODEL_CUNDUN_PART4, L"Data\\Monster\\", L"cd71d", -1);
        gLoadData.OpenTexture(MODEL_CUNDUN_PART4, L"Monster\\");
        gLoadData.AccessModel(MODEL_CUNDUN_PART5, L"Data\\Monster\\", L"cd71e", -1);
        gLoadData.OpenTexture(MODEL_CUNDUN_PART5, L"Monster\\");
        gLoadData.AccessModel(MODEL_CUNDUN_PART6, L"Data\\Monster\\", L"cd71f", -1);
        gLoadData.OpenTexture(MODEL_CUNDUN_PART6, L"Monster\\");
        gLoadData.AccessModel(MODEL_CUNDUN_PART7, L"Data\\Monster\\", L"cd71g", -1);
        gLoadData.OpenTexture(MODEL_CUNDUN_PART7, L"Monster\\");
        gLoadData.AccessModel(MODEL_CUNDUN_PART8, L"Data\\Monster\\", L"cd71h", -1);
        gLoadData.OpenTexture(MODEL_CUNDUN_PART8, L"Monster\\");
        gLoadData.AccessModel(MODEL_CUNDUN_DRAGON_HEAD, L"Data\\Skill\\", L"dragonhead", -1);
        gLoadData.OpenTexture(MODEL_CUNDUN_DRAGON_HEAD, L"Skill\\");
        gLoadData.AccessModel(MODEL_CUNDUN_PHOENIX, L"Data\\Skill\\", L"phoenix", -1);
        gLoadData.OpenTexture(MODEL_CUNDUN_PHOENIX, L"Skill\\");
        gLoadData.AccessModel(MODEL_CUNDUN_GHOST, L"Data\\Monster\\", L"cundun_gone", -1);
        gLoadData.OpenTexture(MODEL_CUNDUN_GHOST, L"Monster\\");
        Models[MODEL_CUNDUN_PART6].Actions[0].Loop = false;
        Models[MODEL_CUNDUN_PART6].Actions[0].PlaySpeed = 0.13f;
        Models[MODEL_CUNDUN_PART7].Actions[0].Loop = false;
        Models[MODEL_CUNDUN_PART7].Actions[0].PlaySpeed = 0.13f;
        Models[MODEL_CUNDUN_PHOENIX].Actions[0].Loop = true;
        Models[MODEL_CUNDUN_GHOST].Actions[0].Loop = false;

        LoadWaveFile(SOUND_KALIMA_AMBIENT, L"Data\\Sound\\aKalima.wav", 1);
        LoadWaveFile(SOUND_KALIMA_AMBIENT2, L"Data\\Sound\\aKalima01.wav", 1);
        LoadWaveFile(SOUND_KALIMA_AMBIENT3, L"Data\\Sound\\aKalima02.wav", 1);
        LoadWaveFile(SOUND_KALIMA_WATER_FALL, L"Data\\Sound\\aKalimaWaterFall.wav", 3);
        LoadWaveFile(SOUND_KALIMA_FALLING_STONE, L"Data\\Sound\\aKalimaStone.wav", 3);
        LoadWaveFile(SOUND_DEATH_BUBBLE, L"Data\\Sound\\mDeathBubble.wav", 1);

        LoadWaveFile(SOUND_KUNDUN_AMBIENT1, L"Data\\Sound\\mKundunAmbient1.wav", 1);
        LoadWaveFile(SOUND_KUNDUN_AMBIENT2, L"Data\\Sound\\mKundunAmbient2.wav", 1);
        LoadWaveFile(SOUND_KUNDUN_ROAR, L"Data\\Sound\\mKundunRoar.wav", 1);
        LoadWaveFile(SOUND_KUNDUN_SIGHT, L"Data\\Sound\\mKundunSight.wav", 1);
        LoadWaveFile(SOUND_KUNDUN_SHUDDER, L"Data\\Sound\\mKundunShudder.wav", 1);
        LoadWaveFile(SOUND_KUNDUN_DESTROY, L"Data\\Sound\\mKundunDestory.wav", 1);

        LoadWaveFile(SOUND_SKILL_SKULL, L"Data\\Sound\\eSkull.wav", 1);
        LoadWaveFile(SOUND_GREAT_POISON, L"Data\\Sound\\eGreatPoison.wav", 1);
        LoadWaveFile(SOUND_GREAT_SHIELD, L"Data\\Sound\\eGreatShield.wav", 1);
    }

    if (this->WorldActive != WD_7ATLANSE)
    {
        for (int i = 0; i < 32; i++)
        {
            wchar_t FileName[100];
            if (i < 10)
                swprintf(FileName, L"Object8\\wt0%d.jpg", i);
            else
                swprintf(FileName, L"Object8\\wt%d.jpg", i);

            LoadBitmap(FileName, BITMAP_WATER + i, GL_LINEAR, GL_REPEAT, false);
        }
    }

    LoadBitmap(L"Object8\\light01.jpg", BITMAP_SHINES, GL_LINEAR, GL_REPEAT);

    if (this->WorldActive == 0)
    {
        for (i = 0; i < 13; i++)
            gLoadData.AccessModel(MODEL_TREE01 + i, L"Data\\Object1\\", L"Tree", i + 1);
        for (i = 0; i < 8; i++)
            gLoadData.AccessModel(MODEL_GRASS01 + i, L"Data\\Object1\\", L"Grass", i + 1);
        for (i = 0; i < 5; i++)
            gLoadData.AccessModel(MODEL_STONE01 + i, L"Data\\Object1\\", L"Stone", i + 1);

        for (i = 0; i < 3; i++)
            gLoadData.AccessModel(MODEL_STONE_STATUE01 + i, L"Data\\Object1\\", L"StoneStatue", i + 1);
        gLoadData.AccessModel(MODEL_STEEL_STATUE, L"Data\\Object1\\", L"SteelStatue", 1);
        for (i = 0; i < 3; i++)
            gLoadData.AccessModel(MODEL_TOMB01 + i, L"Data\\Object1\\", L"Tomb", i + 1);
        for (i = 0; i < 2; i++)
            gLoadData.AccessModel(MODEL_FIRE_LIGHT01 + i, L"Data\\Object1\\", L"FireLight", i + 1);
        gLoadData.AccessModel(MODEL_BONFIRE, L"Data\\Object1\\", L"Bonfire", 1);
        gLoadData.AccessModel(MODEL_DUNGEON_GATE, L"Data\\Object1\\", L"DoungeonGate", 1);
        gLoadData.AccessModel(MODEL_TREASURE_DRUM, L"Data\\Object1\\", L"TreasureDrum", 1);
        gLoadData.AccessModel(MODEL_TREASURE_CHEST, L"Data\\Object1\\", L"TreasureChest", 1);
        gLoadData.AccessModel(MODEL_SHIP, L"Data\\Object1\\", L"Ship", 1);

        for (i = 0; i < 6; i++)
            gLoadData.AccessModel(MODEL_STONE_WALL01 + i, L"Data\\Object1\\", L"StoneWall", i + 1);
        for (i = 0; i < 4; i++)
            gLoadData.AccessModel(MODEL_MU_WALL01 + i, L"Data\\Object1\\", L"StoneMuWall", i + 1);
        for (i = 0; i < 3; i++)
            gLoadData.AccessModel(MODEL_STEEL_WALL01 + i, L"Data\\Object1\\", L"SteelWall", i + 1);
        gLoadData.AccessModel(MODEL_STEEL_DOOR, L"Data\\Object1\\", L"SteelDoor", 1);
        for (i = 0; i < 3; i++)
            gLoadData.AccessModel(MODEL_CANNON01 + i, L"Data\\Object1\\", L"Cannon", i + 1);
        gLoadData.AccessModel(MODEL_BRIDGE, L"Data\\Object1\\", L"Bridge", 1);
        for (i = 0; i < 4; i++)
            gLoadData.AccessModel(MODEL_FENCE01 + i, L"Data\\Object1\\", L"Fence", i + 1);
        gLoadData.AccessModel(MODEL_BRIDGE_STONE, L"Data\\Object1\\", L"BridgeStone", 1);

        gLoadData.AccessModel(MODEL_STREET_LIGHT, L"Data\\Object1\\", L"StreetLight", 1);
        gLoadData.AccessModel(MODEL_CURTAIN, L"Data\\Object1\\", L"Curtain", 1);
        for (i = 0; i < 4; i++)
            gLoadData.AccessModel(MODEL_CARRIAGE01 + i, L"Data\\Object1\\", L"Carriage", i + 1);
        for (i = 0; i < 2; i++)
            gLoadData.AccessModel(MODEL_STRAW01 + i, L"Data\\Object1\\", L"Straw", i + 1);
        for (i = 0; i < 2; i++)
            gLoadData.AccessModel(MODEL_SIGN01 + i, L"Data\\Object1\\", L"Sign", i + 1);
        for (i = 0; i < 2; i++)
            gLoadData.AccessModel(MODEL_MERCHANT_ANIMAL01 + i, L"Data\\Object1\\", L"MerchantAnimal", i + 1);
        gLoadData.AccessModel(MODEL_WATERSPOUT, L"Data\\Object1\\", L"Waterspout", 1);
        for (i = 0; i < 4; i++)
            gLoadData.AccessModel(MODEL_WELL01 + i, L"Data\\Object1\\", L"Well", i + 1);
        gLoadData.AccessModel(MODEL_HANGING, L"Data\\Object1\\", L"Hanging", 1);

        for (i = 0; i < 5; i++)
            gLoadData.AccessModel(MODEL_HOUSE01 + i, L"Data\\Object1\\", L"House", i + 1);
        gLoadData.AccessModel(MODEL_TENT, L"Data\\Object1\\", L"Tent", 1);
        gLoadData.AccessModel(MODEL_STAIR, L"Data\\Object1\\", L"Stair", 1);

        for (i = 0; i < 6; i++)
            gLoadData.AccessModel(MODEL_HOUSE_WALL01 + i, L"Data\\Object1\\", L"HouseWall", i + 1);
        for (i = 0; i < 3; i++)
            gLoadData.AccessModel(MODEL_HOUSE_ETC01 + i, L"Data\\Object1\\", L"HouseEtc", i + 1);
        for (i = 0; i < 3; i++)
            gLoadData.AccessModel(MODEL_LIGHT01 + i, L"Data\\Object1\\", L"Light", i + 1);
        gLoadData.AccessModel(MODEL_POSE_BOX, L"Data\\Object1\\", L"PoseBox", 1);

        for (i = 0; i < 7; i++)
            gLoadData.AccessModel(MODEL_FURNITURE01 + i, L"Data\\Object1\\", L"Furniture", i + 1);
        gLoadData.AccessModel(MODEL_CANDLE, L"Data\\Object1\\", L"Candle", 1);
        for (i = 0; i < 3; i++)
            gLoadData.AccessModel(MODEL_BEER01 + i, L"Data\\Object1\\", L"Beer", i + 1);

        for (i = MODEL_WORLD_OBJECT; i < MAX_WORLD_OBJECTS; i++)
            gLoadData.OpenTexture(i, L"Object1\\");
    }
    else
    {
        wchar_t DirName[32];
        int iMapWorld = this->WorldActive + 1;

        if (this->InBloodCastle() == true)
        {
            iMapWorld = WD_11BLOODCASTLE1 + 1;
        }
        else if (this->InChaosCastle() == true)
        {
            iMapWorld = WD_18CHAOS_CASTLE + 1;
        }
        else if (this->InHellas() == true)
        {
            iMapWorld = WD_24HELLAS + 1;
        }
        else if (gMapManager.IsCursedTemple())
        {
            iMapWorld = WD_45CURSEDTEMPLE_LV1 + 2;
        }

        swprintf(DirName, L"Data\\Object%d\\", iMapWorld);
        for (i = MODEL_WORLD_OBJECT; i < MAX_WORLD_OBJECTS; i++)
            gLoadData.AccessModel(i, DirName, L"Object", i + 1);

        swprintf(DirName, L"Object%d\\", iMapWorld);
        for (i = MODEL_WORLD_OBJECT; i < MAX_WORLD_OBJECTS; i++)
        {
            gLoadData.OpenTexture(i, DirName);
        }

        if (this->WorldActive == WD_1DUNGEON)
        {
            Models[40].Actions[1].PlaySpeed = 0.4f;
        }
        else if (this->WorldActive == WD_8TARKAN)
        {
            Models[11].StreamMesh = 0;
            Models[12].StreamMesh = 0;
            Models[13].StreamMesh = 0;
            Models[73].StreamMesh = 0;
            Models[75].StreamMesh = 0;
            Models[79].StreamMesh = 0;
        }
        if (this->WorldActive == WD_51HOME_6TH_CHAR
            )
        {
            Models[MODEL_EAGLE].Actions[0].PlaySpeed = 0.5f;
            if (this->WorldActive == WD_51HOME_6TH_CHAR)
            {
                Models[MODEL_MAP_TORNADO].Actions[0].PlaySpeed = 0.1f;
            }
        }
        else if (this->WorldActive == WD_57ICECITY)
        {
            Models[16].Actions[0].PlaySpeed = 0.8f;
            Models[16].Actions[1].PlaySpeed = 0.8f;
            Models[17].Actions[0].PlaySpeed = 0.8f;
            Models[17].Actions[1].PlaySpeed = 0.8f;
            Models[17].Actions[2].PlaySpeed = 1.f;
            Models[17].Actions[3].PlaySpeed = 1.f;

            Models[68].Actions[0].PlaySpeed = 0.05f;
        }
#ifdef ASG_ADD_MAP_KARUTAN
        else if (IsKarutanMap())
        {
            Models[66].Actions[0].PlaySpeed = 0.15f;
            Models[66].Actions[1].PlaySpeed = 0.15f;

            Models[107].Actions[0].PlaySpeed = 5.f;
        }
#endif	// ASG_ADD_MAP_KARUTAN
    }
}

void CMapManager::LoadWorld(int Map)
{
    if (Map == 32 && this->WorldActive == 32)
    {
        Map = this->WorldActive = 9;
    }

    this->DeleteObjects();
    DeleteNpcs();
    DeleteMonsters();
    if (SceneFlag != CHARACTER_SCENE)
    {
        ClearItems();
        ClearCharacters(HeroKey);
    }
    RemoveAllShopTitleExceptHero();

    g_Direction.Init();
    g_Direction.HeroFallingDownInit();
    g_Direction.DeleteMonster();
    M39Kanturu3rd::Kanturu3rdInit();
    g_Direction.m_CKanturu.m_iKanturuState = 0;
    g_Direction.m_CKanturu.m_iMayaState = 0;
    g_Direction.m_CKanturu.m_iNightmareState = 0;

    this->Load();

    wchar_t FileName[64];
    wchar_t WorldName[32];
    int  iMapWorld = this->WorldActive + 1;

    if (InBloodCastle(Map) == true)
    {
        iMapWorld = WD_11BLOODCASTLE1 + 1;
    }
    else if (gMapManager.InHellas(Map) == true)
    {
        iMapWorld = WD_24HELLAS + 1;
    }
    else if (InChaosCastle(Map) == true)
    {
        iMapWorld = WD_18CHAOS_CASTLE + 1;
    }
    else if (gMapManager.IsCursedTemple())
    {
        iMapWorld = WD_45CURSEDTEMPLE_LV1 + 2;
    }

    battleCastle::Init();

    swprintf(WorldName, L"World%d", iMapWorld);
    swprintf(FileName, L"Data\\%s\\EncTerrain%d.map", WorldName, iMapWorld);

    int iResult = OpenTerrainMapping(FileName);

    if (iMapWorld != iResult && -1 != iResult)
    {
        wchar_t Text[256];
        swprintf(Text, L"%s file corrupted.", FileName);
        g_ErrorReport.Write(Text);
        g_ErrorReport.Write(L"\r\n");
        MessageBox(g_hWnd, Text, NULL, MB_OK);
        SendMessage(g_hWnd, WM_DESTROY, 0, 0);
        return;
    }

    if (this->WorldActive == WD_73NEW_LOGIN_SCENE)
    {
        swprintf(FileName, L"Data\\%s\\CWScript%d.cws", WorldName, iMapWorld);
        CCameraMove::GetInstancePtr()->LoadCameraWalkScript(FileName);
    }

    int CryWolfState = M34CryWolf1st::IsCryWolf1stMVPStart();

    if (gMapManager.InBattleCastle())
    {
        if (battleCastle::IsBattleCastleStart())
        {
            swprintf(FileName, L"Data\\%s\\EncTerrain%d.att", WorldName, iMapWorld * 10 + 2);
        }
        else
        {
            swprintf(FileName, L"Data\\%s\\EncTerrain%d.att", WorldName, iMapWorld);
        }
    }
    else

        if (this->WorldActive == WD_34CRYWOLF_1ST)
        {
            switch (CryWolfState)
            {
            case CRYWOLF_OCCUPATION_STATE_PEACE:
                swprintf(FileName, L"Data\\%s\\EncTerrain%d.att", WorldName, iMapWorld);
                break;
            case CRYWOLF_OCCUPATION_STATE_OCCUPIED:
                swprintf(FileName, L"Data\\%s\\EncTerrain%d.att", WorldName, iMapWorld * 10 + 1);
                break;
            case CRYWOLF_OCCUPATION_STATE_WAR:
                swprintf(FileName, L"Data\\%s\\EncTerrain%d.att", WorldName, iMapWorld * 10 + 2);
                break;
            }
        }
        else if (this->WorldActive == WD_39KANTURU_3RD)
        {
            if (M39Kanturu3rd::IsSuccessBattle())
            {
                swprintf(FileName, L"Data\\%s\\EncTerrain%d.att", WorldName, iMapWorld * 10 + 1);
            }
            else
            {
                swprintf(FileName, L"Data\\%s\\EncTerrain%d.att", WorldName, iMapWorld);
            }
        }
        else
        {
            swprintf(FileName, L"Data\\%s\\EncTerrain%d.att", WorldName, iMapWorld);
        }
    iResult = OpenTerrainAttribute(FileName);
    if (iMapWorld != iResult && -1 != iResult)
    {
        wchar_t Text[256];
        swprintf(Text, L"%s file corrupted.", FileName);
        g_ErrorReport.Write(Text);
        g_ErrorReport.Write(L"\r\n");
        MessageBox(g_hWnd, Text, NULL, MB_OK);
        SendMessage(g_hWnd, WM_DESTROY, 0, 0);
        return;
    }

    swprintf(FileName, L"Data\\%s\\EncTerrain%d.obj", WorldName, iMapWorld);

    iResult = OpenObjectsEnc(FileName);
    if (iMapWorld != iResult && -1 != iResult)
    {
        wchar_t Text[256];
        swprintf(Text, L"%s file corrupted.", FileName);
        g_ErrorReport.Write(Text);
        g_ErrorReport.Write(L"\r\n");
        MessageBox(g_hWnd, Text, NULL, MB_OK);
        SendMessage(g_hWnd, WM_DESTROY, 0, 0);
        return;
    }

    swprintf(FileName, L"%s\\TerrainHeight.bmp", WorldName);
    if (IsTerrainHeightExtMap(this->WorldActive) == true)
    {
        CreateTerrain(FileName, true);
    }
    else
    {
        CreateTerrain(FileName);
    }

    if (gMapManager.InBattleCastle())
    {
        if (battleCastle::IsBattleCastleStart())
        {
            swprintf(FileName, L"%s\\TerrainLight2.jpg", WorldName);
        }
        else
        {
            swprintf(FileName, L"%s\\TerrainLight.jpg", WorldName);
        }
    }
    else
        if (this->WorldActive == WD_34CRYWOLF_1ST)
        {
            switch (CryWolfState)
            {
            case CRYWOLF_OCCUPATION_STATE_PEACE:
                swprintf(FileName, L"%s\\TerrainLight.jpg", WorldName);
                break;
            case CRYWOLF_OCCUPATION_STATE_OCCUPIED:
                swprintf(FileName, L"%s\\TerrainLight1.jpg", WorldName);
                break;
            case CRYWOLF_OCCUPATION_STATE_WAR:
                swprintf(FileName, L"%s\\TerrainLight2.jpg", WorldName);
                break;
            }
        }
        else
        {
            swprintf(FileName, L"%s\\TerrainLight.jpg", WorldName);
        }

    OpenTerrainLight(FileName);

    if (CreateWaterTerrain(this->WorldActive) == false)
    {
        swprintf(FileName, L"%s\\TileGrass01.jpg", WorldName);
        LoadBitmap(FileName, BITMAP_MAPTILE, GL_NEAREST, GL_REPEAT, false);
        swprintf(FileName, L"%s\\TileGrass02.jpg", WorldName);
        LoadBitmap(FileName, BITMAP_MAPTILE + 1, GL_NEAREST, GL_REPEAT, false);
        if (this->WorldActive == WD_51HOME_6TH_CHAR
            )
        {
            swprintf(FileName, L"%s\\AlphaTileGround01.Tga", WorldName);
            LoadBitmap(FileName, BITMAP_MAPTILE + 2, GL_NEAREST, GL_REPEAT, false);
        }
        else
        {
            swprintf(FileName, L"%s\\TileGround01.jpg", WorldName);
            LoadBitmap(FileName, BITMAP_MAPTILE + 2, GL_NEAREST, GL_REPEAT, false);
        }

        if (this->WorldActive == WD_39KANTURU_3RD)
        {
            swprintf(FileName, L"%s\\AlphaTileGround02.Tga", WorldName);
            LoadBitmap(FileName, BITMAP_MAPTILE + 3, GL_NEAREST, GL_REPEAT, false);
        }
        else
        {
            swprintf(FileName, L"%s\\TileGround02.jpg", WorldName);
            LoadBitmap(FileName, BITMAP_MAPTILE + 3, GL_NEAREST, GL_REPEAT, false);
        }
        if (gMapManager.IsCursedTemple())
        {
            swprintf(FileName, L"%s\\AlphaTileGround03.Tga", WorldName);
            LoadBitmap(FileName, BITMAP_MAPTILE + 4, GL_NEAREST, GL_REPEAT, false);
        }
        else
        {
            swprintf(FileName, L"%s\\TileGround03.jpg", WorldName);
            LoadBitmap(FileName, BITMAP_MAPTILE + 4, GL_NEAREST, GL_REPEAT, false);
        }

        swprintf(FileName, L"%s\\TileWater01.jpg", WorldName);
        LoadBitmap(FileName, BITMAP_MAPTILE + 5, GL_NEAREST, GL_REPEAT, false);
        swprintf(FileName, L"%s\\TileWood01.jpg", WorldName);
        LoadBitmap(FileName, BITMAP_MAPTILE + 6, GL_NEAREST, GL_REPEAT, false);
        swprintf(FileName, L"%s\\TileRock01.jpg", WorldName);
        LoadBitmap(FileName, BITMAP_MAPTILE + 7, GL_NEAREST, GL_REPEAT, false);
        swprintf(FileName, L"%s\\TileRock02.jpg", WorldName);
        LoadBitmap(FileName, BITMAP_MAPTILE + 8, GL_NEAREST, GL_REPEAT, false);
        swprintf(FileName, L"%s\\TileRock03.jpg", WorldName);
        LoadBitmap(FileName, BITMAP_MAPTILE + 9, GL_NEAREST, GL_REPEAT, false);

        if (this->WorldActive == WD_73NEW_LOGIN_SCENE || this->WorldActive == WD_74NEW_CHARACTER_SCENE)
        {
            swprintf(FileName, L"%s\\AlphaTile01.Tga", WorldName);
            LoadBitmap(FileName, BITMAP_MAPTILE + 10, GL_NEAREST, GL_REPEAT, false);
        }
        else
        {
            swprintf(FileName, L"%s\\TileRock04.jpg", WorldName);
            LoadBitmap(FileName, BITMAP_MAPTILE + 10, GL_NEAREST, GL_REPEAT, false);
        }

        if (IsPKField() || IsDoppelGanger2())
        {
            LoadBitmap(L"Object64\\song_lava1.jpg", BITMAP_MAPTILE + 11, GL_NEAREST, GL_REPEAT, false);
        }
        else
        {
            swprintf(FileName, L"%s\\TileRock05.jpg", WorldName);
            LoadBitmap(FileName, BITMAP_MAPTILE + 11, GL_NEAREST, GL_REPEAT, false);
        }
#ifdef ASG_ADD_MAP_KARUTAN
        if (IsKarutanMap())
        {
            swprintf(FileName, L"%s\\AlphaTile01.Tga", WorldName);
            LoadBitmap(FileName, BITMAP_MAPTILE + 12, GL_NEAREST, GL_REPEAT, false);
        }
        else
        {
#endif	// ASG_ADD_MAP_KARUTAN
            swprintf(FileName, L"%s\\TileRock06.jpg", WorldName);
            LoadBitmap(FileName, BITMAP_MAPTILE + 12, GL_NEAREST, GL_REPEAT, false);
#ifdef ASG_ADD_MAP_KARUTAN
        }
#endif	// ASG_ADD_MAP_KARUTAN
        swprintf(FileName, L"%s\\TileRock07.jpg", WorldName);
        LoadBitmap(FileName, BITMAP_MAPTILE + 13, GL_NEAREST, GL_REPEAT, false);

        for (int i = 1; i <= 16; i++)
        {
            if (i >= 10)
                swprintf(FileName, L"%s\\ExtTile%d.jpg", WorldName, i);
            else
                swprintf(FileName, L"%s\\ExtTile0%d.jpg", WorldName, i);
            LoadBitmap(FileName, BITMAP_MAPTILE + 13 + i, GL_NEAREST, GL_REPEAT, false);
        }
        if (IsPKField() || IsDoppelGanger2())
        {
            swprintf(FileName, L"%s\\TileGrass01_R.jpg", WorldName);
            LoadBitmap(FileName, BITMAP_MAPGRASS, GL_LINEAR, GL_REPEAT, false);
        }
        else
        {
            swprintf(FileName, L"%s\\TileGrass01.tga", WorldName);
            LoadBitmap(FileName, BITMAP_MAPGRASS, GL_NEAREST, GL_REPEAT, false);
        }

        swprintf(FileName, L"%s\\TileGrass02.tga", WorldName);
        LoadBitmap(FileName, BITMAP_MAPGRASS + 1, GL_NEAREST, GL_REPEAT, false);
        swprintf(FileName, L"%s\\TileGrass03.tga", WorldName);
        LoadBitmap(FileName, BITMAP_MAPGRASS + 2, GL_NEAREST, GL_REPEAT, false);

        
        swprintf(FileName, L"%s\\leaf01.tga", WorldName); 
        LoadBitmap(FileName, BITMAP_LEAF1, GL_NEAREST, GL_CLAMP_TO_EDGE, false);
        swprintf(FileName,(Map==0||Map==3||Map==63)? L"%s\\leaf01.tga": L"%s\\leaf01.jpg",WorldName);
        LoadBitmap(FileName, BITMAP_LEAF1, GL_NEAREST, GL_CLAMP_TO_EDGE, false);
        swprintf(FileName, L"%s\\leaf02.jpg", WorldName);
        LoadBitmap(FileName, BITMAP_LEAF2, GL_NEAREST, GL_CLAMP_TO_EDGE, false);

        if (M34CryWolf1st::IsCyrWolf1st() == true)
        {
            swprintf(FileName, L"%s\\rain011.tga", L"World1");
            LoadBitmap(FileName, BITMAP_RAIN, GL_NEAREST, GL_CLAMP_TO_EDGE, false);
        }
        else
        {
            swprintf(FileName, L"%s\\rain01.tga", L"World1");
            LoadBitmap(FileName, BITMAP_RAIN, GL_NEAREST, GL_CLAMP_TO_EDGE, false);
        }
        swprintf(FileName, L"%s\\rain02.tga", L"World1");
        LoadBitmap(FileName, BITMAP_RAIN_CIRCLE, GL_NEAREST, GL_CLAMP_TO_EDGE, false);
        swprintf(FileName, L"%s\\rain03.tga", L"World10");
        LoadBitmap(FileName, BITMAP_RAIN_CIRCLE + 1, GL_NEAREST, GL_CLAMP_TO_EDGE, false);

        if (IsEmpireGuardian1() || IsEmpireGuardian2() || IsEmpireGuardian3() || IsEmpireGuardian4())
        {
            swprintf(FileName, L"%s\\AlphaTile01.Tga", WorldName);
            LoadBitmap(FileName, BITMAP_MAPTILE + 10, GL_NEAREST, GL_REPEAT, false);
        }
    }

    if (iMapWorld != 74 && iMapWorld != 75)
    {
        g_pNewUIMiniMap->UnloadImages();
        g_pNewUIMiniMap->LoadImages(WorldName);
    }
}

void CMapManager::DeleteObjects()
{
    for (int i = MODEL_WORLD_OBJECT; i < MAX_WORLD_OBJECTS; i++)
    {
        Models[i].Release();
    }

    for (int i = 0; i < 16; i++)
    {
        for (int j = 0; j < 16; j++)
        {
            OBJECT_BLOCK* ob = &ObjectBlock[i * 16 + j];
            OBJECT* o = ob->Tail;
            while (1)
            {
                if (o != NULL)
                {
                    OBJECT* Temp = o->Prior;
                    DeleteObject(o, ob);
                    if (Temp == NULL) break;
                    o = Temp;
                }
                else break;
            }
            ob->Head = NULL;
            ob->Tail = NULL;
        }
    }

    for (int i = BITMAP_MAPTILE; i <= BITMAP_RAIN_CIRCLE; i++)
    {
        DeleteBitmap(i);
    }

    for (int i = 0; i < MAX_SPRITES; i++)
        Sprites[i].Live = false;
    for (int i = 0; i < MAX_BOIDS; i++)
        Boids[i].Live = false;
    for (int i = 0; i < MAX_FISHS; i++)
        Fishs[i].Live = false;
    for (int i = 0; i < MAX_LEAVES; i++)
        Leaves[i].Live = false;
    for (int i = 0; i < MAX_PARTICLES; i++)
        Particles[i].Live = false;
    for (int i = 0; i < MAX_POINTS; i++)
        Points[i].Live = false;
    for (int i = 0; i < MAX_JOINTS; i++)
        Joints[i].Live = false;
    for (int i = 0; i < MAX_OPERATES; i++)
        Operates[i].Live = false;
    for (int i = 0; i < MAX_EFFECTS; i++)
        Effects[i].Live = false;
}

bool CMapManager::InChaosCastle(int iMap)
{
    if (iMap == -1)
    {
        iMap = this->WorldActive;
    }

    if ((iMap >= WD_18CHAOS_CASTLE && iMap <= WD_18CHAOS_CASTLE_END) || iMap == WD_53CAOSCASTLE_MASTER_LEVEL)
    {
        return true;
    }

    return false;
}

bool CMapManager::InBloodCastle(int iMap)
{
    if (iMap == -1)
    {
        iMap = this->WorldActive;
    }

    if ((iMap >= WD_11BLOODCASTLE1 && iMap <= WD_11BLOODCASTLE_END) || iMap == WD_52BLOODCASTLE_MASTER_LEVEL)
    {
        return true;
    }

    return false;
}

bool CMapManager::InDevilSquare()
{
    return WD_9DEVILSQUARE == this->WorldActive ? true : false;
}

bool CMapManager::InHellas(int iMap)
{
    if (iMap == -1)
    {
        iMap = this->WorldActive;
    }

    return ((iMap >= WD_24HELLAS && iMap <= WD_24HELLAS_END)
        || (iMap == WD_24HELLAS_7));
}

bool CMapManager::InHiddenHellas(int iMap)
{
    if (iMap == -1)
    {
        iMap = this->WorldActive;
    }

    return iMap == WD_24HELLAS_7;
}

bool CMapManager::IsPKField()
{
    return (this->WorldActive == WD_63PK_FIELD) ? true : false;
}

bool CMapManager::IsCursedTemple()
{
    return (this->WorldActive >= WD_45CURSEDTEMPLE_LV1 && this->WorldActive <= WD_45CURSEDTEMPLE_LV6) ? true : false;
}

bool CMapManager::IsEmpireGuardian1()
{
    return (this->WorldActive == WD_69EMPIREGUARDIAN1) ? true : false;
}

bool CMapManager::IsEmpireGuardian2()
{
    return (this->WorldActive == WD_70EMPIREGUARDIAN2) ? true : false;
}

bool CMapManager::IsEmpireGuardian3()
{
    return (this->WorldActive == WD_71EMPIREGUARDIAN3) ? true : false;
}

bool CMapManager::IsEmpireGuardian4()
{
    if (this->WorldActive == WD_72EMPIREGUARDIAN4 || this->WorldActive == WD_73NEW_LOGIN_SCENE || this->WorldActive == WD_74NEW_CHARACTER_SCENE)
    {
        return true;
    }
    return false;
}

bool CMapManager::IsEmpireGuardian()
{
    if (this->IsEmpireGuardian1() == false &&
        this->IsEmpireGuardian2() == false &&
        this->IsEmpireGuardian3() == false &&
        this->IsEmpireGuardian4() == false)
    {
        return false;
    }

    return true;
}

bool CMapManager::InBattleCastle(int iMap)
{
    if (iMap == -1)
    {
        iMap = this->WorldActive;
    }

    return (iMap == WD_30BATTLECASTLE) ? true : false;
}

const wchar_t* CMapManager::GetMapName(int iMap)
{
    if (iMap == WD_34CRYWOLF_1ST)
    {
        return(GlobalText[1851]);
    }
    else if (iMap == WD_33AIDA)
    {
        return(GlobalText[1850]);
    }
    else if (iMap == WD_37KANTURU_1ST)
    {
        return(GlobalText[2177]);
    }
    else if (iMap == WD_38KANTURU_2ND)
    {
        return(GlobalText[2178]);
    }
    else if (iMap == WD_39KANTURU_3RD)
    {
        return(GlobalText[2179]);
    }
    else if (iMap == WD_40AREA_FOR_GM)
    {
        return(GlobalText[2324]);
    }
    else if (iMap == WD_51HOME_6TH_CHAR)
    {
        return(GlobalText[1853]);
    }
    else if (iMap == WD_57ICECITY)
    {
        return(GlobalText[1855]);
    }
    else if (iMap == WD_58ICECITY_BOSS)
    {
        return(GlobalText[1856]);
    }

    if (gMapManager.InBattleCastle(iMap) == true)
    {
        return (GlobalText[669]);
    }
    if (iMap == WD_31HUNTING_GROUND)
    {
        return (GlobalText[59]);
    }
    if (InChaosCastle(iMap) == true)
    {
        return (GlobalText[57]);
    }
    if (InHellas(iMap) == true)
    {
        if (InHiddenHellas(iMap) == true)
            return (GlobalText[1852]);
        return (GlobalText[58]);
    }
    if (InBloodCastle(iMap) == true)
    {
        return (GlobalText[56]);
    }
    if (iMap == WD_10HEAVEN)
    {
        return (GlobalText[55 + iMap - WD_10HEAVEN]);
    }
    if (iMap == 32)
    {
        return (GlobalText[39]);
    }
    if (SEASON3A::CGM3rdChangeUp::Instance().IsBalgasBarrackMap())
        return GlobalText[1678];
    else if (SEASON3A::CGM3rdChangeUp::Instance().IsBalgasRefugeMap())
        return GlobalText[1679];
    if (this->IsCursedTemple())
    {
        return (GlobalText[2369]);
    }
    if (iMap == WD_51HOME_6TH_CHAR)
    {
        return (GlobalText[1853]);
    }
    if (iMap == WD_56MAP_SWAMP_OF_QUIET)
    {
        return (GlobalText[1854]);
    }
    if (iMap == WD_62SANTA_TOWN)
    {
        return (GlobalText[2611]);
    }
    if (iMap == WD_64DUELARENA)
    {
        return (GlobalText[2703]);
    }
    if (iMap == WD_63PK_FIELD)
    {
        return (GlobalText[2686]);
    }
    if (iMap == WD_65DOPPLEGANGER1)
    {
        return (GlobalText[3057]);
    }
    if (iMap == WD_66DOPPLEGANGER2)
    {
        return (GlobalText[3057]);
    }
    if (iMap == WD_67DOPPLEGANGER3)
    {
        return (GlobalText[3057]);
    }
    if (iMap == WD_68DOPPLEGANGER4)
    {
        return (GlobalText[3057]);
    }
    if (iMap == WD_69EMPIREGUARDIAN1)
    {
        return (GlobalText[2806]);
    }
    if (iMap == WD_70EMPIREGUARDIAN2)
    {
        return (GlobalText[2806]);
    }
    if (iMap == WD_71EMPIREGUARDIAN3)
    {
        return (GlobalText[2806]);
    }
    if (iMap == WD_72EMPIREGUARDIAN4)
    {
        return (GlobalText[2806]);
    }
    if (iMap == WD_79UNITEDMARKETPLACE)
    {
        return (GlobalText[3017]);
    }
    if (iMap == WD_80KARUTAN1 || iMap == WD_81KARUTAN2)
    {
        return (GlobalText[3285]);
    }
    return (GlobalText[30 + iMap]);
}
