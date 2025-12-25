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
#include "ZzzAI.h"
#include "ZzzEffect.h"
#include "DSPlaySound.h"
#include "WSclient.h"
#include "Random.h"
#include "CustomCamera3D.h"

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
                o->LifeTime = 50 + Random::RangeInt(0, 31);
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
    // Disable depth test and face culling to ensure pointers are always visible
    DisableDepthTest();
    bool wasCulling = CullFaceEnable;
    if (wasCulling) {
        glDisable(GL_CULL_FACE);
        CullFaceEnable = false;
    }

    for (int i = 0; i < MAX_POINTERS; i++)
    {
        PARTICLE* o = &Pointers[i];
        if (o->Live)
        {
            if (Bitmaps[o->Type].Components == 3)
                EnableAlphaBlend();
            else
                EnableAlphaTest();

            // Scale up pointer when 3D camera is zoomed out for better visibility
            float effectiveScale = o->Scale;
            if (CCustomCamera3D::IsEnabled())
            {
                float zoomScale = CCustomCamera3D::GetZoomDistance() / 100.0f;
                // Scale much more aggressively so indicator is visible when far away
                // At 1.0x zoom: 1.0 + 1.0 * 2.0 = 3.0x scale
                // At 2.0x zoom: 1.0 + 2.0 * 2.0 = 5.0x scale
                effectiveScale = o->Scale * (1.0f + zoomScale * 2.0f);
            }

            RenderTerrainAlphaBitmap(o->Type, o->Position[0], o->Position[1], effectiveScale, effectiveScale, o->Light, -o->Angle[2], o->Alpha, o->Position[2]);
        }
    }

    // Re-enable depth test and restore face culling state
    EnableDepthTest();
    if (wasCulling) {
        glEnable(GL_CULL_FACE);
        CullFaceEnable = true;
    }
}