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
  `ItemRenderStyles.cpp`) as data where they fit; layers per mesh; the mesh
  list; the look editor.
- **To decide (Q-L1):** the item effects (`itemEffect`, `ItemEffects.cpp`)
  that place sprites, particles and lightning on bones. D25 includes them.
- **Later:** monster and NPC looks (MN8) use the same format (D25: one look
  format for all areas); effect behavior as data is FX2.

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

| Q | Question |
|---|---|
| Q-L1 | The building blocks of the first version: draw passes only (chrome, metal, glow, a texture of its own, scrolling, pulsing), or also things placed on bones (D25). |
| Q-L2 | The fields of a layer: meshes, texture, color or tint, alpha, order (under where the texture has holes, or over), scrolling, pulse, conditions (item level, doppelganger). |
| Q-L3 | Where looks live: named looks in `Data/Effects/` that model entries name (D25, D26), layers inside the model entry, or a named look with meshes chosen per item. |
| Q-L4 | How `renderStyle` and the new looks relate while the styles move (same names, one style at a time). |
| Q-L5 | Which of the 140 styles fit the first building blocks (a survey), and the order of moving them. |
| Q-L6 | The look editor: in the Item Model window under Looks, or a window of its own (D25: one look editor for all areas); layer list, preview, the items that use a look. |
| Q-L7 | The mesh list: what each row shows (number, texture, hidden, glowing, has holes), and whether hovering a row highlights the mesh in the preview. |
| Q-L8 | Data checks: mesh numbers the model doesn't have, missing textures, unknown kinds. |
| Q-L9 | How each moved style is compared with its old code (the render recorder). |
| Q-L10 | The pull requests and their order. |

## Tasks

1. [ ] Write this design: answer the open questions one by one (in progress).
2. [ ] The pull requests from Q-L10 (filled in once it is answered).
3. [ ] Move what stays true into `docs/item-data.md` (and `docs/effect-data.md`
   for the look format), and point the roadmap (row "Items 13") and the
   items design (phase 13) at those docs.
4. [ ] Delete this document.
