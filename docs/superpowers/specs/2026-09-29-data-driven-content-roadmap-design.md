# Data-Driven Content Roadmap (WIP)

> **Working document.** It keeps the order of the data-driven refactorings
> that go beyond one area, the decisions they share, and the editor parts
> they share. Each area has its own design document with its own phases;
> this document links them and stays while areas are open.
>
> Started from the items design
> ([2026-09-25-data-driven-items-design.md](2026-09-25-data-driven-items-design.md)),
> based on MuMain `upstream/main` @ `e4831483`.

## Goal

Content that is hardcoded in the client today (items and their looks,
effects and particles, skills, monsters and NPCs, ...) becomes data that
can be changed and edited in MuEditor without code. What belongs together
is planned together: items, skills and monsters all use the effect code,
so they share one way to describe looks and one set of editor parts.
Nothing gets slower in game: names and values are resolved when loading.

## Why these areas belong together

Items are not the only users of the effect code. About 4,250 calls create
effects, particles, lightning and sprites: about 1,540 for monsters and
NPCs, 590 for the maps, 420 for items, 390 for skills, 970 inside the
effects themselves, and the rest for events, pets and the server's skill
results (counted for the
[effect catalogue design](2026-10-02-effect-catalogue-design.md)). The
effect code knows 464 effect types, 98 particle types, 31 lightning types
(joints) and about 50 sprite textures; `Render/Effects/EffectRegistry` has
started to describe them as a table instead of three large switches. Skills are
still read from `Skill.bmd` (with a table editor, as items had before
their phase 2), and monsters and NPCs are set up in `CreateMonster` (a
switch of 269 cases in `ZzzCharacter.cpp`), 15 creators and 9 map processes
outside it (see the
[monsters design](2026-10-23-data-driven-monsters-design.md)).

## Shared decisions

These decisions were made in the items design (D25, D26) and the effect
catalogue design (D43); they keep their numbers, so the references there
stay valid.

| # | Topic | Decision |
|---|---|---|
| D25 | Looks in data | Items, skills, monsters and NPCs reference their looks by name (for items: the render style and the item effect; the glow colors already are names in data). What a named look is made of moves from code into data files in `Data/Effects/`: a list of building blocks, namely draw passes (mesh or body, flags, texture, color, alpha, texture scrolling), things placed on bones (sprites, particles, lightning between two bones, effects), animated object values (glow mesh brightness, hidden mesh, texture scrolling), timing (pulses, random chances per frame) and conditions (item level, doppelganger, ...). The drawing code runs these lists; data that names looks stays valid when their definitions move. One look format and one look editor for all areas: a look made for an item can be used by a monster. Effect and particle types are referenced by the names of the effect catalogue (FX1) and become data themselves later (FX2). |
| D26 | Shared definitions | Anything that several things use is defined once, with a name, in a data file, and its users reference that name: glow colors, looks, effect types, skills. Editors show where a definition is used (items, skills, monsters, NPCs, other effects). Saving a change to a definition that others use first shows the list of those users; a copy makes a variant for one user; renaming updates all references; a definition that is still used cannot be deleted (the list shows why). Unknown names are reported when loading. Names are resolved when loading, so drawing and game logic never look names up. Every move from code into data is compared with the old code (recorder) before the old code goes. |
| D43 | Logic as data where it is meaningful | Not only values and lists: functions and logic can become configurable data too, wherever changing them is meaningful, that is, something a designer wants to adjust and can understand: how an item's stats change with its level, the bonus of excellent and ancient items, how requirements grow, which items a rule applies to, how an effect moves and fades (FX2). It is written as named tables and building blocks with parameters, not as code transcribed statement by statement. Each move says who would change it and why, stays checkable against the old code, and is resolved when loading. Logic without a meaningful knob (the exact order of random draws in an effect's creation, protocol decoding) stays code. Decided with the effect catalogue design (2026-10-02). |

## Bones

Effects place sprites, particles and lightning on bones of a model, so
they follow its animation: the lights of a wing move as it flaps, the
casting effect of a skill sits in the caster's hand. Looks store bones as
numbers, as the code does, and the model loader checks that they exist.

- **Item models:** the names in the `.bmd` files are export names that do
  not help to find a bone. Of the 778 item model files, 340 have the
  biped skeleton with names like "Bip01 L Hand", 249 only names like
  `Bone01`, `Box02` or `zx12`, in 396 the bone called "BoneNN" is not bone
  NN, and one file has a name twice. Some models have helper bones placed
  as effect points (the 22 blue lights of the Wing of Eternal sit on the
  bones `zx01` to `zx22`).
- **Characters:** skills place effects mostly on the caster's bones
  (hands, weapon), and effects follow their owner's bones. Characters have
  the biped skeleton, whose names are readable ("Bip01 R Hand").

The look editor shows numbers and names together and picks bones by
clicking them.

## Models

The item models that several items use are named shared models in
`Data/Items/Models/SharedModels.json` (items phase 4d1): the file, its
texture folders and none-blend meshes, opened once. Items name them and
keep their own display, glow, render style and item effect. The file may
get more:

- **Level variants** (items phase 12): the event models drawn for level
  variants (the Box of Luck levels, the Devil's Square items, …) become
  models of items and are shared by several of them.
- **Display defaults**: inventory and ground values on the shared model
  that each item can override. For 18 of the 34 shared models all items
  have the same values.
- **Bone labels**: readable names for the bones that looks use (see
  Bones), which belong to the model file, not to an item.

**Decided with the [monsters design](2026-10-23-data-driven-monsters-design.md)
(its D47, agreed 2026-10-23):** monsters, NPCs, skills and map objects also
load `.bmd` files with texture folders and values per model (the animation
speeds of monsters are set in code, each model has its own sounds). Models
get a list per area instead of one list for all areas: monster and NPC models
in `Data/Monsters/Models.json`, item models in `Data/Items/Models`; skills
and map objects decide in their own designs. Models are used across areas
(player-shaped NPCs wear item models, some monsters hold item weapons, the
part models monsters drop are effect types), so the lists reference entries
of other areas.

## Areas and order

| Area | Name | Design document | Depends on | What moves into data | Editor |
|---|---|---|---|---|---|
| Items | Items | [items design](2026-09-25-data-driven-items-design.md), phases 0–16 | – | Item data, rules and categories, models, glow, render styles and effects (as names). | The item tools of section 9 there. |
| FX1 | Effect catalogue | [effect catalogue design](2026-10-02-effect-catalogue-design.md) (draft) | items 4c | Every effect, particle, lightning and sprite type gets a name and its creation values (the `CreateParams` of the registry) in `Data/Effects/`: the effect types (`effectTypes` in the data). Behavior stays code. Items, skills and monsters then name effect types instead of using type numbers; an item effect (`itemEffect`, what an item does every frame before it is drawn) creates instances of effect types. | Effect browser: each effect with a preview, its values, and where it is used. |
| Items 13 | Looks in data (items) | items design, phase 13 | FX1, items 6 | What the item looks are made of (D25). | Look editor. |
| SK1 | Skills as data | not yet | items 2 (the same format rules) | `Skill.bmd` into JSON, like the items in their phase 2: names, requirements, rules; later synced with OpenMU like the items. | Focused skill editors, like the item tools; "used by" (classes, items that give a skill, monsters). A read-only view of what a skill uses: the effect types, models and sounds its code creates and the code places that handle it (from the effect browser's call sites); SK2 turns these into looks. |
| SK2 | Skill looks | not yet | SK1, FX1, items 13 | How a skill looks when cast, flying and hitting (effects, particles, sounds, character animations), as looks (D25), mostly on the caster's bones. | Look editor, with a preview of the skill cast by a test character. |
| MN | Monsters and NPCs | [monsters design](2026-10-23-data-driven-monsters-design.md) (draft), phases MN0–MN9 and MN-B in three PRs now (list and names; models and attacks; setup and NPC rules) and three later ones (sync, looks, cleanup) | items 2 (the same format rules); the OpenMU sync (MN6) needs items 7; the looks (MN8) need FX1 and items 13; skill names in the editor need SK1 | The monster list with translatable names and kind; model entries (files, textures, animation speeds, sounds, bounding boxes, weapon bones); the setup at spawn (model, size, the equipment of player-shaped NPCs, the player transformations); which animation each skill plays (the skills themselves are SK1 and SK2); NPC talk rules; their looks (glow, render styles and effects as in D25; the plate of the helper NPCs, `helperNpcPlate`, moves from the items to them). | Monster tools sharing one selection (browser, setup, attacks, model, preview, translations, checks); the look editor. |
| FX2 | Effect behavior as data | not yet | FX1 | How effects and particles move, fade, spawn and draw, as building blocks, one effect family at a time (continuing the handlers of the registry). New effects without code. | Effect editor with a live preview. |
| later | Map objects, buffs, pets, sounds, NPC dialogues and quests | not yet | FX1 | To decide: the objects of the maps with their effects (most of `World/GameMaps`), buff visuals, pets and mounts (about 97 places choose animations, skills, ride height, camera and the spawned model by the mount; see "Item-specific code that is left" in the items design; also the mount looks as data with a Mount Looks editor, today the Dark Horse rule of `ZzzObject.cpp:827` that the monsters' `mountLook` names, and drawing mounts in Monster Preview: monsters design Q-MN40, Q-MN57), cutscenes as data with a cutscene editor (the Crywolf and Kanturu 3rd scripts: create a monster with a walk speed, wait, move, attack, camera; monsters design Q-MN58), sound names, NPC dialogues and quests (`NPCDialogue.bmd`, `Quest*.bmd`, `QuestWords_<lang>.bmd`), the minimap's NPC markers; maps as data: their names, groups (Devil Square 1–4 and 5–7, Kalima 1–7, Blood Castle 1–8, Chaos Castle 1–7, Illusion Temple 1–6), server numbers and files folders, the code's map helpers (`InChaosCastle` and about 25 others) on that table, and files shared between maps without the 122 MB of identical copies in `Data/World*` and `Data/Object*` (monsters design D67). | – |

- **The effect catalogue comes first.** Its names are what the looks of
  items (items 13), skills (SK2) and monsters (MN) reference. It moves no
  behavior, so it is small and can be checked like the render styles.
- **Skills before monsters,** because monsters use skills. No monster phase
  needs skill data, though: they keep skill numbers, and the monster tools
  show skill names once SK1 exists, so the monster phases can start before
  SK1.
- **Effect behavior comes last.** Once everything references effects by
  name, their behavior can move into data without touching their users.
- **Map objects later,** although the maps are the largest users of
  effects: they only need the effect catalogue, not the looks of items,
  skills or monsters.

## Shared editor parts

The editors of all areas follow D26: focused tools that share one
selection, "used by" lists, a warning with the list of users before a
shared definition changes, copies for variants. These parts are shared
and opened from the item, skill, monster and NPC editors:

- **Look editor** (from items 13 on): edits a named look (D25) as its
  list of building blocks, with a live preview of the model and its
  animations (for skills: cast by a test character). Shows the skeleton
  with every bone's number and name, highlights the bones the look uses,
  and picks bones by clicking them. Lists all users of the look; saving a
  change to a look that others use shows a warning with that list. A new
  look starts as a copy of another.
- **Effect browser** (FX1) and **effect editor** (FX2): an effect alone in a
  preview (shown on nothing, a plane, a cube or an item) and in the world in
  front of the hero, its values, and where it is used.
- **Tool basics** (from MN1 on, monsters design D56): the editor bar's undo
  for every area (`MuEditor::UndoTarget`, each area with a history of its
  own), the names × languages grid, the tool window frame with its undo keys,
  and a docking group and Windows sub-menu per area, made generic from the
  item tools.

## When an area starts

Its design document is written from the code first, like the items design
(current state, decisions, phases), linked in the table above, and this
document is updated. The default verification is the recorder method of
the items design: the old and the new code are run for every model, setup
and level, and what they draw and spawn is compared.
