// GMEmpireGuardian2.cpp: implementation of the GMEmpireGuardian2 class.
//
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"

#ifdef LDS_ADD_MAP_EMPIREGUARDIAN2

#include "GMEmpireGuardian2.h"
#include "ZzzBMD.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzAI.h"
#include "ZzzEffect.h"
#include "w_MapHeaders.h"
#include "DSPlaySound.h"
#include "ZzzOpenData.h"
#include "ZzzLodTerrain.h"

#include "GOBoid.h"		// 어스퀘이크 스킬

extern char* g_lpszMp3[NUM_MUSIC];

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

GMEmpireGuardian2Ptr GMEmpireGuardian2::Make()
{
	GMEmpireGuardian2Ptr doppelganger(new GMEmpireGuardian2);
	doppelganger->Init();
	return doppelganger;
}

GMEmpireGuardian2::GMEmpireGuardian2()
{

}

GMEmpireGuardian2::~GMEmpireGuardian2()
{
	Destroy();
}

void GMEmpireGuardian2::Init()
{

}

void GMEmpireGuardian2::Destroy()
{

}

bool GMEmpireGuardian2::CreateObject(OBJECT* o)
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

CHARACTER* GMEmpireGuardian2::CreateMonster(int iType, int PosX, int PosY, int Key)
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
#ifdef LDS_ADD_EG_2_MONSTER_2NDCORP_VERMONT		// 제국 수호군 맵 2   (화,  금)몬스터 2군단장 버몬트			(509/169)
	case 509: // 제국 수호군 맵 2   (화,  금)몬스터 2군단장 버몬트			(509/169)
		{
			OpenMonsterModel(169);
			pCharacter = CreateCharacter(Key, MODEL_MONSTER01+169, PosX, PosY);
			strcpy(pCharacter->ID, "버몬트");

			pCharacter->Object.Scale = 1.3f;

			m_bCurrentIsRage_Bermont = false;
		}
		break;
#endif //LDS_ADD_EG_2_MONSTER_2NDCORP_VERMONT
#ifdef LDS_ADD_EG_2_MONSTER_ARTICLECAPTAIN			// 제국 수호군 맵 2   (화,  금)몬스터 기사단장					(514/174)
	case 514: // 제국 수호군 기사단장
		{
			OpenMonsterModel(174);
			pCharacter = CreateCharacter(Key, MODEL_MONSTER01+174, PosX, PosY);
			strcpy(pCharacter->ID, "기사단장");

			pCharacter->Object.Scale = 1.35f;
		}
		break;
#endif //LDS_ADD_EG_2_MONSTER_ARTICLECAPTAIN
#ifdef LDS_ADD_EG_2_MONSTER_GRANDWIZARD			// 제국 수호군 맵 2   (화,  금)몬스터 대마법사					(515/175)
	case 515: // 제국 수호군 맵 2   (화,  금)몬스터 대마법사					(515/175)
		{
			OpenMonsterModel(175);
			pCharacter = CreateCharacter(Key, MODEL_MONSTER01+175, PosX, PosY);
			strcpy(pCharacter->ID, "대마법사");

			OBJECT * pObject = &pCharacter->Object;
			pObject->Scale = 1.3f;
			
			MoveEye(pObject, &Models[pObject->Type], 79, 33);
	   		CreateJoint(BITMAP_JOINT_ENERGY,pObject->Position,pObject->Position,pObject->Angle,22,pObject,30.f);
	  		CreateJoint(BITMAP_JOINT_ENERGY,pObject->Position,pObject->Position,pObject->Angle,23,pObject,30.f);
		}
		break;
#endif //LDS_ADD_EG_2_MONSTER_GRANDWIZARD

	default: return pCharacter;
 	}

	return pCharacter;
}

bool GMEmpireGuardian2::MoveObject(OBJECT* o)
{
	if(IsEmpireGuardian2() == false)
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

bool GMEmpireGuardian2::MoveMonsterVisual(OBJECT* o, BMD* b)
{
	if(IsEmpireGuardian2() == false)
		return false;
	
	//g_EmpireGuardian1에 character정보 있음
	if(true == g_EmpireGuardian1.MoveMonsterVisual(o, b))
	{
		return true;
	}

	return false;
}



// 몬스터(NPC) 프로세서 - 제국 수호군 특화 (Boss gaion이 선택한 케릭터 정보필요로 character* c 인자값 추가.)
bool GMEmpireGuardian2::MoveMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b)
{
 	if(IsEmpireGuardian2() == false)
 		return false;
	
	//g_EmpireGuardian1에 character정보 있음
	if(true == g_EmpireGuardian1.MoveMonsterVisual(c, o, b))
	{
		return true;
	}

	switch(o->Type)
	{
#ifdef LDS_ADD_EG_2_MONSTER_2NDCORP_VERMONT		// 제국 수호군 맵 2   (화,  금)몬스터 2군단장 버몬트			(509/169)
	case MODEL_MONSTER01+169:		// 버몬트	
		{
			vec3_t vPos;
			switch( o->CurrentAction )
			{
			case MONSTER01_STOP1:
			case MONSTER01_STOP2:
				break;
			case MONSTER01_WALK:
				{
					int		iTypeSubType = 0;
					
					Vector(0.4f, 0.4f, 0.4f, o->Light);
					
					if( 6.8f <= o->AnimationFrame && o->AnimationFrame < 7.5f )
					{
						//왼발
						b->TransformByObjectBone( vPos, o, 42 );
						vPos[2] += 25.0f;
						vPos[1] += 0.0f;
						//vPos[0] += 100.0f;
						CreateParticle ( BITMAP_SMOKE, vPos, o->Angle, o->Light, iTypeSubType, 1.0f );
					}
					if( 0.8f <= o->AnimationFrame && o->AnimationFrame < 1.5f )
					{
						//오른발
						b->TransformByObjectBone( vPos, o, 49 );
						vPos[2] += 25.0f;
						vPos[1] += 0.0f;
						//vPos[0] += 100.0f;
						CreateParticle ( BITMAP_SMOKE, vPos, o->Angle, o->Light, iTypeSubType, 1.0f );
					}
				}
				break;
			case MONSTER01_DIE:
				{
// 					// 샘플 이펙트
//  					float Scale = 0.2f;
//  					b->TransformByObjectBone( vPos, o, 30 );
//  					CreateParticle(BITMAP_SMOKE+1, vPos, o->Angle, o->Light, 1, Scale);
//  					
//  					b->TransformByObjectBone( vPos, o, 17 );
//  					CreateParticle(BITMAP_SMOKE+1, vPos, o->Angle, o->Light, 1, Scale);
				}
				break;
			case MONSTER01_ATTACK1:
				{					
					// - 검기 이펙트 정의 부분
					if( 6.0f <= o->AnimationFrame && o->AnimationFrame < 12.0f )
					{
						vec3_t  Light;
						Vector(1.0f, 1.0f, 1.0f, Light);
						
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
							
							b->TransformPosition(BoneTransform[34], StartRelative, StartPos, false);
							b->TransformPosition(BoneTransform[35], EndRelative, EndPos, false);
							CreateBlur(c, StartPos, EndPos, Light, 0);
							
							fAnimationFrame += fSpeedPerFrame;
						}
					}
				}
				break;
			case MONSTER01_ATTACK2:	
				{
					// - 스킬 이펙트 정의 부분 - 어스 퀘이크 스킬 발동 : 되도록 범위 변동 없도록 유의
					if( o->AnimationFrame >= 5.4f && o->AnimationFrame <= 11.0f )
					{
						RenderSkillEarthQuake ( c, o, b, 14 );
					}

					// - 검기 이펙트 정의 부분
					if( 6.0f <= o->AnimationFrame && o->AnimationFrame < 10.0f )
					{
						vec3_t  Light;
						Vector(1.0f, 1.0f, 1.0f, Light);
						
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
							
							b->TransformPosition(BoneTransform[34], StartRelative, StartPos, false);
							b->TransformPosition(BoneTransform[35], EndRelative, EndPos, false);
							CreateBlur(c, StartPos, EndPos, Light, 0);
							
							fAnimationFrame += fSpeedPerFrame;
						}
					}			
				}
				break;
			case MONSTER01_ATTACK3:
				{
					// - 스킬 이펙트 정의 부분 - 파괴의 일격 발동
					if( o->AnimationFrame >= 1.7f && o->AnimationFrame <= 2.3f )
					{
						CHARACTER	*chT = &CharactersClient[c->TargetCharacter];
						vec3_t	&v3TargetPos = chT->Object.Position;
						
						// 이펙트 발생
						// o->Light 에다가 타켓 위치 담아서 이펙트 발생한다.
						Vector(v3TargetPos[0], v3TargetPos[1], v3TargetPos[2], o->Light);						
						CreateEffect(MODEL_EMPIREGUARDIAN_BLOW_OF_DESTRUCTION, 
							o->Position, o->Angle, o->Light, 0, o,-1,0,0,0,1.0f); // 단지 스케일을 위한 공인자값들.
					}

					// - 검기 이펙트 정의 부분
					if( 6.0f <= o->AnimationFrame && o->AnimationFrame < 12.0f )
					{
						vec3_t  Light;
						Vector(1.0f, 1.0f, 1.0f, Light);
						
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
							
							b->TransformPosition(BoneTransform[34], StartRelative, StartPos, false);
							b->TransformPosition(BoneTransform[35], EndRelative, EndPos, false);
							CreateBlur(c, StartPos, EndPos, Light, 0);
							
							fAnimationFrame += fSpeedPerFrame;
						}
					}
				}
				break;
			case MONSTER01_APEAR:		// 광폭화
				{// 광폭화 액션 EFFECT 처리
					vec3_t		Light;
					
					if( m_bCurrentIsRage_Bermont == true )
					{
						Vector ( 1.0f, 1.0f, 1.0f, Light );
						CreateInferno(o->Position);
						
						CreateEffect(MODEL_CIRCLE,o->Position,o->Angle,Light,4,o);
						CreateEffect(MODEL_CIRCLE,o->Position,o->Angle,Light,4,o);	
						m_bCurrentIsRage_Bermont = false;
					}
				}
				break;
			}
		}
		return true;
#endif // LDS_ADD_EG_2_MONSTER_2NDCORP_VERMONT		// 제국 수호군 맵 2   (화,  금)몬스터 2군단장 버몬트			(509/169)
#ifdef LDS_ADD_EG_2_MONSTER_ARTICLECAPTAIN			// 제국 수호군 맵 2   (화,  금)몬스터 기사단장					(514/174)
		case MODEL_MONSTER01+174:		// 기사단장
			{
				vec3_t vPos, vRelative, vLight, v3Temp;
				// 기본 Effect
				// 기본 Effect 1. 어깨 글로우 
				//VectorCopy(o->Position, b->BodyOrigin);.
				
				float fLumi1 = (sinf(WorldTime*0.004f) + 1.f) * 0.25f;
				
				Vector(0.7f+fLumi1, 0.7f+fLumi1, 0.7f+fLumi1, vLight);
				Vector(0.0f, 0.0f, 0.0f, vRelative);
				b->TransformByObjectBone(vPos, o, 21, vRelative);
				CreateSprite(BITMAP_LIGHT, vPos, 2.0f+fLumi1, vLight, o);
				
				b->TransformByObjectBone(vPos, o, 11, vRelative);
				CreateSprite(BITMAP_LIGHT, vPos, 2.0f+fLumi1, vLight, o);
				
				// 기본 Effect 2. 헤드공간 글로우
				Vector(0.4f, 0.5f, 0.7f, vLight);
				b->TransformPosition(o->BoneTransform[8],v3Temp,vPos,true);
				CreateSprite(BITMAP_LIGHTMARKS, vPos, 2.0f, vLight, o);
				// 기본 Effect / 까지
				
				switch( o->CurrentAction )
				{
				case MONSTER01_STOP1:
				case MONSTER01_STOP2:
					{
					}
					break;
				case MONSTER01_WALK:
					{
						int		iTypeSubType = 0;
						
						Vector(0.7f, 0.7f, 0.7f, o->Light);
						
						if( 7.0f <= o->AnimationFrame && o->AnimationFrame < 8.0f )
						{
							//왼발
							b->TransformByObjectBone( vPos, o, 42 );
							vPos[2] += 25.0f;
							vPos[1] += 0.0f;
							//vPos[0] += 100.0f;
							CreateParticle ( BITMAP_SMOKE, vPos, o->Angle, o->Light, iTypeSubType, 2.0f );
						}
						if( 1.0f <= o->AnimationFrame && o->AnimationFrame < 2.0f )
						{
							//오른발
							b->TransformByObjectBone( vPos, o, 47 );
							vPos[2] += 25.0f;
							vPos[1] += 0.0f;
							//vPos[0] += 100.0f;
							CreateParticle ( BITMAP_SMOKE, vPos, o->Angle, o->Light, iTypeSubType, 2.0f );
						}
					}
					break;
				case MONSTER01_DIE:
					{
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
// 						if( o->AnimationFrame >= 6.6f && o->AnimationFrame <= 7.4f )
// 						{
// 							CreateEffect ( MODEL_WAVES, o->Position, o->Angle, o->Light, 1 );
// 							CreateEffect ( MODEL_WAVES, o->Position, o->Angle, o->Light, 1 );
// 							CreateEffect ( MODEL_PIERCING2, o->Position, o->Angle, o->Light );
// 							PlayBuffer ( SOUND_ATTACK_SPEAR );
// 						}
					}
					break;
				case MONSTER01_ATTACK2:	
					{		
						vec3_t	v3PosPiercing, v3AnglePiercing;
						
						if( o->AnimationFrame >= 6.6f && o->AnimationFrame <= 7.4f )
						{
							b->TransformByObjectBone( v3PosPiercing, o, 23 );
							v3PosPiercing[2] = v3PosPiercing[2] - 150.0f;

							VectorCopy(o->Angle, v3AnglePiercing);
							v3AnglePiercing[2] = v3AnglePiercing[2] + 5.0f;

							//CreateEffect ( MODEL_WAVES, v3PosPiercing, v3AnglePiercing, o->Light, 2 );
							//CreateEffect ( MODEL_WAVES, v3PosPiercing, v3AnglePiercing, o->Light, 1 );
							CreateEffect ( MODEL_PIERCING2, v3PosPiercing, v3AnglePiercing,o->Light, 0 );
							
							PlayBuffer ( SOUND_ATTACK_SPEAR );		
						}
					}
					break;
				case MONSTER01_APEAR:
					{
						vec3_t	v3PosPiercing, v3AnglePiercing;
						if( o->AnimationFrame >= 6.6f && o->AnimationFrame <= 7.4f )
						{
							b->TransformByObjectBone( v3PosPiercing, o, 23 );
							v3PosPiercing[2] = v3PosPiercing[2] - 150.0f;

							VectorCopy(o->Angle, v3AnglePiercing);
							v3AnglePiercing[2] = v3AnglePiercing[2] + 5.0f;
							
							CreateEffect ( MODEL_WAVES, v3PosPiercing, v3AnglePiercing, o->Light, 1 );
							CreateEffect ( MODEL_WAVES, v3PosPiercing, v3AnglePiercing, o->Light, 1 );
							CreateEffect ( MODEL_WAVES, v3PosPiercing, v3AnglePiercing, o->Light, 1 );
							CreateEffect ( MODEL_WAVES, v3PosPiercing, v3AnglePiercing, o->Light, 1 );
							CreateEffect ( MODEL_WAVES, v3PosPiercing, v3AnglePiercing, o->Light, 1 );
							CreateEffect ( MODEL_WAVES, v3PosPiercing, v3AnglePiercing, o->Light, 1 );
							
							CreateEffect ( MODEL_PIERCING2, v3PosPiercing, v3AnglePiercing, o->Light );
							CreateEffect ( MODEL_PIERCING2, v3PosPiercing, v3AnglePiercing, o->Light );
							CreateEffect ( MODEL_PIERCING2, v3PosPiercing, v3AnglePiercing, o->Light );
							CreateEffect ( MODEL_PIERCING2, v3PosPiercing, v3AnglePiercing, o->Light );
							CreateEffect ( MODEL_PIERCING2, v3PosPiercing, v3AnglePiercing, o->Light );
							
							PlayBuffer ( SOUND_ATTACK_SPEAR );	
						}
					}
					break;
				}
			}
			return true;
#endif // LDS_ADD_EG_2_MONSTER_ARTICLECAPTAIN			// 제국 수호군 맵 2   (화,  금)몬스터 기사단장					(514/174)
		}
		
		return false;
}


void GMEmpireGuardian2::MoveBlurEffect(CHARACTER* c, OBJECT* o, BMD* b)
{
	switch(o->Type)
	{
	case MODEL_MONSTER01+167:	// 에르카느
	case MODEL_MONSTER01+178:	// 방패병
	case MODEL_MONSTER01+179:	// 치유병
	case MODEL_MONSTER01+180:	// 기사단
	case MODEL_MONSTER01+181:	// 호위병 (검기 이펙트)
		{
			g_EmpireGuardian1.MoveBlurEffect(c, o, b);
		}
		break;
	}
}

bool GMEmpireGuardian2::RenderObjectMesh(OBJECT* o, BMD* b, bool ExtraMon)
{
	if(IsEmpireGuardian2() == false)
		return false;

	//g_EmpireGuardian1에 character정보 있음
	if(true == g_EmpireGuardian1.RenderObjectMesh(o, b, ExtraMon))
	{
		return true;
	}

	switch(o->Type)
	{
	case MODEL_MONSTER01+167:	// 제국 수호군 맵 내에 몬스터 렌더처리
	case MODEL_MONSTER01+169:
	case MODEL_MONSTER01+174:
	case MODEL_MONSTER01+175:
		{
			RenderMonster(o, b, ExtraMon);
			
			return true;
		}		
	}

	return false;
}

bool GMEmpireGuardian2::RenderObjectVisual( OBJECT* o, BMD* b )
{
	if(IsEmpireGuardian2() == false)
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


bool GMEmpireGuardian2::RenderMonster(OBJECT* o, BMD* b, bool ExtraMon)
{
	switch(o->Type)
	{
#ifdef LDS_ADD_EG_2_MONSTER_2NDCORP_VERMONT		// 제국 수호군 맵 2   (화,  금)몬스터 2군단장 버몬트			(509/169)
	case MODEL_MONSTER01+169: // 제국 수호군 맵 2   (화,  금)몬스터 2군단장 버몬트			(509/169)
		{
			b->RenderBody(RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
		}
		return true;
#endif // 제국 수호군 맵 2   (화,  금)몬스터 2군단장 버몬트			(509/169)
#ifdef LDS_ADD_EG_2_MONSTER_ARTICLECAPTAIN			// 제국 수호군 맵 2   (화,  금)몬스터 기사단장					(514/174)
	case MODEL_MONSTER01+174: // 제국 수호군 기사단장
		{
			//b->RenderBody(RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
			
			b->RenderMesh ( 0, RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV);
			//b->RenderMesh ( 0, RENDER_BRIGHT|RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );

			b->RenderMesh ( 1, RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV);
			//b->RenderMesh ( 1, RENDER_BRIGHT|RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );
			
			b->RenderMesh ( 2, RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV);
			b->RenderMesh ( 2, RENDER_BRIGHT|RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );
		}
		return true;
#endif //LDS_ADD_EG_2_MONSTER_ARTICLECAPTAIN
#ifdef LDS_ADD_EG_2_MONSTER_GRANDWIZARD			// 제국 수호군 맵 2   (화,  금)몬스터 대마법사					(515/175)
	case MODEL_MONSTER01+175: // 제국 수호군 맵 2   (화,  금)몬스터 대마법사					(515/175)
		{
			if (o->CurrentAction == MONSTER01_DIE)
			{
				Vector(0.3f,1.0f,0.2f,b->BodyLight);
			}
			b->RenderMesh(0,RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV);
			b->RenderMesh(1,RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV);
			b->RenderMesh(2,RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV);
			b->RenderMesh(3,RENDER_TEXTURE|RENDER_BRIGHT,o->Alpha,3,0.5f,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV);
		}
		return true;
#endif //LDS_ADD_EG_2_MONSTER_GRANDWIZARD
	}
	
	return false;
}


bool GMEmpireGuardian2::RenderMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b)
{
	//g_EmpireGuardian1에 character정보 있음
	if(g_EmpireGuardian1.RenderMonsterVisual(c, o, b))
	{
		return true;
	}

	vec3_t vPos, vLight;
 	
 	switch(o->Type)
 	{
#ifdef LDS_ADD_EG_2_MONSTER_2NDCORP_VERMONT		// 제국 수호군 맵 2   (화,  금)몬스터 2군단장 버몬트			(509/169)
	case MODEL_MONSTER01+169:		// 버몬트
		{
			vec3_t vPos, vRelative, vLight;
			// 기본 Effect
			//VectorCopy(o->Position, b->BodyOrigin);.
			
			float fLumi1 = (sinf(WorldTime*0.004f) + 1.f) * 0.25f;
			
			Vector(0.1f+fLumi1, 0.4f+fLumi1, 0.8f+fLumi1, vLight);
			Vector(0.0f, 0.0f, 0.0f, vRelative);
			b->TransformByObjectBone(vPos, o, 21, vRelative);
			CreateSprite(BITMAP_LIGHT, vPos, 1.5f+fLumi1, vLight, o);
			
			b->TransformByObjectBone(vPos, o, 38, vRelative);
			CreateSprite(BITMAP_LIGHT, vPos, 1.5f+fLumi1, vLight, o);
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
#endif // LDS_ADD_EG_2_MONSTER_2NDCORP_VERMONT		// 제국 수호군 맵 2   (화,  금)몬스터 2군단장 버몬트			(509/169)
#ifdef LDS_ADD_EG_2_MONSTER_GRANDWIZARD			// 제국 수호군 맵 2   (화,  금)몬스터 대마법사					(515/175)
	case MODEL_MONSTER01+175: // 제국 수호군 맵 2   (화,  금)몬스터 대마법사					(515/175)
		{
			int i;
			// 라이트
			float fLumi = (sinf(WorldTime*0.08f) + 1.0f) * 0.5f * 0.3f + 0.7f;
			Vector(0.1f * fLumi, 0.6f * fLumi, 1.0f * fLumi, vLight);

			int iBlueLights[] = { 93, 100, 47, 54 };
			for (i = 0; i < 4; ++i)
			{
				b->TransformByObjectBone(vPos, o, iBlueLights[i]);
				CreateSprite(BITMAP_LIGHT, vPos, 3.0f, vLight, o);
			}

			Vector(0.1f * fLumi, 1.0f * fLumi, 0.2f * fLumi, vLight);
			int iGreenLights[] = { 92, 99, 46, 53, 95, 88, 42, 49 };
			for (i = 0; i < 8; ++i)
			{
				b->TransformByObjectBone(vPos, o, iGreenLights[i]);
				CreateSprite(BITMAP_LIGHT, vPos, 1.0f, vLight, o);
			}

			int iBigGreenLights[] = { 80, 34 };
			for (i = 0; i < 2; ++i)
			{
				b->TransformByObjectBone(vPos, o, iBigGreenLights[i]);
				Vector(0.1f * fLumi, 1.0f * fLumi, 0.3f * fLumi, vLight);
				CreateSprite(BITMAP_LIGHT, vPos, 3.0f, vLight, o);

				// 양손에 파티클
				Vector(0.5f, 0.5f, 0.5f, vLight);
				CreateParticle(BITMAP_CHROME2, vPos, o->Angle, vLight, 0, 0.9f, o);
			}

			MoveEye(o, b, 80, 34);

			if (o->CurrentAction == MONSTER01_ATTACK1 || o->CurrentAction == MONSTER01_ATTACK2)
			{
				for (i = 0; i < 2; ++i)
				{
					// 먼지
					b->TransformByObjectBone(vPos, o, iBigGreenLights[i]);
					Vector(0.3f, 1.0f, 0.8f, vLight);
					CreateParticle ( BITMAP_WATERFALL_4, vPos, o->Angle, vLight, 15, 2.0f );

					// 불똥
					Vector(0.0f, 0.4f, 0.0f, vLight);
					CreateParticle ( BITMAP_SPARK+1, vPos, o->Angle, vLight, 13, 1.0f, o );
					CreateParticle ( BITMAP_SPARK+1, vPos, o->Angle, vLight, 13, 1.0f, o );
				}
			}
			else if(o->CurrentAction == MONSTER01_DIE)
			{
				o->BlendMesh = -2;
				o->m_bRenderShadow = false;
				if(o->AnimationFrame <= 3.0f)
				{
					// 연기 올라오는 이펙트
					Vector(0.1f, 1.0f, 0.2f, vLight);
					for(int i=0; i<5; i++)
					{
						CreateParticle(BITMAP_SMOKE, o->Position, o->Angle, vLight, 39);
					}

					// 인페르노 효과 이펙트
					CreateEffect ( MODEL_SKILL_INFERNO, o->Position, o->Angle, o->Light, 9, o );

					// 구름 회전 시키는 이펙트
					if(o->AnimationFrame <= 0.2f)
					{
						Vector(0.4f, 1.0f, 0.6f, vLight);
						CreateEffect(MODEL_TWINTAIL_EFFECT, o->Position, o->Angle, vLight, 1, o);
						CreateEffect(MODEL_TWINTAIL_EFFECT, o->Position, o->Angle, vLight, 2, o);
					}
				}
			}
		}
		return true;
#endif //LDS_ADD_EG_2_MONSTER_GRANDWIZARD
	default:
		{
		}
		return true;
 	}

	return true;
}

void GMEmpireGuardian2::RenderAfterObjectMesh(OBJECT* o, BMD* b, bool ExtraMon)
{
	if(IsEmpireGuardian2() == false)
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

void GMEmpireGuardian2::SetWeather(int weather)
{
	g_EmpireGuardian1.SetWeather(weather);
}

bool GMEmpireGuardian2::CreateRain( PARTICLE* o )
{
	//비
	return g_EmpireGuardian1.CreateRain(o);
}

void GMEmpireGuardian2::RenderFrontSideVisual()
{
	//안개, 천둥
	g_EmpireGuardian1.RenderFrontSideVisual();
}

bool IsEmpireGuardian2()
{
	if(World == WD_70EMPIREGUARDIAN2)
	{
		return true;
	}

	return false;
}

bool GMEmpireGuardian2::SetCurrentActionMonster(CHARACTER* c, OBJECT* o)
{
	if(IsEmpireGuardian2() == false)
		return false;

	//g_EmpireGuardian1에 SetCurrentActionMonster정보 있음
	if( true == g_EmpireGuardian1.SetCurrentActionMonster(c, o) )
	{
		return true;
	}
	
	switch(c->MonsterIndex) // (기준 인덱스 가이온:504)
	{
#ifdef LDS_ADD_EG_2_MONSTER_2NDCORP_VERMONT		// 제국 수호군 맵 2   (화,  금)몬스터 2군단장 버몬트			(509/169)
	case 509: // 제국 수호군 맵 2   (화,  금)몬스터 2군단장 버몬트			(509/169)
		{
			// 광폭화 서버로부터 신호 이후, 광폭화 비주얼 호출 여부. - 버몬트.
			if( m_bCurrentIsRage_Bermont == true )
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
			case 61:	// Skill:61	// 공격 3
				{
					SetAction(o, MONSTER01_ATTACK3);
					c->MonsterSkill = -1;	// 스킬 공격과 일반 공격이 있는 경우 스킬 공격 후 초기화 해야됨.
				}
				break;	
			case ATMON_SKILL_EMPIREGUARDIAN_BERSERKER:	// 59 광폭화
				{
					SetAction(o, MONSTER01_APEAR);
					c->MonsterSkill = -1;	// 스킬 공격과 일반 공격이 있는 경우 스킬 공격 후 초기화 해야됨.

					m_bCurrentIsRage_Bermont = true;
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
#endif //LDS_ADD_EG_2_MONSTER_2NDCORP_VERMONT
#ifdef LDS_ADD_EG_2_MONSTER_ARTICLECAPTAIN			// 제국 수호군 맵 2   (화,  금)몬스터 기사단장					(514/174)
	case 514: // 제국 수호군 기사단장
		{
			switch(c->MonsterSkill)
			{
			case 47:	// Skill:47	// 공격 2
				{
					SetAction(o, MONSTER01_ATTACK2);
					c->MonsterSkill = -1;	// 스킬 공격과 일반 공격이 있는 경우 스킬 공격 후 초기화 해야됨.
				}
				break;
			case 50:	// Skill:50	// 공격 3
				{
					SetAction(o, MONSTER01_APEAR);
					c->MonsterSkill = -1;	// 스킬 공격과 일반 공격이 있는 경우 스킬 공격 후 초기화 해야됨.
				}
				break;	
			default:	// 기타 모든 일반공격 
				{				
					SetAction(o, MONSTER01_ATTACK1);
					c->MonsterSkill = -1;	// 스킬 공격과 일반 공격이 있는 경우 스킬 공격 후 초기화 해야됨.
				}
				break;
			}
		}
		return true;
#endif //LDS_ADD_EG_2_MONSTER_ARTICLECAPTAIN
#ifdef LDS_ADD_EG_2_MONSTER_GRANDWIZARD			// 제국 수호군 맵 2   (화,  금)몬스터 대마법사					(515/175)
	case 515: // 제국 수호군 맵 2   (화,  금)몬스터 대마법사					(515/175)
		{
			switch(c->MonsterSkill)
			{
			case 51:	// Skill:51	// 공격 2
				{
					SetAction(o, MONSTER01_ATTACK2);
				}
				break;
			case 52:	// Skill:52	// 공격 3
				{
					SetAction(o, MONSTER01_ATTACK2);
				}
				break;	
			default:	// 기타 모든 일반공격 
				{
					SetAction(o, MONSTER01_ATTACK1);
					c->MonsterSkill = -1;	// 스킬 공격과 일반 공격이 있는 경우 스킬 공격 후 초기화 해야됨.
				}
				break;
			}
		}
		return true;
#endif //LDS_ADD_EG_2_MONSTER_GRANDWIZARD
#ifdef LDS_ADD_EG_MONSTER_GUARDIANDEFENDER	// 제국 수호군 맵 1234(모든요일)몬스터 수호군 방패병			(518/178)
	case 518: // 제국 수호군 맵 1234(모든요일)몬스터 수호군 방패병			(518/178)
		{

		}
		return true;
#endif // LDS_ADD_EG_MONSTER_GUARDIANDEFENDER
#ifdef LDS_ADD_EG_MONSTER_GUARDIANPRIEST	// 제국 수호군 맵 1234(모든요일)몬스터 수호군 치유병			(519/179)
	case 519: // 제국 수호군 맵 1234(모든요일)몬스터 수호군 치유병			(519/179)
		{
#ifdef PBG_ADD_RAYMOND_GUARDIANPRIEST_MONSTER_EFFECT
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
#endif //PBG_ADD_RAYMOND_GUARDIANPRIEST_MONSTER_EFFECT
		}
		return true;
#endif // LDS_ADD_EG_MONSTER_GUARDIANPRIEST
	}
	
	return false;
}

bool GMEmpireGuardian2::AttackEffectMonster(CHARACTER* c, OBJECT* o, BMD* b)
{
	if(IsEmpireGuardian2() == false)
		return false;

	switch(c->MonsterIndex) 
	{
#ifdef LDS_ADD_EG_2_MONSTER_GRANDWIZARD			// 제국 수호군 맵 2   (화,  금)몬스터 대마법사					(515/175)
	case 515: // 제국 수호군 맵 2   (화,  금)몬스터 대마법사					(515/175)
		{
			vec3_t vPos;

			// 스킬 이펙트
			if (o->CurrentAction == MONSTER01_ATTACK1)
			{
				// 운석
				if (c->TargetCharacter != -1)
				{
					CHARACTER * pTargetCharacter = &CharactersClient[c->TargetCharacter];
					CreateEffect(MODEL_FIRE,pTargetCharacter->Object.Position,o->Angle,o->Light);
					PlayBuffer(SOUND_METEORITE01);
					c->TargetCharacter = -1;
					c->MonsterSkill = -1;
				}
			}
			else if (c->MonsterSkill == 51)
			{
				// 불기둥
				if (c->TargetCharacter != -1)
				{
					CHARACTER * pTargetCharacter = &CharactersClient[c->TargetCharacter];
					OBJECT * pTargetObject = &pTargetCharacter->Object;
					CreateEffect(BITMAP_FLAME,pTargetObject->Position,pTargetObject->Angle,pTargetObject->Light,0,pTargetObject);
					PlayBuffer(SOUND_FLAME);
					c->MonsterSkill = -1;
				}
			}
			else if (c->MonsterSkill == 52)
			{
				// 광역운석
				Vector(o->Position[0]+rand()%1024-512,o->Position[1]+rand()%1024-512,o->Position[2],vPos);
				CreateEffect(MODEL_FIRE,vPos,o->Angle,o->Light);
				PlayBuffer(SOUND_METEORITE01);
			}
		}
		return true;
#endif //LDS_ADD_EG_2_MONSTER_GRANDWIZARD
	}

	return false;
}

// 몬스터 사운드
bool GMEmpireGuardian2::PlayMonsterSound(OBJECT* o) 
{
	if(IsEmpireGuardian2() == false)
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
#ifdef LDS_ADD_EG_2_MONSTER_2NDCORP_VERMONT		// 제국 수호군 맵 2   (화,  금)몬스터 2군단장 버몬트			(509/169)
	case MODEL_MONSTER01+169:// 2군단장 버몬트
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
					PlayBuffer( SOUND_EMPIREGUARDIAN_2CORP_VERMONT_MONSTER_ATTACK01 );	// 공격1 사운드
				}
				break;
			case MONSTER01_ATTACK2:
				{
					PlayBuffer( SOUND_EMPIREGUARDIAN_2CORP_VERMONT_MONSTER_ATTACK02 );	// 공격2 사운드
				}
				break;
			case MONSTER01_ATTACK3:
				{
					PlayBuffer( SOUND_EMPIREGUARDIAN_2CORP_VERMONT_MONSTER_ATTACK01 );	// 공격3 사운드
				}
				break;
			case MONSTER01_APEAR:
				{
					PlayBuffer( SOUND_EMPIREGUARDIAN_JERINT_MONSTER_RAGE );	// 광폭화 사운드
				}
				break;
			case MONSTER01_DIE:
				{
					PlayBuffer( SOUND_EMPIREGUARDIAN_2CORP_VERMONT_MONSTER_DEATH );	// 죽음 사운드
				}
				break;
			}
		}
		return true;
#endif // LDS_ADD_EG_2_MONSTER_2NDCORP_VERMONT		// 제국 수호군 맵 2   (화,  금)몬스터 2군단장 버몬트			(509/169)
#ifdef LDS_ADD_EG_2_MONSTER_ARTICLECAPTAIN			// 제국 수호군 맵 2   (화,  금)몬스터 기사단장					(514/174)
	case MODEL_MONSTER01+174:		// 기사단장
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
					PlayBuffer( SOUND_EMPIREGUARDIAN_COMBATMASTER_ATTACK03 );	// 공격1 사운드
				}
				break;
			case MONSTER01_ATTACK2:
				{
					PlayBuffer( SOUND_EMPIREGUARDIAN_COMBATMASTER_ATTACK03 );	// 공격2 사운드
				}
				break;
			case MONSTER01_ATTACK3:
				{
					PlayBuffer( SOUND_EMPIREGUARDIAN_COMBATMASTER_ATTACK03 );	// 공격3 사운드
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
#endif // LDS_ADD_EG_2_MONSTER_ARTICLECAPTAIN			// 제국 수호군 맵 2   (화,  금)몬스터 기사단장					(514/174)
#ifdef LDS_ADD_EG_2_MONSTER_GRANDWIZARD			// 제국 수호군 맵 2   (화,  금)몬스터 대마법사					(515/175)
	case MODEL_MONSTER01+175: // 제국 수호군 맵 2   (화,  금)몬스터 대마법사					(515/175)
		{
			switch(o->CurrentAction)
			{
			case MONSTER01_WALK:
				{
					PlayBuffer( SOUND_EMPIREGUARDIAN_BOSS_GAION_MONSTER_MOVE );
				}
				break;
			case MONSTER01_ATTACK1:
				{
					PlayBuffer( SOUND_METEORITE01);	// 공격1 사운드
					PlayBuffer(SOUND_EXPLOTION01);
				}
				break;
			case MONSTER01_ATTACK2:
				{
					PlayBuffer(SOUND_3RD_CHANGE_UP_BG_FIREPILLAR);	// 공격2 사운드	// 재확인
				}
				break;
			case MONSTER01_ATTACK3:
				{
					PlayBuffer(SOUND_EXPLOTION01);	// 공격3 사운드
				}
				break;
			case MONSTER01_DIE:
				{
					PlayBuffer( SOUND_EMPIREGUARDIAN_GRANDWIZARD_DEATH );	// 죽음 사운드
				}
				break;
			}
		}
		return true;
#endif // LDS_ADD_EG_2_MONSTER_GRANDWIZARD			// 제국 수호군 맵 2   (화,  금)몬스터 대마법사					(515/175)
	}

	return false; 
}

// 오브젝트 사운드
void GMEmpireGuardian2::PlayObjectSound(OBJECT* o)
{
	g_EmpireGuardian1.PlayObjectSound(o);
}

void GMEmpireGuardian2::PlayBGM()
{
	if (IsEmpireGuardian2())
	{
		PlayMp3(g_lpszMp3[MUSIC_EMPIREGUARDIAN2]);
	}
#ifdef PBG_FIX_STOPBGMSOUND
	else
	{
		StopMp3(g_lpszMp3[MUSIC_EMPIREGUARDIAN2]);
	}
#endif //PBG_FIX_STOPBGMSOUND
}
#endif	// LDS_ADD_MAP_EMPIREGUARDIAN2