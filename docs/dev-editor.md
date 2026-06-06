# DevEditor

The DevEditor is an in-game tuning UI for camera, scene, and rendering
parameters. It was added as part of PR
[#335](https://github.com/sven-n/MuMain/pull/335).

For the camera architecture itself, see [`camera-system.md`](camera-system.md).

---

## 1. Who it's for

Developers and modders. The DevEditor lets you live-tune camera FOV, far
plane, fog, 2D culling trapezoids, terrain/object render distances, and a
suite of debug visualisations - without rebuilding.

It is **debug-only**:

- Built only when `_EDITOR` is defined (see [Build Configurations in
  `README.md`](../README.md#build-configurations)).
- Requires the ImGui submodule (`git submodule update --init`).
- Not present in Release builds.

---

## 2. Opening it

Two ways:

- **Press F12** in-game.
- Start the client with `--editor` to launch with the editor enabled (or
  toggle later with F12).

There is also an **on-screen toggle button** in the top-right of the window.

The editor is rendered with ImGui and consumes input over the game
(see `MuInputBlockerCore.cpp` for the input-routing logic).

---

## 3. Tabs

### 3.1 Scenes tab

#### Camera Mode controls
Switch between FreeFly and the active gameplay camera (Default / Orbital).
While FreeFly is active, the gameplay camera continues running in the
background (the "spectated camera" tracked by `CameraManager`) - toggling back
puts you exactly where the gameplay camera now is, not where you left off.

The standard in-game hotkeys still work with the editor open:

- **F9** cycles Default ↔ Orbital.
- **F10** toggles the zoom lock. F10 is intercepted at the Win32 message
  layer (`WM_SYSKEYDOWN`) so it fires reliably even when ImGui has
  keyboard focus.
- **F11** resets the active camera (zoom rung for Default, rotation +
  zoom for Orbital).

While FreeFly is active and spectating a gameplay camera, the cone
wireframe is augmented with two ground lines: a **red** line where the
bottom of the spectated view hits the terrain and a **yellow** line where
the top hits it. They terminate at the cone's apex→corner edges, so they
visualise the camera's actual visible extent on the ground.

#### Summary line
Shows real-time camera position (world coords + tile coords) and pitch / yaw.
Useful when placing fixed camera shots or noting positions for screenshots.

#### Login Scene
Tunes the login scene only. Defaults come from `LoginSceneCameraDefaults`
in `src/source/CameraMove.h`.

| Control | What it changes | When you'd use it |
|---------|-----------------|-------------------|
| **Offset X / Y / Z** | Translates the camera in world units relative to the tour-path point. | Fixing clipping, framing the title art differently. |
| **Pitch / Yaw** | Rotates the login camera. Pitch is the engine convention (`-90°` = horizontal, more negative = tilted up). | Lining up a screenshot or matching legacy framing. |
| **Tour mode - pause / resume / restart** | Halts or rewinds the login waypoint walk. | Holding the camera still on a specific frame to tune offsets, then resuming. |
| **Render terrain distance** | Hard cap on how far login terrain tiles render. | Login fly-throughs that show distant terrain that normally pops in. |
| **Render object distance** | Hard cap on object cull radius for the login scene. | Same as above for objects/decor. |

#### Game Scene - Default Camera Override
Live-overrides `CameraConfig::ForMainSceneDefaultCamera()`. All values reset
to the factory config on restart.

| Control | What it changes | When you'd use it |
|---------|-----------------|-------------------|
| **Far Plane** (~0.5m to 20km) | Grows or shrinks the visible far clip - also drives 3D object cull range. | Checking how much the game can render before fog/cull cuts in; tuning visibility for screenshots. |
| **Camera Offset X / Y / Z** | Hero-relative. Shifts the camera's eye point relative to the hero anchor. | Repositioning the third-person rig (e.g. higher angle, off-shoulder). |
| **2D Culling Trapezoid Width - near / far multipliers** | Scales the Default camera's view-space ground trapezoid (near and far half-widths). The trapezoid itself is now derived from the live GL projection (FOV/aspect), so 16:9 corners stay covered without the legacy 4:3 fudge. | Tightening to see culling pop, or widening to confirm tiles render - useful when chasing edge-case clipping at corners. |
| **Fog override** checkbox | Gates the next three controls. Off = camera config's fog wins. | Toggle while comparing current vs. tuned fog. |
| **Fog On / Off** | Enables/disables OpenGL fog. | Quick A/B for "is this fog or culling cutting things off?" |
| **Fog Start / End** (% of ViewFar) | Sets the fog density ramp as a fraction of the current far plane. | Pulling fog in to mask LOD pop, or pushing it out for cleaner long-range shots. |

#### Game Scene - Orbital Camera Override
Live-overrides `CameraConfig::ForMainSceneOrbitalCamera()`. The orbital
trapezoid is **seeded from the camera's natural pyramid on first enable**
so the first tweak doesn't snap.

| Control | What it changes | When you'd use it |
|---------|-----------------|-------------------|
| **2D Culling Trapezoid - far distance / far width** | Sets the far edge of the ground trapezoid (in world units). | Adjusting how far past the hero terrain stays loaded as the orbital camera pans. |
| **2D Culling Trapezoid - near distance / near width** | Sets the near edge. | Catching pop-in close to the hero on heavy zoom-in. |
| **Fog On / Off** | Enables/disables fog for the orbital config. | Quick A/B as with the default camera. |
| **Fog Start / End** (% of ViewFar) | Fog density ramp. | Same as default - orbital uses a much wider FOV (90°), so fog feels different. |

#### Debug Section

| Control | What it changes | When you'd use it |
|---------|-----------------|-------------------|
| **Character Pick Boxes** | Draws per-character pick volumes. | Debugging click-to-target or hover hit-testing. |
| **Item Pick Boxes** | Same for dropped items. | Debugging item pickup ranges. |
| **Item Cull Sphere** | Visualises the item cull radius around each dropped item. | Sanity-checking the **Item Cull Radius** slider's effect. |
| **Item Cull Radius** slider | Overrides `DEFAULT_CULL_RADIUS_ITEM` (400) for the active session. | Tuning how close you need to be before items render. |
| **Tile Grid** | Overlays a debug grid on terrain tiles. | Checking tile boundaries when investigating LOD or terrain bugs. |
| **Rendering: Terrain / Static Objects / Effects / Dropped Items / Weather / Item Labels** | Cuts each pass out of the frame entirely. | Isolating which pass owns a visual artefact, or measuring per-pass cost via `$details`. |

### 3.2 Graphics tab

Read-only diagnostics:

- Real-time **window** and **OpenGL viewport** dimensions.
- Screen-rate scale factors.
- Window mode (windowed / fullscreen).
- A warning if the viewport and window dimensions disagree.
- A **Copy debug info to clipboard** button - handy for bug reports.

---

## 4. What persists, what doesn't

DevEditor overrides are **runtime-only**. They live on the `m_DefaultOverride`
and `m_OrbitalOverride` member structs and are applied each frame
(`ApplyDefaultOverrideToConfig()` / `ApplyOrbitalOverrideToConfig()`).
Closing the client throws them away.

The only camera value that survives a restart is the orbital wheel-zoom
radius - see [Camera System §3](camera-system.md#what-persists).

If you want to keep a tuned set of values, copy them out of the DevEditor
and feed them into the relevant `CameraConfig::For…` factory in
`src/source/Camera/CameraConfig.h`.

---

## 5. File map

```
src/MuEditor/
  Core/
    MuEditorCore.h / .cpp        # editor lifecycle, F12 toggle wiring
    MuInputBlockerCore.cpp       # routes input to the editor when active
  UI/
    Common/MuEditorUI.h / .cpp   # shared editor chrome
    Console/
      MuEditorConsoleUI.h / .cpp # debug console / logging output
    DevEditor/
      DevEditorUI.h              # struct + interface
      DevEditorUI.cpp            # all panels, overrides, debug toggles
```
