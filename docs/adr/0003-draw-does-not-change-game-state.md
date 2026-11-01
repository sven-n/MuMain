# ADR 0003: Drawing Does Not Change Game State

**Status:** Accepted

**Date:** 2026-11-02

## Context

The legacy `Render*` functions did three jobs at once: they decided what to
draw and how it looked, they ran game logic (spawning effects, advancing
animations, simulating cloth, fading and counting), and they issued the draw
calls. That made the game depend on how often things were drawn:

- An object drawn twice in a frame (body and shadow, both water passes, a
  mirror) spawned its effects twice and advanced its state twice. An object
  off screen spawned nothing and stopped advancing.
- Effects read the shared `BoneTransform` array, which held the pose of
  whichever object was drawn last, so an effect could spawn from another
  object's bones.
- A different drawer, a second pass, culling or reordering could not be
  added without changing gameplay.
- Pause, frame stepping, replays and a frozen camera were not possible,
  because drawing advanced the game.

The goal is draw logic that can be replaced: game code describes what to
draw, and a drawer decides how. This is the draw-logic side of a replaceable
renderer ([#865](https://github.com/sven-n/MuMain/issues/865)); the work is
tracked in [#914](https://github.com/sven-n/MuMain/issues/914).

## Decision

- Each frame runs the update once and then the draw once
  (`UpdateSceneState`, then the scene's render).
- **The update owns game state.** It spawns and deletes effects, sprites,
  particles and joints, plays animations (`SetAction`, `PlayAnimation`),
  advances counters, fades and lifetimes, simulates cloth, and builds the
  poses effects read.
- **The draw only reads.** It may compute what it needs locally (a flicker
  brightness, a texture offset) but writes nothing the update or the next
  frame reads, and spawns nothing.
- **Effects that depend on being drawn** run in the update for the objects
  the last frame drew. They start one frame after an object comes into view.
  This one-frame delay is accepted.
- **Poses belong to their object.** An object whose effects need its bones
  keeps its own pose (`Engine::Object::Pose::World`); attached parts keep a
  pose per slot (`CharacterAttachments`). The update builds the pose, the
  draw reuses it, and no effect reads the shared `BoneTransform` for another
  object.
- **Allowed exceptions:** an attached part on its first frame (the update
  hasn't seen it yet), and previews that have no update of their own
  (characters outside the client's character list, forced mount previews).
  The code marks each one with `DrawSpawnCheck::AllowScope`.

## How it is done

The separation is built bottom-up, one stage per series of PRs, with the
same output at each step:

| Stage | What | PRs |
|---|---|---|
| D0 | Audit of the draw path: look decisions, spawns inside `Render*`, per-draw writes to the shared models, `Animation()` call sites | none (audit) |
| D1 | Model meshes and shadows are described as `MeshDraw` / `ShadowDraw` and handed to the active `ModelDrawer`; the default drawer keeps today's output | [#917](https://github.com/sven-n/MuMain/pull/917) |
| D2 | Draws carry the skinning their model was transformed with, instead of reading the shared model | [#918](https://github.com/sven-n/MuMain/pull/918) |
| D3 | Spawns, animation, cloth and state writes move from the draw to the update: characters and monsters, map objects, attached parts, body draws, pets, mounts, boids, the effects themselves; the guard and the purity tools | [#929](https://github.com/sven-n/MuMain/pull/929), [#930](https://github.com/sven-n/MuMain/pull/930), [#931](https://github.com/sven-n/MuMain/pull/931), [#932](https://github.com/sven-n/MuMain/pull/932), [#933](https://github.com/sven-n/MuMain/pull/933) |
| D4 | Effects (sprites, particles, trails) are described and handed to a replaceable effect drawer; the draw can no longer write effect state | planned |
| D5 | The drawer interface is complete, with a second drawer as proof | planned |

The Classic and Enhanced looks are not drawers: both go through the same
drawer, and the renderer applies the active look per draw.

### Known behaviour changes

- Effects of an object that just came into view, and cloth of a character
  that just came into view, start one frame later.
- Items shown in the inventory no longer spawn world sparks; they were placed
  in the inventory's 3D space and not visible.
- Damage numbers on water maps reach full brightness one frame later; they no
  longer brighten in both water passes.
- Objects drawn more than once per frame no longer fade faster during the
  Chaos Castle fall.
- An attached part on its first frame (newly equipped, or moved between hand
  and back) still spawns from the draw for that one frame.

## Consequences

- A spawn or a state write inside a draw is a defect unless it is one of the
  allowed exceptions.
- When code moves from the draw to the update, its order changes relative to
  the code still in the draw. State handed between the two halves (a
  lifetime the draw checks and the update changes) must move with them.
- `Pose::World` bones are already in world space. On them,
  `TransformPosition` takes no translate argument (`false`), unlike the
  legacy draw's model-space bones.
- Reading another object's pose through a shared array is a defect. Pass the
  bones in as an argument.
- An object that owns a pose buffer releases it (`Pose::Release`) before its
  slot is reused.
- Draw-side bookkeeping (`Visible`, drawn-frame marks) is still written by
  the draw. It is meant to move to a cull step before the draw, so that
  drawing writes nothing at all.

## Enforcement

- **Every build:** the spawn functions report any spawn inside the frame's
  draw (`Diagnostics::DrawSpawnCheck`). Developer builds log each offending
  caller once to the error log. Tests read the count.
- **Tests:** draws under test run inside a `DrawScope` and must spawn
  nothing (`CheckDrawSpawnsNone`). Effects that moved are tested with
  sentinel shared bones and a non-zero position, so a stale or wrong-space
  read lands where a check fails.
- **Control socket (developer builds):**
  - `drawspawns` lists every spawn the draw made, with its callers.
  - `drawpurity` compares the gameplay fields of characters, effects, items,
    map objects, mounts, boids, particles and joints before and after the
    draw, and lists the changes.
  - `spawn-effect` creates any effect, particle or joint type at the
    character, so effects no map or skill reaches can be checked.
- These checks show *whether* the draw spawns or writes, not *where* an
  effect lands or whether one stopped spawning. Changes in this area also
  need position and presence tests, and a sweep that lists what it covered.

## References

- [Tracking issue: rendering no longer changes the game (#914)](https://github.com/sven-n/MuMain/issues/914)
- [Replaceable renderer (#865)](https://github.com/sven-n/MuMain/issues/865)
- [Frame order and the draw scope](../../src/source/Scenes/SceneManager.cpp)
- [Draw-spawn guard](../../src/source/Core/Utilities/DrawSpawnCheck.h)
- [Draw purity check](../../src/source/App/Control/DrawPurityCheck.h)
- [Per-object poses](../../src/source/Engine/Object/ObjectPose.h)
- [Attached-part slots](../../src/source/Engine/Object/CharacterAttachments.h)
- [Effects-phase tests](../../tests/gamelogic/test_character_effects_phase.cpp)
- [Control-socket commands](../control-socket.md)
- [ADR 0001: 25 FPS reference timing](0001-preserve-25-fps-reference-timing.md)
