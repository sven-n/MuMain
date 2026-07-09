# Frame-Rate Coupling Audit (2026-04-26)

Audit of audio playback and animation timing under uncapped SDL3 GPU
frame rate, prompted by two user-reported symptoms:

1. **Audio**: Lorencia anvil (and other map-ambient SFX) sounding "hundreds
   of times" instead of once per intended hammer cycle.
2. **Animation**: General perception that animations may be playing too fast,
   particularly inventory item previews like wings.

Outcome: one audio bug found and fixed (`MuMain 3b6378b4`); the general
animation system verified mathematically frame-rate-independent; six narrow
frame-rate-tied bugs catalogued in non-critical subsystems.

---

## 1. Environmental context

The SDL3 GPU swapchain in `RenderFX/MuRendererSDLGpu.cpp` does **not** set
a present mode — `SDL_ClaimWindowForGPUDevice` is called at line 612, but
no follow-up `SDL_SetGPUSwapchainParameters(...)` configures vsync or
frame-cap. The swapchain therefore runs in the driver default (immediate
present mode on most drivers), and frame rate floats anywhere from
60 FPS (low-end) to 300+ FPS (modern GPUs at low scene complexity).

Per-frame logic that was implicitly throttled by the original Win32 build's
vsync is now exposed to that variability.

---

## 2. Audio: ambient SFX retrigger — fixed

### Root cause

`Platform/MiniAudio/MiniAudioBackend.cpp::PlaySound` had no dedupe guard.
Every call did `ma_sound_stop` → `ma_sound_seek_to_pcm_frame(0)` → `ma_sound_set_looping` → `ma_sound_set_position` → `ma_sound_start` on a round-robin channel.

The trigger chain `MapProcess::MoveObject → BaseMap::PlayObjectSound → PlayBuffer(SOUND_X, pObject, false)` runs **per-object per-frame**. With 4-channel round-robin (`MAX_CHANNEL = 4` per `Audio/DSPlaySound.h:8`), per-frame triggering produces:

- 4 simultaneous overlapping plays, each restarted on its 4th frame
- 60-300 restarts per second → audible "hundreds of times" stutter

Win32 + DirectSound legacy path likely had implicit "Play() on a still-playing
buffer is a no-op" semantics, OR was simply vsync-clamped to ~60 Hz. The SDL3
port (stories 5-2-1 / 5-2-2 / 7-9-4) didn't carry forward an explicit dedupe.

### Fix

`MuMain 3b6378b4` adds a guard before the round-robin advance: when the caller
passes a non-null `pObject` (the ambient/positional pattern), skip if any
channel is already playing this sound. Event-driven SFX (UI clicks, attack
swings, item pickups, `SOUND_REPAIR`) pass `pObject == nullptr` and continue
to overlap freely, preserving rapid-swing layering.

```cpp
if (pObject != nullptr)
{
    for (int existingCh = 0; existingCh < m_loadedChannels[bufIdx]; ++existingCh)
    {
        if (ma_sound_is_playing(&m_sounds[bufIdx][existingCh]))
        {
            return true;
        }
    }
}
```

The fix is in the audio primitive, not in the ~10 per-map `GM*::PlayObjectSound`
implementations — every existing and future ambient SFX self-corrects without
touching call sites.

Parent repo bumped: `MuMain-workspace 255d504`.

---

## 3. Animation: general system verified correct

### The `FPS_ANIMATION_FACTOR` design

The codebase normalizes animation rate to a constant **wall-clock** rate
through an FPS-derived multiplier:

```cpp
// Gameplay/Characters/ZzzAI.cpp:730-735 — runs once per frame from RenderScene
const double differenceMs = std::max(WorldTime - last, 0.001);
FPS = 1000.0 / differenceMs;
const double fpsRatio = (FPS <= 0) ? 0 : 25.0 / FPS;  // REFERENCE_FPS = 25
FPS_ANIMATION_FACTOR = std::clamp(fpsRatio, 0.f, 1.f);

// RenderFX/ZzzBMD.cpp:413 — the canonical animation step
*AnimationFrame += Speed * FPS_ANIMATION_FACTOR;
```

`WorldTime` from `Core/Timer.cpp` is `std::chrono::steady_clock` in
milliseconds — wall-clock-correct. `CalcFPS()` is called once per frame at
the top of `Scenes/SceneManager.cpp::RenderScene` (line 1014).

### The math — FPS-invariant at typical rates

Per frame: `*AnimationFrame += Speed × (25 / FPS)` (when `FPS ≥ 25`).
Per second: `FPS × Speed × (25 / FPS) = Speed × 25` keyframes — constant.

| FPS | Factor | Step/frame (Speed=0.33) | Steps/sec |
|---|---|---|---|
| 25 (reference) | 1.000 | 0.330 | 8.25 |
| 60 | 0.417 | 0.137 | 8.25 |
| 144 | 0.174 | 0.057 | 8.25 |
| 250 | 0.100 | 0.033 | 8.25 |
| 10 (clamped) | 1.000 (clamp) | 0.330 | 3.30 (slower — intentional) |

The `clamp(0, 1)` ceiling makes animations *slower than designed* below
25 FPS (per the inline comment "animate with no less than REFERENCE_FPS").
This is the only place the system is genuinely frame-rate-tied, and it's in
the slow-only direction — never faster. So `clamp` cannot produce a
"too fast" perception.

### Paths verified correct

| Path | File:Line | Uses factor? |
|---|---|---|
| Character/monster/object animation | `RenderFX/ZzzBMD.cpp:413` | ✓ |
| World map per-frame stepping | `World/Maps/GM*.cpp` (`fSpeedPerFrame`) | ✓ |
| Sprite effects | `RenderFX/zzzeffectsprite.cpp:28` (`kSpriteFrameStep × FPS_ANIMATION_FACTOR`) | ✓ |
| Player weapon blur sub-frame interpolation | `Gameplay/Characters/ZzzCharacter.cpp:3988` | ✓ |
| `WING_OF_STORM` texture U-scroll | `Gameplay/Characters/ZzzObject.cpp:7637` (`s_iTexAni += FPS_ANIMATION_FACTOR`) | ✓ |
| Velocity-as-Speed (14 sites in `RenderFX/ZzzEffect.cpp`, `Gameplay/Characters/GOBoid.cpp`, etc.) | `o->Velocity` is a literal constant; factor applied inside `PlayAnimation` | ✓ |
| Ground-item gravity | `Gameplay/Characters/ZzzObject.cpp:6700` (`o->Gravity * FPS_ANIMATION_FACTOR`) | ✓ |

### Conclusion

The general animation system is **time-based** by construction. Any
"too fast" perception in normal gameplay (character walking, attacking,
monster idle, world ambient) cannot originate from the math of the
factor itself.

---

## 4. Frame-rate-tied bugs catalogued (narrow scope)

Six call sites step a per-frame value with a hardcoded literal, **bypassing**
`FPS_ANIMATION_FACTOR`. At 144 FPS these advance ~5.7× faster than at 25 FPS.
At 250 FPS, ~10× faster.

### MonkSystem (Rage Fighter abilities)

| File:Line | Code | Affects |
|---|---|---|
| `Gameplay/Characters/MonkSystem.cpp:727` | `m_fDistanceFrame += 1.0f;` | Dark Side teleport trail position |
| `Gameplay/Characters/MonkSystem.cpp:1503` | `m_fDisFrame += 0.7f;` | Dummy/Phantom unit position interpolation |
| `Gameplay/Characters/MonkSystem.cpp:1504` | `m_fAniFrame += 0.05f;` | Dummy/Phantom unit animation frame |

### Chaos Castle event physics

| File:Line | Code | Affects |
|---|---|---|
| `Gameplay/Events/CSChaosCastle.cpp:265` | `g_fActionObjectVelocity += 0.4f;` | Chaos Castle action object falling speed (block 1) |
| `Gameplay/Events/CSChaosCastle.cpp:299` | `g_fActionObjectVelocity += 0.4f;` | Block 2 |
| `Gameplay/Events/CSChaosCastle.cpp:338` | `g_fActionObjectVelocity += 0.4f;` | Block 3 |

(There's also `Gameplay/Characters/ZzzObject.cpp:121 g_fActionObjectVelocity += 1.5f` in the same global's update, with the same shape — likely the same Chaos Castle physics step.)

### Status

These are **real bugs** but scoped to specific class abilities (Rage Fighter Dark
Side / Phantom Stance) and a specific event (Chaos Castle action objects).
They do **not** explain a general "animations too fast" feel during normal play.

If they prove visible in-game (Phantom moves are visibly too fast at >25 FPS,
Chaos Castle traps fall too fast), the fix is to multiply each literal by
`FPS_ANIMATION_FACTOR` — analogous to the existing pattern in `MapProcess::MoveItems`
where `o->Gravity * FPS_ANIMATION_FACTOR` is the canonical form.

Not yet filed as a story; deferred until in-game reproduction confirms visibility.

---

## 5. Open question: wings in inventory

The user reports inventory item previews — particularly wings — appear to
animate too fast. Code search **could not** identify the responsible code path:

- `RenderItem3D` (`Gameplay/Items/ZzzInventory.cpp:11104`) creates a
  stack-allocated `CHARACTER Armor` per call; its `Object.AnimationFrame`
  is uninitialized but the function then calls `RenderPartObject(o, Type, ...)`
  without first running `PlayAnimation`.
- `ObjectSelect.AnimationFrame` is **reset to 0** every call
  (`ZzzInventory.cpp:10276`).
- `PART_t::PlaySpeed` defaults to 0 in its constructor
  (`Gameplay/Characters/w_CharacterInfo.h:68`); `grep` finds **no** assignment
  anywhere except `ZzzCharacter.cpp:6638` for `MODEL_WING_OF_RUIN` and
  `ZzzCharacter.cpp:8943, 8963` for monster-only wings.
- Through the `RenderLinkObject → PlayAnimation(&f->AnimationFrame, …, f->PlaySpeed, …)`
  path, wings with `PlaySpeed = 0` should **not** advance their animation
  frame at all — yet wings clearly animate.

The flap motion must originate from a path not yet traced — most likely a
`WorldTime`-keyed bone rotation, a `static` per-call accumulator inside
`RenderPartObjectEffect` or `RenderPartObjectBody`, or a BMD intrinsic
animation system not driven by `*AnimationFrame +=`. Code search has reached
its useful limit for this question.

### Two productive next steps if the issue is later prioritised

1. **Runtime probe** — add temporary `mu::log::Get("anim")->info(...)` to
   `RenderLinkObject` for wing-Type values, log per-frame `AnimationFrame`,
   `WorldTime`, `FPS`. Run game in inventory view, observe what's changing
   per frame and at what rate. ~15 min.

2. **VSync bisect** — temporarily call
   `SDL_SetGPUSwapchainParameters(s_device, s_window, format, SDL_GPU_PRESENTMODE_VSYNC)`
   after `SDL_ClaimWindowForGPUDevice` in `MuRendererSDLGpu.cpp:612`. If wings
   then look correct, the issue is genuinely frame-rate-tied (and the runtime
   probe in step 1 narrows it). If wings still look fast under vsync, the issue
   is rate-correctness (PlaySpeed asset values authored against a different
   reference rate) — code-level audits won't help. ~10 min to add the line.

VSync bisect is the cheaper first move — its outcome partitions the next
investigation into "code search" vs. "asset calibration" cleanly.

---

## 6. Forward-looking notes

### REFERENCE_FPS = 25

`Gameplay/Characters/ZzzAI.h:11` defines `constexpr double REFERENCE_FPS = 25.0`.
This is the rate at which animations are designed to play — i.e., BMD asset
`PlaySpeed` values are interpreted as "per 25-FPS-frame". If the original game
was authored against a different reference (60 FPS is plausible for late-90s
clients), all animations would be uniformly mistuned. There's no code-level
way to verify which reference was used during asset creation; a side-by-side
visual comparison with the Win32 reference build would settle it.

### VSync as a global mitigation

A vsync-locked frame rate at the monitor refresh (60-144 Hz typical) would:

- Bound `FPS` variance, making any latent frame-rate-tied bugs reproducible
- Not fix the 6 catalogued bugs (they'd advance more slowly but still wrong)
- Not fix asset-rate-mistune bugs (if any)
- Reduce GPU/CPU load, which is generally good

Worth considering as a follow-up enhancement story regardless of the
animation question. The minimum diff is one `SDL_SetGPUSwapchainParameters`
call after `SDL_ClaimWindowForGPUDevice` in `MuRendererSDLGpu.cpp`.

---

## 7. References

- Audio fix commit: `MuMain 3b6378b4` (`fix(audio): dedupe per-frame ambient SFX retriggers in MiniAudioBackend`)
- Parent submodule bump: `MuMain-workspace 255d504` (`chore(submodule): bump MuMain — fix ambient SFX per-frame retrigger`)
- Audit-related stories filed under `_bmad-output/stories/7-9-{5,14,15,16,17,18,19}` cover the cross-platform stub deletion thread; this audit is independent of those.
- Original PR with the per-frame ambient pattern: stories 5-2-1, 5-2-2, 7-9-4 (DirectSound → miniaudio migration).
