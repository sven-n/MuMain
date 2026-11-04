# Looks in Data (Items Phase 13) Design (WIP)

> **Draft, 2026-11-01.** Items phase 13 of the
> [items design](2026-09-25-data-driven-items-design.md) and decision D25 of
> the [data-driven content roadmap](2026-09-29-data-driven-content-roadmap-design.md).
> Written from the code of MuMain `upstream/main` @ `f81fb1b2c` and refreshed
> on 2026-11-03 against `b154e014d`, after the draw refactors #917, #918,
> #929 to #933, #947, #949, #950 and #952
> ([ADR 0003](../../adr/0003-draw-does-not-change-game-state.md): the draw
> only reads, spawns happen in the update). Line numbers are those of
> `b154e014d`. sven-n/MuMain#915 (open) changes the Magic Gladiator
> styles; where this design depends on it, it says so.
>
> This document is temporary. Its last task deletes it, after what stays true
> has moved into `docs/item-data.md`, `docs/effect-data.md` and the code (see
> "Tasks").

## Goal

What a named item look is made of moves from code into data, with an
editor for looks in MuEditor, the Style Editor (D25, L18, L24). Each part (mesh) of a model can get its own layers:
a chrome layer on the hilt only, a glow on the blade, and so on. The Item
Model window gets a list of the model's meshes, so a mesh number can be
picked knowing which part it is.

As before for the items: nothing changes in game when a code look moves into
data, and each move is compared with the old code before the old code goes.

## Scope

- **In:** the item render styles (`renderStyle`, 140 names in
  `ItemRenderStyles.cpp`) and the item effects (`itemEffect`, 25 names in
  `ItemEffects.cpp`), all of them, in three steps (L7); layers per mesh; the
  mesh list; the Style Editor; FX1.7c, the live preview of effects in an
  editor view, which the Style Editor needs for things on bones.
- **Also in (L27):** the glow values `ItemObjectAttribute` sets per item type,
  the bone effects of held and worn items in the character drawing, and the
  looks inside `RenderModelWithoutStyle` once their models have entries.
- **Later:** monster and NPC looks (MN8) use the same format (D25: one look
  format for all areas); effect behavior as data is FX2, which the looks
  don't need: they create the existing effects with their FX1 names.

## Decisions

Made while reviewing sven-n/MuMain#915, 2026-11-01.

| D | Topic | Decision |
|---|---|---|
| L1 | Texture scripts | `_H` (hidden mesh), `_R` (glowing mesh) and `_DC` / `_DT` (shadow pass) are applied in one place that `RenderBody` and the look drawing share. Today only `RenderBody` applies them (`ZzzBMD.cpp:2401-2422`). |
| L2 | Glowing meshes and chrome | A glowing mesh (a `_R` texture, or the object's blend mesh) gets a chrome layer added over it, as on main. None-blend meshes takes the chrome off a mesh. |
| L3 | Control per part | Through the meshes of each layer, not through a separate field for the chrome styles. |
| L4 | Mesh list | The Item Model window lists the model's meshes under Model file. |
| L5 | No render style per mesh | A style draws the whole model, so a mesh gets layers, not a style of its own. |
| L6 | This document | Deleted by its last task, after its lasting content has moved into the docs. |
| L7 | Building blocks (Q-L1, 2026-11-01) | All of D25's blocks for the item looks, in three steps. **Step 1:** draw passes (texture, chrome, metal, glow, dark; pulse, scrolling, light, tint) and conditions (doppelganger, inventory and ground, item level, which item). **Step 2:** FX1.7c, the live preview of effects in an editor view, with its own short design first (see the possible follow-up ideas). **Step 3:** things on bones (sprites, particles, lightning; chances per frame), spawned in the update from the item's own pose as ADR 0003 asks, named as FX1 decided for phase 13 (`{ "joint": "jointThunder", "subType": 14, "fromBone": 3, "toBone": 4 }`, SubType a number, D30), shown live in the Style Editor. |
| L8 | Names of a layer's fields (Q-L2a, 2026-11-01) | Designer terms, with no part for raw engine values (one could be added later without changing data). **Kind:** texture, reflection, light map, flat color. **Motion** (reflections): slow drift (`RENDER_CHROME`), fast sweep (`CHROME2`), follows the light (`CHROME3`), swirl (`CHROME4`), strong swirl (`CHROME5`), stripes (`CHROME6`), very slow drift (`CHROME7`), oil sheen (`OIL`), still shine (`METAL`). **Blend:** normal, glow (`RENDER_BRIGHT`, adds light), darken (`RENDER_DARK`), as the drawing asks (the drawing's own render type). **Meshes:** empty for all. **Where** (reflections): over, or under the holes (as #915 draws it). **Texture:** empty for the mesh's own, or the one that belongs to the motion. **Color:** the model's light, a fixed color, or a tint of the model's light. Kind, Motion and Blend combine freely; an engine feature with no field (e.g. `RENDER_NODEPTH`) gets one added in code. Opacity, the glow mesh and scrolling: L10. |
| L9 | Values that change over time (Q-L2b, 2026-11-01) | A field that changes over time names a shape whose parameters a designer can picture: **fixed**; **wave** (from, to, seconds per cycle); **bounce** (the absolute sine: from, to, seconds); **scroll** (per second); **ramp** (from, to, seconds, then back to the start); **frames** (count, step, seconds each); **random** (from, to, step, a new value each frame; since #949 a draw takes it from `Random::DrawRand`, fixed for the frame, the object and a salt). The loader turns them into the code's numbers; a moved look keeps its exact values (e.g. 6.283185 s for `sin(WorldTime * 0.001f)`), checked against the old code. The editor draws the value over time beside the field. No formula text (D43: building blocks with parameters, not code transcribed). Chances per frame belong to things on bones (L7 step 3). |
| L10 | Glow mesh, scrolling and opacity (Q-L2b, 2026-11-01) | **Fields of each layer**, empty for the item's value: glow mesh (none, a mesh, or all meshes), glow strength, scrolling mesh, scrolling (U and V), opacity (the item's, the drawing's, a number or a shape). **Item values of the look:** the glow mesh, glow strength, scrolling mesh, scrolling, opacity and hidden mesh it leaves on the item for what is drawn after it (the +7 glow, the excellent glow), as `masterScepter` leaves glow mesh 1 (`ItemRenderStyles.cpp:345-359`). Since #950 main keeps them that way (ADR 0003): item draws take their object read-only, and a `DrawLook` (`Render/Models/DrawLook.h`: opacity, glow mesh and strength, texture offsets, hidden mesh) carries this draw's values from pass to pass, starting as the object's; the scrolling mesh and the scene light are still values of the model, and the glow level and opacity of an item effect come back through its call (refresh, 2026-11-03). The 11 item effects that only change values become looks with item values and no layers. When item values apply: L13 (an item effect's before the layers, a render style's after them). |
| L11 | Conditions (Q-L2c, 2026-11-01) | **On the look, "Not for":** the drawings the look is not for, which the game then draws plainly, as a style returning false does today: doppelgangers, and outside a monster's hands (`monsterBattleBow`). **On a layer, "Only when":** worn, in the inventory and on the ground (the skin hidden), an item level range, the item's hidden mesh, a buff on the character. "Which item" is no condition: values chosen per item are Q-L3. Conditions found in the survey that fit neither list (the light being exactly white, the turning angle): Q-L5. |
| L12 | Where looks live (Q-L3, 2026-11-01) | **Named looks** in a look file in `Data/Effects/`, which model entries name (D25), with D26's rules: the editor shows which items use a look, saving a look that others use first lists them, a copy makes a variant for one user, renaming updates the references, a look still in use can't be deleted, names are resolved when loading. **Values per item:** in the Style Editor a field can be ticked "Each item sets this", with a name and the look's value as default. Every item using the look shows those fields under "Values for this item" in its Item Model window, stored in its model entry; Reset goes back to the look's default. Everything else is shared, so a change to the look reaches all its items and keeps their own values. This covers a color per item (the 30 Seed Spheres), meshes per item (the Lucky items, the Violent Wind pieces), and chrome on chosen parts of one item (e.g. the Divine Sword using `thunderHawk` with its chrome meshes set to 1). The Item Model window keeps its **Render style** dropdown for picking the look (a new "Look" label was found confusing); the field name in the data and the look file's name: Q-L4. |
| L13 | Render style and item effect (Q-L4a, 2026-11-01) | **Two looks per item, with fixed roles**, as today's two fields: at most one render style and one item effect. A **render style** look draws the item: its layers, its "Not for" and the item values it leaves after drawing. An **item effect** look does what comes before the drawing: item values before drawing, and things on bones (which since ADR 0003 spawn in the update; render styles have none: L29). The Style Editor offers only the parts of a look's kind, so two looks can never both draw. Mixing is a field on the item (Wing of Ruin can pick the item effect `wingsOfDarkness`), and an edit to a look reaches every item that uses it (D26). Render styles and item effects stay two lists, so a name can be in both (`wingsOfDarkness`). This answers L10's open point: an item effect's item values apply before the layers, a render style's after them. The 7 item effects that draw have layers, so they become render styles: 6 draw the model themselves (`darkLordScroll`, `fruits`, `spirit`, `firecracker`, `gmGift`, `meshesPerLevel`), and `billOfBalrog` draws an extra pass before the usual drawing (found in the survey, Q-L5). One look per item (its parts copied when mixing) and one look whose parts can be linked to other looks were weighed: both need a new look for every mix, and copies don't get later edits. Compared with linked parts in five tasks, two looks are quicker to combine (one dropdown), keep today's data unchanged and are less to build; linked parts only read better (one window per item). |
| L14 | The move from code to data (Q-L4b, 2026-11-01) | **Same names, data first.** When loading, a name is looked up in the look file first, then in the code list; a name in neither is reported and drawn plainly, as today. Model entries never change during the move. Each pull request moves a group of looks: it adds them to the look file under their names, a test draws their items with the old code and with the data in the same build and compares the records (as FX1's moves did), and then it deletes their functions and code names; a test checks that no name is in both. The editor marks looks that are still code ("(code)" in the dropdowns); they can be picked but open read-only in the Style Editor until moved. New names for data looks (every model entry changed twice) and one move of all looks at once (one very large pull request) were weighed. |
| L15 | The look files (Q-L4c, 2026-11-01) | **One file per kind** in `Data/Effects/`: `RenderStyles.json` and `ItemEffects.json`, named like the fields `renderStyle` and `itemEffect` and the two dropdowns. Built like the effect catalogue files next to them: a format version, the kind, and a list sorted by name, which the Style Editor writes back sorted so a change stays a few lines. Looks are shared across item groups, so they are not split by group like the model files. When monsters and NPCs get looks (MN8), they use the same files (D25); renaming the "item effect" kind for them is the monsters design's call. One file for both kinds and one file per look (165 files) were weighed. |
| L16 | Looks that don't fit (Q-L5a, 2026-11-01) | **Real features become blocks:** a value that is the item's value times a shape (the four Phoenix Soul items, `cursedCastleWater`); the model's light at a fixed brightness (`invisibilityCloak`); a value from the item's turning angle (`gambleItem`); "in the inventory" as a place condition next to L11's "inventory and ground" (`capeOfEmperor`, `smallCapeOfLord`, which test for exactly white light today; the pull request checks which drawings have white light); a zigzag shape, straight up and down (`symbolOfKundun`); the model lit by the scene light (`firecracker` sets `b->LightEnable`). **Bug-like quirks are fixed while moving**, each in its own commit with a before/after in game, not copied with blocks of their own: `wingOfStorm` and `symbolOfKundun` count their animation per drawing in a shared `static` counter, so it speeds up with every one on screen (moved: counted in time; done on main by #950, both read the reference clock now); `bloodBone` scrolls by `sin(map number × 0.0001)` instead of the clock (moved: no offset, under 1/100 of the texture); `runeBlade`, `elementalMace` and `grandSoulShield` make a grey from the red channel of the model's light only (moved: the light's tint, the same under white light). Copying the quirks exactly, and leaving these looks as code, were weighed. |
| L17 | The order of moving (Q-L5b, 2026-11-01) | **By building block**, so each pull request brings one kind of block and moves the looks that need exactly it: (1) the machinery (loader, layer drawing, comparison test, Style Editor basics) with the 36 render styles of plain passes; (2) light and time, 67 render styles; (3) conditions and values per item, 20 render styles, among them the Magic Gladiator sets after #915 is merged (reflection under the holes); (4) the item effect kind, 16 item effects (9 that set values, 7 that draw become render styles); (5) the additions and quirk fixes of L16, 14 render styles. Then step 2 (FX1.7c) and step 3 (3 render styles, 9 item effects). The counts come from the survey script; each pull request's comparison test shows the exact list. The split into pull requests, with the mesh list and the editor parts: Q-L10. The Magic Gladiator sets as a first pilot, one pull request per item group, and the most-used looks first were weighed. |
| L18 | Where the Style Editor lives (Q-L6a, 2026-11-01) | **A window of its own**, Windows > Style Editor, next to the Effect Browser, for the items now and the monsters later (D25: one look editor for all areas). Built like Glow Colors (then `MuItemGlowColorsUI.cpp`; since #926 the Glow root of the Colors tree, `MuColorsUI.cpp`, whose tree replaces the tabs and flat list here: L31): tabs Render styles and Item effects; a button row (New look, Copy, Rename) and a grey note; on the left a resizable list with each look's name, how many items use it, and "(code)" for looks not moved yet; on the right the picked look, Apply to the look / Revert / Remove, "Used by N items:" with the list of items, and the confirm that lists them (D26). The Item Model window keeps its two dropdowns, "Values for this item" and gets "Open in Style Editor". An item tool under Windows > Items, and editing inside the Item Model window, were weighed. |
| L19 | The layer list (Q-L6b, 2026-11-01) | **A list, then the picked layer.** The look's layers in drawing order, one line each written from its fields ("texture, mesh 0, glowing blue, pulsing"), with + Add, Copy, up, down and Remove above the list, and "Item values after drawing" (or "before drawing" for an item effect) as the last line. Clicking a line shows its fields below: the fields of L8 to L11, the value graph beside a field that changes over time (L9), and the "Each item sets this" box (L12). A collapsing section per layer, and a table with a column per field, were weighed. |
| L20 | The Style Editor's preview (Q-L6c, 2026-11-01) | **The Item Model preview with helpers for designing.** As in the Item Model window (`ItemModelPreview.h`): the look shown on an item (any of its users, or any other item to try it on), at an item level, excellent or ancient, in the inventory pose or on the ground, turning, with the same camera; changes show before they are applied. Added: stopping time and moving it with a slider (the clock the looks read while the preview draws), an eye per layer to hide it or show only it, and "Draw as: doppelganger" to see "Not for". Things on bones show here after FX1.7c (L7, step 2). **Later, on top:** a pose worn by a character in an animation (no preview draws a character with equipment yet). |
| L21 | The mesh list's rows (Q-L7a, 2026-11-01) | Under Model file in the Item Model window, after Texture folders and None-blend meshes: **one row per mesh with its number, its texture name, marks and "used by"**. Marks: hidden (a "hid…" texture name or an `_H` script: never drawn), glows (`_R`), scrolls (`_S`), no chrome (`_N` or None-blend meshes), holes (the load-time flag asked for in #915; added by the first pull request of the move if #915 doesn't). "Used by": the fields that name the mesh (None-blend, glow, shine and excellent meshes) and the layers of the item's looks that draw it ("Render style textured: layer 1, all meshes"); a look that is still code says so until its move. A tooltip shows the texture's size and the folder it was found in, and the mesh's vertices and triangles. Number and texture only, and a wide table with every column, were weighed. |
| L22 | Highlighting a mesh (Q-L7b, 2026-11-01) | **Pointing at a mesh row, or at a number in a mesh field** (None-blend meshes, glow meshes, a layer's Meshes in the Style Editor), **draws the other meshes see-through** in the preview, so the pointed mesh stands out as it looks in game and a part behind another shows (editor builds only); the pointed mesh itself gets no highlight color. A click keeps it so until clicked again; without a pointed mesh the item is drawn as in game. **Clicking a part in the preview selects its row** and its number in the mesh field being edited (a ray from the mouse through the preview camera, tested from either side against the item's triangles as the drawing poses them; the nearest hit wins. Changed on 2026-11-03 while building pull request 1: the first choice, meshes drawn in ID colors into a hidden picture and the pixel under the mouse read back, needs a readback the SDL_GPU renderer does not have for its captures, and the game's own line test, `BMD::CollisionDetectLineToMesh`, skips the faces turned away and keeps its result in globals). Works in the Item Model preview and the Style Editor's preview. A mesh that is never drawn can't be shown this way; its row says so. A highlight color drawn on top of the mesh, with or without the others dimmed, and no highlight at all, were weighed. |
| L23 | Data checks (Q-L8, 2026-11-01) | **The project's three levels** (`docs/item-data.md`, "Loading and errors"; `docs/dev-editor.md`, Data Checks). **Fatal** (Save and Apply refuse, the game does not start): invalid JSON or `formatVersion`, a file of the wrong kind; a look without a name, a name that is not letters and digits, a name twice in a file; a value of the wrong type or out of range (seconds of 0 or less, a frame count of 0); a kind, motion, blend, shape, condition or place that does not exist; a part the look's kind does not have (layers in an item effect, bone effects in a render style, L13); a field taken from an item value the look does not declare. **Error** (the game runs without it): a texture a layer names is not found (that layer is skipped); in step 3, an effect type name FX1 does not have. **Warning:** a mesh number (meshes, glow mesh, scrolling mesh, hidden mesh, a mesh item value) that the model of an item using the look does not have, reported per item (that mesh is skipped for it); a value an item sets that its look does not offer (ignored); unknown fields (ignored); a look no item uses; and, as today, a model entry naming a render style or item effect that exists neither in the files nor in code (drawn plainly, or without it). They show in **Data Checks** in two new areas, Render styles and Item effects; a click opens the Style Editor at the look and layer, a problem in an item's own values opens the Item Model window. The Style Editor lists the picked look's problems under it; Apply stays off while a fatal one is left, as Save does in the item tools. Stricter checks (missing textures and mesh numbers fatal) and lenient ones (nothing fatal) were weighed. |
| L24 | Names (Q-L12, 2026-11-01) | "Look" already names the **Classic and Enhanced graphics look** (`docs/enhanced-look.md`: `Data/Render/Look.json`, `Data/Maps/LightMood.json`, `$look`), and the editor used "Looks" for both that (the Data Checks area, `FileCheckArea.cpp:87`) and the Item Model section with the two dropdowns (`MuItemModelUI.cpp:125`). So the new names leave "look" to the graphics: the window is **Style Editor** (Windows > Style Editor, tabs Render styles and Item effects; L18 first called it Look Editor), the new Data Checks areas are **Render styles** and **Item effects**, the existing graphics area becomes **Graphics look**, and the Item Model section becomes **Style and effect**, with "Open in Style Editor". The files and fields (`RenderStyles.json`, `ItemEffects.json`, `renderStyle`, `itemEffect`) already say it. "Model Looks", and keeping "Look Editor" with only the graphics area renamed, were weighed. |
| L25 | Comparing a moved look with its old code (Q-L9, 2026-11-01) | **A draw recorder in the tests**, like FX1's effect recorder (`tests/effects/EffectRecorder.h`) and the monster model recorder (`tests/monsters`). It records every draw call a look makes, as a drawer put in place of the default `ModelDrawer` (`Render/Models/ModelDraw.h`, #917), which is handed each draw with the state it reads (mesh, kind of pass, texture, opacity, glow mesh and strength, scrolling, the model's light, and the blend, culling and depth state it sets) and what the look leaves on the item and the model afterwards (item values, the scrolling mesh, the lighting switch), plus the `Random::DrawRand` values it used (since #949 a draw may not draw from the update's random sequences). Every item that uses a moved look is drawn with the old code and with the data look in the same build, in a grid of situations: several clock times (pulses, ramps, frames), item levels +0, +7, +9, +11, +13, +15 (the level glow after the look), drawn normally, as a doppelganger and in a monster's hands, worn, in the inventory and on the ground, opaque and fading, and per look its buffs and hidden-mesh states. The records must be equal. Items are set up by the game's own `ItemObjectAttribute` (an earlier recorder that assumed defaults missed a regression), the clock and the random numbers are pinned. It records calls instead of drawing pixels, so it runs in CI without a graphics card; the draws run inside a `DrawScope` and must spawn nothing (ADR 0003). L16's fixes, which are meant to differ, list their expected differences in the pull request and carry in-game before/after pictures. Picture comparison, a picture sheet per pull request on top, and checking in game only were weighed. |
| L26 | The pull requests (Q-L10, 2026-11-01) | **Five pull requests after the design**, each as a fork pull request for CI and an upstream one for sven-n (the fork one closed, not merged, after the upstream merge). **0, the design:** this document as a docs pull request, reviewed first (as the monsters design was). **1, the mesh list:** L21, L22 and the names of L24; small and independent, it can run beside the design's review. **2, step 1 part 1:** the machinery (loader, the two files, layer drawing, the draw recorder of L25, the checks of L23), the Style Editor (L18 to L20), and L17's groups 1 and 2 (103 render styles, with colors and time shapes); the largest, about the size of the monsters' second pull request. **3, step 1 part 2:** L17's groups 3 to 5 (50 looks: conditions, values per item and the Magic Gladiator sets, which need #915 merged; the item effects; the additions, and the fixes in commits of their own with in-game before/after). **4, steps 2 and 3, and the end:** FX1.7c (its short design and the live preview), the 3 render styles and 9 item effects on bones, the lasting parts moved into the docs, and this document deleted. Inside each pull request, one commit or more per group of L17, so the review can go group by group. A pull request per group (9 in all), and the Style Editor only after step 1, were weighed. |
| L27 | Looks outside the two lists (Q-L11, 2026-11-01) | **All of them join this work.** (1) The glow values `ItemObjectAttribute` (`ZzzObject.cpp:6085-6235`) sets per item type, for the hand, the inventory, the ground and the previews: a glow mesh, a pulsing or random glow strength, a scroll or a hidden mesh, for 39 item types (e.g. the Saint Crossbow, the Staff of Kundun, the Chaos Lightning Staff, the Rune Blade, Wings of Elf and the orbs). They are item values before drawing, the item effect kind (L13), and move in pull request 3 with the item effects; items that already have an item effect (`staffOfKundun`, `redSpirit`) get them in that effect. (2) The bone effects of held and worn items: sprites, particles and lights on the item by its type. Since #931 they spawn in the update, in `SpawnLinkedPartEffects` (`ZzzCharacter.cpp:7204-8633`, 62 item-type cases, about 300 spawn calls) and `SpawnHeldWeaponGlow` (`:11595-11905`, 30 cases); `RenderLinkObject` (`:7029-7200`) and `RenderCharacter` (`:9585-10277`) keep only what the draw decides (refresh, 2026-11-02; before, about 75 item types in those two draw functions). They move in a **sixth work pull request, after step 3**, with a survey of their own first that sorts looks from other code. (3) The 3 looks inside `RenderModelWithoutStyle` (`ZzzObject.cpp:6706-6761`) for 11 models without model entries (the +15 armor objects, `MODEL_SWORD_35_WING`, `MODEL_ARMORINVEN_74`) join once those models have entries (how: Q-L13). This adds to L26: a pull request 5 for (2) after pull request 4, and "the end" (the docs, this document deleted) moves to the last pull request of the work. Not looks, only noted: `ItemObjectAttribute` also sets a scale for some items on the ground (spears, staffs, the Rena), which is ground display data. Glow values only, and all three later, were weighed. |
| L28 | Entries for the models of the plain drawing's looks (Q-L13, 2026-11-01) | **Entries for the models only, in this work.** The 11 models are no items: 9 decorations that every worn armor piece gets at +15, at fixed bones (`class15_*.bmd`, `ForEachNextGradeObject`, `ZzzObject.cpp:7363-7437`; their sparks spawn in `SpawnLinkedPartEffects`), the wing attached to the held Phoenix Soul Star (0,35) (`sword36wing.bmd`, `ZzzCharacter.cpp:6645`), and a Rage Fighter armor's own inventory model (`MODEL_ARMORINVEN_74`). Pull request 3 gives them entries in a small list of part models, `Data/Items/Models/PartModels.json`: a name, the file, the texture folders and the render style, edited like the shared models (where exactly in the editor: with pull request 3's editor work); their 3 looks move with them. The code keeps deciding when and where they are drawn (which slot gets which part model at which bone, the wing's offset, which armor uses an inventory model): that is item logic, not a look, and can become data later without changing these entries. The model entries the item docs plan for effect and event models can use the same list. Entries where each model belongs with when and where (three new kinds of data), and waiting for those planned entries, were weighed. |
| L29 | Things on bones in render styles (Q-L14, 2026-11-03) | **Only item effects have things on bones**, as L13 decided. On main since #931 render styles also have a spawn part in the update (`ItemRenderStyles.h:33-38`): `pumpkinOfLuck` (three lights on bones 8, 10 and 11) and `cursedCastleWater` (a light on bone 1 and the model's animation step at speed 2/7). When step 3 moves them (pull request 4), each gets an item effect of the same name with those things on bones, the water's animation speed with them; its render style keeps only the drawing, and the model entries (14,45) and (14,64) gain an `itemEffect`. L23's fatal check for bones in a render style stays. Both kinds with bones, as on main, and a third kind "bone effect" with its own file and dropdown were weighed. |
| L30 | A render style's own glow and shine (Q-L15, 2026-11-03) | **Into the item's glow data.** Two extras of the level glow hang on render style entries today (`ItemRenderStyles.cpp:31-66`): the shine below +3 of 10 items (`ZzzObject.cpp:7703-7725`: at +0 to +2 the look is drawn with the item's light times a factor, then a fast sweep at opacity 1.5 and a swirl, both glowing, in the item's shine color on its shine meshes), named `HarmonyShine` (Jewel of Harmony, Moonstone Pendant, Illusion Sorcerer Covenant), `SealShine` (the six Seals and Master Seals, light times 0.9) and `CursedCastleWaterShine` (the model's light, a texture glow at 0.5, then a swirl); and the Deadly Staff's own +7 glow pass (`ItemGlow.cpp:151`: the whole staff glows, then mesh 1 again in white). The Glow section of a model entry gets a **shine below +3**, which names one of these shines from a small shared list of named shines in `Data/Effects/` (a light factor and two passes each; the file is pull request 3's), and the Deadly Staff gets **glow again in white on mesh 1**. The style `harmonyShine`, which draws plainly and only carried the shine, goes away. The 11 model entries change in pull request 3, compared by the draw recorder at every level. Layers of the render style with level conditions (three new blocks for 11 items, and looks split by level), and leaving both as code for later, were weighed. |
| L31 | The Style Editor's list (Q-L16, 2026-11-03) | **One tree like the Colors window.** Glow Colors, whose flat list L18 copied, became the Glow root of the Colors window in #926 (`MuColorsUI.cpp`): a fixed root per use, your own nested folders, drag and drop within a root, New folder, New color, Rename, Remove, Move up and Move down, and the folders saved next to the colors as `"colorFolders"`. The Style Editor works the same way: the roots Render styles and Item effects take the place of L18's tabs, folders sort the looks (a set's styles, the wings, the tickets), the toolbar is New folder, New look, Copy, Rename, Remove, Move up and Move down, and each look file keeps its folders as `"lookFolders"`, which the game skips. A look stays in its root, looks still in code can be in folders, and folder names follow the Colors window's rules (unique among siblings; stale names warned and dropped on save). The rest of L18 to L20 stays. Tabs with a flat list (L18 as first written), and tabs with folders, were weighed. |

## Current state

### Looks today

- A model entry's `renderStyle` names one of 140 code styles
  (`ItemRenderStyles.cpp`, names at the end of the file), and `itemEffect`
  names one of the item effects (`ItemEffects.cpp`). See
  `docs/item-data.md:988`.
- Many styles draw the whole model, then an extra pass on one mesh:
  - `lordScepter` (`ItemRenderStyles.cpp:691`): the model, then a pulsing
    glowing chrome pass on mesh 1.
  - `chromeMesh0` (`:1448`): the model, then chrome on mesh 0.
  - `stormCrow` (`:92`), `thunderHawk` (`:101`) and `hurricane` (`:727`): the
    model, then a chrome layer in the set's tint on all meshes. #915 changes
    this to chrome under the texture where the texture has holes.
- The level glow and shine (+3 to +15) are chrome-type passes, drawn after
  the look (`ZzzObject.cpp:7746` and on).
- A render style entry can carry two more parts besides its drawing
  (`ItemRenderStyles.cpp:31-66`, found in the refresh): its own glow pass
  for +7 and up, which `ItemGlow.cpp:151` draws instead of the item's glow
  meshes (`deadlyStaff`), and a shine below +3 (`ZzzObject.cpp:7703-7725`:
  the look drawn with its light times a factor, then two bright passes),
  named `HarmonyShine` (`illusionSorcererCovenant`, and `harmonyShine`, a
  style that draws plainly and exists for the shine), `SealShine` (the
  three seals) and `CursedCastleWaterShine`. L30.
- **Since the draw refactors (ADR 0003):** `RenderMesh` describes each draw
  as a `MeshDraw` (mesh, flags, opacity, glow mesh and strength, scrolling,
  texture, the model's light, lighting, hidden skin, scrolling mesh, the
  skinning) and hands it to the active `ModelDrawer`; the default drawer
  runs the old body, now `BMD::DrawMesh`. Spawns moved to the update: render
  styles and item effects each got a spawn part
  (`ItemRenderStyles.h:33-38`, `ItemEffects.h:41-44`), run for the items the
  last frame drew, from the item's own pose. It holds the render styles
  `pumpkinOfLuck` and `cursedCastleWater` (lights, and the water's
  animation step), the helper NPCs' plate lights, and the item effects
  `devilsEye` (its light), `devilsKey`, `devilsInvitation`, `rena` and the
  wings. Items in the inventory spawn none. A spawn inside the draw is a
  defect, reported by `DrawSpawnCheck` in every build. Since #947, #949 and
  #950 the drawer takes its draw read-only, item effects, render styles and
  the glow take the object read-only with a `DrawLook` for the draw (L10),
  and draws take random values from `Random::DrawRand` instead of the
  update's sequence. Since #952 a source check keeps every `Render*`,
  `Draw*` and `Describe*` function from taking a writable object.

### What the looks are made of

Sorted by a script that reads each look's function and the helpers it calls,
so a few can be off:

| | Draw passes only | Plus light, pulse or scrolling | Plus conditions | Spawns effects (in the update since #931) |
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
  scrolling and opacity 1 (`ItemObjectAttribute`, `ZzzObject.cpp:6085`).
  The looks pass these values through, name their own for single passes
  (116 draw calls name a glow mesh number), and change them on the item for
  what is drawn after them (about 45 writes; `b->StreamMesh`, the scrolling
  mesh, is a value of the model). Scrolling moves the texture of the glow
  mesh and the scrolling mesh only.
- Values that change over time have four shapes: waves (a sine of the time,
  57), steady scrolling (31), repeating ramps (time modulo a period: scrolls
  that jump back, texture frames; 17) and random chances per frame (8).
- Each reflection motion binds its own texture when the look names none
  (`ZzzBMD.cpp:1809-1835`): `CHROME` binds `Effect/Chrome01.jpg`, `CHROME2`
  to `CHROME4` bind `BITMAP_CHROME2`, `CHROME6` binds `BITMAP_CHROME6`, and
  `METAL` binds `BITMAP_SHINY`. `CHROME5`, `CHROME7` and `OIL` bind none and
  use whatever texture was bound before (e.g. `ItemRenderStyles.cpp:820`);
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
- The plain drawing, `RenderModelWithoutStyle` (`ZzzObject.cpp:6706`), has looks
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
| Things on bones, step 3 | 3 | 9 | `pumpkinOfLuck`, `cursedCastleWater` (their bones become item effects of the same name, L29), `helperNpcPlate` (which the roadmap moves to the helper NPCs); `devilsKey`, `devilsInvitation`, `rena`, `wingsOfDarkness`, `wingOfStorm`, `wingOfEternal`, `wingOfIllusion`, `wingOfRuin`, `wingOfDimension` |

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
- Random values (`elementalShield`, `grandSoulShield`, now
  `Random::DrawRand(o, salt) % 10 * 0.1`) are L9's random shape. Since #949
  a draw's random value is fixed for the frame, the object and a salt, so
  the data names the salt and the comparison stays exact.
- So step 1 of L7 covers 137 render styles and 16 item effects, with L16's
  additions and fixes; step 3 covers the other 3 render styles and 9 item
  effects.

### Control per mesh today

- `noneBlendMeshes` (Item Model window, Model file, None-blend meshes,
  `ItemModelForms.cpp:272`): `RenderMesh` skips every chrome-type pass on
  these meshes (`ZzzBMD.cpp:1762`). That removes the look's chrome and also
  the level glow.
- `glow.meshes` / `glow.hiddenMesh`, `glow.shineMeshes` and
  `glow.excellentMesh` (Glow section, `ItemModelForms.cpp:330`).
- The window shows no mesh numbers and no textures, so these fields need
  numbers the user already knows.

### Texture scripts

`RenderBody` (`ZzzBMD.cpp:2389`) reads a script from each texture's file name:
- a hidden mesh (`_H`) is skipped;
- a glowing mesh (`_R`) is drawn as a blend mesh;
- `_DC` / `_DT` add a shadow pass.

`RenderMesh` handles `_N` (no chrome) and `_S` (scrolling) itself. Of the
744 stock item models, none has a mesh hidden by its script, 59 have a glowing
one, and none uses the shadow scripts. The parser drops a texture's whole
script at its first unknown letter, so a name like `Item762_Helm.jpg` has no
script (a count of 20 hidden meshes, made without that rule, was wrong).

### Textures with holes

A mesh's texture counts as having alpha when it has 4 components. That is
true for every `.tga`, and for every glTF texture whose file has an alpha
channel, whether or not it has holes. Of the 142 `.tga` textures on stock
item models, 124 have 1% or more of their pixels at or below the 0.25
alpha-test cutoff, one has 0.5%, and 17 have none. The #915 review asks for
a flag set when a texture loads. A chrome layer that goes under the texture
where there are holes needs that flag.

## Target design

The finished design, put together from L1 to L28. The examples use real
looks with the values of their code. Field names in the examples are this
design's working names; the pull request that first reads a field (L26)
fixes its name in `docs/item-data.md`. Points that are no decision of their
own but follow from two of them say so.

### 1. Two kinds of looks, in two files

An item model has at most two looks, as today (L13):

| Kind | Named by | Parts | When it acts |
|---|---|---|---|
| **Render style** | `renderStyle` in the model entry, the Render style dropdown | Not for; layers; item values after drawing; values per item | It draws the item. Without one, the game draws the item plainly. |
| **Item effect** | `itemEffect` in the model entry, the Item effect dropdown | item values before drawing; things on bones (step 3); values per item | Item values: before the drawing. Things on bones: in the update, for items the last frame drew (ADR 0003). Render styles have none (L29). |

A look can't have the other kind's parts (a fatal check, L23), so two looks
never both draw. Combining two looks is a field on the item: Wing of Ruin
can keep its render style and pick the item effect `wingsOfDarkness`. A
change to a look reaches every item that uses it (D26).

Looks are named and shared (L12), in one file per kind in `Data/Effects/`
(L15), built like the effect catalogue's files: a format version, the kind,
a list sorted by name, and the Style Editor's folders (`lookFolders`, L31),
which the game skips. The two lists are separate, so a name can be in both.

```json
Data/Effects/RenderStyles.json
{
  "formatVersion": 1,
  "kind": "renderStyle",
  "looks": [
    {
      "name": "greatScepter",
      "notFor": ["doppelganger"],
      "layers": [
        { "kind": "texture", "meshes": { "allBut": [2] }, "opacity": "drawing" },
        { "kind": "texture", "meshes": [2], "glowMesh": 2, "glowStrength": 1,
          "opacity": "drawing",
          "scrollU": { "scroll": { "perSecond": 0.5 } },
          "scrollV": { "scroll": { "perSecond": 0.5 } } }
      ],
      "itemValuesAfter": { "hiddenMesh": 2, "glowMesh": 2, "glowStrength": 1 }
    },
    {
      "name": "masterScepter",
      "layers": [
        { "kind": "texture", "opacity": "drawing" },
        { "kind": "texture", "meshes": [0], "color": [0.1, 0.3, 1.0],
          "glowMesh": 0, "opacity": "drawing",
          "glowStrength": { "wave": { "from": -0.2, "to": 1.0, "seconds": 6.283185 } } },
        { "kind": "texture", "meshes": [1], "texture": "Effect/Chrome01.jpg",
          "color": [0.6, 0.8, 1.0], "glowMesh": 1, "glowStrength": 1,
          "opacity": "drawing", "scrollV": { "scroll": { "perSecond": 0.3 } } }
      ],
      "itemValuesAfter": { "glowMesh": 1, "glowStrength": 1 }
    },
    {
      "name": "thunderHawk",
      "layers": [
        { "kind": "texture" },
        { "kind": "reflection", "motion": "slowDrift", "blend": "glow",
          "where": "underHoles", "texture": "Effect/bab2.jpg",
          "color": { "tint": [0.85, 0.85, 1.2] },
          "meshes": { "eachItem": "chromeMeshes", "default": "all" } }
      ]
    }
  ]
}
```

```json
Data/Effects/ItemEffects.json
{
  "formatVersion": 1,
  "kind": "itemEffect",
  "looks": [
    { "name": "hiddenMeshByLevel",
      "itemValuesBefore": [
        { "onlyWhen": { "level": [0, 0] }, "hiddenMesh": 1 },
        { "onlyWhen": { "level": [1, 1] }, "hiddenMesh": 0 } ] },
    { "name": "potion",
      "itemValuesBefore": [ { "onlyWhen": { "level": [1, 15] }, "glowsLikeLevel": 7 } ] }
  ]
}
```

### 2. A layer

A render style draws its layers in their order; each layer draws its
meshes. Control per part is a layer's meshes, never a style per mesh (L3,
L5). The fields, in designer terms (L8, L10, L11), each with the engine
value it becomes:

| Field | Values | Becomes |
|---|---|---|
| Kind | texture, reflection, light map, flat color | the texture pass, the chrome family, `RENDER_LIGHTMAP`, `RENDER_COLOR` |
| Motion (reflections) | slow drift, fast sweep, follows the light, swirl, strong swirl, stripes, very slow drift, oil sheen, still shine | `RENDER_CHROME`, `CHROME2` to `CHROME7`, `OIL`, `METAL` |
| Blend | normal, glow, darken, as the drawing asks | none, `RENDER_BRIGHT`, `RENDER_DARK`, the drawing's own render type |
| Meshes | empty for all, a list, all but a list | the meshes the layer draws |
| Where (reflections) | over, or under the holes | section 4 |
| Texture | empty for the mesh's own (or the motion's own, `ZzzBMD.cpp:1809-1835`), or a file | the texture bound for the pass |
| Color | empty for the model's light, a fixed color, a tint of the model's light, the model's light at a fixed brightness (L16) | the model's light while the layer draws |
| Opacity | empty for the item's, the drawing's, a number, a shape | the pass's alpha |
| Glow mesh, glow strength | empty for the item's, none, a mesh, all meshes; a number or a shape | the blend mesh and its light |
| Scrolling mesh, scroll U and V | empty for the item's, a mesh; numbers or shapes | the stream mesh and the texture offsets |
| Only when | worn; in the inventory and on the ground; in the inventory (L16); an item level range; the item's hidden mesh; a buff on the character | the layer is skipped otherwise |

Kind, Motion and Blend combine freely. An engine feature with no field gets
one added in code, never raw flags in the data (L8). A layer draws with
`RenderBody`'s mesh rules (section 4).

**Values over time (L9, L16).** A field that changes over time names a
shape: fixed; wave (from, to, seconds per cycle); bounce (from, to,
seconds); scroll (per second); ramp (from, to, seconds, then back to the
start); frames (count, step, seconds each); random (from, to, step, a new
value each frame); zigzag (straight up and down, L16). Two more sources
(L16): the item's own value times a shape, and the item's turning angle.
The loader turns shapes into the code's numbers, and a moved look keeps its
exact values (6.283185 s above is `sin(WorldTime * 0.001f)`). The editor
draws the value over time beside the field.

**Values per item (L12).** Any field can be ticked "Each item sets this",
with a name and the look's value as default (`eachItem` above). The item
stores its values in its model entry, one set per look kind (follows from
L12 and L13):

```json
{ "number": 100, "renderStyle": "socketSeedSphere",
  "renderStyleValues": { "color": [0.9, 0.1, 0.2] } }
```

In the same way the Divine Sword of L12 names `thunderHawk` and sets
`chromeMeshes` to `[1]`.

So `socketSeedSphere` loses its test for the sphere item types
(`ItemRenderStyles.cpp:1316`): its users set their color, and other items
don't name it.

### 3. Item values and conditions

**Item values** are what a look leaves on the item for what is drawn after
it in the same drawing, the +7 glow and the excellent glow (L10, L16): glow
mesh, glow strength, scrolling mesh, scrolling, opacity, hidden mesh, the
level the item glows like (`potion`), and the model lit by the scene light
(`firecracker`). An item effect's apply before the layers, a render style's
after them (L13). An entry of item values can carry an Only when, so a value
can depend on the item level (follows from L11 and L13; `hiddenMeshByLevel`,
`potion`). Item values are this drawing's `DrawLook` (#950): handed from
pass to pass and never written on the item's object, since the draw must
not change what the update or the next frame reads (ADR 0003, L10). A look that sets an item value before its own passes today
(`greatScepter` sets its hidden mesh, glow mesh and glow strength first)
names the values in its layers and leaves them after.

**Not for** (L11) is on a render style: for those drawings (doppelgangers,
outside a monster's hands) the game draws the item plainly, as a style
returning false does today.

### 4. Drawing

**Shared mesh rules (L1, L2).** `RenderBody` and the layer drawing use the
same code for the texture scripts:
- a hidden mesh (`_H`, or a "hid…" texture name) is skipped;
- a glowing mesh (`_R`, or the item's glow mesh) is drawn glowing; a
  reflection layer on it is still added over it; None-blend meshes takes
  reflections and the level glow off a mesh, as today;
- `_DC` / `_DT` add their shadow pass, as in `RenderBody`.

**The level glow** stays the glow code's, drawn after the look from the
item's glow data, now with the shine below +3 and the Deadly Staff's second
glow pass among its values (L30).

**Under the holes (L8, as #915 draws it).** On a mesh whose texture has
holes, a reflection marked "under the holes" is drawn before the texture,
solid and on both faces, so it fills the holes and the texture covers the
rest. On a mesh without holes, and on every mesh of a see-through drawing
(the doppelgangers), it is added over the texture with the layer's blend. "Has holes" is a flag set
when a texture loads, from its pixels at or below the alpha-test cutoff
(0.25), the same for `.tga` and glTF textures; the pull request that adds it
sets the share that counts and checks it against the 142 stock `.tga`
textures. #915 is asked to add it; if it doesn't, pull request 2 does.

### 5. Model entries and part models

Model entries keep `renderStyle` and `itemEffect` (L13, L15) and gain the
values per item. During the move a name is looked up in the look files
first, then in the code list (L14). Four groups of entries do change, because their look changes kind or
gains a part (follows from L13, L27, L29 and L30; the exception to L14's
"model entries never change"):
- in pull request 3, the 12 items whose item effect draws (the 7 effects of L13) name it as
  their render style instead; none of them has a render style today;
- in pull request 3, the items with `ItemObjectAttribute` glow values (L27)
  get an item effect holding them; `staffOfKundun` and `redSpirit` get them
  in their own;
- in pull request 3, the 10 items with a shine below +3 name it in their
  glow data, the Jewel of Harmony and the Moonstone Pendant lose the render
  style `harmonyShine`, and the Deadly Staff gets its second glow pass as a
  glow value (L30);
- in pull request 4, the Pumpkin of Luck (14,45) and Cursed Castle Water
  (14,64) gain an item effect of the same name that holds their things on
  bones (L29).

**Part models (L28).** The 11 models that are no items (the 9 +15
decorations, the Phoenix Soul Star's wing, inventory armor model 74) get
entries in `Data/Items/Models/PartModels.json`, so their 3 looks can be
render styles. The code keeps deciding when and where they are drawn.

```json
Data/Items/Models/PartModels.json
{ "formatVersion": 1,
  "partModels": [
    { "name": "phoenixSoulStarWing", "file": "Data/Item/sword36wing.bmd",
      "textureFolders": ["Item"], "renderStyle": "phoenixSoulStarWing" } ] }
```

### 6. Checks

The project's three levels (L23). **Fatal:** the data can't be used (broken
JSON or format version, a file of the wrong kind, bad or doubled names,
wrong types and ranges, unknown kinds, motions, blends, shapes, conditions
or places, a part the look's kind doesn't have, a field taken from an item
value the look doesn't declare). **Error:** something named is missing (a
texture, in step 3 an effect type); the game runs without it. **Warning:**
it works but is worth knowing (a mesh number an item's model lacks, per
item; values an item sets that its look doesn't offer; unknown fields; a
look no item uses; a model entry naming a look that exists nowhere). They
show in Data Checks in the areas Render styles and Item effects, and in the
Style Editor under the picked look, whose Apply is off while a fatal one is
left. A glow naming a shine below +3 that doesn't exist is reported like an
unknown glow color (L30).

### 7. The editor

- **Style Editor** (L18, L24, L31): Windows > Style Editor, built like the
  Colors window. A tree with the roots Render styles and Item effects and
  your own folders, drag and drop within a root; New folder, New look,
  Copy, Rename, Remove, Move up, Move down; each look with how many items
  use it and "(code)" for looks not moved yet (read-only); the picked look with Apply to the look, Revert, Remove,
  "Used by N items:" and the confirm that lists them (D26).
- **Layers** (L19): the list in drawing order, one line each written from
  its fields, with + Add, Copy, up, down, Remove, and "Item values after
  drawing" (or "before drawing") as the last line; the picked line's fields
  below, with the value graphs and the "Each item sets this" boxes.
- **Preview** (L20): the Item Model preview (any item, its level,
  excellent, ancient, inventory or ground, turning), showing changes before
  they are applied, plus stopping and moving time, an eye per layer, and
  "Draw as: doppelganger".
- **Item Model window** (L12, L21, L22, L24, L30): the section **Style and
  effect** with the two dropdowns, "Values for this item" with Reset, and
  "Open in Style Editor". The Glow section gets "Shine below +3" and "Glow
  again in white on mesh". Under Model file the **mesh list**: a row per mesh
  with its number, texture, marks (hidden, glows, scrolls, no chrome, holes)
  and "used by", a tooltip with the texture's size and folder and the mesh's
  vertices and triangles. Pointing at a row or at a number in a mesh field
  draws the other meshes see-through, a click keeps it, and clicking a part
  in a preview picks its row.
- **Data Checks** (L23, L24): the new areas Render styles and Item effects;
  the graphics area renamed Graphics look.

### 8. The move and its proof

Looks move by building block (L17), in the pull requests of section 9. Each
move is proven by a **draw recorder** in the tests (L25), a drawer put in
place of the default `ModelDrawer`: every item that
uses a moved look is drawn with the old code and with the data in the same
build, over clock times, levels +0, +7, +9, +11, +13 and +15, normal, doppelganger and in a
monster's hands, worn, in the inventory and on the ground, opaque and
fading, and per look its buffs and hidden-mesh states, set up by the game's
own `ItemObjectAttribute`, with the clock and random numbers pinned. The
records must be equal. Then the moved functions and their code names are
deleted, and a test checks that no name is both in data and in code (L14).
The quirks of L16 are fixed instead of copied (four are left: #950 already made the Wing of Storm and Symbol of Kundun read the clock),
each in its own commit with its expected differences and an in-game
before/after.

**Step 2** (L7) is FX1.7c: the live preview of effects in an editor view,
with a short design of its own first. **Step 3** adds things on bones to
item effects, spawned in the update from the item's own pose (ADR 0003;
only item effects hold them, L29): sprites, particles and lightning by their
FX1 names, the
SubType a number (`{ "joint": "jointThunder", "subType": 14, "fromBone": 3,
"toBone": 4 }`, D30), with chances per frame and values as in section 2,
shown live in the Style Editor. The bone effects of held and worn items
(L27) follow after a survey of their own that sorts the looks in
`SpawnLinkedPartEffects` and `SpawnHeldWeaponGlow` (in the update since
#931) from the other code there.

### 9. Pull requests

| PR | Contents | Waits for |
|---|---|---|
| 0 | This design, for review (L26) | – |
| 1 | The mesh list and showing a mesh (L21, L22), the names of L24 | – |
| 2 | Step 1 part 1: loader, the two look files, layer drawing with the shared mesh rules, the holes flag if #915 has none, the draw recorder, the checks, the Style Editor; L17's groups 1 and 2 (103 render styles) | – |
| 3 | Step 1 part 2: L17's groups 3 to 5 (50 looks), `ItemObjectAttribute`'s glow values, the part models with the 3 looks of `RenderModelWithoutStyle`, the shines below +3 and the Deadly Staff's glow pass as glow data (L30) | #915 merged |
| 4 | Steps 2 and 3: FX1.7c, the 12 looks with things on bones | – |
| 5 | The bone effects of held and worn items, after their survey; then task 8 (the docs, this document deleted) | PR 4 |

Each goes up as a fork pull request for CI and an upstream one, with one
commit or more per group inside (L26).

### 10. Not in this work

- A pose worn by a character in the Style Editor's preview (L20).
- A part for raw engine values in a layer (L8); it can be added without
  changing data.
- When and where the part models are drawn, as data (L28).
- Monster and NPC looks, in the same files (MN8, L15); effect behavior as
  data (FX2).
- The ground scales `ItemObjectAttribute` sets for some items (ground
  display data, L27).

## Open questions

Answered one by one, each with the current state, every option with an
example, and a recommendation.

Answered: Q-L1 (L7), Q-L2a (L8), Q-L2b (L9, L10), Q-L2c (L11), Q-L3 (L12),
Q-L4a (L13), Q-L4b (L14), Q-L4c (L15), Q-L5a (L16), Q-L5b (L17), Q-L6a to Q-L6c (L18 to L20), Q-L7a and Q-L7b (L21, L22), Q-L8 (L23), Q-L12 (L24), Q-L9 (L25), Q-L10 (L26), Q-L11 (L27), Q-L13 (L28), all on 2026-11-01; Q-L14 (L29), Q-L15 (L30) and Q-L16 (L31) on 2026-11-03. All questions are answered.

## Tasks

The design, then the pull requests of L26 and L27:

1. [x] Write this design: Q-L1 to Q-L16 are answered and the target design
   is written from them, refreshed against main on 2026-11-02 and
   2026-11-03.
2. [ ] Pull request 0, the design: this document, for review.
3. [ ] Pull request 1, the mesh list (L21, L22) and the names of L24.
4. [ ] Pull request 2, step 1 part 1: the machinery, the Style Editor, L17's
   groups 1 and 2.
5. [ ] Pull request 3, step 1 part 2: L17's groups 3 to 5 (after #915 is
   merged), with `ItemObjectAttribute`'s glow values (L27), and the part
   models with the 3 looks of `RenderModelWithoutStyle` (L28), and the
   shines below +3 and the Deadly Staff's glow pass as glow data (L30).
6. [ ] Pull request 4, steps 2 and 3: FX1.7c, the 12 looks with things on
   bones.
7. [ ] Pull request 5: the bone effects of held and worn items (L27), after a
   survey of their own.
8. [ ] The end, in the last pull request of the work:
   - move what stays true into `docs/item-data.md` and `docs/effect-data.md`
     (the look format, the Style Editor, the live preview), and point the
     roadmap (row "Items 13"), the items design (phase 13) and the possible
     follow-up ideas (FX1.7c) at those docs;
   - delete this document.
