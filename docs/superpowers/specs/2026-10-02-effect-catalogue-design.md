# Effect Catalogue (FX1) — Design

> **Draft, 2026-10-02.** Area FX1 of the
> [data-driven content roadmap](2026-09-29-data-driven-content-roadmap-design.md).
> Written from the code of MuMain `upstream/main` @ `b9362c5a` (with items
> phase 4d2). The owner decided D27–D42 on 2026-10-02, except D36, which is
> marked *(to decide)*.
>
> The counts come from scripts that parse the code (comments and switched-off
> blocks removed, case groups and call arguments read), not from a compiler.
> Where a case body is sorted into "values" or "logic", a borderline body can
> be off by one. Phase FX1.1 checks the exact type lists once more.

## Goal

Every effect, particle, joint ("lightning" in the roadmap) and sprite type
gets a name, and the creation values of effects move into data in
`src/bin/Data/Effects/`. Items, skills and monsters then name effect types
in their data instead of using type numbers in code (items phase 13, SK2,
MN). Behavior stays code: how effects move, spawn and draw moves into data
later, in FX2.

As for the items: nothing changes in game, nothing gets slower in game
(names are resolved when loading), and every move from code into data is
compared with the old code before the old code goes.

## Decisions

The numbers go on from the items design and the roadmap (D1–D26), so a
decision can move to the roadmap without being renumbered.

| # | Topic | Decision |
|---|---|---|
| D27 | Scope | FX1 names the types of the four creation systems: effects (`Effects[]`), particles, joints and sprites. Creation values move into data only for effects, which have the registry and its `CreateParams`. Trails (`CreateBlur`) come with the swing trails of items phase 13; damage numbers, ground decals, weather and the `CreateFire` recipes are not in FX1. |
| D28 | Identity | The type number stays the key at runtime. Each data entry has a `name` and a `code` field with the enum symbol it stands for (`"code": "MODEL_KENTAUROS_ARROW"`). A compiled list of the symbols per kind turns them into numbers once, when loading. The data never stores raw numbers, because they move with the `#ifdef` blocks of the enum headers. |
| D29 | One namespace per kind | A name is unique within its kind: effect, particle, joint, sprite. Data that references a type gives the kind in the field name (`"particle": "smoke"`). The editor shows joints as "Lightning and trails". |
| D30 | Names for types | FX1 names (kind, type). The SubType stays a number in references (`{"joint": "jointThunder", "subType": 14}`), because it is not always a variant: it is also runtime state, a random value, a model number and the sprite blend mode. |
| D31 | How names are chosen | From the enum name in camelCase without `MODEL_`/`BITMAP_`, with collisions and marker names fixed by hand, misspellings corrected (`explotion` → `explosion`), and the types without an enum name named after the file loaded into their slot (`MODEL_SKILL_FURY_STRIKE+1` loads `EarthQuake01` → `earthQuake1`). The full list is proposed in the FX1.1 PR and reviewed there. Types that are never created get entries too; the dead ones are listed in an upstream issue. |
| D32 | Files and house rules | One file per kind: `EffectTypes.json`, `ParticleTypes.json`, `JointTypes.json`, `SpriteTypes.json`. The rules of the item and model files: `formatVersion`, sorted by name, fixed field order, defaults left out, a writer and a test that the shipped files are in its format, names of letters and digits, errors stop the start, warnings go to the log. Documented in `docs/effect-data.md`. |
| D33 | Loading and lookup | The catalogue is loaded once on the loading screen, next to the item model data, before the first effect is created. The registry table is built from it: the same array indexed by type, not changed after loading. A lookup stays a bounds check and one array read. No names, strings or allocations after loading. |
| D34 | Creation values | A registry row replaces the whole legacy case, so a type moves only when every statement of its case can be written as data. `CreateParams` gets exactly the fields the moved cases need. Values are applied in one fixed order (values, then offsets, then copies). An unset field keeps what the common setup chose, or the slot's old value where the common setup sets nothing. Values the old code multiplies by `FPS_ANIMATION_FACTOR` keep that as a flag. |
| D35 | Variants by SubType | A row can hold `variants` keyed by SubType that override its values (44 cases, 47 types choose only values by SubType). |
| D36 | Random and logic creation *(to decide)* | Cases with `rand()` and the cases with logic stay code in FX1 (167 of 244 cases). Data with random values would have to draw `rand()` in exactly the same order and number. |
| D37 | Particles and joints | Names only in FX1; their creation values wait for FX2. They have no registry, their structs differ from effects, and their creation is mostly random formulas. |
| D38 | Sprites | Sprite entries are names for the textures sprites draw, without values: `CreateSprite` has no values per type, and the blend is chosen per call. |
| D39 | What stays code | The per-type lists (`IsSkillEffect`, the shadow list, `DrawCaseOverridesBlend`, …), the range rules, all move and render handlers, the creation hooks, loading assets into slots, and the code call sites, which keep their enum constants. |
| D40 | Verification | A recorder in the test binary compares the old and the new creation for every moved type (see Verification). It is committed as a test tool, with a seed function for `Random::`, because FX1.3 to FX2 all need it. |
| D41 | Fixes | The data reproduces the old behavior with its quirks. The bugs found while writing this document are fixed in its PR (the owner's choice, see "Fixed with this document"); bugs found later are filed on sven-n/MuMain and fixed in their own PRs. |
| D42 | Effect browser | Read only in FX1: lists per kind, details, "used by" from data, and a preview in the world in editor builds. Values are edited in the FX2 effect editor. |

## Current state

### One number space, four kinds

There is no effect enum. Every type is a `MODEL_*` value (0–10121,
`Core/Globals/_enum.h`) or a `BITMAP_*` value (30000 and up,
`Core/Globals/_TextureIndex.h`), and the number is also the asset slot:
particles and joints start with the texture of their number
(`ZzzEffectParticle.cpp:72`, `ZzzEffectJoint.cpp:85`), sprites draw the
texture of their number, and effects with a `MODEL_*` number draw
`Models[o->Type]`. The values move when macros change: `_TextureIndex.h`
has 8 `#ifdef` blocks and `_enum.h` 3, so raw numbers are no stable keys
for data.

| Kind | Code | Types | Pool | Registry |
|---|---|---|---|---|
| Effect (`OBJECT Effects[]`) | `ZzzEffect.cpp` | 464 numbers (406 `MODEL_*`, 58 `BITMAP_*`); 455 have code | 200 + 100 for the hero's skill effects | 308 types |
| Particle (`PARTICLE`) | `ZzzEffectParticle.cpp` | 98 (all `BITMAP_*`, in 79 case groups) | 3000 | none |
| Joint (`JOINT`: lightning, beams, trails of the FLARE and SPIRIT kind) | `ZzzEffectJoint.cpp` | 31 (29 `BITMAP_*`, 2 `MODEL_*`) | 500, each with up to 200 trail segments | none |
| Sprite (one frame, camera-facing) | `zzzeffectsprite.cpp` | 44 textures at literal call sites, 54 with the computed ones | 1000 | none |

- **Shared numbers.** The kinds use 560 distinct numbers. 55 of them are a
  type in two or more kinds, with different code in each; 4 are a type in
  all four (`BITMAP_LIGHT`, `BITMAP_SPARK+1`, `BITMAP_FLARE`,
  `BITMAP_PIN_LIGHT`). Code also passes an effect's own number on to
  `CreateParticle` or `CreateSprite` (`ZzzEffect.cpp:1288`, 9303), so it
  relies on the numbers being equal across kinds.
- **Names.** Most numbers have a descriptive enum name. 36 have none and
  are written as `BASE+n` (`MODEL_SKILL_FURY_STRIKE+1..+8`,
  `BITMAP_MAGIC+1`) or as a bare number (`CreateEffect(9, …)` in
  `GMHellas.cpp`). 9 more only have a `_BEGIN`/`_END` marker name
  (`BITMAP_SHINY+6` is `BITMAP_STONE_BEGIN`). The enum names in camelCase
  without the prefix collide 6 times among the effects (`MODEL_FIRE` and
  `BITMAP_FIRE`) and once among the sprites.
- **Computed numbers and ranges.** About 140 call sites compute the type
  (`MODEL_STONE1 + rand() % 2`), and code depends on ranges: the default
  drawing only for 225–642 (`ZzzEffect.cpp:9643`), the model animation only
  from `MODEL_BIRD01`. So the numbers must stay as they are.

### The registry

The registry of sven-n/MuMain#493 (`Render/Effects/EffectRegistry.cpp`,
`EffectDef.h`) describes an effect type by up to four stages: creation
values (`CreateParams`), a creation hook, a move handler and a render
handler. A stage that has an entry replaces that stage's case in the old
switch; a creation entry returns before the switch
(`ZzzEffect.cpp:392-399`).

- 308 types have an entry: 32 with `CreateParams`, 2 with a creation hook,
  302 with a move handler (287 copied out of the old switch into 202
  functions in `Behaviors/MoveHandlers.cpp`, 15 written by hand), 1 with a
  render handler.
- `CreateParams` has lifeTime, scale, velocity, gravity, hiddenMesh,
  blendMesh, blendMeshLight, alpha, light and copyLightToDirection. The 32
  rows use lifeTime (32), scale (15), blendMesh (8), alpha (6), light (2),
  velocity (2), blendMeshLight (1) and copyLightToDirection (1).
- The rows are C++ (`EffectRegistry.cpp:39`). The table is an array of
  32,448 pointers indexed by type (the largest type is `BITMAP_DAMAGE1`),
  built on first use. A lookup is a guard, a bounds check and one array
  read; it runs once per creation, once per live effect per frame and once
  per drawn effect per drawing pass.
- No test covers the registry, and the script that extracted the move
  handlers was not kept.

### Creation today

- **Common setup.** `CreateEffect` takes the first free slot (and calls
  `IsSkillEffect` for every slot it tries), then sets 28 values
  (`ZzzEffect.cpp:345-389`). It does not reset LifeTime, Gravity,
  StartPosition, HeadAngle, Distance, Timer, Weapon and others, so these
  keep the values of the slot's previous effect unless the case sets them.
  Particles do not reset Alpha and TurningForce; joints do not reset Scale,
  LifeTime and MaxTails.
- **The 244 cases of the effect creation switch** (417 types):

| Cases / types | What they do | Example |
|---|---|---|
| 7 / 8 | Only values that today's `CreateParams` holds | `MODEL_KENTAUROS_ARROW`, `ZzzEffect.cpp:532` |
| 26 / 26 | Only values, but more fields | `MODEL_DRAGON` (`:403`): CollisionRange, Kind, Timer, Distance, an angle, a position offset, Direction, StartPosition from Position |
| 44 / 47 | Only values, chosen by SubType | `MODEL_MAGIC_CIRCLE1` (`:1477`) |
| 167 / 336 | Logic: spawning other effects (50 cases), global state (39), owner or skill values (17), only random values (47 cases, 172 types, mostly debris), other math (14) | `MODEL_GAION`, about 700 lines |

- **Other inputs.** Effect creation calls `rand()` 471 times in 88 cases;
  63 cases multiply values by `FPS_ANIMATION_FACTOR`, so creation depends on
  the frame rate; 30 cases use the owner, and 3 of them check it for null.
- **Particles** have about 405 creation variants by SubType
  (`BITMAP_SMOKE` alone 69), 1,123 `rand()` calls and 238 values multiplied
  by the frame factor. **Joints** have about 344 (type, SubType) variants,
  137 `rand()` calls, and reuse call arguments as values (PKKey becomes the
  lifetime). **Sprites** only copy the call's arguments; SubType is the
  blend mode.

### Callers

4,256 calls create effects (1,194), particles (1,430), joints (455) and
sprites (1,177). 969 of them are inside `Render/Effects` (effects that
create other effects). The other 3,287 come from monsters and NPCs (1,538),
maps (588), items (418), skills (388), events (149), pets and mounts (81)
and others. The item effects of phase 4c3 (`ItemEffects.cpp`) make 31 calls
on 10 types. SubType picks the variant: 116 effect, 62 particle and 21
joint types are created with more than one SubType.

### Data, tests, CI

- The only effect data so far is `Data/Effects/GlowColors.json` (45 glow
  colors), read with the item data helpers and loaded on the loading screen
  (`OpenBasicData`, before any scene creates effects).
- Tests: `tests/effects/test_particle_draw_order` and
  `test_aura_joint_lifecycle`; none for the registry. `tests/data` links the
  whole client, so a test can call `CreateEffect`, but `Hero` is not set
  there and `IsSkillEffect` reads it.
- CI: cppcheck checks every changed file in full. It failed on
  `ZzzEffect.cpp` (an out-of-bounds write, an unused variable, a macro call
  without its semicolon) and on `MoveHandlers.cpp` (an uninitialized
  variable, a self-assignment, a null check after a dereference); these are
  fixed with this document (FX1.P), so later PRs can change the files.

## Target design

### Data files

One file per kind in `src/bin/Data/Effects/` (D32), sorted by name, with
default values left out. `code` is the enum symbol (D28); only the bare
Hellas number is written as `"9"`.

```json
{
  "formatVersion": 1,
  "kind": "effect",
  "types": [
    { "name": "cundunGhost", "code": "MODEL_CUNDUN_GHOST",
      "create": { "lifeTime": 200, "scale": 1.8, "velocity": 0.08, "blendMesh": -2, "light": [0.5, 0.5, 0.5] } },
    { "name": "earthQuake1", "code": "MODEL_SKILL_FURY_STRIKE+1" },
    { "name": "kentaurosArrow", "code": "MODEL_KENTAUROS_ARROW",
      "create": { "lifeTime": 34, "scale": 0.7, "velocity": 70, "alpha": 0, "light": [1, 1, 1] } }
  ]
}
```

```json
{ "formatVersion": 1, "kind": "particle",
  "types": [ { "name": "smoke", "code": "BITMAP_SMOKE" }, { "name": "smoke2", "code": "BITMAP_SMOKE+1" } ] }
```

- `name`: letters and digits, unique per kind; every compiled symbol of a
  kind is named exactly once.
- An unset `create` field keeps the common setup, or the slot's old value
  where the common setup sets nothing (D34). There are no new defaults.
- A vector field can set single components (`"angle": { "y": 0 }`); a value
  the old code multiplies by the frame factor is written
  `{ "value": -100, "timesFrameFactor": true }`.
- Unknown fields are warnings. Unknown symbols, duplicate names and a newer
  `formatVersion` are errors that stop the start.

### Loading and lookup

- A compiled list per kind (kind, enum symbol) in `Data/GameData/EffectData`
  turns the symbols into numbers. It only depends on the enum headers.
- Loaded in `OpenBasicData` with the item model data. The load time is
  logged.
- For effects, the registry's array of descriptors (32,448 entries) is
  built once after loading. Lookups stay a bounds check and one array read;
  names are only used while loading, for error messages and in the editor.
- Creating an effect before the catalogue is loaded is logged as an error,
  because moved types no longer have their old case.

### How other data references types

Not part of FX1; used from items phase 13 on (and SK2, MN). The field name
gives the kind, the SubType stays a number (D30):

```json
{ "sprite": "spark2", "bones": [1, 2], "scale": 2.5 }
{ "joint": "jointThunder", "subType": 14, "fromBone": 3, "toBone": 4 }
```

### Effect browser

A MuEditor tool, in editor builds only, read only in FX1 (D42):

- One tab per kind (Effects, Particles, Lightning and trails, Sprites),
  with search and filters (has creation values, creation in data, hook or
  switch, asset loaded on the current map).
- Details: name, code symbol and number (and the other kinds that use the
  same number), creation values and variants, which stages are data, hook
  or switch, and what the slot holds right now (model or texture file, or
  "nothing loaded on this map"; 143 effect models are loaded only by maps
  or events).
- "Used by" from data users (D26): the registry rows in FX1, the looks of
  items from phase 13 on, later skills and monsters.
- Preview: creates the type in the world in front of the hero, with the
  hero as owner and a chosen SubType; sprites are created again every frame
  while previewing.

## Phases

One PR each, small enough to check against the old code.

| Phase | Name | Depends on | Summary |
|---|---|---|---|
| FX1.0 | Design document | items 4c | This document; the roadmap links it and gets the new counts. |
| FX1.P | cppcheck fixes | – | Done with FX1.0: the findings of cppcheck in `ZzzEffect.cpp` and `MoveHandlers.cpp` are fixed, so later PRs can change these files. |
| FX1.1 | Names for all types | FX1.0 | The compiled symbol lists, the four catalogue files with `name` and `code`, loader, validation, writer, `docs/effect-data.md`. No game code reads the catalogue yet. |
| FX1.2 | Registry rows from data | FX1.1 | The 32 `CreateParams` of `EffectRegistry.cpp` move into `EffectTypes.json`; the registry table is built from the catalogue at loading. Handlers stay C++. |
| FX1.3 | The 8 types that fit `CreateParams` | FX1.2 | Their cases move into data and are deleted. Sets up the recorder. |
| FX1.4 | More creation fields | FX1.3 | The fields the 26 value-only cases need; those cases move into data. |
| FX1.5 | Variants by SubType | FX1.4 | `variants` in effect rows; the 47 types that choose values by SubType move. |
| FX1.6 | Effect browser | FX1.1 | Read-only tool in MuEditor; values from FX1.2 on. |
| FX1.7 | Preview | FX1.6 | Creating the selected type in the world in editor builds. |

**FX1.1 Names for all types.** The compiled list of symbols per kind (about
640 lines), generated once by a script that stays in `tools/`. The four
files with `name` and `code`, the `Data::Effects` loader with validation and
a writer, and the arrays from number to name for logs. Loaded on the loading
screen; nothing under `Render/Effects` changes, so the frame time cannot
change. Checked by tests (every symbol has exactly one name in its kind,
names are unique and valid, the shipped files are in the writer's format),
by a one-time comparison of the compiled lists with the case labels,
registry entries and call sites of the four systems, and by the logged load
time.

**FX1.2 Registry rows from data.** The 22 rows (32 types) of `CreateParams`
move into `create` objects of `EffectTypes.json`, and the registry table is
built from the catalogue, without the guard of today's static table. Only
`EffectRegistry.cpp`, `EffectDef.h` and the data change. Checked by a
one-time test that `Lookup` gives the same values and handlers as the old
rows for every number, by the recorder for the 32 types, and by a Release
benchmark of creation and lookup (old against new).

**FX1.3–FX1.5** move effect cases into data in growing steps: first the 8
types whose cases only set fields `CreateParams` has, then the 26 that need
more fields (angle, direction, position offset, StartPosition from
Position, Kind, Timer, Distance, CollisionRange, …), then the 47 that
choose values by SubType. Each step deletes the moved cases and is checked
with the recorder.

**FX1.6–FX1.7** add the effect browser and its preview to MuEditor. They
change no game code outside editor builds.

## Verification

Every PR that moves values is checked with a recorder in the test binary
(D40), as the items phases did, but committed as a test tool, because FX1.3
to FX2 all need it:

- **Fixed inputs:** a fixed `srand` seed, a seed for `Random::` (today
  seeded from `random_device`, without a way to set it), the frame factor
  pinned to 1.0 and 0.5, a fixed `WorldTime`, and the hero and owners set
  up with the game's own setup functions.
- **Slots:** every pool slot filled with pattern A, then pattern B, so the
  types that read old values of a slot show up.
- **Cases:** every moved type with each SubType its case handles, every
  SubType callers pass, and one that no case handles.
- **Recorded:** the whole created slot, all pools (effects, skill effects,
  particles, joints, sprites), a `rand()` and a `Random::` sentinel, and
  the global state some cases change.
- **Baseline:** the old case stays reachable in the PR's working commits
  and is deleted after the comparison; spot checks stay as tests.
- **Speed:** a Release benchmark of creation and lookup, old against new,
  and the frame profile of the effect rows in a busy scene.

## Risks

- **Random draws.** `rand()` is shared by everything, and `MoveEffect`
  draws one per live effect per frame. Any data path must make the same
  draws in the same order and number, or every later random value differs.
  This is why random creation stays code in FX1 (D36).
- **Frame rate inside creation.** The data keeps the multiplications by
  `FPS_ANIMATION_FACTOR`; the recorder runs at two frame factors.
- **Old values of a slot.** Fields that the common setup does not reset
  carry over from the previous effect in the slot. A loader that adds
  defaults would change looks, so unset fields stay unset (D34).
- **A row replaces the whole case.** A missed statement changes the look
  without an error. Only cases that are fully expressible move (D34).
- **Loading order.** The registry table comes from data; an effect created
  before loading would find neither its row nor its old case. Loading on
  the loading screen is early enough today, and a violation is logged.
- **Numbers must not change.** About 1,570 places in the effect code and
  148 delete or search calls use the numbers, and ranges and computed types
  depend on them. Names only stand for today's numbers.
- **Speed.** Lookups stay array reads; more optional creation fields cost a
  few branches per creation, measured against the old compiled cases.
- **Assets depend on the map.** 143 effect models are loaded only by map or
  event code, and slots from 160 up are not released when the map changes.
  Recorder runs and previews pin the map.
- **Names last.** Once looks reference them, renames go through D26, so
  poor generated names are fixed in FX1.1.

## Fixed with this document

The owner chose to fix the bugs found while writing this document in its
PR (D41). Each fix changes only what was broken:

- **Summoner casting effects** (`ZzzEffect.cpp`, `MODEL_SUMMONER_CASTING_EFFECT1`
  and its five siblings): `if (o->SubType = 0)` assigned instead of
  comparing, so every casting effect lost its SubType and the scale of
  SubType 0 was never set. Now SubType 0 gets the scale 1.0 the code meant
  (the summon skills' casting circles, 0.9 before); the casts with SubType 1
  keep their scale 0.6. The move and draw code does not read their SubType.
- **The effect `BITMAP_JOINT_FORCE`** (Battle Castle, Aida): its move code
  fell through into the move code of `MODEL_SWORD_FORCE` in the original
  client: in Battle Castle (SubType 0) the effect grew, spawned sword force
  effects, sparks and fire and lit the ground; in Aida (SubType 1) it faded
  out. When sven-n/MuMain#493 moved that code into a handler, the case
  started to fall through into `MODEL_EFFECT_SAPITRES_ATTACK_1` instead,
  which pushed the Battle Castle effect away by 40 times its direction
  every frame and spawned that monster's attack effects, and left the Aida
  effect without its fade. It runs the sword force handler again, as before
  #493.
- **The particle `BITMAP_SPARK + 1`, SubType 7** (`ZzzEffectParticle.cpp`):
  the position jitter was written to y, z and past the end of the position
  (`Position[3]`, which is `Angle[0]` of the particle). Now it goes to x, y
  and z; the three random draws stay the same.
- **The blend-mesh passes** (`RenderEffects`, `RenderAfterEffects`): on water
  maps they read `Models[o->Type]` for effects with a texture number
  (`BITMAP_*`), far past the end of the model array. Effects without a model
  skip that check now.
- **cppcheck findings** (FX1.P): four path points kept in an array of three
  (`arv3PosProcess` in the creation of the Gaion swords, an out-of-bounds
  write, also MSVC warning C4789), an unused variable and a statement
  without effect, and a macro call without its semicolon in `ZzzEffect.cpp`;
  in `MoveHandlers.cpp` a distance read before it was set (now the distance
  to the target, as the first move step would measure it), a
  self-assignment, and a null check after the pointer was used
  (`MODEL_ALICE_DRAIN_LIFE`). None of these changes a value in the game.
- With the files passing cppcheck, the last conversions of items phase 4d2
  follow: `RenderWheelWeapon` and `RenderFuryStrike` use `ToModelSlot`, and
  the spear check of the move handlers uses `ITEM_SPEAR`.

Still to check and file upstream: the owner is used without a null check in
27 creation cases; 21 effect, 6 particle and 2 joint types have code but are
never created (a script check; computed types may reach some), and 9 effect
types are created but have no code (`MODEL_EX01_SHADOW_MASTER_*`).

## Open questions

- **Random creation (D36).** Random and logic creation stays code in FX1,
  or random ranges in data for the simple cases (with a defined draw
  order, proved by the recorder)?
- **Sprite list.** 44 textures at literal call sites, or 54 with the
  computed ones; settled in FX1.1.
- **Browser, later.** A generated index of the code call sites for "used
  by", besides the data users.
