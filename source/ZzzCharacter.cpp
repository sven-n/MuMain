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
#include "wsclientinline.h"
#include "PhysicsManager.h"
#include "GOBoid.h"
#include "CSItemOption.h"
#include "CSChaosCastle.h"
#ifdef PET_SYSTEM
#include "GIPetManager.h"
#endif// PET_SYSTEM
#include "GMHellas.h"
#include "CSParts.h"
#include "GMBattleCastle.h"
#include "GMHuntingGround.h"
#ifdef CRYINGWOLF_2NDMVP
#include "GMCryingWolf2nd.h"
#endif // CRYINGWOLF_2NDMVP
#include "CSMapInterface.h"
#include "BoneManager.h"
#include "GMAida.h"
#include "GMCryWolf1st.h"
#include "GM_Kanturu_1st.h"
#include "GM_Kanturu_2nd.h"
#include "CDirection.h"
#include "GM_Kanturu_3rd.h"
#include "GM3rdChangeUp.h"
#include "Input.h"
#include "ChangeRingManager.h"
#include "Event.h"
#include "PartyManager.h"
#include "GMNewTown.h"
#include "w_CursedTemple.h"
#include "SummonSystem.h"

#include "GMSwampOfQuiet.h"

// 맵 관련 include
#ifdef PSW_ADD_MAPSYSTEM
#include "w_MapHeaders.h"
#endif // PSW_ADD_MAPSYSTEM

#ifdef LDK_ADD_NEW_PETPROCESS
#include "w_PetProcess.h"
#endif //LDK_ADD_NEW_PETPROCESS
#ifdef YDG_ADD_NEW_DUEL_SYSTEM
#include "DuelMgr.h"
#endif	// YDG_ADD_NEW_DUEL_SYSTEM
#ifdef PBG_ADD_NEWCHAR_MONK
#include "MonkSystem.h"
#endif //PBG_ADD_NEWCHAR_MONK

//#include "GMEmpireGuardian1.h"
//--------------------------------------------------------------------------------------------------------------------

CHARACTER *CharactersClient;
CHARACTER CharacterView;
CHARACTER *Hero;
Script_Skill MonsterSkill[MODEL_MONSTER_END];
extern CKanturuDirection KanturuDirection;
float g_fBoneSave[10][3][4];	// 임시 저장을 위한 공간
//  사막 보스 ( 데스 나이트 빔 )몬스터를 위한 인덱스 데이터.
//  본 위치 인덱스.
static  char    vec_list[35] = 
{
     5,  6, 33, 53, 35, 49, 50, 45, 46, 41, 
    42, 37, 38, 11, 31, 13, 27, 28, 23, 24, 
    19, 20, 15, 16, 54, 55, 62, 69, 70, 77,  
     2, 79, 81, 84, 86
};
static  char    wingLeft[15][2] = 
{
    {0,2},  {2,3}, {2,4},  {4,5}, {5,6}, {4,7}, {7,8}, {4,9}, {9,10}, {4,11}, {11,12},
    {6,5}, {8,7}, {10,9},{12,11}
};
static  char    wingRight[15][2] = 
{
    {0,13}, {13,14}, {13,15}, {15,16}, {16,17}, {15,18}, {18,19}, {15,20}, {20,21}, {15,22}, {22,23},
    {17,16},{19,18}, {21,20}, {23,22}
};
static  char    arm_leg_Left[4][2] =
{
    {29,28}, {28,27}, {34,33}, {33,30}, 
};
static  char    arm_leg_Right[4][2] =
{
    {26,25}, {25,24}, {32,31}, {31,30}, 
};

//  
static  char    g_chStar[10] = 
{
    10, 18, 37, 38, 51, 52, 58, 59, 66, 24
};

//  셋트 장비.
int  EquipmentLevelSet = 0;
bool g_bAddDefense = false;

int g_iLimitAttackTime = 15;

//  정지 시간 계산을 위한 백업 위치값.
static  int     g_iOldPositionX = 0;
static  int     g_iOldPositionY = 0;
static  float   g_fStopTime = 0.f;

void RegisterBuff( eBuffState buff, OBJECT* o, const int bufftime = 0 );
void UnRegisterBuff( eBuffState buff, OBJECT* o );

extern CGuildCache g_GuildCache;

//--------------------------------------------------------------------------------------------------------------------

// 펜릴 타입 구하는 함수
int GetFenrirType(CHARACTER* c)
{	
	if(c->Helper.Option1 == 0x01)
		return FENRIR_TYPE_BLACK;
	else if(c->Helper.Option1 == 0x02)
		return FENRIR_TYPE_BLUE;
	else if(c->Helper.Option1 == 0x04)
		return FENRIR_TYPE_GOLD;
	
	return FENRIR_TYPE_RED;
}

void FallingMonster(CHARACTER *c, OBJECT* o)
{
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
	if(c == NULL)	return;
	if(o == NULL)	return;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	float AngleY = 0.0f;
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	float AngleY;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX

	o->Gravity		+= 2.5f;
	o->Angle[0]		-= 4.f;
	o->m_bActionStart = true;
	o->Direction[1]	+= o->Direction[0];

	if(o->Gravity >= 1.f)
		AngleY = o->Angle[2];

	if(o->Angle[0] <= -90.0f)
		o->Angle[0] = -90.0f;
	
	o->Angle[2] = AngleY;

	if(o->Gravity >= 150.0f)
		o->Gravity = 150.0f;

	o->Position[2] -= o->Gravity;
}

#ifdef YDG_ADD_SANTA_MONSTER
BOOL PlayMonsterSoundGlobal(OBJECT * pObject);	// 전역 몬스터 사운드 처리용
#endif	// YDG_ADD_SANTA_MONSTER

BOOL PlayMonsterSound(OBJECT * pObject)	// 몬스터 정지, 이동, 공격, 사망 동작 사운드
{
	// 정지, 이동 동작은 동작중에 계속 호출되고 (발자국 소리 반복)
	// 공격, 사망 동작은 동작 시작시에 한번 호출된다. (0프레임에서)

#ifdef YDG_ADD_SANTA_MONSTER
	// 전역맵 몬스터 사운드
	if (PlayMonsterSoundGlobal(pObject))
	{
		return TRUE;
	}
#endif	// YDG_ADD_SANTA_MONSTER

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

#ifdef PSW_ADD_MAPSYSTEM
	if(TheMapProcess().PlayMonsterSound( pObject ) == true)
	{
		return TRUE;
	}
#endif //PSW_ADD_MAPSYSTEM

	return FALSE;
}

///////////////////////////////////////////////////////////////////////////////
// 캐릭터 애니메이션 설정(제자리 동작)
///////////////////////////////////////////////////////////////////////////////
void SetPlayerStop(CHARACTER *c)
{
	c->Run = 0;

	OBJECT *o = &c->Object;
	if(c->Object.Type==MODEL_PLAYER)
	{
		//^ 펜릴 캐릭터 에니메이션 정지동작 설정
		if(c->Helper.Type == MODEL_HELPER+37 && !c->SafeZone)
		{
#ifndef CSK_FIX_FENRIR_RUN		// 정리할 때 지워야 하는 소스	
			g_bFenrir_Run = FALSE;
#endif //! CSK_FIX_FENRIR_RUN	// 정리할 때 지워야 하는 소스
	
#ifdef PBG_ADD_NEWCHAR_MONK_ANI
			if(GetBaseClass(c->Class) == CLASS_RAGEFIGHTER)//레이지파이터이면
			{
				if(c->Weapon[0].Type != -1 && c->Weapon[1].Type != -1)
					SetAction(&c->Object, PLAYER_RAGE_FENRIR_STAND_TWO_SWORD);
				else if(c->Weapon[0].Type != -1 && c->Weapon[1].Type == -1)
					SetAction(&c->Object, PLAYER_RAGE_FENRIR_STAND_ONE_RIGHT);
				else if(c->Weapon[0].Type == -1 && c->Weapon[1].Type != -1)
					SetAction(&c->Object, PLAYER_RAGE_FENRIR_STAND_ONE_LEFT);
 				else
					SetAction(&c->Object, PLAYER_RAGE_FENRIR_STAND);
			}
			else
			{
#endif //PBG_ADD_NEWCHAR_MONK_ANI
			if(c->Weapon[0].Type != -1 && c->Weapon[1].Type != -1)	// 양손무기
				SetAction(&c->Object, PLAYER_FENRIR_STAND_TWO_SWORD);
			else if(c->Weapon[0].Type != -1 && c->Weapon[1].Type == -1) // 오른손 무기
				SetAction(&c->Object, PLAYER_FENRIR_STAND_ONE_RIGHT);
			else if(c->Weapon[0].Type == -1 && c->Weapon[1].Type != -1) // 왼손 무기
				SetAction(&c->Object, PLAYER_FENRIR_STAND_ONE_LEFT);
 			else	// 맨손
				SetAction(&c->Object, PLAYER_FENRIR_STAND);
#ifdef PBG_ADD_NEWCHAR_MONK_ANI
			}
#endif //PBG_ADD_NEWCHAR_MONK_ANI
		}
		else if ( c->Helper.Type==MODEL_HELPER+4 && !c->SafeZone )
		{
			if(c->Weapon[0].Type==-1 && c->Weapon[1].Type==-1)
				SetAction(&c->Object,PLAYER_STOP_RIDE_HORSE );
			else
				SetAction(&c->Object,PLAYER_STOP_RIDE_HORSE );
		}
        //  다크스피릿 마을에서 정지 상태.
#ifdef PBG_FIX_DARKSPIRIT_ACTION
		else if(c->SafeZone && c->m_PetInfo->m_dwPetType == PET_TYPE_DARK_SPIRIT
#ifdef YDG_FIX_DARKSPIRIT_CHAOSCASTLE_CRASH
			&& !InChaosCastle()
#endif	// YDG_FIX_DARKSPIRIT_CHAOSCASTLE_CRASH
			)
#else //PBG_FIX_DARKSPIRIT_ACTION
        else if ( c->SafeZone && c->Weapon[1].Type==MODEL_HELPER+5 )
#endif //PBG_FIX_DARKSPIRIT_ACTION
        {
            SetAction ( &c->Object, PLAYER_DARKLORD_STAND );
        }
        else
		if( ( c->Helper.Type==MODEL_HELPER+2 || c->Helper.Type==MODEL_HELPER+3 ) && !c->SafeZone)
		{
			if(c->Weapon[0].Type==-1 && c->Weapon[1].Type==-1)
				SetAction(&c->Object,PLAYER_STOP_RIDE);
#ifdef PBG_ADD_NEWCHAR_MONK_ANI
			else if(GetBaseClass(c->Class)==CLASS_RAGEFIGHTER)
				SetAction(&c->Object,PLAYER_RAGE_UNI_STOP_ONE_RIGHT);
#endif //PBG_ADD_NEWCHAR_MONK_ANI
			else
				SetAction(&c->Object,PLAYER_STOP_RIDE_WEAPON);
		}
		else
		{
			bool Fly = false;

			if( !(c->Object.SubType == MODEL_CURSEDTEMPLE_ALLIED_PLAYER || c->Object.SubType == MODEL_CURSEDTEMPLE_ILLUSION_PLAYER) && 
				!c->SafeZone && c->Wing.Type!=-1) Fly = true;

           	int Index = TERRAIN_INDEX_REPEAT((int)(c->Object.Position[0]/TERRAIN_SCALE),(int)(c->Object.Position[1]/TERRAIN_SCALE));
			
            if ( SceneFlag==MAIN_SCENE && 
                 ( World==WD_7ATLANSE || InHellas()
#ifdef YDG_ADD_MAP_DOPPELGANGER3
					|| World == WD_67DOPPLEGANGER3
#endif	// YDG_ADD_MAP_DOPPELGANGER3
                 ) && (TerrainWall[Index]&TW_SAFEZONE)!=TW_SAFEZONE ) Fly = true;
			
			if( c->MonsterIndex == 257) 
				Fly = true;
			
            if(Fly)
			{
				// MVP 일때 요정이 제단과 계약 된 상태 이면서 공격등을 받아서 계약된 상태 일때의 에니메이션이 풀리는 경우에 다시 서버에 요청한다
				if( g_isCharacterBuff((&c->Object), eBuff_CrywolfHeroContracted) )
				{
					if(c->Object.CurrentAction != PLAYER_HEALING_FEMALE1)
					{
						SetAction(&c->Object,PLAYER_HEALING_FEMALE1);
						SendRequestAction(AT_HEALING1,((BYTE)((c->Object.Angle[2]+22.5f)/360.f*8.f+1.f)%8));
					}
				}
				else
				{	
#ifdef ADD_SOCKET_ITEM
					// 석궁일때
					if( GetEquipedBowType( c ) == BOWTYPE_CROSSBOW )
					{
						SetAction( &c->Object, PLAYER_STOP_FLY_CROSSBOW );
					}
					else
					{
						SetAction( &c->Object, PLAYER_STOP_FLY );
					}
#else // ADD_SOCKET_ITEM				// 정리할 때 지워야 하는 소스
					//. 석궁
					if((c->Weapon[0].Type>=MODEL_BOW+8 && c->Weapon[0].Type<MODEL_BOW+15) ||
						(c->Weapon[0].Type>=MODEL_BOW+16 && c->Weapon[0].Type<MODEL_BOW+17) || 
						(c->Weapon[0].Type>=MODEL_BOW+18 && c->Weapon[0].Type<MODEL_BOW+MAX_ITEM_INDEX) )
	     				SetAction(&c->Object,PLAYER_STOP_FLY_CROSSBOW);
					else	//. 활
	     				SetAction(&c->Object,PLAYER_STOP_FLY);
#endif // ADD_SOCKET_ITEM				// 정리할 때 지워야 하는 소스
				}
			}
			else
			{
				// MVP 일때 요정이 제단과 계약 된 상태 이면서 공격등을 받아서 계약된 상태 일때의 에니메이션이 풀리는 경우에 다시 서버에 요청한다
				if( g_isCharacterBuff((&c->Object), eBuff_CrywolfHeroContracted) )
				{
					if(c->Object.CurrentAction != PLAYER_HEALING_FEMALE1)
					{
						SetAction(&c->Object,PLAYER_HEALING_FEMALE1);
						SendRequestAction(AT_HEALING1,((BYTE)((c->Object.Angle[2]+22.5f)/360.f*8.f+1.f)%8));
					}
				}
				else
				{
                //  무기가 없거나. 블러듴캐슬이 아닌 안전지대 일때
			    if ( ( c->Weapon[0].Type==-1 && c->Weapon[1].Type==-1 ) || ( c->SafeZone && ( InBloodCastle() == false ) ) )
				{
					if (GetBaseClass(c->Class) == CLASS_ELF)
						SetAction(&c->Object, PLAYER_STOP_FEMALE);
					else if (GetBaseClass(c->Class) == CLASS_SUMMONER && !InChaosCastle())	// 소환술사는 카오스캐슬에선 남자 애니메이션.
						SetAction(&c->Object, PLAYER_STOP_SUMMONER);
#ifdef PBG_ADD_NEWCHAR_MONK_ANI
					else if (GetBaseClass(c->Class) == CLASS_RAGEFIGHTER)
						SetAction(&c->Object, PLAYER_STOP_RAGEFIGHTER);
#endif //PBG_ADD_NEWCHAR_MONK_ANI
					else
						SetAction(&c->Object, PLAYER_STOP_MALE);
				}
				else
				{
                    //  칼 장착.
					if(c->Weapon[0].Type>=MODEL_SWORD && c->Weapon[0].Type<MODEL_MACE+MAX_ITEM_INDEX)
					{
						if(!ItemAttribute[c->Weapon[0].Type-MODEL_ITEM].TwoHand)
						{
							SetAction(&c->Object,PLAYER_STOP_SWORD);
						}
						else if ( c->Weapon[0].Type==MODEL_SWORD+21 || c->Weapon[0].Type==MODEL_SWORD+31 || c->Weapon[0].Type==MODEL_SWORD+23 || c->Weapon[0].Type==MODEL_SWORD+25)
						{
							SetAction ( &c->Object, PLAYER_STOP_TWO_HAND_SWORD_TWO );
						}
						else
						{
							SetAction(&c->Object,PLAYER_STOP_TWO_HAND_SWORD);
						}
					}
                    //  창 장착.
    				else if(c->Weapon[0].Type==MODEL_SPEAR+1 || c->Weapon[0].Type==MODEL_SPEAR+2)
					{
						SetAction(&c->Object,PLAYER_STOP_SPEAR);
					}
                    //  창 장착.
					else if(c->Weapon[0].Type>=MODEL_SPEAR && c->Weapon[0].Type<MODEL_SPEAR+MAX_ITEM_INDEX)
					{
						if(!ItemAttribute[c->Weapon[0].Type-MODEL_ITEM].TwoHand)
							SetAction(&c->Object,PLAYER_STOP_SWORD);
						else
							SetAction(&c->Object,PLAYER_STOP_SCYTHE);
					}
					// 소환술사 스틱.
					else if (c->Weapon[0].Type >= MODEL_STAFF+14 && c->Weapon[0].Type <= MODEL_STAFF+20)
					{
						::SetAction(&c->Object, PLAYER_STOP_WAND);
					}
					else if(c->Weapon[0].Type>=MODEL_STAFF && c->Weapon[0].Type<MODEL_STAFF+MAX_ITEM_INDEX)
					{
						if(!ItemAttribute[c->Weapon[0].Type-MODEL_ITEM].TwoHand)
							SetAction(&c->Object,PLAYER_STOP_SWORD);
						else
							SetAction(&c->Object,PLAYER_STOP_SCYTHE);
					}
#ifdef ADD_SOCKET_ITEM
					// 활
					else if( GetEquipedBowType( c ) == BOWTYPE_BOW )
					{
						SetAction( &c->Object, PLAYER_STOP_BOW );
					}
					// 석궁
					else if( GetEquipedBowType( c ) == BOWTYPE_CROSSBOW )
					{
						SetAction( &c->Object, PLAYER_STOP_CROSSBOW );
					}
#else // ADD_SOCKET_ITEM			// 정리할 때 지워야 하는 소스
					//. 활
					else if( ( c->Weapon[1].Type>=MODEL_BOW && c->Weapon[1].Type<MODEL_BOW+7 ) 
						|| c->Weapon[1].Type==MODEL_BOW+17 
						|| c->Weapon[1].Type==MODEL_BOW+20 
						|| c->Weapon[1].Type == MODEL_BOW+21
						|| c->Weapon[1].Type == MODEL_BOW+22
						)
						SetAction(&c->Object,PLAYER_STOP_BOW);
					//. 석궁
					else if((c->Weapon[0].Type>=MODEL_BOW+8 && c->Weapon[0].Type<MODEL_BOW+15) ||
							(c->Weapon[0].Type>=MODEL_BOW+16 && c->Weapon[0].Type<MODEL_BOW+17) ||
							(c->Weapon[0].Type>=MODEL_BOW+18 && c->Weapon[0].Type<MODEL_BOW+MAX_ITEM_INDEX) )

						SetAction(&c->Object,PLAYER_STOP_CROSSBOW);
#endif ADD_SOCKET_ITEM				// 정리할 때 지워야 하는 소스
					else
					{
						if (GetBaseClass(c->Class) == CLASS_ELF)
							SetAction(&c->Object, PLAYER_STOP_FEMALE);
						else if (GetBaseClass(c->Class) == CLASS_SUMMONER)
							SetAction(&c->Object, PLAYER_STOP_SUMMONER);
#ifdef PBG_ADD_NEWCHAR_MONK_ANI
						else if (GetBaseClass(c->Class) == CLASS_RAGEFIGHTER)
							SetAction(&c->Object, PLAYER_STOP_RAGEFIGHTER);
#endif //PBG_ADD_NEWCHAR_MONK_ANI
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
     	int Index = TERRAIN_INDEX_REPEAT(( c->PositionX),( c->PositionY));
		if(o->Type==MODEL_MONSTER01+32 && (TerrainWall[Index]&TW_SAFEZONE)==TW_SAFEZONE)//발리
    		SetAction(&c->Object,MONSTER01_APEAR);
		else
	    	SetAction(&c->Object,MONSTER01_STOP1);
	}

	if(rand()%16==0)
	{
#ifndef YDG_ADD_SANTA_MONSTER	// 정리시 삭제해야함
#ifdef KJH_FIX_WOPS_K26908_ELF_MULTI_SKILL_SOUND_IN_SWAMPOFQUIET
		PlayMonsterSound(o);				// 몬스터 정지음
#else // KJH_FIX_WOPS_K26908_ELF_MULTI_SKILL_SOUND_IN_SWAMPOFQUIET
		if (PlayMonsterSound(o) == TRUE);	// 몬스터 정지음
		else
#endif // KJH_FIX_WOPS_K26908_ELF_MULTI_SKILL_SOUND_IN_SWAMPOFQUIET
#endif	// YDG_ADD_SANTA_MONSTER	// 정리시 삭제해야함
	    if(o->Type!=MODEL_PLAYER || (o->SubType>=MODEL_SKELETON1 && o->SubType<=MODEL_SKELETON3))
		{
			if(Models[o->Type].Sounds[0]!=-1)
			{
                int offset = 0;
                if ( o->SubType==9 )
                {
                    offset = 5;
                }
				PlayBuffer ( SOUND_MONSTER+offset+Models[o->Type].Sounds[rand()%2], o );
			}
		}
		//^ 펜릴 사운드 관련(평상시)
		else if(c->Helper.Type == MODEL_HELPER+37)
		{
			if((rand()%3) == 0)
			{
				PlayBuffer(SOUND_FENRIR_IDLE_1+rand()%2, o);
			}
		}
	}
}


///////////////////////////////////////////////////////////////////////////////
// 케릭터 에니메이션 설정(걷거나 뛰기 동작)
///////////////////////////////////////////////////////////////////////////////

void SetPlayerWalk(CHARACTER *c)
{
	// 안전지대이면
	if( c->SafeZone)
	{	
		c->Run = 0;		// 뛰다가 안전지대로 들어가면 걷게 하기 위함
	}
	
    ITEM* pItemBoots = &CharacterMachine->Equipment[EQUIPMENT_BOOTS];
	ITEM* pItemGloves = &CharacterMachine->Equipment[EQUIPMENT_GLOVES];
	int iItemBootsLevel = pItemBoots->Level;
	int iItemGlovesLevel = pItemGloves->Level;

#ifdef YDG_ADD_DOPPELGANGER_MONSTER
	if (c->MonsterIndex >= 534 && c->MonsterIndex <= 539)	// 도플갱어 몬스터는 뛰지 않는다
	{
		iItemBootsLevel = 0;
		iItemGlovesLevel = 0;
		c->Run = 0;
	}
	else
#endif	// YDG_ADD_DOPPELGANGER_MONSTER
	if(!c->SafeZone && c->Run < 40)
	{
		if (GetBaseClass(c->Class) == CLASS_DARK || GetBaseClass(c->Class) == CLASS_DARK_LORD 
#ifdef PBG_ADD_NEWCHAR_MONK
			|| GetBaseClass(c->Class) == CLASS_RAGEFIGHTER
#endif //PBG_ADD_NEWCHAR_MONK
			|| ( ( World != WD_7ATLANSE && !InHellas() 
#ifdef YDG_ADD_MAP_DOPPELGANGER3
			&& World != WD_67DOPPLEGANGER3
#endif	// YDG_ADD_MAP_DOPPELGANGER3
			) && c->BodyPart[BODYPART_BOOTS ].Type!=-1 && c->BodyPart[BODYPART_BOOTS ].Level>=5) 
			|| ( ( World == WD_7ATLANSE ||  InHellas()
#ifdef YDG_ADD_MAP_DOPPELGANGER3
			|| World == WD_67DOPPLEGANGER3
#endif	// YDG_ADD_MAP_DOPPELGANGER3
			) && c->BodyPart[BODYPART_GLOVES].Type!=-1 && c->BodyPart[BODYPART_GLOVES].Level>=5) 
			|| ( ( World != WD_7ATLANSE && !InHellas() 
#ifdef YDG_ADD_MAP_DOPPELGANGER3
			&& World != WD_67DOPPLEGANGER3
#endif	// YDG_ADD_MAP_DOPPELGANGER3
			) && iItemBootsLevel >= 40 )
			|| ( ( World == WD_7ATLANSE ||  InHellas()
#ifdef YDG_ADD_MAP_DOPPELGANGER3
			|| World == WD_67DOPPLEGANGER3
#endif	// YDG_ADD_MAP_DOPPELGANGER3
			) && iItemGlovesLevel >= 40 ) 
			|| c->Helper.Type == MODEL_HELPER+37	//^ 펜릴 달리기 전 5~6타일 정도 걷는 모션 취하기 위해서
			|| c->Object.SubType == MODEL_CURSEDTEMPLE_ALLIED_PLAYER 
			|| c->Object.SubType == MODEL_CURSEDTEMPLE_ILLUSION_PLAYER )
		{
#ifndef CSK_FIX_FENRIR_RUN		// 정리할 때 지워야 하는 소스	
			if(c->Helper.Type == MODEL_HELPER+37)	//^ 펜릴 걷다 달리기 관련
				g_iFenrir_Run++;
			else
#endif //! CSK_FIX_FENRIR_RUN	// 정리할 때 지워야 하는 소스
				c->Run++;
		}
	}
	OBJECT *o = &c->Object;
	if(c->Object.Type==MODEL_PLAYER)
	{
		for(int i=PLAYER_WALK_MALE;i<=PLAYER_WALK_CROSSBOW;i++)
		{
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
			if(GetBaseClass(c->Class) == CLASS_RAGEFIGHTER)			// 모델링사이즈에 따른 조정
				Models[MODEL_PLAYER].Actions[i].PlaySpeed = 0.32f;
			else
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
			Models[MODEL_PLAYER].Actions[i].PlaySpeed = 0.33f;

			if( g_isCharacterBuff(o, eDeBuff_Freeze) )
			{
    			Models[MODEL_PLAYER].Actions[i].PlaySpeed *= 0.5f;
			}
#ifdef CSK_ADD_SKILL_BLOWOFDESTRUCTION
			else if(g_isCharacterBuff(o, eDeBuff_BlowOfDestruction))
			{
				Models[MODEL_PLAYER].Actions[i].PlaySpeed *= 0.33f;
			}
#endif // CSK_ADD_SKILL_BLOWOFDESTRUCTION

		}

		for(int i=PLAYER_RUN;i<=PLAYER_RUN_RIDE_WEAPON;i++)
		{
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
			if(GetBaseClass(c->Class) == CLASS_RAGEFIGHTER)			// 모델링사이즈에 따른 조정
				Models[MODEL_PLAYER].Actions[i].PlaySpeed = 0.28f;
			else
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
			Models[MODEL_PLAYER].Actions[i].PlaySpeed = 0.34f;

#ifdef PBG_ADD_NEWCHAR_MONK_ANI
			Models[MODEL_PLAYER].Actions[PLAYER_RAGE_UNI_RUN].PlaySpeed = 0.34f;
			Models[MODEL_PLAYER].Actions[PLAYER_RAGE_UNI_RUN_ONE_RIGHT].PlaySpeed = 0.34f;
#endif //PBG_ADD_NEWCHAR_MONK_ANI

			if( g_isCharacterBuff(o, eDeBuff_Freeze) )
			{
    			Models[MODEL_PLAYER].Actions[i].PlaySpeed *= 0.5f;
			}
#ifdef CSK_ADD_SKILL_BLOWOFDESTRUCTION
			else if(g_isCharacterBuff(o, eDeBuff_BlowOfDestruction))
			{
				Models[MODEL_PLAYER].Actions[i].PlaySpeed *= 0.33f;
			}
#endif // CSK_ADD_SKILL_BLOWOFDESTRUCTION	
		}

		Models[MODEL_PLAYER].Actions[PLAYER_CHANGE_UP].PlaySpeed = 0.049f;
		Models[MODEL_PLAYER].Actions[PLAYER_RUN_RIDE_HORSE].PlaySpeed = 0.33f;
        Models[MODEL_PLAYER].Actions[PLAYER_DARKLORD_WALK].PlaySpeed = 0.33f;
#ifdef YDG_FIX_ALICE_ANIMATIONS
		Models[MODEL_PLAYER].Actions[PLAYER_WALK_WAND].PlaySpeed = 0.44f;
		Models[MODEL_PLAYER].Actions[PLAYER_RUN_WAND].PlaySpeed = 0.76f;
#endif	// YDG_FIX_ALICE_ANIMATIONS		
		if( g_isCharacterBuff(o, eDeBuff_Freeze ) )
        {
            Models[MODEL_PLAYER].Actions[PLAYER_RUN_RIDE_HORSE].PlaySpeed *= 0.5f;
            Models[MODEL_PLAYER].Actions[PLAYER_DARKLORD_WALK].PlaySpeed *= 0.5f;
        }
#ifdef CSK_ADD_SKILL_BLOWOFDESTRUCTION
		else if(g_isCharacterBuff(o, eDeBuff_BlowOfDestruction))
		{
			Models[MODEL_PLAYER].Actions[PLAYER_RUN_RIDE_HORSE].PlaySpeed *= 0.33f;
            Models[MODEL_PLAYER].Actions[PLAYER_DARKLORD_WALK].PlaySpeed *= 0.33f;
		}
#endif // CSK_ADD_SKILL_BLOWOFDESTRUCTION
			

		//^ 펜릴 캐릭터 에니메이션 걷기 설정
		if(c->Helper.Type == MODEL_HELPER+37 && !c->SafeZone)	// 펜릴
		{
#ifdef CSK_FIX_FENRIR_RUN
			if(c->Run < FENRIR_RUN_DELAY)	// 걷기
#else // CSK_FIX_FENRIR_RUN
			if(g_iFenrir_Run < FENRIR_RUN_DELAY)	// 걷기
#endif // CSK_FIX_FENRIR_RUN
			{
				SetAction_Fenrir_Walk(c, &c->Object);
			}
			else	// 달리기
			{
				SetAction_Fenrir_Run(c, &c->Object);
			}
		}
		else if ( c->Helper.Type==MODEL_HELPER+4 && !c->SafeZone )  // 다크호스
		{
			SetAction(&c->Object,PLAYER_RUN_RIDE_HORSE );
		}
        else if ( c->Weapon[1].Type==MODEL_HELPER+5 && c->SafeZone)	// 다크스피릿 마을에서 정지 상태
        {
            SetAction ( &c->Object, PLAYER_DARKLORD_WALK );
        }
		else if( c->Helper.Type==MODEL_HELPER+2 && !c->SafeZone)    //  유니를 타고있음.
		{
#ifdef PBG_ADD_NEWCHAR_MONK_ANI
			if(GetBaseClass(c->Class)==CLASS_RAGEFIGHTER)
			{				
				if(c->Weapon[0].Type==-1 && c->Weapon[1].Type==-1)
					SetAction(&c->Object,PLAYER_RAGE_UNI_RUN);
				else
					SetAction(&c->Object,PLAYER_RAGE_UNI_RUN_ONE_RIGHT);
			}
			else
			{
#endif //PBG_ADD_NEWCHAR_MONK_ANI
            if(c->Weapon[0].Type==-1 && c->Weapon[1].Type==-1)
				SetAction(&c->Object,PLAYER_RUN_RIDE);
			else
				SetAction(&c->Object,PLAYER_RUN_RIDE_WEAPON);
#ifdef PBG_ADD_NEWCHAR_MONK_ANI
			}
#endif //PBG_ADD_NEWCHAR_MONK_ANI
        }
        else if( c->Helper.Type==MODEL_HELPER+3 && !c->SafeZone )   //  페가시아를 타고있음.
		{
            if ( World==WD_8TARKAN || World==WD_10HEAVEN || g_Direction.m_CKanturu.IsMayaScene() )
            {
				// 애니메이션 튀는거때문에 아예 막아버림
//                if(c->Weapon[0].Type==-1 && c->Weapon[1].Type==-1)
//				    SetAction(&c->Object,PLAYER_FLY_RIDE);
//			    else
//				    SetAction(&c->Object,PLAYER_FLY_RIDE_WEAPON);
            }
            else
            {
#ifdef PBG_ADD_NEWCHAR_MONK_ANI
				if(GetBaseClass(c->Class)==CLASS_RAGEFIGHTER)
				{
					if(c->Weapon[0].Type==-1 && c->Weapon[1].Type==-1)
						SetAction(&c->Object,PLAYER_RAGE_UNI_RUN);
					else
						SetAction(&c->Object,PLAYER_RAGE_UNI_RUN_ONE_RIGHT);
				}
				else
				{
#endif //PBG_ADD_NEWCHAR_MONK_ANI
                if(c->Weapon[0].Type==-1 && c->Weapon[1].Type==-1)
				    SetAction(&c->Object,PLAYER_RUN_RIDE);
			    else
				    SetAction(&c->Object,PLAYER_RUN_RIDE_WEAPON);
#ifdef PBG_ADD_NEWCHAR_MONK_ANI
				}
#endif //PBG_ADD_NEWCHAR_MONK_ANI
            }
		}
		else                                                        //  보통.
		{
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
           	int Index = TERRAIN_INDEX_REPEAT((int)(c->Object.Position[0]/TERRAIN_SCALE),(int)(c->Object.Position[1]/TERRAIN_SCALE));
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
			if( !(c->Object.SubType == MODEL_CURSEDTEMPLE_ALLIED_PLAYER || c->Object.SubType == MODEL_CURSEDTEMPLE_ILLUSION_PLAYER) && 
				!c->SafeZone && c->Wing.Type!=-1)
			{
#ifdef ADD_SOCKET_ITEM
				if( GetEquipedBowType( c ) == BOWTYPE_CROSSBOW )	
#else // ADD_SOCKET_ITEM				// 정리할 때 지워야 하는 소스
				if( (c->Weapon[0].Type>=MODEL_BOW+8 && c->Weapon[0].Type<MODEL_BOW+15) ||
					(c->Weapon[0].Type>=MODEL_BOW+16 && c->Weapon[0].Type<MODEL_BOW+17) || 
					(c->Weapon[0].Type>=MODEL_BOW+18 && c->Weapon[0].Type<MODEL_BOW+MAX_ITEM_INDEX) )
#endif // ADD_SOCKET_ITEM				// 정리할 때 지워야 하는 소스
    				SetAction(&c->Object,PLAYER_FLY_CROSSBOW);
				else
    				SetAction(&c->Object,PLAYER_FLY);
			}
			else if(!c->SafeZone && ( World==WD_7ATLANSE || InHellas() 
#ifdef YDG_ADD_MAP_DOPPELGANGER3
				|| World == WD_67DOPPLEGANGER3
#endif	// YDG_ADD_MAP_DOPPELGANGER3
				) )
			{
				if(c->Run >= 40)
    				SetAction(&c->Object,PLAYER_RUN_SWIM);
				else
    				SetAction(&c->Object,PLAYER_WALK_SWIM);
			}
			else
			{
#ifndef GUILD_WAR_EVENT
                //  무기가 없거나. 블러듴캐슬이 아닌 안전지대 일때
				if( (c->Weapon[0].Type==-1 && c->Weapon[1].Type==-1) || ( c->SafeZone && ( InBloodCastle() == false ) ) )
#else // GUILD_WAR_EVENT
				if( (c->Weapon[0].Type==-1 && c->Weapon[1].Type==-1) || ( c->SafeZone && ( InBloodCastle() == false ) ) )
#endif //GUILD_WAR_EVENT
				{
					if(c->Run>=40)
						SetAction(&c->Object,PLAYER_RUN);
					else
					{
     					if(!IsFemale(c->Class))
							SetAction(&c->Object,PLAYER_WALK_MALE);
						else if (GetBaseClass(c->Class) == CLASS_SUMMONER && InChaosCastle())	// 카오스캐슬 대기시 소환술사 걷는 모습을 남자 애니메이션으로 바꿈.
							SetAction(&c->Object,PLAYER_WALK_MALE);
						else
							SetAction(&c->Object,PLAYER_WALK_FEMALE);
					}
				}
				else
				{
					if(c->Run < 40)
					{
						if(c->Weapon[0].Type>=MODEL_SWORD && c->Weapon[0].Type<MODEL_MACE+MAX_ITEM_INDEX)
						{
							if(!ItemAttribute[c->Weapon[0].Type-MODEL_ITEM].TwoHand)
							{
								SetAction(&c->Object,PLAYER_WALK_SWORD);
							}
							else if ( c->Weapon[0].Type==MODEL_SWORD+21 || c->Weapon[0].Type==MODEL_SWORD+31 || c->Weapon[0].Type==MODEL_SWORD+23 || c->Weapon[0].Type==MODEL_SWORD+25)
							{
								SetAction ( &c->Object, PLAYER_WALK_TWO_HAND_SWORD_TWO );
							}
							else
							{
								SetAction(&c->Object,PLAYER_WALK_TWO_HAND_SWORD);
							}
						}
						// 소환술사 스틱.
						else if (c->Weapon[0].Type >= MODEL_STAFF+14 && c->Weapon[0].Type <= MODEL_STAFF+20)
						{
							::SetAction(&c->Object, PLAYER_WALK_WAND);
						}
						else if(c->Weapon[0].Type>=MODEL_STAFF && c->Weapon[0].Type<MODEL_STAFF+MAX_ITEM_INDEX)
						{
							if(!ItemAttribute[c->Weapon[0].Type-MODEL_ITEM].TwoHand)
								SetAction(&c->Object,PLAYER_WALK_SWORD);
							else
								SetAction(&c->Object,PLAYER_WALK_SCYTHE);
						}
                        //  창.
         				else if(c->Weapon[0].Type==MODEL_SPEAR+1 || c->Weapon[0].Type==MODEL_SPEAR+2+MAX_ITEM_INDEX)
							SetAction(&c->Object,PLAYER_WALK_SPEAR);
                        //  창.
        				else if(c->Weapon[0].Type>=MODEL_SPEAR && c->Weapon[0].Type<MODEL_SPEAR+MAX_ITEM_INDEX)
							SetAction(&c->Object,PLAYER_WALK_SCYTHE);
#ifdef ADD_SOCKET_ITEM
						// 활
						else if( GetEquipedBowType( c ) == BOWTYPE_BOW )
						{
							SetAction( &c->Object, PLAYER_WALK_BOW);
						}
						// 석궁
						else if( GetEquipedBowType( c ) == BOWTYPE_CROSSBOW )
						{
							SetAction( &c->Object, PLAYER_WALK_CROSSBOW);
						}
#else // ADD_SOCKET_ITEM				// 정리할 때 지워야 하는 소스
						//. 활
						else if( ( c->Weapon[1].Type>=MODEL_BOW && c->Weapon[1].Type<MODEL_BOW+7 ) 
							|| c->Weapon[1].Type==MODEL_BOW+17 
							|| c->Weapon[1].Type==MODEL_BOW+20 
							|| c->Weapon[1].Type == MODEL_BOW+21
							|| c->Weapon[1].Type == MODEL_BOW+22
							)
							SetAction(&c->Object,PLAYER_WALK_BOW);
						//. 석궁
      					else if((c->Weapon[0].Type>=MODEL_BOW+8 && c->Weapon[0].Type<MODEL_BOW+15) ||
	       						(c->Weapon[0].Type>=MODEL_BOW+16 && c->Weapon[0].Type<MODEL_BOW+17)|| 
								(c->Weapon[0].Type>=MODEL_BOW+18 && c->Weapon[0].Type<MODEL_BOW+MAX_ITEM_INDEX) )
							SetAction(&c->Object,PLAYER_WALK_CROSSBOW);
#endif // ADD_SOCKET_ITEM				// 정리할 때 지워야 하는 소스
						else
						{
         					if(!IsFemale(c->Class))
								SetAction(&c->Object,PLAYER_WALK_MALE);
							else
								SetAction(&c->Object,PLAYER_WALK_FEMALE);
						}
					}
					else
					{
						if(c->Weapon[0].Type>=MODEL_SWORD && c->Weapon[0].Type<MODEL_MACE+MAX_ITEM_INDEX)
						{
							if(c->Weapon[1].Type>=MODEL_SWORD && c->Weapon[1].Type<MODEL_MACE+MAX_ITEM_INDEX)
							{
#ifdef PBG_ADD_NEWCHAR_MONK_ANI
								if(GetBaseClass(c->Class) == CLASS_RAGEFIGHTER)
								{
									SetAction(&c->Object,PLAYER_RUN);
								}
								else
#endif //PBG_ADD_NEWCHAR_MONK_ANI
								SetAction(&c->Object,PLAYER_RUN_TWO_SWORD);
							}
							else if(!ItemAttribute[c->Weapon[0].Type-MODEL_ITEM].TwoHand)
							{
								SetAction(&c->Object,PLAYER_RUN_SWORD);
							}
							else if ( c->Weapon[0].Type==MODEL_SWORD+21 || c->Weapon[0].Type==MODEL_SWORD+31 || c->Weapon[0].Type==MODEL_SWORD+23 || c->Weapon[0].Type==MODEL_SWORD+25)
							{
								SetAction ( &c->Object, PLAYER_RUN_TWO_HAND_SWORD_TWO );
							}
							else
							{
								SetAction(&c->Object,PLAYER_RUN_TWO_HAND_SWORD);
							}
						}
						// 소환술사 스틱.
						else if (c->Weapon[0].Type >= MODEL_STAFF+14 && c->Weapon[0].Type <= MODEL_STAFF+20)
						{
							::SetAction(&c->Object, PLAYER_RUN_WAND);
						}
						else if(c->Weapon[0].Type>=MODEL_STAFF && c->Weapon[0].Type<MODEL_STAFF+MAX_ITEM_INDEX)
						{
							if(!ItemAttribute[c->Weapon[0].Type-MODEL_ITEM].TwoHand)
								SetAction(&c->Object,PLAYER_RUN_SWORD);
							else
								SetAction(&c->Object,PLAYER_RUN_SPEAR);
						}
                        //  창.
        				else if(c->Weapon[0].Type>=MODEL_SPEAR && c->Weapon[0].Type<MODEL_SPEAR+MAX_ITEM_INDEX)
						{
							SetAction(&c->Object,PLAYER_RUN_SPEAR);
						}
#ifdef ADD_SOCKET_ITEM
						// 활
						else if( GetEquipedBowType( c ) == BOWTYPE_BOW )
						{
							SetAction( &c->Object, PLAYER_RUN_BOW );
						}
						// 석궁
						else if( GetEquipedBowType( c ) == BOWTYPE_CROSSBOW )
						{
							SetAction( &c->Object, PLAYER_RUN_CROSSBOW );
						}
#else // ADD_SOCKET_ITEM				// 정리할 때 지워야 하는 소스
						else if((c->Weapon[1].Type>=MODEL_BOW && c->Weapon[1].Type<MODEL_BOW+7 ) || c->Weapon[1].Type==MODEL_BOW+17 
							|| c->Weapon[1].Type==MODEL_BOW+20 
							|| c->Weapon[1].Type == MODEL_BOW+21
							|| c->Weapon[1].Type == MODEL_BOW+22
							)
						{
							SetAction(&c->Object,PLAYER_RUN_BOW);
						}
      					else if((c->Weapon[0].Type>=MODEL_BOW+8 && c->Weapon[0].Type<MODEL_BOW+15) ||
	    						(c->Weapon[0].Type>=MODEL_BOW+16 && c->Weapon[0].Type<MODEL_BOW+17)||
								(c->Weapon[0].Type>=MODEL_BOW+18 && c->Weapon[0].Type<MODEL_BOW+MAX_ITEM_INDEX) )
						{
							SetAction(&c->Object,PLAYER_RUN_CROSSBOW);
						}
#endif // ADD_SOCKET_ITEM				// 정리할 때 지워야 하는 소스
						else
						{
							SetAction(&c->Object,PLAYER_RUN);
						}
					}
				}
			}
		}
    }
	else
	{
		SetAction(&c->Object,MONSTER01_WALK);
	}
#ifdef KJH_FIX_WOPS_K26908_ELF_MULTI_SKILL_SOUND_IN_SWAMPOFQUIET
	PlayMonsterSound(o);				// 몬스터 이동음
#else // KJH_FIX_WOPS_K26908_ELF_MULTI_SKILL_SOUND_IN_SWAMPOFQUIET
	if (PlayMonsterSound(o) == TRUE);	// 몬스터 이동음
	else
#endif // KJH_FIX_WOPS_K26908_ELF_MULTI_SKILL_SOUND_IN_SWAMPOFQUIET
    if(o->Type == MODEL_MONSTER01+27)
     	PlayBuffer(SOUND_BONE2,o);
    else if ( GetBaseClass(c->Class)==CLASS_DARK_LORD && c->Helper.Type==MODEL_HELPER+4 && !c->SafeZone )
    {
		PlayBuffer ( SOUND_RUN_DARK_HORSE_1+rand()%3, o );
    }
	//^ 펜릴 사운드 관련(달리기, 걷기)
	else if(c->Helper.Type == MODEL_HELPER+37 && !c->SafeZone	// 달리기
			&& (c->Object.CurrentAction >= PLAYER_FENRIR_RUN && c->Object.CurrentAction <= PLAYER_FENRIR_RUN_ONE_LEFT_ELF))
	{
		PlayBuffer ( SOUND_FENRIR_RUN_1+rand()%3, o );
	}
	else if(c->Helper.Type == MODEL_HELPER+37 && !c->SafeZone	// 걷기
			&& (c->Object.CurrentAction >= PLAYER_FENRIR_WALK && c->Object.CurrentAction <= PLAYER_FENRIR_WALK_ONE_LEFT))
	{
		PlayBuffer ( SOUND_FENRIR_RUN_1+rand()%2, o );	
	}
#ifdef PBG_ADD_NEWCHAR_MONK_ANI
	else if(c->Helper.Type == MODEL_HELPER+37 && !c->SafeZone
			&& (c->Object.CurrentAction >= PLAYER_RAGE_FENRIR_RUN && c->Object.CurrentAction <= PLAYER_RAGE_FENRIR_RUN_ONE_LEFT))
	{
		PlayBuffer ( SOUND_FENRIR_RUN_1+rand()%3, o );
	}
	else if(c->Helper.Type == MODEL_HELPER+37 && !c->SafeZone
			&& (c->Object.CurrentAction >= PLAYER_RAGE_FENRIR_WALK && c->Object.CurrentAction <= PLAYER_RAGE_FENRIR_WALK_TWO_SWORD))
	{
		PlayBuffer ( SOUND_FENRIR_RUN_1+rand()%2, o );	
	}
#endif //PBG_ADD_NEWCHAR_MONK_ANI
	else if((c==Hero&&rand()%64==0) || (c!=Hero&&rand()%16==0))
	{
		if(o->Type!=MODEL_PLAYER || (o->SubType>=MODEL_SKELETON1 && o->SubType<=MODEL_SKELETON3))
		{
			if(o->SubType>=MODEL_SKELETON1 && o->SubType<=MODEL_SKELETON3)
     			PlayBuffer(SOUND_BONE1,o);
			if(Models[o->Type].Sounds[0]!=-1)
            {
                int offset = 0;
                if ( o->SubType==9 )
                {
                    offset = 5;
                }
                PlayBuffer(SOUND_MONSTER+offset+Models[o->Type].Sounds[rand()%2],o);
            }
		}
	}
}

//extern bool EnableEdit;
extern int CurrentSkill;
int AttackHand = 0;

void SetAttackSpeed()
{

	float AttackSpeed1 = CharacterAttribute->AttackSpeed * 0.004f;	// 기사, 일반공격
	float MagicSpeed1  = CharacterAttribute->MagicSpeed * 0.004f;	// 요정
	float MagicSpeed2  = CharacterAttribute->MagicSpeed * 0.002f;	// 법사

 	Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_FIST].PlaySpeed = 0.6f + AttackSpeed1;

	for(int i=PLAYER_ATTACK_SWORD_RIGHT1; i<=PLAYER_ATTACK_RIDE_CROSSBOW; i++)
	{
   		Models[MODEL_PLAYER].Actions[i].PlaySpeed = 0.25f + AttackSpeed1;
	}

    Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_SKILL_SWORD1].PlaySpeed = 0.30f + AttackSpeed1; // 내려찍기
    Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_SKILL_SWORD2].PlaySpeed = 0.30f + AttackSpeed1; // 찌르기
    Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_SKILL_SWORD3].PlaySpeed = 0.27f + AttackSpeed1; // 올려치기
    Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_SKILL_SWORD4].PlaySpeed = 0.30f + AttackSpeed1; // 돌려치기
    Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_SKILL_SWORD5].PlaySpeed = 0.24f + AttackSpeed1; // 베기
    Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_SKILL_WHEEL ].PlaySpeed = 0.24f + AttackSpeed1; // 회오리베기
#ifdef YDG_ADD_SKILL_RIDING_ANIMATIONS
    Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_SKILL_WHEEL_UNI   ].PlaySpeed = 0.24f + AttackSpeed1;// 회오리베기
    Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_SKILL_WHEEL_DINO  ].PlaySpeed = 0.24f + AttackSpeed1;// 회오리베기
    Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_SKILL_WHEEL_FENRIR].PlaySpeed = 0.24f + AttackSpeed1;// 회오리베기
#endif	// YDG_ADD_SKILL_RIDING_ANIMATIONS

	Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_ONETOONE].PlaySpeed = 0.25f + AttackSpeed1;	// 블로우
    Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_SKILL_SPEAR].PlaySpeed = 0.30f + AttackSpeed1;	// 창찌르기

	// 탈 것 탄 상태
	Models[MODEL_PLAYER].Actions[PLAYER_SKILL_RIDER].PlaySpeed      = 0.3f + AttackSpeed1;	// 스턴, 스턴제거, 투명, 투명해제, 마나증가, 보조마법 일시제거, 레이드슛	
	Models[MODEL_PLAYER].Actions[PLAYER_SKILL_RIDER_FLY].PlaySpeed  = 0.3f + AttackSpeed1;	// 스턴, 스턴제거, 투명, 투명해제, 마나증가, 보조마법 일시제거, 레이드슛

	Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_TWO_HAND_SWORD_TWO].PlaySpeed = 0.25f + AttackSpeed1;	// 파워슬래쉬

	for(int i=PLAYER_ATTACK_BOW;i<=PLAYER_ATTACK_FLY_CROSSBOW;i++)
		Models[MODEL_PLAYER].Actions[i].PlaySpeed = 0.30f + AttackSpeed1;
	for(int i=PLAYER_ATTACK_RIDE_BOW;i<=PLAYER_ATTACK_RIDE_CROSSBOW;i++)
		Models[MODEL_PLAYER].Actions[i].PlaySpeed = 0.30f + AttackSpeed1;

	Models[MODEL_PLAYER].Actions[PLAYER_SKILL_ELF1].PlaySpeed = 0.25f + MagicSpeed1;

	for(int i=PLAYER_SKILL_HAND1; i<=PLAYER_SKILL_WEAPON2; i++)
	    Models[MODEL_PLAYER].Actions[i].PlaySpeed = 0.29f + MagicSpeed2;

	Models[MODEL_PLAYER].Actions[PLAYER_SKILL_TELEPORT].PlaySpeed = 0.30f + MagicSpeed2; // 순간이동
	Models[MODEL_PLAYER].Actions[PLAYER_SKILL_FLASH   ].PlaySpeed = 0.40f + MagicSpeed2; // 아쿠아플래쉬
	Models[MODEL_PLAYER].Actions[PLAYER_SKILL_INFERNO ].PlaySpeed = 0.60f + MagicSpeed2; // 인페르노
	Models[MODEL_PLAYER].Actions[PLAYER_SKILL_HELL    ].PlaySpeed = 0.50f + MagicSpeed2; // 헬파이어
	Models[MODEL_PLAYER].Actions[PLAYER_RIDE_SKILL    ].PlaySpeed = 0.30f + MagicSpeed2; // 탈 것 탄 상태에서 쓰는 스킬

	Models[MODEL_PLAYER].Actions[PLAYER_SKILL_HELL_BEGIN].PlaySpeed			= 0.50f + MagicSpeed2;
	Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_STRIKE].PlaySpeed            = 0.25f + AttackSpeed1;
	Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_RIDE_STRIKE].PlaySpeed       = 0.2f + AttackSpeed1;
	Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_RIDE_HORSE_SWORD].PlaySpeed  = 0.25f + AttackSpeed1;
	Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_RIDE_ATTACK_FLASH].PlaySpeed = 0.40f + MagicSpeed2;
	Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_RIDE_ATTACK_MAGIC].PlaySpeed = 0.3f + MagicSpeed2;

	Models[MODEL_PLAYER].Actions[PLAYER_FENRIR_ATTACK].PlaySpeed				= 0.25f + AttackSpeed1;
	Models[MODEL_PLAYER].Actions[PLAYER_FENRIR_ATTACK_DARKLORD_STRIKE].PlaySpeed = 0.2f + AttackSpeed1;
	Models[MODEL_PLAYER].Actions[PLAYER_FENRIR_ATTACK_DARKLORD_SWORD].PlaySpeed	= 0.25f + AttackSpeed1;
	Models[MODEL_PLAYER].Actions[PLAYER_FENRIR_ATTACK_DARKLORD_FLASH].PlaySpeed	= 0.40f + MagicSpeed2;
	Models[MODEL_PLAYER].Actions[PLAYER_FENRIR_ATTACK_TWO_SWORD].PlaySpeed		= 0.25f + AttackSpeed1;
	Models[MODEL_PLAYER].Actions[PLAYER_FENRIR_ATTACK_MAGIC].PlaySpeed			= 0.37f + MagicSpeed2;
	Models[MODEL_PLAYER].Actions[PLAYER_FENRIR_ATTACK_CROSSBOW].PlaySpeed		= 0.30f + AttackSpeed1;
	Models[MODEL_PLAYER].Actions[PLAYER_FENRIR_ATTACK_SPEAR].PlaySpeed			= 0.25f + AttackSpeed1;
	Models[MODEL_PLAYER].Actions[PLAYER_FENRIR_ATTACK_ONE_SWORD].PlaySpeed		= 0.25f + AttackSpeed1;
	Models[MODEL_PLAYER].Actions[PLAYER_FENRIR_ATTACK_BOW].PlaySpeed			= 0.30f + AttackSpeed1;
	
	for ( int i=PLAYER_ATTACK_BOW_UP; i<=PLAYER_ATTACK_RIDE_CROSSBOW_UP; ++i )
	{
	    Models[MODEL_PLAYER].Actions[i].PlaySpeed = 0.30f + AttackSpeed1;
	}

	Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_ONE_FLASH].PlaySpeed = 0.4f + AttackSpeed1;	// 기마검 일섬공격
	Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_RUSH].PlaySpeed		= 0.3f + AttackSpeed1;			// 기사 돌격 스킬.
	Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_DEATH_CANNON].PlaySpeed = 0.2f + AttackSpeed1;
	
	// 슬립
	Models[MODEL_PLAYER].Actions[PLAYER_SKILL_SLEEP].PlaySpeed			= 0.3f + MagicSpeed2;
	Models[MODEL_PLAYER].Actions[PLAYER_SKILL_SLEEP_UNI].PlaySpeed		= 0.3f + MagicSpeed2;
	Models[MODEL_PLAYER].Actions[PLAYER_SKILL_SLEEP_DINO].PlaySpeed		= 0.3f + MagicSpeed2;
	Models[MODEL_PLAYER].Actions[PLAYER_SKILL_SLEEP_FENRIR].PlaySpeed	= 0.3f + MagicSpeed2;

	// 라이트닝 오브
	Models[MODEL_PLAYER].Actions[PLAYER_SKILL_LIGHTNING_ORB].PlaySpeed			= 0.4f + MagicSpeed2;
	Models[MODEL_PLAYER].Actions[PLAYER_SKILL_LIGHTNING_ORB_UNI].PlaySpeed		= 0.25f + MagicSpeed2;
	Models[MODEL_PLAYER].Actions[PLAYER_SKILL_LIGHTNING_ORB_DINO].PlaySpeed		= 0.25f + MagicSpeed2;
	Models[MODEL_PLAYER].Actions[PLAYER_SKILL_LIGHTNING_ORB_FENRIR].PlaySpeed	= 0.25f + MagicSpeed2;

	// 체인라이트닝
	Models[MODEL_PLAYER].Actions[PLAYER_SKILL_CHAIN_LIGHTNING].PlaySpeed		= 0.25f + MagicSpeed2;
	Models[MODEL_PLAYER].Actions[PLAYER_SKILL_CHAIN_LIGHTNING_UNI].PlaySpeed	= 0.15f + MagicSpeed2;
	Models[MODEL_PLAYER].Actions[PLAYER_SKILL_CHAIN_LIGHTNING_DINO].PlaySpeed	= 0.15f + MagicSpeed2;
	Models[MODEL_PLAYER].Actions[PLAYER_SKILL_CHAIN_LIGHTNING_FENRIR].PlaySpeed = 0.15f + MagicSpeed2;

	// 드레인라이프
	Models[MODEL_PLAYER].Actions[PLAYER_SKILL_DRAIN_LIFE].PlaySpeed			= 0.25f + MagicSpeed2;
	Models[MODEL_PLAYER].Actions[PLAYER_SKILL_DRAIN_LIFE_UNI].PlaySpeed		= 0.25f + MagicSpeed2;
	Models[MODEL_PLAYER].Actions[PLAYER_SKILL_DRAIN_LIFE_DINO].PlaySpeed	= 0.25f + MagicSpeed2;
	Models[MODEL_PLAYER].Actions[PLAYER_SKILL_DRAIN_LIFE_FENRIR].PlaySpeed	= 0.25f + MagicSpeed2;

#ifdef YDG_ADD_SKILL_GIGANTIC_STORM
	// 기간틱스톰
	Models[MODEL_PLAYER].Actions[PLAYER_SKILL_GIGANTICSTORM].PlaySpeed = 0.55f + MagicSpeed1;
	// 프레임스트라이크
	Models[MODEL_PLAYER].Actions[PLAYER_SKILL_FLAMESTRIKE].PlaySpeed = 0.69f + MagicSpeed2;
#endif // YDG_ADD_SKILL_GIGANTIC_STORM
#ifdef YDG_ADD_SKILL_RIDING_ANIMATIONS
	Models[MODEL_PLAYER].Actions[PLAYER_SKILL_GIGANTICSTORM_UNI].PlaySpeed		= 0.55f + MagicSpeed1;
	Models[MODEL_PLAYER].Actions[PLAYER_SKILL_GIGANTICSTORM_DINO].PlaySpeed		= 0.55f + MagicSpeed1;
	Models[MODEL_PLAYER].Actions[PLAYER_SKILL_GIGANTICSTORM_FENRIR].PlaySpeed	= 0.55f + MagicSpeed1;
#endif	// YDG_ADD_SKILL_RIDING_ANIMATIONS
#ifdef YDG_ADD_SKILL_LIGHTNING_SHOCK
	// 라이트닝쇼크
	Models[MODEL_PLAYER].Actions[PLAYER_SKILL_LIGHTNING_SHOCK].PlaySpeed = 0.35f + MagicSpeed2;
#endif	// YDG_ADD_SKILL_LIGHTNING_SHOCK

	// 셔먼
	Models[MODEL_PLAYER].Actions[PLAYER_SKILL_SUMMON].PlaySpeed			= 0.25f;
	Models[MODEL_PLAYER].Actions[PLAYER_SKILL_SUMMON_UNI].PlaySpeed		= 0.25f;
	Models[MODEL_PLAYER].Actions[PLAYER_SKILL_SUMMON_DINO].PlaySpeed	= 0.25f;
	Models[MODEL_PLAYER].Actions[PLAYER_SKILL_SUMMON_FENRIR].PlaySpeed	= 0.25f;
	
#ifdef CSK_ADD_SKILL_BLOWOFDESTRUCTION
	// 파괴의 일격
	Models[MODEL_PLAYER].Actions[PLAYER_SKILL_BLOW_OF_DESTRUCTION].PlaySpeed = 0.3f;
#endif // CSK_ADD_SKILL_BLOWOFDESTRUCTION

#ifdef PJH_SEASON4_SPRITE_NEW_SKILL_RECOVER
	// 회복
	Models[MODEL_PLAYER].Actions[PLAYER_RECOVER_SKILL].PlaySpeed = 0.33f;
#endif //PJH_SEASON4_SPRITE_NEW_SKILL_RECOVER
#ifdef KJH_ADD_SKILL_SWELL_OF_MAGICPOWER		// 마력증대
	// 마력증대
	Models[MODEL_PLAYER].Actions[PLAYER_SKILL_SWELL_OF_MP].PlaySpeed = 0.2f;		
#endif // KJH_ADD_SKILL_SWELL_OF_MAGICPOWER

	Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_SKILL_FURY_STRIKE].PlaySpeed = 0.38f;		// 분노의일격
    Models[MODEL_PLAYER].Actions[PLAYER_SKILL_VITALITY].PlaySpeed = 0.34f;					// 스웰라이프
	Models[MODEL_PLAYER].Actions[PLAYER_SKILL_HELL_START].PlaySpeed = 0.30f;
	Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_TELEPORT].PlaySpeed = 0.28f;
	Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_RIDE_TELEPORT].PlaySpeed = 0.3f;
	Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_DARKHORSE].PlaySpeed = 0.3f;
	Models[MODEL_PLAYER].Actions[PLAYER_FENRIR_ATTACK_DARKLORD_TELEPORT].PlaySpeed = 0.3f;
	Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_REMOVAL].PlaySpeed	= 0.28f;

#ifdef WORLDCUP_ADD
	Models[MODEL_PLAYER].Actions[PLAYER_KOREA_HANDCLAP].PlaySpeed = 0.3f;
	Models[MODEL_PLAYER].Actions[PLAYER_POINT_DANCE].PlaySpeed = 0.4f;
#endif // WORLDCUP_ADD
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
	float RageAttackSpeed = CharacterAttribute->AttackSpeed * 0.002f;

	Models[MODEL_PLAYER].Actions[PLAYER_SKILL_THRUST].PlaySpeed = 0.4f + RageAttackSpeed;
	Models[MODEL_PLAYER].Actions[PLAYER_SKILL_STAMP].PlaySpeed = 0.4f + RageAttackSpeed;
	Models[MODEL_PLAYER].Actions[PLAYER_SKILL_GIANTSWING].PlaySpeed = 0.4f + RageAttackSpeed;
	Models[MODEL_PLAYER].Actions[PLAYER_SKILL_DARKSIDE_READY].PlaySpeed = 0.3f + RageAttackSpeed;
	Models[MODEL_PLAYER].Actions[PLAYER_SKILL_DARKSIDE_ATTACK].PlaySpeed = 0.3f + RageAttackSpeed;
	Models[MODEL_PLAYER].Actions[PLAYER_SKILL_DRAGONKICK].PlaySpeed = 0.4f + RageAttackSpeed;
	Models[MODEL_PLAYER].Actions[PLAYER_SKILL_DRAGONLORE].PlaySpeed = 0.3f+RageAttackSpeed;
	Models[MODEL_PLAYER].Actions[PLAYER_SKILL_ATT_UP_OURFORCES].PlaySpeed = 0.35f;
	Models[MODEL_PLAYER].Actions[PLAYER_SKILL_HP_UP_OURFORCES].PlaySpeed = 0.35f;
	Models[MODEL_PLAYER].Actions[PLAYER_RAGE_FENRIR_ATTACK_RIGHT].PlaySpeed	= 0.25f + RageAttackSpeed;
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
}


void SetPlayerHighBowAttack ( CHARACTER* c )
{
	OBJECT *o = &c->Object;
	if(o->Type==MODEL_PLAYER)
	{
		SetAttackSpeed();
		if( ( c->Helper.Type==MODEL_HELPER+2 || c->Helper.Type==MODEL_HELPER+3 ) && !c->SafeZone)
		{
#ifdef ADD_SOCKET_ITEM
			// 활
			if( GetEquipedBowType( c ) == BOWTYPE_BOW )
			{
				SetAction( &c->Object, PLAYER_ATTACK_RIDE_BOW_UP );
			}
			// 석궁
			else if( GetEquipedBowType( c ) == BOWTYPE_CROSSBOW )
			{
				SetAction( &c->Object, PLAYER_ATTACK_RIDE_CROSSBOW_UP );
			}
#else // ADD_SOCKET_ITEM				// 정리할 때 지워야 하는 소스
			if( (c->Weapon[1].Type>=MODEL_BOW && c->Weapon[1].Type<MODEL_BOW+7 ) || c->Weapon[1].Type==MODEL_BOW+17
				|| c->Weapon[1].Type==MODEL_BOW+20 
				|| c->Weapon[1].Type==MODEL_BOW+21
				|| c->Weapon[1].Type==MODEL_BOW+22
				)
			{
				SetAction ( &c->Object, PLAYER_ATTACK_RIDE_BOW_UP );
			}
			//. 석궁
			else if((c->Weapon[0].Type>=MODEL_BOW+8 && c->Weapon[0].Type<MODEL_BOW+15) ||
					(c->Weapon[0].Type>=MODEL_BOW+16 && c->Weapon[0].Type<MODEL_BOW+17)|| 
				    (c->Weapon[0].Type>=MODEL_BOW+18 && c->Weapon[0].Type<MODEL_BOW+MAX_ITEM_INDEX) )
			{
				SetAction ( &c->Object, PLAYER_ATTACK_RIDE_CROSSBOW_UP );
			}
#endif // ADD_SOCKET_ITEM				// 정리할 때 지워야 하는 소스
		}
		else
		{
#ifdef ADD_SOCKET_ITEM
			// 활
			if( GetEquipedBowType( c ) == BOWTYPE_BOW )
			{
				if ( c->Wing.Type!=-1 )
		         	SetAction( &c->Object, PLAYER_ATTACK_FLY_BOW_UP );
				else
				    SetAction( &c->Object, PLAYER_ATTACK_BOW_UP );
			}
			// 석궁
			else if( GetEquipedBowType( c ) == BOWTYPE_CROSSBOW )
			{
				if ( c->Wing.Type!=-1 )
		         	SetAction( &c->Object, PLAYER_ATTACK_FLY_CROSSBOW_UP );
				else
      				SetAction( &c->Object, PLAYER_ATTACK_CROSSBOW_UP );
			}
#else // ADD_SOCKET_ITEM				// 정리할 때 지워야 하는 소스
			if( ( c->Weapon[1].Type>=MODEL_BOW && c->Weapon[1].Type<MODEL_BOW+7  ) || c->Weapon[1].Type==MODEL_BOW+17
				|| c->Weapon[1].Type==MODEL_BOW+20 
				|| c->Weapon[1].Type==MODEL_BOW+21
				|| c->Weapon[1].Type==MODEL_BOW+22
				)
			{
        		if ( c->Wing.Type!=-1 )
		         	SetAction ( &c->Object, PLAYER_ATTACK_FLY_BOW_UP );
				else
				    SetAction ( &c->Object, PLAYER_ATTACK_BOW_UP );
			}
			//. 석궁
			else if((c->Weapon[0].Type>=MODEL_BOW+8 && c->Weapon[0].Type<MODEL_BOW+15) ||
					(c->Weapon[0].Type>=MODEL_BOW+16 && c->Weapon[0].Type<MODEL_BOW+17)||
					(c->Weapon[0].Type>=MODEL_BOW+18 && c->Weapon[0].Type<MODEL_BOW+MAX_ITEM_INDEX) )
			{
        		if ( c->Wing.Type!=-1 )
		         	SetAction ( &c->Object, PLAYER_ATTACK_FLY_CROSSBOW_UP );
				else
      				SetAction ( &c->Object, PLAYER_ATTACK_CROSSBOW_UP );
			}
#endif // ADD_SOCKET_ITEM				// 정리할 때 지워야 하는 소스
		}
	}
	c->SwordCount++;
}

void SetPlayerAttack(CHARACTER *c)
{
	OBJECT *o = &c->Object;
	if(o->Type==MODEL_PLAYER)
	{
		SetAttackSpeed();

		//^ 펜릴 캐릭터 에니메이션 설정(공격 관련)
		if(c->Helper.Type == MODEL_HELPER+37 && !c->SafeZone)
		{
			if(c->Weapon[0].Type>=MODEL_SPEAR && c->Weapon[0].Type<MODEL_SPEAR+5)
				SetAction(&c->Object, PLAYER_FENRIR_ATTACK_SPEAR);	// 창공격
#ifdef ADD_SOCKET_ITEM
			// 활
			else if( GetEquipedBowType( c ) == BOWTYPE_BOW )
			{
				SetAction( &c->Object, PLAYER_FENRIR_ATTACK_BOW );		// 활공격 
			}
			// 석궁
			else if( GetEquipedBowType( c ) == BOWTYPE_CROSSBOW )
			{
				SetAction( &c->Object, PLAYER_FENRIR_ATTACK_CROSSBOW );	//석궁공격
			}
#else // ADD_SOCKET_ITEM				// 정리할 때 지워야 하는 소스
			else if( (c->Weapon[1].Type>=MODEL_BOW && c->Weapon[1].Type<MODEL_BOW+7 ) || c->Weapon[1].Type==MODEL_BOW+17
				|| c->Weapon[1].Type==MODEL_BOW+20 
				|| c->Weapon[1].Type == MODEL_BOW+21
				|| c->Weapon[1].Type == MODEL_BOW+22
				)
			{
				SetAction(&c->Object,PLAYER_FENRIR_ATTACK_BOW);	// 활공격 
			}
			//. 석궁
			else if((c->Weapon[0].Type>=MODEL_BOW+8 && c->Weapon[0].Type<MODEL_BOW+15) ||
					(c->Weapon[0].Type>=MODEL_BOW+16 && c->Weapon[0].Type<MODEL_BOW+17)|| 
				    (c->Weapon[0].Type>=MODEL_BOW+18 && c->Weapon[0].Type<MODEL_BOW+MAX_ITEM_INDEX) )
				SetAction(&c->Object,PLAYER_FENRIR_ATTACK_CROSSBOW);	//석궁공격
#endif // ADD_SOCKET_ITEM				// 정리할 때 지워야 하는 소스
			else
			{
				if(c->Weapon[0].Type != -1 && c->Weapon[1].Type != -1)	// 양손무기
					SetAction(&c->Object, PLAYER_FENRIR_ATTACK_TWO_SWORD);
				else if(c->Weapon[0].Type != -1 && c->Weapon[1].Type == -1) // 왼손 무기
					SetAction(&c->Object, PLAYER_FENRIR_ATTACK_ONE_SWORD);
#ifdef PBG_ADD_NEWCHAR_MONK_ANI
				else if(c->Weapon[0].Type == -1 && c->Weapon[1].Type != -1 && (GetBaseClass(c->Class) == CLASS_RAGEFIGHTER))
					SetAction(&c->Object, PLAYER_RAGE_FENRIR_ATTACK_RIGHT);
#endif //PBG_ADD_NEWCHAR_MONK_ANI
				else if(c->Weapon[0].Type == -1 && c->Weapon[1].Type != -1) // 오른손 무기
					SetAction(&c->Object, PLAYER_FENRIR_ATTACK_ONE_SWORD);
				else	// 맨손
					SetAction(&c->Object, PLAYER_FENRIR_ATTACK);
			}

			if(GetBaseClass(c->Class) == CLASS_DARK_LORD)
			{
				SetAction( &c->Object, PLAYER_FENRIR_ATTACK_DARKLORD_SWORD);
			}
		}
		else if ( c->Helper.Type==MODEL_HELPER+4 && !c->SafeZone )
		{
			SetAction ( &c->Object, PLAYER_ATTACK_RIDE_HORSE_SWORD );
		}
		else if( ( c->Helper.Type==MODEL_HELPER+2 || c->Helper.Type==MODEL_HELPER+3 ) && !c->SafeZone)
		{
			if(c->Weapon[0].Type>=MODEL_SPEAR && c->Weapon[0].Type<MODEL_SPEAR+5)
				SetAction(&c->Object,PLAYER_ATTACK_RIDE_SPEAR);
			else if(c->Weapon[0].Type>=MODEL_SPEAR+5 && c->Weapon[0].Type<MODEL_SPEAR+MAX_ITEM_INDEX)
				SetAction(&c->Object,PLAYER_ATTACK_RIDE_SCYTHE);
#ifdef ADD_SOCKET_ITEM
			// 활
			else if( GetEquipedBowType( c ) == BOWTYPE_BOW )
			{
				SetAction( &c->Object, PLAYER_ATTACK_RIDE_BOW );
			}
			// 석궁
			else if( GetEquipedBowType( c ) == BOWTYPE_CROSSBOW )
			{
				SetAction( &c->Object, PLAYER_ATTACK_RIDE_CROSSBOW );
			}
#else // ADD_SOCKET_ITEM				// 정리할 때 지워야 하는 소스
			//. 활
			else if( (c->Weapon[1].Type>=MODEL_BOW && c->Weapon[1].Type<MODEL_BOW+7 ) || c->Weapon[1].Type==MODEL_BOW+17
				|| c->Weapon[1].Type==MODEL_BOW+20 
				|| c->Weapon[1].Type == MODEL_BOW+21
				|| c->Weapon[1].Type == MODEL_BOW+22
				)
			{
				SetAction(&c->Object,PLAYER_ATTACK_RIDE_BOW);
			}
			//. 석궁
			else if((c->Weapon[0].Type>=MODEL_BOW+8 && c->Weapon[0].Type<MODEL_BOW+15) ||
					(c->Weapon[0].Type>=MODEL_BOW+16 && c->Weapon[0].Type<MODEL_BOW+17)|| 
				    (c->Weapon[0].Type>=MODEL_BOW+18 && c->Weapon[0].Type<MODEL_BOW+MAX_ITEM_INDEX) )
				SetAction(&c->Object,PLAYER_ATTACK_RIDE_CROSSBOW);
#endif // ADD_SOCKET_ITEM				// 정리할 때 지워야 하는 소스
			else
			{
				if(c->Weapon[0].Type == -1)
				{
#ifdef PBG_ADD_NEWCHAR_MONK_ANI
					if(GetBaseClass(c->Class)==CLASS_RAGEFIGHTER)
						SetAction(&c->Object,PLAYER_RAGE_UNI_ATTACK);
					else
#endif //PBG_ADD_NEWCHAR_MONK_ANI
					SetAction(&c->Object,PLAYER_ATTACK_RIDE_SWORD);
				}
				else
				{
#ifdef PBG_ADD_NEWCHAR_MONK_ANI
					if(GetBaseClass(c->Class)==CLASS_RAGEFIGHTER)
					{
						if(!ItemAttribute[c->Weapon[0].Type-MODEL_ITEM].TwoHand)
							SetAction(&c->Object,PLAYER_RAGE_UNI_ATTACK);
						else
							SetAction(&c->Object,PLAYER_RAGE_UNI_ATTACK_ONE_RIGHT);
					}
					else
					{
#endif //PBG_ADD_NEWCHAR_MONK_ANI
					if(!ItemAttribute[c->Weapon[0].Type-MODEL_ITEM].TwoHand)
						SetAction(&c->Object,PLAYER_ATTACK_RIDE_SWORD);
					else
						SetAction(&c->Object,PLAYER_ATTACK_RIDE_TWO_HAND_SWORD);
#ifdef PBG_ADD_NEWCHAR_MONK_ANI
					}
#endif //PBG_ADD_NEWCHAR_MONK_ANI
				}
			}
		}
		else
		{
			if(c->Weapon[0].Type==-1 && c->Weapon[1].Type==-1)
			{
   				SetAction(&c->Object,PLAYER_ATTACK_FIST);
			}
			else
			{
				if(c->Weapon[0].Type>=MODEL_SWORD && c->Weapon[0].Type<MODEL_MACE+MAX_ITEM_INDEX)
				{
					if(!ItemAttribute[c->Weapon[0].Type-MODEL_ITEM].TwoHand)
					{
						if(c->Weapon[1].Type>=MODEL_SWORD && c->Weapon[1].Type<MODEL_MACE+MAX_ITEM_INDEX)
						{
							switch(c->SwordCount%4)
							{
							case 0:SetAction(&c->Object,PLAYER_ATTACK_SWORD_RIGHT1  );break;
							case 1:SetAction(&c->Object,PLAYER_ATTACK_SWORD_LEFT1   );break;
							case 2:SetAction(&c->Object,PLAYER_ATTACK_SWORD_RIGHT1+1);break;
							case 3:SetAction(&c->Object,PLAYER_ATTACK_SWORD_LEFT1 +1);break;
							}
						}
						else
							SetAction(&c->Object,PLAYER_ATTACK_SWORD_RIGHT1+c->SwordCount%2);
					}
					else if ( c->Weapon[0].Type==MODEL_SWORD+21 || c->Weapon[0].Type==MODEL_SWORD+31 || c->Weapon[0].Type==MODEL_SWORD+23 || c->Weapon[0].Type==MODEL_SWORD+25)
					{
						SetAction ( &c->Object, PLAYER_ATTACK_TWO_HAND_SWORD_TWO);
					}
					else
					{
						SetAction(&c->Object,PLAYER_ATTACK_TWO_HAND_SWORD1+c->SwordCount%3);
					}
				}
				else if(c->Weapon[1].Type>=MODEL_SWORD && c->Weapon[1].Type<MODEL_MACE+MAX_ITEM_INDEX)
				{
					SetAction(&c->Object,PLAYER_ATTACK_SWORD_LEFT1+rand()%2);
				}
				else if(c->Weapon[0].Type>=MODEL_STAFF && c->Weapon[0].Type<MODEL_STAFF+MAX_ITEM_INDEX)
				{
					if(!ItemAttribute[c->Weapon[0].Type-MODEL_ITEM].TwoHand)
						SetAction(&c->Object,PLAYER_ATTACK_SWORD_RIGHT1+rand()%2);
					else
						SetAction(&c->Object,PLAYER_SKILL_WEAPON1+rand()%2);
				}
                //  창. ( 찌르는 애니메이션 ).
				else if(c->Weapon[0].Type==MODEL_SPEAR+1 || c->Weapon[0].Type==MODEL_SPEAR+2)
					SetAction(&c->Object,PLAYER_ATTACK_SPEAR1);
                //  창. ( 휘두르기 애니메이션 ).
				else if(c->Weapon[0].Type>=MODEL_SPEAR && c->Weapon[0].Type<MODEL_SPEAR+MAX_ITEM_INDEX)
					SetAction(&c->Object,PLAYER_ATTACK_SCYTHE1+c->SwordCount%3);
#ifdef ADD_SOCKET_ITEM
				// 활
				else if( GetEquipedBowType( c ) == BOWTYPE_BOW )
				{
					if(c->Wing.Type!=-1)
		         		SetAction(&c->Object,PLAYER_ATTACK_FLY_BOW);
					else
				    	SetAction(&c->Object,PLAYER_ATTACK_BOW);
				}
				// 석궁
				else if( GetEquipedBowType( c ) == BOWTYPE_CROSSBOW )
				{
					if(c->Wing.Type!=-1)
		         		SetAction(&c->Object,PLAYER_ATTACK_FLY_CROSSBOW);
					else
      					SetAction(&c->Object,PLAYER_ATTACK_CROSSBOW);
				}
#else // ADD_SOCKET_ITEM				// 정리할 때 지워야 하는 소스
                //  활.
				else if( ( c->Weapon[1].Type>=MODEL_BOW && c->Weapon[1].Type<MODEL_BOW+7  ) || c->Weapon[1].Type==MODEL_BOW+17
					|| c->Weapon[1].Type==MODEL_BOW+20 
					|| c->Weapon[1].Type == MODEL_BOW+21
					|| c->Weapon[1].Type == MODEL_BOW+22
					)
				{
        			if(c->Wing.Type!=-1)
		         		SetAction(&c->Object,PLAYER_ATTACK_FLY_BOW);
					else
				    	SetAction(&c->Object,PLAYER_ATTACK_BOW);
				}
				//. 석궁
				else if((c->Weapon[0].Type>=MODEL_BOW+8 && c->Weapon[0].Type<MODEL_BOW+15) ||
						(c->Weapon[0].Type>=MODEL_BOW+16 && c->Weapon[0].Type<MODEL_BOW+17)||
					    (c->Weapon[0].Type>=MODEL_BOW+18 && c->Weapon[0].Type<MODEL_BOW+MAX_ITEM_INDEX) )
				{
        			if(c->Wing.Type!=-1)
		         		SetAction(&c->Object,PLAYER_ATTACK_FLY_CROSSBOW);
					else
      					SetAction(&c->Object,PLAYER_ATTACK_CROSSBOW);
				}
#endif // ADD_SOCKET_ITEM				// 정리할 때 지워야 하는 소스
				else 
                    //  주먹 애니메이션.
					SetAction(&c->Object,PLAYER_ATTACK_FIST);
			}
		}
	}
	
	else if(o->Type==39)
	{
       	CreateEffect(MODEL_SAW,o->Position,o->Angle,o->Light);
     	PlayBuffer(SOUND_TRAP01,o);
	}
	else if(o->Type==40)
	{
		SetAction(&c->Object,1);
     	PlayBuffer(SOUND_TRAP01,o);
	}
	else if(o->Type==51)
	{
       	CreateEffect(BITMAP_FIRE+1,o->Position,o->Angle,o->Light);
        PlayBuffer(SOUND_FLAME);
	}
	else
	{
		if(o->Type==MODEL_MONSTER01+32)
		{
			int Action = rand()%8;
			if(Action > 2)
         		SetAction(&c->Object,MONSTER01_ATTACK1+rand()%2);
			else if(Action > 0)
         		SetAction(&c->Object,MONSTER01_ATTACK3);
			else
         		SetAction(&c->Object,MONSTER01_ATTACK4);
		}
		else 
		{
            bool Success = true;

            if ( SetCurrentAction_HellasMonster( c, o )==true ) Success = false;
            if ( battleCastle::SetCurrentAction_BattleCastleMonster( c, o )==true ) Success = false;
			if ( M31HuntingGround::SetCurrentActionHuntingGroundMonster(c, o)==true ) Success = false;
			if ( M33Aida::SetCurrentActionAidaMonster(c, o)==true ) Success = false;
			if(M34CryWolf1st::SetCurrentActionCrywolfMonster(c, o)==true ) Success = false;
			if(M37Kanturu1st::SetCurrentActionKanturu1stMonster(c, o) == true) Success = false;
			if(M38Kanturu2nd::Set_CurrentAction_Kanturu2nd_Monster(c, o) == true) Success = false;
			if ( M39Kanturu3rd::SetCurrentActionKanturu3rdMonster(c, o)==true ) Success = false;
			if (SEASON3A::CGM3rdChangeUp::Instance().SetCurrentActionBalgasBarrackMonster(c, o))	Success = false;
			if( g_CursedTemple->SetCurrentActionMonster(c,o)==true ) Success = false;
			if( SEASON3B::GMNewTown::SetCurrentActionMonster(c,o)==true ) Success = false;
			if( SEASON3C::GMSwampOfQuiet::SetCurrentActionMonster(c,o)==true ) Success = false;

#ifdef PSW_ADD_MAPSYSTEM
			if( TheMapProcess().SetCurrentActionMonster( c, o ) == true ) Success = false;
#endif //PSW_ADD_MAPSYSTEM

            if ( Success )
            {
			    if(c->SwordCount%3 == 0)
				    SetAction(&c->Object,MONSTER01_ATTACK1);
			    else
				    SetAction(&c->Object,MONSTER01_ATTACK2);
                c->SwordCount++;
            }
		}
	}
#ifdef YDG_ADD_SANTA_MONSTER
#ifdef PJH_NEW_SERVER_SELECT_MAP
	if (World == WD_73NEW_LOGIN_SCENE);	// 로그인화면에서는 공격사운드가 나오면 안된다
#else //PJH_NEW_SERVER_SELECT_MAP
	if (World == WD_77NEW_LOGIN_SCENE);	// 로그인화면에서는 공격사운드가 나오면 안된다
#endif //PJH_NEW_SERVER_SELECT_MAP
	else
#endif	// YDG_ADD_SANTA_MONSTER
	if(c->Object.AnimationFrame==0.f)
	{
#ifdef KJH_FIX_WOPS_K26908_ELF_MULTI_SKILL_SOUND_IN_SWAMPOFQUIET
		PlayMonsterSound(o);				// 몬스터 공격음
#else // KJH_FIX_WOPS_K26908_ELF_MULTI_SKILL_SOUND_IN_SWAMPOFQUIET
		if (PlayMonsterSound(o) == TRUE);	// 몬스터 공격음
		else
#endif // KJH_FIX_WOPS_K26908_ELF_MULTI_SKILL_SOUND_IN_SWAMPOFQUIET
		if(o->Type!=MODEL_PLAYER || (o->SubType>=MODEL_SKELETON1 && o->SubType<=MODEL_SKELETON3))
		{
			if(o->SubType>=MODEL_SKELETON1 && o->SubType<=MODEL_SKELETON3)
				PlayBuffer(SOUND_BRANDISH_SWORD01+rand()%2,o);
			if(Models[o->Type].Sounds[2]!=-1)
            {
                int offset = 0;
                if ( o->SubType==9 )
                {
                    offset = 5;
                }
                PlayBuffer(SOUND_MONSTER+offset+Models[o->Type].Sounds[2+rand()%2],o);
            }
		}
		else
		{
#ifdef ADD_SOCKET_ITEM
			// 활
			if( GetEquipedBowType( c ) == BOWTYPE_BOW )
			{
				PlayBuffer( SOUND_BOW01, o );
			}
			// 석궁
			else if( GetEquipedBowType( c ) == BOWTYPE_CROSSBOW )
			{
				PlayBuffer( SOUND_CROSSBOW01, o );
			}
#else // ADD_SOCKET_ITEM				// 정리할 때 지워야 하는 소스
			if(c->Weapon[1].Type>=MODEL_BOW && c->Weapon[1].Type<MODEL_BOW+7 || c->Weapon[1].Type==MODEL_BOW+17
				|| c->Weapon[1].Type==MODEL_BOW+20 
				|| c->Weapon[1].Type == MODEL_BOW+21
				|| c->Weapon[1].Type == MODEL_BOW+22
				)
				PlayBuffer(SOUND_BOW01,o);
			else if((c->Weapon[0].Type>=MODEL_BOW+8 && c->Weapon[0].Type<MODEL_BOW+15) ||
					(c->Weapon[0].Type>=MODEL_BOW+16 && c->Weapon[0].Type<MODEL_BOW+17)||
				    (c->Weapon[0].Type>=MODEL_BOW+18 && c->Weapon[0].Type<MODEL_BOW+MAX_ITEM_INDEX) )
				PlayBuffer(SOUND_CROSSBOW01,o);
#endif // ADD_SOCKET_ITEM				// 정리할 때 지워야 하는 소스
			else if(c->Weapon[0].Type>=MODEL_BOW+13 && c->Weapon[0].Type<MODEL_BOW+15)
				PlayBuffer(SOUND_MAGIC,o);
			else if(c->Weapon[0].Type==MODEL_SWORD+10 || c->Weapon[0].Type==MODEL_SPEAR)
				PlayBuffer(SOUND_BRANDISH_SWORD03,o);
			else if(c->Weapon[0].Type!=-1 || c->Weapon[1].Type!=-1)
				PlayBuffer(SOUND_BRANDISH_SWORD01+rand()%2,o);
		}
	}
	c->SwordCount++;
	
}

///////////////////////////////////////////////////////////////////////////////
// 케릭터 에니메이션 설정(마법 동작)
///////////////////////////////////////////////////////////////////////////////

void SetPlayerMagic(CHARACTER *c)
{
	OBJECT *o = &c->Object;
	if(o->Type==MODEL_PLAYER)
	{
        SetAttackSpeed();
		if( ( c->Helper.Type==MODEL_HELPER+2 || c->Helper.Type==MODEL_HELPER+3 ) && !c->SafeZone)
		{
			SetAction(o,PLAYER_RIDE_SKILL);
		}
		//^ 펜릴 캐릭터 에니메이션 설정 관련(마법 동작)
		else if(c->Helper.Type == MODEL_HELPER+37 && !c->SafeZone)
		{
			SetAction(o, PLAYER_FENRIR_ATTACK_MAGIC);
		}
		else
		{
			if(IsFemale(c->Class))
				SetAction(o,PLAYER_SKILL_ELF1);
      		else
				SetAction(o,PLAYER_SKILL_HAND1+rand()%2);
		}
	}
	else
	{
		if(c->SwordCount%3 == 0)
     		SetAction(&c->Object,MONSTER01_ATTACK1);
		else
     		SetAction(&c->Object,MONSTER01_ATTACK2);
        c->SwordCount++;
	}
}

///////////////////////////////////////////////////////////////////////////////
// 케릭터 에니메이션 설정(순간이동 동작)
///////////////////////////////////////////////////////////////////////////////

void SetPlayerTeleport(CHARACTER *c)
{
	OBJECT *o = &c->Object;
	if(o->Type == MODEL_PLAYER)
	{
		SetAction(o,PLAYER_SKILL_TELEPORT);
	}
	else
	{
		SetAction(o,MONSTER01_SHOCK);
	}
}

///////////////////////////////////////////////////////////////////////////////
// 케릭터 에니메이션 설정(충격 동작)
///////////////////////////////////////////////////////////////////////////////

void SetPlayerShock(CHARACTER *c,int Hit)
{
	if(c->Dead) return;
	if(c->Helper.Type==MODEL_HELPER+2 || c->Helper.Type==MODEL_HELPER+3) return;
	if(c->Helper.Type==MODEL_HELPER+4 ) return;

	OBJECT *o = &c->Object;

    //  기사 스킬2 ( 분노의 일격 ) 사용시에는 충격 동작을 하지 않는다.
    //  최대 생명력 증가 사용시. 
    if ( o->CurrentAction==PLAYER_ATTACK_SKILL_FURY_STRIKE || o->CurrentAction==PLAYER_SKILL_VITALITY ) return;
	if ( o->CurrentAction==PLAYER_SKILL_HELL_BEGIN 
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
		|| (o->CurrentAction==PLAYER_SKILL_ATT_UP_OURFORCES || o->CurrentAction==PLAYER_SKILL_HP_UP_OURFORCES
		|| o->CurrentAction==PLAYER_SKILL_GIANTSWING
		|| o->CurrentAction==PLAYER_SKILL_DRAGONLORE
		)
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
		) return;

	if(Hit > 0)
	{
		if(c->Object.Type==MODEL_PLAYER)
		{
			//^ 펜릴 캐릭터 에니메이션 설정(데미지)
			if(c->Helper.Type == MODEL_HELPER+37)
			{
				SetAction_Fenrir_Damage(c, &c->Object);
				if((rand()%3) == 0)
					PlayBuffer ( SOUND_FENRIR_DAMAGE_1+rand()%2, o );	// 펜릴 사운드 관련(데미지)
			}
			else
			{
				SetAction(&c->Object,PLAYER_SHOCK);
			}

    		c->Movement = false;
		}
		else
		{
			if(o->CurrentAction<MONSTER01_ATTACK1 || o->CurrentAction>MONSTER01_ATTACK2)
     			SetAction(&c->Object,MONSTER01_SHOCK);
    		//c->Movement = false;
		}
		if(c->Object.AnimationFrame==0.f)
		{
#ifdef YDG_ADD_SANTA_MONSTER
			PlayMonsterSound(o);				// 몬스터 피격음
#endif	// YDG_ADD_SANTA_MONSTER
     		if(o->Type!=MODEL_PLAYER || (o->SubType>=MODEL_SKELETON1 && o->SubType<=MODEL_SKELETON3))
			{
				if(o->SubType>=MODEL_SKELETON1 && o->SubType<=MODEL_SKELETON3)
					PlayBuffer(SOUND_BONE1,o);
				else if(o->Type!=MODEL_MONSTER01+14 && Models[o->Type].Sounds[2]!=-1)
                {
                    int offset = 0;
                    if ( o->SubType==9 )
                    {
                        offset = 5;
                    }
                    PlayBuffer(SOUND_MONSTER+offset+Models[o->Type].Sounds[2+rand()%2],o);
                }
			}
			else
			{
     			if(!IsFemale(c->Class))
                {
                    if ( GetBaseClass(c->Class)==CLASS_DARK_LORD && rand()%5 )
                        PlayBuffer ( SOUND_DARKLORD_PAIN, o );
                    else
    				    PlayBuffer(SOUND_HUMAN_SCREAM01+rand()%3,o);
                }
				else
    				PlayBuffer(SOUND_FEMALE_SCREAM01+rand()%2,o);
			}
		}

        if ( o->Type==MODEL_MONSTER01+61 )    //  성문.
        {
            vec3_t Position;
		    for(int i=0;i<5;i++)
            {
                if ( (rand()%2)==0 )
                {
                    Position[0] = o->Position[0]+(rand()%128-64);
                    Position[1] = o->Position[1];
                    Position[2] = o->Position[2]+200+(rand()%50);

                    CreateParticle(BITMAP_SMOKE+1,Position,o->Angle,o->Light);
                }
            }
            PlayBuffer ( SOUND_HIT_GATE );
        }
	}
}

///////////////////////////////////////////////////////////////////////////////
// 케릭터 에니메이션 설정(죽는 동작)
///////////////////////////////////////////////////////////////////////////////

void SetPlayerDie(CHARACTER *c)
{
	OBJECT *o = &c->Object;


	if(c==Hero)
	{
     	CharacterAttribute->Life = 0;
	}

	if(c->Object.Type==MODEL_PLAYER)
	{
		if(o->SubType>=MODEL_SKELETON1 && o->SubType<=MODEL_SKELETON3)
		{
			o->Live = false;
			CreateEffect(MODEL_BONE1,o->Position,o->Angle,o->Light);
			for(int j=0;j<10;j++)
				CreateEffect(MODEL_BONE2,o->Position,o->Angle,o->Light);
       		PlayBuffer(SOUND_BONE2,o);
		}
		else
    		SetAction(&c->Object,PLAYER_DIE1);
	}
	else
	{
		switch ( o->Type)
		{
		case MODEL_MONSTER01+30:	//해골 소뿔
			{
				o->Live = false;
				CreateEffect(MODEL_BONE1,o->Position,o->Angle,o->Light);
				for(int j=0;j<10;j++)
					CreateEffect(MODEL_BONE2,o->Position,o->Angle,o->Light);
       			PlayBuffer(SOUND_BONE2,o);
			}
			break;
		case MODEL_MONSTER01+25:
			{
				o->Live = false;
				for(int j=0;j<8;j++)
				{
					CreateEffect(MODEL_BIG_STONE1,o->Position,o->Angle,o->Light);
					CreateEffect(MODEL_BIG_STONE2,o->Position,o->Angle,o->Light);
				}
       			PlayBuffer(SOUND_BONE2,o);
			}
			break;
		case MODEL_MONSTER01+110:	// 블레이드 헌터
		case MODEL_MONSTER01+115:	// 트윈 테일
			{
				if(World == WD_39KANTURU_3RD && g_Direction.m_CKanturu.m_iKanturuState == KANTURU_STATE_MAYA_BATTLE)
				{
					FallingMonster(c, o);
					SetAction(&c->Object,MONSTER01_STOP2);
				}
				else
					SetAction(&c->Object,MONSTER01_DIE);
			}
			break;
		case MODEL_MONSTER01+118:	// 마야 왼손
		case MODEL_MONSTER01+119:	// 마야 오른손
			{
				if(World == WD_39KANTURU_3RD && g_Direction.m_CKanturu.m_iMayaState >= KANTURU_MAYA_DIRECTION_MAYA3)
					SetAction(&c->Object,MONSTER01_STOP2);
				else
					SetAction(&c->Object,MONSTER01_DIE);
			}
			break;
#ifdef YDG_ADD_DOPPELGANGER_MONSTER
		case MODEL_MONSTER01+191:	// 도플갱어
			{
				if (c->Object.CurrentAction != MONSTER01_APEAR)
				{
					SetAction(&c->Object,MONSTER01_DIE);
				}
			}
			break;
#endif	// YDG_ADD_DOPPELGANGER_MONSTER
		default:
          	SetAction(&c->Object,MONSTER01_DIE);
			break;
		}
    }

	if ( c->Object.AnimationFrame==0.f )
	{
		PlayMonsterSound(o);	// 몬스터 사망음
        if ( InChaosCastle()==true )
        {
         	PlayBuffer ( SOUND_CHAOS_MOB_BOOM01+rand()%2, o );
        }
        else
        {
		    if(o->Type!=MODEL_PLAYER || (o->SubType>=MODEL_SKELETON1 && o->SubType<=MODEL_SKELETON3))
		    {
			    if(Models[o->Type].Sounds[4]!=-1)
                {
                    int offset = 0;
                    if ( o->SubType==9 )
                    {
                        offset = 5;
                    }
                    PlayBuffer(SOUND_MONSTER+offset+Models[o->Type].Sounds[4],o);
                }
		    }
		    else
		    {
			    if(!IsFemale(c->Class))
                {
                    if ( GetBaseClass(c->Class)==CLASS_DARK_LORD )
             		    PlayBuffer ( SOUND_DARKLORD_DEAD, o );
                    else
             		    PlayBuffer(SOUND_HUMAN_SCREAM04,o);
                }
			    else
				{
         		    PlayBuffer(SOUND_FEMALE_SCREAM02,o);
				}

				//^ 펜릴 사운드 관련(죽음)
				if(c->Helper.Type == MODEL_HELPER+37)
					PlayBuffer(SOUND_FENRIR_DEATH, o);	// 펜릴 죽는
		    }
        }
        c->Object.AnimationFrame = 0.001f;
	}
}

///////////////////////////////////////////////////////////////////////////////
// 케릭터 움직임
///////////////////////////////////////////////////////////////////////////////

void CalcAddPosition(OBJECT *o,float x,float y,float z,vec3_t Position)
{
	float Matrix[3][4];
	AngleMatrix(o->Angle,Matrix);
	vec3_t p;
    Vector(x,y,z,p);
    VectorRotate(p,Matrix,Position);
	VectorAdd(Position,o->Position,Position);
}

vec3_t BossHeadPosition;

void AttackEffect(CHARACTER *c)
{
	OBJECT *o = &c->Object;
	BMD *b = &Models[o->Type];
	int i;
	vec3_t Angle,Light;
	vec3_t p,Position;
	float Luminosity = (float)(rand()%6+2)*0.1f;
	Vector(0.f,0.f,0.f,p);
	Vector(1.f,1.f,1.f,Light);

    //  헬라스 몬스터 공격 효과.
    if ( InHellas() )
    {
        CHARACTER *tc = NULL;
        OBJECT *to = NULL;
        
        if ( c->TargetCharacter >= 0 && c->TargetCharacter<MAX_CHARACTERS_CLIENT )
        {
            tc = &CharactersClient[c->TargetCharacter];
            to = &tc->Object;
        }
        
        //  헬라스 몬스터 공격 효과
        if ( AttackEffect_HellasMonster( c, tc, o, to, b )==true )
            return;
    }
	if ( M31HuntingGround::AttackEffectHuntingGroundMonster(c, o, b)==true )
		return;
    if ( battleCastle::AttackEffect_BattleCastleMonster( c, o, b )==true ) 
        return;
	if ( M33Aida::AttackEffectAidaMonster(c, o, b)==true )
		return;
#ifdef CRYINGWOLF_2NDMVP
	if ( M34CryingWolf2nd::AttackEffectCryingWolf2ndMonster(c, o, b)==true )
		return;
#endif // CRYINGWOLF_2NDMVP
	if(M37Kanturu1st::AttackEffectKanturu1stMonster(c, o, b) == true)
		return;
	if(M38Kanturu2nd::AttackEffect_Kanturu2nd_Monster(c, o, b) == true)
		return;
	if ( M39Kanturu3rd::AttackEffectKanturu3rdMonster(c, o, b)==true )
		return;
	if ( M34CryWolf1st::AttackEffectCryWolf1stMonster(c, o, b)==true )
		return;
	if (SEASON3A::CGM3rdChangeUp::Instance().AttackEffectBalgasBarrackMonster(c, o, b))
		return;
	if( g_CursedTemple->AttackEffectMonster(c, o, b)==true )
		return;
	if( SEASON3B::GMNewTown::AttackEffectMonster(c, o, b)==true ) return;
	if( SEASON3C::GMSwampOfQuiet::AttackEffectMonster(c, o, b)==true ) return;

#ifdef PSW_ADD_MAPSYSTEM
	if( TheMapProcess().AttackEffectMonster( c, o, b ) == true ) return;
#endif //PSW_ADD_MAPSYSTEM
	switch(c->MonsterIndex)
	{
    case 162 :  //  카오스캐슬 전사.
    case 164 :
    case 166 :
    case 168 :
    case 170 :
    case 172 :
	case 426:
        break;

    case 89:   //  마법 해골.
    case 95:
    case 112:
    case 118:
    case 124:
    case 130:
    case 143:
		if( ( c->Skill) == AT_SKILL_BOSS)
		{
            if ( (rand()%2)==0 )
            {
		        if(c->AttackTime == 1)
		        {
			        CreateEffect(MODEL_SKILL_INFERNO,o->Position,o->Angle,o->Light, 1);
		        }
            }
            else
            {
			    Vector(o->Position[0]+rand()%1024-512,o->Position[1]+rand()%1024-512,o->Position[2],Position);
			    CreateEffect(MODEL_FIRE,Position,o->Angle,o->Light);
			    PlayBuffer(SOUND_METEORITE01);
            }
		}
        break;

    case 68://깃털괴물.
        break;
    case 69://별자리.
        break;
    case 70://비의여왕.
        if( c->AttackTime==5 )
        {
			if(c->TargetCharacter != -1)
			{
				CHARACTER *tc = &CharactersClient[c->TargetCharacter];
				OBJECT *to = &tc->Object;
				for( int i=0; i<20; ++i )
				{
					CreateEffect(BITMAP_BLIZZARD,to->Position,to->Angle,Light);
				}
			}
        }
        break;
#ifdef LOREN_RAVINE_EVENT
	case 301:
#endif
    case 71://메가크러스트
	case 74://메가크러스트2
        if(c->Object.CurrentAction==MONSTER01_ATTACK1 || c->Object.CurrentAction==MONSTER01_ATTACK2)
        {
            if(c->AttackTime==5)
            {
				CreateInferno(o->Position);
				CreateEffect(MODEL_SKILL_INFERNO,o->Position,o->Angle,o->Light);
            }
        }
        break;
    case 72://이카루스.
		if( ( c->Skill) == AT_SKILL_BOSS)
		{
			if(c->AttackTime == 14)
			{	// 지역공격
				vec3_t Angle = { 0.0f, 0.0f, 0.0f};
				int iCount = 36;

				for (int i = 0; i < iCount; ++i)
				{
					//Angle[2] = ( float)i * 10.0f;
					Angle[0] = ( float)( rand() % 360);
					Angle[1] = ( float)( rand() % 360);
					Angle[2] = ( float)( rand() % 360);
					vec3_t Position;
					VectorCopy( o->Position, Position);
					Position[2] += 100.f;
					CreateJoint(BITMAP_JOINT_SPIRIT,Position,Position,Angle,1,NULL,60.f,0,0);
				}
			}
		}
        break;
    case 73://흑룡.
	case 75://흑룡2
        if(c->Object.CurrentAction==MONSTER01_ATTACK1)
        {
            if(c->AttackTime==11)
            {
				CreateInferno(o->Position);
				CreateEffect(MODEL_SKILL_INFERNO,o->Position,o->Angle,o->Light);

				for ( int k = 0; k < 5; ++k)
				{
					Vector(1.f,0.5f,0.f,Light);
					VectorCopy(o->Angle,Angle);
					Angle[0] += 45.f;
					VectorCopy(o->Position, Position);
					Position[0] += ( float)( rand() % 1001 - 500);
					Position[1] += ( float)( rand() % 1001 - 500);
					Position[2] += 500.f;
					VectorCopy(Position,o->StartPosition );
					CreateEffect(MODEL_PIERCING+1,Position,Angle,Light,1,o);
				}
            }
        }
        else
        {
            if(c->AttackTime==13)
            {
                Vector(1.f,0.5f,0.f,Light);
                Vector(-50.f,100.f,0.f,p);
                VectorCopy(o->Angle,Angle);
                Angle[0] += 45.f;
			    b->TransformPosition(o->BoneTransform[11],p,Position,true);
                VectorCopy(Position,o->StartPosition );
			    CreateEffect(MODEL_PIERCING+1,Position,Angle,Light,1,o);
				PlayBuffer(SOUND_METEORITE01);
            }
            else if ( c->AttackTime==9 )
            {
                Vector(1.f,0.5f,0.f,Light);
                Vector(0.f,0.f,0.f,p);
                VectorCopy(o->Angle,Angle);
                Angle[0] += 45.f;
			    b->TransformPosition(o->BoneTransform[11],p,Position,true);
            }
        }

        break;
	case 77://불사조공격
		if( ( c->Skill) == AT_SKILL_BOSS)
		{
			if(c->AttackTime == 2 || c->AttackTime == 6)
			{	// 지역공격
				vec3_t Angle = { 0.0f, 0.0f, 0.0f};
				int iCount = 40;
				for ( i = 0; i < iCount; ++i)
				{
					//Angle[2] = ( float)i * 10.0f;
					Angle[0] = ( float)( rand() % 360);
					Angle[1] = ( float)( rand() % 360);
					Angle[2] = ( float)( rand() % 360);
					vec3_t Position;
					VectorCopy( o->Position, Position);
					Position[2] += 100.f;
					CreateJoint(BITMAP_JOINT_SPIRIT,Position,Position,Angle,3,NULL,50.f,0,0);
				}
			}
		}
		break;
	case 63://데쓰빔나이트
	case 61://빔나이트
		if(c->MonsterIndex == 63)
		{
			if(c->AttackTime == 1)
			{
				CreateInferno(o->Position);
				CreateEffect(MODEL_SKILL_INFERNO,o->Position,o->Angle,o->Light);
			}
			if(( c->Skill) == AT_SKILL_BOSS)
			{
				if(c->MonsterIndex == 63)
				{
					Vector(o->Position[0]+rand()%800-400,o->Position[1]+rand()%800-400,o->Position[2],Position);
					CreateEffect(MODEL_SKILL_BLAST,Position,o->Angle,o->Light);
				}
				if(c->AttackTime == 14)
				{
					for(int i=0;i<18;i++)
					{
						VectorCopy(o->Angle,Angle);
						Angle[2] += i*20.f;
						CreateEffect(MODEL_STAFF+8,o->Position,Angle,o->Light);
					}
				}
			}
		}
		else
		{
			if(c->AttackTime == 1)
			{
				//CreateInferno(o->Position);
				CreateEffect(MODEL_SKILL_INFERNO,o->Position,o->Angle,o->Light);
			}
		}
		break;
	case 66://저주받은 왕
		if( ( c->Skill) == AT_SKILL_BOSS)
		{
			if(c->AttackTime == 1)
			{
				CreateEffect(MODEL_SKILL_INFERNO,o->Position,o->Angle,o->Light, 1);
			}
		}
		break;
	case 54://  솔져
	case 57://  아이언휠
    case 151:// 소환된 솔져
		if(c->AttackTime == 1)
		{
			Vector(60.f,-110.f,0.f,p);
			b->TransformPosition(o->BoneTransform[c->Weapon[0].LinkBone],p,Position,true);
			CreateEffect(MODEL_ARROW_BOMB,o->Position,o->Angle,o->Light,0,o);
			if(c->MonsterIndex == 57)
			{
				vec3_t Angle;
				VectorCopy(o->Angle,Angle);
				Angle[2] += 20.f;
				CreateEffect(MODEL_ARROW_BOMB,o->Position,Angle,o->Light,0,o);
				Angle[2] -= 40.f;
				CreateEffect(MODEL_ARROW_BOMB,o->Position,Angle,o->Light,0,o);
			}
		}
		break;
	case 53://지하제국
	case 58://탄타로스
	case 59://
		if(c->AttackTime == 1)
		{
			CreateInferno(o->Position);
		}
		if(c->AttackTime == 14)
		{
			if(c->MonsterIndex == 59)
			{
				if(( c->Skill) == AT_SKILL_BOSS)
				{
					for(i=0;i<18;i++)
					{
						VectorCopy(o->Angle,Angle);
						Angle[2] += i*20.f;
						CreateEffect(MODEL_STAFF+8,o->Position,Angle,o->Light);
					}
				}
			}
		}
		break;
	case 49://히드라
		if(c->AttackTime%5 == 1)
		{
			b->TransformPosition(o->BoneTransform[63],p,Position,true);
			CreateEffect(BITMAP_BOSS_LASER+1,Position,o->Angle,o->Light);
		}
		if(( c->Skill) == AT_SKILL_BOSS)
		{
			if(c->AttackTime == 1)
			{
				VectorCopy(o->Angle,Angle);Angle[2] += 20.f;
				VectorCopy(o->Position,p);p[2] += 50.f;
				Luminosity = (15-c->AttackTime)*0.1f;
				Vector(Luminosity*0.3f,Luminosity*0.6f,Luminosity*1.f,Light);

				for(int i=0;i<9;i++)
				{
					Angle[2] += 40.f;
					CreateEffect(BITMAP_BOSS_LASER,p,Angle,Light);
				}
			}
		}
		break;
	case 42://드래곤
		if(( c->Skill) == AT_SKILL_BOSS)
		{
			if(c->AttackTime == 1)
			{
				Vector(0.f,0.f,0.f,p);
				b->TransformPosition(o->BoneTransform[11],p,Position,true);
				Vector(o->Angle[0]-20.f,o->Angle[1],o->Angle[2]-30.f,Angle);
				CreateEffect(MODEL_FIRE,Position,Angle,o->Light,2);
				Vector(o->Angle[0]-30.f,o->Angle[1],o->Angle[2],Angle);
				CreateEffect(MODEL_FIRE,Position,Angle,o->Light,2);
				Vector(o->Angle[0]-20.f,o->Angle[1],o->Angle[2]+30.f,Angle);
				CreateEffect(MODEL_FIRE,Position,Angle,o->Light,2);
				PlayBuffer(SOUND_METEORITE01);
			}
			Vector(o->Position[0]+rand()%1024-512,o->Position[1]+rand()%1024-512,o->Position[2],Position);
			CreateEffect(MODEL_FIRE,Position,o->Angle,o->Light);
			PlayBuffer(SOUND_METEORITE01);
		}
		break;
	case 35://데쓰 고르곤
		if(( c->Skill) == AT_SKILL_BOSS)
		{
			if(c->AttackTime == 1)
			{
				for(int i=0;i<18;i++)
				{
					Vector(0.f,0.f,i*20.f,Angle);
					CreateEffect(MODEL_FIRE,o->Position,Angle,o->Light,1,o);
				}
				PlayBuffer(SOUND_METEORITE01);
			}
		}
		break;
	case 38://발록
	case 67://발록2
		if(( c->Skill) == AT_SKILL_BOSS)
		{
			if(c->AttackTime == 1)
			{
				CreateEffect(MODEL_CIRCLE,o->Position,o->Angle,o->Light);
				CreateEffect(MODEL_CIRCLE_LIGHT,o->Position,o->Angle,o->Light);
				PlayBuffer(SOUND_HELLFIRE);
			}
			Vector(o->Position[0]+rand()%1024-512,o->Position[1]+rand()%1024-512,o->Position[2],Position);
			CreateEffect(MODEL_FIRE,Position,o->Angle,o->Light);
			PlayBuffer(SOUND_METEORITE01);
		}
		break;
	case 103://함정
		if(( c->Skill) == AT_SKILL_BOSS)
		{
			Vector(o->Position[0]+rand()%1024-512,o->Position[1]+rand()%1024-512,o->Position[2],Position);
			CreateEffect(MODEL_FIRE,Position,o->Angle,o->Light);
			PlayBuffer(SOUND_METEORITE01);
		}
		break;
	case 45://물고기
		for(int i=0;i<4;i++)
		{
			Vector((float)(rand()%32-16),(float)(rand()%32-16),(float)(rand()%32-16),p);
			b->TransformPosition(o->BoneTransform[2],p,Position,true);
			CreateParticle(BITMAP_BUBBLE,Position,o->Angle,Light);
			CreateParticle(BITMAP_BLOOD+1,Position,o->Angle,Light);
		}
		break;
    default :
        break;
	}

	if(c->TargetCharacter >= 0 && c->TargetCharacter<MAX_CHARACTERS_CLIENT )
	{
		CHARACTER *tc = &CharactersClient[c->TargetCharacter];
		OBJECT *to = &tc->Object;

		if(( c->Skill) == AT_SKILL_ENERGYBALL)
		{
			switch(c->MonsterIndex)
			{
            case 163 :  //  카오스캐슬 궁수.
            case 165 :
            case 167 :
            case 169 :
            case 171 :
            case 173 :
			case 427:
                if ( c->Weapon[0].Type==MODEL_BOW+19 )
                {
                    if( c->AttackTime==8 )
                    {
                        CreateArrows ( c, o, o, 0, 0, 0 );
                    }
                }
                else if ( c->Object.CurrentAction==MONSTER01_ATTACK1 )
                {
                    if ( c->AttackTime==15 )
                    {
			            CalcAddPosition ( o, -20.f, -90.f, 100.f, Position );
			            CreateEffect ( BITMAP_BOSS_LASER, Position, o->Angle, Light, 0, o );
                    }
                }
                else if ( c->Object.CurrentAction==MONSTER01_ATTACK2 )
                {
                    if ( c->AttackTime==8 )
                    {
                        if ( rand()%2==0 )
                        {
			                CreateEffect ( MODEL_SKILL_BLAST, to->Position, o->Angle, o->Light, 0, o );
			                CreateEffect ( MODEL_SKILL_BLAST, to->Position, o->Angle, o->Light, 0, o );
                        }
                        else
                        {
			                Vector ( 0.8f, 0.5f, 0.1f, Light );
			                CreateEffect ( MODEL_FIRE, to->Position, o->Angle, Light, 6 );
			                CreateEffect ( MODEL_FIRE, to->Position, o->Angle, Light, 6 );
                        }
                    }
                }
                break;

            case 89:   //  마법 해골.
            case 95:
            case 112:
            case 118:
            case 124:
            case 130:
            case 143:
				if ( 14 == c->AttackTime)
				{
					Vector(0.f,0.f,0.f,p);
					b->TransformPosition(o->BoneTransform[33],p,Position,true);
                    VectorCopy(o->Angle,Angle);
			        CreateEffect(MODEL_PIERCING+1,Position,Angle,Light,1);
				    CreateJoint(BITMAP_JOINT_THUNDER,Position,Position,Angle,2,to,50.f);
				}
                break;

            case 87 :	//. 자이언트오거1
	        case 93 :	//. 자이언트오거2
	        case 99 :	//. 자이언트오거3
	        case 116 :	//. 자이언트오거4
	        case 122 :	//. 자이언트오거5
	        case 128 :	//. 자이언트오거6
            case 141:
		        if ( c->AttackTime==13 )
		        {
			        Vector(1.0f,1.0f,1.0f,Light);
			        Vector(60.f,30.f,0.f,p);
			        b->TransformPosition(o->BoneTransform[6],p,Position,true);

			        Vector ( o->Angle[0],o->Angle[1],o->Angle[2],Angle);
			        CreateEffect(MODEL_FIRE,Position,Angle,o->Light,5 );
		        }
                break;

			case 77://불사조공격
				if ( 14 == c->AttackTime)
				{
					Vector(0.f,0.f,0.f,p);
					b->TransformPosition(g_fBoneSave[2],p,Position,true);
					/*for ( int i = 0; i < 10; ++i)
					{
						CreateEffect(MODEL_FIRE,Position,o->Angle,o->Light,3,to);
					}*/
                    VectorCopy(o->Angle,Angle);
			        CreateEffect(MODEL_PIERCING+1,Position,Angle,Light,1);
				    CreateJoint(BITMAP_JOINT_THUNDER,Position,Position,Angle,2,to,50.f);
				}
				break;
            case 73:
			case 75:
                if(c->Object.CurrentAction==MONSTER01_ATTACK2)
                {
                    if(c->AttackTime==13)
                    {
                        Vector(1.f,0.5f,0.f,Light);
                        Vector(-50.f,100.f,0.f,p);
                        VectorCopy(o->Angle,Angle);
                        Angle[0] += 45.f;
			            b->TransformPosition(o->BoneTransform[11],p,Position,true);
			            CreateEffect(MODEL_PIERCING+1,Position,Angle,Light,1);
				        CreateJoint(BITMAP_JOINT_THUNDER,Position,to->Position,Angle,2,to,50.f);
                    }
                }
                break;
			case 69:
				if ( c->AttackTime==1)
				{
					for ( int i = 0; i < 4; ++i)
					{
						CreateJoint(BITMAP_FLARE,o->Position,o->Position,Angle,7,to,50.f);
						CreateJoint(BITMAP_FLARE,Position,Position,Angle,7,to,50.f);
					}
				}
				break;
			case 61:
				for(int i=0;i<6;i++)
				{
					int Hand = 0;
					if(i>=3) Hand = 1;
					b->TransformPosition(o->BoneTransform[c->Weapon[Hand].LinkBone],p,Position,true);//에러
					Vector(0.f,0.f,(float)(rand()%360),Angle);
					CreateJoint(BITMAP_JOINT_THUNDER,Position,to->Position,Angle,2,to,50.f);
					CreateJoint(BITMAP_JOINT_THUNDER,Position,to->Position,Angle,2,to,10.f);
				}

				if(c->AttackTime == 1)
					PlayBuffer(SOUND_EVIL);

				for(int i=0;i<4;i++)
				{
					int Hand = 0;
					if(i>=2) Hand = 1;
					b->TransformPosition(o->BoneTransform[c->Weapon[Hand].LinkBone],p,Position,true);
					Vector(0.f,0.f,(float)(rand()%360),Angle);
					CreateJoint(BITMAP_JOINT_LASER+1,Position,to->Position,Angle,0,to,50.f);
					CreateParticle(BITMAP_FIRE,Position,o->Angle,o->Light);
				}
				break;
			case 46:
				if(c->AttackTime == 1)
					PlayBuffer(SOUND_EVIL);

				for(int i=0;i<4;i++)
				{
					int Hand = 0;
					if(i>=2) Hand = 1;
					Vector(0.f,0.f,0.f,Angle);
					b->TransformPosition(o->BoneTransform[c->Weapon[Hand].LinkBone],p,Position,true);
					CreateJoint(BITMAP_BLUR+1,Position,to->Position,Angle,1,to,50.f);
					CreateJoint(BITMAP_BLUR+1,Position,to->Position,Angle,1,to,10.f);
				}
				break;
			case 37:
				if(c->AttackTime == 1)
					PlayBuffer(SOUND_EVIL);

				for(int i=0;i<4;i++)
				{
					int Hand = 0;
					if(i>=2) Hand = 1;
					b->TransformPosition(o->BoneTransform[c->Weapon[Hand].LinkBone],p,Position,true);
					Vector(0.f,0.f,(float)(rand()%360),Angle);
					CreateJoint(BITMAP_JOINT_LASER+1,Position,to->Position,Angle,0,to,50.f);
					CreateParticle(BITMAP_FIRE,Position,o->Angle,o->Light);
				}
				break;
			case 66:
				{
					if(c->AttackTime == 1)
						PlayBuffer(SOUND_THUNDER01);
					float fAngle = (float)(45.f-(c->AttackTime*3+(int)WorldTime/10)%90)+180.f;

					for(int i=0;i<4;i++)
					{
						b->TransformPosition(o->BoneTransform[c->Weapon[i%2].LinkBone],p,Position,true);
						Vector(0.f,0.f,fAngle,Angle);
						CreateJoint(BITMAP_JOINT_LASER+1,Position,to->Position,Angle,1,to,50.f);
						CreateParticle(BITMAP_FIRE,Position,o->Angle,o->Light);
						fAngle += 270.f;
					}
				}
				break;

            default :
                break;
			}
		}
		else if(( c->Skill) == AT_SKILL_THUNDER)
		{
			switch(c->MonsterIndex)
			{
            case 89:   //  마법 해골.
            case 95:
            case 112:
            case 118:
            case 124:
            case 130:
            case 143:
                {
				    if(c->AttackTime == 1)
					    PlayBuffer(SOUND_THUNDER01);
				    float fAngle = (float)(45.f-(c->AttackTime*3+(int)WorldTime/10)%90)+180.f;
				    for(int i=0;i<4;i++)
				    {
					    b->TransformPosition(o->BoneTransform[c->Weapon[i%2].LinkBone],p,Position,true);
					    Vector(0.f,0.f,fAngle,Angle);
					    CreateJoint(BITMAP_JOINT_LASER+1,Position,to->Position,Angle,1,to,50.f);
					    CreateParticle(BITMAP_FIRE,Position,o->Angle,o->Light);
					    fAngle += 270.f;
				    }
                }
                break;

			case 77://불사조공격
				if ( 8 <= c->AttackTime)
				{
					Vector(0.f,0.f,0.f,p);
					b->TransformPosition(g_fBoneSave[0],p,Position,true);
					for ( int i = 0; i < 4; ++i)
					{
						Vector(0.f,0.f,(float)(rand()%360),Angle);
						CreateJoint(BITMAP_JOINT_THUNDER,Position,to->Position,Angle,0,to,80.f);
					}
				}
				break;
			case 37://데빌
				if(c->AttackTime == 1)
					PlayBuffer(SOUND_EVIL);

				for(int i=0;i<4;i++)
				{
					int Hand = 0;
					if(i>=2) Hand = 1;
					b->TransformPosition(o->BoneTransform[c->Weapon[Hand].LinkBone],p,Position,true);
					Vector(0.f,0.f,(float)(rand()%360),Angle);
					CreateJoint(BITMAP_JOINT_LASER+1,Position,to->Position,Angle,0,to,50.f);
					CreateParticle(BITMAP_FIRE,Position,o->Angle,o->Light);
				}
				break;
			case 34:

				for(int i=0;i<4;i++)
				{
					int Hand = 0;
					if(i>=2) Hand = 1;
					b->TransformPosition(o->BoneTransform[c->Weapon[Hand].LinkBone],p,Position,true);
					Vector(0.f,0.f,(float)(rand()%360),Angle);
					CreateJoint(BITMAP_JOINT_THUNDER,Position,to->Position,Angle,0,to,50.f);
					CreateJoint(BITMAP_JOINT_THUNDER,Position,to->Position,Angle,0,to,10.f);
					CreateParticle(BITMAP_ENERGY,Position,o->Angle,Light);
				}
				break;
			case 48://리자드킹
				for(int i=0;i<6;i++)
				{
					int Hand = 0;
					if(i>=3) Hand = 1;
					b->TransformPosition(o->BoneTransform[c->Weapon[Hand].LinkBone],p,Position,true);//에러
					Vector(0.f,0.f,(float)(rand()%360),Angle);
					CreateJoint(BITMAP_JOINT_THUNDER,Position,to->Position,Angle,2,to,50.f);
					CreateJoint(BITMAP_JOINT_THUNDER,Position,to->Position,Angle,2,to,10.f);
				}
				break;
            case 39:
				if(o->Type == MODEL_PLAYER)
				{
					Vector(0.f,0.f,0.f,p);
				}
				else
				{
					Vector(0.f,-130.f,0.f,p);
				}
				b->TransformPosition(o->BoneTransform[c->Weapon[0].LinkBone],p,Position,true);
				Vector(-60.f,0.f,o->Angle[2],Angle);
				CreateJoint(BITMAP_JOINT_THUNDER,Position,to->Position,Angle,0,to,50.f);
				CreateJoint(BITMAP_JOINT_THUNDER,Position,to->Position,Angle,0,to,10.f);
				CreateParticle(BITMAP_ENERGY,Position,o->Angle,Light);
                break;
			case 386:
			case 389:
			case 392:
			case 395:
			case 398:
			case 401:
				{
					Vector(8.f,0.f,0.f,Light);
					b->TransformPosition(o->BoneTransform[17],p,Position,true);
					Vector(-60.f,0.f,o->Angle[2],Angle);
					//CreateJoint(BITMAP_JOINT_THUNDER,Position,to->Position,Angle,0,to,50.f);
					CreateJoint(BITMAP_JOINT_THUNDER,Position,to->Position,Angle,21,to,50.f);
					CreateParticle(BITMAP_ENERGY,Position,o->Angle,Light);

					b->TransformPosition(o->BoneTransform[41],p,Position,true);
					Vector(-60.f,0.f,o->Angle[2],Angle);
					//CreateJoint(BITMAP_JOINT_THUNDER,Position,to->Position,Angle,0,to,50.f);
					CreateJoint(BITMAP_JOINT_THUNDER,Position,to->Position,Angle,21,to,50.f);
					CreateParticle(BITMAP_ENERGY,Position,o->Angle,Light);
				}
				break;
			// 플레이어 이거나 기타 몬스터가 전기(번개)를 사용했을시
			default:
                if ( b->NumBones<c->Weapon[0].LinkBone ) break;

				if(o->Type == MODEL_PLAYER)
				{
					Vector(0.f,0.f,0.f,p);
				}
				else
				{
					Vector(0.f,-130.f,0.f,p);
				}
				b->TransformPosition(o->BoneTransform[c->Weapon[0].LinkBone],p,Position,true);
				Vector(-60.f,0.f,o->Angle[2],Angle);
				CreateJoint(BITMAP_JOINT_THUNDER,Position,to->Position,Angle,0,to,50.f);
				CreateJoint(BITMAP_JOINT_THUNDER,Position,to->Position,Angle,0,to,10.f);
				CreateParticle(BITMAP_ENERGY,Position,o->Angle,Light);
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

	if(pCharacter->MonsterIndex == 364)	// 마야 일 때 만 예외 처리
	{
		M39Kanturu3rd::MayaSceneMayaAction(pCharacter->MonsterSkill);
		return true;
	}

	int iCheckAttackAni = -1;
#ifdef CSK_FIX_MONSTERSKILL
	for(int i = 0; i < MAX_MONSTERSKILL_NUM; i++)
#else // CSK_FIX_MONSTERSKILL
	for(int i = 0; i < 5; i++)
#endif // CSK_FIX_MONSTERSKILL
	{
		if(	MonsterSkill[pCharacter->MonsterIndex].Skill_Num[i] == pCharacter->MonsterSkill)
		{
			iCheckAttackAni = i;
			break;
		}
		else
		{
			iCheckAttackAni = -1;
		}
	}

	switch(iCheckAttackAni)
	{
		case 0:
			SetAction ( pObject, MONSTER01_ATTACK1 );
			break;
		case 1:
			SetAction ( pObject, MONSTER01_ATTACK2 );
			break;
		case 2:
			SetAction ( pObject, MONSTER01_ATTACK3 );
			break;
		case 3:
			SetAction ( pObject, MONSTER01_ATTACK4 );
			break;
		case 4:
			SetAction ( pObject, MONSTER01_ATTACK5 );
			break;
#ifdef CSK_FIX_MONSTERSKILL
		case 5:
			SetAction ( pObject, MONSTER01_ATTACK5 );
			break;
		case 6:
			SetAction ( pObject, MONSTER01_ATTACK5 );
			break;
		case 7:
			SetAction ( pObject, MONSTER01_ATTACK5 );
			break;
		case 8:
			SetAction ( pObject, MONSTER01_ATTACK5 );
			break;
		case 9:
			SetAction ( pObject, MONSTER01_ATTACK5 );
			break;
#endif // CSK_FIX_MONSTERSKILL	
	}

	pCharacter->MonsterSkill = -1;


	if(iCheckAttackAni < 0)		// 찾은 스킬이 없으면 기본 공격 동작(공격동작1)으로 애니메이션 시킨다 
	{
		SetAction ( pObject, MONSTER01_ATTACK1 );
	}

	if(iCheckAttackAni > 4)		// 혹시 모를 iCheckAttackAni값이 이상하게 들어갈 경우 예외 처리
		return false;

	return true;
}

//  스킬에 따른 애니메이션을 제어한다.
bool CharacterAnimation(CHARACTER* c, OBJECT* o)
{
    bool Play;
	BMD *b = &Models[o->Type];

	float PlaySpeed = 0.f;
	if(b->NumActions > 0)
	{
		PlaySpeed = b->Actions[b->CurrentAction].PlaySpeed; 
		if(PlaySpeed < 0.f)
			PlaySpeed = 0.f;
		if(c->Change && o->CurrentAction>=MONSTER01_ATTACK1 && o->CurrentAction<=MONSTER01_ATTACK2)
			PlaySpeed *= 1.5f;
        if ( o->CurrentAction==PLAYER_SKILL_VITALITY && o->AnimationFrame>6.f )
        {
            PlaySpeed /= 2.f;
        }
        else if ( ( o->CurrentAction==PLAYER_ATTACK_TELEPORT || o->CurrentAction==PLAYER_ATTACK_RIDE_TELEPORT 
				|| o->CurrentAction == PLAYER_FENRIR_ATTACK_DARKLORD_TELEPORT	//^ 펜릴 캐릭터 에니메이션
				) && o->AnimationFrame>5.5f )
        {
            PlaySpeed /= 10.f;
        }
        else if ( GetBaseClass(c->Class)==CLASS_DARK_LORD && 
                  ( o->CurrentAction==PLAYER_SKILL_FLASH || o->CurrentAction==PLAYER_ATTACK_RIDE_ATTACK_FLASH 
				  || o->CurrentAction == PLAYER_FENRIR_ATTACK_DARKLORD_FLASH	//^ 펜릴 스킬 관련
				  ) && ( o->AnimationFrame>1.f && o->AnimationFrame<3.f )
                )
        {	
			if( g_pPartyManager->IsPartyMemberChar( c ) == false )			
            {
                PlaySpeed /= 2.f;
            }
            else
            {
                PlaySpeed /= 8.f;
            }
        }
		if ( o->CurrentAction==PLAYER_SKILL_HELL_BEGIN )
		{
			PlaySpeed /= 2.f;
		}
		if ( o->Type != MODEL_PLAYER )
		{
			switch ( o->Type )
			{
			case MODEL_MONSTER01+64 :	// 진짜 쿤둔
				if (o->CurrentAction == MONSTER01_DIE && o->AnimationFrame > 6)
					PlaySpeed *= 4.0f;
				break;
			case MODEL_FACE:
			case MODEL_FACE+1:
			case MODEL_FACE+2:
			case MODEL_FACE+3:
			case MODEL_FACE+4:
			case MODEL_FACE+5:
#ifdef PBG_ADD_NEWCHAR_MONK
			case MODEL_FACE+6:
#endif //PBG_ADD_NEWCHAR_MONK
				PlaySpeed *=2.0f;
				break;
			}
		}
		if( o->Type == MODEL_MONSTER01+87)	//. 사냥터 보스
		{
			if(o->CurrentAction == MONSTER01_DIE)
				PlaySpeed /= 2.f;
		}
	}

	if( g_isCharacterBuff(o, eDeBuff_Stun) || g_isCharacterBuff(o, eDeBuff_Sleep) )
    {
        return false;
    }
	
	Play = b->PlayAnimation(&o->AnimationFrame,&o->PriorAnimationFrame,&o->PriorAction,PlaySpeed,o->Position,o->Angle);
	if(o->CurrentAction == PLAYER_CHANGE_UP)
	{
		if(Play == false)
			SetPlayerStop(c);
	}

#ifdef PJH_SEASON4_SPRITE_NEW_SKILL_RECOVER
	if(o->CurrentAction == PLAYER_RECOVER_SKILL)
	{
		if(Play == false)
			SetPlayerStop(c);
	}
#endif //PJH_SEASON4_SPRITE_NEW_SKILL_RECOVER
#ifdef WORLDCUP_ADD
#ifndef NO_MORE_DANCING
	if(o->CurrentAction == PLAYER_POINT_DANCE)
	{
		int x = ( c->PositionX);
		int y = ( c->PositionY);

		BoneMatrix_t *bm = &b->Bones[0].BoneMatrixes[o->CurrentAction];

		int ax = (int)(bm->Position[b->CurrentAnimationFrame][0]);
		int ay = (int)(bm->Position[b->CurrentAnimationFrame][1]);

		ax /= TERRAIN_SCALE;
		ay /= TERRAIN_SCALE;
		DWORD wall = TerrainWall[(y+ay)*TERRAIN_SIZE+(x+ax)];
        if ( (wall&TW_NOMOVE)==TW_NOMOVE )
		{
			SetPlayerStop(c);
		}
	}
#endif
#endif
	
    return Play;
}


int GetHandOfWeapon( OBJECT* o)
{
	int Hand = 0;
	if(o->Type == MODEL_PLAYER)
	{
		if(o->CurrentAction==PLAYER_ATTACK_SWORD_LEFT1 || o->CurrentAction==PLAYER_ATTACK_SWORD_LEFT2)
			Hand = 1;
	}

	return ( Hand);
}


bool AttackStage(CHARACTER* c, OBJECT* o)
{
	// 무기 위치 얻기
	int Hand = GetHandOfWeapon( o);

	int iSkill = ( c->Skill);

    g_iLimitAttackTime = 15;
    switch ( iSkill)
    {
#ifdef PJH_SEASON4_MASTER_RANK4
	case AT_SKILL_BLOW_UP:
	case AT_SKILL_BLOW_UP+1:
	case AT_SKILL_BLOW_UP+2:
	case AT_SKILL_BLOW_UP+3:
	case AT_SKILL_BLOW_UP+4:
#endif	//PJH_SEASON4_MASTER_RANK4
	case AT_SKILL_ONETOONE:	// 기사 일대일 스킬
		{
			BMD *b = &Models[o->Type];

            if ( b->Bones[c->Weapon[Hand].LinkBone].Dummy || c->Weapon[Hand].LinkBone>=b->NumBones )
            {
                break;
            }

			if ( 8 == c->AttackTime)
			{
				if (SceneFlag != LOG_IN_SCENE)
					PlayBuffer(SOUND_SKILL_SWORD2);
			}

			if ( 2 <= c->AttackTime && c->AttackTime <= 8)
			{	// 기 모으기
				for ( int j = 0; j < 3; ++j)
				{
					vec3_t CurPos;
					VectorCopy( o->Position, CurPos);
					CurPos[2] += 120.0f;
					vec3_t TempPos;
					GetNearRandomPos( CurPos, 300, TempPos);
					float fDistance = 1400.0f;
					TempPos[0] += -fDistance * sinf( o->Angle[2]*Q_PI/180.0f);
					TempPos[1] += fDistance * cosf( o->Angle[2]*Q_PI/180.0f);
					CreateJoint( MODEL_SPEARSKILL, TempPos, TempPos, o->Angle, 2, o, 40.0f);
				}
			}
			if ( c->AttackTime <= 8)
			{	// 기 모일 곳 위치
				vec3_t Position2 = { 0.0f, 0.0f, 0.0f};
				b->TransformPosition(o->BoneTransform[c->Weapon[Hand].LinkBone],Position2,o->m_vPosSword,true);

				float fDistance = 300.0f;
				o->m_vPosSword[0] += fDistance * sinf( o->Angle[2]*Q_PI/180.0f);
				o->m_vPosSword[1] += -fDistance * cosf( o->Angle[2]*Q_PI/180.0f);
			}
			if ( 6 <= c->AttackTime && c->AttackTime <= 12)
			{	// 꼬깔 만들기
				vec3_t Position;

				//memcpy( Position, o->Position, sizeof ( vec3_t));
				vec3_t Position2 = { 0.0f, 0.0f, 0.0f};

				b->TransformPosition(o->BoneTransform[c->Weapon[Hand].LinkBone],Position2,Position,true);

				float fDistance = 100.0f + ( float)( c->AttackTime - 8) * 10.0f;
				Position[0] += fDistance * sinf( o->Angle[2]*Q_PI/180.0f);
				Position[1] += -fDistance * cosf( o->Angle[2]*Q_PI/180.0f);
				//Position[2] += 110.0f;
				vec3_t Light = { 1.0f, 1.0f, 1.0f};
				CreateEffect( MODEL_SPEAR, Position, o->Angle, Light, 1, o);
				CreateEffect( MODEL_SPEAR, Position, o->Angle, Light, 1, o);

				if(c->TargetCharacter != -1)
				{
					CHARACTER *tc = &CharactersClient[c->TargetCharacter];
					if ( c->TargetCharacter != -1)
					{
						OBJECT *to = &tc->Object;
						if ( 10 <= c->AttackTime && to->Live)
						{
							//PlayBuffer( SOUND_THUNDER01);
							to->m_byHurtByOneToOne = 35;
						}
					}
				}
			}
			if ( c->AttackTime>=12 )
			{
				c->AttackTime = g_iLimitAttackTime;
			}
		}
		break;
	case AT_SKILL_SPEAR:	// 창찌르기
		{
			BMD *b = &Models[o->Type];

			vec3_t p;
			if ( c->AttackTime == 10)
			{
				PlayBuffer( SOUND_RIDINGSPEAR);
			}
			else if ( c->AttackTime == 4)
			{	// 준비동작
				vec3_t Light = { 1.0f, 1.0f, .5f};
				vec3_t Position2 = { 0.0f, 0.0f, 0.0f};
				b->TransformPosition(o->BoneTransform[c->Weapon[Hand].LinkBone],Position2,p,true);
				CreateEffect( MODEL_SPEAR+1, p, o->Angle, Light, c->Weapon[Hand].Type, o);
				//CreateEffect(BITMAP_MAGIC+1,o->Position,o->Angle,Light,4,o);
			}
			else if ( 8 == c->AttackTime)
			{	// 꼬깔 만들기
				vec3_t Position;
				memcpy( Position, o->Position, sizeof ( vec3_t));
				Position[0] += 50.0f * sinf( o->Angle[2]*Q_PI/180.0f);
				Position[1] += -50.0f * cosf( o->Angle[2]*Q_PI/180.0f);
				Position[2] += 110.0f;
				vec3_t Light = { 1.0f, 1.0f, 1.0f};
				CreateEffect( MODEL_SPEAR, Position, o->Angle, Light, 0, o);
				CreateEffect( MODEL_SPEAR, Position, o->Angle, Light, 0, o);
			}
			if ( 13 <= c->AttackTime && c->AttackTime <= 14)
			{	// 현란한 창술
				for ( int i = 0; i < 3; ++i)
				{
					vec3_t Position;
					memcpy( Position, o->Position, sizeof ( vec3_t));
					Position[0] += 145.0f * sinf( o->Angle[2]*Q_PI/180.0f);
					Position[1] += -145.0f * cosf( o->Angle[2]*Q_PI/180.0f);
					Position[2] += 110.0f;
					//vec3_t Light = { .6f, .6f, .2f};
					vec3_t Light = { .3f, .3f, .3f};
					if ( c->AttackTime == 11)
					{
						/*Light[0] = 1.0f;
						Light[1] = 0.5f;
						Light[2] = 0.5f;*/
					}
					else
					{
						Position[0] += ( rand() % 60 - 30);
						Position[1] += ( rand() % 60 - 30);
						Position[2] += ( rand() % 60 - 30);
					}
					CreateEffect( MODEL_SPEARSKILL, Position, o->Angle, Light, c->Weapon[Hand].Type, o);
				}
			}
		}
		break;

    case    AT_SKILL_PIERCING:
        if( o->Type==MODEL_PLAYER && o->CurrentAction>=PLAYER_ATTACK_FIST && o->CurrentAction<=PLAYER_RIDE_SKILL )
        {
            if( o->AnimationFrame>=5.f )
            {
                o->PriorAnimationFrame = 4.f;
                o->AnimationFrame      = 5.f;
            }
        }

        if( c->AttackTime==3 )  //  氣 모으기.
        {
			CreateEffect(BITMAP_GATHERING,o->Position,o->Angle,o->Light,0,o);
		    PlayBuffer(SOUND_PIERCING,o);
        }
        g_iLimitAttackTime = 5;
        break;

#ifdef PJH_SEASON4_MASTER_RANK4
	case AT_SKILL_BLOOD_ATT_UP:
	case AT_SKILL_BLOOD_ATT_UP+1:
	case AT_SKILL_BLOOD_ATT_UP+2:
	case AT_SKILL_BLOOD_ATT_UP+3:
	case AT_SKILL_BLOOD_ATT_UP+4:
#endif //PJH_SEASON4_MASTER_RANK4
    case AT_SKILL_REDUCEDEFENSE:    //  마검사 방어력 감소.
        if(o->Type == MODEL_PLAYER
#ifndef CSK_FIX_DUEL_N_PK_SKILL		// 정리할 때 지워야 하는 소스	

#ifdef YDG_ADD_SKILL_RIDING_ANIMATIONS
			&& (o->CurrentAction==PLAYER_ATTACK_SKILL_WHEEL || o->CurrentAction==PLAYER_ATTACK_SKILL_WHEEL_UNI
			|| o->CurrentAction==PLAYER_ATTACK_SKILL_WHEEL_DINO || o->CurrentAction==PLAYER_ATTACK_SKILL_WHEEL_FENRIR))
#else	// YDG_ADD_SKILL_RIDING_ANIMATIONS
			&& o->CurrentAction==PLAYER_ATTACK_SKILL_WHEEL
#endif	// YDG_ADD_SKILL_RIDING_ANIMATIONS

#endif //! CSK_FIX_DUEL_N_PK_SKILL	// 정리할 때 지워야 하는 소스
			)
        {
#ifdef CSK_FIX_DUEL_N_PK_SKILL
#ifdef YDG_ADD_SKILL_RIDING_ANIMATIONS
			SetActionBloodAttack(c, o);
#else // YDG_ADD_SKILL_RIDING_ANIMATIONS
			SetAction(o, PLAYER_ATTACK_SKILL_WHEEL);
#endif // YDG_ADD_SKILL_RIDING_ANIMATIONS
#endif // CSK_FIX_DUEL_N_PK_SKILL

            if( c->AttackTime >= 1 && c->AttackTime <= 2 )
            {
                vec3_t Angle;
                Vector(1.f,0.f,0.f,Angle);
		        CreateEffect(BITMAP_GATHERING,o->Position,o->Angle,o->Light,1,o);
            }

            if( o->AnimationFrame>=3.f )
            {
				o->PKKey = getTargetCharacterKey( c, SelectedCharacter );

				PlayBuffer( SOUND_SKILL_SWORD3);

#ifdef PJH_FIX_BLOOD_ATTCK
				if(iSkill == AT_SKILL_BLOOD_ATT_UP+4)
				{
					CreateEffect(BITMAP_SWORD_FORCE, o->Position, o->Angle, o->Light,1,o,o->PKKey,FindHotKey( iSkill ));
				}
				else
#endif //PJH_FIX_BLOOD_ATTCK
				{					
					CreateEffect(BITMAP_SWORD_FORCE, o->Position, o->Angle, o->Light,0,o,o->PKKey,FindHotKey( iSkill ));
				}

                float AttackSpeed1 = CharacterAttribute->AttackSpeed*0.004f;//  

                Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_SKILL_WHEEL ].PlaySpeed = 0.54f+AttackSpeed1;//0.44회오리베기
#ifdef YDG_ADD_SKILL_RIDING_ANIMATIONS
				Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_SKILL_WHEEL_UNI   ].PlaySpeed = 0.54f+AttackSpeed1;//0.44회오리베기
				Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_SKILL_WHEEL_DINO  ].PlaySpeed = 0.54f+AttackSpeed1;//0.44회오리베기
				Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_SKILL_WHEEL_FENRIR].PlaySpeed = 0.54f+AttackSpeed1;//0.44회오리베기
#endif	// YDG_ADD_SKILL_RIDING_ANIMATIONS

                c->AttackTime = 15;
            }
        }
        g_iLimitAttackTime = 15;
        break;

#ifdef PJH_SEASON4_MASTER_RANK4
	case AT_SKILL_POWER_SLASH_UP:
	case AT_SKILL_POWER_SLASH_UP+1:
	case AT_SKILL_POWER_SLASH_UP+2:
	case AT_SKILL_POWER_SLASH_UP+3:
	case AT_SKILL_POWER_SLASH_UP+4:
#endif //PJH_SEASON4_MASTER_RANK4
    case AT_SKILL_ICE_BLADE:
		if ( o->Type==MODEL_PLAYER && o->CurrentAction==PLAYER_ATTACK_TWO_HAND_SWORD_TWO)
		{
			vec3_t Angle;


			VectorCopy ( o->Angle, Angle );

			Angle[2] -= 40.f;
			CreateEffect(MODEL_MAGIC2,o->Position,Angle,o->Light,2,o);
			Angle[2] += 20.f;
			CreateEffect(MODEL_MAGIC2,o->Position,Angle,o->Light,2,o);
			Angle[2] += 20.f;
			CreateEffect(MODEL_MAGIC2,o->Position,Angle,o->Light,2,o);
			Angle[2] += 20.f;
			CreateEffect(MODEL_MAGIC2,o->Position,Angle,o->Light,2,o);
			Angle[2] += 20.f;
			CreateEffect(MODEL_MAGIC2,o->Position,Angle,o->Light,2,o);

			PlayBuffer( SOUND_SKILL_SWORD3);

			c->AttackTime = 15;
		}
        g_iLimitAttackTime = 15;
		break;

	case AT_SKILL_BLAST_HELL:
        if ( o->AnimationFrame>=14.f && o->Type==MODEL_PLAYER && o->CurrentAction==PLAYER_SKILL_HELL_START )
        {
            c->AttackTime = 15;
        }
		break;
	case AT_SKILL_LIFE_UP:
	case AT_SKILL_LIFE_UP+1:
	case AT_SKILL_LIFE_UP+2:
	case AT_SKILL_LIFE_UP+3:
	case AT_SKILL_LIFE_UP+4:
    case    AT_SKILL_VITALITY:  //  생명력증가.
        if( c->AttackTime>9 && o->Type==MODEL_PLAYER && o->CurrentAction==PLAYER_SKILL_VITALITY )
        {
            c->AttackTime = 15;
        }
        break;
    case    AT_SKILL_IMPROVE_AG:    //  AG증가 속도 증가.
        if( o->AnimationFrame>=5.f && ((o->Type==MODEL_PLAYER && o->CurrentAction>=PLAYER_ATTACK_FIST && o->CurrentAction<=PLAYER_RIDE_SKILL) ||
 	           ((o->Type>=MODEL_MONSTER01 && o->Type<MODEL_MONSTER_END) && o->CurrentAction>=MONSTER01_ATTACK1 && o->CurrentAction<=MONSTER01_ATTACK2)))
        {
            c->AttackTime = 15;
        }
        break;
    case AT_SKILL_STRONG_PIER:
        if ( o->AnimationFrame>=3.f && o->Type==MODEL_PLAYER && ( o->CurrentAction==PLAYER_ATTACK_STRIKE || o->CurrentAction==PLAYER_ATTACK_RIDE_STRIKE 
			|| o->CurrentAction==PLAYER_FENRIR_ATTACK_DARKLORD_STRIKE	//^ 펜릴 스킬 관련
			) )
        {
            c->AttackTime = 15;
        }
        break;
	case AT_SKILL_FIRE_BUST_UP:
	case AT_SKILL_FIRE_BUST_UP+1:
	case AT_SKILL_FIRE_BUST_UP+2:
	case AT_SKILL_FIRE_BUST_UP+3:
	case AT_SKILL_FIRE_BUST_UP+4:
    case    AT_SKILL_LONGPIER_ATTACK:
        if ( o->AnimationFrame>=3.f && o->Type==MODEL_PLAYER && ( o->CurrentAction==PLAYER_ATTACK_STRIKE || o->CurrentAction==PLAYER_ATTACK_RIDE_STRIKE 
			|| o->CurrentAction==PLAYER_FENRIR_ATTACK_DARKLORD_STRIKE	//^ 펜릴 스킬 관련
			) )
        {
            c->AttackTime = 15;
        }
        break;
    case    AT_SKILL_THUNDER_STRIKE:
        if ( o->AnimationFrame>=5.5f && o->Type==MODEL_PLAYER && ( o->CurrentAction==PLAYER_SKILL_FLASH || o->CurrentAction==PLAYER_ATTACK_RIDE_ATTACK_FLASH 
			|| o->CurrentAction == PLAYER_FENRIR_ATTACK_DARKLORD_FLASH	//^ 펜릴 스킬 관련
			) )
        {
            c->AttackTime = 15;
        }
        else
        {
            c->AttackTime = 10;
        }
        break;
	case AT_SKILL_ASHAKE_UP:
	case AT_SKILL_ASHAKE_UP+1:
	case AT_SKILL_ASHAKE_UP+2:
	case AT_SKILL_ASHAKE_UP+3:
	case AT_SKILL_ASHAKE_UP+4:
    case    AT_SKILL_DARK_HORSE:    //  다크홀스 스킬.
        if ( o->AnimationFrame>=5.f && o->Type==MODEL_PLAYER && o->CurrentAction==PLAYER_ATTACK_DARKHORSE )
        {
            c->AttackTime = 15;
        }
        break;
    case    AT_SKILL_PARTY_TELEPORT://  파티원 소환.
        if ( c->AttackTime>5 && o->Type==MODEL_PLAYER && ( o->CurrentAction==PLAYER_ATTACK_TELEPORT || o->CurrentAction==PLAYER_ATTACK_RIDE_TELEPORT 
				|| o->CurrentAction == PLAYER_FENRIR_ATTACK_DARKLORD_TELEPORT	//^ 펜릴 캐릭터 에니메이션
			) )
        {
            c->AttackTime = 15;
        }
        break;
    case    AT_SKILL_RIDER: //  디노란트 공격.
        if( o->AnimationFrame>=5.f && o->Type==MODEL_PLAYER && ( o->CurrentAction==PLAYER_SKILL_RIDER || o->CurrentAction==PLAYER_SKILL_RIDER_FLY ) )
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
        if( o->AnimationFrame>=3.5f )
        {
            c->AttackTime = 15;
        }
        break;

    case    AT_SKILL_RUSH :
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
	case AT_SKILL_OCCUPY:
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
        //  칼끝에서 불을 나게 한다.
        if ( o->AnimationFrame>5.f )
        {
            c->AttackTime = 15;
        }
        //  움직일때 불꽃이 나게 한다.
		{
            vec3_t Position;
            vec3_t	Angle;

			VectorCopy ( o->Position, Position );
			Position[0] += rand()%30-15.f;
			Position[1] += rand()%30-15.f;
			Position[2] += 20.f;
			for(int i=0;i<4;i++)
			{
				Vector ( (float)(rand()%60+60+90), 0.f, o->Angle[2], Angle );
				CreateJoint ( BITMAP_JOINT_SPARK, Position, Position, Angle );
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
				if(iSkill==AT_SKILL_OCCUPY)
				{
					CreateParticle(BITMAP_FIRE, Position, Angle, o->Light, 18, 1.5f);
				}
				else
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
				CreateParticle ( BITMAP_FIRE, Position, Angle, o->Light, 2, 1.5f );
			}
		}
        break;
    case    AT_SKILL_ONEFLASH:
        if ( o->AnimationFrame>5.f )
        {
            CreateJoint ( BITMAP_FLARE, o->Position, o->Position, o->Angle, 23, NULL, 40.f, 0 );
            CreateJoint ( BITMAP_FLARE, o->Position, o->Position, o->Angle, 23, NULL, 40.f, 1 );
            CreateJoint ( BITMAP_FLARE, o->Position, o->Position, o->Angle, 23, NULL, 40.f, 4 );
            c->AttackTime = 15;

            PlayBuffer ( SOUND_BCS_ONE_FLASH );
        }
        else if ( o->AnimationFrame>2.3f && o->AnimationFrame<2.6f )
        {
            CreateJoint ( BITMAP_FLARE, o->Position, o->Position, o->Angle, 23, NULL, 40.f, 2 );
            CreateJoint ( BITMAP_FLARE, o->Position, o->Position, o->Angle, 23, NULL, 40.f, 3 );

            PlayBuffer ( SOUND_BCS_ONE_FLASH );
        }
        g_iLimitAttackTime = 15;
        break;

    case AT_SKILL_SPACE_SPLIT:      //  공간 가르기.
        if ( o->AnimationFrame >= 3.f 
			&& o->Type == MODEL_PLAYER 
			&& ( o->CurrentAction == PLAYER_ATTACK_STRIKE || o->CurrentAction == PLAYER_ATTACK_RIDE_STRIKE || o->CurrentAction == PLAYER_FENRIR_ATTACK_DARKLORD_STRIKE) 
			)
        {
            c->AttackTime = 15;
        }
        break;

    case    AT_SKILL_DEATH_CANNON :
        if ( o->AnimationFrame>=3.f && o->Type==MODEL_PLAYER && o->CurrentAction==PLAYER_ATTACK_DEATH_CANNON )
        {
            c->AttackTime = 15;
        }
        break;
#ifdef YDG_ADD_SKILL_FLAME_STRIKE
	case AT_SKILL_FLAME_STRIKE:
        {
            c->AttackTime = 15;
        }
		break;
#endif	// YDG_ADD_SKILL_FLAME_STRIKE
#ifdef YDG_ADD_SKILL_GIGANTIC_STORM
	case AT_SKILL_GIGANTIC_STORM:
        if ( o->AnimationFrame>7.f )
        {
            c->AttackTime = 15;	// 스킬 이펙트 딜레이: 7 프레임 이상이면 15로 설정->스킬 이펙트 발동
        }
		break;
#endif	// YDG_ADD_SKILL_GIGANTIC_STORM
#ifdef YDG_ADD_SKILL_LIGHTNING_SHOCK
#ifdef PJH_ADD_MASTERSKILL
		case AT_SKILL_LIGHTNING_SHOCK_UP:
		case AT_SKILL_LIGHTNING_SHOCK_UP+1:
		case AT_SKILL_LIGHTNING_SHOCK_UP+2:
		case AT_SKILL_LIGHTNING_SHOCK_UP+3:
		case AT_SKILL_LIGHTNING_SHOCK_UP+4:
#endif
	case AT_SKILL_LIGHTNING_SHOCK:
        {
            c->AttackTime = 15;
        }
		break;
#endif	// YDG_ADD_SKILL_LIGHTNING_SHOCK
#ifdef CSK_ADD_SKILL_BLOWOFDESTRUCTION
	case AT_SKILL_BLOW_OF_DESTRUCTION:
		{
			c->AttackTime = 15;
		}
		break;
#endif // CSK_ADD_SKILL_BLOWOFDESTRUCTION
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
	case AT_SKILL_ATT_UP_OURFORCES:
	case AT_SKILL_HP_UP_OURFORCES:
	case AT_SKILL_DEF_UP_OURFORCES:
		{
			c->AttackTime = 15;
		}
		break;
	case AT_SKILL_DRAGON_KICK:
		{
			c->AttackTime = 1;
		}
		break;
	case AT_SKILL_GIANTSWING:
	case AT_SKILL_STAMP:
		{
			o->m_sTargetIndex = c->TargetCharacter;
		}
		break;
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
    default :
        if ( o->AnimationFrame>=1.f && o->Type==MODEL_PLAYER && o->CurrentAction==PLAYER_ATTACK_SKILL_FURY_STRIKE )
        {
            c->AttackTime = 15;
        }
        else if( o->AnimationFrame>=5.f && ((o->Type==MODEL_PLAYER && o->CurrentAction>=PLAYER_ATTACK_FIST && o->CurrentAction<=PLAYER_RIDE_SKILL) ||
 	           ((o->Type>=MODEL_MONSTER01 && o->Type<MODEL_MONSTER_END) && o->CurrentAction>=MONSTER01_ATTACK1 && o->CurrentAction<=MONSTER01_ATTACK2)))
        {

#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
			int RightType = CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
			int LeftType = CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type;

			
			if( c->AttackTime>=1 && LeftType == ITEM_BOW+21 && o->Type==MODEL_PLAYER)
			{
				for(int i = 0; i < 20; i++)
				{
					CreateParticle(BITMAP_SPARK+1,o->Position,o->Angle,o->Light,12, 2.0f);
				}
			}

		    c->AttackTime = 15;
        }

        break;
    }
    return true;
}


//extern CPhysicsManager g_PhysicsManager;

void DeleteCloth( CHARACTER *c, OBJECT *o, PART_t *p2)
{
	if ( o && o->m_pCloth)
	{
		CPhysicsCloth *pCloth = ( CPhysicsCloth*)o->m_pCloth;
		for ( int i = 0; i < o->m_byNumCloth; ++i)
		{
			pCloth[i].Destroy();
		}
		delete [] pCloth;
		o->m_pCloth = NULL;
		o->m_byNumCloth = 0;
	}

	if ( c)
	{
		for(int i=0;i<MAX_BODYPART;i++)
		{
			PART_t *p = &c->BodyPart[i];
			if ( p->m_pCloth[0])
			{
				for ( int i = 0; i < p->m_byNumCloth; ++i)
				{
					CPhysicsCloth *pCloth = ( CPhysicsCloth*)p->m_pCloth[i];
					pCloth->Destroy();
					delete pCloth;
					p->m_pCloth[i] = NULL;
				}
			}
		}
	}

	if ( p2)
	{
		if ( p2->m_pCloth[0])
		{
			for ( int i = 0; i < p2->m_byNumCloth; ++i)
			{
				CPhysicsCloth *pCloth = ( CPhysicsCloth*)p2->m_pCloth[i];
				pCloth->Destroy();
				delete pCloth;
				p2->m_pCloth[i] = NULL;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////
//  떨어지는 캐릭터 처리.
//////////////////////////////////////////////////////////////////////////
void FallingCharacter ( CHARACTER* c, OBJECT* o )
{
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
	if(c == NULL)	return;
	if(o == NULL)	return;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
    float Matrix[3][4];
    vec3_t  Position, p;

    Vector ( 0.f, 0.f, 0.f, Position );
    Vector ( 0.f, o->Direction[1], 0.f, p );
	AngleMatrix ( o->m_vDownAngle, Matrix );
    VectorRotate ( p, Matrix, Position );
	
    o->Direction[1]     += o->Direction[0];
    o->Gravity          += o->Velocity;
    o->Velocity         -= o->Direction[2];
    o->Angle[0]         -= 5.f;

    o->Position[0] = o->m_vDeadPosition[0] + Position[0];
    o->Position[1] = o->m_vDeadPosition[1] + Position[1];
    o->Position[2] = o->m_vDeadPosition[2] + o->Gravity;
}

//////////////////////////////////////////////////////////////////////////
//  밀리는 캐릭터 처리 ( 회전, 점프(공)등등 )
//////////////////////////////////////////////////////////////////////////
void  PushingCharacter ( CHARACTER* c, OBJECT* o )
{
	if(c->StormTime > 0)
	{
		o->Angle[2] += c->StormTime*10;
		c->StormTime--;
	}
	if(c->JumpTime > 0)
	{
		float Speed = 0.2f;
		if ( o->Type == MODEL_MONSTER01+52 )	// 야누스
		{
			Speed = 0.07f;
		}
        if ( InChaosCastle()==true )
        {
		    o->Position[0] += (((float)c->TargetX)*TERRAIN_SCALE-o->Position[0])*Speed;
		    o->Position[1] += (((float)c->TargetY)*TERRAIN_SCALE-o->Position[1])*Speed;
            c->JumpTime++;
		    if ( c->JumpTime>15 )
            {
                SetPlayerStop ( c );
                
                o->Position[0] = c->TargetX*TERRAIN_SCALE;
                o->Position[1] = c->TargetY*TERRAIN_SCALE;

                c->PositionX = c->TargetX;
                c->PositionY = c->TargetY;
                
                c->JumpTime = 0;
            }
        }
        else
        {
		    o->Position[0] += (((float)c->TargetX+0.5f)*TERRAIN_SCALE-o->Position[0])*Speed;
		    o->Position[1] += (((float)c->TargetY+0.5f)*TERRAIN_SCALE-o->Position[1])*Speed;
     	    if(o->Type != MODEL_BALL)//공
	      	    o->Position[2] = RequestTerrainHeight(o->Position[0],o->Position[1]);
		    if(c->JumpTime++ > 15)
		    {
			    if ( o->Type == MODEL_MONSTER01+52 )	// 야누스
				    SetPlayerStop( c);

                c->JumpTime = 0;
		    }
        }
	}
	if(o->Type == MODEL_BALL)//공
	{ 
		o->Position[2] += o->Gravity;
		o->Gravity -= 6.f;
		float Height = RequestTerrainHeight(o->Position[0],o->Position[1]) + 30.f;
		if(o->Position[2] < Height)
		{
			o->Position[2] = Height;
			o->Gravity = -o->Gravity*0.4f;
		}
		VectorAdd(o->Angle,o->Direction,o->Angle);
		VectorScale(o->Direction,0.8f,o->Direction);
	}
}


//////////////////////////////////////////////////////////////////////////
//  죽는 캐릭터 처리.
//////////////////////////////////////////////////////////////////////////
void DeadCharacterBuff ( OBJECT* o )
{
	// _buffwani_
	//  죽은 사람의 경우 아이스에로우나 스턴의 속성은 제거한다.
	g_CharacterUnRegisterBuff(o, eDeBuff_Stun );
	g_CharacterUnRegisterBuff(o, eBuff_Cloaking );
	g_CharacterUnRegisterBuff(o, eDeBuff_Harden );
	g_CharacterUnRegisterBuff(o, eDeBuff_Sleep );
}

void DeadCharacter ( CHARACTER* c, OBJECT* o, BMD* b )
{
    //  죽음을 처리한다.
	if ( c->Dead<=0 ) return;

    //  죽은 사람의 경우 아이스에로우나 스턴의 속성은 제거한다.
	DeadCharacterBuff( o );

	if(o->Type==MODEL_MONSTER01+32 && World==WD_7ATLANSE)
	{
		c->Rot += 0.05f;
	}
	else if(o->Type==MODEL_MONSTER01+64)
	{
		if (o->LifeTime >= 100) c->Rot += 0.01f;
		else c->Rot += 0.02f;
	}
	else
		c->Rot += 0.02f;
	float RotTime = 1.f;
	if(c->Rot >= RotTime) 
	{
		if(o->Type != MODEL_MONSTER01+116)
		{
			o->Alpha = 1.f-(c->Rot-RotTime);
		}

		if(o->Alpha >= 0.01f)
     		o->Position[2] -= 0.4f;
		else if(c != Hero)
		{	// 죽은 몹이 투명해지다가 사라지는 지점
      		o->Live = false;
			c->m_byDieType = 0;
			o->m_bActionStart = false;
			o->m_bySkillCount = 0;

			BoneManager::UnregisterBone(c);
		}
		DeleteCloth( c, o);
	}

    //  블러드캐슬에서 떨어지는 연출.
    if ( InBloodCastle() == true && o->m_bActionStart )
    {
        FallingCharacter ( c, o );
    }
    else if ( InChaosCastle()==true )
    {
        int startDeadTime = 25;
        if ( o->m_bActionStart )
        {
            FallingCharacter ( c, o );
            startDeadTime = 15;
        }
        if ( c->Dead<=startDeadTime && c->Dead>=startDeadTime-10 && (c->Dead%2) )
        {
            //  폭발한다.
            vec3_t Position;

            VectorCopy ( o->Position, Position );

            Position[0] += rand()%160-80.f;
            Position[1] += rand()%160-80.f;
            Position[2] += rand()%160-80.f + 50.f;
            CreateBomb ( Position, true );
        }
    }
	else
	{
		switch ( c->m_byDieType )
		{
		case AT_SKILL_BLAST_HELL :
		case AT_SKILL_COMBO :
			if ( c->Dead<15 )
			{
				o->Direction[1] -= o->Velocity;
				o->Velocity += 1.f;

				if ( o->Direction[1]<0 )
				{
					o->Direction[1] = 0.f;
				}
			
				MoveParticle(o,o->HeadAngle);
			}
			
			if ( c->Dead<=30 && c->m_byDieType==AT_SKILL_BLAST_HELL )
			{
				vec3_t Light, p, Position;
				Vector ( 0.3f, 0.3f, 1.f, Light );
				Vector ( 0.f, 0.f, 0.f, p );
				for( int i=0;i<10;i++)
				{
					b->TransformPosition(o->BoneTransform[rand()%32],p,Position,true);
					CreateParticle(BITMAP_LIGHT,Position,o->Angle,Light, 5, 0.5f+(rand()%100)/50.f);
				}
			}
			break;
		}
	}
    if(SceneFlag==MAIN_SCENE && (World==WD_7ATLANSE
#ifdef YDG_ADD_MAP_DOPPELGANGER3
		|| World == WD_67DOPPLEGANGER3
#endif	// YDG_ADD_MAP_DOPPELGANGER3
		))
	{
		for(int i=0;i<4;i++)
		{
			vec3_t Position;
			Vector((float)(rand()%128-64),(float)(rand()%128-64),(float)(rand()%256),Position);
			VectorAdd(Position,o->Position,Position);
			CreateParticle(BITMAP_BUBBLE,Position,o->Angle,o->Light);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//  정지된 시간을 계산한다.
//////////////////////////////////////////////////////////////////////////
void CalcStopTime ( void )
{
    if ( InChaosCastle()==false ) return;

    //  이전 위치와 현재 위치를 검사한다.
    if ( (Hero->PositionX)!=g_iOldPositionX || (Hero->PositionY)!=g_iOldPositionY )
    {
        g_iOldPositionX = (Hero->PositionX);
        g_iOldPositionY = (Hero->PositionY);

        g_fStopTime = WorldTime;
        return;
    }

    //  정지되 있는 시간을 계산한다.
    float fStopTime = ((WorldTime - g_fStopTime)/CLOCKS_PER_SEC);
	if ( fStopTime>=10 )
    {
        int index = TERRAIN_INDEX_REPEAT( g_iOldPositionX, g_iOldPositionY );

        if ( (TerrainWall[index]&TW_NOGROUND)==TW_NOGROUND )
        {
            SendRequestCheckPosition ( g_iOldPositionX, g_iOldPositionY );
        }
        g_fStopTime = WorldTime;
    }
}

//////////////////////////////////////////////////////////////////////////
//  주인공 자신의 능력(변화되는)을 계산한다.
//////////////////////////////////////////////////////////////////////////
void HeroAttributeCalc ( CHARACTER* c )
{
    if ( c!=Hero ) return;

	if(CharacterAttribute->AbilityTime[0] > 0)
	{
		CharacterAttribute->AbilityTime[0]--;
	}
	if(CharacterAttribute->AbilityTime[0] == 0)
	{
		CharacterAttribute->Ability &= ( ~ABILITY_FAST_ATTACK_SPEED);

        //  공격속도 계산.
        CharacterMachine->CalculateAttackSpeed();
	}
	if(CharacterAttribute->AbilityTime[1] > 0)
	{
		CharacterAttribute->AbilityTime[1]--;
	}
	if(CharacterAttribute->AbilityTime[1] == 0)
	{
		CharacterAttribute->Ability &= ( ~ABILITY_PLUS_DAMAGE);

        //  공격력 계산.
        g_csItemOption.ClearListOnOff();
        CharacterMachine->CalculateDamage();
		CharacterMachine->CalculateMagicDamage();
		CharacterMachine->CalculateCurseDamage();
	}
	if(CharacterAttribute->AbilityTime[2] > 0)
	{
		CharacterAttribute->AbilityTime[2]--;
	}
	if(CharacterAttribute->AbilityTime[2] == 0)
	{
        CharacterAttribute->Ability &= ( ~ABILITY_FAST_ATTACK_SPEED2);
        //  공격속도 계산.
        CharacterMachine->CalculateAttackSpeed();
	}

}


//////////////////////////////////////////////////////////////////////////
//  NPC 채팅표시 처리
//////////////////////////////////////////////////////////////////////////
void OnlyNpcChatProcess ( CHARACTER* c, OBJECT* o )
{
    if ( o->Kind==KIND_NPC && (rand()%2)==0 )
    {
        switch ( o->Type )
        {
#ifndef LIMIT_CHAOS_MIX
        case MODEL_MERCHANT_GIRL :  
            if ( battleCastle::InBattleCastle()==false )
            {
#ifdef BATTLE_SOCCER_ADD_NPC_DIALOG
				CreateChat ( c->ID, GlobalText[2052], c ); 
#else
				CreateChat ( c->ID, GlobalText[1974], c ); 
#endif
            }
            break;
        case MODEL_ELF_WIZARD :
#ifdef BATTLE_SOCCER_ADD_NPC_DIALOG
			CreateChat ( c->ID, GlobalText[2051], c ); 
#else
			CreateChat ( c->ID, GlobalText[1975], c ); 
#endif
			break;
        case MODEL_MASTER :         
#ifdef _PVP_MURDERER_HERO_ITEM
			if (c->MonsterIndex != 227)	// 카오 상인
#endif	// _PVP_MURDERER_HERO_ITEM
#ifdef BATTLE_SOCCER_ADD_NPC_DIALOG
				CreateChat ( c->ID, GlobalText[2053], c ); 
#else
				CreateChat ( c->ID, GlobalText[1976], c ); 
#endif
				break;
		case MODEL_PLAYER:
			if (c->MonsterIndex == 257) // 엘프 도우미
				CreateChat ( c->ID, GlobalText[1827], c );
			break;
#ifdef MYSTERY_BEAD
		case MODEL_MIX_NPC:
			CreateChat( c->ID, GlobalText[1841], c );
			break;
#endif //HELPER_ELF
#endif //LIMIT_CHAOS_MIX
        }

    }
}


//////////////////////////////////////////////////////////////////////////
//  NPC 애니메이션 정지시 세팅.
//////////////////////////////////////////////////////////////////////////
void PlayerNpcStopAnimationSetting ( CHARACTER* c, OBJECT* o )
{
    int action = rand()%100;
    
    if( o->CurrentAction!=PLAYER_STOP_MALE )
    {
        SetAction(&c->Object,PLAYER_STOP_MALE);
    }
    else
    {
#ifdef YDG_MOD_GOLDEN_ARCHER_ANGEL_TEXT
		// 황금궁수 전용
		if(action<80)
		{
			SetAction(&c->Object,PLAYER_STOP_MALE);
		}
		else if(action<85)
		{
			SetAction(&c->Object,PLAYER_CLAP1); //   박수.
		}
		else if(action<90)
		{
			SetAction(&c->Object,PLAYER_CHEER1); //  환호.
		}
		else if(action<95)
		{               
			SetAction(&c->Object,PLAYER_SEE1);  //  
		}
		else if(action<100)
		{
			SetAction(&c->Object,PLAYER_UNKNOWN1);  //  
		}

		int TextIndex = 0;
		if ( World == WD_2DEVIAS )
		{
#if SELECTED_LANGUAGE == LANGUAGE_KOREAN
			int iTextIndices[] = { 904, 905, 1148 };	// 국내 프리미엄플러스 수호정령,데몬 안내문구
			int iNumTextIndices = 3;
// 			int iTextIndices[] = { 1167, 1168 };		// 복주머니 이벤트
// 			int iNumTextIndices = 2;
#else	// SELECTED_LANGUAGE == LANGUAGE_KOREAN
			int iTextIndices[] = { 904, 905 };			// 해외 황금궁수 안내문구
			int iNumTextIndices = 2;
#endif	// SELECTED_LANGUAGE == LANGUAGE_KOREAN

			static int s_iTextIndex = iTextIndices[0];
			if (o->CurrentAction != o->PriorAction)
				s_iTextIndex = iTextIndices[rand()%iNumTextIndices];
			TextIndex = s_iTextIndex;
		}
		else if ( World == WD_0LORENCIA )
		{
#ifdef LENA_EXCHANGE_ZEN
			int iTextIndices[] = { 723, 724 };		// 레나 등록
			int iNumTextIndices = 2;
#else	// LENA_EXCHANGE_ZEN
			int iTextIndices[] = { 823 };			// 스톤 등록
			int iNumTextIndices = 1;
#endif	// LENA_EXCHANGE_ZEN

			static int s_iTextIndex = iTextIndices[0];
			if (o->CurrentAction != o->PriorAction)
				s_iTextIndex = iTextIndices[rand()%iNumTextIndices];
			TextIndex = s_iTextIndex;
		}

		char szText[512];
		wsprintf(szText, GlobalText[TextIndex]);	// Text.txt에서는 무조건 %를 %%로 쓰기로 약속되어 있음.
		CreateChat(c->ID, szText, c);

#else	// YDG_MOD_GOLDEN_ARCHER_ANGEL_TEXT
        int TextIndex = 0;
        if(action<80)
        {
#ifdef SCRATCH_TICKET
            if ( World==WD_2DEVIAS )
                TextIndex = 904;	// "100%% 당첨 감사카드 행운의 번호를 등록하세요."
            else
                TextIndex = 0;
#endif
            if( rand()%2==0 && TextIndex )
            {
#ifdef ASG_FIX_TEXT_SCRIPT_PERCENT_TREATMENT
				char szText[512];
				wsprintf(szText, GlobalText[TextIndex]);	// Text.txt에서는 무조건 %를 %%로 쓰기로 약속되어 있음.
				CreateChat(c->ID, szText, c);
#else	// ASG_FIX_TEXT_SCRIPT_PERCENT_TREATMENT
                CreateChat ( c->ID, GlobalText[TextIndex], c);
#endif	// ASG_FIX_TEXT_SCRIPT_PERCENT_TREATMENT
            }
            SetAction(&c->Object,PLAYER_STOP_MALE);
        }
        else if(action<85)
        {
#ifdef SCRATCH_TICKET
            if ( World==WD_2DEVIAS )
                TextIndex = 904;	// "100%% 당첨 감사카드 행운의 번호를 등록하세요."
            else
                TextIndex = 0;
#endif
            if( rand()%2==0 && TextIndex )
            {
#ifdef ASG_FIX_TEXT_SCRIPT_PERCENT_TREATMENT
				char szText[512];
				wsprintf(szText, GlobalText[TextIndex]);	// Text.txt에서는 무조건 %를 %%로 쓰기로 약속되어 있음.
				CreateChat(c->ID, szText, c);
#else	// ASG_FIX_TEXT_SCRIPT_PERCENT_TREATMENT
                CreateChat ( c->ID, GlobalText[TextIndex], c);
#endif	// ASG_FIX_TEXT_SCRIPT_PERCENT_TREATMENT
            }
            SetAction(&c->Object,PLAYER_CLAP1); //   박수.
        }
        else if(action<90)
        {
#ifdef LENA_EXCHANGE_ZEN
            TextIndex = 723;
#endif
            TextIndex = 823;
#ifdef SCRATCH_TICKET
            if ( World==WD_2DEVIAS )
                TextIndex = 904;	// "100%% 당첨 감사카드 행운의 번호를 등록하세요."
            else
                TextIndex = 0;
#endif
            if( rand()%2==0 && TextIndex )
            {
#ifdef ASG_FIX_TEXT_SCRIPT_PERCENT_TREATMENT
				if (904 == TextIndex)
				{
					char szText[512];
					wsprintf(szText, GlobalText[TextIndex]);	// Text.txt에서는 무조건 %를 %%로 쓰기로 약속되어 있음.
					CreateChat(c->ID, szText, c);
				}
				else
#endif	// ASG_FIX_TEXT_SCRIPT_PERCENT_TREATMENT
					CreateChat ( c->ID, GlobalText[TextIndex], c);
            }
            
            SetAction(&c->Object,PLAYER_CHEER1); //  환호.
        }
        else if(action<95)
        {               
#ifdef LENA_EXCHANGE_ZEN
            TextIndex = 724;
#endif
            TextIndex = 0;
#ifdef SCRATCH_TICKET
            if ( World==WD_2DEVIAS )
                TextIndex = 905;	// "100%% 당첨 감사카드을 받는 방법은 뮤 홈페이지 공지사항을 확인하세요"
            else
                TextIndex = 0;
#endif
            
            if( rand()%3==0 && TextIndex )
            {
#ifdef ASG_FIX_TEXT_SCRIPT_PERCENT_TREATMENT
				if (905 == TextIndex)
				{
					char szText[512];
					wsprintf(szText, GlobalText[TextIndex]);	// Text.txt에서는 무조건 %를 %%로 쓰기로 약속되어 있음.
					CreateChat(c->ID, szText, c);
				}
				else
#endif	// ASG_FIX_TEXT_SCRIPT_PERCENT_TREATMENT
                CreateChat ( c->ID, GlobalText[TextIndex], c);
            }
            SetAction(&c->Object,PLAYER_SEE1);  //  
        }
        else if(action<100)
        {
#ifdef SCRATCH_TICKET
            if ( World==WD_2DEVIAS )
                TextIndex = 905;
            else
                TextIndex = 0;
            if( rand()%3==0 && TextIndex )
            {
#ifdef ASG_FIX_TEXT_SCRIPT_PERCENT_TREATMENT
				char szText[512];
				wsprintf(szText, GlobalText[TextIndex]);	// Text.txt에서는 무조건 %를 %%로 쓰기로 약속되어 있음.
				CreateChat(c->ID, szText, c);
#else	// ASG_FIX_TEXT_SCRIPT_PERCENT_TREATMENT
                CreateChat ( c->ID, GlobalText[TextIndex], c);
#endif	// ASG_FIX_TEXT_SCRIPT_PERCENT_TREATMENT
            }
#else
/*
#ifndef EVENT_BLOODCASTLE
#ifdef LENA_EXCHANGE_ZEN
            if( rand()%3==0 )
            {
                if( (rand()%2)==0 )
                {
                    CreateChat ( c->ID, GlobalText[723], c);
                }
                else
                {
                    CreateChat ( c->ID, GlobalText[724], c);
                }
            }
#endif
#endif
*/
#endif
            SetAction(&c->Object,PLAYER_UNKNOWN1);  //  
        }
#endif	// YDG_MOD_GOLDEN_ARCHER_ANGEL_TEXT
    }
}


//////////////////////////////////////////////////////////////////////////
//  Player 정지시 애니메이션 설정.
//////////////////////////////////////////////////////////////////////////
void PlayerStopAnimationSetting ( CHARACTER* c, OBJECT* o )
{
	if(o->CurrentAction==PLAYER_DIE1 || o->CurrentAction==PLAYER_DIE2)
	{
		if(!c->Blood)
		{
			c->Blood = true;
			CreateBlood(o);
		}
		return;
	}
		
	if( o->CurrentAction<PLAYER_WALK_MALE ||
		(o->CurrentAction>=PLAYER_PROVOCATION && o->CurrentAction<=PLAYER_CHEERS) ||
		(o->CurrentAction>=PLAYER_IDLE1_DARKHORSE && o->CurrentAction<=PLAYER_IDLE2_DARKHORSE) ||
#ifdef PBG_ADD_NEWCHAR_MONK_ANI
		((o->CurrentAction>=PLAYER_SKILL_THRUST && o->CurrentAction<=PLAYER_RAGE_FENRIR_ATTACK_RIGHT) &&
		!(o->CurrentAction >= PLAYER_RAGE_FENRIR_RUN && o->CurrentAction <= PLAYER_RAGE_FENRIR_RUN_ONE_LEFT) &&
		!(o->CurrentAction >= PLAYER_RAGE_UNI_RUN && o->CurrentAction <= PLAYER_RAGE_UNI_STOP_ONE_RIGHT)) ||
		o->CurrentAction==PLAYER_STOP_RAGEFIGHTER ||
#endif //PBG_ADD_NEWCHAR_MONK_ANI
	  ( o->CurrentAction>=PLAYER_ATTACK_FIST && o->CurrentAction<=PLAYER_SHOCK 
	  && o->CurrentAction!=PLAYER_WALK_TWO_HAND_SWORD_TWO && o->CurrentAction!=PLAYER_RUN_TWO_HAND_SWORD_TWO 
	  && o->CurrentAction!=PLAYER_FLY_RIDE && o->CurrentAction!=PLAYER_FLY_RIDE_WEAPON
	  && o->CurrentAction!=PLAYER_SKILL_HELL_BEGIN
      && o->CurrentAction!=PLAYER_DARKLORD_WALK && o->CurrentAction!=PLAYER_RUN_RIDE_HORSE 
	  && (o->CurrentAction < PLAYER_FENRIR_RUN || o->CurrentAction > PLAYER_FENRIR_RUN_ONE_LEFT_ELF) //^ 펜릴 계속 달리기 되게 하기
#ifdef PJH_SEASON4_SPRITE_NEW_SKILL_RECOVER
	  && o->CurrentAction!=PLAYER_RECOVER_SKILL
#endif //#ifdef PJH_SEASON4_SPRITE_NEW_SKILL_RECOVER
	  ))
#ifdef WORLDCUP_ADD
	  if(o->CurrentAction != PLAYER_POINT_DANCE)
#endif
		SetPlayerStop(c);

	if ( o->CurrentAction==PLAYER_SKILL_HELL_BEGIN )
	{
		o->AnimationFrame = 0;
	}
}


//////////////////////////////////////////////////////////////////////////
//  기타 애니메이션 정지시 세팅
//////////////////////////////////////////////////////////////////////////
void EtcStopAnimationSetting ( CHARACTER* c, OBJECT* o )
{
    if ( o->Type==MODEL_WARCRAFT )
    {
        o->CurrentAction = 1;
    }
    else 
    //  공성전 특별 몬스터 정지처리.
    if ( battleCastle::StopBattleCastleMonster( c, o )==true )
    {
        CharacterAnimation ( c, o );
        return;
    }
    else
	if ( World==WD_1DUNGEON && o->Type==40 )//함정
	{
		SetAction(&c->Object,0);
	}
	else if(o->Type>=MODEL_MONSTER01 && o->Type<MODEL_MONSTER_END)
	{
		if(o->CurrentAction==MONSTER01_DIE)
		{
			if(!c->Blood)
			{
				c->Blood = true;
				CreateBlood(o);
			}
			return;
		}
		else if(o->CurrentAction==MONSTER01_STOP2 || o->CurrentAction==MONSTER01_SHOCK
			|| o->CurrentAction==MONSTER01_ATTACK1 || o->CurrentAction==MONSTER01_ATTACK2
			|| o->CurrentAction==MONSTER01_ATTACK3 || o->CurrentAction==MONSTER01_ATTACK4
			|| o->CurrentAction==MONSTER01_ATTACK5
		)
		{
			SetAction(o,MONSTER01_STOP1);
		}

		if(o->CurrentAction == MONSTER01_APEAR 
			&& (o->Type == MODEL_MONSTER01+118 || o->Type == MODEL_MONSTER01+119
#ifdef CSK_RAKLION_BOSS
			|| o->Type == MODEL_MONSTER01+150	// 세루판
#endif // CSK_RAKLION_BOSS
			))
		{
#ifdef CSK_RAKLION_BOSS
			if(o->Type == MODEL_MONSTER01+150)
			{
				// 세루판의 경우 MONSTER01_APEAR 동작과 MONSTER01_STOP1 동작 사이의 위치 격차가 너무 벌어져서
				// 튀는 느낌이 듬. 동작간 보간을 안시키기 위해서 CurrentAction을 설정
				o->CurrentAction = MONSTER01_STOP1;
			}
#endif // CSK_RAKLION_BOSS	
			
			SetAction(o,MONSTER01_STOP1);
		}
	}
	else
	{
		switch(o->Type)
		{
		case MODEL_WEDDING_NPC:
			if((rand()%16<4) && o->SubType == 0)
			{
				SetAction(o,1);
				o->SubType = 1;	//. 에니메이션 백업: 연속 에니메이션 방지
			}
			else
			{
				SetAction(o,0);
				o->SubType = 0;
			}
			break;
		case MODEL_SMITH:
		case MODEL_SCIENTIST:
			if(rand()%16<12)
				SetAction(o,0);
			else
				SetAction(o,rand()%2+1);
			break;
		case MODEL_FACE:
		case MODEL_FACE+1:
		case MODEL_FACE+2:
		case MODEL_FACE+3:
		case MODEL_FACE+4:
		case MODEL_FACE+5:
#ifdef PBG_ADD_NEWCHAR_MONK
		case MODEL_FACE+6:
#endif //PBG_ADD_NEWCHAR_MONK
			break;
		case MODEL_ELBELAND_SILVIA:
		case MODEL_ELBELAND_RHEA:
			if (rand() % 5 < 4 || o->CurrentAction == 1)
				SetAction(o, 0);
			else
				SetAction(o, 1);
			break;
#ifdef ADD_NPC_DEVIN
		case MODEL_NPC_DEVIN:
			if (rand() % 5 < 4)
				SetAction(o, 0);
			else
				SetAction(o, 1);
			break;
#endif	// ADD_NPC_DEVIN

#ifdef ADD_SOCKET_MIX
		case MODEL_SEED_MASTER:
			if (rand() % 3 < 2 || o->CurrentAction != 0)
				SetAction(o, 0);
			else
			{
				SetAction(o, rand()%3+1);
			}
			break;
		case MODEL_SEED_INVESTIGATOR:
			if (rand() % 3 < 2 || o->CurrentAction == 1)
				SetAction(o, 0);
			else
				SetAction(o, 1);
			break;
#endif	// ADD_SOCKET_MIX
#ifdef PBG_ADD_LITTLESANTA_NPC
			//대기동작 1개에 모션이 3가지 있다.
		case MODEL_LITTLESANTA:
		case MODEL_LITTLESANTA+1:
		case MODEL_LITTLESANTA+2:
		case MODEL_LITTLESANTA+3:
 			if(rand()%5 < 2)				//전체의 40%는 기본동작 나머지는 모션
 				SetAction(o, 0);			//xmassanta_stand_1	기본동작
 			else
 				SetAction(o, rand()%3 + 2);	//123번중에 하나..xmassanta_idle1~3 모션(2,3,4)
			break;

		case MODEL_LITTLESANTA+4:
		case MODEL_LITTLESANTA+5:
		case MODEL_LITTLESANTA+6:
		case MODEL_LITTLESANTA+7:
  			if(rand()%5 < 2)				//전체의 40%는 기본동작 나머지는 모션
  				SetAction(o, 1);			//xmassanta_stand_2 기본동작
  			else
				SetAction(o, rand()%3 + 2);	//123번중에 하나..xmassanta_idle1~3 모션(2,3,4)
			break;
#endif //PBG_ADD_LITTLESANTA_NPC
#ifdef LDK_ADD_SANTA_NPC
			//대기 동작 3종
		case MODEL_XMAS2008_SANTA_NPC:
			if (rand() % 3 < 2 || (o->CurrentAction == 1 || o->CurrentAction == 2))
			{
				SetAction(o, 0);
			}
			else
			{
				SetAction(o, rand()%2 + 1);
			}
			break;
#endif //LDK_ADD_SANTA_NPC
#ifdef LDK_ADD_SNOWMAN_NPC
			//대기 동작 1종
		case MODEL_XMAS2008_SNOWMAN_NPC:
			SetAction(o, 0);
			break;
#endif //LDK_ADD_SNOWMAN_NPC
#ifdef LDK_ADD_GAMBLE_NPC_MOSS
		case MODEL_GAMBLE_NPC_MOSS:
#ifdef ASG_ADD_TIME_LIMIT_QUEST_NPC
		case MODEL_TIME_LIMIT_QUEST_NPC_ZAIRO:
#endif	// ASG_ADD_TIME_LIMIT_QUEST_NPC
			if( rand()%5 < 4 || o->CurrentAction == 1 )
			{
				SetAction(o, 0);
			}
			else
			{
				SetAction(o, 1);
			}
			break;
#endif //LDK_ADD_GAMBLE_NPC_MOSS
#ifdef ASG_ADD_GENS_NPC
		case MODAL_GENS_NPC_DUPRIAN:
		case MODAL_GENS_NPC_BARNERT:
			if (rand() % 5 < 4)
				SetAction(o, 0);
			else
				SetAction(o, rand()%2 + 1);
			break;
#endif	// ASG_ADD_GENS_NPC
#ifdef LDS_ADD_NPC_UNITEDMARKETPLACE
		case MODEL_UNITEDMARKETPLACE_RAUL:
		case MODEL_UNITEDMARKETPLACE_JULIA:
			if (rand() % 5 < 4)
				SetAction(o, 0);
			else
				SetAction(o, rand()%2 + 1);
			break;
		case MODEL_UNITEDMARKETPLACE_CHRISTIN:
#ifdef ASG_ADD_KARUTAN_NPC
		case MODEL_KARUTAN_NPC_REINA:
#endif	// ASG_ADD_KARUTAN_NPC
			if (rand() % 5 < 3)
				SetAction(o, 0);
			else
				SetAction(o, rand()%2 + 1);
			break;
#endif // LDS_ADD_NPC_UNITEDMARKETPLACE
#ifdef ASG_ADD_TIME_LIMIT_QUEST_NPC
		case MODEL_TIME_LIMIT_QUEST_NPC_TERSIA:
			if (rand() % 5 < 4)
				SetAction(o, 0);
			else
				SetAction(o, rand()%2 + 1);
			break;
#endif	// ASG_ADD_TIME_LIMIT_QUEST_NPC
#ifdef ASG_ADD_KARUTAN_NPC
		case MODEL_KARUTAN_NPC_VOLVO:
			if (rand() % 5 < 4)
				SetAction(o, 0);
			else
				SetAction(o, 1);
			break;
#endif	// ASG_ADD_KARUTAN_NPC
#ifdef LEM_ADD_LUCKYITEM
		case MODEL_LUCKYITEM_NPC:
			if( rand()%5 < 4 || o->CurrentAction == 1 )
			{
				SetAction(o, 0);
			}
			else
			{
				SetAction(o, 1);
			}
			break;
#endif // LEM_ADD_LUCKYITEM
		default:
			SetAction(o,rand()%2);
			break;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
//  캐릭터들의 애니메이션.
//////////////////////////////////////////////////////////////////////////
void AnimationCharacter ( CHARACTER* c, OBJECT* o, BMD* b )
{
    bool bEventNpc = false;
    if ( o->Kind==KIND_NPC && ( World==WD_0LORENCIA || World==WD_2DEVIAS ) && o->Type==MODEL_PLAYER && ( o->SubType>=MODEL_SKELETON1&&o->SubType<=MODEL_SKELETON3 ) )
    {
		if ( World==WD_0LORENCIA )
		{
			Vector(0.f,0.f,90.f,o->Angle);
		}
		else
		{
			Vector(0.f,0.f,0.f,o->Angle);
		}

        bEventNpc = true;
    }

    //  NPC 채팅표시 처리
    OnlyNpcChatProcess ( c, o );

#ifndef YDG_MOD_GOLDEN_ARCHER_ANGEL_TEXT	// 정리할때 삭제해야 하는 부분
#ifdef CASTLE_EVENT
	// 4초마다 황금의 궁수가 2가지 말을 번갈아가면서 한다
	static bool bCatsleEbentMsg = 0;
	static DWORD dwPrevCastleMSGTime = GetTickCount() - 5000;
	DWORD dwCastleMSGTime = GetTickCount();
	if( dwCastleMSGTime - dwPrevCastleMSGTime > 4000 )
	{
		if( !strcmp(c->ID, "황금의 궁수") )
		{
			if( bCatsleEbentMsg )
				CreateChat ( c->ID, GlobalText[1167], c );
			else
				CreateChat ( c->ID, GlobalText[1168], c );
			dwPrevCastleMSGTime = dwCastleMSGTime;
			bCatsleEbentMsg = !bCatsleEbentMsg;
		}
	}
#endif // CASTLE_EVENT
#endif	// YDG_MOD_GOLDEN_ARCHER_ANGEL_TEXT	// 정리할때 삭제해야 하는 부분

	bool Play = CharacterAnimation ( c, o );

	if ( !Play )
	{
		c->LongRangeAttack = -1;
		c->Foot[0] = false;
		c->Foot[1] = false;
		if(o->Type==MODEL_PLAYER)
		{
            if( bEventNpc )
            {
                //  NPC 애니메이션 정지시 세팅.
                PlayerNpcStopAnimationSetting ( c, o );
            }
            else
            {
                //  플레이어 애니메이션 정지시 세팅.
                PlayerStopAnimationSetting ( c, o );
            }
		}
        else
        {
            //  기타 애니메이션 정지시 세팅.
            EtcStopAnimationSetting ( c, o );
        }
#ifdef YDG_ADD_SANTA_MONSTER
		if(o->CurrentAction == MONSTER01_STOP1 || o->CurrentAction == MONSTER01_STOP2)
			PlayMonsterSound(o);				// 몬스터 정지음
#endif	// YDG_ADD_SANTA_MONSTER

        if ( o->Type==MODEL_WARCRAFT )
        {
            o->AnimationFrame = 8.f;
        }
	}

    //  데비아스, 로랜시아 추가 상점 NPC
	switch(o->Type)
	{
	case MODEL_DEVIAS_TRADER:
		if ( b->CurrentAnimationFrame==b->Actions[o->CurrentAction].NumAnimationKeys-1 )
        {
			if ( rand()%32==0 )
				SetAction ( o, 1 );
			else
				SetAction ( o, 0 );
		}
		break;
	case MODEL_MONSTER01+128:	// 토끼 머리 긁는 동작
		if ( o->CurrentAction <= 1 && b->CurrentAnimationFrame == b->Actions[o->CurrentAction].NumAnimationKeys - 1 )
        {
			if ( rand()%10==0 )
				SetAction ( o, 1 );
			else
				SetAction ( o, 0 );
		}
		break;
	}
}


//////////////////////////////////////////////////////////////////////////
//  무기의 잔상 처리를 한다.
//////////////////////////////////////////////////////////////////////////
void CreateWeaponBlur ( CHARACTER* c, OBJECT* o, BMD* b )
{
	if ( o->AnimationFrame>=3.f 
	  || ( o->CurrentAction==PLAYER_ATTACK_TWO_HAND_SWORD_TWO )
	   )
	{
		int Hand = 0;
        int Hand2= 0;
		if(o->Type == MODEL_PLAYER)
		{
			if(o->CurrentAction==PLAYER_ATTACK_SWORD_LEFT1 || o->CurrentAction==PLAYER_ATTACK_SWORD_LEFT2)
            {
                Hand = 1;
                Hand2= 1;
            }
		}
		int BlurType = 0;
		int BlurMapping = 0;//0기본,1파랑,2빨강
		int Type  = c->Weapon[Hand].Type;
		int Level = c->Weapon[Hand].Level;
		if(o->Type == MODEL_PLAYER)
		{
            if ( o->CurrentAction==PLAYER_ATTACK_ONE_FLASH || o->CurrentAction==PLAYER_ATTACK_RUSH )
            {
                BlurType = 1;
                BlurMapping = 2;
            }
            else if(o->CurrentAction==PLAYER_ATTACK_SKILL_SWORD2 || o->CurrentAction==PLAYER_ATTACK_SKILL_SWORD3 || o->CurrentAction==PLAYER_ATTACK_SKILL_SWORD4)
			{
				BlurType = 1;
				if ( Type==MODEL_SWORD+14 || Type==MODEL_SWORD+21 || Type==MODEL_SWORD+31 )
					BlurMapping = 1;
				else
					BlurMapping = 2;
			}
            else if ( o->CurrentAction==PLAYER_ATTACK_STRIKE )
            {
				BlurType = 1;
				BlurMapping = 2;
            }
			else if(o->CurrentAction == PLAYER_SKILL_LIGHTNING_ORB 
				|| o->CurrentAction == PLAYER_SKILL_LIGHTNING_ORB_UNI 
				|| o->CurrentAction == PLAYER_SKILL_LIGHTNING_ORB_DINO 
				|| o->CurrentAction == PLAYER_SKILL_LIGHTNING_ORB_FENRIR)
			{
				BlurType = 1;
                BlurMapping = 1;
			}
#ifdef CSK_ADD_SKILL_BLOWOFDESTRUCTION
			else if(o->CurrentAction == PLAYER_SKILL_BLOW_OF_DESTRUCTION && o->AnimationFrame >= 2.f && o->AnimationFrame <= 8.f)
			{
				BlurType = 1;	
				BlurMapping = 2;
			}
#endif // CSK_ADD_SKILL_BLOWOFDESTRUCTION	
			else if(o->CurrentAction==PLAYER_ATTACK_SKILL_SWORD5)
			{
				BlurType = 1;
				if(Type==MODEL_MACE+5)
					BlurMapping = 1;
				else
    				BlurMapping = 2;
			}
			else if(Type>=MODEL_SWORD && Type<MODEL_SWORD+MAX_ITEM_INDEX)
			{
				if ( ( o->CurrentAction>=PLAYER_ATTACK_SWORD_RIGHT1 && o->CurrentAction<=PLAYER_ATTACK_TWO_HAND_SWORD3 )
					|| o->CurrentAction==PLAYER_ATTACK_TWO_HAND_SWORD_TWO 
				   )
				{
					BlurType = 1;
                    if( Type==MODEL_SWORD+17 )  //  다크브레이커.
					{
                        BlurMapping = 6;
					}
                    else if( o->CurrentAction==PLAYER_ATTACK_TWO_HAND_SWORD3
						  || o->CurrentAction==PLAYER_ATTACK_TWO_HAND_SWORD_TWO )
					{
						   if(Type==MODEL_SWORD+25)
								BlurMapping = 2;
							else
								BlurMapping = 1;
					}
				}
			}
			else if(Type==MODEL_AXE+3 || Type>=MODEL_AXE+5 && Type<MODEL_MACE+MAX_ITEM_INDEX)
			{
				if(o->CurrentAction>=PLAYER_ATTACK_SKILL_SWORD1 && o->CurrentAction<=PLAYER_ATTACK_SKILL_SWORD5 )
				{
					BlurType = 1;
                    BlurMapping = 2;
				}
			}
			else if(Type>=MODEL_SPEAR && Type<MODEL_SPEAR+MAX_ITEM_INDEX)
			{
				if(o->CurrentAction>=PLAYER_ATTACK_SPEAR1 && o->CurrentAction<=PLAYER_ATTACK_SCYTHE3)
				{
					BlurType = 3;
					if ( Type==MODEL_SPEAR+10 )
					{
						BlurType = 1;
						BlurMapping = 0;
					}
					else if(o->CurrentAction==PLAYER_ATTACK_SCYTHE3)
						BlurMapping = 1;
				}
			}
		}
		else
		{
            if(c->MonsterIndex==71 || c->MonsterIndex==74
#ifdef LOREN_RAVINE_EVENT
				|| c->MonsterIndex==301
#endif
				)//   야누스.
            {
				if(o->CurrentAction>=MONSTER01_ATTACK1 && o->CurrentAction<=MONSTER01_ATTACK2)
                {
                    BlurType = 1;
                    BlurMapping = 6;
                }
            }
            else if ( o->Type==MODEL_MONSTER01+66 )
            {
				if ( o->CurrentAction==MONSTER01_ATTACK1 )
                {
                    BlurType = 5;
                    BlurMapping = 2;
                    Hand  = 0;
                    Hand2 = 1;
                }
            }
            else if ( o->Type==MODEL_MONSTER01+67 )
            {
				if ( o->CurrentAction>=MONSTER01_ATTACK1 && o->CurrentAction<=MONSTER01_ATTACK2 )
                {
                    BlurType = 4;
                    BlurMapping = 0;
                    Level = 99;
                }
            }
            else if ( o->Type==MODEL_MONSTER01+69 ) //  쿤둔의 잔영.
            {
				if(o->CurrentAction>=MONSTER01_ATTACK1 && o->CurrentAction<=MONSTER01_ATTACK2)
                {
                    if ( o->SubType==9 )    //  보스급 쿤둔의 잔영.
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
            else if(Type>=MODEL_SWORD && Type<MODEL_SWORD+MAX_ITEM_INDEX)
			{
				if(o->CurrentAction>=MONSTER01_ATTACK1 && o->CurrentAction<=MONSTER01_ATTACK2)
					BlurType = 1;
			}
		}
		if(BlurType > 0)
		{
			vec3_t  Light;
            vec3_t  Pos1, Pos2;
            vec3_t  p, p2;
			switch(BlurType)
			{
			case 1:Vector(0.f,-20.f,0.f,Pos1);break;
			case 2:Vector(0.f,-80.f,0.f,Pos1);break;
			case 3:Vector(0.f,-100.f,0.f,Pos1);break;
			}
			Vector(0.f,-120.f,0.f,Pos2);

            if ( BlurType==4 )
            {
			    Vector(0.f,0.f,0.f,Pos1);
			    Vector(0.f,-200.f,0.f,Pos2);
            }
            else if ( BlurType==5 )
            {
			    Vector(0.f,0.f,0.f,Pos1);
			    Vector(0.f,-20.f,0.f,Pos2);
            }

			if(Type==MODEL_SWORD+13 || Type==MODEL_MACE+6 || Type==MODEL_SPEAR+9 )//더블 블레이드
			{
				Vector(1.f,0.2f,0.2f,Light);
			}
            else if ( Level==99 )
            {
				Vector(0.3f,0.2f,1.f,Light);
            }
			else if(BlurMapping == 0)
			{
				if(Level>=7)
				{
					Vector(1.f,0.6f,0.2f,Light);
				}
				else if(Level>=5)
				{
					Vector(0.2f,0.4f,1.f,Light);
				}
				else if(Level>=3)
				{
					Vector(1.f,0.2f,0.2f,Light);
				}
				else
				{
					Vector(0.8f,0.8f,0.8f,Light);
				}
			}
			else
			{
				Vector(1.f,1.f,1.f,Light);
			}

            if( ( o->Type != MODEL_PLAYER || Type==MODEL_SWORD+3 || Type==MODEL_SWORD+6 || Type==MODEL_SWORD+9 || Type==MODEL_SWORD+11 || Type==MODEL_SPEAR+4 )
               && o->Type!=MODEL_MONSTER01+66 && o->Type!=MODEL_MONSTER01+67 && o->Type!=MODEL_MONSTER01+69 
              )
            {
                b->TransformPosition(o->BoneTransform[c->Weapon[Hand].LinkBone],Pos1,p,true);
			    b->TransformPosition(o->BoneTransform[c->Weapon[Hand2].LinkBone],Pos2,p2,true);
			    CreateBlur(c,p,p2,Light,BlurMapping);
            }
#ifdef PBG_ADD_NEWCHAR_MONK
			else if(g_CMonkSystem.IsSwordformGloves(Type))
			{
				g_CMonkSystem.MoveBlurEffect(c, o, b);
			}
#endif //PBG_ADD_NEWCHAR_MONK
            else
            {
                float inter = 10.f;
                float animationFrame = o->AnimationFrame - b->Actions[b->CurrentAction].PlaySpeed;
                float priorAnimationFrame = o->PriorAnimationFrame;
                float animationSpeed = b->Actions[b->CurrentAction].PlaySpeed/inter;

                for( int i=0; i<(int)(inter); ++i )
                {
    	            b->Animation(BoneTransform,animationFrame,priorAnimationFrame,o->PriorAction,o->Angle,o->HeadAngle);

                    b->TransformPosition(BoneTransform[c->Weapon[Hand].LinkBone],Pos1,p,false);
                    b->TransformPosition(BoneTransform[c->Weapon[Hand2].LinkBone],Pos2,p2,false);

                    if ( o->Type==MODEL_MONSTER01+66 && i%2 )
                    {
                        CreateParticle ( BITMAP_FIRE+3, p2, o->Angle, Light, 12 );
                    }
#ifdef PJH_SEASON4_DARK_NEW_SKILL_CAOTIC
					if(c->Weapon[0].Type != -1 || c->Weapon[1].Type != -1)	// 양손무기
#endif //#ifdef PJH_SEASON4_DARK_NEW_SKILL_CAOTIC
						CreateBlur(c,p,p2,Light,BlurMapping,true);

                    animationFrame += animationSpeed;
                }
            }
		}
		M31HuntingGround::MoveHuntingGroundBlurEffect(c, o, b);
#ifdef CRYINGWOLF_2NDMVP
		M34CryingWolf2nd::MoveCryingWolf2ndBlurEffect(c, o, b);
#endif // CRYINGWOLF_2NDMVP

		M34CryWolf1st::MoveCryWolf1stBlurEffect(c, o, b);
		M33Aida::MoveAidaBlurEffect(c, o, b);
		M37Kanturu1st::MoveKanturu1stBlurEffect(c, o, b);
		M38Kanturu2nd::Move_Kanturu2nd_BlurEffect(c, o, b);
		M39Kanturu3rd::MoveKanturu3rdBlurEffect(c, o, b);
		SEASON3A::CGM3rdChangeUp::Instance().MoveBalgasBarrackBlurEffect(c, o, b);
		g_CursedTemple->MoveBlurEffect( c, o, b );
		SEASON3B::GMNewTown::MoveBlurEffect( c, o, b );
		SEASON3C::GMSwampOfQuiet::MoveBlurEffect( c, o, b );

#ifdef PSW_ADD_MAPSYSTEM
		TheMapProcess().MoveBlurEffect( c, o, b );
#endif //PSW_ADD_MAPSYSTEM
	}
    else
    {
        VectorCopy(o->Position,o->StartPosition);
    }
}


//////////////////////////////////////////////////////////////////////////
//  화면의 캐릭터 처리.
//////////////////////////////////////////////////////////////////////////
void MoveCharacter(CHARACTER *c,OBJECT *o)
{
    if( o->Type == MODEL_WARCRAFT )
    {
        char Text[100];
        char ID[100];
        wsprintf ( ID, "%s .", c->ID );
		// 1176 "제%d 칼리마행"
        wsprintf ( Text, GlobalText[1176], c->Level );
        strcat ( ID, Text );

        AddObjectDescription ( ID, o->Position );
    }

	BMD *b = &Models[o->Type];
	VectorCopy(o->Position,b->BodyOrigin); 
	b->BodyScale     = o->Scale;
	b->CurrentAction = o->CurrentAction;

    // 주인공의 정지 시간을 계산한다.
    CalcStopTime ();
    // 주인공 자신의 능력(변화되는)을 계산한다.
    HeroAttributeCalc ( c );
    // 밀리는 캐릭터 처리 ( 회전, 점프(공)등등 )
    PushingCharacter ( c, o );
    // 죽는 캐릭터 처리.
    DeadCharacter ( c, o, b );
	// 알파 처리
	Alpha ( o );

	if( c->Freeze > 0.f ) 
	{
		c->Freeze -= 0.03f;
	}

    //  캐릭터 에니메이션을 한다.
    AnimationCharacter ( c, o, b );

	//  공격
	if ( c->Dead>0 )
	{
		c->Dead++;
		if(c->Dead >= 15)
		{
			SetPlayerDie(c);
		}
        if ( InBloodCastle() && o->m_bActionStart )
        {
            SetPlayerDie(c);
        }
	}

	vec3_t p,Position;
	vec3_t Light;
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
	float Luminosity = (float)(rand()%6+2)*0.1f;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
	Vector(0.f,0.f,0.f,p);
	Vector(1.f,1.f,1.f,Light);

	//  맞는 효과
    if ( battleCastle::InBattleCastle()==false && o->m_byHurtByOneToOne>0 )
	{
		vec3_t pos1, pos2;

		Vector(0.f,0.f,0.f,p);
		for( int i=0; i < b->NumBones; ++i)
		{
            if ( !b->Bones[i].Dummy )
            {
			    int iParent = b->Bones[i].Parent;
			    if ( iParent > -1 && iParent < b->NumBones )
			    {
				    b->TransformPosition(o->BoneTransform[i],p,pos1,true);
				    b->TransformPosition(o->BoneTransform[iParent],p,pos2,true);

                    GetNearRandomPos( pos1, 20, pos1);
				    GetNearRandomPos( pos2, 20, pos2);
				    CreateJoint(BITMAP_JOINT_THUNDER,pos1,pos2,o->Angle,7,NULL,20.f);
			    }
            }
		}
		o->m_byHurtByOneToOne--;
	}
    //  텔레포트일 때의 효과.
    if ( ( o->CurrentAction==PLAYER_ATTACK_TELEPORT || o->CurrentAction==PLAYER_ATTACK_RIDE_TELEPORT 
		|| o->CurrentAction == PLAYER_FENRIR_ATTACK_DARKLORD_TELEPORT	//^ 펜릴 캐릭터 에니메이션
		)&& o->AnimationFrame>5.5f )
    {
        Vector ( 0.f, 0.f, 0.f, p );
        Vector ( 0.3f, 0.5f, 1.f, Light );
		b->TransformPosition ( o->BoneTransform[42], p, Position, true );
        CreateParticle ( BITMAP_LIGHT, Position, o->Angle, Light );
    }

#ifdef USE_SELFCHECKCODE
	SendCrcOfFunction( 5, 21, Scene, 0xA041);
#endif

    //  공격중.
    if ( c->AttackTime>0 )
	{
        AttackStage ( c, o );
        AttackEffect ( c );
		c->AttackTime++;
	}

    if ( c->AttackTime>=g_iLimitAttackTime )
	{
		c->AttackTime = 0;
		o->PKKey = getTargetCharacterKey ( c, SelectedCharacter );

#ifdef _PVP_ATTACK_GUARD
		if (SelectedCharacter == -1)
			o->PKKey = getTargetCharacterKey( c, SelectedNpc );
#endif	// _PVP_ATTACK_GUARD

		switch ( ( c->Skill ) )
		{
		case AT_SKILL_SUMMON:
		case AT_SKILL_SUMMON+1:
		case AT_SKILL_SUMMON+2:
		case AT_SKILL_SUMMON+3:
		case AT_SKILL_SUMMON+4:
		case AT_SKILL_SUMMON+5:
	    case AT_SKILL_SUMMON+6:
#ifdef ADD_ELF_SUMMON
		case AT_SKILL_SUMMON+7:
#endif// ADD_ELF_SUMMON
			CreateEffect(BITMAP_MAGIC+1,o->Position,o->Angle,o->Light,3,o);
			break;
#ifdef PJH_SEASON4_MASTER_RANK4
				case AT_SKILL_ANGER_SWORD_UP:
				case AT_SKILL_ANGER_SWORD_UP+1:
				case AT_SKILL_ANGER_SWORD_UP+2:
				case AT_SKILL_ANGER_SWORD_UP+3:
				case AT_SKILL_ANGER_SWORD_UP+4:
#endif //PJH_SEASON4_MASTER_RANK4
        
//---------------------------------------------------------------------------------------------------------
					
		// 흑기사 - 분노의 일격
		case AT_SKILL_FURY_STRIKE:
		{
            o->Weapon = c->Weapon[0].Type-MODEL_SWORD;
			o->WeaponLevel = (BYTE)c->Weapon[0].Level;
      		CreateEffect(MODEL_SKILL_FURY_STRIKE,o->Position,o->Angle,o->Light,0,o,o->PKKey,FindHotKey( ( c->Skill )));//클라이언트마법처리
     		PlayBuffer(SOUND_FURY_STRIKE1);
            break;
		}
//---------------------------------------------------------------------------------------------------------
			
		// 흑기사 - 파괴의 일격 
#ifdef CSK_ADD_SKILL_BLOWOFDESTRUCTION
		case AT_SKILL_BLOW_OF_DESTRUCTION:
			o->Weapon = c->Weapon[0].Type-MODEL_SWORD;
			o->WeaponLevel = (BYTE)c->Weapon[0].Level;
			
			// 이펙트 발생
			// o->Light 에다가 타켓 위치 담아서 이펙트 발생한다.
			Vector(0.f, 0.f, 0.f, o->Light);
			o->Light[0] = (float)(c->SkillX+0.5f)*TERRAIN_SCALE;
			o->Light[1] = (float)(c->SkillY+0.5f)*TERRAIN_SCALE;
			o->Light[2] = o->Position[2];
			CreateEffect(MODEL_BLOW_OF_DESTRUCTION, o->Position, o->Angle, o->Light, 0, o);
		
#ifdef CSK_ADD_SKILL_BLOWOFDESTRUCTION_SOUND
			PlayBuffer(SOUND_SKILL_BLOWOFDESTRUCTION);
#endif // CSK_ADD_SKILL_BLOWOFDESTRUCTION_SOUND
			
			break;
#endif // CSK_ADD_SKILL_BLOWOFDESTRUCTION	
		
	

//---------------------------------------------------------------------------------------------------------

#ifdef PJH_SEASON4_MASTER_RANK4
		case AT_SKILL_BLOOD_ATT_UP:
		case AT_SKILL_BLOOD_ATT_UP+1:
		case AT_SKILL_BLOOD_ATT_UP+2:
		case AT_SKILL_BLOOD_ATT_UP+3:
		case AT_SKILL_BLOOD_ATT_UP+4:
#endif //PJH_SEASON4_MASTER_RANK4
        case AT_SKILL_REDUCEDEFENSE:    //  마검사 방어력 감소.
			o->Weapon = c->Weapon[0].Type-MODEL_SWORD;
			o->WeaponLevel = (BYTE)c->Weapon[0].Level;
            break;
#ifdef PJH_SEASON4_MASTER_RANK4
	case AT_SKILL_POWER_SLASH_UP:
	case AT_SKILL_POWER_SLASH_UP+1:
	case AT_SKILL_POWER_SLASH_UP+2:
	case AT_SKILL_POWER_SLASH_UP+3:
	case AT_SKILL_POWER_SLASH_UP+4:
#endif //PJH_SEASON4_MASTER_RANK4
        case AT_SKILL_ICE_BLADE:
			o->Weapon = c->Weapon[0].Type-MODEL_SWORD;
			o->WeaponLevel = (BYTE)c->Weapon[0].Level;
			break;
		case AT_SKILL_LIFE_UP:
		case AT_SKILL_LIFE_UP+1:
		case AT_SKILL_LIFE_UP+2:
		case AT_SKILL_LIFE_UP+3:
		case AT_SKILL_LIFE_UP+4:
        case AT_SKILL_VITALITY:	// 스웰라이프
            {
                vec3_t Angle = { 0.0f, 0.0f, 0.0f};
			    int iCount = 36;
			    for ( int i = 0; i < iCount; ++i)
			    {
				    Angle[0] = -10.f;
				    Angle[1] = 0.f;
				    Angle[2] = i*10.f;
				    vec3_t Position;
				    VectorCopy( o->Position, Position);
				    Position[2] += 100.f;
					CreateJoint(BITMAP_JOINT_SPIRIT,Position,Position,Angle,2,o,60.f,0,0);

                    if( (i%20) == 0 )
					{
                      CreateEffect(BITMAP_MAGIC+1,o->Position,Angle,o->Light,4,o);
					}
			    }
            }
     		PlayBuffer(SOUND_SWELLLIFE);
            break;

        case AT_SKILL_STUN:
//            CreateEffect ( MODEL_STUN_STONE, o->Position, o->Angle, o->Light, 1 );
			CreateJoint(BITMAP_FLASH,o->Position,o->Position,o->Angle,7,NULL);

            PlayBuffer ( SOUND_BMS_STUN );
            break;

        case AT_SKILL_REMOVAL_STUN:
			{
				if(c->TargetCharacter != -1)
				{
					CHARACTER *tc = &CharactersClient[c->TargetCharacter];
					OBJECT *to = &tc->Object;
					if(to != o)
					{
						VectorCopy ( to->Position, Position );
						Position[2] += 1200.f;
						CreateJoint ( BITMAP_FLASH, Position, Position, to->Angle, 0, to, 120.f );

						PlayBuffer ( SOUND_BMS_STUN_REMOVAL );
					}
				}
			}
            break;

        case AT_SKILL_MANA:
            {
                vec3_t Angle = { 0.0f, 0.0f, 0.0f};
			    int iCount = 36;
				
				for ( int i = 0; i < iCount; ++i)
				{
					Angle[0] = -10.f;
					Angle[1] = 0.f;
					Angle[2] = i*10.f;
					vec3_t Position;
					VectorCopy( o->Position, Position);
					Position[2] += 100.f;
					CreateJoint ( BITMAP_JOINT_SPIRIT, Position, Position, Angle, 21, o, 60.f, 0, 0 );
					if ( (i%20)==0 )
						CreateEffect ( BITMAP_MAGIC+1, o->Position, Angle, Light, 10, o );
				}
            }
     		PlayBuffer ( SOUND_BMS_MANA );
            break;

        case AT_SKILL_INVISIBLE:
			{
				if(c->TargetCharacter != -1)
				{
					CHARACTER *tc = &CharactersClient[c->TargetCharacter];
					OBJECT *to = &tc->Object;

					DeleteJoint(MODEL_SPEARSKILL, to, 4);
					DeleteJoint(MODEL_SPEARSKILL, to, 9);
	//				if(to != o)
	//				{
					CreateEffect ( BITMAP_MAGIC+1, to->Position, to->Angle, to->Light, 6, to );
					PlayBuffer ( SOUND_BMS_INVISIBLE );
	//				}
				}
			}
            break;
        case AT_SKILL_REMOVAL_INVISIBLE:
			{
				if(c->TargetCharacter != -1)
				{
					CHARACTER *tc = &CharactersClient[c->TargetCharacter];
					OBJECT *to = &tc->Object;

					if(to != o)
					{
						VectorCopy ( to->Position, Position );
						Position[2] += 1200.f;
						CreateJoint ( BITMAP_FLASH, Position, Position, to->Angle, 1, to, 120.f );

						PlayBuffer ( SOUND_BMS_STUN_REMOVAL );
					}
				}
			}
            break;
        case AT_SKILL_REMOVAL_BUFF:
            {
                //  마법 해제 효과.
				vec3_t Angle;
				vec3_t Position;
				VectorCopy ( o->Position, Position );
				Position[2] += 100.f;

				list<eBuffState> bufflist;
				
				//debuff
				bufflist.push_back( eDeBuff_Poison ); 
				bufflist.push_back( eDeBuff_Freeze );
				bufflist.push_back( eDeBuff_Harden ); 
				bufflist.push_back( eDeBuff_Defense );
				bufflist.push_back( eDeBuff_Stun );
				bufflist.push_back( eDeBuff_Sleep );
#ifdef CSK_ADD_SKILL_BLOWOFDESTRUCTION
				bufflist.push_back( eDeBuff_BlowOfDestruction );
#endif // CSK_ADD_SKILL_BLOWOFDESTRUCTION

				//buff
				bufflist.push_back( eBuff_HpRecovery ); bufflist.push_back( eBuff_Attack );
				//bufflist.push_back( eBuff_Life ); bufflist.push_back( eBuff_Attack );
				bufflist.push_back( eBuff_Defense ); bufflist.push_back( eBuff_AddAG );
				bufflist.push_back( eBuff_Cloaking ); bufflist.push_back( eBuff_AddSkill );
				bufflist.push_back( eBuff_PhysDefense ); bufflist.push_back( eBuff_AddCriticalDamage );
				bufflist.push_back( eBuff_CrywolfAltarOccufied );

				g_CharacterUnRegisterBuffList(o, bufflist );
				
				Vector ( 0.f, 0.f, 45.f, Angle );
				CreateJoint ( MODEL_SPEARSKILL, Position, Position, Angle, 5, o, 170.0f );
				Position[2] -= 10.f;                                              
				Vector ( 0.f, 0.f, 135.f, Angle );
				CreateJoint ( MODEL_SPEARSKILL, Position, Position, Angle, 6, o, 170.0f );
				Position[2] -= 10.f;
				Vector ( 0.f, 0.f, 225.f, Angle );
				CreateJoint ( MODEL_SPEARSKILL, Position, Position, Angle, 7, o, 170.0f );

				Vector ( 0.f, 0.f, 90.f, Angle );
				CreateJoint ( MODEL_SPEARSKILL, Position, Position, Angle, 5, o, 170.0f );
				Position[2] -= 10.f;                                              
				Vector ( 0.f, 0.f, 180.f, Angle );
				CreateJoint ( MODEL_SPEARSKILL, Position, Position, Angle, 6, o, 170.0f );
				Position[2] -= 10.f;
				Vector ( 0.f, 0.f, 270.f, Angle );
				CreateJoint ( MODEL_SPEARSKILL, Position, Position, Angle, 7, o, 170.0f );
            }

            PlayBuffer ( SOUND_BMS_MAGIC_REMOVAL );
            break;
        case AT_SKILL_IMPROVE_AG:
            {
                vec3_t Angle = { -45.f, 0.f, 45.f };
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
                vec3_t Light = { 1.f, 1.f, 1.f };
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
                vec3_t Position;

                Position[0] = o->Position[0]+sinf( 45*0.1f )*80.f;
                Position[1] = o->Position[1]+cosf( 45*0.1f )*80.f;
                Position[2] = o->Position[2]+300;
                CreateJoint ( BITMAP_JOINT_HEALING, Position, Position, Angle, 10, o, 15.f );
                Angle[2] = 405.f;
                CreateJoint ( BITMAP_JOINT_HEALING, Position, Position, Angle, 10, o, 15.f );

                Angle[2] = 225.f;
                Position[0] = o->Position[0]+cosf( 225*0.1f )*80.f;
                Position[1] = o->Position[1]+sinf( 225*0.1f )*80.f;
                Position[2] = o->Position[2]+300;
                CreateJoint ( BITMAP_JOINT_HEALING, Position, Position, Angle, 10, o, 15.f );
                Angle[2] = 450.f;
                CreateJoint ( BITMAP_JOINT_HEALING, Position, Position, Angle, 10, o, 15.f );
            }
     		PlayBuffer(SOUND_SWELLLIFE);
            break;
        case AT_SKILL_ADD_CRITICAL: //  크리티컬 데미지 확률증가.
            Vector ( 0.f, 0.f, 0.f, p );
            Vector ( 1.f, 0.6f, 0.3f, Light );
            if ( c->Weapon[0].Type!=MODEL_BOW+15 )
            {
                b->TransformPosition ( o->BoneTransform[c->Weapon[0].LinkBone], p, Position, true );
                CreateEffect ( MODEL_DARKLORD_SKILL, Position, o->Angle, Light, 0 );
            }
            if ( c->Weapon[1].Type!=MODEL_BOW+7 && ( c->Weapon[1].Type<MODEL_SHIELD || c->Weapon[1].Type>=MODEL_SHIELD+MAX_ITEM_INDEX ) )
            {
                b->TransformPosition ( o->BoneTransform[c->Weapon[1].LinkBone], p, Position, true );
                CreateEffect ( MODEL_DARKLORD_SKILL, Position, o->Angle, Light, 1 );
            }
            PlayBuffer ( SOUND_CRITICAL, o );
            break;
        case AT_SKILL_PARTY_TELEPORT:// 파티원 소환.
			CreateEffect ( MODEL_CIRCLE, o->Position, o->Angle, o->Light, 2, o );
			CreateEffect ( MODEL_CIRCLE_LIGHT, o->Position, o->Angle, o->Light, 3 );

            PlayBuffer ( SOUND_PART_TELEPORT, o );
            break;
        case AT_SKILL_THUNDER_STRIKE:// 선더스트라이크.
			CalcAddPosition ( o, 0.f, -90.f, -50.f, Position );
            if ( o->CurrentAction==PLAYER_ATTACK_RIDE_ATTACK_FLASH )
            {
                Position[2] += 80.f;
            }
			else if(o->CurrentAction == PLAYER_FENRIR_ATTACK_DARKLORD_FLASH)	//^ 펜릴 스킬 관련
			{
				Position[2] += 40.f;
			}

            CreateEffect ( BITMAP_FLARE_FORCE, Position, o->Angle, o->Light, 0, o );

            PlayBuffer ( SOUND_ELEC_STRIKE, o );
            break;
        case AT_SKILL_RIDER:
            CreateEffect ( BITMAP_SHOTGUN, o->Position, o->Angle, o->Light, 0, o, o->PKKey, CurrentSkill );

            PlayBuffer(SOUND_SKILL_SWORD3);
            break;
		case AT_SKILL_TORNADO_SWORDA_UP:
		case AT_SKILL_TORNADO_SWORDA_UP+1:
		case AT_SKILL_TORNADO_SWORDA_UP+2:
		case AT_SKILL_TORNADO_SWORDA_UP+3:
		case AT_SKILL_TORNADO_SWORDA_UP+4:

		case AT_SKILL_TORNADO_SWORDB_UP:
		case AT_SKILL_TORNADO_SWORDB_UP+1:
		case AT_SKILL_TORNADO_SWORDB_UP+2:
		case AT_SKILL_TORNADO_SWORDB_UP+3:
		case AT_SKILL_TORNADO_SWORDB_UP+4:
		case AT_SKILL_WHEEL:
			o->Weapon = c->Weapon[0].Type-MODEL_SWORD;
			o->WeaponLevel = (BYTE)c->Weapon[0].Level;
      		CreateEffect(MODEL_SKILL_WHEEL1,o->Position,o->Angle,o->Light,0,o,o->PKKey,FindHotKey( ( c->Skill )));//클라이언트마법처리

			if (SceneFlag != LOG_IN_SCENE)
     			PlayBuffer(SOUND_SKILL_SWORD4);

			// 회오리 베기를 지정된 Tick 동안 오른손검 미출력을 위해 PostMoveProcess를 연동시킵니다.
#ifdef LDS_FIX_MODULE_POSTMOVECHARACTER_SKILL_WHEEL
			c->PostMoveProcess_Active( g_iLimitAttackTime );
#endif // LDS_FIX_MODULE_POSTMOVECHARACTER_SKILL_WHEEL

            break;
		case AT_SKILL_HELL_FIRE_UP:
		case AT_SKILL_HELL_FIRE_UP+1:
		case AT_SKILL_HELL_FIRE_UP+2:
		case AT_SKILL_HELL_FIRE_UP+3:
		case AT_SKILL_HELL_FIRE_UP+4:
        case AT_SKILL_HELL:
			CreateEffect(MODEL_CIRCLE,o->Position,o->Angle,o->Light,0,o,o->PKKey,FindHotKey(( c->Skill )));//클라이언트마법처리
			CreateEffect(MODEL_CIRCLE_LIGHT,o->Position,o->Angle,o->Light);

            if ( InHellas()==true )
            {
                AddWaterWave( (c->PositionX), (c->PositionY), 2, -1500 );
            }

          	PlayBuffer(SOUND_HELLFIRE);
			break;
		case AT_SKILL_BLAST_HELL:
			CreateEffect(MODEL_CIRCLE,o->Position,o->Angle,o->Light,1,o);
			StopBuffer(SOUND_NUKE1,true);
          	PlayBuffer(SOUND_NUKE2);
			break;
		case AT_SKILL_BLAST_POISON:
			Position[0] = (float)(c->SkillX+0.5f)*TERRAIN_SCALE;
			Position[1] = (float)(c->SkillY+0.5f)*TERRAIN_SCALE;
			Position[2] = RequestTerrainHeight(Position[0],Position[1]);

			Vector ( 0.8f, 0.5f, 0.1f, Light );
			CreateEffect(MODEL_FIRE,Position,o->Angle,Light,6,NULL,0);
			CreateEffect(MODEL_FIRE,Position,o->Angle,Light,6,NULL,0);
			PlayBuffer(SOUND_DEATH_POISON1);

			if ( c == Hero)
			{
				++CharacterMachine->PacketSerial;
			}
			break;
		case AT_SKILL_ICE_UP:
		case AT_SKILL_ICE_UP+1:
		case AT_SKILL_ICE_UP+2:
		case AT_SKILL_ICE_UP+3:
		case AT_SKILL_ICE_UP+4:
		case AT_SKILL_BLAST_FREEZE:
			{
				vec3_t TargetPosition, Pos;
				TargetPosition[0] = (float)(c->SkillX+0.5f)*TERRAIN_SCALE;
				TargetPosition[1] = (float)(c->SkillY+0.5f)*TERRAIN_SCALE;
				TargetPosition[2] = RequestTerrainHeight(TargetPosition[0],TargetPosition[1]);

				for( int i=0; i<10; ++i )
				{
					Pos[0] = TargetPosition[0];
					Pos[1] = TargetPosition[1];
					Pos[2] = TargetPosition[2] + (rand()%50)*i;
					CreateEffect(MODEL_BLIZZARD,Pos,o->Angle,Light,0, NULL, i+1 );
				}
				if ( c == Hero)
				{
					++CharacterMachine->PacketSerial;
				}

				PlayBuffer(SOUND_SUDDEN_ICE1);
			}
			break;

        case AT_SKILL_FLAME:
			Position[0] = (float)(c->SkillX+0.5f)*TERRAIN_SCALE;
			Position[1] = (float)(c->SkillY+0.5f)*TERRAIN_SCALE;
			Position[2] = RequestTerrainHeight(Position[0],Position[1]);
			CreateEffect(BITMAP_FLAME,Position,o->Angle,o->Light,0,o,o->PKKey,FindHotKey( AT_SKILL_FLAME));//클라이언트마법처리

            if ( c == Hero)
			{
				++CharacterMachine->PacketSerial;
			}
			if (SceneFlag != LOG_IN_SCENE)
				PlayBuffer(SOUND_FLAME);
			break;
		case AT_SKILL_STORM:
			CreateEffect(MODEL_STORM,o->Position,o->Angle,Light,0,o,o->PKKey,FindHotKey( AT_SKILL_STORM ));//클라이언트마법처리
            PlayBuffer(SOUND_STORM);
			break;
			
#ifdef PJH_SEASON4_MASTER_RANK4
	case AT_SKILL_FIRE_SCREAM_UP:
	case AT_SKILL_FIRE_SCREAM_UP+1:
	case AT_SKILL_FIRE_SCREAM_UP+2:
	case AT_SKILL_FIRE_SCREAM_UP+3:
	case AT_SKILL_FIRE_SCREAM_UP+4:
#endif //PJH_SEASON4_MASTER_RANK4
		case AT_SKILL_DARK_SCREAM:
			{
			int SkillIndex = FindHotKey( ( c->Skill ));			
			OBJECT* pObj = o;
			vec3_t ap,P,dp;
			float BkO = pObj->Angle[2];

			VectorCopy(pObj->Position,ap);
			CreateEffect(MODEL_DARK_SCREAM ,pObj->Position,pObj->Angle,pObj->Light,0,pObj,pObj->PKKey,SkillIndex);
			CreateEffect(MODEL_DARK_SCREAM_FIRE ,pObj->Position,pObj->Angle,pObj->Light,0,pObj,pObj->PKKey,SkillIndex);
			
			Vector(80.f,0.f,0.f,P);
			
			pObj->Angle[2] += 10.f;

			AngleMatrix(pObj->Angle,pObj->Matrix);
			VectorRotate(P,pObj->Matrix,dp);
			VectorAdd(dp,pObj->Position,pObj->Position);
			CreateEffect(MODEL_DARK_SCREAM ,pObj->Position,pObj->Angle,pObj->Light,0,pObj,pObj->PKKey,SkillIndex);
			CreateEffect(MODEL_DARK_SCREAM_FIRE ,pObj->Position,pObj->Angle,pObj->Light,0,pObj,pObj->PKKey,SkillIndex);

			VectorCopy(ap,pObj->Position);
			VectorCopy(pObj->Position,ap);
			Vector(-80.f,0.f,0.f,P);
			pObj->Angle[2] -= 20.f; 

			AngleMatrix(pObj->Angle,pObj->Matrix);
			VectorRotate(P,pObj->Matrix,dp);
			VectorAdd(dp,pObj->Position,pObj->Position);
			CreateEffect(MODEL_DARK_SCREAM ,pObj->Position,pObj->Angle,pObj->Light,0,pObj,pObj->PKKey,SkillIndex);
			CreateEffect(MODEL_DARK_SCREAM_FIRE ,pObj->Position,pObj->Angle,pObj->Light,0,pObj,pObj->PKKey,SkillIndex);
			VectorCopy(ap,pObj->Position);
			pObj->Angle[2] = BkO;

			if ( (c->Helper.Type>=MODEL_HELPER+2 && c->Helper.Type<=MODEL_HELPER+4) && !c->SafeZone )
			{
				SetAction ( o, PLAYER_ATTACK_RIDE_STRIKE );
			}
			else if(c->Helper.Type == MODEL_HELPER+37 && !c->SafeZone)
			{
				SetAction(o, PLAYER_FENRIR_ATTACK_DARKLORD_STRIKE);	//^ 펜릴 스킬 관련
			}
			else
			{
				SetAction ( o, PLAYER_ATTACK_STRIKE );
			}
			
			// 사운드 안나는 현상
			PlayBuffer(SOUND_FIRE_SCREAM);

			}
			break;
			
		case AT_SKILL_FLASH:
			CalcAddPosition(o,-20.f,-90.f,100.f,Position);
			CreateEffect(BITMAP_BOSS_LASER,Position,o->Angle,Light,0,o,o->PKKey,FindHotKey( AT_SKILL_FLASH ));//클라이언트마법처리
            PlayBuffer(SOUND_FLASH);
			break;
		case AT_SKILL_BLAST_UP:
		case AT_SKILL_BLAST_UP+1:
		case AT_SKILL_BLAST_UP+2:
		case AT_SKILL_BLAST_UP+3:
		case AT_SKILL_BLAST_UP+4:
		case AT_SKILL_BLAST:
			Position[0] = (float)(c->SkillX+0.5f)*TERRAIN_SCALE;
			Position[1] = (float)(c->SkillY+0.5f)*TERRAIN_SCALE;
			Position[2] = RequestTerrainHeight(Position[0],Position[1]);
            {
                int SkillIndex = FindHotKey( ( c->Skill ));
			    CreateEffect(MODEL_SKILL_BLAST,Position,o->Angle,o->Light,0,o,o->PKKey,SkillIndex);//클라이언트마법처리
			    CreateEffect(MODEL_SKILL_BLAST,Position,o->Angle,o->Light,0,o,o->PKKey,SkillIndex);//클라이언트마법처리
            }

            if ( c == Hero)
			{
				++CharacterMachine->PacketSerial;
			}
			break;
		case AT_SKILL_INFERNO:
			CreateInferno(o->Position);
            CreateEffect(MODEL_SKILL_INFERNO,o->Position,o->Angle,o->Light,0,o,o->PKKey,FindHotKey( AT_SKILL_INFERNO));

            if ( c == Hero)
			{
				++CharacterMachine->PacketSerial;
			}
			break;
#ifdef PJH_SEASON4_MASTER_RANK4
		case AT_SKILL_EVIL_SPIRIT_UP_M:	//악령강화(마검사용)
		case AT_SKILL_EVIL_SPIRIT_UP_M+1:	//악령강화
		case AT_SKILL_EVIL_SPIRIT_UP_M+2:	//악령강화
		case AT_SKILL_EVIL_SPIRIT_UP_M+3:	//악령강화
		case AT_SKILL_EVIL_SPIRIT_UP_M+4:	//악령강화
		case AT_SKILL_EVIL_SPIRIT_UP:	//악령강화(흑마법사용)
		case AT_SKILL_EVIL_SPIRIT_UP+1:	//악령강화
		case AT_SKILL_EVIL_SPIRIT_UP+2:	//악령강화
		case AT_SKILL_EVIL_SPIRIT_UP+3:	//악령강화
		case AT_SKILL_EVIL_SPIRIT_UP+4:	//악령강화
#endif //PJH_SEASON4_MASTER_RANK4
		case AT_SKILL_EVIL:
			vec3_t Position;
			VectorCopy(o->Position,Position);
			Position[2] += 100.f;

			for(int i=0;i<4;i++)
			{
				vec3_t Angle;
				Vector(0.f,0.f,i*90.f,Angle);

                int SkillIndex = FindHotKey( 
#ifdef PJH_SEASON4_MASTER_RANK4
					( c->Skill )
#else
					AT_SKILL_EVIL
#endif //PJH_SEASON4_MASTER_RANK4
					);
				CreateJoint(BITMAP_JOINT_SPIRIT,Position,o->Position,Angle,0,o,80.f,o->PKKey,SkillIndex,o->m_bySkillSerialNum);//클라이언트마법처리
				CreateJoint(BITMAP_JOINT_SPIRIT,Position,o->Position,Angle,0,o,20.f);
			}
			if ( c == Hero)
			{
				++CharacterMachine->PacketSerial;
			}
            PlayBuffer(SOUND_EVIL);
			break;
		case AT_SKILL_PLASMA_STORM_FENRIR:	//^ 펜릴 스킬 관련
		{
			// 사운드
			PlayBuffer(SOUND_FENRIR_SKILL);  // 펜릴 사운드 관련(스킬)

			// 몬스터 검사
			CHARACTER* p_temp_c;
			OBJECT* p_o[MAX_FENRIR_SKILL_MONSTER_NUM];
			int iMonsterNum = 0;

			for(int i=0; i<MAX_CHARACTERS_CLIENT; i++)
			{
				p_temp_c = &CharactersClient[i];	

				if(p_temp_c->Object.Live == TRUE && p_temp_c->Object.Kind == KIND_MONSTER && p_temp_c->Object.CurrentAction != MONSTER01_DIE)	// 몬스터이고 살아있으면
				{	// 몬스터
					float dx = c->Object.Position[0] - p_temp_c->Object.Position[0]; // 자기와의 거리를 계산한다.
					float dy = c->Object.Position[1] - p_temp_c->Object.Position[1];
					float fDistance = sqrtf(dx*dx+dy*dy) / TERRAIN_SCALE;
					if(fDistance <= GetSkillDistance(AT_SKILL_PLASMA_STORM_FENRIR))
					{
						p_o[iMonsterNum] = &p_temp_c->Object;
						iMonsterNum++;
					}
				}
				else if(p_temp_c->Object.Live == TRUE && p_temp_c->Object.Kind == KIND_PLAYER && p_temp_c->Object.CurrentAction != PLAYER_DIE1)
				{	
					// 플레이어
					if( CheckAttack_Fenrir(p_temp_c) == true && CInput::Instance().IsKeyDown(VK_LCONTROL) )
					{
						float dx = c->Object.Position[0] - p_temp_c->Object.Position[0];
						float dy = c->Object.Position[1] - p_temp_c->Object.Position[1];
						float fDistance = sqrtf(dx*dx+dy*dy) / TERRAIN_SCALE;
						if(fDistance <= GetSkillDistance(AT_SKILL_PLASMA_STORM_FENRIR))
						{
							p_o[iMonsterNum] = &p_temp_c->Object;
							iMonsterNum++;
						}
					}
				}

				if(iMonsterNum >= 10)	// 최대 10마리 까지
					break;
			}

			//^ 펜릴 이펙트 관련 (스킬 이펙트 연출)
			vec3_t vAngle;
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
			int iAngle = rand()%360;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING

			if(c->m_iFenrirSkillTarget != -1 && c->m_iFenrirSkillTarget < MAX_CHARACTERS_CLIENT)
			{
				CHARACTER* p_tc = &CharactersClient[c->m_iFenrirSkillTarget];
				OBJECT* p_to = &p_tc->Object;

				// 타켓한테
				for(int j=0; j<2; j++)
				{
					CalcAddPosition(o, 0.f, -140.f, 130.f, Position);
					Vector((float)(rand()%360), 0.0f, (float)(rand()%360), vAngle);
					
					CreateJoint(MODEL_FENRIR_SKILL_THUNDER, Position, p_to->Position, vAngle, 0+GetFenrirType(c), p_to, 100.f);
					CreateJoint(MODEL_FENRIR_SKILL_THUNDER, Position, p_to->Position, vAngle, 3+GetFenrirType(c), p_to, 80.f);
				}

				for(int i=0; i<iMonsterNum; i++)
				{
					for(int j=0; j<2; j++)
					{
						CalcAddPosition(o, 0.f, -140.f, 130.f, Position);
						Vector((float)(rand()%360), 0.0f, (float)(rand()%360), vAngle);
						
						CreateJoint(MODEL_FENRIR_SKILL_THUNDER, Position, p_o[i]->Position, vAngle, 0+GetFenrirType(c), p_o[i], 100.f);
						CreateJoint(MODEL_FENRIR_SKILL_THUNDER, Position, p_o[i]->Position, vAngle, 4+GetFenrirType(c), p_o[i], 80.f);
					}
				}
				
				for(int k=0; k<6; k++)
				{
					CalcAddPosition(o, 0.f, 10.f+(rand()%40-20), 130.f, Position);
					Vector((float)(rand()%360), 0.0f, (float)(rand()%360), vAngle);
					CreateJoint ( BITMAP_FLARE_FORCE, Position, Position, vAngle, 11+GetFenrirType(c), NULL, 60.f );
				}
			}
			break;
		}
        case AT_SKILL_RUSH:     //  기사 돌격 스킬.
			CreateEffect( MODEL_SWORD_FORCE, o->Position, o->Angle, Light, 0, o );
            PlayBuffer ( SOUND_BCS_RUSH );
            break;
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
		case AT_SKILL_OCCUPY:
			{
				CreateEffect(MODEL_SWORD_FORCE, o->Position, o->Angle, Light, 2, o);
				PlayBuffer (SOUND_BCS_RUSH);
			}
			break;
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
        case AT_SKILL_ONEFLASH:
            break;

        case AT_SKILL_BRAND_OF_SKILL:
            Vector ( 0.f, 0.f, 0.f, p );
            Vector ( 1.f, 1.f, 1.f, Light );
            if ( c->Weapon[0].Type!=MODEL_BOW+15 )
            {
                b->TransformPosition ( o->BoneTransform[c->Weapon[0].LinkBone], p, Position, true );
                CreateEffect ( MODEL_DARKLORD_SKILL, Position, o->Angle, Light, 0 );
            }
            if ( c->Weapon[1].Type!=MODEL_BOW+7 && ( c->Weapon[1].Type<MODEL_SHIELD || c->Weapon[1].Type>=MODEL_SHIELD+MAX_ITEM_INDEX ) )
            {
                b->TransformPosition ( o->BoneTransform[c->Weapon[1].LinkBone], p, Position, true );
                CreateEffect ( MODEL_DARKLORD_SKILL, Position, o->Angle, Light, 1 );
            }
            CreateEffect ( MODEL_MANA_RUNE, o->Position, o->Angle, o->Light );
         
            PlayBuffer ( SOUND_BCS_BRAND_OF_SKILL );
            break;
#ifdef ASG_ADD_SKILL_BERSERKER
		case AT_SKILL_ALICE_BERSERKER:
			// 바닥
			Vector(1.0f, 0.1f, 0.2f, Light);
			CreateEffect(BITMAP_MAGIC+1, o->Position, o->Angle, Light, 11, o);
			// 도넛츠
			CreateEffect(MODEL_ALICE_BUFFSKILL_EFFECT, o->Position, o->Angle, Light, 0, o);
			CreateEffect(MODEL_ALICE_BUFFSKILL_EFFECT2, o->Position, o->Angle, Light, 0, o);
#ifdef ASG_ADD_SKILL_BERSERKER_SOUND
			PlayBuffer(SOUND_SKILL_BERSERKER);
#endif	// ASG_ADD_SKILL_BERSERKER_SOUND
			break;
#endif	// ASG_ADD_SKILL_BERSERKER

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
			CreateEffect(BITMAP_SHINY+6, o->Position, o->Angle, Light, 0, NULL, -1, 0, 0, 0, 0.5f);
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
			CreateEffect(BITMAP_SHINY+6, o->Position, o->Angle, Light, 0, NULL, -1, 0, 0, 0, 0.5f);
			CreateEffect(BITMAP_PIN_LIGHT, o->Position, o->Angle, Light, 0, NULL, -1, 0, 0, 0, 1.f);
			PlayBuffer(SOUND_SKILL_ENERVATION);
			break;

#ifdef YDG_ADD_SKILL_FLAME_STRIKE
		case AT_SKILL_FLAME_STRIKE:
			{
				//DeleteEffect(MODEL_EFFECT_FLAME_STRIKE, o, 0);
				CreateEffect(MODEL_EFFECT_FLAME_STRIKE, o->Position, o->Angle, o->Light, 0, o);
#ifdef YDG_ADD_SKILL_FLAME_STRIKE_SOUND
				PlayBuffer(SOUND_SKILL_FLAME_STRIKE);
#endif	// YDG_ADD_SKILL_FLAME_STRIKE_SOUND
			}
			break;
#endif	// YDG_ADD_SKILL_FLAME_STRIKE
#ifdef YDG_ADD_SKILL_GIGANTIC_STORM
		case AT_SKILL_GIGANTIC_STORM:
			{
				vec34_t Matrix;
				vec3_t vAngle, vDirection, vPosition;
				float fAngle;
				for (int i = 0; i < 5; ++i)
				{
					Vector(0.f, 200.f, 0.f, vDirection );
					fAngle = o->Angle[2] + i * 72.f;
					Vector(0.f, 0.f, fAngle, vAngle);
					AngleMatrix(vAngle, Matrix);
					VectorRotate(vDirection, Matrix, vPosition);
					VectorAdd(vPosition, o->Position, vPosition);
					
					CreateEffect(BITMAP_JOINT_THUNDER, vPosition, o->Angle, o->Light);
				}
#ifdef YDG_ADD_SKILL_GIGANTIC_STORM_SOUND
				PlayBuffer(SOUND_SKILL_GIGANTIC_STORM);
#endif	// YDG_ADD_SKILL_GIGANTIC_STORM_SOUND
			}
			break;
#endif	// YDG_ADD_SKILL_GIGANTIC_STORM
#ifdef YDG_ADD_SKILL_LIGHTNING_SHOCK
#ifdef PJH_ADD_MASTERSKILL
		case AT_SKILL_LIGHTNING_SHOCK_UP:
		case AT_SKILL_LIGHTNING_SHOCK_UP+1:
		case AT_SKILL_LIGHTNING_SHOCK_UP+2:
		case AT_SKILL_LIGHTNING_SHOCK_UP+3:
		case AT_SKILL_LIGHTNING_SHOCK_UP+4:
#endif
		case AT_SKILL_LIGHTNING_SHOCK:
			{
// 				CHARACTER *tc = &CharactersClient[c->TargetCharacter];
// 				OBJECT *to = &tc->Object;
// 
// 				CreateEffect(MODEL_LIGHTNING_SHOCK, to->Position, to->Angle, to->Light, 2, to);

				vec3_t vLight;
				Vector(1.0f, 1.0f, 1.0f, vLight);

				CreateEffect(MODEL_LIGHTNING_SHOCK, o->Position, o->Angle, vLight, 0, o);
#ifdef YDG_ADD_SKILL_LIGHTNING_SHOCK_SOUND
				PlayBuffer(SOUND_SKILL_LIGHTNING_SHOCK);
#endif	// YDG_ADD_SKILL_LIGHTNING_SHOCK_SOUND
			}
			break;
#endif	// YDG_ADD_SKILL_LIGHTNING_SHOCK
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
		case AT_SKILL_THRUST:
			{
				o->Angle[2] = CreateAngle(o->Position[0],o->Position[1],c->TargetPosition[0],c->TargetPosition[1]);
				o->m_sTargetIndex = c->TargetCharacter;
			}
			break;
		case AT_SKILL_STAMP:
			{
				o->m_sTargetIndex = c->TargetCharacter;
			}
			break;
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
		}

		if(c->TargetCharacter == -1)
		{
			BYTE Skill = 0;
			if(( c->Skill)==AT_SKILL_CROSSBOW || (AT_SKILL_MANY_ARROW_UP <= ( c->Skill) && ( c->Skill) <= AT_SKILL_MANY_ARROW_UP+4))
				Skill = 1;
			if ( ( o->Type==MODEL_PLAYER && 
			     ( o->CurrentAction==PLAYER_ATTACK_BOW || o->CurrentAction==PLAYER_ATTACK_CROSSBOW ||
				   o->CurrentAction==PLAYER_ATTACK_FLY_BOW || o->CurrentAction==PLAYER_ATTACK_FLY_CROSSBOW ||
				   o->CurrentAction==PLAYER_FENRIR_ATTACK_BOW || o->CurrentAction==PLAYER_FENRIR_ATTACK_CROSSBOW ||	//^ 펜릴 스킬 관련(요정 화살)
				   o->CurrentAction==PLAYER_ATTACK_RIDE_BOW || o->CurrentAction==PLAYER_ATTACK_RIDE_CROSSBOW ) ) || 
                   o->Type!=MODEL_PLAYER && o->Kind==KIND_PLAYER 
                )
            {
#ifdef PJH_SEASON4_SPRITE_NEW_SKILL_MULTI_SHOT
				if(AT_SKILL_MULTI_SHOT != ( c->Skill))
#endif //PJH_SEASON4_SPRITE_NEW_SKILL_MULTI_SHOT
				CreateArrows(c,o,NULL,FindHotKey(( c->Skill)),Skill,( c->Skill));
            }

			if(o->Type==MODEL_MONSTER01+22 || o->Type==MODEL_MONSTER01+35 || o->Type==MODEL_MONSTER01+40
				|| o->Type==MODEL_MONSTER01+46) 
            {
				CreateArrows(c,o,NULL,0,0);
            }
		}
		else
		{
			CHARACTER *tc = &CharactersClient[c->TargetCharacter];
			OBJECT *to = &tc->Object;
			if(o->Type==MODEL_PLAYER && 
				(o->CurrentAction==PLAYER_ATTACK_BOW || o->CurrentAction==PLAYER_ATTACK_CROSSBOW ||
				o->CurrentAction==PLAYER_ATTACK_FLY_BOW || o->CurrentAction==PLAYER_ATTACK_FLY_CROSSBOW ||
				o->CurrentAction==PLAYER_ATTACK_RIDE_BOW || o->CurrentAction==PLAYER_ATTACK_RIDE_CROSSBOW
				|| o->CurrentAction==PLAYER_FENRIR_ATTACK_BOW || o->CurrentAction==PLAYER_FENRIR_ATTACK_CROSSBOW	//^ 펜릴 스킬 관련(요정 화살 나가게 하는 것)
				))
            {
#ifdef PJH_SEASON4_SPRITE_NEW_SKILL_MULTI_SHOT
				if(AT_SKILL_MULTI_SHOT != ( c->Skill))
#endif //PJH_SEASON4_SPRITE_NEW_SKILL_MULTI_SHOT
				CreateArrows(c,o,to,FindHotKey(( c->Skill)),0,( c->Skill));
            }

            if(o->Type==MODEL_MONSTER01+22 || o->Type==MODEL_MONSTER01+35 || o->Type==MODEL_MONSTER01+40) 
            {
				CreateArrows(c,o,to,0,0);
            }

			if(tc->Hit >= 1)
			{
                if ( to->Type != MODEL_MONSTER01+7 )
                {
					for(int i=0; i<10; i++)
				    {
					    Vector(to->Position[0]+(float)(rand()%64-32),to->Position[1]+(float)(rand()%64-32),to->Position[2]+(float)(rand()%64+90),Position);
					    CreateParticle(BITMAP_BLOOD+1,Position,o->Angle,Light);
				    }
                }

                //  성문, 크리스탈 타격시 효과 처리.
                if ( to->Type==MODEL_MONSTER01+60 )  //  석관.
                {
					for(int i=0;i<5;i++)
                    {
                        if ( (rand()%2)==0 )
                        {
                            Position[0] = to->Position[0];
                            Position[1] = to->Position[1];
                            Position[2] = to->Position[2]+50+rand()%30;

    					    CreateEffect(MODEL_STONE_COFFIN+1,Position,o->Angle,o->Light);
                        }
                    }
                    PlayBuffer ( SOUND_HIT_CRISTAL );
                }
                battleCastle::RenderMonsterHitEffect ( to );
            }
			if(o->CurrentAction>=PLAYER_ATTACK_SKILL_SWORD1 && o->CurrentAction<=PLAYER_ATTACK_SKILL_SWORD5)
			{
				CreateSpark(0,tc,to->Position,o->Angle);
			}
      	    vec3_t Angle;
			VectorCopy(o->Angle,Angle);
			Angle[2] = CreateAngle(o->Position[0],o->Position[1],to->Position[0],to->Position[1]);
			switch(( c->Skill))
			{
			case AT_SKILL_MANY_ARROW_UP:
			case AT_SKILL_MANY_ARROW_UP+1:
			case AT_SKILL_MANY_ARROW_UP+2:
			case AT_SKILL_MANY_ARROW_UP+3:
			case AT_SKILL_MANY_ARROW_UP+4:
			case AT_SKILL_CROSSBOW:
				CreateArrows(c,o,NULL,FindHotKey(( c->Skill)),1);
            case AT_SKILL_PIERCING:
                CreateArrows(c,o,NULL,FindHotKey(( c->Skill)),0,( c->Skill));
                break;
            case AT_SKILL_PARALYZE:
                CreateArrows(c,o,NULL,FindHotKey(( c->Skill)),0,( c->Skill));
                break;
            case AT_SKILL_DEEPIMPACT:
                CreateArrows(c,o,to,FindHotKey(( c->Skill)),0,( c->Skill));
                PlayBuffer ( SOUND_BCS_DEEP_IMPACT );
                break;
			case AT_SKILL_HEAL_UP:
			case AT_SKILL_HEAL_UP+1:
			case AT_SKILL_HEAL_UP+2:
			case AT_SKILL_HEAL_UP+3:
			case AT_SKILL_HEAL_UP+4:
			case AT_SKILL_HEALING:
				CreateEffect(BITMAP_MAGIC+1,to->Position,to->Angle,to->Light,1,to);
				break;
#ifdef PJH_SEASON4_MASTER_RANK4
	case AT_SKILL_DEF_POWER_UP:
	case AT_SKILL_DEF_POWER_UP+1:
	case AT_SKILL_DEF_POWER_UP+2:
	case AT_SKILL_DEF_POWER_UP+3:
	case AT_SKILL_DEF_POWER_UP+4:
#endif //PJH_SEASON4_MASTER_RANK4
			case AT_SKILL_DEFENSE:
				if(c->SkillSuccess)
                {
                    //  투명일때 보조계열 효과는 보이지 않는다.
					if( g_isCharacterBuff(o, eBuff_Cloaking) )
					{
						break;
					}

				    CreateEffect(BITMAP_MAGIC+1,to->Position,to->Angle,to->Light,2,to);

					if( !g_isCharacterBuff(to, eBuff_Defense) )
                    {
						g_CharacterRegisterBuff(to, eBuff_Defense );
                        
				        for(int j = 0; j < 5; ++j)
				        {
							CreateJoint( MODEL_SPEARSKILL, to->Position, to->Position, to->Angle, 4, to, 20.0f, -1, 0, 0, c->TargetCharacter);
				        }
                    }
                    else if ( !SearchJoint( MODEL_SPEARSKILL, to, 4 ) 
						&& !SearchJoint( MODEL_SPEARSKILL, to, 9 )	)
                    {
				        for ( int j = 0; j < 5; ++j)
				        {
							CreateJoint( MODEL_SPEARSKILL, to->Position, to->Position, to->Angle, 4, to, 20.0f, -1, 0, 0, c->TargetCharacter);
				        }
                    }
                }
				break;
#ifdef PJH_SEASON4_MASTER_RANK4
	case AT_SKILL_ATT_POWER_UP:
	case AT_SKILL_ATT_POWER_UP+1:
	case AT_SKILL_ATT_POWER_UP+2:
	case AT_SKILL_ATT_POWER_UP+3:
	case AT_SKILL_ATT_POWER_UP+4:
#endif //PJH_SEASON4_MASTER_RANK4
			case AT_SKILL_ATTACK:
                //  투명일때 보조계열 효과는 보이지 않는다.
				if( g_isCharacterBuff(o, eBuff_Cloaking) ) break;
				CreateEffect(BITMAP_MAGIC+1,to->Position,to->Angle,to->Light,3,to);
				if(c->SkillSuccess)
                {
					g_CharacterRegisterBuff(to, eBuff_Attack );
                }
				break;
			case AT_SKILL_SLOW:
				CreateEffect(MODEL_ICE,to->Position,o->Angle,Light);

				for(int i=0;i<5;i++)
					CreateEffect(MODEL_ICE_SMALL,to->Position,o->Angle,o->Light);

				if(c->SkillSuccess)
                {
					if( !g_isCharacterBuff(to, eDeBuff_Freeze) )
                    {
						g_CharacterRegisterBuff(to, eDeBuff_Freeze );
                    }
                }
                PlayBuffer(SOUND_ICE);
				break;
			case AT_SKILL_SOUL_UP:
			case AT_SKILL_SOUL_UP+1:
			case AT_SKILL_SOUL_UP+2:
			case AT_SKILL_SOUL_UP+3:
			case AT_SKILL_SOUL_UP+4:
			case AT_SKILL_WIZARDDEFENSE:
				if ( o->Type == MODEL_MONSTER01+55)
				{
					g_CharacterRegisterBuff(o, eBuff_PhysDefense );
				}
				else
				{
                    //  투명일때 보조계열 효과는 보이지 않는다.
					if( g_isCharacterBuff(to, eBuff_Cloaking) ) break;
                    g_CharacterRegisterBuff(to, eBuff_PhysDefense );

					// 중복 사용 피하기
					PlayBuffer(SOUND_SOULBARRIER);
					DeleteJoint( MODEL_SPEARSKILL, to, 0);
					for ( int j = 0; j < 5; ++j)
					{
						CreateJoint( MODEL_SPEARSKILL, to->Position, to->Position, to->Angle, 0, to, 20.0f);
					}
				}
				break;

			case AT_SKILL_POISON:
				if(o->Type == MODEL_PLAYER)
      				CreateEffect(MODEL_POISON,to->Position,o->Angle,o->Light);
				Vector(0.4f,0.6f,1.f,Light);

				for(int i=0;i<10;i++)
					CreateParticle(BITMAP_SMOKE,to->Position,o->Angle,Light,1);

				if(c->SkillSuccess)
                {
					g_CharacterRegisterBuff(to, eDeBuff_Poison );
                }
				PlayBuffer(SOUND_HEART);
				break;

			case AT_SKILL_METEO:
				CreateEffect(MODEL_FIRE,to->Position,to->Angle,o->Light);
				PlayBuffer(SOUND_METEORITE01);
				break;

	        case AT_SKILL_JAVELIN:
				CreateEffect ( MODEL_SKILL_JAVELIN, o->Position, o->Angle, o->Light, 0, to );
				CreateEffect ( MODEL_SKILL_JAVELIN, o->Position, o->Angle, o->Light, 1, to );
				CreateEffect ( MODEL_SKILL_JAVELIN, o->Position, o->Angle, o->Light, 2, to );

                PlayBuffer ( SOUND_BCS_JAVELIN );
                break;

            case AT_SKILL_DEATH_CANNON:
                Vector ( 0.f, 0.f, o->Angle[2], Angle );
                VectorCopy ( o->Position, Position );

                Position[2] += 130.f;
                CreateJoint ( BITMAP_JOINT_FORCE, Position, Position, Angle, 4, NULL, 40.f );

                PlayBuffer ( SOUND_BCS_DEATH_CANON );
                break;

            case AT_SKILL_SPACE_SPLIT:           //  공간 가르기.
                CreateEffect ( MODEL_PIER_PART, o->Position, o->Angle, o->Light, 2, to );
                PlayBuffer ( SOUND_BCS_SPACE_SPLIT );
                break;

			case AT_SKILL_FIREBALL:
				CreateEffect(MODEL_FIRE,o->Position,Angle,o->Light,1,to);
				PlayBuffer(SOUND_METEORITE01);
				break;

			case AT_SKILL_FLAME:
				Position[0] = to->Position[0];
				Position[1] = to->Position[1];
				Position[2] = RequestTerrainHeight(Position[0],Position[1]);
				CreateEffect(BITMAP_FLAME,Position,o->Angle,o->Light,5,o,o->PKKey,FindHotKey( AT_SKILL_FLAME));
				PlayBuffer(SOUND_FLAME);
				break;

			case AT_SKILL_POWERWAVE:
				if(o->Type == MODEL_MONSTER01+18)
				{
					Angle[2] += 10.f;
					CreateEffect(MODEL_MAGIC2,o->Position,Angle,o->Light);
					Angle[2] -= 20.f;
					CreateEffect(MODEL_MAGIC2,o->Position,Angle,o->Light);
					Angle[2] += 10.f;
				}
				CreateEffect(MODEL_MAGIC2,o->Position,Angle,o->Light);
				PlayBuffer(SOUND_MAGIC);
				break;

            case AT_SKILL_STRONG_PIER:
                //  스트롱피어.
                CreateEffect ( MODEL_WAVES, o->Position, o->Angle, o->Light, 1 );
                CreateEffect ( MODEL_WAVES, o->Position, o->Angle, o->Light, 1 );
                CreateEffect ( MODEL_PIERCING2, o->Position, o->Angle, o->Light );
                PlayBuffer ( SOUND_ATTACK_SPEAR );
                break;
			case AT_SKILL_FIRE_BUST_UP:
			case AT_SKILL_FIRE_BUST_UP+1:
			case AT_SKILL_FIRE_BUST_UP+2:
			case AT_SKILL_FIRE_BUST_UP+3:
			case AT_SKILL_FIRE_BUST_UP+4:
            case AT_SKILL_LONGPIER_ATTACK:
                {
                    //  롱피어 어택.
                    vec3_t Angle = { 0.f, 0.f, o->Angle[2] };
                    vec3_t Pos = { 0.f, 0.f, (to->BoundingBoxMax[2]/1.f) };

                    Vector ( 80.f, 0.f, 20.f, p );
                    b->TransformPosition ( o->BoneTransform[0], p, Position, true );
                    Angle[2] = o->Angle[2]+90;
                	CreateEffect ( MODEL_PIER_PART, Position, Angle, Pos, 0, to );
                    Pos[2] -= to->BoundingBoxMax[2]/2;
                    Angle[2] = o->Angle[2];
                	CreateEffect ( MODEL_PIER_PART, Position, Angle, Pos, 0, to );
                    Angle[2] = o->Angle[2]-90;
                	CreateEffect ( MODEL_PIER_PART, Position, Angle, Pos, 0, to );

                    Vector ( 1.f, 0.6f, 0.3f, Light );
                    CreateEffect ( MODEL_DARKLORD_SKILL, Position, o->Angle, Light, 0 );
                    CreateEffect ( MODEL_DARKLORD_SKILL, Position, o->Angle, Light, 1 );
                }
                break;

			case AT_SKILL_ENERGYBALL:
				switch ( c->MonsterIndex)
				{
				case 37://데빌, 저주받은 왕 등등
				case 46:
				case 61:
				case 66:
				case 69://별자리
				case 70://퀸레이너(비의여왕)
				case 73://흑룡
				case 75://흑룡2
				case 77://불사조

                case 89:   //  마법 해골.
                case 95:
                case 112:
                case 118:
                case 124:
                case 130:
                case 143:
                case 87 :	//. 자이언트오거1
	            case 93 :	//. 자이언트오거2
	            case 99 :	//. 자이언트오거3
	            case 116 :	//. 자이언트오거4
	            case 122 :	//. 자이언트오거5
	            case 128 :	//. 자이언트오거6
                case 141:
                case 163 :  //  카오스캐슬 궁수.
                case 165 :
                case 167 :
                case 169 :
                case 171 :
                case 173 :
				case 427:
#ifdef LOREN_RAVINE_EVENT
				case 303:
#endif
				case 293 :	//. 공성 사냥터 독골렘
					break;
				default:
					if (o->Type== MODEL_MONSTER01+12)
					{
						CreateEffect(MODEL_SNOW1,o->Position,Angle,o->Light,0,to);
					}
					else if (o->Type== MODEL_MONSTER01+134)	// 우씨 돌 던지기
					{
						CreateEffect(MODEL_WOOSISTONE,o->Position,Angle,o->Light,0,to);
					}
					else if( o->Type == MODEL_MONSTER01+138 ) // 사피트레스 가시 공격
					{
						vec3_t vLight;
						Vector( 1.0f, 1.0f, 1.0f, vLight );
						CreateEffect( MODEL_EFFECT_SAPITRES_ATTACK, o->Position, o->Angle, vLight, 0, to );
					}
					else
					{
						CreateEffect(BITMAP_ENERGY,o->Position,Angle,o->Light,0,to);
						PlayBuffer(SOUND_MAGIC);
					}
					break;
				}
				break;

			case AT_SKILL_ALICE_LIGHTNINGORB:	// 라이트닝오브
				{
					// 이펙트 발생 및 사운드 발생
					vec3_t vLight;
					Vector(1.0f, 1.0f, 1.0f, vLight);
					
					// 라이트닝 오브 이펙트 
					// 타켓과 충돌처리 때문에 to(target object) 넣어줌
					CreateEffect(MODEL_LIGHTNING_ORB, o->Position, o->Angle, vLight, 0, to);

					// 사운드
					PlayBuffer(SOUND_SUMMON_SKILL_LIGHTORB);
				}
				break;

			case AT_SKILL_ALICE_SLEEP:		// nukun
			case AT_SKILL_ALICE_BLIND:
#ifdef PJH_ADD_MASTERSKILL
		case AT_SKILL_ALICE_SLEEP_UP:
		case AT_SKILL_ALICE_SLEEP_UP+1:
		case AT_SKILL_ALICE_SLEEP_UP+2:
		case AT_SKILL_ALICE_SLEEP_UP+3:
		case AT_SKILL_ALICE_SLEEP_UP+4:
#endif
			case AT_SKILL_ALICE_THORNS:
				{
					int iSkillIndex = ( c->Skill);		
					vec3_t vLight;
					// 바닥
					if(iSkillIndex == AT_SKILL_ALICE_SLEEP
#ifdef PJH_ADD_MASTERSKILL
						|| (AT_SKILL_ALICE_SLEEP_UP <= iSkillIndex && iSkillIndex <= AT_SKILL_ALICE_SLEEP_UP+4)
#endif
						)
					{
						Vector(0.7f, 0.3f, 0.8f, vLight);
					}
					else if(iSkillIndex == AT_SKILL_ALICE_BLIND)
					{
						// 검정
						Vector(1.0f, 1.0f, 1.0f, vLight);
					}
					else if(iSkillIndex == AT_SKILL_ALICE_THORNS
						)
					{
						Vector(0.8f, 0.5f, 0.2f, vLight);
					}
					
					if(iSkillIndex == AT_SKILL_ALICE_SLEEP || iSkillIndex == AT_SKILL_ALICE_THORNS
#ifdef PJH_ADD_MASTERSKILL
						|| (AT_SKILL_ALICE_SLEEP_UP <= iSkillIndex && iSkillIndex <= AT_SKILL_ALICE_SLEEP_UP+4)
#endif
						)
					{
						CreateEffect(BITMAP_MAGIC+1, o->Position, o->Angle, vLight, 11, o);
					}
					else if(iSkillIndex == AT_SKILL_ALICE_BLIND)
					{
						CreateEffect(BITMAP_MAGIC+1, o->Position, o->Angle, vLight, 12, o);
					}

					// 도넛츠
					if(iSkillIndex == AT_SKILL_ALICE_SLEEP
#ifdef PJH_ADD_MASTERSKILL
						|| (AT_SKILL_ALICE_SLEEP_UP <= iSkillIndex && iSkillIndex <= AT_SKILL_ALICE_SLEEP_UP+4)
#endif
						)
					{
						Vector(0.8f, 0.3f, 0.9f, vLight);
						CreateEffect(MODEL_ALICE_BUFFSKILL_EFFECT, to->Position, to->Angle, vLight, 0, to);
						CreateEffect(MODEL_ALICE_BUFFSKILL_EFFECT2, to->Position, to->Angle, vLight, 0, to);
					}
					else if(iSkillIndex == AT_SKILL_ALICE_BLIND)
					{
						// 검정
						Vector(1.0f, 1.0f, 1.0f, vLight);		
						CreateEffect(MODEL_ALICE_BUFFSKILL_EFFECT, to->Position, to->Angle, vLight, 1, to);
						CreateEffect(MODEL_ALICE_BUFFSKILL_EFFECT2, to->Position, to->Angle, vLight, 1, to);
					}
					else if(iSkillIndex == AT_SKILL_ALICE_THORNS)
					{
						Vector(0.8f, 0.5f, 0.2f, vLight);	
						CreateEffect(MODEL_ALICE_BUFFSKILL_EFFECT, to->Position, to->Angle, vLight, 2, to);
						CreateEffect(MODEL_ALICE_BUFFSKILL_EFFECT2, to->Position, to->Angle, vLight, 2, to);
					}
				}
				break;
				
			// ChainLighting	
			// 서버로부터 패킷을 받은 후 에 들어온다.
#ifdef PJH_ADD_MASTERSKILL
		case AT_SKILL_ALICE_CHAINLIGHTNING_UP:
		case AT_SKILL_ALICE_CHAINLIGHTNING_UP+1:
		case AT_SKILL_ALICE_CHAINLIGHTNING_UP+2:
		case AT_SKILL_ALICE_CHAINLIGHTNING_UP+3:
		case AT_SKILL_ALICE_CHAINLIGHTNING_UP+4:
#endif
			case AT_SKILL_ALICE_CHAINLIGHTNING:	// 체인라이트닝 기본이펙트
				{	
					// ReceiveChainMagic() 안에서 이펙트호출하였음.
					PlayBuffer(SOUND_SKILL_CHAIN_LIGHTNING);
				}
				break;
#ifdef PJH_ADD_MASTERSKILL
		case AT_SKILL_ALICE_DRAINLIFE_UP:
		case AT_SKILL_ALICE_DRAINLIFE_UP+1:
		case AT_SKILL_ALICE_DRAINLIFE_UP+2:
		case AT_SKILL_ALICE_DRAINLIFE_UP+3:
		case AT_SKILL_ALICE_DRAINLIFE_UP+4:
#endif
			case AT_SKILL_ALICE_DRAINLIFE:			// pruarin2
				{	
					CHARACTER *pTargetChar = &CharactersClient[c->TargetCharacter];
					OBJECT* pSourceObj = o;
					pSourceObj->Owner = &(pTargetChar->Object);
					
					CreateEffect(MODEL_ALICE_DRAIN_LIFE, pSourceObj->Position, pSourceObj->Angle, pSourceObj->Light, 0, pSourceObj );
					PlayBuffer(SOUND_SKILL_DRAIN_LIFE);
				}
				break;
			}
			
			VectorCopy(to->Position,Position);
			Position[2] += 120.f;
			
			int Hand = 0;
			if(o->CurrentAction==PLAYER_ATTACK_SWORD_LEFT1 || o->CurrentAction==PLAYER_ATTACK_SWORD_LEFT2)
				Hand = 1;
			
			if(tc == Hero)
			{
             	Vector(1.f,0.f,0.f,Light);
			}
			else
			{
             	Vector(1.f,0.6f,0.f,Light);
			}
			
			switch(c->AttackFlag)
			{
			case ATTACK_DIE:
      			CreateJoint(BITMAP_JOINT_ENERGY,to->Position,to->Position,o->Angle,0,o,20.f);
      			CreateJoint(BITMAP_JOINT_ENERGY,to->Position,to->Position,o->Angle,1,o,20.f);
				break;
			}

			switch(( c->Skill))
			{
			case AT_SKILL_HEALING:
#ifdef PJH_SEASON4_MASTER_RANK4
	case AT_SKILL_ATT_POWER_UP:
	case AT_SKILL_ATT_POWER_UP+1:
	case AT_SKILL_ATT_POWER_UP+2:
	case AT_SKILL_ATT_POWER_UP+3:
	case AT_SKILL_ATT_POWER_UP+4:
#endif //PJH_SEASON4_MASTER_RANK4
			case AT_SKILL_ATTACK:
#ifdef PJH_SEASON4_MASTER_RANK4
	case AT_SKILL_DEF_POWER_UP:
	case AT_SKILL_DEF_POWER_UP+1:
	case AT_SKILL_DEF_POWER_UP+2:
	case AT_SKILL_DEF_POWER_UP+3:
	case AT_SKILL_DEF_POWER_UP+4:
#endif //PJH_SEASON4_MASTER_RANK4
			case AT_SKILL_DEFENSE:
			case AT_SKILL_SUMMON:
			case AT_SKILL_SUMMON+1:
			case AT_SKILL_SUMMON+2:
			case AT_SKILL_SUMMON+3:
			case AT_SKILL_SUMMON+4:
			case AT_SKILL_SUMMON+5:
	        case AT_SKILL_SUMMON+6:
#ifdef ADD_ELF_SUMMON
			case AT_SKILL_SUMMON+7:
#endif // ADD_ELF_SUMMON
			case AT_SKILL_HEAL_UP:
			case AT_SKILL_HEAL_UP+1:
			case AT_SKILL_HEAL_UP+2:
			case AT_SKILL_HEAL_UP+3:
			case AT_SKILL_HEAL_UP+4:

			case AT_SKILL_SOUL_UP:
			case AT_SKILL_SOUL_UP+1:
			case AT_SKILL_SOUL_UP+2:
			case AT_SKILL_SOUL_UP+3:
			case AT_SKILL_SOUL_UP+4:
			case AT_SKILL_WIZARDDEFENSE:
#ifdef PJH_SEASON4_MASTER_RANK4
	case AT_SKILL_BLOW_UP:
	case AT_SKILL_BLOW_UP+1:
	case AT_SKILL_BLOW_UP+2:
	case AT_SKILL_BLOW_UP+3:
	case AT_SKILL_BLOW_UP+4:
#endif	//PJH_SEASON4_MASTER_RANK4
			case AT_SKILL_ONETOONE:
			case AT_SKILL_SPEAR:			//	창찌르기
			case AT_SKILL_LIFE_UP:
			case AT_SKILL_LIFE_UP+1:
			case AT_SKILL_LIFE_UP+2:
			case AT_SKILL_LIFE_UP+3:
			case AT_SKILL_LIFE_UP+4:
            case AT_SKILL_VITALITY:
			case AT_SKILL_BLAST_HELL:
            case AT_SKILL_IMPROVE_AG:
            case AT_SKILL_ADD_CRITICAL:     //  크리티컬 데미지 확률 증가.
            case AT_SKILL_PARTY_TELEPORT:   //  파티원 소환.
            case AT_SKILL_STUN:				//  배틀마스터 스킬.
            case AT_SKILL_REMOVAL_STUN:
            case AT_SKILL_MANA:
            case AT_SKILL_INVISIBLE:
            case AT_SKILL_REMOVAL_BUFF:
            case AT_SKILL_BRAND_OF_SKILL://  다크로드 스킬의 낙인.
				break;
				
            default:
				if(68<=c->MonsterIndex && c->MonsterIndex<=75)
				{
				}
				else
				{
					ITEM *r = &CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT];
					ITEM *l = &CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT];
					
#ifndef CSK_FIX_BACKUPITEM_DAMAGE		// 정리할 때 지워야 하는 소스	
					// 장비창에서 장비를 피크했을 경우 공격을 당하면 백업이 되게 수정
					SEASON3B::CNewUIPickedItem* pPickedItem = SEASON3B::CNewUIInventoryCtrl::GetPickedItem();
					if(pPickedItem)
					{
						ITEM* pItemObj = pPickedItem->GetItem();
						if(pItemObj->ex_src_type == ITEM_EX_SRC_EQUIPMENT)
						{
							if(pItemObj->lineal_pos == EQUIPMENT_WEAPON_RIGHT || pItemObj->lineal_pos == EQUIPMENT_WEAPON_LEFT)	
							{
								SEASON3B::CNewUIInventoryCtrl::BackupPickedItem();
							}
						}
					}
#endif //! CSK_FIX_BACKUPITEM_DAMAGE	// 정리할 때 지워야 하는 소스
					
					if((r->Type>=ITEM_BOW && r->Type<ITEM_BOW+MAX_ITEM_INDEX) && (l->Type>=ITEM_BOW && l->Type<ITEM_BOW+MAX_ITEM_INDEX))
					{	//. 활일 경우 (원거리 공격)
						PlayBuffer(SOUND_ATTACK01+5+rand()%4,o);
					}
					else
					{
						PlayBuffer(SOUND_ATTACK01+rand()%4,o);
					}
				}
				break;
			}
		}
		FAKE_CODE( Pos_PreventModifyingSkill1);
Pos_PreventModifyingSkill1:
		FAKE_CODE( Pos_PreventModifyingSkill2);
Pos_PreventModifyingSkill2:
		

		// 모든 스킬을 동일하게 처리하도록 합니다.
		//	: 회오리 베기에대한 PostMoveProcess 처리가 되었기 때문에 굳이 Skill을 따로 처리 하지 않습니다.
#ifdef LDS_FIX_MODULE_POSTMOVECHARACTER_SKILL_WHEEL
		c->Skill      = 0;
#else // LDS_FIX_MODULE_POSTMOVECHARACTER_SKILL_WHEEL
		
		// 회오리 베기(회오리 베기 강화(흑기사,마검사) 포함)를 제외한 나머지 경우 Skill 초기화.
#ifdef LDS_FIX_WHENBLOODATTACK_DISSAPEARRIGHTSWORD	
		if(!((c->Skill == AT_SKILL_WHEEL 
			|| (AT_SKILL_TORNADO_SWORDA_UP <= c->Skill && c->Skill <= AT_SKILL_TORNADO_SWORDA_UP+4)) 
			|| (AT_SKILL_TORNADO_SWORDB_UP <= c->Skill && c->Skill <= AT_SKILL_TORNADO_SWORDB_UP+4))
			)
#endif // LDS_FIX_WHENBLOODATTACK_DISSAPEARRIGHTSWORD
		{
			c->Skill      = 0;	// 스킬 초기화 (박종훈 디버그)
		}
		
#ifdef LDS_FIX_INITSKILL_WHENARFER_AT_SKILL_WHEEL
		// 회오리 베기 사용후 일반공격시 회오리잔영남지 않기 위해 회오리베기 끝난후 회오리만 스킬 초기화
		if( c->Skill == AT_SKILL_WHEEL && 
			!( (AT_SKILL_TORNADO_SWORDA_UP <= c->Skill && c->Skill <= AT_SKILL_TORNADO_SWORDA_UP+4) 
			|| (AT_SKILL_TORNADO_SWORDB_UP <= c->Skill && c->Skill <= AT_SKILL_TORNADO_SWORDB_UP+4) ) )
		{
			c->Skill      = 0;
		}
#endif // LDS_FIX_INITSKILL_WHENARFER_AT_SKILL_WHEEL
		
#endif // LDS_FIX_MODULE_POSTMOVECHARACTER_SKILL_WHEEL

		
		c->Damage     = 0;
		c->AttackFlag = ATTACK_FAIL;
	}
#ifdef USE_SELFCHECKCODE
	SendCrcOfFunction( 19, 8, UseSkillWizard, 0x569D);
#endif
	FAKE_CODE( Pos_PreventModifyingSkill3);
Pos_PreventModifyingSkill3:

#ifdef USE_SELFCHECKCODE
	SendCrcOfFunction( 0, 7, AttackCharacterRange, 0x8324);
#endif

    if ( c->m_iDeleteTime>0 )
    {
        c->m_iDeleteTime--;
    }
    if ( c->m_iDeleteTime!=-128 && c->m_iDeleteTime<=0 )
    {
        c->m_iDeleteTime = -128;
        DeleteCharacter ( c, o );
    }
    battleCastle::SetBuildTimeLocation ( o );

    CreateWeaponBlur ( c, o, b );

    switch(o->Type)
	{
	case MODEL_BALL:
		CreateFire(0,o,0.f,0.f,0.f);
		break;
	}

    giPetManager::MovePet ( c );

#ifdef USE_SELFCHECKCODE
	END_OF_FUNCTION( Pos_SelfCheck01);
Pos_SelfCheck01:
	;
#endif

	if ( c->Dead>0 )
	{
		if( g_isCharacterBuff(o, eBuff_BlessPotion) ) g_CharacterUnRegisterBuff(o, eBuff_BlessPotion );
		if( g_isCharacterBuff(o, eBuff_SoulPotion) ) g_CharacterUnRegisterBuff(o, eBuff_SoulPotion );
	}
}

void PlayWalkSound()
{
    OBJECT *o = &Hero->Object;
	if(o->CurrentAction==PLAYER_FLY||o->CurrentAction==PLAYER_FLY_CROSSBOW)
	{
	}
	else
	{
		if(World==WD_2DEVIAS && (HeroTile!=3&&HeroTile<10))
		{
			PlayBuffer(SOUND_HUMAN_WALK_SNOW);
		}
		else if(World==WD_0LORENCIA && HeroTile==0)
		{
			PlayBuffer(SOUND_HUMAN_WALK_GRASS);
		}
		else if(World==WD_3NORIA && HeroTile==0)
		{
			PlayBuffer(SOUND_HUMAN_WALK_GRASS);
		}
        else if( ( World==WD_7ATLANSE || InHellas()
#ifdef YDG_ADD_MAP_DOPPELGANGER3
			|| World == WD_67DOPPLEGANGER3
#endif	// YDG_ADD_MAP_DOPPELGANGER3
			) && !Hero->SafeZone)
		{
			PlayBuffer(SOUND_HUMAN_WALK_SWIM);
		}
#ifdef CSK_ADD_MAP_ICECITY
		else if(IsIceCity())
		{
			PlayBuffer(SOUND_HUMAN_WALK_SNOW);
		}
#endif // CSK_ADD_MAP_ICECITY
#ifdef YDG_ADD_MAP_SANTA_TOWN
		else if(IsSantaTown())
		{
			PlayBuffer(SOUND_HUMAN_WALK_SNOW);
		}
#endif // YDG_ADD_MAP_SANTA_TOWN
		else
		{
			PlayBuffer(SOUND_HUMAN_WALK_GROUND);
		}
	}
}

#ifdef CSK_FIX_WOPS_K28674_ADD_DEFENSE

bool CheckFullSet(CHARACTER *c)
{
	int tmpLevel = 10;
	bool Success = true;
    int  start = 5, end = 1;

    EquipmentLevelSet = 0;

	bool bHero = false;
	
	if(c == Hero)
	{
		bHero = true;
		start = EQUIPMENT_BOOTS;
		end = EQUIPMENT_HELM;
	}

#ifdef PBG_FIX_DEFENSEVALUE_DARK
    if ( GetBaseClass(c->Class)==CLASS_DARK )
    {
        end = EQUIPMENT_ARMOR;
    }
#endif //PBG_FIX_DEFENSEVALUE_DARK

	if(bHero == true)
	{
		for(int i = start; i >= end; i--)
		{
#ifdef PBG_ADD_NEWCHAR_MONK
			if((GetBaseClass(c->Class)==CLASS_RAGEFIGHTER) && (i == EQUIPMENT_GLOVES))
				continue;
#endif //PBG_ADD_NEWCHAR_MONK

			if(CharacterMachine->Equipment[i].Type == -1)
			{
				Success = false;
				break;
			}
		}
		
		if(Success)
		{
			int Type = CharacterMachine->Equipment[EQUIPMENT_BOOTS].Type % MAX_ITEM_INDEX;
			tmpLevel = (CharacterMachine->Equipment[EQUIPMENT_BOOTS].Level >> 3) & 15;
			for(int i = start; i >= end; i--)	
			{
#ifdef PBG_ADD_NEWCHAR_MONK
				if((GetBaseClass(c->Class)==CLASS_RAGEFIGHTER) && (i == EQUIPMENT_GLOVES))
					continue;
#endif //PBG_ADD_NEWCHAR_MONK
				int Level = (CharacterMachine->Equipment[i].Level >> 3) & 15;
				if(Level<9)
				{
					EquipmentLevelSet = 0;
					Success = false;
					break;
				}
				if(Type != (CharacterMachine->Equipment[i].Type % MAX_ITEM_INDEX))
				{
					EquipmentLevelSet = 0;
					Success = false;
					break;
				}
				
				if(Level >= 9 && tmpLevel >= Level)
				{
					tmpLevel = Level;
					EquipmentLevelSet = Level;
				}
			}
		}

		g_bAddDefense = true;

		if(GetBaseClass(c->Class)==CLASS_DARK && Success)
		{
			if ( CharacterMachine->Equipment[EQUIPMENT_ARMOR].Type!=ITEM_ARMOR+15
				&& CharacterMachine->Equipment[EQUIPMENT_ARMOR].Type!=ITEM_ARMOR+20  
				&& CharacterMachine->Equipment[EQUIPMENT_ARMOR].Type!=ITEM_ARMOR+23
				&& CharacterMachine->Equipment[EQUIPMENT_ARMOR].Type!=ITEM_ARMOR+32
				&& CharacterMachine->Equipment[EQUIPMENT_ARMOR].Type!=ITEM_ARMOR+37 
				&& CharacterMachine->Equipment[EQUIPMENT_ARMOR].Type!=ITEM_ARMOR+47
				&& CharacterMachine->Equipment[EQUIPMENT_ARMOR].Type!=ITEM_ARMOR+48
				)
			{
				g_bAddDefense = false;
			}
		}
	}
	else
	{
		for(int i=5;i>=end;i--)
		{
			if(c->BodyPart[i].Type==-1 )
			{
				Success = false;
				break;
			}
		}
		
		if(Success)
		{
			int Type = (c->BodyPart[5].Type-MODEL_ITEM)%MAX_ITEM_INDEX;
			tmpLevel = c->BodyPart[5].Level&0xf;

			for(int i=5;i>=end;i--)	
			{
				int Level = c->BodyPart[i].Level&0xf;
				if(Level<9)
				{
					EquipmentLevelSet = 0;
					Success = false;
					break;
				}
				if(Type != (c->BodyPart[i].Type-MODEL_ITEM)%MAX_ITEM_INDEX)
				{
					EquipmentLevelSet = 0;
					Success = false;
					break;
				}
				
				if(Level>=9 && tmpLevel>=Level)
				{
					tmpLevel = Level;
					EquipmentLevelSet = Level;
				}
			}
		}

		g_bAddDefense = true;

		if(GetBaseClass(c->Class)==CLASS_DARK && Success)
		{
			if ( c->BodyPart[BODYPART_ARMOR].Type != ITEM_ARMOR+15
				&& c->BodyPart[BODYPART_ARMOR].Type != ITEM_ARMOR+20  
				&& c->BodyPart[BODYPART_ARMOR].Type != ITEM_ARMOR+23
				&& c->BodyPart[BODYPART_ARMOR].Type != ITEM_ARMOR+32
				&& c->BodyPart[BODYPART_ARMOR].Type != ITEM_ARMOR+37 
				&& c->BodyPart[BODYPART_ARMOR].Type != ITEM_ARMOR+47
				&& c->BodyPart[BODYPART_ARMOR].Type != ITEM_ARMOR+48
				)
			{
				g_bAddDefense = false;
			}
		}
	}
#ifdef PBG_FIX_CHAOS_GOLDSWORD
	if(InChaosCastle())
	{
		ChangeChaosCastleUnit(c);
	}
#endif //PBG_FIX_CHAOS_GOLDSWORD

	return Success;
}

#else // CSK_FIX_WOPS_K28674_ADD_DEFENSE

bool CheckFullSet(CHARACTER *c)
{
	int  i, tmpLevel = 10;
	bool Success = true;
    int  start = 5, end = 1;

    //  마검사.
    if ( GetBaseClass(c->Class)==CLASS_DARK )
    {
        end = 2;
    }

    EquipmentLevelSet = 0;

	for(i=5;i>=end;i--)
	{
		if(c->BodyPart[i].Type==-1 )
		{
			Success = false;
			break;
		}
	}
	
	if(Success)
	{
		int Type = (c->BodyPart[5].Type-MODEL_ITEM)%MAX_ITEM_INDEX;
        tmpLevel = c->BodyPart[5].Level&0xf;
		for(i=5;i>=end;i--)
		{
            int Level = c->BodyPart[i].Level&0xf;
			if(Level<9)
			{
                EquipmentLevelSet = 0;
				Success = false;
				break;
			}
			if(Type != (c->BodyPart[i].Type-MODEL_ITEM)%MAX_ITEM_INDEX)
			{
                EquipmentLevelSet = 0;
				Success = false;
				break;
			}
			
            if(Level>=9 && tmpLevel>=Level)
            {
                tmpLevel = Level;
                EquipmentLevelSet = Level;
            }
		}
	}

    g_bAddDefense = true;

    if(GetBaseClass(c->Class)==CLASS_DARK && Success)
    {
        //  아틀란스, 선더호크 추방 적용됨.
		if ( CharacterMachine->Equipment[EQUIPMENT_ARMOR].Type!=ITEM_ARMOR+15
			&& CharacterMachine->Equipment[EQUIPMENT_ARMOR].Type!=ITEM_ARMOR+20  
			&& CharacterMachine->Equipment[EQUIPMENT_ARMOR].Type!=ITEM_ARMOR+23
			&& CharacterMachine->Equipment[EQUIPMENT_ARMOR].Type!=ITEM_ARMOR+32
			&& CharacterMachine->Equipment[EQUIPMENT_ARMOR].Type!=ITEM_ARMOR+37 
#ifdef KJH_FIX_SOCKET_ITEM_ADD_DEFENCE_BONUS
			&& CharacterMachine->Equipment[EQUIPMENT_ARMOR].Type!=ITEM_ARMOR+47				// 팬텀
			&& CharacterMachine->Equipment[EQUIPMENT_ARMOR].Type!=ITEM_ARMOR+48				// 디스트로이
#endif // KJH_FIX_SOCKET_ITEM_ADD_DEFENCE_BONUS
			)
        {
            g_bAddDefense = false;
        }
    }

	return Success;
}

#endif // CSK_FIX_WOPS_K28674_ADD_DEFENSE

void MoveEye(OBJECT *o,BMD *b,int Right,int Left, int Right2, int Left2, int Right3, int Left3)
{
	vec3_t p;
	Vector(0.f,0.f,0.f,p);
	b->TransformPosition(o->BoneTransform[Right],p,o->EyeRight,true);
	Vector(0.f,0.f,0.f,p);
	b->TransformPosition(o->BoneTransform[Left],p,o->EyeLeft,true);
	if(Right2 != -1)
		b->TransformPosition(o->BoneTransform[Right2], p, o->EyeRight2, true);
	if(Left2 != -1)
		b->TransformPosition(o->BoneTransform[Left2], p, o->EyeLeft2, true);
	if(Right3 != -1)
		b->TransformPosition(o->BoneTransform[Right3], p, o->EyeRight3, true);
	if(Left3 != -1)
		b->TransformPosition(o->BoneTransform[Left3], p, o->EyeLeft3, true);
}

void MonsterDieSandSmoke(OBJECT *o)
{
	if(o->CurrentAction==MONSTER01_DIE && o->AnimationFrame>=8.f && o->AnimationFrame<9.f)
	{
		vec3_t Position;
		for(int i=0;i<20;i++)
		{
			Vector(1.f,1.f,1.f,o->Light);
			Vector(o->Position[0]+(float)(rand()%64-32),
				o->Position[1]+(float)(rand()%64-32),
				o->Position[2]+(float)(rand()%32-16),Position);
			CreateParticle(BITMAP_SMOKE+1,Position,o->Angle,o->Light,1);
		}
	}
}

void MonsterMoveSandSmoke(OBJECT *o)
{
	if(o->CurrentAction == MONSTER01_WALK)
	{
		vec3_t Position;
		Vector(o->Position[0]+rand()%200-100,o->Position[1]+rand()%200-100,o->Position[2],Position);
		CreateParticle(BITMAP_SMOKE+1,Position,o->Angle,o->Light);
	}
}

void MoveCharacterVisual(CHARACTER *c,OBJECT *o)
{
	BMD *b = &Models[o->Type];
    if(b->NumActions == 0)
	{
		VectorCopy(o->BoundingBoxMin,o->OBB.StartPos);
		o->OBB.XAxis[0] = (o->BoundingBoxMax[0] - o->BoundingBoxMin[0]);
		o->OBB.YAxis[1] = (o->BoundingBoxMax[1] - o->BoundingBoxMin[1]);
		o->OBB.ZAxis[2] = (o->BoundingBoxMax[2] - o->BoundingBoxMin[2]);
		VectorAdd(o->OBB.StartPos,o->Position,o->OBB.StartPos);
		o->OBB.XAxis[1] = 0.f;
		o->OBB.XAxis[2] = 0.f;
		o->OBB.YAxis[0] = 0.f;
		o->OBB.YAxis[2] = 0.f;
		o->OBB.ZAxis[0] = 0.f;
		o->OBB.ZAxis[1] = 0.f;
		return;
	}
	VectorCopy(o->Position,b->BodyOrigin); 
	b->BodyScale     = o->Scale;
    b->CurrentAction = o->CurrentAction;

	if(o->Visible)
	{
		int Index = TERRAIN_INDEX_REPEAT(( c->PositionX),( c->PositionY));
		if((TerrainWall[Index]&TW_SAFEZONE)==TW_SAFEZONE)
			c->SafeZone = true;
		else
			c->SafeZone = false;

		if( !g_isCharacterBuff(o, eDeBuff_Harden) && !g_isCharacterBuff(o, eDeBuff_Stun) 
			&& !g_isCharacterBuff(o, eDeBuff_Sleep) 
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
			&& !g_isCharacterBuff(o, eBuff_Att_up_Ourforces)
			&& !g_isCharacterBuff(o, eBuff_Hp_up_Ourforces)
			&& !g_isCharacterBuff(o, eBuff_Def_up_Ourforces)
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
			)
        {
            if(o->Type != MODEL_PLAYER)
                MoveHead(c);
            if(c!=Hero && c->Dead==0 && rand()%32==0)
            {
                o->HeadTargetAngle[0] = (float)(rand()%128-64);
                o->HeadTargetAngle[1] = (float)(rand()%32-16);
                for(int i=0;i<2;i++)
                    if(o->HeadTargetAngle[i] < 0) o->HeadTargetAngle[i] += 360.f;
            }

			for( int j=0;j<2;j++)
                o->HeadAngle[j] = TurnAngle2(o->HeadAngle[j],o->HeadTargetAngle[j],FarAngle(o->HeadAngle[j],o->HeadTargetAngle[j])*0.2f);
        }

    	vec3_t p,Position;
		vec3_t Light;
		float Luminosity = 0.8f;
		if(c->Appear > 0)
		{
			c->Appear--;
			for(int i=0;i<20;i++)
			{
				Vector(1.f,1.f,1.f,o->Light);
				Vector(o->Position[0]+(float)(rand()%64-32),
					o->Position[1]+(float)(rand()%64-32),
					o->Position[2]+(float)(rand()%32-16),Position);
				if(rand()%10==0)
					CreateParticle(BITMAP_SMOKE+1,Position,o->Angle,o->Light,1);
				if(rand()%10==0)
					CreateEffect(MODEL_STONE1+rand()%2,o->Position,o->Angle,o->Light);
			}
		}
		if(c->PK < PVP_MURDERER2)
		{
			for(int j=0;j<2;j++)
			{
				if(c->Weapon[j].Type==MODEL_SWORD+12)//태양검
				{
					Vector(Luminosity,Luminosity*0.8f,Luminosity*0.5f,Light);
					AddTerrainLight(o->Position[0],o->Position[1],Light,3,PrimaryTerrainLight);
				}
				else if(c->Weapon[j].Type==MODEL_SWORD+19 || c->Weapon[j].Type==MODEL_BOW+18 || c->Weapon[j].Type==MODEL_STAFF+10 || c->Weapon[j].Type==MODEL_MACE+13)
				{
					Vector(Luminosity*0.8f,Luminosity*0.5f,Luminosity*0.3f,Light);
					AddTerrainLight(o->Position[0],o->Position[1],Light,2,PrimaryTerrainLight);
				}
			}
		}
		if(c->Freeze>0.f)
		{
			if(c->FreezeType==BITMAP_ICE && rand()%4==0)
			{
				Vector(o->Position[0]+(float)(rand()%100-50),o->Position[1]+(float)(rand()%100-50),o->Position[2]+(float)(rand()%180),Position);
				//CreateParticle(BITMAP_SHINY,Position,o->Angle,o->Light);
				//CreateParticle(BITMAP_SHINY,Position,o->Angle,o->Light,1);
			}
			if(c->FreezeType==BITMAP_BURN)
			{
				Vector(0.f,0.f,-20.f,Position);
				for(int i=1;i<b->NumBones;)
				{
					if(!b->Bones[i].Dummy)
					{
						b->TransformPosition(o->BoneTransform[i],Position,p);
						//CreateParticle(BITMAP_FIRE+1,p,o->Angle,o->Light,rand()%4);
					}
					int d = (int)(4.f/c->Freeze);
					if(d < 1) d = 1;
					i += d;
				}
			}
		}
		Vector(1.f,1.f,1.f,Light);
		Vector(0.f,0.f,0.f,p);
		Luminosity = (float)(rand()%8+2)*0.1f;
		bool Smoke = false;
		switch(o->Type)
		{
		case MODEL_PLAYER:
     		if(SceneFlag==MAIN_SCENE && (World==WD_7ATLANSE
#ifdef YDG_ADD_MAP_DOPPELGANGER3
				|| World == WD_67DOPPLEGANGER3
#endif	// YDG_ADD_MAP_DOPPELGANGER3
				) && (int)WorldTime%10000<1000)
			{
         		Vector(0.f,20.f,-10.f,p);
				b->TransformPosition(o->BoneTransform[b->BoneHead],p,Position,true);
				CreateParticle(BITMAP_BUBBLE,Position,o->Angle,Light);
			}
			Vector(1.f,1.f,1.f,Light);
        	Vector(-15.f,0.f,0.f,p);
			if ( InDevilSquare() == true )
			{
				if(rand()%4==0)
				{
					b->TransformPosition(o->BoneTransform[26],p,Position,true);
					CreateParticle(BITMAP_RAIN_CIRCLE+1,Position,o->Angle,Light);
				}
				if(rand()%4==0)
				{
					b->TransformPosition(o->BoneTransform[35],p,Position,true);
					CreateParticle(BITMAP_RAIN_CIRCLE+1,Position,o->Angle,Light);
				}
			}
			if ( o->CurrentAction == PLAYER_SKILL_HELL_BEGIN || o->CurrentAction == PLAYER_SKILL_HELL_START )
			{
				if ( o->BoneTransform!=NULL )
				{
					Vector ( 0.3f, 0.3f, 1.f, Light );

					for ( int i=0; i<40; i+=2 )
					{
                        if ( !b->Bones[i].Dummy && i<b->NumBones )
                        {
						    b->TransformPosition(o->BoneTransform[i],p,Position,true);

						    for ( int j=0; j<o->m_bySkillCount+1; ++j )
						    {
							    CreateParticle( BITMAP_LIGHT, Position, o->Angle, Light, 6, 1.3f+(o->m_bySkillCount*0.08f) );
						    }
                        }
					}
				}
				VectorCopy ( o->Position, Position );
				CreateForce ( o, Position );
			}

			if ( o->CurrentAction == PLAYER_SKILL_HELL )
			{
				for(int i=0;i<10;i++)
				{
					b->TransformPosition(o->BoneTransform[rand()%b->NumBones],p,Position,true);
					CreateParticle(BITMAP_FIRE,Position,o->Angle,Light);
				}
			}
/*#ifdef USE_EVENT_ELDORADO
			if(c->MonsterIndex == 82 || c->MonsterIndex == 83)
			{
				for(i=0;i<3;i++)
				{
					b->TransformPosition(o->BoneTransform[rand()%b->NumBones],p,Position,true);
					CreateParticle(BITMAP_FIRE,Position,o->Angle,Light);
				}
				Vector(Luminosity*1.f,Luminosity*0.2f,Luminosity*0.f,Light);
				AddTerrainLight(o->Position[0],o->Position[1],Light,2,PrimaryTerrainLight);
			}
#endif*/
        	Vector(0.f,-30.f,0.f,p);
			b->TransformPosition(o->BoneTransform[c->Weapon[0].LinkBone],p,Position,true);
			break;

        case MODEL_MONSTER01+49://  깃털괴물.
            break;
        case MODEL_MONSTER01+50://  별자리.
            break;
        case MODEL_MONSTER01+51://   비의여왕.
            {
                vec3_t pos1, pos2;

				for( int i=2; i<5; ++i )
                {
                    b->TransformPosition(o->BoneTransform[i],p,pos1,true);
    		        b->TransformPosition(o->BoneTransform[i+1],p,pos2,true);
			        CreateJoint(BITMAP_JOINT_THUNDER,pos1,pos2,o->Angle,7,NULL,14.f);
                }

				for( int i=9; i<11; ++i )
                {
                    if( i==9 )
                        b->TransformPosition(o->BoneTransform[2],p,pos1,true);
                    else
                        b->TransformPosition(o->BoneTransform[i],p,pos1,true);
    		        b->TransformPosition(o->BoneTransform[i+1],p,pos2,true);
			        CreateJoint(BITMAP_JOINT_THUNDER,pos1,pos2,o->Angle,7,NULL,14.f);
                }

                b->TransformPosition(o->BoneTransform[2],p,pos1,true);
    		    b->TransformPosition(o->BoneTransform[18],p,pos2,true);
			    CreateJoint(BITMAP_JOINT_THUNDER,pos1,pos2,o->Angle,7,NULL,14.f);

    		    b->TransformPosition(o->BoneTransform[22],p,pos1,true);
			    CreateJoint(BITMAP_JOINT_THUNDER,pos2,pos1,o->Angle,7,NULL,14.f);

    		    b->TransformPosition(o->BoneTransform[23],p,pos2,true);
			    CreateJoint(BITMAP_JOINT_THUNDER,pos1,pos2,o->Angle,7,NULL,14.f);
                                                                           
    		    b->TransformPosition(o->BoneTransform[24],p,pos1,true);
			    CreateJoint(BITMAP_JOINT_THUNDER,pos2,pos1,o->Angle,7,NULL,14.f);

    		    b->TransformPosition(o->BoneTransform[25],p,pos2,true);
			    CreateJoint(BITMAP_JOINT_THUNDER,pos1,pos2,o->Angle,7,NULL,14.f);

                b->TransformPosition(o->BoneTransform[18],p,pos1,true);
    		    b->TransformPosition(o->BoneTransform[31],p,pos2,true);
			    CreateJoint(BITMAP_JOINT_THUNDER,pos1,pos2,o->Angle,7,NULL,14.f);

    		    b->TransformPosition(o->BoneTransform[32],p,pos1,true);
			    CreateJoint(BITMAP_JOINT_THUNDER,pos2,pos1,o->Angle,7,NULL,14.f);

    		    b->TransformPosition(o->BoneTransform[33],p,pos2,true);
			    CreateJoint(BITMAP_JOINT_THUNDER,pos1,pos2,o->Angle,7,NULL,14.f);

    		    b->TransformPosition(o->BoneTransform[34],p,pos1,true);
			    CreateJoint(BITMAP_JOINT_THUNDER,pos2,pos1,o->Angle,7,NULL,14.f);
            }
            break;
        case MODEL_MONSTER01+52:
            o->BlendMeshTexCoordU = -(float)((int)(WorldTime)%10000)*0.0004f;
            break;
        case MODEL_MONSTER01+55:
            o->BlendMeshTexCoordV = (float)((int)(WorldTime)%10000)*0.0001f;
            break;
        case MODEL_MONSTER01+54:
            break;
		case MODEL_MONSTER01+48:
			if ( 0 == ( ( int)rand() % 5))
			{
				Position[0] = o->Position[0] + ( ( rand() % 21) - 10) * ( ( float)TERRAIN_SIZE / 70);
				Position[1] = o->Position[1] + ( ( rand() % 21) - 10) * ( ( float)TERRAIN_SIZE / 70);
				CreatePointer(BITMAP_BLOOD,Position,o->Angle[0],o->Light,0.65f);
			}
			break;

#ifdef LOREN_RAVINE_EVENT
		case MODEL_MONSTER01+88:
#endif

		case MODEL_MONSTER01+45:
			MoveEye(o,b,8,9);
			MonsterMoveSandSmoke(o);
			//MonsterDieSandSmoke(o);
			break;
		case MODEL_MONSTER01+44://
            MoveEye(o,b,8,9);
            if (c->MonsterIndex==63)
            {
                char    body[2] = {30,0};
                char    head = 1;
                vec3_t  vec[35];
                vec3_t  angle;
                vec3_t  dist;
                vec3_t  p;

                Vector(0.f,0.f,0.f,angle);
                Vector(0.f,0.f,0.f,p);

                for ( int i=0; i<35; ++i )
                {
                    b->TransformPosition(o->BoneTransform[vec_list[i]],p,vec[i],true);
                }

                char start, end;
                float scale = 1.0f;

				for (int i=0; i<15; ++i )
                {
                    if ( i>=11 )
                    {
                        scale = 0.5f;
                    }

                    start = wingLeft[i][0];
                    end   = wingLeft[i][1];

                    dist[0] = MoveHumming(vec[end],angle,vec[start],360.0f);
				    CreateParticle(BITMAP_FLAME,vec[start],angle,dist,2,scale);

                    start = wingRight[i][0];
                    end   = wingRight[i][1];

                    dist[0] = MoveHumming(vec[end],angle,vec[start],360.0f);
				    CreateParticle(BITMAP_FLAME,vec[start],angle,dist,2,scale);
                }

                for ( int i=0; i<4; ++i )
                {
                    start = arm_leg_Left[i][0];
                    end   = arm_leg_Left[i][1];

                    dist[0] =MoveHumming(vec[end],angle,vec[start],360.0f);
				    CreateParticle(BITMAP_FLAME,vec[start],angle,dist,2,0.6f);

                    start = arm_leg_Right[i][0];
                    end   = arm_leg_Right[i][1];

                    dist[0] =MoveHumming(vec[end],angle,vec[start],360.0f);
				    CreateParticle(BITMAP_FLAME,vec[start],angle,dist,2,0.6f);
                }

                if ( (int)WorldTime%2==0 )
                {
                    start = body[0];
                    end   = body[1];

                    dist[0] =MoveHumming(vec[end],angle,vec[start],360.0f);
				    CreateParticle(BITMAP_FLAME,vec[start],angle,dist,2,1.3f);

				    CreateParticle(BITMAP_FLAME,vec[head],angle,dist,3,0.5f);
                }

				Vector(-1.3f,-1.3f,-1.3f,Light);
				AddTerrainLight(o->Position[0],o->Position[1],Light,3,PrimaryTerrainLight);
            }
            else
            {
                vec3_t  pos, angle;
                Vector ( 0.f, 0.f, 0.f, angle );

			    Luminosity = (float)sinf(WorldTime*0.002f)*0.3f+0.7f;

			    Vector(Luminosity,Luminosity*0.5f,Luminosity*0.5f,Light);
			    
                b->TransformPosition(o->BoneTransform[55],p,pos,true);
                b->TransformPosition(o->BoneTransform[62],p,Position,true);
                MoveHumming(pos,angle,Position,360.0f);
				CreateParticle(BITMAP_FLAME,Position,angle,Light,1,0.2f);
			    
                b->TransformPosition(o->BoneTransform[70],p,pos,true);
                b->TransformPosition(o->BoneTransform[77],p,Position,true);
                MoveHumming(pos,angle,Position,360.0f);
				CreateParticle(BITMAP_FLAME,Position,angle,Light,1,0.2f);

      			MonsterMoveSandSmoke(o);
				MonsterDieSandSmoke(o);
            }
			break;
		case MODEL_MONSTER01+43:
			MoveEye(o,b,11,12);
			MonsterMoveSandSmoke(o);
			//MonsterDieSandSmoke(o);
			break;
		case MODEL_MONSTER01+42://
			MoveEye(o,b,24,25);
			if(o->SubType == 1)
			{
				b->TransformPosition(o->BoneTransform[6],p,Position,true);
				CreateParticle(BITMAP_FIRE,Position,o->Angle,Light);
				b->TransformPosition(o->BoneTransform[13],p,Position,true);
				CreateParticle(BITMAP_FIRE,Position,o->Angle,Light);
				Vector(-1.3f,-1.3f,-1.3f,Light);
				AddTerrainLight(o->Position[0],o->Position[1],Light,3,PrimaryTerrainLight);
			}
			else
			{
    			MonsterMoveSandSmoke(o);
     			MonsterDieSandSmoke(o);
			}
			break;
		case MODEL_MONSTER01+41:
			MoveEye(o,b,8,9);
			MonsterMoveSandSmoke(o);
			MonsterDieSandSmoke(o);
			break;
		case MODEL_MONSTER01+39:
			MoveEye(o,b,28,27);
			break;
		case MODEL_MONSTER01+37:
			if(o->CurrentAction>=MONSTER01_ATTACK1 && o->CurrentAction<=MONSTER01_ATTACK2)
			{
				o->BlendMeshLight += 0.1f;
				if(o->BlendMeshLight > 1.f)
					o->BlendMeshLight = 1.f;
			}
			else
			{
				o->BlendMeshLight -= 0.1f;
				if(o->BlendMeshLight < 0.f)
					o->BlendMeshLight = 0.f;
			}
			break;
		case MODEL_MONSTER01+29:
			o->BlendMesh = 3;
       		o->BlendMeshTexCoordV = -(float)((int)(WorldTime)%1000)*0.001f;
			if(rand()%2==0)
			{
				Vector(0.f,0.f,0.f,p);
				b->TransformPosition(o->BoneTransform[2],p,Position,true);
				CreateParticle(BITMAP_FIRE,Position,o->Angle,Light);
			}
			break;
		case MODEL_MONSTER01+27:
       		o->BlendMeshTexCoordV = -(float)((int)(WorldTime)%1000)*0.001f;
			break;
		case MODEL_MONSTER01+26:
       		o->BlendMeshTexCoordU = -(float)((int)(WorldTime)%10000)*0.0001f;
			break;
		case MODEL_MONSTER01+32:
			Vector(0.f,0.f,0.f,p);
    		Vector(0.6f,1.f,0.8f,Light);
			if(o->CurrentAction==MONSTER01_ATTACK1)
			{
				b->TransformPosition(o->BoneTransform[33],p,Position,true);
 				CreateParticle(BITMAP_ENERGY,Position,o->Angle,Light);
     			Vector(1.f,0.6f,1.f,Light);
				CreateParticle(BITMAP_FIRE,Position,o->Angle,Light);
			}
			if(o->CurrentAction==MONSTER01_ATTACK2)
			{
				b->TransformPosition(o->BoneTransform[20],p,Position,true);
 				CreateParticle(BITMAP_ENERGY,Position,o->Angle,Light);
     			Vector(1.f,0.6f,1.f,Light);
				CreateParticle(BITMAP_FIRE,Position,o->Angle,Light);
			}
			if(o->CurrentAction==MONSTER01_ATTACK3)
			{
				b->TransformPosition(o->BoneTransform[41],p,Position,true);
 				CreateParticle(BITMAP_ENERGY,Position,o->Angle,Light);
     			Vector(1.f,0.6f,1.f,Light);
				CreateParticle(BITMAP_FIRE,Position,o->Angle,Light);
			}
			if(o->CurrentAction==MONSTER01_ATTACK4)
			{
				b->TransformPosition(o->BoneTransform[49],p,Position,true);
 				CreateParticle(BITMAP_ENERGY,Position,o->Angle,Light);
     			Vector(1.f,0.6f,1.f,Light);
				CreateParticle(BITMAP_FIRE,Position,o->Angle,Light);
			}
			if(o->CurrentAction==MONSTER01_DIE && o->AnimationFrame < 12.f)
			{
				Vector(0.1f,0.8f,0.6f,Light);

				for(int i=0;i<20;i++)
				{
					b->TransformPosition(o->BoneTransform[rand()%b->NumBones],p,Position,true);
					CreateParticle(BITMAP_FIRE,Position,o->Angle,Light);
				}
			}
			break;
		case MODEL_MONSTER01+11:
       		o->BlendMeshLight = (float)(rand()%10)*0.1f;
			if(c->Level == 2)
			{
				for(int i=0;i<10;i++)
				{
					b->TransformPosition(o->BoneTransform[rand()%b->NumBones],p,Position,true);
					CreateParticle(BITMAP_FIRE,Position,o->Angle,Light);
				}
				Vector(Luminosity*1.f,Luminosity*0.2f,Luminosity*0.f,Light);
				AddTerrainLight(o->Position[0],o->Position[1],Light,2,PrimaryTerrainLight);
			}
			break;
		case MODEL_MONSTER01+15:
       		o->BlendMeshTexCoordV = -(float)((int)(WorldTime)%2000)*0.0005f;
			break;
		case MODEL_MIX_NPC:
			if(rand()%64==0)
               PlayBuffer(SOUND_NPC+1);
			break;
		case MODEL_ELF_WIZARD:
			if(rand()%256==0)
               PlayBuffer(SOUND_NPC);
			break;
		case MODEL_SMITH:
			if( g_isCharacterBuff(o, eBuff_CrywolfNPCHide) )
				break;
			if(o->CurrentAction==0 && o->AnimationFrame>=5.f && o->AnimationFrame<=10.f)
               PlayBuffer(SOUND_NPC);
			o->BlendMesh = 4;
			o->BlendMeshLight = Luminosity;
			Vector(Luminosity*1.f,Luminosity*0.4f,Luminosity*0.f,Light);
			AddTerrainLight(o->Position[0],o->Position[1],Light,3,PrimaryTerrainLight);
			Vector(1.f,1.f,1.f,Light);
			Vector(0.f,0.f,0.f,p);
			if(o->CurrentAction==0 && o->AnimationFrame>=5.f && o->AnimationFrame<=6.f)
			{
				b->TransformPosition(o->BoneTransform[17],p,Position,true);
				vec3_t Angle;
				for(int i=0;i<4;i++)
				{
					Vector((float)(rand()%60+60+90),0.f,(float)(rand()%30),Angle);
					CreateJoint(BITMAP_JOINT_SPARK,Position,Position,Angle);
					CreateParticle(BITMAP_SPARK,Position,Angle,Light);
				}
			}
			break;
        case MODEL_DEVIAS_TRADER:
			Vector(1.f,1.f,1.f,Light);
			Vector(0.f,5.f,10.f,p);
			if(o->CurrentAction==0)
			{
				Vector(Luminosity*0.5f,Luminosity*0.3f,Luminosity*0.f,Light);
				AddTerrainLight(o->Position[0],o->Position[1],Light,3,PrimaryTerrainLight);
				b->TransformPosition(o->BoneTransform[37],p,Position,true);
				vec3_t Angle;
				for(int i=0;i<4;i++)
				{
					Vector((float)(rand()%60+60+30),0.f,(float)(rand()%30),Angle);
					CreateJoint(BITMAP_JOINT_SPARK,Position,Position,Angle);
					if(rand()%2)
						CreateParticle(BITMAP_SPARK,Position,Angle,Light);
				}
			}
			break;
		case MODEL_WEDDING_NPC:
			if(o->CurrentAction == 1)
			{
#ifdef YDG_ADD_FIRECRACKER_ITEM
				if(o->AnimationFrame > 4.5f && o->AnimationFrame <= 4.8f)
				{
					CreateEffect(BITMAP_FIRECRACKER0001,o->Position,o->Angle,o->Light,0);
				}
#else	// YDG_ADD_FIRECRACKER_ITEM
				vec3_t Position;
				Vector(0.f,0.f,0.f,p);
				b->TransformPosition(o->BoneTransform[22],p,Position,true);
				Position[2] += 50.f;

				if(o->AnimationFrame > 3.5f && o->AnimationFrame <= 4.0f)
					CreateEffect(BITMAP_FIRECRACKER,Position,o->Angle,o->Light, 1);
				if(o->AnimationFrame > 4.0f && o->AnimationFrame <= 4.5f)
					CreateEffect(BITMAP_FIRECRACKER,Position,o->Angle,o->Light, 1);
				if(o->AnimationFrame > 4.5f && o->AnimationFrame <= 4.8f)
					CreateEffect(BITMAP_FIRECRACKER,Position,o->Angle,o->Light, 1);
#endif	// YDG_ADD_FIRECRACKER_ITEM
			}
			break;
        case MODEL_MONSTER01+2:
			if(o->CurrentAction==MONSTER01_ATTACK1 && o->AnimationFrame<=4.f)
			{
				vec3_t Light;
				Vector(1.f,1.f,1.f,Light);
				Vector(0.f,(float)(rand()%32+32),0.f,p);
				b->TransformPosition(o->BoneTransform[7],p,Position,true);
				CreateParticle(BITMAP_FIRE,Position,o->Angle,Light,1);
			}
		case MODEL_MONSTER01+3:
		case MODEL_MONSTER01+6:
		case MODEL_MONSTER01+20:
			if(o->Type == MODEL_MONSTER01+20)
			{
				Vector(0.f,0.f,0.f,p);
				b->TransformPosition(o->BoneTransform[7],p,Position,true);
				Vector(Luminosity*1.f,Luminosity*0.4f,Luminosity*0.2f,Light);
				CreateSprite(BITMAP_LIGHT,Position,1.f,Light,o);
			}
			if(c->Dead==0 && rand()%4==0)
			{
				Vector(o->Position[0]+(float)(rand()%64-32),
					o->Position[1]+(float)(rand()%64-32),
					o->Position[2]+(float)(rand()%32-16),Position);
				if(World==WD_2DEVIAS)
    				CreateParticle(BITMAP_SMOKE,Position,o->Angle,Light);
				else
    				CreateParticle(BITMAP_SMOKE+1,Position,o->Angle,Light);
			}
			break;
		case MODEL_MONSTER01+33:
			if(c->Dead==0 && c->Level==1 && rand()%4==0)
			{
				Vector(o->Position[0]+(float)(rand()%64-32),
					o->Position[1]+(float)(rand()%64-32),
					o->Position[2]+(float)(rand()%32-16),Position);
   				CreateParticle(BITMAP_SMOKE+1,Position,o->Angle,Light);
			}
			break;
		case MODEL_MONSTER01+5:
			MonsterDieSandSmoke(o);
			break;
		case MODEL_MONSTER01+12:
		case MODEL_MONSTER01+13:
			if(rand()%4==0)
			{
				Vector(0.f,0.f,0.f,p);
				b->TransformPosition(o->BoneTransform[22],p,Position,true);
				CreateParticle(BITMAP_SMOKE,Position,o->Angle,o->Light);
			}
			break;
		case MODEL_MONSTER01:
			if(o->CurrentAction==MONSTER01_STOP1 &&
				(o->AnimationFrame>=15.f&&o->AnimationFrame<=20.f)) Smoke = true;
			if(o->CurrentAction==MONSTER01_STOP2 &&
				(o->AnimationFrame>=20.f&&o->AnimationFrame<=25.f)) Smoke = true;
			if(o->CurrentAction==MONSTER01_WALK && 
				((o->AnimationFrame>=2.f&&o->AnimationFrame<=3.f) || 
				(o->AnimationFrame>=5.f&&o->AnimationFrame<=6.f))) Smoke = true;
			if(Smoke)
			{
				if(rand()%2==0)
				{
					Vector(0.f,-4.f,0.f,p);
    				b->TransformPosition(o->BoneTransform[24],p,Position,true);
					CreateParticle(BITMAP_SMOKE,Position,o->Angle,o->Light);
				}
			}
			break;

#ifdef _PVP_MURDERER_HERO_ITEM
		case MODEL_MASTER:
			if (c->MonsterIndex==227)
			{
				Vector(0.5f,0,0,Light);
				AddTerrainLight(o->Position[0], o->Position[1], Light, 5, PrimaryTerrainLight );
			}
			break;
		case MODEL_HERO_SHOP:
			{
				Vector(0.3,0.3,1,Light);
				AddTerrainLight(o->Position[0], o->Position[1], Light, 15, PrimaryTerrainLight );
			}
			break;
#endif	// _PVP_MURDERER_HERO_ITEM		
        default :
            {
                if ( MoveHellasMonsterVisual( o, b ) ) break;
                if ( battleCastle::MoveBattleCastleMonsterVisual( o, b ) ) break;
				if ( M31HuntingGround::MoveHuntingGroundMonsterVisual(o, b) ) break;
#ifdef CRYINGWOLF_2NDMVP
				if ( M34CryingWolf2nd::MoveCryingWolf2ndMonsterVisual(o, b) ) break;
#endif // CRYINGWOLF_2NDMVP

				if ( M34CryWolf1st::MoveCryWolf1stMonsterVisual(c, o, b) ) break;

				if ( M33Aida::MoveAidaMonsterVisual(o, b) ) break;
				if(M37Kanturu1st::MoveKanturu1stMonsterVisual(c, o, b)) break;
				if(M38Kanturu2nd::Move_Kanturu2nd_MonsterVisual(c, o, b)) break;
				if ( M39Kanturu3rd::MoveKanturu3rdMonsterVisual(o, b) ) break;
				if (SEASON3A::CGM3rdChangeUp::Instance().MoveBalgasBarrackMonsterVisual(c, o, b))	break;
				if( g_NewYearsDayEvent->MoveMonsterVisual (c, o, b) == true) break;
				if( g_CursedTemple->MoveMonsterVisual( o, b ) == true ) break;
				if( SEASON3B::GMNewTown::MoveMonsterVisual( o, b ) == true ) break;
				if( SEASON3C::GMSwampOfQuiet::MoveMonsterVisual( o, b ) == true ) break;

#ifdef KJH_ADD_09SUMMER_EVENT
				if( g_09SummerEvent->MoveMonsterVisual(c, o, b)) break;
#endif // KJH_ADD_09SUMMER_EVENT

#ifdef LDS_ADD_EMPIRE_GUARDIAN
				if( g_EmpireGuardian1.MoveMonsterVisual(c, o, b) == true ) break;
				if( g_EmpireGuardian2.MoveMonsterVisual(c, o, b) == true ) break;
				if( g_EmpireGuardian3.MoveMonsterVisual(c, o, b) == true ) break;
				if( g_EmpireGuardian4.MoveMonsterVisual(c, o, b) == true ) break;
#endif // LDS_ADD_EMPIRE_GUARDIAN

#ifdef PSW_ADD_MAPSYSTEM
				if( TheMapProcess().MoveMonsterVisual( o, b ) == true ) break;
#endif //PSW_ADD_MAPSYSTEM
            }
            break;
		}
		
        if(o->Type==MODEL_PLAYER && c==Hero)
		{
			if ( ( o->CurrentAction>=PLAYER_WALK_MALE && o->CurrentAction<=PLAYER_RUN_RIDE_WEAPON )
			  || ( o->CurrentAction==PLAYER_WALK_TWO_HAND_SWORD_TWO || o->CurrentAction==PLAYER_RUN_TWO_HAND_SWORD_TWO )
              || ( o->CurrentAction==PLAYER_RUN_RIDE_HORSE ) 
#ifdef PBG_ADD_NEWCHAR_MONK_ANI
			  || ( o->CurrentAction==PLAYER_RAGE_UNI_RUN || o->CurrentAction==PLAYER_RAGE_UNI_RUN_ONE_RIGHT )
#endif //PBG_ADD_NEWCHAR_MONK_ANI
			  )
			{
         		Vector(0.f,0.f,0.f,p);
				if(o->AnimationFrame>=1.5f && !c->Foot[0])
				{
					c->Foot[0] = true;
					PlayWalkSound();
				}
				if(o->AnimationFrame>=4.5f && !c->Foot[1])
				{
					c->Foot[1] = true;
					PlayWalkSound();
				}
			}
		}

        if ( ( o->CurrentAction==PLAYER_RUN_RIDE 
#ifdef PBG_ADD_NEWCHAR_MONK_ANI
			|| o->CurrentAction==PLAYER_RAGE_UNI_RUN
			|| o->CurrentAction==PLAYER_RAGE_UNI_RUN_ONE_RIGHT
#endif //PBG_ADD_NEWCHAR_MONK_ANI
			|| o->CurrentAction==PLAYER_RUN_RIDE_WEAPON || o->CurrentAction==PLAYER_RUN_SWIM || o->CurrentAction==PLAYER_WALK_SWIM || o->CurrentAction==PLAYER_FLY || o->CurrentAction==PLAYER_FLY_CROSSBOW || o->CurrentAction==PLAYER_RUN_RIDE_HORSE ) && 
               o->Type==MODEL_PLAYER && InHellas() )
        {
            vec3_t Light = { 0.3f, 0.3f, 0.3f };
            VectorCopy ( o->Position, Position );

            float  Matrix[3][4];
            
            Vector ( 0.f, -40.f, 0.f, p );

            AngleMatrix ( o->Angle, Matrix );
            VectorRotate ( p, Matrix, Position );
            VectorAdd ( o->Position, Position, Position );

            Position[0] += rand()%64-32.f;
            Position[1] += rand()%64-32.f;
            Position[2] += 50.f;

            CreateParticle ( BITMAP_WATERFALL_5, Position, o->Angle, Light, 1 );
        }
	}
}

float CharacterMoveSpeed(CHARACTER *c)
{
	OBJECT *o = &c->Object;
	float Speed = (float)c->MoveSpeed;
	if ( o->Type==MODEL_PLAYER && o->Kind==KIND_PLAYER )
	{
		bool isholyitem = false;

		isholyitem = g_pCursedTempleWindow->CheckInventoryHolyItem(c);
		
		if( isholyitem )
		{
			c->Run = 40;
            Speed  = 8;

#ifndef CSK_FIX_FENRIR_RUN
			if(c->Helper.Type == MODEL_HELPER+37 && !c->SafeZone && !isholyitem )
			{
				if(g_bFenrir_Run == FALSE)
				{
					g_bFenrir_Run = TRUE;
					g_iFenrir_Run = 0;
				}
			}
#endif //! CSK_FIX_FENRIR_RUN

			return Speed;
		}

		if(c->Helper.Type == MODEL_HELPER+37 && !c->SafeZone && !isholyitem )
		{
#ifdef CSK_FIX_FENRIR_RUN
			if(c->Run < FENRIR_RUN_DELAY/2)
				Speed = 15;
			else if(c->Run < FENRIR_RUN_DELAY)
				Speed = 16;
			else
			{
				if(c->Helper.Option1 > 0)
					Speed = 19;
				else
					Speed = 17;
			}
#else // CSK_FIX_FENRIR_RUN
			if(g_bFenrir_Run == FALSE)
			{
				g_bFenrir_Run = TRUE;
				g_iFenrir_Run = 0;
			}
			
			if(g_iFenrir_Run < FENRIR_RUN_DELAY/2)
				Speed = 15;
			else if(g_iFenrir_Run < FENRIR_RUN_DELAY)
				Speed = 16;
			else
			{
				if(c->Helper.Option1 > 0)
					Speed = 19;
				else
					Speed = 17;
			}
#endif // CSK_FIX_FENRIR_RUN
		}
		else if ( c->Helper.Type==MODEL_HELPER+4 && !c->SafeZone && !isholyitem )
        {
            c->Run = 40;
            Speed  = 17;
        }
		else if ( !(c->Object.SubType == MODEL_CURSEDTEMPLE_ALLIED_PLAYER || c->Object.SubType == MODEL_CURSEDTEMPLE_ILLUSION_PLAYER)
			&& ( c->Wing.Type!=-1 || ( c->Helper.Type>=MODEL_HELPER+2 && c->Helper.Type<=MODEL_HELPER+3 ) ) && !c->SafeZone 
			&& !isholyitem )
		{
            if ( c->Wing.Type==MODEL_WING+5
				|| c->Wing.Type==MODEL_WING+36
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
		else if ( !isholyitem )
		{
			if(c->Run < 40)
				Speed = 12;
            else
				Speed = 15;
		}
	}
#ifndef GUILD_WAR_EVENT
    if ( InChaosCastle()==true )
    {
        c->Run = 40;
        Speed = 15;
    }
#endif// GUILD_WAR_EVENT

	if(g_isCharacterBuff((&c->Object), eDeBuff_Freeze))
	{
		Speed *= 0.5f;
	}
#ifdef CSK_ADD_SKILL_BLOWOFDESTRUCTION
	else if(g_isCharacterBuff((&c->Object), eDeBuff_BlowOfDestruction))
	{
		Speed *= 0.33f;
	}
#endif // CSK_ADD_SKILL_BLOWOFDESTRUCTION
	
	if(g_isCharacterBuff((&c->Object), eBuff_CursedTempleQuickness))
	{
		c->Run = 40;
		Speed  = 20;
	}

	return Speed;
}

void MoveCharacterPosition(CHARACTER *c)
{
	OBJECT *o = &c->Object;
	float Matrix[3][4];
	AngleMatrix(o->Angle,Matrix);
	vec3_t v,Velocity;
	Vector(0.f,-CharacterMoveSpeed(c),0.f,v);
	VectorRotate(v,Matrix,Velocity);
	VectorAdd(o->Position,Velocity,o->Position);

    if ( World==-1 || c->Helper.Type!=MODEL_HELPER+3 || c->SafeZone )
    {
        o->Position[2] = RequestTerrainHeight(o->Position[0],o->Position[1]);
    }
    else
    {
        if ( World==WD_8TARKAN || World==WD_10HEAVEN )
            o->Position[2] = RequestTerrainHeight(o->Position[0],o->Position[1])+90.f;
        else
            o->Position[2] = RequestTerrainHeight(o->Position[0],o->Position[1])+30.f;
    }
	if(o->Type==MODEL_MONSTER01+2)
	{ 
		o->Position[2] += -absf(sinf(o->Timer))*70.f+70.f;
	}
	o->Timer += 0.15f;
}

void MoveMonsterClient(CHARACTER *c,OBJECT *o)
{
	if(c==Hero) return;

    if(c->Dead==0)
	{
#ifdef PBG_WOPS_CURSEDTEMPLEBASKET_MOVING
		if(c->MonsterIndex == 384 || c->MonsterIndex == 383)
		{
			c->Movement = false;	
		}
#endif //PBG_WOPS_CURSEDTEMPLEBASKET_MOVING

        //자기위치까지 찾아감
		if(!c->Movement)
		{
			if(c->Appear==0 && o->Type!=MODEL_MONSTER01+7 && (( c->PositionX)!=c->TargetX || ( c->PositionY)!=c->TargetY))
			{
#ifdef YDG_ADD_DOPPELGANGER_MONSTER
				int iDefaultWall = TW_CHARACTER;	//보통일때는 2이상이면 벽으로 체크

				if (World >= WD_65DOPPLEGANGER1 && World <= WD_68DOPPLEGANGER4)
				{
					iDefaultWall = TW_NOMOVE;	// 도플갱어 몬스터는 유저를 뚫고지나간다
				}

				if(PathFinding2(( c->PositionX),( c->PositionY),c->TargetX,c->TargetY,&c->Path, 0.0f, iDefaultWall))
#else	// YDG_ADD_DOPPELGANGER_MONSTER
				if(PathFinding2(( c->PositionX),( c->PositionY),c->TargetX,c->TargetY,&c->Path))
#endif	// YDG_ADD_DOPPELGANGER_MONSTER
				{
					c->Movement = true;
				}
			}
		}
		else
		{
			// 몬스터가 길찾기로 이동 도중에 공격 패킷이 오면 공격 행동 무시되고 계속 걸어가는 애니메이션 나오던 버그 수정(좀 더 수정 필요)
			if(o->Type == MODEL_MONSTER01+121 && (o->CurrentAction == MONSTER01_ATTACK1
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
	
			if(MovePath(c))
			{
				c->Movement = false;
				SetPlayerStop(c);
				c->Object.Angle[2] = ((float)(c->TargetAngle)-1.f)*45.f;
			}
			
			MoveCharacterPosition(c);
		}
	}
	else
	{
		if(o->Type==MODEL_MONSTER01+2)
		{ 
			o->Position[2] = RequestTerrainHeight(o->Position[0],o->Position[1]);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
// 뷰포트안의 전체 케릭터 이동 관리하는 함수
///////////////////////////////////////////////////////////////////////////////

void MoveCharacterClient(CHARACTER *cc)
{
#ifdef DO_PROFILING
	g_pProfiler->BeginUnit( EPROFILING_MOVE_CHARACTERCLIENT, PROFILING_MOVE_CHARACTERCLIENT );
#endif // DO_PROFILING
	OBJECT    *co = &cc->Object;
	if(co->Live)
	{
#ifndef PJH_NEW_SERVER_SELECT_MAP
		if (World == WD_77NEW_LOGIN_SCENE)
		{
			float fDistance_x = CameraPosition[0] - co->Position[0];
			float fDistance_y = CameraPosition[1] - co->Position[1];
			float fDistance = sqrtf(fDistance_x * fDistance_x + fDistance_y * fDistance_y);
			if (!TestFrustrum2D(co->Position[0]*0.01f,co->Position[1]*0.01f,-100.f) || fDistance > 3800.f)	// 컬링 최적화 가능한 부분☆
				return;
		}
#endif //PJH_NEW_SERVER_SELECT_MAP
		co->Visible = TestFrustrum2D(co->Position[0]*0.01f,co->Position[1]*0.01f,-20.f);

        MoveMonsterClient(cc,co);
		MoveCharacter(cc,co);
		MoveCharacterVisual(cc,co);

        battleCastle::MoveBattleCastleMonster ( cc, co );
	}
#ifdef DO_PROFILING
	g_pProfiler->EndUnit( EPROFILING_MOVE_CHARACTERCLIENT );
#endif // DO_PROFILING
}


#ifdef ANTIHACKING_ENABLE
extern BOOL g_bNewFrame;
#endif // ANTIHACKING_ENABLE

void MoveCharactersClient()
{
#ifdef DO_PROFILING
	g_pProfiler->BeginUnit( EPROFILING_MOVE_CHARACTERSCLIENT, PROFILING_MOVE_CHARACTERSCLIENT );
#endif // DO_PROFILING

#ifdef ANTIHACKING_ENABLE
	int iCheck = 0;
#endif //ANTIHACKING_ENABLE
	
	for(int i=0;i<TERRAIN_SIZE*TERRAIN_SIZE;i++)		
	{
		if((TerrainWall[i]&TW_CHARACTER)==TW_CHARACTER) TerrainWall[i] -= TW_CHARACTER;
	}

	for(int i=0;i<MAX_CHARACTERS_CLIENT;i++)		
	{
		CHARACTER *tc = &CharactersClient[i];
		OBJECT    *to = &tc->Object;
		if(to->Live && tc->Dead==0 && to->Kind!=KIND_TRAP)
		{
      		int Index = TERRAIN_INDEX_REPEAT(( tc->PositionX),( tc->PositionY));
			TerrainWall[Index] |= TW_CHARACTER;
		}

		to->Visible = TestFrustrum2D ( to->Position[0]*0.01f, to->Position[1]*0.01f, -20.f );

#ifdef ANTIHACKING_ENABLE
		iCheck += GetTickCount();
#endif //ANTIHACKING_ENABLE
	}
#ifdef USE_SELFCHECKCODE
	SendCrcOfFunction( 18, 4, UseSkillWarrior, 0x7ED1);
#endif
#ifdef ANTIHACKING_ENABLE
	// 체크코드
	if ( g_bNewFrame)
	{
		switch ( iCheck % 10000)
		{
		case 5621:
			hanguo_check3();
			break;
		case 1134:
			hanguo_check4();
			break;
		case 823:
			hanguo_check5();
			break;
		case 2512:
			hanguo_check6();
			break;
		}
	}
#endif //ANTIHACKING_ENABLE

	for(int i=0;i<MAX_CHARACTERS_CLIENT;i++)
	{
		MoveCharacterClient(&CharactersClient[i]);
	}
#ifdef USE_SELFCHECKCODE
	SendCrcOfFunction( 9, 13, MoveInterface, 0x43BA);
#endif
    MoveBlurs();

#ifdef DO_PROFILING
	g_pProfiler->EndUnit( EPROFILING_MOVE_CHARACTERSCLIENT );
#endif // DO_PROFILING	
}

extern float  ParentMatrix[3][4];
#ifdef PBG_ADD_NEWCHAR_MONK
void RenderGuild(OBJECT *o,int Type, vec3_t vPos)
#else //PBG_ADD_NEWCHAR_MONK
void RenderGuild(OBJECT *o,int Type)
#endif //PBG_ADD_NEWCHAR_MONK
{
	EnableAlphaTest();
    EnableCullFace();
	glColor3f(1.f,1.f,1.f);
	BindTexture(BITMAP_GUILD);
	glPushMatrix();

	float Matrix[3][4];
	vec3_t Angle;
	VectorCopy(o->Angle,Angle);
	Angle[2] += 90.f+45.f;
	Angle[1] += 45.f;
	Angle[0] += 80.f;
	AngleMatrix(Angle,Matrix);
	Matrix[0][3] = 20.f;
	Matrix[1][3] = -5.f;
    if ( Type==MODEL_ARMOR+20 && Type!=-1 )
    {
    	Matrix[2][3] = -18.f;//-5
    }
    else
    {
    	Matrix[2][3] = -10.f;//-5
    }
#ifdef PBG_ADD_NEWCHAR_MONK
	if(vPos != NULL)
	{
		Matrix[0][3] = vPos[0];
		Matrix[1][3] = vPos[1];
		Matrix[2][3] = vPos[2];
	}
#endif //PBG_ADD_NEWCHAR_MONK
	R_ConcatTransforms(o->BoneTransform[26],Matrix,ParentMatrix);
	glTranslatef(o->Position[0],o->Position[1],o->Position[2]);
	RenderPlane3D(5.f,7.f,ParentMatrix);
	
	glPopMatrix();
    DisableCullFace();
}

void RenderBrightEffect(BMD *b,int Bitmap,int Link,float Scale,vec3_t Light,OBJECT *o)
{
	vec3_t p,Position;
	Vector(0.f,0.f,0.f,p);
	b->TransformPosition(BoneTransform[Link],p,Position,true);
	CreateSprite(Bitmap,Position,Scale,Light,o);
}

OBJECT g_ItemObject[ITEM_ETC+MAX_ITEM_INDEX];

#ifdef CSK_REF_BACK_RENDERITEM
void RenderLinkObject(float x,float y,float z,CHARACTER *c,PART_t *f,int Type,int Level,int Option1,bool Link,bool Translate,int RenderType, bool bRightHandItem)
#else // CSK_REF_BACK_RENDERITEM
void RenderLinkObject(float x,float y,float z,CHARACTER *c,PART_t *f,int Type,int Level,int Option1,bool Link,bool Translate,int RenderType)
#endif // CSK_REF_BACK_RENDERITEM
{		
	OBJECT *o = &c->Object;
	BMD    *b = &Models[Type];

	if( o->SubType == MODEL_CURSEDTEMPLE_ALLIED_PLAYER || o->SubType == MODEL_CURSEDTEMPLE_ILLUSION_PLAYER )
	{
		if( Type >= MODEL_WING && Type <= MODEL_WING+6 ) return;
#ifdef ADD_ALICE_WINGS_1
		else if (Type >= MODEL_WING+36 && Type <= MODEL_WING+43)
			return;
#else	// ADD_ALICE_WINGS_1
		else if( Type >= MODEL_WING+36 && Type <= MODEL_WING+40 ) return;
#endif	// ADD_ALICE_WINGS_1
#ifdef LDK_ADD_INGAMESHOP_SMALL_WING
		else if( ITEM_WING+130 <= Type && Type <= ITEM_WING+134 ) return;
#endif //LDK_ADD_INGAMESHOP_SMALL_WING
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
		else if (Type >= MODEL_WING+49 && Type <= MODEL_WING+50) return;
		else if (Type == MODEL_WING+135) return;
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
	}

    if ( Type == MODEL_HELPER+30 
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
		|| (Type == MODEL_WING+49)
		|| (Type == MODEL_WING+135)
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
		)
		return;

#ifdef LDK_ADD_INGAMESHOP_SMALL_WING
	if( ITEM_WING+130 == Type )
		return;
#endif //LDK_ADD_INGAMESHOP_SMALL_WING


	if (Type >= MODEL_STAFF+21 && Type <= MODEL_STAFF+29)
	{
		return;
	}

	if( g_isCharacterBuff(o, eBuff_Cloaking) )
		return;

	//CopyShadowAngle(f,b);
 	b->ContrastEnable = o->ContrastEnable;
	b->BodyScale      = o->Scale;
	b->CurrentAction  = f->CurrentAction;
	b->BodyHeight     = 0.f;

	OBJECT* Object = &g_ItemObject[Type];

	Object->Type    = Type;
	ItemObjectAttribute(Object);
	b->LightEnable = Object->LightEnable;
	b->LightEnable = false;

	g_CharacterCopyBuff(Object, o);

#ifdef CSK_EVENT_HALLOWEEN_MAP
	if(Type == MODEL_POTION+45)
	{
		b->BodyScale = 1.8f;
	}
#endif // CSK_EVENT_HALLOWEEN_MAP
	switch (Type)
	{
	case MODEL_WING+39:
		f->PlaySpeed = 0.15f;
		break;
	}

#ifdef YDG_ADD_DOPPELGANGER_MONSTER
	if (c->MonsterIndex >= 529 && c->MonsterIndex <= 539)
	{
		if (World == WD_65DOPPLEGANGER1)
			RenderType = RENDER_DOPPELGANGER|RENDER_TEXTURE;
		else
			RenderType = RENDER_DOPPELGANGER|RENDER_BRIGHT|RENDER_TEXTURE;
	}
#endif	// YDG_ADD_DOPPELGANGER_MONSTER

	OBB_t OBB;
	vec3_t p,Position;

	if(Link)
	{
		vec3_t Angle;
		float Matrix[3][4];

		if ( c->MonsterIndex>=132 && c->MonsterIndex<=134 )
		{
			if ( Type==MODEL_STAFF+10 || Type==MODEL_SWORD+19 )
			{
    			Vector(90.f,0.f,90.f,Angle);
				AngleMatrix(Angle,Matrix);
				Matrix[0][3] = 0.f;
				Matrix[1][3] = 80.f;
				Matrix[2][3] = 120.f;
			}
			else if ( Type==MODEL_BOW+18 )
			{
    			Vector(10.f,0.f,0.f,Angle);
				AngleMatrix(Angle,Matrix);
				Matrix[0][3] = 0.f;
				Matrix[1][3] = 110.f;
				Matrix[2][3] = 80.f;
			}
		}
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
		else if(Type == MODEL_WING+50)
		{
			Vector(0.f,90.f,0.f,Angle);
			AngleMatrix(Angle,Matrix);
			Matrix[0][3] = 10.f;
			Matrix[1][3] = -15.f;
			Matrix[2][3] = 0.f;
		}
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
		else if(Type>=MODEL_WING+40)
		{
    		Vector(0.f,90.f,0.f,Angle);
			AngleMatrix(Angle,Matrix);
			Matrix[0][3] = -47.f;
			Matrix[1][3] = -7.f;
			Matrix[2][3] = 0.f;
		}
		//. 석궁
		else if((Type>=MODEL_BOW+8 && Type<MODEL_BOW+15) || (Type>=MODEL_BOW+16 && Type<MODEL_BOW+17) || 
			(Type>=MODEL_BOW+18 && Type<MODEL_BOW+20))
		{
    		Vector(0.f,20.f,180.f,Angle);
			AngleMatrix(Angle,Matrix);
			Matrix[0][3] = -10.f;
			Matrix[1][3] = 8.f;
			Matrix[2][3] = 40.f;
		}
#ifdef LDK_ADD_14_15_GRADE_ITEM_MODEL
		else if( Type == MODEL_15GRADE_ARMOR_OBJ_ARMLEFT || Type == MODEL_15GRADE_ARMOR_OBJ_ARMRIGHT || 
				Type == MODEL_15GRADE_ARMOR_OBJ_BODYLEFT || Type == MODEL_15GRADE_ARMOR_OBJ_BODYRIGHT ||
				Type == MODEL_15GRADE_ARMOR_OBJ_BOOTLEFT || Type == MODEL_15GRADE_ARMOR_OBJ_BOOTRIGHT ||
				Type == MODEL_15GRADE_ARMOR_OBJ_HEAD || Type == MODEL_15GRADE_ARMOR_OBJ_PANTLEFT ||
				Type == MODEL_15GRADE_ARMOR_OBJ_PANTRIGHT )
		{
			switch(Type)
			{
			case MODEL_15GRADE_ARMOR_OBJ_ARMLEFT:
				{
					Vector(0.f,-90.f,0.f,Angle);
					AngleMatrix(Angle,Matrix);
					Matrix[0][3] = 30.f;
					Matrix[1][3] = 0.f;
					Matrix[2][3] = 20.f;
				}break;
			case MODEL_15GRADE_ARMOR_OBJ_ARMRIGHT:
				{
					Vector(0.f,-90.f,0.f,Angle);
					AngleMatrix(Angle,Matrix);
					Matrix[0][3] = 30.f;
					Matrix[1][3] = 0.f;
					Matrix[2][3] = -20.f;
				}break;
			case MODEL_15GRADE_ARMOR_OBJ_BODYLEFT:
				{
					Vector(0.f,-90.f,0.f,Angle);
					AngleMatrix(Angle,Matrix);
					Matrix[0][3] = 5.f;
					Matrix[1][3] = -20.f;
					Matrix[2][3] = 0.f;
				}break;
			case MODEL_15GRADE_ARMOR_OBJ_BODYRIGHT:
				{
					Vector(0.f,-90.f,0.f,Angle);
					AngleMatrix(Angle,Matrix);
					Matrix[0][3] = 5.f;
					Matrix[1][3] = -20.f;
					Matrix[2][3] = 0.f;
				}break;
			case MODEL_15GRADE_ARMOR_OBJ_BOOTLEFT:
				{
					Vector(0.f,90.f,180.f,Angle);
					AngleMatrix(Angle,Matrix);
					Matrix[0][3] = 20.f;
					Matrix[1][3] = 15.f;
					Matrix[2][3] = -10.f;
				}break;
			case MODEL_15GRADE_ARMOR_OBJ_BOOTRIGHT:
				{
					Vector(0.f,90.f,180.f,Angle);
					AngleMatrix(Angle,Matrix);
					Matrix[0][3] = 20.f;
					Matrix[1][3] = 15.f;
					Matrix[2][3] = 10.f;
				}break;
			case MODEL_15GRADE_ARMOR_OBJ_HEAD:
				{
					Vector(180.f,-90.f,0.f,Angle); //y,x,z
					AngleMatrix(Angle,Matrix);
					Matrix[0][3] = 28.f; //y
					Matrix[1][3] = 20.f; //x
					Matrix[2][3] = 0.f;
				}break;
			case MODEL_15GRADE_ARMOR_OBJ_PANTLEFT:
				{
					Vector(0.f,90.f,180.f,Angle);
					AngleMatrix(Angle,Matrix);
					Matrix[0][3] = 25.f;
					Matrix[1][3] = 5.f;
					Matrix[2][3] = -5.f;
				}break;
			case MODEL_15GRADE_ARMOR_OBJ_PANTRIGHT:
				{
					Vector(0.f,90.f,180.f,Angle);
					AngleMatrix(Angle,Matrix);
					Matrix[0][3] = 25.f;
					Matrix[1][3] = 5.f;
					Matrix[2][3] = 5.f;
				}break;
			}
		}
#endif //LDK_ADD_14_15_GRADE_ITEM_MODEL
		else
		{
			if ( Type==MODEL_STAFF+9 )
			{
				Vector(90.f+20.f,180.f,90.f,Angle);
				AngleMatrix(Angle,Matrix);
			}
			else
			{
				Vector(90.f-20.f,0.f,90.f,Angle);
				AngleMatrix(Angle,Matrix);
			}

			if(Type == MODEL_BOW+20)
			{
				Vector(-60.f,0.f,-80.f,Angle);
				AngleMatrix(Angle,Matrix);
				Matrix[0][3] = -5.f;
				Matrix[1][3] = 20.f;
     			Matrix[2][3] = 0.f;
			}
#ifdef ADD_SOCKET_ITEM
#ifdef ASG_FIX_ARROW_VIPER_BOW_EQUIP_DIRECTION
			else
#endif	// ASG_FIX_ARROW_VIPER_BOW_EQUIP_DIRECTION
			if(Type == MODEL_BOW+23)
			{
				Vector(-60.f,0.f,-80.f,Angle);
				AngleMatrix(Angle,Matrix);
				Matrix[0][3] = -5.f;
				Matrix[1][3] = 20.f;
     			Matrix[2][3] = -5.f;
			}
#endif // ADD_SOCKET_ITEM
#ifdef LDK_ADD_GAMBLERS_WEAPONS
			else if(Type == MODEL_BOW+24)
			{
				Vector(90.f,0.f,-80.f,Angle);
				AngleMatrix(Angle,Matrix);
				Matrix[0][3] = 10.f;
				Matrix[1][3] = 20.f;
     			Matrix[2][3] = -5.f;
			}
#endif //LDK_ADD_GAMBLERS_WEAPONS
			else if(Type>=MODEL_BOW && Type<MODEL_BOW+MAX_ITEM_INDEX)
			{
				Matrix[0][3] = -10.f;
				Matrix[1][3] = 5.f;
     			Matrix[2][3] = 10.f;
			}
			else if ( Type==MODEL_STAFF+9 )
			{
				Matrix[0][3] = -10.f;
				Matrix[1][3] = 5.f;
				Matrix[2][3] = -10.f;
			}
			else if ( Type==MODEL_STAFF+8 )
			{
				Matrix[0][3] = -10.f;
				Matrix[1][3] = 5.f;
				Matrix[2][3] = 10.f;
			}
#ifdef CSK_EVENT_HALLOWEEN_MAP
			else if(Type == MODEL_POTION+45)
			{
				Vector(0.f,0.f,0.f,Angle);
				AngleMatrix(Angle,Matrix);
				Matrix[0][3] = 0.f;
				Matrix[1][3] = 0.f;
				Matrix[2][3] = 0.f;
			}
#endif // CSK_EVENT_HALLOWEEN_MAP
#ifdef CSK_REF_BACK_RENDERITEM
			else if(Type >= MODEL_SHIELD && Type < MODEL_SHIELD+MAX_ITEM_INDEX)
			{
				if(Type == MODEL_SHIELD+16)
				{
					Vector(30.f,0.f,90.f,Angle);
					AngleMatrix(Angle,Matrix);
					Matrix[0][3] = -20.f;
					Matrix[1][3] = 0.f;
					Matrix[2][3] = -20.f;		
				}
				else if(Type == MODEL_SHIELD+14 || Type == MODEL_SHIELD+15)
				{
					Vector(50.f,0.f,90.f,Angle);
					AngleMatrix(Angle,Matrix);
					Matrix[0][3] = -28.f;
					Matrix[1][3] = 0.f;
					Matrix[2][3] = -25.f;	
				}	
				else if(Type == MODEL_SHIELD+6)
				{
					Vector(30.f,0.f,90.f,Angle);
					AngleMatrix(Angle,Matrix);
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
#endif // CSK_REF_BACK_RENDERITEM
			else
			{
				Matrix[0][3] = -20.f;
				Matrix[1][3] = 5.f;
				Matrix[2][3] = 40.f;
			}
		}

#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
		if(g_CMonkSystem.IsRagefighterCommonWeapon(c->Class, Type))
		{
			Matrix[1][3] += 10.0f;
			Matrix[2][3] += 25.0f;
			b->BodyScale = 0.9f;
		}
#endif //PBG_ADD_NEWCHAR_MONK_ITEM

#ifdef CSK_REF_BACK_RENDERITEM
		if (bRightHandItem == false
			&& !(Type >= MODEL_SHIELD && Type < MODEL_SHIELD+MAX_ITEM_INDEX)
#ifdef ASG_FIX_ARROW_VIPER_BOW_EQUIP_DIRECTION
			&& Type != MODEL_BOW+20
#endif	// ASG_FIX_ARROW_VIPER_BOW_EQUIP_DIRECTION
			)
		{
			vec3_t vNewAngle;
			float mNewRot[3][4];
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
			float _Angle = 275.0f;
			if(g_CMonkSystem.IsRagefighterCommonWeapon(c->Class, Type))
			{
				_Angle = 265.0f;
				Matrix[1][3] += 7.0f;
			}
			Vector(145.f, 0.f, _Angle, vNewAngle);
#else //PBG_ADD_NEWCHAR_MONK_ITEM
			Vector(145.f, 0.f, 275.f, vNewAngle);
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
			AngleMatrix(vNewAngle, mNewRot);
			mNewRot[0][3] = 0.f;
			mNewRot[1][3] = 10.f;
			mNewRot[2][3] = -30.f;

			R_ConcatTransforms(Matrix, mNewRot, Matrix);
		}
#endif // CSK_REF_BACK_RENDERITEM
		
		R_ConcatTransforms(o->BoneTransform[f->LinkBone],Matrix,ParentMatrix);
		VectorCopy(c->Object.Position, b->BodyOrigin);

#ifdef CSK_EVENT_HALLOWEEN_MAP
		if(Type == MODEL_POTION+45)
		{
			b->BodyOrigin[0] -= 10.f;
			b->BodyOrigin[1] += 0.f;
			b->BodyOrigin[2] -= 145.f;
		}
#endif // CSK_EVENT_HALLOWEEN_MAP

    	Vector(0.f,0.f,0.f,Object->Angle);
	}
	else
	{
		Vector(x,y,z,p);
		BMD *Owner = &Models[o->Type];
		Owner->RotationPosition(o->BoneTransform[f->LinkBone],p,Position);
		VectorAdd(c->Object.Position,Position,b->BodyOrigin);
    	Vector(0.f,0.f,0.f,Object->Angle);
	}
	if(Type==MODEL_BOSS_HEAD)
	{
		VectorAdd(b->BodyOrigin,BossHeadPosition,b->BodyOrigin);
		b->BoneHead = 0;
		Vector(0.f,0.f,WorldTime,Object->Angle);
	}

    if( ( c->Skill)==AT_SKILL_PIERCING && 
      ((o->Type==MODEL_PLAYER && o->CurrentAction>=PLAYER_ATTACK_FIST && o->CurrentAction<=PLAYER_RIDE_SKILL)))
    {
        if( o->AnimationFrame>=5.f && o->AnimationFrame<=10.f )
        {
            f->PriorAnimationFrame = 2.f;
            f->AnimationFrame      = 3.2f;
        }
    }
    if ( ( f->Type>=MODEL_WING+6 && f->Type<=MODEL_WING+6 ) && c->SafeZone ) //  드라곤 날개.
    {
	    b->CurrentAction  = 1;
    }
#ifdef ADD_SOCKET_ITEM
    if ( !Link || (Type<MODEL_BOW || Type>=MODEL_BOW+MAX_ITEM_INDEX) || Type==MODEL_BOW+23)
#else ADD_SOCKET_ITEM
    if ( !Link || (Type<MODEL_BOW || Type>=MODEL_BOW+MAX_ITEM_INDEX))
#endif // ADD_SOCKET_ITEM
    {
		if( !g_isCharacterBuff(o, eDeBuff_Stun) 
			&& !g_isCharacterBuff(o, eDeBuff_Sleep) 
			)
        {
            b->PlayAnimation(&f->AnimationFrame,&f->PriorAnimationFrame,&f->PriorAction,f->PlaySpeed,Position,Object->Angle);
        }
    }

#ifdef YDG_FIX_ITEM_EFFECT_POSITION_ERROR
#ifdef ADD_SOCKET_ITEM
	VectorCopy( b->BodyOrigin, Object->Position );
#endif // ADD_SOCKET_ITEM
#endif	// YDG_FIX_ITEM_EFFECT_POSITION_ERROR

	vec3_t Temp;
	b->Animation(BoneTransform,f->AnimationFrame,f->PriorAnimationFrame,f->PriorAction,Object->Angle,Object->Angle,true);
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
	if(g_CMonkSystem.IsRagefighterCommonWeapon(c->Class, Type) && !Link)
	{
		float _KnightScale = 0.9f;
		if(Type == MODEL_MACE+2)
			b->InterpolationTrans(BoneTransform[0], BoneTransform[2], _KnightScale);
		b->Transform(BoneTransform,Temp,Temp,&OBB,Translate,_KnightScale);
	}
	else
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
   	b->Transform(BoneTransform,Temp,Temp,&OBB,Translate);
	RenderPartObjectEffect(Object,Type,c->Light,o->Alpha,Level<<3,Option1,false,
#ifdef YDG_ADD_DOPPELGANGER_MONSTER
		0,
#endif	// YDG_ADD_DOPPELGANGER_MONSTER
		RenderType | ((c->MonsterIndex==67 || c->MonsterIndex==137)
		? ( RENDER_EXTRA | RENDER_TEXTURE) : RENDER_TEXTURE));
	
#ifndef YDG_FIX_ITEM_EFFECT_POSITION_ERROR
#ifdef ADD_SOCKET_ITEM

	VectorCopy( b->BodyOrigin, Object->Position );
#endif // ADD_SOCKET_ITEM
#endif	// YDG_FIX_ITEM_EFFECT_POSITION_ERROR

	float Luminosity;
	vec3_t Light;
    Luminosity = (float)(rand()%30+70)*0.005f;
	switch(Type)
	{
	case MODEL_BOW+4:
	case MODEL_BOW+5:
	case MODEL_BOW+6:
    case MODEL_BOW+17:
		if(Type==MODEL_BOW+6)
		{
    		Vector(Luminosity*0.6f,Luminosity*1.f,Luminosity*0.8f,Light);

			for(int i=13;i<=18;i++)
	     		RenderBrightEffect(b,BITMAP_SHINY+1,i,1.f,Light,o);
		}
        else if ( Type==MODEL_BOW+17 )
        {
    		Vector(Luminosity*0.5f,Luminosity*0.5f,Luminosity*0.8f,Light);

			for(int i=13;i<=18;i++)
				RenderBrightEffect(b,BITMAP_SHINY+1,i,1.f,Light,o);

			for(int i=5;i<=8;i++)
				RenderBrightEffect(b,BITMAP_SHINY+1,i,1.f,Light,o);
        }
		else
		{
    		Vector(Luminosity*1.f,Luminosity*0.6f,Luminosity*0.2f,Light);
			RenderBrightEffect(b,BITMAP_SHINY+1,2,1.f,Light,o);
			RenderBrightEffect(b,BITMAP_SHINY+1,6,1.f,Light,o);
		}
		break;
    case MODEL_STAFF+10:
        Vector(Luminosity*1.f,Luminosity*0.3f,Luminosity*0.1f,Light);

        for ( int i=0; i<10; ++i )
        {
            vec3_t Light2;
            Vector ( 0.4f, 0.4f, 0.4f, Light2 );
            Vector ( i*30.f-180.f, -40.f, 0.f, p );
	        b->TransformPosition(BoneTransform[0],p,Position,true);
	        
            if ( (rand()%3)==0 )
            {
                CreateSprite(BITMAP_SHINY+1,Position,0.6f,Light2, o, ( float)( rand()%360));
            }
            CreateSprite(BITMAP_LIGHT,Position,2.f,Light,o);
        }
        break;
	case MODEL_BOW+21:
        {
			Vector(0.8f, 0.8f, 0.2f, Light);
            Vector ( 0.f, 0.f, 0.f, p );
			if(rand()%2 == 1)
            {
				b->TransformPosition(BoneTransform[4],p,Position,true);
				CreateParticle(BITMAP_SPARK+1, Position, o->Angle, Light, 11, 0.8f);

				b->TransformPosition(BoneTransform[12],p,Position,true);
				CreateParticle(BITMAP_SPARK+1, Position, o->Angle, Light, 11, 0.8f);
			}
			Vector(0.5f, 0.5f, 0.1f, Light);
			b->TransformPosition(BoneTransform[7],p,Position,true);
			CreateSprite(BITMAP_LIGHT, Position, 2.0f, Light, o);

			b->TransformPosition(BoneTransform[15],p,Position,true);
			CreateSprite(BITMAP_LIGHT, Position, 2.0f, Light, o);

			Vector ( 20.f, 0.f, 0.f, p );
			Vector(1.0f, 1.0f, 0.4f, Light);
			b->TransformPosition(BoneTransform[0],p,Position,true);
			CreateSprite(BITMAP_LIGHT, Position, 0.8f, Light, o);
			CreateSprite(BITMAP_LIGHT, Position, 1.3f, Light, o);
			CreateSprite(BITMAP_LIGHT, Position, 2.0f, Light, o);
        }
        break;		
	case MODEL_MACE+14:
        {
			Vector(1.0f, 0.3f, 0.0f, Light);
			
			float fRendomPos = (float)(rand()%60)/20.0f - 1.5f;
			float fRendomScale = (float)(rand()%10)/20.0f + 1.4f;
            Vector ( 0.f, -100.f+fRendomPos, fRendomPos, p );
            b->TransformPosition(BoneTransform[0],p,Position,true);
			CreateSprite(BITMAP_SHINY+1,Position,fRendomScale,Light,o);
			CreateSprite(BITMAP_SHINY+1,Position,fRendomScale - 0.3f,Light,o, 20.0f);

			fRendomPos = (float)(rand()%60)/20.0f - 1.5f;
			fRendomScale = (float)(rand()%10)/20.0f + 1.0f;
            Vector ( 0.f, -100.f+fRendomPos, fRendomPos, p );
            b->TransformPosition(BoneTransform[0],p,Position,true);
			CreateSprite(BITMAP_LIGHT,Position,fRendomScale+0.3f,Light,o);

			fRendomPos = (float)(rand()%40)/20.0f - 1.0f;
			fRendomScale = (float)(rand()%8)/20.0f + 0.4f;
            Vector ( 0.f, 100.f+fRendomPos, fRendomPos, p );
            b->TransformPosition(BoneTransform[0],p,Position,true);
			CreateSprite(BITMAP_SHINY+1,Position,fRendomScale,Light,o);
			CreateSprite(BITMAP_SHINY+1,Position,fRendomScale-0.2f,Light,o, 90.0f);
        }
        break;
	case MODEL_SWORD+22:
		{
			float fLight = (float)sinf((WorldTime)*0.4f)*0.25f+0.7f;
			Vector(fLight, fLight-0.5f, fLight-0.5f, Light);
			
            Vector ( 5.f, -22.f, -10.f, p );
            b->TransformPosition(BoneTransform[0],p,Position,true);
			CreateSprite(BITMAP_LIGHT+1,Position,0.75f,Light,o);

            Vector ( -5.f, -22.f, -10.f, p );
            b->TransformPosition(BoneTransform[0],p,Position,true);
			CreateSprite(BITMAP_LIGHT+1,Position,0.75f,Light,o);
		}
		break;
	case MODEL_SWORD+23:
		{
			float fRendomPos = (float)(rand()%60)/20.0f - 1.5f;
			float fRendomScale = (float)(rand()%30)/20.0f + 1.5f;	
			float fLight = (float)sinf((WorldTime)*0.7f)*0.2f+0.5f;
			
			float fRotation = (WorldTime*0.0006f)*360.0f;
			float fRotation2 = (WorldTime*0.0006f)*360.0f;

			Vector(0.2f, 0.2f, fLight, Light);
			
			Vector ( 0.f, fRendomPos, fRendomPos, p );
            b->TransformPosition(BoneTransform[4],p,Position,true);
			CreateSprite(BITMAP_SHINY+1,Position,fRendomScale,Light,o, fRotation);
			CreateSprite(BITMAP_SHINY+1,Position,fRendomScale-0.4f,Light,o, 90.f+fRotation2);
			Vector(0.0f, 0.0f, 0.0f, p);
			CreateSprite(BITMAP_LIGHT,Position,2.3f,Light,o);

			Vector(30.f, 0.f, 0.f, p);
            b->TransformPosition(BoneTransform[4],p,Position,true);
			CreateSprite(BITMAP_LIGHT,Position,2.0f,Light,o);

			Vector(0.f, 0.f, 0.f, p);
            b->TransformPosition(BoneTransform[6],p,Position,true);
			CreateSprite(BITMAP_LIGHT,Position,2.0f,Light,o);

			b->TransformPosition(BoneTransform[7],p,Position,true);
			CreateSprite(BITMAP_LIGHT,Position,2.0f,Light,o);

            b->TransformPosition(BoneTransform[8],p,Position,true);
			CreateSprite(BITMAP_LIGHT,Position,1.0f,Light,o);
		}
		break;		
    case MODEL_STAFF+12:
        {
			Vector(0.4f, 0.4f, 0.4f, Light);
			float fRendomPos = (float)(rand()%60)/20.0f - 1.5f;
			float fRendomScale = (float)(rand()%15)/20.0f + 1.8f;
            Vector ( 0.f, -170.f + fRendomPos, 0.f + fRendomPos, p );
            b->TransformPosition(BoneTransform[0],p,Position,true);
			CreateSprite(BITMAP_SPARK+1,Position,fRendomScale,Light,o);

			VectorCopy(Position,o->EyeLeft);
			CreateJoint(BITMAP_JOINT_ENERGY,Position,Position,o->Angle,17,o,30.f);
 		
			fRendomPos = (float)(rand()%60)/20.0f - 1.5f;
 			fRendomScale = (float)(rand()%15)/20.0f + 1.0f;
			Vector ( 0.f, -170.f + fRendomPos, 0.f + fRendomPos, p );
			Vector(1.0f, 0.4f, 1.0f, Light);
			CreateSprite(BITMAP_LIGHT,Position,fRendomScale,Light,o);
			CreateSprite(BITMAP_SHINY+1,Position,fRendomScale,Light,o);
			CreateSprite(BITMAP_SHINY+1,Position,fRendomScale - 0.3f,Light,o, 90.0f);
			CreateParticle(BITMAP_SPARK+1, Position, o->Angle, Light, 11, 2.0f);

			float fLight = (float)sinf((WorldTime)*0.7f)*0.2f+0.5f;
			float fRotation = (WorldTime*0.0006f)*360.0f;

			Vector(fLight -0.1f, 0.1f, fLight-0.1f, Light);
			Vector ( 0.f, 10.0f, 0.0f, p );
			b->TransformPosition(BoneTransform[2],p,Position,true);
			CreateSprite(BITMAP_SHINY+1,Position,1.5f,Light,o, fRotation);
			CreateSprite(BITMAP_SHINY+1,Position,1.2f,Light,o, 90.0f + fRotation);

			Vector ( -40.f, -10.0f, 0.0f, p );
			b->TransformPosition(BoneTransform[1],p,Position,true);
			CreateSprite(BITMAP_SHINY+1,Position,1.0f,Light,o, fRotation);
			CreateSprite(BITMAP_SHINY+1,Position,0.7f,Light,o, 90.0f + fRotation);

			Vector ( -160.f, -5.0f, 0.0f, p );
			b->TransformPosition(BoneTransform[1],p,Position,true);
			CreateSprite(BITMAP_SHINY+1,Position,1.2f,Light,o, fRotation);
			CreateSprite(BITMAP_SHINY+1,Position,1.0f,Light,o, 90.0f + fRotation);
		}
        break;	
	case MODEL_SWORD+24:
		{
			// Light
			Vector(0.6f, 0.6f, 0.6f, Light);

			Vector(0.f, 0.f, 3.f, p);
			b->TransformPosition(BoneTransform[2], p, Position, true);
			CreateSprite(BITMAP_LIGHT+1, Position, 0.45f, Light, o);

			Vector(0.f, 0.f, -3.f, p);
			b->TransformPosition(BoneTransform[2], p, Position, true);
			CreateSprite(BITMAP_LIGHT+1, Position, 0.45f, Light, o);

			Vector(0.f, 0.f, 0.f, p);
			b->TransformPosition(BoneTransform[2], p, Position, true);
			CreateSprite(BITMAP_SHINY+1, Position, 0.7f, Light, o);

			float fScale = 0.5f + (float)(rand()%100)/180;
			float fRotation = (WorldTime*0.0006f)*360.0f;
			Vector(0.f, 0.f, 0.f, p);
			b->TransformPosition(BoneTransform[2], p, Position, true);
			CreateSprite(BITMAP_SHINY+1, Position, fScale, Light, o, fRotation);

			// Flare01.jpg
			float fLight = (float)sinf((WorldTime)*0.4f)*0.25f+0.6f;
			Vector(fLight, fLight, fLight, Light);
			Vector(0.f, 0.f, 0.f, p);
		    b->TransformPosition(BoneTransform[2], p, Position, true);
		    CreateSprite(BITMAP_LIGHT, Position, 3.f, Light, o, 0.f);
		}
		break;
	case MODEL_SWORD+25:
		{
			float fLight, fScale, fRotation;
			static float fPosition = 0.0f;
			static int iRandom;
			
			Vector(1.0f, 0.1f, 0.0f, Light);
			Vector(0.f, 0.f, 0.f, p);
			b->TransformPosition(BoneTransform[1],p,Position,true);
			CreateSprite(BITMAP_LIGHT,Position,1.0f,Light,o);	// flare01.jpg

			fScale = (float)(rand()%30)/60.0f + 0.2f;
			fLight = (float)sinf((WorldTime)*0.7f)*0.2f+0.3f;
			fRotation = (WorldTime*0.0006f)*360.0f;
			Vector(0.1f+fLight, 0.2f, 0.0f, Light);
			CreateSprite(BITMAP_LIGHT+3,Position,fScale,Light,o, fRotation);	// impact01.jpg

			Vector(1.0f-fLight, 0.0f, 0.0f, Light);
			if(fPosition >= 20.0f)
			{
				iRandom = rand()%5+2;	// 2 ~ 6
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
			fLight = (float)sinf((WorldTime)*0.4f)*0.25f+0.2f;
			Vector(0.8f+fLight, 0.1f, 0.f, Light);
			Vector(0.f, 0.f, 0.f, p);
			for(int i=0; i<5; ++i)
			{
				b->TransformPosition(BoneTransform[2+i], p, Position, true);
				CreateSprite(BITMAP_LIGHT, Position, 1.1f, Light, o);
			}

			if ( o->CurrentAction==PLAYER_RUN_TWO_HAND_SWORD_TWO && World!=WD_10HEAVEN && rand()%2==0 )
			{
				if(!g_Direction.m_CKanturu.IsMayaScene())
				{
					Vector(1.f,1.f,1.f,Light);
					Vector(0.f,0.f,0.f,p);
					b->TransformPosition ( BoneTransform[6],p,Position,true );
					Position[0] += rand()%30-15.f;
					Position[1] += rand()%30-15.f;
					Position[2] += 20.f;

					vec3_t	Angle;
					for(int i=0;i<4;i++)
					{
						Vector((float)(rand()%60+60+90),0.f,o->Angle[2],Angle );//(float)(rand()%30),Angle);
						CreateJoint(BITMAP_JOINT_SPARK,Position,Position,Angle);
						CreateParticle(BITMAP_SPARK,Position,Angle,Light);
					}
				}
			}
		}
		break;
	case MODEL_MACE+15:
		{
			float fScale;
			Luminosity = (float)sinf((WorldTime)*0.002f)*0.3f+0.1f;
			fScale = (float)(rand()%10)/30.0f + 1.7f;

			Vector ( 0.f, 0.f, 0.f, p );

			Vector ( 0.8f+Luminosity, 0.5f+Luminosity, 0.1f+Luminosity, Light );
			b->TransformPosition(BoneTransform[1],p,Position,true);
			CreateSprite(BITMAP_SHINY+1,Position,fScale,Light,o);

			Vector ( 0.7f, 0.5f, 0.3f, Light );
			b->TransformPosition(BoneTransform[1],p,Position,true);
            CreateSprite(BITMAP_LIGHT,Position,3.f,Light,o);

			Vector ( 0.8f, 0.6f, 0.4f, Light );
			b->TransformPosition(BoneTransform[2],p,Position,true);
            CreateSprite(BITMAP_LIGHT,Position,1.5f,Light,o);

			Vector ( 0.8f, 0.6f, 0.4f, Light );
			b->TransformPosition(BoneTransform[3],p,Position,true);
            CreateSprite(BITMAP_LIGHT,Position,1.2f,Light,o);
		}
		break;
	case MODEL_BOW+22:
		{
			float fLight;
			fLight = (float)sinf((WorldTime)*0.4f)*0.25f;
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
			if(rand()%2 == 1)
            {
				b->TransformPosition(BoneTransform[10],p,Position,true);
				CreateParticle(BITMAP_SPARK+1, Position, o->Angle, Light, 14, 0.05f);
				b->TransformPosition(BoneTransform[28],p,Position,true);
				CreateParticle(BITMAP_SPARK+1, Position, o->Angle, Light, 14, 0.05f);
				b->TransformPosition(BoneTransform[34],p,Position,true);
				CreateParticle(BITMAP_SPARK+1, Position, o->Angle, Light, 14, 0.05f);
				b->TransformPosition(BoneTransform[16],p,Position,true);
				CreateParticle(BITMAP_SPARK+1, Position, o->Angle, Light, 14, 0.05f);
			}
		}
		break;
	case MODEL_STAFF+13:
		{
			float fLight, fScale, fRotation;
			fLight = (float)sinf((WorldTime)*0.7f)*0.2f+0.3f;
			Vector(0.f, 0.f, 0.f, p);

			// flare01
			Vector(0.7f, 0.7f, 0.7f, Light);
			fScale = (float)(rand()%10)/500.0f;
			b->TransformPosition(BoneTransform[9], p, Position, true);
			CreateSprite(BITMAP_LIGHT, Position, 0.3f+fScale, Light, o);
//			Vector(0.1f, 0.5f, 0.1f, Light);
//			CreateSprite(BITMAP_LIGHT, Position, 1.5f, Light, o);

			Vector(0.2f, 0.6f+fLight, 0.2f, Light);
			fScale = (float)(rand()%10)/500.0f;
			b->TransformPosition(BoneTransform[9], p, Position, true);
			CreateSprite(BITMAP_LIGHT, Position, 4.0f+fScale, Light, o);
			
			// shiny02
			Vector(0.4f, 0.5f+fLight, 0.4f, Light);
			fScale = (float)(rand()%30)/60.0f;
			fRotation = (WorldTime*0.0004f)*360.0f;
			b->TransformPosition(BoneTransform[9], p, Position, true);
			CreateSprite(BITMAP_SHINY+1, Position, 1.2f+fScale, Light, o, -fRotation);

			// magic_ground1
			Vector(0.6f, 1.f, 0.6f, Light);
			fScale = (float)(rand()%10)/500.0f;
			fRotation = (WorldTime*0.0006f)*360.0f;
			b->TransformPosition(BoneTransform[9], p, Position, true);
			CreateSprite(BITMAP_MAGIC, Position, 0.25f+fScale, Light, o, fRotation);

			// 5, 6, 7, 8 flare01 , 10, 11, 12, 13
			Vector(0.f, 0.f, 0.f, p);

			for(int i=0; i<4; ++i)
			{
				Vector(0.1f, 0.8f, 0.1f, Light);
				b->TransformPosition(BoneTransform[5+i], p, Position, true);
				CreateSprite(BITMAP_LIGHT, Position, 0.2f, Light, o);
				Vector(0.5f, 0.5f, 0.5f, Light);
				b->TransformPosition(BoneTransform[10+i], p, Position, true);
				CreateSprite(BITMAP_LIGHT, Position, 0.3f, Light, o);
			}

			Vector(0.1f, 0.8f, 0.1f, Light);
			b->TransformPosition(BoneTransform[4], p, Position, true);
			CreateSprite(BITMAP_SHINY+1, Position, 0.6f+fScale, Light, o, fRotation);

			Vector(0.6f, 1.f, 0.6f, Light);
			fScale = (float)(rand()%10)/500.0f;
			fRotation = (WorldTime*0.0006f)*360.0f;
			b->TransformPosition(BoneTransform[2], p, Position, true);
			CreateSprite(BITMAP_MAGIC, Position, 0.15f+fScale, Light, o, fRotation);
			Vector(0.8f, 0.8f, 0.8f, Light);
			b->TransformPosition(BoneTransform[2], p, Position, true);
			CreateSprite(BITMAP_LIGHT, Position, 0.7f+fScale, Light, o, -fRotation);
			Vector(0.1f, 1.0f, 0.1f, Light);
			b->TransformPosition(BoneTransform[2], p, Position, true);
			CreateSprite(BITMAP_SHINY+1, Position, 0.5f+fScale, Light, o, fRotation);

			Vector(0.2f, 0.6f+fLight, 0.2f, Light);
			b->TransformPosition(BoneTransform[2], p, Position, true);
			CreateSprite(BITMAP_LIGHT, Position, 2.0f+fScale, Light, o);
		}
		break;
    case MODEL_STAFF+11:
        {
            Luminosity = (float)sinf((WorldTime)*0.002f)*0.3f+0.7f;
            
            Vector ( 0.f, 0.f, 0.f, p );
            Vector ( Luminosity*0.f, Luminosity*0.5f, Luminosity*1.f, Light );

            float Rotation = (float)( rand()%360 );
            b->TransformPosition(BoneTransform[5],p,Position,true);
            Vector ( Luminosity*0.f, Luminosity*0.5f, Luminosity*1.f, Light );
            CreateSprite(BITMAP_LIGHT,Position,1.f,Light,o);
            Vector ( 0.5f, 0.5f, 0.5f, Light );
            CreateSprite(BITMAP_SHINY+1,Position,0.5f,Light,o, Rotation );
            CreateSprite(BITMAP_SHINY+1,Position,0.5f,Light,o, 360.f-Rotation );
            
            b->TransformPosition(BoneTransform[6],p,Position,true);
            Vector ( Luminosity*0.f, Luminosity*0.5f, Luminosity*1.f, Light );
            CreateSprite(BITMAP_LIGHT,Position,1.f,Light,o);
            Vector ( 0.5f, 0.5f, 0.5f, Light );
            CreateSprite(BITMAP_SHINY+1,Position,0.5f,Light,o, Rotation );
            CreateSprite(BITMAP_SHINY+1,Position,0.5f,Light,o, 360.f-Rotation );
            
            b->TransformPosition(BoneTransform[8],p,Position,true);
            Vector ( Luminosity*0.f, Luminosity*0.5f, Luminosity*1.f, Light );
            CreateSprite(BITMAP_LIGHT,Position,2.f,Light,o);
            Vector ( 0.5f, 0.5f, 0.5f, Light );
            CreateSprite(BITMAP_SHINY+1,Position,1.f,Light,o, Rotation );
            CreateSprite(BITMAP_SHINY+1,Position,1.f,Light,o, 360.f-Rotation );
        }
        break;
    case MODEL_MACE+12:
        {
            float Rotation = (float)( rand()%360 );
            Luminosity = (float)sinf((WorldTime)*0.002f)*0.3f+0.7f;
            
            Vector ( 0.f, 0.f, 0.f, p );

            b->TransformPosition(BoneTransform[1],p,Position,true);
            Vector ( Luminosity*0.f, Luminosity*0.5f, Luminosity*1.f, Light );
            CreateSprite(BITMAP_LIGHT,Position,2.f,Light,o);
            Vector ( 0.5f, 0.5f, 0.5f, Light );
            CreateSprite(BITMAP_SHINY+1,Position,1.f,Light,o, Rotation );
            CreateSprite(BITMAP_SHINY+1,Position,1.f,Light,o, 360.f-Rotation );
        }
        break;
    case MODEL_SWORD+19:
        Vector(Luminosity*1.f,Luminosity*0.3f,Luminosity*0.1f,Light);

        Vector ( 0.f, 0.f, 0.f, p );

        for ( int i=0; i<7; ++i )
        {
            vec3_t Light2;
            Vector ( 0.4f, 0.4f, 0.4f, Light2 );
	        b->TransformPosition(BoneTransform[i+2],p,Position,true);
	        
            if ( (rand()%3)==0 )
            {
                CreateSprite(BITMAP_SHINY+1,Position,0.6f,Light2, o, ( float)( rand()%360));
            }
            CreateSprite ( BITMAP_LIGHT, Position, 2.f, Light, o );
        }
		
        if(((o->CurrentAction<PLAYER_WALK_MALE || o->CurrentAction>PLAYER_RUN_RIDE_WEAPON) && 
            (o->CurrentAction<PLAYER_ATTACK_SKILL_SWORD1 || o->CurrentAction>PLAYER_ATTACK_SKILL_SWORD5 ) 
			) )
		{
			vec3_t pos, delta, angle;

			Vector ( 0.f, 0.f, 0.f, p );
			Vector ( -90.f, (float)(rand()%360), o->Angle[2]-45, angle );
			b->TransformPosition(BoneTransform[3],p,pos,true);
			b->TransformPosition(BoneTransform[2],p,Position,true);

			VectorSubtract ( pos, Position, delta );
		}
        break;
    case MODEL_BOW+18:
        Vector(Luminosity*1.f,Luminosity*0.3f,Luminosity*0.1f,Light);
/*
        for ( i=0; i<4; ++i )
        {
            vec3_t Light2;
            Vector ( 0.4f, 0.4f, 0.4f, Light2 );
            Vector(0.f,i*30.f-10.f,0.f,p);
	        b->TransformPosition(BoneTransform[0],p,Position,true);
	        
            if ( (rand()%3)==0 )
            {
                CreateSprite(BITMAP_SHINY+1,Position,0.6f,Light2, o, ( float)( rand()%360));
            }
            CreateSprite(BITMAP_LIGHT,Position,1.f,Light,o);
        }
*/
        break;
	case MODEL_BOW+19:
		Vector ( 0.f, 0.f, 10.f, p );

        for ( int i=1; i<6; ++i )
        {
    		Vector(Luminosity*0.5f,Luminosity*0.5f,Luminosity*0.8f,Light);
	        b->TransformPosition(BoneTransform[i],p,Position,true);

            CreateSprite ( BITMAP_SHINY+1,Position, 1.f, Light, o );

			if ( i==5 )
			{
				Vector(1.f,1.f,1.f,Light);
				CreateSprite ( BITMAP_LIGHT, Position, 2.f, Light, o );
			}
			else
				CreateSprite ( BITMAP_LIGHT, Position, 2.f, Light, o );
        }
		break;
	case MODEL_SHIELD+15:
		Vector(Luminosity*0.6f,Luminosity*0.6f,Luminosity*2.f,Light);

        Vector ( 15.f, -15.f, 0.f, p );
	    b->TransformPosition(BoneTransform[1],p,Position,true);
		CreateSprite(BITMAP_SHINY+1,Position,1.5f,Light,o);
		CreateSprite(BITMAP_LIGHT,Position,Luminosity+1.5f,Light,o);
		break;
	case MODEL_MACE+7:
        Vector(Luminosity*1.f,Luminosity*0.9f,Luminosity*0.f,Light);

        Vector ( 0.f, 0.f, 0.f, p );
	    b->TransformPosition(BoneTransform[1],p,Position,true);
        CreateSprite ( BITMAP_LIGHT, Position, 2.f, Light, o );

		Vector ( 0.5f, 0.5f, 0.5f, Light );
        CreateSprite ( BITMAP_LIGHT, Position, sinf(WorldTime*0.002f)+0.5f, Light, o );
		break;
	case MODEL_MACE+8:
        {
            float Scale = sinf(WorldTime*0.001f)+1.f;
            Vector(Luminosity*0.2f,Luminosity*0.1f,Luminosity*3.f,Light);
            Vector ( -15.f, 0.f, 0.f, p );
            b->TransformPosition ( BoneTransform[1], p, Position, true );
            CreateSprite ( BITMAP_LIGHT, Position, Scale, Light, o );
            Vector ( 10.f, 0.f, 0.f, p );
            b->TransformPosition ( BoneTransform[1], p, Position, true );
            CreateSprite ( BITMAP_LIGHT, Position, Scale, Light, o );
            
            Scale = sinf ( WorldTime*0.01f )*360;
            Luminosity = sinf ( WorldTime*0.001f )*0.5f+0.5f;
            Vector ( Luminosity, Luminosity, Luminosity, Light );
    		CreateSprite ( BITMAP_SHINY+1, Position, 0.6f, Light, o, 360-Scale );
    		CreateSprite ( BITMAP_SHINY+1, Position, 0.6f, Light, o, Scale );
        }
		break;
	case MODEL_MACE+9:
		{
			for ( int i=1; i<5; i++) 
            {
				Luminosity = (float)sinf((WorldTime)*0.002f)*0.35f+0.65f;
				Vector ( Luminosity*0.6f, Luminosity*0.8f, Luminosity*1.f, Light );
				Vector ( -10.f, 0.f, 0.f, p );
				b->TransformPosition ( BoneTransform[i+1], p, Position, true );
				CreateSprite ( BITMAP_LIGHT, Position, /*Luminosity**/1.f, Light, o );

                if ( i==3 )
                {
    				Vector ( 0.5f, 0.5f, 0.5f, Light );
    				CreateSprite ( BITMAP_SHINY+1, Position, 0.6f, Light, o, rand()%360 );
                }
			}
		}
		break;
	case MODEL_MACE+10:
        {
            Luminosity = sinf ( WorldTime*0.001f )*0.5f+0.7f;
            Vector ( Luminosity*1.f, Luminosity*0.8f, Luminosity*0.6f, Light );
            Vector ( 0.f, 0.f, 0.f, p );
            
            b->TransformPosition ( BoneTransform[2], p, Position, true );
            CreateSprite ( BITMAP_LIGHT, Position, 1.5f, Light, o );
            CreateSprite ( BITMAP_SHINY+1, Position, Luminosity*0.5f, Light, o );
            
            Vector ( Luminosity*-10.f, 0.f, 0.f, p );
            Vector ( 0.6f, 0.8f, 1.f, Light );
            Luminosity = rand()%360;
            b->TransformPosition ( BoneTransform[1], p, Position, true );
            CreateSprite ( BITMAP_LIGHT, Position, 0.7f, Light, o );
            CreateSprite ( BITMAP_SHINY+1, Position, 0.5f, Light, o, Luminosity );
            CreateSprite ( BITMAP_SHINY+1, Position, 0.4f, Light, o, 360-Luminosity );
        }
		break;
	case MODEL_MACE+11:
		{
			Vector ( 1.f, 0.6f, 0.3f, Light );
			Vector ( 0.f, 0.f, 0.f, p );
			b->TransformPosition ( BoneTransform[1], p, Position, true );
			CreateSprite ( BITMAP_LIGHT, Position, 1.3f, Light, o );

            for ( int i=0; i<3; ++i )
            {
			    Vector ( i*15.f-10.f, 0.f, 0.f, p );
			    b->TransformPosition ( BoneTransform[2], p, Position, true );
			    CreateSprite ( BITMAP_LIGHT, Position, 1.3f, Light, o );
                CreateSprite ( BITMAP_SHINY+1, Position, 0.4f, Light, o, rand()%360 );
            }
		}
		break;
	case MODEL_MACE+13:
		{
			Vector(Luminosity*1.f,Luminosity*0.3f,Luminosity*0.1f,Light);
			Vector ( 0.f, 0.f, 0.f, p );
			vec3_t Light2;
			Vector ( 0.4f, 0.4f, 0.4f, Light2 );
			b->TransformPosition(BoneTransform[0],p,Position,true);
			if ( (rand()%3)==0 )
			{
				CreateSprite(BITMAP_SHINY+1,Position,0.6f,Light2, o, ( float)( rand()%360));
			}
			CreateSprite ( BITMAP_LIGHT, Position, 2.f, Light, o );
		}
		break;
	case MODEL_SWORD+20:
		{
			for(int i=0; i<2; i++) {
				Vector ( 0.f, 0.f, 0.f, p );
				b->TransformPosition(BoneTransform[i+1],p,Position,true);
				Luminosity = (float)sinf((WorldTime)*0.002f)*0.35f+0.65f;
				Vector(Luminosity*0.43f,Luminosity*0.14f,Luminosity*0.6f,Light);
				CreateSprite ( BITMAP_LIGHT, Position, Luminosity*0.9f, Light, o );
				Vector ( 0.3f, 0.3f, 0.3f, Light );
				CreateSprite ( BITMAP_LIGHT, Position, float(sinf(WorldTime*0.002f)*0.5f)+0.4f, Light, o );
			}
		}
		break;
	case MODEL_BOW+20:
		{
			float Scale = sinf(WorldTime*0.001f)+1.f;
			Vector(Luminosity*3.f,Luminosity,Luminosity,Light);
			Vector ( 0.f, 0.f, 0.f, p );
			b->TransformPosition ( BoneTransform[6], p, Position, true );
			CreateSprite ( BITMAP_LIGHT, Position, Scale*0.8f, Light, o );
			b->TransformPosition ( BoneTransform[2], p, Position, true );
			CreateSprite ( BITMAP_LIGHT, Position, Scale*0.8f, Light, o );

			float Rotation = sinf ( WorldTime*0.01f )*360;
            Luminosity = sinf( WorldTime*0.001f )*0.3+0.3f;
            Vector ( Luminosity, Luminosity, Luminosity, Light );
			if(!c->SafeZone) 
            {
    			Vector ( 10.f, 0.f, 0.f, p );
				b->TransformPosition ( BoneTransform[9], p, Position, true );
				CreateSprite ( BITMAP_LIGHT, Position, Scale*0.8f, Light, o );
				Vector ( Luminosity*3.0f, Luminosity, Luminosity, Light );
				CreateSprite ( BITMAP_SHINY+1, Position, 0.8f, Light, o, 360-Rotation );
				CreateSprite ( BITMAP_SHINY+1, Position, 0.8f, Light, o, Rotation );
			}
		}
		break;
	case MODEL_SWORD+21:
	case MODEL_SWORD+31:
		if ( o->CurrentAction==PLAYER_RUN_TWO_HAND_SWORD_TWO && World!=WD_10HEAVEN && rand()%2==0 )
		{
			 if( !g_Direction.m_CKanturu.IsMayaScene())
			{
				Vector(1.f,1.f,1.f,Light);
				Vector(0.f,0.f,0.f,p);
				b->TransformPosition ( BoneTransform[1],p,Position,true );
				Position[0] += rand()%30-15.f;
				Position[1] += rand()%30-15.f;
				Position[2] += 20.f;

                vec3_t	Angle;
				for(int i=0;i<4;i++)
				{
					Vector((float)(rand()%60+60+90),0.f,o->Angle[2],Angle );//(float)(rand()%30),Angle);
					CreateJoint(BITMAP_JOINT_SPARK,Position,Position,Angle);
					CreateParticle(BITMAP_SPARK,Position,Angle,Light);
				}
			}
		}
		break;
	case MODEL_SPEAR+10:
        Vector(Luminosity*0.2f,Luminosity*0.1f,Luminosity*0.8f,Light);
		Vector ( 0.f, 0.f, 0.f, p );

		for ( int i=1; i<9; i++ )
		{
			b->TransformPosition(BoneTransform[i],p,Position,true);
			CreateSprite ( BITMAP_LIGHT, Position, 1.3f, Light, o );
		}
		break;
#ifdef ADD_SOCKET_ITEM
	case MODEL_SWORD+26:
		{
			Vector( 0.8f, 0.6f, 0.2f, Light );
			b->TransformByObjectBone(Position, Object, 11 );		// Gold01
			CreateSprite(BITMAP_LIGHT, Position, 0.6f, Light, o);
			CreateSprite(BITMAP_LIGHT, Position, 0.6f, Light, o);
	
			Vector( 0.3f, 0.8f, 0.7f, Light );
			b->TransformByObjectBone(Position, Object, 12 );		// b01
			CreateSprite(BITMAP_LIGHT, Position, 0.5f, Light, o);
			b->TransformByObjectBone(Position, Object, 13 );		// n02
			CreateSprite(BITMAP_LIGHT, Position, 0.5f, Light, o);

			b->TransformByObjectBone(Position, Object, 12 );		// n04
			CreateSprite(BITMAP_LIGHT, Position, 0.3f, Light, o);
			b->TransformByObjectBone(Position, Object, 13 );		// b03
			CreateSprite(BITMAP_LIGHT, Position, 0.3f, Light, o);

			Vector(0.9f, 0.1f, 0.1f,Light);	
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
	case MODEL_SWORD+27:
		{
			Vector(0.1f, 0.9f, 0.1f,Light);	
			
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
			
			for( int i=1 ; i<=7 ; i++ )
			{
				if( rand()%4 != 0 )
				{
					continue;
				}
				Vector( 0.f, 0.f, 0.f, Position );
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
				int iNumBone = (rand()%7)+1;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
				b->TransformByObjectBone(Position, Object, i);
				CreateParticle(BITMAP_WATERFALL_4, Position, Object->Angle, Light, 12, 0.5f, Object);
			}	
		}break;
	case MODEL_SWORD+28:
		{
			float fRendomScale = (float)((rand()%15)/30.0f)+0.5f;
			Vector(0.f,0.f,0.f,Position);
			Vector(0.1f, 0.4f, 0.9f,Light);	
			b->TransformPosition(BoneTransform[8], Position, p, true);		// Zx01
			CreateSprite(BITMAP_FLARE_BLUE, p, 0.4f, o->Light, o);
			CreateSprite(BITMAP_SHINY+6, p, fRendomScale, Light, o);
			
			// 잔상 Zx01
			vec3_t vColor;
			VectorCopy(p, o->EyeLeft);
			Vector(0.f, 0.f, 0.9f, vColor);
			CreateJoint(BITMAP_JOINT_ENERGY, p, p, o->Angle, 17, o, 25.f);
			//CreateEffect(MODEL_EFFECT_TRACE, p, o->Angle, vColor, 0, NULL, -1, 0, 0, 0, 25.f);
			
			b->TransformPosition(BoneTransform[9], Position, p, true);		// Zx02
			CreateSprite(BITMAP_FLARE_BLUE, p, 0.4f, o->Light, o);
			CreateSprite(BITMAP_SHINY+6, p, fRendomScale, Light, o);
			b->TransformPosition(BoneTransform[10], Position, p, true);		// Zx03
			CreateSprite(BITMAP_FLARE_BLUE, p, 0.4f, o->Light, o);
			CreateSprite(BITMAP_SHINY+6, p, 0.4f, Light, o);
			b->TransformPosition(BoneTransform[11], Position, p, true);		// Zx04
			CreateSprite(BITMAP_FLARE_BLUE, p, 0.4f, o->Light, o);
			CreateSprite(BITMAP_SHINY+6, p, 0.4f, Light, o);
			
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
	case MODEL_MACE+16:
		{	
			vec3_t vDPos;
			Vector( 0.5f, 0.8f, 0.5f, Light );
			// Zx04
			b->TransformByObjectBone( Position, Object, 11 );
			CreateSprite(BITMAP_SHINY+6, Position, 0.2f, Light, o);
			// Zx05
			b->TransformByObjectBone( Position, Object, 10 );
			CreateSprite(BITMAP_SHINY+6, Position, 0.2f, Light, o);

			Vector( 0.9f, 0.1f, 0.3f, Light );
			// Zx02
			b->TransformByObjectBone( Position, Object, 9 );
			CreateSprite(BITMAP_SHINY+6, Position, 0.2f, Light, o);
			// Zx03
			b->TransformByObjectBone( Position, Object, 8 );
			CreateSprite(BITMAP_SHINY+6, Position, 0.2f, Light, o);
			// Zx01
			b->TransformByObjectBone( Position, Object, 1 );
			CreateSprite(BITMAP_SHINY+6, Position, 0.2f, Light, o);
			CreateSprite(BITMAP_LIGHT, Position, 0.8f, Light, o);
			
			Vector( 0.5f, 0.8f, 0.6f, Light );
			// Zx001
			b->TransformByObjectBone( Position, Object, 24 );
			CreateSprite(BITMAP_SHINY+6, Position, 0.3f, Light, o);
			CreateSprite(BITMAP_LIGHT, Position, 2.0f, Light, o);
			CreateSprite(BITMAP_PIN_LIGHT, Position, 0.5f, Light, o, ((int)(WorldTime*0.04f)%360));
			CreateSprite(BITMAP_PIN_LIGHT, Position, 0.7f, Light, o, -((int)(WorldTime*0.03f)%360));
			CreateSprite(BITMAP_PIN_LIGHT, Position, 0.9f, Light, o, ((int)(WorldTime*0.02f)%360));
			
			if( rand()%3 != 0 )
			{
				float fTemp = Position[2];
				Position[2] -= 15.f;
				Vector( 0.5f, 0.8f, 0.8f, Light );
				CreateParticle(BITMAP_CLUD64, Position, o->Angle, Light, 9, 0.4f);
				Position[2] = fTemp;
			}


			Vector( 0.5f, 0.8f, 0.6f, Light );
			vDPos[0] = Position[0] + ((float)(rand()%20-10)*3.f);
			vDPos[1] = Position[1] + ((float)(rand()%20-10)*3.f);
			vDPos[2] = Position[2] + ((float)(rand()%20-10)*3.f);

			if(rand()%10 == 0 )
			{
				CreateEffect(MODEL_STAR_SHINE, vDPos, o->Angle, Light, 0, Object, -1, 0, 0, 0, 0.22f );
			}
	
			// Zx06
			b->TransformByObjectBone( Position, Object, 12 );
			CreateSprite(BITMAP_LIGHT, Position, 0.2f, Light, o);
			// Zx07
			b->TransformByObjectBone( Position, Object, 13 );
			CreateSprite(BITMAP_LIGHT, Position, 0.2f, Light, o);
			// Zx08
			b->TransformByObjectBone( Position, Object, 14 );
			CreateSprite(BITMAP_LIGHT, Position, 0.2f, Light, o);
			// Zx09
			b->TransformByObjectBone( Position, Object, 15 );
			CreateSprite(BITMAP_LIGHT, Position, 0.2f, Light, o);
			// Zx10
			b->TransformByObjectBone( Position, Object, 16 );
			CreateSprite(BITMAP_LIGHT, Position, 0.3f, Light, o);
			// Zx11
			b->TransformByObjectBone( Position, Object, 17 );
			CreateSprite(BITMAP_LIGHT, Position, 0.3f, Light, o);
			// Zx12
			b->TransformByObjectBone( Position, Object, 18 );
			CreateSprite(BITMAP_LIGHT, Position, 0.3f, Light, o);
			// Zx13
			b->TransformByObjectBone( Position, Object, 19 );
			CreateSprite(BITMAP_LIGHT, Position, 0.3f, Light, o);
			// Zx14
			b->TransformByObjectBone( Position, Object, 20 );
			CreateSprite(BITMAP_LIGHT, Position, 0.3f, Light, o);
			// Zx15
			b->TransformByObjectBone( Position, Object, 21 );
			CreateSprite(BITMAP_LIGHT, Position, 0.4f, Light, o);
			// Zx16
			b->TransformByObjectBone( Position, Object, 22 );
			CreateSprite(BITMAP_LIGHT, Position, 0.4f, Light, o);
			// Zx17
			b->TransformByObjectBone( Position, Object, 23 );
			CreateSprite(BITMAP_LIGHT, Position, 0.4f, Light, o);
		}break;
	case MODEL_MACE+17:
		{
			float fRandomScale;
			vec3_t vPosZx01, vPosZx02, vLight1, vLight2, vDLight;

			float fLumi = absf((sinf( WorldTime*0.0008f )))*0.8+0.2f;	
			Vector( fLumi*0.6f, fLumi*0.5f, fLumi*0.8f, vDLight);

			Vector(0.6f, 0.5f, 0.8f, vLight1);
			Vector(0.8f, 0.8f, 0.8f, vLight2);
			b->TransformByObjectBone( vPosZx01, Object, 3 );		// Zx01
			b->TransformByObjectBone( vPosZx02, Object, 4 );		// Zx02

			if( ((int)WorldTime/100)%10 == 0 )
			{
				Object->m_iAnimation = rand()%100;			
				Object->EyeRight[0] = (rand()%10-5);
				Object->EyeRight[1] = (rand()%10-5);
				Object->EyeRight[2] = (rand()%10-5);
				Object->EyeRight2[0] = (rand()%10-5)*1.2f;
				Object->EyeRight2[1] = (rand()%10-5)*1.2f;
				Object->EyeRight2[2] = (rand()%10-5)*1.2f;
			}
			// Object->m_iAnimation Random Texture
			int iRandomTexure1, iRandomTexure2;
			iRandomTexure1 = (Object->m_iAnimation/10)%3;	// 3개
			iRandomTexure2 = (Object->m_iAnimation)%3;		// 3개

			// Zx01
			fRandomScale = (float)(rand()%10)/10.0f + 1.0f;		//(1.0~2.0)
			CreateSprite(BITMAP_LIGHT, vPosZx01, fRandomScale, vLight1, o);
			CreateSprite(BITMAP_SHINY+1, vPosZx01, 0.5f, vDLight, o);
			VectorAdd( vPosZx01, Object->EyeRight, vPosZx01 );
			CreateSprite(BITMAP_LIGHTNING_MEGA1+iRandomTexure1, vPosZx01, (((rand()%11)-20)/100.f)+0.5f, vLight2, o, rand()%380 );

			// Zx02
			fRandomScale = (float)((rand()%10)/5.0f)+1.5f;		//(2.0~3.25)
			CreateSprite(BITMAP_LIGHT, vPosZx02, fRandomScale, vLight1, o);
			CreateSprite(BITMAP_SHINY+1, vPosZx02, 1.0f, vDLight, o);
			VectorAdd( vPosZx02, Object->EyeRight2, vPosZx02 );
			CreateSprite(BITMAP_LIGHTNING_MEGA1+iRandomTexure1, vPosZx02, (((rand()%11)-20)/50.f)+0.8f, vLight2, o, rand()%380 );
		}break;
	case MODEL_BOW+23:
		{
			vec3_t vZX03, vZx04;
			int iNumCreateFeather = rand()%3;
			
			Vector( 0.2f, 0.25f, 0.3f, Light);	

			for( int i=0 ; i<=43 ; i++ )
			{
				if( i==1 )
				{
					continue;
				}
				b->TransformByObjectBone( Position, Object, i );	// 
				CreateSprite( BITMAP_LIGHT, Position, 0.8f, Light, Object );
			}

			if( o->AnimationFrame >= 4.5f && o->AnimationFrame <= 5.0f )
			{
				for( int i=0 ; i<iNumCreateFeather ; i++ )
				{
					CreateEffect( MODEL_FEATHER, vZX03, o->Angle, Light, 0, NULL, -1, 0, 0, 0, 0.6f );
					CreateEffect( MODEL_FEATHER, vZX03, o->Angle, Light, 1, NULL, -1, 0, 0, 0, 0.6f );
					CreateEffect( MODEL_FEATHER, vZx04, o->Angle, Light, 0, NULL, -1, 0, 0, 0, 0.6f );
					CreateEffect( MODEL_FEATHER, vZx04, o->Angle, Light, 1, NULL, -1, 0, 0, 0, 0.6f );
				}
			}
			

		}break;
	case MODEL_STAFF+30:
		{
			Vector( 0.f, 0.f, 0.f, Position);
			Vector( 0.8f, 0.3f, 0.1f, Light);	
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
			
			float fLumi = absf((sinf( WorldTime*0.001f )))*0.8f + 0.2f;
			vec3_t vDLight;
			Vector( fLumi*0.8f, fLumi*0.1f, fLumi*0.3f, vDLight);
			float fRendomScale = (float)(rand()%10)/20.0f + 0.8f;		// (0.5~1.25)
			Vector(0.8f, 0.2f, 0.4f, Light);
			b->TransformPosition(BoneTransform[2], Position, p, true);		// Red01
			CreateSprite(BITMAP_LIGHT, p, 2.0f, vDLight, o);
			CreateSprite(BITMAP_SHINY+6, p, fRendomScale, Light, o);
			b->TransformPosition(BoneTransform[3], Position, p, true);		// Red02
			CreateSprite(BITMAP_LIGHT, p, 2.0f, vDLight, o);
			CreateSprite(BITMAP_SHINY+6, p, fRendomScale, Light, o);
			
			// 잔상
			vec3_t vColor;
			VectorCopy(p, o->EyeRight);
			Vector(0.9f, 0.f, 0.f, vColor);
			CreateJoint(BITMAP_JOINT_ENERGY, p, p, o->Angle, 47, o, 25.f);
			// CreateJoint()
			// BITMAP_JOINT_ENERGY SubType
			//CreateEffect(MODEL_EFFECT_TRACE, p, o->Angle, vColor, 0, NULL, -1, 0, 0, 0, 20.f);
		}break;
	case MODEL_STAFF+31:
		{
			Vector(0.f,0.f,0.f,Position);
			Vector(0.3f,0.3f,0.9f,Light);	
			b->TransformPosition(BoneTransform[2], Position, p, true);		// Zx01
			CreateSprite(BITMAP_LIGHT, p, 2.f, Light, o);
			CreateSprite(BITMAP_LIGHT, p, 2.f, Light, o);
			b->TransformPosition(BoneTransform[3], Position, p, true);		// Zx02
			CreateSprite(BITMAP_LIGHT, p, 2.5f, Light, o);
			CreateSprite(BITMAP_LIGHT, p, 2.5f, Light, o);
			b->TransformPosition(BoneTransform[4], Position, p, true);		// Zx03
			CreateSprite(BITMAP_LIGHT, p, 3.f, Light, o);
			Vector(0.7f,0.1f,0.2f,Light);	
			b->TransformPosition(BoneTransform[5], Position, p, true);		// Zx04
			CreateSprite(BITMAP_LIGHT, p, 2.f, Light, o);
			Vector(0.9f,0.3f,0.5f,Light);
			CreateSprite(BITMAP_SHINY+6, p, 0.8f, Light, o);
			
			// 깜빡깜박 이팩트
			float fRendomScale = (float)(rand()%15)/20.0f + 1.0f;
			CreateSprite(BITMAP_SHINY+1, p, fRendomScale, Light, o);
			CreateSprite(BITMAP_SHINY+1, p, fRendomScale-0.3f, Light, o, 90.0f);
			CreateParticle(BITMAP_SPARK+1, p, o->Angle, Light, 11, 2.0f);
			
			// 잔상
			vec3_t vColor;
			VectorCopy(p, o->EyeLeft);
			Vector(0.f, 0.f, 0.9f, vColor);
			CreateJoint(BITMAP_JOINT_ENERGY, p, p, o->Angle, 17, o, 30.f);	
			//CreateEffect(MODEL_EFFECT_TRACE, p, o->Angle, vColor, 0, NULL, -1, 0, 0, 0, 30.f);
			
			Vector(0.7f,0.7f,0.7f,Light);
			CreateSprite(BITMAP_SHINY+2, p, 2.f, Light, o);
		}break;
	case MODEL_STAFF+32:
		{
			float fRandomScale;
			vec3_t vLight1, vLight2;
			Vector( 0.9f, 0.7f, 0.4f, vLight1 );
			Vector( 0.9f, 0.1f, 0.3f, vLight2 );

			b->TransformByObjectBone(Position, Object, 1);		// Zx01
			Vector( 1.0f, 0.1f, 0.2f, Light );
			CreateSprite(BITMAP_SHINY+6, Position, 0.7f, Light, o);
			CreateSprite(BITMAP_LIGHTMARKS, Position, 0.6f, Light, o);	
			CreateSprite(BITMAP_LIGHT, Position, 2.f, Light, o);

			Vector( 0.7f, 0.1f, 0.6f, Light );
			CreateSprite(BITMAP_SHOCK_WAVE, Position, 0.65f, Light, o, -((int)(WorldTime*0.05f)%360));
			// Object->Timer 
			// Object->EyeRight 
			Object->Timer += 0.01f;
			if( Object->Timer <= 0.1f || Object->Timer > 0.9f )
			{
				Object->Timer = 0.15f;
				Vector( 0.7f, 0.1f, 0.6f, Object->EyeRight );
			}
			if( Object->Timer > 0.5f )
			{
				Object->EyeRight[0] *= 0.95f;
				Object->EyeRight[1] *= 0.95f;
				Object->EyeRight[2] *= 0.95f;
			}

			CreateSprite( BITMAP_SHOCK_WAVE, Position, Object->Timer, Object->EyeRight, o );

			Vector( 0.9f, 0.5f, 0.2f, Light );
			fRandomScale = (float)(rand()%5)/25.0f + 0.3f;		// (0.3~0.4)
			CreateSprite(BITMAP_LIGHTMARKS, Position, fRandomScale, Light, o);

			b->TransformByObjectBone(Position, Object, 2);		// Zx02
			CreateSprite(BITMAP_SHINY+6, Position, 0.3f, vLight1, o);
			CreateSprite(BITMAP_LIGHTMARKS, Position, 0.4f, vLight2, o);

			b->TransformByObjectBone(Position, Object, 3);		// Zx03
			CreateSprite(BITMAP_SHINY+6, Position, 0.2f, vLight1, o);
			CreateSprite(BITMAP_LIGHTMARKS, Position, 0.3f, vLight2, o);
		}break;
	case MODEL_SHIELD+17:
		{
			vec3_t vDLight;
			Vector( 0.8f, 0.6f, 0.2f, Light );
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
	
			float fLumi = absf((sinf( WorldTime*0.0005f )));	
			Vector( fLumi*1.f, fLumi*1.f, fLumi*1.f, vDLight);
			b->TransformByObjectBone(Position, Object, 1);		// Zx01
			CreateSprite(BITMAP_FLARE_RED, Position, 0.5f, vDLight, o);
			b->TransformByObjectBone(Position, Object, 2);		// Zx02
			CreateSprite(BITMAP_FLARE_RED, Position, 0.3f, vDLight, o);
			b->TransformByObjectBone(Position, Object, 3);		// Zx002
			CreateSprite(BITMAP_FLARE_RED, Position, 0.3f, vDLight, o);


		}break;
	case MODEL_SHIELD+18:
		{
			Vector( 0.9f, 0.f, 0.2f, Light );
			b->TransformByObjectBone(Position, Object, 1);		// Zx01
			CreateSprite(BITMAP_LIGHT, Position, 2.0f, Light, o);
			
			Vector( 1.f, 1.f, 1.f, Light );
			switch(rand()%3)
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
		}break;
	case MODEL_SHIELD+19:
		{
// 			float fScale = 1.0f;
// 
// 			float fLumi = absf((sinf( WorldTime*0.001f )))*0.8f + 0.2f;	// 페이드인/아웃
// 			vec3_t vDLight;
// 			Vector( fLumi*1.0f, fLumi*1.0f, fLumi*1.0f, vDLight);
// 
// 			b->TransformByObjectBone(Position, Object, 1);		// Zx01
// 			CreateSprite(BITMAP_FLARE_BLUE, Position, 0.4f, vDLight, o);
// 
// 			b->TransformByObjectBone(Position, Object, 4);		// Zx02
// 			CreateSprite(BITMAP_FLARE_BLUE, Position, 0.4f, vDLight, o);
// 
// 			b->TransformByObjectBone(Position, Object, 3);		// Zxt03
// 			CreateSprite(BITMAP_FLARE_BLUE, Position, 0.4f, vDLight, o);
// 
// 			b->TransformByObjectBone(Position, Object, 2);		// Zxt04
// 			CreateSprite(BITMAP_FLARE_BLUE, Position, 0.4f, vDLight, o);
		}break;
	case MODEL_SHIELD+20:		// 가디언실드
		{
			Vector(0.f, 0.f, 0.f, Position);
			float fLumi = fabs(sinf( WorldTime*0.001f ))+0.1f;
			Vector(0.8f*fLumi, 0.3f*fLumi, 0.8f*fLumi, Light);
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
#endif // ADD_SOCKET_ITEM
#ifdef CSK_ADD_ITEM_CROSSSHIELD
	case MODEL_SHIELD+21:
		{
			vec3_t vPos, vLight;
			Vector(0.f, 0.f, 0.f, vPos);
			float fLumi = fabs(sinf( WorldTime*0.001f ))+0.1f;
			Vector(0.2f*fLumi, 0.2f*fLumi, 0.8f*fLumi, vLight);
			b->TransformByObjectBone(vPos, Object, 4);
			CreateSprite(BITMAP_LIGHT, vPos, 1.2f, vLight, Object);
			b->TransformByObjectBone(vPos, Object, 5);
			CreateSprite(BITMAP_LIGHT, vPos, 1.2f, vLight, Object);
			b->TransformByObjectBone(vPos, Object, 6);
			CreateSprite(BITMAP_LIGHT, vPos, 1.2f, vLight, Object);
			b->TransformByObjectBone(vPos, Object, 7);
			CreateSprite(BITMAP_LIGHT, vPos, 1.2f, vLight, Object);
			
			Vector(0.2f*fLumi, 0.6f*fLumi, 0.6f*fLumi, vLight);
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
#endif // CSK_ADD_ITEM_CROSSSHIELD
#ifdef LDK_ADD_GAMBLERS_WEAPONS
	case MODEL_STAFF+33:
		{
			vec3_t vPos, vLight;
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
			float fSize = 0.0f;	
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING

			//큰 구슬
			b->TransformByObjectBone(vPos, Object, 8);
			Vector(0.9f, 0.1f, 0.4f, vLight);
			CreateSprite(BITMAP_LIGHT, vPos, 3.5f, vLight, Object);
			vLight[0] = 0.1f + 0.8f * absf(sinf( WorldTime*0.0008f));
			vLight[1] = 0.1f * absf(sinf( WorldTime*0.0008f));
			vLight[2] = 0.1f + 0.4f * absf(sinf( WorldTime*0.0008f));
			CreateSprite(BITMAP_MAGIC, vPos, 0.3f, vLight, Object);
			CreateEffect( MODEL_MOONHARVEST_MOON, vPos, o->Angle, vLight, 2, NULL, -1, 0, 0, 0, 0.12f );
			Vector(0.8f, 0.8f, 0.2f, vLight);
			CreateSprite(BITMAP_SHINY+1, vPos, 1.0f, vLight, Object);

			//작은 구슬
			for(int i=1; i<8; i++)
			{
				b->TransformByObjectBone(vPos, Object, i);
				Vector(0.8f, 0.1f, 0.4f, vLight);
				CreateSprite(BITMAP_LIGHT, vPos, 0.6f, vLight, Object);
				CreateSprite(BITMAP_MAGIC, vPos, 0.12f, vLight, Object);
				Vector(0.8f, 0.8f, 0.2f, vLight);
				CreateSprite(BITMAP_SHINY+1, vPos, 0.3f, vLight, Object);
			}
		}
		break;
	case MODEL_STAFF+34:
		{
			vec3_t vPos, vLight;

			//zx01
			b->TransformByObjectBone(vPos, Object, 1);
			Vector(0.4f, 0.2f, 1.0f, vLight);
			CreateSprite(BITMAP_SHINY+6, vPos, 1.4f, vLight, Object);
			Vector(0.7f, 0.1f, 0.9f, vLight);
			CreateSprite(BITMAP_LIGHT, vPos, 1.5f, vLight, Object);
			Vector(0.0f, 0.0f, 1.0f, vLight);
			CreateSprite(BITMAP_SHINY+2, vPos, 1.5f, vLight, Object);
			Vector(0.4f, 0.4f, 1.0f, vLight);
			CreateSprite(BITMAP_PIN_LIGHT, vPos, 0.7f, vLight, Object, -((int)(WorldTime*0.04f)%360));
			CreateSprite(BITMAP_PIN_LIGHT, vPos, 0.8f, vLight, Object, -((int)(WorldTime*0.03f)%360));

			float quarterAngle, theta, fSize;
			quarterAngle = Q_PI/180.0f*(int(WorldTime*0.02f)%90);
			theta = absf( sinf( quarterAngle + Q_PI/2 ) );
			fSize = absf( sinf(quarterAngle) ) * 0.5f;
			Vector( 0.7f * theta, 0.1f * theta, 0.9f * theta, vLight);
  			CreateSprite(BITMAP_MAGIC, vPos, fSize, vLight, Object);
			quarterAngle = Q_PI/180.0f*( int(WorldTime*0.05f)%60 + 30);
	 		theta = absf( sinf( quarterAngle + Q_PI/2 ) );
 			fSize = absf( sinf(quarterAngle) ) * 0.5f;
			Vector( 0.1f + 0.7f * theta, 0.1f * theta, 0.1f + 0.3f * theta, vLight);
 			CreateSprite(BITMAP_MAGIC, vPos, fSize, vLight, Object);

			//zx02
			Vector(0.9f, 0.0f, 0.1f, vLight);
			b->TransformByObjectBone(vPos, Object, 2);
			CreateSprite(BITMAP_SHINY+6, vPos, 0.8f, vLight, Object);

			//zx03
			Vector(0.9f, 0.4f, 0.7f, vLight);
			b->TransformByObjectBone(vPos, Object, 3);
			CreateSprite(BITMAP_SHINY+6, vPos, 0.6f, vLight, Object);
		}
		break;
	case MODEL_SPEAR+11:
		{
			vec3_t vPos, vLight;

			//zx01, zx02
			Vector(0.9f, 0.2f, 0.7f, vLight);
			b->TransformByObjectBone(vPos, Object, 3);
			CreateSprite(BITMAP_SHINY+6, vPos, 0.8f, vLight, Object);
			CreateSprite(BITMAP_SHINY+6, vPos, 0.3f, vLight, Object);
			b->TransformByObjectBone(vPos, Object, 4);
			CreateSprite(BITMAP_SHINY+6, vPos, 0.8f, vLight, Object);
			CreateSprite(BITMAP_SHINY+6, vPos, 0.3f, vLight, Object);

			//bone02
			Vector(0.2f, 0.9f, 0.1f, vLight);
			b->TransformByObjectBone(vPos, Object, 5);
			CreateSprite(BITMAP_LIGHT, vPos, 1.5f, vLight, Object);
		}
		break;
	case MODEL_MACE+18:
		{
			vec3_t vPos, vLight;
			float fSize = 0.0f;

			//rx01
			b->TransformByObjectBone(vPos, Object, 5);
			Vector(0.5f, 0.8f, 0.9f, vLight);
			CreateSprite(BITMAP_SHINY+1, vPos, 0.5f, vLight, Object);
			Vector(0.4f, 0.7f, 0.9f, vLight);
			CreateSprite(BITMAP_LIGHTMARKS, vPos, 1.0f, vLight, Object);

			Vector(0.4f, 0.6f, 0.9f, vLight);
			if(int(WorldTime)%14 == 0)
				CreateParticle(BITMAP_SHINY+6, vPos, o->Angle, vLight, 1 );

			//rx02
			Vector(0.5f, 0.6f, 0.9f, vLight);
			b->TransformByObjectBone(vPos, Object, 6);
			CreateSprite(BITMAP_SHINY+1, vPos, 1.1f, vLight, Object);

			//zx 01, 02
			Vector(0.8f, 0.5f, 0.2f, vLight);
			b->TransformByObjectBone(vPos, Object, 2);
			CreateSprite(BITMAP_SHINY+1, vPos, 0.5f, vLight, Object);
			b->TransformByObjectBone(vPos, Object, 3);
			CreateSprite(BITMAP_SHINY+1, vPos, 0.5f, vLight, Object);

			//zx03
			Vector(1.0f, 1.0f, 1.0f, vLight);
			b->TransformByObjectBone(vPos, Object, 4);
			fSize = 0.5f * absf((sinf( WorldTime*0.0015f )));
			CreateSprite(BITMAP_FLARE, vPos, fSize, vLight, Object);
		}
		break;
	case MODEL_BOW+24:
		{
			vec3_t vPos, vLight;
			float quarterAngle, theta, fSize;

			//zx02, 03
			Vector(0.2f, 0.8f, 0.5f, vLight);
			int iBoneNum[2] = {7,11};

			for(int i=0; i<2; i++)
			{
				b->TransformByObjectBone(vPos, Object, iBoneNum[i]);

				CreateSprite(BITMAP_LIGHT, vPos, 0.8f, vLight, Object);
				CreateSprite(BITMAP_LIGHT, vPos, 0.8f, vLight, Object);

				quarterAngle = Q_PI/180.0f*(int(WorldTime*0.02f)%90);
				theta = absf( sinf( quarterAngle + Q_PI/2 ) );
				fSize = absf( sinf(quarterAngle) ) * 0.3f;
				Vector( 0.2f * theta, 0.8f * theta, 0.5f * theta, vLight);
				CreateSprite(BITMAP_SHOCK_WAVE, vPos, fSize, vLight, Object);

				quarterAngle = Q_PI/180.0f*( int(WorldTime*0.05f)%60 + 30);
				theta = absf( sinf( quarterAngle + Q_PI/2 ) );
				fSize = absf( sinf(quarterAngle) ) * 0.3f;
				Vector( 0.2f * theta, 0.8f * theta, 0.5f * theta, vLight);
				CreateSprite(BITMAP_SHOCK_WAVE, vPos, fSize, vLight, Object);
			}

			//model_bow action, frame
			if( o->CurrentAction>=PLAYER_ATTACK_FIST && o->CurrentAction<=PLAYER_RIDE_SKILL )
			{
				Vector(0.2f, 0.8f, 0.5f, vLight);
				for(int i=0; i<8; i++)
				{
					b->TransformByObjectBone(vPos, Object, 22+i);
					CreateSprite(BITMAP_FLARE, vPos, 0.4f, vLight, Object);
				}
			}
		}
		break;
#endif //LDK_ADD_GAMBLERS_WEAPONS
	}

}

void RenderLight(OBJECT *o,int Texture,float Scale,int Bone,float x,float y,float z)
{
	BMD *b = &Models[o->Type];
	vec3_t p,Position;
	Vector(x,y,z,p);
	b->TransformPosition(o->BoneTransform[Bone],p,Position,true);
    float Luminosity = sinf(WorldTime*0.002f)*0.3f+0.7f;
	vec3_t Light;
    Vector(Luminosity*1.f,Luminosity*0.6f,Luminosity*0.4f,Light);
	CreateSprite(Texture,Position,Scale,Light,o);
}

void RenderEye(OBJECT *o,int Left,int Right,float fSize = 1.0f)
{
	BMD *b = &Models[o->Type];
	vec3_t p,Position;
    float Luminosity = sinf(WorldTime*0.002f)*0.3f+0.8f;
	vec3_t Light;
	Vector(Luminosity,Luminosity,Luminosity,Light);
	Vector(5.f,0.f,0.f,p);
	b->TransformPosition(o->BoneTransform[Left],p,Position,true);
	//CreateParticle(BITMAP_SHINY+3,Position,o->Angle,Light);
	CreateSprite(BITMAP_SHINY+3,Position,fSize,Light,NULL);
	Vector(-5.f,0.f,0.f,p);
	b->TransformPosition(o->BoneTransform[Right],p,Position,true);
	//CreateParticle(BITMAP_SHINY+3,Position,o->Angle,Light);
	CreateSprite(BITMAP_SHINY+3,Position,fSize,Light,NULL);
	VectorCopy(Position,o->EyeLeft);
}


void RenderCharacter(CHARACTER *c,OBJECT *o,int Select)
{
	if( g_isCharacterBuff(o, eBuff_CrywolfNPCHide) )
	{
		return;
	}

	BMD *b = &Models[o->Type];
	if ( Models[o->Type].NumActions==0 ) return;

	bool Translate = true;

    vec3_t p, Position, Light;

    Vector(0.f, 0.f, 0.f, p);
    Vector(1.f, 1.f, 1.f,Light);

	BYTE byRender = CHARACTER_NONE;

	switch ( c->MonsterIndex)
	{
	case 89:
	case 95:
	case 112:
	case 118:
	case 124:
	case 130:
    case 143:
		{
			BOOL bRender = Calc_RenderObject( o, Translate,Select, 0);

			if ( !c->Object.m_pCloth)
			{
				CPhysicsClothMesh *pCloth = new CPhysicsClothMesh [1];
				pCloth[0].Create( &( c->Object), 2, 18, PCT_HEAVY);
				pCloth[0].AddCollisionSphere( 0.0f, 0.0f, 0.0f, 50.0f, 18);
				pCloth[0].AddCollisionSphere( 0.0f, -20.0f, 0.0f, 30.0f, 18);
				c->Object.m_pCloth = ( void*)pCloth;
				c->Object.m_byNumCloth = 1;
			}
			CPhysicsCloth *pCloth = ( CPhysicsCloth*)c->Object.m_pCloth;

			if ( pCloth)
			{
				if ( !pCloth[0].Move2( 0.005f, 5))
				{
					DeleteCloth( c, o);
				}
				else
				{
					pCloth[0].Render();
				}
			}
			if ( bRender)
			{
				Draw_RenderObject( o, Translate,Select, 0);
			}
		}
		break;

    default:
        if ( o->Type==MODEL_PLAYER )
        {
            byRender = CHARACTER_ANIMATION;
        }
        else
		    byRender = CHARACTER_RENDER_OBJ;
		break;
	}

	if(o->Type == MODEL_PLAYER 
		&& (o->SubType == MODEL_XMAS_EVENT_CHA_SSANTA
		|| o->SubType == MODEL_XMAS_EVENT_CHA_SNOWMAN 
		|| o->SubType == MODEL_XMAS_EVENT_CHA_DEER) )
	{
		OBJECT* pOwner = o->Owner;
	
		if(pOwner->Live == false)
		{
			o->Live = false;
		}

		VectorCopy(pOwner->Position, o->Position);
		VectorCopy(pOwner->Angle, o->Angle);
		o->PriorAction = pOwner->PriorAction;
		o->PriorAnimationFrame = pOwner->PriorAnimationFrame;
		o->CurrentAction = pOwner->CurrentAction;
		o->AnimationFrame = pOwner->AnimationFrame;

		if(timeGetTime() - o->m_dwTime >= XMAS_EVENT_TIME)
		{
			DeleteCharacter(c, o);
		}
	}
	else if(o->Type == MODEL_PLAYER && o->SubType == MODEL_XMAS_EVENT_CHANGE_GIRL
#ifdef PJH_ADD_PANDA_CHANGERING
		|| o->Type == MODEL_PLAYER && o->SubType == MODEL_PANDA
#endif //PJH_ADD_PANDA_CHANGERING
#ifdef YDG_ADD_SKELETON_CHANGE_RING
		|| o->Type == MODEL_PLAYER && o->SubType == MODEL_SKELETON_CHANGED
#endif	// YDG_ADD_SKELETON_CHANGE_RING
		)
	{
		if(o->m_iAnimation >= 1)
		{
			o->m_iAnimation = 0;
			SetPlayerStop(c);
		}
	}

    if ( byRender==CHARACTER_ANIMATION )
        Calc_ObjectAnimation ( o, Translate, Select );

    if ( o->Alpha>=0.5f && c->HideShadow==false )
    {
        if ( World!=WD_10HEAVEN && (o->Type==MODEL_PLAYER) && (!(MODEL_HELPER+2<=c->Helper.Type && c->Helper.Type<=MODEL_HELPER+3) || c->SafeZone ) 
            && InHellas()==false
            )
        {
            if ( InBloodCastle() && o->m_bActionStart && c->Dead>0 )
            {
                float height = RequestTerrainHeight(o->Position[0],o->Position[1]);
                if ( height<o->Position[2] )
                {
                    o->Position[2] = height;
                }
            }
            o->EnableShadow = true;
            RenderPartObject(&c->Object,MODEL_SHADOW_BODY,NULL,c->Light,o->Alpha,0,0,0,false,false,Translate);
            o->EnableShadow = false;
        }
    }

    if ( byRender==CHARACTER_RENDER_OBJ )
	{
		if(67==c->MonsterIndex || 74==c->MonsterIndex || 75==c->MonsterIndex
			|| 135==c->MonsterIndex || 136==c->MonsterIndex || 137==c->MonsterIndex
#ifdef LOREN_RAVINE_EVENT
			|| 300==c->MonsterIndex || 301==c->MonsterIndex || 302==c->MonsterIndex || 303==c->MonsterIndex
#endif
			|| 314==c->MonsterIndex || 315==c->MonsterIndex || 316==c->MonsterIndex || 317==c->MonsterIndex || 318==c->MonsterIndex || 319==c->MonsterIndex
			)
		{
			RenderObject ( o, Translate,Select, c->MonsterIndex);
		}
		else
		{
			if((c->MonsterIndex == 360 && o->CurrentAction == MONSTER01_ATTACK2)) 
			{
				RenderObject_AfterImage(o, Translate, Select, 0);
			}
			else
			{
				RenderObject ( o, Translate,Select, 0);
#ifdef CSK_EVENT_HALLOWEEN_MAP
				if(o->Kind == KIND_NPC && o->Type == MODEL_STORAGE)
				{
					PART_t* pPart = &c->BodyPart[BODYPART_HEAD];
					pPart->LinkBone = 34;
					RenderLinkObject(0.f, 0.f, 0.f, c, pPart, MODEL_POTION+45, 0, 0, true, true);
				}
#endif // CSK_EVENT_HALLOWEEN_MAP
			}
		}
	}

	if ( ( 
           o->Type!=MODEL_PLAYER && o->Kind!=KIND_TRAP && 
		   c->MonsterIndex!=25 && c->MonsterIndex!=22 && c->MonsterIndex!=42 && c->MonsterIndex!=242 && c->MonsterIndex!=59 && c->MonsterIndex!=63 
           && c->MonsterIndex!=152 
         ) && World!=WD_10HEAVEN 
       )
	{
        if ( o->Alpha>=0.3f )
		{
            if ( InBloodCastle() == true && o->m_bActionStart && c->Dead>0 )
            {
                float height = RequestTerrainHeight(o->Position[0],o->Position[1]);
                if ( height<o->Position[2] )
                {
                    o->Position[2] = height;
                }
            }

            if ( c->MonsterIndex==232 )
                o->HiddenMesh = 2;
            else if ( c->MonsterIndex==233 )
                o->HiddenMesh = 2;

            if ( o->Type!=MODEL_MONSTER01+60 && o->Type!=MODEL_MONSTER01+61
				&& !( o->Type >= MODEL_FACE 
#ifdef PBG_ADD_NEWCHAR_MONK
				&& o->Type <= MODEL_FACE+6) )
#else //PBG_ADD_NEWCHAR_MONK
				&& o->Type <= MODEL_FACE+5) )
#endif //PBG_ADD_NEWCHAR_MONK
            {
                o->EnableShadow = true;
				RenderPartObject(&c->Object,o->Type,NULL,c->Light,o->Alpha,0,0,0,false,false,Translate);
				o->EnableShadow = false;
            }
            if ( c->MonsterIndex==232 || c->MonsterIndex==233 )
            {
				EnableAlphaBlend();

                vec3_t Position, Light;

                VectorCopy ( o->Position, Position );
                Position[2] += 20.f;

                float Luminosity = sinf ( WorldTime*0.0015f )*0.3f+0.8f;
                
                Vector ( Luminosity*0.5f, Luminosity*0.5f, Luminosity, Light );
                RenderTerrainAlphaBitmap(BITMAP_MAGIC+1,o->Position[0],o->Position[1],2.7f,2.7f,Light,-o->Angle[2]);

                o->HiddenMesh = -1;
            }
#ifdef _PVP_MURDERER_HERO_ITEM
            if ( c->MonsterIndex==228 )
            {
				EnableAlphaBlend();
                vec3_t Light;
                float Luminosity = sinf ( WorldTime*0.0015f )*0.3f+0.8f;
                Vector ( Luminosity*0.5f, Luminosity*0.5f, Luminosity, Light );
                RenderTerrainAlphaBitmap(BITMAP_MAGIC+1,o->Position[0],o->Position[1]-10,5.0f,5.0f,Light,-o->Angle[2]);

                o->HiddenMesh = -1;
            }
            else if ( c->MonsterIndex==227 )
            {
				EnableAlphaBlend();
                vec3_t Light;
                float Luminosity = sinf ( WorldTime*0.0015f )*0.2f+0.4f;
                Vector ( Luminosity, Luminosity*0.0f, Luminosity*0.0f, Light );
                RenderTerrainAlphaBitmap(BITMAP_MAGIC+1,o->Position[0],o->Position[1],2.7f,2.7f,Light,-o->Angle[2]);

                o->HiddenMesh = -1;
            }
#endif	// _PVP_MURDERER_HERO_ITEM
		}
	}

	if ( c->MonsterIndex==38 || c->MonsterIndex==43 || c->MonsterIndex==52 || c->MonsterIndex==59 || c->MonsterIndex==67
#ifdef USE_EVENT_ELDORADO
		|| ( 78 <= c->MonsterIndex && c->MonsterIndex <= 83 )
#endif // USE_EVENT_ELDORADO
#ifdef CSK_ADD_GOLDCORPS_EVENT
#ifdef KJH_FIX_GOLD_RABBIT_INDEX
		|| (c->MonsterIndex >= 493 && c->MonsterIndex <= 502)
#else // KJH_FIX_GOLD_RABBIT_INDEX
		|| (c->MonsterIndex >= 492 && c->MonsterIndex <= 501)
#endif // KJH_FIX_GOLD_RABBIT_INDEX
#endif // CSK_ADD_GOLDCORPS_EVENT
		)//발록,황금버지드래곤
	{
#ifdef CSK_ADD_GOLDCORPS_EVENT
		vec3_t vBackupBodyLight;
#endif // CSK_ADD_GOLDCORPS_EVENT

		// 황금 효과
		float Bright = 1.f;
		if(c->MonsterIndex==59)
			Bright = 0.5f;
		if(c->MonsterIndex==43
#ifdef USE_EVENT_ELDORADO
			|| ( 78<=c->MonsterIndex && c->MonsterIndex<=83)
#endif // USE_EVENT_ELDORADO
#ifdef CSK_ADD_GOLDCORPS_EVENT
#ifdef KJH_FIX_GOLD_RABBIT_INDEX
			|| (c->MonsterIndex >= 493 && c->MonsterIndex <= 502)
#else // KJH_FIX_GOLD_RABBIT_INDEX
			|| (c->MonsterIndex >= 492 && c->MonsterIndex <= 501)
#endif // KJH_FIX_GOLD_RABBIT_INDEX
#endif // CSK_ADD_GOLDCORPS_EVENT
			)
		{
#ifdef CSK_ADD_GOLDCORPS_EVENT
#ifdef KJH_FIX_GOLD_RABBIT_INDEX
			if(c->MonsterIndex >= 493)
#else // KJH_FIX_GOLD_RABBIT_INDEX
			if(c->MonsterIndex >= 492)
#endif // KJH_FIX_GOLD_RABBIT_INDEX
			{
				VectorCopy(Models[o->Type].BodyLight, vBackupBodyLight);
				Vector(1.f, 0.6f, 0.3f, Models[o->Type].BodyLight);
			}

			if(c->MonsterIndex == 501)
			{
				VectorCopy(Models[o->Type].BodyLight, vBackupBodyLight);
				Vector(1.f, 0.0f, 0.0f, Models[o->Type].BodyLight);

#ifdef LDS_ADD_GOLDCORPS_EVENT_MOD_GREATDRAGON
				float	fEffectScale = o->Scale * 1.6f;
				vec3_t	v3EffectLightColor, v3EffectPosition;

				Vector(1.0f, 0.6f, 0.1f,v3EffectLightColor);

				b->TransformPosition(o->BoneTransform[0],p,v3EffectPosition,true);
 				CreateSprite(BITMAP_LIGHTMARKS, v3EffectPosition, 2.5f, v3EffectLightColor, o);

				b->TransformPosition(o->BoneTransform[4],p,v3EffectPosition,true);
 				CreateSprite(BITMAP_LIGHTMARKS, v3EffectPosition, 2.5f, v3EffectLightColor, o);

				b->TransformPosition(o->BoneTransform[57],p,v3EffectPosition,true);
 				CreateSprite(BITMAP_LIGHTMARKS, v3EffectPosition, 2.5f, v3EffectLightColor, o);

				b->TransformPosition(o->BoneTransform[60],p,v3EffectPosition,true);
 				CreateSprite(BITMAP_LIGHTMARKS, v3EffectPosition, 2.5f, v3EffectLightColor, o);

				b->TransformPosition(o->BoneTransform[87],p,v3EffectPosition,true);
 				CreateSprite(BITMAP_LIGHTMARKS, v3EffectPosition, 2.5f, v3EffectLightColor, o);

				Vector(1.0f, 0.8f, 0.1f,v3EffectLightColor);

				b->TransformPosition(o->BoneTransform[57],p,v3EffectPosition,true);
				CreateEffect(MODEL_EFFECT_FIRE_HIK3_MONO, v3EffectPosition, o->Angle, v3EffectLightColor, 1, NULL, -1, 0, 0, 0,fEffectScale);	
				
				b->TransformPosition(o->BoneTransform[60],p,v3EffectPosition,true);
				CreateEffect(MODEL_EFFECT_FIRE_HIK3_MONO, v3EffectPosition, o->Angle, v3EffectLightColor, 1, NULL, -1, 0, 0, 0,fEffectScale);	
				
				b->TransformPosition(o->BoneTransform[66],p,v3EffectPosition,true);
				CreateEffect(MODEL_EFFECT_FIRE_HIK3_MONO, v3EffectPosition, o->Angle, v3EffectLightColor, 1, NULL, -1, 0, 0, 0,fEffectScale);	
				
				b->TransformPosition(o->BoneTransform[78],p,v3EffectPosition,true);
				CreateEffect(MODEL_EFFECT_FIRE_HIK3_MONO, v3EffectPosition, o->Angle, v3EffectLightColor, 1, NULL, -1, 0, 0, 0,fEffectScale);	
				
				b->TransformPosition(o->BoneTransform[91],p,v3EffectPosition,true);
				CreateEffect(MODEL_EFFECT_FIRE_HIK3_MONO, v3EffectPosition, o->Angle, v3EffectLightColor, 1, NULL, -1, 0, 0, 0,fEffectScale);
#endif // LDS_ADD_GOLDCORPS_EVENT_MOD_GREATDRAGON
			}
#endif // CSK_ADD_GOLDCORPS_EVENT
			
      		RenderPartObjectBodyColor(&Models[o->Type],o,o->Type,o->Alpha,RENDER_METAL|RENDER_BRIGHT,Bright);
		}

		if(c->MonsterIndex==67)
		{
			RenderPartObjectBodyColor(&Models[o->Type],o,o->Type,o->Alpha,RENDER_CHROME|RENDER_BRIGHT|RENDER_EXTRA,Bright);
		}
		else
		{
			RenderPartObjectBodyColor(&Models[o->Type],o,o->Type,o->Alpha,RENDER_CHROME|RENDER_BRIGHT,Bright);
		}

#ifdef CSK_ADD_GOLDCORPS_EVENT
#ifdef KJH_FIX_GOLD_RABBIT_INDEX
		if(c->MonsterIndex >= 493 && c->MonsterIndex <= 502)
#else // KJH_FIX_GOLD_RABBIT_INDEX
		if(c->MonsterIndex >= 492 && c->MonsterIndex <= 501)
#endif // KJH_FIX_GOLD_RABBIT_INDEX
		{
			VectorCopy(vBackupBodyLight, Models[o->Type].BodyLight);
		}
#endif // CSK_ADD_GOLDCORPS_EVENT	
	}
    else if(c->MonsterIndex==69)
    {
    	float Luminosity = (float)(rand()%30+70)*0.01f;
        Vector(Luminosity*0.8f,Luminosity*0.9f,Luminosity*1.f,Light);

		for( int i=0; i<9; ++i )
        {
    		b->TransformPosition(o->BoneTransform[g_chStar[i]],p,Position,true);
			CreateSprite(BITMAP_LIGHT,Position,0.6f,Light,o);
        }

        Vector(Luminosity*0.6f,Luminosity*0.7f,Luminosity*0.8f,Light);

		for(int i=0;i<3;i++)
		{
			Vector((float)(rand()%20-10),(float)(rand()%20-10),(float)(rand()%20-10),p);
			b->TransformPosition(o->BoneTransform[rand()%b->NumBones],p,Position,true);
           	CreateParticle(BITMAP_SPARK+1,Position,o->Angle,Light,3);
		}
    }
    else if(c->MonsterIndex==70)
    {
    	b->TransformPosition(o->BoneTransform[20],p,Position,true);
        CreateSprite(BITMAP_LIGHT,Position,0.8f,Light,o);
    }
    else if(c->MonsterIndex==71 || c->MonsterIndex==74
#ifdef LOREN_RAVINE_EVENT
		|| c->MonsterIndex==301
#endif //LOREN_RAVINE_EVENT
		)
    {
		if ( !c->Object.m_pCloth)
		{
			int iTex = ( c->MonsterIndex==71) ? BITMAP_ROBE+3 : BITMAP_ROBE+5;
			CPhysicsCloth *pCloth = new CPhysicsCloth [1];
			pCloth[0].Create( &( c->Object), 19, 0.0f, 10.0f, 0.0f, 5, 15, 30.0f, 300.0f, iTex, iTex, PCT_RUBBER | PCT_MASK_ALPHA);
			c->Object.m_pCloth = ( void*)pCloth;
			c->Object.m_byNumCloth = 1;
		}
		CPhysicsCloth *pCloth = ( CPhysicsCloth*)c->Object.m_pCloth;
		if ( !pCloth[0].Move2( 0.005f, 5))
		{
			DeleteCloth( c, o);
		}
		else
		{
			pCloth[0].Render();
		}
    }
    else if(c->MonsterIndex==73 || c->MonsterIndex==75)
    {
        vec3_t pos1, pos2;
		switch ( c->MonsterIndex)
		{
		case 73:
			Vector(0.1f,0.1f,1.f,Light);

			for(int i=13; i<27; ++i)
			{
    			b->TransformPosition(o->BoneTransform[i],p,Position,true);
				CreateSprite(BITMAP_LIGHT,Position,0.8f,Light,o);

				VectorCopy( Position, pos2 );
				if( i>=14 && i<=16 || i==23 )
				{
					CreateJoint(BITMAP_JOINT_THUNDER,pos1,pos2,o->Angle,7,NULL,20.f);
				}
				VectorCopy( Position, pos1 );
			}

			for(int i=52; i<59; ++i)
			{
    			b->TransformPosition(o->BoneTransform[i],p,Position,true);
				CreateSprite(BITMAP_LIGHT,Position,0.8f,Light,o);
			}
			break;
		case 75:
			Vector(1.f,1.f,1.0f,Light);

			for(int i=18; i<19; ++i)
			{
				Vector(0.f,0.f,0.f,p);
    			b->TransformPosition(o->BoneTransform[i],p,Position,true);
				CreateParticle(BITMAP_FIRE,Position,o->Angle,Light,0, 0.3f);
			}
			break;
		}

		int RenderType = ( c->MonsterIndex==73) ? 0 : RENDER_EXTRA;
      	RenderPartObjectBodyColor(&Models[o->Type],o,o->Type,o->Alpha,RENDER_CHROME|RENDER_BRIGHT|RenderType,1.f);

		if ( c->MonsterIndex==73)
		{
      		RenderPartObjectBodyColor2(&Models[o->Type],o,o->Type,o->Alpha,RENDER_CHROME2|RENDER_LIGHTMAP|RENDER_BRIGHT,1.f);
		}
    }
	else if ( c->MonsterIndex==77)
	{	
		float fSin = 0.5f * ( 1.0f + sinf( ( float)( ( int)WorldTime % 10000) * 0.001f));
		RenderPartObjectBodyColor(&Models[o->Type],o,o->Type,o->Alpha,RENDER_CHROME|RENDER_BRIGHT,0.3f+fSin*0.7f);
		fSin = 0.3f * ( 1.0f - fSin) + 0.3f;
		o->BlendMeshLight = fSin;
		o->BlendMesh = 0;
		RenderObject(o,Translate,2,0);
		RenderObject(o,Translate,3,0);
		o->BlendMesh = -1;
		memcpy( g_fBoneSave[2], o->BoneTransform[24], 3 * 4 * sizeof ( float));
		o->Type++;
		RenderObject(o,Translate,Select,0);
		memcpy( g_fBoneSave[0], o->BoneTransform[23], 3 * 4 * sizeof ( float));
		memcpy( g_fBoneSave[1], o->BoneTransform[14], 3 * 4 * sizeof ( float));

		if ( !c->Object.m_pCloth)
		{
			CPhysicsCloth *pCloth = new CPhysicsCloth [1];
			pCloth[0].Create( o, 10, 0.0f, -10.0f, 0.0f, 5, 12, 15.0f, 240.0f, BITMAP_PHO_R_HAIR, BITMAP_PHO_R_HAIR, PCT_RUBBER | PCT_MASK_ALPHA);
			pCloth[0].AddCollisionSphere( 0.0f, 0.0f, 40.0f, 30.0f, 10);
			o->m_pCloth = ( void*)pCloth;
			o->m_byNumCloth = 1;
		}
		CPhysicsCloth *pCloth = ( CPhysicsCloth*)c->Object.m_pCloth;
		if ( !pCloth[0].Move2( 0.005f, 5))
		{
			DeleteCloth( c, o);
		}
		else
		{
			pCloth[0].Render();
		}
		o->Type--;
	}

	if ( c->MonsterIndex==53 || c->MonsterIndex==54 )
	{
    	RenderPartObjectBodyColor(&Models[o->Type],o,o->Type,o->Alpha,RENDER_METAL|RENDER_BRIGHT,1.f,BITMAP_SHINY+1);
	}

	if(c->MonsterIndex==42)
	{
		PART_t *w = &c->Wing;
		w->Type          = MODEL_BOSS_HEAD;
		w->LinkBone      = 9;
		w->CurrentAction = 1;
		w->PriorAction   = 1;
		w->PlaySpeed     = 0.2f;
		RenderLinkObject(0.f,0.f,-40.f,c,w,w->Type,0,0,false,Translate);

		w->Type          = MODEL_PRINCESS;
		w->LinkBone      = 61;
		float TempScale = o->Scale;
		vec3_t TempLight;
		VectorCopy(c->Light,TempLight);
		Vector(1.f,1.f,1.f,c->Light);
		o->Scale         = 0.9f;
		RenderLinkObject(0.f,-40.f,45.f,c,w,w->Type,0,0,false,Translate);
		VectorCopy(TempLight,c->Light);
		o->Scale         = TempScale;
	}
    else if ( c->MonsterIndex>=132 && c->MonsterIndex<=134 )
    {
		PART_t *w = &c->Wing;
		w->LinkBone      = 1;
		w->CurrentAction = 1;
		w->PriorAction   = 1;
		w->PlaySpeed     = 0.2f;
		float TempScale = o->Scale;
		o->Scale         = 0.7f;

        if ( c->MonsterIndex==132 ) w->Type = MODEL_STAFF+10;
        if ( c->MonsterIndex==133 ) w->Type = MODEL_SWORD+19;
        if ( c->MonsterIndex==134 )
        {
            w->Type = MODEL_BOW+18;
            o->Scale= 0.9f;
        }

		RenderLinkObject ( 0.f,0.f,0.f,c,w,w->Type,0,0,true,true);
		o->Scale         = TempScale;
    }
    else if ( c->MonsterIndex==135 )
    {
		RenderPartObjectBodyColor(&Models[o->Type],o,o->Type,o->Alpha,RENDER_BRIGHT|RENDER_EXTRA,1.0f);
	}
	
#ifdef PRUARIN_EVENT07_3COLORHARVEST
	if( o->Type == MODEL_MONSTER01+127 )
	{	
		vec3_t vLight;
		vec3_t vPos, vRelatedPos;
		Vector(0.3f, 0.3f, 0.0f, vLight);
		Vector(0.f, 0.f, 0.f, vRelatedPos);

		if( o->CurrentAction == MONSTER01_WALK )
		{
			o->m_iAnimation++;
			if( o->m_iAnimation%20 == 0)
				PlayBuffer( SOUND_MOONRABBIT_WALK );
		}

		if( o->CurrentAction == MONSTER01_SHOCK )
		{
			if( o->AnimationFrame > 2.f && o->AnimationFrame <= 3.f)
				PlayBuffer( SOUND_MOONRABBIT_DAMAGE );
		}
		
		if( o->CurrentAction == MONSTER01_DIE )
		{
			if( o->AnimationFrame > 1.f && o->AnimationFrame <= 2.f)
				PlayBuffer( SOUND_MOONRABBIT_DEAD );

			if(o->AnimationFrame > 9.f)
			{
				if( o->SubType != -1 )
				{
					Vector(0.5f, 0.5f, 0.0f, vLight);
 					BoneManager::GetBonePosition(o, "Rabbit_1", vPos);

					Vector(0.7f, 1.0f, 0.6f, vLight);
					vec3_t vMoonPos;
					VectorCopy( vPos, vMoonPos );
					vMoonPos[2] += 28.f;
					CreateEffect( MODEL_MOONHARVEST_MOON, vMoonPos, o->Angle, vLight, 0, NULL, -1, 0, 0, 0, 0.5f );
					
					Vector(0.4f, 0.4f, 0.8f, vLight);
					CreateParticle(BITMAP_EXPLOTION_MONO, vPos, o->Angle, vLight, 10, 1.0f);	
					
					Vector(1.0f, 1.0f, 1.0f, vLight);
					for( int i=0 ; i<200 ; i++)
					{
						CreateParticle(BITMAP_SPARK+1, vPos, o->Angle, vLight, 21 );
					}				

					for( int i=0 ; i<150 ; i++ )
					{
						CreateParticle(BITMAP_SPARK+1, vPos, o->Angle, vLight, 22 );
					}
					Vector(1.0f, 1.0f, 1.0f, vLight);
					switch(o->SubType)
					{
					case 0:
						{
							for( int i=0 ; i<10 ; i++ )
							{
								CreateEffect(MODEL_MOONHARVEST_GAM, vPos, o->Angle, vLight);
							}
						}break;
					case 1:
						{
							for( int i=0 ; i<5 ; i++ )
							{
								CreateEffect(MODEL_MOONHARVEST_SONGPUEN1, vPos, o->Angle, vLight);
								CreateEffect(MODEL_MOONHARVEST_SONGPUEN2, vPos, o->Angle, vLight);
							}
						}					
						break;
					case 2:
						{
							for( int i=0 ; i<10 ; i++ )
							{
								CreateEffect(MODEL_NEWYEARSDAY_EVENT_BEKSULKI, vPos, o->Angle, vLight);	
							}
						}			
						break;
					}	
					
					o->SubType = -1;			
					
					if(o->AnimationFrame <= 10.f)
					{
						// flare
						Vector(1.0f, 0.0f, 0.0f, vLight);
						CreateSprite( BITMAP_LIGHT, vPos, 8.0f, vLight, o );	
					}
				}
			}
		}	
		else
		{
			BoneManager::GetBonePosition(o, "Rabbit_1", vPos);		// Bip01 Spine 
			Vector(0.4f, 0.4f, 0.9f, vLight);
			CreateSprite( BITMAP_LIGHT, vPos, 3.0f, vLight, o );	// flare01.jpg 

			BoneManager::GetBonePosition(o, "Rabbit_2", vPos);		// Bip01 Head
			Vector(0.4f, 0.4f, 0.9f, vLight);
			CreateSprite( BITMAP_LIGHT, vPos, 2.0f, vLight, o );	// flare01.jpg 

			BoneManager::GetBonePosition(o, "Rabbit_4", vPos);		// Bip01 Pelvis
			Vector(0.4f, 0.4f, 0.9f, vLight);
			CreateSprite( BITMAP_LIGHT, vPos, 1.0f, vLight, o );	// flare01.jpg 
		}
	}
#endif // PRUARIN_EVENT07_3COLORHARVEST
#ifdef CSK_EVENT_CHERRYBLOSSOM
	if(o->Type == MODEL_NPC_CHERRYBLOSSOM)
	{
		vec3_t vRelativePos, vtaWorldPos, vLight, vLight1, vLight2;
		BMD *b = &Models[MODEL_NPC_CHERRYBLOSSOM];

		Vector(0.f, 0.f, 0.f, vRelativePos);
		Vector(1.f, 0.6f, 0.8f, vLight);
		Vector(0.3f, 0.3f, 0.3f,vLight1);
		Vector(1.f, 1.f, 1.f, vLight2);

		float Scale = absf(sinf(WorldTime*0.002f))*0.2f;
		float Luminosity = absf(sinf(WorldTime*0.002f))*0.4f;
		Vector ( 0.5f + Luminosity, 0.0f + Luminosity, 0.0f + Luminosity, Light );
		
		if( rand()%2 == 0 )
		{
			VectorCopy(o->Position, b->BodyOrigin);
			b->Animation(BoneTransform,o->AnimationFrame,o->PriorAnimationFrame,o->PriorAction, o->Angle, o->HeadAngle);
			b->TransformPosition(BoneTransform[43],vRelativePos,vtaWorldPos,false);
			vtaWorldPos[2] += 20.f;
			
			CreateParticle(BITMAP_CHERRYBLOSSOM_EVENT_PETAL, vtaWorldPos, o->Angle, rand()%3 == 0 ? vLight : vLight1, 1, 0.3f);
		}

		VectorCopy(o->Position, b->BodyOrigin);
		b->Animation(BoneTransform,o->AnimationFrame,o->PriorAnimationFrame,o->PriorAction, o->Angle, o->HeadAngle);
		b->TransformPosition(BoneTransform[43],vRelativePos,vtaWorldPos,false);
		vtaWorldPos[2] += 20.f;

		CreateParticle(BITMAP_SPARK+1, vtaWorldPos, o->Angle, rand()%3 == 0 ? vLight2 : vLight1, 25, Scale + 0.2f);
		CreateParticle(BITMAP_SPARK+1, vtaWorldPos, o->Angle, rand()%2 == 0 ? vLight2 : vLight1, 25, Scale + 0.3f);
		
		Vector(0.7f, 0.5f, 0.2f, vLight);
		CreateSprite(BITMAP_LIGHT, vtaWorldPos, 2.f, vLight, o, 0.f);

		Vector(0.7f, 0.2f, 0.6f, vLight);
		VectorCopy(o->Position, b->BodyOrigin);
		b->Animation(BoneTransform,o->AnimationFrame,o->PriorAnimationFrame,o->PriorAction, o->Angle, o->HeadAngle);
		b->TransformPosition(BoneTransform[3],vRelativePos,vtaWorldPos,false);
		CreateSprite(BITMAP_LIGHT, vtaWorldPos, 5.f, vLight, o, 0.f);

		int iRedFlarePos[] = { 53, 56, 59, 62 };
		for (int i = 0; i < 4; ++i) {
			Vector(0.9f, 0.4f, 0.8f, vLight);
			VectorCopy(o->Position, b->BodyOrigin);
			b->Animation(BoneTransform,o->AnimationFrame,o->PriorAnimationFrame,o->PriorAction, o->Angle, o->HeadAngle);
			b->TransformPosition(BoneTransform[iRedFlarePos[i]],vRelativePos,vtaWorldPos,false);

			CreateSprite(BITMAP_LIGHT, vtaWorldPos, 1.5f, vLight, o, 0.f);

			if( rand()%3 == 0 ){

				float randpos = (float)(rand()%30+5);

				if(rand()%2 == 0){
					vtaWorldPos[0] += randpos;
				}
				else{
					vtaWorldPos[0] -= randpos;
				}

				Vector(1.f, 0.8f, 0.4f, vLight);
				CreateParticle(BITMAP_SPARK+1, vtaWorldPos, o->Angle, vLight, 15, Scale + 0.4f);
			}
		}
	}
#endif //CSK_EVENT_CHERRYBLOSSOM
#ifdef PSW_EVENT_CHERRYBLOSSOMTREE
	if(o->Type == MODEL_NPC_CHERRYBLOSSOMTREE)
	{
	}
#endif //PSW_EVENT_CHERRYBLOSSOMTREE

    bool fullset = false;
    if(o->Type == MODEL_PLAYER)
    {
		if (c->HideShadow == false)
		{
	        fullset = CheckFullSet(c);
		}
    }

	bool bCloak = false;

	if((
		GetCharacterClass(c->Class)==CLASS_DARK 
		|| GetBaseClass(c->Class)==CLASS_DARK_LORD
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
		|| GetBaseClass(c->Class)==CLASS_RAGEFIGHTER
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
		) && o->Type==MODEL_PLAYER)
	{
		if(c->Change == false || (c->Change == true && c->Object.Type == MODEL_PLAYER))
		{
			bCloak = true;
		}
	}

	if ( c->MonsterIndex==55 || c->MonsterIndex == 361)
	{
		bCloak = true;
	}
#ifdef YDG_ADD_DOPPELGANGER_MONSTER
	if (c->MonsterIndex >= 529 && c->MonsterIndex <= 539)
	{
		bCloak = false;
	}
#endif	// YDG_ADD_DOPPELGANGER_MONSTER

#ifdef CAMERA_TEST_FPS
    bCloak = false;
#endif

#ifndef GUILD_WAR_EVENT
    if ( InChaosCastle() == true )
    {
        bCloak = false;
    }
#endif //GUILD_WAR_EVENT

    vec3_t CloakLight;
	Vector(1.f,1.f,1.f,CloakLight);
	if ( c->GuildMarkIndex!=-1 )
	{
#ifdef GUILD_WAR_EVENT
		if(EnableSoccer || EnableGuildWar)
#else
		if(EnableSoccer)
#endif
		{
			if(strcmp(GuildMark[Hero->GuildMarkIndex].GuildName, GuildMark[c->GuildMarkIndex].GuildName) == NULL)
			{
				bCloak = true;
				if(HeroSoccerTeam == 0)
				{
					Vector(1.f,0.2f,0.f,CloakLight);
				}
				else
				{
					Vector(0.f,0.2f,1.f,CloakLight);
				}
			}
			if(strcmp(GuildWarName,GuildMark[c->GuildMarkIndex].GuildName)==NULL)
			{
				bCloak = true;
				if(HeroSoccerTeam == 0)
				{
					Vector(0.f,0.2f,1.f,CloakLight);
				}
				else
				{
					Vector(1.f,0.2f,0.f,CloakLight);
				}
			}
		}
		if(SoccerObserver)
		{
			if(strcmp(SoccerTeamName[0],GuildMark[c->GuildMarkIndex].GuildName)==NULL)
			{
				bCloak = true;
				Vector(1.f,0.2f,0.f,CloakLight);
			}
			if(strcmp(SoccerTeamName[1],GuildMark[c->GuildMarkIndex].GuildName)==NULL)
			{
				bCloak = true;
				Vector(0.f,0.2f,1.f,CloakLight);
			}
		}
	}
#ifdef YDG_ADD_NEW_DUEL_SYSTEM
	if(g_DuelMgr.IsDuelEnabled())
	{
#ifdef YDG_FIX_DUEL_SUMMON_CLOAK
		if (g_DuelMgr.IsDuelPlayer(c, DUEL_ENEMY, FALSE))
#else	// YDG_FIX_DUEL_SUMMON_CLOAK
		if (g_DuelMgr.IsDuelPlayer(c, DUEL_ENEMY))
#endif	// YDG_FIX_DUEL_SUMMON_CLOAK
		{
			// 결투 상대일경우
			bCloak = true;
			Vector(1.f,0.2f,0.f,CloakLight);
		}
#ifdef YDG_FIX_DUEL_SUMMON_CLOAK
		else if (g_DuelMgr.IsDuelPlayer(c, DUEL_HERO, FALSE))
#else	// YDG_FIX_DUEL_SUMMON_CLOAK
		else if (g_DuelMgr.IsDuelPlayer(c, DUEL_HERO))
#endif	// YDG_FIX_DUEL_SUMMON_CLOAK
		{
			// 상대편일 경우
			bCloak = true;
			Vector(0.f,0.2f,1.f,CloakLight);
		}
	}
#else	// YDG_ADD_NEW_DUEL_SYSTEM
#ifdef DUEL_SYSTEM
	if(g_bEnableDuel){		//. 결투중이라면
		if( c->Key == g_iDuelPlayerIndex && strcmp(c->ID, g_szDuelPlayerID) == 0){
			//. 결투 상대일경우
			bCloak = true;
			Vector(1.f,0.2f,0.f,CloakLight);
		}
		if( c->Key == Hero->Key && strcmp(c->ID, Hero->ID) == 0){
			//. 상대편일 경우
			bCloak = true;
			Vector(0.f,0.2f,1.f,CloakLight);
		}
	}
#endif // DUEL_SYSTEM
#endif	// YDG_ADD_NEW_DUEL_SYSTEM

	if(SceneFlag == CHARACTER_SCENE)
	{
		Vector(0.4f, 0.4f, 0.4f, Light);
	}
	else
	{
		RequestTerrainLight ( o->Position[0], o->Position[1], Light );
	}

	VectorAdd(Light, o->Light, c->Light);
	if ( o->Type == MODEL_MONSTER01+55)
	{
		Vector( .6f, .6f, .6f, c->Light);
	}
#ifdef YDG_ADD_DOPPELGANGER_MONSTER
	else if (c->MonsterIndex >= 529 && c->MonsterIndex <= 539)
	{
		c->HideShadow = true;

		if (World == WD_65DOPPLEGANGER1)
		{
			Vector( 0.5f, 0.7f, 1.0f, c->Light);
			c->Object.Alpha = 0.7f;
		}
		else
		{
			Vector( 1.0f, 0.3f, 0.1f, c->Light);
		}

		if (World == WD_65DOPPLEGANGER1);
		else
		if (o->CurrentAction!=PLAYER_DIE1)
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

			Vector( 1.0f * fAlpha, 0.3f * fAlpha, 0.1f * fAlpha, c->Light);

// 			vec3_t vLight;
			vec3_t vPos;
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
			BMD *b = &Models[o->Type];
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
			//Vector(0.5f, 0.5f, 0.5f, vLight);

			int iNumBones = Models[o->Type].NumBones;
			//for (int i = 0; i < 10; ++i)
			{
				Models[o->Type].TransformByObjectBone(vPos, o, rand()%iNumBones);	
				CreateParticle(BITMAP_TWINTAIL_WATER, vPos, o->Angle, c->Light, 2, 0.5f);
			}
		}
		c->Object.BlendMesh = -1;

		if (g_isCharacterBuff((&c->Object), eBuff_Doppelganger_Ascension))
		{
			o->Position[2] += 2.0f;
		}
	}
#endif	// YDG_ADD_DOPPELGANGER_MONSTER
#ifdef _PVP_MURDERER_HERO_ITEM
	else if (c->MonsterIndex == 227)
	{
		c->PK = PVP_MURDERER2;
	}
#endif	// _PVP_MURDERER_HERO_ITEM

	if ( c==Hero )
	{
		vec3_t AbilityLight = { 1.f, 1.f, 1.f};
		if(CharacterAttribute->Ability & ABILITY_FAST_ATTACK_SPEED)
		{
			AbilityLight[0] *= 0.9f; AbilityLight[1] *= 0.5f; AbilityLight[2] *= 0.5f;
		}
		if(CharacterAttribute->Ability & ABILITY_PLUS_DAMAGE)
		{
			AbilityLight[0] *= 0.5f; AbilityLight[1] *= 0.9f; AbilityLight[2] *= 0.5f;
		}
		if(CharacterAttribute->Ability & ABILITY_FAST_ATTACK_RING)
		{
			AbilityLight[0] *= 0.9f; AbilityLight[1] *= 0.5f; AbilityLight[2] *= 0.5f;
		}
        if(CharacterAttribute->Ability & ABILITY_FAST_ATTACK_SPEED2)
		{
			AbilityLight[0] *= 0.9f; AbilityLight[1] *= 0.5f; AbilityLight[2] *= 0.5f;
		}
		if(SceneFlag == CHARACTER_SCENE)
		{
			Vector(0.5f, 0.5f, 0.5f, Light);
			VectorAdd(Light,o->Light,c->Light);
		}
		else
			VectorCopy( AbilityLight, c->Light);
	}

    if(o->Kind==KIND_NPC && World==WD_0LORENCIA && o->Type==MODEL_PLAYER && (o->SubType>=MODEL_SKELETON1&&o->SubType<=MODEL_SKELETON3) )
    {
        RenderPartObject(&c->Object,o->SubType,NULL,c->Light,o->Alpha,c->Level<<3,0,0,false,false,Translate,Select);
    }
#ifdef PBG_MOD_RAGEFIGHTERSOUND
	else if(o->Kind==KIND_PLAYER && o->Type==MODEL_PLAYER && GetBaseClass(c->Class) == CLASS_RAGEFIGHTER
		&& (o->SubType==MODEL_SKELETON_PCBANG || o->SubType == MODEL_HALLOWEEN
		|| o->SubType == MODEL_PANDA || o->SubType == MODEL_SKELETON_CHANGED
		|| o->SubType == MODEL_XMAS_EVENT_CHANGE_GIRL || o->SubType == MODEL_SKELETON1))
	{
		PART_t* p = &c->BodyPart[o->SubType];
		if(o->CurrentAction == PLAYER_SKILL_DRAGONKICK)
		{
			p->Type = o->SubType;
			RenderCharacter_AfterImage(c, p, Translate, Select, 2.5f, 1.0f);
		}
		else if(o->CurrentAction == PLAYER_SKILL_DARKSIDE_READY || o->CurrentAction == PLAYER_SKILL_DARKSIDE_ATTACK)
		{
			OBJECT* pObj = &c->Object;
			if(pObj->m_sTargetIndex<0 || c->JumpTime > 0)
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
			RenderPartObject(&c->Object,o->SubType,NULL,c->Light,o->Alpha,0,0,0,false,false,Translate,Select);
		}

		if(o->SubType == MODEL_XMAS_EVENT_CHANGE_GIRL)
		{
			if(c->m_pTempParts == NULL)
			{
				c->m_pTempParts = (CSIPartsMDL*)new CSParts(MODEL_XMAS_EVENT_EARRING, 20);
				CSIPartsMDL* pParts = (CSIPartsMDL*) c->m_pTempParts;
				OBJECT* pObj = pParts->GetObject();
				pObj->Velocity = 0.25f;
				pObj->Owner = &c->Object;
			}
			else
			{
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
				CSIPartsMDL* pParts = (CSIPartsMDL*) c->m_pTempParts;
				OBJECT* pObj = pParts->GetObject();
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
				RenderParts ( c );
			}
		}
	}
#endif //PBG_MOD_RAGEFIGHTERSOUND
	// 엘리트 해골전사 모델 렌더링
	else if(o->Kind==KIND_PLAYER && o->Type==MODEL_PLAYER && o->SubType==MODEL_SKELETON_PCBANG)
	{
        RenderPartObject(&c->Object,o->SubType,NULL,c->Light,o->Alpha,0,0,0,false,false,Translate,Select);
	}
	// 할로윈 이벤트 변신반지 모델 렌더링
	else if(o->Kind == KIND_PLAYER && o->Type == MODEL_PLAYER && o->SubType == MODEL_HALLOWEEN)
	{
		RenderPartObject(&c->Object,o->SubType,NULL,c->Light,o->Alpha,0,0,0,false,false,Translate,Select);
	}
	else if( o->Kind == KIND_PLAYER && o->Type == MODEL_PLAYER && o->SubType == MODEL_CURSEDTEMPLE_ALLIED_PLAYER )
	{
		RenderPartObject(&c->Object,o->SubType,NULL,c->Light,o->Alpha,0,0,0,false,false,Translate,Select);
	}
	else if( o->Kind == KIND_PLAYER && o->Type == MODEL_PLAYER && o->SubType == MODEL_CURSEDTEMPLE_ILLUSION_PLAYER )
	{
		RenderPartObject(&c->Object,o->SubType,NULL,c->Light,o->Alpha,0,0,0,false,false,Translate,Select);
	}
#ifdef PJH_ADD_PANDA_CHANGERING
	else if(o->Kind == KIND_PLAYER && o->Type == MODEL_PLAYER && o->SubType == MODEL_PANDA)
	{
		RenderPartObject(&c->Object,o->SubType,NULL,c->Light,o->Alpha,0,0,0,false,false,Translate,Select);
	}
#endif //PJH_ADD_PANDA_CHANGERING
#ifdef YDG_ADD_SKELETON_CHANGE_RING
	else if(o->Kind == KIND_PLAYER && o->Type == MODEL_PLAYER && o->SubType == MODEL_SKELETON_CHANGED)	// 스켈레톤 변신반지
	{
		RenderPartObject(&c->Object,o->SubType,NULL,c->Light,o->Alpha,0,0,0,false,false,Translate,Select);
	}
#endif	// YDG_ADD_SKELETON_CHANGE_RING
	// 크리스마스 이벤트 변신반지 모델 렌더링
	else if(o->Kind == KIND_PLAYER && o->Type == MODEL_PLAYER && o->SubType == MODEL_XMAS_EVENT_CHANGE_GIRL)
	{
		RenderPartObject(&c->Object,o->SubType,NULL,c->Light,o->Alpha,0,0,0,false,false,Translate,Select);

		if(c->m_pTempParts == NULL)
		{
			c->m_pTempParts = (CSIPartsMDL*)new CSParts(MODEL_XMAS_EVENT_EARRING, 20);
			CSIPartsMDL* pParts = (CSIPartsMDL*) c->m_pTempParts;
			OBJECT* pObj = pParts->GetObject();
			pObj->Velocity = 0.25f;
			pObj->Owner = &c->Object;
		}
		else
		{
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
			CSIPartsMDL* pParts = (CSIPartsMDL*) c->m_pTempParts;
			OBJECT* pObj = pParts->GetObject();
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
			RenderParts ( c );
		}
	}
	// GM 변신반지 모델 렌더링
	else if(o->Kind == KIND_PLAYER && o->Type == MODEL_PLAYER && o->SubType == MODEL_GM_CHARACTER)
	{
		RenderPartObject(&c->Object,o->SubType,NULL,c->Light,o->Alpha,0,0,0,false,false,Translate,Select);

		if( !g_isCharacterBuff(o, eBuff_Cloaking) )
		{
			// 몸 가운데 이펙트
			vec3_t vLight, vPos;
			Vector(0.4f, 0.6f, 0.8f, vLight);
			VectorCopy(o->Position, vPos);
			vPos[2] += 100.f;
			CreateSprite(BITMAP_LIGHT, vPos, 6.0f, vLight, o, 0.5f);
			
			float fLumi;
			fLumi = sinf(WorldTime*0.05f)*0.4f+0.9f;
			Vector(fLumi*0.3f, fLumi*0.5f, fLumi*0.8f, vLight);
			CreateSprite(BITMAP_LIGHT, vPos, 2.0f, vLight, o);

			// 오로라 효과
			Vector ( 0.3f, 0.2f, 1.f, vLight );
			RenderAurora ( BITMAP_MAGIC+1, RENDER_BRIGHT, o->Position[0], o->Position[1], 2.5f, 2.5f, vLight );

			Vector ( 1.0f, 1.0f, 1.f, vLight );
			fLumi = sinf ( WorldTime*0.0015f )*0.3f+0.5f;
			EnableAlphaBlend ();
			Vector ( fLumi*vLight[0], fLumi*vLight[1], fLumi*vLight[2], vLight );
			RenderTerrainAlphaBitmap ( BITMAP_GM_AURORA, o->Position[0], o->Position[1], 1.5f, 1.5f, vLight, WorldTime*0.01f );
			RenderTerrainAlphaBitmap ( BITMAP_GM_AURORA, o->Position[0], o->Position[1], 1.f, 1.f, vLight, -WorldTime*0.01f );
			//RenderAurora ( BITMAP_GM_AURORA, RENDER_BRIGHT, o->Position[0], o->Position[1], 1.5f, 1.5f, vLight );

			if(c->Object.m_pCloth == NULL)	//3차날개
			{
				int iNumCloth = 2;

				CPhysicsCloth *pCloth = new CPhysicsCloth[iNumCloth];

				pCloth[0].Create( o, 20, 0.0f, 5.0f, 10.0f, 6, 5, 30.0f, 70.0f, BITMAP_GM_HAIR_1, BITMAP_GM_HAIR_1, PCT_COTTON | PCT_SHORT_SHOULDER | PCT_MASK_ALPHA);
				pCloth[0].SetWindMinMax( 10, 20 );
				pCloth[0].AddCollisionSphere( -10.f, 20.0f, 20.0f, 27.0f, 17);
				pCloth[0].AddCollisionSphere(  10.f, 20.0f, 20.0f, 27.0f, 17);

				pCloth[1].Create( o, 20, 0.0f, 5.0f, 25.0f, 4, 4, 30.0f, 40.0f, BITMAP_GM_HAIR_3, BITMAP_GM_HAIR_3, PCT_COTTON | PCT_SHORT_SHOULDER | PCT_MASK_ALPHA );
				pCloth[1].SetWindMinMax( 8, 15 );
				pCloth[1].AddCollisionSphere( -10.f, 20.0f, 20.0f, 27.0f, 17);
				pCloth[1].AddCollisionSphere(  10.f, 20.0f, 20.0f, 27.0f, 17);

				o->m_pCloth = ( void*)pCloth;
				o->m_byNumCloth = iNumCloth;
			}

			CPhysicsCloth *pCloth = (CPhysicsCloth*)c->Object.m_pCloth;

			if( g_isCharacterBuff(o, eBuff_Cloaking) )
			{
				for(int i=0; i<o->m_byNumCloth; ++i)
				{
					if(pCloth[i].Move2(0.005f, 5) == false)
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
	else if(o->Type == MODEL_PLAYER 
		&& (o->SubType == MODEL_XMAS_EVENT_CHA_SSANTA
		|| o->SubType == MODEL_XMAS_EVENT_CHA_SNOWMAN 
		|| o->SubType == MODEL_XMAS_EVENT_CHA_DEER) )
	{
		vec3_t vOriginPos;
		VectorCopy(o->Position, vOriginPos);
		o->Position[0] += 50.f;
		o->Position[1] += 50.f;
		o->Position[2] = RequestTerrainHeight(o->Position[0], o->Position[1]);
		RenderPartObject(&c->Object,o->SubType,NULL,c->Light,o->Alpha,0,0,0,false,false,Translate,Select);

		VectorCopy(vOriginPos, o->Position);
		o->Position[0] -= 50.f;
		o->Position[1] -= 50.f;
		o->Position[2] = RequestTerrainHeight(o->Position[0], o->Position[1]);
		RenderPartObject(&c->Object,o->SubType,NULL,c->Light,o->Alpha,0,0,0,false,false,Translate,Select);

		VectorCopy(vOriginPos, o->Position);
		o->Position[0] += 50.f;
		o->Position[1] -= 50.f;
		o->Position[2] = RequestTerrainHeight(o->Position[0], o->Position[1]);
		RenderPartObject(&c->Object,o->SubType,NULL,c->Light,o->Alpha,0,0,0,false,false,Translate,Select);
	}
	// 몬스터 해골전사
	else if(o->Type==MODEL_PLAYER && (o->SubType>=MODEL_SKELETON1&&o->SubType<=MODEL_SKELETON3))//해골씨리즈
	{
        RenderPartObject(&c->Object,o->SubType,NULL,c->Light,o->Alpha,0,0,0,false,false,Translate,Select);
	}
	// 캐릭터가 변신하지 않았다면
	else if(!c->Change)
	{
#ifndef GUILD_WAR_EVENT
        if ( (o->Kind==KIND_PLAYER && InChaosCastle()==true)
#ifdef _PVP_ATTACK_GUARD
			|| ( o->Kind==KIND_NPC && (c->MonsterIndex == 247 || c->MonsterIndex == 249) )
#endif	// _PVP_ATTACK_GUARD
			)
        {
            int RenderType = RENDER_TEXTURE;
			PART_t* p = &c->BodyPart[BODYPART_HEAD];
            if ( c==Hero )
            {
                RenderType |= RENDER_CHROME;
            }
#ifdef _PVP_ATTACK_GUARD
			else if ( o->Kind==KIND_NPC && (c->MonsterIndex == 247 || c->MonsterIndex == 249) )
			{
//                    RenderType |= RENDER_CHROME;
			}
#endif	// _PVP_ATTACK_GUARD
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
			if((o->CurrentAction == PLAYER_SKILL_DRAGONKICK) && (GetBaseClass(c->Class) == CLASS_RAGEFIGHTER))
			{
				p->Type = MODEL_ANGEL;
				RenderCharacter_AfterImage(c, p, Translate, Select, 2.5f, 1.0f);
			}
			else if((GetBaseClass(c->Class) == CLASS_RAGEFIGHTER))
			{
				OBJECT* pObj = &c->Object;
				if(pObj->m_sTargetIndex<0 || c->JumpTime > 0)
				{
					RenderPartObject ( &c->Object, MODEL_ANGEL, p, c->Light, o->Alpha, 0, 0, 0, false, false, Translate, Select, RenderType );
				}
				else
				{
					p->Type = MODEL_ANGEL;
					g_CMonkSystem.DarksideRendering(c, p, Translate, Select);
				}
			}
			else
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
			RenderPartObject ( &c->Object, MODEL_ANGEL, p, c->Light, o->Alpha, 0, 0, 0, false, false, Translate, Select, RenderType );
        }
        else
#endif// GUILD_WAR_EVENT
        {
#ifdef PJH_NEW_CHROME
			int Set_Count = 0; //세트 아이템 카운트.
#endif //PJH_NEW_CHROME
			for(int i=MAX_BODYPART-1; i>=0; i--)
			{
				PART_t *p = &c->BodyPart[i];
				if(p->Type != -1)
				{
					int Type = p->Type;


#ifdef PJH_NEW_CHROME
					int nItemType_n = (Type - MODEL_ITEM) / MAX_ITEM_INDEX;
					int nItemSubType_n = (Type - MODEL_ITEM) % MAX_ITEM_INDEX;

					if (nItemType_n >= 7 && nItemType_n <= 11 && (nItemSubType_n >= 62 && nItemSubType_n <= 72))
					{
						Set_Count = 1;
					}
#endif //PJH_NEW_CHROME

					if (CLASS_SUMMONER == GetBaseClass(c->Class))
					{
						int nItemType = (Type - MODEL_ITEM) / MAX_ITEM_INDEX;
						int nItemSubType = (Type - MODEL_ITEM) % MAX_ITEM_INDEX;
						// 넝쿨, 실크 시리즈인가?
						if (nItemType >= 7 && nItemType <= 11
							&& (nItemSubType == 10 || nItemSubType == 11))
						{
							Type = MODEL_HELM2
								+ (nItemType - 7) * MODEL_ITEM_COMMON_NUM
								+ nItemSubType - 10;
						}
					}
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
					else if(CLASS_RAGEFIGHTER == GetBaseClass(c->Class))
						Type = g_CMonkSystem.ModifyTypeCommonItemMonk(Type);
#endif //PBG_ADD_NEWCHAR_MONK_ITEM

					BMD *b = &Models[Type];
#ifdef PBG_ADD_NEWCHAR_MONK
					if(CLASS_RAGEFIGHTER == GetBaseClass(c->Class))
					{
						b->Skin = GetBaseClass(c->Class)*2 + IsThirdClass(c->Class);
					}
					else
#endif //PBG_ADD_NEWCHAR_MONK
					b->Skin = GetBaseClass(c->Class)*2 + IsSecondClass(c->Class);

                    if ( GetBaseClass(c->Class)==CLASS_DARK_LORD && i==BODYPART_HELM )
                    {
                        o->BlendMeshLight = sinf ( WorldTime*0.001f )*0.1f+0.7f;
                        if ( i==BODYPART_HELM )
                        {
                            int index = Type-MODEL_HELM;
                            if ( index==0 || index==5 || index==6 || index==8 || index==9 )
                            {
                                Type = MODEL_MASK_HELM+index;
                                Models[Type].Skin = b->Skin;
                            }
                        }
                    }

#ifdef YDG_ADD_DOPPELGANGER_MONSTER
					// 도플갱어 몸통
					if (c->MonsterIndex >= 529 && c->MonsterIndex <= 539)
					{
						if (World == WD_65DOPPLEGANGER1)
							RenderPartObject(&c->Object,Type,p,c->Light,o->Alpha,p->Level<<3,p->Option1,p->ExtOption,false,false,Translate,
								Select,RENDER_DOPPELGANGER|RENDER_TEXTURE);
						else
							RenderPartObject(&c->Object,Type,p,c->Light,o->Alpha,p->Level<<3,p->Option1,p->ExtOption,false,false,Translate,
								Select,RENDER_DOPPELGANGER|RENDER_BRIGHT|RENDER_TEXTURE);
// 						RenderPartObject(&c->Object,Type,p,c->Light,o->Alpha,p->Level<<3,p->Option1,p->ExtOption,false,false,Translate,
// 							Select,RENDER_DOPPELGANGER|RENDER_BRIGHT|RENDER_CHROME);
					}
					else
#endif	// YDG_ADD_DOPPELGANGER_MONSTER
					{
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
						if((o->CurrentAction == PLAYER_SKILL_DRAGONKICK) && (GetBaseClass(c->Class) == CLASS_RAGEFIGHTER))
						{
							RenderCharacter_AfterImage(c, p, Translate, Select, 2.5f, 1.0f);
						}
						else
						if((o->CurrentAction == PLAYER_SKILL_DARKSIDE_READY) && (GetBaseClass(c->Class) == CLASS_RAGEFIGHTER))
						{
#ifdef PBG_MOD_RAGEFIGHTERSOUND
							if(o->m_sTargetIndex<0 || c->JumpTime>0)
							{
								RenderPartObject(&c->Object,Type,p,c->Light,o->Alpha,p->Level<<3,p->Option1,p->ExtOption,false,false,Translate,Select);
							}
							else
#endif //PBG_MOD_RAGEFIGHTERSOUND
							g_CMonkSystem.DarksideRendering(c, p, Translate, Select);
						}
						else
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
#ifdef PJH_NEW_CHROME
						if(Set_Count > 0)
						{
							RenderPartObject(&c->Object,Type,p,c->Light,o->Alpha,p->Level<<3,p->Option1,MAX_MODELS+1,false,false,Translate,Select);
//							RenderPartObject(&c->Object,Type,p,c->Light,o->Alpha,p->Level<<3,p->Option1,p->ExtOption,false,false,Translate,Select);
							Set_Count = 0;
						}
						else
#endif //PJH_NEW_CHROME
						RenderPartObject(&c->Object,Type,p,c->Light,o->Alpha,p->Level<<3,p->Option1,p->ExtOption,false,false,Translate,Select);

					}
				}
			}
        }
#ifndef GUILD_WAR_EVENT
        if ( InChaosCastle()==false )
#endif //GUILD_WAR_EVENT
        {
			if(c->GuildMarkIndex>=0 && o->Type==MODEL_PLAYER && o->Alpha != 0.0f
				&& (c->GuildMarkIndex == MARK_EDIT || g_GuildCache.IsExistGuildMark(GuildMark[c->GuildMarkIndex].Key) == TRUE)
				&& ( !g_isCharacterBuff(o, eBuff_Cloaking) ) )
			{
				CreateGuildMark(c->GuildMarkIndex);
#ifdef PBG_ADD_NEWCHAR_MONK
				if(GetBaseClass(c->Class) == CLASS_RAGEFIGHTER)
				{	// 모델 사이즈로 잡기에는 퀄리티가 떨어짐
					vec3_t vPos;
					if(c->BodyPart[BODYPART_ARMOR].Type == MODEL_ARMOR+59
						|| c->BodyPart[BODYPART_ARMOR].Type == MODEL_ARMOR+60
						|| c->BodyPart[BODYPART_ARMOR].Type == MODEL_ARMOR+61)
					{
						Vector(5.0f, 0.0f, -35.0f, vPos);
					}
					else
					{
						Vector(5.0f, 0.0f, -21.0f, vPos);
					}
					RenderGuild(o,c->BodyPart[BODYPART_ARMOR].Type, vPos);
				}
				else
#endif //PBG_ADD_NEWCHAR_MONK
				RenderGuild(o,c->BodyPart[BODYPART_ARMOR].Type);
			}
        }
//#endif
	}

	if ( bCloak )
	{
		if ( !c->Object.m_pCloth)
		{
            if ( GetBaseClass(c->Class)==CLASS_DARK_LORD )
            {
				int numCloth = 4;
				if (c->Wing.Type == MODEL_WING+40)	// 제왕의 망토
				{
					numCloth = 6;
				}
				else	// 군주의 망토
				{
					numCloth = 4;
				}

				
				// 1. 통물리 모델 선언
				CPhysicsCloth *pCloth = new CPhysicsCloth[numCloth];
				
				// 머리카락
				pCloth[0].Create( o, 20, 0.0f, 0.0f, 20.0f, 6, 5, 30.0f, 70.0f, BITMAP_ROBE+6, BITMAP_ROBE+6, PCT_CURVED | PCT_RUBBER2 | PCT_MASK_LIGHT | PLS_STRICTDISTANCE | PCT_SHORT_SHOULDER | PCT_NORMAL_THICKNESS | PCT_OPT_HAIR );
				pCloth[0].SetWindMinMax( 10, 50 );
				pCloth[0].AddCollisionSphere( -10.f, 20.0f, 20.0f, 27.0f, 17);
				pCloth[0].AddCollisionSphere(  10.f, 20.0f, 20.0f, 27.0f, 17);

				pCloth[1].Create( o, 20, 0.0f, 5.0f, 18.0f, 5, 5, 30.0f, 70.0f, BITMAP_ROBE+6, BITMAP_ROBE+6, PCT_CURVED | PCT_RUBBER2 | PCT_MASK_BLEND | PLS_STRICTDISTANCE | PCT_SHORT_SHOULDER | PCT_NORMAL_THICKNESS | PCT_OPT_HAIR );
				pCloth[1].SetWindMinMax( 8, 40 );
				pCloth[1].AddCollisionSphere( -10.f, 20.0f, 20.0f, 27.0f, 17);
				pCloth[1].AddCollisionSphere(  10.f, 20.0f, 20.0f, 27.0f, 17);

				if (c->Wing.Type == MODEL_WING+40)	// 제왕의 망토
				{
					pCloth[2].Create( o, 19, 0.0f, 8.0f, 10.0f, 10, 10, 180.0f, 180.0f, BITMAP_ROBE+9, BITMAP_ROBE+9, PCT_CURVED | PCT_SHORT_SHOULDER | PCT_HEAVY | PCT_MASK_ALPHA );
					pCloth[2].AddCollisionSphere( -10.f, -10.0f, -10.0f, 25.0f, 17);
					pCloth[2].AddCollisionSphere( 10.f, -10.0f, -10.0f, 25.0f, 17);
					pCloth[2].AddCollisionSphere( -10.f, -10.0f, 20.0f, 27.0f, 17);
					pCloth[2].AddCollisionSphere( 10.f, -10.0f, 20.0f, 27.0f, 17);
				}
#ifdef LDK_ADD_INGAMESHOP_SMALL_WING
				else if(c->Wing.Type == MODEL_WING+130)
				{
					pCloth[2].Create( o, 19, 0.0f, 8.0f, 10.0f, 10, 10, 100.0f, 100.0f, BITMAP_ROBE+7, BITMAP_ROBE+7, PCT_CURVED | PCT_SHORT_SHOULDER | PCT_MASK_ALPHA );
					pCloth[2].AddCollisionSphere( -10.f, -10.0f, -10.0f, 25.0f, 17);
					pCloth[2].AddCollisionSphere( 10.f, -10.0f, -10.0f, 25.0f, 17);
					pCloth[2].AddCollisionSphere( -10.f, -10.0f, 20.0f, 27.0f, 17);
					pCloth[2].AddCollisionSphere( 10.f, -10.0f, 20.0f, 27.0f, 17);
				}
#endif //LDK_ADD_INGAMESHOP_SMALL_WING
				else
				{
					// 
					pCloth[2].Create( o, 19, 0.0f, 8.0f, 10.0f, 10, 10, 180.0f, 180.0f, BITMAP_ROBE+7, BITMAP_ROBE+7, PCT_CURVED | PCT_SHORT_SHOULDER | PCT_MASK_ALPHA );
					pCloth[2].AddCollisionSphere( -10.f, -10.0f, -10.0f, 25.0f, 17);
					pCloth[2].AddCollisionSphere( 10.f, -10.0f, -10.0f, 25.0f, 17);
					pCloth[2].AddCollisionSphere( -10.f, -10.0f, 20.0f, 27.0f, 17);
					pCloth[2].AddCollisionSphere( 10.f, -10.0f, 20.0f, 27.0f, 17);
				}

				if (c->Wing.Type == MODEL_WING+40)	// 제왕의 망토
				{
					numCloth = 6;
				}
				else	// 군주의 망토
				{
					numCloth = 3;
				}
				
				// 기본 옷 꼬리
				if ( c->BodyPart[BODYPART_ARMOR].Type==MODEL_BODY_ARMOR+CLASS_DARK_LORD )
				{
					pCloth[3].Create( o, 18, 0.0f, 10.0f, -5.0f, 5, 5, 50.0f, 90.0f, BITMAP_DARK_LOAD_SKIRT, BITMAP_DARK_LOAD_SKIRT, PCT_MASK_ALPHA | PCT_HEAVY | PCT_STICKED | PCT_SHORT_SHOULDER );
					pCloth[3].AddCollisionSphere( 0.0f, -15.0f, -20.0f, 30.0f, 2);
				}
				else if(c->BodyPart[BODYPART_ARMOR].Type==MODEL_BODY_ARMOR+14)	// 3차 전직 다크로드
				{
					pCloth[3].Create( o, 18, 0.0f, 10.0f, -5.0f, 5, 5, 50.0f, 90.0f, BITMAP_DARKLOAD_SKIRT_3RD, BITMAP_DARKLOAD_SKIRT_3RD, PCT_MASK_ALPHA | PCT_HEAVY | PCT_STICKED | PCT_SHORT_SHOULDER );
					pCloth[3].AddCollisionSphere( 0.0f, -15.0f, -20.0f, 30.0f, 2);
				}

				if (c->Wing.Type == MODEL_WING+40)	// 제왕의 망토 꼬리 2개
				{
					pCloth[4].Create( o, 19, 30.0f, 15.0f, 10.0f, 2, 5, 12.0f, 200.0f, BITMAP_ROBE+10, BITMAP_ROBE+10, PCT_FLAT | PCT_SHAPE_NORMAL | PCT_COTTON | PCT_MASK_ALPHA );
					pCloth[4].AddCollisionSphere( 0.0f, -15.0f, -20.0f, 30.0f, 2);
					pCloth[4].AddCollisionSphere( 0.f, 0.0f, 0.0f, 35.0f, 17);

					pCloth[5].Create( o, 19, -30.0f, 20.0f, 10.0f, 2, 5, 12.0f, 200.0f, BITMAP_ROBE+10, BITMAP_ROBE+10, PCT_FLAT | PCT_SHAPE_NORMAL | PCT_COTTON | PCT_MASK_ALPHA );
					pCloth[5].AddCollisionSphere( 0.0f, -15.0f, -20.0f, 30.0f, 2);
					pCloth[5].AddCollisionSphere( 0.f, 0.0f, 0.0f, 35.0f, 17);
				}

				o->m_pCloth = ( void*)pCloth;
				o->m_byNumCloth = numCloth;
            }
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
			else if(GetBaseClass(c->Class)==CLASS_RAGEFIGHTER)
			{
				int numCloth = (c->Wing.Type == MODEL_WING+50) ? 3 : 1;
				
				CPhysicsCloth *pCloth = new CPhysicsCloth[numCloth];
				
				if(c->Wing.Type == MODEL_WING+50)
				{
					pCloth[0].Create(o, 19, 0.0f, 15.0f, 5.0f, 10, 10, 180.0f, 170.0f, BITMAP_MANTOE, BITMAP_MANTOE, PCT_CURVED | PCT_SHORT_SHOULDER | PCT_HEAVY | PCT_MASK_ALPHA);
					pCloth[0].AddCollisionSphere(-10.f, -10.0f, -10.0f, 35.0f, 17);
					pCloth[0].AddCollisionSphere(10.f, -10.0f, -10.0f, 35.0f, 17);
					pCloth[0].AddCollisionSphere(-10.f, -10.0f, 20.0f, 37.0f, 17);
					pCloth[0].AddCollisionSphere(10.f, -10.0f, 20.0f, 37.0f, 17);
				}
				else if(c->Wing.Type == MODEL_WING+135)
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

				if (c->Wing.Type == MODEL_WING+50)
				{
					pCloth[1].Create(o, 19, 25.0f, 15.0f, 2.0f, 2, 5, 12.0f, 180.0f, BITMAP_MANTO01, BITMAP_MANTO01, PCT_FLAT | PCT_SHAPE_NORMAL | PCT_COTTON | PCT_ELASTIC_RAGE_L | PCT_MASK_ALPHA);
					pCloth[1].AddCollisionSphere(0.0f, -15.0f, -20.0f, 35.0f, 2);
					pCloth[1].AddCollisionSphere(0.0f, 0.0f, 0.0f, 45.0f, 17);
					
					pCloth[2].Create(o, 19,-25.0f, 15.0f, 2.0f, 2, 5, 12.0f, 180.0f, BITMAP_MANTO01, BITMAP_MANTO01, PCT_FLAT | PCT_SHAPE_NORMAL | PCT_COTTON | PCT_ELASTIC_RAGE_R | PCT_MASK_ALPHA);
					pCloth[2].AddCollisionSphere(0.0f, -15.0f, -20.0f, 35.0f, 2);
					pCloth[2].AddCollisionSphere(0.0f, 0.0f, 0.0f, 50.0f, 17);
				}

				o->m_pCloth = (void*)pCloth;
				o->m_byNumCloth = numCloth;
			}
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
            else
            {
				CPhysicsCloth *pCloth = new CPhysicsCloth [1];

				if (c->MonsterIndex==55)
				{	// 데쓰킹
					pCloth[0].Create( o, 19, 0.0f, 10.0f, 0.0f, 10, 10, 55.0f, 140.0f, BITMAP_ROBE+2,BITMAP_ROBE+2, PCT_CURVED | PCT_MASK_ALPHA);
					pCloth[0].AddCollisionSphere( -10.f, -10.0f, -10.0f, 35.0f, 17);
					pCloth[0].AddCollisionSphere( 10.f, -10.0f, -10.0f, 35.0f, 17);
					pCloth[0].AddCollisionSphere( 0.f, -10.0f, -40.0f, 50.0f, 19);
				}
				else if (c->MonsterIndex==361)
				{
					pCloth[0].Create( o, 2, 0.0f, 0.0f, 0.0f, 6, 6, 180.0f, 100.0f, BITMAP_NIGHTMARE_ROBE,BITMAP_NIGHTMARE_ROBE, PCT_CYLINDER | PCT_MASK_ALPHA);
				}
                else
				{	// 마검
					if (c->Wing.Type == MODEL_WING+39)
					{
						pCloth[0].Create( o, 19, 0.0f, 15.0f, 0.0f, 10, 10, 120.0f, 120.0f, BITMAP_ROBE+8, BITMAP_ROBE+8, PCT_CURVED | PCT_SHORT_SHOULDER | PCT_HEAVY | PCT_MASK_ALPHA);	
					}
					else
					{
						pCloth[0].Create( o, 19, 0.0f, 10.0f, 0.0f, 10, 10, /*45.0f*/75.0f, 120.0f, BITMAP_ROBE, BITMAP_ROBE+1, PCT_CURVED | PCT_SHORT_SHOULDER | PCT_HEAVY);
					}
				}
				o->m_pCloth = ( void*)pCloth;
				o->m_byNumCloth = 1;
            }
		}
        else if ( GetBaseClass(c->Class)==CLASS_DARK_LORD )
        {
            if ( (c->BodyPart[BODYPART_ARMOR].Type != MODEL_BODY_ARMOR+CLASS_DARK_LORD
				 && c->BodyPart[BODYPART_ARMOR].Type != MODEL_BODY_ARMOR+14	
				) && o->m_byNumCloth==4 )
            {
	            if ( o && o->m_pCloth )
	            {
		            CPhysicsCloth* pCloth = ( CPhysicsCloth*)o->m_pCloth;
			        pCloth[3].Destroy();

                    o->m_byNumCloth = 3;
	            }
            }
			else if ( c->BodyPart[BODYPART_ARMOR].Type==MODEL_BODY_ARMOR+CLASS_DARK_LORD && o->m_byNumCloth==3 )
            {
	            if ( o && o->m_pCloth )
	            {
		            CPhysicsCloth* pCloth = ( CPhysicsCloth*)o->m_pCloth;

					pCloth[3].Create( o, 18, 0.0f, 8.0f, -5.0f, 5, 5, 50.0f, 90.0f, BITMAP_DARK_LOAD_SKIRT, BITMAP_DARK_LOAD_SKIRT, PCT_MASK_ALPHA | PCT_HEAVY | PCT_STICKED | PCT_CURVED | PCT_SHORT_SHOULDER );
				    pCloth[3].AddCollisionSphere( 0.0f, -15.0f, -20.0f, 30.0f, 2);
                    o->m_byNumCloth = 4;
                }
            }
			else if ( c->BodyPart[BODYPART_ARMOR].Type==MODEL_BODY_ARMOR+14 && o->m_byNumCloth==3 )	// 3차 전직 다크로드
            {
	            if ( o && o->m_pCloth )
	            {
		            CPhysicsCloth* pCloth = ( CPhysicsCloth*)o->m_pCloth;

					pCloth[3].Create( o, 18, 0.0f, 8.0f, -5.0f, 5, 5, 50.0f, 90.0f, BITMAP_DARKLOAD_SKIRT_3RD, BITMAP_DARKLOAD_SKIRT_3RD, PCT_MASK_ALPHA | PCT_HEAVY | PCT_STICKED | PCT_CURVED | PCT_SHORT_SHOULDER );
				    pCloth[3].AddCollisionSphere( 0.0f, -15.0f, -20.0f, 30.0f, 2);
                    o->m_byNumCloth = 4;
                }
            }
            else if ( c->BodyPart[BODYPART_ARMOR].Type!=MODEL_BODY_ARMOR+CLASS_DARK_LORD && o->m_byNumCloth==6 )
            {
	            if ( o && o->m_pCloth )
	            {
		            CPhysicsCloth* pCloth = ( CPhysicsCloth*)o->m_pCloth;
			        if (pCloth[3].GetOwner() != NULL)
						pCloth[3].Destroy();
	            }
            }
            else if ( c->BodyPart[BODYPART_ARMOR].Type==MODEL_BODY_ARMOR+CLASS_DARK_LORD && o->m_byNumCloth==6 )
            {
	            if ( o && o->m_pCloth )
	            {
		            CPhysicsCloth* pCloth = ( CPhysicsCloth*)o->m_pCloth;
			        if (pCloth[3].GetOwner() == NULL)
					{
						pCloth[3].Create( o, 18, 0.0f, 8.0f, -5.0f, 5, 5, 50.0f, 90.0f, BITMAP_DARK_LOAD_SKIRT, BITMAP_DARK_LOAD_SKIRT, PCT_MASK_ALPHA | PCT_HEAVY | PCT_STICKED | PCT_CURVED | PCT_SHORT_SHOULDER );
						pCloth[3].AddCollisionSphere( 0.0f, -15.0f, -20.0f, 30.0f, 2);
					}
                }
            }
			else if ( c->BodyPart[BODYPART_ARMOR].Type==MODEL_BODY_ARMOR+14 && o->m_byNumCloth==6 )
            {
	            if ( o && o->m_pCloth )
	            {
		            CPhysicsCloth* pCloth = ( CPhysicsCloth*)o->m_pCloth;
			        if (pCloth[3].GetOwner() == NULL)
					{
						pCloth[3].Create( o, 18, 0.0f, 8.0f, -5.0f, 5, 5, 50.0f, 90.0f, BITMAP_DARKLOAD_SKIRT_3RD, BITMAP_DARKLOAD_SKIRT_3RD, PCT_MASK_ALPHA | PCT_HEAVY | PCT_STICKED | PCT_CURVED | PCT_SHORT_SHOULDER );
						pCloth[3].AddCollisionSphere( 0.0f, -15.0f, -20.0f, 30.0f, 2);
					}
                }
            }
        }
        CPhysicsCloth *pCloth = ( CPhysicsCloth*)c->Object.m_pCloth;

        //  투명 마법시에는 모든 물리적 효과를 나타내지 않는다.
		if( !g_isCharacterBuff(o, eBuff_Cloaking) )
        {
            for ( int i=0; i<o->m_byNumCloth; i++ )
            {
                if ( GetBaseClass( c->Class )==CLASS_DARK_LORD )
                {
                    if ( i==2 && (
						(c->Wing.Type!=MODEL_HELPER+30	// 군주의 망토
						&& c->Wing.Type!=MODEL_WING+40	// 다크로드 제왕의 망토
#ifdef LDK_ADD_INGAMESHOP_SMALL_WING
						&& c->Wing.Type!=MODEL_WING+130	// 작은 군주의 망토
#endif //LDK_ADD_INGAMESHOP_SMALL_WING
						) && ( CloakLight[0]==1.f && CloakLight[1]==1.f && CloakLight[2]==1.f ) ) ) 
					{
						continue;
					}
					// 변신반지 착용하면 다크로드 머리 안보이게 수정
					if((i >= 0 && i <= 1) && g_ChangeRingMgr->CheckDarkLordHair(c->Object.SubType) == true)
					{
						continue;
					}

#ifdef YDG_FIX_PANDA_CHANGERING_PANTS_BUG
					// 다크로드 변신반지 착용하면 바지 안그리게 수정
					if(i == 3 && g_ChangeRingMgr->CheckDarkLordHair(c->Object.SubType) == true)
					{
						continue;
					}
#endif	// YDG_FIX_PANDA_CHANGERING_PANTS_BUG
                }
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
				if(GetBaseClass( c->Class )==CLASS_RAGEFIGHTER)
				{
					if(i==0 && ((c->Wing.Type!=MODEL_WING+49
						&& c->Wing.Type!=MODEL_WING+50
						&& c->Wing.Type!=MODEL_WING+135) 
						&& (CloakLight[0]==1.f && CloakLight[1]==1.f && CloakLight[2]==1.f)))
					{
						continue;
					}
				}
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
				// 변신반지 착용하면 마검사 망토 안보이게 예외 처리
				if(i == 0 && g_ChangeRingMgr->CheckDarkCloak(c->Class, c->Object.SubType) == true)
				{
					continue;
				}

				if(c->Object.SubType == MODEL_CURSEDTEMPLE_ALLIED_PLAYER || c->Object.SubType == MODEL_CURSEDTEMPLE_ILLUSION_PLAYER)
				{
					continue;
				}

				float Flag = 0.005f;
				if( g_isCharacterBuff(o, eDeBuff_Stun ) 
					|| g_isCharacterBuff(o, eDeBuff_Sleep ) )
				{
					Flag = 0.0f;
				}

				if ( pCloth[i].GetOwner() == NULL)	// 지워진 부분이면 그리지 않고 통과한다.
				{
					continue;
				}
				else
                if ( !pCloth[i].Move2( Flag, 5))
                {
                    DeleteCloth( c, o);
                }
                else
                {
					pCloth[i].Render( &CloakLight);
                }
            }
        } // if( !g_isCharacterBuff(o, eBuff_Cloaking) )
	} //if ( bCloak )

	float Luminosity = (float)(rand()%30+70)*0.01f;
	if(c->PK >= PVP_MURDERER2)
	{
		Vector(1.f,0.1f,0.1f,c->Light);
	}
	else
	{
		VectorAdd(Light,o->Light,c->Light);

        //  블러드캐슬의 몬스터의 레벨에 따른 피부색 처리.
		int nCastle = BLOODCASTLE_NUM + (World - WD_11BLOODCASTLE_END );
        if(nCastle > 0 && nCastle <= BLOODCASTLE_NUM )		//. 블러드 캐슬일경우
		{
			if( (c->MonsterIndex >= 86 && c->MonsterIndex <= 89) ||
				(c->MonsterIndex >= 92 && c->MonsterIndex <= 95) ||
				(c->MonsterIndex == 99 && c->MonsterIndex == 111 && c->MonsterIndex == 112) ||
				(c->MonsterIndex >= 115 && c->MonsterIndex <= 118) ||
				(c->MonsterIndex >= 121 && c->MonsterIndex <= 124) ||
				(c->MonsterIndex >= 127 && c->MonsterIndex <= 130) ||
                (c->MonsterIndex >= 138 && c->MonsterIndex <=143 ) )
			{
				int level = nCastle/3;
				if(level)
					Vector(level*0.5f,0.1f,0.1f,c->Light);	
			}
		}
	}
	
#ifdef LDK_ADD_14_15_GRADE_ITEM_MODEL
	// +15 무기
	if(!InChaosCastle() && 
#ifdef LJH_FIX_NO_EFFECT_ON_WEAPONS_IN_SAFE_ZONE_OF_CURSED_TEMPLE
		//환영사원의 안전지역에서는 15레벨 아이템 이펙트 나와야 함
		!(g_CursedTemple->IsCursedTemple() && !c->SafeZone) 
#else  //LJH_FIX_NO_EFFECT_ON_WEAPONS_IN_SAFE_ZONE_OF_CURSED_TEMPLE
		!g_CursedTemple->IsCursedTemple() 
#endif //LJH_FIX_NO_EFFECT_ON_WEAPONS_IN_SAFE_ZONE_OF_CURSED_TEMPLE
	)
	{
		NextGradeObjectRender(c);
	}
#endif //LDK_ADD_14_15_GRADE_ITEM_MODEL

	//  등에 달라붙는 아이템 렌더링
#ifdef CSK_REF_BACK_RENDERITEM
	bool Bind = false;
	Bind = RenderCharacterBackItem(c, o, Translate);
#else // CSK_REF_BACK_RENDERITEM
	int Bind = false;
	if ( c->SafeZone || (o->CurrentAction>=PLAYER_GREETING1 && o->CurrentAction<=PLAYER_SALUTE1) 
                     || ( ( World==WD_7ATLANSE || InHellas() 
#ifdef YDG_ADD_MAP_DOPPELGANGER3
						|| World == WD_67DOPPLEGANGER3
#endif	// YDG_ADD_MAP_DOPPELGANGER3
					 ) 
					 && (o->CurrentAction==PLAYER_WALK_SWIM || o->CurrentAction==PLAYER_RUN_SWIM ) ) 
       )
		Bind = true;
    //  블러드캐슬에서는 무기를 등에 달고 다니지 않는다.
    if ( InBloodCastle() == true )
    {
        Bind = false;
    }
    if ( InChaosCastle() == true )
    {
        Bind = false;
    }
	if(o->Type == MODEL_PLAYER)
	{
		int     Hand = 0;
        bool    Back = false;   //  등.
		if(Bind)
        {
            //  등에 붙는다.
            Back = true;

#ifdef ADD_SOCKET_ITEM
			// 활
			if( GetEquipedBowType( c ) == BOWTYPE_BOW )
			{
				Hand = 1;
			}
			// 석궁
			else if( GetEquipedBowType( c ) == BOWTYPE_CROSSBOW )
			{
				Hand = 0;
			}
#else // ADD_SOCKET_ITEM				// 정리할 때 지워야 하는 소스
            if(c->Weapon[1].Type>=MODEL_BOW && c->Weapon[1].Type<MODEL_BOW+7 || c->Weapon[1].Type==MODEL_BOW+17
				|| c->Weapon[1].Type==MODEL_BOW+20 
				|| c->Weapon[1].Type == MODEL_BOW+21
				|| c->Weapon[1].Type == MODEL_BOW+22
				)
			{
		    	Hand = 1;
			}
            else if((c->Weapon[0].Type>=MODEL_BOW+8 && c->Weapon[0].Type<MODEL_BOW+15) ||
			        (c->Weapon[0].Type>=MODEL_BOW+16 && c->Weapon[0].Type<MODEL_BOW+17)||
					(c->Weapon[0].Type>=MODEL_BOW+18 && c->Weapon[0].Type<MODEL_BOW+MAX_ITEM_INDEX) )
			{
		    	Hand = 0;
			}
#endif // ADD_SOCKET_ITEM				// 정리할 때 지워야 하는 소스
            else if(c->Weapon[1].Type==MODEL_BOW+7)
			{
		     	Hand = 1;
			}
        }
		else 
        {
            //  석궁용 화살.
            if(c->Weapon[1].Type==MODEL_BOW+7)
            {
                Back = true;
			    Hand = 1;
            }
            //  화살.
            else if ( c->Weapon[0].Type==MODEL_BOW+15 )
            {
                Back = true;
            }
        }
		int Type    = c->Weapon[Hand].Type;
		int Level   = c->Weapon[Hand].Level;
		int Option1 = c->Weapon[Hand].Option1;
        if(o->Kind==KIND_NPC && World==WD_0LORENCIA && o->Type==MODEL_PLAYER && (o->SubType>=MODEL_SKELETON1&&o->SubType<=MODEL_SKELETON3) )
        {
            Back = true;
            Type = MODEL_BOW+9;
			Level = 8;
        }
		
		if(Back && Type != -1)
		{
			PART_t *w = &c->Wing;
			
            float fAnimationFrameBackUp = w->AnimationFrame;

			w->LinkBone  = 47;		
			if(o->CurrentAction == PLAYER_FLY || o->CurrentAction == PLAYER_FLY_CROSSBOW)
				w->PlaySpeed = 1.f;
			else
				w->PlaySpeed = 0.25f;

			bool bLink  = false;
			PART_t t_Part;	// 백업용 변수
			// 무기류 타입이면 링크 true
			if(Type >= MODEL_SWORD && Type < MODEL_SHIELD+MAX_ITEM_INDEX)
			{
				bLink = true;

				::memcpy(&t_Part, w, sizeof(PART_t));
				t_Part.CurrentAction = 0;
				t_Part.AnimationFrame = 0.f;
				t_Part.PlaySpeed = 0.f;
				t_Part.PriorAction = 0;
				t_Part.PriorAnimationFrame = 0.f;
			}

			if(bLink == true)
			{
#ifdef ADD_SOCKET_ITEM	
				if( Type == MODEL_BOW+23 )
				{
					// 다크스팅거(23)는 등에 link가 되었을 때 에도 애니메이션이 있습니다.
					PART_t *pWeapon = &c->Weapon[1];
					BYTE byTempLinkBone = pWeapon->LinkBone;
					pWeapon->CurrentAction = 2;
					pWeapon->PlaySpeed = 0.25f;
					pWeapon->LinkBone = 47;
					RenderLinkObject(0.f,0.f,15.f,c, pWeapon, Type, Level, Option1, bLink, Translate);
					pWeapon->LinkBone = byTempLinkBone;
				}
				else
				{
					RenderLinkObject(0.f,0.f,15.f,c,&t_Part,Type,Level,Option1,bLink,Translate);
				}
#else // ADD_SOCKET_ITEM
				RenderLinkObject(0.f,0.f,15.f,c,&t_Part,Type,Level,Option1,bLink,Translate);
#endif // ADD_SOCKET_ITEM
			}
			else
			{
				RenderLinkObject(0.f,0.f,15.f,c,w,Type,Level,Option1,bLink,Translate);
			}

            w->AnimationFrame = fAnimationFrameBackUp;
        }

        //  블러드캐슬의 얻은 절대무기를 등에 표현한다.
        if ( InBloodCastle() && c->EtcPart!=0 )
        {
			PART_t *w = &c->Wing;

			Level   = 0;
			Option1 = 0;

			w->LinkBone  = 47;
			if(o->CurrentAction==PLAYER_FLY||o->CurrentAction==PLAYER_FLY_CROSSBOW)
				w->PlaySpeed = 1.f;
			else
				w->PlaySpeed = 0.25f;
			bool Link  = false;
            switch ( c->EtcPart )
            {
#ifdef LJW_FIX_PARTS_ENUM
            case PARTS_STAFF: Type = MODEL_STAFF+10; break;
            case PARTS_SWORD: Type = MODEL_SWORD+19; break;
            case PARTS_BOW  : Type = MODEL_BOW+18; break;
#else 
            case 1: Type = MODEL_STAFF+10; break;
            case 2: Type = MODEL_SWORD+19; break;
            case 3: Type = MODEL_BOW+18; break;
#endif // LJW_FIX_PARTS_ENUM
            }
			Link = true;
			RenderLinkObject(0.f,0.f,15.f,c,w,Type,Level,Option1,Link,Translate);
        }

        CreatePartsFactory ( c );
        RenderParts ( c );

#ifndef GUILD_WAR_EVENT
        if ( InChaosCastle()==false )
#endif //GUILD_WAR_EVENT
        {
			//  날개
			PART_t *w = &c->Wing;
			if(w->Type != -1)
			{
				w->LinkBone  = 47;
				if(o->CurrentAction==PLAYER_FLY||o->CurrentAction==PLAYER_FLY_CROSSBOW)
				{
					if(w->Type == MODEL_WING+36)
					{
						w->PlaySpeed = 0.5f;
					}
					else
					{
						w->PlaySpeed = 1.f;
					}
				}
				else if(Type == -1)
				{
					w->PlaySpeed = 0.25f;
				}
				else
				{
					w->PlaySpeed = 0.25f;
				}
				bool Link  = false;

				switch(w->Type)		// 날개 위치 보정 - 왠만하면 모델을 고칠것->스케일, 붙는 위치 모두! (안그러면 펜릴탈때 이상함;)
				{
				case MODEL_WING+40:	// 제왕의 망토 (다크로드)
					w->LinkBone  = 19;	// 목 본에 붙인다
					RenderLinkObject(0.f,0.f,15.f,c,w,w->Type,w->Level,w->Option1,1,Translate);
					break;
				default:
					RenderLinkObject(0.f,0.f,15.f,c,w,w->Type,w->Level,w->Option1,Link,Translate);
					break;
				}
			}

            //  사탄
			Type = c->Helper.Type;
			Level = c->Helper.Level;
			if(Type == MODEL_HELPER+1)
			{
				PART_t *w = &c->Helper;
				w->LinkBone  = 34;
				w->PlaySpeed = 0.5f;
				RenderLinkObject(20.f,0.f,0.f,c,w,Type,Level,Option1,false,Translate);
				Vector(20.f,0.f,15.f,p);
				Models[o->Type].TransformPosition(o->BoneTransform[w->LinkBone],p,Position,true);
				Vector(Luminosity*0.5f,Luminosity*0.f,Luminosity*0.f,Light);
				CreateSprite(BITMAP_LIGHT,Position,1.5f,Light,o);
			}
        }
	}
#endif // CSK_REF_BACK_RENDERITEM
	
	//  무기 (손에 들고있을때)
	if ( !Bind )
	{
        //  투명일때 보조계열 효과는 보이지 않는다.
		if( !g_isCharacterBuff(o, eBuff_Cloaking) )
		{
			if ( g_isCharacterBuff(o, eBuff_AddCriticalDamage) && o->Kind==KIND_PLAYER && o->Type==MODEL_PLAYER && (MoveSceneFrame%30)==0 )
            {
                bool    renderSkillWave = (rand()%20) ? true : false;
                short   weaponType = -1;                
                Vector ( 0.f, 0.f, 0.f, p );
                Vector ( 1.f, 0.6f, 0.3f, Light );
                if ( c->Weapon[0].Type!=MODEL_BOW+15 )//&& ( c->Weapon[0].Type<MODEL_SHIELD || c->Weapon[0].Type>=MODEL_SHIELD+MAX_ITEM_INDEX ) )
                {
				    b->TransformPosition ( o->BoneTransform[c->Weapon[0].LinkBone], p, Position, true );
                    if ( c->Weapon[0].Type>=MODEL_BOW && c->Weapon[0].Type<MODEL_BOW+MAX_ITEM_INDEX )
                    {
                        weaponType = 1;
                    }
                    CreateEffect ( BITMAP_FLARE_FORCE, Position, o->Angle, o->Light, 1, o, weaponType, c->Weapon[0].LinkBone );
                    if ( renderSkillWave==false )
                    {
                        CreateEffect ( MODEL_DARKLORD_SKILL, Position, o->Angle, Light, 0 );
                    }
                }
                if ( c->Weapon[1].Type!=MODEL_BOW+7 && ( c->Weapon[1].Type<MODEL_SHIELD || c->Weapon[1].Type>=MODEL_SHIELD+MAX_ITEM_INDEX ) )
                {
				    b->TransformPosition ( o->BoneTransform[c->Weapon[1].LinkBone], p, Position, true );
                    if ( c->Weapon[1].Type>=MODEL_BOW && c->Weapon[1].Type<MODEL_BOW+MAX_ITEM_INDEX )
                    {
                        weaponType = 1;
                    }
                    CreateEffect ( BITMAP_FLARE_FORCE, Position, o->Angle, o->Light, 1, o, weaponType, c->Weapon[1].LinkBone );
                    if ( renderSkillWave==false )
                    {
                        CreateEffect ( MODEL_DARKLORD_SKILL, Position, o->Angle, Light, 1 );
                    }
                }

                PlayBuffer ( SOUND_CRITICAL, o );
            }
        }

		// 회오리 베기를 지정된 Tick 동안 오른손검 미출력을 위해 PostMoveProcess를 연동시킵니다.
#ifdef LDS_FIX_MODULE_POSTMOVECHARACTER_SKILL_WHEEL
		c->PostMoveProcess_Process();
#endif // LDS_FIX_MODULE_POSTMOVECHARACTER_SKILL_WHEEL

		for(int i=0;i<2;i++)
		{

			// 회오리 베기를 15프레임 이상 시간동안 오른손 검 미출력을 하기 위해
			// Frame Tick count를 따로 계산 하여 반영 합니다.
#ifdef LDS_FIX_MODULE_POSTMOVECHARACTER_SKILL_WHEEL
			if( i==0 )
			{
				if ( o->CurrentAction==PLAYER_ATTACK_SKILL_FURY_STRIKE && o->AnimationFrame<=4.f ) 
				{
					continue;
				}
				
				if ( true == c->PostMoveProcess_IsProcessing() )
				{
					c->Skill = 0;
					
					continue;
				}
			}

#else // LDS_FIX_MODULE_POSTMOVECHARACTER_SKILL_WHEEL

#ifdef CSK_FIX_WHEELSKILL_ITEM_RENDER
			// 분노의일격 스킬을 사용하면 오른손 무기를 4.f 프레임 이내에서는 안보이게 수정
			// 회호리베기 스킬을 사용하면 오른손 무기를 안보이게 수정
			if( ( o->CurrentAction==PLAYER_ATTACK_SKILL_FURY_STRIKE && ( i==0 ) && o->AnimationFrame<=4.f ) ||
#ifdef YDG_ADD_SKILL_RIDING_ANIMATIONS

				( (o->CurrentAction==PLAYER_ATTACK_SKILL_WHEEL || o->CurrentAction==PLAYER_ATTACK_SKILL_WHEEL_UNI
					|| o->CurrentAction==PLAYER_ATTACK_SKILL_WHEEL_DINO || o->CurrentAction==PLAYER_ATTACK_SKILL_WHEEL_FENRIR) && i==0 )
#ifdef LDS_FIX_WHENBLOODATTACK_DISSAPEARRIGHTSWORD
					&& ( c->Skill == AT_SKILL_WHEEL 
					|| (AT_SKILL_TORNADO_SWORDA_UP <= c->Skill && c->Skill <= AT_SKILL_TORNADO_SWORDA_UP+4) 
					|| (AT_SKILL_TORNADO_SWORDB_UP <= c->Skill && c->Skill <= AT_SKILL_TORNADO_SWORDB_UP+4) )
#endif // LDS_FIX_WHENBLOODATTACK_DISSAPEARRIGHTSWORD
				)

#else	// YDG_ADD_SKILL_RIDING_ANIMATIONS

				( (o->CurrentAction == PLAYER_ATTACK_SKILL_WHEEL && i==0)
#ifdef LDS_FIX_WHENBLOODATTACK_DISSAPEARRIGHTSWORD
					&& ( c->Skill == AT_SKILL_WHEEL 
					|| (AT_SKILL_TORNADO_SWORDA_UP <= c->Skill && c->Skill <= AT_SKILL_TORNADO_SWORDA_UP+4) 
					|| (AT_SKILL_TORNADO_SWORDB_UP <= c->Skill && c->Skill <= AT_SKILL_TORNADO_SWORDB_UP+4) )
#endif // LDS_FIX_WHENBLOODATTACK_DISSAPEARRIGHTSWORD
					) )

#endif	// YDG_ADD_SKILL_RIDING_ANIMATIONS
			{
					continue;
			}
#else // CSK_FIX_WHEELSKILL_ITEM_RENDER
			if( ( c->Skill)!=AT_SKILL_WHEEL 
				&& !(AT_SKILL_TORNADO_SWORDA_UP <= ( c->Skill) && ( c->Skill) <= AT_SKILL_TORNADO_SWORDA_UP+4)
				&& !(AT_SKILL_TORNADO_SWORDB_UP <= ( c->Skill) && ( c->Skill) <= AT_SKILL_TORNADO_SWORDB_UP+4)
				)
            {
				if( ( o->CurrentAction==PLAYER_ATTACK_SKILL_FURY_STRIKE && ( i==0 ) && o->AnimationFrame<=4.f ) )
                    continue;
            }
            else
            {
                //  기사 스킬 1, 2 사용시 무기 하나를 사라지게 한다.
				if( ( o->CurrentAction==PLAYER_ATTACK_SKILL_FURY_STRIKE && ( i==0 ) && o->AnimationFrame<=4.f ) ||
					( o->CurrentAction == PLAYER_ATTACK_SKILL_WHEEL && i==0 ) )
					continue;
            }
#endif // CSK_FIX_WHEELSKILL_ITEM_RENDER
            
#endif // LDS_FIX_MODULE_POSTMOVECHARACTER_SKILL_WHEEL


            PART_t *w = &c->Weapon[i];
			if ( 
                w->Type!=-1 && w->Type!=MODEL_BOW+7 && w->Type!=MODEL_BOW+15
#ifdef PET_SYSTEM
                && w->Type!=MODEL_HELPER+5
#endif// PET_SYSTEM
               )
			{
				if(o->CurrentAction==PLAYER_ATTACK_BOW || o->CurrentAction==PLAYER_ATTACK_CROSSBOW ||
         			o->CurrentAction==PLAYER_ATTACK_FLY_BOW || o->CurrentAction==PLAYER_ATTACK_FLY_CROSSBOW)
				{
					// 활을 착용하고 있고 공격을 하면 활의 에니메이션이 되게 해주는 코드
					// playspeed의 값을 넣어주므로서 에니메이션이 동작하게 해줌
#ifdef ADD_SOCKET_ITEM
					// 보우 공격 애니메이션
					// 다크스팅거(23)는 다른보우와는 다르게 1번이 공격애니메니션 0번이 기본애니메이션이다.
					if( w->Type == MODEL_BOW+23 )
					{
						w->CurrentAction = 1;
					}
					else
					{
						w->CurrentAction = 0;
					}
					w->PlaySpeed = Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_BOW].PlaySpeed;
#else // ADD_SOCKET_ITEM								// 정리할 때 지워야 하는 소스
					w->CurrentAction = 0;
					w->PlaySpeed = Models[MODEL_PLAYER].Actions[PLAYER_ATTACK_BOW].PlaySpeed;
#endif // ADD_SOCKET_ITEM								// 정리할 때 지워야 하는 소스
				}
				else if(w->Type==MODEL_MACE+2)
				{
					if(o->CurrentAction>=PLAYER_ATTACK_SWORD_RIGHT1 && o->CurrentAction<=PLAYER_ATTACK_SWORD_RIGHT2)
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
				else if(w->Type==MODEL_MACE+5)
				{
					w->CurrentAction = 0;
					w->PlaySpeed = Models[MODEL_PLAYER].Actions[PLAYER_STOP_MALE].PlaySpeed*2.f;
				}
				else if(w->Type==MODEL_STAFF+6)
				{
					w->CurrentAction = 0;
					w->PlaySpeed = Models[MODEL_PLAYER].Actions[PLAYER_STOP_MALE].PlaySpeed*15.f;
				}
				else if(w->Type>=MODEL_SWORD && w->Type<MODEL_SWORD+MAX_ITEM_INDEX)
				{
					w->CurrentAction = 0;
					w->PlaySpeed = Models[MODEL_PLAYER].Actions[PLAYER_STOP_MALE].PlaySpeed;
				}
#ifdef ADD_SOCKET_ITEM
				// 기본애니메이션이 있는 무기들
				else if( w->Type == MODEL_BOW+23 )
				{	
					w->CurrentAction = 0;		
					w->PlaySpeed = Models[MODEL_PLAYER].Actions[PLAYER_STOP_MALE].PlaySpeed;
				}
				else if( (w->Type == MODEL_MACE+16)
					|| (w->Type == MODEL_MACE+17) )
				{
					w->CurrentAction = 0;
					w->PlaySpeed = Models[MODEL_PLAYER].Actions[PLAYER_STOP_MALE].PlaySpeed;
				}
#endif // ADD_SOCKET_ITEM
#ifdef LDK_ADD_GAMBLERS_WEAPONS
				else if( w->Type == MODEL_STAFF+34 )
				{
					w->CurrentAction = 0;
					w->PlaySpeed = Models[MODEL_PLAYER].Actions[PLAYER_STOP_MALE].PlaySpeed;
				}
#endif //LDK_ADD_GAMBLERS_WEAPONS
				else
				{
					w->CurrentAction       = 0;
					w->PlaySpeed           = 0.f;
					w->AnimationFrame      = 0.f;
					w->PriorAnimationFrame = 0.f;
				}
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
				if(g_CMonkSystem.IsSwordformGloves(w->Type))
					g_CMonkSystem.RenderSwordformGloves(c, w->Type, i, o->Alpha, Translate, Select);
				else
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
				RenderLinkObject(0.f,0.f,0.f,c,w,w->Type,w->Level,w->Option1,false,Translate);

				if(w->Level >= 7)
				{
                   	Vector(Luminosity*0.5f,Luminosity*0.4f,Luminosity*0.3f,Light);
				}
				else if(w->Level >= 5)
				{
                   	Vector(Luminosity*0.3f,Luminosity*0.3f,Luminosity*0.5f,Light);
				}
				else if(w->Level >= 3)
				{
                   	Vector(Luminosity*0.5f,Luminosity*0.3f,Luminosity*0.3f,Light);
				}
				else
				{
                   	Vector(Luminosity*0.3f,Luminosity*0.3f,Luminosity*0.3f,Light);
				}
				float Scale;
				if(c->PK<PVP_MURDERER2 && c->Level!=4)
				{
					bool Success = true;

					switch(w->Type)
					{
					case MODEL_SWORD+4:
					case MODEL_SWORD+5:
					case MODEL_SWORD+13:
						Vector(0.f,-110.f,5.f,Position);
						break;
					case MODEL_SWORD+8:
					case MODEL_SWORD+9:
						Vector(0.f,-110.f,-5.f,Position);
						break;
					case MODEL_SWORD+7:
					case MODEL_SWORD+10:
						Vector(0.f,-110.f,0.f,Position);
						break;
					case MODEL_SWORD+14:
					case MODEL_SWORD+11:
						Vector(0.f,-150.f,0.f,Position);
						break;
					case MODEL_SWORD+12:
                       	Vector(Luminosity,Luminosity,Luminosity,Light);
						Vector(0.f,-160.f,0.f,Position);
						break;
                    case MODEL_SWORD+17:
     					Success = false;
						Scale = sinf(WorldTime*0.004f)*10.f+20.f;
                        {
                            vec3_t  pos1, pos2;

                            Vector(0.f,-20.f,-40.f,Position);
							Models[o->Type].TransformPosition(o->BoneTransform[w->LinkBone],Position,pos1,true);
                            Vector(0.f,-160.f,-10.f,Position);
							Models[o->Type].TransformPosition(o->BoneTransform[w->LinkBone],Position,pos2,true);
                            CreateJoint(BITMAP_FLARE+1,pos1,pos2,o->Angle,4,o,Scale);

                            Vector(0.f,-10.f,28.f,Position);
							Models[o->Type].TransformPosition(o->BoneTransform[w->LinkBone],Position,pos1,true);
                            Vector(0.f,-145.f,18.f,Position);
							Models[o->Type].TransformPosition(o->BoneTransform[w->LinkBone],Position,pos2,true);
                            CreateJoint(BITMAP_FLARE+1,pos1,pos2,o->Angle,4,o,Scale);
                        }
                        break;

                    case MODEL_SWORD+18:
                        {
     						Success = false;
							
                            Vector(0.f,-20.f,15.f,Position);
							Models[o->Type].TransformPosition(o->BoneTransform[w->LinkBone],Position,p,true);

                            Scale = sinf(WorldTime*0.004f)*0.3f+0.3f;
                            Vector(Scale*0.2f,Scale*0.2f,Scale*1.f,Light);
							CreateSprite(BITMAP_SHINY+1,p,Scale+1.f,Light,o,0);

                            Vector(0.f,-133.f,7.f,Position);
							Models[o->Type].TransformPosition(o->BoneTransform[w->LinkBone],Position,Position,true);

                            Scale = ( Scale*20.f ) + 20.f;
            				CreateJoint(BITMAP_JOINT_THUNDER,p,Position,o->Angle,10,NULL,Scale);
            				CreateJoint(BITMAP_JOINT_THUNDER,p,Position,o->Angle,10,NULL,Scale);
            				CreateJoint(BITMAP_JOINT_THUNDER,p,Position,o->Angle,10,NULL,Scale);
                        }
                        break;
                    case MODEL_STAFF+9:
						Success = false;
						Vector(0.f,-120.f,5.f,Position);
						Vector(Luminosity*0.6f,Luminosity*0.6f,Luminosity*2.f,Light);
						Models[o->Type].TransformPosition(o->BoneTransform[w->LinkBone],Position,p,true);
						CreateSprite(BITMAP_SHINY+1,p,1.5f,Light,o);
						CreateSprite(BITMAP_LIGHT,p,Luminosity+1.f,Light,o);

                        Vector(0.f,100.f,10.f,Position);
						Models[o->Type].TransformPosition(o->BoneTransform[w->LinkBone],Position,p,true);
						CreateSprite(BITMAP_LIGHT,p,Luminosity+1.f,Light,o);
                        break;
					case MODEL_STAFF+4:
						Success = false;
						Vector(0.f,-90.f,0.f,Position);
						Vector(Luminosity*0.4f,Luminosity*0.8f,Luminosity*0.6f,Light);
						Models[o->Type].TransformPosition(o->BoneTransform[w->LinkBone],Position,p,true);
						CreateSprite(BITMAP_SHINY+1,p,2.f,Light,o);
						break;
					case MODEL_SHIELD+14:
     					Success = false;
						Vector(20.f,0.f,0.f,Position);
						Vector(Luminosity*0.4f,Luminosity*0.6f,Luminosity*1.5f,Light);
						Models[o->Type].TransformPosition(o->BoneTransform[w->LinkBone],Position,p,true);
						CreateSprite(BITMAP_SHINY+1,p,1.5f,Light,o);
						break;
					case MODEL_STAFF+5:
     					Success = false;
						Vector(0.f,-145.f,0.f,Position);
						Vector(Luminosity*0.4f,Luminosity*0.6f,Luminosity*1.f,Light);
						Models[o->Type].TransformPosition(o->BoneTransform[w->LinkBone],Position,p,true);
						CreateSprite(BITMAP_SHINY+1,p,1.5f,Light,o);
						CreateSprite(BITMAP_LIGHTNING+1,p,0.3f,Light,o);
						break;
					case MODEL_STAFF+7:
     					Success = false;
						Vector(Luminosity*0.4f,Luminosity*0.6f,Luminosity*1.f,Light);
                   		RenderBrightEffect(b,BITMAP_SHINY+1,27,2.f,Light,o);

						for(int j=28;j<=37;j++)
						{
                       		RenderBrightEffect(b,BITMAP_LIGHT,j,1.5f,Light,o);
						}
						break;
					case MODEL_STAFF+6:
     					Success = false;
						Vector(0.f,-145.f,0.f,Position);
						Models[o->Type].TransformPosition(o->BoneTransform[w->LinkBone],Position,p,true);
                        Vector(Luminosity*1.f,Luminosity*0.6f,Luminosity*0.4f,Light);
						CreateSprite(BITMAP_SPARK  ,p,3.f,Light,o);
						CreateSprite(BITMAP_SHINY+2,p,1.5f,Light,o);

						for(int j=0;j<4;j++)
						{
							Vector((float)(rand()%20-10),(float)(rand()%20-10-90.f),(float)(rand()%20-10),Position);
							Models[o->Type].TransformPosition(o->BoneTransform[w->LinkBone],Position,p,true);
							CreateParticle(BITMAP_SPARK,p,o->Angle,Light,1);
						}
                        Vector(Luminosity*1.f,Luminosity*0.2f,Luminosity*0.1f,Light);

						for(int j=0;j<10;j++)
						{
							if(rand()%4 < 3)
							{
								Vector(0.f,-j*20+60.f,0.f,Position);
								Models[o->Type].TransformPosition(o->BoneTransform[w->LinkBone],Position,p,true);
								CreateSprite(BITMAP_LIGHT,p,1.f,Light,o);
							}
						}
						break;
					case MODEL_STAFF+15:
     					Success = false;
						Vector(Luminosity*0.2f,Luminosity*0.3f,Luminosity*1.4f,Light);
						Vector(0.f,0.f,0.f,Position);
						b->TransformPosition(BoneTransform[1],Position,p,true);
						CreateSprite(BITMAP_SHINY+1,p,1.f,Light,o,-(int)WorldTime*0.1f);
						b->TransformPosition(BoneTransform[2],Position,p,true);
						CreateSprite(BITMAP_SHINY+1,p,0.5f,Light,o,(int)WorldTime*0.1f);
						break;
					case MODEL_STAFF+16:
     					Success = false;
						Vector(Luminosity*1.0f,Luminosity*0.3f,Luminosity*0.4f,Light);
						Vector(0.f,0.f,0.f,Position);
						b->TransformPosition(BoneTransform[1],Position,p,true);
						CreateSprite(BITMAP_SHINY+1,p,1.f,Light,o,-(int)WorldTime*0.1f);
						CreateSprite(BITMAP_SHINY+1,p,1.f,Light,o,-(int)WorldTime*0.13f);
						break;
					case MODEL_STAFF+17:
     					Success = false;
						Scale = absf(sinf(WorldTime*0.002f))*0.2f;
						Luminosity = absf(sinf(WorldTime*0.002f))*0.4f;
						Vector(0.5f+Luminosity, 0.2f+Luminosity, 0.9f+Luminosity, Light);

						for (int j = 1; j <= 4; ++j)
							RenderBrightEffect(b,BITMAP_LIGHT,j,Scale+1.0f,Light,o);
						break;
					case MODEL_STAFF+18:
     					Success = false;
						Scale = absf(sinf(WorldTime*0.002f))*0.2f;
						Luminosity = absf(sinf(WorldTime*0.002f))*0.4f;
						Vector(0.5f+Luminosity, 0.2f+Luminosity, 0.9f+Luminosity, Light);

						for (int j = 1; j <= 2; ++j)
						{
							RenderBrightEffect(b,BITMAP_SHINY+1,j,Scale+1.0f,Light,o);
							RenderBrightEffect(b,BITMAP_LIGHT,j,Scale+1.0f,Light,o);
						}
						Vector(0.8f+Luminosity, 0.6f+Luminosity, 0.3f+Luminosity, Light);
						RenderBrightEffect(b,BITMAP_LIGHT,3,Scale+1.0f,Light,o);
						break;
					case MODEL_STAFF+19:
     					Success = false;
						Scale = absf(sinf(WorldTime*0.002f))*0.2f;
						Luminosity = absf(sinf(WorldTime*0.002f))*0.4f;
						Vector(0.5f+Luminosity, 0.2f+Luminosity, 0.9f+Luminosity, Light);

						for (int j = 2; j <= 3; ++j)
							RenderBrightEffect(b,BITMAP_LIGHT,j,Scale+1.0f,Light,o);

						Vector(0.8f+Luminosity, 0.6f+Luminosity, 0.3f+Luminosity, Light);
						RenderBrightEffect(b,BITMAP_SHINY+2,2,Scale+1.0f,Light,o);
						break;
					case MODEL_STAFF+20:
     					Success = false;
						Vector(1.0f,0.2f,0.1f,Light);
						Vector(0.f,0.f,0.f,Position);
						b->TransformPosition(BoneTransform[2],Position,p,true);
						CreateSprite(BITMAP_EVENT_CLOUD,p,0.25f,Light,o,-(int)WorldTime*0.1f);
						CreateSprite(BITMAP_EVENT_CLOUD,p,0.25f,Light,o,-(int)WorldTime*0.2f);
						Vector(1.0f,0.4f,0.3f,Light);
						RenderBrightEffect(b,BITMAP_SHINY+1,2,1.0f,Light,o);
						Vector(1.0f,0.2f,0.0f,Light);
#ifndef	ASG_ADD_ETERNALWING_STICK_EFFECT
						CreateParticle(BITMAP_SPARK+1, p, o->Angle, Light, 16, 1.0f);
#endif	// ASG_ADD_ETERNALWING_STICK_EFFECT

						CreateParticle(BITMAP_SPARK+1, p, o->Angle, Light, 23, 1.0f);
#ifdef ASG_ADD_ETERNALWING_STICK_EFFECT
						// 떨어지며 바닥에 튕기는 파티클.
						if (rand()%20 == 0)
							CreateParticle(BITMAP_SPARK+1, p, o->Angle, Light, 20, 1.0f);
						// 이펙트 중심의 발광체 모양 빌보드.
						Vector(1.0f,0.0f,0.0f,Light);
						RenderBrightEffect(b,BITMAP_LIGHT,2,3.0f,Light,o);
#endif	// ASG_ADD_ETERNALWING_STICK_EFFECT
						break;
					case MODEL_BOW+16:
     					Success = false;
						Vector(Luminosity*0.4f,Luminosity*0.6f,Luminosity*1.f,Light);

						for(int j=0;j<6;j++)
						{
							Vector(0.f,-10.f,-j*20.f,Position);
							Models[o->Type].TransformPosition(o->BoneTransform[w->LinkBone],Position,p,true);
							CreateSprite(BITMAP_LIGHT,p,2.f,Light,o);
						}
						break;
					case MODEL_MACE+5:
     					Success = false;
                        Vector(Luminosity*1.f,Luminosity*0.6f,Luminosity*0.4f,Light);

						for(int j=0;j<8;j++)
						{
							if(rand()%4 < 3)
							{
								Vector(0.f,-j*20-30.f,0.f,Position);
								Models[o->Type].TransformPosition(o->BoneTransform[w->LinkBone],Position,p,true);
								CreateSprite(BITMAP_LIGHT,p,1.f,Light,o);
							}
						}
						break;
					case MODEL_MACE+6:
     					Success = false;
						Vector(0.f,-84.f,0.f,Position);
						Models[o->Type].TransformPosition(o->BoneTransform[w->LinkBone],Position,p,true);
						Scale = sinf(WorldTime*0.004f)*0.3f+0.7f;
                        Vector(Scale*1.f,Scale*0.2f,Scale*0.1f,Light);
						CreateSprite(BITMAP_SHINY+1,p,Scale+1.5f,Light,o);

						for(int j=0;j<5;j++)
						{
							Vector(0.f,-j*20.f-10.f,0.f,Position);
							Models[o->Type].TransformPosition(o->BoneTransform[w->LinkBone],Position,p,true);
							CreateSprite(BITMAP_SHINY+1,p,1.f,Light,o);
						}
                        Vector(Scale*0.5f,Scale*0.1f,Scale*0.05f,Light);
						RenderBrightEffect(b,BITMAP_SHINY+1,2,1.f,Light,o);
						RenderBrightEffect(b,BITMAP_SHINY+1,6,1.f,Light,o);
						break;
					case MODEL_BOW+13:
					case MODEL_BOW+14:
     					Success = false;
						if(w->Type==MODEL_BOW+13)
						{
							Vector(Luminosity*0.2f,Luminosity*0.4f,Luminosity*0.6f,Light);
						}
						else
						{
							Vector(Luminosity*0.6f,Luminosity*0.4f,Luminosity*0.2f,Light);
						}

						for(int j=0;j<6;j++)
						{
							Vector(0.f,-20.f,(float)(-j*20),Position);
							Models[o->Type].TransformPosition(o->BoneTransform[w->LinkBone],Position,p,true);
							CreateSprite(BITMAP_LIGHT,p,2.f,Light,o);
						}
						break;
					default:
						Success = false;
					}
					if(Success)
					{
						Models[o->Type].TransformPosition(o->BoneTransform[w->LinkBone],Position,p,true);
						CreateSprite(BITMAP_LIGHT,p,1.4f,Light,o);
					}
				}
			}
		}
	}
	switch ( o->Type )
	{
	case MODEL_PLAYER:
		Vector(0.f,0.f,0.f,p);

		if ( GetCharacterClass(c->Class) == CLASS_SOULMASTER)	        
        {
			if( !g_isCharacterBuff(o, eBuff_Cloaking) )
			{
				Vector(-4.f,11.f,0.f,p);
				Vector(1.f,1.f,1.f,Light);
    			b->TransformPosition(o->BoneTransform[19],p,Position,true);
				CreateSprite(BITMAP_SPARK+1,Position,0.6f,Light,NULL);

				float scale = sinf(WorldTime*0.001f)*0.4f;
				CreateSprite(BITMAP_SHINY+1,Position,scale,Light,NULL);
			}
        }
		if ( o->CurrentAction==PLAYER_SKILL_FLASH || o->CurrentAction==PLAYER_ATTACK_RIDE_ATTACK_FLASH 
			|| o->CurrentAction == PLAYER_FENRIR_ATTACK_DARKLORD_FLASH	//^ 펜릴 스킬 관련
			)
		{
			if ( GetBaseClass(c->Class)==CLASS_DARK_LORD || o->CurrentAction==PLAYER_ATTACK_RIDE_ATTACK_FLASH 
			|| o->CurrentAction == PLAYER_FENRIR_ATTACK_DARKLORD_FLASH	//^ 펜릴 스킬 관련
				)
            {
                if( o->AnimationFrame>=1.2f && o->AnimationFrame<1.6f )
                {
                    vec3_t Angle;
                    Vector(1.f,0.f,0.f,Angle);
		            CreateEffect ( BITMAP_GATHERING, o->Position, o->Angle, o->Light, 2, o );

                    PlayBuffer ( SOUND_ELEC_STRIKE_READY );
                }

                if( o->AnimationFrame<2.f )
                {
                    if ( PartyNumber>0 /*&& rand()%2==0*/ )
                    {
						if ( g_pPartyManager->IsPartyMemberChar( c )==false ) 
							break;
                        
                        for ( int i=0; i<PartyNumber; ++i )
                        {
                            PARTY_t* p = &Party[i];
                            if ( p->index<0 ) continue;
                            
                            CHARACTER* tc = &CharactersClient[p->index];
                            if ( tc!=c )
                            {
                                VectorCopy ( tc->Object.Position, Position );
                                Position[2] += 150.f;
                                CreateJoint ( BITMAP_JOINT_ENERGY, Position, Position, o->Angle, 12, o, 20.f );
                                CreateJoint ( BITMAP_JOINT_ENERGY, Position, Position, o->Angle, 13, o, 20.f );
                            }
                        }
                    }
                }

                if ( o->AnimationFrame>=7 && o->AnimationFrame<8 )
                {
                    vec3_t Angle, Light;

    				b->TransformPosition(o->BoneTransform[c->Weapon[0].LinkBone],p,Position,true);

					Vector ( 0.8f, 0.5f, 1.f, Light );
					Vector ( 180.f, 45.f, 0.f, Angle );
					CreateEffect ( MODEL_DARKLORD_SKILL, Position, Angle, Light, 2 );

					Vector ( 0.f, 0.f, o->Angle[2], Angle );
					CreateEffect ( MODEL_DARKLORD_SKILL, Position, Angle, Light, 2 );
                }
            }
            else
            {
				if( g_isCharacterBuff(o, eBuff_Cloaking) )
				{
					b->TransformPosition(o->BoneTransform[c->Weapon[0].LinkBone],p,Position,true);
					Vector(0.1f,0.1f,1.f,Light);
					float Scale = o->AnimationFrame*0.1f;
					CreateSprite(BITMAP_LIGHTNING+1,Position,Scale*0.3f,Light,o);
					CreateSprite(BITMAP_LIGHTNING+1,Position,Scale*1.f ,o->Light,o,-(int)WorldTime*0.1f);
					CreateSprite(BITMAP_LIGHTNING+1,Position,Scale*2.5f,o->Light,o,(int)WorldTime*0.1f);
				}
            }
		}
		else if(o->CurrentAction == PLAYER_SKILL_LIGHTNING_ORB
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
		else if (o->CurrentAction >= PLAYER_SKILL_SLEEP
			&& o->CurrentAction <= PLAYER_SKILL_SLEEP_FENRIR)
		{
			int iSkillType = CharacterAttribute->Skill[Hero->CurrentSkill];

			vec3_t vLight, vRelativePos, vWorldPos;
			Vector(1.0f, 1.0f, 1.0f, vLight);
			Vector(0.f, 0.f, 0.f, vRelativePos);
			Vector(0.f, 0.f, 0.f, vWorldPos);
			b->TransformPosition(o->BoneTransform[37], vRelativePos, vWorldPos, true);	// "Bip01 L Hand"
			
			float fRot = (WorldTime*0.0006f) * 360.0f;

			// shiny
			if(iSkillType == AT_SKILL_ALICE_SLEEP
#ifdef PJH_ADD_MASTERSKILL
						|| (AT_SKILL_ALICE_SLEEP_UP <= iSkillType && iSkillType <= AT_SKILL_ALICE_SLEEP_UP+4)
#endif
				)	// nukun
			{
				Vector(0.5f, 0.2f, 0.8f, vLight);
			}
			else if(iSkillType == AT_SKILL_ALICE_BLIND)
			{
				Vector(1.0f, 1.0f, 1.0f, vLight);
			}
			else if(iSkillType == AT_SKILL_ALICE_THORNS)
			{
				Vector(0.8f, 0.5f, 0.2f, vLight);
			}
#ifdef ASG_ADD_SKILL_BERSERKER
			else if(iSkillType == AT_SKILL_ALICE_BERSERKER)
			{
				Vector(1.0f, 0.1f, 0.2f, vLight);
			}
#endif	// ASG_ADD_SKILL_BERSERKER
			else if(iSkillType == AT_SKILL_ALICE_WEAKNESS)
			{
				Vector(0.8f, 0.1f, 0.1f, vLight);
			}
			else if(iSkillType == AT_SKILL_ALICE_ENERVATION)
			{
				Vector(0.25f, 1.0f, 0.7f, Light);
			}

			if(iSkillType == AT_SKILL_ALICE_SLEEP || iSkillType == AT_SKILL_ALICE_THORNS
#ifdef PJH_ADD_MASTERSKILL
						|| (AT_SKILL_ALICE_SLEEP_UP <= iSkillType && iSkillType <= AT_SKILL_ALICE_SLEEP_UP+4)
#endif
#ifdef ASG_ADD_SKILL_BERSERKER
				|| iSkillType == AT_SKILL_ALICE_BERSERKER
#endif	// ASG_ADD_SKILL_BERSERKER
				|| iSkillType == AT_SKILL_ALICE_WEAKNESS || iSkillType == AT_SKILL_ALICE_ENERVATION
				)
			{
				CreateSprite(BITMAP_SHINY+5, vWorldPos, 1.0f, vLight, o, fRot);
				CreateSprite(BITMAP_SHINY+5, vWorldPos, 0.7f, vLight, o, -fRot);
			}
			else if(iSkillType == AT_SKILL_ALICE_BLIND)
			{
				CreateSprite(BITMAP_SHINY+5, vWorldPos, 1.0f, vLight, o, fRot, 1);
				CreateSprite(BITMAP_SHINY+5, vWorldPos, 0.7f, vLight, o, -fRot, 1);	
			}

			// pin_light
			if(iSkillType == AT_SKILL_ALICE_SLEEP
#ifdef PJH_ADD_MASTERSKILL
						|| (AT_SKILL_ALICE_SLEEP_UP <= iSkillType && iSkillType <= AT_SKILL_ALICE_SLEEP_UP+4)
#endif
				)
			{
				Vector(0.7f, 0.0f, 0.8f, vLight);
			}
			else if(iSkillType == AT_SKILL_ALICE_BLIND)
			{
				Vector(1.0f, 1.0f, 1.0f, vLight);
			}
			else if(iSkillType == AT_SKILL_ALICE_THORNS)
			{
				Vector(0.8f, 0.5f, 0.2f, vLight);
			}
#ifdef ASG_ADD_SKILL_BERSERKER
			else if(iSkillType == AT_SKILL_ALICE_BERSERKER)
			{
				Vector(1.0f, 0.1f, 0.2f, vLight);
			}
#endif	// ASG_ADD_SKILL_BERSERKER
			else if(iSkillType == AT_SKILL_ALICE_THORNS)
			{
				Vector(0.8f, 0.1f, 0.1f, vLight);
			}
			else if(iSkillType == AT_SKILL_ALICE_ENERVATION)
			{
				Vector(0.25f, 1.f, 0.7f, vLight);
			}

			if(iSkillType == AT_SKILL_ALICE_SLEEP || iSkillType == AT_SKILL_ALICE_THORNS
#ifdef PJH_ADD_MASTERSKILL
						|| (AT_SKILL_ALICE_SLEEP_UP <= iSkillType && iSkillType <= AT_SKILL_ALICE_SLEEP_UP+4)
#endif
#ifdef ASG_ADD_SKILL_BERSERKER
				|| iSkillType == AT_SKILL_ALICE_BERSERKER
#endif	// ASG_ADD_SKILL_BERSERKER
				|| iSkillType == AT_SKILL_ALICE_WEAKNESS || iSkillType == AT_SKILL_ALICE_ENERVATION
				)
			{
				CreateSprite(BITMAP_PIN_LIGHT, vWorldPos, 1.7f, vLight, o, (float)(rand()%360));
				CreateSprite(BITMAP_PIN_LIGHT, vWorldPos, 1.5f, vLight, o, (float)(rand()%360));
			}
			else if(iSkillType == AT_SKILL_ALICE_BLIND)
			{
				CreateSprite(BITMAP_PIN_LIGHT, vWorldPos, 1.7f, vLight, o, (float)(rand()%360), 1);
				CreateSprite(BITMAP_PIN_LIGHT, vWorldPos, 1.5f, vLight, o, (float)(rand()%360), 1);
			}
			
			// cra04, clud64
			if(iSkillType == AT_SKILL_ALICE_SLEEP
#ifdef PJH_ADD_MASTERSKILL
						|| (AT_SKILL_ALICE_SLEEP_UP <= iSkillType && iSkillType <= AT_SKILL_ALICE_SLEEP_UP+4)
#endif
				)
			{
				Vector(0.6f, 0.1f, 0.8f, vLight);
			}
			else if(iSkillType == AT_SKILL_ALICE_BLIND)
			{
				Vector(1.0f, 1.0f, 1.0f, vLight);
			}
			else if(iSkillType == AT_SKILL_ALICE_THORNS)
			{
				Vector(0.8f, 0.5f, 0.2f, vLight);
			}
#ifdef ASG_ADD_SKILL_BERSERKER
			else if(iSkillType == AT_SKILL_ALICE_BERSERKER)
			{
				Vector(1.0f, 0.1f, 0.2f, vLight);
			}
#endif	// ASG_ADD_SKILL_BERSERKER
			else if(iSkillType == AT_SKILL_ALICE_THORNS)
			{
				Vector(0.8f, 0.1f, 0.1f, vLight);
			}
			else if(iSkillType == AT_SKILL_ALICE_ENERVATION)
			{
				Vector(0.25f, 1.f, 0.7f, vLight);
			}

			if(iSkillType == AT_SKILL_ALICE_SLEEP || iSkillType == AT_SKILL_ALICE_THORNS
#ifdef PJH_ADD_MASTERSKILL
						|| (AT_SKILL_ALICE_SLEEP_UP <= iSkillType && iSkillType <= AT_SKILL_ALICE_SLEEP_UP+4)
#endif
#ifdef ASG_ADD_SKILL_BERSERKER
				|| iSkillType == AT_SKILL_ALICE_BERSERKER
#endif	// ASG_ADD_SKILL_BERSERKER
				|| iSkillType == AT_SKILL_ALICE_WEAKNESS || iSkillType == AT_SKILL_ALICE_ENERVATION
				)
			{
				CreateParticle(BITMAP_LIGHT+2, vWorldPos, o->Angle, vLight, 0, 1.0f);
				CreateParticle(BITMAP_CLUD64, vWorldPos, o->Angle, vLight, 3, 0.5f);
			}
			else if(iSkillType == AT_SKILL_ALICE_BLIND)
			{
				CreateParticle(BITMAP_LIGHT+2, vWorldPos, o->Angle, vLight, 4, 1.0f);
				CreateParticle(BITMAP_CLUD64, vWorldPos, o->Angle, vLight, 5, 0.5f);
			}
		}
		// ChainLighting
		else if( o->CurrentAction == PLAYER_SKILL_CHAIN_LIGHTNING )
		{
			vec3_t vRelativePos, vWorldPos, vLight;
			Vector(0.f, 0.f, 0.f, vRelativePos);
			Vector(0.4f,0.4f,0.8f, vLight);

			b->TransformPosition(o->BoneTransform[37], vRelativePos, vWorldPos, true);	// "Bip01 L Hand"
			CreateEffect(MODEL_FENRIR_THUNDER, vWorldPos, o->Angle, vLight, 2, o );
			CreateEffect(MODEL_FENRIR_THUNDER, vWorldPos, o->Angle, vLight, 2, o);
			CreateSprite(BITMAP_LIGHT, vWorldPos, 1.5f, vLight, o, 0.f);
			
			b->TransformPosition(o->BoneTransform[28], vRelativePos, vWorldPos, true);	// "Bip01 R Hand"
			CreateEffect(MODEL_FENRIR_THUNDER, vWorldPos, o->Angle, vLight, 2, o);
			CreateEffect(MODEL_FENRIR_THUNDER, vWorldPos, o->Angle, vLight, 2, o);
			CreateSprite(BITMAP_LIGHT, vWorldPos, 1.5f, vLight, o, 0.f);	
		}

		if( g_isCharacterBuff(o, eBuff_Attack) || g_isCharacterBuff(o, eBuff_HelpNpc) )
		{
			if ( !g_isCharacterBuff(o, eBuff_Cloaking) )
            {
				for(int i=0;i<2;i++)
			    {
				    Luminosity = (float)(rand()%30+70)*0.01f;
				    Vector(Luminosity*1.f,Luminosity*0.3f,Luminosity*0.2f,Light);
				    b->TransformPosition(o->BoneTransform[c->Weapon[i].LinkBone],p,Position,true);
				    CreateSprite(BITMAP_SHINY+1,Position,1.5f,Light,o);
				    b->TransformPosition(o->BoneTransform[c->Weapon[i].LinkBone-6],p,Position,true);
				    CreateSprite(BITMAP_SHINY+1,Position,1.5f,Light,o);
				    b->TransformPosition(o->BoneTransform[c->Weapon[i].LinkBone-7],p,Position,true);
				    CreateSprite(BITMAP_SHINY+1,Position,1.5f,Light,o);
			    }
				if ( !SearchJoint( MODEL_SPEARSKILL, o, 4 ) 
					&& !SearchJoint( MODEL_SPEARSKILL, o, 9 ) )
				{
					for(int i = 0; i < 5; i++)
					{
						CreateJoint( MODEL_SPEARSKILL, o->Position, o->Position, o->Angle, 4, o, 20.0f, -1, 0, 0, c->TargetCharacter);
					}
				}
            }
		}
		else if( g_isCharacterBuff(o, eBuff_Defense) )
		{
			if( !g_isCharacterBuff(o, eBuff_Cloaking) )
			{
				if(SearchJoint(MODEL_SPEARSKILL, o, 4) == false && SearchJoint(MODEL_SPEARSKILL, o, 9) == false)
				{
					for(int i=0; i<5; ++i)
					{
						CreateJoint(MODEL_SPEARSKILL, o->Position, o->Position, o->Angle, 4, o, 20.0f, -1, 0, 0, c->TargetCharacter);
					}
				}
			}
		}

#ifdef CSK_LUCKY_SEAL
#if SELECTED_LANGUAGE != LANGUAGE_JAPANESE

		if( g_isCharacterBuff((&c->Object), eBuff_PcRoomSeal1) 
		 || g_isCharacterBuff((&c->Object), eBuff_PcRoomSeal2) 
		 || g_isCharacterBuff((&c->Object), eBuff_PcRoomSeal3)
		 || g_isCharacterBuff((&c->Object), eBuff_Seal1) 
		 || g_isCharacterBuff((&c->Object), eBuff_Seal2) 
		 || g_isCharacterBuff((&c->Object), eBuff_Seal3)
		 || g_isCharacterBuff((&c->Object), eBuff_Seal4) )
		{			
			if(SearchJoint(MODEL_SPEARSKILL, o, 10)  == false
				&& SearchJoint(MODEL_SPEARSKILL, o, 11)  == false)
			{
				for(int i=0; i<3; ++i)
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
#endif //SELECTED_LANGUAGE != LANGUAGE_JAPANESE

#endif // CSK_LUCKY_SEAL
		if(g_isCharacterBuff((&c->Object), eBuff_Thorns))
		{
			if(SearchJoint(BITMAP_FLARE, o, 43) == false)
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
		
#ifdef ASG_ADD_SKILL_BERSERKER
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
						CreateEffect(BITMAP_SPARK+2, o->Position, o->Angle, vLight[1], i, o);
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
					DeleteEffect(BITMAP_SPARK+2, o, i);
			}
			DeleteEffect(BITMAP_LIGHT_MARKS, o);
		}
#endif	// ASG_ADD_SKILL_BERSERKER
		
		if(g_isCharacterBuff((&c->Object), eDeBuff_Blind))
		{
			if(SearchEffect(MODEL_ALICE_BUFFSKILL_EFFECT, o, 3) == false)
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
		
		if(g_isCharacterBuff((&c->Object), eDeBuff_Sleep))
		{
			if(SearchEffect(MODEL_ALICE_BUFFSKILL_EFFECT, o, 4) == false)
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
			if (!SearchEffect(BITMAP_SHINY+6, o, 1))
			{
				vec3_t vLight;
				Vector(1.4f, 0.2f, 0.2f, vLight);
				CreateEffect(BITMAP_SHINY+6, o->Position, o->Angle, vLight, 1, o, -1, 0, 0, 0, 0.5f);
				CreateEffect(BITMAP_PIN_LIGHT, o->Position, o->Angle, vLight, 1, o, -1, 0, 0, 0, 1.f);
			}
		}
		else
		{
			DeleteEffect(BITMAP_SHINY+6, o, 1);
			DeleteEffect(BITMAP_PIN_LIGHT, o, 1);
		}

		if (g_isCharacterBuff((&c->Object), eDeBuff_DefenseDown))
		{
			if (!SearchEffect(BITMAP_SHINY+6, o, 2))
			{
				vec3_t vLight;
				Vector(0.25f, 1.0f, 0.7f, Light);
				CreateEffect(BITMAP_SHINY+6, o->Position, o->Angle, vLight, 2, o, -1, 0, 0, 0, 0.5f);
				CreateEffect(BITMAP_PIN_LIGHT, o->Position, o->Angle, vLight, 2, o, -1, 0, 0, 0, 1.f);
			}
		}
		else
		{
			DeleteEffect(BITMAP_SHINY+6, o, 2);
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

#ifdef KJH_FIX_SWELLOFMAGIC_EFFECT
#ifdef KJH_ADD_SKILL_SWELL_OF_MAGICPOWER
		if(g_isCharacterBuff((&c->Object), eBuff_SwellOfMagicPower))
		{
			if( !g_isCharacterBuff((&c->Object), eBuff_Cloaking) )
			{
				if( !SearchEffect(MODEL_SWELL_OF_MAGICPOWER_BUFF_EFF, o, 0) )
				{
					vec3_t vLight;
					Vector(0.7f, 0.2f, 0.9f, vLight);
					CreateEffect( MODEL_SWELL_OF_MAGICPOWER_BUFF_EFF, o->Position, o->Angle, vLight, 0, o );
				}
			}
		}
		else
		{
			DeleteEffect(MODEL_SWELL_OF_MAGICPOWER_BUFF_EFF, o, 0);
		}
#endif // KJH_ADD_SKILL_SWELL_OF_MAGICPOWER
#endif // KJH_FIX_SWELLOFMAGIC_EFFECT

#ifndef GUILD_WAR_EVENT
        if ( InChaosCastle()==false )
#endif GUILD_WAR_EVENT
        {
            //  세트 옵션 풀세트.
            if ( c->ExtendState )
            {
#ifdef LDS_FIX_SETITEMEFFECT_WHENFULLSET_SOMEMISSEDEFFECT				
				if ( (	o->CurrentAction<PLAYER_WALK_MALE ||
#ifdef YDG_FIX_DARKLORD_SET_EFFECT_WITH_DARKSPIRIT
					o->CurrentAction==PLAYER_DARKLORD_STAND ||
#endif	// YDG_FIX_DARKLORD_SET_EFFECT_WITH_DARKSPIRIT
					o->CurrentAction==PLAYER_STOP_RIDE_HORSE ||
					o->CurrentAction==PLAYER_STOP_TWO_HAND_SWORD_TWO 
#ifdef PBG_FIX_BUFFSKILLCHAOS
					|| o->CurrentAction == PLAYER_STOP_RAGEFIGHTER
#endif //PBG_FIX_BUFFSKILLCHAOS
					) &&
					c->ExtendStateTime>=100 )
#else	// LDS_FIX_SETITEMEFFECT_WHENFULLSET_SOMEMISSEDEFFECT
					if ( ( o->CurrentAction<PLAYER_WALK_MALE ) && c->ExtendStateTime>=100 )
#endif	// LDS_FIX_SETITEMEFFECT_WHENFULLSET_SOMEMISSEDEFFECT
                {
                    Vector ( 0.2f, 0.7f, 0.9f, Light );
                    CreateEffect ( BITMAP_LIGHT, o->Position, o->Angle, Light, 3, o );

                    c->ExtendStateTime = 0;
                }
                c->ExtendStateTime++;
            }
            if ( fullset )
			{
                PartObjectColor(c->BodyPart[5].Type,o->Alpha,0.5f,Light);

			if(!g_isCharacterBuff(o, eBuff_Cloaking))
			{
				for(int i=0;i<2;i++)
				{
					b->TransformPosition(o->BoneTransform[c->Weapon[i].LinkBone],p,Position,true);
					CreateSprite(BITMAP_LIGHT,Position,1.3f,Light,o);
					b->TransformPosition(o->BoneTransform[c->Weapon[i].LinkBone-6],p,Position,true);
					CreateSprite(BITMAP_LIGHT,Position,1.3f,Light,o);
					b->TransformPosition(o->BoneTransform[c->Weapon[i].LinkBone-7],p,Position,true);
					CreateSprite(BITMAP_LIGHT,Position,1.3f,Light,o);
				}
			}
#ifdef ASG_ADD_STORMBLITZ_380ITEM
			if ((c->BodyPart[BODYPART_BOOTS].Type >= MODEL_BOOTS+29 && c->BodyPart[BODYPART_BOOTS].Type <= MODEL_BOOTS+33)
				|| c->BodyPart[BODYPART_BOOTS].Type == MODEL_BOOTS+43)
#else	// ASG_ADD_STORMBLITZ_380ITEM
			if(c->BodyPart[BODYPART_BOOTS].Type >= MODEL_BOOTS+29 && c->BodyPart[BODYPART_BOOTS].Type <= MODEL_BOOTS+33)
#endif	// ASG_ADD_STORMBLITZ_380ITEM
			{
                if ( EquipmentLevelSet>9 )
                {
                    VectorCopy ( o->Light, Light );

					if(c->BodyPart[BODYPART_BOOTS].Type == MODEL_BOOTS+29) Vector ( 0.65f, 0.3f, 0.1f, o->Light );
					if(c->BodyPart[BODYPART_BOOTS].Type == MODEL_BOOTS+30) Vector ( 0.1f, 0.1f, 0.9f, o->Light );
					if(c->BodyPart[BODYPART_BOOTS].Type == MODEL_BOOTS+31) Vector ( 0.0f, 0.32f, 0.24f, o->Light );
					if(c->BodyPart[BODYPART_BOOTS].Type == MODEL_BOOTS+32) Vector ( 0.5f, 0.24f, 0.8f, o->Light );
					if(c->BodyPart[BODYPART_BOOTS].Type == MODEL_BOOTS+33) Vector ( 0.6f, 0.4f, 0.0f, o->Light );
#ifdef ASG_ADD_STORMBLITZ_380ITEM
					if (c->BodyPart[BODYPART_BOOTS].Type == MODEL_BOOTS+43)
						Vector(0.6f, 0.3f, 0.4f, o->Light);
#endif	// ASG_ADD_STORMBLITZ_380ITEM
                    if ( EquipmentLevelSet==10 )
                    {
						if((rand()%4)==0)
						{
							Vector(0.0f, -18.0f, 50.0f, p);
							b->TransformPosition(o->BoneTransform[0],p,Position,true);
							CreateParticle(BITMAP_WATERFALL_2, Position, o->Angle, o->Light, 3);
							Vector(0.0f, 0.0f, 70.0f, p);
							b->TransformPosition(o->BoneTransform[0],p,Position,true);
							CreateParticle(BITMAP_WATERFALL_2, Position, o->Angle, o->Light, 3);
							Vector(0.0f, 18.0f, 50.0f, p);
							b->TransformPosition(o->BoneTransform[0],p,Position,true);
							CreateParticle(BITMAP_WATERFALL_2, Position, o->Angle, o->Light, 3);
						}
                    }
                    else if ( EquipmentLevelSet==11 )
                    {
						if((rand()%3)==0)
						{
							Vector(0.0f, -18.0f, 50.0f, p);
							b->TransformPosition(o->BoneTransform[0],p,Position,true);
							CreateParticle(BITMAP_WATERFALL_2, Position, o->Angle, o->Light, 3);
							Vector(0.0f, 0.0f, 70.0f, p);
							b->TransformPosition(o->BoneTransform[0],p,Position,true);
							CreateParticle(BITMAP_WATERFALL_2, Position, o->Angle, o->Light, 3);
							Vector(0.0f, 18.0f, 50.0f, p);
							b->TransformPosition(o->BoneTransform[0],p,Position,true);
							CreateParticle(BITMAP_WATERFALL_2, Position, o->Angle, o->Light, 3);
						}
                    }
                    else if ( EquipmentLevelSet==12 )
                    {
						if((rand()%2)==0)
						{
							Vector(0.0f, -18.0f, 50.0f, p);
							b->TransformPosition(o->BoneTransform[0],p,Position,true);
							CreateParticle(BITMAP_WATERFALL_2, Position, o->Angle, o->Light, 3);
							Vector(0.0f, 0.0f, 70.0f, p);
							b->TransformPosition(o->BoneTransform[0],p,Position,true);
							CreateParticle(BITMAP_WATERFALL_2, Position, o->Angle, o->Light, 3);
							Vector(0.0f, 18.0f, 50.0f, p);
							b->TransformPosition(o->BoneTransform[0],p,Position,true);
							CreateParticle(BITMAP_WATERFALL_2, Position, o->Angle, o->Light, 3);
						}
                    }
                    else if ( EquipmentLevelSet==13 )
                    {
						Vector(0.0f, -20.0f, 50.0f, p);
						b->TransformPosition(o->BoneTransform[0],p,Position,true);
						CreateParticle(BITMAP_WATERFALL_2, Position, o->Angle, o->Light, 3);
						Vector(0.0f, 0.0f, 70.0f, p);
						b->TransformPosition(o->BoneTransform[0],p,Position,true);
						CreateParticle(BITMAP_WATERFALL_2, Position, o->Angle, o->Light, 3);
						Vector(0.0f, 20.0f, 50.0f, p);
						b->TransformPosition(o->BoneTransform[0],p,Position,true);
						CreateParticle(BITMAP_WATERFALL_2, Position, o->Angle, o->Light, 3);
                    }
#ifdef LDK_ADD_14_15_GRADE_ITEM_SET_EFFECT
                    else if ( EquipmentLevelSet==14 )
                    {
						Vector(0.0f, -20.0f, 50.0f, p);
						b->TransformPosition(o->BoneTransform[0],p,Position,true);
						CreateParticle(BITMAP_WATERFALL_2, Position, o->Angle, o->Light, 3);
						Vector(0.0f, 0.0f, 70.0f, p);
						b->TransformPosition(o->BoneTransform[0],p,Position,true);
						CreateParticle(BITMAP_WATERFALL_2, Position, o->Angle, o->Light, 3);
						Vector(0.0f, 20.0f, 50.0f, p);
						b->TransformPosition(o->BoneTransform[0],p,Position,true);
						CreateParticle(BITMAP_WATERFALL_2, Position, o->Angle, o->Light, 3);
                    }
                    else if ( EquipmentLevelSet==15 )
                    {
						Vector(0.0f, -20.0f, 50.0f, p);
						b->TransformPosition(o->BoneTransform[0],p,Position,true);
						CreateParticle(BITMAP_WATERFALL_2, Position, o->Angle, o->Light, 3);
						Vector(0.0f, 0.0f, 70.0f, p);
						b->TransformPosition(o->BoneTransform[0],p,Position,true);
						CreateParticle(BITMAP_WATERFALL_2, Position, o->Angle, o->Light, 3);
						Vector(0.0f, 20.0f, 50.0f, p);
						b->TransformPosition(o->BoneTransform[0],p,Position,true);
						CreateParticle(BITMAP_WATERFALL_2, Position, o->Angle, o->Light, 3);
                    }
#endif //LDK_ADD_14_15_GRADE_ITEM_SET_EFFECT
                    VectorCopy ( Light, o->Light );
                }
			}
                if ( EquipmentLevelSet>9 )
                {
                    if ( (rand()%20)==0 )//(o->CurrentAction<PLAYER_WALK_MALE || o->CurrentAction>PLAYER_RUN_RIDE_WEAPON) && (rand()%6)==0)
                    {
                        VectorCopy ( o->Light, Light );
                        Vector ( 1.f, 1.f, 1.f, o->Light );

                        if ( EquipmentLevelSet==10 )
                        {
		                    CreateParticle(BITMAP_FLARE,o->Position,o->Angle,o->Light,0,0.19f,o);
                        }
                        else if ( EquipmentLevelSet==11 )
                        {
                            if((rand()%8)==0)
                            {
                                CreateJoint(BITMAP_FLARE,o->Position,o->Position,o->Angle,0,o);
                            }
                            else
                            {
		                        CreateParticle(BITMAP_FLARE,o->Position,o->Angle,o->Light,0,0.19f,o);
                            }
                        }
                        else if ( EquipmentLevelSet==12 )
                        {
                            if((MoveSceneFrame%6)==0)
                            {
                                CreateJoint ( BITMAP_FLARE, o->Position, o->Position, o->Angle, 18, o, 20, -1, 0 );
                                CreateJoint ( BITMAP_FLARE, o->Position, o->Position, o->Angle, 18, o, 20, -1, 1 );
                            }
                            else if ( (MoveSceneFrame%3)==0 )
                            {
		                        CreateParticle ( BITMAP_FLARE, o->Position, o->Angle, o->Light, 0, 0.19f, o);
                            }
                        }
                        else if ( EquipmentLevelSet==13 )
                        {
                            if((MoveSceneFrame%6)==0)
                            {
                        
                                CreateJoint ( BITMAP_FLARE, o->Position, o->Position, o->Angle, 18, o, 20, -1, 0 );
                                CreateJoint ( BITMAP_FLARE, o->Position, o->Position, o->Angle, 18, o, 20, -1, 1 );
                            }
                        
                            if ( (MoveSceneFrame%4)==0 )
                            {
		                        CreateParticle ( BITMAP_FLARE, o->Position, o->Angle, o->Light, 0, 0.19f, o);
                                CreateJoint ( BITMAP_FLARE+1, o->Position, o->Position, o->Angle, 7, o, 20, 40, 1 );
                            }
                        }
#ifdef LDK_ADD_14_15_GRADE_ITEM_SET_EFFECT
                        else if ( EquipmentLevelSet==14 )
                        {
                            if((MoveSceneFrame%6)==0)
                            {
								
                                CreateJoint ( BITMAP_FLARE, o->Position, o->Position, o->Angle, 18, o, 20, -1, 0 );
                                CreateJoint ( BITMAP_FLARE, o->Position, o->Position, o->Angle, 18, o, 20, -1, 1 );
                            }
							
                            if ( (MoveSceneFrame%4)==0 )
                            {
								CreateParticle ( BITMAP_FLARE, o->Position, o->Angle, o->Light, 0, 0.19f, o);
                                CreateJoint ( BITMAP_FLARE+1, o->Position, o->Position, o->Angle, 7, o, 20, 40, 1 );
                            }
                        }
                        else if ( EquipmentLevelSet==15 )
                        {
                            if((MoveSceneFrame%6)==0)
                            {
								
                                CreateJoint ( BITMAP_FLARE, o->Position, o->Position, o->Angle, 18, o, 20, -1, 0 );
                                CreateJoint ( BITMAP_FLARE, o->Position, o->Position, o->Angle, 18, o, 20, -1, 1 );
                            }
							
                            if ( (MoveSceneFrame%4)==0 )
                            {
								CreateParticle ( BITMAP_FLARE, o->Position, o->Angle, o->Light, 0, 0.19f, o);
                                CreateJoint ( BITMAP_FLARE+1, o->Position, o->Position, o->Angle, 7, o, 20, 40, 1 );
                            }
                        }
#endif //LDK_ADD_14_15_GRADE_ITEM_SET_EFFECT

                        VectorCopy ( Light, o->Light );
                    }

	#ifdef LDK_ADD_14_15_GRADE_ITEM_SET_EFFECT
					if ( EquipmentLevelSet==15 )
					{
						//left
						vec3_t vColor;
						Vector(13.0f, 10.0f, 3.0f, p);
						b->TransformPosition(o->BoneTransform[20],p,Position,true);
						VectorCopy(Position, o->EyeLeft);

						Vector(0.09f, 0.09f, 0.8f, vColor);
						CreateJoint(BITMAP_JOINT_ENERGY,Position,o->Position,o->Angle,55,o,6.0f,-1,0,0,-1, vColor);
						//안광
						float fRad = (float)sinf((WorldTime)*0.002f);
						Vector(0.2f, 0.4f, 0.8f, vColor);
						CreateSprite(BITMAP_SHINY+6, Position, 0.5f*fRad, vColor, o);

						Vector(0.1f, 0.15f, 1.0f, vColor);
						CreateSprite(BITMAP_PIN_LIGHT, Position, 1.3f*fRad+0.5f, vColor, o, 100.0f);
						
						// right
						Vector(13.0f, 10.0f, -3.0f, p);
						b->TransformPosition(o->BoneTransform[20],p,Position,true);
						VectorCopy(Position, o->EyeRight);

						Vector(0.09f, 0.09f, 0.8f, vColor);
						CreateJoint(BITMAP_JOINT_ENERGY,Position,o->Position,o->Angle,56,o,6.0f,-1,0,0,-1, vColor);

						Vector(0.2f, 0.4f, 0.8f, vColor);
						CreateSprite(BITMAP_SHINY+6, Position, 0.5f*fRad, vColor, o);

						Vector(0.1f, 0.15f, 1.0f, vColor);
						CreateSprite(BITMAP_PIN_LIGHT, Position, 1.3f*fRad+0.5f, vColor, o, 80.0f);
					}
	#endif //LDK_ADD_14_15_GRADE_ITEM_SET_EFFECT
                }
    		}
        }
		break;
	case MODEL_MONSTER01:
	case MODEL_MONSTER01+30:
		if((o->Type==MODEL_MONSTER01&&c->Level==1) || (o->Type==MODEL_MONSTER01+30))
			RenderEye(o,22,23);
		break;
	case MODEL_MONSTER01+52:
		RenderEye(o,26,27,2.0f);
		break;
	case MODEL_MONSTER01+37:
        RenderLight(o,BITMAP_LIGHTNING+1,1.f,63,0.f,0.f,20.f);
        RenderLight(o,BITMAP_SHINY+2    ,4.f,63,0.f,0.f,20.f);
		break;
	case MODEL_MONSTER01+34:
        RenderLight(o,BITMAP_LIGHTNING+1,0.5f,30,0.f,0.f,-5.f);
        RenderLight(o,BITMAP_LIGHTNING+1,0.5f,39,0.f,0.f,-5.f);
        RenderLight(o,BITMAP_SPARK      ,4.f ,30,0.f,0.f,-5.f);
        RenderLight(o,BITMAP_SPARK      ,4.f ,39,0.f,0.f,-5.f);
        RenderLight(o,BITMAP_SHINY+2    ,2.f ,30,0.f,0.f,-5.f);
        RenderLight(o,BITMAP_SHINY+2    ,2.f ,39,0.f,0.f,-5.f);
		break;
	case MODEL_MONSTER01+36:
		RenderEye(o,42,43);
        RenderLight(o,BITMAP_SPARK  ,2.f,26,0.f,0.f,0.f);
        RenderLight(o,BITMAP_SPARK  ,2.f,31,0.f,0.f,0.f);
        RenderLight(o,BITMAP_SPARK  ,2.f,36,0.f,0.f,0.f);
        RenderLight(o,BITMAP_SPARK  ,2.f,41,0.f,0.f,0.f);
        RenderLight(o,BITMAP_SHINY+2,1.f,26,0.f,0.f,0.f);
        RenderLight(o,BITMAP_SHINY+2,1.f,31,0.f,0.f,0.f);
        RenderLight(o,BITMAP_SHINY+2,1.f,36,0.f,0.f,0.f);
        RenderLight(o,BITMAP_SHINY+2,1.f,41,0.f,0.f,0.f);
		break;
	case MODEL_MONSTER01+33:
        RenderLight(o,BITMAP_SPARK  ,4.f,9,0.f,0.f,5.f);
        RenderLight(o,BITMAP_SHINY+2,3.f,9,0.f,0.f,5.f);
		break;
	case MODEL_MIX_NPC:
        RenderLight(o,BITMAP_LIGHT,1.5f,32,0.f,0.f,0.f);
		break;
    case MODEL_NPC_SEVINA:
        RenderLight(o,BITMAP_LIGHT,2.5f,6,0.f,0.f,0.f);
        break;
	case MODEL_NPC_DEVILSQUARE:
        {
            Luminosity = (float)sinf((WorldTime)*0.002f)*0.35f+0.65f;

            float Scale = 0.3f;
			Vector(Luminosity,Luminosity,Luminosity,Light);

            Vector( 3.5f,-12.f,10.f,p );
			b->TransformPosition(o->BoneTransform[20],p,Position,1);
			CreateSprite(BITMAP_LIGHTNING+1,Position,Scale,Light,o,(WorldTime/50.0f));
            CreateSprite(BITMAP_LIGHTNING+1,Position,Scale,Light,o,((-WorldTime)/50.0f));

            if ( rand()%30==0 )
            {
                p[0] = Position[0] + rand()%100-50;
                p[1] = Position[1] + rand()%100-50;
                p[2] = Position[2] + rand()%100-50;

      			CreateJoint(BITMAP_JOINT_ENERGY,p,Position,o->Angle,6,NULL,20.f);
            }
        }
		break;
	case MODEL_NPC_CASTEL_GATE:
		{
			vec3_t vPos, vRelative;
			float fLumi, fScale;

			fLumi = (sinf(WorldTime*0.002f) + 2.0f) * 0.5f;
			Vector(fLumi*1.0f, fLumi*0.5f, fLumi*0.3f, Light);
			fScale = fLumi/2.0f;

			Vector(4.0f,  3.0f, -4.0f, vRelative);
			b->TransformPosition(o->BoneTransform[2], vRelative, vPos, true);

			CreateSprite(BITMAP_LIGHT, vPos, fScale, Light, o);
			CreateSprite(BITMAP_KANTURU_2ND_EFFECT1, vPos, fScale, Light, o);

			Vector(5.0f,  3.0f, 2.0f, vRelative);
			b->TransformPosition(o->BoneTransform[4], vRelative, vPos, true);

			CreateSprite(BITMAP_LIGHT, vPos, fScale, Light, o);
			CreateSprite(BITMAP_KANTURU_2ND_EFFECT1, vPos, fScale, Light, o);

			if (rand() % 4 == 0)
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
	case MODEL_MONSTER01+28:
		Vector(0.f,0.f,0.f,p);
		Luminosity = 1.f;
		if(c->Level == 0)
		{
			Vector(Luminosity*1.f,Luminosity*1.f,Luminosity*1.f,Light);
		}
		else
		{
			Vector(Luminosity*1.f,Luminosity*0.4f,Luminosity*0.f,Light);
			Vector(Luminosity*0.2f,Luminosity*0.7f,Luminosity*0.1f,Light);
		}

		for(int i=0;i<b->NumBones;i++)
		{
			if(!b->Bones[i].Dummy)
			{
				if((i>=15&&i<=20) || (i>=27&&i<=32))
				{
				}
				else
				{
					b->TransformPosition(o->BoneTransform[i],p,Position,true);
					if(c->Level == 0)
						CreateSprite(BITMAP_SHINY+1,Position,2.5f,Light,o,0.f,1);
					else
						CreateSprite(BITMAP_MAGIC+1,Position,0.8f,Light,o,0.f);
					if(rand()%4==0 && o->CurrentAction>=MONSTER01_ATTACK1 && o->CurrentAction<=MONSTER01_ATTACK2)
					{
						CreateParticle(BITMAP_ENERGY,Position,o->Angle,Light);
					}
					//CreateParticle(BITMAP_SMOKE,Position,o->Angle,Light,1);
				}
			}
		}
		break;
#ifdef ADD_SOCKET_MIX
	case MODEL_SEED_MASTER:
		{
			float fLumi, fScale;
			fScale = 2.0f;
			b->TransformByObjectBone(Position, o, 39);

			fLumi = (sinf(WorldTime*0.001f) + 1.0f) * 0.3f + 0.4f;
			Vector(fLumi*0.5f, fLumi*0.5f, fLumi*0.5f, Light);
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

				fLumi = (sinf(WorldTime*0.001f) + 1.0f) * 0.3f + 0.4f;
				Vector(fLumi*0.5f, fLumi*0.5f, fLumi*0.5f, Light);
				CreateSprite(BITMAP_FLARE_BLUE, Position, fScale, Light, o);
			}
		}
		break;
#endif	// ADD_SOCKET_MIX
    default :
        {
            if ( RenderHellasMonsterVisual( c, o, b ) ) break;
            if ( battleCastle::RenderBattleCastleMonsterVisual( c, o, b ) ) break;
			if( M31HuntingGround::RenderHuntingGroundMonsterVisual( c, o, b) ) break;
#ifdef CRYINGWOLF_2NDMVP
			if( M34CryingWolf2nd::RenderCryingWolf2ndMonsterVisual( c, o, b) ) break;
#endif // CRYINGWOLF_2NDMVP
			if( M34CryWolf1st::RenderCryWolf1stMonsterVisual( c, o, b) ) break;
			if( M33Aida::RenderAidaMonsterVisual( c, o, b) ) break;
			if(M37Kanturu1st::RenderKanturu1stMonsterVisual(c, o, b)) break;
			if(M38Kanturu2nd::Render_Kanturu2nd_MonsterVisual(c, o, b)) break;
			if( M39Kanturu3rd::RenderKanturu3rdMonsterVisual( c, o, b) ) break;
			if (SEASON3A::CGM3rdChangeUp::Instance().RenderBalgasBarrackMonsterVisual(c, o, b))	break;
			if( g_CursedTemple->RenderMonsterVisual( c, o, b ) ) break;
			if( SEASON3B::GMNewTown::RenderMonsterVisual( c, o, b ) ) break;
			if( SEASON3C::GMSwampOfQuiet::RenderMonsterVisual( c, o, b ) ) break;
#ifdef PSW_ADD_MAPSYSTEM
			if( TheMapProcess().RenderMonsterVisual( c, o, b ) == true ) break;
#endif //PSW_ADD_MAPSYSTEM
        }
	}

	if( SceneFlag == MAIN_SCENE )
	{
		if( (o->Kind == KIND_PLAYER 
			&& InBloodCastle() == false
			&& InChaosCastle() == false
			&& InBattleCastle()) == false )
		{
			RenderProtectGuildMark( c );
		}
	}
#ifdef PET_SYSTEM
    giPetManager::RenderPet ( c );
#endif// PET_SYSTEM

	if (GetBaseClass(c->Class) == CLASS_SUMMONER)
	{
		PART_t *w = &c->Weapon[1];
		g_SummonSystem.MoveEquipEffect(c, w->Type, w->Level, w->Option1);
	}
}

void RenderCharactersClient()
{
#ifdef DO_PROFILING
	g_pProfiler->BeginUnit( EPROFILING_RENDER_CHARACTERSCLIENT, PROFILING_RENDER_CHARACTERSCLIENT );
#endif // DO_PROFILING
	
#ifdef MR0
	VPManager::Enable();
#endif//MR0
	for ( int i=0; i<MAX_CHARACTERS_CLIENT; ++i )
	{
		CHARACTER *c = &CharactersClient[i];
		OBJECT *o = &c->Object;

		if ( c!=Hero && battleCastle::IsBattleCastleStart()==true && g_isCharacterBuff(o, eBuff_Cloaking) )
		{

			if ( ( Hero->EtcPart==PARTS_ATTACK_KING_TEAM_MARK || Hero->EtcPart==PARTS_ATTACK_TEAM_MARK ))
			{
				if(!(c->EtcPart==PARTS_ATTACK_KING_TEAM_MARK || c->EtcPart==PARTS_ATTACK_TEAM_MARK ))
				{
					if(Hero->TargetCharacter == c->Key)
						Hero->TargetCharacter = -1;
					continue;
				}
			}
			else
			if ( ( Hero->EtcPart==PARTS_ATTACK_KING_TEAM_MARK2 || Hero->EtcPart==PARTS_ATTACK_TEAM_MARK2 ))
			{
				if(!(c->EtcPart==PARTS_ATTACK_KING_TEAM_MARK2 || c->EtcPart==PARTS_ATTACK_TEAM_MARK2 ))
				{
					if(Hero->TargetCharacter == c->Key)
						Hero->TargetCharacter = -1;
					continue;
				}
			}
			else
			if ( ( Hero->EtcPart==PARTS_ATTACK_KING_TEAM_MARK3 || Hero->EtcPart==PARTS_ATTACK_TEAM_MARK3 ))
			{
				if(!(c->EtcPart==PARTS_ATTACK_KING_TEAM_MARK3 || c->EtcPart==PARTS_ATTACK_TEAM_MARK3 ))
				{
					if(Hero->TargetCharacter == c->Key)
						Hero->TargetCharacter = -1;
					continue;
				}
			}
			else
			if(Hero->EtcPart==PARTS_DEFENSE_KING_TEAM_MARK || Hero->EtcPart==PARTS_DEFENSE_TEAM_MARK)
			{
				if(!(c->EtcPart==PARTS_DEFENSE_KING_TEAM_MARK || c->EtcPart==PARTS_DEFENSE_TEAM_MARK ))
				{
					if(Hero->TargetCharacter == c->Key)
						Hero->TargetCharacter = -1;
					continue;
				}
			}
		}
		if ( c==Hero && ( 0x04 & Hero->CtlCode) && SceneFlag==MAIN_SCENE )
		{
			o->OBB.StartPos[0] = 1000.0f;
			o->OBB.XAxis[0] = o->OBB.YAxis[1] = o->OBB.ZAxis[2] = 1.0f;
			continue;
		}
		if ( o->Live )
		{
			if ( o->Visible )
			{
#ifdef STATE_LIMIT_TIME
                if ( c==Hero )
                {
                    SetWorldStateTime ( o->State, 60000 );
                }
#endif// STATE_LIMIT_TIME
#ifndef PJH_NEW_SERVER_SELECT_MAP
				if (World == WD_77NEW_LOGIN_SCENE)
				{
					float fDistance_x = CameraPosition[0] - o->Position[0];
					float fDistance_y = CameraPosition[1] - o->Position[1];
					float fDistance = sqrtf(fDistance_x * fDistance_x + fDistance_y * fDistance_y);
					if (!TestFrustrum2D(o->Position[0]*0.01f,o->Position[1]*0.01f,-100.f) || fDistance > 3800.f)	// 컬링 최적화 가능한 부분☆
						continue;
				}
#endif //PJH_NEW_SERVER_SELECT_MAP
				if ( i!=SelectedCharacter && i!=SelectedNpc )
     				RenderCharacter ( c, o );
				else
     				RenderCharacter ( c, o, true );

                if ( o->Type==MODEL_PLAYER )
                    battleCastle::CreateBattleCastleCharacter_Visual ( c, o );

			}
		}
	}

	if ( InBattleCastle() || World == WD_31HUNTING_GROUND )
	{
		battleCastle::InitEtcSetting ();
	}

#ifdef MR0
	VPManager::Disable();
#endif//MR0

#ifdef DO_PROFILING
	g_pProfiler->EndUnit( EPROFILING_RENDER_CHARACTERSCLIENT );
#endif // DO_PROFILING
}

void RenderProtectGuildMark(CHARACTER * c)
{
	if( c->GuildType == GT_ANGEL )
	{
		if( c->ProtectGuildMarkWorldTime == 0 || WorldTime - c->ProtectGuildMarkWorldTime > 5000.0f)
		{
			OBJECT * o = &c->Object;
			CreateEffect(MODEL_PROTECTGUILD, o->Position, o->Angle, o->Light, 0, o);
			c->ProtectGuildMarkWorldTime = WorldTime;
		}
	}
}

void ClearCharacters(int Key)
{
	for(int i=0;i<MAX_CHARACTERS_CLIENT;i++)
	{
      	CHARACTER *c = &CharactersClient[i];
		OBJECT *o = &c->Object;
		if(o->Live && c->Key != Key)
		{
	   		o->Live = false;

			BoneManager::UnregisterBone(c);

			for(int j=0;j<MAX_BUTTERFLES;j++)
			{
				OBJECT *b = &Butterfles[j];
				if(b->Live && b->Owner == o)
					b->Live = false;
			}
		}

        DeletePet ( c );
		DeleteCloth( c, o);
        DeleteParts ( c );
	}
}


void DeleteCharacter(int Key)
{
	for(int i=0;i<MAX_CHARACTERS_CLIENT;i++)
	{
      	CHARACTER *c = &CharactersClient[i];
		OBJECT *o = &c->Object;
		if(o->Live && c->Key == Key)
		{
			o->Live = false;

			BoneManager::UnregisterBone(c);

			for(int j=0;j<MAX_BUTTERFLES;j++)
			{
				OBJECT *b = &Butterfles[j];
				if(b->Live && b->Owner == o)
					b->Live = false;
			}
            DeletePet ( c );
			DeleteCloth( c, o);
            DeleteParts ( c );
			return;
		}
	}
}

void DeleteCharacter ( CHARACTER* c, OBJECT* o )
{
    o->Live = false;

    BoneManager::UnregisterBone ( c );

    for(int j=0;j<MAX_BUTTERFLES;j++)
    {
        OBJECT *b = &Butterfles[j];
        if(b->Live && b->Owner == o)
            b->Live = false;
    }
#ifdef PET_SYSTEM
    DeletePet ( c );
#endif// PET_SYSTEM
    DeleteCloth( c, o);
    DeleteParts ( c );
}

int FindCharacterIndex(int Key)
{
	for(int i=0;i<MAX_CHARACTERS_CLIENT;i++)
	{
      	CHARACTER *c = &CharactersClient[i];
		if(c->Object.Live && c->Key==Key)
		{
			return i;
		}
	}
	return MAX_CHARACTERS_CLIENT;
}       

int FindCharacterIndexByMonsterIndex ( int Type )
{
	for(int i=0;i<MAX_CHARACTERS_CLIENT;i++)
	{
      	CHARACTER *c = &CharactersClient[i];
		if ( c->Object.Live && c->MonsterIndex==Type )
		{
			return i;
		}
	}
	return MAX_CHARACTERS_CLIENT;
}

int HangerBloodCastleQuestItem (int Key)
{
    int index = MAX_CHARACTERS_CLIENT;
	for(int i=0;i<MAX_CHARACTERS_CLIENT;i++)
	{
      	CHARACTER *c = &CharactersClient[i];
		if(c->Object.Live && c->Key==Key)
		{
			index = i;
		}
        c->EtcPart = 0;
	}
	return index;
}

void SetAllAction ( int Action )
{
	for ( int i=0; i<MAX_CHARACTERS_CLIENT; i++ )
	{
      	CHARACTER *c = &CharactersClient[i];
		if ( c->Object.Live && c->Object.Type==MODEL_PLAYER )
		{
            Vector ( 0.f, 0.f, 180.f, c->Object.Angle );
            SetAction ( &c->Object, Action );
		}
	}
}

void ReleaseCharacters(void)
{
	for(int i=0;i<MAX_CHARACTERS_CLIENT;i++)
	{
      	CHARACTER *c = &CharactersClient[i];
		OBJECT *o = &c->Object;
		if(o->BoneTransform != NULL)
		{
			delete[] o->BoneTransform;
			o->BoneTransform = NULL;
		}
        DeletePet ( c );
		DeleteCloth( c, o);
        DeleteParts ( c );
	}
	OBJECT *o = &CharacterView.Object;
	if(o->BoneTransform != NULL)
	{
		delete[] o->BoneTransform;
		o->BoneTransform = NULL;
	}
    DeletePet ( &CharacterView );
    DeleteCloth( &CharacterView, o);
    DeleteParts ( &CharacterView );

	BoneManager::UnregisterAll();
}

void CreateCharacterPointer(CHARACTER *c,int Type,unsigned char PositionX,unsigned char PositionY,float Rotation)
{
	OBJECT *o = &c->Object;
#if defined MR0 || defined OBJECT_ID
	o->m_iID = GenID();
#endif //MR0
	c->PositionX      = PositionX;
	c->PositionY      = PositionY;
	c->TargetX      = PositionX;
	c->TargetY      = PositionY;
	if (c != Hero)
	{
		c->byExtensionSkill = 0;
	}
    c->m_pParts = NULL;

#ifdef YDG_FIX_MEMORY_LEAK_0905_2ND
	giPetManager::DeletePet(c);
#else	// YDG_FIX_MEMORY_LEAK_0905_2ND
    c->m_pPet = NULL;
#endif	// YDG_FIX_MEMORY_LEAK_0905_2ND

	int Index = TERRAIN_INDEX_REPEAT(( c->PositionX),( c->PositionY));
	if((TerrainWall[Index]&TW_SAFEZONE)==TW_SAFEZONE)
		c->SafeZone = true;
	else
		c->SafeZone = false;
	
	c->Path.PathNum     = 0;
	c->Path.CurrentPath = 0;
	c->Movement         = false;
	o->Live             = true;
	o->Visible          = false;
	o->AlphaEnable      = true;
	o->LightEnable      = true;
	o->ContrastEnable   = false;
	o->EnableBoneMatrix = true;
	o->EnableShadow     = false;
	c->Dead             = false;
	c->Blood            = false;
	c->GuildTeam        = 0;
	c->Run              = 0;
	c->GuildMarkIndex         = -1;
	c->PK               = PVP_NEUTRAL;
#ifdef LJH_ADD_MORE_ZEN_FOR_ONE_HAVING_A_PARTY_WITH_MURDERER
	c->PKPartyLevel		= c->PK;
#endif //LJH_ADD_MORE_ZEN_FOR_ONE_HAVING_A_PARTY_WITH_MURDERER
	o->Type             = Type;
	o->Scale            = 0.9f;
	o->Timer            = 0.f;
	o->Alpha            = 1.f;
	o->AlphaTarget      = 1.f;
	o->Velocity         = 0.f;
	o->ShadowScale      = 0.f;
	o->m_byHurtByOneToOne = 0;
    o->AI               = 0;

    o->m_byBuildTime    = 10;
    c->m_iDeleteTime    = -128;
    o->m_bRenderShadow  = true;
    o->m_fEdgeScale     = 1.2f;
    c->m_bIsSelected    = true;
    c->ExtendState = 0;
    c->ExtendStateTime = 0;

#ifdef ASG_ADD_GENS_SYSTEM
	c->m_byGensInfluence = 0;
#endif	// ASG_ADD_GENS_SYSTEM
	
	c->GuildStatus		= -1;
	c->GuildType		= 0;
	c->ProtectGuildMarkWorldTime = 0.0f;
	c->GuildRelationShip= 0;
    c->GuildSkill = AT_SKILL_STUN;
    c->BackupCurrentSkill = 255;
    c->GuildMasterKillCount = 0;
	c->m_byDieType		= 0;
	o->m_bActionStart	= false;
	o->m_bySkillCount	= 0;
    c->m_bFixForm       = false;
	c->CtlCode = 0;
	c->m_CursedTempleCurSkill = AT_SKILL_CURSED_TEMPLE_PRODECTION;
	c->m_CursedTempleCurSkillPacket = false;

	if(Type<MODEL_FACE 
#ifdef PBG_ADD_NEWCHAR_MONK
		|| Type>MODEL_FACE+6
#else //PBG_ADD_NEWCHAR_MONK
		|| Type>MODEL_FACE+5
#endif //PBG_ADD_NEWCHAR_MONK
		)
     	c->Class = 0;

	if(Type==MODEL_PLAYER)
	{
     	o->PriorAction   = 1;
    	o->CurrentAction = 1;
	}
	else
	{
     	o->PriorAction   = 0;
    	o->CurrentAction = 0;
	}
    o->AnimationFrame = 0.f;
	o->PriorAnimationFrame   = 0;
	c->JumpTime         = 0;
	o->HiddenMesh       = -1;
	c->MoveSpeed        = 10;

	g_CharacterClearBuff(o);
	
	o->Teleport         = TELEPORT_NONE;
	o->Kind             = KIND_PLAYER;
	c->Change           = false;
	o->SubType          = 0;
	c->MonsterIndex     = -1;
    o->BlendMeshTexCoordU = 0.f;
    o->BlendMeshTexCoordV = 0.f;
	c->Skill = 0;
	c->AttackTime = 0;
	c->TargetCharacter = -1;
	c->AttackFlag = ATTACK_FAIL;
	c->Weapon[0].Type = -1;
	c->Weapon[0].Level = 0;
	c->Weapon[1].Type = -1;
	c->Weapon[1].Level = 0;
	c->Wing.Type      = -1;
	c->Helper.Type    = -1;

	o->Position[0] = (float)(( c->PositionX)*TERRAIN_SCALE) + 0.5f*TERRAIN_SCALE;
	o->Position[1] = (float)(( c->PositionY)*TERRAIN_SCALE) + 0.5f*TERRAIN_SCALE;

	o->InitialSceneFrame = MoveSceneFrame;
	
    if ( World==-1 || c->Helper.Type!=MODEL_HELPER+3 || c->SafeZone )
    {
        o->Position[2] = RequestTerrainHeight(o->Position[0],o->Position[1]);
    }
    else
    {
        if ( World==WD_8TARKAN || World==WD_10HEAVEN )
            o->Position[2] = RequestTerrainHeight(o->Position[0],o->Position[1])+90.f;
        else
            o->Position[2] = RequestTerrainHeight(o->Position[0],o->Position[1])+30.f;
    }

	Vector(0.f,0.f,Rotation,o->Angle);
	Vector(0.f,0.f,0.f,c->Light);
	Vector(0.f,0.f,0.f,o->Light);
	Vector(-60.f,-60.f,0.f  ,o->BoundingBoxMin);
	switch(Type)
	{
	case MODEL_PLAYER:
      	Vector( 40.f, 40.f,120.f,o->BoundingBoxMax);
		break;
    case MODEL_MONSTER01+70:
    case MODEL_MONSTER01+71:
    case MODEL_MONSTER01+72:
      	Vector( 40.f, 40.f,120.f,o->BoundingBoxMax);
        break;
	case MODEL_MONSTER01+2:
	case MODEL_MONSTER01+6:
	case MODEL_MONSTER01+9:
	case MODEL_MONSTER01+20:
	case MODEL_MONSTER01+19:
	case MODEL_MONSTER01+17:
      	Vector( 50.f, 50.f,80.f,o->BoundingBoxMax);
		break;
	case MODEL_MONSTER01+11:
	case MODEL_MONSTER01+31:
	case MODEL_MONSTER01+39:
	case MODEL_MONSTER01+42:
	case MODEL_MONSTER01+44:
      	Vector( 70.f, 70.f,250.f,o->BoundingBoxMax);
		break;
	case MODEL_MONSTER01+37:
      	Vector( 100.f, 100.f,150.f,o->BoundingBoxMax);
		break;
    case MODEL_MONSTER01+61:
	    Vector(-120.f,-120.f,0.f  ,o->BoundingBoxMin);
      	Vector( 100.f, 100.f,300.f,o->BoundingBoxMax);
        break;
    case MODEL_MONSTER01+60:
    	Vector(-90.f,-50.f,0.f  ,o->BoundingBoxMin);
      	Vector( 90.f, 50.f,200.f,o->BoundingBoxMax);
        break;
#ifdef CSK_ADD_MAP_ICECITY
	case MODEL_MONSTER01+150:
		Vector(-150.f,-150.f,0.f  ,o->BoundingBoxMin);
		Vector( 150.f, 150.f,400.f,o->BoundingBoxMax);
        break;
	case MODEL_MONSTER01+151:
	case MODEL_MONSTER01+152:
	case MODEL_MONSTER01+153:
		Vector(-100.f,-100.f,0.f  ,o->BoundingBoxMin);
		Vector( 100.f, 100.f,200.f,o->BoundingBoxMax);
		break;
#endif // CSK_ADD_MAP_ICECITY
#ifdef PBG_ADD_LITTLESANTA_NPC
	case MODEL_LITTLESANTA:
	case MODEL_LITTLESANTA+1:
	case MODEL_LITTLESANTA+2:
	case MODEL_LITTLESANTA+3:
	case MODEL_LITTLESANTA+4:
	case MODEL_LITTLESANTA+5:
	case MODEL_LITTLESANTA+6:
	case MODEL_LITTLESANTA+7:
		{
			Vector(-160.f,-60.f,-20.f, o->BoundingBoxMin);
			Vector( 10.f, 80.f,50.f, o->BoundingBoxMax);
		}
		break;
#endif //PBG_ADD_LITTLESANTA_NPC
#ifdef PBG_ADD_PKFIELD
	case MODEL_MONSTER01+157:
		{
			Vector(-100.f,-70.f,0.f  ,o->BoundingBoxMin);
			Vector( 100.f, 70.f,150.f,o->BoundingBoxMax);
		}
		break;
	case MODEL_MONSTER01+158:
		{
			Vector(-100.f,-100.f,50.f ,o->BoundingBoxMin);
			Vector( 100.f, 100.f,150.f,o->BoundingBoxMax);
		}
		break;
	case MODEL_MONSTER01+159:
		{
			Vector(-100.f,-100.f,0.f  ,o->BoundingBoxMin);
			Vector( 100.f, 100.f,180.f,o->BoundingBoxMax);
		}
		break;
	case MODEL_MONSTER01+160:
		{
			Vector(-80.f,-80.f,0.f ,o->BoundingBoxMin);
			Vector( 80.f, 80.f,130.f,o->BoundingBoxMax);
		}
		break;
	case MODEL_MONSTER01+161:
		{
			Vector(-80.f,-80.f,0.f ,o->BoundingBoxMin);
			Vector( 80.f, 80.f,130.f,o->BoundingBoxMax);
		}
		break;
	case MODEL_MONSTER01+162:
		{
			Vector(-100.f,-80.f,50.f ,o->BoundingBoxMin);
			Vector( 100.f, 70.f,280.f,o->BoundingBoxMax);
		}
		break;
	case MODEL_MONSTER01+163:
		{
			Vector(-100.f,-80.f,50.f ,o->BoundingBoxMin);
			Vector( 100.f, 70.f,280.f,o->BoundingBoxMax);
		}
		break;
#endif //PBG_ADD_PKFIELD
    default:
      	Vector( 50.f, 50.f,150.f,o->BoundingBoxMax);
		break;
	}

	if(o->BoneTransform != NULL)
	{
		delete[] o->BoneTransform;
		o->BoneTransform = NULL;
	}
	o->BoneTransform = new vec34_t [Models[Type].NumBones];
	hanguo_check2();
	
	for(int i=0;i<2;i++)
	{
		c->Weapon[i].Type = -1;
		c->Weapon[i].Level = 0;
        c->Weapon[i].Option1 = 0;
#if defined MR0 || defined OBJECT_ID
		c->Weapon[i].m_iID = GenID();
#endif //MR0
	}

	for(int i=0;i<MAX_BODYPART;i++)		
	{
		c->BodyPart[i].Type = -1;
		c->BodyPart[i].Level = 0;
        c->BodyPart[i].Option1 = 0;
        c->BodyPart[i].ExtOption = 0;
#if defined MR0 || defined OBJECT_ID
		c->BodyPart[i].m_iID = GenID();
#endif //MR0
	}
	c->Wing.Type       = -1;
	c->Helper.Type     = -1;
	c->Flag.Type       = -1;

#if defined MR0 || defined OBJECT_ID
	c->Wing.m_iID = GenID();
	c->Helper.m_iID = GenID();
	c->Flag.m_iID = GenID();
#endif //MR0
	
	c->LongRangeAttack = -1;
	c->CollisionTime   = 0;
	o->CollisionRange  = 200.f;
	c->Rot             = 0.f;
	c->Level           = 0;
	c->Item            = -1;

#ifdef YDG_ADD_NEW_DUEL_SYSTEM
#if SELECTED_LANGUAGE != LANGUAGE_KOREAN
	for( int i = 0; i < 32; ++i ) c->OwnerID[i] = 0;
#else //SELECTED_LANGUAGE != LANGUAGE_KOREAN
	for( int i = 0; i < 24; ++i ) c->OwnerID[i] = 0;
#endif //SELECTED_LANGUAGE != LANGUAGE_KOREAN
#endif	// YDG_ADD_NEW_DUEL_SYSTEM
	
	o->BlendMesh = -1;
	o->BlendMeshLight = 1.f;
	switch(Type)
	{
    case MODEL_MONSTER01+70:
    case MODEL_MONSTER01+71:
    case MODEL_MONSTER01+72:
		c->Weapon[0].LinkBone = 33; 
		c->Weapon[1].LinkBone = 42;
        break;
	case MODEL_MONSTER01+57:
		c->Weapon[0].LinkBone = 30;
		c->Weapon[1].LinkBone = 39;
		break;
	case MODEL_MONSTER01+59:
		c->Weapon[0].LinkBone = 33;
		c->Weapon[1].LinkBone = 20;
		break;
	case MODEL_MONSTER01+60:
		c->Weapon[0].LinkBone = 1;
		c->Weapon[1].LinkBone = 1;
		break;
	case MODEL_MONSTER01+55:
		c->Weapon[0].LinkBone = 27;
		c->Weapon[1].LinkBone = 18;
		break;
	case MODEL_MONSTER01+52:
		c->Weapon[0].LinkBone = 36;
		c->Weapon[1].LinkBone = 45;
		break;
	case MODEL_MONSTER01+53:
		c->Weapon[0].LinkBone = 30;
		c->Weapon[1].LinkBone = 39;
		break;
	case MODEL_MONSTER01+46:
		c->Weapon[0].LinkBone = 39;
		c->Weapon[1].LinkBone = 39;
		break;
	case MODEL_MONSTER01+47:
		c->Weapon[0].LinkBone = 27;
		c->Weapon[1].LinkBone = 38;
		break;
	case MODEL_MONSTER01+48:
		c->Weapon[0].LinkBone = 32;
		c->Weapon[1].LinkBone = 43;
		break;
	case MODEL_MONSTER01+44:
		c->Weapon[0].LinkBone = 55;
		c->Weapon[1].LinkBone = 70;
		break;
	case MODEL_MONSTER01+42:
		c->Weapon[0].LinkBone = 43;
		break;
	case MODEL_MONSTER01+41:
		c->Weapon[0].LinkBone = 23;
		break;
	case MODEL_MONSTER01+36:
		c->Weapon[0].LinkBone = 52;
		c->Weapon[1].LinkBone = 65;
		break;
	case MODEL_MONSTER01+35:
		c->Weapon[0].LinkBone = 30;
		c->Weapon[1].LinkBone = 39;
		break;
	case MODEL_MONSTER01+34:
		c->Weapon[0].LinkBone = 30;
		c->Weapon[1].LinkBone = 39;
		break;
	case MODEL_MONSTER01+26:
		c->Weapon[0].LinkBone = 16;
		c->Weapon[1].LinkBone = 25;
		break;
	case MODEL_MONSTER01+29:
		c->Weapon[0].LinkBone = 30;
		c->Weapon[1].LinkBone = 39;
		break;
	case MODEL_MONSTER01+27:
		c->Weapon[0].LinkBone = 17;
		c->Weapon[1].LinkBone = 28;
		break;
	case MODEL_MONSTER01+24:
		c->Weapon[0].LinkBone = 39;
		c->Weapon[1].LinkBone = 30;
		break;
	case MODEL_MONSTER01+22:
		c->Weapon[0].LinkBone = 25;
		c->Weapon[1].LinkBone = 16;
		break;
	case MODEL_MONSTER01+21:
		c->Weapon[0].LinkBone = 24;
		c->Weapon[1].LinkBone = 19;
		break;
	case MODEL_MONSTER01+19:
		c->Weapon[0].LinkBone = 31;
		c->Weapon[1].LinkBone = 22;
		break;
	case MODEL_MONSTER01+18:
		c->Weapon[0].LinkBone = 26;
		c->Weapon[1].LinkBone = 35;
		break;
	case MODEL_MONSTER01+16:
	case MODEL_MONSTER01+11:
		c->Weapon[0].LinkBone = 30;
		c->Weapon[1].LinkBone = 39;
		break;
	case MODEL_MONSTER01+3:
		c->Weapon[0].LinkBone = 26;
		c->Weapon[1].LinkBone = 36;
		break;
	case MODEL_MONSTER01:
	case MODEL_MONSTER01+30:
		c->Weapon[0].LinkBone = 42;
		c->Weapon[1].LinkBone = 33;
		break;
	case MODEL_MONSTER01+10:
	case MODEL_MONSTER01+4:
	case MODEL_MONSTER01+5:
		c->Weapon[0].LinkBone = 41;
		c->Weapon[1].LinkBone = 32;
		break;
	case MODEL_MONSTER01+1:
		c->Weapon[0].LinkBone = 19;
		c->Weapon[1].LinkBone = 14;
		break;
	case MODEL_MONSTER01+8:
		c->Weapon[0].LinkBone = 29;
		c->Weapon[1].LinkBone = 38;
		break;
	case MODEL_MONSTER01+40:
		c->Weapon[0].LinkBone = 20;
		c->Weapon[1].LinkBone = 33;
		break;
	default:
        if ( SettingHellasMonsterLinkBone( c, Type ) ) return;
        if ( battleCastle::SettingBattleCastleMonsterLinkBone ( c, Type ) ) return;
		c->Weapon[0].LinkBone = 33; 
		c->Weapon[1].LinkBone = 42;
		break;
	}
}

CHARACTER *CreateCharacter(int Key,int Type,unsigned char PositionX,unsigned char PositionY,float Rotation)
{
	for(int i=0;i<MAX_CHARACTERS_CLIENT;i++)
	{
		CHARACTER *c = &CharactersClient[i];
		OBJECT *o = &c->Object;
		if(o->Live && c->Key==Key)
		{
			CreateCharacterPointer(c,Type,PositionX,PositionY,Rotation);
			g_CharacterClearBuff(o);
            return c;
		}
	}

	for(int i=0;i<MAX_CHARACTERS_CLIENT;i++)
	{
		CHARACTER *c = &CharactersClient[i];
		OBJECT *o = &c->Object;
		if(!o->Live)
		{
			BoneManager::UnregisterBone(c);
            DeletePet ( c );
			DeleteCloth( c, o);
            DeleteParts ( c );
			CreateCharacterPointer(c,Type,PositionX,PositionY,Rotation);
			g_CharacterClearBuff(o);
			c->Key = Key;
            return c;
		}
	}

	return &CharactersClient[MAX_CHARACTERS_CLIENT];
}

void SetCharacterScale(CHARACTER *c)
{
	if(c->Change) 
		return;

    if(c->BodyPart[BODYPART_HELM].Type==MODEL_HELM ||
		c->BodyPart[BODYPART_HELM].Type==MODEL_HELM+2 ||
#ifdef LEM_ADD_LUCKYITEM
		c->BodyPart[BODYPART_HELM].Type==MODEL_HELM+63 ||
		c->BodyPart[BODYPART_HELM].Type==MODEL_HELM+68 ||
		c->BodyPart[BODYPART_HELM].Type==MODEL_HELM+65 ||
		c->BodyPart[BODYPART_HELM].Type==MODEL_HELM+70 ||
#endif // LEM_ADD_LUCKYITEM
		(c->BodyPart[BODYPART_HELM].Type>=MODEL_HELM+10 && c->BodyPart[BODYPART_HELM].Type<=MODEL_HELM+13))
	{
		c->BodyPart[BODYPART_HEAD].Type = MODEL_BODY_HELM + GetSkinModelIndex(c->Class);
	}
	else
	{
		c->BodyPart[BODYPART_HEAD  ].Type = -1;
	}

#ifdef PJH_NEW_SERVER_SELECT_MAP
	if(SceneFlag == CHARACTER_SCENE)
	{
		if( c->Skin==0 )
		{
			switch(GetBaseClass(c->Class))
			{
			case CLASS_WIZARD:    c->Object.Scale = 1.2f;break;
			case CLASS_KNIGHT:    c->Object.Scale = 1.2f;break;
			case CLASS_ELF   :    c->Object.Scale = 1.2f;break;
			case CLASS_DARK  :    c->Object.Scale = 1.2f;break;
			case CLASS_DARK_LORD: c->Object.Scale = 1.2f;break;
			case CLASS_SUMMONER:	c->Object.Scale = 1.2f;break;
#ifdef PBG_ADD_NEWCHAR_MONK
			case CLASS_RAGEFIGHTER:	c->Object.Scale = 1.35f;break;
#endif //PBG_ADD_NEWCHAR_MONK
			}
		}
		else
		{
			switch(GetBaseClass(c->Class))
			{
			case CLASS_WIZARD:    c->Object.Scale = 1.2f;break;
			case CLASS_KNIGHT:    c->Object.Scale = 1.2f;break;
			case CLASS_ELF   :    c->Object.Scale = 1.2f;break;
			case CLASS_DARK  :    c->Object.Scale = 1.2f;break;
			case CLASS_DARK_LORD: c->Object.Scale = 1.2f;break;
			case CLASS_SUMMONER:	c->Object.Scale = 1.2f;break;
#ifdef PBG_ADD_NEWCHAR_MONK
			case CLASS_RAGEFIGHTER:	c->Object.Scale = 1.35f;break;
#endif //PBG_ADD_NEWCHAR_MONK
			}
		}
	}
	else
	{
#endif //PJH_NEW_SERVER_SELECT_MAP
		if( c->Skin==0 )
		{
			switch(GetBaseClass(c->Class))
			{
			case CLASS_WIZARD:    c->Object.Scale = 0.9f;break;
			case CLASS_KNIGHT:    c->Object.Scale = 0.9f;break;
			case CLASS_ELF   :    c->Object.Scale = 0.88f;break;
			case CLASS_DARK  :    c->Object.Scale = 0.95f;break;
			case CLASS_DARK_LORD: c->Object.Scale = 0.92f;break;
			case CLASS_SUMMONER:	c->Object.Scale = 0.90f;break;
#ifdef PBG_ADD_NEWCHAR_MONK
			case CLASS_RAGEFIGHTER:	c->Object.Scale = 1.03f;break;
#endif //PBG_ADD_NEWCHAR_MONK
			}
		}
		else
		{
			switch(GetBaseClass(c->Class))
			{
			case CLASS_WIZARD:    c->Object.Scale = 0.93f;break;
			case CLASS_KNIGHT:    c->Object.Scale = 0.93f;break;
			case CLASS_ELF   :    c->Object.Scale = 0.86f;break;
			case CLASS_DARK  :    c->Object.Scale = 0.95f;break;
			case CLASS_DARK_LORD: c->Object.Scale = 0.92f;break;
			case CLASS_SUMMONER:	c->Object.Scale = 0.90f;break;
#ifdef PBG_ADD_NEWCHAR_MONK
			case CLASS_RAGEFIGHTER:	c->Object.Scale = 1.03f;break;
#endif //PBG_ADD_NEWCHAR_MONK
			}
		}
#ifdef PJH_NEW_SERVER_SELECT_MAP
	}
#endif //PJH_NEW_SERVER_SELECT_MAP
}

void SetCharacterClass(CHARACTER *c)
{
	if(c->Object.Type != MODEL_PLAYER)
	{
		return;
	}
	
	ITEM *p = CharacterMachine->Equipment;

	if(p[EQUIPMENT_WEAPON_RIGHT].Type == -1)
	{
		c->Weapon[0].Type = -1;
	}
	else
	{
	    c->Weapon[0].Type = p[EQUIPMENT_WEAPON_RIGHT].Type+MODEL_ITEM;
	}

	if(p[EQUIPMENT_WEAPON_LEFT].Type == -1)
	{
		c->Weapon[1].Type = -1;
	}
	else
	{
    	c->Weapon[1].Type = p[EQUIPMENT_WEAPON_LEFT].Type+MODEL_ITEM;
	}

	if(p[EQUIPMENT_WING].Type == -1)
	{
		c->Wing.Type = -1;
	}
	else
	{
		c->Wing.Type = p[EQUIPMENT_WING].Type+MODEL_ITEM;
	}

	if(p[EQUIPMENT_HELPER].Type == -1)
	{
		c->Helper.Type = -1;
	}
	else
	{
		c->Helper.Type = p[EQUIPMENT_HELPER].Type+MODEL_ITEM;
	}

	c->Weapon[0].Level = (p[EQUIPMENT_WEAPON_RIGHT].Level>>3)&15;
	c->Weapon[1].Level = (p[EQUIPMENT_WEAPON_LEFT ].Level>>3)&15;
	c->Weapon[0].Option1 = p[EQUIPMENT_WEAPON_RIGHT].Option1;
	c->Weapon[1].Option1 = p[EQUIPMENT_WEAPON_LEFT ].Option1;
#ifdef PBG_ADD_NEWCHAR_MONK
	c->Weapon[0].ExtOption = p[EQUIPMENT_WEAPON_RIGHT].ExtOption;
	c->Weapon[1].ExtOption = p[EQUIPMENT_WEAPON_LEFT ].ExtOption;
#endif //PBG_ADD_NEWCHAR_MONK
	c->Wing.Level = (p[EQUIPMENT_WING].Level>>3)&15;
	c->Helper.Level = (p[EQUIPMENT_HELPER].Level>>3)&15;

	bool Success = true;

    if ( InChaosCastle() == true )
        Success = false;

	if(c->Object.CurrentAction>=PLAYER_SIT1 && c->Object.CurrentAction<=PLAYER_POSE_FEMALE1)
	{
		Success = false;
	}
	if(c->Object.CurrentAction>=PLAYER_ATTACK_FIST && c->Object.CurrentAction<=PLAYER_RIDE_SKILL)
	{
		Success = false;
	}

	if(Success)
	{
        SetPlayerStop(c);
	}

#ifndef KJH_FIX_WOPS_K27082_REFRASH_STAT_EQUIPED_TRANSFORM_RING				// #ifndef
	if(c->Change)
	{
		return;
	}
#endif // KJH_FIX_WOPS_K27082_REFRASH_STAT_EQUIPED_TRANSFORM_RING

    if(p[EQUIPMENT_HELM].Type == -1)
    {
		c->BodyPart[BODYPART_HELM].Type = MODEL_BODY_HELM+GetSkinModelIndex(c->Class);
    }
	else
	{
     	c->BodyPart[BODYPART_HELM].Type = p[EQUIPMENT_HELM  ].Type+MODEL_ITEM;
	}

	if(p[EQUIPMENT_ARMOR].Type == -1)
    {
		c->BodyPart[BODYPART_ARMOR].Type = MODEL_BODY_ARMOR + GetSkinModelIndex(c->Class);
    }
	else
	{
		c->BodyPart[BODYPART_ARMOR].Type = p[EQUIPMENT_ARMOR].Type+MODEL_ITEM;
	}

	if(p[EQUIPMENT_PANTS].Type == -1)
    {
		c->BodyPart[BODYPART_PANTS].Type = MODEL_BODY_PANTS + GetSkinModelIndex(c->Class);
    }
	else
	{
		c->BodyPart[BODYPART_PANTS].Type = p[EQUIPMENT_PANTS].Type+MODEL_ITEM;
	}

	if(p[EQUIPMENT_GLOVES].Type == -1)
    {
		c->BodyPart[BODYPART_GLOVES].Type = MODEL_BODY_GLOVES + GetSkinModelIndex(c->Class);
    }
	else
	{
		c->BodyPart[BODYPART_GLOVES].Type = p[EQUIPMENT_GLOVES].Type+MODEL_ITEM;
	}

	if(p[EQUIPMENT_BOOTS].Type == -1)
    {
		c->BodyPart[BODYPART_BOOTS].Type = MODEL_BODY_BOOTS + GetSkinModelIndex(c->Class);
    }
	else
	{
		c->BodyPart[BODYPART_BOOTS].Type = p[EQUIPMENT_BOOTS ].Type+MODEL_ITEM;
	}

	c->BodyPart[BODYPART_HELM ].Level = (p[EQUIPMENT_HELM].Level>>3)&15;
	c->BodyPart[BODYPART_ARMOR].Level = (p[EQUIPMENT_ARMOR].Level>>3)&15;
	c->BodyPart[BODYPART_PANTS].Level = (p[EQUIPMENT_PANTS].Level>>3)&15;
	c->BodyPart[BODYPART_GLOVES].Level = (p[EQUIPMENT_GLOVES].Level>>3)&15;
	c->BodyPart[BODYPART_BOOTS].Level = (p[EQUIPMENT_BOOTS].Level>>3)&15;
	c->BodyPart[BODYPART_HELM].Option1 = p[EQUIPMENT_HELM].Option1;
	c->BodyPart[BODYPART_ARMOR].Option1 = p[EQUIPMENT_ARMOR].Option1;
	c->BodyPart[BODYPART_PANTS].Option1 = p[EQUIPMENT_PANTS].Option1;
	c->BodyPart[BODYPART_GLOVES].Option1 = p[EQUIPMENT_GLOVES].Option1;
	c->BodyPart[BODYPART_BOOTS].Option1 = p[EQUIPMENT_BOOTS].Option1;
	c->BodyPart[BODYPART_HELM].ExtOption = p[EQUIPMENT_HELM].ExtOption;
	c->BodyPart[BODYPART_ARMOR].ExtOption = p[EQUIPMENT_ARMOR].ExtOption;
	c->BodyPart[BODYPART_PANTS].ExtOption = p[EQUIPMENT_PANTS].ExtOption;
	c->BodyPart[BODYPART_GLOVES].ExtOption = p[EQUIPMENT_GLOVES].ExtOption;
	c->BodyPart[BODYPART_BOOTS].ExtOption = p[EQUIPMENT_BOOTS].ExtOption;

#ifndef GUILD_WAR_EVENT
    ChangeChaosCastleUnit ( c );
#endif //GUILD_WAR_EVENT
    
    SetCharacterScale(c);

    if(c == Hero)
    {
        CheckFullSet(Hero);
    }

    CharacterMachine->CalculateAll();
}

void SetChangeClass(CHARACTER *c)
{
	if(c->Object.Type != MODEL_PLAYER) 
		return;

    bool Success = true;

    if(c->Object.CurrentAction>=PLAYER_SIT1 && c->Object.CurrentAction<=PLAYER_POSE_FEMALE1)
		Success = false;
	if(c->Object.CurrentAction>=PLAYER_ATTACK_FIST && c->Object.CurrentAction<=PLAYER_RIDE_SKILL)
		Success = false;
	if(Success)
        SetPlayerStop(c);

	int iSkinModelIndex = GetSkinModelIndex(c->Class);
	if(c->BodyPart[BODYPART_HELM  ].Type >= MODEL_BODY_HELM)
	{
		c->BodyPart[BODYPART_HELM  ].Type = MODEL_BODY_HELM+iSkinModelIndex;
		c->BodyPart[BODYPART_HELM  ].Level= 0;
		c->BodyPart[BODYPART_HELM  ].Option1= 0;
        c->BodyPart[BODYPART_HELM  ].ExtOption = 0;
	}
	if(c->BodyPart[EQUIPMENT_ARMOR ].Type >= MODEL_BODY_ARMOR)
    {
		c->BodyPart[BODYPART_ARMOR ].Type = MODEL_BODY_ARMOR+iSkinModelIndex;
		c->BodyPart[BODYPART_ARMOR ].Level= 0;
		c->BodyPart[BODYPART_ARMOR ].Option1= 0;
		c->BodyPart[BODYPART_ARMOR ].ExtOption = 0;
    }
	if(c->BodyPart[EQUIPMENT_PANTS ].Type >= MODEL_BODY_PANTS)
    {
		c->BodyPart[BODYPART_PANTS ].Type = MODEL_BODY_PANTS+iSkinModelIndex;
		c->BodyPart[BODYPART_PANTS ].Level= 0;
		c->BodyPart[BODYPART_PANTS ].Option1= 0;
		c->BodyPart[BODYPART_PANTS ].ExtOption = 0;
    }
	if(c->BodyPart[EQUIPMENT_GLOVES].Type >= MODEL_BODY_GLOVES)
    {
		c->BodyPart[BODYPART_GLOVES].Type = MODEL_BODY_GLOVES+iSkinModelIndex;
		c->BodyPart[BODYPART_GLOVES].Level= 0;
		c->BodyPart[BODYPART_GLOVES].Option1= 0;
		c->BodyPart[BODYPART_GLOVES].ExtOption = 0;
    }
	if(c->BodyPart[EQUIPMENT_BOOTS ].Type >= MODEL_BODY_BOOTS)
    {
		c->BodyPart[BODYPART_BOOTS ].Type = MODEL_BODY_BOOTS+iSkinModelIndex;
		c->BodyPart[BODYPART_BOOTS ].Level= 0;
		c->BodyPart[BODYPART_BOOTS ].Option1= 0;
		c->BodyPart[BODYPART_BOOTS ].ExtOption = 0;
    }
    
	SetCharacterScale(c);
}

DWORD GetGuildRelationShipTextColor( BYTE GuildRelationShip )
{
	DWORD dwColor = 0;

	if( GuildRelationShip == GR_NONE )
		dwColor  = (255<<24)+(255<<16)+(230<<8)+(230);
	else if(GuildRelationShip == GR_RIVAL || GuildRelationShip == GR_RIVALUNION )
		dwColor = (255<<24)+(0<<16)+(30<<8)+(255);
	else
		dwColor  = (255<<24)+(0<<16)+(255<<8)+(200);

	return dwColor;
}

DWORD GetGuildRelationShipBGColor( BYTE GuildRelationShip )
{
	DWORD dwColor = 0;

	if( GuildRelationShip == GR_NONE )
    	dwColor = (150<<24)+(50<<16)+(30<<8)+(10);
	else if(GuildRelationShip == GR_RIVAL || GuildRelationShip == GR_RIVALUNION )
		dwColor = (150<<24)+(0<<16)+(0<<8)+(0);
	else
		dwColor = (150<<24)+(80<<16)+(50<<8)+(20);

	return dwColor;
}

CHARACTER* FindCharacterByID( char* szName )
{
	for(int i=0;i<MAX_CHARACTERS_CLIENT;i++)
	{
      	CHARACTER *c = &CharactersClient[i];
		if( c->Object.Live && !strcmp( szName, c->ID ) )
		{
			return c;
		}
	}
	return NULL;
}

int LevelConvert(BYTE Level)
{
	switch(Level)
	{
	case 0:return 0;break;
	case 1:return 3;break;
	case 2:return 5;break;
	case 3:return 7;break;
	case 4:return 9;break;
	case 5:return 11;break;
#ifdef LDK_ADD_14_15_GRADE_ITEM_TYPE_CHANGE
	case 6:return 13;break;
	case 7:return 15;break;
#else //LDK_ADD_14_15_GRADE_ITEM_TYPE_CHANGE
	case 6:return 12;break;
	case 7:return 13;break;
#endif //LDK_ADD_14_15_GRADE_ITEM_TYPE_CHANGE
	}
	return 0;
}

void MakeElfHelper(CHARACTER* c)
{
	OBJECT* o = &c->Object;
	c->Wing.Type = MODEL_WING + 3;
	c->BodyPart[BODYPART_HELM  ].Type = MODEL_HELM + 24;
	c->BodyPart[BODYPART_ARMOR ].Type = MODEL_ARMOR + 24;
	c->BodyPart[BODYPART_PANTS ].Type = MODEL_PANTS + 24;
	c->BodyPart[BODYPART_GLOVES].Type = MODEL_GLOVES + 24;
	c->BodyPart[BODYPART_BOOTS ].Type = MODEL_BOOTS + 24;
	c->BodyPart[BODYPART_HELM  ].Level = 13;
	c->BodyPart[BODYPART_ARMOR ].Level = 13;
	c->BodyPart[BODYPART_PANTS ].Level = 13;
	c->BodyPart[BODYPART_GLOVES].Level = 13;
	c->BodyPart[BODYPART_BOOTS ].Level = 13;

	o->Scale = 1.f;
	o->CurrentAction = PLAYER_STOP_FLY;
	o->BoundingBoxMax[2] += 70.f;
}

void ChangeCharacterExt(int Key,BYTE *Equipment, CHARACTER * pCharacter, OBJECT * pHelper)
{
	CHARACTER *c;
	if (pCharacter == NULL)
		c = &CharactersClient[Key];
	else
		c = pCharacter;

	OBJECT *o = &c->Object;
	if(o->Type != MODEL_PLAYER)
		return;

	BYTE Type = 0;
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
	BYTE ExtBit = 0;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
	short ExtType = 0;

	Type = Equipment[0];
	ExtType = Equipment[11]&240;
	ExtType = (ExtType<<4) | Type;
#ifdef YDG_FIX_VIEWPORT_HAND_CHECK
	if(ExtType == 0x0FFF)
#else	// YDG_FIX_VIEWPORT_HAND_CHECK
	if(ExtType == 0x1FFF)
#endif	// YDG_FIX_VIEWPORT_HAND_CHECK
    {
		c->Weapon[0].Type = -1;
        c->Weapon[0].Option1 = 0;
        c->Weapon[0].ExtOption = 0;
    }
	else
	{
		c->Weapon[0].Type = MODEL_SWORD + ExtType;
	}

	Type = Equipment[1];
	ExtType = Equipment[12]&240;
	ExtType = (ExtType<<4) | Type;
#ifdef YDG_FIX_VIEWPORT_HAND_CHECK
	if(ExtType == 0x0FFF)
#else	// YDG_FIX_VIEWPORT_HAND_CHECK
	if(ExtType == 0x1FFF)
#endif	// YDG_FIX_VIEWPORT_HAND_CHECK
    {
		c->Weapon[1].Type = -1;
        c->Weapon[1].Option1 = 0;
        c->Weapon[1].ExtOption = 0;
    }
	else
    {
#ifdef PET_SYSTEM
        if ( GetBaseClass(c->Class)==CLASS_DARK_LORD && ((MODEL_STAFF+5)-MODEL_SWORD)==ExtType)
        {
#ifdef PBG_FIX_DARKPET_RENDER
			// 서버에서 왼손무기중에 빈곳에 다크 스피릿의 레벨정보를 받아서 사용하자.
			// 현재 아무정보를 받지 못함. 초기값으로 설정되어 버린다.!!!수정 요망!!!
			// 렌더링에만 사용 될 것이므로 레벨 정보만을 받아도 무관하다.
			ITEM* pEquipmentItemSlot = &CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT];
			PET_INFO* pPetInfo = giPetManager::GetPetInfo(pEquipmentItemSlot);
			giPetManager::CreatePetDarkSpirit(c);
			if(!InChaosCastle())
				((CSPetSystem*)c->m_pPet)->SetPetInfo(pPetInfo);
#else //PBG_FIX_DARKPET_RENDER
			c->Weapon[1].Type = MODEL_HELPER+5;//-1;
            giPetManager::CreatePetDarkSpirit ( c );            
#endif //PBG_FIX_DARKPET_RENDER
        }
        else
#endif// PET_SYSTEM
        {
		    c->Weapon[1].Type = MODEL_SWORD + ExtType;
        }
    }

    //날개 체크
	Type = (Equipment[4]>>2)&3;
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
	//신규캐릭터 추가로 인한 날개 인덱스 확장 구조변경
	if(Type == 1)			//1차 날개
	{
		Type = Equipment[8]&0x07;
		switch(Type)
		{
		case 4:
			c->Wing.Type = MODEL_WING + 41;
			break;
		default:
			c->Wing.Type = MODEL_WING + Type-1;
			break;
		}
	}
	else if(Type == 2)		//2차 날개
	{
		Type = Equipment[8]&0x07;
		switch(Type)
		{
		case 5:		c->Wing.Type = MODEL_HELPER+30; break;
		case 6:		c->Wing.Type = MODEL_WING+42; break;
		case 7:		c->Wing.Type = MODEL_WING+49; break;
		default:	
			c->Wing.Type = MODEL_WING + 2+Type;
			break;
		}
	}
	else if(Type == 3)		//3차 날개
	{
		Type = Equipment[8]&0x07;
		switch(Type)
		{
		case 0:				//작은날개
			{
				Type = (Equipment[16]>>5);
				c->Wing.Type = MODEL_WING + 129+Type;
			}
			break;
		case 6:		c->Wing.Type = MODEL_WING+43; break;
		case 7:		c->Wing.Type = MODEL_WING+50; break;
		default:
			c->Wing.Type = MODEL_WING + 35+Type;
			break;
		}
	}
	else	//날개 없음	type == 0
	{
		c->Wing.Type = -1;
        c->Wing.Option1 = 0;
        c->Wing.ExtOption = 0;
	}
#else //PBG_ADD_NEWCHAR_MONK_ITEM
	if(Type == 3)
    {
        Type = Equipment[8]&0x07;
        if ( Type!=0 )
        {
#ifdef ADD_ALICE_WINGS_1
			switch (Type)
			{
			case 5:		c->Wing.Type = MODEL_HELPER+30;	break;	// 망토.
			case 6:		c->Wing.Type = MODEL_WING+41;	break;	// 재앙의 날개.
			case 7:		c->Wing.Type = MODEL_WING+42;	break;	// 절망의 날개.
			default:	c->Wing.Type = MODEL_WING+Type+2;
			}
#else	// ADD_ALICE_WINGS_1
			//  망토. ( 6, 7로 확장 가능 ).
            if ( Type==5 )
    		    c->Wing.Type = MODEL_HELPER + 30;
            else
    		    c->Wing.Type = MODEL_WING + Type + 2;
#endif	// ADD_ALICE_WINGS_1
        }
        else
        {
		    c->Wing.Type = -1;
            c->Wing.Option1 = 0;
            c->Wing.ExtOption = 0;
        }
    }
	else
	{
		c->Wing.Type = MODEL_WING + Type;
	}

	Type = (Equipment[15] >> 2) & 0x07;
	if(Type > 0)
	{
#ifdef ADD_ALICE_WINGS_2
		switch (Type)
		{
		case 6:		c->Wing.Type = MODEL_WING+43;	break;
		default:	c->Wing.Type = MODEL_WING+35+Type;
		}
#else	// ADD_ALICE_WINGS_2
		c->Wing.Type = 	MODEL_WING + 35 + Type;
#endif	// ADD_ALICE_WINGS_2
	}

#ifdef LDK_ADD_INGAMESHOP_SMALL_WING
	Type = (Equipment[16] >> 5);
	if(Type > 0)
	{
		switch (Type)
		{
		case 0x01: c->Wing.Type = MODEL_WING+130;	break;
		case 0x02: c->Wing.Type = MODEL_WING+131;	break;
		case 0x03: c->Wing.Type = MODEL_WING+132;	break;
		case 0x04: c->Wing.Type = MODEL_WING+133;	break;
		case 0x05: c->Wing.Type = MODEL_WING+134;	break;
		}
	}
#endif //LDK_ADD_INGAMESHOP_SMALL_WING
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
	if (pHelper == NULL)
	{
   		DeleteBug(o);
#ifdef LDK_ADD_NEW_PETPROCESS
		ThePetProcess().DeletePet(c, c->Helper.Type, true);
#endif //LDK_ADD_NEW_PETPROCESS
	}
	else
	{
		pHelper->Live = false;
	}
	Type = Equipment[4]&3;
	if(Type == 3)
	{
        Type = Equipment[9]&0x01;
        if ( Type==1 )
        {
    		c->Helper.Type = MODEL_HELPER+3;
			if (pHelper == NULL)
				CreateBug( MODEL_PEGASUS, o->Position,o);
			else
				CreateBugSub( MODEL_PEGASUS, o->Position,o,pHelper);
        }
        else
        {
		    c->Helper.Type = -1;
            c->Helper.Option1 = 0;
            c->Helper.ExtOption = 0;
        }
	}
    else
	{
#if defined LDK_ADD_NEW_PETPROCESS // && defined LDK_ADD_PC4_GUARDIAN 
		BYTE _temp = Equipment[15]&0xE0;

		if( 32 == _temp || 64 == _temp 
#ifdef LDK_ADD_RUDOLPH_PET
			|| 128 == _temp 
#endif //LDK_ADD_RUDOLPH_PET
#ifdef PJH_ADD_PANDA_PET
			|| 224 == _temp 
#endif //PJH_ADD_PANDA_PET
#ifdef LDK_ADD_CS7_UNICORN_PET
			|| 160 == _temp
#endif //LDK_ADD_CS7_UNICORN_PET
#ifdef YDG_ADD_SKELETON_PET
			|| 96 == _temp
#endif	// YDG_ADD_SKELETON_PET
			)
		{
			short _type = 0; 
			switch(_temp) 
			{
			case 32: _type = 64; break;
			case 64: _type = 65; break;
#ifdef LDK_ADD_RUDOLPH_PET
			case 128: _type = 67; break;
#endif //LDK_ADD_RUDOLPH_PET
#ifdef PJH_ADD_PANDA_PET
			case 224: _type = 80; break;
#endif //PJH_ADD_PANDA_PET
#ifdef LDK_ADD_CS7_UNICORN_PET
			case 160: _type = 106; break;
#endif //LDK_ADD_CS7_UNICORN_PET
#ifdef YDG_ADD_SKELETON_PET
			case 96: _type = 123; break;
#endif	// YDG_ADD_SKELETON_PET
			}

			c->Helper.Type = MODEL_HELPER + _type;

#if SELECTED_LANGUAGE == LANGUAGE_JAPANESE
#ifdef LDK_FIX_NOT_CREATE_NEW_PET
			if( ( _type != 64 && _type != 65 ) || ( CHARACTER_SCENE == SceneFlag || c == Hero ) )
#else //LDK_FIX_NOT_CREATE_NEW_PET
			if( CHARACTER_SCENE == SceneFlag || c == Hero )
#endif //LDK_FIX_NOT_CREATE_NEW_PET
#endif //SELECTED_LANGUAGE == LANGUAGE_JAPANESE
			{
				ThePetProcess().CreatePet(ITEM_HELPER+_type, c->Helper.Type, o->Position, c);
			}
		}
		else
#endif //defined LDK_ADD_NEW_PETPROCESS // && defined LDK_ADD_PC4_GUARDIAN
		{
			c->Helper.Type = MODEL_HELPER+Type;
			int HelperType = 0;
			BOOL bCreateHelper = TRUE;
			switch(Type)
			{
			case 0:HelperType = MODEL_HELPER;break;
			case 2:HelperType = MODEL_UNICON;break;
			case 3:HelperType = MODEL_PEGASUS;break;
			default:bCreateHelper = FALSE;break;
			}
			if (bCreateHelper == TRUE)
			{
				if (pHelper == NULL)
					CreateBug( HelperType, o->Position,o);
				else
					CreateBugSub( HelperType, o->Position,o,pHelper);
			}
		}
	}

    Type = Equipment[11]&0x01;
    if ( Type==1 ) 
    {
		c->Helper.Type = MODEL_HELPER+4;
		if (pHelper == NULL)
			CreateBug( MODEL_DARK_HORSE, o->Position,o);
		else
			CreateBugSub( MODEL_DARK_HORSE, o->Position,o,pHelper);
    }

	Type = Equipment[11]&0x04;
	if(Type == 4)
	{
		c->Helper.Type = MODEL_HELPER+37;

		Type = Equipment[15]&3;

		int iFenrirType = Equipment[16]&1;
		if(iFenrirType == 1)
		{
			Type = 0x04;
		}
		
		c->Helper.Option1 = Type;
		if(Type == 0x01)
		{
			if (pHelper == NULL)
				CreateBug(MODEL_FENRIR_BLACK, o->Position, o);
			else
				CreateBugSub(MODEL_FENRIR_BLACK, o->Position, o, pHelper);
		}
		else if(Type == 0x02)
		{
			if (pHelper == NULL)
				CreateBug(MODEL_FENRIR_BLUE, o->Position, o);
			else
				CreateBugSub(MODEL_FENRIR_BLUE, o->Position, o, pHelper);
		}
		else if(Type == 0x04)
		{
			if (pHelper == NULL)
				CreateBug(MODEL_FENRIR_GOLD, o->Position, o);
			else
				CreateBugSub(MODEL_FENRIR_GOLD, o->Position, o, pHelper);
		}
		else
		{
			if (pHelper == NULL)
				CreateBug(MODEL_FENRIR_RED, o->Position, o);
			else
				CreateBugSub(MODEL_FENRIR_RED, o->Position, o, pHelper);
		}
	}

    DeleteParts ( c );
    Type = (Equipment[11]>>1)&0x01;
    if ( Type==1 )
    {
        if ( c->EtcPart<=0 || c->EtcPart>3 )
        {
            c->EtcPart = PARTS_LION;
        }
    }
    else
    {
        if ( c->EtcPart<=0 || c->EtcPart>3 )
        {
            c->EtcPart = 0;
        }
    }

	hanguo_check1();
	int Level = ((int)Equipment[5]<<16)+((int)Equipment[6]<<8)+((int)Equipment[7]);
	c->Weapon  [0              ].Level = LevelConvert((Level>> 0)&7);
	c->Weapon  [1              ].Level = LevelConvert((Level>> 3)&7);
	c->Wing.Level                      = 0;
	c->Helper.Level                    = 0;

	if(c->Change) 
		return;

	ExtType = (Equipment[2]>>4)+((Equipment[8]>>7)&1)*16 +(Equipment[12]&15)*32;
    if(ExtType == 0x1FF)
	{
		c->BodyPart[BODYPART_HELM].Type = MODEL_BODY_HELM + GetSkinModelIndex(c->Class);
	}
	else
	{
		c->BodyPart[BODYPART_HELM].Type = MODEL_HELM+ExtType;
	}

	ExtType = (Equipment[2]&15)+((Equipment[8]>>6)&1)*16 +((Equipment[13]>>4)&15)*32;
	if(ExtType == 0x1FF)
	{
		c->BodyPart[BODYPART_ARMOR].Type = MODEL_BODY_ARMOR + GetSkinModelIndex(c->Class);
	}
	else
	{
		c->BodyPart[BODYPART_ARMOR].Type = MODEL_ARMOR+ExtType;
	}
	
	ExtType = (Equipment[3]>>4)+((Equipment[8]>>5)&1)*16 +(Equipment[13]&15)*32;
	if(ExtType == 0x1FF)
	{
		c->BodyPart[BODYPART_PANTS].Type = MODEL_BODY_PANTS + GetSkinModelIndex(c->Class);
	}
	else
	{
		c->BodyPart[BODYPART_PANTS].Type = MODEL_PANTS+ExtType;
	}
	
	ExtType = (Equipment[3]&15)+((Equipment[8]>>4)&1)*16 +((Equipment[14]>>4)&15)*32;
	if(ExtType == 0x1FF)
	{
		c->BodyPart[BODYPART_GLOVES].Type = MODEL_BODY_GLOVES + GetSkinModelIndex(c->Class);
	}
	else
	{
		c->BodyPart[BODYPART_GLOVES].Type = MODEL_GLOVES+ExtType;
	}
	
	ExtType = (Equipment[4]>>4)+((Equipment[8]>>3)&1)*16 +(Equipment[14]&15)*32;
	if(ExtType == 0x1FF)
	{
		c->BodyPart[BODYPART_BOOTS].Type = MODEL_BODY_BOOTS+GetSkinModelIndex(c->Class);
	}
	else
	{
		c->BodyPart[BODYPART_BOOTS].Type = MODEL_BOOTS+ExtType;
	}

	c->BodyPart[BODYPART_HELM  ].Level = LevelConvert((Level>> 6)&7);
	c->BodyPart[BODYPART_ARMOR ].Level = LevelConvert((Level>> 9)&7);
	c->BodyPart[BODYPART_PANTS ].Level = LevelConvert((Level>>12)&7);
	c->BodyPart[BODYPART_GLOVES].Level = LevelConvert((Level>>15)&7);
	c->BodyPart[BODYPART_BOOTS ].Level = LevelConvert((Level>>18)&7);
	
	c->BodyPart[BODYPART_HELM  ].Option1 = (Equipment[9]&128)/128;
	c->BodyPart[BODYPART_ARMOR ].Option1 = (Equipment[9]&64)/64;
	c->BodyPart[BODYPART_PANTS ].Option1 = (Equipment[9]&32)/32;
	c->BodyPart[BODYPART_GLOVES].Option1 = (Equipment[9]&16)/16;
	c->BodyPart[BODYPART_BOOTS ].Option1 = (Equipment[9]&8)/8;

	c->BodyPart[BODYPART_HELM  ].ExtOption = (Equipment[10]&128)/128;
	c->BodyPart[BODYPART_ARMOR ].ExtOption = (Equipment[10]&64)/64;  
	c->BodyPart[BODYPART_PANTS ].ExtOption = (Equipment[10]&32)/32;  
	c->BodyPart[BODYPART_GLOVES].ExtOption = (Equipment[10]&16)/16;  
	c->BodyPart[BODYPART_BOOTS ].ExtOption = (Equipment[10]&8)/8;    

	c->Weapon[0].Option1 = (Equipment[9]&4)/4;
	c->Weapon[1].Option1 = (Equipment[9]&2)/2;
	c->Weapon[0].ExtOption = (Equipment[10]&4)/4;
	c->Weapon[1].ExtOption = (Equipment[10]&2)/2;

    c->ExtendState = Equipment[10]&0x01;

#ifndef GUILD_WAR_EVENT
    ChangeChaosCastleUnit ( c );
#endif //GUILD_WAR_EVENT
    
	SetCharacterScale(c);	
}

extern int HeroIndex;

void Setting_Monster(CHARACTER *c,int Type,int PositionX,int PositionY)
{
	OBJECT *o;
#ifdef _DEBUG
    int iRealType = Type;
	//Type = 310;
#endif	// _DEBUG

	int nCastle = BLOODCASTLE_NUM + (World - WD_11BLOODCASTLE_END );
	if(nCastle > 0 && nCastle <= BLOODCASTLE_NUM)
	{
		if(Type >= 84 && Type <= 143 )
		{
			c->Level = 0;
			c->Object.Scale += int(nCastle/3)*0.05f;
		}
	}

	if(c != NULL)
	{
		o = &c->Object;
		for(int i=0;i<MAX_MONSTER;i++)
		{
			if(Type == MonsterScript[i].Type)
			{
	     		strcpy(c->ID,MonsterScript[i].Name);
	
				break;
			}
		}
#ifdef _DEBUG
		Type = iRealType;
		char szMonsterIndex[16];
		sprintf(szMonsterIndex, "(%d)", Type);
		strcat(c->ID, szMonsterIndex);
#endif // _DEBUG
		c->MonsterIndex = Type;
		c->Object.ExtState = 0;
		c->TargetCharacter = HeroIndex;
		if(Type == 200)
          	o->Kind = KIND_MONSTER;
        else if ( Type>=260 )	
            o->Kind = KIND_MONSTER;
		else if(Type > 200)
          	o->Kind = KIND_NPC;
		else if(Type >= 150)
			o->Kind = KIND_MONSTER;
        else if( Type>110 )
			o->Kind = KIND_MONSTER;
		else if(Type >= 100)
			o->Kind = KIND_TRAP;
		else
			o->Kind = KIND_MONSTER;
		//c->Object.Kind = KIND_EDIT;
		//sprintf(c->ID,"%x",Key);
		if( Type == 368 || Type == 369 || Type == 370 )
			o->Kind = KIND_NPC;
		if(Type == 367
		|| Type == 371
#ifdef CSK_CHAOS_CARD
		|| Type == 375
#endif // CSK_CHAOS_CARD		
		|| Type == 376 || Type == 377
		|| Type == 379
		|| Type == 380 || Type == 381 || Type == 382
		|| Type == 383 || Type == 384 || Type == 385
#ifdef ADD_NPC_DEVIN
		|| Type == 406
#endif	// ADD_NPC_DEVIN
		|| Type == 407
		|| Type == 408
#ifdef PRUARIN_EVENT07_3COLORHARVEST
		|| Type == 414
#endif // PRUARIN_EVENT07_3COLORHARVEST
		|| Type == 415 || Type == 416 || Type == 417
#ifdef CSK_EVENT_CHERRYBLOSSOM
		|| Type == 450
#endif //CSK_EVENT_CHERRYBLOSSOM
#ifdef ADD_SOCKET_MIX
		|| Type == 452 || Type == 453
#endif	// ADD_SOCKET_MIX
#ifdef PSW_ADD_RESET_CHARACTER_POINT
		|| Type == 464
#endif //PSW_ADD_RESET_CHARACTER_POINT		
#ifdef LDK_ADD_SANTA_NPC
		|| Type == 465
#endif //LDK_ADD_SANTA_NPC
#ifdef LDK_ADD_SNOWMAN_NPC
		|| Type == 467
#endif //LDK_ADD_SNOWMAN_NPC
#ifdef PBG_ADD_LITTLESANTA_NPC
		|| Type == 468 || Type == 469 || Type == 470	//NPC 1~8
		|| Type == 471 || Type == 472 || Type == 473
		|| Type == 474 || Type == 475
#endif //PBG_ADD_LITTLESANTA_NPC
#ifdef KJH_PBG_ADD_SEVEN_EVENT_2008
		|| Type == 478
#endif //KJH_PBG_ADD_SEVEN_EVENT_2008
#ifdef YDG_ADD_NEW_DUEL_NPC
		|| Type == 479
#endif	// YDG_ADD_NEW_DUEL_NPC
#ifdef LDK_ADD_GAMBLE_NPC_MOSS
		|| Type == 492
#endif //LDK_ADD_GAMBLE_NPC_MOSS
#ifdef YDG_ADD_DOPPELGANGER_NPC
		|| Type == 540
		|| Type == 541
		|| Type == 542
#endif	// YDG_ADD_DOPPELGANGER_NPC
#ifdef LDS_ADD_EG_4_MONSTER_JELINT
		|| Type == 522
#endif //LDS_ADD_EG_4_MONSTER_JELINT
#ifdef ASG_ADD_GENS_NPC
		|| Type == 543 || Type == 544
#endif	// ASG_ADD_GENS_NPC
#ifdef LDS_ADD_NPC_UNITEDMARKETPLACE
		|| Type == 545
		|| Type == 546
		|| Type == 547
#endif // LDS_ADD_NPC_UNITEDMARKETPLACE
#ifdef ASG_ADD_TIME_LIMIT_QUEST_NPC
		|| Type == 566 || Type == 567 || Type == 568
#endif	// ASG_ADD_TIME_LIMIT_QUEST_NPC
#ifdef ASG_ADD_KARUTAN_NPC
		|| Type == 577 || Type == 578
#endif	// ASG_ADD_KARUTAN_NPC
#ifdef LEM_ADD_LUCKYITEM
		|| Type == 579
#endif // LEM_ADD_LUCKYITEM
		)
			{
				o->Kind = KIND_NPC;	
			}
#ifdef PBG_ADD_PKFIELD
		if(Type >= 480 && Type <= 491)
		{
			o->Kind = KIND_MONSTER;
		}
#endif //PBG_ADD_PKFIELD
#ifdef PSW_EVENT_CHERRYBLOSSOMTREE
		if( Type == 451 )
		{
			o->Kind = KIND_TMP;	
		}
#endif //PSW_EVENT_CHERRYBLOSSOMTREE		
	}
}

CHARACTER *CreateMonster(int Type,int PositionX,int PositionY,int Key)
{
#ifdef _DEBUG
// 	int iRealType = Type;		//. 임의로 몬스터를 띄우고 싶다면 각자의 몬스터생성 함수 안에서 임의로 지정
// 	Type = (rand()%2 == 0 ? 476 : 466);
//	Type = 511;

//	int arrTypes[] = {504, 505, 507, 509, 511, 513, 514, 517};
//	Type = arrTypes[rand()%8];

//	Type = 46;	// 베파르
//	Type = 504;	// 가이온
//	Type = 505;	// 제린트
//	Type = 506;	// 레이몬드	
//	Type = 507;	// 에르칸느
//	Type = 509;	// 버몬트
// 	if( 6 == Type )		// 리치
// 	{
// 		Type = 504;		// 가이온
// 	}
//	Type = 518;	// 수호군 방패병
//	Type = 463;	// 우산몬스터
	
#endif	// _DEBUG
	CHARACTER *c = NULL;
	OBJECT *o;
	int Level;

	c = g_CursedTemple->CreateCharacters(Type, PositionX, PositionY, Key);
	if(c != NULL)
	{
		Setting_Monster(c, Type, PositionX, PositionY);
		return c;
	}
    c = CreateHellasMonster ( Type, PositionX, PositionY, Key );
    if ( c!=NULL ) 
	{
		Setting_Monster(c,Type, PositionX, PositionY);
		return c;
	}

    c = battleCastle::CreateBattleCastleMonster( Type, PositionX, PositionY, Key );
    if ( c!=NULL )
	{
		Setting_Monster(c,Type, PositionX, PositionY);
		return c;
	}

	c = M31HuntingGround::CreateHuntingGroundMonster(Type, PositionX, PositionY, Key);
	if ( c!=NULL )
	{
		Setting_Monster(c,Type, PositionX, PositionY);
		return c;
	}

#ifdef CRYINGWOLF_2NDMVP
	c = M34CryingWolf2nd::CreateCryingWolf2ndMonster(Type, PositionX, PositionY, Key);
	if ( c!=NULL )
	{
		Setting_Monster(c,Type, PositionX, PositionY);
		return c;
	}
#endif // CRYINGWOLF_2NDMVP

	c = M34CryWolf1st::CreateCryWolf1stMonster(Type, PositionX, PositionY, Key);
	if ( c!=NULL ) 
	{
		Setting_Monster(c,Type, PositionX, PositionY);
		return c;
	}

	c = M33Aida::CreateAidaMonster(Type, PositionX, PositionY, Key);
	if ( c!=NULL )
	{
		Setting_Monster(c,Type, PositionX, PositionY);
		return c;
	}

	c = M37Kanturu1st::CreateKanturu1stMonster(Type, PositionX, PositionY, Key);
	if(c != NULL)
	{
		Setting_Monster(c, Type, PositionX, PositionY);
		return c;
	}

	c = M38Kanturu2nd::Create_Kanturu2nd_Monster(Type, PositionX, PositionY, Key);
	if(c != NULL)
	{
		Setting_Monster(c, Type, PositionX, PositionY);
		return c;
	}

	c = M39Kanturu3rd::CreateKanturu3rdMonster(Type, PositionX, PositionY, Key);
	if ( c!=NULL )
	{
		Setting_Monster(c,Type, PositionX, PositionY);
		return c;
	}
	
	c = SEASON3A::CGM3rdChangeUp::Instance().CreateBalgasBarrackMonster(Type, PositionX, PositionY, Key);
	if(c != NULL)
	{
		Setting_Monster(c, Type, PositionX, PositionY);
		return c;
	}
	
	c = g_NewYearsDayEvent->CreateMonster(Type, PositionX, PositionY, Key);
	if(c != NULL)
	{
		Setting_Monster(c, Type, PositionX, PositionY);
		return c;
	}

	c = SEASON3B::GMNewTown::CreateNewTownMonster(Type, PositionX, PositionY, Key);
	if(c != NULL)
	{
		Setting_Monster(c, Type, PositionX, PositionY);
		return c;
	}

	c = SEASON3C::GMSwampOfQuiet::CreateSwampOfQuietMonster(Type, PositionX, PositionY, Key);
	if(c != NULL)
	{
		Setting_Monster(c, Type, PositionX, PositionY);
		return c;
	}

#ifdef KJH_ADD_09SUMMER_EVENT
	c = g_09SummerEvent->CreateMonster(Type, PositionX, PositionY, Key);
	if( c != NULL )
	{
		Setting_Monster(c, Type, PositionX, PositionY);
		return c;
	}
#endif // KJH_ADD_09SUMMER_EVENT

#ifdef PSW_ADD_MAPSYSTEM
	c = TheMapProcess().CreateMonster( Type, PositionX, PositionY, Key );
	if(c != NULL)
	{
		Setting_Monster(c, Type, PositionX, PositionY);
		return c;
	}
#endif //PSW_ADD_MAPSYSTEM
	switch(Type)
	{
	case 224:
		OpenNpc ( MODEL_NPC_CLERK );        //  
		c = CreateCharacter ( Key, MODEL_NPC_CLERK, PositionX, PositionY );
        c->m_bFixForm = true;
		c->Object.Scale = 1.f;
        c->Object.SubType = rand()%2+10;
		c->Weapon[0].Type = -1;
		c->Weapon[1].Type = -1;
		strcpy( c->ID, "근위병" );
		break;		
#ifdef ADD_ELF_SUMMON
    case 276:
    	OpenMonsterModel(53);
		c = CreateCharacter(Key,MODEL_MONSTER01+53,PositionX,PositionY);
		c->Object.Scale = 1.45f;
		c->Weapon[0].Type = MODEL_SWORD+17;//MODEL_SWORD+15;
		c->Weapon[0].Level = 5;
        break;
#endif // ADD_ELF_SUMMON
    case 152:
    case 153:
    case 154:
    case 155:
    case 156:
    case 157:
	case 158:
		c = CreateCharacter(Key,MODEL_WARCRAFT,PositionX,PositionY);
        c->m_bFixForm = true;
		c->Weapon[0].Type = -1;
		c->Weapon[0].Level = 0;
		c->Object.Scale = 1.f;
        c->HideShadow = false;
		o = &c->Object;
        o->PriorAnimationFrame = 10.f;
        o->AnimationFrame = 10;
        o->BlendMesh = -1;
		strcpy(c->ID,"");
       break;
    case 162 :
    case 164 :
    case 166 :
    case 168 :
    case 170 :
    case 172 :
	case 426:
        {
		    OpenMonsterModel(70);
		    c = CreateCharacter(Key,MODEL_MONSTER01+70,PositionX,PositionY);
		    c->Object.Scale = 0.9f;
		    o = &c->Object;

		    c->Weapon[0].Type = MODEL_SWORD+16;
		    c->Weapon[0].Level= 0;
		    c->Weapon[1].Type = MODEL_SWORD+16;
		    c->Weapon[1].Level= 0;
        }
        break;

    case 163 :
    case 165 :
    case 167 :
    case 169 :
    case 171 :
    case 173 :
	case 427:
        {
            int randType = 0;

            randType = rand()%2;
        
            OpenMonsterModel(71+randType);
		    c = CreateCharacter(Key,MODEL_MONSTER01+71+randType,PositionX,PositionY);
		    c->Object.Scale = 0.9f;
		    o = &c->Object;

		    c->Weapon[0].Type = -1;
		    c->Weapon[0].Level= 0;
		    c->Weapon[1].Type = -1;
		    c->Weapon[1].Level= 0;

            if ( randType==0 )
            {
    		    c->Weapon[0].Type = MODEL_BOW+19;
    		    c->Weapon[0].Level= 0;
            }
            else
            {
                c->Weapon[0].Type = MODEL_STAFF+5;
		        c->Weapon[0].Level= 0;
            }
        }
        break;
    case 89:
    case 95:
    case 112:
    case 118:
    case 124:
    case 130:
    case 143:
	case 433:
    	OpenMonsterModel(62);
		c = CreateCharacter(Key,MODEL_MONSTER01+62,PositionX,PositionY);
		c->Weapon[0].Type = MODEL_STAFF;
		c->Weapon[0].Level = 11;
		c->Object.Scale = 1.2f;
		strcpy(c->ID,"마법해골");
        break;
    case 131 :
    	OpenMonsterModel(61);
		c = CreateCharacter(Key,MODEL_MONSTER01+61,PositionX,PositionY);
        c->m_bFixForm = true;
		c->Object.Scale = 0.8f;
        c->Object.EnableShadow = false;
		strcpy(c->ID,"성문");
        break;
    case 132 :
    	OpenMonsterModel(60);
		c = CreateCharacter(Key,MODEL_MONSTER01+60,PositionX,PositionY);
        c->m_bFixForm = true;
		c->Object.Scale = 0.8f;
        c->Object.EnableShadow = false;
		strcpy(c->ID,"성자의석관");
		break;
	case 133:
		OpenMonsterModel(60);
		c = CreateCharacter(Key,MODEL_MONSTER01+60,PositionX,PositionY);
        c->m_bFixForm = true;
		c->Object.Scale = 0.8f;
        c->Object.EnableShadow = false;
		strcpy(c->ID,"성자의석관");
		break;
	case 134:
		OpenMonsterModel(60);
		c = CreateCharacter(Key,MODEL_MONSTER01+60,PositionX,PositionY);
        c->m_bFixForm = true;
		c->Object.Scale = 0.8f;
        c->Object.EnableShadow = false;
		strcpy(c->ID,"성자의석관");
        break;
	case 84 :
	case 90 :
	case 96 :
	case 113 :
	case 119 :
	case 125 :
    case 138:
	case 428:
		OpenMonsterModel(47);
		c = CreateCharacter(Key,MODEL_MONSTER01+47,PositionX,PositionY);
		c->Object.Scale = 1.1f;
		o = &c->Object;
		break;
	case 85 :
	case 91 :
	case 97 :
	case 114 :
	case 120 :
	case 126 :
    case 139:
	case 429:
    	OpenMonsterModel(46);
		c = CreateCharacter(Key,MODEL_MONSTER01+46,PositionX,PositionY);
		c->Object.Scale = 1.1f;
		c->Weapon[1].Type = MODEL_BOW+3;
		c->Weapon[1].Level = 1;
		o = &c->Object;
		break;
	case 86 :
	case 92 :
	case 98 :
	case 115 :
	case 121 :
	case 127 :
    case 140:
	case 430:
    	OpenMonsterModel(59);
		c = CreateCharacter(Key,MODEL_MONSTER01+59,PositionX,PositionY);
		c->Weapon[0].Type = MODEL_AXE+8;
		c->Weapon[0].Level = 0;
		c->Weapon[1].Type = MODEL_AXE+8;
		c->Weapon[1].Level = 0;
		c->Object.Scale = 1.0f;
		strcpy(c->ID,"흑해골전사");
        break;
    case 87 :
	case 93 :
	case 99 :
	case 116 :
	case 122 :
	case 128 :
    case 141:
	case 431:
    	OpenMonsterModel(58);
		c = CreateCharacter(Key,MODEL_MONSTER01+58,PositionX,PositionY);
		c->Object.Scale = 0.8f;
		strcpy(c->ID,"자이언트오우거");
        break;
    case 88 :
	case 94 :
	case 111 :
	case 117 :
	case 123 :
	case 129 :
    case 142:
	case 432:
    	OpenMonsterModel(57);
		c = CreateCharacter(Key,MODEL_MONSTER01+57,PositionX,PositionY);
		c->Weapon[0].Type = MODEL_MACE+6;

		if( !int((7 + (World - WD_11BLOODCASTLE_END ))/3) )
			c->Weapon[0].Level = 8;
		else
			c->Weapon[0].Level = 0;
		
		c->Object.Scale = 1.19f;
		strcpy(c->ID,"붉은해골기사");
        break;

#ifdef USE_EVENT_ELDORADO
	case 78:
    	OpenMonsterModel(19);
		c = CreateCharacter(Key,MODEL_MONSTER01+19,PositionX,PositionY);
		c->Weapon[0].Type = MODEL_AXE;
		c->Weapon[0].Level = 9;
		c->Object.Scale = 0.8f;
		strcpy(c->ID,"고블린");
		break;
	case 79:
    	OpenMonsterModel(31);
		c = CreateCharacter(Key,MODEL_MONSTER01+31,PositionX,PositionY);
		strcpy(c->ID,"드래곤");
		c->Object.Scale = 0.9f;
		break;
	case 80:
    	OpenMonsterModel(36);
		c = CreateCharacter(Key,MODEL_MONSTER01+36,PositionX,PositionY);
		c->Object.Scale = 1.4f;
		c->Weapon[0].Type = MODEL_STAFF+7;
		c->Weapon[0].Option1 = 63;
		break;
	case 81://인어
    	OpenMonsterModel(34);
		c = CreateCharacter(Key,MODEL_MONSTER01+34,PositionX,PositionY);
		c->Object.Scale = 1.f;
		break;
	case 82:	// 데블 탄탈로스
    	OpenMonsterModel(42);
		c = CreateCharacter(Key,MODEL_MONSTER01+42,PositionX,PositionY);
		c->Object.BlendMesh = 2;
		c->Object.BlendMeshLight = 1.f;
   		o = &c->Object;
    	c->Object.Scale = 1.8f;
      	c->Weapon[0].Type = MODEL_SWORD+16;
		c->Weapon[0].Option1 = 63;
   		CreateJoint(BITMAP_JOINT_ENERGY,o->Position,o->Position,o->Angle,2,o,30.f);
   		CreateJoint(BITMAP_JOINT_ENERGY,o->Position,o->Position,o->Angle,3,o,30.f);
		break;
	case 83:	// 골드휠
    	OpenMonsterModel(41);
		c = CreateCharacter(Key,MODEL_MONSTER01+41,PositionX,PositionY);
		c->Object.Scale = 1.4f;
		c->Weapon[0].Type = MODEL_BOW+14;
		c->Weapon[0].Option1 = 63;
		//c->Weapon[0].Type = MODEL_BOW+16;
   		o = &c->Object;
   		CreateJoint(BITMAP_JOINT_ENERGY,o->Position,o->Position,o->Angle,2,o,30.f);
   		CreateJoint(BITMAP_JOINT_ENERGY,o->Position,o->Position,o->Angle,3,o,30.f);
		break;
#endif
    case 68:    //  깃털괴물.
    	OpenMonsterModel(49);
		c = CreateCharacter(Key,MODEL_MONSTER01+49,PositionX,PositionY);
		c->Object.Scale = 1.4f;
        break;

    case 69:    //  별자리.
    	OpenMonsterModel(50);
		c = CreateCharacter(Key,MODEL_MONSTER01+50,PositionX,PositionY);
		c->Object.Scale = 1.f;
        c->Object.BlendMesh = 0;
        break;
    case 70:    //  비의여왕.
    	OpenMonsterModel(51);
		c = CreateCharacter(Key,MODEL_MONSTER01+51,PositionX,PositionY);
		c->Object.Scale = 1.3f;
        c->Object.BlendMesh = -2;
        c->Object.BlendMeshLight = 1.f;
		c->Object.m_bRenderShadow = false;
        break;

#ifdef LOREN_RAVINE_EVENT
	case 301:
#endif

    case 71:    //  야누스.
	case 74:	//  야누스2
    	OpenMonsterModel(52);
		c = CreateCharacter(Key,MODEL_MONSTER01+52,PositionX,PositionY);
		if ( 71 == Type)
		{
			c->Object.Scale = 1.1f;
			c->Weapon[0].Type = MODEL_SWORD+18;
			c->Weapon[0].Level = 5;
			c->Weapon[1].Type = MODEL_SHIELD+14;
			c->Weapon[1].Level = 0;
		}
		else
		{
			c->Object.Scale = 1.3f;
			c->Weapon[0].Type = MODEL_SWORD+18;
			c->Weapon[0].Level = 9;
			c->Weapon[1].Type = MODEL_SHIELD+14;
			c->Weapon[1].Level = 9;
		}
        c->Object.BlendMesh = 1;
        c->Object.BlendMeshLight = 1.f;
		//Models[MODEL_MONSTER01+52].StreamMesh = 1;
        break;

    case 72:    //  이카루스.
    	OpenMonsterModel(53);
		c = CreateCharacter(Key,MODEL_MONSTER01+53,PositionX,PositionY);
		c->Object.Scale = 1.45f;
		c->Weapon[0].Type = MODEL_SWORD+17;//MODEL_SWORD+15;
		c->Weapon[0].Level = 5;
        break;

    case 73:    //  흑룡.
	case 75://흑룡2
    	OpenMonsterModel(54);
		c = CreateCharacter(Key,MODEL_MONSTER01+54,PositionX,PositionY);
        c->m_bFixForm = true;
		if ( Type == 75)
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
	case 77:	// 불사조
		{
    		OpenMonsterModel(55);
			OpenMonsterModel(56);
			c = CreateCharacter(Key,MODEL_MONSTER01+55,PositionX,PositionY);
            c->m_bFixForm = true;
			c->Object.Scale = 1.0f;
			Models[MODEL_MONSTER01+55].StreamMesh = 0;
		}
		break;
	case 64://오크 궁수 (46, 47, 48)
    	OpenMonsterModel(46);
		c = CreateCharacter(Key,MODEL_MONSTER01+46,PositionX,PositionY);
		c->Object.Scale = 1.2f;
		c->Weapon[1].Type = MODEL_BOW+3;
		c->Weapon[1].Level = 3;
		o = &c->Object;
		o->HiddenMesh = 1;
		break;
	case 137:	// 오크 궁수
    	OpenMonsterModel(46);
		c = CreateCharacter(Key,MODEL_MONSTER01+46,PositionX,PositionY);
		c->Object.Scale = 1.2f;
		c->Weapon[1].Type = MODEL_BOW+3;
		c->Weapon[1].Level = 5;
		o = &c->Object;
		o->HiddenMesh = 1;
		break;
	case 65:	//오크 대장
    	OpenMonsterModel(47);
		c = CreateCharacter(Key,MODEL_MONSTER01+47,PositionX,PositionY);
		c->Object.Scale = 1.3f;
		o = &c->Object;
		o->HiddenMesh = 2;
		break;
	case 136:	// 오크 병사
    	OpenMonsterModel(47);
		c = CreateCharacter(Key,MODEL_MONSTER01+47,PositionX,PositionY);
		c->Object.Scale = 1.3f;
		o = &c->Object;
		o->HiddenMesh = 2;
		break;
	case 66:	//저주받은 왕
	case 135:	// 백색의 마법사
    	OpenMonsterModel(48);
		c = CreateCharacter(Key,MODEL_MONSTER01+48,PositionX,PositionY);
		c->Object.Scale = 1.7f;
		o = &c->Object;
		break;

#ifdef YDG_ADD_SANTA_MONSTER
	case 466:	// 저주받은 고블린
    	OpenMonsterModel(156);
		c = CreateCharacter(Key,MODEL_MONSTER01+156,PositionX,PositionY);
		c->Object.Scale = 0.9f;
		strcpy(c->ID,"저주받은 고블린");
		o = &c->Object;
		break;
	case 476:	// 저주받은 산타
    	OpenMonsterModel(155);
		c = CreateCharacter(Key,MODEL_MONSTER01+155,PositionX,PositionY);
		c->Object.Scale = 1.7f;
		strcpy(c->ID,"저주받은 산타");
		o = &c->Object;
		break;
#endif	// YDG_ADD_SANTA_MONSTER
		
#ifdef LOREN_RAVINE_EVENT
	case 300:
#endif

	case 62://뮤턴트
    	OpenMonsterModel(45);
		c = CreateCharacter(Key,MODEL_MONSTER01+45,PositionX,PositionY);
		c->Object.Scale = 1.5f;
   		o = &c->Object;
   		CreateJoint(BITMAP_JOINT_ENERGY,o->Position,o->Position,o->Angle,2,o,30.f);
   		CreateJoint(BITMAP_JOINT_ENERGY,o->Position,o->Position,o->Angle,3,o,30.f);
		break;
	case 63://데쓰빔나이트
	case 61://빔나이트
    	OpenMonsterModel(44);
		c = CreateCharacter(Key,MODEL_MONSTER01+44,PositionX,PositionY);
		if(Type == 63)
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
   		CreateJoint(BITMAP_JOINT_ENERGY,o->Position,o->Position,o->Angle,2,o,30.f);
   		CreateJoint(BITMAP_JOINT_ENERGY,o->Position,o->Position,o->Angle,3,o,30.f);
		break;
	case 60://블러드울프
    	OpenMonsterModel(43);
		c = CreateCharacter(Key,MODEL_MONSTER01+43,PositionX,PositionY);
   		c->Object.Scale = 2.2f;
   		o = &c->Object;
   		CreateJoint(BITMAP_JOINT_ENERGY,o->Position,o->Position,o->Angle,2,o,30.f);
   		CreateJoint(BITMAP_JOINT_ENERGY,o->Position,o->Position,o->Angle,3,o,30.f);
		break;
	case 58://자이칸
	case 59://탄탈로스
    	OpenMonsterModel(42);
		c = CreateCharacter(Key,MODEL_MONSTER01+42,PositionX,PositionY);
		c->Object.BlendMesh = 2;
		c->Object.BlendMeshLight = 1.f;
   		o = &c->Object;
		if(Type == 58)
		{
    		c->Object.Scale = 1.8f;
      		c->Weapon[0].Type = MODEL_SWORD+16;
		}
		else
		{
    		c->Object.Scale = 2.1f;
			o->SubType = 1;
      		c->Weapon[0].Type = MODEL_STAFF+8;
		}
   		CreateJoint(BITMAP_JOINT_ENERGY,o->Position,o->Position,o->Angle,2,o,30.f);
   		CreateJoint(BITMAP_JOINT_ENERGY,o->Position,o->Position,o->Angle,3,o,30.f);
	break;
	case 57://아이언휠
    	OpenMonsterModel(41);
		c = CreateCharacter(Key,MODEL_MONSTER01+41,PositionX,PositionY);
		c->Object.Scale = 1.4f;
		c->Weapon[0].Type = MODEL_BOW+14;
		//c->Weapon[0].Type = MODEL_BOW+16;
   		o = &c->Object;
   		CreateJoint(BITMAP_JOINT_ENERGY,o->Position,o->Position,o->Angle,2,o,30.f);
   		CreateJoint(BITMAP_JOINT_ENERGY,o->Position,o->Position,o->Angle,3,o,30.f);
		break;
	case 52://실버발키리
    	OpenMonsterModel(35);
		c = CreateCharacter(Key,MODEL_MONSTER01+35,PositionX,PositionY);
		c->Object.Scale = 1.4f;
		c->Weapon[0].Type = MODEL_BOW+13;
		break;
	case 51://대형물고기
    	OpenMonsterModel(33);
		c = CreateCharacter(Key,MODEL_MONSTER01+33,PositionX,PositionY);
		c->Object.Scale = 1.f;
		c->Level = 1;
		break;
	case 50://함정
    	OpenMonsterModel(38);
		c = CreateCharacter(Key,MODEL_MONSTER01+38,PositionX,PositionY);
		c->Object.Scale = 1.8f;
		break;
	case 49://히드라
    	OpenMonsterModel(37);
		c = CreateCharacter(Key,MODEL_MONSTER01+37,PositionX,PositionY);
		c->Object.Scale = 1.f;
		c->Object.BlendMesh = 5;
		c->Object.BlendMeshLight = 0.f;
		break;
	case 48://리자드킹
    	OpenMonsterModel(36);
		c = CreateCharacter(Key,MODEL_MONSTER01+36,PositionX,PositionY);
		c->Object.Scale = 1.4f;
		c->Weapon[0].Type = MODEL_STAFF+6;
		break;
	case 47://발키리
    	OpenMonsterModel(35);
		c = CreateCharacter(Key,MODEL_MONSTER01+35,PositionX,PositionY);
		c->Object.Scale = 1.1f;
		c->Weapon[0].Type = MODEL_BOW+13;
		c->Object.BlendMesh = 0;
		c->Object.BlendMeshLight = 1.f;
		break;
	case 46://인어
    	OpenMonsterModel(34);
		c = CreateCharacter(Key,MODEL_MONSTER01+34,PositionX,PositionY);
		c->Object.Scale = 1.f;
		break;
	case 45://물고기
    	OpenMonsterModel(33);
		c = CreateCharacter(Key,MODEL_MONSTER01+33,PositionX,PositionY);
		c->Object.Scale = 0.6f;
		break;
	case 150:
    	OpenMonsterModel(32);
		c = CreateCharacter(Key,MODEL_MONSTER01+32,PositionX,PositionY);
		strcpy(c->ID,"발리");
		c->Object.Scale = 0.12f;
		break;
	case 44:
    	OpenMonsterModel(31);
		c = CreateCharacter(Key,MODEL_MONSTER01+31,PositionX,PositionY);
		strcpy(c->ID,"드래곤");
		c->Object.Scale = 0.9f;
		break;
	case 43:
    	OpenMonsterModel(2);
		c = CreateCharacter(Key,MODEL_MONSTER01+2,PositionX,PositionY);
		strcpy(c->ID,"황금버지드래곤");
		c->Object.Scale = 0.7f;
		break;
	case 42:
    	OpenMonsterModel(31);
		c = CreateCharacter(Key,MODEL_MONSTER01+31,PositionX,PositionY);
		strcpy(c->ID,"쿤둔");
		c->Object.Scale = 1.3f;
      	Vector(200.f,150.f,280.f,c->Object.BoundingBoxMax);
		break;
	case 41:
    	OpenMonsterModel(30);
		c = CreateCharacter(Key,MODEL_MONSTER01+30,PositionX,PositionY);
		strcpy(c->ID,"데쓰 카우");
		c->Weapon[0].Type = MODEL_MACE+3;
		//c->Weapon[0].Type = MODEL_SWORD+14;
		c->Object.Scale = 1.1f;
		//c->Level = 1;
		break;
	case 40:
    	OpenMonsterModel(29);
		c = CreateCharacter(Key,MODEL_MONSTER01+29,PositionX,PositionY);
		strcpy(c->ID,"데쓰 나이트");
		c->Weapon[0].Type = MODEL_SWORD+15;
		c->Weapon[0].Type = MODEL_SWORD+14;
		//c->Weapon[1].Type = MODEL_SHIELD+8;
		c->Object.Scale = 1.3f;
		//c->Level = 1;
		break;
	case 39:
    	OpenMonsterModel(28);
		c = CreateCharacter(Key,MODEL_MONSTER01+28,PositionX,PositionY);
		strcpy(c->ID,"포이즌 쉐도우");
		c->Object.Scale = 1.2f;
		c->Level = 1;
		break;
	case 38:
	case 67:	//발록2
    	OpenMonsterModel(27);
		c = CreateCharacter(Key,MODEL_MONSTER01+27,PositionX,PositionY);
		strcpy(c->ID,"발록");
		c->Weapon[0].Type = MODEL_SPEAR+9;
		c->Weapon[0].Level = 9;
		c->Object.Scale = 1.6f;
		break;
	case 37:
    	OpenMonsterModel(26);
		c = CreateCharacter(Key,MODEL_MONSTER01+26,PositionX,PositionY);
		strcpy(c->ID,"데빌");
		c->Object.Scale = 1.1f;
		break;
	case 36:
    	OpenMonsterModel(28);
		c = CreateCharacter(Key,MODEL_MONSTER01+28,PositionX,PositionY);
		strcpy(c->ID,"쉐도우");
		c->Object.Scale = 1.2f;
		break;
    	/*OpenMonsterModel(7);
		c = CreateCharacter(Key,MODEL_MONSTER01+7,PositionX,PositionY);
		strcpy(c->ID,"블러드 고스트");
		c->Object.AlphaTarget = 0.4f;
		c->MoveSpeed = 15;
		c->Blood = true;
		c->Object.Scale = 1.1f;
		c->Level = 2;*/
	case 35:
    	OpenMonsterModel(11);
		c = CreateCharacter(Key,MODEL_MONSTER01+11,PositionX,PositionY);
		strcpy(c->ID,"데쓰 고르곤");
		c->Object.Scale = 1.3f;
		c->Weapon[0].Type = MODEL_AXE+8;
		c->Weapon[1].Type = MODEL_AXE+8;
		c->Object.BlendMesh = 1;
		c->Object.BlendMeshLight = 1.f;
		c->Level = 2;
		break;
	case 34:
		c = CreateCharacter(Key,MODEL_PLAYER,PositionX,PositionY);
		strcpy(c->ID,"저주받은 법사");
		c->BodyPart[BODYPART_HELM  ].Type = MODEL_HELM  +3;
		c->BodyPart[BODYPART_ARMOR ].Type = MODEL_ARMOR +3;
		c->BodyPart[BODYPART_PANTS ].Type = MODEL_PANTS +3;
		c->BodyPart[BODYPART_GLOVES].Type = MODEL_GLOVES+3;
		c->BodyPart[BODYPART_BOOTS ].Type = MODEL_BOOTS +3;
		c->Weapon[0].Type = MODEL_STAFF+5;
		c->Weapon[1].Type = MODEL_SHIELD+14;
     	Level = 9;
		c->BodyPart[BODYPART_HELM  ].Level = Level;
		c->BodyPart[BODYPART_ARMOR ].Level = Level;
		c->BodyPart[BODYPART_PANTS ].Level = Level;
		c->BodyPart[BODYPART_GLOVES].Level = Level;
		c->BodyPart[BODYPART_BOOTS ].Level = Level;
		//c->Weapon[0].Level = Level;
		//c->Weapon[1].Level = Level;
		c->PK = PVP_MURDERER2;
		SetCharacterScale(c);
		if ( InDevilSquare() == true )
		{
			c->Object.Scale = 1.0f;
		}
		break;
    	/*OpenMonsterModel(5);
		c = CreateCharacter(Key,MODEL_MONSTER01+5,PositionX,PositionY);
		strcpy(c->ID,"자이언트");
		c->Weapon[0].Type = MODEL_AXE+2;
		c->Weapon[1].Type = MODEL_AXE+2;
		c->Object.Scale = 0.7f;
      	Vector(50.f,50.f,80.f,c->Object.BoundingBoxMax);
		break;*/
	case 33:
    	OpenMonsterModel(19);
		c = CreateCharacter(Key,MODEL_MONSTER01+19,PositionX,PositionY);
		c->Weapon[0].Type = MODEL_MACE+1;
		c->Weapon[1].Type = MODEL_SHIELD+1;
		c->Object.Scale = 1.2f;
		c->Level = 1;
		strcpy(c->ID,"고블린 대장");
		break;
	case 32:
    	OpenMonsterModel(25);
		c = CreateCharacter(Key,MODEL_MONSTER01+25,PositionX,PositionY);
		strcpy(c->ID,"돌괴물");
		break;
	case 31:
    	OpenMonsterModel(24);
		c = CreateCharacter(Key,MODEL_MONSTER01+24,PositionX,PositionY);
		strcpy(c->ID,"아곤");
		c->Object.Scale = 1.3f;
		c->Weapon[0].Type = MODEL_SWORD+8;
		c->Weapon[1].Type = MODEL_SWORD+8;
		break;
	case 30:
    	OpenMonsterModel(23);
		c = CreateCharacter(Key,MODEL_MONSTER01+23,PositionX,PositionY);
		strcpy(c->ID,"숲의괴물");
		c->Object.Scale = 0.75f;
		break;
	case 29:
    	OpenMonsterModel(22);
		c = CreateCharacter(Key,MODEL_MONSTER01+22,PositionX,PositionY);
		strcpy(c->ID,"헌터");
		c->Weapon[0].Type = MODEL_BOW+10;
		c->Object.Scale = 0.95f;
		break;
	case 28:
    	OpenMonsterModel(21);
		c = CreateCharacter(Key,MODEL_MONSTER01+21,PositionX,PositionY);
		c->Weapon[0].Type = MODEL_SPEAR+1;
		c->Object.Scale = 0.8f;
		strcpy(c->ID,"풍뎅이괴물");
		c->Object.BlendMesh = 1;
		break;
	case 27:
    	OpenMonsterModel(20);
		c = CreateCharacter(Key,MODEL_MONSTER01+20,PositionX,PositionY);
		c->Object.Scale = 1.1f;
		strcpy(c->ID,"고리전갈");
		break;
	case 26:
    	OpenMonsterModel(19);
		c = CreateCharacter(Key,MODEL_MONSTER01+19,PositionX,PositionY);
		c->Weapon[0].Type = MODEL_AXE;
		c->Object.Scale = 0.8f;
		strcpy(c->ID,"고블린");
		break;
	case 25:
    	OpenMonsterModel(18);
		c = CreateCharacter(Key,MODEL_MONSTER01+18,PositionX,PositionY);
		c->Weapon[0].Type = MODEL_STAFF+1;
		c->Object.BlendMesh = 2;
		c->Object.BlendMeshLight = 1.f;
		c->Object.Scale = 1.1f;
		c->Object.LightEnable = false;
		c->Level = 3;
		strcpy(c->ID,"아이스퀸");
		break;
	case 24:
    	OpenMonsterModel(17);
		c = CreateCharacter(Key,MODEL_MONSTER01+17,PositionX,PositionY);
		strcpy(c->ID,"웜");
		break;
	case 23:
    	OpenMonsterModel(16);
		c = CreateCharacter(Key,MODEL_MONSTER01+16,PositionX,PositionY);
		c->Weapon[0].Type = MODEL_AXE+7;
		c->Weapon[1].Type = MODEL_SHIELD+10;
		c->Object.Scale = 1.15f;
		strcpy(c->ID,"호머드");
		break;
	case 22:
    	OpenMonsterModel(15);
		c = CreateCharacter(Key,MODEL_MONSTER01+15,PositionX,PositionY);
		c->Object.BlendMesh = 0;
		c->Object.BlendMeshLight = 1.f;
		strcpy(c->ID,"얼음괴물");
		break;
	case 21:
    	OpenMonsterModel(14);
		c = CreateCharacter(Key,MODEL_MONSTER01+14,PositionX,PositionY);
		c->Object.Scale = 0.95f;
		strcpy(c->ID,"암살자");
		break;
	case 20:
    	OpenMonsterModel(13);
		c = CreateCharacter(Key,MODEL_MONSTER01+13,PositionX,PositionY);
		strcpy(c->ID,"설인 대장");
		c->Object.Scale = 1.4f;
		break;
	case 19:
    	OpenMonsterModel(12);
		c = CreateCharacter(Key,MODEL_MONSTER01+12,PositionX,PositionY);
		strcpy(c->ID,"설인");
		c->Object.Scale = 1.1f;
		break;
	case 18:
    	OpenMonsterModel(11);
		c = CreateCharacter(Key,MODEL_MONSTER01+11,PositionX,PositionY);
		strcpy(c->ID,"고르곤");
		c->Object.Scale = 1.5f;
		c->Weapon[0].Type = MODEL_STAFF+4;
		c->Object.BlendMesh = 1;
		c->Object.BlendMeshLight = 1.f;
		break;
	case 3:
    	OpenMonsterModel(9);
		c = CreateCharacter(Key,MODEL_MONSTER01+9,PositionX,PositionY);
		strcpy(c->ID,"거미");
		c->Object.Scale = 0.4f;
		break;
	case 17:
    	OpenMonsterModel(10);
		c = CreateCharacter(Key,MODEL_MONSTER01+10,PositionX,PositionY);
		strcpy(c->ID,"싸이크롭스");
		c->Weapon[0].Type = MODEL_AXE+8;
		//c->Weapon[1].Type = MODEL_MACE+2;
		//c->Object.HiddenMesh = 2;
		break;
	case 0:
	case 4:
	case 8:
	default:
    	OpenMonsterModel(0);
		c = CreateCharacter(Key,MODEL_MONSTER01,PositionX,PositionY);
		if(Type==0)
		{
			c->Object.HiddenMesh = 0;
			strcpy(c->ID,"소뿔전사");
			c->Object.Scale = 0.8f;
			c->Weapon[0].Type = MODEL_AXE+6;
		}
		else if(Type==4)
		{
			c->Weapon[0].Type = MODEL_SPEAR+7;
			strcpy(c->ID,"소뿔전사 대장");
			c->Object.Scale = 1.15f;
			c->Level = 1;
		}
		else if(Type==8)
		{
			c->Weapon[0].Type = MODEL_SPEAR+8;
			strcpy(c->ID,"포이즌 소뿔전사");
			c->Object.Scale = 1.f;
			c->Level = 2;

			g_CharacterRegisterBuff( (&c->Object), eDeBuff_Poison );
		}
		break;
	case 11:
    	OpenMonsterModel(7);
		c = CreateCharacter(Key,MODEL_MONSTER01+7,PositionX,PositionY);
		strcpy(c->ID,"고스트");
		c->Object.AlphaTarget = 0.4f;
		c->MoveSpeed = 15;
		c->Blood = true;
		break;
	case 12:
    	OpenMonsterModel(6);
		c = CreateCharacter(Key,MODEL_MONSTER01+6,PositionX,PositionY);
		strcpy(c->ID,"유충");
		c->Object.Scale = 0.6f;
		break;
	case 13:
    	OpenMonsterModel(8);
		c = CreateCharacter(Key,MODEL_MONSTER01+8,PositionX,PositionY);
		strcpy(c->ID,"헬스파이더");
		c->Weapon[0].Type = MODEL_STAFF+2;
		c->Object.Scale = 1.1f;
		break;
	case 1:
	case 5:
    	OpenMonsterModel(1);
		c = CreateCharacter(Key,MODEL_MONSTER01+1,PositionX,PositionY);
		if(Type==1)
		{
			c->Object.HiddenMesh = 0;
			strcpy(c->ID,"하운드");
			c->Object.Scale = 0.85f;
			c->Weapon[0].Type = MODEL_SWORD+4;
		}
		if(Type==5)
		{
			c->Object.HiddenMesh = 1;
			c->Weapon[0].Type = MODEL_SWORD+7;
			c->Weapon[1].Type = MODEL_SHIELD+9;
			strcpy(c->ID,"헬하운드");
			c->Object.Scale = 1.1f;
			c->Level = 1;
		}
		break;

	case 2:	// 버지드래곤
    	OpenMonsterModel(2);
		c = CreateCharacter(Key, MODEL_MONSTER01+2, PositionX, PositionY);
		strcpy(c->ID,"버지드래곤");
		c->Object.Scale = 0.5f;
		break;
		
	case 10:
    	OpenMonsterModel(3);
		c = CreateCharacter(Key,MODEL_MONSTER01+3,PositionX,PositionY);
		strcpy(c->ID,"다크나이트");
		c->Object.Scale = 0.8f;
		c->Level = 1;
		c->Weapon[0].Type = MODEL_SWORD+13;
		break;
	case 6:
	case 9:
    	OpenMonsterModel(4);
		c = CreateCharacter(Key,MODEL_MONSTER01+4,PositionX,PositionY);
		if(Type==6)
		{
			strcpy(c->ID,"리치");
			c->Weapon[0].Type = MODEL_STAFF+2;
			c->Object.Scale = 0.85f;
		}
		else
		{
			strcpy(c->ID,"썬더 리치");
			c->Weapon[0].Type = MODEL_STAFF+3;
			c->Level = 1;
			c->Object.Scale = 1.1f;
		}
		break;
	case 7:
    	OpenMonsterModel(5);
		c = CreateCharacter(Key,MODEL_MONSTER01+5,PositionX,PositionY);
		strcpy(c->ID,"자이언트");
		c->Weapon[0].Type = MODEL_AXE+2;
		c->Weapon[1].Type = MODEL_AXE+2;
		c->Object.Scale = 1.6f;
		break;

	case 14:    //  해골전사.
	case 55:    //  데쓰킹.
	case 56:    //  데쓰본.
		c = CreateCharacter(Key,MODEL_PLAYER,PositionX,PositionY);
		strcpy(c->ID,"해골전사");
		c->Object.SubType = MODEL_SKELETON1;
		c->Blood = true;
		if(Type==14)
		{
      		c->Object.Scale = 0.95f;
			c->Weapon[0].Type = MODEL_SWORD+6;
			c->Weapon[1].Type = MODEL_SHIELD+4;
		}
		else if(Type==56)
		{
      		c->Object.Scale = 0.8f;
			c->Weapon[0].Type = MODEL_SPEAR+8;
		}
		else
		{
    		c->Level = 1;
      		c->Object.Scale = 1.4f;
			c->Weapon[0].Type = MODEL_SPEAR+9;
		}
		break;
	case 15:	// 해골 궁수
		c = CreateCharacter(Key,MODEL_PLAYER,PositionX,PositionY);
		strcpy(c->ID,"해골궁수");
		c->Object.Scale = 1.1f;
		c->Weapon[1].Type = MODEL_BOW+2;
		c->Object.SubType = MODEL_SKELETON2;
		c->Level = 1;
		c->Blood = true;
		break;
	case 16:	// 해골 전사 대장
		c = CreateCharacter(Key,MODEL_PLAYER,PositionX,PositionY);
		strcpy(c->ID,"해골전사 대장");
		c->Object.Scale = 1.2f;
		c->Weapon[0].Type = MODEL_AXE+3;
		c->Weapon[1].Type = MODEL_SHIELD+6;
		c->Object.SubType = MODEL_SKELETON3;
		c->Level = 1;
		c->Blood = true;
		break;
		// 엘리트 해골전사 모델 데이타 생성
	case 372:
		c = CreateCharacter(Key, MODEL_PLAYER, PositionX, PositionY);
		::strcpy(c->ID, "엘리트 해골전사");
		c->Object.Scale = 0.95f;
		c->Object.SubType = MODEL_SKELETON_PCBANG;
		break;
		// 할로윈 이벤트 모델 데이타 생성
	case 373:	// 373 1 "잭 오랜턴"
		c = CreateCharacter(Key, MODEL_PLAYER, PositionX, PositionY);
		::strcpy(c->ID, "잭 오랜턴");
		c->Object.Scale = 0.95f;
		c->Object.SubType = MODEL_HALLOWEEN;
		break;
		// 크리스마스 이벤트 모델 데이타 생성
	case 374:
		c = CreateCharacter(Key, MODEL_PLAYER, PositionX, PositionY);
		::strcpy(c->ID, "크리스마스 걸");
		c->Object.Scale = 0.85f;
		c->Object.SubType = MODEL_XMAS_EVENT_CHANGE_GIRL;
		break;
		// GM 모델 데이타 생성
	case 378:
		c = CreateCharacter(Key, MODEL_PLAYER, PositionX, PositionY);
		::strcpy(c->ID, "GameMaster");
		c->Object.Scale = 1.0f;
		c->Object.SubType = MODEL_GM_CHARACTER;
		break;
	case 53:
    	OpenMonsterModel(39);
		c = CreateCharacter(Key,MODEL_MONSTER01+39,PositionX,PositionY);
		strcpy(c->ID,"타이탄");
    	c->Object.Scale = 1.8f;
		c->Object.BlendMesh = 2;
		c->Object.BlendMeshLight = 1.f;
   		o = &c->Object;
   		CreateJoint(BITMAP_JOINT_ENERGY,o->Position,o->Position,o->Angle,2,o,30.f);
   		CreateJoint(BITMAP_JOINT_ENERGY,o->Position,o->Position,o->Angle,3,o,30.f);
		break;
	case 54:    //  솔져
	case 151:   //  소환된 솔져.
    	OpenMonsterModel(40);
		c = CreateCharacter(Key,MODEL_MONSTER01+40,PositionX,PositionY);
		strcpy(c->ID,"솔져");
		c->Weapon[1].Type = MODEL_BOW+14;
		if(Type == 54)
          	c->Object.Scale = 1.1f;
		else
          	c->Object.Scale = 1.3f;
		break;
	//  함정.
    case 100:
		c = CreateCharacter(Key,39,PositionX,PositionY);
		break;
	case 101:
		c = CreateCharacter(Key,40,PositionX,PositionY);
		break;
	case 102:
		c = CreateCharacter(Key,51,PositionX,PositionY);
		break;
	case 103://운석
		c = CreateCharacter(Key,25,PositionX,PositionY);
		break;
	case 106:	// 레이져형 트랩
		c = CreateCharacter(Key,51,PositionX,PositionY);
		break;
	case 200://공
		c = CreateCharacter(Key,MODEL_BALL,PositionX,PositionY);
		o = &c->Object;
		o->BlendMesh = 2;
		o->Scale = 1.8f;
		c->Level = 1;
		break;

		//////////////////////////////////////////////////////////////////////////
		//	NPC.
		//////////////////////////////////////////////////////////////////////////
    case 226:   //  조련사 NPC
   		OpenNpc ( MODEL_NPC_BREEDER );
		c = CreateCharacter ( Key, MODEL_NPC_BREEDER, PositionX, PositionY );
		strcpy ( c->ID, "조련사 NPC" );
        break;

#ifdef _PVP_MURDERER_HERO_ITEM
    case 227:
    	OpenNpc(MODEL_MASTER);
		c = CreateCharacter(Key,MODEL_MASTER,PositionX,PositionY);
		strcpy(c->ID,"살인마상점");
        break;

    case 228:
		OpenNpc(MODEL_HERO_SHOP);//    대천사의 전령
		c = CreateCharacter(Key,MODEL_HERO_SHOP,PositionX,PositionY);
		strcpy(c->ID,"영웅상점");
        break;
#endif	// _PVP_MURDERER_HERO_ITEM

	case 229 :	//	말론. ( 퀘스트 NPC ).
		c = CreateCharacter ( Key, MODEL_PLAYER, PositionX, PositionY );
		strcpy ( c->ID, "말론" );
		c->BodyPart[BODYPART_HELM  ].Type = MODEL_HELM  +9;
		c->BodyPart[BODYPART_HELM  ].Level= 7;
		c->BodyPart[BODYPART_ARMOR ].Type = MODEL_ARMOR +9;
		c->BodyPart[BODYPART_ARMOR ].Level= 7;
		c->BodyPart[BODYPART_PANTS ].Type = MODEL_PANTS +9;
		c->BodyPart[BODYPART_PANTS ].Level= 7;
		c->BodyPart[BODYPART_GLOVES].Type = MODEL_GLOVES+9;
		c->BodyPart[BODYPART_GLOVES].Level= 7;
		c->BodyPart[BODYPART_BOOTS ].Type = MODEL_BOOTS +9;
		c->BodyPart[BODYPART_BOOTS ].Level= 7;
		c->Weapon[0].Type = MODEL_SPEAR+7;
		c->Weapon[0].Level= 8;
		c->Weapon[1].Type = -1;
		SetCharacterScale(c);
		break;

    //  데비아스, 로랜시아 추가 상점 NPC
	case 230 :	//	로랜시아 추가 상인.
		OpenNpc(MODEL_MERCHANT_MAN);
		c = CreateCharacter(Key,MODEL_MERCHANT_MAN,PositionX,PositionY);
		strcpy(c->ID,"로랜추가상인");
		c->BodyPart[BODYPART_HELM  ].Type = MODEL_MERCHANT_MAN_HEAD;
		c->BodyPart[BODYPART_ARMOR ].Type = MODEL_MERCHANT_MAN_UPPER+1;
		c->BodyPart[BODYPART_GLOVES].Type = MODEL_MERCHANT_MAN_GLOVES+1;
		c->BodyPart[BODYPART_BOOTS ].Type = MODEL_MERCHANT_MAN_BOOTS;
		break;
	case 231 :	//	데비아스 추가 상인.
    	OpenNpc(MODEL_DEVIAS_TRADER);
		c = CreateCharacter(Key,MODEL_DEVIAS_TRADER,PositionX,PositionY);
		strcpy(c->ID,"데비추가상인");
		break;

	case 232:
		OpenNpc(MODEL_NPC_ARCHANGEL);//    대천사
		c = CreateCharacter(Key,MODEL_NPC_ARCHANGEL,PositionX,PositionY);
        o = &c->Object;
        o->Scale = 1.f;
        o->Kind  = KIND_NPC;
		break;
	case 233:
		OpenNpc(MODEL_NPC_ARCHANGEL_MESSENGER);//    대천사의 전령
		c = CreateCharacter(Key,MODEL_NPC_ARCHANGEL_MESSENGER,PositionX,PositionY);
        o = &c->Object;
        o->Scale = 1.f;
        o->Kind  = KIND_NPC;
		break;
		
    case 234:
    	OpenMonsterModel(19);
		c = CreateCharacter(Key,MODEL_MONSTER01+19,PositionX,PositionY);
		c->Weapon[0].Type = MODEL_STAFF;
        c->Weapon[0].Level= 4;
		c->Object.Scale = 1.5f;
        c->Object.Kind = KIND_NPC;
        SetAction ( &c->Object, 0 );
        break;

    case 235:
		OpenNpc(MODEL_NPC_SEVINA);//    여사제 세비나.
		c = CreateCharacter(Key,MODEL_NPC_SEVINA,PositionX,PositionY);
        o = &c->Object;
        o->Scale = 1.f;
        o->Kind  = KIND_NPC;
        break;

    case 236:   //  이벤트 NPC.
		OpenNpc(MODEL_PLAYER);//
		c = CreateCharacter(Key,MODEL_PLAYER,PositionX,PositionY);
        o = &c->Object;
        o->SubType  = MODEL_SKELETON2;
        o->Scale    = 1.0f;
        o->Kind     = KIND_NPC;
        c->Level    = 8;
        break;
	case 237:
		OpenNpc(MODEL_NPC_DEVILSQUARE);//악마의 광장-카론
		c = CreateCharacter(Key,MODEL_NPC_DEVILSQUARE,PositionX,PositionY);
		break;
	//정제는 따로 한다
	case 369://제련
		OpenNpc(MODEL_REFINERY_NPC);
		c = CreateCharacter(Key,MODEL_REFINERY_NPC,PositionX,PositionY);
		o = &c->Object;
		break;
	case 370://환원
		OpenNpc(MODEL_RECOVERY_NPC);
		c = CreateCharacter(Key,MODEL_RECOVERY_NPC,PositionX,PositionY);
		o = &c->Object;
		break;
	case 238:
    	OpenNpc(MODEL_MIX_NPC);//믹스
		c = CreateCharacter(Key,MODEL_MIX_NPC,PositionX,PositionY);
		o = &c->Object;
		o->BlendMesh = 1;
		break;
	case 239:
    	OpenNpc(MODEL_TOURNAMENT);//경기장지기
		c = CreateCharacter(Key,MODEL_TOURNAMENT,PositionX,PositionY);
		break;
	case 240:
		OpenNpc(MODEL_STORAGE);//창고지기
		c = CreateCharacter(Key,MODEL_STORAGE,PositionX,PositionY);
		break;
	case 241:
    	OpenNpc(MODEL_MASTER);
		c = CreateCharacter(Key,MODEL_MASTER,PositionX,PositionY);
		strcpy(c->ID,"마스터");
		break;
	case 256:  // 축합성 NPC
		OpenNpc(MODEL_NPC_SERBIS);
		c = CreateCharacter(Key, MODEL_NPC_SERBIS, PositionX, PositionY);
		strcpy(c->ID, "세르비스");
		break;
	case 257:  // 요정 도우미
		c = CreateCharacter(Key, MODEL_PLAYER, PositionX, PositionY);
		MakeElfHelper(c);
		strcpy(c->ID, "페이아");
		o = &c->Object;
		CreateJoint(BITMAP_FLARE, o->Position, o->Position, o->Angle, 42, o, 15.f);
		break;
	case 242:
    	OpenNpc(MODEL_ELF_WIZARD);
		c = CreateCharacter(Key,MODEL_ELF_WIZARD,PositionX,PositionY);
		strcpy(c->ID,"라라 요정");
		o = &c->Object;
		o->BlendMesh = 1;
    	o->Position[2] = RequestTerrainHeight(o->Position[0],o->Position[1]) + 140.f;
		break;
	case 243:
    	OpenNpc(MODEL_ELF_MERCHANT);
		c = CreateCharacter(Key,MODEL_ELF_MERCHANT,PositionX,PositionY);
		strcpy(c->ID,"장인");
		break;
	case 244:
    	OpenNpc(MODEL_SNOW_MERCHANT);
		c = CreateCharacter(Key,MODEL_SNOW_MERCHANT,PositionX,PositionY);
		strcpy(c->ID,"술집마담");
		break;
	case 245:
    	OpenNpc(MODEL_SNOW_WIZARD);
		c = CreateCharacter(Key,MODEL_SNOW_WIZARD,PositionX,PositionY);
		strcpy(c->ID,"마법사");
		break;
	case 246:
    	OpenNpc(MODEL_SNOW_SMITH);
		c = CreateCharacter(Key,MODEL_SNOW_SMITH,PositionX,PositionY);
		strcpy(c->ID,"무기상인");
		break;
	case 247:
#ifdef _PVP_ATTACK_GUARD
    	OpenNpc(MODEL_ANGEL);
		c = CreateCharacter(Key,MODEL_PLAYER,PositionX,PositionY);
		strcpy(c->ID,"경비병");
		SetCharacterScale(c);
		c->Weapon[0].Type = MODEL_BOW+9;
		c->Weapon[1].Type = MODEL_BOW+7;
		if (World == WD_10HEAVEN)
			c->Wing.Type = MODEL_WING+1;
#else	// _PVP_ATTACK_GUARD
		c = CreateCharacter(Key,MODEL_PLAYER,PositionX,PositionY);
		//c->Class = 2;
		strcpy(c->ID,"경비병");

		c->BodyPart[BODYPART_HELM  ].Type = MODEL_HELM  +9;
		c->BodyPart[BODYPART_ARMOR ].Type = MODEL_ARMOR +9;
		c->BodyPart[BODYPART_PANTS ].Type = MODEL_PANTS +9;
		c->BodyPart[BODYPART_GLOVES].Type = MODEL_GLOVES+9;
		c->BodyPart[BODYPART_BOOTS ].Type = MODEL_BOOTS +9;
		c->Weapon[0].Type = MODEL_BOW+11;
		c->Weapon[1].Type = MODEL_BOW+7;
		SetCharacterScale(c);
#endif	// _PVP_ATTACK_GUARD
		break;
	case 248:
   		OpenNpc(MODEL_MERCHANT_MAN);
		c = CreateCharacter(Key,MODEL_MERCHANT_MAN,PositionX,PositionY);
		strcpy(c->ID,"떠돌이 상인");
		c->BodyPart[BODYPART_HELM  ].Type = MODEL_MERCHANT_MAN_HEAD+1;
		c->BodyPart[BODYPART_ARMOR ].Type = MODEL_MERCHANT_MAN_UPPER+1;
		c->BodyPart[BODYPART_GLOVES].Type = MODEL_MERCHANT_MAN_GLOVES+1;
		c->BodyPart[BODYPART_BOOTS ].Type = MODEL_MERCHANT_MAN_BOOTS+1;
		break;
	case 249:
#ifdef _PVP_ATTACK_GUARD
    	OpenNpc(MODEL_ANGEL);
		c = CreateCharacter(Key,MODEL_PLAYER,PositionX,PositionY);
		strcpy(c->ID,"경비병");
		SetCharacterScale(c);
		c->Weapon[0].Type = MODEL_MACE+2;
		c->Weapon[1].Type = MODEL_SHIELD+7;
		if (World == WD_10HEAVEN)
			c->Wing.Type = MODEL_WING+1;
#else	// _PVP_ATTACK_GUARD
		c = CreateCharacter(Key,MODEL_PLAYER,PositionX,PositionY);
		strcpy(c->ID,"경비병");
		c->BodyPart[BODYPART_HELM  ].Type = MODEL_HELM  +9;
		c->BodyPart[BODYPART_ARMOR ].Type = MODEL_ARMOR +9;
		c->BodyPart[BODYPART_PANTS ].Type = MODEL_PANTS +9;
		c->BodyPart[BODYPART_GLOVES].Type = MODEL_GLOVES+9;
		c->BodyPart[BODYPART_BOOTS ].Type = MODEL_BOOTS +9;
		c->Weapon[0].Type = MODEL_SPEAR+7;
		SetCharacterScale(c);
#endif	// _PVP_ATTACK_GUARD
		break;
	case 250:
    	OpenNpc(MODEL_MERCHANT_MAN);
		c = CreateCharacter(Key,MODEL_MERCHANT_MAN,PositionX,PositionY);
		strcpy(c->ID,"떠돌이 상인");
		c->BodyPart[BODYPART_HELM  ].Type = MODEL_MERCHANT_MAN_HEAD;
		c->BodyPart[BODYPART_ARMOR ].Type = MODEL_MERCHANT_MAN_UPPER;
		c->BodyPart[BODYPART_GLOVES].Type = MODEL_MERCHANT_MAN_GLOVES;
		c->BodyPart[BODYPART_BOOTS ].Type = MODEL_MERCHANT_MAN_BOOTS;
		break;
	case 251:
   		OpenNpc(MODEL_SMITH);
		c = CreateCharacter(Key,MODEL_SMITH,PositionX,PositionY);
		strcpy(c->ID,"대장장이 한스");
		c->Object.Scale = 0.95f;
		break;
	case 253:
    	OpenNpc(MODEL_MERCHANT_GIRL);
		c = CreateCharacter(Key,MODEL_MERCHANT_GIRL,PositionX,PositionY);
		strcpy(c->ID,"물약파는 소녀");
		c->BodyPart[BODYPART_HELM  ].Type = MODEL_MERCHANT_GIRL_HEAD;
		c->BodyPart[BODYPART_ARMOR ].Type = MODEL_MERCHANT_GIRL_UPPER;
		c->BodyPart[BODYPART_PANTS ].Type = MODEL_MERCHANT_GIRL_LOWER;
		break;
	case 254:
    	OpenNpc(MODEL_SCIENTIST);
		c = CreateCharacter(Key,MODEL_SCIENTIST,PositionX,PositionY);
		strcpy(c->ID,"마법사 파시");
		break;
	case 255:
    	OpenNpc(MODEL_MERCHANT_FEMALE);
		c = CreateCharacter(Key,MODEL_MERCHANT_FEMALE,PositionX,PositionY);
		strcpy(c->ID,"술집마담 리아먼");
		c->BodyPart[BODYPART_HELM  ].Type = MODEL_MERCHANT_FEMALE_HEAD+1;
		c->BodyPart[BODYPART_ARMOR ].Type = MODEL_MERCHANT_FEMALE_UPPER+1;
		c->BodyPart[BODYPART_PANTS ].Type = MODEL_MERCHANT_FEMALE_LOWER+1;
		c->BodyPart[BODYPART_BOOTS ].Type = MODEL_MERCHANT_FEMALE_BOOTS+1;
		break;
	case 204: // 늑대의 신상
		OpenNpc(MODEL_CRYWOLF_STATUE);	// 늑대의 신상
		c = CreateCharacter(Key,MODEL_CRYWOLF_STATUE,PositionX,PositionY);
		strcpy(c->ID,"석상");
		c->Object.Live = false;
		break;
	case 205:
		OpenNpc(MODEL_CRYWOLF_ALTAR1);	// 제단 1
		c = CreateCharacter(Key,MODEL_CRYWOLF_ALTAR1,PositionX,PositionY);
		strcpy(c->ID,"제단1");
		c->Object.Position[2] -= 10.0f;
		c->Object.HiddenMesh = -2;
		c->Object.Visible = false;
		c->Object.EnableShadow = false;
		break;
	case 206:
		OpenNpc(MODEL_CRYWOLF_ALTAR2);	// 제단 2
		c = CreateCharacter(Key,MODEL_CRYWOLF_ALTAR2,PositionX,PositionY);
		strcpy(c->ID,"제단2");
		c->Object.HiddenMesh = -2;
		c->Object.Position[2] -= 10.0f;
		c->Object.Visible = false;
		c->Object.EnableShadow = false;
		break;
	case 207:
		OpenNpc(MODEL_CRYWOLF_ALTAR3);	// 제단 3
		c = CreateCharacter(Key,MODEL_CRYWOLF_ALTAR3,PositionX,PositionY);
		strcpy(c->ID,"제단3");
		c->Object.HiddenMesh = -2;
		c->Object.Position[2] -= 10.0f;
		c->Object.Visible = false;
		c->Object.EnableShadow = false;
		break;
	case 208:
		OpenNpc(MODEL_CRYWOLF_ALTAR4);	// 제단 4
		c = CreateCharacter(Key,MODEL_CRYWOLF_ALTAR4,PositionX,PositionY);
		strcpy(c->ID,"제단4");
		c->Object.HiddenMesh = -2;
		c->Object.Position[2] -= 10.0f;
		c->Object.Visible = false;
		c->Object.EnableShadow = false;
		break;
	case 209:
		OpenNpc(MODEL_CRYWOLF_ALTAR5);	// 제단 5
		c = CreateCharacter(Key,MODEL_CRYWOLF_ALTAR5,PositionX,PositionY);
		strcpy(c->ID,"제단5");
		c->Object.HiddenMesh = -2;
		c->Object.Position[2] -= 10.0f;
		c->Object.Visible = false;
		c->Object.EnableShadow = false;
		break;
	case 368:	// 제련의 탑 npc
		OpenNpc(MODEL_SMELTING_NPC);
		c = CreateCharacter(Key,MODEL_SMELTING_NPC,PositionX+1,PositionY-1);
		strcpy(c->ID,"제련의탑NPC");
		c->Object.Scale = 2.5f;
		c->Object.EnableShadow = false;
		c->Object.m_bRenderShadow = false;
		break;		
	case 379:	// 폭죽판매 NPC Wedding
		OpenNpc(MODEL_WEDDING_NPC);
		c = CreateCharacter(Key,MODEL_WEDDING_NPC,PositionX,PositionY);
		strcpy(c->ID,"WeddingNPC");
		c->Object.Scale = 1.1f;
		c->Object.EnableShadow = false;
		c->Object.m_bRenderShadow = false;
		break;
#ifdef PCROOM_EVENT
	case 258:
	case 371:	// 화이트 앤젤 이벤트 NPC
#ifdef PRUARIN_EVENT07_3COLORHARVEST
	case 414:	// 도우미 앨런 (추석이벤트)
#endif // PRUARIN_EVENT07_3COLORHARVEST
		c = CreateCharacter(Key,MODEL_PLAYER,PositionX,PositionY);
		strcpy(c->ID,"경비병");
		c->BodyPart[BODYPART_HELM  ].Type = MODEL_HELM  +9;
		c->BodyPart[BODYPART_ARMOR ].Type = MODEL_ARMOR +9;
		c->BodyPart[BODYPART_PANTS ].Type = MODEL_PANTS +9;
		c->BodyPart[BODYPART_GLOVES].Type = MODEL_GLOVES+9;
		c->BodyPart[BODYPART_BOOTS ].Type = MODEL_BOOTS +9;
		c->Weapon[0].Type = -1;
		SetCharacterScale(c);
		c->Object.m_bpcroom = true;
		break;
#endif	// PCROOM_EVENT
	case 259:
    	OpenNpc(MODEL_KALIMA_SHOP);
		c = CreateCharacter(Key,MODEL_KALIMA_SHOP,PositionX,PositionY);
		c->Object.Position[2] += 140.0f;
		strcpy(c->ID,"신관 레이라");
		break;
#ifdef CSK_CHAOS_CARD
		// 카오스카드 NPC 를 생성한다.
	case 375:
		{
			c = CreateCharacter(Key, MODEL_PLAYER, PositionX, PositionY);
			strcpy(c->ID,"카오스카드마스터");
			c->BodyPart[BODYPART_HELM  ].Type = MODEL_HELM  +30;
			c->BodyPart[BODYPART_ARMOR ].Type = MODEL_ARMOR +30;
			c->BodyPart[BODYPART_PANTS ].Type = MODEL_PANTS +30;
			c->BodyPart[BODYPART_GLOVES].Type = MODEL_GLOVES+30;
			c->BodyPart[BODYPART_BOOTS ].Type = MODEL_BOOTS +30;
			c->Wing.Type = MODEL_WING+1;
			int iLevel = 9;
			c->BodyPart[BODYPART_HELM  ].Level = iLevel;
			c->BodyPart[BODYPART_ARMOR ].Level = iLevel;
			c->BodyPart[BODYPART_PANTS ].Level = iLevel;
			c->BodyPart[BODYPART_GLOVES].Level = iLevel;
			c->BodyPart[BODYPART_BOOTS ].Level = iLevel;
			c->Weapon[0].Type = -1;
			SetCharacterScale(c);
			c->Object.SubType = Type;	
		}
		break;
#endif // CSK_CHAOS_CARD
	case 376:
		{
			OpenNpc(MODEL_BC_NPC1);
			c = CreateCharacter(Key, MODEL_BC_NPC1, PositionX, PositionY);
			strcpy(c->ID,"공성 NPC");
			c->Object.Scale = 1.0f;
			c->Object.Angle[2] = 0.f;
			CreateObject(MODEL_BC_BOX, c->Object.Position, c->Object.Angle);
		}
		break;
	case 377:
		{
			OpenNpc(MODEL_BC_NPC2);
			c = CreateCharacter(Key, MODEL_BC_NPC2, PositionX, PositionY);
			strcpy(c->ID,"공성 NPC");
			c->Object.Scale = 1.0f;
			c->Object.Angle[2] = 90.f;
			CreateObject(MODEL_BC_BOX, c->Object.Position, c->Object.Angle);
		}
		break;
#ifdef ADD_NPC_DEVIN 
	case 406:	// 사제데빈
		OpenNpc(MODEL_NPC_DEVIN);
		c = CreateCharacter(Key,MODEL_NPC_DEVIN,PositionX,PositionY);
		strcpy(c->ID,"사제데빈");
		break;
#endif	// ADD_NPC_DEVIN
	case 407:	// 웨어울프쿼렐
		OpenNpc(MODEL_NPC_QUARREL);
		c = CreateCharacter(Key,MODEL_NPC_QUARREL,PositionX,PositionY);
		strcpy(c->ID,"웨어울프쿼렐");
		c->Object.Scale = 1.9f;
		break;
	case 408:	// 성문
		OpenNpc(MODEL_NPC_CASTEL_GATE);
		c = CreateCharacter(Key,MODEL_NPC_CASTEL_GATE,PositionX,PositionY, 90.f);
		strcpy(c->ID,"성문");
		o = &c->Object;
		o->Position[2] = RequestTerrainHeight(o->Position[0],o->Position[1]) + 240.f;
		c->Object.Scale = 1.2f;
		c->Object.m_fEdgeScale = 1.1f;	// 마우스 커서로 선택 시 초록 외곽선.
		c->Object.EnableShadow = false;
		c->Object.m_bRenderShadow = false;
		break;
#ifdef PRUARIN_EVENT07_3COLORHARVEST
	case 413:		// 달토끼
		{
			OpenMonsterModel(127);
			c = CreateCharacter(Key, MODEL_MONSTER01+127, PositionX, PositionY);
			strcpy(c->ID,"달토끼");
			c->Object.Scale = 0.8f;
			c->Weapon[0].Type = -1;
			c->Weapon[1].Type = -1;
			c->Object.SubType = rand()%3;		// 죽을때 떨어지는 이펙트 선택
			c->Object.m_iAnimation = 0;			
			
			BoneManager::RegisterBone(c, "Rabbit_1", 3);		// Bip01 Spine 	
			BoneManager::RegisterBone(c, "Rabbit_2", 16);		// Bip01 Head
			BoneManager::RegisterBone(c, "Rabbit_3", 15);		// Bip01 Neck1 
			BoneManager::RegisterBone(c, "Rabbit_4", 2);		// Bip01 Pelvis
		}
		
		break;
#endif // PRUARIN_EVENT07_3COLORHARVEST
#ifdef CSK_EVENT_CHERRYBLOSSOM
	case 450:
		{
			OpenNpc ( MODEL_NPC_CHERRYBLOSSOM );
			c = CreateCharacter(Key,MODEL_NPC_CHERRYBLOSSOM,PositionX,PositionY);
			c->Object.Scale = 0.65f;
			c->Object.m_fEdgeScale = 1.08f;
			o = &c->Object;
			o->Position[2] = RequestTerrainHeight(o->Position[0],o->Position[1]) + 170.f;
			strcpy( c->ID, "벚꽃의정령" );
		}
		break;
#endif //CSK_EVENT_CHERRYBLOSSOM
#ifdef PSW_EVENT_CHERRYBLOSSOMTREE
	case 451:
		{
			OpenNpc ( MODEL_NPC_CHERRYBLOSSOMTREE );
			c = CreateCharacter(Key,MODEL_NPC_CHERRYBLOSSOMTREE,PositionX,PositionY);
			c->Object.Scale = 1.0f;
			c->Object.m_fEdgeScale = 0.0f;
			c->Object.m_bRenderShadow = false;
			strcpy( c->ID, "벚꽃나무" );
		}
		break;
#endif //PSW_EVENT_CHERRYBLOSSOMTREE

#ifdef LEM_ADD_LUCKYITEM
	case 579:
		OpenNpc(MODEL_LUCKYITEM_NPC);
		c = CreateCharacter(Key,MODEL_LUCKYITEM_NPC,PositionX,PositionY);
		strcpy(c->ID,"다비드");
		c->Object.Scale = 0.95f;
		c->Object.m_fEdgeScale				= 1.2f;	// 마우스 커서로 선택 시 초록 외곽선.
		Models[MODEL_LUCKYITEM_NPC].Actions[0].PlaySpeed	= 0.45f;
		Models[MODEL_LUCKYITEM_NPC].Actions[1].PlaySpeed	= 0.5f;
		

	//	Models[MODEL_LUCKYITEM_NPC].Actions[0].PlaySpeed = 50.0f;
	//	Models[MODEL_LUCKYITEM_NPC].Actions[1].PlaySpeed = 50.0f;
		break;
#endif // LEM_ADD_LUCKYITEM
#ifdef ADD_SOCKET_MIX
	case 452:	// 시드 마스터 NPC
   		OpenNpc(MODEL_SEED_MASTER);
		c = CreateCharacter(Key,MODEL_SEED_MASTER,PositionX,PositionY);
		strcpy(c->ID,"시드마스터");
		c->Object.Scale = 1.1f;
		c->Object.m_fEdgeScale = 1.2f;	// 마우스 커서로 선택 시 초록 외곽선.
		break;
	case 453:	// 시드 연구가 NPC
   		OpenNpc(MODEL_SEED_INVESTIGATOR);
		c = CreateCharacter(Key,MODEL_SEED_INVESTIGATOR,PositionX,PositionY);
		strcpy(c->ID,"시드연구가");
		c->Object.Scale = 0.9f;
		c->Object.m_fEdgeScale = 1.15f;	// 마우스 커서로 선택 시 초록 외곽선.
		//Models[MODEL_SEED_INVESTIGATOR].Actions[0].PlaySpeed = 0.2f;
		//Models[MODEL_SEED_INVESTIGATOR].Actions[1].PlaySpeed = 0.1f;
		break;
#endif	// ADD_SOCKET_MIX
#ifdef PSW_ADD_RESET_CHARACTER_POINT
	case 464:
		{
#ifdef _PVP_ATTACK_GUARD
			OpenNpc(MODEL_ANGEL);
			c = CreateCharacter(Key,MODEL_PLAYER,PositionX,PositionY);
			strcpy(c->ID,"초기화 도우미");
			SetCharacterScale(c);
			c->Object.m_fEdgeScale = 1.15f;
			c->Weapon[0].Type = MODEL_BOW+9;
			c->Weapon[1].Type = MODEL_BOW+7;
			if (World == WD_10HEAVEN)
				c->Wing.Type = MODEL_WING+1;
#else	// _PVP_ATTACK_GUARD
			c = CreateCharacter(Key,MODEL_PLAYER,PositionX,PositionY);
			//c->Class = 2;
			strcpy(c->ID,"초기화 도우미");
			
			c->BodyPart[BODYPART_HELM  ].Type = MODEL_HELM  +9;
			c->BodyPart[BODYPART_ARMOR ].Type = MODEL_ARMOR +9;
			c->BodyPart[BODYPART_PANTS ].Type = MODEL_PANTS +9;
			c->BodyPart[BODYPART_GLOVES].Type = MODEL_GLOVES+9;
			c->BodyPart[BODYPART_BOOTS ].Type = MODEL_BOOTS +9;

			c->Object.m_fEdgeScale = 1.15f;
			c->Weapon[0].Type = MODEL_BOW+11;
			c->Weapon[1].Type = MODEL_BOW+7;
			SetCharacterScale(c);
#endif	// _PVP_ATTACK_GUARD
		}	
		break;
#endif //PSW_ADD_RESET_CHARACTER_POINT
#ifdef LDK_ADD_SNOWMAN_CHANGERING
		// 크리스마스 변신반지 모델 데이타 생성
	case 477:
		OpenNpc(MODEL_XMAS2008_SNOWMAN);
		c = CreateCharacter(Key,MODEL_XMAS2008_SNOWMAN,PositionX,PositionY);
		::strcpy(c->ID, "눈사람변신");
		c->Object.LifeTime = 100;
		c->Object.Scale = 1.3f;
		break;
#endif //LDK_ADD_SNOWMAN_CHANGERING
#ifdef PJH_ADD_PANDA_CHANGERING
	case 503:
		c = CreateCharacter(Key, MODEL_PLAYER, PositionX, PositionY);
		::strcpy(c->ID, "팬더변신");
		c->Object.SubType = MODEL_PANDA;
		break;
#endif //PJH_ADD_PANDA_CHANGERING
#ifdef YDG_ADD_SKELETON_CHANGE_RING
	case 548:
		c = CreateCharacter(Key, MODEL_PLAYER, PositionX, PositionY);
		::strcpy(c->ID, "스켈레톤변신");
		c->Object.SubType = MODEL_SKELETON_CHANGED;
		break;
#endif	// YDG_ADD_SKELETON_CHANGE_RING
#ifdef PBG_ADD_LITTLESANTA_NPC
	//리틀산타 1~8
	case 468:
	case 469:
	case 470:
	case 471:
	case 472:
	case 473:
	case 474:
	case 475:
		{
			int _Model_NpcIndex = MODEL_LITTLESANTA+(Type-468);

			OpenNpc(_Model_NpcIndex);
			c = CreateCharacter(Key, _Model_NpcIndex, PositionX, PositionY);
	
			//스케일값.
			c->Object.Scale = 0.43f;

			//애니메이션 스피드.
			for(int i=0; i<5; i++)
			{
				//기본속도의 2배속으로.
				if(i < 2 || i ==4)
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
		 	strcpy(c->ID, "little santa");
		}
		break;
#endif //PBG_ADD_LITTLESANTA_NPC
#ifdef KJH_PBG_ADD_SEVEN_EVENT_2008
	case 478:
		//델가도
		OpenNpc(MODEL_NPC_SERBIS);
		c = CreateCharacter(Key, MODEL_NPC_SERBIS, PositionX, PositionY);
		strcpy(c->ID, "델가도");
		break;
#endif //KJH_PBG_ADD_SEVEN_EVENT_2008
#ifdef YDG_ADD_NEW_DUEL_NPC
	case 479:
		// 결투장 문지기 NPC 타이투스
		OpenNpc(MODEL_DUEL_NPC_TITUS);
		c = CreateCharacter(Key, MODEL_DUEL_NPC_TITUS, PositionX, PositionY);
		strcpy(c->ID, "타이투스");
		c->Object.Scale = 1.1f;
		c->Object.m_fEdgeScale = 1.2f;	// 마우스 커서로 선택 시 초록 외곽선.
		break;
#endif	// YDG_ADD_NEW_DUEL_NPC
#ifdef LDK_ADD_GAMBLE_NPC_MOSS
	case 492: //임시 인덱스
		{
			// 겜블 상인 모스
			OpenNpc(MODEL_GAMBLE_NPC_MOSS);
			c = CreateCharacter(Key, MODEL_GAMBLE_NPC_MOSS, PositionX, PositionY);
			strcpy(c->ID, "모스");
			c->Object.LifeTime = 100;
 			c->Object.Scale = 0.8f;
			c->Object.m_fEdgeScale = 1.1f;	// 마우스 커서로 선택 시 초록 외곽선.

			for(int i=0; i<6; i++)
			{
	 			Models[MODEL_GAMBLE_NPC_MOSS].Actions[i].PlaySpeed = 0.33f;
			}
		}
		break;
#endif //LDK_ADD_GAMBLE_NPC_MOSS
#ifdef CSK_ADD_GOLDCORPS_EVENT
#ifdef KJH_FIX_GOLD_RABBIT_INDEX
	case 502:	// 황금토끼
#else // KJH_FIX_GOLD_RABBIT_INDEX
	case 492:	// 황금토끼
#endif // KJH_FIX_GOLD_RABBIT_INDEX
		OpenMonsterModel(128);
		c = CreateCharacter(Key, MODEL_MONSTER01+128, PositionX, PositionY);
		strcpy(c->ID,"황금토끼");
		c->Object.Scale = 1.0f * 0.95f;
		c->Weapon[0].Type = -1;
		c->Weapon[1].Type = -1;
		break;
	case 493:	// 황금다크나이트
		OpenMonsterModel(3);
		c = CreateCharacter(Key,MODEL_MONSTER01+3,PositionX,PositionY);
		strcpy(c->ID,"황금다크나이트");
		c->Object.Scale = 0.8f;
		c->Level = 1;
		c->Weapon[0].Type = MODEL_SWORD+13;
		break;
		break;
	case 494:	// 황금데빌
		OpenMonsterModel(26);
		c = CreateCharacter(Key,MODEL_MONSTER01+26,PositionX,PositionY);
		strcpy(c->ID,"황금데빌");
		c->Object.Scale = 1.1f;
		break;
	case 495:	// 황금돌괴물
		OpenMonsterModel(101);
		c = CreateCharacter(Key,MODEL_MONSTER01+101,PositionX,PositionY);
		c->Object.Scale = 1.35f;
		c->Weapon[0].Type = -1;
		c->Weapon[1].Type = -1;
		BoneManager::RegisterBone(c, "Monster101_L_Arm", 12);
		BoneManager::RegisterBone(c, "Monster101_R_Arm", 20);
		BoneManager::RegisterBone(c, "Monster101_Head", 6);	
		break;
	case 496:	// 황금크러스트
		OpenMonsterModel(52);
		c = CreateCharacter(Key,MODEL_MONSTER01+52,PositionX,PositionY);
		c->Object.Scale = 1.1f;
		c->Weapon[0].Type = MODEL_SWORD+18;
		c->Weapon[0].Level = 5;
		c->Weapon[1].Type = MODEL_SHIELD+14;
		c->Weapon[1].Level = 0;
        c->Object.BlendMesh = 1;
        c->Object.BlendMeshLight = 1.f;
		break;
	case 497:	// 황금사티로스
		OpenMonsterModel(109);
		c = CreateCharacter(Key, MODEL_MONSTER01+109, PositionX, PositionY);
		c->Object.Scale = 1.3f;
		c->Weapon[0].Type = -1;
		c->Weapon[1].Type = -1;
		strcpy( c->ID, "황금사티로스" );
		break;
	case 498:	// 황금트윈테일
		OpenMonsterModel(115);
		c = CreateCharacter(Key,MODEL_MONSTER01+115,PositionX,PositionY);
		c->Object.Scale = 1.3f;
		c->Object.Angle[0] = 0.0f;
		c->Object.Gravity = 0.0f;
		c->Object.Distance = (float)(rand()%20)/10.0f;
		c->Weapon[0].Type = -1;
		c->Weapon[1].Type = -1;
		// 머리 끝 본
		BoneManager::RegisterBone(c, "Twintail_Hair24", 16);
		BoneManager::RegisterBone(c, "Twintail_Hair32", 24);
		break;
	case 499:	// 황금아이언나이트
		OpenMonsterModel(149);
		c = CreateCharacter(Key, MODEL_MONSTER01+149, PositionX, PositionY);
		strcpy(c->ID, "황금아이언나이트");
		c->Object.Scale = 1.5f;
		c->Weapon[0].Type = -1;
		c->Weapon[1].Type = -1;
		break;
	case 500:	// 황금네이핀
		OpenMonsterModel(142);
		c = CreateCharacter(Key,MODEL_MONSTER01+142,PositionX,PositionY);
		strcpy(c->ID,"황금네이핀");
		c->Object.Scale = 0.95f;
		c->Weapon[0].Type = -1;
		c->Weapon[1].Type = -1;
		break;
	case 501:	// 황금그레이트드레곤
		OpenMonsterModel(31);
		c = CreateCharacter(Key,MODEL_MONSTER01+31,PositionX,PositionY);
		strcpy(c->ID,"황금그레이트드레곤");
		c->Object.Scale = 0.88f;
		c->Weapon[0].Type = -1;
		c->Weapon[1].Type = -1;
		break;
#endif // CSK_ADD_GOLDCORPS_EVENT
#ifdef YDG_ADD_DOPPELGANGER_NPC
	case 540:	// 도플갱어 NPC 루가드
		OpenNpc(MODEL_DOPPELGANGER_NPC_LUGARD);
		c = CreateCharacter(Key, MODEL_DOPPELGANGER_NPC_LUGARD, PositionX, PositionY);
		strcpy(c->ID, "루가드");
		c->Object.Scale = 1.1f;
		c->Object.m_fEdgeScale = 1.2f;	// 마우스 커서로 선택 시 초록 외곽선.
		break;
	case 541:	// 중간보상상자
		OpenNpc(MODEL_DOPPELGANGER_NPC_BOX);
		c = CreateCharacter(Key, MODEL_DOPPELGANGER_NPC_BOX, PositionX, PositionY);
		strcpy(c->ID, "중간보상상자");
		c->Object.Scale = 2.3f;
		c->Object.m_fEdgeScale = 1.1f;	// 마우스 커서로 선택 시 초록 외곽선.
		break;
	case 542:	// 최종보상상자
		OpenNpc(MODEL_DOPPELGANGER_NPC_GOLDENBOX);
		c = CreateCharacter(Key, MODEL_DOPPELGANGER_NPC_GOLDENBOX, PositionX, PositionY);
		strcpy(c->ID, "최종보상상자");
		c->Object.Scale = 3.3f;
		c->Object.m_fEdgeScale = 1.1f;	// 마우스 커서로 선택 시 초록 외곽선.
		break;
#endif	// YDG_ADD_DOPPELGANGER_NPC
#ifdef ASG_ADD_GENS_NPC
	case 543:	// 겐스 듀프리언 집사
		OpenNpc(MODAL_GENS_NPC_DUPRIAN);
		c = CreateCharacter(Key, MODAL_GENS_NPC_DUPRIAN, PositionX, PositionY);
		strcpy(c->ID, "겐스 듀프리언 집사");
		c->Object.Scale = 1.0f;
		break;
	case 544:	// 겐스 바네르트 집사
		OpenNpc(MODAL_GENS_NPC_BARNERT);
		c = CreateCharacter(Key, MODAL_GENS_NPC_BARNERT, PositionX, PositionY);
		strcpy(c->ID, "겐스 바네르트 집사");
		c->Object.Scale = 1.0f;
		break;
#endif	// ASG_ADD_GENS_NPC
#ifdef LDS_ADD_NPC_UNITEDMARKETPLACE
	case 545:	// 크리스틴
		OpenNpc(MODEL_UNITEDMARKETPLACE_CHRISTIN);
		c = CreateCharacter(Key, MODEL_UNITEDMARKETPLACE_CHRISTIN, PositionX, PositionY);
		strcpy(c->ID, "크리스틴");
		c->Object.Scale = 1.1f;
		c->Object.m_fEdgeScale = 1.2f;	// 마우스 커서로 선택 시 초록 외곽선.
		break;
	case 546:	// 라울
		OpenNpc(MODEL_UNITEDMARKETPLACE_RAUL);
		c = CreateCharacter(Key, MODEL_UNITEDMARKETPLACE_RAUL, PositionX, PositionY);
		strcpy(c->ID, "라울");
		c->Object.Scale = 1.0f;
		c->Object.m_fEdgeScale = 1.15f;	// 마우스 커서로 선택 시 초록 외곽선.
		break;
	case 547:	// 줄리아
		OpenNpc(MODEL_UNITEDMARKETPLACE_JULIA);
		c = CreateCharacter(Key, MODEL_UNITEDMARKETPLACE_JULIA, PositionX, PositionY);
		strcpy(c->ID, "줄리아");
		c->Object.Scale = 1.0f;
		c->Object.m_fEdgeScale = 1.1f;	// 마우스 커서로 선택 시 초록 외곽선.
		break;
#endif // LDS_ADD_NPC_UNITEDMARKETPLACE
#ifdef ASG_ADD_TIME_LIMIT_QUEST_NPC
	case 566:	// 길드관리인 테르시아
		OpenNpc(MODEL_TIME_LIMIT_QUEST_NPC_TERSIA);
		c = CreateCharacter(Key, MODEL_TIME_LIMIT_QUEST_NPC_TERSIA, PositionX, PositionY);
		strcpy(c->ID, "길드관리인 테르시아");
		c->Object.Scale = 0.93f;
		break;
	case 567:
		OpenNpc(MODEL_TIME_LIMIT_QUEST_NPC_BENA);
		c = CreateCharacter(Key, MODEL_TIME_LIMIT_QUEST_NPC_BENA, PositionX, PositionY);
		strcpy(c->ID, "신녀 베이나");
		c->Object.Position[2] += 145.0f;
		break;
	case 568:
		{
			OpenNpc(MODEL_TIME_LIMIT_QUEST_NPC_ZAIRO);
			c = CreateCharacter(Key, MODEL_TIME_LIMIT_QUEST_NPC_ZAIRO, PositionX, PositionY);
			strcpy(c->ID, "떠돌이상인 자이로");
			c->Object.LifeTime = 100;
			c->Object.Scale = 0.8f;
			c->Object.m_fEdgeScale = 1.1f;	// 마우스 커서로 선택 시 초록 외곽선.
			int i;
			for(i = 0; i < 6; ++i)
				Models[MODEL_TIME_LIMIT_QUEST_NPC_ZAIRO].Actions[i].PlaySpeed = 0.33f;
		}
		break;
#endif	// ASG_ADD_TIME_LIMIT_QUEST_NPC
#ifdef ASG_ADD_KARUTAN_NPC
	case 577:	// 잡화상인 레이나
		OpenNpc(MODEL_KARUTAN_NPC_REINA);
		c = CreateCharacter(Key, MODEL_KARUTAN_NPC_REINA, PositionX, PositionY);
		strcpy(c->ID, "잡화상인 레이나");
		c->Object.Scale = 1.1f;
		c->Object.m_fEdgeScale = 1.2f;	// 마우스 커서로 선택 시 초록 외곽선.
		break;
	case 578:	// 무기상인 볼로
		OpenNpc(MODEL_KARUTAN_NPC_VOLVO);
		c = CreateCharacter(Key, MODEL_KARUTAN_NPC_VOLVO, PositionX, PositionY);
		strcpy(c->ID, "무기상인 볼로");
		c->Object.Scale = 0.9f;
		break;
#endif	// ASG_ADD_KARUTAN_NPC
	}

	Setting_Monster(c,Type, PositionX, PositionY);

	return c;
}
///////////////////////////////////////////////////////////////////////////////
// 서버 선택 씬과, 캐릭터 선택 씬에서 나오는 캐릭터 생성
///////////////////////////////////////////////////////////////////////////////

CHARACTER *CreateHero(int Index,int Class,int Skin,float x,float y,float Rotate)
{
	CHARACTER *c = &CharactersClient[Index];
	OBJECT    *o = &c->Object;
	CreateCharacterPointer(c,MODEL_PLAYER,0,0,Rotate);
	Vector(0.3f,0.3f,0.3f,o->Light);
	c->Key = Index;
	o->Position[0] = x;
	o->Position[1] = y;
	c->Class = Class;
	c->Skin = Skin;

	g_CharacterClearBuff(o);

#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
	int Level = 0;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING

	if(SceneFlag == LOG_IN_SCENE)
	{
		c->BodyPart[BODYPART_HELM  ].Type = MODEL_HELM+28;
		c->BodyPart[BODYPART_HELM  ].Level = 7;
		c->BodyPart[BODYPART_ARMOR ].Type = MODEL_ARMOR+28;
		c->BodyPart[BODYPART_ARMOR ].Level = 7;
		c->BodyPart[BODYPART_PANTS ].Type = MODEL_PANTS+28;
		c->BodyPart[BODYPART_PANTS ].Level = 7;
		c->BodyPart[BODYPART_GLOVES].Type = MODEL_GLOVES+28;
		c->BodyPart[BODYPART_GLOVES].Level = 7;
		c->BodyPart[BODYPART_BOOTS ].Type = MODEL_BOOTS+28;
		c->BodyPart[BODYPART_BOOTS ].Level = 7;
		c->Weapon[0].Type = MODEL_MACE+14;
		c->Weapon[1].Type = MODEL_HELPER+5;
		c->Weapon[0].Level = 13;
		c->Wing.Type = MODEL_HELPER+30;
		c->Helper.Type = MODEL_HELPER+4;
		//c->Helper.Level = 13;
	}
	else
	{
		c->BodyPart[BODYPART_HELM  ].Type = MODEL_BODY_HELM  +GetBaseClass( Class);
		c->BodyPart[BODYPART_ARMOR ].Type = MODEL_BODY_ARMOR +GetBaseClass( Class);
		c->BodyPart[BODYPART_PANTS ].Type = MODEL_BODY_PANTS +GetBaseClass( Class);
		c->BodyPart[BODYPART_GLOVES].Type = MODEL_BODY_GLOVES+GetBaseClass( Class);
		c->BodyPart[BODYPART_BOOTS ].Type = MODEL_BODY_BOOTS +GetBaseClass( Class);
		c->Weapon[0].Type = -1;
		c->Weapon[1].Type = -1;
		c->Wing.Type      = -1;
		c->Helper.Type    = -1;
	}

	UnRegisterBuff(eBuff_GMEffect, o);
	
	c->CtlCode = 0;
	SetCharacterScale(c);
    SetPlayerStop(c);
	return c;
}


//////////////////////////////////////////////////////////////////////////
//  헬게이트를 생성한다.
//////////////////////////////////////////////////////////////////////////
CHARACTER*  CreateHellGate ( BYTE* ID, int Key, int Index, int x, int y, int CreateFlag )
{
    CHARACTER* portal = CreateMonster ( Index, x, y, Key );
    portal->Level = Index-152+1;
    char Text[100];
    wsprintf ( Text, portal->ID, ID );

	if(portal->Level == 7)
		portal->Object.SubType = 1;

#if SELECTED_LANGUAGE != LANGUAGE_KOREAN
	// 한국이 아니면 32바이트 ID 사용
    memcpy ( portal->ID, Text, sizeof( char )*32 );
#else // SELECTED_LANGUAGE != LANGUAGE_KOREAN
	// 한국이면 24바이트 ID 사용
    memcpy ( portal->ID, Text, sizeof( char )*24 );
#endif // SELECTED_LANGUAGE != LANGUAGE_KOREAN

    if ( CreateFlag )
    {
        CreateJoint ( BITMAP_JOINT_THUNDER+1, portal->Object.Position, portal->Object.Position, portal->Object.Angle, 1, NULL, 60.f+rand()%10 );
        CreateJoint ( BITMAP_JOINT_THUNDER+1, portal->Object.Position, portal->Object.Position, portal->Object.Angle, 1, NULL, 50.f+rand()%10 );
        CreateJoint ( BITMAP_JOINT_THUNDER+1, portal->Object.Position, portal->Object.Position, portal->Object.Angle, 1, NULL, 50.f+rand()%10 );
        CreateJoint ( BITMAP_JOINT_THUNDER+1, portal->Object.Position, portal->Object.Position, portal->Object.Angle, 1, NULL, 60.f+rand()%10 );
    }
    return portal;
}


///////////////////////////////////////////////////////////////////////////////
// 몬스터 위치 스크립트 저장(에디터상에서만 사용함)
///////////////////////////////////////////////////////////////////////////////

void SaveMonsters(char *FileName)
{
	FILE *fp = fopen(FileName,"wt");
    fprintf(fp,"2\n");
	for(int i=0;i<MAX_CHARACTERS_CLIENT;i++)
	{
		CHARACTER *c = &CharactersClient[i];
		OBJECT *o = &c->Object;
		if(o->Live && o->Kind==KIND_EDIT)
		{
			fprintf(fp,"%4d %4d 30 %4d %4d -1\n",c->MonsterIndex,World,(BYTE)(o->Position[0]/TERRAIN_SCALE),(BYTE)(o->Position[1]/TERRAIN_SCALE));
		}
	}
    fprintf(fp,"end\n");
	fclose(fp);
}


// ChangeServerClassTypeToClientClassType() 함수 설명 - rozy 추가.

// 매개 변수 byServerClassType의 구조.(서버에서 사용)
// 1차 직업 : 맨 앞 3개 비트.
// 2차 직업 : 맨 앞에서 4번째 비트.
// 3차 직업 : 맨 앞에서 5번째 비트.
// 맨 뒤의 3개 비트는 안 씀.

// 리턴 값 byClass 구조.(클라이언트에서 사용)
// 1차 직업 : 맨 뒤 3개 비트.
// 2차 직업 : 맨 뒤에서 4번째 비트.
// 3차 직업 : 맨 뒤에서 5번째 비트.
// 맨 앞의 3개 비트는 안 씀.
BYTE ChangeServerClassTypeToClientClassType(const BYTE byServerClassType)
{
	BYTE byClass = 0;

	byClass = (((byServerClassType >> 4) & 0x01) << 3) | (byServerClassType >> 5) | (((byServerClassType >> 3 ) & 0x01) << 4) ;
	
	return byClass;
}

BYTE GetCharacterClass(const BYTE byClass)
{
	BYTE byCharacterClass = 0;
	
	// 1차 직업을 구한다.
	// 뒤의 3비트와 &연산을 해서 0~5의 값을 얻는다.
	BYTE byFirstClass = byClass & 0x7;
	// 2차 직업을 구한다.
	// 4번째 비트에 1이면 2번째 전직이고 0이면 아니다.
	BYTE bySecondClass = (byClass >> 3) & 0x01;
	// 3차 직업을 구한다.
	// 5번째 비트에 1이면 3번째 전직이고 0이면 아니다.
	BYTE byThirdClass = (byClass >> 4) & 0x01;
	switch(byFirstClass)
	{
	case 0:
		{
			if(byThirdClass)
			{
				byCharacterClass = CLASS_GRANDMASTER;	// 그랜드마스터
			}
			else if(bySecondClass)
			{
				byCharacterClass = CLASS_SOULMASTER;	// 소울마스터
			}
			else
			{
				byCharacterClass = CLASS_WIZARD;		// 흑마법사
			}
		}
		
		break;
	case 1:
		{
			if(byThirdClass)
			{
				byCharacterClass = CLASS_BLADEMASTER;	// 블레이드마스터
			}
			else if(bySecondClass)
			{
				byCharacterClass = CLASS_BLADEKNIGHT;	// 블레이드나이트
			}
			else
			{
				byCharacterClass = CLASS_KNIGHT;		// 흑기사
			}
		}
		break;
	case 2:	
		{
			if(byThirdClass)
			{
				byCharacterClass = CLASS_HIGHELF;		// 하이엘프
			}
			else if(bySecondClass)
			{
				byCharacterClass = CLASS_MUSEELF;		// 뮤즈엘프
			}
			else
			{
				byCharacterClass = CLASS_ELF;			// 엘프
			}
		}
		break;
	case 3:	
		{
			if(byThirdClass)
			{
				byCharacterClass = CLASS_DUELMASTER;	// 듀얼마스터
			}
			else
			{
				byCharacterClass = CLASS_DARK;			// 마검사
			}
		}
		break;
	case 4:	
		{
			if(byThirdClass)
			{
				byCharacterClass = CLASS_LORDEMPEROR;	// 로드엠퍼러
			}
			else
			{
				byCharacterClass = CLASS_DARK_LORD;		// 다크로드
			}
		}
		break;
	case 5:	
		{
			if (byThirdClass)
				byCharacterClass = CLASS_DIMENSIONMASTER;// 디멘션마스터
			else if (bySecondClass)
				byCharacterClass = CLASS_BLOODYSUMMONER;// 블러디서머너
			else
				byCharacterClass = CLASS_SUMMONER;		// 소환술사
		}
		break;
#ifdef PBG_ADD_NEWCHAR_MONK
	case 6:
		{
			if(byThirdClass)
			{
				byCharacterClass = CLASS_TEMPLENIGHT;	// 템플나이트
			}
			else
			{
				byCharacterClass = CLASS_RAGEFIGHTER;	// 레이지파이터
			}
		}
#endif //PBG_ADD_NEWCHAR_MONK
	}

	return byCharacterClass;
}

BYTE GetSkinModelIndex(const BYTE byClass)
{
	BYTE bySkinIndex = 0;

	// 1차 직업을 구한다.
	// 뒤의 3비트와 &연산을 해서 0~5의 값을 얻는다.
	BYTE byFirstClass = byClass & 0x7;
	// 2차 직업을 구한다.
	// 4번째 비트에 1이면 2번째 전직이고 0이면 아니다.
	BYTE bySecondClass = (byClass >> 3) & 0x01;
	// 3차 직업을 구한다.
	// 5번째 비트에 1이면 3번째 전직이고 0이면 아니다.
	BYTE byThirdClass = (byClass >> 4) & 0x01;

	// 흑마법사, 흑기사, 엘프, 소환술사
	if (byFirstClass == CLASS_WIZARD
		|| byFirstClass == CLASS_KNIGHT
		|| byFirstClass == CLASS_ELF
		|| byFirstClass == CLASS_SUMMONER
		)
	{
		bySkinIndex = byFirstClass + (bySecondClass + byThirdClass) * MAX_CLASS;
	}
	else	// 마검사, 다크로드는 2차 직업이 없기 때문
	{
		bySkinIndex = byFirstClass + (byThirdClass * 2) * MAX_CLASS;
	}

	return bySkinIndex;
}

bool IsSecondClass(const BYTE byClass)
{
	// 2차 직업을 구한다.
	// 4번째 비트에 1이면 2번째 전직이고 0이면 아니다.
	BYTE bySecondClass = (byClass >> 3) & 0x01;

	return bySecondClass;
}

bool IsThirdClass(const BYTE byClass)
{
	// 3차 직업을 구한다.
	// 5번째 비트에 1이면 3번째 전직이고 0이면 아니다.
	BYTE byThirdClass = (byClass >> 4) & 0x01;

	return byThirdClass;
}

BYTE GetStepClass(const BYTE byClass)
{
	BYTE byStep = 0;

	if(IsThirdClass(byClass))
	{
		byStep = 3;
	}
	else if(IsSecondClass(byClass) == true && IsThirdClass(byClass) == false)
	{
		byStep = 2;
	}
	else
	{
		byStep = 1;
	}

	return byStep;
}

const unicode::t_char* GetCharacterClassText(const BYTE byClass)
{
	BYTE byCharacterClass = GetCharacterClass(byClass);

	if(byCharacterClass == CLASS_WIZARD)
	{
		// 20 "흑마법사"
		return GlobalText[20];
	}
	else if(byCharacterClass == CLASS_SOULMASTER)
	{
		// 25 "소울마스터"
		return GlobalText[25];
	}
	else if(byCharacterClass == CLASS_GRANDMASTER)
	{
		// 1669 "그랜드마스터"
		return GlobalText[1669];
	}
	else if(byCharacterClass == CLASS_KNIGHT)
	{
		// 21 "흑기사"
		return GlobalText[21];
	}
	else if(byCharacterClass == CLASS_BLADEKNIGHT)
	{
		// 26 "블레이드나이트"
		return GlobalText[26];
	}
	else if(byCharacterClass == CLASS_BLADEMASTER)
	{
		// 1668 "블레이드마스터"
		return GlobalText[1668];
	}
	else if(byCharacterClass == CLASS_ELF)
	{
		// 22 "요정"
		return GlobalText[22];
	}
	else if(byCharacterClass == CLASS_MUSEELF)
	{
		// 27 "뮤즈엘프"
		return GlobalText[27];
	}
	else if(byCharacterClass == CLASS_HIGHELF)
	{
		// 1670 "하이엘프"
		return GlobalText[1670];
	}
	else if(byCharacterClass == CLASS_DARK)
	{
		// 23 "마검사"
		return GlobalText[23];
	}
	else if(byCharacterClass == CLASS_DUELMASTER)
	{
		// 1671 "듀얼마스터"
		return GlobalText[1671];
	}
	else if(byCharacterClass == CLASS_DARK_LORD)
	{
		// 24 "다크로드"
		return GlobalText[24];
	}
	else if(byCharacterClass == CLASS_LORDEMPEROR)
	{
		// 1672 "로드엠퍼러"
		return GlobalText[1672];
	}
	else if (byCharacterClass == CLASS_SUMMONER)
		return GlobalText[1687];	// 1687 "소환술사"
	else if (byCharacterClass == CLASS_BLOODYSUMMONER)
		return GlobalText[1688];	// 1688 "블러디서머너"
	else if (byCharacterClass == CLASS_DIMENSIONMASTER)
		return GlobalText[1689];	// 1689 "디멘션마스터"
#ifdef PBG_ADD_NEWCHAR_MONK
	else if(byCharacterClass == CLASS_RAGEFIGHTER)
		return GlobalText[3150];	// 3150 "레이지파이터"
	else if(byCharacterClass == CLASS_TEMPLENIGHT)
		return GlobalText[3151];	// 3151 "템플나이트"
#endif //PBG_ADD_NEWCHAR_MONK
	//assert(!"잘못된 클래스 타입입니다.!");
	// 2305 "에러코드 :"
	return GlobalText[2305];
}

bool IsMasterLevel(const BYTE byClass)
{
	return IsThirdClass(byClass);
}


#ifdef YDG_ADD_SKILL_FLAME_STRIKE
BOOL FindHeroSkill(enum ActionSkillType eSkillType)
{
	for (int i = 0; i < CharacterAttribute->SkillNumber; ++i)
	{
		if (CharacterAttribute->Skill[i] == eSkillType)
		{
			return TRUE;
		}
	}
	return FALSE;
}
#endif	// YDG_ADD_SKILL_FLAME_STRIKE

#ifdef ADD_SOCKET_ITEM
// 캐릭터에 장착된 무기가 활/석궁인지를 구별하여 리턴
int GetEquipedBowType(CHARACTER *pChar)
{
	// 활
	if( (pChar->Weapon[1].Type != MODEL_BOW+7) 
		&& ((pChar->Weapon[1].Type >= MODEL_BOW) && (pChar->Weapon[1].Type < MODEL_BOW+MAX_ITEM_INDEX)) 
		)	
	{
		return BOWTYPE_BOW;
	}
	// 석궁
	else if( (pChar->Weapon[0].Type != MODEL_BOW+15)
			&& ((pChar->Weapon[0].Type >= MODEL_BOW+8) && (pChar->Weapon[0].Type < MODEL_BOW+MAX_ITEM_INDEX))
		)
	{
		return BOWTYPE_CROSSBOW;
	}

	// 아무것도 아닐때
	return BOWTYPE_NONE;
}

// 캐릭터에 장착된 무기가 활/석궁인지를 구별하여 리턴
//** 캐릭터에 장착된것만 검사
int GetEquipedBowType( )
{

	// 활
	if( (CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type != ITEM_BOW+7) &&
		((CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type >= ITEM_BOW) 
			&& (CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type < ITEM_BOW+MAX_ITEM_INDEX))
		)
	{
		return BOWTYPE_BOW;
	}
	// 석궁
	else if( (CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type != ITEM_BOW+15) &&
			((CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type >= ITEM_BOW+8) 
				&& (CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type < ITEM_BOW+MAX_ITEM_INDEX)) 
		)
	{
		return BOWTYPE_CROSSBOW;
	}


	// 아무것도 아닐때
	return BOWTYPE_NONE;
}

#ifdef PJH_SEASON4_SPRITE_NEW_SKILL_MULTI_SHOT
int GetEquipedBowType_Skill( )
{

	// 활
	if( (CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type != ITEM_BOW+7) &&
		((CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type >= ITEM_BOW) 
			&& (CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type < ITEM_BOW+MAX_ITEM_INDEX))
		)
	{
		if(CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type == ITEM_BOW+15)
			return BOWTYPE_BOW;
	}
	// 석궁
	else if( (CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type != ITEM_BOW+15) &&
			((CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type >= ITEM_BOW+8) 
				&& (CharacterMachine->Equipment[EQUIPMENT_WEAPON_RIGHT].Type < ITEM_BOW+MAX_ITEM_INDEX)) 
		)
	{
		if(CharacterMachine->Equipment[EQUIPMENT_WEAPON_LEFT].Type == ITEM_BOW+7)
			return BOWTYPE_CROSSBOW;
	}


	// 아무것도 아닐때
	return BOWTYPE_NONE;
}
#endif //#ifdef PJH_SEASON4_SPRITE_NEW_SKILL_MULTI_SHOT


// Item이 활/석궁인지를 구별하여 리턴
// 활이나 석궁 추가는 이곳에서 꼭 해준다.
int GetEquipedBowType( ITEM* pItem )
{	
	// 활
#ifdef KJH_FIX_RELOAD_ARROW_TO_CROSSBOW
	if( 
		((pItem->Type >= ITEM_BOW) && (pItem->Type <= ITEM_BOW+6))
		|| (pItem->Type == ITEM_BOW+17) 
		|| ((pItem->Type >= ITEM_BOW+20) && (pItem->Type <= ITEM_BOW+23))
#ifdef LDK_ADD_GAMBLERS_WEAPONS
		|| (pItem->Type == ITEM_BOW+24) 
#endif //LDK_ADD_GAMBLERS_WEAPONS
		)
#else KJH_FIX_RELOAD_ARROW_TO_CROSSBOW
	if( (pItem->Type != ITEM_BOW+7) 
		&& ((pItem->Type >= ITEM_BOW) && (pItem->Type < ITEM_BOW+MAX_ITEM_INDEX)) 
		)
#endif // KJH_FIX_RELOAD_ARROW_TO_CROSSBOW
	{
		return BOWTYPE_BOW;
	}
	// 석궁
	
#ifdef KJH_FIX_RELOAD_ARROW_TO_CROSSBOW
	else if(  
			 ((pItem->Type >= ITEM_BOW+8) && (pItem->Type <= ITEM_BOW+14))
			 || (pItem->Type == ITEM_BOW+16)
			 || ((pItem->Type >= ITEM_BOW+18) && (pItem->Type <= ITEM_BOW+19))
			 )
#else // KJH_FIX_RELOAD_ARROW_TO_CROSSBOW
	else if( (pItem->Type != ITEM_BOW+15) 
			&& ((pItem->Type >= ITEM_BOW+8) && (pItem->Type < ITEM_BOW+MAX_ITEM_INDEX))
			)
#endif // KJH_FIX_RELOAD_ARROW_TO_CROSSBOW
	{
		return BOWTYPE_CROSSBOW;
	}

	// 아무것도 아닐때
	return BOWTYPE_NONE;
}

#endif // ADD_SOCKET_ITEM

#ifdef KJH_FIX_WOPS_K26606_TRADE_WING_IN_IKARUS
// 날개를 착용 유무 검사.
bool IsEquipedWing( )
{
	ITEM* pEquippedItem = &CharacterMachine->Equipment[EQUIPMENT_WING];

	if( (pEquippedItem->Type >= ITEM_WING && pEquippedItem->Type <= ITEM_WING+6) 
		|| (pEquippedItem->Type >= ITEM_WING+36 && pEquippedItem->Type <= ITEM_WING+43)
		|| (pEquippedItem->Type == ITEM_HELPER+30)								// 군주의 망토
#ifdef LDK_ADD_INGAMESHOP_SMALL_WING			// 기간제 날개 작은(군망, 재날, 요날, 천날, 사날)
		|| ( ITEM_WING+130 <= pEquippedItem->Type && pEquippedItem->Type <= ITEM_WING+134 )
#endif //LDK_ADD_INGAMESHOP_SMALL_WING
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
		|| (pEquippedItem->Type >= ITEM_WING+49 && pEquippedItem->Type <= ITEM_WING+50)	// 레이지파이터날개
		|| (pEquippedItem->Type == ITEM_WING+135)	//작은 무인의망토
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
		)
	{
		return true;
	}

	return false;
}
#endif // KJH_FIX_WOPS_K26606_TRADE_WING_IN_IKARUS

#ifdef YDG_ADD_SKILL_RIDING_ANIMATIONS
void SetActionBloodAttack(CHARACTER *c, OBJECT* o)
{
	switch(c->Helper.Type)
	{
	case MODEL_HELPER+2:	// 유니리아
		SetAction(o, PLAYER_ATTACK_SKILL_WHEEL_UNI);
		break;
	case MODEL_HELPER+3:	// 디노란트
		SetAction(o, PLAYER_ATTACK_SKILL_WHEEL_DINO);
		break;
	case MODEL_HELPER+37:	// 펜릴	
		SetAction(o, PLAYER_ATTACK_SKILL_WHEEL_FENRIR);
		break;
	default:	// 기본
		SetAction(o, PLAYER_ATTACK_SKILL_WHEEL);
		break;
	}
}
#endif // YDG_ADD_SKILL_RIDING_ANIMATIONS

#ifdef YDG_ADD_SANTA_MONSTER
BOOL PlayMonsterSoundGlobal(OBJECT * pObject)	// 전역 몬스터 사운드 처리용
{
	float fDis_x, fDis_y;
	fDis_x = pObject->Position[0] - Hero->Object.Position[0];
	fDis_y = pObject->Position[1] - Hero->Object.Position[1];
	float fDistance = sqrtf(fDis_x*fDis_x+fDis_y*fDis_y);

	if (fDistance > 500.0f) return true;

	// 사운드 처리
	switch(pObject->Type)
	{
	case MODEL_MONSTER01+155:	// 저주받은 산타
		if (pObject->CurrentAction == MONSTER01_STOP1)
		{
// 			if (rand() % 10 == 0)
			{
				if (rand() % 2 == 0)
					PlayBuffer(SOUND_XMAS_SANTA_IDLE_1);
				else
					PlayBuffer(SOUND_XMAS_SANTA_IDLE_2);
			}
		}
		else if (pObject->CurrentAction == MONSTER01_WALK)
		{
			//if (rand() % 10 == 0)
			{
				if (rand() % 2 == 0)
					PlayBuffer(SOUND_XMAS_SANTA_WALK_1);
				else
					PlayBuffer(SOUND_XMAS_SANTA_WALK_2);
			}
		}
		else if(pObject->CurrentAction == MONSTER01_ATTACK1 || pObject->CurrentAction == MONSTER01_ATTACK2)
		{
			PlayBuffer(SOUND_XMAS_SANTA_ATTACK_1);
		}
		else if(pObject->CurrentAction == MONSTER01_SHOCK)
		{
			if (rand() % 2 == 0)
				PlayBuffer(SOUND_XMAS_SANTA_DAMAGE_1);
			else
				PlayBuffer(SOUND_XMAS_SANTA_DAMAGE_2);
		}
		else if(pObject->CurrentAction == MONSTER01_DIE)
		{
			PlayBuffer(SOUND_XMAS_SANTA_DEATH_1);
		}
		return TRUE;

#ifdef LDK_ADD_SNOWMAN_CHANGERING
	case MODEL_XMAS2008_SNOWMAN:
		if (pObject->CurrentAction == MONSTER01_WALK)
		{
			PlayBuffer(SOUND_XMAS_SNOWMAN_WALK_1);
		}
		else if(pObject->CurrentAction == MONSTER01_ATTACK1)
		{
			PlayBuffer(SOUND_XMAS_SNOWMAN_ATTACK_1);
		}
		else if(pObject->CurrentAction == MONSTER01_ATTACK2)
		{
			PlayBuffer(SOUND_XMAS_SNOWMAN_ATTACK_2);
		}
		else if(pObject->CurrentAction == MONSTER01_SHOCK)
		{
			PlayBuffer(SOUND_XMAS_SNOWMAN_DAMAGE_1);
		}
		else if(pObject->CurrentAction == MONSTER01_DIE)
		{
			if(pObject->LifeTime == 100)
			{
				PlayBuffer(SOUND_XMAS_SNOWMAN_DEATH_1);
			}
		}
		return TRUE;
#endif //LDK_ADD_SNOWMAN_CHANGERING

#ifdef YDG_ADD_NEW_DUEL_NPC
	case MODEL_DUEL_NPC_TITUS:	// 결투장 문지기 타이투스
		if (pObject->CurrentAction == MONSTER01_STOP1)
		{
			PlayBuffer(SOUND_DUEL_NPC_IDLE_1);
		}
		return TRUE;
#endif	// YDG_ADD_NEW_DUEL_NPC

#ifdef YDG_ADD_DOPPELGANGER_SOUND
	case MODEL_DOPPELGANGER_NPC_LUGARD:	// 도플갱어 NPC 루가드
		if (pObject->CurrentAction == MONSTER01_STOP1)
		{
			if (rand() % 2 == 0)
				PlayBuffer(SOUND_DOPPELGANGER_LUGARD_BREATH);
		}
		return TRUE;
	case MODEL_DOPPELGANGER_NPC_BOX:	// 도플갱어 보상 상자
		if (pObject->CurrentAction == MONSTER01_DIE)
		{
			PlayBuffer(SOUND_DOPPELGANGER_JEWELBOX_OPEN);
		}
		return TRUE;
	case MODEL_DOPPELGANGER_NPC_GOLDENBOX:	// 도플갱어 최종보상 상자
		if (pObject->CurrentAction == MONSTER01_DIE)
		{
			PlayBuffer(SOUND_DOPPELGANGER_JEWELBOX_OPEN);
		}
		return TRUE;
#endif	// YDG_ADD_DOPPELGANGER_SOUND
	}

	return FALSE;
}
#endif	// YDG_ADD_SANTA_MONSTER

#ifdef CSK_REF_BACK_RENDERITEM
bool IsBackItem(CHARACTER *c, int iType)
{
	// 활이나 석궁이면 붙는다.
	int iBowType = GetEquipedBowType(c);
	if(iBowType != BOWTYPE_NONE)
	{
		return true;
	}

	// 검류, 도끼류, 창류, 활/석궁류, 지팡이류, 방패류이면 등에 붙는다. 책들은 등에 붙지 않는다.
	if(iType >= MODEL_SWORD && iType < MODEL_SHIELD+MAX_ITEM_INDEX && !(iType >= MODEL_STAFF+21 && iType <= MODEL_STAFF+29))
	{
		return true;
	}

	return false;
}

bool RenderCharacterBackItem(CHARACTER *c, OBJECT* o, bool bTranslate)
{
	// 등에 아이템이 달라붙었나?
	bool bBindBack = false;	

	// 등에 아이템이 붙는 일반적인 조건
	// 안전지대, 인사등 특정행동, 아틀란스나 칼리마맵이고 수영중이면 등에 아이템이 달라붙는다.
	if(c->SafeZone)
	{
		bBindBack = true;
	}
	if(o->CurrentAction >= PLAYER_GREETING1 && o->CurrentAction <= PLAYER_SALUTE1)
	{
		bBindBack = true;
	}
	if((World == WD_7ATLANSE || InHellas()
#ifdef YDG_ADD_MAP_DOPPELGANGER3
		|| World == WD_67DOPPLEGANGER3
#endif	// YDG_ADD_MAP_DOPPELGANGER3
		) && (o->CurrentAction==PLAYER_WALK_SWIM || o->CurrentAction==PLAYER_RUN_SWIM ))
	{
		bBindBack = true;
	}

	// 특정맵에서 등에 아이템이 붙지 않는 조건
    // 카오스캐슬, 블러드캐슬
    if(InBloodCastle() == true)
    {
        bBindBack = false;
#ifdef CSK_ADD_GM_ABILITY
		// GM캐릭터가 블캐에서 등에 벽같은거 붙는 버그 수정
		if(IsGMCharacter() == true)
		{
			return bBindBack;
		}
#endif // CSK_ADD_GM_ABILITY
    }
    if( InChaosCastle() == true )
    {
        bBindBack = false;
    }

	// 플레이어이면
	if(o->Type == MODEL_PLAYER)
	{
        bool bBack = false;		// 등에 붙는가?
		int iBackupType = -1;

		// 두손만 검사
		for(int i=0; i<2; ++i)
		{
			int iType    = c->Weapon[i].Type;
			int iLevel   = c->Weapon[i].Level;
			int iOption1 = c->Weapon[i].Option1;

			if(iType < 0)
				continue;

			// 로렌시아에 해골 NPC
			if(o->Kind == KIND_NPC && World == WD_0LORENCIA && o->Type == MODEL_PLAYER && (o->SubType >= MODEL_SKELETON1 && o->SubType <= MODEL_SKELETON3) )
			{
				// 한번만 렌더링 되기 위해서
				if(i == 0)
				{
					bBack = true;
					iType = MODEL_BOW+9;
					iLevel = 8;
				}
			}

			// 등에 붙는 아이템 타입인가?
			if(IsBackItem(c, iType) == true)
			{
				bBack = true;
			}

			// 화살이면 붙는다.
			if(iType == MODEL_BOW+7 || iType == MODEL_BOW+15)
			{
				bBack = true;
			}
			else
			{
				// 화살이 아니고 등에 붙는 조건이 아니면 등에 안붙는다.
				if(bBindBack == false)
					bBack = false;
			}

			// 두손에 같은 아이템을 들고 있으면 한번만 렌더링 되기 위해서
// 			if(iBackupType == iType)
// 			{
// 				bBack = false;
// 			}

			// 등에 붙고 아이템이 있으면
			if(bBack && iType != -1)
			{
				PART_t *w = &c->Wing;
				
				float fAnimationFrameBackUp = w->AnimationFrame;
				
				// 링크본 번호 
				w->LinkBone  = 47;		

				// 에니메이션 속도 설정
				if(o->CurrentAction == PLAYER_FLY || o->CurrentAction == PLAYER_FLY_CROSSBOW)
				{
					w->PlaySpeed = 1.f;
				}
				else
				{
					w->PlaySpeed = 0.25f;
				}
				
				PART_t t_Part;	// 백업용 변수

				if(iType >= MODEL_SWORD && iType < MODEL_SHIELD+MAX_ITEM_INDEX)
				{
					::memcpy(&t_Part, w, sizeof(PART_t));
					t_Part.CurrentAction = 0;
					t_Part.AnimationFrame = 0.f;
					t_Part.PlaySpeed = 0.f;
					t_Part.PriorAction = 0;
					t_Part.PriorAnimationFrame = 0.f;
				}
				
				if( iType == MODEL_BOW+23 )
				{
					// 다크스팅거(23)는 등에 link가 되었을 때 에도 애니메이션이 있습니다.
					PART_t *pWeapon = &c->Weapon[1];
					BYTE byTempLinkBone = pWeapon->LinkBone;
					pWeapon->CurrentAction = 2;
					pWeapon->PlaySpeed = 0.25f;
					pWeapon->LinkBone = 47;
					RenderLinkObject(0.f, 0.f, 15.f, c, pWeapon, iType, iLevel, iOption1, true, bTranslate);
					pWeapon->LinkBone = byTempLinkBone;
				}
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
				else if(g_CMonkSystem.IsSwordformGloves(iType))
					g_CMonkSystem.RenderSwordformGloves(c, iType, i, o->Alpha, bTranslate);
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
				else
				{
					bool bRightHandItem = false;
					if(i == 0)	// 오른손무기이면
						bRightHandItem = true;
					RenderLinkObject(0.f,0.f,15.f,c,&t_Part,iType,iLevel,iOption1, true, bTranslate, 0, bRightHandItem);
				}
				
				w->AnimationFrame = fAnimationFrameBackUp;
			}

			iBackupType = iType;
		}

        //  블러드캐슬의 얻은 절대무기를 등에 표현한다.
        if ( InBloodCastle() && c->EtcPart != 0 )
        {
			PART_t *w = &c->Wing;

			int iType = 0;
			int iLevel = 0;
			int iOption1 = 0;

			w->LinkBone = 47;
			if(o->CurrentAction == PLAYER_FLY || o->CurrentAction == PLAYER_FLY_CROSSBOW)
				w->PlaySpeed = 1.f;
			else
				w->PlaySpeed = 0.25f;

            switch ( c->EtcPart )
            {
#ifdef LJW_FIX_PARTS_ENUM
            case PARTS_STAFF: iType = MODEL_STAFF+10; break;
            case PARTS_SWORD: iType = MODEL_SWORD+19; break;
            case PARTS_BOW  : iType = MODEL_BOW+18; break;
#else 
            case 1: iType = MODEL_STAFF+10; break;
            case 2: iType = MODEL_SWORD+19; break;
            case 3: iType = MODEL_BOW+18; break;
#endif // LJW_FIX_PARTS_ENUM
            }

			RenderLinkObject(0.f,0.f,15.f, c, w, iType, iLevel, iOption1, true, bTranslate);
        }

        CreatePartsFactory ( c );
        RenderParts ( c );

		// 카오스캐슬이 아니면 날개, 사탄 렌더링
        if(InChaosCastle() == false)
        {
			PART_t *w = &c->Wing;
			if(w->Type != -1)
			{
				w->LinkBone = 47;
				if(o->CurrentAction == PLAYER_FLY||o->CurrentAction == PLAYER_FLY_CROSSBOW)
				{
					if(w->Type == MODEL_WING+36)
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

				// 날개 위치 보정 - 왠만하면 모델을 고칠것->스케일, 붙는 위치 모두! (안그러면 펜릴탈때 이상함;)
				switch(w->Type)		
				{
				case MODEL_WING+40:	// 제왕의 망토 (다크로드)
#ifdef PBG_ADD_NEWCHAR_MONK_ITEM
				case MODEL_WING+50:
#endif //PBG_ADD_NEWCHAR_MONK_ITEM
					w->LinkBone  = 19;	// 목 본에 붙인다
					RenderLinkObject(0.f,0.f,15.f,c,w,w->Type,w->Level,w->Option1,true,bTranslate);
					break;
				default:
					RenderLinkObject(0.f,0.f,15.f,c,w,w->Type,w->Level,w->Option1,false,bTranslate);
					break;
				}
			}

            // 사탄
			int iType = c->Helper.Type;
			int iLevel = c->Helper.Level;
			int iOption1 = 0;
			if(iType == MODEL_HELPER+1)	// 사탄
			{
				PART_t *w = &c->Helper;
				w->LinkBone  = 34;
				w->PlaySpeed = 0.5f;
				iOption1 = w->Option1;
#ifdef PBG_ADD_NEWCHAR_MONK
				if(GetBaseClass(c->Class) == CLASS_RAGEFIGHTER 
					&& (c->BodyPart[BODYPART_ARMOR].Type == MODEL_ARMOR+59
					|| c->BodyPart[BODYPART_ARMOR].Type == MODEL_ARMOR+60
					|| c->BodyPart[BODYPART_ARMOR].Type == MODEL_ARMOR+61))
					RenderLinkObject(20.f, -5.f, 20.f, c, w, iType, iLevel, iOption1, false, bTranslate);
				else
#endif //PBG_ADD_NEWCHAR_MONK
				RenderLinkObject(20.f, 0.f, 0.f, c, w, iType, iLevel, iOption1, false, bTranslate);
				vec3_t vRelativePos, vPos, vLight;
				Vector(20.f, 0.f, 15.f, vRelativePos);
#ifdef PBG_ADD_NEWCHAR_MONK
				if(GetBaseClass(c->Class) == CLASS_RAGEFIGHTER 
					&& (c->BodyPart[BODYPART_ARMOR].Type == MODEL_ARMOR+59
					|| c->BodyPart[BODYPART_ARMOR].Type == MODEL_ARMOR+60
					|| c->BodyPart[BODYPART_ARMOR].Type == MODEL_ARMOR+61))
					Vector(20.f, -5.f, 35.f, vRelativePos);
#endif //PBG_ADD_NEWCHAR_MONK
				Models[o->Type].TransformPosition(o->BoneTransform[w->LinkBone], vRelativePos, vPos, true);

				// 스프라이트
				float fLuminosity = (float)(rand()%30+70)*0.01f;
				Vector(fLuminosity*0.5f, fLuminosity*0.f, fLuminosity*0.f, vLight);
				CreateSprite(BITMAP_LIGHT, vPos, 1.5f, vLight ,o);
			}
        }
	}

	return bBindBack;
}
#endif // CSK_REF_BACK_RENDERITEM
