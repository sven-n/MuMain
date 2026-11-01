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
| L10 | Glow mesh, scrolling and opacity (Q-L2b, 2026-11-01) | **Fields of each layer**, empty for the item's value: glow mesh (none, a mesh, or all meshes), glow strength, scrolling mesh, scrolling (U and V), opacity (the item's, the drawing's, a number or a shape). **Item values of the look:** the glow mesh, glow strength, scrolling mesh, scrolling, opacity and hidden mesh it leaves on the item for what is drawn after it (the +7 glow, the excellent glow), as `masterScepter` leaves glow mesh 1 (`ItemRenderStyles.cpp:338-352`). The 11 item effects that only change values become looks with item values and no layers. When item values apply (before the layers, as item effects do today, or after them, as styles do): Q-L4. |

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
- Step 1 of L7 is meant for those 137 styles and 16 item effects (Q-L5
  checks that each one fits), step 3 for the other 3 styles (`pumpkinOfLuck`,
  `cursedCastleWater`, `helperNpcPlate`) and 9 item effects.

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

Answered: Q-L1 (L7), Q-L2a (L8), Q-L2b (L9, L10), all on 2026-11-01.

| Q | Question |
|---|---|
| Q-L2c | Conditions (doppelganger, inventory and ground, item level, which item): on a layer, or on the whole look. |
| Q-L3 | Where looks live: named looks in `Data/Effects/` that model entries name (D25, D26), layers inside the model entry, or a named look with meshes chosen per item. |
| Q-L4 | How `renderStyle` and the new looks relate while the styles move (same names, one style at a time); how an item's render style and item effect relate as looks, and so when a look's item values apply (before its layers, as item effects do, or after, as styles do). |
| Q-L5 | Which of the 140 styles fit the first building blocks (a survey), and the order of moving them. |
| Q-L6 | The look editor: in the Item Model window under Looks, or a window of its own (D25: one look editor for all areas); layer list, preview, the items that use a look. |
| Q-L7 | The mesh list: what each row shows (number, texture, hidden, glowing, has holes), and whether hovering a row highlights the mesh in the preview. |
| Q-L8 | Data checks: mesh numbers the model doesn't have, missing textures, unknown kinds. |
| Q-L9 | How each moved style is compared with its old code (the render recorder). |
| Q-L10 | The pull requests and their order. |

## Tasks

1. [ ] Write this design: answer the open questions one by one (in progress;
   Q-L1, Q-L2a and Q-L2b done).
2. [ ] Step 1 of L7: draw passes and conditions (the pull requests from
   Q-L10).
3. [ ] Step 2 of L7: FX1.7c, its short design, then the live preview.
4. [ ] Step 3 of L7: things on bones.
5. [ ] Move what stays true into `docs/item-data.md` and `docs/effect-data.md`
   (the look format, the live preview), point the roadmap (row "Items 13"),
   the items design (phase 13) and the possible follow-up ideas (FX1.7c) at
   those docs.
6. [ ] Delete this document.
