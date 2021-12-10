// MapManager.cpp: implementation of the CMapManager class.
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MapManager.h"
#include "CameraMove.h"
#include "CDirection.h"
#include "GMBattleCastle.h"
#include "GMHellas.h"
#include "GM_Kanturu_3rd.h"
#include "DSPlaySound.h"
#include "GlobalBitmap.h"
#include "GM_Kanturu_2nd.h"
#include "GM3rdChangeUp.h"
#include "GMCryWolf1st.h"
#include "GMDoppelGanger2.h"
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
#include "./Utilities/Log/ErrorReport.h"

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
	
	LoadBitmap("Object8\\drop01.jpg"     ,BITMAP_BUBBLE);
    
	switch(gMapManager.WorldActive)
	{
	case WD_0LORENCIA:
		gLoadData.AccessModel(MODEL_BIRD01		 ,"Data\\Object1\\","Bird",1);
		gLoadData.OpenTexture(MODEL_BIRD01		 ,"Object1\\");
		gLoadData.AccessModel(MODEL_FISH01		 ,"Data\\Object1\\","Fish",1);
		gLoadData.OpenTexture(MODEL_FISH01		 ,"Object1\\");
		break;
	case WD_1DUNGEON:
	case WD_4LOSTTOWER:
		gLoadData.AccessModel(MODEL_DUNGEON_STONE01,"Data\\Object2\\","DungeonStone",1);
		gLoadData.OpenTexture(MODEL_DUNGEON_STONE01,"Object2\\");
		gLoadData.AccessModel(MODEL_BAT01          ,"Data\\Object2\\","Bat",1);
		gLoadData.OpenTexture(MODEL_BAT01		     ,"Object2\\");
		gLoadData.AccessModel(MODEL_RAT01          ,"Data\\Object2\\","Rat",1);
		gLoadData.OpenTexture(MODEL_RAT01		     ,"Object2\\");
		break;
	case WD_2DEVIAS:
		{
			vec3_t Pos, Ang;
			gLoadData.AccessModel ( MODEL_NPC_SERBIS_DONKEY, "Data\\Npc\\", "obj_donkey" );
			gLoadData.OpenTexture ( MODEL_NPC_SERBIS_DONKEY, "Npc\\" );
			gLoadData.AccessModel ( MODEL_NPC_SERBIS_FLAG, "Data\\Npc\\", "obj_flag" );
			gLoadData.OpenTexture ( MODEL_NPC_SERBIS_FLAG, "Npc\\" );	

			Vector ( 0.f, 0.f, 0.f, Ang );
			Vector ( 0.f, 0.f, 270.f, Pos );
			Pos[0] = 191*TERRAIN_SCALE; Pos[1] = 16*TERRAIN_SCALE;
			CreateObject ( MODEL_NPC_SERBIS_DONKEY, Pos, Ang );
			Pos[0] = 191*TERRAIN_SCALE; Pos[1] = 17*TERRAIN_SCALE;
			CreateObject ( MODEL_NPC_SERBIS_FLAG, Pos, Ang );
			
			gLoadData.AccessModel(MODEL_WARP	 ,"Data\\Npc\\","warp01");
			gLoadData.AccessModel(MODEL_WARP2	 ,"Data\\Npc\\","warp02");
			gLoadData.AccessModel(MODEL_WARP3	 ,"Data\\Npc\\","warp03");
			gLoadData.OpenTexture(MODEL_WARP	 ,"Npc\\");
			gLoadData.OpenTexture(MODEL_WARP2	 ,"Npc\\");
			gLoadData.OpenTexture(MODEL_WARP3	 ,"Npc\\");
			Vector ( 0.f, 0.f, 10.f, Ang );
			Vector ( 0.f, 0.f, 0.f, Pos );
			Pos[0] = 53*TERRAIN_SCALE + 50.f; 
			Pos[1] = 92*TERRAIN_SCALE + 20.f;
			CreateObject ( MODEL_WARP, Pos, Ang );
		}
		break;
	case WD_3NORIA:
		gLoadData.AccessModel(MODEL_BUTTERFLY01	 ,"Data\\Object1\\","Butterfly",1);
		gLoadData.OpenTexture(MODEL_BUTTERFLY01	 ,"Object1\\");
		gLoadData.AccessModel(MODEL_WARP	 ,"Data\\Npc\\","warp01");
		gLoadData.AccessModel(MODEL_WARP2	 ,"Data\\Npc\\","warp02");
		gLoadData.AccessModel(MODEL_WARP3	 ,"Data\\Npc\\","warp03");
		
		gLoadData.OpenTexture(MODEL_WARP	 ,"Npc\\");
		gLoadData.OpenTexture(MODEL_WARP2	 ,"Npc\\");
		gLoadData.OpenTexture(MODEL_WARP3	 ,"Npc\\");
		
		vec3_t Pos, Ang;
		Vector ( 0.f, 0.f, 10.f, Ang );
		Vector ( 0.f, 0.f, 0.f, Pos );
		Pos[0] = 223*TERRAIN_SCALE; Pos[1] = 30*TERRAIN_SCALE;
		CreateObject ( MODEL_WARP, Pos, Ang );
		break;
	case WD_5UNKNOWN:
		for(int i=0;i<5;i++)
			gLoadData.AccessModel(MODEL_BIG_METEO1+i,"Data\\Object6\\","Meteo",i+1);

		gLoadData.AccessModel(MODEL_BOSS_HEAD,"Data\\Object6\\","BossHead",1);
		gLoadData.AccessModel(MODEL_PRINCESS ,"Data\\Object6\\","Princess",1);
		for(int i=MODEL_BIG_METEO1;i<=MODEL_PRINCESS;i++)
			gLoadData.OpenTexture(i,"Object6\\",GL_NEAREST);
		break;
	case WD_6STADIUM:
		gLoadData.AccessModel(MODEL_BUG01          ,"Data\\Object7\\","Bug",1);
		gLoadData.OpenTexture(MODEL_BUG01		     ,"Object7\\");
		break;
	case WD_7ATLANSE:
	case WD_67DOPPLEGANGER3:
		for(int i=1;i<9;i++)
		{
			gLoadData.AccessModel(MODEL_FISH01+i,"Data\\Object8\\","Fish",i+1);
			gLoadData.OpenTexture(MODEL_FISH01+i,"Object8\\");
		}

		for(int i=0;i<32;i++)
		{
			char FileName[100];
			if(i<10)
      			sprintf(FileName,"Object8\\wt0%d.jpg",i);
			else
      			sprintf(FileName,"Object8\\wt%d.jpg",i);

			LoadBitmap(FileName,BITMAP_WATER+i,GL_LINEAR,GL_REPEAT,false);
			
			if(i<10)
      			sprintf(FileName,"wt0%d.jpg",i);
			else
      			sprintf(FileName,"wt%d.jpg",i);
			strcpy(Bitmaps[BITMAP_WATER+i].FileName,FileName);
		}
		break;
    case WD_8TARKAN:
		LoadBitmap("Object9\\sand01.jpg"     ,BITMAP_CHROME+2    ,GL_LINEAR,GL_REPEAT);
		LoadBitmap("Object9\\sand02.jpg"     ,BITMAP_CHROME+3    ,GL_LINEAR,GL_REPEAT);
        LoadBitmap("Object9\\Impack03.jpg"   ,BITMAP_IMPACT      ,GL_LINEAR,GL_CLAMP_TO_EDGE);
		gLoadData.AccessModel(MODEL_BUG01+1         ,"Data\\Object9\\","Bug",2);
		gLoadData.OpenTexture(MODEL_BUG01+1		  ,"Object9\\");
        break;
    case WD_10HEAVEN:
		LoadBitmap("Effect\\clouds.jpg"	  ,BITMAP_CLOUD ,GL_LINEAR,GL_CLAMP_TO_EDGE);
		gLoadData.AccessModel(MODEL_CLOUD,"Data\\Object11\\","cloud");
		gLoadData.OpenTexture(MODEL_CLOUD,"Object11\\");

		LoadBitmap("Effect\\cloudLight.jpg" ,BITMAP_CLOUD+1, GL_LINEAR, GL_CLAMP_TO_EDGE);
        break;

    case WD_11BLOODCASTLE1:
    case WD_11BLOODCASTLE1+1:
    case WD_11BLOODCASTLE1+2:
    case WD_11BLOODCASTLE1+3:
    case WD_11BLOODCASTLE1+4:
    case WD_11BLOODCASTLE1+5:
    case WD_11BLOODCASTLE1+6:
	case WD_52BLOODCASTLE_MASTER_LEVEL:
        {
		    gLoadData.AccessModel ( MODEL_CROW, "Data\\Object12\\", "Crow", 1 );
		    gLoadData.OpenTexture ( MODEL_CROW, "Object12\\" );

            for ( int i=0; i<2; ++i )
                gLoadData.OpenTexture ( MODEL_GATE+i, "Monster\\" );
			
			LoadBitmap("Effect\\clouds.jpg"	  ,BITMAP_CLOUD ,GL_LINEAR,GL_CLAMP_TO_EDGE);
		    LoadWaveFile ( SOUND_BLOODCASTLE, "Data\\Sound\\iBloodCastle.wav" , 1 );
        }
        break;
	case WD_34CRYWOLF_1ST:
		{
			gLoadData.AccessModel(MODEL_SCOLPION          ,"Data\\Object35\\","scorpion");
			gLoadData.OpenTexture(MODEL_SCOLPION		     ,"Object35\\");

			LoadBitmap( "Effect\\Map_Smoke1.jpg" ,BITMAP_CHROME+2, GL_LINEAR, GL_REPEAT );
			LoadBitmap( "Effect\\Map_Smoke2.tga" ,BITMAP_CHROME+3, GL_LINEAR, GL_REPEAT );
			LoadBitmap("Effect\\mhoujin_R.jpg"  ,BITMAP_MAGIC_CIRCLE, GL_LINEAR, GL_CLAMP_TO_EDGE);

			gLoadData.AccessModel(MODEL_ARROW_TANKER	 ,"Data\\Monster\\","arrowstusk");
			gLoadData.OpenTexture(MODEL_ARROW_TANKER   ,"Monster\\");
			gLoadData.AccessModel(MODEL_ARROW_TANKER_HIT	 ,"Data\\Monster\\","arrowstusk");
			gLoadData.OpenTexture(MODEL_ARROW_TANKER_HIT   ,"Monster\\");

			LoadBitmap("Effect\\Impack03.jpg"			, BITMAP_EXT_LOG_IN+2);
			LoadBitmap("Logo\\chasellight.jpg"		, BITMAP_EFFECT);

			LoadWaveFile ( SOUND_CRY1ST_AMBIENT,    "Data\\Sound\\w35\\crywolf_ambi.wav", 1, true );
			LoadWaveFile ( SOUND_CRY1ST_WWOLF_MOVE1,             "Data\\Sound\\w35\\ww_idle1.wav", 1 );
			LoadWaveFile ( SOUND_CRY1ST_WWOLF_MOVE2,             "Data\\Sound\\w35\\ww_idle2.wav", 1 );
			LoadWaveFile ( SOUND_CRY1ST_WWOLF_ATTACK1,           "Data\\Sound\\w35\\ww_attack1.wav", 1 );
			LoadWaveFile ( SOUND_CRY1ST_WWOLF_ATTACK2,           "Data\\Sound\\w35\\ww_attack2.wav", 1 );
			LoadWaveFile ( SOUND_CRY1ST_WWOLF_DIE,               "Data\\Sound\\w35\\ww_death.wav", 1 );
			LoadWaveFile ( SOUND_CRY1ST_SCOUT1_MOVE1,           "Data\\Sound\\w35\\ww_s1_idle1.wav", 1 );
			LoadWaveFile ( SOUND_CRY1ST_SCOUT1_MOVE2,           "Data\\Sound\\w35\\ww_s1_idle2.wav", 1 );
			LoadWaveFile ( SOUND_CRY1ST_SCOUT1_ATTACK1,         "Data\\Sound\\w35\\ww_s1_attack1.wav", 1 );
			LoadWaveFile ( SOUND_CRY1ST_SCOUT1_ATTACK2,         "Data\\Sound\\w35\\ww_s1_attack2.wav", 1 );
			LoadWaveFile ( SOUND_CRY1ST_SCOUT1_DIE,             "Data\\Sound\\w35\\ww_s1_death.wav", 1 );
			LoadWaveFile ( SOUND_CRY1ST_SCOUT2_MOVE1,             "Data\\Sound\\w35\\ww_s2_idle1.wav", 1 );
			LoadWaveFile ( SOUND_CRY1ST_SCOUT2_MOVE2,             "Data\\Sound\\w35\\ww_s2_idle2.wav", 1 );
			LoadWaveFile ( SOUND_CRY1ST_SCOUT2_ATTACK1,           "Data\\Sound\\w35\\ww_s2_attack1.wav", 1 );
			LoadWaveFile ( SOUND_CRY1ST_SCOUT2_ATTACK2,           "Data\\Sound\\w35\\ww_s2_attack2.wav", 1 );
			LoadWaveFile ( SOUND_CRY1ST_SCOUT2_DIE,               "Data\\Sound\\w35\\ww_s2_death.wav", 1 );
			LoadWaveFile ( SOUND_CRY1ST_SCOUT3_MOVE1,             "Data\\Sound\\w35\\ww_s3_idle1.wav", 1 );
			LoadWaveFile ( SOUND_CRY1ST_SCOUT3_MOVE2,             "Data\\Sound\\w35\\ww_s3_idle2.wav", 1 );
			LoadWaveFile ( SOUND_CRY1ST_SCOUT3_ATTACK1,           "Data\\Sound\\w35\\ww_s3_attack1.wav", 1 );
			LoadWaveFile ( SOUND_CRY1ST_SCOUT3_ATTACK2,           "Data\\Sound\\w35\\ww_s3_attack2.wav", 1 );
			LoadWaveFile ( SOUND_CRY1ST_SCOUT3_DIE,               "Data\\Sound\\w35\\ww_s3_death.wav", 1 );
			LoadWaveFile ( SOUND_CRY1ST_SORAM_MOVE1,             "Data\\Sound\\w35\\soram_idle1.wav", 1 );
			LoadWaveFile ( SOUND_CRY1ST_SORAM_MOVE2,             "Data\\Sound\\w35\\soram_idle2.wav", 1 );
			LoadWaveFile ( SOUND_CRY1ST_SORAM_ATTACK1,           "Data\\Sound\\w35\\soram_attack1.wav", 1 );
			LoadWaveFile ( SOUND_CRY1ST_SORAM_ATTACK2,           "Data\\Sound\\w35\\soram_attack2.wav", 1 );
			LoadWaveFile ( SOUND_CRY1ST_SORAM_DIE,               "Data\\Sound\\w35\\soram_death.wav", 1 );
			LoadWaveFile ( SOUND_CRY1ST_BALRAM_MOVE1,             "Data\\Sound\\w35\\balram_idle1.wav", 1 );
			LoadWaveFile ( SOUND_CRY1ST_BALRAM_MOVE2,             "Data\\Sound\\w35\\balram_idle2.wav", 1 );
			LoadWaveFile ( SOUND_CRY1ST_BALRAM_ATTACK1,           "Data\\Sound\\w35\\balram_attack1.wav", 1 );
			LoadWaveFile ( SOUND_CRY1ST_BALRAM_ATTACK2,           "Data\\Sound\\w35\\balram_attack2.wav", 1 );
			LoadWaveFile ( SOUND_CRY1ST_BALRAM_DIE,               "Data\\Sound\\w35\\balram_death.wav", 1 );
			LoadWaveFile ( SOUND_CRY1ST_BALGAS_MOVE1,             "Data\\Sound\\w35\\balga_idle1.wav", 1 );
			LoadWaveFile ( SOUND_CRY1ST_BALGAS_MOVE2,             "Data\\Sound\\w35\\balga_idle2.wav", 1 );
			LoadWaveFile ( SOUND_CRY1ST_BALGAS_ATTACK1,           "Data\\Sound\\w35\\balga_at1.wav", 1 );
			LoadWaveFile ( SOUND_CRY1ST_BALGAS_ATTACK2,           "Data\\Sound\\w35\\balga_at2.wav", 1 );
			LoadWaveFile ( SOUND_CRY1ST_BALGAS_DIE,               "Data\\Sound\\w35\\balga_death.wav", 1 );
			LoadWaveFile ( SOUND_CRY1ST_BALGAS_SKILL1,            "Data\\Sound\\w35\\balga_skill1.wav", 1 );
			LoadWaveFile ( SOUND_CRY1ST_BALGAS_SKILL2,            "Data\\Sound\\w35\\balga_skill2.wav", 1 );
			LoadWaveFile ( SOUND_CRY1ST_DARKELF_MOVE1,           "Data\\Sound\\w35\\darkelf_idle1.wav", 1 );
			LoadWaveFile ( SOUND_CRY1ST_DARKELF_MOVE2,           "Data\\Sound\\w35\\darkelf_idle2.wav", 1 );
			LoadWaveFile ( SOUND_CRY1ST_DARKELF_ATTACK1,         "Data\\Sound\\w35\\darkelf_at1.wav", 1 );
			LoadWaveFile ( SOUND_CRY1ST_DARKELF_ATTACK2,         "Data\\Sound\\w35\\darkelf_at2.wav", 1 );
			LoadWaveFile ( SOUND_CRY1ST_DARKELF_DIE,             "Data\\Sound\\w35\\darkelf_death.wav", 1 );
			LoadWaveFile ( SOUND_CRY1ST_DARKELF_SKILL1,          "Data\\Sound\\w35\\darkelf_skill1.wav", 1 );
			LoadWaveFile ( SOUND_CRY1ST_DARKELF_SKILL2,          "Data\\Sound\\w35\\darkelf_skill2.wav", 1 );
			LoadWaveFile ( SOUND_CRY1ST_DEATHSPIRIT_MOVE1,		 "Data\\Sound\\w35\\dths_idle1.wav", 1 );
			LoadWaveFile ( SOUND_CRY1ST_DEATHSPIRIT_MOVE2,		 "Data\\Sound\\w35\\dths_idle2.wav", 1 );
			LoadWaveFile ( SOUND_CRY1ST_DEATHSPIRIT_ATTACK1,     "Data\\Sound\\w35\\dths_at1.wav", 1 );
			LoadWaveFile ( SOUND_CRY1ST_DEATHSPIRIT_ATTACK2,     "Data\\Sound\\w35\\dths_at2.wav", 1 );
			LoadWaveFile ( SOUND_CRY1ST_DEATHSPIRIT_DIE,         "Data\\Sound\\w35\\dths_deat.wav", 1 );
			LoadWaveFile ( SOUND_CRY1ST_TANKER_ATTACK1,           "Data\\Sound\\w35\\tanker_attack.wav", 1 );
			LoadWaveFile ( SOUND_CRY1ST_TANKER_DIE,			      "Data\\Sound\\w35\\tanker_death.wav", 1 );
			LoadWaveFile ( SOUND_CRY1ST_SUMMON,					  "Data\\Sound\\w35\\spawn_single.wav", 1 );
			LoadWaveFile ( SOUND_CRY1ST_SUCCESS,				  "Data\\Sound\\w35\\CW_win.wav", 1 );
			LoadWaveFile ( SOUND_CRY1ST_FAILED,					  "Data\\Sound\\w35\\CW_lose.wav", 1 );
		}
		break;
    case WD_30BATTLECASTLE :
		LoadBitmap( "Effect\\Map_Smoke1.jpg" ,BITMAP_CHROME+2, GL_LINEAR, GL_REPEAT );
		LoadBitmap( "Effect\\Map_Smoke2.tga" ,BITMAP_CHROME+3, GL_LINEAR, GL_REPEAT );
		LoadBitmap( "World31\\Map1.jpg" ,BITMAP_INTERFACE_MAP+1, GL_LINEAR, GL_CLAMP_TO_EDGE );
		LoadBitmap( "World31\\Map2.jpg" ,BITMAP_INTERFACE_MAP+2, GL_LINEAR, GL_CLAMP_TO_EDGE );

        LoadWaveFile ( SOUND_BC_AMBIENT        ,   "Data\\Sound\\BattleCastle\\aSiegeAmbi.wav", 1, true );
        LoadWaveFile ( SOUND_BC_AMBIENT_BATTLE1,   "Data\\Sound\\BattleCastle\\RanAmbi1.wav", 1, true );
        LoadWaveFile ( SOUND_BC_AMBIENT_BATTLE2,   "Data\\Sound\\BattleCastle\\RanAmbi2.wav", 1, true ); 
        LoadWaveFile ( SOUND_BC_AMBIENT_BATTLE3,   "Data\\Sound\\BattleCastle\\RanAmbi3.wav", 1, true );
        LoadWaveFile ( SOUND_BC_AMBIENT_BATTLE4,   "Data\\Sound\\BattleCastle\\RanAmbi4.wav", 1, true );
        LoadWaveFile ( SOUND_BC_AMBIENT_BATTLE5,   "Data\\Sound\\BattleCastle\\RanAmbi5.wav", 1, true );
        LoadWaveFile ( SOUND_BC_GUARD_STONE_DIS,   "Data\\Sound\\BattleCastle\\oGuardStoneDis.wav", 1, true );      
        LoadWaveFile ( SOUND_BC_SHIELD_SPACE_DIS,  "Data\\Sound\\BattleCastle\\oProtectionDis.wav", 1, true );       
        LoadWaveFile ( SOUND_BC_CATAPULT_ATTACK,   "Data\\Sound\\BattleCastle\\oSWFire.wav", 1, true  );   
        LoadWaveFile ( SOUND_BC_CATAPULT_HIT   ,   "Data\\Sound\\BattleCastle\\oSWHitG.wav", MAX_CHANNEL, true );
        LoadWaveFile ( SOUND_BC_WALL_HIT       ,   "Data\\Sound\\BattleCastle\\oSWHit.wav", MAX_CHANNEL, true );

        LoadWaveFile ( SOUND_BC_GATE_OPEN      ,   "Data\\Sound\\BattleCastle\\oCDoorMove.wav", 1, true  );
        LoadWaveFile ( SOUND_BC_GUARDIAN_ATTACK,   "Data\\Sound\\BattleCastle\\mGMercAttack.wav", 1, true  );
        LoadWaveFile ( SOUND_BMS_STUN          ,   "Data\\Sound\\BattleCastle\\sDStun.wav", MAX_CHANNEL, true  );        
        LoadWaveFile ( SOUND_BMS_STUN_REMOVAL  ,   "Data\\Sound\\BattleCastle\\sDStunCancel.wav", MAX_CHANNEL, true   );    
        LoadWaveFile ( SOUND_BMS_MANA          ,   "Data\\Sound\\BattleCastle\\sDSwllMana.wav", MAX_CHANNEL, true   );      
        LoadWaveFile ( SOUND_BMS_INVISIBLE     ,   "Data\\Sound\\BattleCastle\\sDTrans.wav", MAX_CHANNEL , true  );      
        LoadWaveFile ( SOUND_BMS_VISIBLE       ,   "Data\\Sound\\BattleCastle\\sDStunCancel.wav", MAX_CHANNEL, true   );         
        LoadWaveFile ( SOUND_BMS_MAGIC_REMOVAL ,   "Data\\Sound\\BattleCastle\\sDMagicCancel.wav", MAX_CHANNEL, true   );        
        LoadWaveFile ( SOUND_BCS_RUSH          ,   "Data\\Sound\\BattleCastle\\sCHaveyBlow.wav", MAX_CHANNEL, true   );    
        LoadWaveFile ( SOUND_BCS_JAVELIN       ,   "Data\\Sound\\BattleCastle\\sCShockWave.wav", MAX_CHANNEL, true   );  
        LoadWaveFile ( SOUND_BCS_DEEP_IMPACT   ,   "Data\\Sound\\BattleCastle\\sCFireArrow.wav", MAX_CHANNEL, true  );  
        LoadWaveFile ( SOUND_BCS_DEATH_CANON   ,   "Data\\Sound\\BattleCastle\\sCMW.wav", MAX_CHANNEL, true  );       
        LoadWaveFile ( SOUND_BCS_ONE_FLASH     ,   "Data\\Sound\\BattleCastle\\sCColdAttack.wav", MAX_CHANNEL, true  );     
        LoadWaveFile ( SOUND_BCS_SPACE_SPLIT   ,   "Data\\Sound\\BattleCastle\\sCDarkAttack.wav", MAX_CHANNEL, true  );
        LoadWaveFile ( SOUND_BCS_BRAND_OF_SKILL,   "Data\\Sound\\BattleCastle\\sCDarkAssist.wav", 1, true  );  
        break;

	case WD_31HUNTING_GROUND:
		gLoadData.AccessModel(MODEL_BUTTERFLY01	 ,"Data\\Object1\\","Butterfly",1);
		gLoadData.OpenTexture(MODEL_BUTTERFLY01	 ,"Object1\\");

		LoadBitmap("Effect\\clouds.jpg"	  ,BITMAP_CLOUD ,GL_LINEAR,GL_CLAMP_TO_EDGE);
		LoadBitmap("Monster\\bossmap1_R.jpg"	  ,BITMAP_HGBOSS_PATTERN ,GL_LINEAR,GL_CLAMP_TO_EDGE);
		LoadBitmap("Monster\\bosswing.tga",BITMAP_HGBOSS_WING  ,GL_NEAREST,GL_REPEAT);
		LoadBitmap("Skill\\bossrock1_R.JPG" ,BITMAP_FISSURE_FIRE ,GL_LINEAR,GL_CLAMP_TO_EDGE);

		LoadWaveFile ( SOUND_BC_HUNTINGGROUND_AMBIENT,    "Data\\Sound\\w31\\aW31.wav", 1, true );
		LoadWaveFile ( SOUND_BC_AXEWARRIOR_MOVE1,         "Data\\Sound\\w31\\mAWidle1.wav", 1 );
		LoadWaveFile ( SOUND_BC_AXEWARRIOR_MOVE2,         "Data\\Sound\\w31\\mAWidle2.wav", 1 );
		LoadWaveFile ( SOUND_BC_AXEWARRIOR_ATTACK1,       "Data\\Sound\\w31\\mAWattack1.wav", 1 );
		LoadWaveFile ( SOUND_BC_AXEWARRIOR_ATTACK2,       "Data\\Sound\\w31\\mAWattack2.wav", 1 );
		LoadWaveFile ( SOUND_BC_AXEWARRIOR_DIE,	          "Data\\Sound\\w31\\mAWdeath.wav", 1 );
		LoadWaveFile ( SOUND_BC_LIZARDWARRIOR_MOVE1,      "Data\\Sound\\w31\\mLWidle1.wav", 1 );
		LoadWaveFile ( SOUND_BC_LIZARDWARRIOR_MOVE2,      "Data\\Sound\\w31\\mLWidle2.wav", 1 );
		LoadWaveFile ( SOUND_BC_LIZARDWARRIOR_ATTACK1,    "Data\\Sound\\w31\\mLWattack1.wav", 1 );
		LoadWaveFile ( SOUND_BC_LIZARDWARRIOR_ATTACK2,    "Data\\Sound\\w31\\mLWattack2.wav", 1 );
		LoadWaveFile ( SOUND_BC_LIZARDWARRIOR_DIE,        "Data\\Sound\\w31\\mLWdeath.wav", 1 );
		LoadWaveFile ( SOUND_BC_POISONGOLEM_MOVE1,      "Data\\Sound\\w31\\mPGidle1.wav", 1 );
		LoadWaveFile ( SOUND_BC_POISONGOLEM_MOVE2,      "Data\\Sound\\w31\\mPGidle2.wav", 1 );
		LoadWaveFile ( SOUND_BC_POISONGOLEM_ATTACK1,    "Data\\Sound\\w31\\mPGattack1.wav", 1 );
		LoadWaveFile ( SOUND_BC_POISONGOLEM_ATTACK2,    "Data\\Sound\\w31\\mPGattack2.wav", 1 );
		LoadWaveFile ( SOUND_BC_POISONGOLEM_ATTACK3,    "Data\\Sound\\w31\\mPGeff1.wav", 1 );
		LoadWaveFile ( SOUND_BC_POISONGOLEM_DIE,        "Data\\Sound\\w31\\mPGdeath.wav", 1 );
		LoadWaveFile ( SOUND_BC_QUEENBEE_MOVE1,           "Data\\Sound\\w31\\mQBidle1.wav", 1 );
		LoadWaveFile ( SOUND_BC_QUEENBEE_MOVE2,           "Data\\Sound\\w31\\mQBidle2.wav", 1 );
		LoadWaveFile ( SOUND_BC_QUEENBEE_ATTACK1,         "Data\\Sound\\w31\\mQBattack1.wav", 1 );
		LoadWaveFile ( SOUND_BC_QUEENBEE_ATTACK2,         "Data\\Sound\\w31\\mQBattack2.wav", 1 );
		LoadWaveFile ( SOUND_BC_QUEENBEE_DIE,             "Data\\Sound\\w31\\mQBdeath.wav", 1 );
		LoadWaveFile ( SOUND_BC_FIREGOLEM_MOVE1,           "Data\\Sound\\w31\\mFGidle1.wav", 1 );
		LoadWaveFile ( SOUND_BC_FIREGOLEM_MOVE2,           "Data\\Sound\\w31\\mFGidle2.wav", 1 );
		LoadWaveFile ( SOUND_BC_FIREGOLEM_ATTACK1,         "Data\\Sound\\w31\\mFGattack1.wav", 1 );
		LoadWaveFile ( SOUND_BC_FIREGOLEM_ATTACK2,         "Data\\Sound\\w31\\mFGattack2.wav", 1 );
		LoadWaveFile ( SOUND_BC_FIREGOLEM_DIE,             "Data\\Sound\\w31\\mFGdeath.wav", 1 );
		LoadWaveFile ( SOUND_BC_EROHIM_ENTER,           "Data\\Sound\\w31\\mELOidle1.wav", 1 );
		LoadWaveFile ( SOUND_BC_EROHIM_ATTACK1,         "Data\\Sound\\w31\\mELOattack1.wav", 1 );
		LoadWaveFile ( SOUND_BC_EROHIM_ATTACK2,         "Data\\Sound\\w31\\mELOattack2.wav", 1 );
		LoadWaveFile ( SOUND_BC_EROHIM_ATTACK3,         "Data\\Sound\\w31\\mELOeff1.wav", 1 );
		LoadWaveFile ( SOUND_BC_EROHIM_DIE,             "Data\\Sound\\w31\\mELOdeath.wav", 1 );
		break;
	case WD_33AIDA:
		gLoadData.AccessModel(MODEL_BUTTERFLY01	 ,"Data\\Object1\\","Butterfly",1);
		gLoadData.OpenTexture(MODEL_BUTTERFLY01	 ,"Object1\\");
		gLoadData.AccessModel(MODEL_TREE_ATTACK	 ,"Data\\Object34\\","tree_eff");
		gLoadData.OpenTexture(MODEL_TREE_ATTACK	 ,"Object34\\");
		gLoadData.AccessModel(MODEL_BUG01+1         ,"Data\\Object9\\","Bug",2);
		gLoadData.OpenTexture(MODEL_BUG01+1		  ,"Object9\\");

		LoadBitmap("Effect\\clouds.jpg"	  ,BITMAP_CLOUD ,GL_LINEAR,GL_CLAMP_TO_EDGE);

		LoadWaveFile ( SOUND_AIDA_AMBIENT,    "Data\\Sound\\w34\\aida_ambi.wav", 1, true );
		LoadWaveFile ( SOUND_AIDA_BLUEGOLEM_MOVE1,             "Data\\Sound\\w34\\bg_idle1.wav", 1 );
		LoadWaveFile ( SOUND_AIDA_BLUEGOLEM_MOVE2,             "Data\\Sound\\w34\\bg_idle2.wav", 1 );
		LoadWaveFile ( SOUND_AIDA_BLUEGOLEM_ATTACK1,           "Data\\Sound\\w34\\bg_attack1.wav", 1 );
		LoadWaveFile ( SOUND_AIDA_BLUEGOLEM_ATTACK2,           "Data\\Sound\\w34\\bg_attack2.wav", 1 );
		LoadWaveFile ( SOUND_AIDA_BLUEGOLEM_DIE,               "Data\\Sound\\w34\\bg_death.wav", 1 );
		LoadWaveFile ( SOUND_AIDA_DEATHRAIDER_MOVE1,           "Data\\Sound\\w34\\dr_idle1.wav", 1 );
		LoadWaveFile ( SOUND_AIDA_DEATHRAIDER_MOVE2,           "Data\\Sound\\w34\\dr_idle2.wav", 1 );
		LoadWaveFile ( SOUND_AIDA_DEATHRAIDER_ATTACK1,         "Data\\Sound\\w34\\dr_attack1.wav", 1 );
		LoadWaveFile ( SOUND_AIDA_DEATHRAIDER_ATTACK2,         "Data\\Sound\\w34\\dr_attack2.wav", 1 );
		LoadWaveFile ( SOUND_AIDA_DEATHRAIDER_DIE,             "Data\\Sound\\w34\\dr_death.wav", 1 );
		LoadWaveFile ( SOUND_AIDA_DEATHTREE_MOVE1,             "Data\\Sound\\w34\\dt_idle1.wav", 1 );
		LoadWaveFile ( SOUND_AIDA_DEATHTREE_MOVE2,             "Data\\Sound\\w34\\dt_idle2.wav", 1 );
		LoadWaveFile ( SOUND_AIDA_DEATHTREE_ATTACK1,           "Data\\Sound\\w34\\dt_attack1.wav", 1 );
		LoadWaveFile ( SOUND_AIDA_DEATHTREE_ATTACK2,           "Data\\Sound\\w34\\dt_attack2.wav", 1 );
		LoadWaveFile ( SOUND_AIDA_DEATHTREE_DIE,               "Data\\Sound\\w34\\dt_death.wav", 1 );
		LoadWaveFile ( SOUND_AIDA_FORESTORC_MOVE1,             "Data\\Sound\\w34\\fo_idle1.wav", 1 );
		LoadWaveFile ( SOUND_AIDA_FORESTORC_MOVE2,             "Data\\Sound\\w34\\fo_idle2.wav", 1 );
		LoadWaveFile ( SOUND_AIDA_FORESTORC_ATTACK1,           "Data\\Sound\\w34\\fo_attack1.wav", 1 );
		LoadWaveFile ( SOUND_AIDA_FORESTORC_ATTACK2,           "Data\\Sound\\w34\\fo_attack2.wav", 1 );
		LoadWaveFile ( SOUND_AIDA_FORESTORC_DIE,               "Data\\Sound\\w34\\fo_death.wav", 1 );
		LoadWaveFile ( SOUND_AIDA_HELL_MOVE1,				   "Data\\Sound\\w34\\hm_idle1.wav", 1 );
		LoadWaveFile ( SOUND_AIDA_HELL_MOVE2,				   "Data\\Sound\\w34\\hm_idle2.wav", 1 );
		LoadWaveFile ( SOUND_AIDA_HELL_ATTACK1,			  	   "Data\\Sound\\w34\\hm_attack1.wav", 1 );
		LoadWaveFile ( SOUND_AIDA_HELL_ATTACK2,			  	   "Data\\Sound\\w34\\hm_firelay.wav", 1 );
		LoadWaveFile ( SOUND_AIDA_HELL_ATTACK3,			  	   "Data\\Sound\\w34\\hm_bloodywind.wav", 1 );
		LoadWaveFile ( SOUND_AIDA_HELL_DIE,					   "Data\\Sound\\w34\\hm_death.wav", 1 );
		LoadWaveFile ( SOUND_AIDA_WITCHQUEEN_MOVE1,            "Data\\Sound\\w34\\wq_idle1.wav", 1 );
		LoadWaveFile ( SOUND_AIDA_WITCHQUEEN_MOVE2,            "Data\\Sound\\w34\\wq_idle2.wav", 1 );
		LoadWaveFile ( SOUND_AIDA_WITCHQUEEN_ATTACK1,          "Data\\Sound\\w34\\wq_attack1.wav", 1 );
		LoadWaveFile ( SOUND_AIDA_WITCHQUEEN_ATTACK2,          "Data\\Sound\\w34\\wq_attack2.wav", 1 );
		LoadWaveFile ( SOUND_AIDA_WITCHQUEEN_DIE,              "Data\\Sound\\w34\\wq_death.wav", 1 );
		LoadWaveFile ( SOUND_CHAOS_THUNDER01,				   "Data\\Sound\\eElec1.wav", 1 );
		LoadWaveFile ( SOUND_CHAOS_THUNDER02,				   "Data\\Sound\\eElec2.wav", 1 );
		break;
	case WD_68DOPPLEGANGER4:
		LoadBitmap("Effect\\clouds.jpg"	  ,BITMAP_CLOUD,GL_LINEAR,GL_CLAMP_TO_EDGE);
		LoadBitmap("effect\\water.jpg", BITMAP_TWINTAIL_WATER, GL_LINEAR, GL_CLAMP_TO_EDGE);
	case WD_37KANTURU_1ST:
		gLoadData.AccessModel(MODEL_BUTTERFLY01	 ,"Data\\Object1\\","Butterfly",1);
		gLoadData.OpenTexture(MODEL_BUTTERFLY01	 ,"Object1\\");

		LoadWaveFile(SOUND_KANTURU_1ST_BG_WATERFALL,	"Data\\Sound\\w37\\kan_ruin_waterfall.wav", 1);
		LoadWaveFile(SOUND_KANTURU_1ST_BG_ELEC,			"Data\\Sound\\w37\\kan_ruin_elec.wav", 1);
		LoadWaveFile(SOUND_KANTURU_1ST_BG_WHEEL,		"Data\\Sound\\w37\\kan_ruin_wheel.wav", 1);
		LoadWaveFile(SOUND_KANTURU_1ST_BG_PLANT,		"Data\\Sound\\w37\\kan_ruin_plant.wav", 1);
		LoadWaveFile(SOUND_KANTURU_1ST_BG_GLOBAL,		"Data\\Sound\\w37\\kan_ruin_global.wav", 1);
		LoadWaveFile(SOUND_KANTURU_1ST_BER_MOVE1,		"Data\\Sound\\w37\\ber_idle-01.wav", 1);
		LoadWaveFile(SOUND_KANTURU_1ST_BER_MOVE2,		"Data\\Sound\\w37\\ber_idle-02.wav", 1);
		LoadWaveFile(SOUND_KANTURU_1ST_BER_ATTACK1,		"Data\\Sound\\w37\\ber_attack-01.wav", 1);
		LoadWaveFile(SOUND_KANTURU_1ST_BER_ATTACK2,		"Data\\Sound\\w37\\ber_attack-02.wav", 1);
		LoadWaveFile(SOUND_KANTURU_1ST_BER_DIE,			"Data\\Sound\\w37\\ber_death.wav", 1);
		LoadWaveFile(SOUND_KANTURU_1ST_GIGAN_MOVE1,		"Data\\Sound\\w37\\gigan_idle-01.wav", 1);
		LoadWaveFile(SOUND_KANTURU_1ST_GIGAN_ATTACK1,	"Data\\Sound\\w37\\gigan_attack-01.wav", 1);
		LoadWaveFile(SOUND_KANTURU_1ST_GIGAN_ATTACK2,	"Data\\Sound\\w37\\gigan_attack-02.wav", 1);
		LoadWaveFile(SOUND_KANTURU_1ST_GIGAN_DIE,		"Data\\Sound\\w37\\gigan_death.wav", 1);
		LoadWaveFile(SOUND_KANTURU_1ST_GENO_MOVE1,		"Data\\Sound\\w37\\geno_idle-01.wav", 1);
		LoadWaveFile(SOUND_KANTURU_1ST_GENO_MOVE2,		"Data\\Sound\\w37\\geno_idle-02.wav", 1);
		LoadWaveFile(SOUND_KANTURU_1ST_GENO_ATTACK1,	"Data\\Sound\\w37\\geno_attack-01.wav", 1);
		LoadWaveFile(SOUND_KANTURU_1ST_GENO_ATTACK2,	"Data\\Sound\\w37\\geno_attack-02.wav", 1);
		LoadWaveFile(SOUND_KANTURU_1ST_GENO_DIE,		"Data\\Sound\\w37\\geno_death.wav", 1);
		LoadWaveFile(SOUND_KANTURU_1ST_KENTA_MOVE1		, "Data\\Sound\\w37\\kenta_idle-01.wav", 1);				
		LoadWaveFile(SOUND_KANTURU_1ST_KENTA_MOVE2		, "Data\\Sound\\w37\\kenta_idle-02.wav", 1);
		LoadWaveFile(SOUND_KANTURU_1ST_KENTA_ATTACK1	, "Data\\Sound\\w37\\kenta_attack-01.wav", 1);
		LoadWaveFile(SOUND_KANTURU_1ST_KENTA_ATTACK2	, "Data\\Sound\\w37\\kenta_skill-01.wav", 1);
		LoadWaveFile(SOUND_KANTURU_1ST_KENTA_DIE		, "Data\\Sound\\w37\\kenta_death.wav", 1);
		LoadWaveFile(SOUND_KANTURU_1ST_BLADE_MOVE1	,	  "Data\\Sound\\w37\\blade_idle-01.wav", 1);					
		LoadWaveFile(SOUND_KANTURU_1ST_BLADE_MOVE2	,		"Data\\Sound\\w37\\blade_idle-02.wav", 1);				
		LoadWaveFile(SOUND_KANTURU_1ST_BLADE_ATTACK1	,"Data\\Sound\\w37\\blade_attack-01.wav", 1);
		LoadWaveFile(SOUND_KANTURU_1ST_BLADE_ATTACK2	,"Data\\Sound\\w37\\blade_attack-02.wav", 1);
		LoadWaveFile(SOUND_KANTURU_1ST_BLADE_DIE		,"Data\\Sound\\w37\\blade_death.wav", 1);
		LoadWaveFile(SOUND_KANTURU_1ST_SATI_MOVE1	,"Data\\Sound\\w37\\sati_idle-01.wav", 1);					
		LoadWaveFile(SOUND_KANTURU_1ST_SATI_MOVE2	,"Data\\Sound\\w37\\sati_idle-02.wav", 1);
		LoadWaveFile(SOUND_KANTURU_1ST_SATI_ATTACK1	,"Data\\Sound\\w37\\sati_attack-01.wav", 1);
		LoadWaveFile(SOUND_KANTURU_1ST_SATI_ATTACK2	,"Data\\Sound\\w37\\sati_attack-02.wav", 1);
		LoadWaveFile(SOUND_KANTURU_1ST_SATI_DIE		,"Data\\Sound\\w37\\sati_death.wav", 1);
		LoadWaveFile(SOUND_KANTURU_1ST_SWOLF_MOVE1	,"Data\\Sound\\w37\\swolf_idle-01.wav", 1);					
		LoadWaveFile(SOUND_KANTURU_1ST_SWOLF_MOVE2	,"Data\\Sound\\w37\\swolf_idle-02.wav", 1);
		LoadWaveFile(SOUND_KANTURU_1ST_SWOLF_ATTACK1	,"Data\\Sound\\w37\\swolf_attack-01.wav", 1);
		LoadWaveFile(SOUND_KANTURU_1ST_SWOLF_ATTACK2	,"Data\\Sound\\w37\\swolf_attack-02.wav", 1);
		LoadWaveFile(SOUND_KANTURU_1ST_SWOLF_DIE		,"Data\\Sound\\w37\\swolf_death.wav", 1);
		LoadWaveFile(SOUND_KANTURU_1ST_IR_MOVE1	,"Data\\Sound\\w37\\ir_idle-01.wav", 1);					
		LoadWaveFile(SOUND_KANTURU_1ST_IR_MOVE2	,"Data\\Sound\\w37\\ir_idle-02.wav", 1);
		LoadWaveFile(SOUND_KANTURU_1ST_IR_ATTACK1	,"Data\\Sound\\w37\\ir_attack-01.wav", 1);
		LoadWaveFile(SOUND_KANTURU_1ST_IR_ATTACK2	,"Data\\Sound\\w37\\ir_attack-02.wav", 1);
		LoadWaveFile(SOUND_KANTURU_1ST_IR_DIE		,"Data\\Sound\\w37\\ir_death.wav", 1);		
		break;
	case WD_38KANTURU_2ND:
		{
			g_TrapCanon.Open_TrapCanon();

			gLoadData.AccessModel(MODEL_STORM2		,"Data\\SKill\\","boswind");
			gLoadData.OpenTexture(MODEL_STORM2		,"SKill\\");
			gLoadData.AccessModel(MODEL_STORM3		,"Data\\Skill\\","mayatonedo");
			gLoadData.OpenTexture(MODEL_STORM3		,"Skill\\");

			LoadBitmap("Object39\\k_effect_01.JPG"		, BITMAP_KANTURU_2ND_EFFECT1, GL_LINEAR, GL_CLAMP_TO_EDGE);
			LoadBitmap("Npc\\khs_kan2gate001.jpg"	, BITMAP_KANTURU_2ND_NPC1, GL_LINEAR, GL_CLAMP_TO_EDGE);
			LoadBitmap("Npc\\khs_kan2gate003.jpg"	, BITMAP_KANTURU_2ND_NPC2, GL_LINEAR, GL_CLAMP_TO_EDGE);
			LoadBitmap("Npc\\khs_kan2gate004.jpg"	, BITMAP_KANTURU_2ND_NPC3, GL_LINEAR, GL_CLAMP_TO_EDGE);
			
			LoadWaveFile(SOUND_KANTURU_2ND_MAPSOUND_GEAR,		"Data\\Sound\\w38\\kan_relic_gear.wav", 1);
			LoadWaveFile(SOUND_KANTURU_2ND_MAPSOUND_INCUBATOR,	"Data\\Sound\\w38\\kan_relic_incubator.wav", 1);
			LoadWaveFile(SOUND_KANTURU_2ND_MAPSOUND_HOLE,		"Data\\Sound\\w38\\kan_relic_hole.wav", 1);
			LoadWaveFile(SOUND_KANTURU_2ND_MAPSOUND_GLOBAL,		"Data\\Sound\\w38\\kan_relic_global.wav", 1);
			LoadWaveFile(SOUND_KANTURU_2ND_PERSO_MOVE1,		"Data\\Sound\\w38\\perso_idle-01.wav", 1);
			LoadWaveFile(SOUND_KANTURU_2ND_PERSO_MOVE2,		"Data\\Sound\\w38\\perso_idle-02.wav", 1);
			LoadWaveFile(SOUND_KANTURU_2ND_PERSO_ATTACK1,	"Data\\Sound\\w38\\perso_attack-01.wav", 1);
			LoadWaveFile(SOUND_KANTURU_2ND_PERSO_ATTACK2,	"Data\\Sound\\w38\\perso_attack-02.wav", 1);
			LoadWaveFile(SOUND_KANTURU_2ND_PERSO_DIE,		"Data\\Sound\\w38\\perso_death.wav", 1);
			LoadWaveFile(SOUND_KANTURU_2ND_TWIN_MOVE1,		"Data\\Sound\\w38\\twin_idle-01.wav", 1);
			LoadWaveFile(SOUND_KANTURU_2ND_TWIN_MOVE2,		"Data\\Sound\\w38\\twin_idle-02.wav", 1);
			LoadWaveFile(SOUND_KANTURU_2ND_TWIN_ATTACK1,	"Data\\Sound\\w38\\twin_attack-01.wav", 1);
			LoadWaveFile(SOUND_KANTURU_2ND_TWIN_ATTACK2,	"Data\\Sound\\w38\\twin_attack-02.wav", 1);
			LoadWaveFile(SOUND_KANTURU_2ND_TWIN_DIE,		"Data\\Sound\\w38\\twin_death.wav", 1);
			LoadWaveFile(SOUND_KANTURU_2ND_DRED_MOVE1,		"Data\\Sound\\w38\\dred_idle-01.wav", 1);
			LoadWaveFile(SOUND_KANTURU_2ND_DRED_MOVE2,		"Data\\Sound\\w38\\dred_idle-02.wav", 1);
			LoadWaveFile(SOUND_KANTURU_2ND_DRED_ATTACK1,	"Data\\Sound\\w38\\dred_attack-01.wav", 1);
			LoadWaveFile(SOUND_KANTURU_2ND_DRED_ATTACK2,	"Data\\Sound\\w38\\dred_attack-02.wav", 1);
			LoadWaveFile(SOUND_KANTURU_2ND_DRED_DIE,		"Data\\Sound\\w38\\dred_death.wav", 1);
		}
		break;
	case WD_39KANTURU_3RD:
		{
			LoadBitmap("Monster\\nightmare_R.jpg"			, BITMAP_NIGHTMARE_EFFECT1, GL_LINEAR, GL_CLAMP_TO_EDGE);
			LoadBitmap("Monster\\nightmaresward_R.jpg"	, BITMAP_NIGHTMARE_EFFECT2, GL_LINEAR, GL_CLAMP_TO_EDGE);
			LoadBitmap("Monster\\nightmare_cloth.tga"		, BITMAP_NIGHTMARE_ROBE);
			LoadBitmap("Object40\\maya01_R.jpg"			, BITMAP_MAYA_BODY, GL_LINEAR, GL_CLAMP_TO_EDGE);
			LoadBitmap("Object40\\Mtowereffe.JPG"			, BITMAP_KANTURU3RD_OBJECT, GL_LINEAR, GL_CLAMP_TO_EDGE);
			LoadBitmap("Effect\\bluering0001_R.jpg"		, BITMAP_ENERGY_RING, GL_LINEAR, GL_CLAMP_TO_EDGE);
			LoadBitmap("Effect\\bluewave0001_R.jpg"		, BITMAP_ENERGY_FIELD, GL_LINEAR, GL_CLAMP_TO_EDGE);

			gLoadData.AccessModel(MODEL_STORM2		,"Data\\SKill\\","boswind");
			gLoadData.OpenTexture(MODEL_STORM2		,"SKill\\");
			gLoadData.AccessModel(MODEL_STORM3		,"Data\\Skill\\","mayatonedo");
			gLoadData.OpenTexture(MODEL_STORM3		,"Skill\\");
			gLoadData.AccessModel(MODEL_MAYASTAR		,"Data\\Skill\\","arrowsre05");
			gLoadData.OpenTexture(MODEL_MAYASTAR		,"Skill\\");
			gLoadData.AccessModel(MODEL_MAYASTONE1	,"Data\\Skill\\","mayastone01");
			gLoadData.OpenTexture(MODEL_MAYASTONE1	,"Skill\\");
			gLoadData.AccessModel(MODEL_MAYASTONE2	,"Data\\Skill\\","mayastone02");
			gLoadData.OpenTexture(MODEL_MAYASTONE2	,"Skill\\");
			gLoadData.AccessModel(MODEL_MAYASTONE3	,"Data\\Skill\\","mayastone03");
			gLoadData.OpenTexture(MODEL_MAYASTONE3	,"Skill\\");
			gLoadData.AccessModel(MODEL_MAYASTONE4	,"Data\\Skill\\","mayastone04");
			gLoadData.OpenTexture(MODEL_MAYASTONE4	,"Skill\\");
			gLoadData.AccessModel(MODEL_MAYASTONE5	,"Data\\Skill\\","mayastone05");
			gLoadData.OpenTexture(MODEL_MAYASTONE5	,"Skill\\");
			gLoadData.AccessModel(MODEL_MAYASTONEFIRE	,"Data\\Skill\\","mayastonebluefire");
			gLoadData.OpenTexture(MODEL_MAYASTONEFIRE	,"Skill\\");
			gLoadData.AccessModel(MODEL_MAYAHANDSKILL	,"Data\\Skill\\","hendlight02");
			gLoadData.OpenTexture(MODEL_MAYAHANDSKILL	,"Skill\\");

			LoadWaveFile ( SOUND_KANTURU_3RD_MAYA_INTRO,          "Data\\Sound\\w39\\maya_intro.wav", 1 );
			LoadWaveFile ( SOUND_KANTURU_3RD_MAYA_END,            "Data\\Sound\\w39\\maya_death.wav", 1 );
			LoadWaveFile ( SOUND_KANTURU_3RD_MAYA_STORM,          "Data\\Sound\\w39\\maya_storm.wav", 1 );
			LoadWaveFile ( SOUND_KANTURU_3RD_MAYAHAND_ATTACK1,    "Data\\Sound\\w39\\maya_hand_attack-01.wav", 1 );
			LoadWaveFile ( SOUND_KANTURU_3RD_MAYAHAND_ATTACK2,	  "Data\\Sound\\w39\\maya_hand_attack-02.wav", 1 );
			LoadWaveFile ( SOUND_KANTURU_3RD_NIGHTMARE_INTRO,     "Data\\Sound\\w39\\nightmare_intro.wav", 1 );
			LoadWaveFile ( SOUND_KANTURU_3RD_NIGHTMARE_ATT1,     "Data\\Sound\\w39\\nightmare_attack-01.wav", 1 );
			LoadWaveFile ( SOUND_KANTURU_3RD_NIGHTMARE_ATT2,     "Data\\Sound\\w39\\nightmare_skill-01", 1 );
			LoadWaveFile ( SOUND_KANTURU_3RD_NIGHTMARE_ATT3,     "Data\\Sound\\w39\\nightmare_skill-02", 1 );
			LoadWaveFile ( SOUND_KANTURU_3RD_NIGHTMARE_ATT4,     "Data\\Sound\\w39\\nightmare_skill-03", 1 );
			LoadWaveFile ( SOUND_KANTURU_3RD_NIGHTMARE_DIE,      "Data\\Sound\\w39\\nightmare_death.wav", 1 );
			LoadWaveFile ( SOUND_KANTURU_3RD_NIGHTMARE_IDLE1,    "Data\\Sound\\w39\\nightmare_idle-01.wav", 1 );
			LoadWaveFile ( SOUND_KANTURU_3RD_NIGHTMARE_IDLE2,    "Data\\Sound\\w39\\nightmare_idle-02.wav", 1 );
			LoadWaveFile ( SOUND_KANTURU_3RD_NIGHTMARE_TELE,     "Data\\Sound\\w39\\nightmare_tele.wav", 1 );
			LoadWaveFile ( SOUND_KANTURU_3RD_MAP_SOUND01,          "Data\\Sound\\w39\\kan_boss_crystal.wav", 1 );
			LoadWaveFile ( SOUND_KANTURU_3RD_MAP_SOUND02,          "Data\\Sound\\w39\\kan_boss_disfield.wav", 1 );
			LoadWaveFile ( SOUND_KANTURU_3RD_MAP_SOUND03,          "Data\\Sound\\w39\\kan_boss_field.wav", 1 );
			LoadWaveFile ( SOUND_KANTURU_3RD_MAP_SOUND04,          "Data\\Sound\\w39\\kan_boss_gear.wav", 1 );
			LoadWaveFile ( SOUND_KANTURU_3RD_MAP_SOUND05,          "Data\\Sound\\w39\\kan_boss_incubator.wav", 1 );
			LoadWaveFile ( SOUND_KANTURU_3RD_AMBIENT,			   "Data\\Sound\\w39\\kan_boss_global.wav", 1 );
		}
		break;
	case WD_45CURSEDTEMPLE_LV1:
	case WD_45CURSEDTEMPLE_LV2:
	case WD_45CURSEDTEMPLE_LV3:	
	case WD_45CURSEDTEMPLE_LV4:	
	case WD_45CURSEDTEMPLE_LV5:	
	case WD_45CURSEDTEMPLE_LV6:
		{
			LoadBitmap("Effect\\clouds.jpg", BITMAP_CLOUD, GL_LINEAR, GL_CLAMP_TO_EDGE);
			LoadBitmap("Effect\\clud64.jpg", BITMAP_CLUD64, GL_LINEAR, GL_CLAMP_TO_EDGE);
			LoadBitmap("Effect\\ghosteffect01.jpg", BITMAP_GHOST_CLOUD1, GL_LINEAR, GL_CLAMP_TO_EDGE);
			LoadBitmap("Effect\\ghosteffect02.jpg", BITMAP_GHOST_CLOUD2, GL_LINEAR, GL_CLAMP_TO_EDGE);
			LoadBitmap("Effect\\torchfire.jpg", BITMAP_TORCH_FIRE, GL_LINEAR, GL_CLAMP_TO_EDGE);
			LoadBitmap("Effect\\clouds2.jpg", BITMAP_EVENT_CLOUD, GL_LINEAR, GL_CLAMP_TO_EDGE);
			
			//EFFECT
			gLoadData.AccessModel(MODEL_FALL_STONE_EFFECT, "Data\\Object47\\", "Stoneeffec");
			gLoadData.OpenTexture(MODEL_FALL_STONE_EFFECT, "Object47\\");
			//game system sound
			LoadWaveFile ( SOUND_CURSEDTEMPLE_GAMESYSTEM1,     "Data\\Sound\\w47\\cursedtemple_start01.wav", 1 );
			LoadWaveFile ( SOUND_CURSEDTEMPLE_GAMESYSTEM2,     "Data\\Sound\\w47\\cursedtemple_statue01.wav", 1 );
			LoadWaveFile ( SOUND_CURSEDTEMPLE_GAMESYSTEM3,     "Data\\Sound\\w47\\cursedtemple_holy01.wav", 1 );
			LoadWaveFile ( SOUND_CURSEDTEMPLE_GAMESYSTEM4,     "Data\\Sound\\w47\\cursedtemple_score01.wav", 1 );
			LoadWaveFile ( SOUND_CURSEDTEMPLE_GAMESYSTEM5,     "Data\\Sound\\w47\\cursedtemple_end01.wav", 1 );
			//moster 1 - 2 effect sound
			LoadWaveFile ( SOUND_CURSEDTEMPLE_MONSTER1_IDLE,   "Data\\Sound\\w47\\cursedtemple_idle01.wav", 1 );
			LoadWaveFile ( SOUND_CURSEDTEMPLE_MONSTER_MOVE,    "Data\\Sound\\w47\\cursedtemple_move01.wav", 1 );
			LoadWaveFile ( SOUND_CURSEDTEMPLE_MONSTER1_DAMAGE, "Data\\Sound\\w47\\cursedtemple_damage01.wav", 1 );
			LoadWaveFile ( SOUND_CURSEDTEMPLE_MONSTER1_DEATH,  "Data\\Sound\\w47\\cursedtemple_death01.wav", 1 );
			//moster 3 effect sound
			LoadWaveFile ( SOUND_CURSEDTEMPLE_MONSTER2_IDLE,    "Data\\Sound\\w47\\cursedtemple_idle02.wav", 1 );
			LoadWaveFile ( SOUND_CURSEDTEMPLE_MONSTER2_ATTACK, "Data\\Sound\\w47\\cursedtemple_Attack01.wav", 1 );
			LoadWaveFile ( SOUND_CURSEDTEMPLE_MONSTER2_DAMAGE,	"Data\\Sound\\w47\\cursedtemple_damage02.wav", 1 );
			LoadWaveFile ( SOUND_CURSEDTEMPLE_MONSTER2_DEATH,	"Data\\Sound\\w47\\cursedtemple_death02.wav", 1 );
		}
		break;
	case WD_41CHANGEUP3RD_1ST:
		LoadBitmap("Effect\\clouds.jpg" , BITMAP_CLOUD, GL_LINEAR, GL_CLAMP_TO_EDGE);
		LoadBitmap("Effect\\firered.jpg" , BITMAP_FIRE_RED, GL_LINEAR, GL_CLAMP_TO_EDGE);
		LoadBitmap("Effect\\FireSnuff.jpg", BITMAP_FIRE_SNUFF, GL_LINEAR, GL_CLAMP_TO_EDGE);      //  불씨.
		
		LoadWaveFile(SOUND_3RD_CHANGE_UP_BG_CAGE1,			"Data\\Sound\\w42\\cage01.wav", 1);
		LoadWaveFile(SOUND_3RD_CHANGE_UP_BG_CAGE2,			"Data\\Sound\\w42\\cage02.wav", 1);
		LoadWaveFile(SOUND_3RD_CHANGE_UP_BG_VOLCANO,		"Data\\Sound\\w42\\volcano.wav", 1);
		LoadWaveFile(SOUND_3RD_CHANGE_UP_BG_FIREPILLAR,		"Data\\Sound\\w42\\firepillar.wav", 1);
		LoadWaveFile ( SOUND_CRY1ST_BALRAM_MOVE1,             "Data\\Sound\\w35\\balram_idle1.wav", 1 );
		LoadWaveFile ( SOUND_CRY1ST_BALRAM_MOVE2,             "Data\\Sound\\w35\\balram_idle2.wav", 1 );
		LoadWaveFile ( SOUND_CRY1ST_BALRAM_ATTACK1,           "Data\\Sound\\w35\\balram_attack1.wav", 1 );
		LoadWaveFile ( SOUND_CRY1ST_BALRAM_ATTACK2,           "Data\\Sound\\w35\\balram_attack2.wav", 1 );
		LoadWaveFile ( SOUND_CRY1ST_BALRAM_DIE,               "Data\\Sound\\w35\\balram_death.wav", 1 );
		LoadWaveFile ( SOUND_CRY1ST_DEATHSPIRIT_MOVE1,		 "Data\\Sound\\w35\\dths_idle1.wav", 1 );
		LoadWaveFile ( SOUND_CRY1ST_DEATHSPIRIT_MOVE2,		 "Data\\Sound\\w35\\dths_idle2.wav", 1 );
		LoadWaveFile ( SOUND_CRY1ST_DEATHSPIRIT_ATTACK1,     "Data\\Sound\\w35\\dths_at1.wav", 1 );
		LoadWaveFile ( SOUND_CRY1ST_DEATHSPIRIT_ATTACK2,     "Data\\Sound\\w35\\dths_at2.wav", 1 );
		LoadWaveFile ( SOUND_CRY1ST_DEATHSPIRIT_DIE,         "Data\\Sound\\w35\\dths_deat.wav", 1 );
		LoadWaveFile ( SOUND_CRY1ST_SORAM_MOVE1,             "Data\\Sound\\w35\\soram_idle1.wav", 1 );
		LoadWaveFile ( SOUND_CRY1ST_SORAM_MOVE2,             "Data\\Sound\\w35\\soram_idle2.wav", 1 );
		LoadWaveFile ( SOUND_CRY1ST_SORAM_ATTACK1,           "Data\\Sound\\w35\\soram_attack1.wav", 1 );
		LoadWaveFile ( SOUND_CRY1ST_SORAM_ATTACK2,           "Data\\Sound\\w35\\soram_attack2.wav", 1 );
		LoadWaveFile ( SOUND_CRY1ST_SORAM_DIE,               "Data\\Sound\\w35\\soram_death.wav", 1 );
		break;
	case WD_42CHANGEUP3RD_2ND:
		LoadBitmap("Effect\\clouds.jpg" , BITMAP_CLOUD, GL_LINEAR, GL_CLAMP_TO_EDGE);
		LoadBitmap("Effect\\firered.jpg" , BITMAP_FIRE_RED, GL_LINEAR, GL_CLAMP_TO_EDGE);
		LoadBitmap("Effect\\FireSnuff.jpg", BITMAP_FIRE_SNUFF, GL_LINEAR, GL_CLAMP_TO_EDGE);

		LoadWaveFile(SOUND_3RD_CHANGE_UP_BG_CAGE1,			"Data\\Sound\\w42\\cage01.wav", 1);
		LoadWaveFile(SOUND_3RD_CHANGE_UP_BG_CAGE2,			"Data\\Sound\\w42\\cage02.wav", 1);
		LoadWaveFile(SOUND_3RD_CHANGE_UP_BG_VOLCANO,		"Data\\Sound\\w42\\volcano.wav", 1);
		LoadWaveFile(SOUND_3RD_CHANGE_UP_BG_FIREPILLAR,		"Data\\Sound\\w42\\firepillar.wav", 1);
		break;
#ifndef PJH_NEW_SERVER_SELECT_MAP
	case WD_77NEW_LOGIN_SCENE:
	case WD_78NEW_CHARACTER_SCENE:
		if(gMapManager.WorldActive == WD_77NEW_LOGIN_SCENE)
		{
			LoadBitmap("Logo\\New_Login_Back01.jpg"         ,BITMAP_LOG_IN+9);
			LoadBitmap("Logo\\New_Login_Back02.jpg"         ,BITMAP_LOG_IN+10);
			LoadBitmap("World78\\bg_b_05.jpg" ,BITMAP_CHROME+3, GL_LINEAR, GL_REPEAT );
			LoadBitmap("World78\\bg_b_08.jpg" ,BITMAP_CHROME+4, GL_LINEAR, GL_REPEAT );
			LoadBitmap("Logo\\MU-logo.tga"         ,BITMAP_LOG_IN+16, GL_LINEAR);
			LoadBitmap("Logo\\MU-logo_g.jpg", BITMAP_LOG_IN+17, GL_LINEAR);

			// 몬스터 미리 읽어놓기
			OpenMonsterModel(129);
			OpenMonsterModel(130);
			OpenMonsterModel(131);
			OpenMonsterModel(133);
			OpenMonsterModel(135);
		}
		else
		{
			LoadBitmap("Logo\\sos3sky01.jpg"         ,BITMAP_LOG_IN+9);
			LoadBitmap("Logo\\sos3sky02.jpg"         ,BITMAP_LOG_IN+10);
		}
#endif //PJH_NEW_SERVER_SELECT_MAP
	case WD_51HOME_6TH_CHAR:
		LoadBitmap("Effect\\clouds.jpg" , BITMAP_CLOUD, GL_LINEAR, GL_CLAMP_TO_EDGE);
		LoadBitmap("Effect\\Map_Smoke1.jpg"     ,BITMAP_CHROME+2    ,GL_LINEAR,GL_REPEAT);
		gLoadData.AccessModel(MODEL_EAGLE			 ,"Data\\Object52\\","sos3bi01");
		gLoadData.OpenTexture(MODEL_EAGLE			 ,"Object52\\");
		if (gMapManager.WorldActive == WD_51HOME_6TH_CHAR)
		{
			gLoadData.AccessModel(MODEL_MAP_TORNADO	 ,"Data\\Object52\\","typhoonall");
			gLoadData.OpenTexture(MODEL_MAP_TORNADO	 ,"Object52\\");

			gLoadData.AccessModel(MODEL_TOTEMGOLEM_PART1	 ,"Data\\Monster\\","totemhead");
			gLoadData.OpenTexture(MODEL_TOTEMGOLEM_PART1	 ,"Monster\\");
			gLoadData.AccessModel(MODEL_TOTEMGOLEM_PART2	 ,"Data\\Monster\\","totembody");
			gLoadData.OpenTexture(MODEL_TOTEMGOLEM_PART2	 ,"Monster\\");
			gLoadData.AccessModel(MODEL_TOTEMGOLEM_PART3	 ,"Data\\Monster\\","totemleft");
			gLoadData.OpenTexture(MODEL_TOTEMGOLEM_PART3	 ,"Monster\\");
			gLoadData.AccessModel(MODEL_TOTEMGOLEM_PART4	 ,"Data\\Monster\\","totemright");
			gLoadData.OpenTexture(MODEL_TOTEMGOLEM_PART4	 ,"Monster\\");
			gLoadData.AccessModel(MODEL_TOTEMGOLEM_PART5	 ,"Data\\Monster\\","totemleg");
			gLoadData.OpenTexture(MODEL_TOTEMGOLEM_PART5	 ,"Monster\\");
			gLoadData.AccessModel(MODEL_TOTEMGOLEM_PART6	 ,"Data\\Monster\\","totemleg2");
			gLoadData.OpenTexture(MODEL_TOTEMGOLEM_PART6	 ,"Monster\\");

			LoadWaveFile ( SOUND_ELBELAND_VILLAGEPROTECTION01,			"Data\\Sound\\w52\\SE_Obj_villageprotection01.wav", 1 );
			LoadWaveFile ( SOUND_ELBELAND_WATERFALLSMALL01,				"Data\\Sound\\w52\\SE_Obj_waterfallsmall01.wav", 1 );
			LoadWaveFile ( SOUND_ELBELAND_WATERWAY01,					"Data\\Sound\\w52\\SE_Obj_waterway01.wav", 1 );
			LoadWaveFile ( SOUND_ELBELAND_ENTERDEVIAS01,				"Data\\Sound\\w52\\SE_Obj_enterdevias01.wav", 1 );
			LoadWaveFile ( SOUND_ELBELAND_WATERSMALL01,					"Data\\Sound\\w52\\SE_Obj_watersmall01.wav", 1 );
			LoadWaveFile ( SOUND_ELBELAND_RAVINE01,						"Data\\Sound\\w52\\SE_Amb_ravine01.wav", 1 );
			LoadWaveFile ( SOUND_ELBELAND_ENTERATLANCE01,				"Data\\Sound\\w52\\SE_Amb_enteratlance01.wav", 1 );
		}
		break;
	case WD_66DOPPLEGANGER2:
		{
			LoadBitmap("Effect\\clouds.jpg", BITMAP_CLOUD, GL_LINEAR, GL_CLAMP_TO_EDGE);
			LoadBitmap("Effect\\FireSnuff.jpg", BITMAP_FIRE_SNUFF, GL_LINEAR, GL_CLAMP_TO_EDGE);
			LoadBitmap("Effect\\WATERFALL2.jpg" ,BITMAP_CHROME3 ,GL_LINEAR,GL_CLAMP_TO_EDGE);
			LoadBitmap("Effect\\clud64.jpg", BITMAP_CLUD64, GL_LINEAR, GL_CLAMP_TO_EDGE);
		}
		break;
	case WD_63PK_FIELD:
		{
			gLoadData.AccessModel(MODEL_PKFIELD_ASSASSIN_EFFECT_GREEN_HEAD,"Data\\Monster\\","pk_manhead_green");
			gLoadData.OpenTexture(MODEL_PKFIELD_ASSASSIN_EFFECT_GREEN_HEAD,"Monster\\");

			gLoadData.AccessModel(MODEL_PKFIELD_ASSASSIN_EFFECT_RED_HEAD,"Data\\Monster\\","pk_manhead_red");
			gLoadData.OpenTexture(MODEL_PKFIELD_ASSASSIN_EFFECT_RED_HEAD,"Monster\\");

			gLoadData.AccessModel(MODEL_PKFIELD_ASSASSIN_EFFECT_GREEN_BODY,"Data\\Monster\\","assassin_dieg");
			gLoadData.OpenTexture(MODEL_PKFIELD_ASSASSIN_EFFECT_GREEN_BODY,"Monster\\");
			
			gLoadData.AccessModel(MODEL_PKFIELD_ASSASSIN_EFFECT_RED_BODY,"Data\\Monster\\","assassin_dier");
			gLoadData.OpenTexture(MODEL_PKFIELD_ASSASSIN_EFFECT_RED_BODY,"Monster\\");
		}
		break;
	case WD_56MAP_SWAMP_OF_QUIET:
		LoadBitmap( "Effect\\Map_Smoke1.jpg" ,BITMAP_CHROME+2, GL_LINEAR, GL_REPEAT );
		LoadBitmap( "Effect\\Map_Smoke2.tga" ,BITMAP_CHROME+3, GL_LINEAR, GL_REPEAT );
		LoadBitmap( "Monster\\red_shadows.jpg" ,BITMAP_SHADOW_PAWN_RED, GL_LINEAR, GL_REPEAT );
		LoadBitmap( "Monster\\blue_shadows.jpg" ,BITMAP_SHADOW_KINGHT_BLUE, GL_LINEAR, GL_REPEAT );
		LoadBitmap( "Monster\\green_shadows.jpg" ,BITMAP_SHADOW_ROOK_GREEN, GL_LINEAR, GL_REPEAT );

		gLoadData.AccessModel(MODEL_SHADOW_PAWN_ANKLE_LEFT	 ,"Data\\Monster\\","shadow_pawn_7_ankle_left");
		gLoadData.OpenTexture(MODEL_SHADOW_PAWN_ANKLE_LEFT	 ,"Monster\\");
		gLoadData.AccessModel(MODEL_SHADOW_PAWN_ANKLE_RIGHT	 ,"Data\\Monster\\","shadow_pawn_7_ankle_right");
		gLoadData.OpenTexture(MODEL_SHADOW_PAWN_ANKLE_RIGHT	 ,"Monster\\");
		gLoadData.AccessModel(MODEL_SHADOW_PAWN_BELT			 ,"Data\\Monster\\","shadow_pawn_7_belt");
		gLoadData.OpenTexture(MODEL_SHADOW_PAWN_BELT			 ,"Monster\\");
		gLoadData.AccessModel(MODEL_SHADOW_PAWN_CHEST			 ,"Data\\Monster\\","shadow_pawn_7_chest");
		gLoadData.OpenTexture(MODEL_SHADOW_PAWN_CHEST			 ,"Monster\\");
		gLoadData.AccessModel(MODEL_SHADOW_PAWN_HELMET		 ,"Data\\Monster\\","shadow_pawn_7_helmet");
		gLoadData.OpenTexture(MODEL_SHADOW_PAWN_HELMET		 ,"Monster\\");
		gLoadData.AccessModel(MODEL_SHADOW_PAWN_KNEE_LEFT		 ,"Data\\Monster\\","shadow_pawn_7_knee_left");
		gLoadData.OpenTexture(MODEL_SHADOW_PAWN_KNEE_LEFT		 ,"Monster\\");
		gLoadData.AccessModel(MODEL_SHADOW_PAWN_KNEE_RIGHT	 ,"Data\\Monster\\","shadow_pawn_7_knee_right");
		gLoadData.OpenTexture(MODEL_SHADOW_PAWN_KNEE_RIGHT	 ,"Monster\\");
		gLoadData.AccessModel(MODEL_SHADOW_PAWN_WRIST_LEFT	 ,"Data\\Monster\\","shadow_pawn_7_wrist_left");
		gLoadData.OpenTexture(MODEL_SHADOW_PAWN_WRIST_LEFT	 ,"Monster\\");
		gLoadData.AccessModel(MODEL_SHADOW_PAWN_WRIST_RIGHT	 ,"Data\\Monster\\","shadow_pawn_7_wrist_right");
		gLoadData.OpenTexture(MODEL_SHADOW_PAWN_WRIST_RIGHT	 ,"Monster\\");
		gLoadData.AccessModel(MODEL_SHADOW_KNIGHT_ANKLE_LEFT	 ,"Data\\Monster\\","shadow_knight_7_ankle_left");
		gLoadData.OpenTexture(MODEL_SHADOW_KNIGHT_ANKLE_LEFT	 ,"Monster\\");
		gLoadData.AccessModel(MODEL_SHADOW_KNIGHT_ANKLE_RIGHT	 ,"Data\\Monster\\","shadow_knight_7_ankle_right");
		gLoadData.OpenTexture(MODEL_SHADOW_KNIGHT_ANKLE_RIGHT	 ,"Monster\\");
		gLoadData.AccessModel(MODEL_SHADOW_KNIGHT_BELT		 ,"Data\\Monster\\","shadow_knight_7_belt");
		gLoadData.OpenTexture(MODEL_SHADOW_KNIGHT_BELT		 ,"Monster\\");
		gLoadData.AccessModel(MODEL_SHADOW_KNIGHT_CHEST		 ,"Data\\Monster\\","shadow_knight_7_chest");
		gLoadData.OpenTexture(MODEL_SHADOW_KNIGHT_CHEST		 ,"Monster\\");
		gLoadData.AccessModel(MODEL_SHADOW_KNIGHT_HELMET		 ,"Data\\Monster\\","shadow_knight_7_helmet");
		gLoadData.OpenTexture(MODEL_SHADOW_KNIGHT_HELMET		 ,"Monster\\");
		gLoadData.AccessModel(MODEL_SHADOW_KNIGHT_KNEE_LEFT	 ,"Data\\Monster\\","shadow_knight_7_knee_left");
		gLoadData.OpenTexture(MODEL_SHADOW_KNIGHT_KNEE_LEFT	 ,"Monster\\");
		gLoadData.AccessModel(MODEL_SHADOW_KNIGHT_KNEE_RIGHT	 ,"Data\\Monster\\","shadow_knight_7_knee_right");
		gLoadData.OpenTexture(MODEL_SHADOW_KNIGHT_KNEE_RIGHT	 ,"Monster\\");
		gLoadData.AccessModel(MODEL_SHADOW_KNIGHT_WRIST_LEFT	 ,"Data\\Monster\\","shadow_knight_7_wrist_left");
		gLoadData.OpenTexture(MODEL_SHADOW_KNIGHT_WRIST_LEFT	 ,"Monster\\");
		gLoadData.AccessModel(MODEL_SHADOW_KNIGHT_WRIST_RIGHT	 ,"Data\\Monster\\","shadow_knight_7_wrist_right");
		gLoadData.OpenTexture(MODEL_SHADOW_KNIGHT_WRIST_RIGHT	 ,"Monster\\");
		gLoadData.AccessModel(MODEL_SHADOW_ROOK_ANKLE_LEFT	 ,"Data\\Monster\\","shadow_rock_7_ankle_left");
		gLoadData.OpenTexture(MODEL_SHADOW_ROOK_ANKLE_LEFT	 ,"Monster\\");
		gLoadData.AccessModel(MODEL_SHADOW_ROOK_ANKLE_RIGHT	 ,"Data\\Monster\\","shadow_rock_7_ankle_right");
		gLoadData.OpenTexture(MODEL_SHADOW_ROOK_ANKLE_RIGHT	 ,"Monster\\");
		gLoadData.AccessModel(MODEL_SHADOW_ROOK_BELT			 ,"Data\\Monster\\","shadow_rock_7_belt");
		gLoadData.OpenTexture(MODEL_SHADOW_ROOK_BELT			 ,"Monster\\");
		gLoadData.AccessModel(MODEL_SHADOW_ROOK_CHEST			 ,"Data\\Monster\\","shadow_rock_7_chest");
		gLoadData.OpenTexture(MODEL_SHADOW_ROOK_CHEST			 ,"Monster\\");
		gLoadData.AccessModel(MODEL_SHADOW_ROOK_HELMET		 ,"Data\\Monster\\","shadow_rock_7_helmet");
		gLoadData.OpenTexture(MODEL_SHADOW_ROOK_HELMET		 ,"Monster\\");
		gLoadData.AccessModel(MODEL_SHADOW_ROOK_KNEE_LEFT		 ,"Data\\Monster\\","shadow_rock_7_knee_left");
		gLoadData.OpenTexture(MODEL_SHADOW_ROOK_KNEE_LEFT		 ,"Monster\\");
		gLoadData.AccessModel(MODEL_SHADOW_ROOK_KNEE_RIGHT	 ,"Data\\Monster\\","shadow_rock_7_knee_right");
		gLoadData.OpenTexture(MODEL_SHADOW_ROOK_KNEE_RIGHT	 ,"Monster\\");
		gLoadData.AccessModel(MODEL_SHADOW_ROOK_WRIST_LEFT	 ,"Data\\Monster\\","shadow_rock_7_wrist_left");
		gLoadData.OpenTexture(MODEL_SHADOW_ROOK_WRIST_LEFT	 ,"Monster\\");
		gLoadData.AccessModel(MODEL_SHADOW_ROOK_WRIST_RIGHT	 ,"Data\\Monster\\","shadow_rock_7_wrist_right");
		gLoadData.OpenTexture(MODEL_SHADOW_ROOK_WRIST_RIGHT	 ,"Monster\\");
		gLoadData.AccessModel(MODEL_EX01_SHADOW_MASTER_ANKLE_LEFT	 ,"Data\\Monster\\","ex01shadow_rock_7_ankle_left");
		gLoadData.OpenTexture(MODEL_EX01_SHADOW_MASTER_ANKLE_LEFT	 ,"Monster\\");
		gLoadData.AccessModel(MODEL_EX01_SHADOW_MASTER_ANKLE_RIGHT	 ,"Data\\Monster\\","ex01shadow_rock_7_ankle_right");
		gLoadData.OpenTexture(MODEL_EX01_SHADOW_MASTER_ANKLE_RIGHT	 ,"Monster\\");
		gLoadData.AccessModel(MODEL_EX01_SHADOW_MASTER_BELT			 ,"Data\\Monster\\","ex01shadow_rock_7_belt");
		gLoadData.OpenTexture(MODEL_EX01_SHADOW_MASTER_BELT			 ,"Monster\\");
		gLoadData.AccessModel(MODEL_EX01_SHADOW_MASTER_CHEST			 ,"Data\\Monster\\","ex01shadow_rock_7_chest");
		gLoadData.OpenTexture(MODEL_EX01_SHADOW_MASTER_CHEST			 ,"Monster\\");
		gLoadData.AccessModel(MODEL_EX01_SHADOW_MASTER_HELMET		 ,"Data\\Monster\\","ex01shadow_rock_7_helmet");
		gLoadData.OpenTexture(MODEL_EX01_SHADOW_MASTER_HELMET		 ,"Monster\\");
		gLoadData.AccessModel(MODEL_EX01_SHADOW_MASTER_KNEE_LEFT		 ,"Data\\Monster\\","ex01shadow_rock_7_knee_left");
		gLoadData.OpenTexture(MODEL_EX01_SHADOW_MASTER_KNEE_LEFT		 ,"Monster\\");
		gLoadData.AccessModel(MODEL_EX01_SHADOW_MASTER_KNEE_RIGHT	 ,"Data\\Monster\\","ex01shadow_rock_7_knee_right");
		gLoadData.OpenTexture(MODEL_EX01_SHADOW_MASTER_KNEE_RIGHT	 ,"Monster\\");
		gLoadData.AccessModel(MODEL_EX01_SHADOW_MASTER_WRIST_LEFT	 ,"Data\\Monster\\","ex01shadow_rock_7_wrist_left");
		gLoadData.OpenTexture(MODEL_EX01_SHADOW_MASTER_WRIST_LEFT	 ,"Monster\\");
		gLoadData.AccessModel(MODEL_EX01_SHADOW_MASTER_WRIST_RIGHT	 ,"Data\\Monster\\","ex01shadow_rock_7_wrist_right");
		gLoadData.OpenTexture(MODEL_EX01_SHADOW_MASTER_WRIST_RIGHT	 ,"Monster\\");
		break;
	case WD_65DOPPLEGANGER1:
		LoadBitmap("Effect\\clouds.jpg", BITMAP_CLOUD, GL_LINEAR, GL_CLAMP_TO_EDGE);
		LoadBitmap("Effect\\Map_Smoke1.jpg", BITMAP_CHROME+2, GL_LINEAR, GL_REPEAT);
		LoadBitmap("Object9\\sand02.jpg", BITMAP_CHROME+3, GL_LINEAR, GL_REPEAT);
		LoadBitmap("Effect\\Chrome08.jpg", BITMAP_CHROME8, GL_LINEAR, GL_CLAMP_TO_EDGE);
		break;
	case WD_57ICECITY:
	case WD_58ICECITY_BOSS:	
		LoadBitmap("Effect\\Map_Smoke1.jpg", BITMAP_CHROME+2, GL_LINEAR, GL_REPEAT);
		LoadBitmap("Object9\\sand02.jpg", BITMAP_CHROME+3, GL_LINEAR, GL_REPEAT);
		LoadBitmap("Effect\\Chrome08.jpg", BITMAP_CHROME8, GL_LINEAR, GL_CLAMP_TO_EDGE);
		
		gLoadData.AccessModel(MODEL_FALL_STONE_EFFECT, "Data\\Object47\\", "Stoneeffec");
		gLoadData.OpenTexture(MODEL_FALL_STONE_EFFECT, "Object47\\");
		gLoadData.AccessModel(MODEL_WARP	 ,"Data\\Npc\\","warp01");
		gLoadData.AccessModel(MODEL_WARP2	 ,"Data\\Npc\\","warp02");
		gLoadData.AccessModel(MODEL_WARP3	 ,"Data\\Npc\\","warp03");
		gLoadData.OpenTexture(MODEL_WARP	 ,"Npc\\");
		gLoadData.OpenTexture(MODEL_WARP2	 ,"Npc\\");
		gLoadData.OpenTexture(MODEL_WARP3	 ,"Npc\\");
		gLoadData.AccessModel(MODEL_WARP4  ,"Data\\Npc\\","warp01");
		gLoadData.AccessModel(MODEL_WARP5	 ,"Data\\Npc\\","warp02");
		gLoadData.AccessModel(MODEL_WARP6	 ,"Data\\Npc\\","warp03");
		gLoadData.OpenTexture(MODEL_WARP4  ,"Npc\\");
		gLoadData.OpenTexture(MODEL_WARP5	 ,"Npc\\");
		gLoadData.OpenTexture(MODEL_WARP6	 ,"Npc\\");
		gLoadData.AccessModel(MODEL_SUMMON ,"Data\\SKill\\", "nightmaresum");
		gLoadData.OpenTexture(MODEL_SUMMON ,"SKill\\");
		gLoadData.AccessModel(MODEL_STORM2		,"Data\\SKill\\","boswind");
		gLoadData.OpenTexture(MODEL_STORM2		,"SKill\\");

		LoadWaveFile ( SOUND_KANTURU_3RD_MAYAHAND_ATTACK2,	  "Data\\Sound\\w39\\maya_hand_attack-02.wav", 1 );

		vec3_t vPos, vAngle;
		if(gMapManager.WorldActive == WD_57ICECITY)
		{
			Vector ( 0.f, 0.f, 35.f, vAngle );
			Vector ( 0.f, 0.f, 0.f, vPos );
			vPos[0] = 162*TERRAIN_SCALE; 
			vPos[1] = 83*TERRAIN_SCALE;
			CreateObject ( MODEL_WARP, vPos, vAngle );
			
			Vector ( 0.f, 0.f, 80.f, vAngle );
			Vector ( 0.f, 0.f, 0.f, vPos );
			vPos[0] = 171*TERRAIN_SCALE; 
			vPos[1] = 24*TERRAIN_SCALE;
			CreateObject ( MODEL_WARP4, vPos, vAngle );
		}
		else if(gMapManager.WorldActive == WD_58ICECITY_BOSS)
		{
			Vector ( 0.f, 0.f, 85.f, vAngle );
			Vector ( 0.f, 0.f, 0.f, vPos );
			vPos[0] = 169*TERRAIN_SCALE; 
			vPos[1] = 24*TERRAIN_SCALE;
			CreateObject ( MODEL_WARP4, vPos, vAngle );

			Vector ( 0.f, 0.f, 85.f, vAngle );
			Vector ( 0.f, 0.f, 0.f, vPos );
			vPos[0] = 170*TERRAIN_SCALE; 
			vPos[1] = 24*TERRAIN_SCALE;
			CreateObject ( MODEL_WARP4, vPos, vAngle );
		}
		break;

	case WD_69EMPIREGUARDIAN1:
	case WD_70EMPIREGUARDIAN2:
	case WD_71EMPIREGUARDIAN3:
	case WD_72EMPIREGUARDIAN4:
 		gLoadData.AccessModel(MODEL_PROJECTILE ,"Data\\Effect\\","choarms_06");
 		gLoadData.OpenTexture(MODEL_PROJECTILE ,"Effect\\");
		gLoadData.AccessModel(MODEL_DOOR_CRUSH_EFFECT_PIECE01 ,"Data\\Effect\\","piece01_01");
		gLoadData.AccessModel(MODEL_DOOR_CRUSH_EFFECT_PIECE02 ,"Data\\Effect\\","piece01_02");
		gLoadData.AccessModel(MODEL_DOOR_CRUSH_EFFECT_PIECE03 ,"Data\\Effect\\","piece01_03");
		gLoadData.AccessModel(MODEL_DOOR_CRUSH_EFFECT_PIECE04 ,"Data\\Effect\\","piece01_04");
		gLoadData.AccessModel(MODEL_DOOR_CRUSH_EFFECT_PIECE05 ,"Data\\Effect\\","piece01_05");
		gLoadData.AccessModel(MODEL_DOOR_CRUSH_EFFECT_PIECE06 ,"Data\\Effect\\","piece01_06");
		gLoadData.AccessModel(MODEL_DOOR_CRUSH_EFFECT_PIECE07 ,"Data\\Effect\\","piece01_07");
		gLoadData.AccessModel(MODEL_DOOR_CRUSH_EFFECT_PIECE08 ,"Data\\Effect\\","piece01_08");
		gLoadData.AccessModel(MODEL_DOOR_CRUSH_EFFECT_PIECE09 ,"Data\\Effect\\","newdoor_break_01");
		gLoadData.OpenTexture(MODEL_DOOR_CRUSH_EFFECT_PIECE01 ,"Effect\\");
		gLoadData.OpenTexture(MODEL_DOOR_CRUSH_EFFECT_PIECE02 ,"Effect\\");
		gLoadData.OpenTexture(MODEL_DOOR_CRUSH_EFFECT_PIECE03 ,"Effect\\");
		gLoadData.OpenTexture(MODEL_DOOR_CRUSH_EFFECT_PIECE04 ,"Effect\\");
		gLoadData.OpenTexture(MODEL_DOOR_CRUSH_EFFECT_PIECE05 ,"Effect\\");
		gLoadData.OpenTexture(MODEL_DOOR_CRUSH_EFFECT_PIECE06 ,"Effect\\");
		gLoadData.OpenTexture(MODEL_DOOR_CRUSH_EFFECT_PIECE07 ,"Effect\\");
		gLoadData.OpenTexture(MODEL_DOOR_CRUSH_EFFECT_PIECE08 ,"Effect\\");
		gLoadData.OpenTexture(MODEL_DOOR_CRUSH_EFFECT_PIECE09 ,"Effect\\");
		gLoadData.AccessModel(MODEL_STATUE_CRUSH_EFFECT_PIECE01 ,"Data\\Effect\\","NpcGagoil_Crack01");
		gLoadData.AccessModel(MODEL_STATUE_CRUSH_EFFECT_PIECE02 ,"Data\\Effect\\","NpcGagoil_Crack02");
		gLoadData.AccessModel(MODEL_STATUE_CRUSH_EFFECT_PIECE03 ,"Data\\Effect\\","NpcGagoil_Crack03");
		gLoadData.AccessModel(MODEL_STATUE_CRUSH_EFFECT_PIECE04 ,"Data\\Effect\\","NpcGagoil_Ruin");
		gLoadData.OpenTexture(MODEL_STATUE_CRUSH_EFFECT_PIECE01 ,"Effect\\");
		gLoadData.OpenTexture(MODEL_STATUE_CRUSH_EFFECT_PIECE02 ,"Effect\\");
		gLoadData.OpenTexture(MODEL_STATUE_CRUSH_EFFECT_PIECE03 ,"Effect\\");
		gLoadData.OpenTexture(MODEL_STATUE_CRUSH_EFFECT_PIECE04 ,"Effect\\");
		gLoadData.AccessModel(MODEL_DOOR_CRUSH_EFFECT_PIECE10 ,"Data\\Effect\\","sojghmoon02");
		gLoadData.AccessModel(MODEL_DOOR_CRUSH_EFFECT_PIECE11 ,"Data\\Effect\\","sojghmj01");
		gLoadData.AccessModel(MODEL_DOOR_CRUSH_EFFECT_PIECE12 ,"Data\\Effect\\","sojghmj02");
		gLoadData.AccessModel(MODEL_DOOR_CRUSH_EFFECT_PIECE13 ,"Data\\Effect\\","sojghmj03");
		gLoadData.OpenTexture(MODEL_DOOR_CRUSH_EFFECT_PIECE10 ,"Effect\\");
		gLoadData.OpenTexture(MODEL_DOOR_CRUSH_EFFECT_PIECE11 ,"Effect\\");
		gLoadData.OpenTexture(MODEL_DOOR_CRUSH_EFFECT_PIECE12 ,"Effect\\");
		gLoadData.OpenTexture(MODEL_DOOR_CRUSH_EFFECT_PIECE13 ,"Effect\\");

		LoadBitmap("Effect\\clouds.jpg", BITMAP_CLOUD, GL_LINEAR, GL_CLAMP_TO_EDGE);
		LoadBitmap("Effect\\flare.jpg", BITMAP_FLARE, GL_LINEAR,GL_CLAMP_TO_EDGE);
		LoadBitmap("Effect\\Map_Smoke1.jpg", BITMAP_CHROME+2, GL_LINEAR,GL_REPEAT);

		LoadWaveFile(SOUND_EMPIREGUARDIAN_WEATHER_RAIN,"Data\\Sound\\w69w70w71w72\\ImperialGuardianFort_out1.wav");
		LoadWaveFile(SOUND_EMPIREGUARDIAN_WEATHER_FOG,"Data\\Sound\\w69w70w71w72\\ImperialGuardianFort_out2.wav");
		LoadWaveFile(SOUND_EMPIREGUARDIAN_WEATHER_STORM,"Data\\Sound\\w69w70w71w72\\ImperialGuardianFort_out3.wav");
		LoadWaveFile(SOUND_EMPIREGUARDIAN_INDOOR_SOUND,"Data\\Sound\\w69w70w71w72\\ImperialGuardianFort_in.wav");
		LoadWaveFile(SOUND_EMPIREGUARDIAN_BOSS_GAION_MONSTER_MOVE,"Data\\Sound\\w69w70w71w72\\GaionKalein_move.wav");
		LoadWaveFile(SOUND_EMPIREGUARDIAN_BOSS_GAION_MONSTER_RAGE,"Data\\Sound\\w69w70w71w72\\GaionKalein_rage.wav");
		LoadWaveFile(SOUND_EMPIREGUARDIAN_BOSS_GAION_MONSTER_DEATH,"Data\\Sound\\w69w70w71w72\\GrandWizard_death.wav");
		LoadWaveFile(SOUND_EMPIREGUARDIAN_JERINT_MONSTER_ATTACK01,"Data\\Sound\\w69w70w71w72\\Jelint_attack1.wav");
		LoadWaveFile(SOUND_EMPIREGUARDIAN_JERINT_MONSTER_ATTACK03,"Data\\Sound\\w69w70w71w72\\Jelint_attack3.wav");
		LoadWaveFile(SOUND_EMPIREGUARDIAN_JERINT_MONSTER_MOVE01,"Data\\Sound\\w69w70w71w72\\Jelint_move01.wav");
		LoadWaveFile(SOUND_EMPIREGUARDIAN_JERINT_MONSTER_MOVE02,"Data\\Sound\\w69w70w71w72\\Jelint_move02.wav");
		LoadWaveFile(SOUND_EMPIREGUARDIAN_JERINT_MONSTER_RAGE,"Data\\Sound\\w69w70w71w72\\Jelint_rage.wav");
		LoadWaveFile(SOUND_EMPIREGUARDIAN_JERINT_MONSTER_DEATH,"Data\\Sound\\w69w70w71w72\\Jelint_death.wav");
		LoadWaveFile(SOUND_EMPIREGUARDIAN_RAYMOND_MONSTER_ATTACK02,"Data\\Sound\\w69w70w71w72\\Raymond_attack2.wav");
		LoadWaveFile(SOUND_EMPIREGUARDIAN_RAYMOND_MONSTER_RAGE,"Data\\Sound\\w69w70w71w72\\Raymond_rage.wav");
		LoadWaveFile(SOUND_EMPIREGUARDIAN_ERCANNE_MONSTER_ATTACK03,"Data\\Sound\\w69w70w71w72\\Ercanne_attack3.wav");
		LoadWaveFile(SOUND_EMPIREGUARDIAN_1CORP_DEASULER_MONSTER_ATTACK02,"Data\\Sound\\w69w70w71w72\\1Deasuler_attack2.wav");
		LoadWaveFile(SOUND_EMPIREGUARDIAN_1CORP_DEASULER_MONSTER_ATTACK03,"Data\\Sound\\w69w70w71w72\\1Deasuler_attack3.wav");
		LoadWaveFile(SOUND_EMPIREGUARDIAN_2CORP_VERMONT_MONSTER_ATTACK01,"Data\\Sound\\w69w70w71w72\\2Vermont_attack1.wav");
		LoadWaveFile(SOUND_EMPIREGUARDIAN_2CORP_VERMONT_MONSTER_ATTACK02,"Data\\Sound\\w69w70w71w72\\2Vermont_attack2.wav");
		LoadWaveFile(SOUND_EMPIREGUARDIAN_2CORP_VERMONT_MONSTER_DEATH,"Data\\Sound\\w69w70w71w72\\2Vermont_death.wav");
		LoadWaveFile(SOUND_EMPIREGUARDIAN_3CORP_CATO_ATTACK02,"Data\\Sound\\w69w70w71w72\\3Cato_attack2.wav");
		LoadWaveFile(SOUND_EMPIREGUARDIAN_3CORP_CATO_MOVE,"Data\\Sound\\w69w70w71w72\\3Cato_move.wav");
		LoadWaveFile(SOUND_EMPIREGUARDIAN_4CORP_GALLIA_ATTACK02,"Data\\Sound\\w69w70w71w72\\4Gallia_attack2.wav");
		LoadWaveFile(SOUND_EMPIREGUARDIAN_QUATERMASTER_ATTACK02,"Data\\Sound\\w69w70w71w72\\QuaterMaster_attack2.wav");
		LoadWaveFile(SOUND_EMPIREGUARDIAN_COMBATMASTER_ATTACK01,"Data\\Sound\\w69w70w71w72\\CombatMaster_attack1.wav");
		LoadWaveFile(SOUND_EMPIREGUARDIAN_COMBATMASTER_ATTACK02,"Data\\Sound\\w69w70w71w72\\CombatMaster_attack2.wav");
		LoadWaveFile(SOUND_EMPIREGUARDIAN_COMBATMASTER_ATTACK03,"Data\\Sound\\w69w70w71w72\\CombatMaster_attack3.wav");
		LoadWaveFile(SOUND_EMPIREGUARDIAN_GRANDWIZARD_DEATH,"Data\\Sound\\w69w70w71w72\\GrandWizard_death.wav");
		LoadWaveFile(SOUND_EMPIREGUARDIAN_ASSASSINMASTER_DEATH,"Data\\Sound\\w69w70w71w72\\AssassinMaster_Death.wav");
		LoadWaveFile(SOUND_EMPIREGUARDIAN_CAVALRYLEADER_ATTACK01,"Data\\Sound\\w69w70w71w72\\CavalryLeader_attack1.wav");
		LoadWaveFile(SOUND_EMPIREGUARDIAN_CAVALRYLEADER_ATTACK02,"Data\\Sound\\w69w70w71w72\\CavalryLeader_attack2.wav");
		LoadWaveFile(SOUND_EMPIREGUARDIAN_CAVALRYLEADER_MOVE01,"Data\\Sound\\w69w70w71w72\\CavalryLeader_move01.wav");
		LoadWaveFile(SOUND_EMPIREGUARDIAN_CAVALRYLEADER_MOVE02,"Data\\Sound\\w69w70w71w72\\CavalryLeader_move02.wav");
		LoadWaveFile(SOUND_EMPIREGUARDIAN_DEFENDER_ATTACK02,"Data\\Sound\\w69w70w71w72\\GrandWizard_death.wav");
		LoadWaveFile(SOUND_EMPIREGUARDIAN_PRIEST_STOP,"Data\\Sound\\w69w70w71w72\\Priest_stay.wav");
		break;
	case WD_73NEW_LOGIN_SCENE:
	case WD_74NEW_CHARACTER_SCENE:
		{
			LoadBitmap("Logo\\MU-logo.tga", BITMAP_LOG_IN+16, GL_LINEAR);
			LoadBitmap("Logo\\MU-logo_g.jpg", BITMAP_LOG_IN+17, GL_LINEAR);
		}
		break;
	case WD_55LOGINSCENE:
		gLoadData.AccessModel(MODEL_DRAGON	 ,"Data\\Object56\\","Dragon");
		gLoadData.OpenTexture(MODEL_DRAGON	 ,"Object56\\");

		Vector ( 0.f, 0.f, 0.f, Ang );
		Vector ( 0.f, 0.f, 0.f, Pos );
		Pos[0] = 56*TERRAIN_SCALE; Pos[1] = 230*TERRAIN_SCALE;
		CreateObject ( MODEL_DRAGON, Pos, Ang );
		break;

#ifdef ASG_ADD_KARUTAN_MONSTERS
	case WD_80KARUTAN1:
	case WD_81KARUTAN2:
		LoadBitmap("Object9\\sand02.jpg", BITMAP_CHROME+3, GL_LINEAR, GL_REPEAT);
		LoadBitmap("Effect\\clouds.jpg", BITMAP_CLOUD, GL_LINEAR, GL_CLAMP_TO_EDGE);

		gLoadData.AccessModel(MODEL_CONDRA_ARM_L	, "Data\\Monster\\", "condra_7_arm_left");
		gLoadData.OpenTexture(MODEL_CONDRA_ARM_L	, "Monster\\");
		gLoadData.AccessModel(MODEL_CONDRA_ARM_L2	, "Data\\Monster\\", "condra_7_arm_left_2");
		gLoadData.OpenTexture(MODEL_CONDRA_ARM_L2	, "Monster\\");
		gLoadData.AccessModel(MODEL_CONDRA_SHOULDER, "Data\\Monster\\", "condra_7_shoulder_right");
		gLoadData.OpenTexture(MODEL_CONDRA_SHOULDER, "Monster\\");
		gLoadData.AccessModel(MODEL_CONDRA_ARM_R	, "Data\\Monster\\", "condra_7_arm_right");
		gLoadData.OpenTexture(MODEL_CONDRA_ARM_R	, "Monster\\");
		gLoadData.AccessModel(MODEL_CONDRA_ARM_R2	, "Data\\Monster\\", "condra_7_arm_right_2");
		gLoadData.OpenTexture(MODEL_CONDRA_ARM_R2	, "Monster\\");
		gLoadData.AccessModel(MODEL_CONDRA_CONE_L	, "Data\\Monster\\", "condra_7_cone_left");
		gLoadData.OpenTexture(MODEL_CONDRA_CONE_L	, "Monster\\");
		gLoadData.AccessModel(MODEL_CONDRA_CONE_R	, "Data\\Monster\\", "condra_7_cone_right");
		gLoadData.OpenTexture(MODEL_CONDRA_CONE_R	, "Monster\\");
		gLoadData.AccessModel(MODEL_CONDRA_PELVIS	, "Data\\Monster\\", "condra_7_pelvis");
		gLoadData.OpenTexture(MODEL_CONDRA_PELVIS	, "Monster\\");
		gLoadData.AccessModel(MODEL_CONDRA_STOMACH, "Data\\Monster\\", "condra_7_stomach");
		gLoadData.OpenTexture(MODEL_CONDRA_STOMACH, "Monster\\");
		gLoadData.AccessModel(MODEL_CONDRA_NECK	, "Data\\Monster\\", "condra_7_neck");
		gLoadData.OpenTexture(MODEL_CONDRA_NECK	, "Monster\\");
		gLoadData.AccessModel(MODEL_NARCONDRA_ARM_L	, "Data\\Monster\\", "nar_condra_7_arm_left");
		gLoadData.OpenTexture(MODEL_NARCONDRA_ARM_L	, "Monster\\");
		gLoadData.AccessModel(MODEL_NARCONDRA_ARM_L2	, "Data\\Monster\\", "nar_condra_7_arm_left_2");
		gLoadData.OpenTexture(MODEL_NARCONDRA_ARM_L2	, "Monster\\");
		gLoadData.AccessModel(MODEL_NARCONDRA_SHOULDER_L, "Data\\Monster\\", "nar_condra_7_shoulder_left");
		gLoadData.OpenTexture(MODEL_NARCONDRA_SHOULDER_L, "Monster\\");
		gLoadData.AccessModel(MODEL_NARCONDRA_SHOULDER_R, "Data\\Monster\\", "nar_condra_7_shoulder_right");
		gLoadData.OpenTexture(MODEL_NARCONDRA_SHOULDER_R, "Monster\\");
		gLoadData.AccessModel(MODEL_NARCONDRA_ARM_R	, "Data\\Monster\\", "nar_condra_7_arm_right");
		gLoadData.OpenTexture(MODEL_NARCONDRA_ARM_R	, "Monster\\");
		gLoadData.AccessModel(MODEL_NARCONDRA_ARM_R2	, "Data\\Monster\\", "nar_condra_7_arm_right_2");
		gLoadData.OpenTexture(MODEL_NARCONDRA_ARM_R2	, "Monster\\");
		gLoadData.AccessModel(MODEL_NARCONDRA_ARM_R3	, "Data\\Monster\\", "nar_condra_7_arm_right_3");
		gLoadData.OpenTexture(MODEL_NARCONDRA_ARM_R3	, "Monster\\");
		gLoadData.AccessModel(MODEL_NARCONDRA_CONE_1	, "Data\\Monster\\", "nar_condra_7_cone_1");
		gLoadData.OpenTexture(MODEL_NARCONDRA_CONE_1	, "Monster\\");
		gLoadData.AccessModel(MODEL_NARCONDRA_CONE_2	, "Data\\Monster\\", "nar_condra_7_cone_2");
		gLoadData.OpenTexture(MODEL_NARCONDRA_CONE_2	, "Monster\\");
		gLoadData.AccessModel(MODEL_NARCONDRA_CONE_3	, "Data\\Monster\\", "nar_condra_7_cone_3");
		gLoadData.OpenTexture(MODEL_NARCONDRA_CONE_3	, "Monster\\");
		gLoadData.AccessModel(MODEL_NARCONDRA_CONE_4	, "Data\\Monster\\", "nar_condra_7_cone_4");
		gLoadData.OpenTexture(MODEL_NARCONDRA_CONE_4	, "Monster\\");
		gLoadData.AccessModel(MODEL_NARCONDRA_CONE_5	, "Data\\Monster\\", "nar_condra_7_cone_5");
		gLoadData.OpenTexture(MODEL_NARCONDRA_CONE_5	, "Monster\\");
		gLoadData.AccessModel(MODEL_NARCONDRA_CONE_6	, "Data\\Monster\\", "nar_condra_7_cone_6");
		gLoadData.OpenTexture(MODEL_NARCONDRA_CONE_6	, "Monster\\");
		gLoadData.AccessModel(MODEL_NARCONDRA_PELVIS	, "Data\\Monster\\", "nar_condra_7_pelvis");
		gLoadData.OpenTexture(MODEL_NARCONDRA_PELVIS	, "Monster\\");
		gLoadData.AccessModel(MODEL_NARCONDRA_STOMACH	, "Data\\Monster\\", "nar_condra_7_stomach");
		gLoadData.OpenTexture(MODEL_NARCONDRA_STOMACH	, "Monster\\");
		gLoadData.AccessModel(MODEL_NARCONDRA_NECK	, "Data\\Monster\\", "nar_condra_7_neck");
		gLoadData.OpenTexture(MODEL_NARCONDRA_NECK	, "Monster\\");

		LoadWaveFile(SOUND_KARUTAN_DESERT_ENV, "Data\\Sound\\Karutan\\Karutan_desert_env.wav", 1 );
		LoadWaveFile(SOUND_KARUTAN_INSECT_ENV, "Data\\Sound\\Karutan\\Karutan_insect_env.wav", 1 );
		LoadWaveFile(SOUND_KARUTAN_KARDAMAHAL_ENV, "Data\\Sound\\Karutan\\Kardamahal_entrance_env.wav", 1 );
		break;
#endif	// ASG_ADD_KARUTAN_MONSTERS
	}

    if ( this->InChaosCastle()==true )
    {
        gLoadData.AccessModel ( MODEL_ANGEL, "Data\\Player\\", "Angel" );
        gLoadData.OpenTexture ( MODEL_ANGEL, "Npc\\" );

		LoadBitmap("Effect\\clouds.jpg"	  ,BITMAP_CLOUD,GL_LINEAR,GL_CLAMP_TO_EDGE);

		LoadWaveFile ( SOUND_CHAOSCASTLE, "Data\\Sound\\iChaosCastle.wav" , 1 );
    }
	
    if ( this->InHellas() )
    {
		LoadBitmap( "Object25\\water1.tga", BITMAP_MAPTILE, GL_LINEAR, GL_REPEAT,false);
		LoadBitmap( "Object25\\water2.jpg", BITMAP_MAPTILE+1, GL_NEAREST, GL_CLAMP_TO_EDGE );

        gLoadData.AccessModel(MODEL_CUNDUN_PART1		 ,"Data\\Monster\\","cd71a",-1);
        gLoadData.OpenTexture(MODEL_CUNDUN_PART1		 ,"Monster\\");
        gLoadData.AccessModel(MODEL_CUNDUN_PART2		 ,"Data\\Monster\\","cd71b",-1);
        gLoadData.OpenTexture(MODEL_CUNDUN_PART2		 ,"Monster\\");
        gLoadData.AccessModel(MODEL_CUNDUN_PART3		 ,"Data\\Monster\\","cd71c",-1);
        gLoadData.OpenTexture(MODEL_CUNDUN_PART3		 ,"Monster\\");
        gLoadData.AccessModel(MODEL_CUNDUN_PART4		 ,"Data\\Monster\\","cd71d",-1);
        gLoadData.OpenTexture(MODEL_CUNDUN_PART4		 ,"Monster\\");
        gLoadData.AccessModel(MODEL_CUNDUN_PART5		 ,"Data\\Monster\\","cd71e",-1);
        gLoadData.OpenTexture(MODEL_CUNDUN_PART5		 ,"Monster\\");
        gLoadData.AccessModel(MODEL_CUNDUN_PART6		 ,"Data\\Monster\\","cd71f",-1);
        gLoadData.OpenTexture(MODEL_CUNDUN_PART6		 ,"Monster\\");
        gLoadData.AccessModel(MODEL_CUNDUN_PART7		 ,"Data\\Monster\\","cd71g",-1);
        gLoadData.OpenTexture(MODEL_CUNDUN_PART7		 ,"Monster\\");
        gLoadData.AccessModel(MODEL_CUNDUN_PART8		 ,"Data\\Monster\\","cd71h",-1);
        gLoadData.OpenTexture(MODEL_CUNDUN_PART8		 ,"Monster\\");
        gLoadData.AccessModel(MODEL_CUNDUN_DRAGON_HEAD ,"Data\\Skill\\","dragonhead",-1);
        gLoadData.OpenTexture(MODEL_CUNDUN_DRAGON_HEAD ,"Skill\\");
        gLoadData.AccessModel(MODEL_CUNDUN_PHOENIX	 ,"Data\\Skill\\","phoenix",-1);
        gLoadData.OpenTexture(MODEL_CUNDUN_PHOENIX	 ,"Skill\\");
        gLoadData.AccessModel(MODEL_CUNDUN_GHOST		 ,"Data\\Monster\\","cundun_gone",-1);
        gLoadData.OpenTexture(MODEL_CUNDUN_GHOST		 ,"Monster\\");
        Models[MODEL_CUNDUN_PART6].Actions[0].Loop = false;
        Models[MODEL_CUNDUN_PART6].Actions[0].PlaySpeed = 0.13f;
        Models[MODEL_CUNDUN_PART7].Actions[0].Loop = false;
        Models[MODEL_CUNDUN_PART7].Actions[0].PlaySpeed = 0.13f;
        Models[MODEL_CUNDUN_PHOENIX].Actions[0].Loop = true;
        Models[MODEL_CUNDUN_GHOST].Actions[0].Loop = false;

        LoadWaveFile ( SOUND_KALIMA_AMBIENT,        "Data\\Sound\\aKalima.wav", 1 );
        LoadWaveFile ( SOUND_KALIMA_AMBIENT2,       "Data\\Sound\\aKalima01.wav", 1 );
        LoadWaveFile ( SOUND_KALIMA_AMBIENT3,       "Data\\Sound\\aKalima02.wav", 1 );
        LoadWaveFile ( SOUND_KALIMA_WATER_FALL,     "Data\\Sound\\aKalimaWaterFall.wav", 3 );
        LoadWaveFile ( SOUND_KALIMA_FALLING_STONE,  "Data\\Sound\\aKalimaStone.wav", 3 );
        LoadWaveFile ( SOUND_DEATH_BUBBLE,          "Data\\Sound\\mDeathBubble.wav", 1 );

        LoadWaveFile ( SOUND_KUNDUN_AMBIENT1,       "Data\\Sound\\mKundunAmbient1.wav", 1 );
        LoadWaveFile ( SOUND_KUNDUN_AMBIENT2,       "Data\\Sound\\mKundunAmbient2.wav", 1 );
        LoadWaveFile ( SOUND_KUNDUN_ROAR,           "Data\\Sound\\mKundunRoar.wav", 1 );
        LoadWaveFile ( SOUND_KUNDUN_SIGHT,          "Data\\Sound\\mKundunSight.wav", 1 );
        LoadWaveFile ( SOUND_KUNDUN_SHUDDER,        "Data\\Sound\\mKundunShudder.wav", 1 );
        LoadWaveFile ( SOUND_KUNDUN_DESTROY,        "Data\\Sound\\mKundunDestory.wav", 1 );

        LoadWaveFile ( SOUND_SKILL_SKULL,           "Data\\Sound\\eSkull.wav", 1 );
        LoadWaveFile ( SOUND_GREAT_POISON,          "Data\\Sound\\eGreatPoison.wav", 1 );
        LoadWaveFile ( SOUND_GREAT_SHIELD,          "Data\\Sound\\eGreatShield.wav", 1 );
    }

    if ( this->WorldActive!=WD_7ATLANSE )
    {
	    for ( int i=0; i<32; i++ )
	    {
		    char FileName[100];
		    if(i<10)
      		    sprintf(FileName,"Object8\\wt0%d.jpg",i);
		    else
      		    sprintf(FileName,"Object8\\wt%d.jpg",i);
			
			LoadBitmap(FileName,BITMAP_WATER+i,GL_LINEAR,GL_REPEAT,false);
		}
    }
	
	LoadBitmap( "Object8\\light01.jpg", BITMAP_SHINES, GL_LINEAR, GL_REPEAT);
	
	if(this->WorldActive==0)
	{
		for(i=0;i<13;i++)
			gLoadData.AccessModel(MODEL_TREE01 +i,"Data\\Object1\\","Tree",i+1);
		for(i=0;i<8;i++)
			gLoadData.AccessModel(MODEL_GRASS01+i,"Data\\Object1\\","Grass",i+1);
		for(i=0;i<5;i++)
			gLoadData.AccessModel(MODEL_STONE01+i,"Data\\Object1\\","Stone",i+1);
		
		for(i=0;i<3;i++)
			gLoadData.AccessModel(MODEL_STONE_STATUE01+i,"Data\\Object1\\","StoneStatue",i+1);
		gLoadData.AccessModel(MODEL_STEEL_STATUE,"Data\\Object1\\","SteelStatue",1);
		for(i=0;i<3;i++)
			gLoadData.AccessModel(MODEL_TOMB01+i,"Data\\Object1\\","Tomb",i+1);
		for(i=0;i<2;i++)
			gLoadData.AccessModel(MODEL_FIRE_LIGHT01+i,"Data\\Object1\\","FireLight",i+1);
		gLoadData.AccessModel(MODEL_BONFIRE,"Data\\Object1\\","Bonfire",1);
		gLoadData.AccessModel(MODEL_DUNGEON_GATE,"Data\\Object1\\","DoungeonGate",1);
		gLoadData.AccessModel(MODEL_TREASURE_DRUM ,"Data\\Object1\\","TreasureDrum",1);
		gLoadData.AccessModel(MODEL_TREASURE_CHEST,"Data\\Object1\\","TreasureChest",1);
		gLoadData.AccessModel(MODEL_SHIP,"Data\\Object1\\","Ship",1);
		
		for(i=0;i<6;i++)
			gLoadData.AccessModel(MODEL_STONE_WALL01+i,"Data\\Object1\\","StoneWall",i+1);
		for(i=0;i<4;i++)
			gLoadData.AccessModel(MODEL_MU_WALL01+i,"Data\\Object1\\","StoneMuWall",i+1);
		for(i=0;i<3;i++)
			gLoadData.AccessModel(MODEL_STEEL_WALL01+i,"Data\\Object1\\","SteelWall",i+1);
		gLoadData.AccessModel(MODEL_STEEL_DOOR,"Data\\Object1\\","SteelDoor",1);
		for(i=0;i<3;i++)
			gLoadData.AccessModel(MODEL_CANNON01+i,"Data\\Object1\\","Cannon",i+1);
		gLoadData.AccessModel(MODEL_BRIDGE,"Data\\Object1\\","Bridge",1);
		for(i=0;i<4;i++)
			gLoadData.AccessModel(MODEL_FENCE01+i,"Data\\Object1\\","Fence",i+1);
		gLoadData.AccessModel(MODEL_BRIDGE_STONE,"Data\\Object1\\","BridgeStone",1);
		
		gLoadData.AccessModel(MODEL_STREET_LIGHT,"Data\\Object1\\","StreetLight",1);
		gLoadData.AccessModel(MODEL_CURTAIN,"Data\\Object1\\","Curtain",1);
		for(i=0;i<4;i++)
			gLoadData.AccessModel(MODEL_CARRIAGE01+i,"Data\\Object1\\","Carriage",i+1);
		for(i=0;i<2;i++)
			gLoadData.AccessModel(MODEL_STRAW01+i,"Data\\Object1\\","Straw",i+1);
		for(i=0;i<2;i++)
			gLoadData.AccessModel(MODEL_SIGN01+i,"Data\\Object1\\","Sign",i+1);
		for(i=0;i<2;i++)
			gLoadData.AccessModel(MODEL_MERCHANT_ANIMAL01  +i,"Data\\Object1\\","MerchantAnimal",i+1);
		gLoadData.AccessModel(MODEL_WATERSPOUT,"Data\\Object1\\","Waterspout",1);
		for(i=0;i<4;i++)
			gLoadData.AccessModel(MODEL_WELL01+i,"Data\\Object1\\","Well",i+1);
		gLoadData.AccessModel(MODEL_HANGING,"Data\\Object1\\","Hanging",1);
		
		for(i=0;i<5;i++)
			gLoadData.AccessModel(MODEL_HOUSE01+i,"Data\\Object1\\","House",i+1);
		gLoadData.AccessModel(MODEL_TENT,"Data\\Object1\\","Tent",1);
		gLoadData.AccessModel(MODEL_STAIR,"Data\\Object1\\","Stair",1);
		
		for(i=0;i<6;i++)
			gLoadData.AccessModel(MODEL_HOUSE_WALL01+i,"Data\\Object1\\","HouseWall",i+1);
		for(i=0;i<3;i++)
			gLoadData.AccessModel(MODEL_HOUSE_ETC01+i,"Data\\Object1\\","HouseEtc",i+1);
		for(i=0;i<3;i++)
			gLoadData.AccessModel(MODEL_LIGHT01+i,"Data\\Object1\\","Light",i+1);
		gLoadData.AccessModel(MODEL_POSE_BOX,"Data\\Object1\\","PoseBox",1);
		
		for(i=0;i<7;i++)
			gLoadData.AccessModel(MODEL_FURNITURE01+i,"Data\\Object1\\","Furniture",i+1);
		gLoadData.AccessModel(MODEL_CANDLE,"Data\\Object1\\","Candle",1);
		for(i=0;i<3;i++)
			gLoadData.AccessModel(MODEL_BEER01+i,"Data\\Object1\\","Beer",i+1);

		for(i=MODEL_WORLD_OBJECT;i<MAX_WORLD_OBJECTS;i++)
			gLoadData.OpenTexture(i,"Object1\\" );
    }
    else
	{
		char DirName[32];
        int iMapWorld = this->WorldActive+1;

        if ( this->InBloodCastle() == true )
        {
            iMapWorld = WD_11BLOODCASTLE1+1;
        }
        else if ( this->InChaosCastle() == true )
        {
            iMapWorld = WD_18CHAOS_CASTLE+1;
        }
        else if ( this->InHellas() == true )
        {
            iMapWorld = WD_24HELLAS+1;
        }
		else if( gMapManager.IsCursedTemple() )
		{
			iMapWorld = WD_45CURSEDTEMPLE_LV1 + 2;
		}

		sprintf(DirName,"Data\\Object%d\\",iMapWorld);
		for(i=MODEL_WORLD_OBJECT;i<MAX_WORLD_OBJECTS;i++)
			gLoadData.AccessModel(i,DirName,"Object",i+1);

		sprintf(DirName,"Object%d\\",iMapWorld);
		for(i=MODEL_WORLD_OBJECT;i<MAX_WORLD_OBJECTS;i++)
		{
			gLoadData.OpenTexture( i, DirName );
		}
		
		if(this->WorldActive == WD_1DUNGEON)
		{
			Models[40].Actions[1].PlaySpeed = 0.4f;
		}
        else if(this->WorldActive == WD_8TARKAN)
        {
            Models[11].StreamMesh = 0;
            Models[12].StreamMesh = 0;
            Models[13].StreamMesh = 0;
            Models[73].StreamMesh = 0;
            Models[75].StreamMesh = 0;
            Models[79].StreamMesh = 0;
        }
		if (this->WorldActive == WD_51HOME_6TH_CHAR 
#ifndef PJH_NEW_SERVER_SELECT_MAP
			|| this->WorldActive == WD_77NEW_LOGIN_SCENE 
			|| this->WorldActive == WD_78NEW_CHARACTER_SCENE
#endif //PJH_NEW_SERVER_SELECT_MAP
			)
		{
			Models[MODEL_EAGLE].Actions[0].PlaySpeed = 0.5f;
			if (this->WorldActive == WD_51HOME_6TH_CHAR)
			{
				Models[MODEL_MAP_TORNADO].Actions[0].PlaySpeed = 0.1f;
			}
		}
		else if(this->WorldActive == WD_57ICECITY)
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
	if(Map == 32 && this->WorldActive == 32)
	{
		Map = this->WorldActive = 9;
	}

	this->DeleteObjects();
	DeleteNpcs();
	DeleteMonsters();
	if(SceneFlag != CHARACTER_SCENE)
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

	char FileName[64];
	char WorldName[32];
    int  iMapWorld = this->WorldActive+1;

    if ( InBloodCastle() == true )
    {
        iMapWorld = WD_11BLOODCASTLE1+1;
    }
    else if ( gMapManager.InHellas() == true )
    {
        iMapWorld = WD_24HELLAS+1;
    }
    else if ( InChaosCastle()==true )
    {
        iMapWorld = WD_18CHAOS_CASTLE+1;
    }
	else if( gMapManager.IsCursedTemple() )
	{
		iMapWorld = WD_45CURSEDTEMPLE_LV1 + 2;
	}

    battleCastle::Init ();

	sprintf ( WorldName, "World%d", iMapWorld );
	{
#if defined DEVIAS_XMAS_END_LOADFIX_FOR_TESTSERVER
		if(World == WD_2DEVIAS)
		{
			sprintf ( FileName, "Data\\%s\\EncTerraintest%d.map", WorldName, iMapWorld);
		}
		else
#endif	// defined DEVIAS_XMAS_END_LOADFIX_FOR_TESTSERVER
#ifdef ASG_ADD_AIDA_KARUTAN_GATE_FOR_TESTSERVER
		if (World == WD_33AIDA)
			sprintf(FileName, "Data\\%s\\EncTerraintest%d.map", WorldName, iMapWorld);
		else
#endif	// ASG_ADD_AIDA_KARUTAN_GATE_FOR_TESTSERVER
#ifdef CHAOSCASTLE_MAP_FOR_TESTSERVER
		if (InChaosCastle())
		{
			sprintf ( FileName, "Data\\%s\\EncTerraintest%d.map", WorldName, iMapWorld);
		}
		else
#endif	// CHAOSCASTLE_MAP_FOR_TESTSERVER
#ifdef KJH_EXTENSIONMAP_FOR_TESTSERVER
		if( (World == WD_33AIDA) || (World == WD_37KANTURU_1ST) || (World == WD_56MAP_SWAMP_OF_QUIET) || (World == WD_57ICECITY))
		{
			sprintf ( FileName, "Data\\%s\\EncTerraintest%d.map", WorldName, iMapWorld);
		}
		else
#endif // KJH_EXTENSIONMAP_FOR_TESTSERVER
#ifdef KJH_FIX_ICARUS_MAP_ATTRIBUTE_FOR_TESTSERVER
		if( (World == WD_10HEAVEN) )
		{
			sprintf ( FileName, "Data\\%s\\EncTerraintest%d.map", WorldName, iMapWorld);
		}
		else
#endif // KJH_FIX_ICARUS_MAP_ATTRIBUTE_FOR_TESTSERVER
		{
			sprintf ( FileName, "Data\\%s\\EncTerrain%d.map", WorldName, iMapWorld);
		}
	}

	int iResult = OpenTerrainMapping ( FileName );

	if ( iMapWorld != iResult && -1 != iResult)
	{
		char Text[256];
   		sprintf(Text,"%s file corrupted.",FileName);
		g_ErrorReport.Write( Text);
		g_ErrorReport.Write( "\r\n");
		MessageBox(g_hWnd,Text,NULL,MB_OK);
		SendMessage(g_hWnd,WM_DESTROY,0,0);
		return;
	}

#ifdef PJH_NEW_SERVER_SELECT_MAP
	if (this->WorldActive == WD_73NEW_LOGIN_SCENE)
#else
	if (this->WorldActive == WD_77NEW_LOGIN_SCENE)
#endif //PJH_NEW_SERVER_SELECT_MAP
	{
		sprintf(FileName,"Data\\%s\\CWScript%d.cws",WorldName, iMapWorld);
		CCameraMove::GetInstancePtr()->LoadCameraWalkScript(FileName);
	}

	int CryWolfState = M34CryWolf1st::IsCryWolf1stMVPStart();

    if ( gMapManager.InBattleCastle() )
    {
        if ( battleCastle::IsBattleCastleStart() )
        {
	        sprintf ( FileName, "Data\\%s\\EncTerrain%d.att", WorldName, iMapWorld*10+2 );
        }
        else
        {
	        sprintf ( FileName, "Data\\%s\\EncTerrain%d.att", WorldName, iMapWorld );
        }
    }
    else

	if (this->WorldActive==WD_34CRYWOLF_1ST)
	{
		switch( CryWolfState )
		{
		case CRYWOLF_OCCUPATION_STATE_PEACE:
			sprintf ( FileName, "Data\\%s\\EncTerrain%d.att", WorldName, iMapWorld);
			break;
		case CRYWOLF_OCCUPATION_STATE_OCCUPIED:
			sprintf ( FileName, "Data\\%s\\EncTerrain%d.att", WorldName, iMapWorld*10+1);
			break;
		case CRYWOLF_OCCUPATION_STATE_WAR:
			sprintf ( FileName, "Data\\%s\\EncTerrain%d.att", WorldName, iMapWorld*10+2);
			break;
		}
	}
	else if (this->WorldActive == WD_39KANTURU_3RD)
	{
		if(M39Kanturu3rd::IsSuccessBattle())
		{
			sprintf ( FileName, "Data\\%s\\EncTerrain%d.att", WorldName, iMapWorld*10+1);
		}
		else
		{
			sprintf ( FileName, "Data\\%s\\EncTerrain%d.att", WorldName, iMapWorld);
		}
	}
	else
    {
		sprintf ( FileName, "Data\\%s\\EncTerrain%d.att", WorldName, iMapWorld );   
    }
	iResult = OpenTerrainAttribute ( FileName);
	if ( iMapWorld != iResult && -1 != iResult)
	{
		char Text[256];
   		sprintf(Text,"%s file corrupted.",FileName);
		g_ErrorReport.Write( Text);
		g_ErrorReport.Write( "\r\n");
		MessageBox(g_hWnd,Text,NULL,MB_OK);
		SendMessage(g_hWnd,WM_DESTROY,0,0);
		return;
	}

	sprintf(FileName,"Data\\%s\\EncTerrain%d.obj",WorldName, iMapWorld);

	iResult = OpenObjectsEnc ( FileName);
	if ( iMapWorld != iResult && -1 != iResult)
	{
		char Text[256];
   		sprintf(Text,"%s file corrupted.",FileName);
		g_ErrorReport.Write( Text);
		g_ErrorReport.Write( "\r\n");
		MessageBox(g_hWnd,Text,NULL,MB_OK);
		SendMessage(g_hWnd,WM_DESTROY,0,0);
		return;
	}

	sprintf(FileName,"%s\\TerrainHeight.bmp",WorldName);
	if(IsTerrainHeightExtMap(this->WorldActive) == true)
	{
		CreateTerrain(FileName, true);
	}
	else
	{
		CreateTerrain(FileName);
	}

    if ( gMapManager.InBattleCastle() )
    {
        if ( battleCastle::IsBattleCastleStart() )
        {
    	    sprintf(FileName,"%s\\TerrainLight2.jpg",WorldName);
        }
        else
        {
	        sprintf(FileName,"%s\\TerrainLight.jpg",WorldName);
        }
    }
    else
	if (this->WorldActive==WD_34CRYWOLF_1ST)
	{
		switch( CryWolfState )
		{
		case CRYWOLF_OCCUPATION_STATE_PEACE:
			sprintf ( FileName, "%s\\TerrainLight.jpg", WorldName);
			break;
		case CRYWOLF_OCCUPATION_STATE_OCCUPIED:
			sprintf ( FileName, "%s\\TerrainLight1.jpg", WorldName);
			break;
		case CRYWOLF_OCCUPATION_STATE_WAR:
			sprintf ( FileName, "%s\\TerrainLight2.jpg", WorldName);
			break;
		}
	}
	else
	{
	    sprintf(FileName,"%s\\TerrainLight.jpg",WorldName);
	}

	OpenTerrainLight(FileName);

    if ( CreateWaterTerrain( this->WorldActive )==false )
    {
	    sprintf (FileName,"%s\\TileGrass01.jpg",WorldName);
	    LoadBitmap(FileName,BITMAP_MAPTILE   ,GL_NEAREST,GL_REPEAT, false);
	    sprintf (FileName,"%s\\TileGrass02.jpg",WorldName);
	    LoadBitmap(FileName,BITMAP_MAPTILE+1 ,GL_NEAREST,GL_REPEAT, false);
		if(this->WorldActive == WD_51HOME_6TH_CHAR 
#ifndef PJH_NEW_SERVER_SELECT_MAP
			|| World == WD_77NEW_LOGIN_SCENE 
			|| World == WD_78NEW_CHARACTER_SCENE
#endif //PJH_NEW_SERVER_SELECT_MAP
			)
		{
			sprintf (FileName,"%s\\AlphaTileGround01.Tga",WorldName);
			LoadBitmap(FileName,BITMAP_MAPTILE+2  ,GL_NEAREST,GL_REPEAT,false);
		}
		else
		{
			sprintf (FileName,"%s\\TileGround01.jpg",WorldName);
			LoadBitmap(FileName,BITMAP_MAPTILE+2 ,GL_NEAREST,GL_REPEAT, false);
		}
		
		if(this->WorldActive == WD_39KANTURU_3RD)
		{
			sprintf (FileName,"%s\\AlphaTileGround02.Tga",WorldName);
			LoadBitmap(FileName,BITMAP_MAPTILE+3  ,GL_NEAREST,GL_REPEAT,false);
		}
		else
		{
			sprintf (FileName,"%s\\TileGround02.jpg",WorldName);
			LoadBitmap(FileName,BITMAP_MAPTILE+3 ,GL_NEAREST,GL_REPEAT,false);
		}
		if( gMapManager.IsCursedTemple() )
		{
			sprintf (FileName,"%s\\AlphaTileGround03.Tga",WorldName);
			LoadBitmap(FileName,BITMAP_MAPTILE+4  ,GL_NEAREST,GL_REPEAT,false);
		}
		else
		{
			sprintf (FileName,"%s\\TileGround03.jpg",WorldName);
			LoadBitmap(FileName,BITMAP_MAPTILE+4 ,GL_NEAREST,GL_REPEAT,false);
		}

	    sprintf (FileName,"%s\\TileWater01.jpg",WorldName);
	    LoadBitmap(FileName,BITMAP_MAPTILE+5 ,GL_NEAREST,GL_REPEAT,false);
	    sprintf (FileName,"%s\\TileWood01.jpg",WorldName);
	    LoadBitmap(FileName,BITMAP_MAPTILE+6 ,GL_NEAREST,GL_REPEAT,false);
	    sprintf (FileName,"%s\\TileRock01.jpg",WorldName);
	    LoadBitmap(FileName,BITMAP_MAPTILE+7 ,GL_NEAREST,GL_REPEAT,false);
	    sprintf (FileName,"%s\\TileRock02.jpg",WorldName);
	    LoadBitmap(FileName,BITMAP_MAPTILE+8 ,GL_NEAREST,GL_REPEAT,false);
	    sprintf (FileName,"%s\\TileRock03.jpg",WorldName);
	    LoadBitmap(FileName,BITMAP_MAPTILE+9 ,GL_NEAREST,GL_REPEAT,false);
#ifdef PJH_NEW_SERVER_SELECT_MAP
		if(this->WorldActive == WD_73NEW_LOGIN_SCENE || this->WorldActive == WD_74NEW_CHARACTER_SCENE)
		{
			sprintf(FileName,"%s\\AlphaTile01.Tga",WorldName);
			LoadBitmap(FileName,BITMAP_MAPTILE+10,GL_NEAREST,GL_REPEAT,false);
		}
		else
		{
#endif //PJH_NEW_SERVER_SELECT_MAP
	    sprintf (FileName,"%s\\TileRock04.jpg",WorldName);
	    LoadBitmap(FileName,BITMAP_MAPTILE+10,GL_NEAREST,GL_REPEAT,false);
#ifdef PJH_NEW_SERVER_SELECT_MAP
		}
#endif //PJH_NEW_SERVER_SELECT_MAP
		if(IsPKField() || IsDoppelGanger2())
		{

			LoadBitmap("Object64\\song_lava1.jpg", BITMAP_MAPTILE+11,GL_NEAREST,GL_REPEAT,false);
		}
		else
		{
			sprintf (FileName,"%s\\TileRock05.jpg",WorldName);
			LoadBitmap(FileName,BITMAP_MAPTILE+11,GL_NEAREST,GL_REPEAT,false);
		}
#ifdef ASG_ADD_MAP_KARUTAN
		if (IsKarutanMap())
		{
			sprintf(FileName,"%s\\AlphaTile01.Tga",WorldName);
			LoadBitmap(FileName,BITMAP_MAPTILE+12,GL_NEAREST,GL_REPEAT,false);
		}
		else
		{
#endif	// ASG_ADD_MAP_KARUTAN
			sprintf (FileName,"%s\\TileRock06.jpg",WorldName);
			LoadBitmap(FileName,BITMAP_MAPTILE+12,GL_NEAREST,GL_REPEAT,false);
#ifdef ASG_ADD_MAP_KARUTAN
		}
#endif	// ASG_ADD_MAP_KARUTAN
	    sprintf (FileName,"%s\\TileRock07.jpg",WorldName);
	    LoadBitmap(FileName,BITMAP_MAPTILE+13,GL_NEAREST,GL_REPEAT,false);

		for(int i = 1; i <= 16; i++)
		{
			if(i >= 10)
				sprintf (FileName,"%s\\ExtTile%d.jpg",WorldName, i);
			else
				sprintf (FileName,"%s\\ExtTile0%d.jpg",WorldName, i);
			LoadBitmap(FileName,BITMAP_MAPTILE+13+i ,GL_NEAREST,GL_REPEAT,false);
		}
		if(IsPKField() || IsDoppelGanger2())
		{
			sprintf(FileName,"%s\\TileGrass01_R.jpg",WorldName);
			LoadBitmap(FileName, BITMAP_MAPGRASS, GL_LINEAR, GL_REPEAT,false);
		}
		else
		{
			sprintf (FileName,"%s\\TileGrass01.tga",WorldName);
			LoadBitmap(FileName,BITMAP_MAPGRASS  ,GL_NEAREST,GL_REPEAT,false);
		}

	    sprintf (FileName,"%s\\TileGrass02.tga",WorldName);
	    LoadBitmap(FileName,BITMAP_MAPGRASS+1,GL_NEAREST,GL_REPEAT,false);
	    sprintf (FileName,"%s\\TileGrass03.tga",WorldName);
	    LoadBitmap(FileName,BITMAP_MAPGRASS+2,GL_NEAREST,GL_REPEAT,false);

        sprintf(FileName,"%s\\leaf01.tga" ,WorldName);
	    LoadBitmap(FileName,BITMAP_LEAF1          ,GL_NEAREST,GL_CLAMP_TO_EDGE,false);
	    sprintf(FileName,"%s\\leaf01.jpg" ,WorldName);
	    LoadBitmap(FileName,BITMAP_LEAF1          ,GL_NEAREST,GL_CLAMP_TO_EDGE,false);
	    sprintf(FileName,"%s\\leaf02.jpg" ,WorldName);
	    LoadBitmap(FileName,BITMAP_LEAF2          ,GL_NEAREST,GL_CLAMP_TO_EDGE,false);

		if(M34CryWolf1st::IsCyrWolf1st()==true)
		{
			sprintf(FileName,"%s\\rain011.tga" ,"World1");
			LoadBitmap(FileName,BITMAP_RAIN           ,GL_NEAREST,GL_CLAMP_TO_EDGE,false);
		}
		else
		{
			sprintf(FileName,"%s\\rain01.tga" ,"World1");
			LoadBitmap(FileName,BITMAP_RAIN           ,GL_NEAREST,GL_CLAMP_TO_EDGE,false);
		}
	    sprintf(FileName,"%s\\rain02.tga" ,"World1");
	    LoadBitmap(FileName,BITMAP_RAIN_CIRCLE    ,GL_NEAREST,GL_CLAMP_TO_EDGE,false);
	    sprintf(FileName,"%s\\rain03.tga" ,"World10");
	    LoadBitmap(FileName,BITMAP_RAIN_CIRCLE+1  ,GL_NEAREST,GL_CLAMP_TO_EDGE,false);

		if( IsEmpireGuardian1() || IsEmpireGuardian2() || IsEmpireGuardian3() || IsEmpireGuardian4())		
		{
			sprintf(FileName,"%s\\AlphaTile01.Tga",WorldName);
			LoadBitmap(FileName,BITMAP_MAPTILE+10,GL_NEAREST,GL_REPEAT,false);
		}
    }

	if(iMapWorld != 74 && iMapWorld != 75)
	{
		g_pNewUIMiniMap->UnloadImages();
		g_pNewUIMiniMap->LoadImages(WorldName);
	}
}

void CMapManager::DeleteObjects()
{
	for(int i=MODEL_WORLD_OBJECT;i<MAX_WORLD_OBJECTS;i++)
	{
		Models[i].Release();
	}

	for(int i=0;i<16;i++)
	{
		for(int j=0;j<16;j++)
		{
			OBJECT_BLOCK *ob = &ObjectBlock[i*16+j];
			OBJECT *o  = ob->Tail;
			while(1)
			{
				if(o != NULL) 
				{
					OBJECT *Temp = o->Prior;
					DeleteObject(o,ob);
					if(Temp == NULL) break;
					o = Temp;
				}
				else break;
			}
			ob->Head = NULL;
			ob->Tail = NULL;
		}
	}

	for(int i=BITMAP_MAPTILE;i<=BITMAP_RAIN_CIRCLE;i++)
	{
     	DeleteBitmap(i);
	}

	for(int i=0;i<MAX_SPRITES;i++)
		Sprites[i].Live = false;
	for(int i=0;i<MAX_BOIDS;i++)
		Boids[i].Live = false;
	for(int i=0;i<MAX_FISHS;i++)
		Fishs[i].Live = false;
	for(int i=0;i<MAX_LEAVES;i++)
		Leaves[i].Live = false;
	for(int i=0;i<MAX_PARTICLES;i++)
		Particles[i].Live = false;
	for(int i=0;i<MAX_POINTS;i++)
		Points[i].Live = false;
	for(int i=0;i<MAX_JOINTS;i++)
		Joints[i].Live = false;
	for(int i=0;i<MAX_OPERATES;i++)
		Operates[i].Live = false;
	for(int i=0;i<MAX_EFFECTS;i++)
		Effects[i].Live = false;
}

bool CMapManager::InChaosCastle(int iMap)
{
	if (iMap == -1) return false;

	if((iMap >= WD_18CHAOS_CASTLE && iMap <= WD_18CHAOS_CASTLE_END)|| iMap == WD_53CAOSCASTLE_MASTER_LEVEL)
	{
		return true;
	}

	return false; 
}

bool CMapManager::InBloodCastle(int iMap)
{
	if (iMap == -1) return false;

	if((iMap >= WD_11BLOODCASTLE1 && iMap <= WD_11BLOODCASTLE_END)|| iMap == WD_52BLOODCASTLE_MASTER_LEVEL)
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
	if (iMap == -1) return false;

	return ((this->WorldActive>=WD_24HELLAS && this->WorldActive<=WD_24HELLAS_END) || (this->WorldActive==WD_24HELLAS_7)); 
}

bool CMapManager::InHiddenHellas(int iMap) 
{ 
	if (iMap == -1) return false;

	return (this->WorldActive==WD_24HELLAS_7) ? true : false; 
}

bool CMapManager::IsPKField()
{
	return (this->WorldActive==WD_63PK_FIELD) ? true : false;
}

bool CMapManager::IsCursedTemple()
{
	return ( this->WorldActive >= WD_45CURSEDTEMPLE_LV1 && this->WorldActive <= WD_45CURSEDTEMPLE_LV6) ? true : false;
}

bool CMapManager::IsEmpireGuardian1()
{
	return (this->WorldActive==WD_69EMPIREGUARDIAN1) ? true : false;
}

bool CMapManager::IsEmpireGuardian2()
{
	return (this->WorldActive==WD_70EMPIREGUARDIAN2) ? true : false;
}

bool CMapManager::IsEmpireGuardian3()
{
	return (this->WorldActive==WD_71EMPIREGUARDIAN3) ? true : false;
}

bool CMapManager::IsEmpireGuardian4()
{
	if(this->WorldActive == WD_72EMPIREGUARDIAN4 || this->WorldActive == WD_73NEW_LOGIN_SCENE || this->WorldActive == WD_74NEW_CHARACTER_SCENE )
	{
		return true;
	}
	return false;
}

bool CMapManager::IsEmpireGuardian()
{
	if(	this->IsEmpireGuardian1() == false &&
		this->IsEmpireGuardian2() == false && 
		this->IsEmpireGuardian3() == false &&
		this->IsEmpireGuardian4() == false )
	{
		return false;
	}

	return true;
}

bool CMapManager::InBattleCastle (int iMap) 
{ 
	if (iMap == -1) return false;

	return (this->WorldActive==WD_30BATTLECASTLE) ? true : false;
}


const char* CMapManager::GetMapName( int iMap)
{

	if(iMap == WD_34CRYWOLF_1ST)
	{
		return( GlobalText[1851]);
	}
	else if( iMap == WD_33AIDA )
	{
		return( GlobalText[1850]);
	}
	else if( iMap == WD_37KANTURU_1ST )
	{
		return( GlobalText[2177]);
	}
	else if( iMap == WD_38KANTURU_2ND )
	{
		return( GlobalText[2178]);
	}
	else if( iMap == WD_39KANTURU_3RD )
	{
		return( GlobalText[2179]);
	}
	else if( iMap == WD_40AREA_FOR_GM)
	{
		return( GlobalText[2324]);
	}
	else if( iMap == WD_51HOME_6TH_CHAR)
	{
		return( GlobalText[1853]);
	}
	else if(iMap == WD_57ICECITY)
	{
		return( GlobalText[1855]);
	}
	else if(iMap == WD_58ICECITY_BOSS)
	{
		return( GlobalText[1856]);
	}
	
    if ( gMapManager.InBattleCastle( iMap )==true )
    {
        return ( GlobalText[669] );
    }
    if ( iMap == WD_31HUNTING_GROUND )
    {
        return ( GlobalText[59] );
    }
    if ( InChaosCastle( iMap )==true )
    {
        return ( GlobalText[57] );
    }
    if ( InHellas( iMap )==true )
    {
		if (InHiddenHellas( iMap) == true)
			return ( GlobalText[1852] );
        return ( GlobalText[58] );
    }
	if ( InBloodCastle( iMap ) == true )
	{
		return ( GlobalText[56] );
	}
	if ( iMap==WD_10HEAVEN )
	{
		return ( GlobalText[55+iMap-WD_10HEAVEN]);
	}
	if ( iMap==32 )
	{
		return ( GlobalText[39]);
	}
	if (SEASON3A::CGM3rdChangeUp::Instance().IsBalgasBarrackMap())
		return GlobalText[1678];
	else if (SEASON3A::CGM3rdChangeUp::Instance().IsBalgasRefugeMap())
		return GlobalText[1679];
	if( this->IsCursedTemple() )
	{
		return ( GlobalText[2369] );
	}
	if( iMap == WD_51HOME_6TH_CHAR )
	{
		return ( GlobalText[1853] );
	}
	if( iMap == WD_56MAP_SWAMP_OF_QUIET )
	{
		return ( GlobalText[1854] );
	}
	if( iMap == WD_62SANTA_TOWN )
	{
		return ( GlobalText[2611] );
	}
	if( iMap == WD_64DUELARENA)
	{
		return ( GlobalText[2703] );
	}
	if( iMap == WD_63PK_FIELD)
	{
		return (GlobalText[2686]);
	}
	if( iMap == WD_65DOPPLEGANGER1)
	{
		return (GlobalText[3057]);
	}
	if( iMap == WD_66DOPPLEGANGER2)
	{
		return (GlobalText[3057]);
	}
	if( iMap == WD_67DOPPLEGANGER3)
	{
		return (GlobalText[3057]);
	}
	if( iMap == WD_68DOPPLEGANGER4)
	{
		return (GlobalText[3057]);
	}
	if( iMap == WD_69EMPIREGUARDIAN1)
	{
		return (GlobalText[2806]);
	}
	if( iMap == WD_70EMPIREGUARDIAN2)
	{
		return (GlobalText[2806]);
	}
	if( iMap == WD_71EMPIREGUARDIAN3)
	{
		return (GlobalText[2806]);
	}
	if( iMap == WD_72EMPIREGUARDIAN4)
	{
		return (GlobalText[2806]);
	}
	if( iMap == WD_79UNITEDMARKETPLACE )
	{
		return (GlobalText[3017]);
	}
	if (iMap == WD_80KARUTAN1 || iMap == WD_81KARUTAN2)
	{
		return (GlobalText[3285]);
	}
	return ( GlobalText[30+iMap] );
}