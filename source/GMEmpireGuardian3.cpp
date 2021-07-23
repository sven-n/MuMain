// GMEmpireGuardian3.cpp: implementation of the GMEmpireGuardian3 class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#ifdef LDK_ADD_MAP_EMPIREGUARDIAN3

#include "GMEmpireGuardian3.h"
#include "ZzzBMD.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzAI.h"
#include "ZzzEffect.h"
#include "w_MapHeaders.h"
#include "DSPlaySound.h"
#include "ZzzOpenData.h"
#include "ZzzLodTerrain.h"

#include "GOBoid.h"

extern char* g_lpszMp3[NUM_MUSIC];

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

GMEmpireGuardian3Ptr GMEmpireGuardian3::Make()
{
	GMEmpireGuardian3Ptr empire(new GMEmpireGuardian3);
	empire->Init();
	return empire;
}

GMEmpireGuardian3::GMEmpireGuardian3()
{
// 	m_iWeather = WEATHER_TYPE::WEATHER_SUN;
}

GMEmpireGuardian3::~GMEmpireGuardian3()
{
	Destroy();
}

void GMEmpireGuardian3::Init()
{

}

void GMEmpireGuardian3::Destroy()
{

}

bool GMEmpireGuardian3::CreateObject(OBJECT* o)
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

CHARACTER* GMEmpireGuardian3::CreateMonster(int iType, int PosX, int PosY, int Key)
{
	CHARACTER* pCharacter = g_EmpireGuardian1.CreateMonster(iType, PosX, PosY, Key);

	// 중복 코딩 방지용
	//g_EmpireGuardian1에 없으면 로딩
	if(NULL != pCharacter)
	{
		return pCharacter;
	}

 	switch (iType)
 	{
	case 510: // 카토
		{
			OpenMonsterModel(170);
			pCharacter = CreateCharacter(Key, MODEL_MONSTER01+170, PosX, PosY);
			strcpy(pCharacter->ID, "카토");
			pCharacter->Object.Scale = 1.35f;
			
			m_bCurrentIsRage_Kato = false;
		}
		break;
#ifdef LDK_ADD_EG_MONSTER_ASSASSINMASTER
	case 516: // 암살단장
		{
			OpenMonsterModel(176);
			pCharacter = CreateCharacter(Key, MODEL_MONSTER01+176, PosX, PosY);
			strcpy(pCharacter->ID, "암살단장");
			pCharacter->Object.Scale = 1.55f;
		}
		break;
#endif //LDK_ADD_EG_MONSTER_ASSASSINMASTER
	case 517: // 기마단장
		{
			OpenMonsterModel(177);
			pCharacter = CreateCharacter(Key, MODEL_MONSTER01+177, PosX, PosY);
			strcpy(pCharacter->ID, "기마단장");
			pCharacter->Object.Scale = 1.25f;
		}
		break;

	default: return pCharacter;
 	}

	return pCharacter;
}

bool GMEmpireGuardian3::MoveObject(OBJECT* o)
{
	if(IsEmpireGuardian3() == false)
		return false;

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
			o->Velocity = 0.44f;	// 애니메이션 속도 조절
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
		return true; //?
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

bool GMEmpireGuardian3::MoveMonsterVisual(OBJECT* o, BMD* b)
{
	//g_EmpireGuardian1에 character정보 있음
	if(true == g_EmpireGuardian1.MoveMonsterVisual(o, b))
	{
		return true;
	}

	return false;
}



// 몬스터(NPC) 프로세서 - 제국 수호군 특화 (Boss gaion이 선택한 케릭터 정보필요로 character* c 인자값 추가.)
bool GMEmpireGuardian3::MoveMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b)
{
//  	if(IsEmpireGuardian3() == false)
  //		return false;

	//g_EmpireGuardian1에 character정보 있음
	if(true == g_EmpireGuardian1.MoveMonsterVisual(c, o, b))
	{
		return true;
	}

	vec3_t vPos;
	
 	switch(o->Type)
 	{
	case MODEL_MONSTER01+170:// 3군단장 카토
		{
			switch(o->CurrentAction)
			{
			case MONSTER01_WALK:
			case MONSTER01_ATTACK1:
			case MONSTER01_ATTACK2:
			case MONSTER01_ATTACK3:
			case MONSTER01_DIE:	
				break;
			case MONSTER01_APEAR:		// 광폭화
				{// 광폭화 액션 EFFECT 처리
					vec3_t		Light;
					
					if( m_bCurrentIsRage_Kato == true )
					{
						Vector ( 1.0f, 1.0f, 1.0f, Light );
						CreateInferno(o->Position);
						
						CreateEffect(MODEL_CIRCLE,o->Position,o->Angle,Light,4,o);
						CreateEffect(MODEL_CIRCLE,o->Position,o->Angle,Light,4,o);	
						m_bCurrentIsRage_Kato = false;
					}
				}
				break;
			}
		}
		return true;	
	case MODEL_MONSTER01+176:// 암살단장
		{
			switch(o->CurrentAction)
			{
			case MONSTER01_WALK:
				break;
			case MONSTER01_ATTACK1:
				{
					if( 2.0f <= o->AnimationFrame && o->AnimationFrame < 15.0f )
					{
						vec3_t  Light;
//						Vector(0.3f, 0.3f, 0.3f, Light);
						Vector(0.3f, 0.8f, 0.4f, Light);
						
						vec3_t StartPos, StartRelative;
						vec3_t EndPos, EndRelative;
						
						float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed;
						float fSpeedPerFrame = fActionSpeed/10.f;
						float fAnimationFrame = o->AnimationFrame - fActionSpeed;
						for(int i=0; i<10; i++) 
						{
							b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);
							
							Vector(0.0f, 0.0f, 0.0f, StartRelative);
							Vector(0.0f, 0.0f, 0.0f, EndRelative);
							
							b->TransformPosition(BoneTransform[49], StartRelative, StartPos, false);
							b->TransformPosition(BoneTransform[51], EndRelative, EndPos, false);
							CreateObjectBlur(o, StartPos, EndPos, Light, 3, false, o->m_iAnimation + 1);
//							CreateObjectBlur(o, StartPos, EndPos, Light, 2, false, o->m_iAnimation + 1);
//							CreateObjectBlur(o, StartPos, EndPos, Light, 3, false, o->m_iAnimation + 1);
//							CreateObjectBlur(o, StartPos, EndPos, Light, 2);
//							CreateObjectBlur(o, StartPos, EndPos, Light, 3);
						
							fAnimationFrame += fSpeedPerFrame;
						}
					}
				}
				break;
			case MONSTER01_ATTACK2:
				{
					if( c->AttackTime >= 1 && c->AttackTime <= 2 )
					{
						vec3_t Angle;
						Vector(1.f,0.f,0.f,Angle);
						CreateEffect(BITMAP_GATHERING,o->Position,o->Angle,o->Light,1,o);
					}

					if( 4.5f <= o->AnimationFrame && o->AnimationFrame < 4.6f )
					{
						vec3_t  Light;
						Vector(1.0f, 0.5f, 0.2f, Light);
						
						vec3_t vPosBlur01, vPosBlurRelative01;
						vec3_t vPosBlur02, vPosBlurRelative02;
						
						float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed;
						float fSpeedPerFrame = fActionSpeed/6.f;
						float fAnimationFrame = o->AnimationFrame - fActionSpeed;
						for(int i=0; i<8; i++) 
						{
							b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);
							
							Vector(0.0f, 0.0f, 0.0f, vPosBlurRelative01);
							Vector(0.0f, 0.0f, 0.0f, vPosBlurRelative02);
							
							b->TransformPosition(BoneTransform[49], vPosBlurRelative01, vPosBlur01, false);
							b->TransformPosition(BoneTransform[51], vPosBlurRelative02, vPosBlur02, false);								
							
							CreateBlur(c, vPosBlur01, vPosBlur02, Light, 2);			
						
							fAnimationFrame += fSpeedPerFrame;	
						}
						Vector(0.1f, 0.6f, 0.3f, Light);
						CreateJoint(BITMAP_JOINT_FORCE,vPosBlur01, vPosBlur02,o->Angle,10,o,150.f,o->PKKey,o->Skill, 1, -1,Light);

						for(int iLoop = 0; iLoop < 3; iLoop++)
						{
							vPosBlur01[2] -= 30.f;
							vPosBlur02[2] -= 30.f;
							CreateJoint(BITMAP_JOINT_FORCE,vPosBlur01, vPosBlur02,o->Angle,10,o,150.f,o->PKKey,o->Skill, 1, -1,Light);
						}

//						c->AttackTime = 15;
					}

				}
				break;
			case MONSTER01_ATTACK3:
				{
					if( c->AttackTime >= 1 && c->AttackTime <= 2 )
					{
						vec3_t Angle;
						Vector(1.f,0.f,0.f,Angle);
						CreateEffect(BITMAP_GATHERING,o->Position,o->Angle,o->Light,1,o);
					}

					if( 4.5f <= o->AnimationFrame && o->AnimationFrame < 4.6f )
					{
						vec3_t  Light;
						Vector(1.0f, 0.5f, 0.2f, Light);
						
						vec3_t vPosBlur01, vPosBlurRelative01;
						vec3_t vPosBlur02, vPosBlurRelative02;
						
						float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed;
						float fSpeedPerFrame = fActionSpeed/6.f;
						float fAnimationFrame = o->AnimationFrame - fActionSpeed;
						for(int i=0; i<8; i++) 
						{
							b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);
							
							Vector(0.0f, 0.0f, 0.0f, vPosBlurRelative01);
							Vector(0.0f, 0.0f, 0.0f, vPosBlurRelative02);
							
							b->TransformPosition(BoneTransform[49], vPosBlurRelative01, vPosBlur01, false);
							b->TransformPosition(BoneTransform[51], vPosBlurRelative02, vPosBlur02, false);								
							
							CreateBlur(c, vPosBlur01, vPosBlur02, Light, 2);			
						
							fAnimationFrame += fSpeedPerFrame;	
						}
						Vector(1.f, 1.f, 1.f, Light);
						CreateJoint(BITMAP_JOINT_FORCE,vPosBlur01, vPosBlur02,o->Angle,10,o,150.f,o->PKKey,o->Skill, 1, -1,Light);

						for(int iLoop = 0; iLoop < 3; iLoop++)
						{
							vPosBlur01[2] -= 30.f;
							vPosBlur02[2] -= 30.f;
							CreateJoint(BITMAP_JOINT_FORCE,vPosBlur01, vPosBlur02,o->Angle,10,o,150.f,o->PKKey,o->Skill, 1, -1,Light);
						}

//						c->AttackTime = 15;
					}
				}
				break;
			case MONSTER01_DIE:	
				break;
			case MONSTER01_APEAR:		// 광폭화
				{// 광폭화 액션 EFFECT 처리

				}
				break;
			}
		}
		return true;	
	case MODEL_MONSTER01+177:	// 기마단장
		{
			switch(o->CurrentAction)
			{
			case MONSTER01_STOP1:
			case MONSTER01_STOP2:
				{
					if( 4.5f <= o->AnimationFrame && o->AnimationFrame < 10.0f )
					{
						Vector(0.8f, 0.4f, 0.1f, o->Light);
						
						b->TransformByObjectBone( vPos, o, 15 );
						//입 연기
						CreateParticle(BITMAP_CLUD64, vPos, o->Angle, o->Light, 3, 0.2f);
						//입 침 튀기기??
						CreateParticle(BITMAP_WATERFALL_3, vPos, o->Angle, o->Light, 13, 1.0f);
					}
				}
				break;
			case MONSTER01_WALK:
				{
					Vector(0.5f, 0.2f, 0.1f, o->Light);
					
					if( 7.0f <= o->AnimationFrame && o->AnimationFrame < 8.0f )
					{
						//왼발
						b->TransformByObjectBone( vPos, o, 32 );
						vPos[2] += 25.0f;
						CreateParticle ( BITMAP_ADV_SMOKE, vPos, o->Angle, o->Light, 3, 2.0f );
					}
					if( 1.0f <= o->AnimationFrame && o->AnimationFrame < 2.0f )
					{
						//오른발
						b->TransformByObjectBone( vPos, o, 19 );
						vPos[2] += 25.0f;
						CreateParticle ( BITMAP_ADV_SMOKE, vPos, o->Angle, o->Light, 3, 2.0f );
					}
				}
				break;
			case MONSTER01_ATTACK1:	// 공격1
				{

				}	
				break;
			case MONSTER01_ATTACK2: // 공격2
				{
					// - 스킬 이펙트 정의 부분 - 어스 퀘이크 스킬 발동 : 되도록 범위 변동 없도록 유의
					if( o->AnimationFrame >= 3.8f && o->AnimationFrame <= 9.4f )
					{
						RenderSkillEarthQuake ( c, o, b, 14 );
					}
					else
					{
						o->WeaponLevel = 0;
					}
				}
				break;
			case MONSTER01_APEAR:	// 공격3
				{
					
				}
				break;
			case MONSTER01_ATTACK3:	// 공격4
				{
					
				}
				break;
			case MONSTER01_DIE:
				{
					float Scale = 0.3f;
					b->TransformByObjectBone( vPos, o, 30 );
					CreateParticle(BITMAP_SMOKE+1, vPos, o->Angle, o->Light, 1, Scale);
					
					b->TransformByObjectBone( vPos, o, 17 );
					CreateParticle(BITMAP_SMOKE+1, vPos, o->Angle, o->Light, 1, Scale);
					
					b->TransformByObjectBone( vPos, o, 2 );
					CreateParticle(BITMAP_SMOKE+1, vPos, o->Angle, o->Light, 1, Scale);

					b->TransformByObjectBone( vPos, o, 6 );
					CreateParticle(BITMAP_SMOKE+1, vPos, o->Angle, o->Light, 1, Scale);
					
					b->TransformByObjectBone( vPos, o, 98 );
					CreateParticle(BITMAP_SMOKE+1, vPos, o->Angle, o->Light, 1, Scale);

					b->TransformByObjectBone( vPos, o, 91 );
					CreateParticle(BITMAP_SMOKE+1, vPos, o->Angle, o->Light, 1, Scale);
					
					b->TransformByObjectBone( vPos, o, 2 );
					CreateParticle(BITMAP_SMOKE+3, vPos, o->Angle, o->Light, 3, Scale);
					
					b->TransformByObjectBone( vPos, o, 3 );
					CreateParticle(BITMAP_SMOKE+3, vPos, o->Angle, o->Light, 3, Scale);
				}
				break;
			} //switch end
		}
		return true;
	} //switch end

	return false;
}


void GMEmpireGuardian3::MoveBlurEffect(CHARACTER* c, OBJECT* o, BMD* b)
{
	switch(o->Type)
	{
	case MODEL_MONSTER01+166:	// 레이몬드
	case MODEL_MONSTER01+178:	// 방패병
	case MODEL_MONSTER01+179:	// 치유병
	case MODEL_MONSTER01+180:	// 기사단
	case MODEL_MONSTER01+181:	// 호위병 (검기 이펙트)
		{
			g_EmpireGuardian1.MoveBlurEffect(c, o, b);
		}
		break;

#ifdef KJH_ADD_EG_MONSTER_KATO_EFFECT
	case MODEL_MONSTER01+170:	// 3군단장 카토
		{
			vec3_t  Light;
			vec3_t StartPos, StartRelative;
			vec3_t EndPos, EndRelative;
			float fActionSpeed, fSpeedPerFrame, fAnimationFrame;

			if( o->CurrentAction == MONSTER01_ATTACK1 )
			{
				if( 4.2f <= o->AnimationFrame && o->AnimationFrame < 6.6f )
				{
					Vector(0.9f, 0.5f, 0.4f, Light);
					
					fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed;
					fSpeedPerFrame = fActionSpeed/5.f;
					fAnimationFrame = o->AnimationFrame - fActionSpeed;

					for(int i=0; i<3; i++) 
					{
						b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);
						
						Vector(0.0f, 0.0f, 0.0f, StartRelative);
						Vector(0.0f, 0.0f, 0.0f, EndRelative);
						
						b->TransformPosition(BoneTransform[24], StartRelative, StartPos, false);
						b->TransformPosition(BoneTransform[25], EndRelative, EndPos, false);
						CreateBlur(c, StartPos, EndPos, Light, 2, false, 1);
						
						fAnimationFrame += fSpeedPerFrame;
					}
				}
				else if( 10.2f <= o->AnimationFrame && o->AnimationFrame < 12.6f )
				{
					Vector(0.9f, 0.5f, 0.4f, Light);

					fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed;
					fSpeedPerFrame = fActionSpeed/5.f;
					fAnimationFrame = o->AnimationFrame - fActionSpeed;
					
					for(int i=0; i<3; i++) 
					{
						b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);
						
						Vector(0.0f, 0.0f, 0.0f, StartRelative);
						Vector(0.0f, 0.0f, 0.0f, EndRelative);
						
						b->TransformPosition(BoneTransform[43], StartRelative, StartPos, false);
						b->TransformPosition(BoneTransform[44], EndRelative, EndPos, false);
						CreateBlur(c, StartPos, EndPos, Light, 2, false, 0);
						//b->TransformPosition(BoneTransform[24], StartRelative, StartPos, false);
						//b->TransformPosition(BoneTransform[25], EndRelative, EndPos, false);
						//CreateBlur(c, StartPos, EndPos, Light, 2, false, 1);
						
						fAnimationFrame += fSpeedPerFrame;
					}
				}
			}
			else if( o->CurrentAction == MONSTER01_ATTACK2 )
			{
				if( 6.2f <= o->AnimationFrame && o->AnimationFrame < 8.2f )
				{
					Vector(0.9f, 0.5f, 0.4f, Light);
							
					fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed;
					fSpeedPerFrame = fActionSpeed/5.f;
					fAnimationFrame = o->AnimationFrame - fActionSpeed;
					for(int i=0; i<3; i++) 
					{
						b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);
						
						Vector(0.0f, 0.0f, 0.0f, StartRelative);
						Vector(0.0f, 0.0f, 0.0f, EndRelative);
						
						b->TransformPosition(BoneTransform[43], StartRelative, StartPos, false);
						b->TransformPosition(BoneTransform[44], EndRelative, EndPos, false);
						CreateBlur(c, StartPos, EndPos, Light, 2, false, 0);
						b->TransformPosition(BoneTransform[24], StartRelative, StartPos, false);
						b->TransformPosition(BoneTransform[25], EndRelative, EndPos, false);
						CreateBlur(c, StartPos, EndPos, Light, 2, false, 1);
						
						fAnimationFrame += fSpeedPerFrame;
					}
				}
			}
			else if( o->CurrentAction == MONSTER01_ATTACK3 )
			{
				// 검기 영역에 불 이펙트 효과.
				vec3_t	vRelative, vRelative2, vPosition;
				Vector(0.0f, 0.0f, 0.0f, vRelative);
				Vector(0.0f, 0.0f, 0.0f, vRelative2);

				if( 6.2f <= o->AnimationFrame && o->AnimationFrame < 8.2f )
				{
					Vector(0.9f, 0.5f, 0.4f, Light);
					
					fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed;
					fSpeedPerFrame = fActionSpeed/10.f;
					fAnimationFrame = o->AnimationFrame - fActionSpeed;
					for(int i=0; i<16; i++) 
					{
						b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);
						
						Vector(0.0f, 0.0f, 0.0f, StartRelative);
						Vector(0.0f, 0.0f, 0.0f, EndRelative);
						
						b->TransformPosition(BoneTransform[43], StartRelative, StartPos, false);
						b->TransformPosition(BoneTransform[44], EndRelative, EndPos, false);
						CreateBlur(c, StartPos, EndPos, Light, 2, false, 10);
						b->TransformPosition(BoneTransform[24], StartRelative, StartPos, false);
						b->TransformPosition(BoneTransform[25], EndRelative, EndPos, false);
						CreateBlur(c, StartPos, EndPos, Light, 2, false, 21);
							
						fAnimationFrame += fSpeedPerFrame;

						// 검기 영역에 불이펙트 출력.
						if( 7.4f <= o->AnimationFrame && o->AnimationFrame < 8.0f )
						{
							int iOffset = 80;

							for( int i_ = 0; i_ < 8; i_++ )
							{
								vRelative[0] = ((rand() % iOffset) - iOffset * 0.5f);
								vRelative[1] = ((rand() % iOffset) - iOffset * 0.5f);
								vRelative[2] = ((rand() % iOffset) - iOffset * 0.5f);
								b->TransformPosition(BoneTransform[24], vRelative, vPosition, false);
								CreateParticle(BITMAP_FIRE,vPosition,o->Angle,o->Light,0);
								
								vRelative[0] = ((rand() % iOffset) - iOffset * 0.5f);
								vRelative[1] = ((rand() % iOffset) - iOffset * 0.5f);
								vRelative[2] = ((rand() % iOffset) - iOffset * 0.5f);
								b->TransformPosition(BoneTransform[25], vRelative, vPosition, false);
								CreateParticle(BITMAP_FIRE,vPosition,o->Angle,o->Light,0);

								vRelative[0] = ((rand() % iOffset) - iOffset * 0.5f);
								vRelative[1] = ((rand() % iOffset) - iOffset * 0.5f);
								vRelative[2] = ((rand() % iOffset) - iOffset * 0.5f);
								b->TransformPosition(BoneTransform[43], vRelative, vPosition, false);
								CreateParticle(BITMAP_FIRE,vPosition,o->Angle,o->Light,0);
								
								vRelative[0] = ((rand() % iOffset) - iOffset * 0.5f);
								vRelative[1] = ((rand() % iOffset) - iOffset * 0.5f);
								vRelative[2] = ((rand() % iOffset) - iOffset * 0.5f);
								b->TransformPosition(BoneTransform[44], vRelative, vPosition, false);
								CreateParticle(BITMAP_FIRE,vPosition,o->Angle,o->Light,0);

								vec3_t	vLight__;
								Vector( 0.9f, 0.9f, 0.9f, vLight__);


								vRelative[0] = ((rand() % iOffset) - iOffset * 0.5f);
								vRelative[1] = ((rand() % iOffset) - iOffset * 0.5f);
								vRelative[2] = ((rand() % iOffset) - iOffset * 0.5f);
								b->TransformPosition(BoneTransform[24], vRelative, vPosition, false);
								CreateParticle(BITMAP_SMOKELINE2,vPosition,o->Angle,vLight__,3);
								
								vRelative[0] = ((rand() % iOffset) - iOffset * 0.5f);
								vRelative[1] = ((rand() % iOffset) - iOffset * 0.5f);
								vRelative[2] = ((rand() % iOffset) - iOffset * 0.5f);
								b->TransformPosition(BoneTransform[25], vRelative, vPosition, false);
								CreateParticle(BITMAP_SMOKELINE2,vPosition,o->Angle,vLight__,3);

								vRelative[0] = ((rand() % iOffset) - iOffset * 0.5f);
								vRelative[1] = ((rand() % iOffset) - iOffset * 0.5f);
								vRelative[2] = ((rand() % iOffset) - iOffset * 0.5f);
								b->TransformPosition(BoneTransform[43], vRelative, vPosition, false);
								CreateParticle(BITMAP_SMOKELINE2,vPosition,o->Angle,vLight__,3);
								
								vRelative[0] = ((rand() % iOffset) - iOffset * 0.5f);
								vRelative[1] = ((rand() % iOffset) - iOffset * 0.5f);
								vRelative[2] = ((rand() % iOffset) - iOffset * 0.5f);
								b->TransformPosition(BoneTransform[44], vRelative, vPosition, false);
								CreateParticle(BITMAP_SMOKELINE2,vPosition,o->Angle,vLight__,3);
							}
						} // 검기 
					}
				}
			}
		}break;
#endif // KJH_ADD_EG_MONSTER_KATO_EFFECT
	case MODEL_MONSTER01+177:	// 기마단장
		{
			switch(o->CurrentAction)
			{
			case MONSTER01_ATTACK1:
				{
					if( 4.2f <= o->AnimationFrame && o->AnimationFrame < 8.9f )
					{
						vec3_t  Light;
						Vector(1.0f, 1.0f, 1.0f, Light);
						
						vec3_t StartPos, StartRelative;
						vec3_t EndPos, EndRelative;
						
						float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed;
						float fSpeedPerFrame = fActionSpeed/5.f;
						float fAnimationFrame = o->AnimationFrame - fActionSpeed;
						for(int i=0; i<5; i++) 
						{
							b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);
							
							Vector(0.0f, 0.0f, 0.0f, StartRelative);
							Vector(0.0f, 0.0f, 0.0f, EndRelative);
							
							b->TransformPosition(BoneTransform[35], StartRelative, StartPos, false);
							b->TransformPosition(BoneTransform[36], EndRelative, EndPos, false);
							//CreateBlur(c, StartPos, EndPos, Light, 6);
							CreateBlur(c, EndPos, StartPos, Light, 6);
							
							fAnimationFrame += fSpeedPerFrame;
						}
					}
				}
				break;
			} //switch end
		}
		break;
	} //switch end
}

bool GMEmpireGuardian3::RenderObjectMesh(OBJECT* o, BMD* b, bool ExtraMon)
{
	if(IsEmpireGuardian3() == false)
		return false;

	//g_EmpireGuardian1에 character정보 있음
	if(true == g_EmpireGuardian1.RenderObjectMesh(o, b, ExtraMon))
	{
		return true;
	}
	
	return false;
}

bool GMEmpireGuardian3::RenderObjectVisual( OBJECT* o, BMD* b )
{
	if(IsEmpireGuardian3() == false)
		return false;

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

bool GMEmpireGuardian3::RenderMonster(OBJECT* o, BMD* b, bool ExtraMon)
{
	return false;
}

bool GMEmpireGuardian3::RenderMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b)
{
	//g_EmpireGuardian1에 character정보 있음
	if(g_EmpireGuardian1.RenderMonsterVisual(c, o, b))
	{
		return true;
	}
 	
 	switch(o->Type)
 	{
#ifdef KJH_ADD_EG_MONSTER_KATO_EFFECT
	case MODEL_MONSTER01+170:	// 3군단장 카토
		{
			vec3_t Light, Position;

			// 베이스이팩트

			Vector(0.4f, 0.4f, 0.4f, Light);
			// 얼굴눈
			b->TransformByObjectBone( Position, o, 9 );
			CreateSprite(BITMAP_LIGHT_RED, Position, 0.4f, Light, o);
			CreateSprite(BITMAP_LIGHT_RED, Position, 1.2f, Light, o);
			b->TransformByObjectBone( Position, o, 10 );
			CreateSprite(BITMAP_LIGHT_RED, Position, 0.4f, Light, o);
			CreateSprite(BITMAP_LIGHT_RED, Position, 1.2f, Light, o);
			// 어깨
			b->TransformByObjectBone( Position, o, 28 );
			CreateSprite(BITMAP_LIGHT_RED, Position, 0.4f, Light, o);
			CreateSprite(BITMAP_LIGHT_RED, Position, 1.2f, Light, o);
			b->TransformByObjectBone( Position, o, 29 );
			CreateSprite(BITMAP_LIGHT_RED, Position, 0.4f, Light, o);
			CreateSprite(BITMAP_LIGHT_RED, Position, 1.2f, Light, o);
			b->TransformByObjectBone( Position, o, 47 );
			CreateSprite(BITMAP_LIGHT_RED, Position, 0.4f, Light, o);
			CreateSprite(BITMAP_LIGHT_RED, Position, 1.2f, Light, o);
			b->TransformByObjectBone( Position, o, 48 );
			CreateSprite(BITMAP_LIGHT_RED, Position, 0.4f, Light, o);
			CreateSprite(BITMAP_LIGHT_RED, Position, 1.2f, Light, o);
			// 골반
			b->TransformByObjectBone( Position, o, 123 );
			CreateSprite(BITMAP_LIGHT_RED, Position, 0.4f, Light, o);
			CreateSprite(BITMAP_LIGHT_RED, Position, 1.2f, Light, o);
			b->TransformByObjectBone( Position, o, 124 );
			CreateSprite(BITMAP_LIGHT_RED, Position, 0.4f, Light, o);
			CreateSprite(BITMAP_LIGHT_RED, Position, 1.2f, Light, o);
			b->TransformByObjectBone( Position, o, 119 );
			CreateSprite(BITMAP_LIGHT_RED, Position, 0.4f, Light, o);
			CreateSprite(BITMAP_LIGHT_RED, Position, 1.2f, Light, o);
			b->TransformByObjectBone( Position, o, 120 );
			CreateSprite(BITMAP_LIGHT_RED, Position, 0.4f, Light, o);
			CreateSprite(BITMAP_LIGHT_RED, Position, 1.2f, Light, o);
			// 아랫배
			b->TransformByObjectBone( Position, o, 125 );
			CreateSprite(BITMAP_LIGHT_RED, Position, 0.4f, Light, o);
			CreateSprite(BITMAP_LIGHT_RED, Position, 1.2f, Light, o);
			b->TransformByObjectBone( Position, o, 126 );
			CreateSprite(BITMAP_LIGHT_RED, Position, 0.4f, Light, o);
			CreateSprite(BITMAP_LIGHT_RED, Position, 1.2f, Light, o);	

			// 날개 주위에서 피어 나는 연기 이펙트
			Vector( 0.9f, 0.9f, 0.9f, Light);
			if(rand()%3 == 0) 
			{
				b->TransformByObjectBone( Position, o, 54 );
				CreateParticle(BITMAP_SMOKELINE2, Position, o->Angle, Light, 3, 0.2f);
				b->TransformByObjectBone( Position, o, 58 );
				CreateParticle(BITMAP_SMOKELINE2, Position, o->Angle, Light, 3, 0.2f);
				b->TransformByObjectBone( Position, o, 64 );
				CreateParticle(BITMAP_SMOKELINE2, Position, o->Angle, Light, 3, 0.2f);
				b->TransformByObjectBone( Position, o, 65 );
				CreateParticle(BITMAP_SMOKELINE2, Position, o->Angle, Light, 3, 0.2f);
				b->TransformByObjectBone( Position, o, 69 );
				CreateParticle(BITMAP_SMOKELINE2, Position, o->Angle, Light, 3, 0.2f);
				b->TransformByObjectBone( Position, o, 70 );
				CreateParticle(BITMAP_SMOKELINE2, Position, o->Angle, Light, 3, 0.2f);
				b->TransformByObjectBone( Position, o, 77 );
				CreateParticle(BITMAP_SMOKELINE2, Position, o->Angle, Light, 3, 0.2f);
				b->TransformByObjectBone( Position, o, 78 );
				CreateParticle(BITMAP_SMOKELINE2, Position, o->Angle, Light, 3, 0.2f);
				b->TransformByObjectBone( Position, o, 83 );
				CreateParticle(BITMAP_SMOKELINE2, Position, o->Angle, Light, 3, 0.2f);
				b->TransformByObjectBone( Position, o, 87 );
				CreateParticle(BITMAP_SMOKELINE2, Position, o->Angle, Light, 3, 0.2f);
				b->TransformByObjectBone( Position, o, 91 );
				CreateParticle(BITMAP_SMOKELINE2, Position, o->Angle, Light, 3, 0.2f);
				b->TransformByObjectBone( Position, o, 92 );
				CreateParticle(BITMAP_SMOKELINE2, Position, o->Angle, Light, 3, 0.2f);
			}
			
			b->RenderMesh ( 1, RENDER_CHROME|RENDER_BRIGHT, 0.5f, 1, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );		

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
		}return true;
#endif // KJH_ADD_EG_MONSTER_KATO_EFFECT
#ifdef LDK_ADD_EG_MONSTER_ASSASSINMASTER
	case MODEL_MONSTER01+176:
		{
		}
		return true;
#endif //LDK_ADD_EG_MONSTER_ASSASSINMASTER
 	}

	return false;
}

void GMEmpireGuardian3::RenderAfterObjectMesh(OBJECT* o, BMD* b, bool ExtraMon)
{
	if(IsEmpireGuardian3() == false)
		return;
	
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

void GMEmpireGuardian3::SetWeather(int weather)
{
	g_EmpireGuardian1.SetWeather(weather);
}

bool GMEmpireGuardian3::CreateRain( PARTICLE* o )
{
	//비
	return g_EmpireGuardian1.CreateRain(o);
}

void GMEmpireGuardian3::RenderFrontSideVisual()
{
	//안개, 천둥
	g_EmpireGuardian1.RenderFrontSideVisual();
}

bool IsEmpireGuardian3()
{
	if(World == WD_71EMPIREGUARDIAN3)
	{
		return true;
	}

	return false;
}

bool GMEmpireGuardian3::SetCurrentActionMonster(CHARACTER* c, OBJECT* o)
{
	if(IsEmpireGuardian3() == false)
		return false;

	//g_EmpireGuardian1에 SetCurrentActionMonster정보 있음
	if( true == g_EmpireGuardian1.SetCurrentActionMonster(c, o) )
	{
		return true;
	}
	
 	switch(c->MonsterIndex) 
 	{
	case 510: // 몬스터 3군단장 카토			(510/170)
		{
			// 광폭화 서버로부터 신호 이후, 광폭화 비주얼 호출 여부. - 카토.
			if( m_bCurrentIsRage_Kato == true )
			{
				SetAction(o, MONSTER01_APEAR);
				c->MonsterSkill = -1;	// 스킬 공격과 일반 공격이 있는 경우 스킬 공격 후 초기화 해야됨.
				return true;
			}
			
			switch(c->MonsterSkill)
			{
			case 58:	// Skill:58	// 공격 1
				{
					SetAction(o, MONSTER01_ATTACK2);		// 공격2번 동작.
					c->MonsterSkill = -1;	// 스킬 공격과 일반 공격이 있는 경우 스킬 공격 후 초기화 해야됨.
				}
				break;
			case 60:	// Skill:60	// 공격 2
				{
					//SetAction(o, MONSTER01_ATTACK2);		// 공격2번 동작.
					SetAction(o, MONSTER01_ATTACK3);		// 공격2번 동작.
					c->MonsterSkill = -1;	// 스킬 공격과 일반 공격이 있는 경우 스킬 공격 후 초기화 해야됨.
				}
				break;	
			case ATMON_SKILL_EMPIREGUARDIAN_BERSERKER:	// 59 광폭화
				{
					SetAction(o, MONSTER01_APEAR);
					c->MonsterSkill = -1;	// 스킬 공격과 일반 공격이 있는 경우 스킬 공격 후 초기화 해야됨.
					
					m_bCurrentIsRage_Kato = true;
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
#ifdef LDK_ADD_EG_MONSTER_RIDERMASTER	// 제국 수호군 기마단장
	case 517:	// 제국 수호군 기마단장
		{
			switch(c->MonsterSkill)
			{
			case 54:	// Skill:58	// 공격 2
				{
					SetAction(o, MONSTER01_ATTACK2);
					c->MonsterSkill = -1;	// 스킬 공격과 일반 공격이 있는 경우 스킬 공격 후 초기화 해야됨.
				}
				break;
			case 55:	// Skill:60	// 공격 3 
				{
					SetAction(o, MONSTER01_APEAR);	// 공격 3임. 주의.
					c->MonsterSkill = -1;	// 스킬 공격과 일반 공격이 있는 경우 스킬 공격 후 초기화 해야됨.
				}
				break;
			case 56:	// Skill:56	// 공격 4
				{
					SetAction(o, MONSTER01_ATTACK3);	// 공격 4임. 주의.
					c->MonsterSkill = -1;	// 스킬 공격과 일반 공격이 있는 경우 스킬 공격 후 초기화 해야됨.
				}
				break;
			default:	// 기타 모든 일반공격 
				{
// 					// FOR TESTDEBUG
// 					int arrSkillAttack[] = { MONSTER01_ATTACK1, MONSTER01_ATTACK2, MONSTER01_ATTACK3 };
// 					int iRandNumCur = rand() % 3;
// 					int iCurrentSkillAttack = arrSkillAttack[iRandNumCur];
// 					
// 					SetAction(o, iCurrentSkillAttack);
// FOR TESTDEBUG
					
					SetAction(o, MONSTER01_ATTACK1);
					c->MonsterSkill = -1;
				}
				break;
			}
		}
		return true;
#endif // LDK_ADD_EG_MONSTER_RIDERMASTER
#ifdef PBG_ADD_RAYMOND_GUARDIANPRIEST_MONSTER_EFFECT
	case 519:			// 치유병
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
#ifdef LDK_ADD_EG_MONSTER_ASSASSINMASTER
	case 516:			// 암살단장
		{
			switch(c->MonsterSkill)
			{
			case 47:	// 공격2
				{
					SetAction(o, MONSTER01_ATTACK2);
					c->MonsterSkill = -1;
				}
				break;
			case 53:	// 공격3
				{
					SetAction(o, MONSTER01_ATTACK3);
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
#endif //LDK_ADD_EG_MONSTER_ASSASSINMASTER
 	}
	
	return false;
}

bool GMEmpireGuardian3::AttackEffectMonster(CHARACTER* c, OBJECT* o, BMD* b)
{
	if(IsEmpireGuardian3() == false)
		return false;

// 	switch(c->MonsterIndex) 
// 	{
// 	}

	return false;
}

// 몬스터 사운드
bool GMEmpireGuardian3::PlayMonsterSound(OBJECT* o) 
{
	if(IsEmpireGuardian3() == false)
		return false;

	//g_EmpireGuardian1에 PlayMonsterSound정보 있음
	if( true == g_EmpireGuardian1.PlayMonsterSound(o) )
	{
		return true;
	}
	
	float fDis_x, fDis_y;
	fDis_x = o->Position[0] - Hero->Object.Position[0];
	fDis_y = o->Position[1] - Hero->Object.Position[1];
	float fDistance = sqrtf(fDis_x*fDis_x+fDis_y*fDis_y);
	
	if (fDistance > 500.0f) 
		return true;

	switch(o->Type)
	{
	case MODEL_MONSTER01+170:// 3군단장 카토
		{
			switch(o->CurrentAction)
			{
			case MONSTER01_WALK:
				{
					PlayBuffer(SOUND_EMPIREGUARDIAN_3CORP_CATO_MOVE);// 3Cato_move.wav
				}
				break;
			case MONSTER01_ATTACK1:
				{
					PlayBuffer(SOUND_EMPIREGUARDIAN_COMBATMASTER_ATTACK01);
				}
				break;
			case MONSTER01_ATTACK2:
			case MONSTER01_ATTACK3:
				{
					PlayBuffer(SOUND_EMPIREGUARDIAN_3CORP_CATO_ATTACK02);// 3Cato_attack2.wav					
				}
				break;
			case MONSTER01_APEAR:
				{
					PlayBuffer(SOUND_EMPIREGUARDIAN_RAYMOND_MONSTER_RAGE);
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

	case MODEL_MONSTER01+176:// 암살단장
		{
			switch(o->CurrentAction)
			{
			case MONSTER01_WALK:
				{
					PlayBuffer(SOUND_EMPIREGUARDIAN_BOSS_GAION_MONSTER_MOVE);
				}
				break;
			case MONSTER01_ATTACK1:
				{
					PlayBuffer(SOUND_EMPIREGUARDIAN_COMBATMASTER_ATTACK01);
				}
				break;
			case MONSTER01_ATTACK2:
			case MONSTER01_ATTACK3:
				{
					PlayBuffer(SOUND_EMPIREGUARDIAN_COMBATMASTER_ATTACK02);
				}
				break;
			case MONSTER01_DIE:
				{
					PlayBuffer(SOUND_EMPIREGUARDIAN_ASSASSINMASTER_DEATH);
				}
				break;
			}
		}
		return true;

	case MODEL_MONSTER01+177:// 기마단장
		{
			switch(o->CurrentAction)
			{
			case MONSTER01_WALK:
				{
					if( 7.0f <= o->AnimationFrame && o->AnimationFrame < 8.0f )
					{
						PlayBuffer(SOUND_EMPIREGUARDIAN_CAVALRYLEADER_MOVE01);
					}
					if( 1.0f <= o->AnimationFrame && o->AnimationFrame < 2.0f )
					{
						PlayBuffer(SOUND_EMPIREGUARDIAN_CAVALRYLEADER_MOVE02);
					}
				}
				break;
			case MONSTER01_ATTACK1:
			case MONSTER01_ATTACK3:
				{
					PlayBuffer(SOUND_EMPIREGUARDIAN_CAVALRYLEADER_ATTACK01);// CavalryLeader_attack1.wav
				}
				break;
			case MONSTER01_ATTACK2:
			case MONSTER01_ATTACK4:
				{
					PlayBuffer(SOUND_EMPIREGUARDIAN_CAVALRYLEADER_ATTACK02);// CavalryLeader_attack2.wav
				}
				break;
			case MONSTER01_DIE:
				{
					PlayBuffer(SOUND_RAKLION_GIANT_MAMUD_DEATH);
				}
				break;
			}
		}
		return true;
	}

	return false; 
}

// 오브젝트 사운드
void GMEmpireGuardian3::PlayObjectSound(OBJECT* o)
{
	g_EmpireGuardian1.PlayObjectSound(o);
}

void GMEmpireGuardian3::PlayBGM()
{
	if (IsEmpireGuardian3())
	{
		PlayMp3(g_lpszMp3[MUSIC_EMPIREGUARDIAN3]);
	}
#ifdef PBG_FIX_STOPBGMSOUND
	else
	{
		StopMp3(g_lpszMp3[MUSIC_EMPIREGUARDIAN3]);
	}
#endif //PBG_FIX_STOPBGMSOUND
}
#endif	// LDK_ADD_MAP_EMPIREGUARDIAN3