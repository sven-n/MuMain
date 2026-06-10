#pragma once

#include "Render/Effects/EffectDef.h"

// Per-frame behaviour handlers extracted from the MoveEffect / RenderEffects
// switches in ZzzEffect.cpp. Each handler owns the logic for one effect type and
// is wired into the registry (see EffectRegistry.cpp). Move handlers return true
// to run MoveEffect's shared tail (lifetime decrement / particle trail /
// destruction) and false to skip it, matching the original cases.
namespace Render::Effects::Behaviors
{
    // Renders with a plain RenderObject(). Effects whose legacy render case was
    // just "RenderObject(o); break;" opt into the registry render path with this.
    void RenderDefault(OBJECT* o);

    bool MoveDesair(OBJECT* o, int index);
    bool MoveInfinityArrow4(OBJECT* o, int index);
    bool MoveMagicCapsule2(OBJECT* o, int index);
    bool MoveSpear(OBJECT* o, int index);
    bool MoveSummonerNeilNife(OBJECT* o, int index);
    bool MoveSummonerNeilGround(OBJECT* o, int index);
    bool MoveBitmapFire(OBJECT* o, int index);
    bool MoveBitmapFireRed(OBJECT* o, int index);
    bool MoveBitmapLightMarks(OBJECT* o, int index);
    bool MoveMagic1(OBJECT* o, int index);
    bool MoveMayaStar(OBJECT* o, int index);
}
