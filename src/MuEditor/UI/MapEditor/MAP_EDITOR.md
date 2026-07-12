# In-Game Map Editor (MuEditor)

An in-client editor to **view and edit a map's terrain textures, terrain height,
world objects, and to capture a top-down minimap screenshot** — saving back to
the game's own on-disk formats. It lives inside the `MuEditor` module and is
compiled only in editor builds (`ENABLE_EDITOR` / `_EDITOR`), so release builds
are unaffected.

- **Build:** `MuMain\rebuild_x64_mueditor.cmd` (or `Build Client with MuEditor.cmd`).
  Output: `out\build\windows-x64-mueditor\src\Release\Main.exe`.
- **Run:** `Main.exe --editor`, or press **F12** in-game to toggle the editor.
- **Open:** toolbar → **Map Editor**.

---

## 1. The core idea

The client already exposes, as **global arrays**, everything the editor needs,
and already ray-casts the mouse onto the terrain each frame **whenever the global
`EditFlag != EDIT_NONE`**. So the Map Editor is mostly:

1. Set `EditFlag` to the mode the active tab wants (`EDIT_MAPPING`, `EDIT_OBJECT`,
   `EDIT_HEIGHT`). The engine's terrain render pass then fills the picking globals
   `SelectXF`, `SelectYF`, `SelectFlag` (mouse-over tile) and `CollisionPosition`
   (world-space hit point) every frame.
2. On a captured mouse click over terrain, mutate the live global arrays.
3. On **Save**, write the arrays back to the map's real files (re-encrypting where
   the engine's own savers don't).

> **Note:** the legacy `Editor::EditObjects()` (`ZzzInterface`/`EditObjects.cpp`)
> that historically did the painting is gated on `#ifdef ENABLE_EDIT` — **not**
> `_EDITOR` — so it is **compiled out** of the editor build. The Map Editor
> therefore performs the actual painting/placement itself (reusing only the
> engine's picking + save functions).

---

## 2. Files

All under `src/MuEditor/UI/MapEditor/`:

| File | Responsibility |
|---|---|
| `MapEditorUI.h/.cpp` | The panel + all tabs (Texture, Objects, Height, T. Browse, Minimap, O. Browse). Owns UI state, edit-mode selection, input capture, and undo. |
| `MapEditorSave.h/.cpp` | `Editor::MapSave` — **encrypting** save of the terrain mapping (`.map`). |
| `MapTextureBrowser.h/.cpp` | **T. Browse** tab — preview a chosen world's tile textures; "Use"/"Upload" a texture onto the current map. |
| `MapTextureImport.h/.cpp` | `Editor::TextureImport` — put a `.jpg`/`.ozj` into a free `ExtTile` slot and load it live; Windows file dialog. |
| `MapObjectPlace.h/.cpp` | `Editor::ObjectPlace` — enumerate loaded models, place/reposition/remove objects, save `.obj`, undo snapshot/restore. |
| `MapObjectImport.h/.cpp` | `Editor::ObjectImport` — cross-map object import (copy `.bmd` + textures into the current map's `Object` folder, load live), plus preview-load into a scratch slot. |
| `MapObjectBrowser.h/.cpp` | **O. Browse** tab — thumbnail grid of any map's object models; import onto current map. |
| `ObjectThumbnail.h/.cpp` | `CObjectThumbnail` — renders a loaded `BMD` model to a cached GL texture via an offscreen framebuffer, for the object thumbnail grids. |

**Engine files touched (kept minimal, all editor-gated where possible):**

| File | Change |
|---|---|
| `MuEditor/Core/MuEditorCore.h/.cpp` | Added `m_bShowMapEditor`; render the panel each frame; call `CaptureInputForPainting()` in `Update()` (before the game consumes input). |
| `MuEditor/UI/Common/MuEditorUI.h/.cpp` | Toolbar **Map Editor** button + param. |
| `Render/Sprites/GlobalBitmap.h` | `RefreshCacheEntry(index)` — invalidate one quick-cache slot (see gotcha #4). |
| `Camera/FreeFlyCamera.h/.cpp` | Widened `MAX_PITCH` to `-2` (near straight-down); `SnapTopDown()`; `RotateYaw()`. |
| `Render/Terrain/ZzzLodTerrain.cpp` | `g_bMapEditorFullTerrain` flag; `RenderTerrain()` forces `ResetFrustrumBoundsFullTerrain()` when set. |
| `Scenes/MainScene.cpp` | Suppress the FreeFly frustum wireframe while `g_bMapEditorFullTerrain` is set (clean minimap shot). |

---

## 3. Tabs / features

### Texture (`EDIT_MAPPING`)
- Palette of the map's 30 tile slots (`Bitmaps[BITMAP_MAPTILE + 0..29]`), Layer 1
  (base) / Layer 2 (overlay) selector, brush size, overlay opacity, **dropper**
  (Alt-click or toggle) to pick the tile under the cursor.
- Paints `TerrainMappingLayer1/2[]` and `TerrainMappingAlpha[]`. One-level **undo**
  snapshots those arrays at stroke start.
- **Save** → `Editor::MapSave::SaveMappingEncrypted()` → `Data\World{N}\EncTerrain{N}.map`.

### T. Browse
- Scan every `Data\World*` folder, preview all `Tile*/ExtTile*/AlphaTile*` textures
  (loaded into a high scratch bitmap range).
- **Use selected / Upload image** → `Editor::TextureImport` copies the texture into
  the current map's next free `ExtTile` slot (`ExtTileNN.OZJ`), loads it live, and
  selects it for painting.

### Objects (`EDIT_OBJECT`)
- **Place new:** thumbnail grid of the map's loaded models; yaw/scale/snap; left-click
  drops an object at the cursor (`Editor::ObjectPlace::Place` → `CreateObject`).
- **Select & edit:** left-click ray-picks a visible object (`CollisionDetectObjects`);
  drag to move (grab-offset so a click doesn't jump it); Pos/Angle/Scale fields;
  **Delete**. Moving re-blocks the object if it crosses a 16×16 grid cell.
- **Undo** snapshots all objects before each edit. **Save** → engine `SaveObjects`
  → `Data\World{N}\EncTerrain{N}.obj` (already encrypted).

### O. Browse
- Thumbnail grid of any map's object models (loaded on demand into a scratch slot
  `Models[MAX_MODELS]`, budget-throttled; failed ones show "n/a").
- **Use selected on current map** → `Editor::ObjectImport::UseModelOnCurrentMap`
  copies the `.bmd` + its textures into `Data\Object{current}\` at a free slot,
  loads it live, and jumps to Objects → Place. Persists on reload for **generic**
  maps (special maps like Lorencia/World1 that don't use the `Object{N}.bmd` scheme
  are live-only).

### Height (`EDIT_HEIGHT`)
- Left-click raises, right-click lowers `BackTerrainHeight[]` via `AddTerrainHeight`
  (brush radius + strength), then `CreateTerrainNormal()` + `CreateTerrainLight()`
  to update visuals. One-level **undo**.
- **Save** → engine `SaveTerrainHeight` → `Data\World{N}\TerrainHeight.OZB` (a plain
  BMP the game reads directly).

### Minimap (top-down capture)
- **Top-down view** switches to the FreeFly camera, snaps it above the map centre
  looking straight down (`SnapTopDown`), and enables full-map rendering:
  `g_bMapEditorFullTerrain` (full terrain bounds) **and** `g_Camera.TopViewEnable`
  (the engine's own minimap-capture flag that bypasses per-tile terrain **and**
  object culling).
- **Rotate 90 CW/CCW** to orient. Arrows pan, PgUp/PgDn height, RMB tilt, Shift fast.
- F12 to hide the editor, screenshot, crop, save as the map's `mini_map.OZT`.
- **Important:** these flags are only written while the mode is active and released
  **once** on exit — never every frame — because the game's own minimap uses
  `TopViewEnable` too (see gotcha #5).

---

## 4. On-disk formats & save encryption

Files live in `Data\World{N}\`. **World folder = world enum value + 1**
(`iMapWorld = WorldActive + 1`; a few maps remap — see `MapManager`). Ciphers:
`MapFileDecrypt`/`MapFileEncrypt` (`ZzzLodTerrain.h`), `BuxConvert` (`_crypt.h`).

| Data | File | Format | Editor save |
|---|---|---|---|
| Tile mapping | `EncTerrain{N}.map` | `mapdecrypt` only. `version, mapNum, Layer1[256²], Layer2[256²], Alpha[256²]` | **We re-encrypt** (`MapEditorSave`) — the engine `SaveTerrainMapping` writes **plaintext**. |
| Walkability | `EncTerrain{N}.att` | `mapdecrypt` + `bux` | (attribute painter not built yet — would also need re-encrypt) |
| Objects | `EncTerrain{N}.obj` | `mapdecrypt`. `version, mapNum, count, {Type i16, Pos[3] f32, Angle[3] f32, Scale f32}×count`. **Flat list; re-blocks by position on load.** | Engine `SaveObjects` **already encrypts** — call directly. |
| Height | `TerrainHeight.OZB` | Plain **BMP**: 1080-byte header + 256×256 grayscale; `height = byte × 1.5`. Loader appends `OZB` to the `.bmp` name. | Engine `SaveTerrainHeight` (plain BMP) — call directly. |
| Minimap | `mini_map.OZT` | 4-byte header `00 00 02 00` + 18-byte TGA header + BGRA pixels, **bottom-origin**, 32-bit, **no footer**. 1024×1024 for Arena. | Wrapped externally (strip 4 bytes → TGA to edit; prepend them back → OZT). |

**Tile slots:** a mapping cell stores an index 0–29 → `BITMAP_MAPTILE + index`.
Slots 0–13 are the fixed `Tile*` set, 14–29 are `ExtTile01..16` (the spare slots the
importer uses). Textures are stored as `.OZJ`/`.OZT` but referenced by `.jpg`/`.tga`
names — `LoadBitmap` swaps the extension.

**Object models:** generic maps use `Object{i}.bmd` (model `Type = i-1`, zero-padded
for i<10); some maps (Lorencia) use named files via a hand-coded model table. The
model + its textures live in `Data\Object{N}\`.

---

## 5. Gotchas learned the hard way

1. **Legacy painter is compiled out.** `Editor::EditObjects()` is `#ifdef ENABLE_EDIT`,
   not `_EDITOR`. The Map Editor does its own painting/placement.
2. **Mouse *held* state.** The game's `MouseLButton` is set on the Windows press/release
   *edge*, and the editor clears it every frame to suppress walking — so it only reads
   `true` on the press frame. For click-**drag** (paint strokes, object drag), read the
   held state from **`ImGui::IsMouseDown()`** instead (see `CaptureInputForPainting`).
3. **Save encryption differs per format.** `.map`/`.att` engine savers write **plaintext**
   → the editor must re-encrypt (`MapFileEncrypt`, plus `BuxConvert` for `.att`).
   `SaveObjects` (`.obj`) and `SaveTerrainHeight` (`.OZB`) are fine as-is.
4. **Quick-cache invalidation.** `Bitmaps[]` uses a quick-cache that caches even *empty*
   slots (the palette polls all 30 every frame). After `LoadBitmap`-ing into a slot that
   was polled while empty, call `Bitmaps.RefreshCacheEntry(index)` or the new texture
   won't appear until a restart.
5. **Don't clobber shared globals.** `g_Camera.TopViewEnable` is used by the game's own
   minimap. Writing it every frame (even `false`) crashes the game minimap. Only write
   editor-owned globals while the editor owns the mode; release once on exit (edge-detect).
6. **World off-by-one.** `Data\World{WorldActive+1}\`. Special maps remap (Blood Castle,
   Chaos Castle, Hellas, Cursed Temple…). The Texture tab exposes a manual world override.
7. **MSVC comment trap.** A `//` comment ending in a backslash (`...Data\`) continues onto
   the next line and silently eats it. Bit us twice — don't end comments with `\`.
8. **FBO functions aren't linked.** The engine never used framebuffer objects, so glew's
   FBO entry points are unresolved and `glewInit` isn't called. `ObjectThumbnail` loads
   the 7 FBO functions itself via `wglGetProcAddress`.
9. **Asset-copy mtime gating.** `CopyAssets` is stamp-gated by source mtime, so editor
   Saves land in the **build output's** `Data`, not `src\bin`. To persist / share with the
   x86 build, copy the saved file back to `src\bin\Data\...` (or delete the
   `.assets_copied_Release.stamp` to force a re-copy if the output `Data` is deleted).
10. **FBO thumbnail render** needs the global `BoneScale = 1` and the sequence
    `Animation → Transform → RenderBody`; display the texture with a **vertical UV flip**
    (FBO is bottom-up vs ImGui top-down).

---

## 6. Adding a new tool/tab (recipe)

1. Add a `RenderXxxTab()` to `CMapEditorUI` and a `BeginTabItem("Xxx")` call.
2. In that tab, set `m_desiredEditFlag = EDIT_XXX` when your tool is enabled — the panel
   applies it after the tab bar (and releases to `EDIT_NONE` otherwise).
3. Do the actual mutation from the captured mouse state (`m_PaintLDown/RDown`) when
   `SelectFlag` is set and the cursor isn't over the UI, using `SelectXF/SelectYF` (tile)
   or `CollisionPosition` (world point).
4. Snapshot for undo at stroke start; add a Save that writes the correct (re-encrypted)
   file.

**Not yet built:** the **Attribute / walkability painter** (`EDIT_WALL`, toggling
`TerrainWall[]` bits `TW_SAFEZONE 0x01`, `TW_NOMOVE 0x04`, `TW_NOGROUND 0x08`, with an
overlay tint, saving the re-encrypted `.att`). It's the natural next tab and fits this
exact recipe.
