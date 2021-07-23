// GMEmpireGuardian1.cpp: implementation of the GMEmpireGuardian1 class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#ifdef LDK_ADD_MAP_EMPIREGUARDIAN1

#include "GMEmpireGuardian1.h"
#include "ZzzBMD.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzAI.h"
#include "ZzzEffect.h"
#include "w_MapHeaders.h"
#include "DSPlaySound.h"
#include "ZzzOpenData.h"
#include "ZzzLodTerrain.h"
#include "BoneManager.h"
#include "PhysicsManager.h"

#include "GOBoid.h"

extern char* g_lpszMp3[NUM_MUSIC];

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

GMEmpireGuardian1Ptr GMEmpireGuardian1::Make()
{
	GMEmpireGuardian1Ptr empire(new GMEmpireGuardian1);
	empire->Init();
	return empire;
}

GMEmpireGuardian1::GMEmpireGuardian1()
{
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	m_iWeather = (int)WEATHER_SUN;
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	m_iWeather = WEATHER_TYPE::WEATHER_SUN;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
}

GMEmpireGuardian1::~GMEmpireGuardian1()
{
	Destroy();
}

void GMEmpireGuardian1::Init()
{

}

void GMEmpireGuardian1::Destroy()
{

}

bool GMEmpireGuardian1::CreateObject(OBJECT* o)
{
	switch(o->Type)
	{
	case 129:	// 불기운 박스 청
	case 130:	// 불기운 박스 녹
	case 131:	// 연기박스
	case 132:	// 연기박스
		{
			o->Angle[2] = (float)((int)o->Angle[2]%360);
			VectorCopy(o->Angle,o->HeadAngle);
			VectorCopy(o->Position,o->HeadTargetAngle);
		}
		return true;

	case 115:
	case 117:
		{
			o->SubType = 100;
		}
		return true;
	}

	return false;
}

CHARACTER* GMEmpireGuardian1::CreateMonster(int iType, int PosX, int PosY, int Key)
{
	CHARACTER* pCharacter = NULL;

	switch (iType)
 	{
#ifdef LDS_ADD_EG_3_4_MONSTER_RAYMOND				// 제국 수호군 맵 3,4 (수토,일)몬스터 부사령관 레이몬드			(506/166)
	case 506: // 제국 수호군 맵 3,4 (수토,일)몬스터 부사령관 레이몬드			(506/166)
		{
			OpenMonsterModel(166);
			pCharacter = CreateCharacter(Key, MODEL_MONSTER01+166, PosX, PosY);
			strcpy(pCharacter->ID, "레이몬드");
			
			pCharacter->Object.Scale = 1.45f;

			m_bCurrentIsRage_Raymond = false;
		}
		break;
#endif //LDS_ADD_EG_3_4_MONSTER_RAYMOND
#ifdef LDS_ADD_EG_2_4_MONSTER_ERCANNE				// 제국 수호군 맵 2,4 (화금,일)몬스터 지휘관 에르칸느			(507/167)
	case 507: // 제국 수호군 맵 2,4 (화금,일)몬스터 지휘관 에르칸느			(507/167)
		{
			OpenMonsterModel(167);
			pCharacter = CreateCharacter(Key, MODEL_MONSTER01+167, PosX, PosY);
			strcpy(pCharacter->ID, "에르칸느");
			
			pCharacter->Object.Scale = 1.25f;

			m_bCurrentIsRage_Ercanne = false;
		}
		break;
#endif //LDS_ADD_EG_2_4_MONSTER_ERCANNE
	case 508: // 제국 수호군 1군단장 데슬러
		{
			OpenMonsterModel(168);
			pCharacter = CreateCharacter(Key, MODEL_MONSTER01+168, PosX, PosY);
			strcpy(pCharacter->ID, "데슬러");
			pCharacter->Object.Scale = 1.55f;

			BoneManager::RegisterBone(pCharacter, "node_eyes01", 14);
			BoneManager::RegisterBone(pCharacter, "node_eyes02", 15);
			BoneManager::RegisterBone(pCharacter, "node_blade01" ,71);
			BoneManager::RegisterBone(pCharacter, "node_blade05" ,72);
			BoneManager::RegisterBone(pCharacter, "node_blade04" ,73);
			BoneManager::RegisterBone(pCharacter, "node_blade02" ,74);
//			BoneManager::RegisterBone(pCharacter, "node_blade03" ,75);

			OBJECT* o = &pCharacter->Object;
			BMD* b = &Models[o->Type];

			MoveEye(o, b, 14, 15, 71,72,73,74);
			vec3_t vColor = { 0.7f, 0.7f, 1.0f };
   			CreateJoint(BITMAP_JOINT_ENERGY,o->Position,o->Position,o->Angle,24,o,10.f,-1, 0, 0, -1, vColor);
   			CreateJoint(BITMAP_JOINT_ENERGY,o->Position,o->Position,o->Angle,25,o,10.f,-1, 0, 0, -1, vColor);
			Vector( 0.7f, 0.7f, 1.0f, vColor);
			float Sca = 100.f;
			CreateJoint(BITMAP_JOINT_ENERGY,o->Position,o->Position,o->Angle,54,o,Sca,0, 0, 0, -1, vColor);
			CreateJoint(BITMAP_JOINT_ENERGY,o->Position,o->Position,o->Angle,54,o,Sca,1, 0, 0, -1, vColor);
			CreateJoint(BITMAP_JOINT_ENERGY,o->Position,o->Position,o->Angle,54,o,Sca,2, 0, 0, -1, vColor);
			CreateJoint(BITMAP_JOINT_ENERGY,o->Position,o->Position,o->Angle,54,o,Sca,3, 0, 0, -1, vColor);

			m_bCurrentIsRage_Daesuler = false;
		}
		break;
	case 511: // 제국 수호군 4군단장 갈리아
		{
			OpenMonsterModel(171);
			pCharacter = CreateCharacter(Key, MODEL_MONSTER01+171, PosX, PosY);
			strcpy(pCharacter->ID, "갈리아");
			pCharacter->Object.Scale = 1.35f;

			m_bCurrentIsRage_Gallia = false;
		}
		break;
	case 512: // 제국 수호군 병참장교
		{
			OpenMonsterModel(172);
			pCharacter = CreateCharacter(Key, MODEL_MONSTER01+172, PosX, PosY);
			strcpy(pCharacter->ID, "병참장교");
			pCharacter->Object.Scale = 1.27f;
		}
		break;
	case 513: // 제국 수호군 제국수호군전투교관
		{
			OpenMonsterModel(173);
			pCharacter = CreateCharacter(Key, MODEL_MONSTER01+173, PosX, PosY);
			strcpy(pCharacter->ID, "전투교관");
			pCharacter->Object.Scale = 1.25f;
		}
		break;
#ifdef LDS_ADD_EG_MONSTER_GUARDIANDEFENDER	// 제국 수호군 맵 1234(모든요일)몬스터 수호군 방패병			(518/178)
	case 518: // 제국 수호군 맵 1234(모든요일)몬스터 수호군 방패병			(518/178)
		{
			OpenMonsterModel(178);
			pCharacter = CreateCharacter(Key, MODEL_MONSTER01+178, PosX, PosY);
			strcpy(pCharacter->ID, "수호군방패병");
			pCharacter->Object.Scale = 1.2f;

#ifdef LDS_ADD_EG_ADDMONSTER_ACTION_GUARDIANDEFENDER_EX2
			Vector(0.0f, 0.0f, 0.0f, pCharacter->Object.EyeRight3);
			Vector(0.0f, 0.0f, 0.0f, pCharacter->Object.EyeLeft3);
#endif // LDS_ADD_EG_ADDMONSTER_ACTION_GUARDIANDEFENDER_EX2
		} 
		break;
#endif // LDS_ADD_EG_MONSTER_GUARDIANDEFENDER
#ifdef LDS_ADD_EG_MONSTER_GUARDIANPRIEST
	case 519: // 제국 수호군 맵 1234(모든요일)몬스터 수호군 치유병			(519/179)
		{
			OpenMonsterModel(179);
			pCharacter = CreateCharacter(Key, MODEL_MONSTER01+179, PosX, PosY);
			strcpy(pCharacter->ID, "수호군치유병");	
			pCharacter->Object.Scale = 0.9f;
		}
		break;
#endif //LDS_ADD_EG_MONSTER_GUARDIANPRIEST
	case 520: //제국 수호군 기사단	Imperial Guardian Knights	520 - 181
		{
			OpenMonsterModel(180);
			pCharacter = CreateCharacter(Key, MODEL_MONSTER01+180, PosX, PosY);
			strcpy(pCharacter->ID, "기사단");
			pCharacter->Object.Scale = 1.1f;
		}
		break;
	case 521: //제국 수호군 호위병	Imperial Guardian guard		521 - 182
		{
			OpenMonsterModel(181);
			pCharacter = CreateCharacter(Key, MODEL_MONSTER01+181, PosX, PosY);
			strcpy(pCharacter->ID, "호위병");
			pCharacter->Object.Scale = 1.1f;
		}
		break;
	case 524:
		{
			OpenMonsterModel(183);
			pCharacter = CreateCharacter(Key, MODEL_MONSTER01+183, PosX, PosY);
			strcpy(pCharacter->ID, "창살형 성문");
			pCharacter->Object.m_bRenderShadow = false;
			pCharacter->Object.Scale = 1.25f;
		}
		break;
	case 525:
		{
			OpenMonsterModel(184);
			pCharacter = CreateCharacter(Key, MODEL_MONSTER01+184, PosX, PosY);
			strcpy(pCharacter->ID, "사자머리 성문");
			pCharacter->Object.m_bRenderShadow = false;
			pCharacter->Object.LifeTime = 100;
			pCharacter->Object.Scale = 1.25f;
		}
		break;
	case 526:
		{
			OpenMonsterModel(185);
			pCharacter = CreateCharacter(Key, MODEL_MONSTER01+185, PosX, PosY);
			strcpy(pCharacter->ID, "석상");
			pCharacter->Object.m_bRenderShadow = false;
			pCharacter->Object.Scale = 0.6f;
			pCharacter->Object.LifeTime = 100;
		}
		break;

	default: return pCharacter;
 	}

	return pCharacter;
}

bool GMEmpireGuardian1::MoveObject(OBJECT* o)
{
	if(IsEmpireGuardian1() == false) return false;

	Alpha(o);
	if(o->Alpha < 0.01f) return false;

	BMD *b = &Models[o->Type];
	float fSpeed = o->Velocity;
	switch(o->Type)
	{
	case 20:
		{
			fSpeed *= 2.0f;
		}
		break;

	case 122:
	case 123:
	case 124:
		{
			fSpeed *= 3.0f;
		}
		break;

	case 128:
		{
			fSpeed *= 6.0f;
		}
		break;
	}

	b->PlayAnimation(&o->AnimationFrame,&o->PriorAnimationFrame,&o->PriorAction,fSpeed,o->Position,o->Angle);

	switch(o->Type)
	{
	case 20:	// 좌우로 자르는 칼은 항상 싱크 맞춰줘야 한다.
		{
			static float fAniFrame = 0;
			
			if (fAniFrame - o->AnimationFrame > 10 || fAniFrame < o->AnimationFrame)
				fAniFrame = o->AnimationFrame;
			else
				o->AnimationFrame = fAniFrame;
		}
		return true;
	case 64:	// 칼찍는 석상
		{
			o->Velocity = 0.64f;	// 애니메이션 속도 조절
		}
		return true;
	case 79:	// 리얼빨간불박스
	case 80:	// 주변광박스
	case 82:	// 폭포 효과 - 물 쏟아짐
	case 83:	// 폭포 효과 - 물 튐
	case 84:	// 폭포 효과 - 물 안개 효과
	case 85:	// 법사불 기둥 박스
	case 86:	// 불기운 박스 적
	case 129:	// 불기운 박스 청
	case 130:	// 불기운 박스 녹
	case 131:	// 연기박스
	case 132:	// 연기박스
		//case 91:	// 입김 박스
		{
			o->HiddenMesh = -2;
		}
		return true;
	case 81:	// 폭포수
		{
			o->BlendMeshTexCoordV += 0.015f;	// 흘리기
		}
		return true;
	case 36:	// 바닥문양
		{
			o->Velocity = 0.02f;		// 애니메이션 속도조절
		}
		return true;
	}

	return false;
}

bool GMEmpireGuardian1::MoveMonsterVisual(OBJECT* o, BMD* b)
{
	if(	IsEmpireGuardian1() == false &&
		IsEmpireGuardian2() == false && 
		IsEmpireGuardian3() == false &&
		IsEmpireGuardian4() == false )
	{
		return false;
	}
	
	vec3_t vPos, vRelativePos, Light;
	
	switch(o->Type)
	{
	case MODEL_MONSTER01+184:	// 사자머리 성문
		{
			if(o->CurrentAction == MONSTER01_DIE)
			{
				if(o->LifeTime == 100)
				{
					o->LifeTime = 90;
					
					Vector( 0, 0, 200.0f, vRelativePos);
					b->TransformPosition(o->BoneTransform[0], vRelativePos, vPos, true);
					CreateEffect(MODEL_DOOR_CRUSH_EFFECT, vPos, o->Angle, o->Light,0,o,0,0);
				}
			}
		}
		return true;
	case MODEL_MONSTER01+185: //석상
		{
			if(o->CurrentAction == MONSTER01_DIE)
			{
				if(o->LifeTime == 100)
				{
					o->LifeTime = 90;
					
					Vector( 0, 0, 0.0f, vRelativePos);
					b->TransformPosition(o->BoneTransform[1], vRelativePos, vPos, true);
					CreateEffect(MODEL_STATUE_CRUSH_EFFECT, vPos, o->Angle, o->Light,0,o,0,0);
					
					CreateEffect(MODEL_STATUE_CRUSH_EFFECT_PIECE04, o->Position, o->Angle, o->Light,0);					
					
					vec3_t Angle;
					Vector(0.f,0.f,(float)(rand()%360),Angle);
					
					CreateEffect(MODEL_STONE1+rand()%2, vPos, Angle, o->Light, 0);
					CreateEffect(MODEL_STONE1+rand()%2, vPos, Angle, o->Light, 0);
					CreateEffect(MODEL_STONE1+rand()%2, vPos, Angle, o->Light, 0);
				}
			}
			else
			{
				//눈 
				float Luminosity = sinf( WorldTime*0.003f )*0.2f+0.6f;
				Vector(1.0f*Luminosity , 0.8f*Luminosity, 0.2f*Luminosity, Light);
				
				b->TransformByObjectBone( vPos, o, 3 );
				CreateSprite(BITMAP_LIGHT_RED, vPos, 1.0f, Light, o);
				b->TransformByObjectBone( vPos, o, 4 );
				CreateSprite(BITMAP_LIGHT_RED, vPos, 1.0f, Light, o);
			}
		}
		return true;
	}


	return false;
}

bool GMEmpireGuardian1::MoveMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b)
{
	
	if(	IsEmpireGuardian1() == false &&
		IsEmpireGuardian2() == false && 
		IsEmpireGuardian3() == false &&
		IsEmpireGuardian4() == false )
	{
		return false;
	}

	vec3_t vPos, Light;
	
	switch(o->Type)
	{
#ifdef PBG_ADD_RAYMOND_GUARDIANPRIEST_MONSTER_EFFECT	// 제국 수호군 맵 4 (일)몬스터 부사령관 레이몬드			(506/166)
	case MODEL_MONSTER01+166:
		{
			float _fActSpdTemp = b->Actions[o->CurrentAction].PlaySpeed;
			vec3_t EndPos, EndRelative, Light, vPos;
			Vector ( 1.0f, 1.0f, 1.0f, Light );
			
			switch(o->CurrentAction)
			{
			case MONSTER01_STOP1:
			case MONSTER01_STOP2:
			case MONSTER01_ATTACK1:
			case MONSTER01_APEAR:
				{
					// 광폭화 액션 EFFECT 처리
					{
						vec3_t		Light;
						
						if( m_bCurrentIsRage_Raymond == true )
						{
							Vector ( 1.0f, 1.0f, 1.0f, Light );
							CreateInferno(o->Position);
							
							CreateEffect(MODEL_CIRCLE,o->Position,o->Angle,Light,4,o);
							CreateEffect(MODEL_CIRCLE,o->Position,o->Angle,Light,4,o);	
							m_bCurrentIsRage_Raymond = false;
						}
					}
				}
				break;
			case MONSTER01_DIE:
				break;
			case MONSTER01_WALK:
				{
					Vector (0.9f, 0.2f, 0.1f, Light);				
					if( 7.5f <= o->AnimationFrame && o->AnimationFrame < 8.0f )
					{
						b->TransformByObjectBone(vPos, o, 54);
						CreateParticle (BITMAP_SMOKE+1, vPos, o->Angle, Light,0,0.1f);
					}
					if( 0.0f <= o->AnimationFrame && o->AnimationFrame < 1.0f )
					{
						b->TransformByObjectBone(vPos, o, 59);
						CreateParticle (BITMAP_SMOKE+1, vPos, o->Angle, Light,0,0.1f);
					}
				}
				break;
			case MONSTER01_ATTACK2:
				{
					// 분노의 일격 이펙트
					if(o->AnimationFrame >= 2.5f && o->AnimationFrame < (2.5f + _fActSpdTemp))
					{
						Vector(0.0f, 0.0f,100.0f, EndRelative);
						b->TransformPosition(o->BoneTransform[31],EndRelative,EndPos,true);
						CreateEffect(MODEL_SKILL_FURY_STRIKE, EndPos, o->Angle, Light, 1, o, -1, 0, 0,0);
					}
				}
				break;
			case MONSTER01_ATTACK3:
				{
					// 헬파이어스킬
					if(o->AnimationFrame >= 7.0f && o->AnimationFrame < (7.0f + _fActSpdTemp))
					{
						CreateEffect(MODEL_CIRCLE,o->Position,o->Angle,o->Light,0,0,-1,0,0,0);
						CreateEffect(MODEL_CIRCLE_LIGHT,o->Position,o->Angle,o->Light,0,0,-1,0,0,0);
					}
					// 하늘에서 떨어지는 운석
					Vector(o->Position[0]+rand()%1024-512,o->Position[1]+rand()%1024-512,o->Position[2],EndPos);
					CreateEffect(MODEL_FIRE,EndPos,o->Angle,o->Light);
				}
				break;
			}
		}
		return true;
#endif //PBG_ADD_RAYMOND_GUARDIANPRIEST_MONSTER_EFFECT	// 제국 수호군 맵 4 (일)몬스터 부사령관 레이몬드			(506/166)
#ifdef LDS_ADD_EG_2_4_MONSTER_ERCANNE				// 제국 수호군 맵 2,4 (화금,일)몬스터 지휘관 에르칸느			(507/167)
	case MODEL_MONSTER01+167:		// 에르칸느
		{
			switch( o->CurrentAction )
			{
			case MONSTER01_STOP1:
			case MONSTER01_STOP2:
				break;
			case MONSTER01_WALK:
				break;	
			case MONSTER01_DIE:
				{
// 					// 샘플 이펙트
// 					float Scale = 0.3f;
// 					b->TransformByObjectBone( vPos, o, 30 );
// 					CreateParticle(BITMAP_SMOKE+1, vPos, o->Angle, o->Light, 1, Scale);
// 					
// 					b->TransformByObjectBone( vPos, o, 17 );
// 					CreateParticle(BITMAP_SMOKE+1, vPos, o->Angle, o->Light, 1, Scale);
				}
				break;
			case MONSTER01_ATTACK1:
				{
					if( 2.0f <= o->AnimationFrame && o->AnimationFrame < 15.0f )
					{
						vec3_t  Light;
						//Vector(0.3f, 0.3f, 0.3f, Light);
						Vector(0.3f, 0.8f, 0.4f, Light);
						
						vec3_t StartPos, StartRelative;
						vec3_t EndPos, EndRelative;
						
						float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed;
						float fSpeedPerFrame = fActionSpeed/10.f;
						float fAnimationFrame = o->AnimationFrame - fActionSpeed;
						for(int i=0; i<25; i++) 
						{
							b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);
							
							Vector(0.0f, 0.0f, 0.0f, StartRelative);
							Vector(0.0f, 0.0f, 0.0f, EndRelative);
							
							b->TransformPosition(BoneTransform[39], StartRelative, StartPos, false);
							b->TransformPosition(BoneTransform[40], EndRelative, EndPos, false);
							CreateBlur(c, StartPos, EndPos, Light, 0);
							
							fAnimationFrame += fSpeedPerFrame;
						}
					}
				}
				break;
			case MONSTER01_ATTACK2:	
				{
					// - 스킬 이펙트 정의 부분 - 파이어 스크림 스킬 발동.
					if( 4.0f <= o->AnimationFrame && o->AnimationFrame < 4.4f )
					{
#ifdef LDS_ADD_EFFECT_FIRESCREAM_FOR_MONSTER
						int SkillIndex = 9;		// 고정 설정.
						OBJECT* pObj = o;
						vec3_t ap,P,dp;
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
						float BkO = pObj->Angle[2];
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
						
						VectorCopy(pObj->Position,ap);
						CreateEffect(MODEL_DARK_SCREAM ,pObj->Position,pObj->Angle,pObj->Light,1,pObj,pObj->PKKey,SkillIndex);
						CreateEffect(MODEL_DARK_SCREAM_FIRE ,pObj->Position,pObj->Angle,pObj->Light,1,pObj,pObj->PKKey,SkillIndex);
						
						Vector(80.f,0.f,0.f,P);
						
						pObj->Angle[2] += 10.f;
						
						AngleMatrix(pObj->Angle,pObj->Matrix);
						VectorRotate(P,pObj->Matrix,dp);
						VectorAdd(dp,pObj->Position,pObj->Position);
						CreateEffect(MODEL_DARK_SCREAM ,pObj->Position,pObj->Angle,pObj->Light,1,pObj,pObj->PKKey,SkillIndex);
						CreateEffect(MODEL_DARK_SCREAM_FIRE ,pObj->Position,pObj->Angle,pObj->Light,1,pObj,pObj->PKKey,SkillIndex);
						
						VectorCopy(ap,pObj->Position);
						VectorCopy(pObj->Position,ap);
						Vector(-80.f,0.f,0.f,P);
						pObj->Angle[2] -= 20.f; 
						
						AngleMatrix(pObj->Angle,pObj->Matrix);
						VectorRotate(P,pObj->Matrix,dp);
						VectorAdd(dp,pObj->Position,pObj->Position);
						CreateEffect(MODEL_DARK_SCREAM ,pObj->Position,pObj->Angle,pObj->Light,1,pObj,pObj->PKKey,SkillIndex);
						CreateEffect(MODEL_DARK_SCREAM_FIRE ,pObj->Position,pObj->Angle,pObj->Light,1,pObj,pObj->PKKey,SkillIndex);
						VectorCopy(ap,pObj->Position);	
#endif // LDS_ADD_EFFECT_FIRESCREAM_FOR_MONSTER
					}					

					if( 2.0f <= o->AnimationFrame && o->AnimationFrame < 15.0f )
					{
						vec3_t  Light;
						//Vector(0.3f, 0.3f, 0.3f, Light);
						Vector(0.3f, 0.8f, 0.4f, Light);
						
						vec3_t StartPos, StartRelative;
						vec3_t EndPos, EndRelative;
						
						float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed;
						float fSpeedPerFrame = fActionSpeed/10.f;
						float fAnimationFrame = o->AnimationFrame - fActionSpeed;
						for(int i=0; i<25; i++) 
						{
							b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);
							
							Vector(0.0f, 0.0f, 0.0f, StartRelative);
							Vector(0.0f, 0.0f, 0.0f, EndRelative);
							
							b->TransformPosition(BoneTransform[39], StartRelative, StartPos, false);
							b->TransformPosition(BoneTransform[40], EndRelative, EndPos, false);
							CreateBlur(c, StartPos, EndPos, Light, 0);
							
							fAnimationFrame += fSpeedPerFrame;
						}
					}
				}
				break;
			case MONSTER01_ATTACK3:	
				{
					// - 스킬 이펙트 정의 부분 - 어스 퀘이크 스킬 발동 : 되도록 범위 변동 없도록 유의
					if( o->AnimationFrame >= 3.4f && o->AnimationFrame <= 9.0f )
					{
						RenderSkillEarthQuake ( c, o, b, 12 );
					}
					
					if( 2.0f <= o->AnimationFrame && o->AnimationFrame < 15.0f )
					{
						vec3_t  Light;
						//Vector(0.3f, 0.3f, 0.3f, Light);
						Vector(0.3f, 0.8f, 0.4f, Light);
						
						vec3_t StartPos, StartRelative;
						vec3_t EndPos, EndRelative;
						
						float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed;
						float fSpeedPerFrame = fActionSpeed/10.f;
						float fAnimationFrame = o->AnimationFrame - fActionSpeed;
						for(int i=0; i<25; i++) 
						{
							b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);
							
							Vector(0.0f, 0.0f, 0.0f, StartRelative);
							Vector(0.0f, 0.0f, 0.0f, EndRelative);
							
							b->TransformPosition(BoneTransform[39], StartRelative, StartPos, false);
							b->TransformPosition(BoneTransform[40], EndRelative, EndPos, false);
							CreateBlur(c, StartPos, EndPos, Light, 0);
							
							fAnimationFrame += fSpeedPerFrame;
						}
					}
				}
				break;
			case MONSTER01_APEAR:
				{
					// 광폭화 액션 EFFECT 처리
					{
						vec3_t		Light;
						
						if( m_bCurrentIsRage_Ercanne == true )
						{
							Vector ( 1.0f, 1.0f, 1.0f, Light );
							CreateInferno(o->Position);
							
							CreateEffect(MODEL_CIRCLE,o->Position,o->Angle,Light,4,o);
							CreateEffect(MODEL_CIRCLE,o->Position,o->Angle,Light,4,o);	
							m_bCurrentIsRage_Ercanne = false;
						}
					}
				}
				break;	
			}
		}
		return true;
#endif // LDS_ADD_EG_2_4_MONSTER_ERCANNE				// 제국 수호군 맵 2,4 (화금,일)몬스터 지휘관 에르칸느			(507/167)
	case MODEL_MONSTER01+168:	// 데슬러
		{
			switch( o->CurrentAction )
			{
			case MONSTER01_STOP1:
			case MONSTER01_STOP2:
				break;
			case MONSTER01_WALK:
				break;
			case MONSTER01_DIE:
				{

				}
				break;
			case MONSTER01_ATTACK1:
				{
					
				}
				break;
			case MONSTER01_ATTACK2:	
				{
					float _fActSpdTemp = b->Actions[o->CurrentAction].PlaySpeed;
					if(o->AnimationFrame >= 4.5f && o->AnimationFrame < (4.5f + _fActSpdTemp))
					{
						CHARACTER *tc = &CharactersClient[c->TargetCharacter];
						OBJECT *to = &tc->Object;
						
						vec3_t Angle, AngleRelative = { 0.0f, 0.0f, 0.0f };
						vec3_t v3PosTo = { to->Position[0], to->Position[1], to->Position[2] };
						vec3_t v3RelativePos, v3StartPos;
						
						Vector ( o->Angle[0] + AngleRelative[0], 
							o->Angle[1] + AngleRelative[1], 
							o->Angle[2] + AngleRelative[2], 
							Angle );

						Vector(0.0f, 0.0f, 0.0f, v3RelativePos);
						b->TransformPosition(o->BoneTransform[20],v3RelativePos,v3StartPos,true);
						CreateEffect(MODEL_DEASULER, v3StartPos, Angle, v3PosTo, 0, o, -1, 0, 0,0,1.8f);
					}
				}
				break;	
			case MONSTER01_ATTACK3:	
				{
					vec3_t Angle,p,Position;
					float Matrix[3][4];
					Vector(0.f,-500.f,0.f,p);
					for(int j=0;j<3;j++)
					{
						Vector((float)(rand()%90),0.f,(float)(rand()%360),Angle);
						AngleMatrix(Angle,Matrix);
						VectorRotate(p,Matrix,Position);
						VectorSubtract(o->Position,Position,Position);
						Position[2] += 120.f;
						CreateJoint(BITMAP_JOINT_HEALING,Position,o->Position,Angle,17,o,10.f);
					}
					vec3_t Light;
					Vector ( 1.f, 0.1f, 0.f, Light );
					Vector ( 0.f, 0.f, 0.f, p );
					for( int i=0;i<10;i++)
					{
						b->TransformPosition(o->BoneTransform[rand()%62],p,Position,true);
						CreateParticle(BITMAP_LIGHT,Position,o->Angle,Light, 5, 0.5f+(rand()%100)/50.f);
						
						b->TransformPosition(o->BoneTransform[50],p,Position,true);
						CreateParticle(BITMAP_LIGHT,Position,o->Angle,Light, 5, 0.5f+(rand()%100)/50.f);
					}
					
					
					
					float _fActSpdTemp = b->Actions[o->CurrentAction].PlaySpeed;
					// 분노의 일격 이펙트
					if(o->AnimationFrame >= 4.5f && o->AnimationFrame < (4.5f + _fActSpdTemp))
						
					{
						vec3_t EndPos, EndRelative;
						
						Vector(0.0f, 0.0f, 0.0f, EndRelative);
						b->TransformPosition(o->BoneTransform[11],EndRelative,EndPos,true);
						CreateEffect(MODEL_SKILL_FURY_STRIKE, EndPos, o->Angle, Light, 1, o, -1, 0, 0);
					}
					
				}
				break;
			case MONSTER01_APEAR:
				{
					// 광폭화 액션 EFFECT 처리
					{
						vec3_t		Light;
						
						if( m_bCurrentIsRage_Daesuler == true )
						{
							Vector ( 1.0f, 1.0f, 1.0f, Light );
							CreateInferno(o->Position);
							
							CreateEffect(MODEL_CIRCLE,o->Position,o->Angle,Light,4,o);
							CreateEffect(MODEL_CIRCLE,o->Position,o->Angle,Light,4,o);	
							m_bCurrentIsRage_Daesuler = false;
						}
					}
				}
				break;	
		}
	}
	break;
	case MODEL_MONSTER01+171:	// 갈리아
		{
			//wing node 1,2,3
			Vector(1.0f, 0.8f, 0.2f, Light);
			b->TransformByObjectBone( vPos, o, 81 );
			CreateSprite(BITMAP_LIGHT_RED, vPos, 0.5f, Light, o);
			b->TransformByObjectBone( vPos, o, 82 );
			CreateSprite(BITMAP_LIGHT_RED, vPos, 1.3f, Light, o);
			b->TransformByObjectBone( vPos, o, 83 );
			CreateSprite(BITMAP_LIGHT_RED, vPos, 2.0f, Light, o);
			
			//wing node 4,5,6
			Vector(1.0f, 0.8f, 0.2f, Light);
			b->TransformByObjectBone( vPos, o, 105 );
			CreateSprite(BITMAP_LIGHT_RED, vPos, 0.5f, Light, o);
			b->TransformByObjectBone( vPos, o, 104 );
			CreateSprite(BITMAP_LIGHT_RED, vPos, 1.3f, Light, o);
			b->TransformByObjectBone( vPos, o, 103 );
			CreateSprite(BITMAP_LIGHT_RED, vPos, 2.0f, Light, o);

			//head
			Vector(1.0f, 0.8f, 0.2f, Light);
			b->TransformByObjectBone( vPos, o, 10 );
			CreateSprite(BITMAP_LIGHT_RED, vPos, 1.5f, Light, o);

			//neak
			Vector(1.0f, 0.8f, 0.2f, Light);
			b->TransformByObjectBone( vPos, o, 9 );
			CreateSprite(BITMAP_LIGHT_RED, vPos, 2.0f, Light, o);

			//spine
			Vector(1.0f, 0.8f, 0.2f, Light);
			b->TransformByObjectBone( vPos, o, 6 );
			CreateSprite(BITMAP_LIGHT_RED, vPos, 4.0f, Light, o);

			//L arm
			Vector(1.0f, 0.8f, 0.2f, Light);
			b->TransformByObjectBone( vPos, o, 13 );
			CreateSprite(BITMAP_LIGHT_RED, vPos, 2.0f, Light, o);
			b->TransformByObjectBone( vPos, o, 30 );
			CreateSprite(BITMAP_LIGHT_RED, vPos, 2.0f, Light, o);

			//R arm
			Vector(1.0f, 0.8f, 0.2f, Light);
			b->TransformByObjectBone( vPos, o, 32 );
			CreateSprite(BITMAP_LIGHT_RED, vPos, 2.0f, Light, o);
			b->TransformByObjectBone( vPos, o, 75 );
			CreateSprite(BITMAP_LIGHT_RED, vPos, 2.0f, Light, o);

			//sword
			float Luminosity = sinf( WorldTime*0.003f )*0.2f+0.6f;
			Vector(1.0f*Luminosity , 0.5f*Luminosity, 0.4f*Luminosity, Light);

			int temp [] = { 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 
							59, 60, 61, 62, 63, 64, 65, 66, 67, 71, 72 };
 			
			for(int i=0; i<21; i++)
			{
				b->TransformByObjectBone( vPos, o, temp[i] );
				CreateSprite(BITMAP_LIGHTMARKS, vPos, 0.6f, Light, o);
			}
			b->TransformByObjectBone( vPos, o, 68 );
			CreateSprite(BITMAP_LIGHTMARKS, vPos, 0.3f, Light, o);
			b->TransformByObjectBone( vPos, o, 69 );
			CreateSprite(BITMAP_LIGHTMARKS, vPos, 0.2f, Light, o);
			b->TransformByObjectBone( vPos, o, 70 );
			CreateSprite(BITMAP_LIGHTMARKS, vPos, 0.3f, Light, o);


			// Action 정의
			float _fActSpdTemp = b->Actions[o->CurrentAction].PlaySpeed;

			switch( o->CurrentAction )
			{
			case MONSTER01_STOP1:
			case MONSTER01_STOP2:
				break;
			case MONSTER01_WALK:
				break;
			case MONSTER01_DIE:
				{

				}
				break;
			case MONSTER01_ATTACK1:
				{
					
				}
				break;
			case MONSTER01_ATTACK2:	
				{
					// 분노의 일격 이펙트
					if(o->AnimationFrame >= 2.5f && o->AnimationFrame < (2.5f + _fActSpdTemp))
					{
						vec3_t EndPos, EndRelative;

						Vector(0.0f, 0.0f, 100.0f, EndRelative);
						b->TransformPosition(o->BoneTransform[11],EndRelative,EndPos,true);
						CreateEffect(MODEL_SKILL_FURY_STRIKE, EndPos, o->Angle, Light, 1, o, -1, 0, 0);
					}
				}
				break;	
			case MONSTER01_ATTACK3:	
				{
					// - 스킬 이펙트 정의 부분 - 어스 퀘이크 스킬 발동 : 되도록 범위 변동 없도록 유의
					if( o->AnimationFrame >= 3.4f && o->AnimationFrame <= 9.0f )
					{
						RenderSkillEarthQuake ( c, o, b, 12 );
					}
				}
				break;
			case MONSTER01_APEAR:
				{
					// 광폭화 액션 EFFECT 처리
					{
						vec3_t		Light;
						
						if( m_bCurrentIsRage_Gallia == true )
						{
							Vector ( 1.0f, 1.0f, 1.0f, Light );
							CreateInferno(o->Position);
							
							CreateEffect(MODEL_CIRCLE,o->Position,o->Angle,Light,4,o);
							CreateEffect(MODEL_CIRCLE,o->Position,o->Angle,Light,4,o);	
							m_bCurrentIsRage_Gallia = false;
						}
					}
				}
				break;	
			}
			
		}
		return true;

	case MODEL_MONSTER01+173:	// 제국 수호군 전투교관
		{
			switch( o->CurrentAction )
			{
			case MONSTER01_WALK:
				{
					Vector(0.5f, 0.2f, 0.1f, o->Light);
					
					if( 7.0f <= o->AnimationFrame && o->AnimationFrame < 8.0f )
					{
						b->TransformByObjectBone( vPos, o, 53 );
						vPos[2] += 25.0f;
						CreateParticle ( BITMAP_SMOKE+1, vPos, o->Angle, o->Light, 1 );
					}
					if( 1.0f <= o->AnimationFrame && o->AnimationFrame < 2.0f )
					{
						b->TransformByObjectBone( vPos, o, 48 );
						vPos[2] += 25.0f;
						CreateParticle ( BITMAP_SMOKE+1, vPos, o->Angle, o->Light, 1 );
					}
				}
				break;
			case MONSTER01_ATTACK1:
				{

				}
				break;
			case MONSTER01_ATTACK2:
				{
					
				}
				break;
			case MONSTER01_APEAR:
				{
					if( o->AnimationFrame >= 5.0f && o->AnimationFrame <= 5.8f )
					{
						CreateEffect ( MODEL_WAVES, o->Position, o->Angle, o->Light, 1 );
						CreateEffect ( MODEL_WAVES, o->Position, o->Angle, o->Light, 1 );
						CreateEffect ( MODEL_PIERCING2, o->Position, o->Angle, o->Light );
						PlayBuffer ( SOUND_ATTACK_SPEAR );	
					}
				}
				break;
			}
		}
		return true;
#ifdef KJH_ADD_EG_MONSTER_GUARDIANDEFENDER_EFFECT

#ifdef LDS_ADD_EG_ADDMONSTER_ACTION_GUARDIANDEFENDER
	case MODEL_MONSTER01+178:	// 수호군 방패병
		{
			// HeadTargetAngle
			switch( o->CurrentAction )	
			{
			case MONSTER01_ATTACK1:
				{
					
				}
				break;
			case MONSTER01_ATTACK2:
				{
					if( 4.0f <= o->AnimationFrame && o->AnimationFrame < 5.0f )
					{
						// 캐릭터가 바라보는 방향의 위치값 계산하기
						vec3_t vLook, vPosition, vLight;
						float matRotate[3][4];
						Vector(0.0f, -100.0f, 100.0f, vPosition );
						AngleMatrix ( o->Angle, matRotate );
						VectorRotate( vPosition, matRotate, vLook );
						VectorAdd( o->Position, vLook, vPosition );
						
						Vector(0.8f, 0.8f, 1.0f, vLight);
						
						CreateEffect(MODEL_EFFECT_EG_GUARDIANDEFENDER_ATTACK2, vPosition, o->Angle, vLight, 0, o);
					}
				}
				break;
			case MONSTER01_APEAR:
				{
#ifdef LDS_ADD_EG_ADDMONSTER_ACTION_GUARDIANDEFENDER_EX2
					VectorCopy( o->EyeRight3, o->Angle );
#else // LDS_ADD_EG_ADDMONSTER_ACTION_GUARDIANDEFENDER_EX2
					VectorCopy( o->HeadTargetAngle, o->Angle );
#endif // LDS_ADD_EG_ADDMONSTER_ACTION_GUARDIANDEFENDER_EX2
				}
				break;	
			};
			
#ifdef LDS_ADD_EG_ADDMONSTER_ACTION_GUARDIANDEFENDER_EX2
			VectorCopy( o->Angle, o->EyeRight3 );		//  오브젝트가 실 방향 백업. 
#else // LDS_ADD_EG_ADDMONSTER_ACTION_GUARDIANDEFENDER_EX2
			VectorCopy( o->Angle, o->HeadTargetAngle );		//  오브젝트가 실 방향 백업. 
#endif // LDS_ADD_EG_ADDMONSTER_ACTION_GUARDIANDEFENDER_EX2
		}
		return true;

#else // LDS_ADD_EG_ADDMONSTER_ACTION_GUARDIANDEFENDER
	case MODEL_MONSTER01+178:	// 수호군 방패병
		{
			if( o->CurrentAction == MONSTER01_ATTACK2 )
			{
				if( 4.0f <= o->AnimationFrame && o->AnimationFrame < 5.0f )
				{
					// 캐릭터가 바라보는 방향의 위치값 계산하기
					vec3_t vLook, vPosition, vLight;
					float matRotate[3][4];
					Vector(0.0f, -100.0f, 100.0f, vPosition );
					AngleMatrix ( o->Angle, matRotate );
					VectorRotate( vPosition, matRotate, vLook );
					VectorAdd( o->Position, vLook, vPosition );

					Vector(0.8f, 0.8f, 1.0f, vLight);
					
					CreateEffect(MODEL_EFFECT_EG_GUARDIANDEFENDER_ATTACK2, vPosition, o->Angle, vLight, 0, o);
				}
			}
		}
		break;
#endif // LDS_ADD_EG_ADDMONSTER_ACTION_GUARDIANDEFENDER

#endif // KJH_ADD_EG_MONSTER_GUARDIANDEFENDER_EFFECT
#ifdef PBG_ADD_RAYMOND_GUARDIANPRIEST_MONSTER_EFFECT
	case MODEL_MONSTER01+179:	// 치유병
		{
			vec3_t vRelative, vLight, vPos;
			switch(o->CurrentAction)
			{
			case MONSTER01_STOP1:
			case MONSTER01_STOP2:
			case MONSTER01_WALK:
				{
					Vector(10.0f, 0.0f, 0.0f, vRelative);
					b->TransformPosition(o->BoneTransform[55], vRelative, vPos, true);
					float fLumi = (sinf(WorldTime*0.001f) + 1.0f) * 0.3f + 0.4f;
					Vector(fLumi*0.5f, fLumi*0.5f, fLumi*0.5f, vLight);
					CreateSprite(BITMAP_FLARE_BLUE, vPos, 1.5f, vLight, o);
					Vector(0.5f, 0.5f, 0.5f, vLight);
					
					CreateSprite(BITMAP_FLARE_BLUE, vPos, 1.2f, vLight, o, -WorldTime*0.1f);
				}
				break;
			case MONSTER01_DIE:
				break;
			case MONSTER01_ATTACK1:
				{
					Vector(10.0f, 0.0f, 0.0f, vRelative);
					b->TransformPosition(o->BoneTransform[55], vRelative, vPos, true);
					float fLumi = (sinf(WorldTime*0.001f) + 1.0f) * 0.3f + 0.4f;
					float fScale = (sinf(WorldTime*0.001f)+1.0f)*0.5f+1.0f;
					Vector(fLumi*0.5f, fLumi*0.5f, fLumi*0.5f, vLight);
					CreateSprite(BITMAP_FLARE_RED, vPos, 1.5f, vLight, o);
					Vector(0.5f, 0.5f, 0.5f, vLight);
					CreateSprite(BITMAP_FLARE_RED, vPos, 1.2f*fScale*o->AnimationFrame*0.15f, vLight, o, -WorldTime*0.1f);
					
					CHARACTER *tc = &CharactersClient[c->TargetCharacter];
					OBJECT *to = &tc->Object;
					//번개 이펙트
					if(o->AnimationFrame > 10)
					{
						vec3_t p,Angle;
						Vector(0.0f,0.0f,0.0f,p);
						Vector(0.88f, 0.12f, 0.08f, vLight);
						b->TransformPosition(o->BoneTransform[55],p,vPos,true);
						Vector(-60.0f,0.0f,o->Angle[2],Angle);
						Vector(-60.0f,0.0f,o->Angle[2],Angle);
						CreateJoint(BITMAP_JOINT_THUNDER,vPos,to->Position,Angle,27,to,50.0f,-1,0,0,0,vLight);
						CreateJoint(BITMAP_JOINT_THUNDER,vPos,to->Position,Angle,27,to,10.0f,-1,0,0,0,vLight);
						CreateParticle(BITMAP_ENERGY,vPos,o->Angle,vLight);
					}
				}
				break;
			case MONSTER01_ATTACK2:
				{
					Vector(10.0f, 0.0f, 0.0f, vRelative);
					b->TransformPosition(o->BoneTransform[55], vRelative, vPos, true);
					float fLumi = (sinf(WorldTime*0.001f) + 1.0f) * 0.3f + 0.4f;
					float fScale = (sinf(WorldTime*0.001f)+1.0f)*0.5f+1.0f;
					Vector(fLumi*0.5f, fLumi*0.5f, fLumi*0.5f, vLight);
					CreateSprite(BITMAP_FLARE_BLUE, vPos, 1.5f, vLight, o);
					Vector(0.5f, 0.5f, 0.5f, vLight);
					//CreateSprite(BITMAP_FLARE_BLUE, vPos, 1.2f, vLight, o, -WorldTime*0.1f);
					CreateSprite(BITMAP_FLARE_BLUE, vPos, 1.2f*fScale*o->AnimationFrame*0.15f, vLight, o, -WorldTime*0.1f);
				
					CHARACTER *tc = &CharactersClient[c->TargetCharacter];
					OBJECT *to = &tc->Object;

					// 치료 이펙트
					if(o->AnimationFrame > 4.5f && o->AnimationFrame <= 4.8f)
						CreateEffect(BITMAP_MAGIC+1,to->Position,to->Angle,to->Light,1,to);
				}
				break;
			}
		}
		return true;
#endif //PBG_ADD_RAYMOND_GUARDIANPRIEST_MONSTER_EFFECT
	} //switch end

	return false;
}

void GMEmpireGuardian1::MoveBlurEffect(CHARACTER* c, OBJECT* o, BMD* b)
{
	vec3_t  Light;
	vec3_t StartPos, StartRelative;
	vec3_t EndPos, EndRelative;
	Vector(1.0f, 1.0f, 1.0f, Light);
	Vector(0.0f, 0.0f, 0.0f, StartRelative);
	Vector(0.0f, 0.0f, 0.0f, EndRelative);

	float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed;
	float fSpeedPerFrame = fActionSpeed/10.f;
	float fAnimationFrame = o->AnimationFrame - fActionSpeed;
	
	switch(o->Type)
	{
#ifdef PBG_ADD_RAYMOND_GUARDIANPRIEST_MONSTER_EFFECT
	case MODEL_MONSTER01+166:		//레이몬드
		{
			float Start_Frame = 0.0f;
			float End_Frame = 0.0f;
			
			switch(o->CurrentAction)
			{
			case MONSTER01_STOP1:
			case MONSTER01_STOP2:
			case MONSTER01_WALK:
			case MONSTER01_DIE:
				break;
			case MONSTER01_ATTACK1:
				{
					Start_Frame = 4.0f;
					End_Frame = 11.0f;
				}
				break;
			case MONSTER01_ATTACK2:
			case MONSTER01_ATTACK3:
				{
					Start_Frame = 5.0f;
					End_Frame = 10.0f;
				}
				break;
			}
			
			if((o->AnimationFrame >= Start_Frame && o->AnimationFrame <= End_Frame )
				&& ((o->CurrentAction == MONSTER01_ATTACK1) || (o->CurrentAction == MONSTER01_ATTACK2) || (o->CurrentAction == MONSTER01_ATTACK3))
				)
			{
				BMD *b = &Models[o->Type];
				vec3_t  Light;
				
				vec3_t StartPos, StartRelative;
				vec3_t EndPos, EndRelative;
				
				float fDelay =5.0f;
				
				float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed;
				float fSpeedPerFrame = fActionSpeed/fDelay;
				float fAnimationFrame = o->AnimationFrame - fActionSpeed;
				for(int i=0; i<fDelay; i++) 
				{
					b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);
					
					Vector(0.f, 0.f, 0.f, StartRelative);
					Vector(0.f, 0.f, 0.f, EndRelative);
					
					Vector(0.9f,0.2f,0.1f, Light);
					b->TransformPosition(BoneTransform[32], StartRelative, StartPos, false);
					b->TransformPosition(BoneTransform[33], EndRelative, EndPos, false);
					CreateBlur(c, StartPos, EndPos, Light, 0);
					
					fAnimationFrame += fSpeedPerFrame;
				}
			}
		}
		break;
#endif //PBG_ADD_RAYMOND_GUARDIANPRIEST_MONSTER_EFFECT
	
	case MODEL_MONSTER01+171:	// 갈리아
		{
			switch(o->CurrentAction)
			{
			case MONSTER01_ATTACK1:
				{
					if( 4.0f <= o->AnimationFrame && o->AnimationFrame < 6.0f )
					{
						Vector(1.0f, 0.2f, 0.0f, Light);
						Vector(0.0f, 0.0f, 0.0f, StartRelative);
						Vector(0.0f, 0.0f, 0.0f, EndRelative);
						
						for(int i=0; i<28; i++) 
						{
							b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);
							
							b->TransformPosition(BoneTransform[69], StartRelative, StartPos, false);
							b->TransformPosition(BoneTransform[49], EndRelative, EndPos, false);
							CreateBlur(c, StartPos, EndPos, Light, 7);
							
							fAnimationFrame += fSpeedPerFrame;
						}
					}
				}
				break;
			case MONSTER01_ATTACK2:
				{
					if( 4.0f <= o->AnimationFrame && o->AnimationFrame < 7.5f )
					{
						Vector(1.0f, 1.0f, 1.0f, Light);
						Vector(0.0f, 0.0f, 0.0f, StartRelative);
						Vector(0.0f, 0.0f, 0.0f, EndRelative);

						for(int i=0; i<28; i++) 
						{
							b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);
							
							b->TransformPosition(BoneTransform[69], StartRelative, StartPos, false);
							b->TransformPosition(BoneTransform[49], EndRelative, EndPos, false);
							CreateBlur(c, StartPos, EndPos, Light, 2);
							
							fAnimationFrame += fSpeedPerFrame;
						}
					}
				}
				break;
			case MONSTER01_ATTACK3:
				{
					if( 4.5f <= o->AnimationFrame && o->AnimationFrame < 7.8f )
					{
						Vector(1.0f, 0.4f, 0.0f, Light);
						Vector(0.0f, 0.0f, 0.0f, StartRelative);
						Vector(0.0f, 0.0f, 0.0f, EndRelative);

						for(int i=0; i<28; i++) 
						{
							b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);
							
							b->TransformPosition(BoneTransform[69], StartRelative, StartPos, false);
							b->TransformPosition(BoneTransform[49], EndRelative, EndPos, false);
							CreateBlur(c, StartPos, EndPos, Light, 8);
							
							fAnimationFrame += fSpeedPerFrame;
						}
					}
				}
				break;
			} //attack switch end
		}
		break;

	case MODEL_MONSTER01+172:	// 병참장교
		{
			if (o->CurrentAction == MONSTER01_ATTACK1 || o->CurrentAction == MONSTER01_ATTACK2)
			{
				vec3_t  vLight;
				Vector(0.5f, 0.5f, 0.7f, vLight);
				
				vec3_t StartPos, StartRelative;
				vec3_t EndPos, EndRelative;
				
				float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed;
				float fSpeedPerFrame = fActionSpeed/10.f;
				float fAnimationFrame = o->AnimationFrame - fActionSpeed;
				for(int i=0; i<10; i++) 
				{
					b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);
					
					Vector(0.f, 0.f, 0.f, StartRelative);
					Vector(0.f, 0.f, 0.f, EndRelative);
					b->TransformPosition(BoneTransform[33], StartRelative, StartPos, false);
					b->TransformPosition(BoneTransform[34], EndRelative, EndPos, false);
					CreateBlur(c, StartPos, EndPos, vLight, 0, false, 0);
					
					fAnimationFrame += fSpeedPerFrame;
				}
			}
		}
		break;

	case MODEL_MONSTER01+173: // 전투교관
		{
			switch(o->CurrentAction)
			{
			case MONSTER01_ATTACK1:
				{
					if( 4.0f <= o->AnimationFrame && o->AnimationFrame < 11.7f )
					{
						Vector(1.0f, 0.2f, 0.0f, Light);
						Vector(0.0f, 0.0f, 0.0f, StartRelative);
						Vector(0.0f, 0.0f, 0.0f, EndRelative);
						
						for(int i=0; i<18; i++) 
						{
							b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);
							
							b->TransformPosition(BoneTransform[36], StartRelative, StartPos, false);
							b->TransformPosition(BoneTransform[37], EndRelative, EndPos, false);
							CreateBlur(c, StartPos, EndPos, Light, 7);
							
							fAnimationFrame += fSpeedPerFrame;
						}
					}
				}
				break;
			case MONSTER01_ATTACK2:
				{
					if( 4.0f <= o->AnimationFrame && o->AnimationFrame < 6.840f )
					{
						Vector(1.0f, 1.0f, 1.0f, Light);
						Vector(0.0f, 0.0f, 0.0f, StartRelative);
						Vector(0.0f, 0.0f, 0.0f, EndRelative);
						
						for(int i=0; i<16; i++) 
						{
							b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);
							
							b->TransformPosition(BoneTransform[36], StartRelative, StartPos, false);
							b->TransformPosition(BoneTransform[37], EndRelative, EndPos, false);
							CreateBlur(c, StartPos, EndPos, Light, 1);
							
							fAnimationFrame += fSpeedPerFrame;
						}
					}
				}
				break;
			case MONSTER01_APEAR:	// 어택 3번 입니다.
				{
					if( 4.0f <= o->AnimationFrame && o->AnimationFrame < 6.0f )
					{
						Vector(1.0f, 0.2f, 0.0f, Light);
						Vector(0.0f, 0.0f, 0.0f, StartRelative);
						Vector(0.0f, 0.0f, 0.0f, EndRelative);
						
						for(int i=0; i<28; i++) 
						{
							b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);
							
							b->TransformPosition(BoneTransform[36], StartRelative, StartPos, false);
							b->TransformPosition(BoneTransform[37], EndRelative, EndPos, false);
							CreateBlur(c, StartPos, EndPos, Light, 7);
							
							fAnimationFrame += fSpeedPerFrame;
						}
					}
				}
				break;
			} // attack switch end
		}
		break;
#ifdef LDK_ADD_EG_MONSTER_KNIGHTS
	case MODEL_MONSTER01+180:	// 기사단
		{
			if (o->CurrentAction == MONSTER01_ATTACK1 || o->CurrentAction == MONSTER01_ATTACK2)
			{
				vec3_t  vLight;
				Vector(0.5f, 0.5f, 0.5f, vLight);
				
				vec3_t StartPos, StartRelative;
				vec3_t EndPos, EndRelative;
				
				float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed;
				float fSpeedPerFrame = fActionSpeed/10.f;
				float fAnimationFrame = o->AnimationFrame - fActionSpeed;
				for(int i=0; i<10; i++) 
				{
					b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);
					
					Vector(0.f, 0.f, 0.f, StartRelative);
					Vector(0.f, 0.f, 0.f, EndRelative);
					b->TransformPosition(BoneTransform[28], StartRelative, StartPos, false);
					b->TransformPosition(BoneTransform[29], EndRelative, EndPos, false);
					CreateBlur(c, StartPos, EndPos, vLight, 0, false, 0);
					
					fAnimationFrame += fSpeedPerFrame;
				}
			}
		}
		break;
#endif	// LDK_ADD_EG_MONSTER_KNIGHTS
#ifdef ASG_ADD_EG_MONSTER_GUARD_EFFECT
	case MODEL_MONSTER01+181:	// 호위병 (검기 이펙트)
		{
			float Start_Frame = 0.f;
			float End_Frame = 10.f;
			if ((o->AnimationFrame >= Start_Frame && o->AnimationFrame <= End_Frame && o->CurrentAction == MONSTER01_ATTACK1) 
				|| (o->AnimationFrame >= Start_Frame && o->AnimationFrame <= End_Frame && o->CurrentAction == MONSTER01_ATTACK2))
			{
				vec3_t  Light;
				Vector(0.5f, 0.5f, 0.5f, Light);
				
				vec3_t StartPos, StartRelative;
				vec3_t EndPos, EndRelative;
				
				float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed;
				float fSpeedPerFrame = fActionSpeed/20.f;
				float fAnimationFrame = o->AnimationFrame - fActionSpeed;
				for(int i=0; i<20; i++) 
				{
					b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);
					
					Vector(0.f, 0.f, 0.f, StartRelative);
					Vector(0.f, 0.f, 0.f, EndRelative);
					
					b->TransformPosition(BoneTransform[37], StartRelative, StartPos, false);
					b->TransformPosition(BoneTransform[38], EndRelative, EndPos, false);
					CreateBlur(c, StartPos, EndPos, Light, 0);
					
					fAnimationFrame += fSpeedPerFrame;
				}
			}
		}
		break;
#endif	// ASG_ADD_EG_MONSTER_GUARD_EFFECT
	} //switch end
}

bool GMEmpireGuardian1::RenderObjectMesh(OBJECT* o, BMD* b, bool ExtraMon)
{
	if(	IsEmpireGuardian1() == false &&
		IsEmpireGuardian2() == false && 
		IsEmpireGuardian3() == false &&
		IsEmpireGuardian4() == false )
	{
		return false;
	}

	switch(o->Type)
	{
	case 0:
	case 1:
	case 3:
	case 44:// 돌기둥들
	case 81:// 폭포수
		o->m_bRenderAfterCharacter = true;
		return true;
		
	case 37:// 벽등
		{
			b->RenderMesh ( 0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );
			b->RenderMesh ( 1, RENDER_TEXTURE|RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );
		}
		return true;

	case 41:// sos_bobpu
		{
			b->RenderMesh ( 0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );
			b->RenderMesh ( 2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );	
			float Luminosity = sinf(WorldTime*0.002f)*0.5f+0.6f;
			Vector( Luminosity*b->BodyLight[0], Luminosity*b->BodyLight[1], Luminosity*b->BodyLight[2], b->BodyLight );
			b->RenderMesh ( 1, RENDER_TEXTURE|RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );
		}
		return true;
		
	case 47:// Gateflag
		{
			b->RenderMesh ( 0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );
			b->RenderMesh ( 1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );
			b->RenderMesh ( 2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );
			Vector(1.2f, 1.2f, 1.2f, b->BodyLight);
			b->RenderMesh ( 3, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );
		}
		return true;
		
	case 48:// Gateflag2
		{
			b->RenderMesh ( 1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );
			b->RenderMesh ( 2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );
			Vector(1.1f, 1.1f, 1.1f, b->BodyLight);
			b->RenderMesh ( 0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );	
		}
		return true;

	case 49:// 대문
		{
			b->RenderMesh ( 1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );
			b->RenderMesh ( 2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );
			b->RenderMesh ( 0, RENDER_TEXTURE|RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );
		}
		return true;
		
	case 55:
		{
			b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
		}
		return true;

	case 64:// 칼찍는 석상
		{
			b->RenderMesh ( 0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );
			b->RenderMesh ( 3, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );
			b->RenderMesh ( 2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );
			Vector(1.0f, 0.0f, 0.0f, b->BodyLight);
			b->RenderMesh ( 1, RENDER_BRIGHT|RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );
			b->RenderMesh ( 1, RENDER_BRIGHT|RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );
		}
		return true;
		
	case 70:// 소형대문
		{
			b->RenderMesh ( 1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );
			b->RenderMesh ( 2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );
			b->RenderMesh ( 0, RENDER_TEXTURE|RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );
		}
		return true;
		
	case 115: //투석기 1
		{
			b->RenderMesh ( 0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );
			b->RenderMesh ( 2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );
			
			o->BlendMeshTexCoordV = (int)WorldTime%25000 * 0.0004f;
			b->StreamMesh = 1;
			b->RenderMesh ( 1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );
			
			o->BlendMeshTexCoordV = (int)WorldTime%25000 * -0.0004f;
			b->StreamMesh = 3;
			b->RenderMesh ( 3, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );
			b->StreamMesh = -1;
			
			vec3_t vRelativePos, vWorldPos, Light, Angle;
			Vector(0, 0, 0, vRelativePos);
			Vector(1.f,1.f,1.f,Light);
			Vector(0.f,0.f,0.f,Angle);
			b->TransformPosition(BoneTransform[9],vRelativePos,vWorldPos,false);
			
			if (0.0f < o->AnimationFrame && o->AnimationFrame < 0.4f)
			{
				if(o->SubType == 100)
				{
					CreateEffect(MODEL_PROJECTILE, vWorldPos, Angle, Light, 0, o);
					o->SubType = 101;
				}
			}
			else if(7.0f < o->AnimationFrame && o->AnimationFrame < 7.5f)
			{
				if( o->SubType == 101 )
				{
					o->SubType = 100;
				}
			}
		}
		return true;
		
	case 117: //투석기 2
		{
			b->RenderMesh ( 0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );
			b->RenderMesh ( 2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );
			b->RenderMesh ( 3, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );
			
			o->BlendMeshTexCoordV = (int)WorldTime%25000 * 0.0004f;
			b->StreamMesh = 1;
			b->RenderMesh ( 1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );
			
			o->BlendMeshTexCoordV = (int)WorldTime%25000 * -0.0004f;
			b->StreamMesh = 4;
			b->RenderMesh ( 4, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );
			b->StreamMesh = -1;
			
			vec3_t vRelativePos, vWorldPos, Light, Angle;
			Vector(0, 0, 0, vRelativePos);
			Vector(1.f,1.f,1.f,Light);
			Vector(0.f,0.f,0.f,Angle);
			b->TransformPosition(BoneTransform[9],vRelativePos,vWorldPos,false);
			
			if (0.0f < o->AnimationFrame && o->AnimationFrame < 0.4f)
			{
				if(o->SubType == 100)
				{
					CreateEffect(MODEL_PROJECTILE, vWorldPos, Angle, Light, 0, o);
					o->SubType = 101;
				}
			}
			else if(7.0f < o->AnimationFrame && o->AnimationFrame < 7.5f)
			{
				if( o->SubType == 101 )
				{
					o->SubType = 100;
				}
			}
		}
		return true;
		
	case MODEL_PROJECTILE: // 투석기 발사체
		{
			b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
		}
		return true;
	case MODEL_DOOR_CRUSH_EFFECT_PIECE01:	//성문 파괴 이펙트 조각 1
	case MODEL_DOOR_CRUSH_EFFECT_PIECE02:	//성문 파괴 이펙트 조각 2
	case MODEL_DOOR_CRUSH_EFFECT_PIECE03:	//성문 파괴 이펙트 조각 3
	case MODEL_DOOR_CRUSH_EFFECT_PIECE04:	//성문 파괴 이펙트 조각 4
	case MODEL_DOOR_CRUSH_EFFECT_PIECE05:	//성문 파괴 이펙트 조각 5
	case MODEL_DOOR_CRUSH_EFFECT_PIECE06:	//성문 파괴 이펙트 조각 6
	case MODEL_DOOR_CRUSH_EFFECT_PIECE07:	//성문 파괴 이펙트 조각 7
	case MODEL_DOOR_CRUSH_EFFECT_PIECE08:	//성문 파괴 이펙트 조각 8
	case MODEL_DOOR_CRUSH_EFFECT_PIECE09:	//성문 파괴 이펙트 조각 9
	case MODEL_DOOR_CRUSH_EFFECT_PIECE10:	//성문 파괴 이펙트 조각 9
	case MODEL_DOOR_CRUSH_EFFECT_PIECE11:	//성문 파괴 이펙트 조각 7
	case MODEL_DOOR_CRUSH_EFFECT_PIECE12:	//성문 파괴 이펙트 조각 8
	case MODEL_DOOR_CRUSH_EFFECT_PIECE13:	//성문 파괴 이펙트 조각 9
	case MODEL_STATUE_CRUSH_EFFECT_PIECE01:	//석상 파괴 이펙트 조각 1
	case MODEL_STATUE_CRUSH_EFFECT_PIECE02:	//석상 파괴 이펙트 조각 2
	case MODEL_STATUE_CRUSH_EFFECT_PIECE03:	//석상 파괴 이펙트 조각 3
	case MODEL_STATUE_CRUSH_EFFECT_PIECE04:	//석상 파괴 이펙트 조각 4
		{
			b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
		}
		return true;

	case MODEL_MONSTER01+166:	// 레이몬드
	case MODEL_MONSTER01+167:	// 에르칸느
	case MODEL_MONSTER01+183:	// 창살형 성문
	case MODEL_MONSTER01+184:	// 사자머리 성문
	case MODEL_MONSTER01+185:	//석상
	case MODEL_MONSTER01+171:	// 갈리아
	case MODEL_MONSTER01+172:	// 병참장교
#ifdef LDS_ADD_EG_ADDMONSTER_ACTION_GUARDIANDEFENDER
	case MODEL_MONSTER01+178:	// 방패병
#endif // LDS_ADD_EG_ADDMONSTER_ACTION_GUARDIANDEFENDER
		{
			RenderMonster(o, b, ExtraMon);
		}
		return true;
	}

	return false;
}

bool GMEmpireGuardian1::RenderObjectVisual( OBJECT* o, BMD* b )
{
	if(IsEmpireGuardian1() == false) return false;

	vec3_t p, Position, Light;
	Vector(0.f,30.f,0.f,Position);
	Vector(0.f,0.f,0.f,p);

	switch(o->Type)
	{
	case 12:		// CobraST (코브라석상 눈 이팩트)
		{	
			vec3_t vPos, vRelativePos, vLight1, vLight2;
			float flumi = absf(sinf(WorldTime*0.0008))*0.9f+0.1f;
			float fScale = o->Scale*0.3f*flumi;
			Vector(0.f, 0.f, 0.f, vPos);
			Vector(8.f, -3.f, -3.f, vRelativePos);
			Vector(flumi, flumi, flumi, vLight1);
			Vector(0.9f, 0.1f, 0.1f, vLight2);
			b->TransformPosition(BoneTransform[2], vRelativePos, vPos);
#ifdef LDS_FIX_ACCESS_INDEXNUMBER_ALREADY_LOADTEXTURE
			CreateSprite(BITMAP_SHINY+6, vPos, 0.5f, vLight2, o );
			CreateSprite(BITMAP_SHINY+6, vPos, fScale, vLight1, o );
			Vector(3.f, -3.f, -3.5f, vRelativePos);
			b->TransformPosition(BoneTransform[3], vRelativePos, vPos);
			CreateSprite(BITMAP_SHINY+6, vPos, 0.5f, vLight2, o );
			CreateSprite(BITMAP_SHINY+6, vPos, fScale, vLight1, o );
#else // LDS_FIX_ACCESS_INDEXNUMBER_ALREADY_LOADTEXTURE
			CreateSprite(BITMAP_SHINY+5, vPos, 0.5f, vLight2, o );
			CreateSprite(BITMAP_SHINY+5, vPos, fScale, vLight1, o );
			Vector(3.f, -3.f, -3.5f, vRelativePos);
			b->TransformPosition(BoneTransform[3], vRelativePos, vPos);
			CreateSprite(BITMAP_SHINY+5, vPos, 0.5f, vLight2, o );
			CreateSprite(BITMAP_SHINY+5, vPos, fScale, vLight1, o );
#endif // LDS_FIX_ACCESS_INDEXNUMBER_ALREADY_LOADTEXTURE
		}
		return true;

	case 20:		// 좌우로 자르는 칼
		{
			if (o->AnimationFrame > 5.4f && o->AnimationFrame < 6.5f)
			{
				vec3_t	Angle;
				for(int i = 0; i < 4; ++i)
				{
					Vector((float)(rand()%60+60+90),0.f,o->Angle[2]+180,Angle );
					CreateJoint(BITMAP_JOINT_SPARK,o->Position,o->Position,Angle,5,o);
				}
				CreateParticle(BITMAP_SPARK,o->Position,Angle,o->Light,11);
			}
			else if (o->AnimationFrame > 15.4f && o->AnimationFrame < 16.5f)
			{
				vec3_t	Angle;
				for(int i = 0; i < 4; ++i)
				{
					Vector((float)(rand()%60+60+90),0.f,o->Angle[2],Angle );
					CreateJoint(BITMAP_JOINT_SPARK,o->Position,o->Position,Angle,5,o);
				}
				CreateParticle(BITMAP_SPARK,o->Position,Angle,o->Light,11);
			}
		}
		return true;

	case 37:	// 벽등
		{
			Vector(0.f, 0.f, 0.f, p);
			b->TransformPosition(BoneTransform[1], p, Position);
			
			float fLumi;
			fLumi = (sinf(WorldTime*0.039f) + 1.0f) * 0.2f + 0.6f;
			vec3_t vLightFire;
			Vector(fLumi*0.7f, fLumi*0.7f, fLumi*0.7f, vLightFire);
			CreateSprite(BITMAP_FLARE, Position, 4.0f * o->Scale, vLightFire, o);
		}
		return true;

	case 50:	// sobo_med01 (만다라 석상 손위의 불 이팩트)
		{	
			vec3_t vPos, vRelativePos, vLight1, vLight2, vAngle;
			Vector(0.f, 0.f, 0.f, vPos);
			Vector(0.f, 0.f, 5.f, vRelativePos);
			Vector(0.0f, -1.0f, 0.0f, vAngle);
			Vector(0.05f, 0.1f, 0.3f, vLight1);
			Vector(1.f, 1.f, 1.f, vLight2);
			
			for( int i=2 ; i<=7 ; i++ )
			{
				b->TransformPosition(BoneTransform[i], vRelativePos, vPos);
				CreateParticle(BITMAP_FIRE_HIK3_MONO,vPos,vAngle,vLight1,4,o->Scale*0.6f);	// 불
				CreateParticle(BITMAP_FIRE_HIK3_MONO,vPos,vAngle,vLight2,4,o->Scale*0.3f);	// 불
			}
		}
		return true;

	case 51:		// 좌우로 자르는 칼
		{
			if (o->AnimationFrame > 5.4f && o->AnimationFrame < 6.5f)
			{
				vec3_t	Angle;
				for(int i = 0; i < 4; ++i)
				{
					Vector((float)(rand()%60+60+90),0.f,o->Angle[2]+180,Angle );
					CreateJoint(BITMAP_JOINT_SPARK,o->Position,o->Position,Angle,5,o);
				}
				CreateParticle(BITMAP_SPARK,o->Position,Angle,o->Light,11);
			}
		}
		return true;

	case 64:	// 땅찍는 석상
		{
			if ((o->AnimationFrame > 9.5f && o->AnimationFrame < 11.5f) ||
				(o->AnimationFrame > 23.5f && o->AnimationFrame < 25.5f))
			{
				float Matrix[3][4];
				vec3_t vAngle, vDirection, vPosition;
				Vector(0.f, 0.f, o->Angle[2]+90, vAngle);
				AngleMatrix(vAngle, Matrix);
				Vector(0.f, 30.0f, 0.f, vDirection);
				VectorRotate(vDirection, Matrix, vPosition);
				VectorAdd(vPosition, o->Position, Position);
				
				Vector(0.04f,0.03f,0.02f,Light);
				for (int i = 0; i < 3; ++i)
				{
					CreateParticle ( BITMAP_CLOUD, Position, o->Angle, Light, 22, o->Scale, o );
				}
			}
		}
		return true;
		
	case 79:		// 리얼빨간불박스
		{
			vec3_t vLightFire;
			Vector(1.0f, 0.2f, 0.0f, vLightFire);
			CreateSprite(BITMAP_LIGHT, o->Position, 2.0f * o->Scale, vLightFire, o);
			
			vec3_t vLight;
			Vector(1.0f, 1.0f, 1.0f, vLight);
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
			float fScale = o->Scale * (rand()%5+13)*0.1f;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
			switch(rand()%3)
			{
			case 0:
				CreateParticle(BITMAP_FIRE_HIK1,o->Position,o->Angle,vLight,0,o->Scale);	// 불
				break;
			case 1:
				CreateParticle(BITMAP_FIRE_CURSEDLICH,o->Position,o->Angle,vLight,4,o->Scale);	// 불
				break;
			case 2:
				CreateParticle(BITMAP_FIRE_HIK3,o->Position,o->Angle,vLight,0,o->Scale);	// 불
				break;
			}
		}
		return true;

	case 80:	// 주변광박스
		{
			float fLumi;
			fLumi = (sinf(WorldTime*0.04f) + 1.0f) * 0.3f + 0.4f;
			vec3_t vLightFire;
			Vector(fLumi*0.1f, fLumi*0.1f, fLumi*0.5f, vLightFire);
			CreateSprite(BITMAP_LIGHT, o->Position, 8.0f * o->Scale, vLightFire, o);
		}
		return true;

	case 82:	// 폭포 효과 - 물 쏟아짐
		{
			CreateParticle ( BITMAP_WATERFALL_5, o->Position, o->Angle, Light, 9, o->Scale );
		}
		return true;

	case 83:	// 폭포 효과 - 물 튐
		{
			Vector ( 1.f, 1.f, 1.f, Light );
			CreateParticle ( BITMAP_WATERFALL_3, o->Position, o->Angle, Light, 14, o->Scale );
		}
		return true;

	case 84:  //  폭포 효과 - 물 안개 효과
		{
			Vector ( 1.f, 1.f, 1.f, Light );
			if ( rand()%8==0 )
			{
				CreateParticle ( BITMAP_WATERFALL_2, o->Position, o->Angle, Light, 4, o->Scale );
			}
		}
		return true;

	case 85:	// 법사불기둥박스
		{
			vec3_t vLight;
			Vector(0.5f, 0.5f, 0.5f, vLight);
			int iWorldTime = (int)WorldTime;
			int iRemainder = iWorldTime % 200;
			
			if(iRemainder >= 0 && iRemainder <= 1)
			{
				int iRand = rand()%4 + 4;
				for(int i=0; i<iRand; ++i)
				{
					CreateEffect(BITMAP_FLAME, o->Position, o->Angle, vLight, 6, NULL, -1, 0, o->Scale);
				}
			}
		}
		return true;

	case 86:	// 불기운 박스 적
		{
			if ( rand()%6==0) 
			{
				Vector(0.05f,0.02f,0.01f,Light);
				CreateParticle ( BITMAP_CLOUD, o->Position, o->Angle, Light, 21, o->Scale, o );
			}
		}
		return true;

	case 129:	// 불기운 박스 청
		{
			if ( rand()%6==0) 
			{
				Vector(0.01f,0.02f,0.05f,Light);
				CreateParticle ( BITMAP_CLOUD, o->Position, o->Angle, Light, 21, o->Scale, o );
			}
		}
		return true;

	case 130:	// 불기운 박스 녹
		{
			if ( rand()%6==0) 
			{
				Vector(0.01f,0.05f,0.02f,Light);
				CreateParticle ( BITMAP_CLOUD, o->Position, o->Angle, Light, 21, o->Scale, o );
			}
		}
		return true;

	case 131 :   //  연기박스1.
		{
			if ( rand()%3==0 )
			{
				Vector ( 1.f, 1.f, 1.f, Light );
				CreateParticle ( BITMAP_SMOKE, o->Position, o->Angle, Light, 22 , o->Scale);
				
				Vector ( 1.f, 1.f, 1.f, Light );
				CreateParticle ( BITMAP_SMOKE, o->Position, o->Angle, Light, 21, o->Scale*2.0f, o);
			}
		}
		return true;

	case 132 :   //  연기박스2.
		{
			if ( rand()%3==0) 
			{
				Vector ( 1.f, 1.f, 1.f, Light );
				CreateParticle ( BITMAP_SMOKE, o->Position, o->Angle, Light, 60, o->Scale, o);
				
				Vector ( 1.f, 1.f, 1.f, Light );
				CreateParticle ( BITMAP_SMOKE, o->Position, o->Angle, Light, 60, o->Scale, o);
				
				Vector ( 1.f, 1.f, 1.f, Light );
				CreateParticle ( BITMAP_SMOKE, o->Position, o->Angle, Light, 21, o->Scale*2.0f, o);
			}
		}
		return true;
	}
	
	return false;
}

// 몬스터 렌더링
bool GMEmpireGuardian1::RenderMonster(OBJECT* o, BMD* b, bool ExtraMon)
{
	switch(o->Type)
	{
#ifdef LDS_ADD_EG_3_4_MONSTER_RAYMOND
	case MODEL_MONSTER01+166: // 제국 수호군 맵 3,4 (수토,일)몬스터 부사령관 레이몬드			(506/166)		
		{
#ifdef PBG_ADD_RAYMOND_GUARDIANPRIEST_MONSTER_EFFECT
			float fBlendMeshLight = (sinf(WorldTime*0.003f)+1.0f)*0.5f;
			b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
			vec3_t _temp;
			VectorCopy(b->BodyLight, _temp);
			Vector(b->BodyLight[0]*3.0f, b->BodyLight[0]*3.0f, b->BodyLight[0]*3.0f, b->BodyLight);
			b->RenderMesh(1,RENDER_TEXTURE|RENDER_BRIGHT, o->Alpha, 1, fBlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,BITMAP_RAYMOND_SWORD);
			VectorCopy(_temp, b->BodyLight);
			b->RenderMesh ( 3, RENDER_BRIGHT|RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );			
#endif //PBG_ADD_RAYMOND_GUARDIANPRIEST_MONSTER_EFFECT
		}
		return true;
#endif // LDS_ADD_EG_3_4_MONSTER_RAYMOND
#ifdef LDS_ADD_EG_2_4_MONSTER_ERCANNE				// 제국 수호군 맵 2,4 (화금,일)몬스터 지휘관 에르칸느			(507/167)
	case MODEL_MONSTER01+167: // 제국 수호군 맵 2,4 (화금,일)몬스터 지휘관 에르칸느			(507/167)
		{
			b->RenderMesh ( 0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );
			
			b->RenderMesh ( 1, RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV);
			
			b->RenderMesh ( 2, RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV);
			
			b->RenderMesh ( 3, RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV);
			b->RenderMesh ( 3, RENDER_BRIGHT|RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );
			
			b->RenderMesh ( 4, RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV);
			//b->RenderMesh ( 4, RENDER_BRIGHT|RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );
			//b->RenderMesh ( 4, RENDER_BRIGHT|RENDER_CHROME6, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );
		}
		return true;
#endif //LDS_ADD_EG_2_4_MONSTER_ERCANNE	
	case MODEL_MONSTER01+171:	// 갈리아
		{
			b->RenderMesh ( 0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );
			b->RenderMesh ( 1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );
			b->RenderMesh ( 3, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );
			
			b->RenderMesh ( 2, RENDER_TEXTURE|RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );
			b->RenderMesh ( 2, RENDER_TEXTURE|RENDER_CHROME4, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );
		}
		return true;
	case MODEL_MONSTER01+172:	// 병참장교
		{
			b->RenderMesh ( 0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );
			b->RenderMesh ( 1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );
			b->RenderMesh ( 2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );
			b->RenderMesh ( 2, RENDER_CHROME|RENDER_BRIGHT, 0.3f, 2, 0.3f, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );
		}
		return true;
#ifdef LDS_ADD_EG_ADDMONSTER_ACTION_GUARDIANDEFENDER	
	case MODEL_MONSTER01+178: // 제국 수호군 맵 1234(모든요일)몬스터 수호군 방패병			(518/178)
		{
			b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
		}
		return true;
#endif // LDS_ADD_EG_ADDMONSTER_ACTION_GUARDIANDEFENDER
	case MODEL_MONSTER01+183:	// 창살형 성문
		{
			int tileX = int(o->Position[0]/100);
			int tileY = int(o->Position[1]/100);
			//위치 강제 이동.
			switch(World)
			{
			case WD_69EMPIREGUARDIAN1:
				{
					if( tileX == 233 && tileY == 55) //1존 함정앞
					{
						o->Position[0] = 23350;	o->Position[1] = 5520;
					}
					else if( (165<=tileX && tileX <= 167) && (25 <= tileY && tileY <= 27) ) //2존 함정앞
					{
						o->Position[0] = 16710;	o->Position[1] = 2620;
					}
				}break;
			case WD_70EMPIREGUARDIAN2:
				{
					if( (49 <= tileX && tileX <= 51) && (64<= tileY && tileY <= 66) ) //1존 함정앞
					{
						o->Position[0] = 5075;	o->Position[1] = 6490;
					}
					else if( (40<=tileX && tileX <= 42) && (116 <= tileY && tileY <= 118) ) //2존 함정앞
					{
						o->Position[0] = 4200;	o->Position[1] = 11680;
					}
				}break;
			case WD_71EMPIREGUARDIAN3:
				{
					if( (118 <= tileX && tileX <= 120) && (191<= tileY && tileY <= 193) ) //1존 함정앞
					{
						o->Scale = 0.9f;
						o->Position[0] = 11985;	o->Position[1] = 19250;
					}
					else if( (221<=tileX && tileX <= 223) && (159 <= tileY && tileY <= 161) ) //2존 함정앞
					{
						o->Scale = 1.08f;
						o->Position[0] = 22300;	o->Position[1] = 16000;
					}
				}break;
			}
			
			b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
		}
		return true;

	case MODEL_MONSTER01+184:	// 사자머리 성문
		{
			int tileX = int(o->Position[0]/100);
			int tileY = int(o->Position[1]/100);
			//위치 강제 이동.
			switch(World)
			{
			case WD_69EMPIREGUARDIAN1:
				{
					if( tileX == 234 && tileY == 28 ) //1존 시작
					{
						o->Position[0] = 23450;	o->Position[1] = 2820;
					}
					else if( tileX == 216 && tileY == 80 ) //1존 워프존
					{
						o->Position[0] = 21650;	o->Position[1] = 8000;
					}
					else if( tileX == 194 && tileY == 25 ) //2존 시작점
					{
						o->Position[0] = 19450;	o->Position[1] = 2530;
					}
					else if( (153 <= tileX && tileX <= 155) && (52<= tileY && tileY <= 54 ) ) //2존 워프존
					{
						o->Scale = 1.15f;
						o->Position[0] = 15510;	o->Position[1] = 5360;
					}
					else if( tileX == 180 && tileY == 79 ) //3존 시작점
					{
						o->Position[0] = 18070;	o->Position[1] = 7950;
					}
				}break;
			case WD_70EMPIREGUARDIAN2:
				{
					if( (74 <= tileX && tileX <= 76) && (66<= tileY && tileY <= 68 ) ) //1존 시작
					{
						o->Scale = 1.17f;
						o->Position[0] = 7620;	o->Position[1] = 6740;
					}
					else if( (18 <= tileX && tileX <= 20) && (64<= tileY && tileY <= 66 ) ) //1존 워프존
					{
						o->Position[0] = 1950;	o->Position[1] = 6500;
					}
					else if( (36 <= tileX && tileX <= 38) && (92<= tileY && tileY <= 94 ) ) //2존 시작점
					{
						o->Scale = 1.1f;
						o->Position[0] = 3770;	o->Position[1] = 9250;
					}
					else if( (54 <= tileX && tileX <= 56) && (153<= tileY && tileY <= 155) ) //2존 워프존
					{
						o->Scale = 1.15f;
						o->Position[0] = 5515;	o->Position[1] = 15350;
					}
					else if( (106 <= tileX && tileX <= 108) && (111<= tileY && tileY <= 113 ) ) //3존 시작점
					{
						o->Scale = 1.05f;
						o->Position[0] = 10830;	o->Position[1] = 11180;
					}
				}break;
			case WD_71EMPIREGUARDIAN3:
				{
					if( (145 <= tileX && tileX <= 147) && (190<= tileY && tileY <= 192) ) //1존 시작
					{
						o->Scale = 1.28f;
						o->Position[0] = 14700;	o->Position[1] = 19140;
					}
					else if( (88 <= tileX && tileX <= 90) && (194<= tileY && tileY <= 196) ) //1존 워프존
					{
						o->Scale = 1.10f;
						o->Position[0] = 9010;	o->Position[1] = 19580;
					}
					else if( (221 <= tileX && tileX <= 223) && (133<= tileY && tileY <= 135 ) ) //2존 시작점
					{
						o->Scale = 1.1f;
						o->Position[0] = 22300;	o->Position[1] = 13360;
					}
					else if( (222 <= tileX && tileX <= 224) && (192<= tileY && tileY <= 194) ) //2존 워프존
					{
						o->Scale = 1.1f;
						o->Position[0] = 22305;	o->Position[1] = 19280;
					}
					else if( (166 <= tileX && tileX <= 168) && (216<= tileY && tileY <= 218 ) ) //3존 시작점
					{
						o->Scale = 1.23f;
						o->Position[0] = 16720;	o->Position[1] = 21750;
					}
				}break;
			}

			if(o->CurrentAction != MONSTER01_DIE)
			{
				b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
			}
		}
		return true;
	case MODEL_MONSTER01+185:	//석상
		{
			if(o->CurrentAction != MONSTER01_DIE)
			{
				b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
			}
		}
		return true;
	}

	return false;
}

bool GMEmpireGuardian1::RenderMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b)
{
 	vec3_t vLight, vPos;
 	
 	switch(o->Type)
 	{
#ifdef LDS_ADD_EG_3_4_MONSTER_RAYMOND
	case MODEL_MONSTER01+166: // 제국 수호군 맵 3,4 (수토,일)몬스터 부사령관 레이몬드			(506/166)		
		{
			// 10. 광폭화 - Effect 
			if( g_isNotCharacterBuff( o ) == true && g_isCharacterBuff( o, eBuff_Berserker ) == true )
			{
				vec3_t vLightRage, vPosRage;
				
				int iBoneCount = b->NumBones;
				Vector(1.0f, 0.1f, 0.1f, vLightRage);
				for (int i = 0; i < iBoneCount; ++i)
				{
					if (rand()%6 > 0) continue;
					if ( i % 5 == 0 )
					{
						b->TransformByObjectBone(vPosRage, o, i);
						CreateParticle(BITMAP_SMOKE,vPosRage,o->Angle,vLightRage,50,1.0f);	// 연기
						CreateParticle(BITMAP_SMOKELINE1+rand()%3,vPosRage,o->Angle,vLightRage,0,1.0f);	// 3종 연기
					}
				}
			} // 10. 광폭화 - Effect 
		}
		return true;
#endif // LDS_ADD_EG_3_4_MONSTER_RAYMOND
#ifdef LDS_ADD_EG_2_4_MONSTER_ERCANNE				// 제국 수호군 맵 2,4 (화금,일)몬스터 지휘관 에르칸느			(507/167)
	case MODEL_MONSTER01+167:	// 제국 수호군 맵 2,4 (화금,일)몬스터 지휘관 에르칸느			(507/167)
		{
			vec3_t vRelative;
			Vector( 0.0f, 0.0f ,0.0f ,vRelative )
			float fLumi1 = (sinf(WorldTime*0.004f) + 0.9f) * 0.25f;
			
			Vector(0.05f+fLumi1, 0.75f+fLumi1, 0.35f+fLumi1, vLight);
			
			
			// 칼날 발광효과
			Vector(0.0f, 0.0f, -10.0f, vRelative);
			b->TransformByObjectBone(vPos, o, 41, vRelative);
			CreateSprite(BITMAP_LIGHT, vPos, 1.0f+fLumi1, vLight, o);
			
			// 칼날 발광효과
			Vector(0.0f, 0.0f, 10.0f, vRelative);
			b->TransformByObjectBone(vPos, o, 41, vRelative);
			CreateSprite(BITMAP_LIGHT, vPos, 1.0f+fLumi1, vLight, o);
			
			
			// 칼날 발광효과
			Vector(0.0f, 0.0f, -10.0f, vRelative);
			b->TransformByObjectBone(vPos, o, 42, vRelative);
			CreateSprite(BITMAP_LIGHT, vPos, 1.0f+fLumi1, vLight, o);
			
			// 칼날 발광효과
			Vector(0.0f, 0.0f, 10.0f, vRelative);
			b->TransformByObjectBone(vPos, o, 42, vRelative);
			CreateSprite(BITMAP_LIGHT, vPos, 1.0f+fLumi1, vLight, o);
			
			// 			// 칼날 발광효과
			// 			Vector(0.0f, 0.0f, -10.0f, vRelative);
			// 			b->TransformByObjectBone(vPos, o, 43, vRelative);
			// 			CreateSprite(BITMAP_LIGHT, vPos, 1.0f+fLumi1, vLight, o);
			// 			
			// 			// 칼날 발광효과
			// 			Vector(0.0f, 0.0f, 10.0f, vRelative);
			// 			b->TransformByObjectBone(vPos, o, 43, vRelative);
			//  			CreateSprite(BITMAP_LIGHT, vPos, 1.0f+fLumi1, vLight, o);
			
			
			// 갑옷 발광 효과
			float fScale = 1.2f;
			for( int i = 50; i <= 55; ++i )
			{
				int iCurrentBoneIndex = i;
				
				Vector(0.0f, 6.0f, 0.0f, vRelative);
				b->TransformByObjectBone(vPos, o, iCurrentBoneIndex, vRelative);
				CreateSprite(BITMAP_LIGHT, vPos, fScale+fLumi1, vLight, o);
				
				fScale = fScale - 0.15f;
			}
			
			// 기본 Effect / 까지

			// 10. 광폭화 - Effect 
			if( g_isNotCharacterBuff( o ) == true && g_isCharacterBuff( o, eBuff_Berserker ) == true )
			{
				vec3_t vLightRage, vPosRage;
				
				int iBoneCount = b->NumBones;
				Vector(1.0f, 0.1f, 0.1f, vLightRage);
				for (int i = 0; i < iBoneCount; ++i)
				{
					if (rand()%6 > 0) continue;
					if ( i % 5 == 0 )
					{
						b->TransformByObjectBone(vPosRage, o, i);
						CreateParticle(BITMAP_SMOKE,vPosRage,o->Angle,vLightRage,50,1.0f);	// 연기
						CreateParticle(BITMAP_SMOKELINE1+rand()%3,vPosRage,o->Angle,vLightRage,0,1.0f);	// 3종 연기
					}
				}
			} // 10. 광폭화 - Effect 
		}
		return true;
#endif // LDS_ADD_EG_2_4_MONSTER_ERCANNE
	case MODEL_MONSTER01+168:	// 데슬러
		{
			// 10. 광폭화 - Effect 
			if( g_isNotCharacterBuff( o ) == true && g_isCharacterBuff( o, eBuff_Berserker ) == true )
			{
				vec3_t vLightRage, vPosRage;
				
				int iBoneCount = b->NumBones;
				Vector(1.0f, 0.1f, 0.1f, vLightRage);
				for (int i = 0; i < iBoneCount; ++i)
				{
					if (rand()%6 > 0) continue;
					if ( i % 5 == 0 )
					{
						b->TransformByObjectBone(vPosRage, o, i);
						CreateParticle(BITMAP_SMOKE,vPosRage,o->Angle,vLightRage,50,1.0f);	// 연기
						CreateParticle(BITMAP_SMOKELINE1+rand()%3,vPosRage,o->Angle,vLightRage,0,1.0f);	// 3종 연기
					}
				}
			} // 10. 광폭화 - Effect 
		}
		return true;
	case MODEL_MONSTER01+171:	// 갈리아
		{
			// 10. 광폭화 - Effect 
			if( g_isNotCharacterBuff( o ) == true && g_isCharacterBuff( o, eBuff_Berserker ) == true )
			{
				vec3_t vLightRage, vPosRage;
				
				int iBoneCount = b->NumBones;
				Vector(1.0f, 0.1f, 0.1f, vLightRage);
				for (int i = 0; i < iBoneCount; ++i)
				{
					if (rand()%6 > 0) continue;
					if ( i % 5 == 0 )
					{
						b->TransformByObjectBone(vPosRage, o, i);
						CreateParticle(BITMAP_SMOKE,vPosRage,o->Angle,vLightRage,50,1.0f);	// 연기
						CreateParticle(BITMAP_SMOKELINE1+rand()%3,vPosRage,o->Angle,vLightRage,0,1.0f);	// 3종 연기
					}
				}
			} // 10. 광폭화 - Effect 
		}
		return true;
	case MODEL_MONSTER01+172:	// 병참장교
		{
			// 라이트
			int i;
			Vector ( 0.5f, 0.5f, 0.8f, vLight );
			int iBoneNumbers[] = { 10, 8, 12, 25 };
			for (i = 1; i < 4; ++i)
			{
				b->TransformByObjectBone(vPos, o, iBoneNumbers[i]);
				CreateSprite ( BITMAP_LIGHT, vPos, 3.1f, vLight, o );
			}

			if(o->AnimationFrame > 2 && o->AnimationFrame < 3)	// 숨연기
			{
				o->m_dwTime = 0;
				b->TransformByObjectBone(vPos, o, 11);
				
				Vector(1.0f,1.0f,1.0f,vLight);
				CreateParticle(BITMAP_SMOKE, vPos, o->Angle, vLight, 4, 1.0f);
			}
		}
		return true;
#ifdef LDS_ADD_EG_MONSTER_GUARDIANDEFENDER	// 제국 수호군 맵 1234(모든요일)몬스터 수호군 방패병			(518/178)
	case MODEL_MONSTER01+178: // 제국 수호군 맵 1234(모든요일)몬스터 수호군 방패병			(518/178)
		{
		}
		return true;
#endif // LDS_ADD_EG_MONSTER_GUARDIANDEFENDER
#ifdef LDS_ADD_EG_MONSTER_GUARDIANPRIEST			// 제국 수호군 맵 1234(모든요일)몬스터 수호군 치유병			(519/179)
	case MODEL_MONSTER01+179: // 제국 수호군 맵 1234(모든요일)몬스터 수호군 치유병			(519/179)
		{
#ifdef PBG_ADD_RAYMOND_GUARDIANPRIEST_MONSTER_EFFECT
			// 기본 구슬에 광원 효과
			if(o->CurrentAction != MONSTER01_ATTACK1 && o->CurrentAction != MONSTER01_ATTACK2)
			{
				vec3_t vRelative;
				Vector(10.0f, 0.0f, 0.0f, vRelative);
				b->TransformPosition(o->BoneTransform[55], vRelative, vPos, true);
				float fLumi = (sinf(WorldTime*0.001f) + 1.0f) * 0.3f + 0.4f;
				Vector(fLumi*0.5f, fLumi*0.5f, fLumi*0.5f, vLight);
				CreateSprite(BITMAP_FLARE_BLUE, vPos, 1.5f, vLight, o);
				Vector(0.5f, 0.5f, 0.5f, vLight);
				CreateSprite(BITMAP_FLARE_BLUE, vPos, 1.2f, vLight, o, -WorldTime*0.1f);
			}
#endif //PBG_ADD_RAYMOND_GUARDIANPRIEST_MONSTER_EFFECT
		}
		return true;
#endif // LDS_ADD_EG_MONSTER_GUARDIANPRIEST
 	}

	return false;
}

void GMEmpireGuardian1::RenderAfterObjectMesh(OBJECT* o, BMD* b, bool ExtraMon)
{
	if(IsEmpireGuardian1() == false) return;
	
	switch(o->Type)
	{
	case 0:
	case 1:
	case 3:
	case 44:	// 돌기둥들
		{
			float fLumi;
			fLumi = (sinf(WorldTime*0.0015f) + 1.0f) * 0.4f + 0.2f;
			
			b->RenderMesh ( 0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );
			b->RenderMesh ( 1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );
			b->RenderMesh ( 2, RENDER_TEXTURE|RENDER_BRIGHT, o->Alpha, 2, fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );
		}
		break;
		
	case 81:	// 폭포물
		{
			b->RenderBody(RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
		}
		break;
	}
}

bool GMEmpireGuardian1::CreateRain( PARTICLE* o )
{
	if(	IsEmpireGuardian1() == false &&
		IsEmpireGuardian2() == false && 
		IsEmpireGuardian3() == false &&
		IsEmpireGuardian4() == false )
	{
		return false;
	}

#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	if(m_iWeather == (int)WEATHER_RAIN || m_iWeather == (int)WEATHER_STORM)
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	if(m_iWeather == WEATHER_TYPE::WEATHER_RAIN || m_iWeather == WEATHER_TYPE::WEATHER_STORM)
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	{
		// 빗줄기
		{
			o->Type = BITMAP_RAIN;
			Vector(	Hero->Object.Position[0]+(float)(rand()%800-300),
				Hero->Object.Position[1]+(float)(rand()%800-400),
				Hero->Object.Position[2]+(float)(rand()%200+400),
				o->Position);
			Vector(-30.f,0.f,0.f,o->Angle);
			vec3_t Velocity;
			Vector(0.f,0.f,-(float)(rand()%24+30),Velocity);
			float Matrix[3][4];
			AngleMatrix(o->Angle,Matrix);
			VectorRotate(Velocity,Matrix,o->Velocity);
		}
		
		// 빗방울 파문
		VectorAdd(o->Position,o->Velocity,o->Position);
		float Height = RequestTerrainHeight(o->Position[0],o->Position[1]);
		if(rand()%2 == 0)
		{
			o->Live = false;
			o->Position[2] = Height+10.f;
			if(rand()%4==0)
				CreateParticle(BITMAP_RAIN_CIRCLE,o->Position,o->Angle,o->Light);
			else
				CreateParticle(BITMAP_RAIN_CIRCLE+1,o->Position,o->Angle,o->Light);
		}
	}
  
	return true;
}

void GMEmpireGuardian1::RenderFrontSideVisual()
{
	switch(m_iWeather)
	{
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	case WEATHER_FOG:
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	case WEATHER_TYPE::WEATHER_FOG: //안개
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
		{
			EnableAlphaBlend();
			glColor3f(0.6f,0.6f,0.9f);
			float WindX2 = (float)((int)WorldTime%100000)*0.00005f; 
			float WindY2 = (float)((int)WorldTime%100000)*0.00008f;
			RenderBitmapUV(BITMAP_CHROME+2,0.f,0.f,640.f,480.f-45.f,WindX2,WindY2,2.0f,2.0f);
			float WindX = -(float)((int)WorldTime%100000)*0.00005f; 
			RenderBitmapUV(BITMAP_CHROME+2,0.f,0.f,640.f,480.f-45.f,WindX,0.f,0.3f,0.3f);
		}
		break;
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	case WEATHER_STORM:
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	case WEATHER_TYPE::WEATHER_STORM: //번개
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
		{
			if( rand()%20 == 0 )
			{
				EnableAlphaBlend();
				glColor3f(0.7f,0.7f,0.9f);
				float WindX2 = (float)((int)WorldTime%100000)*0.0006f; 
				float WindY2 = -(float)((int)WorldTime%100000)*0.0006f; 
				RenderBitmapUV(BITMAP_CHROME+2,0.f,0.f,640.f,480.f-45.f,WindX2,WindY2,3.0f,2.0f);
				WindX2 = -(float)((int)WorldTime%100000)*0.0006f; 
				WindY2 = (float)((int)WorldTime%100000)*0.0006f; 
				RenderBitmapUV(BITMAP_CHROME+2,0.f,0.f,640.f,480.f-45.f,WindX2,WindY2,3.0f,2.0f);
			}
		}
		break;
	}
}

bool IsEmpireGuardian1()
{
	if(World == WD_69EMPIREGUARDIAN1)
	{
		return true;
	}
	return false;
}

#ifdef LDS_FIX_SKILLKEY_DISABLE_WHERE_EG_ALLTELESKILL
bool IsEmpireGuardian()
{
	if(	IsEmpireGuardian1() == false &&
		IsEmpireGuardian2() == false && 
		IsEmpireGuardian3() == false &&
		IsEmpireGuardian4() == false )
	{
		return false;
	}

	return true;
}
#endif // LDS_FIX_SKILLKEY_DISABLE_WHERE_EG_ALLTELESKILL

bool GMEmpireGuardian1::SetCurrentActionMonster(CHARACTER* c, OBJECT* o)
{
	if(	IsEmpireGuardian1() == false &&
		IsEmpireGuardian2() == false && 
		IsEmpireGuardian3() == false &&
		IsEmpireGuardian4() == false )
	{
		return false;
	}
	
 	switch(c->MonsterIndex) 
 	{
#ifdef LDS_ADD_EG_3_4_MONSTER_RAYMOND				// 제국 수호군 맵 3,4 (수토,일)몬스터 부사령관 레이몬드			(506/166)
	case 506: // 제국 수호군 맵 3,4 (수토,일)몬스터 부사령관 레이몬드			(506/166)
		{
			// 광폭화 서버로부터 신호 이후, 광폭화 비주얼 호출 여부. - 레이몬드.
			if( m_bCurrentIsRage_Raymond == true )
			{
				SetAction(o, MONSTER01_APEAR);
				c->MonsterSkill = -1;	// 스킬 공격과 일반 공격이 있는 경우 스킬 공격 후 초기화 해야됨.
				return true;
			}

			switch(c->MonsterSkill)
			{
			case 60:	// Skill:60	// 공격 2
				{
					SetAction(o, MONSTER01_ATTACK2);
					c->MonsterSkill = -1;	// 스킬 공격과 일반 공격이 있는 경우 스킬 공격 후 초기화 해야됨.
				}
				break;
			case 52:	// Skill:52	// 공격 3
				{
					SetAction(o, MONSTER01_ATTACK3);
					c->MonsterSkill = -1;	// 스킬 공격과 일반 공격이 있는 경우 스킬 공격 후 초기화 해야됨.
				}
				break;
			case ATMON_SKILL_EMPIREGUARDIAN_BERSERKER:	// 59 광폭화
				{
					SetAction(o, MONSTER01_APEAR);
					c->MonsterSkill = -1;	// 스킬 공격과 일반 공격이 있는 경우 스킬 공격 후 초기화 해야됨.

					m_bCurrentIsRage_Raymond = true;
				}
				break;
			default:	// 기타 모든 일반공격 
				{
#ifdef PBG_ADD_RAYMOND_GUARDIANPRIEST_MONSTER_EFFECT
					SetAction(o, MONSTER01_ATTACK1);
					c->MonsterSkill = -1;
#else //PBG_ADD_RAYMOND_GUARDIANPRIEST_MONSTER_EFFECT
					// FOR TESTDEBUG
					int arrSkillAttack[] = { MONSTER01_ATTACK1, MONSTER01_ATTACK2, MONSTER01_ATTACK3 };
					int iRandNumCur = rand() % 3;
					int iCurrentSkillAttack = arrSkillAttack[iRandNumCur];
					
					SetAction(o, iCurrentSkillAttack);
					// 					// FOR TESTDEBUG
					//					SetAction(o, MONSTER01_ATTACK2);
#endif //PBG_ADD_RAYMOND_GUARDIANPRIEST_MONSTER_EFFECT
				}
				break;
			}
			
		}
		return true;
#endif //LDS_ADD_EG_3_4_MONSTER_RAYMOND	
#ifdef LDS_ADD_EG_2_4_MONSTER_ERCANNE				// 제국 수호군 맵 2,4 (화금,일)몬스터 지휘관 에르칸느			(507/167)
	case 507: // 제국 수호군 맵 2,4 (화금,일)몬스터 지휘관 에르칸느			(507/167)
		{
			// 광폭화 서버로부터 신호 이후, 광폭화 비주얼 호출 여부. - 에르칸느.
			if( m_bCurrentIsRage_Ercanne == true )
			{
				SetAction(o, MONSTER01_APEAR);
				c->MonsterSkill = -1;	// 스킬 공격과 일반 공격이 있는 경우 스킬 공격 후 초기화 해야됨.
				return true;
			}

			switch(c->MonsterSkill)
			{
			case 62:	// Skill:62	// 공격 3
				{
					SetAction(o, MONSTER01_ATTACK2);
					c->MonsterSkill = -1;	// 스킬 공격과 일반 공격이 있는 경우 스킬 공격 후 초기화 해야됨.
				}
				break;
			case 63:	// Skill:63	// 공격 2
				{
					SetAction(o, MONSTER01_ATTACK3);
					c->MonsterSkill = -1;	// 스킬 공격과 일반 공격이 있는 경우 스킬 공격 후 초기화 해야됨.
				}
				break;	
			case ATMON_SKILL_EMPIREGUARDIAN_BERSERKER:	// 59 광폭화
				{
					SetAction(o, MONSTER01_APEAR);
					c->MonsterSkill = -1;	// 스킬 공격과 일반 공격이 있는 경우 스킬 공격 후 초기화 해야됨.

					m_bCurrentIsRage_Ercanne = true;
				}
				break;
			default:	// 기타 모든 일반공격 
				{	
					SetAction(o, MONSTER01_ATTACK1);
					c->MonsterSkill = -1;
				}
				break;
			}
			
		}
		return true;		
#endif //LDS_ADD_EG_2_4_MONSTER_ERCANNE
#ifdef LDK_ADD_EG_MONSTER_DEASULER
	case 508:	// 데슬러
		{
			
			// 광폭화 서버로부터 신호 이후, 광폭화 비주얼 호출 여부. - 데슬러.
			if( m_bCurrentIsRage_Daesuler == true )
			{
				SetAction(o, MONSTER01_APEAR);
				c->MonsterSkill = -1;	// 스킬 공격과 일반 공격이 있는 경우 스킬 공격 후 초기화 해야됨.
				return true;
			}
			
			switch(c->MonsterSkill)
			{
			case 57:	// Skill:47	// 공격 2
				{
					SetAction(o, MONSTER01_ATTACK2);
					c->MonsterSkill = -1;	// 스킬 공격과 일반 공격이 있는 경우 스킬 공격 후 초기화 해야됨.
				}
				break;
			case 58:	// Skill:49	// 공격 3
				{
					SetAction(o, MONSTER01_ATTACK3);
					c->MonsterSkill = -1;	// 스킬 공격과 일반 공격이 있는 경우 스킬 공격 후 초기화 해야됨.
				}
				break;
			case ATMON_SKILL_EMPIREGUARDIAN_BERSERKER:	// 59 광폭화
				{
					SetAction(o, MONSTER01_APEAR);		// 광폭화 동작
					c->MonsterSkill = -1;	// 스킬 공격과 일반 공격이 있는 경우 스킬 공격 후 초기화 해야됨.
					
					m_bCurrentIsRage_Daesuler = true;
				}
				break;	
			default:	// 기타 모든 일반공격 
				{
					SetAction(o, MONSTER01_ATTACK1);
					c->MonsterSkill = -1;
				}
				break;
			}
		}
		return true;
#endif // LDK_ADD_EG_MONSTER_DEASULER
#ifdef LDK_ADD_EG_MONSTER_GALLIA
		case 511:	// 제국 수호군 갈리아
			{
				// 광폭화 서버로부터 신호 이후, 광폭화 비주얼 호출 여부. - 갈리아.
				if( m_bCurrentIsRage_Gallia == true )
				{
					SetAction(o, MONSTER01_APEAR);
					c->MonsterSkill = -1;	// 스킬 공격과 일반 공격이 있는 경우 스킬 공격 후 초기화 해야됨.
					return true;
				}

				switch(c->MonsterSkill)
				{
				case 58:	// Skill:58	// 공격 2
					{
						SetAction(o, MONSTER01_ATTACK2);
						c->MonsterSkill = -1;	// 스킬 공격과 일반 공격이 있는 경우 스킬 공격 후 초기화 해야됨.
					}
					break;
				case 60:	// Skill:60	// 공격 3
					{
						SetAction(o, MONSTER01_ATTACK3);
						c->MonsterSkill = -1;	// 스킬 공격과 일반 공격이 있는 경우 스킬 공격 후 초기화 해야됨.
					}
					break;
				case ATMON_SKILL_EMPIREGUARDIAN_BERSERKER:	// 59 광폭화
					{
						SetAction(o, MONSTER01_APEAR);		// 광폭화 동작
						c->MonsterSkill = -1;	// 스킬 공격과 일반 공격이 있는 경우 스킬 공격 후 초기화 해야됨.

						m_bCurrentIsRage_Gallia = true;
					}
					break;
				default:	// 기타 모든 일반공격 
					{
						SetAction(o, MONSTER01_ATTACK1);
						c->MonsterSkill = -1;
					}
					break;
				}
			}
			return true;
#endif // LDK_ADD_EG_MONSTER_GALLIA
#ifdef LDK_ADD_EG_MONSTER_DRILLMASTER
		case 513:	// 제국 수호군 전투교관
			{
				switch(c->MonsterSkill)
				{
				case 47:	// Skill:47	// 공격 2
					{
						SetAction(o, MONSTER01_ATTACK2);
						c->MonsterSkill = -1;	// 스킬 공격과 일반 공격이 있는 경우 스킬 공격 후 초기화 해야됨.
					}
					break;
				case 49:	// Skill:49	// 공격 3
					{
						SetAction(o, MONSTER01_APEAR);
						c->MonsterSkill = -1;	// 스킬 공격과 일반 공격이 있는 경우 스킬 공격 후 초기화 해야됨.
					}
					break;
				default:	// 기타 모든 일반공격 
					{
						SetAction(o, MONSTER01_ATTACK1);
						c->MonsterSkill = -1;
					}
					break;
				}
			}
			return true;
#endif // LDK_ADD_EG_MONSTER_DRILLMASTER
#ifdef LDS_ADD_EG_ADDMONSTER_ACTION_GUARDIANDEFENDER
		case 518: // 제국 수호군 맵 1234(모든요일)몬스터 수호군 방패병			(519/178)
			{
				switch(c->MonsterSkill)
				{
				case 44:	// 방패 방어 스킬
					{
						SetAction(o, MONSTER01_APEAR);
						c->MonsterSkill = -1;
					}
					break;
				case 45:	// 방패 가격 스킬
					{
						SetAction(o, MONSTER01_ATTACK2);
						c->MonsterSkill = -1;
					}
					break;
				default:	// 기타 모든 일반공격 
					{
						SetAction(o, MONSTER01_ATTACK1);
						c->MonsterSkill = -1;
					}
					break;
				}
			}
			return true;
#endif // LDS_ADD_EG_ADDMONSTER_ACTION_GUARDIANDEFENDER
#ifdef PBG_ADD_RAYMOND_GUARDIANPRIEST_MONSTER_EFFECT
		case 519: // 제국 수호군 맵 1234(모든요일)몬스터 수호군 치유병			(519/179)
			{
				switch(c->MonsterSkill)
				{
				case 46:	// 공격2
					{
						SetAction(o, MONSTER01_ATTACK2);
						c->MonsterSkill = -1;
					}
					break;
				default:	// 기타 모든 일반공격 
					{
						SetAction(o, MONSTER01_ATTACK1);
						c->MonsterSkill = -1;
					}
					break;
				}
			}
			return true;
#endif //PBG_ADD_RAYMOND_GUARDIANPRIEST_MONSTER_EFFECT
 	}
	
	return false;
}

bool GMEmpireGuardian1::AttackEffectMonster(CHARACTER* c, OBJECT* o, BMD* b)
{
	if(IsEmpireGuardian1() == false)
		return false;

//  switch(c->MonsterIndex) 
//  {
//  }

	return false;
}

// 몬스터 사운드
bool GMEmpireGuardian1::PlayMonsterSound(OBJECT* o) 
{
	if(	IsEmpireGuardian1() == false &&
		IsEmpireGuardian2() == false && 
		IsEmpireGuardian3() == false &&
		IsEmpireGuardian4() == false )
	{
		return false;
	}
	
	float fDis_x, fDis_y;
	fDis_x = o->Position[0] - Hero->Object.Position[0];
	fDis_y = o->Position[1] - Hero->Object.Position[1];
	float fDistance = sqrtf(fDis_x*fDis_x+fDis_y*fDis_y);
	
	if (fDistance > 500.0f) 
		return true;

	switch(o->Type)
	{	
	case MODEL_MONSTER01+166:// 부사령관 레이몬드
		{
			switch(o->CurrentAction)
			{
			case MONSTER01_WALK:
				{
					if( 7.0f <= o->AnimationFrame && o->AnimationFrame < 8.0f )
					{
						PlayBuffer(SOUND_EMPIREGUARDIAN_JERINT_MONSTER_MOVE01);
					}
					if( 1.0f <= o->AnimationFrame && o->AnimationFrame < 2.0f )
					{
						PlayBuffer(SOUND_EMPIREGUARDIAN_JERINT_MONSTER_MOVE02);
					}
				}
				break;
			case MONSTER01_ATTACK1:
				{
					PlayBuffer(SOUND_EMPIREGUARDIAN_JERINT_MONSTER_ATTACK01);
				}
				break;
			case MONSTER01_ATTACK2:
				{
					PlayBuffer(SOUND_EMPIREGUARDIAN_RAYMOND_MONSTER_ATTACK02);// Raymond_attack2.wav
					// 					PlayBuffer(SOUND_SKILL_BLOWOFDESTRUCTION);
				}
				break;
			case MONSTER01_ATTACK3:
				{
					PlayBuffer(SOUND_EMPIREGUARDIAN_RAYMOND_MONSTER_ATTACK02);// Raymond_attack2.wav
					// 					PlayBuffer(SOUND_METEORITE01);
					// 					PlayBuffer(SOUND_EXPLOTION01);
				}
				break;
			case MONSTER01_APEAR:
				{
					PlayBuffer(SOUND_EMPIREGUARDIAN_RAYMOND_MONSTER_RAGE);// Raymond_rage.wav		
				}
				break;
			case MONSTER01_DIE:
				{
					PlayBuffer(SOUND_EMPIREGUARDIAN_JERINT_MONSTER_DEATH);
				}
				break;
			}
		}
		return true;
#ifdef LDS_ADD_EG_2_4_MONSTER_ERCANNE				// 제국 수호군 맵 2,4 (화금,일)몬스터 지휘관 에르칸느			(507/167)
	case MODEL_MONSTER01+167:// 지휘관 에르칸느
		{
			switch(o->CurrentAction)
			{
			case MONSTER01_WALK:
				{
					if( rand() % 2 == 0 )
					{
						PlayBuffer( SOUND_EMPIREGUARDIAN_JERINT_MONSTER_MOVE01 );
					}
					else
					{
						PlayBuffer( SOUND_EMPIREGUARDIAN_JERINT_MONSTER_MOVE02 );
					}
				}
				break;
			case MONSTER01_ATTACK1:
				{
					PlayBuffer( SOUND_EMPIREGUARDIAN_JERINT_MONSTER_ATTACK01 );	// 공격1 사운드
				}
				break;
			case MONSTER01_ATTACK2:
				{
					PlayBuffer( SOUND_EMPIREGUARDIAN_JERINT_MONSTER_ATTACK03 );	// 공격2 사운드
				}
				break;
			case MONSTER01_ATTACK3:
				{
					PlayBuffer( SOUND_EMPIREGUARDIAN_ERCANNE_MONSTER_ATTACK03 );	// 공격3 사운드
				}
				break;
			case MONSTER01_APEAR:
				{
					PlayBuffer( SOUND_EMPIREGUARDIAN_RAYMOND_MONSTER_RAGE );	// 광폭화 사운드
				}
				break;
			case MONSTER01_DIE:
				{
					PlayBuffer( SOUND_EMPIREGUARDIAN_JERINT_MONSTER_DEATH );	// 죽음 사운드
				}
				break;
			}
		}
		return true;
#endif // LDS_ADD_EG_2_4_MONSTER_ERCANNE	지휘관 에르칸느
	case MODEL_MONSTER01+168:// 1군단장 데슬러			(508/168)
		{
			switch(o->CurrentAction)
			{
			case MONSTER01_WALK:
				{
					if( 7.0f <= o->AnimationFrame && o->AnimationFrame < 8.0f )
					{
						PlayBuffer(SOUND_EMPIREGUARDIAN_JERINT_MONSTER_MOVE01);
					}
					if( 1.0f <= o->AnimationFrame && o->AnimationFrame < 2.0f )
					{
						PlayBuffer(SOUND_EMPIREGUARDIAN_JERINT_MONSTER_MOVE02);
					}
				}
				break;
			case MONSTER01_ATTACK1:
				{
					PlayBuffer(SOUND_EMPIREGUARDIAN_COMBATMASTER_ATTACK01);
				}
				break;
			case MONSTER01_ATTACK2:
				{
					PlayBuffer(SOUND_EMPIREGUARDIAN_1CORP_DEASULER_MONSTER_ATTACK02);// 1Deasuler_attack2.wav
				}
				break;
			case MONSTER01_ATTACK3:
				{
					PlayBuffer(SOUND_EMPIREGUARDIAN_1CORP_DEASULER_MONSTER_ATTACK03);// 1Deasuler_attack3.wav
				}
				break;
			case MONSTER01_APEAR:
				{
					PlayBuffer(SOUND_EMPIREGUARDIAN_JERINT_MONSTER_RAGE);
				}
				break;
			case MONSTER01_DIE:
				{
					PlayBuffer(SOUND_EMPIREGUARDIAN_JERINT_MONSTER_DEATH);
				}
				break;
			}
		}
		return true;

	case MODEL_MONSTER01+171://갈리아
		{
			switch(o->CurrentAction)
			{
			case MONSTER01_WALK:
				{ 
					PlayBuffer(SOUND_EMPIREGUARDIAN_3CORP_CATO_MOVE);
				}
				break;
			case MONSTER01_ATTACK1:
				{
					PlayBuffer(SOUND_EMPIREGUARDIAN_JERINT_MONSTER_ATTACK01);
				}
				break;
			case MONSTER01_ATTACK2:
			case MONSTER01_ATTACK3:
				{
					PlayBuffer(SOUND_EMPIREGUARDIAN_4CORP_GALLIA_ATTACK02);// 4Gallia_attack2.wav
				}
				break;
			case MONSTER01_APEAR:
				{
					PlayBuffer(SOUND_EMPIREGUARDIAN_JERINT_MONSTER_RAGE);
				}
				break;
			case MONSTER01_DIE:
				{
					PlayBuffer(SOUND_EMPIREGUARDIAN_2CORP_VERMONT_MONSTER_DEATH);
				}
				break;
			}
		}
		return true;

	case MODEL_MONSTER01+172://병참장교
		{
			switch(o->CurrentAction)
			{
			case MONSTER01_WALK:
				{
					if( 7.0f <= o->AnimationFrame && o->AnimationFrame < 8.0f )
					{
						PlayBuffer(SOUND_EMPIREGUARDIAN_JERINT_MONSTER_MOVE01);
					}
					if( 1.0f <= o->AnimationFrame && o->AnimationFrame < 2.0f )
					{
						PlayBuffer(SOUND_EMPIREGUARDIAN_JERINT_MONSTER_MOVE02);
					}
				}
				break;
			case MONSTER01_ATTACK1:
				{
					PlayBuffer(SOUND_EMPIREGUARDIAN_JERINT_MONSTER_ATTACK01);
				}
				break;
			case MONSTER01_ATTACK2:
			case MONSTER01_ATTACK3:
				{
					PlayBuffer(SOUND_EMPIREGUARDIAN_QUATERMASTER_ATTACK02);// QuaterMaster_attack2.wav
				}
				break;
			case MONSTER01_DIE:
				{
					PlayBuffer(SOUND_EMPIREGUARDIAN_JERINT_MONSTER_DEATH);
				}
				break;
			}
		}
		return true;

	case MODEL_MONSTER01+173://전투교관
		{
			switch(o->CurrentAction)
			{
			case MONSTER01_WALK:
				{
					if( 7.0f <= o->AnimationFrame && o->AnimationFrame < 8.0f )
					{
						PlayBuffer(SOUND_EMPIREGUARDIAN_JERINT_MONSTER_MOVE01);
					}
					if( 1.0f <= o->AnimationFrame && o->AnimationFrame < 2.0f )
					{
						PlayBuffer(SOUND_EMPIREGUARDIAN_JERINT_MONSTER_MOVE02);
					}
				}
				break;
			case MONSTER01_ATTACK1:
				{
					PlayBuffer(SOUND_EMPIREGUARDIAN_COMBATMASTER_ATTACK01);// CombatMaster_attack1.wav
				}break;

			case MONSTER01_ATTACK2:
				{
					PlayBuffer(SOUND_EMPIREGUARDIAN_COMBATMASTER_ATTACK02);// CombatMaster_attack2.wav
				}break;

			case MONSTER01_ATTACK3:
				{
					PlayBuffer(SOUND_EMPIREGUARDIAN_COMBATMASTER_ATTACK03);// CombatMaster_attack3.wav
				}break;
			case MONSTER01_DIE:
				{
					PlayBuffer(SOUND_EMPIREGUARDIAN_JERINT_MONSTER_DEATH);
				}
				break;
			}
		}
		return true;
	case MODEL_MONSTER01+178:	// 수호군 방패병
		{
			switch(o->CurrentAction)
			{
			case MONSTER01_WALK:
				{
					if( 7.0f <= o->AnimationFrame && o->AnimationFrame < 8.0f )
					{
						PlayBuffer(SOUND_EMPIREGUARDIAN_JERINT_MONSTER_MOVE01);
					}
					if( 1.0f <= o->AnimationFrame && o->AnimationFrame < 2.0f )
					{
						PlayBuffer(SOUND_EMPIREGUARDIAN_JERINT_MONSTER_MOVE02);
					}
				}
				break;
			case MONSTER01_ATTACK1:
				{
					PlayBuffer(SOUND_EMPIREGUARDIAN_COMBATMASTER_ATTACK01);// CombatMaster_attack1.wav
				}break;
				
			case MONSTER01_ATTACK2:
				{
					PlayBuffer(SOUND_EMPIREGUARDIAN_COMBATMASTER_ATTACK02);// CombatMaster_attack2.wav
				}break;
			case MONSTER01_DIE:
				{
					PlayBuffer(SOUND_EMPIREGUARDIAN_DEFENDER_ATTACK02);		
				}
				break;
			}
		} // MODEL_MONSTER01+178:	// 수호군 방패병
		return true;

	case MODEL_MONSTER01+179:	// 수호군 치유병 
		{
			switch(o->CurrentAction)
			{
			case MONSTER01_STOP1:	
			case MONSTER01_STOP2:
				{
					PlayBuffer(SOUND_EMPIREGUARDIAN_PRIEST_STOP);
				}break;
			case MONSTER01_ATTACK1:
				{
					PlayBuffer(SOUND_THUNDERS01);
				}break;
				
			case MONSTER01_ATTACK2:
				{
					PlayBuffer(SOUND_RAKLION_SERUFAN_CURE);
				}break;
			case MONSTER01_DIE:
				{
					PlayBuffer(SOUND_DARKLORD_DEAD);		
				}
				break;
			}
		} // MODEL_MONSTER01+179:	// 수호군 치유병 
		return true;
	}

	return false; 
}

// 오브젝트 사운드
void GMEmpireGuardian1::PlayObjectSound(OBJECT* o)
{
	if(	IsEmpireGuardian1() == false &&
		IsEmpireGuardian2() == false && 
		IsEmpireGuardian3() == false &&
		IsEmpireGuardian4() == false )
	{
		return;
	}

	if( IsEmpireGuardian4() )
	{
 		PlayBuffer(SOUND_EMPIREGUARDIAN_INDOOR_SOUND, NULL, false);
 		return;
	}

	switch(m_iWeather)
	{
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	case WEATHER_SUN:
		//사운드 없음
		break;

	case WEATHER_RAIN:
		PlayBuffer(SOUND_EMPIREGUARDIAN_WEATHER_RAIN, NULL, false);
		break;

	case WEATHER_FOG:
		PlayBuffer(SOUND_EMPIREGUARDIAN_WEATHER_FOG, NULL, false);
		break;

	case WEATHER_STORM:
		PlayBuffer(SOUND_EMPIREGUARDIAN_WEATHER_STORM, NULL, false);
		break;
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	case WEATHER_TYPE::WEATHER_SUN:
		//사운드 없음
		break;

	case WEATHER_TYPE::WEATHER_RAIN:
		PlayBuffer(SOUND_EMPIREGUARDIAN_WEATHER_RAIN, NULL, false);
		break;

	case WEATHER_TYPE::WEATHER_FOG:
		PlayBuffer(SOUND_EMPIREGUARDIAN_WEATHER_FOG, NULL, false);
		break;

	case WEATHER_TYPE::WEATHER_STORM:
		PlayBuffer(SOUND_EMPIREGUARDIAN_WEATHER_STORM, NULL, false);
		break;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	}
}

void GMEmpireGuardian1::PlayBGM()
{
	if (IsEmpireGuardian1())
	{
		PlayMp3(g_lpszMp3[MUSIC_EMPIREGUARDIAN1]);
	}
#ifdef PBG_FIX_STOPBGMSOUND
	else
	{
		StopMp3(g_lpszMp3[MUSIC_EMPIREGUARDIAN1]);
	}
#endif //PBG_FIX_STOPBGMSOUND
}
#endif	// LDK_ADD_MAP_EMPIREGUARDIAN1