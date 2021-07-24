///////////////////////////////////////////////////////////////////////////////
// 3D 특수효과 관련 함수
//
// *** 함수 레벨: 3
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ZzzOpenglUtil.h"
#include "ZzzBMD.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzLodTerrain.h"
#include "ZzzTexture.h"
#include "ZzzAi.h"
#include "ZzzEffect.h"
#include "DSPlaySound.h"
#include "wsclientinline.h"
#include "GMCrywolf1st.h"
#include "GMHellas.h"
#include "GMBattleCastle.h"
#include "UIManager.h"
#include "GM_Kanturu_2nd.h"
#include "CDirection.h"
#ifdef PRUARIN_EVENT07_3COLORHARVEST
#include "BoneManager.h"
#endif // PRUARIN_EVENT07_3COLORHARVEST
#ifdef CSK_HACK_TEST
#include "HackTest.h"
#endif // CSK_HACK_TEST
#ifdef YDG_MOD_SEPARATE_EFFECT_SKILLS
#include "SkillEffectMgr.h"
#endif	// YDG_MOD_SEPARATE_EFFECT_SKILLS

PARTICLE  Particles	[MAX_PARTICLES];
#ifdef DEVIAS_XMAS_EVENT
PARTICLE  Leaves	[MAX_LEAVES_DOUBLE];
#else // DEVIAS_XMAS_EVENT
PARTICLE  Leaves	[MAX_LEAVES];
#endif // DEVIAS_XMAS_EVENT
JOINT     Joints	[MAX_JOINTS];

extern int MoveSceneFrame;

BYTE    g_byUpperBoneLocation[7] = { 25, 26, 27, 20, 34, 35, 36 };
///////////////////////////////////////////////////////////////////////////////
// 여러가지 이팩트 효과들
///////////////////////////////////////////////////////////////////////////////

#ifdef MR0
AUTOOBJ Effects[MAX_EFFECTS];
#else
OBJECT Effects[MAX_EFFECTS];
#endif //MR0

bool CheckCharacterRange(OBJECT *so,float Range,short PKKey, BYTE Kind=0 )
{
	for(int i=0;i<MAX_CHARACTERS_CLIENT;i++)
	{
		CHARACTER *c = &CharactersClient[i];
		OBJECT    *o = &c->Object;

        if ( !o->Live || ( Kind!=0 && Kind==o->Kind ) ) continue;

		if(so->Owner != o)
		{
			float dx = so->Position[0]-o->Position[0];
			float dy = so->Position[1]-o->Position[1];
			float Distance = sqrtf(dx*dx+dy*dy);

			if ( o->Live && o->Visible && ( c!=Hero || ( Kind==KIND_MONSTER ) ) && !c->Dead && Distance<=Range )
			{
				return true;
			}
		}
	}
	return false;
}

bool FindSameKey( int DamageKey[], int iCount, int Key)
{
	for ( int i = 0; i < iCount; i++)
	{
		if ( DamageKey[i] == Key)
		{
			return true;
		}
	}

	return false;
}

bool AddRangeAttack( vec3_t Position, float Range, short PKKey, int DamageKey[], int *piCount, int iMaxKey)
{
	if ( *piCount >= iMaxKey)
	{
		return false;
	}

	for(int i=0;i<MAX_CHARACTERS_CLIENT;i++)
	{
		CHARACTER *c = &CharactersClient[i];
		OBJECT    *o = &c->Object;
		float dx = Position[0]-o->Position[0];
		float dy = Position[1]-o->Position[1];
		float Distance = sqrtf(dx*dx+dy*dy);
		if(o->Live && o->Visible && c!=Hero && !c->Dead && Distance<=Range &&
			(o->Kind==KIND_MONSTER || (o->Kind==KIND_PLAYER&&c->Key==PKKey)))
		{
			if ( !FindSameKey( DamageKey, *piCount, c->Key))
			{
				DamageKey[( *piCount)++] = c->Key;
				if(*piCount >= iMaxKey) break;
			}
		}
	}

	return true;
}



bool AttackCharacterRange(int Index,vec3_t Position,float Range,BYTE Serial,short PKKey,WORD SkillSerialNum)
{
	int Skill = CharacterAttribute->Skill[Index];

	int     Count = 0;
	int     DamageKey[5];
    bool    DamageChr = false;

    //  공성전시.
    if ( battleCastle::InBattleCastle() && battleCastle::IsBattleCastleStart() ) 
    {
        DWORD att = TERRAIN_ATTRIBUTE ( Position[0], Position[1] );
        if ( (att&TW_NOATTACKZONE)==TW_NOATTACKZONE )
        {
            return false;
        }
        DamageChr = true;
    }

	for(int i=0;i<MAX_CHARACTERS_CLIENT;i++)
	{
		CHARACTER *c = &CharactersClient[i];
		OBJECT    *o = &c->Object;
		
		if(o->Live == false 
			|| o->Visible == false
			|| c == Hero
			|| (bool)c->Dead == true)
		{
			continue;
		}

		float dx = Position[0]-o->Position[0];
		float dy = Position[1]-o->Position[1];
		float Distance = sqrtf(dx*dx+dy*dy);
		
        //  클라이언트에서 데미지 처리.
		if (Distance <= Range 
			&& ( o->Kind==KIND_MONSTER || ( o->Kind==KIND_PLAYER && ( c->Key==PKKey || DamageChr ) ) ) )
		{
			if ( Skill==AT_SKILL_STORM || Skill==AT_SKILL_EVIL 
#ifdef PJH_SEASON4_MASTER_RANK4
				|| (AT_SKILL_EVIL_SPIRIT_UP <= Skill && AT_SKILL_EVIL_SPIRIT_UP+4 >= Skill)
				|| (AT_SKILL_EVIL_SPIRIT_UP_M <= Skill && AT_SKILL_EVIL_SPIRIT_UP_M+4 >= Skill)
#endif //PJH_SEASON4_MASTER_RANK4
				)
			{
				if ( c->m_bFixForm==false )
				{
					c->StormTime = 10;
				}

#ifdef CSK_RAKLION_BOSS
				// 세루판 보스와 거미알 몬스터는 악령이나 기타 스킬 타격시 회전되는 현상 막는 처리
				if(c->MonsterIndex >= 459 && c->MonsterIndex <= 462)
				{
					c->StormTime = 0;
				}
#endif // CSK_RAKLION_BOSS
#ifdef LDK_FIX_EG_DOOR_ROTATION_FIXED
				// 제국 수호군 성문, 석상 스킬타격시 회전 불가
				else if( 524 <= c->MonsterIndex && c->MonsterIndex <= 528 )
				{
					c->StormTime = 0;
				}
#endif //LDK_FIX_EG_DOOR_ROTATION_FIXED
			}

            DamageKey[Count++] = c->Key;
			if(Count >= 5)
			{
				break;
			}
		}
	}

#ifdef USE_SELFCHECKCODE
	SendCrcOfFunction( 7, 17, Action, 0x823F);
#endif
	if(Count > 0)
	{
		if(Skill == AT_SKILL_DARK_SCREAM
#ifdef PJH_SEASON4_MASTER_RANK4
	|| (AT_SKILL_FIRE_SCREAM_UP <= Skill && AT_SKILL_FIRE_SCREAM_UP+4 >= Skill)
#endif //PJH_SEASON4_MASTER_RANK4
			)
		{
#ifdef CONSOLE_DEBUG
			g_ConsoleDebug->Write(MCD_SEND, "클라이언트에서 판별 스킬번호 : %d", Skill);
			//g_ConsoleDebugLog->log("클라이언트에서 판별 스킬 : %s[%d]\n", SkillAttribute[Skill].Name, Skill);
#endif // CONSOLE_DEBUG
#ifdef CSK_HACK_TEST
			g_pHackTest->AddAttackCount(Skill);
#endif // CSK_HACK_TEST
#ifdef PBG_FIX_DARK_FIRESCREAM_HACKCHECK
			BYTE _SerialTemp = (BYTE)SkillSerialNum;
			SendRequestMagicAttack(Skill,(int)(Position[0]/TERRAIN_SCALE),(int)(Position[1]/TERRAIN_SCALE),_SerialTemp,Count,DamageKey,SkillSerialNum);
#else //PBG_FIX_DARK_FIRESCREAM_HACKCHECK
			SendRequestMagicAttack(Skill,(int)(Position[0]/TERRAIN_SCALE),(int)(Position[1]/TERRAIN_SCALE),SkillSerialNum,Count,DamageKey,SkillSerialNum);
#endif //PBG_FIX_DARK_FIRESCREAM_HACKCHECK
		}
		else
		{
#ifdef PJH_SEASON4_SPRITE_NEW_SKILL_MULTI_SHOT
			if(Skill != AT_SKILL_MULTI_SHOT)
#endif //#define PJH_SEASON4_SPRITE_NEW_SKILL_MULTI_SHOT
			{
#ifdef CONSOLE_DEBUG
				g_ConsoleDebug->Write(MCD_SEND, "클라이언트에서 판별 스킬번호 : %d", Skill);
				//g_ConsoleDebugLog->log("클라이언트에서 판별 스킬 : %s[%d]\n", SkillAttribute[Skill].Name, Skill);
#endif // CONSOLE_DEBUG
#ifdef CSK_HACK_TEST
				g_pHackTest->AddAttackCount(Skill);
#endif // CSK_HACK_TEST
				SendRequestMagicAttack(Skill,(int)(Position[0]/TERRAIN_SCALE),(int)(Position[1]/TERRAIN_SCALE),Serial,Count,DamageKey,SkillSerialNum);
			}
		}
		return true;
	}
	return false;
#ifdef USE_SELFCHECKCODE
	END_OF_FUNCTION( Pos_SelfCheck01);
Pos_SelfCheck01:
	;
#endif
}

void CreateHealing(OBJECT *o)
{
	vec3_t Angle,p,Position;
	float Matrix[3][4];
	Vector(0.f,-200.f,0.f,p);
	for(int j=0;j<3;j++)
	{
		Vector((float)(rand()%90),0.f,(float)(rand()%360),Angle);
		AngleMatrix(Angle,Matrix);
		VectorRotate(p,Matrix,Position);
		VectorSubtract(o->Position,Position,Position);
		Position[2] += 120.f;
		if(o->SubType == 5) 
		{
			CreateJoint(BITMAP_JOINT_HEALING,Position,o->Position,Angle,11,o->Owner,5.f);

		}
		else 
		{
			CreateJoint(BITMAP_JOINT_HEALING,Position,o->Position,Angle,o->SubType,o->Owner,5.f);
		}
	}
}

void CreateForce ( OBJECT* o, vec3_t Pos )
{
	vec3_t Angle,p,Position;
	float Matrix[3][4];
	Vector(0.f,-500.f,0.f,p);
	for(int j=0;j<3;j++)
	{
		Vector((float)(rand()%90),0.f,(float)(rand()%360),Angle);
		AngleMatrix(Angle,Matrix);
		VectorRotate(p,Matrix,Position);
		VectorSubtract(Pos,Position,Position);
		Position[2] += 120.f;
		CreateJoint(BITMAP_JOINT_HEALING,Position,Pos,Angle,8,o,10.f);
	}
}

#ifdef YDG_ADD_SKILL_FLAME_STRIKE
void EffectDestructor(OBJECT *o)
{
	// 이펙트가 사라질때 호출되는 함수 - 지저분한 이펙트 잔여물 제거용
	// DeleteEffect와 LifeTime < 0일때만 자동 호출됨 (MoveEffect에서 자체 Life=false한 이펙트는 여기로 들어오지 않는다)
	switch(o->Type)
	{
	case MODEL_EFFECT_FLAME_STRIKE:
		RemoveObjectBlurs(o, 1);
		RemoveObjectBlurs(o, 2);
		RemoveObjectBlurs(o, 3);
		break;
#ifdef YDG_FIX_RARGLE_CRASH
	case MODEL_SUMMONER_SUMMON_LAGUL:
		for (int i = 48; i <= 53; ++i)
		{
			DeleteJoint(BITMAP_JOINT_ENERGY, o, i);
		}
		break;
#endif	// YDG_FIX_RARGLE_CRASH
	}

	o->Live = false;
	o->Owner = NULL;
}
#endif	// YDG_ADD_SKILL_FLAME_STRIKE


#ifdef LDS_FIX_AFTER_PETDESTRUCTOR_ATTHESAMETIME_TERMINATE_EFFECTOWNER
// DanglingPointer(SourcePointer가 NULL이 된상황에 ReferencePointer가 NULL아닌 NULL이 된 경우)
void TerminateOwnerEffectObject( int iOwnerObjectType )	
{
	if( iOwnerObjectType < 0 )
	{
		return;
	}
	
	for(int i =0; i<MAX_EFFECTS;i++)
	{
		OBJECT	*o = &Effects[i];
		
#ifdef LDS_FIX_PETDESTRUCTOR_TERMINATE_EFFECTOWNER_ADD_TYPE
		if( o->Owner != NULL &&
			o->Type == MODEL_AIR_FORCE &&			// 다크 스피릿 공격:돌진 Effect::MODEL_AIR_FORCE에 대한 Dangling 현상 방지.
			o->Owner->Type == iOwnerObjectType )
#else // LDS_FIX_PETDESTRUCTOR_TERMINATE_EFFECTOWNER_ADD_TYPE
		if( o->Owner != NULL && o->Owner->Type == iOwnerObjectType )
#endif // LDS_FIX_PETDESTRUCTOR_TERMINATE_EFFECTOWNER_ADD_TYPE
		{			
			o->Owner = NULL;
		}
	}
}
#endif // LDS_FIX_AFTER_PETDESTRUCTOR_ATTHESAMETIME_TERMINATE_EFFECTOWNER

bool DeleteEffect(int Type,OBJECT *Owner, int iSubType)
{
    bool bDelete = false;
	for(int i=0;i<MAX_EFFECTS;i++)
	{
		OBJECT *o = &Effects[i];
		if(o->Live && o->Type==Type)
		{
			if ( iSubType == -1 || iSubType == o->SubType)
			{
				if(o->Owner==Owner)
                {
#ifdef YDG_ADD_SKILL_FLAME_STRIKE
					EffectDestructor(o);
                    bDelete = true;
#else	// YDG_ADD_SKILL_FLAME_STRIKE
					o->Live = false;
                    bDelete = true;
					o->Owner = NULL;
#endif	// YDG_ADD_SKILL_FLAME_STRIKE
                }
			}
		}
	}

#ifdef YDG_MOD_SEPARATE_EFFECT_SKILLS
	if (bDelete == false)
	{
		bDelete = g_SkillEffects.DeleteEffect(Type, Owner, iSubType);
	}
#endif	// YDG_MOD_SEPARATE_EFFECT_SKILLS

    return bDelete;
}

void DeleteEffect( int efftype )
{
	for(int i=0;i<MAX_EFFECTS;i++)
	{
		OBJECT *o = &Effects[i];
		if(o->Live && o->Type==efftype)
		{
#ifdef YDG_ADD_SKILL_FLAME_STRIKE
			EffectDestructor(o);
#else	// YDG_ADD_SKILL_FLAME_STRIKE
			o->Live = false;
			o->Owner = NULL;
#endif	// YDG_ADD_SKILL_FLAME_STRIKE
		}
	}

#ifdef YDG_MOD_SEPARATE_EFFECT_SKILLS
	g_SkillEffects.DeleteEffect(efftype);
#endif	// YDG_MOD_SEPARATE_EFFECT_SKILLS
}

bool DeleteParticle(int iType)
{
	for(int i=0; i<MAX_PARTICLES; ++i)
	{
		PARTICLE *o = &Particles[i];
		if(o->Live && o->Type == iType)
		{
			o->Live = false;
			o->Target = NULL;
			return true;
		}
	}

	return false;
}

#ifdef CSK_LUCKY_SEAL
bool SearchEffect(int iType, OBJECT* pOwner, int iSubType)
{
	for(int i=0; i<MAX_EFFECTS; ++i)
	{
		OBJECT *o = &Effects[i];
		if(o->Live && o->Type == iType && o->Owner == pOwner)
		{
			if(iSubType == -1 || o->SubType == iSubType)
			{
				return true;
			}
		}
	}

#ifdef YDG_MOD_SEPARATE_EFFECT_SKILLS
	return g_SkillEffects.SearchEffect(iType, pOwner, iSubType);
#endif	// YDG_MOD_SEPARATE_EFFECT_SKILLS

    return false;
}
#endif // CSK_LUCKY_SEAL

BOOL FindSameEffectOfSameOwner( int iType, OBJECT *pOwner)
{
	for(int i=0;i<MAX_EFFECTS;i++)
	{
		OBJECT *o = &Effects[i];
		if(o->Live)
		{
			if ( o->Type == iType && o->Owner == pOwner)
			{
				return ( TRUE);
			}
		}
	}

#ifdef YDG_MOD_SEPARATE_EFFECT_SKILLS
	return g_SkillEffects.FindSameEffectOfSameOwner(iType, pOwner);
#endif	// YDG_MOD_SEPARATE_EFFECT_SKILLS

	return ( FALSE);
}


#ifdef TAMAJJANG

//////////////////////////////////////////////////////////////////////////
//  타마짱을 생성한다.
//////////////////////////////////////////////////////////////////////////
void CreateTamaJjang ( void )
{
    vec3_t  Position, Angle;

    Position[0] = 16284.f;
    Position[1] = 12031.f;
    Position[2] = RequestTerrainHeight ( Position[0], Position[1] );

    Vector ( 0.f, 0.f, 0.f, Angle );
	Angle[2] = CreateAngle ( Position[0], Position[1], Hero->Object.Position[0], Hero->Object.Position[1] );
    CreateEffect ( MODEL_TAMAJJANG, Position, Angle, Angle );
}


//////////////////////////////////////////////////////////////////////////
//  타마짱을 제거한다.
//////////////////////////////////////////////////////////////////////////
void DeleteTamaJjang ( void )
{
	for ( int i=0; i<MAX_EFFECTS; i++ )
	{
		OBJECT* o = &Effects[i];

        if ( o->Live && o->Type==MODEL_TAMAJJANG )
        {
            o->Live = 0;
            o->LifeTime = -1;
        }
    }
}

//////////////////////////////////////////////////////////////////////////
//  타마짱에게 잠수하라한다.
//////////////////////////////////////////////////////////////////////////
void HideTamaJjang ( void )
{
	for ( int i=0; i<MAX_EFFECTS; i++ )
	{
		OBJECT* o = &Effects[i];

        if ( o->Live && o->Type==MODEL_TAMAJJANG )
        {
            o->LifeTime = 10;
            o->AI = 3;
        }
    }
}

#endif //TAMAJJANG

void CheckTargetRange(OBJECT *o)
{
	OBJECT *to = o->Owner;
	if(to!=NULL && to->Live && to->Visible)
	{
		float dx = o->Position[0]-to->Position[0];
		float dy = o->Position[1]-to->Position[1];
		float Range = sqrtf(dx*dx+dy*dy);
		if(Range <= 100.f)
		{
 			o->LifeTime = 1;
			vec3_t Light;
			Vector(1.f,1.f,1.f,Light);
			switch(o->Type)
			{
			case MODEL_ARROW:
			case MODEL_ARROW_STEEL:
			case MODEL_ARROW_THUNDER:
			case MODEL_ARROW_LASER:
			case MODEL_ARROW_V:
			case MODEL_ARROW_SAW:
			case MODEL_ARROW_NATURE:
			case MODEL_ARROW_WING:
			case MODEL_ARROW_BOMB:
            case MODEL_LACEARROW:
			case MODEL_DARK_SCREAM:
			case MODEL_DARK_SCREAM_FIRE:
			case MODEL_ARROW_SPARK:
			case MODEL_ARROW_RING:
			case MODEL_ARROW_TANKER:
#ifdef ADD_SOCKET_ITEM
			case MODEL_ARROW_DARKSTINGER:
#endif // ADD_SOCKET_ITEM
#ifdef LDK_ADD_GAMBLERS_WEAPONS
			case MODEL_ARROW_GAMBLE:
#endif //LDK_ADD_GAMBLERS_WEAPONS
				break;
			case MODEL_FIRE:
				if(o->SubType==1)
				{
					for(int j=0;j<2;j++)
						CreateEffect(MODEL_STONE1+rand()%2,o->Position,o->Angle,o->Light);
				}
				break;

			case BITMAP_ENERGY:
           		CreateParticle(BITMAP_SPARK+1,o->Position,o->Angle,Light,1,6.f);
				break;

			case MODEL_LIGHTNING_ORB:
				{
					CreateEffect(MODEL_LIGHTNING_ORB, o->Position, o->Angle, o->Light, 1);
				}
				break;
				
			case MODEL_SNOW1:
                {
                    for(int j=0;j<2;j++)
				    {
					    CreateEffect(MODEL_SNOW2+rand()%2,o->Position,o->Angle,o->Light);
					    CreateParticle(BITMAP_SMOKE,o->Position,o->Angle,o->Light);
				    }
               	    PlayBuffer(SOUND_BREAK01);
                }
				break;
			case MODEL_WOOSISTONE:
				if(Range <= 30.f)
                {
                    for(int j=0;j<20;j++)
				    {
					    CreateEffect(MODEL_WOOSISTONE,o->Position,o->Angle,o->Light,1);
						//Vector(1.0f, 0.5f, 0.5f, Light);
					    CreateParticle(BITMAP_FIRE,o->Position,o->Angle,o->Light,0,1,o);
						//Vector(0.5f,0.2f,0.2f,Light);	// 연기 펑
					    //CreateParticle(BITMAP_FIRE,o->Position,o->Angle,o->Light,6,1,o);
						//CreateParticle(BITMAP_SMOKE,o->Position,o->Angle,Light,2);	// 연기
						//CreateParticle(BITMAP_SMOKE,o->Position,o->Angle,Light,48);	// 연기
				    }
               	    PlayBuffer(SOUND_BREAK01);
                }
				else
				{
		 			o->LifeTime = 100;
				}
				break;
			}
  		}
	}
}

void CreateEffect(int Type,vec3_t Position,vec3_t Angle,
				  vec3_t Light,int SubType,OBJECT *Owner,short PKKey,WORD SkillIndex,WORD Skill,WORD SkillSerialNum,
				  float Scale, short int sTargetIndex )
{

	for(int icntEffect=0;icntEffect<MAX_EFFECTS;icntEffect++)
	{

		OBJECT *o = &Effects[icntEffect];

#ifdef YDG_MOD_SEPARATE_EFFECT_SKILLS
		// 스킬 이펙트면 스킬이펙트 구조의 메모리를 사용한다
		if (g_SkillEffects.IsSkillEffect(Type, Position, Angle, Light, SubType, Owner, PKKey, SkillIndex, Skill, SkillSerialNum, Scale, sTargetIndex))
		{
			o = g_SkillEffects.CreateEffect();
		}
#endif	// YDG_MOD_SEPARATE_EFFECT_SKILLS

		if(!o->Live)
		{
			o->Live        = true;
			o->Type        = Type;
			o->SubType     = SubType;
			o->LightEnable = true;
			o->HiddenMesh  = -1;
			o->BlendMesh   = -1;
			o->BlendMeshLight = 1.f;
			o->BlendMeshTexCoordU = 0.f;
			o->BlendMeshTexCoordV = 0.f;
			o->AnimationFrame = 0.f;
			o->AlphaEnable    = false;
			o->Alpha          = 1.f;
			o->PriorAnimationFrame = 0.f;

			if(Scale <= 0.0f)
				o->Scale          = 0.9f;
			else
				o->Scale		  = Scale;
			
			if(Owner) 
				o->Owner		  = Owner;
			else
				o->Owner		  = NULL;

            o->Velocity       = 0.3f;
			o->PKKey          = PKKey;
			o->Kind           = Skill;
			o->Skill          = SkillIndex;
            o->RenderType     = NULL;
            o->AttackPoint[0] = 0;
			o->CurrentAction = 0;
			o->m_bySkillSerialNum = (BYTE)SkillSerialNum;
			o->m_sTargetIndex = sTargetIndex;

			//Vector(1.f,1.f,1.f,o->Light);
			VectorCopy(Light,o->Light);
			VectorCopy(Angle,o->Angle);
			VectorCopy(Position,o->Position);
    		Vector(0.f,0.f,0.f,o->Direction);
			float Matrix[3][4];
			vec3_t p1,p2;
			switch(Type)
			{
			case MODEL_DRAGON:
			{
				o->CollisionRange = true;
				o->Kind = 0;
				o->Timer = 0.0f;
		
				o->Velocity = 1.0f;
				o->Angle[1] = 0.0f;
				o->LifeTime = 1000;
				o->Gravity = 1.0f;
				o->Distance = 1.0f;
				o->Position[2] += 3400.f;
				Vector ( 0.f, -35.0f, 0.f, o->Direction );
				VectorCopy(o->Position, o->StartPosition);
				Vector(1.0f, 1.0f, 1.0f, o->Light);		
			}
			break;
			case MODEL_ARROW_AUTOLOAD:
				{
					if(o->SubType == 0)
					{
						o->LifeTime = 40;
					}
					else if(o->SubType == 1)
					{
						o->LifeTime = 40;
						o->Scale = 1.2f;
						Vector(1.0f, 0.8f, 0.2f, o->Light);
						VectorCopy(o->Light, o->Direction);	
					}
				}
				break;
			case MODEL_INFINITY_ARROW:
				{
					if(o->SubType == 0)
					{
						o->LifeTime = 40;
						CreateEffect(MODEL_INFINITY_ARROW, Owner->Position, o->Angle, o->Light, 1, Owner);
						CreateEffect(MODEL_INFINITY_ARROW1, Owner->Position, o->Angle, o->Light, 0, Owner);
						CreateEffect(MODEL_INFINITY_ARROW2, Owner->Position, o->Angle, o->Light, 0, Owner);
						CreateEffect(MODEL_INFINITY_ARROW3, Owner->Position, o->Angle, o->Light, 0, Owner);
						CreateEffect(MODEL_INFINITY_ARROW4, Owner->Position, o->Angle, o->Light, 0, Owner);
					}
					else if(o->SubType == 1)
					{
						o->LifeTime = 40;
						o->Scale = 1.0f;
						Vector(1.f, 1.f, 1.f, o->Light);
						VectorCopy(o->Light , o->Direction);
					}
				}
				break;
#ifdef PJH_SEASON4_DARK_NEW_SKILL_CAOTIC
			case MODEL_DESAIR:
				{
					o->LifeTime = 52;
//					o->BlendMesh = -2;
					o->Scale    = 1.4f;
				}
				break;
#endif //SKILL_DEBUG
			case MODEL_INFINITY_ARROW1:
			case MODEL_INFINITY_ARROW2:
			case MODEL_INFINITY_ARROW3:
				{
					o->LifeTime = 40.f;
					o->Scale = 1.f;
					Vector(1.f, 1.f, 1.f, o->Light);
					VectorCopy(o->Light, o->Direction);

					if(o->SubType == 1 || o->SubType == 2 || o->SubType == 3)
					{
						o->LifeTime = 60.0f;
						o->Scale = 1.f;
//						Vector(0.9f, 0.1f, 0.f, o->Light);
					}
				}
				break;

			case MODEL_INFINITY_ARROW4:
				{
					o->LifeTime = 15.f;
					o->Scale = 1.f;
					Vector(1.f, 0.5f, 0.3f, o->Light);
					VectorCopy(o->Light, o->Direction);
				}
				break;

			case MODEL_SHIELD_CRASH:
				{
					//실드가 깨지면서 나오는 이펙트
					o->LifeTime = 24;
					o->Scale = 1.1f;
					o->Gravity = o->Velocity;
					Vector(0.5f, 0.5f, 1.f, o->Light);
					VectorCopy(o->Light, o->Direction);

					if( o->SubType == 0 )
					{
						CreateEffect ( MODEL_SHIELD_CRASH2, Owner->Position, Owner->Angle, o->Light, 0, o->Owner);
					}
					else
					{
						if( o->SubType == 2 )
						{
							o->Scale = 0.7;
						}
						o->HiddenMesh = 0;
					}
				}
				break;

			case MODEL_SHIELD_CRASH2:
				{
					o->LifeTime = 24;
					o->Scale = 1.1f;
					o->Gravity = o->Velocity;
					Vector(0.5f, 0.5f, 1.f, o->Light);
					VectorCopy(o->Light, o->Direction);
				}
				break;

			case MODEL_IRON_RIDER_ARROW:
				{
					o->LifeTime = 12;
					o->Scale = 0.5f;
					o->Velocity = 70.f;
					Vector(0.8f, 1.0f, 0.8f, o->Light);
					
					vec3_t vDir;
					vec34_t vMat;
					Vector(0.f, -1.f, 0.f, vDir);
					AngleMatrix(o->Angle, vMat);
					VectorRotate(vDir, vMat, o->Direction);
					CreateJoint(BITMAP_JOINT_HEALING, o->Position, o->Position, o->Angle, 14, o, 30.f);
				}
				break;
			case MODEL_BLADE_SKILL:
				{
					o->LifeTime = 10;
					o->Scale = 1.5f;
#ifdef PJH_SEASON4_SPRITE_NEW_SKILL_MULTI_SHOT
					if(o->SubType == 1)
					{
						o->Scale = 1.f;
						o->LifeTime = 14;
					}
#endif //PJH_SEASON4_SPRITE_NEW_SKILL_MULTI_SHOT
				}
				break;
			case MODEL_KENTAUROS_ARROW:
				{
					o->LifeTime = 34;
					o->Scale = 0.7f;
					o->Velocity = 70.f;
					o->Alpha = 0.f;
					Vector(1.0f, 1.0f, 1.0f, o->Light);
/*					
					vec3_t vDir;
					vec34_t vMat;
					Vector(0.f, -1.f, 0.f, vDir);
					AngleMatrix(o->Angle, vMat);
					VectorRotate(vDir, vMat, o->Direction);
					CreateJoint ( BITMAP_FLARE+1, o->Position, o->Position, o->Angle, 17, o, 15.f, 40 );
					*/
//					CreateJoint ( BITMAP_JOINT_FORCE, o->Position, o->Position, o->Angle, 7, o, 150.f, 40 );
//					CreateJoint ( BITMAP_FLARE+1, o->Position, o->Position, o->Angle, 14, o, 50.f, 40 );
//					CreateJoint(BITMAP_JOINT_ENERGY,o->Position,o->Position,o->Angle,5,o,100.f);
//					CreateJoint(BITMAP_JOINT_HEALING, o->Position, o->Position, o->Angle, 14, o, 30.f);
				}
				break;
			case MODEL_WARP3:
#ifdef CSK_ADD_MAP_ICECITY
			case MODEL_WARP6:
#endif // CSK_ADD_MAP_ICECITY
				o->LifeTime = 0xffffff;
				o->BlendMesh = -2;
				o->Scale = 0.6f;
				break;
			case MODEL_WARP2:
			case MODEL_WARP:
#ifdef CSK_ADD_MAP_ICECITY
			case MODEL_WARP5:
			case MODEL_WARP4:
#endif // CSK_ADD_MAP_ICECITY
				{
					Vector(0.0f,0.0f,0.0f,Light);
					o->BlendMesh = -2;
					o->LifeTime = 0xffffff;
					o->Scale = 1.3f + (float)(rand()%50)/100.f;
					o->Gravity = (float)(rand()%80)/10.f;
					o->Velocity = (float)(rand()%100)/1000.f + 0.01f;
				}
				break;
			case MODEL_GHOST:
				{
					o->Kind = 0;
					o->BlendMeshLight =1.0f;
					o->Velocity = 1.0f;
					o->LifeTime = 1000;
					o->Gravity = 1.0f;
					o->Distance = 1.0f;
					o->Position[2] += (float)(rand()%200) - 100.f;
					o->Position[1] += (float)(rand()%200) - 100.f;
					o->Position[0] += (float)(rand()%200) - 100.f;
					o->Angle[2] += (float)(rand()%360);
					Vector ( 0.f, (float)(rand()%8)*0.1f-5.0f, 0.f, o->Direction );
					o->Scale = o->Scale + (float)(rand()%5)/40.f;
					VectorCopy(o->Position, o->StartPosition);
					Vector(0.01f, 0.01f, 0.03f, o->Light);
				}
				break;
			case MODEL_TREE_ATTACK:
				o->Position[2] += 20.0f;
				o->LifeTime = 20;
				o->LightEnable = false;	
				o->Scale = 0.05f;
				break;
			case MODEL_BUTTERFLY01:		//. 반디불 박스 : 2004/11/04
				if(SceneFlag == CHARACTER_SCENE)
					o->HiddenMesh = -2;
				o->LifeTime = 250;
				o->Velocity    = 0.3f;
				o->LightEnable = false;
				o->Kind = rand()%2;		//. 운동방향
				
				Vector ( 0.f, -5.f*o->Owner->Scale, 0.f, o->Direction );

				if(o->SubType == 3)
					o->Scale = 0.9f;
				else
					o->Scale = 0.25f;
				break;
            case 9 :                         //  칼리마 맵에만 존재하는 돌덩이.
                o->LifeTime = 30;
                o->Scale    = (rand()%3)/10.f + 0.6f;
                o->Velocity = 10.f;
                o->PKKey    = 0;
                Vector ( -0.1f, -0.1f, -0.5f, o->Direction );
                break;
			case BITMAP_SKULL:	// 마검 - 방어 감소
				o->LifeTime = 1000;
				if ( 1 == o->SubType)
				{	// 나타나는 거 안보이기
					o->LifeTime -= 60;
				}
				else if ( 2 == o->SubType)
				{	// 임시
					o->LifeTime = 8;
					o->Position[2] += 150.f;
				}
                else if ( o->SubType==4 )
                {
                    o->LifeTime = 20;
                    o->Velocity = 2.f;
                    Vector ( 0.f, -10.f, 0.f, o->Direction );
                }
				break;
			case MODEL_MAGIC_CAPSULE2:	// 법사 - 방어막
				o->LifeTime = 20;
				o->BlendMesh = 0;
				o->BlendMeshLight = 1.0f;
				break;
			case MODEL_SPEAR:	// 꼬깔모양
				o->LifeTime = 10;
				break;
			case MODEL_SPEAR+1:	// 창술 준비동작
				o->Angle[0] = o->Angle[1] = o->Angle[2] = 0.0f;
				o->LifeTime = 5;
				break;

			case MODEL_SPEARSKILL:
				o->LifeTime = 20;
				o->Scale = 1.5f;
				o->Direction[0] = 5.0f * sinf( o->Angle[2]*Q_PI/180.0f);
				o->Direction[1] = -5.0f * cosf( o->Angle[2]*Q_PI/180.0f);
				//o->Direction[1] = 50.f;
				/*/
				memcpy( o->Matrix, g_pvEffectTemp, 3 * 4 * sizeof ( float));
				o->Matrix[0][3] = 0.0f;
				o->Matrix[1][3] = 0.0f;
				o->Matrix[2][3] = 0.0f;
				o->Angle[2] = 0.0f;
				/*/
				//*/
				break;
			case BITMAP_FIRE_CURSEDLICH:
				if (o->SubType == 0)
				{
					o->BlendMesh = -2;
   					o->LifeTime = 10;
					o->Scale = 0.7f;
					VectorCopy(Position, o->Position);
					Vector(0.f,0.f,0.f,o->Direction);
				}
				else if (o->SubType == 1)
				{
   					o->LifeTime = 50;
				}
#ifdef YDG_ADD_SKILL_FLAME_STRIKE
				else if (o->SubType == 2)
				{
   					o->LifeTime = 20;
				}
#endif	// YDG_ADD_SKILL_FLAME_STRIKE
#ifdef PBG_ADD_PKFIELD
				else if(o->SubType == 3)
				{
					o->LifeTime = 10;
				}
#endif //PBG_ADD_PKFIELD
#ifdef LDS_ADD_EG_4_MONSTER_WORLDBOSS_GAIONKALEIN
				else if(o->SubType == 12)
				{
					o->LifeTime = 20;
				}
#endif // LDS_ADD_EG_4_MONSTER_WORLDBOSS_GAIONKALEIN
				break;
#ifdef KJH_ADD_SKILL_SWELL_OF_MAGICPOWER
			case MODEL_SWELL_OF_MAGICPOWER:
				{
					if( o->SubType == 0 )
					{
						o->LifeTime = 45;
					}
				}break;
			case MODEL_ARROWSRE06:
				{
					if( o->SubType == 0 )
					{
						o->LifeTime = 30;
					}
					else if( o->SubType == 1 )
					{
						o->LifeTime = 40;	
					}
#ifdef PJH_ADD_PANDA_CHANGERING
					else if( o->SubType == 2 )
					{
						o->LifeTime = 30;	
						VectorCopy(Position, o->Position);
					}
#endif //PJH_ADD_PANDA_CHANGERING
				}break;
			case MODEL_SWELL_OF_MAGICPOWER_BUFF_EFF:
				{
					if( o->SubType == 0 )
					{
						// 라이프타임 무한대. (MoveEffect에서 999로 갱신)
						o->LifeTime = 999;
						
						// 시간계산.
						o->Timer = WorldTime;
					}
				}break;
#endif // KJH_ADD_SKILL_SWELL_OF_MAGICPOWER
			case MODEL_SUMMONER_WRISTRING_EFFECT:
				o->BlendMesh = -2;
   				o->LifeTime = 100;
				o->Scale = 0.7f;
				VectorCopy(Position, o->Position);
				Vector(0.f,0.f,0.f,o->Direction);
				break;
			case MODEL_SUMMONER_EQUIP_HEAD_SAHAMUTT:
				if (o->SubType == 0)
				{
   					o->LifeTime = 100;
					o->Scale = 0.8f;
					Vector(0.f,0.f,0.f,o->Direction);
					o->Alpha = 0.0f;
					OBJECT * pObject = o->Owner;
					o->Position[0] = pObject->Position[0] + cosf(WorldTime*0.003f) * 40.0f;
					o->Position[1] = pObject->Position[1] + sinf(WorldTime*0.003f) * 40.0f;
					o->Position[2] = pObject->Position[2] + (sinf(WorldTime*0.0010f)+2.0f) * 80.0f - 60.0f;
				}
				else if (o->SubType == 1)
				{
   					o->LifeTime = 100000;
				}
				break;
			case MODEL_SUMMONER_EQUIP_HEAD_NEIL:
				if (o->SubType == 0)
				{
   					o->LifeTime = 100;
					o->Scale = 0.8f;
					o->Alpha = 0.0f;
					Vector(0.f,0.f,0.f,o->Direction);
					OBJECT * pObject = o->Owner;
					o->Position[0] = pObject->Position[0] + cosf(WorldTime*0.003f) * 40.0f;
					o->Position[1] = pObject->Position[1] + sinf(WorldTime*0.003f) * 40.0f;
					o->Position[2] = pObject->Position[2] + (sinf(WorldTime*0.0010f)+2.0f) * 80.0f - 60.0f;
					CreateJoint( MODEL_SPEARSKILL, o->Position, o->Position, o->Angle, 15, o, 40.0f, -1, 0, 0);
				}
				else if (o->SubType == 1)
				{
   					o->LifeTime = 100000;
				}
				break;
#ifdef ASG_ADD_SUMMON_RARGLE
			case MODEL_SUMMONER_EQUIP_HEAD_LAGUL:	// 소환술사 주변을 도는 소환수 머리 이펙트.
				{
					o->LifeTime = 100;
					o->Scale = 0.8f;
					Vector(0.f,0.f,0.f,o->Direction);
					o->Alpha = 0.0f;
					OBJECT * pObject = o->Owner;
					o->Position[0] = pObject->Position[0] + cosf(WorldTime*0.003f) * 40.0f;
					o->Position[1] = pObject->Position[1] + sinf(WorldTime*0.003f) * 40.0f;
					o->Position[2] = pObject->Position[2] + (sinf(WorldTime*0.0010f)+2.0f) * 80.0f - 60.0f;
					CreateJoint(MODEL_SPEARSKILL, o->Position, o->Position, o->Angle, 17, o, 40.0f, -1, 0, 0);
				}
				break;
#endif	// ASG_ADD_SUMMON_RARGLE
			case MODEL_SUMMONER_CASTING_EFFECT1:
			case MODEL_SUMMONER_CASTING_EFFECT11:
			case MODEL_SUMMONER_CASTING_EFFECT111:
			case MODEL_SUMMONER_CASTING_EFFECT2:
			case MODEL_SUMMONER_CASTING_EFFECT22:
			case MODEL_SUMMONER_CASTING_EFFECT222:
				{
   					o->LifeTime = 40;
					if (o->SubType = 0)
						o->Scale = 1.0f;
					o->Alpha = 1.0f;
					Vector(0.f,0.f,0.f,o->Direction);
					o->BlendMesh = 0;
					o->BlendMeshLight = 0.0f;
				}
				break;
			case MODEL_SUMMONER_CASTING_EFFECT4:
   				o->LifeTime = 25;
				o->Scale = 1.0f;
				o->Alpha = 1.0f;
				Vector(0.f,0.f,0.f,o->Direction);
				o->BlendMesh = 0;
				o->BlendMeshLight = 0.0f;
				break;
			case MODEL_SUMMONER_SUMMON_SAHAMUTT:
   				o->LifeTime = 80;
				if (o->SubType == 2) 
					o->Scale = 0.7f;
				else if (o->SubType == 1) 
					o->Scale = 0.5f;
				else if (o->SubType == 0) 
					o->Scale = 0.35f;
				o->Alpha = 0.0f;
				o->Velocity = 0.5f;

				VectorCopy(Light, o->HeadTargetAngle);	// Light파라메터를 이용해 목표 위치를 전송한다;;
				Vector(1.0f, 1.0f, 1.0f, o->Light);
				break;
			case MODEL_SUMMONER_SUMMON_NEIL:
   				o->LifeTime = 80;
				o->Scale = 1.0f;
				o->Alpha = 0.0f;
				o->Velocity = 0.35f;
				o->Skill = 0;
				o->Position[2] += 10.0f;

				VectorCopy(Light, o->HeadTargetAngle);	// Light파라메터를 이용해 목표 위치를 전송한다;;
				Vector(1.0f, 1.0f, 1.0f, o->Light);
				break;
			case MODEL_SUMMONER_SUMMON_NEIL_NIFE1:
			case MODEL_SUMMONER_SUMMON_NEIL_NIFE2:
			case MODEL_SUMMONER_SUMMON_NEIL_NIFE3:
   				o->LifeTime = 50;
				o->Scale = 1.0f;
				o->Alpha = 1.0f;
				break;
			case MODEL_SUMMONER_SUMMON_NEIL_GROUND1:
			case MODEL_SUMMONER_SUMMON_NEIL_GROUND2:
			case MODEL_SUMMONER_SUMMON_NEIL_GROUND3:
   				o->LifeTime = 50;
				o->Scale = 1.0f;
				o->Alpha = 0.0f;
				break;
#ifdef ASG_ADD_SUMMON_RARGLE
			case MODEL_SUMMONER_SUMMON_LAGUL:
				if (o->SubType == 0)		// 펄루션 스킬 바닥 효과.
				{
					o->LifeTime = 160;
					o->Scale = 1.0f;
					o->Velocity = 0.5f;
					VectorCopy(Position, o->HeadTargetAngle);
				}
				else if (o->SubType == 1)	// 펄루션 스킬 라글 모델 돌아다니기 효과.
				{
					o->LifeTime = 160;
					o->Alpha = 0.0f;

					// 조인트의 위치에 라글 모델을 렌더하기 위해 이런 짓을 한다.
					// 이렇게 해주므로써 조인트의 o->Owner->Target로 라글 오브젝트 위치, 방향을 변경 가능.
					((JOINT*)o->Owner)->Target = o;
					o->Owner = NULL;

					// 라글 손톱 이펙트.
					for (int i = 48; i <= 53; ++i)
					{
						// 아래 o->Position, o->Angledms 의미 없는 값임.
						CreateJoint(BITMAP_JOINT_ENERGY, o->Position, o->Position, o->Angle, i, o, 10.f);
					}
				}
				break;
#endif	// ASG_ADD_SUMMON_RARGLE
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
            case BITMAP_ENERGY:
				if(o->SubType == 0)
				{
					o->LifeTime = 20;
					Vector(0.f,-60.f,0.f,o->Direction);
					o->Position[2] += 100.f;
				}
				break;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

			case MODEL_LIGHTNING_ORB:
				{
					if(o->SubType == 0)
					{
						o->LifeTime = 20;
						// 라이트닝 오브 속도 세팅
						Vector(0.f, -60.f, 0.f, o->Direction);
						// 라이트닝 오브 높이 세팅
						o->Position[2] += 100.f;
					}
					else if(o->SubType == 1)
					{
						o->LifeTime = 18;
					}
				}
				break;
			// ChainLighting
			case MODEL_CHAIN_LIGHTNING:
				{
					switch( o->SubType )
					{
						// 주인공 -> 몬스터 라이트닝 이펙트
					case 0:
					case 1:
					case 2:
						{
							o->LifeTime = 20;
						}
						break;
					}
				}
				break;
			// Drain Life
			case MODEL_ALICE_DRAIN_LIFE:
				{
					if( o->SubType == 0 )
					{
						o->LifeTime = 70;
					}		
				}
				break;
			case MODEL_ALICE_BUFFSKILL_EFFECT:
				{
					if(o->SubType == 0 || o->SubType == 1 || o->SubType == 2)
					{
						o->LifeTime = 34;
						o->Position[2] += 100;
						VectorCopy(Light, o->Light);
						o->AlphaEnable = true;
						o->Alpha = 0.f;
						o->BlendMeshLight = 0.f;
						o->Angle[2] = 0.f;
						o->Scale = 0.1f;	
					}
					// 3, 4 타입은 버프 전용 이펙트 다른사람은 절대 사용하지 마세요!!!!!
					else if(o->SubType == 3 || o->SubType == 4)
					{
						o->LifeTime = 100;	// 무한대

						if(o->SubType == 3)
						{
							o->Scale = 1.5f;
						}
						else if(o->SubType == 4)
						{
							o->Scale = 1.f;
						}		
						
					}
				}
				break;

			case MODEL_ALICE_BUFFSKILL_EFFECT2:
				{
					o->LifeTime = 35;
					o->Position[2] += 100;
					VectorCopy(Light, o->Light);
					o->AlphaEnable = true;
					o->Alpha = 0.f;
					o->BlendMeshLight = 0.f;
					o->Angle[2] = 0.f;
					o->Scale = 0.15f;	
				}
				break;

#ifdef YDG_ADD_SKILL_LIGHTNING_SHOCK
			case MODEL_LIGHTNING_SHOCK:
				{
					if(o->SubType == 0)
					{
						o->LifeTime = 20;
						// 라이트닝 오브 높이 세팅
						o->Position[2] += 280.f;
						o->Velocity = 0.0f;
						o->Gravity = 1.0f;
					}
					else if(o->SubType == 1)
					{
						o->LifeTime = 12;
						vec3_t vLight;
#ifdef CSK_RAKLION_BOSS
						// 땅에서 퍼지는 효과
						Vector(1.0f, 0.8f, 0.5f, vLight);
						CreateEffect(BITMAP_DAMAGE_01_MONO, o->Position, o->Angle, vLight, 1);
						Vector(1.0f, 0.0f, 0.0f, vLight);
						CreateEffect(BITMAP_DAMAGE_01_MONO, o->Position, o->Angle, vLight, 1);
#endif	// CSK_RAKLION_BOSS
						// magic_ground
						vec34_t Matrix;
						vec3_t vAngle, vDirection, vPosition;
						float fAngle;
						Vector(1.0f, 0.2f, 0.05f, vLight);
						for (int i = 0; i < 5; ++i)
						{
							Vector(0.f, 150.f, 0.f, vDirection );
							fAngle = o->Angle[2] + i * 72.f;
							Vector(0.f, 0.f, fAngle, vAngle);
							AngleMatrix(vAngle, Matrix);
							VectorRotate(vDirection, Matrix, vPosition);
							VectorAdd(vPosition, o->Position, vPosition);
							
							CreateEffect ( BITMAP_MAGIC, vPosition, o->Angle, vLight, 12 );
						}

						// 바닥 쪼개짐
						Vector(1.0f, 0.4f, 0.2f, vLight);
						VectorCopy(o->Position, vPosition);
						vPosition[2] = RequestTerrainHeight ( vPosition[0], vPosition[1] ) + 10;

						for (int i = 0; i < 3; ++i)
							CreateEffect(MODEL_KNIGHT_PLANCRACK_A, vPosition, o->Angle, vLight, 1, o, 0, 0, 0, 0, rand()%4*0.1f+1.0f);
					}
					else if(o->SubType == 2)
					{
						o->LifeTime = 15;
						// 바닥 쪼개짐
						vec3_t vLight;
						vec3_t vPosition;
						Vector(1.0f, 0.4f, 0.2f, vLight);
						VectorCopy(o->Position, vPosition);
						vPosition[2] = RequestTerrainHeight ( vPosition[0], vPosition[1] ) + 10;

						for (int i = 0; i < 3; ++i)
							CreateEffect(MODEL_KNIGHT_PLANCRACK_A, o->Position, o->Angle, vLight, 1, o, 0, 0, 0, 0, rand()%4*0.1f+0.5f);
					}
				}
				break;
#endif	// YDG_ADD_SKILL_LIGHTNING_SHOCK

            case BITMAP_SPARK+1:
				o->LifeTime = 10;
				//Vector(0.f,0.f,60.f,o->Direction);
				//o->Position[2] += 100.f;
				break;
			case BITMAP_BOSS_LASER:
			case BITMAP_BOSS_LASER+1:
			case BITMAP_BOSS_LASER+2:
				o->LifeTime = 20;
				switch(Type)
				{
				case BITMAP_BOSS_LASER:
					Vector(0.5f,0.7f,1.f,o->Light);
					Vector(0.f,-50.f,0.f,p1);
					o->Scale = 16.f;

					if(SubType == 1)
					{
						Vector(0.f,-50.f,0.f,p1);
						o->Scale = 3.0f;
						Vector(1.0f,1.0f,1.0f,o->Light);
					}
					else if(SubType == 2)
					{
						o->LifeTime = 35;
						Vector(0.f,-50.f,0.f,p1);
						o->Scale = 2.5f;
						Vector(1.0f,1.0f,1.0f,o->Light);
					}
					break;
				case BITMAP_BOSS_LASER+1:
					Vector(1.f,0.4f,0.2f,o->Light);
					Vector(0.f,-50.f,0.f,p1);
					o->Scale = 16.f;
					break;
				case BITMAP_BOSS_LASER+2:
					Vector(1.f,0.4f,0.2f,o->Light);
					Vector(0.f,-15.f,0.f,p1);
					o->Scale = 5.f;
					break;
				}
				//o->Scale = 10.f;
     			//Vector(0.f,-30.f,0.f,p1);
				AngleMatrix(o->Angle,Matrix);
				VectorRotate(p1,Matrix,o->Direction);
				if(o->Owner == &Hero->Object)
				{
					VectorCopy(o->Position,Position);
					Vector(0.f,-150.f,0.f,p1);
					AngleMatrix(o->Angle,Matrix);
					vec3_t Direction;
					VectorRotate(p1,Matrix,Direction);
					int Number = 4;
					for(int j=0;j<Number;j++)
					{
						VectorAdd(Position,Direction,Position);
           				AttackCharacterRange(o->Skill,Position,150.f,o->Weapon,o->PKKey);//클라이언트마법처리
					}
				}
				break;
			case BITMAP_LIGHTNING+1:
				if(o->SubType == 0)
				{
					o->LifeTime = 10;
					o->Scale = 1.5f;
				}
				else if(o->SubType == 1)
				{
					o->LifeTime = 50;
					o->Alpha = 0.01f;
				}
				break;
			case BITMAP_LIGHT:
                if ( o->SubType==0 )
                {
					//Vector( 0.5f, 0.5f, 1.0f, o->Light);
					Vector( 0.3f, 0.3f, 0.3f, o->Light);
					float fAngle = 70.0f * Q_PI / 180.0f;
					float fAngle2 = 30.0f;//( float)( rand() % 360);
					//float fSpeed = ( float)( 18 + rand() % 10);
					float fSpeed = ( float)( 9 + rand() % 5)*0.5f;
					o->Direction[0] = fSpeed * cosf( fAngle) * sinf( fAngle2 * Q_PI / 180.0f);
					o->Direction[1] = fSpeed * cosf( fAngle) * cosf( fAngle2 * Q_PI / 180.0f);
					o->Direction[2] = fSpeed * sinf( fAngle);
					o->Scale = 3.f;
					//o->LifeTime = 100;
					o->LifeTime = 400;
                }
                else if ( o->SubType==1 || o->SubType==2 )
                {
                    o->LifeTime = 1000;
                    o->Velocity = 0.f;
                }
                else if ( o->SubType==3 )
                {
                    o->LifeTime = 30;
                    o->Velocity = 0.f;
                }
				break;

//------------------------------------------------------------------------------------------------------------

            case BITMAP_FIRE:
                o->LifeTime = 1000;
                break;

//------------------------------------------------------------------------------------------------------------

			case BITMAP_FIRE+1:
				o->LifeTime = 10;
				AngleMatrix(o->Angle,Matrix);
				Vector(0.f,-60.f,0.f,p1);
				VectorRotate(p1,Matrix,p2);
				VectorAdd(o->Position,p2,o->Position)
				o->Position[2] += 130.f;
				break;

//------------------------------------------------------------------------------------------------------------

			case BITMAP_FLAME:
                if ( o->SubType==0 )
                {
				    o->LifeTime = 40;
				    o->Weapon = CharacterMachine->PacketSerial;
                }
                else if ( o->SubType==1 || o->SubType==2 )
                {
				    o->LifeTime = 10;
                    Vector ( 0.f, 0.f, 0.f, o->Angle );
                }
				else if( o->SubType==3)
				{
					o->LifeTime = 15;
				}
				else if( o->SubType==4)
				{
					o->Scale = 0.01f;
					o->LifeTime = 10;
					Vector ( 0.f, 0.f, 0.f, o->Angle );
				}
				else if(o->SubType == 5)
				{
					o->LifeTime = 20;
				}
#ifdef LDS_ADD_EMPIRE_GUARDIAN
				else if(o->SubType == 6)
				{
					o->LifeTime = 40;
				}
#endif // LDS_ADD_EMPIRE_GUARDIAN
				break;

//------------------------------------------------------------------------------------------------------------

#ifdef CSK_RAKLION_BOSS

			case MODEL_RAKLION_BOSS_CRACKEFFECT:
				if(o->SubType == 0)
				{
					o->LifeTime = 40;
					o->Scale = Scale + 1.f;
					o->Position[2] += 30.f;
					o->Angle[2] = rand()%360;
				}
				break;

//------------------------------------------------------------------------------------------------------------	

			case MODEL_RAKLION_BOSS_MAGIC:
				if(o->SubType == 0)
				{
					o->LifeTime = 35;
					o->Scale = Scale;
				}
				break;

//------------------------------------------------------------------------------------------------------------	

			case BITMAP_FIRE_HIK2_MONO:
				if(o->SubType == 0)
				{
					o->LifeTime = 60;
				}
				else if(o->SubType == 1)
				{
					o->LifeTime = 20;
					Vector ( 0.f, 0.f, 0.f, o->Angle );
				}
				break;

//------------------------------------------------------------------------------------------------------------		

			case BITMAP_CLOUD:
				if(o->SubType == 0)
				{
					o->LifeTime = 60;
					o->Scale = Scale;
					o->Angle[2] = (float)(rand()%360);
				}
				break;
				
#endif // CSK_RAKLION_BOSS

//------------------------------------------------------------------------------------------------------------

			case BITMAP_FIRE_RED:
				o->LifeTime = 40;
				break;

//------------------------------------------------------------------------------------------------------------
#ifdef PJH_SEASON4_SPRITE_NEW_SKILL_RECOVER
			case BITMAP_IMPACT:
				{
					o->LifeTime = 80;
					o->Scale = 0.f;
					o->BlendMesh = -2;
				}
				break;
#endif //PJH_SEASON4_SPRITE_NEW_SKILL_RECOVER
            case BITMAP_MAGIC:
                o->LifeTime = 20;
                o->Scale = 0.5f;
                if ( o->SubType==0 )
                {
					o->LifeTime = 15;
                }
				else if ( o->SubType == 8 )
				{
                    o->LifeTime = 30;
                    o->Scale = 1.f;
				}
				else if ( o->SubType == 9 )
				{
                    o->LifeTime = 40;
                    o->Scale = 2.4f;
					Vector(0, 0, 0, o->HeadAngle);
				}
				else if ( o->SubType == 10 )
				{
                    o->LifeTime = 44;
                    o->Scale = 12.f;
					o->Alpha = 0.0f;
				}
				else if( o->SubType == 11 )
				{
					o->LifeTime = 24;
					o->Scale = 0.8f;
					Vector(0, 0, 0, o->HeadAngle);
				}
#ifdef YDG_ADD_SKILL_LIGHTNING_SHOCK
				else if ( o->SubType == 12 )
				{
                    o->LifeTime = 20;
                    o->Scale = Scale * 0.1f;
				}
#endif	// YDG_ADD_SKILL_LIGHTNING_SHOCK
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
				else if(o->SubType==13 || o->SubType==14)
				{
					o->LifeTime = 30;
                    o->Scale = 1.0f;
					VectorCopy(Light, o->Light);
				}
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
                break;

//------------------------------------------------------------------------------------------------------------

			case BITMAP_MAGIC+1:
			case BITMAP_MAGIC+2:
				o->LifeTime = 20;
                if ( o->SubType==4 || o->SubType==10 ) 
                {
                    o->LifeTime = 40;
                    o->Scale = ((rand()%50)+50)/100.f*4.f;
                }
#ifdef ENABLE_POTION_EFFECT
				else if(o->SubType == 5) 
				{
					if((o->State&STATE_HP_RECOVERY) != STATE_HP_RECOVERY)
						o->State = STATE_HP_RECOVERY;
				}
#endif	// ENABLE_POTION_EFFECT
				else if (o->SubType == 6)
				{
                    o->LifeTime = 60;
                    o->Scale = ((rand()%50)+50)/100.f*4.f;
					VectorCopy ( Position, o->StartPosition );
				}
				else if (o->SubType == 7)
				{
                    o->LifeTime = 40;
					o->Angle[2] = rand()%360;
				}
                else if ( o->SubType==8 )
                {
                    o->LifeTime = 20;
                }
                else if ( o->SubType==9 )
                {
                    o->LifeTime = 10;
                    o->Scale    = 0.1f;
                }
				else if(o->SubType == 11)
				{
					o->LifeTime = 20;
				}
				else if(o->SubType == 12)
				{
					o->LifeTime = 20;
				}
#ifdef CSK_RAKLION_BOSS
				else if(o->SubType == 13)
				{
					o->LifeTime = 40;
					o->Scale = Scale;
				}
#endif // CSK_RAKLION_BOSS
				break;

#ifdef ASG_ADD_INFLUENCE_GROUND_EFFECT
			case BITMAP_OUR_INFLUENCE_GROUND:
			case BITMAP_ENEMY_INFLUENCE_GROUND:
                if (o->SubType == 0)
                {
					o->LifeTime = 50;
					o->Scale = 0.6f;
					o->Alpha = 1.0f;
					o->AlphaTarget = 0.75f;	// 다른 알파값으로 쓰임.
                }
				break;
#endif	// ASG_ADD_INFLUENCE_GROUND_EFFECT

//------------------------------------------------------------------------------------------------------------

			case BITMAP_MAGIC_ZIN:
				switch (o->SubType)
                {
                case 0:	o->LifeTime = 50;	break;
				case 1:
					o->LifeTime = 40;
					o->Alpha = 0.f;
					break;
				case 2:	o->LifeTime = 30;	break;
				}
				break;

//------------------------------------------------------------------------------------------------------------
				
			case BITMAP_SHINY+6:
				switch (o->SubType)	// SubType 1,2는 버프(위크니스, 이너베이션) 이펙트용
                {
				case 0:	o->LifeTime = 24;	break;
				case 1:
				case 2:	o->LifeTime = 100;	break;
#ifdef PJH_SEASON4_DARK_NEW_SKILL_CAOTIC
				case 3:
					o->LifeTime = 24;
					o->RenderType = RENDER_TYPE_ALPHA_BLEND_MINUS;
					break;
#endif //PJH_SEASON4_DARK_NEW_SKILL_CAOTIC
				}
				break;

			case BITMAP_PIN_LIGHT:
				switch (o->SubType)	// SubType 1,2는 버프(위크니스, 이너베이션) 이펙트용
                {
#ifdef PJH_SEASON4_FIX_MULTI_SHOT
				case 3:
#endif //PJH_SEASON4_FIX_MULTI_SHOT
				case 0:	o->LifeTime = 40;	break;
				case 1:
				case 2:	o->LifeTime = 100;	break;
#ifdef LDK_ADD_CS7_UNICORN_PET
				case 4:
					{
						o->Alpha = 1.0f;
						o->LifeTime = 30;
						o->Scale += (float)(rand()%5)/10.0f;
						o->Angle[1] = rand()%360;
					}break;
#endif //LDK_ADD_CS7_UNICORN_PET
				}
				break;

#ifdef ASG_ADD_SKILL_BERSERKER
			case BITMAP_ORORA:		// 플레이어 손목 주변을 돌며 커지면서 사라지고 재생성하는 이펙트. 색상만 변경하여 생성 가능.
				switch (o->SubType)
				{
				case 0:
				case 1:	o->LifeTime = 100;	break;
				case 2:
				case 3:	o->LifeTime = 25;	break;
				}
				CreateParticle(o->Type, o->Position, o->Angle, o->Light, o->SubType, 1.0f, o->Owner);
				break;

			case BITMAP_SPARK+2:	// 플레이어 손목에서 간헐적으로 생성되는 이펙트. SubType 2, 3만 가능.
				o->LifeTime = 100;
				break;
				
			case BITMAP_LIGHT_MARKS:// 플레이어 몸에서 빛나는 이펙트.
				o->LifeTime = 65;
				break;
#endif	// ASG_ADD_SKILL_BERSERKER
			//////////////////////////////////////////////////////////////////////////

            case BITMAP_GATHERING:
                o->LifeTime = 10;

                switch( o->SubType )
                {
                case 0:
                    Vector(0.f,-100.f,0.f,p1);
                    AngleMatrix(o->Angle,Matrix);
				    VectorRotate(p1,Matrix,p2);
				    VectorAdd(o->Position,p2,o->Position);

                    o->Position[2] += 150.f;
                    break;
                case 1:
                case 2:
                    VectorCopy ( Position, o->StartPosition );
                    o->LifeTime = 20;
                    break;
#ifdef CSK_ADD_MAP_ICECITY	
				case 3:
					o->LifeTime = 10;
					Vector(-10.f,10.f,0.f,p1);
                    AngleMatrix(o->Angle,Matrix);
					VectorRotate(p1,Matrix,p2);
					VectorAdd(o->Position,p2,o->Position);
					break;
#endif // CSK_ADD_MAP_ICECITY
                }
                break;

			//////////////////////////////////////////////////////////////////////////

#ifdef YDG_ADD_SKILL_GIGANTIC_STORM
			case BITMAP_JOINT_THUNDER:
				{
					o->LifeTime = 20;

					VectorCopy(o->Position, o->StartPosition);
					o->StartPosition[2] += 800.0f;	// 번개 시작 높이
				}
				break;
#endif	// YDG_ADD_SKILL_GIGANTIC_STORM

			case MODEL_STAFF+8:
				o->LifeTime  = 30;
				o->BlendMesh = -2;
				o->Scale     = 1.f;
				o->Position[2] += 280.f;
				o->Angle[0]    += 20.f;
				Vector(0.f,-80.f,-10.f,o->Direction);
				break;
            case MODEL_WAVE:
				o->BlendMesh    = 0;
                o->BlendMeshLight = 1.5f;
				o->Scale        = 0.5f;
				o->LifeTime     = 15;
                o->Position[2] -= 15.f;

                Vector(1.f,1.f,1.f,o->Light);
                break;
            case MODEL_TAIL:
                o->BlendMesh = -2;
				o->LifeTime  = 6;
                o->Scale     = 1.f;
                o->Gravity   = 80.f;
				Vector(0.5f,0.5f,0.5f,o->Light);
                Vector(0.f,0.f,45.f,o->Angle);
                break;
			case MODEL_SKILL_BLAST:
				o->LifeTime  = 30;
				o->BlendMesh = 0;
				o->Scale     = (float)(rand()%8+10)*0.1f;
				o->Position[0] += rand()%100+200;
				o->Position[1] += rand()%100-50;
				o->Position[2] += rand()%500+300;
				Vector(0.f,0.f,-50.f-rand()%50,o->Direction);
				Vector(0.f,20.f,0.f,o->Angle);
				VectorCopy(o->Position,o->EyeLeft);
				CreateJoint(BITMAP_JOINT_ENERGY,o->Position,o->Position,o->Angle,5,o,100.f);
				o->Weapon = CharacterMachine->PacketSerial;
				break;
			case MODEL_WAVE_FORCE:
				o->BlendMesh = -2;
				o->Scale     = 0.9f;
                o->Velocity  = 0.5f;
				o->LifeTime  = 12;
				o->Skill = 0;
				o->PKKey = -1;
                o->Scale = PKKey/100.f;
				o->BlendMeshLight = 0.1f;
				break;
			case MODEL_SKILL_INFERNO:
				o->BlendMesh = -2;
				o->Scale     = 0.9f;
                o->Velocity  = 0.5f;
				switch ( o->SubType)
				{
				case 0: //  마법사 인페르노 마법.
					Vector(0.8f,0.8f,0.8f,o->Light);
					o->LifeTime  = 15;
					break;
				case 1: //  저주받은 왕의 색이 다른 인페르노 마법.
					Vector(1.0f,.5f,.2f,o->Light);
					o->LifeTime  = 35;
					break;
                case 2: //  아래 파형을 나타내는 효과.
					o->LifeTime  = 12;
                    o->HiddenMesh = SkillIndex;
					o->Skill = 0;
					o->PKKey = -1;
					o->Distance = PKKey;
                    o->Scale = PKKey/100.f;
					o->BlendMeshLight = 0.1f;
                    break;
				case 8:
					o->LifeTime  = 12;
                    o->HiddenMesh = SkillIndex;
					o->Skill = 0;
					o->PKKey = -1;
					o->Distance = PKKey;
                    o->Scale = PKKey/100.f;
					o->BlendMeshLight = 0.1f;
					o->Velocity *= 4;
					break;
                case 3: //  가로로 끌어당기는 효과. ( 기사 스킬 ).
					Vector(0.8f,0.8f,0.8f,o->Light);
					o->LifeTime  = 4;
                    o->HiddenMesh = 1;
                    o->Scale    = 0.02f;
                    o->Velocity = 1.f;
                    Vector(90.f,0.f,0.f,o->Angle);
                    break;
                case 4: //  작은 원만을 하늘로 올라가게 한다.
					Vector(0.8f,0.8f,0.8f,o->Light);
					o->LifeTime  = 35;
                    o->HiddenMesh = 1;
                    o->Scale = 0.1f;
                    o->Gravity = 0.f;
                    break;
                case 5: //  회전, 승천하는 인페르노.
					Vector(0.8f,0.8f,0.8f,o->Light);
					o->LifeTime  = 15;
                    break;
                case 6: //  아래 파형을 나타내는 효과.
					o->LifeTime  = 5;
                    o->HiddenMesh = SkillIndex;
					o->Skill = 0;
					o->PKKey = -1;
					o->Distance = PKKey;
                    o->Scale = PKKey/100.f;
                    o->Velocity = 0.1f;
					o->BlendMeshLight = 0.1f;
                    break;
				case 9:
					{
						o->LifeTime  = 13;
						Vector(0.1f,1.0f,0.2f,o->Light);
						o->Scale = 0.45f;
						o->BlendMeshLight = 0.1f;
						o->Gravity = 2.0f;
					}
					break;
				case 10:
					{
						o->LifeTime  = 12;
						o->HiddenMesh = SkillIndex;
						o->Skill = 0;
						o->PKKey = -1;
						o->Distance = PKKey;
						o->Scale = PKKey/100.f;
						o->BlendMeshLight = 0.1f;
					}
					break;
				}
				if(o->Owner == &Hero->Object && o->SubType<2)
				{
					o->Weapon = CharacterMachine->PacketSerial;

     				AttackCharacterRange(o->Skill,o->Position,400.f,o->Weapon,o->PKKey);//클라이언트마법처리
				}
				break;
			case MODEL_BOSS_ATTACK:
				o->LifeTime     = 50;
				o->Scale        = 0.8f;
                o->Velocity     = 1.f;
				o->Angle[0]     = 0.f;
				o->BlendMesh    = 0;
       			o->Direction[1] = -50.f;
				o->Gravity      = -10.f;
				break;
			case MODEL_MAGIC_CIRCLE1:
				o->LifeTime = 30;
				o->Scale = 0.7f;
				o->BlendMesh = -2;
                if(o->SubType==2)
                {
                    o->LifeTime = 15;
                    o->Velocity = 0.3f;
					o->Scale = 0.7f;
				}
                else if(o->SubType==1)
                {
                    o->LifeTime = 20;
                    o->Velocity = 0.1f;
                    o->HiddenMesh = 0;
                }
                else
                {
				    o->Velocity = 0.1f;
                }
				break;
			case MODEL_BIG_METEO1:
			case MODEL_BIG_METEO2:
			case MODEL_BIG_METEO3:
				o->LifeTime = 100;
				o->Scale = (float)(rand()%10+4)*0.1f;
           		Vector(0.f,-15.f/o->Scale,-30.f/o->Scale,o->Direction);
				o->SubType = 1;
				break;
			case MODEL_PROTECT:
				o->LifeTime = 10000;
				o->Velocity = 0.3f;
    			o->BlendMesh = 0;
				break;

            case MODEL_PIERCING:
                o->LifeTime  = 100;
                o->BlendMesh = 0;
                o->BlendMeshLight = 1.f;
                o->Gravity = 0;

                switch ( SubType )
                {
                case 0:
                    {
                        o->Scale = 12.f;
                        VectorCopy ( o->Owner->Position, o->StartPosition );
                    }
                    break;

                case 1:
                    o->HiddenMesh = 0;
                    o->Scale = 24.f;
                    break;

				case 2:
                    o->HiddenMesh	= 0;
                    o->Scale		= 12.f;
					o->LifeTime 	= 10;
					break;

#ifdef CRYINGWOLF_2NDMVP
				case 3:
					{
						o->Scale = 10.f;
						Vector( 0.9f, 0.4f, 0.6f, o->Light);
                        VectorCopy ( o->Owner->Position, o->StartPosition );
					}
					break;
#endif // CRYINGWOLF_2NDMVP
                }

                CreateJoint(BITMAP_FLARE+1,o->Position,o->Position,o->Angle,0, o, o->Scale, 30, SubType );
                CreateJoint(BITMAP_FLARE+1,o->Position,o->Position,o->Angle,1, o, o->Scale, 30, SubType );
                CreateJoint(BITMAP_FLARE+1,o->Position,o->Position,o->Angle,2, o, o->Scale, 30, SubType );
                CreateJoint(BITMAP_FLARE+1,o->Position,o->Position,o->Angle,3, o, o->Scale, 30, SubType );
				PlayBuffer( SOUND_FLASH);
                o->Scale = 1.f;
                break;

            case MODEL_PIERCING+1:
				o->Velocity = 1.f;
				o->LifeTime = 30;
				AngleMatrix(o->Angle,Matrix);
				Vector(-10.f,-60.f,135.f,p1);
				VectorRotate(p1,Matrix,p2);
				VectorAdd(o->Position,p2,o->Position);

                o->Scale        = 0.8f;
     			o->Direction[1] = -70.f;

                CreateEffect(MODEL_PIERCING,o->Position,o->Angle,o->Light,SubType,o);
                break;

			case MODEL_ARROW_BEST_CROSSBOW :
				o->LifeTime  = 30;
				o->BlendMesh = -2;
				o->Scale     = 1.f;
				o->Position[2] += 130.f;
				Vector(0.f,-70.f,0.f,o->Direction);
				VectorCopy(o->Position,o->EyeLeft);
				CreateJoint(BITMAP_JOINT_ENERGY,o->Position,o->Position,o->Angle,5,o,100.f);

                if(o->SubType!=0)
                {
                    CreateEffect(MODEL_PIERCING,o->Position,o->Angle,o->Light,0,o);
                    o->AttackPoint[0] = 0;
                    o->Kind = 1;
                }

				o->Weapon = CharacterMachine->PacketSerial;
				break;

            case MODEL_ARROW_DOUBLE:
				o->LifeTime  = 30;
				o->BlendMesh = -2;
				o->Scale     = 1.f;
				o->Position[2] += 130.f;
				Vector(0.f,-70.f,0.f,o->Direction);
				VectorCopy(o->Position,o->EyeLeft);
				CreateJoint(BITMAP_JOINT_ENERGY,o->Position,o->Position,o->Angle,5,o,100.f);

                if(o->SubType!=0)
                {
                    CreateEffect(MODEL_PIERCING,o->Position,o->Angle,o->Light,0,o);
                    o->AttackPoint[0] = 0;
                    o->Kind = 1;
                }

				o->Weapon = CharacterMachine->PacketSerial;
				break;

            case MODEL_ARROW_HOLY:
				o->LifeTime  = 30;
				o->BlendMesh = -2;
				o->Scale     = 1.f;
				o->Position[2] += 130.f;

				Vector(0.f,-60.f,0.f,o->Direction);
                AngleMatrix(o->Angle,Matrix);
				Vector(-10.f,-100.f,15.f,p1);
				VectorRotate(p1,Matrix,p2);
                VectorCopy(o->Position,o->StartPosition);
				VectorAdd(o->StartPosition,p2,o->StartPosition);

                Vector( 0.f, 0.f, o->Angle[2], Angle );

#ifdef CRYINGWOLF_2NDMVP
				if(o->SubType == 1)
				{
#ifdef _VS2008PORTING
					for ( int i=0; i<3; ++i )
#else // _VS2008PORTING
					for ( i=0; i<3; ++i )
#endif // _VS2008PORTING
					{
						if ( i==1 )
							CreateJoint(BITMAP_FLARE,o->StartPosition,o->StartPosition,Angle,25,o,50.f,-1,1);
						else
							CreateJoint(BITMAP_FLARE,o->StartPosition,o->StartPosition,Angle,25,o,50.f);
					}
//					CheckTargetRange(o);
				}
				else 
#endif // CRYINGWOLF_2NDMVP
				{
#ifdef _VS2008PORTING
					for ( int i=0; i<4; ++i )
#else // _VS2008PORTING
					for ( i=0; i<4; ++i )
#endif // _VS2008PORTING
					{
						if ( i==1 )
							CreateJoint(BITMAP_FLARE,o->StartPosition,o->StartPosition,Angle,11,o,50.f,-1,1);
						else
							CreateJoint(BITMAP_FLARE,o->StartPosition,o->StartPosition,Angle,11,o,50.f);
					}
				}

				o->Weapon = CharacterMachine->PacketSerial;
                break;
#ifdef PJH_SEASON4_SPRITE_NEW_SKILL_MULTI_SHOT
			case MODEL_MULTI_SHOT1:
			case MODEL_MULTI_SHOT2:
			case MODEL_MULTI_SHOT3:
#endif //PJH_SEASON4_SPRITE_NEW_SKILL_MULTI_SHOT
			case MODEL_ARROW:
			case MODEL_ARROW_STEEL:
			case MODEL_ARROW_THUNDER:
			case MODEL_ARROW_LASER:
			case MODEL_ARROW_V:
			case MODEL_ARROW_SAW:
			case MODEL_ARROW_NATURE:
			case MODEL_ARROW_WING:
            case MODEL_LACEARROW:
			case MODEL_DARK_SCREAM:
			case MODEL_DARK_SCREAM_FIRE:
			case MODEL_ARROW_SPARK:
			case MODEL_ARROW_RING:	
			case MODEL_ARROW_TANKER:
#ifdef ADD_SOCKET_ITEM
			case MODEL_ARROW_DARKSTINGER:
#endif // ADD_SOCKET_ITEM
#ifdef LDK_ADD_GAMBLERS_WEAPONS
			case MODEL_ARROW_GAMBLE:
#endif //LDK_ADD_GAMBLERS_WEAPONS
				if(Type==MODEL_ARROW)
     				o->BlendMesh = 1;
                else if(Type==MODEL_ARROW_NATURE)
     				o->BlendMesh = -2;
                else if(Type==MODEL_ARROW_THUNDER || Type==MODEL_ARROW_LASER || Type==MODEL_ARROW_V || Type==MODEL_ARROW_WING )
     				o->BlendMesh = 0;

				o->Velocity = 1.f;
				o->LifeTime = 30;
				AngleMatrix(o->Angle,Matrix);
				Vector(-10.f,-60.f,135.f,p1);
				VectorRotate(p1,Matrix,p2);
				VectorAdd(o->Position,p2,o->Position);
				if(Type==MODEL_ARROW_WING)
				{
					o->Scale        = 1.8f;
        			o->Direction[1] = -50.f;
					o->Gravity      = -10.f;
				}
				else if(Type == MODEL_ARROW_TANKER)
				{
					o->Angle[0] -= 18.0f;
					o->Scale = 1.0f;
					o->Direction[1] = -42.0f;
					o->Direction[2] = 4.0f;
				}
#ifdef PJH_SEASON4_SPRITE_NEW_SKILL_MULTI_SHOT
				else 
				if(Type==MODEL_MULTI_SHOT1)
				{
					o->LifeTime = 16;
					o->BlendMesh = -2;
					o->Scale    = 0.f;
				}
				else 
				if(Type==MODEL_MULTI_SHOT2)
				{
					o->LifeTime = 13;
					o->BlendMesh = -2;
					o->Scale    = 0.f;
				}
				else 
				if(Type==MODEL_MULTI_SHOT3)
				{
					o->LifeTime = 12;
					o->BlendMesh = -2;
					o->Scale    = 0.f;
				}
#endif //PJH_SEASON4_SPRITE_NEW_SKILL_MULTI_SHOT
				else if(Type==MODEL_DARK_SCREAM||Type==MODEL_DARK_SCREAM_FIRE)
				{
					o->LifeTime = 19;
					o->Scale        = 2.3f;
					o->Direction[1] = -35.f;
//					o->Direction[1] = -5.f;
					if(Type==MODEL_DARK_SCREAM)
					{
						o->Scale        = 0.9f;
						o->Position[2] = RequestTerrainHeight(o->Position[0],o->Position[1]) + 3.f;
						vec3_t Light;
						Vector(0.0f,1.0f,0.0f,Light);	//보라색
#ifdef LDS_ADD_EFFECT_FIRESCREAM_FOR_MONSTER
						if( SubType == 0 )
						{
							CreateJoint ( BITMAP_JOINT_FORCE, o->Position, o->Position, o->Angle, 7, o, 150.f, 40 );
						}
						else if( SubType == 1 ) 
						{
							CreateJoint ( BITMAP_JOINT_FORCE, o->Position, o->Position, o->Angle, 20, o, 400.f, 40 );
						}
#else // LDS_ADD_EFFECT_FIRESCREAM_FOR_MONSTER
						CreateJoint ( BITMAP_JOINT_FORCE, o->Position, o->Position, o->Angle, 7, o, 150.f, 40 );
#endif // LDS_ADD_EFFECT_FIRESCREAM_FOR_MONSTER
						o->Position[2]+=20.f;
						vec3_t ap,P,dp;
						VectorCopy(o->Position,ap);
						Vector(0.f,-20.f,0.f,P);
						AngleMatrix(o->Angle,o->Matrix);
						VectorRotate(P,o->Matrix,dp);
						VectorAdd(dp,o->Position,o->Position);
						CreateParticle(BITMAP_BLUE_BLUR,o->Position, o->Angle, Light,1,1.f);
					}
				}
				else if(Type==MODEL_ARROW_SPARK)
				{
					o->Scale        = 1.0f;
					o->Direction[1] = -70.f;
					CreateJoint ( BITMAP_FLARE+1, o->Position, o->Position, o->Angle, 14, o, 50.f, 40 );
				}
#ifdef ADD_SOCKET_ITEM
				else if( Type == MODEL_ARROW_DARKSTINGER )
				{
					o->Scale        = 0.8f;
     				o->Direction[1] = -70.f;
					o->LifeTime		= 30;
				}
#endif //ADD_SOCKET_ITEM
#ifdef LDK_ADD_GAMBLERS_WEAPONS
				else if( Type == MODEL_ARROW_GAMBLE )
				{
					o->Scale        = 0.8f;
     				o->Direction[1] = -70.f;
					o->LifeTime		= 30;
				}
#endif //LDK_ADD_GAMBLERS_WEAPONS
				else
				{
					o->Scale        = 0.8f;
     				o->Direction[1] = -70.f;
				}

                if(o->SubType==2)
                {
                    CreateEffect(MODEL_PIERCING,o->Position,o->Angle,o->Light,0,o);
                    o->AttackPoint[0] = 0;
                    o->Kind = 1;
                }
#ifdef CRYINGWOLF_2NDMVP
				//. Create Effect
				if(Type == MODEL_ARROW_NATURE && o->SubType == 1)
				{	//. 녹색 띠 생성
					CreateJoint ( BITMAP_FLARE+1, o->Position, o->Position, o->Angle, 13, o, 20.f, 40 );
//					CreateJoint ( BITMAP_FLARE+1, o->Position, o->Position, o->Angle, 6, o, 20.f, 40 );
//					CheckTargetRange(o);
				}
#endif // CRYINGWOLF_2NDMVP

                //  땅으로 떨어지는 화살.
                if ( Type==MODEL_ARROW && ( o->SubType==3 || o->SubType==4 ) )
                {
                    o->LifeTime = 40;
                    o->Scale    = 1.5f;
                    o->Gravity  = (rand()%100+50)/15.f;
                    VectorCopy ( Hero->Object.Position, o->StartPosition );
                    if ( o->SubType==3 )
                    {
         				o->Direction[1] = -(rand()%30+50.f);
                        o->Angle[0] = -(rand()%20+45.f);
                    }
                    else
                    {
         				o->Direction[1] = -(rand()%15+55.f);
                        o->Angle[0] = -10.f;
                    }
                }
                else if ( Type==MODEL_LACEARROW )
                {
     				o->BlendMesh = 1;
                    o->Angle[1] = (float)( rand()%360 );
      			    CreateJoint ( BITMAP_FLARE+1, o->Position, o->Position, o->Angle, 12, o, 40.f, 80 );
                }
				else if(Type == MODEL_ARROW_RING)
				{
					o->BlendMesh = 0;
					o->Scale        = 1.0f;
					o->Direction[1] = -70.f;
                    o->Angle[1] = (float)( rand()%360 );
      			    CreateJoint ( BITMAP_FLARE+1, o->Position, o->Position, o->Angle, 15, o, 50.f, 40 );
				}
				o->Weapon = CharacterMachine->PacketSerial;
				break;
			case MODEL_CURSEDTEMPLE_HOLYITEM:
				{
					o->LifeTime     = 9999999;
				}
				break;
			case MODEL_CURSEDTEMPLE_PRODECTION_SKILL:
				{
					o->LifeTime     = 9999999;
				}
				break;
			case MODEL_CURSEDTEMPLE_RESTRAINT_SKILL:
				{
					o->LifeTime     = 9999999;
				}
				break;
			case MODEL_ARROW_BOMB:
				o->Velocity = 1.f;
				o->LifeTime = 30;
				AngleMatrix(o->Angle,Matrix);
				Vector(-10.f,-60.f,135.f,p1);
				VectorRotate(p1,Matrix,p2);
				VectorAdd(o->Position,p2,o->Position);

     			o->BlendMesh = 0;
				o->Scale        = 1.f;
        		o->Direction[1] = -30.f;
				o->Gravity      = -10.f;
    			o->LifeTime     = 40;

				if(o->SubType==2)
				{
					o->AttackPoint[0] = 0;
					o->Kind = 1;
				}

				o->Weapon = CharacterMachine->PacketSerial;
				break;
			case MODEL_SAW:
				o->LifeTime = 10;
				o->Position[2] += 130.f;
				AngleMatrix(o->Angle,Matrix);
				Vector(0.f,-60.f,0.f,p1);
				VectorRotate(p1,Matrix,o->Direction);
				break;
			case MODEL_LASER:
				if(o->SubType==0 || o->SubType==3)
				{
					o->LifeTime  = 1;
					o->BlendMesh = 0;
					o->BlendMeshLight = o->Light[0];
					o->Scale     = 1.3f;
				    o->RenderType = RENDER_DARK;
				}
				else
				{
					o->LifeTime  = 30;
                    o->Velocity  = 1.f;
    			    o->BlendMesh = 0;
    			    o->BlendMeshLight = 1.f;
      			    o->Scale     = 1.3f;
				    o->RenderType = RENDER_DARK;

                    o->Position[2] += 150.f;
					Vector ( 0.f, -1.f, 0.f, o->Direction );
                    Vector ( 1.f, 0.f, 0.f, o->Light );
                    Vector ( 30.f, 0.f, Angle[2], o->Angle );

                    CreateJoint ( BITMAP_JOINT_FORCE, o->Position, o->Position, o->Angle, 1, o, 180.f );
				}
				break;
			case MODEL_MAGIC1:
				o->LifeTime = 20;
				o->BlendMesh = 0;
				break;
			case MODEL_SKILL_WHEEL1:
   				o->LifeTime = 5;
				CharacterMachine->PacketSerial++;
				break;
			case MODEL_SKILL_WHEEL2:
   				o->LifeTime = 25;//
				o->Weapon = CharacterMachine->PacketSerial;
				break;
            case MODEL_SKILL_FURY_STRIKE:
                {
                    VectorCopy(o->Angle,o->HeadAngle);
                    VectorCopy(o->Position,o->StartPosition);

                    o->LifeTime = 20;//18;
                    o->SubType = rand()%100;
				    o->Angle[2] += 330.f;
                    o->HeadAngle[0] += 80.f;
                    o->HeadAngle[2] +=180.f;
                    o->Gravity = 50.f;

				    o->Weapon = CharacterMachine->PacketSerial++;
                }
                break;
            case MODEL_SKILL_FURY_STRIKE+1:
                if ( o->SubType==0 )
                {
				    o->LifeTime = 35;
                    o->Scale = PKKey/100.f;
				    o->BlendMesh = 0;
                }
                else if ( o->SubType==1 )
                {
				    o->LifeTime = 60;
                    o->Scale = PKKey/100.f;
				    o->BlendMesh = 0;
                }
                break;
            case MODEL_SKILL_FURY_STRIKE+2:
                if ( o->SubType==0 || o->SubType==1 )
                {
				    o->LifeTime = 20;
                    o->Scale = PKKey/100.f;
				    o->BlendMesh = 0;
				    if(SubType==1)
    				    o->RenderType = RENDER_DARK;
                }
                else if ( o->SubType==2 )
                {
				    o->LifeTime = 50;
                    o->Scale = PKKey/100.f;
				    o->BlendMesh = 0;
                }
                break;
            case MODEL_SKILL_FURY_STRIKE+3:
				o->LifeTime = 35;
                o->Scale = PKKey/100.f;
                break;
            case MODEL_SKILL_FURY_STRIKE+4:
				o->LifeTime = 35;
                o->Scale = PKKey/100.f;
				o->BlendMesh = 0;
                break;
            case MODEL_SKILL_FURY_STRIKE+5:
				o->LifeTime = 40;
                o->Scale = PKKey/100.f;
				o->BlendMesh = 0;
				if(SubType==1)
    				o->RenderType = RENDER_DARK;
                break;
            case MODEL_SKILL_FURY_STRIKE+6:
				o->LifeTime = 35;
                o->Scale = PKKey/100.f;
                break;
            case MODEL_SKILL_FURY_STRIKE+7:
				o->LifeTime = 40;
                o->Scale = PKKey/100.f;
				o->BlendMesh = 0;
                break;
            case MODEL_SKILL_FURY_STRIKE+8:
				o->LifeTime = 40;
                o->Scale = PKKey/100.f;
				o->BlendMesh = 0;
				if(SubType==1)
    				o->RenderType = RENDER_DARK;
                break;
			case MODEL_SKILL_FISSURE:
				o->LifeTime = 20;
				break;
			case MODEL_FISSURE:
			case MODEL_FISSURE_LIGHT:
				o->LifeTime = 120;
				o->Scale = 0.8f;
				break;
			case MODEL_BALGAS_SKILL:
				o->BlendMesh = 0;
   				o->LifeTime = 20;
				o->Scale = 1.0f;
				break;
			case MODEL_CHANGE_UP_EFF:
				o->BlendMesh = -2;
   				o->LifeTime = 100;
				o->Scale = 0.7f;
				o->Position[0] = Position[0]; 
				o->Position[1] = Position[1]; 
				o->Position[2] = Position[2] +22.f; 
				Vector(0.f,0.f,0.f,o->Direction);
				if(o->SubType == 1)
				{
					o->Scale = 0.4f;
					o->LifeTime = 10;
					o->BlendMeshLight = 0.7f;
				}
				break;
			case MODEL_CHANGE_UP_NASA:
				o->BlendMesh = -2;
   				o->LifeTime = 100;
				o->Scale = 0.9f;
				if(o->SubType >= 1 && o->SubType <= 3)
					o->LifeTime = 80;
//					o->Scale = 0.f;
				o->Position[0] = Position[0]; 
				o->Position[1] = Position[1]; 
				o->Position[2] = Position[2] +12.f; 
				Vector(0.f,0.f,0.f,o->Direction);
//				Vector(0.1f,0.1f,0.1f,b->BodyLight);
				break;
			case MODEL_CHANGE_UP_CYLINDER:
				o->BlendMesh = -2;
   				o->LifeTime = 100;
				o->Scale = 0.9f;
				Vector(0.f,0.f,1.f,o->Direction);
				if(o->SubType == 1)
				{
					//o->Light
					Vector(0.f,0.f,0.f,o->Light);
					o->BlendMesh = -2;
					o->LifeTime = 10;
					o->Scale = 0.1f;
				}
				break;
			case MODEL_DARK_ELF_SKILL:
				o->BlendMesh = -2;
   				o->LifeTime = 30;
				o->Scale = 0.0f;
				Vector(0.f,-40.f,0.f,o->Direction);
				break;
			case MODEL_MAGIC2:
				o->BlendMesh = 0;
   				o->LifeTime = 20;
				Vector(0.f,-60.f,0.f,o->Direction);
				if ( o->SubType==2 )
				{
					o->Weapon = CharacterMachine->PacketSerial++;
				}
				break;
			case MODEL_STORM:
                switch ( o->SubType )
                {
                case 0:
				    o->LifeTime = 59;
				    o->BlendMesh = 0;
				    Vector(0.f,-10.f,0.f,o->Direction);
				    //Vector(0.f,-50.f,0.f,o->Direction);
				    o->Position[2] = RequestTerrainHeight(o->Position[0],o->Position[1]);
				    o->Weapon = CharacterMachine->PacketSerial++;
                    break;

                case 1:
				    o->LifeTime = 30;
                    o->HiddenMesh = -2;
                    o->Scale    = 1.f;
				    o->BlendMesh = 0;
				    Vector ( 0.f, 0.f, 0.f, o->Direction );
                    break;

                case 2:
				    o->LifeTime = 59;
                    o->HiddenMesh = -2;
                    o->Scale    = 1.f;
				    o->BlendMesh = 0;
				    Vector ( 0.f,-10.f,0.f,o->Direction);
				    Vector ( 1.f, 1.f, 1.f, o->Light );
                    break;
				case 3:
				case 4:
				case 5:
				case 6:
				case 7:
					{
						o->LifeTime = 60;
						o->HiddenMesh = -2;
						o->Scale = 1.0f;

						if(o->SubType == 3)	{Vector ( 0.f,-12.f,0.f,o->Direction);}
						else if(o->SubType == 4) {Vector ( 7.f,-6.f,0.f,o->Direction);}
						else if(o->SubType == 5) {Vector ( -7.f,-6.f,0.f,o->Direction);}
						else if(o->SubType == 6) {Vector ( 4.f,5.f,0.f,o->Direction);}
						else {Vector ( -4.f,5.f,0.f,o->Direction);}
					}
					break;
				case 8:
					{
						o->LifeTime = 100;
						o->BlendMesh = 0;
						o->Scale = 2.0f;
					}
					break;
                }
				break;
			case MODEL_SUMMON:
				{
					o->LifeTime = 60;
					o->BlendMesh = 0;
#ifdef PJH_SEASON4_SPRITE_NEW_SKILL_RECOVER
					if(o->SubType == 0)
						o->Scale = 0.7f;
					else
#endif //PJH_SEASON4_SPRITE_NEW_SKILL_RECOVER
					o->Scale = 1.2f;
					o->Angle[2] += rand()%360;
				}
				break;
			case MODEL_STORM2:
				{
					if(o->SubType == 0)
					{
						o->Angle[0] = 90.0f;
						o->LifeTime = 35;
						o->BlendMesh = 0;
						o->Scale = 0.5+rand()%10/100.0f;
					}
					else if(o->SubType == 1)
					{
						o->LifeTime = 60;
						o->BlendMesh = 0;
						o->Gravity = rand()%10 + 30.0f;
					}
					else if(o->SubType == 2)
					{
						o->LifeTime = 100;
						o->Angle[0] = 90.0f;
						o->BlendMesh = 0;
						o->Scale = 1.0f;
					}
				}
				break;
			case MODEL_STORM3:
				{
					o->LifeTime = 50;
					o->BlendMesh = 0;
					o->Angle[0] = 90.0f;
					o->Scale = 3.5f;
					VectorCopy(Hero->Object.Position, o->StartPosition);
				}
				break;
			case MODEL_MAYASTAR:
				{
					o->LifeTime = 50;
					o->Scale = 50.0f;
				}
				break;
			case MODEL_MAYASTONE1:	// 수정 광석 떨어지는 효과
			case MODEL_MAYASTONE2:
			case MODEL_MAYASTONE3:
				{
     				o->LifeTime = 40;
       				o->Scale = 6.0f+(float)(rand()%8+15)*0.1f;
					Vector(0.f,0.f,-60.f,o->Direction);
         			Vector(0.f,30.f,0.f,o->HeadAngle);

					CreateJoint(BITMAP_SMOKE,o->Position,o->Position,o->HeadAngle,2,o,100.f);
				}
				break;
			case MODEL_MAYASTONE4:
			case MODEL_MAYASTONE5:
				Vector(0.f,(float)(rand()%256+64)*0.1f,0.f,p1);
				o->LifeTime = rand()%16+32;
				o->Scale    = (float)(rand()%10)/3.0f+1.0f;
      			o->Angle[2] = (float)(rand()%360);
				AngleMatrix(o->Angle,Matrix);
				VectorRotate(p1,Matrix,o->Direction);
    			o->Gravity = (float)(rand()%16+8);
				break;
			case MODEL_MAYASTONEFIRE:	// 수정 광석 떨어지는 효과에 붙은 파티클 효과
				{
					if(o->SubType == MODEL_MAYASTONE1)
						o->Scale = Scale*0.8f;
					else if(o->SubType == MODEL_MAYASTONE2)
						o->Scale = Scale*0.6f;
					else if(o->SubType == MODEL_MAYASTONE3)
						o->Scale = Scale*0.5f;
					o->LifeTime = 40;
					Vector(0.f,0.f,-60.f,o->Direction);
         			Vector(0.f,30.f,0.f,o->HeadAngle);
				}
				break;
			case MODEL_MAYAHANDSKILL:	// 앞쪽으로 계속 생성 되는 효과
				{
					if(o->SubType == 0)
					{
						o->LifeTime = 20;
						// 라이트 값을 o->StartPosition변수에 담아놓는다.
						VectorCopy(o->Light, o->StartPosition);
					}
					else if(o->SubType == 1)
					{
						o->LifeTime = 20;
						// 라이트 값을 o->StartPosition변수에 담아놓는다.
						VectorCopy(o->Light, o->StartPosition);
					}
				}
				break;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			case MODEL_CIRCLE:
				o->LifeTime = 45;
				o->BlendMesh = 0;
                if ( o->SubType==0 )
                {
				    if ( o->Owner==&Hero->Object )              //  헬파이어 마법.
				    {
					    o->Weapon = CharacterMachine->PacketSerial++;
     				    AttackCharacterRange(o->Skill,o->Position,300.f,o->Weapon,o->PKKey);//클라이언트마법처리
				    }
                }
				else if ( o->SubType==1 || o->SubType == 4 )
				{
					o->LifeTime = 45;
					o->Scale = 1.f;
					o->HiddenMesh = -2;
				}
                else if ( o->SubType==2 )
                {
                    o->LifeTime = 250;
                    o->Scale    = 1.f;
                }
                else if ( o->SubType==3 )
                {
                    o->LifeTime = 30;
                    o->Scale    = 1.f;
                }
				break;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			case MODEL_CIRCLE_LIGHT:
				o->LifeTime = 40;
				o->BlendMesh = 0;
				//o->BlendMeshLight = 0.f;
				if(SubType==1)
    				o->RenderType = RENDER_DARK;
                else if ( o->SubType==2 )
				{
					o->LifeTime = 40;
					o->Scale = 1.f;
				}
                else if ( o->SubType==3 )
                {
                    o->LifeTime = 250;
                    o->Scale    = 1.f;
                }
                else if ( o->SubType==4 )
                {
                    o->LifeTime = 20;
                    o->Scale    = 1.f;
                }
				break;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			case MODEL_BLOOD:
				o->LifeTime = 10;
				o->BlendMesh = 0;
				break;
			case MODEL_ICE:
                switch ( o->SubType )
                {
                case 0:
				    o->LifeTime     = 50;
				    o->Scale        = 0.8f;
                    o->Velocity     = 1.f;
				    o->Angle[0]     = 0.f;
				    o->BlendMesh    = 0;
                    break;
                    
                case 1:
                case 2:
                    o->LifeTime = 20;
                    o->Scale    = 0.8f;
                    o->Angle[0] = -20.f;
                    o->BlendMesh= 0;
                    o->BlendMeshLight = 0.5f;
                    o->Gravity  = 5.f;
                    Vector ( 0.f, 0.f, (float)(rand()%360), o->HeadAngle );

                    //  연기를 내준다.
                    for ( int i=0; i<3; ++i )
			        {
				        vec3_t Position;
				        Vector(o->Position[0]+(float)(rand()%64-32),
					           o->Position[1]+(float)(rand()%64-32),
					           o->Position[2]+(float)(rand()%128+32),Position);
				        CreateParticle(BITMAP_SMOKE,Position,o->Angle,o->Light);
			        }
		            Vector(0.4f,0.3f,0.2f,Light);
		            AddTerrainLight(o->Position[0],o->Position[1],Light,2,PrimaryTerrainLight);

                    VectorCopy ( o->Position, o->StartPosition );
                    break;
                }
				break;
			case MODEL_SNOW1:
				AngleMatrix(o->Angle,Matrix);
				Vector(0.f,-40.f,150.f,p1);
				VectorRotate(p1,Matrix,p2);
				VectorAdd(o->Position,p2,o->Position);
				o->Direction[1] = -40.f;
				o->Direction[2] = 10.f;
				o->LifeTime     = 20;
    			o->Scale        = 1.2f;
				break;
			case MODEL_WOOSISTONE:
				if(o->SubType==1)	// 돌 파편
				{
					o->Direction[0] = 0;
					o->Direction[1] = 0;
					o->Direction[2] = 0;

					o->LifeTime = rand()%16+20;
					o->Scale    = (float)(rand()%13+3)*0.04f;
					o->Gravity = (float)(rand()%3+3);

					o->Angle[2] = (float)(rand()%360);
					AngleMatrix(o->Angle,Matrix);
					Vector(0.f,(float)(rand()%128+64)*0.1f,0.f,p1);
					VectorRotate(p1,Matrix,o->HeadAngle);
					o->HeadAngle[2] += 15.0f;
					break;
				}
				else	// 돌 덩이
				{
					AngleMatrix(o->Angle,Matrix);
					Vector(0.f,-60.f,150.f,p1);
					VectorRotate(p1,Matrix,p2);
					VectorAdd(o->Position,p2,o->Position);
					o->Direction[1] = -40.f;
					o->Direction[2] = 10.f;
					o->LifeTime     = 20;
    				o->Scale        = 1.0f;
					break;
				}
			case MODEL_SKULL:
				if ( o->SubType==0 )
				{
    				o->LifeTime  = 1;
          			o->Scale     = 2.3f;
				}
				else if ( o->SubType==1 )
				{
				    o->LifeTime = 49;
				    o->Scale    = 2.f;
                    o->BlendMesh= -2;
                    o->BlendMeshLight = 0.5f;
                    Vector ( 0.f, 0.f, 0.f, o->HeadAngle );
                    Vector ( 0.f, -45.f, 0.f, o->Direction );
                    VectorCopy ( o->Position, o->StartPosition );
                    VectorCopy ( o->Position, o->m_vDeadPosition );

                    //  
                    VectorCopy ( o->Position, o->EyeLeft );
                    VectorCopy ( o->Position, o->EyeRight );

   		            CreateJoint ( BITMAP_JOINT_ENERGY, o->Position, o->Position, o->Angle, 10, o, 30.f );
   		            CreateJoint ( BITMAP_JOINT_ENERGY, o->Position, o->Position, o->Angle, 11, o, 30.f );
				}
				break;
			case MODEL_CUNDUN_PART1:
			case MODEL_CUNDUN_PART2:
			case MODEL_CUNDUN_PART3:
			case MODEL_CUNDUN_PART4:
			case MODEL_CUNDUN_PART5:
			case MODEL_CUNDUN_PART6:
			case MODEL_CUNDUN_PART7:
			case MODEL_CUNDUN_PART8:
			case MODEL_MONSTER01+64:	// 진짜쿤둔
				switch(o->SubType)
				{
				case 1:	// 몸
					o->LifeTime     = 100;
    				o->Scale        = 2.0f;
					break;
				case 2:	// 투구
				case 3:	// 갑옷
					Vector(o->Position[0], o->Position[1],
						RequestTerrainHeight(o->Position[0],o->Position[1]) - (float)o->PKKey - 80,
						o->StartPosition);
					Vector(0,0,0,o->Direction);
					o->LifeTime     = 250*2;
    				o->Scale        = 2.0f;
					o->HeadAngle[0]	= (float)(rand()%2);
					if (o->SubType == 2) o->HeadAngle[0] = 0;
					break;
				case 4:	// 망또
					Vector(o->Position[0], o->Position[1],
						RequestTerrainHeight(o->Position[0],o->Position[1]) - (float)o->PKKey,
						o->StartPosition);
					Vector(0,0,0,o->Direction);
				case 5:	// 치마
					o->CurrentAction = 0;
					o->LifeTime     = 170;
    				o->Scale        = 2.0f;
					break;
				}
				// 쿤둔 조각 좌표맞추는 코드 -_-
//				o->LifeTime = 1000000;
				// 여기까지 쿤둔 조각 좌표맞추는 코드
				o->PKKey = -1;
				o->Owner = Owner;
				break;
			case MODEL_CURSEDTEMPLE_STATUE_PART1:
			case MODEL_CURSEDTEMPLE_STATUE_PART2:
				o->LifeTime     = 180+(rand()%40);
    			o->Scale        = 0.1f+(rand()%6)*0.1f;
                o->Velocity     = 0.f;
				o->PKKey = -1;
				o->Owner = Owner;
                o->Gravity      = 2.5f;
				o->Direction[0] = (float)((-15 + (rand()%30)));
				o->Direction[1] = (float)((-15 + (rand()%30)));
				o->Direction[2] = 0;
				o->Angle[0] = 0;//( float)( rand() % 360);
				o->Angle[1] = 0;//( float)( rand() % 360);
				o->Angle[2] = ( float)( rand() % 360);
				o->Light[0] = o->Light[1] = o->Light[2] = 0.5+(rand()%6)*0.1f;
				break;
#ifdef LDK_ADD_SNOWMAN_CHANGERING
			case MODEL_XMAS2008_SNOWMAN_HEAD:
				{
					o->LifeTime = 50;
					o->Scale = 1.3f;
					
 					o->Angle[2] = o->Owner->Angle[2];
					AngleMatrix(o->Angle, Matrix);
					o->Gravity = 5.0f;
					vec3_t p;
					Vector((float)(rand()%10-5)*0.1f, (float)(rand()%60-40)*0.1f, 0.0f, p);
					VectorScale(p, 1.3f, p);
					VectorRotate(p, Matrix, o->Direction);
					o->m_iAnimation = rand()%3;
				}
				break;
			case MODEL_XMAS2008_SNOWMAN_BODY:
				{
					o->LifeTime     = 50;
					o->Scale		= 1.3f;
					o->Velocity     = 1.2f;
					
					o->PKKey = -1;
					o->Owner = Owner;
					
					o->Direction[0] = o->Owner->Direction[0];
					o->Direction[1] = o->Owner->Direction[1];
					o->Direction[2] = o->Owner->Direction[2];
				}
				break;
#endif // LDK_ADD_SNOWMAN_CHANGERING
			case MODEL_TOTEMGOLEM_PART1:
			case MODEL_TOTEMGOLEM_PART2:
			case MODEL_TOTEMGOLEM_PART3:
			case MODEL_TOTEMGOLEM_PART4:
			case MODEL_TOTEMGOLEM_PART5:
			case MODEL_TOTEMGOLEM_PART6:
				o->LifeTime     = 30+(rand()%30);
				o->Scale        = 0.17f;//0.1f+(rand()%6)*0.1f;
                o->Velocity     = 0.f;
				o->PKKey = -1;
				o->Owner = Owner;
                o->Gravity      = 3.5f;
				
				o->Angle[2] = (float)(rand()%360);
				AngleMatrix(o->Angle,Matrix);
				Vector(0.f,(float)(rand()%64+48)*0.1f,0.f,p1);
				VectorRotate(p1,Matrix,o->HeadAngle);
				o->HeadAngle[2] = 15;
				
				o->SubType = rand()%2;
				
				o->Light[0] = o->Light[1] = o->Light[2] = 0.5+(rand()%6)*0.1f;
				o->Direction[0] = 0;
				o->Direction[1] = 0;
				o->Direction[2] = 0;
				break;
#ifdef ASG_ADD_KARUTAN_MONSTERS
			// 토템 골렘과 스케일만 빼고 동일.
			case MODEL_CONDRA_ARM_L:
			case MODEL_CONDRA_ARM_L2:
			case MODEL_CONDRA_SHOULDER:
			case MODEL_CONDRA_ARM_R:
			case MODEL_CONDRA_ARM_R2:
			case MODEL_CONDRA_CONE_L:
			case MODEL_CONDRA_CONE_R:
			case MODEL_CONDRA_PELVIS:
			case MODEL_CONDRA_STOMACH:
			case MODEL_CONDRA_NECK:

			case MODEL_NARCONDRA_ARM_L:
			case MODEL_NARCONDRA_ARM_L2:
			case MODEL_NARCONDRA_SHOULDER_L:
			case MODEL_NARCONDRA_SHOULDER_R:
			case MODEL_NARCONDRA_ARM_R:
			case MODEL_NARCONDRA_ARM_R2:
			case MODEL_NARCONDRA_ARM_R3:
			case MODEL_NARCONDRA_CONE_1:
			case MODEL_NARCONDRA_CONE_2:
			case MODEL_NARCONDRA_CONE_3:
			case MODEL_NARCONDRA_CONE_4:
			case MODEL_NARCONDRA_CONE_5:
			case MODEL_NARCONDRA_CONE_6:
			case MODEL_NARCONDRA_PELVIS:
			case MODEL_NARCONDRA_STOMACH:
			case MODEL_NARCONDRA_NECK:
				o->LifeTime     = 30+(rand()%30);
				o->Scale        = 1.4f;
                o->Velocity     = 0.f;
				o->PKKey = -1;
				o->Owner = Owner;
                o->Gravity      = 3.5f;

				o->Angle[2] = (float)(rand()%360);
				AngleMatrix(o->Angle,Matrix);
				Vector(0.f,(float)(rand()%64+48)*0.1f,0.f,p1);
				VectorRotate(p1,Matrix,o->HeadAngle);
 				o->HeadAngle[2] = 15;

				o->SubType = rand()%2;

				o->Light[0] = o->Light[1] = o->Light[2] = 0.5+(rand()%6)*0.1f;
				o->Direction[0] = 0;
				o->Direction[1] = 0;
				o->Direction[2] = 0;
				break;
#endif	// ASG_ADD_KARUTAN_MONSTERS
#ifdef YDG_ADD_DOPPELGANGER_MONSTER
			case MODEL_DOPPELGANGER_SLIME_CHIP:
				o->LifeTime     = 30+(rand()%10);
    			o->Scale        = 0.1f+(rand()%7)*0.1f;
                o->Velocity     = 0.f;
				o->PKKey = -1;
				o->Owner = Owner;
                o->Gravity      = 4.0f;

				o->Angle[2] = (float)(rand()%360);
				AngleMatrix(o->Angle,Matrix);
				Vector(0.f,(float)(rand()%64+48)*0.1f,0.f,p1);
				VectorRotate(p1,Matrix,o->HeadAngle);
 				o->HeadAngle[2] = 35+rand()%5;

				o->SubType = rand()%2;

				VectorCopy(Light, o->Light);
				o->Direction[0] = 0;
				o->Direction[1] = 0;
				o->Direction[2] = 0;

				o->Angle[0] = (float)(rand()%360);
				o->Angle[1] = (float)(rand()%360);
				break;
#endif	// YDG_ADD_DOPPELGANGER_MONSTER
			case MODEL_SHADOW_PAWN_ANKLE_LEFT:		case MODEL_SHADOW_PAWN_ANKLE_RIGHT:
			case MODEL_SHADOW_PAWN_BELT:			case MODEL_SHADOW_PAWN_CHEST:
			case MODEL_SHADOW_PAWN_HELMET:
			case MODEL_SHADOW_PAWN_KNEE_LEFT:		case MODEL_SHADOW_PAWN_KNEE_RIGHT:
			case MODEL_SHADOW_PAWN_WRIST_LEFT:		case MODEL_SHADOW_PAWN_WRIST_RIGHT:

			case MODEL_SHADOW_KNIGHT_ANKLE_LEFT:	case MODEL_SHADOW_KNIGHT_ANKLE_RIGHT:
			case MODEL_SHADOW_KNIGHT_BELT:			case MODEL_SHADOW_KNIGHT_CHEST:
			case MODEL_SHADOW_KNIGHT_HELMET:
			case MODEL_SHADOW_KNIGHT_KNEE_LEFT:		case MODEL_SHADOW_KNIGHT_KNEE_RIGHT:
			case MODEL_SHADOW_KNIGHT_WRIST_LEFT:	case MODEL_SHADOW_KNIGHT_WRIST_RIGHT:
				
#ifdef ADD_RAKLION_MOB_ICEGIANT
			case MODEL_ICE_GIANT_PART1:				case MODEL_ICE_GIANT_PART2:
			case MODEL_ICE_GIANT_PART3:				case MODEL_ICE_GIANT_PART4:
			case MODEL_ICE_GIANT_PART5:				case MODEL_ICE_GIANT_PART6:
#endif	// ADD_RAKLION_MOB_ICEGIANT

				o->LifeTime     = 30+(rand()%30);
    			o->Scale        = 1.1f;//0.1f+(rand()%6)*0.1f;
                o->Velocity     = 0.f;
				o->PKKey = -1;
				o->Owner = Owner;
                o->Gravity      = 3.5f;

				o->Angle[2] = (float)(rand()%360);
				AngleMatrix(o->Angle,Matrix);
				Vector(0.f,(float)(rand()%64+48)*0.1f,0.f,p1);
				VectorRotate(p1,Matrix,o->HeadAngle);
 				o->HeadAngle[2] = 15;

				o->SubType = rand()%2;

				o->Light[0] = o->Light[1] = o->Light[2] = 0.5+(rand()%6)*0.1f;
				o->Direction[0] = 0;
				o->Direction[1] = 0;
				o->Direction[2] = 0;
				break;
			case MODEL_SHADOW_ROOK_ANKLE_LEFT:		case MODEL_SHADOW_ROOK_ANKLE_RIGHT:
			case MODEL_SHADOW_ROOK_BELT:			case MODEL_SHADOW_ROOK_CHEST:
			case MODEL_SHADOW_ROOK_HELMET:
			case MODEL_SHADOW_ROOK_KNEE_LEFT:		case MODEL_SHADOW_ROOK_KNEE_RIGHT:
			case MODEL_SHADOW_ROOK_WRIST_LEFT:		case MODEL_SHADOW_ROOK_WRIST_RIGHT:
				o->LifeTime     = 30+(rand()%30);
    			o->Scale        = 1.3f;//0.1f+(rand()%6)*0.1f;
                o->Velocity     = 0.f;
				o->PKKey = -1;
				o->Owner = Owner;
                o->Gravity      = 3.5f;

				o->Angle[2] = (float)(rand()%360);
				AngleMatrix(o->Angle,Matrix);
				Vector(0.f,(float)(rand()%64+48)*0.1f,0.f,p1);
				VectorRotate(p1,Matrix,o->HeadAngle);
 				o->HeadAngle[2] = 15;

				o->SubType = rand()%2;

				o->Light[0] = o->Light[1] = o->Light[2] = 0.5+(rand()%6)*0.1f;
				o->Direction[0] = 0;
				o->Direction[1] = 0;
				o->Direction[2] = 0;
				break;

            case MODEL_WATER_WAVE:
				o->BlendMesh    = -2;
                o->BlendMeshLight =  0.2f;
                o->Scale        = 0.4f;
   				o->LifeTime     = 20;
                o->Gravity      = 120.f;
                o->Velocity     = -40.f;
				Vector ( 0.f, o->Velocity, 0.f, o->Direction );
				Vector ( 1.f, 1.f, 1.f, o->Light );
                VectorCopy ( o->Position, o->StartPosition );
                break;

			case MODEL_FIRE:
     			o->BlendMesh     = 1;
				if(o->SubType==0)
				{
     				o->LifeTime = 40;
       				o->Scale = (float)(rand()%8+10)*0.1f;
					o->Position[0] += 130.f+rand()%32;
					o->Position[2] += 400.f;
					Vector(0.f,0.f,-50.f,o->Direction);
         			Vector(0.f,20.f,0.f,o->Angle);
				}
				else if(o->SubType==2)//드래곤
				{
     				o->LifeTime = 40;
       				o->Scale = (float)(rand()%8+10)*0.1f;
					Vector(0.f,0.f,-50.f,o->Direction);
         			//o->Angle[0] = -20.f;
				}
				else if(o->SubType==3)//불사조
				{
     				o->LifeTime = 80;
       				o->Scale = 0.3f;
					Vector(0.f,-12.f,0.f,o->Direction);
//					PlayBuffer( SOUND_PHOENIXFIRE);
				}
                else if ( o->SubType==4 )   //  블러드캐슬.
                {
     				o->LifeTime = 40;
       				o->Scale = (float)(rand()%10+15)*0.1f;
					o->Position[0] += 130.f+rand()%32;
					o->Position[2] += 400.f;
					Vector(0.f,-(rand()%20+10.f),-(rand()%10+20.f),o->Direction);
         			Vector(0.f,20.f,0.f,o->Angle);
                }
                else if ( o->SubType==5 )   //  오거.
                {
     				o->LifeTime = 40;
                    o->Gravity  = 5.f;
					Vector(0.f,-30.f,0.f,o->Direction);
                }
				else if ( o->SubType==6 )	//	왕 독.
				{
     				o->LifeTime = 40;
					o->HiddenMesh = -2;
       				o->Scale = (float)(rand()%8+15)*0.1f;
					o->PKKey = -1;
					o->Velocity = PKKey;
					o->Position[0] += rand()%100+200;
					o->Position[1] += rand()%100-50;
					o->Position[2] += rand()%300+500;
					Vector(0.f,0.f,-50.f-rand()%50,o->Direction);
         			Vector(0.f,20.f,0.f,o->HeadAngle);

					CreateJoint(BITMAP_SMOKE,o->Position,o->Position,o->HeadAngle,0,o,100.f);
				}
				else if ( o->SubType==7 )	//	왕 독2 ( 
				{
     				o->LifeTime = 40;
					o->HiddenMesh = -2;
       				o->Scale = (float)(rand()%8+15)*0.1f;
					o->PKKey = -1;
					o->Velocity = PKKey;
					o->Position[0] += rand()%100+200;
					o->Position[1] += rand()%100-50;
					o->Position[2] += rand()%300+500;
					Vector(0.f,0.f,-50.f-rand()%50,o->Direction);
         			Vector(0.f,20.f,0.f,o->HeadAngle);
				}
				else if ( o->SubType==8 )	//	왕 독3 ( 
				{
     				o->LifeTime = 40;
					o->HiddenMesh = -2;
       				o->Scale = (float)(rand()%8+15)*0.1f;
					o->PKKey = -1;
					o->Velocity = PKKey;
//					o->Position[0] += rand()%100+200;
//					o->Position[1] += rand()%100-50;
					o->Position[2] += rand()%300+500;
					Vector(0.f,0.f,-50.f-rand()%50,o->Direction);
         			Vector ( 0.f, (float)(5+rand()%5), 0.f, o->HeadAngle );

					CreateJoint ( BITMAP_SMOKE, o->Position, o->Position, o->HeadAngle, 0, o, 60.f );
				}
				else if(o->SubType==9)
				{
					o->BlendMeshLight =0.f;
     				o->LifeTime = 600;
       				o->Scale = (float)(rand()%10+20)*0.13f;
					o->Position[0] += 130.f+rand()%32;
					o->Position[2] += 400.f+rand()%32;
					Vector(0.f,-rand()%5-8.0f,-10.f-rand()%10,o->Direction);
         			Vector(0.f,0.f,0.f,o->Angle);
				}
				else
				{
     				o->LifeTime = 60;
       				o->Scale = (float)(rand()%4+8)*0.1f;
					o->Position[2] += 120.f;
					Vector(0.f,-50.f,0.f,o->Direction);
				}
				break;
			case MODEL_BONE1:
				o->Position[2] += 50.f;
			case MODEL_BONE2:
				o->Position[2] += 100.f;
			case MODEL_BIG_STONE1:
			case MODEL_BIG_STONE2:
                if(o->SubType==5)
                {
                    o->LifeTime = 60;
                    o->Scale    = (float)(rand()%4+8)*0.1f;
                    VectorCopy( o->Position, o->StartPosition );
                    VectorCopy( o->Owner->Position, o->Position );
                    break;
                }
			case MODEL_SNOW2:
			case MODEL_SNOW3:
			case MODEL_STONE1:
			case MODEL_STONE2:
                if(o->SubType==5)
                {
                    o->LifeTime = 60;
                    o->Scale    = (float)(rand()%4+8)*0.1f;
                    break;
                }
                else if ( o->SubType==11 )
                {
                    o->LifeTime = 40;
                    o->Scale    = (float)(rand()%4+8)*0.8f;
                    o->Angle[2] = (float)(rand()%360);
                    break;
                }
				else if(o->SubType==10)	//. 파편스케일이 좀 큰경우.
				{
					Vector(0.f,(float)(rand()%256+64)*0.2,0.f,p1);
					o->LifeTime = rand()%16+32;
					o->Scale    = (float)(rand()%4+15)*0.05f;
					o->Angle[2] = (float)(rand()%360);
					AngleMatrix(o->Angle,Matrix);
					VectorRotate(p1,Matrix,o->Direction);
					o->Gravity = (float)(rand()%16+28);
					break;
				}
				else if(o->SubType==12)	//. 파편스케일이 좀 큰경우.
				{
					Vector(0.f,(float)(rand()%256+64)*0.1f,0.f,p1);
					o->LifeTime = rand()%16+32;
					o->Scale    = (float)(rand()%4+15)*0.1f;
					o->Angle[2] = (float)(rand()%360);
					AngleMatrix(o->Angle,Matrix);
					VectorRotate(p1,Matrix,o->Direction);
					o->Gravity = (float)(rand()%16+8);
					break;
				}
				else if (o->SubType == 13
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
					|| o->SubType == 14
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
					)
				{
					o->Direction[0] = 0;
					o->Direction[1] = 0;
					o->Direction[2] = 0;

					o->LifeTime = rand()%16+20;
					o->Scale    = (float)(rand()%13+3)*0.08f;
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
					o->Scale *= Scale;
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
					o->Gravity = (float)(rand()%3+3);

					o->Angle[2] = (float)(rand()%360);
					AngleMatrix(o->Angle,Matrix);
					Vector(0.f,(float)(rand()%128+64)*0.1f,0.f,p1);
					VectorRotate(p1,Matrix,o->HeadAngle);
					o->HeadAngle[2] += 15.0f;
					break;
				}

			case MODEL_ICE_SMALL:
			case MODEL_METEO1:
			case MODEL_METEO2:
			case MODEL_EFFECT_SAPITRES_ATTACK_2:		// 사피트레스 가시떨어지기
				if(o->Type==MODEL_BIG_STONE1 || o->Type==MODEL_BIG_STONE2)
				{
					Vector((float)(rand()%128-64),(float)(rand()%128-64),(float)(rand()%180),p1);
					VectorAdd(o->Position,p1,o->Position);
				}
#ifdef YDG_FIX_STONE_FLY
				if(Type==MODEL_ICE_SMALL)
#else	/// YDG_FIX_STONE_FLY
				else if(Type==MODEL_ICE_SMALL )
#endif	// YDG_FIX_STONE_FLY
				{
					o->BlendMesh = 0;
					o->BlendMeshLight = 0.3f;
    				Vector(0.f,(float)(rand()%256+64)*0.1f,0.f,p1);
					o->Position[2] += 50.f;
#ifdef PJH_GIANT_MAMUD
					if(o->SubType == 13)
					{
						Vector(0.f,(float)(rand()%256+64)*0.2,0.f,p1);
						o->LifeTime = rand()%16+32;
						o->Scale    = (float)(rand()%4+15)*0.05f;
						o->Angle[2] = (float)(rand()%360);
						AngleMatrix(o->Angle,Matrix);
						VectorRotate(p1,Matrix,o->Direction);
						o->Gravity = (float)(rand()%16+28);
					}
//						o->Scale += 50.f;
#endif //#ifdef PJH_GIANT_MAMUD
				}
				else if( Type == MODEL_EFFECT_SAPITRES_ATTACK_2 )
				{
					o->BlendMesh = 0;
					o->BlendMeshLight = 1.0f;
					Vector(0.f,(float)(rand()%256+64)*0.1f,0.f,p1);
					o->Position[2] += 50.f;
				}
				else
				{
					if(o->SubType==1)
					{
         				Vector(0.f,0.f,0.f,p1);
					}
					else
					{
         				Vector(0.f,(float)(rand()%256+64)*0.1f,0.f,p1);
					}
				}
				if(o->SubType == 13)
				{
					o->LifeTime = rand()%16+100;
					o->Scale    = (float)(rand()%4+8)*0.1f;
      				o->Angle[2] = (float)(rand()%360);
					AngleMatrix(o->Angle,Matrix);
					VectorRotate(p1,Matrix,o->Direction);
    				o->Gravity = (float)(rand()%16+8);
				}
				else if( Type == MODEL_EFFECT_SAPITRES_ATTACK_2 && o->SubType == 14 )
				{
					o->LifeTime = rand()%5+15;
					o->Angle[0] = (float)(rand()%60-30);
					o->Angle[1] = (float)(rand()%60-30);
      				o->Angle[2] = (float)(rand()%360);
					AngleMatrix(o->Angle, Matrix);
					VectorRotate(p1, Matrix, o->Direction);
					VectorNormalize( o->Direction );
    				o->Gravity = 0.f;
				}
				else
				{
					o->LifeTime = rand()%16+32;
					o->Scale    = (float)(rand()%4+8)*0.1f;
      				o->Angle[2] = (float)(rand()%360);
					AngleMatrix(o->Angle,Matrix);
					VectorRotate(p1,Matrix,o->Direction);
    				o->Gravity = (float)(rand()%16+8);
				}
				break;

//------------------------------------------------------------------------------------------

#ifdef ADD_RAKLION_IRON_KNIGHT
			case MODEL_EFFECT_BROKEN_ICE0:
			case MODEL_EFFECT_BROKEN_ICE1:
			case MODEL_EFFECT_BROKEN_ICE2:
			case MODEL_EFFECT_BROKEN_ICE3:
#ifdef ASG_ADD_KARUTAN_MONSTERS
			case MODEL_CONDRA_STONE:
			case MODEL_CONDRA_STONE1:
			case MODEL_CONDRA_STONE2:
			case MODEL_CONDRA_STONE3:
			case MODEL_CONDRA_STONE4:
			case MODEL_CONDRA_STONE5:
			case MODEL_NARCONDRA_STONE:
			case MODEL_NARCONDRA_STONE1:
			case MODEL_NARCONDRA_STONE2:
			case MODEL_NARCONDRA_STONE3:
#endif	// ASG_ADD_KARUTAN_MONSTERS
				if(o->SubType == 0)	// 얼음 파편
				{
					o->Direction[0] = 0;
					o->Direction[1] = 0;
					o->Direction[2] = 0;

					o->LifeTime = rand()%16+35;
					o->Scale    = (float)(rand()%13+3)*0.2f;
					o->Gravity = (float)(rand()%3+3);

					o->Angle[2] = (float)(rand()%360);
					AngleMatrix(o->Angle,Matrix);
					Vector(0.f,(float)(rand()%128+64)*0.1f,0.f,p1);
					VectorRotate(p1,Matrix,o->HeadAngle);
					o->HeadAngle[2] += 25.0f;
					break;
				}
				else if(o->SubType == 1)	// 얼음 하늘에서 떨어지는 효과
				{
					o->LifeTime = 40;
					o->Scale = Scale + (float)(rand()%8+15) * 0.1f;
					Vector(0.f, 0.f, -60.f, o->Direction);
					Vector(0.f, 30.f, 0.f, o->HeadAngle);
				}
				else if(o->SubType == 2)
				{
					o->LifeTime = 100;
					o->Scale = Scale;
					o->Gravity = 20.f + (float)(rand()%20) * 0.5f;
				}
				break;
#endif	// ADD_RAKLION_IRON_KNIGHT
#ifdef PBG_ADD_PKFIELD
			case MODEL_PKFIELD_ASSASSIN_EFFECT_GREEN_HEAD:
			case MODEL_PKFIELD_ASSASSIN_EFFECT_RED_HEAD:
				{			
					o->LifeTime     = 50+(rand()%30);
					o->Scale        = 1.0f;
					o->Velocity     = 1.0f;
					o->PKKey = -1;
					o->Owner = Owner;
					o->Gravity      = 3.5f;
					
					o->Angle[2] = (float)(rand()%360);
					AngleMatrix(o->Angle,Matrix);
					Vector(0.f,(float)(rand()%64+25)*0.2f,0.f,p1);
					VectorRotate(p1,Matrix,o->HeadAngle);
					o->HeadAngle[2] = 25;
					
					o->SubType = rand()%2;
					
					o->Direction[0] = 0;
					o->Direction[1] = 0;
					o->Direction[2] = 0;	
				}
				break;
			case MODEL_PKFIELD_ASSASSIN_EFFECT_GREEN_BODY:
			case MODEL_PKFIELD_ASSASSIN_EFFECT_RED_BODY:
				{
					o->Scale    = 1.0f;
					o->LifeTime = 40+(rand()%30);
					o->Velocity = 0.4f;						//b->Actions[MONSTER01_DIE].PlaySpeed
					o->CurrentAction = MONSTER01_DIE;
				}
				break;
#endif //PBG_ADD_PKFIELD
//------------------------------------------------------------------------------------------

			case MODEL_DUNGEON_STONE01:
				o->LifeTime     = rand()%16+24;
				o->Scale        = (float)(rand()%8+6)*0.1f;
     			o->Gravity      = -(float)(rand()%4);
     			Vector((float)(rand()%64-32),-(float)(rand()%32+50),(float)(rand()%128+200),p1);
				AngleMatrix(o->Angle,Matrix);
				VectorRotate(p1,Matrix,p2);
				VectorAdd(o->Position,p2,o->Position);
				break;

//------------------------------------------------------------------------------------------

			case MODEL_WARCRAFT:
				o->LifeTime     = 50;
				o->Scale        = 0.7f;
				if(SubType==0)
    				o->BlendMesh    = -3;
				else
    				o->BlendMesh    = -4;
				Vector(0.f,0.f,45.f,o->Angle);
				break;

//------------------------------------------------------------------------------------------

			case MODEL_POISON:
				o->BlendMesh    = 1;
				o->LifeTime     = 40;
				o->Scale        = 1.f;
				break;
			/*case MODEL_STONE01:
				o->LifeTime     = 100;
    			o->Scale        = (float)(rand()%8+8)*0.02f;
     			o->Position[2]  = RequestTerrainHeight(o->Position[0],o->Position[1]);
    			o->Direction[2] = 0.f;
				break;*/
			case BITMAP_FIRECRACKERRISE:	// 발사대
				ZeroMemory( o->Angle, sizeof ( o->Angle));
				o->Position[2] = 100.0f;
				o->LifeTime = 15*5;
				break;
			case BITMAP_FIRECRACKER:	// 폭죽 발사
				Vector( 1.0f, 1.0f, 1.0f, o->Light);
				Vector(( float)( rand()%9-4),( float)( rand()%9-4),26.f,o->Direction);
				
				if(o->SubType == 1)
					o->LifeTime = 4+(rand()%3);
				else
					o->LifeTime = 12;

				PlayBuffer(SOUND_FIRECRACKER1,o);
				break;
#ifdef YDG_ADD_FIRECRACKER_ITEM
			case BITMAP_FIRECRACKER0001:	// 새 폭죽 발사대
				// subtype이 BITMAP_FIRECRACKER0002로 전달되므로 기본폭축 추가 효과 가능
				// subtype 추가하지 마세요!
				{
					o->LifeTime = 31;
					Vector(0, 0, 0.f, o->Direction);
				}
				break;
			case BITMAP_FIRECRACKER0002:	// 새 폭죽 폭발
				// subtype 추가하지 마세요!
				// BITMAP_FIRECRACKER0001에서 전달됨
				{
					o->LifeTime = 30;

					CreateParticle(BITMAP_EXPLOTION_MONO,o->Position,o->Angle,o->Light, 0, 0.6f);
					
					// 스파크
					vec3_t vLight;
					Vector(1.0f, 1.0f, 1.0f, vLight);
					for( int i=0 ; i<60 ; i++)
					{
						// 퍼지는거
						CreateParticle(BITMAP_SPARK+1, o->Position, o->Angle, vLight, 27 );
					}				
#ifdef _VS2008PORTING
					for( int i=0 ; i<30 ; i++ )
#else // _VS2008PORTING
					for( i=0 ; i<30 ; i++ )
#endif // _VS2008PORTING
					{
						// 퍼져서 떨어지는거
						CreateParticle(BITMAP_SPARK+1, o->Position, o->Angle, vLight, 28 );
					}
					
					// 번쩍
					Vector(o->Position[0]+(rand()%100-50), o->Position[1]+(rand()%100-50),
						o->Position[2], Position);
					CreateSprite(BITMAP_DS_SHOCK, Position, rand()%10*0.1f+1.5f, o->Light, o);

					// 별
#ifdef _VS2008PORTING
					for( int i=0 ; i<60 ; i++ )
#else // _VS2008PORTING
					for( i=0 ; i<60 ; i++ )
#endif // _VS2008PORTING
					{
						Vector(0.3f+(rand()%700)*0.001f, 0.3f+(rand()%700)*0.001f, 0.3f+(rand()%700)*0.001f, vLight);
						CreateParticle(BITMAP_SHINY, o->Position, o->Angle, vLight, 6 );
					}

					if (o->SubType == 1)	// 크리스마스 폭죽
					{
						CreateEffect(MODEL_HALLOWEEN_CANDY_STAR, o->Position, o->Angle, vLight, 1);
						CreateEffect(rand()%4+MODEL_XMAS_EVENT_BOX, o->Position, o->Angle, vLight, 0, o);
						CreateEffect(rand()%4+MODEL_XMAS_EVENT_BOX, o->Position, o->Angle, vLight, 0, o);
					}

					PlayBuffer(SOUND_XMAS_FIRECRACKER,o);
				}
				break;
			case BITMAP_FIRECRACKER0003:	// 새 폭죽 폭발 애니
				{
					o->LifeTime = 15;
					o->Angle[2] = rand()%360;
				}
				break;
#endif	// YDG_ADD_FIRECRACKER_ITEM
            case BITMAP_SWORD_FORCE:
                o->LifeTime = 30;
				if(o->SubType == 0 || o->SubType == 1)
					Vector(0.8f,0.8f,0.8f,o->Light);
                Vector(0.f,0.f,Angle[2]+45.f,o->HeadAngle);
                VectorCopy(o->Position, o->StartPosition);
                break;
            case MODEL_CLOUD:
                o->LifeTime = 2;
                o->BlendMesh = 0;
                o->Scale = 10.f;
                o->Position[1] += 200.f;
                o->Position[2] -= 190.f;
                o->LightEnable = false;
                break;
            case BITMAP_BLIZZARD:
                {
                    o->LifeTime = rand()%15+15;
                    o->Gravity  = -20.f;
                    o->Velocity = (float)(rand()%360);
					Vector ( 0.f, 0.f, 0.f, o->Light );

					int rangeX, rangeY, rangeZ;
					if ( o->SubType==1 )
					{
						rangeX = 300; rangeY = 150; rangeZ = 700;
						o->Scale = 0.5f;
						o->Gravity -= rand()%20+10;
					}
					else
					{
						rangeX = 200; rangeY = 100; rangeZ = 500;
						o->Scale = 0.f;
					}

                    o->Position[0] = o->Position[0] + rand()%rangeX-rangeY;
                    o->Position[1] = o->Position[1] + rand()%rangeX-rangeY;
                    o->Position[2] = o->Position[2] + 500.f;
                    o->Position[0] += 100.f;

                    VectorCopy(o->Position,o->StartPosition);
					PlayBuffer( SOUND_METEORITE01);
                }
                break;

#ifdef TAMAJJANG
            case MODEL_TAMAJJANG :
                {
                    //  0: 제자리. 물속에 숨었다가 다시 나오고 반복.
                    //  1: 걷는다 ( 목을 내밀고 )
                    //  2: 걷는다 ( 목을 넣고 )

                    o->LifeTime = rand()%50+50;
                    o->AI = 1;
                    o->Gravity = -30.f;
                    SetAction ( o, o->AI );

					Vector ( 0.f, 0.f, 0.f, o->Direction );
                    Vector ( 1.f, 1.f, 1.f, o->Light );
                    VectorCopy ( o->Position, o->StartPosition );
                    VectorCopy ( o->Position, o->HeadTargetAngle );

                    vec3_t  p, Position, Angle;

                    Vector ( 0.f, -150.f-rand()%150, 0.f, p );
                    Vector ( 0.f, 0.f, 0.f, Angle );
                    Angle[2] = 180.f + rand()%120-60.f;

			        AngleMatrix ( Angle, Matrix );
			        VectorRotate ( p, Matrix, Position );
			        VectorAdd ( o->HeadTargetAngle, Position, o->StartPosition );
                    VectorCopy ( o->StartPosition, o->Position );
                }
                break;
#endif
            case BITMAP_SHOTGUN:
                {
                    o->LifeTime = 10;
				    o->Velocity = 1.f;
				    Vector(0.f,-30.f,0.f,o->Direction);
				    AngleMatrix(o->Angle,Matrix);
				    Vector(0.f,-20.f,50.f,p1);
				    VectorRotate(p1,Matrix,p2);
				    VectorAdd(o->Position,p2,o->Position);

                    //  파편이 나간다.
                    vec3_t  Angle, Pos, p3;
				    Vector(-20.f,-20.f,60.f,p1);
				    VectorRotate(p1,Matrix,p2);
				    VectorAdd(Position,p2,Pos);
                    VectorCopy ( o->Angle, Angle );
                    for ( int i=0; i<20; ++i )
                    {
                        Angle[0] = o->Angle[0] + rand()%20+5;
                        Angle[1] += i*18;
                        CreateJoint ( BITMAP_JOINT_SPARK, Pos, Pos, Angle, 1 );
                    }

				    Vector( 30.f,-20.f,60.f,p3);
				    VectorRotate(p3,Matrix,p2);
				    VectorAdd(Position,p2,Pos);
                    VectorCopy ( o->Angle, Angle );
#ifdef _VS2008PORTING
                    for ( int i=0; i<20; ++i )
#else // _VS2008PORTING
                    for ( i=0; i<20; ++i )
#endif // _VS2008PORTING
                    {
                        Angle[0] = o->Angle[0] + rand()%20+5;
                        Angle[1] += i*18;
                        CreateJoint ( BITMAP_JOINT_SPARK, Pos, Pos, Angle, 1 );
                    }
                }
                break;
            case MODEL_GATE:    //  블러드캐슬 성문.
            case MODEL_GATE+1:
    			Vector(0.f,(float)(rand()%128+64)*0.1f,0.f,p1);
				o->Position[2] += 50.f;
				o->LifeTime = rand()%16+32;
				o->Scale    = (float)(rand()%4+8)*0.1f;
      			o->Angle[2] = (float)(rand()%360);
				AngleMatrix(o->Angle,Matrix);
				VectorRotate(p1,Matrix,o->Direction);
    			o->Gravity = (float)(rand()%5+2);
                if ( o->Type==MODEL_GATE && o->SubType==0 )
                {
                    o->SubType = 1;
                    o->Gravity += (float)(rand()%5);
                }
                else if ( o->SubType==2 )
                {
                    o->Scale *= 0.6f;
                }

                o->Angle[0] = (float)(rand()%360);
                o->Angle[1] = (float)(rand()%360);
                o->Angle[2] = (float)(rand()%360);
                break;

            case MODEL_STONE_COFFIN:    //  블러드캐슬 성자의 석상.
            case MODEL_STONE_COFFIN+1:
    			Vector(0.f,(float)(rand()%128+32)*0.1f,0.f,p1);
				o->Position[2] += 50.f;
				o->LifeTime = rand()%16+32;
				o->Scale    = (float)(rand()%4+8)*0.1f;
      			o->Angle[2] = (float)(rand()%360);
				AngleMatrix(o->Angle,Matrix);
				VectorRotate(p1,Matrix,o->Direction);
    			o->Gravity = (float)(rand()%5+2);
                if ( o->Type==MODEL_STONE_COFFIN+1 && o->SubType==0 )
                {
                    o->SubType = 1;
                    o->Gravity += (float)(rand()%5);
                }

                o->Angle[0] = (float)(rand()%360);
                o->Angle[1] = (float)(rand()%360);
                o->Angle[2] = (float)(rand()%360);
                break;
            case MODEL_SHINE:
                if ( o->SubType==0 )
                {
                    vec3_t Pos;
                    o->LifeTime = 50;
                    for ( int j=0; j<10; ++j )
                    {
		                Pos[0] = o->Position[0]+(j-5)*12.f-30.f;
		                Pos[1] = o->Position[1]+(j-5)*12.f+30.f;
		                Pos[2] = o->Position[2]-300.f;

                        CreateJoint ( BITMAP_FLARE, Pos, o->Position, o->Angle, 16, o->Owner, 120.f );
                    }
                }
                break;
            case MODEL_BLIZZARD:
				if ( o->SubType==0 || o->SubType==2 )
                {
					o->BlendMesh = -2;
                    o->LifeTime = rand()%15+15;
                    o->Gravity  = -20.f;
                    o->Velocity = (float)(rand()%360);
					Vector ( 0.f, 0.f, 0.f, o->Light );

					int rangeX, rangeY, rangeZ;

					rangeX = 300; rangeY = 150; rangeZ = 700;
					o->Scale = 0.5f;
					o->Gravity -= rand()%30+10;

                    o->Position[0] = o->Position[0] + rand()%rangeX-rangeY;
                    o->Position[1] = o->Position[1] + rand()%rangeX-rangeY;
                    o->Position[2] = o->Position[2] + 600.f;
                    o->Position[0] += 100.f;

                    VectorCopy(o->Position,o->StartPosition);

                    if ( o->SubType==2 )
                    {
                        CreateJoint ( BITMAP_JOINT_THUNDER+1, o->Position, o->Position, o->Angle, 4, o, 60.f+rand()%10 );
                        if ( rand()%2==0 )
                            CreateJoint ( BITMAP_JOINT_THUNDER+1, o->Position, o->Position, o->Angle, 4, o, 60.f+rand()%10 );
                    }
                }
				else if ( o->SubType==1 )
				{
					o->BlendMesh = -2;
					o->LifeTime = 20;
					o->Velocity = 0.f;
				}
                break;

			case MODEL_ARROW_DRILL :
				if ( o->SubType==0 || o->SubType==2 )
				{
					o->LifeTime		= 30;
     				o->BlendMesh	= -2;
					o->Scale        = 1.f;
					o->Gravity      = -10.f;
					o->Position[2] += 130.f;

					if(o->SubType!=0)
					{
						CreateEffect(MODEL_PIERCING,o->Position,o->Angle,o->Light,0,o);
						o->AttackPoint[0] = 0;
						o->Kind = 1;
					}

					Vector(0.f,-70.f,0.f,o->Direction);
					VectorCopy(o->Position,o->EyeLeft);
					CreateJoint(BITMAP_JOINT_ENERGY,o->Position,o->Position,o->Angle,5,o,100.f);

					o->Weapon = CharacterMachine->PacketSerial;
				}
				break;

			case MODEL_COMBO:
				o->LifeTime = 20;
				o->Gravity  = 0.1f;
				o->BlendMesh = -2;
				o->BlendMeshLight = 1.f;
				Vector ( 0.f, 0.f, 0.f, o->Angle );
				VectorCopy ( o->Position, o->StartPosition );
				o->Position[2] += 50.f;

				if ( o->SubType==0 )
				{
					for ( int j=0; j<60; ++j )
					{
						CreateJoint ( BITMAP_LIGHT, o->Position, o->Position, o->Angle, 0, NULL, (float)(rand()%40+70) );
					}
				}
				break;
			case MODEL_AIR_FORCE:	    //  공기 저항 효과.
                if ( o->SubType==0 )
                {
				    o->LifeTime = 15;
				    o->BlendMeshLight = 1.0f;
                    o->Scale = 0.6f;
                }
                else if ( o->SubType==1 )
                {
				    o->LifeTime = 20;
				    o->BlendMeshLight = 1.0f;
                    o->Scale = 1.2f;
                }
				break;
			case MODEL_WAVES:
				o->LifeTime = 20;
				o->Gravity  = 0.1f;
				o->BlendMesh = -2;
				o->BlendMeshLight = 1.f;
				Vector ( 0.f, 0.f, 0.f, o->Angle );
				VectorCopy ( o->Position, o->StartPosition );
				o->Position[2] += 50.f;

				if ( o->SubType==0 )
				{
					for ( int j=0; j<60; ++j )
					{
						CreateJoint ( BITMAP_LIGHT, o->Position, o->Position, o->Angle, 0, NULL, (float)(rand()%40+70) );
					}
				}
                else if ( o->SubType==1 )   //  스트롱 피어.
                {
    				o->LifeTime = 15;
                    o->RenderType = RENDER_NODEPTH;
                    o->Position[2] += 80.f;
                    o->Scale    = 0.1f+rand()%50/100.f;
                    o->Gravity  = 0.01f;
                 
                    o->Angle[0] = 90.f;
                    o->Angle[2] = Angle[2];

					for ( int j=0; j<2; ++j )
					{
						CreateJoint ( BITMAP_PIERCING, o->Position, o->Position, o->Angle, 0, NULL, (float)(rand()%40+70) );
					}

                    o->Position[0] += rand()%100-50.f;
                    o->Position[1] += rand()%100-50.f;
                }
                else if ( o->SubType==2 || o->SubType==3 || o->SubType==4 )
                {
    				o->LifeTime = 15;
                    o->PKKey    = -1;
                    o->Scale    = PKKey*0.05f;
                    o->Gravity  = 0.01f;
    				o->Position[2] -= 50.f;
                 
                    o->Angle[0] = 90.f;
                    o->Angle[2] = Angle[2];
                }
                else if ( o->SubType==5 || o->SubType==6 )
                {
    				o->LifeTime = 10;
                    o->PKKey    = -1;
                    o->Scale    = PKKey*0.05f;
                    o->Gravity  = 0.08f;
    				o->Position[2] -= 50.f;
                 
                    o->Angle[0] = -90.f;
                    o->Angle[2] = Angle[2];
                }
				break;
            case MODEL_PIERCING2:     //  스트롱 피어.
				o->Scale = 2.0f;
				if(o->SubType==1)
					o->LifeTime = 6;
#ifdef PJH_SEASON4_SPRITE_NEW_SKILL_MULTI_SHOT
				else
				if(o->SubType==2)
				{
					o->Scale = 3.0f;
					o->LifeTime = 10;
				}
#endif //PJH_SEASON4_SPRITE_NEW_SKILL_MULTI_SHOT
				else
					o->LifeTime = 10;
                o->BlendMesh = -2;
                Vector ( 0.f, -60.f, 0.f, o->Direction );
				VectorCopy ( o->Position, o->StartPosition );
				o->Position[2] += 130.f;
                break;
#ifdef LDK_ADD_EG_MONSTER_DEASULER
			case MODEL_DEASULER:
				if ( o->SubType==0 )
                {
								const int	TOTAL_LIFETIME = 55;
								vec3_t		v3PosProcess01, v3PosProcessFinal, 
											v3DirModify, v3PosModify,
											v3PosTargetModify;
								
								o->ExtState		= TOTAL_LIFETIME;			// TotalLife 미사용중인 임의의 변수 사용.
								o->LifeTime     = TOTAL_LIFETIME;
								o->Gravity      = 0.0f;
								o->HiddenMesh   = 1;
								o->Distance		= 0.0f;
								o->Alpha		= 1.0f;
								o->ChromeEnable = false;	// Owner Bone에 Attach하여 렌더링 할것인지?
								

								// ㄱ. 움직임 각도 백업. 모델의 각도 + 자신의 상대 각도
								Vector ( o->Angle[0], o->Angle[1], o->Angle[2], o->HeadAngle );	
								
								VectorCopy ( o->Position, o->StartPosition );	// ㄴ. 시작 위치점 백업.
								
								// 1. 방향
								VectorSubtract( o->Light, o->Position, o->Direction );
								
								// 2. 기본 거리 값 지정
								o->Distance = sqrt(o->Direction[0] * o->Direction[0] +
									o->Direction[1] * o->Direction[1] + 
									o->Direction[2] * o->Direction[2]);

								VectorDivFSelf( o->Direction, o->Distance );

								// 2.5. 두케릭간 실거리 검사 : 두 케릭 높이를 동일하게 맞추고 실거리 검사.
								float	fDistanceResult = 0;
								vec3_t	v3DirResult;
								
								// 3-0. Dir을 뒤로 늘려야하는데 목표점이 시작점보다 아래에 있으므로 땅밑으로 꺼지는 문제를
								//      해결하기 위해 시작점의 높이값을 목표점이랑 동일하게 해준다.
								
								// 3-0- 방식 1.
								vec3_t v3PosModify02;
								float	fTotalDist = 1700.0f, fFirstDist = 0.0f, fRateFirstDist = 0.0f;
								VectorCopy(o->Owner->Position, v3PosModify02);
								VectorCopy( o->Position,v3PosModify );
								
								v3PosModify02[2] = o->Light[2];
								v3PosModify[2] = o->Light[2];
								VectorCopy( o->Light, v3PosTargetModify );
								v3PosModify[2] = v3PosModify[2] + 100.0f;
								v3PosModify02[2] = v3PosModify02[2] + 100.0f;
								v3PosTargetModify[2] = v3PosTargetModify[2] + 100.0f;
								VectorDistNormalize( v3PosModify02,v3PosTargetModify,v3DirModify);
								
								// 3-1. 첫번째 위치값
								fDistanceResult = o->Distance * 0.3f;
								fFirstDist = fDistanceResult;
								VectorMulF( v3DirModify, fDistanceResult, v3DirResult );
								VectorAdd( v3PosModify, v3DirResult, v3PosProcess01 );
								
								// 3-2. 두번째 위치값
								fDistanceResult = fTotalDist;
								VectorMulF( v3DirModify, fDistanceResult, v3DirResult );
								VectorAdd( v3PosModify, v3DirResult, v3PosProcessFinal );
								
								fRateFirstDist = fFirstDist / fTotalDist;
								
								o->m_Interpolates.ClearContainer();
									
								fRateFirstDist = fFirstDist / fTotalDist;
								
								// 보간컨테이너에 각도 적재 ----------------------------------------------- 
								CInterpolateContainer::INTERPOLATE_FACTOR	InsertFactor;
								InsertFactor.fRateStart	= 0.0f;			// Start
								InsertFactor.fRateEnd	= fRateFirstDist;			// 01 Ready
								Vector(0.0f, 0.0f, 0.0f, InsertFactor.v3Start);
								Vector(0.0f, 90.0f, 0.0f, InsertFactor.v3End);
								o->m_Interpolates.m_vecInterpolatesAngle.push_back( InsertFactor );		// 1#

								InsertFactor.fRateStart	= fRateFirstDist;			// 02 First Final
								InsertFactor.fRateEnd	= 1.01f;			
								Vector(0.0f, 90.0f, 0.0f, InsertFactor.v3Start);
								Vector(0.0f, 90.0f, 2560.0f, InsertFactor.v3End);
								//Vector(90.0f, 0.0f, 1000.0f, InsertFactor.v3End);
								o->m_Interpolates.m_vecInterpolatesAngle.push_back( InsertFactor );		// 2#
								// 보간컨테이너에 각도 적재 ----------------------------------------------- 
								
								
								// 보간컨테이너에 위치 적재 ----------------------------------------------- 
								InsertFactor.fRateStart	= 0.0f;
								InsertFactor.fRateEnd	= fRateFirstDist;
								VectorCopy(o->Position, InsertFactor.v3Start);
								VectorCopy(v3PosProcess01, InsertFactor.v3End);
								o->m_Interpolates.m_vecInterpolatesPos.push_back( InsertFactor );		// 2#

 								InsertFactor.fRateStart	= fRateFirstDist;
 								InsertFactor.fRateEnd	= 1.01f;
 								VectorCopy(v3PosProcess01, InsertFactor.v3Start);
 								VectorCopy(v3PosProcessFinal, InsertFactor.v3End);
 								o->m_Interpolates.m_vecInterpolatesPos.push_back( InsertFactor );		// 2#
								// 보간컨테이너에 위치 적재 ----------------------------------------------- 

								// 초기값 설정
								o->m_Interpolates.GetAngleCurrent(o->Angle, 0.0f);		// 각도
								o->m_Interpolates.GetPosCurrent(o->Position, 0.0f);		// 위치
					/*
                    o->LifeTime     = 60;
					o->Alpha = o->Scale;
                    o->Scale        = 1.4f;
					o->BlendMesh = -1;
                    Vector ( 1.f, 1.f, 1.f, o->Light );
                    Vector ( 0.f, -40.f, 0.f, o->Direction );
				    VectorCopy ( o->Position, o->StartPosition );
                    Vector ( 0.f, 90.f, Angle[2], o->Angle );
                    VectorCopy ( o->Angle, o->HeadAngle );
					*/
				}
				break;
#endif //LDK_ADD_EG_MONSTER_DEASULER
			case MODEL_DEATH_SPI_SKILL:
				if ( o->SubType==0 )
                {
                    o->LifeTime     = 30;
                    o->Gravity      = 1.0f;
                    o->Velocity     = 10.f;
                    o->HiddenMesh   = 1;
                    o->Scale        = 0.3f;
					o->BlendMesh = -1;
					o->Alpha = 0.8f;
                    Vector ( 1.f, 1.f, 1.f, o->Light );
                    Vector ( 0.f, -26.f, 0.f, o->Direction );
				    VectorCopy ( Light, o->StartPosition );
                    VectorCopy ( o->Angle, o->HeadAngle );
                    Vector ( 0.f, 0.f, Angle[2], o->Angle );
				}
                else if ( o->SubType==1 )
                {
//					o->LifeTime     = 20;
                    o->LifeTime     = o->Owner->LifeTime;
                    o->HiddenMesh   = 1;
                    o->Scale        = 0.2f;
					o->BlendMesh = -1;
                    o->Alpha        = (float)((20-o->LifeTime)/5.f);
					Vector ( 0.3f, 0.4f, 1.f, o->Light );
                    Vector ( 0.f, 0.f, 0.f, o->Direction );

					CreateParticle(BITMAP_FIRE+2,o->Position, o->Angle, o->Light,0);
//                    CreateParticle ( BITMAP_FIRE+1, o->Position, o->Angle, o->Light, 0, 1.f, o );
                }
				break;
            case MODEL_PIER_PART:   //  롱피어 어택.
                if ( o->SubType==0 )
                {
                    o->LifeTime     = 20;
                    o->Gravity      = 2.f;
                    o->Velocity     = 10.f;
                    o->HiddenMesh   = 1;
                    o->Scale        = 1.2f;
                    Vector ( 1.f, 1.f, 1.f, o->Light );
                    Vector ( 0.f, -26.f, 0.f, o->Direction );
				    VectorCopy ( Light, o->StartPosition );
                    VectorCopy ( o->Angle, o->HeadAngle );
                    Vector ( 0.f, 0.f, Angle[2], o->Angle );
                }
                else if ( o->SubType==1 )
                {
                    o->LifeTime     = o->Owner->LifeTime;
                    o->HiddenMesh   = 0;
                    o->Scale        = 0.5f;
                    o->Alpha        = (float)((20-o->LifeTime)/5.f);
                    Vector ( 0.f, 0.f, 0.f, o->Direction );

                    CreateParticle ( BITMAP_FIRE+1, o->Position, o->Angle, o->Light, 0, 1.f, o );
                }
                else if ( o->SubType==2 )
                {
                    o->LifeTime  = 20;
                    o->Velocity  = 50.f;
                    o->HiddenMesh= -2;
                    o->Position[2] -= 20.f;

                    Vector ( 0.f, -40.f, 0.f, o->Direction );
                    VectorCopy ( o->Angle, o->HeadAngle );
                    Vector ( 0.f, 0.f, Angle[2], o->Angle );
                }
                break;

            case BITMAP_FLARE_FORCE :   
                if ( o->SubType==0 )    //  선더스트라이크.
                {
                    o->LifeTime = 0;
                    if ( o->Owner!=NULL )
                    {
                        CreateJoint ( BITMAP_FLARE_FORCE, Position, Position, Angle, 1, o->Owner, 100.f );
                        CreateJoint ( BITMAP_FLARE_FORCE, Position, Position, Angle, 0, o->Owner, 250.f );
                        CreateJoint ( BITMAP_FLARE_FORCE, Position, Position, Angle, 2, o->Owner, 100.f );
                        CreateJoint ( BITMAP_FLARE_FORCE, Position, Position, Angle, 3, o->Owner, 100.f );
                        CreateJoint ( BITMAP_FLARE_FORCE, Position, Position, Angle, 4, o->Owner, 100.f );
                    }
                }
                else if ( o->SubType==1 )   //  크리티컬 데미지 확률 증가.
                {
                    CreateJoint ( BITMAP_FLARE_FORCE, o->Position, o->Position, o->Angle, 5, o->Owner, 20.f, PKKey, SkillIndex );
                    CreateJoint ( BITMAP_FLARE_FORCE, o->Position, o->Position, o->Angle, 6, o->Owner, 20.f, PKKey, SkillIndex );
                    CreateJoint ( BITMAP_FLARE_FORCE, o->Position, o->Position, o->Angle, 7, o->Owner, 20.f, PKKey, SkillIndex );
                }
				//^ 펜릴 이펙트 관련
				else if(o->SubType >= 2 && o->SubType <= 4)	// 펜릴 스킬 플라즈마 스톰 이펙트(몬스터가 없을때)
				{	// 8 : 블랙 9 : 빨강 10 : 블루
					CreateJoint(BITMAP_FLARE_FORCE, Position, Position, Angle, 8+(o->SubType-2), o->Owner, 100.f); // 8, 9, 10
					CreateJoint(BITMAP_FLARE_FORCE, Position, Position, Angle, 0, o->Owner, 150.f);
				}
				else if(o->SubType >=5 && o->SubType <= 7)
				{
					CreateJoint(BITMAP_FLARE_FORCE, Position, Position, Angle, 11+(o->SubType-5), o->Owner, 100.f); // 12, 13, 14
					CreateJoint(BITMAP_FLARE_FORCE, Position, Position, Angle, 1, o->Owner, 100.f);
				}
                break;

            case MODEL_DARKLORD_SKILL:
                {
                    o->LifeTime = 10;
                    o->Scale = 0.2f;
                    o->Velocity = 0.1f;

                    if ( o->SubType<=1 )
                    {
                        float angle = 45.f+(-90.f*o->SubType);
                    
                        if ( rand()%2==0 )
                        {
                            if ( o->SubType )
                            {
                                angle = 45.f-90.f;
                            }
                            else
                            {
                                angle = 45.f;
                            }
                        }                    
                        Vector ( 45.f, angle, 0.f, o->Angle );
                    }
                    else if ( o->SubType==2 )
                    {
                        o->LifeTime = 12;
                        o->Velocity = 0.4f;
                    }
                }
                break;

            case MODEL_GROUND_STONE:
                {
					int TargetX = (int)(o->Position[0]/TERRAIN_SCALE);
					int TargetY = (int)(o->Position[1]/TERRAIN_SCALE);

                    WORD wall = TerrainWall[TERRAIN_INDEX(TargetX,TargetY)];
					
                    if ( (wall&TW_NOMOVE)!=TW_NOMOVE && (wall&TW_NOGROUND)!=TW_NOGROUND && (wall&TW_WATER)!=TW_WATER )
                    {
                        o->LifeTime     = 40;
                        o->Scale        = 1.2f+rand()%30/100.f;
                        o->Velocity     = 0.3f;
                        o->Angle[2]     = (float)(rand()%360);
                        SetAction ( o, 0 );
                    }
                    else
                    {
                        o->Live = false;
                    }
                }
                break;
            case MODEL_GROUND_STONE2:
                {
					int TargetX = (int)(o->Position[0]/TERRAIN_SCALE);
					int TargetY = (int)(o->Position[1]/TERRAIN_SCALE);

                    WORD wall = TerrainWall[TERRAIN_INDEX(TargetX,TargetY)];

                    if ( (wall&TW_NOMOVE)!=TW_NOMOVE && (wall&TW_NOGROUND)!=TW_NOGROUND && (wall&TW_WATER)!=TW_WATER )
                    {
                        o->LifeTime     = 40;
                        o->Scale        = 1.f+rand()%30/100.f;
                        o->Velocity     = 0.3f;
                        o->Angle[2]     = (float)(rand()%360);
                        SetAction ( o, 0 );
                        
                    }
                    else
                    {
                        o->Live = false;
                    }
                }
                break;
#ifdef PJH_SEASON4_SPRITE_NEW_SKILL_RECOVER
			case BITMAP_TWLIGHT:
				{
					if( o->SubType==0 || o->SubType==1 || o->SubType==2 )
					{
						o->LifeTime = 80;
						o->Scale    = 8.f;
					}
					else if( o->SubType == 3 )	// 마력증대 (마법사) 스킬에서 사용
					{
						o->LifeTime = 30;
						o->Alpha = 0.0f;
						o->PKKey = 0;		// Color비율 (매 프레임 바뀜)
						// o->light : 현재Color
						// o->EyeRight : 기준Color
						VectorCopy( o->Light, o->EyeRight );
					}
				}
				break;
#endif //PJH_SEASON4_SPRITE_NEW_SKILL_RECOVER
            case BITMAP_SHOCK_WAVE:
				// 서브타입이 있을때는 Scale은 공통으로 쓰지 말자.. (인자로 받을수도 있기 때문에)
#ifdef KJH_ADD_SKILL_SWELL_OF_MAGICPOWER
				// 다크로드(어스퀘이커 마법)
				if( o->SubType == 0 )
				{
					o->LifeTime = 30;	
					o->Scale    = 20.f;
				}
                else if ( o->SubType==1 )
#else // KJH_ADD_SKILL_SWELL_OF_MAGICPOWER
				o->LifeTime = 30;	
				o->Scale    = 20.f; 

				if ( o->SubType==1 )
#endif // KJH_ADD_SKILL_SWELL_OF_MAGICPOWER
                {
                    o->LifeTime = 20;
                    o->Scale = (rand()%10+10.f)/10.f;
                }
                else if ( o->SubType==2 )
                {
                    o->LifeTime = 20;
                    o->Scale = (rand()%10+10.f)/10.f;
                }
                else if ( o->SubType==3 )
                {
                    o->LifeTime = 15;
                    o->Scale    = 1.f;
                }
                else if ( o->SubType==4 )
                {
                    o->LifeTime = 10;
                    o->Scale    = (rand()%6+6.f)/10.f;
                }
                else if ( o->SubType==5 )
                {
                    o->LifeTime = 20;
                    o->Scale    = 9.f;
                }
                else if ( o->SubType==6 )
                {
                    o->LifeTime = 50;
                    o->Scale    = 9.f;
                }
                else if ( o->SubType==7 )
                {
                    o->LifeTime = 10;
                    o->Scale    = 1.f;
                }
                else if ( o->SubType==8 )
                {
                    o->LifeTime = 10;
                    o->Scale = (rand()%10+10.f)/5.f;
                }
				else if ( o->SubType==9 )
				{
					o->LifeTime = 5;
					o->Scale = 5.f;
				}
				else if ( o->SubType==10 )
                {
                    o->LifeTime = 1;
                    o->Scale    = 2.f;

					o->Light[0] *= 2.3f;
					o->Light[1] *= 2.3f;
					o->Light[2] *= 2.3f;
                }
				else if( o->SubType == 11 )
				{
					o->LifeTime = 20;
                    o->Scale = (rand()%10+10.f)/10.f;
				}
				else if( o->SubType == 12 )
				{
					o->LifeTime = 10;
                    o->Scale = Scale;
				}
#ifdef PJH_GIANT_MAMUD
				else if(o->SubType == 13)
				{
                    o->LifeTime = 40;
                    o->Scale = (rand()%10+20.f)/7.f;
				}
#endif //#ifdef PJH_GIANT_MAMUD
#ifdef KJH_ADD_SKILL_SWELL_OF_MAGICPOWER			// 마력증대 (마검사)
				else if( o->SubType == 14 )
				{
					o->LifeTime = 30;
					o->Alpha = 0.0f;
					o->PKKey = 0;		// Color비율 (매 프레임 바뀜)
					// o->light : 현재Color
					// o->EyeRight : 기준Color
					VectorCopy( o->Light, o->EyeRight );
				}
#endif // KJH_ADD_SKILL_SWELL_OF_MAGICPOWER

                break;

//-----------------------------------------------------------------------------------------------------

#ifdef CSK_RAKLION_BOSS
			case BITMAP_DAMAGE_01_MONO:
				if(SubType == 0)
				{
					o->LifeTime = 20;
					o->Scale = Scale;
				}
#ifdef YDG_ADD_SKILL_LIGHTNING_SHOCK
				else if(o->SubType == 1)
				{
					o->LifeTime = 10;
					o->Scale = 0.1f;
					o->Alpha = 1.0f;
				}
#endif	// YDG_ADD_SKILL_LIGHTNING_SHOCK
				break;
#endif // CSK_RAKLION_BOSS

            case BITMAP_FLARE:
				if (SubType == 1 || SubType == 2)
				{
					o->LifeTime = 30;
				}
				else
#ifdef PJH_SEASON4_SPRITE_NEW_SKILL_RECOVER
				if(SubType == 3)
				{
					o->LifeTime = 60;
				}
				else
#endif //PJH_SEASON4_SPRITE_NEW_SKILL_RECOVER
				{
					o->LifeTime = 30;
				}
                break;

//-----------------------------------------------------------------------------------------------------

            case BITMAP_JOINT_THUNDER+1:
                o->LifeTime = 10;
                
                if ( o->Owner!=NULL && o->SubType==0 )
                {
            		float   Matrix[3][4];
                    vec3_t  p, p2;

                    Vector ( -25.f, -80.f, 0.f, p );
				    AngleMatrix ( o->Owner->Angle, Matrix );
				    VectorRotate ( p, Matrix, p2 );
				    VectorAdd ( p2, o->Position, o->Position );
                }
                break;

//-----------------------------------------------------------------------------------------------------

			case MODEL_CUNDUN_DRAGON_HEAD:
				{
					o->LifeTime		= 30;
     				o->BlendMesh	= -2;
					o->Scale        = 1.0f;
					Vector(0.5,0.5,0.5,o->Light);
					o->Alpha = 0;
					vec3_t Position;
					VectorCopy(o->Position, Position);
					o->Position[2] += 100;

					float fAngle = float(rand()%360);
					float fDistance = float(rand()%600+200);
					Position[0] = o->Position[0] + sinf(fAngle)*fDistance;
					Position[1] = o->Position[1] + cosf(fAngle)*fDistance;
					Position[2] = 0;
					CreateJoint(BITMAP_JOINT_SPIRIT2,Position,Position,o->Angle,18,o,100.f,0,0);
					o->Angle[0] = -30;
					o->Angle[1] = 0;
					o->Angle[2] = (float)( rand()%360 );
				}
				break;
			case MODEL_CUNDUN_PHOENIX:
				{
					o->Velocity = 0.34f;
					o->LifeTime		= 20;
     				o->BlendMesh	= -2;
					o->Scale        = 0.7f;
					Vector(0.5,0.5,0.5,o->Light);
					o->Alpha = 0;
					vec3_t Position, Angle;
					VectorCopy(o->Position, Position);
					//o->Position[2] += 100;
					Vector(0,0,o->Angle[2]*Q_PI/180.0f,Angle);
					CreateJoint(BITMAP_JOINT_SPIRIT,Position,Position,Angle,15,o,100.f,0,0);
//					o->Angle[2] -= 180;
					o->AnimationFrame = float(rand() % 5);
				}
				break;
			case MODEL_CUNDUN_GHOST:
				o->Velocity = 0.08f;
				o->LifeTime		= 200;
     			o->BlendMesh	= -2;
				o->Scale        = 1.80f;
//				o->Scale        = 1.88f;
				Vector(0.5,0.5,0.5,o->Light);
				break;
			case MODEL_CUNDUN_SKILL:
				switch(o->SubType)
				{
				case 0:	// 공격1
					o->LifeTime		= 30;
					break;
				case 1:	// 공격2
					o->LifeTime		= 30;
					break;
				case 2:	// 사망시 유령소환
					o->LifeTime		= 40;
					o->PKKey = 0;
					CreateEffect ( MODEL_CUNDUN_GHOST, o->Position, o->Angle, o->Light, 0, o );
					break;
				};
				break;

            case BATTLE_CASTLE_WALL1:   //  성벽. ( 부서지는 효과를 위함.
            case BATTLE_CASTLE_WALL2:
            case BATTLE_CASTLE_WALL3:
            case BATTLE_CASTLE_WALL4:
                o->LifeTime = 2;
                break;

			case MODEL_HALLOWEEN_EX:
				{
					if(o->SubType == 0)
					{
						o->LifeTime = 0;
						int iEffectType;
						for(int i=0; i<24; ++i)
						{
							//iEffectType = rand()%6 + MODEL_HALLOWEEN_CANDY_BLUE;
							iEffectType = rand()%8;
							switch(iEffectType) 
							{
							case 0: iEffectType = MODEL_HALLOWEEN_CANDY_BLUE;	break;
							case 1: iEffectType = MODEL_HALLOWEEN_CANDY_ORANGE;	break;
							case 2: iEffectType = MODEL_HALLOWEEN_CANDY_YELLOW;	break;
							case 3: iEffectType = MODEL_HALLOWEEN_CANDY_RED;	break;
							case 4: iEffectType = MODEL_HALLOWEEN_CANDY_HOBAK;	break;
							case 5: iEffectType = MODEL_HALLOWEEN_CANDY_STAR;	break;
							case 6: iEffectType = MODEL_HALLOWEEN_CANDY_HOBAK;	break;
							case 7: iEffectType = MODEL_HALLOWEEN_CANDY_STAR;	break;
							}
							//iEffectType = MODEL_HALLOWEEN_CANDY_STAR;
							CreateEffect(iEffectType, o->Position, o->Angle, o->Light, 0);
						}
					}
				}
				break;
			case MODEL_HALLOWEEN_CANDY_BLUE:
			case MODEL_HALLOWEEN_CANDY_ORANGE:
			case MODEL_HALLOWEEN_CANDY_YELLOW:
			case MODEL_HALLOWEEN_CANDY_RED:
			case MODEL_HALLOWEEN_CANDY_HOBAK:
			case MODEL_HALLOWEEN_CANDY_STAR:
				{
					if(o->SubType == 0)
					{
						if(o->Type == MODEL_HALLOWEEN_CANDY_HOBAK)
							o->Scale = 2.0f + (rand()%10-5)*0.02f;
						else if(o->Type == MODEL_HALLOWEEN_CANDY_STAR)
							o->Scale = 2.0f + (rand()%10-5)*0.02f;
						else
							o->Scale = 0.6f + (rand()%10-5)*0.02f;
						o->LifeTime = rand()%10+50;
						o->Angle[0] = (float)(rand()%360);
						o->Angle[1] = (float)(rand()%360);
						o->Angle[2] = (float)(rand()%360);
						AngleMatrix(o->Angle,Matrix);
						o->Gravity = (float)(rand()%10+10);
						vec3_t p;
						Vector((float)(rand()%60-30)*0.1f,(float)(rand()%60-30)*0.1f,0.f,p);
						VectorScale(p,2.0f,p);
						VectorRotate(p, Matrix, o->Direction);
						o->m_iAnimation = rand()%3;
					}
					else if(o->SubType == 1)
					{
						o->Scale = 2.0f + (rand()%10-5)*0.02f;
						o->LifeTime = rand()%10+40;
						o->Angle[0] = (float)(rand()%360);
						o->Angle[1] = (float)(rand()%360);
						o->Angle[2] = (float)(rand()%360);
						AngleMatrix(o->Angle,Matrix);
						o->Gravity = (float)(rand()%10+10);
						vec3_t p;
						Vector((float)(rand()%60-30)*0.1f,(float)(rand()%60-30)*0.1f,-1.f,p);
						VectorScale(p,1.5f,p);
						VectorRotate(p, Matrix, o->Direction);
						o->m_iAnimation = rand()%3;
					}
				}
				break;

			case MODEL_XMAS_EVENT_BOX:
			case MODEL_XMAS_EVENT_CANDY:
			case MODEL_XMAS_EVENT_TREE:
			case MODEL_XMAS_EVENT_SOCKS:
			{
				o->Scale = 0.7f + (rand()%10-5)*0.02f;
				if(o->Type == MODEL_XMAS_EVENT_BOX)
				{
					o->Scale += 0.3f;
				}
				o->LifeTime = rand()%10+50;
				o->Angle[0] = (float)(rand()%360);
				o->Angle[1] = (float)(rand()%360);
				o->Angle[2] = (float)(rand()%360);
				AngleMatrix(o->Angle,Matrix);
				o->Gravity = (float)(rand()%5+15);
				vec3_t vPos;
				Vector((float)(rand()%60-30)*0.1f,(float)(rand()%60-30)*0.1f,-1.f,vPos);
				VectorScale(vPos,1.5f,vPos);
				VectorRotate(vPos, Matrix, o->Direction);
				o->m_iAnimation = rand()%3;
			}
				break;
			case MODEL_XMAS_EVENT_ICEHEART:
			{
				o->Scale = 4.0f + (rand()%10-5)*0.02f;
				o->LifeTime = 100;
			}
				break;

			case MODEL_NEWYEARSDAY_EVENT_BEKSULKI:
			case MODEL_NEWYEARSDAY_EVENT_CANDY:
			case MODEL_NEWYEARSDAY_EVENT_MONEY:
			case MODEL_NEWYEARSDAY_EVENT_HOTPEPPER_GREEN:
			case MODEL_NEWYEARSDAY_EVENT_HOTPEPPER_RED:
			case MODEL_NEWYEARSDAY_EVENT_YUT:
			case MODEL_NEWYEARSDAY_EVENT_PIG:
			{
				if(o->Type == MODEL_NEWYEARSDAY_EVENT_HOTPEPPER_GREEN)
				{
					if(rand()%2 == 0)
						o->Type = MODEL_NEWYEARSDAY_EVENT_HOTPEPPER_RED;
				}
				else if(o->Type == MODEL_NEWYEARSDAY_EVENT_HOTPEPPER_RED)
				{
					if(rand()%2 == 0)
						o->Type = MODEL_NEWYEARSDAY_EVENT_HOTPEPPER_GREEN;
				}

				o->LifeTime = rand()%10+50;
				o->Scale = 1.6f + (rand()%10-5)*0.02f;
			
				if(o->Type == MODEL_NEWYEARSDAY_EVENT_BEKSULKI)
				{
					o->Scale = 2.5f + (rand()%10-5)*0.02f;	
				}
				if(o->Type == MODEL_NEWYEARSDAY_EVENT_CANDY)
				{
					o->Scale = 3.0f + (rand()%10-5)*0.02f;	
				}
 				else if(o->Type == MODEL_NEWYEARSDAY_EVENT_PIG)
 				{
 					o->Scale = 1.0f + (rand()%10-5)*0.02f;	
 				}

				o->Angle[0] = (float)(rand()%360);
				o->Angle[1] = (float)(rand()%360);
				o->Angle[2] = (float)(rand()%360);
				AngleMatrix(o->Angle, Matrix);
				o->Gravity = (float)(rand()%10+10);
				vec3_t p;
				Vector((float)(rand()%10-5)*0.1f, (float)(rand()%60-40)*0.1f, 0.0f, p);
				VectorScale(p, 1.3f, p);
				VectorRotate(p, Matrix, o->Direction);
				o->m_iAnimation = rand()%3;
			}
				break;
				
#ifdef PRUARIN_EVENT07_3COLORHARVEST
			case MODEL_MOONHARVEST_MOON:
				if(o->SubType == 0)
				{
					o->Alpha = 0.6f;
					o->LifeTime = 70;
					o->m_iAnimation = 0;
					VectorCopy(Position, o->Position);
				}
#ifdef CSK_RAKLION_BOSS
				else if(o->SubType == 1)
				{
					o->LifeTime = 50;
					VectorCopy(o->Angle, o->Direction);
					Vector(0.f, 0.f, 0.f, o->Angle);
				}
#endif // CSK_RAKLION_BOSS
#ifdef LDK_ADD_GAMBLERS_WEAPONS
				else if(o->SubType == 2)
				{
					o->LifeTime = 1;
					VectorCopy(Position, o->Position);
				}
#endif //LDK_ADD_GAMBLERS_WEAPONS
				break;
				
			case MODEL_MOONHARVEST_GAM:	
			case MODEL_MOONHARVEST_SONGPUEN1:	
			case MODEL_MOONHARVEST_SONGPUEN2:
				{
					o->LifeTime = rand()%10+50;
					if( o->Type == MODEL_MOONHARVEST_GAM )
					{
						o->Scale = 0.5f + (rand()%10-5)*0.02f;
					}
					else
					{
						o->Scale = 0.8f + (rand()%10-5)*0.02f;
					}				
					o->Angle[0] = (float)(rand()%360);
					o->Angle[1] = (float)(rand()%360);
					o->Angle[2] = (float)(rand()%360);
					AngleMatrix(o->Angle, Matrix);
					o->Gravity = (float)(rand()%10+10);
					vec3_t p;
					Vector((float)(rand()%10-5)*0.1f, (float)(rand()%60-30)*0.1f, 0.0f, p);
					VectorScale(p, 1.2f, p);
					VectorRotate(p, Matrix, o->Direction);
				}
				break;	
#endif // PRUARIN_EVENT07_3COLORHARVEST
				
            case MODEL_MONSTER01+77:
                if ( o->SubType==0 )   //  성벽에서 돌무더기가 떨어진다.
                {
                    o->LifeTime = 20;
                    o->HiddenMesh = 3;
                    o->Velocity   = 0.33f;
                    SetAction ( o, 2 );

                    Vector ( 0.f, 0.f, 0.f, o->Angle );
                }
                break;
			case MODEL_ARROW_TANKER_HIT:
				{
					o->LifeTime = 100;
                    o->Scale    = 1.0f;

                    Vector( 1.f, 1.f, 1.f, o->Light );

					VectorCopy ( Angle, o->m_vDeadPosition );
                    VectorCopy ( Position, o->StartPosition );

					if(o->SubType == 0)
					{
						Vector( 40.f, 0.f, 90.f, o->Angle );
						Vector(0.0f, -80.0f, 0.0f, o->Direction);
					}
					else if(o->SubType == 1)
					{
						Vector( 40.f, 0.f, -90.f, o->Angle );
						Vector(0.0f, -80.0f, 0.0f, o->Direction);
					}
					else if(o->SubType == 2)
					{
						Vector( 40.f, 0.f, 0.f, o->Angle );
						Vector(0.0f, -80.0f, 0.0f, o->Direction);
					}
				}
				break;
            case MODEL_FLY_BIG_STONE1:
                if ( o->SubType<=1 )
                {
                    o->LifeTime = 300;
                    o->Velocity = rand()%200/100.f+28.f;
                    o->Gravity  = 2.f;
                    o->Scale    = 2.f;
                    o->PKKey    = PKKey;
                    o->Kind     = Skill;
                    o->Skill    = SkillIndex;

                    switch ( o->PKKey )
                    {
                    case 1:
                        o->Velocity = 29.f;
                        VectorCopy ( Angle, o->m_vDeadPosition );
                        break;

                    default :
                        o->m_vDeadPosition[0] = Hero->Object.Position[0]+(rand()%16-8)*TERRAIN_SCALE;
                        if ( rand()%5==0 )
                            o->m_vDeadPosition[1] = 114*TERRAIN_SCALE;
                        else
                            o->m_vDeadPosition[1] = (rand()%33+98)*TERRAIN_SCALE;
                        o->m_vDeadPosition[2] = 100.f;
                        break;
                    }
                    Vector ( 0.f, 0.f, 0.f, o->Angle );
                    Vector ( 1.f, 1.f, 1.f, o->Light );
                    VectorCopy ( Position, o->StartPosition );

                    VectorSubtract ( o->m_vDeadPosition, o->StartPosition, o->Direction );
                    VectorScale ( o->Direction, 0.001f, o->Direction );
                    o->Direction[2] = 5.f;
                }
                else if ( o->SubType==2 )   //  떨어지는 돌.
                {
                    o->LifeTime = 50;
                    o->Scale    = rand()%8/20.f+0.7f;
                    o->Gravity  = 10.f;
                    o->Angle[0] = (float)( rand()%360 );

                    Vector ( 0.f, 0.f, 0.f, o->Direction );
                    Vector ( 0.f, 0.f, (float)(rand()%360), o->HeadAngle );
                }
                break;

            case MODEL_FLY_BIG_STONE2:
                if ( o->SubType<=1 )
                {
                    o->LifeTime = 300;
                    o->Velocity = rand()%200/100.f+28.f;
                    o->Gravity  = 2.f;
                    o->Scale    = 2.f;
                    o->PKKey    = PKKey;
                    o->Kind     = Skill;
                    o->Skill    = SkillIndex;

                    switch ( o->PKKey )
                    {
                    case 1:
                        o->Velocity = 29.f;
                        VectorCopy ( Angle, o->m_vDeadPosition );
                        break;

                    default :
                        o->m_vDeadPosition[0] = Hero->Object.Position[0]+(rand()%16-8)*TERRAIN_SCALE;
                        o->m_vDeadPosition[1] = (rand()%30+80)*TERRAIN_SCALE;
                        o->m_vDeadPosition[2] = 100.f;
                        break;
                    }
                    Vector ( 0.f, 0.f, 0.f, o->Angle );
                    Vector ( 1.f, 1.f, 1.f, o->Light );
                    VectorCopy ( Position, o->StartPosition );

                    VectorSubtract ( o->m_vDeadPosition, o->StartPosition, o->Direction );
                    VectorScale ( o->Direction, 0.001f, o->Direction );
                    o->Direction[2] = 5.f;
                }
                break;

            case MODEL_BIG_STONE_PART1:
            case MODEL_BIG_STONE_PART2:
            case MODEL_WALL_PART1:
            case MODEL_WALL_PART2:
			case MODEL_GOLEM_STONE:
                if ( o->Type>=MODEL_WALL_PART1 && o->Type<=MODEL_WALL_PART2 )
                {
    			    Vector ( 0.f, (float)(rand()%256+128)*0.1f, 0.f, p1 );
    				o->Scale    = (float)(rand()%2+7)*0.1f;
        			o->Gravity  = (float)(rand()%5+6);
                }
				else if (o->Type == MODEL_GOLEM_STONE )	//. 골렘 파편
				{
					Vector ( 0.f, (float)(rand()%128+32)*0.1f, 0.f, p1 );
					o->Scale    = (float)(rand()%2+10)*0.25f;
        			o->Gravity  = (float)(rand()%5+2);
				}
				else if ( (o->Type == MODEL_BIG_STONE_PART1 || o->Type == MODEL_BIG_STONE_PART2) && o->SubType == 2)
				{	//. 독골렘 파편
					Vector ( 0.f, (float)(rand()%128+32)*0.1f, 0.f, p1 );
					o->Scale    = 0.6F+(float)(rand()%2+4)*0.12f;
        			o->Gravity  = (float)(rand()%5+2);
				}
				else if ( o->Type == MODEL_BIG_STONE_PART2 && o->SubType == 3)
				{	//. 독골렘 공격용 파편
					o->LifeTime = rand()%16+32;
					o->Scale    = 1.2f+(float)(rand()%3+2)*0.12f;
					o->Direction[2] = -(rand()%5+20);
					o->Velocity = 1.8f;
					break;
				}
                else
                {
    			    Vector ( 0.f, (float)(rand()%128+128)*0.1f, 0.f, p1 );
    				o->Scale    = (float)(rand()%7+10)*0.1f;
        			o->Gravity  = (float)(rand()%5+2);
                }
				o->LifeTime = rand()%16+32;
      			o->Angle[2] = (float)(rand()%360);
				AngleMatrix ( o->Angle, Matrix );
				VectorRotate ( p1, Matrix, o->Direction );

                o->Angle[0] = (float)(rand()%360);
                o->Angle[1] = (float)(rand()%360);
                o->Angle[2] = (float)(rand()%360);

                o->Position[2] += 50.f;
                break;

            case MODEL_GATE_PART1:
            case MODEL_GATE_PART2:
            case MODEL_GATE_PART3:
    			Vector(0.f,(float)(rand()%128+64)*0.1f,0.f,p1);
				o->Position[2] += 50.f;
				o->LifeTime = rand()%16+32;
				o->Scale    = (float)(rand()%2+7)*0.1f;
      			o->Angle[2] = (float)(rand()%360);
				AngleMatrix(o->Angle,Matrix);
				VectorRotate(p1,Matrix,o->Direction);
    			o->Gravity = (float)(rand()%5+2);
                if ( o->Type==MODEL_GATE_PART1 && o->SubType==0 )
                {
                    o->SubType = 1;
                    o->Gravity += (float)(rand()%5);
                }
                else if ( o->SubType==2 )
                {
                    o->Scale *= 0.6f;
                }

                o->Angle[0] = (float)(rand()%360);
                o->Angle[1] = (float)(rand()%360);
                o->Angle[2] = (float)(rand()%360);
                break;

            case MODEL_AURORA:
                o->LifeTime = 100;
                o->Scale = PKKey/100.f;
                o->BlendMesh = 0;
                o->BlendMeshLight = 0.f;
                break;

            case MODEL_TOWER_GATE_PLANE:
                o->LifeTime = 100;
                o->BlendMeshLight = 0.3f;
                VectorCopy ( o->Position, o->StartPosition );
                break;

            case BITMAP_CRATER:
                if ( o->SubType==0 )
                {
                    o->LifeTime = 60;
                    o->StartPosition[0] = 4.5;
                    o->StartPosition[1] = 4.5;
                }
                else if ( o->SubType==1 )
                {
                    o->LifeTime = 30;
                    o->StartPosition[0] = 2.5;
                    o->StartPosition[1] = 2.5;
                }
				else if(o->SubType==2)
				{
					o->LifeTime = 40;
                    o->StartPosition[0] = 3.0;
                    o->StartPosition[1] = 3.0;
				}

                Vector ( 1.f, 1.f, 1.f, o->Light );
                break;

#ifdef YDG_ADD_SKILL_GIGANTIC_STORM
			case BITMAP_CHROME_ENERGY2:
                if ( o->SubType==0 )
                {
                    o->LifeTime = 30;
                    o->StartPosition[0] = 1.5;
                    o->StartPosition[1] = 1.5;
                }
				break;
#endif	// YDG_ADD_SKILL_GIGANTIC_STORM

			case MODEL_STUN_STONE:
                if ( o->SubType==0 )
                {
         			Vector ( rand()%256/64.f-2.f, -(float)(rand()%200+64)*0.1f, 0.f, p1 );

				    o->LifeTime = 40;
                    o->Scale    = 1.1f+rand()%100/100.f;
    			    o->Gravity  = -(float)(rand()%16+10);

                    o->ExtState = 0;
                    o->Position[2] += 600.f;

                    AngleMatrix ( Angle, Matrix );
                    VectorRotate ( p1, Matrix, o->StartPosition );

                    Vector ( 1.f, 1.f, 1.f, o->Light );
                    Vector ( -20.f, 0.f, rand()%360, o->Angle );
                    Vector ( 0.f, 0.f, Angle[2], o->HeadAngle );
                }
                else if ( o->SubType==1 )
                {
				    o->LifeTime = 22;
                    o->HiddenMesh = -2;

                    Vector ( 0.f, -25.f, 0.f, o->Direction );
                    Vector ( -1.f, -1.f, -1.f, o->Light );
                }
				break;

            case MODEL_SKIN_SHELL:      //  스킨 파편.
    			Vector(0.f,(float)(rand()%128+32)*0.1f,0.f,p1);
				o->Position[2] += 50.f;
				o->LifeTime = rand()%16+32;
				o->Scale    = (float)(rand()%3+3)*0.1f;
                o->Alpha    = 0.5f;
      			o->Angle[2] = (float)(rand()%360);
				AngleMatrix(o->Angle,Matrix);
				VectorRotate(p1,Matrix,o->Direction);
    			o->Gravity = (float)(rand()%5+2);
                o->Gravity += (float)(rand()%5);

                o->Angle[0] = (float)(rand()%360);
                o->Angle[1] = (float)(rand()%360);
                o->Angle[2] = (float)(rand()%360);

                if ( o->SubType==0 )
                {
                    Vector ( 0.5f, 0.5f, 0.5f, o->Light );
                }
                else if ( o->SubType==1 )
                {
                    Vector ( 0.1f, 0.6f, 1.f, o->Light );
                }
                break;

            case MODEL_MANA_RUNE:
                if ( o->SubType==0 )
                {
                    o->LifeTime = 50;
                    o->Scale    = 0.f;
                    o->Gravity  = 0.1f;
                    o->Alpha = 0.3f;
                    o->HiddenMesh = 0;

                    o->Position[2] += 300.f;

                    Vector ( 0.f, 0.f, 45.f, o->Angle );
                }
                else if ( o->SubType==1 )
                {
                    o->LifeTime = 10;
                    o->Scale    = 1.1f;
                    o->Gravity  = 0.1f;
                    o->HiddenMesh = -1;
                    o->BlendMesh = -2;
                    o->BlendMeshLight = 0.4f;
                }
                break;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
            case MODEL_SKILL_JAVELIN:
                {
                    o->LifeTime     = 35;
                    o->Gravity      = 2.f;
                    o->Velocity     = 10.f;
                    o->Scale        = 1.2f;

                    Vector ( 0.f, -5.f, 0.f, o->Direction );
                    VectorCopy ( o->Angle, o->HeadAngle );
                    VectorCopy ( o->Owner->Position, o->StartPosition );
                    o->Position[2] += 150.f;

                    float Ang = rand()%80+10;
                
                    o->HeadAngle[2] += o->SubType*Ang-Ang;
                }
                break;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
			//^ 펜릴 이펙트 관련
			case MODEL_FENRIR_THUNDER:
			{
				if(o->SubType == 0)
				{
					o->LifeTime = 100;
					o->Scale	= 0.3f + (float)(rand()%100)*0.002f;
					o->m_iAnimation	= 0;	// 페이드인 페이드아웃 상태로 사용
					o->Alpha	= 0.7f;

					o->Angle[0] = rand()%360;
					o->Angle[1] = rand()%360;
					o->Angle[2] = rand()%360;
					
					VectorCopy(Light, o->Light);
					
					vec3_t vPos;
					Vector(0.0f, 0.0f, 0.0f, vPos);
					BMD* p_b = &Models[o->Owner->Type];
					int irandom = rand()%30;

					if(irandom >= 1 && irandom <= 2)	// 목 쪽 
					{
						p_b->TransformPosition(BoneTransform[10], vPos, o->Position, false);
					}
					else if(irandom == 3)	// 턱 바깥
					{
						o->Scale -= 0.2f;
						p_b->TransformPosition(BoneTransform[14], vPos, o->Position, false);
					}
					else if(irandom >= 4 && irandom <= 5)	// 엉덩이 쪽
					{
						p_b->TransformPosition(BoneTransform[2], vPos, o->Position, false);
					}
					else if(irandom >= 6 && irandom <= 7)	// 꼬리쪽
					{
						o->Scale -= 0.2f;
						if(rand()%2 == 0)
							p_b->TransformPosition(BoneTransform[50], vPos, o->Position, false);
						else
							p_b->TransformPosition(BoneTransform[51], vPos, o->Position, false);
					}
					else if(irandom == 8)	// 꼬리 끝
					{	
						o->Scale -= 0.2f;
						p_b->TransformPosition(BoneTransform[53], vPos, o->Position, false);
					}
					else if(irandom >= 9 && irandom <= 10)
					{
						o->Live = false;
						o->Alpha = 0.0f;
					}
					else	// 몸 전체
					{
						o->Scale += 0.1f;
						o->Position[0] += rand()%240 - 120;
						o->Position[1] += rand()%10 - 5;
						o->Position[2] += 110;
					}

					vec3_t vLight;
					vLight[0] = o->Light[0] - 0.3f;
					vLight[1] = o->Light[1] - 0.3f;
					vLight[2] = o->Light[2] - 0.3f;
					CreateSprite ( BITMAP_LIGHT, o->Position, 2.0f, vLight, o);
				}
				else if(o->SubType == 1)
				{	
					o->LifeTime = 100;
					o->Scale	= 0.3f + (float)(rand()%100)*0.002f;
					o->Alpha	= 1.0f;
					o->m_iAnimation = 0;

					o->Angle[0] = rand()%360;
					o->Angle[1] = rand()%360;
					o->Angle[2] = rand()%360;
					
					VectorCopy(Light, o->Light);

					o->Position[0] += rand()%40 - 20;
					o->Position[1] += rand()%40 - 20;
					o->Position[2] += rand()%40 - 20;

					vec3_t vLight;
					vLight[0] = o->Light[0] - 0.3f;
					vLight[1] = o->Light[1] - 0.3f;
					vLight[2] = o->Light[2] - 0.3f;
					CreateSprite ( BITMAP_LIGHT, o->Position, 2.0f, vLight, o);
				}
				else if(o->SubType == 2)
				{
					o->LifeTime = 4;
					o->Scale	= 0.1f + (float)(rand()%100)*0.002f;
					o->Alpha	= 1.0f;
					o->m_iAnimation = 0;

					o->Angle[0] = rand()%360;
					o->Angle[1] = rand()%360;
					o->Angle[2] = rand()%360;
					
					VectorCopy(Light, o->Light);

					o->Position[0] += rand()%40 - 20;
					o->Position[1] += rand()%40 - 20;
					o->Position[2] += rand()%40 - 20;

					vec3_t vLight;
					vLight[0] = o->Light[0] - 0.3f;
					vLight[1] = o->Light[1] - 0.3f;
					vLight[2] = o->Light[2] - 0.3f;
					CreateSprite ( BITMAP_LIGHT, o->Position, 2.0f, vLight, o);
				}
				else if(o->SubType == 3)
				{
					o->LifeTime = 4;
					o->Scale	= 0.5f + (float)(rand()%100)*0.002f;
					o->Alpha	= 1.0f;
					o->m_iAnimation = 0;
					
					o->Angle[0] = rand()%360;
					o->Angle[1] = rand()%360;
					o->Angle[2] = rand()%360;
					
					VectorCopy(Light, o->Light);
					
					o->Position[0] += rand()%160 - 80;
					o->Position[1] += rand()%160 - 80;
					o->Position[2] += rand()%160 - 80;
					
					vec3_t vLight;
					vLight[0] = o->Light[0] - 0.3f;
					vLight[1] = o->Light[1] - 0.3f;
					vLight[2] = o->Light[2] - 0.3f;
					CreateSprite ( BITMAP_LIGHT, o->Position, 2.0f, vLight, o);
				}
			}
			break;
			case MODEL_FALL_STONE_EFFECT:
				{
					if(o->SubType == 0 || o->SubType == 1)
					{
						o->LifeTime = rand()%5 + 100;
						
						if(o->SubType == 0)
							o->Scale = (float)(rand()%20+5) * 0.005f + o->Scale * 0.05f;
						else
							o->Scale = (float)(rand()%10+5) * 0.02f + o->Scale * 0.05f;
						
						o->Angle[0] = (float)(rand()%360);
						o->Angle[1] = (float)(rand()%360);
						o->Angle[2] = (float)(rand()%360);
						
						o->Position[0] += (float)(rand()%100-50);
						o->Position[1] += (float)(rand()%100-50);
						
						o->Gravity = (float)(rand()%10+10)*0.5f;
					}
#ifdef CSK_RAKLION_BOSS
					else if(o->SubType == 2)
					{
						o->LifeTime = 100;
						o->Scale = o->Scale + (float)(rand()%20+5) * 0.05f;

						o->Angle[0] = (float)(rand()%360);
						o->Angle[1] = (float)(rand()%360);
						o->Angle[2] = (float)(rand()%360);

						o->Gravity = (float)(rand()%20+40)*0.5f + o->Scale * 2.f;
					}
					else if(o->SubType == 3)
					{
						o->LifeTime = rand()%5+15;
						o->Scale = Scale;
						o->Gravity = (float)(rand()%3+3);
						
						o->Angle[2] = (float)(rand()%360);
						AngleMatrix(o->Angle,Matrix);
						Vector(0.f,(float)(rand()%60+30)*0.1f,0.f,p1);
						VectorRotate(p1,Matrix,o->HeadAngle);
						o->HeadAngle[2] += 25.0f;

						Vector(0, 0, 0, o->Direction);
					}
#endif // CSK_RAKLION_BOSS	
				}
				break;	
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
			case MODEL_FENRIR_FOOT_THUNDER:
				o->LifeTime = 200;
				//o->SubType = SubType;	// 에니메이션 키값으로 사용
				o->m_iAnimation = 0;
				VectorCopy(Position, o->Position);
				VectorCopy(Light, o->Light);
				o->Alpha = 1.0f;
				o->m_dwTime = timeGetTime();	// 프레임간 시간계산으로 사용
				o->Position[2] = 0.0f;
				break;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
				case MODEL_TWINTAIL_EFFECT:
				{
					if(o->SubType == 0)
					{
						o->LifeTime = 200;
						VectorCopy(Position, o->Position);
						VectorCopy(Light, o->Light);
						o->Alpha = 1.0f;
						o->Position[2] = 0.0f;
						o->m_dwTime = timeGetTime();
						o->m_iAnimation = 0;
					}
					else if(o->SubType == 1 || o->SubType == 2)
					{
						o->LifeTime = 50;
						o->Scale = 3.5f;
						VectorCopy(Position, o->Position);
						VectorCopy(Light, o->Light);
						o->Alpha = 1.0f;
						o->Position[2] = 0.0f;
						o->Angle[0] = 0.0f;
					}
				}
				break;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
			case MODEL_FENRIR_SKILL_DAMAGE:
			{
				switch(o->SubType) 
				{
				case 1:	// 빨강
					Vector(0.6f, 0.2f, 0.2f, o->Light);
					break;
				case 2:	// 파랑
					Vector(0.2f, 0.2f, 0.4f, o->Light);
					break;
				case 3:	// 검정
					Vector(0.6f, 0.8f, 0.6f, o->Light);
					break;
				}

//				Vector(1.0f, 0.7f, 0.7f, o->Light);//석근궁금해???
				
				vec3_t vPos;
				int irandom;
				
				BMD* p_b = &Models[o->Owner->Type];
				for(int i=0; i<10; i++)
				{
					irandom = rand()%p_b->NumBones;
					Vector(0.0f, 0.0f, 0.0f, vPos);
					p_b->TransformPosition(o->Owner->BoneTransform[irandom], vPos, o->Position, true);
					CreateParticle(BITMAP_ENERGY, o->Position, o->Angle, o->Light, 2);
				}
			}
			break;
			/////////////////////////////////////////////////////////////////////////////////////////////////////////////
            case MODEL_ARROW_IMPACT:
				o->Velocity = 1.f;
				o->LifeTime = 20;
				o->Scale        = 1.8f;
                o->BlendMesh    = -2;
     			o->Direction[1] = -30.f;

                Vector ( 0.3f, 0.8f, 1.f, o->Light );

                AngleMatrix ( o->Angle, Matrix );
				Vector ( -10.f, -80.f, 200.f, p1 );
				VectorRotate ( p1, Matrix, p2 );
				VectorAdd ( o->Position, p2, o->Position );

                o->Angle[0] = -30.f;
     			CreateJoint ( BITMAP_FLASH, o->Position, o->Position, o->Angle, 4, o, 40.f, 50 );

				o->Weapon = CharacterMachine->PacketSerial;
                break;
            case BITMAP_JOINT_FORCE:
				if(o->SubType == 0)
				{
					o->LifeTime = 20;
					Vector ( 0.f, -550.f, 0.f, o->Direction );
					Vector ( -90.f, 0.f, Angle[2], o->Angle );
					Vector ( 0.f, 0.f, rand()%360, o->HeadAngle );
					CreateJoint ( BITMAP_JOINT_FORCE, o->Position, o->Position, o->Angle, 3, NULL, 80.f );
					VectorCopy ( Position, o->StartPosition );
				}
				else if(o->SubType == 1)
				{
					o->LifeTime = 20;
					Vector ( 0.f, -250.f, 0.f, o->Direction );
					Vector ( -90.f, 0.f, Angle[2], o->Angle );
					Vector ( 0.f, 0.f, rand()%360, o->HeadAngle );
					VectorCopy ( Position, o->StartPosition );
				}
                break;
			case MODEL_SWORD_FORCE :
                o->Velocity = 0.25f;
                if ( o->SubType==0 
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
					|| o->SubType==2
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
					)
                {
					o->LifeTime = 15;
                    o->Scale    = 0.f;
					
                    o->Position[2] += 100.f;
                    Vector ( 0.f, -10.f, 0.f, o->Direction );
                }
                else if ( o->SubType==1 
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
						|| o->SubType==3
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
						)
                {
                    o->LifeTime = 5;
                    o->Scale = 3.5f;
                }
				o->BlendMesh = 0;
				o->BlendMeshLight = 1.0f;
				break;
			case MODEL_PROTECTGUILD:
				{
					o->Alpha = 0;
					o->Angle[2] = +45.0f;
					
					BMD *b = &Models[o->Owner->Type];
					vec3_t tempPosition, p;
					Vector(0.f,0.f,0.f,p);
					b->TransformPosition(o->Owner->BoneTransform[20],p,tempPosition,true);
					o->Position[0] = tempPosition[0];
					o->Position[1] = tempPosition[1];
					o->Position[2] = o->Owner->Position[2] + tempPosition[2] - o->Owner->Position[2] + 60;
					
					o->LifeTime     = 130;
					o->Scale        = 3.2f;
					Vector(1.0f, 1.0f, 1.0f, o->Light);
					VectorCopy(o->Position, o->StartPosition);
					for (int i = 0; i < 10; ++i)
					{
						float fAngle = rand() % 360;
						Vector(o->Position[0] + (rand() % 20 + 15) * sinf(fAngle),
							o->Position[1] + (rand() % 20 + 15) * cosf(fAngle),
							o->Position[2] /*- 25 + rand()%50*/, p);
						CreateParticle(BITMAP_SPARK+1,p,o->Angle,o->Light,4,0.6f,o);
					} 
				}
				break;
			case MODEL_MOVE_TARGETPOSITION_EFFECT:
				{
					if( o->SubType == 0)
					{
						o->LifeTime = 30;
						o->BlendMesh = 0;
						
						vec3_t vLight, vPos;
						Vector( 1.0f, 0.7f, 0.3f, vLight );
						VectorCopy( o->Position, vPos );
						vPos[2] += 85.f;
						
						// 기존 이팩트가 있으면 삭제
						DeleteEffect( BITMAP_MAGIC, NULL, 11 );
						DeleteEffect( BITMAP_TARGET_POSITION_EFFECT2, NULL, 0 );
						DeleteEffect( BITMAP_TARGET_POSITION_EFFECT1, NULL, 0 );
						DeleteJoint( MODEL_SPEARSKILL, NULL, 16 );
						
						// 추가 이팩트 생성
						CreateEffect( BITMAP_MAGIC, o->Position, o->Angle, vLight, 11 );
						CreateEffect( BITMAP_TARGET_POSITION_EFFECT2, o->Position, o->Angle, vLight, 0);
						CreateJoint( MODEL_SPEARSKILL, vPos, vPos, Angle, 16, o, 5.0f );
						CreateJoint( MODEL_SPEARSKILL, vPos, vPos, Angle, 16, o, 5.0f );
						CreateJoint( MODEL_SPEARSKILL, vPos, vPos, Angle, 16, o, 5.0f );
						CreateJoint( MODEL_SPEARSKILL, vPos, vPos, Angle, 16, o, 5.0f );
					}
				}
				break;

			case BITMAP_TARGET_POSITION_EFFECT1:
				{
					if( o->SubType == 0 )
					{
						o->LifeTime = 20;
						o->Scale = 1.2f;
					}
				}
				break;
			case BITMAP_TARGET_POSITION_EFFECT2:
				{
					if( o->SubType == 0 )
					{
						o->LifeTime = 30;
						o->Scale = 1.8f;
					}
				}
				break;
			case MODEL_EFFECT_SAPITRES_ATTACK:		// 사피트레스 공격 이팩트
				{
					if( o->SubType == 0)
					{
						o->LifeTime = 20;

						// 사방으로 가시 날라가기
					    for( int i=0 ; i<10 ; i++ )
						{
							CreateEffect( MODEL_EFFECT_SAPITRES_ATTACK_2, o->Position, o->Angle, o->Light, 14 );
						}
					}
				}
				break;
			case MODEL_EFFECT_SAPITRES_ATTACK_1:	// 사피트레스 가시 날리기
				{
					if( o->SubType == 0 )
					{
						o->BlendMesh = 0;
						o->BlendMeshLight = 1.0f;
						o->Position[2] += 100.f;
						o->LifeTime = 17;
						o->Scale = 1.1f;
						VectorSubtract( o->Owner->Position, o->Position, o->Direction );
						VectorNormalize( o->Direction );
						// 앵글값 구하기
						o->Angle[2] = CreateAngle( o->Position[0], o->Position[1], o->Owner->Position[0], o->Owner->Position[1] );
					}
				}
				break;
			case MODEL_EFFECT_THUNDER_NAPIN_ATTACK_1:	// 썬더네이핀 썬더 에너지볼
				{
					if( o->SubType == 0 )
					{
						o->LifeTime = 30;
					}
					else if( o->SubType == 1 )
					{
						o->LifeTime = 30;
					}
				}
				break;

//--------------------------------------------------------------------------------------------------------------------------------

#ifdef CSK_EVENT_CHERRYBLOSSOM
			case MODEL_EFFECT_SKURA_ITEM:
				{
#ifdef KJH_ADD_09SUMMER_EVENT
					if( (o->SubType == 0) || (o->SubType == 1))
#else // KJH_ADD_09SUMMER_EVENT
					if( o->SubType == 0 )
#endif // KJH_ADD_09SUMMER_EVENT
					{	
						o->LifeTime = 52;	
					}
				}
				break;
#endif //CSK_EVENT_CHERRYBLOSSOM

//--------------------------------------------------------------------------------------------------------------------------------

#ifdef ADD_SOCKET_ITEM
			case MODEL_EFFECT_TRACE:			// 무기 잔상이팩트
				{
					if( o->SubType == 0 )
					{
						o->LifeTime = 50;
						VectorCopy(o->Position, o->EyeLeft);
						CreateJoint(BITMAP_JOINT_ENERGY, o->Position, o->Position, o->Angle, 17, o, o->Scale, -1, 0, 0, -1, o->Light);
					}
				}break;
			case MODEL_STAR_SHINE:
				{
					switch( o->SubType )
					{
					case 0:
						{
							o->LifeTime = 30;
							o->Alpha = 0.2f;
							o->Angle[0] = (float)(rand()%360);
						}break;
					}		
				}break;
			case MODEL_FEATHER:				// 다크스팅거 보우 깃털 날리기
				{
					switch( o->SubType )
					{
					case 0:
					case 1:
#ifdef PJH_SEASON4_DARK_NEW_SKILL_CAOTIC
					case 2:
					case 3:
#endif //#ifdef PJH_SEASON4_DARK_NEW_SKILL_CAOTIC
 						{
							vec3_t vOriginPos;
							VectorCopy( o->Position, vOriginPos );
	
							o->Position[0] += (float)(rand()%20-10)*4.f;
							o->Position[1] += (float)(rand()%20-10)*4.f;
							o->Position[2] += (float)(rand()%20-10)*4.f;

							VectorSubtract( vOriginPos, o->Position, o->Direction );
							VectorNormalize( o->Direction );
							int iAddDirection = ((float)(rand()%10-5)*0.08f);
							o->Direction[0] += iAddDirection;
							o->Direction[1] += iAddDirection;
							o->Direction[2] += iAddDirection;

 							o->Scale = o->Scale + ((float)(rand()%20-10)*(o->Scale*0.03f)); // o->Scale의 30% (20단계)
 							o->LifeTime = 30+(rand()%20-10);						// 20 ~ 40 (20단계)
#ifdef PJH_SEASON4_DARK_NEW_SKILL_CAOTIC
							if(o->SubType == 2 || o->SubType == 3)
								o->LifeTime = 100;						// 20 ~ 40 (20단계)
#endif //#ifdef PJH_SEASON4_DARK_NEW_SKILL_CAOTIC
							o->Alpha = 0.6f + ((float)(rand()%10)*0.02f);				// 0.8 ~ 1.0f (10단계)

							o->Gravity = 0.1f;
#ifdef PJH_SEASON4_DARK_NEW_SKILL_CAOTIC
							if(o->SubType == 2 || o->SubType == 3)
								o->Gravity = 0.f;
#endif //PJH_SEASON4_DARK_NEW_SKILL_CAOTIC
							o->Angle[0] = (float)(rand()%360);
							o->Angle[1] = (float)(rand()%360);
							o->Angle[2] = (float)(rand()%360);

							// 회전 속도 (이 이팩트에서 EyeRight는 사용불가)
							o->EyeRight[0] = (float)(rand()%10-5);		// -15도 ~ 15도
							o->EyeRight[1] = (float)(rand()%10-5);
							o->EyeRight[2] = (float)(rand()%10-5);

						}break;
					}

				}break;
#endif // ADD_SOCKET_ITEM
#ifdef LDK_ADD_PC4_GUARDIAN_EFFECT_IMAGE
					case MODEL_FEATHER_FOREIGN:				// 수호정령 타입 추가 하자
						{
							switch( o->SubType )
							{
							case 4:
								{
									vec3_t vOriginPos;
									VectorCopy( o->Position, vOriginPos );
									
									o->Position[0] += (float)(rand()%20-10)*4.f;
									o->Position[1] += (float)(rand()%20-10)*4.f;
									o->Position[2] += (float)(rand()%20-10)*4.f;
									
									VectorSubtract( vOriginPos, o->Position, o->Direction );
									VectorNormalize( o->Direction );
									int iAddDirection = ((float)(rand()%10-5)*0.08f);
									o->Direction[0] += iAddDirection;
									o->Direction[1] += iAddDirection;
									o->Direction[2] += iAddDirection;
									
									o->Scale = o->Scale + ((float)(rand()%20-10)*(o->Scale*0.03f)); // o->Scale의 30% (20단계)
									o->LifeTime = 25;//<<-------수치가 틀리게 되어있음 할칠때 조심할것
									o->Alpha = 0.3f; //<<-------수치가 틀리게 되어있음 할칠때 조심할것
									
									o->Gravity = 0.1f;
									o->Angle[0] = (float)(rand()%360);
									o->Angle[1] = (float)(rand()%360);
									o->Angle[2] = (float)(rand()%360);
									
									// 회전 속도 (이 이팩트에서 EyeRight는 사용불가)
									o->EyeRight[0] = (float)(rand()%10-5);		// -15도 ~ 15도
									o->EyeRight[1] = (float)(rand()%10-5);
									o->EyeRight[2] = (float)(rand()%10-5);
									
								}break;
							}
							
						}break;
#endif //LDK_ADD_PC4_GUARDIAN_EFFECT_IMAGE

//--------------------------------------------------------------------------------------------------------------------------------

#ifdef CSK_ADD_SKILL_BLOWOFDESTRUCTION
			case MODEL_BLOW_OF_DESTRUCTION:
				{
					if(o->SubType == 0)
					{
						o->LifeTime = 40;
						
						// 부모오프젝트가 바라보는 방향으로 조금 이동
						assert(o->Owner && "부모 포인트를 넣어줘야 합니다.!");
						vec3_t vPos, vPos2;
						float Matrix[3][4];
						Vector(-20.f, -100.f, 0.f, vPos);
						AngleMatrix(o->Owner->Angle, Matrix);
						VectorRotate(vPos, Matrix, vPos2);	
						VectorAdd(vPos2, o->Position, o->Position);

						// 타켓 위치 복사
						VectorCopy(o->Light, o->StartPosition);
						
						// 라이트값 입력
						Vector(1.2f, 1.2f, 1.2f, o->Light);

						// 타켓에 이펙트 발생
						CreateEffect(MODEL_BLOW_OF_DESTRUCTION, o->StartPosition, o->Angle, o->Light, 1, o->Owner);
					}
					else if(o->SubType == 1)
					{
						o->LifeTime = 40;
						Vector(1.2f, 1.2f, 1.2f, o->Light);
						o->Position[2] = 150.f;
						o->Scale = 5.f;
					}
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
					else if(o->SubType==2)
					{
						o->LifeTime = 50;
						vec3_t vPos, vPos2;
						float Matrix[3][4];
						Vector(0.f, 0.f, 0.f, vPos);
						AngleMatrix(o->Owner->Angle, Matrix);
						VectorRotate(vPos, Matrix, vPos2);	
						VectorAdd(vPos2, o->Position, o->Position);
						if(o->Owner->m_sTargetIndex<0)
							break;
						VectorCopy(CharactersClient[o->Owner->m_sTargetIndex].Object.Position, o->StartPosition);
						o->Scale =  0.5f;
					}
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
				}
				break;

//--------------------------------------------------------------------------------------------------------------------------------	

			case MODEL_NIGHTWATER_01:
				{
					o->LifeTime = 25;
					o->Alpha = 1.f;
					o->Angle[2] = rand()%360;
				}
				break;

//--------------------------------------------------------------------------------------------------------------------------------		
				
			case MODEL_KNIGHT_PLANCRACK_A:
				if (o->SubType == 0)
				{
					o->LifeTime = 25;
					o->Alpha = 1.f;
					o->Angle[2] = rand()%360;
					o->Scale = Scale + rand()%10 * 0.05f;
					o->Position[2] += 10.f;
				}
#ifdef YDG_ADD_SKILL_LIGHTNING_SHOCK
				else if (o->SubType == 1)
				{
					o->LifeTime = 20;
					o->Alpha = 1.f;
					o->Angle[2] = rand()%360;
					o->Scale = Scale + rand()%10 * 0.05f;
					o->Position[2] += 10.f;
				}
#endif	// YDG_ADD_SKILL_LIGHTNING_SHOCK
				break;

//--------------------------------------------------------------------------------------------------------------------------------			

			case MODEL_KNIGHT_PLANCRACK_B:
				{
					o->LifeTime = 25;
					o->Alpha = 1.f;
					o->Position[2] += 15.f;
					o->Angle[2] += 90.f;
				}
				break;
#endif // CSK_ADD_SKILL_BLOWOFDESTRUCTION

//--------------------------------------------------------------------------------------------------------------------------------

#ifdef YDG_ADD_SKILL_FLAME_STRIKE
			case MODEL_EFFECT_FLAME_STRIKE:
				if(o->SubType == 0)
				{
					o->Alpha = 0;
					o->LifeTime = 35;
					o->Velocity = Models[o->Owner->Type].Actions[PLAYER_SKILL_FLAMESTRIKE].PlaySpeed;
					o->AI = 0;
					o->m_iAnimation = rand();
				}
				break;
#endif	// YDG_ADD_SKILL_FLAME_STRIKE

//--------------------------------------------------------------------------------------------------------------------------------
#ifdef LDS_RAKLION_ADDMONSTER_ICEWALKER
			case MODEL_STREAMOFICEBREATH:
				{
					const	float LENSCALAR				= 50.0f;
					const	float RANDOMOFFSET_ANGLE	= 5.0f;
					const	float RANDOMOFFSET_SCALE	= 0.01f;
					const	float RANDOMOFFSET_POSITION	= 20.0f;
					const	float RANDNUM				= ((float)((rand() % 2000)-1000) * 0.001f);
					
					// 1. Set Value
					o->LifeTime = 17;
					o->Light[0] = 0.6f;
					o->Light[1] = 0.6f;
					o->Light[2] = 0.6f;
					
					// 2. Calculate Position 
					vec3_t		v3Len_, v3LenBasis_, v3PosBasis_;
					float		matRotation[3][4];	
					
					Vector( 0.0f, -1.0f, 0.0f, v3LenBasis_ );
					AngleMatrix( o->Angle, matRotation );
					VectorRotate( v3LenBasis_, matRotation, v3Len_ );
					
					v3PosBasis_[0] = o->Position[0] + ( v3Len_[0] * LENSCALAR );
					v3PosBasis_[1] = o->Position[1] + ( v3Len_[1] * LENSCALAR );
					v3PosBasis_[2] = o->Position[2] + ( v3Len_[2] * LENSCALAR );
					
					VectorCopy( v3PosBasis_, o->StartPosition );
					// 3. Reality Position, Angle, Scaling
					for( INT i_ = 0; i_ < 4; ++i_ )
					{
						vec3_t		v3ResultAngle, v3ResultPos;
						float		fResultScale;
						float		fCurrentOffsetScale, fCurrentOffsetPos, fCurrentOffsetAngle;
						fCurrentOffsetScale	= RANDOMOFFSET_SCALE * RANDNUM;
						fCurrentOffsetPos	= RANDOMOFFSET_POSITION * RANDNUM;
						fCurrentOffsetAngle	= RANDOMOFFSET_ANGLE * RANDNUM;
						
						// 1. 위치값 
						v3ResultPos[0] = v3PosBasis_[0] + fCurrentOffsetPos;
						v3ResultPos[1] = v3PosBasis_[1] + fCurrentOffsetPos;
						v3ResultPos[2] = v3PosBasis_[2] + fCurrentOffsetPos;
						
						// 2. 각도 값
						v3ResultAngle[0] = o->Angle[0] + fCurrentOffsetAngle;
						v3ResultAngle[1] = o->Angle[1] + fCurrentOffsetAngle;
						v3ResultAngle[2] = o->Angle[2] + fCurrentOffsetAngle;
						
						// 3. 크기값
						fResultScale = o->Scale + fCurrentOffsetScale;
						
						CreateParticle( BITMAP_RAKLION_CLOUDS, v3ResultPos, v3ResultAngle, o->Light, 0, fResultScale );
					}
					
				}break;
#endif // LDS_RAKLION_ADDMONSTER_ICEWALKER	
//--------------------------------------------------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------------------------------------------------
#ifdef LDS_RAKLION_ADDMONSTER_COOLERTIN
			case MODEL_1_STREAMBREATHFIRE:
				{
					o->LifeTime = 30;
				}
				break;
#endif // LDS_RAKLION_ADDMONSTER_COOLERTIN
//--------------------------------------------------------------------------------------------------------------------------------
#ifdef PBG_ADD_PKFIELD
			case MODEL_LAVAGIANT_FOOTPRINT_R:
			case MODEL_LAVAGIANT_FOOTPRINT_V:	
				{
					o->LifeTime = 200;
					VectorCopy(Position, o->Position);
					VectorCopy(Light, o->Light);
					o->Alpha = 1.0f;
					o->Position[2] = 0.0f;
					o->m_dwTime = timeGetTime();
					o->Scale = o->Scale;
				}
				break;
#endif //PBG_ADD_PKFIELD
#ifdef LDS_ADD_GOLDCORPS_EVENT_MOD_GREATDRAGON
			case MODEL_EFFECT_FIRE_HIK3_MONO:		// 황금 군단의 황금 그레이트 드래곤 비주얼 이펙트. 
				{
					int iRandNum = (rand() % 100);

					if( iRandNum > 20 )
					{
						CreateParticle(BITMAP_FIRE_HIK3_MONO, o->Position, o->Angle, o->Light, 1, o->Scale);
					}
				}
				break;
#endif // LDS_ADD_GOLDCORPS_EVENT_MOD_GREATDRAGON
#ifdef LDS_ADD_EMPIRE_GUARDIAN
			case MODEL_PROJECTILE:
				{
					//o->m_iAnimation을 switch 로 사용하자
					o->LifeTime = 50;
					o->Gravity = 0.0f;
					o->Velocity = 70.0f;
					VectorCopy(Position,o->StartPosition); //시작위치
					//o->Angle //방향
				}
				break;
			case MODEL_DOOR_CRUSH_EFFECT_PIECE01:	//성문 파괴 이펙트 조각 1
			case MODEL_DOOR_CRUSH_EFFECT_PIECE02:	//성문 파괴 이펙트 조각 2
			case MODEL_DOOR_CRUSH_EFFECT_PIECE03:	//성문 파괴 이펙트 조각 3
			case MODEL_DOOR_CRUSH_EFFECT_PIECE04:	//성문 파괴 이펙트 조각 4
			case MODEL_DOOR_CRUSH_EFFECT_PIECE05:	//성문 파괴 이펙트 조각 5
			case MODEL_DOOR_CRUSH_EFFECT_PIECE06:	//성문 파괴 이펙트 조각 6
			case MODEL_DOOR_CRUSH_EFFECT_PIECE07:	//성문 파괴 이펙트 조각 7
			case MODEL_DOOR_CRUSH_EFFECT_PIECE08:	//성문 파괴 이펙트 조각 8
			case MODEL_DOOR_CRUSH_EFFECT_PIECE09:	//성문 파괴 이펙트 조각 9
			case MODEL_DOOR_CRUSH_EFFECT_PIECE11:	//성문 파괴 이펙트 조각 7
			case MODEL_DOOR_CRUSH_EFFECT_PIECE12:	//성문 파괴 이펙트 조각 8
			case MODEL_DOOR_CRUSH_EFFECT_PIECE13:	//성문 파괴 이펙트 조각 9
			case MODEL_STATUE_CRUSH_EFFECT_PIECE01:	//석상 파괴 이펙트 조각 1
			case MODEL_STATUE_CRUSH_EFFECT_PIECE02:	//석상 파괴 이펙트 조각 2
			case MODEL_STATUE_CRUSH_EFFECT_PIECE03:	//석상 파괴 이펙트 조각 3
				{
					o->LifeTime     = 30+(rand()%30);
					o->Velocity     = 0.f;
					o->PKKey = -1;
					o->Owner = Owner;
					o->Gravity      = 2.3f;
					
					o->Angle[2] = (float)(rand()%360);
					AngleMatrix(o->Angle,Matrix);
					Vector(0.f,(float)(rand()%64+48)*0.1f,0.f,p1);
					VectorRotate(p1,Matrix,o->HeadAngle);
					o->HeadAngle[2] = 15;
					
					o->SubType = rand()%2;
					
					o->Light[0] = o->Light[1] = o->Light[2] = 0.5+(rand()%6)*0.1f;
					o->Direction[0] = 0;
					o->Direction[1] = 0;
					o->Direction[2] = 0;
				}
				break;
			case MODEL_STATUE_CRUSH_EFFECT_PIECE04:	//석상 파괴 이펙트 조각 4 잔해
			case MODEL_DOOR_CRUSH_EFFECT_PIECE10:	// 주말 성문 파괴 이펙트 조각1 잔해
				{
					o->LifeTime     = 100;
 					o->Scale		= Scale;
					
					o->PKKey = -1;
					o->Owner = Owner;
				}
				break;
			case MODEL_DOOR_CRUSH_EFFECT:	//성문 파괴 이펙트
				{
					switch(o->SubType)
						
					{
					case 0:
						{
							vec3_t vPos;
							for(int i=0; i<9; i++)
							{
								Vector(	o->Position[0]+(rand()%200-100),
									o->Position[1]+(rand()%200-100),
									o->Position[2]+(rand()%200-100),
									vPos);
								CreateEffect(MODEL_DOOR_CRUSH_EFFECT_PIECE01+i, vPos, o->Angle, o->Light,0,o->Owner,0,0);
							}
						}break;
					case 1:	//주말용
						{
							vec3_t vPos;
							for(int i=0; i<9; i++)
							{
								Vector(	o->Position[0]+(rand()%200-100),
									o->Position[1]+(rand()%200-100),
									o->Position[2]+(rand()%200-100),
									vPos);
								CreateEffect(MODEL_DOOR_CRUSH_EFFECT_PIECE11+(i%3), vPos, o->Angle, o->Light,0,o->Owner,0,0);
							}
						}break;
					}
				}
				break;
			case MODEL_STATUE_CRUSH_EFFECT:			//석상 파괴 이펙트
				{
					for(int i=0; i<6; i++)
					{
						CreateEffect(MODEL_STATUE_CRUSH_EFFECT_PIECE01+(i%3), o->Position, o->Angle, o->Light,0,o->Owner,0,0);
					}
				}
				break;
#endif //LDS_ADD_EMPIRE_GUARDIAN
#ifdef LDS_ADD_EG_4_MONSTER_WORLDBOSS_GAIONKALEIN
			case MODEL_SWORDRIGHT01_EMPIREGUARDIAN_BOSS_GAION_:		
			case MODEL_SWORDLEFT01_EMPIREGUARDIAN_BOSS_GAION_:
			case MODEL_SWORDRIGHT02_EMPIREGUARDIAN_BOSS_GAION_:
			case MODEL_SWORDLEFT02_EMPIREGUARDIAN_BOSS_GAION_:
			case MODEL_SWORDMAIN01_EMPIREGUARDIAN_BOSS_GAION_:
			case MODEL_EMPIREGUARDIANBOSS_FRAMESTRIKE:
			{
				// . 생성되었던 모든 검기를 삭제. 사용되는 검기Index는 113-155에 산발적 분포.
				OBJECT	*O = &(Hero->Object);
				for( int i = 113; i<=155; ++i )
				{
					RemoveObjectBlurs(O, i);
				}
			
				// 가이온 모든 이펙트 생성 시작부분
				// ==========================================================================================>
				if ( o->SubType==0 )		// 가이온기본 본 위치.
				{
					o->LifeTime = 2;
					o->ChromeEnable = true;	// Owner Bone에 Attach하여 렌더링 할것인지?
				}
				else if(o->SubType==3)		// 기간틱 스톰.	(모든 보델을 렌덤하게 일괄 처리 합니다.)
				{
					const int	TOTAL_LIFETIME = 60;		// 3초정도
					vec3_t		v3PosStart, v3PosTarget;
					vec3_t		arv3PosProcess[3];//, arv3AngProcess[3];
					
					o->ExtState		= TOTAL_LIFETIME;			// TotalLife 미사용중인 임의의 변수 사용.
					o->LifeTime     = TOTAL_LIFETIME;
					o->Gravity      = 0.0f;
					o->HiddenMesh   = 1;
					o->Distance		= 0.0f;
					o->Alpha		= 1.0f;
					o->ChromeEnable = false;	// Owner Bone에 Attach하여 렌더링 할것인지?
					
					// ㄱ. 움직임 각도 백업. 모델의 각도 + 자신의 상대 각도
					Vector ( o->Angle[0], o->Angle[1], o->Angle[2], o->HeadAngle );	
					
					VectorCopy ( o->Position, o->StartPosition );	// ㄴ. 시작 위치점 백업.
					VectorCopy ( o->Position, v3PosStart );			// ㄷ. 최초 시작 지점.
					VectorCopy ( o->Light, v3PosTarget );			// ㄹ. 최종 타겟 지점.
					
					// ㄴ. 위치값 지정 -----------------------------------------------------
					vec3_t		v3DirDistAD;
					vec3_t		v3PosStartModify, v3PosTargetModify;
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
					const int	iLimitArea1 = 200;
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
					const int	iLimitArea1 = 200, iLimitArea2 = 200;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
					float		fDistAD, fDistAB, fDistCD;	
					float		fHeightTerrainTarget = RequestTerrainHeight(v3PosTarget[0],v3PosTarget[1]);
					int			iOffsetDist = 100;
					// 1. 위치 선계산
					VectorCopy( v3PosStart, v3PosStartModify );
					VectorCopy( v3PosTarget, v3PosTargetModify );
					v3PosTargetModify[2] = v3PosStartModify[2];
					fDistAD =  VectorDistance3D_DirDist( v3PosStartModify, v3PosTargetModify, v3DirDistAD );
					
					fDistAB = 900.0f + (rand() % iOffsetDist - (iOffsetDist / 2)) ;
					fDistCD;		// 후계산.
					VectorCopy(v3PosStart, arv3PosProcess[0]);		//			A 위치 : 시전자의 위치
					// ------------------------------------------------			B 위치 : 시전자의 머리위 위치
					arv3PosProcess[1][0] = arv3PosProcess[0][0] + (float)(rand()%iLimitArea1 - (iLimitArea1/2));
					arv3PosProcess[1][1] = arv3PosProcess[0][1] + (float)(rand()%iLimitArea1 - (iLimitArea1/2));
					arv3PosProcess[1][2] = v3PosStart[2] + fDistAB;				 
					VectorAdd(arv3PosProcess[1],v3DirDistAD,arv3PosProcess[2]);	//C 위치: 대상의 머리위 위치

					// ------------------------------------------------			D 위치 : 대상의 FLour 위치
					VectorCopy(v3PosTargetModify, arv3PosProcess[3]);
					arv3PosProcess[3][0] = arv3PosProcess[2][0];// + (float)(rand()%iLimitArea2 - (iLimitArea2/2));
					arv3PosProcess[3][1] = arv3PosProcess[2][1];// + (float)(rand()%iLimitArea2 - (iLimitArea2/2));
					arv3PosProcess[3][2] = fHeightTerrainTarget + (float)iOffsetDist*1.2f + ((rand()%iOffsetDist)-(iOffsetDist/2));

// 					// 2. 각도값 선계산
// 					int	iOffsetAng = 100;
// 					VectorCopy(o->Angle, arv3AngProcess[0]);
// 					for(int i = 0; i < 3; ++i)
// 					{
// 						arv3AngProcess[i][0] = (rand()%iOffsetAng * 0.1f );
// 						arv3AngProcess[i][1] = (rand()%iOffsetAng * 0.1f );
// 						arv3AngProcess[i][2] = (rand()%iOffsetAng * 0.1f );
// 					}
// 					
 					float	arfRates[] = { 0.0f, 0.22f, 0.35f, 0.50f, 1.01f };
 					float	fOffsetRate = (0.03f * (o->Type - MODEL_SWORDLEFT01_EMPIREGUARDIAN_BOSS_GAION_));
					
					o->m_Interpolates.ClearContainer();
//  					// 보간컨테이너에 각도 적재 ----------------------------------------------- 
  					CInterpolateContainer::INTERPOLATE_FACTOR	InsertFactor;
// 					InsertFactor.fRateStart	= arfRates[0];
// 					InsertFactor.fRateEnd	= arfRates[1];
// 					VectorCopy(arv3AngProcess[0], InsertFactor.v3Start);
// 					VectorCopy(arv3AngProcess[1], InsertFactor.v3End);
// 					o->m_Interpolates.m_vecInterpolatesAngle.push_back( InsertFactor );		// 1#
// 					// 보간컨테이너에 각도 적재 ----------------------------------------------- 
					
					// 보간컨테이너에 위치 적재 ----------------------------------------------- 
					InsertFactor.fRateStart	= arfRates[0] + fOffsetRate;
					InsertFactor.fRateEnd	= arfRates[1] + fOffsetRate;
					VectorCopy(v3PosStart, InsertFactor.v3Start);
					VectorCopy(arv3PosProcess[1], InsertFactor.v3End);
					o->m_Interpolates.m_vecInterpolatesPos.push_back( InsertFactor );		// 2#
					
					InsertFactor.fRateStart	= arfRates[1] + fOffsetRate;
					InsertFactor.fRateEnd	= arfRates[2] + fOffsetRate;
					VectorCopy(arv3PosProcess[1], InsertFactor.v3Start);
					VectorCopy(arv3PosProcess[2], InsertFactor.v3End);
					o->m_Interpolates.m_vecInterpolatesPos.push_back( InsertFactor );		// 2#

					InsertFactor.fRateStart	= arfRates[2] + fOffsetRate;
					InsertFactor.fRateEnd	= arfRates[3] + fOffsetRate;
					VectorCopy(arv3PosProcess[2], InsertFactor.v3Start);
					VectorCopy(arv3PosProcess[3], InsertFactor.v3End);
					o->m_Interpolates.m_vecInterpolatesPos.push_back( InsertFactor );		// 2#

					InsertFactor.fRateStart	= arfRates[3] + fOffsetRate;
					InsertFactor.fRateEnd	= arfRates[4];
					VectorCopy(arv3PosProcess[3], InsertFactor.v3Start);
					VectorCopy(arv3PosProcess[3], InsertFactor.v3End);
					o->m_Interpolates.m_vecInterpolatesPos.push_back( InsertFactor );		// 2#
					// 보간컨테이너에 위치 적재 ----------------------------------------------- 

					// 보간컨테이너에 ALPHA 적재 ----------------------------------------------- 
					CInterpolateContainer::INTERPOLATE_FACTOR_F	InsertFactorF;						
					InsertFactorF.fRateStart	= arfRates[0];
					InsertFactorF.fRateEnd	= arfRates[3];
					InsertFactorF.fStart = 1.0f;
					InsertFactorF.fEnd = 1.0f;

					InsertFactorF.fRateStart	= arfRates[3];
					InsertFactorF.fRateEnd	= arfRates[4];
					InsertFactorF.fStart = 1.0f;
					InsertFactorF.fEnd = 0.0f;
					o->m_Interpolates.m_vecInterpolatesAlpha.push_back( InsertFactorF );		// 2#
					// 보간컨테이너에 ALPHA 적재 ----------------------------------------------- 

					// 초기값 설정
					o->m_Interpolates.GetAngleCurrent(o->Angle, 0.0f);		// 각도
					o->m_Interpolates.GetPosCurrent(o->Position, 0.0f);		// 위치
					o->m_Interpolates.GetAlphaCurrent(o->Alpha, 0.0f);		// 알파

					// 각 검별로 JOINT 생성
					CreateJoint ( BITMAP_FLARE+1, o->Position, o->Position, o->Angle, 20, o, 160.f, 40 );
				}
				else if( o->SubType == 11 )	// END EFFECT의 APPEAR.
				{
					const int	TOTALLIFETIME = 24;
					o->LifeTime = TOTALLIFETIME;
					o->ExtState = TOTALLIFETIME;
					o->ChromeEnable = true;
					o->Scale = Scale;
					
					o->m_Interpolates.ClearContainer();
					
					// 보간컨테이너에 ALPHA 적재 ----------------------------------------------- 
					CInterpolateContainer::INTERPOLATE_FACTOR_F InsertFactorF;
					InsertFactorF.fRateStart	= 0.0f;
					InsertFactorF.fRateEnd	= 0.61f;
					InsertFactorF.fStart = 0.0f;
					InsertFactorF.fEnd = 1.0f;
					o->m_Interpolates.m_vecInterpolatesAlpha.push_back( InsertFactorF );		// 1#

					InsertFactorF.fRateStart	= 0.61f;
					InsertFactorF.fRateEnd	= 1.01f;
					InsertFactorF.fStart = 1.0f;
					InsertFactorF.fEnd = 1.0f;
					o->m_Interpolates.m_vecInterpolatesAlpha.push_back( InsertFactorF );		// 1#
					// 보간컨테이너에 ALPHA 적재 ----------------------------------------------- 
				}
				else if( o->SubType == 12 )	// END EFFECT의 APPEAR.
				{
					const int	TOTALLIFETIME = 14;
					o->LifeTime = TOTALLIFETIME;
					o->ExtState = TOTALLIFETIME;
					o->ChromeEnable = true;
					o->Scale = Scale;
					
					o->m_Interpolates.ClearContainer();
					
					// 보간컨테이너에 ALPHA 적재 ----------------------------------------------- 
					CInterpolateContainer::INTERPOLATE_FACTOR_F InsertFactorF;
					InsertFactorF.fRateStart	= 0.0f;
					InsertFactorF.fRateEnd	= 1.01f;
					InsertFactorF.fStart = 0.0f;
					InsertFactorF.fEnd = 1.0f;
					o->m_Interpolates.m_vecInterpolatesAlpha.push_back( InsertFactorF );		// 2#
					// 보간컨테이너에 ALPHA 적재 ----------------------------------------------- 
				}
				else if( o->SubType==20 )	// 블러드 어택의 잔상 검기.
				{
					const int	TOTALLIFETIME = 10;
					o->LifeTime = TOTALLIFETIME;
					o->ExtState = TOTALLIFETIME;
					o->ChromeEnable = false;

					switch( o->Type )
					{
						case MODEL_SWORDRIGHT01_EMPIREGUARDIAN_BOSS_GAION_:		
						case MODEL_SWORDLEFT01_EMPIREGUARDIAN_BOSS_GAION_:
						{
							o->LifeTime = 10;
						}
						break;
						case MODEL_SWORDRIGHT02_EMPIREGUARDIAN_BOSS_GAION_:
						case MODEL_SWORDLEFT02_EMPIREGUARDIAN_BOSS_GAION_:
						{
							o->LifeTime = 5;
						}
						break;
						case MODEL_EMPIREGUARDIANBOSS_FRAMESTRIKE:
						{
							o->LifeTime = 5;
						}
					}
					// PASS METHOD
// 					o->Alpha = 0.6f;
// 					o->Distance = o->Alpha;
// 					o->Scale = Scale;
// 					o->AlphaTarget = Scale;								// 스케일   BACKUP
// 					VectorCopy( o->Position, o->StartPosition );		// 시작지점 BACKUP
// 					VectorCopy( o->Angle, o->m_v3PrePos1 );				// 시작각도 BACKUP
// 					VectorCopy( Light, o->Light );
// 					VectorCopy( Position, o->Position );
// 					VectorCopy( Angle, o->Angle );

					// NEW METHOD
					o->m_Interpolates.ClearContainer();
  					// 보간컨테이너에 각도 적재 ----------------------------------------------- 
					CInterpolateContainer::INTERPOLATE_FACTOR	InsertFactor;
					InsertFactor.fRateStart	= 0.0f;
					InsertFactor.fRateEnd	= 1.0f;
					VectorCopy(o->Angle, InsertFactor.v3Start);
					VectorCopy(o->Angle, InsertFactor.v3End);
					o->m_Interpolates.m_vecInterpolatesAngle.push_back( InsertFactor );		// 1#
					// 보간컨테이너에 각도 적재 ----------------------------------------------- 
					
					// 보간컨테이너에 위치 적재 ----------------------------------------------- 
					InsertFactor.fRateStart	= 0.0f;
					InsertFactor.fRateEnd	= 1.0f;
					VectorCopy(o->Position, InsertFactor.v3Start);
					VectorCopy(o->Position, InsertFactor.v3End);
					o->m_Interpolates.m_vecInterpolatesPos.push_back( InsertFactor );		// 2#
					// 보간컨테이너에 위치 적재 ----------------------------------------------- 
					
					// 보간컨테이너에 ALPHA 적재 ----------------------------------------------- 
					CInterpolateContainer::INTERPOLATE_FACTOR_F	InsertFactorF;						
					InsertFactorF.fRateStart = 0.0f;
					InsertFactorF.fRateEnd	 = 1.0f;
					InsertFactorF.fStart = 0.7f;
					InsertFactorF.fEnd = 0.0f;
					o->m_Interpolates.m_vecInterpolatesAlpha.push_back( InsertFactorF );		// 2#
					// 보간컨테이너에 ALPHA 적재 ----------------------------------------------- 

					// 보간컨테이너에 SCALE 적재 ----------------------------------------------- 					
					InsertFactorF.fRateStart = 0.0f;
					InsertFactorF.fRateEnd	 = 1.0f;
					InsertFactorF.fStart = o->Scale;
					InsertFactorF.fEnd = o->Scale;
					o->m_Interpolates.m_vecInterpolatesScale.push_back( InsertFactorF );		// 2#
					// 보간컨테이너에 SCALE 적재 ----------------------------------------------- 
				}
				else	// SubType == 1
				{
					switch(o->Type)
					{
						case MODEL_SWORDRIGHT01_EMPIREGUARDIAN_BOSS_GAION_:		// 블러드어택
						{
							if ( o->SubType==1 )	// 블러드어택 Animation
							{
								const int	TOTAL_LIFETIME = 35;
								vec3_t		v3PosProcess01, v3PosProcessFinal, 
											v3DirModify, v3PosModify,
											v3PosTargetModify;
								
								o->ExtState		= TOTAL_LIFETIME;			// TotalLife 미사용중인 임의의 변수 사용.
								o->LifeTime     = TOTAL_LIFETIME;
								o->Gravity      = 0.0f;
								o->HiddenMesh   = 1;
								o->Distance		= 0.0f;
								o->Alpha		= 1.0f;
								o->ChromeEnable = false;	// Owner Bone에 Attach하여 렌더링 할것인지?
								
								// ㄱ. 움직임 각도 백업. 모델의 각도 + 자신의 상대 각도
								Vector ( o->Angle[0], o->Angle[1], o->Angle[2], o->HeadAngle );	
								
								VectorCopy ( o->Position, o->StartPosition );	// ㄴ. 시작 위치점 백업.
								
								// 1. 방향
								VectorSubtract( o->Light, o->Position, o->Direction );
								
								// 2. 기본 거리 값 지정
								o->Distance = sqrt(o->Direction[0] * o->Direction[0] +
									o->Direction[1] * o->Direction[1] + 
									o->Direction[2] * o->Direction[2]);

								VectorDivFSelf( o->Direction, o->Distance );

								// 2.5. 두케릭간 실거리 검사 : 두 케릭 높이를 동일하게 맞추고 실거리 검사.
								float	fDistanceResult = 0;
								vec3_t	v3DirResult;
								
								// 3-0. Dir을 뒤로 늘려야하는데 목표점이 시작점보다 아래에 있으므로 땅밑으로 꺼지는 문제를
								//      해결하기 위해 시작점의 높이값을 목표점이랑 동일하게 해준다.
								
								// 3-0- 방식 1.
								vec3_t v3PosModify02;
								float	fTotalDist = 1700.0f, fFirstDist = 0.0f, fRateFirstDist = 0.0f;
								VectorCopy(o->Owner->Position, v3PosModify02);
								VectorCopy( o->Position,v3PosModify );
								
								v3PosModify02[2] = o->Light[2];
								v3PosModify[2] = o->Light[2];
								VectorCopy( o->Light, v3PosTargetModify );
								v3PosModify[2] = v3PosModify[2] + 100.0f;
								v3PosModify02[2] = v3PosModify02[2] + 100.0f;
								v3PosTargetModify[2] = v3PosTargetModify[2] + 100.0f;
								VectorDistNormalize( v3PosModify02,v3PosTargetModify,v3DirModify);
								
								// 3-1. 첫번째 위치값
								fDistanceResult = o->Distance * 0.3f;
								fFirstDist = fDistanceResult;
								VectorMulF( v3DirModify, fDistanceResult, v3DirResult );
								VectorAdd( v3PosModify, v3DirResult, v3PosProcess01 );
								
								// 3-2. 두번째 위치값
								fDistanceResult = fTotalDist;
								VectorMulF( v3DirModify, fDistanceResult, v3DirResult );
								VectorAdd( v3PosModify, v3DirResult, v3PosProcessFinal );
								
								fRateFirstDist = fFirstDist / fTotalDist;
								
								o->m_Interpolates.ClearContainer();
									
								fRateFirstDist = fFirstDist / fTotalDist;
								
								// 보간컨테이너에 각도 적재 ----------------------------------------------- 
								CInterpolateContainer::INTERPOLATE_FACTOR	InsertFactor;
								InsertFactor.fRateStart	= 0.0f;			// Start
								InsertFactor.fRateEnd	= fRateFirstDist;			// 01 Ready
								Vector(0.0f, 0.0f, 0.0f, InsertFactor.v3Start);
								Vector(90.0f, 0.0f, 0.0f, InsertFactor.v3End);
								o->m_Interpolates.m_vecInterpolatesAngle.push_back( InsertFactor );		// 1#

								InsertFactor.fRateStart	= fRateFirstDist;			// 02 First Final
								InsertFactor.fRateEnd	= 1.01f;			
								Vector(90.0f, 0.0f, 0.0f, InsertFactor.v3Start);
								Vector(90.0f, 0.0f, 1560.0f, InsertFactor.v3End);
								//Vector(90.0f, 0.0f, 1000.0f, InsertFactor.v3End);
								o->m_Interpolates.m_vecInterpolatesAngle.push_back( InsertFactor );		// 2#
								// 보간컨테이너에 각도 적재 ----------------------------------------------- 
								
								
								// 보간컨테이너에 위치 적재 ----------------------------------------------- 
								InsertFactor.fRateStart	= 0.0f;
								InsertFactor.fRateEnd	= fRateFirstDist;
								VectorCopy(o->Position, InsertFactor.v3Start);
								VectorCopy(v3PosProcess01, InsertFactor.v3End);
								o->m_Interpolates.m_vecInterpolatesPos.push_back( InsertFactor );		// 2#

 								InsertFactor.fRateStart	= fRateFirstDist;
 								InsertFactor.fRateEnd	= 1.01f;
 								VectorCopy(v3PosProcess01, InsertFactor.v3Start);
 								VectorCopy(v3PosProcessFinal, InsertFactor.v3End);
 								o->m_Interpolates.m_vecInterpolatesPos.push_back( InsertFactor );		// 2#
								// 보간컨테이너에 위치 적재 ----------------------------------------------- 

								// 초기값 설정
								o->m_Interpolates.GetAngleCurrent(o->Angle, 0.0f);		// 각도
								o->m_Interpolates.GetPosCurrent(o->Position, 0.0f);		// 위치
							}
						}	// MODEL_SWORDRIGHT01_EMPIREGUARDIAN_BOSS_GAION_
						break;
					case MODEL_SWORDLEFT01_EMPIREGUARDIAN_BOSS_GAION_:		// 블러드어택
						{
							if ( o->SubType==1 )	// 블러드어택 Animation
							{
								const int	TOTAL_LIFETIME = 35;
								vec3_t		v3PosProcess01, v3PosProcessFinal, 
									v3DirModify, v3PosModify,
									v3PosTargetModify;
								
								o->ExtState		= TOTAL_LIFETIME;			// TotalLife 미사용중인 임의의 변수 사용.
								o->LifeTime     = TOTAL_LIFETIME;
								o->Gravity      = 0.0f;
								o->HiddenMesh   = 1;
								o->Distance		= 0.0f;
								o->Alpha		= 1.0f;
								o->ChromeEnable = false;	// Owner Bone에 Attach하여 렌더링 할것인지?
								
								// ㄱ. 움직임 각도 백업. 모델의 각도 + 자신의 상대 각도
								Vector ( o->Angle[0], o->Angle[1], o->Angle[2], o->HeadAngle );	
								
								VectorCopy ( o->Position, o->StartPosition );	// ㄴ. 시작 위치점 백업.
								
								// 1. 방향
								VectorSubtract( o->Light, o->Position, o->Direction );
								
								// 2. 기본 거리 값 지정
								o->Distance = sqrt(o->Direction[0] * o->Direction[0] +
									o->Direction[1] * o->Direction[1] + 
									o->Direction[2] * o->Direction[2]);
								
								VectorDivFSelf( o->Direction, o->Distance );
								
								// 3. 목표 위치값을 타겟케릭터위치값 방향벡터 방향으로 초과한 위치로 설정.
								float	fDistanceResult = 0;
								vec3_t	v3DirResult;
								
								// 3-0. Dir을 뒤로 늘려야하는데 목표점이 시작점보다 아래에 있으므로 땅밑으로 꺼지는 문제를
								//      해결하기 위해 시작점의 높이값을 목표점이랑 동일하게 해준다.
								
								// 3-0- 방식 1.
								vec3_t v3PosModify02;
								float	fTotalDist = 1700.0f, fFirstDist = 0.0f, fRateFirstDist = 0.0f;
								VectorCopy(o->Owner->Position, v3PosModify02);
								VectorCopy( o->Position,v3PosModify );
								
								v3PosModify02[2] = o->Light[2];
								v3PosModify[2] = o->Light[2];
								VectorCopy( o->Light, v3PosTargetModify );
								v3PosModify[2] = v3PosModify[2] + 100.0f;
								v3PosModify02[2] = v3PosModify02[2] + 100.0f;
								v3PosTargetModify[2] = v3PosTargetModify[2] + 100.0f;
								VectorDistNormalize( v3PosModify02,v3PosTargetModify,v3DirModify);
								
								// 3-1. 첫번째 위치값
								fDistanceResult = o->Distance * 0.3f;
								fFirstDist = fDistanceResult;
								VectorMulF( v3DirModify, fDistanceResult, v3DirResult );
								VectorAdd( v3PosModify, v3DirResult, v3PosProcess01 );
								
								// 3-2. 두번째 위치값
								fDistanceResult = fTotalDist;
								VectorMulF( v3DirModify, fDistanceResult, v3DirResult );
								VectorAdd( v3PosModify, v3DirResult, v3PosProcessFinal );

								fRateFirstDist = fFirstDist / fTotalDist;
								
								o->m_Interpolates.ClearContainer();
								
								// 보간컨테이너에 각도 적재 ----------------------------------------------- 
								CInterpolateContainer::INTERPOLATE_FACTOR	InsertFactor;
								InsertFactor.fRateStart	= 0.0f;			// Start
								InsertFactor.fRateEnd	= fRateFirstDist;			// 01 Ready
								Vector(0.0f, 0.0f, 0.0f, InsertFactor.v3Start);
								Vector(90.0f, 0.0f, 0.0f, InsertFactor.v3End);
								o->m_Interpolates.m_vecInterpolatesAngle.push_back( InsertFactor );		// 1#
								
								InsertFactor.fRateStart	= fRateFirstDist;			// 02 First Final
								InsertFactor.fRateEnd	= 1.01f;			
								Vector(90.0f, 0.0f, 0.0f, InsertFactor.v3Start);
								Vector(90.0f, 0.0f, -1560.0f, InsertFactor.v3End);
								//Vector(90.0f, 0.0f, -1000.0f, InsertFactor.v3End);
								o->m_Interpolates.m_vecInterpolatesAngle.push_back( InsertFactor );		// 2#
								// 보간컨테이너에 각도 적재 ----------------------------------------------- 
								
								
								// 보간컨테이너에 위치 적재 ----------------------------------------------- 
								InsertFactor.fRateStart	= 0.0f;
								InsertFactor.fRateEnd	= fRateFirstDist;
								VectorCopy(o->Position, InsertFactor.v3Start);
								VectorCopy(v3PosProcess01, InsertFactor.v3End);
								o->m_Interpolates.m_vecInterpolatesPos.push_back( InsertFactor );		// 2#

								InsertFactor.fRateStart	= fRateFirstDist;
								InsertFactor.fRateEnd	= 1.01f;
								VectorCopy(v3PosProcess01, InsertFactor.v3Start);
								VectorCopy(v3PosProcessFinal, InsertFactor.v3End);
								o->m_Interpolates.m_vecInterpolatesPos.push_back( InsertFactor );		// 2#
								// 보간컨테이너에 위치 적재 ----------------------------------------------- 

								// 보간컨테이너에 SCALE 적재 ----------------------------------------------- 

								// 보간컨테이너에 SCALE 적재 ----------------------------------------------- 
								
								
								// 						InsertFactor.fRateStart	= 0.4f;
								// 						InsertFactor.fRateEnd	= 1.01f;
								// 						VectorCopy(v3PosProcess01, InsertFactor.v3Start);
								// 						VectorCopy(v3PosProcessFinal, InsertFactor.v3End);
								// 						o->m_Interpolates.m_vecInterpolatesPos.push_back( InsertFactor );		// 2#
								// 보간컨테이너에 위치 적재 ----------------------------------------------- 

								// 초기값 설정
								o->m_Interpolates.GetAngleCurrent(o->Angle, 0.0f);		// 각도
								o->m_Interpolates.GetPosCurrent(o->Position, 0.0f);		// 위치
							}
						}	// MODEL_SWORDLEFT01_EMPIREGUARDIAN_BOSS_GAION_
						break;
					case MODEL_SWORDRIGHT02_EMPIREGUARDIAN_BOSS_GAION_:		// 일반공격
						{
							if ( o->SubType==1 )	// 일반공격 Animation
							{
								//const int	TOTAL_LIFETIME = 24;
								const int	TOTAL_LIFETIME = 30;
								vec3_t		v3PosProcess01, v3PosProcessFinal, 
											v3AngleOffset,  
											v3Dir, 
											v3DirPower, 
											v3DirModify,
											v3DirModifyR;
								
								o->ExtState		= TOTAL_LIFETIME;			// TotalLife 미사용중인 임의의 변수 사용.
								o->LifeTime     = TOTAL_LIFETIME;
								o->Gravity      = 0.2f;
								o->HiddenMesh   = 1;
								o->Distance		= 0.0f;
								o->Alpha		= 1.0f;
								o->ChromeEnable = false;	// Owner Bone에 Attach하여 렌더링 할것인지?

								// ㄱ. 움직임 각도 백업. 모델의 각도 + 자신의 상대 각도
								Vector ( o->Angle[0], o->Angle[1], o->Angle[2], o->HeadAngle );	
								
								VectorCopy ( o->Position, o->StartPosition );	// ㄴ. 시작 위치점 백업.
								
								// 1. 방향	// 수평적 방향만을 연산한다.
								vec3_t	v3PosStartModify, v3PosEndModify;
								VectorCopy(o->Light, v3PosEndModify);
								VectorCopy(o->StartPosition, v3PosStartModify);
								v3PosStartModify[2] = v3PosEndModify[2];		

								VectorSubtract( v3PosEndModify, v3PosStartModify,v3DirModify );
								VectorSubtract( o->Light, o->StartPosition, v3Dir );
								
								// 2. 기본 거리 값 지정
								o->Distance = sqrt(v3Dir[0] * v3Dir[0] +
									v3Dir[1] * v3Dir[1] + 
									v3Dir[2] * v3Dir[2]);
								
								v3Dir[0] /= o->Distance;
								v3Dir[1] /= o->Distance;
								v3Dir[2] /= o->Distance;

								VectorNormalize(v3DirModify);
								VectorCopy(v3Dir,o->Direction);

								// 위치점을 계산한다.	- 01
								Vector( 0.0f, 0.0f, -40.0f, v3AngleOffset );
								AngleMatrix(v3AngleOffset,Matrix);
								VectorRotate(v3DirModify,Matrix,v3DirModifyR);
								VectorMulF( v3DirModifyR, o->Distance * 0.4f, v3DirPower );
								VectorAdd( o->StartPosition, v3DirPower, v3PosProcess01 );
								v3PosProcess01[2] -= 100.0f;
								
								// 최종 위치점을 약간 보정하여 계산. offset Angle	- Final
								//Vector( 0.0f, 0.0f, 19.0f, v3AngleOffset );
								Vector( 0.0f, 0.0f, 24.0f, v3AngleOffset );
								AngleMatrix(v3AngleOffset,Matrix);
								VectorRotate(v3DirModify,Matrix,v3DirModifyR);
								//VectorMulF( v3DirModifyR, o->Distance * 1.4f, v3DirPower );
								//VectorMulF( v3DirModifyR, o->Distance + 200.0f, v3DirPower );
								VectorMulF( v3DirModifyR, o->Distance + 700.0f, v3DirPower );
								VectorAdd( o->StartPosition, v3DirPower, v3PosProcessFinal );
								//v3PosProcessFinal[2] -= 230.0f;
								v3PosProcessFinal[2] -= 290.0f;
								
								o->m_Interpolates.ClearContainer();
								
								// 보간컨테이너에 각도 적재 ----------------------------------------------- 
								CInterpolateContainer::INTERPOLATE_FACTOR	InsertFactor;
								InsertFactor.fRateStart	= 0.0f;			// Start
								InsertFactor.fRateEnd	= 0.32f;			// 01 Ready
								Vector(0.0f, 0.0f, 0.0f, InsertFactor.v3Start);
								//Vector(110.0f, 0.0f, 0.0f, InsertFactor.v3End);
								Vector(90.0f, 0.0f, 0.0f, InsertFactor.v3End);
								o->m_Interpolates.m_vecInterpolatesAngle.push_back( InsertFactor );		// 1#
								
								InsertFactor.fRateStart	= 0.32f;			// 01 Ready
								InsertFactor.fRateEnd	= 1.01f;			// 02 First Final
								//Vector(110.0f, 0.0f, 0.0f, InsertFactor.v3Start);
								Vector(90.0f, 0.0f, 0.0f, InsertFactor.v3Start);
								Vector(90.0f, 0.0f, 340.0f, InsertFactor.v3End);
								o->m_Interpolates.m_vecInterpolatesAngle.push_back( InsertFactor );		// 2#
								// 보간컨테이너에 각도 적재 ----------------------------------------------- 
								
								
								// 보간컨테이너에 위치 적재 ----------------------------------------------- 
								InsertFactor.fRateStart	= 0.0f;			
								InsertFactor.fRateEnd	= 0.32f;
								VectorCopy(o->Position, InsertFactor.v3Start);
								VectorCopy(v3PosProcess01, InsertFactor.v3End);
								o->m_Interpolates.m_vecInterpolatesPos.push_back( InsertFactor );		// 2#
								
								InsertFactor.fRateStart	= 0.32f;			
								InsertFactor.fRateEnd	= 1.01f;		
								VectorCopy(v3PosProcess01, InsertFactor.v3Start);
								VectorCopy(v3PosProcessFinal, InsertFactor.v3End);
								o->m_Interpolates.m_vecInterpolatesPos.push_back( InsertFactor );		// 2#
								// 보간컨테이너에 위치 적재 ----------------------------------------------- 

								// 보간컨테이너에 SCALE 적재 ----------------------------------------------- 

								// 보간컨테이너에 SCALE 적재 ----------------------------------------------- 

								// 보간컨테이너에 ALPHA 적재 ----------------------------------------------- 
								CInterpolateContainer::INTERPOLATE_FACTOR_F InsertFactorF;
								InsertFactorF.fRateStart	= 0.0f;
								InsertFactorF.fRateEnd	= 0.15f;
								InsertFactorF.fStart = 1.0f;
								InsertFactorF.fEnd = 1.0f;
								o->m_Interpolates.m_vecInterpolatesAlpha.push_back( InsertFactorF );		// 2#
								
								InsertFactorF.fRateStart	= 0.15f;
								InsertFactorF.fRateEnd	= 0.75f;
								InsertFactorF.fStart = 1.0f;
								InsertFactorF.fEnd = 1.0f;
								o->m_Interpolates.m_vecInterpolatesAlpha.push_back( InsertFactorF );		// 2#
								
								InsertFactorF.fRateStart	= 0.75f;
								InsertFactorF.fRateEnd	= 1.01f;
								InsertFactorF.fStart = 1.0f;
								InsertFactorF.fEnd = 0.0f;
								o->m_Interpolates.m_vecInterpolatesAlpha.push_back( InsertFactorF );		// 2#
								// 보간컨테이너에 ALPHA 적재 ----------------------------------------------- 

								// 초기값 설정
								o->m_Interpolates.GetAngleCurrent(o->Angle, 0.0f);		// 각도
								o->m_Interpolates.GetPosCurrent(o->Position, 0.0f);		// 위치
								o->m_Interpolates.GetAlphaCurrent(o->Alpha, 0.0f);		// 알파
							}
						}	// MODEL_SWORDRIGHT02_EMPIREGUARDIAN_BOSS_GAION_
						break;
					case MODEL_SWORDLEFT02_EMPIREGUARDIAN_BOSS_GAION_:		// 일반공격
						{
							if ( o->SubType==1 )	// 일반공격 Animation
							{
								//const int	TOTAL_LIFETIME = 22;
								const int	TOTAL_LIFETIME = 30;
								vec3_t		v3PosProcess01, v3PosProcessFinal, 
											v3AngleOffset,  
											v3Dir, 
											v3DirPower, 
											v3DirModify,
											v3DirModifyR;
								
								o->ExtState		= TOTAL_LIFETIME;			// TotalLife 미사용중인 임의의 변수 사용.
								o->LifeTime     = TOTAL_LIFETIME;
								o->Gravity      = 0.2f;
								o->HiddenMesh   = 1;
								o->Distance		= 0.0f;
								o->Alpha		= 1.0f;
								o->ChromeEnable = false;	// Owner Bone에 Attach하여 렌더링 할것인지?

								// ㄱ. 움직임 각도 백업. 모델의 각도 + 자신의 상대 각도
								Vector ( o->Angle[0], o->Angle[1], o->Angle[2], o->HeadAngle );	
								
								VectorCopy ( o->Position, o->StartPosition );	// ㄴ. 시작 위치점 백업.
								
								// 1. 방향	// 수평적 방향만을 연산한다.
								vec3_t	v3PosStartModify, v3PosEndModify;
								VectorCopy(o->Light, v3PosEndModify);
								VectorCopy(o->StartPosition, v3PosStartModify);
								v3PosStartModify[2] = v3PosEndModify[2];		

								VectorSubtract( v3PosEndModify, v3PosStartModify,v3DirModify );
								VectorSubtract( o->Light, o->StartPosition, v3Dir );
								
								// 2. 기본 거리 값 지정
								o->Distance = sqrt(v3Dir[0] * v3Dir[0] +
									v3Dir[1] * v3Dir[1] + 
									v3Dir[2] * v3Dir[2]);
								
								v3Dir[0] /= o->Distance;
								v3Dir[1] /= o->Distance;
								v3Dir[2] /= o->Distance;

								VectorNormalize(v3DirModify);
								VectorCopy(v3Dir,o->Direction);

								// 위치점을 계산한다.	- 01
								Vector( 0.0f, 0.0f, 50.0f, v3AngleOffset );
								AngleMatrix(v3AngleOffset,Matrix);
								VectorRotate(v3DirModify,Matrix,v3DirModifyR);
								VectorMulF( v3DirModifyR, o->Distance * 0.4f, v3DirPower );
								VectorAdd( o->StartPosition, v3DirPower, v3PosProcess01 );
								v3PosProcess01[2] -= 100.0f;
								
								// 최종 위치점을 약간 보정하여 계산. offset Angle	- Final
								//Vector( 0.0f, 0.0f, -19.0f, v3AngleOffset );
								Vector( 0.0f, 0.0f, -26.0f, v3AngleOffset );
								AngleMatrix(v3AngleOffset,Matrix);
								VectorRotate(v3DirModify,Matrix,v3DirModifyR);
								//VectorMulF( v3DirModifyR, o->Distance * 1.4f, v3DirPower );
								//VectorMulF( v3DirModifyR, o->Distance + 200.0f, v3DirPower );
								VectorMulF( v3DirModifyR, o->Distance + 700.0f, v3DirPower );
								VectorAdd( o->StartPosition, v3DirPower, v3PosProcessFinal );
								//v3PosProcessFinal[2] -= 230.0f;
								v3PosProcessFinal[2] -= 280.0f;
								
								o->m_Interpolates.ClearContainer();
								
								// 보간컨테이너에 각도 적재 ----------------------------------------------- 
								CInterpolateContainer::INTERPOLATE_FACTOR	InsertFactor;
								InsertFactor.fRateStart	= 0.0f;			// Start
								InsertFactor.fRateEnd	= 0.4f;			// 01 Ready
								Vector(0.0f, 0.0f, 0.0f, InsertFactor.v3Start);
								Vector(110.0f, 0.0f, 0.0f, InsertFactor.v3End);
								o->m_Interpolates.m_vecInterpolatesAngle.push_back( InsertFactor );		// 1#
								
								InsertFactor.fRateStart	= 0.4f;			// 01 Ready
								InsertFactor.fRateEnd	= 1.01f;			// 02 First Final
								Vector(110.0f, 0.0f, 0.0f, InsertFactor.v3Start);
								Vector(60.0f, 0.0f, -300.0f, InsertFactor.v3End);
								o->m_Interpolates.m_vecInterpolatesAngle.push_back( InsertFactor );		// 2#
								// 보간컨테이너에 각도 적재 ----------------------------------------------- 
								
								
								// 보간컨테이너에 위치 적재 ----------------------------------------------- 
								InsertFactor.fRateStart	= 0.0f;			
								InsertFactor.fRateEnd	= 0.33f;
								VectorCopy(o->Position, InsertFactor.v3Start);
								VectorCopy(v3PosProcess01, InsertFactor.v3End);
								o->m_Interpolates.m_vecInterpolatesPos.push_back( InsertFactor );		// 2#
								
								InsertFactor.fRateStart	= 0.33f;			
								InsertFactor.fRateEnd	= 1.01f;		
								VectorCopy(v3PosProcess01, InsertFactor.v3Start);
								VectorCopy(v3PosProcessFinal, InsertFactor.v3End);
								o->m_Interpolates.m_vecInterpolatesPos.push_back( InsertFactor );		// 2#
								// 보간컨테이너에 위치 적재 ----------------------------------------------- 

								// 보간컨테이너에 ALPHA 적재 ----------------------------------------------- 
								CInterpolateContainer::INTERPOLATE_FACTOR_F InsertFactorF;
								InsertFactorF.fRateStart	= 0.0f;
								InsertFactorF.fRateEnd	= 0.15f;
								InsertFactorF.fStart = 1.0f;
								InsertFactorF.fEnd = 1.0f;
								o->m_Interpolates.m_vecInterpolatesAlpha.push_back( InsertFactorF );		// 2#
								
								InsertFactorF.fRateStart	= 0.15f;
								InsertFactorF.fRateEnd	= 0.75f;
								InsertFactorF.fStart = 1.0f;
								InsertFactorF.fEnd = 1.0f;
								o->m_Interpolates.m_vecInterpolatesAlpha.push_back( InsertFactorF );		// 2#
								
								InsertFactorF.fRateStart	= 0.75f;
								InsertFactorF.fRateEnd	= 1.01f;
								InsertFactorF.fStart = 1.0f;
								InsertFactorF.fEnd = 0.0f;
								o->m_Interpolates.m_vecInterpolatesAlpha.push_back( InsertFactorF );		// 2#
								// 보간컨테이너에 ALPHA 적재 ----------------------------------------------- 

								// 초기값 설정
								o->m_Interpolates.GetAngleCurrent(o->Angle, 0.0f);		// 각도
								o->m_Interpolates.GetPosCurrent(o->Position, 0.0f);		// 위치
								o->m_Interpolates.GetAlphaCurrent(o->Alpha, 0.0f);		// 알파
							}
						}	// MODEL_SWORDLEFT02_EMPIREGUARDIAN_BOSS_GAION_
						break;		
					case MODEL_SWORDMAIN01_EMPIREGUARDIAN_BOSS_GAION_:		// 
						{
							if ( o->SubType==1 )
							{
								// 기존 방식 BACKUP
								const int	TOTAL_LIFETIME = 10;
								
								o->ExtState		= TOTAL_LIFETIME;			// TotalLife 미사용중인 임의의 변수 사용.
								o->LifeTime     = TOTAL_LIFETIME;
								o->ChromeEnable = true;	// Owner Bone에 Attach하여 렌더링 할것인지?
								//o->Scale		= 0.8f;

								o->m_Interpolates.ClearContainer();

								// 보간컨테이너에 ALPHA 적재 ----------------------------------------------- 
								CInterpolateContainer::INTERPOLATE_FACTOR_F InsertFactorF;
								InsertFactorF.fRateStart	= 0.0f;
								InsertFactorF.fRateEnd	= 1.01f;
								InsertFactorF.fStart = 1.0f;
								InsertFactorF.fEnd = 0.0f;
								o->m_Interpolates.m_vecInterpolatesAlpha.push_back( InsertFactorF );		
								// 보간컨테이너에 ALPHA 적재 ----------------------------------------------- 
							}
						}	// MODEL_SWORDMAIN01_EMPIREGUARDIAN_BOSS_GAION_
						break;
					case MODEL_EMPIREGUARDIANBOSS_FRAMESTRIKE:
						{
							if ( o->SubType==1 )	
							{
								const int	TOTAL_LIFETIME = 36;
								vec3_t		v3PosProcess01, 
									v3DirModify, 
									v3PosModify,
									v3PosTargetModify;
								
								o->ExtState		= TOTAL_LIFETIME;			// TotalLife 미사용중인 임의의 변수 사용.
								o->LifeTime     = TOTAL_LIFETIME;
								o->Gravity      = 0.0f;
								o->HiddenMesh   = 1;
								o->Distance		= 0.0f;
								o->Alpha		= 1.0f;
								o->ChromeEnable	= false;					// Hierarchical 하지 않게 처리 한다.
								o->Scale		= 1.0f;
								o->Visible		= false;
								o->Velocity		= 0.4f;

#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
								BMD*	b = &Models[o->Type];
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
								//b->Velocity = 10.f;
								
								// ㄱ. 움직임 각도 백업. 모델의 각도 + 자신의 상대 각도
								Vector ( o->Angle[0], o->Angle[1], o->Angle[2], o->HeadAngle );	
								
								// 1. 방향
								VectorSubtract( o->Light, o->Position, o->Direction );
								
								// 2. 기본 거리 값 지정
								o->Distance = sqrt(o->Direction[0] * o->Direction[0] +
									o->Direction[1] * o->Direction[1] + 
									o->Direction[2] * o->Direction[2]);
								
								VectorDivFSelf( o->Direction, o->Distance );
								
								// 3. 목표 위치값을 타겟케릭터위치값 방향벡터 방향으로 초과한 위치로 설정.
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
								float	fDistanceResult = 0;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
								
								// 3-0. Dir을 뒤로 늘려야하는데 목표점이 시작점보다 아래에 있으므로 땅밑으로 꺼지는 문제를
								//      해결하기 위해 시작점의 높이값을 목표점이랑 동일하게 해준다.
								
								// 3-0- 방식 1.
								VectorCopy( o->Owner->Position, v3PosModify );
								
								v3PosModify[2] = o->Light[2];
								VectorCopy( o->Light, v3PosTargetModify );
								
								v3PosModify[2] = v3PosModify[2] + 100.0f;
								v3PosTargetModify[2] = v3PosTargetModify[2] + 100.0f;
								VectorDistNormalize( v3PosTargetModify, v3PosModify,v3DirModify);
								
								//vc = va + scale*vb
								VectorMA( v3PosTargetModify,200.0f,v3DirModify,v3PosProcess01 );

								VectorCopy ( v3PosProcess01, o->StartPosition );	// ㄴ. 시작 위치점 백업.
								
								o->m_Interpolates.ClearContainer();
								
								// 보간컨테이너에 각도 적재 ----------------------------------------------- 
								CInterpolateContainer::INTERPOLATE_FACTOR	InsertFactor;
								InsertFactor.fRateStart	= 0.0f;			
								InsertFactor.fRateEnd	= 0.28f;		
								Vector(0.0f, 0.0f, 0.0f, InsertFactor.v3Start);
								Vector(0.0f, 0.0f, 0.0f, InsertFactor.v3End);
								o->m_Interpolates.m_vecInterpolatesAngle.push_back( InsertFactor );		// 1#
								
								InsertFactor.fRateStart	= 0.28f;			
								InsertFactor.fRateEnd	= 0.36f;			
								Vector(0.0f, 0.0f, 0.0f, InsertFactor.v3Start);
								Vector(-90.0f, 0.0f, 0.0f, InsertFactor.v3End);
								o->m_Interpolates.m_vecInterpolatesAngle.push_back( InsertFactor );		// 2#

								InsertFactor.fRateStart	= 0.36f;
								InsertFactor.fRateEnd	= 1.01f;			
								Vector(-90.0f, 0.0f, 0.0f, InsertFactor.v3Start);
								Vector(-90.0f, 0.0f, 880.0f, InsertFactor.v3End);
								o->m_Interpolates.m_vecInterpolatesAngle.push_back( InsertFactor );		// 2#
								// 보간컨테이너에 각도 적재 ----------------------------------------------- 

								// 보간컨테이너에 위치 적재 ----------------------------------------------- 						
								InsertFactor.fRateStart	= 0.0f;
								InsertFactor.fRateEnd	= 1.01f;		
								VectorCopy(v3PosProcess01, InsertFactor.v3Start);
								VectorCopy(v3PosProcess01, InsertFactor.v3End);
								o->m_Interpolates.m_vecInterpolatesPos.push_back( InsertFactor );		// 2#
								// 보간컨테이너에 위치 적재 ----------------------------------------------- 

								// 보간컨테이너에 SCALE 적재 ----------------------------------------------- 
								CInterpolateContainer::INTERPOLATE_FACTOR_F	InsertFactorF;
								InsertFactorF.fRateStart	= 0.0f;
								InsertFactorF.fRateEnd	= 1.01f;
								InsertFactorF.fStart = o->Scale * 1.0f;
								InsertFactorF.fEnd = o->Scale * 1.0f;
								o->m_Interpolates.m_vecInterpolatesScale.push_back( InsertFactorF );		// 2#
								// 보간컨테이너에 SCALE 적재 ----------------------------------------------- 

								// 보간컨테이너에 ALPHA 적재 ----------------------------------------------- 
								InsertFactorF.fRateStart	= 0.0f;
								InsertFactorF.fRateEnd	= 0.15f;
								InsertFactorF.fStart = 0.0f;
								InsertFactorF.fEnd = 1.0f;
								o->m_Interpolates.m_vecInterpolatesAlpha.push_back( InsertFactorF );		// 2#
								
								InsertFactorF.fRateStart	= 0.15f;
								InsertFactorF.fRateEnd	= 0.75f;
								InsertFactorF.fStart = 1.0f;
								InsertFactorF.fEnd = 1.0f;
								o->m_Interpolates.m_vecInterpolatesAlpha.push_back( InsertFactorF );		// 2#
								
								InsertFactorF.fRateStart	= 0.75f;
								InsertFactorF.fRateEnd	= 1.01f;
								InsertFactorF.fStart = 1.0f;
								InsertFactorF.fEnd = 0.0f;
								o->m_Interpolates.m_vecInterpolatesAlpha.push_back( InsertFactorF );		// 2#
								// 보간컨테이너에 ALPHA 적재 ----------------------------------------------- 

								
								// 초기값 설정
								o->m_Interpolates.GetAngleCurrent(o->Angle, 0.0f);		// 각도
								o->m_Interpolates.GetPosCurrent(o->Position, 0.0f);		// 위치
								o->m_Interpolates.GetAlphaCurrent(o->Alpha, 0.0f);		// 알파
								o->m_Interpolates.GetScaleCurrent(o->Scale, 0.0f);		// 스케일
							}
						}		
					}
				}
				// 가이온 모든 이펙트 생성 끝부분
				// ==========================================================================================<
			}
			break;
#endif // LDS_ADD_EG_4_MONSTER_WORLDBOSS_GAIONKALEIN
#ifdef LDS_ADD_EG_4_MONSTER_JELINT
			case MODEL_EMPIREGUARDIAN_BLOW_OF_DESTRUCTION:	// MODEL_BLOW_OF_DESTRUCTION그대로 참조 : Scale 변경 가능
				{
					if(o->SubType == 0)
					{
						o->LifeTime = 40;
						
						// 부모오프젝트가 바라보는 방향으로 조금 이동
						assert(o->Owner && "부모 포인트를 넣어줘야 합니다.!");
						vec3_t vPos, vPos2;
						float Matrix[3][4];
						Vector(-20.f, -100.f, 0.f, vPos);
						AngleMatrix(o->Owner->Angle, Matrix);
						VectorRotate(vPos, Matrix, vPos2);	
						VectorAdd(vPos2, o->Position, o->Position);
						
						// 타켓 위치 복사
						VectorCopy(o->Light, o->StartPosition);
						
						// 라이트값 입력
						Vector(1.2f, 1.2f, 1.2f, o->Light);
						
						// 타켓에 이펙트 발생
						CreateEffect(MODEL_EMPIREGUARDIAN_BLOW_OF_DESTRUCTION, 
							o->StartPosition, o->Angle, 
							o->Light, 1, o->Owner, -1, 0, 
							0, 0, o->Scale );
					}
					else if(o->SubType == 1)
					{
						o->LifeTime = 40;
						Vector(1.2f, 1.2f, 1.2f, o->Light);
						o->Position[2] = 150.f;
						o->Scale = o->Scale * 2.f;
					}					
				}
				break;
#endif // LDS_ADD_EG_4_MONSTER_JELINT
#ifdef PBG_ADD_CHARACTERSLOT
			case MODEL_SLOT_LOCK:
				{
					o->Scale *= 2.0f;
					o->Position[2] = 300.0f;
					float temptime = sinf(WorldTime * 0.0005f);
					Vector(0.0f, 0.0f, o->Angle[2] + temptime*10, o->Angle);
					o->LifeTime = 1000;
				}
				break;
#endif //PBG_ADD_CHARACTERSLOT
#ifdef KJH_ADD_09SUMMER_EVENT
			case BITMAP_RING_OF_GRADATION:
				{
					if( o->SubType == 0 )
					{
						o->LifeTime = 20;
					}
				}break;
			case MODEL_EFFECT_UMBRELLA_DIE:
				{
					if( o->SubType == 0 )
					{
						o->LifeTime = 30;
					}
				}break;
			case MODEL_EFFECT_UMBRELLA_GOLD:
				{
					o->LifeTime = rand()%10+50;
					o->Scale = 2.0f + (rand()%10-5)*0.2f;
				
					o->Angle[0] = (float)(rand()%360);
					o->Angle[1] = (float)(rand()%360);
					o->Angle[2] = (float)(rand()%360);
					AngleMatrix(o->Angle, Matrix);
					o->Gravity = (float)(rand()%10+10);
					vec3_t p;
					Vector((float)(rand()%10-5)*0.1f, (float)(rand()%60-30)*0.1f, 0.0f, p);
					VectorScale(p, 1.2f, p);
					VectorRotate(p, Matrix, o->Direction);

					o->Position[2] = RequestTerrainHeight(o->Position[0],o->Position[1]) + 100.f;
				}break;
#endif // KJH_ADD_09SUMMER_EVENT
#ifdef KJH_ADD_EG_MONSTER_GUARDIANDEFENDER_EFFECT
			case MODEL_EFFECT_EG_GUARDIANDEFENDER_ATTACK2:		// 제국수호군 방패병 공격2 이팩트
				{
					o->LifeTime = 20;
					o->Scale = 0.9f;
				}break;
#endif // KJH_ADD_EG_MONSTER_GUARDIANDEFENDER_EFFECT
#ifdef PBG_ADD_AURA_EFFECT
			case MODEL_EFFECT_SD_AURA:
				{
					o->LifeTime = 1000;
					o->Scale = 1.0f;
				}
				break;
#endif //PBG_ADD_AURA_EFFECT
#ifdef LDK_ADD_EXTENSIONMAP_BOSS_MEDUSA_WAND_EFFECT
			case BITMAP_WATERFALL_4:
				{
					o->LifeTime = 80;
					o->Scale = (rand()%20*0.01f)+0.1f;
					o->Angle[0] = (float)(rand()%360);

					o->Distance = rand()%10+5.0f;

					o->Timer = (float)(rand()%360); //degree
				}
				break;
#endif //LDK_ADD_EXTENSIONMAP_BOSS_MEDUSA_WAND_EFFECT
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
			case MODEL_WOLF_HEAD_EFFECT:
				{
					BMD* b = &Models[o->Owner->Type];
					b->TransformByObjectBone(p1, o->Owner, 27);
					VectorCopy(p1, o->Position);
					if(o->SubType == 0)
					{
						o->Scale =1.0f;
						o->LifeTime = 10.0f / Models[o->Owner->Type].Actions[PLAYER_SKILL_THRUST].PlaySpeed;
					}
					else if(o->SubType == 1)
					{
						o->Velocity = 0.1f;
						o->Scale        = 1.2f+rand()%5*0.1f;
     					o->Direction[1] = -50.f;
						o->LifeTime		= 2+rand()%2;
					}
					else if(o->SubType == 2)
					{
						o->Velocity = 0.1f;
						o->Scale        = 0.8f+rand()%3*0.1f;
     					o->Direction[1] = -40.f;
						o->LifeTime		= 3+rand()%3;
						o->Angle[2]		+= (rand()%10-5.0f);
					}
				}
				break;
			case BITMAP_SBUMB:
				{
					o->LifeTime = 20;
					o->Scale = Scale;
					BMD* b = &Models[o->Owner->Type];
					b->TransformByObjectBone(p1, o->Owner, 27);
					o->Position[2] = p1[2];
				}
				break;
			case MODEL_DOWN_ATTACK_DUMMY_L:
			case MODEL_DOWN_ATTACK_DUMMY_R:
				{
					VectorCopy(Position, o->Position);
					o->Velocity = Models[o->Owner->Type].Actions[PLAYER_SKILL_STAMP].PlaySpeed*2.0f;
					o->LifeTime = 100;
					Models[MODEL_DOWN_ATTACK_DUMMY_L].Actions[0].PlaySpeed = Models[o->Owner->Type].Actions[PLAYER_SKILL_STAMP].PlaySpeed;
					Models[MODEL_DOWN_ATTACK_DUMMY_R].Actions[0].PlaySpeed = Models[o->Owner->Type].Actions[PLAYER_SKILL_STAMP].PlaySpeed;
				}
				break;
			case BITMAP_SWORDEFF:
				{
					o->LifeTime = 200;
				}
				break;
			case MODEL_SHOCKWAVE01:
				{
					if(o->Owner == NULL)
					{
						o->Live=false;
						break;
					}

					o->Scale = Scale;
					VectorCopy(Light, o->Light);
					if(o->SubType==1)
					{
						o->Velocity = Models[o->Owner->Type].Actions[PLAYER_SKILL_DRAGONLORE].PlaySpeed;
						o->LifeTime = 5.0f / o->Velocity;
					}
					else if(o->SubType==2)
					{
						o->Velocity = 0.3f;
						o->LifeTime = 5.0f / o->Velocity;
					}
					else if(o->SubType==3)
					{
						o->Velocity = Models[o->Owner->Type].Actions[PLAYER_SKILL_DARKSIDE_READY].PlaySpeed*2.0f;
						o->LifeTime = 5.0f / o->Velocity;
						o->Scale = Scale;
						vec3_t vPos, vDir;
						VectorCopy(o->Position, vPos);
						VectorCopy(o->Owner->StartPosition, vDir);
						vPos[2] = RequestTerrainHeight(vPos[0], vPos[1]);
						vDir[2] = RequestTerrainHeight(vDir[0], vDir[1]);
						VectorSubtract(vPos, vDir, vDir);
						VectorNormalize(vDir);
						VectorScale(vDir, 30, vDir);
						VectorCopy(vDir, o->StartPosition);
						vec3_t Transpos;
						VectorScale(vDir, 5.0f, vDir);
						VectorCopy(vDir, Transpos);
						VectorAdd(o->Position, Transpos, o->Position);
					}
					else if(o->SubType==4)
					{
						o->Velocity = 0.6f;
						o->LifeTime = 12.0f;
						o->Scale = Scale;
						vec3_t vPos, vDir;
						VectorCopy(o->Position, vPos);
						VectorCopy(o->Owner->StartPosition, vDir);
						vPos[2] = RequestTerrainHeight(vPos[0], vPos[1]);
						vDir[2] = RequestTerrainHeight(vDir[0], vDir[1]);
						VectorSubtract(vPos, vDir, vDir);
						VectorNormalize(vDir);
						VectorScale(vDir, 30, vDir);
						VectorCopy(vDir, o->StartPosition);
						vec3_t Transpos;
						VectorNormalize(vDir);
						VectorScale(vDir, 150.0f, vDir);
						VectorCopy(vDir, Transpos);
						VectorAdd(o->Position, Transpos, o->Position);
					}
					else
					{
						o->Velocity = Models[o->Owner->Type].Actions[PLAYER_SKILL_GIANTSWING].PlaySpeed*2.0f;
						o->LifeTime = 5.0f / o->Velocity;
						
						vec3_t vPos, vDir;
						VectorCopy(o->Position, vPos);
						VectorCopy(o->Owner->Position, vDir);
						vPos[2] = RequestTerrainHeight(vPos[0], vPos[1]);
						vDir[2] = RequestTerrainHeight(vDir[0], vDir[1]);
						VectorSubtract(vPos, vDir, vDir);
						VectorNormalize(vDir);
						VectorScale(vDir, 30, vDir);
						VectorCopy(vDir, o->StartPosition);
					}					
				}
				break;
			case MODEL_SHOCKWAVE02:
				{
					o->Scale = Scale;
					VectorCopy(Light, o->Light);
					o->Velocity = Models[o->Owner->Type].Actions[PLAYER_SKILL_GIANTSWING].PlaySpeed*2.0f;
					o->LifeTime = 14.0f / o->Velocity;

					vec3_t vPos, vDir;
					VectorCopy(o->Position, vPos);
					VectorCopy(o->Owner->Position, vDir);
					vPos[2] = RequestTerrainHeight(vPos[0], vPos[1]);
					vDir[2] = RequestTerrainHeight(vDir[0], vDir[1]);
					VectorSubtract(vPos, vDir, vDir);
					VectorNormalize(vDir);
					VectorScale(vDir, 25, vDir);
					VectorCopy(vDir, o->StartPosition);
				}
				break;
			case BITMAP_DAMAGE1:
				{
					o->LifeTime = 20;
					o->Scale = Scale;
					BMD* b = &Models[o->Owner->Type];
					b->TransformByObjectBone(p1, o->Owner, 27);
					o->Position[2] = p1[2];
				}
				break;
			case MODEL_SHOCKWAVE_SPIN01:
				{
					o->Scale = Scale;
					VectorCopy(Light, o->Light);
					o->Velocity = Models[o->Owner->Type].Actions[PLAYER_SKILL_GIANTSWING].PlaySpeed;
					o->LifeTime = 8.0f / o->Velocity;
					if(o->SubType == 0)
					{
						o->Velocity *= 2.0f;
					}
					else if(o->SubType==1)
					{
						o->Velocity *= 3.0f;
					}
				}
				break;
			case BITMAP_EVENT_CLOUD:
				{
					o->Scale = Scale;
					o->LifeTime = 30;
					VectorCopy(Light, o->Light);
					if(o->SubType==1)
					{
						o->LifeTime = 30;
					}
				}
				break;
			case MODEL_WINDFOCE:
				{
					o->LifeTime = 50;
					VectorCopy(Light, o->Light);
					o->Scale = Scale;

					if(o->SubType == 1)
					{
						o->LifeTime = 999;
					}
					else if(o->SubType==2)
					{
						o->LifeTime = 70;
					}
					else if(o->SubType==3)
					{
						o->LifeTime = 70;
					}
				}
				break;
			case BITMAP_SHINY+4:
				{
					o->LifeTime = 16;
					o->Scale = Scale;
				}
				break;
			case BITMAP_LIGHT_RED:
				{
					o->LifeTime = 9999;
					o->Velocity = 0.0f;
					o->Scale = 1.5f;
					Vector(1.0f, 1.0f, 1.0f, o->Light);
					VectorCopy(Position, o->Position);
					if(o->SubType==1)
					{
						o->LifeTime = 50;
					}
					else if(o->SubType==3 || o->SubType==4)
					{
						o->Scale = Scale;
						o->LifeTime = 100;
						if(o->SubType==4)
						{
							o->LifeTime = 50;
						}
						vec3_t Light;
						float Luminosity = (float)(rand()%5)*0.01f;
						Vector(Luminosity+1.0f,Luminosity+0.2f,Luminosity+0.2f,Light);
						int range = 2;
						if(o->SubType==4)
						{
							range = 4;
						}
						AddTerrainLight(o->Position[0],o->Position[1], Light, range, PrimaryTerrainLight);
					}
				}
				break;
			case MODEL_WINDFOCE_MIRROR:
				{
					VectorCopy(Light, o->Light);
					o->Scale = Scale;
					o->LifeTime = 50;
				}
				break;
			case BITMAP_SWORD_EFFECT_MONO:
				{
					o->LifeTime = 15;
					VectorCopy(Light, o->Light);
					o->Scale = Scale;
				}
				break;
			case MODEL_WOLF_HEAD_EFFECT2:
				{
					BMD* b = &Models[o->Owner->Type];
					b->TransformByObjectBone(p1, o->Owner, 27);
					VectorCopy(p1, o->Position);

					if(o->SubType == 3 || o->SubType == 5)
					{
						b->TransformByObjectBone(p1, o->Owner, 5);
						VectorCopy(p1, o->Position);
						o->Scale =1.0f;
						if(o->SubType == 5)
						{
							o->Scale =1.5f;
						}
						o->LifeTime = 4.0f / Models[o->Owner->Type].Actions[PLAYER_SKILL_GIANTSWING].PlaySpeed;
					}
					else if(o->SubType == 4)
					{
						o->Velocity = 0.1f;
						o->Scale = 0.7f+rand()%3*0.1f;
						o->Direction[1] = -40.0f;
						o->LifeTime = 5+rand()%2;
					}
					else if(o->SubType == 6)
					{
						o->Velocity = 0.1f;
						o->Scale        = 1.2f+rand()%5*0.1f;
     					o->Direction[1] = -50.f;
						o->LifeTime		= 2+rand()%2;
					}
					Vector(0.4f, 0.4f, 0.6f, o->Light);
				}
				break;
			case MODEL_SHOCKWAVE_GROUND01:
				{
					if(o->SubType==0)
					{
						o->LifeTime = 20;
						VectorCopy(Light, o->Light);
						o->Scale = Scale;
					}
					else if(o->SubType==2)
					{
						o->LifeTime = 10;
						VectorCopy(Light, o->Light);
						o->Scale = Scale;
					}
					else
					{
						o->LifeTime = 50;
						VectorCopy(Light, o->Light);
						o->Scale = Scale;
					}					
				}
				break;
			case MODEL_DRAGON_KICK_DUMMY:
				{
					VectorCopy(Position, o->Position);
					// ani keyframe 10 : 19
					o->LifeTime = 200;
					Models[MODEL_DRAGON_KICK_DUMMY].Actions[0].PlaySpeed = Models[o->Owner->Type].Actions[PLAYER_SKILL_DRAGONKICK].PlaySpeed;
					o->Velocity = Models[MODEL_DRAGON_KICK_DUMMY].Actions[0].PlaySpeed*2.0f;
					VectorCopy(Light, o->Light);
				}
				break;
			case BITMAP_LAVA:
				{
					o->LifeTime = 999;
					VectorCopy(o->Owner->Position, o->StartPosition);
					o->Scale = Scale;
					o->Velocity = Models[o->Owner->Type].Actions[PLAYER_SKILL_DRAGONLORE].PlaySpeed;
				}
				break;
			case MODEL_DRAGON_LOWER_DUMMY:
				{
					VectorCopy(Light, o->Light);
					o->Scale = Scale;
					o->Velocity = 0.3f;
					o->LifeTime = 300;
					o->Alpha = 1.0f;
					vec3_t vPos;
					VectorCopy(o->Position, vPos);
					vPos[2] = RequestTerrainHeight(vPos[0], vPos[1]);
					o->Position[2] = vPos[2]+10.0f;
					o->Angle[2] = 45.0f+ rand()%180;
					Models[o->Type].Actions[o->CurrentAction].PlaySpeed = 0.03f;
					o->m_iAnimation = 0;
				}
				break;
			case MODEL_TARGETMON_EFFECT:
				{
					o->Scale = Scale;
					VectorCopy(Light, o->Light);
					o->LifeTime = 100;
				}
				break;
			case MODEL_VOLCANO_OF_MONK:
				{
					if(o->SubType==1)
					{
						o->Scale = Scale;
						VectorCopy(Light, o->Light);
						VectorCopy(Position, o->Position);
						o->Position[2] = RequestTerrainHeight(o->Position[0], o->Position[1]);
						Models[o->Type].Actions[0].PlaySpeed = 0.1f;
						o->LifeTime = 100;						
						o->Velocity = 0.6f;
						vec3_t vLight;
						Vector(1.0f, 1.0f, 1.0f, vLight);
						for(int cnt=0; cnt<4; ++cnt)
						{
							CreateEffect(MODEL_VOLCANO_STONE, o->Position, o->Angle, vLight, o->SubType, o, -1, 0, 0, 0, 1.0f);
						}
						CreateEffect(BITMAP_LIGHT_RED, o->Position, o->Angle, vLight, 3, o,-1,0,0,0, 3.0f);
					}
					else if(o->SubType==2 || o->SubType==3)
					{
						o->LifeTime = 20+rand()%10;
						if(o->SubType==3)
						{
							o->LifeTime = 30+rand()%20;
						}
						VectorCopy(Light, o->Light);
						VectorCopy(Position, o->Position);
					}
				}
				break;
			case MODEL_VOLCANO_STONE:
				{
					o->Scale = Scale+(float)(rand()%20+5)*0.06f;
					VectorCopy(Light, o->Light);
					VectorCopy(Position, o->Position);
					Vector(0.0f, 0.0f, 0.0f, o->Direction);
					o->LifeTime = rand()%10+30;
					o->Gravity = (float)(rand()%2+2);
					o->Angle[2] = (float)(rand()%360);
					AngleMatrix(o->Angle,Matrix);
					Vector(0.f,(float)(rand()%128+64)*0.1f,0.f,p1);
					VectorRotate(p1,Matrix,o->HeadAngle);
					o->HeadAngle[2] += 25.0f;
				}
				break;
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
			}
			return;
		}
	}
#ifdef CONSOLE_DEBUG
	g_ConsoleDebug->Write(MCD_ERROR, "이펙트 생성 실패");	
#endif	// CONSOLE_DEBUG
}


void MoveParticle(OBJECT *o,int Turn)
{
	if(Turn)
	{
		float Matrix[3][4];
		vec3_t Angle;
		vec3_t Position;
		VectorCopy(o->Angle,Angle);
		AngleMatrix(Angle,Matrix);
		VectorRotate(o->Direction,Matrix,Position);
		VectorAdd(o->Position,Position,o->Position);
	}
	else
	{
		VectorAdd(o->Position,o->Direction,o->Position);
	}
}

void MoveParticle(OBJECT *o,vec3_t angle )
{
	float Matrix[3][4];
	vec3_t Angle;
	vec3_t Position;
	VectorCopy(angle,Angle);
	AngleMatrix(Angle,Matrix);
	VectorRotate(o->Direction,Matrix,Position);
	VectorAdd(o->Position,Position,o->Position);
}

bool MoveJump(OBJECT *o)
{
	o->Position[2] += o->Gravity;
	o->Gravity -= 1.f;
	float Height = RequestTerrainHeight(o->Position[0],o->Position[1]);
	if(o->Position[2] < Height)
	{
		o->Position[2] = Height;
		o->Gravity = -o->Gravity*0.4f;
		o->Direction[1] = o->Direction[1]*0.4f;
		return true;
	}
	return false;
}

void CreateBomb(vec3_t p,bool Exp, int SubType)
{
	vec3_t Position,Light;
	VectorCopy(p,Position);
	Position[2] += 80.f;
	vec3_t Angle;

	Vector(1.f,1.f,1.f,Light);

#ifdef ADD_RAKLION_MOB_ICEGIANT
	if (SubType != 5)
	{
#endif	// ADD_RAKLION_MOB_ICEGIANT
	for(int j=0;j<20;j++)
	{
		Vector((float)(rand()%60+60+90),0.f,(float)(rand()%30),Angle);
		if(SubType == 2)
		{
			Vector(0.7f,0.7f,1.f,Light);
			CreateParticle(BITMAP_SPARK,Position,Angle,Light,8);
		}
		else if(SubType == 3)
		{
			Vector(1.0f,0.7f,0.4f,Light);
			CreateParticle(BITMAP_SPARK,Position,Angle,Light,8);
		}
		else if(SubType == 4)
		{
			if(j == 5) break;
			Vector(1.0f,0.5f,0.3f,Light);	// 빨강
			//Vector(0.3f,0.5f,1.0f,Light);	// 파랑
			CreateParticle(BITMAP_SPARK,Position,Angle,Light,8);
		}
#ifdef PJH_FIX_CAOTIC
		else if(SubType == 6)
		{
			Vector(0.3f,0.3f,0.3f,Light);
			CreateParticle(BITMAP_SPARK,Position,Angle,Light,10);
//			CreateParticle(BITMAP_SMOKE+3, Position, Angle, Light, 3, 1.0f);
		}
#endif //PJH_FIX_CAOTIC
		else
		{
			CreateParticle(BITMAP_SPARK,Position,Angle,Light,2);
		}
	}
#ifdef ADD_RAKLION_MOB_ICEGIANT
	}
#endif	// ADD_RAKLION_MOB_ICEGIANT

	Vector(0.7f,0.7f,0.7f,Light);

	if(Exp)
	{
		if(SubType == 2
#ifdef ADD_RAKLION_MOB_ICEGIANT
			|| SubType == 5
#endif	// ADD_RAKLION_MOB_ICEGIANT
#ifdef PJH_FIX_CAOTIC
			|| SubType == 6
#endif //#ifdef PJH_FIX_CAOTIC
			)
		{
			Vector(0.3f,0.6,1.f,Light);
#ifdef PJH_FIX_CAOTIC
			if(SubType == 6)
			{
				Vector(0.3f,0.3f,0.3f,Light);
//				CreateParticle(BITMAP_SMOKE+3, vPos, o->Angle, vLight, 3, 1.0f);
			}
//			else
#endif //#ifdef PJH_FIX_CAOTIC
			CreateParticle(BITMAP_EXPLOTION_MONO,Position,Angle,Light);
		}
		else if(SubType == 3)
		{
			Vector(1.0f,0.6,0.3f,Light);
			CreateParticle(BITMAP_EXPLOTION_MONO,Position,Angle,Light);
		}
		else if(SubType == 4)
		{
			Vector(1.0f,0.4,0.2f,Light);	// 빨강
			//Vector(0.2f,0.4,1.0f,Light);	// 파랑
			CreateParticle(BITMAP_EXPLOTION_MONO,Position,Angle,Light, 0, 0.5f);
			Vector(1.0f,0.6,0.2f,Light);	// 빨강
			//Vector(0.2f,0.6,1.0f,Light);	// 파랑
			CreateParticle(BITMAP_EXPLOTION_MONO,Position,Angle,Light, 0, 0.3f);
		}
		else
		{
     		CreateParticle(BITMAP_EXPLOTION,Position,Angle,Light);
		}
	}
}

void CreateBomb2(vec3_t p,bool Exp, int SubType, float Scale)
{
	vec3_t Position,Light;
	VectorCopy(p,Position);
	Position[2] += 30.f;
	vec3_t Angle;
	Vector(1.f,1.f,1.f,Light);
	for(int j=0;j<20;j++)
	{
		Vector((float)(rand()%60+60+90),0.f,(float)(rand()%30),Angle);
		CreateParticle(BITMAP_SPARK,Position,Angle,Light,2);
	}
	Vector(1.f,1.f,1.f,Light);
    Vector(0.f,0.f,0.f,Angle);
	if(Exp)
	{
     	CreateParticle(BITMAP_EXPLOTION_MONO,Position,Angle,Light, 0, 4.f+Scale);
	}
	else
	{
		CreateParticle(BITMAP_EXPLOTION+1,Position,Angle,Light, 0, 4.f);
	}
}
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
void CreateBomb3(vec3_t vPos, int iSubType, float fScale)
#else //PBG_ADD_NEWCHAR_MONK_SKILL
void CreateBomb3(vec3_t vPos, int iSubType)
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
{
	vec3_t vBombPos, vAngle, vLight;
	Vector((float)(rand()%60+60+90),0.f,(float)(rand()%30),vAngle);
	Vector(1.0f,1.0f,1.0f,vLight);
	for (int i = 0; i < 2; ++i)
	{
		if (iSubType == 2)
		{
			if (rand()%10 == 0) break;
			Vector(vPos[0] + rand()%150-75, vPos[1] + rand()%150-75, vPos[2] + rand()%130+30, vBombPos);
		}
		else if (iSubType == 1)
		{
			if (i == 1) break;
			else if (rand()%5 == 0) break;
			Vector(vPos[0] + rand()%80-40, vPos[1] + rand()%80-40, vPos[2] + rand()%120+30, vBombPos);
		}
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
		else if(iSubType == 3)
		{
			Vector(vPos[0] + rand()%90-40, vPos[1] + rand()%90-40, vPos[2] + rand()%100+70, vBombPos);
		}
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
		else
		{
			if (i == 1) break;
			else if (rand()%3 != 0) break;
			Vector(vPos[0] + rand()%30-15, vPos[1] + rand()%30-15, vPos[2] + rand()%80+30, vBombPos);
		}
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
		if(fScale!=1.0f)
			CreateParticle(BITMAP_SUMMON_SAHAMUTT_EXPLOSION,vBombPos,vAngle,vLight,0,fScale);
		else
#endif ///PBG_ADD_NEWCHAR_MONK_SKILL
 		CreateParticle(BITMAP_SUMMON_SAHAMUTT_EXPLOSION,vBombPos,vAngle,vLight,0,0.15f*(rand()%10));
	}

	Vector(1.0f,0.5f,0.2f,vLight);
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
	if(fScale!=1.0f)
		CreateParticle(BITMAP_MAGIC+1,vBombPos,vAngle,vLight,0,2.2f*fScale);
	else 
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
 	CreateParticle(BITMAP_MAGIC+1,vBombPos,vAngle,vLight,0,1.0f);
	Vector(1.0f,0.6f,0.2f,vLight);
#ifdef _VS2008PORTING
	for (int i = 0; i < 3; ++i)
#else // _VS2008PORTING
	for (i = 0; i < 3; ++i)
#endif // _VS2008PORTING
	{
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
		if(fScale!=1.0f)
			CreateEffect(MODEL_STONE1+rand()%2,vBombPos,vAngle,vLight,14,NULL,-1,0,0,0,2.2f*fScale);
		else 
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
 		CreateEffect(MODEL_STONE1+rand()%2,vBombPos,vAngle,vLight,13);
	}
	Vector(1.0f,1.0f,1.0f,vLight);

#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
	int nParCnt = 5;
	if(iSubType==3)
	{
		nParCnt = 20;
	}
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
#ifdef _VS2008PORTING
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
	for (int i = 0; i < nParCnt; ++i)
#else //PBG_ADD_NEWCHAR_MONK_SKILL
	for (int i = 0; i < 5; ++i)
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
#else // _VS2008PORTING
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
	for (i = 0; i < nParCnt; ++i)
#else //PBG_ADD_NEWCHAR_MONK_SKILL
	for (i = 0; i < 5; ++i)
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
#endif // _VS2008PORTING
	{
		CreateParticle(BITMAP_SPARK,vBombPos,vAngle,vLight,2);
	}
// 	for (i = 0; i < 2; ++i)
// 	{
// 		Vector(1.0f,1.0f,1.0f,vLight);	// 연기 펑
// 		CreateParticle(BITMAP_SMOKE,vBombPos,vAngle,vLight,7);	// 연기
// 	}
}

void CreateInferno(vec3_t Position, int SubType)
{
	vec3_t p,p2,Angle,Light;
	float Matrix[3][4];
	Vector(1.f,1.f,1.f,Light);
	for(int j=0;j<8;j++)
	{
		if(SubType == 2 || SubType == 3
#ifdef ADD_RAKLION_MOB_ICEGIANT
			|| SubType == 5
#endif	// ADD_RAKLION_MOB_ICEGIANT	
			)
		{
			Vector(0.f,-240.f,0.f,p);
		}
		else
			Vector(0.f,-220.f,0.f,p);
		Vector(0.f,0.f,j*45.f,Angle);
		AngleMatrix(Angle,Matrix);
		VectorRotate(p,Matrix,p2);
		VectorAdd(Position,p2,p2);
		CreateBomb(p2,true, SubType);
		if(SubType == 1)
		{
			CreateEffect(MODEL_STONE1+rand()%2,p2,Angle,Light,0);
			CreateEffect(MODEL_STONE1+rand()%2,p2,Angle,Light,0);
		}
		else if(SubType == 2 || SubType == 3)
		{
			CreateEffect(MODEL_STONE1+rand()%2,p2,Angle,Light,10);
			CreateEffect(MODEL_STONE1+rand()%2,p2,Angle,Light,10);
			CreateEffect(MODEL_STONE1+rand()%2,p2,Angle,Light,10);
		}
#ifdef ADD_RAKLION_MOB_ICEGIANT
		else if(SubType == 5)
		{
			Vector(0.3f, 0.6f, 1.0f, Light);
			for (int i = 0; i < 3; ++i)
				CreateEffect(MODEL_EFFECT_BROKEN_ICE0+rand()%3,p2,Angle,Light,0);
		}
#endif	// ADD_RAKLION_MOB_ICEGIANT
		else
		{
			CreateEffect(MODEL_STONE1+rand()%2,p2,Angle,Light,1);
			CreateEffect(MODEL_STONE1+rand()%2,p2,Angle,Light,0);
		}
	}
}


//  화살이 맞는지 검사를 한다.
void CheckClientArrow(OBJECT *o)
{

    if ( battleCastle::IsBattleCastleStart() )
    {
        DWORD att = TERRAIN_ATTRIBUTE ( o->Position[0], o->Position[1] );
        if ( (att&TW_NOATTACKZONE)==TW_NOATTACKZONE )
        {
            o->Velocity = 0.f;
            Vector ( 0.f, 0.f, 0.f, o->Direction );
            o->LifeTime /= 2.f;
            return;
        }
    }
	
	if(o->Type==MODEL_DARK_SCREAM)
	{
       float range = 100.f;
	   short TKey = o->PKKey;
#ifdef PJH_FIX_4_BUGFIX_7
        if
#endif //#ifdef PJH_FIX_4_BUGFIX_7

			(AttackCharacterRange(Hero->CurrentSkill,o->Position,range,Hero->Object.Weapon,TKey,o->AttackPoint[0]))//클라이언트마법처리
#ifdef PJH_FIX_4_BUGFIX_7
					if(CharacterAttribute->Skill[o->Skill] == AT_SKILL_DARK_SCREAM)
						{
							o->AttackPoint[0] = 2;
						}
#else
						;
#endif //#ifdef PJH_FIX_4_BUGFIX_7
	}
	
	if ( !o->Kind || o->SubType==99 )
	{
        if ( CheckCharacterRange( o, 100.f, o->PKKey ) )
		{
            int Skill = CharacterAttribute->Skill[o->Skill];

            switch ( Skill )
            {

            case AT_SKILL_PIERCING:
                if ( o->SubType==2 )
				{
                    CreateJoint(BITMAP_FLARE,o->Position,o->Position,o->Angle,6,o,30.0f);
				}
                else
                {
                    o->Live = false;
					CreateBomb(o->Position,true);
                }
                break;
            case AT_SKILL_PARALYZE:
                o->Live = false;
				if ( o->Type==MODEL_ARROW_BOMB || o->Type==MODEL_ARROW_HOLY )   //  폭탄
    				CreateBomb(o->Position,true);
                break;

            default:
#ifdef PJH_FIX_4_BUGFIX_7
				if(Skill != AT_SKILL_DARK_SCREAM)
#endif //#ifdef PJH_FIX_4_BUGFIX_7
					o->Live = false;
				if ( o->Type==MODEL_ARROW_BOMB || o->Type==MODEL_ARROW_HOLY	)   //  폭탄
				{
#ifdef CRYINGWOLF_2NDMVP
					if(o->SubType == 1)
					{
						//. 공방해제 효과처리
					}
					else
#endif // CRYINGWOLF_2NDMVP
					{
						CreateBomb(o->Position,true);
					}
				}
                break;
            }
		}
	}
	else
	{
		if(rand()%2 == 0)
			if(o->Owner==&Hero->Object)
			{
                float range = 100.f;
                int Skill = CharacterAttribute->Skill[o->Skill];

                if( Skill == AT_SKILL_PIERCING && o->Type==MODEL_ARROW_BOMB )
				{
					float dx = o->Position[0]-Hero->Object.Position[0];
					float dy = o->Position[1]-Hero->Object.Position[1];
					float Distance = sqrtf(dx*dx+dy*dy);
					if(Distance > 500) {
						return;
					}
				}

                if ( (Skill==AT_SKILL_PIERCING 
#ifdef PJH_FIX_4_BUGFIX_7
					|| Skill==AT_SKILL_DARK_SCREAM
#endif //#ifdef PJH_FIX_4_BUGFIX_7
					)
					&& o->AttackPoint[0]>0 )
                {
                    range = 200.f;
#ifdef PJH_FIX_4_BUGFIX_7
					if( Skill==AT_SKILL_DARK_SCREAM)
						range = 100.f;
#endif //#ifdef PJH_FIX_4_BUGFIX_7
                    o->AttackPoint[0]--;
                }
                else if(AttackCharacterRange(o->Skill,o->Position,range,o->Weapon,o->PKKey))//클라이언트마법처리
				{
                    switch(Skill)
                    {
#ifdef PJH_FIX_4_BUGFIX_7
					case AT_SKILL_DARK_SCREAM:
						{
							o->AttackPoint[0] = 2;
						}
						break;
#endif //#ifdef PJH_FIX_4_BUGFIX_7
                    case AT_SKILL_PIERCING:
                        if ( o->SubType==2 )
                        {
                            if ( o->Type==MODEL_ARROW_HOLY && o->LifeTime>14 )
                            {
                                //  LifeTime 2가 지난후에 또 관통 데미지 검사.
                                o->AttackPoint[0] = 5;
                            }
                            else if ( o->Type==MODEL_ARROW_BOMB )
                            {
                                o->AttackPoint[0] = 5;
                            }
                            else
                            {
                                //  LifeTime 2가 지난후에 또 관통 데미지 검사.
                                o->AttackPoint[0] = 2;
                            }
                            CreateJoint(BITMAP_FLARE,o->Position,o->Position,o->Angle,6,o,30.0f);
                        }
                        else
                        {
                            o->Live = false;
							CreateBomb(o->Position,true);
                        }
                        break;
                    case AT_SKILL_PARALYZE:
                        o->Live = false;
        				if(o->Type==MODEL_ARROW_BOMB || o->Type==MODEL_ARROW_HOLY)//폭탄
		    				CreateBomb(o->Position,true);
                        break;

                    default:
						o->Live = false;
						PlayBuffer(SOUND_ATTACK01+5+rand()%4,o);
						if(o->Type==MODEL_ARROW_BOMB || o->Type==MODEL_ARROW_HOLY)//폭탄
							CreateBomb(o->Position,true);
                        break;
                    }
				}
            }
	}
#ifdef USE_SELFCHECKCODE
	END_OF_FUNCTION( Pos_SelfCheck01);
Pos_SelfCheck01:
	;
#endif
}

void MoveCharacter(CHARACTER *c,OBJECT *o);

void MoveEffect( OBJECT *o, int iIndex)
{
	vec3_t Light;
	vec3_t Angle;
#ifdef _VS2008PORTING
	int Index;
#else // _VS2008PORTING
	int j,Index;
#endif // _VS2008PORTING
	vec3_t p,Position;
	float Matrix[3][4];
	float Height;
	float Luminosity = (float)(rand()%4+7)*0.1f;
	if(o->LifeTime < 5)
	{
		Luminosity -= (5-o->LifeTime)*0.2f;
		if(Luminosity < 0.f) Luminosity = 0.f;
	}
	Vector(1.f,1.f,1.f,Light);

	switch(o->Type)
	{
	case MODEL_DRAGON:
		{
			vec3_t vTemp;
			VectorSubtract(o->Position, o->StartPosition, vTemp);
			float fTemp = VectorLength(vTemp);

			if(o->Angle[2] >= 360.0f) o->Angle[2] = 0.0f;
			if(o->Angle[2] <= -360.0f) o->Angle[2] = 0.0f;
			if(o->Angle[1] >= 360.0f) o->Angle[1] = 0.0f;
			if(o->Angle[1] <= -360.0f) o->Angle[1] = 0.0f;

			switch(o->Kind)
			{
			case 0:
				o->Timer = 30.0f;
				break;
			case 1:
				o->Timer = -30.0f;
				break;
			default:
				o->Timer = 0.0f;
				break;
			}
			if(o->Timer > o->Angle[1]) o->Angle[1] += 0.5f;
			if(o->Timer < o->Angle[1]) o->Angle[1] -= 0.5f;

			if(fTemp <= 800.0f && fTemp >= -800.0f)
			{
				o->Kind = 2;
				o->Distance = 0.0f;
				o->Angle[2] += 0.5f * o->Distance/2.0f;
				o->CollisionRange = true;
			}
			else
			{
				if(o->CollisionRange)
				{
					o->Timer = o->Angle[2];
					o->Gravity = o->Gravity * -1.0f;
					o->CollisionRange = false;
				}
				if(o->Timer >= o->Angle[2] - 60.0f)
				{
					o->Angle[2] += 0.5f * o->Gravity * 3.0f;
					o->Kind = 1;
				}
				else if(o->Timer <= o->Angle[2] + 60.0f)
				{
					o->Angle[2] += 0.5f * o->Gravity * 3.0f;
					o->Kind = 0;
				}
				else
				{
					o->CollisionRange = true;
					o->Kind = 2;
				}
			}

			if(o->LifeTime <= 10)
				o->LifeTime = 1000;
			if(o->AnimationFrame >= 40.0f)
			{
				o->AnimationFrame = 0.0f;
				o->PriorAnimationFrame = 0.0f;
				o->CurrentAction = 0;
			}
		}
		break;

	case MODEL_ARROW_AUTOLOAD:
		{
			if(o->SubType == 0)
			{
				OBJECT* pOwn = o->Owner;
				if(o->LifeTime%10 == 0)
				{
					CreateEffect(MODEL_ARROW_AUTOLOAD, pOwn->Position, pOwn->Angle, pOwn->Light, 1, pOwn);
				}
			}
			else if(o->SubType == 1)
			{
				vec3_t tmp = { -10.f, 5.f, 10.f };

				//점점 사라져간다.
				if(o->LifeTime < 20)
				{
					float flumi = (float)(o->LifeTime-20) / 20.f;
					VectorScale(o->Direction, flumi, o->Light);
				}

				//위치와 각도를 캐릭터 이동에 따라 보정한다.
				OBJECT* pOwner = o->Owner;
				VectorTransform(tmp, pOwner->BoneTransform[47], o->Position);
				VectorScale(o->Position, pOwner->Scale, o->Position);
				VectorAdd(o->Position, pOwner->Position, o->Position);
				VectorCopy(pOwner->Angle, o->Angle);
				o->Angle[0] += 85.f; o->Angle[1] += -17.f; o->Angle[2] += 20.f;
			}
		}
		break;

	case MODEL_INFINITY_ARROW:
		{
			vec3_t tmp = { 0.f, 0.f, 0.f };
			OBJECT* pOwner = o->Owner;
			if(o->SubType == 1)
			{
				o->Light[0] *= 0.95f;
				o->Light[1] *= 0.95f;
				o->Light[2] *= 0.95f;
			}
			VectorTransform(tmp, pOwner->BoneTransform[29], o->Position);
			VectorScale(o->Position, pOwner->Scale, o->Position);
			VectorAdd(o->Position, pOwner->Position, o->Position);
			VectorCopy(pOwner->Angle, o->Angle);
		}
		break;

	case MODEL_INFINITY_ARROW1:
	case MODEL_INFINITY_ARROW2:
	case MODEL_INFINITY_ARROW3:
		{
			if(o->SubType == 0)
			{
				o->Light[0] *= 0.95f;
				o->Light[1] *= 0.95f;
				o->Light[2] *= 0.95f;
				VectorCopy(o->Owner->Position, o->Position);
			}
			else if(o->SubType == 1)
			{
				o->Light[0] *= 0.98f;
				o->Light[1] *= 0.98f;
				o->Light[2] *= 0.98f;
				if(o->LifeTime == 40)
					CreateEffect(MODEL_INFINITY_ARROW3, o->Position, o->Angle, o->Light, 2, o);		
				else
				if(o->LifeTime == 20)
					CreateEffect(MODEL_INFINITY_ARROW3, o->Position, o->Angle, o->Light, 3, o);		
				VectorCopy(o->Owner->Position, o->Position);
			}
			else if(o->SubType == 2)
			{
				o->Light[0] *= 0.98f;
				o->Light[1] *= 0.98f;
				o->Light[2] *= 0.98f;
				VectorCopy(o->Owner->Position, o->Position);
			}
			else if(o->SubType == 3)
			{
				o->Light[0] *= 0.98f;
				o->Light[1] *= 0.98f;
				o->Light[2] *= 0.98f;
				VectorCopy(o->Owner->Position, o->Position);
			}
		}
		break;
	case MODEL_INFINITY_ARROW4:
		{
			vec3_t tmp = { 0.f, 0.f, 0.f };
			OBJECT* pOwner = o->Owner;
			o->Scale *= 1.2f;
			o->Light[0] *= 0.9f;
			o->Light[1] *= 0.9f;
			o->Light[2] *= 0.9f;
			
			VectorTransform(tmp, pOwner->BoneTransform[29], o->Position);
			VectorScale(o->Position, pOwner->Scale, o->Position);
			VectorAdd(o->Position, pOwner->Position, o->Position);
			VectorCopy(pOwner->Angle, o->Angle);
		}
		break;

	case MODEL_SHIELD_CRASH:
		{
			float ftmp = 0.f;
			vec3_t nPos, nLight;

			if(o->LifeTime >= 0 && o->LifeTime < 8)
			{
				ftmp = (float)o->LifeTime/8;
				VectorScale(o->Direction, ftmp, o->Light);
			}

			if(o->LifeTime >= 8 && o->LifeTime < 24)
			{			
				ftmp = 1.f - ((float)(o->LifeTime - 24) / 16);
				VectorScale(o->Direction, ftmp, o->Light);
			}

			if(o->LifeTime == 23)
			{
				if( o->SubType == 1 )
				{
					Vector(0.3f, 0.3f, 0.8f, nLight);
					CreateEffect ( BITMAP_SHOCK_WAVE, o->Owner->Position, o->Owner->Angle, 
						nLight, 9, o->Owner, -1, 0,0,0 );
				}
				else if( o->SubType == 2 )
				{
					vec3_t vShockColor = { 0.8f, 0.3f, 0.3f };
					CreateEffect ( BITMAP_SHOCK_WAVE, o->Owner->Position, o->Owner->Angle, 
						vShockColor, 9, o->Owner, -1, 0,0,0 );
				}
				else
				{
					vec3_t vShockColor = { 1.f, 1.f, 1.f };
					CreateEffect ( BITMAP_SHOCK_WAVE, o->Owner->Position, o->Owner->Angle, 
						vShockColor, 9, o->Owner, -1, 0,0,0 );
				}
			}
			
			VectorCopy(o->Owner->Position, o->Position);
			VectorCopy(o->Owner->Position, nPos);

			if( o->SubType == 1 )
			{
				Vector(0.f, 1.f, 5.f, nLight);
				CreateEffect( MODEL_SKILL_INFERNO, nPos, o->Angle, nLight, 10, o, 30, 0);
				nPos[2] += 120.f;
				CreateParticle ( BITMAP_SMOKE, nPos, o->Angle, o->Light, 11, 2.f );
			}
			else if( o->SubType == 2 )
			{
				Vector(0.8f, 0.3f, 0.3f, nLight);
				CreateEffect( MODEL_SKILL_INFERNO, nPos, o->Angle, nLight, 10, o, 30, 0);
			}
			else if( o->SubType == 0 )
			{
				Vector(0.0f, 0.8f, 1.5f, nLight);
				CreateEffect( MODEL_SKILL_INFERNO, nPos, o->Angle, nLight, 2, o, 30, 0);
				nPos[2] += 120.f;
				CreateParticle ( BITMAP_SMOKE, nPos, o->Angle, o->Light, 11, 2.f );
			}
		}
		break;

	case MODEL_SHIELD_CRASH2:
		{
			float ftmp = 0.f;
			if(o->LifeTime >= 0 && o->LifeTime < 8)
			{
				ftmp = (float)o->LifeTime/8;
				VectorScale(o->Direction, ftmp, o->Light);
			}

			if(o->LifeTime >= 8 && o->LifeTime < 24)
			{			
				ftmp = 1.f - ((float)(o->LifeTime - 24) / 16);
				VectorScale(o->Direction, ftmp, o->Light);
			}

			
			VectorCopy(o->Owner->Position, o->Position);
		}
		break;

	case MODEL_IRON_RIDER_ARROW:
		{
			o->Scale *= 1.05f;
			if(o->LifeTime < 5)
			{
				o->Light[0] *= 0.7f;
				o->Light[1] *= 0.7f;
				o->Light[2] *= 0.7f;
			}
			vec3_t vPos;
			VectorScale(o->Direction, o->Velocity, vPos);
			VectorAdd(o->Position, vPos, o->Position);
			CreateSprite(BITMAP_LIGHT+3, o->Position, 4.0f, o->Light, o);
			CreateSprite(BITMAP_DS_EFFECT, o->Position, 2.5f, o->Light, o);
			CreateParticle(BITMAP_SPARK+1, o->Position, o->Angle, o->Light, 10, 3.0f);
			
			VectorCopy ( o->Position, o->EyeLeft );
            CreateEffect ( MODEL_WAVES, o->Position, o->Angle, o->Light, 3, NULL, 0 );
		}
		break;
#ifdef PJH_SEASON4_SPRITE_NEW_SKILL_MULTI_SHOT
		case MODEL_MULTI_SHOT3:
			o->Scale += 0.25f;
			o->BlendMeshLight = ( float)o->LifeTime/18.f;
			o->Alpha = o->BlendMeshLight;
			break;
		case MODEL_MULTI_SHOT1:
			o->Scale += 0.2f;
			o->BlendMeshLight = ( float)o->LifeTime/18.f;
			o->Alpha = o->BlendMeshLight;
			break;
		case MODEL_MULTI_SHOT2:
			o->Scale += 0.3f;
			o->BlendMeshLight = ( float)o->LifeTime/18.f;
			o->Alpha = o->BlendMeshLight;
			break;
#endif //PJH_SEASON4_SPRITE_NEW_SKILL_MULTI_SHOT
	case MODEL_BLADE_SKILL:
		{
#ifdef PJH_SEASON4_SPRITE_NEW_SKILL_MULTI_SHOT
			if(o->SubType == 0)
#endif //PJH_SEASON4_SPRITE_NEW_SKILL_MULTI_SHOT
			{
			o->Scale -= 0.1f;
			if(o->Scale < 0.8f)
				o->Scale = 0.8f;
			}
#ifdef PJH_SEASON4_SPRITE_NEW_SKILL_MULTI_SHOT
			else
			if(o->SubType == 1)
			{
				o->BlendMesh = -2;
				o->BlendMeshLight = ( float)o->LifeTime/14.f;
				o->Alpha = o->BlendMeshLight;

#ifdef PJH_SEASON4_FIX_MULTI_SHOT
				vec3_t p,Angle,Light;


				OBJECT *to = NULL;

				if(o->LifeTime >= 3 && o->LifeTime%2 == 0)
				{
					VectorCopy(o->Owner->Angle,Angle);
					int Ran = -1;
					CHARACTER *c = &CharactersClient[o->PKKey];

					Ran = rand()%6;
					if(o->m_sTargetIndex > -1)
					{
						while(true)
						{
							Ran = rand()%6;
							if(Ran != o->m_sTargetIndex)
							{
								break;
							}
						}
					}
					o->m_sTargetIndex = Ran;

					switch(Ran)
					{
					case 0:
						o->Owner->Angle[2] = Angle[2] + 7.f;	
						break;
					case 1:
						o->Owner->Angle[2] = Angle[2] + 21.f;	
						break;
					case 2:
						o->Owner->Angle[2] = Angle[2] + 35.f;	
						break;
					case 3:
						o->Owner->Angle[2] = Angle[2] - 7.f;	
						break;
					case 4:
						o->Owner->Angle[2] = Angle[2] - 21.f;	
						break;
					case 5:
						o->Owner->Angle[2] = Angle[2] - 35.f;	
						break;
					}

					if(Ran < 0)
						Ran = Ran;
					for(int i = 0; i < 3; i++)
					{
						VectorCopy(o->Position,p);
						Vector(0.2f,0.3f,1.f,Light);

						p[1] += rand()%200 - 100;
						p[2] += rand()%200 - 100;
						CreateSprite(BITMAP_SHINY+1,p,(float)(rand()%8+8)*0.2f,Light,o->Owner,(float)(rand()%360));
						Vector(1.f,1.f,1.f,Light);
						CreateSprite(BITMAP_SHINY+1,p,(float)(rand()%8+8)*0.07f,Light,o->Owner,(float)(rand()%360));
						VectorCopy(o->Owner->Position,p);
						p[2] += ((rand()%80 - 40) + 200); 
						CreateJoint(BITMAP_SPARK+1,p,p,o->Owner->Angle,1,o->Owner,18.7f);
					}
					CreateArrow(c,o->Owner,to,FindHotKey(( o->Skill)),1,0);
					o->Owner->Angle[2] = Angle[2];
				}
#endif //#ifdef PJH_SEASON4_FIX_MULTI_SHOT
			}
#endif //#ifdef PJH_SEASON4_SPRITE_NEW_SKILL_MULTI_SHOT
		}
		break;
	case MODEL_KENTAUROS_ARROW:
		{
			if(o->LifeTime == 24)
			{
					vec3_t vDir;
					vec34_t vMat;
					Vector(0.f, -1.f, 0.f, vDir);
					AngleMatrix(o->Angle, vMat);
					VectorRotate(vDir, vMat, o->Direction);
					CreateJoint ( BITMAP_FLARE+1, o->Position, o->Position, o->Angle, 17, o, 15.f, 40 );
			}
			if(o->LifeTime <= 24)
			{
				o->Scale *= 1.05f;
				vec3_t vLight = { 0.3f, 0.5f, 1.f };
				vec3_t vPos;
				VectorScale(o->Direction, o->Velocity, vPos);
				VectorAdd(o->Position, vPos, o->Position);
	//			CreateParticle(BITMAP_FLAME, o->Position, o->Angle, vLight, 8, 3.5f);
				CreateParticle(BITMAP_SMOKE, o->Position, o->Angle, vLight, 26, 0.2f);
				CreateParticle(BITMAP_SMOKE, o->Position, o->Angle, vLight, 26, 0.2f);
	//			CreateJoint ( BITMAP_FLARE+1, o->Position, o->Position, o->Angle, 9, o, 10.f, 40 );
			}
//			else
				o->Alpha += 0.001f;
		}
		break;
	case MODEL_WARP3:
	case MODEL_WARP2:
	case MODEL_WARP:
#ifdef CSK_ADD_MAP_ICECITY
	case MODEL_WARP6:
	case MODEL_WARP5:
	case MODEL_WARP4:
#endif // CSK_ADD_MAP_ICECITY
		{
			if(o->SubType == 0)
			{
				float fTemp1 = sinf(WorldTime*0.0011f)*0.2f;
				float fTemp2 = sinf(WorldTime*0.0017f)*0.2f;
				float fTemp3 = sinf(WorldTime*0.0013f)*0.2f;
				Vector(fTemp1+0.01f, fTemp2+0.01f, fTemp3+0.01f, o->Light);
				o->Angle[1] += 4.0f + o->Gravity;
			}
#ifdef CSK_ADD_MAP_ICECITY	
			else if(o->SubType == 1)
			{
				float fTemp1 = sinf(WorldTime*0.0011f)*0.05f;
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
				float fTemp2 = sinf(WorldTime*0.0017f)*0.05f;
				float fTemp3 = sinf(WorldTime*0.0013f)*0.05f;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
				Vector(0.0f+fTemp1, 0.2f+fTemp1, 0.1f+fTemp1, o->Light);
				o->Angle[1] += 2.0f + o->Gravity;
			}
#endif // CSK_ADD_MAP_ICECITY
		}
		break;	
	case MODEL_GHOST:
		{
			if(rand()%40 == 1)
			{
				o->Velocity = o->Velocity * -1.0f;
				o->Gravity = o->Velocity * ((float)(rand()%3)+5.0f);
			}
			if(rand()%40 == 1)
			{
				o->Velocity = o->Velocity * -1.0f;
				o->Distance = o->Velocity * ((float)(rand()%3)+1.0f);
			}
			
			vec3_t vTemp;
			VectorSubtract(o->Position, o->StartPosition, vTemp);
			float fTemp = VectorLength(vTemp);
			
			if(fTemp >= 500.f && o->Kind == 0)
			{
				o->LifeTime = 50;
				o->Kind = 1;
			}

			o->Angle[2] += sinf((int)WorldTime%10000 * 0.0001f) * o->Gravity;
			o->Angle[0] += sinf((int)WorldTime%10000 * 0.0001f) * o->Distance / 3.0f;
			
			Vector ( o->BlendMeshLight/10.f, o->BlendMeshLight/10.f, o->BlendMeshLight/4.f, o->Light );
			CreateParticle(BITMAP_LIGHT,o->Position, o->Angle, o->Light, 9, o->Scale );
		}
		break;			

	case MODEL_TREE_ATTACK:
		{
			Vector(1.0f,1.0f,1.0f,Light);
			o->Scale += 0.1f;
			if(o->Scale >= 1.0f)
			{
				o->Alpha -= 0.07f;
				o->Scale = 1.0f;
			}
		}
		break;
	case MODEL_BUTTERFLY01:		//. 반디불 박스 : 2004/11/05
		{
			//. 이동
			if(o->Kind > 0)
				o->Angle[2] += rand()%10;
			else
				o->Angle[2] -= rand()%10;
			
			if(rand()%32 == 0)
			{
				o->Direction[2] = (float)(rand()%15-7)*1.f;
			}
			o->Direction[2] += (float)(rand()%15-7)*0.2f;
			float Height = RequestTerrainHeight(o->Position[0],o->Position[1]);
			if(o->Position[2] < Height+50.f)
			{
				o->Direction[2] *= 0.8f;
				o->Direction[2] += 1.f;
			}
			if(o->Position[2] > Height+150.f)
			{
				o->Direction[2] *= 0.8f;
				o->Direction[2] -= 1.f;
			}
			o->Position[2] += (float)(rand()%15-7)*0.3f;
			
			//. 스프라이트 생성
			float  Luminosity = (float)(rand()%32+64)*0.01f;
			if(o->SubType == 0) {	//. 약간 초록
				Vector ( Luminosity*0.4f, Luminosity*0.8f, Luminosity*0.6f, Light );
			}
			else if(o->SubType == 1) {	//. 약간 파랑
				Vector ( Luminosity*0.4f, Luminosity*0.6f, Luminosity*0.8f, Light );
			}
			else if(o->SubType == 2) {	//. 약간 노랑
				Vector ( Luminosity*0.6f, Luminosity*0.8f, Luminosity*0.4f, Light );
			}
			else if(o->SubType == 3)
			{
				Vector ( Luminosity*0.7f, Luminosity*0.9f, Luminosity*0.5f, Light );
				if(rand()%2 == 0)
				{
					vec3_t Position;
					Vector((float)(rand()%16-8),(float)(rand()%16-8),(float)(rand()%16-8),Position);
					VectorAdd(Position,o->Position,Position);
					CreateParticle(BITMAP_SPARK,Position,o->Angle, o->Light,7);
				}
			}
			CreateSprite(BITMAP_LIGHT,o->Position,1.f,Light,o);
		}
		break;
    case 9 :                         //  칼리마 맵에만 존재하는 돌덩이.
        VectorScale ( o->Direction, o->Velocity, Position );
        VectorAdd ( Position, o->Position, o->Position );
//        o->Position[2] -= o->Velocity;
        o->Velocity    += 3.f;
        o->Angle[0]    += 5.f*o->Velocity;//rand()%20+20.f;

        if ( o->Position[2]<350.f && o->PKKey==0 )
        {
            o->PKKey = 1;
            int PositionX = (int)(o->Position[0]/TERRAIN_SCALE);
            int PositionY = (int)(o->Position[1]/TERRAIN_SCALE);
            AddWaterWave( PositionX, PositionY, 2, -1000 );
        }
        break;

	case BITMAP_SKULL:	// 마검 - 방어 감소
        if ( o->SubType==4 )
        {
            o->Direction[1] -= o->Velocity;
            o->Angle[0] -= 1.f;
            CreateSprite ( BITMAP_SKULL, o->Position, 10.0f, o->Light, NULL );
        }
        else
        {
            if ( !o->Owner->Live)
            {
                o->LifeTime = 0;
                o->Live = false;
            }
            else
            {
                switch ( o->SubType)
                {
                case 0:
					if( g_isCharacterBuff(o->Owner, eDeBuff_Defense) )				
                    {
                        o->LifeTime = 10;
                        //  투명일때 보조계열 효과는 보이지 않는다.
						if( g_isCharacterBuff(o->Owner, eBuff_Cloaking) )
                        {
                            o->Visible = false;
                            break;
                        }
                    }
                case 1:
                case 3:
                    {
                        for ( int i = 0; i < 3; ++i)
                        {
                            float fParam = ( float)i * Q_PI * 2 / 3.0f + o->LifeTime * 0.17f;
                            if ( o->SubType == 3)
                            {
                                fParam = -fParam;
                            }
                            vec3_t Position;
                            float fDist = 50.f + 20.f * ( float)sinf( i * 15.37f + ( float)WorldTime*0.0031f);
                            Position[0] = o->Owner->Position[0] + fDist * ( float)sinf( fParam);
                            Position[1] = o->Owner->Position[1] + fDist * ( float)cosf( fParam);
                            Position[2] = o->Owner->Position[2] + ( ( o->SubType == 3) ? 250.0f : 200.0f) * o->Owner->Scale;
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
                            vec3_t LightFlame[2] = { { 0.65f, 0.65f, 0.65f}, { 0.1f, 1.0f, 1.0f}};
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
                            CreateSprite(BITMAP_SKULL,Position,1.0f,Light,o->Owner,0.0f);
                        }
                    }
                    break;
				case 5:
                    {
                        for ( int i = 0; i < 3; ++i)
                        {
                            float fParam = ( float)i * Q_PI * 2 / 3.0f + o->LifeTime * 0.17f;
                            vec3_t Position;
                            Position[0] = o->Owner->Position[0] + 50.0f * ( float)sinf( fParam + WorldTime*0.003f);
                            Position[1] = o->Owner->Position[1] + 50.0f * ( float)cosf( fParam + WorldTime*0.003f);
                            Position[2] = o->Owner->Position[2] + (i+1)* 50.0f * o->Owner->Scale;
                            vec3_t LightFlame = { 0.6f, 0.6f, 1.0f};
                            CreateParticle(BITMAP_LIGHT,Position,o->Angle, LightFlame, 5, 0.7f);
						}
                    }
                    break;
                case 2:
                    {
                        vec3_t Light;
                        Light[0] = Light[1] = Light[2] = 0.1f * ( float)o->LifeTime;
                        CreateSprite(BITMAP_SKULL,o->Position,1.5f,Light,o->Owner,0.0f);
                    }
                    break;
                    /*case 3:
                    {
                    vec3_t Position;
                    VectorCopy( o->Owner->Position, Position);
                    Position[2] += 240.0f;
                    CreateSprite(BITMAP_SKULL,Position,1.5f,o->Light,o->Owner,0.0f);
                    }
                    break;*/
                }
            }
        }
		break;
	case MODEL_MAGIC_CAPSULE2:	// 법사 - 방어막
		VectorCopy( o->Owner->Position, o->Position);
		if ( o->LifeTime < 10)
		{
			o->BlendMeshLight = ( float)o->LifeTime * 0.1f;
        }
		break;
	case MODEL_SPEAR:
		if ( 1 == o->SubType)
		{
			CreateJoint(BITMAP_FLARE,o->Position,o->Position,o->Angle,12,o,100.0f);
		}
		else if ( 0 == o->SubType)
		{
			CreateJoint(BITMAP_FLARE,o->Position,o->Position,o->Angle,4,o,50.0f);
		}
		break;
	case MODEL_SPEAR+1:
		//CreateSprite(BITMAP_MAGIC+1,o->Position,(float)(rand()%4+8)*0.1f,o->Light,o,(float)(rand()%360));
		{
			vec3_t Angle,p,Position;
			Vector(0.f, 0.f, 0.f, Position);
			float Matrix[3][4];
			Vector(0.f,-100.f,0.f,p);
			for(int j=0;j<3;j++)
			{
				Vector((float)(rand()%90),0.f,(float)(rand()%360),Angle);
				AngleMatrix(Angle,Matrix);
				VectorRotate(p,Matrix,Position);
				VectorSubtract(o->Position,Position,Position);
				//Position[2] += 30.f;
				CreateJoint(BITMAP_JOINT_HEALING,Position,o->Position,Angle,6,NULL,5.f);
			}
		}
		break;

	case MODEL_HALLOWEEN_CANDY_BLUE:
	case MODEL_HALLOWEEN_CANDY_ORANGE:
	case MODEL_HALLOWEEN_CANDY_YELLOW:
	case MODEL_HALLOWEEN_CANDY_RED:
	case MODEL_HALLOWEEN_CANDY_HOBAK:
	case MODEL_HALLOWEEN_CANDY_STAR:
		if(o->SubType == 0 || o->SubType == 1)
		{
			o->Angle[o->m_iAnimation] += 20.f;
			o->Position[2] += (o->Gravity * 0.5f);
			o->Gravity -= 1.5f;

			Height = RequestTerrainHeight(o->Position[0],o->Position[1]);
			if(o->Position[2] < Height)
			{
				o->Position[2] = Height;
				o->Gravity = -o->Gravity*0.3f;
				o->LifeTime -= 2;
			}

			VectorAdd(o->Position, o->Direction, o->Position);

			if(o->Type == MODEL_HALLOWEEN_CANDY_HOBAK)
			{
				if(rand()%3 == 0)
				{
					CreateParticle(BITMAP_FIRE, o->Position, o->Angle, o->Light, 5, 0.4f+rand()%10*0.01f);
				}
			}
		}
		break;

	case MODEL_HALLOWEEN_EX:
		break;

	case MODEL_XMAS_EVENT_BOX:
	case MODEL_XMAS_EVENT_CANDY:
	case MODEL_XMAS_EVENT_TREE:
	case MODEL_XMAS_EVENT_SOCKS:
		o->Angle[o->m_iAnimation] += 20.f;
		o->Position[2] += (o->Gravity * 0.5f);
		o->Gravity -= 1.5f;

		Height = RequestTerrainHeight(o->Position[0],o->Position[1]);
		if(o->Position[2] < Height+3.f)
		{
			o->Position[2] = Height+3.f;
			o->Gravity = -o->Gravity*0.3f;
			o->LifeTime -= 2;
		}

		VectorAdd(o->Position, o->Direction, o->Position);
		break;

	case MODEL_XMAS_EVENT_ICEHEART:
		o->Angle[2] += 10.f;
		if(o->Owner != NULL)
		{
			if(o->Owner->CurrentAction != PLAYER_SANTA_2)
			{
				o->Live = false;
			}
		}
		break;

	case MODEL_NEWYEARSDAY_EVENT_BEKSULKI:
	case MODEL_NEWYEARSDAY_EVENT_CANDY:
	case MODEL_NEWYEARSDAY_EVENT_MONEY:
	case MODEL_NEWYEARSDAY_EVENT_HOTPEPPER_GREEN:
	case MODEL_NEWYEARSDAY_EVENT_HOTPEPPER_RED:
	case MODEL_NEWYEARSDAY_EVENT_PIG:
	case MODEL_NEWYEARSDAY_EVENT_YUT:
		o->Angle[o->m_iAnimation] += 10.f;
#ifdef PRUARIN_EVENT07_3COLORHARVEST
		o->Position[0] += (o->Direction[0]*1.2f);
		o->Position[1] += (o->Direction[1]*1.2f);
#endif // PRUARIN_EVENT07_3COLORHARVEST
		o->Position[2] += (o->Gravity * 1.5f);
		o->Gravity -= 1.5f;
		
		Height = RequestTerrainHeight(o->Position[0],o->Position[1]);
		if(o->Position[2] < Height)
		{
			o->Position[2] = Height;
			o->Gravity = -o->Gravity*0.3f;
			o->LifeTime -= 2;
		}

		VectorAdd(o->Position, o->Direction, o->Position);
		break;

#ifdef PRUARIN_EVENT07_3COLORHARVEST
	case MODEL_MOONHARVEST_MOON:
		if(o->SubType == 0)
		{
			o->Angle[2] += 5.0f;
			if( o->LifeTime < 25 )
			{
				o->Light[0]-=0.05f;
				o->Light[1]-=0.06f;
				o->Light[2]-=0.05f;
			}
		}
#ifdef CSK_RAKLION_BOSS
		else if(o->SubType == 1)
		{
			VectorAdd(o->Position, o->Direction, o->Position);
			CreateParticle(BITMAP_SMOKELINE1+rand()%3, o->Position, o->Angle,o->Light, 0, 1.0f);
			//CreateParticle(BITMAP_SMOKE, o->Position, o->Angle,o->Light, 23, 1.0f);
			//CreateParticle(BITMAP_SMOKE, o->Position, o->Angle,o->Light, 8, 1.0f);
			CreateParticle(BITMAP_SMOKE, o->Position, o->Angle,o->Light, 11, 1.0f);
			if(rand()%2 == 0)
			{
				CreateParticle ( BITMAP_WATERFALL_3, o->Position, o->Angle, o->Light, 3, 3.f );
			}
		}
#endif // CSK_RAKLION_BOSS
#ifdef LDK_ADD_GAMBLERS_WEAPONS
		else if(o->SubType == 2)
		{
			o->Scale += 0.01f;
		}
#endif //LDK_ADD_GAMBLERS_WEAPONS

		break;

	case MODEL_MOONHARVEST_GAM:
	case MODEL_MOONHARVEST_SONGPUEN1:
	case MODEL_MOONHARVEST_SONGPUEN2:
		o->Angle[0] += 10.f;
		o->Position[0] += (o->Direction[0]*2.2f);
		o->Position[1] += (o->Direction[1]*2.2f);
		o->Position[2] += (o->Gravity * 1.5f);
		o->Gravity -= 1.5f;
		
		Height = RequestTerrainHeight(o->Position[0],o->Position[1]);
		if(o->Position[2] < Height)
		{
			o->Position[2] = Height;
			o->Gravity = -o->Gravity*0.3f;
			o->LifeTime -= 2;
		}
		break;
#endif // PRUARIN_EVENT07_3COLORHARVEST

	case MODEL_SPEARSKILL:
		break;
	case BITMAP_FIRE_CURSEDLICH:
		if (o->SubType == 0)
		{
			OBJECT * pObject = o->Owner;
			if (pObject->Live == false)
			{
				o->Live = false;
				break;
			}
			BMD * pModel = &Models[pObject->Type];
			vec3_t vPos, vRelative;
			Vector(0.0f, 0.0f, 0.0f, vRelative);
			pModel->TransformPosition(pObject->BoneTransform[o->Skill], vRelative, vPos, false);
			VectorScale(vPos, pModel->BodyScale, vPos);
			VectorAdd(vPos, pObject->Position, o->Position);

			vec3_t vLight;
			Vector(o->Alpha * 0.3f, o->Alpha * 0.3f, o->Alpha * 0.3f, vLight);
			CreateParticle(BITMAP_FIRE_CURSEDLICH, o->Position, o->Angle, vLight, 3, 1, pObject);
		}
		else if	(o->SubType == 1)
		{
			vec3_t vFirePosition;
			for (int i = 0; i < 1; ++i)
			{
				Vector(o->Position[0]+(rand()%100-50)*1.0f,
					o->Position[1]+(rand()%100-50)*1.0f, o->Position[2]+(rand()%10+5)*1.0f, vFirePosition);
				float fScale = (rand()%5+13)*0.1f;
 				switch(rand()%3)
				{
				case 0:
					CreateParticle(BITMAP_FIRE_HIK1,vFirePosition,o->Angle,o->Light,1,fScale);	// 불
					break;
				case 1:
					CreateParticle(BITMAP_FIRE_CURSEDLICH,vFirePosition,o->Angle,o->Light,5,fScale);	// 불
					break;
				case 2:
					CreateParticle(BITMAP_FIRE_HIK3,vFirePosition,o->Angle,o->Light,1,fScale);	// 불
					break;
				}
			}			
		}
#ifdef LDS_ADD_EG_4_MONSTER_WORLDBOSS_GAIONKALEIN
		else if	(o->SubType == 12)
		{
			vec3_t vFirePosition;
			for (int i = 0; i < 1; ++i)
			{
				Vector(o->Position[0]+(rand()%50-25)*1.0f,
					o->Position[1]+(rand()%50-25)*1.0f, o->Position[2]+(rand()%10+5)*1.0f, vFirePosition);
				float fScale = ((rand()%5+13)*0.05f) * o->Scale;
				
				switch(rand()%3)
				{
				case 0:
					CreateParticle(BITMAP_FIRE_HIK1,vFirePosition,o->Angle,o->Light,1,fScale);	// 불
					break;
				case 1:
					CreateParticle(BITMAP_FIRE_CURSEDLICH,vFirePosition,o->Angle,o->Light,5,fScale);	// 불
					break;
				case 2:
					CreateParticle(BITMAP_FIRE_HIK3,vFirePosition,o->Angle,o->Light,1,fScale);	// 불
					break;
				}
			}			
		}
#endif // LDS_ADD_EG_2_MONSTER_IMPERIALGUARDIANDEFENDER
#ifdef YDG_ADD_SKILL_FLAME_STRIKE
		else if	(o->SubType == 2)
		{
			if (o->Owner == NULL)
			{
				o->Live = false;
				break;
			}

			vec3_t vFirePosition;
			for (int i = 0; i < 8; ++i)
			{
				int iNumBones = Models[o->Owner->Type].NumBones;
				Models[o->Owner->Type].TransformByObjectBone(vFirePosition, o->Owner, rand()%iNumBones);	

				vec3_t vLightFire;
				Vector(1.0f, 0.2f, 0.0f, vLightFire);
				CreateSprite(BITMAP_LIGHT, vFirePosition, 4.0f, vLightFire, o->Owner);

				vec3_t vLight;
				Vector(1.0f, 1.0f, 1.0f, vLight);
				float fScale = (rand()%5+13)*0.1f;
 				switch(rand()%3)
				{
				case 0:
					CreateParticle(BITMAP_FIRE_HIK1,vFirePosition,o->Angle,vLight,0,fScale);	// 불
					break;
				case 1:
					CreateParticle(BITMAP_FIRE_CURSEDLICH,vFirePosition,o->Angle,vLight,4,fScale);	// 불
					break;
				case 2:
					CreateParticle(BITMAP_FIRE_HIK3,vFirePosition,o->Angle,vLight,0,fScale);	// 불
					break;
				}
			}				
		}
#endif	// YDG_ADD_SKILL_FLAME_STRIKE
#ifdef PBG_ADD_PKFIELD
		else if(o->SubType == 3)
		{
			for (int i = 0; i < 2; ++i)
			{
				float fScale = (rand()%5+18)*0.03f;
			
				switch(rand()%3)
				{
				case 0:
					CreateParticle(BITMAP_FIRE_HIK1,o->Position,o->Angle,o->Light,0,fScale);
					break;
				case 1:
					CreateParticle(BITMAP_FIRE_CURSEDLICH,o->Position,o->Angle,o->Light,4,fScale);
					break;
				case 2:
					CreateParticle(BITMAP_FIRE_HIK3,o->Position,o->Angle,o->Light,0,fScale);
					break;
				}
			}
		}
		else if(o->SubType == 4)
		{
			for (int i = 0; i < 2; ++i)
			{
				float fScale = (rand()%5+18)*0.03f;
				Vector(0.6f,0.9f,0.1f,o->Light);
				switch(rand()%3)
				{
				case 0:
					CreateParticle(BITMAP_FIRE_HIK1_MONO,o->Position,o->Angle,o->Light,0,fScale);
					break;
				case 1:
					CreateParticle(BITMAP_FIRE_HIK2_MONO,o->Position,o->Angle,o->Light,4,fScale);
					break;
				case 2:
					CreateParticle(BITMAP_FIRE_HIK3_MONO,o->Position,o->Angle,o->Light,0,fScale);
					break;
				}
			}
		}
#endif //PBG_ADD_PKFIELD
		break;
	case MODEL_SUMMONER_WRISTRING_EFFECT:
		{
			OBJECT * pObject = o->Owner;
			BMD * pModel = &Models[pObject->Type];
			//pModel->Animation(BoneTransform, o->AnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);
			vec3_t vPos, vRelative;
			Vector(0.0f, 0.0f, 0.0f, vRelative);
			pModel->TransformPosition(pObject->BoneTransform[37], vRelative, vPos, false);
			VectorScale(vPos, pModel->BodyScale, vPos);
			VectorAdd(vPos, pObject->Position, o->Position);

			if(pObject->Live)
				o->LifeTime = 100.f; //무한

			BMD * b = &Models[o->Type];
			b->PlayAnimation( &o->AnimationFrame, &o->PriorAnimationFrame, &o->PriorAction, o->Velocity/5.f, o->Position, o->Angle );
		}
		break;
	case MODEL_SUMMONER_EQUIP_HEAD_SAHAMUTT:
		if (o->SubType == 0)
		{
			OBJECT * pObject = o->Owner;
			o->Position[0] = pObject->Position[0] + cosf(WorldTime*0.003f+o->Skill*0.024f) * 60.0f;
			o->Position[1] = pObject->Position[1] + sinf(WorldTime*0.003f+o->Skill*0.024f) * 60.0f;
			o->Position[2] = pObject->Position[2] + (sinf(WorldTime*0.0010f+o->Skill*0.024f)+2.0f) * 80.0f - 60.0f;

			if (o->StartPosition[0] != o->Position[0] - pObject->Position[0])
			{
				float fAngle = CreateAngle(o->StartPosition[0],o->StartPosition[1],
					o->Position[0] - pObject->Position[0], o->Position[1] - pObject->Position[1]);
				o->Angle[2] = fAngle + 0;
			}
			VectorSubtract(o->Position, pObject->Position, o->StartPosition);

			if (o->Kind == 1)
			{
				if (o->Alpha > 0.0f) o->Alpha -= 0.03f;
				else DeleteEffect(MODEL_SUMMONER_EQUIP_HEAD_SAHAMUTT, o->Owner);
			}
			else
			{
				if (Hero->SafeZone/* || rand()%100 == 0*/|| sinf(WorldTime*0.0004f+o->Skill*0.024f) < 0.3f)
					o->Kind = 1;
				if (o->Alpha < 1.0f) o->Alpha += 0.03f;
			}

			if(pObject->Live)
				o->LifeTime = 100.f; //무한

			vec3_t vLight;
			Vector(o->Alpha * 0.3f, o->Alpha * 0.3f, o->Alpha * 0.3f, vLight);
			CreateParticle(BITMAP_FIRE_CURSEDLICH, o->Position, o->Angle, vLight, 1, 1, pObject);
		}
		else if (o->SubType == 1)
		{
			if (rand()%2 == 0)
			{
				OBJECT * pObject = o->Owner;
				if (pObject->Live == false)
				{
					o->Live = false;
					break;
				}
				BMD * pModel = &Models[pObject->Type];
				int iNumBones = pModel->NumBones;
				CreateEffect(BITMAP_FIRE_CURSEDLICH, pObject->Position, pObject->Angle, pObject->Light, 0, pObject, -1, rand()%iNumBones);
			}
		}
		break;
	case MODEL_SUMMONER_EQUIP_HEAD_NEIL:
		if (o->SubType == 0)
		{
			OBJECT * pObject = o->Owner;
			o->Position[0] = pObject->Position[0] + cosf(WorldTime*0.003f+o->Skill*0.024f) * 60.0f;
			o->Position[1] = pObject->Position[1] + sinf(WorldTime*0.003f+o->Skill*0.024f) * 60.0f;
			o->Position[2] = pObject->Position[2] + (sinf(WorldTime*0.0010f+o->Skill*0.024f)+2.0f) * 80.0f - 60.0f;

			if (o->StartPosition[0] != o->Position[0] - pObject->Position[0])
			{
				float fAngle = CreateAngle(o->StartPosition[0],o->StartPosition[1],
					o->Position[0] - pObject->Position[0], o->Position[1] - pObject->Position[1]);
				o->Angle[2] = fAngle + 0;
			}
			VectorSubtract(o->Position, pObject->Position, o->StartPosition);

			if (o->Kind == 1)
			{
				if (o->Alpha > 0.0f) o->Alpha -= 0.03f;
				else DeleteEffect(MODEL_SUMMONER_EQUIP_HEAD_NEIL, o->Owner);
			}
			else
			{
				if (Hero->SafeZone/* || rand()%100 == 0*/|| sinf(WorldTime*0.0004f+o->Skill*0.024f) < 0.3f)
					o->Kind = 1;
				if (o->Alpha < 1.0f) o->Alpha += 0.03f;
			}

			if(pObject->Live)
				o->LifeTime = 100.f; //무한

			vec3_t vLight;
			Vector(o->Alpha, o->Alpha, o->Alpha, vLight);
			CreateParticle(BITMAP_LIGHT+2, o->Position, o->Angle, o->Light, 3, 0.30f, pObject);
			if (rand()%2 == 0)
				CreateParticle(BITMAP_LIGHT+2, o->Position, o->Angle, o->Light, 3, 0.30f, pObject);

			//DeleteJoint(MODEL_SPEARSKILL, o, 15);
		}
		else if (o->SubType == 1)
		{
			OBJECT * pObject = o->Owner;
			if (pObject->Live == false)
			{
				o->Live = false;
				break;
			}
			vec3_t vColor;
			Vector(1.0f, 1.0f, 1.0f, vColor);
			CreateParticle(BITMAP_LIGHT+2, pObject->Position, pObject->Angle, vColor, 5, 0.1f, pObject);
		}
		break;
#ifdef ASG_ADD_SUMMON_RARGLE
		case MODEL_SUMMONER_EQUIP_HEAD_LAGUL:
		{
			OBJECT* pObject = o->Owner;
			o->Position[0] = pObject->Position[0] + cosf(WorldTime*0.003f+o->Skill*0.024f) * 60.0f;
			o->Position[1] = pObject->Position[1] + sinf(WorldTime*0.003f+o->Skill*0.024f) * 60.0f;
			o->Position[2] = pObject->Position[2] + (sinf(WorldTime*0.0010f+o->Skill*0.024f)+2.0f) * 80.0f - 60.0f;
			
			if (o->StartPosition[0] != o->Position[0] - pObject->Position[0])
			{
				float fAngle = CreateAngle(o->StartPosition[0],o->StartPosition[1],
					o->Position[0] - pObject->Position[0], o->Position[1] - pObject->Position[1]);
				o->Angle[2] = fAngle + 0;
			}
			VectorSubtract(o->Position, pObject->Position, o->StartPosition);
			
			if (o->Kind == 1)
			{
				if (o->Alpha > 0.0f) o->Alpha -= 0.03f;
				else DeleteEffect(MODEL_SUMMONER_EQUIP_HEAD_LAGUL, o->Owner);
			}
			else
			{
				if (Hero->SafeZone || sinf(WorldTime*0.0004f+o->Skill*0.024f) < 0.3f)
					o->Kind = 1;
				if (o->Alpha < 1.0f) o->Alpha += 0.03f;
			}
			
			if(pObject->Live)
				o->LifeTime = 100.f; //무한
			
			vec3_t vLight;
			Vector(o->Alpha * 0.7f, o->Alpha * 0.3f, o->Alpha * 1.f, vLight);
			CreateParticle(BITMAP_CLUD64, o->Position, o->Angle, vLight, 10, 1.f, pObject);
		}
#endif	// ASG_ADD_SUMMON_RARGLE
	case MODEL_SUMMONER_CASTING_EFFECT1:
	case MODEL_SUMMONER_CASTING_EFFECT11:
	case MODEL_SUMMONER_CASTING_EFFECT111:
	case MODEL_SUMMONER_CASTING_EFFECT2:
	case MODEL_SUMMONER_CASTING_EFFECT22:
	case MODEL_SUMMONER_CASTING_EFFECT222:
	case MODEL_SUMMONER_CASTING_EFFECT4:
		{
			if (o->LifeTime < 20) o->BlendMeshLight -= 0.03f;
			else if (o->BlendMeshLight < 0.5f) o->BlendMeshLight += 0.05f;

			switch(o->Type)
			{
			case MODEL_SUMMONER_CASTING_EFFECT1:
				o->Angle[2] -= 3.0f;	// 시계방향 회전
				break;
			case MODEL_SUMMONER_CASTING_EFFECT11:
				o->Angle[2] += 3.0f;	// 반시계방향 회전
				break;
			case MODEL_SUMMONER_CASTING_EFFECT111:
				o->Angle[2] -= 3.0f;	// 시계방향 회전
				break;
			case MODEL_SUMMONER_CASTING_EFFECT2:
				o->Angle[2] += 3.0f;	// 반시계방향 회전
				break;
			case MODEL_SUMMONER_CASTING_EFFECT22:
				o->Angle[2] -= 3.0f;	// 시계방향 회전
				break;
			case MODEL_SUMMONER_CASTING_EFFECT222:
				o->Angle[2] += 3.0f;	// 반시계방향 회전
				break;
			case MODEL_SUMMONER_CASTING_EFFECT4:
				o->Scale += 0.6f;
				break;
			}
		}
		break;
		{
			if (o->LifeTime < 20) o->BlendMeshLight -= 0.03f;
			else if (o->BlendMeshLight < 0.5f) o->BlendMeshLight += 0.05f;

			o->BlendMeshTexCoordV += 0.05f;
		}
		break;
	case MODEL_SUMMONER_SUMMON_SAHAMUTT:
		{
			vec3_t vTempPosition;
			VectorCopy(o->Position, vTempPosition);

			if (o->LifeTime < 20)
			{
				SetAction(o, 0);
				o->Alpha -= 0.05f;
			}
			else if (o->CurrentAction == 0 && o->Alpha < 0.3f)
			{
				o->Alpha += 0.05f;
				{
 					o->Angle[2] = CreateAngle(o->Position[0], o->Position[1], o->HeadTargetAngle[0], o->HeadTargetAngle[1]);
					float dx = o->HeadTargetAngle[0] - o->Position[0];
					float dy = o->HeadTargetAngle[1] - o->Position[1];
					o->Distance = sqrtf(dx*dx+dy*dy);
				}
			}
			else
			{
				SetAction(o, 1);
				if (o->AnimationFrame >= 12.0f)
				{
					o->AnimationFrame = 12.0f;
				}

				if (o->AnimationFrame >= 11.0f)
				{
					if (o->Alpha > 0)
						o->Alpha -= 0.3f;
					else 
						o->Alpha = 0;
				}
				else if (o->AnimationFrame < 3.0f && o->Alpha < 0.7f)
				{
					o->Alpha += 0.05f;
				}

				if (o->AnimationFrame > 4.0f && o->AnimationFrame < 12.0f)
				{
					AngleMatrix(o->Angle,Matrix);
					vec3_t vMoveDir, Position;
					if (o->AnimationFrame < 10.0f)
					{
						Vector(0, o->Distance / -13.0f, 0, vMoveDir);
					}
					else
					{
						Vector(0, o->Distance / -45.0f, 0, vMoveDir);
					}
					VectorRotate(vMoveDir,Matrix,Position);
					VectorAdd(o->Position,Position,o->Position);
				}
				if (o->AnimationFrame <= 4.0f)
				{
 					o->Angle[2] = CreateAngle(o->Position[0], o->Position[1], o->HeadTargetAngle[0], o->HeadTargetAngle[1]);
					float dx = o->HeadTargetAngle[0] - o->Position[0];
					float dy = o->HeadTargetAngle[1] - o->Position[1];
					o->Distance = sqrtf(dx*dx+dy*dy);
				}
			}
			o->Position[2] = RequestTerrainHeight(o->Position[0],o->Position[1]);

			VectorSubtract(vTempPosition, o->Position, vTempPosition);

			// 폭발
			if (o->AnimationFrame >= 11.0f)
			{
				CreateBomb3(o->Position, o->SubType);
				if (rand()%2==0)
					PlayBuffer(SOUND_SUMMON_EXPLOSION);	// 폭발 사운드 ☆
			}

			// 화염 파티클
			BMD * pModel = &Models[o->Type];
			pModel->Animation(BoneTransform, o->AnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);
			vec3_t vPos, vRelative, vLight;
			Vector(0.0f, 0.0f, 0.0f, vRelative);
			int iPositions[] = { 13, 23, 39, 49, 3, 4, 5, 61 };
			for (int i = 0; i < 8; ++i)
			{
				pModel->TransformPosition(BoneTransform[iPositions[i]], vRelative, vPos, false);
				VectorAdd(vPos, vTempPosition, vPos);
				Vector(o->Alpha * 0.3f, o->Alpha * 0.3f, o->Alpha * 0.3f, vLight);
				CreateParticle(BITMAP_FIRE_CURSEDLICH, vPos, o->Angle, vLight, 2, 5, o);
				CreateParticle(BITMAP_FIRE_CURSEDLICH, vPos, o->Angle, vLight, 2, 4, o);
				CreateParticle(BITMAP_FIRE_CURSEDLICH, vPos, o->Angle, vLight, 2, 3, o);
			}
		}
		break;
	case MODEL_SUMMONER_SUMMON_NEIL:
		{
			if (o->LifeTime < 20) o->Alpha -= 0.05f;
			else if (o->Alpha < 0.7f) o->Alpha += 0.04f;

			if (o->AnimationFrame > 8 && o->Skill == 0)
			{
				o->Skill = 1;
				CreateEffect(MODEL_SUMMONER_SUMMON_NEIL_NIFE1, o->HeadTargetAngle, o->Angle, o->Light, o->SubType);
				if (o->SubType >= 1)
					CreateEffect(MODEL_SUMMONER_SUMMON_NEIL_NIFE2, o->HeadTargetAngle, o->Angle, o->Light, o->SubType);
				if (o->SubType >= 2)
					CreateEffect(MODEL_SUMMONER_SUMMON_NEIL_NIFE3, o->HeadTargetAngle, o->Angle, o->Light, o->SubType);
			}
			if (o->AnimationFrame > 10 && o->Skill == 1)
			{
				o->Skill = 2;

				AngleMatrix(o->Angle,Matrix);
				vec3_t vMoveDir, vPosition;
				Vector(0, -60.0f, 0, vMoveDir);
				VectorRotate(vMoveDir,Matrix,vPosition);
				VectorAdd(o->Position,vPosition,vPosition);
				CreateEffect(MODEL_SUMMONER_SUMMON_NEIL_GROUND1, vPosition, o->Angle, o->Light, o->SubType, o);

				CreateEffect(MODEL_SUMMONER_SUMMON_NEIL_GROUND1, o->HeadTargetAngle, o->Angle, o->Light, o->SubType);
				if (o->SubType >= 1)
					CreateEffect(MODEL_SUMMONER_SUMMON_NEIL_GROUND2, o->HeadTargetAngle, o->Angle, o->Light, o->SubType);
				if (o->SubType >= 2)
					CreateEffect(MODEL_SUMMONER_SUMMON_NEIL_GROUND3, o->HeadTargetAngle, o->Angle, o->Light, o->SubType);

				PlayBuffer(SOUND_SUMMON_REQUIEM);	// 가시 사운드 ☆
			}
// 			if (o->AnimationFrame > 11 && o->AnimationFrame < 12)
// 				EarthQuake = (float)(-rand()%2-5)*0.1f;
		}
		break;
	case MODEL_SUMMONER_SUMMON_NEIL_NIFE1:
	case MODEL_SUMMONER_SUMMON_NEIL_NIFE2:
	case MODEL_SUMMONER_SUMMON_NEIL_NIFE3:
		{
			if (o->LifeTime < 20) o->Alpha -= 0.05f;
			else if (o->Alpha < 1.0f) o->Alpha += 0.05f;
		}
		break;
	case MODEL_SUMMONER_SUMMON_NEIL_GROUND1:
	case MODEL_SUMMONER_SUMMON_NEIL_GROUND2:
	case MODEL_SUMMONER_SUMMON_NEIL_GROUND3:
		{
			if (o->LifeTime < 20) o->Alpha -= 0.05f;
			else if (o->Alpha < 1.0f) o->Alpha += 0.3f;
		}
		break;
#ifdef ASG_ADD_SUMMON_RARGLE
	case MODEL_SUMMONER_SUMMON_LAGUL:
		{
			if (o->SubType == 0)
			{
				int anEffectVolume[3] = { 5, 4, 3 };
				if (rand()%anEffectVolume[o->PKKey] == 0)	// 여기서 o->PKKey는 소환수 레벨.
				{
					vec3_t vPos, vLight;

					// 연기효과
					VectorCopy(o->HeadTargetAngle, vPos);
					vPos[0] += (float)(rand()%500-250);
					vPos[1] += (float)(rand()%500-250);
					Vector(1.0f, 1.0f, 1.0f, vLight);
					CreateParticle(BITMAP_SMOKE, vPos, o->Angle, vLight, 57, 3.5f);
					
					// 바닥효과
					VectorCopy(o->HeadTargetAngle, vPos);
					vPos[0] += (float)(rand()%400-200);
					vPos[1] += (float)(rand()%400-200);
					Vector(0.6f, 0.1f, 1.f, vLight);
					CreateEffect(BITMAP_CLOUD, vPos, o->Angle, vLight, 0, NULL, -1, 0, 0, 0, 2.0f);

					// 물방울 올라오는 이펙트
					Vector(0.6f, 0.1f, 1.f, vLight);
					CreateParticle(BITMAP_TWINTAIL_WATER, vPos, o->Angle, vLight, 1);
				}
			}
			else if (o->SubType == 1)
			{
				SetAction(o, 0);

				if (o->LifeTime < 30)
					o->Alpha = o->LifeTime / 40.f ;
				else if (o->LifeTime > 144)
					o->Alpha = (160 - o->LifeTime) / 20.f;
			}
		}
		break;
#endif	// ASG_ADD_SUMMON_RARGLE
    case BITMAP_FIRE:
		CreateParticle ( BITMAP_FIRE, o->Position, o->Angle, Light, 9, 1.f, o->Owner );
        break;
		
	case BITMAP_FIRE+1:
		Vector(1.f,1.f,1.f,Light);
		CreateParticle(BITMAP_FIRE+1,o->Position,o->Angle,Light,1);
		Vector(Luminosity*1.f,Luminosity*0.6f,Luminosity*0.3f,Light);
		AddTerrainLight(o->Position[0],o->Position[1],Light,2,PrimaryTerrainLight);
		break;
    case BITMAP_MAGIC:
        if ( o->SubType==0 )
        {
            CreateEffect ( BITMAP_MAGIC, o->Position, o->Angle, o->Light, 1 );
            if ( o->LifeTime>5 && o->LifeTime<10 )
            {
				CreateParticle(BITMAP_FLARE,o->Position,o->Angle,o->Light,0,0.19f,o);
            }
        }
		else if ( o->SubType==2 || o->SubType == 3 || o->SubType == 7 )
        {
            if ( o->LifeTime>5 && o->LifeTime<10 )
            {
				if(o->SubType == 3)
					CreateParticle(BITMAP_FLARE,o->Position,o->Angle,o->Light,10,0.19f,o);
				else
					CreateParticle(BITMAP_FLARE,o->Position,o->Angle,o->Light,0,0.19f,o);
            }
        }
		else if ( o->SubType==4 )
		{
			CreateParticle(BITMAP_FLARE,o->Position,o->Angle,o->Light,12,0.19f,o);
		}
		else if ( o->SubType == 8 )
		{
            o->Scale += 1.8f;
		}
		else if ( o->SubType == 9 )
		{
			if (o->LifeTime < 20) o->Alpha -= 0.05f;
			else if (o->Alpha < 1.0f) o->Alpha += 0.05f;
            //o->Scale += 1.8f;

			o->HeadAngle[0] += 4.0f;	// 반시계방향 회전
			o->HeadAngle[1] -= 8.0f;	// 시계방향 회전
			o->HeadAngle[2] += 4.0f;	// 반시계방향 회전
		}
		else if ( o->SubType == 10 )
		{
			if (o->LifeTime < 20) o->Alpha -= 0.03f;
			else if (o->Alpha < 1.0f) o->Alpha += 0.05f;
            //o->Scale += 0.1f;
		}
		else if( o->SubType == 11 )
		{
			//if (o->LifeTime < 10) o->Alpha -= 0.1f;
			//else if (o->Alpha < 1.0f) o->Alpha += 0.1f;
            //o->Scale += 1.8f;

			o->HeadAngle[0] += 2.0f;	// 반시계방향 회전
			o->HeadAngle[1] -= 2.0f;	// 시계방향 회전
			o->HeadAngle[2] += 2.0f;	// 반시계방향 회전

			if( o->LifeTime <= 10 )
			{
				o->Alpha -= 0.05f;
				o->Light[0] *= o->Alpha;
				o->Light[1] *= o->Alpha;
				o->Light[2] *= o->Alpha;
			}
		}
#ifdef YDG_ADD_SKILL_LIGHTNING_SHOCK
		else if ( o->SubType == 12 )
		{
			if (o->Alpha > 0.0f)
			{
				o->Alpha -= 0.05f;
			}
			else
			{
				o->Alpha = 0.0f;
			}
			
            o->Scale += o->Alpha * 1.0f;
			if (o->Scale > 5.0f)
			{
				//o->Scale = 5.0f;
				o->Light[0] *= 0.5f;
				o->Light[1] *= 0.5f;
				o->Light[2] *= 0.5f;
			}
		}
#endif	// YDG_ADD_SKILL_LIGHTNING_SHOCK
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
		else if(o->SubType==13)
		{
			o->Scale *= 1.1f;
			VectorScale(o->Light, 0.95f, o->Light);
			if(o->Scale >8)
			{
				o->Live = false;
			}
			if(o->Scale >4)
			{
				VectorScale(o->Light, 0.5f, o->Light);
			}
		}
		else if(o->SubType==14)
		{
			o->Scale *= 1.1f;
			VectorScale(o->Light, 0.95f, o->Light);
			if(o->Scale >8)
			{
				o->Live = false;
			}
			if(o->Scale >4)
			{
				VectorScale(o->Light, 0.5f, o->Light);
			}
		}
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
        break;
	case BITMAP_MAGIC+1:
	case BITMAP_MAGIC+2:
        if ( o->SubType >= 1 
			&& o->SubType!=4 && o->SubType!=6 && o->SubType!=7 && o->SubType!=8 && o->SubType!=9 && o->SubType!=10 
			&& o->SubType != 11 && o->SubType != 12
#ifdef CSK_RAKLION_BOSS
			&& o->SubType != 13
#endif // CSK_RAKLION_BOSS
			)
		{
			CreateHealing(o);
		}
#ifdef ENABLE_POTION_EFFECT
		if(o->SubType == 15)
		{
			if(o->LifeTime == 1) 
			{
				if((o->State&STATE_HP_RECOVERY) == STATE_HP_RECOVERY)
					o->State ^= STATE_HP_RECOVERY;
			}
		}
#endif	// ENABLE_POTION_EFFECT
		else if (o->SubType == 6)
		{
			BMD *b = &Models[o->Owner->Type];
			Vector(1,1,1,Light);
            VectorCopy ( o->StartPosition, b->BodyOrigin );
			o->BlendMeshLight = (float)(rand()%10)*0.1f;

			int iIndex;
			for(int i=0; i < o->LifeTime; i++)
			{
				iIndex = rand()%b->NumBones;
				if (b->Bones[iIndex].Dummy) 
					continue;
				Vector(-50+rand()%100,-10+rand()%20,-10+rand()%20,p);
				b->TransformPosition(o->Owner->BoneTransform[iIndex],p,Position,true);
				CreateParticle(BITMAP_LIGHT,Position,o->Owner->Angle,Light,8);
			}
			o->Owner->Alpha = o->LifeTime / 80.0f;
		}
        else if ( o->SubType == 9 )
        {
            o->Scale += 0.3f;
            o->Light[0] /= 1.1f;
            o->Light[1] /= 1.1f;
            o->Light[2] /= 1.1f;
        }
		break;
#ifdef ASG_ADD_INFLUENCE_GROUND_EFFECT
	case BITMAP_OUR_INFLUENCE_GROUND:
	case BITMAP_ENEMY_INFLUENCE_GROUND:
		if (o->SubType == 0)
		{
			if (o->Owner == NULL || o->Owner->Live == false)
				o->Live = false;

			VectorCopy(o->Owner->Position, o->Position);	// 이펙트 위치를 캐릭터 위치로 이동.

			// 점점 커지는 문양 이펙트 알파값.
			o->Alpha -= 0.02f;
			o->Scale += 0.01f;
			if (o->Alpha < 0.f)
			{
				o->Alpha = 1.0f;
				o->Scale = 0.6f;
			}

			// 내부 알파값.
			if (o->LifeTime < 25)
				o->AlphaTarget -= 0.02f;
			else if (o->AlphaTarget < 1.0f)
				o->AlphaTarget += 0.02f;

			if (1 == o->LifeTime)
				o->LifeTime = 50;
		}
		break;
#endif	// ASG_ADD_INFLUENCE_GROUND_EFFECT

	case BITMAP_MAGIC_ZIN:
		switch (o->SubType)
		{
		case 0:
			if (o->LifeTime < 20)
				o->Alpha -= 0.05f;
			else if (o->Alpha < 1.0f)
				o->Alpha += 0.05f;
			break;
		case 1:
			if (o->LifeTime < 20)
				o->Alpha -= 0.03f;
			else if (o->Alpha < 0.7f)
				o->Alpha += 0.06f;
			break;
		case 2:
			if (o->Scale < 3.5f)
				o->Scale += 0.1f;
			if (o->LifeTime < 20)
				o->Alpha -= 0.05f;
			else if (o->Alpha < 1.0f)
				o->Alpha += 0.05f;
			break;
		}
		break;

	case BITMAP_SHINY+6:
		switch (o->SubType)
		{
		case 0:
			Position[0] = o->Position[0] + (float)(rand()%500-250);
			Position[1] = o->Position[1] + (float)(rand()%500-250);
			Position[2] = o->Position[2] - (float)(rand()%100)+150.0f;
			CreateParticle(o->Type, Position, o->Angle, o->Light, 0, o->Scale);
			break;
		case 1:
		case 2:
			if (o->Owner == NULL || o->Owner->Live == false)
				o->Live = false;
			else
			{
				o->LifeTime = 100;
				BMD* pModel = &Models[o->Owner->Type];
				int iBone = rand() % pModel->NumBones;
				vec3_t vRelativePos, vWorldPos;
				Vector(0.f, 0.f, 100.f, vRelativePos);
				if (!pModel->Bones[iBone].Dummy)
				{
					pModel->TransformPosition(o->Owner->BoneTransform[iBone], vRelativePos, vWorldPos, false);
					VectorScale(vWorldPos, pModel->BodyScale, vWorldPos);
					VectorAdd(vWorldPos, o->Owner->Position, vWorldPos);
					if (rand()%2 == 0)
					{
						vWorldPos[2] -= 20.f;
						CreateParticle(o->Type, vWorldPos, o->Angle, o->Light, 0, o->Scale);
					}
				}
			}
			break;
#ifdef PJH_SEASON4_DARK_NEW_SKILL_CAOTIC
		case 3:
			{
				vec3_t Position,P,dp;
				vec3_t vFirePosition;

				float Matrix[3][4];
				int iNumBones = Models[o->Owner->Type].NumBones;
				Models[o->Owner->Type].TransformByObjectBone(vFirePosition, o->Owner, rand()%iNumBones);
				Vector(0.f,-20.f,0.f,P);
				AngleMatrix(o->Owner->Angle,Matrix);
				VectorRotate(P,Matrix,dp);
				VectorAdd(dp,vFirePosition,Position);
				CreateSprite( BITMAP_SHINY+6, Position, 2.0f, o->Light, o , NULL, 1);
				
			}
			break;
#endif //PJH_SEASON4_DARK_NEW_SKILL_CAOTIC
		}
		break;
#ifdef PJH_SEASON4_DARK_NEW_SKILL_CAOTIC
		case MODEL_DESAIR:
			{
				JOINT *oj = &Joints[o->m_sTargetIndex];
				if(oj->Live == true)
				{
					VectorCopy(oj->Position,o->Position);
					VectorCopy(oj->Angle,o->Angle);
					if(o->LifeTime%10 == 0)
					{
//						Vector(0.1f,0.1f,0.1f,Light);
						CreateEffect( MODEL_FEATHER, o->Position, o->Angle, o->Light, 2, NULL, -1, 0, 0, 0, 1.4f );
						CreateEffect( MODEL_FEATHER, o->Position, o->Angle, o->Light, 3, NULL, -1, 0, 0, 0, 1.4f );
					}
				}
			}
			break;
#endif //SKILL_DEBUG
	case BITMAP_PIN_LIGHT:
		switch (o->SubType)
		{
#ifdef PJH_SEASON4_FIX_MULTI_SHOT
		case 3:
#endif //PJH_SEASON4_FIX_MULTI_SHOT
		case 0:
			Position[0] = o->Position[0] + (float)(rand()%500-250);
			Position[1] = o->Position[1] + (float)(rand()%500-250);
			Position[2] = o->Position[2] - (float)(rand()%100)+150.0f;
			if (rand()%2 == 0)
			{
#ifdef PJH_SEASON4_FIX_MULTI_SHOT
				if(o->SubType == 3)
					CreateParticle(o->Type, Position, o->Angle, o->Light, 1, o->Scale);
				else
#endif //PJH_SEASON4_FIX_MULTI_SHOT
				CreateParticle(o->Type, Position, o->Angle, o->Light, 0, o->Scale);
			}
			break;
		case 1:
		case 2:
			if (o->Owner == NULL || o->Owner->Live == false)
				o->Live = false;
			else
			{
				o->LifeTime = 100;
				BMD* pModel = &Models[o->Owner->Type];
				int iBone = rand() % pModel->NumBones;
				vec3_t vRelativePos, vWorldPos;
				Vector(0.f, 0.f, 100.f, vRelativePos);
				if (!pModel->Bones[iBone].Dummy)
				{
					pModel->TransformPosition(o->Owner->BoneTransform[iBone], vRelativePos, vWorldPos, false);
					VectorScale(vWorldPos, pModel->BodyScale, vWorldPos);
					VectorAdd(vWorldPos, o->Owner->Position, vWorldPos);
					if (rand()%2 == 0)
					{
						vWorldPos[2] -= 20.f;
						CreateParticle(o->Type, vWorldPos, o->Angle, o->Light, 0, o->Scale);
					}
				}
			}
			break;
#ifdef LDK_ADD_CS7_UNICORN_PET
		case 4:
			if (o->Owner == NULL || o->Owner->Live == false || o->Alpha <= 0.0f)
			{
				o->Live = false;
			}
			else
			{
				o->Scale -= 0.02f;
				o->Alpha -= 0.001f;

				OBJECT *Owner = o->Owner;
				BMD* pModel = &Models[o->Owner->Type];
				vec3_t vWorldPos, vRelativePos;
				Vector(0.f, 0.f, 0.f, vRelativePos);

				if (!pModel->Bones[11].Dummy)
				{
					pModel->BodyScale = Owner->Scale;
					pModel->Animation(BoneTransform,Owner->AnimationFrame,Owner->PriorAnimationFrame,Owner->PriorAction, Owner->Angle, Owner->HeadAngle,false,false);
					pModel->TransformByObjectBone(vWorldPos, Owner, 11);
					VectorCopy(vWorldPos, o->Position);
					
					CreateSprite(BITMAP_PIN_LIGHT, vWorldPos, o->Scale, o->Light, Owner, o->Angle[1]);
				}
			}break;
#endif //LDK_ADD_CS7_UNICORN_PET
		}
		break;

#ifdef ASG_ADD_SKILL_BERSERKER
	case BITMAP_ORORA:
		if (o->Owner == NULL || o->Owner->Live == false)
			o->Live = false;
		else
		{
			// 사라지는 시점에서 이펙트 죽이고 다시 생성.
			if (o->LifeTime <= 5)
			{
				o->Live = false;
				CreateEffect(o->Type, o->Position, o->Angle, o->Light, o->SubType, o->Owner);
			}
		}
		break;

	case BITMAP_SPARK+2:
		if (o->Owner == NULL || o->Owner->Live == false)
			o->Live = false;
		else
		{
			o->LifeTime = 100;	// 이펙트 계속 유지.

			if (rand()%60 == 0)
				CreateParticle(o->Type, o->Position, o->Angle, o->Light, o->SubType, 0.5f, o->Owner);
		}
		break;

	case BITMAP_LIGHT_MARKS:
		if (o->Owner == NULL || o->Owner->Live == false)
			o->Live = false;
		else
		{
			if (o->LifeTime <= 5)
				o->LifeTime = 65;	// 이펙트 계속 유지.
		}
		break;
#endif	// ASG_ADD_SKILL_BERSERKER

//////////////////////////////////////////////////////////////////////////

    case BITMAP_GATHERING:
        if( o->SubType == 1 || o->SubType == 2 )
        {
            BMD* b = &Models[o->Owner->Type];
            Vector(0.f,0.f,10.f,p);
            VectorCopy ( o->StartPosition, b->BodyOrigin );
            b->TransformPosition(o->Owner->BoneTransform[33],p,o->Position,true);
        }
#ifdef _VS2008PORTING
        for( int j=0; j<3; ++j )
#else // _VS2008PORTING
        for( j=0; j<3; ++j )
#endif // _VS2008PORTING
        {
            Vector(0.f,120.f,0.f,p);
#ifdef CSK_ADD_MAP_ICECITY	
			if(o->SubType == 3)
			{
				Vector(0.f,25.f,0.f,p);
			}
#endif // CSK_ADD_MAP_ICECITY

		    Vector((float)(rand()%360),0.f,(float)(rand()%360),Angle);
		    AngleMatrix(Angle,Matrix);
		    VectorRotate(p,Matrix,Position);
		    VectorAdd(o->Position,Position,Position);

            if( o->SubType==1 )
            {
                //  전기.
                if((o->LifeTime%2)==0)
                {
				    CreateJoint(BITMAP_JOINT_THUNDER,Position,o->Position,Angle,3,NULL,10.f,10,10);
                }
                else
                {
                    CreateParticle(BITMAP_SPARK+1,Position,Angle,Light,2,(rand()%50+10)/100.f,o);
                }
				CreateSprite(BITMAP_SHINY+1,o->Position,(float)(rand()%8+8)*0.2f,o->Light,o,(float)(rand()%360));
            }
            else if( o->SubType==2 )
            {
                //  전기.
                if((o->LifeTime%2)==0)
                {
				    CreateJoint(BITMAP_JOINT_THUNDER,Position,o->Position,Angle,3,NULL,10.f,10,10);
                }
				CreateSprite(BITMAP_SHINY+1,o->Position,(float)(rand()%8+8)*0.2f,o->Light,o,(float)(rand()%360));
            }
#ifdef CSK_ADD_MAP_ICECITY	
			else if(o->SubType == 3)
			{
				CreateParticle(BITMAP_SPARK+1,Position,Angle,o->Light,26,(rand()%10+5)/25.f,o);
			}
#endif // CSK_ADD_MAP_ICECITY
            else
            {
				CreateSprite(BITMAP_SHINY+1,o->Position,(float)(rand()%8+8)*0.3f,o->Light,o,(float)(rand()%360));
                CreateParticle(BITMAP_SPARK+1,Position,Angle,Light,2,(rand()%50+10)/100.f,o);
            }
        }
        break;

//------------------------------------------------------------------------------------------------------------

#ifdef YDG_ADD_SKILL_GIGANTIC_STORM
	case BITMAP_JOINT_THUNDER:
		{
			float fScale = (rand()%400+100)/10.0f;
			vec3_t vPosition, vStartPosition;
			VectorCopy(o->StartPosition, vStartPosition);
			VectorCopy(o->Position, vPosition);
#ifdef YDG_FIX_GIGANTIC_STORM_OPTIMIZE
			vStartPosition[0] += rand()%100 - 50;
			vStartPosition[1] += rand()%100 - 50;
			vPosition[0] += rand()%100 - 50;
			vPosition[1] += rand()%100 - 50;
#else	// YDG_FIX_GIGANTIC_STORM_OPTIMIZE
			vStartPosition[0] += rand()%80 - 40;
			vStartPosition[1] += rand()%80 - 40;
			vPosition[0] += rand()%80 - 40;
			vPosition[1] += rand()%80 - 40;
#endif	// YDG_FIX_GIGANTIC_STORM_OPTIMIZE

#ifdef YDG_FIX_GIGANTIC_STORM_OPTIMIZE
			if (rand()%10 > 4)
			{
#ifdef LDS_ADD_EG_4_MONSTER_WORLDBOSS_GAIONKALEIN	// 번개 색 변화.
				
				if( o->SubType==1 )
				{
					CreateJoint(BITMAP_JOINT_THUNDER, vStartPosition, vPosition, o->Angle, 33, NULL, fScale);// 위치 퀸 썬더 맞은 효과
				}
				else
#endif // LDS_ADD_EG_4_MONSTER_WORLDBOSS_GAIONKALEIN
				{
					CreateJoint(BITMAP_JOINT_THUNDER, vStartPosition, vPosition, o->Angle, 16, NULL, fScale);// 위치 퀸 썬더 맞은 효과
				}
			}
#endif	// YDG_FIX_GIGANTIC_STORM_OPTIMIZE

			// 바닥
#ifdef YDG_FIX_GIGANTIC_STORM_OPTIMIZE
			vec3_t vLight = { 0.45f, 0.45f, 0.7f };
#else	// YDG_FIX_GIGANTIC_STORM_OPTIMIZE
			vec3_t vLight = { 0.15f, 0.15f, 0.2f };
#endif	// YDG_FIX_GIGANTIC_STORM_OPTIMIZE
			if (o->LifeTime > 10)
			{
#ifdef YDG_FIX_GIGANTIC_STORM_OPTIMIZE
				if (rand()%2 == 0)
#else	// YDG_FIX_GIGANTIC_STORM_OPTIMIZE
				//if (rand()%2 == 0)
#endif	// YDG_FIX_GIGANTIC_STORM_OPTIMIZE
					CreateEffect(BITMAP_MAGIC+1, vPosition, o->Angle, vLight, 11, o);
			}

			// 연기
#ifdef YDG_FIX_GIGANTIC_STORM_OPTIMIZE
			if (rand()%4 == 0)
#endif	// YDG_FIX_GIGANTIC_STORM_OPTIMIZE
				CreateParticle(BITMAP_SMOKE,vPosition,o->Angle,vLight,54,2.8f);

			// 돌
			if (rand()%4 == 0)
			{
				CreateEffect(MODEL_STONE1+rand()%2,o->Position,o->Angle,vLight,13);
			}

			// 바닥 번개
			if (o->LifeTime > 5)
			{
				Vector(0.15f, 0.15f, 0.4f, vLight);
#ifdef YDG_FIX_GIGANTIC_STORM_OPTIMIZE
				if (rand()%5 == 0)
#else	// YDG_FIX_GIGANTIC_STORM_OPTIMIZE
				if (rand()%2 == 0)
#endif	// YDG_FIX_GIGANTIC_STORM_OPTIMIZE
					CreateEffect(BITMAP_CHROME_ENERGY2, vPosition, o->Angle, vLight, 0);
			}

			if (o->Owner != NULL && rand()%5 == 0
#ifdef YDG_FIX_GIGANTIC_STORM_CRASH
				&& o->Owner->BoneTransform != NULL
#endif	// YDG_FIX_GIGANTIC_STORM_CRASH
				)
			{
				BMD *pTargetModel = &Models[o->Owner->Type];
				int iNumBones = pTargetModel->NumBones;
				float fRandom;
				vec3_t vLight, vRelativePos, vPos, vAngle;
				Vector(0.0f, 0.0f, 0.0f, vRelativePos);
				for(int i = 0; i < iNumBones; ++i)
				{
#ifdef YDG_FIX_GIGANTIC_STORM_OPTIMIZE
					if (iNumBones > 100 && rand()%iNumBones > iNumBones/10) continue;
					else if (iNumBones > 50 && rand()%iNumBones > iNumBones/5) continue;
					else if (iNumBones > 20 && rand()%iNumBones > iNumBones/2) continue;
#endif	// YDG_FIX_GIGANTIC_STORM_OPTIMIZE
					VectorCopy ( o->Owner->Position, pTargetModel->BodyOrigin );		// 갱신해주어야 한다.
					pTargetModel->TransformPosition( o->Owner->BoneTransform[i], vRelativePos, vPos, true );
		
					Vector( 0.2f, 0.2f, 0.8f, vLight );
					fRandom = 3.0f + ((float)(rand()%20-10)*0.1f);	
					CreateParticle( BITMAP_LIGHT, vPos, vAngle, vLight, 5, fRandom);
				}
			}
		}
		break;
#endif	// YDG_ADD_SKILL_GIGANTIC_STORM
#ifdef PJH_SEASON4_SPRITE_NEW_SKILL_RECOVER
		case BITMAP_IMPACT:
			{
				float ScaleBk = 0.f;
				if(o->Scale < 2.f)
				{
					o->Scale += 0.1f;
					ScaleBk = o->Scale;
				}
				else
				{
					if(o->Scale < 2.4f)
						o->Scale += 0.02f;
					else
						o->Scale = 2.f;

					if(o->Scale >= 2.2f)
						ScaleBk = 2.2f - (o->Scale - 2.2f);
					else
						ScaleBk = o->Scale;
				}
				vec3_t Light,P,dp;

				if(o->LifeTime <= 40)
				{
					o->Alpha -= 0.1f;
					o->BlendMeshLight -= 0.1f;
				}
				float Matrix[3][4];
				Vector(-10.f,-30.f,0.f,P);
				Vector(o->Light[0]*o->Alpha, o->Light[1]*o->Alpha, o->Light[2]*o->Alpha, Light);
				AngleMatrix(o->Owner->Angle,Matrix);
				VectorRotate(P,Matrix,dp);
				VectorAdd(dp,o->Owner->Position,o->Position);
				o->Position[2] += 130;
				CreateSprite(BITMAP_IMPACT,o->Position,ScaleBk,Light,o);
				CreateSprite(BITMAP_SHINY+1, o->Position, ScaleBk, Light, o, -WorldTime*0.08f);
				if(o->Scale > 1.f)
					CreateSprite(BITMAP_ORORA, o->Position, ScaleBk, Light, o, -WorldTime*0.08f);
				if(o->Scale > 2.f)
					CreateSprite(BITMAP_ORORA, o->Position, ScaleBk, Light, o, WorldTime*0.08f);
			}
		break;
#endif //PJH_SEASON4_SPRITE_NEW_SKILL_RECOVER
	case BITMAP_FLAME:
        if ( o->SubType==0 )
        {
#ifdef _VS2008PORTING
            for(int j=0;j<6;j++)
#else // _VS2008PORTING
            for(j=0;j<6;j++)
#endif // _VS2008PORTING
            {
                Vector((float)(rand()%50-25),(float)(rand()%50-25),0.f,Position);
                VectorAdd(Position,o->Position,Position);
				CreateParticle(BITMAP_FLAME,Position,o->Angle,Light);
            }
            if(rand()%8 == 0)
                CreateEffect(MODEL_STONE1+rand()%2,o->Position,o->Angle,o->Light);
            
            Vector(Luminosity*1.f,Luminosity*0.4f,Luminosity*0.f,Light);
            AddTerrainLight(o->Position[0],o->Position[1],Light,3,PrimaryTerrainLight);
            if(o->Owner == &Hero->Object)
                if(o->LifeTime%20 == 0)
                    AttackCharacterRange(o->Skill,o->Position,150.f,o->Weapon,o->PKKey);//클라이언트마법처리
        }
        else if ( o->SubType==1 || o->SubType==2 )
        {
#ifdef _VS2008PORTING
            for ( int j=0; j<18; j++ )
#else // _VS2008PORTING
            for ( j=0; j<18; j++ )
#endif // _VS2008PORTING
            {
                Vector ( 0.f, 250.f, 0.f, p );
				Vector ( 0.f, 0.f, j*20.f, Angle );
				AngleMatrix ( Angle, Matrix );
				VectorRotate ( p, Matrix, Position );
				VectorAdd ( Position, o->Position, Position );
                Position[0] += rand()%64-32;
                Position[1] += rand()%64-32;
                if ( o->SubType==1 )
                    CreateParticle ( BITMAP_FLAME, Position, o->Angle, Light, 0, 1.2f );
                else if ( o->SubType==2 )
                    CreateParticle ( BITMAP_FIRE+3, Position, o->Angle, Light, 13, 2.5f );
            }
        }
		else if ( o->SubType==3)	//. 불골렘 공격
		{
#ifdef _VS2008PORTING
			for(int j=0;j<3;j++)
#else // _VS2008PORTING
			for(j=0;j<3;j++)
#endif // _VS2008PORTING
            {
				CreateParticle(BITMAP_FLAME,o->Position,o->Angle,Light, 6);
				Vector((float)(rand()%10-5),(float)(rand()%10-5),40.f,Position);
                VectorAdd(Position,o->Position,Position);
				CreateParticle(BITMAP_TRUE_FIRE,Position,o->Angle,Light,0,2.8f);
            }
			Vector((float)(rand()%10-5),(float)(rand()%10-5),-40.f,Position);
            VectorAdd(Position,o->Position,Position);
			CreateParticle(BITMAP_SMOKE,Position,o->Angle,Light,21,0.8f);

			CreateEffect(MODEL_STONE1+rand()%2,o->Position,o->Angle,o->Light, 12);
            
            Vector(Luminosity*1.f,Luminosity*0.4f,Luminosity*0.f,Light);
            AddTerrainLight(o->Position[0],o->Position[1],Light,3,PrimaryTerrainLight);
		}
		else if(o->SubType==4)
		{
			o->Scale += 20.f;
#ifdef _VS2008PORTING
            for ( int j=0; j<18; j++ )
#else // _VS2008PORTING
            for ( j=0; j<18; j++ )
#endif // _VS2008PORTING
            {
                Vector ( 0.f, 150.f - o->Scale, 0.f, p );
				Vector ( 0.f, 0.f, j*20.f, Angle );
				AngleMatrix ( Angle, Matrix );
				VectorRotate ( p, Matrix, Position );
				VectorAdd ( Position, o->Position, Position );
                Position[0] += rand()%64-32;
                Position[1] += rand()%64-32;
                CreateParticle ( BITMAP_FLAME, Position, o->Angle, Light, 0, 1.2f );
            }
			CreateEffect(MODEL_STONE1+rand()%2,o->Position,o->Angle,o->Light, 12);
		}
		else if (o->SubType == 5)
		{
			Vector((float)(rand()%32-16),(float)(rand()%32-16),0.f,Position);
			VectorAdd(Position,o->Position,Position);
			CreateParticle(BITMAP_FLAME, Position, o->Angle, Light, 0, o->Scale);
		}
#ifdef LDS_ADD_EMPIRE_GUARDIAN
		else if( o->SubType == 6 )
		{
			Vector((float)(rand()%32-16),(float)(rand()%32-16),0.f,Position);
			VectorAdd(Position,o->Position,Position);
			CreateParticle(BITMAP_FLAME, Position, o->Angle, o->Light, 12, o->Scale);
		}
#endif //LDS_ADD_EMPIRE_GUARDIAN
		break;

//------------------------------------------------------------------------------------------------------------

#ifdef CSK_RAKLION_BOSS
	case MODEL_RAKLION_BOSS_CRACKEFFECT:
		if(o->SubType == 0)
		{
			o->Alpha -= 0.03f;
		}
		break;

//------------------------------------------------------------------------------------------------------------	

	case MODEL_RAKLION_BOSS_MAGIC:
		if(o->SubType == 0)
		{
			o->Alpha -= 0.03f;
		}
		break;

//------------------------------------------------------------------------------------------------------------

	case BITMAP_FIRE_HIK2_MONO:
		if(o->SubType == 0)
		{
			for(int i=0; i<2; ++i)
			{
				Vector((float)(rand()%30-15),(float)(rand()%30-15),0.f,p);
				VectorAdd(p, o->Position, Position);
				if(rand()%3 != 0)
				{
					CreateParticle(BITMAP_FLAME, Position, o->Angle, o->Light, 11, 1.4f);
				}

				if(rand()%8 == 0)
				{
					CreateEffect(MODEL_ICE_SMALL, Position, o->Angle, o->Light, 0);
				}
			}
		}
		else if(o->SubType == 1)
		{
#ifdef _VS2008PORTING
			for ( int j=0; j<9; j++ )
#else // _VS2008PORTING
			for ( j=0; j<9; j++ )
#endif // _VS2008PORTING
            {
                Vector ( 0.f, 250.f, 0.f, p );
				Vector ( 0.f, 0.f, j*40.f, Angle );
				AngleMatrix ( Angle, Matrix );
				VectorRotate ( p, Matrix, Position );
				VectorAdd ( Position, o->Position, Position );
                Position[0] += rand()%64-32;
                Position[1] += rand()%64-32;
                CreateParticle ( BITMAP_FIRE+3, Position, o->Angle, Light, 13, 2.5f );
            }
		}
		break;

//------------------------------------------------------------------------------------------------------------

	case BITMAP_CLOUD:
		if(o->SubType == 0)
		{
			o->Light[0] /= 1.05f;
			o->Light[1] /= 1.05f;
			o->Light[2] /= 1.05f;
			o->Scale += 0.03f;
		}
		break;

#endif // CSK_RAKLION_BOSS	

//------------------------------------------------------------------------------------------------------------
		
	case BITMAP_FIRE_RED:
		Vector((float)(rand()%32-16),(float)(rand()%32-16),0.f,Position);
		VectorAdd(Position,o->Position,Position);
		Vector ( 1.0f, 0.4f, 0.4f, Light );
		CreateParticle(BITMAP_FIRE_RED, Position, o->Angle, Light, 0, o->Scale);
		break;	

//------------------------------------------------------------------------------------------------------------

	case BITMAP_SPARK+1:
		{
		vec3_t Position;
		VectorCopy(o->Position,Position);
		float Luminosity = o->LifeTime*0.1f;
      	Vector(Luminosity,Luminosity,Luminosity,Light);
		float Scale = 6.f;
#ifdef _VS2008PORTING
		for(int j=0;j<18;j++)
#else // _VS2008PORTING
		for(j=0;j<18;j++)
#endif // _VS2008PORTING
		{
			Position[2] += Scale*4.f;
			if(j == 0)
                CreateParticle(BITMAP_SPARK+1,Position,o->Angle,Light,1,Scale*2.f);
			else
                CreateParticle(BITMAP_SPARK+1,Position,o->Angle,Light,1,Scale);
		}
		break;
		}

//------------------------------------------------------------------------------------------------------------

	case BITMAP_ENERGY:
		if(o->SubType == 0)
		{
    		Luminosity = o->LifeTime*0.2f;
			Vector(Luminosity,Luminosity,Luminosity,Light);
			CreateParticle(BITMAP_ENERGY,o->Position,o->Angle,Light);
			CreateParticle(BITMAP_SPARK+1,o->Position,o->Angle,Light,0,4.f);
			Vector(Luminosity*0.2f,Luminosity*0.4f,Luminosity*1.f,Light);
			AddTerrainLight(o->Position[0],o->Position[1],Light,2,PrimaryTerrainLight);
			CheckTargetRange(o);
		}
		break;

//------------------------------------------------------------------------------------------------------------
		
		case MODEL_LIGHTNING_ORB:
			{
				float fRot = (WorldTime*0.0006f) * 360.0f;
				vec3_t vLight;
				
				if(o->SubType == 0)
				{
					// shiny
					Vector(0.1f, 0.7f, 1.5f, vLight);
					CreateSprite(BITMAP_SHINY+1, o->Position, 4.0f, vLight, o, fRot);
					CreateSprite(BITMAP_SHINY+1, o->Position, 3.0f, vLight, o, -fRot);
					// magic_ground
					Vector(0.1f, 0.1f, 1.5f, vLight);
					CreateSprite(BITMAP_MAGIC, o->Position, 1.0f, vLight, o, fRot);
					CreateSprite(BITMAP_MAGIC, o->Position, 0.5f, vLight, o, -fRot);
					// pin_light
					Vector(0.5f, 0.5f, 1.5f, vLight);
					CreateSprite(BITMAP_PIN_LIGHT, o->Position, 2.0f, vLight, o, (float)(rand()%360));
					CreateSprite(BITMAP_PIN_LIGHT, o->Position, 2.0f, vLight, o, (float)(rand()%360));
					
					// 잔상
					Vector(0.4f, 0.4f, 1.5f, vLight);
					CreateParticle(BITMAP_MAGIC, o->Position, o->Angle, vLight, 0, 1.f);
					for(int i=0; i<3; ++i)
					{
						CreateParticle(BITMAP_SPARK+1, o->Position, o->Angle, o->Light, 13, 1.0f);
					}
					
					// 폭발 검사
					CheckTargetRange(o);
				}
				else if(o->SubType == 1)
				{
					vec3_t vLight;
					Vector(1.0f, 1.0f, 1.0f, vLight);
					if(o->LifeTime >= 5)
					{
						Vector(0.1f, 0.5f, 1.5f, vLight);
						CreateSprite(BITMAP_SHINY+5, o->Position, 3.0f, vLight, o, fRot);
						CreateSprite(BITMAP_SHINY+5, o->Position, 2.0f, vLight, o, -fRot);
						// pin_light
						Vector(o->Light[0]*0.3f, o->Light[1]*0.3f, o->Light[2]*1.0f, vLight);
						CreateSprite(BITMAP_PIN_LIGHT, o->Position, 4.0f, vLight, o, (float)(rand()%360));
						CreateSprite(BITMAP_PIN_LIGHT, o->Position, 4.0f, vLight, o, (float)(rand()%360));
						// thunder
						CreateSprite(BITMAP_ENERGY, o->Position, 4.0f, o->Light, o, fRot);
					}
					
					// 스파클 튀기는거
					if(o->LifeTime >= 15)
					{
						for(int i=0; i<5; ++i)
						{
							CreateParticle(BITMAP_SPARK+1, o->Position, o->Angle, o->Light, 20, 1.0f);
						}
					}
					
					// shockwave
					if(o->LifeTime >= 14)
					{
						Vector(0.4f, 0.3f, 1.0f, vLight);
						CreateParticle(BITMAP_SHOCK_WAVE, o->Position, o->Angle, vLight, 0, 0.3f);
						CreateParticle(BITMAP_SHOCK_WAVE, o->Position, o->Angle, vLight, 0, 0.3f);
					}
					
					// 번개
					Vector(0.2f, 0.2f, 1.0f, vLight);
					for(int i=0; i<2; ++i)
					{
						CreateEffect(MODEL_FENRIR_THUNDER, o->Position, o->Angle, vLight, 3, o);
						
					}
					
					// 연기
					if(o->LifeTime <= 5)
					{
						for(int i=0; i<2; ++i)
						{
							// 색깔 고정
							CreateParticle(BITMAP_SMOKE, o->Position, o->Angle, o->Light, 40);
						}
					}
					
					o->Light[0] /= 1.08f;
					o->Light[1] /= 1.08f;
					o->Light[2] /= 1.08f;
				}
			}
			break;
			// ChainLighting
		case MODEL_CHAIN_LIGHTNING:
			{
				switch( o->SubType )
				{
				case 0:
				case 1:
				case 2:
					{	
						OBJECT* pSourceObj = o->Owner;						// 시전자의 오브젝트
						CHARACTER* pTargetChar = &CharactersClient[FindCharacterIndex(o->m_sTargetIndex)];
						OBJECT* pTargetObj = &pTargetChar->Object;
						
						if( pSourceObj == NULL || pSourceObj->Live == false
							|| pTargetObj == NULL || pTargetObj->Live == false || (bool)(pTargetChar->Dead) == true )
						{
							o->LifeTime = 0;
							o->Live = false;
							break;
						}
						
						BMD *pSourceModel = &Models[pSourceObj->Type];
						BMD *pTargetModel = &Models[pTargetObj->Type];
						
						vec3_t vLight, vRelativePos, vPos, vAngle;
						Vector(0.0f, 0.0f, 0.0f, vRelativePos);
						
						if( o->SubType == 1 || o->SubType == 2)
						{
							if( pSourceObj == pTargetObj)
								break;							
						}

						// 시전자 -> 몬스터 체인라이트닝
						if( o->SubType == 0)
						{				
							// o : 이펙트 오브젝트
							VectorCopy ( pSourceObj->Position, pSourceModel->BodyOrigin );		// 갱신해주어야 한다.
							
							// 왼팔
							Vector( 0.4f, 0.4f, 1.0f, vLight);
							pSourceModel->TransformPosition( pSourceObj->BoneTransform[37], vRelativePos, vPos, true);	
							Vector(-60.f,0.f, pSourceObj->Angle[2], vAngle);
							CreateJoint(BITMAP_JOINT_THUNDER, vPos, pTargetObj->Position, vAngle, 0, pTargetObj, 50.f, -1, 0, 0, -1, vLight );
							CreateJoint(BITMAP_JOINT_THUNDER, vPos, pTargetObj->Position, vAngle, 0, pTargetObj, 10.f, -1, 0, 0, -1, vLight );
							Vector(0.f, 0.f, (pSourceObj->Angle[2])+60.f, vAngle);
							CreateJoint(BITMAP_JOINT_THUNDER, vPos, pTargetObj->Position, vAngle, 0, pTargetObj, 50.f, -1, 0, 0, -1, vLight );
							CreateJoint(BITMAP_JOINT_THUNDER, vPos, pTargetObj->Position, vAngle, 0, pTargetObj, 10.f, -1, 0, 0, -1, vLight );

							// 오른팔
							pSourceModel->TransformPosition( pSourceObj->BoneTransform[28], vRelativePos, vPos, true);
							Vector(-60.f,0.f, pSourceObj->Angle[2], vAngle);
							CreateJoint(BITMAP_JOINT_THUNDER, vPos, pTargetObj->Position, vAngle, 0, pTargetObj, 50.f, -1, 0, 0, -1, vLight );
							CreateJoint(BITMAP_JOINT_THUNDER, vPos, pTargetObj->Position, vAngle, 0, pTargetObj, 10.f, -1, 0, 0, -1, vLight );
							Vector(0.f, 0.f, (pSourceObj->Angle[2])-60.f, vAngle);
							CreateJoint(BITMAP_JOINT_THUNDER, vPos, pTargetObj->Position, vAngle, 0, pTargetObj, 50.f, -1, 0, 0, -1, vLight );
							CreateJoint(BITMAP_JOINT_THUNDER, vPos, pTargetObj->Position, vAngle, 0, pTargetObj, 10.f, -1, 0, 0, -1, vLight );
						}
						else if( o->SubType == 1 || o->SubType == 2)
						{
							VectorCopy( pSourceObj->Position, vPos );
							vPos[2] += 80.0f;
							//Vector(0.f, 0.f, (pSourceObj->Angle[2])-60.f, vAngle);
							vec3_t vTargetPos;
							VectorCopy( pTargetObj->Position, vTargetPos );
							vTargetPos[2] += 80.0f;
							//Vector(0.f, 0.f, 0.f, vAngle);
							// 몬스터 -> 몬스터 체인 라이트닝
							CreateJoint(BITMAP_JOINT_THUNDER, vPos, pTargetObj->Position, pTargetObj->Angle, 0, pTargetObj, 50.f, -1, 0, 0, -1, vLight );
							CreateJoint(BITMAP_JOINT_THUNDER, vPos, pTargetObj->Position, pTargetObj->Angle, 0, pTargetObj, 10.f, -1, 0, 0, -1, vLight );
						}
						
						
						// 타겟 이팩트
						if( o->LifeTime == 15 )
						{
							int iNumBones = pTargetModel->NumBones;
							float fRandom;
							for( int i=0 ; i<iNumBones ; i++)
							{
								VectorCopy ( pTargetObj->Position, pTargetModel->BodyOrigin );		// 갱신해주어야 한다.
								pTargetModel->TransformPosition( pTargetObj->BoneTransform[i], vRelativePos, vPos, true );
					
								Vector( 0.2f, 0.2f, 0.8f, vLight );
								fRandom = 3.0f + ((float)(rand()%20-10)*0.1f);	
								CreateParticle( BITMAP_LIGHT, vPos, vAngle, vLight, 5, fRandom);
							}
						}	
					}
					break;
				}
			}
			break;
			
			
		case MODEL_ALICE_DRAIN_LIFE:			// Drain Life (드레인 라이프)
			{
				int iNumBones = 0;

				vec3_t vSourcePos, vTargetPos, vLight, vRelativePos;
				Vector( 0.0f, 0.0f, 0.0f, vRelativePos );
				
				OBJECT* pSourceObj = o->Owner;
				OBJECT* pTargetObj = pSourceObj->Owner;
				BMD *pSourceModel = &Models[pSourceObj->Type];
				BMD *pTargetModel = &Models[pTargetObj->Type];

				// 공격자나 타겟이 없거나, 죽었을때는 break!!
				if( pSourceObj == NULL || pTargetObj == NULL ||
					pSourceObj->Live == false || pTargetObj->Live ==false )
				{
					break;
				}

				// 몸의 파티클생성 랜덤값 설정
				int iRandom = rand()%10;
				int iCnt = 0;
				switch( iRandom )
				{
				case 0:
					iCnt = 0;
					break;
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
				case 6:
				case 7:
					iCnt = 1;
					break;
				case 8:
				case 9:
					iCnt = 2;
					break;
				}
				
				// 스킬 시전자 이팩트
				for( int i=0 ; i<iCnt ; i++ )
				{
					VectorCopy( pSourceObj->Position, vSourcePos );
					vSourcePos[0] += ((float)((rand()%80-40)));
					vSourcePos[1] += ((float)((rand()%60-30)));
					vSourcePos[2] += (80.0f+((float)((rand()%180-100))));
					Vector( 1.0f, 0.2f, 0.2f, vLight );
					CreateParticle(BITMAP_LIGHT+2, vSourcePos, pSourceObj->Angle, vLight, 7, 1.8f);
				}
				
				// 타겟 이팩트
				if( o->LifeTime <= 60 )
				{
					for( int i=0 ; i<iCnt ; i++ )
					{
						VectorCopy( pTargetObj->Position, vTargetPos );
						vTargetPos[0] += ((float)((rand()%80-40)));
						vTargetPos[1] += ((float)((rand()%60-30)));
						vTargetPos[2] += (80.0f+((float)((rand()%180-100))));
						Vector( 1.0f, 0.2f, 0.2f, vLight );
						CreateParticle(BITMAP_LIGHT+2, vTargetPos, pTargetObj->Angle, vLight, 7, 1.8f);
					}
				}
				
				if( o->LifeTime <= 70 && o-> LifeTime >=66 )
				{
					// 드레인 라이프 고스트생성 랜덤값 설정
					// 5프레임에 걸쳐 0,1,2,3개 생성
					// 0 - 10% / 1 - 30% / 2- 40% / 3 - 20%
					int iRandom = rand()%10;
					int iCnt2 = 0;
					switch( iRandom )
					{
					case 0:
						iCnt2 = 0;
						break;
					case 1:
					case 2:
					case 3:
						iCnt2 = 1;
						break;
					case 4:
					case 5:				
						iCnt2 = 2;
					case 6:
					case 7:		
					case 8:			
						iCnt2 = 3;
						break;
					case 9:
						iCnt2 = 4;
						break;
					}	

					float fMatrix[3][4];
					vec3_t vDir;
					// 랜덤값에 따라 고스트 이팩트 생성
					for( int i=0 ; i<iCnt2 ; i++)
					{							
						VectorCopy(pSourceObj->Position, pSourceModel->BodyOrigin );
						pSourceModel->TransformPosition( pSourceObj->BoneTransform[18], vRelativePos, vSourcePos, true );
						
						// 캐릭터의 방향을 구한다.
						AngleMatrix( pSourceObj->Angle, fMatrix );			
						vDir[0] = fMatrix[0][1];
						vDir[1] = fMatrix[1][1];
						vDir[2] = fMatrix[2][1];
						
						VectorNormalize( vDir );
						
						vSourcePos[0] = vSourcePos[0] + ((vDir[0])*100.0f) + (float)((rand()%10)*5);
						vSourcePos[1] = vSourcePos[1] + ((vDir[1])*100.0f) + (float)((rand()%10)*5);
						vSourcePos[2] += (float)((rand()%10)*5);
						
						VectorCopy( pTargetObj->Position, vTargetPos );
						vTargetPos[2] += 100.f+(float)(((rand()%10-5))*4);		// 80~120
						
						Vector( 0.8f, 0.1f, 0.2f, vLight );
						CreateJoint( BITMAP_DRAIN_LIFE_GHOST, vSourcePos, vTargetPos, o->Angle, 0, pSourceObj, 40.f, 0, 0, 0, -1, vLight );
					}
				}	
				
				// 타겟 몬스터의 이펙트
				if( o->LifeTime == 64 )
				{				
					VectorCopy(pSourceObj->Position, pSourceModel->BodyOrigin );
					pSourceModel->TransformPosition( pSourceObj->BoneTransform[18], vRelativePos, vSourcePos, true );
					
					iNumBones = pTargetModel->NumBones;		
					
					for( int i=0 ; i<iNumBones ; i++)
					{
						VectorCopy ( pTargetObj->Position, pTargetModel->BodyOrigin );		// 갱신해주어야 한다.
						pTargetModel->TransformPosition( pTargetObj->BoneTransform[i], vRelativePos, vTargetPos, true);
						
						Vector( 0.4f, 0.4f, 0.8f, vLight );
						//CreateParticle( BITMAP_LIGHT, vTargetPos, pTargetObj->Angle, vLight, 14, 3.5f);
						
						if( rand()%2 == 0)
						{
							VectorCopy( pSourceObj->Position, vSourcePos );
							vSourcePos[2] += 80.0f;
							// 몬스터로부터 흡수하는 에너지 이팩트
							Vector( 1.0f, 0.0f, 0.1f, vLight );
							CreateJoint( BITMAP_JOINT_ENERGY, vTargetPos, vSourcePos, pTargetObj->Angle, 45, pSourceObj, 10.0f, -1, 0, 0, -1, vLight);
						}
					}
				}
			}
			break;

		case MODEL_ALICE_BUFFSKILL_EFFECT:
		case MODEL_ALICE_BUFFSKILL_EFFECT2:
			{
				if(o->SubType == 0 || o->SubType == 1 || o->SubType == 2)
				{
					// 공통부분
					if(o->Owner == NULL || o->Owner->Live == false)
					{
						o->Live = false;
					}
					
					VectorCopy(o->Owner->Position, o->Position);
					o->Position[2] += 100;
					if(o->LifeTime > 20)
					{
						o->Alpha += 0.05f;
						o->BlendMeshLight += 0.05f;
					}
					else
					{
						o->Alpha -= 0.05f;
						o->BlendMeshLight -= 0.05f;
						if(o->Alpha < 0.f)
						{
							o->Live = false;
						}
					}

					if(o->Type == MODEL_ALICE_BUFFSKILL_EFFECT)
					{
						o->Angle[2] += 8.f;
					}
					else if(o->Type == MODEL_ALICE_BUFFSKILL_EFFECT2)
					{
						o->Angle[2] -= 8.f;
					}
					// 회전속도
					
					o->Scale += 0.035f;

					float fRot = (WorldTime*0.0006f) * 360.0f;
					vec3_t vLight;

					// 각각 서브타입별로 다른 부분
					// flare01
					if(o->SubType == 0)		// 슬립
					{
						Vector(0.8f*o->Alpha, 0.1f*o->Alpha, 0.9f*o->Alpha, vLight);
					}
					else if(o->SubType == 1)
					{
						Vector(1.0f*o->Alpha, 1.0f*o->Alpha, 1.0f*o->Alpha, vLight);
					}
					else if(o->SubType == 2)		// 쏜즈
					{
						Vector(0.8f*o->Alpha, 0.5f*o->Alpha, 0.2f*o->Alpha, vLight);
					}

					if(o->SubType == 0 || o->SubType == 2)
					{
						CreateSprite(BITMAP_LIGHT, o->Position, 5.f, vLight, o, 0.f, 0);
						CreateSprite(BITMAP_LIGHT, o->Position, 5.f, vLight, o, 0.f, 0);
					}
					else if(o->SubType == 1) 
					{
						CreateSprite(BITMAP_LIGHT, o->Position, 5.f, vLight, o, 0.f, 1);
						CreateSprite(BITMAP_LIGHT, o->Position, 5.f, vLight, o, 0.f, 1);
					}

					// shiny04
					if(o->SubType == 0)
					{
						Vector(0.7f*o->Alpha, 0.6f*o->Alpha, 0.9f*o->Alpha, vLight);
					}
					else if(o->SubType == 1)
					{
						Vector(1.0f*o->Alpha, 1.0f*o->Alpha, 1.0f*o->Alpha, vLight);	
					}
					else if(o->SubType == 2)
					{
						Vector(0.8f*o->Alpha, 0.5f*o->Alpha, 0.2f*o->Alpha, vLight);	
					}

					if(o->SubType == 0 || o->SubType == 2)
					{
						CreateSprite(BITMAP_SHINY+5, o->Position, 2.0f, vLight, o, fRot);
						CreateSprite(BITMAP_SHINY+5, o->Position, 1.0f, vLight, o, -fRot);
					}
					else if(o->SubType == 1)
					{
						CreateSprite(BITMAP_SHINY+5, o->Position, 2.0f, vLight, o, fRot, 1);
						CreateSprite(BITMAP_SHINY+5, o->Position, 1.0f, vLight, o, -fRot, 1);
					}

					vec3_t vAngle, vPos, vWorldPos;
					float Matrix[3][4];
					Vector(0.f, -200.f, 0.f, vPos);
					for(int i=0; i<3; ++i)
					{
						Vector((float)(rand()%90), 0.f, (float)(rand()%360), vAngle);
						AngleMatrix(vAngle, Matrix);
						VectorRotate(vPos, Matrix, vWorldPos);
						VectorSubtract(o->Position, vWorldPos, vWorldPos);
						
						if(o->SubType == 0)
						{
							Vector(0.7f, 0.5f, 0.7f, vLight);
						}
						else if(o->SubType == 1)
						{
							Vector(1.0f, 1.0f, 1.0f, vLight);	
						}
						else if(o->SubType == 2)
						{
							Vector(0.8f, 0.5f, 0.2f, vLight);	
						}
						
						if(o->SubType == 0 || o->SubType == 2)
						{
							CreateJoint(BITMAP_JOINT_HEALING, vWorldPos, o->Position, vAngle, 15, o, 5.f, 0, 0, 0, 0, vLight);
						}
						else if(o->SubType == 1)
						{
							CreateJoint(BITMAP_JOINT_HEALING, vWorldPos, o->Position, vAngle, 16, o, 5.f, 0, 0, 0, 0, vLight);	
						}
					}
				}
				else if(o->SubType == 3 || o->SubType == 4)
				{
					if(o->Owner == NULL || o->Owner->Live == false)
					{
						o->Live = false;
					}
					else
					{
						o->LifeTime = 100;

						BMD* pModel = &Models[o->Owner->Type];
						int iBone = rand() % pModel->NumBones;

						vec3_t vRelativePos, vWorldPos;
						Vector(0.f, 0.f, 0.f, vRelativePos);

						if(!pModel->Bones[iBone].Dummy)
						{
							pModel->TransformPosition(o->Owner->BoneTransform[iBone], vRelativePos, vWorldPos, false);
							VectorScale(vWorldPos, pModel->BodyScale, vWorldPos);
							VectorAdd(vWorldPos, o->Owner->Position, vWorldPos);

							if(o->SubType == 3)
							{
								CreateParticle(BITMAP_LIGHT+2, vWorldPos, o->Angle, o->Light, 6, o->Scale);
								iBone = rand() % pModel->NumBones;
								pModel->TransformPosition(o->Owner->BoneTransform[iBone], vRelativePos, vWorldPos, false);
								VectorScale(vWorldPos, pModel->BodyScale, vWorldPos);
								VectorAdd(vWorldPos, o->Owner->Position, vWorldPos);
								CreateParticle(BITMAP_LIGHT+2, vWorldPos, o->Angle, o->Light, 6, o->Scale);

							}
							else if(o->SubType == 4)
							{
								if(rand()%2 == 0)
								{
									vWorldPos[2] -= 20.f;
									CreateParticle(BITMAP_TWINTAIL_WATER, vWorldPos, o->Angle, o->Light, 2);
								}
							}
						}
					}
				}
			}
			break;	

#ifdef YDG_ADD_SKILL_LIGHTNING_SHOCK
	case MODEL_LIGHTNING_SHOCK:
		{
			vec3_t vLight;
			
			if(o->SubType == 0)
			{
				if (o->Owner->AnimationFrame > 6.0f || o->LifeTime < 15)
				{
					Vector(0.f, -20.f, -75.f - o->Velocity, o->Direction);
					o->Gravity += 0.1f;
					o->Velocity += o->Gravity;
				}

				OBJECT * pObject = o;
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
				BMD * pModel = &Models[pObject->Type];
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
				OBJECT * pOwner = pObject->Owner;
				BMD * pOwnerModel = &Models[pOwner->Type];

				float fRot = (WorldTime*0.0006f) * 360.0f;
				float fScale = 0.8f;

				// shiny
				Vector(1.0f, 0.4f, 0.4f, vLight);
				CreateSprite(BITMAP_SHINY+1, o->Position, 4.0f * fScale, vLight, o, fRot);
				CreateSprite(BITMAP_SHINY+1, o->Position, 3.0f * fScale, vLight, o, -fRot);
				// magic_ground
				Vector(1.0f, 0.2f, 0.2f, vLight);
				CreateSprite(BITMAP_MAGIC, o->Position, 1.0f * fScale, vLight, o, fRot);
				CreateSprite(BITMAP_MAGIC, o->Position, 0.5f * fScale, vLight, o, -fRot);
				// pin_light
				Vector(1.0f, 0.4f, 0.4f, vLight);
				CreateSprite(BITMAP_PIN_LIGHT, o->Position, 2.0f * fScale, vLight, o, (float)(rand()%360));
				CreateSprite(BITMAP_PIN_LIGHT, o->Position, 2.0f * fScale, vLight, o, (float)(rand()%360));
				
				// 잔상
				Vector(1.0f, 0.3f, 0.3f, vLight);
				CreateParticle(BITMAP_MAGIC, o->Position, o->Angle, vLight, 0, 1.f * fScale);

				// 구에 전기
				vec3_t vPos, vLightFlare, vRelative;
				Vector(1.0f, 0.7f, 0.4f, vLight);
				Vector(1.0f, 0.2f, 0.1f, vLightFlare);
				for (int i = 0; i < 11; ++i)
				{
					fScale = (float)(rand()%80+32)*0.01f*1.0f;
					Vector(o->Position[0]+(rand()%70-35)*1.0f, o->Position[1]+(rand()%70-35)*1.0f,
						o->Position[2]+(rand()%70-35)*1.0f, vPos);
					CreateSprite(BITMAP_LIGHT, vPos, 2.2f, vLightFlare, pObject);
					if (rand()%3 > 0) continue;
					CreateParticle(BITMAP_LIGHTNING_MEGA1+rand()%3,vPos,pObject->Angle,vLight,0, fScale);	// 전기
				}

				// 몸에 전기
				Vector(1.0f, 0.5f, 0.4f, vLight);
#ifdef _VS2008PORTING
				for (int i = 0; i < 2; ++i)
#else // _VS2008PORTING
				for (i = 0; i < 2; ++i)
#endif // _VS2008PORTING
				{
					fScale = (float)(rand()%60+22)*0.01f*1.0f;
					int iBone = rand() % 41;
					Vector((rand()%30-15)*1.0f, (rand()%30-15)*1.0f, (rand()%30-15)*1.0f, vRelative);
					pOwnerModel->TransformByObjectBone(vPos, pOwner, iBone, vRelative);
					CreateParticle(BITMAP_LIGHTNING_MEGA1+rand()%3,vPos,pObject->Angle,vLight,0, fScale);	// 전기
				}

				float Height = RequestTerrainHeight(o->Position[0],o->Position[1]);
				if (o->Position[2] < Height)
				{
					CreateEffect(MODEL_LIGHTNING_SHOCK, o->Position, o->Angle, o->Light, 1, o);
					EffectDestructor(o);
				}
			}
			else if(o->SubType == 1)
			{
				OBJECT * pObject = o;
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
				BMD * pModel = &Models[pObject->Type];
				OBJECT * pOwner = pObject->Owner;
				BMD * pOwnerModel = &Models[pOwner->Type];
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX

				// 전기
				float fScale = 0.8f;
				vec3_t vPos, vLightFlare;
				Vector(1.0f, 0.7f, 0.4f, vLight);
				Vector(1.0f, 0.2f, 0.1f, vLightFlare);
				for (int i = 0; i < 11; ++i)
				{
					fScale = (float)(rand()%80+32)*0.01f*1.0f;
					Vector(o->Position[0]+(rand()%70-35)*1.0f, o->Position[1]+(rand()%70-35)*1.0f,
						o->Position[2]+(rand()%70-35)*1.0f, vPos);
					CreateParticle(BITMAP_LIGHTNING_MEGA1+rand()%3,vPos,pObject->Angle,vLight,0, fScale);	// 전기
				}

				// 바닥 전기
				vec34_t Matrix;
				vec3_t vAngle, vDirection, vPosition;
				float fAngle;
				Vector(1.0f, 0.0f, 0.0f, vLight);
#ifdef _VS2008PORTING
				for (int i = 0; i < 6; ++i)
#else // _VS2008PORTING
				for (i = 0; i < 6; ++i)
#endif // _VS2008PORTING
				{
					fScale = (float)(rand()%60+22)*0.01f*1.0f;
					Vector(0.f, rand()%400, 0.f, vDirection );
					fAngle = o->Angle[2] + rand()%360;
					Vector(0.f, 0.f, fAngle, vAngle);
					AngleMatrix(vAngle, Matrix);
					VectorRotate(vDirection, Matrix, vPosition);
					VectorAdd(vPosition, o->Position, vPosition);
					vPosition[2] = RequestTerrainHeight(vPosition[0],vPosition[1])+20;
					
					CreateParticle(BITMAP_LIGHTNING_MEGA1+rand()%3,vPosition,pObject->Angle,vLight,0, fScale);	// 전기
				}

				VectorCopy(o->Position, vPosition);
				vPosition[2] = RequestTerrainHeight(vPosition[0],vPosition[1])+10;

				// 독 연기 이펙트
				Vector(1.0f, 0.0f, 0.0f, vLight);
#ifdef _VS2008PORTING
				for(int i=0; i<2; i++)
#else // _VS2008PORTING
				for(i=0; i<2; i++)
#endif // _VS2008PORTING
					CreateParticle(BITMAP_SMOKE, vPosition, o->Angle, vLight, 58);

				// 연기
				if (rand()%2 == 0)
				{
					Vector(1.0f, 0.0f, 0.0f, vLight);
					CreateParticle(BITMAP_SMOKE,vPosition,o->Angle,vLight,54,2.8f);
				}
				// 돌
				CreateEffect(MODEL_STONE1+rand()%2,vPosition,o->Angle,vLight,13,o);
			}
			else if(o->SubType == 2)
			{
				if (o->Owner != NULL && o->Owner->BoneTransform != NULL)
				{
					BMD *pTargetModel = &Models[o->Owner->Type];
					int iNumBones = pTargetModel->NumBones;
					float fRandom;
					vec3_t vLight, vRelativePos, vPos, vAngle;
					Vector(0.0f, 0.0f, 0.0f, vRelativePos);
					for( int i=0 ; i<iNumBones ; i++)
					{
						VectorCopy ( o->Owner->Position, pTargetModel->BodyOrigin );		// 갱신해주어야 한다.
						pTargetModel->TransformPosition( o->Owner->BoneTransform[i], vRelativePos, vPos, true );
			
						Vector(0.8f, 0.0f, 0.0f, vLight);
						if (rand()%60 == 0)
						{
							fRandom = 5.0f + ((float)(rand()%20-10)*0.1f);	
							CreateParticle( BITMAP_LIGHT, vPos, vAngle, vLight, 5, fRandom);
						}
						Vector(1.0f, 0.8f, 0.4f, vLight);
						if (rand()%5 == 0)
						{
							fRandom = (float)(rand()%70+22)*0.01f*1.0f;
							CreateParticle(BITMAP_LIGHTNING_MEGA1+rand()%3,vPos,vAngle,vLight,0, fRandom);	// 전기
						}
					}

					// 독 연기 이펙트
					Vector(1.0f, 0.0f, 0.0f, vLight);
					//for(i=0; i<2; i++)
						CreateParticle(BITMAP_SMOKE, o->Position, o->Angle, vLight, 58);

					// 연기
					if (rand()%4 == 0)
					{
						Vector(1.0f, 0.0f, 0.0f, vLight);
						CreateParticle(BITMAP_SMOKE,o->Position,o->Angle,vLight,54,2.8f);
					}
				}
			}
		}
		break;
#endif	// YDG_ADD_SKILL_LIGHTNING_SHOCK

	case BITMAP_LIGHTNING+1:
		if(o->SubType == 0)
		{
	   		Luminosity = o->LifeTime*0.2f;
			Vector(Luminosity*0.2f,Luminosity*0.5f,Luminosity*1.f,Light);
			AddTerrainLight(o->Position[0],o->Position[1],Light,2,PrimaryTerrainLight);
		}
		else if(o->SubType == 1)
		{
			o->Alpha += 0.01f;
			if(o->Alpha >= 1.0f)
			{
				o->Alpha = 1.0f;
			}
		}
		break;
	case BITMAP_LIGHT:
        if ( o->SubType==0 )
        {
		    VectorAdd( o->Position, o->Direction, o->Position);
		    //o->Direction[2] -= .35f;
		    o->Direction[2] -= .01f;
		    //VectorScale( o->Light, 0.9f, o->Light);
		    CreateParticle( BITMAP_LIGHT, o->Position, o->Angle, o->Light, 1, o->Scale);
		    if ( o->Direction[2] < -2.f)
		    {
			    o->LifeTime = 0;
			    /*vec3_t Light = { 1.f, 1.f, 1.f};
			    CreateParticle(BITMAP_EXPLOTION,o->Position,o->Angle,o->Light);*/
		    }
        }
        else if ( o->SubType==1 )
        {
			if ( o->Owner!=NULL && o->Owner->Live==true && g_isCharacterBuff(o->Owner, eBuff_HpRecovery) ) 
                o->LifeTime = 10;
            else 
                o->LifeTime = 0;

            //  투명일때 보조계열 효과는 보이지 않는다.
			if( g_isCharacterBuff(o->Owner, eBuff_Cloaking) ) break;

            Vector ( 1.f, 0.5f, 0.1f, Light );
            Vector ( 0.f, 0.f, 0.f, Angle );
            
            Index = rand()%7;
            Luminosity = g_byUpperBoneLocation[Index];
            CreateParticle ( BITMAP_LIGHT, o->Position, Angle, Light, 4, Luminosity, o->Owner );

            Luminosity = g_byUpperBoneLocation[6-Index];
            CreateParticle ( BITMAP_LIGHT, o->Position, Angle, Light, 4, Luminosity, o->Owner );
        }
        else if ( o->SubType==2 )
        {
			if ( o->Owner!=NULL && o->Owner->Live==true && g_isCharacterBuff(o->Owner, eBuff_AddAG ) ) 
                o->LifeTime = 10;
            else 
                o->LifeTime = 0;

            //  투명일때 보조계열 효과는 보이지 않는다.
			if( g_isCharacterBuff(o->Owner, eBuff_Cloaking ) ) break;

            o->Velocity ++;

            if ( ((int)(o->Velocity)%50)==0 )
            {
                if ( o->Owner!=NULL )
                {
	                vec3_t Position;
                    o->Angle[2] += 50.f;
                    VectorCopy ( o->Owner->Position, Position );

                    o->Position[0] = o->Owner->Position[0]+sinf( o->Angle[2]*0.1f )*80.f;
                    o->Position[1] = o->Owner->Position[1]+cosf( o->Angle[2]*0.1f )*80.f;
                    o->Position[2] = o->Owner->Position[2] + 50;

		            CreateJoint(BITMAP_JOINT_HEALING,o->Position,o->Position,o->Angle,9,o->Owner,15.f);
                }
            }
        }
        else if ( o->SubType==3 )
        {
            o->Velocity ++;

            if ( (o->LifeTime%15)==0 )
            {
                if ( o->Owner!=NULL )
                {
	                vec3_t Position;
                    o->Angle[2] += 180;
                    VectorCopy ( o->Owner->Position, Position );

                    o->Position[0] = o->Owner->Position[0]+sinf( o->Angle[2]*0.1f )*50.f;
                    o->Position[1] = o->Owner->Position[1]+cosf( o->Angle[2]*0.1f )*50.f;
                    o->Position[2] = o->Owner->Position[2] + 20;

		            CreateJoint(BITMAP_JOINT_HEALING,o->Position,o->Position,o->Angle,9,o->Owner,10.f);
                }
            }
        }
		break;
	case MODEL_SKILL_BLAST:
		Height = RequestTerrainHeight(o->Position[0],o->Position[1]);
		if(o->Position[2] < Height)
		{
			o->Position[2] = Height;
			Vector(0.f,0.f,0.f,o->Direction);
			vec3_t Position;
			Vector(o->Position[0],o->Position[1],o->Position[2]+80.f,Position);
			for(int j=0;j<6;j++)
			{
				CreateEffect(MODEL_STONE1+rand()%2,o->Position,o->Angle,o->Light);
			}
			CreateParticle(BITMAP_SHINY+4,Position,o->Angle,Light);
			CreateParticle(BITMAP_EXPLOTION,Position,o->Angle,Light);
      		if(o->Owner == &Hero->Object)
	       		AttackCharacterRange(o->Skill,o->Position,150.f,o->Weapon,o->PKKey);//클라이언트마법처리
			o->Live = false;
		}
		VectorCopy(o->Position,o->EyeLeft);
		//CreateSprite(BITMAP_SHINY+4,o->Position,2.5f,Light,o);
		Vector(Luminosity*0.2f,Luminosity*0.4f,Luminosity*1.f,Light);
		AddTerrainLight(o->Position[0],o->Position[1],Light,2,PrimaryTerrainLight);
		break;
    case MODEL_WAVE:
        if( o->Scale>2.f )
        {
            o->Scale += 0.1f;
            o->Position[0] -= 1.f;
            o->Position[2] -= 1.5f;
			o->BlendMeshLight = o->LifeTime/30.f;
        }
        else
        {
            o->Scale += 1.2f;
            o->Position[0] -= 1.2f;
            o->Position[2] -= 1.8f;
        }

		Vector(-Luminosity*0.5f,-Luminosity*0.5f,-Luminosity*0.5f,Light);
		AddTerrainLight(o->Position[0],o->Position[1],Light,5,PrimaryTerrainLight);
        break;
    case MODEL_TAIL:
        o->Position[2] -= o->Gravity;
        o->Gravity     += 60.f;
		o->BlendMeshLight = o->LifeTime/20.f;
        break;
	case MODEL_WAVE_FORCE:
		o->BlendMeshLight = o->LifeTime/20.f;
		Vector(-Luminosity*0.5f,-Luminosity*0.5f,-Luminosity*0.5f,Light);
		AddTerrainLight(o->Position[0],o->Position[1],Light,5,PrimaryTerrainLight);
		break;
	case MODEL_SKILL_INFERNO:
        if ( o->SubType==2 )
        {
            o->Scale += 0.04f;
        }
		else if(o->SubType == 8)
		{
			o->Scale += 0.04f;
			if(o->LifeTime < 10)
			{
				o->Light[0] *= 0.8f;
				o->Light[1] *= 0.8f;
				o->Light[2] *= 0.8f;
			}
		}
        else if(o->SubType==3)
        {
            VectorCopy(o->Owner->Position, o->Position);
            o->Position[0] = o->Owner->Owner->Position[0] + 150.f;
        }
        else if(o->SubType==4)
        {
            VectorCopy(o->Owner->Position, o->Position);

            o->Scale   += 0.003f;
            o->Gravity += 0.8f*o->Scale*30.f;
            o->Position[2] += o->Gravity;
        }
        else if ( o->SubType==5 )   //  회전, 승천 인페르노.
        {
            o->Position[2] += 2.f;
            o->Angle[2] += 20.f;
			o->BlendMeshLight = o->LifeTime/20.f;
			Vector(Luminosity*0.1f,Luminosity*0.3f,Luminosity*0.8f,Light);
			AddTerrainLight(o->Position[0],o->Position[1],Light,5,PrimaryTerrainLight);
        }
        else if ( o->SubType==6 )
        {
            o->Scale += 0.01f;
			o->BlendMeshLight = o->LifeTime/5.f*0.1f;
			Vector ( Luminosity*0.8f, Luminosity*0.3f, Luminosity*0.1f, Light );
			AddTerrainLight(o->Position[0],o->Position[1],Light,2,PrimaryTerrainLight);
        }

        if ( o->SubType<4 )
        {
			o->BlendMeshLight = o->LifeTime/20.f;
			if ( o->SubType!=2 )
			{
				Vector(-Luminosity*0.5f,-Luminosity*0.5f,-Luminosity*0.5f,Light);
				AddTerrainLight(o->Position[0],o->Position[1],Light,5,PrimaryTerrainLight);
			}
        }
		else if(o->SubType == 8)
		{
			o->BlendMeshLight = o->LifeTime/20.f;
		}
		else if(o->SubType == 9)
		{
			o->BlendMeshLight = o->LifeTime/80.f;
			o->Position[2] += o->Gravity;
			o->Light[0] /= 1.04f;
			o->Light[2] /= 1.04f;
			o->Alpha -= 0.01f;
		}
		else if( o->SubType == 10 )
		{
			o->Scale += 0.04f;
			BMD *b = &Models[o->Type];
			VectorCopy( o->Light, b->BodyLight );
		}
		break;
	case MODEL_MAGIC_CIRCLE1:
        if(o->SubType==2)
        {
			o->Scale += 0.015f;
			VectorCopy(o->Owner->Position,o->Position);
			Vector( 0.1f, 0.0f, 0.0f, o->Light);
        }
        else if(o->SubType==1)
        {
			o->Scale += 0.01f;
			Vector(Luminosity*1.0f,Luminosity*0.0f,Luminosity*0.0f,Light);
			AddTerrainLight(o->Position[0],o->Position[1],Light,3,PrimaryTerrainLight);
        }
        else
        {
			VectorCopy(o->Owner->Position,o->Position);
			o->Scale += 0.01f;
        }
		break;
	case MODEL_BIG_METEO1:
	case MODEL_BIG_METEO2:
	case MODEL_BIG_METEO3:
		Index = TERRAIN_INDEX_REPEAT((int)o->Position[0]/100,(int)o->Position[1]/100);
		if((TerrainWall[Index]&TW_NOGROUND) != TW_NOGROUND)
		{
			Height = RequestTerrainHeight(o->Position[0],o->Position[1]);
			if(o->Position[2] < Height)
			{
				o->Position[2] = Height;
				o->Live = false;
				EarthQuake = (float)(rand()%8-8)*0.1f;
				for(int j=0;j<5;j++)
					CreateEffect(MODEL_METEO1+rand()%2,o->Position,o->Angle,o->Light,0);
			}
		}
		VectorAdd(o->Position,o->Direction,o->Position);
		o->Angle[0] += 10.f/o->Scale;
   		CreateParticle(BITMAP_SMOKE,o->Position,o->Angle,o->Light,3);
		//CreateParticle(BITMAP_ENERGY,o->Position,o->Angle,Light);
        //CreateParticle(BITMAP_SPARK+1,o->Position,o->Angle,Light,0,4.f);
		return;
	case MODEL_PROTECT:
        o->BlendMeshLight = 1.3f;

		VectorCopy(o->Owner->Position,o->Position);
		o->Angle[2] += 10.f;
		if(!o->Owner->Live || !o->Owner->Visible)
			o->Live = false;
		break;
	case MODEL_POISON:
    	o->BlendMeshLight = o->LifeTime*0.1f;
    	o->Alpha = o->LifeTime*0.1f;
		Vector(Luminosity*0.3f,Luminosity*1.f,Luminosity*0.6f,Light);
		AddTerrainLight(o->Position[0],o->Position[1],Light,2,PrimaryTerrainLight);
		break;
	case MODEL_SAW:
		o->Angle[2] -= 30.f;
		break;
	case MODEL_LASER:
        if ( o->SubType!=0 && o->SubType!=3 )
        {
            o->Direction[1] -= o->Velocity;
            o->Velocity += 1.f;
        }
		break;
	case MODEL_MAGIC1:
     	VectorCopy(o->Owner->Position,o->Position);
		o->Position[2] += 100.f;
		o->Angle[1] += 20.f;
    	o->BlendMeshLight = o->LifeTime*0.1f;
		break;
	case MODEL_SKILL_WHEEL1:
        CreateEffect(MODEL_SKILL_WHEEL2,o->Position,o->Angle,o->Light,4-o->LifeTime,o->Owner,o->PKKey,o->Skill,o->Kind);
		break;
	case MODEL_SKILL_WHEEL2:
		switch(o->SubType)
		{
		case 1:o->Alpha = 0.6f;break;
		case 2:o->Alpha = 0.5f;break;
		case 3:o->Alpha = 0.4f;break;
		case 4:o->Alpha = 0.3f;break;
		}
		if(o->Owner->Weapon>=MODEL_SPEAR-MODEL_SWORD && o->Owner->Weapon<MODEL_SPEAR-MODEL_SWORD+MAX_ITEM_INDEX)
		{
			Vector(0.f,-180.f,0.f,p);
		}
		else
		{
			Vector(0.f,-150.f,0.f,p);
		}
		AngleMatrix(o->Angle,Matrix);
		VectorRotate(p,Matrix,Position);
		VectorAdd(Position,o->Owner->Position,o->Position);
		o->Angle[2] -= 18;//
     	CreateParticle(BITMAP_SMOKE,o->Position,o->Angle,o->Light,3);
		Vector(Luminosity*0.3f,Luminosity*0.3f,Luminosity*0.3f,Light);
		AddTerrainLight(o->Position[0],o->Position[1],Light,3,PrimaryTerrainLight);
		Vector(1.f,1.f,1.f,Light);

        Height = 20.f;
        if ( InHellas() )
        {
            Height = 60.f;
            Vector((float)(rand()%60+60-90),0.f,(float)(rand()%30+90),Angle);
            VectorAdd(Angle,o->Angle,Angle);
            VectorCopy(o->Position,Position);
            Position[0] += rand()%20-10;
            Position[1] += rand()%20-10;
            Position[2] += 120.f;
    		Vector ( 0.5f, 0.5f, 0.5f,Light);
            CreateParticle ( BITMAP_WATERFALL_5, Position, Angle, Light, 3 );
        }
        else
        {
#ifdef _VS2008PORTING
            for(int j=0;j<4;j++)
#else // _VS2008PORTING
            for(j=0;j<4;j++)
#endif // _VS2008PORTING
            {
                Vector((float)(rand()%60+60-90),0.f,(float)(rand()%30+90),Angle);
                VectorAdd(Angle,o->Angle,Angle);
                VectorCopy(o->Position,Position);
                Position[0] += rand()%20-10;
                Position[1] += rand()%20-10;
                CreateJoint(BITMAP_JOINT_SPARK,Position,Position,Angle);
                if(j==0) CreateParticle(BITMAP_SPARK,Position,Angle,Light);
            }
        }
		VectorCopy(o->Position,Position);
		Position[2] += Height;
		Vector(1.f,0.8f,0.6f,Light);
		CreateSprite(BITMAP_LIGHT,Position,2.f,Light,o);

        if ( InHellas() && (o->LifeTime%4)==0 )
        {
            int PositionX = (int)(o->Position[0]/TERRAIN_SCALE);
            int PositionY = (int)(o->Position[1]/TERRAIN_SCALE);
            AddWaterWave ( PositionX, PositionY, 2, -150 );
        }
		break;
	case MODEL_SKILL_FISSURE:
		{
			if(o->LifeTime == 8)
			{
				for (int i = 0; i < 16; ++i)
			    {
				    Angle[0] = -10.f;
				    Angle[1] = 0.f;
				    Angle[2] = i*10.f;
				    vec3_t Position;
				    VectorCopy( o->Position, Position);
				    Position[2] += 100.f;

					//if ( (i%8)==0 )
                        //CreateEffect(BITMAP_MAGIC+1,o->Position,Angle,o->Light,4,o);

					Position[0] += rand()%600-200;
					Position[1] += rand()%600-400;
					CreateJoint(BITMAP_FLARE,Position,Position,Angle,24,NULL,90);
			    }
			}
			else if(o->LifeTime == 2)
			{
				Vector(0.f, 0.f, rand()%360, Angle);
				CreateEffect ( MODEL_FISSURE, o->Position, Angle, o->Light, 0, o);
				CreateEffect ( MODEL_FISSURE_LIGHT, o->Position, Angle, o->Light, 0, o);
				o->Live = false;
			}
		}
		break;
	case MODEL_FISSURE:
		{
			double fLevel = sinf(Q_PI * double(o->LifeTime) / 120.f) * -0.5f;
			vec3_t Light;
			Vector ( fLevel, fLevel, fLevel+0.1f, Light );
			AddTerrainLight(o->Position[0], o->Position[1],Light,10,PrimaryTerrainLight);

			EarthQuake = (float)(rand()%4-2)*0.1f;

			if(rand()%2==0) {
				Vector(0.f,(float)(rand()%150)+300.f,0.f,p);
				Vector(0.f,0.f,(float)(rand()%360),Angle);
				AngleMatrix(Angle,Matrix);
				VectorRotate(p,Matrix,Position);
				VectorAdd(Position,o->Position,Position);
				
				CreateEffect(MODEL_STONE1+rand()%2,Position,o->Angle,o->Light,0);
			}

			/* 따로 Visual처리 하는 부분이 없어서 임시로 여기서 셋팅 */
			BMD* pModel = &Models[o->Type];
			pModel->Animation(BoneTransform,o->AnimationFrame,o->PriorAnimationFrame,o->PriorAction,o->Angle,o->HeadAngle,false,false);
			Vector(1.f, 1.f, 1.f, Light);
			for(int i=0; i<pModel->NumBones; i++) {
				if(!pModel->Bones[i].Dummy && rand()%12 == 0)
				{
					Vector(0.f,0.f,0.f,p);
					pModel->TransformPosition(BoneTransform[i],p,Position,true);
					CreateParticle ( BITMAP_TRUE_FIRE, Position, o->Angle, Light, 3, 4.f, o);
					CreateParticle ( BITMAP_SMOKE, Position, o->Angle, Light, 21 , 1.f);
				}
			}
		}
		break;
    case MODEL_SKILL_FURY_STRIKE:
        {
			vec3_t p,Position, Pos[5];
			vec3_t Angle;
            short  scale = 150;
			float  Matrix[3][4], ang[5];
            int    TargetX, TargetY;

            Vector ( 0.f, 0.f, 0.f, Angle );
            Vector ( 0.f, 0.f, 0.f, p );

            if ( o->LifeTime==11 )   //  듬성 듬성 균열들.
            {
                vec3_t  light;
				Vector( 1.f, 1.f, 1.f, light );
                Vector( 0.f, 0.f, 0.f, Angle);

				if(o->Kind != 3) 
				{
					//. 부모오프젝트가 바라보는 방향으로 조금 이동
					Vector(-25.f,-80.f,0.f,p);
					AngleMatrix(o->Owner->Angle,Matrix);
					VectorRotate(p,Matrix,Position);	
					VectorAdd(Position,o->Position,o->StartPosition);
				}

                float AddHeight = 25.f;

                if ( InHellas()==true )
                {
                    if ( o->Kind==0 || o->Kind==2 )
                    {
                        int PositionX = (int)(o->StartPosition[0]/TERRAIN_SCALE);
                        int PositionY = (int)(o->StartPosition[1]/TERRAIN_SCALE);
                        AddWaterWave( PositionX, PositionY, 2, -1000 );
                    }
                    else
                    {
                        AddHeight = 100.f;
                    }
                }

                o->StartPosition[2] = RequestTerrainHeight( o->StartPosition[0], o->StartPosition[1] ) + AddHeight;
                AddHeight = 3.f;

	            //  중앙에 폭발.
                CreateParticle(BITMAP_EXPLOTION,o->StartPosition,Angle,light,0,0.5f);

                if ( o->Kind==0 )
                {
                    //  쇠 파편.
#ifdef _VS2008PORTING
                    for(int j=0;j<8;j++)
#else // _VS2008PORTING
                    for(j=0;j<8;j++)
#endif // _VS2008PORTING
                    {
                        Vector((float)(rand()%60-60.f),0.f,(float)(rand()%30+90),Angle);
                        VectorAdd(Angle,o->Angle,Angle);
                        VectorCopy(o->StartPosition,Position);
                        Position[0] += rand()%20-10;
                        Position[1] += rand()%20-10;
                        CreateJoint(BITMAP_JOINT_SPARK,Position,Position,Angle);
                        if(j==0) CreateParticle(BITMAP_SPARK,Position,Angle,Light);
                    }
                }
                Vector( 0.f, 0.f, 0.f, Angle);

                //  
                if ( o->Kind==0 )
                    CreateEffect(MODEL_WAVE,o->StartPosition,Angle,o->Light);

                //  자신의 위치에서의 균열.
                o->StartPosition[2] -= 27;
				

				//파란색
				if(o->Owner != NULL)
				{
					if(o->Owner->Type != MODEL_MONSTER01+95)
					{
						CreateEffect(MODEL_SKILL_FURY_STRIKE+3,o->StartPosition,Angle,o->Light,0,o,scale);  //  큰 균열.
						CreateEffect(MODEL_SKILL_FURY_STRIKE+1,o->StartPosition,Angle,o->Light,0,o,scale);  //  큰 균열.
						CreateEffect(MODEL_SKILL_FURY_STRIKE+2,o->StartPosition,Angle,o->Light,0,o,scale);  //  큰 균열.
					}
				}

                //  다섯 테두리.
                for ( int i=0; i<5; ++i )
                {
                    Vector(0.f,(float)(rand()%150+100),0.f,p);
					Vector(0.f,0.f,(float)(o->SubType+(i*72.f)),Angle);
					AngleMatrix(Angle,Matrix);
					VectorRotate(p,Matrix,Position);
					VectorAdd(Position,o->StartPosition,Position);

                    Position[2] += 3;
                    scale = rand()%50+40;

                    Vector ( 0.f, 0.f, 45+(float)(rand()%30-15), Angle );

					TargetX = (int)(Position[0]/TERRAIN_SCALE);
					TargetY = (int)(Position[1]/TERRAIN_SCALE);

                    WORD wall = TerrainWall[TERRAIN_INDEX(TargetX,TargetY)];

                    if ( (wall&TW_NOMOVE)!=TW_NOMOVE && (wall&TW_NOGROUND)!=TW_NOGROUND && (wall&TW_WATER)!=TW_WATER )
                    {
                        if ( InHellas()==true )
                        {
                            AddHeight = 100.f;
                        }
                        Position[2] = RequestTerrainHeight( Position[0], Position[1] ) + AddHeight;

						CreateEffect(MODEL_SKILL_FURY_STRIKE+4,Position,Angle,o->Light,0,o->Owner,scale);
					    CreateEffect(MODEL_SKILL_FURY_STRIKE+5,Position,Angle,o->Light,0,o->Owner,scale);
                    }
                }
            }
            else if ( o->LifeTime==10 )  //  연속적인 균열들.
            {
                for ( int j=0; j<5; ++j )
                {
                    VectorCopy ( o->StartPosition, Pos[j] );
                    ang[j] =0.f;
                }

                int count = 0;
                int random;
#ifdef _VS2008PORTING
                for ( int j=0; j<4; ++j )
#else // _VS2008PORTING
                for ( j=0; j<4; ++j )
#endif // _VS2008PORTING
                {
                    Vector(0.f,rand()%15+85.f,0.f,p);

                    if ( j>=3 ) count = rand();

                    for ( int i=0; i<5; ++i )
                    {
                        if ( (count%2)==0 ) random = rand()%30+50;
                        else                random = -(rand()%30+50);

                        ang[i] += random;
                        Angle[2] = ang[i]+(i*(rand()%10+62));

                        AngleMatrix(Angle,Matrix);
                        VectorRotate(p,Matrix,Position);
						VectorAdd(Position,Pos[i],Pos[i]);

						TargetX = (int)(Pos[i][0]/TERRAIN_SCALE);
						TargetY = (int)(Pos[i][1]/TERRAIN_SCALE);

						WORD wall = TerrainWall[TERRAIN_INDEX(TargetX,TargetY)];

						if ( (wall&TW_NOMOVE)!=TW_NOMOVE || (wall&TW_NOGROUND)!=TW_NOGROUND || (wall&TW_WATER)!=TW_WATER )
                        {
                            if ( InHellas()==true )
                                Pos[i][2] = RequestTerrainHeight( Pos[i][0], Pos[i][1] ) + 100.f;
                            else
                                Pos[i][2] = RequestTerrainHeight( Pos[i][0], Pos[i][1] ) + 3;
                            Angle[2] += 270.f;

                            CreateEffect(MODEL_SKILL_FURY_STRIKE+7,Pos[i],Angle,o->Light,0,o->Owner,100);
                            CreateEffect(MODEL_SKILL_FURY_STRIKE+8,Pos[i],Angle,o->Light,0,o->Owner,100);
                        }
                    }

                    count++;
                }
                o->LifeTime = 0;

     			PlayBuffer(SOUND_FURY_STRIKE3);
            }
            else
            {
                float   count = (float)o->LifeTime;
				float	addAngle = 15.f;
                if ( o->LifeTime>9 && o->LifeTime<16 )
                {
                    count = 12.5;
					addAngle = 18.f;
                    if ( o->LifeTime==15 ) o->Gravity *= -1;
                }

                if ( o->Kind==0 )
                {
                    if ( o->LifeTime==13 )
                    {
                        vec3_t  pos;
                        Vector( 0.f, 0.f, 0.f, Angle);
                        Vector(-25.f,-40.f,0.f,p);
                        AngleMatrix(o->Owner->Angle,Matrix);
                        VectorRotate(p,Matrix,Position);
                        VectorAdd(Position,o->Position,pos);
                        
                        vec3_t  position;
                        //                            pos[2] -= 50.f;
                        for ( int i=0; i<4; ++i )
                        {
                            position[0] = pos[0]; position[1] = pos[1];
                            position[2] = pos[2]-(i*50);
                            CreateEffect(MODEL_TAIL,position,Angle,o->Light);
                        }
                        pos[0] += rand()%30+20;
                        pos[2] += rand()%500-250;
#ifdef _VS2008PORTING
                        for ( int i=0; i<4; ++i )
#else // _VS2008PORTING
                        for ( i=0; i<4; ++i )
#endif // _VS2008PORTING
                        {
                            position[0] = pos[0]; position[1] = pos[1];
                            position[2] = pos[2]-(i*30);
                            CreateEffect(MODEL_TAIL,position,Angle,o->Light);
                        }
                        PlayBuffer(SOUND_FURY_STRIKE2);
                    }
                }

                float   angle = (20-count)*addAngle;

                o->Angle[0] += 80;
				o->Direction[1] = sinf((float)((angle*3.141592)/180.f))*260;
				VectorCopy(o->StartPosition,o->Position);
                if ( count<12.5 || count>12.5 )
                {
                    o->Gravity += 8.f;
                }
                else
                {
                    o->Gravity -= 8.f;
                }

				AngleMatrix(o->HeadAngle,Matrix);
				VectorRotate(o->Direction,Matrix,Position);
				VectorAdd(Position,o->StartPosition,o->Position);

                o->Position[2] += o->Gravity;
				o->Position[2] += 200;
            }
        }
        break;
    case MODEL_SKILL_FURY_STRIKE+1:
     	o->BlendMeshLight = (o->LifeTime*0.1f)/3.f;
        if ( o->LifeTime<10 )
        {
            o->Position[2] -= 0.5f;
        }
        if(o->LifeTime>15 && (o->LifeTime%3)==0)
        {
            EarthQuake = (float)(rand()%8-4)*0.1f;
        }
        break;
    case MODEL_SKILL_FURY_STRIKE+3:
      	o->BlendMeshLight = (o->LifeTime*0.1f)/10.f;
        if ( o->LifeTime<13 )
        {
            o->Position[2] -= 0.5f;
        }
        break;
    case MODEL_SKILL_FURY_STRIKE+2:
        if(o->Scale>50)o->LifeTime = 0;
		if(o->LifeTime >= 10)
       		o->BlendMeshLight = ((20-o->LifeTime)*0.1f);
		else
        {
            if ( o->LifeTime<5 )
            {
                o->Position[2] -= 0.5f;
            }
       		o->BlendMeshLight = (o->LifeTime*0.1f);

      		if(o->LifeTime>=5 && rand()%10==0)
			{
				vec3_t p,Position;
				vec3_t Angle;
				float Matrix[3][4];
				Vector(0.f,(float)(rand()%150),0.f,p);
				Vector(0.f,0.f,(float)(rand()%360),Angle);
				AngleMatrix(Angle,Matrix);
				VectorRotate(p,Matrix,Position);
				VectorAdd(Position,o->Position,Position);
				
      	     	CreateEffect(MODEL_STONE1+rand()%2,Position,o->Angle,o->Light,0);
			}
        }

        o->BlendMeshTexCoordU = -(float)o->LifeTime*0.01f;
				//파란색
				if(o->Owner != NULL)
					if(o->Owner->Type == MODEL_MONSTER01+95)
					{
						Vector(Luminosity*0.0f,Luminosity*0.0f,Luminosity*1.0f,Light);
					}
					else
		Vector(Luminosity*1.0f,Luminosity*0.0f,Luminosity*0.0f,Light);
		AddTerrainLight(o->Position[0],o->Position[1],Light,1,PrimaryTerrainLight);
        break;
    case MODEL_SKILL_FURY_STRIKE+4:
     	o->BlendMeshLight = (o->LifeTime*0.1f)/3.f;
        if ( o->LifeTime<10 )
        {
            o->Position[2] -= 0.5f;
        }
        break;
    case MODEL_SKILL_FURY_STRIKE+6:
      	o->BlendMeshLight = (o->LifeTime*0.1f)/10.f;
        if ( o->LifeTime<13 )
        {
            o->Position[2] -= 0.5f;
        }
        break;
    case MODEL_SKILL_FURY_STRIKE+5:
		if(o->LifeTime >= 30)
       		o->BlendMeshLight = ((40-o->LifeTime)*0.1f);
		else
        {
            if ( o->LifeTime<15 )
            {
                o->Position[2] -= 0.5f;
            }

       		o->BlendMeshLight = (o->LifeTime*0.1f);

      		if(o->LifeTime>=5 && rand()%15==0)
			{
				vec3_t p,Position;
				vec3_t Angle;
				float Matrix[3][4];
				Vector(0.f,(float)(rand()%150),0.f,p);
				Vector(0.f,0.f,(float)(rand()%360),Angle);
				AngleMatrix(Angle,Matrix);
				VectorRotate(p,Matrix,Position);
				VectorAdd(Position,o->Position,Position);
				
      	     	CreateEffect(MODEL_STONE1+rand()%2,Position,o->Angle,o->Light,0);
			}
        }

        o->BlendMeshTexCoordU = -(float)o->LifeTime*0.01f;

				//파란색
				if(o->Owner != NULL)
					if(o->Owner->Type == MODEL_MONSTER01+95)
					{
						Vector(Luminosity*0.0f,Luminosity*0.0f,Luminosity*1.0f,Light);
					}
					else

		Vector(Luminosity*1.0f,Luminosity*0.0f,Luminosity*0.0f,Light);
		AddTerrainLight(o->Position[0],o->Position[1],Light,1,PrimaryTerrainLight);
        break;
    case MODEL_SKILL_FURY_STRIKE+7:
     	o->BlendMeshLight = (o->LifeTime*0.1f)/3.f;
        if ( o->LifeTime<10 )
        {
            o->Position[2] -= 0.5f;
        }
        break;
    case MODEL_SKILL_FURY_STRIKE+8:
		if(o->LifeTime >= 30)
       		o->BlendMeshLight = ((40-o->LifeTime)*0.1f);
		else
        {
       		o->BlendMeshLight = (o->LifeTime*0.1f);
        }
        if ( o->LifeTime<15 )
        {
            o->Position[2] -= 0.5f;
        }

      	if(o->LifeTime>=13 && rand()%5==0)
		{
			vec3_t Position;

            Position[0] =  o->Position[0];
            Position[1] =  o->Position[1];
            Position[2] =  o->Position[2] + 10;
		}

        o->BlendMeshTexCoordU = -(float)o->LifeTime*0.01f;

				//파란색
				if(o->Owner != NULL)
					if(o->Owner->Type == MODEL_MONSTER01+95)
					{
						Vector(Luminosity*0.0f,Luminosity*0.0f,Luminosity*1.0f,Light);
					}
					else

		Vector(Luminosity*1.0f,Luminosity*0.0f,Luminosity*0.0f,Light);
		AddTerrainLight(o->Position[0],o->Position[1],Light,1,PrimaryTerrainLight);
        break;

	case MODEL_BALGAS_SKILL:
//		o->Scale += 0.1f;
    	o->BlendMeshLight = o->LifeTime*0.1f;
//        o->BlendMeshTexCoordU = -(float)o->LifeTime*0.2f;
		Vector(Luminosity*0.3f,Luminosity*0.6f,Luminosity,Light);
		{
            BMD *b = &Models[o->Type];
            b->CurrentAction = o->CurrentAction;
            b->PlayAnimation ( &o->AnimationFrame, &o->PriorAnimationFrame, &o->PriorAction, o->Velocity*2.0f, o->Position, o->Angle );
			AddTerrainLight(o->Position[0],o->Position[1],Light,3,PrimaryTerrainLight);
		}
		break;
	case MODEL_CHANGE_UP_EFF:
		{
			vec3_t  Loc;
			int STwo = 2;
			if(rand()%STwo == 0)
			{
//               Vector(0.f,0.f,(float)(rand()%360),o->Angle);

#ifdef YDG_ADD_DOPPELGANGER_MONSTER
				if(o->SubType == 2)
				{
					VectorCopy(o->Owner->Position, o->Position);
					o->Position[2] = RequestTerrainHeight(o->Position[0], o->Position[1]) + 22.0f;
				}
#endif	// YDG_ADD_DOPPELGANGER_MONSTER

                Loc[0] = o->Position[0] + rand()%200-100;
                Loc[1] = o->Position[1] + rand()%200-100;
                Loc[2] = o->Position[2] - 200;
				if(o->SubType == 0
#ifdef YDG_ADD_DOPPELGANGER_MONSTER
					|| o->SubType == 2
#endif	// YDG_ADD_DOPPELGANGER_MONSTER
					)
				{
					CreateJoint(BITMAP_FLARE,Loc,Loc,o->Angle,50,NULL,40);
				}
//				else
//				if(o->SubType == 1)
//					CreateJoint(BITMAP_FLARE,Loc,Loc,o->Angle,25,o,50.f);
					//CreateJoint(BITMAP_FLARE,Loc,Loc,o->Angle,51,NULL,30);

				if(o->LifeTime > 40)
					CreateEffect(BITMAP_MAGIC,o->Position,o->Angle,Light,4,o,4.0f);

			}
			if(o->LifeTime <= 1 && o->SubType == 0)
				SetPlayerStop(Hero);

			if(o->SubType == 0)
			{
				BMD *b = &Models[o->Type];
				b->PlayAnimation( &o->AnimationFrame, &o->PriorAnimationFrame, &o->PriorAction, o->Velocity/3.f, o->Position, o->Angle );
			}
#ifdef YDG_ADD_DOPPELGANGER_MONSTER
			else if(o->SubType == 2)
			{
				BMD *b = &Models[o->Type];
				b->PlayAnimation( &o->AnimationFrame, &o->PriorAnimationFrame, &o->PriorAction, o->Velocity/3.f, o->Position, o->Angle );
			}
#endif	// YDG_ADD_DOPPELGANGER_MONSTER
		}
		break;
	case MODEL_CHANGE_UP_NASA:
		{
			if(o->SubType == 0)
			{
				if(o->LifeTime == 100)
				{
					CreateEffect(MODEL_CHANGE_UP_NASA,o->Position,o->Angle,o->Light,1,o);
				}
				else
				if(o->LifeTime == 70)
				{
					CreateEffect(MODEL_CHANGE_UP_NASA,o->Position,o->Angle,o->Light,2,o);
				}
				if(o->LifeTime == 40)
				{
					CreateEffect(MODEL_CHANGE_UP_NASA,o->Position,o->Angle,o->Light,3,o);
				}
			}
			if(o->SubType >= 1 && o->SubType <= 3)
			{
				o->Scale += 0.02f;
				o->BlendMeshLight = o->LifeTime*0.005f;
			}
			else
			if(o->SubType == 0)
				o->BlendMeshLight = o->LifeTime*0.01f;
		}
		break;
	case MODEL_CHANGE_UP_CYLINDER:
		{
#ifdef YDG_ADD_DOPPELGANGER_MONSTER
			if(o->SubType == 2)
			{
				VectorCopy(o->Owner->Position, o->Position);
				o->Position[2] = RequestTerrainHeight(o->Position[0], o->Position[1]);
			}
#endif	// YDG_ADD_DOPPELGANGER_MONSTER

			vec3_t  Go;
			AngleMatrix ( Angle, Matrix );
			VectorRotate ( Go, Matrix, Position );
			VectorAdd ( Position, o->Position, Position );

			if(o->SubType == 1)
			{
				o->BlendMeshLight = o->LifeTime*0.015f;
				o->Scale += 0.08f;
			}
//			Vector(Luminosity*0.3f,Luminosity*0.6f,Luminosity,Light);
//			AddTerrainLight(Position[0],Position[1],Light,3,PrimaryTerrainLight);
		}
		break;
	case MODEL_DARK_ELF_SKILL:
		{
			vec3_t  Go;
			if(o->Scale < 0.8f)
				o->Scale += 0.2f;
			else
				o->Scale -= 0.4f;
    		o->BlendMeshLight = o->LifeTime*0.1f;
			o->BlendMeshTexCoordU = -(float)o->LifeTime*0.2f;
			Vector(0.0f, -500.f, 0.0f, Go);
			vec3_t	 	  Angle;
			VectorCopy(o->Angle, Angle);
			Angle[2] += 7;

			AngleMatrix ( Angle, Matrix );
			VectorRotate ( Go, Matrix, Position );
			VectorAdd ( Position, o->Position, Position );

#ifdef _VS2008PORTING
			for(int j=0;j<6;j++)
#else // _VS2008PORTING
			for(j=0;j<6;j++)
#endif // _VS2008PORTING
			{
      			CreateParticle(BITMAP_SMOKE,Position,Angle,o->Light,25);
			}
			Vector(Luminosity*0.3f,Luminosity*0.6f,Luminosity,Light);
			AddTerrainLight(Position[0],Position[1],Light,3,PrimaryTerrainLight);
		}
		break;

	case MODEL_MAGIC2:
    	o->BlendMeshLight = o->LifeTime*0.1f;
        o->BlendMeshTexCoordU = -(float)o->LifeTime*0.2f;
		//VectorAdd(o->Position,o->Direction,o->Position);
#ifdef _VS2008PORTING
		for(int j=0;j<4;j++)
#else // _VS2008PORTING
		for(j=0;j<4;j++)
#endif // _VS2008PORTING
      		CreateParticle(BITMAP_SMOKE,o->Position,o->Angle,o->Light,3);
		Vector(Luminosity*0.3f,Luminosity*0.6f,Luminosity,Light);
		AddTerrainLight(o->Position[0],o->Position[1],Light,3,PrimaryTerrainLight);

		if ( o->SubType==2 )
		{
			o->HiddenMesh = 0;
			vec3_t Light2;
			VectorCopy ( o->Angle, Angle );
			VectorCopy ( o->Position, Position );
			Angle[2] += rand()%10-5;
			CreateParticle(BITMAP_SMOKE,o->Position,o->Angle, Light, 11, (float)(rand()%32+80)*0.015f );

			Vector ( 1.f, 1.f, 1.f, Light2 );
			Position[2] += 50.f;
			CreateSprite ( BITMAP_SHINY+1, Position, 1.5f, Light2, NULL, (float)(rand()%360) );
			CreateSprite ( BITMAP_SHINY+1, Position, 1.5f, Light2, NULL, (float)(rand()%360) );

			CreateSprite ( BITMAP_LIGHT, Position, 3.5f, Light, NULL, (float)(rand()%360) );
		}
		break;
	case MODEL_STORM:
        switch ( o->SubType )
        {
        case 0:
    	    o->BlendMeshLight = o->LifeTime*0.1f;
            o->BlendMeshTexCoordU = -(float)o->LifeTime*0.1f;
		    //VectorAdd(o->Position,o->Direction,o->Position);
		    CreateParticle(BITMAP_SMOKE,o->Position,o->Angle,o->Light,3);
		    Vector(90.f,0.f,o->Angle[2],Angle);
		    if(rand()%2==0)
		    {
			    Vector(o->Position[0]-200.f,o->Position[1],o->Position[2]+700.f,Position);
			    CreateJoint(BITMAP_JOINT_THUNDER,Position,o->Position,Angle,0,o,10.f);
		    }
		    if(rand()%2==0)
		    {
			    Vector(o->Position[0]+200.f,o->Position[1],o->Position[2]+700.f,Position);
			    CreateJoint(BITMAP_JOINT_THUNDER,Position,o->Position,Angle,0,o,10.f);
		    }
		    o->Position[2] = RequestTerrainHeight(o->Position[0],o->Position[1]);
		    if(rand()%4==0)
   			    CreateEffect(MODEL_STONE1+rand()%2,o->Position,o->Angle,o->Light,2);
		    Vector(-Luminosity*0.4f,-Luminosity*0.3f,-Luminosity*0.2f,Light);
		    AddTerrainLight(o->Position[0],o->Position[1],Light,5,PrimaryTerrainLight);

            if ( battleCastle::IsBattleCastleStart() )
            {
                DWORD att = TERRAIN_ATTRIBUTE ( o->Position[0], o->Position[1] );
                if ( (att&TW_NOATTACKZONE)==TW_NOATTACKZONE )
                {
                    o->Velocity = 0.f;
                    Vector ( 0.f, 0.f, 0.f, o->Direction );
                    o->LifeTime /= 5.f;
                    break;
                }
            }
    	    if(o->LifeTime%15 == 0)
			    if(o->Owner == &Hero->Object)
	     		    AttackCharacterRange(o->Skill,o->Position,150.f,o->Weapon,o->PKKey);//클라이언트마법처리
            break;

        case 1:
    	    o->BlendMeshLight = o->LifeTime*0.01f;
            o->BlendMeshTexCoordU = -(float)o->LifeTime*0.1f;
            o->Angle[2] += rand()%30+30.f;

            VectorCopy ( o->Position, Position );
            Position[2] += 100.f;
            CreateParticle ( BITMAP_SMOKE,  Position, o->Angle, o->Light, 3 );
            CreateParticle ( BITMAP_BUBBLE, Position, o->Angle, o->Light, 3, 0.1f );
		    
		    Vector(-Luminosity*0.1f,-Luminosity*0.3f,-Luminosity*1.f,Light);
		    AddTerrainLight(o->Position[0],o->Position[1],o->Light,5,PrimaryTerrainLight);
            break;

        case 2:
    	    o->BlendMeshLight = o->LifeTime*0.1f;
            o->BlendMeshTexCoordU = -(float)o->LifeTime*0.1f;

            o->Gravity = (float)( rand()%360 );

            VectorCopy ( o->Position, Position );
            Position[2] += 100.f;
            CreateParticle ( BITMAP_SMOKE,  Position, o->Angle, o->Light, 3 );
            CreateParticle ( BITMAP_BUBBLE, Position, o->Angle, o->Light, 3, 0.1f );
		    
		    Vector(-Luminosity*0.1f,-Luminosity*0.3f,-Luminosity*1.f,Light);
		    AddTerrainLight(o->Position[0],o->Position[1],o->Light,5,PrimaryTerrainLight);
            break;
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
			EarthQuake = (float)(rand()%8-4)*0.1f;
			CreateParticle ( BITMAP_SMOKE,  o->Position, o->Angle, o->Light, 28 );
			CreateParticle ( BITMAP_SMOKE,  o->Position, o->Angle, o->Light, 29 );
			CreateParticle ( BITMAP_SMOKE,  o->Position, o->Angle, o->Light, 30 );
			if(rand()%2==0)
				CreateEffect(MODEL_STONE1+rand()%2,o->Position,o->Angle,o->Light,2);
			break;
		case 8:
			{
				o->BlendMeshLight = o->LifeTime*0.1f;
				o->BlendMeshTexCoordU = -(float)o->LifeTime*0.1f;
				
				Vector(-Luminosity*0.1f,-Luminosity*0.3f,-Luminosity*1.f,Light);
				AddTerrainLight(o->Position[0],o->Position[1],o->Light,5,PrimaryTerrainLight);
			}
			break;
        }
		break;
		case MODEL_SUMMON:
			{
				if( o->LifeTime >= 30)
				{
					o->Light[0] *= 1.03f;
					o->Light[1] *= 1.03f;
					o->Light[2] *= 1.03f;
				}
				else
				{
					o->Light[0] /= 1.08f;
					o->Light[1] /= 1.08f;
					o->Light[2] /= 1.08f;
				}
				o->Angle[2] += 1.0f;
			}
			break;
		case MODEL_STORM2:
			{
				if(o->SubType == 0)
				{
					o->Angle[1] += 40.0f;

					if(o->LifeTime <= 5)
					{
						o->Light[0] /= 1.01f;
						o->Light[1] /= 1.01f;
						o->Light[2] /= 1.01f;
					}
				}
				else if(o->SubType == 1)
				{
					vec3_t Pos1, Pos2, Direction;

					float Matrix[3][4];

					for(int i = 0; i < 2; i++)
					{
						Angle[0] = o->Angle[0];
						Angle[1] = o->Angle[1];
						Angle[2] = o->Angle[2]/3.0f + 180.0f*i; 
							
						Vector(0.0f, 200.0f, 0.0f, Direction);
						AngleMatrix(Angle,Matrix);
						VectorRotate(Direction,Matrix,Position);
						VectorAdd(Position, o->Position, Position);
						
						Vector(0.3f, 0.5f, 0.6f, Light);
						CreateParticle(BITMAP_CLUD64, Position, o->Angle, Light, 1, 1.0f);
					}

					Vector(90.f,0.f,0.0f,Angle);

					Pos1[0] = o->Position[0]+rand()%600-300.0f;
					Pos1[1] = o->Position[1]+rand()%600-300.0f;
					Pos1[2] = o->Position[2];

					Pos2[0] = Pos1[0]+rand()%100-50.0f;
					Pos2[1] = Pos1[1]+rand()%100-50.0f;
					Pos2[2] = Pos1[2];

					CreateJoint(BITMAP_JOINT_THUNDER,Pos1,Pos2,Angle,20,o,20.f);

					Pos2[0] = Pos1[0]+rand()%100-50.0f;
					Pos2[1] = Pos1[1]+rand()%100-50.0f;
					Pos2[2] = Pos1[2];
					CreateEffect(MODEL_STONE1+rand()%2,Pos2,o->Angle,Light,2);

					o->Angle[2] += o->Gravity;
					o->Light[0] /= 1.02f;
					o->Light[1] /= 1.02f;
					o->Light[2] /= 1.02f;

					o->Alpha += 0.01f;

					if(o->Light[0] <= 0.05f)
						o->Live = false;
				}
				else if(o->SubType == 2)
				{
					o->Angle[1] += 40.0f;

					if(o->LifeTime <= 5)
					{
						o->Light[0] /= 1.01f;
						o->Light[1] /= 1.01f;
						o->Light[2] /= 1.01f;
					}

					CreateEffect(MODEL_STONE1+rand()%2,o->Position,o->Angle,o->Light,2);
				}
			}
			break;
		case MODEL_STORM3:
			{
				if(o->LifeTime <= 30)
				{
					o->Light[0] /= 1.1f;
					o->Light[1] /= 1.1f;
					o->Light[2] /= 1.1f;
				}
				o->Angle[1] += 30.0f;

				Vector(90.f,0.f,o->Angle[2],Angle);

				for(int i = 0; i < 5; i++)
				{
					if(rand()%2==0)
					{
						vec3_t Pos1, Pos2;

						Pos1[0] = o->StartPosition[0]+rand()%2000-1000.0f;
						Pos1[1] = o->StartPosition[1]+rand()%2000-1000.0f;
						Pos1[2] = o->StartPosition[2];

						Pos2[0] = Pos1[1]+rand()%200-100.0f;
						Pos2[1] = Pos1[1]+rand()%200-100.0f;

						CreateJoint(BITMAP_JOINT_THUNDER,Pos1,Pos2,Angle,20,o,20.f);
					}
				}

			}
			break;
		case MODEL_MAYASTAR:
			{
				if(o->LifeTime <= 20)
				{
					o->Light[0] /= 1.2f;
					o->Light[1] /= 1.2f;
					o->Light[2] /= 1.2f;
				}
				o->Scale += sinf(WorldTime*0.005f)*2.0f;
				o->Angle[1] += 10.0f;
				EarthQuake = (float)(rand()%6-6)*0.5f;
			}
			break;
		case MODEL_MAYASTONE1:
		case MODEL_MAYASTONE2:
		case MODEL_MAYASTONE3:
			{

				bool success = false;

				VectorCopy ( o->HeadAngle, o->Angle );

			    float Height = RequestTerrainHeight(o->Position[0],o->Position[1]);
			    if(o->Position[2] < Height)
                   success = true;

				if ( success )
				{
					o->Position[2] = Height;
					Vector(0.f,0.f,0.f,o->Direction);
					vec3_t Position;
					Vector(o->Position[0],o->Position[1],o->Position[2]+80.f,Position);

					BYTE smokeNum = 15;

					Vector(0.0f,0.6f,1.f,Light);
					Vector(0.f,0.f,0.f,Angle);
					CreateEffect(MODEL_SKILL_INFERNO,o->Position,Angle,Light, 2, o, 30, 0);

					for ( int i=0; i<smokeNum; ++i )
					{
						Position[0] = o->Position[0]+(rand()%160-80);
						Position[1] = o->Position[1]+(rand()%160-100);
						Position[2] = o->Position[2]+50;

    					Vector ( 0.3f, 0.5f, 1.0f, Light );
						CreateParticle(BITMAP_SMOKE,Position,o->Angle, Light, 11, (float)(rand()%32+80)*0.025f );
					}

					for(int j=0;j<6;j++)
						CreateEffect(MODEL_MAYASTONE4+rand()%2,o->Position,o->Angle,Light);

					CreateParticle(BITMAP_EXPLOTION,Position,o->Angle,Light,0, 4.0f);

					o->Live = false;
				}
			}
			break;
		case MODEL_MAYASTONE4:
		case MODEL_MAYASTONE5:
			{
				VectorAdd(o->Position,o->Direction,o->Position);
				VectorScale(o->Direction,0.9f,o->Direction);
				o->Position[2] += o->Gravity;
				o->Gravity -= 3.f;
				Height = RequestTerrainHeight(o->Position[0],o->Position[1]);
				if(o->Position[2] < Height)
				{
					o->Position[2] = Height;
					o->Gravity = -o->Gravity*0.5f;
					o->LifeTime -= 4;
					o->Angle[0] -= o->Scale*128.f;
				}
				else
					o->Angle[0] -= o->Scale*32.f;
				
				if ( rand()%10==0 )
   					CreateParticle(BITMAP_FIRE,o->Position,o->Angle,o->Light,15);
			}
			break;
		case MODEL_MAYASTONEFIRE:
			{
				VectorCopy ( o->HeadAngle, o->Angle );

			    float Height = RequestTerrainHeight(o->Position[0],o->Position[1]);
			    if(o->Position[2] < Height)
                   o->Live = false;
			}
			break;
		case MODEL_MAYAHANDSKILL:
			{
				if(o->SubType == 0)
				{
					o->Light[0] /= 1.17f;
					o->Light[1] /= 1.17f;
					o->Light[2] /= 1.17f;
					
					// 연기 발생
					vec3_t vPos;
					VectorCopy(o->Position, vPos);
					vPos[2] += 100.f;
					CreateParticle(BITMAP_SMOKE, vPos, o->Angle, o->StartPosition, 17, 10.0f);
					
					vec3_t Direction;
					Vector(0.0f, (-80.0f-rand()%30)*o->Scale, 0.0f, Direction);
					AngleMatrix(o->Angle,Matrix);
					VectorRotate(Direction,Matrix,Position);
					VectorAdd(Position, o->Position, o->Position);
					CreateEffect ( MODEL_MAYAHANDSKILL, o->Position, o->Angle, o->StartPosition, 1, NULL, -1, 0, 0, 0, o->Scale+0.2f );
					
					if(o->Light[0] <= 0.05f)
						o->Live = false;
				}
				else if(o->SubType == 1)
				{
					o->Light[0] /= 1.17f;
					o->Light[1] /= 1.17f;
					o->Light[2] /= 1.17f;
					
					// 연기 발생
					vec3_t vPos;
					VectorCopy(o->Position, vPos);
					vPos[2] += 100.f;
					CreateParticle(BITMAP_SMOKE, vPos, o->Angle, o->StartPosition, 17, 10.0f);
				}
			}
			break;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	case MODEL_CIRCLE:
      	o->BlendMeshLight = o->LifeTime*0.1f;
		if ( o->SubType==1 )
		{
			if ( o->LifeTime>(44-o->Owner->m_bySkillCount) )
			{
                vec3_t Angle = { 0.0f, 0.0f, 0.0f};
			    int iCount = 36;
			    for ( int i = 0; i < iCount; ++i)
			    {
				    Angle[0] = -10.f;
				    Angle[1] = 0.f;
				    Angle[2] = i*(10.f+rand()%10);
				    vec3_t Position;
				    VectorCopy( o->Position, Position);
				    Position[2] += 100.f;
				    CreateJoint(BITMAP_JOINT_SPIRIT,Position,Position,Angle,6,o,60.f,0,0);

					if ( o->LifeTime==(44-o->Owner->m_bySkillCount+1) )
					{
						Angle[2] = i*10.f;
					    CreateJoint(BITMAP_JOINT_SPIRIT,Position,Position,Angle,7,o,60.f,0,0);
					}
			    }
			}
		}
        else if ( o->SubType==2 )
        {
            if ( o->LifeTime>240 )
            {
                o->BlendMeshLight = (250-o->LifeTime)*0.1f;
            }
            else
            {
                o->BlendMeshLight = o->LifeTime*0.1f;
            }
        }
        else if ( o->SubType==3 )
        {
            if ( o->LifeTime>10 )
            {
                o->BlendMeshLight = (30-o->LifeTime)*0.1f;
            }
            else
            {
                o->BlendMeshLight = o->LifeTime*0.1f;
            }
        }
		else if(o->SubType == 4)
		{
			if ( o->LifeTime>(44-o->Owner->m_bySkillCount) )
			{
                vec3_t Angle, Position;
			    for ( int i = 0; i < 36; ++i)
			    {
				    Angle[0] = -10.f;
				    Angle[1] = 0.f;
				    Angle[2] = i*10.f;

				    VectorCopy( o->Position, Position);
				    Position[2] += 100.f;
				    CreateJoint(BITMAP_JOINT_SPIRIT,Position,Position,Angle,22,o,2.0f,0,0);
				    CreateJoint(BITMAP_JOINT_SPIRIT,Position,Position,Angle,23,o,1.75f,0,0);
			    }
			}			
		}
		break;
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	case MODEL_CIRCLE_LIGHT:
        if ( o->SubType!=3 && o->SubType!=4 )
		{
			int value = 4;

			if(o->LifeTime >= 30)
       			o->BlendMeshLight = (40-o->LifeTime)*0.1f;
			else
       			o->BlendMeshLight = o->LifeTime*0.1f;
			o->BlendMeshTexCoordU = -(float)o->LifeTime*0.01f;
			EarthQuake = (float)(rand()%6-6)*0.1f;

			if(rand()%value==0)
			{
				vec3_t p,Position;
				vec3_t Angle;
				float Matrix[3][4];
				Vector(0.f,(float)(rand()%300),0.f,p);
				Vector(0.f,0.f,(float)(rand()%360),Angle);
				AngleMatrix(Angle,Matrix);
				VectorRotate(p,Matrix,Position);
				VectorAdd(Position,o->Position,Position);
  	     		CreateEffect(MODEL_STONE1+rand()%2,Position,o->Angle,o->Light,1);
			}
		    Vector(Luminosity*1.f,Luminosity*0.8f,Luminosity*0.2f,Light);
		    AddTerrainLight(o->Position[0],o->Position[1],Light,4,PrimaryTerrainLight);
		}
        else if ( o->SubType==3 )
        {
			int value = 2;

			if(o->LifeTime >= 240)
       			o->BlendMeshLight = (250-o->LifeTime)*0.1f;
			else
       			o->BlendMeshLight = o->LifeTime*0.1f;

            o->BlendMeshLight = min ( 0.5f, o->BlendMeshLight );
			o->BlendMeshTexCoordU = -(float)o->LifeTime*0.01f;
			if ( o->LifeTime>30 && rand()%value==0 )
			{
				vec3_t p,Position;
				vec3_t Angle;
				float Matrix[3][4];
				Vector(0.f,(float)(rand()%200),0.f,p);
				Vector(0.f,0.f,(float)(rand()%360),Angle);
				AngleMatrix(Angle,Matrix);
				VectorRotate(p,Matrix,Position);
				VectorAdd(Position,o->Position,Position);                           

                CreateParticle ( BITMAP_FLARE_BLUE, Position, o->Angle, o->Light, 0 );
                if ( o->LifeTime>40 )
                {
                    Position[2] += 600.f;
                    Angle[2] = 45.f;
                    CreateJoint ( BITMAP_FLARE_BLUE, Position, Position, Angle, 19, NULL, 40 );
                }
			}
		    Vector ( o->BlendMeshLight, o->BlendMeshLight, o->BlendMeshLight, Light );
		    AddTerrainLight ( o->Position[0], o->Position[1], Light, 4, PrimaryTerrainLight );
        }
        else if ( o->SubType==4 )
        {
			int value = 5;

			if(o->LifeTime >= 10 )
       			o->BlendMeshLight = (20-o->LifeTime)*0.1f;
			else
       			o->BlendMeshLight = o->LifeTime*0.1f;

            o->BlendMeshLight = min ( 0.5f, o->BlendMeshLight );
			o->BlendMeshTexCoordU = -(float)o->LifeTime*0.01f;
			if ( rand()%value==0 && o->LifeTime>5 )
			{
				vec3_t p,Position;
				vec3_t Angle;
				float Matrix[3][4];
				Vector(0.f,(float)(rand()%100),0.f,p);
				Vector(0.f,0.f,(float)(rand()%360),Angle);
				AngleMatrix(Angle,Matrix);
				VectorRotate(p,Matrix,Position);
				VectorAdd(Position,o->Position,Position);                           

                CreateParticle ( BITMAP_FLARE_BLUE, Position, o->Angle, o->Light, 0 );
                Position[2] += 600.f;
                Angle[2] = 45.f;
                CreateJoint ( BITMAP_FLARE_BLUE, Position, Position, Angle, 19, NULL, 40 );
			}
		    Vector ( o->BlendMeshLight, o->BlendMeshLight, o->BlendMeshLight, Light );
		    AddTerrainLight ( o->Position[0], o->Position[1], Light, 4, PrimaryTerrainLight );
        }
		break;
	case MODEL_SNOW2:
	case MODEL_SNOW3:
	case MODEL_STONE1:
	case MODEL_STONE2:
        if(o->SubType==5)
        {
            VectorAdd( o->Owner->Position, o->StartPosition, o->Position );
            break;
        }
        else if( o->SubType==11 ) 
        {
            if ( o->LifeTime<10 )
            {
                o->Alpha *= 0.8f;
            }
            break;
        }
		else if (o->SubType == 13
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
			|| o->SubType == 14
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
			)
		{
			o->HeadAngle[2] -= o->Gravity;

			o->Position[0] += o->HeadAngle[0];
			o->Position[1] += o->HeadAngle[1];
			o->Position[2] += o->HeadAngle[2];
			
			Height = RequestTerrainHeight(o->Position[0],o->Position[1]);
			o->Angle[0] += 0.5f * o->LifeTime;
			o->Angle[1] += 0.5f * o->LifeTime;
			if (o->Position[2] + o->Direction[2] <= Height)
			{
				o->Position[2] = Height;
				o->HeadAngle[0] *= 0.6f;
				o->HeadAngle[1] *= 0.6f;
				o->HeadAngle[2] += 1.0f*o->LifeTime;
				if (o->HeadAngle[2] < 0.5f)
					o->HeadAngle[2] = 0;

				o->Alpha -= 0.1f;
			}

// 			if (rand()%3==0)
// 			{
// 				CreateParticle(BITMAP_FIRE_CURSEDLICH, o->Position, o->Angle, o->Light, 0, 1, o);
// 			}
			break;
		}
	case MODEL_BONE1:
	case MODEL_BONE2:
	case MODEL_BIG_STONE1:
	case MODEL_BIG_STONE2:
        if(o->SubType==5)
        {
            if(o->Owner->Live==0) o->Live = 0;
            VectorAdd( o->Owner->Position, o->StartPosition, o->Position );
            break;
        }
	case MODEL_ICE_SMALL:
	case MODEL_METEO1:
	case MODEL_METEO2:
	case MODEL_BOSS_ATTACK:
	case MODEL_EFFECT_SAPITRES_ATTACK_2:			// 사피트레스 가시 사방으로 날라가기
		if ( o->SubType==0 || o->SubType==10 || o->SubType==12)
		{
			VectorAdd(o->Position,o->Direction,o->Position);
		}
		else if( o->Type == MODEL_EFFECT_SAPITRES_ATTACK_2 && o->SubType == 14 )
		{
			float fMoveSpeed = (float)(rand()%5+25);
			o->Position[0] -= (o->Direction[0] * fMoveSpeed );
			o->Position[1] -= (o->Direction[1] * fMoveSpeed );
			o->Position[2] -= (o->Direction[2] * fMoveSpeed );

			if( o->LifeTime <= 10 )
			{
				o->BlendMeshLight -= 0.1f;
			}
		}
		else
		{
			AngleMatrix(o->Angle,Matrix);
			VectorRotate(o->Direction,Matrix,Position);
			VectorAdd(o->Position,Position,o->Position);
		}
		VectorScale(o->Direction,0.9f,o->Direction);
		o->Position[2] += o->Gravity;
		if ( o->SubType == 0 || o->SubType==10 || o->SubType==12 
#ifdef PJH_GIANT_MAMUD
			|| o->SubType == 13
#endif //#ifdef PJH_GIANT_MAMUD
			)
		{
			o->Gravity -= 3.f;
			Height = RequestTerrainHeight(o->Position[0],o->Position[1]);
			if(o->Position[2] < Height)
			{
				o->Position[2] = Height;
				o->Gravity = -o->Gravity*0.5f;
				o->LifeTime -= 4;
				o->Angle[0] -= o->Scale*128.f;
			}
			else
				o->Angle[0] -= o->Scale*32.f;
			
            if ( (o->SubType==0 || o->SubType==12
#ifdef PJH_GIANT_MAMUD
			|| o->SubType == 13
#endif //#ifdef PJH_GIANT_MAMUD
				)&& rand()%10==0 )
			{
				if(o->Type==MODEL_ICE_SMALL || o->Type==MODEL_METEO1 || o->Type==MODEL_METEO2 )
      				CreateParticle(BITMAP_SMOKE,o->Position,o->Angle,o->Light);
				else if(o->Type==MODEL_STONE1 || o->Type==MODEL_STONE2)
				{
      				CreateParticle(BITMAP_FIRE,o->Position,o->Angle,o->Light,1+rand()%3);
				}
			}
		}
		else if(o->SubType == 1)
			o->Gravity += 0.5f;
		else if( o->Type != MODEL_EFFECT_SAPITRES_ATTACK_2)
		{
			o->Angle[2] += 20.f;
			o->Gravity += 0.5f;
		}
		break;
#ifdef ADD_RAKLION_IRON_KNIGHT
	case MODEL_EFFECT_BROKEN_ICE0:
	case MODEL_EFFECT_BROKEN_ICE1:
	case MODEL_EFFECT_BROKEN_ICE2:
	case MODEL_EFFECT_BROKEN_ICE3:
#ifdef ASG_ADD_KARUTAN_MONSTERS
	case MODEL_CONDRA_STONE:
	case MODEL_CONDRA_STONE1:
	case MODEL_CONDRA_STONE2:
	case MODEL_CONDRA_STONE3:
	case MODEL_CONDRA_STONE4:
	case MODEL_CONDRA_STONE5:
	case MODEL_NARCONDRA_STONE:
	case MODEL_NARCONDRA_STONE1:
	case MODEL_NARCONDRA_STONE2:
	case MODEL_NARCONDRA_STONE3:
#endif	// ASG_ADD_KARUTAN_MONSTERS
		if (o->SubType == 0)	// 얼음 파편
		{
			o->HeadAngle[2] -= o->Gravity;

			o->Position[0] += o->HeadAngle[0];
			o->Position[1] += o->HeadAngle[1];
			o->Position[2] += o->HeadAngle[2];
			
			Height = RequestTerrainHeight(o->Position[0],o->Position[1]);
			o->Angle[0] += 0.5f * o->LifeTime;
			o->Angle[1] += 0.5f * o->LifeTime;
			if (o->Position[2] + o->Direction[2] <= Height)
			{
				o->Position[2] = Height;
				o->HeadAngle[0] *= 0.6f;
				o->HeadAngle[1] *= 0.6f;
				o->HeadAngle[2] += 1.0f*o->LifeTime;
				if (o->HeadAngle[2] < 0.5f)
					o->HeadAngle[2] = 0;

				o->Alpha -= 0.1f;
			}
			
			// 연기 뿌리기
			if (rand()%30 == 0)
				CreateParticle(BITMAP_SMOKE,o->Position,o->Angle,o->Light);
		}
		else if(o->SubType == 1)
		{
			bool bSuccess = false;
			
			VectorCopy ( o->HeadAngle, o->Angle );
			
			float fHeight = RequestTerrainHeight(o->Position[0], o->Position[1]);
			if(o->Position[2] < fHeight)
			{
				bSuccess = true;
			}
			
			if ( bSuccess )
			{
				o->Position[2] = fHeight;
				Vector(0.f,0.f,0.f,o->Direction);
				vec3_t vPos;
				Vector(o->Position[0],o->Position[1],o->Position[2]+80.f,vPos);
				
				Vector(0.0f,0.6f,1.f,Light);
				Vector(0.f,0.f,0.f,Angle);
				// 스케일 안먹음
				CreateEffect(MODEL_SKILL_INFERNO, o->Position, Angle, Light, 2, o);
				
				Vector(0.2f, 0.4f, 0.8f, Light);
				for(int i=0; i<8; ++i)
				{
					vPos[0] = o->Position[0] + (rand()%80-40);
					vPos[1] = o->Position[1] + (rand()%80-40);
					vPos[2] = o->Position[2] + 50;
					
					CreateParticle(BITMAP_SMOKE, vPos,o->Angle, Light, 11, (float)(rand()%40+60)*0.025f );
				}
				
				for(int j=0;j<6;j++)
				{
					CreateEffect(MODEL_EFFECT_BROKEN_ICE0+rand()%3,vPos,o->Angle,Light,0);
				}
				
				Vector(0.6f, 0.6f, 1.f, Light);
				vPos[0] = o->Position[0];
				vPos[1] = o->Position[1];
				vPos[2] = o->Position[2] + 100;
				CreateParticle(BITMAP_EXPLOTION_MONO,vPos,o->Angle,Light, 1, 1.5f);

				o->Live = false;
			}
		}
		else if(o->SubType == 2)
		{
			o->Position[2] -= o->Gravity;

			float fHeight = RequestTerrainHeight(o->Position[0], o->Position[1]);
			if(o->Position[2] < fHeight)
			{
				CreateEffect(o->Type, o->Position, o->Angle, o->Light, 0);
				
				o->Live = false;
			}
		}
		break;
#endif	// ADD_RAKLION_IRON_KNIGHT
	case MODEL_SNOW1:
		o->Direction[2] -= 2.5f;
        CheckTargetRange(o);
		break;
	case MODEL_WOOSISTONE:
		if (o->SubType == 1)	// 돌 파편
		{
			o->HeadAngle[2] -= o->Gravity;

			o->Position[0] += o->HeadAngle[0];
			o->Position[1] += o->HeadAngle[1];
			o->Position[2] += o->HeadAngle[2];
			
			Height = RequestTerrainHeight(o->Position[0],o->Position[1]);
			o->Angle[0] += 0.5f * o->LifeTime;
			o->Angle[1] += 0.5f * o->LifeTime;
			if (o->Position[2] + o->Direction[2] <= Height)
			{
				o->Position[2] = Height;
				o->HeadAngle[0] *= 0.6f;
				o->HeadAngle[1] *= 0.6f;
				o->HeadAngle[2] += 1.0f*o->LifeTime;
				if (o->HeadAngle[2] < 0.5f)
					o->HeadAngle[2] = 0;

				o->Alpha -= 0.1f;
			}
		}
		else	// 돌 덩이
		{
			o->Direction[2] -= 2.0f;
		    CheckTargetRange(o);
		}
		break;
	case MODEL_SKULL:
        if ( o->SubType==1 && o->Owner!=NULL )
        {
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
            float dx = o->Position[0]-o->StartPosition[0];
            float dy = o->Position[1]-o->StartPosition[1];
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX

            if ( (o->LifeTime%10)==0 )
            {
		        VectorCopy ( o->Position, o->m_vDeadPosition );
            }

		    VectorCopy ( o->Owner->Position, o->StartPosition );
		    o->StartPosition[2] += 200.f;

#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
		    float Distance = sqrtf(dx*dx+dy*dy);
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
			MoveHumming(o->Position,o->Angle,o->StartPosition,10.f);

		    o->HeadAngle[0] += (float)(rand()%32-16)*0.2f;
		    o->HeadAngle[2] += (float)(rand()%32-16)*0.8f;
		    o->Angle[0] += o->HeadAngle[0];
		    o->Angle[2] += o->HeadAngle[2];
		    o->HeadAngle[0] *= 0.6f;
		    o->HeadAngle[2] *= 0.8f;
		    
            if ( o->LifeTime<10 )
            {
                o->Alpha /= 1.1f;
                o->BlendMeshLight /= 1.2f;

                if ( o->Angle[0]<=80 )
                {
                    o->Angle[0] += 5.f;
                }
            }
            else
            {
		        Height = RequestTerrainHeight(o->Position[0],o->Position[1]);
		        if(o->Position[2] < Height+100.f)
		        {
			        o->HeadAngle[0] = 0.f;
			        o->Angle[0] = -5.f;
		        }
		        if(o->Position[2] > Height+400.f)
		        {
			        o->HeadAngle[0] = 0.f;
			        o->Angle[0] = 5.f;
		        }
            }

            //  꼬리에 불나게 한다.
            CreateParticle ( BITMAP_FIRE, o->Position, o->Angle, Light, 5, 1.2f );

            Vector(Luminosity,Luminosity*0.3f,Luminosity*0.1f,Light);   //  붉은색.
            AddTerrainLight(o->Position[0],o->Position[1],Light,4,PrimaryTerrainLight);
        }
		break;
	case MODEL_CUNDUN_PART1:
	case MODEL_CUNDUN_PART2:
	case MODEL_CUNDUN_PART3:
	case MODEL_CUNDUN_PART4:
	case MODEL_CUNDUN_PART5:
	case MODEL_CUNDUN_PART6:
	case MODEL_CUNDUN_PART7:
	case MODEL_CUNDUN_PART8:
	case MODEL_MONSTER01+64:	// 진짜쿤둔
		// 쿤둔 조각 좌표맞추는 코드 -_-
		switch(o->SubType)
		{
		case 1:	// 몸
			o->Alpha -= 0.01f;
			break;
		case 2:	// 투구
		case 3:	// 갑옷
			if (o->LifeTime < 250+50+(34-o->Skill)*5)	// 갑옷 떨구기
			{
				//RenderMeshEffect
				if (o->Position[2] > 350)//o->StartPosition[2])
				{
					if (o->Direction[0] == 0 && o->Direction[1] == 0 && o->Direction[2] == 0)
					{
						o->Direction[0] = (float)((-1 + (rand()%2))*2);
						o->Direction[1] = (float)((-1 + (rand()%2))*2);
						o->Direction[2] = 0;
					}
					o->Direction[2] -= 6.0f;
				}
				else// if (o->Direction[2] != o->StartPosition[2])
				{
					BOOL bUseEarthQuake = FALSE;
					if(o->Direction[2] != -3.0f) bUseEarthQuake = TRUE;

					if(bUseEarthQuake == TRUE)
					{
						int PositionX = (int)(o->Position[0]/TERRAIN_SCALE);
						int PositionY = (int)(o->Position[1]/TERRAIN_SCALE);
						AddWaterWave(PositionX, PositionY, 2, -1000);

						vec3_t Light = { 0.3f, 0.3f, 0.3f };
						vec3_t Angle;
						for (int i = 0; i < 100; ++i)
						{
							VectorCopy ( o->Position, Position );
							float fAngle = (float)( rand()%360 );
							float fLength = (float)( rand()%30+30 );
							Position[0] += sinf(fAngle) * fLength;
							Position[1] += cosf(fAngle) * fLength;
							Position[2] -= 30.f;
							VectorCopy ( o->Angle, Angle );
							Angle[2] += rand() % 180;
							CreateParticle ( BITMAP_WATERFALL_5, Position, Angle, Light, 2 );
						}
					}
					VectorCopy ( o->Position, Position );
					Position[0] += rand()%40-20;
					Position[1] += rand()%40-20;
					Position[2] -= 50.f;
					CreateParticle ( BITMAP_BUBBLE, Position, o->Angle, o->Light, 4 );

					o->Direction[0] = 0;
					o->Direction[1] = 0;
					o->Direction[2] = -3.0f;
					// 넘어지기
					if (o->HeadAngle[0] == 0)
					{
						if (o->Angle[0] > -30)
						{
							Vector(o->Angle[0]-(rand()%3+1), o->Angle[1]-0.0f, o->Angle[2]-0.0f, o->Angle);
						}
						else
						{
							o->HeadAngle[0] = 1;

						}
					}
					else
					{
						if (o->Angle[0] < 30)
						{
							Vector(o->Angle[0]+(rand()%3+1), o->Angle[1]-0.0f, o->Angle[2]-0.0f, o->Angle);
						}
						else
						{
							o->HeadAngle[0] = 0;

						}
					}
					if (o->LifeTime < 100)
					{
						o->Alpha -= 0.1f;
					}
				}
			}
			else	// 사라지는 효과
			{
				if (o->SubType != 2 && o->LifeTime > 200+50+135)
				{
					vec3_t Angle = { 0.0f, 0.0f, 0.0f};
					if (rand()%2==0)
					{
						//Angle[2] = ( float)i * 10.0f;
						Angle[0] = 0;//( float)( rand() % 360);
						Angle[1] = 0;//( float)( rand() % 360);
						Angle[2] = ( float)( rand() % 360);
						vec3_t Position;
						VectorCopy( o->Position, Position);
						//Position[2] += 100.f;
						if (rand()%2 == 0)
							CreateJoint(BITMAP_JOINT_SPIRIT2,Position,Position,Angle,8,NULL,50.f,0,0);
					}

					if (rand()%3 == 0)
					{
						vec3_t Light;
						Vector ( 0.5f, 0.5f, 0.5f, Light );
						CreateParticle(BITMAP_SMOKE+3,o->Position,o->Angle,Light,2,(float)(rand()%32+48)*0.02f);
					}
				}
			}
			break;
		case 4:	// 망또
			if (o->LifeTime < 140)
			{
				o->Alpha -= 0.01f;
				if (o->Position[2] > o->StartPosition[2])
				{
					o->Direction[2] -= 3.5f;
				}
				else
				{
					//o->Position[2] = 550;
					o->Direction[0] = 0;
					o->Direction[1] = 0;
					o->Direction[2] = 0;
				}
			}
			else
			{
				o->AnimationFrame = 0;
			}
			break;
		case 5:	// 치마
			if (o->LifeTime < 150)
			{
				o->Alpha -= 0.01f;
			}
			else
			{
				o->AnimationFrame = 0;
			}
			break;
		}
		break;
	case MODEL_CURSEDTEMPLE_STATUE_PART1:
	case MODEL_CURSEDTEMPLE_STATUE_PART2:
		VectorCopy(o->Light,Light);
		if (o->Position[2] > 290)//o->StartPosition[2])
		{
			o->Direction[2] -= o->Gravity;
			o->Angle[2] -= ( float)(rand()%10);
		}
		else if (o->Position[2] < 290)
		{
			o->Position[2] = 290;
			o->Direction[0] = 0;
			o->Direction[1] = 0;
			o->Direction[2] = 0;
            EarthQuake = (float)(rand()%4+1)*0.1f;
		}
		else
		{
			o->Alpha -= 0.01f;
		}
		break;
#ifdef LDK_ADD_SNOWMAN_CHANGERING
	case MODEL_XMAS2008_SNOWMAN_HEAD:
		{
			if(o->LifeTime < 45)
			{
				o->Position[0] += (o->Direction[0]*1.2f);
				o->Position[1] += (o->Direction[1]*1.2f);
				o->Position[2] += (o->Gravity * 1.5f);
				o->Gravity -= 1.5f;
				
				Height = RequestTerrainHeight(o->Position[0],o->Position[1]);
				if(o->Position[2] < Height)
				{
					o->Position[2] = Height;
					o->Gravity = -o->Gravity*0.3f;
					o->LifeTime -= 2;
				}
				
				VectorAdd(o->Position, o->Direction, o->Position);
			}
		}
		break;
	case MODEL_XMAS2008_SNOWMAN_BODY:
		{
			BMD * b = &Models[o->Type];
			b->PlayAnimation( &o->AnimationFrame, &o->PriorAnimationFrame, &o->PriorAction, o->Velocity/5.f, o->Position, o->Angle );
		}
		break;
#endif //LDK_ADD_SNOWMAN_CHANGERING
	case MODEL_TOTEMGOLEM_PART1:
	case MODEL_TOTEMGOLEM_PART2:
	case MODEL_TOTEMGOLEM_PART3:
	case MODEL_TOTEMGOLEM_PART4:
	case MODEL_TOTEMGOLEM_PART5:
	case MODEL_TOTEMGOLEM_PART6:
#ifdef ASG_ADD_KARUTAN_MONSTERS
	case MODEL_CONDRA_ARM_L:
	case MODEL_CONDRA_ARM_L2:
	case MODEL_CONDRA_SHOULDER:
	case MODEL_CONDRA_ARM_R:
	case MODEL_CONDRA_ARM_R2:
	case MODEL_CONDRA_CONE_L:
	case MODEL_CONDRA_CONE_R:
	case MODEL_CONDRA_PELVIS:
	case MODEL_CONDRA_STOMACH:
	case MODEL_CONDRA_NECK:
	case MODEL_NARCONDRA_ARM_L:
	case MODEL_NARCONDRA_ARM_L2:
	case MODEL_NARCONDRA_SHOULDER_L:
	case MODEL_NARCONDRA_SHOULDER_R:
	case MODEL_NARCONDRA_ARM_R:
	case MODEL_NARCONDRA_ARM_R2:
	case MODEL_NARCONDRA_ARM_R3:
	case MODEL_NARCONDRA_CONE_1:
	case MODEL_NARCONDRA_CONE_2:
	case MODEL_NARCONDRA_CONE_3:
	case MODEL_NARCONDRA_CONE_4:
	case MODEL_NARCONDRA_CONE_5:
	case MODEL_NARCONDRA_CONE_6:
	case MODEL_NARCONDRA_PELVIS:
	case MODEL_NARCONDRA_STOMACH:
	case MODEL_NARCONDRA_NECK:
#endif	// ASG_ADD_KARUTAN_MONSTERS
		o->HeadAngle[2] -= o->Gravity;
		VectorCopy(o->Light,Light);

		o->Position[0] += o->HeadAngle[0];
		o->Position[1] += o->HeadAngle[1];
		o->Position[2] += o->HeadAngle[2];
			
		Height = RequestTerrainHeight(o->Position[0],o->Position[1]) +20;

		if (o->Position[2] + o->Direction[2] <= Height)
		{
			o->Position[2] = Height;
			o->HeadAngle[0] *= 0.8f;
			o->HeadAngle[1] *= 0.8f;
			o->HeadAngle[2] += 0.6f*o->LifeTime;
			if (o->HeadAngle[2] < 5.0f)
				o->HeadAngle[2] = 0;

			o->Alpha -= 0.05f;
		}
		else
		{
			if (o->SubType == 0)
			{
				o->Angle[0] += 0.15f * o->LifeTime;
				o->Angle[1] += 0.15f * o->LifeTime;
			}
			else
			{
				o->Angle[0] -= 0.15f * o->LifeTime;
				o->Angle[1] -= 0.15f * o->LifeTime;
			}
		}
		break;
#ifdef YDG_ADD_DOPPELGANGER_MONSTER
	case MODEL_DOPPELGANGER_SLIME_CHIP:
		o->HeadAngle[2] -= o->Gravity;
		VectorCopy(o->Light,Light);

		o->Position[0] += o->HeadAngle[0];
		o->Position[1] += o->HeadAngle[1];
		o->Position[2] += o->HeadAngle[2];
			
		Height = RequestTerrainHeight(o->Position[0],o->Position[1]) +20;

		if (o->LifeTime < 10)
		{
			o->Light[0] *= 0.8f;
			o->Light[1] *= 0.8f;
			o->Light[2] *= 0.8f;
		}

		if (o->Position[2] + o->Direction[2] <= Height)
		{
			o->Position[2] = Height;
			o->HeadAngle[0] *= 0.8f;
			o->HeadAngle[1] *= 0.8f;
			o->HeadAngle[2] += 1.6f*o->LifeTime;
			if (o->HeadAngle[2] < 5.0f)
				o->HeadAngle[2] = 0;
		}
		else
		{
			o->Scale += sinf(WorldTime*0.015f)*0.1f;
		}

		if (o->SubType == 0)
		{
			o->Angle[0] += 0.35f * o->LifeTime;
			o->Angle[1] += 0.35f * o->LifeTime;
		}
		else
		{
			o->Angle[0] -= 0.35f * o->LifeTime;
			o->Angle[1] -= 0.35f * o->LifeTime;
		}
		break;
#endif	// YDG_ADD_DOPPELGANGER_MONSTER

	case MODEL_SHADOW_PAWN_ANKLE_LEFT:		case MODEL_SHADOW_PAWN_ANKLE_RIGHT:
	case MODEL_SHADOW_PAWN_BELT:			case MODEL_SHADOW_PAWN_CHEST:
	case MODEL_SHADOW_PAWN_HELMET:
	case MODEL_SHADOW_PAWN_KNEE_LEFT:		case MODEL_SHADOW_PAWN_KNEE_RIGHT:
	case MODEL_SHADOW_PAWN_WRIST_LEFT:		case MODEL_SHADOW_PAWN_WRIST_RIGHT:

	case MODEL_SHADOW_KNIGHT_ANKLE_LEFT:	case MODEL_SHADOW_KNIGHT_ANKLE_RIGHT:
	case MODEL_SHADOW_KNIGHT_BELT:			case MODEL_SHADOW_KNIGHT_CHEST:
	case MODEL_SHADOW_KNIGHT_HELMET:
	case MODEL_SHADOW_KNIGHT_KNEE_LEFT:		case MODEL_SHADOW_KNIGHT_KNEE_RIGHT:
	case MODEL_SHADOW_KNIGHT_WRIST_LEFT:	case MODEL_SHADOW_KNIGHT_WRIST_RIGHT:

	case MODEL_SHADOW_ROOK_ANKLE_LEFT:		case MODEL_SHADOW_ROOK_ANKLE_RIGHT:
	case MODEL_SHADOW_ROOK_BELT:			case MODEL_SHADOW_ROOK_CHEST:
	case MODEL_SHADOW_ROOK_HELMET:
	case MODEL_SHADOW_ROOK_KNEE_LEFT:		case MODEL_SHADOW_ROOK_KNEE_RIGHT:
	case MODEL_SHADOW_ROOK_WRIST_LEFT:		case MODEL_SHADOW_ROOK_WRIST_RIGHT:

#ifdef ADD_RAKLION_MOB_ICEGIANT
	case MODEL_ICE_GIANT_PART1:				case MODEL_ICE_GIANT_PART2:
	case MODEL_ICE_GIANT_PART3:				case MODEL_ICE_GIANT_PART4:
	case MODEL_ICE_GIANT_PART5:				case MODEL_ICE_GIANT_PART6:
#endif	// ADD_RAKLION_MOB_ICEGIANT
		o->HeadAngle[2] -= o->Gravity;
		VectorCopy(o->Light,Light);

		o->Position[0] += o->HeadAngle[0];
		o->Position[1] += o->HeadAngle[1];
		o->Position[2] += o->HeadAngle[2];
			
		Height = RequestTerrainHeight(o->Position[0],o->Position[1]) +20;

		if (o->Position[2] + o->Direction[2] <= Height)
		{
			o->Position[2] = Height;
			o->HeadAngle[0] *= 0.5f;
			o->HeadAngle[1] *= 0.5f;
			o->HeadAngle[2] += 0.6f*o->LifeTime;
			if (o->HeadAngle[2] < 5.0f)
				o->HeadAngle[2] = 0;

			o->Alpha -= 0.05f;
		}
		else
		{
			if (o->SubType == 0)
			{
				o->Angle[0] += 0.15f * o->LifeTime;
				o->Angle[1] += 0.15f * o->LifeTime;
			}
			else
			{
				o->Angle[0] -= 0.15f * o->LifeTime;
				o->Angle[1] -= 0.15f * o->LifeTime;
			}
		}
		break;

    case MODEL_WATER_WAVE:
        o->Scale += 0.02f;
        o->Position[2] = o->StartPosition[2] + o->Gravity;

        o->Velocity-= 10.f;
        o->Gravity -= 20.f;
        if ( o->Gravity<60 ) o->Gravity = 60;

        Vector ( 0.f, o->Velocity, 0.f, o->Direction );
		Vector ( Luminosity*0.1f, Luminosity*0.3f, Luminosity*0.6f, Light );
#ifdef _VS2008PORTING
        for ( int j=0; j<4; j++ )
#else // _VS2008PORTING
        for ( j=0; j<4; j++ )
#endif // _VS2008PORTING
        {
            CreateParticle ( BITMAP_WATERFALL_5, o->Position, o->Angle, Light, 1 );
        }
		AddTerrainLight ( o->Position[0], o->Position[1], Light, 3, PrimaryTerrainLight );
        break;

	case MODEL_STAFF+8:
		Height = RequestTerrainHeight(o->Position[0],o->Position[1]);
		if(o->Position[2] < Height)
		{
			VectorCopy(o->Position,Position);
			Position[2] += 80;
			CreateParticle(BITMAP_EXPLOTION,Position,o->Angle,Light);
#ifdef _VS2008PORTING
			for(int j=0;j<6;j++)
#else // _VS2008PORTING
			for(j=0;j<6;j++)
#endif // _VS2008PORTING
			{
				CreateEffect(MODEL_STONE1+rand()%2,o->Position,o->Angle,o->Light);
			}
			o->Live = false;
		}
		Vector(Luminosity*0.2f,Luminosity*0.4f,Luminosity*1.f,Light);
		AddTerrainLight(o->Position[0],o->Position[1],Light,2,PrimaryTerrainLight);
		break;

    case MODEL_PIERCING:
        if(o->Owner->Live==false)
        {
            o->LifeTime = -1;
            break;
        }
		/*
					Vector(Luminosity*0.9f,Luminosity*0.4f,Luminosity*0.6f,Light);
			AddTerrainLight(o->StartPosition[0],o->StartPosition[1],Light,2,PrimaryTerrainLight);
		*/
        o->Gravity += 90.f;
        VectorCopy(o->Owner->Angle, o->Angle);
        VectorCopy(o->Owner->Angle, o->HeadAngle);
        VectorCopy(o->Position,o->StartPosition);
        VectorCopy(o->Owner->Position, o->Position);
        if( o->SubType==1 )
        {
            Vector(1.f,1.f,1.f,Light);
            
    		CreateJoint(BITMAP_JOINT_THUNDER,o->Position,o->Position,o->Angle,3,NULL,20.f,7); //  전기 
            CreateSprite(BITMAP_SHINY+1,o->Position,(float)(rand()%8+8)*0.2f,Light,o,(float)(rand()%360));
        }

        if ( InHellas() )
        {
            if ( o->Owner!=NULL && o->Owner->Owner!=NULL && o->Owner->Owner==(&Hero->Object) )
            {
                int PositionX = (int)(o->Position[0]/TERRAIN_SCALE);
                int PositionY = (int)(o->Position[1]/TERRAIN_SCALE);
                AddWaterWave( PositionX, PositionY, 2, -200 );
            }
        }

        o->HeadAngle[1] = o->Gravity;
        break;

    case MODEL_PIERCING+1:
        if((rand()%2)==0)
        {
            CreateParticle(BITMAP_SMOKE,o->Position,o->Angle,Light, 0);
        }
        CreateParticle(BITMAP_FIRE,o->Position,o->Angle,Light,5);
        CreateParticle(BITMAP_FIRE,o->Position,o->Angle,Light,5);

		Vector(Luminosity*0.6f,Luminosity*0.8f,Luminosity*0.8f,Light);
		AddTerrainLight(o->Position[0],o->Position[1],Light,2,PrimaryTerrainLight);
        break;
        
	case MODEL_ARROW_BEST_CROSSBOW :
		if ( o->LifeTime>25 )
		{
			o->BlendMeshLight = (30-o->LifeTime)/40.f;
		}
		else
		{
			o->BlendMeshLight = 1.f;
		}
		o->Angle[1] += rand()%60+30.f;
		VectorCopy(o->Position,o->EyeLeft);
		
		VectorCopy(o->Angle,Angle);
		AngleMatrix(Angle,Matrix);
		VectorRotate(o->Direction,Matrix,Position);
		VectorAdd(o->EyeLeft,Position,o->EyeLeft);

		Vector(Luminosity*1.f,Luminosity*0.4f,Luminosity*0.2f,Light);
		AddTerrainLight(o->Position[0],o->Position[1],Light,2,PrimaryTerrainLight);
		CheckClientArrow(o);
		break;

    case MODEL_ARROW_DOUBLE:
		o->Angle[1] += 30.f;
		VectorCopy(o->Position,o->EyeLeft);
		Vector(Luminosity*0.2f,Luminosity*0.4f,Luminosity*1.f,Light);
		AddTerrainLight(o->Position[0],o->Position[1],Light,2,PrimaryTerrainLight);
		CheckClientArrow(o);
		break;
	case MODEL_ARROW_HOLY:

#ifdef CRYINGWOLF_2NDMVP
		if(o->SubType==1)
		{
			o->Angle[1] += 60.f;
			//. o->Target이 셋팅되어야 작동한다.
			if(o->LifeTime==13)
				CreateEffect(MODEL_PIERCING,o->Position,o->Angle,o->Light,3,o);
			CheckClientArrow(o);

			Vector(Luminosity*0.9f,Luminosity*0.4f,Luminosity*0.6f,Light);
			AddTerrainLight(o->Position[0],o->Position[1],Light,2,PrimaryTerrainLight);
		}
		else
#endif // CRYINGWOLF_2NDMVP
		{
			o->Angle[1] += 30.f;

			Vector(Luminosity*0.2f,Luminosity*0.4f,Luminosity*1.f,Light);	
			AddTerrainLight(o->StartPosition[0],o->StartPosition[1],Light,2,PrimaryTerrainLight);
			
			if(o->SubType!=0)
			{
				if ( o->LifeTime==13 )
				{		
					CreateEffect(MODEL_PIERCING,o->Position,o->Angle,o->Light,0,o);
				}
				else if ( o->LifeTime==30 )
				{
					o->AttackPoint[0] = 0;
					o->Kind = 1;
				}
			}
		}
        break;
	case MODEL_ARROW:
		Vector(Luminosity*0.8f,Luminosity*0.5f,Luminosity*0.2f,Light);
		AddTerrainLight(o->Position[0],o->Position[1],Light,2,PrimaryTerrainLight);

        //  땅으로 떨어지는 화살.
        if ( o->SubType==3 || o->SubType==4 )
        {
            if ( o->SubType==3 )
            {
			    o->Angle[0] += o->Gravity;
            }
            if ( o->Angle[0]>50 )
            {
                o->Angle[0] = 50.f;
            }

            int PositionX = (int)(o->Position[0]/TERRAIN_SCALE);
            int PositionY = (int)(o->Position[1]/TERRAIN_SCALE);
            int WallIndex = TERRAIN_INDEX_REPEAT( PositionX, PositionY );
            int Wall = TerrainWall[WallIndex]&TW_NOGROUND;

            if ( Wall!=TW_NOGROUND )
            {
			    float Height = RequestTerrainHeight( o->Position[0], o->Position[1] );
			    if(o->Position[2] < Height)
                {
				    o->Position[2] = Height+10;
				    Vector(0.f,0.f,0.f,o->Direction);
			    }
            }
            CreateParticle ( BITMAP_FIRE, o->Position, o->Angle, Light, 5 );
        }
		else if( o->SubType==5 )
		{
			Vector(Luminosity*0.1f,Luminosity*0.6f,Luminosity*0.3f,Light);
			CreateParticle(BITMAP_FIRE,o->Position,o->Angle,Light);
            CheckClientArrow(o);
		}
        else
        {
            CreateParticle(BITMAP_FIRE,o->Position,o->Angle,Light);
            CheckClientArrow(o);
        }
		break;
	case MODEL_ARROW_STEEL:
	case MODEL_ARROW_THUNDER:
	case MODEL_ARROW_LASER:
	case MODEL_ARROW_V:
	case MODEL_ARROW_SAW:
	case MODEL_ARROW_NATURE:
	case MODEL_ARROW_WING:
    case MODEL_LACEARROW:
		if(o->Type==MODEL_ARROW_NATURE)
		{
#ifdef CRYINGWOLF_2NDMVP
			if(o->SubType==1) {	//. 몬스터 강력 독화살
				Vector(0.1f,0.4f,0.1f, Light);
				CreateSprite(BITMAP_LIGHTNING+1,o->Position,0.3f,Light,o,(int)WorldTime*0.1f);
				CreateSprite(BITMAP_LIGHTNING+1,o->Position,0.7f,Light,o,-(int)WorldTime*0.1f);

				Vector(Luminosity*0.2f,Luminosity*0.8f,Luminosity*0.2f,Light);
				AddTerrainLight(o->Position[0],o->Position[1],Light,2,PrimaryTerrainLight);

				o->Angle[1] += 60.f;
				
				if ( o->LifeTime > 25 && (o->LifeTime%2)==0)
				{
					for(int j=0;j<2;j++)
					{
						Vector ( (float)(rand()%32-16), (float)(rand()%64-32), (float)(rand()%32-16), Position );
						VectorAdd ( Position, o->Position, Position );
						
						Vector ( 0.4f, 1.f, 0.2f , Light );
						CreateParticle(BITMAP_FLARE,Position,o->Angle,Light,5,0.2f);
					}
				}
			}
			else 
#endif // CRYINGWOLF_2NDMVP
			{
				CreateSprite(BITMAP_LIGHTNING+1,o->Position,0.5f,o->Light,o,(int)WorldTime*0.1f);
				CreateSprite(BITMAP_LIGHTNING+1,o->Position,1.f,o->Light,o,-(int)WorldTime*0.1f);
				for(int j=0;j<4;j++)
				{
					Vector((float)(rand()%32-16),(float)(rand()%64-32),(float)(rand()%32-16),Position);
					VectorAdd(Position,o->Position,Position);
					CreateParticle(BITMAP_FLOWER01+rand()%3,Position,o->Angle,o->Light);
					//CreateParticle(BITMAP_BUBBLE,Position,o->Angle,o->Light);
				}
				o->Angle[1] += 30.f;
				
				Vector(Luminosity*0.6f,Luminosity*0.8f,Luminosity*0.8f,Light);
				AddTerrainLight(o->Position[0],o->Position[1],Light,2,PrimaryTerrainLight);
			}
		}
        else if ( o->Type==MODEL_LACEARROW )
        {
			o->Angle[1] += 60.f;

            if ( (o->LifeTime%2)==0 )
            {
			    for(int j=0;j<3;j++)
			    {
				    Vector ( (float)(rand()%32-16), (float)(rand()%64-32), (float)(rand()%32-16), Position );
				    VectorAdd ( Position, o->Position, Position );

                    Vector ( 0.4f, 0.2f, 1.f , Light );
				    CreateParticle(BITMAP_FLARE,Position,o->Angle,Light,5,0.2f);
			    }
            }
			VectorCopy ( o->Position, o->EyeLeft );
            CreateEffect ( MODEL_WAVES, o->Position, o->Angle, o->Light, 3, NULL, 0 );
		    
            Vector(Luminosity*0.6f,Luminosity*0.2f,Luminosity*0.8f,Light);
		    AddTerrainLight(o->Position[0],o->Position[1],Light,2,PrimaryTerrainLight);
        }
		else if(o->Type==MODEL_ARROW_WING)
		{
     		CreateSprite(BITMAP_LIGHTNING+1,o->Position,0.5f,o->Light,o,(int)WorldTime*0.1f);
     		CreateSprite(BITMAP_LIGHTNING+1,o->Position,1.f,o->Light,o,-(int)WorldTime*0.1f);
			for(int j=0;j<4;j++)
			{
				Vector((float)(rand()%16-8),(float)(rand()%16-8),(float)(rand()%16-8),Position);
				VectorAdd(Position,o->Position,Position);
				CreateParticle(BITMAP_BUBBLE,Position,o->Angle,o->Light,1);
			}
     		CreateParticle(BITMAP_SMOKE,o->Position,o->Angle,o->Light,0);
		    
            Vector(Luminosity*0.6f,Luminosity*0.8f,Luminosity*0.8f,Light);
		    AddTerrainLight(o->Position[0],o->Position[1],Light,2,PrimaryTerrainLight);
		}
		CheckClientArrow(o);
        break;
	case MODEL_DARK_SCREAM_FIRE:
	case MODEL_DARK_SCREAM:
		{
			if(o->Type == MODEL_DARK_SCREAM_FIRE)
			{
				o->Scale -= 0.14f;
			}
			if(o->Type == MODEL_DARK_SCREAM)
			{
				o->Scale -= 0.04f;
			}
			if(o->Scale < 0.1f)
			{
				o->Scale = 0.f;
			}
				
			o->Position[2] = RequestTerrainHeight(o->Position[0],o->Position[1]) + 3.f;

			CreateParticle(BITMAP_FLAME, o->Position, o->Angle, o->Light, 8, (o->Scale - 0.4f)*3.5f);
			if(o->Type == MODEL_DARK_SCREAM)
			{
				CheckClientArrow(o);
			}
		}
		break;
	case MODEL_CURSEDTEMPLE_HOLYITEM:
		{
			if( !o->Owner->Live ) 
			{
				o->Live = false;
				break;
			}

			if( o->LifeTime < 10 ) o->LifeTime = 999999;

			BMD *b = &Models[o->Owner->Type];
			vec3_t vRelativePos, vWorldPos;
			Vector(70.f,0.f,0.f,vRelativePos);

			VectorCopy(o->Owner->Position,b->BodyOrigin);
			b->TransformPosition(o->Owner->BoneTransform[20],vRelativePos,vWorldPos,true);

			//vWorldPos[0] = o->Owner->Position[0];
			//vWorldPos[1] = o->Owner->Position[1];

			Vector(1.0f, 1.0f, 1.0f, o->Light);
			VectorCopy(vWorldPos, o->Position);

			b->PlayAnimation( &o->AnimationFrame, &o->PriorAnimationFrame, &o->PriorAction, o->Velocity/3.f, o->Position, o->Angle );
		}
		break;
	case MODEL_CURSEDTEMPLE_PRODECTION_SKILL:
		{
			if( !o->Owner->Live ) 
			{
				o->Live = false;
				break;
			}

			if( o->LifeTime < 10 ) o->LifeTime = 999999;

			vec3_t RelativePos, Position;
			BMD *herobmd = &Models[o->Owner->Type];
			VectorCopy(o->Owner->Position,herobmd->BodyOrigin);
			Vector(-23.f, 0.f, 0.f, RelativePos);
			herobmd->TransformPosition( o->Owner->BoneTransform[2], RelativePos, Position, true );

			Vector(1.0f, 1.0f, 1.0f, o->Light);
			VectorCopy(Position, o->Position);
			VectorCopy(o->Owner->Angle, o->Angle);
			
			o->Alpha = 0.3f;
			o->Scale = 1.0f;

			BMD *effbmd = &Models[o->Type];
			
			effbmd->SetBodyLight( o->Light );
			effbmd->PlayAnimation( &o->AnimationFrame, &o->PriorAnimationFrame, &o->PriorAction, o->Velocity/3.f, o->Position, o->Angle );

			CreateEffect ( BITMAP_SHOCK_WAVE, o->Owner->Position, o->Angle, o->Light, 10, o->Owner );
		}
		break;
	case MODEL_CURSEDTEMPLE_RESTRAINT_SKILL:
		{
			if( !o->Owner->Live ) 
			{
				o->Live = false;
				break;
			}

			if( o->LifeTime < 10 ) o->LifeTime = 999999;

			Vector(1.0f, 1.0f, 1.0f, o->Light);
			VectorCopy(o->Owner->Position, o->Position);
			VectorCopy(o->Owner->Angle, o->Angle);
			o->Alpha = 0.6f;
			o->Scale = 1.0f;
			
			BMD *b = &Models[o->Type];
			b->PlayAnimation( &o->AnimationFrame, &o->PriorAnimationFrame, &o->PriorAction, o->Velocity/3.f, o->Position, o->Angle );

			vec3_t vRelativePos, Light;
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
			BMD *ob = &Models[o->Owner->Type];
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
			Vector(0.4f, 0.4f, 0.8f, Light);
			Vector(0.f, 0.f, 0.f, vRelativePos);
		}
		break;
	case MODEL_ARROW_SPARK:
		{
			o->Angle[1] += 35.0f;
			CheckClientArrow(o);
		}
		break;
	case MODEL_ARROW_RING:	//$ 요정화살
		{
			VectorCopy ( o->Position, o->EyeLeft );
			Vector(0.0f, 1.0f, 0.1f, o->Light);
			CreateEffect ( MODEL_WAVES, o->Position, o->Angle, o->Light, 4, NULL, 0 );
		    
			Vector(Luminosity*0.6f,Luminosity*0.2f,Luminosity*0.8f,Light);
		    AddTerrainLight(o->Position[0],o->Position[1],Light,2,PrimaryTerrainLight);

			CheckClientArrow(o);
		}
		break;	
	case MODEL_ARROW_TANKER:
		{
			Vector(1.0f, 0.0f, 0.0f, o->Light);
			AddTerrainLight(o->Position[0],o->Position[1],o->Light,2,PrimaryTerrainLight);
		}
		break;
	case MODEL_ARROW_BOMB:
		{
			Vector(1.f,0.6f,0.4f,Light);
			CreateSprite(BITMAP_LIGHT,o->Position,1.f,Light,o,(int)WorldTime*0.1f);
			CreateSprite(BITMAP_LIGHT,o->Position,2.f,Light,o,-(int)WorldTime*0.1f);

			for(int j=0;j<4;j++)
			{
				Vector((float)(rand()%16-8),(float)(rand()%16-8),(float)(rand()%16-8),Position);
				VectorAdd(Position,o->Position,Position);
				CreateParticle(BITMAP_BUBBLE,Position,o->Angle,o->Light,1);
			}
			MoveJump(o);
			if(o->LifeTime == 1)
			{
				CreateBomb(o->Position,true);
				if(o->Owner==&Hero->Object && o->SubType!=99)
					AttackCharacterRange(o->Skill,o->Position,100.f,o->Weapon,o->PKKey);//클라이언트마법처리
			}
			
			Vector(Luminosity*0.6f,Luminosity*0.8f,Luminosity*0.8f,Light);
			AddTerrainLight(o->Position[0],o->Position[1],Light,2,PrimaryTerrainLight);
			CheckClientArrow(o);
		}
        break;
#ifdef ADD_SOCKET_ITEM
	case MODEL_ARROW_DARKSTINGER:
		{
			// 꼭 해주어야 함(안하면 데미지 계산 안됌)	
			CheckClientArrow(o);
			
			BMD *pModel = &Models[o->Type];
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
			int iNumCreateFeather = rand()%2;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
			vec3_t vPos;

			Vector( 0.6f, 0.7f, 0.9f, Light);
			pModel->Animation( BoneTransform, o->AnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle, false, false);	
			pModel->TransformByObjectBone( vPos, o, 0 );
			
			// 오브젝트의 포지션 Update부분이 밑에 있기 때문에
			// 이팩트의 포지션을 원하는 위치에 붙이려면 포지션계산을 미리 해주어야 한다. (-0-)
			float Matrix[3][4];
			vec3_t Angle;
			vec3_t Position;
			VectorCopy(o->Angle,Angle);
			AngleMatrix(Angle,Matrix);
			VectorRotate(o->Direction,Matrix,Position);
			VectorAdd(vPos, Position, vPos);
			
			CreateSprite( BITMAP_LIGHT, vPos, 3.0f, Light, o );
			CreateSprite( BITMAP_LIGHT, vPos, 2.0f, Light, o );


			if( o->LifeTime == 30 || o->LifeTime == 28 || o->LifeTime == 26 || o->LifeTime == 24)
			{
				int iNumCreateFeather = rand()%3;
				Vector( 0.6f, 0.7f, 0.9f, Light);	
				pModel->TransformByObjectBone( vPos, o, 1 );		// Zx01
				for( int i=0 ; i<iNumCreateFeather ; i++ )
				{
					CreateEffect( MODEL_FEATHER, vPos, o->Angle, Light, 0, NULL, -1, 0, 0, 0, 0.6f );
					CreateEffect( MODEL_FEATHER, vPos, o->Angle, Light, 1, NULL, -1, 0, 0, 0, 0.6f );
				}	
			}

			if( o->LifeTime == 30 )
			{
				pModel->TransformByObjectBone( vPos, o, 1 );		// Zx01
				Vector(0.4f, 0.4f, 0.9f, Light);	//보라색
				CreateJoint( BITMAP_FLARE+1, vPos, vPos, o->Angle, 18, o, 90.f, 40, 0, 0, -1, Light );
			}

		}break;
#endif	// ADD_SOCKET_ITEM
#ifdef LDK_ADD_GAMBLERS_WEAPONS
	case MODEL_ARROW_GAMBLE:
		{
			vec3_t vLight;
			Vector(0.2f, 0.8f, 0.5f, vLight);

			o->Angle[1] += 60.f;

			for(int j=0;j<2;j++)
			{
				Vector ( (float)(rand()%32-16), (float)(rand()%64-32), (float)(rand()%32-16), Position );
				VectorAdd ( Position, o->Position, Position );
				
				CreateParticle(BITMAP_SPARK+1, Position, o->Angle, vLight, 30, 1.f);
			}

			VectorCopy ( o->Position, o->EyeLeft );
     		CreateParticle(BITMAP_SMOKE,o->Position,o->Angle,vLight,24, 1.0f);

            Vector(Luminosity*0.2f,Luminosity*0.8f,Luminosity*0.5f,vLight);
		    AddTerrainLight(o->Position[0],o->Position[1],vLight,2,PrimaryTerrainLight);

			CheckClientArrow(o);
		}break;
#endif //LDK_ADD_GAMBLERS_WEAPONS
#ifdef LDK_ADD_PC4_GUARDIAN_EFFECT_IMAGE
	case MODEL_FEATHER_FOREIGN:
		{
			switch( o->SubType )
			{
			case 4:
				{
					o->Light[0] *= 0.97f;
					o->Light[1] *= 0.97f;
					o->Light[2] *= 0.97f;
					
					o->Alpha *= 0.97f;
					
					VectorAdd( o->EyeRight, o->Angle, o->Angle );
				}
				break;
			}
		}break;
#endif //LDK_ADD_PC4_GUARDIAN_EFFECT_IMAGE
	case MODEL_DUNGEON_STONE01:
		o->Position[2] += o->Gravity;
		o->Gravity -= 1.f;
		Height = RequestTerrainHeight(o->Position[0],o->Position[1]);
		if(o->Position[2] < Height)
		{
			o->Position[2] = Height;
			o->Gravity = -o->Gravity*0.4f;
			o->LifeTime -= 4;
			o->Direction[1] -= 2.f;
		}
		break;
	case MODEL_ICE:
		//o->Position[0] += (o->Owner->Position[0]-o->Position[0])*0.3f;
		//o->Position[1] += (o->Owner->Position[1]-o->Position[1])*0.3f;
        switch ( o->SubType )
        {
        case 0:
		    if(o->AnimationFrame >= 5.f)
		    {
			    o->Velocity = 0.f;
			    if(rand()%2==0)
			    {
				    vec3_t Position;
				    Vector(o->Position[0]+(float)(rand()%64-32),
					    o->Position[1]+(float)(rand()%64-32),
					    o->Position[2]+(float)(rand()%128+32),Position);
				    CreateParticle(BITMAP_SMOKE,Position,o->Angle,o->Light);
			    }
			    //AttackRange(o->Position,200.f,10);
			    o->Alpha -= 0.05f;
			    if(o->Alpha < 0.f)
				    o->Live = false;
		    }
		    Vector(-Luminosity*0.4f,-Luminosity*0.3f,-Luminosity*0.2f,Light);
		    //Vector(Luminosity*0.2f,Luminosity*0.4f,Luminosity,Light);
		    AddTerrainLight(o->Position[0],o->Position[1],Light,2,PrimaryTerrainLight);
            break;

        case 1:
        case 2:
            if ( o->Owner!=NULL )
            {
				if( o->Owner->Live && g_isCharacterBuff(o->Owner, eDeBuff_Harden) )
                {
#ifdef GUILD_WAR_EVENT
					if(o->Owner->Type ==  MODEL_BALL) break;
#endif
                    if ( o->Owner->Type==MODEL_PLAYER )
                    {
                        if ( ( o->Owner->CurrentAction>=PLAYER_WALK_MALE && o->Owner->CurrentAction<=PLAYER_RUN_RIDE_WEAPON ) ||
                             ( o->Owner->CurrentAction>=PLAYER_FLY_RIDE  && o->Owner->CurrentAction<=PLAYER_FLY_RIDE_WEAPON ) 
#ifdef PBG_ADD_NEWCHAR_MONK_ANI
							 || ( o->Owner->CurrentAction==PLAYER_RAGE_UNI_RUN || o->Owner->CurrentAction==PLAYER_RAGE_UNI_RUN_ONE_RIGHT )
#endif //PBG_ADD_NEWCHAR_MONK_ANI
							 )
                        {
                            o->AnimationFrame = 0.f;
                            o->PriorAnimationFrame = 0.f;
                            o->HiddenMesh = -2;
                        }
                        else                                                   
                        {
                            o->HiddenMesh = -1;
                        }
                    }
                    else
                    {
                        if ( o->Owner->CurrentAction!=2 ) 
                        {
                            o->HiddenMesh = -1;
                        }
                        else 
                        {
                            o->PriorAnimationFrame = 0.f;
                            o->AnimationFrame = 0.f;
                            o->HiddenMesh = -2;
                        }
                    }
                    VectorCopy ( o->Owner->Position, o->Position );
                    if ( o->AnimationFrame>=4.f ) o->AnimationFrame = 4.f;
                    o->LifeTime = 10;
                }
                else
                {
                    o->Alpha -= 0.05f;
                }

                if ( o->SubType==1 )
                {
                    o->HeadAngle[2]+=20.f;
                    o->Gravity  = sinf( WorldTime*0.01f )*20.f+30;

                    Vector ( 60.f, 0.f, 0.f, p );
                    AngleMatrix ( o->HeadAngle, Matrix );
			        VectorRotate ( p, Matrix, Position );
			        VectorAdd ( o->Owner->Position, Position, Position );

                    Position[2] += o->Gravity;
                    Vector ( 1.f, 1.f, 1.f, Light );
                    CreateParticle ( BITMAP_FIRE+2, Position, o->Angle, Light, 10, 2.f );
                }
            }
            else
            {
                o->Live = false;
            }
            break;
        }
		break;
	case MODEL_FIRE:
		if(o->SubType == 0 || o->SubType == 2 || o->SubType==4 || o->SubType==6 || o->SubType==7 || o->SubType==8 )
		{
            bool success = false;
            float Height;
            if ( o->SubType==4 )
            {
                int PositionX = (int)(o->Position[0]/TERRAIN_SCALE);
                int PositionY = (int)(o->Position[1]/TERRAIN_SCALE);
                int WallIndex = TERRAIN_INDEX_REPEAT( PositionX, PositionY );
                int Wall = TerrainWall[WallIndex]&TW_NOGROUND;
    
                if ( Wall!=TW_NOGROUND )
                {
			        Height = RequestTerrainHeight( o->Position[0], o->Position[1] );
			        if(o->Position[2] < Height)
                    {
                        success = true;
                        EarthQuake = (float)(rand()%4-4)*0.1f;

                        //  캐릭터를 찾는다.
	                    for( int i=0;i<MAX_CHARACTERS_CLIENT;i++ )
	                    {
		                    CHARACTER *tc = &CharactersClient[i];
		                    OBJECT    *to = &tc->Object;
			                float dx = o->Position[0]-to->Position[0];
			                float dy = o->Position[1]-to->Position[1];
			                float Distance = sqrtf(dx*dx+dy*dy);
			                if(to->Live && to->Visible && tc!=Hero && !tc->Dead && Distance<=200 )
			                {
                                if ( to->Type==MODEL_PLAYER )
                                {
									if(tc->Helper.Type == MODEL_HELPER+37)
										SetAction_Fenrir_Damage(tc, to);
									else
										SetAction(to,PLAYER_SHOCK);
                                }
                                else
                                {
		                            SetAction(to,MONSTER01_SHOCK);
                                }
			                }
	                    }
                    }
                }
                else
                {
                    if ( o->Position[2]<-200 )
                    {
                        success = true;
                        Height = o->Position[2];
                    }
                }
            }
            else
            {
				if ( o->SubType==6 || o->SubType==7 || o->SubType==8 )
				{
					Vector ( 0.5f, 0.5f, 1.f, Light );
					CreateSprite ( BITMAP_LIGHT,   o->Position, 3.f, Light, o );
					Vector ( 1.f, 1.f, 1.f, Light );
					CreateSprite ( BITMAP_SHINY+1, o->Position, 3.f, Light, o, (float)(rand()%360), 1 );
					CreateSprite ( BITMAP_SHINY+1, o->Position, 4.f, o->Light, o, (float)(rand()%360) );

					VectorCopy ( o->HeadAngle, o->Angle );
				}
                float AddHeight = 0.f;
                if ( InHellas() )
                {
                    AddHeight = 50.f;
                }
			    Height = RequestTerrainHeight(o->Position[0],o->Position[1]) + AddHeight;
			    if(o->Position[2] < Height)
                {
                    success = true;
                }
            }

            if ( success )
            {
				o->Position[2] = Height;
				Vector(0.f,0.f,0.f,o->Direction);
				vec3_t Position;
				Vector(o->Position[0],o->Position[1],o->Position[2]+80.f,Position);
				if ( o->SubType==6 || o->SubType==7 || o->SubType==8 )
				{
                    BYTE smokeNum = 15;

					Vector(0.f,0.5f,0.f,Light);
					Vector(0.f,0.f,0.f,Angle);
			        CreateEffect(MODEL_SKILL_INFERNO,o->Position,Angle,Light, 2, o, 30, 0);

                    if ( o->SubType==7 )
                    {
                        smokeNum = 7;
                    }
                    else if ( o->SubType==8 )
                    {
                        smokeNum = 5;
                    }

					for ( int i=0; i<smokeNum; ++i )
					{
						Position[0] = o->Position[0]+(rand()%160-80);
						Position[1] = o->Position[1]+(rand()%160-100);
						Position[2] = o->Position[2]+50;

    					Vector ( 0.1f, 0.5f, 0.1f, Light );
						CreateParticle(BITMAP_SMOKE,Position,o->Angle, Light, 11, (float)(rand()%32+80)*0.025f );
					}

                    if ( o->SubType!=7 && o->SubType!=8 )
                    {
					    for(int j=0;j<6;j++)
					    {
						    CreateEffect(MODEL_STONE1+rand()%2,o->Position,o->Angle,Light);
					    }
                    }
					PlayBuffer(SOUND_DEATH_POISON2);
				}
				else
				{
					for(int j=0;j<6;j++)
					{
						CreateEffect(MODEL_STONE1+rand()%2,o->Position,o->Angle,o->Light);
					}
				}
				CreateParticle(BITMAP_EXPLOTION,Position,o->Angle,Light);

				o->Live = false;
			}
		}
        else if ( o->SubType==5 )//  통통볼.
        {
            o->Position[2] += o->Gravity;
            o->Gravity -= 2.f;

            Height = RequestTerrainHeight(o->Position[0],o->Position[1]);
			if(o->Position[2] < Height)
            {
                o->Gravity = 10.f;
                o->Position[2] += 10.f;
                o->Direction[1] *= 0.5f;
                o->Scale *= 1.1f;
            }

            int PositionX = (int)(o->Position[0]/TERRAIN_SCALE);
            int PositionY = (int)(o->Position[1]/TERRAIN_SCALE);
            int WallIndex = TERRAIN_INDEX_REPEAT( PositionX, PositionY );
            int Wall = TerrainWall[WallIndex]&TW_NOGROUND;

            if ( o->LifeTime==1 || Wall==TW_NOGROUND )
            {
				o->Position[2] = Height;

                Vector(0.f,0.f,0.f,o->Direction);
				vec3_t Position;
				Vector(o->Position[0],o->Position[1],o->Position[2]+80.f,Position);
				CreateParticle(BITMAP_EXPLOTION,Position,o->Angle,Light);
				for(int j=0;j<6;j++)
				{
					CreateEffect(MODEL_STONE1+rand()%2,o->Position,o->Angle,o->Light);
				}
				o->Live = false;
			}
        }
        else if( o->SubType==9 )
        {
    		CreateParticle(BITMAP_SMOKE,o->Position,o->Angle,o->Direction, 36, 1.0f+o->Scale, o );
    		CreateParticle(BITMAP_SMOKE,o->Position,o->Angle,o->Direction, 37, 2.0f+o->Scale, o );
			Vector(1.0f, 0.2f, 0.2f, o->Light);
			AddTerrainLight(o->Position[0],o->Position[1],o->Light,4,PrimaryTerrainLight);

			float Height = RequestTerrainHeight(o->Position[0],o->Position[1]);
			if(o->Position[2] < Height)
			{
				o->Live = false;
				break;
			}
        }

        if ( o->SubType==5 )    //  통통볼.
        {
			o->HiddenMesh = 0;
			o->BlendMeshLight = 0.f;

		    Vector(Luminosity*1.f,Luminosity*0.1f,Luminosity*0.f,Light);
		    AddTerrainLight(o->Position[0],o->Position[1],Light,2,PrimaryTerrainLight);
            if ( (rand()%5)!=0 )
            {
			    CreateParticle(BITMAP_POUNDING_BALL,o->Position,o->Angle,Light );
            }
        }
        else
        {
		    if(o->SubType==3)
		    {
			    o->HiddenMesh = 0;
			    o->BlendMeshLight = 0.f;
		    }
		    else if ( o->SubType==0 )
         	    o->BlendMeshLight = (float)(rand()%4+4)*0.1f;
		    else if ( o->SubType==6 || o->SubType==7 )
         	    o->BlendMeshLight = (float)(rand()%4+4)*0.1f;
		    else
         	    o->BlendMeshLight = 0.f;

			if ( o->SubType==6 || o->SubType==7 || o->SubType==8 )
			{
				Vector(Luminosity*0.1f,Luminosity*1.f,Luminosity*0.f,Light);
			}
			else
			{
				Vector(Luminosity*1.f,Luminosity*0.1f,Luminosity*0.f,Light);
			}
		    AddTerrainLight(o->Position[0],o->Position[1],Light,2,PrimaryTerrainLight);
			if ( o->SubType==6 || o->SubType==7 )
			{
				for ( int j=0; j<2; j++ )
				{
					Position[0] = o->Position[0]+rand()%50-25;
					Position[1] = o->Position[1];
					Position[2] = o->Position[2];
					CreateParticle(BITMAP_FIRE,Position,o->Angle,Light, 5);
				}
			}
			else if ( o->SubType != 3)
		    {
			    CreateParticle(BITMAP_FIRE,o->Position,o->Angle,Light, 5);
		    }
		    else
		    {
			    vec3_t Pos;
			    vec3_t vRot;
			    GetMagicScrew( iIndex * 5371, vRot);
			    VectorScale( vRot, 50.f, vRot);
			    VectorCopy( o->Position, Pos);
			    VectorAdd( Pos, vRot, Pos);

			    Vector( 0.0f, 0.0f, 0.0f, Angle);
			    Vector( 1.0f, 1.0f, 1.0f, Light);
			    CreateParticle(BITMAP_FIRE,Pos,Angle,Light, 8,o->Scale*2.5f);
			    CreateSprite(BITMAP_SHINY+1,Pos,o->Scale*3.0f,Light,NULL,(float)(rand()%360));
		    }
//                CreateParticle(BITMAP_SMOKE,o->Position,o->Angle,Light, 5);
            if(o->SubType==1)
		    {
			    CheckTargetRange(o);
			    /*if(CheckCharacterRange(o,100.f,o->PKKey))
			    {
				    o->Live = false;
				    for(int j=0;j<2;j++)
					    CreateEffect(MODEL_STONE1+rand()%2,o->Position,o->Angle,o->Light);
			    }*/
		    }
        }
		break;
	case MODEL_WARCRAFT:
		VectorCopy(o->Owner->Position,o->Position);
		break;
	case BITMAP_FIRECRACKERRISE:	// 발사대
		if ( 0 == ( o->LifeTime % 5) && ( 0 == ( rand() % 3)))
		{	// 폭죽 발사
			CreateEffect(BITMAP_FIRECRACKER,o->Position,o->Angle,o->Light);
		}
		break;
	case BITMAP_FIRECRACKER:
		//VectorAdd(o->Position,o->Direction,o->Position);
		if ( 1 == o->LifeTime)
		{	// 폭죽 터진다
			int iSubType = rand()%30;
			Vector((rand()%3)*.3f+.4f,(rand()%4)*.1f,.0f,Light);
			//for ( int j = 0; j < 200; ++j)
			for ( int j = 0; j < 80; ++j)
			{
				CreateParticle(BITMAP_FIRECRACKER,o->Position,o->Angle,Light,iSubType);
			}
			PlayBuffer(SOUND_FIRECRACKER2,o);
		}
		CreateParticle(BITMAP_FIRECRACKER,o->Position,o->Angle,o->Light,-1);
		Vector(Luminosity*.4f,Luminosity*0.3f,Luminosity*0.2f,Light);
		AddTerrainLight(o->Position[0],o->Position[1],Light,1,PrimaryTerrainLight);
		break;
#ifdef YDG_ADD_FIRECRACKER_ITEM
	case BITMAP_FIRECRACKER0001:	// 새 폭죽 발사대
		{
			if (o->LifeTime == 1 || o->LifeTime == 9 || o->LifeTime == 17
				|| o->LifeTime == 24 || o->LifeTime == 31)
			{
				Vector(o->Position[0]+(rand()%200-100), o->Position[1]+(rand()%200-100),
					o->Position[2], Position);
				CreateJoint(BITMAP_JOINT_SPIRIT, Position, Position, o->Angle, 25, o, 1.f, -1, o->SubType );
			}
		}
		break;
	case BITMAP_FIRECRACKER0002:	// 새 폭죽
		{
			if (rand()%5 == 0)
			{
				// 폭발 애니
				vec3_t vLight;
				Vector(0.3f+(rand()%700)*0.001f, 0.3f+(rand()%700)*0.001f, 0.3f+(rand()%700)*0.001f, vLight);
				Vector(o->Position[0]+(rand()%300-150), o->Position[1]+(rand()%300-150),
					o->Position[2], Position);
				float fScale = 0.5f+(rand()%5)*0.1f;
				CreateEffect(BITMAP_FIRECRACKER0003,Position,o->Angle,vLight, 0, o, -1, 0, 0, 0, fScale);
			}
		}
		break;
	case BITMAP_FIRECRACKER0003:	// 새 폭죽 폭발 애니
		{
			int iFrame;
			if (o->LifeTime > 15 - 8)
			{
				iFrame = (15 - o->LifeTime) / 8.0f * 7;
				if (iFrame >= 7) iFrame = 6;

				if (iFrame == 2)
				{
// 					// 번쩍
// 					CreateSprite(BITMAP_DS_SHOCK, o->Position, o->Scale*1.5f, o->Light, o);
				}
			}
			else
			{
				iFrame = 6;
				o->Position[2] -= 1.0f;
				o->Light[0] /= 1.05f;
				o->Light[1] /= 1.05f;
				o->Light[2] /= 1.05f;
				o->Scale *= 1.02f;
			}
			CreateSprite(BITMAP_FIRECRACKER0001+iFrame, o->Position, o->Scale, o->Light, o, o->Angle[2]);
		}
		break;
#endif	// YDG_ADD_FIRECRACKER_ITEM
    case BITMAP_SWORD_FORCE:
        if( o->LifeTime==30)
        {
            VectorCopy(o->Position,Position);
            Position[2] += 100.f;
#ifdef PJH_FIX_BLOOD_ATTCK
			if ( o->SubType==1 )
			{
				vec3_t Light;
				Vector(1.f,1.f,1.f,Light);
				CreateJoint(BITMAP_JOINT_FORCE,Position,Position,o->HeadAngle,10,o->Owner,150.f,o->PKKey,o->Skill, 0, -1,Light);
			}
			else
#endif //#ifdef PJH_FIX_BLOOD_ATTCK
			if( o->SubType==0 )
				CreateJoint(BITMAP_JOINT_FORCE,Position,Position,o->HeadAngle,0,o->Owner,150.f,o->PKKey,o->Skill);
			else
				CreateJoint(BITMAP_JOINT_FORCE,Position,Position,o->HeadAngle,8,o->Owner,150.f,o->PKKey,o->Skill);
        }
        break;

    case BITMAP_BLIZZARD:
        if( o->LifeTime<=15 )
        {
			if ( o->SubType==0 )
			{
				o->Position[0] = o->StartPosition[0] + sinf((rand()%1000)*0.01f)*10.f;
				o->Position[1] = o->StartPosition[1] + sinf((rand()%1000)*0.01f)*10.f;
				o->Position[2] += o->Gravity;
				o->Gravity -= 2.f;

				o->StartPosition[0] -= 10.f;

				CreateParticle(BITMAP_FIRE+2,o->Position,o->Angle,Light,7,o->Scale);

				o->Light[0]+= 0.1f;
				o->Light[1] = o->Light[0];
				o->Light[2] = o->Light[0];
				CreateSprite(BITMAP_SHINY+1,o->Position,(float)(rand()%4+4)*0.2f,o->Light,o,(float)(rand()%360));
				CreateSprite(BITMAP_LIGHT,o->Position,1.f,o->Light,o,(float)(rand()%360));
			}
			else if ( o->SubType==1 )
			{
				o->Position[0] = o->StartPosition[0];
				o->Position[1] = o->StartPosition[1];
				o->Position[2] += o->Gravity;
				o->Gravity -= 2.f;

				o->StartPosition[0] -= 10.f;

				CreateParticle(BITMAP_FIRE+2,o->Position,o->Angle,Light,11,o->Scale);

				o->Light[0]+= 0.1f;
				o->Light[1] = o->Light[0];
				o->Light[2] = o->Light[0];
				CreateSprite(BITMAP_SHINY+1,o->Position,(float)(rand()%4+4)*0.2f,o->Light,o,(float)(rand()%360));
				CreateSprite(BITMAP_LIGHT,o->Position,1.f,o->Light,o,(float)(rand()%360));

				Height = RequestTerrainHeight(o->Position[0],o->Position[1]);
			    if(o->Position[2] < Height)
                {
					Vector ( 0.24f, 0.28f, 0.8f, Light );
					VectorCopy ( o->Position, Position );
					Position[2] += 50.f;
    				CreateParticle(BITMAP_SMOKE,Position,o->Angle,Light, 11, (float)(rand()%32+80)*0.025f );
					
					if ( (rand()%5)==0 )
						CreateEffect(MODEL_ICE_SMALL,Position,o->Angle,o->Light);
                }
			}
        }
        break;

#ifdef TAMAJJANG
    case MODEL_TAMAJJANG:
        {
            //  0: 제자리.
            //  1: 걷는다 ( 목을 내밀고 )
            //  2: 걷는다 ( 목을 넣고 )
            //  3: 물속에 숨었다가 다시 나오고 반복.

            //  아틀란스.
            if ( World==WD_7ATLANSE )
            {
                o->LifeTime = 10;
            }
            //  로랜시아.
            else if ( World==WD_0LORENCIA )
            {
                if ( o->AI==0 )         //  제자리.
                {
                    if ( o->LifeTime<10 )
                    {
                        o->Gravity = 0.f;
                        o->AI = rand()%2+1;
                        SetAction ( o, o->AI );

                        if ( o->AI==2 )
                        {
                            Vector ( 0.f, -2.f, 0.f, o->Direction );
                        }
                        else
                        {
                            o->LifeTime = rand()%50+50;
                        }
                    }
                    else
                    {
                        o->Position[2] = o->HeadTargetAngle[2] + o->Gravity;// + sinf( WorldTime*0.001f )*5.f;
                        if ( o->Gravity<=5.f )
                        {
                            o->Gravity = 5.f;
                        }
                        else
                        {
                            o->Gravity--;
                        }

	                    Angle[2] = CreateAngle ( o->Position[0], o->Position[1], Hero->Object.Position[0], Hero->Object.Position[1] );
                        Angle[2] -= o->Angle[2];
                        if ( Angle[2]>5.f )
                        {
                            o->Angle[2] += min ( 5.f, Angle[2] );
                        }
                        else if ( Angle[2]<-5.f )
                        {
                            o->Angle[2] += max ( -5.f, Angle[2] );
                        }

                        if ( (rand()%100)==0 )
                        {
                            PlayBuffer ( SOUND_TAMAJJANG2 );
                        }
                    }
                }
                else if ( o->AI==1 )    //  제자리
                {
                    if ( o->LifeTime<10 )
                    {
                        o->Gravity = 40.f;
                        o->AI = 0;
                        SetAction ( o, o->AI );

                        if ( o->AI==2 )
                        {
                            Vector ( 0.f, -2.f, 0.f, o->Direction );
                        }
                        else
                        {
                            o->LifeTime = rand()%50+50;
                        }
                    }
                    else
                    {
                        o->Position[2] = o->HeadTargetAngle[2] + o->Gravity;
                        if ( o->Gravity>=40.f )
                        {
                            o->Gravity = 40.f;
                        }
                        else
                        {
                            o->Gravity++;
                        }

	                    Angle[2] = CreateAngle ( o->Position[0], o->Position[1], Hero->Object.Position[0], Hero->Object.Position[1] );
                        Angle[2] -= o->Angle[2];
                        if ( Angle[2]>5.f )
                        {
                            o->Angle[2] += min ( 5.f, Angle[2] );
                        }
                        else if ( Angle[2]<-5.f )
                        {
                            o->Angle[2] += max ( -5.f, Angle[2] );
                        }

                        if ( (o->LifeTime%(rand()%20+10))==0 )
                        {
                            PlayBuffer ( SOUND_TAMAJJANG1 );
                        }
                    }
                }
                else if ( o->AI==2 )    //  걷는다.
                {
                    o->LifeTime = 10;

                    float Distance;
	                float dx = o->Position[0]-o->StartPosition[0];
	                float dy = o->Position[1]-o->StartPosition[1];

                    Distance = sqrtf ( dx*dx + dy*dy );

                    MoveHumming ( o->Position, o->Angle, o->StartPosition,3000.f/Distance);
                    o->Angle[0] = -10.f;
                    o->Angle[1] = 0.f;
                    o->Position[2] = RequestTerrainHeight ( o->Position[0], o->Position[1] ) - 10.f;
                    if ( Distance <= 10.f )
                    {
                        vec3_t  Angle;

                        Vector ( 0.f, -150.f-rand()%150, 0.f, p );
                        Vector ( 0.f, 0.f, 0.f, Angle );
                        Angle[2] = 180.f + rand()%120-60.f;

			            AngleMatrix ( Angle, Matrix );
			            VectorRotate ( p, Matrix, Position );
			            VectorAdd ( o->HeadTargetAngle, Position, o->StartPosition );

                        o->LifeTime = rand()%100+50;
                        o->AI = rand()%3;
                        SetAction ( o, o->AI );

                        if ( o->AI!=2 )
                        {
                            Vector ( 0.f, 0.f, 0.f, o->Direction );
                        }
                    }

                    if ( (rand()%3)==0 )
                    {
                        VectorCopy ( o->Position, Position );
                        Position[2] += 11.f;
				        CreateParticle ( BITMAP_RAIN_CIRCLE, Position, o->Angle, o->Light, 1, 2.f );
                    }

                    PlayBuffer ( SOUND_HUMAN_WALK_SWIM );
                }
                else if ( o->AI==3 )    //  사라진다.
                {
                    o->Position[2] -= 5.f;
                }
            }
            else
            {
                o->LifeTime = -1;
                o->Live = 0;
            }
        }
        break;
#endif
    case BITMAP_SHOTGUN:
        {
            Vector(Luminosity*0.5f,Luminosity*0.5f,Luminosity*0.8f,Light);
		    AddTerrainLight(o->Position[0],o->Position[1],Light,2,PrimaryTerrainLight);

            Vector ( 1.f, 1.f, 1.f, Light );
	        VectorCopy ( o->Angle, Angle );
            Angle[2] += 90.f;

            o->Scale = ((15-o->LifeTime)/20.f)*(rand()%3+2);

		    AngleMatrix ( Angle, Matrix );
		    Vector(0.f,20.f,0.f,p);
		    VectorRotate ( p, Matrix, Position );
		    VectorAdd ( o->Position, Position, o->StartPosition );
            CreateParticle ( BITMAP_FIRE+2, o->StartPosition, o->Angle, Light, 10, o->Scale );

		    Vector(0.f,-20.f,0.f,p);
		    VectorRotate ( p, Matrix, Position );
		    VectorAdd ( o->Position, Position, o->StartPosition );
            CreateParticle ( BITMAP_FIRE+2, o->StartPosition, o->Angle, Light, 10, o->Scale );

            if ( o->LifeTime==1 )
            {
				CreateBomb2 ( o->Position, false );
            }
        }
        break;

    case MODEL_GATE:    //  블러드캐슬 성문.
    case MODEL_GATE+1:
		VectorAdd(o->Position,o->Direction,o->Position);
		VectorScale(o->Direction,0.9f,o->Direction);
		o->Position[2] += o->Gravity;

        if ( o->SubType==1 )
        {
		    VectorScale(o->Direction,1.1f,o->Direction);
        }

		o->Gravity -= 4.f;
		Height = RequestTerrainHeight(o->Position[0],o->Position[1]);
		if(o->Position[2] < Height)
		{
			o->Position[2] = Height;
			o->Gravity = -o->Gravity*0.5f;
			o->LifeTime -= 5;
			o->Angle[0] -= o->Scale*128.f;
		}
		else
			o->Angle[0] -= o->Scale*16.f;

        if ( (rand()%10)==0 )
        {
		    CreateParticle(BITMAP_SMOKE+1,Position,o->Angle,Light);
        }
        break;

    case MODEL_STONE_COFFIN:    //  블러드캐슬 성자의 석상.
    case MODEL_STONE_COFFIN+1:
        VectorAdd(o->Position,o->Direction,o->Position);
        VectorScale(o->Direction,0.9f,o->Direction);
        o->Position[2] += o->Gravity;
        
        if ( o->SubType==1 )
        {
            VectorScale(o->Direction,1.1f,o->Direction);
        }
        
        o->Gravity -= 3.f;
        Height = RequestTerrainHeight(o->Position[0],o->Position[1]);
        if(o->Position[2] < Height)
        {
            o->Position[2] = Height;
            o->Gravity = -o->Gravity*0.2f;
            o->LifeTime -= 5;
            o->Angle[0] -= o->Scale*128.f;
        }
        else
            o->Angle[0] -= o->Scale*16.f;
        
        o->Alpha = o->LifeTime/10.f;
        if ( (rand()%10)==0 )
        {
            CreateParticle(BITMAP_SMOKE+1,Position,o->Angle,Light);
        }
        break;
    case MODEL_SHINE:
        if ( o->SubType==0 && o->Owner!=NULL && o->LifeTime>10 )
        {
			Position[0] = (float)(rand()%128-64)-50.f;
            Position[1] = Position[0]+100.f;
            Position[2] = 0.f;
			VectorAdd(Position,o->Owner->Position,Position);
            Position[2] += 360.f;

            float Scale = (rand()%30+50) / 100.f;
       		CreateParticle(BITMAP_SHINY,Position,o->Angle,o->Light,2,Scale);
        }
        break;
    case MODEL_BLIZZARD:
		if ( o->SubType==0 )
        {
			o->Position[0] = o->StartPosition[0] + sinf((rand()%1000)*0.01f)*10.f;
			o->Position[1] = o->StartPosition[1] + sinf((rand()%1000)*0.01f)*10.f;
			o->Position[2] += o->Gravity;
			o->Gravity -= rand()%5;

			o->StartPosition[0] -= 10.f;

			CreateParticle(BITMAP_SMOKE,o->Position,o->Angle,Light,0, 1.5f);
			if ( (rand()%2)==0 )
			{
				CreateParticle(BITMAP_ENERGY,o->Position,o->Angle,Light,1,0.5f);
			}
			else
			{
				CreateParticle(BITMAP_FIRE+2,o->Position,o->Angle,Light,7,o->Scale);
			}

			o->Light[0]+= 0.1f;
			o->Light[1] = o->Light[0];
			o->Light[2] = o->Light[0];
			CreateSprite(BITMAP_SHINY+1,o->Position,(float)(rand()%4+4)*0.2f,o->Light,o,(float)(rand()%360));
			CreateSprite(BITMAP_LIGHT,o->Position,1.f,o->Light,o,(float)(rand()%360));

			Height = RequestTerrainHeight(o->Position[0],o->Position[1]);
			if(o->Position[2] < Height)
            {
				Vector ( 0.24f, 0.28f, 0.8f, Light );
				VectorCopy ( o->Position, Position );
				Position[2] += 50.f;
    			CreateParticle(BITMAP_SMOKE,Position,o->Angle,Light, 11, (float)(rand()%32+80)*0.025f );
				
				if ( (rand()%5)==0 )
				{
					CreateEffect(MODEL_ICE_SMALL,Position,o->Angle,o->Light);
				}

				CreateEffect ( MODEL_BLIZZARD, Position, o->Angle, o->Light, 1, NULL, o->PKKey );

				o->Live = false;
            }
        }
		else if ( o->SubType==1 )
		{
			o->BlendMeshLight /= 1.1f;

			if ( o->LifeTime==18 && o->PKKey==1 )
				PlayBuffer(SOUND_SUDDEN_ICE2);
		}
        else if ( o->SubType==2 )
        {
			o->Position[0] = o->StartPosition[0] + sinf((rand()%1000)*0.01f)*50.f;
			o->Position[1] = o->StartPosition[1] + sinf((rand()%1000)*0.01f)*50.f;
			o->Position[2] += o->Gravity;
			o->Gravity -= rand()%5;

			o->StartPosition[0] -= 10.f;

			Height = RequestTerrainHeight(o->Position[0],o->Position[1]);
			if ( o->Position[2]>Height )
            {
                if (  (rand()%2)==0 )
                {
                    CreateParticle(BITMAP_FIRE+2,o->Position,o->Angle,Light,7,o->Scale);
                }

                o->Light[0]+= 0.1f;
			    o->Light[1] = o->Light[0];
			    o->Light[2] = o->Light[0];
			    CreateSprite(BITMAP_SHINY+1,o->Position,(float)(rand()%4+4)*0.2f,o->Light,o,(float)(rand()%360));
			    CreateSprite(BITMAP_LIGHT,o->Position,1.f,o->Light,o,(float)(rand()%360));
            }
        }
        break;

	case MODEL_ARROW_DRILL:
		if ( o->SubType==0 || o->SubType==2 )
		{
			o->Angle[1] += 30.f;

		    CreateParticle(BITMAP_SMOKE,o->Position,o->Angle,Light,13);
		    CreateParticle(BITMAP_SMOKE,o->Position,o->Angle,Light,13);
		    CreateParticle(BITMAP_SMOKE,o->Position,o->Angle,Light,13);

			if(o->LifeTime == 1)
			{
				CreateBomb(o->Position,true);
			}

			VectorCopy(o->Position,o->EyeLeft);
			VectorCopy(o->Angle,Angle);
			AngleMatrix(Angle,Matrix);
			VectorRotate(o->Direction,Matrix,Position);
			VectorAdd(o->EyeLeft,Position,o->EyeLeft);

			Vector(Luminosity*1.f,Luminosity*0.4f,Luminosity*0.2f,Light);
			AddTerrainLight(o->Position[0],o->Position[1],Light,2,PrimaryTerrainLight);
			CheckClientArrow(o);
		}
        break;

	case MODEL_COMBO:
		if ( o->SubType==0 )
		{
		    if ( o->LifeTime>4 )
		    {
			    o->Scale += o->Gravity;
			    o->Gravity+= 0.1f;
		    }
		    o->BlendMeshLight /= 1.4f;
		}
		break;
	case MODEL_WAVES:
		if ( o->SubType==0 )
		{
		    if ( o->LifeTime>4 )
		    {
			    o->Scale += o->Gravity;
			    o->Gravity+= 0.1f;
		    }
		    o->BlendMeshLight /= 1.4f;
		}
		else if ( o->SubType==1 )       //  스트롱 피어.
		{
			o->Scale += o->Gravity;
			o->Gravity+= 0.07f;
            if ( o->Scale>2.f ) o->Scale = 2.f;
		    o->BlendMeshLight /= 1.5f;
		}
		else if ( o->SubType==2 )
		{
			o->Scale += o->Gravity;
			o->Gravity+= 0.01f;
            if ( o->Scale>1.5f ) o->Scale = 1.5f;
		    o->BlendMeshLight /= 1.5f;
		}
		else if ( o->SubType==3 )
		{
			o->Scale += o->Gravity;
			o->Gravity+= 0.005f;
            if ( o->Scale>1.5f ) o->Scale = 1.5f;
		    o->BlendMeshLight /= 1.3f;
            o->Angle[1] += 45.f;
		}
		else if ( o->SubType==4 )
		{
			o->Scale += o->Gravity;
			o->Gravity+= 0.002f;
            if ( o->Scale>2.5f ) o->Scale = 2.5f;
		    o->BlendMeshLight /= 1.2f;
            o->Angle[1] += 45.f;
		}
		else if ( o->SubType==5 )
		{
			o->Scale += o->Gravity;
			o->Gravity+= 0.015f;
            if ( o->Scale>2.5f ) o->Scale = 2.5f;
		    o->BlendMeshLight /= 1.3f;
            o->Angle[1] += 45.f;
		}
		else if ( o->SubType==6 )
		{
			o->Scale += o->Gravity;
			o->Gravity+= 0.015f;
            if ( o->Scale>2.5f ) o->Scale = 2.5f;
		    o->BlendMeshLight /= 1.3f;
            o->Angle[1] += 45.f;
		}
		break;
	case MODEL_AIR_FORCE:	    //  공기 저항 효과.
        if ( o->Owner==NULL )
        {
            o->Live = false;
            return;
        }
        if ( o->SubType==0 )
        {
            o->BlendMeshLight /= 1.5f;
            o->Scale += 0.2f;
        }

        if ( o->SubType==1 )
        {
            o->BlendMeshLight = o->LifeTime/10.f;

            VectorCopy ( o->Owner->Position, o->Position );
            VectorCopy ( o->Owner->Angle, o->Angle );
            Vector ( 0.f, -70.f, 0.f, p );
			AngleMatrix ( o->Angle, Matrix );
            VectorRotate ( p, Matrix, Position );
            Position[2] += 100.f;
            VectorAdd ( o->Position, Position, o->Position );
        }
		break;
    case MODEL_PIERCING2:         //  스트롱 피어.
		AngleMatrix(o->Angle,Matrix);
		VectorRotate(o->Direction,Matrix,Position);
//		VectorAdd(o->Position,Position,o->Position);

        o->Direction[1] += 12.f;
        if ( o->Direction[1]>=0 ) o->Direction[1] =0;

		if(o->SubType == 1) {
			if ( o->LifeTime>1 ) {
				o->BlendMeshLight /= 1.6f;
				CreateEffect ( MODEL_WAVES, o->Position, o->Angle, o->Light, 2, NULL, o->LifeTime );
			}
		}
#ifdef PJH_SEASON4_SPRITE_NEW_SKILL_MULTI_SHOT
		else
		if(o->SubType == 2) 
		{
//			o->Direction[1] -= 10.f;
			if ( o->LifeTime>1 ) 
			{
				o->BlendMeshLight /= 1.6f;
				CreateEffect ( MODEL_WAVES, o->Position, o->Angle, o->Light, 2, NULL, o->LifeTime );
			}
		}
#endif //PJH_SEASON4_SPRITE_NEW_SKILL_MULTI_SHOT
		else {	
			if ( o->LifeTime>5 ) {
				o->BlendMeshLight /= 1.6f;
				CreateEffect ( MODEL_WAVES, o->Position, o->Angle, o->Light, 2, NULL, o->LifeTime );
			}
		}
        break;
#ifdef LDK_ADD_EG_MONSTER_DEASULER
	case MODEL_DEASULER:
		{
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
				float	fEPSILON = 0.000001f;					// 미세값.	
				float	fRateAlpha_EraseOver = 0.7f;			// 알파 적용 시점.
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
				vec3_t	v3RotateAngleRelative;
				o->Visible = true;		// MoveEffect에서 CreateEffect시 간혹 Move를 타지 않고 그린는 것을 방지.

				// 2. 프레임 RATE %
				float	fCurrentRate = 1.0f - ((float)o->LifeTime / (float)o->ExtState);

				// 3. 보간 반영.
				// ========================================================================================>
				// 4. (1) 보간 반영. - Angle
				if( o->m_Interpolates.m_vecInterpolatesAngle.size() > 0 )
				{
					o->m_Interpolates.GetAngleCurrent(v3RotateAngleRelative ,fCurrentRate);		
					
					o->Angle[0] = v3RotateAngleRelative[0] + o->HeadAngle[0];
					o->Angle[1] = v3RotateAngleRelative[1] + o->HeadAngle[1];
					o->Angle[2] = v3RotateAngleRelative[2] + o->HeadAngle[2];
				}
				
				// 6. (2) 보간 반영. - Position
				if( o->m_Interpolates.m_vecInterpolatesPos.size() > 0 )
				{
					o->m_Interpolates.GetPosCurrent(o->Position,fCurrentRate);
				}

				// 8. (3) 보간 반영. - Scale
				if( o->m_Interpolates.m_vecInterpolatesScale.size() > 0 )
				{
					o->m_Interpolates.GetScaleCurrent(o->Scale,fCurrentRate);
				}

				// 9. (4) 보간 반영. - Alpha
				if( o->m_Interpolates.m_vecInterpolatesScale.size() > 0 )
				{
					o->m_Interpolates.GetAlphaCurrent(o->Alpha,fCurrentRate);
				}
				// 3. 보간 반영.
				// ========================================================================================<


				// 검기 출력 정의.
				// ========================================================================================>

				// 검기 범위 정의.
				float fRateBlurStart, fRateBlurEnd, fRateShadowStart, fRateShadowEnd, fRateJointStart, fRateJointEnd;
				fRateBlurStart = fRateBlurEnd = 0.0f;
				fRateShadowStart = fRateShadowEnd = 0.0f;
				fRateJointStart = fRateJointEnd = 0.0f;
				fRateBlurStart = 0.0f; fRateBlurEnd = 0.90f;

				// 검기 범위 정의.

				// 검기 테스트용
				int		iTYPESWORDFORCE = 0;		// 1: FORCE OF SWORD 
				int		iTYPESWORDSHADOW = 0;		// 1: SHADOW SWORD 
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
				int		iTYPESWORDJOINT = 0;		// 1: JOINT OF SWORD
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX

				iTYPESWORDFORCE = 1;
				iTYPESWORDSHADOW = 0;

				// 실 검기 EFFECT 정의.
				
				if( iTYPESWORDFORCE==1 )
				{
					if( fCurrentRate > fRateBlurStart && fCurrentRate < fRateBlurEnd )
					{
						BMD*	b = &Models[o->Type];
						vec3_t  vLightBlur; Vector(1.0f, 1.0f, 1.0f, vLightBlur);
						float	fPreRate = 1.0f - (float)((o->LifeTime)+1)/(float)(o->ExtState);
						SETLIMITS( fPreRate, 1.0f, 0.0f );
						if( fPreRate < fCurrentRate )
						{
							for(int Loop_bk = 0; Loop_bk<2; Loop_bk++)
							{
							float	fStartRate, fEndRate;						
							
							fStartRate = 1.0f - (float)((o->LifeTime)+2)/(float)(o->ExtState);
							fEndRate = 1.0f - (float)((o->LifeTime)+1)/(float)(o->ExtState);
							

							SETLIMITS( fStartRate, 1.0f, 0.0f );
							SETLIMITS( fEndRate, 1.0f, 0.0f );

							vec3_t	*arrEachBonePos;	// 현재 모델의 각 Bone들의 위치값.
							arrEachBonePos		= new vec3_t[b->NumBones];							
							
							vec3_t v3CurBlurAngle, v3CurBlurPos;
							int	iAccess = 20;
							int iBone01, iBone02;
							int iBlurIdentity, iTypeBlur;
							float fUnit;
							float fCurrentRateUnit = fStartRate;
							float fScale = o->Scale;

							if(Loop_bk == 0)
							{
								iBone01 = 4;
								iBone02 = 2;
								iBlurIdentity = 113;				// 임의의 검기 식별자를 부여.
							}
							else
							{
								iBone01 = 5;
								iBone02 = 1;
								iBlurIdentity = 114;				// 임의의 검기 식별자를 부여.
							}
							iTypeBlur = 10;


							fUnit = (fEndRate - fStartRate) / (float)iAccess;

								
							for(int i=0;i<iAccess;i++) 
									{
										fCurrentRateUnit += fUnit;	
										
										o->m_Interpolates.GetAngleCurrent(v3CurBlurAngle, fCurrentRateUnit);
										o->m_Interpolates.GetPosCurrent(v3CurBlurPos, fCurrentRateUnit);
										
										VectorAdd(v3CurBlurAngle, o->HeadAngle, v3CurBlurAngle);
										
										b->AnimationTransformOnlySelf( 
											arrEachBonePos, v3CurBlurAngle, v3CurBlurPos, fScale );
										
										CreateObjectBlur(o, 
											arrEachBonePos[iBone01], 
											arrEachBonePos[iBone02], 
											vLightBlur, iTypeBlur, false, iBlurIdentity, 20);
										
										if( rand()%10==0 )
										{
											vec3_t	vAngle, vRandomDir, vRandomDirPosition, vResultRandomPosition;
											vec34_t	matRandomRotation;
											vec3_t	vPosition;
											
											VectorCopy( arrEachBonePos[1], vPosition );
									
											float	fRandDistance = (float)(rand() % 100) + 100;
											Vector(0.0f, fRandDistance, 0.0f,vRandomDir);
											
											CreateParticle(BITMAP_FIRE+2,vPosition,o->Angle,o->Light,17, 1.35f);
											
											// 효과 : 난수지역에 번개효과
											Vector((float)(rand()%360),0.f,(float)(rand()%360),vAngle);
											AngleMatrix(vAngle,matRandomRotation);
											VectorRotate(vRandomDir,matRandomRotation,vRandomDirPosition);
											VectorAdd(vPosition,vRandomDirPosition,vResultRandomPosition);
											CreateJoint(BITMAP_JOINT_THUNDER,vResultRandomPosition,vPosition,vAngle,3,NULL,10.f,10,10);
										}
										
									}
								delete [] arrEachBonePos;
							}
						}	// if( fPreRate < fCurrentRate )
					}
				}
		}
		break;
#endif //LDK_ADD_EG_MONSTER_DEASULER
		case MODEL_DEATH_SPI_SKILL:
		{
			if(o->SubType == 0)
			{
				if ( o->Owner!=NULL )
				{
					VectorCopy ( o->Owner->Position, p );
					VectorAdd ( p, o->StartPosition, p );

					float Distance;
					for ( int i=1; i<o->Gravity; ++i )
					{
						if ( rand()%2==0 )
						{
							if ( o->Angle[0]<-90 )
								o->Angle[0] += 20.f;//rand()%10+5.f;
							else
								o->Angle[0] -= 20.f;//rand()%10+5.f;
						}
						Distance = MoveHumming ( o->Position, o->Angle, p, o->Velocity );
						o->Velocity += 0.4f;
                
						if ( o->LifeTime<10 )
						{
							o->Velocity += 0.1f;
							CreateEffect ( BITMAP_MAGIC+1, Position, o->Angle, o->Light, 1, o );
						}

						AngleMatrix(o->Angle,Matrix);
						VectorRotate(o->Direction,Matrix,Position);
						VectorAdd(o->Position,Position,o->Position);

						CreateEffect ( MODEL_TAIL, o->Position, o->Angle, o->Light, 0, o );

					}
					if ( Distance<40 && o->LifeTime==5 )
					{
						VectorCopy ( o->Position, Position );
						Position[2] = RequestTerrainHeight ( o->Position[0], o->Position[1] );
					}
					o->Gravity += 0.1f;

					PlayBuffer ( SOUND_ATTACK_FIRE_BUST_EXP );
				}	
			}
			else if ( o->SubType==1 )
			{
				if ( o->LifeTime<5 )
				{
					o->Alpha /= 1.3f;
				}
			}
		}
		break;
    case MODEL_PIER_PART:       //  롱피어 어택.
        if ( o->SubType==0 )
        {
            if ( o->Owner!=NULL )
            {
                VectorCopy ( o->Owner->Position, p );
                VectorAdd ( p, o->StartPosition, p );

                float Distance;
                for ( int i=1; i<o->Gravity; ++i )
                {
                    if ( rand()%2==0 )
                    {
                        if ( o->Angle[0]<-90 )
                            o->Angle[0] += 20.f;//rand()%10+5.f;
                        else
                            o->Angle[0] -= 20.f;//rand()%10+5.f;
                    }
                    Distance = MoveHumming ( o->Position, o->Angle, p, o->Velocity );
                    o->Velocity += 0.4f;
                    
                    if ( o->LifeTime<10 )
                    {
                        o->Velocity += 0.1f;
                    }

		            AngleMatrix(o->Angle,Matrix);
		            VectorRotate(o->Direction,Matrix,Position);
		            VectorAdd(o->Position,Position,o->Position);

                    CreateEffect ( MODEL_PIER_PART, o->Position, o->Angle, o->Light, 1, o );

                }
                if ( Distance<40 && o->LifeTime==5 )
                {
                    VectorCopy ( o->Position, Position );
                    Position[2] = RequestTerrainHeight ( o->Position[0], o->Position[1] );
                }
                o->Gravity += 0.1f;

                PlayBuffer ( SOUND_ATTACK_FIRE_BUST_EXP );
            }
        }
        else if ( o->SubType==2 )
        {
            if ( o->Owner!=NULL )
            {
                VectorCopy ( o->Owner->Position, p );

#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
				MoveHumming ( o->Position, o->Angle, p, o->Velocity );
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
                float Distance = MoveHumming ( o->Position, o->Angle, p, o->Velocity );
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
                o->Velocity += 2.4f;
                
		        AngleMatrix(o->Angle,Matrix);
		        VectorRotate(o->Direction,Matrix,Position);
		        VectorAdd(o->Position,Position,o->Position);

                if ( o->LifeTime%3==0 )
                {
                    Vector ( -90.f, 0.f, o->Angle[2], Angle );
                    CreateJoint ( BITMAP_JOINT_FORCE, o->Position, o->Position, Angle, 2, NULL, 150.f );
                }
            }
        }
        else if ( o->SubType==1 )
        {
            if ( o->LifeTime<5 )
            {
                o->Alpha /= 1.3f;
            }
        }
        break;

    case BITMAP_FLARE_FORCE :   
        break;

    case MODEL_DARKLORD_SKILL:
        if ( o->SubType<=1 )
        {
            o->Scale += o->Velocity;
            o->Velocity += 0.02f;
            if ( o->LifeTime<7 )
            {
                o->BlendMeshLight /= 1.8f;
            }
        }
        else if ( o->SubType==2 )
        {
            o->Scale += o->Velocity;
            o->Velocity -= 0.06f;
            if ( o->LifeTime<8 )
            {
                o->BlendMeshLight /= 1.5f;
            }
        }
        else
        {
            o->Scale += o->Velocity;
            o->Velocity += 0.04f;
            if ( o->LifeTime<8 )
            {
                o->BlendMeshLight /= 1.8f;
            }
        }
        break;
    case MODEL_GROUND_STONE:
    case MODEL_GROUND_STONE2:
        if ( o->LifeTime>32 && o->LifeTime<37 )
        {
            Position[0] = o->Position[0] + 60;
            Position[1] = o->Position[1] - 60;
            Position[2] = o->Position[2] + 50.f;

            Vector ( 1.f, 0.8f, 0.6f, Light );
			CreateParticle ( BITMAP_SMOKE, Position, o->Angle, Light, 11, 2.f, o );
            CreateEffect ( MODEL_STONE1+rand()%2, Position, o->Angle, o->Light, 10 );
        }
//        o->BlendMeshTexCoordU = -(int)WorldTime%2000 * 0.0005f;

        if ( o->LifeTime<8 )
        {
            o->Alpha /= 1.3f;
            o->BlendMeshLight /= 2.f;
        }

        Vector ( 0.79f, 0.72f, 0.49f, Light );
        AddTerrainLight ( o->Position[0], o->Position[1], Light, 2, PrimaryTerrainLight );
        break;

//------------------------------------------------------------------------------------------------------------------------
#ifdef PJH_SEASON4_SPRITE_NEW_SKILL_RECOVER
	case BITMAP_TWLIGHT:
		{
			if(o->SubType == 0)
			{
				o->Scale -= 0.1f;
				o->Angle[2] += 10.f;
			}
			else
			if(o->SubType == 1)
			{
				o->Scale -= 0.1f;
				o->Angle[2] += 5.f;
			}
			else
			if(o->SubType == 2)
			{
				o->Scale -= 0.1f;
				o->Angle[2] += 15.f;
			}
			else if( o->SubType == 3 )		// 마력증대(마법사) 스킬에서 사용
			{
				// o->PKKey : 칼라비율
				// o->EyeRight : 기준칼라
				VectorCopy( o->Owner->Position, o->Position );
				o->Scale -= 0.15f;
				o->Angle[2] += 10.f;
				if( o->LifeTime >= 20 )
				{
					o->Alpha += 0.1f;
					o->PKKey++;
					o->Light[0] = o->EyeRight[0] * (o->PKKey*0.1f);
					o->Light[1] = o->EyeRight[1] * (o->PKKey*0.1f);
					o->Light[2] = o->EyeRight[2] * (o->PKKey*0.1f);
					
				}
				else if( o->LifeTime <= 10 )
				{
					o->PKKey--;
					o->Alpha -= 0.1f;
					o->Light[0] = o->EyeRight[0] * (o->PKKey*0.1f);
					o->Light[1] = o->EyeRight[1] * (o->PKKey*0.1f);
					o->Light[2] = o->EyeRight[2] * (o->PKKey*0.1f);
					
				}
			}
		}
		break;
#endif //PJH_SEASON4_SPRITE_NEW_SKILL_RECOVER
    case BITMAP_SHOCK_WAVE:
        if ( o->SubType==0 )
        {
            o->Scale -= 1.f;
        }
        else if ( o->SubType==1 )
        {
            o->Scale += (rand()%5)/10.f;

            o->Position[0] += rand()%8-4.f;
            o->Position[1] += rand()%8-4.f;
        }
        else if ( o->SubType==2 )
        {
            o->Scale += (rand()%5)/40.f;

            o->Position[0] += rand()%8-4.f;
            o->Position[1] += rand()%8-4.f;
        }
        else if ( o->SubType==3 )
        {
            o->Scale += 2.f;
        }
        else if ( o->SubType==4 )
        {
            o->Scale += 0.3f;
        }
        else if ( o->SubType==5 )
        {
            o->Scale -= 0.4f;
        }
        else if ( o->SubType==6 )
        {   
            if ( (o->LifeTime%8)==0 )
            {
                CreateEffect ( BITMAP_SHOCK_WAVE, o->Position, o->Angle, o->Light, 5 );
            }
            break;
        }
        else if ( o->SubType==7 )
        {
            o->Scale += 2.5f;
        }
		else if ( o->SubType==8 )
        {
            o->Scale += 1.f;
        }
		else if ( o->SubType==9 )
		{
			o->Scale += 0.8f;
			VectorCopy(o->Owner->Position, o->Position);
		}
		else if ( o->SubType==10 )
        {
			o->Scale -= 0.02f;
			VectorCopy(o->Owner->Position, o->Position);
        }
		else if( o->SubType == 11 )
		{
			o->Scale += (rand()%5)/10.f;

            o->Position[0] += rand()%8-4.f;
            o->Position[1] += rand()%8-4.f;
		}
		else if( o->SubType == 12 )
		{
			if (o->LifeTime > 4.0f)
				o->Scale += (o->LifeTime - 4.0f) * 0.25f;
		}
#ifdef PJH_GIANT_MAMUD
		else if(o->SubType == 13)
		{
			o->Scale += 0.08f;
		}
#endif //#ifdef PJH_GIANT_MAMUD
#ifdef KJH_ADD_SKILL_SWELL_OF_MAGICPOWER
		else if( o->SubType == 14 )
		{
			// o->PKKey : 칼라비율
			// o->EyeRight : 기준칼라
			VectorCopy( o->Owner->Position, o->Position );
			o->Scale -= 0.15f;
			if( o->LifeTime >= 20 )
			{
				o->Alpha += 0.1f;
				o->PKKey++;
				o->Light[0] = o->EyeRight[0] * (o->PKKey*0.1f);
				o->Light[1] = o->EyeRight[1] * (o->PKKey*0.1f);
				o->Light[2] = o->EyeRight[2] * (o->PKKey*0.1f);
				
			}
			else if( o->LifeTime <= 10 )
			{
				o->PKKey--;
				o->Alpha -= 0.1f;
				o->Light[0] = o->EyeRight[0] * (o->PKKey*0.1f);
				o->Light[1] = o->EyeRight[1] * (o->PKKey*0.1f);
				o->Light[2] = o->EyeRight[2] * (o->PKKey*0.1f);
				
			}
			break;
		}
#endif // KJH_ADD_SKILL_SWELL_OF_MAGICPOWER

        if ( o->Scale<0 )
        {
            o->Scale = 0;
        }
        if ( o->SubType>=0 && o->SubType<=3 )
        {
            if ( o->LifeTime<=20 )
            {
                Luminosity = o->LifeTime/20.f;
                Vector ( Luminosity, Luminosity, Luminosity, o->Light );
            }
            else
            {
                Luminosity = (40-o->LifeTime)/20.f;
                Vector ( Luminosity, Luminosity, Luminosity, o->Light );
            }
        }
        else
        {
            if ( o->LifeTime<6 )
            {
                o->Light[0] /= 1.3f;
                o->Light[1] /= 1.3f;
                o->Light[2] /= 1.3f;
            }
        }
        break;

//------------------------------------------------------------------------------------------------------------------------

#ifdef CSK_RAKLION_BOSS
		case BITMAP_DAMAGE_01_MONO:
			if(o->SubType == 0)
			{
				o->Scale += 5.f;
			}
#ifdef YDG_ADD_SKILL_LIGHTNING_SHOCK
			else if(o->SubType == 1)
			{
				o->Scale += 0.5f;
				if (o->Scale > 3.5f)
				{
					//o->Scale = 6.0f;

					o->Light[0] *= 0.5f;
					o->Light[1] *= 0.5f;
					o->Light[2] *= 0.5f;
				}

			}
#endif	// YDG_ADD_SKILL_LIGHTNING_SHOCK
			break;
#endif // CSK_RAKLION_BOSS

        case BITMAP_FLARE:
			if (o->SubType == 1)
			{
				CreateParticle(BITMAP_FLARE,o->Position,o->Angle,Light,11,1.0f);
			}
			else if (o->SubType == 2)
			{
				CreateParticle(BITMAP_FLARE_BLUE,o->Position,o->Angle,Light,1,1.0f);
			}
			else
#ifdef PJH_SEASON4_SPRITE_NEW_SKILL_RECOVER
			if (o->SubType == 3)
			{
				Vector( 0.9f, 0.4f, 0.1f, Light );
				float fRandom = 3.5f + ((float)(rand()%20-10)*0.1f);	
				CreateParticle( BITMAP_LIGHT, o->Position,o->Angle,Light, 5, fRandom);
			}
			else
#endif //PJH_SEASON4_SPRITE_NEW_SKILL_RECOVER
            if ( o->LifeTime%2==0 )
            {
                VectorCopy ( o->Position, Position );
                Position[2] += 100.f;
                CreateJoint ( BITMAP_FLARE_BLUE, Position, Position, o->Angle, 14, o, 40.f );
                CreateJoint ( BITMAP_FLARE_BLUE, Position, Position, o->Angle, 15, o, 40.f );

                if ( (rand()%2)==0 && o->LifeTime>5 && o->LifeTime<15 )
                {
			        Vector(o->Position[0]-200.f,o->Position[1],o->Position[2]+700.f,Position);
			        CreateJoint(BITMAP_JOINT_THUNDER,Position,o->Position,Angle,0,o,20.f);
                }
            }
            break;

//------------------------------------------------------------------------------------------------------------------------

        case BITMAP_JOINT_THUNDER+1:
            if ( o->LifeTime<5 && o->LifeTime%2==0 && o->SubType==0 )
            {
                VectorCopy ( o->Position, Position );

                Position[0] += rand()%64-32.f;
                Position[1] += rand()%64-32.f;

                CreateJoint ( BITMAP_JOINT_THUNDER+1, Position, Position, o->Angle, 5, NULL, 50.f+rand()%10 );
            }
            break;

		case MODEL_CUNDUN_DRAGON_HEAD:
			o->Angle[2] += 10 + rand() % 10;

			if (o->Position[2] > 300 && o->Position[2] < 600)
			{
				vec3_t Angle;
				Angle[0] = Angle[1] = 0;
				Position[0] = o->Position[0];
				Position[1] = o->Position[1];
				Position[2] = 350;
				if (rand()%2 == 0)
				{
					Angle[2] = ( float)(rand()%24*30);
					CreateJoint(BITMAP_JOINT_SPIRIT2,Position,Position,Angle,14,NULL,100.f,0,0);
				}
			}
			break;
		case MODEL_CUNDUN_PHOENIX:
			if (o->LifeTime < 5)
			{
				o->Alpha = o->LifeTime * 0.2f;
			}
			break;
		case MODEL_CUNDUN_GHOST:
			if (o->Owner != NULL)
			{
				if (o->Owner->PKKey == 0)
				{
					o->AnimationFrame = 0;
				}
				else
				{
					o->Owner = NULL;
				}
			}
			else if (o->AnimationFrame > 6.0f)
			{
				o->PKKey += 1;
				o->Position[2] += (o->PKKey * 0.8f);//5.0f;
//				o->Position[0] += sinf(o->Angle[2]*Q_PI/180.0f) * (o->PKKey * 0.4f);//10.0f;
//				o->Position[1] -= cosf(o->Angle[2]*Q_PI/180.0f) * (o->PKKey * 0.4f);//10.0f;


				VectorCopy(o->Position, Position);
				Position[0] += rand()%120 - 60;
				Position[1] += rand()%120 - 60;
				Position[2] += rand()%60;
				Vector(1,1,1,Light);
				for (int i = 0; i < 3; ++i)
					CreateParticle(BITMAP_SMOKE,Position,o->Angle,Light,20,10.f);
			}
			else
			{
				o->Skill += 1;
				if (o->Angle[2] < 45.0f) o->Angle[2] += o->Skill * 0.3f;
				if (o->Angle[2] > 45.0f) o->Angle[2] -= o->Skill * 0.3f;

				if (o->Position[0] < Hero->Object.Position[0] + 300.0f) o->Position[0] += o->Skill * 0.2f;
				if (o->Position[0] > Hero->Object.Position[0] + 300.0f) o->Position[0] -= o->Skill * 0.2f;
				if (o->Position[1] < Hero->Object.Position[1] - 300.0f) o->Position[1] += o->Skill * 0.2f;
				if (o->Position[1] > Hero->Object.Position[1] - 300.0f) o->Position[1] -= o->Skill * 0.2f;

				Vector(1,1,1,Light);
				VectorCopy(o->Position, Position);
				Position[0] += rand()%1200 - 600;
				Position[1] += rand()%1200 - 600;
				vec3_t Angle;
				Vector(0.f,0.f,rand()%10*20.f,Angle);
	            CreateEffect ( MODEL_FIRE, Position, Angle, Light, 0, NULL, 0 );

				o->Scale += 0.02f;
//				o->Position[2] -= 0.5f;
				EarthQuake = (float)(rand()%8-8)*0.1f;
			}
			break;
		case MODEL_CUNDUN_SKILL:
			if (o->SubType == 0)
			{
				if (o->LifeTime == 30)
				{
					for (int i = 0; i < 10; ++i)
					{
						vec3_t Angle;
						Vector(0,0,o->Angle[2]+320+i*8,Angle);
						CreateEffect(MODEL_CUNDUN_PHOENIX, o->Position, Angle, o->Light, 0);
					}
				}
			}
			else if (o->SubType == 1)
			{
				vec3_t Angle;
				Angle[0] = 0;
				Angle[1] = 0;
				if (o->LifeTime == 30)
				{
					for (int i = 0; i < 20; ++i)
					{
						Angle[2] = ( float)(o->PKKey*30);
						CreateEffect(MODEL_CUNDUN_DRAGON_HEAD, o->Position, Angle, o->Light );
					}
				}
				if (o->PKKey > 5 && o->PKKey < 10)
				{
					Angle[2] = ( float)(o->PKKey*30);
					CreateEffect(MODEL_CUNDUN_DRAGON_HEAD, o->Position, Angle, o->Light );
				}
				++o->PKKey;
			}
			else if (o->SubType == 2)
			{
				if (o->PKKey == 0 && o->LifeTime < 10)
				{
					o->PKKey = 1;
				}
			}
			break;

        case BATTLE_CASTLE_WALL1:
        case BATTLE_CASTLE_WALL2:
        case BATTLE_CASTLE_WALL3:
        case BATTLE_CASTLE_WALL4:
            break;

        case MODEL_MONSTER01+77:
            if ( o->SubType==0 )
            {
                if ( o->LifeTime%5==0 )   //  성벽에서 떨어지는 돌무더기.
                {
                    VectorCopy ( o->Position, Position );
                
                    Position[0] += rand()%100-50.f;
                    Position[1] -= rand()%30+15.f;
                    CreateEffect ( MODEL_FLY_BIG_STONE1, Position, o->Angle, o->Light, 2 );
                }
            }
            if ( o->LifeTime<5 )
            {
                o->Alpha /= 1.3f;
            }
            break;
		case MODEL_ARROW_TANKER_HIT:
			{
				float Height = RequestTerrainHeight ( o->Position[0], o->Position[1] );

				if ( o->Position[2] < Height + 80.0f)
				{
					if(o->Alpha <= 0.1f) o->LifeTime = 0;
					o->Alpha /= 1.05f;

					Vector(0.0f, 0.0f, -30.0f, o->Direction);
					Vector ( 1.f, 0.6f, 0.2f, Light );
					CreateParticle ( BITMAP_ADV_SMOKE+1, o->Position, o->Angle, Light );

					if(o->HiddenMesh != 99)
					{
						CreateInferno(o->Position, 1);
						VectorCopy(o->Position, o->StartPosition);
						CreateEffect ( BITMAP_CRATER, o->Position, o->Angle, o->Light );
					}

					EarthQuake = (float)(rand()%8-8)*0.1f;
					EarthQuake /= 2.f;

					o->HiddenMesh   = 99;
				}

				if(o->HiddenMesh == 99)
				{
					o->Position[0] += (float)(rand()%100 - 50);
					o->Position[1] += (float)(rand()%100 - 50);
					o->Position[2] += (float)(rand()%100 - 50);
					o->Scale = 2.2f;
				}

				if(o->Visible)
				{
					vec3_t p1, p2;
					BMD* b = &Models[o->Type];
					
					Vector(1.0f, 1.0f, 1.0f, o->Light);
					Vector(0.0f, 1.5f, 0.0f, p1);
					b->TransformPosition(BoneTransform[2], p1, p2);
					CreateParticle(BITMAP_FIRE+1,o->Position,o->Angle,o->Light, 8, o->Scale-0.4f, o );
					CreateParticle(BITMAP_SMOKE,o->Position,o->Angle,o->Light, 38, o->Scale, o );
					Vector(1.0f, 0.4f, 0.0f, o->Light);
      				CreateParticle(BITMAP_FIRE+1,o->Position,o->Angle,o->Light, 9, o->Scale-0.4f, o );
				}

				if(o->HiddenMesh == 99)
				{
					o->Scale = 1.0f;
					VectorCopy(o->StartPosition, o->Position);
				}

				Vector ( 1.0f, 0.0f, 0.0f, Light );
		        AddTerrainLight ( o->Position[0], o->Position[1], Light, 4, PrimaryTerrainLight );
			}
			break;
        case MODEL_FLY_BIG_STONE1:
            {
                if ( o->SubType==2 )   //  떨어지는 돌.
                {
                    o->Position[2] -= o->Gravity;
                    o->Gravity += 1.9f;

                    o->Angle[0] += rand()%20+20.f;
                    o->Angle[1] += rand()%5;

                    float Height = RequestTerrainHeight ( o->Position[0], o->Position[1] );
                    if ( o->Position[2]<Height )
                    {
                        o->Position[2] = Height+50;
                        o->Gravity = -o->Gravity*0.2f;
                        o->LifeTime -= 5;
                        o->Angle[0] -= o->Scale*128.f;

                        Vector ( 0.f, 5.f, 0.f, o->Direction );
                    }
                    else
                        o->Angle[0] -= o->Scale*16.f;

		            AngleMatrix ( o->HeadAngle, Matrix );
		            VectorRotate ( o->Direction, Matrix, Position );
		            VectorAdd ( o->Position, Position, o->Position );

                    if ( o->LifeTime<10 )
                    {
                        o->Alpha /= 1.5f;
                    }

                    if ( o->LifeTime%3==0 )
                    {
                        Vector ( 1.f, 0.6f, 0.2f, Light );
                        CreateParticle ( BITMAP_ADV_SMOKE+1, o->Position, o->Angle, Light, 1, 0.2f );
                    }
                }
                else
                {
                    if ( o->SubType<=1 )
                    {
                        o->Position[0] += o->Direction[0]*o->Velocity;
                        o->Position[1] += o->Direction[1]*o->Velocity;
                        o->StartPosition[2] += o->Direction[2];
                
                        if ( o->Position[2]>1000.f )
                        {
                            o->Position[2] = 1000.f;
                            o->HiddenMesh = -2;
                        }
                        else
                        {
                            o->HiddenMesh = -1;
                            Vector ( Luminosity*1.f, Luminosity*0.5f, Luminosity*0.1f, Light );
       			            CreateParticle ( BITMAP_FIRE, o->Position, o->Angle, Light, 5, 2.f );
                        }
                
                        o->Velocity -= 0.45f;
                        if ( o->Velocity<5.f ) o->Velocity = 5.f;

                        o->Direction[2] += o->Gravity;
                        o->Gravity  -= 0.1f;
                
                        o->Angle[0] += 5.f;
                        o->Angle[1] += 5.f;

                        if ( o->SubType==0 )
                        {
                            o->Position[2] = o->StartPosition[2];
                        }
                        else if ( o->Owner==(&Hero->Object) )
                        {
                            o->Position[2] = o->StartPosition[2];
                            if ( o->Position[2]>800.f )
                            {
                                o->Position[2] = 800.f;
                            }
							g_pCatapultWindow->SetCameraPos(o->Position[0], o->Position[1], o->Position[2]);
                        }
                    }

                    if ( o->SubType==88 || o->SubType==99 )
                    {
                        if ( o->SubType==99 && o->Owner==(&Hero->Object) )
                        {
							g_pCatapultWindow->SetCameraPos(o->Position[0], o->Position[1], o->Position[2]);
                        }

                        if ( o->LifeTime>o->DamageTime )
                        {
                            Vector ( 1.f, 0.6f, 0.2f, Light );
                            CreateParticle ( BITMAP_ADV_SMOKE+1, o->Position, o->Angle, Light );
       			            CreateParticle ( BITMAP_FIRE, o->Position, o->Angle, Light, 2, 2.f );

                            if ( o->SubType==88 && o->Owner!=NULL )
                            {
                                battleCastle::CollisionHeroCharacter ( o->Position, ((15-o->LifeTime)*20)+350.f, PLAYER_HIGH_SHOCK );
                            }
                            else if ( o->SubType==99 )
                            {
                                battleCastle::CollisionTempCharacter ( o->Position, ((40-o->LifeTime)*7)+350.f, PLAYER_HIGH_SHOCK );
                            }
                        }
                    }
                    else
                    {
			            float Height = RequestTerrainHeight(o->Position[0],o->Position[1]);
                        if ( o->LifeTime<250 && o->Position[2]<Height+300.f && o->SubType==1 && o->Kind!=0 && o->Skill!=0 )
                        {
                            SendWeaponExplosion ( o->Kind, o->Skill );
                            o->Kind = 0;
                            o->Skill = 0;
                        }

			            if ( o->Position[2]<Height )
                        {
                            o->Position[2]  = Height-5.f;
                            if ( o->SubType==0 )
                            {
                                o->LifeTime = 15;
                                o->DamageTime = 0;
                                o->SubType  = 88;
                            }
                            else if ( o->SubType==1 )
                            {
                                o->LifeTime = 40;
                                o->DamageTime = 25;
                                o->SubType  = 99;
                            }
                            if ( o->HiddenMesh!=99 && o->Visible )
                            {
                                CreateEffect ( BITMAP_CRATER, o->Position, o->Angle, o->Light );
                                PlayBuffer ( SOUND_BC_CATAPULT_HIT, o );

                                //  특정 오브젝트와 충돌을 검사한다.
                                battleCastle::CollisionEffectToObject ( o, 350.f, 250.f, true );
                                
                                //  충돌시 애니메이션 현상.
                                if ( o->SubType==88 && o->Owner==NULL )
                                {
                                    battleCastle::CollisionHeroCharacter ( o->Position, 350.f, PLAYER_HIGH_SHOCK );
                                }
                                else if ( o->SubType==99 || ( o->SubType==88 && o->Owner!=NULL ) )
                                {
                                    Vector ( 1.f, 0.3f, 0.1f, Light );
                                    CreateEffect ( BITMAP_SHOCK_WAVE, o->Position, o->Angle, Light, 7 );
                                }
                            }
                            o->HiddenMesh   = 99;

                            if ( Height>150.f )
                            {
                                int CollisionX = (int)(o->Position[0]/TERRAIN_SCALE);
                                int CollisionY = (int)(o->Position[1]/TERRAIN_SCALE);

                                EarthQuake = (float)(rand()%8-8)*0.1f;
                                if ( CollisionY<104 && o->Visible )
                                {
                                    AddTerrainHeight ( o->Position[0], o->Position[1], -30.f, 2, BackTerrainHeight );
                                    CreateTerrainNormal_Part ( CollisionX, CollisionY );
                                    CreateTerrainLight_Part ( CollisionX, CollisionY );
                            
                                    EarthQuake /= 2.f;
                                }
                            }
                        }
                        else
                        {
                            //  특정 오브젝트와 충돌을 검사한다.
                            bool collision;
                            if ( o->SubType==1 )
                            {
                                collision = battleCastle::CollisionEffectToObject ( o, 200.f, 250.f, false, true );
                            }
                            else/* if ( o->Visible )*/
                            {
                                collision = battleCastle::CollisionEffectToObject ( o, 200.f, 250.f, false );
                            }
                            if ( collision && o->Visible )
                            {
                                PlayBuffer ( SOUND_BC_CATAPULT_HIT, o );
                            }
                        }
                    }
                    Vector ( -0.5f, -0.5f, -0.5f, Light );
		            AddTerrainLight ( o->Position[0], o->Position[1], Light, 4, PrimaryTerrainLight );
                }
            }
            break;

        case MODEL_FLY_BIG_STONE2:
            {
                if ( o->SubType<=1 )
                {
                    o->Position[0] += o->Direction[0]*o->Velocity;
                    o->Position[1] += o->Direction[1]*o->Velocity;
                    o->StartPosition[2] += o->Direction[2];
                
                    if ( o->Position[2]>1000.f )
                    {
                        o->Position[2] = 1000.f;
                        o->HiddenMesh = -2;
                    }
                    else
                    {
                        o->HiddenMesh = -1;
                        Vector ( 1.f, 1.f, 1.f, Light );

                        if ( (rand()%2)==0 )
                        {
                            CreateParticle ( BITMAP_ADV_SMOKE+1, o->Position, o->Angle, Light, 1, 2.f );
                        }
                    }
                
                    o->Velocity -= 0.45f;
                    if ( o->Velocity<5.f ) o->Velocity = 5.f;

                    o->Direction[2] += o->Gravity;
                    o->Gravity  -= 0.1f;
                
                    o->Angle[0] += 5.f;
                    o->Angle[1] += 5.f;

                    if ( o->SubType==0 )
                    {
                        o->Position[2] = o->StartPosition[2];
                    }
                    else if ( o->Owner==(&Hero->Object) )
                    {
                        o->Position[2] = o->StartPosition[2];
                        if ( o->Position[2]>800.f )
                        {
                            o->Position[2] = 800.f;
                        }
						g_pCatapultWindow->SetCameraPos(o->Position[0], o->Position[1], o->Position[2]);
                    }
                }

                Vector ( -0.5f, -0.5f, -0.5f, Light );
                if ( o->SubType==88 || o->SubType==99 )
                {
                    if ( o->SubType==99 && o->Owner==(&Hero->Object) )
                    {
						g_pCatapultWindow->SetCameraPos(o->Position[0], o->Position[1], o->Position[2]);
                    }

                    if ( o->LifeTime>o->DamageTime )
                    {
                        CreateParticle ( BITMAP_ADV_SMOKE+1, o->Position, o->Angle, o->Light );

                        if ( o->SubType==88 && o->Owner!=NULL )
                        {
                            battleCastle::CollisionHeroCharacter ( o->Position, ((15-o->LifeTime)*20)+350.f, PLAYER_HIGH_SHOCK );
                        }
                        else if ( o->SubType==99 )
                        {
                            battleCastle::CollisionTempCharacter ( o->Position, ((40-o->LifeTime)*7)+350.f, PLAYER_HIGH_SHOCK );
                        }
                    }
                }
                else
                {
			        float Height = RequestTerrainHeight(o->Position[0],o->Position[1]);
                    if ( o->Position[2]<Height+300.f && o->SubType==1 && o->Kind!=0 && o->Skill!=0 )
                    {
                        SendWeaponExplosion ( o->Kind, o->Skill );
                        o->Kind = 0;
                        o->Skill = 0;
                    }
                    
			        if ( o->Position[2]<Height )
                    {
                        o->Position[2]  = Height-5.f;
                        if ( o->SubType==0 )
                        {
                            o->LifeTime = 15;
                            o->DamageTime = 0;
                            o->SubType  = 88;
                        }
                        else if ( o->SubType==1 )
                        {
                            o->LifeTime = 40;
                            o->DamageTime = 25;
                            o->SubType  = 99;
                        }
                        if ( o->HiddenMesh!=99 && o->Visible )
                        {
                            CreateEffect ( BITMAP_CRATER, o->Position, o->Angle, o->Light );
                            PlayBuffer ( SOUND_BC_CATAPULT_HIT, o );

                            //  충돌시 애니메이션 현상.
                            if ( o->SubType==88 && o->Owner==NULL )
                            {
                                battleCastle::CollisionHeroCharacter ( o->Position, 350.f, PLAYER_HIGH_SHOCK );
                            }
                            else if ( o->SubType==99 || ( o->SubType==88 && o->Owner!=NULL ) )
                            {
                                Vector ( 1.f, 0.3f, 0.1f, Light );
                                CreateEffect ( BITMAP_SHOCK_WAVE, o->Position, o->Angle, Light, 7 );
                            }
                        }
                        o->HiddenMesh   = 99;

                        if ( Height>150.f )
                        {
                            int CollisionX = (int)(o->Position[0]/TERRAIN_SCALE);
                            int CollisionY = (int)(o->Position[1]/TERRAIN_SCALE);

                            EarthQuake = (float)(rand()%8-8)*0.1f;
                            if ( CollisionY<104 && o->Visible )
                            {
                                AddTerrainHeight ( o->Position[0], o->Position[1], -30.f, 2, BackTerrainHeight );
                                CreateTerrainNormal_Part ( CollisionX, CollisionY );
                                CreateTerrainLight_Part ( CollisionX, CollisionY );
                            
                                EarthQuake /= 2.f;
                            }
                        }
                    }
                }
		        AddTerrainLight ( o->Position[0], o->Position[1], Light, 4, PrimaryTerrainLight );
            }
            break;
            
        case MODEL_BIG_STONE_PART1:
        case MODEL_BIG_STONE_PART2:
        case MODEL_WALL_PART1:
        case MODEL_WALL_PART2:
		case MODEL_GOLEM_STONE:
			if(o->Type == MODEL_BIG_STONE_PART2 && o->SubType == 3)
			{
				o->Direction[2] -= o->Velocity;
				o->Velocity += 0.3f;
				
				Height = RequestTerrainHeight ( o->Position[0], o->Position[1] );
				if ( o->Position[2]<Height )
				{
					o->Position[2] = Height;
					Vector(0.f,0.f,0.f,o->Direction);
					Vector(0.f,0.f,0.f, o->Angle);
					o->HiddenMesh = 0;

					if(rand()%10==0)
						CreateParticle(BITMAP_SMOKE,o->Position,o->Angle,o->Light,24,1.25f*o->Scale);
				}
				else 
				{
					if(rand()%2==0)
						CreateParticle(BITMAP_SMOKE,o->Position,o->Angle,o->Light,24,0.2f);
				}
				break;
			}
            VectorAdd ( o->Position, o->Direction, o->Position );
            if ( o->Type>=MODEL_WALL_PART1 && o->Type<=MODEL_WALL_PART2 )
            {
                VectorScale ( o->Direction, 0.9f, o->Direction );
                o->Gravity -= 6.f;
            }
            else
            {
                VectorScale ( o->Direction, 0.9f, o->Direction );
                o->Gravity -= 3.f;
            }
            o->Position[2] += o->Gravity;
    
            Height = RequestTerrainHeight ( o->Position[0], o->Position[1] );
            if ( o->Position[2]<Height )
            {
                o->Position[2]  = Height;
                o->Gravity      = -o->Gravity*0.2f;
                o->LifeTime    -= 5;
                o->Angle[0]    -= o->Scale*128.f;
            }
            else
            {
                o->Angle[0] -= o->Scale*16.f;
            }
    
            o->Alpha = o->LifeTime/10.f;
			if( o->Type == MODEL_GOLEM_STONE) {		//. 골렘파편	
				if(rand()%4==0) {
					CreateParticle ( BITMAP_TRUE_FIRE, o->Position, o->Angle, Light, 5, 2.8f);
					CreateParticle ( BITMAP_SMOKE, o->Position, o->Angle, Light, 21 , 1.8f);
				}
				if ( (rand()%10)==0 ) {
					CreateParticle ( BITMAP_SMOKE+1, o->Position, o->Angle, Light );
				}
			}
			else if (o->Type == MODEL_BIG_STONE_PART1 && o->SubType == 2) {	//. 독골렘 파편
				if ( (rand()%10)==0 ) {
					Vector(0.2f,0.5f,0.35f, Light);
					o->Position[2] = Height;
					CreateParticle(BITMAP_SMOKE,o->Position,o->Angle,Light,11, 2.f);
				}
			}
            break;

        case MODEL_GATE_PART1:
        case MODEL_GATE_PART2:
        case MODEL_GATE_PART3:
		    VectorAdd(o->Position,o->Direction,o->Position);
		    VectorScale(o->Direction,0.9f,o->Direction);
		    o->Position[2] += o->Gravity;

            if ( o->SubType==1 )
            {
		        VectorScale(o->Direction,1.1f,o->Direction);
            }

		    o->Gravity -= 4.f;
		    Height = RequestTerrainHeight(o->Position[0],o->Position[1]);
		    if(o->Position[2] < Height)
		    {
			    o->Position[2] = Height;
			    o->Gravity = -o->Gravity*0.5f;
			    o->LifeTime -= 5;
			    o->Angle[0] -= o->Scale*128.f;
		    }
		    else
			    o->Angle[0] -= o->Scale*16.f;

            if ( (rand()%10)==0 )
            {
		        CreateParticle(BITMAP_SMOKE+1,Position,o->Angle,Light);
            }
            break;

        case MODEL_AURORA:
            if ( o->Owner!=NULL && o->Owner->Live==true )
            {
                if ( battleCastle::IsBattleCastleStart()==false )
                {
                    o->HiddenMesh = -2;
                }
                else
                {
                    o->HiddenMesh = -1;
                }
                o->LifeTime = 2;
                o->BlendMeshLight = sinf( WorldTime*0.001f )*0.1f+0.2f;
            }
            else
            {
                o->Live = false;
            }
            o->BlendMeshTexCoordU = WorldTime*0.0005f;
            break;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//^ 펜릴 이펙트 관련
		case MODEL_FENRIR_THUNDER:
			if(o->SubType == 0)
			{
				if(o->Live)
				{
					if(o->m_iAnimation == 0)
					{
						o->Alpha += 0.3f;
						if(o->Alpha >= 1.0f)
						{
							o->m_iAnimation = 1;
							o->Alpha = 1.0f;
						}

						if(o->Alpha < 0.0f)
						{
							o->Alpha = 1.0f;
						}
					}
					else if(o->m_iAnimation == 1)
					{
						o->Alpha -= 0.3f;
						if(o->Alpha <= 0.0f)
						{
							o->Alpha = 0.0f;
							o->Live = false;
						}
					}
					o->Angle[0] += 0.15f;
					o->Angle[1] += 0.15f;
					o->Angle[2] += 0.15f;
				}
			}
			else if(o->SubType == 1)
			{
				if(o->Live)
				{
					if(o->m_iAnimation == 0)
					{
						o->Alpha += 0.3f;
						if(o->Alpha >= 1.0f)
						{
							o->m_iAnimation = 1;
							o->Alpha = 1.0f;
						}

						if(o->Alpha < 0.0f)
						{
							o->Alpha = 1.0f;
						}
					}
					else if(o->m_iAnimation == 1)
					{
						o->Alpha -= 0.3f;
						if(o->Alpha <= 0.0f)
						{
							o->Alpha = 0.0f;
							o->Live = false;
						}
					}
					o->Angle[0] += 0.15f;
					o->Angle[1] += 0.15f;
					o->Angle[2] += 0.15f;
				}
			}	
			break;
		case MODEL_FALL_STONE_EFFECT:
			{
				if(o->SubType == 0 || o->SubType == 1)
				{
					o->Position[2] -= o->Gravity;
					o->Gravity += 0.1f;
					o->Angle[0] += 0.5f;
					o->Angle[1] += 0.5f;
					o->Angle[2] += 0.5f;
				}
#ifdef CSK_RAKLION_BOSS
				else if(o->SubType == 2)
				{
					o->Position[2] -= o->Gravity;
					o->Gravity += 0.1f;

					float fHeight = RequestTerrainHeight(o->Position[0], o->Position[1]);
					if(o->Position[2] < fHeight)
					{
						vec3_t vLight;
						Vector(0.5f, 0.5f, 0.5f, vLight);
						CreateParticle(BITMAP_SMOKE, o->Position, o->Angle, vLight, 11, (float)(rand()%20+30)*0.020f );

						int iRand = rand()%2 + 2;
						for(int i=0; i<iRand; ++i)
						{
							float fScale = 0.03f + (rand()%10) / 40.0f + o->Scale * 0.3f;
							CreateEffect(MODEL_FALL_STONE_EFFECT, o->Position, o->Angle, o->Light, 3, NULL, -1, 0, 0, 0, fScale);
						}

						o->Live = false;
					}
				}
				else if(o->SubType == 3)
				{
					o->HeadAngle[2] -= o->Gravity;
					
					o->Position[0] += o->HeadAngle[0];
					o->Position[1] += o->HeadAngle[1];
					o->Position[2] += o->HeadAngle[2];
					
					float fHeight = RequestTerrainHeight(o->Position[0], o->Position[1]);
					o->Angle[0] += 0.5f * o->LifeTime;
					o->Angle[1] += 0.5f * o->LifeTime;
					if (o->Position[2] + o->Direction[2] <= fHeight)
					{
						o->Position[2] = fHeight;
						o->HeadAngle[0] *= 0.6f;
						o->HeadAngle[1] *= 0.6f;
						o->HeadAngle[2] += 1.0f*o->LifeTime;
						if (o->HeadAngle[2] < 0.5f)
							o->HeadAngle[2] = 0;
						
						o->Alpha -= 0.15f;

						o->Light[0] /= 1.08f;
						o->Light[1] /= 1.08f;
						o->Light[2] /= 1.08f;
					}
				}
#endif // CSK_RAKLION_BOSS	
			}
			break;
		case MODEL_FENRIR_FOOT_THUNDER:
			if(o->Live == true)
			{
				o->Angle[0] += 0.1f;

				if(o->SubType == 1)	// 빨강
				{
					o->Light[1] -= 0.05f;
					o->Light[2] -= 0.05f;
				}
				else if(o->SubType == 2)	// 파랑
				{
					o->Light[0] -= 0.05f;
					o->Light[1] -= 0.05f;
				}
				else if(o->SubType == 3)	// 검정
				{
					o->Light[0] -= 0.05f;
					o->Light[2] -= 0.05f;
				}
				else if(o->SubType == 4)	// 노랑
				{
					o->Light[2] -= 0.05f;
				}
				o->Alpha -= 0.05f;
				if(o->Alpha <= 0.0f)
					o->Live = false;

				if(timeGetTime() - o->m_dwTime > 200)
				{
					o->m_iAnimation++;	// 에니메이션 키값
					if(o->m_iAnimation > 4)
						o->Live = false;
					o->m_dwTime = timeGetTime();
				}
			}
			break;
		case MODEL_TWINTAIL_EFFECT:
			{
				if(o->SubType == 0)
				{
					o->Alpha -= 0.01f;
					if(o->Alpha <= 0.0f)
					{
						o->Live = false;
					}

					o->Light[0] /= 1.02f;
					o->Light[1] /= 1.02f;
					o->Light[2] /= 1.02f;

					if(timeGetTime() - o->m_dwTime > 1000)
					{
						if(o->m_iAnimation == 0)
						{
							o->m_iAnimation = 1;
						}
						else
						{
							o->m_iAnimation = 0;
						}
						
						o->m_dwTime = timeGetTime();	
					}

					if(o->m_iAnimation == 0)	// 축소
					{
						o->Scale -= 0.015f;
						if(o->Scale <= 0.0f)
						{
							o->Scale = 0.0f;
						}
					}
					else	// 확대
					{
						o->Scale += 0.02f;
						if(o->Scale >= 1.2f)
						{
							o->Scale = 1.2f;
						}
					}
				}
				else if(o->SubType == 1 || o->SubType == 2)
				{
					o->Alpha -= 0.01f;
					if(o->Alpha <= 0.0f)
					{
						o->Live = false;
					}

					o->Light[0] /= 1.04f;
					o->Light[1] /= 1.04f;
					o->Light[2] /= 1.04f;

					if(o->SubType == 1)
						o->Angle[0] = -(WorldTime * 0.3f);
					else if(o->SubType == 2)
						o->Angle[0] = -(WorldTime * 0.1f);

					o->Scale -= 0.02f;
				}
			}
			break;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
        case MODEL_TOWER_GATE_PLANE:
            o->LifeTime = 100;
            if ( o->Owner!=NULL && o->Live==true )
            {
                o->Position[2] = o->StartPosition[2] + sinf( WorldTime*0.001f )*200.f+200.f;
            }
            break;

        case BITMAP_CRATER:
            if ( o->LifeTime<10 )
            {
                o->Light[0] = o->LifeTime/10.f;
                o->Light[1] = o->Light[0];
                o->Light[2] = o->Light[0];
                o->Alpha    = o->Light[0];
            }
            Vector ( -0.5f, -0.5f, -0.5f, Light );
		    AddTerrainLight ( o->Position[0], o->Position[1], Light, (int)(o->StartPosition[0]-1), PrimaryTerrainLight );
            break;

#ifdef YDG_ADD_SKILL_GIGANTIC_STORM
		case BITMAP_CHROME_ENERGY2:
            if ( o->LifeTime<10 )
            {
                o->Light[0] *= 0.8f;
                o->Light[1] *= 0.8f;
                o->Light[2] *= 0.8f;
            }
			break;
#endif	// YDG_ADD_SKILL_GIGANTIC_STORM

        case MODEL_STUN_STONE:
            if ( o->SubType==0 )
            {
                o->Position[2] += o->Gravity;
			    o->Gravity -= 15.f;

                Height = RequestTerrainHeight(o->Position[0],o->Position[1])+50;
                if ( o->Position[2]<=Height )
                {
                    o->Position[2] = Height;
                    if ( o->ExtState==0 )
                    {
                        CreateEffect ( BITMAP_CRATER, o->Position, o->Angle, o->Light, 1 );
                        o->ExtState = 1;
                    }
                }
                else
                {
			        VectorAdd ( o->Position, o->StartPosition, o->Position );
                    VectorScale ( o->StartPosition, 0.9f, o->StartPosition );

                    o->HeadAngle[0] -= o->Scale*32.f;

                    if ( o->LifeTime%3==0 )
                    {
                        CreateParticle ( BITMAP_ADV_SMOKE+1, o->Position, o->Angle, o->Light, 1, 1.f );
                    }
                }
                o->Light[0] = o->LifeTime/10.f;
                o->Light[1] = o->Light[0];
                o->Light[2] = o->Light[0];
                o->Alpha    = o->Light[0];
            }
            else if ( o->SubType==1 )
            {
                if ( o->LifeTime%3==0 )
                {
				    CreateEffect ( MODEL_STUN_STONE, o->Position, o->Angle, o->Light );
                }
		        AddTerrainLight ( o->Position[0], o->Position[1], o->Light, 1, PrimaryTerrainLight );
            }
            break;

        case MODEL_SKIN_SHELL:  //  스킨 파편.
            VectorAdd ( o->Position, o->Direction, o->Position );
            VectorScale ( o->Direction, 0.9f, o->Direction );
            o->Position[2] += o->Gravity;
        
            VectorScale(o->Direction,1.1f,o->Direction);
        
            o->Gravity -= 3.f;
            Height = RequestTerrainHeight(o->Position[0],o->Position[1]);
            if(o->Position[2] < Height)
            {
                o->Position[2] = Height;
                o->Gravity = -o->Gravity*0.2f;
                o->LifeTime -= 5;
                o->Angle[0] -= o->Scale*128.f;
            }
            else
                o->Angle[0] -= o->Scale*16.f;
        
            o->Alpha = o->LifeTime/10.f;
            break;

        case MODEL_MANA_RUNE:
            if ( o->SubType==0 )
            {
                if ( o->LifeTime>43 )
                {
                    o->HiddenMesh = -2;
                }
                else if ( o->LifeTime>40 )
                {
                    o->HiddenMesh = 0;
                }
                else if ( o->LifeTime>30 )
                {
                    o->Scale += o->Gravity;
                    o->Gravity += 0.15f;
                    if ( o->Scale>1.f )
                    {
                        o->Scale = 1.f;
                        o->Gravity = 0.01f;

                        CreateEffect ( MODEL_MANA_RUNE, o->Position, o->Angle, o->Light, 1 );
                    }
                }
                else if ( o->LifeTime<15 )
                {
                    o->Scale -= o->Gravity;
                    o->Gravity += 0.2f;
                    if ( o->Scale<0.f )
                    {
                        o->Scale = 0.f;
                    }
                    o->Alpha = (o->LifeTime/20.f);
                    o->Position[2] -= 10.f;
                }
            }
            else if ( o->SubType==1 )
            {
                o->Scale -= 0.02f;
                if ( o->Scale<1.f )
                {
                    o->Scale = 1.f;
                }
                o->Alpha = (o->LifeTime/20.f);
            }
            break;

        case MODEL_SKILL_JAVELIN:
            if ( o->Owner!=NULL )
            {
                VectorCopy ( o->Owner->Position, o->StartPosition );
                o->StartPosition[2] += 150.f;
            }

            o->Scale += 0.015f;
            if ( o->LifeTime<25 )
            {
                float Distance;
                if ( o->LifeTime<20 )
                {
                    Distance = MoveHumming ( o->Position, o->HeadAngle, o->StartPosition, o->Velocity );
                    if ( Distance<100.f )
                    {
                        VectorCopy ( o->StartPosition, o->Position );
                        o->Scale += 0.04f;

                        if ( o->LifeTime%3==0 )
                        {
                            CreateParticle ( BITMAP_POUNDING_BALL, o->Position, o->Angle, o->Light, 1 );
                        }
                    }
                    else
                    {
                        CreateParticle ( BITMAP_POUNDING_BALL, o->Position, o->Angle, o->Light, 1 );
                    }
                }
                o->Velocity += 1.5f;
                
                AngleMatrix ( o->HeadAngle, Matrix );
                VectorRotate ( o->Direction, Matrix, Position );
                VectorAdd ( o->Position, Position, o->Position );
                
                o->Gravity = rand()%30+30.f;

                if ( o->Direction[1]>-50.f )
                {
                    o->Direction[1] -= 8.f;
                }
            }
            else
            {
                o->Gravity += 10.f;
                AngleMatrix(o->HeadAngle,Matrix);
                VectorRotate(o->Direction,Matrix,Position);
                VectorAdd(o->Position,Position,o->Position);
            }
            o->Angle[2] += o->Gravity;
            o->BlendMeshLight = o->LifeTime/10.f;
            o->Alpha = o->BlendMeshLight;

            Height = sinf( o->LifeTime*0.1f )*30.f;
            if ( o->SubType==1 )
            {
                o->Position[2] = o->StartPosition[2]+Height;
            }
            else if ( o->SubType==2 )
            {
                o->Position[2] = o->StartPosition[2]-Height;
            }

		    Vector ( 1.f, 0.6f, 0.3f, Light );
            AddTerrainLight ( o->Position[0], o->Position[1], Light, 2, PrimaryTerrainLight );
            break;

        case MODEL_ARROW_IMPACT:
            o->Angle[0] -= 5.f;
            o->Direction[1] -= 8.f;

            if ( o->LifeTime<2 && o->SubType==0 && o->Owner!=NULL )
            {
                o->Angle[0] = 90.f;
                o->SubType  = 1;

                VectorCopy ( o->Owner->Position, Position );
                Position[0] += rand()%100-50.f;
                Position[1] += rand()%100-50.f;
                Position[2] += 1200.f;
                CreateJoint ( BITMAP_FLASH, Position, Position, o->Angle, 2, o, 50.f );
                CreateJoint ( BITMAP_FLASH, Position, Position, o->Angle, 3, o, 50.f );
            }
            else if ( o->SubType==1 )
            {
                o->Live = false;
            }
            break;

        case BITMAP_JOINT_FORCE:
			if(o->SubType == 0)
			{
				if ( o->LifeTime<11 && o->LifeTime%2==0 )
				{
					Vector ( 90.f, 0.f, 0.f, o->Angle );
					o->HeadAngle[2] += 72.f;
					AngleMatrix ( o->HeadAngle, Matrix );
					VectorRotate ( o->Direction, Matrix, Position );
					VectorAdd ( o->StartPosition, Position, Position );

					Position[2] += rand()%400+700.f;
         			CreateJoint ( BITMAP_FLASH, Position, Position, o->Angle, 5, o, 110.f );
				}
			}
			else if(o->SubType == 1)
			{
			   if ( o->LifeTime<11 && o->LifeTime%2==0 )
				{
					Vector ( 90.f, 0.f, 0.f, o->Angle );
					o->HeadAngle[2] += 72.f;
					AngleMatrix ( o->HeadAngle, Matrix );
					VectorRotate ( o->Direction, Matrix, Position );
					VectorAdd ( o->StartPosition, Position, Position );

					Position[2] += 100.f;
          			CreateJoint ( BITMAP_JOINT_THUNDER+1, Position, Position, o->Angle, 6, o, 80.f );
        			CreateJoint ( BITMAP_JOINT_THUNDER+1, Position, Position, o->Angle, 6, o, 80.f );
				}
			}

		case MODEL_SWORD_FORCE:
            if ( o->SubType==0 
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
				|| o->SubType==2
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
				)
            {
                if ( o->LifeTime>12 )
                {
                    o->Scale += 0.9f;

                    o->Direction[1] -= 2.f;
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
					if(o->SubType==2)
					{
						CreateEffect( MODEL_SWORD_FORCE, o->Position, o->Angle, o->Light, 3, o );
					}
					else
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
                    CreateEffect( MODEL_SWORD_FORCE, o->Position, o->Angle, o->Light, 1, o );
                }
                else
                {
                    o->Scale -= 0.05f;
			        o->BlendMeshLight = ( float)o->LifeTime/18.f;
                    o->Alpha = o->BlendMeshLight;
                    o->Light[0] = o->Alpha;
                    o->Light[1] = o->Alpha;
                    o->Light[2] = o->Alpha;

                    o->Direction[1] -= 2.f;

			        VectorCopy ( o->Position, Position );
			        Position[0] += rand()%30-15.f;
			        Position[1] += rand()%30-15.f;
			        Position[2] -= 100.f;
			        for(int i=0;i<4;i++)
			        {
				        Vector ( (float)(rand()%60+60+90), 0.f, o->Angle[2], Angle );
				        CreateJoint ( BITMAP_JOINT_SPARK, Position, Position, Angle );
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
						if(o->SubType==2)
						{
							CreateParticle(BITMAP_FIRE, Position, Angle, o->Light, 18, 1.5f);
						}
						else
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
				        CreateParticle ( BITMAP_FIRE, Position, Angle, o->Light, 2, 1.5f );
			        }
                }
                Vector ( 1.f, 0.8f, 0.6f, Light );
                AddTerrainLight ( o->Position[0], o->Position[1], Light, 1, PrimaryTerrainLight );
            }
            else if ( o->SubType==1 
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
						|| o->SubType==3
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
					)
            {
                o->BlendMeshLight = ( float)o->LifeTime/10.f;
                o->Alpha = o->BlendMeshLight;
                o->Light[0] = o->Alpha;
                o->Light[1] = o->Alpha;
                o->Light[2] = o->Alpha;
            }
			break;

		case MODEL_PROTECTGUILD:
			{
				if (o->Owner == NULL/* || o->Owner->Alpha < 1.0f*/)
				{
					o->LifeTime = 0;
					break;
				}
				if (o->Alpha >= 1.0f) o->Angle[2] += 5.0f;
//				o->Alpha = sin((o->LifeTime/130.0f)*3.14f) - 0.1f;
				if (o->LifeTime > 120)
				{
					if (o->LifeTime < 125 && o->LifeTime > 120)
					{
						for (int i = 0; i < 5; ++i)
							CreateParticle(BITMAP_SPARK,o->Position,o->Angle,o->Light,6,1.5f,o);
					}
				}
				else if (o->LifeTime > 95)
				{
					if (o->Alpha > 1.0f) o->Alpha = 1.0f;
					else o->Alpha += 0.4f;
				}
				else if (o->LifeTime < 50)
				{
					if (o->Alpha < 0) o->Alpha = 0;
					else o->Alpha -= 0.1f;
				}

				BMD *b = &Models[o->Owner->Type];
				vec3_t tempPosition;
				Vector(0.f,0.f,0.f,p);
				b->TransformPosition(o->Owner->BoneTransform[20],p,tempPosition,true);
				o->Position[0] = tempPosition[0] + (o->Owner->Position[0]-Hero->Object.Position[0]);
				o->Position[1] = tempPosition[1] + (o->Owner->Position[1]-Hero->Object.Position[1]);

				float fHeight = o->Owner->Position[2] + tempPosition[2] - o->Owner->Position[2] + 60;
				if (fHeight < o->Position[2])
					o->Position[2] += (fHeight - o->Position[2]) * 0.1f;
				else
					o->Position[2] += (fHeight - o->Position[2]) * 0.5f;
				
				if (o->LifeTime < 60 && o->LifeTime > 39)
				{
					vec3_t p;
					for (int i = 0; i < 1; ++i)
					{
						float fAngle = rand() % 360;
						Vector(o->Position[0] + (rand() % 26 - 13) * sinf(fAngle),
							o->Position[1] + (rand() % 26 - 13) * cosf(fAngle),
							//o->Position[2] + 45+(10-o->LifeTime)*1.5f+rand()%5, p);
							o->Position[2] + 48 - o->LifeTime + (rand() % 5), p);
						CreateParticle(BITMAP_SPARK,p,o->Angle,o->Light,5,2.0f,o);
					}
				}
			}
			break;
		case MODEL_MOVE_TARGETPOSITION_EFFECT:
			{
				if( o->SubType == 0 )
				{	
					//float fScale = 0.f;
					vec3_t vLight, vPos, vAngle, vRelativePos;
					Vector( 1.0f, 0.7f, 0.3f, vLight );
					Vector( 0.0f, 0.0f, 0.0f, vAngle );
					Vector( 0.0f, 0.0f, 0.0f, vRelativePos );
					VectorCopy( o->Position, vPos );
					vPos[2] += 110.f;

					{
						CreateParticle(BITMAP_SPARK+1, vPos, o->Angle, vLight, 24, 1.0f, o);
					}
					

					if( o->LifeTime%15 == 0 )
					{
						CreateEffect( BITMAP_TARGET_POSITION_EFFECT1, o->Position, vAngle, vLight, 0);//, NULL, -1, 0, 0, 0, fScale );
					}
					
					if( o->LifeTime <= 10 )
					{
						o->BlendMeshLight -= 0.05f;
 						o->Light[0] *= o->BlendMeshLight;
 						o->Light[1] *= o->BlendMeshLight;
 						o->Light[2] *= o->BlendMeshLight;
					}
				}
			}
			break;
		case BITMAP_TARGET_POSITION_EFFECT1:
			{
				if( o->SubType == 0 )
				{
					o->Scale -= 0.04f;
					if( o->Scale <= 0.2f )
					{
						o->Live = false;
					}
					if( o->LifeTime <= 10 )
					{
						o->Alpha -= 0.05f;
						o->Light[0] *= o->Alpha;
						o->Light[1] *= o->Alpha;
						o->Light[2] *= o->Alpha;
					}

				}
			}
			break;
		case BITMAP_TARGET_POSITION_EFFECT2:
			{
				if( o->SubType == 0 )
				{
					if( o->Scale >= 1.8f )
					{
						o->m_iAnimation = 0;
					}
					else if( o->Scale <= 0.8f )
					{
						o->m_iAnimation = 1;
					}

					if( o->m_iAnimation == 0 )
					{
						o->Scale -= 0.15f;
					}
					else if( o->m_iAnimation == 1)
					{
						o->Scale += 0.15f;
					}

					if( o->LifeTime <= 10 )
					{
						o->Alpha -= 0.05f;
						o->Light[0] *= o->Alpha;
						o->Light[1] *= o->Alpha;
						o->Light[2] *= o->Alpha;
					}
				}
			}
			break;
		case MODEL_EFFECT_SAPITRES_ATTACK:		// 사피트레스 가시공격
			{
				if( o->LifeTime%6 == 0 )
				{
					o->Position[0] += (float)((rand()%120)-60);
					o->Position[1] += (float)((rand()%120)-60);
					
					CreateEffect( MODEL_EFFECT_SAPITRES_ATTACK_1, o->Position, o->Angle, o->Light, 0, o->Owner );
				}
			}
			break;
		case MODEL_EFFECT_SAPITRES_ATTACK_1:	// 사피트레스 가시 사방으로 날리기
			{
				if( o->SubType == 0 )
				{
					float fMoveSpeed = 40.0f;
					o->Position[0] += (o->Direction[0] * fMoveSpeed );
					o->Position[1] += (o->Direction[1] * fMoveSpeed );
					
					vec3_t vLight;
					Vector( 0.1f, 0.3f, 1.0f, vLight );
					CreateSprite( BITMAP_LIGHT, o->Position, 3.0f, vLight, o );

					CheckTargetRange( o );	

					if( o->LifeTime == 1 )
					{
						for( int i=0 ; i<3 ; i++ )
						{
							CreateEffect( MODEL_EFFECT_SAPITRES_ATTACK_2, o->Position, o->Angle, vLight, 0);
						}
					}			
				}		 
			}
			break;
		case MODEL_EFFECT_THUNDER_NAPIN_ATTACK_1:	// 썬더네이핀 썬더 에너지볼
			{
				if( o->SubType == 0 )
				{
					vec3_t vLight, vLightFlare, vPos;
					Vector(0.4f, 0.7f, 1.0f, vLight);
					Vector(0.1f, 0.2f, 0.8f, vLightFlare);
					VectorCopy( o->Position, vPos );
					if( o->LifeTime >= 4 )
					{
						o->Scale = 0.8f+((float)(rand()%10)*0.1f);
						vPos[0] += (3.0f*((float)(rand()%40)-20));
						vPos[1] += (3.0f*((float)(rand()%40)-20));
						vPos[2] += (3.0f*((float)(rand()%40)-20));
						CreateParticle(BITMAP_LIGHTNING_MEGA1+rand()%3, vPos, o->Angle, vLight, 0, o->Scale);	
					}
					CreateSprite(BITMAP_LIGHT, o->Position, 8.0f, vLightFlare, o );
					CreateSprite(BITMAP_LIGHT, o->Position, 8.0f, vLightFlare, o );
				}
#ifdef LDS_ADD_EG_4_MONSTER_WORLDBOSS_GAIONKALEIN
				else if( o->SubType == 1 )
				{
					const int iOffsetPos = 200;
					vec3_t vLight, vLightFlare, vPos;

					Vector(0.4f, 0.7f, 1.0f, vLight);
					Vector(0.1f, 0.2f, 0.8f, vLightFlare);
					VectorCopy( o->Position, vPos );

					vPos[0] = vPos[0] + ((rand()%iOffsetPos)-(iOffsetPos*0.5f));
					vPos[1] = vPos[1] + ((rand()%iOffsetPos)-(iOffsetPos*0.5f));
					vPos[2] = vPos[2] + ((rand()%(iOffsetPos/2))-((iOffsetPos*0.5f) * 0.5f));
					if( o->LifeTime >= 4 )
					{
						// 임시적
						o->Scale = (0.8f+((float)(rand()%10)*0.1f));
						vPos[0] += (3.0f*((float)(rand()%40)-20));
						vPos[1] += (3.0f*((float)(rand()%40)-20));
						vPos[2] += (3.0f*((float)(rand()%40)-20));
						CreateParticle(BITMAP_LIGHTNING_MEGA1+rand()%3, vPos, o->Angle, vLight, 0, o->Scale);	
					}

					VectorCopy( o->Position, vPos );
					
					vPos[0] = vPos[0] + ((rand()%iOffsetPos)-(iOffsetPos*0.5f));
					vPos[1] = vPos[1] + ((rand()%iOffsetPos)-(iOffsetPos*0.5f));
					vPos[2] = vPos[2] + ((rand()%(iOffsetPos/2))-((iOffsetPos*0.5f) * 0.5f));
					CreateSprite(BITMAP_LIGHT, vPos, 8.0f, vLightFlare, o );
					CreateSprite(BITMAP_LIGHT, vPos, 8.0f, vLightFlare, o );
				}
#endif // LDS_ADD_EG_4_MONSTER_WORLDBOSS_GAIONKALEIN
			}
			break;
#ifdef CSK_EVENT_CHERRYBLOSSOM
		case MODEL_EFFECT_SKURA_ITEM:
			{
#ifdef KJH_ADD_09SUMMER_EVENT
				if( (o->SubType == 0) || (o->SubType == 1))
				{
#endif // KJH_ADD_09SUMMER_EVENT
					if( o->Owner->Live && o->Live )
					{
						vec3_t vRelativePos, vtaWorldPos, vLight1, vLight2;
						BMD *b = &Models[MODEL_EFFECT_SKURA_ITEM];
						
						VectorCopy(o->Owner->Position, o->Position);
						VectorCopy(o->Position,b->BodyOrigin);
						Vector(0.f, 0.f, 0.f, vRelativePos);
						Vector(1.f, 0.6f, 0.8f,o->Light);
						Vector(0.3f, 0.3f, 0.3f,vLight1);
						Vector(1.f, 0.6f, 0.8f, vLight2);
						
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
						float fRot = (WorldTime*0.0006f) * 360.0f;
						float Scale = (rand()%30+50) / 60.f;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
						
						b->Animation(BoneTransform,o->AnimationFrame,o->PriorAnimationFrame,o->PriorAction, o->Angle, o->HeadAngle);
						
						// 왼쪽
						b->TransformPosition(BoneTransform[1], vRelativePos, vtaWorldPos, false);
						CreateSprite(BITMAP_SHINY+1, vtaWorldPos, 1.8f, o->Light, o, +WorldTime*0.08f);
						for(int i=0; i<7; ++i) {
							CreateParticle(BITMAP_SHINY+1, vtaWorldPos, o->Angle, o->Light, 5, 0.8f, o);
							CreateParticle(BITMAP_CHERRYBLOSSOM_EVENT_PETAL, vtaWorldPos, o->Angle, rand()%7 == 3 ? vLight2 : vLight1, 0, 0.5f);
						}
						
						// 오른쪽
						b->TransformPosition(BoneTransform[2], vRelativePos, vtaWorldPos, false);
						CreateSprite(BITMAP_SHINY+1, vtaWorldPos, 1.8f, o->Light, o, -WorldTime*0.08f);
#ifdef _VS2008PORTING
						for(int i=0; i<7; ++i) {
#else // _VS2008PORTING
						for(i=0; i<7; ++i) {
#endif // _VS2008PORTING
							CreateParticle(BITMAP_SHINY+1, vtaWorldPos, o->Angle, o->Light, 5, 0.8f, o);
							CreateParticle(BITMAP_CHERRYBLOSSOM_EVENT_PETAL, vtaWorldPos, o->Angle, rand()%7 == 3 ? vLight2 : vLight1, 0, 0.5f);
						}
							
						// 마지막 끝나는 이펙트
						if( o->LifeTime == 30 || o->LifeTime == 15 || o->LifeTime == 4 )
						{
							vec3_t vAngle;
								
							BMD *characterBMD = &Models[o->Owner->Type];
							VectorCopy(o->Position,characterBMD->BodyOrigin);
								
							characterBMD->Animation(o->Owner->BoneTransform,o->Owner->AnimationFrame,
								o->Owner->PriorAnimationFrame,o->Owner->PriorAction, o->Owner->Angle, o->Owner->HeadAngle);
							characterBMD->TransformPosition(o->Owner->BoneTransform[20], vRelativePos, vtaWorldPos, false);
								
							if( rand()%2 == 0 ) {
								vtaWorldPos[0] += rand()%40+10;
							}
							else {
								vtaWorldPos[0] -= rand()%30+10;
							}
								
							vtaWorldPos[2] += rand()%110+50;
								
#ifdef KJH_ADD_09SUMMER_EVENT
							Vector(0.7f, 0.71f, 1.0f,vLight1);
							Vector(0.8f, 0.85f, 1.0f, vLight2);
#else // KJH_ADD_09SUMMER_EVENT
							Vector(0.8f, 0.6f, 0.6f,vLight1);
							Vector(1.f, 0.6f, 0.8f, vLight2);
#endif // KJH_ADD_09SUMMER_EVENT
								
							for(int k=0; k<70; ++k) 
							{
								if( rand()%3 == 0 ){
									CreateParticle(BITMAP_CHERRYBLOSSOM_EVENT_PETAL, vtaWorldPos, o->Angle, rand()%7 == 3 ? vLight2 : vLight1, 0, 0.4f);
								}
								CreateParticle(BITMAP_CHERRYBLOSSOM_EVENT_FLOWER, vtaWorldPos, o->Angle, rand()%4 == 3 ? vLight2 : vLight1, 0, 0.4f);
							}
								
							Vector(0.f, 1.f, 0.f, vAngle);
							Vector(1.f, 0.6f, 0.8f, vLight2);
							CreateParticle(BITMAP_SHOCK_WAVE, vtaWorldPos, vAngle, vLight2, 4, 0.005f);
								
							// 폭발 시작점
							characterBMD->Animation(o->Owner->BoneTransform,o->Owner->AnimationFrame,
								o->Owner->PriorAnimationFrame,o->Owner->PriorAction, o->Owner->Angle, o->Owner->HeadAngle);
							characterBMD->TransformPosition(o->Owner->BoneTransform[20], vRelativePos, vtaWorldPos, false);
								
							vtaWorldPos[2] += 70;
							Vector(1.f, 0.3f, 0.6f, vLight2);
							CreateSprite(BITMAP_SHINY+1, vtaWorldPos, 1.8f, o->Light, o, -WorldTime*0.08f);
								
							StopBuffer(SOUND_CHERRYBLOSSOM_EFFECT1, true);
							PlayBuffer(SOUND_CHERRYBLOSSOM_EFFECT1, o->Owner);
						}
							
						b->PlayAnimation( &o->AnimationFrame, &o->PriorAnimationFrame, &o->PriorAction, o->Velocity/3.f, o->Position, o->Angle );
					}
#ifdef KJH_ADD_09SUMMER_EVENT
				}
#endif // KJH_ADD_09SUMMER_EVENT
			}
			break;
#endif //CSK_EVENT_CHERRYBLOSSOM

//--------------------------------------------------------------------------------------------------------------------------------

#ifdef CSK_ADD_SKILL_BLOWOFDESTRUCTION
		case MODEL_BLOW_OF_DESTRUCTION:
			{
				if(o->SubType == 0)
				{
					vec3_t vLight;
					if(o->LifeTime <= 24)
					{
						if(o->LifeTime >= 15.f)
						{
							EarthQuake = (float)(rand()%8-4)*0.1f;
						}
						else
						{
							EarthQuake = 0.f;
						}
						
						if(o->LifeTime == 23)
						{
							Vector(0.3f, 0.3f, 1.0f, vLight);
							// 날카로운 빛 모양 이펙트 발생
							CreateEffect(MODEL_NIGHTWATER_01, o->Position, o->Angle, vLight, 0, o);
							CreateEffect(MODEL_NIGHTWATER_01, o->Position, o->Angle, vLight, 0, o);

							CreateEffect(MODEL_KNIGHT_PLANCRACK_A, o->Position, o->Angle, vLight, 0, o, 0, 0, 0, 0, 1.2f);

							vec3_t vDir, vPos, vAngle;
							VectorSubtract(o->StartPosition, o->Position, vDir);
							float fLength = VectorLength(vDir);
							VectorNormalize(vDir);
							int iNum = (int)(fLength / 100) + 1;
							for(int i=0; i<iNum; ++i)
							{
								VectorScale(vDir, 55.f * i, vPos);
								VectorAdd(o->Position, vPos, vPos);
								VectorCopy(o->Owner->Angle, vAngle);
								if(i % 2 == 0)
								{
									vAngle[2] += (rand()%20 + 10);
								}
								else
								{
									vAngle[2] -= (rand()%20 + 10);
								}
								CreateEffect(MODEL_KNIGHT_PLANCRACK_B, vPos, vAngle, vLight, 0, o, 0, 0, 0, 0, 1.0f);
							}
						}
						
						o->Light[0] /= 1.05f;
						o->Light[1] /= 1.05f;
						o->Light[2] /= 1.05f;
					}
				}
#ifdef CSK_RAKLION_BOSS
				else if(o->SubType == 1)
				{
					if(o->LifeTime <= 24)
					{
						if(o->LifeTime >= 15)
						{
							vec3_t vPos, vLight;
							for(int i=0; i<15; ++i)
							{
								VectorCopy(o->Position, vPos);
								vPos[0] += rand()%300 - 150;
								vPos[1] += rand()%300 - 150;
								vPos[2] += rand()%300 - 150;
								float fScale = 1.6f + rand()%10 * 0.1f;
								Vector(0.5f, 0.5f, 1.0f, vLight);
								int index = (rand()%2 == 0) ? BITMAP_WATERFALL_5 : BITMAP_WATERFALL_3;
								CreateParticle(index, vPos, o->Angle, vLight, 8, fScale);
								Vector(1.0f, 1.0f, 1.0f, vLight);
								if(rand()%2 == 0)
								{
									CreateParticle(BITMAP_SMOKE, vPos, o->Angle, vLight, 55, 1.f );
								}
							}
						}

						if(o->LifeTime == 23)
						{
							vec3_t vLight;
							Vector(0.5f, 0.5f, 1.f, vLight);

							Vector(0.3f, 0.3f, 1.0f, vLight);
							// 날카로운 빛 모양 이펙트 발생
							CreateEffect(MODEL_NIGHTWATER_01, o->Position, o->Angle, vLight, 0, o, -1, 0, 0, 0, 2.f);
							CreateEffect(MODEL_NIGHTWATER_01, o->Position, o->Angle, vLight, 0, o, -1, 0, 0, 0, 1.f);

							// 바닥에 큰 가라짐 이펙트 발생
							CreateEffect(MODEL_RAKLION_BOSS_CRACKEFFECT, o->Position, o->Angle, vLight, 0, o, -1, 0, 0, 0, 0.2f);

							vec3_t vPos,vResult;
							vec3_t vAngle;
							float Matrix[3][4];
							for(int i=0; i<(5 + rand()%3); ++i)
							{
								Vector(0.f,(float)(rand()%150),0.f,vPos);
								Vector(0.f,0.f,(float)(rand()%360),vAngle);
								AngleMatrix(vAngle,Matrix);
								VectorRotate(vPos,Matrix,vResult);
								VectorAdd(vResult,o->Position,vResult);
								
      	     					CreateEffect(MODEL_STONE1+rand()%2, vResult, o->Angle, o->Light, 13);
							}
						}

						o->Light[0] /= 1.05f;
						o->Light[1] /= 1.05f;
						o->Light[2] /= 1.05f;
					}
				}
#endif // CSK_RAKLION_BOSS
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
				else if(o->SubType==2)
				{
					vec3_t vLight;
					if(o->LifeTime <= 35.0f && o->LifeTime >= 15.0f)
					{
						EarthQuake = (float)(rand()%8-4)*0.1f;
					}
					else
					{
						EarthQuake = 0.f;
					}
						
					if(o->LifeTime == 30)
					{
						Vector(1.0f, 1.0f, 1.0f, vLight);
						vec3_t vDir, vPos, vAngle;
						VectorSubtract(o->StartPosition, o->Position, vDir);
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
						VectorLength(vDir);
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
						float fLength = VectorLength(vDir);
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
						VectorNormalize(vDir);
						VectorScale(vDir, 55.f, vPos);
						VectorAdd(o->Position, vPos, vPos);
						VectorCopy(o->Owner->Angle, vAngle);
						vAngle[2] += (rand()%40 + 140);
						CreateEffect(MODEL_DRAGON_LOWER_DUMMY, vPos, vAngle, vLight, 0, o, 0, 0, 0, 0, 0.8f);

						Vector(1.0f, 1.0f, 1.0f, vLight);
						CreateEffect(BITMAP_LIGHT_RED, vPos, vAngle, vLight, 4, o,-1,0,0,0, 3.0f);
					}		
				}
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
			}
			break;

//--------------------------------------------------------------------------------------------------------------------------------	
			
		case MODEL_NIGHTWATER_01:
			{
				o->Alpha -= 0.04f;
			}
			break;	

//--------------------------------------------------------------------------------------------------------------------------------		

		case MODEL_KNIGHT_PLANCRACK_A:
			if (o->SubType == 0)
			{
				o->Alpha -= 0.04f;	
			}
#ifdef YDG_ADD_SKILL_LIGHTNING_SHOCK
			else if (o->SubType == 1)
			{
				//if (o->LifeTime < 10)
				{
					o->Alpha *= 0.9f;
				}
			}
#endif	// YDG_ADD_SKILL_LIGHTNING_SHOCK
			break;	

		case MODEL_KNIGHT_PLANCRACK_B:
			{
				o->Alpha -= 0.04f;	
			}
			break;	
#endif // CSK_ADD_SKILL_BLOWOFDESTRUCTION

//--------------------------------------------------------------------------------------------------------------------------------

#ifdef YDG_ADD_SKILL_FLAME_STRIKE
	case MODEL_EFFECT_FLAME_STRIKE:
		{
			if(o->SubType == 0)
			{
				if ((o->LifeTime < 20 && o->Alpha < 0.1f) || o->Owner == NULL)
				{
					EffectDestructor(o);
					break;
				}
				if (o->LifeTime < 20) o->Alpha -= 0.1f;
				else if (o->Alpha < 1.0f) o->Alpha += 0.1f;
				
				OBJECT * pObject = o;
				BMD * pModel = &Models[pObject->Type];
				OBJECT * pOwner = pObject->Owner;
				BMD * pOwnerModel = &Models[pOwner->Type];
				
				// blur
				float Start_Frame = 5.0f;
				float End_Frame = 13.0f;

				if (pOwner->AnimationFrame > End_Frame || pObject->AI == 1)
				{
					pObject->AI = 1;
				}
				else if (pOwner->CurrentAction == PLAYER_SKILL_FLAMESTRIKE)
				{
					pObject->AnimationFrame = pOwner->AnimationFrame;
					
					pOwnerModel->BodyScale     = pOwner->Scale;
					pOwnerModel->CurrentAction = pOwner->CurrentAction;
					VectorCopy(pOwner->Angle, pOwnerModel->BodyAngle);
					VectorCopy(pOwner->Position, pOwnerModel->BodyOrigin);

					pModel->BodyScale = pObject->Scale;
					pModel->CurrentAction = pObject->CurrentAction;
					VectorCopy(pObject->Angle, pModel->BodyAngle);
					VectorCopy(pObject->Position, pModel->BodyOrigin);
					pModel->CurrentAnimation = pObject->AnimationFrame;
					pModel->CurrentAnimationFrame = (int)pObject->AnimationFrame;

					vec3_t  vLight;
					Vector(1.0f, 1.0f, 1.0f, vLight);
					
					vec3_t StartPos, StartRelative;
					vec3_t EndPos, EndRelative;
					
					float fOwnerActionSpeed = pOwnerModel->Actions[pOwnerModel->CurrentAction].PlaySpeed;
					float fOwnerSpeedPerFrame = fOwnerActionSpeed/10.f;
					float fOwnerAnimationFrame = pOwner->AnimationFrame - fOwnerActionSpeed;
					
					float fActionSpeed = pObject->Velocity;
					float fSpeedPerFrame = fActionSpeed/10.f;
					float fAnimationFrame = pObject->AnimationFrame - fActionSpeed;
					
					for(int i=0; i<10; i++) 
					{
						pOwnerModel->Animation(BoneTransform, 
							fOwnerAnimationFrame, (int)fOwnerAnimationFrame - 1, 
							pOwner->PriorAction, pOwner->Angle, pOwner->HeadAngle,false,false);
						pOwnerModel->RotationPosition(BoneTransform[33], p, p);	// ParentMatrix 전역변수 생성용
						
						pModel->Animation(BoneTransform,fAnimationFrame,
							(int)fAnimationFrame - 1,pObject->PriorAction,
							pObject->Angle,pObject->HeadAngle,true,true);	// BoneTransform 전역변수 생성
						
						if (fOwnerAnimationFrame >= Start_Frame && fOwnerAnimationFrame <= End_Frame)
						{
							Vector(0.f, 0.f, 0.f, StartRelative);
							Vector(0.f, 0.f, 0.f, EndRelative);
							pModel->TransformPosition(BoneTransform[9], StartRelative, StartPos, true);
							pModel->TransformPosition(BoneTransform[6], EndRelative, EndPos, true);
							CreateObjectBlur(pObject, StartPos, EndPos, vLight, 2, false, o->m_iAnimation + 1);
							CreateObjectBlur(pObject, StartPos, EndPos, vLight, 2, false, o->m_iAnimation + 2);
							
							pModel->TransformPosition(BoneTransform[8], StartRelative, StartPos, true);
							pModel->TransformPosition(BoneTransform[5], EndRelative, EndPos, true);
							CreateObjectBlur(pObject, StartPos, EndPos, vLight, 5, false, o->m_iAnimation + 3);
						}
						
						fOwnerAnimationFrame += fOwnerSpeedPerFrame;
						fAnimationFrame += fSpeedPerFrame;
					}
				}
			}
		}
		break;
#endif	// YDG_ADD_SKILL_FLAME_STRIKE
		
#ifdef KJH_ADD_SKILL_SWELL_OF_MAGICPOWER		// 마력증대
	case MODEL_SWELL_OF_MAGICPOWER:
		{
			if( o->SubType == 0)
			{
				// 오브젝트가 플레이어가 아닐시에는 break!! (본넘버를 사용하기 때문에 위험!!)
				if( o->Owner->Type != MODEL_PLAYER )
					break;

				BMD* pModel = &Models[o->Owner->Type];	// 케릭터 모델
				VectorCopy( o->Owner->Position, o->Position );
				vec3_t vLight, vPos;

				// 바닥면 효과
				Vector( 0.4f, 0.3f, 0.9f, vLight );
				if( o->LifeTime == 45 || o->LifeTime == 35 || o->LifeTime == 25 )
				{
					CreateEffect( BITMAP_SHOCK_WAVE, o->Position, o->Angle, vLight, 14, o, -1, 0, 0, 0, 5.0f );
					CreateEffect( BITMAP_SHOCK_WAVE, o->Position, o->Angle, vLight, 14, o, -1, 0, 0, 0, 5.0f );
					CreateEffect( BITMAP_TWLIGHT, o->Position, o->Angle, vLight, 3, o, -1, 0, 0, 0, 6.0f );
				}
				
				// 온몸에 빛나는 효과
				Vector( 0.7f, 0.3f, 0.9f, vLight );
				if( o->LifeTime <= 20 )
				{
					vec3_t vDLight;
					float fDynamicLightVal = o->LifeTime*0.05f;
					Vector( fDynamicLightVal*vLight[0], fDynamicLightVal*vLight[1], fDynamicLightVal*vLight[2], vDLight );
					for( int i=0 ; i<pModel->NumBones ; i++ )
					{
						pModel->TransformByObjectBone( vPos, o->Owner, i );	
						
						CreateSprite( BITMAP_LIGHT, vPos, 1.5f, vDLight, o );
					}
				}
				
				// 손에 붙는 이팩트
				Vector( 0.2f, 0.2f, 0.9f, vLight );
				if( o->LifeTime == 45 )
				{
					pModel->TransformByObjectBone( vPos, o->Owner, 28 );
					CreateEffect( MODEL_ARROWSRE06, vPos, o->Angle, vLight, 1, o->Owner, 28/*본넘버*/);
					pModel->TransformByObjectBone( vPos, o->Owner, 37 );
					CreateEffect( MODEL_ARROWSRE06, vPos, o->Angle, vLight, 1, o->Owner, 37/*본넘버*/);
				}

				// 돌기
				if (o->LifeTime >= 30)
				{
					Vector( 0.3f, 0.2f, 0.9f, vLight );
					for( int i=0 ; i<2 ; i++ )
					{
						CreateJoint( BITMAP_2LINE_GHOST, o->Position, o->Position, o->Angle, 1, o, 20.0f+rand()%10, -1, 0, 0, -1, vLight);
					}
				}
			}
		}break;
	case MODEL_ARROWSRE06:
		{
			if( o->SubType == 0)
			{
				// 오브젝트가 플레이어가 아닐시에는 break!! (본넘버를 사용하기 때문에 위험!!)
				if( o->Owner->Type != MODEL_PLAYER )
					break;

				BMD* pModel = &Models[o->Owner->Type];	// 케릭터 모델
				vec3_t vPos;
				pModel->TransformByObjectBone( vPos, o->Owner, o->PKKey );
				VectorCopy( vPos, o->Position );
				o->Scale -= 1.0f;
				if( o->LifeTime <= 15 )
				{
					o->Scale += 0.5f;
				}
			}
			else if( o->SubType == 1 )
			{
				// 오브젝트가 플레이어가 아닐시에는 break!! (본넘버를 때문에 위험!!)
				if( o->Owner->Type != MODEL_PLAYER )
					break;

				BMD* pModel = &Models[o->Owner->Type];	// 케릭터 모델
				vec3_t vPos;
				pModel->TransformByObjectBone( vPos, o->Owner, o->PKKey );
				VectorCopy( vPos, o->Position );
				

				// 크기 늘었다가 줄었다가.
				if( o->LifeTime >= 15 )
				{
					o->Scale *= 1.05f;
				}
				else
				{
					o->Scale *= 0.95f;
				}


				CreateSprite( BITMAP_LIGHT, o->Position, o->Scale, o->Light, o->Owner );
				CreateSprite( BITMAP_LIGHT, o->Position, o->Scale*0.8f, o->Light, o->Owner );

				// 이팩트 죽을때 희미해 지면서...
				if( o->LifeTime <= 10 )
				{
					o->Alpha *= 0.95f;
				}
			}
#ifdef PJH_ADD_PANDA_CHANGERING
			else if( o->SubType == 2 )
			{
				vec3_t vRelativePos;
				BMD* b = &Models[o->Owner->Type];	// 케릭터 모델
				// 오브젝트가 플레이어가 아닐시에는 break!! (본넘버를 때문에 위험!!)
				if( o->Owner->Type != MODEL_PLAYER )
					break;

				// 크기 늘었다가 줄었다가.
				if( o->LifeTime > 15 )
				{
					o->Scale *= 0.9f;
				}
				else
				if( o->LifeTime == 15 )
				{
					o->Scale = 2.f;
				}
				else
				{
					o->Scale *= 0.65f;
				}


				if(o->PKKey == 0)
				{
					Vector(13.f, 14.f, -10.f, vRelativePos);
					b->TransformPosition(o->Owner->BoneTransform[20], vRelativePos, o->Position);
					VectorAdd(o->Position, o->Owner->Position, o->Position);
				}
				else
				{
					Vector(13.f, 14.f, 10.f, vRelativePos);
					b->TransformPosition(o->Owner->BoneTransform[20], vRelativePos, o->Position);
					VectorAdd(o->Position, o->Owner->Position, o->Position);
				}

				CreateSprite( BITMAP_LIGHT, o->Position, o->Scale, o->Light, o->Owner );
				CreateSprite( BITMAP_LIGHT, o->Position, o->Scale*0.8f, o->Light, o->Owner );

				// 이팩트 죽을때 희미해 지면서...
				if( o->LifeTime <= 10 )
				{
					o->Alpha *= 0.95f;
				}
			}
#endif //PJH_ADD_PANDA_CHANGERING
		}break;
	case MODEL_SWELL_OF_MAGICPOWER_BUFF_EFF:
		{
			if( o->SubType == 0 )
			{
				VectorCopy( o->Owner->Position, o->Position );

				int iPastTime = WorldTime - o->Timer;
				
				if( iPastTime >= 6000 )
				{
					vec3_t vLight;
					Vector( 0.2f, 0.2f, 0.9f, vLight );

					// Bip01 R Hand
					CreateEffect( MODEL_ARROWSRE06, o->Position, o->Angle, vLight, 1, o->Owner, 28 );

					// Bip01 L Hand
					CreateEffect( MODEL_ARROWSRE06, o->Position, o->Angle, vLight, 1, o->Owner, 37 );

					o->Timer = WorldTime;

				}

				
// MoveEffect() 에서 CreateSprite()가 깜빡거리는 문제로 RenderEffects() 에서 호출해 주었음.
// 원인을 찾아보자!!!!!
//------------------------------------------------------------------------------------------
// 				// 플레이어일때 이팩트 사용. (본넘버를 사용하기 때문에위험!!!) 
// 				BMD* pModel = &Models[o->Owner->Type];
// 				if ( o->Owner->Type == MODEL_PLAYER)
// 				{
// 					vec3_t vPos, vLight;
// 					Vector(0.7f, 0.2f, 0.9f, vLight);
// 					pModel->TransformByObjectBone( vPos, o->Owner, 27 );			// Bip01 R Forearm
// 					CreateSprite( BITMAP_LIGHT, vPos, 1.0f, vLight, o->Owner );
// 					CreateSprite( BITMAP_LIGHT, vPos, 1.5f, vLight, o->Owner );
// 					pModel->TransformByObjectBone( vPos, o->Owner, 36 );			// Bip01 L Forearm
// 					CreateSprite( BITMAP_LIGHT, vPos, 1.0f, vLight, o->Owner );
// 					CreateSprite( BITMAP_LIGHT, vPos, 1.5f, vLight, o->Owner );
// 				}

				o->LifeTime = 999;
			}
		}break;
#endif // KJH_ADD_SKILL_SWELL_OF_MAGICPOWER

#ifdef ADD_SOCKET_ITEM
	case MODEL_STAR_SHINE:
		{
			if( o->SubType == 0 )
			{
				if( o->LifeTime <= 10 )
				{
					o->Scale *= 0.9f;
					o->Alpha *= 0.9f;
					o->Light[0] *= 0.9f;
					o->Light[1] *= 0.9f;
					o->Light[2] *= 0.9f;
				}
				else if( o->LifeTime <= 20 )
				{
					CreateSprite(BITMAP_SHINY, o->Position, o->Scale, o->Light, o, o->Angle[0]);
				}
				else if( o->LifeTime <= 30 )
				{
					o->Scale *= 1.1f;
					o->Alpha *= 1.1f;
					o->Light[0] *= 1.1f;
					o->Light[1] *= 1.1f;
					o->Light[2] *= 1.1f;
				}
				
				CreateSprite(BITMAP_SHINY, o->Position, o->Scale, o->Light, o, o->Angle[0]);
			}
		}break;
	case MODEL_FEATHER:
		{
			switch( o->SubType )
			{
#ifdef PJH_SEASON4_DARK_NEW_SKILL_CAOTIC
			case 2:
				{
					o->Light[0] *= 0.97f;
					o->Light[1] *= 0.97f;
					o->Light[2] *= 0.97f;
					
					o->Scale *= 0.97f;
					o->Alpha *= 0.97f;
					VectorAdd( o->EyeRight, o->Angle, o->Angle );
				}
				break;
#endif //#ifdef PJH_SEASON4_DARK_NEW_SKILL_CAOTIC
			case 0:
				{
					o->Light[0] *= 0.97f;
					o->Light[1] *= 0.97f;
					o->Light[2] *= 0.97f;
					
					o->Alpha *= 0.97f;

					VectorAdd( o->EyeRight, o->Angle, o->Angle );
				}
				break;
#ifdef PJH_SEASON4_DARK_NEW_SKILL_CAOTIC
			case 3:
				{
					o->Light[0] *= 0.97f;
					o->Light[1] *= 0.97f;
					o->Light[2] *= 0.97f;
					
					o->Alpha *= 0.97f;

					o->Scale *= 0.97f;

					VectorAdd( o->EyeRight, o->Angle, o->Angle );
				}break;
#endif //#ifdef PJH_SEASON4_DARK_NEW_SKILL_CAOTIC
			case 1:
				{
					o->Light[0] *= 0.97f;
					o->Light[1] *= 0.97f;
					o->Light[2] *= 0.97f;
					
					o->Alpha *= 0.97f;

					o->Scale *= 0.99f;

					VectorAdd( o->EyeRight, o->Angle, o->Angle );
				}break;
			}
		}break;
#endif // ADD_SOCKET_ITEM

#ifdef LDS_RAKLION_ADDMONSTER_ICEWALKER
			case MODEL_STREAMOFICEBREATH:
				{
					if( 0 == o->SubType )
					{
						if( o->LifeTime > 1 )
						{
							for( INT i_ = 0; i_ < 4; ++i_ )
							{
								
								const	float RANDOMOFFSET_ANGLE	= 15.0f;
								const	float RANDOMOFFSET_SCALE	= 0.01f;
								const	float RANDOMOFFSET_POSITION	= 20.0f;
								const	float RANDNUM				= ((float)((rand() % 2000)-1000) * 0.001f);
								
								vec3_t		v3ResultAngle, v3ResultPos, v3Light_;
								float		fResultScale;
								float		fCurrentOffsetScale, fCurrentOffsetPos, fCurrentOffsetAngle;
								
								fCurrentOffsetScale	= RANDOMOFFSET_SCALE * RANDNUM ;
								fCurrentOffsetPos	= RANDOMOFFSET_POSITION * RANDNUM ;
								fCurrentOffsetAngle	= RANDOMOFFSET_ANGLE * RANDNUM ;
								
								// 1. 위치값 
								v3ResultPos[0] = o->StartPosition[0] + fCurrentOffsetPos;
								v3ResultPos[1] = o->StartPosition[1] + fCurrentOffsetPos;
								v3ResultPos[2] = o->StartPosition[2] + fCurrentOffsetPos;
								
								// 2. 각도 값
								v3ResultAngle[0] = o->Angle[0] + fCurrentOffsetAngle;
								v3ResultAngle[1] = o->Angle[1] + fCurrentOffsetAngle;
								v3ResultAngle[2] = o->Angle[2] + fCurrentOffsetAngle;
								
								// 3. 크기값
								fResultScale = o->Scale + fCurrentOffsetScale;
								
								// 4. Color 값
								v3Light_[0] = 0.5f; v3Light_[1] = 0.6f; v3Light_[2] = 0.94f; 
								
								CreateParticle( BITMAP_RAKLION_CLOUDS, v3ResultPos, v3ResultAngle, v3Light_, 1, fResultScale );
							}
						}
					}
				}break;
#endif // LDS_RAKLION_ADDMONSTER_ICEWALKER

#ifdef LDS_RAKLION_ADDMONSTER_COOLERTIN
			case MODEL_1_STREAMBREATHFIRE:
				{
					vec3_t	v3Pos;
					VectorCopy( o->Position, v3Pos );
					
					CreateParticle ( BITMAP_WATERFALL_3, v3Pos, o->Angle, o->Light, 11, 0.6f );
					// 연기
					CreateParticle ( BITMAP_SMOKE, v3Pos, o->Angle, o->Light, 52, 0.6f );
					
					
					if( 15 == o->LifeTime )
					{
						// 달 덩이 한개 발사
						const float fForce = 20.0f;
						
						float matRotation[3][4];
						vec3_t vPos;
						vec3_t vDir, vDir_;
						
						Vector( 0.0f, -fForce, 0.0f, vDir );
						AngleMatrix( o->Angle, matRotation );
						
						VectorRotate( vDir, matRotation, vDir_ );
						
						VectorCopy(o->Position, vPos);
						//vPos[2] += 100.f;
						
						CreateEffect( MODEL_MOONHARVEST_MOON, vPos, vDir_, o->Light, 
							1, 
							NULL, -1, 0, 0, 0, 0.3f );
					}
					
				}
				break;
#endif // LDS_RAKLION_ADDMONSTER_COOLERTIN
#ifdef PBG_ADD_PKFIELD
			case MODEL_PKFIELD_ASSASSIN_EFFECT_GREEN_HEAD:
			case MODEL_PKFIELD_ASSASSIN_EFFECT_RED_HEAD:
				{
					o->HeadAngle[2] -= o->Gravity;
					VectorCopy(o->Light,Light);
					
					o->Position[0] += o->HeadAngle[0];
					o->Position[1] += o->HeadAngle[1];
					o->Position[2] += o->HeadAngle[2];
					
					Height = RequestTerrainHeight(o->Position[0],o->Position[1]) +20;
					
					if (o->Position[2] + o->Direction[2] <= Height)
					{
						o->Position[2] = Height;
						o->HeadAngle[0] *= 0.8f;
						o->HeadAngle[1] *= 0.8f;
						o->HeadAngle[2] += 0.6f*o->LifeTime;
						if (o->HeadAngle[2] < 5.0f)
							o->HeadAngle[2] = 0;
						
						o->Alpha -= 0.05f;
					}
					else
					{
						if (o->SubType == 0)
						{
							o->Angle[0] += 0.15f * o->LifeTime;
							o->Angle[1] += 0.15f * o->LifeTime;
						}
						else
						{
							o->Angle[0] -= 0.15f * o->LifeTime;
							o->Angle[1] -= 0.15f * o->LifeTime;
						}
					}

					vec3_t vLook, vPosition, vLight;
					float matRotate[3][4];
					Vector ( 1.f, 1.f, 1.f, vLight );
					Vector(0.0f, 0.0f, 0.0f, vPosition );
					AngleMatrix ( o->Angle, matRotate );
					VectorRotate( vPosition, matRotate, vLook );
					VectorAdd(o->Position, vLook, vPosition);
					if(o->Type == MODEL_PKFIELD_ASSASSIN_EFFECT_GREEN_HEAD)
					{
						CreateEffect(BITMAP_FIRE_CURSEDLICH, vPosition, o->Angle, vLight, 4, o);
					}
					else
					{
						CreateEffect(BITMAP_FIRE_CURSEDLICH, vPosition, o->Angle, vLight, 3, o);
					}
				}
				break;
			case MODEL_PKFIELD_ASSASSIN_EFFECT_GREEN_BODY:
			case MODEL_PKFIELD_ASSASSIN_EFFECT_RED_BODY:
				{
					o->m_iAnimation++;
					
					if(o->LifeTime < 20)
					{
						o->Alpha  = 0.1f * o->LifeTime;
					}
				}
				break;
#endif //PBG_ADD_PKFIELD
#ifdef PBG_ADD_PKFIELD
		case MODEL_LAVAGIANT_FOOTPRINT_R:
		case MODEL_LAVAGIANT_FOOTPRINT_V:	
			{
				o->Angle[0] += 0.1f;
	
				o->Alpha -= 0.01f;
				if(o->Alpha <= 0.0f)
					o->Live = false;

				o->Light[0] *= 0.93f;
				o->Light[1] *= 0.93f;
				o->Light[2] *= 0.93f;
			}
			break;
#endif //PBG_ADD_PKFIELD
#ifdef LDS_ADD_EMPIRE_GUARDIAN
		case MODEL_PROJECTILE:
			{
				if( o->LifeTime > 38 )
				{
					BMD *b = &Models[o->Owner->Type];
					b->Animation(BoneTransform,o->Owner->AnimationFrame,o->Owner->PriorAnimationFrame,o->Owner->PriorAction,o->Owner->Angle,o->Owner->HeadAngle,false,false);
					vec3_t vRelativePos, vWorldPos;
					Vector(0, 0, 0, vRelativePos);
					b->TransformPosition(BoneTransform[9],vRelativePos,vWorldPos,false);
					
					o->Position[0] = vWorldPos[0]+o->Owner->Position[0];
					o->Position[1] = vWorldPos[1]+o->Owner->Position[1];
					o->Position[2] = vWorldPos[2]+o->Owner->Position[2];
				}
				else
				{
					//정해진 시간이 지나면 움직인다.
					if(o->Gravity == 0.0f)
					{
						o->Gravity = 0.1f;
						
						o->Direction[0] = o->Position[0] - o->StartPosition[0];
						o->Direction[1] = o->Position[1] - o->StartPosition[1];
						o->Direction[2] = o->Position[2] - o->StartPosition[2];
						VectorNormalize(o->Direction);
					}
					o->Angle[0] += 0.5f;
					o->Velocity += 0.2f;
					
					o->Position[0] += o->Direction[0] * o->Velocity;
					o->Position[1] += o->Direction[1] * o->Velocity;
					o->Position[2] += o->Direction[2];
				}
			}
			break;
		case MODEL_DOOR_CRUSH_EFFECT_PIECE01:	//성문 파괴 이펙트 조각 1
		case MODEL_DOOR_CRUSH_EFFECT_PIECE02:	//성문 파괴 이펙트 조각 2
		case MODEL_DOOR_CRUSH_EFFECT_PIECE03:	//성문 파괴 이펙트 조각 3
		case MODEL_DOOR_CRUSH_EFFECT_PIECE04:	//성문 파괴 이펙트 조각 4
		case MODEL_DOOR_CRUSH_EFFECT_PIECE05:	//성문 파괴 이펙트 조각 5
		case MODEL_DOOR_CRUSH_EFFECT_PIECE06:	//성문 파괴 이펙트 조각 6
		case MODEL_DOOR_CRUSH_EFFECT_PIECE07:	//성문 파괴 이펙트 조각 7
		case MODEL_DOOR_CRUSH_EFFECT_PIECE08:	//성문 파괴 이펙트 조각 8
		case MODEL_DOOR_CRUSH_EFFECT_PIECE09:	//성문 파괴 이펙트 조각 9
		case MODEL_DOOR_CRUSH_EFFECT_PIECE11:	//성문 파괴 이펙트 조각 7
		case MODEL_DOOR_CRUSH_EFFECT_PIECE12:	//성문 파괴 이펙트 조각 8
		case MODEL_DOOR_CRUSH_EFFECT_PIECE13:	//성문 파괴 이펙트 조각 9
		case MODEL_STATUE_CRUSH_EFFECT_PIECE01:	//석상 파괴 이펙트 조각 1
		case MODEL_STATUE_CRUSH_EFFECT_PIECE02:	//석상 파괴 이펙트 조각 2
		case MODEL_STATUE_CRUSH_EFFECT_PIECE03:	//석상 파괴 이펙트 조각 3
			{
				o->HeadAngle[2] -= o->Gravity;
				VectorCopy(o->Light,Light);
				
				o->Position[0] += o->HeadAngle[0];
				o->Position[1] += o->HeadAngle[1];
				o->Position[2] += o->HeadAngle[2];
				
				Height = RequestTerrainHeight(o->Position[0],o->Position[1]) +20;
				
				if (o->Position[2] + o->Direction[2] <= Height)
				{
					o->Position[2] = Height;
					o->HeadAngle[0] *= 0.8f;
					o->HeadAngle[1] *= 0.8f;
					o->HeadAngle[2] += 0.6f*o->LifeTime;
					if (o->HeadAngle[2] < 5.0f)
						o->HeadAngle[2] = 0;
					
					o->Alpha -= 0.05f;

					if(rand()%4==0)
					{
						Vector(0.f,(float)(rand()%80)+50.f,0.f,p);
						Vector(0.f,0.f,(float)(rand()%360),Angle);
						AngleMatrix(Angle,Matrix);
						VectorRotate(p,Matrix,Position);
						VectorAdd(Position,o->Position,Position);
						
						CreateEffect(MODEL_STONE1+rand()%2,Position,o->Angle,o->Light,0);
					}
				}
				else
				{
					if (o->SubType == 0)
					{
						o->Angle[0] += 0.15f * o->LifeTime;
						o->Angle[1] += 0.15f * o->LifeTime;
					}
					else
					{
						o->Angle[0] -= 0.15f * o->LifeTime;
						o->Angle[1] -= 0.15f * o->LifeTime;
					}
				}

				if(rand()%3==0)
				{
					CreateParticle ( BITMAP_SMOKE+1, o->Position, o->Angle, o->Light, 6 );
				}
			}
			break;
		case MODEL_STATUE_CRUSH_EFFECT_PIECE04:	//석상 파괴 이펙트 조각 4 잔해
		case MODEL_DOOR_CRUSH_EFFECT_PIECE10:	//성문 파괴 이펙트 조각 7 잔해
			{
				o->LifeTime -= 1;

				if(o->LifeTime < 40)
				{
					o->Alpha -= 0.1f;
				}
			}
			break;
#endif //LDS_ADD_EMPIRE_GUARDIAN
#ifdef LDS_ADD_EG_4_MONSTER_WORLDBOSS_GAIONKALEIN
		case MODEL_SWORDLEFT01_EMPIREGUARDIAN_BOSS_GAION_:
		case MODEL_SWORDRIGHT01_EMPIREGUARDIAN_BOSS_GAION_:
		case MODEL_SWORDLEFT02_EMPIREGUARDIAN_BOSS_GAION_:
		case MODEL_SWORDRIGHT02_EMPIREGUARDIAN_BOSS_GAION_:
		case MODEL_SWORDMAIN01_EMPIREGUARDIAN_BOSS_GAION_:
		case MODEL_EMPIREGUARDIANBOSS_FRAMESTRIKE:	
		{
			if (o->SubType == 0)		// 그냥 그리기
			{
			}
			else if ( o->SubType==1 )	// Attack01(일반공격), Attack02(블러드어택), Attack03(프레임스트라이크)
			{
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
				float	fEPSILON = 0.000001f;					// 미세값.	
				float	fRateAlpha_EraseOver = 0.7f;			// 알파 적용 시점.
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
				vec3_t	v3RotateAngleRelative;
				o->Visible = true;		// MoveEffect에서 CreateEffect시 간혹 Move를 타지 않고 그린는 것을 방지.

				// 2. 프레임 RATE %
				float	fCurrentRate = 1.0f - ((float)o->LifeTime / (float)o->ExtState);

				// 3. 보간 반영.
				// ========================================================================================>
				// 4. (1) 보간 반영. - Angle
				if( o->m_Interpolates.m_vecInterpolatesAngle.size() > 0 )
				{
					o->m_Interpolates.GetAngleCurrent(v3RotateAngleRelative ,fCurrentRate);		
					
					o->Angle[0] = v3RotateAngleRelative[0] + o->HeadAngle[0];
					o->Angle[1] = v3RotateAngleRelative[1] + o->HeadAngle[1];
					o->Angle[2] = v3RotateAngleRelative[2] + o->HeadAngle[2];
				}
				
				// 6. (2) 보간 반영. - Position
				if( o->m_Interpolates.m_vecInterpolatesPos.size() > 0 )
				{
					o->m_Interpolates.GetPosCurrent(o->Position,fCurrentRate);
				}

				// 8. (3) 보간 반영. - Scale
				if( o->m_Interpolates.m_vecInterpolatesScale.size() > 0 )
				{
					o->m_Interpolates.GetScaleCurrent(o->Scale,fCurrentRate);
				}

				// 9. (4) 보간 반영. - Alpha
				if( o->m_Interpolates.m_vecInterpolatesScale.size() > 0 )
				{
					o->m_Interpolates.GetAlphaCurrent(o->Alpha,fCurrentRate);
				}
				// 3. 보간 반영.
				// ========================================================================================<

				// 검기 출력 정의.
				// ========================================================================================>

				// 검기 범위 정의.
				float fRateBlurStart, fRateBlurEnd, fRateShadowStart, fRateShadowEnd, fRateJointStart, fRateJointEnd;
				fRateBlurStart = fRateBlurEnd = 0.0f;
				fRateShadowStart = fRateShadowEnd = 0.0f;
				fRateJointStart = fRateJointEnd = 0.0f;
				switch(o->Type)
				{
				case MODEL_SWORDRIGHT01_EMPIREGUARDIAN_BOSS_GAION_:
					{
						// PlayBuffer ( SOUND_ATTACK_FIRE_BUST_EXP ); // 임시사운드
						fRateBlurStart = 0.1f; fRateBlurEnd = 0.90f;
					}
					break;
				case MODEL_SWORDLEFT01_EMPIREGUARDIAN_BOSS_GAION_:
					{
						fRateBlurStart = 0.1f; fRateBlurEnd = 0.90f;
					}
					break;	
				case MODEL_SWORDRIGHT02_EMPIREGUARDIAN_BOSS_GAION_:
					{
						fRateBlurStart = 0.3f; fRateBlurEnd = 1.01f;
						//fRateBlurStart = 0.2f; fRateBlurEnd = 0.90f;

						fRateShadowStart = 0.3f; fRateShadowEnd = 1.01f;
						fRateJointStart = 0.25f; fRateJointEnd = 0.29f;
					}
					break;
				case MODEL_SWORDLEFT02_EMPIREGUARDIAN_BOSS_GAION_:
					{
						fRateBlurStart = 0.3f; fRateBlurEnd = 1.01f;

						fRateShadowStart = 0.3f; fRateShadowEnd = 1.01f;
						fRateJointStart = 0.25f; fRateJointEnd = 0.29f;
					}
					break;
				case MODEL_SWORDMAIN01_EMPIREGUARDIAN_BOSS_GAION_:
					{
						fRateBlurStart = 0.0f; fRateBlurEnd = 0.0f;
					}
					break;
				case MODEL_EMPIREGUARDIANBOSS_FRAMESTRIKE:
					{
						fRateBlurStart = 0.10f; fRateBlurEnd = 1.01f;
//						fRateBlurStart = 0.0f; fRateBlurEnd = 0.0f;
					}
					break;
				}
				// 검기 범위 정의.

				// 검기 테스트용
				int		iTYPESWORDFORCE = 0;		// 1: FORCE OF SWORD 
				int		iTYPESWORDSHADOW = 0;		// 1: SHADOW SWORD 
				int		iTYPESWORDJOINT = 0;		// 1: JOINT OF SWORD

				switch( o->Type )
				{
				case MODEL_EMPIREGUARDIANBOSS_FRAMESTRIKE:			// 플레임스트라이크
					{
						iTYPESWORDFORCE = 1;
						iTYPESWORDSHADOW = 0;
					}
					break;
				case MODEL_SWORDMAIN01_EMPIREGUARDIAN_BOSS_GAION_:	
				case MODEL_SWORDLEFT01_EMPIREGUARDIAN_BOSS_GAION_:	// ATTACK2 : 블러드어택
				case MODEL_SWORDRIGHT01_EMPIREGUARDIAN_BOSS_GAION_:	// ATTACK2
					{
						iTYPESWORDFORCE = 1;
						iTYPESWORDSHADOW = 0;
					}
					break;
				case MODEL_SWORDLEFT02_EMPIREGUARDIAN_BOSS_GAION_:	// ATTACK1 : 일반공격
				case MODEL_SWORDRIGHT02_EMPIREGUARDIAN_BOSS_GAION_:	// ATTACK1 : 일반공격
					{
						iTYPESWORDFORCE = 0;
						iTYPESWORDSHADOW = 1;
						iTYPESWORDJOINT = 0;
					}
					break;	
				}

				// 실 검기 EFFECT 정의.
				
				if( iTYPESWORDFORCE==1 )
				{
					if( fCurrentRate > fRateBlurStart && fCurrentRate < fRateBlurEnd )
					{
						BMD*	b = &Models[o->Type];
						vec3_t  vLightBlur; Vector(1.0f, 1.0f, 1.0f, vLightBlur);
						float	fPreRate = 1.0f - (float)((o->LifeTime)+1)/(float)(o->ExtState);
						SETLIMITS( fPreRate, 1.0f, 0.0f );
						if( fPreRate < fCurrentRate )
						{
							float	fStartRate, fEndRate;						
							
							fStartRate = 1.0f - (float)((o->LifeTime)+2)/(float)(o->ExtState);
							fEndRate = 1.0f - (float)((o->LifeTime)+1)/(float)(o->ExtState);
							

							SETLIMITS( fStartRate, 1.0f, 0.0f );
							SETLIMITS( fEndRate, 1.0f, 0.0f );

							vec3_t	*arrEachBonePos;	// 현재 모델의 각 Bone들의 위치값.
							arrEachBonePos		= new vec3_t[b->NumBones];							
							
							vec3_t v3CurBlurAngle, v3CurBlurPos;
							int	iAccess = 10;
							int iBone01, iBone02;
							int iBlurIdentity, iTypeBlur;
							int iBlurAccessTimeAttk01, 
								iBlurAccessTimeAttk02, 
								iBlurAccessTimeAttk03,
								iBlurAccessTimeAttk04;
							float fUnit;
							float fCurrentRateUnit = fStartRate;
							float fScale = o->Scale;

							iBlurAccessTimeAttk01 = 3;
							iBlurAccessTimeAttk02 = 20;
							iBlurAccessTimeAttk03 = 5;
							iBlurAccessTimeAttk04 = 6;

							switch(o->Type)
							{
							case MODEL_SWORDRIGHT01_EMPIREGUARDIAN_BOSS_GAION_:
								{
									iAccess = iBlurAccessTimeAttk02;
									iBone01 = 3, iBone02 = 12;
									iBlurIdentity = 113;				// 임의의 검기 식별자를 부여.
									//iTypeBlur = 2;
									iTypeBlur = 13;
								}
								break;
							case MODEL_SWORDLEFT01_EMPIREGUARDIAN_BOSS_GAION_:
								{
									iAccess = iBlurAccessTimeAttk02;
									iBone01 = 13, iBone02 = 1;
									iBlurIdentity = 119;				// 임의의 검기 식별자를 부여.
									//iTypeBlur = 2;
									iTypeBlur = 13;
								}
								break;	
							case MODEL_SWORDLEFT02_EMPIREGUARDIAN_BOSS_GAION_:
								{
									iAccess = iBlurAccessTimeAttk01;
									iBone01 = 4, iBone02 = 6;
									iBlurIdentity = 133;				// 임의의 검기 식별자를 부여.
									iTypeBlur = 10;
								}
								break;	
							case MODEL_SWORDRIGHT02_EMPIREGUARDIAN_BOSS_GAION_:
								{
									iAccess = iBlurAccessTimeAttk01;
									iBone01 = 4, iBone02 = 6;
									iBlurIdentity = 122;				// 임의의 검기 식별자를 부여.
									iTypeBlur = 10;
								}
								break;
							case MODEL_EMPIREGUARDIANBOSS_FRAMESTRIKE:	// MainSword는 FrameStrike로 대체된다.
								{
									iAccess = iBlurAccessTimeAttk04;
									iBone01 = 4, iBone02 = 9;
									iBlurIdentity = 155;				// 임의의 검기 식별자를 부여.
									iTypeBlur = 2;
								}
								break;
							}	// switch(o->Type)

							fUnit = (fEndRate - fStartRate) / (float)iAccess;

							
							switch( o->Type )
							{
								case MODEL_SWORDRIGHT01_EMPIREGUARDIAN_BOSS_GAION_:
								case MODEL_SWORDLEFT01_EMPIREGUARDIAN_BOSS_GAION_:
								{
									for(int i=0;i<iAccess;i++) 
									{
										fCurrentRateUnit += fUnit;	
										
										o->m_Interpolates.GetAngleCurrent(v3CurBlurAngle, fCurrentRateUnit);
										o->m_Interpolates.GetPosCurrent(v3CurBlurPos, fCurrentRateUnit);
										
										VectorAdd(v3CurBlurAngle, o->HeadAngle, v3CurBlurAngle);
										
										b->AnimationTransformOnlySelf( 
											arrEachBonePos, v3CurBlurAngle, v3CurBlurPos, fScale );
										
										CreateObjectBlur(o, 
											arrEachBonePos[iBone01], 
											arrEachBonePos[iBone02], 
											vLightBlur, iTypeBlur, false, iBlurIdentity, 25);
										
										if( rand()%2==0 )
										{
											vec3_t	vAngle, vRandomDir, vRandomDirPosition, vResultRandomPosition;
											vec34_t	matRandomRotation;
											vec3_t	vPosition;
											
											if( o->Type == MODEL_SWORDRIGHT01_EMPIREGUARDIAN_BOSS_GAION_ )
											{
												VectorCopy( arrEachBonePos[12], vPosition );
											}
											else
											{
												VectorCopy( arrEachBonePos[1], vPosition );
											}
											
											
											float	fRandDistance = (float)(rand() % 100) + 100;
											Vector(0.0f, fRandDistance, 0.0f,vRandomDir);
											
											CreateParticle(BITMAP_FIRE,vPosition,o->Angle,o->Light,5, 1.35f);
											
											// 효과 : 난수지역에 번개효과
											Vector((float)(rand()%360),0.f,(float)(rand()%360),vAngle);
											AngleMatrix(vAngle,matRandomRotation);
											VectorRotate(vRandomDir,matRandomRotation,vRandomDirPosition);
											VectorAdd(vPosition,vRandomDirPosition,vResultRandomPosition);
											CreateJoint(BITMAP_JOINT_THUNDER,vResultRandomPosition,vPosition,vAngle,3,NULL,10.f,10,10);
										}
									}
								}
								break;
								case MODEL_SWORDRIGHT02_EMPIREGUARDIAN_BOSS_GAION_:
								case MODEL_SWORDLEFT02_EMPIREGUARDIAN_BOSS_GAION_:
								{
									for(int i=0;i<iAccess;i++) 
									{
										fCurrentRateUnit += fUnit;	
										
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
										if( fCurrentRateUnit > fEndRate )
										{
											int iBreakPoint = 0;
										}
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
										
										o->m_Interpolates.GetAngleCurrent(v3CurBlurAngle, fCurrentRateUnit);
										o->m_Interpolates.GetPosCurrent(v3CurBlurPos, fCurrentRateUnit);
										
										VectorAdd(v3CurBlurAngle, o->HeadAngle, v3CurBlurAngle);
										
										b->AnimationTransformOnlySelf( 
											arrEachBonePos, v3CurBlurAngle, v3CurBlurPos, fScale );
										
										CreateObjectBlur(o, 
											arrEachBonePos[iBone01], 
											arrEachBonePos[iBone02], 
											vLightBlur, iTypeBlur, false, iBlurIdentity, 3);
										
										// 블러드 어택 후기 이펙트 부분
										if( o->Type == MODEL_SWORDRIGHT01_EMPIREGUARDIAN_BOSS_GAION_ || 
											o->Type == MODEL_SWORDLEFT01_EMPIREGUARDIAN_BOSS_GAION_ )
										{
											vec3_t	vAngle, vRandomDir, vRandomDirPosition, vResultRandomPosition;
											vec34_t	matRandomRotation;
											vec3_t	vPosition;
											
											if( o->Type == MODEL_SWORDRIGHT01_EMPIREGUARDIAN_BOSS_GAION_ )
											{
												VectorCopy( arrEachBonePos[12], vPosition );
											}
											else
											{
												VectorCopy( arrEachBonePos[1], vPosition );
											}
											
											
											float	fRandDistance = (float)(rand() % 100) + 100;
											Vector(0.0f, fRandDistance, 0.0f,vRandomDir);
											
											CreateParticle(BITMAP_FIRE,vPosition,o->Angle,o->Light,0);
											
											// 효과 : 난수지역에 번개효과
											Vector((float)(rand()%360),0.f,(float)(rand()%360),vAngle);
											AngleMatrix(vAngle,matRandomRotation);
											VectorRotate(vRandomDir,matRandomRotation,vRandomDirPosition);
											VectorAdd(vPosition,vRandomDirPosition,vResultRandomPosition);
											CreateJoint(BITMAP_JOINT_THUNDER,vResultRandomPosition,vPosition,vAngle,3,NULL,10.f,10,10);
										}
									}
								}
								break;
								case MODEL_EMPIREGUARDIANBOSS_FRAMESTRIKE:
								{

									float			fRateBlur = 0.f;
									
									for(int i=0;i<iAccess;i++) 
									{
										fCurrentRateUnit += fUnit;
										
										o->m_Interpolates.GetAngleCurrent(v3CurBlurAngle, fCurrentRateUnit);
										o->m_Interpolates.GetPosCurrent(v3CurBlurPos, fCurrentRateUnit);
										
										VectorAdd(v3CurBlurAngle, o->HeadAngle, v3CurBlurAngle);
										
										fRateBlur = (float)i / (float)iAccess;
										b->AnimationTransformOnlySelf( 
											arrEachBonePos, v3CurBlurAngle, v3CurBlurPos, o->Scale, o, o->Velocity, fRateBlur );
										
										CreateObjectBlur(o, arrEachBonePos[iBone01], 
											arrEachBonePos[iBone02], vLightBlur, 13, false, iBlurIdentity+2, 11);
									}
								}
								break;
							}	// Switch(o->Type)

							delete [] arrEachBonePos;
						}	// if( fPreRate < fCurrentRate )
					}
				}
					
				if( iTYPESWORDSHADOW==1 )
				{
					if( fCurrentRate > fRateShadowStart && fCurrentRate < fRateShadowEnd )
					{
						CreateEffect( o->Type, o->Position, o->Angle, o->Light, 20, o, -1, 0, 0, 0, o->Scale );
					}
				}

				if( iTYPESWORDJOINT==1 )
				{
					if( fCurrentRate > fRateJointStart && fCurrentRate < fRateJointEnd )
					{
						// 각 검별로 JOINT 생성
						CreateJoint ( BITMAP_FLARE+1, o->Position, o->Position, o->Angle, 20, o, 160.f, 40 );
					}
				}
				
				// 실 검기 EFFECT 정의.
				// 검기 출력 정의 완료.
				// ========================================================================================<


				// - 20. POST EFFECT 처리 : 맵 바닥위 LIGHT COLOR
				// ========================================================================================>
				switch(o->Type)
				{
					case MODEL_SWORDRIGHT01_EMPIREGUARDIAN_BOSS_GAION_:
					case MODEL_SWORDLEFT01_EMPIREGUARDIAN_BOSS_GAION_:
					case MODEL_SWORDRIGHT02_EMPIREGUARDIAN_BOSS_GAION_:
					case MODEL_SWORDLEFT02_EMPIREGUARDIAN_BOSS_GAION_:
					{						
						vec3_t	v3LightTerrain;
						switch(o->Type)
						{
							case MODEL_SWORDRIGHT01_EMPIREGUARDIAN_BOSS_GAION_:
							{
								Vector(0.0f, 0.2f, 0.9f, v3LightTerrain);
								AddTerrainLight(o->Position[0],o->Position[1],v3LightTerrain,2,PrimaryTerrainLight);
							}
							break;
							case MODEL_SWORDLEFT01_EMPIREGUARDIAN_BOSS_GAION_:
							{
								Vector(0.0f, 0.2f, 0.9f, v3LightTerrain);
								AddTerrainLight(o->Position[0],o->Position[1],v3LightTerrain,2,PrimaryTerrainLight);
							}
							break;
							case MODEL_SWORDRIGHT02_EMPIREGUARDIAN_BOSS_GAION_:
							{
								Vector(0.0f, 0.2f, 0.9f, v3LightTerrain);
								AddTerrainLight(o->Position[0],o->Position[1],v3LightTerrain,2,PrimaryTerrainLight);
							}
							break;
							case MODEL_SWORDLEFT02_EMPIREGUARDIAN_BOSS_GAION_:
							{
								Vector(0.0f, 0.2f, 0.9f, v3LightTerrain);
								AddTerrainLight(o->Position[0],o->Position[1],v3LightTerrain,2,PrimaryTerrainLight);
							}
							break;
						}
					}
					break;
					case MODEL_SWORDMAIN01_EMPIREGUARDIAN_BOSS_GAION_:
					{
						// 3. 검 등장시에 APPEAR EFFECT들
						if( fCurrentRate >= 0.0f && fCurrentRate <= 0.6f )
						{
							o->Visible = true;		// MoveEffect에서 CreateEffect시 간혹 Move를 타지 않고 그린는 것을 방지.
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
							const int iNumUnitBone = 11;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
							BMD*	b = &Models[o->Type];
							vec3_t	*arrEachBonePos;			// 현재 모델의 각 Bone들의 위치값.
							vec3_t	v3LightModify;

#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
							int		arriBoneIdxCur [] = {10, 11, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
							
							// 1. BonePosition Particle 처리
							arrEachBonePos = new vec3_t[b->NumBones];


							// - APPEAR WITH FIRE EFFECT
							vec3_t	vRelativePos, vAngle;
							vec3_t	v3CurrentHighHierarchyNodePos;		// 상위 모델의 ATTACH 본 위치값.
							
							Vector(4.f, 0.f, 0.0f, vRelativePos);
							VectorCopy(o->Angle,vAngle);

							BMD *pBMDSwordModel = &Models[o->Type]; 					
							BMD *pOwnerModel = &Models[o->Owner->Type];

							int arrBoneIdxs[] = { 4, 2, 8, 10, 6 };		// 각 모델별 참조해야할 가이온의 본 INDEX. 
							int iBoneIdx = arrBoneIdxs[o->Type-MODEL_SWORDLEFT01_EMPIREGUARDIAN_BOSS_GAION_];
							
							pBMDSwordModel->AnimationTransformWithAttachHighModel( o->Owner, pOwnerModel, iBoneIdx, 
													v3CurrentHighHierarchyNodePos, arrEachBonePos );
							
							// - 등장 END EFFECT 정의
							vec3_t	v3LightTerrain;
							Vector(0.9f, 0.4f, 0.1f, v3LightTerrain);
							
							AddTerrainLight(o->Position[0],o->Position[1],v3LightTerrain,2,PrimaryTerrainLight);							
							
							{
								Vector( 1.0f, 1.0f, 1.0f, v3LightModify );		// 붉은색 계열
							
								for( int j = 0; j < 11; ++j )
								{
									if( rand()%20 == 0 )
									{
										CreateEffect(BITMAP_FIRE_CURSEDLICH,arrEachBonePos[j],vAngle,v3LightModify,12,o,-1,0,0,0,o->Scale);
									}
								}
							}
							
							delete [] arrEachBonePos;
						}	// if( fCurrentRate >= 0.0f && fCurrentRate <= 0.01f )
					}
					break;
					case MODEL_EMPIREGUARDIANBOSS_FRAMESTRIKE:
					{
						// 플레임스트라이크 등장시 화면상에 불꽃 타오르는 이펙트.
						if( fCurrentRate >= 0.0f && fCurrentRate <= 0.5f )
						{
							// - APPEAR WITH FIRE EFFECT
							vec3_t	vAngle;
							vec3_t	v3LightModify, v3PosModify;
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
							float	fParamBasis = 100.0f, fParamUnit = 50.0f;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX

							// 1. BonePosition Particle 처리
							BMD	*b = &Models[o->Type];
							vec3_t* arrEachBonePos = new vec3_t[b->NumBones];

							b->AnimationTransformOnlySelf( arrEachBonePos, o->Angle, o->Position, o->Scale, o );
							
							VectorCopy(o->Angle,vAngle);
							Vector( 1.0f, 1.0f, 1.0f, v3LightModify );

							for( int i = 0; i < b->NumBones; i++ )
							{
								v3PosModify[0] = arrEachBonePos[i][0] + (float)((rand() % 80) - 40);
								v3PosModify[1] = arrEachBonePos[i][1] + (float)((rand() % 80) - 40);
								v3PosModify[2] = arrEachBonePos[i][2] + (float)((rand() % 80) - 40);
								if( rand() % 10 == 0 )
								{
									CreateEffect(BITMAP_FIRE_CURSEDLICH,v3PosModify,vAngle,v3LightModify,12,o,-1,0,0,0,o->Scale * 1.5f);
								}
							}

							delete [] arrEachBonePos;
						}
							

						// - 맵상에 검기COLOR 비주얼 렌더링.
						vec3_t	v3LightTerrain;
						Vector(0.9f, 0.4f, 0.1f, v3LightTerrain);
						
						AddTerrainLight(o->Position[0],o->Position[1],v3LightTerrain,2,PrimaryTerrainLight);
					}
					break;	
				}
				// - 20. POST EFFECT 처리
				// ========================================================================================<

				// - 24. END EFFECT 처리
				// ========================================================================================>
				switch( o->Type )
				{
				case MODEL_SWORDLEFT01_EMPIREGUARDIAN_BOSS_GAION_:
				case MODEL_SWORDRIGHT01_EMPIREGUARDIAN_BOSS_GAION_:
					{
						if( o->LifeTime == 5 )
						{
							CreateEffect ( o->Type, o->Position, o->Angle, o->Light, 11, o->Owner,-1,0,0,0,o->Scale );
						}
					}
					break;
				case MODEL_SWORDRIGHT02_EMPIREGUARDIAN_BOSS_GAION_:	
				case MODEL_SWORDLEFT02_EMPIREGUARDIAN_BOSS_GAION_:
					{
						if( o->LifeTime == 3 )
						{
							CreateEffect ( o->Type, o->Position, o->Angle, o->Light, 11, o->Owner,-1,0,0,0,o->Scale );
						}
					}
					break;
				case MODEL_EMPIREGUARDIANBOSS_FRAMESTRIKE:
					{
						if( o->LifeTime == 10 )
						{
							CreateEffect ( MODEL_SWORDMAIN01_EMPIREGUARDIAN_BOSS_GAION_, 
								o->Position, o->Angle, o->Light, 11, o->Owner,-1,0,0,0,o->Scale );
						}
					}
					break;	
				case MODEL_SWORDMAIN01_EMPIREGUARDIAN_BOSS_GAION_:	// 메인스워드는 프레임스트라이크를 호출
					{
						if(o->LifeTime==1)
						{
							CreateEffect ( MODEL_EMPIREGUARDIANBOSS_FRAMESTRIKE, 
								o->Position, o->Angle, o->Light, 1, 
								o->Owner,-1,0,0,0,o->Scale );
						}
					}
					break;
				}
				// - 24. END EFFECT 처리
				// ========================================================================================<

				// - 28. SOUND 처리
				// ========================================================================================>
				switch(o->Type)
				{
				case MODEL_SWORDRIGHT01_EMPIREGUARDIAN_BOSS_GAION_:
				case MODEL_SWORDLEFT01_EMPIREGUARDIAN_BOSS_GAION_:
					{
						// 28-1. 블러드 어택 : 첫 프레임 들어갈때 한번만 호출한다.
						if( o->LifeTime == o->ExtState -1 )
						{
							//PlayBuffer( SOUND_ASSASSIN );		// 칼 날라가는 사운드
							PlayBuffer( SOUND_BLOODATTACK );
						}
					}
					break;
				case MODEL_SWORDRIGHT02_EMPIREGUARDIAN_BOSS_GAION_:					
				case MODEL_SWORDLEFT02_EMPIREGUARDIAN_BOSS_GAION_:
					{
						// 28-2. 일반 공격 : 첫 프레임 들어갈때 한번만 호출한다.
						if( o->LifeTime == o->ExtState -1 )
						{
							PlayBuffer( SOUND_ATTACK01+4 );
							//PlayBuffer( SOUND_ASSASSIN );		// 칼 날라가는 사운드
						}
					}
					break;
				case MODEL_EMPIREGUARDIANBOSS_FRAMESTRIKE:
					{
						// 28-3. 플레임스트라이크 : 첫 프레임 들어갈때 한번만 호출한다.
						if( o->LifeTime == o-> ExtState - 5 )
						{
							PlayBuffer( SOUND_SKILL_FLAME_STRIKE );	// 플레임스트라이크 사운드
						}
					}
					break;
				}
				
			}
			else if( o->SubType==3 )	// 기간틱스톰 이펙트
			{
				o->Visible = true;		// MoveEffect에서 CreateEffect시 간혹 Move를 타지 않고 그린는 것을 방지.
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
				float	fEPSILON = 0.000001f;					// 미세값.	
				float	fRateAlpha_EraseOver = 0.7f;			// 알파 적용 시점.
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
				vec3_t	v3RotateAngleRelative;

				// 2. 프레임 RATE %
				float	fCurrentRate = 1.0f - ((float)o->LifeTime / (float)o->ExtState);


				// 3. 보간 반영.
				// ========================================================================================>
				// 4. (1) 보간 반영. - Angle
				if( o->m_Interpolates.m_vecInterpolatesAngle.size() > 0 )
				{
					o->m_Interpolates.GetAngleCurrent(v3RotateAngleRelative ,fCurrentRate);		
					
					o->Angle[0] = v3RotateAngleRelative[0] + o->HeadAngle[0];
					o->Angle[1] = v3RotateAngleRelative[1] + o->HeadAngle[1];
					o->Angle[2] = v3RotateAngleRelative[2] + o->HeadAngle[2];
				}
				
				// 6. (2) 보간 반영. - Position
				if( o->m_Interpolates.m_vecInterpolatesPos.size() > 0 )
				{
					o->m_Interpolates.GetPosCurrent(o->Position,fCurrentRate);
				}

				// 8. (3) 보간 반영. - Scale
				if( o->m_Interpolates.m_vecInterpolatesScale.size() > 0 )
				{
					o->m_Interpolates.GetScaleCurrent(o->Scale,fCurrentRate);
				}

				// 9. (4) 보간 반영. - Alpha
				if( o->m_Interpolates.m_vecInterpolatesAlpha.size() > 0 )
				{
					o->m_Interpolates.GetAlphaCurrent(o->Alpha,fCurrentRate);
				}
				// 3. 보간 반영.
				// ========================================================================================<

				// - 20. POST EFFECT 처리
				// ========================================================================================>
				switch(o->Type)
				{
					case MODEL_SWORDRIGHT01_EMPIREGUARDIAN_BOSS_GAION_:
					case MODEL_SWORDLEFT01_EMPIREGUARDIAN_BOSS_GAION_:
					case MODEL_SWORDRIGHT02_EMPIREGUARDIAN_BOSS_GAION_:
					case MODEL_SWORDLEFT02_EMPIREGUARDIAN_BOSS_GAION_:
					{
						// - 각SWORD별 POSTEFFECT 일시 보류.
						if(o->LifeTime==0)
						{
							OBJECT	*oHighHier = o->Owner;
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
							BMD		*bHighHier = &Models[oHighHier->Type];
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
							
							vec3_t	v3RelativePos, v3OwnerBonePos;
							Vector(0.0f, 0.0f, 0.0f ,v3RelativePos);
							Vector(0.0f, 0.0f, 0.0f ,v3OwnerBonePos);
							
							CreateEffect ( o->Type, oHighHier->Position, oHighHier->Angle, oHighHier->Light, 
								11, o->Owner,-1,0,0,0,o->Scale );
						}
						
						// - 맵 바닥위 COLOR.
						vec3_t	v3LightTerrain;
						Vector(0.0f, 0.2f, 0.9f, v3LightTerrain);
						AddTerrainLight(o->Position[0],o->Position[1],v3LightTerrain,2,PrimaryTerrainLight);
					}
					break;
					case MODEL_SWORDMAIN01_EMPIREGUARDIAN_BOSS_GAION_:
					{
						if( o->LifeTime == 40)
						{
							// --------------------------------------------------- 기간틱 이펙트
							vec34_t Matrix;
							vec3_t vAngle, vDirection, vPosition, vLight;
							float fAngle;

							Vector(1.0f, 1.0f, 1.0f, vLight);
							for (int i = 0; i < 5; ++i)
							{
								Vector(0.f, 200.f, 0.f, vDirection );
								fAngle = o->Angle[2] + i * 72.f;
								Vector(0.f, 0.f, fAngle, vAngle);
								AngleMatrix(vAngle, Matrix);
								VectorRotate(vDirection, Matrix, vPosition);
								VectorAdd(vPosition, o->Light, vPosition);
								
								CreateEffect(BITMAP_JOINT_THUNDER, vPosition, o->Angle, vLight, 1);
							}

							// --------------------------------------------------- 추가 이펙트
							Vector ( 1.f, 1.f, 1.f, vLight );
							VectorCopy(o->Light, vPosition);
							//Vector(0.0f, -250.0f, 0.0f, vPosition );
							//AngleMatrix ( pObject->Angle, matRotate );
							//VectorRotate( vPosition, matRotate, vLook );
							//VectorAdd( pObject->Position, vLook, vPosition );
							
						
							// 땅파이는 이펙트
							CreateEffect( BITMAP_CRATER, vPosition,   o->Angle, vLight, 2, NULL, -1, 0, 0, 0, 1.5f );
							for(int iu = 0; iu < 20; iu++)
							{
								CreateEffect ( MODEL_STONE2, vPosition,o->Angle,vLight );
							}
							
							// 연기 이펙트
							vec3_t	v3Pos;
							int		iLimitPos = 200;

							Vector( 0.7f, 0.7f, 1.f, vLight );

							v3Pos[0] = vPosition[0] + ((rand()%iLimitPos)-(iLimitPos*0.5f));
							v3Pos[1] = vPosition[1] + ((rand()%iLimitPos)-(iLimitPos*0.5f));
							v3Pos[2] = vPosition[2] + ((rand()%iLimitPos)-(iLimitPos*0.5f));
							CreateParticle(BITMAP_CLUD64, v3Pos, o->Angle, vLight, 7, 2.0f);

							v3Pos[0] = vPosition[0] + ((rand()%iLimitPos)-(iLimitPos*0.5f));
							v3Pos[1] = vPosition[1] + ((rand()%iLimitPos)-(iLimitPos*0.5f));
							v3Pos[2] = vPosition[2] + ((rand()%iLimitPos)-(iLimitPos*0.5f));
							CreateParticle(BITMAP_CLUD64, v3Pos, o->Angle, vLight, 7, 2.0f);

							v3Pos[0] = vPosition[0] + ((rand()%iLimitPos)-(iLimitPos*0.5f));
							v3Pos[1] = vPosition[1] + ((rand()%iLimitPos)-(iLimitPos*0.5f));
							v3Pos[2] = vPosition[2] + ((rand()%iLimitPos)-(iLimitPos*0.5f));
							CreateParticle(BITMAP_CLUD64, v3Pos, o->Angle, vLight, 7, 2.0f);

							v3Pos[0] = vPosition[0] + ((rand()%iLimitPos)-(iLimitPos*0.5f));
							v3Pos[1] = vPosition[1] + ((rand()%iLimitPos)-(iLimitPos*0.5f));
							v3Pos[2] = vPosition[2] + ((rand()%iLimitPos)-(iLimitPos*0.5f));
							CreateParticle(BITMAP_CLUD64, v3Pos, o->Angle, vLight, 7, 2.0f);

							v3Pos[0] = vPosition[0] + ((rand()%iLimitPos)-(iLimitPos*0.5f));
							v3Pos[1] = vPosition[1] + ((rand()%iLimitPos)-(iLimitPos*0.5f));
							v3Pos[2] = vPosition[2] + ((rand()%iLimitPos)-(iLimitPos*0.5f));
							CreateParticle(BITMAP_CLUD64, v3Pos, o->Angle, vLight, 7, 2.0f);

							v3Pos[0] = vPosition[0] + ((rand()%iLimitPos)-(iLimitPos*0.5f));
							v3Pos[1] = vPosition[1] + ((rand()%iLimitPos)-(iLimitPos*0.5f));
							v3Pos[2] = vPosition[2] + ((rand()%iLimitPos)-(iLimitPos*0.5f));
							CreateParticle(BITMAP_CLUD64, v3Pos, o->Angle, vLight, 7, 2.0f);

							
							// 충격파
							Vector( 0.3f, 0.2f, 1.f, vLight );
							CreateEffect( BITMAP_SHOCK_WAVE, vPosition, o->Angle, vLight, 11 );
							CreateEffect( BITMAP_SHOCK_WAVE, vPosition, o->Angle, vLight, 11 );
							
							// 에너지볼(전기)
							vPosition[2] += 100.0f;
							Vector ( 0.0f, 0.2f, 1.0f, vLight );
							CreateEffect(MODEL_EFFECT_THUNDER_NAPIN_ATTACK_1, vPosition, o->Angle, vLight, 1, NULL, -1, 0, 0, 0, 1.0f );
							// --------------------------------------------------- 기간틱 이펙트 /까지
						}
					}
					break;
				}
				
				// - 24. END EFFECT 처리 : 기간틱스톰
				// ========================================================================================>		
				if( o->LifeTime == 15 )
				{
					CreateEffect ( o->Type, o->Position, o->Angle, o->Light, 12, o->Owner,-1,0,0,0,o->Scale );
				}

				int	iSwordOnTheLand = (o->ExtState /2) + 5;
				if( o->LifeTime <= iSwordOnTheLand && o->LifeTime > 10 )
				{
					EarthQuake = (float)(rand()%2-2)*0.5f;
				}
				// - 24. END EFFECT 처리 : 기간틱스톰
				// ========================================================================================<

				// - 28. SOUND 처리 부분 : 기간틱스톰
				if( o->LifeTime == o->ExtState )
				{
					PlayBuffer( SOUND_ASSASSIN );		// 칼 날라가는 사운드
				}

				if( o->LifeTime == o->ExtState /2 + 2 )
				{
					PlayBuffer( SOUND_SKILL_GIGANTIC_STORM );
				}

				if( o->LifeTime == (o->ExtState /2) + 5  )
				{
					PlayBuffer( SOUND_FURY_STRIKE2 );	// 칼 꽂히는 사운드
				}
			}
			else if( o->SubType==11 )		// END EFFECT 렌더링
			{
				// 2. 프레임 RATE %
				float	fCurrentRate = 1.0f - ((float)o->LifeTime / (float)o->ExtState);

				// 9. (4) 보간 반영. - Alpha
				if( o->m_Interpolates.m_vecInterpolatesScale.size() > 0 )
				{
					o->m_Interpolates.GetAlphaCurrent(o->Alpha,fCurrentRate);
				}
				
				// 13. 검 등장시에 APPEAR EFFECT들
				if( fCurrentRate >= 0.0f && fCurrentRate <= 0.5f )
				{
					o->Visible = true;		// MoveEffect에서 CreateEffect시 간혹 Move를 타지 않고 그린는 것을 방지.
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
					const int iNumUnitBone = 11;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
					BMD*	b = &Models[o->Type];
					vec3_t	*arrEachBonePos;			// 현재 모델의 각 Bone들의 위치값.
					vec3_t	v3LightModify;
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
					int		arriBoneIdx[5][11] = {	{ 2, 3, 13, 12, 5, 6, 1, 8, 9, 10, 4 }, 
													{ 9, 10, 13, 6, 1, 7, 2, 4, 3, 11, 8 }, 
													{ 1, 2, 3, 4, 5, 6, 12, 8, 9, 10, 7 },
													{3, 10, 4, 12, 2, 9, 5, 7, 6, 8, 1 },
													{10, 11, 1, 2, 3, 4, 5, 6, 7, 8, 9 } };
					int		iOffset = o->Type - MODEL_SWORDLEFT01_EMPIREGUARDIAN_BOSS_GAION_;
					int		*arriBoneIdxCur = arriBoneIdx[iOffset];
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
					
					// 1. BonePosition Particle 처리
					arrEachBonePos = new vec3_t[b->NumBones];


					// - APPEAR WITH FIRE EFFECT
					vec3_t	vRelativePos, vAngle;
					vec3_t	v3CurrentHighHierarchyNodePos;		// 상위 모델의 ATTACH 본 위치값.
					
					Vector(4.f, 0.f, 0.0f, vRelativePos);
					VectorCopy(o->Angle,vAngle);

					BMD *pBMDSwordModel = &Models[o->Type]; 					
					BMD *pOwnerModel = &Models[o->Owner->Type];

					int arrBoneIdxs[] = { 4, 2, 8, 10, 6 };		// 각 모델별 참조해야할 가이온의 본 INDEX. 
					int iBoneIdx = arrBoneIdxs[o->Type-MODEL_SWORDLEFT01_EMPIREGUARDIAN_BOSS_GAION_];
					
					pBMDSwordModel->AnimationTransformWithAttachHighModel( o->Owner, pOwnerModel, iBoneIdx, 
											v3CurrentHighHierarchyNodePos, arrEachBonePos );
					
					// - 등장 END EFFECT 정의
					vec3_t	v3LightTerrain;
					Vector(0.9f, 0.4f, 0.1f, v3LightTerrain);
					
					AddTerrainLight(o->Position[0],o->Position[1],v3LightTerrain,2,PrimaryTerrainLight);

					switch( o->Type )
					{
					case MODEL_SWORDLEFT01_EMPIREGUARDIAN_BOSS_GAION_:
					case MODEL_SWORDRIGHT01_EMPIREGUARDIAN_BOSS_GAION_:
						{
// 							Vector( 1.0f, 0.4f, 0.3f, v3LightModify );		// 파란색 계열.
// 							
// 							for( int j = 0; j < 11; ++j )
// 							{
// 								if( rand()%2 == 0 )
// 								{
// 									CreateParticle(BITMAP_FIRE_HIK1_MONO,arrEachBonePos[j],o->Angle,v3LightModify,10,o->Scale);
// 								}								
// 							}

							for( int j = 0; j < 11; ++j )
							{
								Vector(1.0f, 1.0f, 1.0f, v3LightModify);
								if( rand()%2 == 0 )
								{
									CreateParticle(BITMAP_POUNDING_BALL,arrEachBonePos[j],o->Angle,v3LightModify, 3, 1.3f );
								}
							}
						}
						break;
					case MODEL_SWORDLEFT02_EMPIREGUARDIAN_BOSS_GAION_:
					case MODEL_SWORDRIGHT02_EMPIREGUARDIAN_BOSS_GAION_:
						{
							Vector( 0.3f, 0.4f, 1.0f, v3LightModify );		// 파란색 계열.
							
							for( int j = 0; j < 11; ++j )
							{
								if( rand()%2 == 0 )
								{
									CreateParticle(BITMAP_FIRE_HIK1_MONO,arrEachBonePos[j],o->Angle,v3LightModify,10,o->Scale);
								}								
							}
						}
						break;
					case MODEL_SWORDMAIN01_EMPIREGUARDIAN_BOSS_GAION_:
						{
							Vector( 1.0f, 1.0f, 1.0f, v3LightModify );		// 붉은색 계열
						
							for( int j = 0; j < 11; ++j )
							{
								if( rand()%20 == 0 )
								{
									CreateEffect(BITMAP_FIRE_CURSEDLICH,arrEachBonePos[j],vAngle,v3LightModify,12,o,-1,0,0,0,o->Scale);
								}
							}
						}
						break;
					default:
						{
							
						}
						break;
					}
					// 3. 검 등장시에 APPEAR EFFECT들
					
					delete [] arrEachBonePos;
				}	// if( fCurrentRate >= 0.0f && fCurrentRate <= 0.01f )
				// 


				// 2. ALpha 가감쇠 처리 - 보간법
				if( o->m_Interpolates.m_vecInterpolatesAlpha.size() > 0 )
				{
					o->m_Interpolates.GetAlphaCurrent(o->Alpha,fCurrentRate);
				}
			} // else if( o->SubType==11 )
			else if( o->SubType==12 )		// END EFFECT 렌더링 - 기간틱스톰만 해당.
			{
				// 2. 프레임 RATE %
				float	fCurrentRate = 1.0f - ((float)o->LifeTime / (float)o->ExtState);

				// 9. (4) 보간 반영. - Alpha
				if( o->m_Interpolates.m_vecInterpolatesScale.size() > 0 )
				{
					o->m_Interpolates.GetAlphaCurrent(o->Alpha,fCurrentRate);
				}
				
				// 3. 검 등장시에 APPEAR EFFECT들
				if( fCurrentRate >= 0.0f && fCurrentRate <= 0.6f )
				{
					o->Visible = true;		// MoveEffect에서 CreateEffect시 간혹 Move를 타지 않고 그린는 것을 방지.
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
					const int iNumUnitBone = 11;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
					BMD*	b = &Models[o->Type];
					vec3_t	*arrEachBonePos;			// 현재 모델의 각 Bone들의 위치값.
					vec3_t	v3LightModify;
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
					int		arriBoneIdx[5][11] = {	{ 2, 3, 13, 12, 5, 6, 1, 8, 9, 10, 4 }, 
													{ 9, 10, 13, 6, 1, 7, 2, 4, 3, 11, 8 }, 
													{ 1, 2, 3, 4, 5, 6, 12, 8, 9, 10, 7 },
													{3, 10, 4, 12, 2, 9, 5, 7, 6, 8, 1 },
													{10, 11, 1, 2, 3, 4, 5, 6, 7, 8, 9 } };
					int		iOffset = o->Type - MODEL_SWORDLEFT01_EMPIREGUARDIAN_BOSS_GAION_;
					int		*arriBoneIdxCur = arriBoneIdx[iOffset];
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
					
					// 1. BonePosition Particle 처리
					arrEachBonePos = new vec3_t[b->NumBones];


					// - APPEAR WITH FIRE EFFECT
					vec3_t	vRelativePos, vAngle;
					vec3_t	v3CurrentHighHierarchyNodePos;		// 상위 모델의 ATTACH 본 위치값.
					
					Vector(4.f, 0.f, 0.0f, vRelativePos);
					VectorCopy(o->Angle,vAngle);

					BMD *pBMDSwordModel = &Models[o->Type]; 					
					BMD *pOwnerModel = &Models[o->Owner->Type];

					int arrBoneIdxs[] = { 4, 2, 8, 10, 6 };		// 각 모델별 참조해야할 가이온의 본 INDEX. 
					int iBoneIdx = arrBoneIdxs[o->Type-MODEL_SWORDLEFT01_EMPIREGUARDIAN_BOSS_GAION_];
					
					pBMDSwordModel->AnimationTransformWithAttachHighModel( o->Owner, pOwnerModel, iBoneIdx, 
											v3CurrentHighHierarchyNodePos, arrEachBonePos );
					
					// - 등장 END EFFECT 정의
					vec3_t	v3LightTerrain;
					Vector(0.9f, 0.4f, 0.1f, v3LightTerrain);
					
					AddTerrainLight(o->Position[0],o->Position[1],v3LightTerrain,2,PrimaryTerrainLight);

// 					// CASE 1		: Yellow계열. 메인스워드 형식 이펙트 
// 					{
// 						Vector(1.0f, 1.0f, 1.0f, v3LightModify);
// 						for( int j = 0; j < 11; ++j )
// 						{
// 							if( rand()%3 == 0 )
// 							{
// 								CreateParticle(BITMAP_POUNDING_BALL,arrEachBonePos[j],o->Angle,v3LightModify, 3 );
// 							}
// 						}
// 					}
			
					// CASE 2		: Blue계열. 
					{
						Vector( 0.3f, 0.4f, 1.0f, v3LightModify );		// 파란색 계열.
						for( int j = 0; j < 11; ++j )
						{
							if( rand()%2 == 0 )
							{
								CreateParticle(BITMAP_FIRE_HIK1_MONO,arrEachBonePos[j],o->Angle,v3LightModify,10,o->Scale);
							}								
						}
					}
			
// 					// CASE 3		: Red-Yellow 계열
// 					{
// 						Vector( 1.0f, 1.0f, 1.0f, v3LightModify );		// 붉은색 계열
// 						for( int j = 0; j < 11; ++j )
// 						{
// 							if( rand()%20 == 0 )
// 							{
// 								CreateEffect(BITMAP_FIRE_CURSEDLICH,arrEachBonePos[j],vAngle,v3LightModify,12,o,-1,0,0,0,o->Scale);
// 							}
// 						}
// 					}
					// 3. 검 등장시에 APPEAR EFFECT들
					
					delete [] arrEachBonePos;
				}	// if( fCurrentRate >= 0.0f && fCurrentRate <= 0.01f )
				// 


				// 2. ALpha 가감쇠 처리 - 보간법
				if( o->m_Interpolates.m_vecInterpolatesAlpha.size() > 0 )
				{
					o->m_Interpolates.GetAlphaCurrent(o->Alpha,fCurrentRate);
				}
			} // else if( o->SubType==11 )
			else if( o->SubType == 20 )		// 특정 검의 R,T,S만으로 해당 위치에서 적당히 그리고 ALPHA 빼기만.
			{
				o->Visible = true;		// MoveEffect에서 CreateEffect시 간혹 Move를 타지 않고 그린는 것을 방지.
				//float	fRateStatic = 0.6f;
				float	fCurrentRate = (float)(o->LifeTime) / (float)(o->ExtState);
				vec3_t	v3RotateAngleRelative;
				// PASSED METHOD
// 				VectorInterpolation_F( o->Position, o->Owner->Position, o->StartPosition, 1.0f, fRateStatic );
// 				VectorInterpolation_F( o->Angle, o->Owner->Angle, o->m_v3PrePos1, 1.0f, fRateStatic );
// 				LInterpolationF( o->Scale, o->Owner->Scale, o->AlphaTarget, fRateStatic );
// 				LInterpolationF( o->Alpha, 0.0f, o->Distance, fRateCurrent );

				// NEW METHOD
				// 3. 보간 반영.
				// ========================================================================================>
				// 4. (1) 보간 반영. - Angle
				if( o->m_Interpolates.m_vecInterpolatesAngle.size() > 0 )
				{
					o->m_Interpolates.GetAngleCurrent(v3RotateAngleRelative ,fCurrentRate);		
					
					o->Angle[0] = v3RotateAngleRelative[0];
					o->Angle[1] = v3RotateAngleRelative[1];
					o->Angle[2] = v3RotateAngleRelative[2];
				}
				
				// 6. (2) 보간 반영. - Position
				if( o->m_Interpolates.m_vecInterpolatesPos.size() > 0 )
				{
					o->m_Interpolates.GetPosCurrent(o->Position,fCurrentRate);
				}
				
				// 8. (3) 보간 반영. - Scale
				if( o->m_Interpolates.m_vecInterpolatesScale.size() > 0 )
				{
					o->m_Interpolates.GetScaleCurrent(o->Scale,fCurrentRate);
				}
				
				// 9. (4) 보간 반영. - Alpha
				if( o->m_Interpolates.m_vecInterpolatesAlpha.size() > 0 )
				{
					o->m_Interpolates.GetAlphaCurrent(o->Alpha,fCurrentRate);
				}
				// 3. 보간 반영.
				// ========================================================================================<
			}
		}

		break;
#endif // LDS_ADD_EG_4_MONSTER_WORLDBOSS_GAIONKALEIN
		
#ifdef LDS_ADD_EG_4_MONSTER_JELINT
		case MODEL_EMPIREGUARDIAN_BLOW_OF_DESTRUCTION:
			{
				if(o->SubType == 0)
				{
					vec3_t vLight;
					if(o->LifeTime <= 24)
					{
						if(o->LifeTime >= 15.f)
						{
							EarthQuake = (float)(rand()%8-4)*0.1f;
						}
						else
						{
							EarthQuake = 0.f;
						}
						
						if(o->LifeTime == 23)
						{
							Vector(0.3f, 0.3f, 1.0f, vLight);
							// 날카로운 빛 모양 이펙트 발생
							//CreateEffect(MODEL_NIGHTWATER_01, o->Position, o->Angle, vLight, 0, o);
							//CreateEffect(MODEL_NIGHTWATER_01, o->Position, o->Angle, vLight, 0, o);

							CreateEffect(MODEL_NIGHTWATER_01, o->Position, o->Angle, vLight, 0, o, o->PKKey, o->Kind, o->Skill, o->m_bySkillSerialNum, o->Scale * 1.0f);
							CreateEffect(MODEL_NIGHTWATER_01, o->Position, o->Angle, vLight, 0, o, o->PKKey, o->Kind, o->Skill, o->m_bySkillSerialNum, o->Scale * 1.0f);

							CreateEffect(MODEL_KNIGHT_PLANCRACK_A, o->Position, o->Angle, vLight, 0, o, 0, 0, 0, 0, o->Scale*0.2f);

							vec3_t vDir, vPos, vAngle;
							VectorSubtract(o->StartPosition, o->Position, vDir);
							float fLength = VectorLength(vDir);
							VectorNormalize(vDir);
							int iNum = (int)(fLength / 100) + 1;
							for(int i=0; i<iNum; ++i)
							{
								VectorScale(vDir, 55.f * i, vPos);
								VectorAdd(o->Position, vPos, vPos);
								VectorCopy(o->Owner->Angle, vAngle);
								if(i % 2 == 0)
								{
									vAngle[2] += (rand()%20 + 10);
								}
								else
								{
									vAngle[2] -= (rand()%20 + 10);
								}
								CreateEffect(MODEL_KNIGHT_PLANCRACK_B, 
									vPos, vAngle, vLight, 0, o, 0, 0, 0, 0, o->Scale);
							}
						}
						
						o->Light[0] /= 1.05f;
						o->Light[1] /= 1.05f;
						o->Light[2] /= 1.05f;
					}
				}
				else if(o->SubType == 1)
				{
					if(o->LifeTime <= 24)
					{
						if(o->LifeTime >= 15)
						{
							vec3_t vPos, vLight;
							for(int i=0; i<15; ++i)
							{
								VectorCopy(o->Position, vPos);
								vPos[0] += rand()%400 - 200;
								vPos[1] += rand()%400 - 200;
								vPos[2] += rand()%400 - 200;
								float fScale = (o->Scale * 1.6f) + rand()%10 * 0.1f;
								Vector(0.3f, 0.3f, 1.0f, vLight);
								int index = (rand()%2 == 0) ? BITMAP_WATERFALL_5 : BITMAP_WATERFALL_3;
								CreateParticle(index, vPos, o->Angle, vLight, 8, fScale);
								Vector(1.0f, 1.0f, 1.0f, vLight);
								if(rand()%2 == 0)
								{
									CreateParticle(BITMAP_SMOKE, vPos, o->Angle, vLight, 55, o->Scale * 0.9f );
								}
							}
						}

						if(o->LifeTime == 23)
						{
							vec3_t vLight;
							Vector(0.5f, 0.5f, 1.f, vLight);

							Vector(0.3f, 0.3f, 1.0f, vLight);
							// 날카로운 빛 모양 이펙트 발생
							CreateEffect(MODEL_NIGHTWATER_01, o->Position, o->Angle, vLight, 0, o, -1, 0, 0, 0, o->Scale * 1.1f);
							CreateEffect(MODEL_NIGHTWATER_01, o->Position, o->Angle, vLight, 0, o, -1, 0, 0, 0, o->Scale * 1.2f);

							// 바닥에 큰 가라짐 이펙트 발생
							CreateEffect(MODEL_RAKLION_BOSS_CRACKEFFECT, o->Position, o->Angle, vLight, 
								0, o, -1, 0, 0, 0, o->Scale * 0.4f);

							vec3_t vPos,vResult;
							vec3_t vAngle;
							float Matrix[3][4];
							for(int i=0; i<(5 + rand()%3); ++i)
							{
								Vector(0.f,(float)(rand()%150),0.f,vPos);
								Vector(0.f,0.f,(float)(rand()%360),vAngle);
								AngleMatrix(vAngle,Matrix);
								VectorRotate(vPos,Matrix,vResult);
								VectorAdd(vResult,o->Position,vResult);
								
      	     					CreateEffect(MODEL_STONE1+rand()%2, vResult, o->Angle, o->Light, 13);
							}
						}

						o->Light[0] /= 1.05f;
						o->Light[1] /= 1.05f;
						o->Light[2] /= 1.05f;
					}
				}
			}
			break;		
#endif // LDS_ADD_EG_4_MONSTER_JELINT

#ifdef PBG_ADD_CHARACTERSLOT
			case MODEL_SLOT_LOCK:
				{
					o->LifeTime = 100;
				}
				break;
#endif //PBG_ADD_CHARACTERSLOT
#ifdef KJH_ADD_09SUMMER_EVENT
			case BITMAP_RING_OF_GRADATION:
				{
					if( o->SubType == 0 )
					{
						if( o->LifeTime == 20 )
							break;

						o->Scale += 0.1f;
						
						o->Light[0] /= 1.1f;
						o->Light[1] /= 1.1f;
						o->Light[2] /= 1.1f;

						o->Alpha /= 1.1f;
					}
				}break;
			case MODEL_EFFECT_UMBRELLA_DIE:
				{
					if( o->SubType == 0)
					{
						if( o->LifeTime == 28 || o->LifeTime == 18 || o->LifeTime == 8)
						{			
							vec3_t vLight;
							Vector(1.0f, 0.2f, 0.5f, vLight);
							CreateEffect(BITMAP_RING_OF_GRADATION, o->Owner->Position, o->Angle, vLight, 0, o->Owner, 0, 0, 0, 0, 1.2f);
						}
					}
				}break;
			case MODEL_EFFECT_UMBRELLA_GOLD:
				{
					o->Angle[0] += 10.f;
					o->Position[0] += (o->Direction[0]*2.2f);
					o->Position[1] += (o->Direction[1]*2.2f);
					o->Position[2] += (o->Gravity * 1.5f);
					o->Gravity -= 1.5f;
					
					Height = RequestTerrainHeight(o->Position[0],o->Position[1]);
					if(o->Position[2] < Height)
					{
						o->Position[2] = Height;
						o->Gravity = -o->Gravity*0.3f;
						o->LifeTime -= 2;
					}
				}break;
#endif // KJH_ADD_09SUMMER_EVENT
#ifdef KJH_ADD_EG_MONSTER_GUARDIANDEFENDER_EFFECT
			case MODEL_EFFECT_EG_GUARDIANDEFENDER_ATTACK2:		// 제국수호군 방패병 공격2 이팩트
				{
					if( o->LifeTime <= 18 )
					{
						o->Light[0] /= 1.4f;
						o->Light[1] /= 1.4f;
						o->Light[2] /= 1.4f;
						o->Scale *= 1.1f;
					}
					CreateSprite(BITMAP_SHOCK_WAVE, o->Position, o->Scale, o->Light, o->Owner);
					CreateParticle ( BITMAP_FIRE+2, o->Position, o->Angle, o->Light, 16, 2.5f );
					
				}break;
#endif // KJH_ADD_EG_MONSTER_GUARDIANDEFENDER_EFFECT
#ifdef PBG_ADD_AURA_EFFECT
			case MODEL_EFFECT_SD_AURA:
				{
					vec3_t Temp_Pos;
					BMD* b = &Models[o->Owner->Type];
					b->TransformByObjectBone(Temp_Pos, o->Owner, 18);
					Temp_Pos[2] -= 30.0f;
					VectorCopy(Temp_Pos, o->Position);
					o->LifeTime = 100;
				}
				break;
#endif //PBG_ADD_AURA_EFFECT
#ifdef LDK_ADD_EXTENSIONMAP_BOSS_MEDUSA_WAND_EFFECT
			case BITMAP_WATERFALL_4:
				{
 					vec3_t Temp_Pos;
					BMD* b = &Models[o->Owner->Type];
					b->TransformByObjectBone(Temp_Pos, o->Owner, o->PKKey);

					o->Timer += 0.1f;
					o->Distance += 2.1f;
					o->Angle[0] += 0.7f;
					
					o->Light[0] *= 0.95f;
					o->Light[1] *= 0.95f;
					o->Light[2] *= 0.95f;
					//o->Alpha *= 0.92f;
					
					vec3_t vPos;
					for(int i=0; i<3; i++)
					{
						switch(i)
						{
						case 0:
							vPos[0] = cosf(o->Timer) * o->Distance + Temp_Pos[0];
							vPos[1] = Temp_Pos[1];
							vPos[2] = sinf(o->Timer) * o->Distance + Temp_Pos[2]; 
							break;
						case 1:
							vPos[0] = sinf(o->Timer) * o->Distance + Temp_Pos[0];
							vPos[1] = cosf(o->Timer) * o->Distance + Temp_Pos[1]; 
							vPos[2] = Temp_Pos[2];
							break;
						case 2:
							vPos[0] = Temp_Pos[0];
							vPos[1] = sinf(o->Timer) * o->Distance + Temp_Pos[1];
							vPos[2] = cosf(o->Timer) * o->Distance + Temp_Pos[2]; 
							break;
						}
						
						CreateSprite(BITMAP_WATERFALL_4, vPos, o->Scale, o->Light, o, o->Angle[0]);
					}
				}
				break;
#endif //LDK_ADD_EXTENSIONMAP_BOSS_MEDUSA_WAND_EFFECT
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
			case MODEL_WOLF_HEAD_EFFECT:
				{
					if(o->SubType == 0)
					{
						o->BlendMeshLight = (float)o->LifeTime/20.0f;
						o->Alpha = o->BlendMeshLight;
						vec3_t Angle;
						if(o->LifeTime >= 3)
						{
							VectorCopy(o->Owner->Angle,Angle);
							int nRandom = rand()%6;
							float _Value = 7.0f+rand()%3;
							if(nRandom/3)
								o->Owner->Angle[2] = Angle[2] + (_Value*(nRandom%3*2+1));
							else
								o->Owner->Angle[2] = Angle[2] - (_Value*(nRandom%3*2+1));
							
							int _Temp = rand()%3;
							if(_Temp == 1)
								o->Owner->Angle[0] = Angle[0] + (_Value*(nRandom%3*2+1));
							else if(_Temp == 2)
								o->Owner->Angle[0] = Angle[0] - (_Value*(nRandom%3*2+1));

							for(int i=0; i<3; i++)
							{
								BMD* b = &Models[o->Owner->Type];
								b->TransformByObjectBone(p, o->Owner, 27);
								VectorCopy(p, o->Position);
								o->Scale = 1.5f;
								Vector(0.2f,0.3f,1.0f,Light);
								p[1] += rand()%20 - 10;
								p[2] += rand()%28 + 6;
								CreateJoint(BITMAP_PIN_LIGHT,p,p,o->Owner->Angle,0,o->Owner,5.0f);
							}
							CreateEffect(MODEL_WOLF_HEAD_EFFECT, o->Owner->Position, o->Owner->Angle, o->Owner->Light, 1, o->Owner);
							CreateEffect(MODEL_WOLF_HEAD_EFFECT, o->Owner->Position, o->Owner->Angle, o->Owner->Light, 2, o->Owner);
							o->Owner->Angle[2] = Angle[2];
							o->Owner->Angle[0] = Angle[0];
						}
					}
					else if(o->SubType == 1)
					{
						o->Owner->Velocity += 2.5f;
					}
					else if(o->SubType == 2)
					{
						o->Owner->Velocity += 5.5f;
					}
				}
				break;
			case BITMAP_SBUMB:
				{
					if(o->LifeTime < 17)
					{
						vec3_t vLight, vPosition;
						Vector(1.0f,1.0f,1.0f,vLight);
						if(o->Owner->m_sTargetIndex < 0)
							break;
						
						VectorCopy(CharactersClient[o->Owner->m_sTargetIndex].Object.Position, vPosition);
						Vector(vPosition[0] + rand()%80-40, vPosition[1] + rand()%80-40, vPosition[2] + rand()%120+30, vPosition);
						float Scale = 1.5f+(rand()%10*0.02f);
						CreateParticle(BITMAP_SBUMB, vPosition, o->Angle, vLight, 0, Scale*o->Scale, o->Owner);
					}
					if(o->LifeTime==15
#ifdef PBG_MOD_RAGEFIGHTERSOUND
						|| o->LifeTime==9 || o->LifeTime==6
#endif //PBG_MOD_RAGEFIGHTERSOUND
						)
					{
#ifdef PBG_MOD_RAGEFIGHTERSOUND
						StopBuffer(SOUND_RAGESKILL_THRUST_ATTACK, true);
#endif //PBG_MOD_RAGEFIGHTERSOUND
						PlayBuffer(SOUND_RAGESKILL_THRUST_ATTACK);
					}
				}
				break;
			case MODEL_DOWN_ATTACK_DUMMY_L:
				{
					if(o->Owner->Live)
					{
						BMD *pModel = &Models[o->Type];
						pModel->Animation(BoneTransform, o->AnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle, false, false);
						
						if(o->AnimationFrame >= 8.0f || !o->Owner->Live)
							o->Live = false;

						if(o->AnimationFrame >= 5.0f && o->LifeTime > 50)
						{
#ifdef PBG_MOD_RAGEFIGHTERSOUND
							if(o->Owner->m_sTargetIndex<0)
								break;
#endif //PBG_MOD_RAGEFIGHTERSOUND
							CreateBomb3(CharactersClient[o->Owner->m_sTargetIndex].Object.Position, 3, 0.9f);
#ifdef PBG_MOD_RAGEFIGHTERSOUND
							StopBuffer(SOUND_RAGESKILL_STAMP_ATTACK, true);
							PlayBuffer(SOUND_RAGESKILL_STAMP_ATTACK);
#endif //PBG_MOD_RAGEFIGHTERSOUND
							o->LifeTime = 40;						
						}

						vec3_t _StartPos, _EndPos;
						pModel->TransformByObjectBone(_StartPos, o, 0);
						CreateJoint(BITMAP_FORCEPILLAR, _StartPos, _StartPos, o->Angle, 0, o, 50.0f);
						
						VectorCopy(o->Owner->Angle, o->Angle);
						VectorCopy(o->Owner->Position, o->Position);

						vec3_t Light;
						Vector(0.47f, 0.36f, 0.24f, Light);
						pModel->TransformByObjectBone(_EndPos, o, 0);
						CreateSprite(BITMAP_FLARE, _EndPos, 5.0f, Light, o);
					}
					else
					{
						o->Live = false;
					}
				}
				break;
			case MODEL_DOWN_ATTACK_DUMMY_R:
				{
					if(o->Owner->Live)
					{
						BMD *pModel = &Models[o->Type];
						pModel->Animation(BoneTransform, o->AnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle, false, false);

						if(o->AnimationFrame >= 6.0f || !o->Owner->Live)
						{
							o->Live = false;
							CreateEffect(MODEL_DOWN_ATTACK_DUMMY_L, o->Owner->Position, o->Owner->Angle, o->Owner->Light, 0, o->Owner);
						}

						if(o->AnimationFrame >= 3.0f && o->LifeTime > 50)
						{
#ifdef PBG_MOD_RAGEFIGHTERSOUND
							if(o->Owner->m_sTargetIndex<0)
								break;
#endif //PBG_MOD_RAGEFIGHTERSOUND
							CreateBomb3(CharactersClient[o->Owner->m_sTargetIndex].Object.Position, 3, 0.9f);
#ifdef PBG_MOD_RAGEFIGHTERSOUND
							StopBuffer(SOUND_RAGESKILL_STAMP_ATTACK, true);
#endif //PBG_MOD_RAGEFIGHTERSOUND
							PlayBuffer(SOUND_RAGESKILL_STAMP_ATTACK);
							o->LifeTime = 40;
						}

						vec3_t _StartPos, _EndPos;
						pModel->TransformByObjectBone(_StartPos, o, 0);
						CreateJoint(BITMAP_FORCEPILLAR, _StartPos, _StartPos, o->Angle, 1, o, 50.0f);
						
						VectorCopy(o->Owner->Angle, o->Angle);
						VectorCopy(o->Owner->Position, o->Position);

						vec3_t Light;
						Vector(0.47f, 0.36f, 0.24f, Light);
						pModel->TransformByObjectBone(_EndPos, o, 0);
						CreateSprite(BITMAP_FLARE, _EndPos, 5.0f, Light, o);
					}
					else
					{
						o->Live = false;
					}
				}
				break;
			case BITMAP_SWORDEFF:
				{
					if(o->Owner->AnimationFrame > Models[o->Owner->Type].Actions[PLAYER_SKILL_GIANTSWING].NumAnimationKeys)
						o->Live = false;

					BMD* b = &Models[o->Owner->Type];
					vec3_t _StartPos, _EndPos;
					b->TransformByObjectBone(_StartPos, o->Owner, 5);
					b->TransformByObjectBone(_EndPos, o->Owner, 4);
					CreateJoint(BITMAP_SWORDEFF, _StartPos, _EndPos, o->Owner->Angle, 0, o->Owner, 100.0f);

					if(o->Owner->AnimationFrame > 6 && o->LifeTime >50)
					{
						vec3_t Light, Angle;
						Vector(0.4f,0.5f,1.0f,Light);
						VectorCopy(o->Owner->Angle, Angle);

						vec3_t position, Relative;
 						Vector(-180.0f, 100.0f, 0.0f, Relative);
						b->TransformByObjectBone(position, o->Owner, 0, Relative);
						
						Vector(60.0f,0.0f,Angle[2]-180, Angle);
						CreateEffect(MODEL_SHOCKWAVE01, position, Angle, Light, 0, o->Owner, -1, 0, 0, 0, 1.0f);
						CreateEffect(MODEL_WOLF_HEAD_EFFECT2, o->Position, o->Angle, o->Light, 3, o->Owner);
						
						if(o->Owner->m_sTargetIndex<0)
							break;

						CreateEffect(BITMAP_DAMAGE1, CharactersClient[o->Owner->m_sTargetIndex].Object.Position, Angle, Light, 0, o->Owner, -1, 0, 0, 0, 1.4f);
#ifdef PBG_MOD_RAGEFIGHTERSOUND
						StopBuffer(SOUND_RAGESKILL_GIANTSWING_ATTACK, true);
						PlayBuffer(SOUND_RAGESKILL_GIANTSWING_ATTACK);
#endif //PBG_MOD_RAGEFIGHTERSOUND
						o->LifeTime = 45;
					}

					if(o->Owner->AnimationFrame > 11 && o->LifeTime > 20)
					{
						vec3_t Light, Angle, vPosition;
						Vector(0.5f,0.5f,1.0f,Light);
 						VectorCopy(o->Owner->Angle, Angle);
						vec3_t position, Relative;
						Vector(-80.0f, 120.0f, 0.0f, Relative);
						b->TransformByObjectBone(position, o->Owner, 4, Relative);
						Vector(90.f,0.0f,Angle[2]-180, Angle);
						CreateEffect(MODEL_SHOCKWAVE02, position, Angle, Light, 0, o->Owner, -1, 0, 0, 0, 0.5f);
						CreateEffect(MODEL_WOLF_HEAD_EFFECT2, o->Position, o->Angle, o->Light, 5, o->Owner);

						Vector(1.0f, 1.0f, 1.0f, Light);
						Vector(-80.0f, 20.0f, 0.0f, vPosition);
						b->TransformByObjectBone(vPosition, o->Owner, 4, vPosition);
						Vector(90.0f,Angle[1],Angle[2], Angle);
						CreateEffect(MODEL_SHOCKWAVE_SPIN01, vPosition, Angle, Light, 0, o->Owner, -1, 0, 0, 0, 0.5f);
						CreateEffect(MODEL_SHOCKWAVE_SPIN01, vPosition, Angle, Light, 1, o->Owner, -1, 0, 0, 0, 0.9f);

						if(o->Owner->m_sTargetIndex<0)
							break;

						VectorCopy(CharactersClient[o->Owner->m_sTargetIndex].Object.Position, vPosition);
						CreateEffect(BITMAP_SHINY+4, vPosition, o->Angle, Light, 0, o->Owner, -1, 0, 0, 0, 1.9f);
#ifdef PBG_MOD_RAGEFIGHTERSOUND
						StopBuffer(SOUND_RAGESKILL_GIANTSWING_ATTACK, true);
#endif //PBG_MOD_RAGEFIGHTERSOUND
						PlayBuffer(SOUND_RAGESKILL_GIANTSWING_ATTACK);
						o->LifeTime = 20;
					}
				}
				break;
			case MODEL_SHOCKWAVE01:
				{
					if(o->SubType==1 || o->SubType==2)
					{
						o->Alpha = 1.0f;
						o->Scale *= 1.24f;
						o->Position[2] *= 1.19f;
						VectorScale(o->Light,0.45f,o->Light);
						if(o->Light[0] < 0.001f)
						{
							Vector(0.0f,0.0f,0.0f,o->Light);
						}
					}
					else if(o->SubType==3)
					{
						o->Scale *= 1.15f;
						VectorScale(o->StartPosition, 0.99f, o->StartPosition);
						VectorSubtract(o->Position, o->StartPosition, o->Position);
						VectorScale(o->Light,0.7f,o->Light);
						if(o->Light[0]>0.6f)
						{
							o->SubType = 5;
						}
						o->Position[2] = 180.0f + o->Scale*80.0f;
					}
					else if(o->SubType==4)
					{
						o->Scale *= 1.15f;
						VectorScale(o->StartPosition, 1.04f, o->StartPosition);
						VectorSubtract(o->Position, o->StartPosition, o->Position);
						VectorScale(o->Light,1.5f,o->Light);
						if(o->Light[0]>0.6f)
						{
							o->SubType = 5;
						}
						o->Position[2] = 160.0f + o->Scale*10.0f;
					}
					else if(o->SubType ==5)
					{
						o->Scale *= 1.15f;
						VectorScale(o->StartPosition, 1.04f, o->StartPosition);
						VectorSubtract(o->Position, o->StartPosition, o->Position);
						VectorScale(o->Light,0.5f,o->Light);
						o->Position[2] = 160.0f + o->Scale*10.0f;
					}
					else
					{
						o->Alpha = 1.0f;
						o->Scale += 0.2f;
						VectorScale(o->StartPosition, 0.99f, o->StartPosition);
						VectorSubtract(o->Position, o->StartPosition, o->Position);
						VectorScale(o->Light,0.65f,o->Light);
						o->Position[2] = 220.0f + o->Scale*80.0f;
					}					
				}
				break;
			case MODEL_SHOCKWAVE02:
				{
					o->Alpha = 1.0f;
					VectorScale(o->Light,0.8f,o->Light);
					o->Scale += 0.1f;
					VectorScale(o->StartPosition, 0.95f, o->StartPosition);
					VectorSubtract(o->Position, o->StartPosition, o->Position);
				}
				break;
			case BITMAP_DAMAGE1:
				{
					if(o->LifeTime < 17)
					{
						if(o->LifeTime%4 != 0)
							break;

						vec3_t vLight, vPosition;
						Vector(0.6f,0.94f,1.0f,vLight);

						if(o->Owner->m_sTargetIndex < 0)
							break;
						
						VectorCopy(CharactersClient[o->Owner->m_sTargetIndex].Object.Position, vPosition);
						Vector(vPosition[0] + rand()%90-40, vPosition[1] + rand()%90-40, vPosition[2] + rand()%100+50, vPosition);
						float Scale = 1.5f+(rand()%5*0.01f);
						CreateParticle(BITMAP_DAMAGE1, vPosition, o->Angle, vLight, 0, Scale*o->Scale, o->Owner);
					}
				}
				break;
			case MODEL_SHOCKWAVE_SPIN01:
				{
					o->Scale *= 1.01f;
					o->Angle[1] -= 10.0f;
					if(o->SubType==1)
					{
						o->Angle[1] += 20.0f;
					}
					VectorScale(o->Light, 0.9f, o->Light);
					o->Alpha = 1.0f;
				}
				break;
			case BITMAP_EVENT_CLOUD:
				{
					if(o->SubType==1)
					{
						o->Scale *= 1.08f;
						if(o->Scale > 5)
						{
							o->Scale = 5.0f;
						}
						o->Angle[0] = -(int)WorldTime*0.5f;
						VectorScale(o->Light, 0.9f, o->Light);
					}
					else
					{
						o->Scale *= 1.08f;
						if(o->Scale > 3)
						{
							o->Scale =3.0f;
						}
						o->Angle[0] = -(int)WorldTime*0.5f;
						VectorScale(o->Light, 0.9f, o->Light);
					}					
				}
				break;
			case MODEL_WINDFOCE:
				{
					o->Angle[2] = -(int)WorldTime*0.3f;

					if(o->SubType == 0 || o->SubType == 4 || o->SubType == 5)				// 시작때 한번 커지는거
					{
						o->Scale += 0.1f;
						if(o->Scale > 3)
						{
							o->Scale = 3;
						}
						o->Alpha *= 0.92f;
						VectorScale(o->Light, 0.9f, o->Light);
						if(o->LifeTime == 40 || o->LifeTime == 30)
						{
							vec3_t vLight;
							if(o->SubType == 0)
							{
								Vector(0.5f, 0.55f, 1.0f, vLight);
							}
							else if(o->SubType == 4)
							{
								Vector(0.85f, 0.2f,1.0f,vLight);
							}
							else if(o->SubType == 5)
							{
								Vector(1.0f, 0.2f,0.0f,vLight);
							}
							CreateEffect(MODEL_WINDFOCE, o->Position, o->Angle, vLight, 3, o, -1, 0, 0, 0, 1.0f);
						}
					}
					else if(o->SubType == 1)		//지속적인거 
					{
						if(o->Owner!=NULL && o->Owner->Live==true 
						&& (g_isCharacterBuff(o->Owner, eBuff_Att_up_Ourforces)
						|| g_isCharacterBuff(o->Owner, eBuff_Hp_up_Ourforces)
						|| g_isCharacterBuff(o->Owner, eBuff_Def_up_Ourforces)))
						{
							o->LifeTime = 10;
						}
						else
						{
							o->LifeTime = 0;
						}
						
						if(g_isCharacterBuff(o->Owner, eBuff_Cloaking))
							break;
						
						Vector(0.6f, 0.6f, 1.0f, o->Light);
						float fFadeInOut = (float)(sinf(WorldTime*0.001f)+1.0f);
						VectorScale(o->Light, fFadeInOut*0.25f+0.2f, o->Light);
						o->Scale = 1.0f;
						VectorCopy(o->Owner->Position, o->Position);
					}
					else if(o->SubType==2)
					{
						o->Angle[2] = -(int)WorldTime*0.9f;
						o->Scale += 0.1f;
						if(o->Scale > 3)
						{
							o->Scale = 3;
						}
						o->Alpha *= 0.92f;
						VectorScale(o->Light, 0.85f, o->Light);

						if(o->LifeTime == 60 || o->LifeTime == 50)
						{
							vec3_t vLight;
							Vector(1.0f, 0.5f, 0.0f, vLight);
							CreateEffect(MODEL_WINDFOCE, o->Position, o->Angle, vLight, 3, o, -1, 0, 0, 0, 1.0f);
						}
					}
					else if(o->SubType==3)
					{
						o->Angle[2] = -(int)WorldTime*0.9f;
						o->Scale += 0.1f;
						if(o->Scale > 3)
						{
							o->Scale = 3;
						}
						o->Alpha *= 0.92f;
						VectorScale(o->Light, 0.85f, o->Light);
					}
				}
				break;
			case BITMAP_SHINY+4:
				{
					vec3_t vPosition, Light;
					VectorCopy(o->Position, vPosition);
					VectorCopy(o->Light, Light);
					Vector(vPosition[0] + rand()%100-40, vPosition[1] + rand()%100-40, vPosition[2] + rand()%100+50, vPosition);
					CreateParticle(BITMAP_SHINY+4, vPosition, o->Angle, Light, 2, o->Scale);
				}
				break;
			case BITMAP_LIGHT_RED:
				{
					if(o->Owner!=NULL && o->Owner->Live==true 
						&& (g_isCharacterBuff(o->Owner, eBuff_Hp_up_Ourforces)
						|| g_isCharacterBuff(o->Owner, eBuff_Att_up_Ourforces)
						|| g_isCharacterBuff(o->Owner, eBuff_Def_up_Ourforces)))
					{
						o->LifeTime = 10;

						if(g_isCharacterBuff(o->Owner, eBuff_Def_up_Ourforces))
						{
							if(rand()%70 == 7)
							{
								if(!SearchEffect(MODEL_WINDFOCE_MIRROR, o->Owner))
								{
									vec3_t vLight;
									Vector(0.6f, 0.6f, 1.0f, vLight);
									CreateEffect(MODEL_WINDFOCE_MIRROR, o->Owner->Position, o->Angle, vLight, 0, o->Owner, -1, 0, 0, 0, 1.0f);
								}
							}
						}							
					}
					else if(o->SubType==3 || o->SubType==4)
					{
						vec3_t Light;
						float Luminosity = (float)(rand()%5)*0.1f;
						Vector(Luminosity+0.4f,Luminosity+0.0f,Luminosity+0.0f,Light);
						int range = 2;
						if(o->SubType ==4)
						{
							range = 3;
						}
						AddTerrainLight(o->Position[0],o->Position[1], Light, range, PrimaryTerrainLight);
					}
					else 
					{
						o->LifeTime = 0;
					}

					if(g_isCharacterBuff(o->Owner, eBuff_Cloaking) && (o->SubType!=3))
						break;

					if(g_isCharacterBuff(o->Owner, eBuff_Att_up_Ourforces) || o->SubType==1)
					{
						vec3_t Light, Angle;
						Vector(0.f, 0.f, 0.f, Angle);
						BMD* b = &Models[o->Owner->Type];
						vec3_t Position;

						//연기 피어오르는것
						if(g_isCharacterBuff(o->Owner, eBuff_Att_up_Ourforces))
						{
							if(o->LifeTime%2==0)
							{
								VectorCopy(o->Position, Position);
								Vector(1.0f, 0.12f, 0.0f, Light);
								b->TransformByObjectBone(Position, o->Owner, (rand()%2==0) ? 26 : 35);
								CreateParticle(BITMAP_SMOKELINE1, Position, o->Angle, Light, 5, 0.005f, o->Owner);
							}
						}
					}
				}
				break;
			case MODEL_WINDFOCE_MIRROR:
				{
					if(o->Owner==NULL || o->Owner->Live==false
						|| !g_isCharacterBuff(o->Owner, eBuff_Def_up_Ourforces))
					{
						o->Live = false;
						break;
					}
					o->Angle[2] = (int)WorldTime*0.4f;
					o->Scale *= 1.015f;
					
					if(o->Scale > 1.5f)
					{
						o->Scale = 1.5f;
					}
					VectorScale(o->Light, 0.9f, o->Light);
					VectorCopy(o->Owner->Position, o->Position);
				}
				break;
			case BITMAP_SWORD_EFFECT_MONO:
				{
					vec3_t vLight;
					VectorCopy(o->Light, vLight);
					VectorScale(vLight, 0.1f, vLight);
					if(o->LifeTime%3!=0)
						CreateParticle(BITMAP_SWORD_EFFECT_MONO, o->Position, o->Angle, o->Light, 0, o->Scale, o);

					if(o->LifeTime == 14)
					{
						Vector(0.4f, 0.4f, 1.0f, vLight);
 						if(o->SubType==0)
 						{
 							Vector(0.4f, 0.4f, 1.0f, vLight);
 						}
						else if(o->SubType==1)
						{
							Vector(0.7f, 0.5f, 1.0f, vLight);
						}
						else if(o->SubType==2)
						{
							Vector(1.0f, 0.12f, 0.0f, vLight);
						}
						CreateEffect(MODEL_SHOCKWAVE01, o->Position, o->Angle, vLight, 2, o->Owner, -1, 0, 0, 0, 1.0f);
					}
				}
				break;
			case MODEL_WOLF_HEAD_EFFECT2:
				{
					if(o->SubType == 4 || o->SubType == 6)
					{
						o->Owner->Velocity += 2.5f;
					}
					else if(o->SubType == 3)
					{
						o->BlendMeshLight = (float)o->LifeTime/20.0f;
						o->Alpha = o->BlendMeshLight;

						VectorCopy(o->Owner->Angle, o->Angle);

						vec3_t Angle;
						if(o->LifeTime >= 3)
						{
							VectorCopy(o->Owner->Angle,Angle);
							int nRandom = rand()%6;
							float _Value = 2.0f+rand()%3;
							if(nRandom/3)
								o->Owner->Angle[2] = Angle[2] + (_Value*(nRandom%3*2+1));
							else
								o->Owner->Angle[2] = Angle[2] - (_Value*(nRandom%3*2+1));
							
							int _Temp = rand()%3;
							if(_Temp == 1)
								o->Owner->Angle[0] = Angle[0] + (_Value*(nRandom%3*2+1));
							else if(_Temp == 2)
								o->Owner->Angle[0] = Angle[0] - (_Value*(nRandom%3*2+1));

							vec3_t pos;
							BMD* b = &Models[o->Owner->Type];
							Vector(0.0f, 0.0f, 0.0f, pos);
							if(o->SubType == 5)
								b->TransformByObjectBone(pos, o->Owner, 27, pos);
							else
								b->TransformByObjectBone(pos, o->Owner, 4, pos);

							VectorCopy(pos, o->Position);
						
							CreateEffect(MODEL_WOLF_HEAD_EFFECT2, o->Owner->Position, o->Owner->Angle, o->Owner->Light, 4, o->Owner);
							CreateEffect(MODEL_WOLF_HEAD_EFFECT2, o->Owner->Position, o->Owner->Angle, o->Owner->Light, 6, o->Owner);
							o->Owner->Angle[2] = Angle[2];
							o->Owner->Angle[0] = Angle[0];
						}
					}
					else if(o->SubType == 4)
					{
						VectorCopy(o->Owner->Angle, o->Angle);
					}
					else if(o->SubType == 5)
					{
						vec3_t pos;
						BMD* b = &Models[o->Owner->Type];
						Vector(0.0f, 0.0f, 0.0f, pos);
						b->TransformByObjectBone(pos, o->Owner, 27, pos);
						VectorCopy(pos, o->Position);
						VectorCopy(o->Owner->Angle, o->Angle);
					}
				}
				break;
			case MODEL_SHOCKWAVE_GROUND01:
				{
					if(o->SubType==1)
					{
						o->Scale *= 1.2f;
						VectorScale(o->Light, 0.9f, o->Light);
					}
					else if(o->SubType==2)
					{
						o->Scale *= 1.2f;
						VectorScale(o->Light, 0.7f, o->Light);
					}
					else
					{
						o->Scale *= 1.2f;
						VectorScale(o->Light, 0.8f, o->Light);
					}
				}
				break;
			case MODEL_DRAGON_KICK_DUMMY:
				{
					if(o->Owner->AnimationFrame > Models[o->Owner->Type].Actions[PLAYER_SKILL_DRAGONKICK].NumAnimationKeys
						|| o->Owner->CurrentAction != PLAYER_SKILL_DRAGONKICK)
					{
						o->LifeTime = 0;
						o->Live = false;
						break;
					}

					if(o->Owner->Live)
					{
						vec3_t _StartPos, _EndPos;
						BMD *pModel = &Models[o->Type];
						
						if(o->Owner->AnimationFrame > 4 && o->Owner->AnimationFrame < 5)
						{
							float fDelay =o->Velocity*10.0f;
							float fActionSpeed = pModel->Actions[o->CurrentAction].PlaySpeed;
							float fSpeedPerFrame = fActionSpeed/fDelay;
							float fAnimationFrame = o->Owner->AnimationFrame - fActionSpeed;
							o->AlphaTarget = fSpeedPerFrame*(o->Velocity/pModel->Actions[o->CurrentAction].PlaySpeed);

							pModel = &Models[o->Owner->Type];
							for(int i=0; i<fDelay; i++)
							{
								pModel->Animation(BoneTransform, fAnimationFrame, o->Owner->PriorAnimationFrame, o->Owner->PriorAction, o->Owner->Angle, o->Owner->HeadAngle, false, false);

								pModel->TransformByObjectBone(_StartPos, o, 1);
								pModel->TransformByObjectBone(_EndPos, o, 2);

								CreateJoint(BITMAP_GROUND_WIND, _StartPos, _EndPos, o->Angle, i, o, 150.0f);
								fAnimationFrame += fSpeedPerFrame;
 							}
						}

						vec3_t vPosition;
#ifdef PBG_MOD_RAGEFIGHTERSOUND
						if(o->Owner->m_sTargetIndex<0)
							break;
#endif //PBG_MOD_RAGEFIGHTERSOUND
						VectorCopy(CharactersClient[o->Owner->m_sTargetIndex].Object.Position, vPosition);
						if(o->Owner->AnimationFrame > 5)
						{
							for(int j=0; j<2; ++j)
							{
								vec3_t vPos;
								float fLuminosity = (float)sinf((WorldTime)*0.002f)*0.2f;
								VectorCopy(vPosition, vPos);
								vPos[2] += 30.0f;
								float fScale = ((float)(rand()%40+50)*0.01f*1.5f)*o->Scale*1.8f;
								for(int i=0; i<4; ++i)
								{
									CreateParticle(BITMAP_ENERGY, vPos, o->Angle, o->Light, 7, fScale);
									CreateParticle(BITMAP_LIGHTNING_MEGA1, vPos, o->Angle, o->Light, 0, fScale*0.9f);

									CreateParticle(BITMAP_SPARK+1, vPos, o->Angle, o->Light, 3, 1.0f+(fLuminosity*0.05f));
									CreateParticle(BITMAP_SPARK+1, vPos, o->Angle, o->Light, 8, 1.0f+(fLuminosity*0.05f));

									if(i%2)
									{
										CreateParticle(BITMAP_SPARK,vPos,o->Angle,o->Light,13);
									}

									if(i%2)
									{
										Vector(vPos[0]+rand()%40+20.0f, vPos[1]+rand()%30+10.0f, vPos[2]+rand()%30+25.0f, vPos);
									}
									else
									{
										Vector(vPos[0]-rand()%20-10.0f, vPos[1]+rand()%30-10.0f, vPos[2]+rand()%30+30.0f, vPos);
									}
								}
							}
						}
						VectorCopy(o->Owner->Angle, o->Angle);
						VectorCopy(o->Owner->Position, o->Position);

						if(o->Owner->AnimationFrame > 8)
						{
							o->LifeTime = 150;
						}
						if(o->LifeTime == 150)
						{
							PlayBuffer(SOUND_RAGESKILL_DRAGONKICK_ATTACK);
						}
					}
					else
					{
						o->LifeTime = 0;
						o->Live = false;
					}
				}
				break;
			case BITMAP_LAVA:
				{
					if(o->Owner->AnimationFrame > Models[o->Owner->Type].Actions[PLAYER_SKILL_DRAGONLORE].NumAnimationKeys
						|| o->Owner->CurrentAction != PLAYER_SKILL_DRAGONLORE)
					{
						o->Live = false;
						o->LifeTime = 0;
						break;
					}
					if(o->Owner->AnimationFrame > 2 && o->Owner->AnimationFrame < 8)
					{
						BMD* pModel = &Models[o->Owner->Type];
						vec3_t _StartPos, _EndPos;

						float fDelay =o->Velocity*10.0f;
						float fActionSpeed = pModel->Actions[o->Owner->CurrentAction].PlaySpeed;
						float fSpeedPerFrame = fActionSpeed/fDelay;
						float fAnimationFrame = o->Owner->AnimationFrame - fActionSpeed;
						
						for(int i=0; i<fDelay; i++)
						{
							pModel->Animation(BoneTransform, fAnimationFrame, o->Owner->PriorAnimationFrame, o->Owner->PriorAction, o->Owner->Angle, o->Owner->HeadAngle, false, false);

							pModel->TransformByObjectBone(_StartPos, o->Owner, 36);
							pModel->TransformByObjectBone(_EndPos, o->Owner, 28);

							CreateJoint(BITMAP_LAVA, _StartPos, _StartPos, o->Owner->Angle, i, o->Owner, 20.0f);
							CreateJoint(BITMAP_LAVA, _EndPos, _EndPos, o->Owner->Angle, 7+i, o->Owner, 20.0f);
							fAnimationFrame += fSpeedPerFrame;
 						}
					}
					vec3_t vLight, vAngle, vPos;
					Vector(1.0f, 1.0f, 1.0f, vLight);
					VectorCopy(o->Owner->Angle, vAngle);

					if(o->Owner->AnimationFrame > 5 && o->LifeTime>100)
					{
						Vector(1.0f, 0.12f, 0.0f, vLight);
						CreateEffect(MODEL_SHOCKWAVE01, o->Owner->Position, vAngle, vLight, 1, o->Owner, -1, 0, 0, 0, 1.0f);
						o->LifeTime = 95;
					}

					if(o->Owner->AnimationFrame > 5 && o->LifeTime>80)
					{
						Vector(1.0f, 0.1f, 0.0f,vLight);
						CreateEffect(MODEL_SHOCKWAVE_GROUND01, o->Position, o->Angle, vLight, 1, o, -1, 0, 0, 0, 1.0f);
						o->LifeTime = 80;
					}

					if(o->Owner->AnimationFrame > 5)
					{
						Vector(1.0f, 1.0f, 1.0f, vLight);
						float fScale = o->Scale * (rand()%5+18)*0.1f;
						VectorCopy(o->Position, vPos);
						vPos[2] += (80 - o->LifeTime)*1.0f*10.0f - (80 - o->LifeTime)*3.0f;
					
						switch(rand()%3)
						{
						case 0:
							CreateParticle(BITMAP_FIRE_HIK1,vPos,o->Angle,vLight,0,fScale);
							break;
						case 1:
							CreateParticle(BITMAP_FIRE_CURSEDLICH,vPos,o->Angle,vLight,4,fScale);
							break;
						case 2:
							CreateParticle(BITMAP_FIRE_HIK3,vPos,o->Angle,vLight,0,fScale);
							break;
						}
						
						vec3_t PosL, Angle, outPos, tempPos;
						VectorCopy(vPos, tempPos);
						VectorScale(vLight, 0.5f, vLight);
						for(int i=0; i<8; ++i)
						{
							if(i%2==0)
								continue;

							float Matrix[3][4];
							Vector(rand()%100-50, rand()%100-50, 0.0f, PosL);
							Vector((float)(rand()%90),0.0f,45.0f*i,Angle);
							AngleMatrix(Angle,Matrix);
							VectorRotate(PosL,Matrix,outPos);
							VectorAdd(vPos,outPos,vPos);
							vPos[2] = tempPos[2];
							CreateParticle(BITMAP_FIRE_HIK1,vPos,o->Angle,vLight,5,fScale);

							vec3_t TempPos;
							VectorCopy(vPos, TempPos);
							TempPos[2] =250.0f;
							CreateParticle(BITMAP_SPARK, TempPos, o->Angle, o->Light,12, 1.5f);
						}
					}
				}
				break;
			case MODEL_DRAGON_LOWER_DUMMY:
				{
					BMD *pModel = &Models[o->Type];
					pModel->CurrentAction = o->CurrentAction;
					pModel->PlayAnimation(&o->AnimationFrame, &o->PriorAnimationFrame, &o->PriorAction, o->Velocity, o->Position, o->Angle);
					
					if(o->AnimationFrame > 1 && o->AnimationFrame < 6)
					{
						vec3_t temp;
						VectorCopy(o->Position, temp);
						temp[2] =250.0f;
						CreateParticle(BITMAP_SPARK, temp, o->Angle, o->Light,12, 1.5f);
					}

					if(o->AnimationFrame > 1 && o->LifeTime > 80)
					{
						vec3_t vLight;
						Vector(1.0f, 1.0f, 1.0f, vLight);
						CreateEffect(BITMAP_LIGHT_RED, o->Position, o->Angle, vLight, 1, o,-1,0,0,0, 15.0f);
						o->LifeTime = 80;
					}
					o->Alpha *= 0.96f;
				}
				break;
			case MODEL_TARGETMON_EFFECT:
				{
					if(o->Owner == NULL || o->Owner->Live==false)
					{
						o->Live =0;
						o->LifeTime = 0;
						break;
					}

					if(o->SubType==1 && o->Owner->m_sTargetIndex < 0)
						break;

					BMD *pModel = &Models[o->Owner->Type];
					vec3_t MonRanPos;
					VectorCopy(o->Owner->Position, MonRanPos);
					int BoneIndex = rand()%(pModel->NumBones-1);
					pModel->TransformByObjectBone(MonRanPos, o->Owner, BoneIndex);

					vec3_t vLight;
					Vector(1.0f, 1.0f, 1.0f, vLight);
					float fScale = o->Scale * (rand()%5+5)*0.1f;
					VectorCopy(MonRanPos, o->Position);
					for(int i=0; i<3; ++i)
					{
						switch(rand()%3)
						{
						case 0:
							CreateParticle(BITMAP_FIRE_HIK1,o->Position,o->Angle,vLight,0,fScale);
							break;
						case 1:
							CreateParticle(BITMAP_FIRE_CURSEDLICH,o->Position,o->Angle,vLight,4,fScale);
							break;
						case 2:
							CreateParticle(BITMAP_FIRE_HIK3,o->Position,o->Angle,vLight,0,fScale);
							break;
						}
					}
				}
				break;
			case MODEL_VOLCANO_OF_MONK:
				{
					if(o->SubType==1)
					{
						vec3_t vLight, vPos;
						Vector(1.0f, 1.0f, 1.0f, vLight);
						VectorCopy(o->Position, vPos);					
						vPos[2] += (80 - o->LifeTime)*(0.9f+rand()%5*0.1f);
						vPos[2] += o->SubType*20.0f;
						if(vPos[2] >400.0f)
						{
							break;
						}
						float fScale = o->Scale * (rand()%5+8)*0.19f;
						switch(rand()%3)
						{
						case 0:
							CreateParticle(BITMAP_FIRE_HIK1,vPos,o->Angle,vLight,6,fScale);
							break;
						case 1:
							CreateParticle(BITMAP_FIRE_CURSEDLICH,vPos,o->Angle,vLight,9,fScale);
							break;
						case 2:
							CreateParticle(BITMAP_FIRE_HIK3,vPos,o->Angle,vLight,6,fScale);
							break;
						}

						if(o->LifeTime == 10)
						{	
							for(int i=0; i<4; ++i)
							{
								CreateEffect(MODEL_VOLCANO_STONE, o->Position, o->Angle, vLight, 0, o, -1, 0, 0, 0, 1.0f);
							}

							VectorCopy(o->Position, vPos);
							Vector(1.0f, 0.1f, 0.0f,vLight);
							CreateEffect(MODEL_SHOCKWAVE_GROUND01, vPos, o->Angle, vLight, 2, o, -1, 0, 0, 0, 1.0f);
							Vector(1.0f, 1.0f, 1.0f, vLight);
						}
						if(o->LifeTime < 20)
						{
							o->Alpha = o->LifeTime*0.1f;
						}
						if(o->LifeTime < 30)
						{
							o->BlendMeshLight = o->LifeTime*0.03f;
						}
						
						vec3_t Light;
						float Luminosity = (float)(rand()%5)*0.01f;
						Vector(Luminosity+1.0f,Luminosity+0.4f,Luminosity+0.4f,Light);
						AddTerrainLight(o->Position[0],o->Position[1], Light, 1, PrimaryTerrainLight);
					}
					else if(o->SubType==2 || o->SubType==3)
					{
						if(o->Owner == NULL || o->Owner->Live == false)
						{
							o->Live =0;
							o->LifeTime=0;
							break;
						}

						if(o->LifeTime == 2)
						{
							CreateEffect(MODEL_VOLCANO_OF_MONK, o->Position, o->Angle, o->Light, 1, o, -1, 0, 0, 0, 1.0f);
							o->LifeTime=0;
							o->Live = false;
						}
						o->Position[2] = -100.0f;
					}
				}
				break;
			case MODEL_VOLCANO_STONE:
				{
					float Height;
					o->HeadAngle[2] -= o->Gravity;

					o->Position[0] = o->Position[0]+o->HeadAngle[0];
					o->Position[1] = o->Position[1]+o->HeadAngle[1];
					o->Position[2] = o->Position[2]+o->HeadAngle[2];

					Height = RequestTerrainHeight(o->Position[0],o->Position[1]);
					o->Angle[0] += 0.3f * o->LifeTime;
					o->Angle[1] += 0.3f * o->LifeTime;

					if (o->Position[2] + o->Direction[2] <= Height)
					{
						o->Position[2] = Height;
						o->HeadAngle[0] *= 0.6f;
						o->HeadAngle[1] *= 0.6f;
						o->HeadAngle[2] += 1.0f*o->LifeTime;
						if (o->HeadAngle[2] < 0.5f)
							o->HeadAngle[2] = 0;

						o->Alpha -= 0.05f;
					}
					o->Scale -= 0.03f;

					vec3_t vLight;
					Vector(1.0f, 1.0f, 1.0f, vLight);
					float fScale = o->Scale * (rand()%5+5)*0.1f;
					switch(rand()%3)
					{
					case 0:
						CreateParticle(BITMAP_FIRE_HIK1,o->Position,o->Angle,vLight,0,fScale);
						break;
					case 1:
						CreateParticle(BITMAP_FIRE_CURSEDLICH,o->Position,o->Angle,vLight,4,fScale);
						break;
					case 2:
						CreateParticle(BITMAP_FIRE_HIK3,o->Position,o->Angle,vLight,0,fScale);
						break;
					}
				}
				break;
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
	}	

#ifdef USE_SELFCHECKCODE
	SendCrcOfFunction( 3, 15, MoveCharacter, 0x21DE);
#endif

	if(	   
		o->Type == MODEL_SKILL_WHEEL1 
		|| o->Type == MODEL_SKILL_WHEEL2
		|| o->Type == MODEL_SKILL_FURY_STRIKE
		|| ((o->Type == MODEL_STONE1 || o->Type == MODEL_STONE2) && o->SubType == 5 ) 
		|| (o->Type == MODEL_ARROW_DRILL && o->SubType == 3 )
        || o->Type == MODEL_PIER_PART
	   	|| o->Type == MODEL_DEATH_SPI_SKILL
		|| o->Type == MODEL_CHANGE_UP_EFF
	  )
	{
	}
	else 
	{
		if(o->Type>=MODEL_BIRD01 && o->Type<MODEL_SKILL_END)
		{
			BMD *b = &Models[o->Type];
			b->CurrentAction = o->CurrentAction;
			if ( o->CurrentAction!=255 && b->PlayAnimation(&o->AnimationFrame,&o->PriorAnimationFrame,&o->PriorAction,o->Velocity,o->Position,o->Angle)==false )
			{
				o->AnimationFrame = b->Actions[b->CurrentAction].NumAnimationKeys;
				o->CurrentAction = 255;
			}
		}
		if(o->Type<BITMAP_BOSS_LASER || o->Type>BITMAP_BOSS_LASER+2)
		{
			switch ( o->Type)
			{
			case MODEL_SAW:
			case MODEL_SPEARSKILL:
				MoveParticle(o,false);
				break;
            case MODEL_ARROW_HOLY:
                {
		            float Matrix[3][4];
		            vec3_t Angle;
		            vec3_t Position;
//					if(o->LifeTime > 12 && o->SubType == 1)
//						break;
		            VectorCopy(o->Angle,Angle);
		            AngleMatrix(Angle,Matrix);
		            VectorRotate(o->Direction,Matrix,Position);
		            VectorAdd(o->StartPosition,Position,o->StartPosition);
                }
                break;
            case MODEL_FLY_BIG_STONE1:
            case MODEL_FLY_BIG_STONE2:
                break;

            case MODEL_SKILL_JAVELIN:
                break;
            case MODEL_SWORD_FORCE:
                {
                    BMD *b = &Models[o->Type];
                    b->CurrentAction = o->CurrentAction;
                    b->PlayAnimation ( &o->AnimationFrame, &o->PriorAnimationFrame, &o->PriorAction, o->Velocity, o->Position, o->Angle );
    				MoveParticle(o,true);
                }
                break;
			default:
				MoveParticle(o,true);
				break;
			}
		}
	}
	o->LifeTime--;
	if(o->LifeTime <= 0)
	{
#ifdef YDG_ADD_SKILL_FLAME_STRIKE
		EffectDestructor(o);
#else	// YDG_ADD_SKILL_FLAME_STRIKE
		o->Live = false;
#endif	// YDG_ADD_SKILL_FLAME_STRIKE
	}
	else
	{
		switch ( o->Type)
		{
		case BITMAP_LIGHT:
			{
				if ( o->SubType==0 && 0 != ( o->LifeTime % 2))
					MoveEffect( o, iIndex);
			}
			break;
		case MODEL_FIRE:
			if ( o->SubType == 3)
			{
				if ( 0 != ( o->LifeTime % 2))
					MoveEffect( o, iIndex);
			}
			break;
		}
	}
#ifdef USE_SELFCHECKCODE
	END_OF_FUNCTION( Pos_SelfCheck01);
Pos_SelfCheck01:
	;
#endif
}

void MoveEffects()
{
#ifdef DO_PROFILING
	g_pProfiler->BeginUnit( EPROFILING_MOVE_EFFECTS, PROFILING_MOVE_EFFECTS );
#endif // DO_PROFILING

	if(SceneFlag == MAIN_SCENE)
	{
		g_pCatapultWindow->SetCameraPos();
	}

	for(int i=0;i<MAX_EFFECTS;i++)
	{
		OBJECT *o = &Effects[i];
		if(o->Live)
		{
			MoveEffect( o, i);
		}
	}

#ifdef YDG_MOD_SEPARATE_EFFECT_SKILLS
	g_SkillEffects.MoveEffects();
#endif	// YDG_MOD_SEPARATE_EFFECT_SKILLS

#ifdef DO_PROFILING
	g_pProfiler->EndUnit( EPROFILING_MOVE_EFFECTS );
#endif // DO_PROFILING
}

void RenderWheelWeapon(OBJECT *o)
{
	vec3_t TempPosition,TempAngle;
	VectorCopy(o->Position,TempPosition);
	VectorCopy(o->Angle,TempAngle);

	o->Direction[2] -= 30;
	o->Angle[2] += o->Direction[2];
	o->Angle[1] = 90;
	o->Position[2] += 100.f;

	float Alpha = o->Alpha;
	
	int Type = o->Owner->Weapon+MODEL_SWORD;
	BMD *b = &Models[Type];
	b->CurrentAction = 0;
	b->Skin          = GetBaseClass(Hero->Class);
	b->CurrentAction = o->CurrentAction;
	VectorCopy(o->Position,b->BodyOrigin);

	float TempType = o->Type;
	o->Type = Type;
	ItemObjectAttribute(o);
    b->Animation(BoneTransform,o->AnimationFrame,o->PriorAnimationFrame,o->PriorAction,o->Angle,o->HeadAngle,false,false);
	vec3_t Light;
	RequestTerrainLight(o->Position[0],o->Position[1],Light);
	VectorAdd(Light,o->Light,Light);
    RenderPartObject(o,Type,NULL,Light,Alpha,o->Owner->WeaponLevel<<3,0,0,true,true,true);
	o->Type = ( short)TempType;

	VectorCopy(TempPosition,o->Position);
	VectorCopy(TempAngle,o->Angle);
}

void RenderFuryStrike(OBJECT *o)
{
    if ( o->LifeTime>10.f && o->Kind==0 )
    {
		vec3_t p, Light, Angle;
		float Alpha = o->Alpha;

        Vector(0.f,100.f,0.f,p);
		
        //  무기를 나타내준다.
		int Type = o->Owner->Weapon+MODEL_SWORD;
		BMD *b = &Models[Type];
		b->CurrentAction = 0;
		b->Skin          = GetBaseClass(Hero->Class);
		b->CurrentAction = o->CurrentAction;
		VectorCopy(o->Position,b->BodyOrigin);

		float TempType = o->Type;
		float tempGravity = o->Gravity;
		o->Type = Type;
		ItemObjectAttribute(o);
		o->Gravity = tempGravity;
		
        RequestTerrainLight(o->Position[0],o->Position[1],Light);
		VectorAdd(Light,o->Light,Light);
        VectorCopy(o->Angle,Angle);

        b->Animation(BoneTransform,o->AnimationFrame,o->PriorAnimationFrame,o->PriorAction,Angle,o->HeadAngle,false,false);
        RenderPartObject(o,Type,NULL,Light,Alpha,o->Owner->WeaponLevel<<3,0,0,true,true,true);

		o->Type = ( short)TempType;
    }
}


void RenderSkillSpear( OBJECT *o)
{
	BMD *b = &Models[MODEL_SPEARSKILL];
	b->Animation(BoneTransform,o->AnimationFrame,o->PriorAnimationFrame,o->PriorAction,o->Angle,o->HeadAngle,false,false);
	//o->BlendMeshLight = .5f;
	//RenderPartObject(o,MODEL_SPEARSKILL,NULL,o->Light,( float)( o->LifeTime) * 0.05f,0,0,true,true,true,false,RENDER_BRIGHT|RENDER_TEXTURE);
	o->BlendMeshLight = 1.0f;
	vec3_t Light;
	for ( int i = 0; i < 3; ++i)
		Light[i] = o->Light[i] * o->LifeTime * 0.05f;
    RenderPartObject(o,MODEL_SPEARSKILL,NULL,Light,1.0f,0,0,0,true,true,true,false,RENDER_BRIGHT|RENDER_TEXTURE);
}


void RenderEffects ( bool bRenderBlendMesh )
{
#ifdef DO_PROFILING
	g_pProfiler->BeginUnit( EPROFILING_RENDER_EFFECTS, PROFILING_RENDER_EFFECTS );
#endif // DO_PROFILING
	
#ifdef MR0
	VPManager::Enable();
#endif //MR0
#ifdef YDG_MOD_SEPARATE_EFFECT_SKILLS
	int iEffectSize = MAX_EFFECTS + g_SkillEffects.GetSize();
	for(int i = 0; i < iEffectSize; ++i)
	{
		OBJECT *o;
		if (i < MAX_EFFECTS)
		{
			o = &Effects[i];
		}
		else
		{
			o = g_SkillEffects.GetEffect(i - MAX_EFFECTS);
		}
#else	// YDG_MOD_SEPARATE_EFFECT_SKILLS
	for(int i=0;i<MAX_EFFECTS;i++)
	{

		OBJECT *o = &Effects[i];
#endif	// YDG_MOD_SEPARATE_EFFECT_SKILLS
		if(o->Live)
		{
#ifdef DYNAMIC_FRUSTRUM
			CFrustrum* pFrus = FindFrustrum(o->m_iID);
			if(pFrus) pFrus->Test(o->Position, 400.f);
			else o->Visible = TestFrustrum(o->Position,400.f);
#else
        	o->Visible = TestFrustrum(o->Position,400.f);
#endif //DYNAMIC_FRUSTRUM
			if(World == WD_39KANTURU_3RD)
				if(o->Type == MODEL_STORM3 || o->Type == MODEL_MAYASTAR)
					o->Visible = true;

			if(o->Visible)
			{
                if ( bRenderBlendMesh )
                {
                    if ( o->BlendMesh==-1 || o->BlendMesh<-2 ) continue;            //  블랜딩 되지 않는 오브젝트는 찍지 않는다.
                    const BMD& b = Models[o->Type];
                    if ( b.NumMeshs<o->BlendMesh ) continue;                        //  블랜딩 되는 오브젝트가 원래 오브젝트보다 크면 찍지 않는다.
                    //if ( (o->Position[2]+o->BoundingBoxMax[2])<350.f ) continue;    //  물 기본 높이보다 
                }


				switch ( o->Type)
				{
				case MODEL_DRAGON:
				case MODEL_WARP3:
				case MODEL_WARP2:
				case MODEL_WARP:
#ifdef CSK_ADD_MAP_ICECITY
				case MODEL_WARP6:
				case MODEL_WARP5:
				case MODEL_WARP4:
#endif // CSK_ADD_MAP_ICECITY
#ifdef PJH_SEASON4_DARK_NEW_SKILL_CAOTIC
				case MODEL_DESAIR:
#endif //SKILL_DEBUG
				case MODEL_GHOST:			//  아이다 유령 박스
				case MODEL_TREE_ATTACK:		//  아이다 데쓰트리 공격용 나무
					RenderObject(o);
					break;
#ifdef PJH_SEASON4_SPRITE_NEW_SKILL_MULTI_SHOT
			case MODEL_MULTI_SHOT1:
			case MODEL_MULTI_SHOT2:
			case MODEL_MULTI_SHOT3:
			{
				BMD *b = &Models[o->Type];
				Vector(1.f,1.f,1.f,b->BodyLight);
				b->PlayAnimation( &o->AnimationFrame, &o->PriorAnimationFrame, &o->PriorAction, o->Velocity/6.f, o->Position, o->Angle );
				RenderObject(o);
			}
			break;
#endif //PJH_SEASON4_SPRITE_NEW_SKILL_MULTI_SHOT
				case MODEL_STORM:

					if(o->SubType == 3 || o->SubType == 4 || o->SubType == 5 || o->SubType == 6 || o->SubType  == 7)
					{
#ifdef MR0
						VPManager::Disable();
#endif //MR0
						vec3_t Light;
						EnableAlphaBlend();
						Vector(0.3f, 0.3f, 0.3f, Light);
						float Rotation = (float)(WorldTime);
						RenderTerrainAlphaBitmap(BITMAP_POUNDING_BALL,o->Position[0],o->Position[1],2.f,2.f,Light, Rotation);
#ifdef MR0
						VPManager::Enable();
#endif //MR0
					}
					else
						RenderObject(o);
					break;
				case MODEL_STORM3:
				case MODEL_MAYAHANDSKILL:
					o->m_bRenderAfterCharacter = true;
					break;
				case MODEL_SUMMON:
				case MODEL_STORM2:
				case MODEL_MAYASTAR:
				case MODEL_MAYASTONE1:
				case MODEL_MAYASTONE2:
				case MODEL_MAYASTONE3:
				case MODEL_MAYASTONE4:
				case MODEL_MAYASTONE5:
				case MODEL_MAYASTONEFIRE:
				case MODEL_BUTTERFLY01:		//. 반디불 박스 : 2004/11/05
					RenderObject(o);
					break;
                case 9:
					RenderObject(o);
                    break;

				case MODEL_HALLOWEEN_CANDY_BLUE:
				case MODEL_HALLOWEEN_CANDY_ORANGE:
				case MODEL_HALLOWEEN_CANDY_YELLOW:
				case MODEL_HALLOWEEN_CANDY_RED:
				case MODEL_HALLOWEEN_CANDY_HOBAK:
				case MODEL_HALLOWEEN_CANDY_STAR:
					RenderObject(o);
					break;
				case MODEL_HALLOWEEN:
				case MODEL_HALLOWEEN_EX:
					break;

				case MODEL_XMAS_EVENT_BOX:
				case MODEL_XMAS_EVENT_CANDY:
				case MODEL_XMAS_EVENT_TREE:
				case MODEL_XMAS_EVENT_SOCKS:
					RenderObject(o);
					break;
				case MODEL_XMAS_EVENT_ICEHEART:
					RenderObject(o);
					break;

				case MODEL_NEWYEARSDAY_EVENT_BEKSULKI:
				case MODEL_NEWYEARSDAY_EVENT_CANDY:
				case MODEL_NEWYEARSDAY_EVENT_MONEY:
				case MODEL_NEWYEARSDAY_EVENT_HOTPEPPER_GREEN:
				case MODEL_NEWYEARSDAY_EVENT_HOTPEPPER_RED:
				case MODEL_NEWYEARSDAY_EVENT_PIG:
				case MODEL_NEWYEARSDAY_EVENT_YUT:
					RenderObject(o);
					break;
					
#ifdef PRUARIN_EVENT07_3COLORHARVEST
				case MODEL_MOONHARVEST_MOON:
					{
						if(o->SubType == 0)
						{
							vec3_t vLight;
							// 달 주위 빛 (Shockwave)
							Vector(0.6f, 0.8f, 0.6f, vLight);
							o->m_iAnimation++;
							CreateSprite( BITMAP_SHOCK_WAVE, o->Position, 0.8f, vLight, o, -(o->m_iAnimation*3.f));
							// Flare1
							Vector(0.8f, 0.6f, 0.f, vLight);
							CreateSprite( BITMAP_LIGHT, o->Position, 5.0f, vLight, o , NULL);
							RenderObject(o);
						}
#ifdef CSK_RAKLION_BOSS
						else if(o->SubType == 1)
						{
							CreateSprite( BITMAP_LIGHT, o->Position, 5.0f, o->Light, o , NULL);
							CreateSprite( BITMAP_LIGHT, o->Position, 3.0f, o->Light, o , NULL);
							CreateSprite( BITMAP_LIGHT, o->Position, 3.0f, o->Light, o , NULL);
							CreateSprite( BITMAP_SHINY+6, o->Position, 2.0f, o->Light, o , NULL);
							
							RenderObject(o);
						}
#endif // CSK_RAKLION_BOSS
#ifdef LDK_ADD_GAMBLERS_WEAPONS
						else if(o->SubType == 2)
						{
							RenderObject(o);
						}
#endif //LDK_ADD_GAMBLERS_WEAPONS

					}
					break;
					
				case MODEL_MOONHARVEST_GAM:
				case MODEL_MOONHARVEST_SONGPUEN1:
				case MODEL_MOONHARVEST_SONGPUEN2:
					RenderObject(o);
					break;
#endif // PRUARIN_EVENT07_3COLORHARVEST

				case MODEL_CHANGE_UP_EFF:
					RenderObject(o);
					break;
				case MODEL_CURSEDTEMPLE_HOLYITEM:
				case MODEL_CURSEDTEMPLE_PRODECTION_SKILL:
					{
						RenderObject(o);
					}
					break;
				case MODEL_CURSEDTEMPLE_RESTRAINT_SKILL:
					{
						RenderObject(o);
					}
					break;
				case MODEL_SPEARSKILL:
					RenderSkillSpear(o);
					break;
				case BITMAP_FIRE_CURSEDLICH:
					break;
				case MODEL_SUMMONER_WRISTRING_EFFECT:
					RenderObject(o);
					break;
				case MODEL_SUMMONER_EQUIP_HEAD_SAHAMUTT:
					if (o->SubType == 0)
					{
						RenderObject(o);
					}
					break;
				case MODEL_SUMMONER_EQUIP_HEAD_NEIL:
					if (o->SubType == 0)
					{
						RenderObject(o);
					}
					break;
				case MODEL_SUMMONER_CASTING_EFFECT1:
				case MODEL_SUMMONER_CASTING_EFFECT11:
				case MODEL_SUMMONER_CASTING_EFFECT111:
				case MODEL_SUMMONER_CASTING_EFFECT2:
				case MODEL_SUMMONER_CASTING_EFFECT22:
				case MODEL_SUMMONER_CASTING_EFFECT222:
				case MODEL_SUMMONER_CASTING_EFFECT4:
					RenderObject(o);
					break;
				case MODEL_SUMMONER_SUMMON_SAHAMUTT:
					RenderObject(o);
					break;
				case MODEL_SUMMONER_SUMMON_NEIL:
					RenderObject(o);
					break;
				case MODEL_SUMMONER_SUMMON_NEIL_NIFE1:
				case MODEL_SUMMONER_SUMMON_NEIL_NIFE2:
				case MODEL_SUMMONER_SUMMON_NEIL_NIFE3:
					RenderObject(o);
					break;
				case MODEL_SUMMONER_SUMMON_NEIL_GROUND1:
				case MODEL_SUMMONER_SUMMON_NEIL_GROUND2:
				case MODEL_SUMMONER_SUMMON_NEIL_GROUND3:
					RenderObject(o);
					break;
#ifdef ASG_ADD_SUMMON_RARGLE
				case MODEL_SUMMONER_SUMMON_LAGUL:
					if (o->SubType == 1)
					{
						BMD* pModel = &Models[o->Type];
						vec3_t vPos, vLight;
						
					// 라글 등의 빛.
						const int nBoneCount = 6;
						int nBone[nBoneCount] = { 54, 55, 56, 57, 58, 59 };
						Vector(0.2f, 0.3f, 1.0f, vLight);
						// 애니메이션이 있는 모델에서 본위치를 구하기 위해서는 아래와 같이
						//강제적으로 애니메이션 함수를 호출.
						pModel->Animation(BoneTransform, o->AnimationFrame, o->PriorAnimationFrame,
							o->PriorAction, o->Angle, o->HeadAngle, false, false);
						for (int i = 0; i < nBoneCount; ++i)
						{
							pModel->TransformByObjectBone(vPos, o, nBone[i]);
							CreateSprite(BITMAP_SHINY+6, vPos, o->Scale * 0.3f, vLight, NULL);
						}

					// 라글이 뿌리는 연기.
						if (rand()%5 == 0)
						{
							int nBones = Models[o->Type].NumBones;
							pModel->TransformByObjectBone(vPos, o, rand()%nBones);
							Vector(0.6f, 0.5f, 0.9f, vLight);
							CreateParticle(BITMAP_SMOKE, vPos, o->Angle, vLight, 57, 2.0f);
							Vector(0.7f, 0.7f, 1.f, vLight);
							CreateParticle(BITMAP_TWINTAIL_WATER, vPos, o->Angle, vLight, 1);
						}
						RenderObject(o);
					}
					break;
#endif	// ASG_ADD_SUMMON_RARGLE
#ifdef ADD_RAKLION_IRON_KNIGHT
				case MODEL_EFFECT_BROKEN_ICE0:
				case MODEL_EFFECT_BROKEN_ICE1:
				case MODEL_EFFECT_BROKEN_ICE2:
				case MODEL_EFFECT_BROKEN_ICE3:
					RenderObject(o);
					break;
#endif	// ADD_RAKLION_IRON_KNIGHT
				case MODEL_SKILL_WHEEL1:
					break;
				case MODEL_SKILL_WHEEL2:
					RenderWheelWeapon(o);
					break;
				case MODEL_SKILL_FURY_STRIKE:
					RenderFuryStrike(o);
					break;
                case MODEL_ARROW_HOLY:
                    break;
				case BITMAP_BOSS_LASER:
				case BITMAP_BOSS_LASER+1:
				case BITMAP_BOSS_LASER+2://이미지 없음
					{
						vec3_t Position;
						VectorCopy(o->Position,Position);
						for(int j=0;j<20;j++)
						{
							if(o->SubType == 1)
							{
#ifdef MR0
								VPManager::Disable();
#endif //MR0
								o->Angle[2] -= 0.1f;
								Vector ( 1.0f, 1.0f, 1.0f, o->Light );
								CreateSprite(BITMAP_FIRE+1,Position,o->Scale,o->Light,o, o->Angle[2]);
								EnableAlphaBlend();
								Vector ( 0.2f, 0.0f, 0.0f, o->Light );
								RenderTerrainAlphaBitmap(BITMAP_SMOKE,Position[0],Position[1],4.0f,4.0f,o->Light,-o->Angle[2]);
								DisableAlphaBlend();
#ifdef MR0
								VPManager::Enable();
#endif //MR0
							}
							else if(o->SubType == 2)
							{
								Vector ( 0.0f, 0.2f, 1.f, o->Light );
								o->Angle[2] -= 0.1f;
								CreateSprite(BITMAP_FIRE+1,Position,o->Scale,o->Light,o, o->Angle[2]);
							}
							else
								CreateSprite(BITMAP_SPARK+1,Position,o->Scale,o->Light,o);


							VectorAdd(Position,o->Direction,Position);
                            
                            if ( battleCastle::IsBattleCastleStart() )
                            {
                                DWORD att = TERRAIN_ATTRIBUTE ( Position[0], Position[1] );
                                if ( (att&TW_NOATTACKZONE)==TW_NOATTACKZONE )
                                {
                                    return;
                                }
                            }
							AddTerrainLight(Position[0],Position[1],o->Light,2,PrimaryTerrainLight);
						}
					}
					break;

                case MODEL_GATE:            //  블러드캐슬 성문.
                case MODEL_GATE+1:
                case MODEL_STONE_COFFIN:    //  블러드캐슬 성자의 석상.
                case MODEL_STONE_COFFIN+1:
				case MODEL_STAFF+8:
                case MODEL_CLOUD:
	      			RenderObject(o);
					break;
                case MODEL_SHINE:
                    break;
#ifdef TAMAJJANG
                case MODEL_TAMAJJANG:
	      			RenderObject(o);
                    break;
#endif
				case MODEL_WAVE_FORCE:
	      			RenderObject(o);
					break;
				case MODEL_MAGIC_CAPSULE2:	// 법사 - 방어막
					RenderObject(o);
					break;
				case MODEL_AIR_FORCE :	//  공기 저항 효과.
                case MODEL_PIER_PART:
					RenderObject(o);
                    break;
				case MODEL_PIERCING2:
					if(o->SubType == 1
#ifdef PJH_SEASON4_SPRITE_NEW_SKILL_MULTI_SHOT
		|| o->SubType == 2 
#endif //PJH_SEASON4_SPRITE_NEW_SKILL_MULTI_SHOT
						)
						break;
					RenderObject(o);
                    break;
#ifdef CRYINGWOLF_2NDMVP
				case MODEL_PIERCING:
					if(o->SubType == 3)
						break;
					RenderObject(o);
					break;
#endif // CRYINGWOLF_2NDMVP
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
                case MODEL_TOWER_GATE_PLANE:
                    RenderObject ( o );
 
                    o->Position[2] = o->StartPosition[2]+400.f-(o->Position[2]-o->StartPosition[2]);
                    RenderObject ( o );
                    break;

                case BATTLE_CASTLE_WALL1:
                case BATTLE_CASTLE_WALL2:
                case BATTLE_CASTLE_WALL3:
                case BATTLE_CASTLE_WALL4:
                    RenderObject ( o );
                    break;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
				//^ 펜릴 이펙트 관련
				case MODEL_FENRIR_THUNDER:
						RenderObject(o);
					break;
				case MODEL_FALL_STONE_EFFECT:
					RenderObject(o);
					break;
				case MODEL_FENRIR_FOOT_THUNDER:
				{
#ifdef MR0
					VPManager::Disable();
#endif //MR0
					EnableAlphaBlend();
					RenderTerrainAlphaBitmap(BITMAP_FENRIR_FOOT_THUNDER1+(o->m_iAnimation%5), o->Position[0], o->Position[1], 0.6f, 0.6f, o->Light);
					DisableAlphaBlend();
#ifdef MR0
					VPManager::Enable();
#endif //MR0
				}
					break;
				case MODEL_TWINTAIL_EFFECT:
					{
#ifdef MR0
						VPManager::Disable();
#endif //MR0
						if(o->SubType == 0)
						{
							EnableAlphaBlend();
							RenderTerrainAlphaBitmap(BITMAP_SPARK+1, o->Position[0], o->Position[1], o->Scale, o->Scale, o->Light, 0.f, o->Alpha);
							DisableAlphaBlend();
						}
						else if(o->SubType == 1 || o->SubType == 2)
						{
							EnableAlphaBlend();
							RenderTerrainAlphaBitmap(BITMAP_CLOUD, o->Position[0], o->Position[1], o->Scale, o->Scale, o->Light, o->Angle[0], o->Alpha);
							DisableAlphaBlend();
						}
#ifdef MR0
						VPManager::Enable();
#endif //MR0
					}
					break;
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
				case MODEL_CUNDUN_PART1:
				case MODEL_CUNDUN_PART2:
				case MODEL_CUNDUN_PART3:
				case MODEL_CUNDUN_PART4:
				case MODEL_CUNDUN_PART5:
				case MODEL_CUNDUN_PART6:
				case MODEL_CUNDUN_PART7:
				case MODEL_CUNDUN_PART8:
				case MODEL_MONSTER01+64:	// 진짜쿤둔
				case MODEL_CUNDUN_DRAGON_HEAD:
				case MODEL_CUNDUN_PHOENIX:
				case MODEL_CUNDUN_GHOST:
      				RenderObject(o);
					break;
				case MODEL_CURSEDTEMPLE_STATUE_PART1:
 				case MODEL_CURSEDTEMPLE_STATUE_PART2:
     				RenderObject(o);
					break;
#ifdef LDK_ADD_SNOWMAN_CHANGERING
				case MODEL_XMAS2008_SNOWMAN_HEAD:
				case MODEL_XMAS2008_SNOWMAN_BODY:
     				RenderObject(o);
					break;
#endif //LDK_ADD_SNOWMAN_CHANGERING
#ifdef PJH_ADD_PANDA_CHANGERING
				case MODEL_PANDA:
     				RenderObject(o);
					break;
#endif //PJH_ADD_PANDA_CHANGERING
				case MODEL_TOTEMGOLEM_PART1:
				case MODEL_TOTEMGOLEM_PART2:
				case MODEL_TOTEMGOLEM_PART3:
				case MODEL_TOTEMGOLEM_PART4:
				case MODEL_TOTEMGOLEM_PART5:
				case MODEL_TOTEMGOLEM_PART6:
     				RenderObject(o);
					break;
				case MODEL_SHADOW_PAWN_ANKLE_LEFT:		case MODEL_SHADOW_PAWN_ANKLE_RIGHT:
				case MODEL_SHADOW_PAWN_BELT:			case MODEL_SHADOW_PAWN_CHEST:
				case MODEL_SHADOW_PAWN_HELMET:
				case MODEL_SHADOW_PAWN_KNEE_LEFT:		case MODEL_SHADOW_PAWN_KNEE_RIGHT:
				case MODEL_SHADOW_PAWN_WRIST_LEFT:		case MODEL_SHADOW_PAWN_WRIST_RIGHT:

				case MODEL_SHADOW_KNIGHT_ANKLE_LEFT:	case MODEL_SHADOW_KNIGHT_ANKLE_RIGHT:
				case MODEL_SHADOW_KNIGHT_BELT:			case MODEL_SHADOW_KNIGHT_CHEST:
				case MODEL_SHADOW_KNIGHT_HELMET:
				case MODEL_SHADOW_KNIGHT_KNEE_LEFT:		case MODEL_SHADOW_KNIGHT_KNEE_RIGHT:
				case MODEL_SHADOW_KNIGHT_WRIST_LEFT:	case MODEL_SHADOW_KNIGHT_WRIST_RIGHT:

				case MODEL_SHADOW_ROOK_ANKLE_LEFT:		case MODEL_SHADOW_ROOK_ANKLE_RIGHT:
				case MODEL_SHADOW_ROOK_BELT:			case MODEL_SHADOW_ROOK_CHEST:
				case MODEL_SHADOW_ROOK_HELMET:
				case MODEL_SHADOW_ROOK_KNEE_LEFT:		case MODEL_SHADOW_ROOK_KNEE_RIGHT:
				case MODEL_SHADOW_ROOK_WRIST_LEFT:		case MODEL_SHADOW_ROOK_WRIST_RIGHT:

#ifdef ADD_RAKLION_MOB_ICEGIANT
				case MODEL_ICE_GIANT_PART1:				case MODEL_ICE_GIANT_PART2:
				case MODEL_ICE_GIANT_PART3:				case MODEL_ICE_GIANT_PART4:
				case MODEL_ICE_GIANT_PART5:				case MODEL_ICE_GIANT_PART6:
#endif	// ADD_RAKLION_MOB_ICEGIANT
     				RenderObject(o);
					break;

				case MODEL_PROTECTGUILD:
					RenderObject(o);
					break;
				case MODEL_ARROW_AUTOLOAD:
					if(o->SubType == 1) RenderObject(o);
					break;

				case MODEL_INFINITY_ARROW:
					if(o->SubType == 1) RenderObject(o);
					break;

				case MODEL_INFINITY_ARROW1:
				case MODEL_INFINITY_ARROW2:
				case MODEL_INFINITY_ARROW3:
				case MODEL_INFINITY_ARROW4:
					RenderObject(o);
					break;

				case MODEL_ARROW_BEST_CROSSBOW:
					RenderObject(o);
					break;

				case MODEL_ALICE_BUFFSKILL_EFFECT:
				case MODEL_ALICE_BUFFSKILL_EFFECT2:
					{
						if(o->SubType == 0 || o->SubType == 1 || o->SubType == 2)
						{
							RenderObject(o);
						}
					}
					break;

//------------------------------------------------------------------------------------------------------------	

#ifdef CSK_RAKLION_BOSS

				case MODEL_RAKLION_BOSS_CRACKEFFECT:
					{
						RenderObject(o);
					}
					break;

//------------------------------------------------------------------------------------------------------------		

				case MODEL_RAKLION_BOSS_MAGIC:
					{
						RenderObject(o);
					}
					break;
					
#endif // CSK_RAKLION_BOSS
#ifdef PBG_ADD_PKFIELD
				case MODEL_LAVAGIANT_FOOTPRINT_R:
				case MODEL_LAVAGIANT_FOOTPRINT_V:	
					{
						EnableAlphaBlend();
						if(o->Type == MODEL_LAVAGIANT_FOOTPRINT_R)
						{
							RenderTerrainAlphaBitmap(BITMAP_LAVAGIANT_FOOTPRINT_R, o->Position[0], o->Position[1], o->Scale, o->Scale, o->Light);
						}
						else
						{
							RenderTerrainAlphaBitmap(BITMAP_LAVAGIANT_FOOTPRINT_V, o->Position[0], o->Position[1], o->Scale, o->Scale, o->Light);
						}
						DisableAlphaBlend();
					}
					break;
#endif //PBG_ADD_PKFIELD
#ifdef PBG_ADD_CHARACTERSLOT
			case MODEL_SLOT_LOCK:
				{
					RenderObject( o );					
				}
				break;
#endif //PBG_ADD_CHARACTERSLOT
//------------------------------------------------------------------------------------------------------------	
				case MODEL_MOVE_TARGETPOSITION_EFFECT:
					{
						RenderObject( o );
					}
					break;
//------------------------------------------------------------------------------------------------------------	
				case MODEL_EFFECT_SAPITRES_ATTACK_1:
				case MODEL_EFFECT_SAPITRES_ATTACK_2:
					{
						RenderObject( o );
					}
					break;
//--------------------------------------------------------------------------------------------------------------------------------

#ifdef CSK_EVENT_CHERRYBLOSSOM
				//case MODEL_EFFECT_SKURA_ITEM:
					//{
						//RenderObject( o );
					//}
					//break;
#endif //CSK_EVENT_CHERRYBLOSSOM


//--------------------------------------------------------------------------------------------------------------------------------
					
#ifdef CSK_ADD_SKILL_BLOWOFDESTRUCTION
				case MODEL_BLOW_OF_DESTRUCTION:
					{
						if(o->SubType == 0)
						{
							if(o->LifeTime <= 24)
							{
								vec3_t vLight, vPos;
								Vector(0.5f, 0.5f, 1.f, vLight);
								VectorCopy(o->Position, vPos);
								vPos[2] += 65.f;
								float fScale = (float)(rand()%10 * 0.011f) + 3.f;
								CreateSprite(BITMAP_SWORD_EFFECT_MONO, vPos, fScale, vLight, NULL);
							}
						}
						else if(o->SubType == 1)
						{
							if(o->LifeTime <= 24)
							{
								vec3_t vLight, vPos;
								Vector(o->Light[0] * 0.5f, o->Light[1] * 0.5f, o->Light[2] * 1.0f, vLight);
								VectorCopy(o->Position, vPos);
								vPos[2] += 100.f;
								CreateSprite(BITMAP_LIGHT, vPos, o->Scale, vLight, NULL);
							}
						}
					}
					break;

//--------------------------------------------------------------------------------------------------------------------------------	
					
				case MODEL_NIGHTWATER_01:
				case MODEL_KNIGHT_PLANCRACK_A:
				case MODEL_KNIGHT_PLANCRACK_B:
					{
						RenderObject(o);		
					}
					break;	
#endif // CSK_ADD_SKILL_BLOWOFDESTRUCTION	

//--------------------------------------------------------------------------------------------------------------------------------

#ifdef YDG_ADD_SKILL_FLAME_STRIKE
				case MODEL_EFFECT_FLAME_STRIKE:
					{
						if(o->SubType == 0)
						{
							OBJECT * pObject = o;
							BMD * pModel = &Models[pObject->Type];
							OBJECT * pOwner = pObject->Owner;
							BMD * pOwnerModel = &Models[pOwner->Type];
							
							vec3_t vPos, p;
							Vector(0.0f, 0.0f, 0.0f, p);
							pOwnerModel->RotationPosition(pOwner->BoneTransform[33], p, vPos);	// ParentMatrix 전역변수 생성용
							VectorAdd(pOwner->Position, vPos, pModel->BodyOrigin);
							Vector(0.0f, 0.0f, 0.0f, pObject->Angle);
							
							OBB_t OBB;
							vec3_t Temp;
							pModel->Animation(BoneTransform,pObject->AnimationFrame,pObject->PriorAnimationFrame,pObject->PriorAction,pObject->Angle,pObject->Angle,true);	// BoneTransform 전역변수 생성
							pModel->Transform(BoneTransform,Temp,Temp,&OBB,true);	// BoneTransform으로 실제 모델 transform
							
							BodyLight(pObject, pModel);
							pModel->BodyScale = pObject->Scale;

#ifdef LDS_MR0_MODIFY_MISSRENDERSHADOW_MOSTMESH		// LDS_MR0_MODIFY_MISSRENDERSHADOW_MOSTMESH
							ModelManager::SetTargetObject(o);
#endif // LDS_MR0_MODIFY_MISSRENDERSHADOW_MOSTMESH

							pModel->RenderMesh(0,RENDER_TEXTURE|RENDER_BRIGHT,pObject->Alpha,0,pObject->Alpha,pObject->BlendMeshTexCoordU,pObject->BlendMeshTexCoordV,-1);
							float fV = (((int)(WorldTime*0.05)%16)/4)*0.25f;
							pModel->RenderMesh(1,RENDER_TEXTURE|RENDER_BRIGHT,pObject->Alpha,1,pObject->Alpha,pObject->BlendMeshTexCoordU, fV );

#ifdef LDS_MR0_MODIFY_MISSRENDERSHADOW_MOSTMESH		// LDS_MR0_MODIFY_MISSRENDERSHADOW_MOSTMESH
							ModelManager::SetTargetObject(NULL);
#endif // LDS_MR0_MODIFY_MISSRENDERSHADOW_MOSTMESH
						}
					}
					break;
#endif	// YDG_ADD_SKILL_FLAME_STRIKE

#ifdef ASG_ADD_SKILL_BERSERKER
				case BITMAP_LIGHT_MARKS:
					{
						float fLight = 1.035f;
						if (o->LifeTime >= 35)
						{
							o->Light[0] /= fLight;
							o->Light[1] /= fLight;
							o->Light[2] /= fLight;
						}
						else
						{
							o->Light[0] *= fLight;
							o->Light[1] *= fLight;
							o->Light[2] *= fLight;
						}
						// 플레이어 모델
						BMD* pModel = &Models[o->Owner->Type];
						vec3_t vPos;
						const int nBoneCount = 14;
						int nBone[nBoneCount]
							= { 20, 20, 19, 18, 17, 2, 35, 26, 36, 27, 37, 28, 39, 30 };
						float fScale[nBoneCount]
							= { 1.5f, 1.5f, 0.6f, 1.1f, 0.9f, 0.8f, 0.6f, 0.6f,
							0.8f, 0.8f, 0.8f, 0.8f, 0.7f, 0.7f};
						for (int i = 0; i < nBoneCount; ++i)
						{
							pModel->TransformByObjectBone(vPos, o->Owner, nBone[i]);
							CreateSprite(o->Type, vPos, o->Scale * fScale[i], o->Light, o->Owner);
						}
					}
					break;
#endif	// ASG_ADD_SKILL_BERSERKER
//--------------------------------------------------------------------------------------------------------------------------------
#ifdef KJH_ADD_SKILL_SWELL_OF_MAGICPOWER
				case MODEL_SWELL_OF_MAGICPOWER_BUFF_EFF:
					{
						if( o->SubType == 0 )
						{
							// 플레이어일때 이팩트 사용. (본넘버를 사용하기 때문에위험!!!) 
							BMD* pModel = &Models[o->Owner->Type];
							vec3_t vPos, vDLight;
							if ( o->Owner->Type != MODEL_PLAYER )
								break;
							
							// 온몸에 빛나는 효과
							float fLumi = (absf((sinf( WorldTime*0.001f )))+0.2f)*0.5f;	// 페이드인/아웃
// 							if( fLumi >= 0.5f)
// 							{
// 								fLumi = 0.5f;
// 							}
							Vector( fLumi*0.7f, fLumi*0.3f, fLumi*0.9f, vDLight);

							for( int i=0 ; i<pModel->NumBones ; i++ )
							{
								pModel->TransformByObjectBone( vPos, o->Owner, i );	
								
								CreateSprite( BITMAP_LIGHT, vPos, 1.8f, vDLight, o );
							}
						}
					}break;
#endif // KJH_ADD_SKILL_SWELL_OF_MAGICPOWER
#ifdef LDS_ADD_EMPIRE_GUARDIAN
				case MODEL_PROJECTILE:
					{
						RenderObject(o);
					}
					break;

				case MODEL_DOOR_CRUSH_EFFECT_PIECE01:	//성문 파괴 이펙트 조각 1
				case MODEL_DOOR_CRUSH_EFFECT_PIECE02:	//성문 파괴 이펙트 조각 2
				case MODEL_DOOR_CRUSH_EFFECT_PIECE03:	//성문 파괴 이펙트 조각 3
				case MODEL_DOOR_CRUSH_EFFECT_PIECE04:	//성문 파괴 이펙트 조각 4
				case MODEL_DOOR_CRUSH_EFFECT_PIECE05:	//성문 파괴 이펙트 조각 5
				case MODEL_DOOR_CRUSH_EFFECT_PIECE06:	//성문 파괴 이펙트 조각 6
				case MODEL_DOOR_CRUSH_EFFECT_PIECE07:	//성문 파괴 이펙트 조각 7
				case MODEL_DOOR_CRUSH_EFFECT_PIECE08:	//성문 파괴 이펙트 조각 8
				case MODEL_DOOR_CRUSH_EFFECT_PIECE09:	//성문 파괴 이펙트 조각 9
				case MODEL_DOOR_CRUSH_EFFECT_PIECE10:	//성문 파괴 이펙트 조각 7
				case MODEL_DOOR_CRUSH_EFFECT_PIECE11:	//성문 파괴 이펙트 조각 7
				case MODEL_DOOR_CRUSH_EFFECT_PIECE12:	//성문 파괴 이펙트 조각 8
				case MODEL_DOOR_CRUSH_EFFECT_PIECE13:	//성문 파괴 이펙트 조각 9
				case MODEL_STATUE_CRUSH_EFFECT_PIECE01:	//석상 파괴 이펙트 조각 1
				case MODEL_STATUE_CRUSH_EFFECT_PIECE02:	//석상 파괴 이펙트 조각 2
				case MODEL_STATUE_CRUSH_EFFECT_PIECE03:	//석상 파괴 이펙트 조각 3
				case MODEL_STATUE_CRUSH_EFFECT_PIECE04:	//석상 파괴 이펙트 조각 4
					{
						RenderObject(o);
					}
					break;
#endif //LDS_ADD_EMPIRE_GUARDIAN
#ifdef LDS_ADD_EG_4_MONSTER_WORLDBOSS_GAIONKALEIN
				case MODEL_SWORDLEFT01_EMPIREGUARDIAN_BOSS_GAION_:
				case MODEL_SWORDLEFT02_EMPIREGUARDIAN_BOSS_GAION_:
				case MODEL_SWORDRIGHT01_EMPIREGUARDIAN_BOSS_GAION_:
				case MODEL_SWORDRIGHT02_EMPIREGUARDIAN_BOSS_GAION_:
				case MODEL_SWORDMAIN01_EMPIREGUARDIAN_BOSS_GAION_:
					{
//						RenderObject(o);
						// 1. 각 가이온의 스워드들의 Attach 및 Render
						BMD *pBMDSwordModel = &Models[o->Type]; 
						
						vec3_t			*arrEachBoneTranslations;			// 현재 모델의 각 Bone들의 위치값.
						vec3_t			v3CurrentHighHierarchyNodePos;		// 상위 모델의 ATTACH 본 위치값.

						// 1-1. 상위 모델 가이온인 경우만 가이온의 Bone을 가져와서 그리도록 한다.
						if( true == o->ChromeEnable )		// 1-1. 일반 모델 렌더의 경우 : 상속관계 처리를 해준다.
						{
							arrEachBoneTranslations = new vec3_t[pBMDSwordModel->NumBones];

							BMD *pOwnerModel = &Models[o->Owner->Type];
							
							int arrBoneIdxs[] = { 4, 2, 8, 10, 6 };		// 각 모델별 참조해야할 가이온의 본 INDEX. 
							int iBoneIdx = arrBoneIdxs[o->Type-MODEL_SWORDLEFT01_EMPIREGUARDIAN_BOSS_GAION_];
							// 1-1-1. o를 상위 모델로 o의 특정 본에 현재 모델pBMDSwordModel을 attach 하여 Vertex들에 적용하여 준다.
							//		동시에 attach 할 위치값(arg4)과 현재 모델pBMDSwordModel의 각 본별 Translation들을 Out한다.
#ifdef LDS_ADD_ANIMATIONTRANSFORMWITHMODEL_USINGGLOBALTM
							pBMDSwordModel->AnimationTransformWithAttachHighModel_usingGlobalTM( o->Owner, pOwnerModel, iBoneIdx, 
													v3CurrentHighHierarchyNodePos, arrEachBoneTranslations, true );
#else // LDS_ADD_ANIMATIONTRANSFORMWITHMODEL_USINGGLOBALTM
							pBMDSwordModel->AnimationTransformWithAttachHighModel( o->Owner, pOwnerModel, iBoneIdx, 
													v3CurrentHighHierarchyNodePos, arrEachBoneTranslations );
#endif // LDS_ADD_ANIMATIONTRANSFORMWITHMODEL_USINGGLOBALTM
						}
	 					else	// 상속관계 처리를 하지 않는다.
	 					{
							pBMDSwordModel->BodyHeight = 0.f;
							pBMDSwordModel->ContrastEnable = o->ContrastEnable;
							BodyLight(o,pBMDSwordModel);
							pBMDSwordModel->BodyScale = o->Scale;
							pBMDSwordModel->CurrentAction = o->CurrentAction;
							VectorCopy(o->Position,pBMDSwordModel->BodyOrigin);

							pBMDSwordModel->Animation(BoneTransform,o->AnimationFrame,o->PriorAnimationFrame,o->PriorAction,o->Angle,o->HeadAngle,false,true );
							pBMDSwordModel->Transform(BoneTransform,o->BoundingBoxMin,o->BoundingBoxMax,&o->OBB,false);

							arrEachBoneTranslations = new vec3_t[pBMDSwordModel->NumBones];
	 						pBMDSwordModel->AnimationTransformOnlySelf( arrEachBoneTranslations, o );
	 					}

						pBMDSwordModel->LightEnable = true;
					
#ifdef MR0
						ModelManager::SetTargetObject(o);
#endif // MR0

						Vector( 1.0f, 1.0f, 1.0f, pBMDSwordModel->BodyLight );
						pBMDSwordModel->RenderMesh ( 0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );

						Vector( 1.0f, 1.0f, 1.0f, pBMDSwordModel->BodyLight );
						pBMDSwordModel->RenderMesh ( 1, RENDER_TEXTURE|RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );

						Vector( 0.2f, 0.2f, 1.0f, pBMDSwordModel->BodyLight );
						o->Alpha	= 1.0f;
						pBMDSwordModel->RenderMesh ( 2, RENDER_TEXTURE|RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );
						o->Alpha	= 1.0f;

#ifdef MR0
						ModelManager::SetTargetObject(NULL);
#endif // MR0

						if( o->SubType != 20 )
						{
							// 2. 모든 가이온의 검의 기본 EFFECT 정의
							// 2-1. 기본 Effect 정의
							vec3_t	vRelative, vLight;
							vec3_t	vPos_SwordEffectRed01, 
								vPos_SwordEffectRed02, 
								vPos_SwordEffectEdge01,
								vPos_SwordEffectEdge02,
								vPos_SwordEffectEdge03,
								vPos_SwordEffectEdge04,
								vPos_SwordEffectEdge05,
								vPos_SwordEffectEdge06,
								vPos_SwordEffectEdge07,
								vPos_SwordEffectEdge08,
								vPos_SwordEffectEdge09;
							float	fLumi1, fLumi2;
							int		arrBoneIdxs_SwordEffectRed01[]	= { 2,	9,	1,	3,	10 };	// SWORD 모델별 참조해야할 MainEffect01 BoneINDEX. 
							int		arrBoneIdxs_SwordEffectRed02[]	= { 3,	10, 2,	10, 11 };	// SWORD 모델별 참조해야할 MainEffect02 BoneINDEX. 
							int		arrBoneIdxs_SwordEffectEdge01[] = { 13, 13, 3,	4,	1 };	// SWORD 모델별 참조해야할 EdgeEffect01 BoneINDEX. 
							int		arrBoneIdxs_SwordEffectEdge02[] = { 12, 6,	4,	12, 2 };	// SWORD 모델별 참조해야할 EdgeEffect02 BoneINDEX. 
							int		arrBoneIdxs_SwordEffectEdge03[] = { 5,	1,	5,	2,	3 };	// SWORD 모델별 참조해야할 EdgeEffect03 BoneINDEX. 
							int		arrBoneIdxs_SwordEffectEdge04[] = { 6,	7,	6,	9,	4 };	// SWORD 모델별 참조해야할 EdgeEffect04 BoneINDEX. 
							int		arrBoneIdxs_SwordEffectEdge05[] = { 1,	2,	12,	5,	5 };	// SWORD 모델별 참조해야할 EdgeEffect05 BoneINDEX. 
							int		arrBoneIdxs_SwordEffectEdge06[] = { 8,	4,	8,	7,	6 };	// SWORD 모델별 참조해야할 EdgeEffect06 BoneINDEX. 
							int		arrBoneIdxs_SwordEffectEdge07[] = { 9,	3,	9,	6,	7 };	// SWORD 모델별 참조해야할 EdgeEffect07 BoneINDEX. 
							int		arrBoneIdxs_SwordEffectEdge08[] = { 10, 11, 10, 8,	8 };	// SWORD 모델별 참조해야할 EdgeEffect08 BoneINDEX. 
							int		arrBoneIdxs_SwordEffectEdge09[] = { 4,	8,	7,	1,	9 };	// SWORD 모델별 참조해야할 EdgeEffect09 BoneINDEX. 
							//MODEL_SWORDLEFT01_EMPIREGUARDIAN_BOSS_GAION_  
							//MODEL_SWORDLEFT02_EMPIREGUARDIAN_BOSS_GAION_ 
							//MODEL_SWORDRIGHT01_EMPIREGUARDIAN_BOSS_GAION_
							//MODEL_SWORDRIGHT02_EMPIREGUARDIAN_BOSS_GAION_
							//MODEL_SWORDMAIN01_EMPIREGUARDIAN_BOSS_GAION_
							
							int		iBoneIdx_SwordEffectMain01 = arrBoneIdxs_SwordEffectRed01[o->Type-MODEL_SWORDLEFT01_EMPIREGUARDIAN_BOSS_GAION_];
							int		iBoneIdx_SwordEffectMain02 = arrBoneIdxs_SwordEffectRed02[o->Type-MODEL_SWORDLEFT01_EMPIREGUARDIAN_BOSS_GAION_];
							
							int		iBoneIdx_SwordEffectEdge01 = arrBoneIdxs_SwordEffectEdge01[o->Type-MODEL_SWORDLEFT01_EMPIREGUARDIAN_BOSS_GAION_];
							int		iBoneIdx_SwordEffectEdge02 = arrBoneIdxs_SwordEffectEdge02[o->Type-MODEL_SWORDLEFT01_EMPIREGUARDIAN_BOSS_GAION_];
							int		iBoneIdx_SwordEffectEdge03 = arrBoneIdxs_SwordEffectEdge03[o->Type-MODEL_SWORDLEFT01_EMPIREGUARDIAN_BOSS_GAION_];
							int		iBoneIdx_SwordEffectEdge04 = arrBoneIdxs_SwordEffectEdge04[o->Type-MODEL_SWORDLEFT01_EMPIREGUARDIAN_BOSS_GAION_];
							int		iBoneIdx_SwordEffectEdge05 = arrBoneIdxs_SwordEffectEdge05[o->Type-MODEL_SWORDLEFT01_EMPIREGUARDIAN_BOSS_GAION_];
							int		iBoneIdx_SwordEffectEdge06 = arrBoneIdxs_SwordEffectEdge06[o->Type-MODEL_SWORDLEFT01_EMPIREGUARDIAN_BOSS_GAION_];
							int		iBoneIdx_SwordEffectEdge07 = arrBoneIdxs_SwordEffectEdge07[o->Type-MODEL_SWORDLEFT01_EMPIREGUARDIAN_BOSS_GAION_];
							int		iBoneIdx_SwordEffectEdge08 = arrBoneIdxs_SwordEffectEdge08[o->Type-MODEL_SWORDLEFT01_EMPIREGUARDIAN_BOSS_GAION_];
							int		iBoneIdx_SwordEffectEdge09 = arrBoneIdxs_SwordEffectEdge09[o->Type-MODEL_SWORDLEFT01_EMPIREGUARDIAN_BOSS_GAION_];
							
							fLumi1 = 1.0f; fLumi2 = 1.0f;
							Vector(0.0f, 0.0f, 0.0f, vRelative);
							
							Vector(0.0f, 0.0f, 0.0f, vLight);
							
							// 2-2. 기본 Jewel Effect // 
							Vector( fLumi1 * 1.0f, fLumi1 * 0.4f, fLumi1 * 0.1f, vLight );
							
							//			if( MODEL_MONSTER01+164 == o->Owner->Type )	// 2-3-1. 일반 모델 렌더의 경우
							{
								VectorCopy( arrEachBoneTranslations[iBoneIdx_SwordEffectMain01] , vPos_SwordEffectRed01 );
								VectorCopy( arrEachBoneTranslations[iBoneIdx_SwordEffectMain02] , vPos_SwordEffectRed02 );
							}
							// 			else // 2-3-1. Effect를 통한 렌더의 경우.
							// 			{			
							// 				pBMDSwordModel->TransformByObjectBone(vPos_SwordEffectRed01, o, iBoneIdx_SwordEffectMain01, vRelative);
							// 				pBMDSwordModel->TransformByObjectBone(vPos_SwordEffectRed02, o, iBoneIdx_SwordEffectMain02, vRelative);
							// 			}
							
							CreateSprite(BITMAP_LIGHT_RED, vPos_SwordEffectRed01, 1.3f, vLight, o);
							CreateSprite(BITMAP_LIGHT_RED, vPos_SwordEffectRed02, 1.3f, vLight, o);
							// 2-2. 기본 Jewel Effect // 
							
							// 2-3. 기본 Edge Effect
							{
								VectorCopy( arrEachBoneTranslations[iBoneIdx_SwordEffectEdge01] , vPos_SwordEffectEdge01 );
								VectorCopy( arrEachBoneTranslations[iBoneIdx_SwordEffectEdge02] , vPos_SwordEffectEdge02 );
								VectorCopy( arrEachBoneTranslations[iBoneIdx_SwordEffectEdge03] , vPos_SwordEffectEdge03 );
								VectorCopy( arrEachBoneTranslations[iBoneIdx_SwordEffectEdge04] , vPos_SwordEffectEdge04 );
								VectorCopy( arrEachBoneTranslations[iBoneIdx_SwordEffectEdge05] , vPos_SwordEffectEdge05 );
								VectorCopy( arrEachBoneTranslations[iBoneIdx_SwordEffectEdge06] , vPos_SwordEffectEdge06 );
								VectorCopy( arrEachBoneTranslations[iBoneIdx_SwordEffectEdge07] , vPos_SwordEffectEdge07 );
								
								VectorCopy( arrEachBoneTranslations[iBoneIdx_SwordEffectEdge08] , vPos_SwordEffectEdge08 );
								VectorCopy( arrEachBoneTranslations[iBoneIdx_SwordEffectEdge09] , vPos_SwordEffectEdge09 );
							}
							
							Vector( fLumi1 * 0.3f, fLumi1 * 0.3f, fLumi1 * 1.0f, vLight );
							CreateSprite(BITMAP_LIGHT, vPos_SwordEffectEdge01, 1.5f, vLight, o);
							CreateSprite(BITMAP_LIGHT, vPos_SwordEffectEdge02, 1.5f, vLight, o);
							CreateSprite(BITMAP_LIGHT, vPos_SwordEffectEdge03, 1.5f, vLight, o);
							CreateSprite(BITMAP_LIGHT, vPos_SwordEffectEdge04, 1.5f, vLight, o);
							CreateSprite(BITMAP_LIGHT, vPos_SwordEffectEdge05, 1.5f, vLight, o);
							CreateSprite(BITMAP_LIGHT, vPos_SwordEffectEdge06, 1.5f, vLight, o);
							CreateSprite(BITMAP_LIGHT, vPos_SwordEffectEdge07, 1.5f, vLight, o);
							
							//Vector( fLumi1 * 0.3f, fLumi1 * 0.5f, fLumi1 * 0.1f, vLight );
							CreateSprite(BITMAP_LIGHT, vPos_SwordEffectEdge08, 1.0f, vLight, o);
							CreateSprite(BITMAP_LIGHT, vPos_SwordEffectEdge09, 1.0f, vLight, o);
							// 2-3. 기본 Edge Effect // 

						}

						delete [] arrEachBoneTranslations;
					}
					break;
				case MODEL_EMPIREGUARDIANBOSS_FRAMESTRIKE:
					{
						RenderObject(o);
					}
					break;
#endif // LDS_ADD_EG_4_MONSTER_WORLDBOSS_GAIONKALEIN
#ifdef LDS_ADD_EG_4_MONSTER_JELINT
				case MODEL_EMPIREGUARDIAN_BLOW_OF_DESTRUCTION:
					{
						if(o->SubType == 0)
						{
							if(o->LifeTime <= 24)
							{
								vec3_t vLight, vPos;
								Vector(0.5f, 0.5f, 1.f, vLight);
								VectorCopy(o->Position, vPos);
								vPos[2] += 65.f;
								float fScale = (float)(rand()%10 * 0.011f) + 3.f;
								CreateSprite(BITMAP_SWORD_EFFECT_MONO, vPos, fScale, vLight, NULL);
							}
						}
						else if(o->SubType == 1)
						{
							if(o->LifeTime <= 24)
							{
								vec3_t vLight, vPos;
								Vector(o->Light[0] * 0.5f, o->Light[1] * 0.5f, o->Light[2] * 1.0f, vLight);
								VectorCopy(o->Position, vPos);
								vPos[2] += 100.f;
								CreateSprite(BITMAP_LIGHT, vPos, o->Scale, vLight, NULL);
							}
						}
					}
					break;
#endif // LDS_ADD_EG_4_MONSTER_JELINT
#ifdef LDK_ADD_EXTENSIONMAP_BOSS_MEDUSA_WAND_EFFECT
				case BITMAP_WATERFALL_4:
					{
						//CreateSprite(BITMAP_WATERFALL_4, o->Position, o->Scale, o->Light, o, o->Angle[0]);
					}
					break;
#endif //LDK_ADD_EXTENSIONMAP_BOSS_MEDUSA_WAND_EFFECT
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
				case BITMAP_EVENT_CLOUD:
					{
						EnableAlphaBlend();
						RenderTerrainAlphaBitmap(BITMAP_EVENT_CLOUD, o->Position[0], o->Position[1], o->Scale, o->Scale, o->Light, o->Angle[0], o->Alpha);
						DisableAlphaBlend();
					}
					break;
				case MODEL_SHOCKWAVE02:
					{
						RenderObject(o);
					}
					break;
				case MODEL_DRAGON_LOWER_DUMMY:
					{
						BMD *b = &Models[o->Type];
						b->CurrentAction = o->CurrentAction;
						b->PlayAnimation(&o->AnimationFrame, &o->PriorAnimationFrame, &o->PriorAction, o->Velocity/6.f, o->Position, o->Angle);
						RenderObject(o);
					}
					break;
				case BITMAP_LIGHT_RED:
					{
						vec3_t vLight;
						VectorCopy(o->Light, vLight);
						float fScale=1.0f;
						fScale = 3.5f;
						Vector(1.0f,1.0f,1.0f,vLight);
						float fFadeInOut = (float)(sinf(WorldTime*0.0005f)+1.0f)*0.25f+0.2f;
						if(o->SubType==3 || o->SubType==4)
						{
							fScale = o->Scale;
							fFadeInOut = o->LifeTime*0.01f;
						}
						VectorScale(vLight, fFadeInOut, vLight);
						
						if(o->SubType==0)
						{
							EnableAlphaBlend();
							RenderTerrainAlphaBitmap(BITMAP_LIGHT_RED, o->Owner->Position[0], o->Owner->Position[1], fScale, fScale, vLight, o->Angle[0], o->Alpha);
							RenderTerrainAlphaBitmap(BITMAP_LIGHT_RED, o->Owner->Position[0], o->Owner->Position[1], fScale, fScale, vLight, o->Angle[0], o->Alpha);
							DisableAlphaBlend();
						}
						else if(o->SubType==3)
						{
							EnableAlphaBlend();
							RenderTerrainAlphaBitmap(BITMAP_LIGHT_RED, o->Owner->Position[0], o->Owner->Position[1], fScale, fScale, vLight, o->Angle[0], o->Alpha);
							DisableAlphaBlend();
						}
						else if(o->SubType==4)
						{
							EnableAlphaBlend();
							RenderTerrainAlphaBitmap(BITMAP_LIGHT_RED, o->Position[0], o->Position[1], fScale, fScale, vLight, o->Angle[0], o->Alpha);
							RenderTerrainAlphaBitmap(BITMAP_LIGHT_RED, o->Position[0], o->Position[1], fScale, fScale, vLight, o->Angle[0], o->Alpha);
							DisableAlphaBlend();
						}
						
						if(o->SubType == 0)
						{
							vec3_t Light, Angle;
							int arr[17] = {12, 17, 5,	10, 36, 27, 37,		28, 11, 35,		2, 3, 36, 20,	27, 4,26 };
							BMD* b = &Models[o->Owner->Type];
							vec3_t _vPos;
							Vector(0.f, 0.f, 0.f, Angle);
							
							for(int i=0; i<17; ++i)
							{
								float fFadeInOut = (float)(sinf(WorldTime*0.01f)+1.0f)*0.25f+0.2f;
								VectorCopy(o->Owner->Position, o->Position);
								Vector(1.0f, 1.0f, 1.0f, Light);

								if(i == 0 || i == 1 || i == 2)
								{
									//Vector(0.0f, 0.0f, 15.0f, _vPos);
									Vector(0.0f, 0.0f, 0.0f, _vPos);
									b->TransformByObjectBone(_vPos, o->Owner, arr[i], _vPos);
									VectorScale(Light, fFadeInOut, Light);
									CreateSprite(BITMAP_LIGHT_RED, _vPos, 1.5f, Light, o);
								}
								else if(i == 5 || i == 12 || i == 7 || i == 8 || i == 9 || i == 14 || i == 15 || i == 16)
								{
									//Vector(0.0f, 5.0f, 0.0f, _vPos);
									Vector(0.0f, 0.0f, 0.0f, _vPos);
									b->TransformByObjectBone(_vPos, o->Owner, arr[i], _vPos);
									fFadeInOut = (float)(sinf(WorldTime*0.005f)+1.0f)*0.25f+0.5f;
									VectorScale(Light, fFadeInOut, Light);
									CreateSprite(BITMAP_LIGHT_RED, _vPos, 1.5f, Light, o);
								}
								else
								{
									Vector(0.0f, 0.0f, 0.0f, _vPos);
									b->TransformByObjectBone(_vPos, o->Owner, arr[i], _vPos);
									fFadeInOut = (float)(sinf(WorldTime*0.01f)+1.0f)*0.3f+0.3f;
									VectorScale(Light, fFadeInOut, Light);
									CreateSprite(BITMAP_LIGHT_RED, _vPos, 1.5f, Light, o);
								}
							}
						}
					}
					break;
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
//--------------------------------------------------------------------------------------------------------------------------------

				default:
					if(o->Type>=MODEL_SKILL_BEGIN && o->Type<MODEL_SKILL_END)
					{
						RenderObject(o);
					}
					break;

//--------------------------------------------------------------------------------------------------------------------------------	
				}
			}
		}
	}

#ifdef MR0
	VPManager::Disable();
#endif //MR0

#ifdef DO_PROFILING
	g_pProfiler->EndUnit( EPROFILING_RENDER_EFFECTS );
#endif // DO_PROFILING
}

void RenderAfterEffects ( bool bRenderBlendMesh )
{
	if(!g_Direction.m_CKanturu.IsMayaScene())
		return;

#ifdef YDG_MOD_SEPARATE_EFFECT_SKILLS
	int iEffectSize = MAX_EFFECTS + g_SkillEffects.GetSize();
	for(int i = 0; i < iEffectSize; ++i)
	{
		OBJECT *o;
		if (i < MAX_EFFECTS)
		{
			o = &Effects[i];
		}
		else
		{
			o = g_SkillEffects.GetEffect(i - MAX_EFFECTS);
		}
#else	// YDG_MOD_SEPARATE_EFFECT_SKILLS
	for(int i=0;i<MAX_EFFECTS;i++)
	{
		OBJECT *o = &Effects[i];
#endif	// YDG_MOD_SEPARATE_EFFECT_SKILLS
		if(o->Live && o->m_bRenderAfterCharacter )
		{
#ifdef DYNAMIC_FRUSTRUM
			CFrustrum* pFrus = FindFrustrum(o->m_iID);
			if(pFrus) pFrus->Test(o->Position, 400.f);
			else o->Visible = TestFrustrum(o->Position,400.f);
#else
        	o->Visible = TestFrustrum(o->Position,400.f);
#endif //DYNAMIC_FRUSTRUM

			if(World == WD_39KANTURU_3RD)
				if(o->Type == MODEL_STORM3 || o->Type == MODEL_MAYASTAR)
					o->Visible = true;

			if(o->Visible)
			{
                if ( bRenderBlendMesh )
                {
                    if ( o->BlendMesh==-1 || o->BlendMesh<-2 ) continue;            //  블랜딩 되지 않는 오브젝트는 찍지 않는다.
                    const BMD& b = Models[o->Type];
                    if ( b.NumMeshs<o->BlendMesh ) continue;                        //  블랜딩 되는 오브젝트가 원래 오브젝트보다 크면 찍지 않는다.
                }

				switch ( o->Type)
				{
				case MODEL_STORM3:
				case MODEL_MAYASTAR:
				case MODEL_MAYAHANDSKILL:
	      			RenderObject_AfterCharacter(o);
				}
			}
		}
	}
}

void RenderEffectShadows()
{
#ifdef YDG_MOD_SEPARATE_EFFECT_SKILLS
	int iEffectSize = MAX_EFFECTS + g_SkillEffects.GetSize();
	for(int i = 0; i < iEffectSize; ++i)
	{
		OBJECT *o;
		if (i < MAX_EFFECTS)
		{
			o = &Effects[i];
		}
		else
		{
			o = g_SkillEffects.GetEffect(i - MAX_EFFECTS);
		}
#else	// YDG_MOD_SEPARATE_EFFECT_SKILLS
	for(int i=0;i<MAX_EFFECTS;i++)
	{
		OBJECT *o = &Effects[i];
#endif	// YDG_MOD_SEPARATE_EFFECT_SKILLS
		if(o->Live)
		{
			if(o->Visible)
			{
				vec3_t Light;
				float Luminosity = (float)(rand()%4+8)*0.1f;
				float Rotation,Scale;
				EnableAlphaBlend();
				
				switch(o->Type)
				{
                case BITMAP_MAGIC:
                    if(o->SubType==1)
                    {
    					RenderTerrainAlphaBitmap(BITMAP_MAGIC,o->Position[0],o->Position[1],o->Scale,o->Scale,o->Light,-o->Angle[2]);

                        o->Light[0] /= 1.1f;
                        o->Light[1] /= 1.1f;
                        o->Light[2] /= 1.1f;
                        o->Scale += 0.05f;
                    }
					else if ( o->SubType == 8 )
					{
					    RenderTerrainAlphaBitmap ( o->Type, o->Position[0], o->Position[1], o->Scale, o->Scale, o->Light, -o->Angle[2] );
					    RenderTerrainAlphaBitmap ( o->Type, o->Position[0], o->Position[1], o->Scale*0.8f, o->Scale*0.8f, o->Light, -o->Angle[2] );
					    RenderTerrainAlphaBitmap ( o->Type, o->Position[0], o->Position[1], o->Scale*1.2f, o->Scale*1.2f, o->Light, -o->Angle[2] );
					}
					else if ( o->SubType == 9 )
					{
						vec3_t vLight;
						Vector(o->Light[0] * o->Alpha, o->Light[1] * o->Alpha, o->Light[2] * o->Alpha, vLight);
					    RenderTerrainAlphaBitmap ( BITMAP_SUMMON_IMPACT, o->Position[0], o->Position[1], o->Scale, o->Scale, vLight, o->HeadAngle[1] );
					    //RenderTerrainAlphaBitmap ( BITMAP_SUMMON_IMPACT, o->Position[0], o->Position[1], o->Scale*0.8f, o->Scale*0.8f, vLight, o->HeadAngle[0] );
					    RenderTerrainAlphaBitmap ( BITMAP_SUMMON_IMPACT, o->Position[0], o->Position[1], o->Scale*1.2f, o->Scale*1.2f, vLight, o->HeadAngle[2] );
					}
					else if ( o->SubType == 10 )
					{
						EnableAlphaBlendMinus();
						vec3_t vLight;
						Vector(o->Light[0] * o->Alpha, o->Light[1] * o->Alpha, o->Light[2] * o->Alpha, vLight);
						for (int i = 0; i < 5; ++i)
						{
							if (o->LifeTime < 3 && i > 0) continue;
							else if (o->LifeTime < 6 && i > 1) continue;
							else if (o->LifeTime < 9 && i > 2) continue;
							else if (o->LifeTime < 12 && i > 3) continue;
							else if (o->LifeTime < 16 && i > 4) continue;
						    RenderTerrainAlphaBitmap ( BITMAP_LIGHT, o->Position[0], o->Position[1], o->Scale, o->Scale, vLight, -o->Angle[2] );
						}
					}
					else if ( o->SubType == 11 )
					{
						vec3_t vLight;
						Vector(o->Light[0] * o->Alpha, o->Light[1] * o->Alpha, o->Light[2] * o->Alpha, vLight);
					    RenderTerrainAlphaBitmap( BITMAP_SUMMON_IMPACT, o->Position[0], o->Position[1], o->Scale, o->Scale, vLight, o->HeadAngle[1] );
					    RenderTerrainAlphaBitmap( BITMAP_SUMMON_IMPACT, o->Position[0], o->Position[1], o->Scale*1.2f, o->Scale*1.2f, vLight, o->HeadAngle[2] );
					}
#ifdef YDG_ADD_SKILL_LIGHTNING_SHOCK
					else if ( o->SubType == 12 )
					{
					    RenderTerrainAlphaBitmap ( o->Type, o->Position[0], o->Position[1], o->Scale, o->Scale, o->Light, -o->Angle[2] );
					    RenderTerrainAlphaBitmap ( o->Type, o->Position[0], o->Position[1], o->Scale, o->Scale, o->Light, -o->Angle[2] );
					}
#endif	// YDG_ADD_SKILL_LIGHTNING_SHOCK
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
					else if(o->SubType==13 || o->SubType==14)
					{
						RenderTerrainAlphaBitmap(o->Type, o->Position[0], o->Position[1], o->Scale, o->Scale, o->Light, -o->Angle[2]);
						RenderTerrainAlphaBitmap(o->Type, o->Position[0], o->Position[1], o->Scale, o->Scale, o->Light, -o->Angle[2]);
					}
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
                    break;

//-------------------------------------------------------------------------------------------------------------------------------------------

				case BITMAP_MAGIC+1:
					Luminosity = 1.f;
					if(o->LifeTime < 5)
					{
						Luminosity -= (float)(5-o->LifeTime)*0.2f;
					}
					else if(o->SubType == 7)
					{
						if (o->LifeTime > 30)
							Luminosity = (float)(40-o->LifeTime)*0.1f;
					}
				
                    if ( o->SubType==4 || o->SubType==10 )
                    {
                        Scale = o->Scale;//(rand()%100)/100.f*4.f;
                        if ( Luminosity==1.f )
                        {
                            Luminosity = sinf ( (60-o->LifeTime)*0.05f )*1.f+0.5f;
                        }
                    }
					else if(o->SubType == 6)
					{
						Scale = (80-o->LifeTime)*0.05f;
					}
					else if(o->SubType == 7)
					{
						Scale = (o->LifeTime)*0.07f;
					}
					else if(o->SubType == 9)
					{
						Scale = o->Scale;
					}
					else if(o->SubType == 11)
					{
						 Scale = (20-o->LifeTime)*0.15f;
					}
					else if(o->SubType == 12)
					{
						Scale = (20-o->LifeTime)*0.15f;
					}
#ifdef CSK_RAKLION_BOSS
					else if(o->SubType == 13)
					{
						o->Light[0] /= 1.05f;
						o->Light[1] /= 1.05f;
						o->Light[2] /= 1.05f;
						Scale = o->Scale;
					}
#endif // CSK_RAKLION_BOSS	
                    else
                    {
                        Scale = (20-o->LifeTime)*0.15f;
                    }
					if (o->SubType == 7)
					{
						VectorCopy(o->Owner->Position, o->Position);
						o->Angle[2] += 10.0f;
					}
					if ( o->SubType!=5 && o->SubType!=6 && o->SubType!=8 )
                    {
						switch(o->SubType)
						{
						case 0:
							Vector(Luminosity*0.4f,Luminosity*0.6f,Luminosity*1.f,Light);
							break;
						case 1:
							Vector(Luminosity*0.4f,Luminosity*0.6f,Luminosity*1.f,Light);
							break;
						case 2:
							Vector(Luminosity*0.4f,Luminosity*1.f,Luminosity*0.6f,Light);
							break;
						case 3:
							Vector(Luminosity*1.f,Luminosity*0.6f,Luminosity*0.4f,Light);
							break;
						case 4:
							Vector(Luminosity*1.f,Luminosity*0.5f,Luminosity*0.1f,Light);
							break;
						case 7:
							Vector(Luminosity*0.9f,Luminosity*0.4f,Luminosity*0.2f,Light);
							break;
						case 9:
							Vector(Luminosity*0.9f,Luminosity*0.4f,Luminosity*0.2f,Light);
							break;
						case 10:
							Vector(Luminosity*0.1f,Luminosity*0.5f,Luminosity*1.f,Light);
							break;
						case 11:
							Vector(Luminosity*o->Light[0],Luminosity*o->Light[1],Luminosity*o->Light[2],Light);
							break;
						case 12:	
							EnableAlphaBlendMinus();
							Vector(Luminosity*o->Light[0],Luminosity*o->Light[1],Luminosity*o->Light[2],Light);
							break;
#ifdef CSK_RAKLION_BOSS
						case 13:
							Vector(o->Light[0],o->Light[1],o->Light[2],Light);
							break;
#endif // CSK_RAKLION_BOSS
						}
						RenderTerrainAlphaBitmap(BITMAP_MAGIC+1,o->Position[0],o->Position[1],Scale,Scale,Light,-o->Angle[2]);
					}
					else if( o->SubType == 6 && o->LifeTime%25 == 0 )
					{
						CreateEffect(BITMAP_MAGIC+1,o->Position,o->Angle,o->Light,7,o->Owner);
					}
					else if( o->SubType == 8 && o->LifeTime%2 == 0 )
					{
						CreateEffect(BITMAP_MAGIC+1,o->Position,o->Angle,o->Light,9,o->Owner);
					}
					break;

//-----------------------------------------------------------------------------------------------------		

#ifdef CSK_RAKLION_BOSS

				case BITMAP_FIRE_HIK2_MONO:
					if(o->SubType == 0)
					{
						Vector(Luminosity*1.f,Luminosity*1.0f,Luminosity*1.0f,Light);
						RenderTerrainAlphaBitmap(BITMAP_LIGHTNING+1,o->Position[0],o->Position[1],2.f,2.f,Light,-o->Angle[2]);
					}
					break;

				case BITMAP_CLOUD:
					if(o->SubType == 0)
					{
						Vector(Luminosity*o->Light[0],Luminosity*o->Light[1],Luminosity*o->Light[2],Light);
						RenderTerrainAlphaBitmap(BITMAP_CLOUD,o->Position[0],o->Position[1],o->Scale,o->Scale,Light,-o->Angle[2]);
					}
					break;

//-----------------------------------------------------------------------------------------------------			

				case MODEL_RAKLION_BOSS_MAGIC:
					{
						if(o->SubType == 0)
						{
							Vector(Luminosity*0.4f,Luminosity*0.4f,Luminosity*1.f,Light);
							RenderTerrainAlphaBitmap(BITMAP_LIGHT,o->Position[0],o->Position[1],15.f,15.f,Light,-o->Angle[2]);
							Vector(Luminosity*1.0f,Luminosity*1.0f,Luminosity*1.f,Light);
							RenderTerrainAlphaBitmap(BITMAP_LIGHT,o->Position[0],o->Position[1],5.f,5.f,Light,-o->Angle[2]);
						}
					}
					break;

//-----------------------------------------------------------------------------------------------------			

				case MODEL_EFFECT_BROKEN_ICE0:
				case MODEL_EFFECT_BROKEN_ICE1:
				case MODEL_EFFECT_BROKEN_ICE2:
				case MODEL_EFFECT_BROKEN_ICE3:
					{
						if(o->SubType == 1)
						{
							Vector(Luminosity*o->Light[0],Luminosity*o->Light[0],Luminosity*o->Light[0],Light);
							RenderTerrainAlphaBitmap(BITMAP_LIGHT,o->Position[0],o->Position[1],5.f,5.f,Light,-o->Angle[2]);
						}
					}		
					break;
#endif // CSK_RAKLION_BOSS

//-----------------------------------------------------------------------------------------------------	

				case BITMAP_MAGIC+2:
                    EnableAlphaBlend();
					Rotation = (int)WorldTime%3600/(float)10.f;

                    Luminosity = 1.f;
                    if(o->SubType!=2)
                    {
					    RenderCircle(BITMAP_MAGIC+2,o->Position,90.f,130.f,200.f, Rotation,0.f,0.f);
					    RenderCircle(BITMAP_MAGIC+2,o->Position,90.f,130.f,200.f,-Rotation,0.f,0.f);

					    if(o->LifeTime < 5) Luminosity -= (float)(5-o->LifeTime)*0.2f;
    					Scale = (20-o->LifeTime)*0.15f;
                    }
                    else if(o->SubType==2)
                    {
                        if ( o->LifeTime>10 )
                        {
    					    Scale = (20-o->LifeTime)*0.55f;
                        }
                        else
                        {
					        Luminosity -= (float)(10-o->LifeTime)*0.1f;
                        }
                    }
					Vector(Luminosity*1.f,Luminosity*0.4f,Luminosity*0.2f,Light);
    				RenderTerrainAlphaBitmap(BITMAP_MAGIC+1,o->Position[0],o->Position[1],Scale,Scale,Light,-o->Angle[2]);
                    break;

				case BITMAP_MAGIC_ZIN:
					{
						vec3_t vLight;
						switch( o->SubType )
						{
						case 0:
							Vector(o->Light[0] * o->Alpha * 2.f, o->Light[1] * o->Alpha * 2.f, o->Light[2] * o->Alpha * 2.f, vLight);							
							break;
						case 1:							
							Vector(o->Light[0] * o->Alpha / 2.5f, o->Light[1] * o->Alpha / 2.5f, o->Light[2] * o->Alpha / 2.5f, vLight);
							break;
						case 2:
							Vector(o->Light[0] * o->Alpha, o->Light[1] * o->Alpha, o->Light[2] * o->Alpha, vLight);
							break;
						}
						RenderTerrainAlphaBitmap(o->Type, o->Position[0], o->Position[1], o->Scale, o->Scale, vLight, o->HeadAngle[1]);
					}
					break;

#ifdef ASG_ADD_INFLUENCE_GROUND_EFFECT
				case BITMAP_OUR_INFLUENCE_GROUND:
					if (o->SubType == 0)
					{
						vec3_t vLight;
						// 바깥 문양.
						Vector(0.6f*o->Alpha, 0.9f*o->Alpha, 1.0f*o->Alpha, vLight);
						RenderTerrainAlphaBitmap(o->Type, o->Position[0], o->Position[1], o->Scale, o->Scale, vLight, 45.f);
						// 안쪽 문양.
						Vector(0.6f*o->AlphaTarget, 0.9f*o->AlphaTarget, 1.0f*o->AlphaTarget, vLight);
						RenderTerrainAlphaBitmap(o->Type, o->Position[0], o->Position[1], 0.8f, 0.8f, vLight, 45.f);
						// 가운데 이펙트.
						Vector(0.2f*o->AlphaTarget, 0.8f*o->AlphaTarget, 1.0f*o->AlphaTarget, vLight);
						RenderTerrainAlphaBitmap(BITMAP_LIGHT, o->Position[0], o->Position[1], 2.0f, 2.0f, vLight);
						RenderTerrainAlphaBitmap(BITMAP_LIGHT, o->Position[0], o->Position[1], 2.0f, 2.0f, vLight);
					}
					break;
					
				case BITMAP_ENEMY_INFLUENCE_GROUND:
					if (o->SubType == 0)
					{
						vec3_t vLight;
						// 바깥 문양.
						Vector(1.0f*o->Alpha, 0.3f*o->Alpha, 0.2f*o->Alpha, vLight);
						RenderTerrainAlphaBitmap(o->Type, o->Position[0], o->Position[1], o->Scale*1.6f, o->Scale*1.6f, vLight);
						// 안쪽 문양.
						Vector(1.0f*o->AlphaTarget, 0.3f*o->AlphaTarget, 0.2f*o->AlphaTarget, vLight);
						RenderTerrainAlphaBitmap(o->Type, o->Position[0], o->Position[1], 1.15f, 1.15f, vLight);
						// 가운데 이펙트.
						Vector(1.0f*o->AlphaTarget, 1.0f*o->AlphaTarget, 1.0f*o->AlphaTarget, vLight);
						RenderTerrainAlphaBitmap(BITMAP_LIGHT_RED, o->Position[0], o->Position[1], 1.8f, 1.8f, vLight);
						Vector(1.0f*o->AlphaTarget, 0.0f*o->AlphaTarget, 0.0f*o->AlphaTarget, vLight);
						RenderTerrainAlphaBitmap(BITMAP_LIGHT, o->Position[0], o->Position[1], 2.0f, 2.0f, vLight);
					}
					break;
#endif	// ASG_ADD_INFLUENCE_GROUND_EFFECT

//-------------------------------------------------------------------------------------------------------------------------------------------

				case BITMAP_FLAME:
#ifdef LDS_ADD_EMPIRE_GUARDIAN
					if(o->SubType!=3 && o->SubType!=6 )	// 1.불골렘 공격효과, 2.제국수호군 화면상에 법사불기둥 효과시 제외
#else // LDS_ADD_EMPIRE_GUARDIAN
					if(o->SubType!=3)	// 불골렘 공격효과에서는 제외
#endif // LDS_ADD_EMPIRE_GUARDIAN
					{
						Luminosity = (float)(rand()%4+8)*0.1f;
						Vector(Luminosity,Luminosity,Luminosity,Light);
						RenderTerrainAlphaBitmap(BITMAP_FLAME,o->Position[0],o->Position[1],2.f,2.f,Light,-o->Angle[2]);
					}
					break;

//-------------------------------------------------------------------------------------------------------------------------------------------

				case BITMAP_LIGHTNING+1:
					o->Scale += 0.2f;
					Luminosity = (float)(o->LifeTime)*0.1f;
					Vector(Luminosity,Luminosity,Luminosity,Light);
					RenderTerrainAlphaBitmap(BITMAP_LIGHTNING+1,o->Position[0],o->Position[1],o->Scale,o->Scale,Light,-o->Angle[2]);
					break;

//-------------------------------------------------------------------------------------------------------------------------------------------
#ifdef PJH_SEASON4_SPRITE_NEW_SKILL_RECOVER
				case BITMAP_TWLIGHT:
#endif //PJH_SEASON4_SPRITE_NEW_SKILL_RECOVER
                case BITMAP_SHOCK_WAVE:
#ifdef KJH_ADD_SKILL_SWELL_OF_MAGICPOWER
					if ( o->Type == BITMAP_SHOCK_WAVE && InHellas() && o->SubType!=6 )
#else // KJH_ADD_SKILL_SWELL_OF_MAGICPOWER
                    if ( InHellas() && o->SubType!=6 )
#endif // KJH_ADD_SKILL_SWELL_OF_MAGICPOWER
                    {
                        DisableDepthMask();
					    RenderWaterTerrain ( o->Type, o->Position[0], o->Position[1], o->Scale, o->Scale, o->Light, -o->Angle[2] );
                        EnableDepthMask();
                    }
                    else
                    {
					    RenderTerrainAlphaBitmap ( o->Type, o->Position[0], o->Position[1], o->Scale, o->Scale, o->Light, -o->Angle[2] );
                    }
                    break;
	
//-------------------------------------------------------------------------------------------------------------------------------------------

#ifdef CSK_RAKLION_BOSS
				case BITMAP_DAMAGE_01_MONO:
					if(o->SubType == 0)
					{
						RenderTerrainAlphaBitmap ( o->Type, o->Position[0], o->Position[1], o->Scale, o->Scale, o->Light, -o->Angle[2] );
					}
#ifdef YDG_ADD_SKILL_LIGHTNING_SHOCK
					else if(o->SubType == 1)
					{
						RenderTerrainAlphaBitmap ( o->Type, o->Position[0], o->Position[1], o->Scale, o->Scale, o->Light, -o->Angle[2] );
					}
#endif	// YDG_ADD_SKILL_LIGHTNING_SHOCK
					break;
#endif // CSK_RAKLION_BOSS

//------------------------------------------------------------------------------------------------------------------------------------	
		
                case BITMAP_CRATER:
		            EnableAlphaTest ();
                    RenderTerrainAlphaBitmap( o->Type, o->Position[0], o->Position[1], o->StartPosition[0], o->StartPosition[1], o->Light );
                    break;

//------------------------------------------------------------------------------------------------------------------------------------

#ifdef CSK_ADD_SKILL_BLOWOFDESTRUCTION
				case MODEL_BLOW_OF_DESTRUCTION:
					if(o->SubType == 0)
					{
						if(o->LifeTime <= 24)
						{
							RenderTerrainAlphaBitmap(BITMAP_FLARE_BLUE,o->Position[0],o->Position[1],4.f,4.f,o->Light,-o->Angle[2]);
						}
					}
					else if(o->SubType == 1)
					{
						if(o->LifeTime <= 24)
						{
							RenderTerrainAlphaBitmap(BITMAP_FLARE_BLUE,o->Position[0],o->Position[1],6.f,6.f,o->Light,-o->Angle[2]);
						}
					}
					break;
#endif // CSK_ADD_SKILL_BLOWOFDESTRUCTION	

//------------------------------------------------------------------------------------------------------------------------------------	

#ifdef YDG_ADD_SKILL_GIGANTIC_STORM
				case BITMAP_CHROME_ENERGY2:
		            EnableAlphaBlend();
                    RenderTerrainAlphaBitmap( o->Type, o->Position[0], o->Position[1], o->StartPosition[0], o->StartPosition[1], o->Light );
                    break;
#endif	// YDG_ADD_SKILL_GIGANTIC_STORM

//------------------------------------------------------------------------------------------------------------------------------------
				case BITMAP_TARGET_POSITION_EFFECT1:
					{
						if( o->SubType == 0 )
						{
							RenderTerrainAlphaBitmap( o->Type, o->Position[0], o->Position[1], o->Scale, o->Scale, o->Light );					
						}
					}
					break;
				case BITMAP_TARGET_POSITION_EFFECT2:
					{
						if( o->SubType == 0 )
						{
							RenderTerrainAlphaBitmap( o->Type, o->Position[0], o->Position[1], o->Scale, o->Scale, o->Light );					
						}
					}
					break;
#ifdef KJH_ADD_09SUMMER_EVENT
				case BITMAP_RING_OF_GRADATION:
					{
						if( o->SubType == 0 )
						{
							RenderTerrainAlphaBitmap( o->Type, o->Position[0], o->Position[1], o->Scale, o->Scale, o->Light );					
						}
					}break;
#endif // KJH_ADD_09SUMMER_EVENT
				}
			}
		}
	}
}

#ifdef ASG_ADD_INFLUENCE_GROUND_EFFECT
void CreateMyGensInfluenceGroundEffect()
{
	DeleteEffect(BITMAP_OUR_INFLUENCE_GROUND, &Hero->Object, 0);
	if (::IsStrifeMap(World))
	{
		vec3_t vTemp = {0.f, 0.f, 0.f};
		CreateEffect(BITMAP_OUR_INFLUENCE_GROUND, Hero->Object.Position, vTemp, vTemp, 0, &Hero->Object);
	}
}
#endif	// ASG_ADD_INFLUENCE_GROUND_EFFECT