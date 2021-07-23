// GMDoppelGanger1.cpp: implementation of the CGMDoppelGanger1 class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#ifdef YDG_ADD_MAP_DOPPELGANGER1

#include "GMDoppelGanger1.h"
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
#include "NewUISystem.h"

extern char* g_lpszMp3[NUM_MUSIC];
extern int GetMp3PlayPosition();

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGMDoppelGanger1Ptr CGMDoppelGanger1::Make()
{
	CGMDoppelGanger1Ptr doppelganger(new CGMDoppelGanger1);
	doppelganger->Init();
	return doppelganger;
}

CGMDoppelGanger1::CGMDoppelGanger1()
{
	m_bIsMP3Playing = FALSE;
}

CGMDoppelGanger1::~CGMDoppelGanger1()
{
	Destroy();
}

void CGMDoppelGanger1::Init()
{

}

void CGMDoppelGanger1::Destroy()
{

}

bool CGMDoppelGanger1::CreateObject(OBJECT* o)
{
// 	switch(o->Type)
// 	{
// 	}

	return false;
}

CHARACTER* CGMDoppelGanger1::CreateMonster(int iType, int PosX, int PosY, int Key)
{
	CHARACTER* pCharacter = NULL;

// 	iType = 529+rand()%10;
// 	iType = 533;

	switch (iType)
	{
	case 529:	// 분노한 도살자
		OpenMonsterModel(190);
		pCharacter = CreateCharacter(Key, MODEL_MONSTER01+190, PosX, PosY);
		strcpy(pCharacter->ID, "도살자");
		pCharacter->Object.Scale = 1.0f;
		break;
	case 530:	// 도살자
		OpenMonsterModel(189);
		pCharacter = CreateCharacter(Key, MODEL_MONSTER01+189, PosX, PosY);
		strcpy(pCharacter->ID, "분노한 도살자");
		pCharacter->Object.Scale = 0.8f;
		break;
	case 531:	// 아이스 워커
		OpenMonsterModel(145);
		pCharacter = CreateCharacter(Key, MODEL_MONSTER01+145, PosX, PosY);
		strcpy(pCharacter->ID, "아이스 워커");
		pCharacter->Object.Scale = 1.2f;
		pCharacter->Weapon[0].Type = -1;
		pCharacter->Weapon[1].Type = -1;
		break;
	case 532:	// 유충
    	OpenMonsterModel(6);
		pCharacter = CreateCharacter(Key, MODEL_MONSTER01+6, PosX, PosY);
		strcpy(pCharacter->ID, "유충");
		pCharacter->Object.Scale = 0.6f;
		break;
	case 533:	// 도플갱어
		OpenMonsterModel(191);
		pCharacter = CreateCharacter(Key, MODEL_MONSTER01+191, PosX, PosY);
		strcpy(pCharacter->ID, "도플갱어");
		pCharacter->Object.Scale = 1.1f;
		pCharacter->Object.m_bRenderShadow = false;
		break;
	case 534:
		pCharacter = CreateCharacter(Key,MODEL_PLAYER,PosX,PosY);
		pCharacter->Object.Scale = 1.0f;
		pCharacter->Class = CLASS_ELF;
		strcpy(pCharacter->ID,"도플갱어요정");
		pCharacter->BodyPart[BODYPART_HELM  ].Type = MODEL_HELM  +13;
		pCharacter->BodyPart[BODYPART_ARMOR ].Type = MODEL_ARMOR +13;
		pCharacter->BodyPart[BODYPART_PANTS ].Type = MODEL_PANTS +13;
		pCharacter->BodyPart[BODYPART_GLOVES].Type = MODEL_GLOVES+13;
		pCharacter->BodyPart[BODYPART_BOOTS ].Type = MODEL_BOOTS +13;
		pCharacter->Weapon[0].Type = MODEL_BOW+15;
		pCharacter->Weapon[1].Type = MODEL_BOW+17;
		break;
	case 535:
		pCharacter = CreateCharacter(Key,MODEL_PLAYER,PosX,PosY);
		pCharacter->Object.Scale = 1.0f;
		pCharacter->Class = CLASS_KNIGHT;
		strcpy(pCharacter->ID,"도플갱어기사");
		pCharacter->BodyPart[BODYPART_HELM  ].Type = MODEL_HELM  +1;
		pCharacter->BodyPart[BODYPART_ARMOR ].Type = MODEL_ARMOR +1;
		pCharacter->BodyPart[BODYPART_PANTS ].Type = MODEL_PANTS +1;
		pCharacter->BodyPart[BODYPART_GLOVES].Type = MODEL_GLOVES+1;
		pCharacter->BodyPart[BODYPART_BOOTS ].Type = MODEL_BOOTS +1;
		pCharacter->Weapon[0].Type = MODEL_SWORD+13;
		pCharacter->Weapon[1].Type = -1;
		break;
	case 536:
		pCharacter = CreateCharacter(Key,MODEL_PLAYER,PosX,PosY);
		pCharacter->Object.Scale = 1.0f;
		pCharacter->Class = CLASS_WIZARD;
		strcpy(pCharacter->ID,"도플갱어마법사");
		pCharacter->BodyPart[BODYPART_HELM  ].Type = MODEL_HELM  +18;
		pCharacter->BodyPart[BODYPART_ARMOR ].Type = MODEL_ARMOR +18;
		pCharacter->BodyPart[BODYPART_PANTS ].Type = MODEL_PANTS +18;
		pCharacter->BodyPart[BODYPART_GLOVES].Type = MODEL_GLOVES+18;
		pCharacter->BodyPart[BODYPART_BOOTS ].Type = MODEL_BOOTS +18;
		pCharacter->Weapon[0].Type = MODEL_STAFF+9;
		pCharacter->Weapon[1].Type = MODEL_SHIELD+15;
		break;
	case 537:
		pCharacter = CreateCharacter(Key,MODEL_PLAYER,PosX,PosY);
		pCharacter->Object.Scale = 1.0f;
		pCharacter->Class = CLASS_DARK;
		strcpy(pCharacter->ID,"도플갱어마검사");
		pCharacter->BodyPart[BODYPART_HELM  ].Type = MODEL_BODY_HELM+15;
		pCharacter->BodyPart[BODYPART_ARMOR ].Type = MODEL_ARMOR +15;
		pCharacter->BodyPart[BODYPART_PANTS ].Type = MODEL_PANTS +15;
		pCharacter->BodyPart[BODYPART_GLOVES].Type = MODEL_GLOVES+15;
		pCharacter->BodyPart[BODYPART_BOOTS ].Type = MODEL_BOOTS +15;
		pCharacter->Weapon[0].Type = MODEL_SWORD+31;
		pCharacter->Weapon[1].Type = -1;
		break;
	case 538:
		pCharacter = CreateCharacter(Key,MODEL_PLAYER,PosX,PosY);
		pCharacter->Object.Scale = 1.0f;
		pCharacter->Class = CLASS_DARK_LORD;
		strcpy(pCharacter->ID,"도플갱어다크로드");
		pCharacter->BodyPart[BODYPART_HELM  ].Type = MODEL_HELM  +27;
		pCharacter->BodyPart[BODYPART_ARMOR ].Type = MODEL_ARMOR +27;
		pCharacter->BodyPart[BODYPART_PANTS ].Type = MODEL_PANTS +27;
		pCharacter->BodyPart[BODYPART_GLOVES].Type = MODEL_GLOVES+27;
		pCharacter->BodyPart[BODYPART_BOOTS ].Type = MODEL_BOOTS +27;
		pCharacter->Weapon[0].Type = MODEL_MACE+10;
		pCharacter->Weapon[1].Type = MODEL_SHIELD+7;
		pCharacter->Helper.Type    = MODEL_HELPER+4;
		CreateBug( MODEL_DARK_HORSE, pCharacter->Object.Position,&pCharacter->Object,1);
		break;
	case 539:
		pCharacter = CreateCharacter(Key,MODEL_PLAYER,PosX,PosY);
		pCharacter->Object.Scale = 1.0f;
		pCharacter->Class = CLASS_SUMMONER;
		strcpy(pCharacter->ID,"도플갱어소환술사");
		pCharacter->BodyPart[BODYPART_HELM  ].Type = MODEL_HELM  +40;
		pCharacter->BodyPart[BODYPART_ARMOR ].Type = MODEL_ARMOR +40;
		pCharacter->BodyPart[BODYPART_PANTS ].Type = MODEL_PANTS +40;
		pCharacter->BodyPart[BODYPART_GLOVES].Type = MODEL_GLOVES+40;
		pCharacter->BodyPart[BODYPART_BOOTS ].Type = MODEL_BOOTS +40;
		pCharacter->Weapon[0].Type = MODEL_STAFF+18;
		//pCharacter->Weapon[1].Type = MODEL_STAFF+22;
		break;
	}

	return pCharacter;
}

bool CGMDoppelGanger1::MoveObject(OBJECT* o)
{
	if(IsDoppelGanger1() == false)
		return false;

	switch(o->Type)
	{
	case 22:
		{
			o->BlendMeshLight = (float)sinf(WorldTime*0.001f)+1.0f;
		}
		return true;
	case 70:	// 리얼파란불
	case 80:
	case 99:	// 하늘색안개
	case 101:	// 하늘색빛박스
		{
			o->HiddenMesh = -2;
		}
		return true;
	}

	return false;
}

bool CGMDoppelGanger1::MoveMonsterVisual(OBJECT* o, BMD* b)
{
	if(IsDoppelGanger1() == false)
		return false;
	
	switch(o->Type)
	{
	case MODEL_MONSTER01+145:	// 아이스워커
		{			
			switch( o->CurrentAction )
			{
			case MONSTER01_ATTACK2:
				if( o->AnimationFrame > 4.4f && o->AnimationFrame < 4.7f )
				{
					//const float OFFSETLEN = 250.0f, POS_HEIGHT = 220.0f;
#ifndef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
					const float OFFSETLEN	= 70.0f;
					const float POS_HEIGHT	= 240.0f;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
					const float ANG_REVISION = 20.0f;
					
					// 위치 보정이 필요한 경우 
					vec3_t v3Pos, v3Ang_, v3BasisPos;
					
					//vec3_t v3Dir_, v3Dir;
					//float  matRotation[3][4];
					
					// a. Bipad Header의 위치 값을 가져온다.
					b->TransformByObjectBone( v3BasisPos, o, 8 );
					
					// b. Position 보정 및 최종 Position 산출 부분..
					VectorCopy( v3BasisPos, v3Pos );
					// b. Position 보정 부분..
					
					// c. 각도 보정 부분.
					VectorCopy( o->Angle, v3Ang_ );
					v3Ang_[0] = v3Ang_[0] + ANG_REVISION;
					// c. 각도 보정 부분.
					
					// d. Effect 호출 부분..
					CreateEffect( MODEL_STREAMOFICEBREATH, v3Pos, o->Angle, o->Light, 0, 0, -1, 0, 0, 0, 0.2f, -1 );	
				}
				break;
			case MONSTER01_DIE:
				{
					vec3_t vPos, vRelative;
					Vector(0.f, 0.f, 0.f, vRelative);
					
					float Scale = 3.5f;
					Vector(1.f, 1.f, 1.f, o->Light);
					b->TransformByObjectBone( vPos, o, 6 );
					CreateParticle(BITMAP_SMOKE, vPos, o->Angle, o->Light, 3, Scale);
					b->TransformByObjectBone( vPos, o, 79 );
					CreateParticle(BITMAP_SMOKE, vPos, o->Angle, o->Light, 53, Scale);
				}
				break;
			}
		}
		return true;
	case MODEL_MONSTER01+189:	// 도살자
	case MODEL_MONSTER01+190:
		{
			if (o->CurrentAction == MONSTER01_STOP1 || o->CurrentAction == MONSTER01_STOP2 || o->CurrentAction == MONSTER01_ATTACK1 || o->CurrentAction == MONSTER01_ATTACK2)
				o->CurrentAction = MONSTER01_WALK;
		}
		return true;
	case MODEL_MONSTER01+191:	// 도플갱어
		if (o->CurrentAction == MONSTER01_APEAR)
		{
			if (o->AnimationFrame > 18.0f)
			{
				if (o->m_bActionStart == FALSE)
				{
					o->m_bActionStart = TRUE;

					vec3_t vPos, vLight;
					int i;

					// 바닥효과
					for (i = 0; i < 6; ++i)
					{
						VectorCopy(o->Position, vPos);
						vPos[0] += (float)(rand()%140-70);
						vPos[1] += (float)(rand()%140-70);
						Vector(0.2f, 1.0f, 0.3f, vLight);
						CreateEffect(BITMAP_CLOUD, vPos, o->Angle, vLight, 0, NULL, -1, 0, 0, 0, 2.0f);
					}

					// 폭발
					Vector(0.4f, 1.0f, 0.6f, vLight);
					for (i = 0; i < 3; ++i)
					{
						vPos[0] = o->Position[0] + (rand()%100-50)*1.0f;
						vPos[1] = o->Position[1] + (rand()%100-50)*1.0f;
						vPos[2] = o->Position[2] + 10.f+(rand()%20)*10.0f;
						CreateParticle(BITMAP_EXPLOTION_MONO,vPos,o->Angle,vLight, 0, (rand()%8+7)*0.1f);
					}

					// 젤리 덩어리
					Vector(0.0f, 0.5f, 0.0f, vLight);
					for (i = 0; i < 15; ++i)
					{
						vPos[0] = o->Position[0] + (rand()%200-100)*1.0f;
						vPos[1] = o->Position[1] + (rand()%200-100)*1.0f;
						vPos[2] = o->Position[2] + (rand()%10)*10.0f;
						CreateEffect(MODEL_DOPPELGANGER_SLIME_CHIP,vPos,o->Angle,vLight,0,o,0,0);
					}

					// 입자
					Vector(0.2f, 0.9f, 0.3f, vLight);
					for (i = 0; i < 30; ++i)
					{
						vPos[0] = o->Position[0] + (rand()%300-150)*1.0f;
						vPos[1] = o->Position[1] + (rand()%300-150)*1.0f;
						vPos[2] = o->Position[2] + 20.0f + (rand()%10)*10.0f;
						CreateParticle(BITMAP_SPARK+1,vPos,o->Angle,vLight,31);
					}
					
					// 연기
					Vector(0.8f, 1.0f, 0.8f, vLight);
					CreateParticle(BITMAP_SMOKE,o->Position,o->Angle,vLight,54,2.8f);

					SetAction(o, MONSTER01_ATTACK1);
				}
			}
		}
#ifndef LDS_MR0_FIX_BREAKVISUAL_DOPPELGANGER_CHARACTER		// !추후 삭제 요망 RenderMonsterVisual로 옮김
 		if (o->m_bActionStart == FALSE)
 		{
 			b->RenderBody(RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
 		}
#endif // LDS_MR0_FIX_BREAKVISUAL_DOPPELGANGER_CHARACTER
		return true;
	}

	return false;
}

void CGMDoppelGanger1::MoveBlurEffect(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel)
{
	switch(pObject->Type)
	{
	case MODEL_MONSTER01+189:	// 도살자
	case MODEL_MONSTER01+190:
		{
			if (!(pObject->CurrentAction == MONSTER01_WALK || pObject->CurrentAction == MONSTER01_ATTACK1 || pObject->CurrentAction == MONSTER01_ATTACK2))
				break;

			vec3_t  vLight;
			Vector(0.6f, 0.4f, 0.2f, vLight);

			vec3_t StartPos, StartRelative;
			vec3_t EndPos, EndRelative;
			
			float fActionSpeed = pModel->Actions[pObject->CurrentAction].PlaySpeed;
			float fSpeedPerFrame = fActionSpeed/10.f;
			float fAnimationFrame = pObject->AnimationFrame - fActionSpeed;
			for(int i=0; i<10; i++) 
			{
				pModel->Animation(BoneTransform, fAnimationFrame, pObject->PriorAnimationFrame, pObject->PriorAction, pObject->Angle, pObject->HeadAngle);

				Vector(0.f, 0.f, 0.f, StartRelative);
				Vector(0.f, 0.f, 0.f, EndRelative);
				pModel->TransformPosition(BoneTransform[33], StartRelative, StartPos, false);
				pModel->TransformPosition(BoneTransform[34], EndRelative, EndPos, false);
				CreateBlur(pCharacter, StartPos, EndPos, vLight, 0, false, 0);
				
				fAnimationFrame += fSpeedPerFrame;
			}
		}
		break;
	}
}

bool CGMDoppelGanger1::RenderObjectMesh(OBJECT* o, BMD* b, bool ExtraMon)
{
	if(IsDoppelGanger1() == false)
		return false;

	float fBlendMeshLight = 0;

	switch(o->Type)
	{
#ifdef YDG_ADD_DOPPELGANGER_PORTAL
	case 19:
	case 20:
	case 31:
	case 33:
		o->m_bRenderAfterCharacter = true;
		return true;
#endif	// YDG_ADD_DOPPELGANGER_PORTAL
	case 76:
		b->RenderMesh(0,RENDER_TEXTURE|RENDER_BRIGHT,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV);
		b->RenderMesh(0,RENDER_TEXTURE|RENDER_BRIGHT,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV);
		return true;
	case 98:
		o->m_bRenderAfterCharacter = true;
		return true;
	case 102:	// 포탈
		b->RenderMesh(1, RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
		b->StreamMesh = 0;
		b->RenderMesh(0, RENDER_TEXTURE|RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, -(int)WorldTime%4000*0.00025f );
		b->StreamMesh = -1;
		return true;
	case MODEL_MONSTER01+145:
		// 몸통
		b->RenderMesh(0,RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV, BITMAP_DOPPELGANGER_ICEWALKER0);
		// 털
		b->RenderMesh(1,RENDER_TEXTURE|RENDER_BRIGHT,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV, BITMAP_DOPPELGANGER_ICEWALKER1);
		return true;
	case MODEL_MONSTER01+6:
		b->RenderMesh(0,RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,BITMAP_DOPPELGANGER_SNAKE01);
		b->RenderMesh(1,RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV);
		return true;
	case MODEL_MONSTER01+189:	// 도살자
		b->RenderMesh(0,RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV);
		b->RenderMesh(1,RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV);
		b->RenderMesh(2,RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV);
		fBlendMeshLight = (sinf(WorldTime*0.003f)+1.0f)*0.5f*0.8f;
		b->RenderMesh(0,RENDER_TEXTURE|RENDER_BRIGHT, o->Alpha, 0, fBlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV, BITMAP_BUGBEAR_R);
		//b->Actions[MONSTER01_WALK].PlaySpeed =		0.34f;
		return true;
	case MODEL_MONSTER01+190:	// 도살자
		b->RenderMesh(0,RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV);
		b->RenderMesh(1,RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV);
		b->RenderMesh(2,RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV);
		fBlendMeshLight = (sinf(WorldTime*0.003f)+1.0f)*0.5f*0.8f;
		b->RenderMesh(0,RENDER_TEXTURE|RENDER_BRIGHT, o->Alpha, 0, fBlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV, BITMAP_BUGBEAR_R);
		return true;
	case MODEL_MONSTER01+191:	// 도플갱어
		return true;
	}
	
	return false;
}

bool CGMDoppelGanger1::RenderObjectVisual( OBJECT* o, BMD* b )
{
	if(IsDoppelGanger1() == false)
		return false;

	switch(o->Type)
	{
	case 70:		// 리얼파란불박스
		{
			vec3_t vLight;
			Vector(0.1f, 0.4f, 1.0f, vLight);
			// 리얼한 불 만들기!!
			switch(rand()%3)
			{
			case 0:
				CreateParticle(BITMAP_FIRE_HIK1_MONO,o->Position,o->Angle,vLight,2,o->Scale);	// 불
				break;
			case 1:
				CreateParticle(BITMAP_FIRE_HIK2_MONO,o->Position,o->Angle,vLight,6,o->Scale);	// 불
				break;
			case 2:
				CreateParticle(BITMAP_FIRE_HIK3_MONO,o->Position,o->Angle,vLight,2,o->Scale);	// 불
				break;
			}
			// 리얼한 불 만들기!!
			CreateParticle(BITMAP_FIRE_HIK1_MONO,o->Position,o->Angle,vLight,2,o->Scale);	// 불
		}
		return true;
	case 80:		// 리얼빨간불박스
		{
			vec3_t vLight;
			Vector(0.7f, 0.2f, 0.1f, vLight);
			// 리얼한 불 만들기!!
			switch(rand()%3)
			{
			case 0:
				CreateParticle(BITMAP_FIRE_HIK1_MONO,o->Position,o->Angle,vLight,2,o->Scale);	// 불
				break;
			case 1:
				CreateParticle(BITMAP_FIRE_HIK2_MONO,o->Position,o->Angle,vLight,6,o->Scale);	// 불
				break;
			case 2:
				CreateParticle(BITMAP_FIRE_HIK3_MONO,o->Position,o->Angle,vLight,2,o->Scale);	// 불
				break;
			}
			// 리얼한 불 만들기!!
			CreateParticle(BITMAP_FIRE_HIK1_MONO,o->Position,o->Angle,vLight,2,o->Scale);	// 불
		}
		return true;
	case 99:	// 하늘색안개
		if (o->HiddenMesh != -2)
		{
			vec3_t Light;
			//Vector(0.02f, 0.03f, 0.04f, Light);
			Vector(0.04f,0.06f,0.08f,Light);	// 푸른빛
			for (int i = 0; i < 10; ++i)
				CreateParticle ( BITMAP_CLOUD, o->Position, o->Angle, Light, 3, o->Scale, o );
				//CreateParticle(BITMAP_CLOUD, o->Position, o->Angle, Light, 0, o->Scale, o);
		}
		return true;
	case 101:	// 하늘색빛박스
		if (rand()%3 == 0)
		{
			vec3_t Light, vPos;
			Vector(0.6f, 0.8f, 1.0f, Light);
			VectorCopy(o->Position, vPos);
			int iScale = o->Scale * 60;
			vPos[0] += rand()%iScale-iScale/2;	
			vPos[1] += rand()%iScale-iScale/2;
			CreateParticle(BITMAP_LIGHT, vPos, o->Angle, Light, 15, o->Scale, o);
		}
		return true;
	}
	
	return false;
}

bool CGMDoppelGanger1::RenderMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b)
{
	vec3_t vPos, vLight;
	
	switch(o->Type)
	{
	case MODEL_MONSTER01+189:	// 도살자
	case MODEL_MONSTER01+190:	// 도살자
		if(rand()%4 == 0)	// 숨연기
		{
			b->TransformByObjectBone(vPos, o, 6);
			vPos[1] += 50.0f;
			
			Vector(1.0f,1.0f,1.0f,vLight);
			CreateParticle(BITMAP_SMOKE, vPos, o->Angle, vLight, 61);
		}
		if(c->Dead==0 && rand()%4==0)	// 발연기
		{
			Vector(o->Position[0]+(float)(rand()%64-32),
				o->Position[1]+(float)(rand()%64-32),
				o->Position[2]+(float)(rand()%32-16),vPos);
    			CreateParticle(BITMAP_SMOKE+1,vPos,o->Angle,vLight, 0);
		}

		if (o->Type == MODEL_MONSTER01+190)
		{
			// 본에 불붙이기
			Vector(1.0f,0.2f,0.1f,vLight);
			for (int j = 0; j < 50; ++j)
			{
				if (j >= 0 && j <= 1) continue;
				if (j >= 12 && j <= 20) continue;
				if (j >= 24 && j <= 32) continue;
				if (j >= 35 && j <= 50) continue;
				b->TransformByObjectBone(vPos, o, j);
				CreateSprite ( BITMAP_LIGHT, vPos, 3.1f, vLight, o );
			}

			float fScale = 0.8f;
			Vector(1.0f,1.0f,1.0f,vLight);
			for (int i = 0; i < 30; ++i)
			{
				b->TransformByObjectBone(vPos, o, rand()%50);
				vPos[0] += rand()%10-20;
				vPos[1] += rand()%10-20;
				vPos[2] += rand()%10-20;

				// 리얼한 불 만들기!!
 				switch(rand()%3)
				{
				case 0:
					CreateParticle(BITMAP_FIRE_HIK1,vPos,o->Angle,vLight,0,fScale);	// 불
					break;
				case 1:
					CreateParticle(BITMAP_FIRE_CURSEDLICH,vPos,o->Angle,vLight,4,fScale);	// 불
					break;
				case 2:
					CreateParticle(BITMAP_FIRE_HIK3,vPos,o->Angle,vLight,0,fScale);	// 불
					break;
				}
			}
		}
		return true;
	case MODEL_MONSTER01+191:	// 도플갱어
#ifdef MR0
		ModelManager::SetTargetObject(o);
#endif //MR0
#ifdef LDS_MR0_FIX_BREAKVISUAL_DOPPELGANGER_CHARACTER
		if (o->m_bActionStart == FALSE)
		{
			b->RenderBody(RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
		}
#endif // LDS_MR0_FIX_BREAKVISUAL_DOPPELGANGER_CHARACTER
		b->RenderMesh(0,RENDER_TEXTURE|RENDER_BRIGHT,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV);
		b->RenderMesh(0,RENDER_CHROME|RENDER_BRIGHT,0.3f,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV);
#ifdef MR0
		ModelManager::SetTargetObject(NULL);
#endif // MR0
		return true;
		break;
	}

	return false;
}

void CGMDoppelGanger1::RenderAfterObjectMesh(OBJECT* o, BMD* b, bool ExtraMon)
{
	if(IsDoppelGanger1() == false)
		return;

	switch(o->Type)
	{
#ifdef YDG_ADD_DOPPELGANGER_PORTAL
	case 19:	// 포탈
	case 20:
	case 31:
		b->RenderBody(RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
		break;
	case 33:	// 포탈 빛
		b->StreamMesh = 0;
		b->RenderMesh(0, RENDER_TEXTURE|RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, (int)WorldTime%10000*0.0001f);
		b->StreamMesh = -1;
		break;
#endif	// YDG_ADD_DOPPELGANGER_PORTAL
	case 98:
		b->RenderMesh ( 0, RENDER_TEXTURE|RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );
		b->RenderMesh ( 0, RENDER_CHROME|RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );
		break;
	default:
		break;
	}
}

bool IsDoppelGanger1()
{
	if(World == WD_65DOPPLEGANGER1)
	{
		return true;
	}

	return false;
}

bool CGMDoppelGanger1::SetCurrentActionMonster(CHARACTER* c, OBJECT* o)
{
// 	if(IsDoppelGanger1() == false)
// 		return false;

	return false;
}

bool CGMDoppelGanger1::AttackEffectMonster(CHARACTER* c, OBJECT* o, BMD* b)
{
// 	if(IsDoppelGanger1() == false)
// 		return false;
	
	return false;
}

// 몬스터 사운드
bool CGMDoppelGanger1::PlayMonsterSound(OBJECT* o) 
{
// 	if(IsDoppelGanger1() == false)
// 		return false;
	
	float fDis_x, fDis_y;
	fDis_x = o->Position[0] - Hero->Object.Position[0];
	fDis_y = o->Position[1] - Hero->Object.Position[1];
	float fDistance = sqrtf(fDis_x*fDis_x+fDis_y*fDis_y);
	
	if (fDistance > 500.0f) 
		return true;

	switch(o->Type)
	{
#ifdef LDS_RAKLION_ADDMONSTER_ICEWALKER
	case MODEL_MONSTER01+145:		// Ice Walker
		if( MONSTER01_ATTACK1 == o->CurrentAction || MONSTER01_ATTACK2 == o->CurrentAction)
		{
			PlayBuffer( SOUND_RAKLION_ICEWALKER_ATTACK );
		}
		else if( MONSTER01_WALK == o->CurrentAction )
		{
			if (rand() % 20 == 0)
			{
				PlayBuffer( SOUND_RAKLION_ICEWALKER_MOVE );
			}
		}
		else if( MONSTER01_DIE == o->CurrentAction )
		{
			{
				PlayBuffer( SOUND_ELBELAND_WOLFHUMAN_DEATH01 );
			}
		}
		return true;
#endif // LDS_RAKLION_ADDMONSTER_ICEWALKER
	case MODEL_MONSTER01+190:		// 분노한 도살자
		{
			if(MONSTER01_ATTACK1 == o->CurrentAction || MONSTER01_ATTACK2 == o->CurrentAction)
			{
				PlayBuffer(SOUND_DOPPELGANGER_RED_BUGBEAR_ATTACK);
			}
			else if(MONSTER01_DIE == o->CurrentAction)
			{
				PlayBuffer(SOUND_DOPPELGANGER_RED_BUGBEAR_DEATH);
			}
		}
		return true;
	case MODEL_MONSTER01+189:		// 도살자
		{
			if(MONSTER01_ATTACK1 == o->CurrentAction || MONSTER01_ATTACK2 == o->CurrentAction)
			{
				PlayBuffer(SOUND_DOPPELGANGER_BUGBEAR_ATTACK);
			}
			else if(MONSTER01_DIE == o->CurrentAction)
			{
				PlayBuffer(SOUND_DOPPELGANGER_BUGBEAR_DEATH);
			}
		}
		return true;
	case MODEL_MONSTER01+191:		// 도플갱어
		{
			if(MONSTER01_APEAR == o->CurrentAction)
			{
				PlayBuffer(SOUND_DOPPELGANGER_SLIME_ATTACK);
			}
			else if(MONSTER01_DIE == o->CurrentAction)
			{
				PlayBuffer(SOUND_DOPPELGANGER_SLIME_DEATH);
			}
		}
		return true;
	}

	return false; 
}

// 오브젝트 사운드
void CGMDoppelGanger1::PlayObjectSound(OBJECT* o)
{

}

void CGMDoppelGanger1::PlayBGM()
{
	if (IsDoppelGanger1() || IsDoppelGanger2() || IsDoppelGanger3() || IsDoppelGanger4())
	{
		if (!g_pDoppelGangerFrame->IsDoppelGangerEnabled())
		{
			StopMp3(g_lpszMp3[MUSIC_DOPPELGANGER]);
			m_bIsMP3Playing = FALSE;
		}
		else
		{
			if (m_bIsMP3Playing == TRUE && GetMp3PlayPosition() == 0)
			{
				StopMp3(g_lpszMp3[MUSIC_DOPPELGANGER]);
				m_bIsMP3Playing = FALSE;
			}
			if (m_bIsMP3Playing == FALSE)
			{
				PlayMp3(g_lpszMp3[MUSIC_DOPPELGANGER]);

				if (GetMp3PlayPosition() > 0)
				{
					m_bIsMP3Playing = TRUE;
				}
			}
		}
	}
}
#endif	// YDG_ADD_MAP_DOPPELGANGER1