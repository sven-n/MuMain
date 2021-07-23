//*****************************************************************************
// File: GM_kanturu_1st.cpp
//
// Desc: 칸투르 1차(외부) 맵, 몬스터.
//
// producer: Ahn Sang-Kyu
//*****************************************************************************
#include "stdafx.h"
#include "ZzzInfomation.h"
#include "ZzzBMD.h"
#include "zzzlodterrain.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzAI.h"
#include "ZzzTexture.h"
#include "UIWindows.h"
#include "ZzzOpenData.h"
#include "ZzzEffect.h"
#include "CDirection.h"

#include "BoneManager.h"
#include "dsplaysound.h"

#include "GM_Kanturu_1st.h"
#include "GMGmArea.h"

bool M40GMArea::IsGmArea()
{
	return (World == WD_40AREA_FOR_GM) ? true : false;
}

using namespace M37Kanturu1st;

bool M37Kanturu1st::IsKanturu1st()
{
	return (World == WD_37KANTURU_1ST) ? true : false;
}

bool M37Kanturu1st::CreateKanturu1stObject(OBJECT* pObject)
{
	if (!IsKanturu1st())
		return false;

	return true;
}

bool M37Kanturu1st::MoveKanturu1stObject(OBJECT* pObject)
{
	if (!(IsKanturu1st() || M40GMArea::IsGmArea()))
		return false;

	vec3_t Light;
	float Luminosity;

	switch (pObject->Type)
	{
	case 59:	// 검은연기 박스.
	case 62:	// 안개 박스.(흰색)
	case 81:
	case 82:
	case 83:
	case 107:
	case 108:
		pObject->HiddenMesh = -2;	// 메시 자체가 안보임.
		break;
	case 44:	// 톱니바퀴.
		pObject->Velocity = 0.02f;	// 애니메이션 속도 조절.
		break;
	case 46:	// 발광바퀴.
		pObject->Velocity = 0.01f;	// 애니메이션 속도 조절.
		// 발광.
		pObject->BlendMeshLight
			= (float)sinf(WorldTime * 0.0015f) * 0.8f + 1.0f;
		::PlayBuffer(SOUND_KANTURU_1ST_BG_WHEEL);
		break;
	case 60:	// 라이트 박스.
		Luminosity = (float)(rand() % 4 + 3) * 0.1f;
		Vector(Luminosity * 0.9f, Luminosity * 0.2f, Luminosity * 0.1f, Light);
		AddTerrainLight(pObject->Position[0], pObject->Position[1], Light, 3,
			PrimaryTerrainLight);
		pObject->HiddenMesh = -2;
		break;
	case 61:	// 빨간불 박스.
		Luminosity = (float)(rand() %4 + 3) * 0.1f;
		Vector(Luminosity, Luminosity * 0.6f, Luminosity * 0.2f, Light);
		AddTerrainLight(pObject->Position[0], pObject->Position[1], Light, 3,
			PrimaryTerrainLight);
		pObject->HiddenMesh = -2;
		break;
	case 70:	// 중심 빨간 구 밖에 3개 띠가 공중에 떠서 돌아가는 오브젝트.
		pObject->Velocity = 0.04f;	// 애니메이션 속도 조절.
		// 지형 라이트 색 변화.
		Luminosity = (float)sinf(WorldTime * 0.002f) * 0.45f + 0.55f;
		Vector(Luminosity * 1.4f, Luminosity * 0.7f, Luminosity * 0.4f, Light);
		AddTerrainLight(pObject->Position[0], pObject->Position[1], Light, 4,
			PrimaryTerrainLight);
		break;
	case 76:	// 빛
		pObject->Alpha = 0.5f;
		break;
	case 77:	// 폭포
		pObject->BlendMeshTexCoordV = -(int)WorldTime % 10000 * 0.0002f;
		::PlayBuffer(SOUND_KANTURU_1ST_BG_WATERFALL);
		break;
	case 90:	// 도는 바퀴.
		pObject->Velocity = 0.04f;	// 애니메이션 속도 조절.
		break;
	case 92:	// 가로 유리관.
		::PlayBuffer(SOUND_KANTURU_1ST_BG_ELEC);
		break;
	case 96:	// 입구 계단 모래
		pObject->Alpha = 0.5f;
		break;
	case 97:
		pObject->HiddenMesh = -2;
		pObject->Timer += 0.1f;
		if (pObject->Timer > 10.f)
			pObject->Timer = 0.f;
		if (pObject->Timer > 5.f)
     		CreateParticle(BITMAP_BUBBLE, pObject->Position, pObject->Angle,
				pObject->Light, 5);
		break;
	case 98:	// 식충 식물.
		::PlayBuffer(SOUND_KANTURU_1ST_BG_PLANT);
		break;
	case 102:	// 칸투르 2차 번쩍이는 바닥 타일.
		pObject->BlendMeshLight = (float)sinf(WorldTime * 0.0010f) + 1.0f;
		break;
	}

	::PlayBuffer(SOUND_KANTURU_1ST_BG_GLOBAL);
	
	return true;
}

// 오브젝트 효과 랜더.
bool M37Kanturu1st::RenderKanturu1stObjectVisual(OBJECT* pObject, BMD* pModel)
{
	if (!(IsKanturu1st() || M40GMArea::IsGmArea()))
		return false;

	vec3_t Light, p, Position;

	switch (pObject->Type)
	{
	case 37:  //  반딧불
		{
			int time = timeGetTime() % 1024;
			if (time >= 0 && time < 10)
			{
				Vector(1.f, 1.f, 1.f, Light);
				CreateEffect(MODEL_BUTTERFLY01, pObject->Position,
					pObject->Angle, Light, 3, pObject);
			}
			pObject->HiddenMesh = -2;	// Hide Object				
		}
		break;
	case 59:	// 검은연기 박스.
		if (rand() % 3 == 0)
		{
			Vector(1.f, 1.f, 1.f, Light);
			CreateParticle(BITMAP_SMOKE, pObject->Position, pObject->Angle,
				Light, 21, pObject->Scale);
		}
		break;
	case 61:	// 빨간불 박스.
		if (rand() % 3 == 0)
		{
			Vector(1.f, 1.f, 1.f, Light);
			CreateParticle(BITMAP_TRUE_FIRE, pObject->Position, pObject->Angle,
				Light, 0, pObject->Scale);
		}
		break;
	case 62:	// 안개 박스.(흰색)
		if (pObject->HiddenMesh != -2)
		{
			Vector(0.04f, 0.04f, 0.04f, Light);
			for (int i = 0; i < 20; ++i)
				CreateParticle(BITMAP_CLOUD, pObject->Position, pObject->Angle,
					Light, 20, pObject->Scale, pObject);
		}
		break;
	case 70:	// 중심 빨간 구 밖에 3개 띠가 공중에 떠서 돌아가는 오브젝트.
		// 광원 색 변화.
		float Luminosity;
		Vector(0.0f, 0.0f, 0.0f, p);
		pModel->TransformPosition(BoneTransform[6], p, Position, false);
		Luminosity = (float)sinf(WorldTime * 0.002f) + 1.8f;
		Vector(0.8f, 0.4f, 0.2f, Light);
		CreateSprite(
			BITMAP_SPARK+1, Position, Luminosity * 7.0f, Light, pObject);
		Vector(0.65f, 0.65f, 0.65f, Light);
		CreateSprite(
			BITMAP_SPARK+1, Position, Luminosity * 4.0f, Light, pObject);
		break;
	case 81:	// 폭포 효과 - 위에서 떨어지는것
		// 색상 변경 시 CreateParticle() 안에서 Light 강제 적용 막아야 함.
		Vector(1.f, 1.f, 1.f, Light);
		if (rand() % 2 == 0)
			CreateParticle(BITMAP_WATERFALL_1, pObject->Position,
				pObject->Angle, Light, 2, pObject->Scale);
		break;
	case 82:	// 폭포 효과 - 물 뛰는 느낌
		Vector(1.f, 1.f, 1.f, Light);
		CreateParticle(BITMAP_WATERFALL_3, pObject->Position,
			pObject->Angle, Light, 4, pObject->Scale);
		break;
	case 83:	//  폭포 효과 - 물 안개 효과
		Vector(1.f, 1.f, 1.f, Light);
		if (rand() % 3 == 0)
			CreateParticle(BITMAP_WATERFALL_2, pObject->Position,
				pObject->Angle, Light, 2, pObject->Scale);
		break;
	case 85:	// 입구.
		// 수정 반짝임.
		pModel->BeginRender(pObject->Alpha);
		pModel->RenderMesh(3, RENDER_TEXTURE | RENDER_BRIGHT, pObject->Alpha,
			pObject->BlendMesh, pObject->BlendMeshLight,
			pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV);
		pModel->RenderMesh(3, RENDER_CHROME | RENDER_DARK, pObject->Alpha,
			pObject->BlendMesh, pObject->BlendMeshLight,
			pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV,
			BITMAP_CHROME);
		pModel->EndRender();
		break;
	case 91:	// 세로 유리관.
		/*
		// 크롬 효과.
		pModel->BeginRender(pObject->Alpha);
		pModel->RenderMesh(0, RENDER_TEXTURE, pObject->Alpha,
			pObject->BlendMesh, pObject->BlendMeshLight,
			pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV);
		pModel->RenderMesh(0, RENDER_CHROME | RENDER_BRIGHT,
			pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight,
			pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV,
			BITMAP_CHROME);
		pModel->EndRender();
		*/
		break;
	case 92:	// 가로 유리관.
		//  번개 효과
		{
			vec3_t EndRelative, StartPos, EndPos;
			Vector(0.f, 0.f, 0.f, EndRelative);
			pModel->TransformPosition(
				BoneTransform[1], EndRelative, StartPos, false);
			pModel->TransformPosition(
				BoneTransform[2], EndRelative, EndPos, false);
			if (rand() % 30 == 1)
				CreateJoint(BITMAP_JOINT_THUNDER, StartPos, EndPos,
					pObject->Angle, 18, NULL, 50.f);
			// 크롬 효과.
		/*	pModel->BeginRender(pObject->Alpha);
			pModel->RenderMesh(0, RENDER_TEXTURE | RENDER_DARK, pObject->Alpha,
				pObject->BlendMesh, pObject->BlendMeshLight,
				pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV);
			pModel->RenderMesh(0, RENDER_CHROME | RENDER_BRIGHT, pObject->Alpha,
				pObject->BlendMesh, pObject->BlendMeshLight,
				pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV,
				BITMAP_CHROME);
			pModel->EndRender();
			*/
		}
		break;
	case 96:	// 입구 계단 모래
		pModel->StreamMesh = 0;
		glAlphaFunc(GL_GREATER,0.0f);	// 0.0f 이상의 알파값을 표현해줌.
		pModel->RenderMesh(
			0, RENDER_TEXTURE, 1.0f, pObject->BlendMesh,
			pObject->BlendMeshLight, pObject->BlendMeshTexCoordU,
			-(int)WorldTime % 20000 * 0.00005f);
		glAlphaFunc(GL_GREATER, 0.25f);	// 0.25f 이상의 알파값을 표현해줌.(원래값)
		pModel->StreamMesh = -1;
		break;
	case 98:	// 거대 식충 식물 물방울 효과.
		if (rand() % 3 == 0)
		{
			vec3_t vPos;
			Vector(0.0f, 0.0f, 0.0f, vPos);
			pModel->TransformPosition(BoneTransform[1], vPos, Position, false);
			Vector(0.5f, 0.6f, 0.1f, Light);
			CreateParticle(
				BITMAP_TWINTAIL_WATER, Position, pObject->Angle, Light, 2);
		}
		break;
	case 105:	// 칸투르 2차 회전하는 수정(?) 오브젝트(8번 오브젝트)와 동일
		{
			float fLumi = (sinf(WorldTime * 0.001f) + 1.0f) * 0.5f;
			vec3_t vPos;
			Vector(0.0f, 0.0f, 0.0f, vPos);
			Vector (fLumi, fLumi, fLumi, Light);
			pModel->TransformPosition(BoneTransform[4], vPos, Position, false);
			CreateParticle(
				BITMAP_ENERGY, Position, pObject->Angle, Light, 0, 1.5f);
			CreateSprite(BITMAP_SPARK+1, Position, 10.0f, Light, pObject);
			vec3_t StartPos, EndPos;
			VectorCopy(Position, StartPos);
			VectorCopy(Position, EndPos);
			StartPos[0] -= 50.f;
			StartPos[0] -= rand() % 100;
			EndPos[0] += rand() % 80;
			StartPos[1] -= rand() % 50;
			EndPos[1] += rand() % 50;
			StartPos[2] += 10.0f;
			EndPos[2] += 10.f;
			if (rand() % 20 == 0)
				CreateJoint(BITMAP_JOINT_THUNDER, StartPos, EndPos,
					pObject->Angle, 8, NULL, 40.f);
		}
		break;
	case 107:	// 흰안개박스2
		if (pObject->HiddenMesh != -2)
		{
			vec3_t Light;
			Vector(0.06f, 0.06f, 0.06f, Light);
			for (int i = 0; i < 20; ++i)
				CreateParticle(BITMAP_CLOUD, pObject->Position, pObject->Angle,
					Light, 2, pObject->Scale, pObject);
		}
		break;
	case 108:	// 검은안개박스1
		if (pObject->HiddenMesh != -2)
		{
			vec3_t Light;
			Vector(0.2f, 0.2f, 0.2f, Light);
			for (int i = 0; i < 20; ++i)
				CreateParticle(BITMAP_CLOUD, pObject->Position, pObject->Angle,
					Light, 7, pObject->Scale, pObject);
		}
		break;
	case 110:	// 칸투르 2차 4번 오브젝트 동일.
		{
			float fLumi = (sinf(WorldTime * 0.002f) + 1.5f) * 0.5f;
			Vector(fLumi * 0.6f, fLumi * 1.0f, fLumi * 0.8f, Light);
			vec3_t vPos;
			Vector(0.0f, 0.0f, 0.0f, vPos);
			pModel->TransformPosition(BoneTransform[1], vPos, Position, false);
			CreateSprite(BITMAP_LIGHT, Position, 1.1f, Light, pObject);
		}
		break;
	}

	return true;
}

// 오브젝트 렌더.
bool M37Kanturu1st::RenderKanturu1stObjectMesh(OBJECT* o, BMD* b, bool ExtraMon)
{
	if (IsKanturu1st() || M40GMArea::IsGmArea())
	{
		switch(o->Type) 
		{
		case 103:	// 칸투르 2차 1번과 동일.
			{
				float fLumi = (sinf(WorldTime * 0.002f) + 1.f) * 0.5f;
				o->HiddenMesh = 1;
				b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh,
					o->BlendMeshLight, o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV, o->HiddenMesh);
				b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, 1, fLumi,
					o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);

				return true;
			}

		// 랜더 순서에 의해 반투명 오류가 나서 맨 나중에 그림.
		//	ZzzObject.cpp의 Draw_RenderObject_AfterCharacter()에서 처리.
		case 76:	// 빛.
		case 77:	// 폭포.
		case 91:	// 세로 유리관.
		case 92:	// 가로 유리관.
		case 95:	// 크리스탈.
		case 105:	// 칸투르 2차 회전하는 수정(?) 오브젝트(8번 오브젝트)와 동일
			o->m_bRenderAfterCharacter = true;
			return true;
		}
	}

	return RenderKanturu1stMonsterObjectMesh(o, b,ExtraMon);
}

void M37Kanturu1st::RenderKanturu1stAfterObjectMesh(OBJECT* o, BMD* b)
{
	if (!(IsKanturu1st() || M40GMArea::IsGmArea()))
		return;

	switch(o->Type) 
	{
	case 76:	// 빛.
		b->BodyLight[0] = 0.52f;
		b->BodyLight[1] = 0.52f;
		b->BodyLight[2] = 0.52f;

		b->StreamMesh = 0;
		b->RenderMesh(
			0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, o->BlendMesh,
			o->BlendMeshLight, -(int)WorldTime % 100000 * 0.00001f,
			o->BlendMeshTexCoordV);
		b->StreamMesh = -1;
		break;

	case 95:	// 크리스탈.
		{
			// 크롬 효과.
			b->BeginRender(o->Alpha);

//			b->BodyLight[0] = 0.4f;
//			b->BodyLight[1] = 0.4f;
//			b->BodyLight[2] = 0.4f;

			b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha,
				o->BlendMesh, o->BlendMeshLight,
				o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
			b->RenderMesh(0, RENDER_CHROME7 | RENDER_DARK, o->Alpha,
				o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV, BITMAP_CHROME);
			b->EndRender();
			// 발광체
			vec3_t Light, p, Position;
			for (int i = 0; i < 10; ++i)
			{
				Vector(0.0f, 0.0f, 0.0f, p);
				b->TransformPosition(BoneTransform[i], p, Position, false);
				Vector(0.1f, 0.1f, 0.3f, Light);
				CreateSprite(BITMAP_SPARK+1, Position, 7.5f, Light, o);
			}
		}
		break;

	case 105:	// 칸투르 2차 회전하는 수정(?) 오브젝트(8번 오브젝트)와 동일
			b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh,
				o->BlendMeshLight, o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV, o->HiddenMesh);
			b->RenderMesh(1, RENDER_CHROME|RENDER_BRIGHT, o->Alpha,
				o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV, BITMAP_CHROME);
		break;

	// 기본 오브젝트만 랜더하는것은 이곳에.
	case 77:	// 폭포.
	case 91:	// 세로 유리관.
	case 92:	// 가로 유리관.
			b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh,
				o->BlendMeshLight, o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV, o->HiddenMesh);
		break;
	}
}

CHARACTER* M37Kanturu1st::CreateKanturu1stMonster(int iType, int PosX, int PosY, int Key)
{
	CHARACTER* pCharacter = NULL;

//	iType = 353;
	switch(iType)
	{
	case 435:
	case 350:	// 버서커
		{
			OpenMonsterModel(106);
		    pCharacter = CreateCharacter(Key, MODEL_MONSTER01+106, PosX, PosY);
		    pCharacter->Object.Scale = 0.95f;
		    pCharacter->Weapon[0].Type = -1;
		    pCharacter->Weapon[1].Type = -1;

			// 본
			BoneManager::RegisterBone(pCharacter, "BERSERK_MOUTH", 9);
		}
		break;
	case 434:
	case 356:	// 기간티스
		{
			OpenMonsterModel(112);
		    pCharacter = CreateCharacter(Key, MODEL_MONSTER01+112, PosX, PosY);
		    pCharacter->Object.Scale = 1.2f;
		    pCharacter->Weapon[0].Type = -1;
		    pCharacter->Weapon[1].Type = -1;
		}
		break;
	case 357:	// 제노사이더
		{
			OpenMonsterModel(113);
		    pCharacter = CreateCharacter(Key, MODEL_MONSTER01+113, PosX, PosY);
		    pCharacter->Object.Scale = 1.2f;
		    pCharacter->Weapon[0].Type = -1;
		    pCharacter->Weapon[1].Type = -1;

			BoneManager::RegisterBone(pCharacter, "GENO_WP", 47);
		}
		break;

	//칸투르 1차 스플린터 울프 & 아이언 라이더
	case 351:
		{
			OpenMonsterModel(107);
			pCharacter = CreateCharacter(Key, MODEL_MONSTER01+107, PosX, PosY);
			pCharacter->Object.Scale = 0.8f;
			pCharacter->Weapon[0].Type = -1;
			pCharacter->Weapon[1].Type = -1;
			//인광 위치
			BoneManager::RegisterBone(pCharacter, "SPL_WOLF_EYE_26", 16);
			BoneManager::RegisterBone(pCharacter, "SPL_WOLF_EYE_25", 17);
			strcpy( pCharacter->ID, "스플린터 울프" );

			OBJECT* o = &pCharacter->Object;
			BMD* b = &Models[o->Type];

			MoveEye(o, b, 16, 17);
			vec3_t vColor = { 1.5f, 0.01f, 0.0f };
   			CreateJoint(BITMAP_JOINT_ENERGY,o->Position,o->Position,o->Angle,24,o,10.f,-1, 0, 0, -1, vColor);
   			CreateJoint(BITMAP_JOINT_ENERGY,o->Position,o->Position,o->Angle,25,o,10.f,-1, 0, 0, -1, vColor);
		}
		break;

	case 352:
		{
			OpenMonsterModel(108);
			pCharacter = CreateCharacter(Key, MODEL_MONSTER01+108, PosX, PosY);
			pCharacter->Object.Scale = 1.3f;
			pCharacter->Weapon[0].Type = -1;
			pCharacter->Weapon[1].Type = -1;
			BoneManager::RegisterBone(pCharacter, "IRON_RIDER_BIP01", 2);
			BoneManager::RegisterBone(pCharacter, "IRON_RIDER_BOW_6", 42);
			BoneManager::RegisterBone(pCharacter, "IRON_RIDER_BOW_15", 52);
			BoneManager::RegisterBone(pCharacter, "IRON_RIDER_BOW_16", 47);
			strcpy( pCharacter->ID, "아이언 라이더" );	
		}
		break;
	case 354:
		{
			OpenMonsterModel(110);
			pCharacter = CreateCharacter(Key, MODEL_MONSTER01+110, PosX, PosY);
			pCharacter->Object.Scale = 1.3f;
			pCharacter->Object.Gravity = 0.0f;
			pCharacter->Object.Distance = (float)(rand()%20)/10.0f;
			pCharacter->Object.Angle[0] = 0.0f;
			pCharacter->Weapon[0].Type = -1;
			pCharacter->Weapon[1].Type = -1;
			BoneManager::RegisterBone(pCharacter, "BLADE_L_HAND", 12);
			BoneManager::RegisterBone(pCharacter, "BOX1", 54);
			BoneManager::RegisterBone(pCharacter, "BOX2", 55);
			strcpy( pCharacter->ID, "블레이드 헌터" );	
		}
		break;
	case 353:
		{
			OpenMonsterModel(109);
			pCharacter = CreateCharacter(Key, MODEL_MONSTER01+109, PosX, PosY);
			pCharacter->Object.Scale = 1.3f;
			pCharacter->Weapon[0].Type = -1;
			pCharacter->Weapon[1].Type = -1;
			strcpy( pCharacter->ID, "사티로스" );	
		}
		break;
	case 355:
		{
			OpenMonsterModel(111);
			pCharacter = CreateCharacter(Key, MODEL_MONSTER01+111, PosX, PosY);
			pCharacter->Object.Scale = 1.1f;
			pCharacter->Weapon[0].Type = -1;
			pCharacter->Weapon[1].Type = -1;
			BoneManager::RegisterBone(pCharacter, "KENTAUROS_BIP_23", 27);
			BoneManager::RegisterBone(pCharacter, "KENTAUROS_BIP_24", 28);
			BoneManager::RegisterBone(pCharacter, "KENTAUROS_BIP_25", 29);
			BoneManager::RegisterBone(pCharacter, "KENTAUROS_BIP_26", 30);
			BoneManager::RegisterBone(pCharacter, "KENTAUROS_BIP_18", 34);
			BoneManager::RegisterBone(pCharacter, "KENTAUROS_BIP_19", 35);
			BoneManager::RegisterBone(pCharacter, "KENTAUROS_BIP_20", 36);
			BoneManager::RegisterBone(pCharacter, "KENTAUROS_BIP_21", 37);
			BoneManager::RegisterBone(pCharacter, "KENTAUROS_BIP_TAIL",81 );
			BoneManager::RegisterBone(pCharacter, "KENTAUROS_BIP_TAIL_1",82 );
			BoneManager::RegisterBone(pCharacter, "KENTAUROS_BIP_TAIL_2", 83);
			BoneManager::RegisterBone(pCharacter, "KENTAUROS_BIP_SPAIN_1",4 );
			BoneManager::RegisterBone(pCharacter, "KENTAUROS_BIP_SPAIN_2",5 );
			BoneManager::RegisterBone(pCharacter, "KENTAUROS_BIP_SPAIN_3", 6);
			strcpy( pCharacter->ID, "켄타우로스" );	
		}
		break;
#ifdef LDS_EXTENSIONMAP_MONSTERS_KANTUR
	case 553:	// 버서커 워리어
		{
			OpenMonsterModel(197);
			pCharacter = CreateCharacter(Key, MODEL_MONSTER01+197, PosX, PosY);
			//pCharacter->Object.Scale = 0.95f;
			pCharacter->Object.Scale = 1.15f;
			pCharacter->Weapon[0].Type = -1;
			pCharacter->Weapon[1].Type = -1;
			
			// 본
			BoneManager::RegisterBone(pCharacter, "BERSERK_MOUTH", 9);
		}
		break;
	case 554:	// 켄타우로스 워리어
		{
			OpenMonsterModel(198);
			pCharacter = CreateCharacter(Key, MODEL_MONSTER01+198, PosX, PosY);
			//pCharacter->Object.Scale = 1.1f;
			pCharacter->Object.Scale = 1.3f;
			pCharacter->Weapon[0].Type = -1;
			pCharacter->Weapon[1].Type = -1;
			BoneManager::RegisterBone(pCharacter, "KENTAUROS_BIP_23", 27);
			BoneManager::RegisterBone(pCharacter, "KENTAUROS_BIP_24", 28);
			BoneManager::RegisterBone(pCharacter, "KENTAUROS_BIP_25", 29);
			BoneManager::RegisterBone(pCharacter, "KENTAUROS_BIP_26", 30);
			BoneManager::RegisterBone(pCharacter, "KENTAUROS_BIP_18", 34);
			BoneManager::RegisterBone(pCharacter, "KENTAUROS_BIP_19", 35);
			BoneManager::RegisterBone(pCharacter, "KENTAUROS_BIP_20", 36);
			BoneManager::RegisterBone(pCharacter, "KENTAUROS_BIP_21", 37);
			BoneManager::RegisterBone(pCharacter, "KENTAUROS_BIP_TAIL",81 );
			BoneManager::RegisterBone(pCharacter, "KENTAUROS_BIP_TAIL_1",82 );
			BoneManager::RegisterBone(pCharacter, "KENTAUROS_BIP_TAIL_2", 83);
			BoneManager::RegisterBone(pCharacter, "KENTAUROS_BIP_SPAIN_1",4 );
			BoneManager::RegisterBone(pCharacter, "KENTAUROS_BIP_SPAIN_2",5 );
			BoneManager::RegisterBone(pCharacter, "KENTAUROS_BIP_SPAIN_3", 6);
			strcpy( pCharacter->ID, "켄타우로스워리어" );	
		}
		break;
	case 555:	// 기간티스 워리어
		{
			OpenMonsterModel(199);
			pCharacter = CreateCharacter(Key, MODEL_MONSTER01+199, PosX, PosY);
			//pCharacter->Object.Scale = 1.2f;
			pCharacter->Object.Scale = 1.5f;
			pCharacter->Weapon[0].Type = -1;
			pCharacter->Weapon[1].Type = -1;
		}
		break;
	case 556:	// 제노사이더 워리어
		{
			OpenMonsterModel(200);
			pCharacter = CreateCharacter(Key, MODEL_MONSTER01+200, PosX, PosY);
			//pCharacter->Object.Scale = 1.2f;
			pCharacter->Object.Scale = 1.35f;
			pCharacter->Weapon[0].Type = -1;
			pCharacter->Weapon[1].Type = -1;
			
			BoneManager::RegisterBone(pCharacter, "GENO_WP", 47);
		}
		break;
#endif // LDS_EXTENSIONMAP_MONSTERS_KANTUR
	}
	
	return pCharacter;
}

bool M37Kanturu1st::SetCurrentActionKanturu1stMonster(CHARACTER* c, OBJECT* o)
{
	if (!IsKanturu1st())
		return false;

	switch (c->MonsterIndex) 
	{
	case 350:	// 버서커
	case 356:	// 기간티스
	case 357:	// 제노사이더
	case 351:	// 스플린터 울프
	case 352:	// 아이언 라이더
		{
			if (rand() % 2 == 0)
				SetAction(o, MONSTER01_ATTACK1);
			else
				SetAction(o, MONSTER01_ATTACK2);

			return true;
		}
		break;
	case 355:	//켄타우로스
		{
			
			if(c->MonsterSkill == ATMON_SKILL_NUM9)
			{
				SetAction ( o, MONSTER01_ATTACK2 );
				c->MonsterSkill = -1;
			}
			else
				SetAction ( o, MONSTER01_ATTACK1 );

		}
		break;
#ifdef LDS_EXTENSIONMAP_MONSTERS_KANTUR
	case 553:	// 버서커 워리어
		{
			if(c->MonsterSkill == ATMON_SKILL_EX_BERSERKERWARRIOR_ATTACKSKILL)
			{
				SetAction ( o, MONSTER01_ATTACK2 );
				c->MonsterSkill = -1;
			}
			else
				SetAction ( o, MONSTER01_ATTACK1 );
			
		}
		break;
	case 554:	//켄타우로스 워리어
		{
			if(c->MonsterSkill == ATMON_SKILL_EX_KENTAURUSWARRIOR_ATTACKSKILL)
			{
				SetAction ( o, MONSTER01_ATTACK2 );
				c->MonsterSkill = -1;
			}
			else
				SetAction ( o, MONSTER01_ATTACK1 );
			
		}
		break;
	case 555:	// 기간티스 워리어
		{
			if (rand() % 2 == 0)
				SetAction(o, MONSTER01_ATTACK1);
			else
				SetAction(o, MONSTER01_ATTACK2);
			
			return true;
		}
		break;
	case 556:	// 제노사이더 워리어
		{
			if(c->MonsterSkill == ATMON_SKILL_EX_GENOSIDEWARRIOR_ATTACKSKILL)
			{
				SetAction ( o, MONSTER01_ATTACK2 );
				c->MonsterSkill = -1;
			}
			else
				SetAction ( o, MONSTER01_ATTACK1 );
			
		}
		break;
#endif // LDS_EXTENSIONMAP_MONSTERS_KANTUR
	}

	return false;
}

bool M37Kanturu1st::AttackEffectKanturu1stMonster(CHARACTER* c, OBJECT* o, BMD* b)
{
	if (!IsKanturu1st())
		return false;
	
	switch(o->Type) 
	{
	case MODEL_MONSTER01+106:	// 버서커
		{
			return true;
		}
		break;
	case MODEL_MONSTER01+112:	// 기간티스
		{
			return true;
		}
		break;
	case MODEL_MONSTER01+113:	// 제노사이더
		{
			return true;
		}
		break;
	
	//스플린터 울프, 아이언 라이더
	case MODEL_MONSTER01+107:
		{
			return true;
		}
		break;
	
	//화살 발사
	case MODEL_MONSTER01+108:
		{
			if(o->CurrentAction == MONSTER01_ATTACK1 || o->CurrentAction == MONSTER01_ATTACK2)
			{
				vec3_t vPos, vRelative;
				vec3_t vLight = { 0.8f, 1.0f, 0.8f };
				Vector(0.f, 0.f, 0.f, vRelative);
				vRelative[0] = (float)(4 - rand() % 5);
				vRelative[1] = (float)(4 - rand() % 5);
				vRelative[2] = (float)(4 - rand() % 5);
				BoneManager::GetBonePosition(o, "IRON_RIDER_BOW_6", vRelative, vPos);
				CreateParticle(BITMAP_SPARK+1, vPos, o->Angle, vLight, 10, 4.0f);	
			}

			if(c->AttackTime == 10)
			{
				vec3_t vPos, vRelative;
				Vector(0.f, 0.f, 0.f, vRelative);
				BoneManager::GetBonePosition(o, "IRON_RIDER_BOW_6", vRelative, vPos);
				CreateEffect(MODEL_IRON_RIDER_ARROW, vPos, o->Angle, o->Light, 0);
			}
			
			return true;
		}
		break;	
	case MODEL_MONSTER01+110:
		{
			if(o->CurrentAction == MONSTER01_ATTACK1 || o->CurrentAction == MONSTER01_ATTACK2)
			{
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
				float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed;
				float StartAction = 3.23f;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
				if(c->AttackTime == 14)
//				if(o->AnimationFrame >= StartAction && o->AnimationFrame < (StartAction + fActionSpeed))
				{
					vec3_t vPos, vRelative,Light;
					Vector(140.f, 0.f, -30.f, vRelative);
					Vector(0.2f, 0.2f, 1.f, Light);
					BoneManager::GetBonePosition(o, "BLADE_L_HAND", vRelative, vPos);
					
					CreateEffect(MODEL_BLADE_SKILL, vPos, o->Angle, Light, 0);

					CreateParticle(BITMAP_EXPLOTION+1,vPos,o->Angle,o->Light, 0, 1.3f);
					CreateParticle(BITMAP_EXPLOTION+1,vPos,o->Angle,o->Light, 0, 2.3f);
					CreateParticle(BITMAP_EXPLOTION+1,vPos,o->Angle,o->Light, 0, 1.8f);
					CreateParticle(BITMAP_EXPLOTION+1,vPos,o->Angle,o->Light, 0, 1.3f);
					CreateParticle(BITMAP_EXPLOTION+1,vPos,o->Angle,o->Light, 0, 2.3f);
					CreateParticle(BITMAP_EXPLOTION+1,vPos,o->Angle,o->Light, 0, 1.8f);
					
				}
			}

			return true;
		}
		break;
	case MODEL_MONSTER01+111://켄타우로스
		{
			if(o->CurrentAction == MONSTER01_ATTACK1 || o->CurrentAction == MONSTER01_ATTACK2)
			{
				vec3_t vLight = { 0.3f, 0.5f, 0.7f };
				vec3_t vPos, vRelative;
				Vector(0.f, 0.f, 0.f, vRelative);
				int index = 31;
				float Width = 2.f;
				BoneManager::GetBonePosition(o, "KENTAUROS_BIP_23", vRelative, vPos);
				CreateParticle(BITMAP_SMOKE, vPos, o->Angle, vLight, index, Width);
				BoneManager::GetBonePosition(o, "KENTAUROS_BIP_24", vRelative, vPos);
				CreateParticle(BITMAP_SMOKE, vPos, o->Angle, vLight, index, Width);
				BoneManager::GetBonePosition(o, "KENTAUROS_BIP_25", vRelative, vPos);
				CreateParticle(BITMAP_SMOKE, vPos, o->Angle, vLight, index, Width);
				BoneManager::GetBonePosition(o, "KENTAUROS_BIP_26", vRelative, vPos);
				CreateParticle(BITMAP_SMOKE, vPos, o->Angle, vLight, index, Width);
				BoneManager::GetBonePosition(o, "KENTAUROS_BIP_18", vRelative, vPos);
				CreateParticle(BITMAP_SMOKE, vPos, o->Angle, vLight, index, Width);
				BoneManager::GetBonePosition(o, "KENTAUROS_BIP_19", vRelative, vPos);
				CreateParticle(BITMAP_SMOKE, vPos, o->Angle, vLight, index, Width);
				BoneManager::GetBonePosition(o, "KENTAUROS_BIP_20", vRelative, vPos);
				CreateParticle(BITMAP_SMOKE, vPos, o->Angle, vLight, index, Width);
				BoneManager::GetBonePosition(o, "KENTAUROS_BIP_21", vRelative, vPos);
				CreateParticle(BITMAP_SMOKE, vPos, o->Angle, vLight, index, Width);
			}
			if(c->AttackTime == 14)
			{
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
				vec3_t vPos, vRelative;
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
				vec3_t vLight = { 0.3f, 0.5f, 0.7f };
				vec3_t vPos, vRelative;
				int index = 31;
				float Width = 2.f;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
				
				if(o->CurrentAction == MONSTER01_ATTACK1)
				{
					Vector(30.f, -30.f, 0.f, vRelative);
					BoneManager::GetBonePosition(o, "KENTAUROS_BIP_25", vRelative, vPos);
					CreateEffect(MODEL_KENTAUROS_ARROW, vPos, o->Angle, o->Light, 0);
				}
				else
				{
					Vector(60.f, -30.f, 50.f, vRelative);
					BoneManager::GetBonePosition(o, "KENTAUROS_BIP_23", vRelative, vPos);
					CreateEffect(MODEL_KENTAUROS_ARROW, vPos, o->Angle, o->Light, 0);
					o->Angle[2] += 15.f;
					CreateEffect(MODEL_KENTAUROS_ARROW, vPos, o->Angle, o->Light, 0);
					o->Angle[2] -= 30.f;
					CreateEffect(MODEL_KENTAUROS_ARROW, vPos, o->Angle, o->Light, 0);
					o->Angle[2] += 15.f;
				}
			}
			
			return true;
		}
		break;
#ifdef LDS_EXTENSIONMAP_MONSTERS_KANTUR
	case MODEL_MONSTER01+197:	// 버서커 워리어
		{
			
			return true;
		}
		break;
	case MODEL_MONSTER01+198:	// 켄타우루스 워리어
		{
			if(o->CurrentAction == MONSTER01_ATTACK1 || o->CurrentAction == MONSTER01_ATTACK2)
			{
				vec3_t vLight = { 0.3f, 0.5f, 0.7f };
				vec3_t vPos, vRelative;
				Vector(0.f, 0.f, 0.f, vRelative);
				int index = 31;
				float Width = 2.f;
				BoneManager::GetBonePosition(o, "KENTAUROS_BIP_23", vRelative, vPos);
				CreateParticle(BITMAP_SMOKE, vPos, o->Angle, vLight, index, Width);
				BoneManager::GetBonePosition(o, "KENTAUROS_BIP_24", vRelative, vPos);
				CreateParticle(BITMAP_SMOKE, vPos, o->Angle, vLight, index, Width);
				BoneManager::GetBonePosition(o, "KENTAUROS_BIP_25", vRelative, vPos);
				CreateParticle(BITMAP_SMOKE, vPos, o->Angle, vLight, index, Width);
				BoneManager::GetBonePosition(o, "KENTAUROS_BIP_26", vRelative, vPos);
				CreateParticle(BITMAP_SMOKE, vPos, o->Angle, vLight, index, Width);
				BoneManager::GetBonePosition(o, "KENTAUROS_BIP_18", vRelative, vPos);
				CreateParticle(BITMAP_SMOKE, vPos, o->Angle, vLight, index, Width);
				BoneManager::GetBonePosition(o, "KENTAUROS_BIP_19", vRelative, vPos);
				CreateParticle(BITMAP_SMOKE, vPos, o->Angle, vLight, index, Width);
				BoneManager::GetBonePosition(o, "KENTAUROS_BIP_20", vRelative, vPos);
				CreateParticle(BITMAP_SMOKE, vPos, o->Angle, vLight, index, Width);
				BoneManager::GetBonePosition(o, "KENTAUROS_BIP_21", vRelative, vPos);
				CreateParticle(BITMAP_SMOKE, vPos, o->Angle, vLight, index, Width);
			}
			if(c->AttackTime == 14)
			{
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING
				vec3_t vPos, vRelative;
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING
				vec3_t vLight = { 0.3f, 0.5f, 0.7f };
				vec3_t vPos, vRelative;
				int index = 31;
				float Width = 2.f;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING
				
				if(o->CurrentAction == MONSTER01_ATTACK1)
				{
					Vector(30.f, -30.f, 0.f, vRelative);
					BoneManager::GetBonePosition(o, "KENTAUROS_BIP_25", vRelative, vPos);
					CreateEffect(MODEL_KENTAUROS_ARROW, vPos, o->Angle, o->Light, 0);
				}
				else
				{
					Vector(60.f, -30.f, 50.f, vRelative);
					BoneManager::GetBonePosition(o, "KENTAUROS_BIP_23", vRelative, vPos);
					CreateEffect(MODEL_KENTAUROS_ARROW, vPos, o->Angle, o->Light, 0);
					o->Angle[2] += 15.f;
					CreateEffect(MODEL_KENTAUROS_ARROW, vPos, o->Angle, o->Light, 0);
					o->Angle[2] -= 30.f;
					CreateEffect(MODEL_KENTAUROS_ARROW, vPos, o->Angle, o->Light, 0);
					o->Angle[2] += 15.f;
				}
			}
			
			return true;
		}
		break;
	case MODEL_MONSTER01+199:	// 기간티스 워리어
		{
			return true;
		}
		break;
	case MODEL_MONSTER01+200:	// 제노사이더 워리어
		{
			return true;
		}
		break;
#endif // LDS_EXTENSIONMAP_MONSTERS_KANTUR
	}

	return false;
}

bool M37Kanturu1st::MoveKanturu1stMonsterVisual(CHARACTER* c,OBJECT* o, BMD* b)
{
	switch (o->Type)
	{
	case MODEL_MONSTER01+106:	// 버서커
		break;
	case MODEL_MONSTER01+112:	// 기간티스
		break;
	case MODEL_MONSTER01+113:	// 제노사이더
		break;
	case MODEL_MONSTER01+111:
		break;
#ifdef LDS_EXTENSIONMAP_MONSTERS_KANTUR
	case MODEL_MONSTER01+197:	// 버서커 워리어
		{
			
		}
		break;
	case MODEL_MONSTER01+198:	// 켄타우로스 워리어
		{
			
		}
		break;
	case MODEL_MONSTER01+199:	// 기간티스 워리어
		{	
			
		}
		break;
	case MODEL_MONSTER01+200:	// 제노사이더 워리어
		{
			
		}
		break;
#endif // LDS_EXTENSIONMAP_MONSTERS_KANTUR
	}
	
	return false;
}

bool M37Kanturu1st::RenderKanturu1stMonsterObjectMesh(OBJECT* o, BMD* b,int ExtraMon)
{
	switch (o->Type)
	{
	case MODEL_MONSTER01+106:	// 버서커
		{
			b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh,
				o->BlendMeshLight, o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV, o->HiddenMesh);

			if (o->CurrentAction == MONSTER01_DIE)
				return true;

			// 점멸 효과
			float fLumi = sinf(WorldTime * 0.002f) + 1.0f;
			b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0,
				fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV,
				BITMAP_BERSERK_EFFECT);	// 몸
			b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 1,
				fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV,
				BITMAP_BERSERK_WP_EFFECT);	// 무기

			return true;
		}
		break;
	case MODEL_MONSTER01+112:	// 기간티스
		{
			// 죽었을 때 뿔만 안그림.
			//b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh,
			//	o->BlendMeshLight, o->BlendMeshTexCoordU,
			//	o->BlendMeshTexCoordV,o->HiddenMesh);
			b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh,
				o->BlendMeshLight, o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV);

			if (o->CurrentAction != MONSTER01_DIE)
				b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha,
					o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
					o->BlendMeshTexCoordV);
			
			b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh,
				o->BlendMeshLight, o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV);

			return true;

			// 뿔 부위 점멸 효과
/*			float fLumi = sinf(WorldTime * 0.002f) + 1.0f;
			b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0,
				fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV,
				BITMAP_GIGANTIS_EFFECT);
			return true;	
			*/
		}
		break;
	case MODEL_MONSTER01+113:	// 제노사이더
		{
			b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh,
				o->BlendMeshLight, o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV, o->HiddenMesh);
			return true;	
		}
		break;

			//스플린터 울프와 아이언 라이더
	case MODEL_MONSTER01+107:
		{
			float fLumi = (sinf(WorldTime*0.002f) + 1.f) * 0.5f;
			Vector(1.f,1.f,1.f,b->BodyLight);
			b->RenderMesh(0, RENDER_TEXTURE);
			b->RenderMesh(1, RENDER_TEXTURE);
			b->RenderMesh(2, RENDER_TEXTURE|RENDER_BRIGHT, o->Alpha, 2, fLumi);
			b->RenderMesh(3, RENDER_TEXTURE|RENDER_BRIGHT, o->Alpha, 3, fLumi);
			b->RenderMesh(4, RENDER_TEXTURE);
			return true;
		}
		break;

	case MODEL_MONSTER01+108:
		{
			b->RenderBody(RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
			return true;
		}
		break;
	case MODEL_MONSTER01+110:
		{
			if(o->CurrentAction != MONSTER01_DIE)
			{
				float fLumi = sinf(WorldTime*0.002f) + 1.0f;
				b->RenderBody(RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
				b->RenderMesh(0, RENDER_TEXTURE|RENDER_BRIGHT, o->Alpha, 0, fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_BLADEHUNTER_EFFECT);	// 몸
			}
			else
			{
//				o->Alpha-=0.1f;
				if(o->Alpha < 0.f)
					o->Alpha = 0.f;
				o->m_bRenderShadow = false;
//				float fLumi = o->Alpha;//sinf(WorldTime*0.002f);// + 1.0f;
				if(o->Alpha > 0.8f)
				{
					b->RenderBody(RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
//				b->RenderMesh(0, RENDER_TEXTURE|RENDER_BRIGHT, o->Alpha, 0, fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_BLADEHUNTER_EFFECT);	// 몸
//				b->RenderMesh(0,RENDER_CHROME|RENDER_BRIGHT,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,BITMAP_CHROME);
				}
				else
				b->RenderBody(RENDER_TEXTURE|RENDER_BRIGHT,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,BITMAP_CHROME);
				b->RenderMesh(0,RENDER_CHROME|RENDER_BRIGHT,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,BITMAP_CHROME);
//				b->RenderMesh ( 0, RENDER_CHROME2, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV ,RENDER_CHROME3);//파란투명
//				b->RenderMesh ( 0, RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );//하얀투명
			}
			return true;
		}
		break;
	case MODEL_MONSTER01+109:
		{
			vec3_t Light;
			VectorCopy(b->BodyLight, Light);
			b->BeginRender(1.f);
			b->RenderBody(RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
			b->StreamMesh = 1;
			b->RenderMesh(1,RENDER_TEXTURE|RENDER_BRIGHT,1.f,0,o->BlendMeshLight,o->BlendMeshTexCoordU,WorldTime*0.0005f);
			b->RenderMesh(3,RENDER_CHROME|RENDER_BRIGHT, 1.f, 0,o->BlendMeshLight,o->BlendMeshTexCoordU,WorldTime*0.001f,BITMAP_CHROME);

			float Luminosity = sinf( WorldTime*0.001f )*0.5f+0.5f;
	        Vector(Light[0]*Luminosity,Light[0]*Luminosity,Light[0]*Luminosity,b->BodyLight);
			b->RenderMesh(4,RENDER_TEXTURE|RENDER_BRIGHT,1.f,2,o->BlendMeshLight,WorldTime*0.0001f,-WorldTime*0.0005f);
			b->EndRender();
			return true;
		}
		break;
	case MODEL_MONSTER01+111:
		{
			float fLumi = (sinf(WorldTime*0.002f) + 1.f) * 0.5f;
			Vector(1.f,1.f,1.f,b->BodyLight);
			b->RenderBody(RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
			b->RenderMesh(1, RENDER_TEXTURE|RENDER_BRIGHT, o->Alpha, 1, fLumi);
			b->RenderMesh ( 1, RENDER_CHROME2|RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );
			return true;
		}
		break;
#ifdef LDS_EXTENSIONMAP_MONSTERS_KANTUR
	case MODEL_MONSTER01+197:	// 버서커 워리어
		{
			b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh,
				o->BlendMeshLight, o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV, o->HiddenMesh);
			
			if (o->CurrentAction == MONSTER01_DIE)
				return true;
			
			// 점멸 효과
			float fLumi = sinf(WorldTime * 0.002f) + 1.0f;
			b->RenderMesh(0, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 0,
				fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV,
				BITMAP_BERSERK_EFFECT);	// 몸
			b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha, 1,
				fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV,
				BITMAP_BERSERK_WP_EFFECT);	// 무기
			
			return true;
		}
		break;
	case MODEL_MONSTER01+198:	// 켄타우루스 워리어
		{
			float fLumi = (sinf(WorldTime*0.002f) + 1.f) * 0.5f;
			Vector(1.f,1.f,1.f,b->BodyLight);
			b->RenderBody(RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
			b->RenderMesh(1, RENDER_TEXTURE|RENDER_BRIGHT, o->Alpha, 1, fLumi);
			b->RenderMesh ( 1, RENDER_CHROME2|RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV );
			return true;
		}
		break;
	case MODEL_MONSTER01+199:	// 기간티스 워리어
		{
			b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh,
				o->BlendMeshLight, o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV);
			
			if (o->CurrentAction != MONSTER01_DIE)
				b->RenderMesh(1, RENDER_TEXTURE | RENDER_BRIGHT, o->Alpha,
				o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV);
			
			b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh,
				o->BlendMeshLight, o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV);
			
			return true;
		}
		break;
	case MODEL_MONSTER01+200:	// 제노사이더 워리어
		{
			b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh,
				o->BlendMeshLight, o->BlendMeshTexCoordU,
				o->BlendMeshTexCoordV, o->HiddenMesh);
			return true;	
		}
		break;
#endif // LDS_EXTENSIONMAP_MONSTERS_KANTUR
	}

	return false;
}

bool M37Kanturu1st::RenderKanturu1stMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b)
{
	switch (o->Type)
	{
	case MODEL_MONSTER01+106:	// 버서커
		{
		// 사운드
			if (o->CurrentAction == MONSTER01_WALK)
			{
				if (rand() % 15 == 0)
					PlayBuffer(SOUND_KANTURU_1ST_BER_MOVE1 + rand() % 2);
			}
			else if (o->CurrentAction == MONSTER01_ATTACK1 || o->CurrentAction == MONSTER01_ATTACK2)
			{
				if (o->SubType == FALSE)
				{
					o->SubType = TRUE;
					PlayBuffer(SOUND_KANTURU_1ST_BER_ATTACK1 + rand() % 2);
				}
			}
			else if(o->CurrentAction == MONSTER01_DIE)
			{
				if (o->SubType == FALSE)
				{
					o->SubType = TRUE;
					PlayBuffer(SOUND_KANTURU_1ST_BER_DIE);
				}
			}

			if (o->CurrentAction == MONSTER01_STOP1 || o->CurrentAction == MONSTER01_STOP2)
				o->SubType = FALSE;

			// 죽었거나 공격시 파티클 생성 안함.
			if (o->CurrentAction == MONSTER01_DIE
				|| o->CurrentAction == MONSTER01_ATTACK1
				|| o->CurrentAction == MONSTER01_ATTACK2)
				return true;

			// 입에서 나오는 입김.
			vec3_t vPos, vRelative;
			Vector(0.f, 0.f, 0.f, vRelative);
			BoneManager::GetBonePosition(o, "BERSERK_MOUTH", vRelative, vPos);
			CreateParticle(
				BITMAP_SMOKE, vPos, o->Angle, o->Light, 42, o->Scale);

			return true;
		}
		break;

	case MODEL_MONSTER01+112:	// 기간티스
		{
		// 사운드
			if (o->CurrentAction == MONSTER01_WALK)
			{
				if (rand() % 15 == 0)
					PlayBuffer(SOUND_KANTURU_1ST_GIGAN_MOVE1);
			}
			else if (o->CurrentAction == MONSTER01_ATTACK1 || o->CurrentAction == MONSTER01_ATTACK2)
			{
				if (o->SubType == FALSE)
				{
					o->SubType = TRUE;
					PlayBuffer(SOUND_KANTURU_1ST_GIGAN_ATTACK1 + rand() % 2);
				}
			}
			else if(o->CurrentAction == MONSTER01_DIE)
			{
				if (o->SubType == FALSE)
				{
					o->SubType = TRUE;
					PlayBuffer(SOUND_KANTURU_1ST_GIGAN_DIE);
				}
			}

			if (o->CurrentAction == MONSTER01_STOP1 || o->CurrentAction == MONSTER01_STOP2)
				o->SubType = FALSE;

		// 뿔 발광 효과.
			if (o->CurrentAction == MONSTER01_DIE)
				return true;

			vec3_t EndRelative, EndPos;
			Vector(19.f, -2.f, 0.f, EndRelative);

			b->TransformPosition(
				o->BoneTransform[7], EndRelative, EndPos, true);

			Vector(0.4f, 0.6f, 0.8f, o->Light);
			CreateSprite(BITMAP_LIGHT, EndPos, 3.0f, o->Light, o, 0.5f);

			float Luminosity;
			Luminosity = sinf(WorldTime * 0.05f) * 0.4f + 0.9f;
			Vector(Luminosity * 0.3f, Luminosity * 0.5f, Luminosity * 0.8f,
				o->Light);
			CreateSprite(BITMAP_LIGHT, EndPos, 1.0f, o->Light, o);

			return true;	
		}
		break;

	case MODEL_MONSTER01+113:	// 제노사이더
		{
		// 사운드
			if (o->CurrentAction == MONSTER01_WALK)
			{
				if (rand() % 15 == 0)
					PlayBuffer(SOUND_KANTURU_1ST_GENO_MOVE1 + rand() % 2);
			}
			else if (o->CurrentAction == MONSTER01_ATTACK1 || o->CurrentAction == MONSTER01_ATTACK2)
			{
				if (o->SubType == FALSE)
				{
					o->SubType = TRUE;
					PlayBuffer(SOUND_KANTURU_1ST_GENO_ATTACK1 + rand() % 2);
				}
			}
			else if(o->CurrentAction == MONSTER01_DIE)
			{
				if (o->SubType == FALSE)
				{
					o->SubType = TRUE;
					PlayBuffer(SOUND_KANTURU_1ST_GENO_DIE);
				}
			}

			if (o->CurrentAction == MONSTER01_STOP1 || o->CurrentAction == MONSTER01_STOP2)
				o->SubType = FALSE;

			// 죽었거나 공격시 파티클 생성 안함.
			if (o->CurrentAction == MONSTER01_WALK)
			{
				// 철퇴 끝에서 연기.
				vec3_t vPos, vRelative;
				Vector(0.f, 0.f, 0.f, vRelative);
				BoneManager::GetBonePosition(o, "GENO_WP", vRelative, vPos);
				CreateParticle(
					BITMAP_SMOKE+1, vPos, o->Angle, o->Light, 1, o->Scale);

				CreateEffect(MODEL_STONE1+rand()%2, vPos, o->Angle, o->Light);
			}
			return true;	
		}
		break;

	//스플린트 울프 & 아이언 라이더
	case MODEL_MONSTER01+107:
		{
			MoveEye(o, b, 16, 17);
	
			if (o->CurrentAction == MONSTER01_WALK)
			{
				if (rand() % 15 == 0)
					PlayBuffer(SOUND_KANTURU_1ST_SWOLF_MOVE1 + rand() % 2);
			}
			else if (o->CurrentAction == MONSTER01_ATTACK1 || o->CurrentAction == MONSTER01_ATTACK2)
			{
				if (o->SubType == FALSE)
				{
					o->SubType = TRUE;
					PlayBuffer(SOUND_KANTURU_1ST_SWOLF_ATTACK1 + rand() % 2);
				}
			}
			else if(o->CurrentAction == MONSTER01_DIE)
			{
				if (o->SubType == FALSE)
				{
					o->SubType = TRUE;
					PlayBuffer(SOUND_KANTURU_1ST_SWOLF_DIE);
				}
			}
			return true;
		}
		break;
	case MODEL_MONSTER01+108:
		{
			if(o->CurrentAction != MONSTER01_DIE)
			{
				//활에서 반짝반짝
				vec3_t vPos, vRelative;
				vec3_t vLight = { 0.8f, 1.0f, 0.8f };
				Vector(0.f, 0.f, 0.f, vRelative);
				vRelative[0] = (float)(4 - rand() % 5);
				vRelative[1] = (float)(4 - rand() % 5);
				vRelative[2] = (float)(4 - rand() % 5);
				BoneManager::GetBonePosition(o, "IRON_RIDER_BOW_15", vRelative, vPos);
				CreateParticle(BITMAP_SPARK+1, vPos, o->Angle, vLight, 10, 4.0f);
				BoneManager::GetBonePosition(o, "IRON_RIDER_BOW_16", vRelative, vPos);
				CreateParticle(BITMAP_SPARK+1, vPos, o->Angle, vLight, 10, 4.0f);

				if (o->CurrentAction == MONSTER01_WALK)
				{
					if (rand() % 15 == 0)
						PlayBuffer(SOUND_KANTURU_1ST_IR_MOVE1 + rand() % 2);
				}
				else if (o->CurrentAction == MONSTER01_ATTACK1 || o->CurrentAction == MONSTER01_ATTACK2)
				{
					if (o->SubType == FALSE)
					{
						o->SubType = TRUE;
						PlayBuffer(SOUND_KANTURU_1ST_IR_ATTACK1 + rand() % 2);
					}
				}
			}
			else
			{
				//죽었을 때 검은 연기 연출
				vec3_t vLight = { 1.0f, 1.0f, 1.0f };
				vec3_t vPos, vRelative;
				Vector(0.f, 0.f, 0.f, vRelative);
				BoneManager::GetBonePosition(o, "IRON_RIDER_BIP01", vRelative, vPos);
				CreateParticle(BITMAP_SMOKE+3, vPos, o->Angle, vLight, 3, 2.0f);
				CreateParticle(BITMAP_SMOKE+3, vPos, o->Angle, vLight, 4, 1.0f);
				if (o->SubType == FALSE)
				{
					o->SubType = TRUE;
					PlayBuffer(SOUND_KANTURU_1ST_IR_DIE);
				}				
			}
			return true;
		}
		break;
	case MODEL_MONSTER01+109:
		{
			if (o->CurrentAction == MONSTER01_WALK)
			{
				if (rand() % 15 == 0)
					PlayBuffer(SOUND_KANTURU_1ST_SATI_MOVE1 + rand() % 2);
			}
			else if (o->CurrentAction == MONSTER01_ATTACK1 || o->CurrentAction == MONSTER01_ATTACK2)
			{
				if (o->SubType == FALSE)
				{
					o->SubType = TRUE;
					PlayBuffer(SOUND_KANTURU_1ST_SATI_ATTACK1 + rand() % 2);
				}
			}
			else if(o->CurrentAction == MONSTER01_DIE)
			{
				if (o->SubType == FALSE)
				{
					o->SubType = TRUE;
					PlayBuffer(SOUND_KANTURU_1ST_SATI_DIE);
				}
			}
		}
		break;
		case MODEL_MONSTER01+110:
		{
			if(World == WD_39KANTURU_3RD && g_Direction.m_CKanturu.m_iKanturuState == KANTURU_STATE_MAYA_BATTLE)
			{
				vec3_t Light;
				float Scale, Angle;

				o->Distance += 0.05f; 
				Scale = o->Distance;

				if(Scale >= 3.0f)
					o->Distance = 0.1f;

				Angle = (timeGetTime()%9000)/10.0f;

				Light[0] = 0.6f - (Scale/5.0f);
				Light[1] = 0.6f - (Scale/5.0f);
				Light[2] = 0.6f - (Scale/5.0f);

				EnableAlphaBlend();
				RenderTerrainAlphaBitmap ( BITMAP_ENERGY_RING, o->Position[0], o->Position[1], Scale, Scale, Light, Angle );

				Vector(0.5f, 0.5f, 0.5f, Light);
				RenderTerrainAlphaBitmap ( BITMAP_ENERGY_FIELD, o->Position[0], o->Position[1], 2.0f, 2.0f, Light, Angle );
			}
			if (o->CurrentAction == MONSTER01_WALK)
			{
				if(rand()%10==0)
				{
					if(World != WD_39KANTURU_3RD)
						CreateParticle ( BITMAP_SMOKE+1, o->Position, o->Angle, o->Light );
					PlayBuffer(SOUND_KANTURU_1ST_BLADE_MOVE1 + rand() % 2);
				}
			}
			else if (o->CurrentAction == MONSTER01_ATTACK1 || o->CurrentAction == MONSTER01_ATTACK2)
			{
				if (o->SubType == FALSE)
				{
					o->SubType = TRUE;
					PlayBuffer(SOUND_KANTURU_1ST_BLADE_ATTACK1 + rand() % 2);
				}
			}
			else if(o->CurrentAction == MONSTER01_DIE)
			{
				if (o->SubType == FALSE)
				{
					o->SubType = TRUE;
					PlayBuffer(SOUND_KANTURU_1ST_BLADE_DIE);
				}
			}
		}
		break;
		case MODEL_MONSTER01+111:
		{
			if (o->CurrentAction == MONSTER01_WALK)
			{
				if (rand() % 15 == 0)
					PlayBuffer(SOUND_KANTURU_1ST_KENTA_MOVE1 + rand() % 2);
			}
			else if (o->CurrentAction == MONSTER01_ATTACK1 || o->CurrentAction == MONSTER01_ATTACK2)
			{
				if (o->SubType == FALSE)
				{
					o->SubType = TRUE;
					PlayBuffer(SOUND_KANTURU_1ST_KENTA_ATTACK1 + rand() % 2);
				}
			}
			else if(o->CurrentAction == MONSTER01_DIE)
			{
				if (o->SubType == FALSE)
				{
					o->SubType = TRUE;
					PlayBuffer(SOUND_KANTURU_1ST_KENTA_DIE);
				}
			}
			vec3_t vPos, vRelative;
			Vector(0.f, 0.f, 0.f, vRelative);
			if(o->CurrentAction == MONSTER01_DIE)
			{
				float Scale = 0.3f;
				BoneManager::GetBonePosition(o, "KENTAUROS_BIP_TAIL", vRelative, vPos);
				CreateParticle(BITMAP_SMOKE+3, vPos, o->Angle, o->Light, 3, Scale);
				BoneManager::GetBonePosition(o, "KENTAUROS_BIP_TAIL_1", vRelative, vPos);
				CreateParticle(BITMAP_SMOKE+1, vPos, o->Angle, o->Light, 1, Scale);
				BoneManager::GetBonePosition(o, "KENTAUROS_BIP_TAIL_2", vRelative, vPos);
				CreateParticle(BITMAP_SMOKE+3, vPos, o->Angle, o->Light, 3, Scale);
				BoneManager::GetBonePosition(o, "KENTAUROS_BIP_SPAIN_1", vRelative, vPos);
				CreateParticle(BITMAP_SMOKE+1, vPos, o->Angle, o->Light, 1, Scale);
				BoneManager::GetBonePosition(o, "KENTAUROS_BIP_SPAIN_2", vRelative, vPos);
				CreateParticle(BITMAP_SMOKE+3, vPos, o->Angle, o->Light, 3, Scale);
				BoneManager::GetBonePosition(o, "KENTAUROS_BIP_SPAIN_3", vRelative, vPos);
				CreateParticle(BITMAP_SMOKE+1, vPos, o->Angle, o->Light, 1, Scale);
			}
			else
			if(o->CurrentAction == MONSTER01_WALK || o->CurrentAction == MONSTER01_RUN)
			{
				if(rand()%10==0) {
					CreateParticle ( BITMAP_SMOKE+1, o->Position, o->Angle, o->Light );
				}
			}
		}
		break;
#ifdef LDS_EXTENSIONMAP_MONSTERS_KANTUR
	case MODEL_MONSTER01+197:   // 버서커 워리어
		{
			// 사운드
			if (o->CurrentAction == MONSTER01_WALK)
			{
				if (rand() % 15 == 0)
					PlayBuffer(SOUND_KANTURU_1ST_BER_MOVE1 + rand() % 2);
			}
			else if (o->CurrentAction == MONSTER01_ATTACK1 || o->CurrentAction == MONSTER01_ATTACK2)
			{
				if (o->SubType == FALSE)
				{
					o->SubType = TRUE;
					PlayBuffer(SOUND_KANTURU_1ST_BER_ATTACK1 + rand() % 2);
				}
			}
			else if(o->CurrentAction == MONSTER01_DIE)
			{
				if (o->SubType == FALSE)
				{
					o->SubType = TRUE;
					PlayBuffer(SOUND_KANTURU_1ST_BER_DIE);
				}
			}
			
			if (o->CurrentAction == MONSTER01_STOP1 || o->CurrentAction == MONSTER01_STOP2)
				o->SubType = FALSE;
			
			// 죽었거나 공격시 파티클 생성 안함.
			if (o->CurrentAction == MONSTER01_DIE
				|| o->CurrentAction == MONSTER01_ATTACK1
				|| o->CurrentAction == MONSTER01_ATTACK2)
				return true;
			
			// 입에서 나오는 입김.
			vec3_t vPos, vRelative;
			Vector(0.f, 0.f, 0.f, vRelative);
			BoneManager::GetBonePosition(o, "BERSERK_MOUTH", vRelative, vPos);
			CreateParticle(
				BITMAP_SMOKE, vPos, o->Angle, o->Light, 42, o->Scale);
		}
		return true;
	case MODEL_MONSTER01+198:   // 켄타우로스 워리어
		{
			if (o->CurrentAction == MONSTER01_WALK)
			{
				if (rand() % 15 == 0)
					PlayBuffer(SOUND_KANTURU_1ST_KENTA_MOVE1 + rand() % 2);
			}
			else if (o->CurrentAction == MONSTER01_ATTACK1 || o->CurrentAction == MONSTER01_ATTACK2)
			{
				if (o->SubType == FALSE)
				{
					o->SubType = TRUE;
					PlayBuffer(SOUND_KANTURU_1ST_KENTA_ATTACK1 + rand() % 2);
				}
			}
			else if(o->CurrentAction == MONSTER01_DIE)
			{
				if (o->SubType == FALSE)
				{
					o->SubType = TRUE;
					PlayBuffer(SOUND_KANTURU_1ST_KENTA_DIE);
				}
			}
			vec3_t vPos, vRelative;
			Vector(0.f, 0.f, 0.f, vRelative);
			if(o->CurrentAction == MONSTER01_DIE)
			{
				float Scale = 0.3f;
				BoneManager::GetBonePosition(o, "KENTAUROS_BIP_TAIL", vRelative, vPos);
				CreateParticle(BITMAP_SMOKE+3, vPos, o->Angle, o->Light, 3, Scale);
				BoneManager::GetBonePosition(o, "KENTAUROS_BIP_TAIL_1", vRelative, vPos);
				CreateParticle(BITMAP_SMOKE+1, vPos, o->Angle, o->Light, 1, Scale);
				BoneManager::GetBonePosition(o, "KENTAUROS_BIP_TAIL_2", vRelative, vPos);
				CreateParticle(BITMAP_SMOKE+3, vPos, o->Angle, o->Light, 3, Scale);
				BoneManager::GetBonePosition(o, "KENTAUROS_BIP_SPAIN_1", vRelative, vPos);
				CreateParticle(BITMAP_SMOKE+1, vPos, o->Angle, o->Light, 1, Scale);
				BoneManager::GetBonePosition(o, "KENTAUROS_BIP_SPAIN_2", vRelative, vPos);
				CreateParticle(BITMAP_SMOKE+3, vPos, o->Angle, o->Light, 3, Scale);
				BoneManager::GetBonePosition(o, "KENTAUROS_BIP_SPAIN_3", vRelative, vPos);
				CreateParticle(BITMAP_SMOKE+1, vPos, o->Angle, o->Light, 1, Scale);
			}
			else
			{
				if(o->CurrentAction == MONSTER01_WALK || o->CurrentAction == MONSTER01_RUN)
				{
					if(rand()%10==0) {
						CreateParticle ( BITMAP_SMOKE+1, o->Position, o->Angle, o->Light );
					}
				}
			}
		}
		return true;
	case MODEL_MONSTER01+199:   // 기간티스 워리어
		{
			// 사운드
			if (o->CurrentAction == MONSTER01_WALK)
			{
				if (rand() % 15 == 0)
					PlayBuffer(SOUND_KANTURU_1ST_GIGAN_MOVE1);
			}
			else if (o->CurrentAction == MONSTER01_ATTACK1 || o->CurrentAction == MONSTER01_ATTACK2)
			{
				if (o->SubType == FALSE)
				{
					o->SubType = TRUE;
					PlayBuffer(SOUND_KANTURU_1ST_GIGAN_ATTACK1 + rand() % 2);
				}
			}
			else if(o->CurrentAction == MONSTER01_DIE)
			{
				if (o->SubType == FALSE)
				{
					o->SubType = TRUE;
					PlayBuffer(SOUND_KANTURU_1ST_GIGAN_DIE);
				}
			}
			
			if (o->CurrentAction == MONSTER01_STOP1 || o->CurrentAction == MONSTER01_STOP2)
				o->SubType = FALSE;
			
			// 뿔 발광 효과.
			if (o->CurrentAction == MONSTER01_DIE)
				return true;
			
			vec3_t EndRelative, EndPos;
			Vector(19.f, -2.f, 0.f, EndRelative);
			
			b->TransformPosition(
				o->BoneTransform[7], EndRelative, EndPos, true);
			
			Vector(0.4f, 0.6f, 0.8f, o->Light);
			CreateSprite(BITMAP_LIGHT, EndPos, 3.0f, o->Light, o, 0.5f);
			
			float Luminosity;
			Luminosity = sinf(WorldTime * 0.05f) * 0.4f + 0.9f;
			Vector(Luminosity * 0.3f, Luminosity * 0.5f, Luminosity * 0.8f,
				o->Light);
				CreateSprite(BITMAP_LIGHT, EndPos, 1.0f, o->Light, o);
		}
		return true;
	case MODEL_MONSTER01+200:   // 제노사이더 워리어
		{
			// 사운드
			if (o->CurrentAction == MONSTER01_WALK)
			{
				if (rand() % 15 == 0)
					PlayBuffer(SOUND_KANTURU_1ST_GENO_MOVE1 + rand() % 2);
			}
			else if (o->CurrentAction == MONSTER01_ATTACK1 || o->CurrentAction == MONSTER01_ATTACK2)
			{
				if (o->SubType == FALSE)
				{
					o->SubType = TRUE;
					PlayBuffer(SOUND_KANTURU_1ST_GENO_ATTACK1 + rand() % 2);
				}
			}
			else if(o->CurrentAction == MONSTER01_DIE)
			{
				if (o->SubType == FALSE)
				{
					o->SubType = TRUE;
					PlayBuffer(SOUND_KANTURU_1ST_GENO_DIE);
				}
			}
			
			if (o->CurrentAction == MONSTER01_STOP1 || o->CurrentAction == MONSTER01_STOP2)
				o->SubType = FALSE;
			
			// 죽었거나 공격시 파티클 생성 안함.
			if (o->CurrentAction == MONSTER01_WALK)
			{
				// 철퇴 끝에서 연기.
				vec3_t vPos, vRelative;
				Vector(0.f, 0.f, 0.f, vRelative);
				BoneManager::GetBonePosition(o, "GENO_WP", vRelative, vPos);
				CreateParticle(
					BITMAP_SMOKE+1, vPos, o->Angle, o->Light, 1, o->Scale);
				
				CreateEffect(MODEL_STONE1+rand()%2, vPos, o->Angle, o->Light);
			}
		}
		return true;
#endif // LDS_EXTENSIONMAP_MONSTERS_KANTUR
	}
	return false;
}

void M37Kanturu1st::MoveKanturu1stBlurEffect(CHARACTER* c, OBJECT* o, BMD* b)
{
	switch (o->Type)
	{
	case MODEL_MONSTER01+109:
		{
			if ((o->AnimationFrame >= 3.6f && o->AnimationFrame <= 6.0f
				&& o->CurrentAction == MONSTER01_ATTACK1) 
				|| (o->AnimationFrame >= 3.6f && o->AnimationFrame <= 6.0f
				&& o->CurrentAction == MONSTER01_ATTACK2))
			{
				vec3_t  Light;
				Vector(0.3f, 1.0f, 3.5f, Light);

				vec3_t StartPos, StartRelative;
				vec3_t EndPos, EndRelative;
				
				float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed;
				float fSpeedPerFrame = fActionSpeed / 10.f;
				float fAnimationFrame = o->AnimationFrame - fActionSpeed;
				for (int i=0; i<10; i++) 
				{
					b->Animation(BoneTransform, fAnimationFrame,
						o->PriorAnimationFrame, o->PriorAction, o->Angle,
						o->HeadAngle);

					Vector(0.f, 0.f, 0.f, StartRelative);
					Vector(0.f, 0.f, -200.f, EndRelative);

					b->TransformPosition(
						BoneTransform[44], StartRelative, StartPos, false);
					b->TransformPosition(
						BoneTransform[44], EndRelative, EndPos, false);
					CreateBlur(c, StartPos, EndPos, Light, 1);
					
					fAnimationFrame += fSpeedPerFrame;
				}
			}
		}
		break;
	case MODEL_MONSTER01+106:	// 버서커
		{
			if ((o->AnimationFrame >= 3.5f && o->AnimationFrame <= 6.7f
				&& o->CurrentAction == MONSTER01_ATTACK1) 
				|| (o->AnimationFrame >= 3.5f && o->AnimationFrame <= 6.7f
				&& o->CurrentAction == MONSTER01_ATTACK2))
			{
				vec3_t  Light;
				Vector(0.3f, 2.0f, 0.5f, Light);

				vec3_t StartPos, StartRelative;
				vec3_t EndPos, EndRelative;
				
				float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed;
				float fSpeedPerFrame = fActionSpeed / 10.f;
				float fAnimationFrame = o->AnimationFrame - fActionSpeed;
				for (int i=0; i<10; i++) 
				{
					b->Animation(BoneTransform, fAnimationFrame,
						o->PriorAnimationFrame, o->PriorAction, o->Angle,
						o->HeadAngle);

					Vector(-20.f, 0.f, 0.f, StartRelative);
					Vector(100.f, 0.f, 0.f, EndRelative);

					b->TransformPosition(
						BoneTransform[33], StartRelative, StartPos, false);
					b->TransformPosition(
						BoneTransform[34], EndRelative, EndPos, false);
					CreateBlur(c, StartPos, EndPos, Light, 1);
					
					fAnimationFrame += fSpeedPerFrame;
				}
			}
		}
		break;
	case MODEL_MONSTER01+110:
		{
			float Start_Frame = 5.9f;
			float End_Frame = 7.55f;
			if((o->AnimationFrame >= Start_Frame && o->AnimationFrame <= End_Frame && o->CurrentAction == MONSTER01_ATTACK1) 
				|| (o->AnimationFrame >= Start_Frame && o->AnimationFrame <= End_Frame && o->CurrentAction == MONSTER01_ATTACK2))
			{
				vec3_t  Light;
				Vector(1.0f, 1.2f, 2.f, Light);

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

					b->TransformPosition(BoneTransform[55], StartRelative, StartPos, false);
					b->TransformPosition(BoneTransform[54], EndRelative, EndPos, false);
					CreateBlur(c, StartPos, EndPos, Light, 0);
					
					fAnimationFrame += fSpeedPerFrame;
				}
			}
		}
		break;
	case MODEL_MONSTER01+112:	// 기간티스
		{
			if ((o->AnimationFrame >= 3.5f && o->AnimationFrame <= 5.9f
				&& o->CurrentAction == MONSTER01_ATTACK1) 
				|| (o->AnimationFrame >= 3.5f && o->AnimationFrame <= 5.9f
				&& o->CurrentAction == MONSTER01_ATTACK2))
			{
				vec3_t  Light;
				Vector(1.0f, 1.2f, 2.f, Light);

				vec3_t StartPos, StartRelative;
				vec3_t EndPos, EndRelative;
				
				float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed;
				float fSpeedPerFrame = fActionSpeed / 10.f;
				float fAnimationFrame = o->AnimationFrame - fActionSpeed;
				for (int i = 0; i < 10; i++) 
				{
					b->Animation(BoneTransform, fAnimationFrame,
						o->PriorAnimationFrame, o->PriorAction, o->Angle,
						o->HeadAngle);

					Vector(0.f, 0.f, 0.f, StartRelative);
					Vector(0.f, 0.f, 0.f, EndRelative);

					b->TransformPosition(
						BoneTransform[34], StartRelative, StartPos, false);
					b->TransformPosition(
						BoneTransform[33], EndRelative, EndPos, false);
					CreateBlur(c, StartPos, EndPos, Light, 0);
					
					fAnimationFrame += fSpeedPerFrame;
				}
			}
		}
		break;
	case MODEL_MONSTER01+113:	// 제노사이더
		{
			if ((o->AnimationFrame >= 5.5f && o->AnimationFrame <= 6.9f
				&& o->CurrentAction == MONSTER01_ATTACK1) 
				|| (o->AnimationFrame >= 5.5f && o->AnimationFrame <= 6.9f
				&& o->CurrentAction == MONSTER01_ATTACK2))
			{
				vec3_t  Light;
				Vector(1.0f, 0.2f, 0.0f, Light);

//				vec3_t vPos, vRelative;
				vec3_t StartPos, StartRelative;
				vec3_t EndPos, EndRelative;
				
				float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed;
				float fSpeedPerFrame = fActionSpeed/10.f;
				float fAnimationFrame = o->AnimationFrame - fActionSpeed;
				for (int i = 0; i < 18; i++) 
				{
					b->Animation(BoneTransform, fAnimationFrame,
						o->PriorAnimationFrame, o->PriorAction, o->Angle,
						o->HeadAngle);

					Vector(-40.f, 0.f, 0.f, StartRelative);
					Vector(10.f, 0.f, 0.f, EndRelative);

					b->TransformPosition(
						BoneTransform[49], StartRelative, StartPos, false);
					b->TransformPosition(
						BoneTransform[49], EndRelative, EndPos, false);
					CreateBlur(c, StartPos, EndPos, Light, 0, true, 0);

					b->TransformPosition(
						BoneTransform[51], StartRelative, StartPos, false);
					b->TransformPosition(
						BoneTransform[51], EndRelative, EndPos, false);
					CreateBlur(c, StartPos, EndPos, Light, 0, true, 1);

					b->TransformPosition(
						BoneTransform[50], StartRelative, StartPos, false);
					b->TransformPosition(
						BoneTransform[50], EndRelative, EndPos, false);
					CreateBlur(c, StartPos, EndPos, Light, 0, true, 2);

					b->TransformPosition(
						BoneTransform[52], StartRelative, StartPos, false);
					b->TransformPosition(
						BoneTransform[52], EndRelative, EndPos, false);
					CreateBlur(c, StartPos, EndPos, Light, 0, true, 3);

					fAnimationFrame += fSpeedPerFrame;
				}
			}
		}
		break;
#ifdef LDS_EXTENSIONMAP_MONSTERS_KANTUR
	case MODEL_MONSTER01+197:	// 버서커 워리어
		{
			if ((o->AnimationFrame >= 3.5f && o->AnimationFrame <= 6.7f
				&& o->CurrentAction == MONSTER01_ATTACK1) 
				|| (o->AnimationFrame >= 3.5f && o->AnimationFrame <= 6.7f
				&& o->CurrentAction == MONSTER01_ATTACK2))
			{
				vec3_t  Light;
				Vector(0.3f, 2.0f, 0.5f, Light);
				
				vec3_t StartPos, StartRelative;
				vec3_t EndPos, EndRelative;
				
				float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed;
				float fSpeedPerFrame = fActionSpeed / 10.f;
				float fAnimationFrame = o->AnimationFrame - fActionSpeed;
				for (int i=0; i<10; i++) 
				{
					b->Animation(BoneTransform, fAnimationFrame,
						o->PriorAnimationFrame, o->PriorAction, o->Angle,
						o->HeadAngle);
					
					Vector(-20.f, 0.f, 0.f, StartRelative);
					Vector(100.f, 0.f, 0.f, EndRelative);
					
					b->TransformPosition(
						BoneTransform[33], StartRelative, StartPos, false);
					b->TransformPosition(
						BoneTransform[34], EndRelative, EndPos, false);
					CreateBlur(c, StartPos, EndPos, Light, 1);
					
					fAnimationFrame += fSpeedPerFrame;
				}
			}
		}
		break;
	case MODEL_MONSTER01+198:	// 기간티스 워리어
		{
			if ((o->AnimationFrame >= 3.5f && o->AnimationFrame <= 5.9f
				&& o->CurrentAction == MONSTER01_ATTACK1) 
				|| (o->AnimationFrame >= 3.5f && o->AnimationFrame <= 5.9f
				&& o->CurrentAction == MONSTER01_ATTACK2))
			{
				vec3_t  Light;
				Vector(1.0f, 1.2f, 2.f, Light);
				
				vec3_t StartPos, StartRelative;
				vec3_t EndPos, EndRelative;
				
				float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed;
				float fSpeedPerFrame = fActionSpeed / 10.f;
				float fAnimationFrame = o->AnimationFrame - fActionSpeed;
				for (int i = 0; i < 10; i++) 
				{
					b->Animation(BoneTransform, fAnimationFrame,
						o->PriorAnimationFrame, o->PriorAction, o->Angle,
						o->HeadAngle);
					
					Vector(0.f, 0.f, 0.f, StartRelative);
					Vector(0.f, 0.f, 0.f, EndRelative);
					
					b->TransformPosition(
						BoneTransform[34], StartRelative, StartPos, false);
					b->TransformPosition(
						BoneTransform[33], EndRelative, EndPos, false);
					CreateBlur(c, StartPos, EndPos, Light, 0);
					
					fAnimationFrame += fSpeedPerFrame;
				}
			}
		}
		break;
	case MODEL_MONSTER01+199:	// 제노사이더 워리어
		{
			if ((o->AnimationFrame >= 5.5f && o->AnimationFrame <= 6.9f
				&& o->CurrentAction == MONSTER01_ATTACK1) 
				|| (o->AnimationFrame >= 5.5f && o->AnimationFrame <= 6.9f
				&& o->CurrentAction == MONSTER01_ATTACK2))
			{
				vec3_t  Light;
				Vector(1.0f, 0.2f, 0.0f, Light);
				
				//				vec3_t vPos, vRelative;
				vec3_t StartPos, StartRelative;
				vec3_t EndPos, EndRelative;
				
				float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed;
				float fSpeedPerFrame = fActionSpeed/10.f;
				float fAnimationFrame = o->AnimationFrame - fActionSpeed;
				for (int i = 0; i < 18; i++) 
				{
					b->Animation(BoneTransform, fAnimationFrame,
						o->PriorAnimationFrame, o->PriorAction, o->Angle,
						o->HeadAngle);
					
					Vector(-40.f, 0.f, 0.f, StartRelative);
					Vector(10.f, 0.f, 0.f, EndRelative);
					
					b->TransformPosition(
						BoneTransform[49], StartRelative, StartPos, false);
					b->TransformPosition(
						BoneTransform[49], EndRelative, EndPos, false);
					CreateBlur(c, StartPos, EndPos, Light, 0, true, 0);
					
					b->TransformPosition(
						BoneTransform[51], StartRelative, StartPos, false);
					b->TransformPosition(
						BoneTransform[51], EndRelative, EndPos, false);
					CreateBlur(c, StartPos, EndPos, Light, 0, true, 1);
					
					b->TransformPosition(
						BoneTransform[50], StartRelative, StartPos, false);
					b->TransformPosition(
						BoneTransform[50], EndRelative, EndPos, false);
					CreateBlur(c, StartPos, EndPos, Light, 0, true, 2);
					
					b->TransformPosition(
						BoneTransform[52], StartRelative, StartPos, false);
					b->TransformPosition(
						BoneTransform[52], EndRelative, EndPos, false);
					CreateBlur(c, StartPos, EndPos, Light, 0, true, 3);
					
					fAnimationFrame += fSpeedPerFrame;
				}
			}
		}
		break;
#endif // LDS_EXTENSIONMAP_MONSTERS_KANTUR
	}
}
