#include "stdafx.h"
#include "UIControls.h"
#include "ZzzOpenglUtil.h"
#include "ZzzBMD.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzInterface.h"
#include "ZzzLodTerrain.h"
#include "ZzzTexture.h"
#include "ZzzOpenData.h"
#include "ZzzAI.h"
#include "ZzzInventory.h"
#include "DSPlaySound.h"
#include "wsclientinline.h"
#include "SMD.h" 
#include "./Utilities/Log/ErrorReport.h"
#include "PhysicsManager.h"
#include "CSQuest.h"
#include "GMCrywolf1st.h"
#include "CMVP1stDirection.h"
#include "CSItemOption.h"
#include "CSChaosCastle.h"
#include "GMHellas.h"
#include "GMBattleCastle.h"
#include "CComGem.h"
#include "GM_Kanturu_3rd.h"
#include "CDirection.h"
#include "PersonalShopTitleImp.h"
#include "MoveCommandData.h"
#include "UIMng.h"
#include "LoadData.h"
#include "MapManager.h"
#include "Event.h"
#include "ChangeRingManager.h"
#include "NewUISystem.h"
#include "w_CursedTemple.h"
#include "CameraMove.h"
#include "w_MapHeaders.h"
#include "QuestMng.h"
#include "ServerListManager.h"
#ifdef PBG_ADD_NEWCHAR_MONK
#include "MonkSystem.h"
#endif //PBG_ADD_NEWCHAR_MONK


///////////////////////////////////////////
extern BOOL g_bUseChatListBox;
///////////////////////////////////////////

bool Flip           = false;

void OpenModel(int Type,char *Dir,char *ModelFileName,...)
{
	char ModelName[100];
	char AnimationName[20][100];
	strcpy(ModelName,Dir);
	strcat(ModelName,ModelFileName);

    va_list Marker;
	va_start(Marker,ModelFileName);
	int AnimationCount = 0;
	while( 1 )
	{
		char *Temp = va_arg(Marker,char*);
		if(Temp==NULL || strcmp(Temp,"end")==NULL)
		{
			break;
		}
		else
		{
			strcpy(AnimationName[AnimationCount],Dir);
			strcat(AnimationName[AnimationCount],Temp);
			AnimationCount++;
		}
	}
	if(AnimationCount == 0)
	{
     	OpenSMDModel(Type,ModelName,1,Flip);
     	OpenSMDAnimation(Type,ModelName);
	}
	else
	{
     	OpenSMDModel(Type,ModelName,AnimationCount,Flip);
		for(int i=0;i<AnimationCount;i++)
		{
			bool Walk = false;
			if(i==MONSTER01_WALK || i==MONSTER01_RUN) Walk = true;
			OpenSMDAnimation(Type,AnimationName[i],Walk);
		}
	}
	va_end(Marker);

	Flip = false;
}

void OpenModels(int Model,char *FileName,int i)
{
	char Name[64];
	if(i < 10)
     	sprintf(Name,"%s0%d.smd",FileName,i);
	else
     	sprintf(Name,"%s%d.smd",FileName,i);
	OpenSMDModel(Model,Name);
   	OpenSMDAnimation(Model,Name);
}

void OpenPlayers()
{
	ModelsDump = new BMD [MAX_MODELS+1024];
	Models = ModelsDump + ( rand() % 1024);
	ZeroMemory( Models, MAX_MODELS * sizeof ( BMD));

	gLoadData.AccessModel(MODEL_PLAYER,"Data\\Player\\","Player");
	
	if(Models[MODEL_PLAYER].NumMeshs > 0)
	{
		g_ErrorReport.Write( "Player.bmd file error.\r\n");
		MessageBox(g_hWnd,"Player.bmd file error!!!",NULL,MB_OK);
		SendMessage(g_hWnd,WM_DESTROY,0,0);
	}

	for (int i = 0; i < MAX_CLASS; ++i)
	{
		gLoadData.AccessModel(MODEL_BODY_HELM  +i, "Data\\Player\\", "HelmClass" , i+1);
		gLoadData.AccessModel(MODEL_BODY_ARMOR +i, "Data\\Player\\", "ArmorClass", i+1);
		gLoadData.AccessModel(MODEL_BODY_PANTS +i, "Data\\Player\\", "PantClass" , i+1);
		gLoadData.AccessModel(MODEL_BODY_GLOVES+i, "Data\\Player\\", "GloveClass", i+1);
		gLoadData.AccessModel(MODEL_BODY_BOOTS +i, "Data\\Player\\", "BootClass" , i+1);

		if (CLASS_DARK != i && CLASS_DARK_LORD != i
#ifdef PBG_ADD_NEWCHAR_MONK
			&& (CLASS_RAGEFIGHTER != i)
#endif //PBG_ADD_NEWCHAR_MONK
			)
		{
			gLoadData.AccessModel(MODEL_BODY_HELM  +MAX_CLASS+i, "Data\\Player\\", "HelmClass2" , i+1);
    		gLoadData.AccessModel(MODEL_BODY_ARMOR +MAX_CLASS+i, "Data\\Player\\", "ArmorClass2", i+1);
    		gLoadData.AccessModel(MODEL_BODY_PANTS +MAX_CLASS+i, "Data\\Player\\", "PantClass2" , i+1);
    		gLoadData.AccessModel(MODEL_BODY_GLOVES+MAX_CLASS+i, "Data\\Player\\", "GloveClass2", i+1);
    		gLoadData.AccessModel(MODEL_BODY_BOOTS +MAX_CLASS+i, "Data\\Player\\", "BootClass2" , i+1);
		}

		gLoadData.AccessModel(MODEL_BODY_HELM  +(MAX_CLASS*2)+i, "Data\\Player\\", "HelmClass3" , i+1);
    	gLoadData.AccessModel(MODEL_BODY_ARMOR +(MAX_CLASS*2)+i, "Data\\Player\\", "ArmorClass3", i+1);
    	gLoadData.AccessModel(MODEL_BODY_PANTS +(MAX_CLASS*2)+i, "Data\\Player\\", "PantClass3" , i+1);
    	gLoadData.AccessModel(MODEL_BODY_GLOVES+(MAX_CLASS*2)+i, "Data\\Player\\", "GloveClass3", i+1);
    	gLoadData.AccessModel(MODEL_BODY_BOOTS +(MAX_CLASS*2)+i, "Data\\Player\\", "BootClass3" , i+1);
	}

	for(int i=0;i<10;i++)
	{
		gLoadData.AccessModel(MODEL_HELM  +i,"Data\\Player\\","HelmMale" ,i+1);
		gLoadData.AccessModel(MODEL_ARMOR +i,"Data\\Player\\","ArmorMale",i+1);
		gLoadData.AccessModel(MODEL_PANTS +i,"Data\\Player\\","PantMale" ,i+1);
		gLoadData.AccessModel(MODEL_GLOVES+i,"Data\\Player\\","GloveMale",i+1);
		gLoadData.AccessModel(MODEL_BOOTS +i,"Data\\Player\\","BootMale" ,i+1);
	}

	for(int i=0;i<5;i++)
	{
		gLoadData.AccessModel(MODEL_HELM  +i+10,"Data\\Player\\","HelmElf",i+1);
		gLoadData.AccessModel(MODEL_ARMOR +i+10,"Data\\Player\\","ArmorElf",i+1);
		gLoadData.AccessModel(MODEL_PANTS +i+10,"Data\\Player\\","PantElf" ,i+1);
		gLoadData.AccessModel(MODEL_GLOVES+i+10,"Data\\Player\\","GloveElf",i+1);
		gLoadData.AccessModel(MODEL_BOOTS +i+10,"Data\\Player\\","BootElf" ,i+1);

	}
	gLoadData.AccessModel(MODEL_ARMOR +15,"Data\\Player\\","ArmorMale",16);
	gLoadData.AccessModel(MODEL_PANTS +15,"Data\\Player\\","PantMale" ,16);
	gLoadData.AccessModel(MODEL_GLOVES+15,"Data\\Player\\","GloveMale",16);
	gLoadData.AccessModel(MODEL_BOOTS +15,"Data\\Player\\","BootMale" ,16);

	gLoadData.AccessModel(MODEL_HELM  +16,"Data\\Player\\","HelmMale" ,17);
	gLoadData.AccessModel(MODEL_ARMOR +16,"Data\\Player\\","ArmorMale",17);
	gLoadData.AccessModel(MODEL_PANTS +16,"Data\\Player\\","PantMale" ,17);
	gLoadData.AccessModel(MODEL_GLOVES+16,"Data\\Player\\","GloveMale",17);
	gLoadData.AccessModel(MODEL_BOOTS +16,"Data\\Player\\","BootMale" ,17);

	gLoadData.AccessModel ( MODEL_MASK_HELM+0, "Data\\Player\\", "MaskHelmMale", 1 );
	gLoadData.AccessModel ( MODEL_MASK_HELM+5, "Data\\Player\\", "MaskHelmMale", 6 );
	gLoadData.AccessModel ( MODEL_MASK_HELM+6, "Data\\Player\\", "MaskHelmMale", 7 );
	gLoadData.AccessModel ( MODEL_MASK_HELM+8, "Data\\Player\\", "MaskHelmMale", 9 );
	gLoadData.AccessModel ( MODEL_MASK_HELM+9, "Data\\Player\\", "MaskHelmMale", 10 );

    for ( int i=0; i<4; i++ )
    {   
		if ( 18+i==20 )
		{
			gLoadData.AccessModel( MODEL_HELM  +17+i, "Data\\Player\\", "HelmMaleTest", 18+i );
			gLoadData.AccessModel( MODEL_ARMOR +17+i, "Data\\Player\\", "ArmorMaleTest",18+i );
			gLoadData.AccessModel( MODEL_PANTS +17+i, "Data\\Player\\", "PantMaleTest", 18+i );
			gLoadData.AccessModel( MODEL_GLOVES+17+i, "Data\\Player\\", "GloveMaleTest",18+i );
			gLoadData.AccessModel( MODEL_BOOTS +17+i, "Data\\Player\\", "BootMaleTest", 18+i );
		}
		else
		{
			if( i<3 )
			{
				gLoadData.AccessModel( MODEL_HELM  +17+i, "Data\\Player\\", "HelmMale",  18+i );
			}
			gLoadData.AccessModel( MODEL_ARMOR +17+i, "Data\\Player\\", "ArmorMale",18+i );
			if ( 18+i == 19)
			{
				gLoadData.AccessModel( MODEL_PANTS +17+i, "Data\\Player\\", "t_PantMale", 18+i );
			}
			else
			{
				gLoadData.AccessModel( MODEL_PANTS +17+i, "Data\\Player\\", "PantMale", 18+i );
			}
			gLoadData.AccessModel( MODEL_GLOVES+17+i, "Data\\Player\\", "GloveMale",18+i );
			gLoadData.AccessModel( MODEL_BOOTS +17+i, "Data\\Player\\", "BootMale", 18+i );
		}
	}

    for ( int i=0; i<4; i++ )
    {
        if ( i!=2 )
        {
            gLoadData.AccessModel( MODEL_HELM+21+i, "Data\\Player\\", "HelmMale", 22+i );
        }
		gLoadData.AccessModel( MODEL_ARMOR+21+i, "Data\\Player\\", "ArmorMale",22+i );
		gLoadData.AccessModel( MODEL_PANTS+21+i, "Data\\Player\\", "PantMale", 22+i );
		gLoadData.AccessModel( MODEL_GLOVES+21+i,"Data\\Player\\", "GloveMale",22+i );
		gLoadData.AccessModel( MODEL_BOOTS+21+i, "Data\\Player\\", "BootMale", 22+i );
    }

    for ( int i=0; i<4; i++ )
    {
		gLoadData.AccessModel( MODEL_HELM  +25+i, "Data\\Player\\", "HelmMale", 26+i );
		gLoadData.AccessModel( MODEL_ARMOR +25+i, "Data\\Player\\", "ArmorMale",26+i );
		gLoadData.AccessModel( MODEL_PANTS +25+i, "Data\\Player\\", "PantMale", 26+i );
		gLoadData.AccessModel( MODEL_GLOVES+25+i, "Data\\Player\\", "GloveMale",26+i );
		gLoadData.AccessModel( MODEL_BOOTS +25+i, "Data\\Player\\", "BootMale", 26+i );
    }

	for(int i = 0; i < 5; ++i)
	{
		gLoadData.AccessModel( MODEL_ARMOR +29+i, "Data\\Player\\", "HDK_ArmorMale",i+1);
		gLoadData.AccessModel( MODEL_PANTS +29+i, "Data\\Player\\", "HDK_PantMale",i+1);
		gLoadData.AccessModel( MODEL_GLOVES+29+i, "Data\\Player\\", "HDK_GloveMale",i+1);
		gLoadData.AccessModel( MODEL_BOOTS +29+i, "Data\\Player\\", "HDK_BootMale",i+1);
	}

	gLoadData.AccessModel( MODEL_HELM +29, "Data\\Player\\", "HDK_HelmMale", 1);
	gLoadData.AccessModel( MODEL_HELM +30, "Data\\Player\\", "HDK_HelmMale", 2);
	gLoadData.AccessModel( MODEL_HELM +31, "Data\\Player\\", "HDK_HelmMale", 3);
	gLoadData.AccessModel( MODEL_HELM +33, "Data\\Player\\", "HDK_HelmMale", 5);
	
	for(int i = 0; i < 5; ++i)
	{
		gLoadData.AccessModel( MODEL_ARMOR +34+i, "Data\\Player\\", "CW_ArmorMale",i+1);
		gLoadData.AccessModel( MODEL_PANTS +34+i, "Data\\Player\\", "CW_PantMale",i+1);
		gLoadData.AccessModel( MODEL_GLOVES+34+i, "Data\\Player\\", "CW_GloveMale",i+1);
		gLoadData.AccessModel( MODEL_BOOTS +34+i, "Data\\Player\\", "CW_BootMale",i+1);
	}

	//마검사는 제외하고 투구도 추가
	gLoadData.AccessModel( MODEL_HELM +34, "Data\\Player\\", "CW_HelmMale", 1);
	gLoadData.AccessModel( MODEL_HELM +35, "Data\\Player\\", "CW_HelmMale", 2);
	gLoadData.AccessModel( MODEL_HELM +36, "Data\\Player\\", "CW_HelmMale", 3);
	gLoadData.AccessModel( MODEL_HELM +38, "Data\\Player\\", "CW_HelmMale", 5);
	
    for (int i = 0; i < 6; ++i)
    {
		gLoadData.AccessModel(MODEL_HELM  +39+i, "Data\\Player\\", "HelmMale", 40+i);
		gLoadData.AccessModel(MODEL_ARMOR +39+i, "Data\\Player\\", "ArmorMale",40+i);
		gLoadData.AccessModel(MODEL_PANTS +39+i, "Data\\Player\\", "PantMale", 40+i);
		gLoadData.AccessModel(MODEL_GLOVES+39+i, "Data\\Player\\", "GloveMale",40+i);
		gLoadData.AccessModel(MODEL_BOOTS +39+i, "Data\\Player\\", "BootMale", 40+i);
    }

	for (int i = 0; i < MODEL_ITEM_COMMON_NUM; ++i)
	{
		gLoadData.AccessModel(MODEL_HELM2  +i, "Data\\Player\\", "HelmElfC" , i+1);
		gLoadData.AccessModel(MODEL_ARMOR2 +i, "Data\\Player\\", "ArmorElfC", i+1);
		gLoadData.AccessModel(MODEL_PANTS2 +i, "Data\\Player\\", "PantElfC" , i+1);
		gLoadData.AccessModel(MODEL_GLOVES2+i, "Data\\Player\\", "GloveElfC", i+1);
		gLoadData.AccessModel(MODEL_BOOTS2 +i, "Data\\Player\\", "BootElfC" , i+1);
	}

	for(int i=45 ; i<=53 ; i++)
	{
		if( i==47 || i== 48)
			continue;
		gLoadData.AccessModel( MODEL_HELM+i, "Data\\Player\\", "HelmMale", i+1);
	} // for()
	
	for(int i=45 ; i<=53 ; i++)
	{	
		gLoadData.AccessModel(MODEL_ARMOR+i, "Data\\Player\\", "ArmorMale", i+1);
		gLoadData.AccessModel(MODEL_PANTS+i, "Data\\Player\\", "PantMale", i+1);
		gLoadData.AccessModel(MODEL_GLOVES+i, "Data\\Player\\", "GloveMale", i+1);
		gLoadData.AccessModel(MODEL_BOOTS+i, "Data\\Player\\", "BootMale", i+1);
	} // for()

#ifdef PBG_ADD_NEWCHAR_MONK_ITEM

	for (int i = 0; i<MODEL_ITEM_COMMONCNT_RAGEFIGHTER; ++i)
	{
		gLoadData.AccessModel(MODEL_HELM_MONK  +i, "Data\\Player\\", "HelmMonk" , i+1);
		gLoadData.AccessModel(MODEL_ARMOR_MONK +i, "Data\\Player\\", "ArmorMonk", i+1);
		gLoadData.AccessModel(MODEL_PANTS_MONK +i, "Data\\Player\\", "PantMonk" , i+1);
		gLoadData.AccessModel(MODEL_BOOTS_MONK +i, "Data\\Player\\", "BootMonk" , i+1);
	}

	for(int i=0; i<3; ++i)
	{
		gLoadData.AccessModel(MODEL_HELM  +59+i, "Data\\Player\\", "HelmMale", 60+i);
		gLoadData.AccessModel(MODEL_ARMOR +59+i, "Data\\Player\\", "ArmorMale",60+i);
		gLoadData.AccessModel(MODEL_PANTS +59+i, "Data\\Player\\", "PantMale", 60+i);
		gLoadData.AccessModel(MODEL_BOOTS +59+i, "Data\\Player\\", "BootMale", 60+i);
	}
#endif //PBG_ADD_NEWCHAR_MONK_ITEM

	{
		CPhysicsClothMesh *pCloth = new CPhysicsClothMesh [1];
		pCloth[0].Create( &Hero->Object, 2, 17, 0.0f, 0.0f, 0.0f, 5, 8, 45.0f, 85.0f, BITMAP_PANTS_G_SOUL, BITMAP_PANTS_G_SOUL, PCT_MASK_ALPHA | PCT_HEAVY | PCT_STICKED, MODEL_PANTS+18);
		delete [] pCloth;
	}

	for(int i=0;i<1;i++)
      	gLoadData.AccessModel(MODEL_SHADOW_BODY+i,"Data\\Player\\","Shadow",i+1);

	Models[MODEL_PLAYER].BoneHead    = 20;
	Models[MODEL_PLAYER].BoneFoot[0] = 6;
	Models[MODEL_PLAYER].BoneFoot[1] = 13;
	
	for(int i=PLAYER_STOP_MALE;i<=PLAYER_STOP_RIDE_WEAPON;i++)
	    Models[MODEL_PLAYER].Actions[i].PlaySpeed = 0.28f;

	Models[MODEL_PLAYER].Actions[PLAYER_STOP_SWORD         ].PlaySpeed = 0.26f;
	Models[MODEL_PLAYER].Actions[PLAYER_STOP_TWO_HAND_SWORD].PlaySpeed = 0.24f;
	Models[MODEL_PLAYER].Actions[PLAYER_STOP_SPEAR		   ].PlaySpeed = 0.24f;
	Models[MODEL_PLAYER].Actions[PLAYER_STOP_BOW		   ].PlaySpeed = 0.22f;
	Models[MODEL_PLAYER].Actions[PLAYER_STOP_CROSSBOW      ].PlaySpeed = 0.22f;
	Models[MODEL_PLAYER].Actions[PLAYER_STOP_SUMMONER].PlaySpeed = 0.24f;
	Models[MODEL_PLAYER].Actions[PLAYER_STOP_WAND].PlaySpeed = 0.30f;

	for(int i=PLAYER_WALK_MALE;i<=PLAYER_RUN_RIDE_WEAPON;i++)
	    Models[MODEL_PLAYER].Actions[i].PlaySpeed = 0.3f;

	Models[MODEL_PLAYER].Actions[PLAYER_WALK_WAND].PlaySpeed = 0.44f;
	Models[MODEL_PLAYER].Actions[PLAYER_RUN_WAND].PlaySpeed = 0.76f;
    Models[MODEL_PLAYER].Actions[PLAYER_WALK_SWIM].PlaySpeed = 0.35f;
    Models[MODEL_PLAYER].Actions[PLAYER_RUN_SWIM ].PlaySpeed = 0.35f;

	for(int i=PLAYER_DEFENSE1;i<=PLAYER_SHOCK;i++)
	    Models[MODEL_PLAYER].Actions[i].PlaySpeed = 0.32f;

	for(int i=PLAYER_DIE1;i<=PLAYER_DIE2;i++)
	    Models[MODEL_PLAYER].Actions[i].PlaySpeed = 0.45f;

	for(int i=PLAYER_SIT1;i<MAX_PLAYER_ACTION;i++)
	    Models[MODEL_PLAYER].Actions[i].PlaySpeed = 0.4f;

	Models[MODEL_PLAYER].Actions[PLAYER_SHOCK              ].PlaySpeed = 0.4f;
	Models[MODEL_PLAYER].Actions[PLAYER_SEE1               ].PlaySpeed = 0.28f;
	Models[MODEL_PLAYER].Actions[PLAYER_SEE_FEMALE1        ].PlaySpeed = 0.28f;
	Models[MODEL_PLAYER].Actions[PLAYER_HEALING1           ].PlaySpeed = 0.2f;
	Models[MODEL_PLAYER].Actions[PLAYER_HEALING_FEMALE1    ].PlaySpeed = 0.2f;

	Models[MODEL_PLAYER].Actions[PLAYER_JACK_1].PlaySpeed = 0.38f;
	Models[MODEL_PLAYER].Actions[PLAYER_JACK_2].PlaySpeed = 0.38f;
	
	Models[MODEL_PLAYER].Actions[PLAYER_SANTA_1].PlaySpeed = 0.34f;
	Models[MODEL_PLAYER].Actions[PLAYER_SANTA_2].PlaySpeed = 0.30f;

	Models[MODEL_PLAYER].Actions[PLAYER_SKILL_RIDER].PlaySpeed      = 0.2f;
	Models[MODEL_PLAYER].Actions[PLAYER_SKILL_RIDER_FLY].PlaySpeed  = 0.2f;

	Models[MODEL_PLAYER].Actions[PLAYER_STOP_TWO_HAND_SWORD_TWO].PlaySpeed	= 0.24f;
	Models[MODEL_PLAYER].Actions[PLAYER_WALK_TWO_HAND_SWORD_TWO].PlaySpeed	= 0.3f;
	Models[MODEL_PLAYER].Actions[PLAYER_RUN_TWO_HAND_SWORD_TWO].PlaySpeed	= 0.3f;
	Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_TWO_HAND_SWORD_TWO].PlaySpeed= 0.24f;

	//Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_ONETOONE    ].PlaySpeed = 0.30f;
	Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_ONETOONE    ].PlaySpeed = 0.45f;
	
	Models[MODEL_PLAYER].Actions[PLAYER_DIE1   ].Loop = true;
	Models[MODEL_PLAYER].Actions[PLAYER_DIE2   ].Loop = true;
	Models[MODEL_PLAYER].Actions[PLAYER_COME_UP].Loop = true;

	Models[MODEL_PLAYER].Actions[PLAYER_SKILL_HELL_BEGIN].Loop = true;

    Models[MODEL_PLAYER].Actions[PLAYER_DARKLORD_STAND].PlaySpeed           = 0.3f;
    Models[MODEL_PLAYER].Actions[PLAYER_DARKLORD_WALK].PlaySpeed            = 0.3f;
	Models[MODEL_PLAYER].Actions[PLAYER_STOP_RIDE_HORSE].PlaySpeed          = 0.3f;
    Models[MODEL_PLAYER].Actions[PLAYER_RUN_RIDE_HORSE].PlaySpeed           = 0.3f;
	Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_STRIKE].PlaySpeed            = 0.2f;
	Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_TELEPORT].PlaySpeed          = 0.28f;

    Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_RIDE_STRIKE].PlaySpeed       = 0.3f;
	Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_RIDE_TELEPORT].PlaySpeed     = 0.3f;
    Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_RIDE_HORSE_SWORD].PlaySpeed  = 0.28f;
	Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_RIDE_ATTACK_FLASH].PlaySpeed = 0.3f;
	Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_RIDE_ATTACK_MAGIC].PlaySpeed = 0.3f;
    Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_DARKHORSE].PlaySpeed         = 0.2f;

	Models[MODEL_PLAYER].Actions[PLAYER_IDLE1_DARKHORSE].PlaySpeed          = 1.0f;
	Models[MODEL_PLAYER].Actions[PLAYER_IDLE2_DARKHORSE].PlaySpeed          = 1.0f;	
	
	for(int i=PLAYER_FENRIR_ATTACK; i<=PLAYER_FENRIR_WALK_ONE_LEFT; i++)
	{
		Models[MODEL_PLAYER].Actions[i].PlaySpeed		= 0.45f;
	}

	for(int i=PLAYER_FENRIR_RUN; i<=PLAYER_FENRIR_RUN_ONE_LEFT_ELF; i++)
	{
		Models[MODEL_PLAYER].Actions[i].PlaySpeed		= 0.71f;	
	}

	for(int i=PLAYER_FENRIR_STAND; i<=PLAYER_FENRIR_STAND_ONE_LEFT; i++)
	{
		Models[MODEL_PLAYER].Actions[i].PlaySpeed		= 0.4f;
	}
	
	Models[MODEL_PLAYER].Actions[PLAYER_FENRIR_ATTACK_MAGIC].PlaySpeed = 0.30f;
    Models[MODEL_PLAYER].Actions[PLAYER_FENRIR_ATTACK_DARKLORD_STRIKE].PlaySpeed = 0.3f;
	Models[MODEL_PLAYER].Actions[PLAYER_FENRIR_ATTACK_DARKLORD_TELEPORT].PlaySpeed = 0.3f;
    Models[MODEL_PLAYER].Actions[PLAYER_FENRIR_ATTACK_DARKLORD_SWORD].PlaySpeed = 0.28f;
	Models[MODEL_PLAYER].Actions[PLAYER_FENRIR_ATTACK_DARKLORD_FLASH].PlaySpeed = 0.3f;

    Models[MODEL_PLAYER].Actions[PLAYER_HIGH_SHOCK].PlaySpeed         = 0.3f;

#ifdef PBG_ADD_NEWCHAR_MONK_ANI

	for(int i=PLAYER_RAGE_FENRIR; i<=PLAYER_RAGE_FENRIR_ATTACK_RIGHT; i++)
	{
		if(i>=PLAYER_RAGE_FENRIR_TWO_SWORD && i<= PLAYER_RAGE_FENRIR_ONE_LEFT)
			Models[MODEL_PLAYER].Actions[i].PlaySpeed		= 0.225f;
		else
			Models[MODEL_PLAYER].Actions[i].PlaySpeed		= 0.45f;
	}
	for(int i=PLAYER_RAGE_FENRIR_STAND_TWO_SWORD; i<=PLAYER_RAGE_FENRIR_STAND_ONE_LEFT; i++)
	{
		Models[MODEL_PLAYER].Actions[i].PlaySpeed		= 0.2f;
	}
	Models[MODEL_PLAYER].Actions[PLAYER_RAGE_FENRIR_STAND].PlaySpeed = 0.21f;

	for(int i=PLAYER_RAGE_FENRIR_RUN; i<=PLAYER_RAGE_FENRIR_RUN_ONE_LEFT; i++)
	{
		Models[MODEL_PLAYER].Actions[i].PlaySpeed		= 0.355f;
	}
	Models[MODEL_PLAYER].Actions[PLAYER_RAGE_UNI_RUN].PlaySpeed = 0.3f;
#ifndef PBG_FIX_NEWCHAR_MONK_UNIANI
	Models[MODEL_PLAYER].Actions[PLAYER_RAGE_UNI_ATTACK_ONE_RIGHT].PlaySpeed = 0.2f;
#endif //PBG_FIX_NEWCHAR_MONK_UNIANI
	Models[MODEL_PLAYER].Actions[PLAYER_RAGE_UNI_STOP_ONE_RIGHT].PlaySpeed = 0.18f;

	Models[MODEL_PLAYER].Actions[PLAYER_STOP_RAGEFIGHTER].PlaySpeed = 0.16f;
#endif //PBG_ADD_NEWCHAR_MONK_ANI
	SetAttackSpeed();
	
	gLoadData.AccessModel(MODEL_GM_CHARACTER, "Data\\Skill\\", "youngza");
}

void OpenPlayerTextures()
{
	LoadBitmap("Player\\skin_barbarian_01.jpg",BITMAP_SKIN  );
	LoadBitmap("Player\\level_man022.jpg"     ,BITMAP_SKIN+1);
	LoadBitmap("Player\\skin_wizard_01.jpg"   ,BITMAP_SKIN+2);
	LoadBitmap("Player\\level_man01.jpg"      ,BITMAP_SKIN+3);
	LoadBitmap("Player\\skin_archer_01.jpg"   ,BITMAP_SKIN+4);
	LoadBitmap("Player\\level_man033.jpg"     ,BITMAP_SKIN+5);
   	LoadBitmap("Player\\skin_special_01.jpg"  ,BITMAP_SKIN+6);
	LoadBitmap("Player\\level_man02.jpg"      ,BITMAP_SKIN+8);
	LoadBitmap("Player\\hair_r.jpg"       ,BITMAP_HAIR);
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
	LoadBitmap("Player\\LevelClass107.jpg"   ,BITMAP_SKIN+12);
	LoadBitmap("Player\\LevelClass207.jpg"   ,BITMAP_SKIN+13);
	LoadBitmap("Player\\LevelClass207_1.jpg" ,BITMAP_SKIN+14);
#endif //PBG_ADD_NEWCHAR_MONK_ITEM

	int nIndex;

	for (int j = 0; j < 3; ++j)
	{
		for (int i = 0; i < MAX_CLASS; ++i)
		{
			nIndex = MAX_CLASS*j+i;
			if (1 == j && (CLASS_DARK == i || CLASS_DARK_LORD == i
#ifdef PBG_ADD_NEWCHAR_MONK
				|| (CLASS_RAGEFIGHTER == i)
#endif //PBG_ADD_NEWCHAR_MONK
				))
				continue;

			gLoadData.OpenTexture(MODEL_BODY_HELM  +nIndex, "Player\\");
			gLoadData.OpenTexture(MODEL_BODY_ARMOR +nIndex, "Player\\");
			gLoadData.OpenTexture(MODEL_BODY_PANTS +nIndex, "Player\\");
			gLoadData.OpenTexture(MODEL_BODY_GLOVES+nIndex, "Player\\");
			gLoadData.OpenTexture(MODEL_BODY_BOOTS +nIndex, "Player\\");
		}
	}

	for ( int i=0; i<17; i++ )
	{
		gLoadData.OpenTexture(MODEL_HELM+i  ,"Player\\");
		gLoadData.OpenTexture(MODEL_ARMOR+i ,"Player\\");
		gLoadData.OpenTexture(MODEL_PANTS+i ,"Player\\");
		gLoadData.OpenTexture(MODEL_GLOVES+i,"Player\\");
		gLoadData.OpenTexture(MODEL_BOOTS+i ,"Player\\");
	}

    for ( int i=0; i<4; i++ )
    {
        if ( i!=2 )
        {
            gLoadData.OpenTexture ( MODEL_HELM+21+i, "Player\\" );
        }
		gLoadData.OpenTexture ( MODEL_ARMOR+21+i,  "Player\\" );
		gLoadData.OpenTexture ( MODEL_PANTS+21+i,  "Player\\" );
		gLoadData.OpenTexture ( MODEL_GLOVES+21+i, "Player\\" );
		gLoadData.OpenTexture ( MODEL_BOOTS+21+i,  "Player\\" );
    }

	for ( int i=0; i<4; i++ )
	{
		gLoadData.OpenTexture ( MODEL_HELM  +25+i, "Player\\" );
		gLoadData.OpenTexture ( MODEL_ARMOR +25+i, "Player\\" );
		gLoadData.OpenTexture ( MODEL_PANTS +25+i, "Player\\" );
		gLoadData.OpenTexture ( MODEL_GLOVES+25+i, "Player\\" );
		gLoadData.OpenTexture ( MODEL_BOOTS +25+i, "Player\\" );
	}

    for( int i=0; i<4; i++ )
    {   
	    gLoadData.OpenTexture(MODEL_HELM  +17+i, "Player\\");
	    gLoadData.OpenTexture(MODEL_ARMOR +17+i, "Player\\");
	    gLoadData.OpenTexture(MODEL_PANTS +17+i, "Player\\");
	    gLoadData.OpenTexture(MODEL_GLOVES+17+i, "Player\\");
	    gLoadData.OpenTexture(MODEL_BOOTS +17+i, "Player\\");
    }

	gLoadData.OpenTexture ( MODEL_MASK_HELM+0, "Player\\" );
	gLoadData.OpenTexture ( MODEL_MASK_HELM+5, "Player\\" );
	gLoadData.OpenTexture ( MODEL_MASK_HELM+6, "Player\\" );
	gLoadData.OpenTexture ( MODEL_MASK_HELM+8, "Player\\" );
	gLoadData.OpenTexture ( MODEL_MASK_HELM+9, "Player\\" );

    gLoadData.OpenTexture(MODEL_SHADOW_BODY,"Player\\");
	{
		LoadBitmap("Player\\Robe01.jpg",BITMAP_ROBE);
		LoadBitmap("Player\\Robe02.jpg",BITMAP_ROBE+1);
		LoadBitmap("Player\\Robe03.tga",BITMAP_ROBE+2);
		LoadBitmap("Player\\DarklordRobe.tga",BITMAP_ROBE+7);
		LoadBitmap("Item\\msword03.tga",BITMAP_ROBE+8);
		LoadBitmap("Item\\dl_redwings02.tga",BITMAP_ROBE+9);
		LoadBitmap("Item\\dl_redwings03.tga",BITMAP_ROBE+10);
	}

	for(int i = 0; i < 5; ++i)
	{
		gLoadData.OpenTexture( MODEL_ARMOR+29+i, "Player\\" );
		gLoadData.OpenTexture( MODEL_PANTS+29+i, "Player\\" );
		gLoadData.OpenTexture( MODEL_GLOVES+29+i, "Player\\" );
		gLoadData.OpenTexture( MODEL_BOOTS+29+i, "Player\\" );
	}
	gLoadData.OpenTexture( MODEL_HELM+29, "Player\\");
	gLoadData.OpenTexture( MODEL_HELM+30, "Player\\");
	gLoadData.OpenTexture( MODEL_HELM+31, "Player\\");
	gLoadData.OpenTexture( MODEL_HELM+33, "Player\\");

	for(int i = 0; i < 5; ++i)
	{
		gLoadData.OpenTexture( MODEL_ARMOR+34+i,	"Player\\" );
		gLoadData.OpenTexture( MODEL_PANTS+34+i,	"Player\\" );
		gLoadData.OpenTexture( MODEL_GLOVES+34+i, "Player\\" );
		gLoadData.OpenTexture( MODEL_BOOTS+34+i,	"Player\\" );
	}
	gLoadData.OpenTexture( MODEL_HELM+34, "Player\\");
	gLoadData.OpenTexture( MODEL_HELM+35, "Player\\");
	gLoadData.OpenTexture( MODEL_HELM+36, "Player\\");
	gLoadData.OpenTexture( MODEL_HELM+38, "Player\\");

	char szFileName[64];

	for (int i = 0; i < 6; ++i)
	{
		gLoadData.OpenTexture(MODEL_HELM  +39+i, "Player\\" );
		gLoadData.OpenTexture(MODEL_ARMOR +39+i, "Player\\" );
		gLoadData.OpenTexture(MODEL_PANTS +39+i, "Player\\" );
		gLoadData.OpenTexture(MODEL_GLOVES+39+i, "Player\\" );
		gLoadData.OpenTexture(MODEL_BOOTS +39+i, "Player\\" );

		::sprintf(szFileName, "Player\\InvenArmorMale%d.tga", 40+i);
		::LoadBitmap(szFileName, BITMAP_INVEN_ARMOR+i);
		::sprintf(szFileName, "Player\\InvenPantsMale%d.tga", 40+i);
		::LoadBitmap(szFileName, BITMAP_INVEN_PANTS+i);
	}

	::sprintf(szFileName, "Player\\Item312_Armoritem.tga");
	::LoadBitmap(szFileName, BITMAP_SKIN_ARMOR_DEVINE);
	::sprintf(szFileName, "Player\\Item312_Pantitem.tga");
	::LoadBitmap(szFileName, BITMAP_SKIN_PANTS_DEVINE);
	::sprintf(szFileName, "Player\\SkinClass706_upperitem.tga");
	::LoadBitmap(szFileName, BITMAP_SKIN_ARMOR_SUCCUBUS);
	::sprintf(szFileName, "Player\\SkinClass706_loweritem.tga");
	::LoadBitmap(szFileName, BITMAP_SKIN_PANTS_SUCCUBUS);

	for (int i = 0; i < MODEL_ITEM_COMMON_NUM; ++i)
	{
		gLoadData.OpenTexture(MODEL_HELM2  +i, "Player\\");
		gLoadData.OpenTexture(MODEL_ARMOR2 +i, "Player\\");
		gLoadData.OpenTexture(MODEL_PANTS2 +i, "Player\\");
		gLoadData.OpenTexture(MODEL_GLOVES2+i, "Player\\");
		gLoadData.OpenTexture(MODEL_BOOTS2 +i, "Player\\");
	}

	for(int i=45 ; i<=53 ; i++)
	{
		if( i==47 || i== 48)
			continue;

		gLoadData.OpenTexture( MODEL_HELM+i, "Player\\" );
	} // for()
	
	for(int i=45 ; i<=53 ; i++)
	{
		gLoadData.OpenTexture(MODEL_ARMOR+i, "Player\\" );
		gLoadData.OpenTexture(MODEL_PANTS+i, "Player\\" );
		gLoadData.OpenTexture(MODEL_GLOVES+i, "Player\\" );
		gLoadData.OpenTexture(MODEL_BOOTS+i, "Player\\" );
	} // for()

	gLoadData.OpenTexture(MODEL_GM_CHARACTER, "Skill\\");

#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
	for (int i = 0; i<MODEL_ITEM_COMMONCNT_RAGEFIGHTER; ++i)
	{
		gLoadData.OpenTexture(MODEL_HELM_MONK  +i, "Player\\");
		gLoadData.OpenTexture(MODEL_ARMOR_MONK +i, "Player\\");
		gLoadData.OpenTexture(MODEL_PANTS_MONK +i, "Player\\");
		gLoadData.OpenTexture(MODEL_BOOTS_MONK +i, "Player\\");
	}

	for(int i=0; i<3; ++i)
	{
		gLoadData.OpenTexture(MODEL_HELM  +59+i, "Player\\");
		gLoadData.OpenTexture(MODEL_ARMOR +59+i, "Player\\");
		gLoadData.OpenTexture(MODEL_PANTS +59+i, "Player\\");
		gLoadData.OpenTexture(MODEL_BOOTS +59+i, "Player\\");
	}
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
}

void OpenItems()
{
    //////////////////////////////////////////////////////////////////////////
    //  MODEL_SWORD
    //////////////////////////////////////////////////////////////////////////

	for(int i=0;i<17;i++)
      	gLoadData.AccessModel(MODEL_SWORD+i ,"Data\\Item\\","Sword" ,i+1);

    gLoadData.AccessModel(MODEL_SWORD+17 ,"Data\\Item\\","Sword" ,18);
    gLoadData.AccessModel(MODEL_SWORD+18 ,"Data\\Item\\","Sword" ,19);
    gLoadData.AccessModel(MODEL_SWORD+19 ,"Data\\Item\\","Sword" ,20);

	gLoadData.AccessModel(MODEL_SWORD+20 ,"Data\\Item\\","Sword" ,21);
	gLoadData.AccessModel(MODEL_SWORD+21 ,"Data\\Item\\","Sword" ,22);
	gLoadData.AccessModel ( MODEL_SWORD+31, "Data\\Item\\", "Sword", 32 );

	//////////////////////////////////////////////////////////////////////////
	//	MODEL_AXE
	//////////////////////////////////////////////////////////////////////////

	for(int i=0;i<9;i++)
      	gLoadData.AccessModel(MODEL_AXE+i   ,"Data\\Item\\","Axe"   ,i+1);

	//////////////////////////////////////////////////////////////////////////
	//	MODEL_MACE
	//////////////////////////////////////////////////////////////////////////

	for(int i=0;i<7;i++)
      	gLoadData.AccessModel(MODEL_MACE+i  ,"Data\\Item\\","Mace"  ,i+1);

	gLoadData.AccessModel ( MODEL_MACE+7, "Data\\Item\\", "Mace", 8);

	// MODEL_MACE+8,9,10,11,12
	for(int i=0; i<5; i++)
		gLoadData.AccessModel( MODEL_MACE+8+i, "Data\\Item\\", "Mace", 9+i);
	
	gLoadData.AccessModel( MODEL_MACE+13, "Data\\Item\\", "Saint");

	//////////////////////////////////////////////////////////////////////////
	//	MODEL_SPEAR
	//////////////////////////////////////////////////////////////////////////

	for(int i=0;i<10;i++)
      	gLoadData.AccessModel(MODEL_SPEAR+i ,"Data\\Item\\","Spear" ,i+1);

	gLoadData.AccessModel ( MODEL_SPEAR+10, "Data\\Item\\", "Spear", 11 );

	//////////////////////////////////////////////////////////////////////////
	//	MODEL_SHIELD
	//////////////////////////////////////////////////////////////////////////

	for(int i=0;i<15;i++)
      	gLoadData.AccessModel(MODEL_SHIELD+i,"Data\\Item\\","Shield",i+1);

	gLoadData.AccessModel ( MODEL_SHIELD+15, "Data\\Item\\", "Shield", 16 );
	gLoadData.AccessModel ( MODEL_SHIELD+16, "Data\\Item\\", "Shield", 17 );

    //////////////////////////////////////////////////////////////////////////
    //  MODEL_STAFF
    //////////////////////////////////////////////////////////////////////////

    for(int i=0;i<9;i++)
      	gLoadData.AccessModel(MODEL_STAFF+i ,"Data\\Item\\","Staff" ,i+1);

    gLoadData.AccessModel( MODEL_STAFF+9, "Data\\Item\\", "Staff", 10 );
    gLoadData.AccessModel(MODEL_STAFF+10 ,"Data\\Item\\","Staff" ,11);
    gLoadData.AccessModel(MODEL_STAFF+11 ,"Data\\Item\\","Staff" ,12);

	for (int i = 14; i <= 20; ++i)
		::gLoadData.AccessModel(MODEL_STAFF+i, "Data\\Item\\", "Staff", i+1);

    //////////////////////////////////////////////////////////////////////////
    //  MODEL_BOW
    //////////////////////////////////////////////////////////////////////////

	for(int i=0;i<7;i++)
      	gLoadData.AccessModel(MODEL_BOW+i   ,"Data\\Item\\","Bow"   ,i+1);

	for(int i=0;i<7;i++)
      	gLoadData.AccessModel(MODEL_BOW+i+8 ,"Data\\Item\\","CrossBow",i+1);

   	gLoadData.AccessModel(MODEL_BOW+7 ,"Data\\Item\\","Arrows",1); 
   	gLoadData.AccessModel(MODEL_BOW+15,"Data\\Item\\","Arrows",2);
   	gLoadData.AccessModel(MODEL_BOW+16,"Data\\Item\\","CrossBow",17);
    gLoadData.AccessModel( MODEL_BOW+17, "Data\\Item\\", "Bow", 18 );
    gLoadData.AccessModel(MODEL_BOW+18 ,"Data\\Item\\","Bow" ,19);
	gLoadData.AccessModel(MODEL_BOW+19, "Data\\Item\\", "CrossBow", 20 );
	gLoadData.AccessModel(MODEL_BOW+20, "Data\\Item\\", "Bow", 20 );

    //////////////////////////////////////////////////////////////////////////
    //  MODEL_HELPER
    //////////////////////////////////////////////////////////////////////////

    for(int i=0;i<3;i++)
      	gLoadData.AccessModel(MODEL_HELPER+i,"Data\\Player\\","Helper",i+1);

	for(int i=0;i<2;i++)
      	gLoadData.AccessModel(MODEL_HELPER+i+8,"Data\\Item\\","Ring"  ,i+1);

	g_ChangeRingMgr->LoadItemModel();
   	

	for(int i=0;i<2;i++)
      	gLoadData.AccessModel(MODEL_HELPER+i+12,"Data\\Item\\","Necklace"  ,i+1);

    gLoadData.AccessModel( MODEL_HELPER+3, "Data\\Player\\", "Helper", 4 );
    gLoadData.AccessModel( MODEL_HELPER+4, "Data\\Item\\", "DarkHorseHorn" );
    gLoadData.AccessModel( MODEL_HELPER+31,"Data\\Item\\", "DarkHorseSoul" );
    gLoadData.AccessModel( MODEL_HELPER+5, "Data\\Item\\", "SpiritBill" );

    gLoadData.AccessModel( MODEL_HELPER+21, "Data\\Item\\", "FireRing" );	
    gLoadData.AccessModel( MODEL_HELPER+22, "Data\\Item\\", "GroundRing" );
    gLoadData.AccessModel( MODEL_HELPER+23, "Data\\Item\\", "WindRing" );
    gLoadData.AccessModel( MODEL_HELPER+24, "Data\\Item\\", "ManaRing" );
    gLoadData.AccessModel( MODEL_HELPER+25, "Data\\Item\\", "IceNecklace" );
    gLoadData.AccessModel( MODEL_HELPER+26, "Data\\Item\\", "WindNecklace" );
    gLoadData.AccessModel( MODEL_HELPER+27, "Data\\Item\\", "WaterNecklace" );
    gLoadData.AccessModel( MODEL_HELPER+28, "Data\\Item\\", "AgNecklace" );
    gLoadData.AccessModel( MODEL_HELPER+29, "Data\\Item\\", "EventChaosCastle" );
	gLoadData.AccessModel( MODEL_HELPER+7,  "Data\\Item\\", "Covenant" );
	gLoadData.AccessModel( MODEL_HELPER+11, "Data\\Item\\", "SummonBook" );
	gLoadData.AccessModel(MODEL_EVENT + 18, "Data\\Item\\", "LifeStoneItem");

	gLoadData.AccessModel( MODEL_HELPER+32, "Data\\Item\\", "FR_1" );
	gLoadData.AccessModel( MODEL_HELPER+33, "Data\\Item\\", "FR_2" );
	gLoadData.AccessModel( MODEL_HELPER+34, "Data\\Item\\", "FR_3" );
	gLoadData.AccessModel( MODEL_HELPER+35, "Data\\Item\\", "FR_4" );
	gLoadData.AccessModel( MODEL_HELPER+36, "Data\\Item\\", "FR_5" );
	gLoadData.AccessModel( MODEL_HELPER+37, "Data\\Item\\", "FR_6" );

	gLoadData.AccessModel(MODEL_HELPER+46, "Data\\Item\\partCharge1\\", "entrancegray");
	gLoadData.AccessModel(MODEL_HELPER+47, "Data\\Item\\partCharge1\\", "entrancered");
	gLoadData.AccessModel(MODEL_HELPER+48, "Data\\Item\\partCharge1\\", "entrancebleu");
	gLoadData.AccessModel(MODEL_POTION+54, "Data\\Item\\partCharge1\\", "juju");
	gLoadData.AccessModel(MODEL_HELPER+43, "Data\\Item\\partCharge1\\", "monmark01");
	gLoadData.AccessModel(MODEL_HELPER+44, "Data\\Item\\partCharge1\\", "monmark02");
	gLoadData.AccessModel(MODEL_HELPER+45, "Data\\Item\\partCharge1\\", "monmark03");
	gLoadData.AccessModel(MODEL_POTION+53, "Data\\Item\\partCharge1\\", "bujuck01");
	
	gLoadData.AccessModel(MODEL_POTION+58, "Data\\Item\\partCharge1\\", "expensiveitem01");
	gLoadData.AccessModel(MODEL_POTION+59, "Data\\Item\\partCharge1\\", "expensiveitem02a");
	gLoadData.AccessModel(MODEL_POTION+60, "Data\\Item\\partCharge1\\", "expensiveitem02b");
	gLoadData.AccessModel(MODEL_POTION+61, "Data\\Item\\partCharge1\\", "expensiveitem03a");
	gLoadData.AccessModel(MODEL_POTION+62, "Data\\Item\\partCharge1\\", "expensiveitem03b");
	gLoadData.AccessModel(MODEL_POTION+70, "Data\\Item\\partCharge2\\", "EPotionR");
	gLoadData.AccessModel(MODEL_POTION+71, "Data\\Item\\partCharge2\\", "EPotionB");
	gLoadData.AccessModel(MODEL_POTION+72, "Data\\Item\\partCharge2\\", "elitescroll_quick");
	gLoadData.AccessModel(MODEL_POTION+73, "Data\\Item\\partCharge2\\", "elitescroll_depence");
	gLoadData.AccessModel(MODEL_POTION+74, "Data\\Item\\partCharge2\\", "elitescroll_anger");
	gLoadData.AccessModel(MODEL_POTION+75, "Data\\Item\\partCharge2\\", "elitescroll_magic");
	gLoadData.AccessModel(MODEL_POTION+76, "Data\\Item\\partCharge2\\", "elitescroll_strenth");
	gLoadData.AccessModel(MODEL_POTION+77, "Data\\Item\\partCharge2\\", "elitescroll_mana");

	char	szPC6Path[24];
	sprintf( szPC6Path, "Data\\Item\\partCharge6\\" );

	gLoadData.AccessModel(MODEL_TYPE_CHARM_MIXWING+EWS_KNIGHT_1_CHARM, szPC6Path, "amulet_satan");         
	gLoadData.AccessModel(MODEL_TYPE_CHARM_MIXWING+EWS_MAGICIAN_1_CHARM, szPC6Path, "amulet_sky");          
	gLoadData.AccessModel(MODEL_TYPE_CHARM_MIXWING+EWS_ELF_1_CHARM, szPC6Path, "amulet_elf");          
	gLoadData.AccessModel(MODEL_TYPE_CHARM_MIXWING+EWS_SUMMONER_1_CHARM, szPC6Path, "amulet_disaster");       
	gLoadData.AccessModel(MODEL_TYPE_CHARM_MIXWING+EWS_DARKLORD_1_CHARM, szPC6Path, "amulet_cloak");
	gLoadData.AccessModel(MODEL_TYPE_CHARM_MIXWING+EWS_KNIGHT_2_CHARM, szPC6Path, "amulet_dragon");         
	gLoadData.AccessModel(MODEL_TYPE_CHARM_MIXWING+EWS_MAGICIAN_2_CHARM, szPC6Path, "amulet_soul");        
	gLoadData.AccessModel(MODEL_TYPE_CHARM_MIXWING+EWS_ELF_2_CHARM, szPC6Path, "amulet_spirit");         
	gLoadData.AccessModel(MODEL_TYPE_CHARM_MIXWING+EWS_SUMMONER_2_CHARM, szPC6Path, "amulet_despair");     
	gLoadData.AccessModel(MODEL_TYPE_CHARM_MIXWING+EWS_DARKKNIGHT_2_CHARM, szPC6Path, "amulet_dark");        
	gLoadData.AccessModel(MODEL_HELPER+59, "Data\\Item\\partCharge2\\", "sealmove");
	gLoadData.AccessModel(MODEL_HELPER+54, "Data\\Item\\partCharge2\\", "resetfruit_power");
	gLoadData.AccessModel(MODEL_HELPER+55, "Data\\Item\\partCharge2\\", "resetfruit_quick");
	gLoadData.AccessModel(MODEL_HELPER+56, "Data\\Item\\partCharge2\\", "resetfruit_strenth");
	gLoadData.AccessModel(MODEL_HELPER+57, "Data\\Item\\partCharge2\\", "resetfruit_energe");
	gLoadData.AccessModel(MODEL_HELPER+58, "Data\\Item\\partCharge2\\", "resetfruit_command");
	gLoadData.AccessModel(MODEL_POTION+78, "Data\\Item\\partCharge2\\", "secret_power");
	gLoadData.AccessModel(MODEL_POTION+79, "Data\\Item\\partCharge2\\", "secret_quick");
	gLoadData.AccessModel(MODEL_POTION+80, "Data\\Item\\partCharge2\\", "secret_strenth");
	gLoadData.AccessModel(MODEL_POTION+81, "Data\\Item\\partCharge2\\", "secret_energe");
	gLoadData.AccessModel(MODEL_POTION+82, "Data\\Item\\partCharge2\\", "secret_command");
	gLoadData.AccessModel(MODEL_HELPER+60, "Data\\Item\\partCharge2\\", "indulgence");
	gLoadData.AccessModel(MODEL_HELPER+61, "Data\\Item\\partCharge2\\", "entrancepurple");
	gLoadData.AccessModel(MODEL_POTION+83, "Data\\Item\\partCharge2\\", "expensiveitem04b");
	gLoadData.AccessModel(MODEL_POTION+145, "Data\\Item\\partCharge8\\", "rareitem_ticket7");
	gLoadData.AccessModel(MODEL_POTION+146, "Data\\Item\\partCharge8\\", "rareitem_ticket8");
	gLoadData.AccessModel(MODEL_POTION+147, "Data\\Item\\partCharge8\\", "rareitem_ticket9");
	gLoadData.AccessModel(MODEL_POTION+148, "Data\\Item\\partCharge8\\", "rareitem_ticket10");
	gLoadData.AccessModel(MODEL_POTION+149, "Data\\Item\\partCharge8\\", "rareitem_ticket11");
	gLoadData.AccessModel(MODEL_POTION+150, "Data\\Item\\partCharge8\\", "rareitem_ticket12");
	gLoadData.AccessModel(MODEL_HELPER+125, "Data\\Item\\partCharge8\\", "DoppelCard");
	gLoadData.AccessModel(MODEL_HELPER+126, "Data\\Item\\partCharge8\\", "BarcaCard");
	gLoadData.AccessModel(MODEL_HELPER+127, "Data\\Item\\partCharge8\\", "Barca7Card");

#ifdef LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM
	gLoadData.AccessModel(MODEL_HELPER+128, "Data\\Item\\", "HawkStatue");
	gLoadData.AccessModel(MODEL_HELPER+129, "Data\\Item\\", "SheepStatue");
	gLoadData.AccessModel(MODEL_HELPER+134, "Data\\Item\\", "horseshoe");
#endif //LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM
#ifdef LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM_PART_2
	gLoadData.AccessModel(MODEL_HELPER+130, "Data\\Item\\", "ork_cham");
//	gLoadData.AccessModel(MODEL_HELPER+131, "Data\\Item\\", "maple_cham");
	gLoadData.AccessModel(MODEL_HELPER+132, "Data\\Item\\", "goldenork_cham");
//	gLoadData.AccessModel(MODEL_HELPER+132, "Data\\Item\\", "goldenmaple_cham");
#endif //LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM_PART_2	

	gLoadData.AccessModel(MODEL_POTION+91, "Data\\Item\\partCharge3\\", "alicecard");	

	gLoadData.AccessModel(MODEL_POTION+92, "Data\\Item\\partCharge3\\", "juju");	
	gLoadData.AccessModel(MODEL_POTION+93, "Data\\Item\\partCharge3\\", "juju");
	gLoadData.AccessModel(MODEL_POTION+95, "Data\\Item\\partCharge3\\", "juju");

	gLoadData.AccessModel(MODEL_POTION+94, "Data\\Item\\partCharge2\\", "EPotionR");
	gLoadData.AccessModel(MODEL_POTION+84, "Data\\Item\\cherryblossom\\", "cherrybox");
	gLoadData.AccessModel(MODEL_POTION+85, "Data\\Item\\cherryblossom\\", "chwine");
	gLoadData.AccessModel(MODEL_POTION+86, "Data\\Item\\cherryblossom\\", "chgateaux");
	gLoadData.AccessModel(MODEL_POTION+87, "Data\\Item\\cherryblossom\\", "chpetal");
	gLoadData.AccessModel(MODEL_POTION+88, "Data\\Item\\cherryblossom\\", "chbranche");
	gLoadData.AccessModel(MODEL_POTION+89, "Data\\Item\\cherryblossom\\", "chbranche_red");
	gLoadData.AccessModel(MODEL_POTION+90, "Data\\Item\\cherryblossom\\", "chbranche_yellow");
	gLoadData.AccessModel(MODEL_HELPER+62, "Data\\Item\\partCharge4\\", "Curemark");
	gLoadData.AccessModel(MODEL_HELPER+63, "Data\\Item\\partCharge4\\", "Holinessmark");
	gLoadData.AccessModel(MODEL_POTION+97, "Data\\Item\\partCharge4\\", "battlescroll");
	gLoadData.AccessModel(MODEL_POTION+98, "Data\\Item\\partCharge4\\", "strengscroll");

	gLoadData.AccessModel(MODEL_POTION+96, "Data\\Item\\partCharge4\\", "strengamulet");

    gLoadData.AccessModel(MODEL_HELPER+64,"Data\\Item\\partCharge4\\","demon");
    gLoadData.AccessModel(MODEL_HELPER+65,"Data\\Item\\partCharge4\\","maria");

	gLoadData.AccessModel(MODEL_HELPER+80,"Data\\Item\\","PandaPet");
	gLoadData.AccessModel(MODEL_HELPER+67,"Data\\Item\\xmas\\","xmas_deer");
	gLoadData.AccessModel(MODEL_HELPER+106,"Data\\Item\\partcharge7\\","pet_unicorn");
	gLoadData.AccessModel(MODEL_HELPER+123,"Data\\Item\\","skeletonpet");

	gLoadData.AccessModel(MODEL_HELPER+69, "Data\\Item\\partCharge5\\", "ressurection");
	gLoadData.AccessModel(MODEL_HELPER+70, "Data\\Item\\partCharge5\\", "potalcharm");
	gLoadData.AccessModel(MODEL_HELPER+81, "Data\\Item\\partCharge6\\", "suhocham01");
	gLoadData.AccessModel(MODEL_HELPER+82, "Data\\Item\\partCharge6\\", "imteam_protect");
	gLoadData.AccessModel(MODEL_HELPER+93, "Data\\Item\\partCharge6\\", "MasterSealA");

	gLoadData.AccessModel(MODEL_HELPER+94, "Data\\Item\\partCharge6\\", "MasterSealB");
	gLoadData.AccessModel(MODEL_POTION+140, "Data\\Item\\", "strengscroll");


	gLoadData.AccessModel(MODEL_HELPER+49, "Data\\Item\\", "scrollpaper");
	gLoadData.AccessModel(MODEL_HELPER+50, "Data\\Item\\", "oath");
	gLoadData.AccessModel(MODEL_HELPER+51, "Data\\Item\\", "songbl");

	gLoadData.AccessModel(MODEL_HELPER+52, "Data\\Item\\", "condolstone");
	gLoadData.AccessModel(MODEL_HELPER+53, "Data\\Item\\", "condolwing");

	gLoadData.AccessModel(MODEL_POTION+64, "Data\\Item\\", "songss");
	gLoadData.AccessModel(MODEL_POTION+65, "Data\\Item\\", "deathbeamstone");
	gLoadData.AccessModel(MODEL_POTION+66, "Data\\Item\\", "hellhorn");
	gLoadData.AccessModel(MODEL_POTION+67, "Data\\Item\\", "phoenixfeather");
	gLoadData.AccessModel(MODEL_POTION+68, "Data\\Item\\", "Deye");
	
	for (int i = 0; i < 6; ++i)
		gLoadData.AccessModel(MODEL_WING+60+i, "Data\\Item\\", "s30_seed");
	for (int i = 0; i < 5; ++i)
		gLoadData.AccessModel(MODEL_WING+70+i, "Data\\Item\\", "s30_sphere_body", i + 1);
	for (int i = 0; i < 5; ++i)
	{
		for (int j = 0; j < 6; ++j)
			gLoadData.AccessModel(MODEL_WING+100+i*6+j, "Data\\Item\\", "s30_sphere", i + 1);
	}

	gLoadData.AccessModel(MODEL_HELPER+107,"Data\\Item\\partcharge7\\","FatalRing");
	gLoadData.AccessModel(MODEL_HELPER+104,"Data\\Item\\partcharge7\\","injang_AG");
	gLoadData.AccessModel(MODEL_HELPER+105,"Data\\Item\\partcharge7\\","injang_SD");
	gLoadData.AccessModel(MODEL_HELPER+103,"Data\\Item\\partcharge7\\","EXPscroll");
	gLoadData.AccessModel(MODEL_POTION+133,"Data\\Item\\partcharge7\\","ESDPotion");

	gLoadData.AccessModel(MODEL_HELPER+109,"Data\\Item\\InGameShop\\","PeriodRingBlue");

	gLoadData.AccessModel(MODEL_HELPER+110,"Data\\Item\\InGameShop\\","PeriodRingRed");
	gLoadData.AccessModel(MODEL_HELPER+111,"Data\\Item\\InGameShop\\","PeriodRingYellow");
	gLoadData.AccessModel(MODEL_HELPER+112,"Data\\Item\\InGameShop\\","PeriodRingViolet");
	gLoadData.AccessModel(MODEL_HELPER+113,"Data\\Item\\InGameShop\\","necklace_red");
	gLoadData.AccessModel(MODEL_HELPER+114,"Data\\Item\\InGameShop\\","necklace_blue");
	gLoadData.AccessModel(MODEL_HELPER+115,"Data\\Item\\InGameShop\\","necklace_green");

	gLoadData.AccessModel(MODEL_HELPER+116, "Data\\Item\\", "monmark02");

	gLoadData.AccessModel(MODEL_WING+130,"Data\\Item\\","DarkLordRobe");
 	gLoadData.AccessModel(MODEL_WING+131,"Data\\Item\\Ingameshop\\","alice1wing");
	gLoadData.AccessModel(MODEL_WING+132,"Data\\Item\\Ingameshop\\","elf_wing");
	gLoadData.AccessModel(MODEL_WING+133,"Data\\Item\\Ingameshop\\","angel_wing");
	gLoadData.AccessModel(MODEL_WING+134,"Data\\Item\\Ingameshop\\","devil_wing");
	gLoadData.AccessModel(MODEL_HELPER+124, "Data\\Item\\partCharge6\\", "ChannelCard");


    for(int i=0;i<7;i++)
      	gLoadData.AccessModel(MODEL_POTION+i,"Data\\Item\\","Potion",i+1);

   	gLoadData.AccessModel(MODEL_POTION+8 ,"Data\\Item\\","Antidote",1);
   	gLoadData.AccessModel(MODEL_POTION+9 ,"Data\\Item\\","Beer",1);
   	gLoadData.AccessModel(MODEL_POTION+10,"Data\\Item\\","Scroll",1);
   	gLoadData.AccessModel(MODEL_POTION+11,"Data\\Item\\","MagicBox",1);
   	gLoadData.AccessModel(MODEL_POTION+12,"Data\\Item\\","Event",1);

	for(int i=0;i<2;i++)
        gLoadData.AccessModel(MODEL_POTION+i+13,"Data\\Item\\","Jewel",i+1);

    gLoadData.AccessModel(MODEL_POTION+15,"Data\\Item\\","Gold",1);
    gLoadData.AccessModel(MODEL_POTION+16,"Data\\Item\\","Jewel",3);

	for(int i=0;i<3;i++)
		gLoadData.AccessModel(MODEL_POTION+17+i,"Data\\Item\\","Devil",i);

	gLoadData.AccessModel(MODEL_POTION + 20, "Data\\Item\\", "Drink", 0);
    gLoadData.AccessModel(MODEL_POTION+21,"Data\\Item\\","ConChip",0);
    gLoadData.AccessModel(MODEL_POTION+31,"Data\\Item\\","suho",-1);
	gLoadData.AccessModel(MODEL_HELPER+38,"Data\\Item\\","kanneck2");
	gLoadData.AccessModel(MODEL_POTION+41,"Data\\Item\\","rs");
	gLoadData.AccessModel(MODEL_POTION+42,"Data\\Item\\","jos");
	gLoadData.AccessModel(MODEL_POTION+43,"Data\\Item\\","LowRefineStone");
	gLoadData.AccessModel(MODEL_POTION+44,"Data\\Item\\","HighRefineStone");

	gLoadData.AccessModel(MODEL_POTION+7, "Data\\Item\\","SpecialPotion" );

    for ( int i=0; i<4; ++i )
    {
        gLoadData.AccessModel(MODEL_POTION+23+i, "Data\\Item\\", "Quest", i);
    }
    gLoadData.AccessModel(MODEL_POTION+27, "Data\\Item\\", "godesteel" );

	for(int i=0; i<2; i++) 
	{
		gLoadData.AccessModel ( MODEL_POTION+28+i, "Data\\Item\\", "HELLASITEM", i);
	}

	for(int i=0; i<3; ++i)
		gLoadData.AccessModel ( MODEL_POTION+35+i, "Data\\Item\\", "sdwater", i+1);

	for(int i=0; i<3; ++i)
		gLoadData.AccessModel ( MODEL_POTION+38+i, "Data\\Item\\", "megawater", i+1);
	
	gLoadData.AccessModel(MODEL_POTION+120,"Data\\Item\\InGameShop\\","gold_coin");
	gLoadData.AccessModel(MODEL_POTION+121,"Data\\Item\\InGameShop\\","itembox_gold");
	gLoadData.AccessModel(MODEL_POTION+122,"Data\\Item\\InGameShop\\","itembox_silver");
	gLoadData.AccessModel(MODEL_POTION+123,"Data\\Item\\InGameShop\\","itembox_gold");
	gLoadData.AccessModel(MODEL_POTION+124,"Data\\Item\\InGameShop\\","itembox_silver");
	for(int k=0; k<6; k++)
	{
		gLoadData.AccessModel(MODEL_POTION+134+k,"Data\\Item\\InGameShop\\","package_money_item");
	}

	gLoadData.AccessModel(MODEL_COMPILED_CELE,"Data\\Item\\","Jewel",1);
	gLoadData.AccessModel(MODEL_COMPILED_SOUL,"Data\\Item\\","Jewel",2);
	gLoadData.AccessModel(MODEL_WING+136, "Data\\Item\\", "Jewel",	3);
	gLoadData.AccessModel(MODEL_WING+137, "Data\\Item\\", "jewel",	22);
	gLoadData.AccessModel(MODEL_WING+138, "Data\\Item\\", "suho", -1);
	gLoadData.AccessModel(MODEL_WING+139, "Data\\Item\\", "rs");
	gLoadData.AccessModel(MODEL_WING+140, "Data\\Item\\", "jos");
	gLoadData.AccessModel(MODEL_WING+141, "Data\\Item\\", "Jewel", 15);
	gLoadData.AccessModel(MODEL_WING+142, "Data\\Item\\", "LowRefineStone");
	gLoadData.AccessModel(MODEL_WING+143, "Data\\Item\\", "HighRefineStone");

    gLoadData.AccessModel(MODEL_EVENT+4,"Data\\Item\\","MagicBox",2);
	gLoadData.AccessModel(MODEL_EVENT+6,"Data\\Item\\","MagicBox",5);
	gLoadData.AccessModel(MODEL_EVENT+7,"Data\\Item\\","Beer",2);
	gLoadData.AccessModel(MODEL_EVENT+8,"Data\\Item\\","MagicBox",6);
	gLoadData.AccessModel(MODEL_EVENT+9,"Data\\Item\\","MagicBox",7);

	gLoadData.AccessModel(MODEL_HELPER+66, "Data\\Item\\xmas\\", "santa_village", -1);

	gLoadData.AccessModel(MODEL_POTION+100, "Data\\Item\\", "coin7", -1);

	g_XmasEvent->LoadXmasEventItem();

	gLoadData.AccessModel(MODEL_EVENT+10,"Data\\Item\\","MagicBox",8);

	gLoadData.AccessModel(MODEL_EVENT+5,"Data\\Item\\","MagicBox",3);
	

	gLoadData.AccessModel(MODEL_POTION+63,"Data\\Item\\", "GM", 1);
	gLoadData.AccessModel(MODEL_POTION+52,"Data\\Item\\", "GM", 2);

    gLoadData.AccessModel(MODEL_EVENT    ,"Data\\Item\\","Event",2);
   	gLoadData.AccessModel(MODEL_EVENT+1  ,"Data\\Item\\","Event",3);
	
	gLoadData.AccessModel(MODEL_POTION+99,"Data\\Item\\XMas\\", "xmasfire", -1);

	for ( int i=0; i<4; i++ )
    {
        if ( i<3 )
        {
            gLoadData.AccessModel( MODEL_HELPER+16+i, "Data\\Item\\", "EventBloodCastle", i );
        }
        else
        {
            gLoadData.AccessModel( MODEL_EVENT+11+(i-3), "Data\\Item\\", "EventBloodCastle", i );
        }
    }
	
	gLoadData.AccessModel( MODEL_EVENT+12, "Data\\Item\\", "QuestItem3RD", 0);
	gLoadData.AccessModel( MODEL_EVENT+13, "Data\\Item\\", "QuestItem3RD", 1);
	gLoadData.AccessModel( MODEL_EVENT+14, "Data\\Item\\", "RingOfLordEvent", 0);
	gLoadData.AccessModel( MODEL_EVENT+15, "Data\\Item\\", "MagicRing", 0);

    gLoadData.AccessModel(MODEL_POTION+22, "Data\\Item\\", "jewel", 22);

    for ( int i=0; i<2; ++i )
    {
        gLoadData.AccessModel(MODEL_HELPER+14+i, "Data\\Item\\", "Quest", 4+i);
    }

	gLoadData.AccessModel ( MODEL_EVENT+16,  "Data\\Item\\", "DarkLordSleeve" );
	gLoadData.AccessModel ( MODEL_HELPER+30, "Data\\Item\\", "DarkLordRobe" ); 

    for(int i=0;i<3;i++)
      	gLoadData.AccessModel(MODEL_WING+i,"Data\\Item\\","Wing"  ,i+1);

    for ( int i=0; i<4; i++ )
    {
        gLoadData.AccessModel(MODEL_WING+3+i,"Data\\Item\\","Wing"  ,4+i);
    }

    for ( int i=0; i<4; i++ )
    {
        gLoadData.AccessModel(MODEL_WING+36+i,"Data\\Item\\","Wing"  ,8+i);
    }
	gLoadData.AccessModel ( MODEL_WING+40, "Data\\Item\\", "DarkLordRobe02" );

	for (int i = 41; i <= 43; ++i)
		::gLoadData.AccessModel(MODEL_WING+i, "Data\\Item\\", "Wing", i+1);

	gLoadData.AccessModel(MODEL_STAFF+21, "Data\\Item\\", "Book_of_Sahamutt");
	gLoadData.AccessModel(MODEL_STAFF+22, "Data\\Item\\", "Book_of_Neil");
	gLoadData.AccessModel(MODEL_STAFF+23, "Data\\Item\\", "Book_of_Rargle");

	for(int i=0; i<9; ++i)
	{
		gLoadData.AccessModel(MODEL_ETC+19+i, "Data\\Item\\", "rollofpaper");
	}

    for(int i=0;i<13;i++)
	{
		if ( i+7 != 15)
	      	gLoadData.AccessModel(MODEL_WING+i+7,"Data\\Item\\","Gem",i+1);
	}

    gLoadData.AccessModel(MODEL_WING+15,"Data\\Item\\","Jewel",15);
	gLoadData.AccessModel(MODEL_WING+20,"Data\\Item\\","Gem", 14 );
	
	gLoadData.AccessModel(MODEL_WING+44,"Data\\Item\\","Gem",6);

	gLoadData.AccessModel(MODEL_WING+45,"Data\\Item\\","Gem",6);

	gLoadData.AccessModel(MODEL_WING+46,"Data\\Item\\","Gem",6);

	gLoadData.AccessModel(MODEL_WING+47,"Data\\Item\\","Gem",6);

    for ( int i=0; i<4; i++ )
    {
        gLoadData.AccessModel ( MODEL_WING+21+i,"Data\\Item\\","SkillScroll" );
    }
	gLoadData.AccessModel ( MODEL_WING+35,"Data\\Item\\","SkillScroll" );

	gLoadData.AccessModel ( MODEL_WING+48,"Data\\Item\\","SkillScroll" );

	gLoadData.AccessModel( MODEL_POTION+45,"Data\\Item\\","hobakhead");
	gLoadData.AccessModel( MODEL_POTION+46,"Data\\Item\\","hellowinscroll");
	gLoadData.AccessModel( MODEL_POTION+47,"Data\\Item\\","hellowinscroll");
	gLoadData.AccessModel( MODEL_POTION+48,"Data\\Item\\","hellowinscroll");
	gLoadData.AccessModel( MODEL_POTION+49,"Data\\Item\\","Gogi");
	gLoadData.AccessModel( MODEL_POTION+50,"Data\\Item\\","pumpkincup");

	gLoadData.AccessModel( MODEL_POTION+32,"Data\\Item\\","giftbox_bp");
	gLoadData.AccessModel( MODEL_POTION+33,"Data\\Item\\","giftbox_br");
	gLoadData.AccessModel( MODEL_POTION+34,"Data\\Item\\","giftbox_bb");

	gLoadData.AccessModel( MODEL_EVENT+21,"Data\\Item\\","p03box");
	gLoadData.AccessModel( MODEL_EVENT+22,"Data\\Item\\","obox02");
	gLoadData.AccessModel( MODEL_EVENT+23,"Data\\Item\\","blue01");

	gLoadData.AccessModel( MODEL_WING+32,"Data\\Item\\","giftbox_r");
	gLoadData.AccessModel( MODEL_WING+33,"Data\\Item\\","giftbox_g");
	gLoadData.AccessModel( MODEL_WING+34,"Data\\Item\\","giftbox_b");


	for(int i=0;i<19;i++)
      	gLoadData.AccessModel(MODEL_ETC+i,"Data\\Item\\","Book",i+1);
	
	gLoadData.AccessModel(MODEL_ETC+29,"Data\\Item\\","Book",18);

	gLoadData.AccessModel(MODEL_ETC+28,"Data\\Item\\","Book",18);	


	gLoadData.AccessModel(MODEL_SWORD+22, "Data\\Item\\", "HDK_Sword");
	gLoadData.AccessModel(MODEL_SWORD+23, "Data\\Item\\", "HDK_Sword2");
	gLoadData.AccessModel(MODEL_MACE+14, "Data\\Item\\", "HDK_Mace");
	gLoadData.AccessModel(MODEL_BOW+21, "Data\\Item\\", "HDK_Bow");
	gLoadData.AccessModel(MODEL_STAFF+12, "Data\\Item\\", "HDK_Staff");
	
	gLoadData.AccessModel(MODEL_SWORD+24, "Data\\Item\\", "CW_Sword");
	gLoadData.AccessModel(MODEL_SWORD+25, "Data\\Item\\", "CW_Sword2");
	gLoadData.AccessModel(MODEL_MACE+15, "Data\\Item\\", "CW_Mace");
	gLoadData.AccessModel(MODEL_BOW+22, "Data\\Item\\", "CW_Bow");
	gLoadData.AccessModel(MODEL_STAFF+13, "Data\\Item\\", "CW_Staff");
	
	gLoadData.AccessModel(MODEL_SWORD+26, "Data\\Item\\", "Sword_27");
	gLoadData.AccessModel(MODEL_SWORD+27, "Data\\Item\\", "Sword_28");
	gLoadData.AccessModel(MODEL_SWORD+28, "Data\\Item\\", "Sword_29");
	gLoadData.AccessModel(MODEL_MACE+16, "Data\\Item\\", "Mace_17");
	gLoadData.AccessModel(MODEL_MACE+17, "Data\\Item\\", "Mace_18");
	gLoadData.AccessModel(MODEL_BOW+23, "Data\\Item\\", "Bow_24");
	gLoadData.AccessModel(MODEL_STAFF+30, "Data\\Item\\", "Staff_31");
	gLoadData.AccessModel(MODEL_STAFF+31, "Data\\Item\\", "Staff_32");
	gLoadData.AccessModel(MODEL_STAFF+32, "Data\\Item\\", "Staff_33");
	gLoadData.AccessModel(MODEL_SHIELD+17, "Data\\Item\\", "Shield_18");
	gLoadData.AccessModel(MODEL_SHIELD+18, "Data\\Item\\", "Shield_19");
	gLoadData.AccessModel(MODEL_SHIELD+19, "Data\\Item\\", "Shield_20");
	gLoadData.AccessModel(MODEL_SHIELD+20, "Data\\Item\\", "Shield_21");

	gLoadData.AccessModel(MODEL_SHIELD+21, "Data\\Item\\", "crosssheild");

	gLoadData.AccessModel(MODEL_BOW+24, "Data\\Item\\", "gamblebow");
	gLoadData.AccessModel(MODEL_STAFF+33, "Data\\Item\\", "gamble_wand");
	gLoadData.AccessModel(MODEL_STAFF+34, "Data\\Item\\", "gamble_stick");
	gLoadData.AccessModel(MODEL_SPEAR+11, "Data\\Item\\", "gamble_scyder01");
	gLoadData.AccessModel(MODEL_MACE+18, "Data\\Item\\", "gamble_safter01");

	gLoadData.AccessModel(MODEL_HELPER+71, "Data\\Item\\", "gamble_scyderx01");
	gLoadData.AccessModel(MODEL_HELPER+72, "Data\\Item\\", "gamble_wand01");
	gLoadData.AccessModel(MODEL_HELPER+73, "Data\\Item\\", "gamble_bowx01");
	gLoadData.AccessModel(MODEL_HELPER+74, "Data\\Item\\", "gamble_safterx01");
 	gLoadData.AccessModel(MODEL_HELPER+75, "Data\\Item\\", "gamble_stickx01");

	gLoadData.AccessModel(MODEL_HELPER+97, "Data\\Item\\Ingameshop\\", "charactercard");
	gLoadData.AccessModel(MODEL_HELPER+98, "Data\\Item\\Ingameshop\\", "charactercard");
	gLoadData.AccessModel(MODEL_POTION+91, "Data\\Item\\partCharge3\\", "alicecard");

#ifdef PBG_ADD_CHARACTERSLOT
	gLoadData.AccessModel(MODEL_HELPER+99, "Data\\Item\\Ingameshop\\", "key");
	gLoadData.AccessModel(MODEL_SLOT_LOCK, "Data\\Item\\Ingameshop\\", "lock");
#endif //PBG_ADD_CHARACTERSLOT
#ifdef PBG_ADD_SECRETITEM
#ifdef PBG_MOD_SECRETITEM
	gLoadData.AccessModel(MODEL_HELPER+117, "Data\\Item\\Ingameshop\\", "FRpotionD");
#else //PBG_MOD_SECRETITEM
	gLoadData.AccessModel(MODEL_HELPER+117, "Data\\Item\\Ingameshop\\", "FRpotionA");
#endif //PBG_MOD_SECRETITEM
	gLoadData.AccessModel(MODEL_HELPER+118, "Data\\Item\\Ingameshop\\", "FRpotionA");
	gLoadData.AccessModel(MODEL_HELPER+119, "Data\\Item\\Ingameshop\\", "FRpotionB");
	gLoadData.AccessModel(MODEL_HELPER+120, "Data\\Item\\Ingameshop\\", "FRpotionC");
#endif //PBG_ADD_SECRETITEM

	gLoadData.AccessModel(MODEL_POTION+110, "Data\\Item\\","indication");
	gLoadData.AccessModel(MODEL_POTION+111, "Data\\Item\\","speculum");

	gLoadData.AccessModel(MODEL_POTION+101, "Data\\Item\\","doubt_paper");
	gLoadData.AccessModel(MODEL_POTION+102, "Data\\Item\\","warrant");
	gLoadData.AccessModel(MODEL_POTION+109, "Data\\Item\\","secromicon");
	for(int c=0; c<6; c++)
	{
		gLoadData.AccessModel(MODEL_POTION+103+c, "Data\\Item\\","secromicon_piece");
	}

	gLoadData.AccessModel(MODEL_POTION+112, "Data\\Item\\Ingameshop\\","ItemBoxKey_silver");

	gLoadData.AccessModel(MODEL_POTION+113, "Data\\Item\\Ingameshop\\","ItemBoxKey_gold");

	{
		for(int c=0; c<6; c++)
		{
			gLoadData.AccessModel(MODEL_POTION+114+c, "Data\\Item\\Ingameshop\\","primium_membership_item");
		}
	}
	{
		for(int c=0; c<4; c++)
		{
			gLoadData.AccessModel(MODEL_POTION+126+c, "Data\\Item\\Ingameshop\\","primium_membership_item");
		}
	}
	{
		for(int c=0; c<3; c++)
		{
			gLoadData.AccessModel(MODEL_POTION+130+c, "Data\\Item\\Ingameshop\\","primium_membership_item");
		}
	}
	{
		gLoadData.AccessModel(MODEL_HELPER+121, "Data\\Item\\Ingameshop\\", "entrancegreen");
	}
	gLoadData.AccessModel(MODEL_POTION+141, "Data\\Item\\", "requitalbox_red");
	gLoadData.AccessModel(MODEL_POTION+142, "Data\\Item\\", "requitalbox_violet");
	gLoadData.AccessModel(MODEL_POTION+143, "Data\\Item\\", "requitalbox_blue");
	gLoadData.AccessModel(MODEL_POTION+144, "Data\\Item\\", "requitalbox_wood");

	gLoadData.AccessModel(MODEL_15GRADE_ARMOR_OBJ_ARMLEFT,	"Data\\Item\\", "class15_armleft");	
	gLoadData.AccessModel(MODEL_15GRADE_ARMOR_OBJ_ARMRIGHT,	"Data\\Item\\", "class15_armright");
	gLoadData.AccessModel(MODEL_15GRADE_ARMOR_OBJ_BODYLEFT,	"Data\\Item\\", "class15_bodyleft");
	gLoadData.AccessModel(MODEL_15GRADE_ARMOR_OBJ_BODYRIGHT,	"Data\\Item\\", "class15_bodyright");
	gLoadData.AccessModel(MODEL_15GRADE_ARMOR_OBJ_BOOTLEFT,	"Data\\Item\\", "class15_bootleft");
	gLoadData.AccessModel(MODEL_15GRADE_ARMOR_OBJ_BOOTRIGHT,	"Data\\Item\\", "class15_bootright");
	gLoadData.AccessModel(MODEL_15GRADE_ARMOR_OBJ_HEAD,		"Data\\Item\\", "class15_head");
	gLoadData.AccessModel(MODEL_15GRADE_ARMOR_OBJ_PANTLEFT,	"Data\\Item\\", "class15_pantleft");
	gLoadData.AccessModel(MODEL_15GRADE_ARMOR_OBJ_PANTRIGHT,	"Data\\Item\\", "class15_pantright");

#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
	gLoadData.AccessModel(MODEL_WING+49, "Data\\Item\\", "Wing", 50);
	gLoadData.AccessModel(MODEL_WING+50, "Data\\Item\\", "Wing", 51);
	gLoadData.AccessModel(MODEL_WING+135, "Data\\Item\\", "Wing", 50);
	LoadBitmap("Item\\NCcape.tga", BITMAP_NCCAPE, GL_LINEAR, GL_REPEAT);
	LoadBitmap("Item\\monk_manto01.TGA", BITMAP_MANTO01, GL_LINEAR, GL_REPEAT);
	LoadBitmap("Item\\monke_manto.TGA", BITMAP_MANTOE, GL_LINEAR, GL_REPEAT);
	g_CMonkSystem.LoadModelItem();
	for(int _nRollIndex=0; _nRollIndex<7; ++_nRollIndex)
		gLoadData.AccessModel(MODEL_ETC+30+_nRollIndex, "Data\\Item\\", "rollofpaper");
#endif //PBG_ADD_NEWCHAR_MONK_ITEM

#ifdef LEM_ADD_LUCKYITEM	// LuckyItem Data AccesModel
	gLoadData.AccessModel(MODEL_HELPER+135, "Data\\Item\\LuckyItem\\", "LuckyCardgreen");				
	gLoadData.AccessModel(MODEL_HELPER+136, "Data\\Item\\LuckyItem\\", "LuckyCardgreen");		
	gLoadData.AccessModel(MODEL_HELPER+137, "Data\\Item\\LuckyItem\\", "LuckyCardgreen");		
	gLoadData.AccessModel(MODEL_HELPER+138, "Data\\Item\\LuckyItem\\", "LuckyCardgreen");		
	gLoadData.AccessModel(MODEL_HELPER+139, "Data\\Item\\LuckyItem\\", "LuckyCardgreen");		
	gLoadData.AccessModel(MODEL_HELPER+140, "Data\\Item\\LuckyItem\\", "LuckyCardred");
	gLoadData.AccessModel(MODEL_HELPER+141, "Data\\Item\\LuckyItem\\", "LuckyCardred");		
	gLoadData.AccessModel(MODEL_HELPER+142, "Data\\Item\\LuckyItem\\", "LuckyCardred");		
	gLoadData.AccessModel(MODEL_HELPER+143, "Data\\Item\\LuckyItem\\", "LuckyCardred");		
	gLoadData.AccessModel(MODEL_HELPER+144, "Data\\Item\\LuckyItem\\", "LuckyCardred");
	
	gLoadData.AccessModel(MODEL_POTION+160, "Data\\Item\\LuckyItem\\", "lucky_items01");
	gLoadData.AccessModel(MODEL_POTION+161, "Data\\Item\\LuckyItem\\", "lucky_items02");

	char	szLuckySetFileName[][50] = { "new_Helm", "new_Armor", "new_Pant", "new_Glove", "new_Boot" };
	char*	szLuckySetPath			 = { "Data\\Player\\LuckyItem\\" };
	char	szLuckySetPathName[50]	 = { "" };
	int		nIndex					 = 62;

	for( int i=0; i<11; i++ )
	{
		sprintf( szLuckySetPathName, "%s%d\\", szLuckySetPath, nIndex );
		if( nIndex != 71 )	gLoadData.AccessModel(MODEL_HELM+nIndex,	szLuckySetPathName, szLuckySetFileName[0], i+1);
							gLoadData.AccessModel(MODEL_ARMOR+nIndex,	szLuckySetPathName, szLuckySetFileName[1], i+1);
							gLoadData.AccessModel(MODEL_PANTS+nIndex,	szLuckySetPathName, szLuckySetFileName[2], i+1);
		if( nIndex != 72 )	gLoadData.AccessModel(MODEL_GLOVES+nIndex,szLuckySetPathName, szLuckySetFileName[3], i+1);
							gLoadData.AccessModel(MODEL_BOOTS+nIndex,	szLuckySetPathName, szLuckySetFileName[4], i+1);
		nIndex++;
	}
#endif // LEM_ADD_LUCKYITEM

    Models[MODEL_SPEAR].Meshs[1].NoneBlendMesh = true;
    Models[MODEL_SWORD+10].Meshs[1].NoneBlendMesh = true;
    Models[MODEL_STAFF+6].Meshs[2].NoneBlendMesh = true;
    Models[MODEL_MACE+6].Meshs[1].NoneBlendMesh = true;
	Models[MODEL_EVENT+9].Meshs[1].NoneBlendMesh = true;
}


void OpenItemTextures()
{
    for ( int i=0; i<4; i++ )
    {
        if ( i<3 )
        {
            gLoadData.OpenTexture( MODEL_HELPER+16+i, "Item\\" );
        }
        else
        {
            gLoadData.OpenTexture( MODEL_EVENT+11+(i-3), "Item\\" );
        }
    }
	gLoadData.OpenTexture( MODEL_EVENT+12, "Item\\" );
	gLoadData.OpenTexture( MODEL_EVENT+13, "Item\\" );
	gLoadData.OpenTexture( MODEL_EVENT+14, "Item\\" );
	gLoadData.OpenTexture( MODEL_EVENT+15, "Item\\" );

    for( int i=0; i<4; i++ )
    {
        gLoadData.OpenTexture( MODEL_WING+3+i, "Item\\" );
    }

    for ( int i=0; i<4; i++ )
    {
        gLoadData.OpenTexture( MODEL_WING+36+i, "Item\\" );
    }
	gLoadData.OpenTexture ( MODEL_WING+40, "Item\\" );

	LoadBitmap("Item\\msword01_r.jpg"     ,BITMAP_3RDWING_LAYER    ,GL_LINEAR,GL_REPEAT);

	for (int i = 41; i <= 43; ++i)
        ::gLoadData.OpenTexture(MODEL_WING+i, "Item\\");



	for (int i = 21; i <= 23; ++i)
		::gLoadData.OpenTexture(MODEL_STAFF+i, "Item\\");

	for(int i=0; i<9; ++i)
	{
		gLoadData.OpenTexture(MODEL_ETC+19+i, "Item\\");
	}

	LoadBitmap("Item\\rollofpaper_R.jpg", BITMAP_ROOLOFPAPER_EFFECT_R, GL_LINEAR);

    gLoadData.OpenTexture( MODEL_HELPER+4, "Skill\\" );
	gLoadData.OpenTexture( MODEL_HELPER+4, "Item\\" );
    gLoadData.OpenTexture( MODEL_HELPER+31, "Item\\" );
    gLoadData.OpenTexture( MODEL_HELPER+5, "Skill\\" );
	gLoadData.OpenTexture( MODEL_HELPER+5, "Item\\" );

    gLoadData.OpenTexture(MODEL_POTION+22, "Item\\");

    gLoadData.OpenTexture(MODEL_POTION+31, "Item\\");

	gLoadData.OpenTexture( MODEL_POTION+7,  "Item\\" );
	gLoadData.OpenTexture( MODEL_HELPER+7,  "Item\\" );
	gLoadData.OpenTexture( MODEL_HELPER+11, "Item\\" );
    gLoadData.OpenTexture( MODEL_EVENT+18,  "Monster\\" );

    for(int i=0;i<2;i++)
        gLoadData.OpenTexture(MODEL_HELPER+14+i, "Item\\");

    gLoadData.OpenTexture( MODEL_SWORD+17, "Item\\" );
    gLoadData.OpenTexture( MODEL_SWORD+18, "Item\\" );

    gLoadData.OpenTexture( MODEL_STAFF+9, "Item\\" );
    gLoadData.OpenTexture( MODEL_BOW+17,  "Item\\" );

    gLoadData.OpenTexture( MODEL_HELPER+3, "Skill\\" );

    for(int i=0;i<4;i++)
        gLoadData.OpenTexture(MODEL_POTION+23+i, "Item\\");

    gLoadData.OpenTexture(MODEL_POTION+27, "Item\\");

	for(int i=0;i<2;i++)
		gLoadData.OpenTexture(MODEL_POTION+28+i, "Item\\");

    gLoadData.OpenTexture(MODEL_HELPER+29, "Npc\\");
    gLoadData.OpenTexture( MODEL_SWORD+19, "Item\\" );
    gLoadData.OpenTexture( MODEL_STAFF+10, "Item\\" );
    gLoadData.OpenTexture( MODEL_BOW+18,   "Item\\" );
    gLoadData.OpenTexture( MODEL_BOW+19,   "Item\\" );

    gLoadData.OpenTexture( MODEL_SWORD+31,  "Item\\" );
	gLoadData.OpenTexture( MODEL_SHIELD+15, "Item\\" );
	gLoadData.OpenTexture( MODEL_SHIELD+16, "Item\\" );
	gLoadData.OpenTexture( MODEL_SPEAR+10,  "Item\\" );
	gLoadData.OpenTexture( MODEL_MACE+7,	  "Item\\" );

	for(int i=0;i<17;i++)
	{
		gLoadData.OpenTexture(MODEL_SWORD+i ,"Item\\");
		gLoadData.OpenTexture(MODEL_AXE+i   ,"Item\\");
		gLoadData.OpenTexture(MODEL_MACE+i  ,"Item\\");
		gLoadData.OpenTexture(MODEL_SPEAR+i ,"Item\\");
		gLoadData.OpenTexture(MODEL_STAFF+i ,"Item\\");
		gLoadData.OpenTexture(MODEL_SHIELD+i,"Item\\");
		gLoadData.OpenTexture(MODEL_BOW+i   ,"Item\\");
		gLoadData.OpenTexture(MODEL_HELPER+i,"Item\\");
		gLoadData.OpenTexture(MODEL_WING+i  ,"Item\\");
		gLoadData.OpenTexture(MODEL_POTION+i,"Item\\");
		gLoadData.OpenTexture(MODEL_ETC+i   ,"Item\\");
	}

	for (int i = 14; i <= 20; ++i)
		gLoadData.OpenTexture(MODEL_STAFF+i, "Item\\");

    for ( int i=21; i<=28; ++i )
        gLoadData.OpenTexture(MODEL_HELPER+i, "Item\\");
	//. MODEL_MACE+8,9,10,11
	for(int i=0; i<5; i++)
		gLoadData.OpenTexture(MODEL_MACE+8+i, "Item\\" );

	for(int i=0; i<2; i++)
		gLoadData.OpenTexture(MODEL_SWORD+20+i, "Item\\");

	gLoadData.OpenTexture(MODEL_BOW+20, "Item\\");

	for(int i=17;i<21;i++)
	{
		gLoadData.OpenTexture(MODEL_WING+i  ,"Item\\");
	}
    for ( int i=0; i<4; i++ )
    {
		gLoadData.OpenTexture ( MODEL_WING+21+i, "Item\\" );
    }

	gLoadData.OpenTexture ( MODEL_WING+48, "Item\\" );
	gLoadData.OpenTexture ( MODEL_WING+35, "Item\\" );
	gLoadData.OpenTexture(MODEL_WING+44, "Item\\" );
	gLoadData.OpenTexture(MODEL_WING+45, "Item\\");
	gLoadData.OpenTexture(MODEL_WING+46, "Item\\" );
	gLoadData.OpenTexture(MODEL_WING+47, "Item\\" );
	gLoadData.OpenTexture(MODEL_ETC+29, "Item\\" );
	gLoadData.OpenTexture( MODEL_ETC+28, "Item\\" );

	for( int i=17; i<19; ++i )
	{
		gLoadData.OpenTexture(MODEL_ETC+i, "Item\\" );
	}

    gLoadData.OpenTexture(MODEL_ARROW      ,"Item\\Bow\\");

    gLoadData.OpenTexture(MODEL_EVENT+4	 ,"Item\\");
    gLoadData.OpenTexture(MODEL_EVENT+5	 ,"Item\\");
	gLoadData.OpenTexture(MODEL_EVENT+6	 ,"Item\\");
    gLoadData.OpenTexture(MODEL_EVENT+7	 ,"Item\\");
	gLoadData.OpenTexture(MODEL_EVENT+8	 ,"Item\\");
	gLoadData.OpenTexture(MODEL_EVENT+9	 ,"Item\\");
	gLoadData.OpenTexture(MODEL_EVENT+10	 ,"Item\\");
	gLoadData.OpenTexture ( MODEL_EVENT+16,  "Item\\" );
	gLoadData.OpenTexture ( MODEL_HELPER+30, "Item\\" );

    for(int i=0;i<3;i++)
		gLoadData.OpenTexture(MODEL_POTION+17+i	,"Item\\");

    for(int i=0;i<2;i++)
        gLoadData.OpenTexture(MODEL_POTION+20+i, "Item\\");

	for(int i=0;i<6;++i)
		gLoadData.OpenTexture(MODEL_POTION+35+i, "Item\\");
	
	for(int i=0;i<2;i++)
        gLoadData.OpenTexture(MODEL_EVENT+i ,"Item\\");

    //gLoadData.OpenTexture(MODEL_GOLD01  ,"Data\\Item\\Etc\\");
    //gLoadData.OpenTexture(MODEL_APPLE01 ,"Data\\Item\\Etc\\");
    //gLoadData.OpenTexture(MODEL_BOTTLE01,"Data\\Item\\Etc\\");
	gLoadData.OpenTexture(MODEL_COMPILED_CELE, "Item\\");
	gLoadData.OpenTexture(MODEL_COMPILED_SOUL, "Item\\");

	gLoadData.OpenTexture(MODEL_WING+136, "Item\\" );
	gLoadData.OpenTexture(MODEL_WING+137, "Item\\" );
	gLoadData.OpenTexture(MODEL_WING+138, "Item\\" );
	gLoadData.OpenTexture(MODEL_WING+139, "Item\\" );
	gLoadData.OpenTexture(MODEL_WING+140, "Item\\" );
	gLoadData.OpenTexture(MODEL_WING+141, "Item\\" );
	gLoadData.OpenTexture(MODEL_WING+142, "Item\\" );
	gLoadData.OpenTexture(MODEL_WING+143, "Item\\" );

	gLoadData.OpenTexture(MODEL_SWORD+22, "Item\\");
	gLoadData.OpenTexture(MODEL_SWORD+23, "Item\\");
	gLoadData.OpenTexture(MODEL_MACE+14, "Item\\");
	gLoadData.OpenTexture(MODEL_BOW+21, "Item\\");
	gLoadData.OpenTexture(MODEL_STAFF+12, "Item\\");

	gLoadData.OpenTexture(MODEL_POTION+45, "Item\\");
	gLoadData.OpenTexture(MODEL_POTION+46, "Item\\");
	gLoadData.OpenTexture(MODEL_POTION+47, "Item\\");
	gLoadData.OpenTexture(MODEL_POTION+48, "Item\\");
	gLoadData.OpenTexture(MODEL_POTION+49, "Item\\");
	gLoadData.OpenTexture(MODEL_POTION+50, "Item\\");
	gLoadData.OpenTexture(MODEL_POTION+32, "Item\\");
	gLoadData.OpenTexture(MODEL_POTION+33, "Item\\");
	gLoadData.OpenTexture(MODEL_POTION+34, "Item\\");

	gLoadData.OpenTexture(MODEL_EVENT+21, "Item\\");
	gLoadData.OpenTexture(MODEL_EVENT+22, "Item\\");
	gLoadData.OpenTexture(MODEL_EVENT+23, "Item\\");
	gLoadData.OpenTexture(MODEL_HELPER+38,"Item\\");
	gLoadData.OpenTexture(MODEL_POTION+41,"Item\\");
	gLoadData.OpenTexture(MODEL_POTION+42,"Item\\");
	gLoadData.OpenTexture(MODEL_POTION+43,"Item\\");
	gLoadData.OpenTexture(MODEL_POTION+44,"Item\\");
	gLoadData.OpenTexture(MODEL_WING+32, "Item\\");
	gLoadData.OpenTexture(MODEL_WING+33, "Item\\");
	gLoadData.OpenTexture(MODEL_WING+34, "Item\\");

	gLoadData.OpenTexture(MODEL_SWORD+24, "Item\\");
	gLoadData.OpenTexture(MODEL_SWORD+25, "Item\\");
	gLoadData.OpenTexture(MODEL_MACE+15, "Item\\");
	gLoadData.OpenTexture(MODEL_BOW+22, "Item\\");
	gLoadData.OpenTexture(MODEL_STAFF+13, "Item\\");

	gLoadData.OpenTexture(MODEL_HELPER+32, "Item\\");
	gLoadData.OpenTexture(MODEL_HELPER+33, "Item\\");
	gLoadData.OpenTexture(MODEL_HELPER+34, "Item\\");
	gLoadData.OpenTexture(MODEL_HELPER+35, "Item\\");
	gLoadData.OpenTexture(MODEL_HELPER+36, "Item\\");
	gLoadData.OpenTexture(MODEL_HELPER+37, "Item\\");

	gLoadData.OpenTexture(MODEL_HELPER+46, "Item\\partCharge1\\");
	gLoadData.OpenTexture(MODEL_HELPER+47, "Item\\partCharge1\\");
	gLoadData.OpenTexture(MODEL_HELPER+48, "Item\\partCharge1\\");
	
	gLoadData.OpenTexture(MODEL_POTION+54, "Item\\partCharge1\\");

	gLoadData.OpenTexture(MODEL_HELPER+43, "Item\\partCharge1\\");
	gLoadData.OpenTexture(MODEL_HELPER+44, "Item\\partCharge1\\");
	gLoadData.OpenTexture(MODEL_HELPER+45, "Item\\partCharge1\\");

	gLoadData.OpenTexture(MODEL_POTION+53, "Item\\partCharge1\\");
	
	for(int i=0; i<5; ++i)
	{
		gLoadData.OpenTexture(MODEL_POTION+58+i, "Item\\partCharge1\\");
	}
	gLoadData.OpenTexture(MODEL_POTION+70, "Item\\partCharge2\\");
	gLoadData.OpenTexture(MODEL_POTION+71, "Item\\partCharge2\\");
	gLoadData.OpenTexture(MODEL_POTION+72, "Item\\partCharge2\\");
	gLoadData.OpenTexture(MODEL_POTION+73, "Item\\partCharge2\\");
	gLoadData.OpenTexture(MODEL_POTION+74, "Item\\partCharge2\\");
	gLoadData.OpenTexture(MODEL_POTION+75, "Item\\partCharge2\\");
	gLoadData.OpenTexture(MODEL_POTION+76, "Item\\partCharge2\\");
	gLoadData.OpenTexture(MODEL_POTION+77, "Item\\partCharge2\\");
	gLoadData.OpenTexture(MODEL_TYPE_CHARM_MIXWING+EWS_KNIGHT_1_CHARM, "Item\\partCharge6\\");     
	gLoadData.OpenTexture(MODEL_TYPE_CHARM_MIXWING+EWS_MAGICIAN_1_CHARM, "Item\\partCharge6\\");       
	gLoadData.OpenTexture(MODEL_TYPE_CHARM_MIXWING+EWS_ELF_1_CHARM, "Item\\partCharge6\\"); 
	gLoadData.OpenTexture(MODEL_TYPE_CHARM_MIXWING+EWS_SUMMONER_1_CHARM, "Item\\partCharge6\\");  
	gLoadData.OpenTexture(MODEL_TYPE_CHARM_MIXWING+EWS_DARKLORD_1_CHARM, "Item\\partCharge6\\");
		
	gLoadData.OpenTexture(MODEL_TYPE_CHARM_MIXWING+EWS_KNIGHT_2_CHARM, "Item\\partCharge6\\");   
	gLoadData.OpenTexture(MODEL_TYPE_CHARM_MIXWING+EWS_MAGICIAN_2_CHARM, "Item\\partCharge6\\");     
	gLoadData.OpenTexture(MODEL_TYPE_CHARM_MIXWING+EWS_ELF_2_CHARM, "Item\\partCharge6\\");    
	gLoadData.OpenTexture(MODEL_TYPE_CHARM_MIXWING+EWS_SUMMONER_2_CHARM, "Item\\partCharge6\\");   
	gLoadData.OpenTexture(MODEL_TYPE_CHARM_MIXWING+EWS_DARKKNIGHT_2_CHARM, "Item\\partCharge6\\");      

	gLoadData.OpenTexture(MODEL_HELPER+59, "Item\\partCharge2\\");

	gLoadData.OpenTexture(MODEL_HELPER+54, "Item\\partCharge2\\");
	gLoadData.OpenTexture(MODEL_HELPER+55, "Item\\partCharge2\\");
	gLoadData.OpenTexture(MODEL_HELPER+56, "Item\\partCharge2\\");
	gLoadData.OpenTexture(MODEL_HELPER+57, "Item\\partCharge2\\");
	gLoadData.OpenTexture(MODEL_HELPER+58, "Item\\partCharge2\\");
	gLoadData.OpenTexture(MODEL_POTION+78, "Item\\partCharge2\\");
	gLoadData.OpenTexture(MODEL_POTION+79, "Item\\partCharge2\\");
	gLoadData.OpenTexture(MODEL_POTION+80, "Item\\partCharge2\\");
	gLoadData.OpenTexture(MODEL_POTION+81, "Item\\partCharge2\\");
	gLoadData.OpenTexture(MODEL_POTION+82, "Item\\partCharge2\\");
	gLoadData.OpenTexture(MODEL_HELPER+60, "Item\\partCharge2\\");
	
	gLoadData.OpenTexture(MODEL_HELPER+61, "Item\\partCharge2\\");

	gLoadData.OpenTexture(MODEL_POTION+91, "Item\\partCharge3\\");
	
	gLoadData.OpenTexture(MODEL_POTION+92, "Item\\partCharge3\\");
	gLoadData.OpenTexture(MODEL_POTION+93, "Item\\partCharge3\\");
	gLoadData.OpenTexture(MODEL_POTION+95, "Item\\partCharge3\\");

	gLoadData.OpenTexture(MODEL_POTION+94, "Item\\partCharge2\\");

	gLoadData.OpenTexture(MODEL_HELPER+62, "Item\\partCharge4\\");
	gLoadData.OpenTexture(MODEL_HELPER+63, "Item\\partCharge4\\");

	gLoadData.OpenTexture(MODEL_POTION+97, "Item\\partCharge4\\");
	gLoadData.OpenTexture(MODEL_POTION+98, "Item\\partCharge4\\");

	gLoadData.OpenTexture(MODEL_POTION+96, "Item\\partCharge4\\");

	gLoadData.OpenTexture(MODEL_HELPER+64,"Item\\partCharge4\\"); 
	gLoadData.OpenTexture(MODEL_HELPER+65,"Item\\partCharge4\\"); 

	gLoadData.OpenTexture( MODEL_HELPER+67, "Item\\xmas\\" );
	gLoadData.OpenTexture( MODEL_HELPER+80, "Item\\" );

	gLoadData.OpenTexture(MODEL_HELPER+106,"Item\\partcharge7\\");
	gLoadData.OpenTexture(MODEL_HELPER+123,"Item\\");
	gLoadData.OpenTexture(MODEL_HELPER+66, "Item\\xmas\\");
	gLoadData.OpenTexture(MODEL_POTION+100, "Item\\");
	gLoadData.OpenTexture(MODEL_HELPER+69, "Item\\partCharge5\\");
	gLoadData.OpenTexture(MODEL_HELPER+70, "Item\\partCharge5\\");
	gLoadData.OpenTexture(MODEL_HELPER+81, "Item\\partCharge6\\");
	gLoadData.OpenTexture(MODEL_HELPER+82, "Item\\partCharge6\\");
	gLoadData.OpenTexture(MODEL_HELPER+93, "Item\\partCharge6\\");
	gLoadData.OpenTexture(MODEL_HELPER+94, "Item\\partCharge6\\");
	gLoadData.OpenTexture(MODEL_POTION+140, "Item\\");

	gLoadData.OpenTexture(MODEL_POTION+84, "Item\\cherryblossom\\");
	gLoadData.OpenTexture(MODEL_POTION+85, "Item\\cherryblossom\\");
	gLoadData.OpenTexture(MODEL_POTION+86, "Item\\cherryblossom\\");
	gLoadData.OpenTexture(MODEL_POTION+87, "Item\\cherryblossom\\");
	gLoadData.OpenTexture(MODEL_POTION+88, "Item\\cherryblossom\\");
	gLoadData.OpenTexture(MODEL_POTION+89, "Item\\cherryblossom\\");
	gLoadData.OpenTexture(MODEL_POTION+90, "Item\\cherryblossom\\");

	gLoadData.OpenTexture(MODEL_HELPER+49, "Item\\");
	gLoadData.OpenTexture(MODEL_HELPER+50, "Item\\");
	gLoadData.OpenTexture(MODEL_HELPER+51, "Item\\");

	gLoadData.OpenTexture(MODEL_HELPER+52, "Item\\");
	gLoadData.OpenTexture(MODEL_HELPER+53, "Item\\");

	gLoadData.OpenTexture(MODEL_POTION+55, "Item\\");
	gLoadData.OpenTexture(MODEL_POTION+56, "Item\\");
	gLoadData.OpenTexture(MODEL_POTION+57, "Item\\");
	
	gLoadData.OpenTexture(MODEL_POTION+64, "Item\\");
	gLoadData.OpenTexture(MODEL_POTION+65, "Item\\");
	gLoadData.OpenTexture(MODEL_POTION+66, "Item\\");
	gLoadData.OpenTexture(MODEL_POTION+67, "Item\\");
	gLoadData.OpenTexture(MODEL_POTION+68, "Item\\");

	gLoadData.OpenTexture(MODEL_POTION+63, "Item\\");
	gLoadData.OpenTexture(MODEL_POTION+52, "Item\\");
	g_ChangeRingMgr->LoadItemTexture();

	gLoadData.OpenTexture(MODEL_POTION+99, "Item\\XMas\\");

	gLoadData.OpenTexture(MODEL_POTION+145, "Item\\partCharge8\\");
	gLoadData.OpenTexture(MODEL_POTION+146, "Item\\partCharge8\\");
	gLoadData.OpenTexture(MODEL_POTION+147, "Item\\partCharge8\\");
	gLoadData.OpenTexture(MODEL_POTION+148, "Item\\partCharge8\\");
	gLoadData.OpenTexture(MODEL_POTION+149, "Item\\partCharge8\\");
	gLoadData.OpenTexture(MODEL_POTION+150, "Item\\partCharge8\\");

	gLoadData.OpenTexture(MODEL_HELPER+125, "Item\\partCharge8\\");
	gLoadData.OpenTexture(MODEL_HELPER+126, "Item\\partCharge8\\");
	gLoadData.OpenTexture(MODEL_HELPER+127, "Item\\partCharge8\\");

#ifdef LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM
	gLoadData.OpenTexture(MODEL_HELPER+128, "Item\\");
	gLoadData.OpenTexture(MODEL_HELPER+129, "Item\\");
	gLoadData.OpenTexture(MODEL_HELPER+134, "Item\\");
#endif //LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM
#ifdef LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM_PART_2
	gLoadData.OpenTexture(MODEL_HELPER+130, "Item\\");
//	gLoadData.OpenTexture(MODEL_HELPER+131, "Item\\");
	gLoadData.OpenTexture(MODEL_HELPER+132, "Item\\");
//	gLoadData.OpenTexture(MODEL_HELPER+133, "Item\\");
#endif //LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM_PART_2
//---------------------------------------------------------------------------------

	gLoadData.OpenTexture(MODEL_SWORD+26, "Item\\");
	gLoadData.OpenTexture(MODEL_SWORD+27, "Item\\");
	gLoadData.OpenTexture(MODEL_SWORD+28, "Item\\");
	gLoadData.OpenTexture(MODEL_MACE+16, "Item\\");
	gLoadData.OpenTexture(MODEL_MACE+17, "Item\\");
	gLoadData.OpenTexture(MODEL_BOW+23, "Item\\");
	gLoadData.OpenTexture(MODEL_STAFF+30, "Item\\");
	gLoadData.OpenTexture(MODEL_STAFF+31, "Item\\");
	gLoadData.OpenTexture(MODEL_STAFF+32, "Item\\");
	gLoadData.OpenTexture(MODEL_SHIELD+17, "Item\\");
	gLoadData.OpenTexture(MODEL_SHIELD+18, "Item\\");
	gLoadData.OpenTexture(MODEL_SHIELD+19, "Item\\");
	gLoadData.OpenTexture(MODEL_SHIELD+20, "Item\\");

	for (int i = 0; i < 6; ++i)
		gLoadData.OpenTexture(MODEL_WING+60+i, "Effect\\");

	for (int i = 0; i < 5; ++i)
		gLoadData.OpenTexture(MODEL_WING+70+i, "Item\\");

	for (int i = 0; i < 30; ++i)
	{
		gLoadData.OpenTexture(MODEL_WING+100+i, "Effect\\");
		gLoadData.OpenTexture(MODEL_WING+100+i, "Item\\");
	}

	gLoadData.OpenTexture(MODEL_SHIELD+21, "Item\\");

	gLoadData.OpenTexture(MODEL_BOW+24,   "Item\\");
	gLoadData.OpenTexture(MODEL_STAFF+33, "Item\\");
	gLoadData.OpenTexture(MODEL_STAFF+34, "Item\\");
	gLoadData.OpenTexture(MODEL_SWORD+29, "Item\\");
	gLoadData.OpenTexture(MODEL_MACE+18,  "Item\\");

	gLoadData.OpenTexture(MODEL_HELPER+71, "Item\\");
	gLoadData.OpenTexture(MODEL_HELPER+72, "Item\\");
	gLoadData.OpenTexture(MODEL_HELPER+73, "Item\\");
	gLoadData.OpenTexture(MODEL_HELPER+74, "Item\\");
	gLoadData.OpenTexture(MODEL_HELPER+75, "Item\\");

	gLoadData.OpenTexture(MODEL_HELPER+97, "Item\\Ingameshop\\");
	gLoadData.OpenTexture(MODEL_HELPER+98, "Item\\Ingameshop\\");
	gLoadData.OpenTexture(MODEL_POTION+91, "Item\\partCharge3\\");

#ifdef PBG_ADD_CHARACTERSLOT
	gLoadData.OpenTexture(MODEL_HELPER+99, "Item\\Ingameshop\\");
	gLoadData.OpenTexture(MODEL_SLOT_LOCK, "Item\\Ingameshop\\");
#endif //PBG_ADD_CHARACTERSLOT


	gLoadData.OpenTexture(MODEL_POTION+110, "Item\\");
	gLoadData.OpenTexture(MODEL_POTION+111, "Item\\");

	gLoadData.OpenTexture(MODEL_POTION+101, "Item\\");
	gLoadData.OpenTexture(MODEL_POTION+102, "Item\\");
	gLoadData.OpenTexture(MODEL_POTION+109, "Item\\");
	for(int c=0; c<6; c++)
	{
		gLoadData.OpenTexture(MODEL_POTION+103+c, "Item\\");
	}

	gLoadData.OpenTexture(MODEL_HELPER+107,"Item\\partcharge7\\");
	gLoadData.OpenTexture(MODEL_HELPER+104,"Item\\partcharge7\\");
	gLoadData.OpenTexture(MODEL_HELPER+105,"Item\\partcharge7\\");
	gLoadData.OpenTexture(MODEL_HELPER+103,"Item\\partcharge7\\");
	gLoadData.OpenTexture(MODEL_POTION+133,"Item\\partcharge7\\");

	gLoadData.OpenTexture(MODEL_HELPER+109,"Item\\InGameShop\\");
	gLoadData.OpenTexture(MODEL_HELPER+110,"Item\\InGameShop\\");
	gLoadData.OpenTexture(MODEL_HELPER+111,"Item\\InGameShop\\");
	gLoadData.OpenTexture(MODEL_HELPER+112,"Item\\InGameShop\\");
	gLoadData.OpenTexture(MODEL_HELPER+113,"Item\\InGameShop\\");
	gLoadData.OpenTexture(MODEL_HELPER+114,"Item\\InGameShop\\");
	gLoadData.OpenTexture(MODEL_HELPER+115,"Item\\InGameShop\\");

	gLoadData.OpenTexture(MODEL_POTION+120,"Item\\InGameShop\\");
	gLoadData.OpenTexture(MODEL_POTION+121,"Item\\InGameShop\\");
	gLoadData.OpenTexture(MODEL_POTION+122,"Item\\InGameShop\\");
	gLoadData.OpenTexture(MODEL_POTION+123,"Item\\InGameShop\\");
	gLoadData.OpenTexture(MODEL_POTION+124,"Item\\InGameShop\\");

	for(int k=0; k<6; k++)
	{
		gLoadData.OpenTexture(MODEL_POTION+134+k,"Item\\InGameShop\\");
	}
	LoadBitmap("Item\\InGameShop\\membership_item_blue.jpg"		,BITMAP_PACKAGEBOX_BLUE	,GL_LINEAR,GL_REPEAT);
	LoadBitmap("Item\\InGameShop\\membership_item_gold.jpg"		,BITMAP_PACKAGEBOX_GOLD	,GL_LINEAR,GL_REPEAT);
	LoadBitmap("Item\\InGameShop\\membership_item_green.jpg"	,BITMAP_PACKAGEBOX_GREEN,GL_LINEAR,GL_REPEAT);
	LoadBitmap("Item\\InGameShop\\membership_item_pouple.jpg"	,BITMAP_PACKAGEBOX_PUPLE,GL_LINEAR,GL_REPEAT);
	LoadBitmap("Item\\InGameShop\\membership_item_red.jpg"		,BITMAP_PACKAGEBOX_RED	,GL_LINEAR,GL_REPEAT);
	LoadBitmap("Item\\InGameShop\\membership_item_sky.jpg"		,BITMAP_PACKAGEBOX_SKY	,GL_LINEAR,GL_REPEAT);
	
	gLoadData.OpenTexture(MODEL_HELPER+116, "Item\\");

	
	gLoadData.OpenTexture(MODEL_WING+130, "Item\\");
	for(int j=0; j<4; j++)
	{
		gLoadData.OpenTexture(MODEL_WING+131+j, "Item\\Ingameshop\\");
	}

	gLoadData.OpenTexture(MODEL_HELPER+124, "Item\\partCharge6\\");
	gLoadData.OpenTexture(MODEL_POTION+112, "Item\\Ingameshop\\");
	gLoadData.OpenTexture(MODEL_POTION+113, "Item\\Ingameshop\\");

	for(int k=0; k<6; k++)
	{
		gLoadData.OpenTexture(MODEL_POTION+114+k,"Item\\InGameShop\\");
	}

	LoadBitmap("Item\\InGameShop\\mebership_3items_green.jpg"	,BITMAP_INGAMESHOP_PRIMIUM6	,GL_LINEAR, GL_REPEAT);
	for(int k=0; k<4; k++)
	{
		gLoadData.OpenTexture(MODEL_POTION+126+k,"Item\\InGameShop\\");
	}

	LoadBitmap("Item\\InGameShop\\mebership_3items_red.jpg"	,BITMAP_INGAMESHOP_COMMUTERTICKET4	,GL_LINEAR, GL_REPEAT);
	for(int k=0; k<3; k++)
	{
		gLoadData.OpenTexture(MODEL_POTION+130+k,"Item\\InGameShop\\");
	}

	LoadBitmap("Item\\InGameShop\\mebership_3items_yellow.jpg"	,BITMAP_INGAMESHOP_SIZECOMMUTERTICKET3	,GL_LINEAR, GL_REPEAT);

	gLoadData.OpenTexture(MODEL_HELPER+121,"Item\\InGameShop\\");

#ifdef PBG_ADD_GENSRANKING
	for(int _index=0; _index<4; ++_index)
		gLoadData.OpenTexture(MODEL_POTION+141+_index, "Item\\");
#endif //PBG_ADD_GENSRANKING

	gLoadData.OpenTexture(MODEL_15GRADE_ARMOR_OBJ_ARMLEFT,	"Item\\");		// 14, 15 efeito chifres brancos
	gLoadData.OpenTexture(MODEL_15GRADE_ARMOR_OBJ_ARMRIGHT,	"Item\\");		// 14, 15
	gLoadData.OpenTexture(MODEL_15GRADE_ARMOR_OBJ_BODYLEFT,	"Item\\");		// 14, 15
	gLoadData.OpenTexture(MODEL_15GRADE_ARMOR_OBJ_BODYRIGHT,	"Item\\");	// 14, 15
	gLoadData.OpenTexture(MODEL_15GRADE_ARMOR_OBJ_BOOTLEFT,	"Item\\");		// 14, 15
	gLoadData.OpenTexture(MODEL_15GRADE_ARMOR_OBJ_BOOTRIGHT,	"Item\\");	// 14, 15
	gLoadData.OpenTexture(MODEL_15GRADE_ARMOR_OBJ_HEAD,		"Item\\");		// 14, 15
	gLoadData.OpenTexture(MODEL_15GRADE_ARMOR_OBJ_PANTLEFT,	"Item\\");		// 14, 15
	gLoadData.OpenTexture(MODEL_15GRADE_ARMOR_OBJ_PANTRIGHT,	"Item\\");	// 14, 15
	LoadBitmap("Item\\rgb_mix.jpg"	,BITMAP_RGB_MIX	,GL_LINEAR, GL_REPEAT);

#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
	gLoadData.OpenTexture(MODEL_WING+49, "Item\\");
	gLoadData.OpenTexture(MODEL_WING+50, "Item\\");
	gLoadData.OpenTexture(MODEL_WING+135, "Item\\");
	g_CMonkSystem.LoadModelItemTexture();
	for(int _nRollIndex=0; _nRollIndex<7; ++_nRollIndex)
		gLoadData.OpenTexture(MODEL_ETC+30+_nRollIndex, "Item\\");
#endif //PBG_ADD_NEWCHAR_MONK_ITEM

#ifdef LEM_ADD_LUCKYITEM
	gLoadData.OpenTexture(MODEL_HELPER+135, "Item\\LuckyItem\\" );
	gLoadData.OpenTexture(MODEL_HELPER+136, "Item\\LuckyItem\\" );	
	gLoadData.OpenTexture(MODEL_HELPER+137, "Item\\LuckyItem\\" );
	gLoadData.OpenTexture(MODEL_HELPER+138, "Item\\LuckyItem\\" );
	gLoadData.OpenTexture(MODEL_HELPER+139, "Item\\LuckyItem\\" );
	gLoadData.OpenTexture(MODEL_HELPER+140, "Item\\LuckyItem\\" );
	gLoadData.OpenTexture(MODEL_HELPER+141, "Item\\LuckyItem\\" );
	gLoadData.OpenTexture(MODEL_HELPER+142, "Item\\LuckyItem\\" );
	gLoadData.OpenTexture(MODEL_HELPER+143, "Item\\LuckyItem\\" );
	gLoadData.OpenTexture(MODEL_HELPER+144, "Item\\LuckyItem\\" );

	char*	szLuckySetPath			 = { "Player\\LuckyItem\\" };
	char	szLuckySetPathName[50]	 = { "" };
	int		nIndex					 = 62;

	for( int i=0; i<11; i++ )
	{
		sprintf( szLuckySetPathName, "%s%d\\", szLuckySetPath, nIndex );
		if( nIndex != 71 )	gLoadData.OpenTexture(MODEL_HELM+nIndex,	szLuckySetPathName);
							gLoadData.OpenTexture(MODEL_ARMOR+nIndex,	szLuckySetPathName);
							gLoadData.OpenTexture(MODEL_PANTS+nIndex,	szLuckySetPathName);
		if( nIndex != 72 )	gLoadData.OpenTexture(MODEL_GLOVES+nIndex,szLuckySetPathName);
							gLoadData.OpenTexture(MODEL_BOOTS+nIndex,	szLuckySetPathName);
		nIndex++;
	}
	
	gLoadData.OpenTexture(MODEL_POTION+160, "Item\\LuckyItem\\" );
	gLoadData.OpenTexture(MODEL_POTION+161, "Item\\LuckyItem\\" );

	sprintf(szLuckySetPathName, "Player\\LuckyItem\\65\\InvenArmorMale40_luck.tga");
	LoadBitmap(szLuckySetPathName, BITMAP_INVEN_ARMOR+6);
	sprintf(szLuckySetPathName, "Player\\LuckyItem\\65\\InvenPantsMale40_luck.tga");
	LoadBitmap(szLuckySetPathName, BITMAP_INVEN_PANTS+6);
	sprintf(szLuckySetPathName, "Player\\LuckyItem\\70\\InvenArmorMale41_luck.tga");
	LoadBitmap(szLuckySetPathName, BITMAP_INVEN_ARMOR+7);
	sprintf(szLuckySetPathName, "Player\\LuckyItem\\70\\InvenPantsMale41_luck.tga");
	LoadBitmap(szLuckySetPathName, BITMAP_INVEN_PANTS+7);
#endif // LEM_ADD_LUCKYITEM

}

void DeleteNpcs()
{
	for(int i=MODEL_NPC_BEGIN;i<MODEL_NPC_END;i++)
		Models[i].Release();

	for(int i=SOUND_NPC;i<SOUND_NPC_END;i++)
		ReleaseBuffer(i);
}

void OpenNpc(int Type)
{
  	BMD *b = &Models[Type];
	if(b->NumActions > 0) return;

	switch(Type)
	{
	case MODEL_MERCHANT_FEMALE:
		gLoadData.AccessModel(MODEL_MERCHANT_FEMALE,"Data\\Npc\\","Female",1);

		for(int i=0;i<2;i++)
		{
			gLoadData.AccessModel(MODEL_MERCHANT_FEMALE_HEAD  +i,"Data\\Npc\\","FemaleHead"  ,i+1);
			gLoadData.AccessModel(MODEL_MERCHANT_FEMALE_UPPER +i,"Data\\Npc\\","FemaleUpper" ,i+1);
			gLoadData.AccessModel(MODEL_MERCHANT_FEMALE_LOWER +i,"Data\\Npc\\","FemaleLower" ,i+1);
			gLoadData.AccessModel(MODEL_MERCHANT_FEMALE_BOOTS +i,"Data\\Npc\\","FemaleBoots" ,i+1);
         	gLoadData.OpenTexture(MODEL_MERCHANT_FEMALE_HEAD  +i,"Npc\\");
         	gLoadData.OpenTexture(MODEL_MERCHANT_FEMALE_UPPER +i,"Npc\\");
         	gLoadData.OpenTexture(MODEL_MERCHANT_FEMALE_LOWER +i,"Npc\\");
         	gLoadData.OpenTexture(MODEL_MERCHANT_FEMALE_BOOTS +i,"Npc\\");
		}
		break;
	case MODEL_MERCHANT_MAN:
		gLoadData.AccessModel(MODEL_MERCHANT_MAN   ,"Data\\Npc\\","Man",1);

		for(int i=0;i<2;i++)
		{
			gLoadData.AccessModel(MODEL_MERCHANT_MAN_HEAD  +i,"Data\\Npc\\","ManHead"  ,i+1);
			gLoadData.AccessModel(MODEL_MERCHANT_MAN_UPPER +i,"Data\\Npc\\","ManUpper" ,i+1);
			gLoadData.AccessModel(MODEL_MERCHANT_MAN_GLOVES+i,"Data\\Npc\\","ManGloves",i+1);
			gLoadData.AccessModel(MODEL_MERCHANT_MAN_BOOTS +i,"Data\\Npc\\","ManBoots" ,i+1);
         	gLoadData.OpenTexture(MODEL_MERCHANT_MAN_HEAD  +i,"Npc\\");
         	gLoadData.OpenTexture(MODEL_MERCHANT_MAN_UPPER +i,"Npc\\");
         	gLoadData.OpenTexture(MODEL_MERCHANT_MAN_GLOVES+i,"Npc\\");
         	gLoadData.OpenTexture(MODEL_MERCHANT_MAN_BOOTS +i,"Npc\\");
		}
		break;
	case MODEL_MERCHANT_GIRL:
		gLoadData.AccessModel(MODEL_MERCHANT_GIRL  ,"Data\\Npc\\","Girl",1);

		for(int i=0;i<2;i++)
		{
			gLoadData.AccessModel(MODEL_MERCHANT_GIRL_HEAD  +i,"Data\\Npc\\","GirlHead"  ,i+1);
			gLoadData.AccessModel(MODEL_MERCHANT_GIRL_UPPER +i,"Data\\Npc\\","GirlUpper" ,i+1);
			gLoadData.AccessModel(MODEL_MERCHANT_GIRL_LOWER +i,"Data\\Npc\\","GirlLower" ,i+1);
         	gLoadData.OpenTexture(MODEL_MERCHANT_GIRL_HEAD  +i,"Npc\\");
         	gLoadData.OpenTexture(MODEL_MERCHANT_GIRL_UPPER +i,"Npc\\");
         	gLoadData.OpenTexture(MODEL_MERCHANT_GIRL_LOWER +i,"Npc\\");
		}
		break;
	case MODEL_SMITH:
		gLoadData.AccessModel(MODEL_SMITH          ,"Data\\Npc\\","Smith",1);
		LoadWaveFile(SOUND_NPC       	 ,"Data\\Sound\\nBlackSmith.wav",1);
		break;
	case MODEL_SCIENTIST:
		gLoadData.AccessModel(MODEL_SCIENTIST      ,"Data\\Npc\\","Wizard",1);
		break;
	case MODEL_SNOW_MERCHANT:
		gLoadData.AccessModel(MODEL_SNOW_MERCHANT  ,"Data\\Npc\\","SnowMerchant",1);
		break;
	case MODEL_SNOW_SMITH:
		gLoadData.AccessModel(MODEL_SNOW_SMITH     ,"Data\\Npc\\","SnowSmith",1);
		break;
	case MODEL_SNOW_WIZARD:
		gLoadData.AccessModel(MODEL_SNOW_WIZARD    ,"Data\\Npc\\","SnowWizard",1);
		break;
	case MODEL_ELF_WIZARD:
		gLoadData.AccessModel(MODEL_ELF_WIZARD     ,"Data\\Npc\\","ElfWizard",1);
		LoadWaveFile(SOUND_NPC        	 ,"Data\\Sound\\nHarp.wav",1);
		break;
	case MODEL_ELF_MERCHANT:
		gLoadData.AccessModel(MODEL_ELF_MERCHANT   ,"Data\\Npc\\","ElfMerchant",1);
		break;
	case MODEL_MASTER:
		gLoadData.AccessModel(MODEL_MASTER         ,"Data\\Npc\\","Master",1);
		break;
	case MODEL_STORAGE:
		gLoadData.AccessModel(MODEL_STORAGE        ,"Data\\Npc\\","Storage",1);
		break;
	case MODEL_TOURNAMENT:
		gLoadData.AccessModel(MODEL_TOURNAMENT     ,"Data\\Npc\\","Tournament",1);
		break;
	case MODEL_MIX_NPC:
		gLoadData.AccessModel(MODEL_MIX_NPC        ,"Data\\Npc\\","MixNpc",1);
		LoadWaveFile(SOUND_NPC+1       	 ,"Data\\Sound\\nMix.wav",1);
		break;
	case MODEL_REFINERY_NPC:
		gLoadData.AccessModel(MODEL_REFINERY_NPC        ,"Data\\Npc\\","os");
		gLoadData.OpenTexture(Type,"Npc\\");
		break;
	case MODEL_RECOVERY_NPC:
		gLoadData.AccessModel(MODEL_RECOVERY_NPC        ,"Data\\Npc\\","je");
		gLoadData.OpenTexture(Type,"Npc\\");
		break;	
	case MODEL_NPC_DEVILSQUARE:
		gLoadData.AccessModel(MODEL_NPC_DEVILSQUARE,"Data\\Npc\\","DevilNpc",1);
		break;

    case MODEL_NPC_SEVINA:
        gLoadData.AccessModel( MODEL_NPC_SEVINA, "Data\\Npc\\","Sevina",1);
        gLoadData.OpenTexture(Type,"Npc\\");
        break;
	case MODEL_NPC_ARCHANGEL:
        gLoadData.AccessModel( MODEL_NPC_ARCHANGEL, "Data\\Npc\\","BloodCastle",1);
        gLoadData.OpenTexture(Type,"Npc\\");
		break;
	case MODEL_NPC_ARCHANGEL_MESSENGER:
        gLoadData.AccessModel( MODEL_NPC_ARCHANGEL_MESSENGER, "Data\\Npc\\","BloodCastle",2);
        gLoadData.OpenTexture(Type,"Npc\\");
		break;

    //  데비아스, 로랜시아 추가 상점 NPC
	case MODEL_DEVIAS_TRADER:
		gLoadData.AccessModel( MODEL_DEVIAS_TRADER, "Data\\Npc\\", "DeviasTrader", 1);
		gLoadData.OpenTexture(Type, "Npc\\");
		break;

#ifdef _PVP_ATTACK_GUARD
	case MODEL_ANGEL:
		gLoadData.AccessModel ( MODEL_ANGEL, "Data\\Player\\", "Angel" );
		gLoadData.OpenTexture ( MODEL_ANGEL, "Npc\\" );
		break;
#endif	// _PVP_ATTACK_GUARD
    case MODEL_NPC_BREEDER:    //  조련사 NPC.
        gLoadData.AccessModel ( MODEL_NPC_BREEDER, "Data\\Npc\\", "Breeder" );
        gLoadData.OpenTexture ( MODEL_NPC_BREEDER, "Npc\\" );
        break;
#ifdef _PVP_MURDERER_HERO_ITEM
	case MODEL_HERO_SHOP:	// 영웅 상점
        gLoadData.AccessModel ( MODEL_HERO_SHOP, "Data\\Npc\\", "HeroNpc" );
        gLoadData.OpenTexture ( MODEL_HERO_SHOP, "Npc\\" );
		break;
#endif	// _PVP_MURDERER_HERO_ITEM

    case MODEL_NPC_CAPATULT_ATT:
        gLoadData.AccessModel ( MODEL_NPC_CAPATULT_ATT, "Data\\Npc\\", "Model_Npc_Catapult_Att" );
        gLoadData.OpenTexture ( MODEL_NPC_CAPATULT_ATT, "Npc\\" );
        break;

    case MODEL_NPC_CAPATULT_DEF:
        gLoadData.AccessModel ( MODEL_NPC_CAPATULT_DEF, "Data\\Npc\\", "Model_Npc_Catapult_Def" );
        gLoadData.OpenTexture ( MODEL_NPC_CAPATULT_DEF, "Npc\\" );
        break;

    case MODEL_NPC_SENATUS :
        gLoadData.AccessModel ( MODEL_NPC_SENATUS, "Data\\Npc\\", "NpcSenatus" );
        gLoadData.OpenTexture ( MODEL_NPC_SENATUS, "Npc\\" );
        break;

    case MODEL_NPC_GATE_SWITCH:
        gLoadData.AccessModel ( MODEL_NPC_GATE_SWITCH, "Data\\Npc\\", "NpcGateSwitch" );
        gLoadData.OpenTexture ( MODEL_NPC_GATE_SWITCH, "Npc\\" );
        break;

    case MODEL_NPC_CROWN :
        gLoadData.AccessModel ( MODEL_NPC_CROWN, "Data\\Npc\\", "NpcCrown" );
        gLoadData.OpenTexture ( MODEL_NPC_CROWN, "Npc\\" );
        break;

    case MODEL_NPC_CHECK_FLOOR :
        gLoadData.AccessModel ( MODEL_NPC_CHECK_FLOOR, "Data\\Npc\\", "NpcCheckFloor" );
        gLoadData.OpenTexture ( MODEL_NPC_CHECK_FLOOR, "Npc\\" );
        break;

    case MODEL_NPC_CLERK :
        gLoadData.AccessModel ( MODEL_NPC_CLERK, "Data\\Npc\\", "NpcClerk" );
        gLoadData.OpenTexture ( MODEL_NPC_CLERK, "Npc\\" );
        break;

    case MODEL_NPC_BARRIER:
        gLoadData.AccessModel ( MODEL_NPC_BARRIER, "Data\\Npc\\", "NpcBarrier" );
        gLoadData.OpenTexture ( MODEL_NPC_BARRIER, "Npc\\" );
        break;
	case MODEL_NPC_SERBIS:
		gLoadData.AccessModel ( MODEL_NPC_SERBIS, "Data\\Npc\\", "npc_mulyak" );
		gLoadData.OpenTexture ( MODEL_NPC_SERBIS, "Npc\\" );
		break;
	case MODEL_KALIMA_SHOP:
		gLoadData.AccessModel ( MODEL_KALIMA_SHOP, "Data\\Npc\\", "kalnpc" );
		gLoadData.OpenTexture ( MODEL_KALIMA_SHOP, "Npc\\" );
		break;
	case MODEL_BC_NPC1:
		gLoadData.AccessModel(MODEL_BC_NPC1, "Data\\Npc\\", "npcpharmercy1" );
		gLoadData.OpenTexture(MODEL_BC_NPC1, "Npc\\" );
		gLoadData.AccessModel(MODEL_BC_BOX, "Data\\Npc\\", "box");
		gLoadData.OpenTexture(MODEL_BC_BOX, "Npc\\" );
		break;
	case MODEL_BC_NPC2:
		gLoadData.AccessModel(MODEL_BC_NPC2, "Data\\Npc\\", "npcpharmercy2" );
		gLoadData.OpenTexture(MODEL_BC_NPC2, "Npc\\" );
		gLoadData.AccessModel(MODEL_BC_BOX, "Data\\Npc\\", "box");
		gLoadData.OpenTexture(MODEL_BC_BOX, "Npc\\" );
		break;
	case MODEL_CRYWOLF_STATUE:
		gLoadData.AccessModel ( MODEL_CRYWOLF_STATUE, "Data\\Object35\\", "Object82" );
		gLoadData.OpenTexture ( MODEL_CRYWOLF_STATUE, "Object35\\" );
		break;
	case MODEL_CRYWOLF_ALTAR1:
		gLoadData.AccessModel ( MODEL_CRYWOLF_ALTAR1, "Data\\Object35\\", "Object57" );
		gLoadData.OpenTexture ( MODEL_CRYWOLF_ALTAR1, "Object35\\" );
		break;
	case MODEL_CRYWOLF_ALTAR2:
		gLoadData.AccessModel ( MODEL_CRYWOLF_ALTAR2, "Data\\Object35\\", "Object57" );
		gLoadData.OpenTexture ( MODEL_CRYWOLF_ALTAR2, "Object35\\" );
		break;
	case MODEL_CRYWOLF_ALTAR3:
		gLoadData.AccessModel ( MODEL_CRYWOLF_ALTAR3, "Data\\Object35\\", "Object57" );
		gLoadData.OpenTexture ( MODEL_CRYWOLF_ALTAR3, "Object35\\" );
		break;
	case MODEL_CRYWOLF_ALTAR4:
		gLoadData.AccessModel ( MODEL_CRYWOLF_ALTAR4, "Data\\Object35\\", "Object57" );
		gLoadData.OpenTexture ( MODEL_CRYWOLF_ALTAR4, "Object35\\" );
		break;
	case MODEL_CRYWOLF_ALTAR5:
		gLoadData.AccessModel ( MODEL_CRYWOLF_ALTAR5, "Data\\Object35\\", "Object57" );
		gLoadData.OpenTexture ( MODEL_CRYWOLF_ALTAR5, "Object35\\" );
		break;
	case MODEL_KANTURU2ND_ENTER_NPC:
		{
			gLoadData.AccessModel(MODEL_KANTURU2ND_ENTER_NPC, "Data\\Npc\\", "to3gate");
			gLoadData.OpenTexture(MODEL_KANTURU2ND_ENTER_NPC, "Npc\\");
		}
		break;
	case MODEL_SMELTING_NPC:
		gLoadData.AccessModel ( MODEL_SMELTING_NPC, "Data\\Npc\\", "Elpis" );
		gLoadData.OpenTexture ( MODEL_SMELTING_NPC, "Npc\\" );
		break;
	case MODEL_NPC_DEVIN:
		gLoadData.AccessModel(MODEL_NPC_DEVIN, "Data\\Npc\\", "devin" );
		gLoadData.OpenTexture(MODEL_NPC_DEVIN, "Npc\\");
		break;
	case MODEL_NPC_QUARREL:
		gLoadData.AccessModel(MODEL_NPC_QUARREL, "Data\\Npc\\", "WereQuarrel" );
		gLoadData.OpenTexture(MODEL_NPC_QUARREL, "Monster\\");
		break;
	case MODEL_NPC_CASTEL_GATE:
		gLoadData.AccessModel(MODEL_NPC_CASTEL_GATE, "Data\\Npc\\", "cry2doorhead" );
		gLoadData.OpenTexture(MODEL_NPC_CASTEL_GATE, "Npc\\");
		break;
	case MODEL_CURSEDTEMPLE_ENTER_NPC:
		gLoadData.AccessModel ( MODEL_CURSEDTEMPLE_ENTER_NPC, "Data\\Npc\\", "mirazu" );
		gLoadData.OpenTexture ( MODEL_CURSEDTEMPLE_ENTER_NPC, "Npc\\" );
		break;
	case MODEL_CURSEDTEMPLE_ALLIED_NPC:
		gLoadData.AccessModel ( MODEL_CURSEDTEMPLE_ALLIED_NPC, "Data\\Npc\\", "allied" );
		gLoadData.OpenTexture ( MODEL_CURSEDTEMPLE_ALLIED_NPC, "Npc\\" );
		break;
	case MODEL_CURSEDTEMPLE_ILLUSION_NPC:
		gLoadData.AccessModel ( MODEL_CURSEDTEMPLE_ILLUSION_NPC, "Data\\Npc\\", "illusion" );
		gLoadData.OpenTexture ( MODEL_CURSEDTEMPLE_ILLUSION_NPC, "Npc\\" );
		break;		
	case MODEL_CURSEDTEMPLE_STATUE:
		gLoadData.AccessModel ( MODEL_CURSEDTEMPLE_STATUE, "Data\\Npc\\", "songsom" );
		gLoadData.OpenTexture ( MODEL_CURSEDTEMPLE_STATUE, "Npc\\" );
		break;
	case MODEL_CURSEDTEMPLE_ALLIED_BASKET:
		gLoadData.AccessModel ( MODEL_CURSEDTEMPLE_ALLIED_BASKET, "Data\\Npc\\", "songko" );
		gLoadData.OpenTexture ( MODEL_CURSEDTEMPLE_ALLIED_BASKET, "Npc\\" );
		break;
	case MODEL_CURSEDTEMPLE_ILLUSION__BASKET:
		gLoadData.AccessModel ( MODEL_CURSEDTEMPLE_ILLUSION__BASKET, "Data\\Npc\\", "songk2" );
		gLoadData.OpenTexture ( MODEL_CURSEDTEMPLE_ILLUSION__BASKET, "Npc\\" );
		break;
	case MODEL_WEDDING_NPC:
		gLoadData.AccessModel ( MODEL_WEDDING_NPC, "Data\\Npc\\", "Wedding" );
		gLoadData.OpenTexture( MODEL_WEDDING_NPC, "Npc\\");
		break;
	case MODEL_ELBELAND_SILVIA:
		gLoadData.AccessModel(MODEL_ELBELAND_SILVIA, "Data\\Npc\\", "silvia" );
		gLoadData.OpenTexture(MODEL_ELBELAND_SILVIA, "Npc\\");
		break;
	case MODEL_ELBELAND_RHEA:
		gLoadData.AccessModel(MODEL_ELBELAND_RHEA, "Data\\Npc\\", "rhea" );
		gLoadData.OpenTexture(MODEL_ELBELAND_RHEA, "Npc\\");
		break;
	case MODEL_ELBELAND_MARCE:
		gLoadData.AccessModel(MODEL_ELBELAND_MARCE, "Data\\Npc\\", "marce" );
		gLoadData.OpenTexture(MODEL_ELBELAND_MARCE, "Npc\\");
		break;
	case MODEL_NPC_CHERRYBLOSSOM:
		gLoadData.AccessModel(MODEL_NPC_CHERRYBLOSSOM, "Data\\Npc\\cherryblossom\\", "cherry_blossom" );
		gLoadData.OpenTexture(MODEL_NPC_CHERRYBLOSSOM, "Npc\\cherryblossom\\");
		break;
	case MODEL_NPC_CHERRYBLOSSOMTREE:
		gLoadData.AccessModel(MODEL_NPC_CHERRYBLOSSOMTREE, "Data\\Npc\\cherryblossom\\", "sakuratree" );
		gLoadData.OpenTexture(MODEL_NPC_CHERRYBLOSSOMTREE, "Npc\\cherryblossom\\");
		break;
	case MODEL_SEED_MASTER:
		gLoadData.AccessModel(MODEL_SEED_MASTER, "Data\\Npc\\", "goblinmaster" );
		gLoadData.OpenTexture(MODEL_SEED_MASTER, "Npc\\");
		break;
	case MODEL_SEED_INVESTIGATOR:
		gLoadData.AccessModel(MODEL_SEED_INVESTIGATOR, "Data\\Npc\\", "seedgoblin" );
		gLoadData.OpenTexture(MODEL_SEED_INVESTIGATOR, "Npc\\");
		break;
	case MODEL_LITTLESANTA:
	case MODEL_LITTLESANTA+1:
	case MODEL_LITTLESANTA+2:
	case MODEL_LITTLESANTA+3:
	case MODEL_LITTLESANTA+4:	
	case MODEL_LITTLESANTA+5:
	case MODEL_LITTLESANTA+6:
	case MODEL_LITTLESANTA+7:
		{
			gLoadData.AccessModel(MODEL_LITTLESANTA +(Type - MODEL_LITTLESANTA), "Data\\Npc\\", "xmassanta");

			int _index = 9;
			int _index_end = 14;

			Models[Type].Textures->FileName[_index_end] = NULL;
			for(int i=_index_end-1; i>_index; i--)
			{
				Models[Type].Textures->FileName[i] = Models[Type].Textures->FileName[i-1];
			}
			int _temp = (Type - MODEL_LITTLESANTA) +1;
			char _temp2[10] = {0,};
			itoa(_temp, _temp2, 10);
			Models[Type].Textures->FileName[_index] = _temp2[0];

			gLoadData.OpenTexture(MODEL_LITTLESANTA +(Type - MODEL_LITTLESANTA), "Npc\\");
		}
		break;
	case MODEL_XMAS2008_SNOWMAN:
		{
			gLoadData.AccessModel(MODEL_XMAS2008_SNOWMAN,			"Data\\Item\\xmas\\", "snowman");
			gLoadData.AccessModel(MODEL_XMAS2008_SNOWMAN_HEAD,	"Data\\Item\\xmas\\", "snowman_die_head_model");
			gLoadData.AccessModel(MODEL_XMAS2008_SNOWMAN_BODY,	"Data\\Item\\xmas\\", "snowman_die_body");
			gLoadData.OpenTexture(MODEL_XMAS2008_SNOWMAN,			"Item\\xmas\\" );
			gLoadData.OpenTexture(MODEL_XMAS2008_SNOWMAN_HEAD,	"Item\\xmas\\" );
			gLoadData.OpenTexture(MODEL_XMAS2008_SNOWMAN_BODY,	"Item\\xmas\\" );
			
			LoadWaveFile(SOUND_XMAS_SNOWMAN_WALK_1,		"Data\\Sound\\xmas\\SnowMan_Walk01.wav");
			LoadWaveFile(SOUND_XMAS_SNOWMAN_ATTACK_1,	"Data\\Sound\\xmas\\SnowMan_Attack01.wav");
			LoadWaveFile(SOUND_XMAS_SNOWMAN_ATTACK_2,	"Data\\Sound\\xmas\\SnowMan_Attack02.wav");
			LoadWaveFile(SOUND_XMAS_SNOWMAN_DAMAGE_1,	"Data\\Sound\\xmas\\SnowMan_Damage01.wav");
			LoadWaveFile(SOUND_XMAS_SNOWMAN_DEATH_1,	"Data\\Sound\\xmas\\SnowMan_Death01.wav");
		}
		break;
	case MODEL_XMAS2008_SNOWMAN_NPC:
		gLoadData.AccessModel(MODEL_XMAS2008_SNOWMAN_NPC, "Data\\Npc\\", "snowman" );
		gLoadData.OpenTexture(MODEL_XMAS2008_SNOWMAN_NPC, "Npc\\");
		break;
	case MODEL_XMAS2008_SANTA_NPC:
		gLoadData.AccessModel(MODEL_XMAS2008_SANTA_NPC, "Data\\Npc\\", "npcsanta" );
		gLoadData.OpenTexture(MODEL_XMAS2008_SANTA_NPC, "Npc\\");
		break;
	case MODEL_DUEL_NPC_TITUS:
		gLoadData.AccessModel(MODEL_DUEL_NPC_TITUS, "Data\\Npc\\", "titus" );
		gLoadData.OpenTexture(MODEL_DUEL_NPC_TITUS, "Npc\\");

		LoadWaveFile(SOUND_DUEL_NPC_IDLE_1, "Data\\Sound\\w64\\GatekeeperTitus.wav");
		break;
	case MODEL_GAMBLE_NPC_MOSS:
		gLoadData.AccessModel(MODEL_GAMBLE_NPC_MOSS, "Data\\Npc\\", "gambler_moss" );
		gLoadData.OpenTexture(MODEL_GAMBLE_NPC_MOSS, "Npc\\");
		break;
	case MODEL_DOPPELGANGER_NPC_LUGARD:
		gLoadData.AccessModel(MODEL_DOPPELGANGER_NPC_LUGARD, "Data\\Npc\\", "Lugard" );
		gLoadData.OpenTexture(MODEL_DOPPELGANGER_NPC_LUGARD, "Npc\\");
		LoadWaveFile(SOUND_DOPPELGANGER_LUGARD_BREATH, "Data\\Sound\\Doppelganger\\Lugard.wav");
		break;
	case MODEL_DOPPELGANGER_NPC_BOX:
		gLoadData.AccessModel(MODEL_DOPPELGANGER_NPC_BOX, "Data\\Npc\\", "DoppelgangerBox" );
		gLoadData.OpenTexture(MODEL_DOPPELGANGER_NPC_BOX, "Npc\\");
		LoadWaveFile(SOUND_DOPPELGANGER_JEWELBOX_OPEN, "Data\\Sound\\Doppelganger\\treasurebox_open.wav");
		break;
	case MODEL_DOPPELGANGER_NPC_GOLDENBOX:
		gLoadData.AccessModel(MODEL_DOPPELGANGER_NPC_GOLDENBOX, "Data\\Npc\\", "DoppelgangerBox" );
		gLoadData.OpenTexture(MODEL_DOPPELGANGER_NPC_GOLDENBOX, "Npc\\");
		LoadWaveFile(SOUND_DOPPELGANGER_JEWELBOX_OPEN, "Data\\Sound\\Doppelganger\\treasurebox_open.wav");
		break;
	case MODAL_GENS_NPC_DUPRIAN:
		gLoadData.AccessModel(MODAL_GENS_NPC_DUPRIAN, "Data\\Npc\\", "duprian" );
		gLoadData.OpenTexture(MODAL_GENS_NPC_DUPRIAN, "Npc\\");
		break;
	case MODAL_GENS_NPC_BARNERT:
		gLoadData.AccessModel(MODAL_GENS_NPC_BARNERT, "Data\\Npc\\", "barnert" );
		gLoadData.OpenTexture(MODAL_GENS_NPC_BARNERT, "Npc\\");
		break;
	case MODEL_UNITEDMARKETPLACE_CHRISTIN:
		gLoadData.AccessModel(MODEL_UNITEDMARKETPLACE_CHRISTIN, "Data\\Npc\\", "UnitedMarketPlace_christine" );
		gLoadData.OpenTexture(MODEL_UNITEDMARKETPLACE_CHRISTIN, "Npc\\");
		break;
	case MODEL_UNITEDMARKETPLACE_RAUL:
		gLoadData.AccessModel(MODEL_UNITEDMARKETPLACE_RAUL, "Data\\Npc\\", "UnitedMarkedPlace_raul" );
		gLoadData.OpenTexture(MODEL_UNITEDMARKETPLACE_RAUL, "Npc\\");
		break;
	case MODEL_UNITEDMARKETPLACE_JULIA:
		gLoadData.AccessModel(MODEL_UNITEDMARKETPLACE_JULIA, "Data\\Npc\\", "UnitedMarkedPlace_julia" );
		gLoadData.OpenTexture(MODEL_UNITEDMARKETPLACE_JULIA, "Npc\\");
		break;
#ifdef ASG_ADD_KARUTAN_NPC
	case MODEL_KARUTAN_NPC_REINA:	// 로랜시장 NPC 잡화상인 크리스틴과 동일.
		gLoadData.AccessModel(MODEL_KARUTAN_NPC_REINA, "Data\\Npc\\", "UnitedMarketPlace_christine");
		gLoadData.OpenTexture(MODEL_KARUTAN_NPC_REINA, "Npc\\");
		break;
	case MODEL_KARUTAN_NPC_VOLVO:
		gLoadData.AccessModel(MODEL_KARUTAN_NPC_VOLVO, "Data\\Npc\\", "volvo");
		gLoadData.OpenTexture(MODEL_KARUTAN_NPC_VOLVO, "Npc\\");
		break;
#endif	// ASG_ADD_KARUTAN_NPC
#ifdef LEM_ADD_LUCKYITEM
	case MODEL_LUCKYITEM_NPC:
		//gLoadData.AccessModel(MODEL_LUCKYITEM_NPC, "Data\\Npc\\", "volvo");
		gLoadData.AccessModel(MODEL_LUCKYITEM_NPC, "Data\\Npc\\LuckyItem\\", "npc_burial");
		gLoadData.OpenTexture(MODEL_LUCKYITEM_NPC, "Npc\\LuckyItem\\");
		break;
#endif // LEM_ADD_LUCKYITEM
	}

	for(int i=0;i<b->NumActions;i++)
		b->Actions[i].PlaySpeed = 0.25f;
    //SetTexture(BITMAP_NPC);

    if(b->NumMeshs > 0)
    	gLoadData.OpenTexture(Type,"Npc\\");

	switch(Type)
	{
	case MODEL_XMAS2008_SNOWMAN:
		Models[Type].Actions[MONSTER01_WALK].PlaySpeed = 0.9f;
		break;
	case MODEL_PANDA:
		Models[Type].Actions[MONSTER01_WALK].PlaySpeed = 0.9f;
		break;
	case MODEL_DOPPELGANGER_NPC_BOX:
		Models[Type].Actions[MONSTER01_DIE].PlaySpeed = 0.1f;
		break;
	case MODEL_DOPPELGANGER_NPC_GOLDENBOX:
		Models[Type].Actions[MONSTER01_DIE].PlaySpeed = 0.1f;
		break;
	case MODAL_GENS_NPC_DUPRIAN:
		Models[Type].Actions[MONSTER01_STOP1].PlaySpeed = 0.6f;
		Models[Type].Actions[MONSTER01_STOP2].PlaySpeed = 0.6f;
		Models[Type].Actions[MONSTER01_WALK].PlaySpeed = 1.4f;
		break;
	case MODAL_GENS_NPC_BARNERT:
		Models[Type].Actions[MONSTER01_STOP1].PlaySpeed = 0.3f;
		Models[Type].Actions[MONSTER01_STOP2].PlaySpeed = 1.2f;
		Models[Type].Actions[MONSTER01_WALK].PlaySpeed = 0.3f;
	case MODEL_UNITEDMARKETPLACE_RAUL:
		Models[Type].Actions[MONSTER01_STOP1].PlaySpeed = 0.5f;
		Models[Type].Actions[MONSTER01_STOP2].PlaySpeed = 0.5f;
		Models[Type].Actions[MONSTER01_WALK].PlaySpeed = 0.5f;
		break;
	case MODEL_UNITEDMARKETPLACE_JULIA:
		Models[Type].Actions[MONSTER01_STOP1].PlaySpeed = 0.5f;
		Models[Type].Actions[MONSTER01_STOP2].PlaySpeed = 0.5f;
		Models[Type].Actions[MONSTER01_WALK].PlaySpeed = 0.5f;
		break;
	case MODEL_UNITEDMARKETPLACE_CHRISTIN:
#ifdef ASG_ADD_KARUTAN_NPC
	case MODEL_KARUTAN_NPC_REINA:
#endif	// ASG_ADD_KARUTAN_NPC
		Models[Type].Actions[MONSTER01_STOP1].PlaySpeed = 0.5f;
		Models[Type].Actions[MONSTER01_STOP2].PlaySpeed = 0.6f;
		Models[Type].Actions[MONSTER01_WALK].PlaySpeed = 0.5f;
		break;
#ifdef ASG_ADD_TIME_LIMIT_QUEST_NPC
	case MODEL_TIME_LIMIT_QUEST_NPC_TERSIA:
		Models[Type].Actions[MONSTER01_STOP1].PlaySpeed = 0.35f;
		Models[Type].Actions[MONSTER01_STOP2].PlaySpeed = 0.3f;
		break;
#endif	// ASG_ADD_TIME_LIMIT_QUEST_NPC
#ifdef ASG_ADD_KARUTAN_NPC
	case MODEL_KARUTAN_NPC_VOLVO:
		Models[Type].Actions[MONSTER01_STOP1].PlaySpeed = 0.2f;
		Models[Type].Actions[MONSTER01_STOP2].PlaySpeed = 0.25f;
		break;
#endif	// ASG_ADD_KARUTAN_NPC
	}


//#endif
}

void SetMonsterSound(int Type,int s1,int s2,int s3,int s4,int s5, int s6=-1, int s7=-1, int s8=-1, int s9=-1, int s10=-1 )
{
	Models[Type].Sounds[0] = s1;
	Models[Type].Sounds[1] = s2;
	Models[Type].Sounds[2] = s3;
	Models[Type].Sounds[3] = s4;
	Models[Type].Sounds[4] = s5;
	Models[Type].Sounds[5] = s6;
	Models[Type].Sounds[6] = s7;
	Models[Type].Sounds[7] = s8;
	Models[Type].Sounds[8] = s9;
	Models[Type].Sounds[9] = s10;
}

void DeleteMonsters()
{
	for(int i=MODEL_MONSTER01;i<MODEL_MONSTER_END;i++)
		Models[i].Release();

	for(int i=SOUND_MONSTER;i<SOUND_MONSTER_END;i++)
		ReleaseBuffer(i);

	for(int i=SOUND_ELBELAND_RABBITSTRANGE_ATTACK01;i<=SOUND_ELBELAND_ENTERATLANCE01;i++)
		ReleaseBuffer(i);
}

void OpenMonsterModel(int Type)
{
	g_ErrorReport.Write( "OpenMonsterModel(%d)\r\n", Type);

	int Index = MODEL_MONSTER01+Type;

  	BMD *b = &Models[Index];
	if(b->NumActions > 0 || b->NumMeshs > 0) return;

	gLoadData.AccessModel(Index,"Data\\Monster\\","Monster",Type+1);

	if(b->NumMeshs == 0) return;

    if ( gMapManager.InChaosCastle()==true && Type>=70 && Type<=72 )
    {
    	gLoadData.OpenTexture(Index,"Npc\\");
    }
    else if ( gMapManager.InBattleCastle()==true && Type==74 )
    {
        gLoadData.OpenTexture ( Index, "Object31\\" );
    }
    else
    {
		gLoadData.OpenTexture(Index,"Monster\\");
    }

	switch(Type)
	{
	case 147:
		{
			gLoadData.AccessModel(MODEL_ICE_GIANT_PART1, "Data\\Monster\\", "icegiantpart_1");
			gLoadData.OpenTexture(MODEL_ICE_GIANT_PART1, "Monster\\");
			gLoadData.AccessModel(MODEL_ICE_GIANT_PART2, "Data\\Monster\\", "icegiantpart_2");
			gLoadData.OpenTexture(MODEL_ICE_GIANT_PART2, "Monster\\");
			gLoadData.AccessModel(MODEL_ICE_GIANT_PART3, "Data\\Monster\\", "icegiantpart_3");
			gLoadData.OpenTexture(MODEL_ICE_GIANT_PART3, "Monster\\");
			gLoadData.AccessModel(MODEL_ICE_GIANT_PART4, "Data\\Monster\\", "icegiantpart_4");
			gLoadData.OpenTexture(MODEL_ICE_GIANT_PART4, "Monster\\");
			gLoadData.AccessModel(MODEL_ICE_GIANT_PART5, "Data\\Monster\\", "icegiantpart_5");
			gLoadData.OpenTexture(MODEL_ICE_GIANT_PART5, "Monster\\");
			gLoadData.AccessModel(MODEL_ICE_GIANT_PART6, "Data\\Monster\\", "icegiantpart_6");
			gLoadData.OpenTexture(MODEL_ICE_GIANT_PART6, "Monster\\");
		}
		break;
	}

	b->Actions[MONSTER01_STOP1  ].PlaySpeed = 0.25f;
	b->Actions[MONSTER01_STOP2  ].PlaySpeed = 0.2f;
	b->Actions[MONSTER01_WALK   ].PlaySpeed = 0.34f;
	b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.33f;
	b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.33f;
	b->Actions[MONSTER01_SHOCK  ].PlaySpeed = 0.5f;
	b->Actions[MONSTER01_DIE    ].PlaySpeed = 0.55f;
	b->Actions[MONSTER01_DIE    ].Loop      = true;

	for(int j=MONSTER01_STOP1;j<MONSTER01_DIE;j++)
	{
		if(Type==3)
			b->Actions[j].PlaySpeed *= 1.2f;
		if(Type==5 || Type==25)
			b->Actions[j].PlaySpeed *= 0.7f;
		if(Type==37 || Type==42)
			b->Actions[j].PlaySpeed *= 0.4f;
	}

	switch(Type)
	{
	case 2 :b->Actions[MONSTER01_WALK].PlaySpeed = 0.7f;break;
	case 6 :b->Actions[MONSTER01_WALK].PlaySpeed = 0.6f;break;
	case 8 :b->Actions[MONSTER01_WALK].PlaySpeed = 0.7f;break;
	case 9 :b->Actions[MONSTER01_WALK].PlaySpeed = 1.2f;break;
	case 10:b->Actions[MONSTER01_WALK].PlaySpeed = 0.28f;break;
	case 12:b->Actions[MONSTER01_WALK].PlaySpeed = 0.3f;break;
	case 13:b->Actions[MONSTER01_WALK].PlaySpeed = 0.28f;break;
	case 17:b->Actions[MONSTER01_WALK].PlaySpeed = 0.5f;break;
	case 19:b->Actions[MONSTER01_WALK].PlaySpeed = 0.6f;break;
	case 20:b->Actions[MONSTER01_WALK].PlaySpeed = 0.4f;break;
	case 21:b->Actions[MONSTER01_WALK].PlaySpeed = 0.5f;break;
	case 28:b->Actions[MONSTER01_WALK].PlaySpeed = 0.3f;break;
	case 39:b->Actions[MONSTER01_WALK].PlaySpeed = 0.22f;break;
	case 41:b->Actions[MONSTER01_WALK].PlaySpeed = 0.18f;break;
	case 42:
		b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.35f;
		b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.35f;
		break;
	case 44:
		b->Actions[MONSTER01_DIE].PlaySpeed = 0.3f;
		break;
    case 63:
		b->Actions[MONSTER01_DIE].PlaySpeed = 0.1f;
        break;
    case 64:
		b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.3f;
		b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.25f;
		b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.15f;
		b->Actions[MONSTER01_DIE].PlaySpeed = 0.25f;
        break;
    case 66:
		b->Actions[MONSTER01_DIE].PlaySpeed = 0.1f;
        break;
    case 67:
		b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.2f;
		b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.3f;
        break;
    case 69:
		b->Actions[MONSTER01_DIE].PlaySpeed = 0.1f;
        break;
    case 70:	// 
    case 71:	// 
		b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.5f;
		b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.5f;
		b->Actions[MONSTER01_WALK].PlaySpeed = 0.3f;
        break;
    case 72:
//    case 73:
		b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.4f;
		b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.4f;
		b->Actions[MONSTER01_WALK].PlaySpeed = 0.3f;
        break;
	case 89:
		b->Actions[MONSTER01_APEAR  ].PlaySpeed = 0.33f;
		b->Actions[MONSTER01_ATTACK3].PlaySpeed = 0.33f;
		b->Actions[MONSTER01_ATTACK4].PlaySpeed = 0.33f;
		break;
	case 94:
		b->Actions[MONSTER01_WALK].PlaySpeed = 0.20f;
		break;
	case 92:
		b->Actions[MONSTER01_APEAR  ].PlaySpeed = 0.33f;
		b->Actions[MONSTER01_ATTACK3].PlaySpeed = 0.33f;
		break;
	case 99:
		b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.37f;
		b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.37f;
		b->Actions[MONSTER01_DIE].PlaySpeed = 0.15f;
		break;
	case 100:
		b->Actions[MONSTER01_DIE].PlaySpeed = 0.2f;
		break;
	case 101:
		b->Actions[MONSTER01_DIE].PlaySpeed = 0.2f;
	case 102:
		b->Actions[MONSTER01_STOP1  ].PlaySpeed = 0.15f;
		b->Actions[MONSTER01_STOP2  ].PlaySpeed = 0.15f;
		b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.23f;
		b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.23f;
		b->Actions[MONSTER01_WALK].PlaySpeed = 0.3f;
		b->Actions[MONSTER01_DIE].PlaySpeed = 0.15f;
		break;
	case 104:
		b->Actions[MONSTER01_DIE].PlaySpeed = 0.2f;
		break;
	case 105:
		b->Actions[MONSTER01_WALK].PlaySpeed = 0.15f;
		break;
	case 106:
		b->Actions[MONSTER01_STOP1  ].PlaySpeed = 0.25f;
		b->Actions[MONSTER01_STOP2  ].PlaySpeed = 0.25f;
		b->Actions[MONSTER01_WALK   ].PlaySpeed = 0.23f;
		b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.28f;
		b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.28f;
		b->Actions[MONSTER01_DIE].PlaySpeed = 0.19f;
		break;
	case 107:
		b->Actions[MONSTER01_STOP1  ].PlaySpeed = 0.25f;
		b->Actions[MONSTER01_STOP2  ].PlaySpeed = 0.25f;		
		b->Actions[MONSTER01_WALK   ].PlaySpeed = 0.27f;
		b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.30f;
		b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.30f;
		b->Actions[MONSTER01_DIE].PlaySpeed = 0.30f;
		break;
	case 108:
		b->Actions[MONSTER01_STOP1  ].PlaySpeed = 0.25f;
		b->Actions[MONSTER01_STOP2  ].PlaySpeed = 0.25f;		
		b->Actions[MONSTER01_WALK   ].PlaySpeed = 0.27f;
		b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.25f;
		b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.25f;
		b->Actions[MONSTER01_DIE].PlaySpeed = 0.25f;
		break;
	case 109:
		b->Actions[MONSTER01_STOP1  ].PlaySpeed = 0.25f;
		b->Actions[MONSTER01_STOP2  ].PlaySpeed = 0.25f;
		b->Actions[MONSTER01_WALK   ].PlaySpeed = 0.27f;
		b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.3f;
		b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.3f;
		b->Actions[MONSTER01_DIE].PlaySpeed = 0.3f;
		break;
	case 110:
		b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.23f;
		b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.23f;
		b->Actions[MONSTER01_DIE].PlaySpeed = 0.2f;
		break;
	case 111:
		b->Actions[MONSTER01_STOP1  ].PlaySpeed = 0.25f;
		b->Actions[MONSTER01_STOP2  ].PlaySpeed = 0.25f;
		b->Actions[MONSTER01_WALK   ].PlaySpeed = 0.27f;
		b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.27f;
		b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.27f;
		b->Actions[MONSTER01_DIE].PlaySpeed = 0.25f;
		break;
	case 112:
		b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.26f;
		b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.26f;
		b->Actions[MONSTER01_DIE].PlaySpeed = 0.21f;
		break;
	case 113:
		b->Actions[MONSTER01_STOP1  ].PlaySpeed = 0.25f;
		b->Actions[MONSTER01_STOP2  ].PlaySpeed = 0.25f;
		b->Actions[MONSTER01_WALK   ].PlaySpeed = 0.25f;
		b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.3f;
		b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.3f;
		b->Actions[MONSTER01_SHOCK  ].PlaySpeed = 0.5f;
		b->Actions[MONSTER01_DIE].PlaySpeed = 0.3f;
		break;
	case 114:
		b->Actions[MONSTER01_STOP1  ].PlaySpeed = 0.25f;
		b->Actions[MONSTER01_STOP2  ].PlaySpeed = 0.25f;
		b->Actions[MONSTER01_WALK   ].PlaySpeed = 0.34f;
		b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.33f;
		b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.33f;
		b->Actions[MONSTER01_DIE].PlaySpeed = 0.23f;
		break;
	case 115:
		b->Actions[MONSTER01_STOP1  ].PlaySpeed = 0.25f;
		b->Actions[MONSTER01_STOP2  ].PlaySpeed = 0.25f;
		b->Actions[MONSTER01_WALK   ].PlaySpeed = 0.34f;
		b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.23f;
		b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.23f;
		b->Actions[MONSTER01_DIE].PlaySpeed = 0.23f;
		break;
	case 116:
		b->Actions[MONSTER01_STOP1  ].PlaySpeed = 0.25f;
		b->Actions[MONSTER01_STOP2  ].PlaySpeed = 0.25f;
		b->Actions[MONSTER01_WALK   ].PlaySpeed = 0.34f;
		b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.25f;
		b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.25f;
		b->Actions[MONSTER01_DIE].PlaySpeed = 0.25f;
		break;
	case 121:
		b->Actions[MONSTER01_STOP1  ].PlaySpeed = 0.22f;
		b->Actions[MONSTER01_STOP2  ].PlaySpeed = 0.22f;
		b->Actions[MONSTER01_WALK   ].PlaySpeed = 0.25f;
		b->Actions[MONSTER01_DIE    ].PlaySpeed = 0.12f;
		b->Actions[MONSTER01_APEAR  ].PlaySpeed = 0.22f;
		b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.25f;
		b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.25f;
		b->Actions[MONSTER01_ATTACK3].PlaySpeed = 0.25f;
		b->Actions[MONSTER01_ATTACK4].PlaySpeed = 0.25f;
		break;
	case 118:
		b->Actions[MONSTER01_APEAR  ].PlaySpeed = 0.12f;
		b->Actions[MONSTER01_DIE    ].PlaySpeed = 0.12f;
	case 119:
		b->Actions[MONSTER01_APEAR  ].PlaySpeed = 0.12f;
		b->Actions[MONSTER01_DIE    ].PlaySpeed = 0.12f;
	case 120:
		break;
	case 122:
		b->Actions[MONSTER01_SHOCK  ].PlaySpeed = 0.3f;
		b->Actions[MONSTER01_DIE	].PlaySpeed = 0.22f;
		break;
	case 127:
		b->Actions[MONSTER01_STOP1].PlaySpeed = 0.40f;
		b->Actions[MONSTER01_STOP2].PlaySpeed = 0.40f;		
		b->Actions[MONSTER01_WALK ].PlaySpeed = 0.40f;
		b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.40f;
		b->Actions[MONSTER01_DIE].PlaySpeed = 0.50f;
		break;
	case 154:
		b->Actions[MONSTER01_STOP1].PlaySpeed = 0.30f;
		b->Actions[MONSTER01_STOP2].PlaySpeed = 0.30f;		
		b->Actions[MONSTER01_WALK ].PlaySpeed = 0.60f;
		b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.50f;
		b->Actions[MONSTER01_DIE].PlaySpeed = 0.50f;
		break;
	case 157:
		{
			b->Actions[MONSTER01_STOP1].PlaySpeed = 0.28f;
			b->Actions[MONSTER01_WALK].PlaySpeed = 0.3f;
			b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.17f;
			b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.25f;
			b->Actions[MONSTER01_DIE].PlaySpeed = 0.2f;
		}
		break;
	case 158:
		{
			b->Actions[MONSTER01_STOP1].PlaySpeed = 0.25f;
			b->Actions[MONSTER01_WALK].PlaySpeed = 0.3f;
			b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.28f;
			b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.2f;
			b->Actions[MONSTER01_DIE].PlaySpeed = 0.18f;
		}
		break;
	case 159:
		{
			b->Actions[MONSTER01_STOP1].PlaySpeed = 0.2f;
			b->Actions[MONSTER01_WALK].PlaySpeed = 0.4f;
			b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.17f;
			b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.25f;
			b->Actions[MONSTER01_DIE].PlaySpeed = 0.2f;
		}
		break;
	case 160:
		{
			b->Actions[MONSTER01_STOP1].PlaySpeed = 0.28f;
			b->Actions[MONSTER01_WALK].PlaySpeed = 0.5f;
			b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.17f;
			b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.25f;
			b->Actions[MONSTER01_DIE].PlaySpeed = 0.4f;
		}
		break;
	case 161:
		{
			b->Actions[MONSTER01_STOP1].PlaySpeed = 0.28f;
			b->Actions[MONSTER01_WALK].PlaySpeed = 0.5f;
			b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.17f;
			b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.25f;
			b->Actions[MONSTER01_DIE].PlaySpeed = 0.4f;
		}
		break;
	case 162:
		{
			b->Actions[MONSTER01_STOP1].PlaySpeed = 0.28f;
			b->Actions[MONSTER01_WALK].PlaySpeed = 0.3f;
			b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.25f;
			b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.25f;
			b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.25f;
			b->Actions[MONSTER01_DIE].PlaySpeed = 0.25f;
		}
		break;
	case 163:
		{
			b->Actions[MONSTER01_STOP1].PlaySpeed = 0.28f;
			b->Actions[MONSTER01_WALK].PlaySpeed = 0.3f;
			b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.25f;
			b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.25f;
			b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.25f;
			b->Actions[MONSTER01_DIE].PlaySpeed = 0.25f;
		}
		break;
	case 128:
		b->Actions[MONSTER01_STOP1].PlaySpeed = 0.25f;
		b->Actions[MONSTER01_STOP2].PlaySpeed = 0.28f;
		b->Actions[MONSTER01_WALK ].PlaySpeed = 0.25f;
		b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.25f;
		b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.25f;
		b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.25f;
		b->Actions[MONSTER01_DIE  ].PlaySpeed = 0.25f;
		break;
	case 129:
		b->Actions[MONSTER01_STOP1].PlaySpeed = 0.20f;
		b->Actions[MONSTER01_STOP2].PlaySpeed = 0.20f;
		b->Actions[MONSTER01_WALK ].PlaySpeed = 0.25f;
		b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.30f;
		b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.30f;
		b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.25f;
		b->Actions[MONSTER01_DIE  ].PlaySpeed = 0.23f;
		break;
	case 130:
		b->Actions[MONSTER01_STOP1].PlaySpeed = 0.25f;
		b->Actions[MONSTER01_STOP2].PlaySpeed = 0.25f;
		b->Actions[MONSTER01_WALK ].PlaySpeed = 0.25f;
		b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.50f;
		b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.50f;
		b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.25f;
		b->Actions[MONSTER01_DIE  ].PlaySpeed = 0.30f;
		break;
	case 131:
		b->Actions[MONSTER01_STOP1].PlaySpeed = 0.45f;
		b->Actions[MONSTER01_STOP2].PlaySpeed = 0.45f;
		b->Actions[MONSTER01_WALK ].PlaySpeed = 0.45f;
		b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.45f;
		b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.45f;
		b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.45f;
		b->Actions[MONSTER01_DIE  ].PlaySpeed = 0.45f;
		break;
	case 132:
		b->Actions[MONSTER01_STOP1].PlaySpeed = 0.20f;
		b->Actions[MONSTER01_STOP2].PlaySpeed = 0.20f;
		b->Actions[MONSTER01_WALK ].PlaySpeed = 0.25f;
		b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.35;
		b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.35f;
		b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.25f;
		b->Actions[MONSTER01_DIE  ].PlaySpeed = 0.30f;
		break;
	case 133:
		b->Actions[MONSTER01_STOP1].PlaySpeed = 0.25f;
		b->Actions[MONSTER01_STOP2].PlaySpeed = 0.25f;
		b->Actions[MONSTER01_WALK ].PlaySpeed = 0.20f;
		b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.35;
		b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.35f;
		b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.25f;
		b->Actions[MONSTER01_DIE  ].PlaySpeed = 0.20f;
		break;
	case 134:
		b->Actions[MONSTER01_STOP1].PlaySpeed = 0.25f;
		b->Actions[MONSTER01_STOP2].PlaySpeed = 0.25f;
		b->Actions[MONSTER01_WALK ].PlaySpeed = 0.25f;
		b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.33f;
		b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.33f;
		b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.25f;
		b->Actions[MONSTER01_DIE  ].PlaySpeed = 0.22f;
		break;
	case 135:
		b->Actions[MONSTER01_STOP1].PlaySpeed = 0.25f;
		b->Actions[MONSTER01_STOP2].PlaySpeed = 0.25f;
		b->Actions[MONSTER01_WALK ].PlaySpeed = 0.25f;
		b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.30f;
		b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.30f;
		b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.25f;
		b->Actions[MONSTER01_DIE  ].PlaySpeed = 0.28f;
		break;
	case 136:
		b->Actions[MONSTER01_DIE  ].PlaySpeed = 0.25f;
		break;
	case 137:
		b->Actions[MONSTER01_DIE  ].PlaySpeed = 0.25f;
		break;
	case 138:
		b->Actions[MONSTER01_DIE  ].PlaySpeed = 0.25f;
		break;
	case 139:
		b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.4f;
		b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.4f;
		b->Actions[MONSTER01_DIE  ].PlaySpeed = 0.25f;
		break;
	case 140:
		b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.45f;
		b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.45f;
		b->Actions[MONSTER01_DIE  ].PlaySpeed = 0.25f;
		break;
	case 141:
		b->Actions[MONSTER01_DIE  ].PlaySpeed = 0.25f;
		break;
	case 142:
		b->Actions[MONSTER01_WALK ].PlaySpeed = 0.25f;
		b->Actions[MONSTER01_DIE  ].PlaySpeed = 0.20f;
		break;
	case 143:
		b->Actions[MONSTER01_WALK ].PlaySpeed = 0.25f;
		b->Actions[MONSTER01_DIE  ].PlaySpeed = 0.20f;
		break;
	case 144:
		b->Actions[MONSTER01_WALK ].PlaySpeed = 0.25f;
		b->Actions[MONSTER01_DIE  ].PlaySpeed = 0.20f;
		break;
	case 145:
		b->Actions[MONSTER01_WALK   ].PlaySpeed = 0.6f;
		b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.4f;
		b->Actions[MONSTER01_DIE    ].PlaySpeed = 0.35f;
		break;
	case 146:
		break;
	case 147:
		b->Actions[MONSTER01_DIE].PlaySpeed = 0.13f;
		break;
	case 148:
		b->Actions[MONSTER01_WALK   ].PlaySpeed = 0.46f;
		break;
	case 149:
		b->Actions[MONSTER01_WALK].PlaySpeed = 0.25f;
		b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.21f;
		b->Actions[MONSTER01_DIE].PlaySpeed = 0.23f;
		break;
	case 150:
		b->Actions[MONSTER01_WALK].PlaySpeed = 0.20f;
		b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.25f;
		b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.25f;
		b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.35f;
		b->Actions[MONSTER01_DIE].PlaySpeed = 0.18f;
		b->Actions[MONSTER01_APEAR].PlaySpeed = 0.25f;
		b->Actions[MONSTER01_ATTACK3].PlaySpeed = 0.25f;
		b->Actions[MONSTER01_ATTACK4].PlaySpeed = 0.25f;
		break;
	case 151:
	case 152:
	case 153:
		break;
	case 155:
		b->Actions[MONSTER01_STOP2].PlaySpeed = 0.3f;
		b->Actions[MONSTER01_WALK].PlaySpeed = 0.29f;
		b->Actions[MONSTER01_DIE].PlaySpeed = 0.4f;
		break;
		case 164:
			b->Actions[MONSTER01_STOP1].PlaySpeed =		0.30f;
			b->Actions[MONSTER01_STOP2].PlaySpeed =		0.30f;
			b->Actions[MONSTER01_WALK].PlaySpeed =		0.40f;
			b->Actions[MONSTER01_ATTACK1].PlaySpeed =	0.38f;
			b->Actions[MONSTER01_ATTACK2].PlaySpeed =	0.40f;
			b->Actions[MONSTER01_SHOCK].PlaySpeed =		0.50f;
			b->Actions[MONSTER01_DIE].PlaySpeed =		0.20f;
			b->Actions[MONSTER01_APEAR].PlaySpeed =		0.40f;
			b->Actions[MONSTER01_ATTACK3].PlaySpeed =	0.38f;
			b->Actions[MONSTER01_ATTACK4].PlaySpeed =	0.38f;
			break;
		case 165:
			b->Actions[MONSTER01_STOP1].PlaySpeed =		0.30f;
			b->Actions[MONSTER01_STOP2].PlaySpeed =		0.30f;
			b->Actions[MONSTER01_WALK].PlaySpeed =		0.50f;
			b->Actions[MONSTER01_ATTACK1].PlaySpeed =	0.86f;
			b->Actions[MONSTER01_ATTACK2].PlaySpeed =	0.86f;
			b->Actions[MONSTER01_SHOCK].PlaySpeed =		0.50f;
			b->Actions[MONSTER01_DIE].PlaySpeed =		0.40f;
			b->Actions[MONSTER01_APEAR].PlaySpeed =		0.40f;
			b->Actions[MONSTER01_ATTACK3].PlaySpeed =	0.76f;
			break;
		case 166:
			b->Actions[MONSTER01_STOP1].PlaySpeed =		0.60f;
			b->Actions[MONSTER01_STOP2].PlaySpeed =		0.60f;
			b->Actions[MONSTER01_WALK].PlaySpeed =		0.55f;
			b->Actions[MONSTER01_ATTACK1].PlaySpeed =	0.75f;
			b->Actions[MONSTER01_ATTACK2].PlaySpeed =	0.50f;
			b->Actions[MONSTER01_SHOCK].PlaySpeed =		0.50f;
			b->Actions[MONSTER01_DIE].PlaySpeed =		0.30f;
			b->Actions[MONSTER01_ATTACK3].PlaySpeed =	0.50f;
			b->Actions[MONSTER01_APEAR].PlaySpeed =		0.80f;
			break;
		case 167:
			b->Actions[MONSTER01_STOP1].PlaySpeed =		0.60f;
			b->Actions[MONSTER01_STOP2].PlaySpeed =		0.60f;
			b->Actions[MONSTER01_WALK].PlaySpeed =		0.50f;
			b->Actions[MONSTER01_ATTACK1].PlaySpeed =	0.71f;
			b->Actions[MONSTER01_ATTACK2].PlaySpeed =	0.40f;
			b->Actions[MONSTER01_SHOCK].PlaySpeed =		0.50f;
			b->Actions[MONSTER01_DIE].PlaySpeed =		0.30f;
			b->Actions[MONSTER01_APEAR].PlaySpeed =		0.80f;
			b->Actions[MONSTER01_ATTACK3].PlaySpeed =	0.40f;
			break;
		case 168:
			b->Actions[MONSTER01_STOP1].PlaySpeed =		0.30f;
			b->Actions[MONSTER01_STOP2].PlaySpeed =		0.30f;
			b->Actions[MONSTER01_WALK].PlaySpeed =		0.40f;
			b->Actions[MONSTER01_ATTACK1].PlaySpeed =	0.33f;
			b->Actions[MONSTER01_ATTACK2].PlaySpeed =	0.38f;
			b->Actions[MONSTER01_SHOCK].PlaySpeed =		0.50f;
			b->Actions[MONSTER01_DIE].PlaySpeed =		0.30f;
			b->Actions[MONSTER01_ATTACK3].PlaySpeed =	0.40f;
			b->Actions[MONSTER01_ATTACK4].PlaySpeed =	0.45f;
			break;
		case 169:
			b->Actions[MONSTER01_STOP1].PlaySpeed =		0.30f;
			b->Actions[MONSTER01_STOP2].PlaySpeed =		0.30f;
			b->Actions[MONSTER01_WALK].PlaySpeed =		0.35f;
			b->Actions[MONSTER01_ATTACK1].PlaySpeed =	0.40f;
			b->Actions[MONSTER01_ATTACK2].PlaySpeed =	0.45f;
			b->Actions[MONSTER01_SHOCK].PlaySpeed =		0.50f;
			b->Actions[MONSTER01_DIE].PlaySpeed =		0.40f;
			b->Actions[MONSTER01_APEAR].PlaySpeed =		0.45f;
			b->Actions[MONSTER01_ATTACK3].PlaySpeed =	0.45f;
			break;
		case 170:
			b->Actions[MONSTER01_STOP1].PlaySpeed =		0.30f;
			b->Actions[MONSTER01_STOP2].PlaySpeed =		0.30f;
			b->Actions[MONSTER01_WALK].PlaySpeed =		0.35f;
			b->Actions[MONSTER01_ATTACK1].PlaySpeed =	0.40f;
			b->Actions[MONSTER01_ATTACK2].PlaySpeed =	0.35f;
			b->Actions[MONSTER01_SHOCK].PlaySpeed =		0.50f;
			b->Actions[MONSTER01_DIE].PlaySpeed =		0.35f;
			b->Actions[MONSTER01_ATTACK3].PlaySpeed =	0.40f;
			break;
		case 171:
			b->Actions[MONSTER01_STOP1].PlaySpeed =		0.30f;
			b->Actions[MONSTER01_STOP2].PlaySpeed =		0.30f;
			b->Actions[MONSTER01_WALK].PlaySpeed =		0.40f;
			b->Actions[MONSTER01_ATTACK1].PlaySpeed =	0.45f;
			b->Actions[MONSTER01_ATTACK2].PlaySpeed =	0.50f;
			b->Actions[MONSTER01_SHOCK].PlaySpeed =		0.50f;
			b->Actions[MONSTER01_DIE].PlaySpeed =		0.30f;
			b->Actions[MONSTER01_ATTACK3].PlaySpeed =	0.45f;
			break;
		case 172:
			b->Actions[MONSTER01_STOP1].PlaySpeed =		0.25f;
			b->Actions[MONSTER01_STOP2].PlaySpeed =		0.20f;
			b->Actions[MONSTER01_WALK].PlaySpeed =		0.50f;
			b->Actions[MONSTER01_ATTACK1].PlaySpeed =	0.66f;
			b->Actions[MONSTER01_ATTACK2].PlaySpeed =	0.33f;
			b->Actions[MONSTER01_SHOCK].PlaySpeed =		0.50f;
			b->Actions[MONSTER01_DIE].PlaySpeed =		0.55f;
			break;
		case 173:
			b->Actions[MONSTER01_STOP1].PlaySpeed =		0.25f;
			b->Actions[MONSTER01_STOP2].PlaySpeed =		0.20f;
			b->Actions[MONSTER01_WALK].PlaySpeed =		0.40f;
			b->Actions[MONSTER01_ATTACK1].PlaySpeed =	0.33f;
			b->Actions[MONSTER01_ATTACK2].PlaySpeed =	0.36f;
			b->Actions[MONSTER01_SHOCK].PlaySpeed =		0.50f;
			b->Actions[MONSTER01_DIE].PlaySpeed =		0.30f;
			b->Actions[MONSTER01_APEAR].PlaySpeed =		0.40f;
			break;
		case 174:
			b->Actions[MONSTER01_STOP1].PlaySpeed =		0.30f;
			b->Actions[MONSTER01_STOP2].PlaySpeed =		0.30f;
			b->Actions[MONSTER01_WALK].PlaySpeed =		0.65f;
			b->Actions[MONSTER01_ATTACK1].PlaySpeed =	0.86f;
			b->Actions[MONSTER01_ATTACK2].PlaySpeed =	0.86f;
			b->Actions[MONSTER01_SHOCK].PlaySpeed =		0.50f;
			b->Actions[MONSTER01_DIE].PlaySpeed =		0.30f;
			b->Actions[MONSTER01_APEAR].PlaySpeed =		0.86f;
			break;
		case 175:
			b->Actions[MONSTER01_STOP1].PlaySpeed =		0.60f;
			b->Actions[MONSTER01_STOP2].PlaySpeed =		0.60f;
			b->Actions[MONSTER01_WALK].PlaySpeed =		0.80f;
			b->Actions[MONSTER01_ATTACK1].PlaySpeed =	0.96f;
			b->Actions[MONSTER01_ATTACK2].PlaySpeed =	0.96f;
			b->Actions[MONSTER01_SHOCK].PlaySpeed =		1.00f;
			b->Actions[MONSTER01_DIE].PlaySpeed =		0.30f;
			break;
		case 176:
			b->Actions[MONSTER01_STOP1].PlaySpeed =		0.25f;
			b->Actions[MONSTER01_STOP2].PlaySpeed =		0.20f;
			b->Actions[MONSTER01_WALK].PlaySpeed =		0.20f;
			b->Actions[MONSTER01_ATTACK1].PlaySpeed =	0.40f;
			b->Actions[MONSTER01_ATTACK2].PlaySpeed =	0.38f;
			b->Actions[MONSTER01_SHOCK].PlaySpeed =		0.50f;
			b->Actions[MONSTER01_DIE].PlaySpeed =		0.40f;
			b->Actions[MONSTER01_ATTACK3].PlaySpeed =	0.38f;
			break;
		case 177:
			b->Actions[MONSTER01_STOP1].PlaySpeed =		0.30f;
			b->Actions[MONSTER01_STOP2].PlaySpeed =		0.30f;
			b->Actions[MONSTER01_WALK].PlaySpeed =		0.37f;
			b->Actions[MONSTER01_ATTACK1].PlaySpeed =	0.33f;
			b->Actions[MONSTER01_ATTACK2].PlaySpeed =	0.33f;
			b->Actions[MONSTER01_SHOCK].PlaySpeed =		0.50f;
			b->Actions[MONSTER01_DIE].PlaySpeed =		0.30f;
			break;
		case 178:
			b->Actions[MONSTER01_STOP1].PlaySpeed =		0.40f;
			b->Actions[MONSTER01_STOP2].PlaySpeed =		0.40f;
			b->Actions[MONSTER01_WALK].PlaySpeed =		0.40f;
			b->Actions[MONSTER01_ATTACK1].PlaySpeed =	0.40f;
			b->Actions[MONSTER01_ATTACK2].PlaySpeed =	0.40f;
			b->Actions[MONSTER01_SHOCK].PlaySpeed =		0.40f;
			b->Actions[MONSTER01_DIE].PlaySpeed =		0.45f;
			b->Actions[MONSTER01_APEAR].PlaySpeed =		0.40f;
			break;
	#ifdef LDS_ADD_EG_2_MONSTER_GUARDIANPRIEST
		case 179:
			b->Actions[MONSTER01_STOP1].PlaySpeed =		0.80f;
			b->Actions[MONSTER01_STOP2].PlaySpeed =		0.80f;
			b->Actions[MONSTER01_WALK].PlaySpeed =		0.80f;
			b->Actions[MONSTER01_ATTACK1].PlaySpeed =	0.80f;
			b->Actions[MONSTER01_ATTACK2].PlaySpeed =	0.80f;
			b->Actions[MONSTER01_SHOCK].PlaySpeed =		0.40f;
			b->Actions[MONSTER01_DIE].PlaySpeed =		0.40f;
			break;
	#endif // LDS_ADD_EG_2_MONSTER_GUARDIANPRIEST
		case 180:
	 		b->Actions[MONSTER01_STOP1].PlaySpeed =		0.25f;
	 		b->Actions[MONSTER01_STOP2].PlaySpeed =		0.20f;
	 		b->Actions[MONSTER01_WALK].PlaySpeed =		0.55f;
	 		b->Actions[MONSTER01_ATTACK1].PlaySpeed =	0.66f;
	 		b->Actions[MONSTER01_ATTACK2].PlaySpeed =	0.66f;
	 		b->Actions[MONSTER01_SHOCK].PlaySpeed =		0.50f;
	 		b->Actions[MONSTER01_DIE].PlaySpeed =		0.55f;
			break;
		case 181:
			b->Actions[MONSTER01_STOP1].PlaySpeed =		0.30f;
			b->Actions[MONSTER01_STOP2].PlaySpeed =		0.30f;
			b->Actions[MONSTER01_WALK].PlaySpeed =		0.35f;
			b->Actions[MONSTER01_ATTACK1].PlaySpeed =	0.33f;
			b->Actions[MONSTER01_ATTACK2].PlaySpeed =	0.33f;
			b->Actions[MONSTER01_SHOCK].PlaySpeed =		0.50f;
			b->Actions[MONSTER01_DIE].PlaySpeed =		0.30f;
			break;
		case 189:
		case 190:
			b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.25f;
			b->Actions[MONSTER01_DIE].PlaySpeed = 0.3f;
			break;
		case 191:
			b->Actions[MONSTER01_STOP1].PlaySpeed = 0.25f * 2.0f;
			b->Actions[MONSTER01_STOP2].PlaySpeed = 0.2f * 2.0f;
			b->Actions[MONSTER01_WALK].PlaySpeed = 0.34f * 2.0f;
			b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.25f * 2.0f;
			b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.25f * 2.0f;
			b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.5f * 2.0f;
			b->Actions[MONSTER01_DIE].PlaySpeed = 0.55f * 2.0f;
			b->Actions[MONSTER01_APEAR].PlaySpeed = 0.33f * 3.0f;
			break;
		case 192:
			b->Actions[MONSTER01_STOP1].PlaySpeed =		0.30f;
			b->Actions[MONSTER01_STOP2].PlaySpeed =		0.30;
			b->Actions[MONSTER01_WALK].PlaySpeed =		0.30;
			b->Actions[MONSTER01_ATTACK1].PlaySpeed =	0.30;
			b->Actions[MONSTER01_ATTACK2].PlaySpeed =	0.30;
			b->Actions[MONSTER01_SHOCK].PlaySpeed =		0.30;
			b->Actions[MONSTER01_DIE].PlaySpeed =		0.30;
			b->Actions[MONSTER01_APEAR].PlaySpeed =		0.30;
			b->Actions[MONSTER01_ATTACK3].PlaySpeed =	0.30;
			break;
		case 205:
			break;	
		case 206:
			b->Actions[MONSTER01_DIE].PlaySpeed			= 0.13f;
			break;
		case 207:
			b->Actions[MONSTER01_WALK   ].PlaySpeed		= 0.46f;
			break;
		case 208:
			b->Actions[MONSTER01_WALK].PlaySpeed		= 0.25f;
			b->Actions[MONSTER01_ATTACK1].PlaySpeed		= 0.21f;
			b->Actions[MONSTER01_DIE].PlaySpeed			= 0.23f;
			break;
		case 193:
			break;
		case 194:
			b->Actions[MONSTER01_STOP1  ].PlaySpeed		= 0.15f;
			b->Actions[MONSTER01_STOP2  ].PlaySpeed		= 0.15f;
			b->Actions[MONSTER01_ATTACK1].PlaySpeed		= 0.23f;
			b->Actions[MONSTER01_ATTACK2].PlaySpeed		= 0.23f;
			b->Actions[MONSTER01_WALK].PlaySpeed		= 0.3f;
			b->Actions[MONSTER01_DIE].PlaySpeed			= 0.15f;
			break;
		case 195:
			b->Actions[MONSTER01_DIE].PlaySpeed			= 0.2f;
			break;
		case 196:
			b->Actions[MONSTER01_DIE].PlaySpeed			= 0.2f;
			break;
		case 201:
			b->Actions[MONSTER01_DIE  ].PlaySpeed = 0.25f;
			break;
		case 202:
			b->Actions[MONSTER01_WALK ].PlaySpeed = 0.25f;
			b->Actions[MONSTER01_DIE  ].PlaySpeed = 0.20f;
			break;
		case 203:
			b->Actions[MONSTER01_DIE  ].PlaySpeed = 0.25f;
			break;
		case 197:
			b->Actions[MONSTER01_STOP1  ].PlaySpeed = 0.25f;
			b->Actions[MONSTER01_STOP2  ].PlaySpeed = 0.25f;
			b->Actions[MONSTER01_WALK   ].PlaySpeed = 0.23f;
			b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.28f;
			b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.28f;
			b->Actions[MONSTER01_DIE].PlaySpeed = 0.19f;
			break;
		case 198:
			b->Actions[MONSTER01_STOP1  ].PlaySpeed = 0.25f;
			b->Actions[MONSTER01_STOP2  ].PlaySpeed = 0.25f;
			b->Actions[MONSTER01_WALK   ].PlaySpeed = 0.27f;
			b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.27f;
			b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.27f;
			b->Actions[MONSTER01_DIE].PlaySpeed = 0.25f;
			break;
		case 199:
			b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.26f;
			b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.26f;
			b->Actions[MONSTER01_DIE].PlaySpeed = 0.21f;
			break;
		case 200:
			b->Actions[MONSTER01_STOP1  ].PlaySpeed = 0.25f;
			b->Actions[MONSTER01_STOP2  ].PlaySpeed = 0.25f;
			b->Actions[MONSTER01_WALK   ].PlaySpeed = 0.25f;
			b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.3f;
			b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.3f;
			b->Actions[MONSTER01_SHOCK  ].PlaySpeed = 0.5f;
			b->Actions[MONSTER01_DIE].PlaySpeed = 0.3f;
			break;
#ifdef ASG_ADD_KARUTAN_MONSTERS
		case 209:
			b->Actions[MONSTER01_WALK   ].PlaySpeed = 0.95f;
			break;
		case 210:
			b->Actions[MONSTER01_WALK   ].PlaySpeed = 1.00f;
			b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.40f;
			b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.40f;
			b->Actions[MONSTER01_DIE    ].PlaySpeed = 0.20f;
			break;
		case 211:
			b->Actions[MONSTER01_STOP1  ].PlaySpeed = 0.7f;
			b->Actions[MONSTER01_STOP2  ].PlaySpeed = 0.7f;
			b->Actions[MONSTER01_WALK   ].PlaySpeed = 0.6f;
			b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.8f;
			b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.8f;
			b->Actions[MONSTER01_SHOCK  ].PlaySpeed = 0.25f;
			b->Actions[MONSTER01_DIE    ].PlaySpeed = 0.3f;
			break;
		case 212:
			b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.4f;
			b->Actions[MONSTER01_DIE    ].PlaySpeed = 0.3f;
			break;
		case 213:
		case 214:
			b->Actions[MONSTER01_WALK   ].PlaySpeed = 0.9f;
			b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.37f;
			b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.37f;
			b->Actions[MONSTER01_DIE    ].PlaySpeed = 0.25f;
			break;
		case 215:
			b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.80f;
			b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.80f;
			break;
		case 216:
			b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.75f;
			b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.75f;
			break;
#endif	// ASG_ADD_KARUTAN_MONSTERS
	}
	switch(Type)
	{
	case 14:
	case 31:
	case 39:
	case 40:
		b->Actions[MONSTER01_STOP2  ].Loop = true;
		break;
	}

    int Channel = 2;
	bool Enable = true;
	switch(Type)
	{
    case 70:
    case 71:
		LoadWaveFile ( SOUND_MONSTER+161, "Data\\Sound\\mOrcCapAttack1.wav", Channel, Enable );
		SetMonsterSound ( MODEL_MONSTER01+Type, -1, -1, 161, 161, -1 );
		Models[MODEL_MONSTER01+Type].BoneHead = 20;
        break;

    case 72:
		LoadWaveFile ( SOUND_MONSTER+162, "Data\\Sound\\mOrcArcherAttack1.wav"  ,Channel, Enable );
		SetMonsterSound ( MODEL_MONSTER01+Type, -1, -1, 162, 162, -1 );
		Models[MODEL_MONSTER01+Type].BoneHead = 20;
        break;
	case 0:
	case 30:
		LoadWaveFile(SOUND_MONSTER	,"Data\\Sound\\mBull1.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+1,"Data\\Sound\\mBull2.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+2,"Data\\Sound\\mBullAttack1.wav",Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+3,"Data\\Sound\\mBullAttack2.wav",Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+4,"Data\\Sound\\mBullDie.wav" ,Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type,0 ,1 ,2 ,3 ,4 );
		Models[MODEL_MONSTER01+Type].BoneHead = 20;
		break;
	case 1:
		LoadWaveFile(SOUND_MONSTER+5,"Data\\Sound\\mHound1.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+6,"Data\\Sound\\mHound2.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+7,"Data\\Sound\\mHoundAttack1.wav",Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+8,"Data\\Sound\\mHoundAttack2.wav",Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+9,"Data\\Sound\\mHoundDie.wav" ,Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type,5 ,6 ,7 ,8 ,9 );
		Models[MODEL_MONSTER01+Type].BoneHead = 5;
		break;
	case 2:
		LoadWaveFile(SOUND_MONSTER+10,"Data\\Sound\\mBudge1.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+11,"Data\\Sound\\mBudgeAttack1.wav",Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+12,"Data\\Sound\\mBudgeDie.wav" ,Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type,10,11,11,11,12);
		Models[MODEL_MONSTER01+Type].BoneHead = 7;
		break;
	case 9:
		LoadWaveFile(SOUND_MONSTER+13,"Data\\Sound\\mSpider1.wav"    ,Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type,13,13,13,13,13);
		break;
	case 3:
	case 29:
	case 39:
		LoadWaveFile(SOUND_MONSTER+15,"Data\\Sound\\mDarkKnight1.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+16,"Data\\Sound\\mDarkKnight2.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+17,"Data\\Sound\\mDarkKnightAttack1.wav",Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+18,"Data\\Sound\\mDarkKnightAttack2.wav",Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+19,"Data\\Sound\\mDarkKnightDie.wav" ,Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type,15,16,17,18,19);
		if(Type==3)
       		Models[MODEL_MONSTER01+Type].BoneHead = 16;
		else if(Type==29)
      		Models[MODEL_MONSTER01+Type].BoneHead = 20;
		else
      		Models[MODEL_MONSTER01+Type].BoneHead = 19;
		break;
	case 4:
		LoadWaveFile(SOUND_MONSTER+20,"Data\\Sound\\mWizard1.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+21,"Data\\Sound\\mWizard2.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+22,"Data\\Sound\\mWizardAttack1.wav",Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+23,"Data\\Sound\\mWizardAttack2.wav",Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+24,"Data\\Sound\\mWizardDie.wav" ,Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type,20,21,22,23,24);
		Models[MODEL_MONSTER01+Type].BoneHead = 20;
		break;
	case 5:
		LoadWaveFile(SOUND_MONSTER+25,"Data\\Sound\\mGiant1.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+26,"Data\\Sound\\mGiant2.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+27,"Data\\Sound\\mGiantAttack1.wav",Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+28,"Data\\Sound\\mGiantAttack2.wav",Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+29,"Data\\Sound\\mGiantDie.wav" ,Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type,25,26,27,28,29);
		Models[MODEL_MONSTER01+Type].BoneHead = 20;
		break;
	case 6:
		LoadWaveFile(SOUND_MONSTER+30,"Data\\Sound\\mLarva1.wav"      ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+31,"Data\\Sound\\mLarva2.wav"   ,Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type,30,31,30,31,31);
		break;
	case 8:
		LoadWaveFile(SOUND_MONSTER+32,"Data\\Sound\\mHellSpider1.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+33,"Data\\Sound\\mHellSpiderAttack1.wav",Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+34,"Data\\Sound\\mHellSpiderDie.wav" ,Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type,32,33,33,33,34);
		Models[MODEL_MONSTER01+Type].BoneHead = 16;
		break;
	case 7:
		LoadWaveFile(SOUND_MONSTER+35,"Data\\Sound\\mGhost1.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+36,"Data\\Sound\\mGhost2.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+37,"Data\\Sound\\mGhostAttack1.wav",Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+38,"Data\\Sound\\mGhostAttack2.wav",Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+39,"Data\\Sound\\mGhostDie.wav" ,Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type,35,36,37,38,39);
		Models[MODEL_MONSTER01+Type].BoneHead = 20;
		break;
	case 10:
		LoadWaveFile(SOUND_MONSTER+40,"Data\\Sound\\mOgre1.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+41,"Data\\Sound\\mOgre2.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+42,"Data\\Sound\\mOgreAttack1.wav",Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+43,"Data\\Sound\\mOgreAttack2.wav",Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+44,"Data\\Sound\\mOgreDie.wav" ,Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type,40,41,42,43,44);
		Models[MODEL_MONSTER01+Type].BoneHead = 20;
		break;
	case 11:
		LoadWaveFile(SOUND_MONSTER+45,"Data\\Sound\\mGorgon1.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+46,"Data\\Sound\\mGorgon2.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+47,"Data\\Sound\\mGorgonAttack1.wav",Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+48,"Data\\Sound\\mGorgonAttack2.wav",Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+49,"Data\\Sound\\mGorgonDie.wav" ,Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type,45,46,47,48,49);
		Models[MODEL_MONSTER01+Type].BoneHead = 20;
		break;
	case 15:
		LoadWaveFile(SOUND_MONSTER+50,"Data\\Sound\\mIceMonster1.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+51,"Data\\Sound\\mIceMonster2.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+52,"Data\\Sound\\mIceMonsterDie.wav" ,Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type,50,51,50,50,52);
		Models[MODEL_MONSTER01+Type].BoneHead = 19;
		break;
	case 17:
		LoadWaveFile(SOUND_MONSTER+53,"Data\\Sound\\mWorm1.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+54,"Data\\Sound\\mWorm2.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+55,"Data\\Sound\\mWormDie.wav" ,Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type,53,53,55,55,55);
		break;
	case 16:
		LoadWaveFile(SOUND_MONSTER+56,"Data\\Sound\\mHomord1.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+57,"Data\\Sound\\mHomord2.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+58,"Data\\Sound\\mHomordAttack1.wav",Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+59,"Data\\Sound\\mHomordDie.wav" ,Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type,56,57,58,58,59);
		break;
	case 18:
		LoadWaveFile(SOUND_MONSTER+60,"Data\\Sound\\mIceQueen1.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+61,"Data\\Sound\\mIceQueen2.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+62,"Data\\Sound\\mIceQueenAttack1.wav",Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+63,"Data\\Sound\\mIceQueenAttack2.wav",Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+64,"Data\\Sound\\mIceQueenDie.wav" ,Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type,60,61,62,63,64);
		Models[MODEL_MONSTER01+Type].BoneHead = 16;
		break;
	case 14:
		LoadWaveFile(SOUND_MONSTER+65,"Data\\Sound\\mAssassinAttack1.wav",Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+66,"Data\\Sound\\mAssassinAttack2.wav",Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+67,"Data\\Sound\\mAssassinDie.wav" ,Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type,-1,-1,65,66,67);
		Models[MODEL_MONSTER01+Type].BoneHead = 20;
	    b->Actions[MONSTER01_STOP2].PlaySpeed = 0.35f;
		break;
	case 12:
	case 13:
		LoadWaveFile(SOUND_MONSTER+68,"Data\\Sound\\mYeti1.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+69,"Data\\Sound\\mYeti2.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+70,"Data\\Sound\\mYetiAttack1.wav",Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+71,"Data\\Sound\\mYetiDie.wav" ,Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+12,68,68,70,70,71);
		SetMonsterSound(MODEL_MONSTER01+13,68,69,70,70,71);
		Models[MODEL_MONSTER01+12].BoneHead = 20;
		Models[MODEL_MONSTER01+13].BoneHead = 20;
		break;
	case 19:
		LoadWaveFile(SOUND_MONSTER+72,"Data\\Sound\\mGoblin1.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+73,"Data\\Sound\\mGoblin2.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+74,"Data\\Sound\\mGoblinAttack1.wav",Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+75,"Data\\Sound\\mGoblinAttack2.wav",Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+76,"Data\\Sound\\mGoblinDie.wav" ,Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type,72,73,74,75,76);
		Models[MODEL_MONSTER01+Type].BoneHead = 6;
		break;
	case 20:
		LoadWaveFile(SOUND_MONSTER+77,"Data\\Sound\\mScorpion1.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+78,"Data\\Sound\\mScorpion2.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+79,"Data\\Sound\\mScorpionAttack1.wav",Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+80,"Data\\Sound\\mScorpionAttack2.wav",Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+81,"Data\\Sound\\mScorpionDie.wav" ,Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type,77,78,79,80,81);
		break;
	case 21:
		LoadWaveFile(SOUND_MONSTER+82,"Data\\Sound\\mBeetle1.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+83,"Data\\Sound\\mBeetleAttack1.wav",Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+84,"Data\\Sound\\mBeetleDie.wav" ,Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type,82,82,83,83,84);
		Models[MODEL_MONSTER01+Type].BoneHead = 5;
		break;
	case 22:
		LoadWaveFile(SOUND_MONSTER+85,"Data\\Sound\\mHunter1.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+86,"Data\\Sound\\mHunter2.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+87,"Data\\Sound\\mHunterAttack1.wav",Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+88,"Data\\Sound\\mHunterAttack2.wav",Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+89,"Data\\Sound\\mHunterDie.wav" ,Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type,85,86,87,88,89);
		Models[MODEL_MONSTER01+Type].BoneHead = 6;
		break;
	case 23:
		LoadWaveFile(SOUND_MONSTER+90,"Data\\Sound\\mWoodMon1.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+91,"Data\\Sound\\mWoodMon2.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+92,"Data\\Sound\\mWoodMonAttack1.wav",Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+93,"Data\\Sound\\mWoodMonAttack2.wav",Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+94,"Data\\Sound\\mWoodMonDie.wav" ,Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type,90,91,92,93,94);
		Models[MODEL_MONSTER01+Type].BoneHead = 6;
		break;
	case 24:
		LoadWaveFile(SOUND_MONSTER+95,"Data\\Sound\\mArgon1.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+96,"Data\\Sound\\mArgon2.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+97,"Data\\Sound\\mArgonAttack1.wav",Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+98,"Data\\Sound\\mArgonAttack2.wav",Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+99,"Data\\Sound\\mArgonDie.wav" ,Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type,95,96,97,98,99);
		Models[MODEL_MONSTER01+Type].BoneHead = 16;
		break;
	case 25:
		LoadWaveFile(SOUND_MONSTER+100,"Data\\Sound\\mGolem1.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+101,"Data\\Sound\\mGolem2.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+102,"Data\\Sound\\mGolemAttack1.wav",Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+103,"Data\\Sound\\mGolemAttack2.wav",Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+104,"Data\\Sound\\mGolemDie.wav" ,Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type,100,101,102,103,104);
		Models[MODEL_MONSTER01+Type].BoneHead = 5;
		break;
	case 26:
		LoadWaveFile(SOUND_MONSTER+105,"Data\\Sound\\mYeti1.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+106,"Data\\Sound\\mSatanAttack1.wav",Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+107,"Data\\Sound\\mYetiDie.wav" ,Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type,105,105,106,106,107);
		Models[MODEL_MONSTER01+Type].BoneHead = 6;
		break;
	case 27:
		LoadWaveFile(SOUND_MONSTER+108,"Data\\Sound\\mBalrog1.wav"      ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+109,"Data\\Sound\\mBalrog2.wav"      ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+110,"Data\\Sound\\mWizardAttack2.wav",Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+111,"Data\\Sound\\mGorgonAttack2.wav"  ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+112,"Data\\Sound\\mBalrogDie.wav"   ,Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type,108,109,110,111,112);
		Models[MODEL_MONSTER01+Type].BoneHead = 6;
		Models[MODEL_MONSTER01+Type].StreamMesh = 1;
		break;
	case 28:
		LoadWaveFile(SOUND_MONSTER+113,"Data\\Sound\\mShadow1.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+114,"Data\\Sound\\mShadow2.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+115,"Data\\Sound\\mShadowAttack1.wav",Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+116,"Data\\Sound\\mShadowAttack2.wav",Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+117,"Data\\Sound\\mShadowDie.wav" ,Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type,113,114,115,116,117);
		Models[MODEL_MONSTER01+Type].BoneHead = 5;
		break;
	case 31:
		LoadWaveFile(SOUND_MONSTER+123,"Data\\Sound\\mYeti1.wav"      ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+124,"Data\\Sound\\mBullAttack1.wav"  ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+125,"Data\\Sound\\mYetiDie.wav"   ,Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type,123,123,124,124,125);
		b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.5f;
		b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.7f;
		b->Actions[MONSTER01_STOP2  ].PlaySpeed = 0.8f;
		b->Actions[MONSTER01_DIE+1  ].PlaySpeed = 0.8f;
		break;
	case 32:
		LoadWaveFile(SOUND_MONSTER+126,"Data\\Sound\\mBali1.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+127,"Data\\Sound\\mBali2.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+128,"Data\\Sound\\mBaliAttack1.wav",Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+129,"Data\\Sound\\mBaliAttack2.wav",Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type,126,127,128,129,127);
		b->Actions[MONSTER01_ATTACK3].PlaySpeed = 0.4f;
		b->Actions[MONSTER01_ATTACK4].PlaySpeed = 0.4f;
		b->Actions[MONSTER01_APEAR  ].PlaySpeed = 0.4f;
		b->Actions[MONSTER01_RUN    ].PlaySpeed = 0.4f;
		b->BoneHead = 6;
		break;

	case 33:
		LoadWaveFile(SOUND_MONSTER+130,"Data\\Sound\\mBahamut1.wav",Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+131,"Data\\Sound\\mYeti1.wav"    ,Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type,130,130,131,131,130);
		break;
	case 34:
		LoadWaveFile(SOUND_MONSTER+132,"Data\\Sound\\mBepar1.wav"  ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+133,"Data\\Sound\\mBepar2.wav"  ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+134,"Data\\Sound\\mBalrog1.wav"    ,Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type,132,133,104,104,133);
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.5f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.5f;
		b->BoneHead = 20;//인어
		break;
	case 35:
		LoadWaveFile(SOUND_MONSTER+135,"Data\\Sound\\mValkyrie1.wav"   ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+136,"Data\\Sound\\mBaliAttack2.wav" ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+137,"Data\\Sound\\mValkyrieDie.wav",Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type,135,135,136,136,137);
		Models[MODEL_MONSTER01+Type].BoneHead = 19;
		break;
	case 36:
	case 40:
		LoadWaveFile(SOUND_MONSTER+138,"Data\\Sound\\mLizardKing1.wav"  ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+139,"Data\\Sound\\mLizardKing2.wav"  ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+140,"Data\\Sound\\mGorgonDie.wav"   ,Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type,138,139,138,139,140);
		if(Type==36)
			Models[MODEL_MONSTER01+Type].BoneHead = 19;
		else
			Models[MODEL_MONSTER01+Type].BoneHead = 6;
		break;
	case 37:
		LoadWaveFile(SOUND_MONSTER+141,"Data\\Sound\\mHydra1.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+142,"Data\\Sound\\mHydraAttack1.wav",Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type,141,141,142,142,141);
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.15f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.15f;
        b->Actions[MONSTER01_DIE    ].PlaySpeed = 0.2f;
		break;
	case 41:
		LoadWaveFile(SOUND_MONSTER+143,"Data\\Sound\\iron1.wav"       ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+144,"Data\\Sound\\iron_attack1.wav",Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type,143,143,144,144,144);
		Models[MODEL_MONSTER01+Type].BoneHead = 3;
		break;
	case 42:
		LoadWaveFile(SOUND_MONSTER+145,"Data\\Sound\\jaikan1.wav"       ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+146,"Data\\Sound\\jaikan2.wav"       ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+147,"Data\\Sound\\jaikan_attack1.wav",Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+148,"Data\\Sound\\jaikan_attack2.wav",Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+149,"Data\\Sound\\jaikan_die.wav"    ,Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type,145,146,147,148,149);
		LoadBitmap("Monster\\bv01_2.jpg"     ,BITMAP_MONSTER_SKIN    ,GL_LINEAR,GL_REPEAT);
		LoadBitmap("Monster\\bv02_2.jpg"     ,BITMAP_MONSTER_SKIN+1  ,GL_LINEAR,GL_REPEAT);
		Models[MODEL_MONSTER01+Type].BoneHead = 20;
		break;
	case 43:
		LoadWaveFile(SOUND_MONSTER+150,"Data\\Sound\\blood1.wav"       ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+151,"Data\\Sound\\blood_attack1.wav",Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+152,"Data\\Sound\\blood_attack2.wav",Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+153,"Data\\Sound\\blood_die.wav"    ,Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type,150,150,151,152,153);
		Models[MODEL_MONSTER01+Type].BoneHead = 7;
		break;
	case 44:
		LoadWaveFile(SOUND_MONSTER+154,"Data\\Sound\\death1.wav"       ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+155,"Data\\Sound\\death_attack1.wav",Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+156,"Data\\Sound\\death_die.wav"    ,Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type,154,154,155,155,156);
		Models[MODEL_MONSTER01+Type].BoneHead = 6;
		break;
	case 45:
		LoadWaveFile(SOUND_MONSTER+157,"Data\\Sound\\mutant1.wav"       ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+158,"Data\\Sound\\mutant2.wav"       ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+159,"Data\\Sound\\mutant_attack1.wav",Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type,157,158,159,159,159);
		Models[MODEL_MONSTER01+Type].BoneHead = 6;
		break;
	case 46:
		LoadWaveFile(SOUND_MONSTER+162,"Data\\Sound\\mOrcArcherAttack1.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+4,"Data\\Sound\\mBullDie.wav" ,Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type, -1, -1, 162, 162,4);
		Models[MODEL_MONSTER01+Type].BoneHead = 7;
		break;
	case 47:
		LoadWaveFile(SOUND_MONSTER+86,"Data\\Sound\\mHunter2.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+4,"Data\\Sound\\mBullDie.wav" ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+161,"Data\\Sound\\mOrcCapAttack1.wav"    ,Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type,86,86,161,161,4);
		Models[MODEL_MONSTER01+Type].BoneHead = 20;
		break;
	case 48:
		LoadWaveFile(SOUND_MONSTER+160,"Data\\Sound\\mCursedKing1.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+164,"Data\\Sound\\mCursedKing2.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+163,"Data\\Sound\\mCursedKingDie1.wav"    ,Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type,160,164,-1,-1,163);
		Models[MODEL_MONSTER01+Type].BoneHead = 20;
		break;
	case 52:
		LoadBitmap("Monster\\iui02.tga",BITMAP_ROBE+3);
		LoadBitmap("Monster\\iui03.tga",BITMAP_ROBE+5);
		b->Actions[MONSTER01_DIE    ].PlaySpeed = 0.22f;
		LoadWaveFile(SOUND_MONSTER+180,"Data\\Sound\\mMegaCrust1.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+181,"Data\\Sound\\mMegaCrustAttack1.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+182,"Data\\Sound\\mMegaCrustDie.wav"    ,Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type,180,180,181,181,182);
		break;
	case 49:
		b->Actions[MONSTER01_DIE    ].PlaySpeed = 0.22f;
		LoadWaveFile(SOUND_MONSTER+177,"Data\\Sound\\mMolt1.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+178,"Data\\Sound\\mMoltAttack1.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+179,"Data\\Sound\\mMoltDie.wav"    ,Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type,177,177,178,178,179);
		break;
	case 50:
		b->Actions[MONSTER01_DIE    ].PlaySpeed = 0.22f;
		//LoadWaveFile(SOUND_MONSTER+174,"Data\\Sound\\mAlquamos1.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+175,"Data\\Sound\\mAlquamosAttack1.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+176,"Data\\Sound\\mAlquamosDie.wav"    ,Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type,175,175,175,175,176);
		break;
	case 51:
		b->Actions[MONSTER01_DIE    ].PlaySpeed = 0.22f;
		LoadWaveFile(SOUND_MONSTER+171,"Data\\Sound\\mRainner1.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+172,"Data\\Sound\\mRainnerAttack1.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+173,"Data\\Sound\\mRainnerDie.wav"    ,Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type,171,-1,172,172,173);
		break;
	case 53:
		b->Actions[MONSTER01_DIE    ].PlaySpeed = 0.22f;
		LoadWaveFile(SOUND_MONSTER+168,"Data\\Sound\\mPhantom1.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+169,"Data\\Sound\\mPhantomAttack1.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+170,"Data\\Sound\\mPhantomDie.wav"    ,Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type,168,168,169,169,170);
		break;
	case 54:
		b->Actions[MONSTER01_DIE    ].PlaySpeed = 0.22f;
		LoadWaveFile(SOUND_MONSTER+165,"Data\\Sound\\mDrakan1.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+166,"Data\\Sound\\mDrakanAttack1.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+167,"Data\\Sound\\mDrakanDie.wav"    ,Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type,165,165,166,166,167);
		break;
	case 55:
		LoadWaveFile(SOUND_MONSTER+183,"Data\\Sound\\mPhoenix1.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+184,"Data\\Sound\\mPhoenix1.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+185,"Data\\Sound\\mPhoenixAttack1.wav"    ,Channel,Enable);
		//LoadWaveFile(SOUND_MONSTER+186,"Data\\Sound\\mDarkPhoenixDie.wav"    ,Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type,183,184,185,185,-1);
	case 56:
		b->Actions[MONSTER01_DIE    ].PlaySpeed = 0.22f;
		//b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.01f;
		//b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.01f;
		break;
    case 62:
		LoadWaveFile(SOUND_MONSTER+186,"Data\\Sound\\mMagicSkull.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+187,"Data\\Sound\\mMagicSkull.wav"    ,Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type,186,-1,-1,-1,187);
        break;  
    case 61:
        break;
    case 60:
        break;
    case 59:
		LoadWaveFile(SOUND_MONSTER+4,"Data\\Sound\\mBullDie.wav" ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+188,"Data\\Sound\\mHunter2.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+189,"Data\\Sound\\mBlackSkullDie.wav" ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+190,"Data\\Sound\\mBlackSkullAttack.wav"    ,Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type,188, -1, 190, -1, 189 );
        break;
    case 58:
		LoadWaveFile(SOUND_MONSTER+188,"Data\\Sound\\mHunter2.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+191,"Data\\Sound\\mGhaintOrgerDie.wav" ,Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type,188, -1, 190, -1, 191 );
        break;
    case 57:
		LoadWaveFile(SOUND_MONSTER+192,"Data\\Sound\\mRedSkull.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+193,"Data\\Sound\\mRedSkullDie.wav" ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+194,"Data\\Sound\\mRedSkullAttack.wav"    ,Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type,192, -1, 194, -1, 193 );
        break;
    case 63 :
		LoadWaveFile(SOUND_MONSTER+195,"Data\\Sound\\mDAngelIdle.wav"   ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+196,"Data\\Sound\\mDAngelAttack.wav"	,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+197,"Data\\Sound\\mDAngelDeath.wav"  ,Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type,195, 195, 196, 196, 197 );
        break;
    case 64 :
		LoadWaveFile(SOUND_MONSTER+232,"Data\\Sound\\mKundunIdle.wav"       ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+233,"Data\\Sound\\mKundunAttack1.wav"	,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+234,"Data\\Sound\\mKundunAttack2.wav"	,Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type,232, 232, 233, 234, -1 );
        break;
	case 65 :
		LoadWaveFile(SOUND_MONSTER+198,"Data\\Sound\\mBSoldierIdle1.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+199,"Data\\Sound\\mBSoldierIdle2.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+200,"Data\\Sound\\mBSoldierAttack1.wav"	,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+201,"Data\\Sound\\mBSoldierAttack2.wav"	,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+202,"Data\\Sound\\mBSoldierDeath.wav"    ,Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type,198, 199, 200, 201, 202 );
		break;
	case 66 :
		LoadWaveFile(SOUND_MONSTER+203,"Data\\Sound\\mEsisIdle.wav"     ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+204,"Data\\Sound\\mEsisAttack1.wav"	,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+205,"Data\\Sound\\mEsisAttack2.wav"	,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+206,"Data\\Sound\\mEsisDeath.wav"    ,Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type,203, 203, 204, 205, 206 );
		break;
    case 67 :
		LoadWaveFile(SOUND_MONSTER+207,"Data\\Sound\\mDsIdle1.wav"      ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+208,"Data\\Sound\\mDsIdle2.wav"      ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+209,"Data\\Sound\\mDsAttack1.wav"	,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+210,"Data\\Sound\\mDsAttack2.wav"	,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+211,"Data\\Sound\\mDsDeath.wav"      ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+212,"Data\\Sound\\mLsIdle1.wav"      ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+213,"Data\\Sound\\mLsIdle2.wav"      ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+214,"Data\\Sound\\mLsAttack1.wav"	,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+215,"Data\\Sound\\mLsAttack2.wav"	,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+216,"Data\\Sound\\mLsDeath.wav"      ,Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type,207, 208, 209, 210, 211, 212, 213, 214, 215, 216 );
        break;
    case 68 :
		LoadWaveFile(SOUND_MONSTER+217,"Data\\Sound\\mNecronIdle1.wav"      ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+218,"Data\\Sound\\mNecronIdle2.wav"      ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+219,"Data\\Sound\\mNecronAttack1.wav"	,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+220,"Data\\Sound\\mNecronAttack2.wav"	,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+221,"Data\\Sound\\mNecronDeath.wav"      ,Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type,217, 218, 219, 220, 221 );
        break;
    case 69 :
		LoadWaveFile(SOUND_MONSTER+222,"Data\\Sound\\mSvIdle1.wav"      ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+223,"Data\\Sound\\mSvIdle2.wav"      ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+224,"Data\\Sound\\mSvAttack1.wav"	,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+225,"Data\\Sound\\mSvAttack2.wav"	,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+226,"Data\\Sound\\mSvDeath.wav"      ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+227,"Data\\Sound\\mLvIdle1.wav"      ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+228,"Data\\Sound\\mLvIdle2.wav"      ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+229,"Data\\Sound\\mLvAttack1.wav"	,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+230,"Data\\Sound\\mLvAttack2.wav"	,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+231,"Data\\Sound\\mLvDeath.wav"      ,Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type,222, 223, 224, 225, 226, 227, 228, 229, 230, 231 );
        break;
    case 73:
        LoadWaveFile ( SOUND_MONSTER+232, "Data\\Sound\\BattleCastle\\oCDoorDis.wav",    Channel, Enable );
		SetMonsterSound ( MODEL_MONSTER01+Type, -1, -1, -1, -1, 232 );
        break;
    case 86:
		SetMonsterSound ( MODEL_MONSTER01+Type, -1, -1, -1, -1, -1 );
	    b->Actions[MONSTER01_STOP1].PlaySpeed = 0.05f;
	    b->Actions[MONSTER01_STOP2].PlaySpeed = 0.05f;
	    b->Actions[MONSTER01_WALK].PlaySpeed = 0.1f;
        break;
    case 76:
        LoadWaveFile ( SOUND_MONSTER+233, "Data\\Sound\\BattleCastle\\m_BowMercAttack.wav",    Channel, Enable );
        LoadWaveFile ( SOUND_MONSTER+234, "Data\\Sound\\BattleCastle\\m_BowMercDeath.wav",    Channel, Enable );
        SetMonsterSound ( MODEL_MONSTER01+Type, -1, -1, 233, 233, 234 );
        break;
    case 77:
        LoadWaveFile ( SOUND_MONSTER+235, "Data\\Sound\\BattleCastle\\m_SpearMercAttack.wav",    Channel, Enable );
        LoadWaveFile ( SOUND_MONSTER+236, "Data\\Sound\\BattleCastle\\m_SpearMercDeath.wav",    Channel, Enable );
        SetMonsterSound ( MODEL_MONSTER01+Type, -1, -1, 235, 235, 236 );
        break;
    case 79:
        break;
	case 128:
		LoadWaveFile ( SOUND_ELBELAND_RABBITSTRANGE_ATTACK01,		"Data\\Sound\\w52\\SE_Mon_rabbitstrange_attack01.wav", 1 );
		LoadWaveFile ( SOUND_ELBELAND_RABBITSTRANGE_DEATH01,		"Data\\Sound\\w52\\SE_Mon_rabbitstrange_death01.wav", 1 );
		break;
	case 129:
		LoadWaveFile ( SOUND_ELBELAND_RABBITUGLY_BREATH01,			"Data\\Sound\\w52\\SE_Mon_rabbitugly_breath01.wav", 1 );
		LoadWaveFile ( SOUND_ELBELAND_RABBITUGLY_ATTACK01,			"Data\\Sound\\w52\\SE_Mon_rabbitugly_attack01.wav", 1 );
		LoadWaveFile ( SOUND_ELBELAND_RABBITUGLY_DEATH01,			"Data\\Sound\\w52\\SE_Mon_rabbitugly_death01.wav", 1 );
		break;
	case 130:
		LoadWaveFile ( SOUND_ELBELAND_WOLFHUMAN_MOVE02,				"Data\\Sound\\w52\\SE_Mon_wolfhuman_move02.wav", 1 );
		LoadWaveFile ( SOUND_ELBELAND_WOLFHUMAN_ATTACK01,			"Data\\Sound\\w52\\SE_Mon_wolfhuman_attack01.wav", 1 );
		LoadWaveFile ( SOUND_ELBELAND_WOLFHUMAN_DEATH01,			"Data\\Sound\\w52\\SE_Mon_wolfhuman_death01.wav", 1 );
		break;
	case 131:
		LoadWaveFile ( SOUND_ELBELAND_BUTTERFLYPOLLUTION_MOVE01,	"Data\\Sound\\w52\\SE_Mon_butterflypollution_move01.wav", 1 );
		LoadWaveFile ( SOUND_ELBELAND_BUTTERFLYPOLLUTION_DEATH01,	"Data\\Sound\\w52\\SE_Mon_butterflypollution_death01.wav", 1 );
		break;
	case 132:
		LoadWaveFile ( SOUND_ELBELAND_CURSERICH_MOVE01,				"Data\\Sound\\w52\\SE_Mon_curserich_move01.wav", 1 );
		LoadWaveFile ( SOUND_ELBELAND_CURSERICH_ATTACK01,			"Data\\Sound\\w52\\SE_Mon_curserich_attack01.wav", 1 );
		LoadWaveFile ( SOUND_ELBELAND_CURSERICH_DEATH01,			"Data\\Sound\\w52\\SE_Mon_curserich_death01.wav", 1 );
		break;
	case 133:
		LoadWaveFile ( SOUND_ELBELAND_TOTEMGOLEM_MOVE01,			"Data\\Sound\\w52\\SE_Mon_totemgolem_move01.wav", 1 );
		LoadWaveFile ( SOUND_ELBELAND_TOTEMGOLEM_MOVE02,			"Data\\Sound\\w52\\SE_Mon_totemgolem_move02.wav", 1 );
		LoadWaveFile ( SOUND_ELBELAND_TOTEMGOLEM_ATTACK01,			"Data\\Sound\\w52\\SE_Mon_totemgolem_attack01.wav", 1 );
		LoadWaveFile ( SOUND_ELBELAND_TOTEMGOLEM_ATTACK02,			"Data\\Sound\\w52\\SE_Mon_totemgolem_attack02.wav", 1 );
		LoadWaveFile ( SOUND_ELBELAND_TOTEMGOLEM_DEATH01,			"Data\\Sound\\w52\\SE_Mon_totemgolem_death01.wav", 1 );
		break;
	case 134:
 		LoadWaveFile ( SOUND_ELBELAND_BEASTWOO_MOVE01,				"Data\\Sound\\w52\\SE_Mon_beastwoo_move01.wav", 1 );
		LoadWaveFile ( SOUND_ELBELAND_BEASTWOO_ATTACK01,			"Data\\Sound\\w52\\SE_Mon_beastwoo_attack01.wav", 1 );
		LoadWaveFile ( SOUND_ELBELAND_BEASTWOO_DEATH01,				"Data\\Sound\\w52\\SE_Mon_beastwoo_death01.wav", 1 );
		break;
	case 135:
 		LoadWaveFile ( SOUND_ELBELAND_BEASTWOOLEADER_MOVE01,		"Data\\Sound\\w52\\SE_Mon_beastwooleader_move01.wav", 1 );
		LoadWaveFile ( SOUND_ELBELAND_BEASTWOOLEADER_ATTACK01,		"Data\\Sound\\w52\\SE_Mon_beastwooleader_attack01.wav", 1 );
		LoadWaveFile ( SOUND_ELBELAND_BEASTWOO_DEATH01,				"Data\\Sound\\w52\\SE_Mon_beastwoo_death01.wav", 1 );
		break;
	case 136:
 		LoadWaveFile ( SOUND_SWAMPOFQUIET_SAPI_UNUS_ATTACK01,		"Data\\Sound\\w57\\Sapi-Attack.wav", 1 );
		LoadWaveFile ( SOUND_SWAMPOFQUIET_SAPI_DEATH01,				"Data\\Sound\\w57\\Sapi-Death.wav", 1 );
		break;
	case 137:
 		LoadWaveFile ( SOUND_SWAMPOFQUIET_SAPI_UNUS_ATTACK01,		"Data\\Sound\\w57\\Sapi-Attack.wav", 1 );
		LoadWaveFile ( SOUND_SWAMPOFQUIET_SAPI_DEATH01,				"Data\\Sound\\w57\\Sapi-Death.wav", 1 );
		break;
	case 138:
 		LoadWaveFile ( SOUND_SWAMPOFQUIET_SAPI_TRES_ATTACK01,		"Data\\Sound\\w57\\Sapi-Attack1.wav", 1 );
		LoadWaveFile ( SOUND_SWAMPOFQUIET_SAPI_DEATH01,				"Data\\Sound\\w57\\Sapi-Death.wav", 1 );
		break;
	case 139:
 		LoadWaveFile ( SOUND_SWAMPOFQUIET_SHADOW_PAWN_ATTACK01,		"Data\\Sound\\w57\\ShadowPawn-Attack.wav", 1 );
		LoadWaveFile ( SOUND_SWAMPOFQUIET_SHADOW_DEATH01,			"Data\\Sound\\w57\\Shadow-Death.wav", 1 );
		break;
	case 140:
 		LoadWaveFile ( SOUND_SWAMPOFQUIET_SHADOW_KNIGHT_ATTACK01,	"Data\\Sound\\w57\\ShadowKnight-Attack.wav", 1 );
		LoadWaveFile ( SOUND_SWAMPOFQUIET_SHADOW_DEATH01,			"Data\\Sound\\w57\\Shadow-Death.wav", 1 );
		break;
	case 141:
 		LoadWaveFile ( SOUND_SWAMPOFQUIET_SHADOW_ROOK_ATTACK01,		"Data\\Sound\\w57\\ShadowRook-Attack.wav", 1 );
		LoadWaveFile ( SOUND_SWAMPOFQUIET_SHADOW_DEATH01,			"Data\\Sound\\w57\\Shadow-Death.wav", 1 );
		break;
	case 142:
 		LoadWaveFile ( SOUND_SWAMPOFQUIET_THUNDER_NAIPIN_BREATH01,	"Data\\Sound\\w57\\Naipin-Thunder.wav", 1 );
		LoadWaveFile ( SOUND_SWAMPOFQUIET_NAIPIN_ATTACK01,			"Data\\Sound\\w57\\Naipin-Attack.wav", 1 );
		LoadWaveFile ( SOUND_SWAMPOFQUIET_NAIPIN_DEATH01,			"Data\\Sound\\w57\\Naipin-Death.wav", 1 );
		break;
	case 143:
 		LoadWaveFile ( SOUND_SWAMPOFQUIET_GHOST_NAIPIN_BREATH01,	"Data\\Sound\\w57\\Naipin-Ghost.wav", 1 );
		LoadWaveFile ( SOUND_SWAMPOFQUIET_NAIPIN_ATTACK01,			"Data\\Sound\\w57\\Naipin-Attack.wav", 1 );
		LoadWaveFile ( SOUND_SWAMPOFQUIET_NAIPIN_DEATH01,			"Data\\Sound\\w57\\Naipin-Death.wav", 1 );
		break;
	case 144:
 		LoadWaveFile ( SOUND_SWAMPOFQUIET_BLAZE_NAIPIN_BREATH01,	"Data\\Sound\\w57\\Naipin-Blaze.wav", 1 );
		LoadWaveFile ( SOUND_SWAMPOFQUIET_NAIPIN_ATTACK01,			"Data\\Sound\\w57\\Naipin-Attack.wav", 1 );
		LoadWaveFile ( SOUND_SWAMPOFQUIET_NAIPIN_DEATH01,			"Data\\Sound\\w57\\Naipin-Death.wav", 1 );
		break;
	case 145:
		LoadWaveFile( SOUND_ELBELAND_WOLFHUMAN_DEATH01,		"Data\\Sound\\w52\\SE_Mon_wolfhuman_death01.wav", 1 );
		LoadWaveFile( SOUND_RAKLION_ICEWALKER_ATTACK,		"Data\\Sound\\w58w59\\IceWalker_attack.wav", 1 );
		LoadWaveFile( SOUND_RAKLION_ICEWALKER_MOVE,			"Data\\Sound\\w58w59\\IceWalker_move.wav", 1 );
		break;
 		LoadWaveFile ( SOUND_RAKLION_GIANT_MAMUD_MOVE,	"Data\\Sound\\w57\\GiantMammoth_move.wav", 1 );
		LoadWaveFile ( SOUND_RAKLION_GIANT_MAMUD_ATTACK,			"Data\\Sound\\w57\\GiantMammoth_attack.wav", 1 );
		LoadWaveFile ( SOUND_RAKLION_GIANT_MAMUD_DEATH,			"Data\\Sound\\w57\\GiantMammoth_death.wav", 1 );
	case 147:
		LoadWaveFile ( SOUND_RAKLION_ICEGIANT_MOVE,			"Data\\Sound\\w58w59\\IceGiant_move.wav", 1 );
		LoadWaveFile ( SOUND_RAKLION_ICEGIANT_DEATH,		"Data\\Sound\\w58w59\\IceGiant_death.wav", 1 );
		break;
	case 148:
		LoadWaveFile( SOUND_MONSTER+34,						"Data\\Sound\\m헬스파이더죽기.wav" ,1);
		LoadWaveFile( SOUND_RAKLION_COOLERTIN_ATTACK,		"Data\\Sound\\w58w59\\Coolertin_attack.wav", 1 );
		LoadWaveFile( SOUND_RAKLION_COOLERTIN_MOVE,			"Data\\Sound\\w58w59\\Coolertin_move.wav", 1 );
		break;
	case 149:
		LoadWaveFile ( SOUND_RAKLION_IRON_KNIGHT_MOVE,			"Data\\Sound\\w58w59\\IronKnight_move.wav", 1 );
		LoadWaveFile ( SOUND_RAKLION_IRON_KNIGHT_ATTACK,		"Data\\Sound\\w58w59\\IronKnight_attack.wav", 1 );
		LoadWaveFile ( SOUND_MONSTER+154,						"Data\\Sound\\death1.wav", 1 );
		break;
	case 150:
		LoadWaveFile ( SOUND_RAKLION_SERUFAN_ATTACK1,		"Data\\Sound\\w58w59\\Selupan_attack1.wav", 1 );
		LoadWaveFile ( SOUND_RAKLION_SERUFAN_ATTACK2,		"Data\\Sound\\w58w59\\Selupan_attack2.wav", 1 );
		LoadWaveFile ( SOUND_RAKLION_SERUFAN_CURE,			"Data\\Sound\\w58w59\\Selupan_cure.wav", 1 );
		LoadWaveFile ( SOUND_RAKLION_SERUFAN_RAGE,			"Data\\Sound\\w58w59\\Selupan_rage.wav", 1 );
		LoadWaveFile ( SOUND_RAKLION_SERUFAN_WORD1,			"Data\\Sound\\w58w59\\Selupan_word1.wav", 1 );
		LoadWaveFile ( SOUND_RAKLION_SERUFAN_WORD2,			"Data\\Sound\\w58w59\\Selupan_word2.wav", 1 );
		LoadWaveFile ( SOUND_RAKLION_SERUFAN_WORD3,			"Data\\Sound\\w58w59\\Selupan_word3.wav", 1 );
		LoadWaveFile ( SOUND_RAKLION_SERUFAN_WORD4,			"Data\\Sound\\w58w59\\Selupan_word4.wav", 1 );
		break;
	case 155:
		LoadWaveFile(SOUND_XMAS_SANTA_IDLE_1, "Data\\Sound\\xmas\\DarkSanta_Idle01.wav");
		LoadWaveFile(SOUND_XMAS_SANTA_IDLE_2, "Data\\Sound\\xmas\\DarkSanta_Idle02.wav");
		LoadWaveFile(SOUND_XMAS_SANTA_WALK_1, "Data\\Sound\\xmas\\DarkSanta_Walk01.wav");
		LoadWaveFile(SOUND_XMAS_SANTA_WALK_2, "Data\\Sound\\xmas\\DarkSanta_Walk02.wav");
		LoadWaveFile(SOUND_XMAS_SANTA_ATTACK_1, "Data\\Sound\\xmas\\DarkSanta_Attack01.wav");
		LoadWaveFile(SOUND_XMAS_SANTA_DAMAGE_1, "Data\\Sound\\xmas\\DarkSanta_Damage01.wav");
		LoadWaveFile(SOUND_XMAS_SANTA_DAMAGE_2, "Data\\Sound\\xmas\\DarkSanta_Damage02.wav");
		LoadWaveFile(SOUND_XMAS_SANTA_DEATH_1, "Data\\Sound\\xmas\\DarkSanta_Death01.wav");
		break;
	case 156:
		LoadWaveFile(SOUND_MONSTER+72,"Data\\Sound\\mGoblin1.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+73,"Data\\Sound\\mGoblin2.wav"    ,Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+74,"Data\\Sound\\mGoblinAttack1.wav",Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+75,"Data\\Sound\\mGoblinAttack2.wav",Channel,Enable);
		LoadWaveFile(SOUND_MONSTER+76,"Data\\Sound\\mGoblinDie.wav" ,Channel,Enable);
		SetMonsterSound(MODEL_MONSTER01+Type,72,73,74,75,76);
		break;
	case 157:
		{
			LoadWaveFile(SOUND_PKFIELD_ZOMBIEWARRIOR_ATTACK,"Data\\Sound\\w64\\ZombieWarrior_attack.wav");
			LoadWaveFile(SOUND_PKFIELD_ZOMBIEWARRIOR_DAMAGE01,"Data\\Sound\\w64\\ZombieWarrior_damage01.wav");
			LoadWaveFile(SOUND_PKFIELD_ZOMBIEWARRIOR_DAMAGE02,"Data\\Sound\\w64\\ZombieWarrior_damage02.wav");
			LoadWaveFile(SOUND_PKFIELD_ZOMBIEWARRIOR_DEATH,"Data\\Sound\\w64\\ZombieWarrior_death.wav");
			LoadWaveFile(SOUND_PKFIELD_ZOMBIEWARRIOR_MOVE01,"Data\\Sound\\w64\\ZombieWarrior_move01.wav");
			LoadWaveFile(SOUND_PKFIELD_ZOMBIEWARRIOR_MOVE02,"Data\\Sound\\w64\\ZombieWarrior_move02.wav");
		}
		break;
	case 158:
		{
			LoadWaveFile(SOUND_PKFIELD_RAISEDGLADIATOR_ATTACK,"Data\\Sound\\w64\\RaisedGladiator_attack.wav");
			LoadWaveFile(SOUND_PKFIELD_RAISEDGLADIATOR_DAMAGE01,"Data\\Sound\\w64\\RaisedGladiator_damage01.wav");
			LoadWaveFile(SOUND_PKFIELD_RAISEDGLADIATOR_DAMAGE02,"Data\\Sound\\w64\\RaisedGladiator_damage02.wav");
			LoadWaveFile(SOUND_PKFIELD_RAISEDGLADIATOR_DEATH,"Data\\Sound\\w64\\RaisedGladiator_death.wav");
			LoadWaveFile(SOUND_PKFIELD_RAISEDGLADIATOR_MOVE01,"Data\\Sound\\w64\\RaisedGladiator_move01.wav");
			LoadWaveFile(SOUND_PKFIELD_RAISEDGLADIATOR_MOVE02,"Data\\Sound\\w64\\RaisedGladiator_move02.wav");	
		}
		break;
	case 159:
		{
			LoadWaveFile(SOUND_PKFIELD_ASHESBUTCHER_ATTACK,"Data\\Sound\\w64\\AshesButcher_attack.wav");
			LoadWaveFile(SOUND_PKFIELD_ASHESBUTCHER_DAMAGE01,"Data\\Sound\\w64\\AshesButcher_damage01.wav");
			LoadWaveFile(SOUND_PKFIELD_ASHESBUTCHER_DAMAGE02,"Data\\Sound\\w64\\AshesButcher_damage02.wav");
			LoadWaveFile(SOUND_PKFIELD_ASHESBUTCHER_DEATH,"Data\\Sound\\w64\\AshesButcher_death.wav");
			LoadWaveFile(SOUND_PKFIELD_ASHESBUTCHER_MOVE01,"Data\\Sound\\w64\\AshesButcher_move01.wav");
			LoadWaveFile(SOUND_PKFIELD_ASHESBUTCHER_MOVE02,"Data\\Sound\\w64\\AshesButcher_move02.wav");
		}
		break;
	case 160:
		{
			LoadWaveFile(SOUND_PKFIELD_BLOODASSASSIN_ATTACK,"Data\\Sound\\w64\\BloodAssassin_attack.wav");
			LoadWaveFile(SOUND_PKFIELD_BLOODASSASSIN_DAMAGE01,"Data\\Sound\\w64\\BloodAssassin_damage01.wav");
			LoadWaveFile(SOUND_PKFIELD_BLOODASSASSIN_DAMAGE02,"Data\\Sound\\w64\\BloodAssassin_damage02.wav");
			LoadWaveFile(SOUND_PKFIELD_BLOODASSASSIN_DEDTH,"Data\\Sound\\w64\\BloodAssassin_death.wav");
			LoadWaveFile(SOUND_PKFIELD_BLOODASSASSIN_MOVE01,"Data\\Sound\\w64\\BloodAssassin_move01.wav");
			LoadWaveFile(SOUND_PKFIELD_BLOODASSASSIN_MOVE02,"Data\\Sound\\w64\\BloodAssassin_move02.wav");
		}
		break;
	case 162:
		{
			LoadWaveFile(SOUND_PKFIELD_BURNINGLAVAGOLEM_ATTACK01,"Data\\Sound\\w64\\BurningLavaGolem_attack01.wav");
			LoadWaveFile(SOUND_PKFIELD_BURNINGLAVAGOLEM_ATTACK02,"Data\\Sound\\w64\\BurningLavaGolem_attack02.wav");
			LoadWaveFile(SOUND_PKFIELD_BURNINGLAVAGOLEM_DAMAGE01,"Data\\Sound\\w64\\BurningLavaGolem_damage01.wav");
			LoadWaveFile(SOUND_PKFIELD_BURNINGLAVAGOLEM_DAMAGE02,"Data\\Sound\\w64\\BurningLavaGolem_damage02.wav");
			LoadWaveFile(SOUND_PKFIELD_BURNINGLAVAGOLEM_DEATH,"Data\\Sound\\w64\\BurningLavaGolem_death.wav");
			LoadWaveFile(SOUND_PKFIELD_BURNINGLAVAGOLEM_MOVE01,"Data\\Sound\\w64\\BurningLavaGolem_move01.wav");
			LoadWaveFile(SOUND_PKFIELD_BURNINGLAVAGOLEM_MOVE02,"Data\\Sound\\w64\\BurningLavaGolem_move02.wav");
		}
		break;
	case 190:
		{
			LoadWaveFile(SOUND_DOPPELGANGER_RED_BUGBEAR_ATTACK,"Data\\Sound\\Doppelganger\\Angerbutcher_attack.wav");
			LoadWaveFile(SOUND_DOPPELGANGER_RED_BUGBEAR_DEATH,"Data\\Sound\\Doppelganger\\Angerbutcher_death.wav");
		}
		break;
	case 189:
		{
			LoadWaveFile(SOUND_DOPPELGANGER_BUGBEAR_ATTACK,"Data\\Sound\\Doppelganger\\Butcher_attack.wav");
			LoadWaveFile(SOUND_DOPPELGANGER_BUGBEAR_DEATH,"Data\\Sound\\Doppelganger\\Butcher_death.wav");
		}
		break;
	case 191:
		{
			LoadWaveFile(SOUND_DOPPELGANGER_SLIME_ATTACK,"Data\\Sound\\Doppelganger\\Doppelganger_attack.wav");
			LoadWaveFile(SOUND_DOPPELGANGER_SLIME_DEATH,"Data\\Sound\\Doppelganger\\Doppelganger_death.wav");
		}
		break;
#ifdef ASG_ADD_KARUTAN_MONSTERS
	case 209:
		LoadWaveFile(SOUND_KARUTAN_TCSCORPION_ATTACK, "Data\\Sound\\Karutan\\ToxyChainScorpion_attack.wav");
		LoadWaveFile(SOUND_KARUTAN_TCSCORPION_DEATH, "Data\\Sound\\Karutan\\ToxyChainScorpion_death.wav");
		LoadWaveFile(SOUND_KARUTAN_TCSCORPION_HIT, "Data\\Sound\\Karutan\\ToxyChainScorpion_hit.wav");
		break;
	case 210:
		LoadWaveFile(SOUND_KARUTAN_BONESCORPION_ATTACK, "Data\\Sound\\Karutan\\BoneScorpion_attack.wav");
		LoadWaveFile(SOUND_KARUTAN_BONESCORPION_DEATH, "Data\\Sound\\Karutan\\BoneScorpion_death.wav");
		LoadWaveFile(SOUND_KARUTAN_BONESCORPION_HIT, "Data\\Sound\\Karutan\\BoneScorpion_hit.wav");
		break;
	case 211:
		LoadWaveFile(SOUND_KARUTAN_ORCUS_MOVE1, "Data\\Sound\\Karutan\\Orcus_move1.wav");
		LoadWaveFile(SOUND_KARUTAN_ORCUS_MOVE2, "Data\\Sound\\Karutan\\Orcus_move2.wav");
		LoadWaveFile(SOUND_KARUTAN_ORCUS_ATTACK1, "Data\\Sound\\Karutan\\Orcus_attack_1.wav");
		LoadWaveFile(SOUND_KARUTAN_ORCUS_ATTACK2, "Data\\Sound\\Karutan\\Orcus_attack_2.wav");
		LoadWaveFile(SOUND_KARUTAN_ORCUS_DEATH, "Data\\Sound\\Karutan\\Orcus_death.wav");
		break;
	case 212:
		LoadWaveFile(SOUND_KARUTAN_GOLOCH_MOVE1, "Data\\Sound\\Karutan\\Goloch_move1.wav");
		LoadWaveFile(SOUND_KARUTAN_GOLOCH_MOVE2, "Data\\Sound\\Karutan\\Goloch_move2.wav");
		LoadWaveFile(SOUND_KARUTAN_GOLOCH_ATTACK, "Data\\Sound\\Karutan\\Goloch_attack.wav");
		LoadWaveFile(SOUND_KARUTAN_GOLOCH_DEATH, "Data\\Sound\\Karutan\\Goloch_death.wav");
		break;
	case 213:
		LoadWaveFile(SOUND_KARUTAN_CRYPTA_MOVE1, "Data\\Sound\\Karutan\\Crypta_move1.wav");
		LoadWaveFile(SOUND_KARUTAN_CRYPTA_MOVE2, "Data\\Sound\\Karutan\\Crypta_move2.wav");
		LoadWaveFile(SOUND_KARUTAN_CRYPTA_ATTACK, "Data\\Sound\\Karutan\\Crypta_attack.wav");
		LoadWaveFile(SOUND_KARUTAN_CRYPTA_DEATH, "Data\\Sound\\Karutan\\Crypta_death.wav");
		break;
	case 214:
		LoadWaveFile(SOUND_KARUTAN_CRYPOS_MOVE1, "Data\\Sound\\Karutan\\Crypos_move1.wav");
		LoadWaveFile(SOUND_KARUTAN_CRYPOS_MOVE2, "Data\\Sound\\Karutan\\Crypos_move2.wav");
		LoadWaveFile(SOUND_KARUTAN_CRYPOS_ATTACK1, "Data\\Sound\\Karutan\\Crypos_attack_1.wav");
		LoadWaveFile(SOUND_KARUTAN_CRYPOS_ATTACK2, "Data\\Sound\\Karutan\\Crypos_attack_2.wav");
		break;
	case 215:
		LoadWaveFile(SOUND_KARUTAN_CONDRA_MOVE1, "Data\\Sound\\Karutan\\Condra_move1.wav");
		LoadWaveFile(SOUND_KARUTAN_CONDRA_MOVE2, "Data\\Sound\\Karutan\\Condra_move2.wav");
		LoadWaveFile(SOUND_KARUTAN_CONDRA_ATTACK, "Data\\Sound\\Karutan\\Condra_attack.wav");
		LoadWaveFile(SOUND_KARUTAN_CONDRA_DEATH, "Data\\Sound\\Karutan\\Condra_death.wav");
		break;
	case 216:
		LoadWaveFile(SOUND_KARUTAN_CONDRA_MOVE1, "Data\\Sound\\Karutan\\Condra_move1.wav");
		LoadWaveFile(SOUND_KARUTAN_CONDRA_MOVE2, "Data\\Sound\\Karutan\\Condra_move2.wav");
		LoadWaveFile(SOUND_KARUTAN_NARCONDRA_ATTACK, "Data\\Sound\\Karutan\\NarCondra_attack.wav");
		LoadWaveFile(SOUND_KARUTAN_CONDRA_DEATH, "Data\\Sound\\Karutan\\Condra_death.wav");
		break;
#endif	// ASG_ADD_KARUTAN_MONSTERS
	}
}

void OpenMonsterModels()
{

}

void OpenSkills()
{
	gLoadData.AccessModel(MODEL_ICE,"Data\\Skill\\","Ice",1);
	gLoadData.AccessModel(MODEL_ICE_SMALL,"Data\\Skill\\","Ice",2);
	gLoadData.AccessModel(MODEL_FIRE,"Data\\Skill\\","Fire",1);
	gLoadData.AccessModel(MODEL_POISON,"Data\\Skill\\","Poison",1);
	for(int i = 0; i < 2; i++)
		gLoadData.AccessModel(MODEL_STONE1 + i,"Data\\Skill\\","Stone",i + 1);
	gLoadData.AccessModel(MODEL_CIRCLE,"Data\\Skill\\","Circle",1);
	gLoadData.AccessModel(MODEL_CIRCLE_LIGHT,"Data\\Skill\\","Circle",2);
	gLoadData.AccessModel(MODEL_MAGIC1,"Data\\Skill\\","Magic",1);
	gLoadData.AccessModel(MODEL_MAGIC2,"Data\\Skill\\","Magic",2);
	gLoadData.AccessModel(MODEL_STORM,"Data\\Skill\\","Storm",1);
	gLoadData.AccessModel(MODEL_LASER,"Data\\Skill\\","Laser",1);

	for(int i = 0; i < 3; i++)
		gLoadData.AccessModel(MODEL_SKELETON1 + i,"Data\\Skill\\","Skeleton",i + 1);

	gLoadData.AccessModel(MODEL_SKELETON_PCBANG,"Data\\Skill\\","Skeleton",3);
	gLoadData.AccessModel(MODEL_HALLOWEEN, "Data\\Skill\\", "Jack");

	gLoadData.AccessModel(MODEL_HALLOWEEN_CANDY_BLUE,"Data\\Skill\\","hcandyblue");
	gLoadData.AccessModel(MODEL_HALLOWEEN_CANDY_ORANGE,"Data\\Skill\\","hcandyorange");
	gLoadData.AccessModel(MODEL_HALLOWEEN_CANDY_RED,"Data\\Skill\\","hcandyred");
	gLoadData.AccessModel(MODEL_HALLOWEEN_CANDY_YELLOW,"Data\\Skill\\","hcandyyellow");
	gLoadData.AccessModel(MODEL_HALLOWEEN_CANDY_HOBAK,"Data\\Skill\\","hhobak");
	gLoadData.AccessModel(MODEL_HALLOWEEN_CANDY_STAR,"Data\\Skill\\","hstar");
	LoadBitmap("Skill\\jack04.jpg", BITMAP_JACK_1);
	LoadBitmap("Skill\\jack05.jpg", BITMAP_JACK_2);
	LoadBitmap("Monster\\iui02.tga",BITMAP_ROBE+3);
	gLoadData.AccessModel(MODEL_POTION+45, "Data\\Item\\", "hobakhead");
	gLoadData.OpenTexture(MODEL_POTION+45, "Item\\");

	gLoadData.AccessModel(MODEL_CURSEDTEMPLE_ALLIED_PLAYER, "Data\\Skill\\", "unitedsoldier");
	gLoadData.AccessModel(MODEL_CURSEDTEMPLE_ILLUSION_PLAYER, "Data\\Skill\\", "illusionist");

	gLoadData.AccessModel(MODEL_WOOSISTONE, "Data\\Skill\\", "woositone");
	gLoadData.OpenTexture(MODEL_WOOSISTONE, "Skill\\");

	g_NewYearsDayEvent->LoadModel();

	gLoadData.AccessModel(MODEL_SAW      ,"Data\\Skill\\","Saw",1);

	for(int i=0;i<2;i++)
      	gLoadData.AccessModel(MODEL_BONE1+i,"Data\\Skill\\","Bone",i+1);

	for(int i=0;i<3;i++)
      	gLoadData.AccessModel(MODEL_SNOW1+i,"Data\\Skill\\","Snow",i+1);

	gLoadData.AccessModel(MODEL_UNICON       ,"Data\\Skill\\","Rider",1);
    gLoadData.AccessModel ( MODEL_PEGASUS, "Data\\Skill\\", "Rider", 2 );
    gLoadData.AccessModel ( MODEL_DARK_HORSE , "Data\\Skill\\", "DarkHorse" );
	
	gLoadData.AccessModel(MODEL_FENRIR_BLACK, "Data\\Skill\\", "fenril_black");
	gLoadData.OpenTexture(MODEL_FENRIR_BLACK, "Skill\\");

	gLoadData.AccessModel(MODEL_FENRIR_RED, "Data\\Skill\\", "fenril_red");
	gLoadData.OpenTexture(MODEL_FENRIR_RED, "Skill\\");

	gLoadData.AccessModel(MODEL_FENRIR_BLUE, "Data\\Skill\\", "fenril_blue");
	gLoadData.OpenTexture(MODEL_FENRIR_BLUE, "Skill\\");

	gLoadData.AccessModel(MODEL_FENRIR_GOLD, "Data\\Skill\\", "fenril_gold");
	gLoadData.OpenTexture(MODEL_FENRIR_BLUE, "Skill\\");

	gLoadData.AccessModel(MODEL_PANDA,			"Data\\Item\\", "panda");
	gLoadData.OpenTexture(MODEL_PANDA,			"Item\\" );

	gLoadData.AccessModel(MODEL_SKELETON_CHANGED,			"Data\\Item\\", "trans_skeleton");
	gLoadData.OpenTexture(MODEL_SKELETON_CHANGED,			"Item\\" );


    gLoadData.AccessModel ( MODEL_DARK_SPIRIT, "Data\\Skill\\", "DarkSpirit" );
	LoadBitmap( "Skill\\dkthreebody_r.jpg" , BITMAP_MONSTER_SKIN+2, GL_LINEAR, GL_REPEAT );

    gLoadData.AccessModel ( MODEL_WARCRAFT,   "Data\\Skill\\", "HellGate" );
    Models[MODEL_WARCRAFT].Actions[0].LockPositions = false;
	SAFE_DELETE_ARRAY(Models[MODEL_WARCRAFT].Actions[0].Positions);
    Models[MODEL_WARCRAFT].Actions[0].PlaySpeed = 0.15f;

	gLoadData.AccessModel(MODEL_ARROW        ,"Data\\Skill\\","Arrow",1);
	gLoadData.AccessModel(MODEL_ARROW_STEEL  ,"Data\\Skill\\","ArrowSteel",1);
	gLoadData.AccessModel(MODEL_ARROW_THUNDER,"Data\\Skill\\","ArrowThunder",1);
	gLoadData.AccessModel(MODEL_ARROW_LASER  ,"Data\\Skill\\","ArrowLaser",1);
	gLoadData.AccessModel(MODEL_ARROW_V      ,"Data\\Skill\\","ArrowV",1);
	gLoadData.AccessModel(MODEL_ARROW_SAW    ,"Data\\Skill\\","ArrowSaw",1);
	gLoadData.AccessModel(MODEL_ARROW_NATURE ,"Data\\Skill\\","ArrowNature",1);

	gLoadData.OpenTexture(MODEL_MAGIC_CAPSULE2	,"Skill\\");
	gLoadData.AccessModel(MODEL_MAGIC_CAPSULE2    ,"Data\\Skill\\","Protect",2);

	gLoadData.AccessModel(MODEL_ARROW_SPARK ,"Data\\Skill\\","Arrow_Spark");
	gLoadData.OpenTexture(MODEL_ARROW_SPARK ,"Skill\\");

	gLoadData.AccessModel(MODEL_DARK_SCREAM ,"Data\\Skill\\","darkfirescrem02");
	gLoadData.OpenTexture(MODEL_DARK_SCREAM ,"Skill\\");
	gLoadData.AccessModel(MODEL_DARK_SCREAM_FIRE ,"Data\\Skill\\","darkfirescrem01");
	gLoadData.OpenTexture(MODEL_DARK_SCREAM_FIRE ,"Skill\\");
	gLoadData.AccessModel(MODEL_SUMMON		,"Data\\SKill\\","nightmaresum");
	gLoadData.OpenTexture(MODEL_SUMMON		,"SKill\\");
	gLoadData.AccessModel(MODEL_MULTI_SHOT1 ,"Data\\Effect\\","multishot01");
	gLoadData.OpenTexture(MODEL_MULTI_SHOT1 ,"Effect\\");
	gLoadData.AccessModel(MODEL_MULTI_SHOT2 ,"Data\\Effect\\","multishot02");
	gLoadData.OpenTexture(MODEL_MULTI_SHOT2 ,"Effect\\");
	gLoadData.AccessModel(MODEL_MULTI_SHOT3 ,"Data\\Effect\\","multishot03");
	gLoadData.OpenTexture(MODEL_MULTI_SHOT3 ,"Effect\\");
	gLoadData.AccessModel(MODEL_DESAIR		,"Data\\SKill\\","desair");
	gLoadData.OpenTexture(MODEL_DESAIR		,"SKill\\");
	gLoadData.AccessModel(MODEL_ARROW_RING ,"Data\\Skill\\","CW_Bow_Skill");
	gLoadData.OpenTexture(MODEL_ARROW_RING ,"Skill\\");
	
	gLoadData.AccessModel(MODEL_ARROW_DARKSTINGER, "Data\\Skill\\", "sketbows_arrows");
	gLoadData.OpenTexture(MODEL_ARROW_DARKSTINGER, "Skill\\");
	gLoadData.AccessModel(MODEL_FEATHER, "Data\\Skill\\", "darkwing_hetachi");
	gLoadData.OpenTexture(MODEL_FEATHER, "Skill\\");
	gLoadData.AccessModel(MODEL_FEATHER_FOREIGN, "Data\\Skill\\", "darkwing_hetachi");
	gLoadData.OpenTexture(MODEL_FEATHER_FOREIGN, "Skill\\");
	LoadBitmap("Effect\\Bugbear_R.jpg", BITMAP_BUGBEAR_R, GL_LINEAR, GL_REPEAT);
	LoadBitmap("Effect\\pk_mon02_fire.jpg", BITMAP_PKMON01, GL_LINEAR, GL_REPEAT);
	LoadBitmap("Effect\\pk_mon03_red.jpg" , BITMAP_PKMON02, GL_LINEAR, GL_REPEAT);
	LoadBitmap("Effect\\pk_mon02_green.jpg", BITMAP_PKMON03, GL_LINEAR, GL_REPEAT);
	LoadBitmap("Effect\\pk_mon03_green.jpg", BITMAP_PKMON04, GL_LINEAR, GL_REPEAT);
	LoadBitmap("Effect\\lavagiantAa_e.jpg", BITMAP_PKMON05, GL_LINEAR, GL_REPEAT);
	LoadBitmap("Effect\\lavagiantBa_e.jpg", BITMAP_PKMON06, GL_LINEAR, GL_REPEAT);
	LoadBitmap("Effect\\eff_magma_red.jpg", BITMAP_LAVAGIANT_FOOTPRINT_R, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\eff_magma_violet.jpg", BITMAP_LAVAGIANT_FOOTPRINT_V, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\raymond_sword_R.jpg", BITMAP_RAYMOND_SWORD, GL_LINEAR, GL_REPEAT);
	LoadBitmap("Effect\\mist01.jpg", BITMAP_AG_ADDITION_EFFECT, GL_LINEAR, GL_REPEAT);
	gLoadData.AccessModel(MODEL_ARROW_GAMBLE, "Data\\Skill\\", "gamble_arrows01");
	gLoadData.OpenTexture(MODEL_ARROW_GAMBLE, "Skill\\");

	gLoadData.OpenTexture(MODEL_SPEARSKILL		,"Skill\\");
	gLoadData.AccessModel(MODEL_SPEARSKILL      ,"Data\\Skill\\","RidingSpear",1);
	gLoadData.AccessModel(MODEL_PROTECT      ,"Data\\Skill\\","Protect",1);

	for(int i=0;i<2;i++)
      	gLoadData.AccessModel(MODEL_BIG_STONE1+i,"Data\\Skill\\","BigStone",i+1);

	gLoadData.AccessModel(MODEL_MAGIC_CIRCLE1,"Data\\Skill\\","MagicCircle",1);
	gLoadData.AccessModel(MODEL_ARROW_WING   ,"Data\\Skill\\","ArrowWing",1);
	gLoadData.AccessModel(MODEL_ARROW_BOMB   ,"Data\\Skill\\","ArrowBomb",1);
	gLoadData.AccessModel(MODEL_BALL         ,"Data\\Skill\\","Ball",1);//공
	Models[MODEL_BALL].Actions[0].PlaySpeed = 0.5f;
	gLoadData.AccessModel(MODEL_SKILL_BLAST  ,"Data\\Skill\\","Blast",1);
	gLoadData.AccessModel(MODEL_SKILL_INFERNO,"Data\\Skill\\","Inferno",1);
	gLoadData.AccessModel(MODEL_ARROW_DOUBLE ,"Data\\Skill\\","ArrowDouble",1);

	gLoadData.AccessModel(MODEL_ARROW_BEST_CROSSBOW, "Data\\Skill\\", "KCross" );
	gLoadData.AccessModel(MODEL_ARROW_DRILL, "Data\\Skill\\", "Carow" );
	gLoadData.AccessModel(MODEL_COMBO		 , "Data\\Skill\\", "combo" );

    gLoadData.AccessModel ( MODEL_GATE+0, "Data\\Object12\\", "Gate", 1 );
    gLoadData.AccessModel ( MODEL_GATE+1, "Data\\Object12\\", "Gate", 2 );
    gLoadData.AccessModel ( MODEL_STONE_COFFIN+0, "Data\\Object12\\", "StoneCoffin", 1 );
    gLoadData.AccessModel ( MODEL_STONE_COFFIN+1, "Data\\Object12\\", "StoneCoffin", 2 );

    for ( int i=0; i<2; ++i )
    {
        gLoadData.OpenTexture ( MODEL_GATE+1, "Monster\\" );
        gLoadData.OpenTexture ( MODEL_STONE_COFFIN+i, "Monster\\" );
    }

	gLoadData.AccessModel ( MODEL_AIR_FORCE, "Data\\Skill\\", "AirForce" );
	gLoadData.AccessModel ( MODEL_WAVES    , "Data\\Skill\\", "m_Waves" );
	gLoadData.AccessModel ( MODEL_PIERCING2, "Data\\Skill\\", "m_Piercing" );
	gLoadData.AccessModel ( MODEL_PIER_PART, "Data\\Skill\\", "PierPart" );
	gLoadData.AccessModel ( MODEL_DARKLORD_SKILL, "Data\\Skill\\", "DarkLordSkill" );
	gLoadData.AccessModel ( MODEL_GROUND_STONE,   "Data\\Skill\\", "groundStone" );
    Models[MODEL_GROUND_STONE].Actions[0].Loop = false;
    gLoadData.AccessModel ( MODEL_GROUND_STONE2,  "Data\\Skill\\", "groundStone2" );
    Models[MODEL_GROUND_STONE2].Actions[0].Loop = false;
	gLoadData.AccessModel ( MODEL_WATER_WAVE,     "Data\\Skill\\", "seamanFX" );
	gLoadData.AccessModel ( MODEL_SKULL,          "Data\\Skill\\", "Skull" );
	gLoadData.AccessModel ( MODEL_LACEARROW,		"Data\\Skill\\", "LaceArrow");
	gLoadData.OpenTexture ( MODEL_LACEARROW,      "Item\\", GL_CLAMP_TO_EDGE );

    gLoadData.AccessModel ( MODEL_MANY_FLAG, "Data\\Skill\\", "ManyFlag" );
    gLoadData.AccessModel ( MODEL_WEBZEN_MARK, "Data\\Skill\\", "MuSign" );
	gLoadData.AccessModel ( MODEL_STUN_STONE,  "Data\\Skill\\", "GroundCrystal" );
	gLoadData.AccessModel ( MODEL_SKIN_SHELL,  "Data\\Skill\\", "skinshell" );
	gLoadData.AccessModel ( MODEL_MANA_RUNE,   "Data\\Skill\\", "ManaRune" );
	gLoadData.AccessModel ( MODEL_SKILL_JAVELIN,  "Data\\Skill\\", "Javelin" );
	gLoadData.AccessModel ( MODEL_ARROW_IMPACT,   "Data\\Skill\\", "ArrowImpact" );
	gLoadData.AccessModel ( MODEL_SWORD_FORCE,    "Data\\Skill\\", "SwordForce" );

    gLoadData.AccessModel ( MODEL_FLY_BIG_STONE1,  "Data\\Skill\\", "FlyBigStone1" );
    gLoadData.AccessModel ( MODEL_FLY_BIG_STONE2,  "Data\\Skill\\", "FlyBigStone2" );
    gLoadData.AccessModel ( MODEL_BIG_STONE_PART1, "Data\\Skill\\", "FlySmallStone1" );
    gLoadData.AccessModel ( MODEL_BIG_STONE_PART2, "Data\\Skill\\", "FlySmallStone2" );
    gLoadData.AccessModel ( MODEL_WALL_PART1,      "Data\\Skill\\", "WallStone1" );
    gLoadData.AccessModel ( MODEL_WALL_PART2,      "Data\\Skill\\", "WallStone2" );
    gLoadData.AccessModel ( MODEL_GATE_PART1,      "Data\\Skill\\", "GatePart1" );
    gLoadData.AccessModel ( MODEL_GATE_PART2,      "Data\\Skill\\", "GatePart2" );
    gLoadData.AccessModel ( MODEL_GATE_PART3,      "Data\\Skill\\", "GatePart3" );
    gLoadData.AccessModel ( MODEL_AURORA,          "Data\\Skill\\", "Aurora" );
    gLoadData.AccessModel ( MODEL_TOWER_GATE_PLANE,"Data\\Skill\\", "TowerGatePlane" );
	gLoadData.AccessModel ( MODEL_GOLEM_STONE,	"Data\\Skill\\", "golem_stone" );
	gLoadData.AccessModel ( MODEL_FISSURE,		"Data\\Skill\\", "bossrock" );
	gLoadData.AccessModel ( MODEL_FISSURE_LIGHT,		"Data\\Skill\\", "bossrocklight" );
    gLoadData.AccessModel ( MODEL_PROTECTGUILD, "Data\\Skill\\", "ProtectGuild" );
	gLoadData.AccessModel(MODEL_DARK_ELF_SKILL   ,"Data\\Skill\\","elf_skill");
	gLoadData.AccessModel(MODEL_BALGAS_SKILL   ,"Data\\Skill\\","WaveForce");
	gLoadData.AccessModel(MODEL_DEATH_SPI_SKILL   ,"Data\\Skill\\","deathsp_eff");
	Models[MODEL_BALGAS_SKILL].Actions[0].Loop = false;

	gLoadData.OpenTexture(MODEL_DARK_ELF_SKILL,"Skill\\");
	gLoadData.OpenTexture(MODEL_BALGAS_SKILL,"Skill\\");
	gLoadData.OpenTexture(MODEL_DEATH_SPI_SKILL,"Skill\\");

    gLoadData.OpenTexture ( MODEL_DARK_HORSE, "Skill\\" );
    gLoadData.OpenTexture ( MODEL_DARK_SPIRIT, "Skill\\" );

    gLoadData.OpenTexture ( MODEL_WARCRAFT, "Skill\\" );
    gLoadData.OpenTexture ( MODEL_PEGASUS, "Skill\\" );
	gLoadData.OpenTexture (MODEL_SKILL_FURY_STRIKE+1,"Skill\\");
	gLoadData.OpenTexture (MODEL_SKILL_FURY_STRIKE+2,"Skill\\");
	gLoadData.OpenTexture (MODEL_SKILL_FURY_STRIKE+3,"Skill\\");
	gLoadData.OpenTexture (MODEL_SKILL_FURY_STRIKE+5,"Skill\\");
	gLoadData.OpenTexture (MODEL_SKILL_FURY_STRIKE+7,"Skill\\");
	gLoadData.OpenTexture (MODEL_SKILL_FURY_STRIKE+8,"Skill\\");
    gLoadData.OpenTexture (MODEL_WAVE, "Skill\\");
    gLoadData.OpenTexture (MODEL_TAIL, "Skill\\");

	gLoadData.OpenTexture (MODEL_WAVE_FORCE, "Skill\\" );
	gLoadData.OpenTexture (MODEL_BLIZZARD, "Skill\\" );

	g_XmasEvent->LoadXmasEvent();

	gLoadData.OpenTexture(MODEL_ARROW_BEST_CROSSBOW,"Skill\\");
	gLoadData.OpenTexture(MODEL_ARROW_DRILL,"Skill\\");
	gLoadData.OpenTexture(MODEL_COMBO,"Skill\\");

	gLoadData.OpenTexture(MODEL_AIR_FORCE,"Skill\\",GL_CLAMP_TO_EDGE);
	gLoadData.OpenTexture(MODEL_WAVES,"Skill\\");
	gLoadData.OpenTexture(MODEL_PIERCING2,"Skill\\");
	gLoadData.OpenTexture(MODEL_PIER_PART,"Skill\\");
	gLoadData.OpenTexture(MODEL_GROUND_STONE,"Skill\\");
	gLoadData.OpenTexture(MODEL_GROUND_STONE2,"Skill\\");
	gLoadData.OpenTexture(MODEL_WATER_WAVE,"Skill\\",GL_CLAMP_TO_EDGE);
	gLoadData.OpenTexture(MODEL_SKULL,"Skill\\",GL_CLAMP_TO_EDGE);
	gLoadData.OpenTexture(MODEL_MANY_FLAG,"Skill\\");
	gLoadData.OpenTexture(MODEL_WEBZEN_MARK,"Skill\\");

	gLoadData.OpenTexture(MODEL_FLY_BIG_STONE1,"Npc\\");
	gLoadData.OpenTexture(MODEL_FLY_BIG_STONE2,"Skill\\");
	gLoadData.OpenTexture(MODEL_BIG_STONE_PART1,"Skill\\");
	gLoadData.OpenTexture(MODEL_BIG_STONE_PART2,"Skill\\");
	gLoadData.OpenTexture(MODEL_WALL_PART1,"Object31\\");
	gLoadData.OpenTexture(MODEL_WALL_PART2,"Object31\\");
	gLoadData.OpenTexture(MODEL_GATE_PART1,"Monster\\");
	gLoadData.OpenTexture(MODEL_GATE_PART2,"Monster\\");
	gLoadData.OpenTexture(MODEL_GATE_PART3,"Monster\\");
	gLoadData.OpenTexture(MODEL_AURORA,"Monster\\");
	gLoadData.OpenTexture(MODEL_TOWER_GATE_PLANE,"Skill\\");
	gLoadData.OpenTexture(MODEL_GOLEM_STONE,"Monster\\");
	gLoadData.OpenTexture(MODEL_FISSURE,"Skill\\");
	gLoadData.OpenTexture(MODEL_FISSURE_LIGHT,"Skill\\");
	gLoadData.OpenTexture(MODEL_SKIN_SHELL,"Effect\\");
	gLoadData.OpenTexture(MODEL_PROTECTGUILD,"Item\\");

	gLoadData.AccessModel(MODEL_SKILL_FURY_STRIKE+1,"Data\\Skill\\","EarthQuake",1);
	gLoadData.AccessModel(MODEL_SKILL_FURY_STRIKE+2,"Data\\Skill\\","EarthQuake",2);
	gLoadData.AccessModel(MODEL_SKILL_FURY_STRIKE+3,"Data\\Skill\\","EarthQuake",3);
	gLoadData.AccessModel(MODEL_SKILL_FURY_STRIKE+5,"Data\\Skill\\","EarthQuake",5);
	gLoadData.AccessModel(MODEL_SKILL_FURY_STRIKE+7,"Data\\Skill\\","EarthQuake",7);
	gLoadData.AccessModel(MODEL_SKILL_FURY_STRIKE+8,"Data\\Skill\\","EarthQuake",8);
	gLoadData.AccessModel(MODEL_WAVE,"Data\\Skill\\","flashing");
	gLoadData.AccessModel(MODEL_TAIL,"Data\\Skill\\","tail");

	gLoadData.AccessModel(MODEL_SKILL_FURY_STRIKE+4,"Data\\Skill\\","EarthQuake",4);
	gLoadData.AccessModel(MODEL_SKILL_FURY_STRIKE+6,"Data\\Skill\\","EarthQuake",6);
    gLoadData.AccessModel(MODEL_PIERCING,"Data\\Skill\\","Piercing");

	gLoadData.AccessModel(MODEL_WAVE_FORCE,"Data\\Skill\\","WaveForce");
	gLoadData.AccessModel(MODEL_BLIZZARD,"Data\\Skill\\","Blizzard");

	gLoadData.AccessModel(MODEL_ARROW_AUTOLOAD, "Data\\Skill\\", "arrowsrefill");
	gLoadData.OpenTexture(MODEL_ARROW_AUTOLOAD, "Effect\\");

	gLoadData.AccessModel(MODEL_INFINITY_ARROW, "Data\\Skill\\", "arrowsre", 1);
	gLoadData.OpenTexture(MODEL_INFINITY_ARROW, "Skill\\");
	gLoadData.AccessModel(MODEL_INFINITY_ARROW1, "Data\\Skill\\", "arrowsre", 2);
	gLoadData.OpenTexture(MODEL_INFINITY_ARROW1, "Skill\\");
	gLoadData.AccessModel(MODEL_INFINITY_ARROW2, "Data\\Skill\\", "arrowsre", 3);
	gLoadData.OpenTexture(MODEL_INFINITY_ARROW2, "Skill\\");
	gLoadData.AccessModel(MODEL_INFINITY_ARROW3, "Data\\Skill\\", "arrowsre", 4);
	gLoadData.OpenTexture(MODEL_INFINITY_ARROW3, "Effect\\");
	gLoadData.AccessModel(MODEL_INFINITY_ARROW4, "Data\\Skill\\", "arrowsre", 5);
	gLoadData.OpenTexture(MODEL_INFINITY_ARROW4, "Skill\\");

	gLoadData.AccessModel(MODEL_SHIELD_CRASH, "Data\\Effect\\", "atshild");
	gLoadData.OpenTexture(MODEL_SHIELD_CRASH, "Effect\\");

	gLoadData.AccessModel(MODEL_SHIELD_CRASH2, "Data\\Effect\\", "atshild2");
	gLoadData.OpenTexture(MODEL_SHIELD_CRASH2, "Effect\\");

	gLoadData.AccessModel(MODEL_IRON_RIDER_ARROW, "Data\\Effect\\", "ironobj");
	gLoadData.OpenTexture(MODEL_IRON_RIDER_ARROW, "Effect\\");

	gLoadData.AccessModel(MODEL_KENTAUROS_ARROW, "Data\\Effect\\", "cantasarrow");
	gLoadData.OpenTexture(MODEL_KENTAUROS_ARROW, "Effect\\");

	gLoadData.AccessModel(MODEL_BLADE_SKILL, "Data\\Effect\\", "bladetonedo");
	gLoadData.OpenTexture(MODEL_BLADE_SKILL, "Effect\\");
	
	gLoadData.AccessModel(MODEL_CHANGE_UP_EFF   ,"Data\\Effect\\","Change_Up_Eff");
	gLoadData.OpenTexture(MODEL_CHANGE_UP_EFF, "Effect\\");
	Models[MODEL_CHANGE_UP_EFF].Actions[0].PlaySpeed = 0.005f;
	gLoadData.AccessModel(MODEL_CHANGE_UP_NASA   ,"Data\\Effect\\","changup_nasa");

	gLoadData.OpenTexture(MODEL_CHANGE_UP_NASA, "Effect\\");
	gLoadData.AccessModel(MODEL_CHANGE_UP_CYLINDER   ,"Data\\Effect\\","clinderlight");
	gLoadData.OpenTexture(MODEL_CHANGE_UP_CYLINDER, "Effect\\");

	gLoadData.AccessModel(MODEL_CURSEDTEMPLE_HOLYITEM, "Data\\Skill\\", "eventsungmul");
	gLoadData.OpenTexture(MODEL_CURSEDTEMPLE_HOLYITEM, "Skill\\");

	gLoadData.AccessModel(MODEL_CURSEDTEMPLE_PRODECTION_SKILL, "Data\\Skill\\", "eventshild");
	gLoadData.OpenTexture(MODEL_CURSEDTEMPLE_PRODECTION_SKILL, "Skill\\");

	gLoadData.AccessModel(MODEL_CURSEDTEMPLE_RESTRAINT_SKILL, "Data\\Skill\\", "eventroofe");
	gLoadData.OpenTexture(MODEL_CURSEDTEMPLE_RESTRAINT_SKILL, "Skill\\");
		
	gLoadData.AccessModel ( MODEL_CURSEDTEMPLE_STATUE_PART1, "Data\\Npc\\", "songck1" );
	gLoadData.OpenTexture ( MODEL_CURSEDTEMPLE_STATUE_PART1, "Npc\\" );

	gLoadData.AccessModel ( MODEL_CURSEDTEMPLE_STATUE_PART2, "Data\\Npc\\", "songck2" );
	gLoadData.OpenTexture ( MODEL_CURSEDTEMPLE_STATUE_PART2, "Npc\\" );
	
	gLoadData.AccessModel(MODEL_FENRIR_THUNDER, "Data\\Effect\\", "lightning_type01");
	gLoadData.OpenTexture(MODEL_FENRIR_THUNDER, "Effect\\");

	for(int i=MODEL_SKILL_BEGIN;i<MODEL_SKILL_END;i++)
	{
        if( i==MODEL_PIERCING )
        {
            gLoadData.OpenTexture(i,"Skill\\");
        }
        else
            gLoadData.OpenTexture(i,"Skill\\");
	}

	LoadBitmap("Skill\\flower1.tga",BITMAP_FLOWER01);
	LoadBitmap("Skill\\flower2.tga",BITMAP_FLOWER01+1);
	LoadBitmap("Skill\\flower3.tga",BITMAP_FLOWER01+2);
	
	gLoadData.AccessModel(MODEL_MOONHARVEST_GAM, "Data\\Effect\\", "chusukgam" );
	gLoadData.OpenTexture(MODEL_MOONHARVEST_GAM, "Effect\\");
	gLoadData.AccessModel(MODEL_MOONHARVEST_SONGPUEN1, "Data\\Effect\\", "chusukseung1" );
	gLoadData.OpenTexture(MODEL_MOONHARVEST_SONGPUEN1, "Effect\\");
	gLoadData.AccessModel(MODEL_MOONHARVEST_SONGPUEN2, "Data\\Effect\\", "chusukseung2" );
	gLoadData.OpenTexture(MODEL_MOONHARVEST_SONGPUEN2, "Effect\\");
	gLoadData.AccessModel(MODEL_MOONHARVEST_MOON, "Data\\Effect\\", "chysukmoon" );
	gLoadData.OpenTexture(MODEL_MOONHARVEST_MOON, "Effect\\");
	
	gLoadData.AccessModel(MODEL_ALICE_BUFFSKILL_EFFECT,"Data\\Effect\\","elshildring");
	gLoadData.OpenTexture(MODEL_ALICE_BUFFSKILL_EFFECT, "Effect\\");
	gLoadData.AccessModel(MODEL_ALICE_BUFFSKILL_EFFECT2, "Data\\Effect\\", "elshildring2" );
	gLoadData.OpenTexture(MODEL_ALICE_BUFFSKILL_EFFECT2, "Effect\\");

	gLoadData.AccessModel(MODEL_SUMMONER_WRISTRING_EFFECT, "Data\\Effect\\", "ringtyperout" );
	gLoadData.OpenTexture(MODEL_SUMMONER_WRISTRING_EFFECT, "Effect\\");

	gLoadData.AccessModel(MODEL_SUMMONER_EQUIP_HEAD_SAHAMUTT, "Data\\Skill\\", "sahatail" );
	gLoadData.OpenTexture(MODEL_SUMMONER_EQUIP_HEAD_SAHAMUTT, "Skill\\");
	gLoadData.AccessModel(MODEL_SUMMONER_EQUIP_HEAD_NEIL, "Data\\Skill\\", "nillsohwanz" );
	gLoadData.OpenTexture(MODEL_SUMMONER_EQUIP_HEAD_NEIL, "Skill\\");

	gLoadData.AccessModel(MODEL_SUMMONER_EQUIP_HEAD_LAGUL, "Data\\Skill\\", "lagul_head" );
	gLoadData.OpenTexture(MODEL_SUMMONER_EQUIP_HEAD_LAGUL, "Skill\\");

	gLoadData.AccessModel(MODEL_SUMMONER_CASTING_EFFECT1, "Data\\Effect\\", "Suhwanzin1" );
	gLoadData.OpenTexture(MODEL_SUMMONER_CASTING_EFFECT1, "Effect\\");
	gLoadData.AccessModel(MODEL_SUMMONER_CASTING_EFFECT11, "Data\\Effect\\", "Suhwanzin11" );
	gLoadData.OpenTexture(MODEL_SUMMONER_CASTING_EFFECT11, "Effect\\");
	gLoadData.AccessModel(MODEL_SUMMONER_CASTING_EFFECT111, "Data\\Effect\\", "Suhwanzin111" );
	gLoadData.OpenTexture(MODEL_SUMMONER_CASTING_EFFECT111, "Effect\\");
	gLoadData.AccessModel(MODEL_SUMMONER_CASTING_EFFECT2, "Data\\Effect\\", "Suhwanzin2" );
	gLoadData.OpenTexture(MODEL_SUMMONER_CASTING_EFFECT2, "Effect\\");
	gLoadData.AccessModel(MODEL_SUMMONER_CASTING_EFFECT22, "Data\\Effect\\", "Suhwanzin22" );
	gLoadData.OpenTexture(MODEL_SUMMONER_CASTING_EFFECT22, "Effect\\");
	gLoadData.AccessModel(MODEL_SUMMONER_CASTING_EFFECT222, "Data\\Effect\\", "Suhwanzin222" );
	gLoadData.OpenTexture(MODEL_SUMMONER_CASTING_EFFECT222, "Effect\\");
	gLoadData.AccessModel(MODEL_SUMMONER_CASTING_EFFECT4, "Data\\Effect\\", "Suhwanzin4" );
	gLoadData.OpenTexture(MODEL_SUMMONER_CASTING_EFFECT4, "Effect\\");
	gLoadData.AccessModel(MODEL_SUMMONER_SUMMON_SAHAMUTT, "Data\\Skill\\", "summon_sahamutt" );
	gLoadData.OpenTexture(MODEL_SUMMONER_SUMMON_SAHAMUTT, "Skill\\");
	gLoadData.AccessModel(MODEL_SUMMONER_SUMMON_NEIL, "Data\\Skill\\", "summon_neil" );
	gLoadData.OpenTexture(MODEL_SUMMONER_SUMMON_NEIL, "Skill\\");
	gLoadData.OpenTexture(MODEL_SUMMONER_SUMMON_NEIL, "Effect\\");
	gLoadData.AccessModel(MODEL_SUMMONER_SUMMON_LAGUL, "Data\\Skill\\", "summon_lagul");
	gLoadData.OpenTexture(MODEL_SUMMONER_SUMMON_LAGUL, "Skill\\");

	gLoadData.AccessModel(MODEL_SUMMONER_SUMMON_NEIL_NIFE1, "Data\\Skill\\", "nelleff_nife01" );
	gLoadData.OpenTexture(MODEL_SUMMONER_SUMMON_NEIL_NIFE1, "Skill\\");
	gLoadData.AccessModel(MODEL_SUMMONER_SUMMON_NEIL_NIFE2, "Data\\Skill\\", "nelleff_nife02" );
	gLoadData.OpenTexture(MODEL_SUMMONER_SUMMON_NEIL_NIFE2, "Skill\\");
	gLoadData.AccessModel(MODEL_SUMMONER_SUMMON_NEIL_NIFE3, "Data\\Skill\\", "nelleff_nife03" );
	gLoadData.OpenTexture(MODEL_SUMMONER_SUMMON_NEIL_NIFE3, "Skill\\");
	gLoadData.AccessModel(MODEL_SUMMONER_SUMMON_NEIL_GROUND1, "Data\\Skill\\", "nell_nifegrund01" );
	gLoadData.OpenTexture(MODEL_SUMMONER_SUMMON_NEIL_GROUND1, "Skill\\");
	gLoadData.AccessModel(MODEL_SUMMONER_SUMMON_NEIL_GROUND2, "Data\\Skill\\", "nell_nifegrund02" );
	gLoadData.OpenTexture(MODEL_SUMMONER_SUMMON_NEIL_GROUND2, "Skill\\");
	gLoadData.AccessModel(MODEL_SUMMONER_SUMMON_NEIL_GROUND3, "Data\\Skill\\", "nell_nifegrund03" );
	gLoadData.OpenTexture(MODEL_SUMMONER_SUMMON_NEIL_GROUND3, "Skill\\");
	gLoadData.AccessModel(MODEL_MOVE_TARGETPOSITION_EFFECT, "Data\\Effect\\", "MoveTargetPosEffect" );
	gLoadData.OpenTexture(MODEL_MOVE_TARGETPOSITION_EFFECT, "Effect\\");
	gLoadData.AccessModel(MODEL_EFFECT_SAPITRES_ATTACK_1, "Data\\Effect\\", "Sapiatttres" );
	gLoadData.OpenTexture(MODEL_EFFECT_SAPITRES_ATTACK_1, "Effect\\");
	gLoadData.AccessModel(MODEL_EFFECT_SAPITRES_ATTACK_2, "Data\\Effect\\", "Sapiatttres2" );
	gLoadData.OpenTexture(MODEL_EFFECT_SAPITRES_ATTACK_2, "Effect\\");

	gLoadData.AccessModel(MODEL_RAKLION_BOSS_CRACKEFFECT, "Data\\Effect\\", "knight_plancrack_grand" );
	gLoadData.OpenTexture(MODEL_RAKLION_BOSS_CRACKEFFECT, "Effect\\");
	gLoadData.AccessModel(MODEL_RAKLION_BOSS_MAGIC, "Data\\Effect\\", "serufan_magic" );
	gLoadData.OpenTexture(MODEL_RAKLION_BOSS_MAGIC, "Effect\\");
	//Models[MODEL_RAKLION_BOSS_MAGIC].Actions[0].PlaySpeed = 0.005f;

	gLoadData.AccessModel(MODEL_EFFECT_SKURA_ITEM, "Data\\Effect\\cherryblossom\\", "Skura_iteam_event" );

	gLoadData.AccessModel(MODEL_EFFECT_BROKEN_ICE0, "Data\\Effect\\", "ice_stone00" );
	gLoadData.OpenTexture(MODEL_EFFECT_BROKEN_ICE0, "Effect\\");
	gLoadData.AccessModel(MODEL_EFFECT_BROKEN_ICE1, "Data\\Effect\\", "ice_stone01" );
	gLoadData.OpenTexture(MODEL_EFFECT_BROKEN_ICE1, "Effect\\");
	gLoadData.AccessModel(MODEL_EFFECT_BROKEN_ICE2, "Data\\Effect\\", "ice_stone02" );
	gLoadData.OpenTexture(MODEL_EFFECT_BROKEN_ICE2, "Effect\\");
	gLoadData.AccessModel(MODEL_EFFECT_BROKEN_ICE3, "Data\\Effect\\", "ice_stone03" );
	gLoadData.OpenTexture(MODEL_EFFECT_BROKEN_ICE3, "Effect\\");
	gLoadData.AccessModel(MODEL_NIGHTWATER_01, "Data\\Effect\\", "nightwater01" );
	gLoadData.OpenTexture(MODEL_NIGHTWATER_01, "Effect\\");
	gLoadData.AccessModel(MODEL_KNIGHT_PLANCRACK_A, "Data\\Effect\\", "knight_plancrack_a" );
	gLoadData.OpenTexture(MODEL_KNIGHT_PLANCRACK_A, "Effect\\");
	Models[MODEL_KNIGHT_PLANCRACK_A].Actions[0].PlaySpeed = 0.3f;
	gLoadData.AccessModel(MODEL_KNIGHT_PLANCRACK_B, "Data\\Effect\\", "knight_plancrack_b" );
	gLoadData.OpenTexture(MODEL_KNIGHT_PLANCRACK_B, "Effect\\");
	Models[MODEL_KNIGHT_PLANCRACK_B].Actions[0].PlaySpeed = 0.3f;
	gLoadData.AccessModel(MODEL_EFFECT_FLAME_STRIKE, "Data\\Effect\\", "FlameStrike" );
	gLoadData.OpenTexture(MODEL_EFFECT_FLAME_STRIKE, "Effect\\");
	gLoadData.AccessModel(MODEL_SWELL_OF_MAGICPOWER, "Data\\Effect\\", "magic_powerup" );
	gLoadData.OpenTexture(MODEL_SWELL_OF_MAGICPOWER, "Effect\\");
	gLoadData.AccessModel(MODEL_ARROWSRE06, "Data\\Effect\\", "arrowsre06" );
	gLoadData.OpenTexture(MODEL_ARROWSRE06, "Effect\\");
	gLoadData.AccessModel(MODEL_DOPPELGANGER_SLIME_CHIP, "Data\\Effect\\", "slime_chip" );
	gLoadData.OpenTexture(MODEL_DOPPELGANGER_SLIME_CHIP, "Effect\\");
	gLoadData.AccessModel(MODEL_EFFECT_UMBRELLA_GOLD, "Data\\Effect\\", "japan_gold01" );
	gLoadData.OpenTexture(MODEL_EFFECT_UMBRELLA_GOLD, "Effect\\");
	gLoadData.AccessModel(MODEL_EMPIREGUARDIANBOSS_FRAMESTRIKE, "Data\\Effect\\", "Karanebos_sword_framestrike" );
	gLoadData.OpenTexture(MODEL_EMPIREGUARDIANBOSS_FRAMESTRIKE, "Effect\\");
	//Models[MODEL_EMPIREGUARDIANBOSS_FRAMESTRIKE].Actions[MONSTER01_STOP1].PlaySpeed = 3.0f;
	gLoadData.AccessModel(MODEL_DEASULER, "Data\\Monster\\", "deasther_boomerang" );
	gLoadData.OpenTexture(MODEL_DEASULER, "Monster\\");
	gLoadData.AccessModel(MODEL_EFFECT_SD_AURA, "Data\\Effect\\", "shield_up" );
	gLoadData.OpenTexture(MODEL_EFFECT_SD_AURA, "Effect\\");
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
	gLoadData.AccessModel(MODEL_WOLF_HEAD_EFFECT, "Data\\Effect\\", "wolf_head_effect" );
	gLoadData.OpenTexture(MODEL_WOLF_HEAD_EFFECT, "Effect\\");
	LoadBitmap("Effect\\sbumb.jpg", BITMAP_SBUMB, GL_LINEAR, GL_REPEAT);
	gLoadData.AccessModel(MODEL_DOWN_ATTACK_DUMMY_L, "Data\\Effect\\", "down_right_punch");
	gLoadData.OpenTexture(MODEL_DOWN_ATTACK_DUMMY_L, "Effect\\");
	gLoadData.AccessModel(MODEL_DOWN_ATTACK_DUMMY_R, "Data\\Effect\\", "down_left_punch");
	gLoadData.OpenTexture(MODEL_DOWN_ATTACK_DUMMY_R, "Effect\\");
	gLoadData.AccessModel(MODEL_SHOCKWAVE01, "Data\\Effect\\", "shockwave01");
	gLoadData.OpenTexture(MODEL_SHOCKWAVE01, "Effect\\");
	gLoadData.AccessModel(MODEL_SHOCKWAVE02, "Data\\Effect\\", "shockwave02");
	gLoadData.OpenTexture(MODEL_SHOCKWAVE02, "Effect\\");
	gLoadData.AccessModel(MODEL_SHOCKWAVE_SPIN01, "Data\\Effect\\", "shockwave_spin01");
	gLoadData.OpenTexture(MODEL_SHOCKWAVE_SPIN01, "Effect\\");
	gLoadData.AccessModel(MODEL_WINDFOCE, "Data\\Effect\\", "wind_foce");
	gLoadData.OpenTexture(MODEL_WINDFOCE, "Effect\\");
	gLoadData.AccessModel(MODEL_WINDFOCE_MIRROR, "Data\\Effect\\", "wind_foce_mirror");
	gLoadData.OpenTexture(MODEL_WINDFOCE_MIRROR, "Effect\\");
	gLoadData.AccessModel(MODEL_WOLF_HEAD_EFFECT2, "Data\\Effect\\", "wolf_head_effect2");
	gLoadData.OpenTexture(MODEL_WOLF_HEAD_EFFECT2, "skill\\");
	gLoadData.AccessModel(MODEL_SHOCKWAVE_GROUND01, "Data\\Effect\\", "shockwave_ground01");
	gLoadData.OpenTexture(MODEL_SHOCKWAVE_GROUND01, "Effect\\");
	gLoadData.AccessModel(MODEL_DRAGON_KICK_DUMMY, "Data\\Effect\\", "dragon_kick_dummy");
	gLoadData.OpenTexture(MODEL_DRAGON_KICK_DUMMY, "Effect\\");
	gLoadData.AccessModel(MODEL_DRAGON_LOWER_DUMMY, "Data\\Effect\\", "knight_plancrack_dragon");
	gLoadData.OpenTexture(MODEL_DRAGON_LOWER_DUMMY, "Effect\\");
	gLoadData.AccessModel(MODEL_VOLCANO_OF_MONK, "Data\\Effect\\", "volcano_of_monk");
	gLoadData.OpenTexture(MODEL_VOLCANO_OF_MONK, "Effect\\");
	gLoadData.AccessModel(MODEL_VOLCANO_STONE, "Data\\Effect\\", "volcano_stone");
	gLoadData.OpenTexture(MODEL_VOLCANO_STONE, "Effect\\");
	LoadBitmap("Effect\\force_Pillar.jpg", BITMAP_FORCEPILLAR, GL_LINEAR, GL_REPEAT);
	LoadBitmap("Effect\\!SwordEff.jpg", BITMAP_SWORDEFF, GL_LINEAR, GL_REPEAT);
	LoadBitmap("Effect\\Damage1.jpg", BITMAP_DAMAGE1, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\ground_wind.jpg", BITMAP_GROUND_WIND, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\Kwave2.jpg", BITMAP_KWAVE2, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\Damage2.jpg", BITMAP_DAMAGE2, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\volcano_core.jpg", BITMAP_VOLCANO_CORE, GL_LINEAR, GL_CLAMP_TO_EDGE);
	gLoadData.AccessModel(MODEL_SHOCKWAVE03, "Data\\Effect\\", "shockwave03");
	gLoadData.OpenTexture(MODEL_SHOCKWAVE03, "Effect\\");
	LoadBitmap("Effect\\ground_smoke.tga", BITMAP_GROUND_SMOKE, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\knightSt_blue.jpg", BITMAP_KNIGHTST_BLUE, GL_LINEAR, GL_CLAMP_TO_EDGE);
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
#ifdef ASG_ADD_KARUTAN_MONSTERS
	// 콘드라 돌조각
	gLoadData.AccessModel(MODEL_CONDRA_STONE, "Data\\Monster\\", "condra_7_stone" );
	gLoadData.OpenTexture(MODEL_CONDRA_STONE, "Monster\\");
	gLoadData.AccessModel(MODEL_CONDRA_STONE1, "Data\\Monster\\", "condra_7_stone_2" );
	gLoadData.OpenTexture(MODEL_CONDRA_STONE1, "Monster\\");
	gLoadData.AccessModel(MODEL_CONDRA_STONE2, "Data\\Monster\\", "condra_7_stone_3" );
	gLoadData.OpenTexture(MODEL_CONDRA_STONE2, "Monster\\");
	gLoadData.AccessModel(MODEL_CONDRA_STONE3, "Data\\Monster\\", "condra_7_stone_4" );
	gLoadData.OpenTexture(MODEL_CONDRA_STONE3, "Monster\\");
	gLoadData.AccessModel(MODEL_CONDRA_STONE4, "Data\\Monster\\", "condra_7_stone_5" );
	gLoadData.OpenTexture(MODEL_CONDRA_STONE4, "Monster\\");
	gLoadData.AccessModel(MODEL_CONDRA_STONE5, "Data\\Monster\\", "condra_7_stone_6" );
	gLoadData.OpenTexture(MODEL_CONDRA_STONE5, "Monster\\");

	gLoadData.AccessModel(MODEL_NARCONDRA_STONE, "Data\\Monster\\", "nar_condra_7_stone_1" );
	gLoadData.OpenTexture(MODEL_NARCONDRA_STONE, "Monster\\");
	gLoadData.AccessModel(MODEL_NARCONDRA_STONE1, "Data\\Monster\\", "nar_condra_7_stone_2" );
	gLoadData.OpenTexture(MODEL_NARCONDRA_STONE1, "Monster\\");
	gLoadData.AccessModel(MODEL_NARCONDRA_STONE2, "Data\\Monster\\", "nar_condra_7_stone_3" );
	gLoadData.OpenTexture(MODEL_NARCONDRA_STONE2, "Monster\\");
	gLoadData.AccessModel(MODEL_NARCONDRA_STONE3, "Data\\Monster\\", "nar_condra_7_stone_4" );
	gLoadData.OpenTexture(MODEL_NARCONDRA_STONE3, "Monster\\");
#endif	// ASG_ADD_KARUTAN_MONSTERS
}

#include "ReadScript.h"

void SaveWorld(int World)
{
	char WorldName[32];
	char FileName[64];
	sprintf(WorldName,"World%d",World);

	sprintf(FileName,"Data2\\%s\\TerrainLight.jpg",WorldName);
    SaveTerrainLight(FileName);
	sprintf(FileName,"Data2\\%s\\TerrainHeight.bmp",WorldName);
    SaveTerrainHeight(FileName);
	sprintf(FileName,"Data\\%s\\Terrain.map",WorldName);
	SaveTerrainMapping(FileName,World);
	sprintf(FileName,"Data\\%s\\Terrain.att",WorldName);
	SaveTerrainAttribute(FileName,World);
	sprintf(FileName,"Data\\%s\\Terrain.obj",WorldName);
	SaveObjects(FileName,World);
}

void OpenImages()
{
	LoadBitmap("Interface\\command.jpg"       ,BITMAP_SKILL_INTERFACE+2);
	LoadBitmap("Interface\\Item_Back01.jpg"    ,BITMAP_INVENTORY  );
	LoadBitmap("Interface\\Item_Money.jpg"     ,BITMAP_INVENTORY+11);
	LoadBitmap("Interface\\Item_box.jpg"       ,BITMAP_INVENTORY+17);
	LoadBitmap("Interface\\InventoryBox2.jpg"  ,BITMAP_INVENTORY+18);
	LoadBitmap("Interface\\Trading_line.jpg"   ,BITMAP_INVENTORY+19);
	LoadBitmap("Interface\\exit_01.jpg"     ,BITMAP_INVENTORY_BUTTON);
	LoadBitmap("Interface\\exit_02.jpg"     ,BITMAP_INVENTORY_BUTTON+1);
	LoadBitmap("Interface\\accept_box01.jpg",BITMAP_INVENTORY_BUTTON+10);
	LoadBitmap("Interface\\accept_box02.jpg",BITMAP_INVENTORY_BUTTON+11);
	LoadBitmap("Interface\\mix_button1.jpg" ,BITMAP_INVENTORY_BUTTON+12);
	LoadBitmap("Interface\\mix_button2.jpg" ,BITMAP_INVENTORY_BUTTON+13);
	LoadBitmap("Interface\\lock_01.jpg" ,BITMAP_INVENTORY_BUTTON+14);
	LoadBitmap("Interface\\lock_02.jpg" ,BITMAP_INVENTORY_BUTTON+15);
	LoadBitmap("Interface\\lock_03.jpg" ,BITMAP_INVENTORY_BUTTON+16);
	LoadBitmap("Interface\\lock_04.jpg" ,BITMAP_INVENTORY_BUTTON+17);
	LoadBitmap("Interface\\guild.tga",BITMAP_GUILD);
}

void OpenSounds()
{
	bool Enable3DSound = true;

    LoadWaveFile(SOUND_WIND01			,"Data\\Sound\\aWind.wav",1);
    LoadWaveFile(SOUND_RAIN01			,"Data\\Sound\\aRain.wav",1);
    LoadWaveFile(SOUND_DUNGEON01		,"Data\\Sound\\aDungeon.wav",1);
    LoadWaveFile(SOUND_FOREST01			,"Data\\Sound\\aForest.wav",1);
    LoadWaveFile(SOUND_TOWER01		    ,"Data\\Sound\\aTower.wav",1);
    LoadWaveFile(SOUND_WATER01		    ,"Data\\Sound\\aWater.wav",1);
    LoadWaveFile(SOUND_DESERT01		    ,"Data\\Sound\\desert.wav",1);
    //LoadWaveFile(SOUND_BOSS01		    ,"Data\\Sound\\a쿤둔.wav",1);
    LoadWaveFile(SOUND_HUMAN_WALK_GROUND,"Data\\Sound\\pWalk(Soil).wav",2);
    LoadWaveFile(SOUND_HUMAN_WALK_GRASS	,"Data\\Sound\\pWalk(Grass).wav",2);
    LoadWaveFile(SOUND_HUMAN_WALK_SNOW	,"Data\\Sound\\pWalk(Snow).wav",2);
    LoadWaveFile(SOUND_HUMAN_WALK_SWIM	,"Data\\Sound\\pSwim.wav",2);

	LoadWaveFile(SOUND_BIRD01			,"Data\\Sound\\aBird1.wav",1,Enable3DSound); 
    LoadWaveFile(SOUND_BIRD02			,"Data\\Sound\\aBird2.wav",1,Enable3DSound);
    LoadWaveFile(SOUND_BAT01			,"Data\\Sound\\aBat.wav",1,Enable3DSound);
    LoadWaveFile(SOUND_RAT01			,"Data\\Sound\\aMouse.wav",1,Enable3DSound);
    LoadWaveFile(SOUND_TRAP01			,"Data\\Sound\\aGrate.wav",1,Enable3DSound);
    LoadWaveFile(SOUND_DOOR01			,"Data\\Sound\\aDoor.wav",1);
    LoadWaveFile(SOUND_DOOR02			,"Data\\Sound\\aCastleDoor.wav",1);

    LoadWaveFile(SOUND_HEAVEN01         ,"Data\\Sound\\aHeaven.wav",1);
    LoadWaveFile(SOUND_THUNDERS01       ,"Data\\Sound\\aThunder01.wav",1);
    LoadWaveFile(SOUND_THUNDERS02       ,"Data\\Sound\\aThunder02.wav",1);
    LoadWaveFile(SOUND_THUNDERS03       ,"Data\\Sound\\aThunder03.wav",1);

	//attack
    LoadWaveFile(SOUND_BRANDISH_SWORD01 ,"Data\\Sound\\eSwingWeapon1.wav",2); 
    LoadWaveFile(SOUND_BRANDISH_SWORD02 ,"Data\\Sound\\eSwingWeapon2.wav",2); 
    LoadWaveFile(SOUND_BRANDISH_SWORD03 ,"Data\\Sound\\eSwingLightSword.wav",2); 
    LoadWaveFile(SOUND_BOW01	        ,"Data\\Sound\\eBow.wav",2);
    LoadWaveFile(SOUND_CROSSBOW01	    ,"Data\\Sound\\eCrossbow.wav",2);
    LoadWaveFile(SOUND_MIX01	        ,"Data\\Sound\\eMix.wav",2);

	//player
    LoadWaveFile(SOUND_DRINK01     	    ,"Data\\Sound\\pDrink.wav",1);
    LoadWaveFile(SOUND_EAT_APPLE01  	,"Data\\Sound\\pEatApple.wav",1);
    LoadWaveFile(SOUND_HEART           	,"Data\\Sound\\pHeartBeat.wav",1);
    LoadWaveFile(SOUND_GET_ENERGY      	,"Data\\Sound\\pEnergy.wav",1);
    LoadWaveFile(SOUND_HUMAN_SCREAM01	,"Data\\Sound\\pMaleScream1.wav",2); 
    LoadWaveFile(SOUND_HUMAN_SCREAM02	,"Data\\Sound\\pMaleScream2.wav",2);
    LoadWaveFile(SOUND_HUMAN_SCREAM03	,"Data\\Sound\\pMaleScream3.wav",2);
    LoadWaveFile(SOUND_HUMAN_SCREAM04	,"Data\\Sound\\pMaleDie.wav",2);
    LoadWaveFile(SOUND_FEMALE_SCREAM01	,"Data\\Sound\\pFemaleScream1.wav",2); 
    LoadWaveFile(SOUND_FEMALE_SCREAM02	,"Data\\Sound\\pFemaleScream2.wav",2);

    LoadWaveFile(SOUND_DROP_ITEM01	    ,"Data\\Sound\\pDropItem.wav",1); 
    LoadWaveFile(SOUND_DROP_GOLD01      ,"Data\\Sound\\pDropMoney.wav",1); 
    LoadWaveFile(SOUND_JEWEL01	      	,"Data\\Sound\\eGem.wav",1); 
    LoadWaveFile(SOUND_GET_ITEM01		,"Data\\Sound\\pGetItem.wav",1); 
    //LoadWaveFile(SOUND_SHOUT01    		,"Data\\Sound\\p기합.wav",1); 

	//skill
    LoadWaveFile(SOUND_SKILL_DEFENSE    ,"Data\\Sound\\sKnightDefense.wav",1);
    LoadWaveFile(SOUND_SKILL_SWORD1     ,"Data\\Sound\\sKnightSkill1.wav",1);
    LoadWaveFile(SOUND_SKILL_SWORD2     ,"Data\\Sound\\sKnightSkill2.wav",1);
    LoadWaveFile(SOUND_SKILL_SWORD3     ,"Data\\Sound\\sKnightSkill3.wav",1);
    LoadWaveFile(SOUND_SKILL_SWORD4     ,"Data\\Sound\\sKnightSkill4.wav",1);
	LoadWaveFile(SOUND_MONSTER+116      ,"Data\\Sound\\mShadowAttack1.wav",1);

    LoadWaveFile(SOUND_STORM            ,"Data\\Sound\\sTornado.wav",2,Enable3DSound);
    LoadWaveFile(SOUND_EVIL             ,"Data\\Sound\\sEvil.wav",2,Enable3DSound);
    LoadWaveFile(SOUND_MAGIC            ,"Data\\Sound\\sMagic.wav",2,Enable3DSound);
    LoadWaveFile(SOUND_HELLFIRE         ,"Data\\Sound\\sHellFire.wav",2,Enable3DSound);
    LoadWaveFile(SOUND_ICE              ,"Data\\Sound\\sIce.wav",2,Enable3DSound);
    LoadWaveFile(SOUND_FLAME            ,"Data\\Sound\\sFlame.wav",2,Enable3DSound);
    //LoadWaveFile(SOUND_FLASH            ,"Data\\Sound\\m히드라공격1.wav",2,Enable3DSound);
    LoadWaveFile(SOUND_FLASH            ,"Data\\Sound\\sAquaFlash.wav",2,Enable3DSound);

    LoadWaveFile(SOUND_BREAK01	      	,"Data\\Sound\\eBreak.wav",1,Enable3DSound); 
    LoadWaveFile(SOUND_EXPLOTION01		,"Data\\Sound\\eExplosion.wav",1,Enable3DSound); 
    LoadWaveFile(SOUND_METEORITE01		,"Data\\Sound\\eMeteorite.wav",2,Enable3DSound); 
    //LoadWaveFile(SOUND_METEORITE02	    ,"Data\\Sound\\e유성.wav",2,Enable3DSound);
    LoadWaveFile(SOUND_THUNDER01		,"Data\\Sound\\eThunder.wav",1,Enable3DSound); 

    LoadWaveFile(SOUND_BONE1	     	,"Data\\Sound\\mBone1.wav",2,Enable3DSound);
    LoadWaveFile(SOUND_BONE2    		,"Data\\Sound\\mBone2.wav",2,Enable3DSound);
    LoadWaveFile(SOUND_ASSASSIN		    ,"Data\\Sound\\mAssassin1.wav",1,Enable3DSound);


	LoadWaveFile(SOUND_ATTACK01			,"Data\\Sound\\eMeleeHit1.wav",2); 
    LoadWaveFile(SOUND_ATTACK01+1     	,"Data\\Sound\\eMeleeHit2.wav",2); 
    LoadWaveFile(SOUND_ATTACK01+2		,"Data\\Sound\\eMeleeHit3.wav",2); 
    LoadWaveFile(SOUND_ATTACK01+3		,"Data\\Sound\\eMeleeHit4.wav",2); 
    LoadWaveFile(SOUND_ATTACK01+4		,"Data\\Sound\\eMeleeHit5.wav",2); 
    LoadWaveFile(SOUND_ATTACK01+5		,"Data\\Sound\\eMissileHit1.wav",2); 
    LoadWaveFile(SOUND_ATTACK01+6		,"Data\\Sound\\eMissileHit2.wav",2); 
	LoadWaveFile(SOUND_ATTACK01+7		,"Data\\Sound\\eMissileHit3.wav",2);
	LoadWaveFile(SOUND_ATTACK01+8		,"Data\\Sound\\eMissileHit4.wav",2);

    LoadWaveFile(SOUND_FIRECRACKER1	    ,"Data\\Sound\\eFirecracker1.wav",1,Enable3DSound);
    LoadWaveFile(SOUND_FIRECRACKER2	    ,"Data\\Sound\\eFirecracker2.wav",1,Enable3DSound);
    LoadWaveFile(SOUND_MEDAL	        ,"Data\\Sound\\eMedal.wav",1,Enable3DSound);
	LoadWaveFile(SOUND_PHOENIXEXP		,"Data\\Sound\\ePhoenixExp.wav",1,Enable3DSound);
//	LoadWaveFile(SOUND_PHOENIXFIRE		,"Data\\Sound\\ePhoenixFire.wav",1,Enable3DSound);

    LoadWaveFile(SOUND_RIDINGSPEAR		,"Data\\Sound\\eRidingSpear.wav",1);
	LoadWaveFile(SOUND_RAIDSHOOT		,"Data\\Sound\\eRaidShoot.wav",1);
	LoadWaveFile(SOUND_SWELLLIFE		,"Data\\Sound\\eSwellLife.wav",1);
	LoadWaveFile(SOUND_PIERCING			,"Data\\Sound\\ePiercing.wav",1);
	LoadWaveFile(SOUND_ICEARROW			,"Data\\Sound\\eIceArrow.wav",1);
	LoadWaveFile(SOUND_TELEKINESIS		,"Data\\Sound\\eTelekinesis.wav",1);
	LoadWaveFile(SOUND_SOULBARRIER      ,"Data\\Sound\\eSoulBarrier.wav",1);
	LoadWaveFile(SOUND_BLOODATTACK      ,"Data\\Sound\\eBloodAttack.wav",1);

	LoadWaveFile(SOUND_HIT_GATE         ,"Data\\Sound\\eHitGate.wav",1);
	LoadWaveFile(SOUND_HIT_GATE2        ,"Data\\Sound\\eHitGate2.wav",1);
	LoadWaveFile(SOUND_HIT_CRISTAL      ,"Data\\Sound\\eHitCristal.wav",1);
	LoadWaveFile(SOUND_DOWN_GATE        ,"Data\\Sound\\eDownGate.wav",1);
	LoadWaveFile(SOUND_CROW             ,"Data\\Sound\\eCrow.wav",1);

	LoadWaveFile ( SOUND_DEATH_POISON1, "Data\\Sound\\eBlastPoison_1.wav", 2 );
	LoadWaveFile ( SOUND_DEATH_POISON2, "Data\\Sound\\eBlastPoison_2.wav", 2 );
	LoadWaveFile ( SOUND_SUDDEN_ICE1  , "Data\\Sound\\eSuddenIce_1.wav", 2 );
	LoadWaveFile ( SOUND_SUDDEN_ICE2  , "Data\\Sound\\eSuddenIce_2.wav", 2 );
	LoadWaveFile ( SOUND_NUKE1		  , "Data\\Sound\\eHellFire2_1.wav", 1 );
	LoadWaveFile ( SOUND_NUKE2		  , "Data\\Sound\\eHellFire2_2.wav", 1 );
	LoadWaveFile ( SOUND_COMBO		  , "Data\\Sound\\eCombo.wav", 1 );
	LoadWaveFile ( SOUND_FURY_STRIKE1 , "Data\\Sound\\eRageBlow_1.wav", 1 );
	LoadWaveFile ( SOUND_FURY_STRIKE2 , "Data\\Sound\\eRageBlow_2.wav", 1 );
	LoadWaveFile ( SOUND_FURY_STRIKE3 , "Data\\Sound\\eRageBlow_3.wav", 1 );
	LoadWaveFile ( SOUND_LEVEL_UP	  , "Data\\Sound\\pLevelUp.wav", 1 );
	LoadWaveFile ( SOUND_CHANGE_UP	  , "Data\\Sound\\nMalonSkillMaster.wav", 1 );

	LoadWaveFile ( SOUND_CHAOS_ENVIR,       "Data\\Sound\\aChaos.wav", 1 );
	LoadWaveFile ( SOUND_CHAOS_END,         "Data\\Sound\\aChaosEnd.wav", 1 );
	LoadWaveFile ( SOUND_CHAOS_FALLING,     "Data\\Sound\\pMaleScream.wav", 1 );
	LoadWaveFile ( SOUND_CHAOS_FALLING_STONE,"Data\\Sound\\eWallFall.wav", 1 );
	LoadWaveFile ( SOUND_CHAOS_MOB_BOOM01,  "Data\\Sound\\eMonsterBoom1.wav", 2 );
	LoadWaveFile ( SOUND_CHAOS_MOB_BOOM02,  "Data\\Sound\\eMonsterBoom2.wav", 2 );
	LoadWaveFile ( SOUND_CHAOS_THUNDER01,   "Data\\Sound\\eElec1.wav", 1 );
	LoadWaveFile ( SOUND_CHAOS_THUNDER02,   "Data\\Sound\\eElec2.wav", 1 );

    LoadWaveFile ( SOUND_RUN_DARK_HORSE_1,      "Data\\Sound\\pHorseStep1.wav", 1 );
    LoadWaveFile ( SOUND_RUN_DARK_HORSE_2,      "Data\\Sound\\pHorseStep2.wav", 1 );
    LoadWaveFile ( SOUND_RUN_DARK_HORSE_3,      "Data\\Sound\\pHorseStep3.wav", 1 );
    LoadWaveFile ( SOUND_DARKLORD_PAIN,         "Data\\Sound\\pDarkPain.wav", 1 );
    LoadWaveFile ( SOUND_DARKLORD_DEAD,         "Data\\Sound\\pDarkDeath.wav", 1 );
    LoadWaveFile ( SOUND_ATTACK_SPEAR,          "Data\\Sound\\sDarkSpear.wav", 1 );
    LoadWaveFile ( SOUND_ATTACK_FIRE_BUST,      "Data\\Sound\\eFirebust.wav", 1 );
    LoadWaveFile ( SOUND_ATTACK_FIRE_BUST_EXP,  "Data\\Sound\\eFirebustBoom.wav", 1 );
    LoadWaveFile ( SOUND_PART_TELEPORT,         "Data\\Sound\\eSummon.wav", 1 );
    LoadWaveFile ( SOUND_ELEC_STRIKE,           "Data\\Sound\\sDarkElecSpike.wav", 1 );
    LoadWaveFile ( SOUND_ELEC_STRIKE_READY,     "Data\\Sound\\sDarkElecSpikeReady.wav", 1 );
    LoadWaveFile ( SOUND_EARTH_QUAKE,           "Data\\Sound\\sDarkEarthQuake.wav", 1 );
    LoadWaveFile ( SOUND_CRITICAL,              "Data\\Sound\\sDarkCritical.wav", 1 );
    LoadWaveFile ( SOUND_DSPIRIT_MISSILE,       "Data\\Sound\\DSpirit_Missile.wav", 4 );
    LoadWaveFile ( SOUND_DSPIRIT_SHOUT,         "Data\\Sound\\DSpirit_Shout.wav", 1 );
    LoadWaveFile ( SOUND_DSPIRIT_RUSH,          "Data\\Sound\\DSpirit_Rush.wav", 3 );
	
	LoadWaveFile ( SOUND_FENRIR_RUN_1,		"Data\\Sound\\pW_run-01.wav",	1 );
	LoadWaveFile ( SOUND_FENRIR_RUN_2,      "Data\\Sound\\pW_run-02.wav",	1 );
	LoadWaveFile ( SOUND_FENRIR_RUN_3,      "Data\\Sound\\pW_run-03.wav",	1 );
	LoadWaveFile ( SOUND_FENRIR_WALK_1,     "Data\\Sound\\pW_step-01.wav",	1 );
	LoadWaveFile ( SOUND_FENRIR_WALK_2,     "Data\\Sound\\pW_step-02.wav",	1 );
	LoadWaveFile ( SOUND_FENRIR_DEATH,		"Data\\Sound\\pWdeath.wav",		1 );
	LoadWaveFile ( SOUND_FENRIR_IDLE_1,     "Data\\Sound\\pWidle1.wav",		1 );
	LoadWaveFile ( SOUND_FENRIR_IDLE_2,     "Data\\Sound\\pWidle2.wav",		1 );
	LoadWaveFile ( SOUND_FENRIR_DAMAGE_1,   "Data\\Sound\\pWpain1.wav",		1 );
	LoadWaveFile ( SOUND_FENRIR_DAMAGE_1,   "Data\\Sound\\pWpain2.wav",		1 );
	LoadWaveFile ( SOUND_FENRIR_SKILL,		"Data\\Sound\\pWskill.wav",		1 );
	LoadWaveFile(SOUND_JEWEL02,				"Data\\Sound\\Jewel_Sound.wav",1);

	LoadWaveFile ( SOUND_KUNDUN_ITEM_SOUND,	"Data\\Sound\\kundunitem.wav",	1 );

	g_XmasEvent->LoadXmasEventSound();
	g_NewYearsDayEvent->LoadSound();
	
	LoadWaveFile ( SOUND_SHIELDCLASH,	"Data\\Sound\\shieldclash.wav",	1 );
	LoadWaveFile ( SOUND_INFINITYARROW,	"Data\\Sound\\infinityArrow.wav",	1 );
	//SOUND_FIRE_SCREAM
	LoadWaveFile ( SOUND_FIRE_SCREAM,	"Data\\Sound\\Darklord_firescream.wav",	1 );

	LoadWaveFile ( SOUND_MOONRABBIT_WALK,		"Data\\Sound\\SE_Ev_rabbit_walk.wav",	1 );
	LoadWaveFile ( SOUND_MOONRABBIT_DAMAGE,		"Data\\Sound\\SE_Ev_rabbit_damage.wav", 1 );
	LoadWaveFile ( SOUND_MOONRABBIT_DEAD,		"Data\\Sound\\SE_Ev_rabbit_death.wav", 1 );
	LoadWaveFile ( SOUND_MOONRABBIT_EXPLOSION,	"Data\\Sound\\SE_Ev_rabbit_Explosion.wav", 1 );

//	LoadWaveFile(SOUND_SUMMON_CASTING,		"Data\\Sound\\eSummon.wav"	,1);
	LoadWaveFile(SOUND_SUMMON_SAHAMUTT,		"Data\\Sound\\SE_Ch_summoner_skill05_explosion01.wav"	,1);
	LoadWaveFile(SOUND_SUMMON_EXPLOSION,	"Data\\Sound\\SE_Ch_summoner_skill05_explosion03.wav"	,1);
	LoadWaveFile(SOUND_SUMMON_NEIL,			"Data\\Sound\\SE_Ch_summoner_skill06_requiem01.wav"		,1);
	LoadWaveFile(SOUND_SUMMON_REQUIEM,		"Data\\Sound\\SE_Ch_summoner_skill06_requiem02.wav"		,1);
	LoadWaveFile(SOUND_SUMMOM_RARGLE, "Data\\Sound\\Rargle.wav", 1);
	LoadWaveFile(SOUND_SUMMON_SKILL_LIGHTORB,	"Data\\Sound\\SE_Ch_summoner_skill01_lightningof.wav"	,1);
	LoadWaveFile(SOUND_SUMMON_SKILL_SLEEP,		"Data\\Sound\\SE_Ch_summoner_skill03_sleep.wav"			,1);
	LoadWaveFile(SOUND_SUMMON_SKILL_BLIND,		"Data\\Sound\\SE_Ch_summoner_skill04_blind.wav"			,1);
	LoadWaveFile(SOUND_SUMMON_SKILL_THORNS,		"Data\\Sound\\SE_Ch_summoner_skill02_ssonze.wav"		,1);
	LoadWaveFile(SOUND_SKILL_CHAIN_LIGHTNING,	"Data\\Sound\\SE_Ch_summoner_skill08_chainlightning.wav",1);
	LoadWaveFile(SOUND_SKILL_DRAIN_LIFE,		"Data\\Sound\\SE_Ch_summoner_skill07_lifedrain.wav"		,1);
	LoadWaveFile(SOUND_SKILL_WEAKNESS, "Data\\Sound\\SE_Ch_summoner_weakness.wav", 1);
	LoadWaveFile(SOUND_SKILL_ENERVATION, "Data\\Sound\\SE_Ch_summoner_innovation.wav", 1);
	LoadWaveFile(SOUND_SKILL_BERSERKER, "Data\\Sound\\Berserker.wav", 1);
	LoadWaveFile(SOUND_CHERRYBLOSSOM_EFFECT0, "Data\\Sound\\cherryblossom\\Eve_CherryBlossoms01.wav");
	LoadWaveFile(SOUND_CHERRYBLOSSOM_EFFECT1, "Data\\Sound\\cherryblossom\\Eve_CherryBlossoms02.wav");
	LoadWaveFile(SOUND_SKILL_BLOWOFDESTRUCTION, "Data\\Sound\\BLOW_OF_DESTRUCTION.wav");
	LoadWaveFile(SOUND_SKILL_FLAME_STRIKE, "Data\\Sound\\flame_strike.wav");
	LoadWaveFile(SOUND_SKILL_GIGANTIC_STORM, "Data\\Sound\\gigantic_storm.wav");
	LoadWaveFile(SOUND_SKILL_LIGHTNING_SHOCK, "Data\\Sound\\lightning_shock.wav");
	LoadWaveFile(SOUND_SKILL_SWELL_OF_MAGICPOWER, "Data\\Sound\\SwellofMagicPower.wav");
	LoadWaveFile(SOUND_SKILL_MULTI_SHOT, "Data\\Sound\\multi_shot.wav");
	LoadWaveFile(SOUND_SKILL_RECOVER, "Data\\Sound\\recover.wav");
	LoadWaveFile(SOUND_SKILL_CAOTIC, "Data\\Sound\\caotic.wav");

	LoadWaveFile(SOUND_XMAS_FIRECRACKER, "Data\\Sound\\xmas\\Christmas_Fireworks01.wav");

	g_09SummerEvent->LoadSound();

#ifdef PBG_ADD_NEWCHAR_MONK
	LoadWaveFile(SOUND_RAGESKILL_THRUST, "Data\\Sound\\Ragefighter\\Rage_Thrust.wav");
	LoadWaveFile(SOUND_RAGESKILL_THRUST_ATTACK, "Data\\Sound\\Ragefighter\\Rage_Thrust_Att.wav");
	LoadWaveFile(SOUND_RAGESKILL_STAMP, "Data\\Sound\\Ragefighter\\Rage_Stamp.wav");
	LoadWaveFile(SOUND_RAGESKILL_STAMP_ATTACK, "Data\\Sound\\Ragefighter\\Rage_Stamp_Att.wav");
	LoadWaveFile(SOUND_RAGESKILL_GIANTSWING, "Data\\Sound\\Ragefighter\\Rage_Giantswing.wav");
	LoadWaveFile(SOUND_RAGESKILL_GIANTSWING_ATTACK, "Data\\Sound\\Ragefighter\\Rage_Giantswing_Att.wav");
	LoadWaveFile(SOUND_RAGESKILL_DARKSIDE, "Data\\Sound\\Ragefighter\\Rage_Darkside.wav");
	LoadWaveFile(SOUND_RAGESKILL_DARKSIDE_ATTACK, "Data\\Sound\\Ragefighter\\Rage_Darkside_Att.wav");
	LoadWaveFile(SOUND_RAGESKILL_DRAGONLOWER, "Data\\Sound\\Ragefighter\\Rage_Dragonlower.wav");
	LoadWaveFile(SOUND_RAGESKILL_DRAGONLOWER_ATTACK, "Data\\Sound\\Ragefighter\\Rage_Dragonlower_Att.wav");
	LoadWaveFile(SOUND_RAGESKILL_DRAGONKICK, "Data\\Sound\\Ragefighter\\Rage_Dragonkick.wav");
	LoadWaveFile(SOUND_RAGESKILL_DRAGONKICK_ATTACK, "Data\\Sound\\Ragefighter\\Rage_Dragonkick_Att.wav");
	LoadWaveFile(SOUND_RAGESKILL_BUFF_1, "Data\\Sound\\Ragefighter\\Rage_Buff_1.wav");
	LoadWaveFile(SOUND_RAGESKILL_BUFF_2, "Data\\Sound\\Ragefighter\\Rage_Buff_2.wav");
#endif //PBG_ADD_NEWCHAR_MONK
}

extern int	g_iRenderTextType;

void OpenFont()
{
   	InitPath();

    LoadBitmap("Interface\\FontInput.tga"     ,BITMAP_FONT    ,GL_NEAREST,GL_CLAMP_TO_EDGE);
	LoadBitmap("Interface\\FontTest.tga"      ,BITMAP_FONT+1  );
	LoadBitmap("Interface\\Hit.tga"			,BITMAP_FONT_HIT,GL_NEAREST,GL_CLAMP_TO_EDGE);

	g_pRenderText->Create(0, g_hDC);
}

void SaveMacro(char *FileName)
{
	FILE *fp = fopen(FileName,"wt");
	for(int i=0;i<10;i++)
	{
		fprintf(fp,"%s\n",MacroText[i]);
	}
	fclose(fp);
}

void OpenMacro(char *FileName)
{
	FILE *fp = fopen(FileName,"rt");
	if(fp == NULL) return;
	for(int i=0;i<10;i++)
	{
		fscanf(fp,"%s",MacroText[i]);
	}
	fclose(fp);
}

void SaveOptions()
{
	// 0 ~ 19 skill hotkey
	BYTE options[30] = { 0x00, };

	int iSkillType = -1;
	for(int i=0; i<10; ++i)
	{
		iSkillType = g_pMainFrame->GetSkillHotKey(i);

		int iIndex = i * 2;
		if(iSkillType != -1)
		{
			
			options[iIndex] = HIBYTE(CharacterAttribute->Skill[iSkillType]);
			options[iIndex+1] = LOBYTE(CharacterAttribute->Skill[iSkillType]);
		}
		else
		{
			options[iIndex] = 0xff;
			options[iIndex+1] = 0xff;
		}
	}

	if( g_pOption->IsAutoAttack() )
	{
		options[20] |= AUTOATTACK_ON;
	}
	else
	{
		options[20] |= AUTOATTACK_OFF;
    }

	if(g_pOption->IsWhisperSound())
	{
		options[20] |= WHISPER_SOUND_ON;
	}
	else
	{
		options[20] |= WHISPER_SOUND_OFF;
    }

	if(g_pOption->IsSlideHelp() == false)
	{
		options[20] |= SLIDE_HELP_OFF;
    }

	options[21] = static_cast<BYTE>((g_pMainFrame->GetItemHotKey(SEASON3B::HOTKEY_Q) - ITEM_POTION) & 0xFF);
	options[22] = static_cast<BYTE>((g_pMainFrame->GetItemHotKey(SEASON3B::HOTKEY_W) - ITEM_POTION) & 0xFF);
	options[23] = static_cast<BYTE>((g_pMainFrame->GetItemHotKey(SEASON3B::HOTKEY_E) - ITEM_POTION) & 0xFF);

	BYTE wChatListBoxSize = g_pChatListBox->GetNumberOfLines(g_pChatListBox->GetCurrentMsgType()) / 3;
	if (g_bUseChatListBox == FALSE) 
		wChatListBoxSize = 0;
	BYTE wChatListBoxBackAlpha = g_pChatListBox->GetBackAlpha() * 10;
	options[24] = wChatListBoxSize << 4 | wChatListBoxBackAlpha;
	options[25] = static_cast<BYTE>((g_pMainFrame->GetItemHotKey(SEASON3B::HOTKEY_R) - ITEM_POTION) & 0xFF);


	options[26] = g_pMainFrame->GetItemHotKeyLevel(SEASON3B::HOTKEY_Q);
	options[27] = g_pMainFrame->GetItemHotKeyLevel(SEASON3B::HOTKEY_W);
	options[28] = g_pMainFrame->GetItemHotKeyLevel(SEASON3B::HOTKEY_E);
	options[29] = g_pMainFrame->GetItemHotKeyLevel(SEASON3B::HOTKEY_R);

	SendRequestHotKey (options);
}

void OpenLogoSceneData()
{
#ifndef PJH_NEW_SERVER_SELECT_MAP
	gLoadData.AccessModel(MODEL_LOGO+2,"Data\\Logo\\","Logo",3);
    gLoadData.OpenTexture(MODEL_LOGO+2,"Logo\\",GL_REPEAT, GL_LINEAR);
#endif //PJH_NEW_SERVER_SELECT_MAP
	//image
	::LoadBitmap("Interface\\cha_bt.tga", BITMAP_LOG_IN);
	::LoadBitmap("Interface\\server_b2_all.tga", BITMAP_LOG_IN+1);
	::LoadBitmap("Interface\\server_b2_loding.jpg", BITMAP_LOG_IN+2);
	::LoadBitmap("Interface\\server_deco_all.tga", BITMAP_LOG_IN+3);
	::LoadBitmap("Interface\\server_menu_b_all.tga", BITMAP_LOG_IN+4);
	::LoadBitmap("Interface\\server_credit_b_all.tga", BITMAP_LOG_IN+5);
	::LoadBitmap("Interface\\deco.tga", BITMAP_LOG_IN+6);
	::LoadBitmap("Interface\\login_back.tga", BITMAP_LOG_IN+7);
	::LoadBitmap("Interface\\login_me.tga", BITMAP_LOG_IN+8);
	::LoadBitmap("Interface\\server_ex03.tga", BITMAP_LOG_IN+11, GL_NEAREST, GL_REPEAT);
	::LoadBitmap("Interface\\server_ex01.tga", BITMAP_LOG_IN+12);
	::LoadBitmap("Interface\\server_ex02.jpg", BITMAP_LOG_IN+13, GL_NEAREST, GL_REPEAT);
	::LoadBitmap("Interface\\cr_mu_lo.tga", BITMAP_LOG_IN+14, GL_LINEAR);
#ifdef MOVIE_DIRECTSHOW
	::LoadBitmap("Interface\\movie_b_all.tga", BITMAP_LOG_IN+15);// 동영상 버튼.
#endif	// MOVIE_DIRECTSHOW

}

void ReleaseLogoSceneData()
{
	for (int i = BITMAP_LOG_IN; i <= BITMAP_LOG_IN_END; ++i)
		::DeleteBitmap(i);
	for(int i=BITMAP_TEMP;i<BITMAP_TEMP+30;i++)
		DeleteBitmap(i);

	gMapManager.DeleteObjects();
	ClearCharacters();
}

void OpenCharacterSceneData()
{
	LoadBitmap("Interface\\cha_id.tga", BITMAP_LOG_IN);
	LoadBitmap("Interface\\cha_bt.tga", BITMAP_LOG_IN+1);
	LoadBitmap("Interface\\deco.tga", BITMAP_LOG_IN+2);
	LoadBitmap("Interface\\b_create.tga", BITMAP_LOG_IN+3);
	LoadBitmap("Interface\\server_menu_b_all.tga", BITMAP_LOG_IN+4);
	LoadBitmap("Interface\\b_connect.tga", BITMAP_LOG_IN+5);
	LoadBitmap("Interface\\b_delete.tga", BITMAP_LOG_IN+6);
	LoadBitmap("Interface\\character_ex.tga", BITMAP_LOG_IN+7);	
	::LoadBitmap("Interface\\server_ex03.tga", BITMAP_LOG_IN+11, GL_NEAREST, GL_REPEAT);
	::LoadBitmap("Interface\\server_ex01.tga", BITMAP_LOG_IN+12);
	::LoadBitmap("Interface\\server_ex02.jpg", BITMAP_LOG_IN+13, GL_NEAREST, GL_REPEAT);
	LoadBitmap("Effect\\Impack03.jpg"			, BITMAP_EXT_LOG_IN+2);
	LoadBitmap("Logo\\chasellight.jpg"		, BITMAP_EFFECT);
    
	int Class = MAX_CLASS;

	for(int i=4;i<5;i++)
      	gLoadData.AccessModel(MODEL_LOGO+i,"Data\\Logo\\","Logo",i+1);

	for(int i=0;i<Class;i++)
     	gLoadData.AccessModel(MODEL_FACE+i,"Data\\Logo\\","NewFace",i+1);
	
	for(int i=4;i<5;i++)
     	gLoadData.OpenTexture(MODEL_LOGO+i,"Logo\\", GL_REPEAT, GL_LINEAR);

	for(int i=0;i<Class;i++)
     	gLoadData.OpenTexture(MODEL_FACE+i,"Logo\\");

	for(int i=0;i<Class;i++)
	{
		Models[MODEL_FACE+i].Actions[0].PlaySpeed = 0.3f;
		Models[MODEL_FACE+i].Actions[1].PlaySpeed = 0.3f;
	}
	Models[MODEL_FACE+CLASS_SUMMONER].Actions[0].PlaySpeed = 0.25f;
}

void ReleaseCharacterSceneData()
{
	for (int i = BITMAP_LOG_IN; i <= BITMAP_LOG_IN_END; ++i)
		::DeleteBitmap(i);

	for(int i=BITMAP_TEMP;i<BITMAP_EXT_LOG_IN + 7;i++)
		DeleteBitmap(i);

	gMapManager.DeleteObjects();

	for(int i=0;i<MAX_CLASS;i++)
		Models[MODEL_FACE+i].Release();

	ClearCharacters();
}

void OpenBasicData(HDC hDC)
{
	CUIMng& rUIMng = CUIMng::Instance();

	rUIMng.RenderTitleSceneUI(hDC, 0, 11);

	LoadBitmap("Interface\\Cursor.tga"           ,BITMAP_CURSOR  ,GL_LINEAR,GL_CLAMP_TO_EDGE);
	LoadBitmap("Interface\\CursorPush.tga"       ,BITMAP_CURSOR+1,GL_LINEAR,GL_CLAMP_TO_EDGE);
	LoadBitmap("Interface\\CursorAttack.tga"     ,BITMAP_CURSOR+2,GL_LINEAR,GL_CLAMP_TO_EDGE);
	LoadBitmap("Interface\\CursorGet.tga"        ,BITMAP_CURSOR+3,GL_LINEAR,GL_CLAMP_TO_EDGE);
	LoadBitmap("Interface\\CursorTalk.tga"       ,BITMAP_CURSOR+4,GL_LINEAR,GL_CLAMP_TO_EDGE);
	LoadBitmap("Interface\\CursorRepair.tga"     ,BITMAP_CURSOR+5,GL_LINEAR,GL_CLAMP_TO_EDGE);
	LoadBitmap("Interface\\CursorLeanAgainst.tga",BITMAP_CURSOR+6,GL_LINEAR,GL_CLAMP_TO_EDGE);
	LoadBitmap("Interface\\CursorSitDown.tga"    ,BITMAP_CURSOR+7,GL_LINEAR,GL_CLAMP_TO_EDGE);
	LoadBitmap("Interface\\CursorDontMove.tga"   ,BITMAP_CURSOR+8,GL_LINEAR,GL_CLAMP_TO_EDGE);	
	//interface
	LoadBitmap("Interface\\ok.jpg"             ,BITMAP_INTERFACE+11);
	LoadBitmap("Interface\\ok2.jpg"            ,BITMAP_INTERFACE+12);
	LoadBitmap("Interface\\cancel.jpg"         ,BITMAP_INTERFACE+13);
	LoadBitmap("Interface\\cancel2.jpg"        ,BITMAP_INTERFACE+14);
	LoadBitmap("Interface\\win_titlebar.jpg"	 ,BITMAP_INTERFACE_EX+8);
	LoadBitmap("Interface\\win_button.tga"		 ,BITMAP_INTERFACE_EX+9);
	LoadBitmap("Interface\\win_size.jpg"		 ,BITMAP_INTERFACE_EX+10);
	LoadBitmap("Interface\\win_resize.tga"		 ,BITMAP_INTERFACE_EX+11);
	LoadBitmap("Interface\\win_scrollbar.jpg"	 ,BITMAP_INTERFACE_EX+12);
	LoadBitmap("Interface\\win_check.tga"		 ,BITMAP_INTERFACE_EX+13);
	LoadBitmap("Interface\\win_mail.tga"		 ,BITMAP_INTERFACE_EX+14);
	LoadBitmap("Interface\\win_mark.tga"		 ,BITMAP_INTERFACE_EX+15);
	LoadBitmap("Interface\\win_letter.jpg"	 ,BITMAP_INTERFACE_EX+16);
	LoadBitmap("Interface\\win_man.jpg"		 ,BITMAP_INTERFACE_EX+17);
	LoadBitmap("Interface\\win_push.jpg"		 ,BITMAP_INTERFACE_EX+18);
	LoadBitmap("Interface\\win_question.tga"	 ,BITMAP_INTERFACE_EX+20);
	LoadBitmap("Local\\Webzenlogo.jpg"	 ,BITMAP_INTERFACE_EX+22);

#ifdef DUEL_SYSTEM
	LoadWaveFile(SOUND_OPEN_DUELWINDOW,	"Data\\Sound\\iDuelWindow.wav", 1);
	LoadWaveFile(SOUND_START_DUEL,	"Data\\Sound\\iDuelStart.wav", 1);
#endif // DUEL_SYSTEM
	
	LoadBitmap("Interface\\CursorID.tga",	BITMAP_INTERFACE_EX+29); 
	LoadBitmap("Interface\\bar.jpg"			,BITMAP_INTERFACE+23);
	LoadBitmap("Interface\\back1.jpg"			,BITMAP_INTERFACE+24);
	LoadBitmap("Interface\\back2.jpg"			,BITMAP_INTERFACE+25);
	LoadBitmap("Interface\\back3.jpg"			,BITMAP_INTERFACE+26);

	LoadBitmap("Effect\\Fire01.jpg"		,BITMAP_FIRE      ,GL_LINEAR,GL_CLAMP_TO_EDGE);			// GM3rdChangeUp, GMCrywolf1st,GMHellas,Kanturu 3rd
	LoadBitmap("Effect\\Fire02.jpg"		,BITMAP_FIRE+1    ,GL_LINEAR,GL_CLAMP_TO_EDGE);			// GM3rdChangeUp, GMCrywolf1st,GMHellas,Kanturu 3rd
	LoadBitmap("Effect\\Fire03.jpg"		,BITMAP_FIRE+2    ,GL_LINEAR,GL_CLAMP_TO_EDGE);			// GM3rdChangeUp, GMCrywolf1st,GMHellas,Kanturu 3rd
	LoadBitmap("Effect\\PoundingBall.jpg"		,BITMAP_POUNDING_BALL,GL_LINEAR,GL_CLAMP_TO_EDGE); // Kanturu 2nd
	LoadBitmap( "Effect\\fi01.jpg"		,BITMAP_ADV_SMOKE,  GL_LINEAR, GL_CLAMP_TO_EDGE );		// GM 3rd ChangeUp, CryingWolf2nd
	LoadBitmap( "Effect\\fi02.tga"	    ,BITMAP_ADV_SMOKE+1,GL_LINEAR, GL_CLAMP_TO_EDGE );		// GM 3rd ChangeUp, CryingWolf2nd
	LoadBitmap( "Effect\\fantaF.jpg"		,BITMAP_TRUE_FIRE,  GL_LINEAR, GL_CLAMP_TO_EDGE );	// GM Aida, GMBattleCastle, ....
	LoadBitmap( "Effect\\fantaB.jpg"		,BITMAP_TRUE_BLUE,  GL_LINEAR, GL_CLAMP_TO_EDGE ); 
	LoadBitmap( "Effect\\JointSpirit02.jpg" ,BITMAP_JOINT_SPIRIT2  ,GL_LINEAR,GL_CLAMP_TO_EDGE);
	LoadBitmap( "Effect\\Piercing.jpg" , BITMAP_PIERCING, GL_LINEAR, GL_REPEAT );
	LoadBitmap( "Monster\\iui06.jpg",    BITMAP_ROBE+6 );
	LoadBitmap( "Effect\\Magic_b.jpg",   BITMAP_MAGIC_EMBLEM );
	LoadBitmap("Player\\dark3chima3.tga", BITMAP_DARKLOAD_SKIRT_3RD, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap( "Player\\kaa.tga",       BITMAP_DARK_LOAD_SKIRT,  GL_LINEAR, GL_CLAMP_TO_EDGE );
	LoadBitmap( "Effect\\ShockWave.jpg", BITMAP_SHOCK_WAVE, GL_LINEAR ); 
    LoadBitmap("Effect\\Flame01.jpg"		,BITMAP_FLAME     ,GL_LINEAR,GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\flare01.jpg"	    ,BITMAP_LIGHT      ,GL_LINEAR,GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\Magic_Ground1.jpg",BITMAP_MAGIC      ,GL_LINEAR,GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\Magic_Ground2.jpg",BITMAP_MAGIC+1    ,GL_LINEAR,GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\Magic_Circle1.jpg",BITMAP_MAGIC+2    ,GL_LINEAR);
#ifdef ASG_ADD_INFLUENCE_GROUND_EFFECT
	LoadBitmap("Effect\\guild_ring01.jpg", BITMAP_OUR_INFLUENCE_GROUND, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\enemy_ring02.jpg", BITMAP_ENEMY_INFLUENCE_GROUND, GL_LINEAR, GL_CLAMP_TO_EDGE);
#endif	// ASG_ADD_INFLUENCE_GROUND_EFFECT
	LoadBitmap("Effect\\Spark02.jpg"  	,BITMAP_SPARK     ,GL_LINEAR,GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\Spark03.jpg"  	,BITMAP_SPARK+1   ,GL_LINEAR,GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\smoke01.jpg"	    ,BITMAP_SMOKE     ,GL_LINEAR,GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\smoke02.tga"	    ,BITMAP_SMOKE+1   ,GL_LINEAR,GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\smoke05.tga"	    ,BITMAP_SMOKE+4   ,GL_LINEAR,GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\blood01.tga"      ,BITMAP_BLOOD     ,GL_LINEAR,GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\blood.tga"        ,BITMAP_BLOOD+1   ,GL_LINEAR,GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\Explotion01.jpg"  ,BITMAP_EXPLOTION ,GL_LINEAR,GL_CLAMP_TO_EDGE);
	LoadBitmap("Skill\\twlighthik01.jpg"  ,BITMAP_TWLIGHT ,GL_LINEAR,GL_CLAMP_TO_EDGE);
	LoadBitmap("Skill\\2line_gost.jpg"  ,BITMAP_2LINE_GHOST ,GL_LINEAR,GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\damage01mono.jpg", BITMAP_DAMAGE_01_MONO, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\SwordEff_mono.jpg", BITMAP_SWORD_EFFECT_MONO, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\flamestani.jpg", BITMAP_FLAMESTANI, GL_LINEAR, GL_CLAMP_TO_EDGE);

	LoadBitmap("Effect\\Spark.jpg", BITMAP_SPARK+2, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\firehik02.jpg"  ,BITMAP_FIRE_CURSEDLICH ,GL_LINEAR,GL_CLAMP_TO_EDGE);
	LoadBitmap("Monster\\totemgolem_leaf.tga",BITMAP_LEAF_TOTEMGOLEM ,GL_LINEAR,GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\empact01.jpg"  ,BITMAP_SUMMON_IMPACT ,GL_LINEAR,GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\loungexflow.jpg" ,BITMAP_SUMMON_SAHAMUTT_EXPLOSION ,GL_LINEAR,GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\gostmark01.jpg"  , BITMAP_DRAIN_LIFE_GHOST ,GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\mzine_typer2.jpg", BITMAP_MAGIC_ZIN, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\shiny05.jpg", BITMAP_SHINY+6, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\hikorora.jpg", BITMAP_ORORA, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\lightmarks.jpg", BITMAP_LIGHT_MARKS, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\cursorpin01.jpg"  , BITMAP_TARGET_POSITION_EFFECT1 ,GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\cursorpin02.jpg"  , BITMAP_TARGET_POSITION_EFFECT2 ,GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap( "Effect\\smokelines01.jpg" ,BITMAP_SMOKELINE1, GL_LINEAR, GL_REPEAT );
	LoadBitmap( "Effect\\smokelines02.jpg" ,BITMAP_SMOKELINE2, GL_LINEAR, GL_REPEAT );
	LoadBitmap( "Effect\\smokelines03.jpg" ,BITMAP_SMOKELINE3, GL_LINEAR, GL_REPEAT );
	LoadBitmap( "Effect\\lighting_mega01.jpg" ,BITMAP_LIGHTNING_MEGA1, GL_LINEAR, GL_REPEAT );
	LoadBitmap( "Effect\\lighting_mega02.jpg" ,BITMAP_LIGHTNING_MEGA2, GL_LINEAR, GL_REPEAT );
	LoadBitmap( "Effect\\lighting_mega03.jpg" ,BITMAP_LIGHTNING_MEGA3, GL_LINEAR, GL_REPEAT );
	LoadBitmap("Effect\\firehik01.jpg"  ,BITMAP_FIRE_HIK1 ,GL_LINEAR,GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\firehik03.jpg"  ,BITMAP_FIRE_HIK3 ,GL_LINEAR,GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\firehik_mono01.jpg", BITMAP_FIRE_HIK1_MONO, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\firehik_mono02.jpg", BITMAP_FIRE_HIK2_MONO, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\firehik_mono03.jpg", BITMAP_FIRE_HIK3_MONO, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\clouds3.jpg", BITMAP_RAKLION_CLOUDS, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Monster\\icenightlight.jpg", BITMAP_IRONKNIGHT_BODY_BRIGHT, GL_LINEAR, GL_CLAMP_TO_EDGE);

	g_XmasEvent->LoadXmasEventEffect();

	LoadBitmap("Skill\\younghtest1.tga", BITMAP_GM_HAIR_1);
	LoadBitmap("Skill\\younghtest3.tga", BITMAP_GM_HAIR_3);
	LoadBitmap("Skill\\gmmzine.jpg", BITMAP_GM_AURORA, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\explotion01mono.jpg"  ,BITMAP_EXPLOTION_MONO  ,GL_LINEAR,GL_CLAMP_TO_EDGE);
	LoadBitmap("Interface\\Success_kantru.tga" ,BITMAP_KANTURU_SUCCESS ,GL_LINEAR,GL_CLAMP_TO_EDGE);
	LoadBitmap("Interface\\Failure_kantru.tga" ,BITMAP_KANTURU_FAILED  ,GL_LINEAR,GL_CLAMP_TO_EDGE);
	LoadBitmap("Interface\\MonsterCount.tga"   ,BITMAP_KANTURU_COUNTER ,GL_LINEAR,GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\Clud64.jpg",	BITMAP_CLUD64,	GL_LINEAR,	GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\clouds.jpg"	  ,BITMAP_CLOUD,GL_LINEAR,GL_CLAMP_TO_EDGE);
	LoadBitmap("Skill\\SwordEff.jpg"	    ,BITMAP_BLUE_BLUR     ,GL_LINEAR,GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\Impack03.jpg"   ,BITMAP_IMPACT      ,GL_LINEAR,GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\ScolTail.jpg"  ,BITMAP_SCOLPION_TAIL  ,GL_LINEAR);
	LoadBitmap( "Effect\\FireSnuff.jpg"       , BITMAP_FIRE_SNUFF,    GL_LINEAR, GL_CLAMP_TO_EDGE );
	LoadBitmap("Effect\\coll.jpg"	,BITMAP_SPOT_WATER,GL_LINEAR, GL_CLAMP_TO_EDGE );
	LoadBitmap("Effect\\BowE.jpg"	,BITMAP_DS_EFFECT,GL_LINEAR, GL_CLAMP_TO_EDGE );
	LoadBitmap("Effect\\Shockwave.jpg"	,BITMAP_DS_SHOCK,GL_LINEAR, GL_CLAMP_TO_EDGE );
	LoadBitmap("Effect\\DinoE.jpg"  ,BITMAP_EXPLOTION+1,GL_LINEAR,GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\Shiny01.jpg"      ,BITMAP_SHINY     ,GL_LINEAR,GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\Shiny02.jpg"     	,BITMAP_SHINY+1   ,GL_LINEAR,GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\Shiny03.jpg"     	,BITMAP_SHINY+2   ,GL_LINEAR,GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\eye01.jpg"     	,BITMAP_SHINY+3   ,GL_LINEAR,GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\ring.jpg"     	,BITMAP_SHINY+4   ,GL_LINEAR,GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\Shiny04.jpg", BITMAP_SHINY+5, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\Chrome01.jpg"     ,BITMAP_CHROME     ,GL_LINEAR,GL_REPEAT);
	LoadBitmap("Effect\\blur01.jpg"       ,BITMAP_BLUR      ,GL_NEAREST,GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\bab2.jpg"          ,BITMAP_CHROME+1  ,GL_LINEAR,GL_REPEAT);
	LoadBitmap("Effect\\motion_blur.jpg"  ,BITMAP_BLUR+1     ,GL_NEAREST,GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\motion_blur_r.jpg",BITMAP_BLUR+2     ,GL_NEAREST,GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\motion_mono.jpg",	BITMAP_BLUR+3, GL_NEAREST,GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\motion_blur_r3.jpg",BITMAP_BLUR+6   ,GL_NEAREST,GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\gra.jpg",BITMAP_BLUR+7				,GL_NEAREST,GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\spinmark01.jpg",BITMAP_BLUR+8		,GL_NEAREST,GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\flamestani.jpg", BITMAP_BLUR+9		,GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\sword_blur.jpg", BITMAP_BLUR+10		,GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\joint_sword_red.jpg", BITMAP_BLUR+11,GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\motion_blur_r2.jpg", BITMAP_BLUR+12,GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\motion_blur_r3.jpg", BITMAP_BLUR+13,GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\blur02.jpg"       ,BITMAP_BLUR2      ,GL_NEAREST,GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\lightning2.jpg"	,BITMAP_LIGHTNING+1,GL_LINEAR,GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\Thunder01.jpg"     ,BITMAP_ENERGY        ,GL_LINEAR,GL_REPEAT);
	LoadBitmap("Effect\\Spark01.jpg"	     ,BITMAP_JOINT_SPARK   ,GL_LINEAR,GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\JointThunder01.jpg",BITMAP_JOINT_THUNDER ,GL_LINEAR,GL_REPEAT);
	LoadBitmap("Effect\\JointSpirit01.jpg" ,BITMAP_JOINT_SPIRIT  ,GL_LINEAR,GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\JointLaser01.jpg"  ,BITMAP_JOINT_ENERGY  ,GL_LINEAR);
	LoadBitmap("Effect\\JointEnergy01.jpg" ,BITMAP_JOINT_HEALING ,GL_LINEAR);
	LoadBitmap("Effect\\JointLaser02.jpg"  ,BITMAP_JOINT_LASER+1 ,GL_LINEAR,GL_REPEAT);
	LoadBitmap("Monster\\iui03.jpg"  ,BITMAP_JANUSEXT ,GL_LINEAR,GL_REPEAT);
	LoadBitmap("Monster\\magic_H.tga"  ,BITMAP_PHO_R_HAIR ,GL_LINEAR,GL_REPEAT);
	LoadBitmap("Item\\t_lower_14m.tga"  ,BITMAP_PANTS_G_SOUL ,GL_LINEAR,GL_REPEAT);
	LoadBitmap("Skill\\Skull.jpg"  ,BITMAP_SKULL,GL_NEAREST);
	LoadBitmap("Effect\\motion_blur_r2.jpg"  ,BITMAP_JOINT_FORCE ,GL_LINEAR,GL_REPEAT);
    LoadBitmap("Effect\\Fire04.jpg"   ,BITMAP_FIRECRACKER,	GL_LINEAR);
	LoadBitmap("Effect\\Flare.jpg"     	,BITMAP_FLARE,  GL_LINEAR,GL_CLAMP_TO_EDGE);

	LoadBitmap("Effect\\Chrome02.jpg"     ,BITMAP_CHROME2,GL_NEAREST,GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\Chrome03.jpg"     ,BITMAP_CHROME3,GL_NEAREST,GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\Chrome06.jpg"     ,BITMAP_CHROME6,GL_NEAREST,GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\Chrome07.jpg"     ,BITMAP_CHROME7,GL_NEAREST,GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\energy01.jpg"     ,BITMAP_CHROME_ENERGY,GL_NEAREST,GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\energy02.jpg"     ,BITMAP_CHROME_ENERGY2,GL_LINEAR,GL_CLAMP_TO_EDGE);

	LoadBitmap("Effect\\firecracker0001.jpg"	,BITMAP_FIRECRACKER0001, GL_NEAREST, GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\firecracker0002.jpg"	,BITMAP_FIRECRACKER0002, GL_NEAREST, GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\firecracker0003.jpg"	,BITMAP_FIRECRACKER0003, GL_NEAREST, GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\firecracker0004.jpg"	,BITMAP_FIRECRACKER0004, GL_NEAREST, GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\firecracker0005.jpg"	,BITMAP_FIRECRACKER0005, GL_NEAREST, GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\firecracker0006.jpg"	,BITMAP_FIRECRACKER0006, GL_NEAREST, GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\firecracker0007.jpg"	,BITMAP_FIRECRACKER0007, GL_NEAREST, GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\Shiny05.jpg"     	,BITMAP_SHINY+5   ,GL_LINEAR,GL_CLAMP_TO_EDGE);

	LoadBitmap("Effect\\partCharge1\\bujuckline.jpg", BITMAP_LUCKY_SEAL_EFFECT, GL_LINEAR, GL_CLAMP_TO_EDGE);

	LoadBitmap("Item\\nfm03.jpg"			,BITMAP_BLUECHROME,GL_NEAREST,GL_REPEAT);
	LoadBitmap("Effect\\flareBlue.jpg"     	,BITMAP_FLARE_BLUE,  GL_LINEAR,GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\NSkill.jpg"     	,BITMAP_FLARE_FORCE,  GL_LINEAR,GL_CLAMP_TO_EDGE);
    LoadBitmap("Effect\\Flare02.jpg"      ,BITMAP_FLARE+1, GL_LINEAR,GL_REPEAT);
	LoadBitmap("Monster\\King11.jpg", BITMAP_WHITE_WIZARD, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Monster\\Kni000.jpg", BITMAP_DEST_ORC_WAR0, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Monster\\Kni011.jpg", BITMAP_DEST_ORC_WAR1, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Monster\\Kni022.jpg", BITMAP_DEST_ORC_WAR2, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap( "Skill\\pinkWave.jpg"     	, BITMAP_PINK_WAVE,     GL_LINEAR, GL_CLAMP_TO_EDGE );
	LoadBitmap( "Effect\\flareRed.jpg"     	, BITMAP_FLARE_RED,     GL_LINEAR, GL_CLAMP_TO_EDGE );
	LoadBitmap( "Effect\\Fire05.jpg"		    , BITMAP_FIRE+3,        GL_LINEAR, GL_CLAMP_TO_EDGE );
	LoadBitmap( "Effect\\Hole.jpg"           , BITMAP_HOLE,          GL_LINEAR, GL_REPEAT );
//	LoadBitmap( "Monster\\mop011.jpg"        , BITMAP_OTHER_SKIN,    GL_LINEAR, GL_REPEAT );
	LoadBitmap( "Effect\\WATERFALL1.jpg"		, BITMAP_WATERFALL_1,   GL_LINEAR, GL_CLAMP_TO_EDGE );
	LoadBitmap( "Effect\\WATERFALL2.jpg"		, BITMAP_WATERFALL_2,   GL_LINEAR, GL_CLAMP_TO_EDGE );
	LoadBitmap( "Effect\\WATERFALL3.jpg"	    , BITMAP_WATERFALL_3,   GL_LINEAR, GL_CLAMP_TO_EDGE );
	LoadBitmap( "Effect\\WATERFALL4.jpg"	    , BITMAP_WATERFALL_4,   GL_LINEAR, GL_CLAMP_TO_EDGE );
	LoadBitmap( "Effect\\WATERFALL5.jpg"		, BITMAP_WATERFALL_5,   GL_LINEAR, GL_CLAMP_TO_EDGE );
	/*
	LoadBitmap("Interface\\in_bar.tga"		, BITMAP_MVP_INTERFACE, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Interface\\in_bar2.jpg"		, BITMAP_MVP_INTERFACE + 1, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Interface\\in_deco.tga"		, BITMAP_MVP_INTERFACE + 2, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Interface\\in_main.tga"		, BITMAP_MVP_INTERFACE + 3, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Interface\\in_main_icon_bal1.tga"		, BITMAP_MVP_INTERFACE + 4, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Interface\\in_main_icon_dl1.tga"		, BITMAP_MVP_INTERFACE + 5, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Interface\\in_main_icon_dl2.tga"		, BITMAP_MVP_INTERFACE + 6, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Interface\\in_main_number1.tga"		, BITMAP_MVP_INTERFACE + 7, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Interface\\in_main_number2.tga"		, BITMAP_MVP_INTERFACE + 8, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Interface\\in_main2.tga"		, BITMAP_MVP_INTERFACE + 9, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Interface\\icon_failure.tga"		, BITMAP_MVP_INTERFACE + 10, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Interface\\icon_success.tga"		, BITMAP_MVP_INTERFACE + 11, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Interface\\t_main.tga"		, BITMAP_MVP_INTERFACE + 12, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Interface\\m_b_no1.tga"		, BITMAP_MVP_INTERFACE + 13, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Interface\\m_b_no2.tga"		, BITMAP_MVP_INTERFACE + 14, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Interface\\m_b_no3.tga"		, BITMAP_MVP_INTERFACE + 15, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Interface\\m_b_ok1.tga"		, BITMAP_MVP_INTERFACE + 16, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Interface\\m_b_ok2.tga"		, BITMAP_MVP_INTERFACE + 17, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Interface\\m_b_ok3.tga"		, BITMAP_MVP_INTERFACE + 18, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Interface\\m_b_yes1.tga"		, BITMAP_MVP_INTERFACE + 19, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Interface\\m_b_yes2.tga"		, BITMAP_MVP_INTERFACE + 20, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Interface\\m_b_yes3.tga"		, BITMAP_MVP_INTERFACE + 21, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Interface\\m_main.tga"		, BITMAP_MVP_INTERFACE + 22, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Interface\\in_main_number1_1.tga"		, BITMAP_MVP_INTERFACE + 23, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Interface\\in_main_number2_1.tga"		, BITMAP_MVP_INTERFACE + 24, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Interface\\in_main_number0_2.tga"		, BITMAP_MVP_INTERFACE + 25, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Interface\\icon_Rank_table.tga"		, BITMAP_MVP_INTERFACE + 26, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Interface\\icon_Rank_rank.tga"		, BITMAP_MVP_INTERFACE + 27, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Interface\\icon_Rank_D.tga"		, BITMAP_MVP_INTERFACE + 28, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Interface\\icon_Rank_C.tga"		, BITMAP_MVP_INTERFACE + 29, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Interface\\icon_Rank_B.tga"		, BITMAP_MVP_INTERFACE + 30, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Interface\\icon_Rank_A.tga"		, BITMAP_MVP_INTERFACE + 31, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Interface\\icon_Rank_S.tga"		, BITMAP_MVP_INTERFACE + 32, GL_LINEAR, GL_CLAMP_TO_EDGE);
	//icon_Rank_0
	LoadBitmap("Interface\\icon_Rank_0.tga"		, BITMAP_MVP_INTERFACE + 33, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Interface\\icon_Rank_1.tga"		, BITMAP_MVP_INTERFACE + 34, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Interface\\icon_Rank_2.tga"		, BITMAP_MVP_INTERFACE + 35, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Interface\\icon_Rank_3.tga"		, BITMAP_MVP_INTERFACE + 36, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Interface\\icon_Rank_4.tga"		, BITMAP_MVP_INTERFACE + 37, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Interface\\icon_Rank_5.tga"		, BITMAP_MVP_INTERFACE + 38, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Interface\\icon_Rank_6.tga"		, BITMAP_MVP_INTERFACE + 39, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Interface\\icon_Rank_7.tga"		, BITMAP_MVP_INTERFACE + 40, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Interface\\icon_Rank_8.tga"		, BITMAP_MVP_INTERFACE + 41, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Interface\\icon_Rank_9.tga"		, BITMAP_MVP_INTERFACE + 42, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Interface\\icon_Rank_exp.tga"		, BITMAP_MVP_INTERFACE + 43, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Interface\\m_main_rank.tga"		, BITMAP_MVP_INTERFACE + 44, GL_LINEAR, GL_CLAMP_TO_EDGE);
	*/
	//m_main_rank
	//icon_Rank_exp
	LoadBitmap( "Interface\\BattleSkill.tga"	, BITMAP_INTERFACE_EX+34 );
	LoadBitmap( "Effect\\Flashing.jpg"        , BITMAP_FLASH, GL_LINEAR, GL_CLAMP_TO_EDGE );
	LoadBitmap( "Interface\\senatusmap.jpg"	, BITMAP_INTERFACE_EX+35 );
	LoadBitmap( "Interface\\gate_button2.jpg"	, BITMAP_INTERFACE_EX+36 ); 
	LoadBitmap( "Interface\\gate_button1.jpg"	, BITMAP_INTERFACE_EX+37 );
	LoadBitmap( "Interface\\suho_button2.jpg"	, BITMAP_INTERFACE_EX+38 );
	LoadBitmap( "Interface\\suho_button1.jpg"	, BITMAP_INTERFACE_EX+39 );
	LoadBitmap( "Interface\\DoorCL.jpg"	    , BITMAP_INTERFACE_EX+40 ); 
	LoadBitmap( "Interface\\DoorOP.jpg"	    , BITMAP_INTERFACE_EX+41 );
    //OpenJpeg( "Effect\\FireSnuff.jpg"       , BITMAP_FIRE_SNUFF,    GL_LINEAR, GL_CLAMP_TO_EDGE );
	LoadBitmap( "Object31\\Flag.tga"          , BITMAP_INTERFACE_MAP+0, GL_LINEAR, GL_CLAMP_TO_EDGE );
	LoadBitmap( "Interface\\CursorAttack2.tga", BITMAP_CURSOR2 );
	LoadBitmap( "Effect\\Cratered.tga"	    , BITMAP_CRATER ); 
	LoadBitmap( "Effect\\FormationMark.tga"	, BITMAP_FORMATION_MARK );
    LoadBitmap( "Effect\\Plus.tga"	        , BITMAP_PLUS );
	LoadBitmap("Effect\\eff_lighting.jpg" , BITMAP_FENRIR_THUNDER);
	LoadBitmap("Effect\\eff_lightinga01.jpg" , BITMAP_FENRIR_FOOT_THUNDER1);
	LoadBitmap("Effect\\eff_lightinga02.jpg" , BITMAP_FENRIR_FOOT_THUNDER2);
	LoadBitmap("Effect\\eff_lightinga03.jpg" , BITMAP_FENRIR_FOOT_THUNDER3);
	LoadBitmap("Effect\\eff_lightinga04.jpg" , BITMAP_FENRIR_FOOT_THUNDER4);
	LoadBitmap("Effect\\eff_lightinga05.jpg" , BITMAP_FENRIR_FOOT_THUNDER5);
	LoadBitmap( "Interface\\Progress_Back.jpg",BITMAP_INTERFACE_EX+42 );
	LoadBitmap( "Interface\\Progress.jpg"     ,BITMAP_INTERFACE_EX+43 );
	LoadBitmap( "Effect\\Inferno.jpg"         , BITMAP_INFERNO,       GL_LINEAR, GL_CLAMP_TO_EDGE );
	LoadBitmap( "Effect\\Lava.jpg"            , BITMAP_LAVA,          GL_LINEAR, GL_CLAMP_TO_EDGE );
	LoadBitmap( "Interface\\frame.tga",       BITMAP_INTERFACE_MAP+3 ); 
	LoadBitmap( "Interface\\i_attack.tga",    BITMAP_INTERFACE_MAP+4 );
	LoadBitmap( "Interface\\i_defense.tga",   BITMAP_INTERFACE_MAP+5 );
	LoadBitmap( "Interface\\i_wait.tga",      BITMAP_INTERFACE_MAP+6 );
	LoadBitmap( "Interface\\b_command01.jpg", BITMAP_INTERFACE_MAP+8 );
	LoadBitmap( "Interface\\b_command02.jpg", BITMAP_INTERFACE_MAP+9 );
	LoadBitmap( "Interface\\b_group02.jpg",   BITMAP_INTERFACE_MAP+10);
	LoadBitmap( "Interface\\b_zoomout01.jpg", BITMAP_INTERFACE_MAP+11);
	LoadBitmap( "Interface\\hourglass.tga",   BITMAP_INTERFACE_MAP+7 );
	LoadBitmap( "Interface\\dot.tga",         BITMAP_INTERFACE_EX+44 );
	LoadBitmap("Object9\\Impack03.jpg"   ,BITMAP_LIGHT+1      ,GL_LINEAR,GL_CLAMP_TO_EDGE);
	LoadBitmap("Monster\\buserbody_r.jpg", BITMAP_BERSERK_EFFECT, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Monster\\busersword_r.jpg", BITMAP_BERSERK_WP_EFFECT, GL_LINEAR, GL_CLAMP_TO_EDGE);
//	LoadBitmap("Monster\\gigantiscorn_R.jpg", BITMAP_GIGANTIS_EFFECT, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Monster\\bladeeff2_r.jpg", BITMAP_BLADEHUNTER_EFFECT, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Monster\\illumi_R.jpg", BITMAP_TWINTAIL_EFFECT, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Monster\\prsona_R.jpg", BITMAP_PRSONA_EFFECT, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Monster\\prsonass_R.jpg", BITMAP_PRSONA_EFFECT2, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("effect\\water.jpg", BITMAP_TWINTAIL_WATER, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\cra_04.jpg"   ,BITMAP_LIGHT+2      ,GL_LINEAR,GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\Impack01.jpg"		,BITMAP_LIGHT+3,	GL_LINEAR,GL_CLAMP_TO_EDGE);
	::LoadBitmap("Interface\\message_ok_b_all.tga", BITMAP_BUTTON);
	::LoadBitmap("Interface\\loding_cancel_b_all.tga", BITMAP_BUTTON+1);
	::LoadBitmap("Interface\\message_close_b_all.tga", BITMAP_BUTTON+2);
	::LoadBitmap("Interface\\message_back.tga", BITMAP_MESSAGE_WIN);
	::LoadBitmap("Interface\\delete_secret_number.tga", BITMAP_MSG_WIN_INPUT);
	::LoadBitmap("Interface\\op1_stone.jpg", BITMAP_SYS_WIN, GL_NEAREST, GL_REPEAT);
	::LoadBitmap("Interface\\op1_back1.tga", BITMAP_SYS_WIN+1);
	::LoadBitmap("Interface\\op1_back2.tga", BITMAP_SYS_WIN+2);
	::LoadBitmap("Interface\\op1_back3.jpg", BITMAP_SYS_WIN+3, GL_NEAREST, GL_REPEAT);
	::LoadBitmap("Interface\\op1_back4.jpg", BITMAP_SYS_WIN+4, GL_NEAREST, GL_REPEAT);
	::LoadBitmap("Interface\\op1_b_all.tga", BITMAP_TEXT_BTN);
	::LoadBitmap("Interface\\op2_back1.tga", BITMAP_OPTION_WIN);
	::LoadBitmap("Interface\\op2_ch.tga", BITMAP_CHECK_BTN);
	::LoadBitmap("Interface\\op2_volume3.tga", BITMAP_SLIDER);
	::LoadBitmap("Interface\\op2_volume2.jpg", BITMAP_SLIDER+1, GL_NEAREST, GL_REPEAT);
	::LoadBitmap("Interface\\op2_volume1.tga", BITMAP_SLIDER+2);
	::LoadBitmap("Effect\\clouds2.jpg", BITMAP_EVENT_CLOUD, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\pin_lights.jpg", BITMAP_PIN_LIGHT, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Monster\\deasuler_cloth.tga", BITMAP_DEASULER_CLOTH, GL_LINEAR, GL_CLAMP_TO_EDGE);

	LoadBitmap("Item\\soketmagic_stape02.jpg", BITMAP_SOCKETSTAFF, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\lightmarks.jpg", BITMAP_LIGHTMARKS, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Effect\\lightmarks.jpg", BITMAP_LIGHTMARKS_FOREIGN, GL_LINEAR, GL_CLAMP_TO_EDGE);

	rUIMng.RenderTitleSceneUI(hDC, 1, 11);

	::LoadBitmap("Item\\partCharge1\\entrance_R.jpg", BITMAP_FREETICKET_R, GL_LINEAR, GL_CLAMP_TO_EDGE);
	::LoadBitmap("Item\\partCharge1\\juju_R.jpg", BITMAP_CHAOSCARD_R, GL_LINEAR, GL_CLAMP_TO_EDGE);

	::LoadBitmap("Item\\partCharge1\\monmark01a.jpg", BITMAP_LUCKY_SEAL_EFFECT43, GL_LINEAR, GL_CLAMP_TO_EDGE);
	::LoadBitmap("Item\\partCharge1\\monmark02a.jpg", BITMAP_LUCKY_SEAL_EFFECT44, GL_LINEAR, GL_CLAMP_TO_EDGE);
	::LoadBitmap("Item\\partCharge1\\monmark03a.jpg", BITMAP_LUCKY_SEAL_EFFECT45, GL_LINEAR, GL_CLAMP_TO_EDGE);

	::LoadBitmap("Item\\partCharge1\\bujuck01alpa.jpg", BITMAP_LUCKY_CHARM_EFFECT53, GL_LINEAR, GL_CLAMP_TO_EDGE);
	::LoadBitmap("Item\\partCharge1\\expensiveitem01_R.jpg", BITMAP_RAREITEM1_R, GL_LINEAR, GL_CLAMP_TO_EDGE);
	::LoadBitmap("Item\\partCharge1\\expensiveitem02a_R.jpg", BITMAP_RAREITEM2_R, GL_LINEAR, GL_CLAMP_TO_EDGE);
	::LoadBitmap("Item\\partCharge1\\expensiveitem02b_R.jpg", BITMAP_RAREITEM3_R, GL_LINEAR, GL_CLAMP_TO_EDGE);
	::LoadBitmap("Item\\partCharge1\\expensiveitem03a_R.jpg", BITMAP_RAREITEM4_R, GL_LINEAR, GL_CLAMP_TO_EDGE);
	::LoadBitmap("Item\\partCharge1\\expensiveitem03b_R.jpg", BITMAP_RAREITEM5_R, GL_LINEAR, GL_CLAMP_TO_EDGE);

	::LoadBitmap("Item\\partCharge3\\alicecard_R.tga", BITMAP_CHARACTERCARD_R, GL_LINEAR, GL_CLAMP_TO_EDGE);

	::LoadBitmap("Item\\Ingameshop\\kacama_R.jpg", BITMAP_CHARACTERCARD_R_MA, GL_LINEAR, GL_CLAMP_TO_EDGE);
	::LoadBitmap("Item\\Ingameshop\\kacada_R.jpg", BITMAP_CHARACTERCARD_R_DA, GL_LINEAR, GL_CLAMP_TO_EDGE);

	::LoadBitmap("Item\\partCharge3\\jujug_R.jpg", BITMAP_NEWCHAOSCARD_GOLD_R, GL_LINEAR, GL_CLAMP_TO_EDGE);
	::LoadBitmap("Item\\partCharge3\\jujul_R.jpg", BITMAP_NEWCHAOSCARD_RARE_R, GL_LINEAR, GL_CLAMP_TO_EDGE);
	::LoadBitmap("Item\\partCharge3\\jujum_R.jpg", BITMAP_NEWCHAOSCARD_MINI_R, GL_LINEAR, GL_CLAMP_TO_EDGE);
     
	::LoadBitmap("Effect\\cherryblossom\\sakuras01.jpg", BITMAP_CHERRYBLOSSOM_EVENT_PETAL, GL_LINEAR, GL_CLAMP_TO_EDGE);
	::LoadBitmap("Effect\\cherryblossom\\sakuras02.jpg", BITMAP_CHERRYBLOSSOM_EVENT_FLOWER, GL_LINEAR, GL_CLAMP_TO_EDGE);

	::LoadBitmap("Object39\\k_effect_01.JPG", BITMAP_KANTURU_2ND_EFFECT1, GL_LINEAR, GL_CLAMP_TO_EDGE);
	::LoadBitmap("Item\\deathbeamstone_R.jpg", BITMAP_ITEM_EFFECT_DBSTONE_R, GL_LINEAR, GL_CLAMP_TO_EDGE);
	::LoadBitmap("Item\\hellhorn_R.jpg", BITMAP_ITEM_EFFECT_HELLHORN_R, GL_LINEAR, GL_CLAMP_TO_EDGE);
	::LoadBitmap("Item\\phoenixfeather_R.tga", BITMAP_ITEM_EFFECT_PFEATHER_R, GL_LINEAR, GL_CLAMP_TO_EDGE);
	::LoadBitmap("Item\\Deye_R.jpg", BITMAP_ITEM_EFFECT_DEYE_R, GL_LINEAR, GL_CLAMP_TO_EDGE);
	::LoadBitmap("Item\\wing3chaking2.jpg", BITMAP_ITEM_NIGHT_3RDWING_R, GL_LINEAR, GL_CLAMP_TO_EDGE);

	LoadBitmap("NPC\\lumi.jpg", BITMAP_CURSEDTEMPLE_NPC_MESH_EFFECT, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("item\\songko2_R.jpg", BITMAP_CURSEDTEMPLE_HOLYITEM_MESH_EFFECT, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("skill\\unitedsoldier_wing.tga", BITMAP_CURSEDTEMPLE_ALLIED_PHYSICSCLOTH);
	LoadBitmap("skill\\illusionistcloth.tga", BITMAP_CURSEDTEMPLE_ILLUSION_PHYSICSCLOTH);
	LoadBitmap("effect\\masker.jpg", BITMAP_CURSEDTEMPLE_EFFECT_MASKER, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("effect\\wind01.jpg",BITMAP_CURSEDTEMPLE_EFFECT_WIND, GL_LINEAR, GL_CLAMP_TO_EDGE );
	::LoadBitmap("Interface\\menu01_new2_SD.jpg" ,BITMAP_INTERFACE_EX+46);

#ifdef ASG_ADD_GENS_SYSTEM
	std::string strFileName = "Local\\" + g_strSelectedML + "\\ImgsMapName\\MapNameAddStrife.tga";
	::LoadBitmap(strFileName.c_str(), BITMAP_INTERFACE_EX+47);
#endif	// ASG_ADD_GENS_SYSTEM

#ifdef ASG_ADD_GENS_MARK
	::LoadBitmap("Interface\\Gens_mark_D_new.tga", BITMAP_GENS_MARK_DUPRIAN);
	::LoadBitmap("Interface\\Gens_mark_V_new.tga", BITMAP_GENS_MARK_BARNERT);
#endif	// ASG_ADD_GENS_MARK

	LoadBitmap("Monster\\serufanarm_R.jpg", BITMAP_SERUFAN_ARM_R, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Monster\\serufanwand_R.jpg", BITMAP_SERUFAN_WAND_R, GL_LINEAR, GL_CLAMP_TO_EDGE);

	LoadBitmap("npc\\santa.jpg", BITMAP_GOOD_SANTA, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("npc\\santa_baggage.jpg", BITMAP_GOOD_SANTA_BAGGAGE, GL_LINEAR, GL_CLAMP_TO_EDGE);

#ifdef PJH_ADD_PANDA_CHANGERING	
		LoadBitmap("Item\\pandabody_R.jpg", BITMAP_PANDABODY_R, GL_LINEAR, GL_LINEAR);	// OPENGL_ERROR
#endif //PJH_ADD_PANDA_CHANGERING

		LoadBitmap("Monster\\DGicewalker_body.jpg", BITMAP_DOPPELGANGER_ICEWALKER0, GL_LINEAR, GL_LINEAR);	// OPENGL_ERROR
		LoadBitmap("Monster\\DGicewalker_R.jpg", BITMAP_DOPPELGANGER_ICEWALKER1, GL_LINEAR, GL_LINEAR);	// OPENGL_ERROR
		LoadBitmap("Monster\\Snake1.jpg", BITMAP_DOPPELGANGER_SNAKE01, GL_LINEAR, GL_LINEAR);	// OPENGL_ERROR

		LoadBitmap("NPC\\goldboit.jpg", BITMAP_DOPPELGANGER_GOLDENBOX1, GL_LINEAR, GL_LINEAR);	// OPENGL_ERROR
		LoadBitmap("NPC\\goldline.jpg", BITMAP_DOPPELGANGER_GOLDENBOX2, GL_LINEAR, GL_LINEAR);	// OPENGL_ERROR

	//BITMAP_LIGHT_RED
	LoadBitmap("effect\\flare01_red.jpg", BITMAP_LIGHT_RED, GL_LINEAR, GL_CLAMP_TO_EDGE );

	LoadBitmap("effect\\gra.jpg", BITMAP_GRA, GL_LINEAR, GL_CLAMP_TO_EDGE );
	LoadBitmap("effect\\ring_of_gradation.jpg", BITMAP_RING_OF_GRADATION, GL_LINEAR, GL_CLAMP_TO_EDGE );

	LoadBitmap("Interface\\InGameShop\\ingame_pack_check.tga", BITMAP_IGS_CHECK_BUTTON, GL_LINEAR);
	LoadBitmap("Monster\\AssassinLeader_body_R.jpg"	, BITMAP_ASSASSIN_EFFECT1, GL_LINEAR, GL_CLAMP_TO_EDGE);

	LoadBitmap("Item\\partCharge8\\rareitem_ticket_7_body.jpg", BITMAP_RAREITEM7, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Item\\partCharge8\\rareitem_ticket_8_body.jpg", BITMAP_RAREITEM8, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Item\\partCharge8\\rareitem_ticket_9_body.jpg", BITMAP_RAREITEM9, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Item\\partCharge8\\rareitem_ticket_10_body.jpg", BITMAP_RAREITEM10, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Item\\partCharge8\\rareitem_ticket_11_body.jpg", BITMAP_RAREITEM11, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Item\\partCharge8\\rareitem_ticket_12_body.jpg", BITMAP_RAREITEM12, GL_LINEAR, GL_CLAMP_TO_EDGE);
	
	LoadBitmap("Item\\partCharge8\\DoppelCard.jpg", BITMAP_DOPPLEGANGGER_FREETICKET, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Item\\partCharge8\\BarcaCard.jpg", BITMAP_BARCA_FREETICKET, GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Item\\partCharge8\\Barca7Card.jpg", BITMAP_BARCA7TH_FREETICKET, GL_LINEAR, GL_CLAMP_TO_EDGE);
	
#ifdef LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM_PART_2
	LoadBitmap("Item\\ork_cham_R.jpg",				BITMAP_ORK_CHAM_LAYER_R,			GL_LINEAR, GL_CLAMP_TO_EDGE);
	//LoadBitmap("Item\\maple_cham_R.jpg",			BITMAP_MAPLE_CHAM_LAYER_R,			GL_LINEAR, GL_CLAMP_TO_EDGE);
	LoadBitmap("Item\\goldenork_cham_R.jpg",		BITMAP_GOLDEN_ORK_CHAM_LAYER_R,		GL_LINEAR, GL_CLAMP_TO_EDGE);
	//LoadBitmap("Item\\goldenmaple_cham_R.jpg",	BITMAP_GOLDEN_MAPLE_CHAM_LAYER_R,	GL_LINEAR, GL_CLAMP_TO_EDGE);
#endif //LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM_PART_2

#ifdef ASG_ADD_KARUTAN_MONSTERS
	::LoadBitmap("Monster\\BoneSE.jpg", BITMAP_BONE_SCORPION_SKIN_EFFECT, GL_LINEAR, GL_CLAMP_TO_EDGE);
	::LoadBitmap("Monster\\KryptaBall2.jpg", BITMAP_KRYPTA_BALL_EFFECT, GL_LINEAR, GL_CLAMP_TO_EDGE);
	::LoadBitmap("Monster\\bora_golem_effect.jpg", BITMAP_CONDRA_SKIN_EFFECT, GL_LINEAR, GL_CLAMP_TO_EDGE);
	::LoadBitmap("Monster\\bora_golem2_effect.jpg", BITMAP_CONDRA_SKIN_EFFECT2, GL_LINEAR, GL_CLAMP_TO_EDGE);
	::LoadBitmap("Monster\\king_golem01_effect.jpg", BITMAP_NARCONDRA_SKIN_EFFECT, GL_LINEAR, GL_CLAMP_TO_EDGE);
	::LoadBitmap("Monster\\king_golem02_effect.jpg", BITMAP_NARCONDRA_SKIN_EFFECT2, GL_LINEAR, GL_CLAMP_TO_EDGE);
	::LoadBitmap("Monster\\king_golem03_effect.jpg", BITMAP_NARCONDRA_SKIN_EFFECT3, GL_LINEAR, GL_CLAMP_TO_EDGE);
#endif	// ASG_ADD_KARUTAN_MONSTERS

#ifdef ASG_ADD_KARUTAN_NPC
	::LoadBitmap("NPC\\voloE.jpg", BITMAP_VOLO_SKIN_EFFECT, GL_LINEAR, GL_CLAMP_TO_EDGE);
#endif	// ASG_ADD_KARUTAN_NPC

	g_ErrorReport.Write( "> First Load Files OK.\r\n");

	OpenPlayers();


	rUIMng.RenderTitleSceneUI(hDC, 2, 11);

    OpenPlayerTextures();
	rUIMng.RenderTitleSceneUI(hDC, 3, 11);

    OpenItems();
	rUIMng.RenderTitleSceneUI(hDC, 4, 11);

    OpenItemTextures();
	rUIMng.RenderTitleSceneUI(hDC, 5, 11);

    OpenSkills();
	rUIMng.RenderTitleSceneUI(hDC, 6, 11);

	OpenImages();
	rUIMng.RenderTitleSceneUI(hDC, 7, 11);

    OpenSounds();
	rUIMng.RenderTitleSceneUI(hDC, 8, 11);

	char Text[100];

	g_ServerListManager->LoadServerListScript();

	sprintf(Text, "Data\\Local\\%s\\Dialog_%s.bmd", g_strSelectedML.c_str(), g_strSelectedML.c_str());
	OpenDialogFile(Text);

	sprintf(Text, "Data\\Local\\%s\\Item_%s.bmd", g_strSelectedML.c_str(), g_strSelectedML.c_str());
	OpenItemScript(Text);

	sprintf(Text, "Data\\Local\\%s\\movereq_%s.bmd", g_strSelectedML.c_str(), g_strSelectedML.c_str());
	SEASON3B::CMoveCommandData::OpenMoveReqScript(Text);
	
	sprintf(Text, "Data\\Local\\%s\\NpcName_%s.txt", g_strSelectedML.c_str(), g_strSelectedML.c_str());
   	OpenMonsterScript(Text);
	
	sprintf(Text, "Data\\Local\\%s\\Quest_%s.bmd", g_strSelectedML.c_str(), g_strSelectedML.c_str());
	g_csQuest.OpenQuestScript (Text);

	sprintf(Text, "Data\\Local\\%s\\Skill_%s.bmd", g_strSelectedML.c_str(), g_strSelectedML.c_str());
	OpenSkillScript(Text);

	sprintf(Text, "Data\\Local\\%s\\SocketItem_%s.bmd", g_strSelectedML.c_str(), g_strSelectedML.c_str());
	g_SocketItemMgr.OpenSocketItemScript(Text);

	OpenTextData();		//. Text.bmd, Testtest.bmd

	g_csItemOption.OpenItemSetScript ( false );

	g_QuestMng.LoadQuestScript();

	OpenGateScript ("Data\\Gate.bmd");

	OpenFilterFile ("Data\\Local\\Filter.bmd");

	OpenNameFilterFile ("Data\\Local\\FilterName.bmd");

	OpenMonsterSkillScript ("Data\\Local\\MonsterSkill.bmd");

	g_pMasterLevelInterface->OpenMasterLevel("Data\\Local\\MasterSKillTree.bmd");

	rUIMng.RenderTitleSceneUI(hDC, 9, 11);

	LoadWaveFile(SOUND_TITLE01			,"Data\\Sound\\iTitle.wav",1); 
	LoadWaveFile(SOUND_MENU01			,"Data\\Sound\\iButtonMove.wav",2); 
	LoadWaveFile(SOUND_CLICK01			,"Data\\Sound\\iButtonClick.wav",1); 
	LoadWaveFile(SOUND_ERROR01			,"Data\\Sound\\iButtonError.wav",1); 
	LoadWaveFile(SOUND_INTERFACE01		,"Data\\Sound\\iCreateWindow.wav",1);

    LoadWaveFile(SOUND_REPAIR           ,"Data\\Sound\\iRepair.wav",1);
    LoadWaveFile(SOUND_WHISPER          ,"Data\\Sound\\iWhisper.wav",1);

	LoadWaveFile(SOUND_FRIEND_CHAT_ALERT ,"Data\\Sound\\iFMSGAlert.wav",1);
	LoadWaveFile(SOUND_FRIEND_MAIL_ALERT ,"Data\\Sound\\iFMailAlert.wav",1);
	LoadWaveFile(SOUND_FRIEND_LOGIN_ALERT,"Data\\Sound\\iFLogInAlert.wav",1);

	LoadWaveFile(SOUND_RING_EVENT_READY ,"Data\\Sound\\iEvent3min.wav",1);
	LoadWaveFile(SOUND_RING_EVENT_START ,"Data\\Sound\\iEventStart.wav",1);
	LoadWaveFile(SOUND_RING_EVENT_END   ,"Data\\Sound\\iEventEnd.wav",1);

	rUIMng.RenderTitleSceneUI(hDC, 10, 11);
}

void OpenTextData()
{
	char Text[100];

	sprintf(Text,"Data\\Local\\%s\\Text_%s.bmd", g_strSelectedML.c_str(), g_strSelectedML.c_str());
	GlobalText.Load(Text, CGlobalText::LD_USA_CANADA_TEXTS|CGlobalText::LD_FOREIGN_TEXTS);
	OpenMacro("Data\\Macro.txt");
}

void ReleaseMainData()
{
	gMapManager.DeleteObjects();
	DeleteNpcs();
	DeleteMonsters();
	ClearItems();
	ClearCharacters();
}
