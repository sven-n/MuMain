# Camera System

User guide for the in-game cameras. Covers what cameras you can switch
between, the keys to control them, and the player-visible changes that
landed in this rework.

For the in-editor tuning UI, see [`dev-editor.md`](dev-editor.md).

---

## Cameras

| Camera | Where | Notes |
|--------|-------|-------|
| **Default** | Everywhere | Original third-person follow. |
| **Orbital** | Gameplay maps only | Middle-mouse drag to look around, wheel to zoom. |
| **FreeFly** | Editor only | Free-look spectator. Not in Release builds. |

Switching cameras outside gameplay (login, character select) is not
supported - only the Default camera runs there. Leaving a gameplay map
automatically returns you to Default.

---

## Controls

| Key | Action |
|-----|--------|
| **F9** | Cycle to the next camera (Default ↔ Orbital). |
| **F10** | Toggle zoom lock. Default is **on** so the wheel never zooms by accident. |
| **F11** | Reset the active camera. Default returns to its starting zoom rung; Orbital also resets rotation. |
| **Mouse wheel** | Zoom in / out (when zoom is unlocked). |
| **Middle-mouse drag** | Rotate the Orbital camera. |

Tip: F10 is "global" - toggling it once unlocks the wheel for whichever
camera is active, and it stays unlocked through camera switches until you
press F10 again.

---

## What's new

- **F10 zoom lock and F11 reset.** Both keys work with whichever camera
  is currently active.
- **Orbital wheel zoom remembers your last setting** across sessions (saved
  to `config.ini` under `[Camera] Zoom`).
- **Eight Default zoom rungs** (was five). The ladder now adds three
  closer-in steps below the previous floor; the starting rung sits one
  step in from the original middle. F11 returns you to that starting rung.
- **Per-map camera overrides removed.** Castle Siege, PK Field, the
  6th-character home, and a few others used to forcibly clamp or reposition
  the camera. They now all share the same Default-camera zoom range.
- **Widescreen rendering fix.** On 16:9 the upper-left and upper-right
  screen corners no longer show missing terrain.
- **Editor-only:** the FreeFly cone overlay now draws coloured lines on
  the ground showing where the spectated camera's view actually meets the
  terrain - red at the near edge, yellow at the far edge.

---

## For developers

If you want the architecture, code layout, or tuning sliders, see
[`dev-editor.md`](dev-editor.md) and the merged PRs that introduced this:
[#335](https://github.com/sven-n/MuMain/pull/335) (3D camera rework) and
[#364](https://github.com/sven-n/MuMain/pull/364) (zoom controls).
