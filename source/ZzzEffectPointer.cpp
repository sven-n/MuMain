///////////////////////////////////////////////////////////////////////////////
// 3D 특수효과 관련 함수
//
// *** 함수 레벨: 3
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ZzzOpenglUtil.h"
#include "ZzzBMD.h"
#include "ZzzInfomation.h"
#include "ZzzObject.h"
#include "ZzzCharacter.h"
#include "ZzzLodTerrain.h"
#include "ZzzTexture.h"
#include "ZzzAi.h"
#include "ZzzEffect.h"
#include "DSPlaySound.h"
#include "WSClient.h"

PARTICLE  Pointers	[MAX_POINTERS];


///////////////////////////////////////////////////////////////////////////////
// 바닥에 찍히는 효과 처리(피 표현할때 쓰임)
///////////////////////////////////////////////////////////////////////////////

void CreatePointer(int Type,vec3_t Position,float Angle,vec3_t Light,float Scale)
{
#ifdef KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	PARTICLE* Select = NULL;
#else // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	PARTICLE *Select;
#endif // KWAK_FIX_COMPILE_LEVEL4_WARNING_EX
	int MinLifeTime = 9999;
	for(int i=0;i<MAX_POINTERS;i++)
	{
		PARTICLE *o = &Pointers[i];
		if(!o->Live)
		{
			o->Live = true;
			o->Type = Type;
			VectorCopy(Position,o->Position);
			VectorCopy(Light,o->Light);
			o->Angle[2] = Angle;
			o->Alpha = 1.f;
			o->Scale = Scale;
			switch(Type)
			{
			case BITMAP_BLOOD:
			case BITMAP_BLOOD+1:
			case BITMAP_FOOT:
				o->LifeTime = 50+rand()%32;
				break;
			}
			return;
		}
		else
		{
			if(MinLifeTime > o->LifeTime)
			{
				MinLifeTime = o->LifeTime;
				Select = o;
			}
		}
	}
	Select->Live = false;
}

void MovePointers()
{
	for(int i=0;i<MAX_POINTERS;i++)
	{
		PARTICLE *o = &Pointers[i];
		if(o->Live)
		{
			o->LifeTime--;
			switch(o->Type)
			{
			case BITMAP_CURSOR+5:
				o->Scale -= 0.05f;
				if(o->Scale < 0.1f) o->Live = false;
				break;
			case BITMAP_BLOOD:
			case BITMAP_BLOOD+1:
			case BITMAP_FOOT:
				if(o->Type==BITMAP_BLOOD)
				{
					o->Scale += 0.004f;
				}
#ifndef FOR_ADULT
      			Vector(0.1f,0.f,0.f,o->Light);
#else
      			RequestTerrainLight(o->Position[0],o->Position[1],o->Light);
#endif
				if(o->LifeTime <= 0) o->Live = false;
				if(o->LifeTime < 50) o->Alpha = o->LifeTime*0.02f;
				break;
			}
		}
	}
}

void RenderPointers()
{
	EnableAlphaBlend();
	for(int i=0;i<MAX_POINTERS;i++)
	{
		PARTICLE *o = &Pointers[i];
		if(o->Live)
		{
			if(Bitmaps[o->Type].Components==3)
				EnableAlphaBlend();
			else
				EnableAlphaTest();
			RenderTerrainAlphaBitmap(o->Type,o->Position[0],o->Position[1],o->Scale,o->Scale,o->Light,-o->Angle[2],o->Alpha);
		}
	}
}
