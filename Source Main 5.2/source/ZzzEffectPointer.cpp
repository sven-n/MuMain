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

PARTICLE  Pointers[MAX_POINTERS];

void CreatePointer(int Type, vec3_t Position, float Angle, vec3_t Light, float Scale)
{
    PARTICLE* Select = NULL;
    int MinLifeTime = 9999;
    for (int i = 0; i < MAX_POINTERS; i++)
    {
        PARTICLE* o = &Pointers[i];
        if (!o->Live)
        {
            o->Live = true;
            o->Type = Type;
            VectorCopy(Position, o->Position);
            VectorCopy(Light, o->Light);
            o->Angle[2] = Angle;
            o->Alpha = 1.f;
            o->Scale = Scale;
            switch (Type)
            {
            case BITMAP_BLOOD:
            case BITMAP_BLOOD + 1:
            case BITMAP_FOOT:
                o->LifeTime = 50 + rand() % 32;
                break;
            }
            return;
        }
        else
        {
            if (MinLifeTime > o->LifeTime)
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
    for (int i = 0; i < MAX_POINTERS; i++)
    {
        PARTICLE* o = &Pointers[i];
        if (o->Live)
        {
            o->LifeTime -= FPS_ANIMATION_FACTOR;
            switch (o->Type)
            {
            case BITMAP_CURSOR + 5:
                o->Scale -= 0.05f * FPS_ANIMATION_FACTOR;
                if (o->Scale < 0.1f) o->Live = false;
                break;
            case BITMAP_BLOOD:
            case BITMAP_BLOOD + 1:
            case BITMAP_FOOT:
                if (o->Type == BITMAP_BLOOD)
                {
                    o->Scale += 0.004f * FPS_ANIMATION_FACTOR;
                }
                Vector(0.1f, 0.f, 0.f, o->Light);
                if (o->LifeTime <= 0) o->Live = false;
                if (o->LifeTime < 50) o->Alpha = o->LifeTime * 0.02f;
                break;
            }
        }
    }
}

void RenderPointers()
{
    EnableAlphaBlend();
    for (int i = 0; i < MAX_POINTERS; i++)
    {
        PARTICLE* o = &Pointers[i];
        if (o->Live)
        {
            if (Bitmaps[o->Type].Components == 3)
                EnableAlphaBlend();
            else
                EnableAlphaTest();
            RenderTerrainAlphaBitmap(o->Type, o->Position[0], o->Position[1], o->Scale, o->Scale, o->Light, -o->Angle[2], o->Alpha);
        }
    }
}