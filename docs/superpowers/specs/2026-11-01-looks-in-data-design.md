# Looks in Data (Items Phase 13) Design (WIP)

> **Draft, 2026-11-01.** Items phase 13 of the
> [items design](2026-09-25-data-driven-items-design.md) and decision D25 of
> the [data-driven content roadmap](2026-09-29-data-driven-content-roadmap-design.md).
> Written from the code of MuMain `upstream/main` @ `f81fb1b2c`. Line numbers
> are those of that commit. sven-n/MuMain#915 (open) changes the Magic
> Gladiator styles; where this design depends on it, it says so.
>
> This document is temporary. Its last task deletes it, after what stays true
> has moved into `docs/item-data.md`, `docs/effect-data.md` and the code (see
> "Tasks").

## Goal

What a named item look is made of moves from code into data, with a look
editor in MuEditor (D25). Each part (mesh) of a model can get its own layers:
a chrome layer on the hilt only, a glow on the blade, and so on. The Item
Model window gets a list of the model's meshes, so a mesh number can be
picked knowing which part it is.

As before for the items: nothing changes in game when a code look moves into
data, and each move is compared with the old code before the old code goes.

## Scope

- **In:** the item render styles (`renderStyle`, 140 names in
  `ItemRenderStyles.cpp`) and the item effects (`itemEffect`, 25 names in
  `ItemEffects.cpp`), all of them, in three steps (L7); layers per mesh; the
  mesh list; the look editor; FX1.7c, the live preview of effects in an
  editor view, which the look editor needs for things on bones.
- **Later:** monster and NPC looks (MN8) use the same format (D25: one look
  format for all areas); effect behavior as data is FX2, which the looks
  don't need: they create the existing effects with their FX1 names.

## Decisions

Made while reviewing sven-n/MuMain#915, 2026-11-01.

| D | Topic | Decision |
|---|---|---|
| L1 | Texture scripts | `_H` (hidden mesh), `_R` (glowing mesh) and `_DC` / `_DT` (shadow pass) are applied in one place that `RenderBody` and the look drawing share. Today only `RenderBody` applies them (`ZzzBMD.cpp:2650-2671`). |
| L2 | Glowing meshes and chrome | A glowing mesh (a `_R` texture, or the object's blend mesh) gets a chrome layer added over it, as on main. None-blend meshes takes the chrome off a mesh. |
| L3 | Control per part | Through the meshes of each layer, not through a separate field for the chrome styles. |
| L4 | Mesh list | The Item Model window lists the model's meshes under Model file. |
| L5 | No render style per mesh | A style draws the whole model, so a mesh gets layers, not a style of its own. |
| L6 | This document | Deleted by its last task, after its lasting content has moved into the docs. |
| L7 | Building blocks (Q-L1, 2026-11-01) | All of D25's blocks for the item looks, in three steps. **Step 1:** draw passes (texture, chrome, metal, glow, dark; pulse, scrolling, light, tint) and conditions (doppelganger, inventory and ground, item level, which item). **Step 2:** FX1.7c, the live preview of effects in an editor view, with its own short design first (see the possible follow-up ideas). **Step 3:** things on bones (sprites, particles, lightning; chances per frame), named as FX1 decided for phase 13 (`{ "joint": "jointThunder", "subType": 14, "fromBone": 3, "toBone": 4 }`, SubType a number, D30), shown live in the look editor. |
| L8 | Names of a layer's fields (Q-L2a, 2026-11-01) | Designer terms, with no part for raw engine values (one could be added later without changing data). **Kind:** texture, reflection, light map, flat color. **Motion** (reflections): slow drift (`RENDER_CHROME`), fast sweep (`CHROME2`), follows the light (`CHROME3`), swirl (`CHROME4`), strong swirl (`CHROME5`), stripes (`CHROME6`), very slow drift (`CHROME7`), oil sheen (`OIL`), still shine (`METAL`). **Blend:** normal, glow (`RENDER_BRIGHT`, adds light), darken (`RENDER_DARK`), as the drawing asks (the drawing's own render type). **Meshes:** empty for all. **Where** (reflections): over, or under the holes (as #915 draws it). **Texture:** empty for the mesh's own, or the one that belongs to the motion. **Color:** the model's light, a fixed color, or a tint of the model's light. Kind, Motion and Blend combine freely; an engine feature with no field (e.g. `RENDER_NODEPTH`) gets one added in code. Opacity, the glow mesh and scrolling: L10. |
| L9 | Values that change over time (Q-L2b, 2026-11-01) | A field that changes over time names a shape whose parameters a designer can picture: **fixed**; **wave** (from, to, seconds per cycle); **bounce** (the absolute sine: from, to, seconds); **scroll** (per second); **ramp** (from, to, seconds, then back to the start); **frames** (count, step, seconds each); **random** (from, to, step, a new value each frame). The loader turns them into the code's numbers; a moved look keeps its exact values (e.g. 6.283185 s for `sin(WorldTime * 0.001f)`), checked against the old code. The editor draws the value over time beside the field. No formula text (D43: building blocks with parameters, not code transcribed). Chances per frame belong to things on bones (L7 step 3). |
| L10 | Glow mesh, scrolling and opacity (Q-L2b, 2026-11-01) | **Fields of each layer**, empty for the item's value: glow mesh (none, a mesh, or all meshes), glow strength, scrolling mesh, scrolling (U and V), opacity (the item's, the drawing's, a number or a shape). **Item values of the look:** the glow mesh, glow strength, scrolling mesh, scrolling, opacity and hidden mesh it leaves on the item for what is drawn after it (the +7 glow, the excellent glow), as `masterScepter` leaves glow mesh 1 (`ItemRenderStyles.cpp:338-352`). The 11 item effects that only change values become looks with item values and no layers. When item values apply: L13 (an item effect's before the layers, a render style's after them). |
| L11 | Conditions (Q-L2c, 2026-11-01) | **On the look, "Not for":** the drawings the look is not for, which the game then draws plainly, as a style returning false does today: doppelgangers, and outside a monster's hands (`monsterBattleBow`). **On a layer, "Only when":** worn, in the inventory and on the ground (the skin hidden), an item level range, the item's hidden mesh, a buff on the character. "Which item" is no condition: values chosen per item are Q-L3. Conditions found in the survey that fit neither list (the light being exactly white, the turning angle): Q-L5. |
| L12 | Where looks live (Q-L3, 2026-11-01) | **Named looks** in a look file in `Data/Effects/`, which model entries name (D25), with D26's rules: the editor shows which items use a look, saving a look that others use first lists them, a copy makes a variant for one user, renaming updates the references, a look still in use can't be deleted, names are resolved when loading. **Values per item:** in the Style Editor a field can be ticked "Each item sets this", with a name and the look's value as default. Every item using the look shows those fields under "Values for this item" in its Item Model window, stored in its model entry; Reset goes back to the look's default. Everything else is shared, so a change to the look reaches all its items and keeps their own values. This covers a color per item (the 30 Seed Spheres), meshes per item (the Lucky items, the Violent Wind pieces), and chrome on chosen parts of one item (e.g. the Divine Sword using `thunderHawk` with its chrome meshes set to 1). The Item Model window keeps its **Render style** dropdown for picking the look (a new "Look" label was found confusing); the field name in the data and the look file's name: Q-L4. |
| L13 | Render style and item effect (Q-L4a, 2026-11-01) | **Two looks per item, with fixed roles**, as today's two fields: at most one render style and one item effect. A **render style** look draws the item: its layers, its "Not for" and the item values it leaves after drawing. An **item effect** look does what comes before the drawing: item values before drawing, and things on bones. The Style Editor offers only the parts of a look's kind, so two looks can never both draw. Mixing is a field on the item (Wing of Ruin can pick the item effect `wingsOfDarkness`), and an edit to a look reaches every item that uses it (D26). Render styles and item effects stay two lists, so a name can be in both (`wingsOfDarkness`). This answers L10's open point: an item effect's item values apply before the layers, a render style's after them. The 7 item effects that draw have layers, so they become render styles: 6 draw the model themselves (`darkLordScroll`, `fruits`, `spirit`, `firecracker`, `gmGift`, `meshesPerLevel`), and `billOfBalrog` draws an extra pass before the usual drawing (found in the survey, Q-L5). One look per item (its parts copied when mixing) and one look whose parts can be linked to other looks were weighed: both need a new look for every mix, and copies don't get later edits. Compared with linked parts in five tasks, two looks are quicker to combine (one dropdown), keep today's data unchanged and are less to build; linked parts only read better (one window per item). |
| L14 | The move from code to data (Q-L4b, 2026-11-01) | **Same names, data first.** When loading, a name is looked up in the look file first, then in the code list; a name in neither is reported and drawn plainly, as today. Model entries never change during the move. Each pull request moves a group of looks: it adds them to the look file under their names, a test draws their items with the old code and with the data in the same build and compares the records (as FX1's moves did), and then it deletes their functions and code names; a test checks that no name is in both. The editor marks looks that are still code ("(code)" in the dropdowns); they can be picked but open read-only in the Style Editor until moved. New names for data looks (every model entry changed twice) and one move of all looks at once (one very large pull request) were weighed. |
| L15 | The look files (Q-L4c, 2026-11-01) | **One file per kind** in `Data/Effects/`: `RenderStyles.json` and `ItemEffects.json`, named like the fields `renderStyle` and `itemEffect` and the two dropdowns. Built like the effect catalogue files next to them: a format version, the kind, and a list sorted by name, which the Style Editor writes back sorted so a change stays a few lines. Looks are shared across item groups, so they are not split by group like the model files. When monsters and NPCs get looks (MN8), they use the same files (D25); renaming the "item effect" kind for them is the monsters design's call. One file for both kinds and one file per look (165 files) were weighed. |
| L16 | Looks that don't fit (Q-L5a, 2026-11-01) | **Real features become blocks:** a value that is the item's value times a shape (the four Phoenix Soul items, `cursedCastleWater`); the model's light at a fixed brightness (`invisibilityCloak`); a value from the item's turning angle (`gambleItem`); "in the inventory" as a place condition next to L11's "inventory and ground" (`capeOfEmperor`, `smallCapeOfLord`, which test for exactly white light today; the pull request checks which drawings have white light); a zigzag shape, straight up and down (`symbolOfKundun`); the model lit by the scene light (`firecracker` sets `b->LightEnable`). **Bug-like quirks are fixed while moving**, each in its own commit with a before/after in game, not copied with blocks of their own: `wingOfStorm` and `symbolOfKundun` count their animation per drawing in a shared `static` counter, so it speeds up with every one on screen (moved: counted in time); `bloodBone` scrolls by `sin(map number × 0.0001)` instead of the clock (moved: no offset, under 1/100 of the texture); `runeBlade`, `elementalMace` and `grandSoulShield` make a grey from the red channel of the model's light only (moved: the light's tint, the same under white light). Copying the quirks exactly, and leaving these looks as code, were weighed. |
| L17 | The order of moving (Q-L5b, 2026-11-01) | **By building block**, so each pull request brings one kind of block and moves the looks that need exactly it: (1) the machinery (loader, layer drawing, comparison test, Style Editor basics) with the 36 render styles of plain passes; (2) light and time, 67 render styles; (3) conditions and values per item, 20 render styles, among them the Magic Gladiator sets after #915 is merged (reflection under the holes); (4) the item effect kind, 16 item effects (9 that set values, 7 that draw become render styles); (5) the additions and quirk fixes of L16, 14 render styles. Then step 2 (FX1.7c) and step 3 (3 render styles, 9 item effects). The counts come from the survey script; each pull request's comparison test shows the exact list. The split into pull requests, with the mesh list and the editor parts: Q-L10. The Magic Gladiator sets as a first pilot, one pull request per item group, and the most-used looks first were weighed. |
| L18 | Where the Style Editor lives (Q-L6a, 2026-11-01) | **A window of its own**, Windows > Style Editor, next to the Effect Browser, for the items now and the monsters later (D25: one look editor for all areas). Built like Glow Colors (`MuItemGlowColorsUI.cpp`): tabs Render styles and Item effects; a button row (New look, Copy, Rename) and a grey note; on the left a resizable list with each look's name, how many items use it, and "(code)" for looks not moved yet; on the right the picked look, Apply to the look / Revert / Remove, "Used by N items:" with the list of items, and the confirm that lists them (D26). The Item Model window keeps its two dropdowns, "Values for this item" and gets "Open in Style Editor". An item tool under Windows > Items, and editing inside the Item Model window, were weighed. |
| L19 | The layer list (Q-L6b, 2026-11-01) | **A list, then the picked layer.** The look's layers in drawing order, one line each written from its fields ("texture, mesh 0, glowing blue, pulsing"), with + Add, Copy, up, down and Remove above the list, and "Item values after drawing" (or "before drawing" for an item effect) as the last line. Clicking a line shows its fields below: the fields of L8 to L11, the value graph beside a field that changes over time (L9), and the "Each item sets this" box (L12). A collapsing section per layer, and a table with a column per field, were weighed. |
| L20 | The Style Editor's preview (Q-L6c, 2026-11-01) | **The Item Model preview with helpers for designing.** As in the Item Model window (`ItemModelPreview.h`): the look shown on an item (any of its users, or any other item to try it on), at an item level, excellent or ancient, in the inventory pose or on the ground, turning, with the same camera; changes show before they are applied. Added: stopping time and moving it with a slider (the clock the looks read while the preview draws), an eye per layer to hide it or show only it, and "Draw as: doppelganger" to see "Not for". Things on bones show here after FX1.7c (L7, step 2). **Later, on top:** a pose worn by a character in an animation (no preview draws a character with equipment yet). |
| L21 | The mesh list's rows (Q-L7a, 2026-11-01) | Under Model file in the Item Model window, after Texture folders and None-blend meshes: **one row per mesh with its number, its texture name, marks and "used by"**. Marks: hidden (a "hid…" texture name or an `_H` script: never drawn), glows (`_R`), scrolls (`_S`), no chrome (`_N` or None-blend meshes), holes (the load-time flag asked for in #915; added by the first pull request of the move if #915 doesn't). "Used by": the fields that name the mesh (None-blend, glow, shine and excellent meshes) and the layers of the item's looks that draw it ("Render style textured: layer 1, all meshes"); a look that is still code says so until its move. A tooltip shows the texture's size and the folder it was found in, and the mesh's vertices and triangles. Number and texture only, and a wide table with every column, were weighed. |
| L22 | Highlighting a mesh (Q-L7b, 2026-11-01) | **Pointing at a mesh row, or at a number in a mesh field** (None-blend meshes, glow meshes, a layer's Meshes in the Style Editor), **draws the other meshes see-through** in the preview, so the pointed mesh stands out as it looks in game and a part behind another shows (editor builds only); the pointed mesh itself gets no highlight color. A click keeps it so until clicked again; without a pointed mesh the item is drawn as in game. **Clicking a part in the preview selects its row** and its number in the mesh field being edited (new code: the meshes drawn with ID colors into a hidden picture, the pixel under the mouse read back). Works in the Item Model preview and the Style Editor's preview. A mesh that is never drawn can't be shown this way; its row says so. A highlight color drawn on top of the mesh, with or without the others dimmed, and no highlight at all, were weighed. |
| L23 | Data checks (Q-L8, 2026-11-01) | **The project's three levels** (`docs/item-data.md`, "Loading and errors"; `docs/dev-editor.md`, Data Checks). **Fatal** (Save and Apply refuse, the game does not start): invalid JSON or `formatVersion`, a file of the wrong kind; a look without a name, a name that is not letters and digits, a name twice in a file; a value of the wrong type or out of range (seconds of 0 or less, a frame count of 0); a kind, motion, blend, shape, condition or place that does not exist; a part the look's kind does not have (layers in an item effect, bone effects in a render style, L13); a field taken from an item value the look does not declare. **Error** (the game runs without it): a texture a layer names is not found (that layer is skipped); in step 3, an effect type name FX1 does not have. **Warning:** a mesh number (meshes, glow mesh, scrolling mesh, hidden mesh, a mesh item value) that the model of an item using the look does not have, reported per item (that mesh is skipped for it); a value an item sets that its look does not offer (ignored); unknown fields (ignored); a look no item uses; and, as today, a model entry naming a render style or item effect that exists neither in the files nor in code (drawn plainly, or without it). They show in **Data Checks** in two new areas, Render styles and Item effects; a click opens the Style Editor at the look and layer, a problem in an item's own values opens the Item Model window. The Style Editor lists the picked look's problems under it; Apply stays off while a fatal one is left, as Save does in the item tools. Stricter checks (missing textures and mesh numbers fatal) and lenient ones (nothing fatal) were weighed. |
| L24 | Names (Q-L12, 2026-11-01) | "Look" already names the **Classic and Enhanced graphics look** (`docs/enhanced-look.md`: `Data/Render/Look.json`, `Data/Maps/LightMood.json`, `$look`), and the editor used "Looks" for both that (the Data Checks area, `FileCheckArea.cpp:83`) and the Item Model section with the two dropdowns (`MuItemModelUI.cpp:125`). So the new names leave "look" to the graphics: the window is **Style Editor** (Windows > Style Editor, tabs Render styles and Item effects; L18 first called it Look Editor), the new Data Checks areas are **Render styles** and **Item effects**, the existing graphics area becomes **Graphics look**, and the Item Model section becomes **Style and effect**, with "Open in Style Editor". The files and fields (`RenderStyles.json`, `ItemEffects.json`, `renderStyle`, `itemEffect`) already say it. "Model Looks", and keeping "Look Editor" with only the graphics area renamed, were weighed. |
| L25 | Comparing a moved look with its old code (Q-L9, 2026-11-01) | **A draw recorder in the tests**, like FX1's effect recorder (`tests/effects/EffectRecorder.h`) and the monster model recorder (`tests/monsters`). It records every draw call a look makes (mesh, kind of pass, texture, opacity, glow mesh and strength, scrolling, the model's light, and the blend, culling and depth state it sets) and what the look leaves on the item and the model afterwards (item values, the scrolling mesh, the lighting switch), plus how many random numbers it drew. Every item that uses a moved look is drawn with the old code and with the data look in the same build, in a grid of situations: several clock times (pulses, ramps, frames), item levels +0, +7, +9, +11, +13, +15 (the level glow after the look), drawn normally, as a doppelganger and in a monster's hands, worn, in the inventory and on the ground, opaque and fading, and per look its buffs and hidden-mesh states. The records must be equal. Items are set up by the game's own `ItemObjectAttribute` (an earlier recorder that assumed defaults missed a regression), the clock and the random numbers are pinned. It records calls instead of drawing pixels, so it runs in CI without a graphics card. L16's fixes, which are meant to differ, list their expected differences in the pull request and carry in-game before/after pictures. Picture comparison, a picture sheet per pull request on top, and checking in game only were weighed. |

## Current state

### Looks today

- A model entry's `renderStyle` names one of 140 code styles
  (`ItemRenderStyles.cpp`, names at the end of the file), and `itemEffect`
  names one of the item effects (`ItemEffects.cpp`). See
  `docs/item-data.md:983`.
- Many styles draw the whole model, then an extra pass on one mesh:
  - `lordScepter` (`ItemRenderStyles.cpp:705`): the model, then a pulsing
    glowing chrome pass on mesh 1.
  - `chromeMesh0` (`:1518`): the model, then chrome on mesh 0.
  - `stormCrow` (`:82`), `thunderHawk` (`:91`) and `hurricane` (`:741`): the
    model, then a chrome layer in the set's tint on all meshes. #915 changes
    this to chrome under the texture where the texture has holes.
- The level glow and shine (+3 to +15) are chrome-type passes, drawn after
  the look (`ZzzObject.cpp:7036` and on).

### What the looks are made of

Sorted by a script that reads each look's function and the helpers it calls,
so a few can be off:

| | Draw passes only | Plus light, pulse or scrolling | Plus conditions | Creates effects |
|---|---|---|---|---|
| Render styles (140, from 112 functions) | 37 | 74 | 26 | 3 |
| Item effects (25) | – | 11 | 5 (item level) | 9 |

- The conditions the script recognized: the drawing's render type (not for
  doppelgangers, or only in a monster's hands: 7), only without the skin, in
  the inventory and on the ground (10), and which item (6); a style can have
  several. The other conditional styles test other values; the survey of
  Q-L5 reads each one.
- Things placed on bones, e.g. the Wings of Darkness (`ApplyWingsOfDarkness`):
  lightning between bones 22–18 and 30–26 by chance each frame, and a pulsing
  blue flare on each of those bones.
- The 332 draw calls of the looks (228 `RenderMesh`, 104 `RenderBody`) use
  18 flag combinations: the texture (176 calls), the texture glowing (69),
  chrome glowing (38), the drawing's own render type (21), and 14 rarer ones
  (chrome variants 3 to 6, metal, dark chrome, light map, flat color).
  Most other arguments pass the object's own values through (its alpha,
  glow mesh and its strength, scrolling). The looks set the model's light 75
  times: 64 fixed colors, 5 tints of the model's light, 6 that change over
  time.
- An item starts each drawing with no glow mesh, glow strength 1, no
  scrolling and opacity 1 (`ItemObjectAttribute`, `ZzzObject.cpp:5372`).
  The looks pass these values through, name their own for single passes
  (116 draw calls name a glow mesh number), and change them on the item for
  what is drawn after them (about 45 writes; `b->StreamMesh`, the scrolling
  mesh, is a value of the model). Scrolling moves the texture of the glow
  mesh and the scrolling mesh only.
- Values that change over time have four shapes: waves (a sine of the time,
  57), steady scrolling (31), repeating ramps (time modulo a period: scrolls
  that jump back, texture frames; 17) and random chances per frame (8).
- Each reflection motion binds its own texture when the look names none
  (`ZzzBMD.cpp:1718-1743`): `CHROME` binds `Effect/Chrome01.jpg`, `CHROME2`
  to `CHROME4` bind `BITMAP_CHROME2`, `CHROME6` binds `BITMAP_CHROME6`, and
  `METAL` binds `BITMAP_SHINY`. `CHROME5`, `CHROME7` and `OIL` bind none and
  use whatever texture was bound before (e.g. `ItemRenderStyles.cpp:838`);
  Q-L5 checks those uses.
- The conditions, read one by one: the drawing's render type (not for
  doppelgangers: `runeBlade`, `greatScepter`, `grandSoulShield`, the three
  `violentWindToEternalWing…`; only in a monster's hands: `monsterBattleBow`);
  the skin hidden, in the inventory and on the ground (`onlyMesh0/1/2WithoutSkin`,
  `sylphidRay`, `phoenixSoulHelmet`, `violentWindToEternalWing…`,
  `divineAndSuccubusSkin`, `luckyItem`); which item (`socketSeed`,
  `socketSeedSphere`, `sylphWindBow`, `luckyItem`, `violentWindToEternalWing…`);
  the item level (the item effects `potion`, `fruits`, `spirit`,
  `meshesPerLevel`, `hiddenMeshByLevel`); the item's hidden mesh
  (`siegePotion`, `contractSummon`); a buff (`ammunition`: Infinity Arrow);
  the light being exactly white (`capeOfEmperor`, `smallCapeOfLord`, likely
  the inventory's light); the turning angle (`gambleItem`, a brightness
  that follows it); a light value kept between bounds (`symbolOfKundun`,
  `invisibilityCloak`); a texture frame counter (`wingOfStorm`).
- The plain drawing, `RenderModelWithoutStyle` (`ZzzObject.cpp`), has looks
  of its own for some models (the +15 armor objects, `MODEL_SWORD_35_WING`,
  `MODEL_ARMORINVEN_74`, …) outside the style list; Q-L5 looks at them too.
### The survey of the looks (Q-L5)

Every render style and item effect, with the helper functions it calls, read
against the blocks of L8 to L13 (a script flagged the candidates, each one
was read by hand):

| Group | Render styles | Item effects | Names |
|---|---|---|---|
| Fit as designed | 123 | 9 | All render styles not listed below. The item effects that only set values: `devilsEye`, `wingsOfDragon`, `wingsOfSoul`, `redSpirit`, `staffOfKundun`, `divineSet`, `hideMesh1`, `potion`, `hiddenMeshByLevel`. |
| Item effects that draw, become render styles (L13) | – | 7 | `darkLordScroll`, `fruits`, `spirit`, `gmGift`, `meshesPerLevel`, `firecracker`, `billOfBalrog` |
| Need a feature (L16) | 8 | – | `phoenixSoulStar`, `phoenixSoulHelmet`, `phoenixSoulArmor`, `phoenixSoulBoots`, `invisibilityCloak`, `gambleItem`, `capeOfEmperor`, `smallCapeOfLord` |
| Bug-like quirks, fixed while moving (L16) | 6 | – | `wingOfStorm`, `symbolOfKundun`, `bloodBone`, `runeBlade`, `elementalMace`, `grandSoulShield` |
| Things on bones, step 3 | 3 | 9 | `pumpkinOfLuck`, `cursedCastleWater`, `helperNpcPlate` (which the roadmap moves to the helper NPCs); `devilsKey`, `devilsInvitation`, `rena`, `wingsOfDarkness`, `wingOfStorm`, `wingOfEternal`, `wingOfIllusion`, `wingOfRuin`, `wingOfDimension` |

Notes from reading them:
- Setting the model's scrolling mesh (`b->StreamMesh`, or
  `Models[...].StreamMesh`) for a pass or for good (`crossShield`,
  `bloodBone`, `darkReignBlade`, `sylphWindBow`, `lostMap`; the item effects
  `billOfBalrog`, `meshesPerLevel`, `firecracker`) is L10's scrolling mesh,
  as a layer field or an item value.
- A mesh or texture chosen per item (`violentWindToEternalWing…`,
  `luckyItem` with inventory textures of its own, `divineAndSuccubusSkin`,
  `sylphWindBow`) is L12's values per item.
- `albatrossBow` uses `RENDER_CHROME5`, which binds no texture: it draws
  with the texture the pass before it bound, mesh 2's own, so its data
  names the mesh's own texture.
- Random values (`elementalShield`, `grandSoulShield`, `rand() % 10 * 0.1`)
  are L9's random shape; the data must draw the random numbers in the same
  order for the comparison to stay exact (Q-L9).
- So step 1 of L7 covers 137 render styles and 16 item effects, with L16's
  additions and fixes; step 3 covers the other 3 render styles and 9 item
  effects.

### Control per mesh today

- `noneBlendMeshes` (Item Model window, Model file, None-blend meshes,
  `ItemModelForms.cpp:272`): `RenderMesh` skips every chrome-type pass on
  these meshes (`ZzzBMD.cpp:1665`). That removes the look's chrome and also
  the level glow.
- `glow.meshes` / `glow.hiddenMesh`, `glow.shineMeshes` and
  `glow.excellentMesh` (Glow section, `ItemModelForms.cpp:330`).
- The window shows no mesh numbers and no textures, so these fields need
  numbers the user already knows.

### Texture scripts

`RenderBody` (`ZzzBMD.cpp:2638`) reads a script from each texture's file name:
- a hidden mesh (`_H`) is skipped;
- a glowing mesh (`_R`) is drawn as a blend mesh;
- `_DC` / `_DT` add a shadow pass.

`RenderMesh` handles `_N` (no chrome) and `_S` (scrolling) itself. Of the
744 stock item models, 20 have a hidden mesh, 60 a glowing one, and none
uses the shadow scripts.

### Textures with holes

A mesh's texture counts as having alpha when it has 4 components. That is
true for every `.tga`, and for every glTF texture whose file has an alpha
channel, whether or not it has holes. Of the 142 `.tga` textures on stock
item models, 124 have 1% or more of their pixels at or below the 0.25
alpha-test cutoff, one has 0.5%, and 17 have none. The #915 review asks for
a flag set when a texture loads. A chrome layer that goes under the texture
where there are holes needs that flag.

## Target design

Written from the answers to the open questions.

## Open questions

Answered one by one, each with the current state, every option with an
example, and a recommendation.

Answered: Q-L1 (L7), Q-L2a (L8), Q-L2b (L9, L10), Q-L2c (L11), Q-L3 (L12),
Q-L4a (L13), Q-L4b (L14), Q-L4c (L15), Q-L5a (L16), Q-L5b (L17), Q-L6a to Q-L6c (L18 to L20), Q-L7a and Q-L7b (L21, L22), Q-L8 (L23), Q-L12 (L24), Q-L9 (L25), all on 2026-11-01.

| Q | Question |
|---|---|
| Q-L10 | The pull requests and their order (the order of moving looks is L17). |
| Q-L11 | Looks outside the two lists: the 3 looks inside `RenderModelWithoutStyle` (`ZzzObject.cpp:6069-6123`: the +15 armor objects, `MODEL_SWORD_35_WING`, `MODEL_ARMORINVEN_74`), models without model entries; and the glow meshes, glow strengths and hidden meshes that `ItemObjectAttribute` (`ZzzObject.cpp:5372-5521`) sets for 45 item types (e.g. the Saint Crossbow and the Staff of Kundun with a pulsing glow mesh, the Chaos Lightning Staff with a random one). Whether and when they become data (the item docs plan model entries for such effect models later; the item values could become item effects). |

## Tasks

1. [ ] Write this design: answer the open questions one by one (in progress;
   Q-L1 to Q-L9 and Q-L12 done).
2. [ ] Step 1 of L7: draw passes and conditions, in L17's order (the pull
   requests from Q-L10).
3. [ ] Step 2 of L7: FX1.7c, its short design, then the live preview.
4. [ ] Step 3 of L7: things on bones.
5. [ ] Move what stays true into `docs/item-data.md` and `docs/effect-data.md`
   (the look format, the live preview), point the roadmap (row "Items 13"),
   the items design (phase 13) and the possible follow-up ideas (FX1.7c) at
   those docs.
6. [ ] Delete this document.
