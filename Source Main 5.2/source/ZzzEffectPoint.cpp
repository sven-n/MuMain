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


void RenderNumberPoints(vec3_t Position, int Num, vec3_t Color, float Alpha, float Scale)
{
    vec3_t p;
    VectorCopy(Position, p);
    vec3_t Light[4];
    VectorCopy(Color, Light[0]);
    VectorCopy(Color, Light[1]);
    VectorCopy(Color, Light[2]);
    VectorCopy(Color, Light[3]);

    char Text[32];
    itoa(Num, Text, 10);
    p[0] -= strlen(Text) * 5.f;
    unsigned int Length = strlen(Text);
    p[0] -= Length * Scale * 0.125f;
    p[1] -= Length * Scale * 0.125f;

    float sinTh = sinf((float)(ANGLE_TO_RAD * (CameraAngle[2])));
    float cosTh = cosf((float)(ANGLE_TO_RAD * (CameraAngle[2])));

    for (unsigned int i = 0;i < Length;i++)
    {
        float UV[4][2];
        float u = (float)(Text[i] - 48) * 16.f / 256.f;
        TEXCOORD(UV[0], u, 16.f / 32.f);
        TEXCOORD(UV[1], u + 16.f / 256.f, 16.f / 32.f);
        TEXCOORD(UV[2], u + 16.f / 256.f, 0.f);
        TEXCOORD(UV[3], u, 0.f);
        RenderSpriteUV(BITMAP_FONT + 1, p, Scale, Scale, UV, Light, Alpha);
        p[0] += Scale / 0.7071067f * cosTh / 2;
        p[1] -= Scale / 0.7071067f * sinTh / 2;
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

            if (o->Type > -1)
            {
                RenderNumberPoints(o->Position, o->Type, o->Angle, o->Gravity * 0.4f, o->Scale);
            }
            else
            {
                RenderNumber(o->Position, o->Type, o->Angle, o->Gravity * 0.4f, o->Scale);
            }
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