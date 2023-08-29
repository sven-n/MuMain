///////////////////////////////////////////////////////////////////////////////
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
#include "NewUISystem.h"

PARTICLE  Points[MAX_POINTS];

int g_iLatestPoint = -1;

void CreatePoint(vec3_t Position, int Value, vec3_t Color, float scale, bool bMove, bool bRepeatedly)
{
    if (!g_pOption->GetRenderAllEffects())
    {
        return;
    }

    for (int i = 0; i < MAX_POINTS; i++)
    {
        PARTICLE* o = &Points[i];
        if (!o->Live)
        {
            o->Live = true;
            o->Type = Value;
            VectorCopy(Position, o->Position);
            o->Position[2] += 140.f;
            VectorCopy(Color, o->Angle);
            o->bRepeatedly = bRepeatedly;
            o->fRepeatedlyHeight = RequestTerrainHeight(o->Position[0], o->Position[1]) + 140.0f;
            o->Gravity = 10.f;
            o->Scale = scale;
            o->LifeTime = 0;
            o->bEnableMove = bMove;
            return;
        }
    }
}

void RenderPoints(BYTE byRenderOneMore)
{
    if (!g_pOption->GetRenderAllEffects())
    {
        return;
    }

    EnableAlphaTest();
    DisableDepthTest();
    for (int i = 0; i < MAX_POINTS; i++)
    {
        PARTICLE* o = &Points[i];
        if (o->Live)
        {
            if (byRenderOneMore == 1)
            {
                if (o->Position[2] > 350.f) continue;
            }
            else if (byRenderOneMore == 2)
            {
                if (o->Position[2] <= 300.f) continue;
            }
            else if (o->bRepeatedly)
            {
                if (o->Position[2] <= o->fRepeatedlyHeight) continue;
            }

            RenderNumber(o->Position, o->Type, o->Angle, o->Gravity * 0.4f, o->Scale);
        }
    }
}

void MovePoints()
{
    if (!g_pOption->GetRenderAllEffects())
    {
        return;
    }

    for (int i = 0; i < MAX_POINTS; i++)
    {
        PARTICLE* o = &Points[i];
        if (o->Live)
        {
            o->LifeTime -= FPS_ANIMATION_FACTOR;
            if (o->LifeTime < 0)
            {
                if (o->bRepeatedly && o->Position[2] > o->fRepeatedlyHeight)
                {
                    o->Gravity = 10.0f;
                    o->bRepeatedly = false;
                }
                if (o->bEnableMove)
                {
                    o->Position[2] += o->Gravity * FPS_ANIMATION_FACTOR;
                }
                o->Gravity -= 0.3f * FPS_ANIMATION_FACTOR;
                if (o->Gravity <= 0.f)
                    o->Live = false;
                if (o->Type != -2)
                {
                    o->Scale -= 5.f * FPS_ANIMATION_FACTOR;//20.f;
                    if (o->Scale < 15.f)
                        o->Scale = 15.f;
                }
            }
        }
    }
}