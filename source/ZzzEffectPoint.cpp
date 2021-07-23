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


PARTICLE  Points	[MAX_POINTS];


///////////////////////////////////////////////////////////////////////////////
// 숫자 처리(게임상의 공격시 데미지 뜨는거 등등)
///////////////////////////////////////////////////////////////////////////////

int g_iLatestPoint = -1;	// 가장 최근 숫자의 인덱스

#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
void CreatePoint(vec3_t Position,int Value,vec3_t Color,float scale, bool bMove, bool bRepeatedly)
#else //PBG_ADD_NEWCHAR_MONK_SKILL
void CreatePoint(vec3_t Position,int Value,vec3_t Color,float scale, bool bMove)
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
{
	for(int i=0;i<MAX_POINTS;i++)
	{
		PARTICLE *o = &Points[i];
		if(!o->Live)
		{
			o->Live = true;
			o->Type = Value;
			VectorCopy(Position,o->Position);
			o->Position[2] += 140.f;
			VectorCopy(Color,o->Angle);
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
			o->bRepeatedly = bRepeatedly;
			o->fRepeatedlyHeight = RequestTerrainHeight(o->Position[0], o->Position[1])+140.0f;
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
			o->Gravity  = 10.f;
			o->Scale    = scale;
			o->LifeTime = 0;
            o->bEnableMove = bMove;
			return;
		}
	}
}

void RenderPoints( BYTE byRenderOneMore )
{
#ifdef DO_PROFILING
	g_pProfiler->BeginUnit( EPROFILING_RENDER_POINTS, PROFILING_RENDER_POINTS );
#endif // DO_PROFILING
	
	EnableAlphaTest();
    DisableDepthTest();
    for(int i=0;i<MAX_POINTS;i++)
	{
		PARTICLE *o = &Points[i];
		if(o->Live)
		{
            if ( byRenderOneMore==1 )            //  수면 아래의 것들만 찍는다.
            {
                if ( o->Position[2]>350.f ) continue;
            }
            else if ( byRenderOneMore==2 )       //  수면위의 것들만 찍는다.
            {
                if ( o->Position[2]<=300.f ) continue;
            }
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
			else if(o->bRepeatedly)
			{
				if( o->Position[2] <= o->fRepeatedlyHeight) continue;
			}
#endif //PBG_ADD_NEWCHAR_MONK_SKILL

            RenderNumber(o->Position,o->Type,o->Angle,o->Gravity*0.4f,o->Scale);
		}
	}

#ifdef DO_PROFILING
	g_pProfiler->EndUnit( EPROFILING_RENDER_POINTS );
#endif // DO_PROFILING
}

void MovePoints()
{
#ifdef DO_PROFILING
	g_pProfiler->BeginUnit( EPROFILING_MOVE_POINTS, PROFILING_MOVE_POINTS );
#endif // DO_PROFILING
	for(int i=0;i<MAX_POINTS;i++)
	{
		PARTICLE *o = &Points[i];
		if(o->Live)
		{
			o->LifeTime--;
			if(o->LifeTime < 0)
			{
#ifdef PBG_ADD_NEWCHAR_MONK_SKILL
				if(o->bRepeatedly && o->Position[2]>o->fRepeatedlyHeight)
				{
					o->Gravity = 10.0f;
					o->bRepeatedly = false;
				}
#endif //PBG_ADD_NEWCHAR_MONK_SKILL
                if ( o->bEnableMove )
                {
				    o->Position[2] += o->Gravity;
                }
				o->Gravity -= 0.3f;
				if(o->Gravity <= 0.f)
					o->Live = false;
				if(o->Type != -2)
				{
					o->Scale -= 5.f;//20.f;
					if(o->Scale < 15.f)
						o->Scale = 15.f;
				}
			}
		}
	}
#ifdef DO_PROFILING
	g_pProfiler->EndUnit( EPROFILING_MOVE_POINTS );
#endif // DO_PROFILING
}
