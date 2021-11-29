// GM_Kanturu_In.cpp: implementation of the GM_Kanturu_In class.
//
//////////////////////////////////////////////////////////////////////


#include "stdafx.h"
#include "ZzzInfomation.h"
#include "ZzzBMD.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzAI.h"
#include "ZzzTexture.h"
#include "ZzzOpenData.h"
#include "ZzzEffect.h"
#include "ZzzLodTerrain.h"
#include "BoneManager.h"
#include "UIManager.h"
#include "CKanturuDirection.h"
#include "CDirection.h"
#include "UIBaseDef.h"
#include "MapManager.h"
#include "GM_Kanturu_2nd.h"
#include "DSPlaySound.h"
#include "wsclientinline.h"
#include "ChangeRingManager.h"
#include "LoadData.h"


extern float g_fScreenRate_x;
CTrapCanon g_TrapCanon;

using namespace M38Kanturu2nd;

// TODO

bool M38Kanturu2nd::Create_Kanturu2nd_Object(OBJECT* o)
{
	if(!Is_Kanturu2nd())
		return false;

	switch(o->Type) 
	{
	case 3:
		{
			CreateOperate(o);
		}
		break;
	}
	
	return true;
}

CHARACTER* M38Kanturu2nd::Create_Kanturu2nd_Monster(int iType, int PosX, int PosY, int Key)
{
	CHARACTER* pCharacter = NULL;

	switch(iType)
	{
	case 438:
	case 358:
		{
			OpenMonsterModel(114);
		    pCharacter = CreateCharacter(Key,MODEL_MONSTER01+114,PosX,PosY);
		    pCharacter->Object.Scale = 1.0f;
		    pCharacter->Weapon[0].Type = -1;
		    pCharacter->Weapon[1].Type = -1;

			BoneManager::RegisterBone(pCharacter, "PRSona_A1", 73);
			BoneManager::RegisterBone(pCharacter, "PRSona_Tail", 76);
			BoneManager::RegisterBone(pCharacter, "PRSona_Tail1", 77);
		}
		break;
	case 359:
		{
			OpenMonsterModel(115);
		    pCharacter = CreateCharacter(Key,MODEL_MONSTER01+115,PosX,PosY);
		    pCharacter->Object.Scale = 1.3f;
			pCharacter->Object.Angle[0] = 0.0f;
			pCharacter->Object.Gravity = 0.0f;
			pCharacter->Object.Distance = (float)(rand()%20)/10.0f;
		    pCharacter->Weapon[0].Type = -1;
		    pCharacter->Weapon[1].Type = -1;

			BoneManager::RegisterBone(pCharacter, "Twintail_Hair24", 16);
			BoneManager::RegisterBone(pCharacter, "Twintail_Hair32", 24);
		}
		break;
	case 439:
	case 360:
		{
			OpenMonsterModel(116);
		    pCharacter = CreateCharacter(Key,MODEL_MONSTER01+116,PosX,PosY);
		    pCharacter->Object.Scale = 1.3f;
			pCharacter->Object.Angle[0] = 0.0f;
			pCharacter->Object.Gravity = 0.0f;
			pCharacter->Object.Distance = (float)(rand()%20)/10.0f;
		    pCharacter->Weapon[0].Type = -1;
		    pCharacter->Weapon[1].Type = -1;

			BoneManager::RegisterBone(pCharacter, "Dreadfear_Wing32", 71);
			BoneManager::RegisterBone(pCharacter, "Dreadfear_Wing34", 68);
			BoneManager::RegisterBone(pCharacter, "Dreadfear_Wing51", 50);
			BoneManager::RegisterBone(pCharacter, "Dreadfear_Wing53", 47);
			BoneManager::RegisterBone(pCharacter, "Dreadfear_Eye52", 9);
			BoneManager::RegisterBone(pCharacter, "Dreadfear_Eye54", 10);

		}
		break;
	case 367:
		{
			OpenNpc(MODEL_KANTURU2ND_ENTER_NPC);
			pCharacter = CreateCharacter(Key, MODEL_KANTURU2ND_ENTER_NPC, PosX, PosY);
			strcpy(pCharacter->ID, "출입 관리 장치");
			pCharacter->Object.Scale = 4.76f;
			pCharacter->Object.Position[0] -= 20.0f;
			pCharacter->Object.Position[1] -= 200.0f;
			pCharacter->Object.m_bRenderShadow = false;
			SetAction(&pCharacter->Object, KANTURU2ND_NPC_ANI_STOP);

			g_pKanturu2ndEnterNpc->SetNpcObject(&pCharacter->Object);
			g_pKanturu2ndEnterNpc->SetNpcAnimation(false);

			BoneManager::RegisterBone(pCharacter, "KANTURU2ND_ENTER_NPC_1", 37);
			BoneManager::RegisterBone(pCharacter, "KANTURU2ND_ENTER_NPC_2", 38);
			BoneManager::RegisterBone(pCharacter, "KANTURU2ND_ENTER_NPC_3", 39);
			BoneManager::RegisterBone(pCharacter, "KANTURU2ND_ENTER_NPC_4", 40);
			BoneManager::RegisterBone(pCharacter, "KANTURU2ND_ENTER_NPC_5", 41);
			BoneManager::RegisterBone(pCharacter, "KANTURU2ND_ENTER_NPC_6", 42);
			BoneManager::RegisterBone(pCharacter, "KANTURU2ND_ENTER_NPC_7", 43);
			BoneManager::RegisterBone(pCharacter, "KANTURU2ND_ENTER_NPC_8", 6);
			BoneManager::RegisterBone(pCharacter, "KANTURU2ND_ENTER_NPC_9", 7);
			BoneManager::RegisterBone(pCharacter, "KANTURU2ND_ENTER_NPC_10", 8);
			BoneManager::RegisterBone(pCharacter, "KANTURU2ND_ENTER_NPC_11", 15);
			BoneManager::RegisterBone(pCharacter, "KANTURU2ND_ENTER_NPC_12", 16);
			BoneManager::RegisterBone(pCharacter, "KANTURU2ND_ENTER_NPC_13", 17);
			BoneManager::RegisterBone(pCharacter, "KANTURU2ND_ENTER_NPC_14", 10);
		}
		break;
	case 105:
		{
			pCharacter = g_TrapCanon.Create_TrapCanon(PosX, PosY, Key);
		}
		break;
	}
	
	return pCharacter;
}

bool M38Kanturu2nd::Set_CurrentAction_Kanturu2nd_Monster(CHARACTER* c, OBJECT* o)
{
	if(Is_Kanturu2nd_3rd() == false)
		return false;

	switch(c->MonsterIndex) 
	{
	case 358:
	case 359:
	case 360:
		{
			return CheckMonsterSkill(c, o);
		}
		break;
	}

	return false;
}

bool M38Kanturu2nd::AttackEffect_Kanturu2nd_Monster(CHARACTER* c, OBJECT* o, BMD* b)
{
	switch(o->Type) 
	{
	case MODEL_MONSTER01+114:
		{
			return true;
		}
		break;
	case MODEL_MONSTER01+115:
		{
			return true;
		}
		break;
	case MODEL_MONSTER01+116:
		{
			return true;
		}
		break;
	case MODEL_TRAP_CANON:
		{
			g_TrapCanon.Render_AttackEffect(c, o, b);
			return true;
		}
		break;
	}

	return false;
}

void M38Kanturu2nd::Sound_Kanturu2nd_Object(OBJECT* o)
{
	PlayBuffer(SOUND_KANTURU_2ND_MAPSOUND_GLOBAL);
	
	switch(o->Type)
	{
	case 9:
		PlayBuffer(SOUND_KANTURU_2ND_MAPSOUND_GEAR);
		break;
	case 31:
	case 35:	
	case 36:
	case 37:
		PlayBuffer(SOUND_KANTURU_2ND_MAPSOUND_INCUBATOR);
		break;
	}
}

bool M38Kanturu2nd::Move_Kanturu2nd_Object(OBJECT* o)
{
	if(!Is_Kanturu2nd())
		return false;

	Sound_Kanturu2nd_Object(o);
	
	switch(o->Type)
	{
		case 10:
			{
				o->Velocity = 0.04f;
				o->BlendMeshLight = (float)sinf(WorldTime*0.0015f) + 1.0f;

				if(o->BlendMeshLight <= 0.1f)
				{
					o->BlendMeshLight = 0.1f;
				}
				else if(o->BlendMeshLight >= 0.9f)
				{
					o->BlendMeshLight = 0.9f;
				}
			}
			break;
		case 38:
			{
				o->BlendMeshLight = (float)sinf(WorldTime*0.0010f)+1.0f;
			}
			break;
		case 42:
			{
				o->BlendMeshTexCoordU = -(int)WorldTime%10000 * 0.0002f;
				o->BlendMeshTexCoordV = -(int)WorldTime%10000 * 0.0002f;
			}
			break;
		case 44:
			{
				o->Velocity = 0.02f;
			}
			break;
		case 45:
		case 46:
		case 47:
		case 48:
		case 49:
		case 50:
		case 51:
		case 52:
		case 53:
		case 54:
		case 55:
		case 56:
		case 65:
			{
				o->HiddenMesh = -2;
			}
			break;
	}

	return true;
}

bool M38Kanturu2nd::Move_Kanturu2nd_MonsterVisual(CHARACTER* c,OBJECT* o, BMD* b)
{
	switch(o->Type)
	{
	case MODEL_MONSTER01+114:
		{
		}
		break;
	case MODEL_MONSTER01+115:
		{
			if(o->CurrentAction == MONSTER01_ATTACK2 &&
				(o->AnimationFrame >= 5.5f && o->AnimationFrame <= 6.5f))
			{
				CHARACTER *tc = &CharactersClient[c->TargetCharacter];
				OBJECT *to = &tc->Object;
				vec3_t vLight;

				if(gMapManager.WorldActive == WD_39KANTURU_3RD)
				{
					Vector(0.4f,0.9f,0.6f,vLight);
					for(int i=0; i<2; i++)
						CreateParticle(BITMAP_SMOKE, to->Position, to->Angle, vLight, 1);

					Vector(0.4f,1.0f,0.6f,vLight);
					CreateParticle(BITMAP_TWINTAIL_WATER, to->Position, to->Angle, vLight, 0);
				}
				else
				{
					Vector(0.4f,0.9f,0.6f,vLight);
					for(int i=0; i<5; i++)
					{
						CreateParticle(BITMAP_SMOKE, to->Position, to->Angle, vLight, 1);
					}

					Vector(0.4f,1.0f,0.6f,vLight);

					for(int i=0; i<2; i++)
					{
						CreateParticle(BITMAP_TWINTAIL_WATER, to->Position, to->Angle, vLight, 0);
					}
				}							
			}
		}
		break;
	case MODEL_MONSTER01+116:
		{
			
		}
		break;
	}
	
	return false;
}

bool M38Kanturu2nd::Render_Kanturu2nd_ObjectVisual(OBJECT* o, BMD* b)
{
	if(Is_Kanturu2nd() == false)
		return false;

	vec3_t Position, Light;

	switch(o->Type)
	{
	case 4:
		{
			vec3_t vPos, Light;
			float fLumi = (sinf(WorldTime*0.002f) + 2.0f) * 0.5f;
			Vector ( fLumi*0.3f, fLumi*0.5f, fLumi*1.0f, Light );
			Vector(-1.0f,  0.0f, 0.0f, vPos);
			b->TransformPosition(BoneTransform[1],vPos,Position,false);
			CreateSprite(BITMAP_LIGHT, Position, fLumi/3.2, Light, o);
			CreateSprite(BITMAP_KANTURU_2ND_EFFECT1, Position, fLumi/3.2, Light, o);
			CreateSprite(BITMAP_KANTURU_2ND_EFFECT1, Position, fLumi/3.2, Light, o);
		}
		break;
	case 8:
		{
			float fLumi = (sinf(WorldTime*0.001f) + 1.0f) * 0.5f;
			vec3_t vPos;
			Vector(0.0f,  0.0f, 0.0f, vPos);
			Vector ( fLumi, fLumi, fLumi, Light );
			b->TransformPosition(BoneTransform[4],vPos,Position,false);
			CreateParticle(BITMAP_ENERGY,Position,o->Angle,Light, 0, 1.5f);
			CreateSprite(BITMAP_SPARK+1, Position, 10.0f, Light, o);	
			vec3_t StartPos, EndPos;
			VectorCopy(Position, StartPos);
			VectorCopy(Position, EndPos);
			StartPos[0] -= 50.f;
			StartPos[0] -= rand()%100;
			EndPos[0] += rand()%80;
			StartPos[1] -= rand()%50;
			EndPos[1] += rand()%50;
			StartPos[2] += 10.0f;
			EndPos[2] += 10.f;
			if(rand()%20 == 0)
			{
				CreateJoint ( BITMAP_JOINT_THUNDER, StartPos, EndPos, o->Angle, 18, NULL, 40.f );
			}
		}
		break;
	case 10:
		{
			if(rand()%6 == 0) 
			{
				vec3_t vPos;
				VectorCopy(o->Position, vPos);
				vPos[2] = 410.f;
				Vector ( 1.f, 1.f, 1.f, Light );
				CreateParticle(BITMAP_WATERFALL_3,vPos,o->Angle,Light,7,o->Scale);
			}
		}
		break;
	case 45:
		{
			if(o->HiddenMesh != -2)
			{
				vec3_t  Light;
				Vector(0.06f,0.06f,0.06f,Light);
				for ( int i=0; i<20; ++i )
				{
					CreateParticle ( BITMAP_CLOUD, o->Position, o->Angle, Light, 1, o->Scale, o );
				}
			}
		}
		break;
	case 46:
		{
			if(o->HiddenMesh != -2)
			{
				vec3_t  Light;
				Vector(0.06f,0.06f,0.06f,Light);
				for ( int i=0; i<20; ++i )
				{
					CreateParticle ( BITMAP_CLOUD, o->Position, o->Angle, Light, 2, o->Scale, o );
				}
			}
		}
		break;
	case 47:
		{
			if(o->HiddenMesh != -2)
			{
				vec3_t  Light;
				Vector(0.2f,0.2f,0.2f,Light);
				for ( int i=0; i<20; ++i )
				{
					CreateParticle ( BITMAP_CLOUD, o->Position, o->Angle, Light, 7, o->Scale, o );
				}
			}
		}
		break;
	case 48:
		{
			if(rand()%3 == 0)
			{
				vec3_t  Light;
				Vector(0.2f,0.2f,0.2f,Light);
				CreateParticle ( BITMAP_CLOUD, o->Position, o->Angle, Light, 10, o->Scale, o );
			}
		}
		break;
	case 49:
		{
			if ( o->HiddenMesh!=-2 )
			{
				vec3_t  Light;
				Vector(0.0f,0.01f,0.03f,Light);
				for ( int i=0; i<5; ++i )
				{
					CreateParticle ( BITMAP_CLOUD, o->Position, o->Angle, Light, 1, o->Scale, o );
				}
			}
		}
		break;
	case 50:
		{
			if(rand()%3 == 0)
			{
				float fBlue = (rand()%3)*0.01f + 0.02f;
				vec3_t  Light;
				Vector(0.0f,0.01f,fBlue,Light);
				CreateParticle ( BITMAP_CLOUD, o->Position, o->Angle, Light, 11, o->Scale, o );
			}
		}
		break;
	case 51:
		{
			if(rand()%3 == 0)
			{
				float fRed = (rand()%3)*0.01f + 0.01f;
				vec3_t  Light;
				Vector(fRed,0.00f,0.0f,Light);
				CreateParticle ( BITMAP_CLOUD, o->Position, o->Angle, Light, 11, o->Scale, o );
			}
		}
		break;
	case 52:
		{
			if(rand()%6 == 0) 
			{
				Vector ( 1.f, 1.f, 1.f, Light );
				CreateParticle(BITMAP_WATERFALL_3,o->Position,o->Angle,Light,7, o->Scale);
			}
		}
		break;
	case 53:
		{
			if(rand()%3 == 0)
			{
				Vector(0.4f,0.6f,0.7f,Light);
				CreateParticle(BITMAP_TWINTAIL_WATER, o->Position, o->Angle, Light, 1);
			}
		}
		break;
	case 54:
		{
			if(rand()%20 == 0)
			{
				CreateJoint( BITMAP_JOINT_THUNDER+1, o->Position, o->Position, o->Angle, 8, NULL, 30.f+rand()%10 );
			}
		}
		break;
	case 55:
		{
			if(rand()%5 == 0)
			{
				Vector(0.8f,0.8f,1.0f,Light);
				CreateEffect(MODEL_FENRIR_THUNDER, o->Position, o->Angle, Light, 1, o);
			}
			
		}
		break;
	case 56:
		{
			if(rand()%10 == 0)
			{
				CreateJoint( BITMAP_JOINT_THUNDER+1, o->Position, o->Position, o->Angle, 9, NULL, 30.f+rand()%10 );
			}
		}
		break;
	case 65:
		{
			if(rand()%10 == 0)
			{
				CreateJoint( BITMAP_JOINT_THUNDER+1, o->Position, o->Position, o->Angle, 10, NULL, 30.f+rand()%10 );
			}
		}
		break;
	}

	return true;
}

bool M38Kanturu2nd::Render_Kanturu2nd_ObjectMesh(OBJECT* o, BMD* b,bool ExtraMon)
{
	if(Is_Kanturu2nd() == true)
	{
		switch(o->Type) 
		{
		case 1:
			{
				float fLumi = (sinf(WorldTime*0.002f) + 1.f) * 0.5f;
				o->HiddenMesh = 1;
				b->RenderBody(RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
				b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, 1, fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);

				return true;
			}
			break;
		case 2:
			{
				float fLumi = (sinf(WorldTime*0.002f) + 1.f) * 0.5f;
				o->HiddenMesh = 1;
				b->RenderBody(RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
				b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, 1, fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);

				return true;
			}
			break;
		case 12:
			{
				o->HiddenMesh = 0;
				b->StreamMesh = 1;
				b->RenderBody(RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,-(int)WorldTime%2000*0.0005f,o->HiddenMesh);
				b->StreamMesh = -1;
				b->RenderMesh(0,RENDER_TEXTURE,o->Alpha,0,o->BlendMeshLight,(int)WorldTime%2000*0.0005f,(int)WorldTime%2000*0.0005f);
				b->RenderMesh(0,RENDER_CHROME|RENDER_DARK,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,BITMAP_CHROME);

				return true;
			}
			break;
		case 13:
			{
				b->StreamMesh = 1;
				b->RenderBody(RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,-(int)WorldTime%2000*0.0005f,o->HiddenMesh);
				b->StreamMesh = -1;

				return true;
			}
			break;
		case 14:
			{
				b->StreamMesh = 1;
				b->RenderBody(RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,-(int)WorldTime%2000*0.0005f,o->HiddenMesh);
				b->StreamMesh = -1;

				return true;
			}
			break;
		case 15:
		case 16:
			{
				b->StreamMesh = 3;
				b->RenderBody(RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,-(int)WorldTime%2000*0.0005f,o->HiddenMesh);
				b->StreamMesh = -1;

				return true;
			}
			break;
		case 27:
			{
				o->HiddenMesh = 2;
				b->StreamMesh = 4;
				b->RenderBody(RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,-(int)WorldTime%2000*0.0005f,o->HiddenMesh);
				b->StreamMesh = -1;
				b->RenderMesh(2, RENDER_TEXTURE|RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);

				return true;
			}
			break;
		case 8:
		case 10:
		case 31:
		case 33:
		case 35:
		case 36:
		case 59:
		case 76:
		case 80:
			{
				o->m_bRenderAfterCharacter = true;

				return true;
			}
			break;
		}
	}

	return Render_Kanturu2nd_MonsterObjectMesh(o, b,ExtraMon);
}

void M38Kanturu2nd::Render_Kanturu2nd_AfterObjectMesh(OBJECT* o, BMD* b)
{
	if(false == Is_Kanturu2nd())
		return;

	if(o->Type == 31)
	{
		o->HiddenMesh = 0;
		b->RenderBody(RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
		b->RenderMesh(0, RENDER_TEXTURE|RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
	}
	else if(o->Type == 8)
	{
		b->RenderBody(RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
		b->RenderMesh(1, RENDER_CHROME|RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_CHROME);
	}
	else if(o->Type == 10)
	{
		o->HiddenMesh = 2;
		b->RenderBody(RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
		b->RenderMesh(2, RENDER_CHROME|RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_CHROME);
	}
	else if(o->Type == 35 || o->Type == 36)	
	{
		o->HiddenMesh = 1;
		b->RenderBody(RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
		b->RenderMesh(1, RENDER_TEXTURE|RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
	}
	else if(o->Type == 76)
	{
		b->BeginRender(o->Alpha);

		b->BodyLight[0] = 0.4f;
		b->BodyLight[1] = 0.4f;
		b->BodyLight[2] = 0.4f;

		b->RenderMesh(0,RENDER_TEXTURE|RENDER_BRIGHT,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV);
		b->RenderMesh(0,RENDER_CHROME7|RENDER_DARK,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,BITMAP_CHROME);
		b->EndRender();
		
		vec3_t Light, p, Position;
		for (int i = 0; i < 10; ++i)
		{
			Vector(0.0f,  0.0f, 0.0f, p);
			b->TransformPosition(BoneTransform[i],p,Position,false);
			Vector(0.1f,0.1f,0.3f,Light);
			CreateSprite(BITMAP_SPARK+1, Position, 7.5f, Light, o);
		}
	}
	else if(o->Type == 33 || o->Type == 59 || o->Type == 80)
	{
		b->RenderBody(RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
	}
}

bool M38Kanturu2nd::Render_Kanturu2nd_MonsterObjectMesh(OBJECT* o, BMD* b,int ExtraMon)
{
	switch(o->Type)
	{
	case MODEL_MONSTER01+114:
		{
			float fLumi2 = (sinf(WorldTime*0.002f) + 1.f) * 0.5f;
			b->RenderBody(RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
			b->RenderMesh(0, RENDER_TEXTURE|RENDER_BRIGHT, o->Alpha, 0, fLumi2, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_PRSONA_EFFECT);
			b->RenderMesh(2, RENDER_TEXTURE|RENDER_BRIGHT, o->Alpha, 2, fLumi2, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_PRSONA_EFFECT2);
			return true;	
		}
		break;
	case MODEL_MONSTER01+115:
		{
			float fLumi = (sinf(WorldTime*0.002f) + 1.f) * 0.5f;
			float fLumi2 = (sinf(WorldTime*0.002f) + 1.f);

			if(o->CurrentAction != MONSTER01_DIE)
			{
				Vector(0.9f,0.9f,1.0f,b->BodyLight);
			}
			else
			{
				Vector(0.3f,1.0f,0.2f,b->BodyLight);
			}

			b->RenderBody(RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);

			b->RenderMesh(2, RENDER_TEXTURE|RENDER_BRIGHT, o->Alpha, 2, fLumi2, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_TWINTAIL_EFFECT);

			b->RenderMesh(3, RENDER_TEXTURE|RENDER_BRIGHT, o->Alpha, 3, fLumi);
			return true;	
		}
		break;
	case MODEL_MONSTER01+116:	
		{
			if(o->CurrentAction == MONSTER01_DIE)
			{
				o->Alpha -= 0.1f;
				if(o->Alpha <= 0.0f)
				{
					o->Alpha = 0.0f;
				}
			}
			b->RenderBody(RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
			return true;
		}
		break;
	case MODEL_KANTURU2ND_ENTER_NPC:
		{
			b->RenderMesh(0, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
			
			b->RenderMesh(1, RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV,BITMAP_CHROME);
			b->RenderMesh(1, RENDER_TEXTURE|RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_KANTURU_2ND_NPC3);
			b->RenderMesh(1, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
			
			b->RenderMesh(2, RENDER_CHROME, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV,BITMAP_CHROME);
			b->RenderMesh(2, RENDER_TEXTURE|RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_KANTURU_2ND_NPC2);
			b->RenderMesh(2, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
			
			b->RenderMesh(3, RENDER_TEXTURE, o->Alpha, 3, o->BlendMeshLight, o->BlendMeshTexCoordU, -WorldTime*0.0003f,BITMAP_CHROME);
			b->RenderMesh(3, RENDER_TEXTURE|RENDER_BRIGHT, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, BITMAP_KANTURU_2ND_NPC1);
			b->RenderMesh(3, RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
		
			return true;
		}
		break;
	case MODEL_TRAP_CANON:
		{
			g_TrapCanon.Render_Object(o, b);
			return true;
		}
		break;
	}

	return false;
}

bool M38Kanturu2nd::Render_Kanturu2nd_MonsterVisual(CHARACTER* c, OBJECT* o, BMD* b)
{
	switch(o->Type)
	{
	case MODEL_MONSTER01+114:
		{
			if (o->CurrentAction == MONSTER01_WALK)
			{
				if (rand() % 15 == 0)
				{
					PlayBuffer(SOUND_KANTURU_2ND_PERSO_MOVE1 + rand() % 2);
				}
			}
			else if(o->CurrentAction == MONSTER01_ATTACK1)
			{
				if (o->SubType == FALSE)
				{
					o->SubType = TRUE;
					PlayBuffer(SOUND_KANTURU_2ND_PERSO_ATTACK1);
				}
			}
			else if(o->CurrentAction == MONSTER01_ATTACK2)
			{
				if (o->SubType == FALSE)
				{
					o->SubType = TRUE;
					PlayBuffer(SOUND_KANTURU_2ND_PERSO_ATTACK2);
				}
			}
			else if(o->CurrentAction == MONSTER01_DIE)
			{
				o->SubType = TRUE;
				PlayBuffer(SOUND_KANTURU_2ND_PERSO_DIE);
			}

			if (o->CurrentAction == MONSTER01_STOP1 
				|| o->CurrentAction == MONSTER01_STOP2
				|| o->CurrentAction == MONSTER01_WALK)
			{
				o->SubType = FALSE;
			}

			vec3_t vPos;
			vec3_t vLight = {1.0f, 1.0f, 1.0f};
			float Luminosity = (float)(rand()%30 + 70)*0.01f;
			Vector(Luminosity*0.5f,Luminosity*0.6f,Luminosity*1.0f,vLight);

			BoneManager::GetBonePosition(o, "PRSona_A1", vPos);
			CreateSprite ( BITMAP_LIGHT, vPos, 2.0f, vLight, o );

			if(rand()%5 == 0)
			{
				CreateParticle(BITMAP_SMOKE, o->Position, o->Angle, vLight, 40);
			}

			if(rand()%3 == 0)
			{
				Vector(0.5f, 0.5f, 0.5f, vLight);
				BoneManager::GetBonePosition(o, "PRSona_Tail", vPos);
				CreateParticle ( BITMAP_WATERFALL_3, vPos, o->Angle, vLight, 5, 0.8f, o );
				BoneManager::GetBonePosition(o, "PRSona_Tail1", vPos);
				CreateParticle ( BITMAP_WATERFALL_3, vPos, o->Angle, vLight, 5, 0.8f, o );
			}

			if(o->CurrentAction == MONSTER01_ATTACK1 && (o->AnimationFrame >= 3.f && o->AnimationFrame <= 4.f))
			{
				Vector(0.6f, 0.6f, 1.0f, vLight);
				CreateEffect(MODEL_STORM,o->Position,o->Angle,vLight, 0);
			}
			else if(o->CurrentAction == MONSTER01_ATTACK2 && (o->AnimationFrame >= 3.f && o->AnimationFrame <= 4.f))
			{
				vec3_t vPos;
				VectorCopy(o->Position, vPos);
				vPos[2] += 100.f;
				
				int i;
				for(int j=0; j<2; j++)
				{
					i = rand()%4;
					vec3_t vAngle;
					Vector(0.f, 0.f, i*90.f, vAngle);

					CreateJoint(BITMAP_JOINT_SPIRIT,vPos,o->Position,vAngle,0,o,80.f);
					CreateJoint(BITMAP_JOINT_SPIRIT,vPos,o->Position,vAngle,0,o,20.f);
				}
			}

			else if(o->CurrentAction == MONSTER01_DIE)
			{
				o->BlendMesh = -2;
				Vector(0.5f, 0.5f, 0.5f, vLight);
				BoneManager::GetBonePosition(o, "PRSona_Tail", vPos);
				CreateParticle ( BITMAP_WATERFALL_3, vPos, o->Angle, vLight, 6, 0.8f, o );
				BoneManager::GetBonePosition(o, "PRSona_Tail1", vPos);
				CreateParticle ( BITMAP_WATERFALL_3, vPos, o->Angle, vLight, 6, 0.8f, o );

				for(int i=0; i<5; i++)
				{
					int j = rand()%90;
					vec3_t p;
					Vector ( 0.f, 0.f, 0.f, p );
					Vector(0.5f, 0.5f, 0.5f, vLight);
					b->TransformPosition ( o->BoneTransform[j], p, vPos, true );
					CreateParticle ( BITMAP_WATERFALL_3, vPos, o->Angle, vLight, 6, 0.8f, o );
				}
			}

			return true;	
		}
		break;
	case MODEL_MONSTER01+115:
		{
			if(gMapManager.WorldActive == WD_39KANTURU_3RD && g_Direction.m_CKanturu.m_iKanturuState == KANTURU_STATE_MAYA_BATTLE)
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
				Light[2] = 0.0f;

				EnableAlphaBlend();
				RenderTerrainAlphaBitmap ( BITMAP_ENERGY_RING, o->Position[0], o->Position[1], Scale, Scale, Light, Angle );

				Vector(0.5f, 0.5f, 0.1f, Light);
				RenderTerrainAlphaBitmap ( BITMAP_ENERGY_FIELD, o->Position[0], o->Position[1], 2.0f, 2.0f, Light, Angle );
			}

			if(o->CurrentAction == MONSTER01_ATTACK1)
			{
				if (o->SubType == FALSE)
				{
					o->SubType = TRUE;
					PlayBuffer(SOUND_KANTURU_2ND_TWIN_ATTACK1);
				}
			}
			else if(o->CurrentAction == MONSTER01_ATTACK2)
			{
				if (o->SubType == FALSE)
				{
					o->SubType = TRUE;
					PlayBuffer(SOUND_KANTURU_2ND_TWIN_ATTACK2);
				}
			}
			else if(o->CurrentAction == MONSTER01_DIE)
			{
				o->SubType = TRUE;
				PlayBuffer(SOUND_KANTURU_2ND_TWIN_DIE);
			}

			if (o->CurrentAction == MONSTER01_STOP1 
				|| o->CurrentAction == MONSTER01_STOP2
				|| o->CurrentAction == MONSTER01_WALK)
			{
				o->SubType = FALSE;
			}

			vec3_t vRelative, vPos;
			vec3_t vLight = {0.6f, 1.0f, 0.4f};

			if(o->CurrentAction == MONSTER01_WALK)
			{
				DWORD dwTime = timeGetTime() - o->m_dwTime;
				if(dwTime > 500)
				{
					if(o->m_iAnimation % 2 == 1)
					{
						Vector(0.0f, 0.0f, 0.0f, vRelative);
						BoneManager::GetBonePosition(o, "Twintail_Hair32", vRelative, vPos);
						CreateEffect(MODEL_TWINTAIL_EFFECT, vPos, o->Angle, vLight, 0, o);
					}
					else if(o->m_iAnimation % 2 == 0)
					{
						Vector(0.0f, 0.0f, 0.0f, vRelative);
						BoneManager::GetBonePosition(o, "Twintail_Hair24", vRelative, vPos);
						CreateEffect(MODEL_TWINTAIL_EFFECT, vPos, o->Angle, vLight, 0, o);
					}

					if(o->m_iAnimation % 2 == 0)
					{
						o->m_iAnimation = 1;
					}
					else if(o->m_iAnimation  % 2 == 1)
					{
						o->m_iAnimation = 0;
					}
					o->m_dwTime = timeGetTime();
					
					PlayBuffer(SOUND_KANTURU_2ND_TWIN_MOVE1 + rand() % 2);
				}
			}
			else if(o->CurrentAction == MONSTER01_DIE)
			{
				o->BlendMesh = -2;
				o->m_bRenderShadow = false;
				if(o->AnimationFrame <= 3.0f)
				{
					Vector(0.1f, 1.0f, 0.2f, vLight);
					for(int i=0; i<5; i++)
					{
						CreateParticle(BITMAP_SMOKE, o->Position, o->Angle, vLight, 39);
					}
					Vector(0.4f, 1.0f, 0.6f, vLight);
					CreateParticle(BITMAP_TWINTAIL_WATER, o->Position, o->Angle, vLight, 0);

					CreateEffect ( MODEL_SKILL_INFERNO, o->Position, o->Angle, o->Light, 9, o );

					if(o->AnimationFrame <= 0.2f)
					{
						Vector(0.4f, 1.0f, 0.6f, vLight);
						CreateEffect(MODEL_TWINTAIL_EFFECT, o->Position, o->Angle, vLight, 1, o);
						CreateEffect(MODEL_TWINTAIL_EFFECT, o->Position, o->Angle, vLight, 2, o);
					}
				}
			}
			return true;	
		}
		break;
	case MODEL_MONSTER01+116:
		{
			if(gMapManager.WorldActive == WD_39KANTURU_3RD && g_Direction.m_CKanturu.m_iKanturuState == KANTURU_STATE_MAYA_BATTLE)
			{
				vec3_t Light;
				float Scale, Angle;

				o->Distance += 0.05f; 
				Scale = o->Distance;

				if(Scale >= 3.0f)
					o->Distance = 0.1f;

				Angle = (timeGetTime()%9000)/10.0f;

				Light[0] = 1.0f - (Scale/3.0f);
				Light[1] = 0.0f;
				Light[2] = 0.0f;

				EnableAlphaBlend();
				RenderTerrainAlphaBitmap ( BITMAP_ENERGY_RING, o->Position[0], o->Position[1], Scale, Scale, Light, Angle );

				Vector(1.0f, 0.3f, 0.1f, Light);
				RenderTerrainAlphaBitmap ( BITMAP_ENERGY_FIELD, o->Position[0], o->Position[1], 2.0f, 2.0f, Light, Angle );
			}

			if (o->CurrentAction == MONSTER01_WALK)
			{
				if (rand() % 15 == 0)
				{
					PlayBuffer(SOUND_KANTURU_2ND_DRED_MOVE1 + rand() % 2);
				}
			}
			else if(o->CurrentAction == MONSTER01_ATTACK1)
			{
				if (o->SubType == FALSE)
				{
					o->SubType = TRUE;
					PlayBuffer(SOUND_KANTURU_2ND_DRED_ATTACK1);
				}
			}
			else if(o->CurrentAction == MONSTER01_ATTACK2 && (o->AnimationFrame >= 0 && o->AnimationFrame <= 2))
			{
				o->SubType = TRUE;
				PlayBuffer(SOUND_KANTURU_2ND_DRED_ATTACK2);
			}
			else if(o->CurrentAction == MONSTER01_DIE)
			{
				o->SubType = TRUE;
				PlayBuffer(SOUND_KANTURU_2ND_DRED_DIE);
			}

			if (o->CurrentAction == MONSTER01_STOP1 
				|| o->CurrentAction == MONSTER01_STOP2
				|| o->CurrentAction == MONSTER01_WALK)
			{
				o->SubType = FALSE;
			}

			vec3_t vPos, vLight;
			float fScale;
			Vector(1.f, 1.f, 1.f, vLight);

			if(o->CurrentAction != MONSTER01_STOP1 && o->CurrentAction != MONSTER01_DIE && gMapManager.WorldActive != WD_39KANTURU_3RD)
			{
				if(rand()%3 == 0) 
				{
					BoneManager::GetBonePosition(o, "Dreadfear_Wing32", vPos);
					CreateParticle(BITMAP_CLUD64, vPos, o->Angle, vLight, 0);
					BoneManager::GetBonePosition(o, "Dreadfear_Wing34", vPos);
					CreateParticle(BITMAP_CLUD64, vPos, o->Angle, vLight, 0);
				}
				if(rand()%3 == 0)
				{
					BoneManager::GetBonePosition(o, "Dreadfear_Wing51", vPos);
					CreateParticle(BITMAP_CLUD64, vPos, o->Angle, vLight, 0);
					BoneManager::GetBonePosition(o, "Dreadfear_Wing53", vPos);
					CreateParticle(BITMAP_CLUD64, vPos, o->Angle, vLight, 0);
				}	
			}
			else if(o->CurrentAction == MONSTER01_DIE)
			{
				vec3_t vRelative;
				Vector(0.f, 0.f, 0.f, vRelative);
				BoneManager::GetBonePosition(o, "Dreadfear_Wing32", vRelative, vPos);
				CreateParticle(BITMAP_SMOKE+3, vPos, o->Angle, vLight, 3, 1.0f);
				BoneManager::GetBonePosition(o, "Dreadfear_Wing34", vRelative, vPos);
				CreateParticle(BITMAP_SMOKE+3, vPos, o->Angle, vLight, 3, 1.0f);
				BoneManager::GetBonePosition(o, "Dreadfear_Wing51", vRelative, vPos);
				CreateParticle(BITMAP_SMOKE+3, vPos, o->Angle, vLight, 3, 1.0f);
				BoneManager::GetBonePosition(o, "Dreadfear_Wing53", vRelative, vPos);
				CreateParticle(BITMAP_SMOKE+3, vPos, o->Angle, vLight, 3, 1.0f);
			}
						
			Vector(0.25f, 0.7f, 0.6f, vLight);
			fScale = (rand()%10-5)*0.01f;
			BoneManager::GetBonePosition(o, "Dreadfear_Eye52", vPos);
			CreateSprite(BITMAP_LIGHT+1, vPos, 0.5f+fScale, vLight, o);
			BoneManager::GetBonePosition(o, "Dreadfear_Eye54", vPos);
			CreateSprite(BITMAP_LIGHT+1, vPos, 0.5f+fScale, vLight, o);

			return true;
		}
		break;
	case MODEL_KANTURU2ND_ENTER_NPC:
		{
			if(o->CurrentAction == KANTURU2ND_NPC_ANI_ROT)
			{
				PlayBuffer(SOUND_KANTURU_2ND_MAPSOUND_HOLE);

				int iAnimationFrame = (int)o->AnimationFrame;

				vec3_t vPos, vLight;
				Vector(0.f, 0.f, 0.f, vPos);
				Vector(1.0f,1.0f,2.0f,vLight);
				if(iAnimationFrame < 40)
				{
					if(rand()%4 == 0)
					{
						BoneManager::GetBonePosition(o, "KANTURU2ND_ENTER_NPC_1", vPos);
						CreateParticle(BITMAP_SPARK+1, vPos, o->Angle, vLight, 18, 1.0f);
						BoneManager::GetBonePosition(o, "KANTURU2ND_ENTER_NPC_2", vPos);
						CreateParticle(BITMAP_SPARK+1, vPos, o->Angle, vLight, 18, 1.0f);
						BoneManager::GetBonePosition(o, "KANTURU2ND_ENTER_NPC_3", vPos);
						CreateParticle(BITMAP_SPARK+1, vPos, o->Angle, vLight, 18, 1.0f);
						BoneManager::GetBonePosition(o, "KANTURU2ND_ENTER_NPC_4", vPos);
						CreateParticle(BITMAP_SPARK+1, vPos, o->Angle, vLight, 18, 1.0f);
						BoneManager::GetBonePosition(o, "KANTURU2ND_ENTER_NPC_5", vPos);
						CreateParticle(BITMAP_SPARK+1, vPos, o->Angle, vLight, 18, 1.0f);
						BoneManager::GetBonePosition(o, "KANTURU2ND_ENTER_NPC_6", vPos);
						CreateParticle(BITMAP_SPARK+1, vPos, o->Angle, vLight, 18, 1.0f);
						BoneManager::GetBonePosition(o, "KANTURU2ND_ENTER_NPC_7", vPos);
						CreateParticle(BITMAP_SPARK+1, vPos, o->Angle, vLight, 18, 1.0f);
					}
				}

				Vector(1.0f, 1.0f, 2.f, vLight);
				if(rand()%4 == 0)
				{
					BoneManager::GetBonePosition(o, "KANTURU2ND_ENTER_NPC_8", vPos);
					CreateParticle(BITMAP_CLUD64, vPos, o->Angle, vLight, 2, 2.f);
					BoneManager::GetBonePosition(o, "KANTURU2ND_ENTER_NPC_9", vPos);
					CreateParticle(BITMAP_CLUD64, vPos, o->Angle, vLight, 2, 2.f);
					BoneManager::GetBonePosition(o, "KANTURU2ND_ENTER_NPC_10", vPos);
					CreateParticle(BITMAP_CLUD64, vPos, o->Angle, vLight, 2, 2.f);
					BoneManager::GetBonePosition(o, "KANTURU2ND_ENTER_NPC_11", vPos);
					CreateParticle(BITMAP_CLUD64, vPos, o->Angle, vLight, 2, 2.f);
					BoneManager::GetBonePosition(o, "KANTURU2ND_ENTER_NPC_12", vPos);
					CreateParticle(BITMAP_CLUD64, vPos, o->Angle, vLight, 2, 2.f);
					BoneManager::GetBonePosition(o, "KANTURU2ND_ENTER_NPC_13", vPos);
					CreateParticle(BITMAP_CLUD64, vPos, o->Angle, vLight, 2, 2.f);
				}

				if(iAnimationFrame <= 10)
				{
					Vector(0.0f,0.0f,0.0f,vLight);
				}
				else if(iAnimationFrame == 10)
				{
					Vector(0.1f,0.1f,0.1f,vLight);
				}
				else if(iAnimationFrame >= 11 && iAnimationFrame <= 28)
				{
					vLight[0] = (iAnimationFrame - 11)*0.06f;
					vLight[1] = (iAnimationFrame - 11)*0.06f;
					vLight[2] = (iAnimationFrame - 11)*0.06f;
				}
				else if(iAnimationFrame >= 29)
				{
					Vector(1.0f,1.0f,1.0f,vLight);
				}
				VectorCopy(o->Position, vPos);
				vPos[2] += 310;	// Y
				vPos[1] -= 100;	// X
				vPos[0] += 40;	// Z
				CreateSprite(BITMAP_LIGHTNING+1,vPos,1.0,vLight,o,(WorldTime/10.0f));
				
				if(iAnimationFrame >= 12 && iAnimationFrame <= 25)
				{
					vec3_t vPos, vPos2;
					VectorCopy(o->Position, vPos2);
					vPos2[2] += 290.f;
					VectorCopy(o->Position, vPos);
					vPos[0] += (rand()%600 - 300.f);
					vPos[1] -= (rand()%200 + 350.f);
					vPos[2] += (rand()%600 - 100.f);
					CreateJoint(BITMAP_JOINT_ENERGY, vPos, vPos2, o->Angle, 42, o, 10.f);
					CreateJoint(BITMAP_JOINT_ENERGY, vPos, vPos2, o->Angle, 42, o, 10.f);
				}

				VectorCopy(o->Position, vPos);
				vPos[0] += 260.0f;
				vPos[1] -= 115.0f;
				vPos[2] += 50.f;
				if(iAnimationFrame >= 1 && iAnimationFrame <= 9)
				{
					vPos[0] += iAnimationFrame*4;
				}
				else if(iAnimationFrame >= 10 && iAnimationFrame <= 18)
				{
					vPos[0] += 36.0f;
					vPos[0] -= (iAnimationFrame-9)*4;
				}

				if(iAnimationFrame >= 1 && iAnimationFrame <= 20)
				{
					vec3_t vAngle;
					Vector((float)(rand()%60+290),0.f,(float)(rand()%30+90),vAngle);
					CreateJoint(BITMAP_JOINT_SPARK, vPos, vPos, vAngle, 0);
				}
				
				if(iAnimationFrame >= 42)
				{
					if(g_pKanturu2ndEnterNpc->IsEnterRequest() == false)
					{
						if(Hero->Dead == 0)
						{
							g_pKanturu2ndEnterNpc->SendRequestKanturu3rdEnter();
						}
					}
				}

				if(iAnimationFrame >= 50)
				{
					o->AnimationFrame = 0;
					SetAction(o, KANTURU2ND_NPC_ANI_STOP);
					g_pKanturu2ndEnterNpc->SetNpcAnimation(false);
					g_pKanturu2ndEnterNpc->SetEnterRequest(false);
				}
			}
		}
		break;
	case MODEL_TRAP_CANON:
		{
			g_TrapCanon.Render_Object_Visual(c, o, b);
		}
		break;
	}
	return false;
}

bool M38Kanturu2nd::Is_Kanturu2nd()
{
	if(gMapManager.WorldActive == WD_38KANTURU_2ND)
		return true;

	return false;
}

bool M38Kanturu2nd::Is_Kanturu2nd_3rd()
{
	if(gMapManager.WorldActive == WD_38KANTURU_2ND || gMapManager.WorldActive == WD_39KANTURU_3RD)
		return true;

	return false;
}

void M38Kanturu2nd::Move_Kanturu2nd_BlurEffect(CHARACTER* c, OBJECT* o, BMD* b)
{
	switch(o->Type)
	{
	case MODEL_MONSTER01+114:
		{
		}
		break;
	case MODEL_MONSTER01+115:
		{
			if((o->AnimationFrame <= 4.12f && o->CurrentAction == MONSTER01_ATTACK1))
			{
				vec3_t  Light;
				Vector(1.0f, 0.2f, 0.5f, Light);

				vec3_t StartPos, StartRelative;
				vec3_t EndPos, EndRelative;
				
				float fActionSpeed = b->Actions[o->CurrentAction].PlaySpeed;
				float fSpeedPerFrame = fActionSpeed/10.f;
				float fAnimationFrame = o->AnimationFrame - fActionSpeed;
				for(int i=0; i<10; i++) 
				{
					b->Animation(BoneTransform, fAnimationFrame, o->PriorAnimationFrame, o->PriorAction, o->Angle, o->HeadAngle);

					Vector(-150.f, 50.f, 0.f, StartRelative);
					Vector(150.f, -200.f, 0.f, EndRelative);

					b->TransformPosition(BoneTransform[58], StartRelative, StartPos, false);
					b->TransformPosition(BoneTransform[59], EndRelative, EndPos, false);
					CreateBlur(c, StartPos, EndPos, Light, 1);
					
					fAnimationFrame += fSpeedPerFrame;
				}
			}
			else if(o->CurrentAction == MONSTER01_ATTACK2)
			{
				CHARACTER *tc = &CharactersClient[c->TargetCharacter];
				OBJECT *to = &tc->Object;
				
				vec3_t vPos, vRelative;
				Vector(0.f, 0.f, 0.f, vRelative);
				if(gMapManager.WorldActive == WD_39KANTURU_3RD)
				{
					if(rand()%4 == 0)
					{
						BoneManager::GetBonePosition(o, "Twintail_Hair24", vRelative, vPos);
						CreateJoint(BITMAP_JOINT_ENERGY, vPos, to->Position, o->Angle, 0, to, 30.f);
					}
					
					if(rand()%4 == 0)
					{
						BoneManager::GetBonePosition(o, "Twintail_Hair32", vRelative, vPos);
						CreateJoint(BITMAP_JOINT_ENERGY, vPos, to->Position, o->Angle, 0, to, 30.f);
					}
				}
				else
				{
					if(rand()%2 == 0)
					{
						BoneManager::GetBonePosition(o, "Twintail_Hair24", vRelative, vPos);
						CreateJoint(BITMAP_JOINT_ENERGY, vPos, to->Position, o->Angle, 0, to, 30.f);
					}
					
					if(rand()%2 == 0)
					{
						BoneManager::GetBonePosition(o, "Twintail_Hair32", vRelative, vPos);
						CreateJoint(BITMAP_JOINT_ENERGY, vPos, to->Position, o->Angle, 0, to, 30.f);
					}
				}
			}
		}
		break;
	case MODEL_MONSTER01+116:
		{
			if((o->AnimationFrame <= 5.0f && o->CurrentAction == MONSTER01_ATTACK1) 
				|| (o->AnimationFrame <= 9.0f && o->CurrentAction == MONSTER01_ATTACK2))
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

					Vector(0.f, 0.f, 0.f, StartRelative);
					Vector(100.f, 120.f, 0.f, EndRelative);

					b->TransformPosition(BoneTransform[33], StartRelative, StartPos, false);
					b->TransformPosition(BoneTransform[33], EndRelative, EndPos, false);
					CreateBlur(c, StartPos, EndPos, Light, 4);
					
					fAnimationFrame += fSpeedPerFrame;
				}
			}
		}
		break;
	}
}

extern char* g_lpszMp3[NUM_MUSIC];

void M38Kanturu2nd::PlayBGM()
{
	if(gMapManager.WorldActive == WD_38KANTURU_2ND)
	{
		PlayMp3(g_lpszMp3[MUSIC_KANTURU_2ND]);
	}
	else
	{
		StopMp3(g_lpszMp3[MUSIC_KANTURU_2ND]);
	}
}

CTrapCanon::CTrapCanon()
{
	Initialize();
}
CTrapCanon::~CTrapCanon()
{
	Destroy();	
}

void CTrapCanon::Initialize()
{

}

void CTrapCanon::Destroy()
{

}

void CTrapCanon::Open_TrapCanon()
{
	gLoadData.AccessModel(MODEL_TRAP_CANON, "Data\\Npc\\", "c_mon");
	gLoadData.OpenTexture(MODEL_TRAP_CANON, "Npc\\");
}

CHARACTER* CTrapCanon::Create_TrapCanon(int iPosX, int iPosY, int iKey)
{
	CHARACTER* pCha = NULL;

	pCha = CreateCharacter(iKey, MODEL_TRAP_CANON, iPosX, iPosY);
	pCha->Object.Scale = 1.0f;
	pCha->AttackTime = 0;

	return pCha;
}

void CTrapCanon::Render_Object(OBJECT* o, BMD* b)
{
	b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
	float fLumi = (sinf(WorldTime*0.002f) + 1.f) * 0.5f;
	b->RenderMesh(2, RENDER_TEXTURE|RENDER_BRIGHT, o->Alpha, 2, fLumi, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
}

void CTrapCanon::Render_Object_Visual(CHARACTER* c, OBJECT* o, BMD* b)
{
	if(c->AttackTime == 0)
	{
		float fLumi;
		vec3_t vPos, vLight;
		VectorCopy(o->Position, vPos);
		fLumi = (sinf(WorldTime*0.003f) + 1.f);
		vPos[1] -= 5.0f;
		vPos[2] += 80.f + fLumi*5.f;
		fLumi = (sinf(WorldTime*0.002f) + 1.f) * 0.1f;
		Vector(2.0f+fLumi, 1.0f+fLumi, 1.0f+fLumi, vLight);
		CreateSprite(BITMAP_POUNDING_BALL,vPos,0.7f+fLumi,vLight,o,(WorldTime/10.0f));
		fLumi = (sinf(WorldTime*0.002f) + 1.f) * 1.0f;
		Vector(2.0f+(rand()%10)*0.03f, 0.4f+(rand()%10)*0.03f, 0.4f+(rand()%10)*0.03f, vLight);
		CreateSprite(BITMAP_LIGHT,vPos,2.0f,vLight,o,-(WorldTime*0.1f));
		CreateSprite(BITMAP_LIGHT,vPos,2.0f,vLight,o,(WorldTime*0.12f));
	}
}

void CTrapCanon::Render_AttackEffect(CHARACTER* c, OBJECT* o, BMD* b)
{
	if(c->AttackTime == 1)
	{
		CHARACTER *tc = &CharactersClient[c->TargetCharacter];
		OBJECT *to = &tc->Object;
		vec3_t vPos, vPos2;
		VectorCopy(o->Position, vPos);
		VectorCopy(to->Position, vPos2);
		vPos[2] += 85.f;
		CreateJoint(BITMAP_JOINT_ENERGY, vPos, vPos2, to->Angle, 43, to, 30.0f);
	}
}

