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
}
