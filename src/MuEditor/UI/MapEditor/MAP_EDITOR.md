# In-Game Map Editor (MuEditor)

An in-client editor to **view and edit a map's terrain textures, terrain height,
walkability attributes, and world objects, and to capture a top-down minimap
screenshot** — saving back to the game's own on-disk formats. It lives inside the
`MuEditor` module and is compiled only in editor builds (`ENABLE_EDITOR` /
`_EDITOR`), so release builds are unaffected.

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
| `MapAttributeSave.h/.cpp` | `Editor::AttrSave` — **encrypting** save of the walkability `.att` (client) **and** the plain `.att` for OpenMU's Admin Panel + a HOWTO text file (server). |
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
| `Render/Terrain/ZzzLodTerrain.cpp` | `g_bMapEditorFullTerrain` flag; `RenderTerrain()` forces `ResetFrustrumBoundsFullTerrain()` when set. Also `g_bMapEditorAttrOverlay` + `RenderAttributeOverlay()` (tints tiles by their `TerrainWall` bits). |
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
- **Height is capped at `255 * factor`** (factor 1.5 normally, 3.0 on the login scene):
  the file stores one byte per cell as `height / factor`, so anything higher can't be
  saved and collapses to a flat plateau on reload. The sculpt clamps `BackTerrainHeight`
  to that ceiling each stroke, so the editor never shows a height that won't persist.

### Attribute (`EDIT_WALL`) — walkability
- Paints `TerrainWall[]` (**sets**, not ORs, the value — same as the standalone
  `_upscale/terrain_editor.html` tool). Left-click paints the selected attribute,
  right-click resets to Walkable. Brush size + one-level **undo**.
- Brushes are the **byte-sized** `TW_*` bits the `.att` stores: Walkable `0`
  (= normal combat area), Safezone `1`, Blocked/no-go `4` (`TW_NOMOVE`), NoGround
  `8` (`TW_NOGROUND`), Water `16` (`TW_WATER`).
  > `TW_NOATTACKZONE` (`0x100`) and friends **don't fit in a byte** and so can't be
  > stored in the byte-per-cell `.att` these maps use — don't add them as brushes.
- **In-world overlay** (`g_bMapEditorAttrOverlay` → `RenderAttributeOverlay()`)
  tints each tile so you can read the walk map while painting: red = blocked,
  green = safezone, black = void, blue = water, faint grey = walkable.
- **Two saves are needed — the client and the server each keep their own walk map:**
  1. **Save client .att** → `Data\World{N}\EncTerrain{N}.att` (see the format table).
  2. **Save server .att** → `terrain_map{E}_server.att` **+ a `_HOWTO.txt`** next to
     `Main.exe`. Upload it in the Admin Panel's **"Terrain Data"** field for that map,
     then **restart the game server**.
- ⚠️ **The two map numbers are different.** The server keys maps by `"Number"` = the
  **world enum** (Lorencia `0`, Arena `6`); the client folder is `World{enum + 1}`
  (`World1`, `World7`). The tab prints both side by side — edit the Admin Panel map
  whose `Number` matches the one shown, not the client folder number.
- If client and server disagree, players walk through walls and get rubber-banded
  back. Always ship both.

**Applying the server file**: Admin Panel → map list → edit the map with that
`Number` → **"Terrain Data"** → upload `terrain_map{E}_server.att` → Save → restart
the game server.

> ⚠️ **Never upload the client's `EncTerrain{N}.att` to the Admin Panel.** See gotcha 14.

### Minimap (top-down capture)
- **Top-down view** switches to the FreeFly camera, snaps it above the map centre
  looking straight down (`SnapTopDown`), and enables full-map rendering:
  `g_bMapEditorFullTerrain` (full terrain bounds) **and** `g_Camera.TopViewEnable`
  (the engine's own minimap-capture flag that bypasses per-tile terrain **and**
  object culling).
- **Controls (top-down):** all four **arrows pan** the map in the ground plane, the
  **mouse wheel zooms** (PgUp/PgDn also zoom), Shift pans faster. A dedicated
  `m_bTopDownPan` branch in `FreeFlyCamera` handles this — the normal fly-along-forward
  would just dive into the terrain when looking straight down. The wheel is read from
  **ImGui's** wheel in the editor (the game HUD consumes the global `MouseWheel` earlier
  in the frame than the camera update runs).
- **Rotate 90 CW/CCW** to orient the shot.
- **Capture minimap to file** writes the map's `mini_map.OZT` (drop-in) **and** an
  editable `mini_map.tga`, both 1024×1024, to `Data\World{N}\` and next to `Main.exe`.
  No F12 needed: the read happens inside the editor's per-frame `Render`, *before* ImGui
  draws (`ImGui::Render` is the last thing `CMuEditorCore::Render` does), so the back
  buffer still holds the clean top-down scene. `FreeFlyCamera::ComputeMapScreenRect`
  projects the four map corners to find the square to `glReadPixels`; it's resampled to
  1024² and wrapped (see the Minimap format row). Aspect-correct projection makes the map
  square on screen, so no stretching.
- **Important:** the render flags are only written while the mode is active and released
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
| Walkability (client) | `EncTerrain{N}.att` | Decode = `bux(mapdecrypt(f))` → `[version, mapId, 255, 255] + attr[256²]`, **1 byte/cell**, index `y*256+x` (same order as `TerrainWall[]`). | **We re-encrypt**: `MapFileEncrypt(BuxConvert(plain))` (`MapAttributeSave`). The engine `SaveTerrainAttribute` writes **plaintext AND 2 bytes/cell** — unusable. |
| Walkability (server) | `terrain_map{E}_server.att` | The **exact bytes** of OpenMU's `config."GameMapDefinition"."TerrainData"` (bytea): `[0, 255, 255] + attr[256²]` = **65539 bytes, NOT encrypted** (attr `i` at offset `3+i`). Same layout as OpenMU's own `Resources\Terrain{N}.att` seed files. Row keyed by `"Number"` = world **enum**. | Written verbatim (`SaveServerAtt`) — no `BuxConvert`, no `MapFileEncrypt`, and **no `mapId` byte** in the header. Upload via the Admin Panel's **"Terrain Data"** field. |
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
   **And the server disagrees with the client:** OpenMU keys maps by `"Number"` =
   the raw world **enum** (Lorencia `0`, Arena `6`), while the client folder is
   `enum + 1` (`World1`, `World7`). Mixing them up is the #1 way to write the wrong map.
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
11. **`TerrainWall[]` is not pure map data — never persist it raw.** `MoveCharactersClient()`
    clears and re-stamps `TW_CHARACTER` (`0x02`) **every frame** on every tile a live
    character/NPC occupies. Saving verbatim bakes "an NPC stood here when I hit Save" into
    the map (a safezone tile becomes `1|2 = 3`). The server then reads `3` as **blocked**
    (see #12) while the client still reads it as walkable → **rubber-banding**. `MapAttributeSave`
    masks the bit out (`StaticAttribute()`); any new attribute save must do the same.
12. **Client and server interpret the attribute byte DIFFERENTLY.** Byte-identical is *not*
    behaviour-identical:
    - **Server** (`GameMapTerrain.cs`, exact value): `walkable = (v==0 || v==1)`,
      `safezone = (v==1)`. So **water `16` blocks**, and *any* combination (`3`, `5`, …) blocks.
    - **Client** (`_define.h`, bit test): `blocked = v & 4 || v & 8`, `safezone = v & 1`.
      So `16` does **not** block and `3` is walkable.

    Paint clean single values (`0`/`1`/`4`/`8`) and the two agree. `_upscale/server_att.py`
    can dump the server's walk map, **diff** it against a client `.att`, and export a
    behaviour-faithful `.att` (`--normalize`).
13. **The `.att` loader has anti-tamper sentinels.** `OpenTerrainAttribute()` hard-checks one
    magic tile per map and calls `ExitProgram()` ("data error", client closes) on mismatch:
    Lorencia `(135,123)=5`, Dungeon `(227,120)=4`, Devias `(208,55)=5`, Noria `(186,119)=5`,
    Lost Tower `(193,75)=5`. It also rejects any byte `>= 128`. Anything that rewrites a `.att`
    must preserve these (`server_att.py` does, and validates before writing).
14. **The Admin Panel's "Terrain Data" upload stores the file VERBATIM.** It is
    `ByteArrayField.razor`, a *generic* `byte[]` widget — it does **not** decrypt and it does
    **not** fix up the header (`CurrentValue = memoryStream.ToArray()`, that's the whole
    handler). So **never** upload the client's `EncTerrain{N}.att` to it. That file breaks it
    two ways: it is **encrypted** (the server would read ciphertext as walkability flags), and
    its plaintext header is **4 bytes** (`version, mapId, w, h`) against the server's **3**
    (`version, w, h`) — which alone would shift the entire map by one tile, since
    `GameMapTerrain` does `ReadTerrainData(terrainData.AsSpan(3))`. Upload
    `terrain_map{E}_server.att` (from **Save server .att**) instead: it is written in exactly
    the server's layout.

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

The **Attribute** tab is the cleanest end-to-end example of this recipe (edit mode +
overlay + undo + two save targets).

**Prior art:** `_upscale/terrain_editor.html` and `_upscale/terrain_tool.py` are the
standalone (out-of-game) versions of the attribute editor — same crypto, same byte
layout, same colours. Useful for bulk/scripted edits and for cross-checking the
in-game tool.
