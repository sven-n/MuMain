# ADR 0001: Preserve 25 FPS Reference Timing

**Status:** Accepted

**Date:** 2026-09-02

## Context

MU animation speeds, movement offsets, and related effects were authored
against a 25 FPS reference rate. Runtime frame rates vary by platform, GPU,
VSync mode, and scene complexity.

Advancing those values once per rendered frame without scaling changes game
speed. Limiting frame rate may hide that defect, but does not make the logic
correct.

## Decision

- Keep `REFERENCE_FPS` at `25.0`.
- Compute `FPS_ANIMATION_FACTOR` from elapsed wall-clock time as
  `clamp(REFERENCE_FPS / FPS, 0, 1)`.
- Multiply additive per-frame animation, movement, physics, and effect changes
  by `FPS_ANIMATION_FACTOR`.
- Apply exponential changes as `pow(base, FPS_ANIMATION_FACTOR)`.
- Use actual elapsed milliseconds for wall-clock timers and timeouts.
- Do not rely on VSync or a frame cap for gameplay correctness.

At frame rates of at least 25 FPS:

```text
step per second
  = FPS * speed * (25 / FPS)
  = speed * 25
```

Below 25 FPS the factor remains capped at `1`. The simulation slows instead
of taking oversized steps that can skip animation keys or destabilize legacy
logic.

## Ambient flock timing

Ambient birds, bats, butterflies and fish use the same reference-time basis for flock steering.
The turn limit is applied as degrees per reference frame multiplied by the frame factor.
For a Lorencia bird's 13-degree reference step, this is 325 degrees per second at frame rates of at least 25 FPS.
Below 25 FPS, each update remains capped at the reference step.
Fractional headings are retained so small high-FPS turns are not rounded away.
Butterflies steer only when `rand_fps_check(4)` passes, about every four reference frames at any frame rate.
That gate already follows time, so each of their steers takes the whole reference step rather than a share of it.
Normalized flock directions describe geometry, not elapsed time; only the turn step is time-scaled.
Players do not need to change VSync or their FPS limit to keep the intended turning speed.

## Head-turn smoothing

Characters turn their heads toward their existing look targets by closing 20% of the remaining angular gap per 25-FPS reference frame.
For a fractional reference frame, the blend is `1 - pow(0.8, FPS_ANIMATION_FACTOR)` rather than `0.2 * FPS_ANIMATION_FACTOR`.
This preserves the exponential response over equal elapsed time at frame rates of at least 25 FPS.
Below 25 FPS, each update remains capped at the original 20% blend.
Yaw and pitch follow the shortest angular path; roll and look-target selection are unchanged.
This also applies to helper NPCs using player models, such as Leo the Helper.
No player configuration or FPS cap is required.

## Dark Raven timing

The Dark Lord's Dark Raven travels and turns using the same 25-FPS reference timing as other creatures.
Its stored flight direction is a displacement per reference frame; rotating it into world space does not change that unit.
Only the displacement applied to its position takes the frame factor.
Random course changes already occur at the reference rate, so each event keeps its full random turn and averages the new speed with the full previous speed.
Its recovery teleport places it 250 units above the owner regardless of frame rate.
The opening attack trail emits its ten-joint batches at the reference rate rather than on every rendered frame.
Animation playback and attack timers retain their existing elapsed-time scaling.
Players do not need to change VSync or their FPS limit for the intended flight and attack presentation.

## Consequences

- Raw per-frame increments are defects unless the value is intentionally
  frame-count based.
- `rand_fps_check(n)` is the per-frame form of the original `rand() % n == 0`
  and belongs only on chances rolled every frame. A pick between variants, or
  a chance rolled once per event (spawn setup in `CreateEffect`,
  `CreateJoint`, `CreateParticle`, packet handlers), stays a plain `rand()`
  pick. A gate nested in another gate, or around a `*FpsChecked` spawn,
  scales the chance twice. Where the original rolled `rand() % n`, keep that
  `n`: it is the intended rate at 25 FPS.
- A value set once, when an effect, joint or particle is created, never
  takes `FPS_ANIMATION_FACTOR`: spawn offsets, start angles, random scatter,
  scale, lifetime and the start values of rates such as gravity, direction
  and velocity. The move code that applies a rate each frame scales it.
- The same holds for a value rebuilt every frame before it is used: an aim
  or spawn point offset from a fresh copy of a position, a draw offset the
  function undoes afterwards, a brightness or texture coordinate computed
  per segment, or a multiplier on a speed read fresh from the action
  (`PlaySpeed /= 2`, since `PlayAnimation` scales the result). Only state
  that carries over to the next frame takes the factor.
- To choose, ask whether the line runs more often per second when the frame
  rate rises. Code that looks one-shot can run every frame while its
  condition holds: a clamped `AnimationFrame` with a `>=` test, a range of
  attack times, a state nothing resets. Only a condition that ends (the object
  dies, a counter resets, `CheckAttackTime` fires once) makes a roll per
  event. When unsure, count the line over 10 seconds at `$fps 25` and
  uncapped.
- `if (gate) A; else B;`, with both rolled every frame, runs `B` on nearly
  every frame at high FPS. Gate the frame, then pick:
  `if (rand_fps_check(1)) { if (rand() % n == 0) A; else B; }`.
- An integer frame counter, or anything laid once per frame (trail tails,
  weapon blurs), cannot take a fractional step. It steps by
  `REFERENCE_FRAME_STEP`, the whole reference frames that passed this frame
  (0 or 1), or keeps its own accumulator like `JOINT::TailFrames`. A counter
  compared with `==` needs care: between steps it holds its value for
  several frames. Blurs age their trail on reference frames only; the
  poses one frame submits replace the previous frame's poses until a
  reference frame keeps them, so a multi-pose swing stays whole.
- A trail that lays one tail per reference frame keeps its authored
  `MaxTails`; only trails still laid every rendered frame need the
  `MaxTails / FPS_ANIMATION_FACTOR` stretch from `CreateJoint`.
- A per-frame multiplier on persistent state (a light fade, friction, a
  velocity growth such as `VectorScale(o->Light, 0.9f, o->Light)`) becomes
  `powf(k, FPS_ANIMATION_FACTOR)`, so it compounds to `k` per reference frame.
  A plain `k` fades or grows exponentially faster at high FPS.
- A loop that builds a shape within one frame, such as a lightning bolt
  walking toward its target over `MaxTails` steps, takes spatial steps: its
  step length, jitter and turns (`MoveHumming(…, false)`) take no factor.
- Motion driven by `WorldTime` uses a continuous clock, not
  `(int)(WorldTime / 40)`, which holds still for 40 ms at a time at high FPS.
- Changing `REFERENCE_FPS` changes global gameplay timing and requires
  asset calibration plus broad behavioral verification.
- Frame caps remain performance and presentation controls, not correctness
  controls.

## References

- [Reference rate and factor calculation](../../src/source/Engine/AI/ZzzAI.cpp)
- [Reference-rate declaration](../../src/source/Engine/AI/ZzzAI.h)
- [Canonical animation step](../../src/source/Render/Models/ZzzBMD.cpp)
