///////////////////////////////////////////////////////////////////////////////
// 케릭터 관련 함수
// 케릭터 랜더링, 움직임등을 처리
//
// *** 함수 레벨: 3
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "_enum.h"
#include <eh.h>
#include "UIManager.h"
#include "GuildCache.h"
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
#include "ZzzOpenData.h"
#include "ZzzScene.h"
#include "DSPlaySound.h"

#include "PhysicsManager.h"
#include "GOBoid.h"
#include "CSItemOption.h"
#include "CSChaosCastle.h"
#include "GIPetManager.h"
#include "CSParts.h"
#include "BoneManager.h"
#include "CDirection.h"
#include "Input.h"
#include "ChangeRingManager.h"
#include "Event.h"
#include "PartyManager.h"
#include "w_CursedTemple.h"
#include "SummonSystem.h"
#include "CharacterManager.h"
#include "SkillManager.h"
#include "w_MapHeaders.h"
#include "w_PetProcess.h"
#include "DuelMgr.h"
#include "MonkSystem.h"
#include <NewUISystem.h>

CHARACTER* CharactersClient;
CHARACTER CharacterView;
CHARACTER* Hero;
Script_Skill MonsterSkill[MODEL_MONSTER_END];
extern CKanturuDirection KanturuDirection;
float g_fBoneSave[10][3][4];
static  wchar_t    vec_list[35] =
{
     5,  6, 33, 53, 35, 49, 50, 45, 46, 41,
    42, 37, 38, 11, 31, 13, 27, 28, 23, 24,
    19, 20, 15, 16, 54, 55, 62, 69, 70, 77,
     2, 79, 81, 84, 86
};
static  wchar_t    wingLeft[15][2] =
{
    {0,2},  {2,3}, {2,4},  {4,5}, {5,6}, {4,7}, {7,8}, {4,9}, {9,10}, {4,11}, {11,12},
    {6,5}, {8,7}, {10,9},{12,11}
};
static  wchar_t    wingRight[15][2] =
{
    {0,13}, {13,14}, {13,15}, {15,16}, {16,17}, {15,18}, {18,19}, {15,20}, {20,21}, {15,22}, {22,23},
    {17,16},{19,18}, {21,20}, {23,22}
};
static  wchar_t    arm_leg_Left[4][2] =
{
    {29,28}, {28,27}, {34,33}, {33,30},
};
static  wchar_t    arm_leg_Right[4][2] =
{
    {26,25}, {25,24}, {32,31}, {31,30},
};

//
static  wchar_t    g_chStar[10] =
{
    10, 18, 37, 38, 51, 52, 58, 59, 66, 24
};

int  EquipmentLevelSet = 0;
bool g_bAddDefense = false;
int g_iLimitAttackTime = 15;
static  int     g_iOldPositionX = 0;
static  int     g_iOldPositionY = 0;
static  float   g_fStopTime = 0.f;

void RegisterBuff(eBuffState buff, OBJECT* o, const int bufftime = 0);
void UnRegisterBuff(eBuffState buff, OBJECT* o);

int GetFenrirType(CHARACTER* c)
{
    if (c->Helper.ExcellentFlags == 0x01)
        return FENRIR_TYPE_BLACK;
    else if (c->Helper.ExcellentFlags == 0x02)
        return FENRIR_TYPE_BLUE;
    else if (c->Helper.ExcellentFlags == 0x04)
        return FENRIR_TYPE_GOLD;

    return FENRIR_TYPE_RED;
}

void FallingMonster(CHARACTER* c, OBJECT* o)
{
    float AngleY;
    o->Gravity += 2.5f;
    o->Angle[0] -= 4.f;
    o->m_bActionStart = true;
    o->Direction[1] += o->Direction[0];

    if (o->Gravity >= 1.f)
        AngleY = o->Angle[2];

    if (o->Angle[0] <= -90.0f)
        o->Angle[0] = -90.0f;

    o->Angle[2] = AngleY;

    if (o->Gravity >= 150.0f)
        o->Gravity = 150.0f;

    o->Position[2] -= o->Gravity;
}

BOOL PlayMonsterSoundGlobal(OBJECT* pObject);

BOOL PlayMonsterSound(OBJECT* pObject)
{
    if (PlayMonsterSoundGlobal(pObject))
    {
        return TRUE;
    }
    if (SEASON3B::GMNewTown::IsCurrentMap())
    {
        SEASON3B::GMNewTown::PlayMonsterSound(pObject);
        return TRUE;
    }
    if (SEASON3C::GMSwampOfQuiet::IsCurrentMap())
    {
        SEASON3C::GMSwampOfQuiet::PlayMonsterSound(pObject);
        return TRUE;
    }
    if (TheMapProcess().PlayMonsterSound(pObject) == true)
    {
        return TRUE;
    }
    return FALSE;
}

void SetPlayerStop(CHARACTER* c)
{
    c->Run = 0;

    OBJECT* o = &c->Object;
    if (c->Object.Type == MODEL_PLAYER)
    {
        if (c->Helper.Type == MODEL_HORN_OF_FENRIR && !c->SafeZone)
        {
            if (gCharacterManager.GetBaseClass(c->Class) == CLASS_RAGEFIGHTER)//레이지파이터이면
            {
                if (c->Weapon[0].Type != -1 && c->Weapon[1].Type != -1)
                    SetAction(&c->Object, PLAYER_RAGE_FENRIR_STAND_TWO_SWORD);
                else if (c->Weapon[0].Type != -1 && c->Weapon[1].Type == -1)
                    SetAction(&c->Object, PLAYER_RAGE_FENRIR_STAND_ONE_RIGHT);
                else if (c->Weapon[0].Type == -1 && c->Weapon[1].Type != -1)
                    SetAction(&c->Object, PLAYER_RAGE_FENRIR_STAND_ONE_LEFT);
                else
                    SetAction(&c->Object, PLAYER_RAGE_FENRIR_STAND);
            }
            else
            {
                if (c->Weapon[0].Type != -1 && c->Weapon[1].Type != -1)	// 양손무기
                    SetAction(&c->Object, PLAYER_FENRIR_STAND_TWO_SWORD);
                else if (c->Weapon[0].Type != -1 && c->Weapon[1].Type == -1) // 오른손 무기
                    SetAction(&c->Object, PLAYER_FENRIR_STAND_ONE_RIGHT);
                else if (c->Weapon[0].Type == -1 && c->Weapon[1].Type != -1) // 왼손 무기
                    SetAction(&c->Object, PLAYER_FENRIR_STAND_ONE_LEFT);
                else	// 맨손
                    SetAction(&c->Object, PLAYER_FENRIR_STAND);
            }
        }
        else if (c->Helper.Type == MODEL_DARK_HORSE_ITEM && !c->SafeZone)
        {
            if (c->Weapon[0].Type == -1 && c->Weapon[1].Type == -1)
                SetAction(&c->Object, PLAYER_STOP_RIDE_HORSE);
            else
                SetAction(&c->Object, PLAYER_STOP_RIDE_HORSE);
        }
        else if (c->SafeZone && c->m_PetInfo->m_dwPetType == PET_TYPE_DARK_SPIRIT && !gMapManager.InChaosCastle())
        {
            SetAction(&c->Object, PLAYER_DARKLORD_STAND);
        }
        else
            if ((c->Helper.Type == MODEL_HORN_OF_UNIRIA || c->Helper.Type == MODEL_HORN_OF_DINORANT) && !c->SafeZone)
            {
                if (c->Weapon[0].Type == -1 && c->Weapon[1].Type == -1)
                    SetAction(&c->Object, PLAYER_STOP_RIDE);
                else if (gCharacterManager.GetBaseClass(c->Class) == CLASS_RAGEFIGHTER)
                    SetAction(&c->Object, PLAYER_RAGE_UNI_STOP_ONE_RIGHT);
                else
                    SetAction(&c->Object, PLAYER_STOP_RIDE_WEAPON);
            }
            else
            {
                bool Fly = false;

                if (!(c->Object.SubType == MODEL_CURSEDTEMPLE_ALLIED_PLAYER || c->Object.SubType == MODEL_CURSEDTEMPLE_ILLUSION_PLAYER) && !c->SafeZone && c->Wing.Type != -1)
                    Fly = true;

                int Index = TERRAIN_INDEX_REPEAT((int)(c->Object.Position[0] / TERRAIN_SCALE), (int)(c->Object.Position[1] / TERRAIN_SCALE));

                if (SceneFlag == MAIN_SCENE && (gMapManager.WorldActive == WD_7ATLANSE || gMapManager.InHellas() || gMapManager.WorldActive == WD_67DOPPLEGANGER3) && (TerrainWall[Index] & TW_SAFEZONE) != TW_SAFEZONE)
                    Fly = true;

                if (c->MonsterIndex == MONSTER_ELF_SOLDIER)
                    Fly = true;

                if (Fly)
                {
                    if (g_isCharacterBuff((&c->Object), eBuff_CrywolfHeroContracted))
                    {
                        if (c->Object.CurrentAction != PLAYER_HEALING_FEMALE1)
                        {
                            SetAction(&c->Object, PLAYER_HEALING_FEMALE1);
                            SendRequestAction(c->Object, AT_HEALING1);
                        }
                    }
                    else
                    {
                        if (gCharacterManager.GetEquipedBowType(c) == BOWTYPE_CROSSBOW)
                        {
                            SetAction(&c->Object, PLAYER_STOP_FLY_CROSSBOW);
                        }
                        else
                        {
                            SetAction(&c->Object, PLAYER_STOP_FLY);
                        }
                    }
                }
                else
                {
                    if (g_isCharacterBuff((&c->Object), eBuff_CrywolfHeroContracted))
                    {
                        if (c->Object.CurrentAction != PLAYER_HEALING_FEMALE1)
                        {
                            SetAction(&c->Object, PLAYER_HEALING_FEMALE1);
                            SendRequestAction(c->Object, AT_HEALING1);
                        }
                    }
                    else
                    {
                        //  No weapons or. Blur when you are in a safe zone that is not a hunting ground.
                        if ((c->Weapon[0].Type == -1 && c->Weapon[1].Type == -1) || (c->SafeZone && (gMapManager.InBloodCastle() == false)))
                        {
                            if (gCharacterManager.GetBaseClass(c->Class) == CLASS_ELF)
                                SetAction(&c->Object, PLAYER_STOP_FEMALE);
                            else if (gCharacterManager.GetBaseClass(c->Class) == CLASS_SUMMONER && !gMapManager.InChaosCastle())
                                SetAction(&c->Object, PLAYER_STOP_SUMMONER);
                            else if (gCharacterManager.GetBaseClass(c->Class) == CLASS_RAGEFIGHTER)
                                SetAction(&c->Object, PLAYER_STOP_RAGEFIGHTER);
                            else
                                SetAction(&c->Object, PLAYER_STOP_MALE);
                        }
                        else
                        {
                            //  칼 장착.
                            if (c->Weapon[0].Type >= MODEL_SWORD && c->Weapon[0].Type < MODEL_MACE + MAX_ITEM_INDEX)
                            {
                                if (!ItemAttribute[c->Weapon[0].Type - MODEL_ITEM].TwoHand)
                                {
                                    SetAction(&c->Object, PLAYER_STOP_SWORD);
                                }
                                else if (c->Weapon[0].Type == MODEL_DARK_REIGN_BLADE || c->Weapon[0].Type == MODEL_RUNE_BLADE || c->Weapon[0].Type == MODEL_EXPLOSION_BLADE || c->Weapon[0].Type == MODEL_SWORD_DANCER)
                                {
                                    SetAction(&c->Object, PLAYER_STOP_TWO_HAND_SWORD_TWO);
                                }
                                else
                                {
                                    SetAction(&c->Object, PLAYER_STOP_TWO_HAND_SWORD);
                                }
                            }
                            //  창 장착.
                            else if (c->Weapon[0].Type == MODEL__SPEAR || c->Weapon[0].Type == MODEL_DRAGON_LANCE)
                            {
                                SetAction(&c->Object, PLAYER_STOP_SPEAR);
                            }
                            //  창 장착.
                            else if (c->Weapon[0].Type >= MODEL_SPEAR && c->Weapon[0].Type < MODEL_SPEAR + MAX_ITEM_INDEX)
                            {
                                if (!ItemAttribute[c->Weapon[0].Type - MODEL_ITEM].TwoHand)
                                    SetAction(&c->Object, PLAYER_STOP_SWORD);
                                else
                                    SetAction(&c->Object, PLAYER_STOP_SCYTHE);
                            }
                            // 소환술사 스틱.
                            else if (c->Weapon[0].Type >= MODEL_MISTERY_STICK && c->Weapon[0].Type <= MODEL_ETERNAL_WING_STICK)
                            {
                                ::SetAction(&c->Object, PLAYER_STOP_WAND);
                            }
                            else if (c->Weapon[0].Type >= MODEL_STAFF && c->Weapon[0].Type < MODEL_STAFF + MAX_ITEM_INDEX)
                            {
                                if (!ItemAttribute[c->Weapon[0].Type - MODEL_ITEM].TwoHand)
                                    SetAction(&c->Object, PLAYER_STOP_SWORD);
                                else
                                    SetAction(&c->Object, PLAYER_STOP_SCYTHE);
                            }
                            else if (gCharacterManager.GetEquipedBowType(c) == BOWTYPE_BOW)
                            {
                                SetAction(&c->Object, PLAYER_STOP_BOW);
                            }
                            else if (gCharacterManager.GetEquipedBowType(c) == BOWTYPE_CROSSBOW)
                            {
                                SetAction(&c->Object, PLAYER_STOP_CROSSBOW);
                            }
                            else
                            {
                                if (gCharacterManager.GetBaseClass(c->Class) == CLASS_ELF)
                                    SetAction(&c->Object, PLAYER_STOP_FEMALE);
                                else if (gCharacterManager.GetBaseClass(c->Class) == CLASS_SUMMONER)
                                    SetAction(&c->Object, PLAYER_STOP_SUMMONER);
                                else if (gCharacterManager.GetBaseClass(c->Class) == CLASS_RAGEFIGHTER)
                                    SetAction(&c->Object, PLAYER_STOP_RAGEFIGHTER);
                                else
                                    SetAction(&c->Object, PLAYER_STOP_MALE);
                            }
                        }
                    }
                }
            }
    }
    else
    {
        int Index = TERRAIN_INDEX_REPEAT((c->PositionX), (c->PositionY));
        if (o->Type == MODEL_BALI && (TerrainWall[Index] & TW_SAFEZONE) == TW_SAFEZONE)//발리
            SetAction(&c->Object, MONSTER01_APEAR);
        else
            SetAction(&c->Object, MONSTER01_STOP1);
    }

    if (rand_fps_check(16))
    {
        if (o->Type != MODEL_PLAYER || (o->SubType >= MODEL_SKELETON1 && o->SubType <= MODEL_SKELETON3))
        {
            if (Models[o->Type].Sounds[0] != -1)
            {
                int offset = 0;
                if (o->SubType == 9)
                {
                    offset = 5;
                }

                PlayBuffer(static_cast<ESound>(SOUND_MONSTER + offset + Models[o->Type].Sounds[rand() % 2]), o);
            }
        }
        else if (c->Helper.Type == MODEL_HORN_OF_FENRIR)
        {
            if (rand_fps_check(3))
            {
                PlayBuffer(static_cast<ESound>(SOUND_FENRIR_IDLE_1 + rand() % 2), o);
            }
        }
    }
}

void SetPlayerWalk(CHARACTER* c)
{
    if (c->SafeZone)
    {
        c->Run = 0;
    }

    ITEM* pItemBoots = &CharacterMachine->Equipment[EQUIPMENT_BOOTS];
    ITEM* pItemGloves = &CharacterMachine->Equipment[EQUIPMENT_GLOVES];

    if (c->MonsterIndex >= MONSTER_DOPPELGANGER_ELF && c->MonsterIndex <= MONSTER_DOPPELGANGER_SUM)
    {
        c->Run = 0;
    }
    else
        if (!c->SafeZone && c->Run < 40)
        {
            if (gCharacterManager.GetBaseClass(c->Class) == CLASS_DARK || gCharacterManager.GetBaseClass(c->Class) == CLASS_DARK_LORD
                || gCharacterManager.GetBaseClass(c->Class) == CLASS_RAGEFIGHTER
                || ((gMapManager.WorldActive != WD_7ATLANSE && !gMapManager.InHellas() && gMapManager.WorldActive != WD_67DOPPLEGANGER3) && c->BodyPart[BODYPART_BOOTS].Type != -1 && c->BodyPart[BODYPART_BOOTS].Level >= 5)
                || ((gMapManager.WorldActive == WD_7ATLANSE || gMapManager.InHellas()
                    || gMapManager.WorldActive == WD_67DOPPLEGANGER3
                    ) && c->BodyPart[BODYPART_GLOVES].Type != -1 && c->BodyPart[BODYPART_GLOVES].Level >= 5)
                || ((gMapManager.WorldActive != WD_7ATLANSE && !gMapManager.InHellas()
                    && gMapManager.WorldActive != WD_67DOPPLEGANGER3
                    ) && pItemBoots->Level >= 5)
                || ((gMapManager.WorldActive == WD_7ATLANSE || gMapManager.InHellas()
                    || gMapManager.WorldActive == WD_67DOPPLEGANGER3
                    ) && pItemGloves->Level >= 5)
                || c->Helper.Type == MODEL_HORN_OF_FENRIR
                || c->Object.SubType == MODEL_CURSEDTEMPLE_ALLIED_PLAYER
                || c->Object.SubType == MODEL_CURSEDTEMPLE_ILLUSION_PLAYER)
            {
                // RUN are the number of frames which are required until running gets active
                c->Run += FPS_ANIMATION_FACTOR;
            }
        }
    OBJECT* o = &c->Object;
    if (c->Object.Type == MODEL_PLAYER)
    {
        for (int i = PLAYER_WALK_MALE; i <= PLAYER_WALK_CROSSBOW; i++)
        {
            if (gCharacterManager.GetBaseClass(c->Class) == CLASS_RAGEFIGHTER)
                Models[MODEL_PLAYER].Actions[i].PlaySpeed = 0.32f;
            else
                Models[MODEL_PLAYER].Actions[i].PlaySpeed = 0.33f;

            if (g_isCharacterBuff(o, eDeBuff_Freeze))
            {
                Models[MODEL_PLAYER].Actions[i].PlaySpeed *= 0.5f;
            }
            else if (g_isCharacterBuff(o, eDeBuff_BlowOfDestruction))
            {
                Models[MODEL_PLAYER].Actions[i].PlaySpeed *= 0.33f;
            }
        }

        for (int i = PLAYER_RUN; i <= PLAYER_RUN_RIDE_WEAPON; i++)
        {
            if (gCharacterManager.GetBaseClass(c->Class) == CLASS_RAGEFIGHTER)
                Models[MODEL_PLAYER].Actions[i].PlaySpeed = 0.28f;
            else
                Models[MODEL_PLAYER].Actions[i].PlaySpeed = 0.34f;

            Models[MODEL_PLAYER].Actions[PLAYER_RAGE_UNI_RUN].PlaySpeed = 0.34f;
            Models[MODEL_PLAYER].Actions[PLAYER_RAGE_UNI_RUN_ONE_RIGHT].PlaySpeed = 0.34f;

            if (g_isCharacterBuff(o, eDeBuff_Freeze))
            {
                Models[MODEL_PLAYER].Actions[i].PlaySpeed *= 0.5f;
            }
            else if (g_isCharacterBuff(o, eDeBuff_BlowOfDestruction))
            {
                Models[MODEL_PLAYER].Actions[i].PlaySpeed *= 0.33f;
            }
        }

        Models[MODEL_PLAYER].Actions[PLAYER_CHANGE_UP].PlaySpeed = 0.049f;
        Models[MODEL_PLAYER].Actions[PLAYER_RUN_RIDE_HORSE].PlaySpeed = 0.33f;
        Models[MODEL_PLAYER].Actions[PLAYER_DARKLORD_WALK].PlaySpeed = 0.33f;
        Models[MODEL_PLAYER].Actions[PLAYER_WALK_WAND].PlaySpeed = 0.44f;
        Models[MODEL_PLAYER].Actions[PLAYER_RUN_WAND].PlaySpeed = 0.76f;

        if (g_isCharacterBuff(o, eDeBuff_Freeze))
        {
            Models[MODEL_PLAYER].Actions[PLAYER_RUN_RIDE_HORSE].PlaySpeed *= 0.5f;
            Models[MODEL_PLAYER].Actions[PLAYER_DARKLORD_WALK].PlaySpeed *= 0.5f;
        }
        else if (g_isCharacterBuff(o, eDeBuff_BlowOfDestruction))
        {
            Models[MODEL_PLAYER].Actions[PLAYER_RUN_RIDE_HORSE].PlaySpeed *= 0.33f;
            Models[MODEL_PLAYER].Actions[PLAYER_DARKLORD_WALK].PlaySpeed *= 0.33f;
        }

        if (c->Helper.Type == MODEL_HORN_OF_FENRIR && !c->SafeZone)
        {
            if (c->Run < FENRIR_RUN_DELAY)
            {
                SetAction_Fenrir_Walk(c, &c->Object);
            }
            else
            {
                SetAction_Fenrir_Run(c, &c->Object);
            }
        }
        else if (c->Helper.Type == MODEL_DARK_HORSE_ITEM && !c->SafeZone)
        {
            SetAction(&c->Object, PLAYER_RUN_RIDE_HORSE);
        }
        else if (c->SafeZone && c->m_PetInfo->m_dwPetType == PET_TYPE_DARK_SPIRIT)
        {
            SetAction(&c->Object, PLAYER_DARKLORD_WALK);
        }
        else if (c->Helper.Type == MODEL_HORN_OF_UNIRIA && !c->SafeZone)
        {
            if (gCharacterManager.GetBaseClass(c->Class) == CLASS_RAGEFIGHTER)
            {
                if (c->Weapon[0].Type == -1 && c->Weapon[1].Type == -1)
                    SetAction(&c->Object, PLAYER_RAGE_UNI_RUN);
                else
                    SetAction(&c->Object, PLAYER_RAGE_UNI_RUN_ONE_RIGHT);
            }
            else
            {
                if (c->Weapon[0].Type == -1 && c->Weapon[1].Type == -1)
                    SetAction(&c->Object, PLAYER_RUN_RIDE);
                else
                    SetAction(&c->Object, PLAYER_RUN_RIDE_WEAPON);
            }
        }
        else if (c->Helper.Type == MODEL_HORN_OF_DINORANT && !c->SafeZone)   //  페가시아를 타고있음.
        {
            if (gMapManager.WorldActive == WD_8TARKAN || gMapManager.WorldActive == WD_10HEAVEN || g_Direction.m_CKanturu.IsMayaScene())
            {
                // 애니메이션 튀는거때문에 아예 막아버림
//                if(c->Weapon[0].Type==-1 && c->Weapon[1].Type==-1)
//				    SetAction(&c->Object,PLAYER_FLY_RIDE);
//			    else
//				    SetAction(&c->Object,PLAYER_FLY_RIDE_WEAPON);
            }
            else
            {
                if (gCharacterManager.GetBaseClass(c->Class) == CLASS_RAGEFIGHTER)
                {
                    if (c->Weapon[0].Type == -1 && c->Weapon[1].Type == -1)
                        SetAction(&c->Object, PLAYER_RAGE_UNI_RUN);
                    else
                        SetAction(&c->Object, PLAYER_RAGE_UNI_RUN_ONE_RIGHT);
                }
                else
                {
                    if (c->Weapon[0].Type == -1 && c->Weapon[1].Type == -1)
                        SetAction(&c->Object, PLAYER_RUN_RIDE);
                    else
                        SetAction(&c->Object, PLAYER_RUN_RIDE_WEAPON);
                }
            }
        }
        else
        {
            int Index = TERRAIN_INDEX_REPEAT((int)(c->Object.Position[0] / TERRAIN_SCALE), (int)(c->Object.Position[1] / TERRAIN_SCALE));

            if (!(c->Object.SubType == MODEL_CURSEDTEMPLE_ALLIED_PLAYER || c->Object.SubType == MODEL_CURSEDTEMPLE_ILLUSION_PLAYER) &&
                !c->SafeZone && c->Wing.Type != -1)
            {
                if (gCharacterManager.GetEquipedBowType(c) == BOWTYPE_CROSSBOW)
                    SetAction(&c->Object, PLAYER_FLY_CROSSBOW);
                else
                    SetAction(&c->Object, PLAYER_FLY);
            }
            else if (!c->SafeZone && (gMapManager.WorldActive == WD_7ATLANSE || gMapManager.InHellas() || gMapManager.WorldActive == WD_67DOPPLEGANGER3))
            {
                if (c->Run >= 40)
                    SetAction(&c->Object, PLAYER_RUN_SWIM);
                else
                    SetAction(&c->Object, PLAYER_WALK_SWIM);
            }
            else
            {
                if ((c->Weapon[0].Type == -1 && c->Weapon[1].Type == -1) || (c->SafeZone && (gMapManager.InBloodCastle() == false)))
                {
                    if (c->Run >= 40)
                        SetAction(&c->Object, PLAYER_RUN);
                    else
                    {
                        if (!gCharacterManager.IsFemale(c->Class))
                            SetAction(&c->Object, PLAYER_WALK_MALE);
                        else if (gCharacterManager.GetBaseClass(c->Class) == CLASS_SUMMONER && gMapManager.InChaosCastle())
                            SetAction(&c->Object, PLAYER_WALK_MALE);
                        else
                            SetAction(&c->Object, PLAYER_WALK_FEMALE);
                    }
                }
                else
                {
                    if (c->Run < 40)
                    {
                        if (c->Weapon[0].Type >= MODEL_SWORD && c->Weapon[0].Type < MODEL_MACE + MAX_ITEM_INDEX)
                        {
                            if (!ItemAttribute[c->Weapon[0].Type - MODEL_ITEM].TwoHand)
                            {
                                SetAction(&c->Object, PLAYER_WALK_SWORD);
                            }
                            else if (c->Weapon[0].Type == MODEL_DARK_REIGN_BLADE || c->Weapon[0].Type == MODEL_RUNE_BLADE || c->Weapon[0].Type == MODEL_EXPLOSION_BLADE || c->Weapon[0].Type == MODEL_SWORD_DANCER)
                            {
                                SetAction(&c->Object, PLAYER_WALK_TWO_HAND_SWORD_TWO);
                            }
                            else
                            {
                                SetAction(&c->Object, PLAYER_WALK_TWO_HAND_SWORD);
                            }
                        }
                        else if (c->Weapon[0].Type >= MODEL_MISTERY_STICK && c->Weapon[0].Type <= MODEL_ETERNAL_WING_STICK)
                        {
                            ::SetAction(&c->Object, PLAYER_WALK_WAND);
                        }
                        else if (c->Weapon[0].Type >= MODEL_STAFF && c->Weapon[0].Type < MODEL_STAFF + MAX_ITEM_INDEX)
                        {
                            if (!ItemAttribute[c->Weapon[0].Type - MODEL_ITEM].TwoHand)
                                SetAction(&c->Object, PLAYER_WALK_SWORD);
                            else
                                SetAction(&c->Object, PLAYER_WALK_SCYTHE);
                        }
                        else if (c->Weapon[0].Type == MODEL__SPEAR || c->Weapon[0].Type == MODEL_DRAGON_LANCE + MAX_ITEM_INDEX)
                            SetAction(&c->Object, PLAYER_WALK_SPEAR);
                        else if (c->Weapon[0].Type >= MODEL_SPEAR && c->Weapon[0].Type < MODEL_SPEAR + MAX_ITEM_INDEX)
                            SetAction(&c->Object, PLAYER_WALK_SCYTHE);

                        else if (gCharacterManager.GetEquipedBowType(c) == BOWTYPE_BOW)
                        {
                            SetAction(&c->Object, PLAYER_WALK_BOW);
                        }
                        // 석궁
                        else if (gCharacterManager.GetEquipedBowType(c) == BOWTYPE_CROSSBOW)
                        {
                            SetAction(&c->Object, PLAYER_WALK_CROSSBOW);
                        }
                        else
                        {
                            if (!gCharacterManager.IsFemale(c->Class))
                                SetAction(&c->Object, PLAYER_WALK_MALE);
                            else
                                SetAction(&c->Object, PLAYER_WALK_FEMALE);
                        }
                    }
                    else
                    {
                        if (c->Weapon[0].Type >= MODEL_SWORD && c->Weapon[0].Type < MODEL_MACE + MAX_ITEM_INDEX)
                        {
                            if (c->Weapon[1].Type >= MODEL_SWORD && c->Weapon[1].Type < MODEL_MACE + MAX_ITEM_INDEX)
                            {
                                if (gCharacterManager.GetBaseClass(c->Class) == CLASS_RAGEFIGHTER)
                                {
                                    SetAction(&c->Object, PLAYER_RUN);
                                }
                                else
                                    SetAction(&c->Object, PLAYER_RUN_TWO_SWORD);
                            }
                            else if (!ItemAttribute[c->Weapon[0].Type - MODEL_ITEM].TwoHand)
                            {
                                SetAction(&c->Object, PLAYER_RUN_SWORD);
                            }
                            else if (c->Weapon[0].Type == MODEL_DARK_REIGN_BLADE || c->Weapon[0].Type == MODEL_RUNE_BLADE || c->Weapon[0].Type == MODEL_EXPLOSION_BLADE || c->Weapon[0].Type == MODEL_SWORD_DANCER)
                            {
                                SetAction(&c->Object, PLAYER_RUN_TWO_HAND_SWORD_TWO);
                            }
                            else
                            {
                                SetAction(&c->Object, PLAYER_RUN_TWO_HAND_SWORD);
                            }
                        }
                        else if (c->Weapon[0].Type >= MODEL_MISTERY_STICK && c->Weapon[0].Type <= MODEL_ETERNAL_WING_STICK)
                        {
                            ::SetAction(&c->Object, PLAYER_RUN_WAND);
                        }
                        else if (c->Weapon[0].Type >= MODEL_STAFF && c->Weapon[0].Type < MODEL_STAFF + MAX_ITEM_INDEX)
                        {
                            if (!ItemAttribute[c->Weapon[0].Type - MODEL_ITEM].TwoHand)
                                SetAction(&c->Object, PLAYER_RUN_SWORD);
                            else
                                SetAction(&c->Object, PLAYER_RUN_SPEAR);
                        }
                        else if (c->Weapon[0].Type >= MODEL_SPEAR && c->Weapon[0].Type < MODEL_SPEAR + MAX_ITEM_INDEX)
                        {
                            SetAction(&c->Object, PLAYER_RUN_SPEAR);
                        }
                        else if (gCharacterManager.GetEquipedBowType(c) == BOWTYPE_BOW)
                        {
                            SetAction(&c->Object, PLAYER_RUN_BOW);
                        }
                        else if (gCharacterManager.GetEquipedBowType(c) == BOWTYPE_CROSSBOW)
                        {
                            SetAction(&c->Object, PLAYER_RUN_CROSSBOW);
                        }
                        else
                        {
                            SetAction(&c->Object, PLAYER_RUN);
                        }
                    }
                }
            }
        }
    }
    else
    {
        SetAction(&c->Object, MONSTER01_WALK);
    }
    PlayMonsterSound(o);
    if (o->Type == MODEL_BALROG)
        PlayBuffer(SOUND_BONE2, o);
    else if (gCharacterManager.GetBaseClass(c->Class) == CLASS_DARK_LORD && c->Helper.Type == MODEL_DARK_HORSE_ITEM && !c->SafeZone)
    {
        PlayBuffer(static_cast<ESound>(SOUND_RUN_DARK_HORSE_1 + rand() % 3), o);
    }
    else if (c->Helper.Type == MODEL_HORN_OF_FENRIR && !c->SafeZone
        && (c->Object.CurrentAction >= PLAYER_FENRIR_RUN && c->Object.CurrentAction <= PLAYER_FENRIR_RUN_ONE_LEFT_ELF))
    {
        PlayBuffer(static_cast<ESound>(SOUND_FENRIR_RUN_1 + rand() % 3), o);
    }
    else if (c->Helper.Type == MODEL_HORN_OF_FENRIR && !c->SafeZone
        && (c->Object.CurrentAction >= PLAYER_FENRIR_WALK && c->Object.CurrentAction <= PLAYER_FENRIR_WALK_ONE_LEFT))
    {
        PlayBuffer(static_cast<ESound>(SOUND_FENRIR_RUN_1 + rand() % 2), o);
    }
    else if (c->Helper.Type == MODEL_HORN_OF_FENRIR && !c->SafeZone
        && (c->Object.CurrentAction >= PLAYER_RAGE_FENRIR_RUN && c->Object.CurrentAction <= PLAYER_RAGE_FENRIR_RUN_ONE_LEFT))
    {
        PlayBuffer(static_cast<ESound>(SOUND_FENRIR_RUN_1 + rand() % 3), o);
    }
    else if (c->Helper.Type == MODEL_HORN_OF_FENRIR && !c->SafeZone
        && (c->Object.CurrentAction >= PLAYER_RAGE_FENRIR_WALK && c->Object.CurrentAction <= PLAYER_RAGE_FENRIR_WALK_TWO_SWORD))
    {
        PlayBuffer(static_cast<ESound>(SOUND_FENRIR_RUN_1 + rand() % 2), o);
    }
    else if ((c == Hero && rand_fps_check(64)) || (c != Hero && rand_fps_check(16)))
    {
        if (o->Type != MODEL_PLAYER || (o->SubType >= MODEL_SKELETON1 && o->SubType <= MODEL_SKELETON3))
        {
            if (o->SubType >= MODEL_SKELETON1 && o->SubType <= MODEL_SKELETON3)
                PlayBuffer(SOUND_BONE1, o);
            if (Models[o->Type].Sounds[0] != -1)
            {
                int offset = 0;
                if (o->SubType == 9)
                {
                    offset = 5;
                }
                PlayBuffer(static_cast<ESound>(SOUND_MONSTER + offset + Models[o->Type].Sounds[rand() % 2]), o);
            }
        }
    }
}

//extern bool EnableEdit;
extern int CurrentSkill;
int AttackHand = 0;

#define RGZ_FIX_ATTACK_SPEED

void SetAttackSpeed()
{
#ifndef RGZ_FIX_ATTACK_SPEED
    float AttackSpeed1 = CharacterAttribute->AttackSpeed * 0.004f;
    float MagicSpeed1 = CharacterAttribute->MagicSpeed * 0.004f;
    float MagicSpeed2 = CharacterAttribute->MagicSpeed * 0.002f;
#else
    float AttackSpeed1 = CharacterAttribute->AttackSpeed;
    float MagicSpeed1 = CharacterAttribute->MagicSpeed;
    float MagicSpeed2 = CharacterAttribute->MagicSpeed;

    if (CharacterAttribute->AttackSpeed >= 509 && CharacterAttribute->AttackSpeed <= 549)
    {
        AttackSpeed1 = AttackSpeed1 * 0.0026000f;
    }
    else if (CharacterAttribute->AttackSpeed >= 550 && CharacterAttribute->AttackSpeed <= 750)
    {
        AttackSpeed1 = AttackSpeed1 * 0.0017000f;
    }
    else
    {
        AttackSpeed1 = AttackSpeed1 * 0.0040000f;
    }

    if (CharacterAttribute->MagicSpeed >= 509 && CharacterAttribute->MagicSpeed <= 549)
    {
        MagicSpeed1 = MagicSpeed1 * 0.0026000f;
    }
    else if (CharacterAttribute->MagicSpeed >= 550 && CharacterAttribute->MagicSpeed <= 750)
    {
        MagicSpeed1 = MagicSpeed1 * 0.0017000f;
    }
    else
    {
        MagicSpeed1 = MagicSpeed1 * 0.0040000f;
    }

    if (CharacterAttribute->MagicSpeed >= 455 && CharacterAttribute->MagicSpeed <= 479)
    {
        MagicSpeed2 = MagicSpeed2 * 0.0024700f;
    }
    else if (CharacterAttribute->MagicSpeed >= 605 && CharacterAttribute->MagicSpeed <= 636)
    {
        MagicSpeed2 = MagicSpeed2 * 0.0019000f;
    }
    else if (CharacterAttribute->MagicSpeed >= 637 && CharacterAttribute->MagicSpeed <= 668)
    {
        MagicSpeed2 = MagicSpeed2 * 0.0018000f;
    }
    else if (CharacterAttribute->MagicSpeed >= 669 && CharacterAttribute->MagicSpeed <= 688)
    {
        MagicSpeed2 = MagicSpeed2 * 0.0017000f;
    }
    else if (CharacterAttribute->MagicSpeed >= 855 && CharacterAttribute->MagicSpeed <= 1040)
    {
        MagicSpeed2 = MagicSpeed2 * 0.0016300f;
    }
    else if (CharacterAttribute->MagicSpeed >= 1041 && CharacterAttribute->MagicSpeed <= 1104)
    {
        MagicSpeed2 = MagicSpeed2 * 0.0015500f;
    }
    else if (CharacterAttribute->MagicSpeed >= 1301 && CharacterAttribute->MagicSpeed <= 1500)
    {
        MagicSpeed2 = MagicSpeed2 * 0.0017500f;
    }
    else if (CharacterAttribute->MagicSpeed >= 1501 && CharacterAttribute->MagicSpeed <= 1524)
    {
        MagicSpeed2 = MagicSpeed2 * 0.0015000f;
    }
    else if (CharacterAttribute->MagicSpeed >= 1525 && CharacterAttribute->MagicSpeed <= 1800)
    {
        MagicSpeed2 = MagicSpeed2 * 0.0014500f;
    }
    else if (CharacterAttribute->MagicSpeed >= 1801 && CharacterAttribute->MagicSpeed <= 1999)
    {
        MagicSpeed2 = MagicSpeed2 * 0.0013000f;
    }
    else if (CharacterAttribute->MagicSpeed >= 2000 && CharacterAttribute->MagicSpeed <= 2167)
    {
        MagicSpeed2 = MagicSpeed2 * 0.0012500f;
    }
    else if (CharacterAttribute->MagicSpeed >= 2168 && CharacterAttribute->MagicSpeed <= 2354)
    {
        MagicSpeed2 = MagicSpeed2 * 0.0011500f;
    }
    else if (CharacterAttribute->MagicSpeed >= 2855 && CharacterAttribute->MagicSpeed <= 3011)
    {
        MagicSpeed2 = MagicSpeed2 * 0.0009000f;
    }
    else if (CharacterAttribute->MagicSpeed >= 3011)
    {
        MagicSpeed2 = MagicSpeed2 * 0.0008100f;
    }
    else
    {
        MagicSpeed2 = MagicSpeed2 * 0.0020000f;
    }
#endif

    Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_FIST].PlaySpeed = 0.6f + AttackSpeed1;

    for (int i = PLAYER_ATTACK_SWORD_RIGHT1; i <= PLAYER_ATTACK_RIDE_CROSSBOW; i++)
    {
        Models[MODEL_PLAYER].Actions[i].PlaySpeed = 0.25f + AttackSpeed1;
    }

    Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_SKILL_SWORD1].PlaySpeed = 0.30f + AttackSpeed1;
    Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_SKILL_SWORD2].PlaySpeed = 0.30f + AttackSpeed1;
    Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_SKILL_SWORD3].PlaySpeed = 0.27f + AttackSpeed1;
    Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_SKILL_SWORD4].PlaySpeed = 0.30f + AttackSpeed1;
    Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_SKILL_SWORD5].PlaySpeed = 0.24f + AttackSpeed1;
    Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_SKILL_WHEEL].PlaySpeed = 0.24f + AttackSpeed1;
    Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_DEATHSTAB].PlaySpeed = 0.25f + AttackSpeed1;
    Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_SKILL_SPEAR].PlaySpeed = 0.30f + AttackSpeed1;
    Models[MODEL_PLAYER].Actions[PLAYER_SKILL_RIDER].PlaySpeed = 0.3f + AttackSpeed1;
    Models[MODEL_PLAYER].Actions[PLAYER_SKILL_RIDER_FLY].PlaySpeed = 0.3f + AttackSpeed1;

    Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_TWO_HAND_SWORD_TWO].PlaySpeed = 0.25f + AttackSpeed1;

    for (int i = PLAYER_ATTACK_BOW; i <= PLAYER_ATTACK_FLY_CROSSBOW; i++)
        Models[MODEL_PLAYER].Actions[i].PlaySpeed = 0.30f + AttackSpeed1;
    for (int i = PLAYER_ATTACK_RIDE_BOW; i <= PLAYER_ATTACK_RIDE_CROSSBOW; i++)
        Models[MODEL_PLAYER].Actions[i].PlaySpeed = 0.30f + AttackSpeed1;

    Models[MODEL_PLAYER].Actions[PLAYER_SKILL_ELF1].PlaySpeed = 0.25f + MagicSpeed1;

    for (int i = PLAYER_SKILL_HAND1; i <= PLAYER_SKILL_WEAPON2; i++)
        Models[MODEL_PLAYER].Actions[i].PlaySpeed = 0.29f + MagicSpeed2;

    Models[MODEL_PLAYER].Actions[PLAYER_SKILL_TELEPORT].PlaySpeed = 0.30f + MagicSpeed2;
    Models[MODEL_PLAYER].Actions[PLAYER_SKILL_FLASH].PlaySpeed = 0.40f + MagicSpeed2;
    Models[MODEL_PLAYER].Actions[PLAYER_SKILL_INFERNO].PlaySpeed = 0.60f + MagicSpeed2;
    Models[MODEL_PLAYER].Actions[PLAYER_SKILL_HELL].PlaySpeed = 0.50f + MagicSpeed2;
    Models[MODEL_PLAYER].Actions[PLAYER_RIDE_SKILL].PlaySpeed = 0.30f + MagicSpeed2;

    Models[MODEL_PLAYER].Actions[PLAYER_SKILL_HELL_BEGIN].PlaySpeed = 0.50f + MagicSpeed2;
    Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_STRIKE].PlaySpeed = 0.25f + AttackSpeed1;
    Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_RIDE_STRIKE].PlaySpeed = 0.2f + AttackSpeed1;
    Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_RIDE_HORSE_SWORD].PlaySpeed = 0.25f + AttackSpeed1;
    Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_RIDE_ATTACK_FLASH].PlaySpeed = 0.40f + MagicSpeed2;
    Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_RIDE_ATTACK_MAGIC].PlaySpeed = 0.3f + MagicSpeed2;

    Models[MODEL_PLAYER].Actions[PLAYER_FENRIR_ATTACK].PlaySpeed = 0.25f + AttackSpeed1;
    Models[MODEL_PLAYER].Actions[PLAYER_FENRIR_ATTACK_DARKLORD_STRIKE].PlaySpeed = 0.2f + AttackSpeed1;
    Models[MODEL_PLAYER].Actions[PLAYER_FENRIR_ATTACK_DARKLORD_SWORD].PlaySpeed = 0.25f + AttackSpeed1;
    Models[MODEL_PLAYER].Actions[PLAYER_FENRIR_ATTACK_DARKLORD_FLASH].PlaySpeed = 0.40f + MagicSpeed2;
    Models[MODEL_PLAYER].Actions[PLAYER_FENRIR_ATTACK_TWO_SWORD].PlaySpeed = 0.25f + AttackSpeed1;
    Models[MODEL_PLAYER].Actions[PLAYER_FENRIR_ATTACK_MAGIC].PlaySpeed = 0.37f + MagicSpeed2;
    Models[MODEL_PLAYER].Actions[PLAYER_FENRIR_ATTACK_CROSSBOW].PlaySpeed = 0.30f + AttackSpeed1;
    Models[MODEL_PLAYER].Actions[PLAYER_FENRIR_ATTACK_SPEAR].PlaySpeed = 0.25f + AttackSpeed1;
    Models[MODEL_PLAYER].Actions[PLAYER_FENRIR_ATTACK_ONE_SWORD].PlaySpeed = 0.25f + AttackSpeed1;
    Models[MODEL_PLAYER].Actions[PLAYER_FENRIR_ATTACK_BOW].PlaySpeed = 0.30f + AttackSpeed1;

    for (int i = PLAYER_ATTACK_BOW_UP; i <= PLAYER_ATTACK_RIDE_CROSSBOW_UP; ++i)
    {
        Models[MODEL_PLAYER].Actions[i].PlaySpeed = 0.30f + AttackSpeed1;
    }

    Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_ONE_FLASH].PlaySpeed = 0.4f + AttackSpeed1;
    Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_RUSH].PlaySpeed = 0.3f + AttackSpeed1;
    Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_DEATH_CANNON].PlaySpeed = 0.2f + AttackSpeed1;

    Models[MODEL_PLAYER].Actions[PLAYER_SKILL_SLEEP].PlaySpeed = 0.3f + MagicSpeed2;
    Models[MODEL_PLAYER].Actions[PLAYER_SKILL_SLEEP_UNI].PlaySpeed = 0.3f + MagicSpeed2;
    Models[MODEL_PLAYER].Actions[PLAYER_SKILL_SLEEP_DINO].PlaySpeed = 0.3f + MagicSpeed2;
    Models[MODEL_PLAYER].Actions[PLAYER_SKILL_SLEEP_FENRIR].PlaySpeed = 0.3f + MagicSpeed2;

    Models[MODEL_PLAYER].Actions[PLAYER_SKILL_LIGHTNING_ORB].PlaySpeed = 0.4f + MagicSpeed2;
    Models[MODEL_PLAYER].Actions[PLAYER_SKILL_LIGHTNING_ORB_UNI].PlaySpeed = 0.25f + MagicSpeed2;
    Models[MODEL_PLAYER].Actions[PLAYER_SKILL_LIGHTNING_ORB_DINO].PlaySpeed = 0.25f + MagicSpeed2;
    Models[MODEL_PLAYER].Actions[PLAYER_SKILL_LIGHTNING_ORB_FENRIR].PlaySpeed = 0.25f + MagicSpeed2;

    Models[MODEL_PLAYER].Actions[PLAYER_SKILL_CHAIN_LIGHTNING].PlaySpeed = 0.25f + MagicSpeed2;
    Models[MODEL_PLAYER].Actions[PLAYER_SKILL_CHAIN_LIGHTNING_UNI].PlaySpeed = 0.15f + MagicSpeed2;
    Models[MODEL_PLAYER].Actions[PLAYER_SKILL_CHAIN_LIGHTNING_DINO].PlaySpeed = 0.15f + MagicSpeed2;
    Models[MODEL_PLAYER].Actions[PLAYER_SKILL_CHAIN_LIGHTNING_FENRIR].PlaySpeed = 0.15f + MagicSpeed2;

    Models[MODEL_PLAYER].Actions[PLAYER_SKILL_DRAIN_LIFE].PlaySpeed = 0.25f + MagicSpeed2;
    Models[MODEL_PLAYER].Actions[PLAYER_SKILL_DRAIN_LIFE_UNI].PlaySpeed = 0.25f + MagicSpeed2;
    Models[MODEL_PLAYER].Actions[PLAYER_SKILL_DRAIN_LIFE_DINO].PlaySpeed = 0.25f + MagicSpeed2;
    Models[MODEL_PLAYER].Actions[PLAYER_SKILL_DRAIN_LIFE_FENRIR].PlaySpeed = 0.25f + MagicSpeed2;

    Models[MODEL_PLAYER].Actions[PLAYER_SKILL_GIGANTICSTORM].PlaySpeed = 0.55f + MagicSpeed1;
    Models[MODEL_PLAYER].Actions[PLAYER_SKILL_FLAMESTRIKE].PlaySpeed = 0.69f + MagicSpeed2;
    Models[MODEL_PLAYER].Actions[PLAYER_SKILL_LIGHTNING_SHOCK].PlaySpeed = 0.35f + MagicSpeed2;

    Models[MODEL_PLAYER].Actions[PLAYER_SKILL_SUMMON].PlaySpeed = 0.25f;
    Models[MODEL_PLAYER].Actions[PLAYER_SKILL_SUMMON_UNI].PlaySpeed = 0.25f;
    Models[MODEL_PLAYER].Actions[PLAYER_SKILL_SUMMON_DINO].PlaySpeed = 0.25f;
    Models[MODEL_PLAYER].Actions[PLAYER_SKILL_SUMMON_FENRIR].PlaySpeed = 0.25f;

    Models[MODEL_PLAYER].Actions[PLAYER_SKILL_BLOW_OF_DESTRUCTION].PlaySpeed = 0.3f;
    Models[MODEL_PLAYER].Actions[PLAYER_RECOVER_SKILL].PlaySpeed = 0.33f;
    Models[MODEL_PLAYER].Actions[PLAYER_SKILL_SWELL_OF_MP].PlaySpeed = 0.2f;

    Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_SKILL_FURY_STRIKE].PlaySpeed = 0.38f;
    Models[MODEL_PLAYER].Actions[PLAYER_SKILL_VITALITY].PlaySpeed = 0.34f;
    Models[MODEL_PLAYER].Actions[PLAYER_SKILL_HELL_START].PlaySpeed = 0.30f;
    Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_TELEPORT].PlaySpeed = 0.28f;
    Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_RIDE_TELEPORT].PlaySpeed = 0.3f;
    Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_DARKHORSE].PlaySpeed = 0.3f;
    Models[MODEL_PLAYER].Actions[PLAYER_FENRIR_ATTACK_DARKLORD_TELEPORT].PlaySpeed = 0.3f;
    Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_REMOVAL].PlaySpeed = 0.28f;

    float RageAttackSpeed = CharacterAttribute->AttackSpeed * 0.002f;

    Models[MODEL_PLAYER].Actions[PLAYER_SKILL_THRUST].PlaySpeed = 0.4f + RageAttackSpeed;
    Models[MODEL_PLAYER].Actions[PLAYER_SKILL_STAMP].PlaySpeed = 0.4f + RageAttackSpeed;
    Models[MODEL_PLAYER].Actions[PLAYER_SKILL_GIANTSWING].PlaySpeed = 0.4f + RageAttackSpeed;
    Models[MODEL_PLAYER].Actions[PLAYER_SKILL_DARKSIDE_READY].PlaySpeed = 0.3f + RageAttackSpeed;
    Models[MODEL_PLAYER].Actions[PLAYER_SKILL_DARKSIDE_ATTACK].PlaySpeed = 0.3f + RageAttackSpeed;
    Models[MODEL_PLAYER].Actions[PLAYER_SKILL_DRAGONKICK].PlaySpeed = 0.4f + RageAttackSpeed;
    Models[MODEL_PLAYER].Actions[PLAYER_SKILL_DRAGONLORE].PlaySpeed = 0.3f + RageAttackSpeed;
    Models[MODEL_PLAYER].Actions[PLAYER_SKILL_ATT_UP_OURFORCES].PlaySpeed = 0.35f;
    Models[MODEL_PLAYER].Actions[PLAYER_SKILL_HP_UP_OURFORCES].PlaySpeed = 0.35f;
    Models[MODEL_PLAYER].Actions[PLAYER_RAGE_FENRIR_ATTACK_RIGHT].PlaySpeed = 0.25f + RageAttackSpeed;
}

void SetPlayerHighBowAttack(CHARACTER* c)
{
    OBJECT* o = &c->Object;
    if (o->Type == MODEL_PLAYER)
    {
        SetAttackSpeed();
        if ((c->Helper.Type == MODEL_HORN_OF_UNIRIA || c->Helper.Type == MODEL_HORN_OF_DINORANT) && !c->SafeZone)
        {
            if (gCharacterManager.GetEquipedBowType(c) == BOWTYPE_BOW)
            {
                SetAction(&c->Object, PLAYER_ATTACK_RIDE_BOW_UP);
            }
            // 석궁
            else if (gCharacterManager.GetEquipedBowType(c) == BOWTYPE_CROSSBOW)
            {
                SetAction(&c->Object, PLAYER_ATTACK_RIDE_CROSSBOW_UP);
            }
        }
        else
        {
            if (gCharacterManager.GetEquipedBowType(c) == BOWTYPE_BOW)
            {
                if (c->Wing.Type != -1)
                    SetAction(&c->Object, PLAYER_ATTACK_FLY_BOW_UP);
                else
                    SetAction(&c->Object, PLAYER_ATTACK_BOW_UP);
            }
            else if (gCharacterManager.GetEquipedBowType(c) == BOWTYPE_CROSSBOW)
            {
                if (c->Wing.Type != -1)
                    SetAction(&c->Object, PLAYER_ATTACK_FLY_CROSSBOW_UP);
                else
                    SetAction(&c->Object, PLAYER_ATTACK_CROSSBOW_UP);
            }
        }
    }
    c->SwordCount++;
}

void SetPlayerAttack(CHARACTER* c)
{
    OBJECT* o = &c->Object;
    if (o->Type == MODEL_PLAYER)
    {
        SetAttackSpeed();

        if (c->Helper.Type == MODEL_HORN_OF_FENRIR && !c->SafeZone)
        {
            if (c->Weapon[0].Type >= MODEL_SPEAR && c->Weapon[0].Type < MODEL_DOUBLE_POLEAXE)
                SetAction(&c->Object, PLAYER_FENRIR_ATTACK_SPEAR);
            else if (gCharacterManager.GetEquipedBowType(c) == BOWTYPE_BOW)
            {
                SetAction(&c->Object, PLAYER_FENRIR_ATTACK_BOW);
            }
            else if (gCharacterManager.GetEquipedBowType(c) == BOWTYPE_CROSSBOW)
            {
                SetAction(&c->Object, PLAYER_FENRIR_ATTACK_CROSSBOW);	//석궁공격
            }
            else
            {
                if (c->Weapon[0].Type != -1 && c->Weapon[1].Type != -1)
                    SetAction(&c->Object, PLAYER_FENRIR_ATTACK_TWO_SWORD);
                else if (c->Weapon[0].Type != -1 && c->Weapon[1].Type == -1)
                    SetAction(&c->Object, PLAYER_FENRIR_ATTACK_ONE_SWORD);
                else if (c->Weapon[0].Type == -1 && c->Weapon[1].Type != -1 && (gCharacterManager.GetBaseClass(c->Class) == CLASS_RAGEFIGHTER))
                    SetAction(&c->Object, PLAYER_RAGE_FENRIR_ATTACK_RIGHT);
                else if (c->Weapon[0].Type == -1 && c->Weapon[1].Type != -1)
                    SetAction(&c->Object, PLAYER_FENRIR_ATTACK_ONE_SWORD);
                else
                    SetAction(&c->Object, PLAYER_FENRIR_ATTACK);
            }

            if (gCharacterManager.GetBaseClass(c->Class) == CLASS_DARK_LORD)
            {
                SetAction(&c->Object, PLAYER_FENRIR_ATTACK_DARKLORD_SWORD);
            }
        }
        else if (c->Helper.Type == MODEL_DARK_HORSE_ITEM && !c->SafeZone)
        {
            SetAction(&c->Object, PLAYER_ATTACK_RIDE_HORSE_SWORD);
        }
        else if ((c->Helper.Type == MODEL_HORN_OF_UNIRIA || c->Helper.Type == MODEL_HORN_OF_DINORANT) && !c->SafeZone)
        {
            if (c->Weapon[0].Type >= MODEL_SPEAR && c->Weapon[0].Type < MODEL_DOUBLE_POLEAXE)
                SetAction(&c->Object, PLAYER_ATTACK_RIDE_SPEAR);
            else if (c->Weapon[0].Type >= MODEL_DOUBLE_POLEAXE && c->Weapon[0].Type < MODEL_SPEAR + MAX_ITEM_INDEX)
                SetAction(&c->Object, PLAYER_ATTACK_RIDE_SCYTHE);
            else if (gCharacterManager.GetEquipedBowType(c) == BOWTYPE_BOW)
            {
                SetAction(&c->Object, PLAYER_ATTACK_RIDE_BOW);
            }
            else if (gCharacterManager.GetEquipedBowType(c) == BOWTYPE_CROSSBOW)
            {
                SetAction(&c->Object, PLAYER_ATTACK_RIDE_CROSSBOW);
            }
            else
            {
                if (c->Weapon[0].Type == -1)
                {
                    if (gCharacterManager.GetBaseClass(c->Class) == CLASS_RAGEFIGHTER)
                        SetAction(&c->Object, PLAYER_RAGE_UNI_ATTACK);
                    else
                        SetAction(&c->Object, PLAYER_ATTACK_RIDE_SWORD);
                }
                else
                {
                    if (gCharacterManager.GetBaseClass(c->Class) == CLASS_RAGEFIGHTER)
                    {
                        if (!ItemAttribute[c->Weapon[0].Type - MODEL_ITEM].TwoHand)
                            SetAction(&c->Object, PLAYER_RAGE_UNI_ATTACK);
                        else
                            SetAction(&c->Object, PLAYER_RAGE_UNI_ATTACK_ONE_RIGHT);
                    }
                    else
                    {
                        if (!ItemAttribute[c->Weapon[0].Type - MODEL_ITEM].TwoHand)
                            SetAction(&c->Object, PLAYER_ATTACK_RIDE_SWORD);
                        else
                            SetAction(&c->Object, PLAYER_ATTACK_RIDE_TWO_HAND_SWORD);
                    }
                }
            }
        }
        else
        {
            if (c->Weapon[0].Type == -1 && c->Weapon[1].Type == -1)
            {
                SetAction(&c->Object, PLAYER_ATTACK_FIST);
            }
            else
            {
                if (c->Weapon[0].Type >= MODEL_SWORD && c->Weapon[0].Type < MODEL_MACE + MAX_ITEM_INDEX)
                {
                    if (!ItemAttribute[c->Weapon[0].Type - MODEL_ITEM].TwoHand)
                    {
                        if (c->Weapon[1].Type >= MODEL_SWORD && c->Weapon[1].Type < MODEL_MACE + MAX_ITEM_INDEX)
                        {
                            switch (c->SwordCount % 4)
                            {
                            case 0:SetAction(&c->Object, PLAYER_ATTACK_SWORD_RIGHT1); break;
                            case 1:SetAction(&c->Object, PLAYER_ATTACK_SWORD_LEFT1); break;
                            case 2:SetAction(&c->Object, PLAYER_ATTACK_SWORD_RIGHT1 + 1); break;
                            case 3:SetAction(&c->Object, PLAYER_ATTACK_SWORD_LEFT1 + 1); break;
                            }
                        }
                        else
                            SetAction(&c->Object, PLAYER_ATTACK_SWORD_RIGHT1 + c->SwordCount % 2);
                    }
                    else if (c->Weapon[0].Type == MODEL_DARK_REIGN_BLADE || c->Weapon[0].Type == MODEL_RUNE_BLADE || c->Weapon[0].Type == MODEL_EXPLOSION_BLADE || c->Weapon[0].Type == MODEL_SWORD_DANCER)
                    {
                        SetAction(&c->Object, PLAYER_ATTACK_TWO_HAND_SWORD_TWO);
                    }
                    else
                    {
                        SetAction(&c->Object, PLAYER_ATTACK_TWO_HAND_SWORD1 + c->SwordCount % 3);
                    }
                }
                else if (c->Weapon[1].Type >= MODEL_SWORD && c->Weapon[1].Type < MODEL_MACE + MAX_ITEM_INDEX)
                {
                    SetAction(&c->Object, PLAYER_ATTACK_SWORD_LEFT1 + rand() % 2);
                }
                else if (c->Weapon[0].Type >= MODEL_STAFF && c->Weapon[0].Type < MODEL_STAFF + MAX_ITEM_INDEX)
                {
                    if (!ItemAttribute[c->Weapon[0].Type - MODEL_ITEM].TwoHand)
                        SetAction(&c->Object, PLAYER_ATTACK_SWORD_RIGHT1 + rand() % 2);
                    else
                        SetAction(&c->Object, PLAYER_SKILL_WEAPON1 + rand() % 2);
                }
                else if (c->Weapon[0].Type == MODEL__SPEAR || c->Weapon[0].Type == MODEL_DRAGON_LANCE)
                    SetAction(&c->Object, PLAYER_ATTACK_SPEAR1);
                else if (c->Weapon[0].Type >= MODEL_SPEAR && c->Weapon[0].Type < MODEL_SPEAR + MAX_ITEM_INDEX)
                    SetAction(&c->Object, PLAYER_ATTACK_SCYTHE1 + c->SwordCount % 3);
                else if (gCharacterManager.GetEquipedBowType(c) == BOWTYPE_BOW)
                {
                    if (c->Wing.Type != -1)
                        SetAction(&c->Object, PLAYER_ATTACK_FLY_BOW);
                    else
                        SetAction(&c->Object, PLAYER_ATTACK_BOW);
                }
                else if (gCharacterManager.GetEquipedBowType(c) == BOWTYPE_CROSSBOW)
                {
                    if (c->Wing.Type != -1)
                        SetAction(&c->Object, PLAYER_ATTACK_FLY_CROSSBOW);
                    else
                        SetAction(&c->Object, PLAYER_ATTACK_CROSSBOW);
                }
                else
                    SetAction(&c->Object, PLAYER_ATTACK_FIST);
            }
        }
    }

    else if (o->Type == 39)
    {
        CreateEffect(MODEL_SAW, o->Position, o->Angle, o->Light);
        PlayBuffer(SOUND_TRAP01, o);
    }
    else if (o->Type == 40)
    {
        SetAction(&c->Object, 1);
        PlayBuffer(SOUND_TRAP01, o);
    }
    else if (o->Type == 51)
    {
        CreateEffect(BITMAP_FIRE + 1, o->Position, o->Angle, o->Light);
        PlayBuffer(SOUND_FLAME);
    }
    else
    {
        if (o->Type == MODEL_BALI)
        {
            int Action = rand() % 8;
            if (Action > 2)
                SetAction(&c->Object, MONSTER01_ATTACK1 + rand() % 2);
            else if (Action > 0)
                SetAction(&c->Object, MONSTER01_ATTACK3);
            else
                SetAction(&c->Object, MONSTER01_ATTACK4);
        }
        else
        {
            bool Success = true;

            if (SetCurrentAction_HellasMonster(c, o) == true) Success = false;
            if (battleCastle::SetCurrentAction_BattleCastleMonster(c, o) == true) Success = false;
            if (M31HuntingGround::SetCurrentActionHuntingGroundMonster(c, o) == true) Success = false;
            if (M33Aida::SetCurrentActionAidaMonster(c, o) == true) Success = false;
            if (M34CryWolf1st::SetCurrentActionCrywolfMonster(c, o) == true) Success = false;
            if (M37Kanturu1st::SetCurrentActionKanturu1stMonster(c, o) == true) Success = false;
            if (M38Kanturu2nd::Set_CurrentAction_Kanturu2nd_Monster(c, o) == true) Success = false;
            if (M39Kanturu3rd::SetCurrentActionKanturu3rdMonster(c, o) == true) Success = false;
            if (SEASON3A::CGM3rdChangeUp::Instance().SetCurrentActionBalgasBarrackMonster(c, o))	Success = false;
            if (g_CursedTemple->SetCurrentActionMonster(c, o) == true) Success = false;
            if (SEASON3B::GMNewTown::SetCurrentActionMonster(c, o) == true) Success = false;
            if (SEASON3C::GMSwampOfQuiet::SetCurrentActionMonster(c, o) == true) Success = false;

            if (TheMapProcess().SetCurrentActionMonster(c, o) == true) Success = false;

            if (Success)
            {
                if (c->SwordCount % 3 == 0)
                    SetAction(&c->Object, MONSTER01_ATTACK1);
                else
                    SetAction(&c->Object, MONSTER01_ATTACK2);
                c->SwordCount++;
            }
        }
    }
    if (gMapManager.WorldActive == WD_73NEW_LOGIN_SCENE);
    else
        if (c->Object.AnimationFrame == 0.f)
        {
            PlayMonsterSound(o);

            if (o->Type != MODEL_PLAYER || (o->SubType >= MODEL_SKELETON1 && o->SubType <= MODEL_SKELETON3))
            {
                if (o->SubType >= MODEL_SKELETON1 && o->SubType <= MODEL_SKELETON3)
                    PlayBuffer(static_cast<ESound>(SOUND_BRANDISH_SWORD01 + rand() % 2), o);
                if (Models[o->Type].Sounds[2] != -1)
                {
                    int offset = 0;
                    if (o->SubType == 9)
                    {
                        offset = 5;
                    }
                    PlayBuffer(static_cast<ESound>(SOUND_MONSTER + offset + Models[o->Type].Sounds[2 + rand() % 2]), o);
                }
            }
            else
            {
                if (gCharacterManager.GetEquipedBowType(c) == BOWTYPE_BOW)
                {
                    PlayBuffer(SOUND_BOW01, o);
                }
                else if (gCharacterManager.GetEquipedBowType(c) == BOWTYPE_CROSSBOW)
                {
                    PlayBuffer(SOUND_CROSSBOW01, o);
                }
                else if (c->Weapon[0].Type >= MODEL_BLUEWING_CROSSBOW && c->Weapon[0].Type < MODEL_ARROWS)
                    PlayBuffer(SOUND_MAGIC, o);
                else if (c->Weapon[0].Type == MODEL_LIGHT_SABER || c->Weapon[0].Type == MODEL_SPEAR)
                    PlayBuffer(SOUND_BRANDISH_SWORD03, o);
                else if (c->Weapon[0].Type != -1 || c->Weapon[1].Type != -1)
                    PlayBuffer(static_cast<ESound>(SOUND_BRANDISH_SWORD01 + rand() % 2), o);
            }
        }
    c->SwordCount++;
}

void SetPlayerMagic(CHARACTER* c)
{
    OBJECT* o = &c->Object;
    if (o->Type == MODEL_PLAYER)
    {
        SetAttackSpeed();
        if ((c->Helper.Type == MODEL_HORN_OF_UNIRIA || c->Helper.Type == MODEL_HORN_OF_DINORANT) && !c->SafeZone)
        {
            SetAction(o, PLAYER_RIDE_SKILL);
        }
        else if (c->Helper.Type == MODEL_HORN_OF_FENRIR && !c->SafeZone)
        {
            SetAction(o, PLAYER_FENRIR_ATTACK_MAGIC);
        }
        else
        {
            if (gCharacterManager.IsFemale(c->Class))
                SetAction(o, PLAYER_SKILL_ELF1);
            else
                SetAction(o, PLAYER_SKILL_HAND1 + rand() % 2);
        }
    }
    else
    {
        if (c->SwordCount % 3 == 0)
            SetAction(&c->Object, MONSTER01_ATTACK1);
        else
            SetAction(&c->Object, MONSTER01_ATTACK2);
        c->SwordCount++;
    }
}

void SetPlayerTeleport(CHARACTER* c)
{
    OBJECT* o = &c->Object;
    if (o->Type == MODEL_PLAYER)
    {
        SetAction(o, PLAYER_SKILL_TELEPORT);
    }
    else
    {
        SetAction(o, MONSTER01_SHOCK);
    }
}

void SetPlayerShock(CHARACTER* c, int Hit)
{
    if (c->Dead > 0) return;
    if (c->Helper.Type == MODEL_HORN_OF_UNIRIA || c->Helper.Type == MODEL_HORN_OF_DINORANT) return;
    if (c->Helper.Type == MODEL_DARK_HORSE_ITEM) return;

    OBJECT* o = &c->Object;

    if (o->CurrentAction == PLAYER_ATTACK_SKILL_FURY_STRIKE || o->CurrentAction == PLAYER_SKILL_VITALITY) return;
    if (o->CurrentAction == PLAYER_SKILL_HELL_BEGIN
        || (o->CurrentAction == PLAYER_SKILL_ATT_UP_OURFORCES || o->CurrentAction == PLAYER_SKILL_HP_UP_OURFORCES
            || o->CurrentAction == PLAYER_SKILL_GIANTSWING
            || o->CurrentAction == PLAYER_SKILL_DRAGONLORE))
        return;

    if (Hit > 0)
    {
        if (c->Object.Type == MODEL_PLAYER)
        {
            if (c->Helper.Type == MODEL_HORN_OF_FENRIR)
            {
                SetAction_Fenrir_Damage(c, &c->Object);
                if (rand_fps_check(3))
                    PlayBuffer(static_cast<ESound>(SOUND_FENRIR_DAMAGE_1 + rand() % 2), o);
            }
            else
            {
                SetAction(&c->Object, PLAYER_SHOCK);
            }

            c->Movement = false;
        }
        else
        {
            if (o->CurrentAction<MONSTER01_ATTACK1 || o->CurrentAction>MONSTER01_ATTACK2)
                SetAction(&c->Object, MONSTER01_SHOCK);
            //c->Movement = false;
        }
        if (c->Object.AnimationFrame == 0.f)
        {
            PlayMonsterSound(o);

            if (o->Type != MODEL_PLAYER || (o->SubType >= MODEL_SKELETON1 && o->SubType <= MODEL_SKELETON3))
            {
                if (o->SubType >= MODEL_SKELETON1 && o->SubType <= MODEL_SKELETON3)
                    PlayBuffer(SOUND_BONE1, o);
                else if (o->Type != MODEL_ASSASSIN && Models[o->Type].Sounds[2] != -1)
                {
                    int offset = 0;
                    if (o->SubType == 9)
                    {
                        offset = 5;
                    }
                    PlayBuffer(static_cast<ESound>(SOUND_MONSTER + offset + Models[o->Type].Sounds[2 + rand() % 2]), o);
                }
            }
            else
            {
                if (!gCharacterManager.IsFemale(c->Class))
                {
                    if (gCharacterManager.GetBaseClass(c->Class) == CLASS_DARK_LORD && rand() % 5)
                        PlayBuffer(SOUND_DARKLORD_PAIN, o);
                    else
                        PlayBuffer(static_cast<ESound>(SOUND_HUMAN_SCREAM01 + rand() % 3), o);
                }
                else
                    PlayBuffer(static_cast<ESound>(SOUND_FEMALE_SCREAM01 + rand() % 2), o);
            }
        }

        if (o->Type == MODEL_CASTLE_GATE)
        {
            vec3_t Position;
            for (int i = 0; i < 5; i++)
            {
                if (rand_fps_check(2))
                {
                    Position[0] = o->Position[0] + (rand() % 128 - 64);
                    Position[1] = o->Position[1];
                    Position[2] = o->Position[2] + 200 + (rand() % 50);

                    CreateParticle(BITMAP_SMOKE + 1, Position, o->Angle, o->Light);
                }
            }
            PlayBuffer(SOUND_HIT_GATE);
        }
    }
}

void SetPlayerDie(CHARACTER* c)
{
    OBJECT* o = &c->Object;

    if (c == Hero)
    {
        CharacterAttribute->Life = 0;
    }

    if (c->Object.Type == MODEL_PLAYER)
    {
        if (o->SubType >= MODEL_SKELETON1 && o->SubType <= MODEL_SKELETON3)
        {
            o->Live = false;
            CreateEffect(MODEL_BONE1, o->Position, o->Angle, o->Light);
            for (int j = 0; j < 10; j++)
                CreateEffect(MODEL_BONE2, o->Position, o->Angle, o->Light);
            PlayBuffer(SOUND_BONE2, o);
        }
        else
            SetAction(&c->Object, PLAYER_DIE1);
    }
    else
    {
        switch (o->Type)
        {
        case MODEL_DEATH_COW:
        {
            o->Live = false;
            CreateEffect(MODEL_BONE1, o->Position, o->Angle, o->Light);
            for (int j = 0; j < 10; j++)
                CreateEffect(MODEL_BONE2, o->Position, o->Angle, o->Light);
            PlayBuffer(SOUND_BONE2, o);
        }
        break;
        case MODEL_STONE_GOLEM:
        {
            o->Live = false;
            for (int j = 0; j < 8; j++)
            {
                CreateEffect(MODEL_BIG_STONE1, o->Position, o->Angle, o->Light);
                CreateEffect(MODEL_BIG_STONE2, o->Position, o->Angle, o->Light);
            }
            PlayBuffer(SOUND_BONE2, o);
        }
        break;
        case MODEL_BLADE_HUNTER:
        case MODEL_TWIN_TAIL:
        {
            if (gMapManager.WorldActive == WD_39KANTURU_3RD && g_Direction.m_CKanturu.m_iKanturuState == KANTURU_STATE_MAYA_BATTLE)
            {
                FallingMonster(c, o);
                SetAction(&c->Object, MONSTER01_STOP2);
            }
            else
                SetAction(&c->Object, MONSTER01_DIE);
        }
        break;
        case MODEL_MAYA_HAND_LEFT:
        case MODEL_MAYA_HAND_RIGHT:
        {
            if (gMapManager.WorldActive == WD_39KANTURU_3RD && g_Direction.m_CKanturu.m_iMayaState >= KANTURU_MAYA_DIRECTION_MAYA3)
                SetAction(&c->Object, MONSTER01_STOP2);
            else
                SetAction(&c->Object, MONSTER01_DIE);
        }
        break;
        case MODEL_DOPPELGANGER:
        {
            if (c->Object.CurrentAction != MONSTER01_APEAR)
            {
                SetAction(&c->Object, MONSTER01_DIE);
            }
        }
        break;
        default:
            SetAction(&c->Object, MONSTER01_DIE);
            break;
        }
    }

    if (c->Object.AnimationFrame == 0.f)
    {
        PlayMonsterSound(o);
        if (gMapManager.InChaosCastle() == true)
        {
            PlayBuffer(static_cast<ESound>(SOUND_CHAOS_MOB_BOOM01 + rand() % 2), o);
        }
        else
        {
            if (o->Type != MODEL_PLAYER || (o->SubType >= MODEL_SKELETON1 && o->SubType <= MODEL_SKELETON3))
            {
                if (Models[o->Type].Sounds[4] != -1)
                {
                    int offset = 0;
                    if (o->SubType == 9)
                    {
                        offset = 5;
                    }
                    PlayBuffer(static_cast<ESound>(SOUND_MONSTER + offset + Models[o->Type].Sounds[4]), o);
                }
            }
            else
            {
                if (!gCharacterManager.IsFemale(c->Class))
                {
                    if (gCharacterManager.GetBaseClass(c->Class) == CLASS_DARK_LORD)
                        PlayBuffer(SOUND_DARKLORD_DEAD, o);
                    else
                        PlayBuffer(SOUND_HUMAN_SCREAM04, o);
                }
                else
                {
                    PlayBuffer(SOUND_FEMALE_SCREAM02, o);
                }

                if (c->Helper.Type == MODEL_HORN_OF_FENRIR)
                    PlayBuffer(SOUND_FENRIR_DEATH, o);	// 펜릴 죽는
            }
        }
        c->Object.AnimationFrame = 0.001f;
    }
}

void CalcAddPosition(OBJECT* o, float x, float y, float z, vec3_t Position)
{
    float Matrix[3][4];
    AngleMatrix(o->Angle, Matrix);
    vec3_t p;
    Vector(x, y, z, p);
    VectorRotate(p, Matrix, Position);
    VectorAdd(Position, o->Position, Position);
}

vec3_t BossHeadPosition;

void AttackEffect(CHARACTER* c)
{
    OBJECT* o = &c->Object;
    BMD* b = &Models[o->Type];
    int i;
    vec3_t Angle, Light;
    vec3_t p, Position;
    float Luminosity = (float)(rand() % 6 + 2) * 0.1f;
    Vector(0.f, 0.f, 0.f, p);
    Vector(1.f, 1.f, 1.f, Light);

    if (gMapManager.InHellas())
    {
        CHARACTER* tc = NULL;
        OBJECT* to = NULL;

        if (c->TargetCharacter >= 0 && c->TargetCharacter < MAX_CHARACTERS_CLIENT)
        {
            tc = &CharactersClient[c->TargetCharacter];
            to = &tc->Object;
        }

        if (AttackEffect_HellasMonster(c, tc, o, to, b) == true)
            return;
    }
    if (M31HuntingGround::AttackEffectHuntingGroundMonster(c, o, b) == true)
        return;
    if (battleCastle::AttackEffect_BattleCastleMonster(c, o, b) == true)
        return;
    if (M33Aida::AttackEffectAidaMonster(c, o, b) == true)
        return;
    if (M34CryingWolf2nd::AttackEffectCryingWolf2ndMonster(c, o, b) == true)
        return;
    if (M37Kanturu1st::AttackEffectKanturu1stMonster(c, o, b) == true)
        return;
    if (M38Kanturu2nd::AttackEffect_Kanturu2nd_Monster(c, o, b) == true)
        return;
    if (M39Kanturu3rd::AttackEffectKanturu3rdMonster(c, o, b) == true)
        return;
    if (M34CryWolf1st::AttackEffectCryWolf1stMonster(c, o, b) == true)
        return;
    if (SEASON3A::CGM3rdChangeUp::Instance().AttackEffectBalgasBarrackMonster(c, o, b))
        return;
    if (g_CursedTemple->AttackEffectMonster(c, o, b) == true)
        return;
    if (SEASON3B::GMNewTown::AttackEffectMonster(c, o, b) == true) return;
    if (SEASON3C::GMSwampOfQuiet::AttackEffectMonster(c, o, b) == true) return;

    if (TheMapProcess().AttackEffectMonster(c, o, b) == true) return;

    switch (c->MonsterIndex)
    {
    case MONSTER_CHAOS_CASTLE_1:
    case MONSTER_CHAOS_CASTLE_3:
    case MONSTER_CHAOS_CASTLE_5:
    case MONSTER_CHAOS_CASTLE_7:
    case MONSTER_CHAOS_CASTLE_9:
    case MONSTER_CHAOS_CASTLE_11:
    case MONSTER_CHAOS_CASTLE_13:
        break;

    case MONSTER_MAGIC_SKELETON_1:
    case MONSTER_MAGIC_SKELETON_2:
    case MONSTER_MAGIC_SKELETON_3:
    case MONSTER_MAGIC_SKELETON_4:
    case MONSTER_MAGIC_SKELETON_5:
    case MONSTER_MAGIC_SKELETON_6:
    case MONSTER_MAGIC_SKELETON_7:
        if ((c->Skill) == AT_SKILL_BOSS)
        {
            if (rand_fps_check(2))
            {
                if (c->CheckAttackTime(1))
                {
                    CreateEffect(MODEL_SKILL_INFERNO, o->Position, o->Angle, o->Light, 1);
                    c->SetLastAttackEffectTime();
                }
            }
            else
            {
                Vector(o->Position[0] + rand() % 1024 - 512, o->Position[1] + rand() % 1024 - 512, o->Position[2], Position);
                CreateEffect(MODEL_FIRE, Position, o->Angle, o->Light);
                PlayBuffer(SOUND_METEORITE01);
            }
        }
        break;

    case MONSTER_MOLT:
        break;
    case MONSTER_ALQUAMOS:
        break;
    case MONSTER_QUEEN_RAINER:
        if (c->CheckAttackTime(5))
        {
            if (c->TargetCharacter != -1)
            {
                CHARACTER* tc = &CharactersClient[c->TargetCharacter];
                OBJECT* to = &tc->Object;
                for (int i = 0; i < 20; ++i)
                {
                    CreateEffect(BITMAP_BLIZZARD, to->Position, to->Angle, Light);
                }
            }

            c->SetLastAttackEffectTime();
        }
        break;
    case MONSTER_OMEGA_WING:
    case MONSTER_MEGA_CRUST:
    case MONSTER_ALPHA_CRUST:
        if (c->Object.CurrentAction == MONSTER01_ATTACK1 || c->Object.CurrentAction == MONSTER01_ATTACK2)
        {
            if (c->CheckAttackTime(5))
            {
                CreateInferno(o->Position);
                CreateEffect(MODEL_SKILL_INFERNO, o->Position, o->Angle, o->Light);
                c->SetLastAttackEffectTime();
            }
        }
        break;
    case MONSTER_PHANTOM_KNIGHT:
        if ((c->Skill) == AT_SKILL_BOSS)
        {
            if (c->CheckAttackTime(14))
            {
                vec3_t Angle = { 0.0f, 0.0f, 0.0f };
                int iCount = 36;

                for (int i = 0; i < iCount; ++i)
                {
                    //Angle[2] = ( float)i * 10.0f;
                    Angle[0] = (float)(rand() % 360);
                    Angle[1] = (float)(rand() % 360);
                    Angle[2] = (float)(rand() % 360);
                    vec3_t Position;
                    VectorCopy(o->Position, Position);
                    Position[2] += 100.f;
                    CreateJoint(BITMAP_JOINT_SPIRIT, Position, Position, Angle, 1, NULL, 60.f, 0, 0);
                }

                c->SetLastAttackEffectTime();
            }
        }
        break;
    case MONSTER_DRAKAN:
    case MONSTER_GREAT_DRAKAN:
        if (c->Object.CurrentAction == MONSTER01_ATTACK1)
        {
            if (c->CheckAttackTime(11))
            {
                CreateInferno(o->Position);
                CreateEffect(MODEL_SKILL_INFERNO, o->Position, o->Angle, o->Light);

                for (int k = 0; k < 5; ++k)
                {
                    Vector(1.f, 0.5f, 0.f, Light);
                    VectorCopy(o->Angle, Angle);
                    Angle[0] += 45.f;
                    VectorCopy(o->Position, Position);
                    Position[0] += (float)(rand() % 1001 - 500);
                    Position[1] += (float)(rand() % 1001 - 500);
                    Position[2] += 500.f;
                    VectorCopy(Position, o->StartPosition);
                    CreateEffect(MODEL_PIERCING + 1, Position, Angle, Light, 1, o);
                }

                c->SetLastAttackEffectTime();
            }
        }
        else
        {
            if (c->CheckAttackTime(13))
            {
                Vector(1.f, 0.5f, 0.f, Light);
                Vector(-50.f, 100.f, 0.f, p);
                VectorCopy(o->Angle, Angle);
                Angle[0] += 45.f;
                b->TransformPosition(o->BoneTransform[11], p, Position, true);
                VectorCopy(Position, o->StartPosition);
                CreateEffect(MODEL_PIERCING + 1, Position, Angle, Light, 1, o);
                PlayBuffer(SOUND_METEORITE01);
                c->SetLastAttackEffectTime();
            }
            else if (c->CheckAttackTime(9))
            {
                Vector(1.f, 0.5f, 0.f, Light);
                Vector(0.f, 0.f, 0.f, p);
                VectorCopy(o->Angle, Angle);
                Angle[0] += 45.f;
                b->TransformPosition(o->BoneTransform[11], p, Position, true);
                c->SetLastAttackEffectTime();
            }
        }

        break;
    case MONSTER_DARK_PHOENIX:
        if ((c->Skill) == AT_SKILL_BOSS)
        {
            if (c->CheckAttackTime(2) || c->CheckAttackTime(6))
            {
                vec3_t Angle = { 0.0f, 0.0f, 0.0f };
                int iCount = 40;
                for (i = 0; i < iCount; ++i)
                {
                    //Angle[2] = ( float)i * 10.0f;
                    Angle[0] = (float)(rand() % 360);
                    Angle[1] = (float)(rand() % 360);
                    Angle[2] = (float)(rand() % 360);
                    vec3_t Position;
                    VectorCopy(o->Position, Position);
                    Position[2] += 100.f;
                    CreateJoint(BITMAP_JOINT_SPIRIT, Position, Position, Angle, 3, NULL, 50.f, 0, 0);
                }

                c->SetLastAttackEffectTime();
            }
        }
        break;
    case MONSTER_DEATH_BEAM_KNIGHT:
    case MONSTER_BEAM_KNIGHT:
        if (c->MonsterIndex == MONSTER_DEATH_BEAM_KNIGHT)
        {
            if (c->CheckAttackTime(1))
            {
                CreateInferno(o->Position);
                CreateEffect(MODEL_SKILL_INFERNO, o->Position, o->Angle, o->Light);
                c->SetLastAttackEffectTime();
            }
            if ((c->Skill) == AT_SKILL_BOSS)
            {
                if (c->MonsterIndex == MONSTER_DEATH_BEAM_KNIGHT && rand_fps_check(1))
                {
                    Vector(o->Position[0] + rand() % 800 - 400, o->Position[1] + rand() % 800 - 400, o->Position[2], Position);
                    CreateEffect(MODEL_SKILL_BLAST, Position, o->Angle, o->Light);
                }

                if (c->CheckAttackTime(14))
                {
                    for (int i = 0; i < 18; i++)
                    {
                        VectorCopy(o->Angle, Angle);
                        Angle[2] += i * 20.f;
                        CreateEffect(MODEL_STAFF_OF_DESTRUCTION, o->Position, Angle, o->Light);
                    }
                    c->SetLastAttackEffectTime();
                }
            }
        }
        else
        {
            if (c->CheckAttackTime(1))
            {
                //CreateInferno(o->Position);
                CreateEffect(MODEL_SKILL_INFERNO, o->Position, o->Angle, o->Light);
                c->SetLastAttackEffectTime();
            }
        }
        break;
    case MONSTER_CURSED_KING:
        if ((c->Skill) == AT_SKILL_BOSS)
        {
            if (c->CheckAttackTime(1))
            {
                CreateEffect(MODEL_SKILL_INFERNO, o->Position, o->Angle, o->Light, 1);
                c->SetLastAttackEffectTime();
            }
        }
        break;
    case MONSTER_GOLDEN_SOLDIER:
    case MONSTER_IRON_WHEEL:
    case MONSTER_SOLDIER:
        if (c->CheckAttackTime(1))
        {
            Vector(60.f, -110.f, 0.f, p);
            b->TransformPosition(o->BoneTransform[c->Weapon[0].LinkBone], p, Position, true);
            CreateEffect(MODEL_ARROW_BOMB, o->Position, o->Angle, o->Light, 0, o);
            if (c->MonsterIndex == MONSTER_IRON_WHEEL)
            {
                vec3_t Angle;
                VectorCopy(o->Angle, Angle);
                Angle[2] += 20.f;
                CreateEffect(MODEL_ARROW_BOMB, o->Position, Angle, o->Light, 0, o);
                Angle[2] -= 40.f;
                CreateEffect(MODEL_ARROW_BOMB, o->Position, Angle, o->Light, 0, o);
            }

            c->SetLastAttackEffectTime();
        }
        break;
    case MONSTER_GOLDEN_TITAN:
    case MONSTER_TANTALLOS:
    case MONSTER_ZAIKAN:
        if (c->CheckAttackTime(1))
        {
            CreateInferno(o->Position);
            c->SetLastAttackEffectTime();
        }
        if (c->CheckAttackTime(14))
        {
            if (c->MonsterIndex == MONSTER_ZAIKAN)
            {
                if ((c->Skill) == AT_SKILL_BOSS)
                {
                    for (i = 0; i < 18; i++)
                    {
                        VectorCopy(o->Angle, Angle);
                        Angle[2] += i * 20.f;
                        CreateEffect(MODEL_STAFF_OF_DESTRUCTION, o->Position, Angle, o->Light);
                    }
                }
            }

            c->SetLastAttackEffectTime();
        }
        break;
    case MONSTER_HYDRA:
    {
        const int attackTime = (int)c->AttackTime;
        if (attackTime % 5 == 1 && c->CheckAttackTime(attackTime))
        {
            b->TransformPosition(o->BoneTransform[63], p, Position, true);
            CreateEffect(BITMAP_BOSS_LASER + 1, Position, o->Angle, o->Light);
            c->SetLastAttackEffectTime();
        }

        if ((c->Skill) == AT_SKILL_BOSS)
        {
            if (c->CheckAttackTime(1))
            {
                VectorCopy(o->Angle, Angle); Angle[2] += 20.f;
                VectorCopy(o->Position, p); p[2] += 50.f;
                Luminosity = (15 - c->AttackTime) * 0.1f;
                Vector(Luminosity * 0.3f, Luminosity * 0.6f, Luminosity * 1.f, Light);

                for (int i = 0; i < 9; i++)
                {
                    Angle[2] += 40.f;
                    CreateEffect(BITMAP_BOSS_LASER, p, Angle, Light);
                }

                c->SetLastAttackEffectTime();
            }
        }
        break;
    }
    case MONSTER_RED_DRAGON:
        if ((c->Skill) == AT_SKILL_BOSS)
        {
            if (c->CheckAttackTime(1))
            {
                Vector(0.f, 0.f, 0.f, p);
                b->TransformPosition(o->BoneTransform[11], p, Position, true);
                Vector(o->Angle[0] - 20.f, o->Angle[1], o->Angle[2] - 30.f, Angle);
                CreateEffect(MODEL_FIRE, Position, Angle, o->Light, 2);
                Vector(o->Angle[0] - 30.f, o->Angle[1], o->Angle[2], Angle);
                CreateEffect(MODEL_FIRE, Position, Angle, o->Light, 2);
                Vector(o->Angle[0] - 20.f, o->Angle[1], o->Angle[2] + 30.f, Angle);
                CreateEffect(MODEL_FIRE, Position, Angle, o->Light, 2);
                PlayBuffer(SOUND_METEORITE01);
                c->SetLastAttackEffectTime();
            }
            if (rand_fps_check(1))
            {
                Vector(o->Position[0] + rand() % 1024 - 512, o->Position[1] + rand() % 1024 - 512, o->Position[2], Position);
                CreateEffect(MODEL_FIRE, Position, o->Angle, o->Light);
                PlayBuffer(SOUND_METEORITE01);
            }
        }
        break;
    case MONSTER_DEATH_GORGON:
        if ((c->Skill) == AT_SKILL_BOSS)
        {
            if (c->CheckAttackTime(1))
            {
                for (int i = 0; i < 18; i++)
                {
                    Vector(0.f, 0.f, i * 20.f, Angle);
                    CreateEffect(MODEL_FIRE, o->Position, Angle, o->Light, 1, o);
                }
                PlayBuffer(SOUND_METEORITE01);
                c->SetLastAttackEffectTime();
            }
        }
        break;
    case MONSTER_BALROG:
    case MONSTER_METAL_BALROG:
        if ((c->Skill) == AT_SKILL_BOSS)
        {
            if (c->CheckAttackTime(1))
            {
                CreateEffect(MODEL_CIRCLE, o->Position, o->Angle, o->Light);
                CreateEffect(MODEL_CIRCLE_LIGHT, o->Position, o->Angle, o->Light);
                PlayBuffer(SOUND_HELLFIRE);
                c->SetLastAttackEffectTime();
            }

            if (rand_fps_check(1))
            {
                Vector(o->Position[0] + rand() % 1024 - 512, o->Position[1] + rand() % 1024 - 512, o->Position[2], Position);
                CreateEffect(MODEL_FIRE, Position, o->Angle, o->Light);
                PlayBuffer(SOUND_METEORITE01);
            }
        }
        break;
    case MONSTER_METEORITE_TRAP://함정
        if ((c->Skill) == AT_SKILL_BOSS && rand_fps_check(1))
        {
            Vector(o->Position[0] + rand() % 1024 - 512, o->Position[1] + rand() % 1024 - 512, o->Position[2], Position);
            CreateEffect(MODEL_FIRE, Position, o->Angle, o->Light);
            PlayBuffer(SOUND_METEORITE01);
        }
        break;
    case MONSTER_BAHAMUT://물고기
        if (rand_fps_check(1))
        {
            for (int i = 0; i < 4; i++)
            {
                Vector((float)(rand() % 32 - 16), (float)(rand() % 32 - 16), (float)(rand() % 32 - 16), p);
                b->TransformPosition(o->BoneTransform[2], p, Position, true);
                CreateParticle(BITMAP_BUBBLE, Position, o->Angle, Light);
                CreateParticle(BITMAP_BLOOD + 1, Position, o->Angle, Light);
            }
        }
        break;
    default:
        break;
    }

    if (c->TargetCharacter >= 0 && c->TargetCharacter < MAX_CHARACTERS_CLIENT)
    {
        CHARACTER* tc = &CharactersClient[c->TargetCharacter];
        OBJECT* to = &tc->Object;

        if ((c->Skill) == AT_SKILL_ENERGYBALL)
        {
            switch (c->MonsterIndex)
            {
            case MONSTER_CHAOS_CASTLE_2:  //  카오스캐슬 궁수.
            case MONSTER_CHAOS_CASTLE_4:
            case MONSTER_CHAOS_CASTLE_6:
            case MONSTER_CHAOS_CASTLE_8:
            case MONSTER_CHAOS_CASTLE_10:
            case MONSTER_CHAOS_CASTLE_12:
            case MONSTER_CHAOS_CASTLE_14:
                if (c->Weapon[0].Type == MODEL_GREAT_REIGN_CROSSBOW)
                {
                    if (c->CheckAttackTime(8))
                    {
                        CreateArrows(c, o, o, 0, 0, 0);
                        c->SetLastAttackEffectTime();
                    }
                }
                else if (c->Object.CurrentAction == MONSTER01_ATTACK1)
                {
                    if (c->CheckAttackTime(15))
                    {
                        CalcAddPosition(o, -20.f, -90.f, 100.f, Position);
                        CreateEffect(BITMAP_BOSS_LASER, Position, o->Angle, Light, 0, o);
                        c->SetLastAttackEffectTime();
                    }
                }
                else if (c->Object.CurrentAction == MONSTER01_ATTACK2)
                {
                    if (c->CheckAttackTime(8))
                    {
                        if (rand_fps_check(2))
                        {
                            CreateEffect(MODEL_SKILL_BLAST, to->Position, o->Angle, o->Light, 0, o);
                            CreateEffect(MODEL_SKILL_BLAST, to->Position, o->Angle, o->Light, 0, o);
                        }
                        else
                        {
                            Vector(0.8f, 0.5f, 0.1f, Light);
                            CreateEffect(MODEL_FIRE, to->Position, o->Angle, Light, 6);
                            CreateEffect(MODEL_FIRE, to->Position, o->Angle, Light, 6);
                        }
                        c->SetLastAttackEffectTime();
                    }
                }
                break;

            case MONSTER_MAGIC_SKELETON_1:   //  마법 해골.
            case MONSTER_MAGIC_SKELETON_2:
            case MONSTER_MAGIC_SKELETON_3:
            case MONSTER_MAGIC_SKELETON_4:
            case MONSTER_MAGIC_SKELETON_5:
            case MONSTER_MAGIC_SKELETON_6:
            case MONSTER_MAGIC_SKELETON_7:
                if (c->CheckAttackTime(14))
                {
                    Vector(0.f, 0.f, 0.f, p);
                    b->TransformPosition(o->BoneTransform[33], p, Position, true);
                    VectorCopy(o->Angle, Angle);
                    CreateEffect(MODEL_PIERCING + 1, Position, Angle, Light, 1);
                    CreateJoint(BITMAP_JOINT_THUNDER, Position, Position, Angle, 2, to, 50.f);
                    c->SetLastAttackEffectTime();
                }
                break;

            case MONSTER_GIANT_OGRE_1:	//. 자이언트오거1
            case MONSTER_GIANT_OGRE_2:	//. 자이언트오거2
            case MONSTER_GIANT_OGRE_3:	//. 자이언트오거3
            case MONSTER_GIANT_OGRE_4:	//. 자이언트오거4
            case MONSTER_GIANT_OGRE_5:	//. 자이언트오거5
            case MONSTER_GIANT_OGRE_6:	//. 자이언트오거6
            case MONSTER_GIANT_OGRE_7:
                if (c->CheckAttackTime(13))
                {
                    Vector(1.0f, 1.0f, 1.0f, Light);
                    Vector(60.f, 30.f, 0.f, p);
                    b->TransformPosition(o->BoneTransform[6], p, Position, true);

                    Vector(o->Angle[0], o->Angle[1], o->Angle[2], Angle);
                    CreateEffect(MODEL_FIRE, Position, Angle, o->Light, 5);
                    c->SetLastAttackEffectTime();
                }
                break;

            case MONSTER_DARK_PHOENIX://불사조공격
                if (c->CheckAttackTime(14))
                {
                    Vector(0.f, 0.f, 0.f, p);
                    b->TransformPosition(g_fBoneSave[2], p, Position, true);
                    /*for ( int i = 0; i < 10; ++i)
                    {
                        CreateEffect(MODEL_FIRE,Position,o->Angle,o->Light,3,to);
                    }*/
                    VectorCopy(o->Angle, Angle);
                    CreateEffect(MODEL_PIERCING + 1, Position, Angle, Light, 1);
                    CreateJoint(BITMAP_JOINT_THUNDER, Position, Position, Angle, 2, to, 50.f);
                    c->SetLastAttackEffectTime();
                }
                break;
            case MONSTER_DRAKAN:
            case MONSTER_GREAT_DRAKAN:
                if (c->Object.CurrentAction == MONSTER01_ATTACK2)
                {
                    if (c->CheckAttackTime(13))
                    {
                        Vector(1.f, 0.5f, 0.f, Light);
                        Vector(-50.f, 100.f, 0.f, p);
                        VectorCopy(o->Angle, Angle);
                        Angle[0] += 45.f;
                        b->TransformPosition(o->BoneTransform[11], p, Position, true);
                        CreateEffect(MODEL_PIERCING + 1, Position, Angle, Light, 1);
                        CreateJoint(BITMAP_JOINT_THUNDER, Position, to->Position, Angle, 2, to, 50.f);
                        c->SetLastAttackEffectTime();
                    }
                }
                break;
            case MONSTER_ALQUAMOS:
                if (c->CheckAttackTime(1))
                {
                    for (int i = 0; i < 4; ++i)
                    {
                        CreateJoint(BITMAP_FLARE, o->Position, o->Position, Angle, 7, to, 50.f);
                        // CreateJoint(BITMAP_FLARE, Position, Position, Angle, 7, to, 50.f);
                    }

                    c->SetLastAttackEffectTime();
                }
                break;
            case MONSTER_BEAM_KNIGHT:
                if (rand_fps_check(1))
                {
                    for (int i = 0; i < 6; i++)
                    {
                        int Hand = 0;
                        if (i >= 3) Hand = 1;
                        b->TransformPosition(o->BoneTransform[c->Weapon[Hand].LinkBone], p, Position, true);//에러
                        Vector(0.f, 0.f, (float)(rand() % 360), Angle);
                        CreateJoint(BITMAP_JOINT_THUNDER, Position, to->Position, Angle, 2, to, 50.f);
                        CreateJoint(BITMAP_JOINT_THUNDER, Position, to->Position, Angle, 2, to, 10.f);
                    }

                    for (int i = 0; i < 4; i++)
                    {
                        int Hand = 0;
                        if (i >= 2) Hand = 1;
                        b->TransformPosition(o->BoneTransform[c->Weapon[Hand].LinkBone], p, Position, true);
                        Vector(0.f, 0.f, (float)(rand() % 360), Angle);
                        CreateJoint(BITMAP_JOINT_LASER + 1, Position, to->Position, Angle, 0, to, 50.f);
                        CreateParticle(BITMAP_FIRE, Position, o->Angle, o->Light);
                    }
                }

                if (c->CheckAttackTime(1))
                {
                    PlayBuffer(SOUND_EVIL);
                    c->SetLastAttackEffectTime();
                }

                break;
            case MONSTER_VEPAR:
                if (c->CheckAttackTime(1))
                {
                    PlayBuffer(SOUND_EVIL);
                    c->SetLastAttackEffectTime();
                }

                if (rand_fps_check(1))
                {
                    for (int i = 0; i < 4; i++)
                    {
                        int Hand = 0;
                        if (i >= 2) Hand = 1;
                        Vector(0.f, 0.f, 0.f, Angle);
                        b->TransformPosition(o->BoneTransform[c->Weapon[Hand].LinkBone], p, Position, true);
                        CreateJoint(BITMAP_BLUR + 1, Position, to->Position, Angle, 1, to, 50.f);
                        CreateJoint(BITMAP_BLUR + 1, Position, to->Position, Angle, 1, to, 10.f);
                    }
                }
                break;
            case MONSTER_DEVIL:
                if (c->CheckAttackTime(1))
                {
                    PlayBuffer(SOUND_EVIL);
                    c->SetLastAttackEffectTime();
                }

                if (rand_fps_check(1))
                {
                    for (int i = 0; i < 4; i++)
                    {
                        int Hand = 0;
                        if (i >= 2) Hand = 1;
                        b->TransformPosition(o->BoneTransform[c->Weapon[Hand].LinkBone], p, Position, true);
                        Vector(0.f, 0.f, (float)(rand() % 360), Angle);
                        CreateJoint(BITMAP_JOINT_LASER + 1, Position, to->Position, Angle, 0, to, 50.f);
                        CreateParticle(BITMAP_FIRE, Position, o->Angle, o->Light);
                    }
                }
                break;
            case MONSTER_CURSED_KING:
            {
                if (c->CheckAttackTime(1))
                {
                    PlayBuffer(SOUND_THUNDER01);
                    c->SetLastAttackEffectTime();
                }

                if (rand_fps_check(1))
                {
                    float fAngle = (float)((int)(45.f - (c->AttackTime * 3 + (int)WorldTime / 10)) % 90) + 180.f;

                    for (int i = 0; i < 4; i++)
                    {
                        b->TransformPosition(o->BoneTransform[c->Weapon[i % 2].LinkBone], p, Position, true);
                        Vector(0.f, 0.f, fAngle, Angle);
                        CreateJoint(BITMAP_JOINT_LASER + 1, Position, to->Position, Angle, 1, to, 50.f);
                        CreateParticle(BITMAP_FIRE, Position, o->Angle, o->Light);
                        fAngle += 270.f;
                    }
                }
            }
            break;

            default:
                break;
            }
        }
        else if ((c->Skill) == AT_SKILL_LIGHTNING || c->Skill == AT_SKILL_LIGHTNING_STR || c->Skill == AT_SKILL_LIGHTNING_STR_MG)
        {
            switch (c->MonsterIndex)
            {
            case MONSTER_MAGIC_SKELETON_1:   //  마법 해골.
            case MONSTER_MAGIC_SKELETON_2:
            case MONSTER_MAGIC_SKELETON_3:
            case MONSTER_MAGIC_SKELETON_4:
            case MONSTER_MAGIC_SKELETON_5:
            case MONSTER_MAGIC_SKELETON_6:
            case MONSTER_MAGIC_SKELETON_7:
            {
                if (c->CheckAttackTime(1))
                {
                    PlayBuffer(SOUND_THUNDER01);
                    c->SetLastAttackEffectTime();
                }
                if (rand_fps_check(1))
                {
                    float fAngle = (float)(45.f - (int)(c->AttackTime * 3 + (int)WorldTime / 10) % 90) + 180.f;
                    for (int i = 0; i < 4; i++)
                    {
                        b->TransformPosition(o->BoneTransform[c->Weapon[i % 2].LinkBone], p, Position, true);
                        Vector(0.f, 0.f, fAngle, Angle);
                        CreateJoint(BITMAP_JOINT_LASER + 1, Position, to->Position, Angle, 1, to, 50.f);
                        CreateParticle(BITMAP_FIRE, Position, o->Angle, o->Light);
                        fAngle += 270.f;
                    }
                }
            }
            break;

            case MONSTER_DARK_PHOENIX://불사조공격
                if (8 <= c->AttackTime && rand_fps_check(1))
                {
                    Vector(0.f, 0.f, 0.f, p);
                    b->TransformPosition(g_fBoneSave[0], p, Position, true);
                    for (int i = 0; i < 4; ++i)
                    {
                        Vector(0.f, 0.f, (float)(rand() % 360), Angle);
                        CreateJoint(BITMAP_JOINT_THUNDER, Position, to->Position, Angle, 0, to, 80.f);
                    }
                }
                break;
            case MONSTER_DEVIL://데빌
                if (c->CheckAttackTime(1))
                {
                    PlayBuffer(SOUND_EVIL);
                    c->SetLastAttackEffectTime();
                }

                if (rand_fps_check(1))
                {
                    for (int i = 0; i < 4; i++)
                    {
                        int Hand = 0;
                        if (i >= 2) Hand = 1;
                        b->TransformPosition(o->BoneTransform[c->Weapon[Hand].LinkBone], p, Position, true);
                        Vector(0.f, 0.f, (float)(rand() % 360), Angle);
                        CreateJoint(BITMAP_JOINT_LASER + 1, Position, to->Position, Angle, 0, to, 50.f);
                        CreateParticle(BITMAP_FIRE, Position, o->Angle, o->Light);
                    }
                }
                break;
            case MONSTER_CURSED_WIZARD:
                if (rand_fps_check(1))
                {
                    for (int i = 0; i < 4; i++)
                    {
                        int Hand = 0;
                        if (i >= 2) Hand = 1;
                        b->TransformPosition(o->BoneTransform[c->Weapon[Hand].LinkBone], p, Position, true);
                        Vector(0.f, 0.f, (float)(rand() % 360), Angle);
                        CreateJoint(BITMAP_JOINT_THUNDER, Position, to->Position, Angle, 0, to, 50.f);
                        CreateJoint(BITMAP_JOINT_THUNDER, Position, to->Position, Angle, 0, to, 10.f);
                        CreateParticle(BITMAP_ENERGY, Position, o->Angle, Light);
                    }
                }
                break;
            case MONSTER_LIZARD_KING://리자드킹
                if (rand_fps_check(1))
                {
                    for (int i = 0; i < 6; i++)
                    {
                        int Hand = 0;
                        if (i >= 3) Hand = 1;
                        b->TransformPosition(o->BoneTransform[c->Weapon[Hand].LinkBone], p, Position, true);//에러
                        Vector(0.f, 0.f, (float)(rand() % 360), Angle);
                        CreateJoint(BITMAP_JOINT_THUNDER, Position, to->Position, Angle, 2, to, 50.f);
                        CreateJoint(BITMAP_JOINT_THUNDER, Position, to->Position, Angle, 2, to, 10.f);
                    }
                }
                break;
            case MONSTER_POISON_SHADOW:
                if (rand_fps_check(1))
                {
                    if (o->Type == MODEL_PLAYER)
                    {
                        Vector(0.f, 0.f, 0.f, p);
                    }
                    else
                    {
                        Vector(0.f, -130.f, 0.f, p);
                    }
                    b->TransformPosition(o->BoneTransform[c->Weapon[0].LinkBone], p, Position, true);
                    Vector(-60.f, 0.f, o->Angle[2], Angle);
                    CreateJoint(BITMAP_JOINT_THUNDER, Position, to->Position, Angle, 0, to, 50.f);
                    CreateJoint(BITMAP_JOINT_THUNDER, Position, to->Position, Angle, 0, to, 10.f);
                    CreateParticle(BITMAP_ENERGY, Position, o->Angle, Light);
                }
                break;
            case MONSTER_ILLUSION_SORCERER_SPIRIT1_LIGHTNING:
            case MONSTER_ILLUSION_SORCERER_SPIRIT2_LIGHTNING:
            case MONSTER_ILLUSION_SORCERER_SPIRIT3_LIGHTNING:
            case MONSTER_ILLUSION_SORCERER_SPIRIT4_LIGHTNING:
            case MONSTER_ILLUSION_SORCERER_SPIRIT5_LIGHTNING:
            case MONSTER_ILLUSION_SORCERER_SPIRIT6_LIGHTNING:
            {
                if (rand_fps_check(1))
                {
                    Vector(8.f, 0.f, 0.f, Light);
                    b->TransformPosition(o->BoneTransform[17], p, Position, true);
                    Vector(-60.f, 0.f, o->Angle[2], Angle);
                    //CreateJoint(BITMAP_JOINT_THUNDER,Position,to->Position,Angle,0,to,50.f);
                    CreateJoint(BITMAP_JOINT_THUNDER, Position, to->Position, Angle, 21, to, 50.f);
                    CreateParticle(BITMAP_ENERGY, Position, o->Angle, Light);

                    b->TransformPosition(o->BoneTransform[41], p, Position, true);
                    Vector(-60.f, 0.f, o->Angle[2], Angle);
                    //CreateJoint(BITMAP_JOINT_THUNDER,Position,to->Position,Angle,0,to,50.f);
                    CreateJoint(BITMAP_JOINT_THUNDER, Position, to->Position, Angle, 21, to, 50.f);
                    CreateParticle(BITMAP_ENERGY, Position, o->Angle, Light);
                }
            }
            break;
            // 플레이어 이거나 기타 몬스터가 전기(번개)를 사용했을시
            default:
                if (rand_fps_check(1))
                {
                    if (b->NumBones < c->Weapon[0].LinkBone) break;

                    if (o->Type == MODEL_PLAYER)
                    {
                        Vector(0.f, 0.f, 0.f, p);
                    }
                    else
                    {
                        Vector(0.f, -130.f, 0.f, p);
                    }
                    b->TransformPosition(o->BoneTransform[c->Weapon[0].LinkBone], p, Position, true);
                    Vector(-60.f, 0.f, o->Angle[2], Angle);
                    CreateJoint(BITMAP_JOINT_THUNDER, Position, to->Position, Angle, 0, to, 50.f);
                    CreateJoint(BITMAP_JOINT_THUNDER, Position, to->Position, Angle, 0, to, 10.f);
                    CreateParticle(BITMAP_ENERGY, Position, o->Angle, Light);
                }
                break;
            }
        }
        else
        {
        }
    }
}

bool CheckMonsterSkill(CHARACTER* pCharacter, OBJECT* pObject)
{
    if (pCharacter->MonsterIndex == MONSTER_MAYA)
    {
        M39Kanturu3rd::MayaSceneMayaAction(pCharacter->MonsterSkill);
        return true;
    }

    int iCheckAttackAni = -1;

    for (int i = 0; i < MAX_MONSTERSKILL_NUM; i++)
    {
        if (MonsterSkill[pCharacter->MonsterIndex].Skill_Num[i] == pCharacter->MonsterSkill)
        {
            iCheckAttackAni = i;
            break;
        }
        else
        {
            iCheckAttackAni = -1;
        }
    }

    switch (iCheckAttackAni)
    {
    case 0:
        SetAction(pObject, MONSTER01_ATTACK1);
        break;
    case 1:
        SetAction(pObject, MONSTER01_ATTACK2);
        break;
    case 2:
        SetAction(pObject, MONSTER01_ATTACK3);
        break;
    case 3:
        SetAction(pObject, MONSTER01_ATTACK4);
        break;
    case 4:
        SetAction(pObject, MONSTER01_ATTACK5);
        break;
    case 5:
        SetAction(pObject, MONSTER01_ATTACK5);
        break;
    case 6:
        SetAction(pObject, MONSTER01_ATTACK5);
        break;
    case 7:
        SetAction(pObject, MONSTER01_ATTACK5);
        break;
    case 8:
        SetAction(pObject, MONSTER01_ATTACK5);
        break;
    case 9:
        SetAction(pObject, MONSTER01_ATTACK5);
        break;
    }

    pCharacter->MonsterSkill = -1;

    if (iCheckAttackAni < 0)
    {
        SetAction(pObject, MONSTER01_ATTACK1);
    }

    if (iCheckAttackAni > 4)
        return false;

    return true;
}

bool CharacterAnimation(CHARACTER* c, OBJECT* o)
{
    bool Play;
    BMD* b = &Models[o->Type];

    float PlaySpeed = 0.f;
    if (b->NumActions > 0)
    {
        PlaySpeed = b->Actions[b->CurrentAction].PlaySpeed;
        if (PlaySpeed < 0.f)
            PlaySpeed = 0.f;
        if (c->Change && o->CurrentAction >= MONSTER01_ATTACK1 && o->CurrentAction <= MONSTER01_ATTACK2)
            PlaySpeed *= 1.5f;
        if (o->CurrentAction == PLAYER_SKILL_VITALITY && o->AnimationFrame > 6.f)
        {
            PlaySpeed *= pow(1.0f / (2.f), FPS_ANIMATION_FACTOR);
        }
        else if ((o->CurrentAction == PLAYER_ATTACK_TELEPORT || o->CurrentAction == PLAYER_ATTACK_RIDE_TELEPORT
            || o->CurrentAction == PLAYER_FENRIR_ATTACK_DARKLORD_TELEPORT
            ) && o->AnimationFrame > 5.5f)
        {
            PlaySpeed *= pow(1.0f / (10.f), FPS_ANIMATION_FACTOR);
        }
        else if (gCharacterManager.GetBaseClass(c->Class) == CLASS_DARK_LORD &&
            (o->CurrentAction == PLAYER_SKILL_FLASH || o->CurrentAction == PLAYER_ATTACK_RIDE_ATTACK_FLASH
                || o->CurrentAction == PLAYER_FENRIR_ATTACK_DARKLORD_FLASH
                ) && (o->AnimationFrame > 1.f && o->AnimationFrame < 3.f)
            )
        {
            if (g_pPartyManager->IsPartyMemberChar(c) == false)
            {
                PlaySpeed *= pow(1.0f / (2.f), FPS_ANIMATION_FACTOR);
            }
            else
            {
                PlaySpeed *= pow(1.0f / (8.f), FPS_ANIMATION_FACTOR);
            }
        }
        if (o->CurrentAction == PLAYER_SKILL_HELL_BEGIN)
        {
            PlaySpeed *= powf(1.0f / (2.f), FPS_ANIMATION_FACTOR);
        }
        if (o->Type != MODEL_PLAYER)
        {
            switch (o->Type)
            {
            case MODEL_ILLUSION_OF_KUNDUN:
                if (o->CurrentAction == MONSTER01_DIE && o->AnimationFrame > 6)
                    PlaySpeed *= pow(4.0f, FPS_ANIMATION_FACTOR);
                break;
            case MODEL_FACE:
            case MODEL_FACE + 1:
            case MODEL_FACE + 2:
            case MODEL_FACE + 3:
            case MODEL_FACE + 4:
            case MODEL_FACE + 5:
            case MODEL_FACE + 6:
                PlaySpeed *= powf(2.0f, FPS_ANIMATION_FACTOR);
                break;
            }
        }
        if (o->Type == MODEL_EROHIM)
        {
            if (o->CurrentAction == MONSTER01_DIE)
                PlaySpeed *= pow(1.0f / (2.f), FPS_ANIMATION_FACTOR);
        }
    }

    if (g_isCharacterBuff(o, eDeBuff_Stun) || g_isCharacterBuff(o, eDeBuff_Sleep))
    {
        return false;
    }

    Play = b->PlayAnimation(&o->AnimationFrame, &o->PriorAnimationFrame, &o->PriorAction, PlaySpeed, o->Position, o->Angle);
    if (o->CurrentAction == PLAYER_CHANGE_UP)
    {
        if (Play == false)
            SetPlayerStop(c);
    }

    if (o->CurrentAction == PLAYER_RECOVER_SKILL)
    {
        if (Play == false)
            SetPlayerStop(c);
    }
    return Play;
}

int GetHandOfWeapon(OBJECT* o)
{
    int Hand = 0;
    if (o->Type == MODEL_PLAYER)
    {
        if (o->CurrentAction == PLAYER_ATTACK_SWORD_LEFT1 || o->CurrentAction == PLAYER_ATTACK_SWORD_LEFT2)
            Hand = 1;
    }

    return (Hand);
}

bool AttackStage(CHARACTER* c, OBJECT* o)
{
    // 무기 위치 얻기
    int Hand = GetHandOfWeapon(o);

    int iSkill = (c->Skill);

    g_iLimitAttackTime = 15;
    switch (iSkill)
    {
    case AT_SKILL_DEATHSTAB:
    case AT_SKILL_DEATHSTAB_STR:
    {
        BMD* b = &Models[o->Type];

        if (b->Bones[c->Weapon[Hand].LinkBone].Dummy || c->Weapon[Hand].LinkBone >= b->NumBones)
        {
            break;
        }

        if (c->CheckAttackTime(8))
        {
            if (SceneFlag != LOG_IN_SCENE)
                PlayBuffer(SOUND_SKILL_SWORD2);

            c->SetLastAttackEffectTime();
        }

        if (2 <= c->AttackTime && c->AttackTime <= 8 && rand_fps_check(1))
        {
            for (int j = 0; j < 3; ++j)
            {
                vec3_t CurPos;
                VectorCopy(o->Position, CurPos);
                CurPos[2] += 120.0f;
                vec3_t TempPos;
                GetNearRandomPos(CurPos, 300, TempPos);
                float fDistance = 1400.0f;
                TempPos[0] += -fDistance * sinf(o->Angle[2] * Q_PI / 180.0f);
                TempPos[1] += fDistance * cosf(o->Angle[2] * Q_PI / 180.0f);
                CreateJoint(MODEL_SPEARSKILL, TempPos, TempPos, o->Angle, 2, o, 40.0f);
            }
        }

        if (c->AttackTime <= 8 && rand_fps_check(1))
        {	// 기 모일 곳 위치
            vec3_t Position2 = { 0.0f, 0.0f, 0.0f };
            b->TransformPosition(o->BoneTransform[c->Weapon[Hand].LinkBone], Position2, o->m_vPosSword, true);

            float fDistance = 300.0f;
            o->m_vPosSword[0] += fDistance * sinf(o->Angle[2] * Q_PI / 180.0f);
            o->m_vPosSword[1] += -fDistance * cosf(o->Angle[2] * Q_PI / 180.0f);
        }

        if (6 <= c->AttackTime && c->AttackTime <= 12)
        {	// 꼬깔 만들기
            if (rand_fps_check(2))
            {
                vec3_t Position;

                //memcpy( Position, o->Position, sizeof ( vec3_t));
                vec3_t Position2 = { 0.0f, 0.0f, 0.0f };

                b->TransformPosition(o->BoneTransform[c->Weapon[Hand].LinkBone], Position2, Position, true);

                float fDistance = 100.0f + (float)(c->AttackTime - 8) * 10.0f;
                Position[0] += fDistance * sinf(o->Angle[2] * Q_PI / 180.0f);
                Position[1] += -fDistance * cosf(o->Angle[2] * Q_PI / 180.0f);
                //Position[2] += 110.0f;
                vec3_t Light = { 1.0f, 1.0f, 1.0f };
                CreateEffect(MODEL_SPEAR, Position, o->Angle, Light, 1, o);
                CreateEffect(MODEL_SPEAR, Position, o->Angle, Light, 1, o);
            }

            if (c->TargetCharacter != -1)
            {
                CHARACTER* tc = &CharactersClient[c->TargetCharacter];
                if (c->TargetCharacter != -1)
                {
                    OBJECT* to = &tc->Object;
                    if (10 <= c->AttackTime && to->Live)
                    {
                        //PlayBuffer( SOUND_THUNDER01);
                        to->m_byHurtByDeathstab = 35;
                    }
                }
            }
        }
        if (c->AttackTime >= 12)
        {
            c->AttackTime = g_iLimitAttackTime;
        }
    }
    break;
    case AT_SKILL_IMPALE:	// 창찌르기
    {
        BMD* b = &Models[o->Type];

        vec3_t p;
        if (c->CheckAttackTime(10))
        {
            PlayBuffer(SOUND_RIDINGSPEAR);
            c->SetLastAttackEffectTime();
        }
        else if (c->CheckAttackTime(4))
        {	// 준비동작
            vec3_t Light = { 1.0f, 1.0f, .5f };
            vec3_t Position2 = { 0.0f, 0.0f, 0.0f };
            b->TransformPosition(o->BoneTransform[c->Weapon[Hand].LinkBone], Position2, p, true);
            CreateEffect(MODEL__SPEAR, p, o->Angle, Light, c->Weapon[Hand].Type, o);
            //CreateEffect(BITMAP_MAGIC+1,o->Position,o->Angle,Light,4,o);
            c->SetLastAttackEffectTime();
        }
        else if (c->CheckAttackTime(8))
        {	// 꼬깔 만들기
            vec3_t Position;
            memcpy(Position, o->Position, sizeof(vec3_t));
            Position[0] += 50.0f * sinf(o->Angle[2] * Q_PI / 180.0f);
            Position[1] += -50.0f * cosf(o->Angle[2] * Q_PI / 180.0f);
            Position[2] += 110.0f;
            vec3_t Light = { 1.0f, 1.0f, 1.0f };
            CreateEffect(MODEL_SPEAR, Position, o->Angle, Light, 0, o);
            CreateEffect(MODEL_SPEAR, Position, o->Angle, Light, 0, o);
            c->SetLastAttackEffectTime();
        }
        if (13 <= c->AttackTime && c->AttackTime <= 14 && rand_fps_check(1))
        {	// 현란한 창술
            for (int i = 0; i < 3; ++i)
            {
                vec3_t Position;
                memcpy(Position, o->Position, sizeof(vec3_t));
                Position[0] += 145.0f * sinf(o->Angle[2] * Q_PI / 180.0f);
                Position[1] += -145.0f * cosf(o->Angle[2] * Q_PI / 180.0f);
                Position[2] += 110.0f;
                //vec3_t Light = { .6f, .6f, .2f};
                vec3_t Light = { .3f, .3f, .3f };
                if (c->CheckAttackTime(11))
                {
                    /*Light[0] = 1.0f;
                    Light[1] = 0.5f;
                    Light[2] = 0.5f;*/
                }
                else
                {
                    Position[0] += (rand() % 60 - 30);
                    Position[1] += (rand() % 60 - 30);
                    Position[2] += (rand() % 60 - 30);
                }
                CreateEffect(MODEL_SPEARSKILL, Position, o->Angle, Light, c->Weapon[Hand].Type, o);
            }
        }
    }
    break;

    case    AT_SKILL_PENETRATION:
    case AT_SKILL_PENETRATION_STR:
        if (o->Type == MODEL_PLAYER && o->CurrentAction >= PLAYER_ATTACK_FIST && o->CurrentAction <= PLAYER_RIDE_SKILL)
        {
            if (o->AnimationFrame >= 5.f)
            {
                o->PriorAnimationFrame = 4.f;
                o->AnimationFrame = 5.f;
            }
        }

        if (c->CheckAttackTime(3))  //  氣 모으기.
        {
            CreateEffect(BITMAP_GATHERING, o->Position, o->Angle, o->Light, 0, o);
            PlayBuffer(SOUND_PIERCING, o);
            c->SetLastAttackEffectTime();
        }
        g_iLimitAttackTime = 5;
        break;

    case AT_SKILL_FIRE_SLASH:
    case AT_SKILL_FIRE_SLASH_STR:
        if (o->Type == MODEL_PLAYER)
        {
            SetAction(o, PLAYER_ATTACK_SKILL_WHEEL);

            if (c->CheckAttackTime(1) || c->CheckAttackTime(2))
            {
                vec3_t Angle;
                Vector(1.f, 0.f, 0.f, Angle);
                CreateEffect(BITMAP_GATHERING, o->Position, o->Angle, o->Light, 1, o);
                c->SetLastAttackEffectTime();
            }

            if (o->AnimationFrame >= 3.f && rand_fps_check(1))
            {
                o->PKKey = getTargetCharacterKey(c, SelectedCharacter);

                PlayBuffer(SOUND_SKILL_SWORD3);

                if (iSkill == AT_SKILL_FIRE_SLASH_STR)
                {
                    CreateEffect(BITMAP_SWORD_FORCE, o->Position, o->Angle, o->Light, 1, o, o->PKKey, FindHotKey(iSkill));
                }
                else
                {
                    CreateEffect(BITMAP_SWORD_FORCE, o->Position, o->Angle, o->Light, 0, o, o->PKKey, FindHotKey(iSkill));
                }

                float AttackSpeed1 = CharacterAttribute->AttackSpeed * 0.004f;//
                Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_SKILL_WHEEL].PlaySpeed = 0.54f + AttackSpeed1;
                c->AttackTime = 15;
            }
        }
        g_iLimitAttackTime = 15;
        break;

    case AT_SKILL_POWER_SLASH:
    case AT_SKILL_POWER_SLASH_STR:
        if (o->Type == MODEL_PLAYER && o->CurrentAction == PLAYER_ATTACK_TWO_HAND_SWORD_TWO && rand_fps_check(1))
        {
            vec3_t Angle;

            VectorCopy(o->Angle, Angle);

            Angle[2] -= 40.f;
            CreateEffect(MODEL_MAGIC2, o->Position, Angle, o->Light, 2, o);
            Angle[2] += 20.f;
            CreateEffect(MODEL_MAGIC2, o->Position, Angle, o->Light, 2, o);
            Angle[2] += 20.f;
            CreateEffect(MODEL_MAGIC2, o->Position, Angle, o->Light, 2, o);
            Angle[2] += 20.f;
            CreateEffect(MODEL_MAGIC2, o->Position, Angle, o->Light, 2, o);
            Angle[2] += 20.f;
            CreateEffect(MODEL_MAGIC2, o->Position, Angle, o->Light, 2, o);

            PlayBuffer(SOUND_SKILL_SWORD3);

            c->AttackTime = 15;
        }
        g_iLimitAttackTime = 15;
        break;

    case AT_SKILL_NOVA:
        if (o->AnimationFrame >= 14.f && o->Type == MODEL_PLAYER && o->CurrentAction == PLAYER_SKILL_HELL_START)
        {
            c->AttackTime = 15;
        }
        break;
    case AT_SKILL_SWELL_LIFE:
    case AT_SKILL_SWELL_LIFE_STR:
    case AT_SKILL_SWELL_LIFE_PROFICIENCY:
        if ((int)c->AttackTime > 9 && o->Type == MODEL_PLAYER && o->CurrentAction == PLAYER_SKILL_VITALITY)
        {
            c->AttackTime = 15;
        }
        break;
    case    AT_SKILL_IMPROVE_AG:
        if (o->AnimationFrame >= 5.f && ((o->Type == MODEL_PLAYER && o->CurrentAction >= PLAYER_ATTACK_FIST && o->CurrentAction <= PLAYER_RIDE_SKILL) ||
            ((o->Type >= MODEL_MONSTER01 && o->Type < MODEL_MONSTER_END) && o->CurrentAction >= MONSTER01_ATTACK1 && o->CurrentAction <= MONSTER01_ATTACK2)))
        {
            c->AttackTime = 15;
        }
        break;
    case AT_SKILL_FORCE:
    case AT_SKILL_FORCE_WAVE:
    case AT_SKILL_FORCE_WAVE_STR:
        if (o->AnimationFrame >= 3.f && o->Type == MODEL_PLAYER && (o->CurrentAction == PLAYER_ATTACK_STRIKE || o->CurrentAction == PLAYER_ATTACK_RIDE_STRIKE
            || o->CurrentAction == PLAYER_FENRIR_ATTACK_DARKLORD_STRIKE
            ))
        {
            c->AttackTime = 15;
        }
        break;
    case AT_SKILL_FIREBURST:
    case AT_SKILL_FIREBURST_STR:
    case AT_SKILL_FIREBURST_MASTERY:
        if (o->AnimationFrame >= 3.f && o->Type == MODEL_PLAYER && (o->CurrentAction == PLAYER_ATTACK_STRIKE || o->CurrentAction == PLAYER_ATTACK_RIDE_STRIKE
            || o->CurrentAction == PLAYER_FENRIR_ATTACK_DARKLORD_STRIKE))
        {
            c->AttackTime = 15;
        }
        break;
    case    AT_SKILL_THUNDER_STRIKE:
        if (o->AnimationFrame >= 5.5f && o->Type == MODEL_PLAYER && (o->CurrentAction == PLAYER_SKILL_FLASH || o->CurrentAction == PLAYER_ATTACK_RIDE_ATTACK_FLASH
            || o->CurrentAction == PLAYER_FENRIR_ATTACK_DARKLORD_FLASH))
        {
            c->AttackTime = 15;
        }
        else
        {
            c->AttackTime = 10;
        }
        break;
    case    AT_SKILL_EARTHSHAKE:
    case    AT_SKILL_EARTHSHAKE_STR:
    case    AT_SKILL_EARTHSHAKE_MASTERY:
        if (o->AnimationFrame >= 5.f && o->Type == MODEL_PLAYER && o->CurrentAction == PLAYER_ATTACK_DARKHORSE)
        {
            c->AttackTime = 15;
        }
        break;
    case    AT_SKILL_PARTY_TELEPORT:
        if ((int)c->AttackTime > 5 && o->Type == MODEL_PLAYER && (o->CurrentAction == PLAYER_ATTACK_TELEPORT || o->CurrentAction == PLAYER_ATTACK_RIDE_TELEPORT
            || o->CurrentAction == PLAYER_FENRIR_ATTACK_DARKLORD_TELEPORT))
        {
            c->AttackTime = 15;
        }
        break;
    case    AT_SKILL_RIDER:
        if (o->AnimationFrame >= 5.f && o->Type == MODEL_PLAYER && (o->CurrentAction == PLAYER_SKILL_RIDER || o->CurrentAction == PLAYER_SKILL_RIDER_FLY))
        {
            c->AttackTime = 15;
        }
        break;

    case    AT_SKILL_STUN:
    case    AT_SKILL_MANA:
        break;
    case    AT_SKILL_INVISIBLE:
        c->AttackTime = 15;
        break;
    case    AT_SKILL_REMOVAL_STUN:
    case    AT_SKILL_REMOVAL_INVISIBLE:
        c->AttackTime = 15;
        break;
    case    AT_SKILL_REMOVAL_BUFF:
        if (o->AnimationFrame >= 3.5f)
        {
            c->AttackTime = 15;
        }
        break;

    case    AT_SKILL_RUSH:
    case AT_SKILL_OCCUPY:
        if (o->AnimationFrame > 5.f)
        {
            c->AttackTime = 15;
        }
        if (rand_fps_check(1))
        {
            vec3_t Position;
            vec3_t	Angle;

            VectorCopy(o->Position, Position);
            Position[0] += rand() % 30 - 15.f;
            Position[1] += rand() % 30 - 15.f;
            Position[2] += 20.f;
            for (int i = 0; i < 4; i++)
            {
                Vector((float)(rand() % 60 + 60 + 90), 0.f, o->Angle[2], Angle);
                CreateJoint(BITMAP_JOINT_SPARK, Position, Position, Angle);
                if (iSkill == AT_SKILL_OCCUPY)
                    CreateParticle(BITMAP_FIRE, Position, Angle, o->Light, 18, 1.5f);
                else
                    CreateParticle(BITMAP_FIRE, Position, Angle, o->Light, 2, 1.5f);
            }
        }
        break;
    case    AT_SKILL_SPIRAL_SLASH:
        if (o->AnimationFrame > 5.f && rand_fps_check(1))
        {
            CreateJoint(BITMAP_FLARE, o->Position, o->Position, o->Angle, 23, NULL, 40.f, 0);
            CreateJoint(BITMAP_FLARE, o->Position, o->Position, o->Angle, 23, NULL, 40.f, 1);
            CreateJoint(BITMAP_FLARE, o->Position, o->Position, o->Angle, 23, NULL, 40.f, 4);
            c->AttackTime = 15;

            PlayBuffer(SOUND_BCS_ONE_FLASH);
        }
        else if (o->AnimationFrame > 2.3f && o->AnimationFrame < 2.6f && rand_fps_check(1))
        {
            CreateJoint(BITMAP_FLARE, o->Position, o->Position, o->Angle, 23, NULL, 40.f, 2);
            CreateJoint(BITMAP_FLARE, o->Position, o->Position, o->Angle, 23, NULL, 40.f, 3);

            PlayBuffer(SOUND_BCS_ONE_FLASH);
        }
        g_iLimitAttackTime = 15;
        break;

    case AT_SKILL_SPACE_SPLIT:
        if (o->AnimationFrame >= 3.f
            && o->Type == MODEL_PLAYER
            && (o->CurrentAction == PLAYER_ATTACK_STRIKE || o->CurrentAction == PLAYER_ATTACK_RIDE_STRIKE || o->CurrentAction == PLAYER_FENRIR_ATTACK_DARKLORD_STRIKE)
            )
        {
            c->AttackTime = 15;
        }
        break;

    case    AT_SKILL_DEATH_CANNON:
        if (o->AnimationFrame >= 3.f && o->Type == MODEL_PLAYER && o->CurrentAction == PLAYER_ATTACK_DEATH_CANNON)
        {
            c->AttackTime = 15;
        }
        break;
    case AT_SKILL_FLAME_STRIKE:
    {
        c->AttackTime = 15;
    }
    break;
    case AT_SKILL_GIGANTIC_STORM:
        if (o->AnimationFrame > 7.f)
        {
            c->AttackTime = 15;
        }
        break;

    case AT_SKILL_LIGHTNING_SHOCK_STR:
    case AT_SKILL_LIGHTNING_SHOCK:
    {
        c->AttackTime = 15;
    }
    break;
    case AT_SKILL_STRIKE_OF_DESTRUCTION:
    case AT_SKILL_STRIKE_OF_DESTRUCTION_STR:
    {
        c->AttackTime = 15;
    }
    break;
    case AT_SKILL_ATT_UP_OURFORCES:
    case AT_SKILL_HP_UP_OURFORCES:
    case AT_SKILL_HP_UP_OURFORCES_STR:
    case AT_SKILL_DEF_UP_OURFORCES:
    case AT_SKILL_DEF_UP_OURFORCES_STR:
    case AT_SKILL_DEF_UP_OURFORCES_MASTERY:
    {
        c->AttackTime = 15;
    }
    break;
    case AT_SKILL_DRAGON_KICK:
    {
        c->AttackTime = 1;
    }
    break;
    case AT_SKILL_CHAIN_DRIVE:
    case AT_SKILL_CHAIN_DRIVE_STR:
    case AT_SKILL_BEAST_UPPERCUT:
    case AT_SKILL_BEAST_UPPERCUT_STR:
    case AT_SKILL_BEAST_UPPERCUT_MASTERY:
    {
        o->m_sTargetIndex = c->TargetCharacter;
    }
    break;
    default:
        if (o->AnimationFrame >= 1.f && o->Type == MODEL_PLAYER && o->CurrentAction == PLAYER_ATTACK_SKILL_FURY_STRIKE)
        {
            c->AttackTime = 15;
        }
        else if (o->AnimationFrame >= 5.f && ((o->Type == MODEL_PLAYER && o->CurrentAction >= PLAYER_ATTACK_FIST && o->CurrentAction <= PLAYER_RIDE_SKILL) ||
            ((o->Type >= MODEL_MONSTER01 && o->Type < MODEL_MONSTER_END) && o->CurrentAction >= MONSTER01_ATTACK1 && o->CurrentAction <= MONSTER01_ATTACK2)))
        {
            int RightType = CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type;
            int LeftType = CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type;

            if (c->AttackTime >= 1 && LeftType == ITEM_SYLPH_WIND_BOW && o->Type == MODEL_PLAYER && rand_fps_check(1))
            {
                for (int i = 0; i < 20; i++)
                {
                    CreateParticle(BITMAP_SPARK + 1, o->Position, o->Angle, o->Light, 12, 2.0f);
                }
            }

            c->AttackTime = 15;
        }

        break;
    }
    return true;
}

void DeleteCloth(CHARACTER* c, OBJECT* o, PART_t* p2)
{
    if (o && o->m_pCloth)
    {
        auto* pCloth = (CPhysicsCloth*)o->m_pCloth;
        for (int i = 0; i < o->m_byNumCloth; ++i)
        {
            pCloth[i].Destroy();
        }
        delete[] pCloth;
        o->m_pCloth = NULL;
        o->m_byNumCloth = 0;
    }

    if (c)
    {
        for (int i = 0; i < MAX_BODYPART; i++)
        {
            PART_t* p = &c->BodyPart[i];
            if (p->m_pCloth[0])
            {
                for (int i = 0; i < p->m_byNumCloth; ++i)
                {
                    auto* pCloth = (CPhysicsCloth*)p->m_pCloth[i];
                    pCloth->Destroy();
                    delete pCloth;
                    p->m_pCloth[i] = NULL;
                }
            }
        }
    }

    if (p2)
    {
        if (p2->m_pCloth[0])
        {
            for (int i = 0; i < p2->m_byNumCloth; ++i)
            {
                auto* pCloth = (CPhysicsCloth*)p2->m_pCloth[i];
                pCloth->Destroy();
                delete pCloth;
                p2->m_pCloth[i] = NULL;
            }
        }
    }
}

void FallingCharacter(CHARACTER* c, OBJECT* o)
{
    float Matrix[3][4];
    vec3_t  Position, p;

    Vector(0.f, 0.f, 0.f, Position);
    Vector(0.f, o->Direction[1], 0.f, p);
    AngleMatrix(o->m_vDownAngle, Matrix);
    VectorRotate(p, Matrix, Position);

    o->Direction[1] += o->Direction[0];
    o->Gravity += o->Velocity;
    o->Velocity -= o->Direction[2];
    o->Angle[0] -= 5.f;

    o->Position[0] = o->m_vDeadPosition[0] + Position[0];
    o->Position[1] = o->m_vDeadPosition[1] + Position[1];
    o->Position[2] = o->m_vDeadPosition[2] + o->Gravity;
}

void  PushingCharacter(CHARACTER* c, OBJECT* o)
{
    if (c->StormTime > 0)
    {
        o->Angle[2] += c->StormTime * 10 * FPS_ANIMATION_FACTOR;
        c->StormTime -= FPS_ANIMATION_FACTOR;
    }
    if (c->JumpTime > 0)
    {
        float Speed = 0.2f;
        if (o->Type == MODEL_CRUST)
        {
            Speed = 0.07f;
        }
        if (gMapManager.InChaosCastle() == true)
        {
            o->Position[0] += (((float)c->TargetX) * TERRAIN_SCALE - o->Position[0]) * Speed * FPS_ANIMATION_FACTOR;
            o->Position[1] += (((float)c->TargetY) * TERRAIN_SCALE - o->Position[1]) * Speed * FPS_ANIMATION_FACTOR;
            c->JumpTime += FPS_ANIMATION_FACTOR;
            if (c->JumpTime > 15)
            {
                SetPlayerStop(c);

                o->Position[0] = c->TargetX * TERRAIN_SCALE;
                o->Position[1] = c->TargetY * TERRAIN_SCALE;

                c->PositionX = c->TargetX;
                c->PositionY = c->TargetY;

                c->JumpTime = 0;
            }
        }
        else
        {
            o->Position[0] += (((float)c->TargetX + 0.5f) * TERRAIN_SCALE - o->Position[0]) * Speed + FPS_ANIMATION_FACTOR;
            o->Position[1] += (((float)c->TargetY + 0.5f) * TERRAIN_SCALE - o->Position[1]) * Speed + FPS_ANIMATION_FACTOR;
            if (o->Type != MODEL_BALL)
                o->Position[2] = RequestTerrainHeight(o->Position[0], o->Position[1]);
            c->JumpTime += FPS_ANIMATION_FACTOR;
            if (c->JumpTime > 15)
            {
                if (o->Type == MODEL_CRUST)
                    SetPlayerStop(c);

                c->JumpTime = 0;
            }
        }
    }
    if (o->Type == MODEL_BALL)
    {
        o->Position[2] += o->Gravity;
        o->Gravity -= 6.f;
        float Height = RequestTerrainHeight(o->Position[0], o->Position[1]) + 30.f;
        if (o->Position[2] < Height)
        {
            o->Position[2] = Height;
            o->Gravity = -o->Gravity * 0.4f;
        }
        VectorAdd(o->Angle, o->Direction, o->Angle);
        VectorScale(o->Direction, 0.8f, o->Direction);
    }
}

void DeadCharacterBuff(OBJECT* o)
{
    g_CharacterUnRegisterBuff(o, eDeBuff_Stun);
    g_CharacterUnRegisterBuff(o, eBuff_Cloaking);
    g_CharacterUnRegisterBuff(o, eDeBuff_Harden);
    g_CharacterUnRegisterBuff(o, eDeBuff_Sleep);
}

void DeadCharacter(CHARACTER* c, OBJECT* o, BMD* b)
{
    if (c->Dead <= 0) return;

    DeadCharacterBuff(o);

    if (o->Type == MODEL_BALI && gMapManager.WorldActive == WD_7ATLANSE)
    {
        c->Rot += 0.05f * FPS_ANIMATION_FACTOR;
    }
    else if (o->Type == MODEL_ILLUSION_OF_KUNDUN)
    {
        if (o->LifeTime >= 100) c->Rot += 0.01f * FPS_ANIMATION_FACTOR;
        else c->Rot += 0.02f * FPS_ANIMATION_FACTOR;
    }
    else
        c->Rot += 0.02f * FPS_ANIMATION_FACTOR;
    float RotTime = 1.f;
    if (c->Rot >= RotTime)
    {
        if (o->Type != MODEL_DREADFEAR)
        {
            o->Alpha = 1.f - (c->Rot - RotTime);
        }

        if (o->Alpha >= 0.01f)
            o->Position[2] -= 0.4f;
        else if (c != Hero)
        {
            o->Live = false;
            c->m_byDieType = 0;
            o->m_bActionStart = false;
            o->m_bySkillCount = 0;

            BoneManager::UnregisterBone(c);
        }
        DeleteCloth(c, o);
    }

    if (gMapManager.InBloodCastle() == true && o->m_bActionStart)
    {
        FallingCharacter(c, o);
    }
    else if (gMapManager.InChaosCastle() == true)
    {
        int startDeadTime = 25;
        if (o->m_bActionStart)
        {
            FallingCharacter(c, o);
            startDeadTime = 15;
        }
        if (c->Dead <= startDeadTime && c->Dead >= startDeadTime - 10 && (((int)c->Dead) % 2))
        {
            vec3_t Position;

            VectorCopy(o->Position, Position);

            Position[0] += rand() % 160 - 80.f;
            Position[1] += rand() % 160 - 80.f;
            Position[2] += rand() % 160 - 80.f + 50.f;
            CreateBomb(Position, true);
        }
    }
    else
    {
        switch (c->m_byDieType)
        {
        case AT_SKILL_NOVA:
        case AT_SKILL_COMBO:
            if (c->Dead < 15)
            {
                o->Direction[1] -= o->Velocity;
                o->Velocity += 1.f;

                if (o->Direction[1] < 0)
                {
                    o->Direction[1] = 0.f;
                }

                MoveParticle(o, o->HeadAngle);
            }

            if (c->Dead <= 30 && c->m_byDieType == AT_SKILL_NOVA && rand_fps_check(1))
            {
                vec3_t Light, p, Position;
                Vector(0.3f, 0.3f, 1.f, Light);
                Vector(0.f, 0.f, 0.f, p);
                for (int i = 0; i < 10; i++)
                {
                    b->TransformPosition(o->BoneTransform[rand() % 32], p, Position, true);
                    CreateParticle(BITMAP_LIGHT, Position, o->Angle, Light, 5, 0.5f + (rand() % 100) / 50.f);
                }
            }
            break;
        }
    }
    if (SceneFlag == MAIN_SCENE && (gMapManager.WorldActive == WD_7ATLANSE || gMapManager.WorldActive == WD_67DOPPLEGANGER3) && rand_fps_check(1))
    {
        for (int i = 0; i < 4; i++)
        {
            vec3_t Position;
            Vector((float)(rand() % 128 - 64), (float)(rand() % 128 - 64), (float)(rand() % 256), Position);
            VectorAdd(Position, o->Position, Position);
            CreateParticle(BITMAP_BUBBLE, Position, o->Angle, o->Light);
        }
    }
}

void CalcStopTime(void)
{
    if (gMapManager.InChaosCastle() == false) return;

    if ((Hero->PositionX) != g_iOldPositionX || (Hero->PositionY) != g_iOldPositionY)
    {
        g_iOldPositionX = (Hero->PositionX);
        g_iOldPositionY = (Hero->PositionY);

        g_fStopTime = WorldTime;
        return;
    }

    float fStopTime = ((WorldTime - g_fStopTime) / CLOCKS_PER_SEC);
    if (fStopTime >= 10)
    {
        int index = TERRAIN_INDEX_REPEAT(g_iOldPositionX, g_iOldPositionY);

        if ((TerrainWall[index] & TW_NOGROUND) == TW_NOGROUND)
        {
            SocketClient->ToGameServer()->SendChaosCastlePositionSet(g_iOldPositionX, g_iOldPositionY);
        }
        g_fStopTime = WorldTime;
    }
}

void HeroAttributeCalc(CHARACTER* c)
{
    if (c != Hero) return;

    if (CharacterAttribute->AbilityTime[0] > 0)
    {
        CharacterAttribute->AbilityTime[0] -= FPS_ANIMATION_FACTOR;
    }
    if (CharacterAttribute->AbilityTime[0] <= 0)
    {
        CharacterAttribute->Ability &= (~ABILITY_FAST_ATTACK_SPEED);
    }
    if (CharacterAttribute->AbilityTime[1] > 0)
    {
        CharacterAttribute->AbilityTime[1] -= FPS_ANIMATION_FACTOR;
    }
    if (CharacterAttribute->AbilityTime[1] <= 0)
    {
        CharacterAttribute->Ability &= (~ABILITY_PLUS_DAMAGE);
        CharacterMachine->CalculateDamage();
        CharacterMachine->CalculateMagicDamage();
        CharacterMachine->CalculateCurseDamage();
    }
    if (CharacterAttribute->AbilityTime[2] > 0)
    {
        CharacterAttribute->AbilityTime[2] -= FPS_ANIMATION_FACTOR;
    }
    if (CharacterAttribute->AbilityTime[2] <= 0)
    {
        CharacterAttribute->Ability &= (~ABILITY_FAST_ATTACK_SPEED2);
    }
}

void OnlyNpcChatProcess(CHARACTER* c, OBJECT* o)
{
    if (o->Kind == KIND_NPC && rand_fps_check(2))
    {
        switch (o->Type)
        {
        case MODEL_MERCHANT_GIRL:
            if (gMapManager.InBattleCastle() == false)
            {
                CreateChat(c->ID, GlobalText[1974], c);
            }
            break;
        case MODEL_ELF_WIZARD:
            CreateChat(c->ID, GlobalText[1975], c);
            break;
        case MODEL_MASTER:
            CreateChat(c->ID, GlobalText[1976], c);
            break;
        case MODEL_PLAYER:
            if (c->MonsterIndex == MONSTER_ELF_SOLDIER)
                CreateChat(c->ID, GlobalText[1827], c);
            break;
        }
    }
}

void PlayerNpcStopAnimationSetting(CHARACTER* c, OBJECT* o)
{
    int action = rand() % 100;

    if (o->CurrentAction != PLAYER_STOP_MALE)
    {
        SetAction(&c->Object, PLAYER_STOP_MALE);
    }
    else
    {
        if (action < 80)
        {
            SetAction(&c->Object, PLAYER_STOP_MALE);
        }
        else if (action < 85)
        {
            SetAction(&c->Object, PLAYER_CLAP1);
        }
        else if (action < 90)
        {
            SetAction(&c->Object, PLAYER_CHEER1);
        }
        else if (action < 95)
        {
            SetAction(&c->Object, PLAYER_SEE1);
        }
        else if (action < 100)
        {
            SetAction(&c->Object, PLAYER_UNKNOWN1);
        }

        int TextIndex = 0;
        if (gMapManager.WorldActive == WD_2DEVIAS)
        {
            int iTextIndices[] = { 904, 905 };
            int iNumTextIndices = 2;

            static int s_iTextIndex = iTextIndices[0];
            if (o->CurrentAction != o->PriorAction)
                s_iTextIndex = iTextIndices[rand() % iNumTextIndices];
            TextIndex = s_iTextIndex;
        }
        else if (gMapManager.WorldActive == WD_0LORENCIA)
        {
            int iTextIndices[] = { 823 };
            int iNumTextIndices = 1;
            static int s_iTextIndex = iTextIndices[0];
            if (o->CurrentAction != o->PriorAction)
                s_iTextIndex = iTextIndices[rand() % iNumTextIndices];
            TextIndex = s_iTextIndex;
        }

        wchar_t szText[512];
        swprintf(szText, GlobalText[TextIndex]);
        CreateChat(c->ID, szText, c);
    }
}

void PlayerStopAnimationSetting(CHARACTER* c, OBJECT* o)
{
    if (o->CurrentAction == PLAYER_DIE1 || o->CurrentAction == PLAYER_DIE2)
    {
        if (!c->Blood)
        {
            c->Blood = true;
            CreateBlood(o);
        }
        return;
    }

    if (o->CurrentAction < PLAYER_WALK_MALE ||
        (o->CurrentAction >= PLAYER_PROVOCATION && o->CurrentAction <= PLAYER_CHEERS) ||
        (o->CurrentAction >= PLAYER_IDLE1_DARKHORSE && o->CurrentAction <= PLAYER_IDLE2_DARKHORSE) ||
        ((o->CurrentAction >= PLAYER_SKILL_THRUST && o->CurrentAction <= PLAYER_RAGE_FENRIR_ATTACK_RIGHT) &&
            !(o->CurrentAction >= PLAYER_RAGE_FENRIR_RUN && o->CurrentAction <= PLAYER_RAGE_FENRIR_RUN_ONE_LEFT) &&
            !(o->CurrentAction >= PLAYER_RAGE_UNI_RUN && o->CurrentAction <= PLAYER_RAGE_UNI_STOP_ONE_RIGHT)) ||
        o->CurrentAction == PLAYER_STOP_RAGEFIGHTER ||
        (o->CurrentAction >= PLAYER_ATTACK_FIST && o->CurrentAction <= PLAYER_SHOCK
            && o->CurrentAction != PLAYER_WALK_TWO_HAND_SWORD_TWO && o->CurrentAction != PLAYER_RUN_TWO_HAND_SWORD_TWO
            && o->CurrentAction != PLAYER_FLY_RIDE && o->CurrentAction != PLAYER_FLY_RIDE_WEAPON
            && o->CurrentAction != PLAYER_SKILL_HELL_BEGIN
            && o->CurrentAction != PLAYER_DARKLORD_WALK && o->CurrentAction != PLAYER_RUN_RIDE_HORSE
            && (o->CurrentAction < PLAYER_FENRIR_RUN || o->CurrentAction > PLAYER_FENRIR_RUN_ONE_LEFT_ELF)
            && o->CurrentAction != PLAYER_RECOVER_SKILL
            ))

        SetPlayerStop(c);

    if (o->CurrentAction == PLAYER_SKILL_HELL_BEGIN)
    {
        o->AnimationFrame = 0;
    }
}

void EtcStopAnimationSetting(CHARACTER* c, OBJECT* o)
{
    if (o->Type == MODEL_WARCRAFT)
    {
        o->CurrentAction = 1;
    }
    else
        if (battleCastle::StopBattleCastleMonster(c, o) == true)
        {
            CharacterAnimation(c, o);
            return;
        }
        else
            if (gMapManager.WorldActive == WD_1DUNGEON && o->Type == 40)
            {
                SetAction(&c->Object, 0);
            }
            else if (o->Type >= MODEL_MONSTER01 && o->Type < MODEL_MONSTER_END)
            {
                if (o->CurrentAction == MONSTER01_DIE)
                {
                    if (!c->Blood)
                    {
                        c->Blood = true;
                        CreateBlood(o);
                    }
                    return;
                }
                else if (o->CurrentAction == MONSTER01_STOP2 || o->CurrentAction == MONSTER01_SHOCK
                    || o->CurrentAction == MONSTER01_ATTACK1 || o->CurrentAction == MONSTER01_ATTACK2
                    || o->CurrentAction == MONSTER01_ATTACK3 || o->CurrentAction == MONSTER01_ATTACK4
                    || o->CurrentAction == MONSTER01_ATTACK5
                    )
                {
                    SetAction(o, MONSTER01_STOP1);
                }

                if (o->CurrentAction == MONSTER01_APEAR && (o->Type == MODEL_MAYA_HAND_LEFT || o->Type == MODEL_MAYA_HAND_RIGHT || o->Type == MODEL_SELUPAN))
                {
                    if (o->Type == MODEL_SELUPAN)
                    {
                        o->CurrentAction = MONSTER01_STOP1;
                    }
                    SetAction(o, MONSTER01_STOP1);
                }
            }
            else
            {
                switch (o->Type)
                {
                case MODEL_WEDDING_NPC:
                    if ((rand() % 16 < 4) && o->SubType == 0)
                    {
                        SetAction(o, 1);
                        o->SubType = 1;
                    }
                    else
                    {
                        SetAction(o, 0);
                        o->SubType = 0;
                    }
                    break;
                case MODEL_SMITH:
                case MODEL_SCIENTIST:
                    if (rand() % 16 < 12)
                        SetAction(o, 0);
                    else
                        SetAction(o, rand() % 2 + 1);
                    break;
                case MODEL_FACE:
                case MODEL_FACE + 1:
                case MODEL_FACE + 2:
                case MODEL_FACE + 3:
                case MODEL_FACE + 4:
                case MODEL_FACE + 5:
                case MODEL_FACE + 6:
                    break;
                case MODEL_ELBELAND_SILVIA:
                case MODEL_ELBELAND_RHEA:
                    if (rand() % 5 < 4 || o->CurrentAction == 1)
                        SetAction(o, 0);
                    else
                        SetAction(o, 1);
                    break;
                case MODEL_NPC_DEVIN:
                    if (rand() % 5 < 4)
                        SetAction(o, 0);
                    else
                        SetAction(o, 1);
                    break;

                case MODEL_SEED_MASTER:
                    if (rand() % 3 < 2 || o->CurrentAction != 0)
                        SetAction(o, 0);
                    else
                    {
                        SetAction(o, rand() % 3 + 1);
                    }
                    break;
                case MODEL_SEED_INVESTIGATOR:
                    if (rand() % 3 < 2 || o->CurrentAction == 1)
                        SetAction(o, 0);
                    else
                        SetAction(o, 1);
                    break;
                case MODEL_LITTLESANTA:
                case MODEL_LITTLESANTA + 1:
                case MODEL_LITTLESANTA + 2:
                case MODEL_LITTLESANTA + 3:
                    if (rand() % 5 < 2)
                        SetAction(o, 0);
                    else
                        SetAction(o, rand() % 3 + 2);
                    break;

                case MODEL_LITTLESANTA + 4:
                case MODEL_LITTLESANTA + 5:
                case MODEL_LITTLESANTA + 6:
                case MODEL_LITTLESANTA + 7:
                    if (rand() % 5 < 2)
                        SetAction(o, 1);
                    else
                        SetAction(o, rand() % 3 + 2);
                    break;
                case MODEL_XMAS2008_SANTA_NPC:
                    if (rand() % 3 < 2 || (o->CurrentAction == 1 || o->CurrentAction == 2))
                    {
                        SetAction(o, 0);
                    }
                    else
                    {
                        SetAction(o, rand() % 2 + 1);
                    }
                    break;
                case MODEL_XMAS2008_SNOWMAN_NPC:
                    SetAction(o, 0);
                    break;
                case MODEL_GAMBLE_NPC_MOSS:
                    if (rand() % 5 < 4 || o->CurrentAction == 1)
                    {
                        SetAction(o, 0);
                    }
                    else
                    {
                        SetAction(o, 1);
                    }
                    break;
                case MODAL_GENS_NPC_DUPRIAN:
                case MODAL_GENS_NPC_BARNERT:
                    if (rand() % 5 < 4)
                        SetAction(o, 0);
                    else
                        SetAction(o, rand() % 2 + 1);
                    break;
                case MODEL_UNITEDMARKETPLACE_RAUL:
                case MODEL_UNITEDMARKETPLACE_JULIA:
                    if (rand() % 5 < 4)
                        SetAction(o, 0);
                    else
                        SetAction(o, rand() % 2 + 1);
                    break;
                case MODEL_UNITEDMARKETPLACE_CHRISTIN:
                case MODEL_KARUTAN_NPC_REINA:
                    if (rand() % 5 < 3)
                        SetAction(o, 0);
                    else
                        SetAction(o, rand() % 2 + 1);
                    break;
                case MODEL_KARUTAN_NPC_VOLVO:
                    if (rand() % 5 < 4)
                        SetAction(o, 0);
                    else
                        SetAction(o, 1);
                    break;
                case MODEL_LUCKYITEM_NPC:
                    if (rand() % 5 < 4 || o->CurrentAction == 1)
                    {
                        SetAction(o, 0);
                    }
                    else
                    {
                        SetAction(o, 1);
                    }
                    break;
                default:
                    SetAction(o, rand() % 2);
                    break;
                }
            }
}

void AnimationCharacter(CHARACTER* c, OBJECT* o, BMD* b)
{
    bool bEventNpc = false;
    if (o->Kind == KIND_NPC && (gMapManager.WorldActive == WD_0LORENCIA || gMapManager.WorldActive == WD_2DEVIAS) && o->Type == MODEL_PLAYER && (o->SubType >= MODEL_SKELETON1 && o->SubType <= MODEL_SKELETON3))
    {
        if (gMapManager.WorldActive == WD_0LORENCIA)
        {
            Vector(0.f, 0.f, 90.f, o->Angle);
        }
        else
        {
            Vector(0.f, 0.f, 0.f, o->Angle);
        }

        bEventNpc = true;
    }

    OnlyNpcChatProcess(c, o);

    bool Play = CharacterAnimation(c, o);

    if (!Play)
    {
        c->LongRangeAttack = -1;
        c->Foot[0] = false;
        c->Foot[1] = false;
        if (o->Type == MODEL_PLAYER)
        {
            if (bEventNpc)
            {
                PlayerNpcStopAnimationSetting(c, o);
            }
            else
            {
                PlayerStopAnimationSetting(c, o);
            }
        }
        else
        {
            EtcStopAnimationSetting(c, o);
        }
        if (o->CurrentAction == MONSTER01_STOP1 || o->CurrentAction == MONSTER01_STOP2)
            PlayMonsterSound(o);

        if (o->Type == MODEL_WARCRAFT)
        {
            o->AnimationFrame = 8.f;
        }
    }

    switch (o->Type)
    {
    case MODEL_DEVIAS_TRADER:
        if (b->CurrentAnimationFrame == b->Actions[o->CurrentAction].NumAnimationKeys - 1)
        {
            if (rand_fps_check(32))
                SetAction(o, 1);
            else
                SetAction(o, 0);
        }
        break;
    case MODEL_RABBIT:
        if (o->CurrentAction <= 1 && b->CurrentAnimationFrame == b->Actions[o->CurrentAction].NumAnimationKeys - 1)
        {
            if (rand_fps_check(10))
                SetAction(o, 1);
            else
                SetAction(o, 0);
        }
        break;
    }
}

void CreateWeaponBlur(CHARACTER* c, OBJECT* o, BMD* b)
{
    if (o->AnimationFrame >= 3.f
        || (o->CurrentAction == PLAYER_ATTACK_TWO_HAND_SWORD_TWO)
        )
    {
        int Hand = 0;
        int Hand2 = 0;
        if (o->Type == MODEL_PLAYER)
        {
            if (o->CurrentAction == PLAYER_ATTACK_SWORD_LEFT1 || o->CurrentAction == PLAYER_ATTACK_SWORD_LEFT2)
            {
                Hand = 1;
                Hand2 = 1;
            }
        }
        int BlurType = 0;
        int BlurMapping = 0;
        int Type = c->Weapon[Hand].Type;
        int Level = c->Weapon[Hand].Level;
        if (o->Type == MODEL_PLAYER)
        {
            if (o->CurrentAction == PLAYER_ATTACK_ONE_FLASH || o->CurrentAction == PLAYER_ATTACK_RUSH)
            {
                BlurType = 1;
                BlurMapping = 2;
            }
            else if (o->CurrentAction == PLAYER_ATTACK_SKILL_SWORD2 || o->CurrentAction == PLAYER_ATTACK_SKILL_SWORD3 || o->CurrentAction == PLAYER_ATTACK_SKILL_SWORD4)
            {
                BlurType = 1;
                if (Type == MODEL_LIGHTING_SWORD || Type == MODEL_DARK_REIGN_BLADE || Type == MODEL_RUNE_BLADE)
                    BlurMapping = 1;
                else
                    BlurMapping = 2;
            }
            else if (o->CurrentAction == PLAYER_ATTACK_STRIKE)
            {
                BlurType = 1;
                BlurMapping = 2;
            }
            else if (o->CurrentAction == PLAYER_SKILL_LIGHTNING_ORB
                || o->CurrentAction == PLAYER_SKILL_LIGHTNING_ORB_UNI
                || o->CurrentAction == PLAYER_SKILL_LIGHTNING_ORB_DINO
                || o->CurrentAction == PLAYER_SKILL_LIGHTNING_ORB_FENRIR)
            {
                BlurType = 1;
                BlurMapping = 1;
            }
            else if (o->CurrentAction == PLAYER_SKILL_BLOW_OF_DESTRUCTION && o->AnimationFrame >= 2.f && o->AnimationFrame <= 8.f)
            {
                BlurType = 1;
                BlurMapping = 2;
            }
            else if (o->CurrentAction == PLAYER_ATTACK_SKILL_SWORD5)
            {
                BlurType = 1;
                if (Type == MODEL_CRYSTAL_SWORD)
                    BlurMapping = 1;
                else
                    BlurMapping = 2;
            }
            else if (Type >= MODEL_SWORD && Type < MODEL_SWORD + MAX_ITEM_INDEX)
            {
                if ((o->CurrentAction >= PLAYER_ATTACK_SWORD_RIGHT1 && o->CurrentAction <= PLAYER_ATTACK_TWO_HAND_SWORD3)
                    || o->CurrentAction == PLAYER_ATTACK_TWO_HAND_SWORD_TWO
                    )
                {
                    BlurType = 1;
                    if (Type == MODEL_DARK_BREAKER)
                    {
                        BlurMapping = 6;
                    }
                    else if (o->CurrentAction == PLAYER_ATTACK_TWO_HAND_SWORD3
                        || o->CurrentAction == PLAYER_ATTACK_TWO_HAND_SWORD_TWO)
                    {
                        if (Type == MODEL_SWORD_DANCER)
                            BlurMapping = 2;
                        else
                            BlurMapping = 1;
                    }
                }
            }
            else if (Type == MODEL_TOMAHAWK || Type >= MODEL_BATTLE_AXE && Type < MODEL_MACE + MAX_ITEM_INDEX)
            {
                if (o->CurrentAction >= PLAYER_ATTACK_SKILL_SWORD1 && o->CurrentAction <= PLAYER_ATTACK_SKILL_SWORD5)
                {
                    BlurType = 1;
                    BlurMapping = 2;
                }
            }
            else if (Type >= MODEL_SPEAR && Type < MODEL_SPEAR + MAX_ITEM_INDEX)
            {
                if (o->CurrentAction >= PLAYER_ATTACK_SPEAR1 && o->CurrentAction <= PLAYER_ATTACK_SCYTHE3)
                {
                    BlurType = 3;
                    if (Type == MODEL_DRAGON_SPEAR)
                    {
                        BlurType = 1;
                        BlurMapping = 0;
                    }
                    else if (o->CurrentAction == PLAYER_ATTACK_SCYTHE3)
                        BlurMapping = 1;
                }
            }
        }
        else
        {
            if (c->MonsterIndex == MONSTER_MEGA_CRUST || c->MonsterIndex == MONSTER_ALPHA_CRUST || c->MonsterIndex == MONSTER_OMEGA_WING)
            {
                if (o->CurrentAction >= MONSTER01_ATTACK1 && o->CurrentAction <= MONSTER01_ATTACK2)
                {
                    BlurType = 1;
                    BlurMapping = 6;
                }
            }
            else if (o->Type == MODEL_AEGIS)
            {
                if (o->CurrentAction == MONSTER01_ATTACK1)
                {
                    BlurType = 5;
                    BlurMapping = 2;
                    Hand = 0;
                    Hand2 = 1;
                }
            }
            else if (o->Type == MODEL_DEATH_CENTURION)
            {
                if (o->CurrentAction >= MONSTER01_ATTACK1 && o->CurrentAction <= MONSTER01_ATTACK2)
                {
                    BlurType = 4;
                    BlurMapping = 0;
                    Level = 99;
                }
            }
            else if (o->Type == MODEL_SHRIKER)
            {
                if (o->CurrentAction >= MONSTER01_ATTACK1 && o->CurrentAction <= MONSTER01_ATTACK2)
                {
                    if (o->SubType == 9)
                    {
                        BlurType = 1;
                        BlurMapping = 2;
                    }
                    else
                    {
                        BlurType = 1;
                        BlurMapping = 0;
                        Level = 99;
                        Type = 0;
                    }
                }
            }
            else if (Type >= MODEL_SWORD && Type < MODEL_SWORD + MAX_ITEM_INDEX)
            {
                if (o->CurrentAction >= MONSTER01_ATTACK1 && o->CurrentAction <= MONSTER01_ATTACK2)
                    BlurType = 1;
            }
        }
        if (BlurType > 0)
        {
            vec3_t  Light;
            vec3_t  Pos1, Pos2;
            vec3_t  p, p2;
            switch (BlurType)
            {
            case 1:Vector(0.f, -20.f, 0.f, Pos1); break;
            case 2:Vector(0.f, -80.f, 0.f, Pos1); break;
            case 3:Vector(0.f, -100.f, 0.f, Pos1); break;
            }
            Vector(0.f, -120.f, 0.f, Pos2);

            if (BlurType == 4)
            {
                Vector(0.f, 0.f, 0.f, Pos1);
                Vector(0.f, -200.f, 0.f, Pos2);
            }
            else if (BlurType == 5)
            {
                Vector(0.f, 0.f, 0.f, Pos1);
                Vector(0.f, -20.f, 0.f, Pos2);
            }

            if (Type == MODEL_DOUBLE_BLADE || Type == MODEL_CHAOS_DRAGON_AXE || Type == MODEL_BILL_OF_BALROG)
            {
                Vector(1.f, 0.2f, 0.2f, Light);
            }
            else if (Level == 99)
            {
                Vector(0.3f, 0.2f, 1.f, Light);
            }
            else if (BlurMapping == 0)
            {
                if (Level >= 7)
                {
                    Vector(1.f, 0.6f, 0.2f, Light);
                }
                else if (Level >= 5)
                {
                    Vector(0.2f, 0.4f, 1.f, Light);
                }
                else if (Level >= 3)
                {
                    Vector(1.f, 0.2f, 0.2f, Light);
                }
                else
                {
                    Vector(0.8f, 0.8f, 0.8f, Light);
                }
            }
            else
            {
                Vector(1.f, 1.f, 1.f, Light);
            }

            if ((o->Type != MODEL_PLAYER || Type == MODEL_KATACHE || Type == MODEL_GLADIUS || Type == MODEL_SWORD_OF_SALAMANDER || Type == MODEL_LEGENDARY_SWORD || Type == MODEL_SERPENT_SPEAR)
                && o->Type != MODEL_AEGIS && o->Type != MODEL_DEATH_CENTURION && o->Type != MODEL_SHRIKER
                )
            {
                b->TransformPosition(o->BoneTransform[c->Weapon[Hand].LinkBone], Pos1, p, true);
                b->TransformPosition(o->BoneTransform[c->Weapon[Hand2].LinkBone], Pos2, p2, true);
                CreateBlur(c, p, p2, Light, BlurMapping);
            }
            else if (g_CMonkSystem.IsSwordformGloves(Type))
            {
                g_CMonkSystem.MoveBlurEffect(c, o, b);
            }
            else
            {
                constexpr float inter = 10.f;
                const float playSpeed = b->Actions[b->CurrentAction].PlaySpeed * FPS_ANIMATION_FACTOR;
                float animationFrame = o->AnimationFrame - playSpeed;
                const float priorAnimationFrame = o->PriorAnimationFrame;
                const float animationSpeed = playSpeed / inter;
                for (int i = 0; i < (int)(inter); ++i)
                {
                    b->Animation(BoneTransform, animationFrame, priorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);

                    b->TransformPosition(BoneTransform[c->Weapon[Hand].LinkBone], Pos1, p, false);
                    b->TransformPosition(BoneTransform[c->Weapon[Hand2].LinkBone], Pos2, p2, false);

                    if (o->Type == MODEL_AEGIS && i % 2)
                    {
                        CreateParticle(BITMAP_FIRE + 3, p2, o->Angle, Light, 12);
                    }

                    if (c->Weapon[0].Type != -1 || c->Weapon[1].Type != -1)
                        CreateBlur(c, p, p2, Light, BlurMapping, true);

                    animationFrame += animationSpeed;
                }
            }
        }
        M31HuntingGround::MoveHuntingGroundBlurEffect(c, o, b);
        M34CryingWolf2nd::MoveCryingWolf2ndBlurEffect(c, o, b);
        M34CryWolf1st::MoveCryWolf1stBlurEffect(c, o, b);
        M33Aida::MoveAidaBlurEffect(c, o, b);
        M37Kanturu1st::MoveKanturu1stBlurEffect(c, o, b);
        M38Kanturu2nd::Move_Kanturu2nd_BlurEffect(c, o, b);
        M39Kanturu3rd::MoveKanturu3rdBlurEffect(c, o, b);
        SEASON3A::CGM3rdChangeUp::Instance().MoveBalgasBarrackBlurEffect(c, o, b);
        g_CursedTemple->MoveBlurEffect(c, o, b);
        SEASON3B::GMNewTown::MoveBlurEffect(c, o, b);
        SEASON3C::GMSwampOfQuiet::MoveBlurEffect(c, o, b);

        TheMapProcess().MoveBlurEffect(c, o, b);
    }
    else
    {
        VectorCopy(o->Position, o->StartPosition);
    }
}

void MoveCharacter(CHARACTER* c, OBJECT* o)
{
    if (o->Type == MODEL_WARCRAFT)
    {
        wchar_t Text[100];
        wchar_t ID[100];
        swprintf(ID, L"%s .", c->ID);
        swprintf(Text, GlobalText[1176], c->Level);
        wcscat(ID, Text);
        AddObjectDescription(ID, o->Position);
    }

    BMD* b = &Models[o->Type];
    VectorCopy(o->Position, b->BodyOrigin);
    b->BodyScale = o->Scale;
    b->CurrentAction = o->CurrentAction;

    CalcStopTime();
    HeroAttributeCalc(c);
    PushingCharacter(c, o);
    DeadCharacter(c, o, b);
    Alpha(o);

    if (c->Freeze > 0.f)
    {
        c->Freeze -= 0.03f;
    }

    AnimationCharacter(c, o, b);

    if (c->Dead > 0)
    {
        c->Dead += FPS_ANIMATION_FACTOR;
        if (c->Dead >= 15)
        {
            SetPlayerDie(c);
        }
        if (gMapManager.InBloodCastle() && o->m_bActionStart)
        {
            SetPlayerDie(c);
        }
    }

    vec3_t p, Position;
    vec3_t Light;
    float Luminosity = (float)(rand() % 6 + 2) * 0.1f;

    Vector(0.f, 0.f, 0.f, p);
    Vector(1.f, 1.f, 1.f, Light);

    if (gMapManager.InBattleCastle() == false && o->m_byHurtByDeathstab > 0)
    {
        if (rand_fps_check(2))
        {
            vec3_t pos1, pos2;

            Vector(0.f, 0.f, 0.f, p);
            for (int i = 0; i < b->NumBones; ++i)
            {
                if (!b->Bones[i].Dummy)
                {
                    int iParent = b->Bones[i].Parent;
                    if (iParent > -1 && iParent < b->NumBones)
                    {
                        b->TransformPosition(o->BoneTransform[i], p, pos1, true);
                        b->TransformPosition(o->BoneTransform[iParent], p, pos2, true);

                        GetNearRandomPos(pos1, 20, pos1);
                        GetNearRandomPos(pos2, 20, pos2);
                        CreateJoint(BITMAP_JOINT_THUNDER, pos1, pos2, o->Angle, 7, NULL, 20.f);
                    }
                }
            }
        }
        o->m_byHurtByDeathstab -= FPS_ANIMATION_FACTOR;
    }

    if ((o->CurrentAction == PLAYER_ATTACK_TELEPORT || o->CurrentAction == PLAYER_ATTACK_RIDE_TELEPORT
        || o->CurrentAction == PLAYER_FENRIR_ATTACK_DARKLORD_TELEPORT)
        && o->AnimationFrame > 5.5f
        && rand_fps_check(1))
    {
        Vector(0.f, 0.f, 0.f, p);
        Vector(0.3f, 0.5f, 1.f, Light);
        b->TransformPosition(o->BoneTransform[42], p, Position, true);
        CreateParticle(BITMAP_LIGHT, Position, o->Angle, Light);
    }

    if ((int)c->AttackTime > 0)
    {
        AttackStage(c, o);
        AttackEffect(c);
        c->AttackTime += FPS_ANIMATION_FACTOR;
    }

    if ((int)c->AttackTime >= g_iLimitAttackTime)
    {
        c->AttackTime = 0;
        c->LastAttackEffectTime = -1;
        o->PKKey = getTargetCharacterKey(c, SelectedCharacter);

        switch ((c->Skill))
        {
        case AT_SKILL_SUMMON:
        case AT_SKILL_SUMMON + 1:
        case AT_SKILL_SUMMON + 2:
        case AT_SKILL_SUMMON + 3:
        case AT_SKILL_SUMMON + 4:
        case AT_SKILL_SUMMON + 5:
        case AT_SKILL_SUMMON + 6:
#ifdef ADD_ELF_SUMMON
        case AT_SKILL_SUMMON + 7:
#endif// ADD_ELF_SUMMON
            CreateEffect(BITMAP_MAGIC + 1, o->Position, o->Angle, o->Light, 3, o);
            break;
        case AT_SKILL_RAGEFUL_BLOW:
        case AT_SKILL_RAGEFUL_BLOW_STR:
        case AT_SKILL_RAGEFUL_BLOW_MASTERY:
        {
            o->Weapon = c->Weapon[0].Type - MODEL_SWORD;
            o->WeaponLevel = (BYTE)c->Weapon[0].Level;
            CreateEffect(MODEL_SKILL_FURY_STRIKE, o->Position, o->Angle, o->Light, 0, o, o->PKKey, FindHotKey((c->Skill)));
            PlayBuffer(SOUND_FURY_STRIKE1);
            break;
        }
        case AT_SKILL_STRIKE_OF_DESTRUCTION:
        case AT_SKILL_STRIKE_OF_DESTRUCTION_STR:
            o->Weapon = c->Weapon[0].Type - MODEL_SWORD;
            o->WeaponLevel = (BYTE)c->Weapon[0].Level;
            Vector(0.f, 0.f, 0.f, o->Light);
            o->Light[0] = (float)(c->SkillX + 0.5f) * TERRAIN_SCALE;
            o->Light[1] = (float)(c->SkillY + 0.5f) * TERRAIN_SCALE;
            o->Light[2] = o->Position[2];
            CreateEffect(MODEL_BLOW_OF_DESTRUCTION, o->Position, o->Angle, o->Light, 0, o);
            PlayBuffer(SOUND_SKILL_BLOWOFDESTRUCTION);
            break;
        case AT_SKILL_FIRE_SLASH:
        case AT_SKILL_FIRE_SLASH_STR:
            o->Weapon = c->Weapon[0].Type - MODEL_SWORD;
            o->WeaponLevel = (BYTE)c->Weapon[0].Level;
            break;
        case AT_SKILL_POWER_SLASH:
        case AT_SKILL_POWER_SLASH_STR:
            o->Weapon = c->Weapon[0].Type - MODEL_SWORD;
            o->WeaponLevel = (BYTE)c->Weapon[0].Level;
            break;
        case AT_SKILL_SWELL_LIFE:
        case AT_SKILL_SWELL_LIFE_STR:
        case AT_SKILL_SWELL_LIFE_PROFICIENCY:
        {
            vec3_t Angle = { 0.0f, 0.0f, 0.0f };
            int iCount = 36;
            for (int i = 0; i < iCount; ++i)
            {
                Angle[0] = -10.f;
                Angle[1] = 0.f;
                Angle[2] = i * 10.f;
                vec3_t Position;
                VectorCopy(o->Position, Position);
                Position[2] += 100.f;
                CreateJoint(BITMAP_JOINT_SPIRIT, Position, Position, Angle, 2, o, 60.f, 0, 0);

                if ((i % 20) == 0)
                {
                    CreateEffect(BITMAP_MAGIC + 1, o->Position, Angle, o->Light, 4, o);
                }
            }
        }
        PlayBuffer(SOUND_SWELLLIFE);
        break;
        case AT_SKILL_STUN:
            //            CreateEffect ( MODEL_STUN_STONE, o->Position, o->Angle, o->Light, 1 );
            CreateJoint(BITMAP_FLASH, o->Position, o->Position, o->Angle, 7, NULL);

            PlayBuffer(SOUND_BMS_STUN);
            break;

        case AT_SKILL_REMOVAL_STUN:
        {
            if (c->TargetCharacter != -1)
            {
                CHARACTER* tc = &CharactersClient[c->TargetCharacter];
                OBJECT* to = &tc->Object;
                if (to != o)
                {
                    VectorCopy(to->Position, Position);
                    Position[2] += 1200.f;
                    CreateJoint(BITMAP_FLASH, Position, Position, to->Angle, 0, to, 120.f);

                    PlayBuffer(SOUND_BMS_STUN_REMOVAL);
                }
            }
        }
        break;

        case AT_SKILL_MANA:
        {
            vec3_t Angle = { 0.0f, 0.0f, 0.0f };
            int iCount = 36;

            for (int i = 0; i < iCount; ++i)
            {
                Angle[0] = -10.f;
                Angle[1] = 0.f;
                Angle[2] = i * 10.f;
                vec3_t Position;
                VectorCopy(o->Position, Position);
                Position[2] += 100.f;
                CreateJoint(BITMAP_JOINT_SPIRIT, Position, Position, Angle, 21, o, 60.f, 0, 0);
                if ((i % 20) == 0)
                    CreateEffect(BITMAP_MAGIC + 1, o->Position, Angle, Light, 10, o);
            }
        }
        PlayBuffer(SOUND_BMS_MANA);
        break;

        case AT_SKILL_INVISIBLE:
        {
            if (c->TargetCharacter != -1)
            {
                CHARACTER* tc = &CharactersClient[c->TargetCharacter];
                OBJECT* to = &tc->Object;

                DeleteJoint(MODEL_SPEARSKILL, to, 4);
                DeleteJoint(MODEL_SPEARSKILL, to, 9);
                //				if(to != o)
                //				{
                CreateEffect(BITMAP_MAGIC + 1, to->Position, to->Angle, to->Light, 6, to);
                PlayBuffer(SOUND_BMS_INVISIBLE);
                //				}
            }
        }
        break;
        case AT_SKILL_REMOVAL_INVISIBLE:
        {
            if (c->TargetCharacter != -1)
            {
                CHARACTER* tc = &CharactersClient[c->TargetCharacter];
                OBJECT* to = &tc->Object;

                if (to != o)
                {
                    VectorCopy(to->Position, Position);
                    Position[2] += 1200.f;
                    CreateJoint(BITMAP_FLASH, Position, Position, to->Angle, 1, to, 120.f);

                    PlayBuffer(SOUND_BMS_STUN_REMOVAL);
                }
            }
        }
        break;
        case AT_SKILL_REMOVAL_BUFF:
        {
            vec3_t Angle;
            vec3_t Position;
            VectorCopy(o->Position, Position);
            Position[2] += 100.f;

            std::list<eBuffState> bufflist;

            //debuff
            bufflist.push_back(eDeBuff_Poison);
            bufflist.push_back(eDeBuff_Freeze);
            bufflist.push_back(eDeBuff_Harden);
            bufflist.push_back(eDeBuff_Defense);
            bufflist.push_back(eDeBuff_Stun);
            bufflist.push_back(eDeBuff_Sleep);
            bufflist.push_back(eDeBuff_BlowOfDestruction);

            //buff
            bufflist.push_back(eBuff_Life); bufflist.push_back(eBuff_Attack);
            bufflist.push_back(eBuff_Defense); bufflist.push_back(eBuff_AddAG);
            bufflist.push_back(eBuff_Cloaking); bufflist.push_back(eBuff_AddSkill);
            bufflist.push_back(eBuff_WizDefense); bufflist.push_back(eBuff_AddCriticalDamage);
            bufflist.push_back(eBuff_CrywolfAltarOccufied);

            g_CharacterUnRegisterBuffList(o, bufflist);

            Vector(0.f, 0.f, 45.f, Angle);
            CreateJoint(MODEL_SPEARSKILL, Position, Position, Angle, 5, o, 170.0f);
            Position[2] -= 10.f;
            Vector(0.f, 0.f, 135.f, Angle);
            CreateJoint(MODEL_SPEARSKILL, Position, Position, Angle, 6, o, 170.0f);
            Position[2] -= 10.f;
            Vector(0.f, 0.f, 225.f, Angle);
            CreateJoint(MODEL_SPEARSKILL, Position, Position, Angle, 7, o, 170.0f);

            Vector(0.f, 0.f, 90.f, Angle);
            CreateJoint(MODEL_SPEARSKILL, Position, Position, Angle, 5, o, 170.0f);
            Position[2] -= 10.f;
            Vector(0.f, 0.f, 180.f, Angle);
            CreateJoint(MODEL_SPEARSKILL, Position, Position, Angle, 6, o, 170.0f);
            Position[2] -= 10.f;
            Vector(0.f, 0.f, 270.f, Angle);
            CreateJoint(MODEL_SPEARSKILL, Position, Position, Angle, 7, o, 170.0f);
        }

        PlayBuffer(SOUND_BMS_MAGIC_REMOVAL);
        break;
        case AT_SKILL_IMPROVE_AG:
        {
            vec3_t Angle = { -45.f, 0.f, 45.f };
            vec3_t Light = { 1.f, 1.f, 1.f };
            vec3_t Position;

            Position[0] = o->Position[0] + sinf(45 * 0.1f) * 80.f;
            Position[1] = o->Position[1] + cosf(45 * 0.1f) * 80.f;
            Position[2] = o->Position[2] + 300;
            CreateJoint(BITMAP_JOINT_HEALING, Position, Position, Angle, 10, o, 15.f);
            Angle[2] = 405.f;
            CreateJoint(BITMAP_JOINT_HEALING, Position, Position, Angle, 10, o, 15.f);

            Angle[2] = 225.f;
            Position[0] = o->Position[0] + cosf(225 * 0.1f) * 80.f;
            Position[1] = o->Position[1] + sinf(225 * 0.1f) * 80.f;
            Position[2] = o->Position[2] + 300;
            CreateJoint(BITMAP_JOINT_HEALING, Position, Position, Angle, 10, o, 15.f);
            Angle[2] = 450.f;
            CreateJoint(BITMAP_JOINT_HEALING, Position, Position, Angle, 10, o, 15.f);
        }
        PlayBuffer(SOUND_SWELLLIFE);
        break;
        case AT_SKILL_ADD_CRITICAL:
        case AT_SKILL_ADD_CRITICAL_STR1:
        case AT_SKILL_ADD_CRITICAL_STR2:
        case AT_SKILL_ADD_CRITICAL_STR3:
            Vector(0.f, 0.f, 0.f, p);
            Vector(1.f, 0.6f, 0.3f, Light);
            if (c->Weapon[0].Type != MODEL_ARROWS)
            {
                b->TransformPosition(o->BoneTransform[c->Weapon[0].LinkBone], p, Position, true);
                CreateEffect(MODEL_DARKLORD_SKILL, Position, o->Angle, Light, 0);
            }
            if (c->Weapon[1].Type != MODEL_BOLT && (c->Weapon[1].Type < MODEL_SHIELD || c->Weapon[1].Type >= MODEL_SHIELD + MAX_ITEM_INDEX))
            {
                b->TransformPosition(o->BoneTransform[c->Weapon[1].LinkBone], p, Position, true);
                CreateEffect(MODEL_DARKLORD_SKILL, Position, o->Angle, Light, 1);
            }
            PlayBuffer(SOUND_CRITICAL, o);
            break;
        case AT_SKILL_PARTY_TELEPORT:
            CreateEffect(MODEL_CIRCLE, o->Position, o->Angle, o->Light, 2, o);
            CreateEffect(MODEL_CIRCLE_LIGHT, o->Position, o->Angle, o->Light, 3);

            PlayBuffer(SOUND_PART_TELEPORT, o);
            break;
        case AT_SKILL_THUNDER_STRIKE:
            CalcAddPosition(o, 0.f, -90.f, -50.f, Position);
            if (o->CurrentAction == PLAYER_ATTACK_RIDE_ATTACK_FLASH)
            {
                Position[2] += 80.f;
            }
            else if (o->CurrentAction == PLAYER_FENRIR_ATTACK_DARKLORD_FLASH)
            {
                Position[2] += 40.f;
            }

            CreateEffect(BITMAP_FLARE_FORCE, Position, o->Angle, o->Light, 0, o);

            PlayBuffer(SOUND_ELEC_STRIKE, o);
            break;
        case AT_SKILL_RIDER:
            CreateEffect(BITMAP_SHOTGUN, o->Position, o->Angle, o->Light, 0, o, o->PKKey, CurrentSkill);

            PlayBuffer(SOUND_SKILL_SWORD3);
            break;
        case AT_SKILL_TWISTING_SLASH:
        case AT_SKILL_TWISTING_SLASH_STR:
        case AT_SKILL_TWISTING_SLASH_STR_MG:
        case AT_SKILL_TWISTING_SLASH_MASTERY:
            o->Weapon = c->Weapon[0].Type - MODEL_SWORD;
            o->WeaponLevel = (BYTE)c->Weapon[0].Level;
            CreateEffect(MODEL_SKILL_WHEEL1, o->Position, o->Angle, o->Light, 0, o, o->PKKey, FindHotKey((c->Skill)));

            if (SceneFlag != LOG_IN_SCENE)
                PlayBuffer(SOUND_SKILL_SWORD4);

            c->PostMoveProcess_Active(g_iLimitAttackTime);
            break;
        case AT_SKILL_HELL_FIRE:
        case AT_SKILL_HELL_FIRE_STR:
            CreateEffect(MODEL_CIRCLE, o->Position, o->Angle, o->Light, 0, o, o->PKKey, FindHotKey((c->Skill)));
            CreateEffect(MODEL_CIRCLE_LIGHT, o->Position, o->Angle, o->Light);

            if (gMapManager.InHellas() == true)
            {
                AddWaterWave((c->PositionX), (c->PositionY), 2, -1500);
            }

            PlayBuffer(SOUND_HELLFIRE);
            break;
        case AT_SKILL_NOVA:
            CreateEffect(MODEL_CIRCLE, o->Position, o->Angle, o->Light, 1, o);
            StopBuffer(SOUND_NUKE1, true);
            PlayBuffer(SOUND_NUKE2);
            break;
        case AT_SKILL_DECAY:
        case AT_SKILL_DECAY_STR:
            Position[0] = (float)(c->SkillX + 0.5f) * TERRAIN_SCALE;
            Position[1] = (float)(c->SkillY + 0.5f) * TERRAIN_SCALE;
            Position[2] = RequestTerrainHeight(Position[0], Position[1]);

            Vector(0.8f, 0.5f, 0.1f, Light);
            CreateEffect(MODEL_FIRE, Position, o->Angle, Light, 6, NULL, 0);
            CreateEffect(MODEL_FIRE, Position, o->Angle, Light, 6, NULL, 0);
            PlayBuffer(SOUND_DEATH_POISON1);

            if (c == Hero)
            {
                ++CharacterMachine->PacketSerial;
            }
            break;
        case AT_SKILL_ICE_STORM:
        {
            vec3_t TargetPosition, Pos;
            TargetPosition[0] = (float)(c->SkillX + 0.5f) * TERRAIN_SCALE;
            TargetPosition[1] = (float)(c->SkillY + 0.5f) * TERRAIN_SCALE;
            TargetPosition[2] = RequestTerrainHeight(TargetPosition[0], TargetPosition[1]);

            for (int i = 0; i < 10; ++i)
            {
                Pos[0] = TargetPosition[0];
                Pos[1] = TargetPosition[1];
                Pos[2] = TargetPosition[2] + (rand() % 50) * i;
                CreateEffect(MODEL_BLIZZARD, Pos, o->Angle, Light, 0, NULL, i + 1);
            }
            if (c == Hero)
            {
                ++CharacterMachine->PacketSerial;
            }

            PlayBuffer(SOUND_SUDDEN_ICE1);
        }
        break;

        case AT_SKILL_FLAME:
        case AT_SKILL_FLAME_STR:
        case AT_SKILL_FLAME_STR_MG:
            Position[0] = (float)(c->SkillX + 0.5f) * TERRAIN_SCALE;
            Position[1] = (float)(c->SkillY + 0.5f) * TERRAIN_SCALE;
            Position[2] = RequestTerrainHeight(Position[0], Position[1]);
            CreateEffect(BITMAP_FLAME, Position, o->Angle, o->Light, 0, o, o->PKKey, FindHotKey(AT_SKILL_FLAME));

            if (c == Hero)
            {
                ++CharacterMachine->PacketSerial;
            }
            if (SceneFlag != LOG_IN_SCENE)
                PlayBuffer(SOUND_FLAME);
            break;
        case AT_SKILL_STORM:
            CreateEffect(MODEL_STORM, o->Position, o->Angle, Light, 0, o, o->PKKey, FindHotKey(AT_SKILL_STORM));
            PlayBuffer(SOUND_STORM);
            break;

        case AT_SKILL_FIRE_SCREAM:
        case AT_SKILL_FIRE_SCREAM_STR:
        {
            int SkillIndex = FindHotKey((c->Skill));
            OBJECT* pObj = o;
            vec3_t ap, P, dp;
            float BkO = pObj->Angle[2];

            VectorCopy(pObj->Position, ap);
            CreateEffect(MODEL_DARK_SCREAM, pObj->Position, pObj->Angle, pObj->Light, 0, pObj, pObj->PKKey, SkillIndex);
            CreateEffect(MODEL_DARK_SCREAM_FIRE, pObj->Position, pObj->Angle, pObj->Light, 0, pObj, pObj->PKKey, SkillIndex);

            Vector(80.f, 0.f, 0.f, P);

            pObj->Angle[2] += 10.f;

            AngleMatrix(pObj->Angle, pObj->Matrix);
            VectorRotate(P, pObj->Matrix, dp);
            VectorAdd(dp, pObj->Position, pObj->Position);
            CreateEffect(MODEL_DARK_SCREAM, pObj->Position, pObj->Angle, pObj->Light, 0, pObj, pObj->PKKey, SkillIndex);
            CreateEffect(MODEL_DARK_SCREAM_FIRE, pObj->Position, pObj->Angle, pObj->Light, 0, pObj, pObj->PKKey, SkillIndex);

            VectorCopy(ap, pObj->Position);
            VectorCopy(pObj->Position, ap);
            Vector(-80.f, 0.f, 0.f, P);
            pObj->Angle[2] -= 20.f;

            AngleMatrix(pObj->Angle, pObj->Matrix);
            VectorRotate(P, pObj->Matrix, dp);
            VectorAdd(dp, pObj->Position, pObj->Position);
            CreateEffect(MODEL_DARK_SCREAM, pObj->Position, pObj->Angle, pObj->Light, 0, pObj, pObj->PKKey, SkillIndex);
            CreateEffect(MODEL_DARK_SCREAM_FIRE, pObj->Position, pObj->Angle, pObj->Light, 0, pObj, pObj->PKKey, SkillIndex);
            VectorCopy(ap, pObj->Position);
            pObj->Angle[2] = BkO;

            if ((c->Helper.Type >= MODEL_HORN_OF_UNIRIA && c->Helper.Type <= MODEL_DARK_HORSE_ITEM) && !c->SafeZone)
            {
                SetAction(o, PLAYER_ATTACK_RIDE_STRIKE);
            }
            else if (c->Helper.Type == MODEL_HORN_OF_FENRIR && !c->SafeZone)
            {
                SetAction(o, PLAYER_FENRIR_ATTACK_DARKLORD_STRIKE);
            }
            else
            {
                SetAction(o, PLAYER_ATTACK_STRIKE);
            }

            PlayBuffer(SOUND_FIRE_SCREAM);
        }
        break;
        case AT_SKILL_FLASH:
            CalcAddPosition(o, -20.f, -90.f, 100.f, Position);
            CreateEffect(BITMAP_BOSS_LASER, Position, o->Angle, Light, 0, o, o->PKKey, FindHotKey(AT_SKILL_FLASH));
            PlayBuffer(SOUND_FLASH);
            break;

        case AT_SKILL_BLAST:
        case AT_SKILL_BLAST_STR:
        case AT_SKILL_BLAST_STR_MG:
            Position[0] = (float)(c->SkillX + 0.5f) * TERRAIN_SCALE;
            Position[1] = (float)(c->SkillY + 0.5f) * TERRAIN_SCALE;
            Position[2] = RequestTerrainHeight(Position[0], Position[1]);
            {
                int SkillIndex = FindHotKey((c->Skill));
                CreateEffect(MODEL_SKILL_BLAST, Position, o->Angle, o->Light, 0, o, o->PKKey, SkillIndex);
                CreateEffect(MODEL_SKILL_BLAST, Position, o->Angle, o->Light, 0, o, o->PKKey, SkillIndex);
            }

            if (c == Hero)
            {
                ++CharacterMachine->PacketSerial;
            }
            break;
        case AT_SKILL_INFERNO:
        case AT_SKILL_INFERNO_STR:
        case AT_SKILL_INFERNO_STR_MG:
            CreateInferno(o->Position);
            CreateEffect(MODEL_SKILL_INFERNO, o->Position, o->Angle, o->Light, 0, o, o->PKKey, FindHotKey(c->Skill));

            if (c == Hero)
            {
                ++CharacterMachine->PacketSerial;
            }
            break;
        case AT_SKILL_EVIL_SPIRIT:
        case AT_SKILL_EVIL_SPIRIT_STR:
        case AT_SKILL_EVIL_SPIRIT_STR_MG:
            vec3_t Position;
            VectorCopy(o->Position, Position);
            Position[2] += 100.f;

            for (int i = 0; i < 4; i++)
            {
                vec3_t Angle;
                Vector(0.f, 0.f, i * 90.f, Angle);

                int SkillIndex = FindHotKey((c->Skill));
                CreateJoint(BITMAP_JOINT_SPIRIT, Position, o->Position, Angle, 0, o, 80.f, o->PKKey, SkillIndex, o->m_bySkillSerialNum);
                CreateJoint(BITMAP_JOINT_SPIRIT, Position, o->Position, Angle, 0, o, 20.f);
            }
            if (c == Hero)
            {
                ++CharacterMachine->PacketSerial;
            }
            PlayBuffer(SOUND_EVIL);
            break;
        case AT_SKILL_PLASMA_STORM_FENRIR:
        {
            PlayBuffer(SOUND_FENRIR_SKILL);
            CHARACTER* p_temp_c;
            OBJECT* p_o[MAX_FENRIR_SKILL_MONSTER_NUM];
            int iMonsterNum = 0;

            for (int i = 0; i < MAX_CHARACTERS_CLIENT; i++)
            {
                p_temp_c = &CharactersClient[i];

                if (p_temp_c->Object.Live == TRUE && p_temp_c->Object.Kind == KIND_MONSTER && p_temp_c->Object.CurrentAction != MONSTER01_DIE)
                {
                    float dx = c->Object.Position[0] - p_temp_c->Object.Position[0];
                    float dy = c->Object.Position[1] - p_temp_c->Object.Position[1];
                    float fDistance = sqrtf(dx * dx + dy * dy) / TERRAIN_SCALE;
                    if (fDistance <= gSkillManager.GetSkillDistance(AT_SKILL_PLASMA_STORM_FENRIR))
                    {
                        p_o[iMonsterNum] = &p_temp_c->Object;
                        iMonsterNum++;
                    }
                }
                else if (p_temp_c->Object.Live == TRUE && p_temp_c->Object.Kind == KIND_PLAYER && p_temp_c->Object.CurrentAction != PLAYER_DIE1)
                {
                    if (CheckAttack_Fenrir(p_temp_c) == true && CInput::Instance().IsKeyDown(VK_LCONTROL))
                    {
                        float dx = c->Object.Position[0] - p_temp_c->Object.Position[0];
                        float dy = c->Object.Position[1] - p_temp_c->Object.Position[1];
                        float fDistance = sqrtf(dx * dx + dy * dy) / TERRAIN_SCALE;
                        if (fDistance <= gSkillManager.GetSkillDistance(AT_SKILL_PLASMA_STORM_FENRIR))
                        {
                            p_o[iMonsterNum] = &p_temp_c->Object;
                            iMonsterNum++;
                        }
                    }
                }

                if (iMonsterNum >= 10)
                    break;
            }

            vec3_t vAngle;
            int iAngle = rand() % 360;

            if (c->m_iFenrirSkillTarget != -1 && c->m_iFenrirSkillTarget < MAX_CHARACTERS_CLIENT)
            {
                CHARACTER* p_tc = &CharactersClient[c->m_iFenrirSkillTarget];
                OBJECT* p_to = &p_tc->Object;
                auto fenrirType = GetFenrirType(c);

                for (int j = 0; j < 2; j++)
                {
                    CalcAddPosition(o, 0.f, -140.f, 130.f, Position);
                    Vector((float)(rand() % 360), 0.0f, (float)(rand() % 360), vAngle);

                    CreateJoint(MODEL_FENRIR_SKILL_THUNDER, Position, p_to->Position, vAngle, 0 + fenrirType, p_to, 100.f);
                    CreateJoint(MODEL_FENRIR_SKILL_THUNDER, Position, p_to->Position, vAngle, 3 + fenrirType, p_to, 80.f);
                }

                for (int i = 0; i < iMonsterNum; i++)
                {
                    for (int j = 0; j < 2; j++)
                    {
                        CalcAddPosition(o, 0.f, -140.f, 130.f, Position);
                        Vector((float)(rand() % 360), 0.0f, (float)(rand() % 360), vAngle);

                        CreateJoint(MODEL_FENRIR_SKILL_THUNDER, Position, p_o[i]->Position, vAngle, 0 + fenrirType, p_o[i], 100.f);
                        CreateJoint(MODEL_FENRIR_SKILL_THUNDER, Position, p_o[i]->Position, vAngle, 4 + fenrirType, p_o[i], 80.f);
                    }
                }

                for (int k = 0; k < 6; k++)
                {
                    CalcAddPosition(o, 0.f, 10.f + (rand() % 40 - 20), 130.f, Position);
                    Vector((float)(rand() % 360), 0.0f, (float)(rand() % 360), vAngle);
                    CreateJoint(BITMAP_FLARE_FORCE, Position, Position, vAngle, 11 + fenrirType, NULL, 60.f);
                }
            }
            break;
        }
        case AT_SKILL_RUSH:
            CreateEffect(MODEL_SWORD_FORCE, o->Position, o->Angle, Light, 0, o);
            PlayBuffer(SOUND_BCS_RUSH);
            break;
        case AT_SKILL_OCCUPY:
        {
            CreateEffect(MODEL_SWORD_FORCE, o->Position, o->Angle, Light, 2, o);
            PlayBuffer(SOUND_BCS_RUSH);
        }
        break;
        case AT_SKILL_SPIRAL_SLASH:
            break;

        case AT_SKILL_BRAND_OF_SKILL:
            Vector(0.f, 0.f, 0.f, p);
            Vector(1.f, 1.f, 1.f, Light);
            if (c->Weapon[0].Type != MODEL_ARROWS)
            {
                b->TransformPosition(o->BoneTransform[c->Weapon[0].LinkBone], p, Position, true);
                CreateEffect(MODEL_DARKLORD_SKILL, Position, o->Angle, Light, 0);
            }
            if (c->Weapon[1].Type != MODEL_BOLT && (c->Weapon[1].Type < MODEL_SHIELD || c->Weapon[1].Type >= MODEL_SHIELD + MAX_ITEM_INDEX))
            {
                b->TransformPosition(o->BoneTransform[c->Weapon[1].LinkBone], p, Position, true);
                CreateEffect(MODEL_DARKLORD_SKILL, Position, o->Angle, Light, 1);
            }
            CreateEffect(MODEL_MANA_RUNE, o->Position, o->Angle, o->Light);

            PlayBuffer(SOUND_BCS_BRAND_OF_SKILL);
            break;
        case AT_SKILL_ALICE_BERSERKER:
        case AT_SKILL_ALICE_BERSERKER_STR:
            Vector(1.0f, 0.1f, 0.2f, Light);
            CreateEffect(BITMAP_MAGIC + 1, o->Position, o->Angle, Light, 11, o);
            CreateEffect(MODEL_ALICE_BUFFSKILL_EFFECT, o->Position, o->Angle, Light, 0, o);
            CreateEffect(MODEL_ALICE_BUFFSKILL_EFFECT2, o->Position, o->Angle, Light, 0, o);
            PlayBuffer(SOUND_SKILL_BERSERKER);
            break;
        case AT_SKILL_ALICE_WEAKNESS:
            Vector(2.0f, 0.1f, 0.1f, Light);
            CreateEffect(BITMAP_MAGIC_ZIN, o->Position, o->Angle, Light, 1, NULL, -1, 0, 0, 0, 7.0f);
            Vector(2.0f, 0.4f, 0.3f, Light);
            CreateEffect(BITMAP_MAGIC_ZIN, o->Position, o->Angle, Light, 0, NULL, -1, 0, 0, 0, 2.0f);
            CreateEffect(BITMAP_MAGIC_ZIN, o->Position, o->Angle, Light, 2, NULL, -1, 0, 0, 0, 1.0f);
            CreateEffect(BITMAP_MAGIC_ZIN, o->Position, o->Angle, Light, 2, NULL, -1, 0, 0, 0, 0.2f);
            CreateEffect(BITMAP_MAGIC_ZIN, o->Position, o->Angle, Light, 2, NULL, -1, 0, 0, 0, 0.1f);
            CreateEffect(MODEL_SUMMONER_CASTING_EFFECT2, o->Position, o->Angle, Light, 1, NULL, -1, 0, 0, 0, 0.6f);
            CreateEffect(MODEL_SUMMONER_CASTING_EFFECT22, o->Position, o->Angle, Light, 1, NULL, -1, 0, 0, 0, 0.6f);
            CreateEffect(MODEL_SUMMONER_CASTING_EFFECT222, o->Position, o->Angle, Light, 1, NULL, -1, 0, 0, 0, 0.6f);
            Vector(1.4f, 0.2f, 0.2f, Light);
            CreateEffect(BITMAP_SHINY + 6, o->Position, o->Angle, Light, 0, NULL, -1, 0, 0, 0, 0.5f);
            CreateEffect(BITMAP_PIN_LIGHT, o->Position, o->Angle, Light, 0, NULL, -1, 0, 0, 0, 1.f);
            PlayBuffer(SOUND_SKILL_WEAKNESS);
            break;
        case AT_SKILL_ALICE_ENERVATION:
            Vector(0.25f, 1.0f, 0.7f, Light);
            CreateEffect(BITMAP_MAGIC_ZIN, o->Position, o->Angle, Light, 1, NULL, -1, 0, 0, 0, 7.0f);
            CreateEffect(BITMAP_MAGIC_ZIN, o->Position, o->Angle, Light, 0, NULL, -1, 0, 0, 0, 2.0f);
            CreateEffect(BITMAP_MAGIC_ZIN, o->Position, o->Angle, Light, 2, NULL, -1, 0, 0, 0, 1.0f);
            CreateEffect(BITMAP_MAGIC_ZIN, o->Position, o->Angle, Light, 2, NULL, -1, 0, 0, 0, 0.2f);
            CreateEffect(BITMAP_MAGIC_ZIN, o->Position, o->Angle, Light, 2, NULL, -1, 0, 0, 0, 0.1f);
            CreateEffect(MODEL_SUMMONER_CASTING_EFFECT2, o->Position, o->Angle, Light, 1, NULL, -1, 0, 0, 0, 0.6f);
            CreateEffect(MODEL_SUMMONER_CASTING_EFFECT22, o->Position, o->Angle, Light, 1, NULL, -1, 0, 0, 0, 0.6f);
            CreateEffect(MODEL_SUMMONER_CASTING_EFFECT222, o->Position, o->Angle, Light, 1, NULL, -1, 0, 0, 0, 0.6f);
            CreateEffect(BITMAP_SHINY + 6, o->Position, o->Angle, Light, 0, NULL, -1, 0, 0, 0, 0.5f);
            CreateEffect(BITMAP_PIN_LIGHT, o->Position, o->Angle, Light, 0, NULL, -1, 0, 0, 0, 1.f);
            PlayBuffer(SOUND_SKILL_ENERVATION);
            break;
        case AT_SKILL_FLAME_STRIKE:
        {
            //DeleteEffect(MODEL_EFFECT_FLAME_STRIKE, o, 0);
            CreateEffect(MODEL_EFFECT_FLAME_STRIKE, o->Position, o->Angle, o->Light, 0, o);
            PlayBuffer(SOUND_SKILL_FLAME_STRIKE);
        }
        break;
        case AT_SKILL_GIGANTIC_STORM:
        {
            vec34_t Matrix;
            vec3_t vAngle, vDirection, vPosition;
            float fAngle;
            for (int i = 0; i < 5; ++i)
            {
                Vector(0.f, 200.f, 0.f, vDirection);
                fAngle = o->Angle[2] + i * 72.f;
                Vector(0.f, 0.f, fAngle, vAngle);
                AngleMatrix(vAngle, Matrix);
                VectorRotate(vDirection, Matrix, vPosition);
                VectorAdd(vPosition, o->Position, vPosition);

                CreateEffect(BITMAP_JOINT_THUNDER, vPosition, o->Angle, o->Light);
            }
            PlayBuffer(SOUND_SKILL_GIGANTIC_STORM);
        }
        break;
        case AT_SKILL_LIGHTNING_SHOCK:
        case AT_SKILL_LIGHTNING_SHOCK_STR:
        {
            // 				CHARACTER *tc = &CharactersClient[c->TargetCharacter];
            // 				OBJECT *to = &tc->Object;
            //
            // 				CreateEffect(MODEL_LIGHTNING_SHOCK, to->Position, to->Angle, to->Light, 2, to);
            vec3_t vLight;
            Vector(1.0f, 1.0f, 1.0f, vLight);

            CreateEffect(MODEL_LIGHTNING_SHOCK, o->Position, o->Angle, vLight, 0, o);
            PlayBuffer(SOUND_SKILL_LIGHTNING_SHOCK);
        }
        break;
        case AT_SKILL_KILLING_BLOW:
        case AT_SKILL_KILLING_BLOW_STR:
        case AT_SKILL_KILLING_BLOW_MASTERY:
        {
            o->Angle[2] = CreateAngle2D(o->Position, c->TargetPosition);
            o->m_sTargetIndex = c->TargetCharacter;
        }
        break;
        case AT_SKILL_BEAST_UPPERCUT:
        case AT_SKILL_BEAST_UPPERCUT_STR:
        case AT_SKILL_BEAST_UPPERCUT_MASTERY:
        {
            o->m_sTargetIndex = c->TargetCharacter;
        }
        break;
        }

        if (c->TargetCharacter == -1)
        {
            BYTE Skill = 0;
            if (c->Skill == AT_SKILL_TRIPLE_SHOT
                || c->Skill == AT_SKILL_TRIPLE_SHOT_STR
                || c->Skill == AT_SKILL_TRIPLE_SHOT_MASTERY)
                Skill = 1;
            if ((o->Type == MODEL_PLAYER &&
                (o->CurrentAction == PLAYER_ATTACK_BOW || o->CurrentAction == PLAYER_ATTACK_CROSSBOW ||
                    o->CurrentAction == PLAYER_ATTACK_FLY_BOW || o->CurrentAction == PLAYER_ATTACK_FLY_CROSSBOW ||
                    o->CurrentAction == PLAYER_FENRIR_ATTACK_BOW || o->CurrentAction == PLAYER_FENRIR_ATTACK_CROSSBOW ||
                    o->CurrentAction == PLAYER_ATTACK_RIDE_BOW || o->CurrentAction == PLAYER_ATTACK_RIDE_CROSSBOW)) ||
                o->Type != MODEL_PLAYER && o->Kind == KIND_PLAYER
                )
            {
                if (AT_SKILL_MULTI_SHOT != (c->Skill))
                    CreateArrows(c, o, NULL, FindHotKey((c->Skill)), Skill, (c->Skill));
            }

            if (o->Type == MODEL_HUNTER || o->Type == MODEL_VALKYRIE || o->Type == MODEL_SOLDIER
                || o->Type == MODEL_ORC_ARCHER)
            {
                CreateArrows(c, o, NULL, 0, 0);
            }
        }
        else
        {
            CHARACTER* tc = &CharactersClient[c->TargetCharacter];
            OBJECT* to = &tc->Object;
            if (o->Type == MODEL_PLAYER &&
                (o->CurrentAction == PLAYER_ATTACK_BOW || o->CurrentAction == PLAYER_ATTACK_CROSSBOW ||
                    o->CurrentAction == PLAYER_ATTACK_FLY_BOW || o->CurrentAction == PLAYER_ATTACK_FLY_CROSSBOW ||
                    o->CurrentAction == PLAYER_ATTACK_RIDE_BOW || o->CurrentAction == PLAYER_ATTACK_RIDE_CROSSBOW
                    || o->CurrentAction == PLAYER_FENRIR_ATTACK_BOW || o->CurrentAction == PLAYER_FENRIR_ATTACK_CROSSBOW	//^ 펜릴 스킬 관련(요정 화살 나가게 하는 것)
                    ))
            {
                if (AT_SKILL_MULTI_SHOT != (c->Skill))
                    CreateArrows(c, o, to, FindHotKey((c->Skill)), 0, (c->Skill));
            }

            if (o->Type == MODEL_HUNTER || o->Type == MODEL_VALKYRIE || o->Type == MODEL_SOLDIER)
            {
                CreateArrows(c, o, to, 0, 0);
            }

            if (tc->Hit >= 1)
            {
                if (to->Type != MODEL_GHOST)
                {
                    for (int i = 0; i < 10; i++)
                    {
                        Vector(to->Position[0] + (float)(rand() % 64 - 32), to->Position[1] + (float)(rand() % 64 - 32), to->Position[2] + (float)(rand() % 64 + 90), Position);
                        CreateParticle(BITMAP_BLOOD + 1, Position, o->Angle, Light);
                    }
                }

                if (to->Type == MODEL_STATUE_OF_SAINT)
                {
                    for (int i = 0; i < 5; i++)
                    {
                        if (rand_fps_check(2))
                        {
                            Position[0] = to->Position[0];
                            Position[1] = to->Position[1];
                            Position[2] = to->Position[2] + 50 + rand() % 30;

                            CreateEffect(MODEL_STONE_COFFIN + 1, Position, o->Angle, o->Light);
                        }
                    }
                    PlayBuffer(SOUND_HIT_CRISTAL);
                }
                battleCastle::RenderMonsterHitEffect(to);
            }
            if (o->CurrentAction >= PLAYER_ATTACK_SKILL_SWORD1 && o->CurrentAction <= PLAYER_ATTACK_SKILL_SWORD5)
            {
                CreateSpark(0, tc, to->Position, o->Angle);
            }
            vec3_t Angle;
            VectorCopy(o->Angle, Angle);
            Angle[2] = CreateAngle2D(o->Position, to->Position);
            switch ((c->Skill))
            {
            case AT_SKILL_TRIPLE_SHOT:
            case AT_SKILL_TRIPLE_SHOT_STR:
            case AT_SKILL_TRIPLE_SHOT_MASTERY:
                CreateArrows(c, o, NULL, FindHotKey((c->Skill)), 1);
            case AT_SKILL_PENETRATION:
            case AT_SKILL_PENETRATION_STR:
                CreateArrows(c, o, NULL, FindHotKey((c->Skill)), 0, (c->Skill));
                break;
            case AT_SKILL_ICE_ARROW:
            case AT_SKILL_ICE_ARROW_STR:
                CreateArrows(c, o, NULL, FindHotKey((c->Skill)), 0, (c->Skill));
                break;
            case AT_SKILL_DEEPIMPACT:
                CreateArrows(c, o, to, FindHotKey((c->Skill)), 0, (c->Skill));
                PlayBuffer(SOUND_BCS_DEEP_IMPACT);
                break;
            case AT_SKILL_HEALING:
            case AT_SKILL_HEALING_STR:
                CreateEffect(BITMAP_MAGIC + 1, to->Position, to->Angle, to->Light, 1, to);
                break;
            case AT_SKILL_DEFENSE:
            case AT_SKILL_DEFENSE_STR:
            case AT_SKILL_DEFENSE_MASTERY:
                if (c->SkillSuccess)
                {
                    if (g_isCharacterBuff(o, eBuff_Cloaking))
                    {
                        break;
                    }

                    CreateEffect(BITMAP_MAGIC + 1, to->Position, to->Angle, to->Light, 2, to);

                    if (!g_isCharacterBuff(to, eBuff_Defense))
                    {
                        g_CharacterRegisterBuff(to, eBuff_Defense);

                        for (int j = 0; j < 5; ++j)
                        {
                            CreateJoint(MODEL_SPEARSKILL, to->Position, to->Position, to->Angle, 4, to, 20.0f, -1, 0, 0, c->TargetCharacter);
                        }
                    }
                    else if (!SearchJoint(MODEL_SPEARSKILL, to, 4)
                        && !SearchJoint(MODEL_SPEARSKILL, to, 9))
                    {
                        for (int j = 0; j < 5; ++j)
                        {
                            CreateJoint(MODEL_SPEARSKILL, to->Position, to->Position, to->Angle, 4, to, 20.0f, -1, 0, 0, c->TargetCharacter);
                        }
                    }
                }
                break;
            case AT_SKILL_ATTACK:
            case AT_SKILL_ATTACK_STR:
            case AT_SKILL_ATTACK_MASTERY:
                if (g_isCharacterBuff(o, eBuff_Cloaking)) break;
                CreateEffect(BITMAP_MAGIC + 1, to->Position, to->Angle, to->Light, 3, to);
                if (c->SkillSuccess)
                {
                    g_CharacterRegisterBuff(to, eBuff_Attack);
                }
                break;
            case AT_SKILL_ICE:
            case AT_SKILL_ICE_STR:
            case AT_SKILL_ICE_STR_MG:
                CreateEffect(MODEL_ICE, to->Position, o->Angle, Light);

                for (int i = 0; i < 5; i++)
                    CreateEffect(MODEL_ICE_SMALL, to->Position, o->Angle, o->Light);

                if (c->SkillSuccess)
                {
                    if (!g_isCharacterBuff(to, eDeBuff_Freeze))
                    {
                        g_CharacterRegisterBuff(to, eDeBuff_Freeze);
                    }
                }
                PlayBuffer(SOUND_ICE);
                break;
            case AT_SKILL_SOUL_BARRIER:
            case AT_SKILL_SOUL_BARRIER_STR:
            case AT_SKILL_SOUL_BARRIER_PROFICIENCY:
                if (o->Type == MODEL_DARK_PHEONIX_SHIELD)
                {
                    g_CharacterRegisterBuff(o, eBuff_WizDefense);
                }
                else
                {
                    if (g_isCharacterBuff(to, eBuff_Cloaking)) break;
                    g_CharacterRegisterBuff(to, eBuff_WizDefense);

                    PlayBuffer(SOUND_SOULBARRIER);
                    DeleteJoint(MODEL_SPEARSKILL, to, 0);
                    for (int j = 0; j < 5; ++j)
                    {
                        CreateJoint(MODEL_SPEARSKILL, to->Position, to->Position, to->Angle, 0, to, 20.0f);
                    }
                }
                break;
            case AT_SKILL_POISON:
            case AT_SKILL_POISON_STR:
                if (o->Type == MODEL_PLAYER)
                    CreateEffect(MODEL_POISON, to->Position, o->Angle, o->Light);
                Vector(0.4f, 0.6f, 1.f, Light);

                for (int i = 0; i < 10; i++)
                    CreateParticle(BITMAP_SMOKE, to->Position, o->Angle, Light, 1);

                if (c->SkillSuccess)
                {
                    g_CharacterRegisterBuff(to, eDeBuff_Poison);
                }
                PlayBuffer(SOUND_HEART);
                break;
            case AT_SKILL_METEO:
                CreateEffect(MODEL_FIRE, to->Position, to->Angle, o->Light);
                PlayBuffer(SOUND_METEORITE01);
                break;
            case AT_SKILL_JAVELIN:
                CreateEffect(MODEL_SKILL_JAVELIN, o->Position, o->Angle, o->Light, 0, to);
                CreateEffect(MODEL_SKILL_JAVELIN, o->Position, o->Angle, o->Light, 1, to);
                CreateEffect(MODEL_SKILL_JAVELIN, o->Position, o->Angle, o->Light, 2, to);

                PlayBuffer(SOUND_BCS_JAVELIN);
                break;
            case AT_SKILL_DEATH_CANNON:
                Vector(0.f, 0.f, o->Angle[2], Angle);
                VectorCopy(o->Position, Position);

                Position[2] += 130.f;
                CreateJoint(BITMAP_JOINT_FORCE, Position, Position, Angle, 4, NULL, 40.f);

                PlayBuffer(SOUND_BCS_DEATH_CANON);
                break;
            case AT_SKILL_SPACE_SPLIT:
                CreateEffect(MODEL_PIER_PART, o->Position, o->Angle, o->Light, 2, to);
                PlayBuffer(SOUND_BCS_SPACE_SPLIT);
                break;
            case AT_SKILL_FIREBALL:
                CreateEffect(MODEL_FIRE, o->Position, Angle, o->Light, 1, to);
                PlayBuffer(SOUND_METEORITE01);
                break;
            case AT_SKILL_FLAME:
            case AT_SKILL_FLAME_STR:
            case AT_SKILL_FLAME_STR_MG:
                Position[0] = to->Position[0];
                Position[1] = to->Position[1];
                Position[2] = RequestTerrainHeight(Position[0], Position[1]);
                CreateEffect(BITMAP_FLAME, Position, o->Angle, o->Light, 5, o, o->PKKey, FindHotKey(AT_SKILL_FLAME));
                PlayBuffer(SOUND_FLAME);
                break;
            case AT_SKILL_POWERWAVE:
                if (o->Type == MODEL_ICE_QUEEN)
                {
                    Angle[2] += 10.f;
                    CreateEffect(MODEL_MAGIC2, o->Position, Angle, o->Light);
                    Angle[2] -= 20.f;
                    CreateEffect(MODEL_MAGIC2, o->Position, Angle, o->Light);
                    Angle[2] += 10.f;
                }
                CreateEffect(MODEL_MAGIC2, o->Position, Angle, o->Light);
                PlayBuffer(SOUND_MAGIC);
                break;
            case AT_SKILL_FORCE:
            case AT_SKILL_FORCE_WAVE:
            case AT_SKILL_FORCE_WAVE_STR:
                CreateEffect(MODEL_WAVES, o->Position, o->Angle, o->Light, 1);
                CreateEffect(MODEL_WAVES, o->Position, o->Angle, o->Light, 1);
                CreateEffect(MODEL_PIERCING2, o->Position, o->Angle, o->Light);
                PlayBuffer(SOUND_ATTACK_SPEAR);
                break;
            case AT_SKILL_FIREBURST:
            case AT_SKILL_FIREBURST_STR:
            case AT_SKILL_FIREBURST_MASTERY:
            {
                vec3_t Angle = { 0.f, 0.f, o->Angle[2] };
                vec3_t Pos = { 0.f, 0.f, (to->BoundingBoxMax[2] / 1.f) };

                Vector(40.f, 0.f, 10.f, p);
                b->TransformPosition(o->BoneTransform[0], p, Position, true);
                Angle[2] = o->Angle[2] + 90;
                CreateEffect(MODEL_PIER_PART, Position, Angle, Pos, 0, to);
                Pos[2] -= to->BoundingBoxMax[2] / 2;
                Angle[2] = o->Angle[2];
                CreateEffect(MODEL_PIER_PART, Position, Angle, Pos, 0, to);
                Angle[2] = o->Angle[2] - 90;
                CreateEffect(MODEL_PIER_PART, Position, Angle, Pos, 0, to);

                Vector(1.f, 0.6f, 0.3f, Light);
                CreateEffect(MODEL_DARKLORD_SKILL, Position, o->Angle, Light, 0);
                CreateEffect(MODEL_DARKLORD_SKILL, Position, o->Angle, Light, 1);
            }
            break;

            case AT_SKILL_ENERGYBALL:
                switch (c->MonsterIndex)
                {
                case MONSTER_DEVIL:
                case MONSTER_VEPAR:
                case MONSTER_BEAM_KNIGHT:
                case MONSTER_CURSED_KING:
                case MONSTER_ALQUAMOS:
                case MONSTER_QUEEN_RAINER:
                case MONSTER_DRAKAN:
                case MONSTER_GREAT_DRAKAN:
                case MONSTER_DARK_PHOENIX:
                case MONSTER_MAGIC_SKELETON_1:
                case MONSTER_MAGIC_SKELETON_2:
                case MONSTER_MAGIC_SKELETON_3:
                case MONSTER_MAGIC_SKELETON_4:
                case MONSTER_MAGIC_SKELETON_5:
                case MONSTER_MAGIC_SKELETON_6:
                case MONSTER_MAGIC_SKELETON_7:
                case MONSTER_GIANT_OGRE_1:
                case MONSTER_GIANT_OGRE_2:
                case MONSTER_GIANT_OGRE_3:
                case MONSTER_GIANT_OGRE_4:
                case MONSTER_GIANT_OGRE_5:
                case MONSTER_GIANT_OGRE_6:
                case MONSTER_GIANT_OGRE_7:
                case MONSTER_CHAOS_CASTLE_2:
                case MONSTER_CHAOS_CASTLE_4:
                case MONSTER_CHAOS_CASTLE_6:
                case MONSTER_CHAOS_CASTLE_8:
                case MONSTER_CHAOS_CASTLE_10:
                case MONSTER_CHAOS_CASTLE_12:
                case MONSTER_CHAOS_CASTLE_14:
                case MONSTER_GIGAS_GOLEM:
                case MONSTER_POISON_GOLEM:
                    break;
                default:
                    if (o->Type == MODEL_YETI)
                    {
                        CreateEffect(MODEL_SNOW1, o->Position, Angle, o->Light, 0, to);
                    }
                    else if (o->Type == MODEL_GRIZZLY)
                    {
                        CreateEffect(MODEL_WOOSISTONE, o->Position, Angle, o->Light, 0, to);
                    }
                    else if (o->Type == MODEL_SAPITRES)
                    {
                        vec3_t vLight;
                        Vector(1.0f, 1.0f, 1.0f, vLight);
                        CreateEffect(MODEL_EFFECT_SAPITRES_ATTACK, o->Position, o->Angle, vLight, 0, to);
                    }
                    else
                    {
                        CreateEffect(BITMAP_ENERGY, o->Position, Angle, o->Light, 0, to);
                        PlayBuffer(SOUND_MAGIC);
                    }
                    break;
                }
                break;

            case AT_SKILL_ALICE_LIGHTNINGORB:
            {
                vec3_t vLight;
                Vector(1.0f, 1.0f, 1.0f, vLight);

                CreateEffect(MODEL_LIGHTNING_ORB, o->Position, o->Angle, vLight, 0, to);

                PlayBuffer(SOUND_SUMMON_SKILL_LIGHTORB);
            }
            break;

            case AT_SKILL_ALICE_BLIND:
            case AT_SKILL_ALICE_SLEEP:
            case AT_SKILL_ALICE_SLEEP_STR:
            case AT_SKILL_ALICE_THORNS:
            {
                int iSkillIndex = (c->Skill);
                vec3_t vLight;

                if (iSkillIndex == AT_SKILL_ALICE_SLEEP || iSkillIndex == AT_SKILL_ALICE_SLEEP_STR)
                {
                    Vector(0.7f, 0.3f, 0.8f, vLight);
                }
                else if (iSkillIndex == AT_SKILL_ALICE_BLIND)
                {
                    Vector(1.0f, 1.0f, 1.0f, vLight);
                }
                else if (iSkillIndex == AT_SKILL_ALICE_THORNS)
                {
                    Vector(0.8f, 0.5f, 0.2f, vLight);
                }
                if (iSkillIndex == AT_SKILL_ALICE_SLEEP || iSkillIndex == AT_SKILL_ALICE_THORNS || iSkillIndex == AT_SKILL_ALICE_SLEEP_STR)
                {
                    CreateEffect(BITMAP_MAGIC + 1, o->Position, o->Angle, vLight, 11, o);
                }
                else if (iSkillIndex == AT_SKILL_ALICE_BLIND)
                {
                    CreateEffect(BITMAP_MAGIC + 1, o->Position, o->Angle, vLight, 12, o);
                }

                if (iSkillIndex == AT_SKILL_ALICE_SLEEP || iSkillIndex == AT_SKILL_ALICE_SLEEP_STR)
                {
                    Vector(0.8f, 0.3f, 0.9f, vLight);
                    CreateEffect(MODEL_ALICE_BUFFSKILL_EFFECT, to->Position, to->Angle, vLight, 0, to);
                    CreateEffect(MODEL_ALICE_BUFFSKILL_EFFECT2, to->Position, to->Angle, vLight, 0, to);
                }
                else if (iSkillIndex == AT_SKILL_ALICE_BLIND)
                {
                    Vector(1.0f, 1.0f, 1.0f, vLight);
                    CreateEffect(MODEL_ALICE_BUFFSKILL_EFFECT, to->Position, to->Angle, vLight, 1, to);
                    CreateEffect(MODEL_ALICE_BUFFSKILL_EFFECT2, to->Position, to->Angle, vLight, 1, to);
                }
                else if (iSkillIndex == AT_SKILL_ALICE_THORNS)
                {
                    Vector(0.8f, 0.5f, 0.2f, vLight);
                    CreateEffect(MODEL_ALICE_BUFFSKILL_EFFECT, to->Position, to->Angle, vLight, 2, to);
                    CreateEffect(MODEL_ALICE_BUFFSKILL_EFFECT2, to->Position, to->Angle, vLight, 2, to);
                }
            }
            break;
            case AT_SKILL_ALICE_CHAINLIGHTNING:
            case AT_SKILL_ALICE_CHAINLIGHTNING_STR:
            {
                PlayBuffer(SOUND_SKILL_CHAIN_LIGHTNING);
            }
            break;
            case AT_SKILL_ALICE_DRAINLIFE:
            case AT_SKILL_ALICE_DRAINLIFE_STR:
            {
                CHARACTER* pTargetChar = &CharactersClient[c->TargetCharacter];
                OBJECT* pSourceObj = o;
                pSourceObj->Owner = &(pTargetChar->Object);

                CreateEffect(MODEL_ALICE_DRAIN_LIFE, pSourceObj->Position, pSourceObj->Angle, pSourceObj->Light, 0, pSourceObj);
                PlayBuffer(SOUND_SKILL_DRAIN_LIFE);
            }
            break;
            }

            VectorCopy(to->Position, Position);
            Position[2] += 120.f;

            int Hand = 0;
            if (o->CurrentAction == PLAYER_ATTACK_SWORD_LEFT1 || o->CurrentAction == PLAYER_ATTACK_SWORD_LEFT2)
                Hand = 1;

            if (tc == Hero)
            {
                Vector(1.f, 0.f, 0.f, Light);
            }
            else
            {
                Vector(1.f, 0.6f, 0.f, Light);
            }

            switch (c->AttackFlag)
            {
            case ATTACK_DIE:
                CreateJoint(BITMAP_JOINT_ENERGY, to->Position, to->Position, o->Angle, 0, o, 20.f);
                CreateJoint(BITMAP_JOINT_ENERGY, to->Position, to->Position, o->Angle, 1, o, 20.f);
                break;
            }

            switch ((c->Skill))
            {
            case AT_SKILL_HEALING:
            case AT_SKILL_HEALING_STR:
            case AT_SKILL_ATTACK:
            case AT_SKILL_ATTACK_STR:
            case AT_SKILL_ATTACK_MASTERY:
            case AT_SKILL_DEFENSE:
            case AT_SKILL_DEFENSE_STR:
            case AT_SKILL_DEFENSE_MASTERY:
            case AT_SKILL_SUMMON:
            case AT_SKILL_SUMMON + 1:
            case AT_SKILL_SUMMON + 2:
            case AT_SKILL_SUMMON + 3:
            case AT_SKILL_SUMMON + 4:
            case AT_SKILL_SUMMON + 5:
            case AT_SKILL_SUMMON + 6:
            case AT_SKILL_SUMMON + 7:
            case AT_SKILL_SOUL_BARRIER:
            case AT_SKILL_SOUL_BARRIER_STR:
            case AT_SKILL_SOUL_BARRIER_PROFICIENCY:
            case AT_SKILL_DEATHSTAB:
            case AT_SKILL_DEATHSTAB_STR:
            case AT_SKILL_IMPALE:
            case AT_SKILL_SWELL_LIFE:
            case AT_SKILL_SWELL_LIFE_STR:
            case AT_SKILL_SWELL_LIFE_PROFICIENCY:
            case AT_SKILL_NOVA:
            case AT_SKILL_IMPROVE_AG:
            case AT_SKILL_ADD_CRITICAL:
            case AT_SKILL_ADD_CRITICAL_STR1:
            case AT_SKILL_ADD_CRITICAL_STR2:
            case AT_SKILL_ADD_CRITICAL_STR3:
            case AT_SKILL_PARTY_TELEPORT:
            case AT_SKILL_STUN:
            case AT_SKILL_REMOVAL_STUN:
            case AT_SKILL_MANA:
            case AT_SKILL_INVISIBLE:
            case AT_SKILL_REMOVAL_BUFF:
            case AT_SKILL_BRAND_OF_SKILL:
                break;
            default:
                if (MONSTER_MOLT <= c->MonsterIndex && c->MonsterIndex <= MONSTER_GREAT_DRAKAN)
                {
                }
                else
                {
                    ITEM* r = &CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT];
                    ITEM* l = &CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT];

                    if ((r->Type >= ITEM_BOW && r->Type < ITEM_BOW + MAX_ITEM_INDEX) && (l->Type >= ITEM_BOW && l->Type < ITEM_BOW + MAX_ITEM_INDEX))
                    {
                        PlayBuffer(static_cast<ESound>(SOUND_ATTACK_MELEE_HIT1 + 5 + rand() % 4), o);
                    }
                    else
                    {
                        PlayBuffer(static_cast<ESound>(SOUND_ATTACK_MELEE_HIT1 + rand() % 4), o);
                    }
                }
                break;
            }
        }

        c->Skill = 0;
        c->Damage = 0;
        c->AttackFlag = ATTACK_FAIL;
    }

    if (c->m_iDeleteTime > 0)
    {
        c->m_iDeleteTime -= FPS_ANIMATION_FACTOR;
    }
    if (static_cast<int>(c->m_iDeleteTime) != -128 && c->m_iDeleteTime <= 0)
    {
        c->m_iDeleteTime = -128;
        DeleteCharacter(c, o);
    }
    battleCastle::SetBuildTimeLocation(o);

    CreateWeaponBlur(c, o, b);

    switch (o->Type)
    {
    case MODEL_BALL:
        CreateFire(0, o, 0.f, 0.f, 0.f);
        break;
    }

    giPetManager::MovePet(c);

    if (c->Dead > 0)
    {
        if (g_isCharacterBuff(o, eBuff_BlessPotion)) g_CharacterUnRegisterBuff(o, eBuff_BlessPotion);
        if (g_isCharacterBuff(o, eBuff_SoulPotion)) g_CharacterUnRegisterBuff(o, eBuff_SoulPotion);
    }
}

void PlayWalkSound()
{
    OBJECT* o = &Hero->Object;
    if (o->CurrentAction == PLAYER_FLY || o->CurrentAction == PLAYER_FLY_CROSSBOW)
    {
    }
    else
    {
        if (gMapManager.WorldActive == WD_2DEVIAS && (HeroTile != 3 && HeroTile < 10))
        {
            PlayBuffer(SOUND_HUMAN_WALK_SNOW);
        }
        else if (gMapManager.WorldActive == WD_0LORENCIA && HeroTile == 0)
        {
            PlayBuffer(SOUND_HUMAN_WALK_GRASS);
        }
        else if (gMapManager.WorldActive == WD_3NORIA && HeroTile == 0)
        {
            PlayBuffer(SOUND_HUMAN_WALK_GRASS);
        }
        else if ((gMapManager.WorldActive == WD_7ATLANSE || gMapManager.InHellas() || gMapManager.WorldActive == WD_67DOPPLEGANGER3) && !Hero->SafeZone)
        {
            PlayBuffer(SOUND_HUMAN_WALK_SWIM);
        }
        else if (IsIceCity())
        {
            PlayBuffer(SOUND_HUMAN_WALK_SNOW);
        }
        else if (IsSantaTown())
        {
            PlayBuffer(SOUND_HUMAN_WALK_SNOW);
        }
        else
        {
            PlayBuffer(SOUND_HUMAN_WALK_GROUND);
        }
    }
}

bool CheckFullSet(CHARACTER* c)
{
    int tmpLevel = 10;
    bool Success = true;
    int  start = 5, end = 1;

    EquipmentLevelSet = 0;

    bool bHero = false;

    if (c == Hero)
    {
        bHero = true;
        start = EQUIPMENT_BOOTS;
        end = EQUIPMENT_HELM;
    }

    if (gCharacterManager.GetBaseClass(c->Class) == CLASS_DARK)
    {
        end = EQUIPMENT_ARMOR;
    }

    if (bHero == true)
    {
        for (int i = start; i >= end; i--)
        {
            if ((gCharacterManager.GetBaseClass(c->Class) == CLASS_RAGEFIGHTER) && (i == EQUIPMENT_GLOVES))
                continue;

            if (CharacterMachine->Equipment[i].Type == -1)
            {
                Success = false;
                break;
            }
        }

        if (Success)
        {
            int Type = CharacterMachine->Equipment[EQUIPMENT_BOOTS].Type % MAX_ITEM_INDEX;
            tmpLevel = CharacterMachine->Equipment[EQUIPMENT_BOOTS].Level;
            for (int i = start; i >= end; i--)
            {
                if ((gCharacterManager.GetBaseClass(c->Class) == CLASS_RAGEFIGHTER) && (i == EQUIPMENT_GLOVES))
                    continue;
                int Level = CharacterMachine->Equipment[i].Level;
                if (Level < 9)
                {
                    EquipmentLevelSet = 0;
                    Success = false;
                    break;
                }
                if (Type != (CharacterMachine->Equipment[i].Type % MAX_ITEM_INDEX))
                {
                    EquipmentLevelSet = 0;
                    Success = false;
                    break;
                }

                if (Level >= 9 && tmpLevel >= Level)
                {
                    tmpLevel = Level;
                    EquipmentLevelSet = Level;
                }
            }
        }

        g_bAddDefense = true;

        if (gCharacterManager.GetBaseClass(c->Class) == CLASS_DARK && Success)
        {
            if (CharacterMachine->Equipment[EQUIPMENT_ARMOR].Type != ITEM_STORM_CROW_ARMOR
                && CharacterMachine->Equipment[EQUIPMENT_ARMOR].Type != ITEM_THUNDER_HAWK_ARMOR
                && CharacterMachine->Equipment[EQUIPMENT_ARMOR].Type != ITEM_HURRICANE_ARMOR
                && CharacterMachine->Equipment[EQUIPMENT_ARMOR].Type != ITEM_VOLCANO_ARMOR
                && CharacterMachine->Equipment[EQUIPMENT_ARMOR].Type != ITEM_VALIANT_ARMOR
                && CharacterMachine->Equipment[EQUIPMENT_ARMOR].Type != ITEM_DESTORY_ARMOR
                && CharacterMachine->Equipment[EQUIPMENT_ARMOR].Type != ITEM_PHANTOM_ARMOR
                )
            {
                g_bAddDefense = false;
            }
        }
    }
    else
    {
        for (int i = 5; i >= end; i--)
        {
            if (c->BodyPart[i].Type == -1)
            {
                Success = false;
                break;
            }
        }

        if (Success)
        {
            int Type = (c->BodyPart[5].Type - MODEL_ITEM) % MAX_ITEM_INDEX;
            tmpLevel = c->BodyPart[5].Level & 0xf;

            for (int i = 5; i >= end; i--)
            {
                int Level = c->BodyPart[i].Level & 0xf;
                if (Level < 9)
                {
                    EquipmentLevelSet = 0;
                    Success = false;
                    break;
                }
                if (Type != (c->BodyPart[i].Type - MODEL_ITEM) % MAX_ITEM_INDEX)
                {
                    EquipmentLevelSet = 0;
                    Success = false;
                    break;
                }

                if (Level >= 9 && tmpLevel >= Level)
                {
                    tmpLevel = Level;
                    EquipmentLevelSet = Level;
                }
            }
        }

        g_bAddDefense = true;

        if (gCharacterManager.GetBaseClass(c->Class) == CLASS_DARK && Success)
        {
            if (c->BodyPart[BODYPART_ARMOR].Type != ITEM_STORM_CROW_ARMOR
                && c->BodyPart[BODYPART_ARMOR].Type != ITEM_THUNDER_HAWK_ARMOR
                && c->BodyPart[BODYPART_ARMOR].Type != ITEM_HURRICANE_ARMOR
                && c->BodyPart[BODYPART_ARMOR].Type != ITEM_VOLCANO_ARMOR
                && c->BodyPart[BODYPART_ARMOR].Type != ITEM_VALIANT_ARMOR
                && c->BodyPart[BODYPART_ARMOR].Type != ITEM_DESTORY_ARMOR
                && c->BodyPart[BODYPART_ARMOR].Type != ITEM_PHANTOM_ARMOR
                )
            {
                g_bAddDefense = false;
            }
        }
    }

    if (gMapManager.InChaosCastle())
    {
        ChangeChaosCastleUnit(c);
    }
    return Success;
}

void MoveEye(OBJECT* o, BMD* b, int Right, int Left, int Right2, int Left2, int Right3, int Left3)
{
    vec3_t p;
    Vector(0.f, 0.f, 0.f, p);
    b->TransformPosition(o->BoneTransform[Right], p, o->EyeRight, true);
    Vector(0.f, 0.f, 0.f, p);
    b->TransformPosition(o->BoneTransform[Left], p, o->EyeLeft, true);
    if (Right2 != -1)
        b->TransformPosition(o->BoneTransform[Right2], p, o->EyeRight2, true);
    if (Left2 != -1)
        b->TransformPosition(o->BoneTransform[Left2], p, o->EyeLeft2, true);
    if (Right3 != -1)
        b->TransformPosition(o->BoneTransform[Right3], p, o->EyeRight3, true);
    if (Left3 != -1)
        b->TransformPosition(o->BoneTransform[Left3], p, o->EyeLeft3, true);
}

void MonsterDieSandSmoke(OBJECT* o)
{
    if (o->CurrentAction == MONSTER01_DIE && o->AnimationFrame >= 8.f && o->AnimationFrame < 9.f)
    {
        vec3_t Position;
        for (int i = 0; i < 20; i++)
        {
            Vector(1.f, 1.f, 1.f, o->Light);
            Vector(o->Position[0] + (float)(rand() % 64 - 32),
                o->Position[1] + (float)(rand() % 64 - 32),
                o->Position[2] + (float)(rand() % 32 - 16), Position);
            if (rand_fps_check(1))
            {
                CreateParticle(BITMAP_SMOKE + 1, Position, o->Angle, o->Light, 1);
            }
        }
    }
}

void MonsterMoveSandSmoke(OBJECT* o)
{
    if (o->CurrentAction == MONSTER01_WALK)
    {
        vec3_t Position;
        Vector(o->Position[0] + rand() % 200 - 100, o->Position[1] + rand() % 200 - 100, o->Position[2], Position);
        if (rand_fps_check(1))
        {
            CreateParticle(BITMAP_SMOKE + 1, Position, o->Angle, o->Light);
        }
    }
}

void MoveCharacterVisual(CHARACTER* c, OBJECT* o)
{
    BMD* b = &Models[o->Type];
    if (b->NumActions == 0)
    {
        VectorCopy(o->BoundingBoxMin, o->OBB.StartPos);
        o->OBB.XAxis[0] = (o->BoundingBoxMax[0] - o->BoundingBoxMin[0]);
        o->OBB.YAxis[1] = (o->BoundingBoxMax[1] - o->BoundingBoxMin[1]);
        o->OBB.ZAxis[2] = (o->BoundingBoxMax[2] - o->BoundingBoxMin[2]);
        VectorAdd(o->OBB.StartPos, o->Position, o->OBB.StartPos);
        o->OBB.XAxis[1] = 0.f;
        o->OBB.XAxis[2] = 0.f;
        o->OBB.YAxis[0] = 0.f;
        o->OBB.YAxis[2] = 0.f;
        o->OBB.ZAxis[0] = 0.f;
        o->OBB.ZAxis[1] = 0.f;
        return;
    }
    VectorCopy(o->Position, b->BodyOrigin);
    b->BodyScale = o->Scale;
    b->CurrentAction = o->CurrentAction;

    if (o->Visible)
    {
        int Index = TERRAIN_INDEX_REPEAT((c->PositionX), (c->PositionY));
        if ((TerrainWall[Index] & TW_SAFEZONE) == TW_SAFEZONE)
            c->SafeZone = true;
        else
            c->SafeZone = false;

        if (!g_isCharacterBuff(o, eDeBuff_Harden) && !g_isCharacterBuff(o, eDeBuff_Stun)
            && !g_isCharacterBuff(o, eDeBuff_Sleep)
            && !g_isCharacterBuff(o, eBuff_Att_up_Ourforces)
            && !g_isCharacterBuff(o, eBuff_Hp_up_Ourforces)
            && !g_isCharacterBuff(o, eBuff_Def_up_Ourforces))
        {
            if (o->Type != MODEL_PLAYER)
                MoveHead(c);
            if (c != Hero && c->Dead == 0 && rand_fps_check(32))
            {
                o->HeadTargetAngle[0] = (float)(rand() % 128 - 64);
                o->HeadTargetAngle[1] = (float)(rand() % 32 - 16);
                for (int i = 0; i < 2; i++)
                    if (o->HeadTargetAngle[i] < 0) o->HeadTargetAngle[i] += 360.f;
            }

            for (int j = 0; j < 2; j++)
                o->HeadAngle[j] = TurnAngle2(o->HeadAngle[j], o->HeadTargetAngle[j], FarAngle(o->HeadAngle[j], o->HeadTargetAngle[j]) * 0.2f);
        }

        vec3_t p, Position;
        vec3_t Light;
        float Luminosity = 0.8f;
        if (c->Appear > 0)
        {
            c->Appear -= FPS_ANIMATION_FACTOR;
            for (int i = 0; i < 20; i++)
            {
                Vector(1.f, 1.f, 1.f, o->Light);
                Vector(o->Position[0] + (float)(rand() % 64 - 32),
                    o->Position[1] + (float)(rand() % 64 - 32),
                    o->Position[2] + (float)(rand() % 32 - 16), Position);
                if (rand_fps_check(10))
                    CreateParticle(BITMAP_SMOKE + 1, Position, o->Angle, o->Light, 1);
                if (rand_fps_check(10))
                    CreateEffect(MODEL_STONE1 + rand() % 2, o->Position, o->Angle, o->Light);
            }
        }
        else
        {
            c->Appear = 0;
        }

        if (c->PK < PVP_MURDERER2)
        {
            for (int j = 0; j < 2; j++)
            {
                if (c->Weapon[j].Type == MODEL_HELIACAL_SWORD)
                {
                    Vector(Luminosity, Luminosity * 0.8f, Luminosity * 0.5f, Light);
                    AddTerrainLight(o->Position[0], o->Position[1], Light, 3, PrimaryTerrainLight);
                }
                else if (c->Weapon[j].Type == MODEL_DIVINE_SWORD_OF_ARCHANGEL || c->Weapon[j].Type == MODEL_DIVINE_CB_OF_ARCHANGEL || c->Weapon[j].Type == MODEL_DIVINE_STAFF_OF_ARCHANGEL || c->Weapon[j].Type == MODEL_DIVINE_SCEPTER_OF_ARCHANGEL)
                {
                    Vector(Luminosity * 0.8f, Luminosity * 0.5f, Luminosity * 0.3f, Light);
                    AddTerrainLight(o->Position[0], o->Position[1], Light, 2, PrimaryTerrainLight);
                }
            }
        }
        if (c->Freeze > 0.f)
        {
            if (c->FreezeType == BITMAP_ICE && rand_fps_check(4))
            {
                Vector(o->Position[0] + (float)(rand() % 100 - 50), o->Position[1] + (float)(rand() % 100 - 50), o->Position[2] + (float)(rand() % 180), Position);
                //CreateParticle(BITMAP_SHINY,Position,o->Angle,o->Light);
                //CreateParticle(BITMAP_SHINY,Position,o->Angle,o->Light,1);
            }
            if (c->FreezeType == BITMAP_BURN)
            {
                Vector(0.f, 0.f, -20.f, Position);
                for (int i = 1; i < b->NumBones;)
                {
                    if (!b->Bones[i].Dummy)
                    {
                        b->TransformPosition(o->BoneTransform[i], Position, p);
                        //CreateParticle(BITMAP_FIRE+1,p,o->Angle,o->Light,rand()%4);
                    }
                    int d = (int)(4.f / c->Freeze);
                    if (d < 1) d = 1;
                    i += d;
                }
            }
        }
        Vector(1.f, 1.f, 1.f, Light);
        Vector(0.f, 0.f, 0.f, p);
        Luminosity = (float)(rand() % 8 + 2) * 0.1f;
        bool Smoke = false;
        switch (o->Type)
        {
        case MODEL_PLAYER:
            if (SceneFlag == MAIN_SCENE
                && (gMapManager.WorldActive == WD_7ATLANSE || gMapManager.WorldActive == WD_67DOPPLEGANGER3)
                && (int)WorldTime % 10000 < 1000
                && rand_fps_check(1))
            {
                Vector(0.f, 20.f, -10.f, p);
                b->TransformPosition(o->BoneTransform[b->BoneHead], p, Position, true);
                CreateParticle(BITMAP_BUBBLE, Position, o->Angle, Light);
            }
            Vector(1.f, 1.f, 1.f, Light);
            Vector(-15.f, 0.f, 0.f, p);
            if (gMapManager.InDevilSquare() == true)
            {
                if (rand_fps_check(4))
                {
                    b->TransformPosition(o->BoneTransform[26], p, Position, true);
                    CreateParticle(BITMAP_RAIN_CIRCLE + 1, Position, o->Angle, Light);
                }
                if (rand_fps_check(4))
                {
                    b->TransformPosition(o->BoneTransform[35], p, Position, true);
                    CreateParticle(BITMAP_RAIN_CIRCLE + 1, Position, o->Angle, Light);
                }
            }
            if (o->CurrentAction == PLAYER_SKILL_HELL_BEGIN || o->CurrentAction == PLAYER_SKILL_HELL_START && rand_fps_check(1))
            {
                if (o->BoneTransform != NULL)
                {
                    Vector(0.3f, 0.3f, 1.f, Light);

                    for (int i = 0; i < 40; i += 2)
                    {
                        if (!b->Bones[i].Dummy && i < b->NumBones)
                        {
                            b->TransformPosition(o->BoneTransform[i], p, Position, true);

                            for (int j = 0; j < o->m_bySkillCount + 1; ++j)
                            {
                                CreateParticle(BITMAP_LIGHT, Position, o->Angle, Light, 6, 1.3f + (o->m_bySkillCount * 0.08f));
                            }
                        }
                    }
                }
                VectorCopy(o->Position, Position);
                CreateForce(o, Position);
            }

            if (o->CurrentAction == PLAYER_SKILL_HELL && rand_fps_check(1))
            {
                for (int i = 0; i < 10; i++)
                {
                    b->TransformPosition(o->BoneTransform[rand() % b->NumBones], p, Position, true);
                    CreateParticle(BITMAP_FIRE, Position, o->Angle, Light);
                }
            }

            Vector(0.f, -30.f, 0.f, p);
            b->TransformPosition(o->BoneTransform[c->Weapon[0].LinkBone], p, Position, true);
            break;

        case MODEL_MOLT:
            break;
        case MODEL_ALQUAMOS:
            break;
        case MODEL_QUEEN_RAINER:
        {
            vec3_t pos1, pos2;

            for (int i = 2; i < 5; ++i)
            {
                b->TransformPosition(o->BoneTransform[i], p, pos1, true);
                b->TransformPosition(o->BoneTransform[i + 1], p, pos2, true);
                CreateJoint(BITMAP_JOINT_THUNDER, pos1, pos2, o->Angle, 7, NULL, 14.f);
            }

            for (int i = 9; i < 11; ++i)
            {
                if (i == 9)
                    b->TransformPosition(o->BoneTransform[2], p, pos1, true);
                else
                    b->TransformPosition(o->BoneTransform[i], p, pos1, true);
                b->TransformPosition(o->BoneTransform[i + 1], p, pos2, true);
                CreateJoint(BITMAP_JOINT_THUNDER, pos1, pos2, o->Angle, 7, NULL, 14.f);
            }

            b->TransformPosition(o->BoneTransform[2], p, pos1, true);
            b->TransformPosition(o->BoneTransform[18], p, pos2, true);
            CreateJoint(BITMAP_JOINT_THUNDER, pos1, pos2, o->Angle, 7, NULL, 14.f);

            b->TransformPosition(o->BoneTransform[22], p, pos1, true);
            CreateJoint(BITMAP_JOINT_THUNDER, pos2, pos1, o->Angle, 7, NULL, 14.f);

            b->TransformPosition(o->BoneTransform[23], p, pos2, true);
            CreateJoint(BITMAP_JOINT_THUNDER, pos1, pos2, o->Angle, 7, NULL, 14.f);

            b->TransformPosition(o->BoneTransform[24], p, pos1, true);
            CreateJoint(BITMAP_JOINT_THUNDER, pos2, pos1, o->Angle, 7, NULL, 14.f);

            b->TransformPosition(o->BoneTransform[25], p, pos2, true);
            CreateJoint(BITMAP_JOINT_THUNDER, pos1, pos2, o->Angle, 7, NULL, 14.f);

            b->TransformPosition(o->BoneTransform[18], p, pos1, true);
            b->TransformPosition(o->BoneTransform[31], p, pos2, true);
            CreateJoint(BITMAP_JOINT_THUNDER, pos1, pos2, o->Angle, 7, NULL, 14.f);

            b->TransformPosition(o->BoneTransform[32], p, pos1, true);
            CreateJoint(BITMAP_JOINT_THUNDER, pos2, pos1, o->Angle, 7, NULL, 14.f);

            b->TransformPosition(o->BoneTransform[33], p, pos2, true);
            CreateJoint(BITMAP_JOINT_THUNDER, pos1, pos2, o->Angle, 7, NULL, 14.f);

            b->TransformPosition(o->BoneTransform[34], p, pos1, true);
            CreateJoint(BITMAP_JOINT_THUNDER, pos2, pos1, o->Angle, 7, NULL, 14.f);
        }
        break;
        case MODEL_CRUST:
            o->BlendMeshTexCoordU = -(float)((int)(WorldTime) % 10000) * 0.0004f;
            break;
        case MODEL_DARK_PHEONIX_SHIELD:
            o->BlendMeshTexCoordV = (float)((int)(WorldTime) % 10000) * 0.0001f;
            break;
        case MODEL_DRAKAN:
            break;
        case MODEL_CURSED_KING:
            if (0 == ((int)rand() % 5))
            {
                Position[0] = o->Position[0] + ((rand() % 21) - 10) * ((float)TERRAIN_SIZE / 70);
                Position[1] = o->Position[1] + ((rand() % 21) - 10) * ((float)TERRAIN_SIZE / 70);
                CreatePointer(BITMAP_BLOOD, Position, o->Angle[0], o->Light, 0.65f);
            }
            break;
        case MODEL_RED_SKELETON_KNIGHT_1:
        case MODEL_MUTANT:
            MoveEye(o, b, 8, 9);
            MonsterMoveSandSmoke(o);
            //MonsterDieSandSmoke(o);
            break;
        case MODEL_BEAM_KNIGHT://
            MoveEye(o, b, 8, 9);
            if (c->MonsterIndex == MONSTER_DEATH_BEAM_KNIGHT)
            {
                wchar_t    body[2] = { 30,0 };
                char    head = 1;
                vec3_t  vec[35];
                vec3_t  angle;
                vec3_t  dist;
                vec3_t  p;

                Vector(0.f, 0.f, 0.f, angle);
                Vector(0.f, 0.f, 0.f, p);

                for (int i = 0; i < 35; ++i)
                {
                    b->TransformPosition(o->BoneTransform[vec_list[i]], p, vec[i], true);
                }

                char start, end;
                float scale = 1.0f;

                for (int i = 0; i < 15; ++i)
                {
                    if (i >= 11)
                    {
                        scale = 0.5f;
                    }

                    start = wingLeft[i][0];
                    end = wingLeft[i][1];

                    dist[0] = MoveHumming(vec[end], angle, vec[start], 360.0f);
                    if (rand_fps_check(1))
                    {
                        CreateParticle(BITMAP_FLAME, vec[start], angle, dist, 2, scale);
                    }

                    start = wingRight[i][0];
                    end = wingRight[i][1];

                    dist[0] = MoveHumming(vec[end], angle, vec[start], 360.0f);
                    if (rand_fps_check(1))
                    {
                        CreateParticle(BITMAP_FLAME, vec[start], angle, dist, 2, scale);
                    }
                }

                for (int i = 0; i < 4; ++i)
                {
                    start = arm_leg_Left[i][0];
                    end = arm_leg_Left[i][1];

                    dist[0] = MoveHumming(vec[end], angle, vec[start], 360.0f);
                    if (rand_fps_check(1))
                    {
                        CreateParticle(BITMAP_FLAME, vec[start], angle, dist, 2, 0.6f);
                    }

                    start = arm_leg_Right[i][0];
                    end = arm_leg_Right[i][1];

                    dist[0] = MoveHumming(vec[end], angle, vec[start], 360.0f);
                    if (rand_fps_check(1))
                    {
                        CreateParticle(BITMAP_FLAME, vec[start], angle, dist, 2, 0.6f);
                    }
                }

                if ((int)WorldTime % 2 == 0)
                {
                    start = body[0];
                    end = body[1];

                    dist[0] = MoveHumming(vec[end], angle, vec[start], 360.0f);
                    if (rand_fps_check(1))
                    {
                        CreateParticle(BITMAP_FLAME, vec[start], angle, dist, 2, 1.3f);
                        CreateParticle(BITMAP_FLAME, vec[head], angle, dist, 3, 0.5f);
                    }
                }

                Vector(-1.3f, -1.3f, -1.3f, Light);
                AddTerrainLight(o->Position[0], o->Position[1], Light, 3, PrimaryTerrainLight);
            }
            else
            {
                vec3_t  pos, angle;
                Vector(0.f, 0.f, 0.f, angle);

                Luminosity = (float)sinf(WorldTime * 0.002f) * 0.3f + 0.7f;

                Vector(Luminosity, Luminosity * 0.5f, Luminosity * 0.5f, Light);

                b->TransformPosition(o->BoneTransform[55], p, pos, true);
                b->TransformPosition(o->BoneTransform[62], p, Position, true);
                MoveHumming(pos, angle, Position, 360.0f);
                if (rand_fps_check(1))
                {
                    CreateParticle(BITMAP_FLAME, Position, angle, Light, 1, 0.2f);
                }

                b->TransformPosition(o->BoneTransform[70], p, pos, true);
                b->TransformPosition(o->BoneTransform[77], p, Position, true);
                MoveHumming(pos, angle, Position, 360.0f);
                if (rand_fps_check(1))
                {
                    CreateParticle(BITMAP_FLAME, Position, angle, Light, 1, 0.2f);
                }

                MonsterMoveSandSmoke(o);
                MonsterDieSandSmoke(o);
            }
            break;
        case MODEL_BLOODY_WOLF:
            MoveEye(o, b, 11, 12);
            MonsterMoveSandSmoke(o);
            //MonsterDieSandSmoke(o);
            break;
        case MODEL_TANTALLOS://
            MoveEye(o, b, 24, 25);
            if (o->SubType == 1)
            {
                if (rand_fps_check(1))
                {
                    b->TransformPosition(o->BoneTransform[6], p, Position, true);
                    CreateParticle(BITMAP_FIRE, Position, o->Angle, Light);
                    b->TransformPosition(o->BoneTransform[13], p, Position, true);
                    CreateParticle(BITMAP_FIRE, Position, o->Angle, Light);
                }

                Vector(-1.3f, -1.3f, -1.3f, Light);
                AddTerrainLight(o->Position[0], o->Position[1], Light, 3, PrimaryTerrainLight);
            }
            else
            {
                MonsterMoveSandSmoke(o);
                MonsterDieSandSmoke(o);
            }
            break;
        case MODEL_GOLDEN_WHEEL:
            MoveEye(o, b, 8, 9);
            MonsterMoveSandSmoke(o);
            MonsterDieSandSmoke(o);
            break;
        case MODEL_TITAN:
            MoveEye(o, b, 28, 27);
            break;
        case MODEL_HYDRA:
            if (o->CurrentAction >= MONSTER01_ATTACK1 && o->CurrentAction <= MONSTER01_ATTACK2)
            {
                o->BlendMeshLight += 0.1f;
                if (o->BlendMeshLight > 1.f)
                    o->BlendMeshLight = 1.f;
            }
            else
            {
                o->BlendMeshLight -= 0.1f;
                if (o->BlendMeshLight < 0.f)
                    o->BlendMeshLight = 0.f;
            }
            break;
        case MODEL_DEATH_KNIGHT:
            o->BlendMesh = 3;
            o->BlendMeshTexCoordV = -(float)((int)(WorldTime) % 1000) * 0.001f;
            if (rand_fps_check(2))
            {
                Vector(0.f, 0.f, 0.f, p);
                b->TransformPosition(o->BoneTransform[2], p, Position, true);
                CreateParticle(BITMAP_FIRE, Position, o->Angle, Light);
            }
            break;
        case MODEL_BALROG:
            o->BlendMeshTexCoordV = -(float)((int)(WorldTime) % 1000) * 0.001f;
            break;
        case MODEL_DEVIL:
            o->BlendMeshTexCoordU = -(float)((int)(WorldTime) % 10000) * 0.0001f;
            break;
        case MODEL_BALI:
            Vector(0.f, 0.f, 0.f, p);
            Vector(0.6f, 1.f, 0.8f, Light);
            if (o->CurrentAction == MONSTER01_ATTACK1 && rand_fps_check(1))
            {
                b->TransformPosition(o->BoneTransform[33], p, Position, true);
                CreateParticle(BITMAP_ENERGY, Position, o->Angle, Light);
                Vector(1.f, 0.6f, 1.f, Light);
                CreateParticle(BITMAP_FIRE, Position, o->Angle, Light);
            }
            if (o->CurrentAction == MONSTER01_ATTACK2 && rand_fps_check(1))
            {
                b->TransformPosition(o->BoneTransform[20], p, Position, true);
                CreateParticle(BITMAP_ENERGY, Position, o->Angle, Light);
                Vector(1.f, 0.6f, 1.f, Light);
                CreateParticle(BITMAP_FIRE, Position, o->Angle, Light);
            }
            if (o->CurrentAction == MONSTER01_ATTACK3 && rand_fps_check(1))
            {
                b->TransformPosition(o->BoneTransform[41], p, Position, true);
                CreateParticle(BITMAP_ENERGY, Position, o->Angle, Light);
                Vector(1.f, 0.6f, 1.f, Light);
                CreateParticle(BITMAP_FIRE, Position, o->Angle, Light);
            }
            if (o->CurrentAction == MONSTER01_ATTACK4 && rand_fps_check(1))
            {
                b->TransformPosition(o->BoneTransform[49], p, Position, true);
                CreateParticle(BITMAP_ENERGY, Position, o->Angle, Light);
                Vector(1.f, 0.6f, 1.f, Light);
                CreateParticle(BITMAP_FIRE, Position, o->Angle, Light);
            }
            if (o->CurrentAction == MONSTER01_DIE && o->AnimationFrame < 12.f && rand_fps_check(1))
            {
                Vector(0.1f, 0.8f, 0.6f, Light);

                for (int i = 0; i < 20; i++)
                {
                    b->TransformPosition(o->BoneTransform[rand() % b->NumBones], p, Position, true);
                    CreateParticle(BITMAP_FIRE, Position, o->Angle, Light);
                }
            }
            break;
        case MODEL_GORGON:
            o->BlendMeshLight = (float)(rand() % 10) * 0.1f;
            if (c->Level == 2)
            {
                if (rand_fps_check(1))
                {
                    for (int i = 0; i < 10; i++)
                    {
                        b->TransformPosition(o->BoneTransform[rand() % b->NumBones], p, Position, true);
                        CreateParticle(BITMAP_FIRE, Position, o->Angle, Light);
                    }
                }

                Vector(Luminosity * 1.f, Luminosity * 0.2f, Luminosity * 0.f, Light);
                AddTerrainLight(o->Position[0], o->Position[1], Light, 2, PrimaryTerrainLight);
            }
            break;
        case MODEL_ICE_MONSTER:
            o->BlendMeshTexCoordV = -(float)((int)(WorldTime) % 2000) * 0.0005f;
            break;
        case MODEL_MIX_NPC:
            if (rand_fps_check(64))
                PlayBuffer(SOUND_NPC_MIX);
            break;
        case MODEL_ELF_WIZARD:
            if (rand_fps_check(256))
                PlayBuffer(SOUND_NPC_HARP);
            break;
        case MODEL_SMITH:
            if (g_isCharacterBuff(o, eBuff_CrywolfNPCHide))
                break;
            if (o->CurrentAction == 0 && o->AnimationFrame >= 5.f && o->AnimationFrame <= 10.f)
                PlayBuffer(SOUND_NPC_BLACK_SMITH);
            o->BlendMesh = 4;
            o->BlendMeshLight = Luminosity;
            Vector(Luminosity * 1.f, Luminosity * 0.4f, Luminosity * 0.f, Light);
            AddTerrainLight(o->Position[0], o->Position[1], Light, 3, PrimaryTerrainLight);
            Vector(1.f, 1.f, 1.f, Light);
            Vector(0.f, 0.f, 0.f, p);
            if (o->CurrentAction == 0 && o->AnimationFrame >= 5.f && o->AnimationFrame <= 6.f && rand_fps_check(1))
            {
                b->TransformPosition(o->BoneTransform[17], p, Position, true);
                vec3_t Angle;
                for (int i = 0; i < 4; i++)
                {
                    Vector((float)(rand() % 60 + 60 + 90), 0.f, (float)(rand() % 30), Angle);
                    CreateJoint(BITMAP_JOINT_SPARK, Position, Position, Angle);
                    CreateParticle(BITMAP_SPARK, Position, Angle, Light);
                }
            }
            break;
        case MODEL_DEVIAS_TRADER:
            Vector(1.f, 1.f, 1.f, Light);
            Vector(0.f, 5.f, 10.f, p);
            if (o->CurrentAction == 0)
            {
                Vector(Luminosity * 0.5f, Luminosity * 0.3f, Luminosity * 0.f, Light);
                AddTerrainLight(o->Position[0], o->Position[1], Light, 3, PrimaryTerrainLight);
                b->TransformPosition(o->BoneTransform[37], p, Position, true);
                vec3_t Angle;
                for (int i = 0; i < 4; i++)
                {
                    if (rand_fps_check(1))
                    {
                        Vector((float)(rand() % 60 + 60 + 30), 0.f, (float)(rand() % 30), Angle);
                        CreateJoint(BITMAP_JOINT_SPARK, Position, Position, Angle);
                        if (rand() % 2)
                            CreateParticle(BITMAP_SPARK, Position, Angle, Light);
                    }
                }
            }
            break;
        case MODEL_WEDDING_NPC:
            if (o->CurrentAction == 1 && (o->AnimationFrame > 4.5f && o->AnimationFrame <= 4.8f && rand_fps_check(1)))
            {
                CreateEffect(BITMAP_FIRECRACKER0001, o->Position, o->Angle, o->Light, 0);
            }
            break;
        case MODEL_BUDGE_DRAGON:
            if (o->CurrentAction == MONSTER01_ATTACK1 && o->AnimationFrame <= 4.f && rand_fps_check(1))
            {
                vec3_t Light;
                Vector(1.f, 1.f, 1.f, Light);
                Vector(0.f, (float)(rand() % 32 + 32), 0.f, p);
                b->TransformPosition(o->BoneTransform[7], p, Position, true);
                CreateParticle(BITMAP_FIRE, Position, o->Angle, Light, 1);
            }
        case MODEL_DARK_KNIGHT:
        case MODEL_LARVA:
        case MODEL_CHAIN_SCORPION:
            if (o->Type == MODEL_CHAIN_SCORPION)
            {
                Vector(0.f, 0.f, 0.f, p);
                b->TransformPosition(o->BoneTransform[7], p, Position, true);
                Vector(Luminosity * 1.f, Luminosity * 0.4f, Luminosity * 0.2f, Light);
                CreateSprite(BITMAP_LIGHT, Position, 1.f, Light, o);
            }
            if (c->Dead == 0 && rand_fps_check(4))
            {
                Vector(o->Position[0] + (float)(rand() % 64 - 32),
                    o->Position[1] + (float)(rand() % 64 - 32),
                    o->Position[2] + (float)(rand() % 32 - 16), Position);
                if (gMapManager.WorldActive == WD_2DEVIAS)
                    CreateParticle(BITMAP_SMOKE, Position, o->Angle, Light);
                else
                    CreateParticle(BITMAP_SMOKE + 1, Position, o->Angle, Light);
            }
            break;
        case MODEL_BAHAMUT:
            if (c->Dead == 0 && c->Level == 1 && rand_fps_check(4))
            {
                Vector(o->Position[0] + (float)(rand() % 64 - 32),
                    o->Position[1] + (float)(rand() % 64 - 32),
                    o->Position[2] + (float)(rand() % 32 - 16), Position);
                CreateParticle(BITMAP_SMOKE + 1, Position, o->Angle, Light);
            }
            break;
        case MODEL_GIANT:
            MonsterDieSandSmoke(o);
            break;
        case MODEL_YETI:
        case MODEL_ELITE_YETI:
            if (rand_fps_check(4))
            {
                Vector(0.f, 0.f, 0.f, p);
                b->TransformPosition(o->BoneTransform[22], p, Position, true);
                CreateParticle(BITMAP_SMOKE, Position, o->Angle, o->Light);
            }
            break;
        case MODEL_BULL_FIGHTER:
            if (o->CurrentAction == MONSTER01_STOP1 &&
                (o->AnimationFrame >= 15.f && o->AnimationFrame <= 20.f)) Smoke = true;
            if (o->CurrentAction == MONSTER01_STOP2 &&
                (o->AnimationFrame >= 20.f && o->AnimationFrame <= 25.f)) Smoke = true;
            if (o->CurrentAction == MONSTER01_WALK &&
                ((o->AnimationFrame >= 2.f && o->AnimationFrame <= 3.f) ||
                    (o->AnimationFrame >= 5.f && o->AnimationFrame <= 6.f))) Smoke = true;
            if (Smoke)
            {
                if (rand_fps_check(2))
                {
                    Vector(0.f, -4.f, 0.f, p);
                    b->TransformPosition(o->BoneTransform[24], p, Position, true);
                    CreateParticle(BITMAP_SMOKE, Position, o->Angle, o->Light);
                }
            }
            break;
        default:
        {
            if (MoveHellasMonsterVisual(o, b)) break;
            if (battleCastle::MoveBattleCastleMonsterVisual(o, b)) break;
            if (M31HuntingGround::MoveHuntingGroundMonsterVisual(o, b)) break;
            if (M34CryingWolf2nd::MoveCryingWolf2ndMonsterVisual(o, b)) break;
            if (M34CryWolf1st::MoveCryWolf1stMonsterVisual(c, o, b)) break;
            if (M33Aida::MoveAidaMonsterVisual(o, b)) break;
            if (M37Kanturu1st::MoveKanturu1stMonsterVisual(c, o, b)) break;
            if (M38Kanturu2nd::Move_Kanturu2nd_MonsterVisual(c, o, b)) break;
            if (M39Kanturu3rd::MoveKanturu3rdMonsterVisual(o, b)) break;
            if (SEASON3A::CGM3rdChangeUp::Instance().MoveBalgasBarrackMonsterVisual(c, o, b))	break;
            if (g_NewYearsDayEvent->MoveMonsterVisual(c, o, b) == true) break;
            if (g_CursedTemple->MoveMonsterVisual(o, b) == true) break;
            if (SEASON3B::GMNewTown::MoveMonsterVisual(o, b) == true) break;
            if (SEASON3C::GMSwampOfQuiet::MoveMonsterVisual(o, b) == true) break;
            if (g_09SummerEvent->MoveMonsterVisual(c, o, b)) break;
            if (g_EmpireGuardian1.MoveMonsterVisual(c, o, b) == true) break;
            if (g_EmpireGuardian2.MoveMonsterVisual(c, o, b) == true) break;
            if (g_EmpireGuardian3.MoveMonsterVisual(c, o, b) == true) break;
            if (g_EmpireGuardian4.MoveMonsterVisual(c, o, b) == true) break;
            if (TheMapProcess().MoveMonsterVisual(o, b) == true) break;
        }
        break;
        }

        if (o->Type == MODEL_PLAYER && c == Hero)
        {
            if ((o->CurrentAction >= PLAYER_WALK_MALE && o->CurrentAction <= PLAYER_RUN_RIDE_WEAPON)
                || (o->CurrentAction == PLAYER_WALK_TWO_HAND_SWORD_TWO || o->CurrentAction == PLAYER_RUN_TWO_HAND_SWORD_TWO)
                || (o->CurrentAction == PLAYER_RUN_RIDE_HORSE)
                || (o->CurrentAction == PLAYER_RAGE_UNI_RUN || o->CurrentAction == PLAYER_RAGE_UNI_RUN_ONE_RIGHT))
            {
                Vector(0.f, 0.f, 0.f, p);
                if (o->AnimationFrame >= 1.5f && !c->Foot[0])
                {
                    c->Foot[0] = true;
                    PlayWalkSound();
                }
                if (o->AnimationFrame >= 4.5f && !c->Foot[1])
                {
                    c->Foot[1] = true;
                    PlayWalkSound();
                }
            }
        }

        if ((o->CurrentAction == PLAYER_RUN_RIDE
            || o->CurrentAction == PLAYER_RAGE_UNI_RUN
            || o->CurrentAction == PLAYER_RAGE_UNI_RUN_ONE_RIGHT
            || o->CurrentAction == PLAYER_RUN_RIDE_WEAPON || o->CurrentAction == PLAYER_RUN_SWIM || o->CurrentAction == PLAYER_WALK_SWIM || o->CurrentAction == PLAYER_FLY || o->CurrentAction == PLAYER_FLY_CROSSBOW || o->CurrentAction == PLAYER_RUN_RIDE_HORSE)
            && o->Type == MODEL_PLAYER
            && gMapManager.InHellas()
            && rand_fps_check(1))
        {
            vec3_t Light = { 0.3f, 0.3f, 0.3f };
            VectorCopy(o->Position, Position);

            float  Matrix[3][4];

            Vector(0.f, -40.f, 0.f, p);

            AngleMatrix(o->Angle, Matrix);
            VectorRotate(p, Matrix, Position);
            VectorAddScaled(o->Position, Position, Position, FPS_ANIMATION_FACTOR);

            Position[0] += rand() % 64 - 32.f;
            Position[1] += rand() % 64 - 32.f;
            Position[2] += 50.f;
            VectorScale(Position, FPS_ANIMATION_FACTOR, Position)

            CreateParticle(BITMAP_WATERFALL_5, Position, o->Angle, Light, 1);
        }
    }
}

float CharacterMoveSpeed(CHARACTER* c)
{
    OBJECT* o = &c->Object;
    auto Speed = (float)c->MoveSpeed;
    if (o->Type == MODEL_PLAYER && o->Kind == KIND_PLAYER)
    {
        bool isholyitem = false;

        isholyitem = g_pCursedTempleWindow->CheckInventoryHolyItem(c);

        if (isholyitem)
        {
            c->Run = 40;
            Speed = 8;
            return Speed;
        }

        if (c->Helper.Type == MODEL_HORN_OF_FENRIR && !c->SafeZone && !isholyitem)
        {
            if (c->Run < FENRIR_RUN_DELAY / 2)
                Speed = 15;
            else if (c->Run < FENRIR_RUN_DELAY)
                Speed = 16;
            else
            {
                if (c->Helper.ExcellentFlags > 0)
                    Speed = 19;
                else
                    Speed = 17;
            }
        }
        else if (c->Helper.Type == MODEL_DARK_HORSE_ITEM && !c->SafeZone && !isholyitem)
        {
            c->Run = 40;
            Speed = 17;
        }
        else if (!(c->Object.SubType == MODEL_CURSEDTEMPLE_ALLIED_PLAYER || c->Object.SubType == MODEL_CURSEDTEMPLE_ILLUSION_PLAYER)
            && (c->Wing.Type != -1 || (c->Helper.Type >= MODEL_HORN_OF_UNIRIA && c->Helper.Type <= MODEL_HORN_OF_DINORANT)) && !c->SafeZone
            && !isholyitem)
        {
            if (c->Wing.Type == MODEL_WINGS_OF_DRAGON
                || c->Wing.Type == MODEL_WING_OF_STORM
                )
            {
                c->Run = 40;
                Speed = 16;
            }
            else
            {
                c->Run = 40;
                Speed = 15;
            }
        }
        else if (!isholyitem)
        {
            if (c->Run < 40)
                Speed = 12;
            else
                Speed = 15;
        }
    }
#ifndef GUILD_WAR_EVENT
    if (gMapManager.InChaosCastle() == true)
    {
        c->Run = 40;
        Speed = 15;
    }
#endif// GUILD_WAR_EVENT

    if (g_isCharacterBuff((&c->Object), eDeBuff_Freeze))
    {
        Speed *= 0.5f;
    }
    else if (g_isCharacterBuff((&c->Object), eDeBuff_BlowOfDestruction))
    {
        Speed *= 0.33f;
    }

    if (g_isCharacterBuff((&c->Object), eBuff_CursedTempleQuickness))
    {
        c->Run = 40;
        Speed = 20;
    }

    return Speed;
}

void MoveCharacterPosition(CHARACTER* c)
{
    OBJECT* o = &c->Object;
    float Matrix[3][4];
    AngleMatrix(o->Angle, Matrix);
    vec3_t v, Velocity;
    Vector(0.f, -CharacterMoveSpeed(c), 0.f, v);
    VectorRotate(v, Matrix, Velocity);
    VectorAddScaled(o->Position, Velocity, o->Position, FPS_ANIMATION_FACTOR);

    if (gMapManager.WorldActive == -1 || c->Helper.Type != MODEL_HORN_OF_DINORANT || c->SafeZone)
    {
        o->Position[2] = RequestTerrainHeight(o->Position[0], o->Position[1]);
    }
    else
    {
        if (gMapManager.WorldActive == WD_8TARKAN || gMapManager.WorldActive == WD_10HEAVEN)
            o->Position[2] = RequestTerrainHeight(o->Position[0], o->Position[1]) + 90.f;
        else
            o->Position[2] = RequestTerrainHeight(o->Position[0], o->Position[1]) + 30.f;
    }
    if (o->Type == MODEL_BUDGE_DRAGON)
    {
        o->Position[2] += (-absf(sinf(o->Timer)) * 70.f + 70.f) * FPS_ANIMATION_FACTOR;
    }
    o->Timer += 0.15f * FPS_ANIMATION_FACTOR;
}

void MoveMonsterClient(CHARACTER* c, OBJECT* o)
{
    if (c == Hero) return;

    if (c->Dead == 0)
    {
        if (c->MonsterIndex == MONSTER_ILLUSION_ITEM_STORAGE || c->MonsterIndex == MONSTER_ALLIANCE_ITEM_STORAGE)
        {
            c->Movement = false;
        }

        if (!c->Movement)
        {
            if (c->Appear == 0 && o->Type != MODEL_GHOST && ((c->PositionX) != c->TargetX || (c->PositionY) != c->TargetY))
            {
                int iDefaultWall = TW_CHARACTER;

                if (gMapManager.WorldActive >= WD_65DOPPLEGANGER1 && gMapManager.WorldActive <= WD_68DOPPLEGANGER4)
                {
                    iDefaultWall = TW_NOMOVE;
                }

                if (PathFinding2((c->PositionX), (c->PositionY), c->TargetX, c->TargetY, &c->Path, 0.0f, iDefaultWall))
                {
                    c->Movement = true;
                }
            }
        }
        else
        {
            if (o->Type == MODEL_DARK_SKULL_SOLDIER_5 && (o->CurrentAction == MONSTER01_ATTACK1
                || o->CurrentAction == MONSTER01_ATTACK2
                || o->CurrentAction == MONSTER01_ATTACK3
                || o->CurrentAction == MONSTER01_ATTACK4
                || o->CurrentAction == MONSTER01_ATTACK5))
            {
                c->Movement = true;
                SetAction(o, o->CurrentAction);
            }
            else
            {
                SetPlayerWalk(c);
            }

            if (MovePath(c))
            {
                c->Movement = false;
                SetPlayerStop(c);
                c->Object.Angle[2] = ((float)(c->TargetAngle) - 1.f) * 45.f;
            }

            MoveCharacterPosition(c);
        }
    }
    else
    {
        if (o->Type == MODEL_BUDGE_DRAGON)
        {
            o->Position[2] = RequestTerrainHeight(o->Position[0], o->Position[1]);
        }
    }
}

void MoveCharacterClient(CHARACTER* cc)
{
    OBJECT* co = &cc->Object;
    if (co->Live)
    {
        co->Visible = TestFrustrum2D(co->Position[0] * 0.01f, co->Position[1] * 0.01f, -20.f);

        MoveMonsterClient(cc, co);
        MoveCharacter(cc, co);
        MoveCharacterVisual(cc, co);

        battleCastle::MoveBattleCastleMonster(cc, co);
    }
}

void MoveCharactersClient()
{
    for (int i = 0; i < TERRAIN_SIZE * TERRAIN_SIZE; i++)
    {
        if ((TerrainWall[i] & TW_CHARACTER) == TW_CHARACTER) TerrainWall[i] -= TW_CHARACTER;
    }

    for (int i = 0; i < MAX_CHARACTERS_CLIENT; i++)
    {
        CHARACTER* tc = &CharactersClient[i];
        OBJECT* to = &tc->Object;
        if (to->Live && tc->Dead == 0 && to->Kind != KIND_TRAP)
        {
            int Index = TERRAIN_INDEX_REPEAT((tc->PositionX), (tc->PositionY));
            TerrainWall[Index] |= TW_CHARACTER;
        }

        to->Visible = TestFrustrum2D(to->Position[0] * 0.01f, to->Position[1] * 0.01f, -20.f);
    }

    for (int i = 0; i < MAX_CHARACTERS_CLIENT; i++)
    {
        MoveCharacterClient(&CharactersClient[i]);
    }
    MoveBlurs();
}

extern float  ParentMatrix[3][4];

void RenderGuild(OBJECT* o, int Type, vec3_t vPos)
{
    EnableAlphaTest();
    EnableCullFace();
    glColor3f(1.f, 1.f, 1.f);
    BindTexture(BITMAP_GUILD);
    glPushMatrix();

    float Matrix[3][4];
    vec3_t Angle;
    VectorCopy(o->Angle, Angle);
    Angle[2] += 90.f + 45.f;
    Angle[1] += 45.f;
    Angle[0] += 80.f;
    AngleMatrix(Angle, Matrix);
    Matrix[0][3] = 20.f;
    Matrix[1][3] = -5.f;
    if (Type == MODEL_THUNDER_HAWK_ARMOR && Type != -1)
    {
        Matrix[2][3] = -18.f;//-5
    }
    else
    {
        Matrix[2][3] = -10.f;//-5
    }

    if (vPos != NULL)
    {
        Matrix[0][3] = vPos[0];
        Matrix[1][3] = vPos[1];
        Matrix[2][3] = vPos[2];
    }

    R_ConcatTransforms(o->BoneTransform[26], Matrix, ParentMatrix);
    glTranslatef(o->Position[0], o->Position[1], o->Position[2]);
    RenderPlane3D(5.f, 7.f, ParentMatrix);

    glPopMatrix();
    DisableCullFace();
}

void RenderBrightEffect(BMD* b, int Bitmap, int Link, float Scale, vec3_t Light, OBJECT* o)
{
    vec3_t p, Position;
    Vector(0.f, 0.f, 0.f, p);
    b->TransformPosition(BoneTransform[Link], p, Position, true);
    CreateSprite(Bitmap, Position, Scale, Light, o);
}

OBJECT g_ItemObject[ITEM_ETC + MAX_ITEM_INDEX];

void RenderLinkObject(float x, float y, float z, CHARACTER* c, PART_t* f, int Type, int Level, int Option1, bool Link, bool Translate, int RenderType, bool bRightHandItem)
{
    OBJECT* o = &c->Object;
    BMD* b = &Models[Type];

    if (o->SubType == MODEL_CURSEDTEMPLE_ALLIED_PLAYER || o->SubType == MODEL_CURSEDTEMPLE_ILLUSION_PLAYER)
    {
        if (Type >= MODEL_WING && Type <= MODEL_WINGS_OF_DARKNESS) return; // 1st and 2nd Wings
        else if (Type >= MODEL_WING_OF_STORM && Type <= MODEL_WING_OF_DIMENSION) // 3rd Wings
            return;
        else if (MODEL_WING + 130 >= Type && Type <= MODEL_WING + 135) return; // Small Wings and Capes
        else if (Type >= MODEL_CAPE_OF_FIGHTER && Type <= MODEL_CAPE_OF_OVERRULE) return; // Capes
    }

    if (Type >= MODEL_BOOK_OF_SAHAMUTT && Type <= MODEL_STAFF + 29)
    {
        return;
    }

    if (g_isCharacterBuff(o, eBuff_Cloaking))
        return;

    //CopyShadowAngle(f,b);
    b->ContrastEnable = o->ContrastEnable;
    b->BodyScale = o->Scale;
    b->CurrentAction = f->CurrentAction;
    b->BodyHeight = 0.f;

    OBJECT* Object = &g_ItemObject[Type];

    Object->Type = Type;
    ItemObjectAttribute(Object);
    b->LightEnable = Object->LightEnable;
    b->LightEnable = false;

    g_CharacterCopyBuff(Object, o);

    switch (Type)
    {
    case MODEL_WING_OF_RUIN:
        f->PlaySpeed = 0.15f;
        break;
    }

    if (c->MonsterIndex >= MONSTER_TERRIBLE_BUTCHER && c->MonsterIndex <= MONSTER_DOPPELGANGER_SUM)
    {
        if (gMapManager.WorldActive == WD_65DOPPLEGANGER1)
            RenderType = RENDER_DOPPELGANGER | RENDER_TEXTURE;
        else
            RenderType = RENDER_DOPPELGANGER | RENDER_BRIGHT | RENDER_TEXTURE;
    }

    OBB_t OBB;
    vec3_t p, Position;

    if (Link)
    {
        vec3_t Angle;
        float Matrix[3][4];

        if (c->MonsterIndex >= MONSTER_STATUE_OF_SAINT_1 && c->MonsterIndex <= MONSTER_STATUE_OF_SAINT_3)
        {
            if (Type == MODEL_DIVINE_STAFF_OF_ARCHANGEL || Type == MODEL_DIVINE_SWORD_OF_ARCHANGEL)
            {
                Vector(90.f, 0.f, 90.f, Angle);
                AngleMatrix(Angle, Matrix);
                Matrix[0][3] = 0.f;
                Matrix[1][3] = 80.f;
                Matrix[2][3] = 120.f;
            }
            else if (Type == MODEL_DIVINE_CB_OF_ARCHANGEL)
            {
                Vector(10.f, 0.f, 0.f, Angle);
                AngleMatrix(Angle, Matrix);
                Matrix[0][3] = 0.f;
                Matrix[1][3] = 110.f;
                Matrix[2][3] = 80.f;
            }
        }
        else if (Type == MODEL_SWORD_35_WING)
        {
            Vector(x, y, z, Angle);
            AngleMatrix(Angle, Matrix);
            Matrix[0][3] = 15.f;
            Matrix[1][3] = -5.f;
            Matrix[2][3] = 0.f;
        }
        else if (Type == MODEL_CAPE_OF_OVERRULE)
        {
            Vector(0.f, 90.f, 0.f, Angle);
            AngleMatrix(Angle, Matrix);
            Matrix[0][3] = 10.f;
            Matrix[1][3] = -15.f;
            Matrix[2][3] = 0.f;
        }
        else if (Type >= MODEL_CAPE_OF_EMPEROR)
        {
            Vector(0.f, 90.f, 0.f, Angle);
            AngleMatrix(Angle, Matrix);
            Matrix[0][3] = -47.f;
            Matrix[1][3] = -7.f;
            Matrix[2][3] = 0.f;
        }

        else if ((Type >= MODEL_CROSSBOW && Type < MODEL_ARROWS) || (Type >= MODEL_SAINT_CROSSBOW && Type < MODEL_CELESTIAL_BOW) ||
            (Type >= MODEL_DIVINE_CB_OF_ARCHANGEL && Type < MODEL_ARROW_VIPER_BOW))
        {
            Vector(0.f, 20.f, 180.f, Angle);
            AngleMatrix(Angle, Matrix);
            Matrix[0][3] = -10.f;
            Matrix[1][3] = 8.f;
            Matrix[2][3] = 40.f;
        }
        else if (Type == MODEL_15GRADE_ARMOR_OBJ_ARMLEFT || Type == MODEL_15GRADE_ARMOR_OBJ_ARMRIGHT ||
            Type == MODEL_15GRADE_ARMOR_OBJ_BODYLEFT || Type == MODEL_15GRADE_ARMOR_OBJ_BODYRIGHT ||
            Type == MODEL_15GRADE_ARMOR_OBJ_BOOTLEFT || Type == MODEL_15GRADE_ARMOR_OBJ_BOOTRIGHT ||
            Type == MODEL_15GRADE_ARMOR_OBJ_HEAD || Type == MODEL_15GRADE_ARMOR_OBJ_PANTLEFT ||
            Type == MODEL_15GRADE_ARMOR_OBJ_PANTRIGHT)
        {
            switch (Type)
            {
            case MODEL_15GRADE_ARMOR_OBJ_ARMLEFT:
            {
                Vector(0.f, -90.f, 0.f, Angle);
                AngleMatrix(Angle, Matrix);
                Matrix[0][3] = 30.f;
                Matrix[1][3] = 0.f;
                Matrix[2][3] = 20.f;
            }break;
            case MODEL_15GRADE_ARMOR_OBJ_ARMRIGHT:
            {
                Vector(0.f, -90.f, 0.f, Angle);
                AngleMatrix(Angle, Matrix);
                Matrix[0][3] = 30.f;
                Matrix[1][3] = 0.f;
                Matrix[2][3] = -20.f;
            }break;
            case MODEL_15GRADE_ARMOR_OBJ_BODYLEFT:
            {
                Vector(0.f, -90.f, 0.f, Angle);
                AngleMatrix(Angle, Matrix);
                Matrix[0][3] = 5.f;
                Matrix[1][3] = -20.f;
                Matrix[2][3] = 0.f;
            }break;
            case MODEL_15GRADE_ARMOR_OBJ_BODYRIGHT:
            {
                Vector(0.f, -90.f, 0.f, Angle);
                AngleMatrix(Angle, Matrix);
                Matrix[0][3] = 5.f;
                Matrix[1][3] = -20.f;
                Matrix[2][3] = 0.f;
            }break;
            case MODEL_15GRADE_ARMOR_OBJ_BOOTLEFT:
            {
                Vector(0.f, 90.f, 180.f, Angle);
                AngleMatrix(Angle, Matrix);
                Matrix[0][3] = 20.f;
                Matrix[1][3] = 15.f;
                Matrix[2][3] = -10.f;
            }break;
            case MODEL_15GRADE_ARMOR_OBJ_BOOTRIGHT:
            {
                Vector(0.f, 90.f, 180.f, Angle);
                AngleMatrix(Angle, Matrix);
                Matrix[0][3] = 20.f;
                Matrix[1][3] = 15.f;
                Matrix[2][3] = 10.f;
            }break;
            case MODEL_15GRADE_ARMOR_OBJ_HEAD:
            {
                Vector(180.f, -90.f, 0.f, Angle); //y,x,z
                AngleMatrix(Angle, Matrix);
                Matrix[0][3] = 28.f; //y
                Matrix[1][3] = 20.f; //x
                Matrix[2][3] = 0.f;
            }break;
            case MODEL_15GRADE_ARMOR_OBJ_PANTLEFT:
            {
                Vector(0.f, 90.f, 180.f, Angle);
                AngleMatrix(Angle, Matrix);
                Matrix[0][3] = 25.f;
                Matrix[1][3] = 5.f;
                Matrix[2][3] = -5.f;
            }break;
            case MODEL_15GRADE_ARMOR_OBJ_PANTRIGHT:
            {
                Vector(0.f, 90.f, 180.f, Angle);
                AngleMatrix(Angle, Matrix);
                Matrix[0][3] = 25.f;
                Matrix[1][3] = 5.f;
                Matrix[2][3] = 5.f;
            }break;
            }
        }
        else
        {
            if (Type == MODEL_DRAGON_SOUL_STAFF)
            {
                Vector(90.f + 20.f, 180.f, 90.f, Angle);
                AngleMatrix(Angle, Matrix);
            }
            else
            {
                Vector(90.f - 20.f, 0.f, 90.f, Angle);
                AngleMatrix(Angle, Matrix);
            }

            if (Type == MODEL_ARROW_VIPER_BOW)
            {
                Vector(-60.f, 0.f, -80.f, Angle);
                AngleMatrix(Angle, Matrix);
                Matrix[0][3] = -5.f;
                Matrix[1][3] = 20.f;
                Matrix[2][3] = 0.f;
            }
            else
                if (Type == MODEL_STINGER_BOW)
                {
                    Vector(-60.f, 0.f, -80.f, Angle);
                    AngleMatrix(Angle, Matrix);
                    Matrix[0][3] = -5.f;
                    Matrix[1][3] = 20.f;
                    Matrix[2][3] = -5.f;
                }
                else if (Type == MODEL_AIR_LYN_BOW)
                {
                    Vector(90.f, 0.f, -80.f, Angle);
                    AngleMatrix(Angle, Matrix);
                    Matrix[0][3] = 10.f;
                    Matrix[1][3] = 20.f;
                    Matrix[2][3] = -5.f;
                }
                else if (Type >= MODEL_BOW && Type < MODEL_BOW + MAX_ITEM_INDEX)
                {
                    Matrix[0][3] = -10.f;
                    Matrix[1][3] = 5.f;
                    Matrix[2][3] = 10.f;
                }
                else if (Type == MODEL_DRAGON_SOUL_STAFF)
                {
                    Matrix[0][3] = -10.f;
                    Matrix[1][3] = 5.f;
                    Matrix[2][3] = -10.f;
                }
                else if (Type == MODEL_STAFF_OF_DESTRUCTION)
                {
                    Matrix[0][3] = -10.f;
                    Matrix[1][3] = 5.f;
                    Matrix[2][3] = 10.f;
                }
                else if (Type >= MODEL_SHIELD && Type < MODEL_SHIELD + MAX_ITEM_INDEX)
                {
                    if (Type == MODEL_ELEMENTAL_SHIELD)
                    {
                        Vector(30.f, 0.f, 90.f, Angle);
                        AngleMatrix(Angle, Matrix);
                        Matrix[0][3] = -20.f;
                        Matrix[1][3] = 0.f;
                        Matrix[2][3] = -20.f;
                    }
                    else if (Type == MODEL_LEGENDARY_SHIELD || Type == MODEL_GRAND_SOUL_SHIELD)
                    {
                        Vector(50.f, 0.f, 90.f, Angle);
                        AngleMatrix(Angle, Matrix);
                        Matrix[0][3] = -28.f;
                        Matrix[1][3] = 0.f;
                        Matrix[2][3] = -25.f;
                    }
                    else if (Type == MODEL_SKULL_SHIELD)
                    {
                        Vector(30.f, 0.f, 90.f, Angle);
                        AngleMatrix(Angle, Matrix);
                        Matrix[0][3] = -15.f;
                        Matrix[1][3] = 0.f;
                        Matrix[2][3] = -25.f;
                    }
                    else
                    {
                        Matrix[0][3] = -10.f;
                        Matrix[1][3] = 0.f;
                        Matrix[2][3] = 0.f;
                    }
                }
                else
                {
                    Matrix[0][3] = -20.f;
                    Matrix[1][3] = 5.f;
                    Matrix[2][3] = 40.f;
                }
        }

        if (g_CMonkSystem.IsRagefighterCommonWeapon(c->Class, Type))
        {
            Matrix[1][3] += 10.0f;
            Matrix[2][3] += 25.0f;
            b->BodyScale = 0.9f;
        }

        if (bRightHandItem == false && !(Type >= MODEL_SHIELD && Type < MODEL_SHIELD + MAX_ITEM_INDEX) && Type != MODEL_ARROW_VIPER_BOW)
        {
            vec3_t vNewAngle;
            float mNewRot[3][4];
            float _Angle = 275.0f;
            if (g_CMonkSystem.IsRagefighterCommonWeapon(c->Class, Type))
            {
                _Angle = 265.0f;
                Matrix[1][3] += 7.0f;
            }
            Vector(145.f, 0.f, _Angle, vNewAngle);
            AngleMatrix(vNewAngle, mNewRot);
            mNewRot[0][3] = 0.f;
            mNewRot[1][3] = 10.f;
            mNewRot[2][3] = -30.f;

            R_ConcatTransforms(Matrix, mNewRot, Matrix);
        }

        R_ConcatTransforms(o->BoneTransform[f->LinkBone], Matrix, ParentMatrix);
        VectorCopy(c->Object.Position, b->BodyOrigin);
        Vector(0.f, 0.f, 0.f, Object->Angle);
    }
    else
    {
        Vector(x, y, z, p);
        BMD* Owner = &Models[o->Type];
        Owner->RotationPosition(o->BoneTransform[f->LinkBone], p, Position);
        VectorAdd(c->Object.Position, Position, b->BodyOrigin);
        Vector(0.f, 0.f, 0.f, Object->Angle);
    }
    if (Type == MODEL_BOSS_HEAD)
    {
        VectorAdd(b->BodyOrigin, BossHeadPosition, b->BodyOrigin);
        b->BoneHead = 0;
        Vector(0.f, 0.f, WorldTime, Object->Angle);
    }

    if ((c->Skill == AT_SKILL_PENETRATION || c->Skill == AT_SKILL_PENETRATION_STR) &&
        ((o->Type == MODEL_PLAYER && o->CurrentAction >= PLAYER_ATTACK_FIST && o->CurrentAction <= PLAYER_RIDE_SKILL)))
    {
        if (o->AnimationFrame >= 5.f && o->AnimationFrame <= 10.f)
        {
            f->PriorAnimationFrame = 2.f;
            f->AnimationFrame = 3.2f;
        }
    }
    if ((f->Type >= MODEL_WINGS_OF_DARKNESS && f->Type <= MODEL_WINGS_OF_DARKNESS) && c->SafeZone)
    {
        b->CurrentAction = 1;
    }
    if (!Link || (Type < MODEL_BOW || Type >= MODEL_BOW + MAX_ITEM_INDEX) || Type == MODEL_STINGER_BOW)
    {
        if (!g_isCharacterBuff(o, eDeBuff_Stun) && !g_isCharacterBuff(o, eDeBuff_Sleep))
        {
            b->PlayAnimation(&f->AnimationFrame, &f->PriorAnimationFrame, &f->PriorAction, f->PlaySpeed, Position, Object->Angle);
        }
    }

    VectorCopy(b->BodyOrigin, Object->Position);

    vec3_t Temp;
    b->Animation(BoneTransform, f->AnimationFrame, f->PriorAnimationFrame, f->PriorAction, Object->Angle, Object->Angle, true);
    if (g_CMonkSystem.IsRagefighterCommonWeapon(c->Class, Type) && !Link)
    {
        float _KnightScale = 0.9f;
        if (Type == MODEL_FLAIL)
            b->InterpolationTrans(BoneTransform[0], BoneTransform[2], _KnightScale);
        b->Transform(BoneTransform, Temp, Temp, &OBB, Translate, _KnightScale);
    }
    else
        b->Transform(BoneTransform, Temp, Temp, &OBB, Translate);

    if (Type != MODEL_CAPE_OF_LORD // Cape of Lord
        && Type != MODEL_CAPE_OF_FIGHTER // Cape of Fighter
        && Type != MODEL_WING + 130 // Small Cape of Lord
        && Type != MODEL_WING + 135 // Small Cape of Fighter
        )
    {
        RenderPartObjectEffect(Object, Type, c->Light, o->Alpha, Level, Option1, false, 0, RenderType | ((c->MonsterIndex == MONSTER_METAL_BALROG || c->MonsterIndex == MONSTER_ORC_ARCHER_OF_DOOM) ? (RENDER_EXTRA | RENDER_TEXTURE) : RENDER_TEXTURE));
    }

    float Luminosity;
    vec3_t Light;
    Luminosity = (float)(rand() % 30 + 70) * 0.005f;
    switch (Type)
    {
    case MODEL_TIGER_BOW:
    case MODEL_SILVER_BOW:
    case MODEL_CHAOS_NATURE_BOW:
    case MODEL_CELESTIAL_BOW:
        if (Type == MODEL_CHAOS_NATURE_BOW)
        {
            Vector(Luminosity * 0.6f, Luminosity * 1.f, Luminosity * 0.8f, Light);

            for (int i = 13; i <= 18; i++)
                RenderBrightEffect(b, BITMAP_SHINY + 1, i, 1.f, Light, o);
        }
        else if (Type == MODEL_CELESTIAL_BOW)
        {
            Vector(Luminosity * 0.5f, Luminosity * 0.5f, Luminosity * 0.8f, Light);

            for (int i = 13; i <= 18; i++)
                RenderBrightEffect(b, BITMAP_SHINY + 1, i, 1.f, Light, o);

            for (int i = 5; i <= 8; i++)
                RenderBrightEffect(b, BITMAP_SHINY + 1, i, 1.f, Light, o);
        }
        else
        {
            Vector(Luminosity * 1.f, Luminosity * 0.6f, Luminosity * 0.2f, Light);
            RenderBrightEffect(b, BITMAP_SHINY + 1, 2, 1.f, Light, o);
            RenderBrightEffect(b, BITMAP_SHINY + 1, 6, 1.f, Light, o);
        }
        break;
    case MODEL_DIVINE_STAFF_OF_ARCHANGEL:
        Vector(Luminosity * 1.f, Luminosity * 0.3f, Luminosity * 0.1f, Light);

        for (int i = 0; i < 10; ++i)
        {
            vec3_t Light2;
            Vector(0.4f, 0.4f, 0.4f, Light2);
            Vector(i * 30.f - 180.f, -40.f, 0.f, p);
            b->TransformPosition(BoneTransform[0], p, Position, true);

            if (rand_fps_check(3))
            {
                CreateSprite(BITMAP_SHINY + 1, Position, 0.6f, Light2, o, (float)(rand() % 360));
            }
            CreateSprite(BITMAP_LIGHT, Position, 2.f, Light, o);
        }
        break;
    case MODEL_SYLPH_WIND_BOW:
    {
        Vector(0.8f, 0.8f, 0.2f, Light);
        Vector(0.f, 0.f, 0.f, p);
        if (rand_fps_check(2))
        {
            b->TransformPosition(BoneTransform[4], p, Position, true);
            CreateParticle(BITMAP_SPARK + 1, Position, o->Angle, Light, 11, 0.8f);

            b->TransformPosition(BoneTransform[12], p, Position, true);
            CreateParticle(BITMAP_SPARK + 1, Position, o->Angle, Light, 11, 0.8f);
        }
        Vector(0.5f, 0.5f, 0.1f, Light);
        b->TransformPosition(BoneTransform[7], p, Position, true);
        CreateSprite(BITMAP_LIGHT, Position, 2.0f, Light, o);

        b->TransformPosition(BoneTransform[15], p, Position, true);
        CreateSprite(BITMAP_LIGHT, Position, 2.0f, Light, o);

        Vector(20.f, 0.f, 0.f, p);
        Vector(1.0f, 1.0f, 0.4f, Light);
        b->TransformPosition(BoneTransform[0], p, Position, true);
        CreateSprite(BITMAP_LIGHT, Position, 0.8f, Light, o);
        CreateSprite(BITMAP_LIGHT, Position, 1.3f, Light, o);
        CreateSprite(BITMAP_LIGHT, Position, 2.0f, Light, o);
    }
    break;
    case MODEL_SOLEIL_SCEPTER:
    {
        Vector(1.0f, 0.3f, 0.0f, Light);

        float fRendomPos = (float)(rand() % 60) / 20.0f - 1.5f;
        float fRendomScale = (float)(rand() % 10) / 20.0f + 1.4f;
        Vector(0.f, -100.f + fRendomPos, fRendomPos, p);
        b->TransformPosition(BoneTransform[0], p, Position, true);
        CreateSprite(BITMAP_SHINY + 1, Position, fRendomScale, Light, o);
        CreateSprite(BITMAP_SHINY + 1, Position, fRendomScale - 0.3f, Light, o, 20.0f);

        fRendomPos = (float)(rand() % 60) / 20.0f - 1.5f;
        fRendomScale = (float)(rand() % 10) / 20.0f + 1.0f;
        Vector(0.f, -100.f + fRendomPos, fRendomPos, p);
        b->TransformPosition(BoneTransform[0], p, Position, true);
        CreateSprite(BITMAP_LIGHT, Position, fRendomScale + 0.3f, Light, o);

        fRendomPos = (float)(rand() % 40) / 20.0f - 1.0f;
        fRendomScale = (float)(rand() % 8) / 20.0f + 0.4f;
        Vector(0.f, 100.f + fRendomPos, fRendomPos, p);
        b->TransformPosition(BoneTransform[0], p, Position, true);
        CreateSprite(BITMAP_SHINY + 1, Position, fRendomScale, Light, o);
        CreateSprite(BITMAP_SHINY + 1, Position, fRendomScale - 0.2f, Light, o, 90.0f);
    }
    break;
    case MODEL_BONE_BLADE:
    {
        float fLight = (float)sinf((WorldTime) * 0.4f) * 0.25f + 0.7f;
        Vector(fLight, fLight - 0.5f, fLight - 0.5f, Light);

        Vector(5.f, -22.f, -10.f, p);
        b->TransformPosition(BoneTransform[0], p, Position, true);
        CreateSprite(BITMAP_LIGHT + 1, Position, 0.75f, Light, o);

        Vector(-5.f, -22.f, -10.f, p);
        b->TransformPosition(BoneTransform[0], p, Position, true);
        CreateSprite(BITMAP_LIGHT + 1, Position, 0.75f, Light, o);
    }
    break;
    case MODEL_EXPLOSION_BLADE:
    {
        float fRendomPos = (float)(rand() % 60) / 20.0f - 1.5f;
        float fRendomScale = (float)(rand() % 30) / 20.0f + 1.5f;
        float fLight = (float)sinf((WorldTime) * 0.7f) * 0.2f + 0.5f;

        float fRotation = (WorldTime * 0.0006f) * 360.0f;
        float fRotation2 = (WorldTime * 0.0006f) * 360.0f;

        Vector(0.2f, 0.2f, fLight, Light);

        Vector(0.f, fRendomPos, fRendomPos, p);
        b->TransformPosition(BoneTransform[4], p, Position, true);
        CreateSprite(BITMAP_SHINY + 1, Position, fRendomScale, Light, o, fRotation);
        CreateSprite(BITMAP_SHINY + 1, Position, fRendomScale - 0.4f, Light, o, 90.f + fRotation2);
        Vector(0.0f, 0.0f, 0.0f, p);
        CreateSprite(BITMAP_LIGHT, Position, 2.3f, Light, o);

        Vector(30.f, 0.f, 0.f, p);
        b->TransformPosition(BoneTransform[4], p, Position, true);
        CreateSprite(BITMAP_LIGHT, Position, 2.0f, Light, o);

        Vector(0.f, 0.f, 0.f, p);
        b->TransformPosition(BoneTransform[6], p, Position, true);
        CreateSprite(BITMAP_LIGHT, Position, 2.0f, Light, o);

        b->TransformPosition(BoneTransform[7], p, Position, true);
        CreateSprite(BITMAP_LIGHT, Position, 2.0f, Light, o);

        b->TransformPosition(BoneTransform[8], p, Position, true);
        CreateSprite(BITMAP_LIGHT, Position, 1.0f, Light, o);
    }
    break;
    case MODEL_GRAND_VIPER_STAFF:
    {
        Vector(0.4f, 0.4f, 0.4f, Light);
        float fRendomPos = (float)(rand() % 60) / 20.0f - 1.5f;
        float fRendomScale = (float)(rand() % 15) / 20.0f + 1.8f;
        Vector(0.f, -170.f + fRendomPos, 0.f + fRendomPos, p);
        b->TransformPosition(BoneTransform[0], p, Position, true);
        CreateSprite(BITMAP_SPARK + 1, Position, fRendomScale, Light, o);

        VectorCopy(Position, o->EyeLeft);

        if (rand_fps_check(1))
        {
            CreateJoint(BITMAP_JOINT_ENERGY, Position, Position, o->Angle, 17, o, 30.f);
        }

        fRendomPos = (float)(rand() % 60) / 20.0f - 1.5f;
        fRendomScale = (float)(rand() % 15) / 20.0f + 1.0f;
        Vector(0.f, -170.f + fRendomPos, 0.f + fRendomPos, p);
        Vector(1.0f, 0.4f, 1.0f, Light);
        CreateSprite(BITMAP_LIGHT, Position, fRendomScale, Light, o);
        CreateSprite(BITMAP_SHINY + 1, Position, fRendomScale, Light, o);
        CreateSprite(BITMAP_SHINY + 1, Position, fRendomScale - 0.3f, Light, o, 90.0f);
        if (rand_fps_check(1))
        {
            CreateParticle(BITMAP_SPARK + 1, Position, o->Angle, Light, 11, 2.0f);
        }

        float fLight = (float)sinf((WorldTime) * 0.7f) * 0.2f + 0.5f;
        float fRotation = (WorldTime * 0.0006f) * 360.0f;

        Vector(fLight - 0.1f, 0.1f, fLight - 0.1f, Light);
        Vector(0.f, 10.0f, 0.0f, p);
        b->TransformPosition(BoneTransform[2], p, Position, true);
        CreateSprite(BITMAP_SHINY + 1, Position, 1.5f, Light, o, fRotation);
        CreateSprite(BITMAP_SHINY + 1, Position, 1.2f, Light, o, 90.0f + fRotation);

        Vector(-40.f, -10.0f, 0.0f, p);
        b->TransformPosition(BoneTransform[1], p, Position, true);
        CreateSprite(BITMAP_SHINY + 1, Position, 1.0f, Light, o, fRotation);
        CreateSprite(BITMAP_SHINY + 1, Position, 0.7f, Light, o, 90.0f + fRotation);

        Vector(-160.f, -5.0f, 0.0f, p);
        b->TransformPosition(BoneTransform[1], p, Position, true);
        CreateSprite(BITMAP_SHINY + 1, Position, 1.2f, Light, o, fRotation);
        CreateSprite(BITMAP_SHINY + 1, Position, 1.0f, Light, o, 90.0f + fRotation);
    }
    break;
    case MODEL_DAYBREAK:
    {
        // Light
        Vector(0.6f, 0.6f, 0.6f, Light);

        Vector(0.f, 0.f, 3.f, p);
        b->TransformPosition(BoneTransform[2], p, Position, true);
        CreateSprite(BITMAP_LIGHT + 1, Position, 0.45f, Light, o);

        Vector(0.f, 0.f, -3.f, p);
        b->TransformPosition(BoneTransform[2], p, Position, true);
        CreateSprite(BITMAP_LIGHT + 1, Position, 0.45f, Light, o);

        Vector(0.f, 0.f, 0.f, p);
        b->TransformPosition(BoneTransform[2], p, Position, true);
        CreateSprite(BITMAP_SHINY + 1, Position, 0.7f, Light, o);

        float fScale = 0.5f + (float)(rand() % 100) / 180;
        float fRotation = (WorldTime * 0.0006f) * 360.0f;
        Vector(0.f, 0.f, 0.f, p);
        b->TransformPosition(BoneTransform[2], p, Position, true);
        CreateSprite(BITMAP_SHINY + 1, Position, fScale, Light, o, fRotation);

        // Flare01.jpg
        float fLight = (float)sinf((WorldTime) * 0.4f) * 0.25f + 0.6f;
        Vector(fLight, fLight, fLight, Light);
        Vector(0.f, 0.f, 0.f, p);
        b->TransformPosition(BoneTransform[2], p, Position, true);
        CreateSprite(BITMAP_LIGHT, Position, 3.f, Light, o, 0.f);
    }
    break;
    case MODEL_SWORD_DANCER:
    {
        float fLight, fScale, fRotation;
        static float fPosition = 0.0f;
        static int iRandom;

        Vector(1.0f, 0.1f, 0.0f, Light);
        Vector(0.f, 0.f, 0.f, p);
        b->TransformPosition(BoneTransform[1], p, Position, true);
        CreateSprite(BITMAP_LIGHT, Position, 1.0f, Light, o);	// flare01.jpg

        fScale = (float)(rand() % 30) / 60.0f + 0.2f;
        fLight = (float)sinf((WorldTime) * 0.7f) * 0.2f + 0.3f;
        fRotation = (WorldTime * 0.0006f) * 360.0f;
        Vector(0.1f + fLight, 0.2f, 0.0f, Light);
        CreateSprite(BITMAP_LIGHT + 3, Position, fScale, Light, o, fRotation);	// impact01.jpg

        Vector(1.0f - fLight, 0.0f, 0.0f, Light);
        if (fPosition >= 20.0f)
        {
            iRandom = rand() % 5 + 2;	// 2 ~ 6
            fPosition = 0.0f;
            //Vector(1.0f, 0.0f, 0.0f, Light);
        }
        else
            fPosition += 1.5f;
        Vector(0.f, fPosition, 0.f, p);
        b->TransformPosition(BoneTransform[iRandom], p, Position, true);
        CreateSprite(BITMAP_WATERFALL_4, Position, 0.7f, Light, o);
        Vector(0.1f, 0.1f, 0.1f, Light);
        CreateSprite(BITMAP_WATERFALL_2, Position, 0.3f, Light, o);

        // Flare01
        fLight = (float)sinf((WorldTime) * 0.4f) * 0.25f + 0.2f;
        Vector(0.8f + fLight, 0.1f, 0.f, Light);
        Vector(0.f, 0.f, 0.f, p);
        for (int i = 0; i < 5; ++i)
        {
            b->TransformPosition(BoneTransform[2 + i], p, Position, true);
            CreateSprite(BITMAP_LIGHT, Position, 1.1f, Light, o);
        }

        if (o->CurrentAction == PLAYER_RUN_TWO_HAND_SWORD_TWO && gMapManager.WorldActive != WD_10HEAVEN && rand_fps_check(2))
        {
            if (!g_Direction.m_CKanturu.IsMayaScene())
            {
                Vector(1.f, 1.f, 1.f, Light);
                Vector(0.f, 0.f, 0.f, p);
                b->TransformPosition(BoneTransform[6], p, Position, true);
                Position[0] += rand() % 30 - 15.f;
                Position[1] += rand() % 30 - 15.f;
                Position[2] += 20.f;

                vec3_t	Angle;
                for (int i = 0; i < 4; i++)
                {
                    Vector((float)(rand() % 60 + 60 + 90), 0.f, o->Angle[2], Angle);//(float)(rand()%30),Angle);
                    CreateJoint(BITMAP_JOINT_SPARK, Position, Position, Angle);
                    CreateParticle(BITMAP_SPARK, Position, Angle, Light);
                }
            }
        }
    }
    break;
    case MODEL_SHINING_SCEPTER:
    {
        float fScale;
        Luminosity = (float)sinf((WorldTime) * 0.002f) * 0.3f + 0.1f;
        fScale = (float)(rand() % 10) / 30.0f + 1.7f;

        Vector(0.f, 0.f, 0.f, p);

        Vector(0.8f + Luminosity, 0.5f + Luminosity, 0.1f + Luminosity, Light);
        b->TransformPosition(BoneTransform[1], p, Position, true);
        CreateSprite(BITMAP_SHINY + 1, Position, fScale, Light, o);

        Vector(0.7f, 0.5f, 0.3f, Light);
        b->TransformPosition(BoneTransform[1], p, Position, true);
        CreateSprite(BITMAP_LIGHT, Position, 3.f, Light, o);

        Vector(0.8f, 0.6f, 0.4f, Light);
        b->TransformPosition(BoneTransform[2], p, Position, true);
        CreateSprite(BITMAP_LIGHT, Position, 1.5f, Light, o);

        Vector(0.8f, 0.6f, 0.4f, Light);
        b->TransformPosition(BoneTransform[3], p, Position, true);
        CreateSprite(BITMAP_LIGHT, Position, 1.2f, Light, o);
    }
    break;
    case MODEL_ALBATROSS_BOW:
    {
        float fLight;
        fLight = (float)sinf((WorldTime) * 0.4f) * 0.25f;
        Vector(0.0f, 0.7f, 0.0f, Light);
        Vector(0.f, 0.f, 0.f, p);
        b->TransformPosition(BoneTransform[10], p, Position, true);
        CreateSprite(BITMAP_LIGHT, Position, 1.2f, Light, o);

        b->TransformPosition(BoneTransform[28], p, Position, true);
        CreateSprite(BITMAP_LIGHT, Position, 1.2f, Light, o);

        b->TransformPosition(BoneTransform[34], p, Position, true);
        CreateSprite(BITMAP_LIGHT, Position, 1.2f, Light, o);

        b->TransformPosition(BoneTransform[16], p, Position, true);
        CreateSprite(BITMAP_LIGHT, Position, 1.2f, Light, o);

        Vector(0.f, 0.f, 0.f, p);
        Vector(0.3f, 0.9f, 0.2f, Light);
        if (rand_fps_check(2))
        {
            b->TransformPosition(BoneTransform[10], p, Position, true);
            CreateParticle(BITMAP_SPARK + 1, Position, o->Angle, Light, 14, 0.05f);
            b->TransformPosition(BoneTransform[28], p, Position, true);
            CreateParticle(BITMAP_SPARK + 1, Position, o->Angle, Light, 14, 0.05f);
            b->TransformPosition(BoneTransform[34], p, Position, true);
            CreateParticle(BITMAP_SPARK + 1, Position, o->Angle, Light, 14, 0.05f);
            b->TransformPosition(BoneTransform[16], p, Position, true);
            CreateParticle(BITMAP_SPARK + 1, Position, o->Angle, Light, 14, 0.05f);
        }
    }
    break;
    case MODEL_PLATINA_STAFF:
    {
        float fLight, fScale, fRotation;
        fLight = (float)sinf((WorldTime) * 0.7f) * 0.2f + 0.3f;
        Vector(0.f, 0.f, 0.f, p);

        // flare01
        Vector(0.7f, 0.7f, 0.7f, Light);
        fScale = (float)(rand() % 10) / 500.0f;
        b->TransformPosition(BoneTransform[9], p, Position, true);
        CreateSprite(BITMAP_LIGHT, Position, 0.3f + fScale, Light, o);
        //			Vector(0.1f, 0.5f, 0.1f, Light);
        //			CreateSprite(BITMAP_LIGHT, Position, 1.5f, Light, o);

        Vector(0.2f, 0.6f + fLight, 0.2f, Light);
        fScale = (float)(rand() % 10) / 500.0f;
        b->TransformPosition(BoneTransform[9], p, Position, true);
        CreateSprite(BITMAP_LIGHT, Position, 4.0f + fScale, Light, o);

        // shiny02
        Vector(0.4f, 0.5f + fLight, 0.4f, Light);
        fScale = (float)(rand() % 30) / 60.0f;
        fRotation = (WorldTime * 0.0004f) * 360.0f;
        b->TransformPosition(BoneTransform[9], p, Position, true);
        CreateSprite(BITMAP_SHINY + 1, Position, 1.2f + fScale, Light, o, -fRotation);

        // magic_ground1
        Vector(0.6f, 1.f, 0.6f, Light);
        fScale = (float)(rand() % 10) / 500.0f;
        fRotation = (WorldTime * 0.0006f) * 360.0f;
        b->TransformPosition(BoneTransform[9], p, Position, true);
        CreateSprite(BITMAP_MAGIC, Position, 0.25f + fScale, Light, o, fRotation);

        // 5, 6, 7, 8 flare01 , 10, 11, 12, 13
        Vector(0.f, 0.f, 0.f, p);

        for (int i = 0; i < 4; ++i)
        {
            Vector(0.1f, 0.8f, 0.1f, Light);
            b->TransformPosition(BoneTransform[5 + i], p, Position, true);
            CreateSprite(BITMAP_LIGHT, Position, 0.2f, Light, o);
            Vector(0.5f, 0.5f, 0.5f, Light);
            b->TransformPosition(BoneTransform[10 + i], p, Position, true);
            CreateSprite(BITMAP_LIGHT, Position, 0.3f, Light, o);
        }

        Vector(0.1f, 0.8f, 0.1f, Light);
        b->TransformPosition(BoneTransform[4], p, Position, true);
        CreateSprite(BITMAP_SHINY + 1, Position, 0.6f + fScale, Light, o, fRotation);

        Vector(0.6f, 1.f, 0.6f, Light);
        fScale = (float)(rand() % 10) / 500.0f;
        fRotation = (WorldTime * 0.0006f) * 360.0f;
        b->TransformPosition(BoneTransform[2], p, Position, true);
        CreateSprite(BITMAP_MAGIC, Position, 0.15f + fScale, Light, o, fRotation);
        Vector(0.8f, 0.8f, 0.8f, Light);
        b->TransformPosition(BoneTransform[2], p, Position, true);
        CreateSprite(BITMAP_LIGHT, Position, 0.7f + fScale, Light, o, -fRotation);
        Vector(0.1f, 1.0f, 0.1f, Light);
        b->TransformPosition(BoneTransform[2], p, Position, true);
        CreateSprite(BITMAP_SHINY + 1, Position, 0.5f + fScale, Light, o, fRotation);

        Vector(0.2f, 0.6f + fLight, 0.2f, Light);
        b->TransformPosition(BoneTransform[2], p, Position, true);
        CreateSprite(BITMAP_LIGHT, Position, 2.0f + fScale, Light, o);
    }
    break;
    case MODEL_STAFF_OF_KUNDUN:
    {
        Luminosity = (float)sinf((WorldTime) * 0.002f) * 0.3f + 0.7f;

        Vector(0.f, 0.f, 0.f, p);
        Vector(Luminosity * 0.f, Luminosity * 0.5f, Luminosity * 1.f, Light);

        auto Rotation = (float)(rand() % 360);
        b->TransformPosition(BoneTransform[5], p, Position, true);
        Vector(Luminosity * 0.f, Luminosity * 0.5f, Luminosity * 1.f, Light);
        CreateSprite(BITMAP_LIGHT, Position, 1.f, Light, o);
        Vector(0.5f, 0.5f, 0.5f, Light);
        CreateSprite(BITMAP_SHINY + 1, Position, 0.5f, Light, o, Rotation);
        CreateSprite(BITMAP_SHINY + 1, Position, 0.5f, Light, o, 360.f - Rotation);

        b->TransformPosition(BoneTransform[6], p, Position, true);
        Vector(Luminosity * 0.f, Luminosity * 0.5f, Luminosity * 1.f, Light);
        CreateSprite(BITMAP_LIGHT, Position, 1.f, Light, o);
        Vector(0.5f, 0.5f, 0.5f, Light);
        CreateSprite(BITMAP_SHINY + 1, Position, 0.5f, Light, o, Rotation);
        CreateSprite(BITMAP_SHINY + 1, Position, 0.5f, Light, o, 360.f - Rotation);

        b->TransformPosition(BoneTransform[8], p, Position, true);
        Vector(Luminosity * 0.f, Luminosity * 0.5f, Luminosity * 1.f, Light);
        CreateSprite(BITMAP_LIGHT, Position, 2.f, Light, o);
        Vector(0.5f, 0.5f, 0.5f, Light);
        CreateSprite(BITMAP_SHINY + 1, Position, 1.f, Light, o, Rotation);
        CreateSprite(BITMAP_SHINY + 1, Position, 1.f, Light, o, 360.f - Rotation);
    }
    break;
    case MODEL_GREAT_LORD_SCEPTER:
    {
        auto Rotation = (float)(rand() % 360);
        Luminosity = (float)sinf((WorldTime) * 0.002f) * 0.3f + 0.7f;

        Vector(0.f, 0.f, 0.f, p);

        b->TransformPosition(BoneTransform[1], p, Position, true);
        Vector(Luminosity * 0.f, Luminosity * 0.5f, Luminosity * 1.f, Light);
        CreateSprite(BITMAP_LIGHT, Position, 2.f, Light, o);
        Vector(0.5f, 0.5f, 0.5f, Light);
        CreateSprite(BITMAP_SHINY + 1, Position, 1.f, Light, o, Rotation);
        CreateSprite(BITMAP_SHINY + 1, Position, 1.f, Light, o, 360.f - Rotation);
    }
    break;
    case MODEL_DIVINE_SWORD_OF_ARCHANGEL:
        Vector(Luminosity * 1.f, Luminosity * 0.3f, Luminosity * 0.1f, Light);

        Vector(0.f, 0.f, 0.f, p);

        for (int i = 0; i < 7; ++i)
        {
            vec3_t Light2;
            Vector(0.4f, 0.4f, 0.4f, Light2);
            b->TransformPosition(BoneTransform[i + 2], p, Position, true);

            if (rand_fps_check(3))
            {
                CreateSprite(BITMAP_SHINY + 1, Position, 0.6f, Light2, o, (float)(rand() % 360));
            }
            CreateSprite(BITMAP_LIGHT, Position, 2.f, Light, o);
        }

        if (((o->CurrentAction<PLAYER_WALK_MALE || o->CurrentAction>PLAYER_RUN_RIDE_WEAPON) &&
            (o->CurrentAction<PLAYER_ATTACK_SKILL_SWORD1 || o->CurrentAction>PLAYER_ATTACK_SKILL_SWORD5)
            ))
        {
            vec3_t pos, delta, angle;

            Vector(0.f, 0.f, 0.f, p);
            Vector(-90.f, (float)(rand() % 360), o->Angle[2] - 45, angle);
            b->TransformPosition(BoneTransform[3], p, pos, true);
            b->TransformPosition(BoneTransform[2], p, Position, true);

            VectorSubtract(pos, Position, delta);
        }
        break;
    case MODEL_DIVINE_CB_OF_ARCHANGEL:
        Vector(Luminosity * 1.f, Luminosity * 0.3f, Luminosity * 0.1f, Light);
        /*
                for ( i=0; i<4; ++i )
                {
                    vec3_t Light2;
                    Vector ( 0.4f, 0.4f, 0.4f, Light2 );
                    Vector(0.f,i*30.f-10.f,0.f,p);
                    b->TransformPosition(BoneTransform[0],p,Position,true);

                    if ( rand_fps_check(3) )
                    {
                        CreateSprite(BITMAP_SHINY+1,Position,0.6f,Light2, o, ( float)( rand()%360));
                    }
                    CreateSprite(BITMAP_LIGHT,Position,1.f,Light,o);
                }
        */
        break;
    case MODEL_GREAT_REIGN_CROSSBOW:
        Vector(0.f, 0.f, 10.f, p);

        for (int i = 1; i < 6; ++i)
        {
            Vector(Luminosity * 0.5f, Luminosity * 0.5f, Luminosity * 0.8f, Light);
            b->TransformPosition(BoneTransform[i], p, Position, true);

            CreateSprite(BITMAP_SHINY + 1, Position, 1.f, Light, o);

            if (i == 5)
            {
                Vector(1.f, 1.f, 1.f, Light);
                CreateSprite(BITMAP_LIGHT, Position, 2.f, Light, o);
            }
            else
                CreateSprite(BITMAP_LIGHT, Position, 2.f, Light, o);
        }
        break;
    case MODEL_GRAND_SOUL_SHIELD:
        Vector(Luminosity * 0.6f, Luminosity * 0.6f, Luminosity * 2.f, Light);

        Vector(15.f, -15.f, 0.f, p);
        b->TransformPosition(BoneTransform[1], p, Position, true);
        CreateSprite(BITMAP_SHINY + 1, Position, 1.5f, Light, o);
        CreateSprite(BITMAP_LIGHT, Position, Luminosity + 1.5f, Light, o);
        break;
    case MODEL_ELEMENTAL_MACE:
        Vector(Luminosity * 1.f, Luminosity * 0.9f, Luminosity * 0.f, Light);

        Vector(0.f, 0.f, 0.f, p);
        b->TransformPosition(BoneTransform[1], p, Position, true);
        CreateSprite(BITMAP_LIGHT, Position, 2.f, Light, o);

        Vector(0.5f, 0.5f, 0.5f, Light);
        CreateSprite(BITMAP_LIGHT, Position, sinf(WorldTime * 0.002f) + 0.5f, Light, o);
        break;
    case MODEL_BATTLE_SCEPTER:
    {
        float Scale = sinf(WorldTime * 0.001f) + 1.f;
        Vector(Luminosity * 0.2f, Luminosity * 0.1f, Luminosity * 3.f, Light);
        Vector(-15.f, 0.f, 0.f, p);
        b->TransformPosition(BoneTransform[1], p, Position, true);
        CreateSprite(BITMAP_LIGHT, Position, Scale, Light, o);
        Vector(10.f, 0.f, 0.f, p);
        b->TransformPosition(BoneTransform[1], p, Position, true);
        CreateSprite(BITMAP_LIGHT, Position, Scale, Light, o);

        Scale = sinf(WorldTime * 0.01f) * 360;
        Luminosity = sinf(WorldTime * 0.001f) * 0.5f + 0.5f;
        Vector(Luminosity, Luminosity, Luminosity, Light);
        CreateSprite(BITMAP_SHINY + 1, Position, 0.6f, Light, o, 360 - Scale);
        CreateSprite(BITMAP_SHINY + 1, Position, 0.6f, Light, o, Scale);
    }
    break;
    case MODEL_MASTER_SCEPTER:
    {
        for (int i = 1; i < 5; i++)
        {
            Luminosity = (float)sinf((WorldTime) * 0.002f) * 0.35f + 0.65f;
            Vector(Luminosity * 0.6f, Luminosity * 0.8f, Luminosity * 1.f, Light);
            Vector(-10.f, 0.f, 0.f, p);
            b->TransformPosition(BoneTransform[i + 1], p, Position, true);
            CreateSprite(BITMAP_LIGHT, Position, /*Luminosity**/1.f, Light, o);

            if (i == 3)
            {
                Vector(0.5f, 0.5f, 0.5f, Light);
                CreateSprite(BITMAP_SHINY + 1, Position, 0.6f, Light, o, rand() % 360);
            }
        }
    }
    break;
    case MODEL_GREAT_SCEPTER:
    {
        Luminosity = sinf(WorldTime * 0.001f) * 0.5f + 0.7f;
        Vector(Luminosity * 1.f, Luminosity * 0.8f, Luminosity * 0.6f, Light);
        Vector(0.f, 0.f, 0.f, p);

        b->TransformPosition(BoneTransform[2], p, Position, true);
        CreateSprite(BITMAP_LIGHT, Position, 1.5f, Light, o);
        CreateSprite(BITMAP_SHINY + 1, Position, Luminosity * 0.5f, Light, o);

        Vector(Luminosity * -10.f, 0.f, 0.f, p);
        Vector(0.6f, 0.8f, 1.f, Light);
        Luminosity = rand() % 360;
        b->TransformPosition(BoneTransform[1], p, Position, true);
        CreateSprite(BITMAP_LIGHT, Position, 0.7f, Light, o);
        CreateSprite(BITMAP_SHINY + 1, Position, 0.5f, Light, o, Luminosity);
        CreateSprite(BITMAP_SHINY + 1, Position, 0.4f, Light, o, 360 - Luminosity);
    }
    break;
    case MODEL_LORD_SCEPTER:
    {
        Vector(1.f, 0.6f, 0.3f, Light);
        Vector(0.f, 0.f, 0.f, p);
        b->TransformPosition(BoneTransform[1], p, Position, true);
        CreateSprite(BITMAP_LIGHT, Position, 1.3f, Light, o);

        for (int i = 0; i < 3; ++i)
        {
            Vector(i * 15.f - 10.f, 0.f, 0.f, p);
            b->TransformPosition(BoneTransform[2], p, Position, true);
            CreateSprite(BITMAP_LIGHT, Position, 1.3f, Light, o);
            CreateSprite(BITMAP_SHINY + 1, Position, 0.4f, Light, o, rand() % 360);
        }
    }
    break;
    case MODEL_DIVINE_SCEPTER_OF_ARCHANGEL:
    {
        Vector(Luminosity * 1.f, Luminosity * 0.3f, Luminosity * 0.1f, Light);
        Vector(0.f, 0.f, 0.f, p);
        vec3_t Light2;
        Vector(0.4f, 0.4f, 0.4f, Light2);
        b->TransformPosition(BoneTransform[0], p, Position, true);
        if (rand_fps_check(3))
        {
            CreateSprite(BITMAP_SHINY + 1, Position, 0.6f, Light2, o, (float)(rand() % 360));
        }
        CreateSprite(BITMAP_LIGHT, Position, 2.f, Light, o);
    }
    break;
    case MODEL_KNIGHT_BLADE:
    {
        for (int i = 0; i < 2; i++) {
            Vector(0.f, 0.f, 0.f, p);
            b->TransformPosition(BoneTransform[i + 1], p, Position, true);
            Luminosity = (float)sinf((WorldTime) * 0.002f) * 0.35f + 0.65f;
            Vector(Luminosity * 0.43f, Luminosity * 0.14f, Luminosity * 0.6f, Light);
            CreateSprite(BITMAP_LIGHT, Position, Luminosity * 0.9f, Light, o);
            Vector(0.3f, 0.3f, 0.3f, Light);
            CreateSprite(BITMAP_LIGHT, Position, float(sinf(WorldTime * 0.002f) * 0.5f) + 0.4f, Light, o);
        }
    }
    break;
    case MODEL_ARROW_VIPER_BOW:
    {
        float Scale = sinf(WorldTime * 0.001f) + 1.f;
        Vector(Luminosity * 3.f, Luminosity, Luminosity, Light);
        Vector(0.f, 0.f, 0.f, p);
        b->TransformPosition(BoneTransform[6], p, Position, true);
        CreateSprite(BITMAP_LIGHT, Position, Scale * 0.8f, Light, o);
        b->TransformPosition(BoneTransform[2], p, Position, true);
        CreateSprite(BITMAP_LIGHT, Position, Scale * 0.8f, Light, o);

        float Rotation = sinf(WorldTime * 0.01f) * 360;
        Luminosity = sinf(WorldTime * 0.001f) * 0.3 + 0.3f;
        Vector(Luminosity, Luminosity, Luminosity, Light);
        if (!c->SafeZone)
        {
            Vector(10.f, 0.f, 0.f, p);
            b->TransformPosition(BoneTransform[9], p, Position, true);
            CreateSprite(BITMAP_LIGHT, Position, Scale * 0.8f, Light, o);
            Vector(Luminosity * 3.0f, Luminosity, Luminosity, Light);
            CreateSprite(BITMAP_SHINY + 1, Position, 0.8f, Light, o, 360 - Rotation);
            CreateSprite(BITMAP_SHINY + 1, Position, 0.8f, Light, o, Rotation);
        }
    }
    break;
    case MODEL_DARK_REIGN_BLADE:
    case MODEL_RUNE_BLADE:
        if (o->CurrentAction == PLAYER_RUN_TWO_HAND_SWORD_TWO && gMapManager.WorldActive != WD_10HEAVEN && rand_fps_check(2))
        {
            if (!g_Direction.m_CKanturu.IsMayaScene())
            {
                Vector(1.f, 1.f, 1.f, Light);
                Vector(0.f, 0.f, 0.f, p);
                b->TransformPosition(BoneTransform[1], p, Position, true);
                Position[0] += rand() % 30 - 15.f;
                Position[1] += rand() % 30 - 15.f;
                Position[2] += 20.f;

                vec3_t	Angle;
                for (int i = 0; i < 4; i++)
                {
                    Vector((float)(rand() % 60 + 60 + 90), 0.f, o->Angle[2], Angle);//(float)(rand()%30),Angle);
                    CreateJoint(BITMAP_JOINT_SPARK, Position, Position, Angle);
                    CreateParticle(BITMAP_SPARK, Position, Angle, Light);
                }
            }
        }
        break;
    case MODEL_DRAGON_SPEAR:
        Vector(Luminosity * 0.2f, Luminosity * 0.1f, Luminosity * 0.8f, Light);
        Vector(0.f, 0.f, 0.f, p);

        for (int i = 1; i < 9; i++)
        {
            b->TransformPosition(BoneTransform[i], p, Position, true);
            CreateSprite(BITMAP_LIGHT, Position, 1.3f, Light, o);
        }
        break;
    case MODEL_FLAMBERGE:
    {
        Vector(0.8f, 0.6f, 0.2f, Light);
        b->TransformByObjectBone(Position, Object, 11);		// Gold01
        CreateSprite(BITMAP_LIGHT, Position, 0.6f, Light, o);
        CreateSprite(BITMAP_LIGHT, Position, 0.6f, Light, o);

        Vector(0.3f, 0.8f, 0.7f, Light);
        b->TransformByObjectBone(Position, Object, 12);		// b01
        CreateSprite(BITMAP_LIGHT, Position, 0.5f, Light, o);
        b->TransformByObjectBone(Position, Object, 13);		// n02
        CreateSprite(BITMAP_LIGHT, Position, 0.5f, Light, o);

        b->TransformByObjectBone(Position, Object, 12);		// n04
        CreateSprite(BITMAP_LIGHT, Position, 0.3f, Light, o);
        b->TransformByObjectBone(Position, Object, 13);		// b03
        CreateSprite(BITMAP_LIGHT, Position, 0.3f, Light, o);

        Vector(0.9f, 0.1f, 0.1f, Light);
        b->TransformByObjectBone(Position, Object, 1);		// Zx01
        CreateSprite(BITMAP_LIGHTMARKS, Position, 0.8f, Light, o);
        b->TransformByObjectBone(Position, Object, 2);		// Zx02
        CreateSprite(BITMAP_LIGHTMARKS, Position, 0.7f, Light, o);
        b->TransformByObjectBone(Position, Object, 3);		// Zx03
        CreateSprite(BITMAP_LIGHTMARKS, Position, 0.7f, Light, o);
        b->TransformByObjectBone(Position, Object, 4);		// Zx04
        CreateSprite(BITMAP_LIGHTMARKS, Position, 0.7f, Light, o);
        b->TransformByObjectBone(Position, Object, 5);		// Zx05
        CreateSprite(BITMAP_LIGHTMARKS, Position, 0.6f, Light, o);
        b->TransformByObjectBone(Position, Object, 6);		// Zx06
        CreateSprite(BITMAP_LIGHTMARKS, Position, 0.5f, Light, o);
        b->TransformByObjectBone(Position, Object, 7);		// Zx07
        CreateSprite(BITMAP_LIGHTMARKS, Position, 0.5f, Light, o);
    }break;
    case MODEL_SWORD_BREAKER:
    {
        Vector(0.1f, 0.9f, 0.1f, Light);

        b->TransformByObjectBone(Position, Object, 1);		// Zx01
        CreateSprite(BITMAP_LIGHTMARKS, Position, 0.6f, Light, o);
        b->TransformByObjectBone(Position, Object, 2);		// Zx02
        CreateSprite(BITMAP_LIGHTMARKS, Position, 0.5f, Light, o);
        b->TransformByObjectBone(Position, Object, 3);		// Zx03
        CreateSprite(BITMAP_LIGHTMARKS, Position, 0.5f, Light, o);
        b->TransformByObjectBone(Position, Object, 4);		// Zx04
        CreateSprite(BITMAP_LIGHTMARKS, Position, 0.5f, Light, o);
        b->TransformByObjectBone(Position, Object, 5);		// Zx05
        CreateSprite(BITMAP_LIGHTMARKS, Position, 0.4f, Light, o);
        b->TransformByObjectBone(Position, Object, 6);		// Zx06
        CreateSprite(BITMAP_LIGHTMARKS, Position, 0.3f, Light, o);
        b->TransformByObjectBone(Position, Object, 7);		// Zx07
        CreateSprite(BITMAP_LIGHTMARKS, Position, 0.3f, Light, o);

        for (int i = 1; i <= 7; i++)
        {
            if (!rand_fps_check(4))
            {
                continue;
            }

            Vector(0.f, 0.f, 0.f, Position);
            b->TransformByObjectBone(Position, Object, i);
            CreateParticle(BITMAP_WATERFALL_4, Position, Object->Angle, Light, 12, 0.5f, Object);
        }
    }break;
    case MODEL_IMPERIAL_SWORD:
    {
        float fRendomScale = (float)((rand() % 15) / 30.0f) + 0.5f;
        Vector(0.f, 0.f, 0.f, Position);
        Vector(0.1f, 0.4f, 0.9f, Light);
        b->TransformPosition(BoneTransform[8], Position, p, true);		// Zx01
        CreateSprite(BITMAP_FLARE_BLUE, p, 0.4f, o->Light, o);
        CreateSprite(BITMAP_SHINY + 6, p, fRendomScale, Light, o);

        // 잔상 Zx01
        vec3_t vColor;
        VectorCopy(p, o->EyeLeft);
        Vector(0.f, 0.f, 0.9f, vColor);
        if (rand_fps_check(1))
        {
            CreateJoint(BITMAP_JOINT_ENERGY, p, p, o->Angle, 17, o, 25.f);
            //CreateEffect(MODEL_EFFECT_TRACE, p, o->Angle, vColor, 0, NULL, -1, 0, 0, 0, 25.f);
        }

        b->TransformPosition(BoneTransform[9], Position, p, true);		// Zx02
        CreateSprite(BITMAP_FLARE_BLUE, p, 0.4f, o->Light, o);
        CreateSprite(BITMAP_SHINY + 6, p, fRendomScale, Light, o);
        b->TransformPosition(BoneTransform[10], Position, p, true);		// Zx03
        CreateSprite(BITMAP_FLARE_BLUE, p, 0.4f, o->Light, o);
        CreateSprite(BITMAP_SHINY + 6, p, 0.4f, Light, o);
        b->TransformPosition(BoneTransform[11], Position, p, true);		// Zx04
        CreateSprite(BITMAP_FLARE_BLUE, p, 0.4f, o->Light, o);
        CreateSprite(BITMAP_SHINY + 6, p, 0.4f, Light, o);

        // 칼주변
        Vector(0.0f, 0.3f, 0.7f, Light);
        b->TransformPosition(BoneTransform[2], Position, p, true);		// rx01
        CreateSprite(BITMAP_LIGHTMARKS, p, 1.0f, Light, o);
        b->TransformPosition(BoneTransform[3], Position, p, true);		// rx02
        CreateSprite(BITMAP_LIGHTMARKS, p, 0.8f, Light, o);
        b->TransformPosition(BoneTransform[4], Position, p, true);		// rx03
        CreateSprite(BITMAP_LIGHTMARKS, p, 0.6f, Light, o);
        b->TransformPosition(BoneTransform[5], Position, p, true);		// rx04
        CreateSprite(BITMAP_LIGHTMARKS, p, 0.4f, Light, o);
        b->TransformPosition(BoneTransform[6], Position, p, true);		// Zx05
        CreateSprite(BITMAP_LIGHTMARKS, p, 0.2f, Light, o);
        b->TransformPosition(BoneTransform[7], Position, p, true);		// Zx06
        CreateSprite(BITMAP_LIGHTMARKS, p, 0.1f, Light, o);
    }break;
    case MODEL_FROST_MACE:
    {
        vec3_t vDPos;
        Vector(0.5f, 0.8f, 0.5f, Light);
        // Zx04
        b->TransformByObjectBone(Position, Object, 11);
        CreateSprite(BITMAP_SHINY + 6, Position, 0.2f, Light, o);
        // Zx05
        b->TransformByObjectBone(Position, Object, 10);
        CreateSprite(BITMAP_SHINY + 6, Position, 0.2f, Light, o);

        Vector(0.9f, 0.1f, 0.3f, Light);
        // Zx02
        b->TransformByObjectBone(Position, Object, 9);
        CreateSprite(BITMAP_SHINY + 6, Position, 0.2f, Light, o);
        // Zx03
        b->TransformByObjectBone(Position, Object, 8);
        CreateSprite(BITMAP_SHINY + 6, Position, 0.2f, Light, o);
        // Zx01
        b->TransformByObjectBone(Position, Object, 1);
        CreateSprite(BITMAP_SHINY + 6, Position, 0.2f, Light, o);
        CreateSprite(BITMAP_LIGHT, Position, 0.8f, Light, o);

        Vector(0.5f, 0.8f, 0.6f, Light);
        // Zx001
        b->TransformByObjectBone(Position, Object, 24);
        CreateSprite(BITMAP_SHINY + 6, Position, 0.3f, Light, o);
        CreateSprite(BITMAP_LIGHT, Position, 2.0f, Light, o);
        CreateSprite(BITMAP_PIN_LIGHT, Position, 0.5f, Light, o, ((int)(WorldTime * 0.04f) % 360));
        CreateSprite(BITMAP_PIN_LIGHT, Position, 0.7f, Light, o, -((int)(WorldTime * 0.03f) % 360));
        CreateSprite(BITMAP_PIN_LIGHT, Position, 0.9f, Light, o, ((int)(WorldTime * 0.02f) % 360));

        if (rand_fps_check(3))
        {
            float fTemp = Position[2];
            Position[2] -= 15.f;
            Vector(0.5f, 0.8f, 0.8f, Light);
            CreateParticle(BITMAP_CLUD64, Position, o->Angle, Light, 9, 0.4f);
            Position[2] = fTemp;
        }

        Vector(0.5f, 0.8f, 0.6f, Light);
        vDPos[0] = Position[0] + ((float)(rand() % 20 - 10) * 3.f);
        vDPos[1] = Position[1] + ((float)(rand() % 20 - 10) * 3.f);
        vDPos[2] = Position[2] + ((float)(rand() % 20 - 10) * 3.f);

        if (rand_fps_check(10))
        {
            CreateEffect(MODEL_STAR_SHINE, vDPos, o->Angle, Light, 0, Object, -1, 0, 0, 0, 0.22f);
        }

        // Zx06
        b->TransformByObjectBone(Position, Object, 12);
        CreateSprite(BITMAP_LIGHT, Position, 0.2f, Light, o);
        // Zx07
        b->TransformByObjectBone(Position, Object, 13);
        CreateSprite(BITMAP_LIGHT, Position, 0.2f, Light, o);
        // Zx08
        b->TransformByObjectBone(Position, Object, 14);
        CreateSprite(BITMAP_LIGHT, Position, 0.2f, Light, o);
        // Zx09
        b->TransformByObjectBone(Position, Object, 15);
        CreateSprite(BITMAP_LIGHT, Position, 0.2f, Light, o);
        // Zx10
        b->TransformByObjectBone(Position, Object, 16);
        CreateSprite(BITMAP_LIGHT, Position, 0.3f, Light, o);
        // Zx11
        b->TransformByObjectBone(Position, Object, 17);
        CreateSprite(BITMAP_LIGHT, Position, 0.3f, Light, o);
        // Zx12
        b->TransformByObjectBone(Position, Object, 18);
        CreateSprite(BITMAP_LIGHT, Position, 0.3f, Light, o);
        // Zx13
        b->TransformByObjectBone(Position, Object, 19);
        CreateSprite(BITMAP_LIGHT, Position, 0.3f, Light, o);
        // Zx14
        b->TransformByObjectBone(Position, Object, 20);
        CreateSprite(BITMAP_LIGHT, Position, 0.3f, Light, o);
        // Zx15
        b->TransformByObjectBone(Position, Object, 21);
        CreateSprite(BITMAP_LIGHT, Position, 0.4f, Light, o);
        // Zx16
        b->TransformByObjectBone(Position, Object, 22);
        CreateSprite(BITMAP_LIGHT, Position, 0.4f, Light, o);
        // Zx17
        b->TransformByObjectBone(Position, Object, 23);
        CreateSprite(BITMAP_LIGHT, Position, 0.4f, Light, o);
    }break;
    case MODEL_ABSOLUTE_SCEPTER:
    {
        float fRandomScale;
        vec3_t vPosZx01, vPosZx02, vLight1, vLight2, vDLight;

        float fLumi = absf((sinf(WorldTime * 0.0008f))) * 0.8 + 0.2f;
        Vector(fLumi * 0.6f, fLumi * 0.5f, fLumi * 0.8f, vDLight);

        Vector(0.6f, 0.5f, 0.8f, vLight1);
        Vector(0.8f, 0.8f, 0.8f, vLight2);
        b->TransformByObjectBone(vPosZx01, Object, 3);		// Zx01
        b->TransformByObjectBone(vPosZx02, Object, 4);		// Zx02

        if (((int)WorldTime / 100) % 10 == 0)
        {
            Object->m_iAnimation = rand() % 100;
            Object->EyeRight[0] = (rand() % 10 - 5);
            Object->EyeRight[1] = (rand() % 10 - 5);
            Object->EyeRight[2] = (rand() % 10 - 5);
            Object->EyeRight2[0] = (rand() % 10 - 5) * 1.2f;
            Object->EyeRight2[1] = (rand() % 10 - 5) * 1.2f;
            Object->EyeRight2[2] = (rand() % 10 - 5) * 1.2f;
        }
        // Object->m_iAnimation Random Texture
        int iRandomTexure1, iRandomTexure2;
        iRandomTexure1 = (Object->m_iAnimation / 10) % 3;	// 3개
        iRandomTexure2 = (Object->m_iAnimation) % 3;		// 3개

        // Zx01
        fRandomScale = (float)(rand() % 10) / 10.0f + 1.0f;		//(1.0~2.0)
        CreateSprite(BITMAP_LIGHT, vPosZx01, fRandomScale, vLight1, o);
        CreateSprite(BITMAP_SHINY + 1, vPosZx01, 0.5f, vDLight, o);
        VectorAdd(vPosZx01, Object->EyeRight, vPosZx01);
        CreateSprite(BITMAP_LIGHTNING_MEGA1 + iRandomTexure1, vPosZx01, (((rand() % 11) - 20) / 100.f) + 0.5f, vLight2, o, rand() % 380);

        // Zx02
        fRandomScale = (float)((rand() % 10) / 5.0f) + 1.5f;		//(2.0~3.25)
        CreateSprite(BITMAP_LIGHT, vPosZx02, fRandomScale, vLight1, o);
        CreateSprite(BITMAP_SHINY + 1, vPosZx02, 1.0f, vDLight, o);
        VectorAdd(vPosZx02, Object->EyeRight2, vPosZx02);
        CreateSprite(BITMAP_LIGHTNING_MEGA1 + iRandomTexure1, vPosZx02, (((rand() % 11) - 20) / 50.f) + 0.8f, vLight2, o, rand() % 380);
    }break;
    case MODEL_STINGER_BOW:
    {
        vec3_t vZX03, vZx04;
        int iNumCreateFeather = rand() % 3;

        Vector(0.2f, 0.25f, 0.3f, Light);

        for (int i = 0; i <= 43; i++)
        {
            if (i == 1)
            {
                continue;
            }
            b->TransformByObjectBone(Position, Object, i);	//
            CreateSprite(BITMAP_LIGHT, Position, 0.8f, Light, Object);
        }

        if (o->AnimationFrame >= 4.5f && o->AnimationFrame <= 5.0f && rand_fps_check(1))
        {
            for (int i = 0; i < iNumCreateFeather; i++)
            {
                CreateEffect(MODEL_FEATHER, vZX03, o->Angle, Light, 0, NULL, -1, 0, 0, 0, 0.6f);
                CreateEffect(MODEL_FEATHER, vZX03, o->Angle, Light, 1, NULL, -1, 0, 0, 0, 0.6f);
                CreateEffect(MODEL_FEATHER, vZx04, o->Angle, Light, 0, NULL, -1, 0, 0, 0, 0.6f);
                CreateEffect(MODEL_FEATHER, vZx04, o->Angle, Light, 1, NULL, -1, 0, 0, 0, 0.6f);
            }
        }
    }break;
    case MODEL_DEADLY_STAFF:
    {
        Vector(0.f, 0.f, 0.f, Position);
        Vector(0.8f, 0.3f, 0.1f, Light);
        b->TransformPosition(BoneTransform[4], Position, p, true);		// Rx01
        CreateSprite(BITMAP_LIGHT, p, 0.8f, Light, o);
        b->TransformPosition(BoneTransform[5], Position, p, true);		// Rx02
        CreateSprite(BITMAP_LIGHT, p, 0.4f, Light, o);
        b->TransformPosition(BoneTransform[6], Position, p, true);		// Zx02
        CreateSprite(BITMAP_LIGHT, p, 0.8f, Light, o);
        b->TransformPosition(BoneTransform[7], Position, p, true);		// Zx03
        CreateSprite(BITMAP_LIGHT, p, 0.8f, Light, o);
        b->TransformPosition(BoneTransform[8], Position, p, true);		// Zx04
        CreateSprite(BITMAP_LIGHT, p, 2.0f, Light, o);
        b->TransformPosition(BoneTransform[9], Position, p, true);		// Zx05
        CreateSprite(BITMAP_LIGHT, p, 0.8f, Light, o);

        float fLumi = absf((sinf(WorldTime * 0.001f))) * 0.8f + 0.2f;
        vec3_t vDLight;
        Vector(fLumi * 0.8f, fLumi * 0.1f, fLumi * 0.3f, vDLight);
        float fRendomScale = (float)(rand() % 10) / 20.0f + 0.8f;		// (0.5~1.25)
        Vector(0.8f, 0.2f, 0.4f, Light);
        b->TransformPosition(BoneTransform[2], Position, p, true);		// Red01
        CreateSprite(BITMAP_LIGHT, p, 2.0f, vDLight, o);
        CreateSprite(BITMAP_SHINY + 6, p, fRendomScale, Light, o);
        b->TransformPosition(BoneTransform[3], Position, p, true);		// Red02
        CreateSprite(BITMAP_LIGHT, p, 2.0f, vDLight, o);
        CreateSprite(BITMAP_SHINY + 6, p, fRendomScale, Light, o);

        vec3_t vColor;
        VectorCopy(p, o->EyeRight);
        Vector(0.9f, 0.f, 0.f, vColor);
        if (rand_fps_check(1))
        {
            CreateJoint(BITMAP_JOINT_ENERGY, p, p, o->Angle, 47, o, 25.f);
        }
    }break;
    case MODEL_IMPERIAL_STAFF:
    {
        Vector(0.f, 0.f, 0.f, Position);
        Vector(0.3f, 0.3f, 0.9f, Light);
        b->TransformPosition(BoneTransform[2], Position, p, true);		// Zx01
        CreateSprite(BITMAP_LIGHT, p, 2.f, Light, o);
        CreateSprite(BITMAP_LIGHT, p, 2.f, Light, o);
        b->TransformPosition(BoneTransform[3], Position, p, true);		// Zx02
        CreateSprite(BITMAP_LIGHT, p, 2.5f, Light, o);
        CreateSprite(BITMAP_LIGHT, p, 2.5f, Light, o);
        b->TransformPosition(BoneTransform[4], Position, p, true);		// Zx03
        CreateSprite(BITMAP_LIGHT, p, 3.f, Light, o);
        Vector(0.7f, 0.1f, 0.2f, Light);
        b->TransformPosition(BoneTransform[5], Position, p, true);		// Zx04
        CreateSprite(BITMAP_LIGHT, p, 2.f, Light, o);
        Vector(0.9f, 0.3f, 0.5f, Light);
        CreateSprite(BITMAP_SHINY + 6, p, 0.8f, Light, o);

        float fRendomScale = (float)(rand() % 15) / 20.0f + 1.0f;
        CreateSprite(BITMAP_SHINY + 1, p, fRendomScale, Light, o);
        CreateSprite(BITMAP_SHINY + 1, p, fRendomScale - 0.3f, Light, o, 90.0f);
        if (rand_fps_check(1))
        {
            CreateParticle(BITMAP_SPARK + 1, p, o->Angle, Light, 11, 2.0f);
            CreateJoint(BITMAP_JOINT_ENERGY, p, p, o->Angle, 17, o, 30.f);
            //vec3_t vColor;
            //Vector(0.f, 0.f, 0.9f, vColor);
            //CreateEffect(MODEL_EFFECT_TRACE, p, o->Angle, vColor, 0, NULL, -1, 0, 0, 0, 30.f);
        }

        // 잔상
        
        VectorCopy(p, o->EyeLeft);

        Vector(0.7f, 0.7f, 0.7f, Light);
        CreateSprite(BITMAP_SHINY + 2, p, 2.f, Light, o);
    }break;
    case MODEL_STAFF + 32:
    {
        float fRandomScale;
        vec3_t vLight1, vLight2;
        Vector(0.9f, 0.7f, 0.4f, vLight1);
        Vector(0.9f, 0.1f, 0.3f, vLight2);

        b->TransformByObjectBone(Position, Object, 1);		// Zx01
        Vector(1.0f, 0.1f, 0.2f, Light);
        CreateSprite(BITMAP_SHINY + 6, Position, 0.7f, Light, o);
        CreateSprite(BITMAP_LIGHTMARKS, Position, 0.6f, Light, o);
        CreateSprite(BITMAP_LIGHT, Position, 2.f, Light, o);

        Vector(0.7f, 0.1f, 0.6f, Light);
        CreateSprite(BITMAP_SHOCK_WAVE, Position, 0.65f, Light, o, -((int)(WorldTime * 0.05f) % 360));
        // Object->Timer
        // Object->EyeRight
        Object->Timer += 0.01f * FPS_ANIMATION_FACTOR;
        if (Object->Timer <= 0.1f || Object->Timer > 0.9f)
        {
            Object->Timer = 0.15f;
            Vector(0.7f, 0.1f, 0.6f, Object->EyeRight);
        }
        if (Object->Timer > 0.5f)
        {
            Object->EyeRight[0] *= 0.95f;
            Object->EyeRight[1] *= 0.95f;
            Object->EyeRight[2] *= 0.95f;
        }

        CreateSprite(BITMAP_SHOCK_WAVE, Position, Object->Timer, Object->EyeRight, o);

        Vector(0.9f, 0.5f, 0.2f, Light);
        fRandomScale = (float)(rand() % 5) / 25.0f + 0.3f;		// (0.3~0.4)
        CreateSprite(BITMAP_LIGHTMARKS, Position, fRandomScale, Light, o);

        b->TransformByObjectBone(Position, Object, 2);		// Zx02
        CreateSprite(BITMAP_SHINY + 6, Position, 0.3f, vLight1, o);
        CreateSprite(BITMAP_LIGHTMARKS, Position, 0.4f, vLight2, o);

        b->TransformByObjectBone(Position, Object, 3);		// Zx03
        CreateSprite(BITMAP_SHINY + 6, Position, 0.2f, vLight1, o);
        CreateSprite(BITMAP_LIGHTMARKS, Position, 0.3f, vLight2, o);
    }break;
    case MODEL_CRIMSONGLORY:
    {
        vec3_t vDLight;
        Vector(0.8f, 0.6f, 0.2f, Light);
        b->TransformByObjectBone(Position, Object, 4);		// Zx03
        CreateSprite(BITMAP_LIGHT, Position, 1.5f, Light, o);
        b->TransformByObjectBone(Position, Object, 5);		// Zx04
        CreateSprite(BITMAP_LIGHT, Position, 1.5f, Light, o);
        b->TransformByObjectBone(Position, Object, 6);		// Zx05
        CreateSprite(BITMAP_LIGHT, Position, 1.5f, Light, o);
        b->TransformByObjectBone(Position, Object, 7);		// Zx06
        CreateSprite(BITMAP_LIGHT, Position, 1.5f, Light, o);
        b->TransformByObjectBone(Position, Object, 8);		// Zx07
        CreateSprite(BITMAP_LIGHT, Position, 1.5f, Light, o);
        b->TransformByObjectBone(Position, Object, 9);		// Zx08
        CreateSprite(BITMAP_LIGHT, Position, 1.5f, Light, o);
        b->TransformByObjectBone(Position, Object, 10);		// Zx09
        CreateSprite(BITMAP_LIGHT, Position, 1.5f, Light, o);

        float fLumi = absf((sinf(WorldTime * 0.0005f)));
        Vector(fLumi * 1.f, fLumi * 1.f, fLumi * 1.f, vDLight);
        b->TransformByObjectBone(Position, Object, 1);		// Zx01
        CreateSprite(BITMAP_FLARE_RED, Position, 0.5f, vDLight, o);
        b->TransformByObjectBone(Position, Object, 2);		// Zx02
        CreateSprite(BITMAP_FLARE_RED, Position, 0.3f, vDLight, o);
        b->TransformByObjectBone(Position, Object, 3);		// Zx002
        CreateSprite(BITMAP_FLARE_RED, Position, 0.3f, vDLight, o);
    }break;
    case MODEL_SALAMANDER_SHIELD:
    {
        Vector(0.9f, 0.f, 0.2f, Light);
        b->TransformByObjectBone(Position, Object, 1);		// Zx01
        CreateSprite(BITMAP_LIGHT, Position, 2.0f, Light, o);

        if (rand_fps_check(1))
        {
            Vector(1.f, 1.f, 1.f, Light);
            switch (rand() % 3)
            {
            case 0:
                CreateParticle(BITMAP_FIRE_HIK1, Position, Object->Angle, Light, 0, 0.7f);
                break;
            case 1:
                CreateParticle(BITMAP_FIRE_CURSEDLICH, Position, Object->Angle, Light, 4, 0.7f);
                break;
            case 2:
                CreateParticle(BITMAP_FIRE_HIK3, Position, Object->Angle, Light, 0, 0.7);
                break;
            }
        }
    }break;
    case MODEL_GUARDIAN_SHILED:
    {
        Vector(0.f, 0.f, 0.f, Position);
        float fLumi = fabs(sinf(WorldTime * 0.001f)) + 0.1f;
        Vector(0.8f * fLumi, 0.3f * fLumi, 0.8f * fLumi, Light);
        b->TransformPosition(BoneTransform[1], Position, p, true);		// b01
        CreateSprite(BITMAP_LIGHT, p, 1.5f, Light, o);
        b->TransformPosition(BoneTransform[2], Position, p, true);		// Zx01
        CreateSprite(BITMAP_LIGHT, p, 1.5f, Light, o);
        b->TransformPosition(BoneTransform[3], Position, p, true);		// Zx05
        CreateSprite(BITMAP_LIGHT, p, 1.5f, Light, o);
        b->TransformPosition(BoneTransform[4], Position, p, true);		// Zx04
        CreateSprite(BITMAP_LIGHT, p, 1.5f, Light, o);
        b->TransformPosition(BoneTransform[5], Position, p, true);		// Object04
        CreateSprite(BITMAP_LIGHT, p, 1.5f, Light, o);
        b->TransformPosition(BoneTransform[6], Position, p, true);		// Object05
        CreateSprite(BITMAP_LIGHT, p, 1.5f, Light, o);
        b->TransformPosition(BoneTransform[7], Position, p, true);		// Zx02
        CreateSprite(BITMAP_LIGHT, p, 1.5f, Light, o);
        b->TransformPosition(BoneTransform[8], Position, p, true);		// Object01
        CreateSprite(BITMAP_LIGHT, p, 1.5f, Light, o);
        b->TransformPosition(BoneTransform[9], Position, p, true);		// Object03
        CreateSprite(BITMAP_LIGHT, p, 1.5f, Light, o);
    }break;
    case MODEL_CROSS_SHIELD:
    {
        vec3_t vPos, vLight;
        Vector(0.f, 0.f, 0.f, vPos);
        float fLumi = fabs(sinf(WorldTime * 0.001f)) + 0.1f;
        Vector(0.2f * fLumi, 0.2f * fLumi, 0.8f * fLumi, vLight);
        b->TransformByObjectBone(vPos, Object, 4);
        CreateSprite(BITMAP_LIGHT, vPos, 1.2f, vLight, Object);
        b->TransformByObjectBone(vPos, Object, 5);
        CreateSprite(BITMAP_LIGHT, vPos, 1.2f, vLight, Object);
        b->TransformByObjectBone(vPos, Object, 6);
        CreateSprite(BITMAP_LIGHT, vPos, 1.2f, vLight, Object);
        b->TransformByObjectBone(vPos, Object, 7);
        CreateSprite(BITMAP_LIGHT, vPos, 1.2f, vLight, Object);

        Vector(0.2f * fLumi, 0.6f * fLumi, 0.6f * fLumi, vLight);
        b->TransformByObjectBone(vPos, Object, 8);
        CreateSprite(BITMAP_LIGHT, vPos, 1.4f, vLight, Object);
        b->TransformByObjectBone(vPos, Object, 9);
        CreateSprite(BITMAP_LIGHT, vPos, 1.4f, vLight, Object);
        b->TransformByObjectBone(vPos, Object, 10);
        CreateSprite(BITMAP_LIGHT, vPos, 1.4f, vLight, Object);
        b->TransformByObjectBone(vPos, Object, 11);
        CreateSprite(BITMAP_LIGHT, vPos, 1.4f, vLight, Object);
        b->TransformByObjectBone(vPos, Object, 12);
        CreateSprite(BITMAP_LIGHT, vPos, 1.4f, vLight, Object);
        b->TransformByObjectBone(vPos, Object, 13);
        CreateSprite(BITMAP_LIGHT, vPos, 1.4f, vLight, Object);
    }
    break;
    case MODEL_CHROMATIC_STAFF:
    {
        vec3_t vPos, vLight;

        b->TransformByObjectBone(vPos, Object, 8);
        Vector(0.9f, 0.1f, 0.4f, vLight);
        CreateSprite(BITMAP_LIGHT, vPos, 3.5f, vLight, Object);
        vLight[0] = 0.1f + 0.8f * absf(sinf(WorldTime * 0.0008f));
        vLight[1] = 0.1f * absf(sinf(WorldTime * 0.0008f));
        vLight[2] = 0.1f + 0.4f * absf(sinf(WorldTime * 0.0008f));
        CreateSprite(BITMAP_MAGIC, vPos, 0.3f, vLight, Object);
        if (rand_fps_check(1))
        {
            CreateEffect(MODEL_MOONHARVEST_MOON, vPos, o->Angle, vLight, 2, NULL, -1, 0, 0, 0, 0.12f);
        }

        Vector(0.8f, 0.8f, 0.2f, vLight);
        CreateSprite(BITMAP_SHINY + 1, vPos, 1.0f, vLight, Object);

        //작은 구슬
        for (int i = 1; i < 8; i++)
        {
            b->TransformByObjectBone(vPos, Object, i);
            Vector(0.8f, 0.1f, 0.4f, vLight);
            CreateSprite(BITMAP_LIGHT, vPos, 0.6f, vLight, Object);
            CreateSprite(BITMAP_MAGIC, vPos, 0.12f, vLight, Object);
            Vector(0.8f, 0.8f, 0.2f, vLight);
            CreateSprite(BITMAP_SHINY + 1, vPos, 0.3f, vLight, Object);
        }
    }
    break;
    case MODEL_RAVEN_STICK:
    {
        vec3_t vPos, vLight;

        //zx01
        b->TransformByObjectBone(vPos, Object, 1);
        Vector(0.4f, 0.2f, 1.0f, vLight);
        CreateSprite(BITMAP_SHINY + 6, vPos, 1.4f, vLight, Object);
        Vector(0.7f, 0.1f, 0.9f, vLight);
        CreateSprite(BITMAP_LIGHT, vPos, 1.5f, vLight, Object);
        Vector(0.0f, 0.0f, 1.0f, vLight);
        CreateSprite(BITMAP_SHINY + 2, vPos, 1.5f, vLight, Object);
        Vector(0.4f, 0.4f, 1.0f, vLight);
        CreateSprite(BITMAP_PIN_LIGHT, vPos, 0.7f, vLight, Object, -((int)(WorldTime * 0.04f) % 360));
        CreateSprite(BITMAP_PIN_LIGHT, vPos, 0.8f, vLight, Object, -((int)(WorldTime * 0.03f) % 360));

        float quarterAngle, theta, fSize;
        quarterAngle = Q_PI / 180.0f * (int(WorldTime * 0.02f) % 90);
        theta = absf(sinf(quarterAngle + Q_PI / 2));
        fSize = absf(sinf(quarterAngle)) * 0.5f;
        Vector(0.7f * theta, 0.1f * theta, 0.9f * theta, vLight);
        CreateSprite(BITMAP_MAGIC, vPos, fSize, vLight, Object);
        quarterAngle = Q_PI / 180.0f * (int(WorldTime * 0.05f) % 60 + 30);
        theta = absf(sinf(quarterAngle + Q_PI / 2));
        fSize = absf(sinf(quarterAngle)) * 0.5f;
        Vector(0.1f + 0.7f * theta, 0.1f * theta, 0.1f + 0.3f * theta, vLight);
        CreateSprite(BITMAP_MAGIC, vPos, fSize, vLight, Object);

        //zx02
        Vector(0.9f, 0.0f, 0.1f, vLight);
        b->TransformByObjectBone(vPos, Object, 2);
        CreateSprite(BITMAP_SHINY + 6, vPos, 0.8f, vLight, Object);

        //zx03
        Vector(0.9f, 0.4f, 0.7f, vLight);
        b->TransformByObjectBone(vPos, Object, 3);
        CreateSprite(BITMAP_SHINY + 6, vPos, 0.6f, vLight, Object);
    }
    break;
    case MODEL_BEUROBA:
    {
        vec3_t vPos, vLight;

        //zx01, zx02
        Vector(0.9f, 0.2f, 0.7f, vLight);
        b->TransformByObjectBone(vPos, Object, 3);
        CreateSprite(BITMAP_SHINY + 6, vPos, 0.8f, vLight, Object);
        CreateSprite(BITMAP_SHINY + 6, vPos, 0.3f, vLight, Object);
        b->TransformByObjectBone(vPos, Object, 4);
        CreateSprite(BITMAP_SHINY + 6, vPos, 0.8f, vLight, Object);
        CreateSprite(BITMAP_SHINY + 6, vPos, 0.3f, vLight, Object);

        //bone02
        Vector(0.2f, 0.9f, 0.1f, vLight);
        b->TransformByObjectBone(vPos, Object, 5);
        CreateSprite(BITMAP_LIGHT, vPos, 1.5f, vLight, Object);
    }
    break;
    case MODEL_STRYKER_SCEPTER:
    {
        vec3_t vPos, vLight;
        float fSize = 0.0f;

        //rx01
        b->TransformByObjectBone(vPos, Object, 5);
        Vector(0.5f, 0.8f, 0.9f, vLight);
        CreateSprite(BITMAP_SHINY + 1, vPos, 0.5f, vLight, Object);
        Vector(0.4f, 0.7f, 0.9f, vLight);
        CreateSprite(BITMAP_LIGHTMARKS, vPos, 1.0f, vLight, Object);

        Vector(0.4f, 0.6f, 0.9f, vLight);
        if (int(WorldTime) % 14 == 0)
            CreateParticle(BITMAP_SHINY + 6, vPos, o->Angle, vLight, 1);

        //rx02
        Vector(0.5f, 0.6f, 0.9f, vLight);
        b->TransformByObjectBone(vPos, Object, 6);
        CreateSprite(BITMAP_SHINY + 1, vPos, 1.1f, vLight, Object);

        //zx 01, 02
        Vector(0.8f, 0.5f, 0.2f, vLight);
        b->TransformByObjectBone(vPos, Object, 2);
        CreateSprite(BITMAP_SHINY + 1, vPos, 0.5f, vLight, Object);
        b->TransformByObjectBone(vPos, Object, 3);
        CreateSprite(BITMAP_SHINY + 1, vPos, 0.5f, vLight, Object);

        //zx03
        Vector(1.0f, 1.0f, 1.0f, vLight);
        b->TransformByObjectBone(vPos, Object, 4);
        fSize = 0.5f * absf((sinf(WorldTime * 0.0015f)));
        CreateSprite(BITMAP_FLARE, vPos, fSize, vLight, Object);
    }
    break;
    case MODEL_AIR_LYN_BOW:
    {
        vec3_t vPos, vLight;
        float quarterAngle, theta, fSize;

        //zx02, 03
        Vector(0.2f, 0.8f, 0.5f, vLight);
        int iBoneNum[2] = { 7,11 };

        for (int i = 0; i < 2; i++)
        {
            b->TransformByObjectBone(vPos, Object, iBoneNum[i]);

            CreateSprite(BITMAP_LIGHT, vPos, 0.8f, vLight, Object);
            CreateSprite(BITMAP_LIGHT, vPos, 0.8f, vLight, Object);

            quarterAngle = Q_PI / 180.0f * (int(WorldTime * 0.02f) % 90);
            theta = absf(sinf(quarterAngle + Q_PI / 2));
            fSize = absf(sinf(quarterAngle)) * 0.3f;
            Vector(0.2f * theta, 0.8f * theta, 0.5f * theta, vLight);
            CreateSprite(BITMAP_SHOCK_WAVE, vPos, fSize, vLight, Object);

            quarterAngle = Q_PI / 180.0f * (int(WorldTime * 0.05f) % 60 + 30);
            theta = absf(sinf(quarterAngle + Q_PI / 2));
            fSize = absf(sinf(quarterAngle)) * 0.3f;
            Vector(0.2f * theta, 0.8f * theta, 0.5f * theta, vLight);
            CreateSprite(BITMAP_SHOCK_WAVE, vPos, fSize, vLight, Object);
        }

        //model_bow action, frame
        if (o->CurrentAction >= PLAYER_ATTACK_FIST && o->CurrentAction <= PLAYER_RIDE_SKILL)
        {
            Vector(0.2f, 0.8f, 0.5f, vLight);
            for (int i = 0; i < 8; i++)
            {
                b->TransformByObjectBone(vPos, Object, 22 + i);
                CreateSprite(BITMAP_FLARE, vPos, 0.4f, vLight, Object);
            }
        }
    }
    break;
    case MODEL_SWORD_35_WING:
        vec3_t vLight;
        float fSize = (double)(rand() % 30) / 300.0;
        Vector(0.18f, 0.45f, 0.22f, vLight);
        Vector(0.f, 0.f, 0.f, p);
        for (int i = 0; i < 8; ++i)
        {
            b->TransformPosition(BoneTransform[3 + i], p, Position, true);
            CreateSprite(BITMAP_LIGHT_MARKS, Position, fSize + 0.3f, vLight, Object, rand() % 360);
        }
        for (int i = 0; i < 8; ++i)
        {
            b->TransformPosition(BoneTransform[13 + i], p, Position, true);
            CreateSprite(BITMAP_LIGHT_MARKS, Position, fSize + 0.3f, vLight, Object, rand() % 360);
        }
        break;
    }

    if (gMapManager.WorldActive != WD_10HEAVEN && gMapManager.InHellas() == FALSE && !g_Direction.m_CKanturu.IsMayaScene())
    {
        switch (Type)        // 날개인지 검사
        {
        case MODEL_WINGS_OF_ELF:        // Wings of Elf
        case MODEL_WINGS_OF_HEAVEN:        // Wings of Heaven
        case MODEL_WINGS_OF_SATAN:        // Wings of Satan
        case MODEL_WING_OF_CURSE:       // Wing of Curse

        case MODEL_WINGS_OF_SPIRITS:        // Wings of Spirits
        case MODEL_WINGS_OF_SOUL:        // Wings of Soul
        case MODEL_WINGS_OF_DRAGON:        // Wings of Dragon
        case MODEL_WINGS_OF_DARKNESS:        // Wings of Darkness
        case MODEL_WINGS_OF_DESPAIR:        // Wings of Despair
        
        case MODEL_WING_OF_STORM:        // Wing of Storm
        case MODEL_WING_OF_ETERNAL:        // Wing of Eternal
        case MODEL_WING_OF_ILLUSION:        // Wing of Illusion
        case MODEL_WING_OF_RUIN:        // Wing of Ruin
        case MODEL_WING_OF_DIMENSION:        // Wing of Dimension

        case MODEL_WING + 131:        // Small Wing of Curse
        case MODEL_WING + 132:        // Small Wings of Elf
        case MODEL_WING + 133:        // Small Wings of Heaven
        case MODEL_WING + 134:        // Small Wings of Satan
            b->RenderBodyShadow();
            break;
            
        case MODEL_CAPE_OF_LORD:    // Cape of Lord
        case MODEL_CAPE_OF_FIGHTER:      // Cape of Fighter
        case MODEL_CAPE_OF_EMPEROR:        // Cape of Emperor
        case MODEL_CAPE_OF_OVERRULE:        // Cape of Overrule
        case MODEL_WING + 130:        // Small Cape of Lord
        case MODEL_WING + 135:        // Little Warrior's Cloak
            b->RenderBodyShadow(-1, -1, -1, -1, o->m_pCloth, o->m_byNumCloth);
            break;
        default:
            if (o->m_pCloth)
            {
                b->RenderBodyShadow(-1, -1, -1, -1, o->m_pCloth, o->m_byNumCloth);
            }
            break;
        }
    }
}

void RenderLight(OBJECT* o, int Texture, float Scale, int Bone, float x, float y, float z)
{
    BMD* b = &Models[o->Type];
    vec3_t p, Position;
    Vector(x, y, z, p);
    b->TransformPosition(o->BoneTransform[Bone], p, Position, true);
    float Luminosity = sinf(WorldTime * 0.002f) * 0.3f + 0.7f;
    vec3_t Light;
    Vector(Luminosity * 1.f, Luminosity * 0.6f, Luminosity * 0.4f, Light);
    CreateSprite(Texture, Position, Scale, Light, o);
}

void RenderEye(OBJECT* o, int Left, int Right, float fSize = 1.0f)
{
    BMD* b = &Models[o->Type];
    vec3_t p, Position;
    float Luminosity = sinf(WorldTime * 0.002f) * 0.3f + 0.8f;
    vec3_t Light;
    Vector(Luminosity, Luminosity, Luminosity, Light);
    Vector(5.f, 0.f, 0.f, p);
    b->TransformPosition(o->BoneTransform[Left], p, Position, true);
    //CreateParticle(BITMAP_SHINY+3,Position,o->Angle,Light);
    CreateSprite(BITMAP_SHINY + 3, Position, fSize, Light, NULL);
    Vector(-5.f, 0.f, 0.f, p);
    b->TransformPosition(o->BoneTransform[Right], p, Position, true);
    //CreateParticle(BITMAP_SHINY+3,Position,o->Angle,Light);
    CreateSprite(BITMAP_SHINY + 3, Position, fSize, Light, NULL);
    VectorCopy(Position, o->EyeLeft);
}

void RenderCharacter(CHARACTER* c, OBJECT* o, int Select)
{
    if (g_isCharacterBuff(o, eBuff_CrywolfNPCHide))
    {
        return;
    }

    BMD* b = &Models[o->Type];
    if (Models[o->Type].NumActions == 0) return;

    bool Translate = true;

    vec3_t p, Position, Light;

    Vector(0.f, 0.f, 0.f, p);
    Vector(1.f, 1.f, 1.f, Light);

    BYTE byRender = CHARACTER_NONE;

    switch (c->MonsterIndex)
    {
    case MONSTER_MAGIC_SKELETON_1:
    case MONSTER_MAGIC_SKELETON_2:
    case MONSTER_MAGIC_SKELETON_3:
    case MONSTER_MAGIC_SKELETON_4:
    case MONSTER_MAGIC_SKELETON_5:
    case MONSTER_MAGIC_SKELETON_6:
    case MONSTER_MAGIC_SKELETON_7:
    {
        BOOL bRender = Calc_RenderObject(o, Translate, Select, 0);

        if (!c->Object.m_pCloth)
        {
            auto* pCloth = new CPhysicsClothMesh[1];
            pCloth[0].Create(&(c->Object), 2, 18, PCT_HEAVY);
            pCloth[0].AddCollisionSphere(0.0f, 0.0f, 0.0f, 50.0f, 18);
            pCloth[0].AddCollisionSphere(0.0f, -20.0f, 0.0f, 30.0f, 18);
            c->Object.m_pCloth = (void*)pCloth;
            c->Object.m_byNumCloth = 1;
        }
        auto* pCloth = (CPhysicsCloth*)c->Object.m_pCloth;

        if (pCloth)
        {
            if (!pCloth[0].Move2(0.005f, 5))
            {
                DeleteCloth(c, o);
            }
            else
            {
                pCloth[0].Render();
            }
        }
        if (bRender)
        {
            Draw_RenderObject(o, Translate, Select, 0);
        }
    }
    break;

    default:
        if (o->Type == MODEL_PLAYER)
        {
            byRender = CHARACTER_ANIMATION;
        }
        else
            byRender = CHARACTER_RENDER_OBJ;
        break;
    }

    if (o->Type == MODEL_PLAYER
        && (o->SubType == MODEL_XMAS_EVENT_CHA_SSANTA
            || o->SubType == MODEL_XMAS_EVENT_CHA_SNOWMAN
            || o->SubType == MODEL_XMAS_EVENT_CHA_DEER))
    {
        OBJECT* pOwner = o->Owner;

        if (pOwner->Live == false)
        {
            o->Live = false;
        }

        VectorCopy(pOwner->Position, o->Position);
        VectorCopy(pOwner->Angle, o->Angle);
        o->PriorAction = pOwner->PriorAction;
        o->PriorAnimationFrame = pOwner->PriorAnimationFrame;
        o->CurrentAction = pOwner->CurrentAction;
        o->AnimationFrame = pOwner->AnimationFrame;

        if (timeGetTime() - o->m_dwTime >= XMAS_EVENT_TIME)
        {
            DeleteCharacter(c, o);
        }
    }
    else if (o->Type == MODEL_PLAYER && o->SubType == MODEL_XMAS_EVENT_CHANGE_GIRL
        || o->Type == MODEL_PLAYER && o->SubType == MODEL_PANDA
        || o->Type == MODEL_PLAYER && o->SubType == MODEL_SKELETON_CHANGED
        )
    {
        if (o->m_iAnimation >= 1)
        {
            o->m_iAnimation = 0;
            SetPlayerStop(c);
        }
    }

    if (byRender == CHARACTER_ANIMATION)
        Calc_ObjectAnimation(o, Translate, Select);

    if (o->Alpha >= 0.5f && c->HideShadow == false)
    {
        if (gMapManager.WorldActive != WD_10HEAVEN && (o->Type == MODEL_PLAYER) && (!(MODEL_HORN_OF_UNIRIA <= c->Helper.Type && c->Helper.Type <= MODEL_HORN_OF_DINORANT) || c->SafeZone)
            && gMapManager.InHellas() == false
            )
        {
            if (gMapManager.InBloodCastle() && o->m_bActionStart && c->Dead > 0)
            {
                float height = RequestTerrainHeight(o->Position[0], o->Position[1]);
                if (height < o->Position[2])
                {
                    o->Position[2] = height;
                }
            }
            o->EnableShadow = true;
            for (int i = MAX_BODYPART - 1; i >= 0; i--)
            {
                PART_t* p = &c->BodyPart[i];
                if (p->Type != -1)
                {
                    int Type = p->Type;

                    RenderPartObject(&c->Object, Type, p, c->Light, o->Alpha, 0, 0, 0, false, false, Translate);
                }
                else
                {
                    RenderPartObject(&c->Object, MODEL_SHADOW_BODY, NULL, c->Light, o->Alpha, 0, 0, 0, false, false, Translate);
                }
            }

            for (int i = 0; i < 2; i++)
            {
                if (c->Weapon[i].Type >= MODEL_SHORT_BOW && c->Weapon[i].Type <= MODEL_BOW + 32)
                {
                    continue;
                }

                PART_t* p = &c->Weapon[i];

                if (p->Type != -1 && c->SafeZone == false)
                {
                    int Type = p->Type;

                    RenderPartObject(&c->Object, Type, p, c->Light, o->Alpha, 0, 0, 0, false, false, Translate);
                }
            }
            o->EnableShadow = false;
        }
    }

    if (byRender == CHARACTER_RENDER_OBJ)
    {
        if (MONSTER_METAL_BALROG == c->MonsterIndex || MONSTER_ALPHA_CRUST == c->MonsterIndex || MONSTER_GREAT_DRAKAN == c->MonsterIndex
            || MONSTER_WHITE_WIZARD == c->MonsterIndex || MONSTER_ORC_SOLDIER_OF_DOOM == c->MonsterIndex || MONSTER_ORC_ARCHER_OF_DOOM == c->MonsterIndex
            || MONSTER_MUTANT_HERO == c->MonsterIndex || MONSTER_OMEGA_WING == c->MonsterIndex || MONSTER_AXE_HERO == c->MonsterIndex || MONSTER_GIGAS_GOLEM == c->MonsterIndex
            || MONSTER_SCOUTHERO == c->MonsterIndex || MONSTER_WEREWOLFHERO == c->MonsterIndex || MONSTER_VALAM == c->MonsterIndex || MONSTER_SOLAM == c->MonsterIndex || MONSTER_SCOUT == c->MonsterIndex || 319 == c->MonsterIndex
            )
        {
            RenderObject(o, Translate, Select, c->MonsterIndex);
        }
        else
        {
            if ((c->MonsterIndex == MONSTER_DREADFEAR && o->CurrentAction == MONSTER01_ATTACK2))
            {
                RenderObject_AfterImage(o, Translate, Select, 0);
            }
            else
            {
                RenderObject(o, Translate, Select, 0);
            }
        }
    }

    if ((
        o->Type != MODEL_PLAYER && o->Kind != KIND_TRAP &&
        c->MonsterIndex != MONSTER_ICE_QUEEN && c->MonsterIndex != MONSTER_ICE_MONSTER && c->MonsterIndex != MONSTER_RED_DRAGON && c->MonsterIndex != MONSTER_ELF_LALA && c->MonsterIndex != MONSTER_ZAIKAN && c->MonsterIndex != MONSTER_DEATH_BEAM_KNIGHT
        && c->MonsterIndex != MONSTER_GATE_TO_KALIMA_1
        ) && gMapManager.WorldActive != WD_10HEAVEN
        )
    {
        if (o->Alpha >= 0.3f)
        {
            if (gMapManager.InBloodCastle() == true && o->m_bActionStart && c->Dead > 0)
            {
                float height = RequestTerrainHeight(o->Position[0], o->Position[1]);
                if (height < o->Position[2])
                {
                    o->Position[2] = height;
                }
            }

            if (c->MonsterIndex == MONSTER_ARCHANGEL)
                o->HiddenMesh = 2;
            else if (c->MonsterIndex == MONSTER_MESSENGER_OF_ARCH)
                o->HiddenMesh = 2;

            if (o->Type != MODEL_STATUE_OF_SAINT && o->Type != MODEL_CASTLE_GATE
                && !(o->Type >= MODEL_FACE
                    && o->Type <= MODEL_FACE + 6))
            {
                o->EnableShadow = true;
                RenderPartObject(&c->Object, o->Type, NULL, c->Light, o->Alpha, 0, 0, 0, false, false, Translate);
                o->EnableShadow = false;
            }
            if (c->MonsterIndex == MONSTER_ARCHANGEL || c->MonsterIndex == MONSTER_MESSENGER_OF_ARCH)
            {
                EnableAlphaBlend();

                vec3_t Position, Light;

                VectorCopy(o->Position, Position);
                Position[2] += 20.f;

                float Luminosity = sinf(WorldTime * 0.0015f) * 0.3f + 0.8f;

                Vector(Luminosity * 0.5f, Luminosity * 0.5f, Luminosity, Light);
                RenderTerrainAlphaBitmap(BITMAP_MAGIC + 1, o->Position[0], o->Position[1], 2.7f, 2.7f, Light, -o->Angle[2]);

                o->HiddenMesh = -1;
            }
        }
    }

    if (c->MonsterIndex == MONSTER_BALROG || c->MonsterIndex == MONSTER_GOLDEN_BUDGE_DRAGON || c->MonsterIndex == MONSTER_SILVER_VALKYRIE || c->MonsterIndex == MONSTER_ZAIKAN || c->MonsterIndex == MONSTER_METAL_BALROG || (78 <= c->MonsterIndex && c->MonsterIndex <= MONSTER_GOLDEN_WHEEL) || (c->MonsterIndex >= MONSTER_GOLDEN_DARK_KNIGHT && c->MonsterIndex <= MONSTER_GOLDEN_RABBIT))
    {
        vec3_t vBackupBodyLight;

        float Bright = 1.f;
        if (c->MonsterIndex == MONSTER_ZAIKAN)
            Bright = 0.5f;
        if (c->MonsterIndex == MONSTER_GOLDEN_BUDGE_DRAGON || (78 <= c->MonsterIndex && c->MonsterIndex <= MONSTER_GOLDEN_WHEEL) || (c->MonsterIndex >= MONSTER_GOLDEN_DARK_KNIGHT && c->MonsterIndex <= MONSTER_GOLDEN_RABBIT))
        {
            if (c->MonsterIndex >= MONSTER_GOLDEN_DARK_KNIGHT)
            {
                VectorCopy(Models[o->Type].BodyLight, vBackupBodyLight);
                Vector(1.f, 0.6f, 0.3f, Models[o->Type].BodyLight);
            }

            if (c->MonsterIndex == MONSTER_GOLDEN_GREAT_DRAGON)
            {
                VectorCopy(Models[o->Type].BodyLight, vBackupBodyLight);
                Vector(1.f, 0.0f, 0.0f, Models[o->Type].BodyLight);

                float	fEffectScale = o->Scale * 1.6f;
                vec3_t	v3EffectLightColor, v3EffectPosition;

                Vector(1.0f, 0.6f, 0.1f, v3EffectLightColor);

                b->TransformPosition(o->BoneTransform[0], p, v3EffectPosition, true);
                CreateSprite(BITMAP_LIGHTMARKS, v3EffectPosition, 2.5f, v3EffectLightColor, o);

                b->TransformPosition(o->BoneTransform[4], p, v3EffectPosition, true);
                CreateSprite(BITMAP_LIGHTMARKS, v3EffectPosition, 2.5f, v3EffectLightColor, o);

                b->TransformPosition(o->BoneTransform[57], p, v3EffectPosition, true);
                CreateSprite(BITMAP_LIGHTMARKS, v3EffectPosition, 2.5f, v3EffectLightColor, o);

                b->TransformPosition(o->BoneTransform[60], p, v3EffectPosition, true);
                CreateSprite(BITMAP_LIGHTMARKS, v3EffectPosition, 2.5f, v3EffectLightColor, o);

                b->TransformPosition(o->BoneTransform[87], p, v3EffectPosition, true);
                CreateSprite(BITMAP_LIGHTMARKS, v3EffectPosition, 2.5f, v3EffectLightColor, o);

                Vector(1.0f, 0.8f, 0.1f, v3EffectLightColor);
                if (rand_fps_check(1))
                {
                    b->TransformPosition(o->BoneTransform[57], p, v3EffectPosition, true);
                    CreateEffect(MODEL_EFFECT_FIRE_HIK3_MONO, v3EffectPosition, o->Angle, v3EffectLightColor, 1, NULL, -1, 0, 0, 0, fEffectScale);

                    b->TransformPosition(o->BoneTransform[60], p, v3EffectPosition, true);
                    CreateEffect(MODEL_EFFECT_FIRE_HIK3_MONO, v3EffectPosition, o->Angle, v3EffectLightColor, 1, NULL, -1, 0, 0, 0, fEffectScale);

                    b->TransformPosition(o->BoneTransform[66], p, v3EffectPosition, true);
                    CreateEffect(MODEL_EFFECT_FIRE_HIK3_MONO, v3EffectPosition, o->Angle, v3EffectLightColor, 1, NULL, -1, 0, 0, 0, fEffectScale);

                    b->TransformPosition(o->BoneTransform[78], p, v3EffectPosition, true);
                    CreateEffect(MODEL_EFFECT_FIRE_HIK3_MONO, v3EffectPosition, o->Angle, v3EffectLightColor, 1, NULL, -1, 0, 0, 0, fEffectScale);

                    b->TransformPosition(o->BoneTransform[91], p, v3EffectPosition, true);
                    CreateEffect(MODEL_EFFECT_FIRE_HIK3_MONO, v3EffectPosition, o->Angle, v3EffectLightColor, 1, NULL, -1, 0, 0, 0, fEffectScale);
                }
            }
            RenderPartObjectBodyColor(&Models[o->Type], o, o->Type, o->Alpha, RENDER_METAL | RENDER_BRIGHT, Bright);
        }

        if (c->MonsterIndex == MONSTER_METAL_BALROG)
        {
            RenderPartObjectBodyColor(&Models[o->Type], o, o->Type, o->Alpha, RENDER_CHROME | RENDER_BRIGHT | RENDER_EXTRA, Bright);
        }
        else
        {
            RenderPartObjectBodyColor(&Models[o->Type], o, o->Type, o->Alpha, RENDER_CHROME | RENDER_BRIGHT, Bright);
        }

        if (c->MonsterIndex >= MONSTER_GOLDEN_DARK_KNIGHT && c->MonsterIndex <= MONSTER_GOLDEN_RABBIT)
        {
            VectorCopy(vBackupBodyLight, Models[o->Type].BodyLight);
        }
    }
    else if (c->MonsterIndex == MONSTER_ALQUAMOS)
    {
        float Luminosity = (float)(rand() % 30 + 70) * 0.01f;
        Vector(Luminosity * 0.8f, Luminosity * 0.9f, Luminosity * 1.f, Light);

        for (int i = 0; i < 9; ++i)
        {
            b->TransformPosition(o->BoneTransform[g_chStar[i]], p, Position, true);
            CreateSprite(BITMAP_LIGHT, Position, 0.6f, Light, o);
        }

        Vector(Luminosity * 0.6f, Luminosity * 0.7f, Luminosity * 0.8f, Light);

        if (rand_fps_check(1))
        {
            for (int i = 0; i < 3; i++)
            {
                Vector((float)(rand() % 20 - 10), (float)(rand() % 20 - 10), (float)(rand() % 20 - 10), p);
                b->TransformPosition(o->BoneTransform[rand() % b->NumBones], p, Position, true);
                CreateParticle(BITMAP_SPARK + 1, Position, o->Angle, Light, 3);
            }
        }
    }
    else if (c->MonsterIndex == MONSTER_QUEEN_RAINER)
    {
        b->TransformPosition(o->BoneTransform[20], p, Position, true);
        CreateSprite(BITMAP_LIGHT, Position, 0.8f, Light, o);
    }
    else if (c->MonsterIndex == MONSTER_MEGA_CRUST || c->MonsterIndex == MONSTER_ALPHA_CRUST || c->MonsterIndex == MONSTER_OMEGA_WING)
    {
        if (!c->Object.m_pCloth)
        {
            int iTex = (c->MonsterIndex == MONSTER_MEGA_CRUST) ? BITMAP_ROBE + 3 : BITMAP_ROBE + 5;
            auto* pCloth = new CPhysicsCloth[1];
            pCloth[0].Create(&(c->Object), 19, 0.0f, 10.0f, 0.0f, 5, 15, 30.0f, 300.0f, iTex, iTex, PCT_RUBBER | PCT_MASK_ALPHA);
            c->Object.m_pCloth = (void*)pCloth;
            c->Object.m_byNumCloth = 1;
        }
        auto* pCloth = (CPhysicsCloth*)c->Object.m_pCloth;
        if (!pCloth[0].Move2(0.005f, 5))
        {
            DeleteCloth(c, o);
        }
        else
        {
            pCloth[0].Render();
        }
    }
    else if (c->MonsterIndex == MONSTER_DRAKAN || c->MonsterIndex == MONSTER_GREAT_DRAKAN)
    {
        vec3_t pos1, pos2;
        switch (c->MonsterIndex)
        {
        case MONSTER_DRAKAN:
            Vector(0.1f, 0.1f, 1.f, Light);

            for (int i = 13; i < 27; ++i)
            {
                b->TransformPosition(o->BoneTransform[i], p, Position, true);
                CreateSprite(BITMAP_LIGHT, Position, 0.8f, Light, o);

                VectorCopy(Position, pos2);
                if ((i >= 14 && i <= 16 || i == 23) && rand_fps_check(1))
                {
                    CreateJoint(BITMAP_JOINT_THUNDER, pos1, pos2, o->Angle, 7, NULL, 20.f);
                }
                VectorCopy(Position, pos1);
            }

            for (int i = 52; i < 59; ++i)
            {
                b->TransformPosition(o->BoneTransform[i], p, Position, true);
                CreateSprite(BITMAP_LIGHT, Position, 0.8f, Light, o);
            }
            break;
        case MONSTER_GREAT_DRAKAN:
            Vector(1.f, 1.f, 1.0f, Light);

            if (rand_fps_check(1))
            {
                for (int i = 18; i < 19; ++i)
                {
                    Vector(0.f, 0.f, 0.f, p);
                    b->TransformPosition(o->BoneTransform[i], p, Position, true);
                    CreateParticle(BITMAP_FIRE, Position, o->Angle, Light, 0, 0.3f);
                }
            }
            break;
        }

        int RenderType = (c->MonsterIndex == MONSTER_DRAKAN) ? 0 : RENDER_EXTRA;
        RenderPartObjectBodyColor(&Models[o->Type], o, o->Type, o->Alpha, RENDER_CHROME | RENDER_BRIGHT | RenderType, 1.f);

        if (c->MonsterIndex == MONSTER_DRAKAN)
        {
            RenderPartObjectBodyColor2(&Models[o->Type], o, o->Type, o->Alpha, RENDER_CHROME2 | RENDER_LIGHTMAP | RENDER_BRIGHT, 1.f);
        }
    }
    else if (c->MonsterIndex == MONSTER_DARK_PHOENIX)
    {
        float fSin = 0.5f * (1.0f + sinf((float)((int)WorldTime % 10000) * 0.001f));
        RenderPartObjectBodyColor(&Models[o->Type], o, o->Type, o->Alpha, RENDER_CHROME | RENDER_BRIGHT, 0.3f + fSin * 0.7f);
        fSin = 0.3f * (1.0f - fSin) + 0.3f;
        o->BlendMeshLight = fSin;
        o->BlendMesh = 0;
        RenderObject(o, Translate, 2, 0);
        RenderObject(o, Translate, 3, 0);
        o->BlendMesh = -1;
        memcpy(g_fBoneSave[2], o->BoneTransform[24], 3 * 4 * sizeof(float));
        o->Type++;
        RenderObject(o, Translate, Select, 0);
        memcpy(g_fBoneSave[0], o->BoneTransform[23], 3 * 4 * sizeof(float));
        memcpy(g_fBoneSave[1], o->BoneTransform[14], 3 * 4 * sizeof(float));

        if (!c->Object.m_pCloth)
        {
            auto* pCloth = new CPhysicsCloth[1];
            pCloth[0].Create(o, 10, 0.0f, -10.0f, 0.0f, 5, 12, 15.0f, 240.0f, BITMAP_PHO_R_HAIR, BITMAP_PHO_R_HAIR, PCT_RUBBER | PCT_MASK_ALPHA);
            pCloth[0].AddCollisionSphere(0.0f, 0.0f, 40.0f, 30.0f, 10);
            o->m_pCloth = (void*)pCloth;
            o->m_byNumCloth = 1;
        }
        auto* pCloth = (CPhysicsCloth*)c->Object.m_pCloth;
        if (!pCloth[0].Move2(0.005f, 5))
        {
            DeleteCloth(c, o);
        }
        else
        {
            pCloth[0].Render();
        }
        o->Type--;
    }

    if (c->MonsterIndex == MONSTER_GOLDEN_TITAN || c->MonsterIndex == MONSTER_GOLDEN_SOLDIER)
    {
        RenderPartObjectBodyColor(&Models[o->Type], o, o->Type, o->Alpha, RENDER_METAL | RENDER_BRIGHT, 1.f, BITMAP_SHINY + 1);
    }

    if (c->MonsterIndex == MONSTER_RED_DRAGON)
    {
        PART_t* w = &c->Wing;
        w->Type = MODEL_BOSS_HEAD;
        w->LinkBone = 9;
        w->CurrentAction = 1;
        w->PriorAction = 1;
        w->PlaySpeed = 0.2f;
        RenderLinkObject(0.f, 0.f, -40.f, c, w, w->Type, 0, 0, false, Translate);

        w->Type = MODEL_PRINCESS;
        w->LinkBone = 61;
        float TempScale = o->Scale;
        vec3_t TempLight;
        VectorCopy(c->Light, TempLight);
        Vector(1.f, 1.f, 1.f, c->Light);
        o->Scale = 0.9f;
        RenderLinkObject(0.f, -40.f, 45.f, c, w, w->Type, 0, 0, false, Translate);
        VectorCopy(TempLight, c->Light);
        o->Scale = TempScale;
    }
    else if (c->MonsterIndex >= MONSTER_STATUE_OF_SAINT_1 && c->MonsterIndex <= MONSTER_STATUE_OF_SAINT_3)
    {
        PART_t* w = &c->Wing;
        w->LinkBone = 1;
        w->CurrentAction = 1;
        w->PriorAction = 1;
        w->PlaySpeed = 0.2f;
        float TempScale = o->Scale;
        o->Scale = 0.7f;

        if (c->MonsterIndex == MONSTER_STATUE_OF_SAINT_1) w->Type = MODEL_DIVINE_STAFF_OF_ARCHANGEL;
        if (c->MonsterIndex == MONSTER_STATUE_OF_SAINT_2) w->Type = MODEL_DIVINE_SWORD_OF_ARCHANGEL;
        if (c->MonsterIndex == MONSTER_STATUE_OF_SAINT_3)
        {
            w->Type = MODEL_DIVINE_CB_OF_ARCHANGEL;
            o->Scale = 0.9f;
        }

        RenderLinkObject(0.f, 0.f, 0.f, c, w, w->Type, 0, 0, true, true);
        o->Scale = TempScale;
    }
    else if (c->MonsterIndex == MONSTER_WHITE_WIZARD)
    {
        RenderPartObjectBodyColor(&Models[o->Type], o, o->Type, o->Alpha, RENDER_BRIGHT | RENDER_EXTRA, 1.0f);
    }

    if (o->Type == MODEL_LUNAR_RABBIT)
    {
        vec3_t vLight;
        vec3_t vPos, vRelatedPos;
        Vector(0.3f, 0.3f, 0.0f, vLight);
        Vector(0.f, 0.f, 0.f, vRelatedPos);

        if (o->CurrentAction == MONSTER01_WALK)
        {
            o->m_iAnimation++;
            if (rand_fps_check(20))
                PlayBuffer(SOUND_MOONRABBIT_WALK);
        }

        if (o->CurrentAction == MONSTER01_SHOCK)
        {
            if (o->AnimationFrame > 2.f && o->AnimationFrame <= 3.f && rand_fps_check(1))
                PlayBuffer(SOUND_MOONRABBIT_DAMAGE);
        }

        if (o->CurrentAction == MONSTER01_DIE)
        {
            if (o->AnimationFrame > 1.f && o->AnimationFrame <= 2.f && rand_fps_check(1))
                PlayBuffer(SOUND_MOONRABBIT_DEAD);

            if (o->AnimationFrame > 9.f)
            {
                if (o->SubType != -1)
                {
                    Vector(0.5f, 0.5f, 0.0f, vLight);
                    BoneManager::GetBonePosition(o, L"Rabbit_1", vPos);

                    Vector(0.7f, 1.0f, 0.6f, vLight);
                    vec3_t vMoonPos;
                    VectorCopy(vPos, vMoonPos);
                    vMoonPos[2] += 28.f;
                    CreateEffect(MODEL_MOONHARVEST_MOON, vMoonPos, o->Angle, vLight, 0, NULL, -1, 0, 0, 0, 0.5f);

                    Vector(0.4f, 0.4f, 0.8f, vLight);
                    CreateParticle(BITMAP_EXPLOTION_MONO, vPos, o->Angle, vLight, 10, 1.0f);

                    Vector(1.0f, 1.0f, 1.0f, vLight);
                    for (int i = 0; i < 200; i++)
                    {
                        CreateParticle(BITMAP_SPARK + 1, vPos, o->Angle, vLight, 21);
                    }

                    for (int i = 0; i < 150; i++)
                    {
                        CreateParticle(BITMAP_SPARK + 1, vPos, o->Angle, vLight, 22);
                    }
                    Vector(1.0f, 1.0f, 1.0f, vLight);
                    switch (o->SubType)
                    {
                    case 0:
                    {
                        for (int i = 0; i < 10; i++)
                        {
                            CreateEffect(MODEL_MOONHARVEST_GAM, vPos, o->Angle, vLight);
                        }
                    }break;
                    case 1:
                    {
                        for (int i = 0; i < 5; i++)
                        {
                            CreateEffect(MODEL_MOONHARVEST_SONGPUEN1, vPos, o->Angle, vLight);
                            CreateEffect(MODEL_MOONHARVEST_SONGPUEN2, vPos, o->Angle, vLight);
                        }
                    }
                    break;
                    case 2:
                    {
                        for (int i = 0; i < 10; i++)
                        {
                            CreateEffect(MODEL_NEWYEARSDAY_EVENT_BEKSULKI, vPos, o->Angle, vLight);
                        }
                    }
                    break;
                    }

                    o->SubType = -1;

                    if (o->AnimationFrame <= 10.f)
                    {
                        // flare
                        Vector(1.0f, 0.0f, 0.0f, vLight);
                        CreateSprite(BITMAP_LIGHT, vPos, 8.0f, vLight, o);
                    }
                }
            }
        }
        else
        {
            BoneManager::GetBonePosition(o, L"Rabbit_1", vPos);		// Bip01 Spine
            Vector(0.4f, 0.4f, 0.9f, vLight);
            CreateSprite(BITMAP_LIGHT, vPos, 3.0f, vLight, o);	// flare01.jpg

            BoneManager::GetBonePosition(o, L"Rabbit_2", vPos);		// Bip01 Head
            Vector(0.4f, 0.4f, 0.9f, vLight);
            CreateSprite(BITMAP_LIGHT, vPos, 2.0f, vLight, o);	// flare01.jpg

            BoneManager::GetBonePosition(o, L"Rabbit_4", vPos);		// Bip01 Pelvis
            Vector(0.4f, 0.4f, 0.9f, vLight);
            CreateSprite(BITMAP_LIGHT, vPos, 1.0f, vLight, o);	// flare01.jpg
        }
    }

    if (o->Type == MODEL_NPC_CHERRYBLOSSOM)
    {
        vec3_t vRelativePos, vtaWorldPos, vLight, vLight1, vLight2;
        BMD* b = &Models[MODEL_NPC_CHERRYBLOSSOM];

        Vector(0.f, 0.f, 0.f, vRelativePos);
        Vector(1.f, 0.6f, 0.8f, vLight);
        Vector(0.3f, 0.3f, 0.3f, vLight1);
        Vector(1.f, 1.f, 1.f, vLight2);

        float Scale = absf(sinf(WorldTime * 0.002f)) * 0.2f;
        float Luminosity = absf(sinf(WorldTime * 0.002f)) * 0.4f;
        Vector(0.5f + Luminosity, 0.0f + Luminosity, 0.0f + Luminosity, Light);

        if (rand_fps_check(2))
        {
            VectorCopy(o->Position, b->BodyOrigin);
            b->Animation(BoneTransform, o->AnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);
            b->TransformPosition(BoneTransform[43], vRelativePos, vtaWorldPos, false);
            vtaWorldPos[2] += 20.f;

            CreateParticle(BITMAP_CHERRYBLOSSOM_EVENT_PETAL, vtaWorldPos, o->Angle, rand_fps_check(3) ? vLight : vLight1, 1, 0.3f);
        }

        VectorCopy(o->Position, b->BodyOrigin);
        b->Animation(BoneTransform, o->AnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);
        b->TransformPosition(BoneTransform[43], vRelativePos, vtaWorldPos, false);
        vtaWorldPos[2] += 20.f;

        if (rand_fps_check(1))
        {
            CreateParticle(BITMAP_SPARK + 1, vtaWorldPos, o->Angle, rand_fps_check(3) ? vLight2 : vLight1, 25, Scale + 0.2f);
            CreateParticle(BITMAP_SPARK + 1, vtaWorldPos, o->Angle, rand_fps_check(2) ? vLight2 : vLight1, 25, Scale + 0.3f);
        }

        Vector(0.7f, 0.5f, 0.2f, vLight);
        CreateSprite(BITMAP_LIGHT, vtaWorldPos, 2.f, vLight, o, 0.f);

        Vector(0.7f, 0.2f, 0.6f, vLight);
        VectorCopy(o->Position, b->BodyOrigin);
        b->Animation(BoneTransform, o->AnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);
        b->TransformPosition(BoneTransform[3], vRelativePos, vtaWorldPos, false);
        CreateSprite(BITMAP_LIGHT, vtaWorldPos, 5.f, vLight, o, 0.f);

        int iRedFlarePos[] = { 53, 56, 59, 62 };
        for (int i = 0; i < 4; ++i) {
            Vector(0.9f, 0.4f, 0.8f, vLight);
            VectorCopy(o->Position, b->BodyOrigin);
            b->Animation(BoneTransform, o->AnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);
            b->TransformPosition(BoneTransform[iRedFlarePos[i]], vRelativePos, vtaWorldPos, false);

            CreateSprite(BITMAP_LIGHT, vtaWorldPos, 1.5f, vLight, o, 0.f);

            if (rand_fps_check(3))
            {
                auto randpos = (float)(rand() % 30 + 5);

                if (rand_fps_check(2)) {
                    vtaWorldPos[0] += randpos;
                }
                else {
                    vtaWorldPos[0] -= randpos;
                }

                Vector(1.f, 0.8f, 0.4f, vLight);
                CreateParticle(BITMAP_SPARK + 1, vtaWorldPos, o->Angle, vLight, 15, Scale + 0.4f);
            }
        }
    }

    bool fullset = false;
    if (o->Type == MODEL_PLAYER)
    {
        if (c->HideShadow == false)
        {
            fullset = CheckFullSet(c);
        }
    }

    bool bCloak = false;

    if ((c->Class == CLASS_DARK || gCharacterManager.GetBaseClass(c->Class) == CLASS_DARK_LORD
        || gCharacterManager.GetBaseClass(c->Class) == CLASS_RAGEFIGHTER) && o->Type == MODEL_PLAYER)
    {
        if (c->Change == false || (c->Change == true && c->Object.Type == MODEL_PLAYER))
        {
            bCloak = true;
        }
    }

    if (c->MonsterIndex == MONSTER_DEATH_KING || c->MonsterIndex == MONSTER_NIGHTMARE)
    {
        bCloak = true;
    }

    if (c->MonsterIndex >= MONSTER_TERRIBLE_BUTCHER && c->MonsterIndex <= MONSTER_DOPPELGANGER_SUM)
    {
        bCloak = false;
    }

    if (gMapManager.InChaosCastle() == true)
    {
        bCloak = false;
    }

    vec3_t CloakLight;
    Vector(1.f, 1.f, 1.f, CloakLight);
    if (c->GuildMarkIndex != -1)
    {
        if (EnableSoccer)
        {
            if (wcscmp(GuildMark[Hero->GuildMarkIndex].GuildName, GuildMark[c->GuildMarkIndex].GuildName) == NULL)
            {
                bCloak = true;
                if (HeroSoccerTeam == 0)
                {
                    Vector(1.f, 0.2f, 0.f, CloakLight);
                }
                else
                {
                    Vector(0.f, 0.2f, 1.f, CloakLight);
                }
            }
            if (wcscmp(GuildWarName, GuildMark[c->GuildMarkIndex].GuildName) == NULL)
            {
                bCloak = true;
                if (HeroSoccerTeam == 0)
                {
                    Vector(0.f, 0.2f, 1.f, CloakLight);
                }
                else
                {
                    Vector(1.f, 0.2f, 0.f, CloakLight);
                }
            }
        }
        if (SoccerObserver)
        {
            if (wcscmp(SoccerTeamName[0], GuildMark[c->GuildMarkIndex].GuildName) == NULL)
            {
                bCloak = true;
                Vector(1.f, 0.2f, 0.f, CloakLight);
            }
            if (wcscmp(SoccerTeamName[1], GuildMark[c->GuildMarkIndex].GuildName) == NULL)
            {
                bCloak = true;
                Vector(0.f, 0.2f, 1.f, CloakLight);
            }
        }
    }

    if (g_DuelMgr.IsDuelEnabled())
    {
        if (g_DuelMgr.IsDuelPlayer(c, DUEL_ENEMY, FALSE))
        {
            bCloak = true;
            Vector(1.f, 0.2f, 0.f, CloakLight);
        }
        else if (g_DuelMgr.IsDuelPlayer(c, DUEL_HERO, FALSE))
        {
            bCloak = true;
            Vector(0.f, 0.2f, 1.f, CloakLight);
        }
    }

    if (SceneFlag == CHARACTER_SCENE)
    {
        Vector(0.4f, 0.4f, 0.4f, Light);
    }
    else
    {
        RequestTerrainLight(o->Position[0], o->Position[1], Light);
    }

    VectorAdd(Light, o->Light, c->Light);
    if (o->Type == MODEL_DARK_PHEONIX_SHIELD)
    {
        Vector(.6f, .6f, .6f, c->Light);
    }

    else if (c->MonsterIndex >= MONSTER_TERRIBLE_BUTCHER && c->MonsterIndex <= MONSTER_DOPPELGANGER_SUM)
    {
        c->HideShadow = true;

        if (gMapManager.WorldActive == WD_65DOPPLEGANGER1)
        {
            Vector(0.5f, 0.7f, 1.0f, c->Light);
            c->Object.Alpha = 0.7f;
        }
        else
        {
            Vector(1.0f, 0.3f, 0.1f, c->Light);
        }

        if (gMapManager.WorldActive == WD_65DOPPLEGANGER1);
        else
            if (o->CurrentAction != PLAYER_DIE1)
            {
                // 			if (!g_isCharacterBuff((&c->Object), eBuff_Doppelganger_Ascension))
                // 			{
                c->Object.Alpha = 1.0f;
                // 			}
            }
            else
            {
                c->Object.Alpha -= 0.07f;
                float fAlpha = c->Object.Alpha;
                if (fAlpha < 0) fAlpha = 0;

                Vector(1.0f * fAlpha, 0.3f * fAlpha, 0.1f * fAlpha, c->Light);

                // 			vec3_t vLight;
                vec3_t vPos;
                BMD* b = &Models[o->Type];

                //Vector(0.5f, 0.5f, 0.5f, vLight);

                int iNumBones = Models[o->Type].NumBones;
                if (rand_fps_check(1))
                {
                    Models[o->Type].TransformByObjectBone(vPos, o, rand() % iNumBones);
                    CreateParticle(BITMAP_TWINTAIL_WATER, vPos, o->Angle, c->Light, 2, 0.5f);
                }
            }
        c->Object.BlendMesh = -1;

        if (g_isCharacterBuff((&c->Object), eBuff_Doppelganger_Ascension))
        {
            o->Position[2] += 2.0f;
        }
    }

    if (c == Hero)
    {
        vec3_t AbilityLight = { 1.f, 1.f, 1.f };
        if (CharacterAttribute->Ability & ABILITY_FAST_ATTACK_SPEED)
        {
            AbilityLight[0] *= 0.9f; AbilityLight[1] *= 0.5f; AbilityLight[2] *= 0.5f;
        }
        if (CharacterAttribute->Ability & ABILITY_PLUS_DAMAGE)
        {
            AbilityLight[0] *= 0.5f; AbilityLight[1] *= 0.9f; AbilityLight[2] *= 0.5f;
        }
        if (CharacterAttribute->Ability & ABILITY_FAST_ATTACK_RING)
        {
            AbilityLight[0] *= 0.9f; AbilityLight[1] *= 0.5f; AbilityLight[2] *= 0.5f;
        }
        if (CharacterAttribute->Ability & ABILITY_FAST_ATTACK_SPEED2)
        {
            AbilityLight[0] *= 0.9f; AbilityLight[1] *= 0.5f; AbilityLight[2] *= 0.5f;
        }
        if (SceneFlag == CHARACTER_SCENE)
        {
            Vector(0.5f, 0.5f, 0.5f, Light);
            VectorAdd(Light, o->Light, c->Light);
        }
        else
            VectorCopy(AbilityLight, c->Light);
    }

    if (o->Kind == KIND_NPC && gMapManager.WorldActive == WD_0LORENCIA && o->Type == MODEL_PLAYER && (o->SubType >= MODEL_SKELETON1 && o->SubType <= MODEL_SKELETON3))
    {
        RenderPartObject(&c->Object, o->SubType, NULL, c->Light, o->Alpha, c->Level, 0, 0, false, false, Translate, Select);
    }
    else if (o->Kind == KIND_PLAYER && o->Type == MODEL_PLAYER && gCharacterManager.GetBaseClass(c->Class) == CLASS_RAGEFIGHTER
        && (o->SubType == MODEL_SKELETON_PCBANG || o->SubType == MODEL_HALLOWEEN
            || o->SubType == MODEL_PANDA || o->SubType == MODEL_SKELETON_CHANGED
            || o->SubType == MODEL_XMAS_EVENT_CHANGE_GIRL || o->SubType == MODEL_SKELETON1))
    {
        PART_t* p = &c->BodyPart[o->SubType];
        if (o->CurrentAction == PLAYER_SKILL_DRAGONKICK)
        {
            p->Type = o->SubType;
            RenderCharacter_AfterImage(c, p, Translate, Select, 2.5f, 1.0f);
        }
        else if (o->CurrentAction == PLAYER_SKILL_DARKSIDE_READY || o->CurrentAction == PLAYER_SKILL_DARKSIDE_ATTACK)
        {
            OBJECT* pObj = &c->Object;
            if (pObj->m_sTargetIndex < 0 || c->JumpTime > 0)
            {
                RenderPartObject(&c->Object, o->SubType, p, c->Light, o->Alpha, 0, 0, 0, false, false, Translate, Select);
            }
            else
            {
                p->Type = o->SubType;
                g_CMonkSystem.DarksideRendering(c, p, Translate, Select);
            }
        }
        else
        {
            RenderPartObject(&c->Object, o->SubType, NULL, c->Light, o->Alpha, 0, 0, 0, false, false, Translate, Select);
        }

        if (o->SubType == MODEL_XMAS_EVENT_CHANGE_GIRL)
        {
            if (c->m_pTempParts == NULL)
            {
                c->m_pTempParts = (CSIPartsMDL*)new CSParts(MODEL_XMAS_EVENT_EARRING, 20);
                auto* pParts = (CSIPartsMDL*)c->m_pTempParts;
                OBJECT* pObj = pParts->GetObject();
                pObj->Velocity = 0.25f;
                pObj->Owner = &c->Object;
            }
            else
            {
                RenderParts(c);
            }
        }
    }
    else if (o->Kind == KIND_PLAYER && o->Type == MODEL_PLAYER && o->SubType == MODEL_SKELETON_PCBANG)
    {
        RenderPartObject(&c->Object, o->SubType, NULL, c->Light, o->Alpha, 0, 0, 0, false, false, Translate, Select);
    }
    else if (o->Kind == KIND_PLAYER && o->Type == MODEL_PLAYER && o->SubType == MODEL_HALLOWEEN)
    {
        RenderPartObject(&c->Object, o->SubType, NULL, c->Light, o->Alpha, 0, 0, 0, false, false, Translate, Select);
    }
    else if (o->Kind == KIND_PLAYER && o->Type == MODEL_PLAYER && o->SubType == MODEL_CURSEDTEMPLE_ALLIED_PLAYER)
    {
        RenderPartObject(&c->Object, o->SubType, NULL, c->Light, o->Alpha, 0, 0, 0, false, false, Translate, Select);
    }
    else if (o->Kind == KIND_PLAYER && o->Type == MODEL_PLAYER && o->SubType == MODEL_CURSEDTEMPLE_ILLUSION_PLAYER)
    {
        RenderPartObject(&c->Object, o->SubType, NULL, c->Light, o->Alpha, 0, 0, 0, false, false, Translate, Select);
    }
    else if (o->Kind == KIND_PLAYER && o->Type == MODEL_PLAYER && o->SubType == MODEL_PANDA)
    {
        RenderPartObject(&c->Object, o->SubType, NULL, c->Light, o->Alpha, 0, 0, 0, false, false, Translate, Select);
    }
    else if (o->Kind == KIND_PLAYER && o->Type == MODEL_PLAYER && o->SubType == MODEL_SKELETON_CHANGED)
    {
        RenderPartObject(&c->Object, o->SubType, NULL, c->Light, o->Alpha, 0, 0, 0, false, false, Translate, Select);
    }
    else if (o->Kind == KIND_PLAYER && o->Type == MODEL_PLAYER && o->SubType == MODEL_XMAS_EVENT_CHANGE_GIRL)
    {
        RenderPartObject(&c->Object, o->SubType, NULL, c->Light, o->Alpha, 0, 0, 0, false, false, Translate, Select);

        if (c->m_pTempParts == NULL)
        {
            c->m_pTempParts = (CSIPartsMDL*)new CSParts(MODEL_XMAS_EVENT_EARRING, 20);
            auto* pParts = (CSIPartsMDL*)c->m_pTempParts;
            OBJECT* pObj = pParts->GetObject();
            pObj->Velocity = 0.25f;
            pObj->Owner = &c->Object;
        }
        else
        {
            auto* pParts = (CSIPartsMDL*)c->m_pTempParts;
            OBJECT* pObj = pParts->GetObject();

            RenderParts(c);
        }
    }
    else if (o->Kind == KIND_PLAYER && o->Type == MODEL_PLAYER && o->SubType == MODEL_GM_CHARACTER)
    {
        RenderPartObject(&c->Object, o->SubType, NULL, c->Light, o->Alpha, 0, 0, 0, false, false, Translate, Select);

        if (!g_isCharacterBuff(o, eBuff_Cloaking))
        {
            vec3_t vLight, vPos;
            Vector(0.4f, 0.6f, 0.8f, vLight);
            VectorCopy(o->Position, vPos);
            vPos[2] += 100.f;
            CreateSprite(BITMAP_LIGHT, vPos, 6.0f, vLight, o, 0.5f);

            float fLumi;
            fLumi = sinf(WorldTime * 0.05f) * 0.4f + 0.9f;
            Vector(fLumi * 0.3f, fLumi * 0.5f, fLumi * 0.8f, vLight);
            CreateSprite(BITMAP_LIGHT, vPos, 2.0f, vLight, o);

            Vector(0.3f, 0.2f, 1.f, vLight);
            RenderAurora(BITMAP_MAGIC + 1, RENDER_BRIGHT, o->Position[0], o->Position[1], 2.5f, 2.5f, vLight);

            Vector(1.0f, 1.0f, 1.f, vLight);
            fLumi = sinf(WorldTime * 0.0015f) * 0.3f + 0.5f;
            EnableAlphaBlend();
            Vector(fLumi * vLight[0], fLumi * vLight[1], fLumi * vLight[2], vLight);
            RenderTerrainAlphaBitmap(BITMAP_GM_AURORA, o->Position[0], o->Position[1], 1.5f, 1.5f, vLight, WorldTime * 0.01f);
            RenderTerrainAlphaBitmap(BITMAP_GM_AURORA, o->Position[0], o->Position[1], 1.f, 1.f, vLight, -WorldTime * 0.01f);
            //RenderAurora ( BITMAP_GM_AURORA, RENDER_BRIGHT, o->Position[0], o->Position[1], 1.5f, 1.5f, vLight );

            if (c->Object.m_pCloth == NULL)
            {
                int iNumCloth = 2;

                auto* pCloth = new CPhysicsCloth[iNumCloth];

                pCloth[0].Create(o, 20, 0.0f, 5.0f, 10.0f, 6, 5, 30.0f, 70.0f, BITMAP_GM_HAIR_1, BITMAP_GM_HAIR_1, PCT_COTTON | PCT_SHORT_SHOULDER | PCT_MASK_ALPHA);
                pCloth[0].SetWindMinMax(10, 20);
                pCloth[0].AddCollisionSphere(-10.f, 20.0f, 20.0f, 27.0f, 17);
                pCloth[0].AddCollisionSphere(10.f, 20.0f, 20.0f, 27.0f, 17);

                pCloth[1].Create(o, 20, 0.0f, 5.0f, 25.0f, 4, 4, 30.0f, 40.0f, BITMAP_GM_HAIR_3, BITMAP_GM_HAIR_3, PCT_COTTON | PCT_SHORT_SHOULDER | PCT_MASK_ALPHA);
                pCloth[1].SetWindMinMax(8, 15);
                pCloth[1].AddCollisionSphere(-10.f, 20.0f, 20.0f, 27.0f, 17);
                pCloth[1].AddCollisionSphere(10.f, 20.0f, 20.0f, 27.0f, 17);

                o->m_pCloth = (void*)pCloth;
                o->m_byNumCloth = iNumCloth;
            }

            auto* pCloth = (CPhysicsCloth*)c->Object.m_pCloth;

            if (g_isCharacterBuff(o, eBuff_Cloaking))
            {
                for (int i = 0; i < o->m_byNumCloth; ++i)
                {
                    if (pCloth[i].Move2(0.005f, 5) == false)
                    {
                        DeleteCloth(c, o);
                    }
                    else
                    {
                        vec3_t vLight;
                        Vector(1.f, 1.f, 1.f, vLight);
                        pCloth[i].Render(&vLight);
                    }
                }
            }

            DisableAlphaBlend();
        }
    }
    else if (o->Type == MODEL_PLAYER
        && (o->SubType == MODEL_XMAS_EVENT_CHA_SSANTA
            || o->SubType == MODEL_XMAS_EVENT_CHA_SNOWMAN
            || o->SubType == MODEL_XMAS_EVENT_CHA_DEER))
    {
        vec3_t vOriginPos;
        VectorCopy(o->Position, vOriginPos);
        o->Position[0] += 50.f;
        o->Position[1] += 50.f;
        o->Position[2] = RequestTerrainHeight(o->Position[0], o->Position[1]);
        RenderPartObject(&c->Object, o->SubType, NULL, c->Light, o->Alpha, 0, 0, 0, false, false, Translate, Select);

        VectorCopy(vOriginPos, o->Position);
        o->Position[0] -= 50.f;
        o->Position[1] -= 50.f;
        o->Position[2] = RequestTerrainHeight(o->Position[0], o->Position[1]);
        RenderPartObject(&c->Object, o->SubType, NULL, c->Light, o->Alpha, 0, 0, 0, false, false, Translate, Select);

        VectorCopy(vOriginPos, o->Position);
        o->Position[0] += 50.f;
        o->Position[1] -= 50.f;
        o->Position[2] = RequestTerrainHeight(o->Position[0], o->Position[1]);
        RenderPartObject(&c->Object, o->SubType, NULL, c->Light, o->Alpha, 0, 0, 0, false, false, Translate, Select);
    }
    else if (o->Type == MODEL_PLAYER && (o->SubType >= MODEL_SKELETON1 && o->SubType <= MODEL_SKELETON3))
    {
        RenderPartObject(&c->Object, o->SubType, NULL, c->Light, o->Alpha, 0, 0, 0, false, false, Translate, Select);
    }
    else if (!c->Change)
    {
#ifndef GUILD_WAR_EVENT
        if ((o->Kind == KIND_PLAYER && gMapManager.InChaosCastle() == true))
        {
            int RenderType = RENDER_TEXTURE;
            PART_t* p = &c->BodyPart[BODYPART_HEAD];
            if (c == Hero)
            {
                RenderType |= RENDER_CHROME;
            }

            if ((o->CurrentAction == PLAYER_SKILL_DRAGONKICK) && (gCharacterManager.GetBaseClass(c->Class) == CLASS_RAGEFIGHTER))
            {
                p->Type = MODEL_ANGEL;
                RenderCharacter_AfterImage(c, p, Translate, Select, 2.5f, 1.0f);
            }
            else if ((gCharacterManager.GetBaseClass(c->Class) == CLASS_RAGEFIGHTER))
            {
                OBJECT* pObj = &c->Object;
                if (pObj->m_sTargetIndex < 0 || c->JumpTime > 0)
                {
                    RenderPartObject(&c->Object, MODEL_ANGEL, p, c->Light, o->Alpha, 0, 0, 0, false, false, Translate, Select, RenderType);
                }
                else
                {
                    p->Type = MODEL_ANGEL;
                    g_CMonkSystem.DarksideRendering(c, p, Translate, Select);
                }
            }
            else
            {
                RenderPartObject(&c->Object, MODEL_ANGEL, p, c->Light, o->Alpha, 0, 0, 0, false, false, Translate, Select, RenderType);
            }
        }
        else
#endif// GUILD_WAR_EVENT
        {
            for (int i = MAX_BODYPART - 1; i >= 0; i--)
            {
                PART_t* p = &c->BodyPart[i];
                if (p->Type != -1)
                {
                    int Type = p->Type;

                    if (CLASS_SUMMONER == gCharacterManager.GetBaseClass(c->Class))
                    {
                        int nItemType = (Type - MODEL_ITEM) / MAX_ITEM_INDEX;
                        int nItemSubType = (Type - MODEL_ITEM) % MAX_ITEM_INDEX;

                        if (nItemType >= 7 && nItemType <= 11
                            && (nItemSubType == 10 || nItemSubType == 11))
                        {
                            Type = MODEL_HELM2 + (nItemType - 7) * MODEL_ITEM_COMMON_NUM + nItemSubType - 10;
                        }
                    }
                    else if (CLASS_RAGEFIGHTER == gCharacterManager.GetBaseClass(c->Class))
                    {
                        Type = g_CMonkSystem.ModifyTypeCommonItemMonk(Type);
                    }

                    BMD* b = &Models[Type];
                    
                    if (CLASS_RAGEFIGHTER == gCharacterManager.GetBaseClass(c->Class))
                    {
                        b->Skin = gCharacterManager.GetBaseClass(c->Class) * 2 + gCharacterManager.IsThirdClass(c->Class);
                    }
                    else
                    {
                        b->Skin = gCharacterManager.GetBaseClass(c->Class) * 2 + gCharacterManager.IsSecondClass(c->Class);
                    }

                    if (gCharacterManager.GetBaseClass(c->Class) == CLASS_DARK_LORD && i == BODYPART_HELM)
                    {
                        o->BlendMeshLight = sinf(WorldTime * 0.001f) * 0.1f + 0.7f;
                        if (i == BODYPART_HELM)
                        {
                            int index = Type - MODEL_HELM;
                            if (index == 0 || index == 5 || index == 6 || index == 8 || index == 9)
                            {
                                Type = MODEL_MASK_HELM + index;
                                Models[Type].Skin = b->Skin;
                            }
                        }
                    }

                    if (c->MonsterIndex >= MONSTER_TERRIBLE_BUTCHER && c->MonsterIndex <= MONSTER_DOPPELGANGER_SUM)
                    {
                        if (gMapManager.WorldActive == WD_65DOPPLEGANGER1)
                            RenderPartObject(&c->Object, Type, p, c->Light, o->Alpha, p->Level, p->ExcellentFlags, p->AncientDiscriminator, false, false, Translate,
                                Select, RENDER_DOPPELGANGER | RENDER_TEXTURE);
                        else
                            RenderPartObject(&c->Object, Type, p, c->Light, o->Alpha, p->Level, p->ExcellentFlags, p->AncientDiscriminator, false, false, Translate,
                                Select, RENDER_DOPPELGANGER | RENDER_BRIGHT | RENDER_TEXTURE);
                        // 						RenderPartObject(&c->Object,Type,p,c->Light,o->Alpha,p->Level<<3,p->Option1,p->ExtOption,false,false,Translate,
                        // 							Select,RENDER_DOPPELGANGER|RENDER_BRIGHT|RENDER_CHROME);
                    }
                    else
                    {
                        if ((o->CurrentAction == PLAYER_SKILL_DRAGONKICK) && (gCharacterManager.GetBaseClass(c->Class) == CLASS_RAGEFIGHTER))
                        {
                            RenderCharacter_AfterImage(c, p, Translate, Select, 2.5f, 1.0f);
                        }
                        else
                            if ((o->CurrentAction == PLAYER_SKILL_DARKSIDE_READY) && (gCharacterManager.GetBaseClass(c->Class) == CLASS_RAGEFIGHTER))
                            {
                                if (o->m_sTargetIndex < 0 || c->JumpTime>0)
                                {
                                    RenderPartObject(&c->Object, Type, p, c->Light, o->Alpha, p->Level, p->ExcellentFlags, p->AncientDiscriminator, false, false, Translate, Select);
                                }
                                else
                                {
                                    g_CMonkSystem.DarksideRendering(c, p, Translate, Select);
                                }
                            }
                            else
                            {
                                RenderPartObject(&c->Object, Type, p, c->Light, o->Alpha, p->Level, p->ExcellentFlags, p->AncientDiscriminator, false, false, Translate, Select);
                            }
                    }
                }
            }
        }

        if (gMapManager.InChaosCastle() == false)
        {
            if (c->GuildMarkIndex >= 0 && o->Type == MODEL_PLAYER && o->Alpha != 0.0f
                && (c->GuildMarkIndex == MARK_EDIT || g_GuildCache.IsExistGuildMark(GuildMark[c->GuildMarkIndex].Key) == TRUE)
                && (!g_isCharacterBuff(o, eBuff_Cloaking)))
            {
                CreateGuildMark(c->GuildMarkIndex);
                if (gCharacterManager.GetBaseClass(c->Class) == CLASS_RAGEFIGHTER)
                {
                    vec3_t vPos;
                    if (c->BodyPart[BODYPART_ARMOR].Type == MODEL_SACRED_ARMOR
                        || c->BodyPart[BODYPART_ARMOR].Type == MODEL_STORM_HARD_ARMOR
                        || c->BodyPart[BODYPART_ARMOR].Type == MODEL_PIERCING_ARMOR
                        || c->BodyPart[BODYPART_ARMOR].Type == MODEL_PHOENIX_SOUL_ARMOR)
                    {
                        Vector(5.0f, 0.0f, -35.0f, vPos);
                    }
                    else
                    {
                        Vector(5.0f, 0.0f, -21.0f, vPos);
                    }
                    RenderGuild(o, c->BodyPart[BODYPART_ARMOR].Type, vPos);
                }
                else
                {
                    RenderGuild(o, c->BodyPart[BODYPART_ARMOR].Type);
                }
            }
        }
    }

    if (bCloak)
    {
        if (!c->Object.m_pCloth)
        {
            if (gCharacterManager.GetBaseClass(c->Class) == CLASS_DARK_LORD)
            {
                int numCloth = 4;
                if (c->Wing.Type == MODEL_CAPE_OF_EMPEROR)
                {
                    numCloth = 6;
                }
                else
                {
                    numCloth = 4;
                }

                auto* pCloth = new CPhysicsCloth[numCloth];

                pCloth[0].Create(o, 20, 0.0f, 0.0f, 20.0f, 6, 5, 30.0f, 70.0f, BITMAP_ROBE + 6, BITMAP_ROBE + 6, PCT_CURVED | PCT_RUBBER2 | PCT_MASK_LIGHT | PLS_STRICTDISTANCE | PCT_SHORT_SHOULDER | PCT_NORMAL_THICKNESS | PCT_OPT_HAIR);
                pCloth[0].SetWindMinMax(10, 50);
                pCloth[0].AddCollisionSphere(-10.f, 20.0f, 20.0f, 27.0f, 17);
                pCloth[0].AddCollisionSphere(10.f, 20.0f, 20.0f, 27.0f, 17);

                pCloth[1].Create(o, 20, 0.0f, 5.0f, 18.0f, 5, 5, 30.0f, 70.0f, BITMAP_ROBE + 6, BITMAP_ROBE + 6, PCT_CURVED | PCT_RUBBER2 | PCT_MASK_BLEND | PLS_STRICTDISTANCE | PCT_SHORT_SHOULDER | PCT_NORMAL_THICKNESS | PCT_OPT_HAIR);
                pCloth[1].SetWindMinMax(8, 40);
                pCloth[1].AddCollisionSphere(-10.f, 20.0f, 20.0f, 27.0f, 17);
                pCloth[1].AddCollisionSphere(10.f, 20.0f, 20.0f, 27.0f, 17);

                if (c->Wing.Type == MODEL_CAPE_OF_EMPEROR)
                {
                    pCloth[2].Create(o, 19, 0.0f, 8.0f, 10.0f, 10, 10, 180.0f, 180.0f, BITMAP_ROBE + 9, BITMAP_ROBE + 9, PCT_CURVED | PCT_SHORT_SHOULDER | PCT_HEAVY | PCT_MASK_ALPHA);
                    pCloth[2].AddCollisionSphere(-10.f, -10.0f, -10.0f, 25.0f, 17);
                    pCloth[2].AddCollisionSphere(10.f, -10.0f, -10.0f, 25.0f, 17);
                    pCloth[2].AddCollisionSphere(-10.f, -10.0f, 20.0f, 27.0f, 17);
                    pCloth[2].AddCollisionSphere(10.f, -10.0f, 20.0f, 27.0f, 17);
                }
                else if (c->Wing.Type == MODEL_WING + 130)
                {
                    pCloth[2].Create(o, 19, 0.0f, 8.0f, 10.0f, 10, 10, 100.0f, 100.0f, BITMAP_ROBE + 7, BITMAP_ROBE + 7, PCT_CURVED | PCT_SHORT_SHOULDER | PCT_MASK_ALPHA);
                    pCloth[2].AddCollisionSphere(-10.f, -10.0f, -10.0f, 25.0f, 17);
                    pCloth[2].AddCollisionSphere(10.f, -10.0f, -10.0f, 25.0f, 17);
                    pCloth[2].AddCollisionSphere(-10.f, -10.0f, 20.0f, 27.0f, 17);
                    pCloth[2].AddCollisionSphere(10.f, -10.0f, 20.0f, 27.0f, 17);
                }
                else
                {
                    //
                    pCloth[2].Create(o, 19, 0.0f, 8.0f, 10.0f, 10, 10, 180.0f, 180.0f, BITMAP_ROBE + 7, BITMAP_ROBE + 7, PCT_CURVED | PCT_SHORT_SHOULDER | PCT_MASK_ALPHA);
                    pCloth[2].AddCollisionSphere(-10.f, -10.0f, -10.0f, 25.0f, 17);
                    pCloth[2].AddCollisionSphere(10.f, -10.0f, -10.0f, 25.0f, 17);
                    pCloth[2].AddCollisionSphere(-10.f, -10.0f, 20.0f, 27.0f, 17);
                    pCloth[2].AddCollisionSphere(10.f, -10.0f, 20.0f, 27.0f, 17);
                }

                if (c->Wing.Type == MODEL_CAPE_OF_EMPEROR)
                {
                    numCloth = 6;
                }
                else
                {
                    numCloth = 3;
                }

                if (c->BodyPart[BODYPART_ARMOR].Type == MODEL_BODY_ARMOR + SKIN_CLASS_DARK_LORD)
                {
                    pCloth[3].Create(o, 18, 0.0f, 10.0f, -5.0f, 5, 5, 50.0f, 90.0f, BITMAP_DARK_LOAD_SKIRT, BITMAP_DARK_LOAD_SKIRT, PCT_MASK_ALPHA | PCT_HEAVY | PCT_STICKED | PCT_SHORT_SHOULDER);
                    pCloth[3].AddCollisionSphere(0.0f, -15.0f, -20.0f, 30.0f, 2);
                }
                else if (c->BodyPart[BODYPART_ARMOR].Type == MODEL_BODY_ARMOR + SKIN_CLASS_LORDEMPEROR)
                {
                    pCloth[3].Create(o, 18, 0.0f, 10.0f, -5.0f, 5, 5, 50.0f, 90.0f, BITMAP_DARKLOAD_SKIRT_3RD, BITMAP_DARKLOAD_SKIRT_3RD, PCT_MASK_ALPHA | PCT_HEAVY | PCT_STICKED | PCT_SHORT_SHOULDER);
                    pCloth[3].AddCollisionSphere(0.0f, -15.0f, -20.0f, 30.0f, 2);
                }

                if (c->Wing.Type == MODEL_CAPE_OF_EMPEROR)
                {
                    pCloth[4].Create(o, 19, 30.0f, 15.0f, 10.0f, 2, 5, 12.0f, 200.0f, BITMAP_ROBE + 10, BITMAP_ROBE + 10, PCT_FLAT | PCT_SHAPE_NORMAL | PCT_COTTON | PCT_MASK_ALPHA);
                    pCloth[4].AddCollisionSphere(0.0f, -15.0f, -20.0f, 30.0f, 2);
                    pCloth[4].AddCollisionSphere(0.f, 0.0f, 0.0f, 35.0f, 17);

                    pCloth[5].Create(o, 19, -30.0f, 20.0f, 10.0f, 2, 5, 12.0f, 200.0f, BITMAP_ROBE + 10, BITMAP_ROBE + 10, PCT_FLAT | PCT_SHAPE_NORMAL | PCT_COTTON | PCT_MASK_ALPHA);
                    pCloth[5].AddCollisionSphere(0.0f, -15.0f, -20.0f, 30.0f, 2);
                    pCloth[5].AddCollisionSphere(0.f, 0.0f, 0.0f, 35.0f, 17);
                }

                o->m_pCloth = (void*)pCloth;
                o->m_byNumCloth = numCloth;
            }
            else if (gCharacterManager.GetBaseClass(c->Class) == CLASS_RAGEFIGHTER)
            {
                int numCloth = (c->Wing.Type == MODEL_CAPE_OF_OVERRULE) ? 3 : 1;

                auto* pCloth = new CPhysicsCloth[numCloth];

                if (c->Wing.Type == MODEL_CAPE_OF_OVERRULE)
                {
                    pCloth[0].Create(o, 19, 0.0f, 15.0f, 5.0f, 10, 10, 180.0f, 170.0f, BITMAP_MANTOE, BITMAP_MANTOE, PCT_CURVED | PCT_SHORT_SHOULDER | PCT_HEAVY | PCT_MASK_ALPHA);
                    pCloth[0].AddCollisionSphere(-10.f, -10.0f, -10.0f, 35.0f, 17);
                    pCloth[0].AddCollisionSphere(10.f, -10.0f, -10.0f, 35.0f, 17);
                    pCloth[0].AddCollisionSphere(-10.f, -10.0f, 20.0f, 37.0f, 17);
                    pCloth[0].AddCollisionSphere(10.f, -10.0f, 20.0f, 37.0f, 17);
                }
                else if (c->Wing.Type == MODEL_WING + 135)
                {
                    pCloth[0].Create(o, 19, 0.0f, 15.0f, 5.0f, 10, 10, 150.0f, 130.0f, BITMAP_NCCAPE, BITMAP_NCCAPE, PCT_CURVED | PCT_SHORT_SHOULDER | PCT_HEAVY | PCT_MASK_ALPHA);
                    pCloth[0].AddCollisionSphere(-10.f, -10.0f, -10.0f, 35.0f, 17);
                    pCloth[0].AddCollisionSphere(10.f, -10.0f, -10.0f, 35.0f, 17);
                    pCloth[0].AddCollisionSphere(-10.f, -10.0f, 20.0f, 37.0f, 17);
                    pCloth[0].AddCollisionSphere(10.f, -10.0f, 20.0f, 37.0f, 17);
                }
                else
                {
                    pCloth[0].Create(o, 19, 0.0f, 15.0f, 5.0f, 10, 10, 180.0f, 170.0f, BITMAP_NCCAPE, BITMAP_NCCAPE, PCT_CURVED | PCT_SHORT_SHOULDER | PCT_HEAVY | PCT_MASK_ALPHA);
                    pCloth[0].AddCollisionSphere(-10.f, -10.0f, -10.0f, 35.0f, 17);
                    pCloth[0].AddCollisionSphere(10.f, -10.0f, -10.0f, 35.0f, 17);
                    pCloth[0].AddCollisionSphere(-10.f, -10.0f, 20.0f, 37.0f, 17);
                    pCloth[0].AddCollisionSphere(10.f, -10.0f, 20.0f, 37.0f, 17);
                }

                if (c->Wing.Type == MODEL_CAPE_OF_OVERRULE)
                {
                    pCloth[1].Create(o, 19, 25.0f, 15.0f, 2.0f, 2, 5, 12.0f, 180.0f, BITMAP_MANTO01, BITMAP_MANTO01, PCT_FLAT | PCT_SHAPE_NORMAL | PCT_COTTON | PCT_ELASTIC_RAGE_L | PCT_MASK_ALPHA);
                    pCloth[1].AddCollisionSphere(0.0f, -15.0f, -20.0f, 35.0f, 2);
                    pCloth[1].AddCollisionSphere(0.0f, 0.0f, 0.0f, 45.0f, 17);

                    pCloth[2].Create(o, 19, -25.0f, 15.0f, 2.0f, 2, 5, 12.0f, 180.0f, BITMAP_MANTO01, BITMAP_MANTO01, PCT_FLAT | PCT_SHAPE_NORMAL | PCT_COTTON | PCT_ELASTIC_RAGE_R | PCT_MASK_ALPHA);
                    pCloth[2].AddCollisionSphere(0.0f, -15.0f, -20.0f, 35.0f, 2);
                    pCloth[2].AddCollisionSphere(0.0f, 0.0f, 0.0f, 50.0f, 17);
                }

                o->m_pCloth = (void*)pCloth;
                o->m_byNumCloth = numCloth;
            }
            else
            {
                auto* pCloth = new CPhysicsCloth[1];

                if (c->MonsterIndex == MONSTER_DEATH_KING)
                {
                    pCloth[0].Create(o, 19, 0.0f, 10.0f, 0.0f, 10, 10, 55.0f, 140.0f, BITMAP_ROBE + 2, BITMAP_ROBE + 2, PCT_CURVED | PCT_MASK_ALPHA);
                    pCloth[0].AddCollisionSphere(-10.f, -10.0f, -10.0f, 35.0f, 17);
                    pCloth[0].AddCollisionSphere(10.f, -10.0f, -10.0f, 35.0f, 17);
                    pCloth[0].AddCollisionSphere(0.f, -10.0f, -40.0f, 50.0f, 19);
                }
                else if (c->MonsterIndex == MONSTER_NIGHTMARE)
                {
                    pCloth[0].Create(o, 2, 0.0f, 0.0f, 0.0f, 6, 6, 180.0f, 100.0f, BITMAP_NIGHTMARE_ROBE, BITMAP_NIGHTMARE_ROBE, PCT_CYLINDER | PCT_MASK_ALPHA);
                }
                else
                {
                    if (c->Wing.Type == MODEL_WING_OF_RUIN)
                    {
                        pCloth[0].Create(o, 19, 0.0f, 15.0f, 0.0f, 10, 10, 120.0f, 120.0f, BITMAP_ROBE + 8, BITMAP_ROBE + 8, PCT_CURVED | PCT_SHORT_SHOULDER | PCT_HEAVY | PCT_MASK_ALPHA);
                    }
                    else
                    {
                        pCloth[0].Create(o, 19, 0.0f, 10.0f, 0.0f, 10, 10, /*45.0f*/75.0f, 120.0f, BITMAP_ROBE, BITMAP_ROBE + 1, PCT_CURVED | PCT_SHORT_SHOULDER | PCT_HEAVY);
                    }
                }
                o->m_pCloth = (void*)pCloth;
                o->m_byNumCloth = 1;
            }
        }
        else if (gCharacterManager.GetBaseClass(c->Class) == CLASS_DARK_LORD)
        {
            if ((c->BodyPart[BODYPART_ARMOR].Type != MODEL_BODY_ARMOR + SKIN_CLASS_DARK_LORD
                && c->BodyPart[BODYPART_ARMOR].Type != MODEL_BODY_ARMOR + SKIN_CLASS_LORDEMPEROR
                ) && o->m_byNumCloth == 4)
            {
                if (o && o->m_pCloth)
                {
                    auto* pCloth = (CPhysicsCloth*)o->m_pCloth;
                    pCloth[3].Destroy();

                    o->m_byNumCloth = 3;
                }
            }
            else if (c->BodyPart[BODYPART_ARMOR].Type == MODEL_BODY_ARMOR + CLASS_DARK_LORD && o->m_byNumCloth == 3)
            {
                if (o && o->m_pCloth)
                {
                    auto* pCloth = (CPhysicsCloth*)o->m_pCloth;

                    pCloth[3].Create(o, 18, 0.0f, 8.0f, -5.0f, 5, 5, 50.0f, 90.0f, BITMAP_DARK_LOAD_SKIRT, BITMAP_DARK_LOAD_SKIRT, PCT_MASK_ALPHA | PCT_HEAVY | PCT_STICKED | PCT_CURVED | PCT_SHORT_SHOULDER);
                    pCloth[3].AddCollisionSphere(0.0f, -15.0f, -20.0f, 30.0f, 2);
                    o->m_byNumCloth = 4;
                }
            }
            else if (c->BodyPart[BODYPART_ARMOR].Type == MODEL_BODY_ARMOR + SKIN_CLASS_LORDEMPEROR && o->m_byNumCloth == 3)
            {
                if (o && o->m_pCloth)
                {
                    auto* pCloth = (CPhysicsCloth*)o->m_pCloth;

                    pCloth[3].Create(o, 18, 0.0f, 8.0f, -5.0f, 5, 5, 50.0f, 90.0f, BITMAP_DARKLOAD_SKIRT_3RD, BITMAP_DARKLOAD_SKIRT_3RD, PCT_MASK_ALPHA | PCT_HEAVY | PCT_STICKED | PCT_CURVED | PCT_SHORT_SHOULDER);
                    pCloth[3].AddCollisionSphere(0.0f, -15.0f, -20.0f, 30.0f, 2);
                    o->m_byNumCloth = 4;
                }
            }
            else if (c->BodyPart[BODYPART_ARMOR].Type != MODEL_BODY_ARMOR + SKIN_CLASS_DARK_LORD && o->m_byNumCloth == 6)
            {
                if (o && o->m_pCloth)
                {
                    auto* pCloth = (CPhysicsCloth*)o->m_pCloth;
                    if (pCloth[3].GetOwner() != NULL)
                        pCloth[3].Destroy();
                }
            }
            else if (c->BodyPart[BODYPART_ARMOR].Type == MODEL_BODY_ARMOR + SKIN_CLASS_DARK_LORD && o->m_byNumCloth == 6)
            {
                if (o && o->m_pCloth)
                {
                    auto* pCloth = (CPhysicsCloth*)o->m_pCloth;
                    if (pCloth[3].GetOwner() == NULL)
                    {
                        pCloth[3].Create(o, 18, 0.0f, 8.0f, -5.0f, 5, 5, 50.0f, 90.0f, BITMAP_DARK_LOAD_SKIRT, BITMAP_DARK_LOAD_SKIRT, PCT_MASK_ALPHA | PCT_HEAVY | PCT_STICKED | PCT_CURVED | PCT_SHORT_SHOULDER);
                        pCloth[3].AddCollisionSphere(0.0f, -15.0f, -20.0f, 30.0f, 2);
                    }
                }
            }
            else if (c->BodyPart[BODYPART_ARMOR].Type == MODEL_BODY_ARMOR + SKIN_CLASS_LORDEMPEROR && o->m_byNumCloth == 6)
            {
                if (o && o->m_pCloth)
                {
                    auto* pCloth = (CPhysicsCloth*)o->m_pCloth;
                    if (pCloth[3].GetOwner() == NULL)
                    {
                        pCloth[3].Create(o, 18, 0.0f, 8.0f, -5.0f, 5, 5, 50.0f, 90.0f, BITMAP_DARKLOAD_SKIRT_3RD, BITMAP_DARKLOAD_SKIRT_3RD, PCT_MASK_ALPHA | PCT_HEAVY | PCT_STICKED | PCT_CURVED | PCT_SHORT_SHOULDER);
                        pCloth[3].AddCollisionSphere(0.0f, -15.0f, -20.0f, 30.0f, 2);
                    }
                }
            }
        }
        auto* pCloth = (CPhysicsCloth*)c->Object.m_pCloth;

        if (!g_isCharacterBuff(o, eBuff_Cloaking))
        {
            for (int i = 0; i < o->m_byNumCloth; i++)
            {
                if (gCharacterManager.GetBaseClass(c->Class) == CLASS_DARK_LORD)
                {
                    if (i == 2 && ((c->Wing.Type != MODEL_CAPE_OF_LORD && c->Wing.Type != MODEL_CAPE_OF_EMPEROR && c->Wing.Type != MODEL_WING + 130) && (CloakLight[0] == 1.f && CloakLight[1] == 1.f && CloakLight[2] == 1.f)))
                    {
                        continue;
                    }
                    if ((i >= 0 && i <= 1) && g_ChangeRingMgr->CheckDarkLordHair(c->Object.SubType) == true)
                    {
                        continue;
                    }
                    if (i == 3 && g_ChangeRingMgr->CheckDarkLordHair(c->Object.SubType) == true)
                    {
                        continue;
                    }
                }
                if (gCharacterManager.GetBaseClass(c->Class) == CLASS_RAGEFIGHTER)
                {
                    if (i == 0 && ((c->Wing.Type != MODEL_CAPE_OF_FIGHTER
                        && c->Wing.Type != MODEL_CAPE_OF_OVERRULE
                        && c->Wing.Type != MODEL_WING + 135)
                        && (CloakLight[0] == 1.f && CloakLight[1] == 1.f && CloakLight[2] == 1.f)))
                    {
                        continue;
                    }
                }
                if (i == 0 && g_ChangeRingMgr->CheckDarkCloak(c->Class, c->Object.SubType) == true)
                {
                    continue;
                }

                if (c->Object.SubType == MODEL_CURSEDTEMPLE_ALLIED_PLAYER || c->Object.SubType == MODEL_CURSEDTEMPLE_ILLUSION_PLAYER)
                {
                    continue;
                }

                float Flag = 0.005f;
                if (g_isCharacterBuff(o, eDeBuff_Stun)
                    || g_isCharacterBuff(o, eDeBuff_Sleep))
                {
                    Flag = 0.0f;
                }

                if (pCloth[i].GetOwner() == NULL)
                {
                    continue;
                }
                else
                    if (!pCloth[i].Move2(Flag, 5))
                    {
                        DeleteCloth(c, o);
                    }
                    else
                    {
                        pCloth[i].Render(&CloakLight);
                    }
            }
        } // if( !g_isCharacterBuff(o, eBuff_Cloaking) )
    } //if ( bCloak )

    float Luminosity = (float)(rand() % 30 + 70) * 0.01f;
    if (c->PK >= PVP_MURDERER2)
    {
        Vector(1.f, 0.1f, 0.1f, c->Light);
    }
    else
    {
        VectorAdd(Light, o->Light, c->Light);

        int nCastle = BLOODCASTLE_NUM + (gMapManager.WorldActive - WD_11BLOODCASTLE_END);
        if (nCastle > 0 && nCastle <= BLOODCASTLE_NUM)		//. 블러드 캐슬일경우
        {
            if ((c->MonsterIndex >= MONSTER_DARK_SKULL_SOLDIER_1 && c->MonsterIndex <= MONSTER_MAGIC_SKELETON_1) ||
                (c->MonsterIndex >= MONSTER_DARK_SKULL_SOLDIER_2 && c->MonsterIndex <= MONSTER_MAGIC_SKELETON_2) ||
                (c->MonsterIndex == MONSTER_GIANT_OGRE_3 && c->MonsterIndex == MONSTER_RED_SKELETON_KNIGHT_3 && c->MonsterIndex == MONSTER_MAGIC_SKELETON_3) ||
                (c->MonsterIndex >= MONSTER_DARK_SKULL_SOLDIER_4 && c->MonsterIndex <= MONSTER_MAGIC_SKELETON_4) ||
                (c->MonsterIndex >= MONSTER_DARK_SKULL_SOLDIER_5 && c->MonsterIndex <= MONSTER_MAGIC_SKELETON_5) ||
                (c->MonsterIndex >= MONSTER_DARK_SKULL_SOLDIER_6 && c->MonsterIndex <= MONSTER_MAGIC_SKELETON_6) ||
                (c->MonsterIndex >= MONSTER_CHIEF_SKELETON_WARRIOR_7 && c->MonsterIndex <= MONSTER_MAGIC_SKELETON_7))
            {
                int level = nCastle / 3;
                if (level)
                    Vector(level * 0.5f, 0.1f, 0.1f, c->Light);
            }
        }
    }

    if (!gMapManager.InChaosCastle() && !(gMapManager.IsCursedTemple() && !c->SafeZone))
    {
        NextGradeObjectRender(c);
    }

    for (int i = 0; i < 2; i++) {
        if (g_CMonkSystem.EqualItemModelType(c->Weapon[i].Type) == MODEL_PHOENIX_SOUL_STAR) {
            g_CMonkSystem.RenderPhoenixGloves(c, i);
        }
    }

    bool Bind = false;
    Bind = RenderCharacterBackItem(c, o, Translate);

    if (!Bind)
    {
        if (!g_isCharacterBuff(o, eBuff_Cloaking))
        {
            constexpr float CritDamageEffectInterval = 1200.0f;
            if (g_isCharacterBuff(o, eBuff_AddCriticalDamage) && o->Kind == KIND_PLAYER && o->Type == MODEL_PLAYER && (c->LastCritDamageEffect < WorldTime - CritDamageEffectInterval))
            {
                c->LastCritDamageEffect = WorldTime;
                bool    renderSkillWave = (rand() % 20) ? true : false;
                short   weaponType = -1;
                Vector(0.f, 0.f, 0.f, p);
                Vector(1.f, 0.6f, 0.3f, Light);
                if (c->Weapon[0].Type != -1 && c->Weapon[0].Type != MODEL_ARROWS)//&& ( c->Weapon[0].Type<MODEL_SHIELD || c->Weapon[0].Type>=MODEL_SHIELD+MAX_ITEM_INDEX ) )
                {
                    b->TransformPosition(o->BoneTransform[c->Weapon[0].LinkBone], p, Position, true);
                    if (c->Weapon[0].Type >= MODEL_BOW && c->Weapon[0].Type < MODEL_BOW + MAX_ITEM_INDEX)
                    {
                        weaponType = 1;
                    }
                    CreateEffect(BITMAP_FLARE_FORCE, Position, o->Angle, o->Light, 1, o, weaponType, c->Weapon[0].LinkBone);
                    if (renderSkillWave == false)
                    {
                        CreateEffect(MODEL_DARKLORD_SKILL, Position, o->Angle, Light, 0);
                    }
                }
                if (c->Weapon[1].Type != -1 && c->Weapon[1].Type != MODEL_BOLT && (c->Weapon[1].Type < MODEL_SHIELD || c->Weapon[1].Type >= MODEL_SHIELD + MAX_ITEM_INDEX))
                {
                    b->TransformPosition(o->BoneTransform[c->Weapon[1].LinkBone], p, Position, true);
                    if (c->Weapon[1].Type >= MODEL_BOW && c->Weapon[1].Type < MODEL_BOW + MAX_ITEM_INDEX)
                    {
                        weaponType = 1;
                    }
                    CreateEffect(BITMAP_FLARE_FORCE, Position, o->Angle, o->Light, 1, o, weaponType, c->Weapon[1].LinkBone);
                    if (renderSkillWave == false)
                    {
                        CreateEffect(MODEL_DARKLORD_SKILL, Position, o->Angle, Light, 1);
                    }
                }

                PlayBuffer(SOUND_CRITICAL, o);
            }
        }

        c->PostMoveProcess_Process();

        for (int i = 0; i < 2; i++)
        {
            if (i == 0)
            {
                if (o->CurrentAction == PLAYER_ATTACK_SKILL_FURY_STRIKE && o->AnimationFrame <= 4.f)
                {
                    continue;
                }

                if (true == c->PostMoveProcess_IsProcessing())
                {
                    c->Skill = 0;

                    continue;
                }
            }

            PART_t* w = &c->Weapon[i];
            if (w->Type != -1 && w->Type != MODEL_BOLT && w->Type != MODEL_ARROWS && w->Type != MODEL_DARK_RAVEN_ITEM)
            {
                if (o->CurrentAction == PLAYER_ATTACK_BOW || o->CurrentAction == PLAYER_ATTACK_CROSSBOW || o->CurrentAction == PLAYER_ATTACK_FLY_BOW || o->CurrentAction == PLAYER_ATTACK_FLY_CROSSBOW)
                {
                    if (w->Type == MODEL_STINGER_BOW)
                    {
                        w->CurrentAction = 1;
                    }
                    else
                    {
                        w->CurrentAction = 0;
                    }
                    w->PlaySpeed = Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_BOW].PlaySpeed;
                }
                else if (w->Type == MODEL_FLAIL)
                {
                    if (o->CurrentAction >= PLAYER_ATTACK_SWORD_RIGHT1 && o->CurrentAction <= PLAYER_ATTACK_SWORD_RIGHT2)
                    {
                        w->CurrentAction = 2;
                        w->PlaySpeed = Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_SWORD_RIGHT1].PlaySpeed;
                    }
                    else
                    {
                        w->CurrentAction = 1;
                        w->PlaySpeed = Models[MODEL_PLAYER].Actions[PLAYER_STOP_MALE].PlaySpeed;
                    }
                }
                else if (w->Type == MODEL_CRYSTAL_SWORD)
                {
                    w->CurrentAction = 0;
                    w->PlaySpeed = Models[MODEL_PLAYER].Actions[PLAYER_STOP_MALE].PlaySpeed * 2.f;
                }
                else if (w->Type == MODEL_STAFF_OF_RESURRECTION)
                {
                    w->CurrentAction = 0;
                    w->PlaySpeed = Models[MODEL_PLAYER].Actions[PLAYER_STOP_MALE].PlaySpeed * 15.f;
                }
                else if (w->Type == MODEL_PHOENIX_SOUL_STAR) {
                    if (w->AnimationFrame < 2.f) {
                        w->CurrentAction = 0;
                        w->PlaySpeed = Models[MODEL_PLAYER].Actions[PLAYER_STOP_RAGEFIGHTER].PlaySpeed * 1.5f;
                        PlayBuffer(SOUND_EMPIREGUARDIAN_DEFENDER_ATTACK02);
                    }
                    else
                    {
                        w->AnimationFrame = 2.f;
                    }
                }
                else if (w->Type >= MODEL_SWORD && w->Type < MODEL_SWORD + MAX_ITEM_INDEX)
                {
                    w->CurrentAction = 0;
                    w->PlaySpeed = Models[MODEL_PLAYER].Actions[PLAYER_STOP_MALE].PlaySpeed;
                }
                else if (w->Type == MODEL_STINGER_BOW)
                {
                    w->CurrentAction = 0;
                    w->PlaySpeed = Models[MODEL_PLAYER].Actions[PLAYER_STOP_MALE].PlaySpeed;
                }
                else if ((w->Type == MODEL_FROST_MACE) || (w->Type == MODEL_ABSOLUTE_SCEPTER))
                {
                    w->CurrentAction = 0;
                    w->PlaySpeed = Models[MODEL_PLAYER].Actions[PLAYER_STOP_MALE].PlaySpeed;
                }
                else if (w->Type == MODEL_RAVEN_STICK)
                {
                    w->CurrentAction = 0;
                    w->PlaySpeed = Models[MODEL_PLAYER].Actions[PLAYER_STOP_MALE].PlaySpeed;
                }
                else
                {
                    w->CurrentAction = 0;
                    w->PlaySpeed = 0.f;
                    w->AnimationFrame = 0.001f;
                    w->PriorAnimationFrame = 0.f;
                }

                if (g_CMonkSystem.IsSwordformGloves(w->Type))
                    g_CMonkSystem.RenderSwordformGloves(c, w->Type, i, o->Alpha, Translate, Select);
                else
                    RenderLinkObject(0.f, 0.f, 0.f, c, w, w->Type, w->Level, w->ExcellentFlags, false, Translate);

                if (w->Level >= 7)
                {
                    Vector(Luminosity * 0.5f, Luminosity * 0.4f, Luminosity * 0.3f, Light);
                }
                else if (w->Level >= 5)
                {
                    Vector(Luminosity * 0.3f, Luminosity * 0.3f, Luminosity * 0.5f, Light);
                }
                else if (w->Level >= 3)
                {
                    Vector(Luminosity * 0.5f, Luminosity * 0.3f, Luminosity * 0.3f, Light);
                }
                else
                {
                    Vector(Luminosity * 0.3f, Luminosity * 0.3f, Luminosity * 0.3f, Light);
                }
                float Scale;
                if (c->PK < PVP_MURDERER2 && c->Level != 4)
                {
                    bool Success = true;

                    switch (w->Type)
                    {
                    case MODEL_SWORD_OF_ASSASSIN:
                    case MODEL_BLADE:
                    case MODEL_DOUBLE_BLADE:
                        Vector(0.f, -110.f, 5.f, Position);
                        break;
                    case MODEL_SERPENT_SWORD:
                    case MODEL_SWORD_OF_SALAMANDER:
                        Vector(0.f, -110.f, -5.f, Position);
                        break;
                    case MODEL_FALCHION:
                    case MODEL_LIGHT_SABER:
                        Vector(0.f, -110.f, 0.f, Position);
                        break;
                    case MODEL_LIGHTING_SWORD:
                    case MODEL_LEGENDARY_SWORD:
                        Vector(0.f, -150.f, 0.f, Position);
                        break;
                    case MODEL_HELIACAL_SWORD:
                        Vector(Luminosity, Luminosity, Luminosity, Light);
                        Vector(0.f, -160.f, 0.f, Position);
                        break;
                    case MODEL_DARK_BREAKER:
                        Success = false;
                        Scale = sinf(WorldTime * 0.004f) * 10.f + 20.f;
                        {
                            vec3_t  pos1, pos2;

                            Vector(0.f, -20.f, -40.f, Position);
                            Models[o->Type].TransformPosition(o->BoneTransform[w->LinkBone], Position, pos1, true);
                            Vector(0.f, -160.f, -10.f, Position);
                            Models[o->Type].TransformPosition(o->BoneTransform[w->LinkBone], Position, pos2, true);
                            CreateJoint(BITMAP_FLARE + 1, pos1, pos2, o->Angle, 4, o, Scale);

                            Vector(0.f, -10.f, 28.f, Position);
                            Models[o->Type].TransformPosition(o->BoneTransform[w->LinkBone], Position, pos1, true);
                            Vector(0.f, -145.f, 18.f, Position);
                            Models[o->Type].TransformPosition(o->BoneTransform[w->LinkBone], Position, pos2, true);
                            CreateJoint(BITMAP_FLARE + 1, pos1, pos2, o->Angle, 4, o, Scale);
                        }
                        break;

                    case MODEL_THUNDER_BLADE:
                    {
                        Success = false;

                        Vector(0.f, -20.f, 15.f, Position);
                        Models[o->Type].TransformPosition(o->BoneTransform[w->LinkBone], Position, p, true);

                        Scale = sinf(WorldTime * 0.004f) * 0.3f + 0.3f;
                        Vector(Scale * 0.2f, Scale * 0.2f, Scale * 1.f, Light);
                        CreateSprite(BITMAP_SHINY + 1, p, Scale + 1.f, Light, o, 0);

                        Vector(0.f, -133.f, 7.f, Position);
                        Models[o->Type].TransformPosition(o->BoneTransform[w->LinkBone], Position, Position, true);

                        Scale = (Scale * 20.f) + 20.f;
                        CreateJoint(BITMAP_JOINT_THUNDER, p, Position, o->Angle, 10, NULL, Scale);
                        CreateJoint(BITMAP_JOINT_THUNDER, p, Position, o->Angle, 10, NULL, Scale);
                        CreateJoint(BITMAP_JOINT_THUNDER, p, Position, o->Angle, 10, NULL, Scale);
                    }
                    break;
                    case MODEL_DRAGON_SOUL_STAFF:
                        Success = false;
                        Vector(0.f, -120.f, 5.f, Position);
                        Vector(Luminosity * 0.6f, Luminosity * 0.6f, Luminosity * 2.f, Light);
                        Models[o->Type].TransformPosition(o->BoneTransform[w->LinkBone], Position, p, true);
                        CreateSprite(BITMAP_SHINY + 1, p, 1.5f, Light, o);
                        CreateSprite(BITMAP_LIGHT, p, Luminosity + 1.f, Light, o);

                        Vector(0.f, 100.f, 10.f, Position);
                        Models[o->Type].TransformPosition(o->BoneTransform[w->LinkBone], Position, p, true);
                        CreateSprite(BITMAP_LIGHT, p, Luminosity + 1.f, Light, o);
                        break;
                    case MODEL_GORGON_STAFF:
                        Success = false;
                        Vector(0.f, -90.f, 0.f, Position);
                        Vector(Luminosity * 0.4f, Luminosity * 0.8f, Luminosity * 0.6f, Light);
                        Models[o->Type].TransformPosition(o->BoneTransform[w->LinkBone], Position, p, true);
                        CreateSprite(BITMAP_SHINY + 1, p, 2.f, Light, o);
                        break;
                    case MODEL_LEGENDARY_SHIELD:
                        Success = false;
                        Vector(20.f, 0.f, 0.f, Position);
                        Vector(Luminosity * 0.4f, Luminosity * 0.6f, Luminosity * 1.5f, Light);
                        Models[o->Type].TransformPosition(o->BoneTransform[w->LinkBone], Position, p, true);
                        CreateSprite(BITMAP_SHINY + 1, p, 1.5f, Light, o);
                        break;
                    case MODEL_LEGENDARY_STAFF:
                        Success = false;
                        Vector(0.f, -145.f, 0.f, Position);
                        Vector(Luminosity * 0.4f, Luminosity * 0.6f, Luminosity * 1.f, Light);
                        Models[o->Type].TransformPosition(o->BoneTransform[w->LinkBone], Position, p, true);
                        CreateSprite(BITMAP_SHINY + 1, p, 1.5f, Light, o);
                        CreateSprite(BITMAP_LIGHTNING + 1, p, 0.3f, Light, o);
                        break;
                    case MODEL_CHAOS_LIGHTNING_STAFF:
                        Success = false;
                        Vector(Luminosity * 0.4f, Luminosity * 0.6f, Luminosity * 1.f, Light);
                        RenderBrightEffect(b, BITMAP_SHINY + 1, 27, 2.f, Light, o);

                        for (int j = 28; j <= 37; j++)
                        {
                            RenderBrightEffect(b, BITMAP_LIGHT, j, 1.5f, Light, o);
                        }
                        break;
                    case MODEL_STAFF_OF_RESURRECTION:
                        Success = false;
                        Vector(0.f, -145.f, 0.f, Position);
                        Models[o->Type].TransformPosition(o->BoneTransform[w->LinkBone], Position, p, true);
                        Vector(Luminosity * 1.f, Luminosity * 0.6f, Luminosity * 0.4f, Light);
                        CreateSprite(BITMAP_SPARK, p, 3.f, Light, o);
                        CreateSprite(BITMAP_SHINY + 2, p, 1.5f, Light, o);

                        for (int j = 0; j < 4; j++)
                        {
                            Vector((float)(rand() % 20 - 10), (float)(rand() % 20 - 10 - 90.f), (float)(rand() % 20 - 10), Position);
                            Models[o->Type].TransformPosition(o->BoneTransform[w->LinkBone], Position, p, true);
                            if (rand_fps_check(1))
                            {
                                CreateParticle(BITMAP_SPARK, p, o->Angle, Light, 1);
                            }
                        }
                        Vector(Luminosity * 1.f, Luminosity * 0.2f, Luminosity * 0.1f, Light);

                        for (int j = 0; j < 10; j++)
                        {
                            if (rand() % 4 < 3)
                            {
                                Vector(0.f, -j * 20 + 60.f, 0.f, Position);
                                Models[o->Type].TransformPosition(o->BoneTransform[w->LinkBone], Position, p, true);
                                CreateSprite(BITMAP_LIGHT, p, 1.f, Light, o);
                            }
                        }
                        break;
                    case MODEL_VIOLENT_WIND_STICK:
                        Success = false;
                        Vector(Luminosity * 0.2f, Luminosity * 0.3f, Luminosity * 1.4f, Light);
                        Vector(0.f, 0.f, 0.f, Position);
                        b->TransformPosition(BoneTransform[1], Position, p, true);
                        CreateSprite(BITMAP_SHINY + 1, p, 1.f, Light, o, -(int)WorldTime * 0.1f);
                        b->TransformPosition(BoneTransform[2], Position, p, true);
                        CreateSprite(BITMAP_SHINY + 1, p, 0.5f, Light, o, (int)WorldTime * 0.1f);
                        break;
                    case MODEL_RED_WING_STICK:
                        Success = false;
                        Vector(Luminosity * 1.0f, Luminosity * 0.3f, Luminosity * 0.4f, Light);
                        Vector(0.f, 0.f, 0.f, Position);
                        b->TransformPosition(BoneTransform[1], Position, p, true);
                        CreateSprite(BITMAP_SHINY + 1, p, 1.f, Light, o, -(int)WorldTime * 0.1f);
                        CreateSprite(BITMAP_SHINY + 1, p, 1.f, Light, o, -(int)WorldTime * 0.13f);
                        break;
                    case MODEL_ANCIENT_STICK:
                        Success = false;
                        Scale = absf(sinf(WorldTime * 0.002f)) * 0.2f;
                        Luminosity = absf(sinf(WorldTime * 0.002f)) * 0.4f;
                        Vector(0.5f + Luminosity, 0.2f + Luminosity, 0.9f + Luminosity, Light);

                        for (int j = 1; j <= 4; ++j)
                            RenderBrightEffect(b, BITMAP_LIGHT, j, Scale + 1.0f, Light, o);
                        break;
                    case MODEL_DEMONIC_STICK:
                        Success = false;
                        Scale = absf(sinf(WorldTime * 0.002f)) * 0.2f;
                        Luminosity = absf(sinf(WorldTime * 0.002f)) * 0.4f;
                        Vector(0.5f + Luminosity, 0.2f + Luminosity, 0.9f + Luminosity, Light);

                        for (int j = 1; j <= 2; ++j)
                        {
                            RenderBrightEffect(b, BITMAP_SHINY + 1, j, Scale + 1.0f, Light, o);
                            RenderBrightEffect(b, BITMAP_LIGHT, j, Scale + 1.0f, Light, o);
                        }
                        Vector(0.8f + Luminosity, 0.6f + Luminosity, 0.3f + Luminosity, Light);
                        RenderBrightEffect(b, BITMAP_LIGHT, 3, Scale + 1.0f, Light, o);
                        break;
                    case MODEL_STORM_BLITZ_STICK:
                        Success = false;
                        Scale = absf(sinf(WorldTime * 0.002f)) * 0.2f;
                        Luminosity = absf(sinf(WorldTime * 0.002f)) * 0.4f;
                        Vector(0.5f + Luminosity, 0.2f + Luminosity, 0.9f + Luminosity, Light);

                        for (int j = 2; j <= 3; ++j)
                            RenderBrightEffect(b, BITMAP_LIGHT, j, Scale + 1.0f, Light, o);

                        Vector(0.8f + Luminosity, 0.6f + Luminosity, 0.3f + Luminosity, Light);
                        RenderBrightEffect(b, BITMAP_SHINY + 2, 2, Scale + 1.0f, Light, o);
                        break;
                    case MODEL_ETERNAL_WING_STICK:
                        Success = false;
                        Vector(1.0f, 0.2f, 0.1f, Light);
                        Vector(0.f, 0.f, 0.f, Position);
                        b->TransformPosition(BoneTransform[2], Position, p, true);
                        CreateSprite(BITMAP_EVENT_CLOUD, p, 0.25f, Light, o, -(int)WorldTime * 0.1f);
                        CreateSprite(BITMAP_EVENT_CLOUD, p, 0.25f, Light, o, -(int)WorldTime * 0.2f);
                        Vector(1.0f, 0.4f, 0.3f, Light);
                        RenderBrightEffect(b, BITMAP_SHINY + 1, 2, 1.0f, Light, o);
                        Vector(1.0f, 0.2f, 0.0f, Light);
                        if (rand_fps_check(1))
                        {
                            CreateParticle(BITMAP_SPARK + 1, p, o->Angle, Light, 16, 1.0f);
                            CreateParticle(BITMAP_SPARK + 1, p, o->Angle, Light, 23, 1.0f);
                        }

#ifdef ASG_ADD_ETERNALWING_STICK_EFFECT
                        if (rand_fps_check(20))
                            CreateParticle(BITMAP_SPARK + 1, p, o->Angle, Light, 20, 1.0f);
                        Vector(1.0f, 0.0f, 0.0f, Light);
                        RenderBrightEffect(b, BITMAP_LIGHT, 2, 3.0f, Light, o);
#endif	// ASG_ADD_ETERNALWING_STICK_EFFECT
                        break;
                    case MODEL_SAINT_CROSSBOW:
                        Success = false;
                        Vector(Luminosity * 0.4f, Luminosity * 0.6f, Luminosity * 1.f, Light);

                        for (int j = 0; j < 6; j++)
                        {
                            Vector(0.f, -10.f, -j * 20.f, Position);
                            Models[o->Type].TransformPosition(o->BoneTransform[w->LinkBone], Position, p, true);
                            CreateSprite(BITMAP_LIGHT, p, 2.f, Light, o);
                        }
                        break;
                    case MODEL_CRYSTAL_SWORD:
                        Success = false;
                        Vector(Luminosity * 1.f, Luminosity * 0.6f, Luminosity * 0.4f, Light);

                        for (int j = 0; j < 8; j++)
                        {
                            if (rand() % 4 < 3)
                            {
                                Vector(0.f, -j * 20 - 30.f, 0.f, Position);
                                Models[o->Type].TransformPosition(o->BoneTransform[w->LinkBone], Position, p, true);
                                CreateSprite(BITMAP_LIGHT, p, 1.f, Light, o);
                            }
                        }
                        break;
                    case MODEL_CHAOS_DRAGON_AXE:
                        Success = false;
                        Vector(0.f, -84.f, 0.f, Position);
                        Models[o->Type].TransformPosition(o->BoneTransform[w->LinkBone], Position, p, true);
                        Scale = sinf(WorldTime * 0.004f) * 0.3f + 0.7f;
                        Vector(Scale * 1.f, Scale * 0.2f, Scale * 0.1f, Light);
                        CreateSprite(BITMAP_SHINY + 1, p, Scale + 1.5f, Light, o);

                        for (int j = 0; j < 5; j++)
                        {
                            Vector(0.f, -j * 20.f - 10.f, 0.f, Position);
                            Models[o->Type].TransformPosition(o->BoneTransform[w->LinkBone], Position, p, true);
                            CreateSprite(BITMAP_SHINY + 1, p, 1.f, Light, o);
                        }
                        Vector(Scale * 0.5f, Scale * 0.1f, Scale * 0.05f, Light);
                        RenderBrightEffect(b, BITMAP_SHINY + 1, 2, 1.f, Light, o);
                        RenderBrightEffect(b, BITMAP_SHINY + 1, 6, 1.f, Light, o);
                        break;
                    case MODEL_BLUEWING_CROSSBOW:
                    case MODEL_AQUAGOLD_CROSSBOW:
                        Success = false;
                        if (w->Type == MODEL_BLUEWING_CROSSBOW)
                        {
                            Vector(Luminosity * 0.2f, Luminosity * 0.4f, Luminosity * 0.6f, Light);
                        }
                        else
                        {
                            Vector(Luminosity * 0.6f, Luminosity * 0.4f, Luminosity * 0.2f, Light);
                        }

                        for (int j = 0; j < 6; j++)
                        {
                            Vector(0.f, -20.f, (float)(-j * 20), Position);
                            Models[o->Type].TransformPosition(o->BoneTransform[w->LinkBone], Position, p, true);
                            CreateSprite(BITMAP_LIGHT, p, 2.f, Light, o);
                        }
                        break;
                    default:
                        Success = false;
                    }
                    if (Success)
                    {
                        Models[o->Type].TransformPosition(o->BoneTransform[w->LinkBone], Position, p, true);
                        CreateSprite(BITMAP_LIGHT, p, 1.4f, Light, o);
                    }
                }
            }
        }
    }

    else {
        for (int i = 0; i < 2; i++) {
            if (c->Weapon[i].Type == MODEL_PHOENIX_SOUL_STAR) {
                if (c->Weapon[i].AnimationFrame != 0.f);
                PlayBuffer(SOUND_EMPIREGUARDIAN_DEFENDER_ATTACK02);
                c->Weapon[i].CurrentAction = 0;
                c->Weapon[i].AnimationFrame = 0;
                c->Weapon[i].PriorAnimationFrame = 0;
                c->Weapon[i].PlaySpeed = 0;
            }
        }
    }

    switch (o->Type)
    {
    case MODEL_PLAYER:
        Vector(0.f, 0.f, 0.f, p);

        if (c->Class == CLASS_SOULMASTER)
        {
            if( !g_isCharacterBuff(o, eBuff_Cloaking) )
   {
      Vector(-4.f,11.f,0.f,p);
      Vector(1.f,1.f,1.f,Light);
          b->TransformPosition(o->BoneTransform[19],p,Position,true);
      //CreateSprite(BITMAP_SPARK+1,Position,0.6f,Light,NULL); //Effect Point Armor SM

      float scale = sinf(WorldTime*0.001f)*0.4f;
      //CreateSprite(BITMAP_SHINY+1,Position,scale,Light,NULL); //Effect Point Armor SM
        }
     }
        if (o->CurrentAction == PLAYER_SKILL_FLASH || o->CurrentAction == PLAYER_ATTACK_RIDE_ATTACK_FLASH || o->CurrentAction == PLAYER_FENRIR_ATTACK_DARKLORD_FLASH)
        {
            if (gCharacterManager.GetBaseClass(c->Class) == CLASS_DARK_LORD || o->CurrentAction == PLAYER_ATTACK_RIDE_ATTACK_FLASH || o->CurrentAction == PLAYER_FENRIR_ATTACK_DARKLORD_FLASH)
            {
                if (o->AnimationFrame >= 1.2f && o->AnimationFrame < 1.6f)
                {
                    vec3_t Angle;
                    Vector(1.f, 0.f, 0.f, Angle);
                    CreateEffect(BITMAP_GATHERING, o->Position, o->Angle, o->Light, 2, o);

                    PlayBuffer(SOUND_ELEC_STRIKE_READY);
                }

                if (o->AnimationFrame < 2.f)
                {
                    if (PartyNumber > 0 /*&& rand_fps_check(2)*/)
                    {
                        if (g_pPartyManager->IsPartyMemberChar(c) == false)
                            break;

                        for (int i = 0; i < PartyNumber; ++i)
                        {
                            PARTY_t* p = &Party[i];
                            if (p->index < 0) continue;

                            CHARACTER* tc = &CharactersClient[p->index];
                            if (tc != c)
                            {
                                VectorCopy(tc->Object.Position, Position);
                                Position[2] += 150.f;
                                CreateJoint(BITMAP_JOINT_ENERGY, Position, Position, o->Angle, 12, o, 20.f);
                                CreateJoint(BITMAP_JOINT_ENERGY, Position, Position, o->Angle, 13, o, 20.f);
                            }
                        }
                    }
                }

                if (o->AnimationFrame >= 7 && o->AnimationFrame < 8)
                {
                    vec3_t Angle, Light;

                    b->TransformPosition(o->BoneTransform[c->Weapon[0].LinkBone], p, Position, true);

                    Vector(0.8f, 0.5f, 1.f, Light);
                    Vector(180.f, 45.f, 0.f, Angle);
                    CreateEffect(MODEL_DARKLORD_SKILL, Position, Angle, Light, 2);

                    Vector(0.f, 0.f, o->Angle[2], Angle);
                    CreateEffect(MODEL_DARKLORD_SKILL, Position, Angle, Light, 2);
                }
            }
            else
            {
                if (g_isCharacterBuff(o, eBuff_Cloaking))
                {
                    b->TransformPosition(o->BoneTransform[c->Weapon[0].LinkBone], p, Position, true);
                    Vector(0.1f, 0.1f, 1.f, Light);
                    float Scale = o->AnimationFrame * 0.1f;
                    CreateSprite(BITMAP_LIGHTNING + 1, Position, Scale * 0.3f, Light, o);
                    CreateSprite(BITMAP_LIGHTNING + 1, Position, Scale * 1.f, o->Light, o, -(int)WorldTime * 0.1f);
                    CreateSprite(BITMAP_LIGHTNING + 1, Position, Scale * 2.5f, o->Light, o, (int)WorldTime * 0.1f);
                }
            }
        }
        else if (o->CurrentAction == PLAYER_SKILL_LIGHTNING_ORB
            || o->CurrentAction == PLAYER_SKILL_LIGHTNING_ORB_UNI
            || o->CurrentAction == PLAYER_SKILL_LIGHTNING_ORB_DINO
            || o->CurrentAction == PLAYER_SKILL_LIGHTNING_ORB_FENRIR
            )
        {
            vec3_t vLight, vRelativePos, vWorldPos;
            Vector(1.0f, 1.0f, 1.0f, vLight);
            Vector(0.f, 0.f, 0.f, vRelativePos);
            Vector(0.f, 0.f, 0.f, vWorldPos);
            // 27 "Bip01 R Forearm"
            b->TransformPosition(o->BoneTransform[27], vRelativePos, vWorldPos, true);

            Vector(0.2f, 0.2f, 1.0f, vLight);
            CreateEffect(MODEL_FENRIR_THUNDER, vWorldPos, o->Angle, vLight, 2, o);
            CreateEffect(MODEL_FENRIR_THUNDER, vWorldPos, o->Angle, vLight, 2, o);
        }
        else if (o->CurrentAction >= PLAYER_SKILL_SLEEP && o->CurrentAction <= PLAYER_SKILL_SLEEP_FENRIR)
        {
            auto iSkillType = CharacterAttribute->Skill[Hero->CurrentSkill];

            vec3_t vLight, vRelativePos, vWorldPos;
            Vector(1.0f, 1.0f, 1.0f, vLight);
            Vector(0.f, 0.f, 0.f, vRelativePos);
            Vector(0.f, 0.f, 0.f, vWorldPos);
            b->TransformPosition(o->BoneTransform[37], vRelativePos, vWorldPos, true);	// "Bip01 L Hand"

            float fRot = (WorldTime * 0.0006f) * 360.0f;

            // shiny
            if (iSkillType == AT_SKILL_ALICE_SLEEP
                || iSkillType == AT_SKILL_ALICE_SLEEP_STR
                )
            {
                Vector(0.5f, 0.2f, 0.8f, vLight);
            }
            else if (iSkillType == AT_SKILL_ALICE_BLIND)
            {
                Vector(1.0f, 1.0f, 1.0f, vLight);
            }
            else if (iSkillType == AT_SKILL_ALICE_THORNS)
            {
                Vector(0.8f, 0.5f, 0.2f, vLight);
            }
            else if (iSkillType == AT_SKILL_ALICE_BERSERKER || iSkillType == AT_SKILL_ALICE_BERSERKER_STR)
            {
                Vector(1.0f, 0.1f, 0.2f, vLight);
            }
            else if (iSkillType == AT_SKILL_ALICE_WEAKNESS)
            {
                Vector(0.8f, 0.1f, 0.1f, vLight);
            }
            else if (iSkillType == AT_SKILL_ALICE_ENERVATION)
            {
                Vector(0.25f, 1.0f, 0.7f, Light);
            }

            if (iSkillType == AT_SKILL_ALICE_SLEEP
                || iSkillType == AT_SKILL_ALICE_SLEEP_STR
                || iSkillType == AT_SKILL_ALICE_THORNS
                || iSkillType == AT_SKILL_ALICE_BERSERKER
                || iSkillType == AT_SKILL_ALICE_BERSERKER_STR
                || iSkillType == AT_SKILL_ALICE_WEAKNESS
                || iSkillType == AT_SKILL_ALICE_ENERVATION
                )
            {
                CreateSprite(BITMAP_SHINY + 5, vWorldPos, 1.0f, vLight, o, fRot);
                CreateSprite(BITMAP_SHINY + 5, vWorldPos, 0.7f, vLight, o, -fRot);
            }
            else if (iSkillType == AT_SKILL_ALICE_BLIND)
            {
                CreateSprite(BITMAP_SHINY + 5, vWorldPos, 1.0f, vLight, o, fRot, 1);
                CreateSprite(BITMAP_SHINY + 5, vWorldPos, 0.7f, vLight, o, -fRot, 1);
            }

            // pin_light
            if (iSkillType == AT_SKILL_ALICE_SLEEP
                || iSkillType == AT_SKILL_ALICE_SLEEP_STR
                )
            {
                Vector(0.7f, 0.0f, 0.8f, vLight);
            }
            else if (iSkillType == AT_SKILL_ALICE_BLIND)
            {
                Vector(1.0f, 1.0f, 1.0f, vLight);
            }
            else if (iSkillType == AT_SKILL_ALICE_THORNS)
            {
                Vector(0.8f, 0.5f, 0.2f, vLight);
            }
            else if (iSkillType == AT_SKILL_ALICE_BERSERKER || iSkillType == AT_SKILL_ALICE_BERSERKER_STR)
            {
                Vector(1.0f, 0.1f, 0.2f, vLight);
            }
            else if (iSkillType == AT_SKILL_ALICE_THORNS)
            {
                Vector(0.8f, 0.1f, 0.1f, vLight);
            }
            else if (iSkillType == AT_SKILL_ALICE_ENERVATION)
            {
                Vector(0.25f, 1.f, 0.7f, vLight);
            }

            if (iSkillType == AT_SKILL_ALICE_SLEEP
                || iSkillType == AT_SKILL_ALICE_SLEEP_STR
                || iSkillType == AT_SKILL_ALICE_THORNS
                || iSkillType == AT_SKILL_ALICE_BERSERKER
                || iSkillType == AT_SKILL_ALICE_BERSERKER_STR
                || iSkillType == AT_SKILL_ALICE_WEAKNESS
                || iSkillType == AT_SKILL_ALICE_ENERVATION
                )
            {
                CreateSprite(BITMAP_PIN_LIGHT, vWorldPos, 1.7f, vLight, o, (float)(rand() % 360));
                CreateSprite(BITMAP_PIN_LIGHT, vWorldPos, 1.5f, vLight, o, (float)(rand() % 360));
            }
            else if (iSkillType == AT_SKILL_ALICE_BLIND)
            {
                CreateSprite(BITMAP_PIN_LIGHT, vWorldPos, 1.7f, vLight, o, (float)(rand() % 360), 1);
                CreateSprite(BITMAP_PIN_LIGHT, vWorldPos, 1.5f, vLight, o, (float)(rand() % 360), 1);
            }

            // cra04, clud64
            if (iSkillType == AT_SKILL_ALICE_SLEEP
                || iSkillType == AT_SKILL_ALICE_SLEEP_STR
                )
            {
                Vector(0.6f, 0.1f, 0.8f, vLight);
            }
            else if (iSkillType == AT_SKILL_ALICE_BLIND)
            {
                Vector(1.0f, 1.0f, 1.0f, vLight);
            }
            else if (iSkillType == AT_SKILL_ALICE_THORNS)
            {
                Vector(0.8f, 0.5f, 0.2f, vLight);
            }
            else if (iSkillType == AT_SKILL_ALICE_BERSERKER || iSkillType == AT_SKILL_ALICE_BERSERKER_STR)
            {
                Vector(1.0f, 0.1f, 0.2f, vLight);
            }
            else if (iSkillType == AT_SKILL_ALICE_THORNS)
            {
                Vector(0.8f, 0.1f, 0.1f, vLight);
            }
            else if (iSkillType == AT_SKILL_ALICE_ENERVATION)
            {
                Vector(0.25f, 1.f, 0.7f, vLight);
            }
            if (rand_fps_check(1))
            {
                if (iSkillType == AT_SKILL_ALICE_SLEEP
                    || iSkillType == AT_SKILL_ALICE_SLEEP_STR
                    || iSkillType == AT_SKILL_ALICE_THORNS
                    || iSkillType == AT_SKILL_ALICE_BERSERKER
                    || iSkillType == AT_SKILL_ALICE_BERSERKER_STR
                    || iSkillType == AT_SKILL_ALICE_WEAKNESS
                    || iSkillType == AT_SKILL_ALICE_ENERVATION
                    )
                {
                    CreateParticle(BITMAP_LIGHT + 2, vWorldPos, o->Angle, vLight, 0, 1.0f);
                    CreateParticle(BITMAP_CLUD64, vWorldPos, o->Angle, vLight, 3, 0.5f);
                }
                else if (iSkillType == AT_SKILL_ALICE_BLIND)
                {
                    CreateParticle(BITMAP_LIGHT + 2, vWorldPos, o->Angle, vLight, 4, 1.0f);
                    CreateParticle(BITMAP_CLUD64, vWorldPos, o->Angle, vLight, 5, 0.5f);
                }
            }
        }
        // ChainLighting
        else if (o->CurrentAction == PLAYER_SKILL_CHAIN_LIGHTNING)
        {
            vec3_t vRelativePos, vWorldPos, vLight;
            Vector(0.f, 0.f, 0.f, vRelativePos);
            Vector(0.4f, 0.4f, 0.8f, vLight);

            b->TransformPosition(o->BoneTransform[37], vRelativePos, vWorldPos, true);	// "Bip01 L Hand"

            if (rand_fps_check(1))
            {
                CreateEffect(MODEL_FENRIR_THUNDER, vWorldPos, o->Angle, vLight, 2, o);
                CreateEffect(MODEL_FENRIR_THUNDER, vWorldPos, o->Angle, vLight, 2, o);
            }

            CreateSprite(BITMAP_LIGHT, vWorldPos, 1.5f, vLight, o, 0.f);

            b->TransformPosition(o->BoneTransform[28], vRelativePos, vWorldPos, true);	// "Bip01 R Hand"
            if (rand_fps_check(1))
            {
                CreateEffect(MODEL_FENRIR_THUNDER, vWorldPos, o->Angle, vLight, 2, o);
                CreateEffect(MODEL_FENRIR_THUNDER, vWorldPos, o->Angle, vLight, 2, o);
            }

            CreateSprite(BITMAP_LIGHT, vWorldPos, 1.5f, vLight, o, 0.f);
        }

        if (g_isCharacterBuff(o, eBuff_Attack) || g_isCharacterBuff(o, eBuff_HelpNpc))
        {
            if (!g_isCharacterBuff(o, eBuff_Cloaking))
            {
                for (int i = 0; i < 2; i++)
                {
                    Luminosity = (float)(rand() % 30 + 70) * 0.01f;
                    Vector(Luminosity * 1.f, Luminosity * 0.3f, Luminosity * 0.2f, Light);
                    b->TransformPosition(o->BoneTransform[c->Weapon[i].LinkBone], p, Position, true);
                    CreateSprite(BITMAP_SHINY + 1, Position, 1.5f, Light, o);
                    b->TransformPosition(o->BoneTransform[c->Weapon[i].LinkBone - 6], p, Position, true);
                    CreateSprite(BITMAP_SHINY + 1, Position, 1.5f, Light, o);
                    b->TransformPosition(o->BoneTransform[c->Weapon[i].LinkBone - 7], p, Position, true);
                    CreateSprite(BITMAP_SHINY + 1, Position, 1.5f, Light, o);
                }
                if (!SearchJoint(MODEL_SPEARSKILL, o, 4)
                    && !SearchJoint(MODEL_SPEARSKILL, o, 9))
                {
                    for (int i = 0; i < 5; i++)
                    {
                        CreateJoint(MODEL_SPEARSKILL, o->Position, o->Position, o->Angle, 4, o, 20.0f, -1, 0, 0, c->TargetCharacter);
                    }
                }
            }
        }
        else if (g_isCharacterBuff(o, eBuff_Defense))
        {
            if (!g_isCharacterBuff(o, eBuff_Cloaking))
            {
                if (SearchJoint(MODEL_SPEARSKILL, o, 4) == false && SearchJoint(MODEL_SPEARSKILL, o, 9) == false)
                {
                    for (int i = 0; i < 5; ++i)
                    {
                        CreateJoint(MODEL_SPEARSKILL, o->Position, o->Position, o->Angle, 4, o, 20.0f, -1, 0, 0, c->TargetCharacter);
                    }
                }
            }
        }

        if (g_isCharacterBuff((&c->Object), eBuff_PcRoomSeal1)
            || g_isCharacterBuff((&c->Object), eBuff_PcRoomSeal2)
            || g_isCharacterBuff((&c->Object), eBuff_PcRoomSeal3)
            || g_isCharacterBuff((&c->Object), eBuff_Seal1)
            || g_isCharacterBuff((&c->Object), eBuff_Seal2)
            || g_isCharacterBuff((&c->Object), eBuff_Seal3)
            || g_isCharacterBuff((&c->Object), eBuff_Seal4))
        {
            if (SearchJoint(MODEL_SPEARSKILL, o, 10) == false
                && SearchJoint(MODEL_SPEARSKILL, o, 11) == false)
            {
                for (int i = 0; i < 3; ++i)
                {
                    CreateJoint(MODEL_SPEARSKILL, o->Position, o->Position, o->Angle, 10, o, 12.0f, -1, 0, 0, c->Key);
                }
            }
        }
        else
        {
            DeleteJoint(MODEL_SPEARSKILL, o, 10);
            DeleteJoint(MODEL_SPEARSKILL, o, 11);
        }
        if (g_isCharacterBuff((&c->Object), eBuff_Thorns))
        {
            if (SearchJoint(BITMAP_FLARE, o, 43) == false)
            {
                vec3_t vLight;
                Vector(0.9f, 0.6f, 0.1f, vLight);
                CreateJoint(BITMAP_FLARE, o->Position, o->Position, o->Angle, 43, o, 50.f, 0, 0, 0, 0, vLight);
            }
        }
        else
        {
            DeleteJoint(BITMAP_FLARE, o, 43);
        }

        if (g_isCharacterBuff((&c->Object), eBuff_Berserker))
        {
            if (!SearchEffect(BITMAP_ORORA, o, 0))
            {
                vec3_t vLight[2];
                Vector(0.9f, 0.0f, 0.1f, vLight[0]);
                Vector(1.0f, 1.0f, 1.0f, vLight[1]);
                for (int i = 0; i < 4; ++i)
                {
                    CreateEffect(BITMAP_ORORA, o->Position, o->Angle, vLight[0], i, o);
                    if (i == 2 || i == 3)
                        CreateEffect(BITMAP_SPARK + 2, o->Position, o->Angle, vLight[1], i, o);
                }
                CreateEffect(BITMAP_LIGHT_MARKS, o->Position, o->Angle, vLight[0], 0, o);
            }
        }
        else
        {
            for (int i = 0; i < 4; ++i)
            {
                DeleteEffect(BITMAP_ORORA, o, i);
                if (i == 2 || i == 3)
                    DeleteEffect(BITMAP_SPARK + 2, o, i);
            }
            DeleteEffect(BITMAP_LIGHT_MARKS, o);
        }

        if (g_isCharacterBuff((&c->Object), eDeBuff_Blind))
        {
            if (SearchEffect(MODEL_ALICE_BUFFSKILL_EFFECT, o, 3) == false)
            {
                vec3_t vLight;
                Vector(1.0f, 1.f, 1.f, vLight);
                CreateEffect(MODEL_ALICE_BUFFSKILL_EFFECT, o->Position, o->Angle, vLight, 3, o);
            }
        }
        else
        {
            DeleteEffect(MODEL_ALICE_BUFFSKILL_EFFECT, o, 3);
        }

        if (g_isCharacterBuff((&c->Object), eDeBuff_Sleep))
        {
            if (SearchEffect(MODEL_ALICE_BUFFSKILL_EFFECT, o, 4) == false)
            {
                vec3_t vLight;
                Vector(0.8f, 0.3f, 0.9f, vLight);
                CreateEffect(MODEL_ALICE_BUFFSKILL_EFFECT, o->Position, o->Angle, vLight, 4, o);
            }
        }
        else
        {
            DeleteEffect(MODEL_ALICE_BUFFSKILL_EFFECT, o, 4);
        }

        if (g_isCharacterBuff((&c->Object), eDeBuff_AttackDown))
        {
            if (!SearchEffect(BITMAP_SHINY + 6, o, 1))
            {
                vec3_t vLight;
                Vector(1.4f, 0.2f, 0.2f, vLight);
                CreateEffect(BITMAP_SHINY + 6, o->Position, o->Angle, vLight, 1, o, -1, 0, 0, 0, 0.5f);
                CreateEffect(BITMAP_PIN_LIGHT, o->Position, o->Angle, vLight, 1, o, -1, 0, 0, 0, 1.f);
            }
        }
        else
        {
            DeleteEffect(BITMAP_SHINY + 6, o, 1);
            DeleteEffect(BITMAP_PIN_LIGHT, o, 1);
        }

        if (g_isCharacterBuff((&c->Object), eDeBuff_DefenseDown))
        {
            if (!SearchEffect(BITMAP_SHINY + 6, o, 2))
            {
                vec3_t vLight;
                Vector(0.25f, 1.0f, 0.7f, Light);
                CreateEffect(BITMAP_SHINY + 6, o->Position, o->Angle, vLight, 2, o, -1, 0, 0, 0, 0.5f);
                CreateEffect(BITMAP_PIN_LIGHT, o->Position, o->Angle, vLight, 2, o, -1, 0, 0, 0, 1.f);
            }
        }
        else
        {
            DeleteEffect(BITMAP_SHINY + 6, o, 2);
            DeleteEffect(BITMAP_PIN_LIGHT, o, 2);
        }

        if (g_isCharacterBuff((&c->Object), eDeBuff_SahamuttDOT))
        {
            g_SummonSystem.CreateDamageOfTimeEffect(AT_SKILL_SUMMON_EXPLOSION, &c->Object);
        }
        else
        {
            g_SummonSystem.RemoveDamageOfTimeEffect(AT_SKILL_SUMMON_EXPLOSION, &c->Object);
        }

        if (g_isCharacterBuff((&c->Object), eDeBuff_NeilDOT))
        {
            g_SummonSystem.CreateDamageOfTimeEffect(AT_SKILL_SUMMON_REQUIEM, &c->Object);
        }
        else
        {
            g_SummonSystem.RemoveDamageOfTimeEffect(AT_SKILL_SUMMON_REQUIEM, &c->Object);
        }

        if (g_isCharacterBuff((&c->Object), eBuff_SwellOfMagicPower))
        {
            if (!g_isCharacterBuff((&c->Object), eBuff_Cloaking))
            {
                if (!SearchEffect(MODEL_SWELL_OF_MAGICPOWER_BUFF_EFF, o, 0))
                {
                    vec3_t vLight;
                    Vector(0.7f, 0.2f, 0.9f, vLight);
                    CreateEffect(MODEL_SWELL_OF_MAGICPOWER_BUFF_EFF, o->Position, o->Angle, vLight, 0, o);
                }
            }
        }
        else
        {
            DeleteEffect(MODEL_SWELL_OF_MAGICPOWER_BUFF_EFF, o, 0);
        }

        if (gMapManager.InChaosCastle() == false)
        {
            if (c->ExtendState)
            {
                if ((o->CurrentAction < PLAYER_WALK_MALE ||
                    o->CurrentAction == PLAYER_DARKLORD_STAND ||
                    o->CurrentAction == PLAYER_STOP_RIDE_HORSE ||
                    o->CurrentAction == PLAYER_STOP_TWO_HAND_SWORD_TWO
                    ) &&
                    c->ExtendStateTime >= 100)
                {
                    Vector(0.2f, 0.7f, 0.9f, Light);
                    CreateEffect(BITMAP_LIGHT, o->Position, o->Angle, Light, 3, o);

                    c->ExtendStateTime = 0;
                }
                c->ExtendStateTime += min(1, FPS_ANIMATION_FACTOR);
            }

            if (fullset && g_pOption->GetRenderLevel() >= 2)
            {
                PartObjectColor(c->BodyPart[5].Type, o->Alpha, 0.5f, Light);

                if (!g_isCharacterBuff(o, eBuff_Cloaking))
                {
                    for (int i = 0; i < 2; i++)
                    {
                        b->TransformPosition(o->BoneTransform[c->Weapon[i].LinkBone], p, Position, true);
                        CreateSprite(BITMAP_LIGHT, Position, 1.3f, Light, o);
                        b->TransformPosition(o->BoneTransform[c->Weapon[i].LinkBone - 6], p, Position, true);
                        CreateSprite(BITMAP_LIGHT, Position, 1.3f, Light, o);
                        b->TransformPosition(o->BoneTransform[c->Weapon[i].LinkBone - 7], p, Position, true);
                        CreateSprite(BITMAP_LIGHT, Position, 1.3f, Light, o);
                    }
                }
                if ((c->BodyPart[BODYPART_BOOTS].Type >= MODEL_DRAGON_KNIGHT_BOOTS && c->BodyPart[BODYPART_BOOTS].Type <= MODEL_SUNLIGHT_BOOTS)
                    || c->BodyPart[BODYPART_BOOTS].Type == MODEL_AURA_BOOTS)
                {
                    if (EquipmentLevelSet > 9)
                    {
                        VectorCopy(o->Light, Light);

                        if (c->BodyPart[BODYPART_BOOTS].Type == MODEL_DRAGON_KNIGHT_BOOTS) Vector(0.65f, 0.3f, 0.1f, o->Light);
                        if (c->BodyPart[BODYPART_BOOTS].Type == MODEL_VENOM_MIST_BOOTS) Vector(0.1f, 0.1f, 0.9f, o->Light);
                        if (c->BodyPart[BODYPART_BOOTS].Type == MODEL_SYLPHID_RAY_BOOTS) Vector(0.0f, 0.32f, 0.24f, o->Light);
                        if (c->BodyPart[BODYPART_BOOTS].Type == MODEL_VOLCANO_BOOTS) Vector(0.5f, 0.24f, 0.8f, o->Light);
                        if (c->BodyPart[BODYPART_BOOTS].Type == MODEL_SUNLIGHT_BOOTS) Vector(0.6f, 0.4f, 0.0f, o->Light);
                        if (c->BodyPart[BODYPART_BOOTS].Type == MODEL_AURA_BOOTS) Vector(0.6f, 0.3f, 0.4f, o->Light);
                        if (EquipmentLevelSet == 10 && rand_fps_check(4))
                        {
                            Vector(0.0f, -18.0f, 50.0f, p);
                            b->TransformPosition(o->BoneTransform[0], p, Position, true);
                            CreateParticle(BITMAP_WATERFALL_2, Position, o->Angle, o->Light, 3);
                            Vector(0.0f, 0.0f, 70.0f, p);
                            b->TransformPosition(o->BoneTransform[0], p, Position, true);
                            CreateParticle(BITMAP_WATERFALL_2, Position, o->Angle, o->Light, 3);
                            Vector(0.0f, 18.0f, 50.0f, p);
                            b->TransformPosition(o->BoneTransform[0], p, Position, true);
                            CreateParticle(BITMAP_WATERFALL_2, Position, o->Angle, o->Light, 3);
                        }
                        else if (EquipmentLevelSet == 11 && rand_fps_check(3))
                        {
                            Vector(0.0f, -18.0f, 50.0f, p);
                            b->TransformPosition(o->BoneTransform[0], p, Position, true);
                            CreateParticle(BITMAP_WATERFALL_2, Position, o->Angle, o->Light, 3);
                            Vector(0.0f, 0.0f, 70.0f, p);
                            b->TransformPosition(o->BoneTransform[0], p, Position, true);
                            CreateParticle(BITMAP_WATERFALL_2, Position, o->Angle, o->Light, 3);
                            Vector(0.0f, 18.0f, 50.0f, p);
                            b->TransformPosition(o->BoneTransform[0], p, Position, true);
                            CreateParticle(BITMAP_WATERFALL_2, Position, o->Angle, o->Light, 3);
                        }
                        else if (EquipmentLevelSet == 12 && rand_fps_check(2))
                        {
                            Vector(0.0f, -18.0f, 50.0f, p);
                            b->TransformPosition(o->BoneTransform[0], p, Position, true);
                            CreateParticle(BITMAP_WATERFALL_2, Position, o->Angle, o->Light, 3);
                            Vector(0.0f, 0.0f, 70.0f, p);
                            b->TransformPosition(o->BoneTransform[0], p, Position, true);
                            CreateParticle(BITMAP_WATERFALL_2, Position, o->Angle, o->Light, 3);
                            Vector(0.0f, 18.0f, 50.0f, p);
                            b->TransformPosition(o->BoneTransform[0], p, Position, true);
                            CreateParticle(BITMAP_WATERFALL_2, Position, o->Angle, o->Light, 3);
                        }
                        else if (EquipmentLevelSet == 13 && rand_fps_check(1))
                        {
                            Vector(0.0f, -20.0f, 50.0f, p);
                            b->TransformPosition(o->BoneTransform[0], p, Position, true);
                            CreateParticle(BITMAP_WATERFALL_2, Position, o->Angle, o->Light, 3);
                            Vector(0.0f, 0.0f, 70.0f, p);
                            b->TransformPosition(o->BoneTransform[0], p, Position, true);
                            CreateParticle(BITMAP_WATERFALL_2, Position, o->Angle, o->Light, 3);
                            Vector(0.0f, 20.0f, 50.0f, p);
                            b->TransformPosition(o->BoneTransform[0], p, Position, true);
                            CreateParticle(BITMAP_WATERFALL_2, Position, o->Angle, o->Light, 3);
                        }
                        else if (EquipmentLevelSet == 14 && rand_fps_check(1))
                        {
                            Vector(0.0f, -20.0f, 50.0f, p);
                            b->TransformPosition(o->BoneTransform[0], p, Position, true);
                            CreateParticle(BITMAP_WATERFALL_2, Position, o->Angle, o->Light, 3);
                            Vector(0.0f, 0.0f, 70.0f, p);
                            b->TransformPosition(o->BoneTransform[0], p, Position, true);
                            CreateParticle(BITMAP_WATERFALL_2, Position, o->Angle, o->Light, 3);
                            Vector(0.0f, 20.0f, 50.0f, p);
                            b->TransformPosition(o->BoneTransform[0], p, Position, true);
                            CreateParticle(BITMAP_WATERFALL_2, Position, o->Angle, o->Light, 3);
                        }
                        else if (EquipmentLevelSet == 15 && rand_fps_check(1))
                        {
                            Vector(0.0f, -20.0f, 50.0f, p);
                            b->TransformPosition(o->BoneTransform[0], p, Position, true);
                            CreateParticle(BITMAP_WATERFALL_2, Position, o->Angle, o->Light, 3);
                            Vector(0.0f, 0.0f, 70.0f, p);
                            b->TransformPosition(o->BoneTransform[0], p, Position, true);
                            CreateParticle(BITMAP_WATERFALL_2, Position, o->Angle, o->Light, 3);
                            Vector(0.0f, 20.0f, 50.0f, p);
                            b->TransformPosition(o->BoneTransform[0], p, Position, true);
                            CreateParticle(BITMAP_WATERFALL_2, Position, o->Angle, o->Light, 3);
                        }

                        VectorCopy(Light, o->Light);
                    }
                }

                if (EquipmentLevelSet > 9)
                {
                    if (rand_fps_check(20))//(o->CurrentAction<PLAYER_WALK_MALE || o->CurrentAction>PLAYER_RUN_RIDE_WEAPON) && rand_fps_check(6))
                    {
                        VectorCopy(o->Light, Light);
                        Vector(1.f, 1.f, 1.f, o->Light);

                        if (EquipmentLevelSet == 10)
                        {
                            CreateParticle(BITMAP_FLARE, o->Position, o->Angle, o->Light, 0, 0.19f, o);
                        }
                        else if (EquipmentLevelSet == 11)
                        {
                            if (rand_fps_check(8))
                            {
                                CreateJoint(BITMAP_FLARE, o->Position, o->Position, o->Angle, 0, o);
                            }
                            else
                            {
                                CreateParticle(BITMAP_FLARE, o->Position, o->Angle, o->Light, 0, 0.19f, o);
                            }
                        }
                        else if (EquipmentLevelSet == 12)
                        {
                            if (rand_fps_check(6))
                            {
                                CreateJoint(BITMAP_FLARE, o->Position, o->Position, o->Angle, 18, o, 20, -1, 0);
                                CreateJoint(BITMAP_FLARE, o->Position, o->Position, o->Angle, 18, o, 20, -1, 1);
                            }
                            else if (rand_fps_check(3))
                            {
                                CreateParticle(BITMAP_FLARE, o->Position, o->Angle, o->Light, 0, 0.19f, o);
                            }
                        }
                        else if (EquipmentLevelSet == 13)
                        {
                            if (rand_fps_check(6))
                            {
                                CreateJoint(BITMAP_FLARE, o->Position, o->Position, o->Angle, 18, o, 20, -1, 0);
                                CreateJoint(BITMAP_FLARE, o->Position, o->Position, o->Angle, 18, o, 20, -1, 1);
                            }

                            if (rand_fps_check(4))
                            {
                                CreateParticle(BITMAP_FLARE, o->Position, o->Angle, o->Light, 0, 0.19f, o);
                                CreateJoint(BITMAP_FLARE + 1, o->Position, o->Position, o->Angle, 7, o, 20, 40, 1);
                            }
                        }
                        else if (EquipmentLevelSet == 14)
                        {
                            if (rand_fps_check(6))
                            {
                                CreateJoint(BITMAP_FLARE, o->Position, o->Position, o->Angle, 18, o, 20, -1, 0);
                                CreateJoint(BITMAP_FLARE, o->Position, o->Position, o->Angle, 18, o, 20, -1, 1);
                            }

                            if (rand_fps_check(4))
                            {
                                CreateParticle(BITMAP_FLARE, o->Position, o->Angle, o->Light, 0, 0.19f, o);
                                CreateJoint(BITMAP_FLARE + 1, o->Position, o->Position, o->Angle, 7, o, 20, 40, 1);
                            }
                        }
                        else if (EquipmentLevelSet == 15)
                        {
                            if (rand_fps_check(6))
                            {
                                CreateJoint(BITMAP_FLARE, o->Position, o->Position, o->Angle, 18, o, 20, -1, 0);
                                CreateJoint(BITMAP_FLARE, o->Position, o->Position, o->Angle, 18, o, 20, -1, 1);
                            }

                            if (rand_fps_check(4))
                            {
                                CreateParticle(BITMAP_FLARE, o->Position, o->Angle, o->Light, 0, 0.19f, o);
                                CreateJoint(BITMAP_FLARE + 1, o->Position, o->Position, o->Angle, 7, o, 20, 40, 1);
                            }
                        }
                        VectorCopy(Light, o->Light);
                    }

                    if (EquipmentLevelSet == 15 && g_pOption->GetRenderLevel() >= 4 && rand_fps_check(1))
                    {
                        //left
                        vec3_t vColor;
                        Vector(13.0f, 10.0f, 3.0f, p);
                        b->TransformPosition(o->BoneTransform[20], p, Position, true);
                        VectorCopy(Position, o->EyeLeft);

                        Vector(0.09f, 0.09f, 0.8f, vColor);
                        CreateJoint(BITMAP_JOINT_ENERGY, Position, o->Position, o->Angle, 55, o, 6.0f, -1, 0, 0, -1, vColor);
                        auto fRad = (float)sinf((WorldTime) * 0.002f);
                        Vector(0.2f, 0.4f, 0.8f, vColor);
                        CreateSprite(BITMAP_SHINY + 6, Position, 0.5f * fRad, vColor, o);

                        Vector(0.1f, 0.15f, 1.0f, vColor);
                        CreateSprite(BITMAP_PIN_LIGHT, Position, 1.3f * fRad + 0.5f, vColor, o, 100.0f);

                        // right
                        Vector(13.0f, 10.0f, -3.0f, p);
                        b->TransformPosition(o->BoneTransform[20], p, Position, true);
                        VectorCopy(Position, o->EyeRight);

                        Vector(0.09f, 0.09f, 0.8f, vColor);
                        CreateJoint(BITMAP_JOINT_ENERGY, Position, o->Position, o->Angle, 56, o, 6.0f, -1, 0, 0, -1, vColor);

                        Vector(0.2f, 0.4f, 0.8f, vColor);
                        CreateSprite(BITMAP_SHINY + 6, Position, 0.5f * fRad, vColor, o);

                        Vector(0.1f, 0.15f, 1.0f, vColor);
                        CreateSprite(BITMAP_PIN_LIGHT, Position, 1.3f * fRad + 0.5f, vColor, o, 80.0f);
                    }
                }
            }
        }
        break;
    case MODEL_BULL_FIGHTER:
    case MODEL_DEATH_COW:
        if ((o->Type == MODEL_BULL_FIGHTER && c->Level == 1) || (o->Type == MODEL_DEATH_COW))
            RenderEye(o, 22, 23);
        break;
    case MODEL_CRUST:
        RenderEye(o, 26, 27, 2.0f);
        break;
    case MODEL_HYDRA:
        RenderLight(o, BITMAP_LIGHTNING + 1, 1.f, 63, 0.f, 0.f, 20.f);
        RenderLight(o, BITMAP_SHINY + 2, 4.f, 63, 0.f, 0.f, 20.f);
        break;
    case MODEL_VEPAR:
        RenderLight(o, BITMAP_LIGHTNING + 1, 0.5f, 30, 0.f, 0.f, -5.f);
        RenderLight(o, BITMAP_LIGHTNING + 1, 0.5f, 39, 0.f, 0.f, -5.f);
        RenderLight(o, BITMAP_SPARK, 4.f, 30, 0.f, 0.f, -5.f);
        RenderLight(o, BITMAP_SPARK, 4.f, 39, 0.f, 0.f, -5.f);
        RenderLight(o, BITMAP_SHINY + 2, 2.f, 30, 0.f, 0.f, -5.f);
        RenderLight(o, BITMAP_SHINY + 2, 2.f, 39, 0.f, 0.f, -5.f);
        break;
    case MODEL_LIZARD:
        RenderEye(o, 42, 43);
        RenderLight(o, BITMAP_SPARK, 2.f, 26, 0.f, 0.f, 0.f);
        RenderLight(o, BITMAP_SPARK, 2.f, 31, 0.f, 0.f, 0.f);
        RenderLight(o, BITMAP_SPARK, 2.f, 36, 0.f, 0.f, 0.f);
        RenderLight(o, BITMAP_SPARK, 2.f, 41, 0.f, 0.f, 0.f);
        RenderLight(o, BITMAP_SHINY + 2, 1.f, 26, 0.f, 0.f, 0.f);
        RenderLight(o, BITMAP_SHINY + 2, 1.f, 31, 0.f, 0.f, 0.f);
        RenderLight(o, BITMAP_SHINY + 2, 1.f, 36, 0.f, 0.f, 0.f);
        RenderLight(o, BITMAP_SHINY + 2, 1.f, 41, 0.f, 0.f, 0.f);
        break;
    case MODEL_BAHAMUT:
        RenderLight(o, BITMAP_SPARK, 4.f, 9, 0.f, 0.f, 5.f);
        RenderLight(o, BITMAP_SHINY + 2, 3.f, 9, 0.f, 0.f, 5.f);
        break;
    case MODEL_MIX_NPC:
        RenderLight(o, BITMAP_LIGHT, 1.5f, 32, 0.f, 0.f, 0.f);
        break;
    case MODEL_NPC_SEVINA:
        RenderLight(o, BITMAP_LIGHT, 2.5f, 6, 0.f, 0.f, 0.f);
        break;
    case MODEL_NPC_DEVILSQUARE:
    {
        Luminosity = (float)sinf((WorldTime) * 0.002f) * 0.35f + 0.65f;

        float Scale = 0.3f;
        Vector(Luminosity, Luminosity, Luminosity, Light);

        Vector(3.5f, -12.f, 10.f, p);
        b->TransformPosition(o->BoneTransform[20], p, Position, 1);
        CreateSprite(BITMAP_LIGHTNING + 1, Position, Scale, Light, o, (WorldTime / 50.0f));
        CreateSprite(BITMAP_LIGHTNING + 1, Position, Scale, Light, o, ((-WorldTime) / 50.0f));

        if (rand_fps_check(30))
        {
            p[0] = Position[0] + rand() % 100 - 50;
            p[1] = Position[1] + rand() % 100 - 50;
            p[2] = Position[2] + rand() % 100 - 50;

            CreateJoint(BITMAP_JOINT_ENERGY, p, Position, o->Angle, 6, NULL, 20.f);
        }
    }
    break;
    case MODEL_NPC_CASTEL_GATE:
    {
        vec3_t vPos, vRelative;
        float fLumi, fScale;

        fLumi = (sinf(WorldTime * 0.002f) + 2.0f) * 0.5f;
        Vector(fLumi * 1.0f, fLumi * 0.5f, fLumi * 0.3f, Light);
        fScale = fLumi / 2.0f;

        Vector(4.0f, 3.0f, -4.0f, vRelative);
        b->TransformPosition(o->BoneTransform[2], vRelative, vPos, true);

        CreateSprite(BITMAP_LIGHT, vPos, fScale, Light, o);
        CreateSprite(BITMAP_KANTURU_2ND_EFFECT1, vPos, fScale, Light, o);

        Vector(5.0f, 3.0f, 2.0f, vRelative);
        b->TransformPosition(o->BoneTransform[4], vRelative, vPos, true);

        CreateSprite(BITMAP_LIGHT, vPos, fScale, Light, o);
        CreateSprite(BITMAP_KANTURU_2ND_EFFECT1, vPos, fScale, Light, o);

        if (rand_fps_check(4))
        {
            Vector(-20.0f, 10.0f, 0.0f, vRelative);
            b->TransformPosition(
                o->BoneTransform[3], vRelative, vPos, true);

            vec3_t Angle;
            VectorCopy(o->Angle, Angle);
            Angle[0] += 120.0f;
            Angle[2] -= 60.0f;

            Vector(1.0f, 1.0f, 1.0f, Light);

            CreateParticle(
                BITMAP_FLAME, vPos, Angle, Light, 10, o->Scale / 2);
        }
    }
    break;
    case MODEL_SHADOW:
        Vector(0.f, 0.f, 0.f, p);
        Luminosity = 1.f;
        if (c->Level == 0)
        {
            Vector(Luminosity * 1.f, Luminosity * 1.f, Luminosity * 1.f, Light);
        }
        else
        {
            Vector(Luminosity * 1.f, Luminosity * 0.4f, Luminosity * 0.f, Light);
            Vector(Luminosity * 0.2f, Luminosity * 0.7f, Luminosity * 0.1f, Light);
        }

        for (int i = 0; i < b->NumBones; i++)
        {
            if (!b->Bones[i].Dummy)
            {
                if ((i >= 15 && i <= 20) || (i >= 27 && i <= 32))
                {
                }
                else
                {
                    b->TransformPosition(o->BoneTransform[i], p, Position, true);
                    if (c->Level == 0)
                        CreateSprite(BITMAP_SHINY + 1, Position, 2.5f, Light, o, 0.f, 1);
                    else
                        CreateSprite(BITMAP_MAGIC + 1, Position, 0.8f, Light, o, 0.f);
                    if (rand_fps_check(4) && o->CurrentAction >= MONSTER01_ATTACK1 && o->CurrentAction <= MONSTER01_ATTACK2)
                    {
                        CreateParticle(BITMAP_ENERGY, Position, o->Angle, Light);
                    }
                    //CreateParticle(BITMAP_SMOKE,Position,o->Angle,Light,1);
                }
            }
        }
        break;
    case MODEL_SEED_MASTER:
    {
        float fLumi, fScale;
        fScale = 2.0f;
        b->TransformByObjectBone(Position, o, 39);

        fLumi = (sinf(WorldTime * 0.001f) + 1.0f) * 0.3f + 0.4f;
        Vector(fLumi * 0.5f, fLumi * 0.5f, fLumi * 0.5f, Light);
        CreateSprite(BITMAP_FLARE, Position, fScale, Light, o);
    }
    break;
    case MODEL_SEED_INVESTIGATOR:
    {
        float fLumi, fScale;
        fScale = 1.0f;
        for (int i = 69; i <= 70; ++i)
        {
            b->TransformByObjectBone(Position, o, i);

            fLumi = (sinf(WorldTime * 0.001f) + 1.0f) * 0.3f + 0.4f;
            Vector(fLumi * 0.5f, fLumi * 0.5f, fLumi * 0.5f, Light);
            CreateSprite(BITMAP_FLARE_BLUE, Position, fScale, Light, o);
        }
    }
    break;
    default:
    {
        if (RenderHellasMonsterVisual(c, o, b)) break;
        if (battleCastle::RenderBattleCastleMonsterVisual(c, o, b)) break;
        if (M31HuntingGround::RenderHuntingGroundMonsterVisual(c, o, b)) break;
        if (M34CryingWolf2nd::RenderCryingWolf2ndMonsterVisual(c, o, b)) break;
        if (M34CryWolf1st::RenderCryWolf1stMonsterVisual(c, o, b)) break;
        if (M33Aida::RenderAidaMonsterVisual(c, o, b)) break;
        if (M37Kanturu1st::RenderKanturu1stMonsterVisual(c, o, b)) break;
        if (M38Kanturu2nd::Render_Kanturu2nd_MonsterVisual(c, o, b)) break;
        if (M39Kanturu3rd::RenderKanturu3rdMonsterVisual(c, o, b)) break;
        if (SEASON3A::CGM3rdChangeUp::Instance().RenderBalgasBarrackMonsterVisual(c, o, b))	break;
        if (g_CursedTemple->RenderMonsterVisual(c, o, b)) break;
        if (SEASON3B::GMNewTown::RenderMonsterVisual(c, o, b)) break;
        if (SEASON3C::GMSwampOfQuiet::RenderMonsterVisual(c, o, b)) break;
        if (TheMapProcess().RenderMonsterVisual(c, o, b) == true) break;
    }
    }

    if (SceneFlag == MAIN_SCENE)
    {
        if ((o->Kind == KIND_PLAYER
            && gMapManager.InBloodCastle() == false
            && gMapManager.InChaosCastle() == false
            && gMapManager.InBattleCastle()) == false)
        {
            RenderProtectGuildMark(c);
        }
    }

    giPetManager::RenderPet(c);

    if (gCharacterManager.GetBaseClass(c->Class) == CLASS_SUMMONER)
    {
        PART_t* w = &c->Weapon[1];
        g_SummonSystem.MoveEquipEffect(c, w->Type, w->Level, w->ExcellentFlags);
    }
}

void RenderCharactersClient()
{
    for (int i = 0; i < MAX_CHARACTERS_CLIENT; ++i)
    {
        CHARACTER* c = &CharactersClient[i];
        OBJECT* o = &c->Object;

        if (c != Hero && battleCastle::IsBattleCastleStart() == true && g_isCharacterBuff(o, eBuff_Cloaking))
        {
            if ((Hero->EtcPart == PARTS_ATTACK_KING_TEAM_MARK || Hero->EtcPart == PARTS_ATTACK_TEAM_MARK))
            {
                if (!(c->EtcPart == PARTS_ATTACK_KING_TEAM_MARK || c->EtcPart == PARTS_ATTACK_TEAM_MARK))
                {
                    if (Hero->TargetCharacter == c->Key)
                        Hero->TargetCharacter = -1;
                    continue;
                }
            }
            else
                if ((Hero->EtcPart == PARTS_ATTACK_KING_TEAM_MARK2 || Hero->EtcPart == PARTS_ATTACK_TEAM_MARK2))
                {
                    if (!(c->EtcPart == PARTS_ATTACK_KING_TEAM_MARK2 || c->EtcPart == PARTS_ATTACK_TEAM_MARK2))
                    {
                        if (Hero->TargetCharacter == c->Key)
                            Hero->TargetCharacter = -1;
                        continue;
                    }
                }
                else
                    if ((Hero->EtcPart == PARTS_ATTACK_KING_TEAM_MARK3 || Hero->EtcPart == PARTS_ATTACK_TEAM_MARK3))
                    {
                        if (!(c->EtcPart == PARTS_ATTACK_KING_TEAM_MARK3 || c->EtcPart == PARTS_ATTACK_TEAM_MARK3))
                        {
                            if (Hero->TargetCharacter == c->Key)
                                Hero->TargetCharacter = -1;
                            continue;
                        }
                    }
                    else
                        if (Hero->EtcPart == PARTS_DEFENSE_KING_TEAM_MARK || Hero->EtcPart == PARTS_DEFENSE_TEAM_MARK)
                        {
                            if (!(c->EtcPart == PARTS_DEFENSE_KING_TEAM_MARK || c->EtcPart == PARTS_DEFENSE_TEAM_MARK))
                            {
                                if (Hero->TargetCharacter == c->Key)
                                    Hero->TargetCharacter = -1;
                                continue;
                            }
                        }
        }
        if (c == Hero && (0x04 & Hero->CtlCode) && SceneFlag == MAIN_SCENE)
        {
            o->OBB.StartPos[0] = 1000.0f;
            o->OBB.XAxis[0] = o->OBB.YAxis[1] = o->OBB.ZAxis[2] = 1.0f;
            continue;
        }
        if (o->Live)
        {
            if (o->Visible)
            {
                if (i != SelectedCharacter && i != SelectedNpc)
                    RenderCharacter(c, o);
                else
                    RenderCharacter(c, o, true);

                if (o->Type == MODEL_PLAYER)
                    battleCastle::CreateBattleCastleCharacter_Visual(c, o);
            }
        }
    }

    if (gMapManager.InBattleCastle() || gMapManager.WorldActive == WD_31HUNTING_GROUND)
    {
        battleCastle::InitEtcSetting();
    }
}

void RenderProtectGuildMark(CHARACTER* c)
{
    if (c->GuildType == GT_ANGEL)
    {
        if (c->ProtectGuildMarkWorldTime == 0 || WorldTime - c->ProtectGuildMarkWorldTime > 5000.0f)
        {
            OBJECT* o = &c->Object;
            CreateEffect(MODEL_PROTECTGUILD, o->Position, o->Angle, o->Light, 0, o);
            c->ProtectGuildMarkWorldTime = WorldTime;
        }
    }
}

void ClearCharacters(int Key)
{
    for (int i = 0; i < MAX_CHARACTERS_CLIENT; i++)
    {
        CHARACTER* c = &CharactersClient[i];
        OBJECT* o = &c->Object;
        if (o->Live && c->Key != Key)
        {
            o->Live = false;

            BoneManager::UnregisterBone(c);

            for (int j = 0; j < MAX_MOUNTS; j++)
            {
                OBJECT* b = &Mounts[j];
                if (b->Live && b->Owner == o)
                    b->Live = false;
            }
        }

        DeletePet(c);
        DeleteCloth(c, o);
        DeleteParts(c);
    }
}

void DeleteCharacter(int Key)
{
    for (int i = 0; i < MAX_CHARACTERS_CLIENT; i++)
    {
        CHARACTER* c = &CharactersClient[i];
        OBJECT* o = &c->Object;
        if (o->Live && c->Key == Key)
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
            return;
        }
    }
}

void DeleteCharacter(CHARACTER* c, OBJECT* o)
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

int FindCharacterIndex(int Key)
{
    for (int i = 0; i < MAX_CHARACTERS_CLIENT; i++)
    {
        CHARACTER* c = &CharactersClient[i];
        if (c->Object.Live && c->Key == Key)
        {
            return i;
        }
    }
    return MAX_CHARACTERS_CLIENT;
}

int FindCharacterIndexByMonsterIndex(int Type)
{
    for (int i = 0; i < MAX_CHARACTERS_CLIENT; i++)
    {
        CHARACTER* c = &CharactersClient[i];
        if (c->Object.Live && c->MonsterIndex == Type)
        {
            return i;
        }
    }
    return MAX_CHARACTERS_CLIENT;
}

int HangerBloodCastleQuestItem(int Key)
{
    int index = MAX_CHARACTERS_CLIENT;
    for (int i = 0; i < MAX_CHARACTERS_CLIENT; i++)
    {
        CHARACTER* c = &CharactersClient[i];
        if (c->Object.Live && c->Key == Key)
        {
            index = i;
        }
        c->EtcPart = 0;
    }
    return index;
}

void SetAllAction(int Action)
{
    for (int i = 0; i < MAX_CHARACTERS_CLIENT; i++)
    {
        CHARACTER* c = &CharactersClient[i];
        if (c->Object.Live && c->Object.Type == MODEL_PLAYER)
        {
            Vector(0.f, 0.f, 180.f, c->Object.Angle);
            SetAction(&c->Object, Action);
        }
    }
}

void ReleaseCharacters(void)
{
    for (int i = 0; i < MAX_CHARACTERS_CLIENT; i++)
    {
        CHARACTER* c = &CharactersClient[i];
        OBJECT* o = &c->Object;
        if (o->BoneTransform != NULL)
        {
            delete[] o->BoneTransform;
            o->BoneTransform = NULL;
        }
        DeletePet(c);
        DeleteCloth(c, o);
        DeleteParts(c);
    }
    OBJECT* o = &CharacterView.Object;
    if (o->BoneTransform != NULL)
    {
        delete[] o->BoneTransform;
        o->BoneTransform = NULL;
    }
    DeletePet(&CharacterView);
    DeleteCloth(&CharacterView, o);
    DeleteParts(&CharacterView);

    BoneManager::UnregisterAll();
}

void CreateCharacterPointer(CHARACTER* c, int Type, unsigned char PositionX, unsigned char PositionY, float Rotation)
{
    OBJECT* o = &c->Object;
    c->PositionX = PositionX;
    c->PositionY = PositionY;
    c->TargetX = PositionX;
    c->TargetY = PositionY;
    if (c != Hero)
    {
        c->byExtensionSkill = 0;
    }
    c->m_pParts = NULL;

    giPetManager::DeletePet(c);

    int Index = TERRAIN_INDEX_REPEAT((c->PositionX), (c->PositionY));
    if ((TerrainWall[Index] & TW_SAFEZONE) == TW_SAFEZONE)
        c->SafeZone = true;
    else
        c->SafeZone = false;

    c->Path.PathNum = 0;
    c->Path.CurrentPath = 0;
    c->Movement = false;
    o->Live = true;
    o->Visible = false;
    o->AlphaEnable = true;
    o->LightEnable = true;
    o->ContrastEnable = false;
    o->EnableBoneMatrix = true;
    o->EnableShadow = false;
    c->Dead = 0;
    c->Blood = false;
    c->GuildTeam = 0;
    c->Run = 0;
    c->GuildMarkIndex = -1;
    c->PK = PVP_NEUTRAL;
    o->Type = Type;
    o->Scale = 0.9f;
    o->Timer = 0.f;
    o->Alpha = 1.f;
    o->AlphaTarget = 1.f;
    o->Velocity = 0.f;
    o->ShadowScale = 0.f;
    o->m_byHurtByDeathstab = 0;
    o->AI = 0;
    o->m_byBuildTime = 10;
    c->m_iDeleteTime = -128;
    o->m_bRenderShadow = true;
    o->m_fEdgeScale = 1.2f;
    c->m_bIsSelected = true;
    c->ExtendState = 0;
    c->ExtendStateTime = 0;
    c->m_byGensInfluence = 0;
    c->GuildStatus = -1;
    c->GuildType = 0;
    c->ProtectGuildMarkWorldTime = 0.0f;
    c->GuildRelationShip = 0;
    c->GuildSkill = AT_SKILL_STUN;
    c->BackupCurrentSkill = 255;
    c->GuildMasterKillCount = 0;
    c->m_byDieType = 0;
    o->m_bActionStart = false;
    o->m_bySkillCount = 0;
    c->NotRotateOnMagicHit = false;
    c->CtlCode = 0;
    c->m_CursedTempleCurSkill = AT_SKILL_CURSED_TEMPLE_PRODECTION;
    c->m_CursedTempleCurSkillPacket = false;
    c->HealthStatus = -1;
    c->ShieldStatus = -1;

    if (Type < MODEL_FACE || Type > MODEL_FACE + 6)
    {
        c->Class = CLASS_WIZARD;
    }

    if (Type == MODEL_PLAYER)
    {
        o->PriorAction = 1;
        o->CurrentAction = 1;
    }
    else
    {
        o->PriorAction = 0;
        o->CurrentAction = 0;
    }
    o->AnimationFrame = 0.002f;
    o->PriorAnimationFrame = 0;
    c->JumpTime = 0;
    o->HiddenMesh = -1;
    c->MoveSpeed = 10;

    g_CharacterClearBuff(o);

    o->Teleport = TELEPORT_NONE;
    o->Kind = KIND_PLAYER;
    c->Change = false;
    o->SubType = 0;
    c->MonsterIndex = MONSTER_UNDEFINED;
    o->BlendMeshTexCoordU = 0.f;
    o->BlendMeshTexCoordV = 0.f;
    c->Skill = 0;
    c->AttackTime = 0;
    c->LastAttackEffectTime = -1;
    c->TargetCharacter = -1;
    c->AttackFlag = ATTACK_FAIL;
    c->Weapon[0].Type = -1;
    c->Weapon[0].Level = 0;
    c->Weapon[1].Type = -1;
    c->Weapon[1].Level = 0;
    c->Wing.Type = -1;
    c->Helper.Type = -1;

    o->Position[0] = (float)((c->PositionX) * TERRAIN_SCALE) + 0.5f * TERRAIN_SCALE;
    o->Position[1] = (float)((c->PositionY) * TERRAIN_SCALE) + 0.5f * TERRAIN_SCALE;

    o->InitialSceneTime = WorldTime;

    if (gMapManager.WorldActive == -1 || c->Helper.Type != MODEL_HORN_OF_DINORANT || c->SafeZone)
    {
        o->Position[2] = RequestTerrainHeight(o->Position[0], o->Position[1]);
    }
    else
    {
        if (gMapManager.WorldActive == WD_8TARKAN || gMapManager.WorldActive == WD_10HEAVEN)
            o->Position[2] = RequestTerrainHeight(o->Position[0], o->Position[1]) + 90.f;
        else
            o->Position[2] = RequestTerrainHeight(o->Position[0], o->Position[1]) + 30.f;
    }

    Vector(0.f, 0.f, Rotation, o->Angle);
    Vector(0.f, 0.f, 0.f, c->Light);
    Vector(0.f, 0.f, 0.f, o->Light);
    Vector(-60.f, -60.f, 0.f, o->BoundingBoxMin);
    switch (Type)
    {
    case MODEL_PLAYER:
        Vector(40.f, 40.f, 120.f, o->BoundingBoxMax);
        break;
    case MODEL_CHAOS_CASTLE_KNIGHT:
    case MODEL_CHAOS_CASTLE_ELF:
    case MODEL_CHAOS_CASTLE_WIZARD:
        Vector(40.f, 40.f, 120.f, o->BoundingBoxMax);
        break;
    case MODEL_BUDGE_DRAGON:
    case MODEL_LARVA:
    case MODEL_SPIDER:
    case MODEL_CHAIN_SCORPION:
    case MODEL_GOBLIN:
    case MODEL_WORM:
        Vector(50.f, 50.f, 80.f, o->BoundingBoxMax);
        break;
    case MODEL_GORGON:
    case MODEL_DRAGON_:
    case MODEL_TITAN:
    case MODEL_TANTALLOS:
    case MODEL_BEAM_KNIGHT:
        Vector(70.f, 70.f, 250.f, o->BoundingBoxMax);
        break;
    case MODEL_HYDRA:
        Vector(100.f, 100.f, 150.f, o->BoundingBoxMax);
        break;
    case MODEL_CASTLE_GATE:
        Vector(-120.f, -120.f, 0.f, o->BoundingBoxMin);
        Vector(100.f, 100.f, 300.f, o->BoundingBoxMax);
        break;
    case MODEL_STATUE_OF_SAINT:
        Vector(-90.f, -50.f, 0.f, o->BoundingBoxMin);
        Vector(90.f, 50.f, 200.f, o->BoundingBoxMax);
        break;
    case MODEL_SELUPAN:
        Vector(-150.f, -150.f, 0.f, o->BoundingBoxMin);
        Vector(150.f, 150.f, 400.f, o->BoundingBoxMax);
        break;
    case MODEL_SPIDER_EGGS_1:
    case MODEL_SPIDER_EGGS_2:
    case MODEL_SPIDER_EGGS_3:
        Vector(-100.f, -100.f, 0.f, o->BoundingBoxMin);
        Vector(100.f, 100.f, 200.f, o->BoundingBoxMax);
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
        Vector(-160.f, -60.f, -20.f, o->BoundingBoxMin);
        Vector(10.f, 80.f, 50.f, o->BoundingBoxMax);
    }
    break;
    case MODEL_ZOMBIE_FIGHTER:
    {
        Vector(-100.f, -70.f, 0.f, o->BoundingBoxMin);
        Vector(100.f, 70.f, 150.f, o->BoundingBoxMax);
    }
    break;
    case MODEL_GLADIATOR:
    {
        Vector(-100.f, -100.f, 50.f, o->BoundingBoxMin);
        Vector(100.f, 100.f, 150.f, o->BoundingBoxMax);
    }
    break;
    case MODEL_SLAUGHTERER:
    {
        Vector(-100.f, -100.f, 0.f, o->BoundingBoxMin);
        Vector(100.f, 100.f, 180.f, o->BoundingBoxMax);
    }
    break;
    case MODEL_BLOOD_ASSASSIN:
    {
        Vector(-80.f, -80.f, 0.f, o->BoundingBoxMin);
        Vector(80.f, 80.f, 130.f, o->BoundingBoxMax);
    }
    break;
    case MODEL_CRUEL_BLOOD_ASSASSIN:
    {
        Vector(-80.f, -80.f, 0.f, o->BoundingBoxMin);
        Vector(80.f, 80.f, 130.f, o->BoundingBoxMax);
    }
    break;
    case MODEL_LAVA_GIANT:
    {
        Vector(-100.f, -80.f, 50.f, o->BoundingBoxMin);
        Vector(100.f, 70.f, 280.f, o->BoundingBoxMax);
    }
    break;
    case MODEL_BURNING_LAVA_GIANT:
    {
        Vector(-100.f, -80.f, 50.f, o->BoundingBoxMin);
        Vector(100.f, 70.f, 280.f, o->BoundingBoxMax);
    }
    break;
    default:
        Vector(50.f, 50.f, 150.f, o->BoundingBoxMax);
        break;
    }

    if (o->BoneTransform != NULL)
    {
        delete[] o->BoneTransform;
        o->BoneTransform = NULL;
    }
    o->BoneTransform = new vec34_t[Models[Type].NumBones];

    for (int i = 0; i < 2; i++)
    {
        c->Weapon[i].Type = -1;
        c->Weapon[i].Level = 0;
        c->Weapon[i].ExcellentFlags = 0;
    }

    for (int i = 0; i < MAX_BODYPART; i++)
    {
        c->BodyPart[i].Type = -1;
        c->BodyPart[i].Level = 0;
        c->BodyPart[i].ExcellentFlags = 0;
        c->BodyPart[i].AncientDiscriminator = 0;
    }
    c->Wing.Type = -1;
    c->Helper.Type = -1;
    c->Flag.Type = -1;

    c->LongRangeAttack = -1;
    c->CollisionTime = 0;
    o->CollisionRange = 200.f;
    c->Rot = 0.f;
    c->Level = 0;
    c->Item = -1;

    for (int i = 0; i < 32; ++i) c->OwnerID[i] = 0;

    o->BlendMesh = -1;
    o->BlendMeshLight = 1.f;
    switch (Type)
    {
    case MODEL_CHAOS_CASTLE_KNIGHT:
    case MODEL_CHAOS_CASTLE_ELF:
    case MODEL_CHAOS_CASTLE_WIZARD:
        c->Weapon[0].LinkBone = 33;
        c->Weapon[1].LinkBone = 42;
        break;
    case MODEL_RED_SKELETON_KNIGHT:
        c->Weapon[0].LinkBone = 30;
        c->Weapon[1].LinkBone = 39;
        break;
    case MODEL_DARK_SKULL_SOLDIER:
        c->Weapon[0].LinkBone = 33;
        c->Weapon[1].LinkBone = 20;
        break;
    case MODEL_STATUE_OF_SAINT:
        c->Weapon[0].LinkBone = 1;
        c->Weapon[1].LinkBone = 1;
        break;
    case MODEL_DARK_PHEONIX_SHIELD:
        c->Weapon[0].LinkBone = 27;
        c->Weapon[1].LinkBone = 18;
        break;
    case MODEL_CRUST:
        c->Weapon[0].LinkBone = 36;
        c->Weapon[1].LinkBone = 45;
        break;
    case MODEL_PHANTOM_KNIGHT:
        c->Weapon[0].LinkBone = 30;
        c->Weapon[1].LinkBone = 39;
        break;
    case MODEL_ORC_ARCHER:
        c->Weapon[0].LinkBone = 39;
        c->Weapon[1].LinkBone = 39;
        break;
    case MODEL_ORC:
        c->Weapon[0].LinkBone = 27;
        c->Weapon[1].LinkBone = 38;
        break;
    case MODEL_CURSED_KING:
        c->Weapon[0].LinkBone = 32;
        c->Weapon[1].LinkBone = 43;
        break;
    case MODEL_BEAM_KNIGHT:
        c->Weapon[0].LinkBone = 55;
        c->Weapon[1].LinkBone = 70;
        break;
    case MODEL_TANTALLOS:
        c->Weapon[0].LinkBone = 43;
        break;
    case MODEL_GOLDEN_WHEEL:
        c->Weapon[0].LinkBone = 23;
        break;
    case MODEL_LIZARD:
        c->Weapon[0].LinkBone = 52;
        c->Weapon[1].LinkBone = 65;
        break;
    case MODEL_VALKYRIE:
        c->Weapon[0].LinkBone = 30;
        c->Weapon[1].LinkBone = 39;
        break;
    case MODEL_VEPAR:
        c->Weapon[0].LinkBone = 30;
        c->Weapon[1].LinkBone = 39;
        break;
    case MODEL_DEVIL:
        c->Weapon[0].LinkBone = 16;
        c->Weapon[1].LinkBone = 25;
        break;
    case MODEL_DEATH_KNIGHT:
        c->Weapon[0].LinkBone = 30;
        c->Weapon[1].LinkBone = 39;
        break;
    case MODEL_BALROG:
        c->Weapon[0].LinkBone = 17;
        c->Weapon[1].LinkBone = 28;
        break;
    case MODEL_AGON:
        c->Weapon[0].LinkBone = 39;
        c->Weapon[1].LinkBone = 30;
        break;
    case MODEL_HUNTER:
        c->Weapon[0].LinkBone = 25;
        c->Weapon[1].LinkBone = 16;
        break;
    case MODEL_BEETLE_MONSTER:
        c->Weapon[0].LinkBone = 24;
        c->Weapon[1].LinkBone = 19;
        break;
    case MODEL_GOBLIN:
        c->Weapon[0].LinkBone = 31;
        c->Weapon[1].LinkBone = 22;
        break;
    case MODEL_ICE_QUEEN:
        c->Weapon[0].LinkBone = 26;
        c->Weapon[1].LinkBone = 35;
        break;
    case MODEL_HOMMERD:
    case MODEL_GORGON:
        c->Weapon[0].LinkBone = 30;
        c->Weapon[1].LinkBone = 39;
        break;
    case MODEL_DARK_KNIGHT:
        c->Weapon[0].LinkBone = 26;
        c->Weapon[1].LinkBone = 36;
        break;
    case MODEL_BULL_FIGHTER:
    case MODEL_DEATH_COW:
        c->Weapon[0].LinkBone = 42;
        c->Weapon[1].LinkBone = 33;
        break;
    case MODEL_CYCLOPS:
    case MODEL_LICH:
    case MODEL_GIANT:
        c->Weapon[0].LinkBone = 41;
        c->Weapon[1].LinkBone = 32;
        break;
    case MODEL_HOUND:
        c->Weapon[0].LinkBone = 19;
        c->Weapon[1].LinkBone = 14;
        break;
    case MODEL_HELL_SPIDER:
        c->Weapon[0].LinkBone = 29;
        c->Weapon[1].LinkBone = 38;
        break;
    case MODEL_SOLDIER:
        c->Weapon[0].LinkBone = 20;
        c->Weapon[1].LinkBone = 33;
        break;
    default:
        if (SettingHellasMonsterLinkBone(c, Type)) return;
        if (battleCastle::SettingBattleCastleMonsterLinkBone(c, Type)) return;
        c->Weapon[0].LinkBone = 33;
        c->Weapon[1].LinkBone = 42;
        break;
    }
}

CHARACTER* CreateCharacter(int Key, int Type, unsigned char PositionX, unsigned char PositionY, float Rotation)
{
    for (int i = 0; i < MAX_CHARACTERS_CLIENT; i++)
    {
        CHARACTER* c = &CharactersClient[i];
        OBJECT* o = &c->Object;
        if (o->Live && c->Key == Key)
        {
            CreateCharacterPointer(c, Type, PositionX, PositionY, Rotation);
            g_CharacterClearBuff(o);
            return c;
        }
    }

    for (int i = 0; i < MAX_CHARACTERS_CLIENT; i++)
    {
        CHARACTER* c = &CharactersClient[i];
        OBJECT* o = &c->Object;
        if (!o->Live)
        {
            BoneManager::UnregisterBone(c);
            DeletePet(c);
            DeleteCloth(c, o);
            DeleteParts(c);
            CreateCharacterPointer(c, Type, PositionX, PositionY, Rotation);
            g_CharacterClearBuff(o);
            c->Key = Key;
            return c;
        }
    }

    return &CharactersClient[MAX_CHARACTERS_CLIENT];
}

void SetCharacterScale(CHARACTER* c)
{
    if (c->Change)
        return;

    if (c->BodyPart[BODYPART_HELM].Type == MODEL_HELM ||
        c->BodyPart[BODYPART_HELM].Type == MODEL_PAD_HELM ||
        c->BodyPart[BODYPART_HELM].Type == MODEL_HELM + 63 ||
        c->BodyPart[BODYPART_HELM].Type == MODEL_HELM + 68 ||
        c->BodyPart[BODYPART_HELM].Type == MODEL_HELM + 65 ||
        c->BodyPart[BODYPART_HELM].Type == MODEL_HELM + 70 ||
        (c->BodyPart[BODYPART_HELM].Type >= MODEL_VINE_HELM && c->BodyPart[BODYPART_HELM].Type <= MODEL_SPIRIT_HELM))
    {
        c->BodyPart[BODYPART_HEAD].Type = MODEL_BODY_HELM + c->SkinIndex;
    }
    else
    {
        c->BodyPart[BODYPART_HEAD].Type = -1;
    }

    if (SceneFlag == CHARACTER_SCENE)
    {
        switch (gCharacterManager.GetBaseClass(c->Class))
        {
        case CLASS_RAGEFIGHTER:	c->Object.Scale = 1.35f; break;
        default: c->Object.Scale = 1.2f; break;
        }
    }
    else
    {
        if (c->Skin == 0)
        {
            switch (gCharacterManager.GetBaseClass(c->Class))
            {
            case CLASS_WIZARD:    c->Object.Scale = 0.9f; break;
            case CLASS_KNIGHT:    c->Object.Scale = 0.9f; break;
            case CLASS_ELF:    c->Object.Scale = 0.88f; break;
            case CLASS_DARK:    c->Object.Scale = 0.95f; break;
            case CLASS_DARK_LORD: c->Object.Scale = 0.92f; break;
            case CLASS_SUMMONER:	c->Object.Scale = 0.90f; break;
            case CLASS_RAGEFIGHTER:	c->Object.Scale = 1.03f; break;
            }
        }
        else
        {
            switch (gCharacterManager.GetBaseClass(c->Class))
            {
            case CLASS_WIZARD:    c->Object.Scale = 0.93f; break;
            case CLASS_KNIGHT:    c->Object.Scale = 0.93f; break;
            case CLASS_ELF:    c->Object.Scale = 0.86f; break;
            case CLASS_DARK:    c->Object.Scale = 0.95f; break;
            case CLASS_DARK_LORD: c->Object.Scale = 0.92f; break;
            case CLASS_SUMMONER:	c->Object.Scale = 0.90f; break;
            case CLASS_RAGEFIGHTER:	c->Object.Scale = 1.03f; break;
            }
        }
    }
}

void SetCharacterClass(CHARACTER* c)
{
    if (c->Object.Type != MODEL_PLAYER)
    {
        return;
    }

    ITEM* p = CharacterMachine->Equipment;

    if (p[EQUIPMENT_WEAPON_RIGHT].Type == -1)
    {
        c->Weapon[0].Type = -1;
    }
    else
    {
        c->Weapon[0].Type = p[EQUIPMENT_WEAPON_RIGHT].Type + MODEL_ITEM;
    }

    if (p[EQUIPMENT_WEAPON_LEFT].Type == -1)
    {
        c->Weapon[1].Type = -1;
    }
    else
    {
        c->Weapon[1].Type = p[EQUIPMENT_WEAPON_LEFT].Type + MODEL_ITEM;
    }

    if (p[EQUIPMENT_WING].Type == -1)
    {
        c->Wing.Type = -1;
    }
    else
    {
        c->Wing.Type = p[EQUIPMENT_WING].Type + MODEL_ITEM;
    }

    if (p[EQUIPMENT_HELPER].Type == -1)
    {
        c->Helper.Type = -1;
    }
    else
    {
        c->Helper.Type = p[EQUIPMENT_HELPER].Type + MODEL_ITEM;
    }

    c->Weapon[0].Level = p[EQUIPMENT_WEAPON_RIGHT].Level;
    c->Weapon[1].Level = p[EQUIPMENT_WEAPON_LEFT].Level;
    c->Weapon[0].ExcellentFlags = p[EQUIPMENT_WEAPON_RIGHT].ExcellentFlags;
    c->Weapon[1].ExcellentFlags = p[EQUIPMENT_WEAPON_LEFT].ExcellentFlags;
    c->Weapon[0].AncientDiscriminator = p[EQUIPMENT_WEAPON_RIGHT].AncientDiscriminator;
    c->Weapon[1].AncientDiscriminator = p[EQUIPMENT_WEAPON_LEFT].AncientDiscriminator;
    c->Wing.Level = p[EQUIPMENT_WING].Level;
    c->Helper.Level = p[EQUIPMENT_HELPER].Level;

    bool Success = true;

    if (gMapManager.InChaosCastle() == true)
        Success = false;

    if (c->Object.CurrentAction >= PLAYER_SIT1 && c->Object.CurrentAction <= PLAYER_POSE_FEMALE1)
    {
        Success = false;
    }
    if (c->Object.CurrentAction >= PLAYER_ATTACK_FIST && c->Object.CurrentAction <= PLAYER_RIDE_SKILL)
    {
        Success = false;
    }

    if (Success)
    {
        SetPlayerStop(c);
    }

    if (p[EQUIPMENT_HELM].Type == -1)
    {
        c->BodyPart[BODYPART_HELM].Type = MODEL_BODY_HELM + c->SkinIndex;
        c->BodyPart[BODYPART_HELM].Level = 0;
        c->BodyPart[BODYPART_HELM].ExcellentFlags = 0;
        c->BodyPart[BODYPART_HELM].AncientDiscriminator = 0;
    }
    else
    {
        c->BodyPart[BODYPART_HELM].Type = p[EQUIPMENT_HELM].Type + MODEL_ITEM;
        c->BodyPart[BODYPART_HELM].Level = p[EQUIPMENT_HELM].Level;
        c->BodyPart[BODYPART_HELM].ExcellentFlags = p[EQUIPMENT_HELM].ExcellentFlags;
        c->BodyPart[BODYPART_HELM].AncientDiscriminator = p[EQUIPMENT_HELM].AncientDiscriminator;
    }

    if (p[EQUIPMENT_ARMOR].Type == -1)
    {
        c->BodyPart[BODYPART_ARMOR].Type = MODEL_BODY_ARMOR + c->SkinIndex;
        c->BodyPart[BODYPART_ARMOR].Level = 0;
        c->BodyPart[BODYPART_ARMOR].ExcellentFlags = 0;
        c->BodyPart[BODYPART_ARMOR].AncientDiscriminator = 0;
    }
    else
    {
        c->BodyPart[BODYPART_ARMOR].Type = p[EQUIPMENT_ARMOR].Type + MODEL_ITEM;
        c->BodyPart[BODYPART_ARMOR].Level = p[EQUIPMENT_ARMOR].Level;
        c->BodyPart[BODYPART_ARMOR].ExcellentFlags = p[EQUIPMENT_ARMOR].ExcellentFlags;
        c->BodyPart[BODYPART_ARMOR].AncientDiscriminator = p[EQUIPMENT_ARMOR].AncientDiscriminator;
    }

    if (p[EQUIPMENT_PANTS].Type == -1)
    {
        c->BodyPart[BODYPART_PANTS].Type = MODEL_BODY_PANTS + c->SkinIndex;
        c->BodyPart[BODYPART_PANTS].Level = 0;
        c->BodyPart[BODYPART_PANTS].ExcellentFlags = 0;
        c->BodyPart[BODYPART_PANTS].AncientDiscriminator = 0;
    }
    else
    {
        c->BodyPart[BODYPART_PANTS].Type = p[EQUIPMENT_PANTS].Type + MODEL_ITEM;
        c->BodyPart[BODYPART_PANTS].Level = p[EQUIPMENT_PANTS].Level;
        c->BodyPart[BODYPART_PANTS].ExcellentFlags = p[EQUIPMENT_PANTS].ExcellentFlags;
        c->BodyPart[BODYPART_PANTS].AncientDiscriminator = p[EQUIPMENT_PANTS].AncientDiscriminator;
    }

    if (p[EQUIPMENT_GLOVES].Type == -1)
    {
        c->BodyPart[BODYPART_GLOVES].Type = MODEL_BODY_GLOVES + c->SkinIndex;
        c->BodyPart[BODYPART_GLOVES].Level = 0;
        c->BodyPart[BODYPART_GLOVES].ExcellentFlags = 0;
        c->BodyPart[BODYPART_GLOVES].AncientDiscriminator = 0;
    }
    else
    {
        c->BodyPart[BODYPART_GLOVES].Type = p[EQUIPMENT_GLOVES].Type + MODEL_ITEM;
        c->BodyPart[BODYPART_GLOVES].Level = p[EQUIPMENT_GLOVES].Level;
        c->BodyPart[BODYPART_GLOVES].ExcellentFlags = p[EQUIPMENT_GLOVES].ExcellentFlags;
        c->BodyPart[BODYPART_GLOVES].AncientDiscriminator = p[EQUIPMENT_GLOVES].AncientDiscriminator;
    }

    if (p[EQUIPMENT_BOOTS].Type == -1)
    {
        c->BodyPart[BODYPART_BOOTS].Type = MODEL_BODY_BOOTS + c->SkinIndex;
        c->BodyPart[BODYPART_BOOTS].Level = 0;
        c->BodyPart[BODYPART_BOOTS].ExcellentFlags = 0;
        c->BodyPart[BODYPART_BOOTS].AncientDiscriminator = 0;
    }
    else
    {
        c->BodyPart[BODYPART_BOOTS].Type = p[EQUIPMENT_BOOTS].Type + MODEL_ITEM;
        c->BodyPart[BODYPART_BOOTS].Level = p[EQUIPMENT_BOOTS].Level;
        c->BodyPart[BODYPART_BOOTS].ExcellentFlags = p[EQUIPMENT_BOOTS].ExcellentFlags;
        c->BodyPart[BODYPART_BOOTS].AncientDiscriminator = p[EQUIPMENT_BOOTS].AncientDiscriminator;
    }

    ChangeChaosCastleUnit(c);

    SetCharacterScale(c);

    if (c == Hero)
    {
        CheckFullSet(Hero);
    }

    CharacterMachine->CalculateAll();
}

void SetChangeClass(CHARACTER* c)
{
    if (c->Object.Type != MODEL_PLAYER)
        return;

    bool Success = true;

    if (c->Object.CurrentAction >= PLAYER_SIT1 && c->Object.CurrentAction <= PLAYER_POSE_FEMALE1)
        Success = false;
    if (c->Object.CurrentAction >= PLAYER_ATTACK_FIST && c->Object.CurrentAction <= PLAYER_RIDE_SKILL)
        Success = false;
    if (Success)
        SetPlayerStop(c);

    if (c->BodyPart[BODYPART_HELM].Type >= MODEL_BODY_HELM)
    {
        c->BodyPart[BODYPART_HELM].Type = MODEL_BODY_HELM + c->SkinIndex;
        c->BodyPart[BODYPART_HELM].Level = 0;
        c->BodyPart[BODYPART_HELM].ExcellentFlags = 0;
        c->BodyPart[BODYPART_HELM].AncientDiscriminator = 0;
    }
    if (c->BodyPart[BODYPART_ARMOR].Type >= MODEL_BODY_ARMOR)
    {
        c->BodyPart[BODYPART_ARMOR].Type = MODEL_BODY_ARMOR + c->SkinIndex;
        c->BodyPart[BODYPART_ARMOR].Level = 0;
        c->BodyPart[BODYPART_ARMOR].ExcellentFlags = 0;
        c->BodyPart[BODYPART_ARMOR].AncientDiscriminator = 0;
    }
    if (c->BodyPart[BODYPART_PANTS].Type >= MODEL_BODY_PANTS)
    {
        c->BodyPart[BODYPART_PANTS].Type = MODEL_BODY_PANTS + c->SkinIndex;
        c->BodyPart[BODYPART_PANTS].Level = 0;
        c->BodyPart[BODYPART_PANTS].ExcellentFlags = 0;
        c->BodyPart[BODYPART_PANTS].AncientDiscriminator = 0;
    }
    if (c->BodyPart[BODYPART_GLOVES].Type >= MODEL_BODY_GLOVES)
    {
        c->BodyPart[BODYPART_GLOVES].Type = MODEL_BODY_GLOVES + c->SkinIndex;
        c->BodyPart[BODYPART_GLOVES].Level = 0;
        c->BodyPart[BODYPART_GLOVES].ExcellentFlags = 0;
        c->BodyPart[BODYPART_GLOVES].AncientDiscriminator = 0;
    }
    if (c->BodyPart[BODYPART_BOOTS].Type >= MODEL_BODY_BOOTS)
    {
        c->BodyPart[BODYPART_BOOTS].Type = MODEL_BODY_BOOTS + c->SkinIndex;
        c->BodyPart[BODYPART_BOOTS].Level = 0;
        c->BodyPart[BODYPART_BOOTS].ExcellentFlags = 0;
        c->BodyPart[BODYPART_BOOTS].AncientDiscriminator = 0;
    }

    SetCharacterScale(c);
}

DWORD GetGuildRelationShipTextColor(BYTE GuildRelationShip)
{
    DWORD dwColor = 0;

    if (GuildRelationShip == GR_NONE)
        dwColor = (255 << 24) + (255 << 16) + (230 << 8) + (230);
    else if (GuildRelationShip == GR_RIVAL || GuildRelationShip == GR_RIVALUNION)
        dwColor = (255 << 24) + (0 << 16) + (30 << 8) + (255);
    else
        dwColor = (255 << 24) + (0 << 16) + (255 << 8) + (200);

    return dwColor;
}

DWORD GetGuildRelationShipBGColor(BYTE GuildRelationShip)
{
    DWORD dwColor = 0;

    if (GuildRelationShip == GR_NONE)
        dwColor = (150 << 24) + (50 << 16) + (30 << 8) + (10);
    else if (GuildRelationShip == GR_RIVAL || GuildRelationShip == GR_RIVALUNION)
        dwColor = (150 << 24) + (0 << 16) + (0 << 8) + (0);
    else
        dwColor = (150 << 24) + (80 << 16) + (50 << 8) + (20);

    return dwColor;
}

CHARACTER* FindCharacterByID(wchar_t* szName)
{
    for (int i = 0; i < MAX_CHARACTERS_CLIENT; i++)
    {
        CHARACTER* c = &CharactersClient[i];
        if (c->Object.Live && !wcscmp(szName, c->ID))
        {
            return c;
        }
    }
    return NULL;
}



CHARACTER* FindCharacterByKey(int Key)
{
    for (int i = 0; i < MAX_CHARACTERS_CLIENT; i++)
    {
        CHARACTER* c = &CharactersClient[i];
        if (c->Object.Live && c->Key == Key)
        {
            return c;
        }
    }
    return NULL;
}

int LevelConvert(BYTE Level)
{
    switch (Level)
    {
    case 0:return 0; break;
    case 1:return 3; break;
    case 2:return 5; break;
    case 3:return 7; break;
    case 4:return 9; break;
    case 5:return 11; break;
    case 6:return 13; break;
    case 7:return 15; break;
    }
    return 0;
}

void MakeElfHelper(CHARACTER* c)
{
    OBJECT* o = &c->Object;
    c->Wing.Type = MODEL_WINGS_OF_SPIRITS;
    c->BodyPart[BODYPART_HELM].Type = MODEL_RED_SPIRIT_HELM;
    c->BodyPart[BODYPART_ARMOR].Type = MODEL_RED_SPRIT_ARMOR;
    c->BodyPart[BODYPART_PANTS].Type = MODEL_RED_SPIRIT_PANTS;
    c->BodyPart[BODYPART_GLOVES].Type = MODEL_RED_SPIRIT_GLOVES;
    c->BodyPart[BODYPART_BOOTS].Type = MODEL_RED_SPIRIT_BOOTS;
    c->BodyPart[BODYPART_HELM].Level = 13;
    c->BodyPart[BODYPART_ARMOR].Level = 13;
    c->BodyPart[BODYPART_PANTS].Level = 13;
    c->BodyPart[BODYPART_GLOVES].Level = 13;
    c->BodyPart[BODYPART_BOOTS].Level = 13;

    o->Scale = 1.f;
    o->CurrentAction = PLAYER_STOP_FLY;
    o->BoundingBoxMax[2] += 70.f;
}

void ChangeCharacterExt(int Key, BYTE* Equipment, CHARACTER* pCharacter, OBJECT* pHelper)
{
    CHARACTER* c;
    if (pCharacter == NULL)
        c = &CharactersClient[Key];
    else
        c = pCharacter;

    OBJECT* o = &c->Object;
    if (o->Type != MODEL_PLAYER)
        return;

    BYTE Type = 0;
    BYTE ExtBit = 0;
    short ExtType = 0;

    Type = Equipment[0];
    ExtType = Equipment[11] & 0xF0;
    ExtType = (ExtType << 4) | Type;

    int ItemLevels = ((int)Equipment[5] << 16) + ((int)Equipment[6] << 8) + ((int)Equipment[7]);
    c->Wing.Level = 0;
    c->Helper.Level = 0;
    if (ExtType == 0x0FFF)
    {
        c->Weapon[0].Type = -1;
        c->Weapon[0].ExcellentFlags = 0;
        c->Weapon[0].AncientDiscriminator = 0;
    }
    else
    {
        c->Weapon[0].Type = MODEL_SWORD + ExtType;
        c->Weapon[0].Level = LevelConvert((ItemLevels >> 0) & 7);
        c->Weapon[0].ExcellentFlags = (Equipment[9] & 4) / 4;
        c->Weapon[0].AncientDiscriminator = (Equipment[10] & 4) / 4;
    }

    Type = Equipment[1];
    ExtType = Equipment[12] & 240;
    ExtType = (ExtType << 4) | Type;

    if (ExtType == 0x0FFF)
    {
        c->Weapon[1].Type = -1;
        c->Weapon[1].ExcellentFlags = 0;
        c->Weapon[1].AncientDiscriminator = 0;
    }
    else
    {
        if (gCharacterManager.GetBaseClass(c->Class) == CLASS_DARK_LORD && ((MODEL_LEGENDARY_STAFF) - MODEL_SWORD) == ExtType)
        {
            ITEM* pEquipmentItemSlot = &CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT];
            PET_INFO* pPetInfo = giPetManager::GetPetInfo(pEquipmentItemSlot);
            giPetManager::CreatePetDarkSpirit(c);
            if (!gMapManager.InChaosCastle())
                ((CSPetSystem*)c->m_pPet)->SetPetInfo(pPetInfo);
        }
        else
        {
            c->Weapon[1].Type = MODEL_SWORD + ExtType;
        }

        c->Weapon[1].Level = LevelConvert((ItemLevels >> 3) & 7);
        c->Weapon[1].ExcellentFlags = (Equipment[9] & 2) / 2;
        c->Weapon[1].AncientDiscriminator = (Equipment[10] & 2) / 2;
    }

    Type = (Equipment[4] >> 2) & 3;

    //신규캐릭터 추가로 인한 날개 인덱스 확장 구조변경
    if (Type == 1)			//1차 날개
    {
        Type = Equipment[8] & 0x07;
        switch (Type)
        {
        case 4:
            c->Wing.Type = MODEL_WING_OF_CURSE;
            break;
        default:
            c->Wing.Type = MODEL_WING + Type - 1;
            break;
        }
    }
    else if (Type == 2)		//2차 날개
    {
        Type = Equipment[8] & 0x07;
        switch (Type)
        {
        case 5:		c->Wing.Type = MODEL_CAPE_OF_LORD; break;
        case 6:		c->Wing.Type = MODEL_WINGS_OF_DESPAIR; break;
        case 7:		c->Wing.Type = MODEL_CAPE_OF_FIGHTER; break;
        default:
            c->Wing.Type = MODEL_WINGS_OF_SATAN + Type;
            break;
        }
    }
    else if (Type == 3)		//3차 날개
    {
        Type = Equipment[8] & 0x07;
        switch (Type)
        {
        case 0:				//작은날개
        {
            Type = (Equipment[16] >> 5);
            c->Wing.Type = MODEL_SEED_SPHERE_EARTH_5 + Type;
        }
        break;
        case 6:		c->Wing.Type = MODEL_WING_OF_DIMENSION; break;
        case 7:		c->Wing.Type = MODEL_CAPE_OF_OVERRULE; break;
        default:
            c->Wing.Type = MODEL_SCROLL_OF_FIRE_SCREAM + Type;
            break;
        }
    }
    else
    {
        c->Wing.Type = -1;
        c->Wing.ExcellentFlags = 0;
        c->Wing.AncientDiscriminator = 0;
    }

    if (pHelper == NULL)
    {
        DeleteMount(o);
        ThePetProcess().DeletePet(c, c->Helper.Type, true);
    }
    else
    {
        pHelper->Live = false;
    }
    Type = Equipment[4] & 3;
    if (Type == 3)
    {
        Type = Equipment[9] & 0x01;
        if (Type == 1)
        {
            c->Helper.Type = MODEL_HORN_OF_DINORANT;
            if (pHelper == NULL)
                CreateMount(MODEL_PEGASUS, o->Position, o);
            else
                CreateMountSub(MODEL_PEGASUS, o->Position, o, pHelper);
        }
        else
        {
            c->Helper.Type = -1;
            c->Helper.ExcellentFlags = 0;
            c->Helper.AncientDiscriminator = 0;
        }
    }
    else
    {
        BYTE _temp = Equipment[15] & 0xE0;

        if (32 == _temp || 64 == _temp || 128 == _temp || 224 == _temp || 160 == _temp || 96 == _temp)
        {
            short _type = 0;
            switch (_temp)
            {
            case MONSTER_STONE_GOLEM: _type = 64; break;
            case MONSTER_ORC_ARCHER: _type = 65; break;
            case MONSTER_GIANT_OGRE_6: _type = 67; break;
            case MONSTER_GUARDSMAN: _type = 80; break;
            case MONSTER_SCHRIKER_1: _type = 106; break;
            case MONSTER_CHIEF_SKELETON_WARRIOR_3: _type = 123; break;
            }

            c->Helper.Type = MODEL_HELPER + _type;

            ThePetProcess().CreatePet(ITEM_HELPER + _type, c->Helper.Type, o->Position, c);
        }
        else
        {
            c->Helper.Type = MODEL_HELPER + Type;
            int HelperType = 0;
            BOOL bCreateHelper = TRUE;
            switch (Type)
            {
            case 0:HelperType = MODEL_HELPER; break;
            case 2:HelperType = MODEL_UNICON; break;
            case 3:HelperType = MODEL_PEGASUS; break;
            default:bCreateHelper = FALSE; break;
            }
            if (bCreateHelper == TRUE)
            {
                if (pHelper == NULL)
                    CreateMount(HelperType, o->Position, o);
                else
                    CreateMountSub(HelperType, o->Position, o, pHelper);
            }
        }
    }

    Type = Equipment[11] & 0x01;
    if (Type == 1)
    {
        c->Helper.Type = MODEL_DARK_HORSE_ITEM;
        if (pHelper == NULL)
            CreateMount(MODEL_DARK_HORSE, o->Position, o);
        else
            CreateMountSub(MODEL_DARK_HORSE, o->Position, o, pHelper);
    }

    Type = Equipment[11] & 0x04;
    if (Type == 4)
    {
        c->Helper.Type = MODEL_HORN_OF_FENRIR;

        Type = Equipment[15] & 3;

        int iFenrirType = Equipment[16] & 1;
        if (iFenrirType == 1)
        {
            Type = 0x04;
        }

        c->Helper.ExcellentFlags = Type;
        if (Type == 0x01)
        {
            if (pHelper == NULL)
                CreateMount(MODEL_FENRIR_BLACK, o->Position, o);
            else
                CreateMountSub(MODEL_FENRIR_BLACK, o->Position, o, pHelper);
        }
        else if (Type == 0x02)
        {
            if (pHelper == NULL)
                CreateMount(MODEL_FENRIR_BLUE, o->Position, o);
            else
                CreateMountSub(MODEL_FENRIR_BLUE, o->Position, o, pHelper);
        }
        else if (Type == 0x04)
        {
            if (pHelper == NULL)
                CreateMount(MODEL_FENRIR_GOLD, o->Position, o);
            else
                CreateMountSub(MODEL_FENRIR_GOLD, o->Position, o, pHelper);
        }
        else
        {
            if (pHelper == NULL)
                CreateMount(MODEL_FENRIR_RED, o->Position, o);
            else
                CreateMountSub(MODEL_FENRIR_RED, o->Position, o, pHelper);
        }
    }

    DeleteParts(c);
    Type = (Equipment[11] >> 1) & 0x01;
    if (Type == 1)
    {
        if (c->EtcPart <= 0 || c->EtcPart > 3)
        {
            c->EtcPart = PARTS_LION;
        }
    }
    else
    {
        if (c->EtcPart <= 0 || c->EtcPart > 3)
        {
            c->EtcPart = 0;
        }
    }

    if (c->Change)
        return;

    ExtType = (Equipment[2] >> 4) + ((Equipment[8] >> 7) & 1) * 16 + (Equipment[12] & 15) * 32;
    if (ExtType == 0x1FF)
    {
        c->BodyPart[BODYPART_HELM].Type = MODEL_BODY_HELM + c->SkinIndex;
        c->BodyPart[BODYPART_HELM].Level = 0;
        c->BodyPart[BODYPART_HELM].ExcellentFlags = 0;
        c->BodyPart[BODYPART_HELM].AncientDiscriminator = 0;
    }
    else
    {
        c->BodyPart[BODYPART_HELM].Type = MODEL_HELM + ExtType;
        c->BodyPart[BODYPART_HELM].Level = LevelConvert((ItemLevels >> 6) & 7);
        c->BodyPart[BODYPART_HELM].ExcellentFlags = (Equipment[9] & 128) / 128;
        c->BodyPart[BODYPART_HELM].AncientDiscriminator = (Equipment[10] & 128) / 128;
        
    }

    ExtType = (Equipment[2] & 15) + ((Equipment[8] >> 6) & 1) * 16 + ((Equipment[13] >> 4) & 15) * 32;
    if (ExtType == 0x1FF)
    {
        c->BodyPart[BODYPART_ARMOR].Type = MODEL_BODY_ARMOR + c->SkinIndex;
        c->BodyPart[BODYPART_ARMOR].Level = 0;
        c->BodyPart[BODYPART_ARMOR].ExcellentFlags = 0;
        c->BodyPart[BODYPART_ARMOR].AncientDiscriminator = 0;
    }
    else
    {
        c->BodyPart[BODYPART_ARMOR].Type = MODEL_ARMOR + ExtType;
        c->BodyPart[BODYPART_ARMOR].Level = LevelConvert((ItemLevels >> 9) & 7);
        c->BodyPart[BODYPART_ARMOR].ExcellentFlags = (Equipment[9] & 64) / 64;
        c->BodyPart[BODYPART_ARMOR].AncientDiscriminator = (Equipment[10] & 64) / 64;
    }

    ExtType = (Equipment[3] >> 4) + ((Equipment[8] >> 5) & 1) * 16 + (Equipment[13] & 15) * 32;
    if (ExtType == 0x1FF)
    {
        c->BodyPart[BODYPART_PANTS].Type = MODEL_BODY_PANTS + c->SkinIndex;
        c->BodyPart[BODYPART_PANTS].Level = 0;
        c->BodyPart[BODYPART_PANTS].ExcellentFlags = 0;
        c->BodyPart[BODYPART_PANTS].AncientDiscriminator = 0;
    }
    else
    {
        c->BodyPart[BODYPART_PANTS].Type = MODEL_PANTS + ExtType;
        c->BodyPart[BODYPART_PANTS].Level = LevelConvert((ItemLevels >> 12) & 7);
        c->BodyPart[BODYPART_PANTS].ExcellentFlags = (Equipment[9] & 32) / 32;
        c->BodyPart[BODYPART_PANTS].AncientDiscriminator = (Equipment[10] & 32) / 32;
    }

    ExtType = (Equipment[3] & 15) + ((Equipment[8] >> 4) & 1) * 16 + ((Equipment[14] >> 4) & 15) * 32;
    if (ExtType == 0x1FF)
    {
        c->BodyPart[BODYPART_GLOVES].Type = MODEL_BODY_GLOVES + c->SkinIndex;
        c->BodyPart[BODYPART_GLOVES].Level = 0;
        c->BodyPart[BODYPART_GLOVES].ExcellentFlags = 0;
        c->BodyPart[BODYPART_GLOVES].AncientDiscriminator = 0;
    }
    else
    {
        c->BodyPart[BODYPART_GLOVES].Type = MODEL_GLOVES + ExtType;
        c->BodyPart[BODYPART_GLOVES].Level = LevelConvert((ItemLevels >> 15) & 7);
        c->BodyPart[BODYPART_GLOVES].ExcellentFlags = (Equipment[9] & 16) / 16;
        c->BodyPart[BODYPART_GLOVES].AncientDiscriminator = (Equipment[10] & 16) / 16;
    }

    ExtType = (Equipment[4] >> 4) + ((Equipment[8] >> 3) & 1) * 16 + (Equipment[14] & 15) * 32;
    if (ExtType == 0x1FF)
    {
        c->BodyPart[BODYPART_BOOTS].Type = MODEL_BODY_BOOTS + c->SkinIndex;
        c->BodyPart[BODYPART_BOOTS].Level = 0;
        c->BodyPart[BODYPART_BOOTS].ExcellentFlags = 0;
        c->BodyPart[BODYPART_BOOTS].AncientDiscriminator = 0;
    }
    else
    {
        c->BodyPart[BODYPART_BOOTS].Type = MODEL_BOOTS + ExtType;
        c->BodyPart[BODYPART_BOOTS].Level = LevelConvert((ItemLevels >> 18) & 7);
        c->BodyPart[BODYPART_BOOTS].ExcellentFlags = (Equipment[9] & 8) / 8;
        c->BodyPart[BODYPART_BOOTS].AncientDiscriminator = (Equipment[10] & 8) / 8;
    }

    c->ExtendState = Equipment[10] & 0x01;

    ChangeChaosCastleUnit(c);
    SetCharacterScale(c);
}

void ReadEquipmentExtended(int Key, BYTE flags, BYTE* Equipment, CHARACTER* pCharacter, OBJECT* pHelper)
{
    CHARACTER* c;
    if (pCharacter == NULL)
        c = &CharactersClient[Key];
    else
        c = pCharacter;

    OBJECT* o = &c->Object;
    if (o->Type != MODEL_PLAYER)
        return;

    c->ExtendState = (flags & 0x10) > 0;
    int offset = 0;
    for (int i = 0; i < 2; i++)
    {
        c->Weapon[i].Type = -1;
        c->Weapon[i].ExcellentFlags = 0;
        c->Weapon[i].AncientDiscriminator = 0;
        if (Equipment[offset] != 0xFF && Equipment[offset + 1] != 0xFF)
        {
            short number = MAKEWORD(Equipment[offset + 1], Equipment[offset] & 0xF);
            BYTE group = (Equipment[offset] & 0xF0) >> 4;
            bool isAncient = Equipment[offset + 2] & 0x04;
            bool isExcellent = Equipment[offset + 2] & 0x08;
            BYTE glowLevel = (Equipment[offset + 2] & 0xF0) >> 4;
            if (number > MAX_ITEM_INDEX)
            {
                // not supported yet!
            }
            else if (group == ITEM_GROUP_HELPER && number == ITEM_NUMBER_DARK_SPIRIT)
            {
                ITEM* pEquipmentItemSlot = &CharacterMachine->Equipment[i];
                PET_INFO* pPetInfo = giPetManager::GetPetInfo(pEquipmentItemSlot);
                giPetManager::CreatePetDarkSpirit(c);
                if (!gMapManager.InChaosCastle())
                    ((CSPetSystem*)c->m_pPet)->SetPetInfo(pPetInfo);
            }
            else
            {
                auto modelOffset = group * MAX_ITEM_INDEX + number;
                c->Weapon[i].Type = MODEL_ITEM + modelOffset;
                c->Weapon[i].Level = LevelConvert(glowLevel);
                c->Weapon[i].ExcellentFlags = isExcellent;
                c->Weapon[i].AncientDiscriminator = isAncient;
            }
        }

        offset += 3;
    }

    DeleteParts(c);
    if (c->Change)
    {
        // todo: is this the correct place to return?
        return;
    }

    short bodyParts[] = { 0, MODEL_BODY_HELM, MODEL_BODY_ARMOR, MODEL_BODY_PANTS, MODEL_BODY_GLOVES, MODEL_BODY_BOOTS};
    for (int i = 1; i < MAX_BODYPART; i++)
    {
        c->BodyPart[i].Type = bodyParts[i] + c->SkinIndex;
        c->BodyPart[i].ExcellentFlags = 0;
        c->BodyPart[i].AncientDiscriminator = 0;
        c->BodyPart[i].Level = 0;

        if (Equipment[offset] != 0xFF && Equipment[offset + 1] != 0xFF)
        {
            short number = MAKEWORD(Equipment[offset + 1], Equipment[offset] & 0xF);
            BYTE group = (Equipment[offset] & 0xF0) >> 4;
            BYTE glowLevel = (Equipment[offset + 2] & 0xF0) >> 4;
            bool isAncient = Equipment[offset + 2] & 0x04;
            bool isExcellent = Equipment[offset + 2] & 0x08;

            if (number > MAX_ITEM_INDEX)
            {
                // not supported
            }
            else
            {
                auto modelOffset = group * MAX_ITEM_INDEX + number;
                c->BodyPart[i].Type = MODEL_ITEM + modelOffset;
                c->BodyPart[i].Level = LevelConvert(glowLevel);
                c->BodyPart[i].ExcellentFlags = isExcellent;
                c->BodyPart[i].AncientDiscriminator = isAncient;
            }
        }

        offset += 3;
    }

    // Wings:
    {
        c->Wing.Type = -1;
        c->Wing.Level = 0;
        
        if (Equipment[offset] != 0xFF && Equipment[offset + 1] != 0xFF)
        {
            short number = Equipment[offset + 1] + ((Equipment[offset] & 0xF) << 4);
            BYTE group = (Equipment[offset] & 0xF0) >> 4;
            if (number > MAX_ITEM_INDEX)
            {
                // not supported
            }
            else
            {
                auto modelOffset = group * MAX_ITEM_INDEX + number;
                c->Wing.Type = MODEL_ITEM + modelOffset;
            }
        }

        offset += 2;
    }

    // Helper:
    int HelperVariant = 0;
    {
        c->Helper.Type = -1;
        c->Helper.Level = 0;

        if (Equipment[offset] != 0xFF && Equipment[offset + 1] != 0xFF)
        {
            short number = Equipment[offset + 1] + ((Equipment[offset] & 0xF) << 8);
            short itemNumber = number & (MAX_ITEM_INDEX-1);
            HelperVariant = (Equipment[offset] & 0xE) >> 1;
            BYTE group = (Equipment[offset] & 0xF0) >> 4;
            auto modelOffset = group * MAX_ITEM_INDEX + itemNumber;
            c->Helper.Type = MODEL_ITEM + modelOffset;
        }

        // offset += 2;
    }

    if (pHelper == nullptr)
    {
        DeleteMount(o);
        ThePetProcess().DeletePet(c, c->Helper.Type, true);
    }
    else
    {
        pHelper->Live = false;
    }

    switch (c->Helper.Type)
    {
    case MODEL_GUARDIAN_ANGEL:
    case MODEL_HORN_OF_UNIRIA:
    case MODEL_HORN_OF_DINORANT:
    {
        int modelType = 0;
        switch (c->Helper.Type)
        {
        case MODEL_GUARDIAN_ANGEL:
            modelType = MODEL_HELPER; break;
        case MODEL_HORN_OF_UNIRIA:
            modelType = MODEL_UNICON; break;
        case MODEL_HORN_OF_DINORANT:
            modelType = MODEL_PEGASUS; break;
        }

        if (pHelper == NULL)
            CreateMount(modelType, o->Position, o);
        else
            CreateMountSub(modelType, o->Position, o, pHelper);
        break;
    }
    case MODEL_DEMON:
    case MODEL_SPIRIT_OF_GUARDIAN:
    case MODEL_PET_RUDOLF:
    case MODEL_PET_PANDA:
    case MODEL_PET_UNICORN:
    case MODEL_PET_SKELETON:
        ThePetProcess().CreatePet(c->Helper.Type - MODEL_ITEM, c->Helper.Type, o->Position, c);
        break;
    case MODEL_DARK_HORSE_ITEM:
        if (pHelper == NULL)
            CreateMount(MODEL_DARK_HORSE, o->Position, o);
        else
            CreateMountSub(MODEL_DARK_HORSE, o->Position, o, pHelper);
        break;
    case MODEL_HORN_OF_FENRIR:
        int type = MODEL_FENRIR_RED;
        switch(HelperVariant)
        {
        case 1: type = MODEL_FENRIR_BLACK; break;
        case 2: type = MODEL_FENRIR_BLUE; break;
        case 3: type = MODEL_FENRIR_GOLD; break;
        }
        if (pHelper == NULL)
            CreateMount(type, o->Position, o);
        else
            CreateMountSub(type, o->Position, o, pHelper);
        break;
    }

    c->EtcPart = 0;
    if ((flags & 0x20) > 0)
    {
        c->EtcPart = PARTS_LION;
    }

    ChangeChaosCastleUnit(c);
    SetCharacterScale(c);
}

extern int HeroIndex;

void Setting_Monster(CHARACTER* c, EMonsterType Type, int PositionX, int PositionY)
{
    OBJECT* o;

    int nCastle = BLOODCASTLE_NUM + (gMapManager.WorldActive - WD_11BLOODCASTLE_END);
    if (nCastle > 0 && nCastle <= BLOODCASTLE_NUM)
    {
        if (Type >= 84 && Type <= 143)
        {
            c->Level = 0;
            c->Object.Scale += int(nCastle / 3) * 0.05f;
        }
    }

    if (c != NULL)
    {
        o = &c->Object;
        for (int i = 0; i < MAX_MONSTER; i++)
        {
            if (Type == MonsterScript[i].Type)
            {
                wcscpy_s(c->ID, MAX_MONSTER_NAME + 1, MonsterScript[i].Name);
                break;
            }
        }

        c->MonsterIndex = Type;
        c->Object.ExtState = 0;
        c->TargetCharacter = HeroIndex;
        if (Type == 200)
            o->Kind = KIND_MONSTER;
        else if (Type >= 260)
            o->Kind = KIND_MONSTER;
        else if (Type > 200)
            o->Kind = KIND_NPC;
        else if (Type >= 150)
            o->Kind = KIND_MONSTER;
        else if (Type > 110)
            o->Kind = KIND_MONSTER;
        else if (Type >= 100)
            o->Kind = KIND_TRAP;
        else
            o->Kind = KIND_MONSTER;
        //c->Object.Kind = KIND_EDIT;
        //swprintf(c->ID,"%x",Key);
        if (Type == 368 || Type == 369 || Type == 370)
            o->Kind = KIND_NPC;
        if (Type == 367
            || Type == 371
            || Type == 375
            || Type == 376 || Type == 377
            || Type == 379
            || Type == 380 || Type == 381 || Type == 382
            || Type == 383 || Type == 384 || Type == 385
            || Type == 406
            || Type == 407
            || Type == 408
            || Type == 414
            || Type == 415 || Type == 416 || Type == 417
            || Type == 450
            || Type == 452 || Type == 453
            || Type == 464
            || Type == 465
            || Type == 467
            || Type == 468 || Type == 469 || Type == 470	//NPC 1~8
            || Type == 471 || Type == 472 || Type == 473
            || Type == 474 || Type == 475
            || Type == 478
            || Type == 479
            || Type == 492
            || Type == 540
            || Type == 541
            || Type == 542
            || Type == 522
            || Type == 543 || Type == 544
            || Type == 545
            || Type == 546
            || Type == 547
            || Type == 577 || Type == 578
            || Type == 579
            )
        {
            o->Kind = KIND_NPC;
        }
        if (Type >= 480 && Type <= 491)
        {
            o->Kind = KIND_MONSTER;
        }
        if (Type == 451)
        {
            o->Kind = KIND_TMP;
        }
    }
}

CHARACTER* CreateMonster(EMonsterType Type, int PositionX, int PositionY, int Key)
{
    CHARACTER* c = NULL;
    OBJECT* o;
    int Level;

    c = g_CursedTemple->CreateCharacters(Type, PositionX, PositionY, Key);
    if (c != NULL)
    {
        Setting_Monster(c, Type, PositionX, PositionY);
        return c;
    }
    c = CreateHellasMonster(Type, PositionX, PositionY, Key);
    if (c != NULL)
    {
        Setting_Monster(c, Type, PositionX, PositionY);
        return c;
    }

    c = battleCastle::CreateBattleCastleMonster(Type, PositionX, PositionY, Key);
    if (c != NULL)
    {
        Setting_Monster(c, Type, PositionX, PositionY);
        return c;
    }

    c = M31HuntingGround::CreateHuntingGroundMonster(Type, PositionX, PositionY, Key);
    if (c != NULL)
    {
        Setting_Monster(c, Type, PositionX, PositionY);
        return c;
    }

    c = M34CryingWolf2nd::CreateCryingWolf2ndMonster(Type, PositionX, PositionY, Key);
    if (c != NULL)
    {
        Setting_Monster(c, Type, PositionX, PositionY);
        return c;
    }

    c = M34CryWolf1st::CreateCryWolf1stMonster(Type, PositionX, PositionY, Key);
    if (c != NULL)
    {
        Setting_Monster(c, Type, PositionX, PositionY);
        return c;
    }

    c = M33Aida::CreateAidaMonster(Type, PositionX, PositionY, Key);
    if (c != NULL)
    {
        Setting_Monster(c, Type, PositionX, PositionY);
        return c;
    }

    c = M37Kanturu1st::CreateKanturu1stMonster(Type, PositionX, PositionY, Key);
    if (c != NULL)
    {
        Setting_Monster(c, Type, PositionX, PositionY);
        return c;
    }

    c = M38Kanturu2nd::Create_Kanturu2nd_Monster(Type, PositionX, PositionY, Key);
    if (c != NULL)
    {
        Setting_Monster(c, Type, PositionX, PositionY);
        return c;
    }

    c = M39Kanturu3rd::CreateKanturu3rdMonster(Type, PositionX, PositionY, Key);
    if (c != NULL)
    {
        Setting_Monster(c, Type, PositionX, PositionY);
        return c;
    }

    c = SEASON3A::CGM3rdChangeUp::Instance().CreateBalgasBarrackMonster(Type, PositionX, PositionY, Key);
    if (c != NULL)
    {
        Setting_Monster(c, Type, PositionX, PositionY);
        return c;
    }

    c = g_NewYearsDayEvent->CreateMonster(Type, PositionX, PositionY, Key);
    if (c != NULL)
    {
        Setting_Monster(c, Type, PositionX, PositionY);
        return c;
    }

    c = SEASON3B::GMNewTown::CreateNewTownMonster(Type, PositionX, PositionY, Key);
    if (c != NULL)
    {
        Setting_Monster(c, Type, PositionX, PositionY);
        return c;
    }

    c = SEASON3C::GMSwampOfQuiet::CreateSwampOfQuietMonster(Type, PositionX, PositionY, Key);
    if (c != NULL)
    {
        Setting_Monster(c, Type, PositionX, PositionY);
        return c;
    }

    c = g_09SummerEvent->CreateMonster(Type, PositionX, PositionY, Key);
    if (c != NULL)
    {
        Setting_Monster(c, Type, PositionX, PositionY);
        return c;
    }

    c = TheMapProcess().CreateMonster(Type, PositionX, PositionY, Key);
    if (c != NULL)
    {
        Setting_Monster(c, Type, PositionX, PositionY);
        return c;
    }

    switch (Type)
    {
    case MONSTER_GUARDSMAN:
        OpenNpc(MODEL_NPC_CLERK);        //
        c = CreateCharacter(Key, MODEL_NPC_CLERK, PositionX, PositionY);
        c->NotRotateOnMagicHit = true;
        c->Object.Scale = 1.f;
        c->Object.SubType = rand() % 2 + 10;
        c->Weapon[0].Type = -1;
        c->Weapon[1].Type = -1;
        wcscpy(c->ID, L"Clerk");
        break;
#ifdef ADD_ELF_SUMMON
    case 276:
        OpenMonsterModel(MONSTER_MODEL_GOLDEN_TITAN);
        c = CreateCharacter(Key, MODEL_GOLDEN_TITAN, PositionX, PositionY);
        c->Object.Scale = 1.45f;
        c->Weapon[0].Type = MODEL_DARK_BREAKER;//MODEL_SWORD+15;
        c->Weapon[0].Level = 5;
        break;
#endif // ADD_ELF_SUMMON
    case MONSTER_GATE_TO_KALIMA_1:
    case MONSTER_GATE_TO_KALIMA_2:
    case MONSTER_GATE_TO_KALIMA_3:
    case MONSTER_GATE_TO_KALIMA_4:
    case MONSTER_GATE_TO_KALIMA_5:
    case MONSTER_GATE_TO_KALIMA_6:
    case MONSTER_GATE_TO_KALIMA_7:
        c = CreateCharacter(Key, MODEL_WARCRAFT, PositionX, PositionY);
        c->NotRotateOnMagicHit = true;
        c->Weapon[0].Type = -1;
        c->Weapon[0].Level = 0;
        c->Object.Scale = 1.f;
        c->HideShadow = false;
        o = &c->Object;
        o->PriorAnimationFrame = 10.f;
        o->AnimationFrame = 10;
        o->BlendMesh = -1;
        wcscpy(c->ID, L"");
        break;
    case MONSTER_CHAOS_CASTLE_1:
    case MONSTER_CHAOS_CASTLE_3:
    case MONSTER_CHAOS_CASTLE_5:
    case MONSTER_CHAOS_CASTLE_7:
    case MONSTER_CHAOS_CASTLE_9:
    case MONSTER_CHAOS_CASTLE_11:
    case MONSTER_CHAOS_CASTLE_13:
    {
        OpenMonsterModel(MONSTER_MODEL_CHAOSCASTLE_KNIGHT);
        c = CreateCharacter(Key, MODEL_CHAOS_CASTLE_KNIGHT, PositionX, PositionY);
        c->Object.Scale = 0.9f;
        o = &c->Object;

        c->Weapon[0].Type = MODEL_SWORD_OF_DESTRUCTION;
        c->Weapon[0].Level = 0;
        c->Weapon[1].Type = MODEL_SWORD_OF_DESTRUCTION;
        c->Weapon[1].Level = 0;
    }
    break;

    case MONSTER_CHAOS_CASTLE_2:
    case MONSTER_CHAOS_CASTLE_4:
    case MONSTER_CHAOS_CASTLE_6:
    case MONSTER_CHAOS_CASTLE_8:
    case MONSTER_CHAOS_CASTLE_10:
    case MONSTER_CHAOS_CASTLE_12:
    case MONSTER_CHAOS_CASTLE_14:
    {
        int randType = 0;

        randType = rand() % 2;

        OpenMonsterModel(randType == 0 ? MONSTER_MODEL_CHAOSCASTLE_ELF : MONSTER_MODEL_CHAOSCASTLE_WIZARD);
        c = CreateCharacter(Key, MODEL_CHAOS_CASTLE_ELF + randType, PositionX, PositionY);
        c->Object.Scale = 0.9f;
        o = &c->Object;

        c->Weapon[0].Type = -1;
        c->Weapon[0].Level = 0;
        c->Weapon[1].Type = -1;
        c->Weapon[1].Level = 0;

        if (randType == 0)
        {
            c->Weapon[0].Type = MODEL_GREAT_REIGN_CROSSBOW;
            c->Weapon[0].Level = 0;
        }
        else
        {
            c->Weapon[0].Type = MODEL_LEGENDARY_STAFF;
            c->Weapon[0].Level = 0;
        }
    }
    break;
    case MONSTER_MAGIC_SKELETON_1:
    case MONSTER_MAGIC_SKELETON_2:
    case MONSTER_MAGIC_SKELETON_3:
    case MONSTER_MAGIC_SKELETON_4:
    case MONSTER_MAGIC_SKELETON_5:
    case MONSTER_MAGIC_SKELETON_6:
    case MONSTER_MAGIC_SKELETON_7:
    case MONSTER_MAGIC_SKELETON_8:
        OpenMonsterModel(MONSTER_MODEL_MAGIC_SKELETON);
        c = CreateCharacter(Key, MODEL_MAGIC_SKELETON, PositionX, PositionY);
        c->Weapon[0].Type = MODEL_STAFF;
        c->Weapon[0].Level = 11;
        c->Object.Scale = 1.2f;
        wcscpy(c->ID, L"마법해골");
        break;
    case MONSTER_CASTLE_GATE: // ???
        OpenMonsterModel(MONSTER_MODEL_CASTLE_GATE);
        c = CreateCharacter(Key, MODEL_CASTLE_GATE, PositionX, PositionY);
        c->NotRotateOnMagicHit = true;
        c->Object.Scale = 0.8f;
        c->Object.EnableShadow = false;
        wcscpy(c->ID, L"성문");
        break;
    case MONSTER_STATUE_OF_SAINT_1:
        OpenMonsterModel(MONSTER_MODEL_STATUE_OF_SAINT);
        c = CreateCharacter(Key, MODEL_STATUE_OF_SAINT, PositionX, PositionY);
        c->NotRotateOnMagicHit = true;
        c->Object.Scale = 0.8f;
        c->Object.EnableShadow = false;
        wcscpy(c->ID, L"성자의석관");
        break;
    case MONSTER_STATUE_OF_SAINT_2:
        OpenMonsterModel(MONSTER_MODEL_STATUE_OF_SAINT);
        c = CreateCharacter(Key, MODEL_STATUE_OF_SAINT, PositionX, PositionY);
        c->NotRotateOnMagicHit = true;
        c->Object.Scale = 0.8f;
        c->Object.EnableShadow = false;
        wcscpy(c->ID, L"성자의석관");
        break;
    case MONSTER_STATUE_OF_SAINT_3:
        OpenMonsterModel(MONSTER_MODEL_STATUE_OF_SAINT);
        c = CreateCharacter(Key, MODEL_STATUE_OF_SAINT, PositionX, PositionY);
        c->NotRotateOnMagicHit = true;
        c->Object.Scale = 0.8f;
        c->Object.EnableShadow = false;
        wcscpy(c->ID, L"성자의석관");
        break;
    case MONSTER_CHIEF_SKELETON_WARRIOR_1:
    case MONSTER_CHIEF_SKELETON_WARRIOR_2:
    case MONSTER_CHIEF_SKELETON_WARRIOR_3:
    case MONSTER_CHIEF_SKELETON_WARRIOR_4:
    case MONSTER_CHIEF_SKELETON_WARRIOR_5:
    case MONSTER_CHIEF_SKELETON_WARRIOR_6:
    case MONSTER_CHIEF_SKELETON_WARRIOR_7:
    case MONSTER_CHIEF_SKELETON_WARRIOR_8:
        OpenMonsterModel(MONSTER_MODEL_ORC);
        c = CreateCharacter(Key, MODEL_ORC, PositionX, PositionY);
        c->Object.Scale = 1.1f;
        o = &c->Object;
        break;
    case MONSTER_CHIEF_SKELETON_ARCHER_1:
    case MONSTER_CHIEF_SKELETON_ARCHER_2:
    case MONSTER_CHIEF_SKELETON_ARCHER_3:
    case MONSTER_CHIEF_SKELETON_ARCHER_4:
    case MONSTER_CHIEF_SKELETON_ARCHER_5:
    case MONSTER_CHIEF_SKELETON_ARCHER_6:
    case MONSTER_CHIEF_SKELETON_ARCHER_7:
    case MONSTER_CHIEF_SKELETON_ARCHER_8:
        OpenMonsterModel(MONSTER_MODEL_ORC_ARCHER);
        c = CreateCharacter(Key, MODEL_ORC_ARCHER, PositionX, PositionY);
        c->Object.Scale = 1.1f;
        c->Weapon[1].Type = MODEL_BATTLE_BOW;
        c->Weapon[1].Level = 1;
        o = &c->Object;
        break;
    case MONSTER_DARK_SKULL_SOLDIER_1:
    case MONSTER_DARK_SKULL_SOLDIER_2:
    case MONSTER_DARK_SKULL_SOLDIER_3:
    case MONSTER_DARK_SKULL_SOLDIER_4:
    case MONSTER_DARK_SKULL_SOLDIER_5:
    case MONSTER_DARK_SKULL_SOLDIER_6:
    case MONSTER_DARK_SKULL_SOLDIER_7:
    case MONSTER_DARK_SKULL_SOLDIER_8:
        OpenMonsterModel(MONSTER_MODEL_DARK_SKULL_SOLDIER);
        c = CreateCharacter(Key, MODEL_DARK_SKULL_SOLDIER, PositionX, PositionY);
        c->Weapon[0].Type = MODEL_CRESCENT_AXE;
        c->Weapon[0].Level = 0;
        c->Weapon[1].Type = MODEL_CRESCENT_AXE;
        c->Weapon[1].Level = 0;
        c->Object.Scale = 1.0f;
        wcscpy(c->ID, L"흑해골전사");
        break;
    case MONSTER_GIANT_OGRE_1:
    case MONSTER_GIANT_OGRE_2:
    case MONSTER_GIANT_OGRE_3:
    case MONSTER_GIANT_OGRE_4:
    case MONSTER_GIANT_OGRE_5:
    case MONSTER_GIANT_OGRE_6:
    case MONSTER_GIANT_OGRE_7:
    case MONSTER_GIANT_OGRE_8:
        OpenMonsterModel(MONSTER_MODEL_GIANT_OGRE);
        c = CreateCharacter(Key, MODEL_GIANT_OGRE, PositionX, PositionY);
        c->Object.Scale = 0.8f;
        wcscpy(c->ID, L"자이언트오우거");
        break;
    case MONSTER_RED_SKELETON_KNIGHT_1:
    case MONSTER_RED_SKELETON_KNIGHT_2:
    case MONSTER_RED_SKELETON_KNIGHT_3:
    case MONSTER_RED_SKELETON_KNIGHT_4:
    case MONSTER_RED_SKELETON_KNIGHT_5:
    case MONSTER_RED_SKELETON_KNIGHT_6:
    case MONSTER_RED_SKELETON_KNIGHT_7:
    case MONSTER_RED_SKELETON_KNIGHT_8:
        OpenMonsterModel(MONSTER_MODEL_RED_SKELETON_KNIGHT);
        c = CreateCharacter(Key, MODEL_RED_SKELETON_KNIGHT, PositionX, PositionY);
        c->Weapon[0].Type = MODEL_CHAOS_DRAGON_AXE;

        if (!int((7 + (gMapManager.WorldActive - WD_11BLOODCASTLE_END)) / 3))
            c->Weapon[0].Level = 8;
        else
            c->Weapon[0].Level = 0;

        c->Object.Scale = 1.19f;
        wcscpy(c->ID, L"붉은해골기사");
        break;
    case MONSTER_GOLDEN_GOBLIN:
        OpenMonsterModel(MONSTER_MODEL_GOBLIN);
        c = CreateCharacter(Key, MODEL_GOBLIN, PositionX, PositionY);
        c->Weapon[0].Type = MODEL_AXE;
        c->Weapon[0].Level = 9;
        c->Object.Scale = 0.8f;
        wcscpy(c->ID, L"고블린");
        break;
    case MONSTER_GOLDEN_DERKON:
        OpenMonsterModel(MONSTER_MODEL_DRAGON);
        c = CreateCharacter(Key, MODEL_DRAGON_, PositionX, PositionY);
        wcscpy(c->ID, L"드래곤");
        c->Object.Scale = 0.9f;
        break;
    case MONSTER_GOLDEN_LIZARD_KING:
        OpenMonsterModel(MONSTER_MODEL_LIZARD);
        c = CreateCharacter(Key, MODEL_LIZARD, PositionX, PositionY);
        c->Object.Scale = 1.4f;
        c->Weapon[0].Type = MODEL_CHAOS_LIGHTNING_STAFF;
        c->Weapon[0].ExcellentFlags = 63;
        break;
    case MONSTER_GOLDEN_VEPAR:
        OpenMonsterModel(MONSTER_MODEL_VEPAR);
        c = CreateCharacter(Key, MODEL_VEPAR, PositionX, PositionY);
        c->Object.Scale = 1.f;
        break;
    case MONSTER_GOLDEN_TANTALLOS: //??
        OpenMonsterModel(MONSTER_MODEL_TANTALLOS);
        c = CreateCharacter(Key, MODEL_TANTALLOS, PositionX, PositionY);
        c->Object.BlendMesh = 2;
        c->Object.BlendMeshLight = 1.f;
        o = &c->Object;
        c->Object.Scale = 1.8f;
        c->Weapon[0].Type = MODEL_SWORD_OF_DESTRUCTION;
        c->Weapon[0].ExcellentFlags = 63;
        CreateJoint(BITMAP_JOINT_ENERGY, o->Position, o->Position, o->Angle, 2, o, 30.f);
        CreateJoint(BITMAP_JOINT_ENERGY, o->Position, o->Position, o->Angle, 3, o, 30.f);
        break;
    case MONSTER_GOLDEN_WHEEL:
        OpenMonsterModel(MONSTER_MODEL_GOLDEN_WHEEL);
        c = CreateCharacter(Key, MODEL_GOLDEN_WHEEL, PositionX, PositionY);
        c->Object.Scale = 1.4f;
        c->Weapon[0].Type = MODEL_AQUAGOLD_CROSSBOW;
        c->Weapon[0].ExcellentFlags = 63;
        //c->Weapon[0].Type = MODEL_BOW+16;
        o = &c->Object;
        CreateJoint(BITMAP_JOINT_ENERGY, o->Position, o->Position, o->Angle, 2, o, 30.f);
        CreateJoint(BITMAP_JOINT_ENERGY, o->Position, o->Position, o->Angle, 3, o, 30.f);
        break;
    case MONSTER_MOLT:
        OpenMonsterModel(MONSTER_MODEL_MOLT);
        c = CreateCharacter(Key, MODEL_MOLT, PositionX, PositionY);
        c->Object.Scale = 1.4f;
        break;

    case MONSTER_ALQUAMOS:
        OpenMonsterModel(MONSTER_MODEL_ALQUAMOS);
        c = CreateCharacter(Key, MODEL_ALQUAMOS, PositionX, PositionY);
        c->Object.Scale = 1.f;
        c->Object.BlendMesh = 0;
        break;
    case MONSTER_QUEEN_RAINER:
        OpenMonsterModel(MONSTER_MODEL_QUEEN_RAINER);
        c = CreateCharacter(Key, MODEL_QUEEN_RAINER, PositionX, PositionY);
        c->Object.Scale = 1.3f;
        c->Object.BlendMesh = -2;
        c->Object.BlendMeshLight = 1.f;
        c->Object.m_bRenderShadow = false;
        break;
    case MONSTER_OMEGA_WING:
    case MONSTER_MEGA_CRUST:
    case MONSTER_ALPHA_CRUST:
        OpenMonsterModel(MONSTER_MODEL_CRUST);
        c = CreateCharacter(Key, MODEL_CRUST, PositionX, PositionY);
        if (MONSTER_MEGA_CRUST == Type)
        {
            c->Object.Scale = 1.1f;
            c->Weapon[0].Type = MODEL_THUNDER_BLADE;
            c->Weapon[0].Level = 5;
            c->Weapon[1].Type = MODEL_LEGENDARY_SHIELD;
            c->Weapon[1].Level = 0;
        }
        else
        {
            c->Object.Scale = 1.3f;
            c->Weapon[0].Type = MODEL_THUNDER_BLADE;
            c->Weapon[0].Level = 9;
            c->Weapon[1].Type = MODEL_LEGENDARY_SHIELD;
            c->Weapon[1].Level = 9;
        }
        c->Object.BlendMesh = 1;
        c->Object.BlendMeshLight = 1.f;
        //Models[MODEL_MONSTER01+52].StreamMesh = 1;
        break;

    case MONSTER_PHANTOM_KNIGHT:
        OpenMonsterModel(MONSTER_MODEL_PHANTOM_KNIGHT);
        c = CreateCharacter(Key, MODEL_PHANTOM_KNIGHT, PositionX, PositionY);
        c->Object.Scale = 1.45f;
        c->Weapon[0].Type = MODEL_DARK_BREAKER;//MODEL_SWORD+15;
        c->Weapon[0].Level = 5;
        break;

    case MONSTER_DRAKAN:
    case MONSTER_GREAT_DRAKAN:
        OpenMonsterModel(MONSTER_MODEL_DRAKAN);
        c = CreateCharacter(Key, MODEL_DRAKAN, PositionX, PositionY);
        c->NotRotateOnMagicHit = true;
        if (Type == MONSTER_GREAT_DRAKAN)
        {
            c->Object.Scale = 1.0f;
        }
        else
        {
            c->Object.Scale = 0.8f;
        }
        Models[c->Object.Type].Meshs[0].NoneBlendMesh = true;
        Models[c->Object.Type].Meshs[1].NoneBlendMesh = false;
        Models[c->Object.Type].Meshs[2].NoneBlendMesh = false;
        Models[c->Object.Type].Meshs[3].NoneBlendMesh = true;
        Models[c->Object.Type].Meshs[4].NoneBlendMesh = true;
        break;
    case MONSTER_DARK_PHOENIX:
    {
        OpenMonsterModel(MONSTER_MODEL_DARK_PHOENIX_SHIELD);
        OpenMonsterModel(MONSTER_MODEL_DARK_PHOENIX);
        c = CreateCharacter(Key, MODEL_DARK_PHEONIX_SHIELD, PositionX, PositionY);
        c->NotRotateOnMagicHit = true;
        c->Object.Scale = 1.0f;
        Models[MODEL_DARK_PHEONIX_SHIELD].StreamMesh = 0;
    }
    break;
    case MONSTER_ORC_ARCHER:
        OpenMonsterModel(MONSTER_MODEL_ORC_ARCHER);
        c = CreateCharacter(Key, MODEL_ORC_ARCHER, PositionX, PositionY);
        c->Object.Scale = 1.2f;
        c->Weapon[1].Type = MODEL_BATTLE_BOW;
        c->Weapon[1].Level = 3;
        o = &c->Object;
        o->HiddenMesh = 1;
        break;
    case MONSTER_ORC_ARCHER_OF_DOOM:
        OpenMonsterModel(MONSTER_MODEL_ORC_ARCHER);
        c = CreateCharacter(Key, MODEL_ORC_ARCHER, PositionX, PositionY);
        c->Object.Scale = 1.2f;
        c->Weapon[1].Type = MODEL_BATTLE_BOW;
        c->Weapon[1].Level = 5;
        o = &c->Object;
        o->HiddenMesh = 1;
        break;
    case MONSTER_ELITE_ORC:
        OpenMonsterModel(MONSTER_MODEL_ORC);
        c = CreateCharacter(Key, MODEL_ORC, PositionX, PositionY);
        c->Object.Scale = 1.3f;
        o = &c->Object;
        o->HiddenMesh = 2;
        break;
    case MONSTER_ORC_SOLDIER_OF_DOOM:
        OpenMonsterModel(MONSTER_MODEL_ORC);
        c = CreateCharacter(Key, MODEL_ORC, PositionX, PositionY);
        c->Object.Scale = 1.3f;
        o = &c->Object;
        o->HiddenMesh = 2;
        break;
    case MONSTER_CURSED_KING:
    case MONSTER_WHITE_WIZARD:
        OpenMonsterModel(MONSTER_MODEL_CURSED_KING);
        c = CreateCharacter(Key, MODEL_CURSED_KING, PositionX, PositionY);
        c->Object.Scale = 1.7f;
        o = &c->Object;
        break;
    case MONSTER_EVIL_GOBLIN:
        OpenMonsterModel(MONSTER_MODEL_EVIL_GOBLIN);
        c = CreateCharacter(Key, MODEL_EVIL_GOBLIN, PositionX, PositionY);
        c->Object.Scale = 0.9f;
        wcscpy(c->ID, L"저주받은 고블린");
        o = &c->Object;
        break;
    case MONSTER_CURSED_SANTA:
        OpenMonsterModel(MONSTER_MODEL_CURSED_SANTA);
        c = CreateCharacter(Key, MODEL_CURSED_SANTA, PositionX, PositionY);
        c->Object.Scale = 1.7f;
        wcscpy(c->ID, L"저주받은 산타");
        o = &c->Object;
        break;
    case MONSTER_MUTANT_HERO:
    case MONSTER_MUTANT:
        OpenMonsterModel(MONSTER_MODEL_MUTANT);
        c = CreateCharacter(Key, MODEL_MUTANT, PositionX, PositionY);
        c->Object.Scale = 1.5f;
        o = &c->Object;
        CreateJoint(BITMAP_JOINT_ENERGY, o->Position, o->Position, o->Angle, 2, o, 30.f);
        CreateJoint(BITMAP_JOINT_ENERGY, o->Position, o->Position, o->Angle, 3, o, 30.f);
        break;
    case MONSTER_DEATH_BEAM_KNIGHT:
    case MONSTER_BEAM_KNIGHT:
        OpenMonsterModel(MONSTER_MODEL_BEAM_KNIGHT);
        c = CreateCharacter(Key, MODEL_BEAM_KNIGHT, PositionX, PositionY);
        if (Type == MONSTER_DEATH_BEAM_KNIGHT)
        {
            c->Object.Scale = 1.9f;
            c->Object.BlendMesh = -2;
            c->Object.BlendMeshLight = 1.f;
        }
        else
        {
            c->Object.Scale = 1.5f;
        }
        o = &c->Object;
        CreateJoint(BITMAP_JOINT_ENERGY, o->Position, o->Position, o->Angle, 2, o, 30.f);
        CreateJoint(BITMAP_JOINT_ENERGY, o->Position, o->Position, o->Angle, 3, o, 30.f);
        break;
    case MONSTER_BLOODY_WOLF:
        OpenMonsterModel(MONSTER_MODEL_BLOODY_WOLF);
        c = CreateCharacter(Key, MODEL_BLOODY_WOLF, PositionX, PositionY);
        c->Object.Scale = 2.2f;
        o = &c->Object;
        CreateJoint(BITMAP_JOINT_ENERGY, o->Position, o->Position, o->Angle, 2, o, 30.f);
        CreateJoint(BITMAP_JOINT_ENERGY, o->Position, o->Position, o->Angle, 3, o, 30.f);
        break;
    case MONSTER_TANTALLOS:
    case MONSTER_ZAIKAN:
        OpenMonsterModel(MONSTER_MODEL_TANTALLOS);
        c = CreateCharacter(Key, MODEL_TANTALLOS, PositionX, PositionY);
        c->Object.BlendMesh = 2;
        c->Object.BlendMeshLight = 1.f;
        o = &c->Object;
        if (Type == MONSTER_TANTALLOS)
        {
            c->Object.Scale = 1.8f;
            c->Weapon[0].Type = MODEL_SWORD_OF_DESTRUCTION;
        }
        else
        {
            c->Object.Scale = 2.1f;
            o->SubType = 1;
            c->Weapon[0].Type = MODEL_STAFF_OF_DESTRUCTION;
        }
        CreateJoint(BITMAP_JOINT_ENERGY, o->Position, o->Position, o->Angle, 2, o, 30.f);
        CreateJoint(BITMAP_JOINT_ENERGY, o->Position, o->Position, o->Angle, 3, o, 30.f);
        break;
    case MONSTER_IRON_WHEEL:
        OpenMonsterModel(MONSTER_MODEL_GOLDEN_WHEEL);
        c = CreateCharacter(Key, MODEL_GOLDEN_WHEEL, PositionX, PositionY);
        c->Object.Scale = 1.4f;
        c->Weapon[0].Type = MODEL_AQUAGOLD_CROSSBOW;
        //c->Weapon[0].Type = MODEL_BOW+16;
        o = &c->Object;
        CreateJoint(BITMAP_JOINT_ENERGY, o->Position, o->Position, o->Angle, 2, o, 30.f);
        CreateJoint(BITMAP_JOINT_ENERGY, o->Position, o->Position, o->Angle, 3, o, 30.f);
        break;
    case MONSTER_SILVER_VALKYRIE:
        OpenMonsterModel(MONSTER_MODEL_VALKYRIE);
        c = CreateCharacter(Key, MODEL_VALKYRIE, PositionX, PositionY);
        c->Object.Scale = 1.4f;
        c->Weapon[0].Type = MODEL_BLUEWING_CROSSBOW;
        break;
    case MONSTER_GREAT_BAHAMUT:
        OpenMonsterModel(MONSTER_MODEL_BAHAMUT);
        c = CreateCharacter(Key, MODEL_BAHAMUT, PositionX, PositionY);
        c->Object.Scale = 1.f;
        c->Level = 1;
        break;
    case MONSTER_SEA_WORM:
        OpenMonsterModel(MONSTER_MODEL_SEA_WORM);
        c = CreateCharacter(Key, MODEL_SEA_WORM, PositionX, PositionY);
        c->Object.Scale = 1.8f;
        break;
    case MONSTER_HYDRA:
        OpenMonsterModel(MONSTER_MODEL_HYDRA);
        c = CreateCharacter(Key, MODEL_HYDRA, PositionX, PositionY);
        c->Object.Scale = 1.f;
        c->Object.BlendMesh = 5;
        c->Object.BlendMeshLight = 0.f;
        break;
    case MONSTER_LIZARD_KING:
        OpenMonsterModel(MONSTER_MODEL_LIZARD);
        c = CreateCharacter(Key, MODEL_LIZARD, PositionX, PositionY);
        c->Object.Scale = 1.4f;
        c->Weapon[0].Type = MODEL_STAFF_OF_RESURRECTION;
        break;
    case MONSTER_VALKYRIE:
        OpenMonsterModel(MONSTER_MODEL_VALKYRIE);
        c = CreateCharacter(Key, MODEL_VALKYRIE, PositionX, PositionY);
        c->Object.Scale = 1.1f;
        c->Weapon[0].Type = MODEL_BLUEWING_CROSSBOW;
        c->Object.BlendMesh = 0;
        c->Object.BlendMeshLight = 1.f;
        break;
    case MONSTER_VEPAR:
        OpenMonsterModel(MONSTER_MODEL_VEPAR);
        c = CreateCharacter(Key, MODEL_VEPAR, PositionX, PositionY);
        c->Object.Scale = 1.f;
        break;
    case MONSTER_BAHAMUT:
        OpenMonsterModel(MONSTER_MODEL_BAHAMUT);
        c = CreateCharacter(Key, MODEL_BAHAMUT, PositionX, PositionY);
        c->Object.Scale = 0.6f;
        break;
    case MONSTER_BALI:
        OpenMonsterModel(MONSTER_MODEL_BALI);
        c = CreateCharacter(Key, MODEL_BALI, PositionX, PositionY);
        wcscpy(c->ID, L"발리");
        c->Object.Scale = 0.12f;
        break;
    case MONSTER_GOLDEN_DRAGON:
        OpenMonsterModel(MONSTER_MODEL_DRAGON);
        c = CreateCharacter(Key, MODEL_DRAGON_, PositionX, PositionY);
        wcscpy(c->ID, L"드래곤");
        c->Object.Scale = 0.9f;
        break;
    case MONSTER_GOLDEN_BUDGE_DRAGON:
        OpenMonsterModel(MONSTER_MODEL_BUDGE_DRAGON);
        c = CreateCharacter(Key, MODEL_BUDGE_DRAGON, PositionX, PositionY);
        wcscpy(c->ID, L"황금버지드래곤");
        c->Object.Scale = 0.7f;
        break;
    case MONSTER_RED_DRAGON:
        OpenMonsterModel(MONSTER_MODEL_DRAGON);
        c = CreateCharacter(Key, MODEL_DRAGON_, PositionX, PositionY);
        wcscpy(c->ID, L"쿤둔");
        c->Object.Scale = 1.3f;
        Vector(200.f, 150.f, 280.f, c->Object.BoundingBoxMax);
        break;
    case MONSTER_DEATH_COW:
        OpenMonsterModel(MONSTER_MODEL_DEATH_COW);
        c = CreateCharacter(Key, MODEL_DEATH_COW, PositionX, PositionY);
        wcscpy(c->ID, L"데쓰 카우");
        c->Weapon[0].Type = MODEL_GREAT_HAMMER;
        //c->Weapon[0].Type = MODEL_SWORD+14;
        c->Object.Scale = 1.1f;
        //c->Level = 1;
        break;
    case MONSTER_DEATH_KNIGHT:
        OpenMonsterModel(MONSTER_MODEL_DEATH_KNIGHT);
        c = CreateCharacter(Key, MODEL_DEATH_KNIGHT, PositionX, PositionY);
        wcscpy(c->ID, L"데쓰 나이트");
        c->Weapon[0].Type = MODEL_GIANT_SWORD;
        c->Weapon[0].Type = MODEL_LIGHTING_SWORD;
        //c->Weapon[1].Type = MODEL_SHIELD+8;
        c->Object.Scale = 1.3f;
        //c->Level = 1;
        break;
    case MONSTER_POISON_SHADOW:
        OpenMonsterModel(MONSTER_MODEL_SHADOW);
        c = CreateCharacter(Key, MODEL_SHADOW, PositionX, PositionY);
        wcscpy(c->ID, L"포이즌 쉐도우");
        c->Object.Scale = 1.2f;
        c->Level = 1;
        break;
    case MONSTER_BALROG:
    case MONSTER_METAL_BALROG:	//발록2
        OpenMonsterModel(MONSTER_MODEL_BALROG);
        c = CreateCharacter(Key, MODEL_BALROG, PositionX, PositionY);
        wcscpy(c->ID, L"발록");
        c->Weapon[0].Type = MODEL_BILL_OF_BALROG;
        c->Weapon[0].Level = 9;
        c->Object.Scale = 1.6f;
        break;
    case MONSTER_DEVIL:
        OpenMonsterModel(MONSTER_MODEL_DEVIL);
        c = CreateCharacter(Key, MODEL_DEVIL, PositionX, PositionY);
        wcscpy(c->ID, L"데빌");
        c->Object.Scale = 1.1f;
        break;
    case MONSTER_SHADOW:
        OpenMonsterModel(MONSTER_MODEL_SHADOW);
        c = CreateCharacter(Key, MODEL_SHADOW, PositionX, PositionY);
        wcscpy(c->ID, L"쉐도우");
        c->Object.Scale = 1.2f;
        break;
        /*OpenMonsterModel(MONSTER_MODEL_GIANT);
        c = CreateCharacter(Key,MODEL_MONSTER01+7,PositionX,PositionY);
        wcscpy(c->ID,"블러드 고스트");
        c->Object.AlphaTarget = 0.4f;
        c->MoveSpeed = 15;
        c->Blood = true;
        c->Object.Scale = 1.1f;
        c->Level = 2;*/
    case MONSTER_DEATH_GORGON:
        OpenMonsterModel(MONSTER_MODEL_GORGON);
        c = CreateCharacter(Key, MODEL_GORGON, PositionX, PositionY);
        wcscpy(c->ID, L"데쓰 고르곤");
        c->Object.Scale = 1.3f;
        c->Weapon[0].Type = MODEL_CRESCENT_AXE;
        c->Weapon[1].Type = MODEL_CRESCENT_AXE;
        c->Object.BlendMesh = 1;
        c->Object.BlendMeshLight = 1.f;
        c->Level = 2;
        break;
    case MONSTER_CURSED_WIZARD:
        c = CreateCharacter(Key, MODEL_PLAYER, PositionX, PositionY);
        wcscpy(c->ID, L"저주받은 법사");
        c->BodyPart[BODYPART_HELM].Type = MODEL_LEGENDARY_HELM;
        c->BodyPart[BODYPART_ARMOR].Type = MODEL_LEGENDARY_ARMOR;
        c->BodyPart[BODYPART_PANTS].Type = MODEL_LEGENDARY_PANTS;
        c->BodyPart[BODYPART_GLOVES].Type = MODEL_LEGENDARY_GLOVES;
        c->BodyPart[BODYPART_BOOTS].Type = MODEL_LEGENDARY_BOOTS;
        c->Weapon[0].Type = MODEL_LEGENDARY_STAFF;
        c->Weapon[1].Type = MODEL_LEGENDARY_SHIELD;
        Level = 9;
        c->BodyPart[BODYPART_HELM].Level = Level;
        c->BodyPart[BODYPART_ARMOR].Level = Level;
        c->BodyPart[BODYPART_PANTS].Level = Level;
        c->BodyPart[BODYPART_GLOVES].Level = Level;
        c->BodyPart[BODYPART_BOOTS].Level = Level;
        //c->Weapon[0].Level = Level;
        //c->Weapon[1].Level = Level;
        c->PK = PVP_MURDERER2;
        SetCharacterScale(c);
        if (gMapManager.InDevilSquare() == true)
        {
            c->Object.Scale = 1.0f;
        }
        break;
        /*OpenMonsterModel(MONSTER_MODEL_HELL_HOUND);
        c = CreateCharacter(Key,MODEL_MONSTER01+5,PositionX,PositionY);
        wcscpy(c->ID,"자이언트");
        c->Weapon[0].Type = MODEL_AXE+2;
        c->Weapon[1].Type = MODEL_AXE+2;
        c->Object.Scale = 0.7f;
        Vector(50.f,50.f,80.f,c->Object.BoundingBoxMax);
        break;*/
    case MONSTER_ELITE_GOBLIN:
        OpenMonsterModel(MONSTER_MODEL_GOBLIN);
        c = CreateCharacter(Key, MODEL_GOBLIN, PositionX, PositionY);
        c->Weapon[0].Type = MODEL_MORNING_STAR;
        c->Weapon[1].Type = MODEL_HORN_SHIELD;
        c->Object.Scale = 1.2f;
        c->Level = 1;
        wcscpy(c->ID, L"고블린 대장");
        break;
    case MONSTER_STONE_GOLEM:
        OpenMonsterModel(MONSTER_MODEL_STONE_GOLEM);
        c = CreateCharacter(Key, MODEL_STONE_GOLEM, PositionX, PositionY);
        wcscpy(c->ID, L"돌괴물");
        break;
    case MONSTER_AGON:
        OpenMonsterModel(MONSTER_MODEL_AGON);
        c = CreateCharacter(Key, MODEL_AGON, PositionX, PositionY);
        wcscpy(c->ID, L"아곤");
        c->Object.Scale = 1.3f;
        c->Weapon[0].Type = MODEL_SERPENT_SWORD;
        c->Weapon[1].Type = MODEL_SERPENT_SWORD;
        break;
    case MONSTER_FOREST_MONSTER:
        OpenMonsterModel(MONSTER_MODEL_FOREST_MONSTER);
        c = CreateCharacter(Key, MODEL_FOREST_MONSTER, PositionX, PositionY);
        wcscpy(c->ID, L"숲의괴물");
        c->Object.Scale = 0.75f;
        break;
    case MONSTER_HUNTER:
        OpenMonsterModel(MONSTER_MODEL_HUNTER);
        c = CreateCharacter(Key, MODEL_HUNTER, PositionX, PositionY);
        wcscpy(c->ID, L"헌터");
        c->Weapon[0].Type = MODEL_ARQUEBUS;
        c->Object.Scale = 0.95f;
        break;
    case MONSTER_BEETLE_MONSTER:
        OpenMonsterModel(MONSTER_MODEL_BEETLE_MONSTER);
        c = CreateCharacter(Key, MODEL_BEETLE_MONSTER, PositionX, PositionY);
        c->Weapon[0].Type = MODEL__SPEAR;
        c->Object.Scale = 0.8f;
        wcscpy(c->ID, L"풍뎅이괴물");
        c->Object.BlendMesh = 1;
        break;
    case MONSTER_CHAIN_SCORPION:
        OpenMonsterModel(MONSTER_MODEL_CHAIN_SCORPION);
        c = CreateCharacter(Key, MODEL_CHAIN_SCORPION, PositionX, PositionY);
        c->Object.Scale = 1.1f;
        wcscpy(c->ID, L"고리전갈");
        break;
    case MONSTER_GOBLIN:
        OpenMonsterModel(MONSTER_MODEL_GOBLIN);
        c = CreateCharacter(Key, MODEL_GOBLIN, PositionX, PositionY);
        c->Weapon[0].Type = MODEL_AXE;
        c->Object.Scale = 0.8f;
        wcscpy(c->ID, L"고블린");
        break;
    case MONSTER_ICE_QUEEN:
        OpenMonsterModel(MONSTER_MODEL_ICE_QUEEN);
        c = CreateCharacter(Key, MODEL_ICE_QUEEN, PositionX, PositionY);
        c->Weapon[0].Type = MODEL_ANGELIC_STAFF;
        c->Object.BlendMesh = 2;
        c->Object.BlendMeshLight = 1.f;
        c->Object.Scale = 1.1f;
        c->Object.LightEnable = false;
        c->Level = 3;
        wcscpy(c->ID, L"아이스퀸");
        break;
    case MONSTER_WORM:
        OpenMonsterModel(MONSTER_MODEL_WORM);
        c = CreateCharacter(Key, MODEL_WORM, PositionX, PositionY);
        wcscpy(c->ID, L"웜");
        break;
    case MONSTER_HOMMERD:
        OpenMonsterModel(MONSTER_MODEL_HOMMERD);
        c = CreateCharacter(Key, MODEL_HOMMERD, PositionX, PositionY);
        c->Weapon[0].Type = MODEL_LARKAN_AXE;
        c->Weapon[1].Type = MODEL_BIG_ROUND_SHIELD;
        c->Object.Scale = 1.15f;
        wcscpy(c->ID, L"호머드");
        break;
    case MONSTER_ICE_MONSTER:
        OpenMonsterModel(MONSTER_MODEL_ICE_MONSTER);
        c = CreateCharacter(Key, MODEL_ICE_MONSTER, PositionX, PositionY);
        c->Object.BlendMesh = 0;
        c->Object.BlendMeshLight = 1.f;
        wcscpy(c->ID, L"얼음괴물");
        break;
    case MONSTER_ASSASSIN:
        OpenMonsterModel(MONSTER_MODEL_ASSASSIN);
        c = CreateCharacter(Key, MODEL_ASSASSIN, PositionX, PositionY);
        c->Object.Scale = 0.95f;
        wcscpy(c->ID, L"암살자");
        break;
    case MONSTER_ELITE_YETI:
        OpenMonsterModel(MONSTER_MODEL_ELITE_YETI);
        c = CreateCharacter(Key, MODEL_ELITE_YETI, PositionX, PositionY);
        wcscpy(c->ID, L"설인 대장");
        c->Object.Scale = 1.4f;
        break;
    case MONSTER_YETI:
        OpenMonsterModel(MONSTER_MODEL_YETI);
        c = CreateCharacter(Key, MODEL_YETI, PositionX, PositionY);
        wcscpy(c->ID, L"설인");
        c->Object.Scale = 1.1f;
        break;
    case MONSTER_GORGON:
        OpenMonsterModel(MONSTER_MODEL_GORGON);
        c = CreateCharacter(Key, MODEL_GORGON, PositionX, PositionY);
        wcscpy(c->ID, L"고르곤");
        c->Object.Scale = 1.5f;
        c->Weapon[0].Type = MODEL_GORGON_STAFF;
        c->Object.BlendMesh = 1;
        c->Object.BlendMeshLight = 1.f;
        break;
    case MONSTER_SPIDER:
        OpenMonsterModel(MONSTER_MODEL_SPIDER);
        c = CreateCharacter(Key, MODEL_SPIDER, PositionX, PositionY);
        wcscpy(c->ID, L"거미");
        c->Object.Scale = 0.4f;
        break;
    case MONSTER_CYCLOPS:
        OpenMonsterModel(MONSTER_MODEL_CYCLOPS);
        c = CreateCharacter(Key, MODEL_CYCLOPS, PositionX, PositionY);
        wcscpy(c->ID, L"싸이크롭스");
        c->Weapon[0].Type = MODEL_CRESCENT_AXE;
        //c->Weapon[1].Type = MODEL_MACE+2;
        //c->Object.HiddenMesh = 2;
        break;
    case MONSTER_BULL_FIGHTER:
    case MONSTER_ELITE_BULL_FIGHTER:
    case MONSTER_POISON_BULL:
    default:
        OpenMonsterModel(MONSTER_MODEL_BULL_FIGHTER);
        c = CreateCharacter(Key, MODEL_BULL_FIGHTER, PositionX, PositionY);
        if (Type == MONSTER_BULL_FIGHTER)
        {
            c->Object.HiddenMesh = 0;
            wcscpy(c->ID, L"소뿔전사");
            c->Object.Scale = 0.8f;
            c->Weapon[0].Type = MODEL_NIKKEA_AXE;
        }
        else if (Type == MONSTER_ELITE_BULL_FIGHTER)
        {
            c->Weapon[0].Type = MODEL_BERDYSH;
            wcscpy(c->ID, L"소뿔전사 대장");
            c->Object.Scale = 1.15f;
            c->Level = 1;
        }
        else if (Type == MONSTER_POISON_BULL)
        {
            c->Weapon[0].Type = MODEL_GREAT_SCYTHE;
            wcscpy(c->ID, L"포이즌 소뿔전사");
            c->Object.Scale = 1.f;
            c->Level = 2;

            g_CharacterRegisterBuff((&c->Object), eDeBuff_Poison);
        }
        break;
    case MONSTER_GHOST:
        OpenMonsterModel(MONSTER_MODEL_GHOST);
        c = CreateCharacter(Key, MODEL_GHOST_MONSTER, PositionX, PositionY);
        wcscpy(c->ID, L"고스트");
        c->Object.AlphaTarget = 0.4f;
        c->MoveSpeed = 15;
        c->Blood = true;
        break;
    case MONSTER_LARVA:
        OpenMonsterModel(MONSTER_MODEL_LARVA);
        c = CreateCharacter(Key, MODEL_LARVA, PositionX, PositionY);
        wcscpy(c->ID, L"유충");
        c->Object.Scale = 0.6f;
        break;
    case MONSTER_HELL_SPIDER:
        OpenMonsterModel(MONSTER_MODEL_HELL_SPIDER);
        c = CreateCharacter(Key, MODEL_HELL_SPIDER, PositionX, PositionY);
        wcscpy(c->ID, L"헬스파이더");
        c->Weapon[0].Type = MODEL_SERPENT_STAFF;
        c->Object.Scale = 1.1f;
        break;
    case MONSTER_HOUND:
    case MONSTER_HELL_HOUND:
        OpenMonsterModel(MONSTER_MODEL_HOUND);
        c = CreateCharacter(Key, MODEL_HOUND, PositionX, PositionY);
        if (Type == MONSTER_HOUND)
        {
            c->Object.HiddenMesh = 0;
            wcscpy(c->ID, L"하운드");
            c->Object.Scale = 0.85f;
            c->Weapon[0].Type = MODEL_SWORD_OF_ASSASSIN;
        }
        if (Type == MONSTER_HELL_HOUND)
        {
            c->Object.HiddenMesh = 1;
            c->Weapon[0].Type = MODEL_FALCHION;
            c->Weapon[1].Type = MODEL_PLATE_SHIELD;
            wcscpy(c->ID, L"헬하운드");
            c->Object.Scale = 1.1f;
            c->Level = 1;
        }
        break;

    case MONSTER_BUDGE_DRAGON:
        OpenMonsterModel(MONSTER_MODEL_BUDGE_DRAGON);
        c = CreateCharacter(Key, MODEL_BUDGE_DRAGON, PositionX, PositionY);
        wcscpy(c->ID, L"Unknown2");
        c->Object.Scale = 0.5f;
        break;

    case MONSTER_DARK_KNIGHT:
        OpenMonsterModel(MONSTER_MODEL_DARK_KNIGHT);
        c = CreateCharacter(Key, MODEL_DARK_KNIGHT, PositionX, PositionY);
        wcscpy(c->ID, L"Unknown10");
        c->Object.Scale = 0.8f;
        c->Level = 1;
        c->Weapon[0].Type = MODEL_DOUBLE_BLADE;
        break;
    case MONSTER_LICH:
    case MONSTER_THUNDER_LICH:
        OpenMonsterModel(MONSTER_MODEL_LICH);
        c = CreateCharacter(Key, MODEL_LICH, PositionX, PositionY);
        if (Type == MONSTER_LICH)
        {
            wcscpy(c->ID, L"리치");
            c->Weapon[0].Type = MODEL_SERPENT_STAFF;
            c->Object.Scale = 0.85f;
        }
        else
        {
            wcscpy(c->ID, L"썬더 리치");
            c->Weapon[0].Type = MODEL_THUNDER_STAFF;
            c->Level = 1;
            c->Object.Scale = 1.1f;
        }
        break;
    case MONSTER_GIANT:
        OpenMonsterModel(MONSTER_MODEL_GIANT);
        c = CreateCharacter(Key, MODEL_GIANT, PositionX, PositionY);
        wcscpy(c->ID, L"자이언트");
        c->Weapon[0].Type = MODEL_DOUBLE_AXE;
        c->Weapon[1].Type = MODEL_DOUBLE_AXE;
        c->Object.Scale = 1.6f;
        break;

    case MONSTER_SKELETON_WARRIOR:
    case MONSTER_DEATH_KING:
    case MONSTER_DEATH_BONE:
        c = CreateCharacter(Key, MODEL_PLAYER, PositionX, PositionY);
        wcscpy(c->ID, L"해골전사");
        c->Object.SubType = MODEL_SKELETON1;
        c->Blood = true;
        if (Type == 14)
        {
            c->Object.Scale = 0.95f;
            c->Weapon[0].Type = MODEL_GLADIUS;
            c->Weapon[1].Type = MODEL_BUCKLER;
        }
        else if (Type == 56)
        {
            c->Object.Scale = 0.8f;
            c->Weapon[0].Type = MODEL_GREAT_SCYTHE;
        }
        else
        {
            c->Level = 1;
            c->Object.Scale = 1.4f;
            c->Weapon[0].Type = MODEL_BILL_OF_BALROG;
        }
        break;
    case MONSTER_SKELETON_ARCHER:
        c = CreateCharacter(Key, MODEL_PLAYER, PositionX, PositionY);
        wcscpy(c->ID, L"해골궁수");
        c->Object.Scale = 1.1f;
        c->Weapon[1].Type = MODEL_ELVEN_BOW;
        c->Object.SubType = MODEL_SKELETON2;
        c->Level = 1;
        c->Blood = true;
        break;
    case MONSTER_ELITE_SKELETON:
        c = CreateCharacter(Key, MODEL_PLAYER, PositionX, PositionY);
        wcscpy(c->ID, L"해골전사 대장");
        c->Object.Scale = 1.2f;
        c->Weapon[0].Type = MODEL_TOMAHAWK;
        c->Weapon[1].Type = MODEL_SKULL_SHIELD;
        c->Object.SubType = MODEL_SKELETON3;
        c->Level = 1;
        c->Blood = true;
        break;
    case MONSTER_ELITE_SKILL_SOLDIER:
        c = CreateCharacter(Key, MODEL_PLAYER, PositionX, PositionY);
        ::wcscpy(c->ID, L"엘리트 해골전사");
        c->Object.Scale = 0.95f;
        c->Object.SubType = MODEL_SKELETON_PCBANG;
        break;
    case MONSTER_JACK_OLANTERN:
        c = CreateCharacter(Key, MODEL_PLAYER, PositionX, PositionY);
        ::wcscpy(c->ID, L"잭 오랜턴");
        c->Object.Scale = 0.95f;
        c->Object.SubType = MODEL_HALLOWEEN;
        break;
    case MONSTER_SANTA:
        c = CreateCharacter(Key, MODEL_PLAYER, PositionX, PositionY);
        ::wcscpy(c->ID, L"크리스마스 걸");
        c->Object.Scale = 0.85f;
        c->Object.SubType = MODEL_XMAS_EVENT_CHANGE_GIRL;
        break;
    case MONSTER_GAMEMASTER:
        c = CreateCharacter(Key, MODEL_PLAYER, PositionX, PositionY);
        ::wcscpy(c->ID, L"GameMaster");
        c->Object.Scale = 1.0f;
        c->Object.SubType = MODEL_GM_CHARACTER;
        break;
    case MONSTER_GOLDEN_TITAN:
        OpenMonsterModel(MONSTER_MODEL_TITAN);
        c = CreateCharacter(Key, MODEL_TITAN, PositionX, PositionY);
        wcscpy(c->ID, L"타이탄");
        c->Object.Scale = 1.8f;
        c->Object.BlendMesh = 2;
        c->Object.BlendMeshLight = 1.f;
        o = &c->Object;
        CreateJoint(BITMAP_JOINT_ENERGY, o->Position, o->Position, o->Angle, 2, o, 30.f);
        CreateJoint(BITMAP_JOINT_ENERGY, o->Position, o->Position, o->Angle, 3, o, 30.f);
        break;
    case MONSTER_GOLDEN_SOLDIER:
    case MONSTER_SOLDIER:
        OpenMonsterModel(MONSTER_MODEL_SOLDIER);
        c = CreateCharacter(Key, MODEL_SOLDIER, PositionX, PositionY);
        wcscpy(c->ID, L"솔져");
        c->Weapon[1].Type = MODEL_AQUAGOLD_CROSSBOW;
        if (Type == 54)
            c->Object.Scale = 1.1f;
        else
            c->Object.Scale = 1.3f;
        break;
    case MONSTER_LANCE_TRAP:
        c = CreateCharacter(Key, 39, PositionX, PositionY);
        break;
    case MONSTER_IRON_STICK_TRAP:
        c = CreateCharacter(Key, 40, PositionX, PositionY);
        break;
    case MONSTER_FIRE_TRAP:
        c = CreateCharacter(Key, 51, PositionX, PositionY);
        break;
    case MONSTER_METEORITE_TRAP:
        c = CreateCharacter(Key, 25, PositionX, PositionY);
        break;
    case MONSTER_LASER_TRAP:
        c = CreateCharacter(Key, 51, PositionX, PositionY);
        break;
    case MONSTER_SOCCERBALL:
        c = CreateCharacter(Key, MODEL_BALL, PositionX, PositionY);
        o = &c->Object;
        o->BlendMesh = 2;
        o->Scale = 1.8f;
        c->Level = 1;
        break;
    case MONSTER_PET_TRAINER:
        OpenNpc(MODEL_NPC_BREEDER);
        c = CreateCharacter(Key, MODEL_NPC_BREEDER, PositionX, PositionY);
        wcscpy(c->ID, L"조련사 NPC");
        break;

#ifdef _PVP_MURDERER_HERO_ITEM
    case 227:
        OpenNpc(MODEL_MASTER);
        c = CreateCharacter(Key, MODEL_MASTER, PositionX, PositionY);
        wcscpy(c->ID, L"살인마상점");
        break;

    case 228:
        OpenNpc(MODEL_HERO_SHOP);
        c = CreateCharacter(Key, MODEL_HERO_SHOP, PositionX, PositionY);
        wcscpy(c->ID, L"영웅상점");
        break;
#endif	// _PVP_MURDERER_HERO_ITEM

    case MONSTER_MARLON:
        c = CreateCharacter(Key, MODEL_PLAYER, PositionX, PositionY);
        wcscpy(c->ID, L"말론");
        c->BodyPart[BODYPART_HELM].Type = MODEL_PLATE_HELM;
        c->BodyPart[BODYPART_HELM].Level = 7;
        c->BodyPart[BODYPART_ARMOR].Type = MODEL_PLATE_ARMOR;
        c->BodyPart[BODYPART_ARMOR].Level = 7;
        c->BodyPart[BODYPART_PANTS].Type = MODEL_PLATE_PANTS;
        c->BodyPart[BODYPART_PANTS].Level = 7;
        c->BodyPart[BODYPART_GLOVES].Type = MODEL_PLATE_GLOVES;
        c->BodyPart[BODYPART_GLOVES].Level = 7;
        c->BodyPart[BODYPART_BOOTS].Type = MODEL_PLATE_BOOTS;
        c->BodyPart[BODYPART_BOOTS].Level = 7;
        c->Weapon[0].Type = MODEL_BERDYSH;
        c->Weapon[0].Level = 8;
        c->Weapon[1].Type = -1;
        SetCharacterScale(c);
        c->Object.m_bpcroom = false;
        break;
    case MONSTER_ALEX:
        OpenNpc(MODEL_MERCHANT_MAN);
        c = CreateCharacter(Key, MODEL_MERCHANT_MAN, PositionX, PositionY);
        wcscpy(c->ID, L"로랜추가상인");
        c->BodyPart[BODYPART_HELM].Type = MODEL_MERCHANT_MAN_HEAD;
        c->BodyPart[BODYPART_ARMOR].Type = MODEL_MERCHANT_MAN_UPPER + 1;
        c->BodyPart[BODYPART_GLOVES].Type = MODEL_MERCHANT_MAN_GLOVES + 1;
        c->BodyPart[BODYPART_BOOTS].Type = MODEL_MERCHANT_MAN_BOOTS;
        break;
    case MONSTER_THOMPSON_THE_MERCHANT:
        OpenNpc(MODEL_DEVIAS_TRADER);
        c = CreateCharacter(Key, MODEL_DEVIAS_TRADER, PositionX, PositionY);
        wcscpy(c->ID, L"데비추가상인");
        break;

    case MONSTER_ARCHANGEL:
        OpenNpc(MODEL_NPC_ARCHANGEL);
        c = CreateCharacter(Key, MODEL_NPC_ARCHANGEL, PositionX, PositionY);
        o = &c->Object;
        o->Scale = 1.f;
        o->Kind = KIND_NPC;
        break;
    case MONSTER_MESSENGER_OF_ARCH:
        OpenNpc(MODEL_NPC_ARCHANGEL_MESSENGER);
        c = CreateCharacter(Key, MODEL_NPC_ARCHANGEL_MESSENGER, PositionX, PositionY);
        o = &c->Object;
        o->Scale = 1.f;
        o->Kind = KIND_NPC;
        break;

    case MONSTER_GOBLIN_GATE:
        OpenMonsterModel(MONSTER_MODEL_GOBLIN);
        c = CreateCharacter(Key, MODEL_GOBLIN, PositionX, PositionY);
        c->Weapon[0].Type = MODEL_STAFF;
        c->Weapon[0].Level = 4;
        c->Object.Scale = 1.5f;
        c->Object.Kind = KIND_NPC;
        SetAction(&c->Object, 0);
        break;

    case MONSTER_SEVINA_THE_PRIESTESS:
        OpenNpc(MODEL_NPC_SEVINA);
        c = CreateCharacter(Key, MODEL_NPC_SEVINA, PositionX, PositionY);
        o = &c->Object;
        o->Scale = 1.f;
        o->Kind = KIND_NPC;
        break;

    case MONSTER_GOLDEN_ARCHER:
        OpenNpc(MODEL_PLAYER);
        c = CreateCharacter(Key, MODEL_PLAYER, PositionX, PositionY);
        o = &c->Object;
        o->SubType = MODEL_SKELETON2;
        o->Scale = 1.0f;
        o->Kind = KIND_NPC;
        c->Level = 8;
        break;
    case MONSTER_CHARON:
        OpenNpc(MODEL_NPC_DEVILSQUARE);
        c = CreateCharacter(Key, MODEL_NPC_DEVILSQUARE, PositionX, PositionY);
        break;
    case MONSTER_OSBOURNE:
        OpenNpc(MODEL_REFINERY_NPC);
        c = CreateCharacter(Key, MODEL_REFINERY_NPC, PositionX, PositionY);
        o = &c->Object;
        break;
    case MONSTER_JERRIDON://환원
        OpenNpc(MODEL_RECOVERY_NPC);
        c = CreateCharacter(Key, MODEL_RECOVERY_NPC, PositionX, PositionY);
        o = &c->Object;
        break;
    case MONSTER_CHAOS_GOBLIN:
        OpenNpc(MODEL_MIX_NPC);
        c = CreateCharacter(Key, MODEL_MIX_NPC, PositionX, PositionY);
        o = &c->Object;
        o->BlendMesh = 1;
        break;
    case MONSTER_ARENA_GUARD:
        OpenNpc(MODEL_TOURNAMENT);
        c = CreateCharacter(Key, MODEL_TOURNAMENT, PositionX, PositionY);
        break;
    case MONSTER_BAZ_THE_VAULT_KEEPER:
        OpenNpc(MODEL_STORAGE);
        c = CreateCharacter(Key, MODEL_STORAGE, PositionX, PositionY);
        break;
    case MONSTER_GUILD_MASTER:
        OpenNpc(MODEL_MASTER);
        c = CreateCharacter(Key, MODEL_MASTER, PositionX, PositionY);
        wcscpy(c->ID, L"마스터");
        break;
    case MONSTER_LAHAP:
        OpenNpc(MODEL_NPC_SERBIS);
        c = CreateCharacter(Key, MODEL_NPC_SERBIS, PositionX, PositionY);
        wcscpy(c->ID, L"세르비스");
        break;
    case MONSTER_ELF_SOLDIER:
        c = CreateCharacter(Key, MODEL_PLAYER, PositionX, PositionY);
        MakeElfHelper(c);
        wcscpy(c->ID, L"페이아");
        o = &c->Object;
        CreateJoint(BITMAP_FLARE, o->Position, o->Position, o->Angle, 42, o, 15.f);
        break;
    case MONSTER_ELF_LALA:
        OpenNpc(MODEL_ELF_WIZARD);
        c = CreateCharacter(Key, MODEL_ELF_WIZARD, PositionX, PositionY);
        wcscpy(c->ID, L"라라 요정");
        o = &c->Object;
        o->BlendMesh = 1;
        o->Position[2] = RequestTerrainHeight(o->Position[0], o->Position[1]) + 140.f;
        break;
    case MONSTER_EO_THE_CRAFTSMAN:
        OpenNpc(MODEL_ELF_MERCHANT);
        c = CreateCharacter(Key, MODEL_ELF_MERCHANT, PositionX, PositionY);
        wcscpy(c->ID, L"장인");
        break;
    case MONSTER_CAREN_THE_BARMAID:
        OpenNpc(MODEL_SNOW_MERCHANT);
        c = CreateCharacter(Key, MODEL_SNOW_MERCHANT, PositionX, PositionY);
        wcscpy(c->ID, L"술집마담");
        break;
    case MONSTER_IZABEL_THE_WIZARD:
        OpenNpc(MODEL_SNOW_WIZARD);
        c = CreateCharacter(Key, MODEL_SNOW_WIZARD, PositionX, PositionY);
        wcscpy(c->ID, L"마법사");
        break;
    case MONSTER_ZIENNA_THE_WEAPONS_MERCHANT:
        OpenNpc(MODEL_SNOW_SMITH);
        c = CreateCharacter(Key, MODEL_SNOW_SMITH, PositionX, PositionY);
        wcscpy(c->ID, L"무기상인");
        break;
    case MONSTER_CROSSBOW_GUARD:
        c = CreateCharacter(Key, MODEL_PLAYER, PositionX, PositionY);
        wcscpy(c->ID, L"경비병");
        c->BodyPart[BODYPART_HELM].Type = MODEL_PLATE_HELM;
        c->BodyPart[BODYPART_ARMOR].Type = MODEL_PLATE_ARMOR;
        c->BodyPart[BODYPART_PANTS].Type = MODEL_PLATE_PANTS;
        c->BodyPart[BODYPART_GLOVES].Type = MODEL_PLATE_GLOVES;
        c->BodyPart[BODYPART_BOOTS].Type = MODEL_PLATE_BOOTS;
        c->Weapon[0].Type = MODEL_LIGHT_CROSSBOW;
        c->Weapon[1].Type = MODEL_BOLT;
        SetCharacterScale(c);
        break;
    case MONSTER_WANDERING_MERCHANT_MARTIN:
        OpenNpc(MODEL_MERCHANT_MAN);
        c = CreateCharacter(Key, MODEL_MERCHANT_MAN, PositionX, PositionY);
        wcscpy(c->ID, L"떠돌이 상인");
        c->BodyPart[BODYPART_HELM].Type = MODEL_MERCHANT_MAN_HEAD + 1;
        c->BodyPart[BODYPART_ARMOR].Type = MODEL_MERCHANT_MAN_UPPER + 1;
        c->BodyPart[BODYPART_GLOVES].Type = MODEL_MERCHANT_MAN_GLOVES + 1;
        c->BodyPart[BODYPART_BOOTS].Type = MODEL_MERCHANT_MAN_BOOTS + 1;
        break;
    case MONSTER_BERDYSH_GUARD:
        c = CreateCharacter(Key, MODEL_PLAYER, PositionX, PositionY);
        wcscpy(c->ID, L"경비병");
        c->BodyPart[BODYPART_HELM].Type = MODEL_PLATE_HELM;
        c->BodyPart[BODYPART_ARMOR].Type = MODEL_PLATE_ARMOR;
        c->BodyPart[BODYPART_PANTS].Type = MODEL_PLATE_PANTS;
        c->BodyPart[BODYPART_GLOVES].Type = MODEL_PLATE_GLOVES;
        c->BodyPart[BODYPART_BOOTS].Type = MODEL_PLATE_BOOTS;
        c->Weapon[0].Type = MODEL_BERDYSH;
        SetCharacterScale(c);
        break;
    case MONSTER_WANDERING_MERCHANT_HAROLD:
        OpenNpc(MODEL_MERCHANT_MAN);
        c = CreateCharacter(Key, MODEL_MERCHANT_MAN, PositionX, PositionY);
        wcscpy(c->ID, L"떠돌이 상인");
        c->BodyPart[BODYPART_HELM].Type = MODEL_MERCHANT_MAN_HEAD;
        c->BodyPart[BODYPART_ARMOR].Type = MODEL_MERCHANT_MAN_UPPER;
        c->BodyPart[BODYPART_GLOVES].Type = MODEL_MERCHANT_MAN_GLOVES;
        c->BodyPart[BODYPART_BOOTS].Type = MODEL_MERCHANT_MAN_BOOTS;
        break;
    case MONSTER_HANZO_THE_BLACKSMITH:
        OpenNpc(MODEL_SMITH);
        c = CreateCharacter(Key, MODEL_SMITH, PositionX, PositionY);
        wcscpy(c->ID, L"대장장이 한스");
        c->Object.Scale = 0.95f;
        break;
    case MONSTER_POTION_GIRL_AMY:
        OpenNpc(MODEL_MERCHANT_GIRL);
        c = CreateCharacter(Key, MODEL_MERCHANT_GIRL, PositionX, PositionY);
        wcscpy(c->ID, L"물약파는 소녀");
        c->BodyPart[BODYPART_HELM].Type = MODEL_MERCHANT_GIRL_HEAD;
        c->BodyPart[BODYPART_ARMOR].Type = MODEL_MERCHANT_GIRL_UPPER;
        c->BodyPart[BODYPART_PANTS].Type = MODEL_MERCHANT_GIRL_LOWER;
        break;
    case MONSTER_PASI_THE_MAGE:
        OpenNpc(MODEL_SCIENTIST);
        c = CreateCharacter(Key, MODEL_SCIENTIST, PositionX, PositionY);
        wcscpy(c->ID, L"마법사 파시");
        break;
    case MONSTER_LUMEN_THE_BARMAID:
        OpenNpc(MODEL_MERCHANT_FEMALE);
        c = CreateCharacter(Key, MODEL_MERCHANT_FEMALE, PositionX, PositionY);
        wcscpy(c->ID, L"술집마담 리아먼");
        c->BodyPart[BODYPART_HELM].Type = MODEL_MERCHANT_FEMALE_HEAD + 1;
        c->BodyPart[BODYPART_ARMOR].Type = MODEL_MERCHANT_FEMALE_UPPER + 1;
        c->BodyPart[BODYPART_PANTS].Type = MODEL_MERCHANT_FEMALE_LOWER + 1;
        c->BodyPart[BODYPART_BOOTS].Type = MODEL_MERCHANT_FEMALE_BOOTS + 1;
        break;
    case MONSTER_WOLF_STATUS:
        OpenNpc(MODEL_CRYWOLF_STATUE);
        c = CreateCharacter(Key, MODEL_CRYWOLF_STATUE, PositionX, PositionY);
        wcscpy(c->ID, L"석상");
        c->Object.Live = false;
        break;
    case MONSTER_WOLF_ALTAR1:
        OpenNpc(MODEL_CRYWOLF_ALTAR1);
        c = CreateCharacter(Key, MODEL_CRYWOLF_ALTAR1, PositionX, PositionY);
        wcscpy(c->ID, L"제단1");
        c->Object.Position[2] -= 10.0f;
        c->Object.HiddenMesh = -2;
        c->Object.Visible = false;
        c->Object.EnableShadow = false;
        break;
    case MONSTER_WOLF_ALTAR2:
        OpenNpc(MODEL_CRYWOLF_ALTAR2);
        c = CreateCharacter(Key, MODEL_CRYWOLF_ALTAR2, PositionX, PositionY);
        wcscpy(c->ID, L"제단2");
        c->Object.HiddenMesh = -2;
        c->Object.Position[2] -= 10.0f;
        c->Object.Visible = false;
        c->Object.EnableShadow = false;
        break;
    case MONSTER_WOLF_ALTAR3:
        OpenNpc(MODEL_CRYWOLF_ALTAR3);
        c = CreateCharacter(Key, MODEL_CRYWOLF_ALTAR3, PositionX, PositionY);
        wcscpy(c->ID, L"제단3");
        c->Object.HiddenMesh = -2;
        c->Object.Position[2] -= 10.0f;
        c->Object.Visible = false;
        c->Object.EnableShadow = false;
        break;
    case MONSTER_WOLF_ALTAR4:
        OpenNpc(MODEL_CRYWOLF_ALTAR4);
        c = CreateCharacter(Key, MODEL_CRYWOLF_ALTAR4, PositionX, PositionY);
        wcscpy(c->ID, L"제단4");
        c->Object.HiddenMesh = -2;
        c->Object.Position[2] -= 10.0f;
        c->Object.Visible = false;
        c->Object.EnableShadow = false;
        break;
    case MONSTER_WOLF_ALTAR5:
        OpenNpc(MODEL_CRYWOLF_ALTAR5);
        c = CreateCharacter(Key, MODEL_CRYWOLF_ALTAR5, PositionX, PositionY);
        wcscpy(c->ID, L"제단5");
        c->Object.HiddenMesh = -2;
        c->Object.Position[2] -= 10.0f;
        c->Object.Visible = false;
        c->Object.EnableShadow = false;
        break;
    case MONSTER_ELPHIS:
        OpenNpc(MODEL_SMELTING_NPC);
        c = CreateCharacter(Key, MODEL_SMELTING_NPC, PositionX + 1, PositionY - 1);
        wcscpy(c->ID, L"제련의탑NPC");
        c->Object.Scale = 2.5f;
        c->Object.EnableShadow = false;
        c->Object.m_bRenderShadow = false;
        break;
    case MONSTER_FIREWORKS_GIRL:
        OpenNpc(MODEL_WEDDING_NPC);
        c = CreateCharacter(Key, MODEL_WEDDING_NPC, PositionX, PositionY);
        wcscpy(c->ID, L"WeddingNPC");
        c->Object.Scale = 1.1f;
        c->Object.EnableShadow = false;
        c->Object.m_bRenderShadow = false;
        break;
    case MONSTER_LUKE_THE_HELPER:
    case MONSTER_LEO_THE_HELPER:
    case MONSTER_HELPER_ELLEN:
        c = CreateCharacter(Key, MODEL_PLAYER, PositionX, PositionY);
        wcscpy(c->ID, L"HelperName");
        c->BodyPart[BODYPART_HELM].Type = MODEL_PLATE_HELM;
        c->BodyPart[BODYPART_ARMOR].Type = MODEL_PLATE_ARMOR;
        c->BodyPart[BODYPART_PANTS].Type = MODEL_PLATE_PANTS;
        c->BodyPart[BODYPART_GLOVES].Type = MODEL_PLATE_GLOVES;
        c->BodyPart[BODYPART_BOOTS].Type = MODEL_PLATE_BOOTS;
        c->Weapon[0].Type = -1;
        SetCharacterScale(c);
        c->Object.m_bpcroom = true;
        break;
    case MONSTER_ORACLE_LAYLA:
        OpenNpc(MODEL_KALIMA_SHOP);
        c = CreateCharacter(Key, MODEL_KALIMA_SHOP, PositionX, PositionY);
        c->Object.Position[2] += 140.0f;
        wcscpy(c->ID, L"KalimaShop");
        break;
    case MONSTER_CHAOS_CARD_MASTER:
    {
        c = CreateCharacter(Key, MODEL_PLAYER, PositionX, PositionY);
        wcscpy(c->ID, L"ChaosCard");
        c->BodyPart[BODYPART_HELM].Type = MODEL_VENOM_MIST_HELM;
        c->BodyPart[BODYPART_ARMOR].Type = MODEL_VENOM_MIST_ARMOR;
        c->BodyPart[BODYPART_PANTS].Type = MODEL_VENOM_MIST_PANTS;
        c->BodyPart[BODYPART_GLOVES].Type = MODEL_VENOM_MIST_GLOVES;
        c->BodyPart[BODYPART_BOOTS].Type = MODEL_VENOM_MIST_BOOTS;
        c->Wing.Type = MODEL_WINGS_OF_HEAVEN;
        int iLevel = 9;
        c->BodyPart[BODYPART_HELM].Level = iLevel;
        c->BodyPart[BODYPART_ARMOR].Level = iLevel;
        c->BodyPart[BODYPART_PANTS].Level = iLevel;
        c->BodyPart[BODYPART_GLOVES].Level = iLevel;
        c->BodyPart[BODYPART_BOOTS].Level = iLevel;
        c->Weapon[0].Type = -1;
        SetCharacterScale(c);
        c->Object.SubType = Type;
    }
    break;
    case MONSTER_PAMELA_THE_SUPPLIER:
    {
        OpenNpc(MODEL_BC_NPC1);
        c = CreateCharacter(Key, MODEL_BC_NPC1, PositionX, PositionY);
        wcscpy(c->ID, L"공성 NPC");
        c->Object.Scale = 1.0f;
        c->Object.Angle[2] = 0.f;
        CreateObject(MODEL_BC_BOX, c->Object.Position, c->Object.Angle);
    }
    break;
    case MONSTER_ANGELA_THE_SUPPLIER:
    {
        OpenNpc(MODEL_BC_NPC2);
        c = CreateCharacter(Key, MODEL_BC_NPC2, PositionX, PositionY);
        wcscpy(c->ID, L"공성 NPC");
        c->Object.Scale = 1.0f;
        c->Object.Angle[2] = 90.f;
        CreateObject(MODEL_BC_BOX, c->Object.Position, c->Object.Angle);
    }
    break;
    case MONSTER_PRIEST_DEVIN:
        OpenNpc(MODEL_NPC_DEVIN);
        c = CreateCharacter(Key, MODEL_NPC_DEVIN, PositionX, PositionY);
        wcscpy(c->ID, L"사제데빈");
        break;
    case MONSTER_WEREWOLF_QUARREL:
        OpenNpc(MODEL_NPC_QUARREL);
        c = CreateCharacter(Key, MODEL_NPC_QUARREL, PositionX, PositionY);
        wcscpy(c->ID, L"웨어울프쿼렐");
        c->Object.Scale = 1.9f;
        break;
    case MONSTER_GATEKEEPER:
        OpenNpc(MODEL_NPC_CASTEL_GATE);
        c = CreateCharacter(Key, MODEL_NPC_CASTEL_GATE, PositionX, PositionY, 90.f);
        wcscpy(c->ID, L"성문");
        o = &c->Object;
        o->Position[2] = RequestTerrainHeight(o->Position[0], o->Position[1]) + 240.f;
        c->Object.Scale = 1.2f;
        c->Object.m_fEdgeScale = 1.1f;
        c->Object.EnableShadow = false;
        c->Object.m_bRenderShadow = false;
        break;
    case MONSTER_LUNAR_RABBIT:
    {
        OpenMonsterModel(MONSTER_MODEL_LUNAR_RABBIT);
        c = CreateCharacter(Key, MODEL_LUNAR_RABBIT, PositionX, PositionY);
        wcscpy(c->ID, L"달토끼");
        c->Object.Scale = 0.8f;
        c->Weapon[0].Type = -1;
        c->Weapon[1].Type = -1;
        c->Object.SubType = rand() % 3;
        c->Object.m_iAnimation = 0;

        BoneManager::RegisterBone(c, L"Rabbit_1", 3);		// Bip01 Spine
        BoneManager::RegisterBone(c, L"Rabbit_2", 16);		// Bip01 Head
        BoneManager::RegisterBone(c, L"Rabbit_3", 15);		// Bip01 Neck1
        BoneManager::RegisterBone(c, L"Rabbit_4", 2);		// Bip01 Pelvis
    }

    break;
    case MONSTER_CHERRY_BLOSSOM_SPIRIT:
    {
        OpenNpc(MODEL_NPC_CHERRYBLOSSOM);
        c = CreateCharacter(Key, MODEL_NPC_CHERRYBLOSSOM, PositionX, PositionY);
        c->Object.Scale = 0.65f;
        c->Object.m_fEdgeScale = 1.08f;
        o = &c->Object;
        o->Position[2] = RequestTerrainHeight(o->Position[0], o->Position[1]) + 170.f;
        wcscpy(c->ID, L"벚꽃의정령");
    }
    break;
    case MONSTER_CHERRY_BLOSSOM_TREE:
    {
        OpenNpc(MODEL_NPC_CHERRYBLOSSOMTREE);
        c = CreateCharacter(Key, MODEL_NPC_CHERRYBLOSSOMTREE, PositionX, PositionY);
        c->Object.Scale = 1.0f;
        c->Object.m_fEdgeScale = 0.0f;
        c->Object.m_bRenderShadow = false;
        wcscpy(c->ID, L"벚꽃나무");
    }
    break;

    case MONSTER_DAVID:
        OpenNpc(MODEL_LUCKYITEM_NPC);
        c = CreateCharacter(Key, MODEL_LUCKYITEM_NPC, PositionX, PositionY);
        wcscpy(c->ID, L"Lucky Item NPC");
        c->Object.Scale = 0.95f;
        c->Object.m_fEdgeScale = 1.2f;
        Models[MODEL_LUCKYITEM_NPC].Actions[0].PlaySpeed = 0.45f;
        Models[MODEL_LUCKYITEM_NPC].Actions[1].PlaySpeed = 0.5f;

        //	Models[MODEL_LUCKYITEM_NPC].Actions[0].PlaySpeed = 50.0f;
        //	Models[MODEL_LUCKYITEM_NPC].Actions[1].PlaySpeed = 50.0f;
        break;
    case MONSTER_SEED_MASTER:
        OpenNpc(MODEL_SEED_MASTER);
        c = CreateCharacter(Key, MODEL_SEED_MASTER, PositionX, PositionY);
        wcscpy(c->ID, L"시드마스터");
        c->Object.Scale = 1.1f;
        c->Object.m_fEdgeScale = 1.2f;
        break;
    case MONSTER_SEED_RESEARCHER:
        OpenNpc(MODEL_SEED_INVESTIGATOR);
        c = CreateCharacter(Key, MODEL_SEED_INVESTIGATOR, PositionX, PositionY);
        wcscpy(c->ID, L"시드연구가");
        c->Object.Scale = 0.9f;
        c->Object.m_fEdgeScale = 1.15f;
        //Models[MODEL_SEED_INVESTIGATOR].Actions[0].PlaySpeed = 0.2f;
        //Models[MODEL_SEED_INVESTIGATOR].Actions[1].PlaySpeed = 0.1f;
        break;
    case MONSTER_REINIT_HELPER:
    {
        c = CreateCharacter(Key, MODEL_PLAYER, PositionX, PositionY);
        //c->Class = 2;
        wcscpy(c->ID, L"초기화 도우미");

        c->BodyPart[BODYPART_HELM].Type = MODEL_PLATE_HELM;
        c->BodyPart[BODYPART_ARMOR].Type = MODEL_PLATE_ARMOR;
        c->BodyPart[BODYPART_PANTS].Type = MODEL_PLATE_PANTS;
        c->BodyPart[BODYPART_GLOVES].Type = MODEL_PLATE_GLOVES;
        c->BodyPart[BODYPART_BOOTS].Type = MODEL_PLATE_BOOTS;

        c->Object.m_fEdgeScale = 1.15f;
        c->Weapon[0].Type = MODEL_LIGHT_CROSSBOW;
        c->Weapon[1].Type = MODEL_BOLT;
        SetCharacterScale(c);
    }
    break;
    case MONSTER_TRANSFORMED_SNOWMAN:
        OpenNpc(MODEL_XMAS2008_SNOWMAN);
        c = CreateCharacter(Key, MODEL_XMAS2008_SNOWMAN, PositionX, PositionY);
        ::wcscpy(c->ID, L"Unknown");
        c->Object.LifeTime = 100;
        c->Object.Scale = 1.3f;
        break;
#ifdef PJH_ADD_PANDA_CHANGERING
    case MONSTER_TRANSFORMED_PANDA:
        c = CreateCharacter(Key, MODEL_PLAYER, PositionX, PositionY);
        ::wcscpy(c->ID, L"Unknown");
        c->Object.SubType = MODEL_PANDA;
        break;
#endif //PJH_ADD_PANDA_CHANGERING
    case MONSTER_TRANSFORMED_SKELETON:
        c = CreateCharacter(Key, MODEL_PLAYER, PositionX, PositionY);
        ::wcscpy(c->ID, L"Unknown");
        c->Object.SubType = MODEL_SKELETON_CHANGED;
        break;
    case MONSTER_LITTLE_SANTA_YELLOW:
    case MONSTER_LITTLE_SANTA_GREEN:
    case MONSTER_LITTLE_SANTA_RED:
    case MONSTER_LITTLE_SANTA_BLUE:
    case MONSTER_LITTLE_SANTA_WHITE:
    case MONSTER_LITTLE_SANTA_BLACK:
    case MONSTER_LITTLE_SANTA_ORANGE:
    case MONSTER_LITTLE_SANTA_PINK:
    {
        int _Model_NpcIndex = MODEL_LITTLESANTA + (Type - 468);

        OpenNpc(_Model_NpcIndex);
        c = CreateCharacter(Key, _Model_NpcIndex, PositionX, PositionY);

        c->Object.Scale = 0.43f;

        for (int i = 0; i < 5; i++)
        {
            if (i < 2 || i == 4)
            {
                //xmassanta_stand_1~2 || xmassanta_idle3
                Models[_Model_NpcIndex].Actions[i].PlaySpeed = 0.4f;
            }
            else// if(i >= 2 && i < 4)
            {
                //xmassanta_idle1~2
                Models[_Model_NpcIndex].Actions[i].PlaySpeed = 0.5f;
            }
        }
        wcscpy(c->ID, L"little santa");
    }
    break;
    case MONSTER_DELGADO:
        //델가도
        OpenNpc(MODEL_NPC_SERBIS);
        c = CreateCharacter(Key, MODEL_NPC_SERBIS, PositionX, PositionY);
        wcscpy(c->ID, L"Unknown");
        break;
    case MONSTER_GATEKEEPER_TITUS:
        // 결투장 문지기 NPC 타이투스
        OpenNpc(MODEL_DUEL_NPC_TITUS);
        c = CreateCharacter(Key, MODEL_DUEL_NPC_TITUS, PositionX, PositionY);
        wcscpy(c->ID, L"Unknown");
        c->Object.Scale = 1.1f;
        c->Object.m_fEdgeScale = 1.2f;
        break;
    case MONSTER_MOSS_THE_MERCHANT:
    {
        OpenNpc(MODEL_GAMBLE_NPC_MOSS);
        c = CreateCharacter(Key, MODEL_GAMBLE_NPC_MOSS, PositionX, PositionY);
        wcscpy(c->ID, L"Unknown");
        c->Object.LifeTime = 100;
        c->Object.Scale = 0.8f;
        c->Object.m_fEdgeScale = 1.1f;

        for (int i = 0; i < 6; i++)
        {
            Models[MODEL_GAMBLE_NPC_MOSS].Actions[i].PlaySpeed = 0.33f;
        }
    }
    break;
    case MONSTER_GOLDEN_RABBIT:
        OpenMonsterModel(MONSTER_MODEL_RABBIT);
        c = CreateCharacter(Key, MODEL_RABBIT, PositionX, PositionY);
        wcscpy(c->ID, L"Unknown");
        c->Object.Scale = 1.0f * 0.95f;
        c->Weapon[0].Type = -1;
        c->Weapon[1].Type = -1;
        break;
    case MONSTER_GOLDEN_DARK_KNIGHT:
        OpenMonsterModel(MONSTER_MODEL_DARK_KNIGHT);
        c = CreateCharacter(Key, MODEL_DARK_KNIGHT, PositionX, PositionY);
        wcscpy(c->ID, L"Unknown");
        c->Object.Scale = 0.8f;
        c->Level = 1;
        c->Weapon[0].Type = MODEL_DOUBLE_BLADE;
        break;
        break;
    case MONSTER_GOLDEN_DEVIL:
        OpenMonsterModel(MONSTER_MODEL_DEVIL);
        c = CreateCharacter(Key, MODEL_DEVIL, PositionX, PositionY);
        wcscpy(c->ID, L"Unknown");
        c->Object.Scale = 1.1f;
        break;
    case MONSTER_GOLDEN_STONE_GOLEM:
        OpenMonsterModel(MONSTER_MODEL_GOLDEN_STONE_GOLEM);
        c = CreateCharacter(Key, MODEL_GOLDEN_STONE_GOLEM, PositionX, PositionY);
        c->Object.Scale = 1.35f;
        c->Weapon[0].Type = -1;
        c->Weapon[1].Type = -1;
        BoneManager::RegisterBone(c, L"Monster101_L_Arm", 12);
        BoneManager::RegisterBone(c, L"Monster101_R_Arm", 20);
        BoneManager::RegisterBone(c, L"Monster101_Head", 6);
        break;
    case MONSTER_GOLDEN_CRUST:
        OpenMonsterModel(MONSTER_MODEL_CRUST);
        c = CreateCharacter(Key, MODEL_CRUST, PositionX, PositionY);
        c->Object.Scale = 1.1f;
        c->Weapon[0].Type = MODEL_THUNDER_BLADE;
        c->Weapon[0].Level = 5;
        c->Weapon[1].Type = MODEL_LEGENDARY_SHIELD;
        c->Weapon[1].Level = 0;
        c->Object.BlendMesh = 1;
        c->Object.BlendMeshLight = 1.f;
        break;
    case MONSTER_GOLDEN_SATYROS:
        OpenMonsterModel(MONSTER_MODEL_SATYROS);
        c = CreateCharacter(Key, MODEL_SATYROS, PositionX, PositionY);
        c->Object.Scale = 1.3f;
        c->Weapon[0].Type = -1;
        c->Weapon[1].Type = -1;
        wcscpy(c->ID, L"Unknown");
        break;
    case MONSTER_GOLDEN_TWIN_TAIL:
        OpenMonsterModel(MONSTER_MODEL_TWIN_TAIL);
        c = CreateCharacter(Key, MODEL_TWIN_TAIL, PositionX, PositionY);
        c->Object.Scale = 1.3f;
        c->Object.Angle[0] = 0.0f;
        c->Object.Gravity = 0.0f;
        c->Object.Distance = (float)(rand() % 20) / 10.0f;
        c->Weapon[0].Type = -1;
        c->Weapon[1].Type = -1;
        BoneManager::RegisterBone(c, L"Twintail_Hair24", 16);
        BoneManager::RegisterBone(c, L"Twintail_Hair32", 24);
        break;
    case MONSTER_GOLDEN_IRON_KNIGHT:
        OpenMonsterModel(MONSTER_MODEL_IRON_KNIGHT);
        c = CreateCharacter(Key, MODEL_IRON_KNIGHT, PositionX, PositionY);
        wcscpy(c->ID, L"Unknown");
        c->Object.Scale = 1.5f;
        c->Weapon[0].Type = -1;
        c->Weapon[1].Type = -1;
        break;
    case MONSTER_GOLDEN_NAPIN:
        OpenMonsterModel(MONSTER_MODEL_NAPIN);
        c = CreateCharacter(Key, MODEL_NAPIN, PositionX, PositionY);
        wcscpy(c->ID, L"Unknown");
        c->Object.Scale = 0.95f;
        c->Weapon[0].Type = -1;
        c->Weapon[1].Type = -1;
        break;
    case MONSTER_GOLDEN_GREAT_DRAGON:
        OpenMonsterModel(MONSTER_MODEL_DRAGON);
        c = CreateCharacter(Key, MODEL_DRAGON_, PositionX, PositionY);
        wcscpy(c->ID, L"Unknown");
        c->Object.Scale = 0.88f;
        c->Weapon[0].Type = -1;
        c->Weapon[1].Type = -1;
        break;
    case MONSTER_LUGARD:
        OpenNpc(MODEL_DOPPELGANGER_NPC_LUGARD);
        c = CreateCharacter(Key, MODEL_DOPPELGANGER_NPC_LUGARD, PositionX, PositionY);
        wcscpy(c->ID, L"Unknown");
        c->Object.Scale = 1.1f;
        c->Object.m_fEdgeScale = 1.2f;
        break;
    case MONSTER_COMPENSATION_BOX:
        OpenNpc(MODEL_DOPPELGANGER_NPC_BOX);
        c = CreateCharacter(Key, MODEL_DOPPELGANGER_NPC_BOX, PositionX, PositionY);
        wcscpy(c->ID, L"Unknown");
        c->Object.Scale = 2.3f;
        c->Object.m_fEdgeScale = 1.1f;
        break;
    case MONSTER_GOLDEN_COMPENSATION_BOX:
        OpenNpc(MODEL_DOPPELGANGER_NPC_GOLDENBOX);
        c = CreateCharacter(Key, MODEL_DOPPELGANGER_NPC_GOLDENBOX, PositionX, PositionY);
        wcscpy(c->ID, L"Unknown");
        c->Object.Scale = 3.3f;
        c->Object.m_fEdgeScale = 1.1f;
        break;
    case MONSTER_GENS_DUPRIAN:
        OpenNpc(MODAL_GENS_NPC_DUPRIAN);
        c = CreateCharacter(Key, MODAL_GENS_NPC_DUPRIAN, PositionX, PositionY);
        wcscpy(c->ID, L"Unknown");
        c->Object.Scale = 1.0f;
        break;
    case MONSTER_GENS_VANERT:
        OpenNpc(MODAL_GENS_NPC_BARNERT);
        c = CreateCharacter(Key, MODAL_GENS_NPC_BARNERT, PositionX, PositionY);
        wcscpy(c->ID, L"Unknown");
        c->Object.Scale = 1.0f;
        break;
    case MONSTER_CHRISTINE_THE_GENERAL_GOODS_MERCHANT:
        OpenNpc(MODEL_UNITEDMARKETPLACE_CHRISTIN);
        c = CreateCharacter(Key, MODEL_UNITEDMARKETPLACE_CHRISTIN, PositionX, PositionY);
        wcscpy(c->ID, L"Unknown");
        c->Object.Scale = 1.1f;
        c->Object.m_fEdgeScale = 1.2f;
        break;
    case MONSTER_JEWELER_RAUL:
        OpenNpc(MODEL_UNITEDMARKETPLACE_RAUL);
        c = CreateCharacter(Key, MODEL_UNITEDMARKETPLACE_RAUL, PositionX, PositionY);
        wcscpy(c->ID, L"Unknown");
        c->Object.Scale = 1.0f;
        c->Object.m_fEdgeScale = 1.15f;
        break;
    case MONSTER_MARKET_UNION_MEMBER_JULIA:
        OpenNpc(MODEL_UNITEDMARKETPLACE_JULIA);
        c = CreateCharacter(Key, MODEL_UNITEDMARKETPLACE_JULIA, PositionX, PositionY);
        wcscpy(c->ID, L"Unknown");
        c->Object.Scale = 1.0f;
        c->Object.m_fEdgeScale = 1.1f;
        break;

    case MONSTER_MERCENARY_GUILD_FELICIA:
        OpenNpc(MODEL_TERSIA);
        c = CreateCharacter(Key, MODEL_TERSIA, PositionX, PositionY);
        wcscpy(c->ID, L"길드관리인 테르시아");
        c->Object.Scale = 0.93f;
        break;
    case MONSTER_PRIESTESS_VEINA:
        OpenNpc(MODEL_BENA);
        c = CreateCharacter(Key, MODEL_BENA, PositionX, PositionY);
        wcscpy(c->ID, L"신녀 베이나");
        c->Object.Position[2] += 145.0f;
        break;
    case MONSTER_LEINA_THE_GENERAL_GOODS_MERCHANT:
        OpenNpc(MODEL_KARUTAN_NPC_REINA);
        c = CreateCharacter(Key, MODEL_KARUTAN_NPC_REINA, PositionX, PositionY);
        wcscpy(c->ID, L"잡화상인 레이나");
        c->Object.Scale = 1.1f;
        c->Object.m_fEdgeScale = 1.2f;
        break;
    case MONSTER_WEAPONS_MERCHANT_BOLO:
        OpenNpc(MODEL_KARUTAN_NPC_VOLVO);
        c = CreateCharacter(Key, MODEL_KARUTAN_NPC_VOLVO, PositionX, PositionY);
        wcscpy(c->ID, L"무기상인 볼로");
        c->Object.Scale = 0.9f;
        break;

    }

    Setting_Monster(c, Type, PositionX, PositionY);

    return c;
}

CHARACTER* CreateHero(int Index, CLASS_TYPE Class, int Skin, float x, float y, float Rotate)
{
    CHARACTER* c = &CharactersClient[Index];
    OBJECT* o = &c->Object;
    CreateCharacterPointer(c, MODEL_PLAYER, 0, 0, Rotate);
    Vector(0.3f, 0.3f, 0.3f, o->Light);
    c->Key = Index;
    o->Position[0] = x;
    o->Position[1] = y;
    c->Class = Class;
    c->SkinIndex = gCharacterManager.GetSkinModelIndex(c->Class);
    c->Skin = Skin;

    g_CharacterClearBuff(o);

    if (SceneFlag == LOG_IN_SCENE)
    {
        c->BodyPart[BODYPART_HELM].Type = MODEL_DARK_MASTER_MASK;
        c->BodyPart[BODYPART_HELM].Level = 7;
        c->BodyPart[BODYPART_ARMOR].Type = MODEL_DARK_MASTER_ARMOR;
        c->BodyPart[BODYPART_ARMOR].Level = 7;
        c->BodyPart[BODYPART_PANTS].Type = MODEL_DARK_MASTER_PANTS;
        c->BodyPart[BODYPART_PANTS].Level = 7;
        c->BodyPart[BODYPART_GLOVES].Type = MODEL_DARK_MASTER_GLOVES;
        c->BodyPart[BODYPART_GLOVES].Level = 7;
        c->BodyPart[BODYPART_BOOTS].Type = MODEL_DARK_MASTER_BOOTS;
        c->BodyPart[BODYPART_BOOTS].Level = 7;
        c->Weapon[0].Type = MODEL_SOLEIL_SCEPTER;
        c->Weapon[1].Type = MODEL_DARK_RAVEN_ITEM;
        c->Weapon[0].Level = 13;
        c->Wing.Type = MODEL_CAPE_OF_LORD;
        c->Helper.Type = MODEL_DARK_HORSE_ITEM;
        //c->Helper.Level = 13;
    }
    else
    {
        c->BodyPart[BODYPART_HELM].Type = MODEL_BODY_HELM + c->SkinIndex;
        c->BodyPart[BODYPART_ARMOR].Type = MODEL_BODY_ARMOR + c->SkinIndex;
        c->BodyPart[BODYPART_PANTS].Type = MODEL_BODY_PANTS + c->SkinIndex;
        c->BodyPart[BODYPART_GLOVES].Type = MODEL_BODY_GLOVES + c->SkinIndex;
        c->BodyPart[BODYPART_BOOTS].Type = MODEL_BODY_BOOTS + c->SkinIndex;
        c->Weapon[0].Type = -1;
        c->Weapon[1].Type = -1;
        c->Wing.Type = -1;
        c->Helper.Type = -1;
    }

    UnRegisterBuff(eBuff_GMEffect, o);

    c->CtlCode = 0;
    SetCharacterScale(c);
    SetPlayerStop(c);
    return c;
}

CHARACTER* CreateHellGate(char* ID, int Key, EMonsterType Index, int x, int y, int CreateFlag)
{
    CHARACTER* portal = CreateMonster(Index, x, y, Key);
    portal->Level = Index - 152 + 1;
    wchar_t portalText[100];
    wchar_t name[sizeof portal->ID];

    CMultiLanguage::ConvertFromUtf8(name, ID);

    swprintf(portalText, portal->ID, name);

    if (portal->Level == 7)
        portal->Object.SubType = 1;

    memcpy(portal->ID, portalText, sizeof portalText);

    if (CreateFlag)
    {
        CreateJoint(BITMAP_JOINT_THUNDER + 1, portal->Object.Position, portal->Object.Position, portal->Object.Angle, 1, NULL, 60.f + rand() % 10);
        CreateJoint(BITMAP_JOINT_THUNDER + 1, portal->Object.Position, portal->Object.Position, portal->Object.Angle, 1, NULL, 50.f + rand() % 10);
        CreateJoint(BITMAP_JOINT_THUNDER + 1, portal->Object.Position, portal->Object.Position, portal->Object.Angle, 1, NULL, 50.f + rand() % 10);
        CreateJoint(BITMAP_JOINT_THUNDER + 1, portal->Object.Position, portal->Object.Position, portal->Object.Angle, 1, NULL, 60.f + rand() % 10);
    }
    return portal;
}

BOOL PlayMonsterSoundGlobal(OBJECT* pObject)
{
    float fDis_x, fDis_y;
    fDis_x = pObject->Position[0] - Hero->Object.Position[0];
    fDis_y = pObject->Position[1] - Hero->Object.Position[1];
    float fDistance = sqrtf(fDis_x * fDis_x + fDis_y * fDis_y);

    if (fDistance > 500.0f) return true;

    switch (pObject->Type)
    {
    case MODEL_CURSED_SANTA:
        if (pObject->CurrentAction == MONSTER01_STOP1)
        {
            // 			if (rand_fps_check(10))
            {
                if (rand_fps_check(2))
                    PlayBuffer(SOUND_XMAS_SANTA_IDLE_1);
                else
                    PlayBuffer(SOUND_XMAS_SANTA_IDLE_2);
            }
        }
        else if (pObject->CurrentAction == MONSTER01_WALK)
        {
            //if (rand_fps_check(10))
            {
                if (rand_fps_check(2))
                    PlayBuffer(SOUND_XMAS_SANTA_WALK_1);
                else
                    PlayBuffer(SOUND_XMAS_SANTA_WALK_2);
            }
        }
        else if (pObject->CurrentAction == MONSTER01_ATTACK1 || pObject->CurrentAction == MONSTER01_ATTACK2)
        {
            PlayBuffer(SOUND_XMAS_SANTA_ATTACK_1);
        }
        else if (pObject->CurrentAction == MONSTER01_SHOCK)
        {
            if (rand_fps_check(2))
                PlayBuffer(SOUND_XMAS_SANTA_DAMAGE_1);
            else
                PlayBuffer(SOUND_XMAS_SANTA_DAMAGE_2);
        }
        else if (pObject->CurrentAction == MONSTER01_DIE)
        {
            PlayBuffer(SOUND_XMAS_SANTA_DEATH_1);
        }
        return TRUE;

    case MODEL_XMAS2008_SNOWMAN:
        if (pObject->CurrentAction == MONSTER01_WALK)
        {
            PlayBuffer(SOUND_XMAS_SNOWMAN_WALK_1);
        }
        else if (pObject->CurrentAction == MONSTER01_ATTACK1)
        {
            PlayBuffer(SOUND_XMAS_SNOWMAN_ATTACK_1);
        }
        else if (pObject->CurrentAction == MONSTER01_ATTACK2)
        {
            PlayBuffer(SOUND_XMAS_SNOWMAN_ATTACK_2);
        }
        else if (pObject->CurrentAction == MONSTER01_SHOCK)
        {
            PlayBuffer(SOUND_XMAS_SNOWMAN_DAMAGE_1);
        }
        else if (pObject->CurrentAction == MONSTER01_DIE)
        {
            if (pObject->LifeTime == 100)
            {
                PlayBuffer(SOUND_XMAS_SNOWMAN_DEATH_1);
            }
        }
        return TRUE;
    case MODEL_DUEL_NPC_TITUS:
        if (pObject->CurrentAction == MONSTER01_STOP1)
        {
            PlayBuffer(SOUND_DUEL_NPC_IDLE_1);
        }
        return TRUE;
    case MODEL_DOPPELGANGER_NPC_LUGARD:
        if (pObject->CurrentAction == MONSTER01_STOP1)
        {
            if (rand_fps_check(2))
                PlayBuffer(SOUND_DOPPELGANGER_LUGARD_BREATH);
        }
        return TRUE;
    case MODEL_DOPPELGANGER_NPC_BOX:
        if (pObject->CurrentAction == MONSTER01_DIE)
        {
            PlayBuffer(SOUND_DOPPELGANGER_JEWELBOX_OPEN);
        }
        return TRUE;
    case MODEL_DOPPELGANGER_NPC_GOLDENBOX:
        if (pObject->CurrentAction == MONSTER01_DIE)
        {
            PlayBuffer(SOUND_DOPPELGANGER_JEWELBOX_OPEN);
        }
        return TRUE;
    }

    return FALSE;
}

bool IsBackItem(CHARACTER* c, int iType)
{
    int iBowType = gCharacterManager.GetEquipedBowType(c);
    if (iBowType != BOWTYPE_NONE)
    {
        return true;
    }

    if (iType >= MODEL_SWORD && iType < MODEL_SHIELD + MAX_ITEM_INDEX && !(iType >= MODEL_BOOK_OF_SAHAMUTT && iType <= MODEL_STAFF + 29))
    {
        return true;
    }

    return false;
}

bool RenderCharacterBackItem(CHARACTER* c, OBJECT* o, bool bTranslate)
{
    bool bBindBack = false;

    if (c->SafeZone)
    {
        bBindBack = true;
    }
    if (o->CurrentAction >= PLAYER_GREETING1 && o->CurrentAction <= PLAYER_SALUTE1)
    {
        bBindBack = true;
    }
    if ((gMapManager.WorldActive == WD_7ATLANSE || gMapManager.InHellas() || gMapManager.WorldActive == WD_67DOPPLEGANGER3) && (o->CurrentAction == PLAYER_WALK_SWIM || o->CurrentAction == PLAYER_RUN_SWIM))
    {
        bBindBack = true;
    }

    if (gMapManager.InBloodCastle() == true)
    {
        bBindBack = false;
        if (IsGMCharacter() == true)
        {
            return bBindBack;
        }
    }
    if (gMapManager.InChaosCastle() == true)
    {
        bBindBack = false;
    }

    if (o->Type == MODEL_PLAYER)
    {
        bool bBack = false;
        int iBackupType = -1;

        for (int i = 0; i < 2; ++i)
        {
            int iType = c->Weapon[i].Type;
            int iLevel = c->Weapon[i].Level;
            int iOption1 = c->Weapon[i].ExcellentFlags;

            if (iType < 0)
                continue;

            if (o->Kind == KIND_NPC && gMapManager.WorldActive == WD_0LORENCIA && o->Type == MODEL_PLAYER && (o->SubType >= MODEL_SKELETON1 && o->SubType <= MODEL_SKELETON3))
            {
                if (i == 0)
                {
                    bBack = true;
                    iType = MODEL_GOLDEN_CROSSBOW;
                    iLevel = 8;
                }
            }

            if (IsBackItem(c, iType) == true)
            {
                bBack = true;
            }

            if (iType == MODEL_BOLT || iType == MODEL_ARROWS)
            {
                bBack = true;
            }
            else
            {
                if (bBindBack == false)
                    bBack = false;
            }

            // 			if(iBackupType == iType)
            // 			{
            // 				bBack = false;
            // 			}

            if (bBack && iType != -1)
            {
                PART_t* w = &c->Wing;

                float fAnimationFrameBackUp = w->AnimationFrame;

                w->LinkBone = 47;

                if (o->CurrentAction == PLAYER_FLY || o->CurrentAction == PLAYER_FLY_CROSSBOW)
                {
                    w->PlaySpeed = 1.f;
                }
                else
                {
                    w->PlaySpeed = 0.25f;
                }

                PART_t t_Part;

                if (iType >= MODEL_SWORD && iType < MODEL_SHIELD + MAX_ITEM_INDEX)
                {
                    ::memcpy(&t_Part, w, sizeof(PART_t));
                    t_Part.CurrentAction = 0;
                    t_Part.AnimationFrame = 0.f;
                    t_Part.PlaySpeed = 0.f;
                    t_Part.PriorAction = 0;
                    t_Part.PriorAnimationFrame = 0.f;
                }

                if (iType == MODEL_STINGER_BOW)
                {
                    PART_t* pWeapon = &c->Weapon[1];
                    BYTE byTempLinkBone = pWeapon->LinkBone;
                    pWeapon->CurrentAction = 2;
                    pWeapon->PlaySpeed = 0.25f;
                    pWeapon->LinkBone = 47;
                    RenderLinkObject(0.f, 0.f, 15.f, c, pWeapon, iType, iLevel, iOption1, true, bTranslate);
                    pWeapon->LinkBone = byTempLinkBone;
                }
                else if (g_CMonkSystem.IsSwordformGloves(iType))
                    g_CMonkSystem.RenderSwordformGloves(c, iType, i, o->Alpha, bTranslate);
                else
                {
                    bool bRightHandItem = false;
                    if (i == 0)
                        bRightHandItem = true;
                    RenderLinkObject(0.f, 0.f, 15.f, c, &t_Part, iType, iLevel, iOption1, true, bTranslate, 0, bRightHandItem);
                }

                w->AnimationFrame = fAnimationFrameBackUp;
            }

            iBackupType = iType;
        }

        if (gMapManager.InBloodCastle() && c->EtcPart != 0)
        {
            PART_t* w = &c->Wing;

            int iType = 0;
            int iLevel = 0;
            int iOption1 = 0;

            w->LinkBone = 47;
            if (o->CurrentAction == PLAYER_FLY || o->CurrentAction == PLAYER_FLY_CROSSBOW)
                w->PlaySpeed = 1.f;
            else
                w->PlaySpeed = 0.25f;

            switch (c->EtcPart)
            {
            case 1: iType = MODEL_DIVINE_STAFF_OF_ARCHANGEL; break;
            case 2: iType = MODEL_DIVINE_SWORD_OF_ARCHANGEL; break;
            case 3: iType = MODEL_DIVINE_CB_OF_ARCHANGEL; break;
            }

            RenderLinkObject(0.f, 0.f, 15.f, c, w, iType, iLevel, iOption1, true, bTranslate);
        }

        CreatePartsFactory(c);
        RenderParts(c);

        if (gMapManager.InChaosCastle() == false)
        {
            PART_t* w = &c->Wing;
            if (w->Type != -1)
            {
                w->LinkBone = 47;
                if (o->CurrentAction == PLAYER_FLY || o->CurrentAction == PLAYER_FLY_CROSSBOW)
                {
                    if (w->Type == MODEL_WING_OF_STORM)
                    {
                        w->PlaySpeed = 0.5f;
                    }
                    else
                    {
                        w->PlaySpeed = 1.f;
                    }
                }
                else
                {
                    w->PlaySpeed = 0.25f;
                }

                switch (w->Type)
                {
                case MODEL_CAPE_OF_EMPEROR:
                case MODEL_CAPE_OF_OVERRULE:
                    w->LinkBone = 19;
                    RenderLinkObject(0.f, 0.f, 15.f, c, w, w->Type, w->Level, w->ExcellentFlags, true, bTranslate);
                    break;
                default:
                    RenderLinkObject(0.f, 0.f, 15.f, c, w, w->Type, w->Level, w->ExcellentFlags, false, bTranslate);
                    break;
                }
            }

            // 사탄
            int iType = c->Helper.Type;
            int iLevel = c->Helper.Level;
            int iOption1 = 0;
            if (iType == MODEL_IMP)
            {
                PART_t* w = &c->Helper;
                w->LinkBone = 34;
                w->PlaySpeed = 0.5f;
                iOption1 = w->ExcellentFlags;
                vec3_t vRelativePos;
                if (gCharacterManager.GetBaseClass(c->Class) == CLASS_RAGEFIGHTER
                    && (c->BodyPart[BODYPART_ARMOR].Type == MODEL_SACRED_ARMOR
                        || c->BodyPart[BODYPART_ARMOR].Type == MODEL_STORM_HARD_ARMOR
                        || c->BodyPart[BODYPART_ARMOR].Type == MODEL_PIERCING_ARMOR
                        || c->BodyPart[BODYPART_ARMOR].Type == MODEL_PHOENIX_SOUL_ARMOR))
                {
                    RenderLinkObject(20.f, -5.f, 20.f, c, w, iType, iLevel, iOption1, false, bTranslate);
                    Vector(20.f, -5.f, 35.f, vRelativePos);
                }
                else
                {
                    RenderLinkObject(20.f, 0.f, 0.f, c, w, iType, iLevel, iOption1, false, bTranslate);
                    Vector(20.f, 0.f, 15.f, vRelativePos);
                }

                vec3_t vPos;
                Models[o->Type].TransformPosition(o->BoneTransform[w->LinkBone], vRelativePos, vPos, true);

                vec3_t vLight;
                float fLuminosity = (float)(rand() % 30 + 70) * 0.01f;
                Vector(fLuminosity * 0.5f, fLuminosity * 0.f, fLuminosity * 0.f, vLight);
                CreateSprite(BITMAP_LIGHT, vPos, 1.5f, vLight, o);
            }
        }
    }

    return bBindBack;
}

bool IsPlayer(CHARACTER* c)
{
    return c && c->Object.Kind == KIND_PLAYER;
}

bool IsMonster(CHARACTER* c)
{
    return c && c->Object.Kind == KIND_MONSTER;
}
