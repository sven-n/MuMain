# Effect Catalogue (FX1) — Design

> **Draft, 2026-10-02.** Area FX1 of the
> [data-driven content roadmap](2026-09-29-data-driven-content-roadmap-design.md).
> Written from the code of MuMain `upstream/main` @ `99b73b27` (the merge of
> sven-n/MuMain#672, items phase 4d2). Line numbers are those of that code
> with the fixes of this document. The owner decided D27–D42 on 2026-10-02.
>
> The counts come from scripts that parse the code (comments and switched-off
> blocks removed, case groups and call arguments read), not from a compiler.
> Where a case body is sorted into "values" or "logic", a borderline body can
> be off by one. Phase FX1.1 checked the type lists with the compiler: 464
> effect, 98 particle, 31 joint and 53 sprite types.

## Goal

Every effect, particle, joint ("lightning" in the roadmap) and sprite type
gets a name, and the creation values of effects move into data in
`src/bin/Data/Effects/`. Items, skills and monsters then name effect types
in their data instead of using type numbers in code (items phase 13, SK2,
MN). Behavior stays code: how effects move, spawn and draw moves into data
later, in FX2, as building blocks wherever adjusting them is meaningful
(D43 of the roadmap).

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
| D34 | Creation values | A registry row replaces the whole legacy case, so a type moves only when every statement of its case can be written as data. `CreateParams` gets exactly the fields the moved cases need. Values are applied in one fixed order (values, then offsets, then copies); an offset of a field a copy writes adds to the copy, and a copy reads its source as it is then, or the call's argument (`callLight`, `callScale`). An unset field keeps what the common setup chose, or the slot's old value where the common setup sets nothing. Values the old code multiplies by `FPS_ANIMATION_FACTOR` keep that as a flag. |
| D35 | Variants by SubType | A row can hold `variants` keyed by SubType that override its values (44 cases, 47 types choose only values by SubType). |
| D36 | Random and logic creation | Cases with `rand()` and the cases with logic stay code in FX1 (167 of 244 cases). Data with random values would have to draw `rand()` in exactly the same order and number, and values like "a random yaw, then the launch vector turned by it" are small programs. They become data in FX2, as building blocks with parameters where adjusting them is meaningful (D43). |
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
| Sprite (one frame, camera-facing) | `zzzeffectsprite.cpp` | 53 textures (44 at literal call sites, the others computed) | 1000 | none |

- **Shared numbers.** The kinds use 560 distinct numbers. 55 of them are a
  type in two or more kinds, with different code in each; 4 are a type in
  all four (`BITMAP_LIGHT`, `BITMAP_SPARK+1`, `BITMAP_FLARE`,
  `BITMAP_PIN_LIGHT`). Code also passes an effect's own number on to
  `CreateParticle` or `CreateSprite` (`ZzzEffect.cpp:1290`, 9274), so it
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
  drawing only for 225–642 (`ZzzEffect.cpp:9614`), the model animation only
  from `MODEL_BIRD01`. So the numbers must stay as they are.

### The registry

The registry of sven-n/MuMain#493 (`Render/Effects/EffectRegistry.cpp`,
`EffectDef.h`) describes an effect type by up to four stages: creation
values (`CreateParams`), a creation hook, a move handler and a render
handler. A stage that has an entry replaces that stage's case in the old
switch; a creation entry returns before the switch
(`ZzzEffect.cpp:394-401`).

- 308 types have an entry: 32 with `CreateParams`, 2 with a creation hook,
  302 with a move handler (287 copied out of the old switch into 202
  functions in `Behaviors/MoveHandlers.cpp`, 15 written by hand), 1 with a
  render handler. FX1.0 adds one handler, `Move_BITMAP_JOINT_FORCE` (see
  "Fixed with this document").
- `CreateParams` has lifeTime, scale, velocity, gravity, hiddenMesh,
  blendMesh, blendMeshLight, alpha, light and copyLightToDirection. The 32
  types (22 rows) use lifeTime (32), scale (15), blendMesh (8), alpha (6),
  light (2), velocity (2), blendMeshLight (1) and copyLightToDirection (1).
- The rows are C++ (`EffectRegistry.cpp:39`). The table is an array of
  32,470 pointers indexed by type (the largest type is `BITMAP_DAMAGE1`),
  built on first use. A lookup is a guard, a bounds check and one array
  read; it runs once per creation, once per live effect per frame and once
  per drawn effect per drawing pass.
- No test covers the registry, and the script that extracted the move
  handlers was not kept.

### Creation today

- **Common setup.** `CreateEffect` takes the first free slot (and calls
  `IsSkillEffect` for every slot it tries), then sets 28 values
  (`ZzzEffect.cpp:347-387`). It does not reset LifeTime, Gravity,
  StartPosition, HeadAngle, Distance, Timer, Weapon and others, so these
  keep the values of the slot's previous effect unless the case sets them.
  Particles do not reset Alpha and TurningForce; joints do not reset Scale,
  LifeTime and MaxTails.
- **The 244 cases of the effect creation switch** (417 types):

| Cases / types | What they do | Example |
|---|---|---|
| 7 / 8 | Only values that today's `CreateParams` holds | `MODEL_KENTAUROS_ARROW`, `ZzzEffect.cpp:534` |
| 26 / 26 | Only values, but more fields | `MODEL_DRAGON` (`:405`): CollisionRange, Kind, Timer, Distance, an angle, a position offset, Direction, StartPosition from Position |
| 44 / 47 | Only values, chosen by SubType | `MODEL_MAGIC_CIRCLE1` (`:1479`) |
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
default values left out. `code` is the enum symbol (D28), with an offset
where the code has no name (`MODEL_SKILL_FURY_STRIKE+1`). The one bare
number, `CreateEffect(9, …)` in `GMHellas.cpp`, gets the name
`MODEL_KALIMA_FALLING_STONE` in FX1.1 (world object slot 9, where the
Kalima maps load the rock `Object25\Object10.bmd`).

```json
{
  "formatVersion": 1,
  "kind": "effect",
  "types": [
    { "name": "kundunGhost", "code": "MODEL_CUNDUN_GHOST",
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
- A vector field can set single components (`"angle": {"y": 0}`); an offset
  the old code multiplies by the frame factor is written
  `{"value": -100, "timesFrameFactor": true}`. Offsets and copies are
  groups: `"offset": {"position": {"z": 3400}}`,
  `"copy": {"startPosition": "position"}` (since FX1.4).
- Unknown fields are warnings. Unknown symbols, duplicate names and a newer
  `formatVersion` are errors that stop the start.

### Loading and lookup

- A compiled list per kind (kind, enum symbol) in `Data/GameData/EffectData`
  turns the symbols into numbers. It only depends on the enum headers.
- Loaded in `OpenBasicData` with the item model data. The load time is
  logged.
- For effects, the registry's array of descriptors (32,470 entries) is
  built once after loading. Lookups stay a bounds check and one array read;
  names are only used while loading, for error messages and in the editor.
- Creating an effect before the catalogue is loaded is logged as an error,
  because moved types no longer have their old case. The handlers are code
  and work before that; only the values from the data are missing.

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
| FX1.2 | Registry rows from data | FX1.1 | The `CreateParams` of `EffectRegistry.cpp` (22 rows, 32 types) move into `EffectTypes.json`; the registry table is built from the catalogue at loading. Handlers stay C++. |
| FX1.3 | The 8 types that fit `CreateParams` | FX1.2 | Their cases move into data and are deleted. Sets up the recorder. |
| FX1.4 | More creation fields | FX1.3 | The fields the 26 value-only cases need; those cases move into data. |
| FX1.5 | Variants by SubType | FX1.4 | `variants` in effect rows; the 47 types that choose values by SubType move. |
| FX1.6 | Effect browser | FX1.1 | Read-only tool in MuEditor; values from FX1.2 on. |
| FX1.7 | Preview | FX1.6 | Creating the selected type in the world in editor builds. |

**FX1.1 Names for all types.** The compiled list of symbols per kind (about
640 lines), generated once by a script and kept by hand afterwards. The four
files with `name` and `code`, the `Data::Effects` loader with validation and
a writer, and the arrays from number to name for logs. Loaded on the loading
screen; nothing under `Render/Effects` changes, so the frame time cannot
change. Checked by tests (every symbol has exactly one name in its kind,
names are unique and valid, the shipped files are in the writer's format),
by a one-time comparison of the compiled lists with the case labels,
registry entries and call sites of the four systems, and by the logged load
time.

*Done:* a one-time script collected the types (case labels, registry rows,
call sites; computed types expanded), a compiled probe gave their numbers,
and expressions with the same number are one type (36 effect and 5 particle
numbers have two spellings; the plain enum name wins). The script is not
kept: new types in code are rare, as new content comes as data, and one is
added with a line in the list and a name in its file, which the tests check
against each other.
The four files have 646 names; the rules are in
[effect-data.md](../../effect-data.md). Beyond D31: when an effect with a model
number and one with a texture number get the same name, the model one gets
`Model` at the end (`fireModel`, 6 times), because texture numbers are types
in several kinds and keep one name in all of them; names start with a
letter; 10 names are chosen by hand (collisions the rules do not settle, file
names that say nothing like `cra_04`). The bare Hellas number got an enum
name, `MODEL_KALIMA_FALLING_STONE` (`kalimaFallingStone`), so no file holds
a raw number (D28). Loading the
four files takes about 17 ms on the loading screen (Release, editor build).

**FX1.2 Registry rows from data.** The 22 rows (32 types) of `CreateParams`
move into `create` objects of `EffectTypes.json`, and the registry table is
built from the catalogue, without the guard of today's static table. Only
`EffectRegistry.cpp`, `EffectDef.h` and the data change. Checked by a
one-time test that `Lookup` gives the same values and handlers as the old
rows for every number, by the recorder for the 32 types, and by a Release
benchmark of creation and lookup (old against new).

*Done:* the creation values of the 32 types are `create` objects in
`EffectTypes.json` (format in [effect-data.md](../../effect-data.md)); the
C++ rows of `EffectRegistry.cpp` keep only the handlers. Besides the
registry, the catalogue reads and writes `create` (`EffectCreateParamsJson`)
and keeps the values sorted by number for `BuildRegistry`, which builds the
table on the loading screen right after the catalogue (`OpenBasicData`) and
converts the values to `CreateParams` once, so creating an effect only
copies them, as before. A lookup before the build (a test or a tool without
the data) builds the handlers alone, which are code, and logs an error that
the creation values are missing; in the game nothing creates or moves an
effect that early. A one-time test compared the old rows with the built
registry for every number: all 309 descriptors are equal (32 with creation
values), and applying the values to a slot gives the same fields. The
recorder comes with FX1.3, the first phase that deletes cases; FX1.2 deletes
none and applies the values with the unchanged `ApplyCreateParams`. Release
timing: a lookup takes 1.13 ns instead of 1.70 ns (no static guard any
more), applying the values 2.4 ns as before.

**FX1.3–FX1.5** move effect cases into data in growing steps: first the 8
types whose cases only set fields `CreateParams` has, then the 26 that need
more fields (angle, direction, position offset, StartPosition from
Position, Kind, Timer, Distance, CollisionRange, …), then the 47 that
choose values by SubType. Each step deletes the moved cases and is checked
with the recorder.

*FX1.3 done:* the creation values of `MODEL_KENTAUROS_ARROW`, `MODEL_WARP3`,
`MODEL_WARP6`, `BITMAP_SPARK+1`, `BITMAP_SPARK+2`,
`MODEL_1_STREAMBREATHFIRE`, `MODEL_EFFECT_EG_GUARDIANDEFENDER_ATTACK2` and
`MODEL_EFFECT_SD_AURA` are `create` objects of `EffectTypes.json`, and their
7 cases are deleted from `CreateEffect` (`CreateEffect` runs nothing after
its switch, so a row replaces a case completely). The recorder is
`tests/effects/EffectRecorder`, with `Random::Seed` as the only change to
game code (no game code calls it). The PR's first commit records the old
cases against the rows in one build, for the sub types 0 to 3 and 99, each
owner (none, the hero, a monster), both frame factors and both slot
patterns: all 480 calls are equal. The second commit deletes the cases and
keeps spot checks of the values they set. g++ finds the same 6 fallthroughs
in `ZzzEffect.cpp` before and after (with `-Wimplicit-fallthrough` in a real
compile; it does not warn with `-fsyntax-only`). Close to fitting, for
FX1.4: four cases whose other statements only repeat the common setup
(`MODEL_SUMMONER_WRISTRING_EFFECT`, `MODEL_SUMMONER_CASTING_EFFECT4`,
`BITMAP_FIRECRACKER0001`, and `MODEL_SHIELD_CRASH2`, whose `Gravity =
Velocity` is always 0.3 there), the empty case of `MODEL_PHOENIX_SHOT`, and
four cases that set `Scale = Scale` (the call's scale even when it is 0).

*FX1.4 done:* `create` has the fields lightEnable, alphaEnable, kind, skill,
pkKey, timer, distance and collisionRange, the vectors position, angle and
direction (a list of three or an object of the components that change), the
group `offset` (position, angle, startPosition; components can be multiplied
by the frame factor) and the group `copy` (direction from light, which
replaces copyLightToDirection, startPosition from position, headTargetAngle
from the call's light, scale from the call's scale). The creation cases of
28 types are rows now: the 21 that only set values (`MODEL_DRAGON`,
`MODEL_SHIELD_CRASH2`, `MODEL_TREE_ATTACK`, `MODEL__SPEAR`,
`MODEL_SUMMONER_WRISTRING_EFFECT`, `MODEL_SUMMONER_CASTING_EFFECT4`,
`MODEL_SUMMONER_SUMMON_NEIL`, `MODEL_ALICE_BUFFSKILL_EFFECT2`,
`BITMAP_JOINT_THUNDER`, `MODEL_STAFF_OF_DESTRUCTION`, `MODEL_WAVE`,
`MODEL_TAIL`, `MODEL_BOSS_ATTACK`, `MODEL_DARK_ELF_SKILL`,
`MODEL_WATER_WAVE`, `BITMAP_FIRECRACKERRISE`, `BITMAP_FIRECRACKER0001`,
`MODEL_CLOUD`, `MODEL_TOWER_GATE_PLANE`, `MODEL_KNIGHT_PLANCRACK_B`,
`MODEL_PROJECTILE`) and the 7 types of 6 cases that keep the call's scale
(`BITMAP_SHINY+4`, `MODEL_WINDFOCE_MIRROR`, `BITMAP_SWORD_EFFECT_MONO`,
`MODEL_TARGETMON_EFFECT`, `BITMAP_EVENT_CLOUD`,
`MODEL_STATUE_CRUSH_EFFECT_PIECE04` and `MODEL_DOOR_CRUSH_EFFECT_PIECE10`).
The empty case of `MODEL_PHOENIX_SHOT` is deleted without a row. Two cases
needed more than "values, offsets, copies": `BITMAP_JOINT_THUNDER` adds to
the start position after copying it (an offset of a field a copy writes adds
to the copy), and `MODEL_SUMMONER_SUMMON_NEIL` copies the call's light
before setting its own (a copy from the call's argument); statements that
only repeat the common setup are left out. The PR's second commit compared
the old cases with the rows in one build: all 4,640 calls equal at the frame
factors 1 and 0.5, the calls without an owner also with a second position,
angle and light, and all 2,320 at 25/60. The third deletes the 28 case
groups; g++ finds the same 6 fallthroughs. `CreateParams` keeps which groups
of fields a row sets, so rows without the new fields cost what they did; a
benchmark of `CreateEffect` gave 14.4 ns per creation of the 28 types with
their cases and 15.7 ns with their rows. The `Scale = PKKey / 100.f` cases
(`MODEL_SKILL_FURY_STRIKE+3/+4/+6/+7`, `MODEL_AURORA`, `MODEL_WAVE_FORCE`)
compute a value from a skill argument and stay code (D36).

**FX1.6–FX1.7** add the effect browser and its preview to MuEditor. They
change no game code outside editor builds.

## Verification

Every PR that moves values is checked with a recorder in the test binary
(D40), as the items phases did, but committed as a test tool, because FX1.3
to FX2 all need it:

- **Fixed inputs:** a fixed `srand` seed, a seed for `Random::`
  (`Random::Seed`, since FX1.3), the frame factor
  pinned to 1.0 and 0.5, a fixed `WorldTime`, and the hero and owners set
  up with the game's own setup functions.
- **Slots:** every pool slot filled with pattern A, then pattern B, so the
  types that read old values of a slot show up.
- **Cases:** every moved type with each SubType its case handles, every
  SubType callers pass, and one that no case handles; also the types whose
  case falls through into a moved case.
- **Fallthrough:** a deleted case can be where a case above it without
  `break` goes on. sven-n/MuMain#493 broke three this way
  (`BITMAP_JOINT_FORCE`, the stone and bone debris, and
  `MODEL_SUMMONER_EQUIP_HEAD_LAGUL`). Compiling `ZzzEffect.cpp` with clang
  and `-Wimplicit-fallthrough` lists these places: 6 today, 5 in
  `CreateEffect` (the bones, the big stones and the stones into
  `MODEL_ICE_SMALL`, and one in the SubType switch of the Kundun parts) and
  1 in `MoveEffect` (the stones and snow into the bones), all into cases
  that stay code in FX1. Each one must land on the same code after a
  deletion: a case that fell into a moved case calls its handler.
- **Recorded:** every changed field of every slot of the five pools
  (effects, skill effects, particles, joints, sprites), of the hero and the
  monster owner, and of the call's position, angle and light; the trails
  (blurs and object blurs, some of them live in pattern B), the lit cells of
  the terrain light, and the sounds started and stopped (a recording audio
  backend); how many values `rand()` and `Random::` gave. Pointers are named
  (hero, monster, pool slots, character slots) or written as an address.
  Bytes that change outside the field lists show up for particles and
  joints, and the 64-bit Windows build stops when one of the structs changes
  size. Each call runs with the game's default arguments and with uneven
  ones (scale, PK key, skill values, target index); since FX1.4 the calls
  without an owner also with a second position, angle and light, so a copy
  differs from a constant, and the comparison of a phase adds the frame
  factor 25/60, where products with it round (its digests are not committed:
  a compiler may fuse a multiply-add on one platform and not on another).
  Not recorded yet: the play speed of models and the owner's fields outside
  its object; not varied yet: live slots, terrain height, `timeGetTime`. The
  test binary has no option window and no models, so cases that reach
  `CreateParticle`, `CreateSprite` or `Models` cannot be recorded yet, and
  the monster owner is not a real monster. The phase that first moves such
  cases adds them.
- **Tool:** `tests/effects/EffectRecorder` (since FX1.3): `RecordCall`
  records one call under given conditions, `Compare` lists the differing
  fields by name, and `EffectTestData::BuildShippedRegistry` builds the
  registry without the rows being checked, so the old cases run in the same
  build. The digests of the whole records of the moved types, taken with
  their old cases, are committed (`tests/effects/recordings`, one file per
  phase; the FX1.2 rows were recorded before FX1.4 changed how rows are
  applied), so later changes are checked against the old cases again;
  `MU_EFFECT_RECORDER_WRITE=1` writes them anew.
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
  effect without its fade. Its move code is a registry handler now
  (`Move_BITMAP_JOINT_FORCE`), which ends with the sword force handler, as
  before #493.
- **Stone, bone and snow debris** (`MODEL_STONE1`/`2`, `MODEL_SNOW2`/`3`,
  `MODEL_BONE1`/`2`, `MODEL_BIG_STONE1`/`2`; 61 creation calls, 55 of them
  with a SubType that reaches this code: the bones of dying skeletons, the
  stones of Stone Golems, Crywolf, Kanturu, Hellas and more): their move
  code fell through into the move code of `MODEL_ICE_SMALL` in the original
  client (every SubType except 5, and for stones and snow also except 11, 13
  and 14), so the debris of SubTypes 0, 10 and 12 flew out, slowed down,
  fell and bounced (stones of SubTypes 0 and 12 threw fire sparks), and
  that of SubTypes 1 and 2 rose (SubType 2 spinning). When
  sven-n/MuMain#493 moved that code into a handler, the cases started to
  fall through into `MODEL_EFFECT_BROKEN_ICE0` instead: SubType 0 moved by
  the slot's old `HeadAngle` with an undamped drift, SubType 1 hung in the
  air (or burst into ice when created below the ground), SubType 2 fell and
  turned into a new SubType 0 stone, and SubTypes 10 and 12 slid on in a
  straight line without gravity. They run the `MODEL_ICE_SMALL` handler
  again, as before #493.
- **`MODEL_SUMMONER_EQUIP_HEAD_LAGUL`** (the head that circles a summoner
  with the Book of Lagle): its move code fell through into the move code of
  the summoner casting effects and ran only their `BlendMeshLight` ramp.
  Since sven-n/MuMain#493 it ran a block after their `break` and scrolled
  `BlendMeshTexCoordV` too. It calls the casting effect handler now, as
  before #493. Nothing changes in the game: the head has no blend mesh.
- **The particle `BITMAP_SPARK + 1`, SubType 7** (`ZzzEffectParticle.cpp`):
  the position jitter was written to y, z and past the end of the position
  (`Position[3]`, which is `Angle[0]` of the particle). Now it goes to x, y
  and z; the three random draws stay the same.
- **The blend-mesh pass of `RenderEffects`** (only on water maps, where it
  is called with `true`): it read `Models[o->Type]` for effects with a
  texture number (`BITMAP_*`), far past the end of the model array. Effects
  without a model skip that check now. `RenderAfterEffects` had the same
  check in a branch that never ran (its only call passes no argument); the
  branch and the parameter are gone, which changes nothing in the game.
- **cppcheck findings** (FX1.P): four path points kept in an array of three
  (`arv3PosProcess` in the creation of the Gaion swords, an out-of-bounds
  write, also MSVC warning C4789; in Release builds MSVC kept the points in
  registers and the code is the same with the fix, Debug builds stopped
  with a run-time check failure when Gaion's attack spawned the swords), an
  unused variable and a statement without effect, and a macro call without
  its semicolon in `ZzzEffect.cpp`; in `MoveHandlers.cpp` a distance read
  before it was set (it only decided a block that wrote a local position
  nothing reads, so the block and the distance are gone:
  `MODEL_DEATH_SPI_SKILL`, `MODEL_PIER_PART`), a self-assignment, and a
  null check after the pointer was used (`MODEL_ALICE_DRAIN_LIFE`). None of
  these changes a value in the game.
- With the files passing cppcheck, the last conversions of items phase 4d2
  follow: `RenderWheelWeapon` and `RenderFuryStrike` use `ToModelSlot`, and
  the spear check of the move handlers uses `ITEM_SPEAR`.

Filed upstream (2026-10-03):

- sven-n/MuMain#680: `MODEL_DEATH_SPI_SKILL` creates its ground circles at
  an uninitialized position (the first of a frame; the further ones of that
  frame at its rotated direction, a point near the map origin), also in the
  original client; the fix adds a visible effect, so the look needs a
  decision.
- sven-n/MuMain#681: 63 creation cases multiply one-time values (spawn
  offsets, start angles) by `FPS_ANIMATION_FACTOR`, so effects start in
  other places above 25 fps (from upstream's 2023 frame rate work; D34
  keeps them as a flag until then).
- sven-n/MuMain#682: 25 effect (5 of them only with a registry row), 6
  particle and 2 joint types are handled but never created (checked in
  FX1.1, also against computed types).
- sven-n/MuMain#683: three item stat changes of 815b8828 found for the items
  design (blocking and magic defense, the level requirement of the late
  wings, wing options counted as excellent).

Still to check and file upstream: the owner is used without a null check in
27 creation cases, and 9 effect types are created but have no code
(`MODEL_EX01_SHADOW_MASTER_*`).

## Open questions

- **Browser, later.** A generated index of the code call sites for "used
  by", besides the data users.
