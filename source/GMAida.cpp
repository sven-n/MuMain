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
#include "ZzzCharacter.h"
#include "DSPlaySound.h"

#include "GMAida.h"
#include "BoneManager.h"

DWORD g_MusicStartStamp2 = 0;


bool M33Aida::IsInAida()
{
	return (World == WD_33AIDA || World == WD_54CHARACTERSCENE) ? true : false;
}

bool M33Aida::IsInAidaSection2(const vec3_t Position)
{
	if(Position[0] > 5449.f && Position[0] < 17822.f && Position[1] > 6784.f && Position[1] < 22419.f)
		return true;
	return false;
}

//. 오브젝트
bool M33Aida::CreateAidaObject(OBJECT* pObject)
{
	if(!IsInAida())
		return false;
	
	return true;
}

bool M33Aida::MoveAidaObject(OBJECT* pObject)
{
	if(!IsInAida())
		return false;
	
	float Luminosity;
	vec3_t Light;
	
	switch(pObject->Type)
	{
		case 25:  // 폭포 1
			{
				pObject->BlendMeshTexCoordV -= 0.015f;
			}
			break;	
		case 28:  // 폭포 2
			{
				pObject->BlendMeshTexCoordV -= 0.015f;
			}
			break;	
		case 30:  //  풀 - 주위 빛
			{
				Luminosity = (float)(rand()%5)*0.01f;
				Vector(Luminosity+0.4f,Luminosity+0.6f,Luminosity+0.4f,Light);
				AddTerrainLight(pObject->Position[0],pObject->Position[1],Light,2,PrimaryTerrainLight);
			}
			break;
		case 71:  //  풀(붉은) - 주위 빛
			{
				Luminosity = (float)(rand()%5)*0.01f;
				Vector(Luminosity+0.9f,Luminosity+0.2f,Luminosity+0.2f,Light);
				AddTerrainLight(pObject->Position[0],pObject->Position[1],Light,2,PrimaryTerrainLight);
			}
			break;
		case 41:  // 빛
			{
				pObject->Alpha = 0.5f;
			}
			break;
		case    56 :
        case    57 :
        case    58 :
		case	59 :
		case	62 :
		case	63 :
		case	67 :
		case	70 :			
			pObject->HiddenMesh = -2;
			break;
		case	64 :
			pObject->Velocity = 0.05f;
			break;
	}

	//. 배경음악 컨트롤
//	if(::timeGetTime() - g_MusicStartStamp2 > 100000) {
//		g_MusicStartStamp2 = ::timeGetTime();
		PlayBuffer ( SOUND_AIDA_AMBIENT );
//	}
	
	return true;
}

//. 오브젝트 효과를 추가한다.
bool M33Aida::RenderAidaObjectVisual(OBJECT* pObject, BMD* pModel)
{
	if(!IsInAida())
		return false;

	vec3_t p,Position, Light;

	switch(pObject->Type)
	{
		case 30:  // 풀
			{
				Vector(0.0f,  -3.0f, 1.0f, p);
				pModel->TransformPosition(BoneTransform[6],p,Position,false);
				Vector(0.1f,0.1f,0.3f,Light);
				CreateSprite(BITMAP_SPARK+1, Position, 5.5f, Light, pObject);
				Vector(0.15f,0.15f,0.15f,Light);
				CreateSprite(BITMAP_SPARK+1, Position, 3.0f, Light, pObject);

				Vector(0.0f, -3.0f, 0.0f, p);
				pModel->TransformPosition(BoneTransform[7],p,Position,false);
				Vector(0.1f,0.1f,0.3f,Light);
				CreateSprite(BITMAP_SPARK+1, Position, 4.5f, Light, pObject);
				Vector(0.15f,0.15f,0.15f,Light);
				CreateSprite(BITMAP_SPARK+1, Position, 3.0f, Light, pObject);

				Vector(0.0f, -3.0f, 0.0f, p);
				pModel->TransformPosition(BoneTransform[8],p,Position,false);
				Vector(0.1f,0.1f,0.3f,Light);
				CreateSprite(BITMAP_SPARK+1, Position, 4.0f, Light, pObject);
				Vector(0.15f,0.15f,0.15f,Light);
				CreateSprite(BITMAP_SPARK+1, Position, 3.0f, Light, pObject);
				
				Vector(-3.0f,  -1.0f, 0.0f, p);
				pModel->TransformPosition(BoneTransform[12],p,Position,false);
				Vector(0.1f,0.1f,0.3f,Light);
				CreateSprite(BITMAP_SPARK+1, Position, 4.5f, Light, pObject);
				Vector(0.15f,0.15f,0.15f,Light);
				CreateSprite(BITMAP_SPARK+1, Position, 3.0f, Light, pObject);

				Vector(-3.0f,  -3.0f, 0.0f, p);
				pModel->TransformPosition(BoneTransform[13],p,Position,false);
				Vector(0.1f,0.1f,0.3f,Light);
				CreateSprite(BITMAP_SPARK+1, Position, 4.0f, Light, pObject);
				Vector(0.15f,0.15f,0.15f,Light);
				CreateSprite(BITMAP_SPARK+1, Position, 3.0f, Light, pObject);

				Vector(0.0f, 2.0f, -8.0f, p);
				pModel->TransformPosition(BoneTransform[17],p,Position,false);
				Vector(0.1f,0.1f,0.3f,Light);
				CreateSprite(BITMAP_SPARK+1, Position, 4.0f, Light, pObject);
				Vector(0.15f,0.15f,0.15f,Light);
				CreateSprite(BITMAP_SPARK+1, Position, 3.0f, Light, pObject);
				
			}
			break;
		case 41:  // 빛
			{
				pModel->BeginRender(1.0f);

				pModel->BodyLight[0] = 0.52f;
				pModel->BodyLight[1] = 0.52f;
				pModel->BodyLight[2] = 0.52f;

				pModel->StreamMesh = 0;
#ifdef LDK_FIX_AIDA_OBJ41_UV_FIX
				pModel->RenderMesh ( 0, RENDER_TEXTURE| RENDER_BRIGHT, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV );
#else //LDK_FIX_AIDA_OBJ41_UV_FIX
				pModel->RenderMesh ( 0, RENDER_TEXTURE| RENDER_BRIGHT, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, -(int)WorldTime%100000*0.00001f, pObject->BlendMeshTexCoordV );
#endif //LDK_FIX_AIDA_OBJ41_UV_FIX
				pModel->StreamMesh = -1;
				
				pModel->EndRender();
			}
			break;	
        case 56 :  //  폭포 효과 - 위에서 떨어지는것
            Vector ( 1.f, 1.f, 1.f, Light );
            if ( rand()%2==0 )
            {
                CreateParticle ( BITMAP_WATERFALL_1, pObject->Position, pObject->Angle, Light, 2, pObject->Scale );
            }
            break;
        case 57 :  //  폭포 효과 - 물 튀는 느낌
            Vector ( 1.f, 1.f, 1.f, Light );
            CreateParticle ( BITMAP_WATERFALL_3, pObject->Position, pObject->Angle, Light, 4, pObject->Scale );
            break;
        case 58 :  //  폭포 효과 - 물 안개 효과
            Vector ( 1.f, 1.f, 1.f, Light );
            if ( rand()%3==0 )
            {
				CreateParticle ( BITMAP_WATERFALL_2, pObject->Position, pObject->Angle, Light, 2, pObject->Scale );
            }
            break;
		case 59:  //  안개 효과 - 푸르슴한 빛
			if ( pObject->HiddenMesh!=-2 )
			{
                vec3_t  Light;
                Vector(0.01f,0.03f,0.05f,Light);
				for ( int i=0; i<20; ++i )
				{
					CreateParticle ( BITMAP_CLOUD, pObject->Position, pObject->Angle, Light, 1, pObject->Scale, pObject );
				}
			}
			break;
		case 60:  //  반딧불
			{
				int time = timeGetTime()%1024;
				if(time >= 0 && time < 10) {
					vec3_t Light;
					Vector(1.f,1.f,1.f,Light);
					CreateEffect(MODEL_BUTTERFLY01,pObject->Position,pObject->Angle,Light, 3, pObject);
				}
				pObject->HiddenMesh = -2;	//. Hide Object				
			}
			break;
		case 62:  //  안개 효과 - 흰색
			if ( pObject->HiddenMesh!=-2 )
			{
                vec3_t  Light;
                Vector(0.05f,0.05f,0.05f,Light);
				for ( int i=0; i<20; ++i )
				{
					CreateParticle ( BITMAP_CLOUD, pObject->Position, pObject->Angle, Light, 1, pObject->Scale, pObject );
				}
			}
			break;
		case 63:  //  안개 효과 - 붉으스레한 빛
			if ( pObject->HiddenMesh!=-2 )
			{
                vec3_t  Light;
                Vector(0.05f,0.02f,0.02f,Light);
				for ( int i=0; i<20; ++i )
				{
					CreateParticle ( BITMAP_CLOUD, pObject->Position, pObject->Angle, Light, 1, pObject->Scale, pObject );
				}
			}
			break;
		case 65:  //  뱀 1
		case 66:  //  뱀 2 
			{
				pModel->BeginRender(1.0f);

				pModel->StreamMesh = 0;
				pModel->RenderMesh ( 0, RENDER_TEXTURE, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, -(int)WorldTime%100000*0.00005f, pObject->BlendMeshTexCoordV );
				pModel->StreamMesh = -1;

				pModel->EndRender();
			}
			break;
		case	67 :  //  불박스
			Vector(0.3f, 0.3f, 0.3f, pObject->Light);
			CreateParticle(BITMAP_FLAME, pObject->Position, pObject->Angle, pObject->Light, 7, pObject->Scale);
			break;
		case	70 :  //  유령박스2
			{
				int time = timeGetTime()%1024;
				if(rand()%5 == 1 && (time >= 0 && time < 30))
				{
					Vector(0.1f, 0.1f, 0.1f, Light);
					CreateEffect(MODEL_GHOST,pObject->Position,pObject->Angle, Light, 0, pObject, -1, 0, pObject->Scale);
				}
			}
			break;
		case	71 :  //  풀 (붉은색)
			{
				Vector(0.0f,  -3.0f, 1.0f, p);
				pModel->TransformPosition(BoneTransform[6],p,Position,false);
				Vector(0.5f,0.0f,0.0f,Light);
				CreateSprite(BITMAP_SPARK+1, Position, 6.0f, Light, pObject);
				Vector(0.15f,0.0f,0.0f,Light);
				CreateSprite(BITMAP_SPARK+1, Position, 3.0f, Light, pObject);

				Vector(0.0f, -3.0f, 0.0f, p);
				pModel->TransformPosition(BoneTransform[7],p,Position,false);
				Vector(0.25f,0.0f,0.0f,Light);
				CreateSprite(BITMAP_SPARK+1, Position, 5.5f, Light, pObject);
				Vector(0.15f,0.0f,0.0f,Light);
				CreateSprite(BITMAP_SPARK+1, Position, 3.0f, Light, pObject);

				Vector(0.0f, -3.0f, 0.0f, p);
				pModel->TransformPosition(BoneTransform[8],p,Position,false);
				Vector(0.25f,0.0f,0.0f,Light);
				CreateSprite(BITMAP_SPARK+1, Position, 5.0f, Light, pObject);
				Vector(0.15f,0.0f,0.0f,Light);
				CreateSprite(BITMAP_SPARK+1, Position, 3.0f, Light, pObject);
				
				Vector(-3.0f,  -1.0f, 0.0f, p);
				pModel->TransformPosition(BoneTransform[12],p,Position,false);
				Vector(0.25f,0.0f,0.0f,Light);
				CreateSprite(BITMAP_SPARK+1, Position, 5.5f, Light, pObject);
				Vector(0.15f,0.0f,0.0f,Light);
				CreateSprite(BITMAP_SPARK+1, Position, 3.0f, Light, pObject);

				Vector(-3.0f,  -3.0f, 0.0f, p);
				pModel->TransformPosition(BoneTransform[13],p,Position,false);
				Vector(0.25f,0.0f,0.0f,Light);
				CreateSprite(BITMAP_SPARK+1, Position, 5.0f, Light, pObject);
				Vector(0.15f,0.0f,0.0f,Light);
				CreateSprite(BITMAP_SPARK+1, Position, 3.0f, Light, pObject);

				Vector(0.0f, 2.0f, -8.0f, p);
				pModel->TransformPosition(BoneTransform[17],p,Position,false);
				Vector(0.25f,0.0f,0.0f,Light);
				CreateSprite(BITMAP_SPARK+1, Position, 5.0f, Light, pObject);
				Vector(0.15f,0.0f,0.0f,Light);
				CreateSprite(BITMAP_SPARK+1, Position, 3.0f, Light, pObject);
			}
			break;
		case 75:
			{
				Vector(0.f, 0.f, 0.f, p);
				//Vector(1.f, 1.f, 1.f, Light);
				pModel->TransformPosition(BoneTransform[4],p,Position,false);
				float fLumi = (sinf(WorldTime*0.002f) + 1.0f) * 0.5f;
				Vector(fLumi, fLumi, fLumi, Light);
				Vector(1.0f,1.0f,1.f,Light);
				CreateSprite(BITMAP_FLARE,Position,3.0f,Light,pObject, (WorldTime/10.0f));
			}
			break;
#ifdef LDK_ADD_AIDA_OBJ77_OBJ78
		case 77:  //  뱀 3
		case 78:  //  뱀 3
			{
				pModel->BeginRender(1.0f);
				pModel->StreamMesh = 0;
				pModel->RenderMesh ( 0, RENDER_TEXTURE, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, (int)WorldTime%100000*0.0002f, pObject->BlendMeshTexCoordV );
				pModel->StreamMesh = -1;
				pModel->EndRender();
			}
			break;
#endif //LDK_ADD_AIDA_OBJ77_OBJ78
	}

	return true;
}
bool M33Aida::RenderAidaObjectMesh(OBJECT* pObject, BMD* pModel,bool ExtraMon)
{
	if(IsInAida())
	{
	}

	return RenderAidaMonsterObjectMesh(pObject, pModel,ExtraMon);
}

//. 몬스터
CHARACTER* M33Aida::CreateAidaMonster(int iType, int PosX, int PosY, int Key)
{
	CHARACTER* pCharacter = NULL;
	switch(iType)
	{
	case 304:	//  위치 퀸
		{
			OpenMonsterModel(100);
		    pCharacter = CreateCharacter(Key,MODEL_MONSTER01+100,PosX,PosY);
		    pCharacter->Object.Scale = 1.4f;
		    pCharacter->Weapon[0].Type = -1;
		    pCharacter->Weapon[1].Type = -1;

			BoneManager::RegisterBone(pCharacter, "Monster100_Footstepst", 0);
			BoneManager::RegisterBone(pCharacter, "Monster100_L_Hand",    76);
			BoneManager::RegisterBone(pCharacter, "Monster100_R_Hand",    94);
			BoneManager::RegisterBone(pCharacter, "Monster100_Pelvis",     2);
			BoneManager::RegisterBone(pCharacter, "Monster100_Head",	  57);
			BoneManager::RegisterBone(pCharacter, "Monster100_z02",		 107);
			BoneManager::RegisterBone(pCharacter, "Monster100_z03",		 108);
			BoneManager::RegisterBone(pCharacter, "Monster100_z04",		 109);
			BoneManager::RegisterBone(pCharacter, "Monster100_z05",	     110);
		}
		break;
	case 305:	//  블루 골렘
		{
			OpenMonsterModel(101);
		    pCharacter = CreateCharacter(Key,MODEL_MONSTER01+101,PosX,PosY);
		    pCharacter->Object.Scale = 1.35f;
		    pCharacter->Weapon[0].Type = -1;
		    pCharacter->Weapon[1].Type = -1;

			BoneManager::RegisterBone(pCharacter, "Monster101_L_Arm", 12);
			BoneManager::RegisterBone(pCharacter, "Monster101_R_Arm", 20);
			BoneManager::RegisterBone(pCharacter, "Monster101_Head", 6);
		}
		break;
	case 306:	//  데스 라이더
		{
			OpenMonsterModel(102);
		    pCharacter = CreateCharacter(Key,MODEL_MONSTER01+102,PosX,PosY);
		    pCharacter->Object.Scale = 1.1f;
		    pCharacter->Weapon[0].Type = -1;
		    pCharacter->Weapon[1].Type = -1;

			BoneManager::RegisterBone(pCharacter, "Monster102_Footstepst", 0);
			BoneManager::RegisterBone(pCharacter, "Monster102_Head",       6);
		}
		break;
	case 307:	//  포트레스 오크
		{
			OpenMonsterModel(103);
		    pCharacter = CreateCharacter(Key,MODEL_MONSTER01+103,PosX,PosY);
		    pCharacter->Object.Scale = 1.15f;
		    pCharacter->Weapon[0].Type = -1;
		    pCharacter->Weapon[1].Type = -1;
		}
		break;
	case 308:	//  데스 트리
		{
			OpenMonsterModel(104);
		    pCharacter = CreateCharacter(Key,MODEL_MONSTER01+104,PosX,PosY);
		    pCharacter->Object.Scale = 1.2f;
		    pCharacter->Weapon[0].Type = -1;
		    pCharacter->Weapon[1].Type = -1;

			BoneManager::RegisterBone(pCharacter, "Monster104_Horn0", 37);
			BoneManager::RegisterBone(pCharacter, "Monster104_Horn1", 38);
			BoneManager::RegisterBone(pCharacter, "Monster104_Horn2", 39);
			BoneManager::RegisterBone(pCharacter, "Monster104_Horn3", 40);
			BoneManager::RegisterBone(pCharacter, "Monster104_Horn4", 44);
			BoneManager::RegisterBone(pCharacter, "Monster104_Horn5", 45);
			BoneManager::RegisterBone(pCharacter, "Monster104_Footsteps",  1);

		}
		break;
	case 309:	//  헬 마이너
		{
			OpenMonsterModel(105);
		    pCharacter = CreateCharacter(Key,MODEL_MONSTER01+105,PosX,PosY);
		    pCharacter->Object.Scale = 1.8f;
		    pCharacter->Weapon[0].Type = -1;
		    pCharacter->Weapon[1].Type = -1;

			BoneManager::RegisterBone(pCharacter, "Monster105_R_Eye", 9);
			BoneManager::RegisterBone(pCharacter, "Monster105_L_Eye", 10);
			BoneManager::RegisterBone(pCharacter, "Monster105_L_Arm00", 33);
			BoneManager::RegisterBone(pCharacter, "Monster105_L_Arm01", 34);
			BoneManager::RegisterBone(pCharacter, "Monster105_L_Arm02", 35);
			BoneManager::RegisterBone(pCharacter, "Monster105_L_Hand", 20);
			BoneManager::RegisterBone(pCharacter, "Monster105_R_Hand", 39);
//			BoneManager::RegisterBone(pCharacter, "Monster105_Footsteps", 1);
		}
		break;
#ifdef LDS_EXTENSIONMAP_MONSTERS_AIDA
	case 549:	//  블러디 포트레스 오크
		{
			OpenMonsterModel(193);
			pCharacter = CreateCharacter(Key,MODEL_MONSTER01+193,PosX,PosY);
			//pCharacter->Object.Scale = 1.15f;
			pCharacter->Object.Scale = 1.35f;
			pCharacter->Weapon[0].Type = -1;
			pCharacter->Weapon[1].Type = -1;
		}
		break;
	case 550:	//  블러디 데스 라이더
		{
			OpenMonsterModel(194);
			pCharacter = CreateCharacter(Key,MODEL_MONSTER01+194,PosX,PosY);
			//pCharacter->Object.Scale = 1.1f;
			pCharacter->Object.Scale = 1.2f;
			pCharacter->Weapon[0].Type = -1;
			pCharacter->Weapon[1].Type = -1;
			
			BoneManager::RegisterBone(pCharacter, "Monster102_Footstepst", 0);
			BoneManager::RegisterBone(pCharacter, "Monster102_Head",       6);
		}
		break;
	case 551:	//  블러디 골렘
		{
			OpenMonsterModel(195);
			pCharacter = CreateCharacter(Key,MODEL_MONSTER01+195,PosX,PosY);
			//pCharacter->Object.Scale = 1.35f;
			pCharacter->Object.Scale = 1.40f;
			pCharacter->Weapon[0].Type = -1;
			pCharacter->Weapon[1].Type = -1;
			
			BoneManager::RegisterBone(pCharacter, "Monster101_L_Arm", 12);
			BoneManager::RegisterBone(pCharacter, "Monster101_R_Arm", 20);
			BoneManager::RegisterBone(pCharacter, "Monster101_Head", 6);
		}
		break;
	case 552:	//  블러디 위치 퀸
		{
			OpenMonsterModel(196);
			pCharacter = CreateCharacter(Key,MODEL_MONSTER01+196,PosX,PosY);
			//pCharacter->Object.Scale = 1.4f;
			pCharacter->Object.Scale = 1.75f;
			pCharacter->Weapon[0].Type = -1;
			pCharacter->Weapon[1].Type = -1;
			
			BoneManager::RegisterBone(pCharacter, "Monster100_Footstepst", 0);
			BoneManager::RegisterBone(pCharacter, "Monster100_L_Hand",    76);
			BoneManager::RegisterBone(pCharacter, "Monster100_R_Hand",    94);
			BoneManager::RegisterBone(pCharacter, "Monster100_Pelvis",     2);
			BoneManager::RegisterBone(pCharacter, "Monster100_Head",	  57);
			BoneManager::RegisterBone(pCharacter, "Monster100_z02",		 107);
			BoneManager::RegisterBone(pCharacter, "Monster100_z03",		 108);
			BoneManager::RegisterBone(pCharacter, "Monster100_z04",		 109);
			BoneManager::RegisterBone(pCharacter, "Monster100_z05",	     110);
		}
		break;
#endif // LDS_EXTENSIONMAP_MONSTERS_AIDA
	}
	return pCharacter;
}

bool M33Aida::MoveAidaMonsterVisual(OBJECT* pObject, BMD* pModel)
{
	switch(pObject->Type)
	{
	case MODEL_MONSTER01+100: //  위치 퀸
		{
			vec3_t Light;
			Vector ( 0.7f, 0.1f, 0.1f, Light );
			AddTerrainLight(pObject->Position[0], pObject->Position[1],Light,3,PrimaryTerrainLight);
		}
		break;
	case MODEL_MONSTER01+101: //  블루 골렘
		{
			vec3_t Light;
			Vector ( 0.f, 0.0f, 0.7f, Light );
			AddTerrainLight(pObject->Position[0], pObject->Position[1],Light,3,PrimaryTerrainLight);
		}
		break;
	case MODEL_MONSTER01+105: // 헬 마이네
		{
			vec3_t Light;
			Vector ( 1.f, 0.0f, 0.0f, Light );
			AddTerrainLight(pObject->Position[0], pObject->Position[1],Light,3,PrimaryTerrainLight);
		}
		break;
#ifdef LDS_EXTENSIONMAP_MONSTERS_AIDA
	case MODEL_MONSTER01+195: //  블러디 골렘
		{
			vec3_t Light;
			Vector ( 0.f, 0.0f, 0.7f, Light );
			AddTerrainLight(pObject->Position[0], pObject->Position[1],Light,3,PrimaryTerrainLight);
		}
		break;
	case MODEL_MONSTER01+196: //  블러디 위치 퀸
		{
			vec3_t Light;
			Vector ( 0.7f, 0.1f, 0.1f, Light );
			AddTerrainLight(pObject->Position[0], pObject->Position[1],Light,3,PrimaryTerrainLight);
		}
		break;
#endif // LDS_EXTENSIONMAP_MONSTERS_AIDA
	}
	return false;
}

void M33Aida::MoveAidaBlurEffect(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel)
{
	switch(pObject->Type)
	{
	case MODEL_MONSTER01+102: //  데스 라이더
		{
			if(pObject->AnimationFrame <= 5.06f && (pObject->CurrentAction == MONSTER01_ATTACK1 || pObject->CurrentAction == MONSTER01_ATTACK2))
			{
				vec3_t  Light;
				Vector(1.0f,1.0f,1.0f,Light);

				vec3_t StartPos, StartRelative;
				vec3_t EndPos, EndRelative;
				
				float fActionSpeed = pModel->Actions[pObject->CurrentAction].PlaySpeed;
				float fSpeedPerFrame = fActionSpeed/10.f;
				float fAnimationFrame = pObject->AnimationFrame - fActionSpeed;
				for(int i=0; i<10; i++) {
					pModel->Animation(BoneTransform,fAnimationFrame,pObject->PriorAnimationFrame,pObject->PriorAction, pObject->Angle, pObject->HeadAngle);

					Vector(20.f, 0.f, 0.f, StartRelative);
					Vector(100.f, 0.f, 0.f, EndRelative);

					pModel->TransformPosition(BoneTransform[18],StartRelative,StartPos,false);
					pModel->TransformPosition(BoneTransform[20],EndRelative,EndPos,false);
					CreateBlur(pCharacter,StartPos,EndPos,Light,3,true,23);

					pModel->TransformPosition(BoneTransform[25],StartRelative,StartPos,false);
					pModel->TransformPosition(BoneTransform[26],EndRelative,EndPos,false);
					CreateBlur(pCharacter,StartPos,EndPos,Light,3,true,24);

					fAnimationFrame += fSpeedPerFrame;
				}
			}
		}
		break;	
	case MODEL_MONSTER01+103: //  포트레스 오크
		{
			if(pObject->CurrentAction == MONSTER01_ATTACK1 || pObject->CurrentAction == MONSTER01_ATTACK2)
			{
				vec3_t  Light;
				Vector(0.5f,0.5f,0.0f,Light);

				vec3_t StartPos, StartRelative;
				vec3_t EndPos, EndRelative;
				
				float fActionSpeed = pModel->Actions[pObject->CurrentAction].PlaySpeed;
				float fSpeedPerFrame = fActionSpeed/10.f;
				float fAnimationFrame = pObject->AnimationFrame - fActionSpeed;
				for(int i=0; i<10; i++) {
					pModel->Animation(BoneTransform,fAnimationFrame,pObject->PriorAnimationFrame,pObject->PriorAction, pObject->Angle, pObject->HeadAngle);

					Vector(20.f, 0.f, 0.f, StartRelative);
					Vector(60.f, 0.f, 0.f, EndRelative);

					pModel->TransformPosition(BoneTransform[35],StartRelative,StartPos,false);
					pModel->TransformPosition(BoneTransform[37],EndRelative,EndPos,false);
					CreateBlur(pCharacter,StartPos,EndPos,Light,3,true,23);
					
					pModel->TransformPosition(BoneTransform[39],StartRelative,StartPos,false);
					pModel->TransformPosition(BoneTransform[41],EndRelative,EndPos,false);
					CreateBlur(pCharacter,StartPos,EndPos,Light,3,true,24);

					pModel->TransformPosition(BoneTransform[43],StartRelative,StartPos,false);
					pModel->TransformPosition(BoneTransform[45],EndRelative,EndPos,false);
					CreateBlur(pCharacter,StartPos,EndPos,Light,3,true,25);

					fAnimationFrame += fSpeedPerFrame;
				}
			}
		}
		break;
	case MODEL_MONSTER01+104: //  데스 트리
		{
			if(pObject->CurrentAction == MONSTER01_ATTACK1 )
			{
				vec3_t  Light;
				Vector(0.3f,0.3f,0.3f,Light);

				vec3_t StartPos, StartRelative;
				vec3_t EndPos, EndRelative;
				
				float fActionSpeed = pModel->Actions[pObject->CurrentAction].PlaySpeed;
				float fSpeedPerFrame = fActionSpeed/10.f;
				float fAnimationFrame = pObject->AnimationFrame - fActionSpeed;
				for(int i=0; i<10; i++) {
					pModel->Animation(BoneTransform,fAnimationFrame,pObject->PriorAnimationFrame,pObject->PriorAction, pObject->Angle, pObject->HeadAngle);

					Vector(0.f, 0.f, 20.f, StartRelative);
					Vector(0.f, 0.f, 70.f, EndRelative);

					pModel->TransformPosition(BoneTransform[26],StartRelative,StartPos,false);
					pModel->TransformPosition(BoneTransform[27],EndRelative,EndPos,false);
					CreateBlur(pCharacter,StartPos,EndPos,Light,3,true,24);
					
					fAnimationFrame += fSpeedPerFrame;
				}
			}
		}
		break;
#ifdef LDS_EXTENSIONMAP_MONSTERS_AIDA
	case MODEL_MONSTER01+193: //  포트레스 오크
		{
			if(pObject->CurrentAction == MONSTER01_ATTACK1 || pObject->CurrentAction == MONSTER01_ATTACK2)
			{
				vec3_t  Light;
				Vector(0.5f,0.5f,0.0f,Light);
				
				vec3_t StartPos, StartRelative;
				vec3_t EndPos, EndRelative;
				
				float fActionSpeed = pModel->Actions[pObject->CurrentAction].PlaySpeed;
				float fSpeedPerFrame = fActionSpeed/10.f;
				float fAnimationFrame = pObject->AnimationFrame - fActionSpeed;
				for(int i=0; i<10; i++) {
					pModel->Animation(BoneTransform,fAnimationFrame,pObject->PriorAnimationFrame,pObject->PriorAction, pObject->Angle, pObject->HeadAngle);
					
					Vector(20.f, 0.f, 0.f, StartRelative);
					Vector(60.f, 0.f, 0.f, EndRelative);
					
					pModel->TransformPosition(BoneTransform[35],StartRelative,StartPos,false);
					pModel->TransformPosition(BoneTransform[37],EndRelative,EndPos,false);
					CreateBlur(pCharacter,StartPos,EndPos,Light,3,true,23);
					
					pModel->TransformPosition(BoneTransform[39],StartRelative,StartPos,false);
					pModel->TransformPosition(BoneTransform[41],EndRelative,EndPos,false);
					CreateBlur(pCharacter,StartPos,EndPos,Light,3,true,24);
					
					pModel->TransformPosition(BoneTransform[43],StartRelative,StartPos,false);
					pModel->TransformPosition(BoneTransform[45],EndRelative,EndPos,false);
					CreateBlur(pCharacter,StartPos,EndPos,Light,3,true,25);
					
					fAnimationFrame += fSpeedPerFrame;
				}
			}
		}
		break;
	case MODEL_MONSTER01+194: //  데스 라이더
		{
			if(pObject->AnimationFrame <= 5.06f && (pObject->CurrentAction == MONSTER01_ATTACK1 || pObject->CurrentAction == MONSTER01_ATTACK2))
			{
				vec3_t  Light;
				Vector(1.0f,1.0f,1.0f,Light);
				
				vec3_t StartPos, StartRelative;
				vec3_t EndPos, EndRelative;
				
				float fActionSpeed = pModel->Actions[pObject->CurrentAction].PlaySpeed;
				float fSpeedPerFrame = fActionSpeed/10.f;
				float fAnimationFrame = pObject->AnimationFrame - fActionSpeed;
				for(int i=0; i<10; i++) {
					pModel->Animation(BoneTransform,fAnimationFrame,pObject->PriorAnimationFrame,pObject->PriorAction, pObject->Angle, pObject->HeadAngle);
					
					Vector(20.f, 0.f, 0.f, StartRelative);
					Vector(100.f, 0.f, 0.f, EndRelative);
					
					pModel->TransformPosition(BoneTransform[18],StartRelative,StartPos,false);
					pModel->TransformPosition(BoneTransform[20],EndRelative,EndPos,false);
					CreateBlur(pCharacter,StartPos,EndPos,Light,3,true,23);
					
					pModel->TransformPosition(BoneTransform[25],StartRelative,StartPos,false);
					pModel->TransformPosition(BoneTransform[26],EndRelative,EndPos,false);
					CreateBlur(pCharacter,StartPos,EndPos,Light,3,true,24);
					
					fAnimationFrame += fSpeedPerFrame;
				}
			}
		}
		break;	
#endif // LDS_EXTENSIONMAP_MONSTERS_AIDA
	}
}

bool M33Aida::RenderAidaMonsterVisual(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel)
{
	switch(pObject->Type)
	{
	case MODEL_MONSTER01+100: // 위치 퀸
		{
			vec3_t Position, Light, Angle;
			float Random_Light = (float)(rand()%30)/100.0f + 0.6f;
			Vector ( Random_Light+0.5f, Random_Light-0.05f, Random_Light+0.5f, Light );

			BoneManager::GetBonePosition(pObject, "Monster100_L_Hand", Position);
			CreateSprite(BITMAP_LIGHT+1,Position,0.8f,Light,pObject);
			BoneManager::GetBonePosition(pObject, "Monster100_z02", Position);
			CreateSprite(BITMAP_LIGHT+1,Position,1.1f,Light,pObject);
			BoneManager::GetBonePosition(pObject, "Monster100_z03", Position);
			CreateSprite(BITMAP_LIGHT+1,Position,1.0f,Light,pObject);
			BoneManager::GetBonePosition(pObject, "Monster100_z04", Position);
			CreateSprite(BITMAP_LIGHT+1,Position,1.7f,Light,pObject);
			BoneManager::GetBonePosition(pObject, "Monster100_z05", Position);
			CreateSprite(BITMAP_LIGHT+1,Position,0.8f,Light,pObject);

			Vector ( 0.7f, 0.5f, 0.7f, Light );
			vec3_t Relative = { 0.0f, 0.0f, -65.0f };
			BoneManager::GetBonePosition(pObject, "Monster100_Footstepst", Relative, Position);
			CreateParticle(BITMAP_LIGHT+1,Position, Angle, Light, 4, 4.0f);

			//사운드//////////////////////////////////////////////////////////////////////////////////
			if(pObject->CurrentAction == MONSTER01_WALK || pObject->CurrentAction == MONSTER01_RUN)
			{
				if(rand()%15==0)
					PlayBuffer ( SOUND_AIDA_WITCHQUEEN_MOVE1+rand()%2 );
			}
			else if(pObject->CurrentAction == MONSTER01_ATTACK1 ) 
			{
				if(pObject->SubType == FALSE) {
					pObject->SubType = TRUE;
					PlayBuffer ( SOUND_AIDA_WITCHQUEEN_ATTACK2 );
				}
			}
			else if(pObject->CurrentAction == MONSTER01_ATTACK2)
			{
				if(pObject->SubType == FALSE) {
					pObject->SubType = TRUE;
					PlayBuffer ( SOUND_CHAOS_THUNDER01+rand()%2 );
					PlayBuffer ( SOUND_AIDA_WITCHQUEEN_ATTACK1 );
				}
			}
			else if(pObject->CurrentAction == MONSTER01_DIE)
			{
				if(pObject->SubType == FALSE) {
					pObject->SubType = TRUE;
					PlayBuffer ( SOUND_AIDA_WITCHQUEEN_DIE );
				}
			}
			//////////////////////////////////////////////////////////////////////////////////////////

			if(pObject->CurrentAction == MONSTER01_ATTACK1 || pObject->CurrentAction == MONSTER01_ATTACK2)
			{
		
				if(pObject->AnimationFrame >= 4.0f)
				{
					vec3_t Relative = { 70.0f, 0.0f, 0.0f };
					BoneManager::GetBonePosition(pObject, "Monster100_Footstepst", Relative, Position);
					
					Vector ( 0.5f, 0.2f, 0.5f, Light );
					CreateParticle(BITMAP_SMOKE, Position, Angle, Light, 27, 2.0f);
					Vector ( 1.0f, 1.0f, 1.0f, Light );
					CreateParticle(BITMAP_LIGHT+1,Position, Angle, Light, 2, 1.0f);
				}
				if(pObject->AnimationFrame >= 6.0f && pObject->CurrentAction == MONSTER01_ATTACK1)
				{
					vec3_t Position, Light, Angle = {0.0f, 0.0f, 0.0f};	
					vec3_t Relative = { 70.0f, 0.0f, 0.0f };
					BoneManager::GetBonePosition(pObject, "Monster100_Footstepst", Relative, Position);
					Vector ( 1.0f, 1.0f, 1.0f, Light );
					CreateParticle(BITMAP_LIGHT+1,Position, Angle, Light, 3, 1.3f);
				}
                if ( pCharacter->TargetCharacter>=0 && pCharacter->TargetCharacter<MAX_CHARACTERS_CLIENT )
                {
                    CHARACTER *tc = &CharactersClient[pCharacter->TargetCharacter];
                    OBJECT *to = &tc->Object;

					vec3_t vTemp;
					VectorCopy(to->Position, vTemp );
					vTemp[2] += 100.0f;

					if(pObject->AnimationFrame >= 6.0f && pObject->CurrentAction == MONSTER01_ATTACK1)
					{
						CreateParticle(BITMAP_LIGHT+1,vTemp, Angle, Light, 3, 1.3f);  // 위치 퀸 챰 맞은 효과
					}
					if(pObject->AnimationFrame >= 6.0f && pObject->CurrentAction == MONSTER01_ATTACK2)
					{
						CreateJoint(BITMAP_JOINT_THUNDER, to->Position, vTemp, pObject->Angle, 16);// 위치 퀸 썬더 맞은 효과
					}
				}
			}

			if(pObject->CurrentAction == MONSTER01_STOP1 || pObject->CurrentAction == MONSTER01_STOP2)
				pObject->SubType = FALSE;			
		}
		break;
	case MODEL_MONSTER01+101: //  블루 골렘
		{
			vec3_t Position, Light, Angle = {0.0f, 0.0f, 0.0f};
			Vector ( 1.0f, 1.0f, 1.0f, Light );		

			BoneManager::GetBonePosition(pObject, "Monster101_L_Arm", Position);
			CreateParticle(BITMAP_WATERFALL_2, Position, Angle, Light, 1);
			BoneManager::GetBonePosition(pObject, "Monster101_R_Arm", Position);
			CreateParticle(BITMAP_WATERFALL_2, Position, Angle, Light, 1);
			BoneManager::GetBonePosition(pObject, "Monster101_Head", Position);
			CreateParticle(BITMAP_WATERFALL_2, Position, Angle, Light, 1);

			//사운드//////////////////////////////////////////////////////////////////////////////////
			if(pObject->CurrentAction == MONSTER01_WALK || pObject->CurrentAction == MONSTER01_RUN)
			{
				if(rand()%15==0)
					PlayBuffer ( SOUND_AIDA_BLUEGOLEM_MOVE1+rand()%2 );
			}
			else if(pObject->CurrentAction == MONSTER01_ATTACK1 ) 
			{
				if(pObject->SubType == FALSE) {
					pObject->SubType = TRUE;
					PlayBuffer ( SOUND_AIDA_BLUEGOLEM_ATTACK1 );
				}
			}
			else if(pObject->CurrentAction == MONSTER01_ATTACK2)
			{
				if(pObject->SubType == FALSE) {
					pObject->SubType = TRUE;
					PlayBuffer ( SOUND_AIDA_BLUEGOLEM_ATTACK2 );
				}
			}
			else if(pObject->CurrentAction == MONSTER01_DIE)
			{
				if(pObject->SubType == FALSE) {
					pObject->SubType = TRUE;
					PlayBuffer ( SOUND_AIDA_BLUEGOLEM_DIE );
				}
			}
			//////////////////////////////////////////////////////////////////////////////////////////

			if(pObject->CurrentAction == MONSTER01_ATTACK1)
			{
				vec3_t Position, Light, Angle = {0.0f, 0.0f, 0.0f};
				Vector ( 1.0f, 1.0f, 1.0f, Light );		
				if(pObject->AnimationFrame >= 4.0f)
				{
					Vector ( 0.0f, 45.0f, 45.0f, Angle );
					vec3_t Relative = { 30.0f, 0.0f, 0.0f };
					BoneManager::GetBonePosition(pObject, "Monster101_L_Arm", Relative, Position);
					CreateParticle(BITMAP_SMOKE, Position, Angle, Light, 25);

					if(pObject->AnimationFrame >= 5.0f && pObject->AnimationFrame <= 5.5f)
					{
						Vector ( 5.0f, 5.0f, 5.0f, Light );		
						BoneManager::GetBonePosition(pObject, "Monster101_L_Arm", Relative, Position);
						CreateParticle(BITMAP_SHOCK_WAVE, Position, Angle, Light, 3, 0.5f);
					}
				}
				
				if ( pCharacter->TargetCharacter>=0 && pCharacter->TargetCharacter<MAX_CHARACTERS_CLIENT && pObject->AnimationFrame >= 6.9f)
				{
					CHARACTER *tc = &CharactersClient[pCharacter->TargetCharacter];
					OBJECT *to = &tc->Object;

					vec3_t vTemp;
					VectorCopy(to->Position, vTemp );
					vTemp[2] += 100.0f;

					CreateJoint(BITMAP_JOINT_ENERGY,vTemp, pObject->Position, to->Angle,16,pObject,20.0f);  // 블루 골렘 : 마나 스틸 맞는 효과
				}
			}
			
			if(pObject->CurrentAction == MONSTER01_STOP1 || pObject->CurrentAction == MONSTER01_STOP2)
				pObject->SubType = FALSE;
		}
		break;
	case MODEL_MONSTER01+102: //  데스 라이더
		{
			vec3_t Relative, Position, Light, Angle = {0.0f, 0.0f, 0.0f};
			
			float Random_Light = (float)(rand()%10)/100.0f + 0.2f;
			Vector ( 60.0f, -30.0f, 0.0f, Relative );		
			Vector ( 0.0f, Random_Light, 0.0f, Light );

			BoneManager::GetBonePosition(pObject, "Monster102_Head", Relative, Position);
			CreateSprite(BITMAP_LIGHT+1,Position,3.0f,Light,pObject);
			CreateParticle(BITMAP_SPARK+1, Position, Angle, Light, 7);

			//사운드//////////////////////////////////////////////////////////////////////////////////
			if(pObject->CurrentAction == MONSTER01_WALK || pObject->CurrentAction == MONSTER01_RUN)
			{
				if(rand()%15==0)
					PlayBuffer ( SOUND_AIDA_DEATHRAIDER_MOVE1+rand()%2 );
			}
			else if(pObject->CurrentAction == MONSTER01_ATTACK1 ) 
			{
				if(pObject->SubType == FALSE) {
					pObject->SubType = TRUE;
					PlayBuffer ( SOUND_AIDA_DEATHRAIDER_ATTACK1 );
				}
			}
			else if(pObject->CurrentAction == MONSTER01_ATTACK2)
			{
				if(pObject->SubType == FALSE) {
					pObject->SubType = TRUE;
					PlayBuffer ( SOUND_AIDA_DEATHRAIDER_ATTACK2 );
				}
			}
			else if(pObject->CurrentAction == MONSTER01_DIE)
			{
				if(pObject->SubType == FALSE) {
					pObject->SubType = TRUE;
					PlayBuffer ( SOUND_AIDA_DEATHRAIDER_DIE );
				}
			}

			if(pObject->CurrentAction == MONSTER01_STOP1 || pObject->CurrentAction == MONSTER01_STOP2)
				pObject->SubType = FALSE;
			//////////////////////////////////////////////////////////////////////////////////////////
		}
		break;

	case MODEL_MONSTER01+103: //  포트레스 오크
		{
			//사운드//////////////////////////////////////////////////////////////////////////////////
			if(pObject->CurrentAction == MONSTER01_WALK || pObject->CurrentAction == MONSTER01_RUN)
			{
				if(rand()%15==0)
					PlayBuffer ( SOUND_AIDA_FORESTORC_MOVE1+rand()%2 );
			}
			else if(pObject->CurrentAction == MONSTER01_ATTACK1 ) 
			{
				if(pObject->SubType == FALSE) {
					pObject->SubType = TRUE;
					PlayBuffer ( SOUND_AIDA_FORESTORC_ATTACK1 );
				}
			}
			else if(pObject->CurrentAction == MONSTER01_ATTACK2)
			{
				if(pObject->SubType == FALSE) {
					pObject->SubType = TRUE;
					PlayBuffer ( SOUND_AIDA_FORESTORC_ATTACK2 );
				}
			}
			else if(pObject->CurrentAction == MONSTER01_DIE)
			{
				if(pObject->SubType == FALSE) {
					pObject->SubType = TRUE;
					PlayBuffer ( SOUND_AIDA_FORESTORC_DIE );
				}
			}

			if(pObject->CurrentAction == MONSTER01_STOP1 || pObject->CurrentAction == MONSTER01_STOP2)
				pObject->SubType = FALSE;
			//////////////////////////////////////////////////////////////////////////////////////////
		}
		break;
	case MODEL_MONSTER01+104: //  데스 트리
		{

			//사운드//////////////////////////////////////////////////////////////////////////////////
			if(pObject->CurrentAction == MONSTER01_WALK || pObject->CurrentAction == MONSTER01_RUN)
			{
				if(rand()%15==0)
					PlayBuffer ( SOUND_AIDA_DEATHTREE_MOVE1+rand()%2 );
			}
			else if(pObject->CurrentAction == MONSTER01_ATTACK1 ) 
			{
				if(pObject->SubType == FALSE) {
					pObject->SubType = TRUE;
					PlayBuffer ( SOUND_AIDA_DEATHTREE_ATTACK1 );
				}
			}
			else if(pObject->CurrentAction == MONSTER01_ATTACK2)
			{
				if(pObject->SubType == FALSE) {
					pObject->SubType = TRUE;
					PlayBuffer ( SOUND_AIDA_DEATHTREE_ATTACK2 );
				}
			}
			else if(pObject->CurrentAction == MONSTER01_DIE)
			{
				if(pObject->SubType == FALSE) {
					pObject->SubType = TRUE;
					PlayBuffer ( SOUND_AIDA_DEATHTREE_DIE );
				}
			}
			//////////////////////////////////////////////////////////////////////////////////////////

			vec3_t Position, Light, Angle = {0.0f, 0.0f, 0.0f};
			float Random_Light;

			for(int i = 0; i < 6; i++)
			{
				char Bone_Name[256];
				sprintf(Bone_Name, "Monster104_Horn%d", i);
				Random_Light = (float)(rand()%10)/100.0f + 0.8f;
				Vector ( 0.4f, Random_Light, 0.5f, Light );
				BoneManager::GetBonePosition(pObject, Bone_Name, Position);
				CreateSprite(BITMAP_LIGHT+1,Position,0.4f,Light,pObject);
				CreateParticle(BITMAP_SPARK+1, Position, Angle, Light, 6);
			}
			if(pObject->CurrentAction == MONSTER01_ATTACK2)
			{
				vec3_t Position, Relative = { 0.0f, -100.0f, 20.0f }, Light = { 0.5f, 0.7f, 0.5f };
				BoneManager::GetBonePosition(pObject, "Monster104_Footsteps", Relative, Position);
				CreateParticle(BITMAP_SMOKE, Position, pObject->Angle, Light, 26);
			}

			if(pObject->CurrentAction == MONSTER01_STOP1 || pObject->CurrentAction == MONSTER01_STOP2)
				pObject->SubType = FALSE;
		}
		break;
	case MODEL_MONSTER01+105: // 헬 마이네
		{
			vec3_t Relative, Position, Light, Angle = {0.0f, 0.0f, 0.0f};
			float Random_Light;

			Random_Light = (float)(rand()%8)/10.0f + 0.6f;
			Vector ( Random_Light, 0.0f, 0.0f, Light );

			BoneManager::GetBonePosition(pObject, "Monster105_R_Eye", Position);
			CreateSprite(BITMAP_LIGHT+1,Position,0.3f,Light,pObject);

			BoneManager::GetBonePosition(pObject, "Monster105_L_Eye", Position);
			CreateSprite(BITMAP_LIGHT+1,Position,0.3f,Light,pObject);
			
			BoneManager::GetBonePosition(pObject, "Monster105_L_Arm00", Position);
			CreateSprite(BITMAP_LIGHT+1,Position,1.0f,Light,pObject);

			BoneManager::GetBonePosition(pObject, "Monster105_L_Arm01", Position);
			CreateSprite(BITMAP_LIGHT+1,Position,1.0f,Light,pObject);
			
			BoneManager::GetBonePosition(pObject, "Monster105_L_Arm02", Position);
			CreateSprite(BITMAP_LIGHT+1,Position,1.0f,Light,pObject);


			//사운드//////////////////////////////////////////////////////////////////////////////////
			if(pObject->CurrentAction == MONSTER01_WALK || pObject->CurrentAction == MONSTER01_RUN)
			{
				if(rand()%15==0)
					PlayBuffer ( SOUND_AIDA_HELL_MOVE1+rand()%2 );
			}
			else if(pObject->CurrentAction == MONSTER01_ATTACK1 ) 
			{
				if(pObject->SubType == FALSE) {
					pObject->SubType = TRUE;
					PlayBuffer ( SOUND_AIDA_HELL_ATTACK3 );
				}
			}
			else if(pObject->CurrentAction == MONSTER01_ATTACK2)
			{
				if(pObject->SubType == FALSE) {
					pObject->SubType = TRUE;
					PlayBuffer ( SOUND_AIDA_HELL_ATTACK2 );
				}
			}
			else if(pObject->CurrentAction == MONSTER01_ATTACK3)
			{
				if(pObject->SubType == FALSE) {
					pObject->SubType = TRUE;
					PlayBuffer ( SOUND_AIDA_HELL_ATTACK1 );
				}
			}			
			else if(pObject->CurrentAction == MONSTER01_DIE)
			{
				if(pObject->SubType == FALSE) {
					pObject->SubType = TRUE;
					PlayBuffer ( SOUND_AIDA_HELL_DIE );
				}
			}
			//////////////////////////////////////////////////////////////////////////////////////////

			if(pObject->CurrentAction == MONSTER01_ATTACK2)
			{
				BoneManager::GetBonePosition(pObject, "Monster105_L_Hand", Position);
				CreateParticle(BITMAP_TRUE_FIRE,Position, Angle, Light, 6, 5.0f);

				BoneManager::GetBonePosition(pObject, "Monster105_R_Hand", Position);
				CreateParticle(BITMAP_TRUE_FIRE,Position, Angle, Light, 6, 5.0f);
			}
			else
			{
				BoneManager::GetBonePosition(pObject, "Monster105_L_Hand", Position);
				CreateParticle(BITMAP_TRUE_FIRE,Position, Angle, Light, 6, 2.5f);

				Vector(0.0f, 0.0f, -10.0f, Relative);
				BoneManager::GetBonePosition(pObject, "Monster105_R_Hand", Relative, Position);
				CreateParticle(BITMAP_TRUE_FIRE,Position, Angle, Light, 6, 2.1f);
			}
			if((pObject->AnimationFrame >= 9.0f && pObject->AnimationFrame <= 10.0f) && pObject->CurrentAction == MONSTER01_ATTACK2)
			{
				CreateEffect(BITMAP_BOSS_LASER,Position,pObject->Angle,Light,1);
			}

//#ifdef AIDA_DAMAGE
            if ( (pCharacter->TargetCharacter>=0 && pCharacter->TargetCharacter<MAX_CHARACTERS_CLIENT))
            {
/*                CHARACTER *tc = &CharactersClient[pCharacter->TargetCharacter];
                OBJECT *to = &tc->Object;
			
				vec3_t p;
				Vector(1.0f, 1.0f, 1.0f, Light);
				BMD* pTModel = &Models[to->Type];
				
				Vector(0.0f, 20.0f, 50.0f, p);
				pTModel->TransformPosition(to->BoneTransform[0],p,p,false);
				VectorAdd(p, to->Position, p)
				CreateParticle(BITMAP_TRUE_FIRE ,p, Angle, Light, 7, 1.0f);  // 헬 마이네 불 장풍 맞은 효과

				Vector(0.0f, -5.0f, 20.0f, p);
				pTModel->TransformPosition(to->BoneTransform[0],p,p,false);
				VectorAdd(p, to->Position, p)
				CreateParticle(BITMAP_TRUE_FIRE ,p, Angle, Light, 7, 1.0f);  // 헬 마이네 불 장풍 맞은 효과

				Vector(0.0f, 15.0f, -20.0f, p);
				pTModel->TransformPosition(to->BoneTransform[0],p,p,false);
				VectorAdd(p, to->Position, p)
				CreateParticle(BITMAP_TRUE_FIRE ,p, Angle, Light, 7, 1.0f);  // 헬 마이네 불 장풍 맞은 효과
*/
/*				vTemp[1] -= 20.0f;
				vTemp[2] += 80.0f;
				CreateParticle(BITMAP_TRUE_FIRE ,vTemp, Angle, Light, 7, 0.6f);  // 헬 마이네 불 장풍 맞은 효과
				vTemp[1] += 40.0f;
				vTemp[2] += 20.0f;
				CreateParticle(BITMAP_TRUE_FIRE ,vTemp, Angle, Light, 7, 0.6f);  // 헬 마이네 불 장풍 맞은 효과
				vTemp[1] -= 40.0f;
				vTemp[2] += 50.0f;
				CreateParticle(BITMAP_TRUE_FIRE ,vTemp, Angle, Light, 7, 0.6f);  // 헬 마이네 불 장풍 맞은 효과
*/			}
//#endif //AIDA_DAMAGE

			if(pObject->CurrentAction == MONSTER01_STOP1 || pObject->CurrentAction == MONSTER01_STOP2)
				pObject->SubType = FALSE;
		}
		break;
#ifdef LDS_EXTENSIONMAP_MONSTERS_AIDA
	case MODEL_MONSTER01+193: //  블러디 포트레스 오크
		{
			//사운드//////////////////////////////////////////////////////////////////////////////////
			if(pObject->CurrentAction == MONSTER01_WALK || pObject->CurrentAction == MONSTER01_RUN)
			{
				if(rand()%15==0)
					PlayBuffer ( SOUND_AIDA_FORESTORC_MOVE1+rand()%2 );
			}
			else if(pObject->CurrentAction == MONSTER01_ATTACK1 ) 
			{
				if(pObject->SubType == FALSE) {
					pObject->SubType = TRUE;
					PlayBuffer ( SOUND_AIDA_FORESTORC_ATTACK1 );
				}
			}
			else if(pObject->CurrentAction == MONSTER01_ATTACK2)
			{
				if(pObject->SubType == FALSE) {
					pObject->SubType = TRUE;
					PlayBuffer ( SOUND_AIDA_FORESTORC_ATTACK2 );
				}
			}
			else if(pObject->CurrentAction == MONSTER01_DIE)
			{
				if(pObject->SubType == FALSE) {
					pObject->SubType = TRUE;
					PlayBuffer ( SOUND_AIDA_FORESTORC_DIE );
				}
			}
			
			if(pObject->CurrentAction == MONSTER01_STOP1 || pObject->CurrentAction == MONSTER01_STOP2)
				pObject->SubType = FALSE;
			//////////////////////////////////////////////////////////////////////////////////////////
		}
		break;
	case MODEL_MONSTER01+194: //  블러디 데스 라이더
		{
			vec3_t Relative, Position, Light, Angle = {0.0f, 0.0f, 0.0f};
			
			float Random_Light = (float)(rand()%10)/100.0f + 0.2f;
			Vector ( 60.0f, -30.0f, 0.0f, Relative );		
			Vector ( 0.0f, Random_Light, 0.0f, Light );
			
			BoneManager::GetBonePosition(pObject, "Monster102_Head", Relative, Position);
			CreateSprite(BITMAP_LIGHT+1,Position,3.0f,Light,pObject);
			CreateParticle(BITMAP_SPARK+1, Position, Angle, Light, 7);
			
			//사운드//////////////////////////////////////////////////////////////////////////////////
			if(pObject->CurrentAction == MONSTER01_WALK || pObject->CurrentAction == MONSTER01_RUN)
			{
				if(rand()%15==0)
					PlayBuffer ( SOUND_AIDA_DEATHRAIDER_MOVE1+rand()%2 );
			}
			else if(pObject->CurrentAction == MONSTER01_ATTACK1 ) 
			{
				if(pObject->SubType == FALSE) {
					pObject->SubType = TRUE;
					PlayBuffer ( SOUND_AIDA_DEATHRAIDER_ATTACK1 );
				}
			}
			else if(pObject->CurrentAction == MONSTER01_ATTACK2)
			{
				if(pObject->SubType == FALSE) {
					pObject->SubType = TRUE;
					PlayBuffer ( SOUND_AIDA_DEATHRAIDER_ATTACK2 );
				}
			}
			else if(pObject->CurrentAction == MONSTER01_DIE)
			{
				if(pObject->SubType == FALSE) {
					pObject->SubType = TRUE;
					PlayBuffer ( SOUND_AIDA_DEATHRAIDER_DIE );
				}
			}
			
			if(pObject->CurrentAction == MONSTER01_STOP1 || pObject->CurrentAction == MONSTER01_STOP2)
				pObject->SubType = FALSE;
			//////////////////////////////////////////////////////////////////////////////////////////
		}
		break;

	case MODEL_MONSTER01+195: //  블러디 골렘
		{
			vec3_t Position, Light, Angle = {0.0f, 0.0f, 0.0f};
			Vector ( 1.0f, 1.0f, 1.0f, Light );		

			BoneManager::GetBonePosition(pObject, "Monster101_L_Arm", Position);
			CreateParticle(BITMAP_WATERFALL_2, Position, Angle, Light, 1);
			BoneManager::GetBonePosition(pObject, "Monster101_R_Arm", Position);
			CreateParticle(BITMAP_WATERFALL_2, Position, Angle, Light, 1);
			BoneManager::GetBonePosition(pObject, "Monster101_Head", Position);
			CreateParticle(BITMAP_WATERFALL_2, Position, Angle, Light, 1);

			//사운드//////////////////////////////////////////////////////////////////////////////////
			if(pObject->CurrentAction == MONSTER01_WALK || pObject->CurrentAction == MONSTER01_RUN)
			{
				if(rand()%15==0)
					PlayBuffer ( SOUND_AIDA_BLUEGOLEM_MOVE1+rand()%2 );
			}
			else if(pObject->CurrentAction == MONSTER01_ATTACK1 ) 
			{
				if(pObject->SubType == FALSE) {
					pObject->SubType = TRUE;
					PlayBuffer ( SOUND_AIDA_BLUEGOLEM_ATTACK1 );
				}
			}
			else if(pObject->CurrentAction == MONSTER01_ATTACK2)
			{
				if(pObject->SubType == FALSE) {
					pObject->SubType = TRUE;
					PlayBuffer ( SOUND_AIDA_BLUEGOLEM_ATTACK2 );
				}
			}
			else if(pObject->CurrentAction == MONSTER01_DIE)
			{
				if(pObject->SubType == FALSE) {
					pObject->SubType = TRUE;
					PlayBuffer ( SOUND_AIDA_BLUEGOLEM_DIE );
				}
			}
			//////////////////////////////////////////////////////////////////////////////////////////

			if(pObject->CurrentAction == MONSTER01_ATTACK1)
			{
				vec3_t Position, Light, Angle = {0.0f, 0.0f, 0.0f};
				Vector ( 1.0f, 1.0f, 1.0f, Light );		
				if(pObject->AnimationFrame >= 4.0f)
				{
					Vector ( 0.0f, 45.0f, 45.0f, Angle );
					vec3_t Relative = { 30.0f, 0.0f, 0.0f };
					BoneManager::GetBonePosition(pObject, "Monster101_L_Arm", Relative, Position);
					CreateParticle(BITMAP_SMOKE, Position, Angle, Light, 25);

					if(pObject->AnimationFrame >= 5.0f && pObject->AnimationFrame <= 5.5f)
					{
						Vector ( 5.0f, 5.0f, 5.0f, Light );		
						BoneManager::GetBonePosition(pObject, "Monster101_L_Arm", Relative, Position);
						CreateParticle(BITMAP_SHOCK_WAVE, Position, Angle, Light, 3, 0.5f);
					}
				}
				
				if ( pCharacter->TargetCharacter>=0 && pCharacter->TargetCharacter<MAX_CHARACTERS_CLIENT && pObject->AnimationFrame >= 6.9f)
				{
					CHARACTER *tc = &CharactersClient[pCharacter->TargetCharacter];
					OBJECT *to = &tc->Object;

					vec3_t vTemp;
					VectorCopy(to->Position, vTemp );
					vTemp[2] += 100.0f;

					CreateJoint(BITMAP_JOINT_ENERGY,vTemp, pObject->Position, to->Angle,16,pObject,20.0f);  // 블루 골렘 : 마나 스틸 맞는 효과
				}
			}
			
			if(pObject->CurrentAction == MONSTER01_STOP1 || pObject->CurrentAction == MONSTER01_STOP2)
				pObject->SubType = FALSE;
		}
		break;
	case MODEL_MONSTER01+196: // 위치 퀸
		{
			vec3_t Position, Light, Angle;
			float Random_Light = (float)(rand()%30)/100.0f + 0.6f;
			Vector ( Random_Light+0.5f, Random_Light-0.05f, Random_Light+0.5f, Light );
			
			BoneManager::GetBonePosition(pObject, "Monster100_L_Hand", Position);
			CreateSprite(BITMAP_LIGHT+1,Position,0.8f,Light,pObject);
			BoneManager::GetBonePosition(pObject, "Monster100_z02", Position);
			CreateSprite(BITMAP_LIGHT+1,Position,1.1f,Light,pObject);
			BoneManager::GetBonePosition(pObject, "Monster100_z03", Position);
			CreateSprite(BITMAP_LIGHT+1,Position,1.0f,Light,pObject);
			BoneManager::GetBonePosition(pObject, "Monster100_z04", Position);
			CreateSprite(BITMAP_LIGHT+1,Position,1.7f,Light,pObject);
			BoneManager::GetBonePosition(pObject, "Monster100_z05", Position);
			CreateSprite(BITMAP_LIGHT+1,Position,0.8f,Light,pObject);
			
			Vector ( 0.7f, 0.5f, 0.7f, Light );
			vec3_t Relative = { 0.0f, 0.0f, -65.0f };
			BoneManager::GetBonePosition(pObject, "Monster100_Footstepst", Relative, Position);
			CreateParticle(BITMAP_LIGHT+1,Position, Angle, Light, 4, 4.0f);
			
			//사운드//////////////////////////////////////////////////////////////////////////////////
			if(pObject->CurrentAction == MONSTER01_WALK || pObject->CurrentAction == MONSTER01_RUN)
			{
				if(rand()%15==0)
					PlayBuffer ( SOUND_AIDA_WITCHQUEEN_MOVE1+rand()%2 );
			}
			else if(pObject->CurrentAction == MONSTER01_ATTACK1 ) 
			{
				if(pObject->SubType == FALSE) {
					pObject->SubType = TRUE;
					PlayBuffer ( SOUND_AIDA_WITCHQUEEN_ATTACK2 );
				}
			}
			else if(pObject->CurrentAction == MONSTER01_ATTACK2)
			{
				if(pObject->SubType == FALSE) {
					pObject->SubType = TRUE;
					PlayBuffer ( SOUND_CHAOS_THUNDER01+rand()%2 );
					PlayBuffer ( SOUND_AIDA_WITCHQUEEN_ATTACK1 );
				}
			}
			else if(pObject->CurrentAction == MONSTER01_DIE)
			{
				if(pObject->SubType == FALSE) {
					pObject->SubType = TRUE;
					PlayBuffer ( SOUND_AIDA_WITCHQUEEN_DIE );
				}
			}
			//////////////////////////////////////////////////////////////////////////////////////////
			
			if(pObject->CurrentAction == MONSTER01_ATTACK1 || pObject->CurrentAction == MONSTER01_ATTACK2)
			{
				if(pObject->AnimationFrame >= 4.0f)
				{
					vec3_t Relative = { 70.0f, 0.0f, 0.0f };
					BoneManager::GetBonePosition(pObject, "Monster100_Footstepst", Relative, Position);
					
					Vector ( 0.5f, 0.2f, 0.5f, Light );
					CreateParticle(BITMAP_SMOKE, Position, Angle, Light, 27, 2.0f);
					Vector ( 1.0f, 1.0f, 1.0f, Light );
					CreateParticle(BITMAP_LIGHT+1,Position, Angle, Light, 2, 1.0f);
				}
				if(pObject->AnimationFrame >= 6.0f && pObject->CurrentAction == MONSTER01_ATTACK1)
				{
					vec3_t Position, Light, Angle = {0.0f, 0.0f, 0.0f};	
					vec3_t Relative = { 70.0f, 0.0f, 0.0f };
					BoneManager::GetBonePosition(pObject, "Monster100_Footstepst", Relative, Position);
					Vector ( 1.0f, 1.0f, 1.0f, Light );
					CreateParticle(BITMAP_LIGHT+1,Position, Angle, Light, 3, 1.3f);
				}
                if ( pCharacter->TargetCharacter>=0 && pCharacter->TargetCharacter<MAX_CHARACTERS_CLIENT )
                {
                    CHARACTER *tc = &CharactersClient[pCharacter->TargetCharacter];
                    OBJECT *to = &tc->Object;
					
					vec3_t vTemp;
					VectorCopy(to->Position, vTemp );
					vTemp[2] += 100.0f;
					
					if(pObject->AnimationFrame >= 6.0f && pObject->CurrentAction == MONSTER01_ATTACK1)
					{
						CreateParticle(BITMAP_LIGHT+1,vTemp, Angle, Light, 3, 1.3f);  // 위치 퀸 챰 맞은 효과
					}
					if(pObject->AnimationFrame >= 6.0f && pObject->CurrentAction == MONSTER01_ATTACK2)
					{
#ifdef LDS_MOD_BLOODWITCHQUEEN_EFFECT_ATTACK2
						vec3_t		v3Light;
						float		fScale = 1.8f;
						Vector(1.0f, 0.0f, 0.4f, v3Light);
						CreateParticle(BITMAP_LIGHT+1,vTemp, Angle, v3Light, 3, fScale);  // 위치 퀸 챰 맞은 효과
#else // LDS_MOD_BLOODWITCHQUEEN_EFFECT_ATTACK2
						CreateJoint(BITMAP_JOINT_THUNDER, to->Position, vTemp, pObject->Angle, 16);// 위치 퀸 썬더 맞은 효과
#endif // LDS_MOD_BLOODWITCHQUEEN_EFFECT_ATTACK2
					}
				}
			}
			
			if(pObject->CurrentAction == MONSTER01_STOP1 || pObject->CurrentAction == MONSTER01_STOP2)
				pObject->SubType = FALSE;			
		}
		break;
#endif // LDS_EXTENSIONMAP_MONSTERS_AIDA
	}
	return false;
}

bool M33Aida::RenderAidaMonsterObjectMesh(OBJECT* pObject, BMD* pModel,bool ExtraMon)
{
	switch(pObject->Type)
	{
	case MODEL_MONSTER01+101: //  블루 골렘
		{
			pModel->BeginRender(1.f);

			pModel->BodyLight[0] = 0.9f;
			pModel->BodyLight[1] = 0.9f;
			pModel->BodyLight[2] = 0.9f;
			
			pModel->StreamMesh = 0;
			pModel->RenderMesh ( 0, RENDER_TEXTURE, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, -(int)WorldTime%10000*0.0003f, -(int)WorldTime%10000*0.0003f );
			pModel->RenderMesh ( 0, RENDER_TEXTURE|RENDER_BRIGHT|RENDER_CHROME, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV );
			pModel->StreamMesh = -1;
			pModel->RenderMesh ( 1, RENDER_TEXTURE, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV );

			pModel->EndRender();

			return true;
		}
		break;
	case MODEL_MONSTER01+102: //  데쓰 라이더
		{
			pModel->BeginRender(1.f);

			pModel->BodyLight[0] = 0.9f;
			pModel->BodyLight[1] = 0.9f;
			pModel->BodyLight[2] = 0.9f;
			
			pModel->RenderMesh ( 0, RENDER_TEXTURE, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV );
			pModel->RenderMesh ( 1, RENDER_TEXTURE, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV );
			pModel->RenderMesh ( 2, RENDER_TEXTURE, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV );
			pModel->StreamMesh = 0;
			pModel->RenderMesh ( 3, RENDER_TEXTURE, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, -(int)WorldTime%10000*0.0006f, -(int)WorldTime%10000*0.0006f );
			pModel->RenderMesh ( 3, RENDER_TEXTURE|RENDER_BRIGHT|RENDER_CHROME, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV );
			pModel->StreamMesh = -1;

			pModel->EndRender();
			
			return true;
		}
		break;
	case MODEL_MONSTER01+105: //  헬 마이네
		{
			pModel->BeginRender(1.f);

			pModel->BodyLight[0] = 1.0f;
			pModel->BodyLight[1] = 1.0f;
			pModel->BodyLight[2] = 1.0f;
			
			pModel->StreamMesh = 0;
			pModel->RenderMesh ( 0, RENDER_TEXTURE, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV );
			pModel->BodyLight[0] = 1.0f;
			pModel->BodyLight[1] = 0.0f;
			pModel->BodyLight[2] = 0.0f;
			pModel->RenderMesh ( 0, RENDER_TEXTURE|RENDER_BRIGHT|RENDER_CHROME, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV );
			pModel->StreamMesh = -1;

			pModel->BodyLight[0] = 1.0f;
			pModel->BodyLight[1] = 1.0f;
			pModel->BodyLight[2] = 1.0f;			
			pModel->RenderMesh ( 1, RENDER_TEXTURE, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV );

			pModel->StreamMesh = 1;
			pModel->RenderMesh ( 2, RENDER_TEXTURE, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV );
			pModel->BodyLight[0] = 0.5f;
			pModel->BodyLight[1] = 0.0f;
			pModel->BodyLight[2] = 0.0f;
			pModel->RenderMesh ( 2, RENDER_TEXTURE|RENDER_BRIGHT|RENDER_CHROME, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV );
			pModel->StreamMesh = -1;

			pModel->EndRender();

			return true;
		}
		break;
#ifdef LDS_EXTENSIONMAP_MONSTERS_AIDA
	case MODEL_MONSTER01+194: //  블러디 데쓰 라이더
		{
			pModel->BeginRender(1.f);
			
			pModel->BodyLight[0] = 0.9f;
			pModel->BodyLight[1] = 0.9f;
			pModel->BodyLight[2] = 0.9f;
			
			pModel->RenderMesh ( 0, RENDER_TEXTURE, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV );
			pModel->RenderMesh ( 1, RENDER_TEXTURE, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV );
			pModel->RenderMesh ( 2, RENDER_TEXTURE, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV );
			pModel->StreamMesh = 0;
			pModel->RenderMesh ( 3, RENDER_TEXTURE, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, -(int)WorldTime%10000*0.0006f, -(int)WorldTime%10000*0.0006f );
			pModel->RenderMesh ( 3, RENDER_TEXTURE|RENDER_BRIGHT|RENDER_CHROME, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV );
			pModel->StreamMesh = -1;
			
			pModel->EndRender();
			
			return true;
		}
		break;
	case MODEL_MONSTER01+195: //  블러디 골렘
		{
			pModel->BeginRender(1.f);
			
			pModel->BodyLight[0] = 0.9f;
			pModel->BodyLight[1] = 0.9f;
			pModel->BodyLight[2] = 0.9f;
			
			pModel->StreamMesh = 0;
			pModel->RenderMesh ( 0, RENDER_TEXTURE, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, -(int)WorldTime%10000*0.0003f, -(int)WorldTime%10000*0.0003f );
			pModel->RenderMesh ( 0, RENDER_TEXTURE|RENDER_BRIGHT|RENDER_CHROME, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV );
			pModel->StreamMesh = -1;
			pModel->RenderMesh ( 1, RENDER_TEXTURE, pObject->Alpha, pObject->BlendMesh, pObject->BlendMeshLight, pObject->BlendMeshTexCoordU, pObject->BlendMeshTexCoordV );
			
			pModel->EndRender();
			
			return true;
		}
		break;
#endif // LDS_EXTENSIONMAP_MONSTERS_AIDA
	}
	return false;
}

bool M33Aida::AttackEffectAidaMonster(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel)
{
	if(!IsInAida())
		return false;

	//. 디폴트 처리로 넘기고 싶지않거든 true를 리턴하라.
	switch(pCharacter->MonsterIndex)
	{
		case 304:  //  위치 퀸
			{
				if(pCharacter->AttackTime == 10 && pObject->CurrentAction == MONSTER01_ATTACK2 )
				{
					vec3_t Light;
					Vector(1.f,1.f,1.f,Light);
					CreateEffect(BITMAP_JOINT_FORCE, pObject->Position, pObject->Angle, Light, 1);
				}
			}
			return true;
/*		case 305:  //  블루 골렘
			{
				if(pCharacter->AttackTime == 0 && pObject->CurrentAction == MONSTER01_ATTACK1 )
				{
					vec3_t Position, Light, Angle = {0.0f, 0.0f, 0.0f};
					Vector ( 5.0f, 5.0f, 5.0f, Light );		
					Vector ( 0.0f, 45.0f, 45.0f, Angle );
					vec3_t Relative = { 0.0f, 0.0f, 0.0f };
					BoneManager::GetBonePosition(pObject, "Monster101_L_Arm", Relative, Position);
					CreateParticle(BITMAP_SHOCK_WAVE, Position, Angle, Light, 3, 0.5f);
				}
			}
			return true;	
*/		case 308:  //  데스 트리
			{
				if(pCharacter->AttackTime == 10 && pObject->CurrentAction == MONSTER01_ATTACK2 )
				{
					vec3_t Light;
					Vector(1.f,1.f,1.f,Light);
					CreateEffect(MODEL_TREE_ATTACK, pObject->Position, pObject->Angle, Light);
				}
			}
			return true;
		case 309:  //  헬 마이네
			{
				for(int i = 0; i < 5; i++)
				{
					if(pCharacter->AttackTime == 10 && pObject->CurrentAction == MONSTER01_ATTACK1)
					{	
						vec3_t Light;
						Vector(1.f,1.f,1.f,Light);
						CreateEffect(MODEL_STORM,pObject->Position,pObject->Angle,Light, 3+i);
					}
				}
			}
			return true;
#ifdef LDS_EXTENSIONMAP_MONSTERS_AIDA
		case 552:  //  블러디 위치 퀸
			{
#ifndef LDS_MOD_BLOODWITCHQUEEN_EFFECT_ATTACK2		// #ifndef
				if(pCharacter->AttackTime == 10 && pObject->CurrentAction == MONSTER01_ATTACK2 )
				{
					vec3_t Light;
					Vector(1.f,1.f,1.f,Light);
					CreateEffect(BITMAP_JOINT_FORCE, pObject->Position, pObject->Angle, Light, 1);
				}
#endif // LDS_MOD_BLOODWITCHQUEEN_EFFECT_ATTACK2
			}
			return true;
#endif // LDS_EXTENSIONMAP_MONSTERS_AIDA
	}
	return false;
}


bool M33Aida::SetCurrentActionAidaMonster(CHARACTER* pCharacter, OBJECT* pObject)
{
	if(!IsInAida())
		return false;

	switch(pCharacter->MonsterIndex)
	{
	case 304:		//  위치 퀸
	case 305:		//  블루 골렘
	case 309:		//  헬 마이네
		return CheckMonsterSkill(pCharacter, pObject);
#ifdef LDS_EXTENSIONMAP_MONSTERS_AIDA
	case 549:	//  포트레스 오크
	case 550:	//  블러디 데스 라이더
		return CheckMonsterSkill(pCharacter, pObject);
	case 551:	//  블러디 골렘
		{
			if (pCharacter->MonsterSkill == ATMON_SKILL_EX_BLOODYGOLUEM_ATTACKSKILL)
			{
				SetAction(pObject, MONSTER01_ATTACK2);
				pCharacter->MonsterSkill = -1;	// 스킬 공격과 일반 공격이 있는 경우 스킬 공격 후 초기화 해야됨.
			}
			else
			{
				SetAction(pObject, MONSTER01_ATTACK1);
				pCharacter->MonsterSkill = -1;
			}
			return true;
		}
		return true;
	case 552:	//  블러디 위치 퀸
		{
			if (pCharacter->MonsterSkill == ATMON_SKILL_EX_BLOODYWITCHQUEEN_ATTACKSKILL)
			{
				SetAction(pObject, MONSTER01_ATTACK2);
				pCharacter->MonsterSkill = -1;	// 스킬 공격과 일반 공격이 있는 경우 스킬 공격 후 초기화 해야됨.
			}
			else
			{
				SetAction(pObject, MONSTER01_ATTACK1);
				pCharacter->MonsterSkill = -1;
			}
			return true;
		}
		return true;
#endif // LDS_EXTENSIONMAP_MONSTERS_AIDA
	}
	return false;
}

//. 화면처리
bool M33Aida::CreateMist(PARTICLE* pParticleObj)
{
	if(!IsInAida())
		return false;
	if(!IsInAidaSection2(Hero->Object.Position))
		return false;

	return false;
}
