# Effect Live Preview in the Browser (FX1.7c)

**Status: not for now, maybe later.** A possible follow-up of the effect
catalogue ([effect catalogue design](2026-10-02-effect-catalogue-design.md),
FX1), decided after FX1.7b. This document says in general what is wanted.
It is to be extended into a full design, checked against the code, when the
work starts.

## Goal

The effect browser's preview (FX1.7a) shows what a type's slot holds, its
model or its texture, on nothing, a plane, a cube or an item. The preview in
the world (FX1.7b) creates the type in front of the character with the
game's own call, so its real code runs: how it moves and draws, the
particles and sprites it creates, its sounds.

FX1.7c brings the two together: the objects the world preview creates are
drawn inside the browser's view, on the chosen object, instead of in the
world. The real effect can then be looked at in the browser, from any side,
without the map around it.

## Why not now

The world preview, with the Dev Editor's free camera and F12 hiding the
editor while it runs, already shows the real effect in the game. FX1.7c
would change game render code for an editor convenience.

## What it would take

From the design of the effect catalogue; to be checked against the code
when starting:

- The render loops of the pools (`RenderEffectShadows`, `RenderEffects`,
  `RenderParticles`, `RenderJoints`, `RenderSprites`, and
  `RenderAfterEffects` in the Kanturu Maya scene; the trails of
  `RenderBlurs` stay in the world) skip the preview's objects in the world
  and draw only them inside the capture, with the view's camera
  (`SaveCameraPerspective` / `RestoreCameraPerspective`). Only
  `RenderEffects` and `RenderAfterEffects` test the frustum, so the objects
  must count as visible there. These changes are in editor builds only.
- An object is drawn either in the view or in the world, not in both: the
  draw code advances animations and creates sprites and particles.
- Effects that follow their owner follow the copy of the character, so the
  view centres on the character's place, or the chosen object becomes the
  owner.
- The view has no terrain. Ground parts are tiles at the terrain height of
  their cell (`RenderTerrainAlphaBitmap`), so the plane goes at that height.
- `ZzzEffectJoint.cpp` and `zzzeffectsprite.cpp` are checked with cppcheck
  first (`ZzzEffectParticle.cpp` passed it in FX1.0).

## Alternative

The game view as an editor window: the game's frame drawn into a window of
the editor. It would show the world preview inside the editor and help the
map editor and the other tools as well. That is an editor change for all
tools, not part of the effect catalogue.

## When starting

Extend this document before writing code:

- check the points above against the code: the order of the drawing in the
  frame, what each loop does per object, where the capture is recorded in
  the frame, and the camera and frustum state the draw code reads;
- decide the owner and the ground;
- list the tests and the checks in the client.

As for the other phases: nothing changes in game, nothing gets slower in
game, and the changes are in editor builds only.
