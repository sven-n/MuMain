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
Normalized flock directions describe geometry, not elapsed time; only the turn step is time-scaled.
Players do not need to change VSync or their FPS limit to keep the intended turning speed.

## Consequences

- Raw per-frame increments are defects unless the value is intentionally
  frame-count based.
- Changing `REFERENCE_FPS` changes global gameplay timing and requires
  asset calibration plus broad behavioral verification.
- Frame caps remain performance and presentation controls, not correctness
  controls.

## References

- [Reference rate and factor calculation](../../src/source/Engine/AI/ZzzAI.cpp)
- [Reference-rate declaration](../../src/source/Engine/AI/ZzzAI.h)
- [Canonical animation step](../../src/source/Render/Models/ZzzBMD.cpp)
