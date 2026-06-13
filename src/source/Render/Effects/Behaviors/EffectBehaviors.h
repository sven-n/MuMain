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

    // onCreate hook for effects whose creation mixes randomness with directional
    // setup, which doesn't reduce to independent scalar parameters.
    void CreateMayaStone45(OBJECT* o);

    bool MoveDesair(OBJECT* o, int index, float luminosity);
    bool MoveInfinityArrow4(OBJECT* o, int index, float luminosity);
    bool MoveMagicCapsule2(OBJECT* o, int index, float luminosity);
    bool MoveSpear(OBJECT* o, int index, float luminosity);
    bool MoveSummonerNeilNife(OBJECT* o, int index, float luminosity);
    bool MoveSummonerNeilGround(OBJECT* o, int index, float luminosity);
    bool MoveBitmapFire(OBJECT* o, int index, float luminosity);
    bool MoveBitmapFireRed(OBJECT* o, int index, float luminosity);
    bool MoveBitmapLightMarks(OBJECT* o, int index, float luminosity);
    bool MoveMagic1(OBJECT* o, int index, float luminosity);
    bool MoveMayaStar(OBJECT* o, int index, float luminosity);
}
