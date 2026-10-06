# Possible Follow-up Ideas

A collection of ideas for later: work a design found useful but did not plan,
or put off. Nothing here is planned. Each idea says where it comes from, what
is wanted in general and, where known, what it would take.

When an idea is started, it gets its own design document (or a phase in the
design of its area), extended from its entry here and checked against the
code; the entry then points there. Designs add their ideas for later here
instead of keeping them as open questions.

## From the effect catalogue (FX1)

From the [effect catalogue design](2026-10-02-effect-catalogue-design.md).

### FX1.7c Live preview in the browser

**Status: not for now, maybe later** (decided after FX1.7b).

**What is wanted.** The effect browser's preview (FX1.7a) shows what a
type's slot holds, its model or its texture, on nothing, a plane, a cube or
an item. The preview in the world (FX1.7b) creates the type in front of the
character with the game's own call, so its real code runs: how it moves and
draws, the particles and sprites it creates, its sounds. FX1.7c brings the
two together: the objects the world preview creates are drawn inside the
browser's view, on the chosen object, instead of in the world. The real
effect can then be looked at in the browser, from any side, without the map
around it.

**Why not now.** The world preview, with the Dev Editor's free camera and
F12 hiding the editor while it runs, already shows the real effect in the
game. FX1.7c would change game render code for an editor convenience.

**What it would take** (from the catalogue design; to be checked against the
code when starting):

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

**When starting.** Write its design before any code: check the points above
against the code (the order of the drawing in the frame, what each loop does
per object, where the capture is recorded in the frame, the camera and
frustum state the draw code reads), decide the owner and the ground, and
list the tests and the checks in the client. As for the other phases:
nothing changes in game, nothing gets slower in game, and the changes are in
editor builds only.

### The game view as an editor window

**What is wanted.** The game's frame drawn into a window of the editor
instead of filling the screen behind it. It would show the world preview
(FX1.7b) inside the editor, and help the map editor and the other tools as
well. It is an editor change for all tools, not part of the effect
catalogue.

### "Used by" from the code

**What is wanted.** The effect browser shows which data uses a type. A
generated index of the code's call sites would add the code that uses it.
The world preview's list of the game's calls already reads the create calls
from the sources at run time.
