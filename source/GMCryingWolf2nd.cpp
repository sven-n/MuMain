
#include "stdafx.h"

#include "UIWindows.h"
#include "ZzzOpenglUtil.h"
#include "zzztexture.h"
#include "ZzzBMD.h"
#include "zzzlodterrain.h"
#include "ZzzScene.h"
#include "zzzEffect.h"
#include "zzzAi.h"
#include "zzzOpenData.h"

#include "GMCryingWolf2nd.h"
#include "BoneManager.h"

#ifdef CRYINGWOLF_2NDMVP

bool M34CryingWolf2nd::IsCyringWolf2nd()
//pjh : 현재 크라이울프2맵에 있는지 여부.(true = 크라이울프 진행,false = 크라이울프 맵이 아니다)
{ return (World == WD_35CRYWOLF_2ND) ? true : false; }

//. 오브젝트
bool M34CryingWolf2nd::CreateCryingWolf2ndObject(OBJECT* pObject)
{
	if(!IsCyringWolf2nd())
		return false;

	return true;
}
bool M34CryingWolf2nd::MoveCryingWolf2ndObject(OBJECT* pObject)
//pjh : 크라이울프 2차맵에 있는 오브젝트들의 상태.
{
	if(!IsCyringWolf2nd())
		return false;
	
	float Luminosity;
	vec3_t Light;
	
	switch(pObject->Type)
	{
	case 2:	//. 검은연기 박스 : 2005/05/24
	case 5:	//. 연기 박스 : 2005/05/25
		pObject->HiddenMesh = -2;
		break;
	case 3:	//. 빨간불 박스 : 2005/05/24
		Luminosity = (float)(rand()%4+3)*0.1f;
		Vector(Luminosity,Luminosity*0.6f,Luminosity*0.2f,Light);
		AddTerrainLight(pObject->Position[0], pObject->Position[1],Light,3,PrimaryTerrainLight);
		pObject->HiddenMesh = -2;
		break;	
	}
	
	return true;
}

//. 오브젝트 효과를 추가한다.
bool M34CryingWolf2nd::RenderCryingWolf2ndObjectVisual(OBJECT* pObject, BMD* pModel)
//pjh : 크라이울프 맵에 있는 오브젝트들의 효과들을 랜더 한다.
{
	if(!IsCyringWolf2nd())
		return false;

	vec3_t Light;

	switch(pObject->Type)
	{
	case 2:	//. 검은연기 박스 : 2005/05/24
		if ( rand()%3==0 )
		{
			Vector ( 1.f, 1.f, 1.f, Light );
			CreateParticle ( BITMAP_SMOKE, pObject->Position, pObject->Angle, Light, 21 , pObject->Scale);
		}
		break;
	case 3:	//. 빨간불 박스 : 2005/05/24
		if(rand()%2==0) {
			Vector ( 1.f, 1.f, 1.f, Light );
			CreateParticle ( BITMAP_TRUE_FIRE, pObject->Position, pObject->Angle, Light, 5, pObject->Scale);
		}
		break;
	case 5:    //. 연기 박스 : 2005/05/25
		{
			Vector ( 1.f, 1.f, 1.f, Light );
			if(rand()%2==0) {
				if((int)((pObject->Timer++)+2)%4==0)
				{
					CreateParticle ( BITMAP_ADV_SMOKE+1, pObject->Position, pObject->Angle, Light );
					CreateParticle ( BITMAP_ADV_SMOKE, pObject->Position, pObject->Angle, Light, 0 ); //작은놈
				}
			}
			if(rand()%2==0) {
				if((int)(pObject->Timer++)%4==0)
				{
					CreateParticle ( BITMAP_CLOUD, pObject->Position, pObject->Angle, Light, 6 );
					CreateParticle ( BITMAP_ADV_SMOKE, pObject->Position, pObject->Angle, Light, 1 ); //큰놈
				}
			}
		}
		break;
	case 6:    //. 연기 박스2 : 2005/05/25
		{
			Vector ( 1.f, 1.f, 1.f, Light );
			Vector ( 0.2f, 0.2f, 0.2f, Light );

			//CreateParticle ( BITMAP_CLOUD, o->Position, o->Angle, Light, 8, o->Scale);
			if(pObject->HiddenMesh != -2) 
			{
				CreateParticle ( BITMAP_CLOUD, pObject->Position, pObject->Angle, Light, 1, pObject->Scale, pObject);
				CreateParticle ( BITMAP_CLOUD, pObject->Position, pObject->Angle, Light, 2, pObject->Scale, pObject);
				CreateParticle ( BITMAP_CLOUD, pObject->Position, pObject->Angle, Light, 3, pObject->Scale, pObject);
				CreateParticle ( BITMAP_CLOUD, pObject->Position, pObject->Angle, Light, 4, pObject->Scale, pObject);
			}
			pObject->HiddenMesh = -2;
			//}
		}
		break;
	}
	
	return true;
}

bool M34CryingWolf2nd::RenderCryingWolf2ndObjectMesh(OBJECT* pObject, BMD* pModel)
{
	if(!IsCyringWolf2nd())
		return false;

	return RenderCryingWolf2ndMonsterObjectMesh(pObject, pModel);
}

//. 몬스터
CHARACTER* M34CryingWolf2nd::CreateCryingWolf2ndMonster(int iType, int PosX, int PosY, int Key)
//pjh : 크라이울프2차맵에서 출현하는 몬스터들을 생성시켜준다.(몬스터 종류,x포지션,y포지션,식별자)
{
	if(!IsCyringWolf2nd())
		return false;
	CHARACTER* pCharacter = NULL;
	
//#ifdef _DEBUG
//	if(IsCyringWolf2nd())
//		iType = 310;	////pjh : 디버그시 특정몬스터를 갈제로 띄우기위해 사용
//#endif 

	switch(iType)
	{
	case 315:	//. 웨어울프
		{
			OpenMonsterModel(95);
			pCharacter = CreateCharacter(Key,MODEL_MONSTER01+95,PosX,PosY);
			pCharacter->Object.Scale = 1.25f;
			pCharacter->Weapon[0].Type = -1;
			pCharacter->Weapon[1].Type = -1;

			BoneManager::RegisterBone(pCharacter, "Monster95_Head", 6);
		}
		break;
	case 316:		//. 스카우트 원거리
		{
			OpenMonsterModel(96);
			pCharacter = CreateCharacter(Key,MODEL_MONSTER01+96,PosX,PosY);
			pCharacter->Object.Scale = 1.2f;
			pCharacter->Weapon[0].Type = -1;
			pCharacter->Weapon[1].Type = -1;

			BoneManager::RegisterBone(pCharacter, "Monster96_Top", 27);
			BoneManager::RegisterBone(pCharacter, "Monster96_Center", 28);
			BoneManager::RegisterBone(pCharacter, "Monster96_Bottom", 29);
		}
		break;
	case 317:		//. 스카우트 중거리
		{
			OpenMonsterModel(97);
			pCharacter = CreateCharacter(Key,MODEL_MONSTER01+97,PosX,PosY);
			pCharacter->Object.Scale = 1.2f;
			pCharacter->Weapon[0].Type = -1;
			pCharacter->Weapon[1].Type = -1;
		}
		break;
	case 318:		//. 스카우트 근거리
		{
			OpenMonsterModel(98);
			pCharacter = CreateCharacter(Key,MODEL_MONSTER01+98,PosX,PosY);
			pCharacter->Object.Scale = 1.2f;
			pCharacter->Weapon[0].Type = -1;
			pCharacter->Weapon[1].Type = -1;
		}
		break;
	case 310:		//. 발람
		{
			OpenMonsterModel(91);
			pCharacter = CreateCharacter(Key,MODEL_MONSTER01+91,PosX,PosY);
			pCharacter->Object.Scale = 1.25f;
			pCharacter->Weapon[0].Type = -1;
			pCharacter->Weapon[1].Type = -1;
		}
		break;
	}
	
	return pCharacter;
}

bool M34CryingWolf2nd::MoveCryingWolf2ndMonsterVisual(OBJECT* pObject, BMD* pModel)
//pjh : 몬스터들의 몸에 붙어있는 효과들 갱신(공격동작시 효과를 여기다가 쓰기도 했음.).
{
	if(!IsCyringWolf2nd())
		return false;
	switch(pObject->Type)
	{
	case MODEL_MONSTER01+95:	//. 웨어울프
		{
			vec3_t Position, Light;
			
			if(pObject->CurrentAction != MONSTER01_DIE) {
				Vector ( 0.9f, 0.2f, 0.1f, Light );
				BoneManager::GetBonePosition(pObject, "Monster95_Head", Position);
				CreateSprite(BITMAP_LIGHT,Position,3.5f,Light,pObject);
			}

			Vector ( 0.9f, 0.2f, 0.1f, Light );
			//. Walking & Running Scene Processing
			if(pObject->CurrentAction == MONSTER01_WALK || pObject->CurrentAction == MONSTER01_RUN)
			{
				if(rand()%10==0) {
					CreateParticle ( BITMAP_SMOKE+1, pObject->Position, pObject->Angle, Light );
				}
			}
		}
		break;
	case MODEL_MONSTER01+97:	//. 스카우트 중거리
		{
			vec3_t Position, Position2, Light;
			Vector(0.f, 0.f, 0.f, Position);
			
			if(pObject->CurrentAction == MONSTER01_ATTACK2 && pObject->AnimationFrame >= 4.0f && pObject->AnimationFrame <= 6.0f)
			{
				float Matrix[3][4];
				AngleMatrix(pObject->Angle,Matrix);
				VectorRotate(pObject->Direction,Matrix,Position2);
				VectorAdd(Position,Position2,Position);

				Vector ( 1.f, 0.0f, 0.5f, Light );
                CreateEffect ( MODEL_PIERCING2, pObject->Position, pObject->Angle, Light, 1);
			}
		}
		break;
	case MODEL_MONSTER01+96:	//. 스카우트 장거리
		{
			vec3_t Position, Light;

			float Rotation = (float)( rand()%360 );
			float Luminosity = sinf(WorldTime*0.0012f)*0.8f+1.3f;
            
			float fScalePercent = 1.f;
			if(pObject->CurrentAction == MONSTER01_ATTACK1 || pObject->CurrentAction == MONSTER01_ATTACK2)
				fScalePercent = .5f;

			BoneManager::GetBonePosition(pObject, "Monster96_Center", Position);
            Vector ( Luminosity*0.f, Luminosity*0.5f, Luminosity*0.1f, Light );
			CreateSprite(BITMAP_LIGHT,Position,fScalePercent,Light,pObject);

            Vector ( 0.5f, 0.5f, 0.5f, Light );
            
			BoneManager::GetBonePosition(pObject, "Monster96_Top", Position);
			CreateSprite(BITMAP_SHINY+1,Position,0.5f*fScalePercent,Light,pObject, Rotation );
            CreateSprite(BITMAP_SHINY+1,Position,0.5f*fScalePercent,Light,pObject, 360.f-Rotation );

			BoneManager::GetBonePosition(pObject, "Monster96_Bottom", Position);
			CreateSprite(BITMAP_SHINY+1,Position,0.5f*fScalePercent,Light,pObject, Rotation );
            CreateSprite(BITMAP_SHINY+1,Position,0.5f*fScalePercent,Light,pObject, 360.f-Rotation );
		}
		break;
	}
	return false;
}
bool M34CryingWolf2nd::RenderCryingWolf2ndMonsterObjectMesh(OBJECT* pObject, BMD* pModel)
{
	return false;
}

bool M34CryingWolf2nd::RenderCryingWolf2ndMonsterVisual(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel)
{
	return false;
}

void M34CryingWolf2nd::MoveCryingWolf2ndBlurEffect(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel)
//pjh : 몬스터들의 검이나 무기등에 붙어있는 블루어 효과를 찍어준다.
{
	switch(pObject->Type)
	{
	case MODEL_MONSTER01+95:	//. 웨어울프
		{
			if(pObject->CurrentAction == MONSTER01_ATTACK2)
			{
				vec3_t  Light;
				Vector(1.f,1.f,1.f,Light);

				vec3_t StartPos, StartRelative;
				vec3_t EndPos, EndRelative;
				
				float fActionSpeed = pModel->Actions[pObject->CurrentAction].PlaySpeed;
				float fSpeedPerFrame = fActionSpeed/10.f;
				float fAnimationFrame = pObject->AnimationFrame - fActionSpeed;
				for(int i=0; i<10; i++) {
					pModel->Animation(BoneTransform,fAnimationFrame,pObject->PriorAnimationFrame,pObject->PriorAction, pObject->Angle, pObject->HeadAngle);

					Vector(0.f, 0.f, -90.f, StartRelative);
					Vector(0.f, 0.f, 0.f, EndRelative);
					pModel->TransformPosition(BoneTransform[80],StartRelative,StartPos,false);
					pModel->TransformPosition(BoneTransform[80],EndRelative,EndPos,false);

					CreateBlur(pCharacter,StartPos,EndPos,Light,3,true,80);	// 왼쪽 끝

					Vector(0.f, 0.f, 90.f, StartRelative);
					Vector(0.f, 0.f, 0.f, EndRelative);
					pModel->TransformPosition(BoneTransform[82],StartRelative,StartPos,false);
					pModel->TransformPosition(BoneTransform[82],EndRelative,EndPos,false);
					
					CreateBlur(pCharacter,StartPos,EndPos,Light,3,true,84);	// 오른쪽 끝
					
					fAnimationFrame += fSpeedPerFrame;
				}
			}
		}
		break;
	case MODEL_MONSTER01+97:	//. 스카우트 중거리
		{
			if(pObject->CurrentAction == MONSTER01_ATTACK1)
			{
				vec3_t  Light;
				Vector(1.f,1.f,1.f,Light);

				vec3_t StartPos, StartRelative;
				vec3_t EndPos, EndRelative;
				
				float fActionSpeed = pModel->Actions[pObject->CurrentAction].PlaySpeed;
				float fSpeedPerFrame = fActionSpeed/10.f;
				float fAnimationFrame = pObject->AnimationFrame - fActionSpeed;
				for(int i=0; i<10; i++) {
					pModel->Animation(BoneTransform,fAnimationFrame,pObject->PriorAnimationFrame,pObject->PriorAction, pObject->Angle, pObject->HeadAngle);

					Vector(0.f, 0.f, 120.f, StartRelative);
					Vector(0.f, 0.f, 0.f, EndRelative);
					pModel->TransformPosition(BoneTransform[25],StartRelative,StartPos,false);
					pModel->TransformPosition(BoneTransform[25],EndRelative,EndPos,false);

					CreateBlur(pCharacter,StartPos,EndPos,Light,3,true,25);	// 칼날
					
					fAnimationFrame += fSpeedPerFrame;
				}
			}
		}
		break;
	}
}

bool M34CryingWolf2nd::AttackEffectCryingWolf2ndMonster(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel)
//pjh : 몬스터들의 공격동작시 타이밍에 맞게 공격효과 찍어준다(주로 원거리 공격시 사용).
{
	if(!IsCyringWolf2nd())
		return false;
	
	switch(pObject->Type)
	{
	case MODEL_MONSTER01+96:	//. 스타우트 원거리
		{
			if( pCharacter->AttackTime==14 )
			{
				CreateEffect(MODEL_ARROW_NATURE,pObject->Position,pObject->Angle,pObject->Light,1,pObject,pObject->PKKey);
				return true;
			}
		}
		break;
	case MODEL_MONSTER01+91:	//. 발람
		{
			if( pCharacter->AttackTime==14 )
			{
				CreateEffect(MODEL_ARROW_HOLY,pObject->Position,pObject->Angle,pObject->Light,1,pObject,pObject->PKKey);
				return true;
			}
		}
		break;
	}
	return false;
}

#endif // CRYINGWOLF_2NDMVP