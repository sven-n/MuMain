#include "stdafx.h"
#include "UIControls.h"
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

#include "SMD.h"

#include "PhysicsManager.h"
#include "CSQuest.h"
#include "CSItemOption.h"
#include "MoveCommandData.h"
#include "UIMng.h"
#include "LoadData.h"
#include "MapManager.h"
#include "Event.h"
#include "ChangeRingManager.h"
#include "NewUISystem.h"
#include "CameraMove.h"
#include "QuestMng.h"
#include "ServerListManager.h"
#include "MonkSystem.h"
#include "SocketSystem.h"

///////////////////////////////////////////
extern BOOL g_bUseChatListBox;
///////////////////////////////////////////

bool Flip = false;

void OpenModel(int Type, wchar_t* Dir, wchar_t* ModelFileName, ...)
{
    wchar_t ModelName[100];
    wchar_t AnimationName[20][100];
    wcscpy(ModelName, Dir);
    wcscat(ModelName, ModelFileName);

    va_list Marker;
    va_start(Marker, ModelFileName);
    int AnimationCount = 0;
    while (1)
    {
        wchar_t* Temp = va_arg(Marker, wchar_t*);
        if (Temp == NULL || wcscmp(Temp, L"end") == NULL)
        {
            break;
        }
        else
        {
            wcscpy(AnimationName[AnimationCount], Dir);
            wcscat(AnimationName[AnimationCount], Temp);
            AnimationCount++;
        }
    }
    if (AnimationCount == 0)
    {
        OpenSMDModel(Type, ModelName, 1, Flip);
        OpenSMDAnimation(Type, ModelName);
    }
    else
    {
        OpenSMDModel(Type, ModelName, AnimationCount, Flip);
        for (int i = 0; i < AnimationCount; i++)
        {
            bool Walk = false;
            if (i == MONSTER01_WALK || i == MONSTER01_RUN) Walk = true;
            OpenSMDAnimation(Type, AnimationName[i], Walk);
        }
    }
    va_end(Marker);

    Flip = false;
}

void OpenModels(int Model, wchar_t* FileName, int i)
{
    wchar_t Name[64];
    if (i < 10)
        swprintf(Name, L"%s0%d.smd", FileName, i);
    else
        swprintf(Name, L"%s%d.smd", FileName, i);
    OpenSMDModel(Model, Name);
    OpenSMDAnimation(Model, Name);
}

void OpenPlayers()
{
    ModelsDump = new BMD[MAX_MODELS + 1024];
    Models = ModelsDump + (rand() % 1024);
    ZeroMemory(Models, MAX_MODELS * sizeof(BMD));

    gLoadData.AccessModel(MODEL_PLAYER, L"Data\\Player\\", L"Player");

    if (Models[MODEL_PLAYER].NumMeshs > 0)
    {
        g_ErrorReport.Write(L"Player.bmd file error.\r\n");
        MessageBox(g_hWnd, L"Player.bmd file error!!!", NULL, MB_OK);
        SendMessage(g_hWnd, WM_DESTROY, 0, 0);
    }

    for (int i = 0; i < MAX_CLASS; ++i)
    {
        gLoadData.AccessModel(MODEL_BODY_HELM + i, L"Data\\Player\\", L"HelmClass", i + 1);
        gLoadData.AccessModel(MODEL_BODY_ARMOR + i, L"Data\\Player\\", L"ArmorClass", i + 1);
        gLoadData.AccessModel(MODEL_BODY_PANTS + i, L"Data\\Player\\", L"PantClass", i + 1);
        gLoadData.AccessModel(MODEL_BODY_GLOVES + i, L"Data\\Player\\", L"GloveClass", i + 1);
        gLoadData.AccessModel(MODEL_BODY_BOOTS + i, L"Data\\Player\\", L"BootClass", i + 1);

        if (CLASS_DARK != i && CLASS_DARK_LORD != i && CLASS_RAGEFIGHTER != i)
        {
            gLoadData.AccessModel(MODEL_BODY_HELM + MAX_CLASS + i, L"Data\\Player\\", L"HelmClass2", i + 1);
            gLoadData.AccessModel(MODEL_BODY_ARMOR + MAX_CLASS + i, L"Data\\Player\\", L"ArmorClass2", i + 1);
            gLoadData.AccessModel(MODEL_BODY_PANTS + MAX_CLASS + i, L"Data\\Player\\", L"PantClass2", i + 1);
            gLoadData.AccessModel(MODEL_BODY_GLOVES + MAX_CLASS + i, L"Data\\Player\\", L"GloveClass2", i + 1);
            gLoadData.AccessModel(MODEL_BODY_BOOTS + MAX_CLASS + i, L"Data\\Player\\", L"BootClass2", i + 1);
        }

        gLoadData.AccessModel(MODEL_BODY_HELM + (MAX_CLASS * 2) + i, L"Data\\Player\\", L"HelmClass3", i + 1);
        gLoadData.AccessModel(MODEL_BODY_ARMOR + (MAX_CLASS * 2) + i, L"Data\\Player\\", L"ArmorClass3", i + 1);
        gLoadData.AccessModel(MODEL_BODY_PANTS + (MAX_CLASS * 2) + i, L"Data\\Player\\", L"PantClass3", i + 1);
        gLoadData.AccessModel(MODEL_BODY_GLOVES + (MAX_CLASS * 2) + i, L"Data\\Player\\", L"GloveClass3", i + 1);
        gLoadData.AccessModel(MODEL_BODY_BOOTS + (MAX_CLASS * 2) + i, L"Data\\Player\\", L"BootClass3", i + 1);
    }

    for (int i = 0; i < 10; i++)
    {
        gLoadData.AccessModel(MODEL_HELM + i, L"Data\\Player\\", L"HelmMale", i + 1);
        gLoadData.AccessModel(MODEL_ARMOR + i, L"Data\\Player\\", L"ArmorMale", i + 1);
        gLoadData.AccessModel(MODEL_PANTS + i, L"Data\\Player\\", L"PantMale", i + 1);
        gLoadData.AccessModel(MODEL_GLOVES + i, L"Data\\Player\\", L"GloveMale", i + 1);
        gLoadData.AccessModel(MODEL_BOOTS + i, L"Data\\Player\\", L"BootMale", i + 1);
    }

    for (int i = 0; i < 5; i++)
    {
        gLoadData.AccessModel(MODEL_HELM + i + 10, L"Data\\Player\\", L"HelmElf", i + 1);
        gLoadData.AccessModel(MODEL_ARMOR + i + 10, L"Data\\Player\\", L"ArmorElf", i + 1);
        gLoadData.AccessModel(MODEL_PANTS + i + 10, L"Data\\Player\\", L"PantElf", i + 1);
        gLoadData.AccessModel(MODEL_GLOVES + i + 10, L"Data\\Player\\", L"GloveElf", i + 1);
        gLoadData.AccessModel(MODEL_BOOTS + i + 10, L"Data\\Player\\", L"BootElf", i + 1);
    }
    gLoadData.AccessModel(MODEL_STORM_CROW_ARMOR, L"Data\\Player\\", L"ArmorMale", 16);
    gLoadData.AccessModel(MODEL_STORM_CROW_PANTS, L"Data\\Player\\", L"PantMale", 16);
    gLoadData.AccessModel(MODEL_STORM_CROW_GLOVES, L"Data\\Player\\", L"GloveMale", 16);
    gLoadData.AccessModel(MODEL_STORM_CROW_BOOTS, L"Data\\Player\\", L"BootMale", 16);

    gLoadData.AccessModel(MODEL_BLACK_DRAGON_HELM, L"Data\\Player\\", L"HelmMale", 17);
    gLoadData.AccessModel(MODEL_BLACK_DRAGON_ARMOR, L"Data\\Player\\", L"ArmorMale", 17);
    gLoadData.AccessModel(MODEL_BLACK_DRAGON_PANTS, L"Data\\Player\\", L"PantMale", 17);
    gLoadData.AccessModel(MODEL_BLACK_DRAGON_GLOVES, L"Data\\Player\\", L"GloveMale", 17);
    gLoadData.AccessModel(MODEL_BLACK_DRAGON_BOOTS, L"Data\\Player\\", L"BootMale", 17);

    gLoadData.AccessModel(MODEL_MASK_HELM + 0, L"Data\\Player\\", L"MaskHelmMale", 1);
    gLoadData.AccessModel(MODEL_MASK_HELM + 5, L"Data\\Player\\", L"MaskHelmMale", 6);
    gLoadData.AccessModel(MODEL_MASK_HELM + 6, L"Data\\Player\\", L"MaskHelmMale", 7);
    gLoadData.AccessModel(MODEL_MASK_HELM + 8, L"Data\\Player\\", L"MaskHelmMale", 9);
    gLoadData.AccessModel(MODEL_MASK_HELM + 9, L"Data\\Player\\", L"MaskHelmMale", 10);

    for (int i = 0; i < 4; i++)
    {
        if (18 + i == 20)
        {
            gLoadData.AccessModel(MODEL_DARK_PHOENIX_HELM + i, L"Data\\Player\\", L"HelmMaleTest", 18 + i);
            gLoadData.AccessModel(MODEL_DARK_PHOENIX_ARMOR + i, L"Data\\Player\\", L"ArmorMaleTest", 18 + i);
            gLoadData.AccessModel(MODEL_DARK_PHOENIX_PANTS + i, L"Data\\Player\\", L"PantMaleTest", 18 + i);
            gLoadData.AccessModel(MODEL_DARK_PHOENIX_GLOVES + i, L"Data\\Player\\", L"GloveMaleTest", 18 + i);
            gLoadData.AccessModel(MODEL_DARK_PHOENIX_BOOTS + i, L"Data\\Player\\", L"BootMaleTest", 18 + i);
        }
        else
        {
            if (i < 3)
            {
                gLoadData.AccessModel(MODEL_DARK_PHOENIX_HELM + i, L"Data\\Player\\", L"HelmMale", 18 + i);
            }
            gLoadData.AccessModel(MODEL_DARK_PHOENIX_ARMOR + i, L"Data\\Player\\", L"ArmorMale", 18 + i);
            if (18 + i == 19)
            {
                gLoadData.AccessModel(MODEL_DARK_PHOENIX_PANTS + i, L"Data\\Player\\", L"t_PantMale", 18 + i);
            }
            else
            {
                gLoadData.AccessModel(MODEL_DARK_PHOENIX_PANTS + i, L"Data\\Player\\", L"PantMale", 18 + i);
            }
            gLoadData.AccessModel(MODEL_DARK_PHOENIX_GLOVES + i, L"Data\\Player\\", L"GloveMale", 18 + i);
            gLoadData.AccessModel(MODEL_DARK_PHOENIX_BOOTS + i, L"Data\\Player\\", L"BootMale", 18 + i);
        }
    }

    for (int i = 0; i < 4; i++)
    {
        if (i != 2)
        {
            gLoadData.AccessModel(MODEL_GREAT_DRAGON_HELM + i, L"Data\\Player\\", L"HelmMale", 22 + i);
        }
        gLoadData.AccessModel(MODEL_GREAT_DRAGON_ARMOR + i, L"Data\\Player\\", L"ArmorMale", 22 + i);
        gLoadData.AccessModel(MODEL_GREAT_DRAGON_PANTS + i, L"Data\\Player\\", L"PantMale", 22 + i);
        gLoadData.AccessModel(MODEL_GREAT_DRAGON_GLOVES + i, L"Data\\Player\\", L"GloveMale", 22 + i);
        gLoadData.AccessModel(MODEL_GREAT_DRAGON_BOOTS + i, L"Data\\Player\\", L"BootMale", 22 + i);
    }

    for (int i = 0; i < 4; i++)
    {
        gLoadData.AccessModel(MODEL_LIGHT_PLATE_MASK + i, L"Data\\Player\\", L"HelmMale", 26 + i);
        gLoadData.AccessModel(MODEL_LIGHT_PLATE_ARMOR + i, L"Data\\Player\\", L"ArmorMale", 26 + i);
        gLoadData.AccessModel(MODEL_LIGHT_PLATE_PANTS + i, L"Data\\Player\\", L"PantMale", 26 + i);
        gLoadData.AccessModel(MODEL_LIGHT_PLATE_GLOVES + i, L"Data\\Player\\", L"GloveMale", 26 + i);
        gLoadData.AccessModel(MODEL_LIGHT_PLATE_BOOTS + i, L"Data\\Player\\", L"BootMale", 26 + i);
    }

    for (int i = 0; i < 5; ++i)
    {
        gLoadData.AccessModel(MODEL_DRAGON_KNIGHT_ARMOR + i, L"Data\\Player\\", L"HDK_ArmorMale", i + 1);
        gLoadData.AccessModel(MODEL_DRAGON_KNIGHT_PANTS + i, L"Data\\Player\\", L"HDK_PantMale", i + 1);
        gLoadData.AccessModel(MODEL_DRAGON_KNIGHT_GLOVES + i, L"Data\\Player\\", L"HDK_GloveMale", i + 1);
        gLoadData.AccessModel(MODEL_DRAGON_KNIGHT_BOOTS + i, L"Data\\Player\\", L"HDK_BootMale", i + 1);
    }

    gLoadData.AccessModel(MODEL_DRAGON_KNIGHT_HELM, L"Data\\Player\\", L"HDK_HelmMale", 1);
    gLoadData.AccessModel(MODEL_VENOM_MIST_HELM, L"Data\\Player\\", L"HDK_HelmMale", 2);
    gLoadData.AccessModel(MODEL_SYLPHID_RAY_HELM, L"Data\\Player\\", L"HDK_HelmMale", 3);
    gLoadData.AccessModel(MODEL_SUNLIGHT_MASK, L"Data\\Player\\", L"HDK_HelmMale", 5);

    for (int i = 0; i < 5; ++i)
    {
        gLoadData.AccessModel(MODEL_ASHCROW_ARMOR + i, L"Data\\Player\\", L"CW_ArmorMale", i + 1);
        gLoadData.AccessModel(MODEL_ASHCROW_PANTS + i, L"Data\\Player\\", L"CW_PantMale", i + 1);
        gLoadData.AccessModel(MODEL_ASHCROW_GLOVES + i, L"Data\\Player\\", L"CW_GloveMale", i + 1);
        gLoadData.AccessModel(MODEL_ASHCROW_BOOTS + i, L"Data\\Player\\", L"CW_BootMale", i + 1);
    }

    //마검사는 제외하고 투구도 추가
    gLoadData.AccessModel(MODEL_ASHCROW_HELM, L"Data\\Player\\", L"CW_HelmMale", 1);
    gLoadData.AccessModel(MODEL_ECLIPSE_HELM, L"Data\\Player\\", L"CW_HelmMale", 2);
    gLoadData.AccessModel(MODEL_IRIS_HELM, L"Data\\Player\\", L"CW_HelmMale", 3);
    gLoadData.AccessModel(MODEL_GLORIOUS_MASK, L"Data\\Player\\", L"CW_HelmMale", 5);

    for (int i = 0; i < 6; ++i)
    {
        gLoadData.AccessModel(MODEL_MISTERY_HELM + i, L"Data\\Player\\", L"HelmMale", 40 + i);
        gLoadData.AccessModel(MODEL_MISTERY_ARMOR + i, L"Data\\Player\\", L"ArmorMale", 40 + i);
        gLoadData.AccessModel(MODEL_MISTERY_PANTS + i, L"Data\\Player\\", L"PantMale", 40 + i);
        gLoadData.AccessModel(MODEL_MISTERY_GLOVES + i, L"Data\\Player\\", L"GloveMale", 40 + i);
        gLoadData.AccessModel(MODEL_MISTERY_BOOTS + i, L"Data\\Player\\", L"BootMale", 40 + i);
    }

    for (int i = 0; i < MODEL_ITEM_COMMON_NUM; ++i)
    {
        gLoadData.AccessModel(MODEL_HELM2 + i, L"Data\\Player\\", L"HelmElfC", i + 1);
        gLoadData.AccessModel(MODEL_ARMOR2 + i, L"Data\\Player\\", L"ArmorElfC", i + 1);
        gLoadData.AccessModel(MODEL_PANTS2 + i, L"Data\\Player\\", L"PantElfC", i + 1);
        gLoadData.AccessModel(MODEL_GLOVES2 + i, L"Data\\Player\\", L"GloveElfC", i + 1);
        gLoadData.AccessModel(MODEL_BOOTS2 + i, L"Data\\Player\\", L"BootElfC", i + 1);
    }

    for (int i = 45; i <= 53; i++)
    {
        if (i == 47 || i == 48)
            continue;
        gLoadData.AccessModel(MODEL_HELM + i, L"Data\\Player\\", L"HelmMale", i + 1);
    } // for()

    for (int i = 45; i <= 53; i++)
    {
        gLoadData.AccessModel(MODEL_ARMOR + i, L"Data\\Player\\", L"ArmorMale", i + 1);
        gLoadData.AccessModel(MODEL_PANTS + i, L"Data\\Player\\", L"PantMale", i + 1);
        gLoadData.AccessModel(MODEL_GLOVES + i, L"Data\\Player\\", L"GloveMale", i + 1);
        gLoadData.AccessModel(MODEL_BOOTS + i, L"Data\\Player\\", L"BootMale", i + 1);
    } // for()

    for (int i = 0; i < MODEL_ITEM_COMMONCNT_RAGEFIGHTER; ++i)
    {
        gLoadData.AccessModel(MODEL_HELM_MONK + i, L"Data\\Player\\", L"HelmMonk", i + 1);
        gLoadData.AccessModel(MODEL_ARMOR_MONK + i, L"Data\\Player\\", L"ArmorMonk", i + 1);
        gLoadData.AccessModel(MODEL_PANTS_MONK + i, L"Data\\Player\\", L"PantMonk", i + 1);
        gLoadData.AccessModel(MODEL_BOOTS_MONK + i, L"Data\\Player\\", L"BootMonk", i + 1);
    }

    for (int i = 0; i < 3; ++i)
    {
        gLoadData.AccessModel(MODEL_SACRED_HELM + i, L"Data\\Player\\", L"HelmMale", 60 + i);
        gLoadData.AccessModel(MODEL_SACRED_ARMOR + i, L"Data\\Player\\", L"ArmorMale", 60 + i);
        gLoadData.AccessModel(MODEL_SACRED_PANTS + i, L"Data\\Player\\", L"PantMale", 60 + i);
        gLoadData.AccessModel(MODEL_SACRED_BOOTS + i, L"Data\\Player\\", L"BootMale", 60 + i);
    }

    gLoadData.AccessModel(MODEL_PHOENIX_SOUL_HELMET, L"Data\\Player\\", L"HelmMale74", -1);
    gLoadData.AccessModel(MODEL_PHOENIX_SOUL_ARMOR, L"Data\\Player\\", L"ArmorMale74", -1);
    gLoadData.AccessModel(MODEL_PHOENIX_SOUL_PANTS, L"Data\\Player\\", L"PantMale74", -1);
    gLoadData.AccessModel(MODEL_PHOENIX_SOUL_BOOTS, L"Data\\Player\\", L"BootMale74", -1);

    {
        auto* pCloth = new CPhysicsClothMesh[1];
        pCloth[0].Create(&Hero->Object, 2, 17, 0.0f, 0.0f, 0.0f, 5, 8, 45.0f, 85.0f, BITMAP_PANTS_G_SOUL, BITMAP_PANTS_G_SOUL, PCT_MASK_ALPHA | PCT_HEAVY | PCT_STICKED, MODEL_GRAND_SOUL_PANTS);
        delete[] pCloth;
    }

    for (int i = 0; i < 1; i++)
        gLoadData.AccessModel(MODEL_SHADOW_BODY + i, L"Data\\Player\\", L"Shadow", i + 1);

    Models[MODEL_PLAYER].BoneHead = 20;
    Models[MODEL_PLAYER].BoneFoot[0] = 6;
    Models[MODEL_PLAYER].BoneFoot[1] = 13;

    for (int i = PLAYER_STOP_MALE; i <= PLAYER_STOP_RIDE_WEAPON; i++)
        Models[MODEL_PLAYER].Actions[i].PlaySpeed = 0.28f;

    Models[MODEL_PLAYER].Actions[PLAYER_STOP_SWORD].PlaySpeed = 0.26f;
    Models[MODEL_PLAYER].Actions[PLAYER_STOP_TWO_HAND_SWORD].PlaySpeed = 0.24f;
    Models[MODEL_PLAYER].Actions[PLAYER_STOP_SPEAR].PlaySpeed = 0.24f;
    Models[MODEL_PLAYER].Actions[PLAYER_STOP_BOW].PlaySpeed = 0.22f;
    Models[MODEL_PLAYER].Actions[PLAYER_STOP_CROSSBOW].PlaySpeed = 0.22f;
    Models[MODEL_PLAYER].Actions[PLAYER_STOP_SUMMONER].PlaySpeed = 0.24f;
    Models[MODEL_PLAYER].Actions[PLAYER_STOP_WAND].PlaySpeed = 0.30f;

    for (int i = PLAYER_WALK_MALE; i <= PLAYER_RUN_RIDE_WEAPON; i++)
        Models[MODEL_PLAYER].Actions[i].PlaySpeed = 0.3f;

    Models[MODEL_PLAYER].Actions[PLAYER_WALK_WAND].PlaySpeed = 0.44f;
    Models[MODEL_PLAYER].Actions[PLAYER_RUN_WAND].PlaySpeed = 0.76f;
    Models[MODEL_PLAYER].Actions[PLAYER_WALK_SWIM].PlaySpeed = 0.35f;
    Models[MODEL_PLAYER].Actions[PLAYER_RUN_SWIM].PlaySpeed = 0.35f;

    for (int i = PLAYER_DEFENSE1; i <= PLAYER_SHOCK; i++)
        Models[MODEL_PLAYER].Actions[i].PlaySpeed = 0.32f;

    for (int i = PLAYER_DIE1; i <= PLAYER_DIE2; i++)
        Models[MODEL_PLAYER].Actions[i].PlaySpeed = 0.45f;

    for (int i = PLAYER_SIT1; i < MAX_PLAYER_ACTION; i++)
        Models[MODEL_PLAYER].Actions[i].PlaySpeed = 0.4f;

    Models[MODEL_PLAYER].Actions[PLAYER_SHOCK].PlaySpeed = 0.4f;
    Models[MODEL_PLAYER].Actions[PLAYER_SEE1].PlaySpeed = 0.28f;
    Models[MODEL_PLAYER].Actions[PLAYER_SEE_FEMALE1].PlaySpeed = 0.28f;
    Models[MODEL_PLAYER].Actions[PLAYER_HEALING1].PlaySpeed = 0.2f;
    Models[MODEL_PLAYER].Actions[PLAYER_HEALING_FEMALE1].PlaySpeed = 0.2f;

    Models[MODEL_PLAYER].Actions[PLAYER_JACK_1].PlaySpeed = 0.38f;
    Models[MODEL_PLAYER].Actions[PLAYER_JACK_2].PlaySpeed = 0.38f;

    Models[MODEL_PLAYER].Actions[PLAYER_SANTA_1].PlaySpeed = 0.34f;
    Models[MODEL_PLAYER].Actions[PLAYER_SANTA_2].PlaySpeed = 0.30f;

    Models[MODEL_PLAYER].Actions[PLAYER_SKILL_RIDER].PlaySpeed = 0.2f;
    Models[MODEL_PLAYER].Actions[PLAYER_SKILL_RIDER_FLY].PlaySpeed = 0.2f;

    Models[MODEL_PLAYER].Actions[PLAYER_STOP_TWO_HAND_SWORD_TWO].PlaySpeed = 0.24f;
    Models[MODEL_PLAYER].Actions[PLAYER_WALK_TWO_HAND_SWORD_TWO].PlaySpeed = 0.3f;
    Models[MODEL_PLAYER].Actions[PLAYER_RUN_TWO_HAND_SWORD_TWO].PlaySpeed = 0.3f;
    Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_TWO_HAND_SWORD_TWO].PlaySpeed = 0.24f;

    Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_DEATHSTAB].PlaySpeed = 0.45f;

    Models[MODEL_PLAYER].Actions[PLAYER_DIE1].Loop = true;
    Models[MODEL_PLAYER].Actions[PLAYER_DIE2].Loop = true;
    Models[MODEL_PLAYER].Actions[PLAYER_COME_UP].Loop = true;

    Models[MODEL_PLAYER].Actions[PLAYER_SKILL_HELL_BEGIN].Loop = true;

    Models[MODEL_PLAYER].Actions[PLAYER_DARKLORD_STAND].PlaySpeed = 0.3f;
    Models[MODEL_PLAYER].Actions[PLAYER_DARKLORD_WALK].PlaySpeed = 0.3f;
    Models[MODEL_PLAYER].Actions[PLAYER_STOP_RIDE_HORSE].PlaySpeed = 0.3f;
    Models[MODEL_PLAYER].Actions[PLAYER_RUN_RIDE_HORSE].PlaySpeed = 0.3f;
    Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_STRIKE].PlaySpeed = 0.2f;
    Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_TELEPORT].PlaySpeed = 0.28f;

    Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_RIDE_STRIKE].PlaySpeed = 0.3f;
    Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_RIDE_TELEPORT].PlaySpeed = 0.3f;
    Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_RIDE_HORSE_SWORD].PlaySpeed = 0.28f;
    Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_RIDE_ATTACK_FLASH].PlaySpeed = 0.3f;
    Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_RIDE_ATTACK_MAGIC].PlaySpeed = 0.3f;
    Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_DARKHORSE].PlaySpeed = 0.2f;

    Models[MODEL_PLAYER].Actions[PLAYER_IDLE1_DARKHORSE].PlaySpeed = 1.0f;
    Models[MODEL_PLAYER].Actions[PLAYER_IDLE2_DARKHORSE].PlaySpeed = 1.0f;

    for (int i = PLAYER_FENRIR_ATTACK; i <= PLAYER_FENRIR_WALK_ONE_LEFT; i++)
    {
        Models[MODEL_PLAYER].Actions[i].PlaySpeed = 0.45f;
    }

    for (int i = PLAYER_FENRIR_RUN; i <= PLAYER_FENRIR_RUN_ONE_LEFT_ELF; i++)
    {
        Models[MODEL_PLAYER].Actions[i].PlaySpeed = 0.71f;
    }

    for (int i = PLAYER_FENRIR_STAND; i <= PLAYER_FENRIR_STAND_ONE_LEFT; i++)
    {
        Models[MODEL_PLAYER].Actions[i].PlaySpeed = 0.4f;
    }

    Models[MODEL_PLAYER].Actions[PLAYER_FENRIR_ATTACK_MAGIC].PlaySpeed = 0.30f;
    Models[MODEL_PLAYER].Actions[PLAYER_FENRIR_ATTACK_DARKLORD_STRIKE].PlaySpeed = 0.3f;
    Models[MODEL_PLAYER].Actions[PLAYER_FENRIR_ATTACK_DARKLORD_TELEPORT].PlaySpeed = 0.3f;
    Models[MODEL_PLAYER].Actions[PLAYER_FENRIR_ATTACK_DARKLORD_SWORD].PlaySpeed = 0.28f;
    Models[MODEL_PLAYER].Actions[PLAYER_FENRIR_ATTACK_DARKLORD_FLASH].PlaySpeed = 0.3f;

    Models[MODEL_PLAYER].Actions[PLAYER_HIGH_SHOCK].PlaySpeed = 0.3f;

    for (int i = PLAYER_RAGE_FENRIR; i <= PLAYER_RAGE_FENRIR_ATTACK_RIGHT; i++)
    {
        if (i >= PLAYER_RAGE_FENRIR_TWO_SWORD && i <= PLAYER_RAGE_FENRIR_ONE_LEFT)
            Models[MODEL_PLAYER].Actions[i].PlaySpeed = 0.225f;
        else
            Models[MODEL_PLAYER].Actions[i].PlaySpeed = 0.45f;
    }
    for (int i = PLAYER_RAGE_FENRIR_STAND_TWO_SWORD; i <= PLAYER_RAGE_FENRIR_STAND_ONE_LEFT; i++)
    {
        Models[MODEL_PLAYER].Actions[i].PlaySpeed = 0.2f;
    }
    Models[MODEL_PLAYER].Actions[PLAYER_RAGE_FENRIR_STAND].PlaySpeed = 0.21f;

    for (int i = PLAYER_RAGE_FENRIR_RUN; i <= PLAYER_RAGE_FENRIR_RUN_ONE_LEFT; i++)
    {
        Models[MODEL_PLAYER].Actions[i].PlaySpeed = 0.355f;
    }
    Models[MODEL_PLAYER].Actions[PLAYER_RAGE_UNI_RUN].PlaySpeed = 0.3f;
    Models[MODEL_PLAYER].Actions[PLAYER_RAGE_UNI_ATTACK_ONE_RIGHT].PlaySpeed = 0.2f;
    Models[MODEL_PLAYER].Actions[PLAYER_RAGE_UNI_STOP_ONE_RIGHT].PlaySpeed = 0.18f;
    Models[MODEL_PLAYER].Actions[PLAYER_STOP_RAGEFIGHTER].PlaySpeed = 0.16f;
    SetAttackSpeed();

    gLoadData.AccessModel(MODEL_GM_CHARACTER, L"Data\\Skill\\", L"youngza");
}

void OpenPlayerTextures()
{
    LoadBitmap(L"Player\\hair_r.jpg", BITMAP_HAIR);
    LoadBitmap(L"Player\\Robe01.jpg", BITMAP_ROBE);
    LoadBitmap(L"Player\\Robe02.jpg", BITMAP_ROBE + 1);
    LoadBitmap(L"Player\\Robe03.tga", BITMAP_ROBE + 2);
    LoadBitmap(L"Player\\DarklordRobe.tga", BITMAP_ROBE + 7);
    LoadBitmap(L"Item\\msword03.tga", BITMAP_ROBE + 8);
    LoadBitmap(L"Item\\dl_redwings02.tga", BITMAP_ROBE + 9);
    LoadBitmap(L"Item\\dl_redwings03.tga", BITMAP_ROBE + 10);

    int nIndex;

    for (int j = 0; j < MAX_CLASS_STAGES; ++j)
    {
        for (int i = 0; i < MAX_CLASS; ++i)
        {
            nIndex = MAX_CLASS * j + i;
            if (1 == j && (CLASS_DARK == i || CLASS_DARK_LORD == i || CLASS_RAGEFIGHTER == i))
                continue;

            gLoadData.OpenTexture(MODEL_BODY_HELM + nIndex, L"Player\\");
            gLoadData.OpenTexture(MODEL_BODY_ARMOR + nIndex, L"Player\\");
            gLoadData.OpenTexture(MODEL_BODY_PANTS + nIndex, L"Player\\");
            gLoadData.OpenTexture(MODEL_BODY_GLOVES + nIndex, L"Player\\");
            gLoadData.OpenTexture(MODEL_BODY_BOOTS + nIndex, L"Player\\");
        }
    }

    for (int i = 0; i <= CLASS_END; i++)
    {
        gLoadData.OpenTexture(MODEL_HELM + i, L"Player\\");
        gLoadData.OpenTexture(MODEL_ARMOR + i, L"Player\\");
        gLoadData.OpenTexture(MODEL_PANTS + i, L"Player\\");
        gLoadData.OpenTexture(MODEL_GLOVES + i, L"Player\\");
        gLoadData.OpenTexture(MODEL_BOOTS + i, L"Player\\");
    }

    for (int i = 0; i < 4; i++)
    {
        if (i != 2)
        {
            gLoadData.OpenTexture(MODEL_GREAT_DRAGON_HELM + i, L"Player\\");
        }
        gLoadData.OpenTexture(MODEL_GREAT_DRAGON_ARMOR + i, L"Player\\");
        gLoadData.OpenTexture(MODEL_GREAT_DRAGON_PANTS + i, L"Player\\");
        gLoadData.OpenTexture(MODEL_GREAT_DRAGON_GLOVES + i, L"Player\\");
        gLoadData.OpenTexture(MODEL_GREAT_DRAGON_BOOTS + i, L"Player\\");
    }

    for (int i = 0; i < 4; i++)
    {
        gLoadData.OpenTexture(MODEL_LIGHT_PLATE_MASK + i, L"Player\\");
        gLoadData.OpenTexture(MODEL_LIGHT_PLATE_ARMOR + i, L"Player\\");
        gLoadData.OpenTexture(MODEL_LIGHT_PLATE_PANTS + i, L"Player\\");
        gLoadData.OpenTexture(MODEL_LIGHT_PLATE_GLOVES + i, L"Player\\");
        gLoadData.OpenTexture(MODEL_LIGHT_PLATE_BOOTS + i, L"Player\\");
    }

    for (int i = 0; i < 4; i++)
    {
        gLoadData.OpenTexture(MODEL_DARK_PHOENIX_HELM + i, L"Player\\");
        gLoadData.OpenTexture(MODEL_DARK_PHOENIX_ARMOR + i, L"Player\\");
        gLoadData.OpenTexture(MODEL_DARK_PHOENIX_PANTS + i, L"Player\\");
        gLoadData.OpenTexture(MODEL_DARK_PHOENIX_GLOVES + i, L"Player\\");
        gLoadData.OpenTexture(MODEL_DARK_PHOENIX_BOOTS + i, L"Player\\");
    }

    gLoadData.OpenTexture(MODEL_MASK_HELM + 0, L"Player\\");
    gLoadData.OpenTexture(MODEL_MASK_HELM + 5, L"Player\\");
    gLoadData.OpenTexture(MODEL_MASK_HELM + 6, L"Player\\");
    gLoadData.OpenTexture(MODEL_MASK_HELM + 8, L"Player\\");
    gLoadData.OpenTexture(MODEL_MASK_HELM + 9, L"Player\\");

    gLoadData.OpenTexture(MODEL_SHADOW_BODY, L"Player\\");


    for (int i = 0; i < 5; ++i)
    {
        gLoadData.OpenTexture(MODEL_DRAGON_KNIGHT_ARMOR + i, L"Player\\");
        gLoadData.OpenTexture(MODEL_DRAGON_KNIGHT_PANTS + i, L"Player\\");
        gLoadData.OpenTexture(MODEL_DRAGON_KNIGHT_GLOVES + i, L"Player\\");
        gLoadData.OpenTexture(MODEL_DRAGON_KNIGHT_BOOTS + i, L"Player\\");
    }
    gLoadData.OpenTexture(MODEL_DRAGON_KNIGHT_HELM, L"Player\\");
    gLoadData.OpenTexture(MODEL_VENOM_MIST_HELM, L"Player\\");
    gLoadData.OpenTexture(MODEL_SYLPHID_RAY_HELM, L"Player\\");
    gLoadData.OpenTexture(MODEL_SUNLIGHT_MASK, L"Player\\");

    for (int i = 0; i < 5; ++i)
    {
        gLoadData.OpenTexture(MODEL_ASHCROW_ARMOR + i, L"Player\\");
        gLoadData.OpenTexture(MODEL_ASHCROW_PANTS + i, L"Player\\");
        gLoadData.OpenTexture(MODEL_ASHCROW_GLOVES + i, L"Player\\");
        gLoadData.OpenTexture(MODEL_ASHCROW_BOOTS + i, L"Player\\");
    }
    gLoadData.OpenTexture(MODEL_ASHCROW_HELM, L"Player\\");
    gLoadData.OpenTexture(MODEL_ECLIPSE_HELM, L"Player\\");
    gLoadData.OpenTexture(MODEL_IRIS_HELM, L"Player\\");
    gLoadData.OpenTexture(MODEL_GLORIOUS_MASK, L"Player\\");

    wchar_t szFileName[64];

    for (int i = 0; i < 6; ++i)
    {
        gLoadData.OpenTexture(MODEL_MISTERY_HELM + i, L"Player\\");
        gLoadData.OpenTexture(MODEL_MISTERY_ARMOR + i, L"Player\\");
        gLoadData.OpenTexture(MODEL_MISTERY_PANTS + i, L"Player\\");
        gLoadData.OpenTexture(MODEL_MISTERY_GLOVES + i, L"Player\\");
        gLoadData.OpenTexture(MODEL_MISTERY_BOOTS + i, L"Player\\");

        ::swprintf(szFileName, L"Player\\InvenArmorMale%d.tga", 40 + i);
        ::LoadBitmap(szFileName, BITMAP_INVEN_ARMOR + i);
        ::swprintf(szFileName, L"Player\\InvenPantsMale%d.tga", 40 + i);
        ::LoadBitmap(szFileName, BITMAP_INVEN_PANTS + i);
    }

    ::swprintf(szFileName, L"Player\\Item312_Armoritem.tga");
    ::LoadBitmap(szFileName, BITMAP_SKIN_ARMOR_DEVINE);
    ::swprintf(szFileName, L"Player\\Item312_Pantitem.tga");
    ::LoadBitmap(szFileName, BITMAP_SKIN_PANTS_DEVINE);
    ::swprintf(szFileName, L"Player\\SkinClass706_upperitem.tga");
    ::LoadBitmap(szFileName, BITMAP_SKIN_ARMOR_SUCCUBUS);
    ::swprintf(szFileName, L"Player\\SkinClass706_loweritem.tga");
    ::LoadBitmap(szFileName, BITMAP_SKIN_PANTS_SUCCUBUS);

    for (int i = 0; i < MODEL_ITEM_COMMON_NUM; ++i)
    {
        gLoadData.OpenTexture(MODEL_HELM2 + i, L"Player\\");
        gLoadData.OpenTexture(MODEL_ARMOR2 + i, L"Player\\");
        gLoadData.OpenTexture(MODEL_PANTS2 + i, L"Player\\");
        gLoadData.OpenTexture(MODEL_GLOVES2 + i, L"Player\\");
        gLoadData.OpenTexture(MODEL_BOOTS2 + i, L"Player\\");
    }

    for (int i = 45; i <= 53; i++)
    {
        if (i == 47 || i == 48)
            continue;

        gLoadData.OpenTexture(MODEL_HELM + i, L"Player\\");
    } // for()

    for (int i = 45; i <= 53; i++)
    {
        gLoadData.OpenTexture(MODEL_ARMOR + i, L"Player\\");
        gLoadData.OpenTexture(MODEL_PANTS + i, L"Player\\");
        gLoadData.OpenTexture(MODEL_GLOVES + i, L"Player\\");
        gLoadData.OpenTexture(MODEL_BOOTS + i, L"Player\\");
    } // for()

    gLoadData.OpenTexture(MODEL_GM_CHARACTER, L"Skill\\");

    for (int i = 0; i < MODEL_ITEM_COMMONCNT_RAGEFIGHTER; ++i)
    {
        gLoadData.OpenTexture(MODEL_HELM_MONK + i, L"Player\\");
        gLoadData.OpenTexture(MODEL_ARMOR_MONK + i, L"Player\\");
        gLoadData.OpenTexture(MODEL_PANTS_MONK + i, L"Player\\");
        gLoadData.OpenTexture(MODEL_BOOTS_MONK + i, L"Player\\");
    }

    for (int i = 0; i < 3; ++i)
    {
        gLoadData.OpenTexture(MODEL_SACRED_HELM + i, L"Player\\");
        gLoadData.OpenTexture(MODEL_SACRED_ARMOR + i, L"Player\\");
        gLoadData.OpenTexture(MODEL_SACRED_PANTS + i, L"Player\\");
        gLoadData.OpenTexture(MODEL_SACRED_BOOTS + i, L"Player\\");
    }
    gLoadData.OpenTexture(MODEL_PHOENIX_SOUL_HELMET, L"Player\\");
    gLoadData.OpenTexture(MODEL_PHOENIX_SOUL_ARMOR, L"Player\\");
    gLoadData.OpenTexture(MODEL_PHOENIX_SOUL_PANTS, L"Player\\");
    gLoadData.OpenTexture(MODEL_PHOENIX_SOUL_BOOTS, L"Player\\");
}

void OpenItems()
{
    //////////////////////////////////////////////////////////////////////////
    //  MODEL_SWORD
    //////////////////////////////////////////////////////////////////////////

    for (int i = 0; i < 17; i++)
        gLoadData.AccessModel(MODEL_SWORD + i, L"Data\\Item\\", L"Sword", i + 1);

    gLoadData.AccessModel(MODEL_DARK_BREAKER, L"Data\\Item\\", L"Sword", 18);
    gLoadData.AccessModel(MODEL_THUNDER_BLADE, L"Data\\Item\\", L"Sword", 19);
    gLoadData.AccessModel(MODEL_DIVINE_SWORD_OF_ARCHANGEL, L"Data\\Item\\", L"Sword", 20);

    gLoadData.AccessModel(MODEL_KNIGHT_BLADE, L"Data\\Item\\", L"Sword", 21);
    gLoadData.AccessModel(MODEL_DARK_REIGN_BLADE, L"Data\\Item\\", L"Sword", 22);
    gLoadData.AccessModel(MODEL_RUNE_BLADE, L"Data\\Item\\", L"Sword", 32);

    //////////////////////////////////////////////////////////////////////////
    //	MODEL_AXE
    //////////////////////////////////////////////////////////////////////////

    for (int i = 0; i < 9; i++)
        gLoadData.AccessModel(MODEL_AXE + i, L"Data\\Item\\", L"Axe", i + 1);

    //////////////////////////////////////////////////////////////////////////
    //	MODEL_MACE
    //////////////////////////////////////////////////////////////////////////

    for (int i = 0; i < 7; i++)
        gLoadData.AccessModel(MODEL_MACE + i, L"Data\\Item\\", L"Mace", i + 1);

    gLoadData.AccessModel(MODEL_ELEMENTAL_MACE, L"Data\\Item\\", L"Mace", 8);

    // MODEL_MACE+8,9,10,11,12
    for (int i = 0; i < 5; i++)
        gLoadData.AccessModel(MODEL_BATTLE_SCEPTER + i, L"Data\\Item\\", L"Mace", 9 + i);

    gLoadData.AccessModel(MODEL_DIVINE_SCEPTER_OF_ARCHANGEL, L"Data\\Item\\", L"Saint");

    //////////////////////////////////////////////////////////////////////////
    //	MODEL_SPEAR
    //////////////////////////////////////////////////////////////////////////

    for (int i = 0; i < 10; i++)
        gLoadData.AccessModel(MODEL_SPEAR + i, L"Data\\Item\\", L"Spear", i + 1);

    gLoadData.AccessModel(MODEL_DRAGON_SPEAR, L"Data\\Item\\", L"Spear", 11);

    //////////////////////////////////////////////////////////////////////////
    //	MODEL_SHIELD
    //////////////////////////////////////////////////////////////////////////

    for (int i = 0; i < 15; i++)
        gLoadData.AccessModel(MODEL_SHIELD + i, L"Data\\Item\\", L"Shield", i + 1);

    gLoadData.AccessModel(MODEL_GRAND_SOUL_SHIELD, L"Data\\Item\\", L"Shield", 16);
    gLoadData.AccessModel(MODEL_ELEMENTAL_SHIELD, L"Data\\Item\\", L"Shield", 17);

    //////////////////////////////////////////////////////////////////////////
    //  MODEL_STAFF
    //////////////////////////////////////////////////////////////////////////

    for (int i = 0; i < 9; i++)
        gLoadData.AccessModel(MODEL_STAFF + i, L"Data\\Item\\", L"Staff", i + 1);

    gLoadData.AccessModel(MODEL_DRAGON_SOUL_STAFF, L"Data\\Item\\", L"Staff", 10);
    gLoadData.AccessModel(MODEL_DIVINE_STAFF_OF_ARCHANGEL, L"Data\\Item\\", L"Staff", 11);
    gLoadData.AccessModel(MODEL_STAFF_OF_KUNDUN, L"Data\\Item\\", L"Staff", 12);

    for (int i = 14; i <= 20; ++i)
        ::gLoadData.AccessModel(MODEL_STAFF + i, L"Data\\Item\\", L"Staff", i + 1);

    //////////////////////////////////////////////////////////////////////////
    //  MODEL_BOW
    //////////////////////////////////////////////////////////////////////////

    for (int i = 0; i < 7; i++)
        gLoadData.AccessModel(MODEL_BOW + i, L"Data\\Item\\", L"Bow", i + 1);

    for (int i = 0; i < 7; i++)
        gLoadData.AccessModel(MODEL_BOW + i + 8, L"Data\\Item\\", L"CrossBow", i + 1);

    gLoadData.AccessModel(MODEL_BOLT, L"Data\\Item\\", L"Arrows", 1);
    gLoadData.AccessModel(MODEL_ARROWS, L"Data\\Item\\", L"Arrows", 2);
    gLoadData.AccessModel(MODEL_SAINT_CROSSBOW, L"Data\\Item\\", L"CrossBow", 17);
    gLoadData.AccessModel(MODEL_CELESTIAL_BOW, L"Data\\Item\\", L"Bow", 18);
    gLoadData.AccessModel(MODEL_DIVINE_CB_OF_ARCHANGEL, L"Data\\Item\\", L"Bow", 19);
    gLoadData.AccessModel(MODEL_GREAT_REIGN_CROSSBOW, L"Data\\Item\\", L"CrossBow", 20);
    gLoadData.AccessModel(MODEL_ARROW_VIPER_BOW, L"Data\\Item\\", L"Bow", 20);

    //////////////////////////////////////////////////////////////////////////
    //  MODEL_HELPER
    //////////////////////////////////////////////////////////////////////////

    for (int i = 0; i < 3; i++)
        gLoadData.AccessModel(MODEL_HELPER + i, L"Data\\Player\\", L"Helper", i + 1);

    for (int i = 0; i < 2; i++)
        gLoadData.AccessModel(MODEL_HELPER + i + 8, L"Data\\Item\\", L"Ring", i + 1);

    g_ChangeRingMgr->LoadItemModel();

    for (int i = 0; i < 2; i++)
        gLoadData.AccessModel(MODEL_HELPER + i + 12, L"Data\\Item\\", L"Necklace", i + 1);

    gLoadData.AccessModel(MODEL_HORN_OF_DINORANT, L"Data\\Player\\", L"Helper", 4);
    gLoadData.AccessModel(MODEL_DARK_HORSE_ITEM, L"Data\\Item\\", L"DarkHorseHorn");
    gLoadData.AccessModel(MODEL_SPIRIT, L"Data\\Item\\", L"DarkHorseSoul");
    gLoadData.AccessModel(MODEL_DARK_RAVEN_ITEM, L"Data\\Item\\", L"SpiritBill");

    gLoadData.AccessModel(MODEL_RING_OF_FIRE, L"Data\\Item\\", L"FireRing");
    gLoadData.AccessModel(MODEL_RING_OF_EARTH, L"Data\\Item\\", L"GroundRing");
    gLoadData.AccessModel(MODEL_RING_OF_WIND, L"Data\\Item\\", L"WindRing");
    gLoadData.AccessModel(MODEL_RING_OF_MAGIC, L"Data\\Item\\", L"ManaRing");
    gLoadData.AccessModel(MODEL_PENDANT_OF_ICE, L"Data\\Item\\", L"IceNecklace");
    gLoadData.AccessModel(MODEL_PENDANT_OF_WIND, L"Data\\Item\\", L"WindNecklace");
    gLoadData.AccessModel(MODEL_PENDANT_OF_WATER, L"Data\\Item\\", L"WaterNecklace");
    gLoadData.AccessModel(MODEL_PENDANT_OF_ABILITY, L"Data\\Item\\", L"AgNecklace");
    gLoadData.AccessModel(MODEL_ARMOR_OF_GUARDSMAN, L"Data\\Item\\", L"EventChaosCastle");
    gLoadData.AccessModel(MODEL_HELPER + 7, L"Data\\Item\\", L"Covenant");
    gLoadData.AccessModel(MODEL_LIFE_STONE_ITEM, L"Data\\Item\\", L"SummonBook");
    gLoadData.AccessModel(MODEL_EVENT + 18, L"Data\\Item\\", L"LifeStoneItem");

    gLoadData.AccessModel(MODEL_SPLINTER_OF_ARMOR, L"Data\\Item\\", L"FR_1");
    gLoadData.AccessModel(MODEL_BLESS_OF_GUARDIAN, L"Data\\Item\\", L"FR_2");
    gLoadData.AccessModel(MODEL_CLAW_OF_BEAST, L"Data\\Item\\", L"FR_3");
    gLoadData.AccessModel(MODEL_FRAGMENT_OF_HORN, L"Data\\Item\\", L"FR_4");
    gLoadData.AccessModel(MODEL_BROKEN_HORN, L"Data\\Item\\", L"FR_5");
    gLoadData.AccessModel(MODEL_HORN_OF_FENRIR, L"Data\\Item\\", L"FR_6");

    gLoadData.AccessModel(MODEL_HELPER + 46, L"Data\\Item\\partCharge1\\", L"entrancegray");
    gLoadData.AccessModel(MODEL_HELPER + 47, L"Data\\Item\\partCharge1\\", L"entrancered");
    gLoadData.AccessModel(MODEL_HELPER + 48, L"Data\\Item\\partCharge1\\", L"entrancebleu");
    gLoadData.AccessModel(MODEL_POTION + 54, L"Data\\Item\\partCharge1\\", L"juju");
    gLoadData.AccessModel(MODEL_HELPER + 43, L"Data\\Item\\partCharge1\\", L"monmark01");
    gLoadData.AccessModel(MODEL_HELPER + 44, L"Data\\Item\\partCharge1\\", L"monmark02");
    gLoadData.AccessModel(MODEL_HELPER + 45, L"Data\\Item\\partCharge1\\", L"monmark03");
    gLoadData.AccessModel(MODEL_POTION + 53, L"Data\\Item\\partCharge1\\", L"bujuck01");

    gLoadData.AccessModel(MODEL_POTION + 58, L"Data\\Item\\partCharge1\\", L"expensiveitem01");
    gLoadData.AccessModel(MODEL_POTION + 59, L"Data\\Item\\partCharge1\\", L"expensiveitem02a");
    gLoadData.AccessModel(MODEL_POTION + 60, L"Data\\Item\\partCharge1\\", L"expensiveitem02b");
    gLoadData.AccessModel(MODEL_POTION + 61, L"Data\\Item\\partCharge1\\", L"expensiveitem03a");
    gLoadData.AccessModel(MODEL_POTION + 62, L"Data\\Item\\partCharge1\\", L"expensiveitem03b");
    gLoadData.AccessModel(MODEL_POTION + 70, L"Data\\Item\\partCharge2\\", L"EPotionR");
    gLoadData.AccessModel(MODEL_POTION + 71, L"Data\\Item\\partCharge2\\", L"EPotionB");
    gLoadData.AccessModel(MODEL_POTION + 72, L"Data\\Item\\partCharge2\\", L"elitescroll_quick");
    gLoadData.AccessModel(MODEL_POTION + 73, L"Data\\Item\\partCharge2\\", L"elitescroll_depence");
    gLoadData.AccessModel(MODEL_POTION + 74, L"Data\\Item\\partCharge2\\", L"elitescroll_anger");
    gLoadData.AccessModel(MODEL_POTION + 75, L"Data\\Item\\partCharge2\\", L"elitescroll_magic");
    gLoadData.AccessModel(MODEL_POTION + 76, L"Data\\Item\\partCharge2\\", L"elitescroll_strenth");
    gLoadData.AccessModel(MODEL_POTION + 77, L"Data\\Item\\partCharge2\\", L"elitescroll_mana");

    wchar_t	szPC6Path[24];
    swprintf(szPC6Path, L"Data\\Item\\partCharge6\\");

    gLoadData.AccessModel(MODEL_TYPE_CHARM_MIXWING + EWS_KNIGHT_1_CHARM, szPC6Path, L"amulet_satan");
    gLoadData.AccessModel(MODEL_TYPE_CHARM_MIXWING + EWS_MAGICIAN_1_CHARM, szPC6Path, L"amulet_sky");
    gLoadData.AccessModel(MODEL_TYPE_CHARM_MIXWING + EWS_ELF_1_CHARM, szPC6Path, L"amulet_elf");
    gLoadData.AccessModel(MODEL_TYPE_CHARM_MIXWING + EWS_SUMMONER_1_CHARM, szPC6Path, L"amulet_disaster");
    gLoadData.AccessModel(MODEL_TYPE_CHARM_MIXWING + EWS_DARKLORD_1_CHARM, szPC6Path, L"amulet_cloak");
    gLoadData.AccessModel(MODEL_TYPE_CHARM_MIXWING + EWS_KNIGHT_2_CHARM, szPC6Path, L"amulet_dragon");
    gLoadData.AccessModel(MODEL_TYPE_CHARM_MIXWING + EWS_MAGICIAN_2_CHARM, szPC6Path, L"amulet_soul");
    gLoadData.AccessModel(MODEL_TYPE_CHARM_MIXWING + EWS_ELF_2_CHARM, szPC6Path, L"amulet_spirit");
    gLoadData.AccessModel(MODEL_TYPE_CHARM_MIXWING + EWS_SUMMONER_2_CHARM, szPC6Path, L"amulet_despair");
    gLoadData.AccessModel(MODEL_TYPE_CHARM_MIXWING + EWS_DARKKNIGHT_2_CHARM, szPC6Path, L"amulet_dark");
    gLoadData.AccessModel(MODEL_HELPER + 59, L"Data\\Item\\partCharge2\\", L"sealmove");
    gLoadData.AccessModel(MODEL_HELPER + 54, L"Data\\Item\\partCharge2\\", L"resetfruit_power");
    gLoadData.AccessModel(MODEL_HELPER + 55, L"Data\\Item\\partCharge2\\", L"resetfruit_quick");
    gLoadData.AccessModel(MODEL_HELPER + 56, L"Data\\Item\\partCharge2\\", L"resetfruit_strenth");
    gLoadData.AccessModel(MODEL_HELPER + 57, L"Data\\Item\\partCharge2\\", L"resetfruit_energe");
    gLoadData.AccessModel(MODEL_HELPER + 58, L"Data\\Item\\partCharge2\\", L"resetfruit_command");
    gLoadData.AccessModel(MODEL_POTION + 78, L"Data\\Item\\partCharge2\\", L"secret_power");
    gLoadData.AccessModel(MODEL_POTION + 79, L"Data\\Item\\partCharge2\\", L"secret_quick");
    gLoadData.AccessModel(MODEL_POTION + 80, L"Data\\Item\\partCharge2\\", L"secret_strenth");
    gLoadData.AccessModel(MODEL_POTION + 81, L"Data\\Item\\partCharge2\\", L"secret_energe");
    gLoadData.AccessModel(MODEL_POTION + 82, L"Data\\Item\\partCharge2\\", L"secret_command");
    gLoadData.AccessModel(MODEL_HELPER + 60, L"Data\\Item\\partCharge2\\", L"indulgence");
    gLoadData.AccessModel(MODEL_HELPER + 61, L"Data\\Item\\partCharge2\\", L"entrancepurple");
    gLoadData.AccessModel(MODEL_POTION + 83, L"Data\\Item\\partCharge2\\", L"expensiveitem04b");
    gLoadData.AccessModel(MODEL_POTION + 145, L"Data\\Item\\partCharge8\\", L"rareitem_ticket7");
    gLoadData.AccessModel(MODEL_POTION + 146, L"Data\\Item\\partCharge8\\", L"rareitem_ticket8");
    gLoadData.AccessModel(MODEL_POTION + 147, L"Data\\Item\\partCharge8\\", L"rareitem_ticket9");
    gLoadData.AccessModel(MODEL_POTION + 148, L"Data\\Item\\partCharge8\\", L"rareitem_ticket10");
    gLoadData.AccessModel(MODEL_POTION + 149, L"Data\\Item\\partCharge8\\", L"rareitem_ticket11");
    gLoadData.AccessModel(MODEL_POTION + 150, L"Data\\Item\\partCharge8\\", L"rareitem_ticket12");
    gLoadData.AccessModel(MODEL_HELPER + 125, L"Data\\Item\\partCharge8\\", L"DoppelCard");
    gLoadData.AccessModel(MODEL_HELPER + 126, L"Data\\Item\\partCharge8\\", L"BarcaCard");
    gLoadData.AccessModel(MODEL_HELPER + 127, L"Data\\Item\\partCharge8\\", L"Barca7Card");

#ifdef LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM
    gLoadData.AccessModel(MODEL_HELPER + 128, L"Data\\Item\\", L"HawkStatue");
    gLoadData.AccessModel(MODEL_HELPER + 129, L"Data\\Item\\", L"SheepStatue");
    gLoadData.AccessModel(MODEL_HELPER + 134, L"Data\\Item\\", L"horseshoe");
#endif //LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM
#ifdef LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM_PART_2
    gLoadData.AccessModel(MODEL_HELPER + 130, L"Data\\Item\\", L"ork_cham");
    //	gLoadData.AccessModel(MODEL_HELPER+131, L"Data\\Item\\", L"maple_cham");
    gLoadData.AccessModel(MODEL_HELPER + 132, L"Data\\Item\\", L"goldenork_cham");
    //	gLoadData.AccessModel(MODEL_HELPER+132, L"Data\\Item\\", L"goldenmaple_cham");
#endif //LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM_PART_2

    gLoadData.AccessModel(MODEL_POTION + 91, L"Data\\Item\\partCharge3\\", L"alicecard");

    gLoadData.AccessModel(MODEL_POTION + 92, L"Data\\Item\\partCharge3\\", L"juju");
    gLoadData.AccessModel(MODEL_POTION + 93, L"Data\\Item\\partCharge3\\", L"juju");
    gLoadData.AccessModel(MODEL_POTION + 95, L"Data\\Item\\partCharge3\\", L"juju");

    gLoadData.AccessModel(MODEL_POTION + 94, L"Data\\Item\\partCharge2\\", L"EPotionR");
    gLoadData.AccessModel(MODEL_CHERRY_BLOSSOM_PLAYBOX, L"Data\\Item\\cherryblossom\\", L"cherrybox");
    gLoadData.AccessModel(MODEL_CHERRY_BLOSSOM_WINE, L"Data\\Item\\cherryblossom\\", L"chwine");
    gLoadData.AccessModel(MODEL_CHERRY_BLOSSOM_RICE_CAKE, L"Data\\Item\\cherryblossom\\", L"chgateaux");
    gLoadData.AccessModel(MODEL_CHERRY_BLOSSOM_FLOWER_PETAL, L"Data\\Item\\cherryblossom\\", L"chpetal");
    gLoadData.AccessModel(MODEL_POTION + 88, L"Data\\Item\\cherryblossom\\", L"chbranche");
    gLoadData.AccessModel(MODEL_POTION + 89, L"Data\\Item\\cherryblossom\\", L"chbranche_red");
    gLoadData.AccessModel(MODEL_GOLDEN_CHERRY_BLOSSOM_BRANCH, L"Data\\Item\\cherryblossom\\", L"chbranche_yellow");
    gLoadData.AccessModel(MODEL_HELPER + 62, L"Data\\Item\\partCharge4\\", L"Curemark");
    gLoadData.AccessModel(MODEL_HELPER + 63, L"Data\\Item\\partCharge4\\", L"Holinessmark");
    gLoadData.AccessModel(MODEL_POTION + 97, L"Data\\Item\\partCharge4\\", L"battlescroll");
    gLoadData.AccessModel(MODEL_POTION + 98, L"Data\\Item\\partCharge4\\", L"strengscroll");

    gLoadData.AccessModel(MODEL_POTION + 96, L"Data\\Item\\partCharge4\\", L"strengamulet");

    gLoadData.AccessModel(MODEL_DEMON, L"Data\\Item\\partCharge4\\", L"demon");
    gLoadData.AccessModel(MODEL_SPIRIT_OF_GUARDIAN, L"Data\\Item\\partCharge4\\", L"maria");

    gLoadData.AccessModel(MODEL_PET_PANDA, L"Data\\Item\\", L"PandaPet");
    gLoadData.AccessModel(MODEL_PET_RUDOLF, L"Data\\Item\\xmas\\", L"xmas_deer");
    gLoadData.AccessModel(MODEL_PET_UNICORN, L"Data\\Item\\partcharge7\\", L"pet_unicorn");
    gLoadData.AccessModel(MODEL_PET_SKELETON, L"Data\\Item\\", L"skeletonpet");

    gLoadData.AccessModel(MODEL_HELPER + 69, L"Data\\Item\\partCharge5\\", L"ressurection");
    gLoadData.AccessModel(MODEL_HELPER + 70, L"Data\\Item\\partCharge5\\", L"potalcharm");
    gLoadData.AccessModel(MODEL_HELPER + 81, L"Data\\Item\\partCharge6\\", L"suhocham01");
    gLoadData.AccessModel(MODEL_HELPER + 82, L"Data\\Item\\partCharge6\\", L"imteam_protect");
    gLoadData.AccessModel(MODEL_HELPER + 93, L"Data\\Item\\partCharge6\\", L"MasterSealA");

    gLoadData.AccessModel(MODEL_HELPER + 94, L"Data\\Item\\partCharge6\\", L"MasterSealB");
    gLoadData.AccessModel(MODEL_POTION + 140, L"Data\\Item\\", L"strengscroll");

    gLoadData.AccessModel(MODEL_OLD_SCROLL, L"Data\\Item\\", L"scrollpaper");
    gLoadData.AccessModel(MODEL_ILLUSION_SORCERER_COVENANT, L"Data\\Item\\", L"oath");
    gLoadData.AccessModel(MODEL_SCROLL_OF_BLOOD, L"Data\\Item\\", L"songbl");

    gLoadData.AccessModel(MODEL_FLAME_OF_CONDOR, L"Data\\Item\\", L"condolstone");
    gLoadData.AccessModel(MODEL_FEATHER_OF_CONDOR, L"Data\\Item\\", L"condolwing");

    gLoadData.AccessModel(MODEL_POTION + 64, L"Data\\Item\\", L"songss");
    gLoadData.AccessModel(MODEL_FLAME_OF_DEATH_BEAM_KNIGHT, L"Data\\Item\\", L"deathbeamstone");
    gLoadData.AccessModel(MODEL_HORN_OF_HELL_MAINE, L"Data\\Item\\", L"hellhorn");
    gLoadData.AccessModel(MODEL_FEATHER_OF_DARK_PHOENIX, L"Data\\Item\\", L"phoenixfeather");
    gLoadData.AccessModel(MODEL_EYE_OF_ABYSSAL, L"Data\\Item\\", L"Deye");

    for (int i = 0; i < 6; ++i)
        gLoadData.AccessModel(MODEL_SEED_FIRE + i, L"Data\\Item\\", L"s30_seed");
    for (int i = 0; i < 5; ++i)
        gLoadData.AccessModel(MODEL_SPHERE_MONO + i, L"Data\\Item\\", L"s30_sphere_body", i + 1);
    for (int i = 0; i < 5; ++i)
    {
        for (int j = 0; j < 6; ++j)
            gLoadData.AccessModel(MODEL_SEED_SPHERE_FIRE_1 + i * 6 + j, L"Data\\Item\\", L"s30_sphere", i + 1);
    }

    gLoadData.AccessModel(MODEL_HELPER + 107, L"Data\\Item\\partcharge7\\", L"FatalRing");
    gLoadData.AccessModel(MODEL_HELPER + 104, L"Data\\Item\\partcharge7\\", L"injang_AG");
    gLoadData.AccessModel(MODEL_HELPER + 105, L"Data\\Item\\partcharge7\\", L"injang_SD");
    gLoadData.AccessModel(MODEL_HELPER + 103, L"Data\\Item\\partcharge7\\", L"EXPscroll");
    gLoadData.AccessModel(MODEL_POTION + 133, L"Data\\Item\\partcharge7\\", L"ESDPotion");

    gLoadData.AccessModel(MODEL_HELPER + 109, L"Data\\Item\\InGameShop\\", L"PeriodRingBlue");

    gLoadData.AccessModel(MODEL_HELPER + 110, L"Data\\Item\\InGameShop\\", L"PeriodRingRed");
    gLoadData.AccessModel(MODEL_HELPER + 111, L"Data\\Item\\InGameShop\\", L"PeriodRingYellow");
    gLoadData.AccessModel(MODEL_HELPER + 112, L"Data\\Item\\InGameShop\\", L"PeriodRingViolet");
    gLoadData.AccessModel(MODEL_HELPER + 113, L"Data\\Item\\InGameShop\\", L"necklace_red");
    gLoadData.AccessModel(MODEL_HELPER + 114, L"Data\\Item\\InGameShop\\", L"necklace_blue");
    gLoadData.AccessModel(MODEL_HELPER + 115, L"Data\\Item\\InGameShop\\", L"necklace_green");

    gLoadData.AccessModel(MODEL_HELPER + 116, L"Data\\Item\\", L"monmark02");

    gLoadData.AccessModel(MODEL_WING + 130, L"Data\\Item\\", L"DarkLordRobe");
    gLoadData.AccessModel(MODEL_WING + 131, L"Data\\Item\\Ingameshop\\", L"alice1wing");
    gLoadData.AccessModel(MODEL_WING + 132, L"Data\\Item\\Ingameshop\\", L"elf_wing");
    gLoadData.AccessModel(MODEL_WING + 133, L"Data\\Item\\Ingameshop\\", L"angel_wing");
    gLoadData.AccessModel(MODEL_WING + 134, L"Data\\Item\\Ingameshop\\", L"devil_wing");
    gLoadData.AccessModel(MODEL_HELPER + 124, L"Data\\Item\\partCharge6\\", L"ChannelCard");

    for (int i = 0; i < 7; i++)
        gLoadData.AccessModel(MODEL_POTION + i, L"Data\\Item\\", L"Potion", i + 1);

    gLoadData.AccessModel(MODEL_ANTIDOTE, L"Data\\Item\\", L"Antidote", 1);
    gLoadData.AccessModel(MODEL_ALE, L"Data\\Item\\", L"Beer", 1);
    gLoadData.AccessModel(MODEL_TOWN_PORTAL_SCROLL, L"Data\\Item\\", L"Scroll", 1);
    gLoadData.AccessModel(MODEL_BOX_OF_LUCK, L"Data\\Item\\", L"MagicBox", 1);
    gLoadData.AccessModel(MODEL_POTION + 12, L"Data\\Item\\", L"Event", 1);

    for (int i = 0; i < 2; i++)
        gLoadData.AccessModel(MODEL_POTION + i + 13, L"Data\\Item\\", L"Jewel", i + 1);

    gLoadData.AccessModel(MODEL_POTION + 15, L"Data\\Item\\", L"Gold", 1);
    gLoadData.AccessModel(MODEL_JEWEL_OF_LIFE, L"Data\\Item\\", L"Jewel", 3);

    for (int i = 0; i < 3; i++)
        gLoadData.AccessModel(MODEL_DEVILS_EYE + i, L"Data\\Item\\", L"Devil", i);

    gLoadData.AccessModel(MODEL_POTION + 20, L"Data\\Item\\", L"Drink", 0);
    gLoadData.AccessModel(MODEL_POTION + 21, L"Data\\Item\\", L"ConChip", 0);
    gLoadData.AccessModel(MODEL_JEWEL_OF_GUARDIAN, L"Data\\Item\\", L"suho", -1);
    gLoadData.AccessModel(MODEL_MOONSTONE_PENDANT, L"Data\\Item\\", L"kanneck2");
    gLoadData.AccessModel(MODEL_GEMSTONE, L"Data\\Item\\", L"rs");
    gLoadData.AccessModel(MODEL_JEWEL_OF_HARMONY, L"Data\\Item\\", L"jos");
    gLoadData.AccessModel(MODEL_LOWER_REFINE_STONE, L"Data\\Item\\", L"LowRefineStone");
    gLoadData.AccessModel(MODEL_HIGHER_REFINE_STONE, L"Data\\Item\\", L"HighRefineStone");

    gLoadData.AccessModel(MODEL_SIEGE_POTION, L"Data\\Item\\", L"SpecialPotion");

    for (int i = 0; i < 4; ++i)
    {
        gLoadData.AccessModel(MODEL_SCROLL_OF_EMPEROR_RING_OF_HONOR + i, L"Data\\Item\\", L"Quest", i);
    }
    gLoadData.AccessModel(MODEL_POTION + 27, L"Data\\Item\\", L"godesteel");

    for (int i = 0; i < 2; i++)
    {
        gLoadData.AccessModel(MODEL_LOST_MAP + i, L"Data\\Item\\", L"HELLASITEM", i);
    }

    for (int i = 0; i < 3; ++i)
        gLoadData.AccessModel(MODEL_SMALL_SHIELD_POTION + i, L"Data\\Item\\", L"sdwater", i + 1);

    for (int i = 0; i < 3; ++i)
        gLoadData.AccessModel(MODEL_SMALL_COMPLEX_POTION + i, L"Data\\Item\\", L"megawater", i + 1);

    gLoadData.AccessModel(MODEL_POTION + 120, L"Data\\Item\\InGameShop\\", L"gold_coin");
    gLoadData.AccessModel(MODEL_POTION + 121, L"Data\\Item\\InGameShop\\", L"itembox_gold");
    gLoadData.AccessModel(MODEL_POTION + 122, L"Data\\Item\\InGameShop\\", L"itembox_silver");
    gLoadData.AccessModel(MODEL_POTION + 123, L"Data\\Item\\InGameShop\\", L"itembox_gold");
    gLoadData.AccessModel(MODEL_POTION + 124, L"Data\\Item\\InGameShop\\", L"itembox_silver");
    for (int k = 0; k < 6; k++)
    {
        gLoadData.AccessModel(MODEL_POTION + 134 + k, L"Data\\Item\\InGameShop\\", L"package_money_item");
    }

    gLoadData.AccessModel(MODEL_COMPILED_CELE, L"Data\\Item\\", L"Jewel", 1);
    gLoadData.AccessModel(MODEL_COMPILED_SOUL, L"Data\\Item\\", L"Jewel", 2);
    gLoadData.AccessModel(MODEL_PACKED_JEWEL_OF_LIFE, L"Data\\Item\\", L"Jewel", 3);
    gLoadData.AccessModel(MODEL_PACKED_JEWEL_OF_CREATION, L"Data\\Item\\", L"jewel", 22);
    gLoadData.AccessModel(MODEL_PACKED_JEWEL_OF_GUARDIAN, L"Data\\Item\\", L"suho", -1);
    gLoadData.AccessModel(MODEL_PACKED_GEMSTONE, L"Data\\Item\\", L"rs");
    gLoadData.AccessModel(MODEL_PACKED_JEWEL_OF_HARMONY, L"Data\\Item\\", L"jos");
    gLoadData.AccessModel(MODEL_PACKED_JEWEL_OF_CHAOS, L"Data\\Item\\", L"Jewel", 15);
    gLoadData.AccessModel(MODEL_PACKED_LOWER_REFINE_STONE, L"Data\\Item\\", L"LowRefineStone");
    gLoadData.AccessModel(MODEL_PACKED_HIGHER_REFINE_STONE, L"Data\\Item\\", L"HighRefineStone");

    gLoadData.AccessModel(MODEL_EVENT + 4, L"Data\\Item\\", L"MagicBox", 2);
    gLoadData.AccessModel(MODEL_EVENT + 6, L"Data\\Item\\", L"MagicBox", 5);
    gLoadData.AccessModel(MODEL_EVENT + 7, L"Data\\Item\\", L"Beer", 2);
    gLoadData.AccessModel(MODEL_EVENT + 8, L"Data\\Item\\", L"MagicBox", 6);
    gLoadData.AccessModel(MODEL_EVENT + 9, L"Data\\Item\\", L"MagicBox", 7);

    gLoadData.AccessModel(MODEL_HELPER + 66, L"Data\\Item\\xmas\\", L"santa_village", -1);

    gLoadData.AccessModel(MODEL_POTION + 100, L"Data\\Item\\", L"coin7", -1);

    g_XmasEvent->LoadXmasEventItem();

    gLoadData.AccessModel(MODEL_EVENT + 10, L"Data\\Item\\", L"MagicBox", 8);

    gLoadData.AccessModel(MODEL_EVENT + 5, L"Data\\Item\\", L"MagicBox", 3);

    gLoadData.AccessModel(MODEL_FIRECRACKER, L"Data\\Item\\", L"GM", 1);
    gLoadData.AccessModel(MODEL_GM_GIFT, L"Data\\Item\\", L"GM", 2);

    gLoadData.AccessModel(MODEL_EVENT, L"Data\\Item\\", L"Event", 2);
    gLoadData.AccessModel(MODEL_EVENT + 1, L"Data\\Item\\", L"Event", 3);

    gLoadData.AccessModel(MODEL_CHRISTMAS_FIRECRACKER, L"Data\\Item\\XMas\\", L"xmasfire", -1);

    for (int i = 0; i < 4; i++)
    {
        if (i < 3)
        {
            gLoadData.AccessModel(MODEL_SCROLL_OF_ARCHANGEL + i, L"Data\\Item\\", L"EventBloodCastle", i);
        }
        else
        {
            gLoadData.AccessModel(MODEL_EVENT + 11 + (i - 3), L"Data\\Item\\", L"EventBloodCastle", i);
        }
    }

    gLoadData.AccessModel(MODEL_EVENT + 12, L"Data\\Item\\", L"QuestItem3RD", 0);
    gLoadData.AccessModel(MODEL_EVENT + 13, L"Data\\Item\\", L"QuestItem3RD", 1);
    gLoadData.AccessModel(MODEL_EVENT + 14, L"Data\\Item\\", L"RingOfLordEvent", 0);
    gLoadData.AccessModel(MODEL_EVENT + 15, L"Data\\Item\\", L"MagicRing", 0);

    gLoadData.AccessModel(MODEL_JEWEL_OF_CREATION, L"Data\\Item\\", L"jewel", 22);

    for (int i = 0; i < 2; ++i)
    {
        gLoadData.AccessModel(MODEL_LOCHS_FEATHER + i, L"Data\\Item\\", L"Quest", 4 + i);
    }

    gLoadData.AccessModel(MODEL_EVENT + 16, L"Data\\Item\\", L"DarkLordSleeve");
    gLoadData.AccessModel(MODEL_CAPE_OF_LORD, L"Data\\Item\\", L"DarkLordRobe");

    for (int i = 0; i < 3; i++)
        gLoadData.AccessModel(MODEL_WING + i, L"Data\\Item\\", L"Wing", i + 1);

    for (int i = 0; i < 4; i++)
    {
        gLoadData.AccessModel(MODEL_WINGS_OF_SPIRITS + i, L"Data\\Item\\", L"Wing", 4 + i);
    }

    for (int i = 0; i < 4; i++)
    {
        gLoadData.AccessModel(MODEL_WING_OF_STORM + i, L"Data\\Item\\", L"Wing", 8 + i);
    }
    gLoadData.AccessModel(MODEL_CAPE_OF_EMPEROR, L"Data\\Item\\", L"DarkLordRobe02");

    for (int i = 41; i <= 43; ++i)
        ::gLoadData.AccessModel(MODEL_WING + i, L"Data\\Item\\", L"Wing", i + 1);

    gLoadData.AccessModel(MODEL_BOOK_OF_SAHAMUTT, L"Data\\Item\\", L"Book_of_Sahamutt");
    gLoadData.AccessModel(MODEL_BOOK_OF_NEIL, L"Data\\Item\\", L"Book_of_Neil");
    gLoadData.AccessModel(MODEL_BOOK_OF_LAGLE, L"Data\\Item\\", L"Book_of_Rargle");

    for (int i = 0; i < 9; ++i)
    {
        gLoadData.AccessModel(MODEL_CHAIN_LIGHTNING_PARCHMENT + i, L"Data\\Item\\", L"rollofpaper");
    }

    for (int i = 0; i < 13; i++)
    {
        if (i + 7 != 15)
            gLoadData.AccessModel(MODEL_WING + i + 7, L"Data\\Item\\", L"Gem", i + 1);
    }

    gLoadData.AccessModel(MODEL_JEWEL_OF_CHAOS, L"Data\\Item\\", L"Jewel", 15);
    gLoadData.AccessModel(MODEL_WING + 20, L"Data\\Item\\", L"Gem", 14);

    gLoadData.AccessModel(MODEL_CRYSTAL_OF_DESTRUCTION, L"Data\\Item\\", L"Gem", 6);

    gLoadData.AccessModel(MODEL_CRYSTAL_OF_MULTI_SHOT, L"Data\\Item\\", L"Gem", 6);

    gLoadData.AccessModel(MODEL_CRYSTAL_OF_RECOVERY, L"Data\\Item\\", L"Gem", 6);

    gLoadData.AccessModel(MODEL_CRYSTAL_OF_FLAME_STRIKE, L"Data\\Item\\", L"Gem", 6);

    for (int i = 0; i < 4; i++)
    {
        gLoadData.AccessModel(MODEL_SCROLL_OF_FIREBURST + i, L"Data\\Item\\", L"SkillScroll");
    }
    gLoadData.AccessModel(MODEL_SCROLL_OF_FIRE_SCREAM, L"Data\\Item\\", L"SkillScroll");

    gLoadData.AccessModel(MODEL_SCROLL_OF_CHAOTIC_DISEIER, L"Data\\Item\\", L"SkillScroll");

    gLoadData.AccessModel(MODEL_PUMPKIN_OF_LUCK, L"Data\\Item\\", L"hobakhead");
    gLoadData.AccessModel(MODEL_JACK_OLANTERN_BLESSINGS, L"Data\\Item\\", L"hellowinscroll");
    gLoadData.AccessModel(MODEL_JACK_OLANTERN_WRATH, L"Data\\Item\\", L"hellowinscroll");
    gLoadData.AccessModel(MODEL_JACK_OLANTERN_CRY, L"Data\\Item\\", L"hellowinscroll");
    gLoadData.AccessModel(MODEL_JACK_OLANTERN_FOOD, L"Data\\Item\\", L"Gogi");
    gLoadData.AccessModel(MODEL_JACK_OLANTERN_DRINK, L"Data\\Item\\", L"pumpkincup");

    gLoadData.AccessModel(MODEL_PINK_CHOCOLATE_BOX, L"Data\\Item\\", L"giftbox_bp");
    gLoadData.AccessModel(MODEL_RED_CHOCOLATE_BOX, L"Data\\Item\\", L"giftbox_br");
    gLoadData.AccessModel(MODEL_BLUE_CHOCOLATE_BOX, L"Data\\Item\\", L"giftbox_bb");

    gLoadData.AccessModel(MODEL_EVENT + 21, L"Data\\Item\\", L"p03box");
    gLoadData.AccessModel(MODEL_EVENT + 22, L"Data\\Item\\", L"obox02");
    gLoadData.AccessModel(MODEL_EVENT + 23, L"Data\\Item\\", L"blue01");

    gLoadData.AccessModel(MODEL_RED_RIBBON_BOX, L"Data\\Item\\", L"giftbox_r");
    gLoadData.AccessModel(MODEL_GREEN_RIBBON_BOX, L"Data\\Item\\", L"giftbox_g");
    gLoadData.AccessModel(MODEL_BLUE_RIBBON_BOX, L"Data\\Item\\", L"giftbox_b");

    for (int i = 0; i < 19; i++)
        gLoadData.AccessModel(MODEL_ETC + i, L"Data\\Item\\", L"Book", i + 1);

    gLoadData.AccessModel(MODEL_SCROLL_OF_GIGANTIC_STORM, L"Data\\Item\\", L"Book", 18);

    gLoadData.AccessModel(MODEL_SCROLL_OF_WIZARDRY_ENHANCE, L"Data\\Item\\", L"Book", 18);

    gLoadData.AccessModel(MODEL_BONE_BLADE, L"Data\\Item\\", L"HDK_Sword");
    gLoadData.AccessModel(MODEL_EXPLOSION_BLADE, L"Data\\Item\\", L"HDK_Sword2");
    gLoadData.AccessModel(MODEL_SOLEIL_SCEPTER, L"Data\\Item\\", L"HDK_Mace");
    gLoadData.AccessModel(MODEL_SYLPH_WIND_BOW, L"Data\\Item\\", L"HDK_Bow");
    gLoadData.AccessModel(MODEL_GRAND_VIPER_STAFF, L"Data\\Item\\", L"HDK_Staff");

    gLoadData.AccessModel(MODEL_DAYBREAK, L"Data\\Item\\", L"CW_Sword");
    gLoadData.AccessModel(MODEL_SWORD_DANCER, L"Data\\Item\\", L"CW_Sword2");
    gLoadData.AccessModel(MODEL_SHINING_SCEPTER, L"Data\\Item\\", L"CW_Mace");
    gLoadData.AccessModel(MODEL_ALBATROSS_BOW, L"Data\\Item\\", L"CW_Bow");
    gLoadData.AccessModel(MODEL_PLATINA_STAFF, L"Data\\Item\\", L"CW_Staff");

    gLoadData.AccessModel(MODEL_FLAMBERGE, L"Data\\Item\\", L"Sword_27");
    gLoadData.AccessModel(MODEL_SWORD_BREAKER, L"Data\\Item\\", L"Sword_28");
    gLoadData.AccessModel(MODEL_IMPERIAL_SWORD, L"Data\\Item\\", L"Sword_29");
    gLoadData.AccessModel(MODEL_FROST_MACE, L"Data\\Item\\", L"Mace_17");
    gLoadData.AccessModel(MODEL_ABSOLUTE_SCEPTER, L"Data\\Item\\", L"Mace_18");
    gLoadData.AccessModel(MODEL_STINGER_BOW, L"Data\\Item\\", L"Bow_24");
    gLoadData.AccessModel(MODEL_DEADLY_STAFF, L"Data\\Item\\", L"Staff_31");
    gLoadData.AccessModel(MODEL_IMPERIAL_STAFF, L"Data\\Item\\", L"Staff_32");
    gLoadData.AccessModel(MODEL_STAFF + 32, L"Data\\Item\\", L"Staff_33");
    gLoadData.AccessModel(MODEL_CRIMSONGLORY, L"Data\\Item\\", L"Shield_18");
    gLoadData.AccessModel(MODEL_SALAMANDER_SHIELD, L"Data\\Item\\", L"Shield_19");
    gLoadData.AccessModel(MODEL_FROST_BARRIER, L"Data\\Item\\", L"Shield_20");
    gLoadData.AccessModel(MODEL_GUARDIAN_SHILED, L"Data\\Item\\", L"Shield_21");

    gLoadData.AccessModel(MODEL_CROSS_SHIELD, L"Data\\Item\\", L"crosssheild");

    gLoadData.AccessModel(MODEL_AIR_LYN_BOW, L"Data\\Item\\", L"gamblebow");
    gLoadData.AccessModel(MODEL_CHROMATIC_STAFF, L"Data\\Item\\", L"gamble_wand");
    gLoadData.AccessModel(MODEL_RAVEN_STICK, L"Data\\Item\\", L"gamble_stick");
    gLoadData.AccessModel(MODEL_BEUROBA, L"Data\\Item\\", L"gamble_scyder01");
    gLoadData.AccessModel(MODEL_STRYKER_SCEPTER, L"Data\\Item\\", L"gamble_safter01");

    gLoadData.AccessModel(MODEL_HELPER + 71, L"Data\\Item\\", L"gamble_scyderx01");
    gLoadData.AccessModel(MODEL_HELPER + 72, L"Data\\Item\\", L"gamble_wand01");
    gLoadData.AccessModel(MODEL_HELPER + 73, L"Data\\Item\\", L"gamble_bowx01");
    gLoadData.AccessModel(MODEL_HELPER + 74, L"Data\\Item\\", L"gamble_safterx01");
    gLoadData.AccessModel(MODEL_HELPER + 75, L"Data\\Item\\", L"gamble_stickx01");

    gLoadData.AccessModel(MODEL_HELPER + 97, L"Data\\Item\\Ingameshop\\", L"charactercard");
    gLoadData.AccessModel(MODEL_HELPER + 98, L"Data\\Item\\Ingameshop\\", L"charactercard");
    gLoadData.AccessModel(MODEL_POTION + 91, L"Data\\Item\\partCharge3\\", L"alicecard");

#ifdef PBG_ADD_CHARACTERSLOT
    gLoadData.AccessModel(MODEL_HELPER + 99, L"Data\\Item\\Ingameshop\\", L"key");
    gLoadData.AccessModel(MODEL_SLOT_LOCK, L"Data\\Item\\Ingameshop\\", L"lock");
#endif //PBG_ADD_CHARACTERSLOT
#ifdef PBG_ADD_SECRETITEM
#ifdef PBG_MOD_SECRETITEM
    gLoadData.AccessModel(MODEL_HELPER + 117, L"Data\\Item\\Ingameshop\\", L"FRpotionD");
#else //PBG_MOD_SECRETITEM
    gLoadData.AccessModel(MODEL_HELPER + 117, L"Data\\Item\\Ingameshop\\", L"FRpotionA");
#endif //PBG_MOD_SECRETITEM
    gLoadData.AccessModel(MODEL_HELPER + 118, L"Data\\Item\\Ingameshop\\", L"FRpotionA");
    gLoadData.AccessModel(MODEL_HELPER + 119, L"Data\\Item\\Ingameshop\\", L"FRpotionB");
    gLoadData.AccessModel(MODEL_HELPER + 120, L"Data\\Item\\Ingameshop\\", L"FRpotionC");
#endif //PBG_ADD_SECRETITEM

    gLoadData.AccessModel(MODEL_POTION + 110, L"Data\\Item\\", L"indication");
    gLoadData.AccessModel(MODEL_POTION + 111, L"Data\\Item\\", L"speculum");

    gLoadData.AccessModel(MODEL_SUSPICIOUS_SCRAP_OF_PAPER, L"Data\\Item\\", L"doubt_paper");
    gLoadData.AccessModel(MODEL_GAIONS_ORDER, L"Data\\Item\\", L"warrant");
    gLoadData.AccessModel(MODEL_COMPLETE_SECROMICON, L"Data\\Item\\", L"secromicon");
    for (int c = 0; c < 6; c++)
    {
        gLoadData.AccessModel(MODEL_FIRST_SECROMICON_FRAGMENT + c, L"Data\\Item\\", L"secromicon_piece");
    }

    gLoadData.AccessModel(MODEL_POTION + 112, L"Data\\Item\\Ingameshop\\", L"ItemBoxKey_silver");

    gLoadData.AccessModel(MODEL_POTION + 113, L"Data\\Item\\Ingameshop\\", L"ItemBoxKey_gold");

    {
        for (int c = 0; c < 6; c++)
        {
            gLoadData.AccessModel(MODEL_POTION + 114 + c, L"Data\\Item\\Ingameshop\\", L"primium_membership_item");
        }
    }
    {
        for (int c = 0; c < 4; c++)
        {
            gLoadData.AccessModel(MODEL_POTION + 126 + c, L"Data\\Item\\Ingameshop\\", L"primium_membership_item");
        }
    }
    {
        for (int c = 0; c < 3; c++)
        {
            gLoadData.AccessModel(MODEL_POTION + 130 + c, L"Data\\Item\\Ingameshop\\", L"primium_membership_item");
        }
    }
    {
        gLoadData.AccessModel(MODEL_HELPER + 121, L"Data\\Item\\Ingameshop\\", L"entrancegreen");
    }
    gLoadData.AccessModel(MODEL_POTION + 141, L"Data\\Item\\", L"requitalbox_red");
    gLoadData.AccessModel(MODEL_POTION + 142, L"Data\\Item\\", L"requitalbox_violet");
    gLoadData.AccessModel(MODEL_POTION + 143, L"Data\\Item\\", L"requitalbox_blue");
    gLoadData.AccessModel(MODEL_POTION + 144, L"Data\\Item\\", L"requitalbox_wood");

    gLoadData.AccessModel(MODEL_15GRADE_ARMOR_OBJ_ARMLEFT, L"Data\\Item\\", L"class15_armleft");
    gLoadData.AccessModel(MODEL_15GRADE_ARMOR_OBJ_ARMRIGHT, L"Data\\Item\\", L"class15_armright");
    gLoadData.AccessModel(MODEL_15GRADE_ARMOR_OBJ_BODYLEFT, L"Data\\Item\\", L"class15_bodyleft");
    gLoadData.AccessModel(MODEL_15GRADE_ARMOR_OBJ_BODYRIGHT, L"Data\\Item\\", L"class15_bodyright");
    gLoadData.AccessModel(MODEL_15GRADE_ARMOR_OBJ_BOOTLEFT, L"Data\\Item\\", L"class15_bootleft");
    gLoadData.AccessModel(MODEL_15GRADE_ARMOR_OBJ_BOOTRIGHT, L"Data\\Item\\", L"class15_bootright");
    gLoadData.AccessModel(MODEL_15GRADE_ARMOR_OBJ_HEAD, L"Data\\Item\\", L"class15_head");
    gLoadData.AccessModel(MODEL_15GRADE_ARMOR_OBJ_PANTLEFT, L"Data\\Item\\", L"class15_pantleft");
    gLoadData.AccessModel(MODEL_15GRADE_ARMOR_OBJ_PANTRIGHT, L"Data\\Item\\", L"class15_pantright");

    gLoadData.AccessModel(MODEL_CAPE_OF_FIGHTER, L"Data\\Item\\", L"Wing", 50);
    gLoadData.AccessModel(MODEL_CAPE_OF_OVERRULE, L"Data\\Item\\", L"Wing", 51);
    gLoadData.AccessModel(MODEL_WING + 135, L"Data\\Item\\", L"Wing", 50);
    LoadBitmap(L"Item\\NCcape.tga", BITMAP_NCCAPE, GL_LINEAR, GL_REPEAT);
    LoadBitmap(L"Item\\monk_manto01.TGA", BITMAP_MANTO01, GL_LINEAR, GL_REPEAT);
    LoadBitmap(L"Item\\monke_manto.TGA", BITMAP_MANTOE, GL_LINEAR, GL_REPEAT);
    g_CMonkSystem.LoadModelItem();
    for (int _nRollIndex = 0; _nRollIndex < 7; ++_nRollIndex)
        gLoadData.AccessModel(MODEL_CHAIN_DRIVE_PARCHMENT + _nRollIndex, L"Data\\Item\\", L"rollofpaper");

    gLoadData.AccessModel(MODEL_HELPER + 135, L"Data\\Item\\LuckyItem\\", L"LuckyCardgreen");
    gLoadData.AccessModel(MODEL_HELPER + 136, L"Data\\Item\\LuckyItem\\", L"LuckyCardgreen");
    gLoadData.AccessModel(MODEL_HELPER + 137, L"Data\\Item\\LuckyItem\\", L"LuckyCardgreen");
    gLoadData.AccessModel(MODEL_HELPER + 138, L"Data\\Item\\LuckyItem\\", L"LuckyCardgreen");
    gLoadData.AccessModel(MODEL_HELPER + 139, L"Data\\Item\\LuckyItem\\", L"LuckyCardgreen");
    gLoadData.AccessModel(MODEL_HELPER + 140, L"Data\\Item\\LuckyItem\\", L"LuckyCardred");
    gLoadData.AccessModel(MODEL_HELPER + 141, L"Data\\Item\\LuckyItem\\", L"LuckyCardred");
    gLoadData.AccessModel(MODEL_HELPER + 142, L"Data\\Item\\LuckyItem\\", L"LuckyCardred");
    gLoadData.AccessModel(MODEL_HELPER + 143, L"Data\\Item\\LuckyItem\\", L"LuckyCardred");
    gLoadData.AccessModel(MODEL_HELPER + 144, L"Data\\Item\\LuckyItem\\", L"LuckyCardred");

    gLoadData.AccessModel(MODEL_POTION + 160, L"Data\\Item\\LuckyItem\\", L"lucky_items01");
    gLoadData.AccessModel(MODEL_POTION + 161, L"Data\\Item\\LuckyItem\\", L"lucky_items02");

    wchar_t	szLuckySetFileName[][50] = { L"new_Helm", L"new_Armor", L"new_Pant", L"new_Glove", L"new_Boot" };
    wchar_t* szLuckySetPath = { L"Data\\Player\\LuckyItem\\" };
    wchar_t	szLuckySetPathName[50] = { L"" };
    int		nIndex = 62;

    for (int i = 0; i < 11; i++)
    {
        swprintf(szLuckySetPathName, L"%s%d\\", szLuckySetPath, nIndex);
        if (nIndex != 71)	gLoadData.AccessModel(MODEL_HELM + nIndex, szLuckySetPathName, szLuckySetFileName[0], i + 1);
        gLoadData.AccessModel(MODEL_ARMOR + nIndex, szLuckySetPathName, szLuckySetFileName[1], i + 1);
        gLoadData.AccessModel(MODEL_PANTS + nIndex, szLuckySetPathName, szLuckySetFileName[2], i + 1);
        if (nIndex != 72)	gLoadData.AccessModel(MODEL_GLOVES + nIndex, szLuckySetPathName, szLuckySetFileName[3], i + 1);
        gLoadData.AccessModel(MODEL_BOOTS + nIndex, szLuckySetPathName, szLuckySetFileName[4], i + 1);
        nIndex++;
    }

    Models[MODEL_SPEAR].Meshs[1].NoneBlendMesh = true;
    Models[MODEL_LIGHT_SABER].Meshs[1].NoneBlendMesh = true;
    Models[MODEL_STAFF_OF_RESURRECTION].Meshs[2].NoneBlendMesh = true;
    Models[MODEL_CHAOS_DRAGON_AXE].Meshs[1].NoneBlendMesh = true;
    Models[MODEL_EVENT + 9].Meshs[1].NoneBlendMesh = true;
}

void OpenItemTextures()
{
    for (int i = 0; i < 4; i++)
    {
        if (i < 3)
        {
            gLoadData.OpenTexture(MODEL_SCROLL_OF_ARCHANGEL + i, L"Item\\");
        }
        else
        {
            gLoadData.OpenTexture(MODEL_EVENT + 11 + (i - 3), L"Item\\");
        }
    }
    gLoadData.OpenTexture(MODEL_EVENT + 12, L"Item\\");
    gLoadData.OpenTexture(MODEL_EVENT + 13, L"Item\\");
    gLoadData.OpenTexture(MODEL_EVENT + 14, L"Item\\");
    gLoadData.OpenTexture(MODEL_EVENT + 15, L"Item\\");

    for (int i = 0; i < 4; i++)
    {
        gLoadData.OpenTexture(MODEL_WINGS_OF_SPIRITS + i, L"Item\\");
    }

    for (int i = 0; i < 4; i++)
    {
        gLoadData.OpenTexture(MODEL_WING_OF_STORM + i, L"Item\\");
    }
    gLoadData.OpenTexture(MODEL_CAPE_OF_EMPEROR, L"Item\\");

    LoadBitmap(L"Item\\msword01_r.jpg", BITMAP_3RDWING_LAYER, GL_LINEAR, GL_REPEAT);

    for (int i = 41; i <= 43; ++i)
        ::gLoadData.OpenTexture(MODEL_WING + i, L"Item\\");

    for (int i = 21; i <= 23; ++i)
        ::gLoadData.OpenTexture(MODEL_STAFF + i, L"Item\\");

    for (int i = 0; i < 9; ++i)
    {
        gLoadData.OpenTexture(MODEL_CHAIN_LIGHTNING_PARCHMENT + i, L"Item\\");
    }

    LoadBitmap(L"Item\\rollofpaper_R.jpg", BITMAP_ROOLOFPAPER_EFFECT_R, GL_LINEAR);

    gLoadData.OpenTexture(MODEL_DARK_HORSE_ITEM, L"Skill\\");
    gLoadData.OpenTexture(MODEL_DARK_HORSE_ITEM, L"Item\\");
    gLoadData.OpenTexture(MODEL_SPIRIT, L"Item\\");
    gLoadData.OpenTexture(MODEL_DARK_RAVEN_ITEM, L"Skill\\");
    gLoadData.OpenTexture(MODEL_DARK_RAVEN_ITEM, L"Item\\");

    gLoadData.OpenTexture(MODEL_JEWEL_OF_CREATION, L"Item\\");

    gLoadData.OpenTexture(MODEL_JEWEL_OF_GUARDIAN, L"Item\\");

    gLoadData.OpenTexture(MODEL_SIEGE_POTION, L"Item\\");
    gLoadData.OpenTexture(MODEL_HELPER + 7, L"Item\\");
    gLoadData.OpenTexture(MODEL_LIFE_STONE_ITEM, L"Item\\");
    gLoadData.OpenTexture(MODEL_EVENT + 18, L"Monster\\");

    for (int i = 0; i < 2; i++)
        gLoadData.OpenTexture(MODEL_LOCHS_FEATHER + i, L"Item\\");

    gLoadData.OpenTexture(MODEL_DARK_BREAKER, L"Item\\");
    gLoadData.OpenTexture(MODEL_THUNDER_BLADE, L"Item\\");

    gLoadData.OpenTexture(MODEL_DRAGON_SOUL_STAFF, L"Item\\");
    gLoadData.OpenTexture(MODEL_CELESTIAL_BOW, L"Item\\");

    gLoadData.OpenTexture(MODEL_HORN_OF_DINORANT, L"Skill\\");

    for (int i = 0; i < 4; i++)
        gLoadData.OpenTexture(MODEL_SCROLL_OF_EMPEROR_RING_OF_HONOR + i, L"Item\\");

    gLoadData.OpenTexture(MODEL_POTION + 27, L"Item\\");

    for (int i = 0; i < 2; i++)
        gLoadData.OpenTexture(MODEL_LOST_MAP + i, L"Item\\");

    gLoadData.OpenTexture(MODEL_ARMOR_OF_GUARDSMAN, L"Npc\\");
    gLoadData.OpenTexture(MODEL_DIVINE_SWORD_OF_ARCHANGEL, L"Item\\");
    gLoadData.OpenTexture(MODEL_DIVINE_STAFF_OF_ARCHANGEL, L"Item\\");
    gLoadData.OpenTexture(MODEL_DIVINE_CB_OF_ARCHANGEL, L"Item\\");
    gLoadData.OpenTexture(MODEL_GREAT_REIGN_CROSSBOW, L"Item\\");

    gLoadData.OpenTexture(MODEL_RUNE_BLADE, L"Item\\");
    gLoadData.OpenTexture(MODEL_GRAND_SOUL_SHIELD, L"Item\\");
    gLoadData.OpenTexture(MODEL_ELEMENTAL_SHIELD, L"Item\\");
    gLoadData.OpenTexture(MODEL_DRAGON_SPEAR, L"Item\\");
    gLoadData.OpenTexture(MODEL_ELEMENTAL_MACE, L"Item\\");

    for (int i = 0; i < 17; i++)
    {
        gLoadData.OpenTexture(MODEL_SWORD + i, L"Item\\");
        gLoadData.OpenTexture(MODEL_AXE + i, L"Item\\");
        gLoadData.OpenTexture(MODEL_MACE + i, L"Item\\");
        gLoadData.OpenTexture(MODEL_SPEAR + i, L"Item\\");
        gLoadData.OpenTexture(MODEL_STAFF + i, L"Item\\");
        gLoadData.OpenTexture(MODEL_SHIELD + i, L"Item\\");
        gLoadData.OpenTexture(MODEL_BOW + i, L"Item\\");
        gLoadData.OpenTexture(MODEL_HELPER + i, L"Item\\");
        gLoadData.OpenTexture(MODEL_WING + i, L"Item\\");
        gLoadData.OpenTexture(MODEL_POTION + i, L"Item\\");
        gLoadData.OpenTexture(MODEL_ETC + i, L"Item\\");
    }

    for (int i = 14; i <= 20; ++i)
        gLoadData.OpenTexture(MODEL_STAFF + i, L"Item\\");

    for (int i = 21; i <= 28; ++i)
        gLoadData.OpenTexture(MODEL_HELPER + i, L"Item\\");
    //. MODEL_MACE+8,9,10,11
    for (int i = 0; i < 5; i++)
        gLoadData.OpenTexture(MODEL_BATTLE_SCEPTER + i, L"Item\\");

    for (int i = 0; i < 2; i++)
        gLoadData.OpenTexture(MODEL_KNIGHT_BLADE + i, L"Item\\");

    gLoadData.OpenTexture(MODEL_ARROW_VIPER_BOW, L"Item\\");

    for (int i = 17; i < 21; i++)
    {
        gLoadData.OpenTexture(MODEL_WING + i, L"Item\\");
    }
    for (int i = 0; i < 4; i++)
    {
        gLoadData.OpenTexture(MODEL_SCROLL_OF_FIREBURST + i, L"Item\\");
    }

    gLoadData.OpenTexture(MODEL_SCROLL_OF_CHAOTIC_DISEIER, L"Item\\");
    gLoadData.OpenTexture(MODEL_SCROLL_OF_FIRE_SCREAM, L"Item\\");
    gLoadData.OpenTexture(MODEL_CRYSTAL_OF_DESTRUCTION, L"Item\\");
    gLoadData.OpenTexture(MODEL_CRYSTAL_OF_MULTI_SHOT, L"Item\\");
    gLoadData.OpenTexture(MODEL_CRYSTAL_OF_RECOVERY, L"Item\\");
    gLoadData.OpenTexture(MODEL_CRYSTAL_OF_FLAME_STRIKE, L"Item\\");
    gLoadData.OpenTexture(MODEL_SCROLL_OF_GIGANTIC_STORM, L"Item\\");
    gLoadData.OpenTexture(MODEL_SCROLL_OF_WIZARDRY_ENHANCE, L"Item\\");

    for (int i = 17; i < 19; ++i)
    {
        gLoadData.OpenTexture(MODEL_ETC + i, L"Item\\");
    }

    gLoadData.OpenTexture(MODEL_ARROW, L"Item\\Bow\\");

    gLoadData.OpenTexture(MODEL_EVENT + 4, L"Item\\");
    gLoadData.OpenTexture(MODEL_EVENT + 5, L"Item\\");
    gLoadData.OpenTexture(MODEL_EVENT + 6, L"Item\\");
    gLoadData.OpenTexture(MODEL_EVENT + 7, L"Item\\");
    gLoadData.OpenTexture(MODEL_EVENT + 8, L"Item\\");
    gLoadData.OpenTexture(MODEL_EVENT + 9, L"Item\\");
    gLoadData.OpenTexture(MODEL_EVENT + 10, L"Item\\");
    gLoadData.OpenTexture(MODEL_EVENT + 16, L"Item\\");
    gLoadData.OpenTexture(MODEL_CAPE_OF_LORD, L"Item\\");

    for (int i = 0; i < 3; i++)
        gLoadData.OpenTexture(MODEL_DEVILS_EYE + i, L"Item\\");

    for (int i = 0; i < 2; i++)
        gLoadData.OpenTexture(MODEL_POTION + 20 + i, L"Item\\");

    for (int i = 0; i < 6; ++i)
        gLoadData.OpenTexture(MODEL_SMALL_SHIELD_POTION + i, L"Item\\");

    for (int i = 0; i < 2; i++)
        gLoadData.OpenTexture(MODEL_EVENT + i, L"Item\\");

    //gLoadData.OpenTexture(MODEL_GOLD01  ,"Data\\Item\\Etc\\");
    //gLoadData.OpenTexture(MODEL_APPLE01 ,"Data\\Item\\Etc\\");
    //gLoadData.OpenTexture(MODEL_BOTTLE01,"Data\\Item\\Etc\\");
    gLoadData.OpenTexture(MODEL_COMPILED_CELE, L"Item\\");
    gLoadData.OpenTexture(MODEL_COMPILED_SOUL, L"Item\\");

    gLoadData.OpenTexture(MODEL_PACKED_JEWEL_OF_LIFE, L"Item\\");
    gLoadData.OpenTexture(MODEL_PACKED_JEWEL_OF_CREATION, L"Item\\");
    gLoadData.OpenTexture(MODEL_PACKED_JEWEL_OF_GUARDIAN, L"Item\\");
    gLoadData.OpenTexture(MODEL_PACKED_GEMSTONE, L"Item\\");
    gLoadData.OpenTexture(MODEL_PACKED_JEWEL_OF_HARMONY, L"Item\\");
    gLoadData.OpenTexture(MODEL_PACKED_JEWEL_OF_CHAOS, L"Item\\");
    gLoadData.OpenTexture(MODEL_PACKED_LOWER_REFINE_STONE, L"Item\\");
    gLoadData.OpenTexture(MODEL_PACKED_HIGHER_REFINE_STONE, L"Item\\");

    gLoadData.OpenTexture(MODEL_BONE_BLADE, L"Item\\");
    gLoadData.OpenTexture(MODEL_EXPLOSION_BLADE, L"Item\\");
    gLoadData.OpenTexture(MODEL_SOLEIL_SCEPTER, L"Item\\");
    gLoadData.OpenTexture(MODEL_SYLPH_WIND_BOW, L"Item\\");
    gLoadData.OpenTexture(MODEL_GRAND_VIPER_STAFF, L"Item\\");

    gLoadData.OpenTexture(MODEL_PUMPKIN_OF_LUCK, L"Item\\");
    gLoadData.OpenTexture(MODEL_JACK_OLANTERN_BLESSINGS, L"Item\\");
    gLoadData.OpenTexture(MODEL_JACK_OLANTERN_WRATH, L"Item\\");
    gLoadData.OpenTexture(MODEL_JACK_OLANTERN_CRY, L"Item\\");
    gLoadData.OpenTexture(MODEL_JACK_OLANTERN_FOOD, L"Item\\");
    gLoadData.OpenTexture(MODEL_JACK_OLANTERN_DRINK, L"Item\\");
    gLoadData.OpenTexture(MODEL_PINK_CHOCOLATE_BOX, L"Item\\");
    gLoadData.OpenTexture(MODEL_RED_CHOCOLATE_BOX, L"Item\\");
    gLoadData.OpenTexture(MODEL_BLUE_CHOCOLATE_BOX, L"Item\\");

    gLoadData.OpenTexture(MODEL_EVENT + 21, L"Item\\");
    gLoadData.OpenTexture(MODEL_EVENT + 22, L"Item\\");
    gLoadData.OpenTexture(MODEL_EVENT + 23, L"Item\\");
    gLoadData.OpenTexture(MODEL_MOONSTONE_PENDANT, L"Item\\");
    gLoadData.OpenTexture(MODEL_GEMSTONE, L"Item\\");
    gLoadData.OpenTexture(MODEL_JEWEL_OF_HARMONY, L"Item\\");
    gLoadData.OpenTexture(MODEL_LOWER_REFINE_STONE, L"Item\\");
    gLoadData.OpenTexture(MODEL_HIGHER_REFINE_STONE, L"Item\\");
    gLoadData.OpenTexture(MODEL_RED_RIBBON_BOX, L"Item\\");
    gLoadData.OpenTexture(MODEL_GREEN_RIBBON_BOX, L"Item\\");
    gLoadData.OpenTexture(MODEL_BLUE_RIBBON_BOX, L"Item\\");

    gLoadData.OpenTexture(MODEL_DAYBREAK, L"Item\\");
    gLoadData.OpenTexture(MODEL_SWORD_DANCER, L"Item\\");
    gLoadData.OpenTexture(MODEL_SHINING_SCEPTER, L"Item\\");
    gLoadData.OpenTexture(MODEL_ALBATROSS_BOW, L"Item\\");
    gLoadData.OpenTexture(MODEL_PLATINA_STAFF, L"Item\\");

    gLoadData.OpenTexture(MODEL_SPLINTER_OF_ARMOR, L"Item\\");
    gLoadData.OpenTexture(MODEL_BLESS_OF_GUARDIAN, L"Item\\");
    gLoadData.OpenTexture(MODEL_CLAW_OF_BEAST, L"Item\\");
    gLoadData.OpenTexture(MODEL_FRAGMENT_OF_HORN, L"Item\\");
    gLoadData.OpenTexture(MODEL_BROKEN_HORN, L"Item\\");
    gLoadData.OpenTexture(MODEL_HORN_OF_FENRIR, L"Item\\");

    gLoadData.OpenTexture(MODEL_HELPER + 46, L"Item\\partCharge1\\");
    gLoadData.OpenTexture(MODEL_HELPER + 47, L"Item\\partCharge1\\");
    gLoadData.OpenTexture(MODEL_HELPER + 48, L"Item\\partCharge1\\");

    gLoadData.OpenTexture(MODEL_POTION + 54, L"Item\\partCharge1\\");

    gLoadData.OpenTexture(MODEL_HELPER + 43, L"Item\\partCharge1\\");
    gLoadData.OpenTexture(MODEL_HELPER + 44, L"Item\\partCharge1\\");
    gLoadData.OpenTexture(MODEL_HELPER + 45, L"Item\\partCharge1\\");

    gLoadData.OpenTexture(MODEL_POTION + 53, L"Item\\partCharge1\\");

    for (int i = 0; i < 5; ++i)
    {
        gLoadData.OpenTexture(MODEL_POTION + 58 + i, L"Item\\partCharge1\\");
    }
    gLoadData.OpenTexture(MODEL_POTION + 70, L"Item\\partCharge2\\");
    gLoadData.OpenTexture(MODEL_POTION + 71, L"Item\\partCharge2\\");
    gLoadData.OpenTexture(MODEL_POTION + 72, L"Item\\partCharge2\\");
    gLoadData.OpenTexture(MODEL_POTION + 73, L"Item\\partCharge2\\");
    gLoadData.OpenTexture(MODEL_POTION + 74, L"Item\\partCharge2\\");
    gLoadData.OpenTexture(MODEL_POTION + 75, L"Item\\partCharge2\\");
    gLoadData.OpenTexture(MODEL_POTION + 76, L"Item\\partCharge2\\");
    gLoadData.OpenTexture(MODEL_POTION + 77, L"Item\\partCharge2\\");
    gLoadData.OpenTexture(MODEL_TYPE_CHARM_MIXWING + EWS_KNIGHT_1_CHARM, L"Item\\partCharge6\\");
    gLoadData.OpenTexture(MODEL_TYPE_CHARM_MIXWING + EWS_MAGICIAN_1_CHARM, L"Item\\partCharge6\\");
    gLoadData.OpenTexture(MODEL_TYPE_CHARM_MIXWING + EWS_ELF_1_CHARM, L"Item\\partCharge6\\");
    gLoadData.OpenTexture(MODEL_TYPE_CHARM_MIXWING + EWS_SUMMONER_1_CHARM, L"Item\\partCharge6\\");
    gLoadData.OpenTexture(MODEL_TYPE_CHARM_MIXWING + EWS_DARKLORD_1_CHARM, L"Item\\partCharge6\\");

    gLoadData.OpenTexture(MODEL_TYPE_CHARM_MIXWING + EWS_KNIGHT_2_CHARM, L"Item\\partCharge6\\");
    gLoadData.OpenTexture(MODEL_TYPE_CHARM_MIXWING + EWS_MAGICIAN_2_CHARM, L"Item\\partCharge6\\");
    gLoadData.OpenTexture(MODEL_TYPE_CHARM_MIXWING + EWS_ELF_2_CHARM, L"Item\\partCharge6\\");
    gLoadData.OpenTexture(MODEL_TYPE_CHARM_MIXWING + EWS_SUMMONER_2_CHARM, L"Item\\partCharge6\\");
    gLoadData.OpenTexture(MODEL_TYPE_CHARM_MIXWING + EWS_DARKKNIGHT_2_CHARM, L"Item\\partCharge6\\");

    gLoadData.OpenTexture(MODEL_HELPER + 59, L"Item\\partCharge2\\");

    gLoadData.OpenTexture(MODEL_HELPER + 54, L"Item\\partCharge2\\");
    gLoadData.OpenTexture(MODEL_HELPER + 55, L"Item\\partCharge2\\");
    gLoadData.OpenTexture(MODEL_HELPER + 56, L"Item\\partCharge2\\");
    gLoadData.OpenTexture(MODEL_HELPER + 57, L"Item\\partCharge2\\");
    gLoadData.OpenTexture(MODEL_HELPER + 58, L"Item\\partCharge2\\");
    gLoadData.OpenTexture(MODEL_POTION + 78, L"Item\\partCharge2\\");
    gLoadData.OpenTexture(MODEL_POTION + 79, L"Item\\partCharge2\\");
    gLoadData.OpenTexture(MODEL_POTION + 80, L"Item\\partCharge2\\");
    gLoadData.OpenTexture(MODEL_POTION + 81, L"Item\\partCharge2\\");
    gLoadData.OpenTexture(MODEL_POTION + 82, L"Item\\partCharge2\\");
    gLoadData.OpenTexture(MODEL_HELPER + 60, L"Item\\partCharge2\\");

    gLoadData.OpenTexture(MODEL_HELPER + 61, L"Item\\partCharge2\\");

    gLoadData.OpenTexture(MODEL_POTION + 91, L"Item\\partCharge3\\");

    gLoadData.OpenTexture(MODEL_POTION + 92, L"Item\\partCharge3\\");
    gLoadData.OpenTexture(MODEL_POTION + 93, L"Item\\partCharge3\\");
    gLoadData.OpenTexture(MODEL_POTION + 95, L"Item\\partCharge3\\");

    gLoadData.OpenTexture(MODEL_POTION + 94, L"Item\\partCharge2\\");

    gLoadData.OpenTexture(MODEL_HELPER + 62, L"Item\\partCharge4\\");
    gLoadData.OpenTexture(MODEL_HELPER + 63, L"Item\\partCharge4\\");

    gLoadData.OpenTexture(MODEL_POTION + 97, L"Item\\partCharge4\\");
    gLoadData.OpenTexture(MODEL_POTION + 98, L"Item\\partCharge4\\");

    gLoadData.OpenTexture(MODEL_POTION + 96, L"Item\\partCharge4\\");

    gLoadData.OpenTexture(MODEL_DEMON, L"Item\\partCharge4\\");
    gLoadData.OpenTexture(MODEL_SPIRIT_OF_GUARDIAN, L"Item\\partCharge4\\");

    gLoadData.OpenTexture(MODEL_PET_RUDOLF, L"Item\\xmas\\");
    gLoadData.OpenTexture(MODEL_PET_PANDA, L"Item\\");

    gLoadData.OpenTexture(MODEL_PET_UNICORN, L"Item\\partcharge7\\");
    gLoadData.OpenTexture(MODEL_PET_SKELETON, L"Item\\");
    gLoadData.OpenTexture(MODEL_HELPER + 66, L"Item\\xmas\\");
    gLoadData.OpenTexture(MODEL_POTION + 100, L"Item\\");
    gLoadData.OpenTexture(MODEL_HELPER + 69, L"Item\\partCharge5\\");
    gLoadData.OpenTexture(MODEL_HELPER + 70, L"Item\\partCharge5\\");
    gLoadData.OpenTexture(MODEL_HELPER + 81, L"Item\\partCharge6\\");
    gLoadData.OpenTexture(MODEL_HELPER + 82, L"Item\\partCharge6\\");
    gLoadData.OpenTexture(MODEL_HELPER + 93, L"Item\\partCharge6\\");
    gLoadData.OpenTexture(MODEL_HELPER + 94, L"Item\\partCharge6\\");
    gLoadData.OpenTexture(MODEL_POTION + 140, L"Item\\");

    gLoadData.OpenTexture(MODEL_CHERRY_BLOSSOM_PLAYBOX, L"Item\\cherryblossom\\");
    gLoadData.OpenTexture(MODEL_CHERRY_BLOSSOM_WINE, L"Item\\cherryblossom\\");
    gLoadData.OpenTexture(MODEL_CHERRY_BLOSSOM_RICE_CAKE, L"Item\\cherryblossom\\");
    gLoadData.OpenTexture(MODEL_CHERRY_BLOSSOM_FLOWER_PETAL, L"Item\\cherryblossom\\");
    gLoadData.OpenTexture(MODEL_POTION + 88, L"Item\\cherryblossom\\");
    gLoadData.OpenTexture(MODEL_POTION + 89, L"Item\\cherryblossom\\");
    gLoadData.OpenTexture(MODEL_GOLDEN_CHERRY_BLOSSOM_BRANCH, L"Item\\cherryblossom\\");

    gLoadData.OpenTexture(MODEL_OLD_SCROLL, L"Item\\");
    gLoadData.OpenTexture(MODEL_ILLUSION_SORCERER_COVENANT, L"Item\\");
    gLoadData.OpenTexture(MODEL_SCROLL_OF_BLOOD, L"Item\\");

    gLoadData.OpenTexture(MODEL_FLAME_OF_CONDOR, L"Item\\");
    gLoadData.OpenTexture(MODEL_FEATHER_OF_CONDOR, L"Item\\");

    gLoadData.OpenTexture(MODEL_POTION + 55, L"Item\\");
    gLoadData.OpenTexture(MODEL_POTION + 56, L"Item\\");
    gLoadData.OpenTexture(MODEL_POTION + 57, L"Item\\");

    gLoadData.OpenTexture(MODEL_POTION + 64, L"Item\\");
    gLoadData.OpenTexture(MODEL_FLAME_OF_DEATH_BEAM_KNIGHT, L"Item\\");
    gLoadData.OpenTexture(MODEL_HORN_OF_HELL_MAINE, L"Item\\");
    gLoadData.OpenTexture(MODEL_FEATHER_OF_DARK_PHOENIX, L"Item\\");
    gLoadData.OpenTexture(MODEL_EYE_OF_ABYSSAL, L"Item\\");

    gLoadData.OpenTexture(MODEL_FIRECRACKER, L"Item\\");
    gLoadData.OpenTexture(MODEL_GM_GIFT, L"Item\\");
    g_ChangeRingMgr->LoadItemTexture();

    gLoadData.OpenTexture(MODEL_CHRISTMAS_FIRECRACKER, L"Item\\XMas\\");

    gLoadData.OpenTexture(MODEL_POTION + 145, L"Item\\partCharge8\\");
    gLoadData.OpenTexture(MODEL_POTION + 146, L"Item\\partCharge8\\");
    gLoadData.OpenTexture(MODEL_POTION + 147, L"Item\\partCharge8\\");
    gLoadData.OpenTexture(MODEL_POTION + 148, L"Item\\partCharge8\\");
    gLoadData.OpenTexture(MODEL_POTION + 149, L"Item\\partCharge8\\");
    gLoadData.OpenTexture(MODEL_POTION + 150, L"Item\\partCharge8\\");

    gLoadData.OpenTexture(MODEL_HELPER + 125, L"Item\\partCharge8\\");
    gLoadData.OpenTexture(MODEL_HELPER + 126, L"Item\\partCharge8\\");
    gLoadData.OpenTexture(MODEL_HELPER + 127, L"Item\\partCharge8\\");

#ifdef LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM
    gLoadData.OpenTexture(MODEL_HELPER + 128, L"Item\\");
    gLoadData.OpenTexture(MODEL_HELPER + 129, L"Item\\");
    gLoadData.OpenTexture(MODEL_HELPER + 134, L"Item\\");
#endif //LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM
#ifdef LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM_PART_2
    gLoadData.OpenTexture(MODEL_HELPER + 130, L"Item\\");
    //	gLoadData.OpenTexture(MODEL_HELPER+131, L"Item\\");
    gLoadData.OpenTexture(MODEL_HELPER + 132, L"Item\\");
    //	gLoadData.OpenTexture(MODEL_HELPER+133, L"Item\\");
#endif //LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM_PART_2
//---------------------------------------------------------------------------------

    gLoadData.OpenTexture(MODEL_FLAMBERGE, L"Item\\");
    gLoadData.OpenTexture(MODEL_SWORD_BREAKER, L"Item\\");
    gLoadData.OpenTexture(MODEL_IMPERIAL_SWORD, L"Item\\");
    gLoadData.OpenTexture(MODEL_FROST_MACE, L"Item\\");
    gLoadData.OpenTexture(MODEL_ABSOLUTE_SCEPTER, L"Item\\");
    gLoadData.OpenTexture(MODEL_STINGER_BOW, L"Item\\");
    gLoadData.OpenTexture(MODEL_DEADLY_STAFF, L"Item\\");
    gLoadData.OpenTexture(MODEL_IMPERIAL_STAFF, L"Item\\");
    gLoadData.OpenTexture(MODEL_STAFF + 32, L"Item\\");
    gLoadData.OpenTexture(MODEL_CRIMSONGLORY, L"Item\\");
    gLoadData.OpenTexture(MODEL_SALAMANDER_SHIELD, L"Item\\");
    gLoadData.OpenTexture(MODEL_FROST_BARRIER, L"Item\\");
    gLoadData.OpenTexture(MODEL_GUARDIAN_SHILED, L"Item\\");

    for (int i = 0; i < 6; ++i)
        gLoadData.OpenTexture(MODEL_SEED_FIRE + i, L"Effect\\");

    for (int i = 0; i < 5; ++i)
        gLoadData.OpenTexture(MODEL_SPHERE_MONO + i, L"Item\\");

    for (int i = 0; i < 30; ++i)
    {
        gLoadData.OpenTexture(MODEL_SEED_SPHERE_FIRE_1 + i, L"Effect\\");
        gLoadData.OpenTexture(MODEL_SEED_SPHERE_FIRE_1 + i, L"Item\\");
    }

    gLoadData.OpenTexture(MODEL_CROSS_SHIELD, L"Item\\");

    gLoadData.OpenTexture(MODEL_AIR_LYN_BOW, L"Item\\");
    gLoadData.OpenTexture(MODEL_CHROMATIC_STAFF, L"Item\\");
    gLoadData.OpenTexture(MODEL_RAVEN_STICK, L"Item\\");
    gLoadData.OpenTexture(MODEL_SWORD + 29, L"Item\\");
    gLoadData.OpenTexture(MODEL_STRYKER_SCEPTER, L"Item\\");

    gLoadData.OpenTexture(MODEL_HELPER + 71, L"Item\\");
    gLoadData.OpenTexture(MODEL_HELPER + 72, L"Item\\");
    gLoadData.OpenTexture(MODEL_HELPER + 73, L"Item\\");
    gLoadData.OpenTexture(MODEL_HELPER + 74, L"Item\\");
    gLoadData.OpenTexture(MODEL_HELPER + 75, L"Item\\");

    gLoadData.OpenTexture(MODEL_HELPER + 97, L"Item\\Ingameshop\\");
    gLoadData.OpenTexture(MODEL_HELPER + 98, L"Item\\Ingameshop\\");
    gLoadData.OpenTexture(MODEL_POTION + 91, L"Item\\partCharge3\\");

#ifdef PBG_ADD_CHARACTERSLOT
    gLoadData.OpenTexture(MODEL_HELPER + 99, L"Item\\Ingameshop\\");
    gLoadData.OpenTexture(MODEL_SLOT_LOCK, L"Item\\Ingameshop\\");
#endif //PBG_ADD_CHARACTERSLOT

    gLoadData.OpenTexture(MODEL_POTION + 110, L"Item\\");
    gLoadData.OpenTexture(MODEL_POTION + 111, L"Item\\");

    gLoadData.OpenTexture(MODEL_SUSPICIOUS_SCRAP_OF_PAPER, L"Item\\");
    gLoadData.OpenTexture(MODEL_GAIONS_ORDER, L"Item\\");
    gLoadData.OpenTexture(MODEL_COMPLETE_SECROMICON, L"Item\\");
    for (int c = 0; c < 6; c++)
    {
        gLoadData.OpenTexture(MODEL_FIRST_SECROMICON_FRAGMENT + c, L"Item\\");
    }

    gLoadData.OpenTexture(MODEL_HELPER + 107, L"Item\\partcharge7\\");
    gLoadData.OpenTexture(MODEL_HELPER + 104, L"Item\\partcharge7\\");
    gLoadData.OpenTexture(MODEL_HELPER + 105, L"Item\\partcharge7\\");
    gLoadData.OpenTexture(MODEL_HELPER + 103, L"Item\\partcharge7\\");
    gLoadData.OpenTexture(MODEL_POTION + 133, L"Item\\partcharge7\\");

    gLoadData.OpenTexture(MODEL_HELPER + 109, L"Item\\InGameShop\\");
    gLoadData.OpenTexture(MODEL_HELPER + 110, L"Item\\InGameShop\\");
    gLoadData.OpenTexture(MODEL_HELPER + 111, L"Item\\InGameShop\\");
    gLoadData.OpenTexture(MODEL_HELPER + 112, L"Item\\InGameShop\\");
    gLoadData.OpenTexture(MODEL_HELPER + 113, L"Item\\InGameShop\\");
    gLoadData.OpenTexture(MODEL_HELPER + 114, L"Item\\InGameShop\\");
    gLoadData.OpenTexture(MODEL_HELPER + 115, L"Item\\InGameShop\\");

    gLoadData.OpenTexture(MODEL_POTION + 120, L"Item\\InGameShop\\");
    gLoadData.OpenTexture(MODEL_POTION + 121, L"Item\\InGameShop\\");
    gLoadData.OpenTexture(MODEL_POTION + 122, L"Item\\InGameShop\\");
    gLoadData.OpenTexture(MODEL_POTION + 123, L"Item\\InGameShop\\");
    gLoadData.OpenTexture(MODEL_POTION + 124, L"Item\\InGameShop\\");

    for (int k = 0; k < 6; k++)
    {
        gLoadData.OpenTexture(MODEL_POTION + 134 + k, L"Item\\InGameShop\\");
    }
    LoadBitmap(L"Item\\InGameShop\\membership_item_blue.jpg", BITMAP_PACKAGEBOX_BLUE, GL_LINEAR, GL_REPEAT);
    LoadBitmap(L"Item\\InGameShop\\membership_item_gold.jpg", BITMAP_PACKAGEBOX_GOLD, GL_LINEAR, GL_REPEAT);
    LoadBitmap(L"Item\\InGameShop\\membership_item_green.jpg", BITMAP_PACKAGEBOX_GREEN, GL_LINEAR, GL_REPEAT);
    LoadBitmap(L"Item\\InGameShop\\membership_item_pouple.jpg", BITMAP_PACKAGEBOX_PUPLE, GL_LINEAR, GL_REPEAT);
    LoadBitmap(L"Item\\InGameShop\\membership_item_red.jpg", BITMAP_PACKAGEBOX_RED, GL_LINEAR, GL_REPEAT);
    LoadBitmap(L"Item\\InGameShop\\membership_item_sky.jpg", BITMAP_PACKAGEBOX_SKY, GL_LINEAR, GL_REPEAT);

    gLoadData.OpenTexture(MODEL_HELPER + 116, L"Item\\");

    gLoadData.OpenTexture(MODEL_WING + 130, L"Item\\");
    for (int j = 0; j < 4; j++)
    {
        gLoadData.OpenTexture(MODEL_WING + 131 + j, L"Item\\Ingameshop\\");
    }

    gLoadData.OpenTexture(MODEL_HELPER + 124, L"Item\\partCharge6\\");
    gLoadData.OpenTexture(MODEL_POTION + 112, L"Item\\Ingameshop\\");
    gLoadData.OpenTexture(MODEL_POTION + 113, L"Item\\Ingameshop\\");

    for (int k = 0; k < 6; k++)
    {
        gLoadData.OpenTexture(MODEL_POTION + 114 + k, L"Item\\InGameShop\\");
    }

    LoadBitmap(L"Item\\InGameShop\\mebership_3items_green.jpg", BITMAP_INGAMESHOP_PRIMIUM6, GL_LINEAR, GL_REPEAT);
    for (int k = 0; k < 4; k++)
    {
        gLoadData.OpenTexture(MODEL_POTION + 126 + k, L"Item\\InGameShop\\");
    }

    LoadBitmap(L"Item\\InGameShop\\mebership_3items_red.jpg", BITMAP_INGAMESHOP_COMMUTERTICKET4, GL_LINEAR, GL_REPEAT);
    for (int k = 0; k < 3; k++)
    {
        gLoadData.OpenTexture(MODEL_POTION + 130 + k, L"Item\\InGameShop\\");
    }

    LoadBitmap(L"Item\\InGameShop\\mebership_3items_yellow.jpg", BITMAP_INGAMESHOP_SIZECOMMUTERTICKET3, GL_LINEAR, GL_REPEAT);

    gLoadData.OpenTexture(MODEL_HELPER + 121, L"Item\\InGameShop\\");

#ifdef PBG_ADD_GENSRANKING
    for (int _index = 0; _index < 4; ++_index)
        gLoadData.OpenTexture(MODEL_POTION + 141 + _index, L"Item\\");
#endif //PBG_ADD_GENSRANKING

    gLoadData.OpenTexture(MODEL_15GRADE_ARMOR_OBJ_ARMLEFT, L"Item\\");		// 14, 15 efeito chifres brancos
    gLoadData.OpenTexture(MODEL_15GRADE_ARMOR_OBJ_ARMRIGHT, L"Item\\");		// 14, 15
    gLoadData.OpenTexture(MODEL_15GRADE_ARMOR_OBJ_BODYLEFT, L"Item\\");		// 14, 15
    gLoadData.OpenTexture(MODEL_15GRADE_ARMOR_OBJ_BODYRIGHT, L"Item\\");	// 14, 15
    gLoadData.OpenTexture(MODEL_15GRADE_ARMOR_OBJ_BOOTLEFT, L"Item\\");		// 14, 15
    gLoadData.OpenTexture(MODEL_15GRADE_ARMOR_OBJ_BOOTRIGHT, L"Item\\");	// 14, 15
    gLoadData.OpenTexture(MODEL_15GRADE_ARMOR_OBJ_HEAD, L"Item\\");		// 14, 15
    gLoadData.OpenTexture(MODEL_15GRADE_ARMOR_OBJ_PANTLEFT, L"Item\\");		// 14, 15
    gLoadData.OpenTexture(MODEL_15GRADE_ARMOR_OBJ_PANTRIGHT, L"Item\\");	// 14, 15
    LoadBitmap(L"Item\\rgb_mix.jpg", BITMAP_RGB_MIX, GL_LINEAR, GL_REPEAT);

    gLoadData.OpenTexture(MODEL_CAPE_OF_FIGHTER, L"Item\\");
    gLoadData.OpenTexture(MODEL_CAPE_OF_OVERRULE, L"Item\\");
    gLoadData.OpenTexture(MODEL_WING + 135, L"Item\\");
    g_CMonkSystem.LoadModelItemTexture();
    for (int _nRollIndex = 0; _nRollIndex < 7; ++_nRollIndex)
        gLoadData.OpenTexture(MODEL_CHAIN_DRIVE_PARCHMENT + _nRollIndex, L"Item\\");

    LoadBitmap(L"Item\\PhoenixSoul_render.JPG", BITMAP_PHOENIXSOULWING, GL_LINEAR, GL_REPEAT);

    gLoadData.OpenTexture(MODEL_HELPER + 135, L"Item\\LuckyItem\\");
    gLoadData.OpenTexture(MODEL_HELPER + 136, L"Item\\LuckyItem\\");
    gLoadData.OpenTexture(MODEL_HELPER + 137, L"Item\\LuckyItem\\");
    gLoadData.OpenTexture(MODEL_HELPER + 138, L"Item\\LuckyItem\\");
    gLoadData.OpenTexture(MODEL_HELPER + 139, L"Item\\LuckyItem\\");
    gLoadData.OpenTexture(MODEL_HELPER + 140, L"Item\\LuckyItem\\");
    gLoadData.OpenTexture(MODEL_HELPER + 141, L"Item\\LuckyItem\\");
    gLoadData.OpenTexture(MODEL_HELPER + 142, L"Item\\LuckyItem\\");
    gLoadData.OpenTexture(MODEL_HELPER + 143, L"Item\\LuckyItem\\");
    gLoadData.OpenTexture(MODEL_HELPER + 144, L"Item\\LuckyItem\\");

    wchar_t* szLuckySetPath = { L"Player\\LuckyItem\\" };
    wchar_t	szLuckySetPathName[50] = { L"" };
    int		nIndex = 62;

    for (int i = 0; i < 11; i++)
    {
        swprintf(szLuckySetPathName, L"%s%d\\", szLuckySetPath, nIndex);
        if (nIndex != 71)	gLoadData.OpenTexture(MODEL_HELM + nIndex, szLuckySetPathName);
        gLoadData.OpenTexture(MODEL_ARMOR + nIndex, szLuckySetPathName);
        gLoadData.OpenTexture(MODEL_PANTS + nIndex, szLuckySetPathName);
        if (nIndex != 72)	gLoadData.OpenTexture(MODEL_GLOVES + nIndex, szLuckySetPathName);
        gLoadData.OpenTexture(MODEL_BOOTS + nIndex, szLuckySetPathName);
        nIndex++;
    }

    gLoadData.OpenTexture(MODEL_POTION + 160, L"Item\\LuckyItem\\");
    gLoadData.OpenTexture(MODEL_POTION + 161, L"Item\\LuckyItem\\");

    swprintf(szLuckySetPathName, L"Player\\LuckyItem\\65\\InvenArmorMale40_luck.tga");
    LoadBitmap(szLuckySetPathName, BITMAP_INVEN_ARMOR + 6);
    swprintf(szLuckySetPathName, L"Player\\LuckyItem\\65\\InvenPantsMale40_luck.tga");
    LoadBitmap(szLuckySetPathName, BITMAP_INVEN_PANTS + 6);
    swprintf(szLuckySetPathName, L"Player\\LuckyItem\\70\\InvenArmorMale41_luck.tga");
    LoadBitmap(szLuckySetPathName, BITMAP_INVEN_ARMOR + 7);
    swprintf(szLuckySetPathName, L"Player\\LuckyItem\\70\\InvenPantsMale41_luck.tga");
    LoadBitmap(szLuckySetPathName, BITMAP_INVEN_PANTS + 7);

}

void DeleteNpcs()
{
    for (int i = MODEL_NPC_BEGIN; i < MODEL_NPC_END; i++)
    {
        if(Models[i].Version)
            Models[i].Release();
    }

    for (int i = SOUND_NPC; i < SOUND_NPC_END; i++)
        ReleaseBuffer(i);
}

void OpenNpc(int Type)
{
    BMD* b = &Models[Type];
    if (b->NumActions > 0) return;

    switch (Type)
    {
    case MODEL_MERCHANT_FEMALE:
        gLoadData.AccessModel(MODEL_MERCHANT_FEMALE, L"Data\\Npc\\", L"Female", 1);

        for (int i = 0; i < 2; i++)
        {
            gLoadData.AccessModel(MODEL_MERCHANT_FEMALE_HEAD + i, L"Data\\Npc\\", L"FemaleHead", i + 1);
            gLoadData.AccessModel(MODEL_MERCHANT_FEMALE_UPPER + i, L"Data\\Npc\\", L"FemaleUpper", i + 1);
            gLoadData.AccessModel(MODEL_MERCHANT_FEMALE_LOWER + i, L"Data\\Npc\\", L"FemaleLower", i + 1);
            gLoadData.AccessModel(MODEL_MERCHANT_FEMALE_BOOTS + i, L"Data\\Npc\\", L"FemaleBoots", i + 1);
            gLoadData.OpenTexture(MODEL_MERCHANT_FEMALE_HEAD + i, L"Npc\\");
            gLoadData.OpenTexture(MODEL_MERCHANT_FEMALE_UPPER + i, L"Npc\\");
            gLoadData.OpenTexture(MODEL_MERCHANT_FEMALE_LOWER + i, L"Npc\\");
            gLoadData.OpenTexture(MODEL_MERCHANT_FEMALE_BOOTS + i, L"Npc\\");
        }
        break;
    case MODEL_MERCHANT_MAN:
        gLoadData.AccessModel(MODEL_MERCHANT_MAN, L"Data\\Npc\\", L"Man", 1);

        for (int i = 0; i < 2; i++)
        {
            gLoadData.AccessModel(MODEL_MERCHANT_MAN_HEAD + i, L"Data\\Npc\\", L"ManHead", i + 1);
            gLoadData.AccessModel(MODEL_MERCHANT_MAN_UPPER + i, L"Data\\Npc\\", L"ManUpper", i + 1);
            gLoadData.AccessModel(MODEL_MERCHANT_MAN_GLOVES + i, L"Data\\Npc\\", L"ManGloves", i + 1);
            gLoadData.AccessModel(MODEL_MERCHANT_MAN_BOOTS + i, L"Data\\Npc\\", L"ManBoots", i + 1);
            gLoadData.OpenTexture(MODEL_MERCHANT_MAN_HEAD + i, L"Npc\\");
            gLoadData.OpenTexture(MODEL_MERCHANT_MAN_UPPER + i, L"Npc\\");
            gLoadData.OpenTexture(MODEL_MERCHANT_MAN_GLOVES + i, L"Npc\\");
            gLoadData.OpenTexture(MODEL_MERCHANT_MAN_BOOTS + i, L"Npc\\");
        }
        break;
    case MODEL_MERCHANT_GIRL:
        gLoadData.AccessModel(MODEL_MERCHANT_GIRL, L"Data\\Npc\\", L"Girl", 1);

        for (int i = 0; i < 2; i++)
        {
            gLoadData.AccessModel(MODEL_MERCHANT_GIRL_HEAD + i, L"Data\\Npc\\", L"GirlHead", i + 1);
            gLoadData.AccessModel(MODEL_MERCHANT_GIRL_UPPER + i, L"Data\\Npc\\", L"GirlUpper", i + 1);
            gLoadData.AccessModel(MODEL_MERCHANT_GIRL_LOWER + i, L"Data\\Npc\\", L"GirlLower", i + 1);
            gLoadData.OpenTexture(MODEL_MERCHANT_GIRL_HEAD + i, L"Npc\\");
            gLoadData.OpenTexture(MODEL_MERCHANT_GIRL_UPPER + i, L"Npc\\");
            gLoadData.OpenTexture(MODEL_MERCHANT_GIRL_LOWER + i, L"Npc\\");
        }
        break;
    case MODEL_SMITH:
        gLoadData.AccessModel(MODEL_SMITH, L"Data\\Npc\\", L"Smith", 1);
        LoadWaveFile(SOUND_NPC_BLACK_SMITH, L"Data\\Sound\\nBlackSmith.wav", 1);
        break;
    case MODEL_SCIENTIST:
        gLoadData.AccessModel(MODEL_SCIENTIST, L"Data\\Npc\\", L"Wizard", 1);
        break;
    case MODEL_SNOW_MERCHANT:
        gLoadData.AccessModel(MODEL_SNOW_MERCHANT, L"Data\\Npc\\", L"SnowMerchant", 1);
        break;
    case MODEL_SNOW_SMITH:
        gLoadData.AccessModel(MODEL_SNOW_SMITH, L"Data\\Npc\\", L"SnowSmith", 1);
        break;
    case MODEL_SNOW_WIZARD:
        gLoadData.AccessModel(MODEL_SNOW_WIZARD, L"Data\\Npc\\", L"SnowWizard", 1);
        break;
    case MODEL_ELF_WIZARD:
        gLoadData.AccessModel(MODEL_ELF_WIZARD, L"Data\\Npc\\", L"ElfWizard", 1);
        LoadWaveFile(SOUND_NPC_HARP, L"Data\\Sound\\nHarp.wav", 1);
        break;
    case MODEL_ELF_MERCHANT:
        gLoadData.AccessModel(MODEL_ELF_MERCHANT, L"Data\\Npc\\", L"ElfMerchant", 1);
        break;
    case MODEL_MASTER:
        gLoadData.AccessModel(MODEL_MASTER, L"Data\\Npc\\", L"Master", 1);
        break;
    case MODEL_STORAGE:
        gLoadData.AccessModel(MODEL_STORAGE, L"Data\\Npc\\", L"Storage", 1);
        break;
    case MODEL_TOURNAMENT:
        gLoadData.AccessModel(MODEL_TOURNAMENT, L"Data\\Npc\\", L"Tournament", 1);
        break;
    case MODEL_MIX_NPC:
        gLoadData.AccessModel(MODEL_MIX_NPC, L"Data\\Npc\\", L"MixNpc", 1);
        LoadWaveFile(SOUND_NPC_MIX, L"Data\\Sound\\nMix.wav", 1);
        break;
    case MODEL_REFINERY_NPC:
        gLoadData.AccessModel(MODEL_REFINERY_NPC, L"Data\\Npc\\", L"os");
        gLoadData.OpenTexture(Type, L"Npc\\");
        break;
    case MODEL_RECOVERY_NPC:
        gLoadData.AccessModel(MODEL_RECOVERY_NPC, L"Data\\Npc\\", L"je");
        gLoadData.OpenTexture(Type, L"Npc\\");
        break;
    case MODEL_NPC_DEVILSQUARE:
        gLoadData.AccessModel(MODEL_NPC_DEVILSQUARE, L"Data\\Npc\\", L"DevilNpc", 1);
        break;

    case MODEL_NPC_SEVINA:
        gLoadData.AccessModel(MODEL_NPC_SEVINA, L"Data\\Npc\\", L"Sevina", 1);
        gLoadData.OpenTexture(Type, L"Npc\\");
        break;
    case MODEL_NPC_ARCHANGEL:
        gLoadData.AccessModel(MODEL_NPC_ARCHANGEL, L"Data\\Npc\\", L"BloodCastle", 1);
        gLoadData.OpenTexture(Type, L"Npc\\");
        break;
    case MODEL_NPC_ARCHANGEL_MESSENGER:
        gLoadData.AccessModel(MODEL_NPC_ARCHANGEL_MESSENGER, L"Data\\Npc\\", L"BloodCastle", 2);
        gLoadData.OpenTexture(Type, L"Npc\\");
        break;

        //  데비아스, 로랜시아 추가 상점 NPC
    case MODEL_DEVIAS_TRADER:
        gLoadData.AccessModel(MODEL_DEVIAS_TRADER, L"Data\\Npc\\", L"DeviasTrader", 1);
        gLoadData.OpenTexture(Type, L"Npc\\");
        break;

#ifdef _PVP_ATTACK_GUARD
    case MODEL_ANGEL:
        gLoadData.AccessModel(MODEL_ANGEL, L"Data\\Player\\", L"Angel");
        gLoadData.OpenTexture(MODEL_ANGEL, L"Npc\\");
        break;
#endif	// _PVP_ATTACK_GUARD
    case MODEL_NPC_BREEDER:    //  조련사 NPC.
        gLoadData.AccessModel(MODEL_NPC_BREEDER, L"Data\\Npc\\", L"Breeder");
        gLoadData.OpenTexture(MODEL_NPC_BREEDER, L"Npc\\");
        break;
#ifdef _PVP_MURDERER_HERO_ITEM
    case MODEL_HERO_SHOP:	// 영웅 상점
        gLoadData.AccessModel(MODEL_HERO_SHOP, L"Data\\Npc\\", L"HeroNpc");
        gLoadData.OpenTexture(MODEL_HERO_SHOP, L"Npc\\");
        break;
#endif	// _PVP_MURDERER_HERO_ITEM

    case MODEL_NPC_CAPATULT_ATT:
        gLoadData.AccessModel(MODEL_NPC_CAPATULT_ATT, L"Data\\Npc\\", L"Model_Npc_Catapult_Att");
        gLoadData.OpenTexture(MODEL_NPC_CAPATULT_ATT, L"Npc\\");
        break;

    case MODEL_NPC_CAPATULT_DEF:
        gLoadData.AccessModel(MODEL_NPC_CAPATULT_DEF, L"Data\\Npc\\", L"Model_Npc_Catapult_Def");
        gLoadData.OpenTexture(MODEL_NPC_CAPATULT_DEF, L"Npc\\");
        break;

    case MODEL_NPC_SENATUS:
        gLoadData.AccessModel(MODEL_NPC_SENATUS, L"Data\\Npc\\", L"NpcSenatus");
        gLoadData.OpenTexture(MODEL_NPC_SENATUS, L"Npc\\");
        break;

    case MODEL_NPC_GATE_SWITCH:
        gLoadData.AccessModel(MODEL_NPC_GATE_SWITCH, L"Data\\Npc\\", L"NpcGateSwitch");
        gLoadData.OpenTexture(MODEL_NPC_GATE_SWITCH, L"Npc\\");
        break;

    case MODEL_NPC_CROWN:
        gLoadData.AccessModel(MODEL_NPC_CROWN, L"Data\\Npc\\", L"NpcCrown");
        gLoadData.OpenTexture(MODEL_NPC_CROWN, L"Npc\\");
        break;

    case MODEL_NPC_CHECK_FLOOR:
        gLoadData.AccessModel(MODEL_NPC_CHECK_FLOOR, L"Data\\Npc\\", L"NpcCheckFloor");
        gLoadData.OpenTexture(MODEL_NPC_CHECK_FLOOR, L"Npc\\");
        break;

    case MODEL_NPC_CLERK:
        gLoadData.AccessModel(MODEL_NPC_CLERK, L"Data\\Npc\\", L"NpcClerk");
        gLoadData.OpenTexture(MODEL_NPC_CLERK, L"Npc\\");
        break;

    case MODEL_NPC_BARRIER:
        gLoadData.AccessModel(MODEL_NPC_BARRIER, L"Data\\Npc\\", L"NpcBarrier");
        gLoadData.OpenTexture(MODEL_NPC_BARRIER, L"Npc\\");
        break;
    case MODEL_NPC_SERBIS:
        gLoadData.AccessModel(MODEL_NPC_SERBIS, L"Data\\Npc\\", L"npc_mulyak");
        gLoadData.OpenTexture(MODEL_NPC_SERBIS, L"Npc\\");
        break;
    case MODEL_KALIMA_SHOP:
        gLoadData.AccessModel(MODEL_KALIMA_SHOP, L"Data\\Npc\\", L"kalnpc");
        gLoadData.OpenTexture(MODEL_KALIMA_SHOP, L"Npc\\");
        break;
    case MODEL_BC_NPC1:
        gLoadData.AccessModel(MODEL_BC_NPC1, L"Data\\Npc\\", L"npcpharmercy1");
        gLoadData.OpenTexture(MODEL_BC_NPC1, L"Npc\\");
        gLoadData.AccessModel(MODEL_BC_BOX, L"Data\\Npc\\", L"box");
        gLoadData.OpenTexture(MODEL_BC_BOX, L"Npc\\");
        break;
    case MODEL_BC_NPC2:
        gLoadData.AccessModel(MODEL_BC_NPC2, L"Data\\Npc\\", L"npcpharmercy2");
        gLoadData.OpenTexture(MODEL_BC_NPC2, L"Npc\\");
        gLoadData.AccessModel(MODEL_BC_BOX, L"Data\\Npc\\", L"box");
        gLoadData.OpenTexture(MODEL_BC_BOX, L"Npc\\");
        break;
    case MODEL_CRYWOLF_STATUE:
        gLoadData.AccessModel(MODEL_CRYWOLF_STATUE, L"Data\\Object35\\", L"Object82");
        gLoadData.OpenTexture(MODEL_CRYWOLF_STATUE, L"Object35\\");
        break;
    case MODEL_CRYWOLF_ALTAR1:
        gLoadData.AccessModel(MODEL_CRYWOLF_ALTAR1, L"Data\\Object35\\", L"Object57");
        gLoadData.OpenTexture(MODEL_CRYWOLF_ALTAR1, L"Object35\\");
        break;
    case MODEL_CRYWOLF_ALTAR2:
        gLoadData.AccessModel(MODEL_CRYWOLF_ALTAR2, L"Data\\Object35\\", L"Object57");
        gLoadData.OpenTexture(MODEL_CRYWOLF_ALTAR2, L"Object35\\");
        break;
    case MODEL_CRYWOLF_ALTAR3:
        gLoadData.AccessModel(MODEL_CRYWOLF_ALTAR3, L"Data\\Object35\\", L"Object57");
        gLoadData.OpenTexture(MODEL_CRYWOLF_ALTAR3, L"Object35\\");
        break;
    case MODEL_CRYWOLF_ALTAR4:
        gLoadData.AccessModel(MODEL_CRYWOLF_ALTAR4, L"Data\\Object35\\", L"Object57");
        gLoadData.OpenTexture(MODEL_CRYWOLF_ALTAR4, L"Object35\\");
        break;
    case MODEL_CRYWOLF_ALTAR5:
        gLoadData.AccessModel(MODEL_CRYWOLF_ALTAR5, L"Data\\Object35\\", L"Object57");
        gLoadData.OpenTexture(MODEL_CRYWOLF_ALTAR5, L"Object35\\");
        break;
    case MODEL_KANTURU2ND_ENTER_NPC:
    {
        gLoadData.AccessModel(MODEL_KANTURU2ND_ENTER_NPC, L"Data\\Npc\\", L"to3gate");
        gLoadData.OpenTexture(MODEL_KANTURU2ND_ENTER_NPC, L"Npc\\");
    }
    break;
    case MODEL_SMELTING_NPC:
        gLoadData.AccessModel(MODEL_SMELTING_NPC, L"Data\\Npc\\", L"Elpis");
        gLoadData.OpenTexture(MODEL_SMELTING_NPC, L"Npc\\");
        break;
    case MODEL_NPC_DEVIN:
        gLoadData.AccessModel(MODEL_NPC_DEVIN, L"Data\\Npc\\", L"devin");
        gLoadData.OpenTexture(MODEL_NPC_DEVIN, L"Npc\\");
        break;
    case MODEL_NPC_QUARREL:
        gLoadData.AccessModel(MODEL_NPC_QUARREL, L"Data\\Npc\\", L"WereQuarrel");
        gLoadData.OpenTexture(MODEL_NPC_QUARREL, L"Monster\\");
        break;
    case MODEL_NPC_CASTEL_GATE:
        gLoadData.AccessModel(MODEL_NPC_CASTEL_GATE, L"Data\\Npc\\", L"cry2doorhead");
        gLoadData.OpenTexture(MODEL_NPC_CASTEL_GATE, L"Npc\\");
        break;
    case MODEL_CURSEDTEMPLE_ENTER_NPC:
        gLoadData.AccessModel(MODEL_CURSEDTEMPLE_ENTER_NPC, L"Data\\Npc\\", L"mirazu");
        gLoadData.OpenTexture(MODEL_CURSEDTEMPLE_ENTER_NPC, L"Npc\\");
        break;
    case MODEL_CURSEDTEMPLE_ALLIED_NPC:
        gLoadData.AccessModel(MODEL_CURSEDTEMPLE_ALLIED_NPC, L"Data\\Npc\\", L"allied");
        gLoadData.OpenTexture(MODEL_CURSEDTEMPLE_ALLIED_NPC, L"Npc\\");
        break;
    case MODEL_CURSEDTEMPLE_ILLUSION_NPC:
        gLoadData.AccessModel(MODEL_CURSEDTEMPLE_ILLUSION_NPC, L"Data\\Npc\\", L"illusion");
        gLoadData.OpenTexture(MODEL_CURSEDTEMPLE_ILLUSION_NPC, L"Npc\\");
        break;
    case MODEL_CURSEDTEMPLE_STATUE:
        gLoadData.AccessModel(MODEL_CURSEDTEMPLE_STATUE, L"Data\\Npc\\", L"songsom");
        gLoadData.OpenTexture(MODEL_CURSEDTEMPLE_STATUE, L"Npc\\");
        break;
    case MODEL_CURSEDTEMPLE_ALLIED_BASKET:
        gLoadData.AccessModel(MODEL_CURSEDTEMPLE_ALLIED_BASKET, L"Data\\Npc\\", L"songko");
        gLoadData.OpenTexture(MODEL_CURSEDTEMPLE_ALLIED_BASKET, L"Npc\\");
        break;
    case MODEL_CURSEDTEMPLE_ILLUSION__BASKET:
        gLoadData.AccessModel(MODEL_CURSEDTEMPLE_ILLUSION__BASKET, L"Data\\Npc\\", L"songk2");
        gLoadData.OpenTexture(MODEL_CURSEDTEMPLE_ILLUSION__BASKET, L"Npc\\");
        break;
    case MODEL_WEDDING_NPC:
        gLoadData.AccessModel(MODEL_WEDDING_NPC, L"Data\\Npc\\", L"Wedding");
        gLoadData.OpenTexture(MODEL_WEDDING_NPC, L"Npc\\");
        break;
    case MODEL_ELBELAND_SILVIA:
        gLoadData.AccessModel(MODEL_ELBELAND_SILVIA, L"Data\\Npc\\", L"silvia");
        gLoadData.OpenTexture(MODEL_ELBELAND_SILVIA, L"Npc\\");
        break;
    case MODEL_ELBELAND_RHEA:
        gLoadData.AccessModel(MODEL_ELBELAND_RHEA, L"Data\\Npc\\", L"rhea");
        gLoadData.OpenTexture(MODEL_ELBELAND_RHEA, L"Npc\\");
        break;
    case MODEL_ELBELAND_MARCE:
        gLoadData.AccessModel(MODEL_ELBELAND_MARCE, L"Data\\Npc\\", L"marce");
        gLoadData.OpenTexture(MODEL_ELBELAND_MARCE, L"Npc\\");
        break;
    case MODEL_NPC_CHERRYBLOSSOM:
        gLoadData.AccessModel(MODEL_NPC_CHERRYBLOSSOM, L"Data\\Npc\\cherryblossom\\", L"cherry_blossom");
        gLoadData.OpenTexture(MODEL_NPC_CHERRYBLOSSOM, L"Npc\\cherryblossom\\");
        break;
    case MODEL_NPC_CHERRYBLOSSOMTREE:
        gLoadData.AccessModel(MODEL_NPC_CHERRYBLOSSOMTREE, L"Data\\Npc\\cherryblossom\\", L"sakuratree");
        gLoadData.OpenTexture(MODEL_NPC_CHERRYBLOSSOMTREE, L"Npc\\cherryblossom\\");
        break;
    case MODEL_SEED_MASTER:
        gLoadData.AccessModel(MODEL_SEED_MASTER, L"Data\\Npc\\", L"goblinmaster");
        gLoadData.OpenTexture(MODEL_SEED_MASTER, L"Npc\\");
        break;
    case MODEL_SEED_INVESTIGATOR:
        gLoadData.AccessModel(MODEL_SEED_INVESTIGATOR, L"Data\\Npc\\", L"seedgoblin");
        gLoadData.OpenTexture(MODEL_SEED_INVESTIGATOR, L"Npc\\");
        break;
    case MODEL_LITTLESANTA:
    case MODEL_LITTLESANTA + 1:
    case MODEL_LITTLESANTA + 2:
    case MODEL_LITTLESANTA + 3:
    case MODEL_LITTLESANTA + 4:
    case MODEL_LITTLESANTA + 5:
    case MODEL_LITTLESANTA + 6:
    case MODEL_LITTLESANTA + 7:
    {
        gLoadData.AccessModel(MODEL_LITTLESANTA + (Type - MODEL_LITTLESANTA), L"Data\\Npc\\", L"xmassanta");

        int _index = 9;
        int _index_end = 14;

        Models[Type].Textures->FileName[_index_end] = NULL;
        for (int i = _index_end - 1; i > _index; i--)
        {
            Models[Type].Textures->FileName[i] = Models[Type].Textures->FileName[i - 1];
        }
        int _temp = (Type - MODEL_LITTLESANTA) + 1;
        wchar_t _temp2[10] = { 0, };
        _itow(_temp, _temp2, 10);
        Models[Type].Textures->FileName[_index] = _temp2[0];

        gLoadData.OpenTexture(MODEL_LITTLESANTA + (Type - MODEL_LITTLESANTA), L"Npc\\");
    }
    break;
    case MODEL_XMAS2008_SNOWMAN:
    {
        gLoadData.AccessModel(MODEL_XMAS2008_SNOWMAN, L"Data\\Item\\xmas\\", L"snowman");
        gLoadData.AccessModel(MODEL_XMAS2008_SNOWMAN_HEAD, L"Data\\Item\\xmas\\", L"snowman_die_head_model");
        gLoadData.AccessModel(MODEL_XMAS2008_SNOWMAN_BODY, L"Data\\Item\\xmas\\", L"snowman_die_body");
        gLoadData.OpenTexture(MODEL_XMAS2008_SNOWMAN, L"Item\\xmas\\");
        gLoadData.OpenTexture(MODEL_XMAS2008_SNOWMAN_HEAD, L"Item\\xmas\\");
        gLoadData.OpenTexture(MODEL_XMAS2008_SNOWMAN_BODY, L"Item\\xmas\\");

        LoadWaveFile(SOUND_XMAS_SNOWMAN_WALK_1, L"Data\\Sound\\xmas\\SnowMan_Walk01.wav");
        LoadWaveFile(SOUND_XMAS_SNOWMAN_ATTACK_1, L"Data\\Sound\\xmas\\SnowMan_Attack01.wav");
        LoadWaveFile(SOUND_XMAS_SNOWMAN_ATTACK_2, L"Data\\Sound\\xmas\\SnowMan_Attack02.wav");
        LoadWaveFile(SOUND_XMAS_SNOWMAN_DAMAGE_1, L"Data\\Sound\\xmas\\SnowMan_Damage01.wav");
        LoadWaveFile(SOUND_XMAS_SNOWMAN_DEATH_1, L"Data\\Sound\\xmas\\SnowMan_Death01.wav");
    }
    break;
    case MODEL_XMAS2008_SNOWMAN_NPC:
        gLoadData.AccessModel(MODEL_XMAS2008_SNOWMAN_NPC, L"Data\\Npc\\", L"snowman");
        gLoadData.OpenTexture(MODEL_XMAS2008_SNOWMAN_NPC, L"Npc\\");
        break;
    case MODEL_XMAS2008_SANTA_NPC:
        gLoadData.AccessModel(MODEL_XMAS2008_SANTA_NPC, L"Data\\Npc\\", L"npcsanta");
        gLoadData.OpenTexture(MODEL_XMAS2008_SANTA_NPC, L"Npc\\");
        break;
    case MODEL_DUEL_NPC_TITUS:
        gLoadData.AccessModel(MODEL_DUEL_NPC_TITUS, L"Data\\Npc\\", L"titus");
        gLoadData.OpenTexture(MODEL_DUEL_NPC_TITUS, L"Npc\\");

        LoadWaveFile(SOUND_DUEL_NPC_IDLE_1, L"Data\\Sound\\w64\\GatekeeperTitus.wav");
        break;
    case MODEL_GAMBLE_NPC_MOSS:
        gLoadData.AccessModel(MODEL_GAMBLE_NPC_MOSS, L"Data\\Npc\\", L"gambler_moss");
        gLoadData.OpenTexture(MODEL_GAMBLE_NPC_MOSS, L"Npc\\");
        break;
    case MODEL_DOPPELGANGER_NPC_LUGARD:
        gLoadData.AccessModel(MODEL_DOPPELGANGER_NPC_LUGARD, L"Data\\Npc\\", L"Lugard");
        gLoadData.OpenTexture(MODEL_DOPPELGANGER_NPC_LUGARD, L"Npc\\");
        LoadWaveFile(SOUND_DOPPELGANGER_LUGARD_BREATH, L"Data\\Sound\\Doppelganger\\Lugard.wav");
        break;
    case MODEL_DOPPELGANGER_NPC_BOX:
        gLoadData.AccessModel(MODEL_DOPPELGANGER_NPC_BOX, L"Data\\Npc\\", L"DoppelgangerBox");
        gLoadData.OpenTexture(MODEL_DOPPELGANGER_NPC_BOX, L"Npc\\");
        LoadWaveFile(SOUND_DOPPELGANGER_JEWELBOX_OPEN, L"Data\\Sound\\Doppelganger\\treasurebox_open.wav");
        break;
    case MODEL_DOPPELGANGER_NPC_GOLDENBOX:
        gLoadData.AccessModel(MODEL_DOPPELGANGER_NPC_GOLDENBOX, L"Data\\Npc\\", L"DoppelgangerBox");
        gLoadData.OpenTexture(MODEL_DOPPELGANGER_NPC_GOLDENBOX, L"Npc\\");
        LoadWaveFile(SOUND_DOPPELGANGER_JEWELBOX_OPEN, L"Data\\Sound\\Doppelganger\\treasurebox_open.wav");
        break;
    case MODAL_GENS_NPC_DUPRIAN:
        gLoadData.AccessModel(MODAL_GENS_NPC_DUPRIAN, L"Data\\Npc\\", L"duprian");
        gLoadData.OpenTexture(MODAL_GENS_NPC_DUPRIAN, L"Npc\\");
        break;
    case MODAL_GENS_NPC_BARNERT:
        gLoadData.AccessModel(MODAL_GENS_NPC_BARNERT, L"Data\\Npc\\", L"barnert");
        gLoadData.OpenTexture(MODAL_GENS_NPC_BARNERT, L"Npc\\");
        break;
    case MODEL_UNITEDMARKETPLACE_CHRISTIN:
        gLoadData.AccessModel(MODEL_UNITEDMARKETPLACE_CHRISTIN, L"Data\\Npc\\", L"UnitedMarketPlace_christine");
        gLoadData.OpenTexture(MODEL_UNITEDMARKETPLACE_CHRISTIN, L"Npc\\");
        break;
    case MODEL_UNITEDMARKETPLACE_RAUL:
        gLoadData.AccessModel(MODEL_UNITEDMARKETPLACE_RAUL, L"Data\\Npc\\", L"UnitedMarkedPlace_raul");
        gLoadData.OpenTexture(MODEL_UNITEDMARKETPLACE_RAUL, L"Npc\\");
        break;
    case MODEL_UNITEDMARKETPLACE_JULIA:
        gLoadData.AccessModel(MODEL_UNITEDMARKETPLACE_JULIA, L"Data\\Npc\\", L"UnitedMarkedPlace_julia");
        gLoadData.OpenTexture(MODEL_UNITEDMARKETPLACE_JULIA, L"Npc\\");
        break;
    case MODEL_KARUTAN_NPC_REINA:	// 로랜시장 NPC 잡화상인 크리스틴과 동일.
        gLoadData.AccessModel(MODEL_KARUTAN_NPC_REINA, L"Data\\Npc\\", L"UnitedMarketPlace_christine");
        gLoadData.OpenTexture(MODEL_KARUTAN_NPC_REINA, L"Npc\\");
        break;
    case MODEL_KARUTAN_NPC_VOLVO:
        gLoadData.AccessModel(MODEL_KARUTAN_NPC_VOLVO, L"Data\\Npc\\", L"volvo");
        gLoadData.OpenTexture(MODEL_KARUTAN_NPC_VOLVO, L"Npc\\");
        break;
    case MODEL_LUCKYITEM_NPC:
        gLoadData.AccessModel(MODEL_LUCKYITEM_NPC, L"Data\\Npc\\LuckyItem\\", L"npc_burial");
        gLoadData.OpenTexture(MODEL_LUCKYITEM_NPC, L"Npc\\LuckyItem\\");
        break;
    case MODEL_TERSIA:
        gLoadData.AccessModel(MODEL_TERSIA, L"Data\\Npc\\", L"tersia");
        gLoadData.OpenTexture(MODEL_TERSIA, L"Npc\\");
    case MODEL_BENA:
        gLoadData.AccessModel(MODEL_BENA, L"Data\\Npc\\", L"bena");
        gLoadData.OpenTexture(MODEL_BENA, L"Npc\\");
        break;

    }

    for (int i = 0; i < b->NumActions; i++)
        b->Actions[i].PlaySpeed = 0.25f;
    //SetTexture(BITMAP_NPC);

    if (b->NumMeshs > 0)
        gLoadData.OpenTexture(Type, L"Npc\\");

    switch (Type)
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
    case MODEL_KARUTAN_NPC_REINA:
        Models[Type].Actions[MONSTER01_STOP1].PlaySpeed = 0.5f;
        Models[Type].Actions[MONSTER01_STOP2].PlaySpeed = 0.6f;
        Models[Type].Actions[MONSTER01_WALK].PlaySpeed = 0.5f;
        break;
    case MODEL_TERSIA:
        Models[Type].Actions[MONSTER01_STOP1].PlaySpeed = 0.35f;
        Models[Type].Actions[MONSTER01_STOP2].PlaySpeed = 0.3f;
        break;
    case MODEL_KARUTAN_NPC_VOLVO:
        Models[Type].Actions[MONSTER01_STOP1].PlaySpeed = 0.2f;
        Models[Type].Actions[MONSTER01_STOP2].PlaySpeed = 0.25f;
        break;
    }

    //#endif
}

void SetMonsterSound(int Type, int s1, int s2, int s3, int s4, int s5, int s6 = -1, int s7 = -1, int s8 = -1, int s9 = -1, int s10 = -1)
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
	for (int i = MODEL_MONSTER01; i < MODEL_MONSTER_END; i++)
		if (Models[i].Version)
         Models[i].Release();

    for (int i = SOUND_MONSTER; i < SOUND_MONSTER_END; i++)
        ReleaseBuffer(i);

    for (int i = SOUND_ELBELAND_RABBITSTRANGE_ATTACK01; i <= SOUND_ELBELAND_ENTERATLANCE01; i++)
        ReleaseBuffer(i);
}

void OpenMonsterModel(EMonsterModelType Type)
{
    g_ErrorReport.Write(L"OpenMonsterModel(%d)\r\n", Type);

    int Index = MODEL_MONSTER01 + Type;

    BMD* b = &Models[Index];
    if (b->NumActions > 0 || b->NumMeshs > 0) return;

    gLoadData.AccessModel(Index, L"Data\\Monster\\", L"Monster", Type + 1);

    if (b->NumMeshs == 0) return;

    if (gMapManager.InChaosCastle() == true && Type >= 70 && Type <= 72)
    {
        gLoadData.OpenTexture(Index, L"Npc\\");
    }
    else if (gMapManager.InBattleCastle() == true && Type == 74)
    {
        gLoadData.OpenTexture(Index, L"Object31\\");
    }
    else
    {
        gLoadData.OpenTexture(Index, L"Monster\\");
    }

    switch (Type)
    {
    case MONSTER_MODEL_ICE_GIANT:
    {
        gLoadData.AccessModel(MODEL_ICE_GIANT_PART1, L"Data\\Monster\\", L"icegiantpart_1");
        gLoadData.OpenTexture(MODEL_ICE_GIANT_PART1, L"Monster\\");
        gLoadData.AccessModel(MODEL_ICE_GIANT_PART2, L"Data\\Monster\\", L"icegiantpart_2");
        gLoadData.OpenTexture(MODEL_ICE_GIANT_PART2, L"Monster\\");
        gLoadData.AccessModel(MODEL_ICE_GIANT_PART3, L"Data\\Monster\\", L"icegiantpart_3");
        gLoadData.OpenTexture(MODEL_ICE_GIANT_PART3, L"Monster\\");
        gLoadData.AccessModel(MODEL_ICE_GIANT_PART4, L"Data\\Monster\\", L"icegiantpart_4");
        gLoadData.OpenTexture(MODEL_ICE_GIANT_PART4, L"Monster\\");
        gLoadData.AccessModel(MODEL_ICE_GIANT_PART5, L"Data\\Monster\\", L"icegiantpart_5");
        gLoadData.OpenTexture(MODEL_ICE_GIANT_PART5, L"Monster\\");
        gLoadData.AccessModel(MODEL_ICE_GIANT_PART6, L"Data\\Monster\\", L"icegiantpart_6");
        gLoadData.OpenTexture(MODEL_ICE_GIANT_PART6, L"Monster\\");
    }
    break;
    }

    b->Actions[MONSTER01_STOP1].PlaySpeed = 0.25f;
    b->Actions[MONSTER01_STOP2].PlaySpeed = 0.2f;
    b->Actions[MONSTER01_WALK].PlaySpeed = 0.34f;
    b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.33f;
    b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.33f;
    b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.5f;
    b->Actions[MONSTER01_DIE].PlaySpeed = 0.55f;
    b->Actions[MONSTER01_DIE].Loop = true;

    for (int j = MONSTER01_STOP1; j < MONSTER01_DIE; j++)
    {
        if (Type == 3)
            b->Actions[j].PlaySpeed *= 1.2f;
        if (Type == 5 || Type == 25)
            b->Actions[j].PlaySpeed *= 0.7f;
        if (Type == 37 || Type == 42)
            b->Actions[j].PlaySpeed *= 0.4f;
    }

    switch (Type)
    {
    case MONSTER_MODEL_BUDGE_DRAGON:b->Actions[MONSTER01_WALK].PlaySpeed = 0.7f; break;
    case MONSTER_MODEL_LARVA:b->Actions[MONSTER01_WALK].PlaySpeed = 0.6f; break;
    case MONSTER_MODEL_HELL_SPIDER:b->Actions[MONSTER01_WALK].PlaySpeed = 0.7f; break;
    case MONSTER_MODEL_SPIDER:b->Actions[MONSTER01_WALK].PlaySpeed = 1.2f; break;
    case MONSTER_MODEL_CYCLOPS:b->Actions[MONSTER01_WALK].PlaySpeed = 0.28f; break;
    case MONSTER_MODEL_YETI:b->Actions[MONSTER01_WALK].PlaySpeed = 0.3f; break;
    case MONSTER_MODEL_ELITE_YETI:b->Actions[MONSTER01_WALK].PlaySpeed = 0.28f; break;
    case MONSTER_MODEL_WORM:b->Actions[MONSTER01_WALK].PlaySpeed = 0.5f; break;
    case MONSTER_MODEL_GOBLIN:b->Actions[MONSTER01_WALK].PlaySpeed = 0.6f; break;
    case MONSTER_MODEL_CHAIN_SCORPION:b->Actions[MONSTER01_WALK].PlaySpeed = 0.4f; break;
    case MONSTER_MODEL_BEETLE_MONSTER:b->Actions[MONSTER01_WALK].PlaySpeed = 0.5f; break;
    case MONSTER_MODEL_SHADOW:b->Actions[MONSTER01_WALK].PlaySpeed = 0.3f; break;
    case MONSTER_MODEL_TITAN:b->Actions[MONSTER01_WALK].PlaySpeed = 0.22f; break;
    case MONSTER_MODEL_GOLDEN_WHEEL:b->Actions[MONSTER01_WALK].PlaySpeed = 0.18f; break;
    case MONSTER_MODEL_TANTALLOS:
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.35f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.35f;
        break;
    case MONSTER_MODEL_BEAM_KNIGHT:
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.3f;
        break;
    case MONSTER_MODEL_DEATH_ANGEL:
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.1f;
        break;
    case MONSTER_MODEL_ILLUSION_OF_KUNDUN:
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.3f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.15f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.25f;
        break;
    case MONSTER_MODEL_AEGIS:
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.1f;
        break;
    case MONSTER_MODEL_DEATH_CENTURION:
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.2f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.3f;
        break;
    case MONSTER_MODEL_SHRIKER:
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.1f;
        break;
    case MONSTER_MODEL_CHAOSCASTLE_KNIGHT:	//
    case MONSTER_MODEL_CHAOSCASTLE_ELF:	//
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.5f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.5f;
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.3f;
        break;
    case MONSTER_MODEL_CHAOSCASTLE_WIZARD:
        //    case MONSTER_MODEL_CASTLE_GATE1:
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.4f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.4f;
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.3f;
        break;
    case MONSTER_MODEL_BALGASS:
        b->Actions[MONSTER01_APEAR].PlaySpeed = 0.33f;
        b->Actions[MONSTER01_ATTACK3].PlaySpeed = 0.33f;
        b->Actions[MONSTER01_ATTACK4].PlaySpeed = 0.33f;
        break;
    case MONSTER_MODEL_SORAM:
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.20f;
        break;
    case MONSTER_MODEL_DARK_ELF_1:
        b->Actions[MONSTER01_APEAR].PlaySpeed = 0.33f;
        b->Actions[MONSTER01_ATTACK3].PlaySpeed = 0.33f;
        break;
    case MONSTER_MODEL_BALLISTA:
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.37f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.37f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.15f;
        break;
    case MONSTER_MODEL_WITCH_QUEEN:
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.2f;
        break;
    case MONSTER_MODEL_GOLDEN_STONE_GOLEM:
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.2f;
    case MONSTER_MODEL_DEATH_RIDER:
        b->Actions[MONSTER01_STOP1].PlaySpeed = 0.15f;
        b->Actions[MONSTER01_STOP2].PlaySpeed = 0.15f;
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.23f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.23f;
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.3f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.15f;
        break;
    case MONSTER_MODEL_DEATH_TREE:
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.2f;
        break;
    case MONSTER_MODEL_HELL_MAINE:
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.15f;
        break;
    case MONSTER_MODEL_BERSERK:
        b->Actions[MONSTER01_STOP1].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_STOP2].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.23f;
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.28f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.28f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.19f;
        break;
    case MONSTER_MODEL_SPLINTER_WOLF:
        b->Actions[MONSTER01_STOP1].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_STOP2].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.27f;
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.30f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.30f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.30f;
        break;
    case MONSTER_MODEL_IRON_RIDER:
        b->Actions[MONSTER01_STOP1].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_STOP2].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.27f;
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.25f;
        break;
    case MONSTER_MODEL_SATYROS:
        b->Actions[MONSTER01_STOP1].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_STOP2].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.27f;
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.3f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.3f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.3f;
        break;
    case MONSTER_MODEL_BLADE_HUNTER:
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.23f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.23f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.2f;
        break;
    case MONSTER_MODEL_KENTAUROS:
        b->Actions[MONSTER01_STOP1].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_STOP2].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.27f;
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.27f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.27f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.25f;
        break;
    case MONSTER_MODEL_GIGANTIS:
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.26f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.26f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.21f;
        break;
    case MONSTER_MODEL_GENOCIDER:
        b->Actions[MONSTER01_STOP1].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_STOP2].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.3f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.3f;
        b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.5f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.3f;
        break;
    case MONSTER_MODEL_PERSONA:
        b->Actions[MONSTER01_STOP1].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_STOP2].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.34f;
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.33f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.33f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.23f;
        break;
    case MONSTER_MODEL_TWIN_TAIL:
        b->Actions[MONSTER01_STOP1].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_STOP2].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.34f;
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.23f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.23f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.23f;
        break;
    case MONSTER_MODEL_DREADFEAR:
        b->Actions[MONSTER01_STOP1].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_STOP2].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.34f;
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.25f;
        break;
    case MONSTER_MODEL_DARK_SKULL_SOLDIER_5:
        b->Actions[MONSTER01_STOP1].PlaySpeed = 0.22f;
        b->Actions[MONSTER01_STOP2].PlaySpeed = 0.22f;
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.12f;
        b->Actions[MONSTER01_APEAR].PlaySpeed = 0.22f;
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_ATTACK3].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_ATTACK4].PlaySpeed = 0.25f;
        break;
    case MONSTER_MODEL_MAYA_HAND_LEFT:
        b->Actions[MONSTER01_APEAR].PlaySpeed = 0.12f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.12f;
    case MONSTER_MODEL_MAYA_HAND_RIGHT:
        b->Actions[MONSTER01_APEAR].PlaySpeed = 0.12f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.12f;
    case MONSTER_MODEL_MAYA:
        break;
    case MONSTER_MODEL_POUCH_OF_BLESSING:
        b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.3f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.22f;
        break;
    case MONSTER_MODEL_LUNAR_RABBIT:
        b->Actions[MONSTER01_STOP1].PlaySpeed = 0.40f;
        b->Actions[MONSTER01_STOP2].PlaySpeed = 0.40f;
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.40f;
        b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.40f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.50f;
        break;
    case MONSTER_MODEL_FIRE_FLAME_GHOST:
        b->Actions[MONSTER01_STOP1].PlaySpeed = 0.30f;
        b->Actions[MONSTER01_STOP2].PlaySpeed = 0.30f;
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.60f;
        b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.50f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.50f;
        break;
    case MONSTER_MODEL_ZOMBIE_FIGHTER:
    {
        b->Actions[MONSTER01_STOP1].PlaySpeed = 0.28f;
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.3f;
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.17f;
        b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.2f;
    }
    break;
    case MONSTER_MODEL_GLADIATOR:
    {
        b->Actions[MONSTER01_STOP1].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.3f;
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.28f;
        b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.2f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.18f;
    }
    break;
    case MONSTER_MODEL_SLAUGTHERER:
    {
        b->Actions[MONSTER01_STOP1].PlaySpeed = 0.2f;
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.4f;
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.17f;
        b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.2f;
    }
    break;
    case MONSTER_MODEL_BLOOD_ASSASSIN:
    {
        b->Actions[MONSTER01_STOP1].PlaySpeed = 0.28f;
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.5f;
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.17f;
        b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.4f;
    }
    break;
    case MONSTER_MODEL_CRUEL_BLOOD_ASSASSIN:
    {
        b->Actions[MONSTER01_STOP1].PlaySpeed = 0.28f;
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.5f;
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.17f;
        b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.4f;
    }
    break;
    case MONSTER_MODEL_LAVA_GIANT:
    {
        b->Actions[MONSTER01_STOP1].PlaySpeed = 0.28f;
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.3f;
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.25f;
    }
    break;
    case MONSTER_MODEL_BURNING_LAVA_GIANT:
    {
        b->Actions[MONSTER01_STOP1].PlaySpeed = 0.28f;
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.3f;
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.25f;
    }
    break;
    case MONSTER_MODEL_RABBIT:
        b->Actions[MONSTER01_STOP1].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_STOP2].PlaySpeed = 0.28f;
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.25f;
        break;
    case MONSTER_MODEL_BUTTERFLY:
        b->Actions[MONSTER01_STOP1].PlaySpeed = 0.20f;
        b->Actions[MONSTER01_STOP2].PlaySpeed = 0.20f;
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.30f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.30f;
        b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.23f;
        break;
    case MONSTER_MODEL_HIDEOUS_RABBIT:
        b->Actions[MONSTER01_STOP1].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_STOP2].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.50f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.50f;
        b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.30f;
        break;
    case MONSTER_MODEL_WEREWOLF2:
        b->Actions[MONSTER01_STOP1].PlaySpeed = 0.45f;
        b->Actions[MONSTER01_STOP2].PlaySpeed = 0.45f;
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.45f;
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.45f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.45f;
        b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.45f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.45f;
        break;
    case MONSTER_MODEL_CURSED_LICH:
        b->Actions[MONSTER01_STOP1].PlaySpeed = 0.20f;
        b->Actions[MONSTER01_STOP2].PlaySpeed = 0.20f;
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.35;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.35f;
        b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.30f;
        break;
    case MONSTER_MODEL_TOTEM_GOLEM:
        b->Actions[MONSTER01_STOP1].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_STOP2].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.20f;
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.35;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.35f;
        b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.20f;
        break;
    case MONSTER_MODEL_GRIZZLY:
        b->Actions[MONSTER01_STOP1].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_STOP2].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.33f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.33f;
        b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.22f;
        break;
    case MONSTER_MODEL_CAPTAIN_GRIZZLY:
        b->Actions[MONSTER01_STOP1].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_STOP2].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.30f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.30f;
        b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.28f;
        break;
    case MONSTER_MODEL_SAPIUNUS:
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.25f;
        break;
    case MONSTER_MODEL_SAPIDUO:
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.25f;
        break;
    case MONSTER_MODEL_SAPITRES:
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.25f;
        break;
    case MONSTER_MODEL_SHADOW_PAWN:
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.4f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.4f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.25f;
        break;
    case MONSTER_MODEL_SHADOW_KNIGHT:
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.45f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.45f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.25f;
        break;
    case MONSTER_MODEL_SHADOW_LOOK:
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.25f;
        break;
    case MONSTER_MODEL_NAPIN:
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.20f;
        break;
    case MONSTER_MODEL_GHOST_NAPIN:
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.20f;
        break;
    case MONSTER_MODEL_BLAZE_NAPIN:
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.20f;
        break;
    case MONSTER_MODEL_ICE_WALKER:
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.6f;
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.4f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.35f;
        break;
    case MONSTER_MODEL_GIANT_MAMMOTH:
        break;
    case MONSTER_MODEL_ICE_GIANT:
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.13f;
        break;
    case MONSTER_MODEL_COOLUTIN:
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.46f;
        break;
    case MONSTER_MODEL_IRON_KNIGHT:
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.21f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.23f;
        break;
    case MONSTER_MODEL_SELUPAN:
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.20f;
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.35f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.18f;
        b->Actions[MONSTER01_APEAR].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_ATTACK3].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_ATTACK4].PlaySpeed = 0.25f;
        break;
    case MONSTER_MODEL_SPIDER_EGGS_1:
    case MONSTER_MODEL_SPIDER_EGGS_2:
    case MONSTER_MODEL_SPIDER_EGGS_3:
        break;
    case MONSTER_MODEL_CURSED_SANTA:
        b->Actions[MONSTER01_STOP2].PlaySpeed = 0.3f;
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.29f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.4f;
        break;
    case MONSTER_MODEL_GAYION:
        b->Actions[MONSTER01_STOP1].PlaySpeed = 0.30f;
        b->Actions[MONSTER01_STOP2].PlaySpeed = 0.30f;
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.40f;
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.38f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.40f;
        b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.50f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.20f;
        b->Actions[MONSTER01_APEAR].PlaySpeed = 0.40f;
        b->Actions[MONSTER01_ATTACK3].PlaySpeed = 0.38f;
        b->Actions[MONSTER01_ATTACK4].PlaySpeed = 0.38f;
        break;
    case MONSTER_MODEL_JERRY:
        b->Actions[MONSTER01_STOP1].PlaySpeed = 0.30f;
        b->Actions[MONSTER01_STOP2].PlaySpeed = 0.30f;
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.50f;
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.86f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.86f;
        b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.50f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.40f;
        b->Actions[MONSTER01_APEAR].PlaySpeed = 0.40f;
        b->Actions[MONSTER01_ATTACK3].PlaySpeed = 0.76f;
        break;
    case MONSTER_MODEL_RAYMOND:
        b->Actions[MONSTER01_STOP1].PlaySpeed = 0.60f;
        b->Actions[MONSTER01_STOP2].PlaySpeed = 0.60f;
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.55f;
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.75f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.50f;
        b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.50f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.30f;
        b->Actions[MONSTER01_ATTACK3].PlaySpeed = 0.50f;
        b->Actions[MONSTER01_APEAR].PlaySpeed = 0.80f;
        break;
    case MONSTER_MODEL_LUCAS:
        b->Actions[MONSTER01_STOP1].PlaySpeed = 0.60f;
        b->Actions[MONSTER01_STOP2].PlaySpeed = 0.60f;
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.50f;
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.71f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.40f;
        b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.50f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.30f;
        b->Actions[MONSTER01_APEAR].PlaySpeed = 0.80f;
        b->Actions[MONSTER01_ATTACK3].PlaySpeed = 0.40f;
        break;
    case MONSTER_MODEL_FRED:
        b->Actions[MONSTER01_STOP1].PlaySpeed = 0.30f;
        b->Actions[MONSTER01_STOP2].PlaySpeed = 0.30f;
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.40f;
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.33f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.38f;
        b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.50f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.30f;
        b->Actions[MONSTER01_ATTACK3].PlaySpeed = 0.40f;
        b->Actions[MONSTER01_ATTACK4].PlaySpeed = 0.45f;
        break;
    case MONSTER_MODEL_HAMMERIZE:
        b->Actions[MONSTER01_STOP1].PlaySpeed = 0.30f;
        b->Actions[MONSTER01_STOP2].PlaySpeed = 0.30f;
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.35f;
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.40f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.45f;
        b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.50f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.40f;
        b->Actions[MONSTER01_APEAR].PlaySpeed = 0.45f;
        b->Actions[MONSTER01_ATTACK3].PlaySpeed = 0.45f;
        break;
    case MONSTER_MODEL_DUAL_BERSERKER:
        b->Actions[MONSTER01_STOP1].PlaySpeed = 0.30f;
        b->Actions[MONSTER01_STOP2].PlaySpeed = 0.30f;
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.35f;
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.40f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.35f;
        b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.50f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.35f;
        b->Actions[MONSTER01_ATTACK3].PlaySpeed = 0.40f;
        break;
    case MONSTER_MODEL_DEVIL_LORD:
        b->Actions[MONSTER01_STOP1].PlaySpeed = 0.30f;
        b->Actions[MONSTER01_STOP2].PlaySpeed = 0.30f;
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.40f;
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.45f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.50f;
        b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.50f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.30f;
        b->Actions[MONSTER01_ATTACK3].PlaySpeed = 0.45f;
        break;
    case MONSTER_MODEL_QUARTER_MASTER:
        b->Actions[MONSTER01_STOP1].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_STOP2].PlaySpeed = 0.20f;
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.50f;
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.66f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.33f;
        b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.50f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.55f;
        break;
    case MONSTER_MODEL_COMBAT_INSTRUCTOR:
        b->Actions[MONSTER01_STOP1].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_STOP2].PlaySpeed = 0.20f;
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.40f;
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.33f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.36f;
        b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.50f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.30f;
        b->Actions[MONSTER01_APEAR].PlaySpeed = 0.40f;
        break;
    case MONSTER_MODEL_ATICLES_HEAD:
        b->Actions[MONSTER01_STOP1].PlaySpeed = 0.30f;
        b->Actions[MONSTER01_STOP2].PlaySpeed = 0.30f;
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.65f;
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.86f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.86f;
        b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.50f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.30f;
        b->Actions[MONSTER01_APEAR].PlaySpeed = 0.86f;
        break;
    case MONSTER_MODEL_DARK_GHOST:
        b->Actions[MONSTER01_STOP1].PlaySpeed = 0.60f;
        b->Actions[MONSTER01_STOP2].PlaySpeed = 0.60f;
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.80f;
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.96f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.96f;
        b->Actions[MONSTER01_SHOCK].PlaySpeed = 1.00f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.30f;
        break;
    case MONSTER_MODEL_BANSHEE:
        b->Actions[MONSTER01_STOP1].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_STOP2].PlaySpeed = 0.20f;
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.20f;
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.40f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.38f;
        b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.50f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.40f;
        b->Actions[MONSTER01_ATTACK3].PlaySpeed = 0.38f;
        break;
    case MONSTER_MODEL_HEAD_MOUNTER:
        b->Actions[MONSTER01_STOP1].PlaySpeed = 0.30f;
        b->Actions[MONSTER01_STOP2].PlaySpeed = 0.30f;
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.37f;
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.33f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.33f;
        b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.50f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.30f;
        break;
    case MONSTER_MODEL_DEFENDER:
        b->Actions[MONSTER01_STOP1].PlaySpeed = 0.40f;
        b->Actions[MONSTER01_STOP2].PlaySpeed = 0.40f;
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.40f;
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.40f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.40f;
        b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.40f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.45f;
        b->Actions[MONSTER01_APEAR].PlaySpeed = 0.40f;
        break;
#ifdef LDS_ADD_EG_2_MONSTER_GUARDIANPRIEST
    case MONSTER_MODEL_FORSAKER:
        b->Actions[MONSTER01_STOP1].PlaySpeed = 0.80f;
        b->Actions[MONSTER01_STOP2].PlaySpeed = 0.80f;
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.80f;
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.80f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.80f;
        b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.40f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.40f;
        break;
#endif // LDS_ADD_EG_2_MONSTER_GUARDIANPRIEST
    case MONSTER_MODEL_OCELOT:
        b->Actions[MONSTER01_STOP1].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_STOP2].PlaySpeed = 0.20f;
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.55f;
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.66f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.66f;
        b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.50f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.55f;
        break;
    case MONSTER_MODEL_ERIC:
        b->Actions[MONSTER01_STOP1].PlaySpeed = 0.30f;
        b->Actions[MONSTER01_STOP2].PlaySpeed = 0.30f;
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.35f;
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.33f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.33f;
        b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.50f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.30f;
        break;
    case MONSTER_MODEL_MAD_BUTCHER:
    case MONSTER_MODEL_TERRIBLE_BUTCHER:
        b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.3f;
        break;
    case MONSTER_MODEL_DOPPELGANGER:
        b->Actions[MONSTER01_STOP1].PlaySpeed = 0.25f * 2.0f;
        b->Actions[MONSTER01_STOP2].PlaySpeed = 0.2f * 2.0f;
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.34f * 2.0f;
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.25f * 2.0f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.25f * 2.0f;
        b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.5f * 2.0f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.55f * 2.0f;
        b->Actions[MONSTER01_APEAR].PlaySpeed = 0.33f * 3.0f;
        break;
    case MONSTER_MODEL_MEDUSA:
        b->Actions[MONSTER01_STOP1].PlaySpeed = 0.30f;
        b->Actions[MONSTER01_STOP2].PlaySpeed = 0.30;
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.30;
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.30;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.30;
        b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.30;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.30;
        b->Actions[MONSTER01_APEAR].PlaySpeed = 0.30;
        b->Actions[MONSTER01_ATTACK3].PlaySpeed = 0.30;
        break;
    case MONSTER_MODEL_DARK_MAMMOTH:
        break;
    case MONSTER_MODEL_DARK_GIANT:
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.13f;
        break;
    case MONSTER_MODEL_DARK_COOLUTIN:
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.46f;
        break;
    case MONSTER_MODEL_DARK_IRON_KNIGHT:
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.21f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.23f;
        break;
    case MONSTER_MODEL_BLOODY_ORC:
        break;
    case MONSTER_MODEL_BLOODY_DEATH_RIDER:
        b->Actions[MONSTER01_STOP1].PlaySpeed = 0.15f;
        b->Actions[MONSTER01_STOP2].PlaySpeed = 0.15f;
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.23f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.23f;
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.3f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.15f;
        break;
    case MONSTER_MODEL_BLOODY_GOLEM:
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.2f;
        break;
    case MONSTER_MODEL_BLOODY_WITCH_QUEEN:
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.2f;
        break;
    case MONSTER_MODEL_SAPI_QUEEN:
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.25f;
        break;
    case MONSTER_MODEL_ICE_NAPIN:
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.20f;
        break;
    case MONSTER_MODEL_SHADOW_MASTER:
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.25f;
        break;
    case MONSTER_MODEL_BERSERKER_WARRIOR:
        b->Actions[MONSTER01_STOP1].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_STOP2].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.23f;
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.28f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.28f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.19f;
        break;
    case MONSTER_MODEL_KENTAUROS_WARRIOR:
        b->Actions[MONSTER01_STOP1].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_STOP2].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.27f;
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.27f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.27f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.25f;
        break;
    case MONSTER_MODEL_GIGANTIS_WARRIOR:
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.26f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.26f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.21f;
        break;
    case MONSTER_MODEL_SOCCERBALL:
        b->Actions[MONSTER01_STOP1].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_STOP2].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.3f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.3f;
        b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.5f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.3f;
        break;
#ifdef ASG_ADD_KARUTAN_MONSTERS
    case MONSTER_MODEL_VENOMOUS_CHAIN_SCORPION:
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.95f;
        break;
    case MONSTER_MODEL_BONE_SCORPION:
        b->Actions[MONSTER01_WALK].PlaySpeed = 1.00f;
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.40f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.40f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.20f;
        break;
    case MONSTER_MODEL_ORCUS:
        b->Actions[MONSTER01_STOP1].PlaySpeed = 0.7f;
        b->Actions[MONSTER01_STOP2].PlaySpeed = 0.7f;
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.6f;
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.8f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.8f;
        b->Actions[MONSTER01_SHOCK].PlaySpeed = 0.25f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.3f;
        break;
    case MONSTER_MODEL_GOLLOCK:
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.4f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.3f;
        break;
    case MONSTER_MODEL_CRYPTA:
    case MONSTER_MODEL_CRYPOS:
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.9f;
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.37f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.37f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.25f;
        break;
    case MONSTER_MODEL_CONDRA:
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.80f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.80f;
        break;
    case MONSTER_MODEL_NACONDRA:
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.75f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.75f;
        break;
#endif	// ASG_ADD_KARUTAN_MONSTERS
    }
    switch (Type)
    {
    case MONSTER_MODEL_ASSASSIN:
    case MONSTER_MODEL_DRAGON:
    case MONSTER_MODEL_TITAN:
    case MONSTER_MODEL_SOLDIER:
        b->Actions[MONSTER01_STOP2].Loop = true;
        break;
    }

    int Channel = 2;
    bool Enable = true;
    switch (Type)
    {
    case MONSTER_MODEL_CHAOSCASTLE_KNIGHT:
    case MONSTER_MODEL_CHAOSCASTLE_ELF:
        LoadWaveFile(SOUND_MONSTER_ORCCAPATTACK1, L"Data\\Sound\\mOrcCapAttack1.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, -1, -1, 161, 161, -1);
        Models[MODEL_MONSTER01 + Type].BoneHead = 20;
        break;

    case MONSTER_MODEL_CHAOSCASTLE_WIZARD:
        LoadWaveFile(SOUND_MONSTER_ORCARCHERATTACK1, L"Data\\Sound\\mOrcArcherAttack1.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, -1, -1, 162, 162, -1);
        Models[MODEL_MONSTER01 + Type].BoneHead = 20;
        break;
    case MONSTER_MODEL_BULL_FIGHTER:
    case MONSTER_MODEL_DEATH_COW:
        LoadWaveFile(SOUND_MONSTER_BULL1, L"Data\\Sound\\mBull1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_BULL2, L"Data\\Sound\\mBull2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_BULLATTACK1, L"Data\\Sound\\mBullAttack1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_BULLATTACK2, L"Data\\Sound\\mBullAttack2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_BULLDIE, L"Data\\Sound\\mBullDie.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, 0, 1, 2, 3, 4);
        Models[MODEL_MONSTER01 + Type].BoneHead = 20;
        break;
    case MONSTER_MODEL_HOUND:
        LoadWaveFile(SOUND_MONSTER_HOUND1, L"Data\\Sound\\mHound1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_HOUND2, L"Data\\Sound\\mHound2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_HOUNDATTACK1, L"Data\\Sound\\mHoundAttack1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_HOUNDATTACK2, L"Data\\Sound\\mHoundAttack2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_HOUNDDIE, L"Data\\Sound\\mHoundDie.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, 5, 6, 7, 8, 9);
        Models[MODEL_MONSTER01 + Type].BoneHead = 5;
        break;
    case MONSTER_MODEL_BUDGE_DRAGON:
        LoadWaveFile(SOUND_MONSTER_BUDGE1, L"Data\\Sound\\mBudge1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_BUDGEATTACK1, L"Data\\Sound\\mBudgeAttack1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_BUDGEDIE, L"Data\\Sound\\mBudgeDie.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, 10, 11, 11, 11, 12);
        Models[MODEL_MONSTER01 + Type].BoneHead = 7;
        break;
    case MONSTER_MODEL_SPIDER:
        LoadWaveFile(SOUND_MONSTER_SPIDER1, L"Data\\Sound\\mSpider1.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, 13, 13, 13, 13, 13);
        break;
    case MONSTER_MODEL_DARK_KNIGHT:
    case MONSTER_MODEL_DEATH_KNIGHT:
    case MONSTER_MODEL_TITAN:
        LoadWaveFile(SOUND_MONSTER_DARKKNIGHT1, L"Data\\Sound\\mDarkKnight1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_DARKKNIGHT2, L"Data\\Sound\\mDarkKnight2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_DARKKNIGHTATTACK1, L"Data\\Sound\\mDarkKnightAttack1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_DARKKNIGHTATTACK2, L"Data\\Sound\\mDarkKnightAttack2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_DARKKNIGHTDIE, L"Data\\Sound\\mDarkKnightDie.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, 15, 16, 17, 18, 19);
        if (Type == 3)
            Models[MODEL_MONSTER01 + Type].BoneHead = 16;
        else if (Type == 29)
            Models[MODEL_MONSTER01 + Type].BoneHead = 20;
        else
            Models[MODEL_MONSTER01 + Type].BoneHead = 19;
        break;
    case MONSTER_MODEL_LICH:
        LoadWaveFile(SOUND_MONSTER_WIZARD1, L"Data\\Sound\\mWizard1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_WIZARD2, L"Data\\Sound\\mWizard2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_WIZARDATTACK1, L"Data\\Sound\\mWizardAttack1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_WIZARDATTACK2, L"Data\\Sound\\mWizardAttack2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_WIZARDDIE, L"Data\\Sound\\mWizardDie.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, 20, 21, 22, 23, 24);
        Models[MODEL_MONSTER01 + Type].BoneHead = 20;
        break;
    case MONSTER_MODEL_GIANT:
        LoadWaveFile(SOUND_MONSTER_GIANT1, L"Data\\Sound\\mGiant1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_GIANT2, L"Data\\Sound\\mGiant2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_GIANTATTACK1, L"Data\\Sound\\mGiantAttack1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_GIANTATTACK2, L"Data\\Sound\\mGiantAttack2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_GIANTDIE, L"Data\\Sound\\mGiantDie.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, 25, 26, 27, 28, 29);
        Models[MODEL_MONSTER01 + Type].BoneHead = 20;
        break;
    case MONSTER_MODEL_LARVA:
        LoadWaveFile(SOUND_MONSTER_LARVA1, L"Data\\Sound\\mLarva1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_LARVA2, L"Data\\Sound\\mLarva2.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, 30, 31, 30, 31, 31);
        break;
    case MONSTER_MODEL_HELL_SPIDER:
        LoadWaveFile(SOUND_MONSTER_HELLSPIDER1, L"Data\\Sound\\mHellSpider1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_HELLSPIDERATTACK1, L"Data\\Sound\\mHellSpiderAttack1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_HELLSPIDERDIE, L"Data\\Sound\\mHellSpiderDie.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, 32, 33, 33, 33, 34);
        Models[MODEL_MONSTER01 + Type].BoneHead = 16;
        break;
    case MONSTER_MODEL_GHOST:
        LoadWaveFile(SOUND_MONSTER_GHOST1, L"Data\\Sound\\mGhost1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_GHOST2, L"Data\\Sound\\mGhost2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_GHOSTATTACK1, L"Data\\Sound\\mGhostAttack1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_GHOSTATTACK2, L"Data\\Sound\\mGhostAttack2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_GHOSTDIE, L"Data\\Sound\\mGhostDie.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, 35, 36, 37, 38, 39);
        Models[MODEL_MONSTER01 + Type].BoneHead = 20;
        break;
    case MONSTER_MODEL_CYCLOPS:
        LoadWaveFile(SOUND_MONSTER_OGRE1, L"Data\\Sound\\mOgre1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_OGRE2, L"Data\\Sound\\mOgre2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_OGREATTACK1, L"Data\\Sound\\mOgreAttack1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_OGREATTACK2, L"Data\\Sound\\mOgreAttack2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_OGREDIE, L"Data\\Sound\\mOgreDie.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, 40, 41, 42, 43, 44);
        Models[MODEL_MONSTER01 + Type].BoneHead = 20;
        break;
    case MONSTER_MODEL_GORGON:
        LoadWaveFile(SOUND_MONSTER_GORGON1, L"Data\\Sound\\mGorgon1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_GORGON2, L"Data\\Sound\\mGorgon2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_GORGONATTACK1, L"Data\\Sound\\mGorgonAttack1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_GORGONATTACK2, L"Data\\Sound\\mGorgonAttack2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_GORGONDIE, L"Data\\Sound\\mGorgonDie.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, 45, 46, 47, 48, 49);
        Models[MODEL_MONSTER01 + Type].BoneHead = 20;
        break;
    case MONSTER_MODEL_ICE_MONSTER:
        LoadWaveFile(SOUND_MONSTER_ICEMONSTER1, L"Data\\Sound\\mIceMonster1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_ICEMONSTER2, L"Data\\Sound\\mIceMonster2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_ICEMONSTERDIE, L"Data\\Sound\\mIceMonsterDie.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, 50, 51, 50, 50, 52);
        Models[MODEL_MONSTER01 + Type].BoneHead = 19;
        break;
    case MONSTER_MODEL_WORM:
        LoadWaveFile(SOUND_MONSTER_WORM1, L"Data\\Sound\\mWorm1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_WORM2, L"Data\\Sound\\mWorm2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_WORMDIE, L"Data\\Sound\\mWormDie.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, 53, 53, 55, 55, 55);
        break;
    case MONSTER_MODEL_HOMMERD:
        LoadWaveFile(SOUND_MONSTER_HOMORD1, L"Data\\Sound\\mHomord1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_HOMORD2, L"Data\\Sound\\mHomord2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_HOMORDATTACK1, L"Data\\Sound\\mHomordAttack1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_HOMORDDIE, L"Data\\Sound\\mHomordDie.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, 56, 57, 58, 58, 59);
        break;
    case MONSTER_MODEL_ICE_QUEEN:
        LoadWaveFile(SOUND_MONSTER_ICEQUEEN1, L"Data\\Sound\\mIceQueen1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_ICEQUEEN2, L"Data\\Sound\\mIceQueen2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_ICEQUEENATTACK1, L"Data\\Sound\\mIceQueenAttack1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_ICEQUEENATTACK2, L"Data\\Sound\\mIceQueenAttack2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_ICEQUEENDIE, L"Data\\Sound\\mIceQueenDie.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, 60, 61, 62, 63, 64);
        Models[MODEL_MONSTER01 + Type].BoneHead = 16;
        break;
    case MONSTER_MODEL_ASSASSIN:
        LoadWaveFile(SOUND_MONSTER_ASSASSINATTACK1, L"Data\\Sound\\mAssassinAttack1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_ASSASSINATTACK2, L"Data\\Sound\\mAssassinAttack2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_ASSASSINDIE, L"Data\\Sound\\mAssassinDie.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, -1, -1, 65, 66, 67);
        Models[MODEL_MONSTER01 + Type].BoneHead = 20;
        b->Actions[MONSTER01_STOP2].PlaySpeed = 0.35f;
        break;
    case MONSTER_MODEL_YETI:
    case MONSTER_MODEL_ELITE_YETI:
        LoadWaveFile(SOUND_MONSTER_YETI1, L"Data\\Sound\\mYeti1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_YETI2, L"Data\\Sound\\mYeti2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_YETIATTACK1, L"Data\\Sound\\mYetiAttack1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_YETIDIE, L"Data\\Sound\\mYetiDie.wav", Channel, Enable);
        SetMonsterSound(MODEL_YETI, 68, 68, 70, 70, 71);
        SetMonsterSound(MODEL_ELITE_YETI, 68, 69, 70, 70, 71);
        Models[MODEL_YETI].BoneHead = 20;
        Models[MODEL_ELITE_YETI].BoneHead = 20;
        break;
    case MONSTER_MODEL_GOBLIN:
        LoadWaveFile(SOUND_MONSTER_GOBLIN1, L"Data\\Sound\\mGoblin1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_GOBLIN2, L"Data\\Sound\\mGoblin2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_GOBLINATTACK1, L"Data\\Sound\\mGoblinAttack1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_GOBLINATTACK2, L"Data\\Sound\\mGoblinAttack2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_GOBLINDIE, L"Data\\Sound\\mGoblinDie.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, 72, 73, 74, 75, 76);
        Models[MODEL_MONSTER01 + Type].BoneHead = 6;
        break;
    case MONSTER_MODEL_CHAIN_SCORPION:
        LoadWaveFile(SOUND_MONSTER_SCORPION1, L"Data\\Sound\\mScorpion1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_SCORPION2, L"Data\\Sound\\mScorpion2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_SCORPIONATTACK1, L"Data\\Sound\\mScorpionAttack1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_SCORPIONATTACK2, L"Data\\Sound\\mScorpionAttack2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_SCORPIONDIE, L"Data\\Sound\\mScorpionDie.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, 77, 78, 79, 80, 81);
        break;
    case MONSTER_MODEL_BEETLE_MONSTER:
        LoadWaveFile(SOUND_MONSTER_BEETLE1, L"Data\\Sound\\mBeetle1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_BEETLEATTACK1, L"Data\\Sound\\mBeetleAttack1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_BEETLEDIE, L"Data\\Sound\\mBeetleDie.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, 82, 82, 83, 83, 84);
        Models[MODEL_MONSTER01 + Type].BoneHead = 5;
        break;
    case MONSTER_MODEL_HUNTER:
        LoadWaveFile(SOUND_MONSTER_HUNTER1, L"Data\\Sound\\mHunter1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_HUNTER2, L"Data\\Sound\\mHunter2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_HUNTERATTACK1, L"Data\\Sound\\mHunterAttack1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_HUNTERATTACK2, L"Data\\Sound\\mHunterAttack2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_HUNTERDIE, L"Data\\Sound\\mHunterDie.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, 85, 86, 87, 88, 89);
        Models[MODEL_MONSTER01 + Type].BoneHead = 6;
        break;
    case MONSTER_MODEL_FOREST_MONSTER:
        LoadWaveFile(SOUND_MONSTER_WOODMON1, L"Data\\Sound\\mWoodMon1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_WOODMON2, L"Data\\Sound\\mWoodMon2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_WOODMONATTACK1, L"Data\\Sound\\mWoodMonAttack1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_WOODMONATTACK2, L"Data\\Sound\\mWoodMonAttack2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_WOODMONDIE, L"Data\\Sound\\mWoodMonDie.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, 90, 91, 92, 93, 94);
        Models[MODEL_MONSTER01 + Type].BoneHead = 6;
        break;
    case MONSTER_MODEL_AGON:
        LoadWaveFile(SOUND_MONSTER_ARGON1, L"Data\\Sound\\mArgon1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_ARGON2, L"Data\\Sound\\mArgon2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_ARGONATTACK1, L"Data\\Sound\\mArgonAttack1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_ARGONATTACK2, L"Data\\Sound\\mArgonAttack2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_ARGONDIE, L"Data\\Sound\\mArgonDie.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, 95, 96, 97, 98, 99);
        Models[MODEL_MONSTER01 + Type].BoneHead = 16;
        break;
    case MONSTER_MODEL_STONE_GOLEM:
        LoadWaveFile(SOUND_MONSTER_GOLEM1, L"Data\\Sound\\mGolem1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_GOLEM2, L"Data\\Sound\\mGolem2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_GOLEMATTACK1, L"Data\\Sound\\mGolemAttack1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_GOLEMATTACK2, L"Data\\Sound\\mGolemAttack2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_GOLEMDIE, L"Data\\Sound\\mGolemDie.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, 100, 101, 102, 103, 104);
        Models[MODEL_MONSTER01 + Type].BoneHead = 5;
        break;
    case MONSTER_MODEL_DEVIL:
        LoadWaveFile(SOUND_MONSTER_YETI1, L"Data\\Sound\\mYeti1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_SATANATTACK1, L"Data\\Sound\\mSatanAttack1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_YETIDIE, L"Data\\Sound\\mYetiDie.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, 105, 105, 106, 106, 107);
        Models[MODEL_MONSTER01 + Type].BoneHead = 6;
        break;
    case MONSTER_MODEL_BALROG:
        LoadWaveFile(SOUND_MONSTER_BALROG1, L"Data\\Sound\\mBalrog1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_BALROG2, L"Data\\Sound\\mBalrog2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_WIZARDATTACK2, L"Data\\Sound\\mWizardAttack2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_GORGONATTACK2, L"Data\\Sound\\mGorgonAttack2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_BALROGDIE, L"Data\\Sound\\mBalrogDie.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, 108, 109, 110, 111, 112);
        Models[MODEL_MONSTER01 + Type].BoneHead = 6;
        Models[MODEL_MONSTER01 + Type].StreamMesh = 1;
        break;
    case MONSTER_MODEL_SHADOW:
        LoadWaveFile(SOUND_MONSTER_SHADOW1, L"Data\\Sound\\mShadow1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_SHADOW2, L"Data\\Sound\\mShadow2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_SHADOWATTACK1, L"Data\\Sound\\mShadowAttack1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_SHADOWATTACK2, L"Data\\Sound\\mShadowAttack2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_SHADOWDIE, L"Data\\Sound\\mShadowDie.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, 113, 114, 115, 116, 117);
        Models[MODEL_MONSTER01 + Type].BoneHead = 5;
        break;
    case MONSTER_MODEL_DRAGON:
        LoadWaveFile(SOUND_MONSTER_YETI1, L"Data\\Sound\\mYeti1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_BULLATTACK1, L"Data\\Sound\\mBullAttack1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_YETIDIE, L"Data\\Sound\\mYetiDie.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, 123, 123, 124, 124, 125);
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.5f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.7f;
        b->Actions[MONSTER01_STOP2].PlaySpeed = 0.8f;
        b->Actions[MONSTER01_DIE + 1].PlaySpeed = 0.8f;
        break;
    case MONSTER_MODEL_BALI:
        LoadWaveFile(SOUND_MONSTER_BALI1, L"Data\\Sound\\mBali1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_BALI2, L"Data\\Sound\\mBali2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_BALIATTACK1, L"Data\\Sound\\mBaliAttack1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_BALIATTACK2, L"Data\\Sound\\mBaliAttack2.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, 126, 127, 128, 129, 127);
        b->Actions[MONSTER01_ATTACK3].PlaySpeed = 0.4f;
        b->Actions[MONSTER01_ATTACK4].PlaySpeed = 0.4f;
        b->Actions[MONSTER01_APEAR].PlaySpeed = 0.4f;
        b->Actions[MONSTER01_RUN].PlaySpeed = 0.4f;
        b->BoneHead = 6;
        break;

    case MONSTER_MODEL_BAHAMUT:
        LoadWaveFile(SOUND_MONSTER_BAHAMUT1, L"Data\\Sound\\mBahamut1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_YETI1, L"Data\\Sound\\mYeti1.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, 130, 130, 131, 131, 130);
        break;
    case MONSTER_MODEL_VEPAR:
        LoadWaveFile(SOUND_MONSTER_BEPAR1, L"Data\\Sound\\mBepar1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_BEPAR2, L"Data\\Sound\\mBepar2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_BALROG1, L"Data\\Sound\\mBalrog1.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, 132, 133, 104, 104, 133);
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.5f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.5f;
        b->BoneHead = 20;//인어
        break;
    case MONSTER_MODEL_VALKYRIE:
        LoadWaveFile(SOUND_MONSTER_VALKYRIE1, L"Data\\Sound\\mValkyrie1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_BALIATTACK2, L"Data\\Sound\\mBaliAttack2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_VALKYRIEDIE, L"Data\\Sound\\mValkyrieDie.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, 135, 135, 136, 136, 137);
        Models[MODEL_MONSTER01 + Type].BoneHead = 19;
        break;
    case MONSTER_MODEL_LIZARD:
    case MONSTER_MODEL_SOLDIER:
        LoadWaveFile(SOUND_MONSTER_LIZARDKING1, L"Data\\Sound\\mLizardKing1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_LIZARDKING2, L"Data\\Sound\\mLizardKing2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_GORGONDIE, L"Data\\Sound\\mGorgonDie.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, 138, 139, 138, 139, 140);
        if (Type == 36)
            Models[MODEL_MONSTER01 + Type].BoneHead = 19;
        else
            Models[MODEL_MONSTER01 + Type].BoneHead = 6;
        break;
    case MONSTER_MODEL_HYDRA:
        LoadWaveFile(SOUND_MONSTER_HYDRA1, L"Data\\Sound\\mHydra1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_HYDRAATTACK1, L"Data\\Sound\\mHydraAttack1.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, 141, 141, 142, 142, 141);
        b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.15f;
        b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.15f;
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.2f;
        break;
    case MONSTER_MODEL_GOLDEN_WHEEL:
        LoadWaveFile(SOUND_MONSTER_IRON1, L"Data\\Sound\\iron1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_IRON_ATTACK1, L"Data\\Sound\\iron_attack1.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, 143, 143, 144, 144, 144);
        Models[MODEL_MONSTER01 + Type].BoneHead = 3;
        break;
    case MONSTER_MODEL_TANTALLOS:
        LoadWaveFile(SOUND_MONSTER_JAIKAN1, L"Data\\Sound\\jaikan1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_JAIKAN2, L"Data\\Sound\\jaikan2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_JAIKAN_ATTACK1, L"Data\\Sound\\jaikan_attack1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_JAIKAN_ATTACK2, L"Data\\Sound\\jaikan_attack2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_JAIKAN_DIE, L"Data\\Sound\\jaikan_die.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, 145, 146, 147, 148, 149);
        LoadBitmap(L"Monster\\bv01_2.jpg", BITMAP_MONSTER_SKIN, GL_LINEAR, GL_REPEAT);
        LoadBitmap(L"Monster\\bv02_2.jpg", BITMAP_MONSTER_SKIN + 1, GL_LINEAR, GL_REPEAT);
        Models[MODEL_MONSTER01 + Type].BoneHead = 20;
        break;
    case MONSTER_MODEL_BLOODY_WOLF:
        LoadWaveFile(SOUND_MONSTER_BLOOD1, L"Data\\Sound\\blood1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_BLOOD_ATTACK1, L"Data\\Sound\\blood_attack1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_BLOOD_ATTACK2, L"Data\\Sound\\blood_attack2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_BLOOD_DIE, L"Data\\Sound\\blood_die.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, 150, 150, 151, 152, 153);
        Models[MODEL_MONSTER01 + Type].BoneHead = 7;
        break;
    case MONSTER_MODEL_BEAM_KNIGHT:
        LoadWaveFile(SOUND_MONSTER_DEATH1, L"Data\\Sound\\death1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_DEATH_ATTACK1, L"Data\\Sound\\death_attack1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_DEATH_DIE, L"Data\\Sound\\death_die.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, 154, 154, 155, 155, 156);
        Models[MODEL_MONSTER01 + Type].BoneHead = 6;
        break;
    case MONSTER_MODEL_MUTANT:
        LoadWaveFile(SOUND_MONSTER_UTANT1, L"Data\\Sound\\mutant1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_UTANT2, L"Data\\Sound\\mutant2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_UTANT_ATTACK1, L"Data\\Sound\\mutant_attack1.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, 157, 158, 159, 159, 159);
        Models[MODEL_MONSTER01 + Type].BoneHead = 6;
        break;
    case MONSTER_MODEL_ORC_ARCHER:
        LoadWaveFile(SOUND_MONSTER_ORCARCHERATTACK1, L"Data\\Sound\\mOrcArcherAttack1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_BULLDIE, L"Data\\Sound\\mBullDie.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, -1, -1, 162, 162, 4);
        Models[MODEL_MONSTER01 + Type].BoneHead = 7;
        break;
    case MONSTER_MODEL_ORC:
        LoadWaveFile(SOUND_MONSTER_HUNTER2, L"Data\\Sound\\mHunter2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_BULLDIE, L"Data\\Sound\\mBullDie.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_ORCCAPATTACK1, L"Data\\Sound\\mOrcCapAttack1.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, 86, 86, 161, 161, 4);
        Models[MODEL_MONSTER01 + Type].BoneHead = 20;
        break;
    case MONSTER_MODEL_CURSED_KING:
        LoadWaveFile(SOUND_MONSTER_CURSEDKING1, L"Data\\Sound\\mCursedKing1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_CURSEDKING2, L"Data\\Sound\\mCursedKing2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_CURSEDKINGDIE1, L"Data\\Sound\\mCursedKingDie1.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, 160, 164, -1, -1, 163);
        Models[MODEL_MONSTER01 + Type].BoneHead = 20;
        break;
    case MONSTER_MODEL_CRUST:
        LoadBitmap(L"Monster\\iui02.tga", BITMAP_ROBE + 3);
        LoadBitmap(L"Monster\\iui03.tga", BITMAP_ROBE + 5);
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.22f;
        LoadWaveFile(SOUND_MONSTER_MEGACRUST1, L"Data\\Sound\\mMegaCrust1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_MEGACRUSTATTACK1, L"Data\\Sound\\mMegaCrustAttack1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_MEGACRUSTDIE, L"Data\\Sound\\mMegaCrustDie.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, 180, 180, 181, 181, 182);
        break;
    case MONSTER_MODEL_MOLT:
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.22f;
        LoadWaveFile(SOUND_MONSTER_MOLT1, L"Data\\Sound\\mMolt1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_MOLTATTACK1, L"Data\\Sound\\mMoltAttack1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_MOLTDIE, L"Data\\Sound\\mMoltDie.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, 177, 177, 178, 178, 179);
        break;
    case MONSTER_MODEL_ALQUAMOS:
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.22f;
        //LoadWaveFile(SOUND_MONSTER+174,"Data\\Sound\\mAlquamos1.wav"    ,Channel,Enable);
        LoadWaveFile(SOUND_MONSTER_ALQUAMOSATTACK1, L"Data\\Sound\\mAlquamosAttack1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_ALQUAMOSDIE, L"Data\\Sound\\mAlquamosDie.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, 175, 175, 175, 175, 176);
        break;
    case MONSTER_MODEL_QUEEN_RAINER:
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.22f;
        LoadWaveFile(SOUND_MONSTER_RAINNER1, L"Data\\Sound\\mRainner1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_RAINNERATTACK1, L"Data\\Sound\\mRainnerAttack1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_RAINNERDIE, L"Data\\Sound\\mRainnerDie.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, 171, -1, 172, 172, 173);
        break;
    case MONSTER_MODEL_PHANTOM_KNIGHT:
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.22f;
        LoadWaveFile(SOUND_MONSTER_PHANTOM1, L"Data\\Sound\\mPhantom1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_PHANTOMATTACK1, L"Data\\Sound\\mPhantomAttack1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_PHANTOMDIE, L"Data\\Sound\\mPhantomDie.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, 168, 168, 169, 169, 170);
        break;
    case MONSTER_MODEL_DRAKAN:
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.22f;
        LoadWaveFile(SOUND_MONSTER_DRAKAN1, L"Data\\Sound\\mDrakan1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_DRAKANATTACK1, L"Data\\Sound\\mDrakanAttack1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_DRAKANDIE, L"Data\\Sound\\mDrakanDie.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, 165, 165, 166, 166, 167);
        break;
    case MONSTER_MODEL_DARK_PHOENIX_SHIELD:
        LoadWaveFile(SOUND_MONSTER_PHOENIX1, L"Data\\Sound\\mPhoenix1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_PHOENIX1, L"Data\\Sound\\mPhoenix1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_PHOENIXATTACK1, L"Data\\Sound\\mPhoenixAttack1.wav", Channel, Enable);
        //LoadWaveFile(SOUND_MONSTER+186,"Data\\Sound\\mDarkPhoenixDie.wav"    ,Channel,Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, 183, 184, 185, 185, -1);
    case MONSTER_MODEL_DARK_PHOENIX:
        b->Actions[MONSTER01_DIE].PlaySpeed = 0.22f;
        //b->Actions[MONSTER01_ATTACK1].PlaySpeed = 0.01f;
        //b->Actions[MONSTER01_ATTACK2].PlaySpeed = 0.01f;
        break;
    case MONSTER_MODEL_MAGIC_SKELETON:
        LoadWaveFile(SOUND_MONSTER_MAGICSKULL1, L"Data\\Sound\\mMagicSkull.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_MAGICSKULL2, L"Data\\Sound\\mMagicSkull.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, 186, -1, -1, -1, 187);
        break;
    case MONSTER_MODEL_CASTLE_GATE:
        break;
    case MONSTER_MODEL_STATUE_OF_SAINT:
        break;
    case MONSTER_MODEL_DARK_SKULL_SOLDIER:
        LoadWaveFile(SOUND_MONSTER_BULLDIE, L"Data\\Sound\\mBullDie.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_HUNTER2, L"Data\\Sound\\mHunter2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_BLACKSKULLDIE, L"Data\\Sound\\mBlackSkullDie.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_BLACKSKULLATTACK, L"Data\\Sound\\mBlackSkullAttack.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, 188, -1, 190, -1, 189);
        break;
    case MONSTER_MODEL_GIANT_OGRE:
        LoadWaveFile(SOUND_MONSTER_HUNTER2, L"Data\\Sound\\mHunter2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_GHAINTORGERDIE, L"Data\\Sound\\mGhaintOrgerDie.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, 188, -1, 190, -1, 191);
        break;
    case MONSTER_MODEL_RED_SKELETON_KNIGHT:
        LoadWaveFile(SOUND_MONSTER_REDSKULL, L"Data\\Sound\\mRedSkull.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_REDSKULLDIE, L"Data\\Sound\\mRedSkullDie.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_REDSKULLATTACK, L"Data\\Sound\\mRedSkullAttack.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, 192, -1, 194, -1, 193);
        break;
    case MONSTER_MODEL_DEATH_ANGEL:
        LoadWaveFile(SOUND_MONSTER_DANGELIDLE, L"Data\\Sound\\mDAngelIdle.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_DANGELATTACK, L"Data\\Sound\\mDAngelAttack.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_DANGELDEATH, L"Data\\Sound\\mDAngelDeath.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, 195, 195, 196, 196, 197);
        break;
    case MONSTER_MODEL_ILLUSION_OF_KUNDUN:
        LoadWaveFile(SOUND_MONSTER_OCDOORDIS, L"Data\\Sound\\mKundunIdle.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_BOWMERCATTACK, L"Data\\Sound\\mKundunAttack1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_BOWMERCDEATH, L"Data\\Sound\\mKundunAttack2.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, 232, 232, 233, 234, -1);
        break;
    case MONSTER_MODEL_BLOOD_SOLDIER:
        LoadWaveFile(SOUND_MONSTER_BSOLDIERIDLE1, L"Data\\Sound\\mBSoldierIdle1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_BSOLDIERIDLE2, L"Data\\Sound\\mBSoldierIdle2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_BSOLDIERATTACK1, L"Data\\Sound\\mBSoldierAttack1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_BSOLDIERATTACK2, L"Data\\Sound\\mBSoldierAttack2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_BSOLDIERDEATH, L"Data\\Sound\\mBSoldierDeath.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, 198, 199, 200, 201, 202);
        break;
    case MONSTER_MODEL_AEGIS:
        LoadWaveFile(SOUND_MONSTER_ESISIDLE, L"Data\\Sound\\mEsisIdle.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_ESISATTACK1, L"Data\\Sound\\mEsisAttack1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_ESISATTACK2, L"Data\\Sound\\mEsisAttack2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_ESISDEATH, L"Data\\Sound\\mEsisDeath.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, 203, 203, 204, 205, 206);
        break;
    case MONSTER_MODEL_DEATH_CENTURION:
        LoadWaveFile(SOUND_MONSTER_DSIDLE1, L"Data\\Sound\\mDsIdle1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_DSIDLE2, L"Data\\Sound\\mDsIdle2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_DSATTACK1, L"Data\\Sound\\mDsAttack1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_DSATTACK2, L"Data\\Sound\\mDsAttack2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_DSDEATH, L"Data\\Sound\\mDsDeath.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_LSIDLE1, L"Data\\Sound\\mLsIdle1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_LSIDLE2, L"Data\\Sound\\mLsIdle2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_LSATTACK1, L"Data\\Sound\\mLsAttack1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_LSATTACK2, L"Data\\Sound\\mLsAttack2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_LSDEATH, L"Data\\Sound\\mLsDeath.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, 207, 208, 209, 210, 211, 212, 213, 214, 215, 216);
        break;
    case MONSTER_MODEL_NECRON:
        LoadWaveFile(SOUND_MONSTER_NECRONIDLE1, L"Data\\Sound\\mNecronIdle1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_NECRONIDLE2, L"Data\\Sound\\mNecronIdle2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_NECRONATTACK1, L"Data\\Sound\\mNecronAttack1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_NECRONATTACK2, L"Data\\Sound\\mNecronAttack2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_NECRONDEATH, L"Data\\Sound\\mNecronDeath.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, 217, 218, 219, 220, 221);
        break;
    case MONSTER_MODEL_SHRIKER:
        LoadWaveFile(SOUND_MONSTER_SVIDLE1, L"Data\\Sound\\mSvIdle1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_SVIDLE2, L"Data\\Sound\\mSvIdle2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_SVATTACK1, L"Data\\Sound\\mSvAttack1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_SVATTACK2, L"Data\\Sound\\mSvAttack2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_SVDEATH, L"Data\\Sound\\mSvDeath.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_LVIDLE1, L"Data\\Sound\\mLvIdle1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_LVIDLE2, L"Data\\Sound\\mLvIdle2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_LVATTACK1, L"Data\\Sound\\mLvAttack1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_LVATTACK2, L"Data\\Sound\\mLvAttack2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_LVDEATH, L"Data\\Sound\\mLvDeath.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231);
        break;
    case MONSTER_MODEL_CASTLE_GATE1:
        LoadWaveFile(SOUND_MONSTER_OCDOORDIS, L"Data\\Sound\\BattleCastle\\oCDoorDis.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, -1, -1, -1, -1, 232);
        break;
    case MONSTER_MODEL_LIFE_STONE:
        SetMonsterSound(MODEL_MONSTER01 + Type, -1, -1, -1, -1, -1);
        b->Actions[MONSTER01_STOP1].PlaySpeed = 0.05f;
        b->Actions[MONSTER01_STOP2].PlaySpeed = 0.05f;
        b->Actions[MONSTER01_WALK].PlaySpeed = 0.1f;
        break;
    case MONSTER_MODEL_BATTLE_GUARD1:
        LoadWaveFile(SOUND_MONSTER_BOWMERCATTACK, L"Data\\Sound\\BattleCastle\\mBowMercAttack.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_BOWMERCDEATH, L"Data\\Sound\\BattleCastle\\mBowMercDeath.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, -1, -1, 233, 233, 234);
        break;
    case MONSTER_MODEL_BATTLE_GUARD2:
        LoadWaveFile(SOUND_MONSTER_SPEARMERCATTACK, L"Data\\Sound\\BattleCastle\\mSpearMercAttack.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_SPEARMERCDEATH, L"Data\\Sound\\BattleCastle\\mSpearMercDeath.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, -1, -1, 235, 235, 236);
        break;
    case MONSTER_MODEL_CANON_TOWER:
        break;
    case MONSTER_MODEL_RABBIT:
        LoadWaveFile(SOUND_ELBELAND_RABBITSTRANGE_ATTACK01, L"Data\\Sound\\w52\\SE_Mon_rabbitstrange_attack01.wav", 1);
        LoadWaveFile(SOUND_ELBELAND_RABBITSTRANGE_DEATH01, L"Data\\Sound\\w52\\SE_Mon_rabbitstrange_death01.wav", 1);
        break;
    case MONSTER_MODEL_BUTTERFLY:
        LoadWaveFile(SOUND_ELBELAND_RABBITUGLY_BREATH01, L"Data\\Sound\\w52\\SE_Mon_rabbitugly_breath01.wav", 1);
        LoadWaveFile(SOUND_ELBELAND_RABBITUGLY_ATTACK01, L"Data\\Sound\\w52\\SE_Mon_rabbitugly_attack01.wav", 1);
        LoadWaveFile(SOUND_ELBELAND_RABBITUGLY_DEATH01, L"Data\\Sound\\w52\\SE_Mon_rabbitugly_death01.wav", 1);
        break;
    case MONSTER_MODEL_HIDEOUS_RABBIT:
        LoadWaveFile(SOUND_ELBELAND_WOLFHUMAN_MOVE02, L"Data\\Sound\\w52\\SE_Mon_wolfhuman_move02.wav", 1);
        LoadWaveFile(SOUND_ELBELAND_WOLFHUMAN_ATTACK01, L"Data\\Sound\\w52\\SE_Mon_wolfhuman_attack01.wav", 1);
        LoadWaveFile(SOUND_ELBELAND_WOLFHUMAN_DEATH01, L"Data\\Sound\\w52\\SE_Mon_wolfhuman_death01.wav", 1);
        break;
    case MONSTER_MODEL_WEREWOLF2:
        LoadWaveFile(SOUND_ELBELAND_BUTTERFLYPOLLUTION_MOVE01, L"Data\\Sound\\w52\\SE_Mon_butterflypollution_move01.wav", 1);
        LoadWaveFile(SOUND_ELBELAND_BUTTERFLYPOLLUTION_DEATH01, L"Data\\Sound\\w52\\SE_Mon_butterflypollution_death01.wav", 1);
        break;
    case MONSTER_MODEL_CURSED_LICH:
        LoadWaveFile(SOUND_ELBELAND_CURSERICH_MOVE01, L"Data\\Sound\\w52\\SE_Mon_curserich_move01.wav", 1);
        LoadWaveFile(SOUND_ELBELAND_CURSERICH_ATTACK01, L"Data\\Sound\\w52\\SE_Mon_curserich_attack01.wav", 1);
        LoadWaveFile(SOUND_ELBELAND_CURSERICH_DEATH01, L"Data\\Sound\\w52\\SE_Mon_curserich_death01.wav", 1);
        break;
    case MONSTER_MODEL_TOTEM_GOLEM:
        LoadWaveFile(SOUND_ELBELAND_TOTEMGOLEM_MOVE01, L"Data\\Sound\\w52\\SE_Mon_totemgolem_move01.wav", 1);
        LoadWaveFile(SOUND_ELBELAND_TOTEMGOLEM_MOVE02, L"Data\\Sound\\w52\\SE_Mon_totemgolem_move02.wav", 1);
        LoadWaveFile(SOUND_ELBELAND_TOTEMGOLEM_ATTACK01, L"Data\\Sound\\w52\\SE_Mon_totemgolem_attack01.wav", 1);
        LoadWaveFile(SOUND_ELBELAND_TOTEMGOLEM_ATTACK02, L"Data\\Sound\\w52\\SE_Mon_totemgolem_attack02.wav", 1);
        LoadWaveFile(SOUND_ELBELAND_TOTEMGOLEM_DEATH01, L"Data\\Sound\\w52\\SE_Mon_totemgolem_death01.wav", 1);
        break;
    case MONSTER_MODEL_GRIZZLY:
        LoadWaveFile(SOUND_ELBELAND_BEASTWOO_MOVE01, L"Data\\Sound\\w52\\SE_Mon_beastwoo_move01.wav", 1);
        LoadWaveFile(SOUND_ELBELAND_BEASTWOO_ATTACK01, L"Data\\Sound\\w52\\SE_Mon_beastwoo_attack01.wav", 1);
        LoadWaveFile(SOUND_ELBELAND_BEASTWOO_DEATH01, L"Data\\Sound\\w52\\SE_Mon_beastwoo_death01.wav", 1);
        break;
    case MONSTER_MODEL_CAPTAIN_GRIZZLY:
        LoadWaveFile(SOUND_ELBELAND_BEASTWOOLEADER_MOVE01, L"Data\\Sound\\w52\\SE_Mon_beastwooleader_move01.wav", 1);
        LoadWaveFile(SOUND_ELBELAND_BEASTWOOLEADER_ATTACK01, L"Data\\Sound\\w52\\SE_Mon_beastwooleader_attack01.wav", 1);
        LoadWaveFile(SOUND_ELBELAND_BEASTWOO_DEATH01, L"Data\\Sound\\w52\\SE_Mon_beastwoo_death01.wav", 1);
        break;
    case MONSTER_MODEL_SAPIUNUS:
        LoadWaveFile(SOUND_SWAMPOFQUIET_SAPI_UNUS_ATTACK01, L"Data\\Sound\\w57\\Sapi-Attack.wav", 1);
        LoadWaveFile(SOUND_SWAMPOFQUIET_SAPI_DEATH01, L"Data\\Sound\\w57\\Sapi-Death.wav", 1);
        break;
    case MONSTER_MODEL_SAPIDUO:
        LoadWaveFile(SOUND_SWAMPOFQUIET_SAPI_UNUS_ATTACK01, L"Data\\Sound\\w57\\Sapi-Attack.wav", 1);
        LoadWaveFile(SOUND_SWAMPOFQUIET_SAPI_DEATH01, L"Data\\Sound\\w57\\Sapi-Death.wav", 1);
        break;
    case MONSTER_MODEL_SAPITRES:
        LoadWaveFile(SOUND_SWAMPOFQUIET_SAPI_TRES_ATTACK01, L"Data\\Sound\\w57\\Sapi-Attack1.wav", 1);
        LoadWaveFile(SOUND_SWAMPOFQUIET_SAPI_DEATH01, L"Data\\Sound\\w57\\Sapi-Death.wav", 1);
        break;
    case MONSTER_MODEL_SHADOW_PAWN:
        LoadWaveFile(SOUND_SWAMPOFQUIET_SHADOW_PAWN_ATTACK01, L"Data\\Sound\\w57\\ShadowPawn-Attack.wav", 1);
        LoadWaveFile(SOUND_SWAMPOFQUIET_SHADOW_DEATH01, L"Data\\Sound\\w57\\Shadow-Death.wav", 1);
        break;
    case MONSTER_MODEL_SHADOW_KNIGHT:
        LoadWaveFile(SOUND_SWAMPOFQUIET_SHADOW_KNIGHT_ATTACK01, L"Data\\Sound\\w57\\ShadowKnight-Attack.wav", 1);
        LoadWaveFile(SOUND_SWAMPOFQUIET_SHADOW_DEATH01, L"Data\\Sound\\w57\\Shadow-Death.wav", 1);
        break;
    case MONSTER_MODEL_SHADOW_LOOK:
        LoadWaveFile(SOUND_SWAMPOFQUIET_SHADOW_ROOK_ATTACK01, L"Data\\Sound\\w57\\ShadowRook-Attack.wav", 1);
        LoadWaveFile(SOUND_SWAMPOFQUIET_SHADOW_DEATH01, L"Data\\Sound\\w57\\Shadow-Death.wav", 1);
        break;
    case MONSTER_MODEL_NAPIN:
        LoadWaveFile(SOUND_SWAMPOFQUIET_THUNDER_NAIPIN_BREATH01, L"Data\\Sound\\w57\\Naipin-Thunder.wav", 1);
        LoadWaveFile(SOUND_SWAMPOFQUIET_NAIPIN_ATTACK01, L"Data\\Sound\\w57\\Naipin-Attack.wav", 1);
        LoadWaveFile(SOUND_SWAMPOFQUIET_NAIPIN_DEATH01, L"Data\\Sound\\w57\\Naipin-Death.wav", 1);
        break;
    case MONSTER_MODEL_GHOST_NAPIN:
        LoadWaveFile(SOUND_SWAMPOFQUIET_GHOST_NAIPIN_BREATH01, L"Data\\Sound\\w57\\Naipin-Ghost.wav", 1);
        LoadWaveFile(SOUND_SWAMPOFQUIET_NAIPIN_ATTACK01, L"Data\\Sound\\w57\\Naipin-Attack.wav", 1);
        LoadWaveFile(SOUND_SWAMPOFQUIET_NAIPIN_DEATH01, L"Data\\Sound\\w57\\Naipin-Death.wav", 1);
        break;
    case MONSTER_MODEL_BLAZE_NAPIN:
        LoadWaveFile(SOUND_SWAMPOFQUIET_BLAZE_NAIPIN_BREATH01, L"Data\\Sound\\w57\\Naipin-Blaze.wav", 1);
        LoadWaveFile(SOUND_SWAMPOFQUIET_NAIPIN_ATTACK01, L"Data\\Sound\\w57\\Naipin-Attack.wav", 1);
        LoadWaveFile(SOUND_SWAMPOFQUIET_NAIPIN_DEATH01, L"Data\\Sound\\w57\\Naipin-Death.wav", 1);
        break;
    case MONSTER_MODEL_ICE_WALKER:
        LoadWaveFile(SOUND_ELBELAND_WOLFHUMAN_DEATH01, L"Data\\Sound\\w52\\SE_Mon_wolfhuman_death01.wav", 1);
        LoadWaveFile(SOUND_RAKLION_ICEWALKER_ATTACK, L"Data\\Sound\\w58w59\\IceWalker_attack.wav", 1);
        LoadWaveFile(SOUND_RAKLION_ICEWALKER_MOVE, L"Data\\Sound\\w58w59\\IceWalker_move.wav", 1);
        break;
    case MONSTER_MODEL_GIANT_MAMMOTH:
        LoadWaveFile(SOUND_RAKLION_GIANT_MAMUD_MOVE, L"Data\\Sound\\w58w59\\GiantMammoth_move.wav", 1);
        LoadWaveFile(SOUND_RAKLION_GIANT_MAMUD_ATTACK, L"Data\\Sound\\w58w59\\GiantMammoth_attack.wav", 1);
        LoadWaveFile(SOUND_RAKLION_GIANT_MAMUD_DEATH, L"Data\\Sound\\w58w59\\GiantMammoth_death.wav", 1);
        break;
    case MONSTER_MODEL_ICE_GIANT:
        LoadWaveFile(SOUND_RAKLION_ICEGIANT_MOVE, L"Data\\Sound\\w58w59\\IceGiant_move.wav", 1);
        LoadWaveFile(SOUND_RAKLION_ICEGIANT_DEATH, L"Data\\Sound\\w58w59\\IceGiant_death.wav", 1);
        break;
    case MONSTER_MODEL_COOLUTIN:
        // LoadWaveFile(SOUND_MONSTER_HELLSPIDERDIE, L"Data\\Sound\\m헬스파이더죽기.wav", 1);
        LoadWaveFile(SOUND_RAKLION_COOLERTIN_ATTACK, L"Data\\Sound\\w58w59\\Coolertin_attack.wav", 1);
        LoadWaveFile(SOUND_RAKLION_COOLERTIN_MOVE, L"Data\\Sound\\w58w59\\Coolertin_move.wav", 1);
        break;
    case MONSTER_MODEL_IRON_KNIGHT:
        LoadWaveFile(SOUND_RAKLION_IRON_KNIGHT_MOVE, L"Data\\Sound\\w58w59\\IronKnight_move.wav", 1);
        LoadWaveFile(SOUND_RAKLION_IRON_KNIGHT_ATTACK, L"Data\\Sound\\w58w59\\IronKnight_attack.wav", 1);
        LoadWaveFile(SOUND_MONSTER_DEATH1, L"Data\\Sound\\death1.wav", 1);
        break;
    case MONSTER_MODEL_SELUPAN:
        LoadWaveFile(SOUND_RAKLION_SERUFAN_ATTACK1, L"Data\\Sound\\w58w59\\Selupan_attack1.wav", 1);
        LoadWaveFile(SOUND_RAKLION_SERUFAN_ATTACK2, L"Data\\Sound\\w58w59\\Selupan_attack2.wav", 1);
        LoadWaveFile(SOUND_RAKLION_SERUFAN_CURE, L"Data\\Sound\\w58w59\\Selupan_cure.wav", 1);
        LoadWaveFile(SOUND_RAKLION_SERUFAN_RAGE, L"Data\\Sound\\w58w59\\Selupan_rage.wav", 1);
        LoadWaveFile(SOUND_RAKLION_SERUFAN_WORD1, L"Data\\Sound\\w58w59\\Selupan_word1.wav", 1);
        LoadWaveFile(SOUND_RAKLION_SERUFAN_WORD2, L"Data\\Sound\\w58w59\\Selupan_word2.wav", 1);
        LoadWaveFile(SOUND_RAKLION_SERUFAN_WORD3, L"Data\\Sound\\w58w59\\Selupan_word3.wav", 1);
        LoadWaveFile(SOUND_RAKLION_SERUFAN_WORD4, L"Data\\Sound\\w58w59\\Selupan_word4.wav", 1);
        break;
    case MONSTER_MODEL_CURSED_SANTA:
        LoadWaveFile(SOUND_XMAS_SANTA_IDLE_1, L"Data\\Sound\\xmas\\DarkSanta_Idle01.wav");
        LoadWaveFile(SOUND_XMAS_SANTA_IDLE_2, L"Data\\Sound\\xmas\\DarkSanta_Idle02.wav");
        LoadWaveFile(SOUND_XMAS_SANTA_WALK_1, L"Data\\Sound\\xmas\\DarkSanta_Walk01.wav");
        LoadWaveFile(SOUND_XMAS_SANTA_WALK_2, L"Data\\Sound\\xmas\\DarkSanta_Walk02.wav");
        LoadWaveFile(SOUND_XMAS_SANTA_ATTACK_1, L"Data\\Sound\\xmas\\DarkSanta_Attack01.wav");
        LoadWaveFile(SOUND_XMAS_SANTA_DAMAGE_1, L"Data\\Sound\\xmas\\DarkSanta_Damage01.wav");
        LoadWaveFile(SOUND_XMAS_SANTA_DAMAGE_2, L"Data\\Sound\\xmas\\DarkSanta_Damage02.wav");
        LoadWaveFile(SOUND_XMAS_SANTA_DEATH_1, L"Data\\Sound\\xmas\\DarkSanta_Death01.wav");
        break;
    case MONSTER_MODEL_EVIL_GOBLIN:
        LoadWaveFile(SOUND_MONSTER_GOBLIN1, L"Data\\Sound\\mGoblin1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_GOBLIN2, L"Data\\Sound\\mGoblin2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_GOBLINATTACK1, L"Data\\Sound\\mGoblinAttack1.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_GOBLINATTACK2, L"Data\\Sound\\mGoblinAttack2.wav", Channel, Enable);
        LoadWaveFile(SOUND_MONSTER_GOBLINDIE, L"Data\\Sound\\mGoblinDie.wav", Channel, Enable);
        SetMonsterSound(MODEL_MONSTER01 + Type, 72, 73, 74, 75, 76);
        break;
    case MONSTER_MODEL_ZOMBIE_FIGHTER:
    {
        LoadWaveFile(SOUND_PKFIELD_ZOMBIEWARRIOR_ATTACK, L"Data\\Sound\\w64\\ZombieWarrior_attack.wav");
        LoadWaveFile(SOUND_PKFIELD_ZOMBIEWARRIOR_DAMAGE01, L"Data\\Sound\\w64\\ZombieWarrior_damage01.wav");
        LoadWaveFile(SOUND_PKFIELD_ZOMBIEWARRIOR_DAMAGE02, L"Data\\Sound\\w64\\ZombieWarrior_damage02.wav");
        LoadWaveFile(SOUND_PKFIELD_ZOMBIEWARRIOR_DEATH, L"Data\\Sound\\w64\\ZombieWarrior_death.wav");
        LoadWaveFile(SOUND_PKFIELD_ZOMBIEWARRIOR_MOVE01, L"Data\\Sound\\w64\\ZombieWarrior_move01.wav");
        LoadWaveFile(SOUND_PKFIELD_ZOMBIEWARRIOR_MOVE02, L"Data\\Sound\\w64\\ZombieWarrior_move02.wav");
    }
    break;
    case MONSTER_MODEL_GLADIATOR:
    {
        LoadWaveFile(SOUND_PKFIELD_RAISEDGLADIATOR_ATTACK, L"Data\\Sound\\w64\\RaisedGladiator_attack.wav");
        LoadWaveFile(SOUND_PKFIELD_RAISEDGLADIATOR_DAMAGE01, L"Data\\Sound\\w64\\RaisedGladiator_damage01.wav");
        LoadWaveFile(SOUND_PKFIELD_RAISEDGLADIATOR_DAMAGE02, L"Data\\Sound\\w64\\RaisedGladiator_damage02.wav");
        LoadWaveFile(SOUND_PKFIELD_RAISEDGLADIATOR_DEATH, L"Data\\Sound\\w64\\RaisedGladiator_death.wav");
        LoadWaveFile(SOUND_PKFIELD_RAISEDGLADIATOR_MOVE01, L"Data\\Sound\\w64\\RaisedGladiator_move01.wav");
        LoadWaveFile(SOUND_PKFIELD_RAISEDGLADIATOR_MOVE02, L"Data\\Sound\\w64\\RaisedGladiator_move02.wav");
    }
    break;
    case MONSTER_MODEL_SLAUGTHERER:
    {
        LoadWaveFile(SOUND_PKFIELD_ASHESBUTCHER_ATTACK, L"Data\\Sound\\w64\\AshesButcher_attack.wav");
        LoadWaveFile(SOUND_PKFIELD_ASHESBUTCHER_DAMAGE01, L"Data\\Sound\\w64\\AshesButcher_damage01.wav");
        LoadWaveFile(SOUND_PKFIELD_ASHESBUTCHER_DAMAGE02, L"Data\\Sound\\w64\\AshesButcher_damage02.wav");
        LoadWaveFile(SOUND_PKFIELD_ASHESBUTCHER_DEATH, L"Data\\Sound\\w64\\AshesButcher_death.wav");
        LoadWaveFile(SOUND_PKFIELD_ASHESBUTCHER_MOVE01, L"Data\\Sound\\w64\\AshesButcher_move01.wav");
        LoadWaveFile(SOUND_PKFIELD_ASHESBUTCHER_MOVE02, L"Data\\Sound\\w64\\AshesButcher_move02.wav");
    }
    break;
    case MONSTER_MODEL_BLOOD_ASSASSIN:
    {
        LoadWaveFile(SOUND_PKFIELD_BLOODASSASSIN_ATTACK, L"Data\\Sound\\w64\\BloodAssassin_attack.wav");
        LoadWaveFile(SOUND_PKFIELD_BLOODASSASSIN_DAMAGE01, L"Data\\Sound\\w64\\BloodAssassin_damage01.wav");
        LoadWaveFile(SOUND_PKFIELD_BLOODASSASSIN_DAMAGE02, L"Data\\Sound\\w64\\BloodAssassin_damage02.wav");
        LoadWaveFile(SOUND_PKFIELD_BLOODASSASSIN_DEDTH, L"Data\\Sound\\w64\\BloodAssassin_death.wav");
        LoadWaveFile(SOUND_PKFIELD_BLOODASSASSIN_MOVE01, L"Data\\Sound\\w64\\BloodAssassin_move01.wav");
        LoadWaveFile(SOUND_PKFIELD_BLOODASSASSIN_MOVE02, L"Data\\Sound\\w64\\BloodAssassin_move02.wav");
    }
    break;
    case MONSTER_MODEL_LAVA_GIANT:
    {
        LoadWaveFile(SOUND_PKFIELD_BURNINGLAVAGOLEM_ATTACK01, L"Data\\Sound\\w64\\BurningLavaGolem_attack01.wav");
        LoadWaveFile(SOUND_PKFIELD_BURNINGLAVAGOLEM_ATTACK02, L"Data\\Sound\\w64\\BurningLavaGolem_attack02.wav");
        LoadWaveFile(SOUND_PKFIELD_BURNINGLAVAGOLEM_DAMAGE01, L"Data\\Sound\\w64\\BurningLavaGolem_damage01.wav");
        LoadWaveFile(SOUND_PKFIELD_BURNINGLAVAGOLEM_DAMAGE02, L"Data\\Sound\\w64\\BurningLavaGolem_damage02.wav");
        LoadWaveFile(SOUND_PKFIELD_BURNINGLAVAGOLEM_DEATH, L"Data\\Sound\\w64\\BurningLavaGolem_death.wav");
        LoadWaveFile(SOUND_PKFIELD_BURNINGLAVAGOLEM_MOVE01, L"Data\\Sound\\w64\\BurningLavaGolem_move01.wav");
        LoadWaveFile(SOUND_PKFIELD_BURNINGLAVAGOLEM_MOVE02, L"Data\\Sound\\w64\\BurningLavaGolem_move02.wav");
    }
    break;
    case MONSTER_MODEL_TERRIBLE_BUTCHER:
    {
        LoadWaveFile(SOUND_DOPPELGANGER_RED_BUGBEAR_ATTACK, L"Data\\Sound\\Doppelganger\\Angerbutcher_attack.wav");
        LoadWaveFile(SOUND_DOPPELGANGER_RED_BUGBEAR_DEATH, L"Data\\Sound\\Doppelganger\\Angerbutcher_death.wav");
    }
    break;
    case MONSTER_MODEL_MAD_BUTCHER:
    {
        LoadWaveFile(SOUND_DOPPELGANGER_BUGBEAR_ATTACK, L"Data\\Sound\\Doppelganger\\Butcher_attack.wav");
        LoadWaveFile(SOUND_DOPPELGANGER_BUGBEAR_DEATH, L"Data\\Sound\\Doppelganger\\Butcher_death.wav");
    }
    break;
    case MONSTER_MODEL_DOPPELGANGER:
    {
        LoadWaveFile(SOUND_DOPPELGANGER_SLIME_ATTACK, L"Data\\Sound\\Doppelganger\\Doppelganger_attack.wav");
        LoadWaveFile(SOUND_DOPPELGANGER_SLIME_DEATH, L"Data\\Sound\\Doppelganger\\Doppelganger_death.wav");
    }
    break;
#ifdef ASG_ADD_KARUTAN_MONSTERS
    case MONSTER_MODEL_VENOMOUS_CHAIN_SCORPION:
        LoadWaveFile(SOUND_KARUTAN_TCSCORPION_ATTACK, L"Data\\Sound\\Karutan\\ToxyChainScorpion_attack.wav");
        LoadWaveFile(SOUND_KARUTAN_TCSCORPION_DEATH, L"Data\\Sound\\Karutan\\ToxyChainScorpion_death.wav");
        LoadWaveFile(SOUND_KARUTAN_TCSCORPION_HIT, L"Data\\Sound\\Karutan\\ToxyChainScorpion_hit.wav");
        break;
    case MONSTER_MODEL_BONE_SCORPION:
        LoadWaveFile(SOUND_KARUTAN_BONESCORPION_ATTACK, L"Data\\Sound\\Karutan\\BoneScorpion_attack.wav");
        LoadWaveFile(SOUND_KARUTAN_BONESCORPION_DEATH, L"Data\\Sound\\Karutan\\BoneScorpion_death.wav");
        LoadWaveFile(SOUND_KARUTAN_BONESCORPION_HIT, L"Data\\Sound\\Karutan\\BoneScorpion_hit.wav");
        break;
    case MONSTER_MODEL_ORCUS:
        LoadWaveFile(SOUND_KARUTAN_ORCUS_MOVE1, L"Data\\Sound\\Karutan\\Orcus_move1.wav");
        LoadWaveFile(SOUND_KARUTAN_ORCUS_MOVE2, L"Data\\Sound\\Karutan\\Orcus_move2.wav");
        LoadWaveFile(SOUND_KARUTAN_ORCUS_ATTACK1, L"Data\\Sound\\Karutan\\Orcus_attack_1.wav");
        LoadWaveFile(SOUND_KARUTAN_ORCUS_ATTACK2, L"Data\\Sound\\Karutan\\Orcus_attack_2.wav");
        LoadWaveFile(SOUND_KARUTAN_ORCUS_DEATH, L"Data\\Sound\\Karutan\\Orcus_death.wav");
        break;
    case MONSTER_MODEL_GOLLOCK:
        LoadWaveFile(SOUND_KARUTAN_GOLOCH_MOVE1, L"Data\\Sound\\Karutan\\Goloch_move1.wav");
        LoadWaveFile(SOUND_KARUTAN_GOLOCH_MOVE2, L"Data\\Sound\\Karutan\\Goloch_move2.wav");
        LoadWaveFile(SOUND_KARUTAN_GOLOCH_ATTACK, L"Data\\Sound\\Karutan\\Goloch_attack.wav");
        LoadWaveFile(SOUND_KARUTAN_GOLOCH_DEATH, L"Data\\Sound\\Karutan\\Goloch_death.wav");
        break;
    case MONSTER_MODEL_CRYPTA:
        LoadWaveFile(SOUND_KARUTAN_CRYPTA_MOVE1, L"Data\\Sound\\Karutan\\Crypta_move1.wav");
        LoadWaveFile(SOUND_KARUTAN_CRYPTA_MOVE2, L"Data\\Sound\\Karutan\\Crypta_move2.wav");
        LoadWaveFile(SOUND_KARUTAN_CRYPTA_ATTACK, L"Data\\Sound\\Karutan\\Crypta_attack.wav");
        LoadWaveFile(SOUND_KARUTAN_CRYPTA_DEATH, L"Data\\Sound\\Karutan\\Crypta_death.wav");
        break;
    case MONSTER_MODEL_CRYPOS:
        LoadWaveFile(SOUND_KARUTAN_CRYPOS_MOVE1, L"Data\\Sound\\Karutan\\Crypos_move1.wav");
        LoadWaveFile(SOUND_KARUTAN_CRYPOS_MOVE2, L"Data\\Sound\\Karutan\\Crypos_move2.wav");
        LoadWaveFile(SOUND_KARUTAN_CRYPOS_ATTACK1, L"Data\\Sound\\Karutan\\Crypos_attack_1.wav");
        LoadWaveFile(SOUND_KARUTAN_CRYPOS_ATTACK2, L"Data\\Sound\\Karutan\\Crypos_attack_2.wav");
        break;
    case MONSTER_MODEL_CONDRA:
        LoadWaveFile(SOUND_KARUTAN_CONDRA_MOVE1, L"Data\\Sound\\Karutan\\Condra_move1.wav");
        LoadWaveFile(SOUND_KARUTAN_CONDRA_MOVE2, L"Data\\Sound\\Karutan\\Condra_move2.wav");
        LoadWaveFile(SOUND_KARUTAN_CONDRA_ATTACK, L"Data\\Sound\\Karutan\\Condra_attack.wav");
        LoadWaveFile(SOUND_KARUTAN_CONDRA_DEATH, L"Data\\Sound\\Karutan\\Condra_death.wav");
        break;
    case MONSTER_MODEL_NACONDRA:
        LoadWaveFile(SOUND_KARUTAN_CONDRA_MOVE1, L"Data\\Sound\\Karutan\\Condra_move1.wav");
        LoadWaveFile(SOUND_KARUTAN_CONDRA_MOVE2, L"Data\\Sound\\Karutan\\Condra_move2.wav");
        LoadWaveFile(SOUND_KARUTAN_NARCONDRA_ATTACK, L"Data\\Sound\\Karutan\\NarCondra_attack.wav");
        LoadWaveFile(SOUND_KARUTAN_CONDRA_DEATH, L"Data\\Sound\\Karutan\\Condra_death.wav");
        break;
#endif	// ASG_ADD_KARUTAN_MONSTERS
    }
}

void OpenMonsterModels()
{
}

void OpenSkills()
{
    gLoadData.AccessModel(MODEL_ICE, L"Data\\Skill\\", L"Ice", 1);
    gLoadData.AccessModel(MODEL_ICE_SMALL, L"Data\\Skill\\", L"Ice", 2);
    gLoadData.AccessModel(MODEL_FIRE, L"Data\\Skill\\", L"Fire", 1);
    gLoadData.AccessModel(MODEL_POISON, L"Data\\Skill\\", L"Poison", 1);
    for (int i = 0; i < 2; i++)
        gLoadData.AccessModel(MODEL_STONE1 + i, L"Data\\Skill\\", L"Stone", i + 1);
    gLoadData.AccessModel(MODEL_CIRCLE, L"Data\\Skill\\", L"Circle", 1);
    gLoadData.AccessModel(MODEL_CIRCLE_LIGHT, L"Data\\Skill\\", L"Circle", 2);
    gLoadData.AccessModel(MODEL_MAGIC1, L"Data\\Skill\\", L"Magic", 1);
    gLoadData.AccessModel(MODEL_MAGIC2, L"Data\\Skill\\", L"Magic", 2);
    gLoadData.AccessModel(MODEL_STORM, L"Data\\Skill\\", L"Storm", 1);
    gLoadData.AccessModel(MODEL_LASER, L"Data\\Skill\\", L"Laser", 1);

    for (int i = 0; i < 3; i++)
        gLoadData.AccessModel(MODEL_SKELETON1 + i, L"Data\\Skill\\", L"Skeleton", i + 1);

    gLoadData.AccessModel(MODEL_SKELETON_PCBANG, L"Data\\Skill\\", L"Skeleton", 3);
    gLoadData.AccessModel(MODEL_HALLOWEEN, L"Data\\Skill\\", L"Jack");

    gLoadData.AccessModel(MODEL_HALLOWEEN_CANDY_BLUE, L"Data\\Skill\\", L"hcandyblue");
    gLoadData.AccessModel(MODEL_HALLOWEEN_CANDY_ORANGE, L"Data\\Skill\\", L"hcandyorange");
    gLoadData.AccessModel(MODEL_HALLOWEEN_CANDY_RED, L"Data\\Skill\\", L"hcandyred");
    gLoadData.AccessModel(MODEL_HALLOWEEN_CANDY_YELLOW, L"Data\\Skill\\", L"hcandyyellow");
    gLoadData.AccessModel(MODEL_HALLOWEEN_CANDY_HOBAK, L"Data\\Skill\\", L"hhobak");
    gLoadData.AccessModel(MODEL_HALLOWEEN_CANDY_STAR, L"Data\\Skill\\", L"hstar");
    LoadBitmap(L"Skill\\jack04.jpg", BITMAP_JACK_1);
    LoadBitmap(L"Skill\\jack05.jpg", BITMAP_JACK_2);
    LoadBitmap(L"Monster\\iui02.tga", BITMAP_ROBE + 3);
    gLoadData.AccessModel(MODEL_PUMPKIN_OF_LUCK, L"Data\\Item\\", L"hobakhead");
    gLoadData.OpenTexture(MODEL_PUMPKIN_OF_LUCK, L"Item\\");

    gLoadData.AccessModel(MODEL_CURSEDTEMPLE_ALLIED_PLAYER, L"Data\\Skill\\", L"unitedsoldier");
    gLoadData.AccessModel(MODEL_CURSEDTEMPLE_ILLUSION_PLAYER, L"Data\\Skill\\", L"illusionist");

    gLoadData.AccessModel(MODEL_WOOSISTONE, L"Data\\Skill\\", L"woositone");
    gLoadData.OpenTexture(MODEL_WOOSISTONE, L"Skill\\");

    g_NewYearsDayEvent->LoadModel();

    gLoadData.AccessModel(MODEL_SAW, L"Data\\Skill\\", L"Saw", 1);

    for (int i = 0; i < 2; i++)
        gLoadData.AccessModel(MODEL_BONE1 + i, L"Data\\Skill\\", L"Bone", i + 1);

    for (int i = 0; i < 3; i++)
        gLoadData.AccessModel(MODEL_SNOW1 + i, L"Data\\Skill\\", L"Snow", i + 1);

    gLoadData.AccessModel(MODEL_UNICON, L"Data\\Skill\\", L"Rider", 1);
    gLoadData.AccessModel(MODEL_PEGASUS, L"Data\\Skill\\", L"Rider", 2);
    gLoadData.AccessModel(MODEL_DARK_HORSE, L"Data\\Skill\\", L"DarkHorse");

    gLoadData.AccessModel(MODEL_FENRIR_BLACK, L"Data\\Skill\\", L"fenril_black");
    gLoadData.OpenTexture(MODEL_FENRIR_BLACK, L"Skill\\");

    gLoadData.AccessModel(MODEL_FENRIR_RED, L"Data\\Skill\\", L"fenril_red");
    gLoadData.OpenTexture(MODEL_FENRIR_RED, L"Skill\\");

    gLoadData.AccessModel(MODEL_FENRIR_BLUE, L"Data\\Skill\\", L"fenril_blue");
    gLoadData.OpenTexture(MODEL_FENRIR_BLUE, L"Skill\\");

    gLoadData.AccessModel(MODEL_FENRIR_GOLD, L"Data\\Skill\\", L"fenril_gold");
    gLoadData.OpenTexture(MODEL_FENRIR_BLUE, L"Skill\\");

    gLoadData.AccessModel(MODEL_PANDA, L"Data\\Item\\", L"panda");
    gLoadData.OpenTexture(MODEL_PANDA, L"Item\\");

    gLoadData.AccessModel(MODEL_SKELETON_CHANGED, L"Data\\Item\\", L"trans_skeleton");
    gLoadData.OpenTexture(MODEL_SKELETON_CHANGED, L"Item\\");

    gLoadData.AccessModel(MODEL_DARK_SPIRIT, L"Data\\Skill\\", L"DarkSpirit");
    LoadBitmap(L"Skill\\dkthreebody_r.jpg", BITMAP_MONSTER_SKIN + 2, GL_LINEAR, GL_REPEAT);

    gLoadData.AccessModel(MODEL_WARCRAFT, L"Data\\Skill\\", L"HellGate");
    Models[MODEL_WARCRAFT].Actions[0].LockPositions = false;
    SAFE_DELETE_ARRAY(Models[MODEL_WARCRAFT].Actions[0].Positions);
    Models[MODEL_WARCRAFT].Actions[0].PlaySpeed = 0.15f;

    gLoadData.AccessModel(MODEL_ARROW, L"Data\\Skill\\", L"Arrow", 1);
    gLoadData.AccessModel(MODEL_ARROW_STEEL, L"Data\\Skill\\", L"ArrowSteel", 1);
    gLoadData.AccessModel(MODEL_ARROW_THUNDER, L"Data\\Skill\\", L"ArrowThunder", 1);
    gLoadData.AccessModel(MODEL_ARROW_LASER, L"Data\\Skill\\", L"ArrowLaser", 1);
    gLoadData.AccessModel(MODEL_ARROW_V, L"Data\\Skill\\", L"ArrowV", 1);
    gLoadData.AccessModel(MODEL_ARROW_SAW, L"Data\\Skill\\", L"ArrowSaw", 1);
    gLoadData.AccessModel(MODEL_ARROW_NATURE, L"Data\\Skill\\", L"ArrowNature", 1);

    gLoadData.OpenTexture(MODEL_MAGIC_CAPSULE2, L"Skill\\");
    gLoadData.AccessModel(MODEL_MAGIC_CAPSULE2, L"Data\\Skill\\", L"Protect", 2);

    gLoadData.AccessModel(MODEL_ARROW_SPARK, L"Data\\Skill\\", L"Arrow_Spark");
    gLoadData.OpenTexture(MODEL_ARROW_SPARK, L"Skill\\");

    gLoadData.AccessModel(MODEL_DARK_SCREAM, L"Data\\Skill\\", L"darkfirescrem02");
    gLoadData.OpenTexture(MODEL_DARK_SCREAM, L"Skill\\");
    gLoadData.AccessModel(MODEL_DARK_SCREAM_FIRE, L"Data\\Skill\\", L"darkfirescrem01");
    gLoadData.OpenTexture(MODEL_DARK_SCREAM_FIRE, L"Skill\\");
    gLoadData.AccessModel(MODEL_SUMMON, L"Data\\SKill\\", L"nightmaresum");
    gLoadData.OpenTexture(MODEL_SUMMON, L"SKill\\");
    gLoadData.AccessModel(MODEL_MULTI_SHOT1, L"Data\\Effect\\", L"multishot01");
    gLoadData.OpenTexture(MODEL_MULTI_SHOT1, L"Effect\\");
    gLoadData.AccessModel(MODEL_MULTI_SHOT2, L"Data\\Effect\\", L"multishot02");
    gLoadData.OpenTexture(MODEL_MULTI_SHOT2, L"Effect\\");
    gLoadData.AccessModel(MODEL_MULTI_SHOT3, L"Data\\Effect\\", L"multishot03");
    gLoadData.OpenTexture(MODEL_MULTI_SHOT3, L"Effect\\");
    gLoadData.AccessModel(MODEL_DESAIR, L"Data\\SKill\\", L"desair");
    gLoadData.OpenTexture(MODEL_DESAIR, L"SKill\\");
    gLoadData.AccessModel(MODEL_ARROW_RING, L"Data\\Skill\\", L"CW_Bow_Skill");
    gLoadData.OpenTexture(MODEL_ARROW_RING, L"Skill\\");

    gLoadData.AccessModel(MODEL_ARROW_DARKSTINGER, L"Data\\Skill\\", L"sketbows_arrows");
    gLoadData.OpenTexture(MODEL_ARROW_DARKSTINGER, L"Skill\\");
    gLoadData.AccessModel(MODEL_FEATHER, L"Data\\Skill\\", L"darkwing_hetachi");
    gLoadData.OpenTexture(MODEL_FEATHER, L"Skill\\");
    gLoadData.AccessModel(MODEL_FEATHER_FOREIGN, L"Data\\Skill\\", L"darkwing_hetachi");
    gLoadData.OpenTexture(MODEL_FEATHER_FOREIGN, L"Skill\\");
    LoadBitmap(L"Effect\\Bugbear_R.jpg", BITMAP_BUGBEAR_R, GL_LINEAR, GL_REPEAT);
    LoadBitmap(L"Effect\\pk_mon02_fire.jpg", BITMAP_PKMON01, GL_LINEAR, GL_REPEAT);
    LoadBitmap(L"Effect\\pk_mon03_red.jpg", BITMAP_PKMON02, GL_LINEAR, GL_REPEAT);
    LoadBitmap(L"Effect\\pk_mon02_green.jpg", BITMAP_PKMON03, GL_LINEAR, GL_REPEAT);
    LoadBitmap(L"Effect\\pk_mon03_green.jpg", BITMAP_PKMON04, GL_LINEAR, GL_REPEAT);
    LoadBitmap(L"Effect\\lavagiantAa_e.jpg", BITMAP_PKMON05, GL_LINEAR, GL_REPEAT);
    LoadBitmap(L"Effect\\lavagiantBa_e.jpg", BITMAP_PKMON06, GL_LINEAR, GL_REPEAT);
    LoadBitmap(L"Effect\\eff_magma_red.jpg", BITMAP_LAVAGIANT_FOOTPRINT_R, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\eff_magma_violet.jpg", BITMAP_LAVAGIANT_FOOTPRINT_V, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\raymond_sword_R.jpg", BITMAP_RAYMOND_SWORD, GL_LINEAR, GL_REPEAT);
    LoadBitmap(L"Effect\\mist01.jpg", BITMAP_AG_ADDITION_EFFECT, GL_LINEAR, GL_REPEAT);
    gLoadData.AccessModel(MODEL_ARROW_GAMBLE, L"Data\\Skill\\", L"gamble_arrows01");
    gLoadData.OpenTexture(MODEL_ARROW_GAMBLE, L"Skill\\");

    gLoadData.OpenTexture(MODEL_SPEARSKILL, L"Skill\\");
    gLoadData.AccessModel(MODEL_SPEARSKILL, L"Data\\Skill\\", L"RidingSpear", 1);
    gLoadData.AccessModel(MODEL_PROTECT, L"Data\\Skill\\", L"Protect", 1);

    for (int i = 0; i < 2; i++)
        gLoadData.AccessModel(MODEL_BIG_STONE1 + i, L"Data\\Skill\\", L"BigStone", i + 1);

    gLoadData.AccessModel(MODEL_MAGIC_CIRCLE1, L"Data\\Skill\\", L"MagicCircle", 1);
    gLoadData.AccessModel(MODEL_ARROW_WING, L"Data\\Skill\\", L"ArrowWing", 1);
    gLoadData.AccessModel(MODEL_ARROW_BOMB, L"Data\\Skill\\", L"ArrowBomb", 1);
    gLoadData.AccessModel(MODEL_BALL, L"Data\\Skill\\", L"Ball", 1);//공
    Models[MODEL_BALL].Actions[0].PlaySpeed = 0.5f;
    gLoadData.AccessModel(MODEL_SKILL_BLAST, L"Data\\Skill\\", L"Blast", 1);
    gLoadData.AccessModel(MODEL_SKILL_INFERNO, L"Data\\Skill\\", L"Inferno", 1);
    gLoadData.AccessModel(MODEL_ARROW_DOUBLE, L"Data\\Skill\\", L"ArrowDouble", 1);

    gLoadData.AccessModel(MODEL_ARROW_BEST_CROSSBOW, L"Data\\Skill\\", L"KCross");
    gLoadData.AccessModel(MODEL_ARROW_DRILL, L"Data\\Skill\\", L"Carow");
    gLoadData.AccessModel(MODEL_COMBO, L"Data\\Skill\\", L"combo");

    gLoadData.AccessModel(MODEL_GATE + 0, L"Data\\Object12\\", L"Gate", 1);
    gLoadData.AccessModel(MODEL_GATE + 1, L"Data\\Object12\\", L"Gate", 2);
    gLoadData.AccessModel(MODEL_STONE_COFFIN + 0, L"Data\\Object12\\", L"StoneCoffin", 1);
    gLoadData.AccessModel(MODEL_STONE_COFFIN + 1, L"Data\\Object12\\", L"StoneCoffin", 2);

    for (int i = 0; i < 2; ++i)
    {
        gLoadData.OpenTexture(MODEL_GATE + 1, L"Monster\\");
        gLoadData.OpenTexture(MODEL_STONE_COFFIN + i, L"Monster\\");
    }

    gLoadData.AccessModel(MODEL_AIR_FORCE, L"Data\\Skill\\", L"AirForce");
    gLoadData.AccessModel(MODEL_WAVES, L"Data\\Skill\\", L"m_Waves");
    gLoadData.AccessModel(MODEL_PIERCING2, L"Data\\Skill\\", L"m_Piercing");
    gLoadData.AccessModel(MODEL_PIER_PART, L"Data\\Skill\\", L"PierPart");
    gLoadData.AccessModel(MODEL_DARKLORD_SKILL, L"Data\\Skill\\", L"DarkLordSkill");
    gLoadData.AccessModel(MODEL_GROUND_STONE, L"Data\\Skill\\", L"groundStone");
    Models[MODEL_GROUND_STONE].Actions[0].Loop = false;
    gLoadData.AccessModel(MODEL_GROUND_STONE2, L"Data\\Skill\\", L"groundStone2");
    Models[MODEL_GROUND_STONE2].Actions[0].Loop = false;
    gLoadData.AccessModel(MODEL_WATER_WAVE, L"Data\\Skill\\", L"seamanFX");
    gLoadData.AccessModel(MODEL_SKULL, L"Data\\Skill\\", L"Skull");
    gLoadData.AccessModel(MODEL_LACEARROW, L"Data\\Skill\\", L"LaceArrow");
    gLoadData.OpenTexture(MODEL_LACEARROW, L"Item\\", GL_CLAMP_TO_EDGE);

    gLoadData.AccessModel(MODEL_MANY_FLAG, L"Data\\Skill\\", L"ManyFlag");
    gLoadData.AccessModel(MODEL_WEBZEN_MARK, L"Data\\Skill\\", L"MuSign");
    gLoadData.AccessModel(MODEL_STUN_STONE, L"Data\\Skill\\", L"GroundCrystal");
    gLoadData.AccessModel(MODEL_SKIN_SHELL, L"Data\\Skill\\", L"skinshell");
    gLoadData.AccessModel(MODEL_MANA_RUNE, L"Data\\Skill\\", L"ManaRune");
    gLoadData.AccessModel(MODEL_SKILL_JAVELIN, L"Data\\Skill\\", L"Javelin");
    gLoadData.AccessModel(MODEL_ARROW_IMPACT, L"Data\\Skill\\", L"ArrowImpact");
    gLoadData.AccessModel(MODEL_SWORD_FORCE, L"Data\\Skill\\", L"SwordForce");

    gLoadData.AccessModel(MODEL_FLY_BIG_STONE1, L"Data\\Skill\\", L"FlyBigStone1");
    gLoadData.AccessModel(MODEL_FLY_BIG_STONE2, L"Data\\Skill\\", L"FlyBigStone2");
    gLoadData.AccessModel(MODEL_BIG_STONE_PART1, L"Data\\Skill\\", L"FlySmallStone1");
    gLoadData.AccessModel(MODEL_BIG_STONE_PART2, L"Data\\Skill\\", L"FlySmallStone2");
    gLoadData.AccessModel(MODEL_WALL_PART1, L"Data\\Skill\\", L"WallStone1");
    gLoadData.AccessModel(MODEL_WALL_PART2, L"Data\\Skill\\", L"WallStone2");
    gLoadData.AccessModel(MODEL_GATE_PART1, L"Data\\Skill\\", L"GatePart1");
    gLoadData.AccessModel(MODEL_GATE_PART2, L"Data\\Skill\\", L"GatePart2");
    gLoadData.AccessModel(MODEL_GATE_PART3, L"Data\\Skill\\", L"GatePart3");
    gLoadData.AccessModel(MODEL_AURORA, L"Data\\Skill\\", L"Aurora");
    gLoadData.AccessModel(MODEL_TOWER_GATE_PLANE, L"Data\\Skill\\", L"TowerGatePlane");
    gLoadData.AccessModel(MODEL_GOLEM_STONE, L"Data\\Skill\\", L"golem_stone");
    gLoadData.AccessModel(MODEL_FISSURE, L"Data\\Skill\\", L"bossrock");
    gLoadData.AccessModel(MODEL_FISSURE_LIGHT, L"Data\\Skill\\", L"bossrocklight");
    gLoadData.AccessModel(MODEL_PROTECTGUILD, L"Data\\Skill\\", L"ProtectGuild");
    gLoadData.AccessModel(MODEL_DARK_ELF_SKILL, L"Data\\Skill\\", L"elf_skill");
    gLoadData.AccessModel(MODEL_BALGAS_SKILL, L"Data\\Skill\\", L"WaveForce");
    gLoadData.AccessModel(MODEL_DEATH_SPI_SKILL, L"Data\\Skill\\", L"deathsp_eff");
    Models[MODEL_BALGAS_SKILL].Actions[0].Loop = false;

    gLoadData.OpenTexture(MODEL_DARK_ELF_SKILL, L"Skill\\");
    gLoadData.OpenTexture(MODEL_BALGAS_SKILL, L"Skill\\");
    gLoadData.OpenTexture(MODEL_DEATH_SPI_SKILL, L"Skill\\");

    gLoadData.OpenTexture(MODEL_DARK_HORSE, L"Skill\\");
    gLoadData.OpenTexture(MODEL_DARK_SPIRIT, L"Skill\\");

    gLoadData.OpenTexture(MODEL_WARCRAFT, L"Skill\\");
    gLoadData.OpenTexture(MODEL_PEGASUS, L"Skill\\");
    gLoadData.OpenTexture(MODEL_SKILL_FURY_STRIKE + 1, L"Skill\\");
    gLoadData.OpenTexture(MODEL_SKILL_FURY_STRIKE + 2, L"Skill\\");
    gLoadData.OpenTexture(MODEL_SKILL_FURY_STRIKE + 3, L"Skill\\");
    gLoadData.OpenTexture(MODEL_SKILL_FURY_STRIKE + 5, L"Skill\\");
    gLoadData.OpenTexture(MODEL_SKILL_FURY_STRIKE + 7, L"Skill\\");
    gLoadData.OpenTexture(MODEL_SKILL_FURY_STRIKE + 8, L"Skill\\");
    gLoadData.OpenTexture(MODEL_WAVE, L"Skill\\");
    gLoadData.OpenTexture(MODEL_TAIL, L"Skill\\");

    gLoadData.OpenTexture(MODEL_WAVE_FORCE, L"Skill\\");
    gLoadData.OpenTexture(MODEL_BLIZZARD, L"Skill\\");

    g_XmasEvent->LoadXmasEvent();

    gLoadData.OpenTexture(MODEL_ARROW_BEST_CROSSBOW, L"Skill\\");
    gLoadData.OpenTexture(MODEL_ARROW_DRILL, L"Skill\\");
    gLoadData.OpenTexture(MODEL_COMBO, L"Skill\\");

    gLoadData.OpenTexture(MODEL_AIR_FORCE, L"Skill\\", GL_CLAMP_TO_EDGE);
    gLoadData.OpenTexture(MODEL_WAVES, L"Skill\\");
    gLoadData.OpenTexture(MODEL_PIERCING2, L"Skill\\");
    gLoadData.OpenTexture(MODEL_PIER_PART, L"Skill\\");
    gLoadData.OpenTexture(MODEL_GROUND_STONE, L"Skill\\");
    gLoadData.OpenTexture(MODEL_GROUND_STONE2, L"Skill\\");
    gLoadData.OpenTexture(MODEL_WATER_WAVE, L"Skill\\", GL_CLAMP_TO_EDGE);
    gLoadData.OpenTexture(MODEL_SKULL, L"Skill\\", GL_CLAMP_TO_EDGE);
    gLoadData.OpenTexture(MODEL_MANY_FLAG, L"Skill\\");
    gLoadData.OpenTexture(MODEL_WEBZEN_MARK, L"Skill\\");

    gLoadData.OpenTexture(MODEL_FLY_BIG_STONE1, L"Npc\\");
    gLoadData.OpenTexture(MODEL_FLY_BIG_STONE2, L"Skill\\");
    gLoadData.OpenTexture(MODEL_BIG_STONE_PART1, L"Skill\\");
    gLoadData.OpenTexture(MODEL_BIG_STONE_PART2, L"Skill\\");
    gLoadData.OpenTexture(MODEL_WALL_PART1, L"Object31\\");
    gLoadData.OpenTexture(MODEL_WALL_PART2, L"Object31\\");
    gLoadData.OpenTexture(MODEL_GATE_PART1, L"Monster\\");
    gLoadData.OpenTexture(MODEL_GATE_PART2, L"Monster\\");
    gLoadData.OpenTexture(MODEL_GATE_PART3, L"Monster\\");
    gLoadData.OpenTexture(MODEL_AURORA, L"Monster\\");
    gLoadData.OpenTexture(MODEL_TOWER_GATE_PLANE, L"Skill\\");
    gLoadData.OpenTexture(MODEL_GOLEM_STONE, L"Monster\\");
    gLoadData.OpenTexture(MODEL_FISSURE, L"Skill\\");
    gLoadData.OpenTexture(MODEL_FISSURE_LIGHT, L"Skill\\");
    gLoadData.OpenTexture(MODEL_SKIN_SHELL, L"Effect\\");
    gLoadData.OpenTexture(MODEL_PROTECTGUILD, L"Item\\");

    gLoadData.AccessModel(MODEL_SKILL_FURY_STRIKE + 1, L"Data\\Skill\\", L"EarthQuake", 1);
    gLoadData.AccessModel(MODEL_SKILL_FURY_STRIKE + 2, L"Data\\Skill\\", L"EarthQuake", 2);
    gLoadData.AccessModel(MODEL_SKILL_FURY_STRIKE + 3, L"Data\\Skill\\", L"EarthQuake", 3);
    gLoadData.AccessModel(MODEL_SKILL_FURY_STRIKE + 5, L"Data\\Skill\\", L"EarthQuake", 5);
    gLoadData.AccessModel(MODEL_SKILL_FURY_STRIKE + 7, L"Data\\Skill\\", L"EarthQuake", 7);
    gLoadData.AccessModel(MODEL_SKILL_FURY_STRIKE + 8, L"Data\\Skill\\", L"EarthQuake", 8);
    gLoadData.AccessModel(MODEL_WAVE, L"Data\\Skill\\", L"flashing");
    gLoadData.AccessModel(MODEL_TAIL, L"Data\\Skill\\", L"tail");

    gLoadData.AccessModel(MODEL_SKILL_FURY_STRIKE + 4, L"Data\\Skill\\", L"EarthQuake", 4);
    gLoadData.AccessModel(MODEL_SKILL_FURY_STRIKE + 6, L"Data\\Skill\\", L"EarthQuake", 6);
    gLoadData.AccessModel(MODEL_PIERCING, L"Data\\Skill\\", L"Piercing");

    gLoadData.AccessModel(MODEL_WAVE_FORCE, L"Data\\Skill\\", L"WaveForce");
    gLoadData.AccessModel(MODEL_BLIZZARD, L"Data\\Skill\\", L"Blizzard");

    gLoadData.AccessModel(MODEL_ARROW_AUTOLOAD, L"Data\\Skill\\", L"arrowsrefill");
    gLoadData.OpenTexture(MODEL_ARROW_AUTOLOAD, L"Effect\\");

    gLoadData.AccessModel(MODEL_INFINITY_ARROW, L"Data\\Skill\\", L"arrowsre", 1);
    gLoadData.OpenTexture(MODEL_INFINITY_ARROW, L"Skill\\");
    gLoadData.AccessModel(MODEL_INFINITY_ARROW1, L"Data\\Skill\\", L"arrowsre", 2);
    gLoadData.OpenTexture(MODEL_INFINITY_ARROW1, L"Skill\\");
    gLoadData.AccessModel(MODEL_INFINITY_ARROW2, L"Data\\Skill\\", L"arrowsre", 3);
    gLoadData.OpenTexture(MODEL_INFINITY_ARROW2, L"Skill\\");
    gLoadData.AccessModel(MODEL_INFINITY_ARROW3, L"Data\\Skill\\", L"arrowsre", 4);
    gLoadData.OpenTexture(MODEL_INFINITY_ARROW3, L"Effect\\");
    gLoadData.AccessModel(MODEL_INFINITY_ARROW4, L"Data\\Skill\\", L"arrowsre", 5);
    gLoadData.OpenTexture(MODEL_INFINITY_ARROW4, L"Skill\\");

    gLoadData.AccessModel(MODEL_SHIELD_CRASH, L"Data\\Effect\\", L"atshild");
    gLoadData.OpenTexture(MODEL_SHIELD_CRASH, L"Effect\\");

    gLoadData.AccessModel(MODEL_SHIELD_CRASH2, L"Data\\Effect\\", L"atshild2");
    gLoadData.OpenTexture(MODEL_SHIELD_CRASH2, L"Effect\\");

    gLoadData.AccessModel(MODEL_IRON_RIDER_ARROW, L"Data\\Effect\\", L"ironobj");
    gLoadData.OpenTexture(MODEL_IRON_RIDER_ARROW, L"Effect\\");

    gLoadData.AccessModel(MODEL_KENTAUROS_ARROW, L"Data\\Effect\\", L"cantasarrow");
    gLoadData.OpenTexture(MODEL_KENTAUROS_ARROW, L"Effect\\");

    gLoadData.AccessModel(MODEL_BLADE_SKILL, L"Data\\Effect\\", L"bladetonedo");
    gLoadData.OpenTexture(MODEL_BLADE_SKILL, L"Effect\\");

    gLoadData.AccessModel(MODEL_CHANGE_UP_EFF, L"Data\\Effect\\", L"Change_Up_Eff");
    gLoadData.OpenTexture(MODEL_CHANGE_UP_EFF, L"Effect\\");
    Models[MODEL_CHANGE_UP_EFF].Actions[0].PlaySpeed = 0.005f;
    gLoadData.AccessModel(MODEL_CHANGE_UP_NASA, L"Data\\Effect\\", L"changup_nasa");

    gLoadData.OpenTexture(MODEL_CHANGE_UP_NASA, L"Effect\\");
    gLoadData.AccessModel(MODEL_CHANGE_UP_CYLINDER, L"Data\\Effect\\", L"clinderlight");
    gLoadData.OpenTexture(MODEL_CHANGE_UP_CYLINDER, L"Effect\\");

    gLoadData.AccessModel(MODEL_CURSEDTEMPLE_HOLYITEM, L"Data\\Skill\\", L"eventsungmul");
    gLoadData.OpenTexture(MODEL_CURSEDTEMPLE_HOLYITEM, L"Skill\\");

    gLoadData.AccessModel(MODEL_CURSEDTEMPLE_PRODECTION_SKILL, L"Data\\Skill\\", L"eventshild");
    gLoadData.OpenTexture(MODEL_CURSEDTEMPLE_PRODECTION_SKILL, L"Skill\\");

    gLoadData.AccessModel(MODEL_CURSEDTEMPLE_RESTRAINT_SKILL, L"Data\\Skill\\", L"eventroofe");
    gLoadData.OpenTexture(MODEL_CURSEDTEMPLE_RESTRAINT_SKILL, L"Skill\\");

    gLoadData.AccessModel(MODEL_CURSEDTEMPLE_STATUE_PART1, L"Data\\Npc\\", L"songck1");
    gLoadData.OpenTexture(MODEL_CURSEDTEMPLE_STATUE_PART1, L"Npc\\");

    gLoadData.AccessModel(MODEL_CURSEDTEMPLE_STATUE_PART2, L"Data\\Npc\\", L"songck2");
    gLoadData.OpenTexture(MODEL_CURSEDTEMPLE_STATUE_PART2, L"Npc\\");

    gLoadData.AccessModel(MODEL_FENRIR_THUNDER, L"Data\\Effect\\", L"lightning_type01");
    gLoadData.OpenTexture(MODEL_FENRIR_THUNDER, L"Effect\\");

    for (int i = MODEL_SKILL_BEGIN; i < MODEL_SKILL_END; i++)
    {
        if (i == MODEL_PIERCING)
        {
            gLoadData.OpenTexture(i, L"Skill\\");
        }
        else
            gLoadData.OpenTexture(i, L"Skill\\");
    }

    LoadBitmap(L"Skill\\flower1.tga", BITMAP_FLOWER01);
    LoadBitmap(L"Skill\\flower2.tga", BITMAP_FLOWER01 + 1);
    LoadBitmap(L"Skill\\flower3.tga", BITMAP_FLOWER01 + 2);

    gLoadData.AccessModel(MODEL_MOONHARVEST_GAM, L"Data\\Effect\\", L"chusukgam");
    gLoadData.OpenTexture(MODEL_MOONHARVEST_GAM, L"Effect\\");
    gLoadData.AccessModel(MODEL_MOONHARVEST_SONGPUEN1, L"Data\\Effect\\", L"chusukseung1");
    gLoadData.OpenTexture(MODEL_MOONHARVEST_SONGPUEN1, L"Effect\\");
    gLoadData.AccessModel(MODEL_MOONHARVEST_SONGPUEN2, L"Data\\Effect\\", L"chusukseung2");
    gLoadData.OpenTexture(MODEL_MOONHARVEST_SONGPUEN2, L"Effect\\");
    gLoadData.AccessModel(MODEL_MOONHARVEST_MOON, L"Data\\Effect\\", L"chysukmoon");
    gLoadData.OpenTexture(MODEL_MOONHARVEST_MOON, L"Effect\\");

    gLoadData.AccessModel(MODEL_ALICE_BUFFSKILL_EFFECT, L"Data\\Effect\\", L"elshildring");
    gLoadData.OpenTexture(MODEL_ALICE_BUFFSKILL_EFFECT, L"Effect\\");
    gLoadData.AccessModel(MODEL_ALICE_BUFFSKILL_EFFECT2, L"Data\\Effect\\", L"elshildring2");
    gLoadData.OpenTexture(MODEL_ALICE_BUFFSKILL_EFFECT2, L"Effect\\");

    gLoadData.AccessModel(MODEL_SUMMONER_WRISTRING_EFFECT, L"Data\\Effect\\", L"ringtyperout");
    gLoadData.OpenTexture(MODEL_SUMMONER_WRISTRING_EFFECT, L"Effect\\");

    gLoadData.AccessModel(MODEL_SUMMONER_EQUIP_HEAD_SAHAMUTT, L"Data\\Skill\\", L"sahatail");
    gLoadData.OpenTexture(MODEL_SUMMONER_EQUIP_HEAD_SAHAMUTT, L"Skill\\");
    gLoadData.AccessModel(MODEL_SUMMONER_EQUIP_HEAD_NEIL, L"Data\\Skill\\", L"nillsohwanz");
    gLoadData.OpenTexture(MODEL_SUMMONER_EQUIP_HEAD_NEIL, L"Skill\\");

    gLoadData.AccessModel(MODEL_SUMMONER_EQUIP_HEAD_LAGUL, L"Data\\Skill\\", L"lagul_head");
    gLoadData.OpenTexture(MODEL_SUMMONER_EQUIP_HEAD_LAGUL, L"Skill\\");

    gLoadData.AccessModel(MODEL_SUMMONER_CASTING_EFFECT1, L"Data\\Effect\\", L"Suhwanzin1");
    gLoadData.OpenTexture(MODEL_SUMMONER_CASTING_EFFECT1, L"Effect\\");
    gLoadData.AccessModel(MODEL_SUMMONER_CASTING_EFFECT11, L"Data\\Effect\\", L"Suhwanzin11");
    gLoadData.OpenTexture(MODEL_SUMMONER_CASTING_EFFECT11, L"Effect\\");
    gLoadData.AccessModel(MODEL_SUMMONER_CASTING_EFFECT111, L"Data\\Effect\\", L"Suhwanzin111");
    gLoadData.OpenTexture(MODEL_SUMMONER_CASTING_EFFECT111, L"Effect\\");
    gLoadData.AccessModel(MODEL_SUMMONER_CASTING_EFFECT2, L"Data\\Effect\\", L"Suhwanzin2");
    gLoadData.OpenTexture(MODEL_SUMMONER_CASTING_EFFECT2, L"Effect\\");
    gLoadData.AccessModel(MODEL_SUMMONER_CASTING_EFFECT22, L"Data\\Effect\\", L"Suhwanzin22");
    gLoadData.OpenTexture(MODEL_SUMMONER_CASTING_EFFECT22, L"Effect\\");
    gLoadData.AccessModel(MODEL_SUMMONER_CASTING_EFFECT222, L"Data\\Effect\\", L"Suhwanzin222");
    gLoadData.OpenTexture(MODEL_SUMMONER_CASTING_EFFECT222, L"Effect\\");
    gLoadData.AccessModel(MODEL_SUMMONER_CASTING_EFFECT4, L"Data\\Effect\\", L"Suhwanzin4");
    gLoadData.OpenTexture(MODEL_SUMMONER_CASTING_EFFECT4, L"Effect\\");
    gLoadData.AccessModel(MODEL_SUMMONER_SUMMON_SAHAMUTT, L"Data\\Skill\\", L"summon_sahamutt");
    gLoadData.OpenTexture(MODEL_SUMMONER_SUMMON_SAHAMUTT, L"Skill\\");
    gLoadData.AccessModel(MODEL_SUMMONER_SUMMON_NEIL, L"Data\\Skill\\", L"summon_neil");
    gLoadData.OpenTexture(MODEL_SUMMONER_SUMMON_NEIL, L"Skill\\");
    gLoadData.OpenTexture(MODEL_SUMMONER_SUMMON_NEIL, L"Effect\\");
    gLoadData.AccessModel(MODEL_SUMMONER_SUMMON_LAGUL, L"Data\\Skill\\", L"summon_lagul");
    gLoadData.OpenTexture(MODEL_SUMMONER_SUMMON_LAGUL, L"Skill\\");

    gLoadData.AccessModel(MODEL_SUMMONER_SUMMON_NEIL_NIFE1, L"Data\\Skill\\", L"nelleff_nife01");
    gLoadData.OpenTexture(MODEL_SUMMONER_SUMMON_NEIL_NIFE1, L"Skill\\");
    gLoadData.AccessModel(MODEL_SUMMONER_SUMMON_NEIL_NIFE2, L"Data\\Skill\\", L"nelleff_nife02");
    gLoadData.OpenTexture(MODEL_SUMMONER_SUMMON_NEIL_NIFE2, L"Skill\\");
    gLoadData.AccessModel(MODEL_SUMMONER_SUMMON_NEIL_NIFE3, L"Data\\Skill\\", L"nelleff_nife03");
    gLoadData.OpenTexture(MODEL_SUMMONER_SUMMON_NEIL_NIFE3, L"Skill\\");
    gLoadData.AccessModel(MODEL_SUMMONER_SUMMON_NEIL_GROUND1, L"Data\\Skill\\", L"nell_nifegrund01");
    gLoadData.OpenTexture(MODEL_SUMMONER_SUMMON_NEIL_GROUND1, L"Skill\\");
    gLoadData.AccessModel(MODEL_SUMMONER_SUMMON_NEIL_GROUND2, L"Data\\Skill\\", L"nell_nifegrund02");
    gLoadData.OpenTexture(MODEL_SUMMONER_SUMMON_NEIL_GROUND2, L"Skill\\");
    gLoadData.AccessModel(MODEL_SUMMONER_SUMMON_NEIL_GROUND3, L"Data\\Skill\\", L"nell_nifegrund03");
    gLoadData.OpenTexture(MODEL_SUMMONER_SUMMON_NEIL_GROUND3, L"Skill\\");
    gLoadData.AccessModel(MODEL_MOVE_TARGETPOSITION_EFFECT, L"Data\\Effect\\", L"MoveTargetPosEffect");
    gLoadData.OpenTexture(MODEL_MOVE_TARGETPOSITION_EFFECT, L"Effect\\");
    gLoadData.AccessModel(MODEL_EFFECT_SAPITRES_ATTACK_1, L"Data\\Effect\\", L"Sapiatttres");
    gLoadData.OpenTexture(MODEL_EFFECT_SAPITRES_ATTACK_1, L"Effect\\");
    gLoadData.AccessModel(MODEL_EFFECT_SAPITRES_ATTACK_2, L"Data\\Effect\\", L"Sapiatttres2");
    gLoadData.OpenTexture(MODEL_EFFECT_SAPITRES_ATTACK_2, L"Effect\\");

    gLoadData.AccessModel(MODEL_RAKLION_BOSS_CRACKEFFECT, L"Data\\Effect\\", L"knight_plancrack_grand");
    gLoadData.OpenTexture(MODEL_RAKLION_BOSS_CRACKEFFECT, L"Effect\\");
    gLoadData.AccessModel(MODEL_RAKLION_BOSS_MAGIC, L"Data\\Effect\\", L"serufan_magic");
    gLoadData.OpenTexture(MODEL_RAKLION_BOSS_MAGIC, L"Effect\\");
    //Models[MODEL_RAKLION_BOSS_MAGIC].Actions[0].PlaySpeed = 0.005f;

    gLoadData.AccessModel(MODEL_EFFECT_SKURA_ITEM, L"Data\\Effect\\cherryblossom\\", L"Skura_iteam_event");

    gLoadData.AccessModel(MODEL_EFFECT_BROKEN_ICE0, L"Data\\Effect\\", L"ice_stone00");
    gLoadData.OpenTexture(MODEL_EFFECT_BROKEN_ICE0, L"Effect\\");
    gLoadData.AccessModel(MODEL_EFFECT_BROKEN_ICE1, L"Data\\Effect\\", L"ice_stone01");
    gLoadData.OpenTexture(MODEL_EFFECT_BROKEN_ICE1, L"Effect\\");
    gLoadData.AccessModel(MODEL_EFFECT_BROKEN_ICE2, L"Data\\Effect\\", L"ice_stone02");
    gLoadData.OpenTexture(MODEL_EFFECT_BROKEN_ICE2, L"Effect\\");
    gLoadData.AccessModel(MODEL_EFFECT_BROKEN_ICE3, L"Data\\Effect\\", L"ice_stone03");
    gLoadData.OpenTexture(MODEL_EFFECT_BROKEN_ICE3, L"Effect\\");
    gLoadData.AccessModel(MODEL_NIGHTWATER_01, L"Data\\Effect\\", L"nightwater01");
    gLoadData.OpenTexture(MODEL_NIGHTWATER_01, L"Effect\\");
    gLoadData.AccessModel(MODEL_KNIGHT_PLANCRACK_A, L"Data\\Effect\\", L"knight_plancrack_a");
    gLoadData.OpenTexture(MODEL_KNIGHT_PLANCRACK_A, L"Effect\\");
    Models[MODEL_KNIGHT_PLANCRACK_A].Actions[0].PlaySpeed = 0.3f;
    gLoadData.AccessModel(MODEL_KNIGHT_PLANCRACK_B, L"Data\\Effect\\", L"knight_plancrack_b");
    gLoadData.OpenTexture(MODEL_KNIGHT_PLANCRACK_B, L"Effect\\");
    Models[MODEL_KNIGHT_PLANCRACK_B].Actions[0].PlaySpeed = 0.3f;
    gLoadData.AccessModel(MODEL_EFFECT_FLAME_STRIKE, L"Data\\Effect\\", L"FlameStrike");
    gLoadData.OpenTexture(MODEL_EFFECT_FLAME_STRIKE, L"Effect\\");
    gLoadData.AccessModel(MODEL_SWELL_OF_MAGICPOWER, L"Data\\Effect\\", L"magic_powerup");
    gLoadData.OpenTexture(MODEL_SWELL_OF_MAGICPOWER, L"Effect\\");
    gLoadData.AccessModel(MODEL_ARROWSRE06, L"Data\\Effect\\", L"arrowsre06");
    gLoadData.OpenTexture(MODEL_ARROWSRE06, L"Effect\\");
    gLoadData.AccessModel(MODEL_DOPPELGANGER_SLIME_CHIP, L"Data\\Effect\\", L"slime_chip");
    gLoadData.OpenTexture(MODEL_DOPPELGANGER_SLIME_CHIP, L"Effect\\");
    gLoadData.AccessModel(MODEL_EFFECT_UMBRELLA_GOLD, L"Data\\Effect\\", L"japan_gold01");
    gLoadData.OpenTexture(MODEL_EFFECT_UMBRELLA_GOLD, L"Effect\\");
    gLoadData.AccessModel(MODEL_EMPIREGUARDIANBOSS_FRAMESTRIKE, L"Data\\Effect\\", L"Karanebos_sword_framestrike");
    gLoadData.OpenTexture(MODEL_EMPIREGUARDIANBOSS_FRAMESTRIKE, L"Effect\\");
    //Models[MODEL_EMPIREGUARDIANBOSS_FRAMESTRIKE].Actions[MONSTER01_STOP1].PlaySpeed = 3.0f;
    gLoadData.AccessModel(MODEL_DEASULER, L"Data\\Monster\\", L"deasther_boomerang");
    gLoadData.OpenTexture(MODEL_DEASULER, L"Monster\\");
    gLoadData.AccessModel(MODEL_EFFECT_SD_AURA, L"Data\\Effect\\", L"shield_up");
    gLoadData.OpenTexture(MODEL_EFFECT_SD_AURA, L"Effect\\");
    gLoadData.AccessModel(MODEL_WOLF_HEAD_EFFECT, L"Data\\Effect\\", L"wolf_head_effect");
    gLoadData.OpenTexture(MODEL_WOLF_HEAD_EFFECT, L"Effect\\");
    LoadBitmap(L"Effect\\sbumb.jpg", BITMAP_SBUMB, GL_LINEAR, GL_REPEAT);
    gLoadData.AccessModel(MODEL_DOWN_ATTACK_DUMMY_L, L"Data\\Effect\\", L"down_right_punch");
    gLoadData.OpenTexture(MODEL_DOWN_ATTACK_DUMMY_L, L"Effect\\");
    gLoadData.AccessModel(MODEL_DOWN_ATTACK_DUMMY_R, L"Data\\Effect\\", L"down_left_punch");
    gLoadData.OpenTexture(MODEL_DOWN_ATTACK_DUMMY_R, L"Effect\\");
    gLoadData.AccessModel(MODEL_SHOCKWAVE01, L"Data\\Effect\\", L"shockwave01");
    gLoadData.OpenTexture(MODEL_SHOCKWAVE01, L"Effect\\");
    gLoadData.AccessModel(MODEL_SHOCKWAVE02, L"Data\\Effect\\", L"shockwave02");
    gLoadData.OpenTexture(MODEL_SHOCKWAVE02, L"Effect\\");
    gLoadData.AccessModel(MODEL_SHOCKWAVE_SPIN01, L"Data\\Effect\\", L"shockwave_spin01");
    gLoadData.OpenTexture(MODEL_SHOCKWAVE_SPIN01, L"Effect\\");
    gLoadData.AccessModel(MODEL_WINDFOCE, L"Data\\Effect\\", L"wind_foce");
    gLoadData.OpenTexture(MODEL_WINDFOCE, L"Effect\\");
    gLoadData.AccessModel(MODEL_WINDFOCE_MIRROR, L"Data\\Effect\\", L"wind_foce_mirror");
    gLoadData.OpenTexture(MODEL_WINDFOCE_MIRROR, L"Effect\\");
    gLoadData.AccessModel(MODEL_WOLF_HEAD_EFFECT2, L"Data\\Effect\\", L"wolf_head_effect2");
    gLoadData.OpenTexture(MODEL_WOLF_HEAD_EFFECT2, L"skill\\");
    gLoadData.AccessModel(MODEL_SHOCKWAVE_GROUND01, L"Data\\Effect\\", L"shockwave_ground01");
    gLoadData.OpenTexture(MODEL_SHOCKWAVE_GROUND01, L"Effect\\");
    gLoadData.AccessModel(MODEL_DRAGON_KICK_DUMMY, L"Data\\Effect\\", L"dragon_kick_dummy");
    gLoadData.OpenTexture(MODEL_DRAGON_KICK_DUMMY, L"Effect\\");
    gLoadData.AccessModel(MODEL_DRAGON_LOWER_DUMMY, L"Data\\Effect\\", L"knight_plancrack_dragon");
    gLoadData.OpenTexture(MODEL_DRAGON_LOWER_DUMMY, L"Effect\\");
    gLoadData.AccessModel(MODEL_VOLCANO_OF_MONK, L"Data\\Effect\\", L"volcano_of_monk");
    gLoadData.OpenTexture(MODEL_VOLCANO_OF_MONK, L"Effect\\");
    gLoadData.AccessModel(MODEL_VOLCANO_STONE, L"Data\\Effect\\", L"volcano_stone");
    gLoadData.OpenTexture(MODEL_VOLCANO_STONE, L"Effect\\");
    LoadBitmap(L"Effect\\force_Pillar.jpg", BITMAP_FORCEPILLAR, GL_LINEAR, GL_REPEAT);
    LoadBitmap(L"Effect\\!SwordEff.jpg", BITMAP_SWORDEFF, GL_LINEAR, GL_REPEAT);
    LoadBitmap(L"Effect\\Damage1.jpg", BITMAP_DAMAGE1, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\ground_wind.jpg", BITMAP_GROUND_WIND, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\Kwave2.jpg", BITMAP_KWAVE2, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\Damage2.jpg", BITMAP_DAMAGE2, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\volcano_core.jpg", BITMAP_VOLCANO_CORE, GL_LINEAR, GL_CLAMP_TO_EDGE);
    gLoadData.AccessModel(MODEL_SHOCKWAVE03, L"Data\\Effect\\", L"shockwave03");
    gLoadData.OpenTexture(MODEL_SHOCKWAVE03, L"Effect\\");
    LoadBitmap(L"Effect\\ground_smoke.tga", BITMAP_GROUND_SMOKE, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\knightSt_blue.jpg", BITMAP_KNIGHTST_BLUE, GL_LINEAR, GL_CLAMP_TO_EDGE);

    gLoadData.AccessModel(MODEL_PHOENIX_SHOT, L"Data\\Effect\\", L"phoenix_shot_effect");
    gLoadData.OpenTexture(MODEL_PHOENIX_SHOT, L"Effect\\");
    gLoadData.AccessModel(MODEL_WINDSPIN01, L"Data\\Effect\\", L"wind_spin01");
    gLoadData.OpenTexture(MODEL_WINDSPIN01, L"Effect\\");
    gLoadData.AccessModel(MODEL_WINDSPIN02, L"Data\\Effect\\", L"wind_spin02");
    gLoadData.OpenTexture(MODEL_WINDSPIN02, L"Effect\\");
    gLoadData.AccessModel(MODEL_WINDSPIN03, L"Data\\Effect\\", L"wind_spin03");
    gLoadData.OpenTexture(MODEL_WINDSPIN03, L"Effect\\");

    gLoadData.AccessModel(MODEL_SWORD_35_WING, L"Data\\Item\\", L"sword36wing");
    gLoadData.OpenTexture(MODEL_SWORD_35_WING, L"Item\\");

#ifdef ASG_ADD_KARUTAN_MONSTERS
    // 콘드라 돌조각
    gLoadData.AccessModel(MODEL_CONDRA_STONE, L"Data\\Monster\\", L"condra_7_stone");
    gLoadData.OpenTexture(MODEL_CONDRA_STONE, L"Monster\\");
    gLoadData.AccessModel(MODEL_CONDRA_STONE1, L"Data\\Monster\\", L"condra_7_stone_2");
    gLoadData.OpenTexture(MODEL_CONDRA_STONE1, L"Monster\\");
    gLoadData.AccessModel(MODEL_CONDRA_STONE2, L"Data\\Monster\\", L"condra_7_stone_3");
    gLoadData.OpenTexture(MODEL_CONDRA_STONE2, L"Monster\\");
    gLoadData.AccessModel(MODEL_CONDRA_STONE3, L"Data\\Monster\\", L"condra_7_stone_4");
    gLoadData.OpenTexture(MODEL_CONDRA_STONE3, L"Monster\\");
    gLoadData.AccessModel(MODEL_CONDRA_STONE4, L"Data\\Monster\\", L"condra_7_stone_5");
    gLoadData.OpenTexture(MODEL_CONDRA_STONE4, L"Monster\\");
    gLoadData.AccessModel(MODEL_CONDRA_STONE5, L"Data\\Monster\\", L"condra_7_stone_6");
    gLoadData.OpenTexture(MODEL_CONDRA_STONE5, L"Monster\\");

    gLoadData.AccessModel(MODEL_NARCONDRA_STONE, L"Data\\Monster\\", L"nar_condra_7_stone_1");
    gLoadData.OpenTexture(MODEL_NARCONDRA_STONE, L"Monster\\");
    gLoadData.AccessModel(MODEL_NARCONDRA_STONE1, L"Data\\Monster\\", L"nar_condra_7_stone_2");
    gLoadData.OpenTexture(MODEL_NARCONDRA_STONE1, L"Monster\\");
    gLoadData.AccessModel(MODEL_NARCONDRA_STONE2, L"Data\\Monster\\", L"nar_condra_7_stone_3");
    gLoadData.OpenTexture(MODEL_NARCONDRA_STONE2, L"Monster\\");
    gLoadData.AccessModel(MODEL_NARCONDRA_STONE3, L"Data\\Monster\\", L"nar_condra_7_stone_4");
    gLoadData.OpenTexture(MODEL_NARCONDRA_STONE3, L"Monster\\");
#endif	// ASG_ADD_KARUTAN_MONSTERS
}

#include "ReadScript.h"

void SaveWorld(int World)
{
    wchar_t WorldName[32];
    wchar_t FileName[64];
    swprintf(WorldName, L"World%d", World);

    swprintf(FileName, L"Data2\\%s\\TerrainLight.jpg", WorldName);
    SaveTerrainLight(FileName);
    swprintf(FileName, L"Data2\\%s\\TerrainHeight.bmp", WorldName);
    SaveTerrainHeight(FileName);
    swprintf(FileName, L"Data\\%s\\Terrain.map", WorldName);
    SaveTerrainMapping(FileName, World);
    swprintf(FileName, L"Data\\%s\\Terrain.att", WorldName);
    SaveTerrainAttribute(FileName, World);
    swprintf(FileName, L"Data\\%s\\Terrain.obj", WorldName);
    SaveObjects(FileName, World);
}

void OpenImages()
{
    LoadBitmap(L"Interface\\command.jpg", BITMAP_SKILL_INTERFACE + 2);
    LoadBitmap(L"Interface\\Item_Back01.jpg", BITMAP_INVENTORY);
    LoadBitmap(L"Interface\\Item_Money.jpg", BITMAP_INVENTORY + 11);
    LoadBitmap(L"Interface\\Item_box.jpg", BITMAP_INVENTORY + 17);
    LoadBitmap(L"Interface\\InventoryBox2.jpg", BITMAP_INVENTORY + 18);
    LoadBitmap(L"Interface\\Trading_line.jpg", BITMAP_INVENTORY + 19);
    LoadBitmap(L"Interface\\exit_01.jpg", BITMAP_INVENTORY_BUTTON);
    LoadBitmap(L"Interface\\exit_02.jpg", BITMAP_INVENTORY_BUTTON + 1);
    LoadBitmap(L"Interface\\accept_box01.jpg", BITMAP_INVENTORY_BUTTON + 10);
    LoadBitmap(L"Interface\\accept_box02.jpg", BITMAP_INVENTORY_BUTTON + 11);
    LoadBitmap(L"Interface\\mix_button1.jpg", BITMAP_INVENTORY_BUTTON + 12);
    LoadBitmap(L"Interface\\mix_button2.jpg", BITMAP_INVENTORY_BUTTON + 13);
    LoadBitmap(L"Interface\\lock_01.jpg", BITMAP_INVENTORY_BUTTON + 14);
    LoadBitmap(L"Interface\\lock_02.jpg", BITMAP_INVENTORY_BUTTON + 15);
    LoadBitmap(L"Interface\\lock_03.jpg", BITMAP_INVENTORY_BUTTON + 16);
    LoadBitmap(L"Interface\\lock_04.jpg", BITMAP_INVENTORY_BUTTON + 17);
    LoadBitmap(L"Interface\\guild.tga", BITMAP_GUILD);
}

void OpenSounds()
{
    bool Enable3DSound = true;

    LoadWaveFile(SOUND_WIND01, L"Data\\Sound\\aWind.wav", 1);
    LoadWaveFile(SOUND_RAIN01, L"Data\\Sound\\aRain.wav", 1);
    LoadWaveFile(SOUND_DUNGEON01, L"Data\\Sound\\aDungeon.wav", 1);
    LoadWaveFile(SOUND_FOREST01, L"Data\\Sound\\aForest.wav", 1);
    LoadWaveFile(SOUND_TOWER01, L"Data\\Sound\\aTower.wav", 1);
    LoadWaveFile(SOUND_WATER01, L"Data\\Sound\\aWater.wav", 1);
    LoadWaveFile(SOUND_DESERT01, L"Data\\Sound\\desert.wav", 1);
    //LoadWaveFile(SOUND_BOSS01		    ,"Data\\Sound\\a쿤둔.wav",1);
    LoadWaveFile(SOUND_HUMAN_WALK_GROUND, L"Data\\Sound\\pWalk(Soil).wav", 2);
    LoadWaveFile(SOUND_HUMAN_WALK_GRASS, L"Data\\Sound\\pWalk(Grass).wav", 2);
    LoadWaveFile(SOUND_HUMAN_WALK_SNOW, L"Data\\Sound\\pWalk(Snow).wav", 2);
    LoadWaveFile(SOUND_HUMAN_WALK_SWIM, L"Data\\Sound\\pSwim.wav", 2);

    LoadWaveFile(SOUND_BIRD01, L"Data\\Sound\\aBird1.wav", 1, Enable3DSound);
    LoadWaveFile(SOUND_BIRD02, L"Data\\Sound\\aBird2.wav", 1, Enable3DSound);
    LoadWaveFile(SOUND_BAT01, L"Data\\Sound\\aBat.wav", 1, Enable3DSound);
    LoadWaveFile(SOUND_RAT01, L"Data\\Sound\\aMouse.wav", 1, Enable3DSound);
    LoadWaveFile(SOUND_TRAP01, L"Data\\Sound\\aGrate.wav", 1, Enable3DSound);
    LoadWaveFile(SOUND_DOOR01, L"Data\\Sound\\aDoor.wav", 1);
    LoadWaveFile(SOUND_DOOR02, L"Data\\Sound\\aCastleDoor.wav", 1);

    LoadWaveFile(SOUND_HEAVEN01, L"Data\\Sound\\aHeaven.wav", 1);
    LoadWaveFile(SOUND_THUNDERS01, L"Data\\Sound\\aThunder01.wav", 1);
    LoadWaveFile(SOUND_THUNDERS02, L"Data\\Sound\\aThunder02.wav", 1);
    LoadWaveFile(SOUND_THUNDERS03, L"Data\\Sound\\aThunder03.wav", 1);

    //attack
    LoadWaveFile(SOUND_BRANDISH_SWORD01, L"Data\\Sound\\eSwingWeapon1.wav", 2);
    LoadWaveFile(SOUND_BRANDISH_SWORD02, L"Data\\Sound\\eSwingWeapon2.wav", 2);
    LoadWaveFile(SOUND_BRANDISH_SWORD03, L"Data\\Sound\\eSwingLightSword.wav", 2);
    LoadWaveFile(SOUND_BOW01, L"Data\\Sound\\eBow.wav", 2);
    LoadWaveFile(SOUND_CROSSBOW01, L"Data\\Sound\\eCrossbow.wav", 2);
    LoadWaveFile(SOUND_MIX01, L"Data\\Sound\\eMix.wav", 2);

    //player
    LoadWaveFile(SOUND_DRINK01, L"Data\\Sound\\pDrink.wav", 1);
    LoadWaveFile(SOUND_EAT_APPLE01, L"Data\\Sound\\pEatApple.wav", 1);
    LoadWaveFile(SOUND_HEART, L"Data\\Sound\\pHeartBeat.wav", 1);
    LoadWaveFile(SOUND_GET_ENERGY, L"Data\\Sound\\pEnergy.wav", 1);
    LoadWaveFile(SOUND_HUMAN_SCREAM01, L"Data\\Sound\\pMaleScream1.wav", 2);
    LoadWaveFile(SOUND_HUMAN_SCREAM02, L"Data\\Sound\\pMaleScream2.wav", 2);
    LoadWaveFile(SOUND_HUMAN_SCREAM03, L"Data\\Sound\\pMaleScream3.wav", 2);
    LoadWaveFile(SOUND_HUMAN_SCREAM04, L"Data\\Sound\\pMaleDie.wav", 2);
    LoadWaveFile(SOUND_FEMALE_SCREAM01, L"Data\\Sound\\pFemaleScream1.wav", 2);
    LoadWaveFile(SOUND_FEMALE_SCREAM02, L"Data\\Sound\\pFemaleScream2.wav", 2);

    LoadWaveFile(SOUND_DROP_ITEM01, L"Data\\Sound\\pDropItem.wav", 1);
    LoadWaveFile(SOUND_DROP_GOLD01, L"Data\\Sound\\pDropMoney.wav", 1);
    LoadWaveFile(SOUND_JEWEL01, L"Data\\Sound\\eGem.wav", 1);
    LoadWaveFile(SOUND_GET_ITEM01, L"Data\\Sound\\pGetItem.wav", 1);
    //LoadWaveFile(SOUND_SHOUT01    		,"Data\\Sound\\p기합.wav",1);

    //skill
    LoadWaveFile(SOUND_SKILL_DEFENSE, L"Data\\Sound\\sKnightDefense.wav", 1);
    LoadWaveFile(SOUND_SKILL_SWORD1, L"Data\\Sound\\sKnightSkill1.wav", 1);
    LoadWaveFile(SOUND_SKILL_SWORD2, L"Data\\Sound\\sKnightSkill2.wav", 1);
    LoadWaveFile(SOUND_SKILL_SWORD3, L"Data\\Sound\\sKnightSkill3.wav", 1);
    LoadWaveFile(SOUND_SKILL_SWORD4, L"Data\\Sound\\sKnightSkill4.wav", 1);
    LoadWaveFile(SOUND_MONSTER_SHADOWATTACK2, L"Data\\Sound\\mShadowAttack1.wav", 1);

    LoadWaveFile(SOUND_STORM, L"Data\\Sound\\sTornado.wav", 2, Enable3DSound);
    LoadWaveFile(SOUND_EVIL, L"Data\\Sound\\sEvil.wav", 2, Enable3DSound);
    LoadWaveFile(SOUND_MAGIC, L"Data\\Sound\\sMagic.wav", 2, Enable3DSound);
    LoadWaveFile(SOUND_HELLFIRE, L"Data\\Sound\\sHellFire.wav", 2, Enable3DSound);
    LoadWaveFile(SOUND_ICE, L"Data\\Sound\\sIce.wav", 2, Enable3DSound);
    LoadWaveFile(SOUND_FLAME, L"Data\\Sound\\sFlame.wav", 2, Enable3DSound);
    //LoadWaveFile(SOUND_FLASH            ,"Data\\Sound\\m히드라공격1.wav",2,Enable3DSound);
    LoadWaveFile(SOUND_FLASH, L"Data\\Sound\\sAquaFlash.wav", 2, Enable3DSound);

    LoadWaveFile(SOUND_BREAK01, L"Data\\Sound\\eBreak.wav", 1, Enable3DSound);
    LoadWaveFile(SOUND_EXPLOTION01, L"Data\\Sound\\eExplosion.wav", 1, Enable3DSound);
    LoadWaveFile(SOUND_METEORITE01, L"Data\\Sound\\eMeteorite.wav", 2, Enable3DSound);
    //LoadWaveFile(SOUND_METEORITE02	    ,"Data\\Sound\\e유성.wav",2,Enable3DSound);
    LoadWaveFile(SOUND_THUNDER01, L"Data\\Sound\\eThunder.wav", 1, Enable3DSound);

    LoadWaveFile(SOUND_BONE1, L"Data\\Sound\\mBone1.wav", 2, Enable3DSound);
    LoadWaveFile(SOUND_BONE2, L"Data\\Sound\\mBone2.wav", 2, Enable3DSound);
    LoadWaveFile(SOUND_ASSASSIN, L"Data\\Sound\\mAssassin1.wav", 1, Enable3DSound);

    LoadWaveFile(SOUND_ATTACK_MELEE_HIT1, L"Data\\Sound\\eMeleeHit1.wav", 2);
    LoadWaveFile(SOUND_ATTACK_MELEE_HIT2, L"Data\\Sound\\eMeleeHit2.wav", 2);
    LoadWaveFile(SOUND_ATTACK_MELEE_HIT3, L"Data\\Sound\\eMeleeHit3.wav", 2);
    LoadWaveFile(SOUND_ATTACK_MELEE_HIT4, L"Data\\Sound\\eMeleeHit4.wav", 2);
    LoadWaveFile(SOUND_ATTACK_MELEE_HIT5, L"Data\\Sound\\eMeleeHit5.wav", 2);
    LoadWaveFile(SOUND_ATTACK_MISSILE_HIT1, L"Data\\Sound\\eMissileHit1.wav", 2);
    LoadWaveFile(SOUND_ATTACK_MISSILE_HIT2, L"Data\\Sound\\eMissileHit2.wav", 2);
    LoadWaveFile(SOUND_ATTACK_MISSILE_HIT3, L"Data\\Sound\\eMissileHit3.wav", 2);
    LoadWaveFile(SOUND_ATTACK_MISSILE_HIT4, L"Data\\Sound\\eMissileHit4.wav", 2);

    LoadWaveFile(SOUND_FIRECRACKER1, L"Data\\Sound\\eFirecracker1.wav", 1, Enable3DSound);
    LoadWaveFile(SOUND_FIRECRACKER2, L"Data\\Sound\\eFirecracker2.wav", 1, Enable3DSound);
    LoadWaveFile(SOUND_MEDAL, L"Data\\Sound\\eMedal.wav", 1, Enable3DSound);
    LoadWaveFile(SOUND_PHOENIXEXP, L"Data\\Sound\\ePhoenixExp.wav", 1, Enable3DSound);
    //	LoadWaveFile(SOUND_PHOENIXFIRE		,"Data\\Sound\\ePhoenixFire.wav",1,Enable3DSound);

    LoadWaveFile(SOUND_RIDINGSPEAR, L"Data\\Sound\\eRidingSpear.wav", 1);
    LoadWaveFile(SOUND_RAIDSHOOT, L"Data\\Sound\\eRaidShoot.wav", 1);
    LoadWaveFile(SOUND_SWELLLIFE, L"Data\\Sound\\eSwellLife.wav", 1);
    LoadWaveFile(SOUND_PIERCING, L"Data\\Sound\\ePiercing.wav", 1);
    LoadWaveFile(SOUND_ICEARROW, L"Data\\Sound\\eIceArrow.wav", 1);
    LoadWaveFile(SOUND_TELEKINESIS, L"Data\\Sound\\eTelekinesis.wav", 1);
    LoadWaveFile(SOUND_SOULBARRIER, L"Data\\Sound\\eSoulBarrier.wav", 1);
    LoadWaveFile(SOUND_BLOODATTACK, L"Data\\Sound\\eBloodAttack.wav", 1);

    LoadWaveFile(SOUND_HIT_GATE, L"Data\\Sound\\eHitGate.wav", 1);
    LoadWaveFile(SOUND_HIT_GATE2, L"Data\\Sound\\eHitGate2.wav", 1);
    LoadWaveFile(SOUND_HIT_CRISTAL, L"Data\\Sound\\eHitCristal.wav", 1);
    LoadWaveFile(SOUND_DOWN_GATE, L"Data\\Sound\\eDownGate.wav", 1);
    LoadWaveFile(SOUND_CROW, L"Data\\Sound\\eCrow.wav", 1);

    LoadWaveFile(SOUND_DEATH_POISON1, L"Data\\Sound\\eBlastPoison_1.wav", 2);
    LoadWaveFile(SOUND_DEATH_POISON2, L"Data\\Sound\\eBlastPoison_2.wav", 2);
    LoadWaveFile(SOUND_SUDDEN_ICE1, L"Data\\Sound\\eSuddenIce_1.wav", 2);
    LoadWaveFile(SOUND_SUDDEN_ICE2, L"Data\\Sound\\eSuddenIce_2.wav", 2);
    LoadWaveFile(SOUND_NUKE1, L"Data\\Sound\\eHellFire2_1.wav", 1);
    LoadWaveFile(SOUND_NUKE2, L"Data\\Sound\\eHellFire2_2.wav", 1);
    LoadWaveFile(SOUND_COMBO, L"Data\\Sound\\eCombo.wav", 1);
    LoadWaveFile(SOUND_FURY_STRIKE1, L"Data\\Sound\\eRageBlow_1.wav", 1);
    LoadWaveFile(SOUND_FURY_STRIKE2, L"Data\\Sound\\eRageBlow_2.wav", 1);
    LoadWaveFile(SOUND_FURY_STRIKE3, L"Data\\Sound\\eRageBlow_3.wav", 1);
    LoadWaveFile(SOUND_LEVEL_UP, L"Data\\Sound\\pLevelUp.wav", 1);
    LoadWaveFile(SOUND_CHANGE_UP, L"Data\\Sound\\nMalonSkillMaster.wav", 1);

    LoadWaveFile(SOUND_CHAOS_ENVIR, L"Data\\Sound\\aChaos.wav", 1);
    LoadWaveFile(SOUND_CHAOS_END, L"Data\\Sound\\aChaosEnd.wav", 1);
    LoadWaveFile(SOUND_CHAOS_FALLING, L"Data\\Sound\\pMaleScream.wav", 1);
    LoadWaveFile(SOUND_CHAOS_FALLING_STONE, L"Data\\Sound\\eWallFall.wav", 1);
    LoadWaveFile(SOUND_CHAOS_MOB_BOOM01, L"Data\\Sound\\eMonsterBoom1.wav", 2);
    LoadWaveFile(SOUND_CHAOS_MOB_BOOM02, L"Data\\Sound\\eMonsterBoom2.wav", 2);
    LoadWaveFile(SOUND_CHAOS_THUNDER01, L"Data\\Sound\\eElec1.wav", 1);
    LoadWaveFile(SOUND_CHAOS_THUNDER02, L"Data\\Sound\\eElec2.wav", 1);

    LoadWaveFile(SOUND_RUN_DARK_HORSE_1, L"Data\\Sound\\pHorseStep1.wav", 1);
    LoadWaveFile(SOUND_RUN_DARK_HORSE_2, L"Data\\Sound\\pHorseStep2.wav", 1);
    LoadWaveFile(SOUND_RUN_DARK_HORSE_3, L"Data\\Sound\\pHorseStep3.wav", 1);
    LoadWaveFile(SOUND_DARKLORD_PAIN, L"Data\\Sound\\pDarkPain.wav", 1);
    LoadWaveFile(SOUND_DARKLORD_DEAD, L"Data\\Sound\\pDarkDeath.wav", 1);
    LoadWaveFile(SOUND_ATTACK_SPEAR, L"Data\\Sound\\sDarkSpear.wav", 1);
    LoadWaveFile(SOUND_ATTACK_FIRE_BUST, L"Data\\Sound\\eFirebust.wav", 1);
    LoadWaveFile(SOUND_ATTACK_FIRE_BUST_EXP, L"Data\\Sound\\eFirebustBoom.wav", 1);
    LoadWaveFile(SOUND_PART_TELEPORT, L"Data\\Sound\\eSummon.wav", 1);
    LoadWaveFile(SOUND_ELEC_STRIKE, L"Data\\Sound\\sDarkElecSpike.wav", 1);
    LoadWaveFile(SOUND_ELEC_STRIKE_READY, L"Data\\Sound\\sDarkElecSpikeReady.wav", 1);
    LoadWaveFile(SOUND_EARTH_QUAKE, L"Data\\Sound\\sDarkEarthQuake.wav", 1);
    LoadWaveFile(SOUND_CRITICAL, L"Data\\Sound\\sDarkCritical.wav", 1);
    LoadWaveFile(SOUND_DSPIRIT_MISSILE, L"Data\\Sound\\DSpirit_Missile.wav", 4);
    LoadWaveFile(SOUND_DSPIRIT_SHOUT, L"Data\\Sound\\DSpirit_Shout.wav", 1);
    LoadWaveFile(SOUND_DSPIRIT_RUSH, L"Data\\Sound\\DSpirit_Rush.wav", 3);

    LoadWaveFile(SOUND_FENRIR_RUN_1, L"Data\\Sound\\pW_run-01.wav", 1);
    LoadWaveFile(SOUND_FENRIR_RUN_2, L"Data\\Sound\\pW_run-02.wav", 1);
    LoadWaveFile(SOUND_FENRIR_RUN_3, L"Data\\Sound\\pW_run-03.wav", 1);
    LoadWaveFile(SOUND_FENRIR_WALK_1, L"Data\\Sound\\pW_step-01.wav", 1);
    LoadWaveFile(SOUND_FENRIR_WALK_2, L"Data\\Sound\\pW_step-02.wav", 1);
    LoadWaveFile(SOUND_FENRIR_DEATH, L"Data\\Sound\\pWdeath.wav", 1);
    LoadWaveFile(SOUND_FENRIR_IDLE_1, L"Data\\Sound\\pWidle1.wav", 1);
    LoadWaveFile(SOUND_FENRIR_IDLE_2, L"Data\\Sound\\pWidle2.wav", 1);
    LoadWaveFile(SOUND_FENRIR_DAMAGE_1, L"Data\\Sound\\pWpain1.wav", 1);
    LoadWaveFile(SOUND_FENRIR_DAMAGE_1, L"Data\\Sound\\pWpain2.wav", 1);
    LoadWaveFile(SOUND_FENRIR_SKILL, L"Data\\Sound\\pWskill.wav", 1);
    LoadWaveFile(SOUND_JEWEL02, L"Data\\Sound\\Jewel_Sound.wav", 1);

    LoadWaveFile(SOUND_KUNDUN_ITEM_SOUND, L"Data\\Sound\\kundunitem.wav", 1);

    g_XmasEvent->LoadXmasEventSound();
    g_NewYearsDayEvent->LoadSound();

    LoadWaveFile(SOUND_SHIELDCLASH, L"Data\\Sound\\shieldclash.wav", 1);
    LoadWaveFile(SOUND_INFINITYARROW, L"Data\\Sound\\infinityArrow.wav", 1);
    //SOUND_FIRE_SCREAM
    LoadWaveFile(SOUND_FIRE_SCREAM, L"Data\\Sound\\Darklord_firescream.wav", 1);

    LoadWaveFile(SOUND_MOONRABBIT_WALK, L"Data\\Sound\\SE_Ev_rabbit_walk.wav", 1);
    LoadWaveFile(SOUND_MOONRABBIT_DAMAGE, L"Data\\Sound\\SE_Ev_rabbit_damage.wav", 1);
    LoadWaveFile(SOUND_MOONRABBIT_DEAD, L"Data\\Sound\\SE_Ev_rabbit_death.wav", 1);
    LoadWaveFile(SOUND_MOONRABBIT_EXPLOSION, L"Data\\Sound\\SE_Ev_rabbit_Explosion.wav", 1);

    //	LoadWaveFile(SOUND_SUMMON_CASTING,		"Data\\Sound\\eSummon.wav"	,1);
    LoadWaveFile(SOUND_SUMMON_SAHAMUTT, L"Data\\Sound\\SE_Ch_summoner_skill05_explosion01.wav", 1);
    LoadWaveFile(SOUND_SUMMON_EXPLOSION, L"Data\\Sound\\SE_Ch_summoner_skill05_explosion03.wav", 1);
    LoadWaveFile(SOUND_SUMMON_NEIL, L"Data\\Sound\\SE_Ch_summoner_skill06_requiem01.wav", 1);
    LoadWaveFile(SOUND_SUMMON_REQUIEM, L"Data\\Sound\\SE_Ch_summoner_skill06_requiem02.wav", 1);
    LoadWaveFile(SOUND_SUMMOM_RARGLE, L"Data\\Sound\\Rargle.wav", 1);
    LoadWaveFile(SOUND_SUMMON_SKILL_LIGHTORB, L"Data\\Sound\\SE_Ch_summoner_skill01_lightningof.wav", 1);
    LoadWaveFile(SOUND_SUMMON_SKILL_SLEEP, L"Data\\Sound\\SE_Ch_summoner_skill03_sleep.wav", 1);
    LoadWaveFile(SOUND_SUMMON_SKILL_BLIND, L"Data\\Sound\\SE_Ch_summoner_skill04_blind.wav", 1);
    LoadWaveFile(SOUND_SUMMON_SKILL_THORNS, L"Data\\Sound\\SE_Ch_summoner_skill02_ssonze.wav", 1);
    LoadWaveFile(SOUND_SKILL_CHAIN_LIGHTNING, L"Data\\Sound\\SE_Ch_summoner_skill08_chainlightning.wav", 1);
    LoadWaveFile(SOUND_SKILL_DRAIN_LIFE, L"Data\\Sound\\SE_Ch_summoner_skill07_lifedrain.wav", 1);
    LoadWaveFile(SOUND_SKILL_WEAKNESS, L"Data\\Sound\\SE_Ch_summoner_weakness.wav", 1);
    LoadWaveFile(SOUND_SKILL_ENERVATION, L"Data\\Sound\\SE_Ch_summoner_innovation.wav", 1);
    LoadWaveFile(SOUND_SKILL_BERSERKER, L"Data\\Sound\\Berserker.wav", 1);
    LoadWaveFile(SOUND_CHERRYBLOSSOM_EFFECT0, L"Data\\Sound\\cherryblossom\\Eve_CherryBlossoms01.wav");
    LoadWaveFile(SOUND_CHERRYBLOSSOM_EFFECT1, L"Data\\Sound\\cherryblossom\\Eve_CherryBlossoms02.wav");
    LoadWaveFile(SOUND_SKILL_BLOWOFDESTRUCTION, L"Data\\Sound\\BLOW_OF_DESTRUCTION.wav");
    LoadWaveFile(SOUND_SKILL_FLAME_STRIKE, L"Data\\Sound\\flame_strike.wav");
    LoadWaveFile(SOUND_SKILL_GIGANTIC_STORM, L"Data\\Sound\\gigantic_storm.wav");
    LoadWaveFile(SOUND_SKILL_LIGHTNING_SHOCK, L"Data\\Sound\\lightning_shock.wav");
    LoadWaveFile(SOUND_SKILL_SWELL_OF_MAGICPOWER, L"Data\\Sound\\SwellofMagicPower.wav");
    LoadWaveFile(SOUND_SKILL_MULTI_SHOT, L"Data\\Sound\\multi_shot.wav");
    LoadWaveFile(SOUND_SKILL_RECOVER, L"Data\\Sound\\recover.wav");
    LoadWaveFile(SOUND_SKILL_CAOTIC, L"Data\\Sound\\caotic.wav");

    LoadWaveFile(SOUND_XMAS_FIRECRACKER, L"Data\\Sound\\xmas\\Christmas_Fireworks01.wav");

    g_09SummerEvent->LoadSound();

    LoadWaveFile(SOUND_RAGESKILL_THRUST, L"Data\\Sound\\Ragefighter\\Rage_Thrust.wav");
    LoadWaveFile(SOUND_RAGESKILL_THRUST_ATTACK, L"Data\\Sound\\Ragefighter\\Rage_Thrust_Att.wav");
    LoadWaveFile(SOUND_RAGESKILL_STAMP, L"Data\\Sound\\Ragefighter\\Rage_Stamp.wav");
    LoadWaveFile(SOUND_RAGESKILL_STAMP_ATTACK, L"Data\\Sound\\Ragefighter\\Rage_Stamp_Att.wav");
    LoadWaveFile(SOUND_RAGESKILL_GIANTSWING, L"Data\\Sound\\Ragefighter\\Rage_Giantswing.wav");
    LoadWaveFile(SOUND_RAGESKILL_GIANTSWING_ATTACK, L"Data\\Sound\\Ragefighter\\Rage_Giantswing_Att.wav");
    LoadWaveFile(SOUND_RAGESKILL_DARKSIDE, L"Data\\Sound\\Ragefighter\\Rage_Darkside.wav");
    LoadWaveFile(SOUND_RAGESKILL_DARKSIDE_ATTACK, L"Data\\Sound\\Ragefighter\\Rage_Darkside_Att.wav");
    LoadWaveFile(SOUND_RAGESKILL_DRAGONLOWER, L"Data\\Sound\\Ragefighter\\Rage_Dragonlower.wav");
    LoadWaveFile(SOUND_RAGESKILL_DRAGONLOWER_ATTACK, L"Data\\Sound\\Ragefighter\\Rage_Dragonlower_Att.wav");
    LoadWaveFile(SOUND_RAGESKILL_DRAGONKICK, L"Data\\Sound\\Ragefighter\\Rage_Dragonkick.wav");
    LoadWaveFile(SOUND_RAGESKILL_DRAGONKICK_ATTACK, L"Data\\Sound\\Ragefighter\\Rage_Dragonkick_Att.wav");
    LoadWaveFile(SOUND_RAGESKILL_BUFF_1, L"Data\\Sound\\Ragefighter\\Rage_Buff_1.wav");
    LoadWaveFile(SOUND_RAGESKILL_BUFF_2, L"Data\\Sound\\Ragefighter\\Rage_Buff_2.wav");
}

extern int	g_iRenderTextType;

void OpenFont()
{
    InitPath();

    LoadBitmap(L"Interface\\FontInput.tga", BITMAP_FONT, GL_NEAREST, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\FontTest.tga", BITMAP_FONT + 1);
    LoadBitmap(L"Interface\\Hit.tga", BITMAP_FONT_HIT, GL_NEAREST, GL_CLAMP_TO_EDGE);

    g_pRenderText->Create(0, g_hDC);
}

void SaveMacro(const wchar_t* FileName)
{
    FILE* fp = _wfopen(FileName, L"wt");
    for (int i = 0; i < 10; i++)
    {
        fwprintf(fp, L"%s\n", MacroText[i]);
    }
    fclose(fp);
}

void OpenMacro(const wchar_t* FileName)
{
    FILE* fp = _wfopen(FileName, L"rt");
    if (fp == NULL) return;
    for (int i = 0; i < 10; i++)
    {
        fwscanf(fp, L"%s", MacroText[i]);
    }
    fclose(fp);
}

void SaveOptions()
{
    // 0 ~ 19 skill hotkey
    BYTE options[30]{};

    int iSkillType = -1;
    for (int i = 0; i < 10; ++i)
    {
        iSkillType = g_pMainFrame->GetSkillHotKey(i);

        int iIndex = i * 2;
        if (iSkillType != -1)
        {
            options[iIndex] = HIBYTE(CharacterAttribute->Skill[iSkillType]);
            options[iIndex + 1] = LOBYTE(CharacterAttribute->Skill[iSkillType]);
        }
        else
        {
            options[iIndex] = 0xff;
            options[iIndex + 1] = 0xff;
        }
    }

    if (g_pOption->IsAutoAttack())
    {
        options[20] |= AUTOATTACK_ON;
    }
    else
    {
        options[20] |= AUTOATTACK_OFF;
    }

    if (g_pOption->IsWhisperSound())
    {
        options[20] |= WHISPER_SOUND_ON;
    }
    else
    {
        options[20] |= WHISPER_SOUND_OFF;
    }

    if (g_pOption->IsSlideHelp() == false)
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
    options[24] = (((wChatListBoxSize << 4) & 0xF0) | (wChatListBoxBackAlpha & 0x0F)) & 0xFF;
    options[25] = static_cast<BYTE>((g_pMainFrame->GetItemHotKey(SEASON3B::HOTKEY_R) - ITEM_POTION) & 0xFF);

    options[26] = g_pMainFrame->GetItemHotKeyLevel(SEASON3B::HOTKEY_Q);
    options[27] = g_pMainFrame->GetItemHotKeyLevel(SEASON3B::HOTKEY_W);
    options[28] = g_pMainFrame->GetItemHotKeyLevel(SEASON3B::HOTKEY_E);
    options[29] = g_pMainFrame->GetItemHotKeyLevel(SEASON3B::HOTKEY_R);

    SocketClient->ToGameServer()->SendSaveKeyConfiguration(options, sizeof options);
}

void OpenLogoSceneData()
{
    //image
    ::LoadBitmap(L"Interface\\cha_bt.tga", BITMAP_LOG_IN);
    ::LoadBitmap(L"Interface\\server_b2_all.tga", BITMAP_LOG_IN + 1);
    ::LoadBitmap(L"Interface\\server_b2_loding.jpg", BITMAP_LOG_IN + 2);
    ::LoadBitmap(L"Interface\\server_deco_all.tga", BITMAP_LOG_IN + 3);
    ::LoadBitmap(L"Interface\\server_menu_b_all.tga", BITMAP_LOG_IN + 4);
    ::LoadBitmap(L"Interface\\server_credit_b_all.tga", BITMAP_LOG_IN + 5);
    ::LoadBitmap(L"Interface\\deco.tga", BITMAP_LOG_IN + 6);
    ::LoadBitmap(L"Interface\\login_back.tga", BITMAP_LOG_IN + 7);
    ::LoadBitmap(L"Interface\\login_me.tga", BITMAP_LOG_IN + 8);
    ::LoadBitmap(L"Interface\\server_ex03.tga", BITMAP_LOG_IN + 11, GL_NEAREST, GL_REPEAT);
    ::LoadBitmap(L"Interface\\server_ex01.tga", BITMAP_LOG_IN + 12);
    ::LoadBitmap(L"Interface\\server_ex02.jpg", BITMAP_LOG_IN + 13, GL_NEAREST, GL_REPEAT);
    ::LoadBitmap(L"Interface\\cr_mu_lo.tga", BITMAP_LOG_IN + 14, GL_LINEAR);
}

void ReleaseLogoSceneData()
{
    for (int i = BITMAP_LOG_IN; i <= BITMAP_LOG_IN_END; ++i)
        ::DeleteBitmap(i);
    for (int i = BITMAP_TEMP; i < BITMAP_TEMP + 30; i++)
        DeleteBitmap(i);

    gMapManager.DeleteObjects();
    ClearCharacters();
}

void OpenCharacterSceneData()
{
    LoadBitmap(L"Interface\\cha_id.tga", BITMAP_LOG_IN);
    LoadBitmap(L"Interface\\cha_bt.tga", BITMAP_LOG_IN + 1);
    LoadBitmap(L"Interface\\deco.tga", BITMAP_LOG_IN + 2);
    LoadBitmap(L"Interface\\b_create.tga", BITMAP_LOG_IN + 3);
    LoadBitmap(L"Interface\\server_menu_b_all.tga", BITMAP_LOG_IN + 4);
    LoadBitmap(L"Interface\\b_connect.tga", BITMAP_LOG_IN + 5);
    LoadBitmap(L"Interface\\b_delete.tga", BITMAP_LOG_IN + 6);
    LoadBitmap(L"Interface\\character_ex.tga", BITMAP_LOG_IN + 7);
    ::LoadBitmap(L"Interface\\server_ex03.tga", BITMAP_LOG_IN + 11, GL_NEAREST, GL_REPEAT);
    ::LoadBitmap(L"Interface\\server_ex01.tga", BITMAP_LOG_IN + 12);
    ::LoadBitmap(L"Interface\\server_ex02.jpg", BITMAP_LOG_IN + 13, GL_NEAREST, GL_REPEAT);
    LoadBitmap(L"Effect\\Impack03.jpg", BITMAP_EXT_LOG_IN + 2);
    LoadBitmap(L"Logo\\chasellight.jpg", BITMAP_EFFECT);

    int Class = MAX_CLASS;

    for (int i = 4; i < 5; i++)
        gLoadData.AccessModel(MODEL_LOGO + i, L"Data\\Logo\\", L"Logo", i + 1);

    for (int i = 0; i < Class; i++)
        gLoadData.AccessModel(MODEL_FACE + i, L"Data\\Logo\\", L"NewFace", i + 1);

    for (int i = 4; i < 5; i++)
        gLoadData.OpenTexture(MODEL_LOGO + i, L"Logo\\", GL_REPEAT, GL_LINEAR);

    for (int i = 0; i < Class; i++)
        gLoadData.OpenTexture(MODEL_FACE + i, L"Logo\\");

    for (int i = 0; i < Class; i++)
    {
        Models[MODEL_FACE + i].Actions[0].PlaySpeed = 0.3f;
        Models[MODEL_FACE + i].Actions[1].PlaySpeed = 0.3f;
    }
    Models[MODEL_FACE + CLASS_SUMMONER].Actions[0].PlaySpeed = 0.25f;
}

void ReleaseCharacterSceneData()
{
    for (int i = BITMAP_LOG_IN; i <= BITMAP_LOG_IN_END; ++i)
        ::DeleteBitmap(i);

    for (int i = BITMAP_TEMP; i < BITMAP_EXT_LOG_IN + 7; i++)
        DeleteBitmap(i);

    gMapManager.DeleteObjects();

    for (int i = 0; i < MAX_CLASS; i++)
        Models[MODEL_FACE + i].Release();

    ClearCharacters();
}

void OpenBasicData(HDC hDC)
{
    CUIMng& rUIMng = CUIMng::Instance();

    rUIMng.RenderTitleSceneUI(hDC, 0, 11);

    LoadBitmap(L"Interface\\Cursor.tga", BITMAP_CURSOR, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\CursorPush.tga", BITMAP_CURSOR + 1, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\CursorAttack.tga", BITMAP_CURSOR + 2, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\CursorGet.tga", BITMAP_CURSOR + 3, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\CursorTalk.tga", BITMAP_CURSOR + 4, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\CursorRepair.tga", BITMAP_CURSOR + 5, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\CursorLeanAgainst.tga", BITMAP_CURSOR + 6, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\CursorSitDown.tga", BITMAP_CURSOR + 7, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\CursorDontMove.tga", BITMAP_CURSOR + 8, GL_LINEAR, GL_CLAMP_TO_EDGE);
    //interface
    LoadBitmap(L"Interface\\ok.jpg", BITMAP_INTERFACE + 11);
    LoadBitmap(L"Interface\\ok2.jpg", BITMAP_INTERFACE + 12);
    LoadBitmap(L"Interface\\cancel.jpg", BITMAP_INTERFACE + 13);
    LoadBitmap(L"Interface\\cancel2.jpg", BITMAP_INTERFACE + 14);
    LoadBitmap(L"Interface\\win_titlebar.jpg", BITMAP_INTERFACE_EX + 8);
    LoadBitmap(L"Interface\\win_button.tga", BITMAP_INTERFACE_EX + 9);
    LoadBitmap(L"Interface\\win_size.jpg", BITMAP_INTERFACE_EX + 10);
    LoadBitmap(L"Interface\\win_resize.tga", BITMAP_INTERFACE_EX + 11);
    LoadBitmap(L"Interface\\win_scrollbar.jpg", BITMAP_INTERFACE_EX + 12);
    LoadBitmap(L"Interface\\win_check.tga", BITMAP_INTERFACE_EX + 13);
    LoadBitmap(L"Interface\\win_mail.tga", BITMAP_INTERFACE_EX + 14);
    LoadBitmap(L"Interface\\win_mark.tga", BITMAP_INTERFACE_EX + 15);
    LoadBitmap(L"Interface\\win_letter.jpg", BITMAP_INTERFACE_EX + 16);
    LoadBitmap(L"Interface\\win_man.jpg", BITMAP_INTERFACE_EX + 17);
    LoadBitmap(L"Interface\\win_push.jpg", BITMAP_INTERFACE_EX + 18);
    LoadBitmap(L"Interface\\win_question.tga", BITMAP_INTERFACE_EX + 20);
    LoadBitmap(L"Local\\Webzenlogo.jpg", BITMAP_INTERFACE_EX + 22);

#ifdef DUEL_SYSTEM
    LoadWaveFile(SOUND_OPEN_DUELWINDOW, L"Data\\Sound\\iDuelWindow.wav", 1);
    LoadWaveFile(SOUND_START_DUEL, L"Data\\Sound\\iDuelStart.wav", 1);
#endif // DUEL_SYSTEM

    LoadBitmap(L"Interface\\CursorID.tga", BITMAP_INTERFACE_EX + 29);
    LoadBitmap(L"Interface\\bar.jpg", BITMAP_INTERFACE + 23);
    LoadBitmap(L"Interface\\back1.jpg", BITMAP_INTERFACE + 24);
    LoadBitmap(L"Interface\\back2.jpg", BITMAP_INTERFACE + 25);
    LoadBitmap(L"Interface\\back3.jpg", BITMAP_INTERFACE + 26);

    LoadBitmap(L"Effect\\Fire01.jpg", BITMAP_FIRE, GL_LINEAR, GL_CLAMP_TO_EDGE);			// GM3rdChangeUp, GMCrywolf1st,GMHellas,Kanturu 3rd
    LoadBitmap(L"Effect\\Fire02.jpg", BITMAP_FIRE + 1, GL_LINEAR, GL_CLAMP_TO_EDGE);			// GM3rdChangeUp, GMCrywolf1st,GMHellas,Kanturu 3rd
    LoadBitmap(L"Effect\\Fire03.jpg", BITMAP_FIRE + 2, GL_LINEAR, GL_CLAMP_TO_EDGE);			// GM3rdChangeUp, GMCrywolf1st,GMHellas,Kanturu 3rd
    LoadBitmap(L"Effect\\PoundingBall.jpg", BITMAP_POUNDING_BALL, GL_LINEAR, GL_CLAMP_TO_EDGE); // Kanturu 2nd
    LoadBitmap(L"Effect\\fi01.jpg", BITMAP_ADV_SMOKE, GL_LINEAR, GL_CLAMP_TO_EDGE);		// GM 3rd ChangeUp, CryingWolf2nd
    LoadBitmap(L"Effect\\fi02.tga", BITMAP_ADV_SMOKE + 1, GL_LINEAR, GL_CLAMP_TO_EDGE);		// GM 3rd ChangeUp, CryingWolf2nd
    LoadBitmap(L"Effect\\fantaF.jpg", BITMAP_TRUE_FIRE, GL_LINEAR, GL_CLAMP_TO_EDGE);	// GM Aida, GMBattleCastle, ....
    LoadBitmap(L"Effect\\fantaB.jpg", BITMAP_TRUE_BLUE, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\JointSpirit02.jpg", BITMAP_JOINT_SPIRIT2, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\Piercing.jpg", BITMAP_PIERCING, GL_LINEAR, GL_REPEAT);
    LoadBitmap(L"Monster\\iui06.jpg", BITMAP_ROBE + 6);
    LoadBitmap(L"Effect\\Magic_b.jpg", BITMAP_MAGIC_EMBLEM);
    LoadBitmap(L"Player\\dark3chima3.tga", BITMAP_DARKLOAD_SKIRT_3RD, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Player\\kaa.tga", BITMAP_DARK_LOAD_SKIRT, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\ShockWave.jpg", BITMAP_SHOCK_WAVE, GL_LINEAR);
    LoadBitmap(L"Effect\\Flame01.jpg", BITMAP_FLAME, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\flare01.jpg", BITMAP_LIGHT, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\Magic_Ground1.jpg", BITMAP_MAGIC, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\Magic_Ground2.jpg", BITMAP_MAGIC + 1, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\Magic_Circle1.jpg", BITMAP_MAGIC + 2, GL_LINEAR);
#ifdef ASG_ADD_INFLUENCE_GROUND_EFFECT
    LoadBitmap(L"Effect\\guild_ring01.jpg", BITMAP_OUR_INFLUENCE_GROUND, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\enemy_ring02.jpg", BITMAP_ENEMY_INFLUENCE_GROUND, GL_LINEAR, GL_CLAMP_TO_EDGE);
#endif	// ASG_ADD_INFLUENCE_GROUND_EFFECT
    LoadBitmap(L"Effect\\Spark02.jpg", BITMAP_SPARK, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\Spark03.jpg", BITMAP_SPARK + 1, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\smoke01.jpg", BITMAP_SMOKE, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\smoke02.tga", BITMAP_SMOKE + 1, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\smoke05.tga", BITMAP_SMOKE + 4, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\blood01.tga", BITMAP_BLOOD, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\blood.tga", BITMAP_BLOOD + 1, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\Explotion01.jpg", BITMAP_EXPLOTION, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Skill\\twlighthik01.jpg", BITMAP_TWLIGHT, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Skill\\2line_gost.jpg", BITMAP_2LINE_GHOST, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\damage01mono.jpg", BITMAP_DAMAGE_01_MONO, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\SwordEff_mono.jpg", BITMAP_SWORD_EFFECT_MONO, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\flamestani.jpg", BITMAP_FLAMESTANI, GL_LINEAR, GL_CLAMP_TO_EDGE);

    LoadBitmap(L"Effect\\Spark.jpg", BITMAP_SPARK + 2, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\firehik02.jpg", BITMAP_FIRE_CURSEDLICH, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Monster\\totemgolem_leaf.tga", BITMAP_LEAF_TOTEMGOLEM, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\empact01.jpg", BITMAP_SUMMON_IMPACT, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\loungexflow.jpg", BITMAP_SUMMON_SAHAMUTT_EXPLOSION, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\gostmark01.jpg", BITMAP_DRAIN_LIFE_GHOST, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\mzine_typer2.jpg", BITMAP_MAGIC_ZIN, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\shiny05.jpg", BITMAP_SHINY + 6, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\hikorora.jpg", BITMAP_ORORA, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\lightmarks.jpg", BITMAP_LIGHT_MARKS, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\cursorpin01.jpg", BITMAP_TARGET_POSITION_EFFECT1, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\cursorpin02.jpg", BITMAP_TARGET_POSITION_EFFECT2, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\smokelines01.jpg", BITMAP_SMOKELINE1, GL_LINEAR, GL_REPEAT);
    LoadBitmap(L"Effect\\smokelines02.jpg", BITMAP_SMOKELINE2, GL_LINEAR, GL_REPEAT);
    LoadBitmap(L"Effect\\smokelines03.jpg", BITMAP_SMOKELINE3, GL_LINEAR, GL_REPEAT);
    LoadBitmap(L"Effect\\lighting_mega01.jpg", BITMAP_LIGHTNING_MEGA1, GL_LINEAR, GL_REPEAT);
    LoadBitmap(L"Effect\\lighting_mega02.jpg", BITMAP_LIGHTNING_MEGA2, GL_LINEAR, GL_REPEAT);
    LoadBitmap(L"Effect\\lighting_mega03.jpg", BITMAP_LIGHTNING_MEGA3, GL_LINEAR, GL_REPEAT);
    LoadBitmap(L"Effect\\firehik01.jpg", BITMAP_FIRE_HIK1, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\firehik03.jpg", BITMAP_FIRE_HIK3, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\firehik_mono01.jpg", BITMAP_FIRE_HIK1_MONO, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\firehik_mono02.jpg", BITMAP_FIRE_HIK2_MONO, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\firehik_mono03.jpg", BITMAP_FIRE_HIK3_MONO, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\clouds3.jpg", BITMAP_RAKLION_CLOUDS, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Monster\\icenightlight.jpg", BITMAP_IRONKNIGHT_BODY_BRIGHT, GL_LINEAR, GL_CLAMP_TO_EDGE);

    g_XmasEvent->LoadXmasEventEffect();

    LoadBitmap(L"Skill\\younghtest1.tga", BITMAP_GM_HAIR_1);
    LoadBitmap(L"Skill\\younghtest3.tga", BITMAP_GM_HAIR_3);
    LoadBitmap(L"Skill\\gmmzine.jpg", BITMAP_GM_AURORA, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\explotion01mono.jpg", BITMAP_EXPLOTION_MONO, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\Success_kantru.tga", BITMAP_KANTURU_SUCCESS, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\Failure_kantru.tga", BITMAP_KANTURU_FAILED, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\MonsterCount.tga", BITMAP_KANTURU_COUNTER, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\Clud64.jpg", BITMAP_CLUD64, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\clouds.jpg", BITMAP_CLOUD, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Skill\\SwordEff.jpg", BITMAP_BLUE_BLUR, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\Impack03.jpg", BITMAP_IMPACT, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\ScolTail.jpg", BITMAP_SCOLPION_TAIL, GL_LINEAR);
    LoadBitmap(L"Effect\\FireSnuff.jpg", BITMAP_FIRE_SNUFF, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\coll.jpg", BITMAP_SPOT_WATER, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\BowE.jpg", BITMAP_DS_EFFECT, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\Shockwave.jpg", BITMAP_DS_SHOCK, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\DinoE.jpg", BITMAP_EXPLOTION + 1, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\Shiny01.jpg", BITMAP_SHINY, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\Shiny02.jpg", BITMAP_SHINY + 1, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\Shiny03.jpg", BITMAP_SHINY + 2, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\eye01.jpg", BITMAP_SHINY + 3, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\ring.jpg", BITMAP_SHINY + 4, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\Shiny04.jpg", BITMAP_SHINY + 5, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\Chrome01.jpg", BITMAP_CHROME, GL_LINEAR, GL_REPEAT);
    LoadBitmap(L"Effect\\blur01.jpg", BITMAP_BLUR, GL_NEAREST, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\bab2.jpg", BITMAP_CHROME + 1, GL_LINEAR, GL_REPEAT);
    LoadBitmap(L"Effect\\motion_blur.jpg", BITMAP_BLUR + 1, GL_NEAREST, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\motion_blur_r.jpg", BITMAP_BLUR + 2, GL_NEAREST, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\motion_mono.jpg", BITMAP_BLUR + 3, GL_NEAREST, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\motion_blur_r3.jpg", BITMAP_BLUR + 6, GL_NEAREST, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\gra.jpg", BITMAP_BLUR + 7, GL_NEAREST, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\spinmark01.jpg", BITMAP_BLUR + 8, GL_NEAREST, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\flamestani.jpg", BITMAP_BLUR + 9, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\sword_blur.jpg", BITMAP_BLUR + 10, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\joint_sword_red.jpg", BITMAP_BLUR + 11, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\motion_blur_r2.jpg", BITMAP_BLUR + 12, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\motion_blur_r3.jpg", BITMAP_BLUR + 13, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\blur02.jpg", BITMAP_BLUR2, GL_NEAREST, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\lightning2.jpg", BITMAP_LIGHTNING + 1, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\Thunder01.jpg", BITMAP_ENERGY, GL_LINEAR, GL_REPEAT);
    LoadBitmap(L"Effect\\Spark01.jpg", BITMAP_JOINT_SPARK, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\JointThunder01.jpg", BITMAP_JOINT_THUNDER, GL_LINEAR, GL_REPEAT);
    LoadBitmap(L"Effect\\JointSpirit01.jpg", BITMAP_JOINT_SPIRIT, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\JointLaser01.jpg", BITMAP_JOINT_ENERGY, GL_LINEAR);
    LoadBitmap(L"Effect\\JointEnergy01.jpg", BITMAP_JOINT_HEALING, GL_LINEAR);
    LoadBitmap(L"Effect\\JointLaser02.jpg", BITMAP_JOINT_LASER + 1, GL_LINEAR, GL_REPEAT);
    LoadBitmap(L"Monster\\iui03.jpg", BITMAP_JANUSEXT, GL_LINEAR, GL_REPEAT);
    LoadBitmap(L"Monster\\magic_H.tga", BITMAP_PHO_R_HAIR, GL_LINEAR, GL_REPEAT);
    LoadBitmap(L"Item\\t_lower_14m.tga", BITMAP_PANTS_G_SOUL, GL_LINEAR, GL_REPEAT);
    LoadBitmap(L"Skill\\Skull.jpg", BITMAP_SKULL, GL_NEAREST);
    LoadBitmap(L"Effect\\motion_blur_r2.jpg", BITMAP_JOINT_FORCE, GL_LINEAR, GL_REPEAT);
    LoadBitmap(L"Effect\\Fire04.jpg", BITMAP_FIRECRACKER, GL_LINEAR);
    LoadBitmap(L"Effect\\Flare.jpg", BITMAP_FLARE, GL_LINEAR, GL_CLAMP_TO_EDGE);

    LoadBitmap(L"Effect\\Chrome02.jpg", BITMAP_CHROME2, GL_NEAREST, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\Chrome03.jpg", BITMAP_CHROME3, GL_NEAREST, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\Chrome06.jpg", BITMAP_CHROME6, GL_NEAREST, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\Chrome07.jpg", BITMAP_CHROME7, GL_NEAREST, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\energy01.jpg", BITMAP_CHROME_ENERGY, GL_NEAREST, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\energy02.jpg", BITMAP_CHROME_ENERGY2, GL_LINEAR, GL_CLAMP_TO_EDGE);

    LoadBitmap(L"Effect\\firecracker0001.jpg", BITMAP_FIRECRACKER0001, GL_NEAREST, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\firecracker0002.jpg", BITMAP_FIRECRACKER0002, GL_NEAREST, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\firecracker0003.jpg", BITMAP_FIRECRACKER0003, GL_NEAREST, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\firecracker0004.jpg", BITMAP_FIRECRACKER0004, GL_NEAREST, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\firecracker0005.jpg", BITMAP_FIRECRACKER0005, GL_NEAREST, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\firecracker0006.jpg", BITMAP_FIRECRACKER0006, GL_NEAREST, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\firecracker0007.jpg", BITMAP_FIRECRACKER0007, GL_NEAREST, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\Shiny05.jpg", BITMAP_SHINY + 5, GL_LINEAR, GL_CLAMP_TO_EDGE);

    LoadBitmap(L"Effect\\partCharge1\\bujuckline.jpg", BITMAP_LUCKY_SEAL_EFFECT, GL_LINEAR, GL_CLAMP_TO_EDGE);

    LoadBitmap(L"Item\\nfm03.jpg", BITMAP_BLUECHROME, GL_NEAREST, GL_REPEAT);
    LoadBitmap(L"Effect\\flareBlue.jpg", BITMAP_FLARE_BLUE, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\NSkill.jpg", BITMAP_FLARE_FORCE, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\Flare02.jpg", BITMAP_FLARE + 1, GL_LINEAR, GL_REPEAT);
    LoadBitmap(L"Monster\\King11.jpg", BITMAP_WHITE_WIZARD, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Monster\\Kni000.jpg", BITMAP_DEST_ORC_WAR0, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Monster\\Kni011.jpg", BITMAP_DEST_ORC_WAR1, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Monster\\Kni022.jpg", BITMAP_DEST_ORC_WAR2, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Skill\\pinkWave.jpg", BITMAP_PINK_WAVE, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\flareRed.jpg", BITMAP_FLARE_RED, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\Fire05.jpg", BITMAP_FIRE + 3, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\Hole.jpg", BITMAP_HOLE, GL_LINEAR, GL_REPEAT);
    //	LoadBitmap( "Monster\\mop011.jpg"        , BITMAP_OTHER_SKIN,    GL_LINEAR, GL_REPEAT );
    LoadBitmap(L"Effect\\WATERFALL1.jpg", BITMAP_WATERFALL_1, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\WATERFALL2.jpg", BITMAP_WATERFALL_2, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\WATERFALL3.jpg", BITMAP_WATERFALL_3, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\WATERFALL4.jpg", BITMAP_WATERFALL_4, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\WATERFALL5.jpg", BITMAP_WATERFALL_5, GL_LINEAR, GL_CLAMP_TO_EDGE);
    /*
    LoadBitmap(L"Interface\\in_bar.tga"		, BITMAP_MVP_INTERFACE, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\in_bar2.jpg"		, BITMAP_MVP_INTERFACE + 1, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\in_deco.tga"		, BITMAP_MVP_INTERFACE + 2, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\in_main.tga"		, BITMAP_MVP_INTERFACE + 3, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\in_main_icon_bal1.tga"		, BITMAP_MVP_INTERFACE + 4, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\in_main_icon_dl1.tga"		, BITMAP_MVP_INTERFACE + 5, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\in_main_icon_dl2.tga"		, BITMAP_MVP_INTERFACE + 6, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\in_main_number1.tga"		, BITMAP_MVP_INTERFACE + 7, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\in_main_number2.tga"		, BITMAP_MVP_INTERFACE + 8, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\in_main2.tga"		, BITMAP_MVP_INTERFACE + 9, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\icon_failure.tga"		, BITMAP_MVP_INTERFACE + 10, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\icon_success.tga"		, BITMAP_MVP_INTERFACE + 11, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\t_main.tga"		, BITMAP_MVP_INTERFACE + 12, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\m_b_no1.tga"		, BITMAP_MVP_INTERFACE + 13, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\m_b_no2.tga"		, BITMAP_MVP_INTERFACE + 14, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\m_b_no3.tga"		, BITMAP_MVP_INTERFACE + 15, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\m_b_ok1.tga"		, BITMAP_MVP_INTERFACE + 16, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\m_b_ok2.tga"		, BITMAP_MVP_INTERFACE + 17, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\m_b_ok3.tga"		, BITMAP_MVP_INTERFACE + 18, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\m_b_yes1.tga"		, BITMAP_MVP_INTERFACE + 19, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\m_b_yes2.tga"		, BITMAP_MVP_INTERFACE + 20, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\m_b_yes3.tga"		, BITMAP_MVP_INTERFACE + 21, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\m_main.tga"		, BITMAP_MVP_INTERFACE + 22, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\in_main_number1_1.tga"		, BITMAP_MVP_INTERFACE + 23, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\in_main_number2_1.tga"		, BITMAP_MVP_INTERFACE + 24, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\in_main_number0_2.tga"		, BITMAP_MVP_INTERFACE + 25, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\icon_Rank_table.tga"		, BITMAP_MVP_INTERFACE + 26, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\icon_Rank_rank.tga"		, BITMAP_MVP_INTERFACE + 27, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\icon_Rank_D.tga"		, BITMAP_MVP_INTERFACE + 28, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\icon_Rank_C.tga"		, BITMAP_MVP_INTERFACE + 29, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\icon_Rank_B.tga"		, BITMAP_MVP_INTERFACE + 30, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\icon_Rank_A.tga"		, BITMAP_MVP_INTERFACE + 31, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\icon_Rank_S.tga"		, BITMAP_MVP_INTERFACE + 32, GL_LINEAR, GL_CLAMP_TO_EDGE);
    //icon_Rank_0
    LoadBitmap(L"Interface\\icon_Rank_0.tga"		, BITMAP_MVP_INTERFACE + 33, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\icon_Rank_1.tga"		, BITMAP_MVP_INTERFACE + 34, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\icon_Rank_2.tga"		, BITMAP_MVP_INTERFACE + 35, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\icon_Rank_3.tga"		, BITMAP_MVP_INTERFACE + 36, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\icon_Rank_4.tga"		, BITMAP_MVP_INTERFACE + 37, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\icon_Rank_5.tga"		, BITMAP_MVP_INTERFACE + 38, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\icon_Rank_6.tga"		, BITMAP_MVP_INTERFACE + 39, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\icon_Rank_7.tga"		, BITMAP_MVP_INTERFACE + 40, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\icon_Rank_8.tga"		, BITMAP_MVP_INTERFACE + 41, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\icon_Rank_9.tga"		, BITMAP_MVP_INTERFACE + 42, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\icon_Rank_exp.tga"		, BITMAP_MVP_INTERFACE + 43, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\m_main_rank.tga"		, BITMAP_MVP_INTERFACE + 44, GL_LINEAR, GL_CLAMP_TO_EDGE);
    */
    //m_main_rank
    //icon_Rank_exp
    LoadBitmap(L"Interface\\BattleSkill.tga", BITMAP_INTERFACE_EX + 34);
    LoadBitmap(L"Effect\\Flashing.jpg", BITMAP_FLASH, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\senatusmap.jpg", BITMAP_INTERFACE_EX + 35);
    LoadBitmap(L"Interface\\gate_button2.jpg", BITMAP_INTERFACE_EX + 36);
    LoadBitmap(L"Interface\\gate_button1.jpg", BITMAP_INTERFACE_EX + 37);
    LoadBitmap(L"Interface\\suho_button2.jpg", BITMAP_INTERFACE_EX + 38);
    LoadBitmap(L"Interface\\suho_button1.jpg", BITMAP_INTERFACE_EX + 39);
    LoadBitmap(L"Interface\\DoorCL.jpg", BITMAP_INTERFACE_EX + 40);
    LoadBitmap(L"Interface\\DoorOP.jpg", BITMAP_INTERFACE_EX + 41);
    //OpenJpeg( "Effect\\FireSnuff.jpg"       , BITMAP_FIRE_SNUFF,    GL_LINEAR, GL_CLAMP_TO_EDGE );
    LoadBitmap(L"Object31\\Flag.tga", BITMAP_INTERFACE_MAP + 0, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\CursorAttack2.tga", BITMAP_CURSOR2);
    LoadBitmap(L"Effect\\Cratered.tga", BITMAP_CRATER);
    LoadBitmap(L"Effect\\FormationMark.tga", BITMAP_FORMATION_MARK);
    LoadBitmap(L"Effect\\Plus.tga", BITMAP_PLUS);
    LoadBitmap(L"Effect\\eff_lighting.jpg", BITMAP_FENRIR_THUNDER);
    LoadBitmap(L"Effect\\eff_lightinga01.jpg", BITMAP_FENRIR_FOOT_THUNDER1);
    LoadBitmap(L"Effect\\eff_lightinga02.jpg", BITMAP_FENRIR_FOOT_THUNDER2);
    LoadBitmap(L"Effect\\eff_lightinga03.jpg", BITMAP_FENRIR_FOOT_THUNDER3);
    LoadBitmap(L"Effect\\eff_lightinga04.jpg", BITMAP_FENRIR_FOOT_THUNDER4);
    LoadBitmap(L"Effect\\eff_lightinga05.jpg", BITMAP_FENRIR_FOOT_THUNDER5);
    LoadBitmap(L"Interface\\Progress_Back.jpg", BITMAP_INTERFACE_EX + 42);
    LoadBitmap(L"Interface\\Progress.jpg", BITMAP_INTERFACE_EX + 43);
    LoadBitmap(L"Effect\\Inferno.jpg", BITMAP_INFERNO, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\Lava.jpg", BITMAP_LAVA, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Interface\\frame.tga", BITMAP_INTERFACE_MAP + 3);
    LoadBitmap(L"Interface\\i_attack.tga", BITMAP_INTERFACE_MAP + 4);
    LoadBitmap(L"Interface\\i_defense.tga", BITMAP_INTERFACE_MAP + 5);
    LoadBitmap(L"Interface\\i_wait.tga", BITMAP_INTERFACE_MAP + 6);
    LoadBitmap(L"Interface\\b_command01.jpg", BITMAP_INTERFACE_MAP + 8);
    LoadBitmap(L"Interface\\b_command02.jpg", BITMAP_INTERFACE_MAP + 9);
    LoadBitmap(L"Interface\\b_group02.jpg", BITMAP_INTERFACE_MAP + 10);
    LoadBitmap(L"Interface\\b_zoomout01.jpg", BITMAP_INTERFACE_MAP + 11);
    LoadBitmap(L"Interface\\hourglass.tga", BITMAP_INTERFACE_MAP + 7);
    LoadBitmap(L"Interface\\dot.tga", BITMAP_INTERFACE_EX + 44);
    LoadBitmap(L"Object9\\Impack03.jpg", BITMAP_LIGHT + 1, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Monster\\buserbody_r.jpg", BITMAP_BERSERK_EFFECT, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Monster\\busersword_r.jpg", BITMAP_BERSERK_WP_EFFECT, GL_LINEAR, GL_CLAMP_TO_EDGE);
    //	LoadBitmap(L"Monster\\gigantiscorn_R.jpg", BITMAP_GIGANTIS_EFFECT, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Monster\\bladeeff2_r.jpg", BITMAP_BLADEHUNTER_EFFECT, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Monster\\illumi_R.jpg", BITMAP_TWINTAIL_EFFECT, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Monster\\prsona_R.jpg", BITMAP_PRSONA_EFFECT, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Monster\\prsonass_R.jpg", BITMAP_PRSONA_EFFECT2, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"effect\\water.jpg", BITMAP_TWINTAIL_WATER, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\cra_04.jpg", BITMAP_LIGHT + 2, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\Impack01.jpg", BITMAP_LIGHT + 3, GL_LINEAR, GL_CLAMP_TO_EDGE);
    ::LoadBitmap(L"Interface\\message_ok_b_all.tga", BITMAP_BUTTON);
    ::LoadBitmap(L"Interface\\loding_cancel_b_all.tga", BITMAP_BUTTON + 1);
    ::LoadBitmap(L"Interface\\message_close_b_all.tga", BITMAP_BUTTON + 2);
    ::LoadBitmap(L"Interface\\message_back.tga", BITMAP_MESSAGE_WIN);
    ::LoadBitmap(L"Interface\\delete_secret_number.tga", BITMAP_MSG_WIN_INPUT);
    ::LoadBitmap(L"Interface\\op1_stone.jpg", BITMAP_SYS_WIN, GL_NEAREST, GL_REPEAT);
    ::LoadBitmap(L"Interface\\op1_back1.tga", BITMAP_SYS_WIN + 1);
    ::LoadBitmap(L"Interface\\op1_back2.tga", BITMAP_SYS_WIN + 2);
    ::LoadBitmap(L"Interface\\op1_back3.jpg", BITMAP_SYS_WIN + 3, GL_NEAREST, GL_REPEAT);
    ::LoadBitmap(L"Interface\\op1_back4.jpg", BITMAP_SYS_WIN + 4, GL_NEAREST, GL_REPEAT);
    ::LoadBitmap(L"Interface\\op1_b_all.tga", BITMAP_TEXT_BTN);
    ::LoadBitmap(L"Interface\\op2_back1.tga", BITMAP_OPTION_WIN);
    ::LoadBitmap(L"Interface\\op2_ch.tga", BITMAP_CHECK_BTN);
    ::LoadBitmap(L"Interface\\op2_volume3.tga", BITMAP_SLIDER);
    ::LoadBitmap(L"Interface\\op2_volume2.jpg", BITMAP_SLIDER + 1, GL_NEAREST, GL_REPEAT);
    ::LoadBitmap(L"Interface\\op2_volume1.tga", BITMAP_SLIDER + 2);
    ::LoadBitmap(L"Effect\\clouds2.jpg", BITMAP_EVENT_CLOUD, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\pin_lights.jpg", BITMAP_PIN_LIGHT, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Monster\\deasuler_cloth.tga", BITMAP_DEASULER_CLOTH, GL_LINEAR, GL_CLAMP_TO_EDGE);

    LoadBitmap(L"Item\\soketmagic_stape02.jpg", BITMAP_SOCKETSTAFF, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\lightmarks.jpg", BITMAP_LIGHTMARKS, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Effect\\lightmarks.jpg", BITMAP_LIGHTMARKS_FOREIGN, GL_LINEAR, GL_CLAMP_TO_EDGE);

    rUIMng.RenderTitleSceneUI(hDC, 1, 11);

    ::LoadBitmap(L"Item\\partCharge1\\entrance_R.jpg", BITMAP_FREETICKET_R, GL_LINEAR, GL_CLAMP_TO_EDGE);
    ::LoadBitmap(L"Item\\partCharge1\\juju_R.jpg", BITMAP_CHAOSCARD_R, GL_LINEAR, GL_CLAMP_TO_EDGE);

    ::LoadBitmap(L"Item\\partCharge1\\monmark01a.jpg", BITMAP_LUCKY_SEAL_EFFECT43, GL_LINEAR, GL_CLAMP_TO_EDGE);
    ::LoadBitmap(L"Item\\partCharge1\\monmark02a.jpg", BITMAP_LUCKY_SEAL_EFFECT44, GL_LINEAR, GL_CLAMP_TO_EDGE);
    ::LoadBitmap(L"Item\\partCharge1\\monmark03a.jpg", BITMAP_LUCKY_SEAL_EFFECT45, GL_LINEAR, GL_CLAMP_TO_EDGE);

    ::LoadBitmap(L"Item\\partCharge1\\bujuck01alpa.jpg", BITMAP_LUCKY_CHARM_EFFECT53, GL_LINEAR, GL_CLAMP_TO_EDGE);
    ::LoadBitmap(L"Item\\partCharge1\\expensiveitem01_R.jpg", BITMAP_RAREITEM1_R, GL_LINEAR, GL_CLAMP_TO_EDGE);
    ::LoadBitmap(L"Item\\partCharge1\\expensiveitem02a_R.jpg", BITMAP_RAREITEM2_R, GL_LINEAR, GL_CLAMP_TO_EDGE);
    ::LoadBitmap(L"Item\\partCharge1\\expensiveitem02b_R.jpg", BITMAP_RAREITEM3_R, GL_LINEAR, GL_CLAMP_TO_EDGE);
    ::LoadBitmap(L"Item\\partCharge1\\expensiveitem03a_R.jpg", BITMAP_RAREITEM4_R, GL_LINEAR, GL_CLAMP_TO_EDGE);
    ::LoadBitmap(L"Item\\partCharge1\\expensiveitem03b_R.jpg", BITMAP_RAREITEM5_R, GL_LINEAR, GL_CLAMP_TO_EDGE);

    ::LoadBitmap(L"Item\\partCharge3\\alicecard_R.tga", BITMAP_CHARACTERCARD_R, GL_LINEAR, GL_CLAMP_TO_EDGE);

    ::LoadBitmap(L"Item\\Ingameshop\\kacama_R.jpg", BITMAP_CHARACTERCARD_R_MA, GL_LINEAR, GL_CLAMP_TO_EDGE);
    ::LoadBitmap(L"Item\\Ingameshop\\kacada_R.jpg", BITMAP_CHARACTERCARD_R_DA, GL_LINEAR, GL_CLAMP_TO_EDGE);

    ::LoadBitmap(L"Item\\partCharge3\\jujug_R.jpg", BITMAP_NEWCHAOSCARD_GOLD_R, GL_LINEAR, GL_CLAMP_TO_EDGE);
    ::LoadBitmap(L"Item\\partCharge3\\jujul_R.jpg", BITMAP_NEWCHAOSCARD_RARE_R, GL_LINEAR, GL_CLAMP_TO_EDGE);
    ::LoadBitmap(L"Item\\partCharge3\\jujum_R.jpg", BITMAP_NEWCHAOSCARD_MINI_R, GL_LINEAR, GL_CLAMP_TO_EDGE);

    ::LoadBitmap(L"Effect\\cherryblossom\\sakuras01.jpg", BITMAP_CHERRYBLOSSOM_EVENT_PETAL, GL_LINEAR, GL_CLAMP_TO_EDGE);
    ::LoadBitmap(L"Effect\\cherryblossom\\sakuras02.jpg", BITMAP_CHERRYBLOSSOM_EVENT_FLOWER, GL_LINEAR, GL_CLAMP_TO_EDGE);

    ::LoadBitmap(L"Object39\\k_effect_01.JPG", BITMAP_KANTURU_2ND_EFFECT1, GL_LINEAR, GL_CLAMP_TO_EDGE);
    ::LoadBitmap(L"Item\\deathbeamstone_R.jpg", BITMAP_ITEM_EFFECT_DBSTONE_R, GL_LINEAR, GL_CLAMP_TO_EDGE);
    ::LoadBitmap(L"Item\\hellhorn_R.jpg", BITMAP_ITEM_EFFECT_HELLHORN_R, GL_LINEAR, GL_CLAMP_TO_EDGE);
    ::LoadBitmap(L"Item\\phoenixfeather_R.tga", BITMAP_ITEM_EFFECT_PFEATHER_R, GL_LINEAR, GL_CLAMP_TO_EDGE);
    ::LoadBitmap(L"Item\\Deye_R.jpg", BITMAP_ITEM_EFFECT_DEYE_R, GL_LINEAR, GL_CLAMP_TO_EDGE);
    ::LoadBitmap(L"Item\\wing3chaking2.jpg", BITMAP_ITEM_NIGHT_3RDWING_R, GL_LINEAR, GL_CLAMP_TO_EDGE);

    LoadBitmap(L"NPC\\lumi.jpg", BITMAP_CURSEDTEMPLE_NPC_MESH_EFFECT, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"item\\songko2_R.jpg", BITMAP_CURSEDTEMPLE_HOLYITEM_MESH_EFFECT, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"skill\\unitedsoldier_wing.tga", BITMAP_CURSEDTEMPLE_ALLIED_PHYSICSCLOTH);
    LoadBitmap(L"skill\\illusionistcloth.tga", BITMAP_CURSEDTEMPLE_ILLUSION_PHYSICSCLOTH);
    LoadBitmap(L"effect\\masker.jpg", BITMAP_CURSEDTEMPLE_EFFECT_MASKER, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"effect\\wind01.jpg", BITMAP_CURSEDTEMPLE_EFFECT_WIND, GL_LINEAR, GL_CLAMP_TO_EDGE);
    ::LoadBitmap(L"Interface\\menu01_new2_SD.jpg", BITMAP_INTERFACE_EX + 46);

#ifdef ASG_ADD_GENS_SYSTEM
    std::wstring strFileName = L"Local\\" + g_strSelectedML + L"\\ImgsMapName\\MapNameAddStrife.tga";
    ::LoadBitmap(strFileName.c_str(), BITMAP_INTERFACE_EX + 47);
#endif	// ASG_ADD_GENS_SYSTEM

#ifdef ASG_ADD_GENS_MARK
    ::LoadBitmap(L"Interface\\Gens_mark_D_new.tga", BITMAP_GENS_MARK_DUPRIAN);
    ::LoadBitmap(L"Interface\\Gens_mark_V_new.tga", BITMAP_GENS_MARK_BARNERT);
#endif	// ASG_ADD_GENS_MARK

    LoadBitmap(L"Monster\\serufanarm_R.jpg", BITMAP_SERUFAN_ARM_R, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Monster\\serufanwand_R.jpg", BITMAP_SERUFAN_WAND_R, GL_LINEAR, GL_CLAMP_TO_EDGE);

    LoadBitmap(L"npc\\santa.jpg", BITMAP_GOOD_SANTA, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"npc\\santa_baggage.jpg", BITMAP_GOOD_SANTA_BAGGAGE, GL_LINEAR, GL_CLAMP_TO_EDGE);

#ifdef PJH_ADD_PANDA_CHANGERING
    LoadBitmap(L"Item\\pandabody_R.jpg", BITMAP_PANDABODY_R, GL_LINEAR, GL_LINEAR);	// OPENGL_ERROR
#endif //PJH_ADD_PANDA_CHANGERING

    LoadBitmap(L"Monster\\DGicewalker_body.jpg", BITMAP_DOPPELGANGER_ICEWALKER0, GL_LINEAR, GL_LINEAR);	// OPENGL_ERROR
    LoadBitmap(L"Monster\\DGicewalker_R.jpg", BITMAP_DOPPELGANGER_ICEWALKER1, GL_LINEAR, GL_LINEAR);	// OPENGL_ERROR
    LoadBitmap(L"Monster\\Snake1.jpg", BITMAP_DOPPELGANGER_SNAKE01, GL_LINEAR, GL_LINEAR);	// OPENGL_ERROR

    LoadBitmap(L"NPC\\goldboit.jpg", BITMAP_DOPPELGANGER_GOLDENBOX1, GL_LINEAR, GL_LINEAR);	// OPENGL_ERROR
    LoadBitmap(L"NPC\\goldline.jpg", BITMAP_DOPPELGANGER_GOLDENBOX2, GL_LINEAR, GL_LINEAR);	// OPENGL_ERROR

    //BITMAP_LIGHT_RED
    LoadBitmap(L"effect\\flare01_red.jpg", BITMAP_LIGHT_RED, GL_LINEAR, GL_CLAMP_TO_EDGE);

    LoadBitmap(L"effect\\gra.jpg", BITMAP_GRA, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"effect\\ring_of_gradation.jpg", BITMAP_RING_OF_GRADATION, GL_LINEAR, GL_CLAMP_TO_EDGE);

    LoadBitmap(L"Interface\\InGameShop\\ingame_pack_check.tga", BITMAP_IGS_CHECK_BUTTON, GL_LINEAR);
    LoadBitmap(L"Monster\\AssassinLeader_body_R.jpg", BITMAP_ASSASSIN_EFFECT1, GL_LINEAR, GL_CLAMP_TO_EDGE);

    LoadBitmap(L"Item\\partCharge8\\rareitem_ticket_7_body.jpg", BITMAP_RAREITEM7, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Item\\partCharge8\\rareitem_ticket_8_body.jpg", BITMAP_RAREITEM8, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Item\\partCharge8\\rareitem_ticket_9_body.jpg", BITMAP_RAREITEM9, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Item\\partCharge8\\rareitem_ticket_10_body.jpg", BITMAP_RAREITEM10, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Item\\partCharge8\\rareitem_ticket_11_body.jpg", BITMAP_RAREITEM11, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Item\\partCharge8\\rareitem_ticket_12_body.jpg", BITMAP_RAREITEM12, GL_LINEAR, GL_CLAMP_TO_EDGE);

    LoadBitmap(L"Item\\partCharge8\\DoppelCard.jpg", BITMAP_DOPPLEGANGGER_FREETICKET, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Item\\partCharge8\\BarcaCard.jpg", BITMAP_BARCA_FREETICKET, GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Item\\partCharge8\\Barca7Card.jpg", BITMAP_BARCA7TH_FREETICKET, GL_LINEAR, GL_CLAMP_TO_EDGE);

#ifdef LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM_PART_2
    LoadBitmap(L"Item\\ork_cham_R.jpg", BITMAP_ORK_CHAM_LAYER_R, GL_LINEAR, GL_CLAMP_TO_EDGE);
    //LoadBitmap(L"Item\\maple_cham_R.jpg",			BITMAP_MAPLE_CHAM_LAYER_R,			GL_LINEAR, GL_CLAMP_TO_EDGE);
    LoadBitmap(L"Item\\goldenork_cham_R.jpg", BITMAP_GOLDEN_ORK_CHAM_LAYER_R, GL_LINEAR, GL_CLAMP_TO_EDGE);
    //LoadBitmap(L"Item\\goldenmaple_cham_R.jpg",	BITMAP_GOLDEN_MAPLE_CHAM_LAYER_R,	GL_LINEAR, GL_CLAMP_TO_EDGE);
#endif //LJH_ADD_ITEMS_EQUIPPED_FROM_INVENTORY_SYSTEM_PART_2

    ::LoadBitmap(L"Monster\\BoneSE.jpg", BITMAP_BONE_SCORPION_SKIN_EFFECT, GL_LINEAR, GL_CLAMP_TO_EDGE);
    ::LoadBitmap(L"Monster\\KryptaBall2.jpg", BITMAP_KRYPTA_BALL_EFFECT, GL_LINEAR, GL_CLAMP_TO_EDGE);
    ::LoadBitmap(L"Monster\\bora_golem_effect.jpg", BITMAP_CONDRA_SKIN_EFFECT, GL_LINEAR, GL_CLAMP_TO_EDGE);
    ::LoadBitmap(L"Monster\\bora_golem2_effect.jpg", BITMAP_CONDRA_SKIN_EFFECT2, GL_LINEAR, GL_CLAMP_TO_EDGE);
    ::LoadBitmap(L"Monster\\king_golem01_effect.jpg", BITMAP_NARCONDRA_SKIN_EFFECT, GL_LINEAR, GL_CLAMP_TO_EDGE);
    ::LoadBitmap(L"Monster\\king_golem02_effect.jpg", BITMAP_NARCONDRA_SKIN_EFFECT2, GL_LINEAR, GL_CLAMP_TO_EDGE);
    ::LoadBitmap(L"Monster\\king_golem03_effect.jpg", BITMAP_NARCONDRA_SKIN_EFFECT3, GL_LINEAR, GL_CLAMP_TO_EDGE);

    ::LoadBitmap(L"NPC\\voloE.jpg", BITMAP_VOLO_SKIN_EFFECT, GL_LINEAR, GL_CLAMP_TO_EDGE);


    g_ErrorReport.Write(L"> First Load Files OK.\r\n");

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

    wchar_t Text[100];

    g_ServerListManager->LoadServerListScript();

    swprintf(Text, L"Data\\Local\\%s\\Dialog_%s.bmd", g_strSelectedML.c_str(), g_strSelectedML.c_str());
    OpenDialogFile(Text);

    swprintf(Text, L"Data\\Local\\%s\\Item_%s.bmd", g_strSelectedML.c_str(), g_strSelectedML.c_str());
    OpenItemScript(Text);

    swprintf(Text, L"Data\\Local\\%s\\movereq_%s.bmd", g_strSelectedML.c_str(), g_strSelectedML.c_str());
    SEASON3B::CMoveCommandData::OpenMoveReqScript(Text);

    swprintf(Text, L"Data\\Local\\%s\\NpcName_%s.txt", g_strSelectedML.c_str(), g_strSelectedML.c_str());
    OpenMonsterScript(Text);

    swprintf(Text, L"Data\\Local\\%s\\Quest_%s.bmd", g_strSelectedML.c_str(), g_strSelectedML.c_str());
    g_csQuest.OpenQuestScript(Text);

    swprintf(Text, L"Data\\Local\\%s\\Skill_%s.bmd", g_strSelectedML.c_str(), g_strSelectedML.c_str());
    OpenSkillScript(Text);

    swprintf(Text, L"Data\\Local\\%s\\SocketItem_%s.bmd", g_strSelectedML.c_str(), g_strSelectedML.c_str());
    g_SocketItemMgr.OpenSocketItemScript(Text);

    OpenTextData();		//. Text.bmd, Testtest.bmd

    g_csItemOption.OpenItemSetScript();

    g_QuestMng.LoadQuestScript();

    OpenGateScript(L"Data\\Gate.bmd");

    OpenFilterFile(L"Data\\Local\\Filter.bmd");

    OpenNameFilterFile(L"Data\\Local\\FilterName.bmd");

    OpenMonsterSkillScript(L"Data\\Local\\MonsterSkill.bmd");

    g_pMasterLevelInterface->OpenMasterSkillTreeData(L"Data\\Local\\MasterSkillTreeData.bmd");
    g_pMasterLevelInterface->OpenMasterSkillTooltip(L"Data\\Local\\Eng\\MasterSkillTooltip_eng.bmd");

    rUIMng.RenderTitleSceneUI(hDC, 9, 11);

    LoadWaveFile(SOUND_TITLE01, L"Data\\Sound\\iTitle.wav", 1);
    LoadWaveFile(SOUND_MENU01, L"Data\\Sound\\iButtonMove.wav", 2);
    LoadWaveFile(SOUND_CLICK01, L"Data\\Sound\\iButtonClick.wav", 1);
    LoadWaveFile(SOUND_ERROR01, L"Data\\Sound\\iButtonError.wav", 1);
    LoadWaveFile(SOUND_INTERFACE01, L"Data\\Sound\\iCreateWindow.wav", 1);

    LoadWaveFile(SOUND_REPAIR, L"Data\\Sound\\iRepair.wav", 1);
    LoadWaveFile(SOUND_WHISPER, L"Data\\Sound\\iWhisper.wav", 1);

    LoadWaveFile(SOUND_FRIEND_CHAT_ALERT, L"Data\\Sound\\iFMSGAlert.wav", 1);
    LoadWaveFile(SOUND_FRIEND_MAIL_ALERT, L"Data\\Sound\\iFMailAlert.wav", 1);
    LoadWaveFile(SOUND_FRIEND_LOGIN_ALERT, L"Data\\Sound\\iFLogInAlert.wav", 1);

    LoadWaveFile(SOUND_RING_EVENT_READY, L"Data\\Sound\\iEvent3min.wav", 1);
    LoadWaveFile(SOUND_RING_EVENT_START, L"Data\\Sound\\iEventStart.wav", 1);
    LoadWaveFile(SOUND_RING_EVENT_END, L"Data\\Sound\\iEventEnd.wav", 1);

    rUIMng.RenderTitleSceneUI(hDC, 10, 11);
}

void OpenTextData()
{
    wchar_t Text[100];

    swprintf(Text, L"Data\\Local\\%s\\Text_%s.bmd", g_strSelectedML.c_str(), g_strSelectedML.c_str());
    GlobalText.Load(Text, CGlobalText::LD_USA_CANADA_TEXTS | CGlobalText::LD_FOREIGN_TEXTS);
    OpenMacro(L"Data\\Macro.txt");
}

void ReleaseMainData()
{
    gMapManager.DeleteObjects();
    DeleteNpcs();
    DeleteMonsters();
    ClearItems();
    ClearCharacters();
}