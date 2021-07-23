// GMDuelArena.cpp: implementation of the CGMDuelArena class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#ifdef YDG_ADD_MAP_DUEL_ARENA

#include "GMDuelArena.h"
#include "ZzzBMD.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzAI.h"
#include "ZzzEffect.h"
#include "w_MapHeaders.h"
#include "DSPlaySound.h"
#include "ZzzOpenData.h"
#include "ZzzLodTerrain.h"

extern char* g_lpszMp3[NUM_MUSIC];

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGMDuelArenaPtr CGMDuelArena::Make()
{
	CGMDuelArenaPtr duelarena(new CGMDuelArena);
	duelarena->Init();
	return duelarena;
}

CGMDuelArena::CGMDuelArena()
{

}

CGMDuelArena::~CGMDuelArena()
{
	Destroy();
}

void CGMDuelArena::Init()
{

}

void CGMDuelArena::Destroy()
{

}


bool CGMDuelArena::CreateObject(OBJECT* o)
{
	switch(o->Type)
	{
	case 0: case 1: case 32:
		o->CollisionRange = -300;
		return true;
	}

	return false;
}

CHARACTER* CGMDuelArena::CreateMonster(int iType, int PosX, int PosY, int Key)
{
	CHARACTER* pCharacter = NULL;

// 	switch (iType)
// 	{
// 	}

	return pCharacter;
}

bool CGMDuelArena::MoveObject(OBJECT* o)
{
	if(IsDuelArena() == false)
		return false;

	switch(o->Type)
	{
	case 35:	// 연기박스
	case 36:	// 불박스
		{
			o->HiddenMesh = -2;
		}
		break;
	case 34:	// 주변광박스
		{
			float Luminosity = (float)(rand()%3+5)*0.1f;
			vec3_t Light;
			Vector(Luminosity*0.9f,Luminosity*0.2f,Luminosity*0.1f,Light);
			AddTerrainLight(o->Position[0], o->Position[1],Light,3,PrimaryTerrainLight);
			o->HiddenMesh = -2;
		}
		break;
	}

	return false;
}

bool CGMDuelArena::MoveMonsterVisual(OBJECT* o, BMD* b)
{
	if(IsDuelArena() == false)
		return false;
	
// 	switch(o->Type)
// 	{
// 	}

	return false;
}

void CGMDuelArena::MoveBlurEffect(CHARACTER* c, OBJECT* o, BMD* b)
{
// 	switch(o->Type)
// 	{
// 	}
}

bool CGMDuelArena::RenderObjectMesh(OBJECT* o, BMD* b, bool ExtraMon)
{
	if(IsDuelArena() == false)
		return false;

// 	switch(o->Type)
// 	{
// 	}
	
	return false;
}

bool CGMDuelArena::RenderObjectVisual( OBJECT* o, BMD* b )
{
	switch(o->Type)
	{
	case 34:	// 주변광박스
		break;
	case 35:	// 연기박스
		{
			vec3_t vLight;
			if ( rand()%3==0) 
			{
				Vector ( 1.f, 1.f, 1.f, vLight );
				CreateParticle ( BITMAP_SMOKE, o->Position, o->Angle, vLight, 14, o->Scale, o);
			}
		}
		break;
	case 36:	// 불박스
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
		break;
	}
	
	return false;
}

bool CGMDuelArena::RenderMonsterVisual(CHARACTER* c, OBJECT* o, BMD* b)
{
// 	vec3_t vPos, vRelative, vLight;
// 	
// 	switch(o->Type)
// 	{
// 	}

	return true;
}

void CGMDuelArena::RenderAfterObjectMesh(OBJECT* o, BMD* b, bool ExtraMon)
{

}

bool IsDuelArena()
{
	if(World == WD_64DUELARENA)
	{
		return true;
	}

	return false;
}

bool CGMDuelArena::SetCurrentActionMonster(CHARACTER* c, OBJECT* o)
{
	if(IsDuelArena() == false)
		return false;
	
// 	switch(c->MonsterIndex) 
// 	{
// 	}
	
	return false;
}

bool CGMDuelArena::AttackEffectMonster(CHARACTER* c, OBJECT* o, BMD* b)
{
	if(IsDuelArena() == false)
		return false;

// 	switch(c->MonsterIndex) 
// 	{
// 	}

	return false;
}

// 몬스터 사운드
bool CGMDuelArena::PlayMonsterSound(OBJECT* o) 
{
	if(IsDuelArena() == false)
		return false;
	
// 	float fDis_x, fDis_y;
// 	fDis_x = o->Position[0] - Hero->Object.Position[0];
// 	fDis_y = o->Position[1] - Hero->Object.Position[1];
// 	float fDistance = sqrtf(fDis_x*fDis_x+fDis_y*fDis_y);
// 	
// 	if (fDistance > 500.0f) 
// 		return true;
// 
// 	switch(o->Type)
// 	{
// 	}

	return false; 
}

// 오브젝트 사운드
void CGMDuelArena::PlayObjectSound(OBJECT* o)
{

}

void CGMDuelArena::PlayBGM()
{
	if (IsDuelArena())
	{
		PlayMp3(g_lpszMp3[MUSIC_DUEL_ARENA]);
	}
}
#endif	// YDG_ADD_MAP_DUEL_ARENA