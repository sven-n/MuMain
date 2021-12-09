// GMNewTown.cpp: implementation of the GMNewTown class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ZzzOpenglUtil.h"
#include "zzztexture.h"
#include "ZzzBMD.h"
#include "zzzlodterrain.h"
#include "ZzzScene.h"
#include "zzzEffect.h"
#include "zzzAi.h"
#include "zzzOpenData.h"
#include "ZzzCharacter.h"
#include "ZzzObject.h"
#include "DSPlaySound.h"
#include "BoneManager.h"
#include "GMNewTown.h"
#include "GOBoid.h"
#include "GIPetManager.h"
#include "MapManager.h"
#include "SkillManager.h"

#ifdef PJH_NEW_SERVER_SELECT_MAP
#include "w_BaseMap.h"
#include "w_MapHeaders.h"
#endif //PJH_NEW_SERVER_SELECT_MAP

using namespace SEASON3B;

bool GMNewTown::m_bCharacterSceneCheckMouse = false;

GMNewTown::GMNewTown()
{
	m_bCharacterSceneCheckMouse = false;
}

GMNewTown::~GMNewTown()
{
}

bool GMNewTown::IsCurrentMap()
{
	return (gMapManager.WorldActive == WD_51HOME_6TH_CHAR || 
#ifdef PJH_NEW_SERVER_SELECT_MAP
		gMapManager.WorldActive == WD_73NEW_LOGIN_SCENE || gMapManager.WorldActive == WD_74NEW_CHARACTER_SCENE
#else
		gMapManager.WorldActive == WD_77NEW_LOGIN_SCENE || World == WD_78NEW_CHARACTER_SCENE
#endif //PJH_NEW_SERVER_SELECT_MAP
		);
}
#ifdef PJH_NEW_SERVER_SELECT_MAP
bool GMNewTown::IsNewMap73_74()
{
	return (gMapManager.WorldActive == WD_73NEW_LOGIN_SCENE || gMapManager.WorldActive == WD_74NEW_CHARACTER_SCENE);
}
#endif //PJH_NEW_SERVER_SELECT_MAP

void GMNewTown::CreateObject(OBJECT* pObject)
{
	if(!IsCurrentMap())
		return;
#ifdef PJH_NEW_SERVER_SELECT_MAP
	if(IsNewMap73_74())
	{
		if(g_EmpireGuardian4.CreateObject(pObject))
		{
			switch(pObject->Type)
			{
			case 129:	case 79:	case 83:	
			case 82:	case 85:	case 86:
			case 130:	case 131:	case 158:
				pObject->HiddenMesh = -2;
				return;
			default:
				break;
			}
		}
		return;
	}
#endif //PJH_NEW_SERVER_SELECT_MAP

	switch(pObject->Type)
	{
	case 103:
		{
			CreateOperate(pObject);
		}
		break;
	}

	if (pObject->Type==15 || pObject->Type==25 || pObject->Type==27 || pObject->Type==45 || pObject->Type==46 || pObject->Type==47 || pObject->Type==48
		|| pObject->Type==53 || pObject->Type==98 || pObject->Type==107 || pObject->Type==115 || pObject->Type==122 || pObject->Type==130)
	{
		pObject->CollisionRange = -300;
	}
#ifndef PJH_NEW_SERVER_SELECT_MAP
	if (World == WD_77NEW_LOGIN_SCENE || World == WD_78NEW_CHARACTER_SCENE)
	{
		if (pObject->Type == 62)
		{
			pObject->SubType = 0;
		}
	}
#endif //PJH_NEW_SERVER_SELECT_MAP
}

bool GMNewTown::MoveObject(OBJECT* pObject)
{
	if(!IsCurrentMap())
		return false;

#ifdef PJH_NEW_SERVER_SELECT_MAP
	if(IsNewMap73_74())
		return g_EmpireGuardian4.MoveObject(pObject);
#endif //PJH_NEW_SERVER_SELECT_MAP

	if (gMapManager.WorldActive == WD_51HOME_6TH_CHAR)
		PlayObjectSound(pObject);

	float Luminosity;
	vec3_t Light;

	switch(pObject->Type)
	{
	case 0:
		Luminosity = (float)(rand() %4 + 3) * 0.1f;
		Vector(Luminosity, Luminosity * 0.6f, Luminosity * 0.2f, Light);
		AddTerrainLight(pObject->Position[0], pObject->Position[1], Light, 3, PrimaryTerrainLight);
		pObject->HiddenMesh = -2;
		break;
	case 2:
		{
			pObject->BlendMeshTexCoordV += 0.015f;
		}
		break;	
	case 53:
		{
			pObject->BlendMeshTexCoordV += 0.015f;
		}
		break;	
	case 54 :
		pObject->HiddenMesh = -2;
		break;
	case 55:
		{
			pObject->BlendMeshTexCoordV += 0.015f;
		}
		break;	
	case 56:
		pObject->BlendMesh = 0;
       	pObject->BlendMeshLight = sinf(WorldTime*0.003f)*0.3f+0.5f;
		pObject->Velocity = 0.05f;
		break;
	case 60 :
		pObject->HiddenMesh = -2;
		break;
	case 61:
		Luminosity = (float)(rand() %4 + 3) * 0.1f;
		Vector(Luminosity * 0.2f, Luminosity * 0.6f, Luminosity, Light);
		AddTerrainLight(pObject->Position[0], pObject->Position[1], Light, 3, PrimaryTerrainLight);
		pObject->HiddenMesh = -2;
		break;
	case 58:
		pObject->HiddenMesh = -2;
		break;
	case 59:
		pObject->HiddenMesh = -2;
		break;
	case 89:
		{
			pObject->BlendMeshTexCoordV += 0.005f;
		}
		break;
	case 62:
		pObject->HiddenMesh = -2;
		{
			int iEagleIndex = 1;
#ifndef PJH_NEW_SERVER_SELECT_MAP
			if(World == WD_77NEW_LOGIN_SCENE || World == WD_78NEW_CHARACTER_SCENE)
			{
				if (pObject->SubType == 0)
				{
					pObject->SubType = iEagleIndex;
					while(Boids[pObject->SubType].Live)
					{
						++pObject->SubType;
					};
					iEagleIndex = pObject->SubType;
				}
				else
				{
					break;
				}
			}
#endif //PJH_NEW_SERVER_SELECT_MAP
			OBJECT *pBoid = &Boids[iEagleIndex];
			if(!pBoid->Live)
			{
				pBoid->Live        = true;
				pBoid->Velocity    = 0.f;
				pBoid->LightEnable = true;
				pBoid->LifeTime    = 0;
				pBoid->SubType     = 1;
				Vector(0.5f,0.5f,0.5f,pBoid->Light);
   				pBoid->Alpha = 0.f;
 				pBoid->AlphaTarget = 1.f;
				pBoid->Gravity = 10.0f * pObject->Scale;
				pBoid->AlphaEnable = true;
				pBoid->Scale       = 0.5f;

				if (
#ifdef PJH_NEW_SERVER_SELECT_MAP
					gMapManager.WorldActive == WD_74NEW_CHARACTER_SCENE
#else //PJH_NEW_SERVER_SELECT_MAP
					gMapManager.WorldActive == WD_77NEW_LOGIN_SCENE					
					|| World == WD_78NEW_CHARACTER_SCENE
#endif //PJH_NEW_SERVER_SELECT_MAP
					)
					pBoid->ShadowScale = 0.f;
				else
				if (pObject->Position[2] > 100)
					pBoid->ShadowScale = 15.f;
				else
					pBoid->ShadowScale = 0.f;
				pBoid->HiddenMesh  = -1;
				pBoid->BlendMesh   = -1;
				pBoid->Timer       = (float)(rand()%314)*0.01f;
				pBoid->Type = MODEL_EAGLE;
				Vector(0.f,0.f,0.f,pBoid->Angle);
				VectorCopy(pObject->Position, pBoid->Position);
			}
		}
		break;
	}
	return true;
}

void GMNewTown::PlayObjectSound(OBJECT* pObject)
{
	float fDis_x, fDis_y;
	fDis_x = pObject->Position[0] - Hero->Object.Position[0];
	fDis_y = pObject->Position[1] - Hero->Object.Position[1];

	int Index = TERRAIN_INDEX_REPEAT(( Hero->PositionX),( Hero->PositionY));
	BOOL bSafeZone = FALSE;
	if((TerrainWall[Index]&TW_SAFEZONE)==TW_SAFEZONE)
		bSafeZone = TRUE;

	switch(pObject->Type)
	{
	case 2:
		if (!bSafeZone)
			PlayBuffer(SOUND_ELBELAND_WATERSMALL01, pObject, false);
		break;
	case 53:
		if (!bSafeZone)
			PlayBuffer(SOUND_ELBELAND_RAVINE01, pObject, false);
		break;
	case 56:
		PlayBuffer(SOUND_ELBELAND_ENTERATLANCE01, pObject, false);
		break;
	case 59:
		if (!bSafeZone)
			PlayBuffer(SOUND_ELBELAND_WATERFALLSMALL01, pObject, false);
		break;
	case 85:
		PlayBuffer(SOUND_ELBELAND_ENTERDEVIAS01, pObject, false);
		break;
	case 89:
		if (!bSafeZone)
			PlayBuffer(SOUND_ELBELAND_WATERWAY01, pObject, false);
		break;
	case 110:
		PlayBuffer(SOUND_ELBELAND_VILLAGEPROTECTION01, pObject, false);
		break;
	}
}

bool GMNewTown::RenderObjectVisual(OBJECT* pObject, BMD* pModel)
{
	if(!IsCurrentMap())
		return false;

#ifdef PJH_NEW_SERVER_SELECT_MAP
	if(IsNewMap73_74())
	{
		g_EmpireGuardian4.RenderObjectVisual(pObject, pModel);
		return true;
	}
#endif //PJH_NEW_SERVER_SELECT_MAP
	
	vec3_t p,Position, Light;

	switch(pObject->Type)
	{
	case 0:
		if (rand() % 3 == 0)
		{
			Vector(1.f, 1.f, 1.f, Light);
			CreateParticle(BITMAP_TRUE_FIRE, pObject->Position, pObject->Angle, Light, 0, pObject->Scale);
		}
		break;
    case 54:
        Vector ( 1.f, 1.f, 1.f, Light );
        if ( rand()%4==0 )
        {
            CreateParticle ( BITMAP_WATERFALL_2, pObject->Position, pObject->Angle, Light, 4, pObject->Scale );
        }
        break;
	case 58:
		CreateParticle ( BITMAP_WATERFALL_5, pObject->Position, pObject->Angle, Light, 0 );
		break;
	case 59:
        Vector ( 1.f, 1.f, 1.f, Light );
        CreateParticle ( BITMAP_WATERFALL_3, pObject->Position, pObject->Angle, Light, 8, pObject->Scale );
		break;
	case 60:
		if ( pObject->HiddenMesh!=-2 )
		{
            vec3_t  Light;
            Vector(0.06f,0.07f,0.08f,Light);
			for ( int i=0; i<10; ++i )
			{
#ifndef PJH_NEW_SERVER_SELECT_MAP
				if (World == WD_77NEW_LOGIN_SCENE)
					CreateParticle ( BITMAP_CLOUD, pObject->Position, pObject->Angle, Light, 3, pObject->Scale * 2, pObject );
				else
#endif //PJH_NEW_SERVER_SELECT_MAP
					CreateParticle ( BITMAP_CLOUD, pObject->Position, pObject->Angle, Light, 3, pObject->Scale, pObject );
			}
			pObject->HiddenMesh = -2;
		}
		break;
	case 61:
		if (rand() % 3 == 0)
		{
			Vector(1.f, 1.f, 1.f, Light);
			CreateParticle(BITMAP_TRUE_BLUE, pObject->Position, pObject->Angle, Light, 0, pObject->Scale);
		}
		break;
	case 63:
		{
			vec3_t vPos, vRelative;
 			float fLumi;
 			fLumi = (sinf(WorldTime*0.001f) + 1.0f) * 0.3f + 0.4f;
 			Vector(fLumi*1.0f, fLumi*1.0f, fLumi*0.6f, Light);

			Vector(-40.0f, -10.0f, 0.0f, vRelative);
			pModel->TransformPosition(BoneTransform[5], vRelative, vPos, false);
			CreateSprite(BITMAP_LIGHT, vPos, pObject->Scale*6.f, Light, pObject);
		}
		break;
	case 110:
			Vector(1.0f, 1.0f, 1.0f, Light);
			Vector(0.f, 0.f, 70.f, p);
			if (rand()%2 == 0)
			{
				p[0] = (float)cosf(WorldTime*0.03f)*(30.f+rand()%5);
				p[1] = (float)sinf(WorldTime*0.03f)*(30.f+rand()%5);
				pModel->TransformPosition(BoneTransform[0],p,Position,false);
				CreateParticle(BITMAP_LIGHT, Position, pObject->Angle, Light, 11, 0.6f, pObject);
			}
		break;
	case 121:
		{
			vec3_t vPos, vRelative;
			float fLumi, fScale;

			fLumi = (sinf(WorldTime*0.002f) + 1.0f) * 0.5f + 0.5f;
			Vector(fLumi*1.0f, fLumi*0.5f, fLumi*0.3f, Light);
			fScale = fLumi/1.0f;

			Vector(5.0f, -4.0f, -1.0f, vRelative);
			for (int i = 3; i <= 8; ++i)
			{
				pModel->TransformPosition(BoneTransform[i], vRelative, vPos, false);
				CreateSprite(BITMAP_LIGHT, vPos, fScale, Light, pObject);
			}
		}
		break;
	case 133:	case 134:	case 135:	case 136:	case 137:	case 138:	case 139:
	case 140:	case 141:	case 142:	case 143:	case 144:	case 145:	case 146:	case 147:
		if (pObject->HiddenMesh != -2)
		{
 			pObject->HiddenMesh = -2;

			int icntIndex=0;

			for(int i = 0; i < MAX_CHARACTERS_CLIENT; i++)
			{
				icntIndex = i;
				if (!CharactersClient[i].Object.Live) break;
			}

			CHARACTER * pCharacter = &CharactersClient[icntIndex];
			OBJECT * pNewObject = &CharactersClient[icntIndex].Object;
			CreateCharacterPointer(pCharacter,MODEL_PLAYER,0,0,0);
			Vector(0.3f,0.3f,0.3f,pNewObject->Light);
			pCharacter->Key = icntIndex;

			int Level = 0;
			switch(pObject->Type)
			{
			case 133:
				pCharacter->Class = CLASS_KNIGHT;
				pCharacter->Skin = 0;
				Level = 9;
				pCharacter->BodyPart[BODYPART_HELM  ].Type = MODEL_HELM  +9;
				pCharacter->BodyPart[BODYPART_HELM  ].Level = Level;
				pCharacter->BodyPart[BODYPART_ARMOR ].Type = MODEL_ARMOR +9;
				pCharacter->BodyPart[BODYPART_ARMOR ].Level = Level;
				pCharacter->BodyPart[BODYPART_PANTS ].Type = MODEL_PANTS +9;
				pCharacter->BodyPart[BODYPART_PANTS ].Level = Level;
				pCharacter->BodyPart[BODYPART_GLOVES].Type = MODEL_GLOVES+9;
				pCharacter->BodyPart[BODYPART_GLOVES].Level = Level;
				pCharacter->BodyPart[BODYPART_BOOTS ].Type = MODEL_BOOTS +9;
				pCharacter->BodyPart[BODYPART_BOOTS ].Level = Level;
				pCharacter->Weapon[0].Type = MODEL_SWORD+14;
				pCharacter->Weapon[0].Level = 9;
				pCharacter->Weapon[1].Type = MODEL_SHIELD+9;
				pCharacter->Weapon[1].Level = 9;
				pCharacter->Wing.Type      = -1;
				pCharacter->Helper.Type    = -1;
				break;
			case 134:
				pCharacter->Class = CLASS_KNIGHT;
				pCharacter->Skin = 0;
				Level = 10;
				pCharacter->BodyPart[BODYPART_HELM  ].Type = MODEL_HELM  +1;
				pCharacter->BodyPart[BODYPART_HELM  ].Level = Level;
				pCharacter->BodyPart[BODYPART_ARMOR ].Type = MODEL_ARMOR +1;
				pCharacter->BodyPart[BODYPART_ARMOR ].Level = Level;
				pCharacter->BodyPart[BODYPART_PANTS ].Type = MODEL_PANTS +1;
				pCharacter->BodyPart[BODYPART_PANTS ].Level = Level;
				pCharacter->BodyPart[BODYPART_GLOVES].Type = MODEL_GLOVES+1;
				pCharacter->BodyPart[BODYPART_GLOVES].Level = Level;
				pCharacter->BodyPart[BODYPART_BOOTS ].Type = MODEL_BOOTS +1;
				pCharacter->BodyPart[BODYPART_BOOTS ].Level = Level;
				pCharacter->Weapon[0].Type = MODEL_SWORD+16;
				pCharacter->Weapon[0].Level = 10;
				pCharacter->Weapon[1].Type = MODEL_SWORD+16;
				pCharacter->Weapon[1].Level = 10;
				pCharacter->Wing.Type      = MODEL_WING+5;
				pCharacter->Helper.Type    = -1;
				break;
// 			case 135:
			case 136:
				pCharacter->Class = CLASS_KNIGHT;
				pCharacter->Skin = 0;
				Level = 13;
				pCharacter->BodyPart[BODYPART_HELM  ].Type = MODEL_HELM  +29;
				pCharacter->BodyPart[BODYPART_HELM  ].Level = Level;
				pCharacter->BodyPart[BODYPART_ARMOR ].Type = MODEL_ARMOR +29;
				pCharacter->BodyPart[BODYPART_ARMOR ].Level = Level;
				pCharacter->BodyPart[BODYPART_PANTS ].Type = MODEL_PANTS +29;
				pCharacter->BodyPart[BODYPART_PANTS ].Level = Level;
				pCharacter->BodyPart[BODYPART_GLOVES].Type = MODEL_GLOVES+29;
				pCharacter->BodyPart[BODYPART_GLOVES].Level = Level;
				pCharacter->BodyPart[BODYPART_BOOTS ].Type = MODEL_BOOTS +29;
				pCharacter->BodyPart[BODYPART_BOOTS ].Level = Level;
				pCharacter->Weapon[0].Type = MODEL_SWORD+17;
				pCharacter->Weapon[0].Level = 13;
				pCharacter->Weapon[1].Type = -1;
				pCharacter->Weapon[1].Level = 0;
				pCharacter->Wing.Type      = MODEL_WING+5;
				pCharacter->Helper.Type    = MODEL_HELPER+37;
				CreateBug( MODEL_FENRIR_BLUE, pNewObject->Position,pNewObject);
				break;
// 			case 137:
			case 138:
				pCharacter->Class = CLASS_WIZARD;
				pCharacter->Skin = 0;
				Level = 10;
				pCharacter->BodyPart[BODYPART_HELM  ].Type = MODEL_HELM  +3;
				pCharacter->BodyPart[BODYPART_HELM  ].Level = Level;
				pCharacter->BodyPart[BODYPART_ARMOR ].Type = MODEL_ARMOR +3;
				pCharacter->BodyPart[BODYPART_ARMOR ].Level = Level;
				pCharacter->BodyPart[BODYPART_PANTS ].Type = MODEL_PANTS +3;
				pCharacter->BodyPart[BODYPART_PANTS ].Level = Level;
				pCharacter->BodyPart[BODYPART_GLOVES].Type = MODEL_GLOVES+3;
				pCharacter->BodyPart[BODYPART_GLOVES].Level = Level;
				pCharacter->BodyPart[BODYPART_BOOTS ].Type = MODEL_BOOTS +3;
				pCharacter->BodyPart[BODYPART_BOOTS ].Level = Level;
				pCharacter->Weapon[0].Type = MODEL_STAFF+5;
				pCharacter->Weapon[0].Level = 10;
				pCharacter->Weapon[1].Type = MODEL_SHIELD+14;
				pCharacter->Weapon[1].Level = 10;
				pCharacter->Wing.Type      = MODEL_WING+4;
				pCharacter->Helper.Type    = -1;
				break;
			case 139:
				pCharacter->Class = CLASS_WIZARD;
				pCharacter->Skin = 0;
				Level = 13;
				pCharacter->BodyPart[BODYPART_HELM  ].Type = MODEL_HELM  +30;
				pCharacter->BodyPart[BODYPART_HELM  ].Level = Level;
				pCharacter->BodyPart[BODYPART_ARMOR ].Type = MODEL_ARMOR +30;
				pCharacter->BodyPart[BODYPART_ARMOR ].Level = Level;
				pCharacter->BodyPart[BODYPART_PANTS ].Type = MODEL_PANTS +30;
				pCharacter->BodyPart[BODYPART_PANTS ].Level = Level;
				pCharacter->BodyPart[BODYPART_GLOVES].Type = MODEL_GLOVES+30;
				pCharacter->BodyPart[BODYPART_GLOVES].Level = Level;
				pCharacter->BodyPart[BODYPART_BOOTS ].Type = MODEL_BOOTS +30;
				pCharacter->BodyPart[BODYPART_BOOTS ].Level = Level;
				pCharacter->Weapon[0].Type = MODEL_STAFF+13;
				pCharacter->Weapon[0].Level = 13;
				pCharacter->Weapon[1].Type = -1;
				pCharacter->Weapon[1].Level = 0;
				pCharacter->Wing.Type      = MODEL_WING+37;
				pCharacter->Helper.Type    = -1;
				break;
// 			case 140:
// 			case 141:
			case 142:
				pCharacter->Class = CLASS_ELF;
				pCharacter->Skin = 0;
				Level = 13;
				pCharacter->BodyPart[BODYPART_HELM  ].Type = MODEL_HELM  +31;
				pCharacter->BodyPart[BODYPART_HELM  ].Level = Level;
				pCharacter->BodyPart[BODYPART_ARMOR ].Type = MODEL_ARMOR +31;
				pCharacter->BodyPart[BODYPART_ARMOR ].Level = Level;
				pCharacter->BodyPart[BODYPART_PANTS ].Type = MODEL_PANTS +31;
				pCharacter->BodyPart[BODYPART_PANTS ].Level = Level;
				pCharacter->BodyPart[BODYPART_GLOVES].Type = MODEL_GLOVES+31;
				pCharacter->BodyPart[BODYPART_GLOVES].Level = Level;
				pCharacter->BodyPart[BODYPART_BOOTS ].Type = MODEL_BOOTS +31;
				pCharacter->BodyPart[BODYPART_BOOTS ].Level = Level;
				pCharacter->Weapon[0].Type = -1;
				pCharacter->Weapon[0].Level = 0;
				pCharacter->Weapon[1].Type = MODEL_BOW+22;
				pCharacter->Weapon[1].Level = 13;
				pCharacter->Wing.Type      = MODEL_WING+38;
				pCharacter->Helper.Type    = -1;
				break;
// 			case 143:
			case 144:
				pCharacter->Class = CLASS_DARK;
				pCharacter->Skin = 0;
				Level = 13;
				pCharacter->BodyPart[BODYPART_HELM  ].Type = MODEL_HELM  +32;
				pCharacter->BodyPart[BODYPART_HELM  ].Level = Level;
				pCharacter->BodyPart[BODYPART_ARMOR ].Type = MODEL_ARMOR +32;
				pCharacter->BodyPart[BODYPART_ARMOR ].Level = Level;
				pCharacter->BodyPart[BODYPART_PANTS ].Type = MODEL_PANTS +32;
				pCharacter->BodyPart[BODYPART_PANTS ].Level = Level;
				pCharacter->BodyPart[BODYPART_GLOVES].Type = MODEL_GLOVES+32;
				pCharacter->BodyPart[BODYPART_GLOVES].Level = Level;
				pCharacter->BodyPart[BODYPART_BOOTS ].Type = MODEL_BOOTS +32;
				pCharacter->BodyPart[BODYPART_BOOTS ].Level = Level;
				pCharacter->Weapon[0].Type = MODEL_SWORD+23;
				pCharacter->Weapon[0].Level = 13;
				pCharacter->Weapon[1].Type = -1;
				pCharacter->Weapon[1].Level = 0;
				pCharacter->Wing.Type      = MODEL_WING+39;
				pCharacter->Helper.Type    = -1;
				break;
// 			case 145:
			case 146:
				pCharacter->Class = CLASS_DARK_LORD;
				pCharacter->Skin = 0;
				Level = 13;
				pCharacter->BodyPart[BODYPART_HELM  ].Type = MODEL_HELM  +28;
				pCharacter->BodyPart[BODYPART_HELM  ].Level = Level;
				pCharacter->BodyPart[BODYPART_ARMOR ].Type = MODEL_ARMOR +28;
				pCharacter->BodyPart[BODYPART_ARMOR ].Level = Level;
				pCharacter->BodyPart[BODYPART_PANTS ].Type = MODEL_PANTS +28;
				pCharacter->BodyPart[BODYPART_PANTS ].Level = Level;
				pCharacter->BodyPart[BODYPART_GLOVES].Type = MODEL_GLOVES+28;
				pCharacter->BodyPart[BODYPART_GLOVES].Level = Level;
				pCharacter->BodyPart[BODYPART_BOOTS ].Type = MODEL_BOOTS +28;
				pCharacter->BodyPart[BODYPART_BOOTS ].Level = Level;
				pCharacter->Weapon[0].Type = MODEL_MACE+15;
				pCharacter->Weapon[0].Level = 10;
				pCharacter->Weapon[1].Type = -1;
				pCharacter->Weapon[1].Level = 0;
				pCharacter->Wing.Type      = MODEL_HELPER+30;
				pCharacter->Helper.Type    = MODEL_HELPER+4;
				CreateBug( MODEL_DARK_HORSE, pNewObject->Position,pNewObject);
				CreatePetDarkSpirit_Now(pCharacter);
				break;
			case 147:
				pCharacter->Class = CLASS_DARK_LORD;
				pCharacter->Skin = 0;
				Level = 13;
				pCharacter->BodyPart[BODYPART_HELM  ].Type = MODEL_HELM  +33;
				pCharacter->BodyPart[BODYPART_HELM  ].Level = Level;
				pCharacter->BodyPart[BODYPART_ARMOR ].Type = MODEL_ARMOR +33;
				pCharacter->BodyPart[BODYPART_ARMOR ].Level = Level;
				pCharacter->BodyPart[BODYPART_PANTS ].Type = MODEL_PANTS +33;
				pCharacter->BodyPart[BODYPART_PANTS ].Level = Level;
				pCharacter->BodyPart[BODYPART_GLOVES].Type = MODEL_GLOVES+33;
				pCharacter->BodyPart[BODYPART_GLOVES].Level = Level;
				pCharacter->BodyPart[BODYPART_BOOTS ].Type = MODEL_BOOTS +33;
				pCharacter->BodyPart[BODYPART_BOOTS ].Level = Level;
				pCharacter->Weapon[0].Type = MODEL_MACE+14;
				pCharacter->Weapon[0].Level = 13;
				pCharacter->Weapon[1].Type = -1;
				pCharacter->Weapon[1].Level = 0;
				pCharacter->Wing.Type      = MODEL_WING+40;
				pCharacter->Helper.Type    = -1;
				break;
			}
			VectorCopy(pObject->Position, pNewObject->Position);
			VectorCopy(pObject->Angle, pNewObject->Angle);
			SetCharacterScale(pCharacter);

			SetPlayerAttack(pCharacter);
			pNewObject->PriorAnimationFrame = pNewObject->AnimationFrame = 
				rand() % Models[pNewObject->Type].Actions[pNewObject->CurrentAction].NumAnimationKeys;
			pObject->Owner = pNewObject;
			pObject->SubType = icntIndex;
		}
		break;
	case 149:		case 150:		case 151:		case 152:		case 153:		case 154:		case 155:
		if (pObject->HiddenMesh != -2)
		{
 			pObject->HiddenMesh = -2;

			int icntIndex = 0;

			for(int i = 0; i < MAX_CHARACTERS_CLIENT; i++)
			{
				icntIndex = i;
				if (!CharactersClient[i].Object.Live)
				{
					break;
				}
			}

			OBJECT * pNewObject = &CharactersClient[icntIndex].Object;

			switch (pObject->Type)
			{
					//case 148: CreateMonster(418, 0, 0, icntIndex); break;
				case 149: CreateMonster(419, 0, 0, icntIndex); break;
				case 150: CreateMonster(420, 0, 0, icntIndex); break;
				case 151: CreateMonster(421, 0, 0, icntIndex); break;
					//case 152: CreateMonster(422, 0, 0, icntIndex); break;
				case 153: CreateMonster(423, 0, 0, icntIndex); break;
					//case 154: CreateMonster(424, 0, 0, icntIndex); break;
				case 155: CreateMonster(425, 0, 0, icntIndex); break;
			}

			VectorCopy(pObject->Position, pNewObject->Position);
			VectorCopy(pObject->Angle, pNewObject->Angle);
		  	SetAction(pNewObject,MONSTER01_ATTACK1);
			if (Models[pNewObject->Type].Actions[MONSTER01_ATTACK1].NumAnimationKeys > 0)
			{
				pNewObject->PriorAnimationFrame = pNewObject->AnimationFrame =
					rand() % Models[pNewObject->Type].Actions[MONSTER01_ATTACK1].NumAnimationKeys;
			}
			else
			{
				assert(!"°ø°Ýµ¿ÀÛÀÌ ¾ø´?");
			}
			pObject->Owner = pNewObject;
		}
		break;
	}

	if (pObject->Type >= 133 && pObject->Type <= 147)
	{
		if (pObject->Owner != NULL)
		{
			CHARACTER * pCharacter = &CharactersClient[pObject->SubType];
			if (pObject->Owner->CurrentAction < PLAYER_WALK_MALE)
			{
				if (pObject->Type == 133)
				{
					if (rand()%2==0)
					{
						pCharacter->Helper.Type = -1;
						SetPlayerAttack(pCharacter);
					}
					else
					{
						pCharacter->Helper.Type = -1;
						SetAction(pObject->Owner,PLAYER_ATTACK_SKILL_WHEEL);
						pCharacter->Skill = AT_SKILL_WHEEL;
						pCharacter->AttackTime = 1;
						pCharacter->TargetCharacter = -1;
						pCharacter->AttackFlag = ATTACK_FAIL;
						pCharacter->SkillX = pCharacter->PositionX;
						pCharacter->SkillY = pCharacter->PositionY;
					}
				}
				else if (pObject->Type == 134)
				{
					if (rand()%2==0)
					{
						SetAction(pObject->Owner,PLAYER_ATTACK_SKILL_WHEEL);
						pCharacter->Skill = AT_SKILL_WHEEL;
						pCharacter->AttackTime = 1;
						pCharacter->TargetCharacter = -1;
						pCharacter->AttackFlag = ATTACK_FAIL;
						pCharacter->SkillX = pCharacter->PositionX;
						pCharacter->SkillY = pCharacter->PositionY;
					}
					else
					{
						SetAction(pObject->Owner,PLAYER_ATTACK_ONETOONE);
						pCharacter->Skill = AT_SKILL_ONETOONE;
						pCharacter->AttackTime = 1;
						pCharacter->TargetCharacter = -1;
						pCharacter->AttackFlag = ATTACK_FAIL;
						pCharacter->SkillX = pCharacter->PositionX;
						pCharacter->SkillY = pCharacter->PositionY;
					}
				}
				else if (pObject->Type == 138)
				{
					if (rand()%2==0)
					{
						SetPlayerAttack(pCharacter);
					}
					else
					{
						SetPlayerMagic(pCharacter);
						pCharacter->Skill = AT_SKILL_INFERNO;
						pCharacter->AttackTime = 1;
						pCharacter->TargetCharacter = -1;
						pCharacter->AttackFlag = ATTACK_FAIL;
						pCharacter->SkillX = pCharacter->PositionX;
						pCharacter->SkillY = pCharacter->PositionY;
					}
				}
				else if (pObject->Type == 139)
				{
					SetPlayerStop(pCharacter);
				}
				else if (pObject->Type == 142)
				{
					SetPlayerStop(pCharacter);
				}
				else if (pObject->Type == 144)
				{
					SetPlayerStop(pCharacter);
				}
				else if (pObject->Type == 147)
				{
					SetPlayerStop(pCharacter);
				}
			}
		}
	}
	else if (pObject->Type >= 149 && pObject->Type <= 155)
	{
		if (pObject->Owner != NULL)
		{
			if (pObject->Owner->CurrentAction < MONSTER01_WALK)
			{
				if (pObject->Type == 153)
				{
					int iRand = rand()%3;
					if (iRand == 0) SetAction(pObject->Owner,MONSTER01_ATTACK1);
					else if (iRand == 1)  SetAction(pObject->Owner,MONSTER01_ATTACK2);
					else SetAction(pObject->Owner,MONSTER01_SHOCK);
				}
				else if ((pObject->Type >= 152 && pObject->Type <= 155))
				{
					if (rand()%2==0) SetAction(pObject->Owner,MONSTER01_ATTACK1);
					else SetAction(pObject->Owner,MONSTER01_SHOCK);
				}
				else if ((pObject->Type >= 149 && pObject->Type <= 151))
				{
					SetAction(pObject->Owner,MONSTER01_STOP1);
				}
			}
		}
	}
	
	return true;
}

extern float  IntensityTransform[MAX_MESH][MAX_VERTICES];

bool GMNewTown::RenderObject(OBJECT* pObject, BMD* pModel,bool ExtraMon)
{
	if(!IsCurrentMap())
		return false;

#ifdef PJH_NEW_SERVER_SELECT_MAP
	if(IsNewMap73_74())
		return g_EmpireGuardian4.RenderObjectMesh(pObject, pModel, ExtraMon);
#endif //PJH_NEW_SERVER_SELECT_MAP

	// ¹è°æ
	if ((pObject->Type>=5 && pObject->Type<=14) || pObject->Type == 4 || pObject->Type == 129)
	{
		Mesh_t *m = NULL;
		for(int i = 0; i < pModel->NumMeshs; i++)
		{
			m = &pModel->Meshs[i];
			for(int j = 0; j < m->NumNormals; j++)
			{
				IntensityTransform[i][j] = 2.0f;
			}
		}
		Vector(1.0f, 1.0f, 1.0f, pObject->Light);
	}
#ifndef PJH_NEW_SERVER_SELECT_MAP
	else
	if (World == WD_77NEW_LOGIN_SCENE)
	{
		Mesh_t *m = NULL;
		for(int i = 0; i < pModel->NumMeshs; i++)
		{
			m = &pModel->Meshs[i];
			for(int j = 0; j < m->NumNormals; j++)
			{
				IntensityTransform[i][j] *= 3.0f;
			}
		}
		Vector(1.0f, 1.0f, 1.0f, pObject->Light);
	}
#endif //PJH_NEW_SERVER_SELECT_MAP
	if(pObject->Type == 53 || pObject->Type == 55
		|| pObject->Type == 110 || pObject->Type == 89
		|| pObject->Type == 78 || pObject->Type == 79
		|| pObject->Type == 125 || pObject->Type == 128
		|| pObject->Type == 2
		)
	{
		pObject->m_bRenderAfterCharacter = true;
	}
	else if (pObject->Type == 73)
	{
		pModel->RenderBody(RENDER_TEXTURE,pObject->Alpha,pObject->BlendMesh,pObject->BlendMeshLight,pObject->BlendMeshTexCoordU,pObject->BlendMeshTexCoordV,pObject->HiddenMesh);
		pModel->RenderBody(RENDER_BRIGHT|RENDER_CHROME,0.5f,pObject->BlendMesh,pObject->BlendMeshLight,pObject->BlendMeshTexCoordU,pObject->BlendMeshTexCoordV,pObject->HiddenMesh);
	}
	else if (pObject->Type == 104)
	{
		pModel->RenderBody(RENDER_TEXTURE,pObject->Alpha,pObject->BlendMesh,pObject->BlendMeshLight,pObject->BlendMeshTexCoordU,pObject->BlendMeshTexCoordV,pObject->HiddenMesh);
		float fLumi = (sinf(WorldTime*0.002f) + 1.f) * 0.5f;
		pModel->RenderMesh(4,RENDER_BRIGHT,pObject->Alpha,4,fLumi);
	}
	else if (pObject->Type == 113)
	{
		pModel->RenderBody(RENDER_TEXTURE,pObject->Alpha,pObject->BlendMesh,pObject->BlendMeshLight,pObject->BlendMeshTexCoordU,pObject->BlendMeshTexCoordV,pObject->HiddenMesh);
		pModel->RenderMesh(9,RENDER_BRIGHT|RENDER_CHROME,pObject->Alpha,pObject->BlendMesh,pObject->BlendMeshLight);
	}
	else if (pObject->Type == 114)
	{
		pModel->RenderBody(RENDER_TEXTURE,pObject->Alpha,pObject->BlendMesh,pObject->BlendMeshLight,pObject->BlendMeshTexCoordU,pObject->BlendMeshTexCoordV,pObject->HiddenMesh);
		pModel->RenderMesh(9,RENDER_BRIGHT|RENDER_CHROME,pObject->Alpha,pObject->BlendMesh,pObject->BlendMeshLight);
	}
	else if (pObject->Type == 121)
	{
		pModel->RenderMesh(2,RENDER_TEXTURE,pObject->Alpha,pObject->BlendMesh,pObject->BlendMeshLight);
		pModel->RenderMesh(3,RENDER_TEXTURE,pObject->Alpha,pObject->BlendMesh,pObject->BlendMeshLight);
 		pModel->RenderMesh(1,RENDER_TEXTURE,pObject->Alpha,pObject->BlendMesh,pObject->BlendMeshLight);
		float fLumi = (sinf(WorldTime*0.002f) + 1.f) * 0.5f;
 		pModel->RenderMesh(1,RENDER_BRIGHT,pObject->Alpha,1,fLumi);
 		fLumi = (sinf(WorldTime*0.0015f) + 1.0f) * 0.1f + 0.1f;
   		pModel->RenderMesh(0,RENDER_BRIGHT|RENDER_CHROME,fLumi,0,fLumi);
	}
	// NPC
	else if(pObject->Type == MODEL_ELBELAND_MARCE)
	{
 		pModel->RenderMesh(0,RENDER_TEXTURE,pObject->Alpha,pObject->BlendMesh,pObject->BlendMeshLight);
 		pModel->RenderMesh(1,RENDER_TEXTURE,pObject->Alpha,pObject->BlendMesh,pObject->BlendMeshLight);
 		pModel->RenderMesh(2,RENDER_TEXTURE|RENDER_BRIGHT,pObject->Alpha,pObject->BlendMesh,pObject->BlendMeshLight);
	}
	else if (pObject->Type == MODEL_MONSTER01+130)
	{
 		pModel->RenderMesh(1,RENDER_TEXTURE,pObject->Alpha,pObject->BlendMesh,pObject->BlendMeshLight);
 		pModel->RenderMesh(0,RENDER_TEXTURE,pObject->Alpha,pObject->BlendMesh,pObject->BlendMeshLight);
	}
	else if (pObject->Type == MODEL_MONSTER01+133 && pObject->CurrentAction == MONSTER01_DIE)
	{

	}
#ifndef PJH_NEW_SERVER_SELECT_MAP
	else if (World == WD_77NEW_LOGIN_SCENE &&
		((pObject->Type >= 133 && pObject->Type <= 164) || pObject->Type == 0 || pObject->Type == 54
		|| pObject->Type == 58 || pObject->Type == 59 || pObject->Type == 60 || pObject->Type == 61
		|| pObject->Type == 62))
	{

	}
	else if (World == WD_78NEW_CHARACTER_SCENE && pObject->Type == 4)
	{
		if(pObject->CurrentAction == 0 && pObject->AnimationFrame >= 15.5)
		{
			if(CharacterSceneCheckMouse(pObject) == false)
			{
				SetAction(pObject, rand()%2+1);
			}
		}
		else if(pObject->CurrentAction == 1 && pObject->AnimationFrame >= 15.5)
		{
			if(CharacterSceneCheckMouse(pObject) == false)
			{
				int iRand = rand()%10;
				if(iRand >= 0 && iRand <= 1)
				{
					iRand = 0;
				}
				else if(iRand >= 3 && iRand <= 6)
				{
					iRand = 1;
				}
				else
				{
					iRand = 2;
				}
				SetAction(pObject, iRand);
			}
		}
		else if(pObject->CurrentAction == 2 && pObject->AnimationFrame >= 14.2)
		{
			if(CharacterSceneCheckMouse(pObject) == false)
			{
				int iRand = rand()%10;
				if(iRand >= 0 && iRand <= 1)
				{
					iRand = 0;
				}
				else if(iRand >= 3 && iRand <= 6)
				{
					iRand = 1;
				}
				else
				{
					iRand = 2;
				}
				SetAction(pObject, iRand);
			}
		}
		
		pModel->RenderBody(RENDER_TEXTURE,pObject->Alpha,pObject->BlendMesh,pObject->BlendMeshLight,pObject->BlendMeshTexCoordU,pObject->BlendMeshTexCoordV);
	}
#endif //PJH_NEW_SERVER_SELECT_MAP
	else
		return false;
	
	return true;
}

void GMNewTown::RenderObjectAfterCharacter(OBJECT* pObject, BMD* pModel,bool ExtraMon0)
{
	if(!IsCurrentMap())
		return;

#ifdef PJH_NEW_SERVER_SELECT_MAP
	if(IsNewMap73_74())
	{
		g_EmpireGuardian4.RenderAfterObjectMesh(pObject, pModel, ExtraMon0);
		return;
	}
#endif //PJH_NEW_SERVER_SELECT_MAP

	if(pObject->Type == 2 || pObject->Type == 53 || pObject->Type == 55 || pObject->Type == 89 || pObject->Type == 125 || pObject->Type == 128)	// ÆøÆ÷¹°1,2, ¼ö·Î, È¸¿À¸®, ºû
	{
		pModel->RenderBody(RENDER_TEXTURE,pObject->Alpha,pObject->BlendMesh,pObject->BlendMeshLight,pObject->BlendMeshTexCoordU,pObject->BlendMeshTexCoordV,pObject->HiddenMesh);
	}
	else if(pObject->Type == 110)
	{
		pModel->RenderMesh(2,RENDER_TEXTURE|RENDER_BRIGHT|RENDER_CHROME2,pObject->Alpha,pObject->BlendMesh,pObject->BlendMeshLight);
		pModel->RenderMesh(1,RENDER_TEXTURE|RENDER_BRIGHT,pObject->Alpha,pObject->BlendMesh,pObject->BlendMeshLight);
		pModel->RenderMesh(0,RENDER_TEXTURE|RENDER_BRIGHT,pObject->Alpha,pObject->BlendMesh,pObject->BlendMeshLight);
	}
	else if (pObject->Type == 78)
	{
		pModel->RenderMesh(0,RENDER_TEXTURE,pObject->Alpha,pObject->BlendMesh,pObject->BlendMeshLight,pObject->BlendMeshTexCoordU,pObject->BlendMeshTexCoordV);
		pModel->RenderMesh(0,RENDER_TEXTURE|RENDER_BRIGHT,pObject->Alpha,pObject->BlendMesh,pObject->BlendMeshLight,pObject->BlendMeshTexCoordU,pObject->BlendMeshTexCoordV);
		vec3_t p, Position, Light;
		for (int i = 1; i <= 4; ++i)
		{
			Vector(0.0f,  0.0f, 0.0f, p);
			pModel->TransformPosition(BoneTransform[i],p,Position,false);
			Vector(0.1f,0.1f,0.3f,Light);
			CreateSprite(BITMAP_SPARK+1, Position, 5.5f, Light, pObject);
		}
	}
	else if (pObject->Type == 79)
	{
		pModel->RenderMesh(0,RENDER_TEXTURE,pObject->Alpha,pObject->BlendMesh,pObject->BlendMeshLight,pObject->BlendMeshTexCoordU,pObject->BlendMeshTexCoordV);
		pModel->RenderMesh(0,RENDER_TEXTURE|RENDER_BRIGHT,pObject->Alpha,pObject->BlendMesh,pObject->BlendMeshLight,pObject->BlendMeshTexCoordU,pObject->BlendMeshTexCoordV);

		vec3_t p, Position, Light;
		for (int i = 1; i <= 6; ++i)
		{
			Vector(0.0f,  0.0f, 0.0f, p);
			pModel->TransformPosition(BoneTransform[i],p,Position,false);
			Vector(0.1f,0.1f,0.3f,Light);
			CreateSprite(BITMAP_SPARK+1, Position, 5.5f, Light, pObject);
		}
	}
}

CHARACTER* GMNewTown::CreateNewTownMonster(int iType, int PosX, int PosY, int Key)
{
	CHARACTER* pCharacter = NULL;

	switch (iType)
	{
	case 415:
   		OpenNpc(MODEL_ELBELAND_SILVIA);
		pCharacter = CreateCharacter(Key,MODEL_ELBELAND_SILVIA,PosX,PosY);
		strcpy(pCharacter->ID,"½Çºñ¾Æ");
		pCharacter->Object.Scale = 1.0f;
		pCharacter->Object.m_fEdgeScale = 1.2f;
		break;
	case 416:
   		OpenNpc(MODEL_ELBELAND_RHEA);
		pCharacter = CreateCharacter(Key,MODEL_ELBELAND_RHEA,PosX,PosY);
		strcpy(pCharacter->ID,"·¹¾Æ");
		pCharacter->Object.Scale = 1.0f;
		pCharacter->Object.m_fEdgeScale = 1.1f;
		Models[MODEL_ELBELAND_RHEA].Actions[0].PlaySpeed = 0.2f;
		Models[MODEL_ELBELAND_RHEA].Actions[1].PlaySpeed = 0.4f;
		break;
	case 417:
   		OpenNpc(MODEL_ELBELAND_MARCE);
		pCharacter = CreateCharacter(Key,MODEL_ELBELAND_MARCE,PosX,PosY);
		strcpy(pCharacter->ID,"¸¶¸£¼¼");
		pCharacter->Object.Scale = 1.05f;
		pCharacter->Object.m_fEdgeScale = 1.2f;
		break;
	case 418:
		OpenMonsterModel(128);
		pCharacter = CreateCharacter(Key,MODEL_MONSTER01+128,PosX,PosY);
		strcpy(pCharacter->ID,"±â±«ÇÑ Åä³¢");
		pCharacter->Object.Scale = 1.0f * 0.95f;
		pCharacter->Weapon[0].Type = -1;
		pCharacter->Weapon[1].Type = -1;
		break;
	case 419:
		OpenMonsterModel(129);
		pCharacter = CreateCharacter(Key,MODEL_MONSTER01+129,PosX,PosY);
		strcpy(pCharacter->ID,"ÈäÃøÇÑ Åä³¢");
		pCharacter->Object.Scale = 0.8f * 0.95f;
		pCharacter->Weapon[0].Type = -1;
		pCharacter->Weapon[1].Type = -1;
		break;
	case 420:
		OpenMonsterModel(130);
		pCharacter = CreateCharacter(Key,MODEL_MONSTER01+130,PosX,PosY);
		strcpy(pCharacter->ID,"´Á´ëÀÎ°£");
		pCharacter->Object.Scale = 1.0f * 0.95f;
		pCharacter->Weapon[0].Type = -1;
		pCharacter->Weapon[1].Type = -1;
		break;
	case 421:
		OpenMonsterModel(131);
		pCharacter = CreateCharacter(Key,MODEL_MONSTER01+131,PosX,PosY);
		strcpy(pCharacter->ID,"¿À¿°µÈ ³ªºñ");
		pCharacter->Object.Scale = 0.8f * 1.1f;
		pCharacter->Weapon[0].Type = -1;
		pCharacter->Weapon[1].Type = -1;
		break;
	case 422:
		OpenMonsterModel(132);
		pCharacter = CreateCharacter(Key,MODEL_MONSTER01+132,PosX,PosY);
		strcpy(pCharacter->ID,"ÀúÁÖ¹ÞÀº¸®Ä¡");
		pCharacter->Object.Scale = 1.0f * 0.95f;
		pCharacter->Weapon[0].Type = -1;
		pCharacter->Weapon[1].Type = -1;
		break;
	case 423:
		OpenMonsterModel(133);
		pCharacter = CreateCharacter(Key,MODEL_MONSTER01+133,PosX,PosY);
		strcpy(pCharacter->ID,"ÅäÅÛ°ñ·½");
		pCharacter->Object.Scale = 0.17f * 0.95f;
		pCharacter->Object.ShadowScale = 0.01f;
		pCharacter->Object.m_fEdgeScale = 1.05f;
		pCharacter->Object.LifeTime = 100;
		pCharacter->Weapon[0].Type = -1;
		pCharacter->Weapon[1].Type = -1;
		break;
	case 424:
		OpenMonsterModel(134);
		pCharacter = CreateCharacter(Key,MODEL_MONSTER01+134,PosX,PosY);
		strcpy(pCharacter->ID,"±«¼ö ¿ì¾¾");
		pCharacter->Object.Scale = 1.2f * 0.95f;
		pCharacter->Weapon[0].Type = -1;
		pCharacter->Weapon[1].Type = -1;
		break;
	case 425:
		OpenMonsterModel(135);
		pCharacter = CreateCharacter(Key,MODEL_MONSTER01+135,PosX,PosY);
		strcpy(pCharacter->ID,"±«¼ö ¿ì¾¾ ´ëÀå");
		pCharacter->Object.Scale = 1.3f * 0.95f;
		pCharacter->Weapon[0].Type = -1;
		pCharacter->Weapon[1].Type = -1;
		break;
	}

	return pCharacter;
}

bool GMNewTown::MoveMonsterVisual(OBJECT* pObject, BMD* pModel)
{
	if(!IsCurrentMap())
		return false;

	return false;
}

void GMNewTown::MoveBlurEffect(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel)
{
	switch(pObject->Type)
	{
	case MODEL_MONSTER01+130:
		{
			float Start_Frame = 0.f;
			float End_Frame = 6.0f;
			if((pObject->AnimationFrame >= Start_Frame && pObject->AnimationFrame <= End_Frame && pObject->CurrentAction == MONSTER01_ATTACK1) 
				|| (pObject->AnimationFrame >= Start_Frame && pObject->AnimationFrame <= End_Frame && pObject->CurrentAction == MONSTER01_ATTACK2))
			{
				vec3_t  Light;
				Vector(1.0f, 1.2f, 2.f, Light);

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

					pModel->TransformPosition(BoneTransform[60], StartRelative, StartPos, false);
					pModel->TransformPosition(BoneTransform[61], EndRelative, EndPos, false);
					CreateBlur(pCharacter, StartPos, EndPos, Light, 0);
					
					fAnimationFrame += fSpeedPerFrame;
				}
			}
		}
		break;
	}
}

bool GMNewTown::RenderMonsterVisual(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel)
{
	if(!IsCurrentMap())
		return false;

	vec3_t vPos, vRelative, Light;
	float fLumi, fScale;

	switch(pObject->Type)
	{
	case MODEL_ELBELAND_RHEA:
		if (pObject->CurrentAction == 0 && rand()%5==0)
		{
			Vector((rand()%90+10)*0.01f, (rand()%90+10)*0.01f, (rand()%90+10)*0.01f, Light);
			fScale = (rand()%5+5)*0.1f;
			Vector(0.f, 0.f, 0.f, vRelative);
			vRelative[0] = 20+(rand()%1000-500)*0.1f;
			vRelative[1] = (rand()%300-150)*0.1f;
			pModel->TransformPosition(pObject->BoneTransform[35],vRelative,vPos,true);
			CreateParticle(BITMAP_LIGHT, vPos, pObject->Angle, Light, 13, fScale, pObject);
			CreateParticle(BITMAP_LIGHT, vPos, pObject->Angle, Light, 12, fScale, pObject);
		}
		break;
	case MODEL_ELBELAND_MARCE:
		{
			Vector(10.0f, 0.0f, 0.0f, vRelative);
			pModel->TransformPosition(pObject->BoneTransform[81], vRelative, vPos, true);
			fScale = 1.5f;

			fLumi = (sinf(WorldTime*0.001f) + 1.0f) * 0.3f + 0.4f;
			Vector(fLumi*0.5f, fLumi*0.5f, fLumi*0.5f, Light);
			CreateSprite(BITMAP_FLARE_BLUE, vPos, fScale, Light, pObject);

			Vector(0.5f, 0.5f, 0.5f, Light);
			CreateSprite(BITMAP_FLARE_BLUE, vPos, fScale*0.8f, Light, pObject, -WorldTime*0.1f);
		}
		break;
	case MODEL_MONSTER01+132:
		for (int i = 0; i < 4; ++i)
		{
			Vector(0, (rand()%300-150)*0.1f, (rand()%200-100)*0.1f, vRelative);
			pModel->TransformPosition(pObject->BoneTransform[30],vRelative,vPos,true);
			CreateParticle(BITMAP_FIRE_CURSEDLICH, vPos, pObject->Angle, pObject->Light, 0, 1, pObject);
		}
		break;
	case MODEL_MONSTER01+133:
		if (pObject->CurrentAction == MONSTER01_WALK)
		{
			vec3_t Position;
			Vector(pObject->Position[0]+rand()%200-100,pObject->Position[1]+rand()%200-100,pObject->Position[2],Position);
			CreateParticle(BITMAP_SMOKE+1,Position,pObject->Angle,pObject->Light);
		}
 		if (pObject->CurrentAction == MONSTER01_DIE)
		{
			if (pObject->LifeTime == 100)
			{
				pObject->LifeTime = 90;
				pObject->m_bRenderShadow = false;

				vec3_t vRelativePos, vWorldPos, Light;
				Vector(1.0f,1.0f,1.0f,Light);

				Vector(0.f, 0.f, 0.f, vRelativePos);
				
				pModel->TransformPosition(pObject->BoneTransform[7], vRelativePos, vWorldPos, true);
				CreateEffect(MODEL_TOTEMGOLEM_PART1,vWorldPos,pObject->Angle,Light,0,pObject,0,0);
				
				pModel->TransformPosition(pObject->BoneTransform[5], vRelativePos, vWorldPos, true);
				CreateEffect(MODEL_TOTEMGOLEM_PART2,vWorldPos,pObject->Angle,Light,0,pObject,0,0);
				
				pModel->TransformPosition(pObject->BoneTransform[29], vRelativePos, vWorldPos, true);
				CreateEffect(MODEL_TOTEMGOLEM_PART3,vWorldPos,pObject->Angle,Light,0,pObject,0,0);
				
				pModel->TransformPosition(pObject->BoneTransform[64], vRelativePos, vWorldPos, true);
				CreateEffect(MODEL_TOTEMGOLEM_PART4,vWorldPos,pObject->Angle,Light,0,pObject,0,0);

				pModel->TransformPosition(pObject->BoneTransform[93], vRelativePos, vWorldPos, true);
				CreateEffect(MODEL_TOTEMGOLEM_PART5,vWorldPos,pObject->Angle,Light,0,pObject,0,0);

				pModel->TransformPosition(pObject->BoneTransform[98], vRelativePos, vWorldPos, true);
				CreateEffect(MODEL_TOTEMGOLEM_PART6,vWorldPos,pObject->Angle,Light,0,pObject,0,0);

				pModel->TransformPosition(pObject->BoneTransform[5], vRelativePos, vWorldPos, true);
				VectorCopy(vWorldPos, vRelativePos);

				for (int i = 0; i < 6; ++i)
				{
					vWorldPos[0] = vRelativePos[0] + rand()%160-80;
					vWorldPos[1] = vRelativePos[1] + rand()%160-80;
					vWorldPos[2] = vRelativePos[2];
					CreateParticle(BITMAP_LEAF_TOTEMGOLEM,vWorldPos,pObject->Angle,Light,0,1.0f);
				}

				Vector(0.5f,0.5f,0.5f,Light);

				for ( int i = 0; i < 20; ++i)
				{
					vWorldPos[0] = vRelativePos[0] + rand()%160-80;
					vWorldPos[1] = vRelativePos[1] + rand()%160-80;
					vWorldPos[2] = vRelativePos[2] + (rand()%150)-50;
					CreateParticle(BITMAP_SMOKE,vWorldPos,pObject->Angle,Light,48,1.0f);
				}
			}
		}
		break;
	}

	return false;
}

bool GMNewTown::PlayMonsterSound(OBJECT* pObject)
{
	if(!IsCurrentMap())
		return false;

	float fDis_x, fDis_y;
	fDis_x = pObject->Position[0] - Hero->Object.Position[0];
	fDis_y = pObject->Position[1] - Hero->Object.Position[1];
	float fDistance = sqrtf(fDis_x*fDis_x+fDis_y*fDis_y);

	if (fDistance > 500.0f) return true;

	switch(pObject->Type)
	{
	case MODEL_MONSTER01+128:
		if(pObject->CurrentAction == MONSTER01_ATTACK1 || pObject->CurrentAction == MONSTER01_ATTACK2)
		{
			PlayBuffer(SOUND_ELBELAND_RABBITSTRANGE_ATTACK01);
		}
		else if(pObject->CurrentAction == MONSTER01_DIE)
		{
			PlayBuffer(SOUND_ELBELAND_RABBITSTRANGE_DEATH01);
		}
		return true;
	case MODEL_MONSTER01+129:
		if (pObject->CurrentAction == MONSTER01_WALK)
		{
			if (rand() % 100 == 0)
			{
				PlayBuffer(SOUND_ELBELAND_RABBITUGLY_BREATH01);
			}
		}
		else if(pObject->CurrentAction == MONSTER01_ATTACK1 || pObject->CurrentAction == MONSTER01_ATTACK2)
		{
			PlayBuffer(SOUND_ELBELAND_RABBITUGLY_ATTACK01);
		}
		else if(pObject->CurrentAction == MONSTER01_DIE)
		{
			PlayBuffer(SOUND_ELBELAND_RABBITUGLY_DEATH01);
		}
		return true;
	case MODEL_MONSTER01+130:
		if (pObject->CurrentAction == MONSTER01_STOP1 || pObject->CurrentAction == MONSTER01_STOP2
			|| pObject->CurrentAction == MONSTER01_WALK)
		{
			if (rand() % 30 == 0)
			{
				PlayBuffer(SOUND_ELBELAND_WOLFHUMAN_MOVE02);
			}
		}
		else if(pObject->CurrentAction == MONSTER01_ATTACK1 || pObject->CurrentAction == MONSTER01_ATTACK2)
		{
			PlayBuffer(SOUND_ELBELAND_WOLFHUMAN_ATTACK01);
		}
		else if(pObject->CurrentAction == MONSTER01_DIE)
		{
			PlayBuffer(SOUND_ELBELAND_WOLFHUMAN_DEATH01);
		}
		return true;
	case MODEL_MONSTER01+131:
		if (pObject->CurrentAction == MONSTER01_WALK)
		{
			if (rand() % 100 == 0)
			{
				PlayBuffer(SOUND_ELBELAND_BUTTERFLYPOLLUTION_MOVE01);
			}
		}
		else if(pObject->CurrentAction == MONSTER01_DIE)
		{
			PlayBuffer(SOUND_ELBELAND_BUTTERFLYPOLLUTION_DEATH01);
		}
		return true;
	case MODEL_MONSTER01+132:
		if (pObject->CurrentAction == MONSTER01_WALK)
		{
			if (rand() % 100 == 0)
			{
				PlayBuffer(SOUND_ELBELAND_CURSERICH_MOVE01);
			}
		}
		else if(pObject->CurrentAction == MONSTER01_ATTACK1 || pObject->CurrentAction == MONSTER01_ATTACK2)
		{
			PlayBuffer(SOUND_ELBELAND_CURSERICH_ATTACK01);
		}
		else if(pObject->CurrentAction == MONSTER01_DIE)
		{
			PlayBuffer(SOUND_ELBELAND_CURSERICH_DEATH01);
		}
		return true;
	case MODEL_MONSTER01+133:
		if (pObject->CurrentAction == MONSTER01_WALK)
		{
			if (rand() % 2 == 0)
				PlayBuffer(SOUND_ELBELAND_TOTEMGOLEM_MOVE01);
			else
				PlayBuffer(SOUND_ELBELAND_TOTEMGOLEM_MOVE02);
		}
		else if(pObject->CurrentAction == MONSTER01_ATTACK1)
		{
			PlayBuffer(SOUND_ELBELAND_TOTEMGOLEM_ATTACK01);
		}
		else if(pObject->CurrentAction == MONSTER01_ATTACK2)
		{
			PlayBuffer(SOUND_ELBELAND_TOTEMGOLEM_ATTACK02);
		}
		else if(pObject->CurrentAction == MONSTER01_DIE)
		{
			PlayBuffer(SOUND_ELBELAND_TOTEMGOLEM_DEATH01);
		}
		return true;
	case MODEL_MONSTER01+134:
		if (pObject->CurrentAction == MONSTER01_WALK)
		{
			if (rand() % 100 == 0)
			{
				PlayBuffer(SOUND_ELBELAND_BEASTWOO_MOVE01);
			}
		}
		else if(pObject->CurrentAction == MONSTER01_ATTACK1 || pObject->CurrentAction == MONSTER01_ATTACK2)
		{
			PlayBuffer(SOUND_ELBELAND_BEASTWOO_ATTACK01);
		}
		else if(pObject->CurrentAction == MONSTER01_DIE)
		{
			PlayBuffer(SOUND_ELBELAND_BEASTWOO_DEATH01);
		}
		return true;
	case MODEL_MONSTER01+135:
		if (pObject->CurrentAction == MONSTER01_WALK)
		{
			if (rand() % 100 == 0)
			{
				PlayBuffer(SOUND_ELBELAND_BEASTWOOLEADER_MOVE01);
			}
		}
		else if(pObject->CurrentAction == MONSTER01_ATTACK1 || pObject->CurrentAction == MONSTER01_ATTACK2)
		{
			PlayBuffer(SOUND_ELBELAND_BEASTWOOLEADER_ATTACK01);
		}
		else if(pObject->CurrentAction == MONSTER01_DIE)
		{
			PlayBuffer(SOUND_ELBELAND_BEASTWOO_DEATH01);
		}
		return true;
	}

	return false;
}

bool GMNewTown::AttackEffectMonster(CHARACTER* pCharacter, OBJECT* pObject, BMD* pModel)
{
	if(!IsCurrentMap())
		return false;

	return false;
}

bool GMNewTown::SetCurrentActionMonster(CHARACTER* pCharacter, OBJECT* pObject)
{
	if(!IsCurrentMap())
		return false;

	return false;
}

bool GMNewTown::CharacterSceneCheckMouse(OBJECT* pObj)
{
	m_bCharacterSceneCheckMouse = false;
	if(CheckMouseIn(480, 90, 30, 20) == true)
	{
		m_bCharacterSceneCheckMouse = true;
		SetAction(pObj, 2);
		return true;
	}
	else if(CheckMouseIn(485, 110, 50, 50) == true)
	{
		m_bCharacterSceneCheckMouse = true;
		SetAction(pObj, 0);
		return true;
	}
	
	return false;
}

bool GMNewTown::IsCheckMouseIn()
{
	return m_bCharacterSceneCheckMouse;
}
