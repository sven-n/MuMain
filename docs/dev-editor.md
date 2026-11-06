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
- Requires the ImGui submodule, which CMake fetches
  (`git submodule update --init --depth 1 -- src/ThirdParty/imgui` by hand).
- Not present in Release builds.

---

## 2. Opening it

Two ways:

- **Press F12** in-game.
- Start the client with `--editor` to launch with the editor enabled (or
  toggle later with F12).

While the editor is closed, an **Open Editor** button sits in the top-right of
the game window; **Close Editor** is on the right of the editor window's menu
bar.

The editor opens in a window of its own, beside the game window where the
screen has room (else over its right side), and stays in front of the game. Its
**Windows** menu opens the tool windows: Item, Monster
([monster-data.md](monster-data.md)), Colors ([item-data.md](item-data.md#colors)),
Data Checks ([below](#data-checks)), the Sound Mixer ([below](#sound-mixer)),
the Sound Editor ([below](#sound-editor)), Server List
([server-list.md](server-list.md)), Skill, Dev and Map Editor, the Effect
Browser and the console. They open as tabs docked in the editor window, the
console along the bottom. Drag a tab out to float a tool window anywhere, over
the game window too, or onto another window to dock it there; **Windows > Reset
Layout** docks them all back. The layout and the windows' places are kept
between sessions (`imgui.ini` in the game folder).

The editor window's bar has the **Windows** menu, **Undo** and **Redo**, the UI
scale and the language. Undo and Redo act on the tool used last: after a click
into the Server List window on its changes, after a click into a monster tool
on monster edits, after a click into an item tool on item edits. While the
tool used last is closed (for the monster tools: all of them), they act on
item edits. A window of tool windows floating outside the editor window has the
same bar once, on the tab shown at its top left when several are docked in it,
so it can be worked with from another monitor; its **Windows > Fullscreen**
switches that window to full screen.

The editor window, and a tool window floating outside the game window, are
ordinary windows of the system, with its title bar, buttons and window
snapping. Closing the editor window closes the editor. A floating tool window
keeps its tab under the title bar: drag the tab, not the title bar, to dock it
again. **Windows > Fullscreen** switches the editor window to full screen and
back; **F11** does the same for whichever of the editor's windows has the
keyboard (in the game window F11 stays the game's camera reset). Back from
full screen, a window returns to the place it had before. While a window is
full screen its places are not saved, so a session that ends then (killed, a
crash) keeps the window's place from before. On Windows a window whose saved
place covers its whole monitor (the title bar above the screen, from a
session before this was so) opens and returns in the monitor's usable area
instead; elsewhere the window manager keeps the title bar on the screen.

- **Windows:** minimize, maximize and close, and a taskbar button for each
  editor window. Dragging or resizing an editor window by its title bar or
  frame pauses the game until the mouse is released, as it does for the game
  window.
- **Linux:** editor builds run on X11 so that the editor's windows can leave
  the game window, through XWayland on a Wayland desktop (the game may look
  softer there on a scaled screen). The window manager draws the title bar and
  decides the buttons, the snapping and whether an editor window gets its own
  taskbar entry. `SDL_VIDEO_DRIVER=wayland` runs on Wayland instead, with the
  editor inside the game window.
- **macOS:** close, minimize and the green button (full screen, and tiling on
  macOS 15). The editor's windows float above the game while MU is the active
  app, and minimized ones are in the Dock and the Window menu. macOS may keep
  F11 for itself: the green button or Ctrl+Cmd+F switch full screen too.

With the game in full-screen mode, the editor's windows stay inside the game
window instead.

The editor is rendered with ImGui and consumes input over the game
(see `MuInputBlockerCore.cpp` for the input-routing logic). Keys go to the
window that has the keyboard: while one of the editor's own windows has it,
the game gets none of them (F12 still closes the editor from there). An
editor window inside the game window (full screen) takes the keyboard with a
click and gives it back with a click into the game.

While the editor has the keyboard, the arrow keys (and PageUp, PageDown,
Home and End) move through its lists and tables, and the row they move onto
is selected as a click would select it; Ctrl with an arrow only moves the
cursor. In the Map Editor's thumbnail grids they move in both directions.

### Data Checks

**Windows > Data Checks** lists the problems of all game data in one table:
the items, the monsters and the monster models as their Save would write
them now, and the glTF model file table, the server list, the effect
catalogue and the graphics look (the look profile and the light moods, area
**Graphics look**) as the game reads their files at start, and the map
object sounds as the Map Editor's Sounds tab has them
([map-data.md](map-data.md)).

Problems have three levels, the same in every area: **fatal** (the data
cannot be used: Save refuses it, and the game does not start with it; only
the graphics look starts anyway, with the built-in look values and the
Classic light),
**error** (something it names is missing, e.g. an item or a model file: the
game runs without it, Save keeps it) and **warning** (it works, but is worth
knowing). The worst come first. The boxes above the table show or hide each
level, with its count, and each area. **Check all** checks again; a change in
the item or monster tools does not, and a note says when the data changed
since the last check.

A click on a problem selects what it is about and opens its tool: the item
or monster tool of its field, Item Formulas for the item stat file, the
Server List, the Effect Browser, the Map Editor for the map sounds, or the
Dev Editor for the graphics look. An arrow
key onto a problem only selects it.

### Sound Mixer

**Windows > Sound Mixer** is for listening: it mutes, solos and turns down
types of sounds and single sounds while the game runs, in editor builds
only.

- **Types**: music, map ambience (wind, rain, water, forest), map object
  sounds ([map-data.md](map-data.md)), monsters (the sound sets' events,
  the attacks' sounds, [monster-data.md](monster-data.md), and their code's
  sounds), NPCs, your character, other players, effects, items, creatures
  (birds, bats, mice, fish), the interface and everything else. A sound's
  type is where it comes from: the data's players name it
  (`Audio::ScopedSoundSource`); else the place the game's code runs for
  (`Audio::ScopedSoundPlace`, `Audio/SoundPlaces.h`): a map object, a
  character by its kind (the hero is your character), the caster of a
  skill the server sends, an effect as its maker (your fireball is your
  character's, Charon's sparks the NPCs'), the windows and the interface's
  input, the map's ambience; else the object `PlayBuffer` is given (a
  character, a ground item, a bird or fish, a mount by its rider); else a
  loop is ambience and the rest is everything else. Each type has **Hear**
  (off mutes it), **Show** (off: Show in world draws nothing of it, but it
  is still heard), a volume slider and **Solo**. A sound two types play (a
  swing of yours and of another player) keeps each copy at the volume of
  the type it started as: muting other players leaves your own swing
  heard, also while one of theirs still plays.
- **Sounds heard**: one row per sound, in the order they were first heard,
  so that a row stays where it is (a music track too, with its file below
  `Data/Music`: its Mute, Solo and volume act on the music while that track
  plays): its file below `Data/Sound` (or its number), its kind and source (the set and monster, the map object type),
  how often it started (a loop or a map object's sound that is asked for each
  frame counts once per start), its length ("loop" for a loop) and when it
  last started ("playing" while it plays; the row is green then). **Clear
  all** empties the list, counts again from 0 and puts every sound's own
  Mute, Solo and volume back; the types' controls stay. A click on a column header sorts by it, a second
  one reverses, a third goes back to the order of first hearing. The filter
  bar above the table leaves the rows whose file or source contains a text,
  of one type, playing now or changed only, loops or one-shots, or played at
  least so often ("3 of 41 shown"). **Mute all** and **Unmute all** act on
  the rows shown, so a filter and Mute all silence, say, every Death Rider
  sound; sounds heard later come as new rows and play.

**Show in world** draws the sound sources over the game (for the session;
the menu bar reminder says "overlay on"). A map object with a sound
([map-data.md](map-data.md)) gets a square, its file and two rings on the
ground: full volume within 3 tiles (solid) and its quietest from 15 tiles
(dashed). A monster with a sound set gets a dot, the set's name and its
5-tile hearing ring, solid green while the hero is inside it (its sounds are
heard) and dashed grey outside; a set whose sounds are all heard anywhere
(Kanturu's, Aida's) has no ring and says "heard anywhere". A source flashes a growing ring for a second
when one of its sounds starts. The sounds that the game's own code plays have no data and so no
range (most maps; Elf Lala's harp, the Chaos Goblin's mix and Hanzo's smithing; your footsteps,
swings and skills, other players', effects, drops, birds): a hollow violet square (a map object)
or circle (anything else) with the files, on the line below a set's name at the same place, marks
where such a sound last started, while it plays and for a second after.
The sounds without a place (the interface, the map's ambience, music, some of the server's
messages) are logged at the top left of the game: the last 10 starts, newest on top, each with
how long ago it started, its file and type ("0.2 s  iButtonClick.wav (Interface)"); a new line is
yellow and turns white within a second.
Only the types whose Show box is ticked are drawn, and while a filter is set
only the sources and log lines of the rows shown (none set: everything in
view); the sources and log lines of the row under the mouse are drawn
yellow.

Each row has a volume slider, **Mute** and **Solo**. A sound plays at its
type's volume times its own; mute wins; while anything is solo, only what is
solo plays, the music too. Changes reach playing sounds at once, and a muted
loop keeps running silently, so it is back where it is when unmuted.
**Reset all** puts everything back (unmuted, not solo, full volume).
Nothing is kept: each start plays everything as normal, and while anything
is changed the editor's menu bar says so ("Sound Mixer: 2 muted, 1 solo"); a click opens the window.

The game's own volumes stay as they are, and the editor's previews (the Play
buttons of Monster Sounds and the Map Editor, Monster Preview, the effect
preview) play at their own volume, whatever the mixer says. The effect
preview's **Mute** still silences the game's one-shot sounds while it runs.

A sound selected in the Sound Mixer (a click on its Sound cell) is selected in
the Sound Editor too, and the other way round; a window that is closed or
behind another tab is not opened or raised, it shows the sound when you look
at it.

### Sound Editor

**Windows > Sound Editor** shows every sound the game loads and edits what
it is: its name, its file, its channels and 3D, its type, the folder it is
in, and which sound the game's code plays when. Every change reaches the game
at once; **Save** writes the files that changed, **Revert** reads them again,
**Undo** and **Redo** step through the changes. A monster set's sounds
changed here are written into the set in `Data/Monsters/Sounds.json`, under
the name the file has (a set renamed in Monster Sounds and not saved there
keeps its old name in the file): what the monster tools changed and did not
save, also in the same set, is not saved with them, and they say **Not saved
yet** until their own **Save**. A sound changed in both windows is left to
the monster tools' **Save**, and the status line says so.

- **Where sounds live.** The code's sounds, which C++ loads by their `SOUND_`
  id (`LoadGameSound`), take their file, channels and 3D from
  `Data/Audio/SoundFiles.json`; the monster sound sets' from
  `Data/Monsters/Sounds.json`, the map object sounds' from
  `Data/Maps/ObjectSounds.json`. `Data/Audio` and not `Data/Sound`: a build
  folder's `Data/Sound` links the shared client folder, which is not in git.
- **The Sounds tab** is a tree of folders with one row per sound: its name
  (edited in the cell; without one the id, or the set and file), the id or
  set in grey, the file, channels, 3D, the type if no place, the types it
  was heard as this session, when it loaded this session (at start, the map
  entered, with its monster, not loaded yet) and the file it lives in. A
  click on the file's **...** picks another file of `Data/Sound` (a search,
  and a play button to listen first); the files on disk are never changed.
  The play arrow at the start of each row (it stays with the names when the
  table scrolls sideways) plays the file once at full volume, past the Sound
  Mixer. Each column keeps its own width: dragging a border moves only that
  column.
  Search finds names, ids, sets and files; **Only problems** leaves the files
  that are not in the client folder, a file two code sounds load and the
  sounds the game played that nothing loads. The editor lists `Data/Sound`
  once, when it first needs it: **Read Data/Sound again** counts a file
  copied in since, in the picker and in the problems.
- **Names.** The `SOUND_` id stays a sound's identity, which C++ and the
  data point at; its name is only what the editor shows (the Sound Mixer,
  its overlay and log, the Monster Sounds event combo too), so a rename
  never breaks anything. Names are optional `"name"` fields in the three
  files; the game reads and ignores them.
- **Type if no place.** A code sound's `"type"` in `SoundFiles.json` is what
  the Sound Mixer counts it as when nothing names its source or place (a
  sound of the server's messages); who plays a sound still decides first.
- **Folders** (`Data/Audio/SoundFolders.json`, read only by the editor):
  folders in folders, each sound in one, else under **Not sorted**. **New
  folder**, **Rename**, **Remove** (its folders and sounds move up), **Move
  up** and **Move down** act on the folder clicked; drag a sound by its id
  onto a folder. A filter opens the folders that hold what it shows, a
  selection the ones that hold the sound. A folder names a set's sound by
  its set and its id or file, and a map object sound by its file: when one
  of those changes, here or in another window (a set renamed in Monster
  Sounds, a file picked in the Map Editor's Sounds tab), the sound keeps its
  folder. `SoundFolders.json` names the sounds as the saved files do: a
  rename another window has not saved stays out of it, and once a file is
  saved with new names (here, or that window's **Save**) the Sound Editor
  says **Not saved yet** until its **Save** writes them. An entry of a sound
  that is gone (removed in another window, or by hand in the file) stays, so
  the sound finds its folder when it comes back; the folder bar counts them,
  and **Drop them** takes them out as a step that **Undo** puts back.
- **The Events tab** (`Data/Audio/SoundEvents.json`) decides which sound the
  code plays when: for each event the first rule that matches plays one of
  its sounds (at random when it has more). Your footsteps (`heroFootstep`)
  match the world (`WD_*` numbers, "7, 24-29, 36"), the hero's ground tile
  and the safe zone; a player's swing at an attack's start
  (`playerAttackSwing`) matches what the player holds (a bow, a crossbow,
  the bow group's items 13 and 14, a Light Saber or Spear, another weapon,
  nothing). A click on a button (`interfaceClick`) matches its click kind,
  which the button's theme gives it (`click-sound: confirm`,
  [building-new-ui.md](rmlui-ui-system/building-new-ui.md)); a click on a
  character in the character select scene is `characterSelected`. An empty
  field matches anything; each event shows only the fields its code fills
  in.

---

## 3. Tabs

### 3.1 Scenes tab

#### Camera Mode controls
Switch between FreeFly and the active gameplay camera (Default / Orbital).
While FreeFly is active, the gameplay camera continues running in the
background (the "spectated camera" tracked by `CameraManager`) - toggling back
puts you exactly where the gameplay camera now is, not where you left off.

The standard in-game hotkeys still work with the editor open, while the game
window has the keyboard:

- **F9** cycles Default ↔ Orbital.
- **F10** toggles the zoom lock. F10 is intercepted at the Win32 message
  layer (`WM_SYSKEYDOWN`) so it fires reliably even when an editor window
  over the game has keyboard focus.
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
| **Rendering: Terrain / Static Objects / Effects / Dropped Items / Weather / Item Labels** | Cuts each pass out of the frame entirely. | Isolating which pass owns a visual artefact, or measuring per-pass cost via `$glstats`. |

### 3.2 Graphics tab

Read-only diagnostics:

- Real-time **window** and **OpenGL viewport** dimensions.
- Screen-rate scale factors.
- Window mode (windowed / fullscreen).
- A warning if the viewport and window dimensions disagree.
- A **Copy debug info to clipboard** button - handy for bug reports.

### 3.3 Map Editor tab

A tool window of its own (**Windows > Map Editor**, not one of the DevEditor's
own tabs) that opens an in-client editor for a map's terrain textures, terrain height,
walkability attributes, world objects and their sounds
([map-data.md](map-data.md)), plus a top-down minimap capture -
saving back to the game's own on-disk formats. See
[`src/MuEditor/UI/MapEditor/MAP_EDITOR.md`](../src/MuEditor/UI/MapEditor/MAP_EDITOR.md)
for the full usage guide (tabs, save targets, and the client/server attribute
merge workflow).

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
    MapEditor/
      MAP_EDITOR.md              # usage guide for the Map Editor (§3.3 above)
      MapEditorUI.h / .cpp       # tab UI + edit-mode dispatch
      Map*.h / .cpp              # per-tab logic: texture/object/attribute/height
                                  # save+load, minimap capture, thumbnails
```
