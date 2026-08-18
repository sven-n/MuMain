#pragma once

// MainScene.h - Main game scene

#include "Core/Platform/WinCompat.h"

// Main scene lifecycle
void MoveMainScene();
bool RenderMainScene();

// DXP-23 diagnostic: force-disable the Effects render pass (RenderEffectShadows/RenderBoids/
// RenderEffects/RenderBlurs) regardless of build config, to test whether effects overdraw is
// contributing to the GPU-stall (Present bucket) cost or the reported HUD blink. Console-toggled
// via $effects on/off (muConsoleDebug.cpp) -- not gated behind _EDITOR since the report came from a
// Release NoEditor build.
void SetDisableEffects(bool disabled);
bool IsEffectsDisabledDebug();

// DXP-23 diagnostic, finer-grained bisection: `$effects off` (above) confirmed effect rendering is
// the dominant GPU cost during a multi-caster fight, but not WHICH effect system inside it -- these
// three isolate the three separate object systems SkillCast.cpp/pet-action code can populate.
// Sprites/Particles are checked additionally to g_pOption->GetRenderAllEffects() in RenderSprites()/
// RenderParticles() (zzzeffectsprite.cpp/ZzzEffectParticle.cpp); SkillEffectModels is checked inside
// RenderEffects()'s loop (ZzzEffect.cpp) to skip only the g_SkillEffects half of its iteration range,
// leaving the legacy Effects[] array (impact/hit-type effects, not skill casts) still rendering.
void SetDisableSprites(bool disabled);
void SetDisableParticles(bool disabled);
void SetDisableSkillEffectModels(bool disabled);
bool IsSpritesDisabledDebug();
bool IsParticlesDisabledDebug();
bool IsSkillEffectModelsDisabledDebug();

// DXP-23 diagnostic: RenderBoids() (GOBoid.cpp) draws ambient wildlife (crows, butterflies, eagles)
// -- always active in town maps regardless of monsters/skill casts -- via a full RenderObject() draw
// plus RenderBodyShadow() per boid every frame. Owner reported the effects-off FPS jump (130->250+)
// reproduces even with zero monsters nearby, which points at an always-on cost like this rather than
// anything combat-triggered. Independent of SetDisableEffects (which already covers RenderBoids' call
// site in MainScene.cpp, but only together with 3 other systems) -- this isolates it alone.
void SetDisableBoids(bool disabled);
bool IsBoidsDisabledDebug();

// DXP-23 diagnostic: owner found unequipping Wings of Ruin alone took FPS from 120 to 180 (in-game,
// no code change). RenderLinkObject() (ZzzCharacter.cpp) calls an EXTRA b->RenderBodyShadow() for
// every visible wing/cape-wearing character, every frame, on top of that character's own body
// shadow -- doubling the shadow-pass draw count per wing-wearer. This isolates that specific call
// (keeps the wing model/texture/animation itself rendering) to test whether the shadow draw
// specifically is the cost, separate from the wing mesh's own render cost.
void SetDisableWingShadow(bool disabled);
bool IsWingShadowDisabledDebug();

// DXP-23 diagnostic: RenderJoints() (ZzzEffectJoint.cpp) -- the beam/tail-trail effect system Wing of
// Ruin's growing tail-light AND Beam Knight's lightning beam both use (CreateTail(), MaxTails-capped
// per-tail but NOT gated by g_pOption->GetRenderAllEffects() at all, so $effects off never covered
// this). MoveJoints() (tail growth) runs at the fixed 50Hz tick; RenderJoints() (the actual draw of
// the whole accumulated tail) runs at full uncapped render rate -- a long tail's geometry gets
// resubmitted every single render frame, not just once per tick.
void SetDisableJoints(bool disabled);
bool IsJointsDisabledDebug();

// DXP-23 diagnostic: MODEL_WING_OF_RUIN's own body render (RenderPartObjectBody(), ZzzObject.cpp
// ~6997-7008) draws its mesh THREE separate times per frame -- a base texture pass, an animated-
// luminosity overlay pass, and a second glow-layer pass (BITMAP_3RDWING_LAYER) -- none chrome-flagged
// (an earlier chrome-specific hypothesis was checked and ruled out for this exact item; RENDER_CHROME6
// belongs to a different wing, MODEL_WINGS_OF_DESPAIR). This skips the two EXTRA passes, keeping only
// the first, to test whether triple mesh-draw submission is the cost. NOTE: visibly changes the wing's
// look (removes its glow layers) while active -- a measurement tool, not a real fix candidate as-is.
void SetDisableWingExtraLayers(bool disabled);
bool IsWingExtraLayersDisabledDebug();
