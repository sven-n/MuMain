//*****************************************************************************
// file    : GM_PK_Field.cpp
//*****************************************************************************
#include "stdafx.h"
#include "GM_PK_Field.h"
#include "ZzzBMD.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzAI.h"
#include "ZzzEffect.h"
#include "ZzzOpenData.h"
#include "w_MapHeaders.h"
#include "DSPlaySound.h"

extern char* g_lpszMp3[NUM_MUSIC];

CGM_PK_FieldPtr CGM_PK_Field::Make()
{
	CGM_PK_FieldPtr pkfield( new CGM_PK_Field );
	pkfield->Init();
	return pkfield;
}

CGM_PK_Field::CGM_PK_Field()
{
	Init();
}

CGM_PK_Field::~CGM_PK_Field()
{
	Destroy();
}

void CGM_PK_Field::Init()
{
	//n/a
}

void CGM_PK_Field::Destroy()
{
	//n/a
}

CHARACTER* CGM_PK_Field::CreateMonster(int iType, int PosX, int PosY, int Key)
{
	if(!gMapManager.IsPKField())
	{
		return false;
	}

	CHARACTER* pCharacter = NULL;
	switch (iType)
	{
	case 480:
		{
			OpenMonsterModel(157);
			pCharacter = CreateCharacter(Key, MODEL_MONSTER01+157, PosX, PosY);
			strcpy(pCharacter->ID,"좀비 투사");
			pCharacter->Object.Scale = 1.0f;
			pCharacter->Weapon[0].Type = -1;
			pCharacter->Weapon[1].Type = -1;
			pCharacter->Object.m_iAnimation = 0;
		}
		break;
	case 481:
		{
			OpenMonsterModel(157);
			pCharacter = CreateCharacter(Key, MODEL_MONSTER01+157, PosX, PosY);
			strcpy(pCharacter->ID,"좀비 투사");
			pCharacter->Object.Scale = 1.0f;
			pCharacter->Weapon[0].Type = -1;
			pCharacter->Weapon[1].Type = -1;
			pCharacter->Object.m_iAnimation = 0;
		}
		break;
	case 482:
		{
			OpenMonsterModel(158);
			pCharacter = CreateCharacter(Key, MODEL_MONSTER01+158, PosX, PosY);
			strcpy(pCharacter->ID,"되살아난 검투사");
			pCharacter->Object.Scale = 1.0f;
			pCharacter->Weapon[0].Type = -1;
			pCharacter->Weapon[1].Type = -1;
			pCharacter->Object.m_iAnimation = 0;
		}
		break;
	case 483:
		{
			OpenMonsterModel(158);
			pCharacter = CreateCharacter(Key, MODEL_MONSTER01+158, PosX, PosY);
			strcpy(pCharacter->ID,"되살아난 검투사");
			pCharacter->Object.Scale = 1.0f;
			pCharacter->Weapon[0].Type = -1;
			pCharacter->Weapon[1].Type = -1;
			pCharacter->Object.m_iAnimation = 0;
		}
		break;
	case 484:
		{
			OpenMonsterModel(159);
			pCharacter = CreateCharacter(Key, MODEL_MONSTER01+159, PosX, PosY);
			strcpy(pCharacter->ID,"잿더미 도살자");
			pCharacter->Object.Scale = 0.7f;
			pCharacter->Weapon[0].Type = -1;
			pCharacter->Weapon[1].Type = -1;
			pCharacter->Object.m_iAnimation = 0;
		}
		break;
	case 485:
		{
			OpenMonsterModel(159);
			pCharacter = CreateCharacter(Key, MODEL_MONSTER01+159, PosX, PosY);
			strcpy(pCharacter->ID,"잿더미 도살자");
			pCharacter->Object.Scale = 0.7f;
			pCharacter->Weapon[0].Type = -1;
			pCharacter->Weapon[1].Type = -1;
			pCharacter->Object.m_iAnimation = 0;
		}
		break;
	case 486:
		{
			OpenMonsterModel(160);
			pCharacter = CreateCharacter(Key, MODEL_MONSTER01+160, PosX, PosY);
			strcpy(pCharacter->ID,"피의 암살자");
			pCharacter->Object.Scale = 1.0f;
			pCharacter->Weapon[0].Type = -1;
			pCharacter->Weapon[1].Type = -1;
			pCharacter->Object.m_iAnimation = 0;
			pCharacter->Object.LifeTime = 100;
		}
		break;
	case 487:
		{
			OpenMonsterModel(161);
			pCharacter = CreateCharacter(Key, MODEL_MONSTER01+161, PosX, PosY);
			strcpy(pCharacter->ID,"잔혹한 피의 암살자");
			pCharacter->Object.Scale = 1.0f;
			pCharacter->Weapon[0].Type = -1;
			pCharacter->Weapon[1].Type = -1;
			pCharacter->Object.m_iAnimation = 0;
			pCharacter->Object.LifeTime = 100;
		}
		break;
	case 488:
		{
			OpenMonsterModel(161);
			pCharacter = CreateCharacter(Key, MODEL_MONSTER01+161, PosX, PosY);
			strcpy(pCharacter->ID,"잔혹한 피의 암살자");
			pCharacter->Object.Scale = 1.0f;
			pCharacter->Weapon[0].Type = -1;
			pCharacter->Weapon[1].Type = -1;
			pCharacter->Object.m_iAnimation = 0;
			pCharacter->Object.LifeTime = 100;
		}
		break;
	case 489:
		{
			OpenMonsterModel(163);
			pCharacter = CreateCharacter(Key, MODEL_MONSTER01+163, PosX, PosY);
			strcpy(pCharacter->ID,"불타는 용암 거인");
			pCharacter->Object.Scale = 1.0f;
			pCharacter->Weapon[0].Type = -1;
			pCharacter->Weapon[1].Type = -1;
			pCharacter->Object.m_iAnimation = 0;
		}
		break;
	case 490:
		{
			OpenMonsterModel(162);
			pCharacter = CreateCharacter(Key, MODEL_MONSTER01+162, PosX, PosY);
			strcpy(pCharacter->ID,"포악한 용암 거인");
			pCharacter->Object.Scale = 1.0f;
			pCharacter->Weapon[0].Type = -1;
			pCharacter->Weapon[1].Type = -1;
			pCharacter->Object.m_iAnimation = 0;
		}
		break;
	case 491:
		{
			OpenMonsterModel(162);
			pCharacter = CreateCharacter(Key, MODEL_MONSTER01+162, PosX, PosY);
			strcpy(pCharacter->ID,"포악한 용암 거인");
			pCharacter->Object.Scale = 1.0f;
			pCharacter->Weapon[0].Type = -1;
			pCharacter->Weapon[1].Type = -1;
			pCharacter->Object.m_iAnimation = 0;
		}
		break;
	}
	return pCharacter;
}

void CGM_PK_Field::MoveBlurEffect(CHARACTER* c, OBJECT* o, BMD* b)
{
	if(!gMapManager.IsPKField())
		return;

	if(o->Type >= MODEL_MONSTER01+158 && o->Type <= MODEL_MONSTER01+161)
	{
		float Start_Frame = 0.0f;
		float End_Frame = 0.0f;
		
		switch(o->Type)
		{
		case MODEL_MONSTER01+159:
			{
				Start_Frame = 4.0f;
				End_Frame = 10.0f;
			}
			break;
		case MODEL_MONSTER01+158:
			{
				Start_Frame = 3.0f;
				End_Frame = 7.0f;
			}
			break;
		case MODEL_MONSTER01+160:
		case MODEL_MONSTER01+161:
			{
				Start_Frame = 3.0f;
				End_Frame = 8.0f;
			}
			break;
		default:
			return;
		}
		
		if((o->AnimationFrame >= Start_Frame && o->AnimationFrame <= End_Frame && o->CurrentAction == MONSTER01_ATTACK1)
			|| (o->AnimationFrame >= Start_Frame && o->AnimationFrame <= End_Frame && o->CurrentAction == MONSTER01_ATTACK2))
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
			
				switch(o->Type)
				{
				case MODEL_MONSTER01+159:
					{
						Vector(0.3f, 0.3f, 0.3f, Light);
						b->TransformPosition(BoneTransform[33], StartRelative, StartPos, false);
						b->TransformPosition(BoneTransform[34], EndRelative, EndPos, false);
						CreateBlur(c, StartPos, EndPos, Light, 0);
					}
					break;
				case MODEL_MONSTER01+158:
					{
						Vector(0.0f,0.3f,0.2f, Light);
						b->TransformPosition(BoneTransform[39], StartRelative, StartPos, false);
						b->TransformPosition(BoneTransform[40], EndRelative, EndPos, false);
						CreateBlur(c, StartPos, EndPos, Light, 0);
					}
					break;
				case MODEL_MONSTER01+160:
				case MODEL_MONSTER01+161:
					{
						if(o->Type == MODEL_MONSTER01+160)
						{
							Vector(0.9f, 0.2f, 0.1f, Light);
						}
						else	//o->Type == MODEL_MONSTER01+161
						{
							Vector(0.2f, 0.9f, 0.1f, Light);
						}
						b->TransformPosition(BoneTransform[40], StartRelative, StartPos, false);
						
						b->TransformByBoneMatrix( EndPos, BoneTransform[40] );
						CreateBlur(c, StartPos, EndPos, Light, 5, false, 0);
						
						b->TransformPosition(BoneTransform[40], EndRelative, EndPos, false);
						CreateBlur(c, StartPos, EndPos, Light, 5, false, 1);
						b->TransformPosition(BoneTransform[53], EndRelative, EndPos, false);
						CreateBlur(c, StartPos, EndPos, Light, 5, false, 2);
						b->TransformPosition(BoneTransform[14], StartRelative, StartPos, false);
						b->TransformPosition(BoneTransform[14], EndRelative, EndPos, false);
						CreateBlur(c, StartPos, EndPos, Light, 5, false, 3);
						b->TransformPosition(BoneTransform[27], EndRelative, EndPos, false);
						CreateBlur(c, StartPos, EndPos, Light, 5, false, 4);
					}
					break;

				default:
					break;
				}
				fAnimationFrame += fSpeedPerFrame;
			}
		}
	}
}
bool CGM_PK_Field::CreateObject(OBJECT* o)
{
	if(!gMapManager.IsPKField())
	{
		return false;
	}

	if(o->Type>=0 && o->Type<=6)
	{
		o->CollisionRange = -300;
		return true;
	}

	return false;
}
bool CGM_PK_Field::MoveObject(OBJECT* o)
{
	if(!gMapManager.IsPKField())
	{
		return false;
	}

	switch(o->Type)
	{
	case 0:
	case 1:
	case 2:
	case 3:
	case 4:
	case 5:
	case 6:
		o->HiddenMesh = -2;
		return true;

	default:
		break;
	}

	return false;
}

bool CGM_PK_Field::RenderObjectVisual(OBJECT* o, BMD* b)
{
	if(!gMapManager.IsPKField())
	{
		return false;
	}

	vec3_t Light;

	switch(o->Type)
	{
	case 0:
		{
			o->HiddenMesh = -2;
			float fLumi = ((sinf(WorldTime*0.001f) + 1.f) * 0.5f) * 100.0f;

			int nRanDelay = o->Position[0];
			nRanDelay = nRanDelay%3+1;
			int nRanTemp = 30;
			nRanTemp = nRanTemp * nRanDelay;
			int nRanGap = 10;
			if(nRanTemp != 90.0f)
			{
				nRanGap = 40;
			}

			if(fLumi >= nRanTemp && fLumi <= nRanTemp+nRanGap)
			{
				Vector ( 1.0f, 1.0f, 1.0f, Light );	
				for(int i=0; i<20; ++i)	
				{
					CreateParticle(BITMAP_WATERFALL_2, o->Position, o->Angle, Light, 6, o->Scale, o);
				}
			}
		}
		return true;
	case 1:
		{
			o->HiddenMesh = -2;
			if ( rand()%3==0)
			{
				Vector ( 1.0f, 1.0f, 1.0f, Light );
				CreateParticle(BITMAP_SMOKE, o->Position, o->Angle, Light, 60, o->Scale, o);
			}
		}
		return true;
	case 2:
		{
			o->HiddenMesh = -2;
			vec3_t  Light;
			if(rand()%3 == 0)
			{
				Vector(0.f, 0.f, 0.f, Light);
				CreateParticle (BITMAP_CLOUD, o->Position, o->Angle, Light, 16, o->Scale, o);
			}
		}
		return true;
	case 3:
		{
			o->HiddenMesh = -2;
			if(rand()%4 == 0)
			{
				float fRed = (rand()%3)*0.01f + 0.015f;
				Vector(fRed,0.0f,0.0f,Light);
				CreateParticle(BITMAP_CLOUD, o->Position, o->Angle, Light, 11, o->Scale, o);
			}
		}
		return true;
	case 4:
		{
			o->HiddenMesh = -2;
			Vector (1.0f, 0.4f, 0.4f, Light);
			vec3_t vAngle;
			if(rand()%3 == 0)
			{
				Vector((float)(rand()%40+120),0.f,(float)(rand()%30),vAngle);
				VectorAdd(vAngle, o->Angle, vAngle);
				CreateJoint(BITMAP_JOINT_SPARK,o->Position,o->Position, vAngle, 4, o, o->Scale);
				CreateParticle(BITMAP_SPARK,o->Position,vAngle,Light, 9, o->Scale);
			}
		}
		return true;
	case 5:
		{
			o->HiddenMesh = -2;
			if(rand()%3 == 0)
			{
				Vector ( 0.3f, 0.3f, 0.3f, o->Light );
				CreateParticle( BITMAP_SMOKE, o->Position, o->Angle, o->Light, 21, o->Scale);
			}
		}
		return true;
	case 6:	
		{
			o->HiddenMesh = -2;
			
			vec3_t vLightFire;
			Vector(1.0f, 0.2f, 0.0f, vLightFire);
			CreateSprite(BITMAP_LIGHT, o->Position, 2.0f * o->Scale, vLightFire, o);
			
			vec3_t vLight;
			Vector(1.0f, 1.0f, 1.0f, vLight);
			switch(rand()%3)
			{
			case 0:
				CreateParticle(BITMAP_FIRE_HIK1,o->Position,o->Angle,vLight,0,o->Scale);
				break;
			case 1:
				CreateParticle(BITMAP_FIRE_CURSEDLICH,o->Position,o->Angle,vLight,4,o->Scale);
				break;
			case 2:
				CreateParticle(BITMAP_FIRE_HIK3,o->Position,o->Angle,vLight,0,o->Scale);
				break;
			}
		}
		return true;
	}
	return false;
}

bool CGM_PK_Field::RenderObjectMesh(OBJECT* o, BMD* b, bool ExtraMon)
{
	if(!gMapManager.IsPKField())
	{
		return false;
	}
	
	if(o->Type >= MODEL_MONSTER01+157 && o->Type <= MODEL_MONSTER01+163)
	{
		RenderMonster(o, b, ExtraMon);
		
		return true;
	}
 	else if(o->Type == 15)
 	{
 		b->StreamMesh = 0;
 		b->RenderBody(RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,-(int)WorldTime%10000*0.0001f);
 		b->StreamMesh = -1;

		return true;
 	}
	else if(o->Type == 67)
	{
		b->StreamMesh = 1;
		b->RenderBody(RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU, (int)WorldTime%10000*0.0001f);
		b->StreamMesh = -1;
 		vec3_t light;
 		Vector(1.0f,0.0f,0.0f,light);
 		b->RenderMesh(0, RENDER_TEXTURE, o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
 		VectorCopy(light, b->BodyLight);
		b->RenderMesh(0, RENDER_BRIGHT|RENDER_CHROME, 0.2f,0,0.2f,o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);

		vec3_t vLightFire, Position, vPos;
		Vector(1.0f, 0.0f, 0.0f, vLightFire);
		Vector(0.0f, 0.0f, 0.0f, vPos);

		b->TransformPosition(BoneTransform[6],vPos,Position,false);
		CreateSprite(BITMAP_LIGHT, Position, o->Scale*5.0f, vLightFire, o);

		Vector(0.0f, 0.0f, -350.0f, vPos);
		b->TransformPosition(BoneTransform[6],vPos,Position,false);
		CreateSprite(BITMAP_LIGHT, Position, o->Scale*5.0f, vLightFire, o);

		if(o->AnimationFrame >= 35 && o->AnimationFrame <= 37)
		{
			o->PKKey = -1;
		}

		if(o->AnimationFrame >= 1 && o->AnimationFrame <= 2  && o->PKKey != 1)
		{ 
			o->AnimationFrame = 1;
			
			int test = rand()%1000;
			if(test >= 0 && test < 2)
			{
				o->PKKey = 1;
			}
			else
			{
				o->PKKey = -1;
			}
		}
		vec3_t p, Pos, Light;
		Vector(0.4f, 0.1f, 0.1f, Light);
		//Vector(rand()%20-30.0f, rand()%20-30.0f, 0.0f, p);
		Vector(-150.0f, 0.0f, 0.0f, p);
		b->TransformPosition(BoneTransform[4],p,Pos,false);
		if(o->AnimationFrame >= 35.0f && o->AnimationFrame < 50.0f)
			CreateParticle(BITMAP_SMOKE,Pos,o->Angle,Light, 63, o->Scale*1.5f);

		return true;
	}
	else if(o->Type == 68)
	{
		b->StreamMesh = 1;
		b->RenderBody(RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU, (int)WorldTime%10000*0.0001f);
		b->StreamMesh = -1;
		
		vec3_t light;
		Vector(1.0f,0.0f,0.0f,light);
		b->RenderMesh(0, RENDER_TEXTURE, o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);
		VectorCopy(light, b->BodyLight);
		b->RenderMesh(0, RENDER_BRIGHT|RENDER_CHROME, 0.2f,0,0.2f,o->BlendMeshTexCoordU, o->BlendMeshTexCoordV);

		vec3_t vLightFire, Position, vPos;
		Vector(1.0f, 0.0f, 0.0f, vLightFire);
		Vector(0.0f, 0.0f, 0.0f, vPos);

		b->TransformPosition(BoneTransform[6],vPos,Position,false);
		CreateSprite(BITMAP_LIGHT, Position, o->Scale*5.0f, vLightFire, o);

		Vector(0.0f, 0.0f, -350.0f, vPos);
		b->TransformPosition(BoneTransform[6],vPos,Position,false);
		CreateSprite(BITMAP_LIGHT, Position, o->Scale*5.0f, vLightFire, o);
		
		vec3_t p, Pos, Light;
		//Vector(0.08f, 0.08f, 0.08f, Light);
		Vector(0.3f, 0.1f, 0.1f, Light);
		Vector(rand()%20-30.0f, rand()%20-30.0f, 0.0f, p);
		b->TransformPosition(BoneTransform[4],p,Pos,false);
		if(o->AnimationFrame >= 7.0f && o->AnimationFrame < 13.0f)
			CreateParticle(BITMAP_SMOKE,Pos,o->Angle,Light, 18, o->Scale*1.5f);

		return true;
	}
	return false;
}

void CGM_PK_Field::RenderAfterObjectMesh(OBJECT* o, BMD* b, bool ExtraMon)
{
	if(!gMapManager.IsPKField())
		return;

	switch(o->Type)
	{
	case 16:		//song_lava2 fade in-out
		{
			float fLumi = (sinf(WorldTime*0.002f) + 1.f) * 0.5f;
			b->RenderBody(RENDER_TEXTURE,o->Alpha,0,fLumi,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
		}
		break;
	default:
		break;
	}
}

bool CGM_PK_Field::MoveMonsterVisual(OBJECT* o, BMD* b)
{
	if(!gMapManager.IsPKField())
		return false;
	
	switch(o->Type)
	{
	case MODEL_MONSTER01+160:
	case MODEL_MONSTER01+161:
		{
			if(o->CurrentAction == MONSTER01_DIE)
			{	
				int iBones[] = {5, 6, 7};		// Neck/ Head/ HeadNub
				vec3_t vLight, vPos, vRelative;
				Vector(1.0f, 1.0f, 1.0f, vLight);
				vec3_t vLightFire;
				if(o->Type == MODEL_MONSTER01+160)
				{
					Vector(1.0f, 0.2f, 0.0f, vLightFire);
				}
				else
				{
					Vector(0.2f, 1.0f, 0.0f, vLightFire);
				}
				for (int i = 0; i<3; ++i)
				{
					float fScale = 1.2f;
					if(i >= 1)
					{
						b->TransformByObjectBone(vPos, o, iBones[i]);
						CreateSprite(BITMAP_LIGHT, vPos, 1.0f, vLightFire, o);
						
						fScale = 0.7f;
						Vector((rand()%10-5)*1.0f, (rand()%10-5)*1.0f, (rand()%10-5)*1.0f, vRelative);
						b->TransformByObjectBone(vPos, o, iBones[i], vRelative);	
					}
					else
					{
						b->TransformByObjectBone(vPos, o, iBones[i]);
						vPos[2] += 50.0f;
						CreateSprite(BITMAP_LIGHT, vPos, 2.5f, vLightFire, o);
						
						Vector((rand()%20-10)*1.0f, (rand()%20-10)*1.0f, (rand()%20-10)*1.0f, vRelative);
						b->TransformByObjectBone(vPos, o, iBones[i], vRelative);
					}
					if(o->Type == MODEL_MONSTER01+160)
					{
						for (int i = 0; i < 2; ++i)
						{
							float fScale = (rand()%5+18)*0.03f;
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
						}
					}
					else		//o->Type == MODEL_MONSTER01+161
					{
						for (int i = 0; i < 2; ++i)
						{
							float fScale = (rand()%5+18)*0.03f;
							Vector(0.6f,0.9f,0.1f,o->Light);
							switch(rand()%3)
							{
							case 0:
								CreateParticle(BITMAP_FIRE_HIK1_MONO,vPos,o->Angle,o->Light,0,fScale);
								break;
							case 1:
								CreateParticle(BITMAP_FIRE_HIK2_MONO,vPos,o->Angle,o->Light,4,fScale);
								break;
							case 2:
								CreateParticle(BITMAP_FIRE_HIK3_MONO,vPos,o->Angle,o->Light,0,fScale);
								break;
							}
						}
					}
				}
			}
		}
		return true;
	}
	return false;
}

bool CGM_PK_Field::RenderMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b)
{
	if(!gMapManager.IsPKField())
	{
		return false;
	}

	vec3_t vPos, vLight;

	switch(o->Type)
	{
	case MODEL_MONSTER01+157:
		{

		}
		return true;
	case MODEL_MONSTER01+158:
		{
			
		}
		return true;
	case MODEL_MONSTER01+159:
		{
			vec3_t p,Position;
			if(rand()%4==0)
			{
				Vector(0.0f,50.0f,0.0f,p);
				b->TransformPosition(o->BoneTransform[6],p,Position,true);
				
				Vector(1.0f,1.0f,1.0f,o->Light);
				CreateParticle(BITMAP_SMOKE, Position, o->Angle, o->Light, 61);
			}
		}
		return true;
	case MODEL_MONSTER01+160:
	case MODEL_MONSTER01+161:
		{
			int iBones[] = {37, 11, 70, 65, 6};
			
			switch(o->Type)
			{
			case MODEL_MONSTER01+160:
				Vector(0.9f, 0.2f, 0.1f, vLight);	//red
				break;
			case MODEL_MONSTER01+161:
				Vector(0.3f, 0.9f, 0.2f, vLight);	//green
				break;
			}
			if(o->CurrentAction != MONSTER01_DIE)
			{
				for (int i=0; i<4; ++i)
				{
					if (rand()%4 > 0) continue;
					
					b->TransformByObjectBone(vPos, o, iBones[i]);
					CreateParticle(BITMAP_SMOKE,				vPos,o->Angle,vLight,50,1.0f);
					CreateParticle(BITMAP_SMOKELINE1+rand()%3,	vPos,o->Angle,vLight,0, 0.01f);
				}
				
				if(o->CurrentAction == MONSTER01_ATTACK1)
				{
					vec3_t TempPos;
					Vector(0.0f,0.0f,0.0f,TempPos);
					
					b->TransformByObjectBone(vPos, o, iBones[4]);
					
					TempPos[1] = (Hero->Object.Position[1] - vPos[1])*0.5f;
					TempPos[0] = (Hero->Object.Position[0] - vPos[0])*0.5f;
					TempPos[2] = 0.0f;

					VectorNormalize(TempPos);
					VectorScale(TempPos, 50.0f, TempPos);

					VectorAdd(vPos, TempPos, vPos);
					
					CreateParticle(BITMAP_LIGHT+2, vPos, o->Angle, vLight, 7, 0.5f);

					switch(o->Type)
					{
					case MODEL_MONSTER01+160:
						Vector(0.9f, 0.4f, 0.1f, vLight);	//red
						break;
					case MODEL_MONSTER01+161:
						Vector(0.6f, 0.9f, 0.2f, vLight);	//green
						break;
					}
					CreateParticle(BITMAP_SPARK+1, vPos, o->Angle, vLight, 29, 1.0f);
				}
			}
			else					//o->CurrentAction == MONSTER01_DIE
			{
				if (o->LifeTime == 100)
				{
					o->LifeTime = 90;
					o->m_bRenderShadow = false;
					
					vec3_t vRelativePos, vWorldPos, Light;
					Vector(1.0f,1.0f,1.0f,Light);
					
					Vector(0.f, 0.f, 0.f, vRelativePos);
					
					b->TransformPosition(o->BoneTransform[5], vRelativePos, vWorldPos, true);
					switch(o->Type)
					{
					case MODEL_MONSTER01+160:
						CreateEffect(MODEL_PKFIELD_ASSASSIN_EFFECT_RED_HEAD,vWorldPos,o->Angle,Light,0,o,0,0);
						break;

					case MODEL_MONSTER01+161:
						CreateEffect(MODEL_PKFIELD_ASSASSIN_EFFECT_GREEN_HEAD,vWorldPos,o->Angle,Light,0,o,0,0);
						break;
					}
				}	
			}
		}
		return true;
	case MODEL_MONSTER01+162:
	case MODEL_MONSTER01+163:
		{
			float fRotation = (float)((int)(WorldTime*0.1f)%360);
			float fAngle = (sinf(WorldTime*0.003f) + 1.0f)*0.4f+1.5f;
			vec3_t vWorldPos, vLight;
			switch(o->Type)
			{
			case MODEL_MONSTER01+162:
				{
					Vector(0.5f,0.1f,0.9f,vLight);
				}
				break;
			case MODEL_MONSTER01+163:
				{
					Vector(0.9f,0.4f,0.1f,vLight);
				}
				break;
			}

			b->TransformByObjectBone(vWorldPos, o, 3);
			CreateSprite(BITMAP_LIGHTMARKS, vWorldPos, fAngle, vLight, o, fRotation);
			fAngle = (sinf(WorldTime*0.003f) + 1.0f)*0.4f+0.5f;
			b->TransformByObjectBone(vWorldPos, o, 37);
			CreateSprite(BITMAP_LIGHTMARKS, vWorldPos, fAngle, vLight, o, fRotation);
			b->TransformByObjectBone(vWorldPos, o, 43);
			CreateSprite(BITMAP_LIGHTMARKS, vWorldPos, fAngle, vLight, o, fRotation);

			vec3_t vRelative, vPos;
			Vector(1.0f,1.0f,1.0f,vLight);
			int iModel = (o->Type == MODEL_MONSTER01+163) ? MODEL_LAVAGIANT_FOOTPRINT_R : MODEL_LAVAGIANT_FOOTPRINT_V;

			if(o->CurrentAction == MONSTER01_WALK)
			{
				if(o->AnimationFrame >= 2.0f && o->AnimationFrame <= 2.4f)
				{
					Vector(0.0f, 0.0f, 0.0f, vPos);
					b->TransformPosition (o->BoneTransform[36], vPos, vRelative, true);
					CreateEffect(iModel, vRelative, o->Angle, vLight, 0, o, -1, 0, 0, 0 ,1.3f);
					CreateParticle(BITMAP_SMOKE, vRelative, o->Angle, vLight, 62, 1.0f);
				}
				else if(o->AnimationFrame >= 7.0f && o->AnimationFrame <= 7.4f)
				{
					Vector(0.0f, 0.0f, 0.0f, vPos);
					b->TransformPosition (o->BoneTransform[42], vPos, vRelative, true);
					CreateEffect(iModel, vRelative, o->Angle, vLight, 0, o, -1, 0, 0, 0 ,1.3f);
					CreateParticle(BITMAP_SMOKE, vRelative, o->Angle, vLight, 62, 1.0f);
				}
			}
			else if(o->CurrentAction == MONSTER01_ATTACK2)
			{
				if(o->AnimationFrame >= 6.0f && o->AnimationFrame <= 6.4f)
				{
					Vector(0.0f, 0.0f, 0.0f, vPos);
					b->TransformPosition (o->BoneTransform[42], vPos, vRelative, true);
					CreateEffect(iModel, vRelative, o->Angle, vLight, 0, o, -1, 0, 0, 0 ,1.6f);
					CreateParticle(BITMAP_SMOKE, vRelative, o->Angle, vLight, 62, 4.0f);
				}
			}
	
			vec3_t p,Position;
			if(rand()%4==0)
			{
				Vector(0.0f,50.0f,0.0f,p);
				b->TransformPosition(o->BoneTransform[7],p,Position,true);
				switch(o->Type)
				{
				case MODEL_MONSTER01+162:
					Vector(0.5f,0.1f,0.9f,o->Light);
					break;
				case MODEL_MONSTER01+163:
					Vector(0.9f,0.4f,0.1f,o->Light);
					break;
				}
				CreateParticle(BITMAP_SMOKE,Position,o->Angle,o->Light,61);
			}
		}
		return true;
	}
	return false;
}

bool CGM_PK_Field::RenderMonster(OBJECT* o, BMD* b, bool ExtraMon)
{
	if(!gMapManager.IsPKField())
	{
		return false;
	}

	float fRotation = (float)((int)(WorldTime*0.1f)%360);
	float fAngle = (sinf(WorldTime*0.004f) + 1.0f)*0.4f + 0.2f;
	vec3_t vWorldPos, vLight;
	Vector(0.1f,0.4f,0.5f,vLight);

	switch(o->Type)
	{
	case MODEL_MONSTER01+157:
		{	
			b->TransformByObjectBone(vWorldPos, o, 9);
			CreateSprite(BITMAP_LIGHTMARKS, vWorldPos, fAngle, vLight, o, fRotation);
			
			b->RenderMesh(1,RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV);
			b->RenderMesh(2,RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV);
			DisableDepthTest();
			b->RenderMesh(0,RENDER_TEXTURE|RENDER_BRIGHT,o->Alpha,0,o->BlendMeshLight*fAngle,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV);
			EnableDepthTest();
		}
		return true;
	case MODEL_MONSTER01+158:
		{	
			b->TransformByObjectBone(vWorldPos, o, 9);
			CreateSprite(BITMAP_LIGHTMARKS, vWorldPos, fAngle, vLight, o, fRotation);
			
			b->RenderMesh(1,RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV);
			b->RenderMesh(2,RENDER_TEXTURE,o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV);
			DisableDepthTest();
			b->RenderMesh(0,RENDER_TEXTURE|RENDER_BRIGHT,o->Alpha,0,o->BlendMeshLight*fAngle,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV);
			EnableDepthTest();
		}
		return true;
	case MODEL_MONSTER01+159:
		{
			float fBlendMeshLight =0.0f;
			fBlendMeshLight = (sinf(WorldTime*0.003f)+1.0f)*0.5f;

			b->RenderMesh(1,RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV);
			b->RenderMesh(2,RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV);

			b->RenderMesh(0,RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV);
			b->RenderMesh(0,RENDER_TEXTURE|RENDER_BRIGHT, o->Alpha, 0, fBlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV, BITMAP_BUGBEAR_R);
		}
		return true;
	case MODEL_MONSTER01+160:
	case MODEL_MONSTER01+161:
		{
			if(o->CurrentAction == MONSTER01_DIE)
			{
				if (o->LifeTime == 100)
				{
					switch(o->Type)
					{
					case MODEL_MONSTER01+160:
						{
							CreateEffect(MODEL_PKFIELD_ASSASSIN_EFFECT_RED_BODY,o->Position,o->Angle,o->Light,0,o,0,0);
						}
						break;
					case MODEL_MONSTER01+161:
						{
							CreateEffect(MODEL_PKFIELD_ASSASSIN_EFFECT_GREEN_BODY,o->Position,o->Angle,o->Light,0,o,0,0);
						}
						break;
					}
				}
			}
			else
			{
				float fBlendMeshLight =0.0f;
				fBlendMeshLight = (sinf(WorldTime*0.005f)+1.0f)*0.3f+0.3f;

				b->RenderMesh(0,RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV);
				
				switch(o->Type)
				{
				case MODEL_MONSTER01+160:
					{
						b->RenderMesh(1,RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV);
						b->RenderMesh(1,RENDER_TEXTURE, o->Alpha, 1,fBlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV, BITMAP_PKMON02);
						
						b->RenderMesh(3,RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV);
						b->RenderMesh(3,RENDER_TEXTURE, o->Alpha, 3,fBlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV, BITMAP_PKMON01);
					}
					break;
				case MODEL_MONSTER01+161:
					{
						b->RenderMesh(1,RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV);
						b->RenderMesh(1,RENDER_TEXTURE, o->Alpha, 1,fBlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV, BITMAP_PKMON04);
						
						b->RenderMesh(3,RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV);
						b->RenderMesh(3,RENDER_TEXTURE, o->Alpha, 3,fBlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV, BITMAP_PKMON03);
					}
					break;
				}
				Vector(b->BodyLight[0]*0.65f, b->BodyLight[0]*0.65f, b->BodyLight[0]*0.65f, b->BodyLight);
				b->RenderMesh(2,RENDER_TEXTURE|RENDER_BRIGHT, o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV);

				Vector(b->BodyLight[0]*2.0f, b->BodyLight[0]*1.0f, b->BodyLight[0]*0.4f, b->BodyLight);
				b->RenderMesh(2,RENDER_BRIGHT|RENDER_CHROME, o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV );
			}
		}
		return true;
	case MODEL_MONSTER01+162:
	case MODEL_MONSTER01+163:
		{
			b->RenderMesh(1,RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV);
			b->RenderMesh(2,RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV);
			
			float fBlendMeshLight = 0.0f;
			fBlendMeshLight = (sinf(WorldTime*0.001f));

			switch(o->Type)
			{
			case MODEL_MONSTER01+162:
			case MODEL_MONSTER01+163:
				{
					float fAlpha = 1.0f;
					b->RenderMesh(0,RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV);

					int iTexture = (o->Type == MODEL_MONSTER01+162) ? BITMAP_PKMON06 : BITMAP_PKMON05;
		
					if(fBlendMeshLight < 0)
					{
						//fBlendMeshLight = fabs(fBlendMeshLight);
						fBlendMeshLight = -(fBlendMeshLight);
						//b->RenderMesh(0,RENDER_TEXTURE|RENDER_DARK, fAlpha, 0,fBlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV, iTexture);
						b->RenderMesh(0,RENDER_TEXTURE, fAlpha, 0,fBlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV, iTexture);
					}
					else
					{
						for(int i=0; i<3; ++i)
							b->RenderMesh(0,RENDER_TEXTURE|RENDER_BRIGHT, fAlpha, 0,fBlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV, iTexture);
					}
				}
				break;
			}
		}
		return true;
	case MODEL_PKFIELD_ASSASSIN_EFFECT_GREEN_HEAD:
	case MODEL_PKFIELD_ASSASSIN_EFFECT_RED_HEAD:
		{
			b->RenderBody(RENDER_TEXTURE, o->Alpha,o->BlendMesh,o->BlendMeshLight,o->BlendMeshTexCoordU,o->BlendMeshTexCoordV,o->HiddenMesh);
		}
		return true;
	case MODEL_PKFIELD_ASSASSIN_EFFECT_GREEN_BODY:
	case MODEL_PKFIELD_ASSASSIN_EFFECT_RED_BODY:
		{
			b->RenderBody(RENDER_TEXTURE, o->Alpha, o->BlendMesh, o->BlendMeshLight, o->BlendMeshTexCoordU, o->BlendMeshTexCoordV, o->HiddenMesh);
		}
		return true;
	}
	return false;
}

bool CGM_PK_Field::CreateFireSpark(PARTICLE* o)
{
	if(!gMapManager.IsPKField())
	{
		return false;
	}

	o->Type = BITMAP_FIRE_SNUFF;
	o->Scale = rand()%50/100.f+0.4f;
	vec3_t Position;
	Vector ( Hero->Object.Position[0]+(float)(rand()%1600-800), Hero->Object.Position[1]+(float)(rand()%1400-500), Hero->Object.Position[2]+(float)(rand()%300+50), Position );
	
	VectorCopy ( Position, o->Position );
	VectorCopy ( Position, o->StartPosition );
	o->Velocity[0] = -(float)(rand()%64+64)*0.1f;
	if ( Position[1]<CameraPosition[1]+400.f )
	{
		o->Velocity[0] = -o->Velocity[0]+2.2f;
	}
	o->Velocity[1] = (float)(rand()%32-16)*0.1f;
	o->Velocity[2] = (float)(rand()%32-16)*0.1f;
	o->TurningForce[0] = (float)(rand()%16-8)*0.1f;
	o->TurningForce[1] = (float)(rand()%64-32)*0.1f;
	o->TurningForce[2] = (float)(rand()%16-8)*0.1f;
	
	Vector ( 1.f, 0.f, 0.f, o->Light );

	return true;
}

bool CGM_PK_Field::PlayMonsterSound(OBJECT* o)
{
	if(!gMapManager.IsPKField())
		return false;
	
	float fDis_x, fDis_y;
	fDis_x = o->Position[0] - Hero->Object.Position[0];
	fDis_y = o->Position[1] - Hero->Object.Position[1];
	float fDistance = sqrtf(fDis_x*fDis_x+fDis_y*fDis_y);
	
	if(fDistance > 500.0f) 
		return true;

	switch(o->Type)
	{
	case MODEL_MONSTER01+157:
		{
			if(MONSTER01_ATTACK1 == o->CurrentAction || MONSTER01_ATTACK2 == o->CurrentAction)
			{
				PlayBuffer(SOUND_PKFIELD_ZOMBIEWARRIOR_ATTACK);
			}
			else if(MONSTER01_SHOCK == o->CurrentAction)
			{
				PlayBuffer(SOUND_PKFIELD_ZOMBIEWARRIOR_DAMAGE01);
			}
			else if(MONSTER01_WALK == o->CurrentAction)
			{
				if(rand()%20 == 0)
				{
					PlayBuffer(SOUND_PKFIELD_ZOMBIEWARRIOR_MOVE01);
				}
			}
			else if(MONSTER01_DIE == o->CurrentAction)
			{
				PlayBuffer(SOUND_PKFIELD_ZOMBIEWARRIOR_DEATH);
			}

//	SOUND_PKFIELD_ZOMBIEWARRIOR_DAMAGE02,
//	SOUND_PKFIELD_ZOMBIEWARRIOR_MOVE02,
		}
		return true;
	case MODEL_MONSTER01+158:
		{
			if(MONSTER01_ATTACK1 == o->CurrentAction || MONSTER01_ATTACK2 == o->CurrentAction)
			{
				PlayBuffer(SOUND_PKFIELD_RAISEDGLADIATOR_ATTACK);
			}
			else if(MONSTER01_SHOCK == o->CurrentAction)
			{
				PlayBuffer(SOUND_PKFIELD_RAISEDGLADIATOR_DAMAGE01);
			}
			else if(MONSTER01_WALK == o->CurrentAction)
			{
				if(rand()%20 == 0)
				{
					PlayBuffer(SOUND_PKFIELD_RAISEDGLADIATOR_MOVE01);
				}
			}
			else if(MONSTER01_DIE == o->CurrentAction)
			{
				PlayBuffer(SOUND_PKFIELD_RAISEDGLADIATOR_DEATH);
			}

		//	SOUND_PKFIELD_RAISEDGLADIATOR_DAMAGE02,
		//	SOUND_PKFIELD_RAISEDGLADIATOR_MOVE02,		
		}
		return true;
	case MODEL_MONSTER01+159:
		{
			if(MONSTER01_ATTACK1 == o->CurrentAction || MONSTER01_ATTACK2 == o->CurrentAction)
			{
				PlayBuffer(SOUND_PKFIELD_ASHESBUTCHER_ATTACK);
			}
			else if(MONSTER01_SHOCK == o->CurrentAction)
			{
				PlayBuffer(SOUND_PKFIELD_ASHESBUTCHER_DAMAGE01);
			}
			else if(MONSTER01_WALK == o->CurrentAction)
			{
				if(rand()%20 == 0)
				{
					PlayBuffer(SOUND_PKFIELD_ASHESBUTCHER_MOVE01);
				}
			}
			else if(MONSTER01_DIE == o->CurrentAction)
			{
				PlayBuffer(SOUND_PKFIELD_ASHESBUTCHER_DEATH);
			}

	//SOUND_PKFIELD_ASHESBUTCHER_DAMAGE02,	
	//SOUND_PKFIELD_ASHESBUTCHER_MOVE02,
		}
		return true;
	case MODEL_MONSTER01+160:
	case MODEL_MONSTER01+161:
		{
			if(MONSTER01_ATTACK1 == o->CurrentAction || MONSTER01_ATTACK2 == o->CurrentAction)
			{
				PlayBuffer(SOUND_PKFIELD_BLOODASSASSIN_ATTACK);
			}
			else if(MONSTER01_SHOCK == o->CurrentAction)
			{
				PlayBuffer(SOUND_PKFIELD_BLOODASSASSIN_DAMAGE01);
			}
			else if(MONSTER01_WALK == o->CurrentAction)
			{
				if(rand()%20 == 0)
				{
					PlayBuffer(SOUND_PKFIELD_BLOODASSASSIN_MOVE01);
				}
			}
			else if(MONSTER01_DIE == o->CurrentAction)
			{
				PlayBuffer(SOUND_PKFIELD_BLOODASSASSIN_DEDTH);
			}

		//	SOUND_PKFIELD_BLOODASSASSIN_DAMAGE02,
		//	SOUND_PKFIELD_BLOODASSASSIN_MOVE01,
		}
		return true;
	case MODEL_MONSTER01+162:
	case MODEL_MONSTER01+163:
		{		
			if(MONSTER01_ATTACK1 == o->CurrentAction)
			{
				PlayBuffer(SOUND_PKFIELD_BURNINGLAVAGOLEM_ATTACK01);
			}
			else if(MONSTER01_ATTACK2 == o->CurrentAction)
			{
				PlayBuffer(SOUND_PKFIELD_BURNINGLAVAGOLEM_ATTACK02);
			}
			else if(MONSTER01_SHOCK == o->CurrentAction)
			{
				PlayBuffer(SOUND_PKFIELD_BURNINGLAVAGOLEM_DAMAGE01);
			}
			else if(MONSTER01_WALK == o->CurrentAction)
			{
				if(rand()%20 == 0)
				{
					PlayBuffer(SOUND_PKFIELD_BURNINGLAVAGOLEM_MOVE01);
				}
			}
			else if(MONSTER01_DIE == o->CurrentAction)
			{
				PlayBuffer(SOUND_PKFIELD_BURNINGLAVAGOLEM_DEATH);
			}

	//SOUND_PKFIELD_BURNINGLAVAGOLEM_DAMAGE02,
	//SOUND_PKFIELD_BURNINGLAVAGOLEM_MOVE02,
		}
		return true;
	}

	return false;
}

void CGM_PK_Field::PlayObjectSound(OBJECT* o)
{

}

void CGM_PK_Field::PlayBGM()
{
	if(gMapManager.IsPKField())
	{
		PlayMp3(g_lpszMp3[MUSIC_PKFIELD]);
	}
	else
	{
		StopMp3(g_lpszMp3[MUSIC_PKFIELD]);
	}
}
