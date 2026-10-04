# Effect data

The effect catalogue gives every effect, particle, lightning and sprite
type of the client a name, and some effects their creation values. It
lives in `src/bin/Data/Effects/`, next to the glow colors of the item models
(`GlowColors.json`), and is loaded once on the loading screen. Other data
will name effect types with these names instead of numbers; the effect code
keeps using the numbers.

Design and plans: [effect catalogue design](superpowers/specs/2026-10-02-effect-catalogue-design.md).

---

## Kinds

The client creates four kinds of effects, each with its own function and
its own code:

| Kind | Created with | What it is | File |
|---|---|---|---|
| `effect` | `CreateEffect` | Objects that move, often with a model: arrows, skills, debris, event objects | `EffectTypes.json` |
| `particle` | `CreateParticle` | Small textured particles: smoke, fire, sparks | `ParticleTypes.json` |
| `joint` | `CreateJoint` | Lightning, beams and trails between two points | `JointTypes.json` |
| `sprite` | `CreateSprite` | One camera-facing texture for one frame | `SpriteTypes.json` |

A type is a number of the code (`MODEL_*` or `BITMAP_*`). The same number is
a different type in each kind, with different code, so each kind has its own
names: `fire` is a particle and also an effect, and the two are unrelated.

## Files

```json
{
  "formatVersion": 1,
  "kind": "particle",
  "types": [
    {
      "name": "smoke",
      "code": "BITMAP_SMOKE"
    },
    {
      "name": "smoke2",
      "code": "BITMAP_SMOKE+1"
    }
  ]
}
```

- `name`: letters and digits, starting with a letter, unique in its kind.
  This is what other data uses.
- `code`: the type as the code writes it: the enum name or an enum name
  with an offset (`BITMAP_SMOKE+1`, no spaces). The client turns it into the
  number with a list compiled into the game, so the files hold no raw
  numbers.
- Every type that the code of a kind uses has exactly one entry. A type
  without a name, an unknown code, a code with two names, two types with
  the same name, a wrong `kind` or a newer `formatVersion` are errors: the
  game shows them and does not start. Unknown fields are warnings in
  `MuError.log`.
- Types are sorted by name and the fields always come in the same order;
  files are UTF-8 with LF line endings.

## Creation values

An effect entry can have a `create` object: the values the effect starts
with when the game creates it. Particles, lightning and sprites have none
(a `create` there is ignored with a warning).

```json
{
  "name": "kundunGhost",
  "code": "MODEL_CUNDUN_GHOST",
  "create": {
    "lifeTime": 200,
    "scale": 1.8,
    "velocity": 0.08,
    "blendMesh": -2,
    "light": [0.5, 0.5, 0.5]
  }
}
```

| Field | What it sets |
|---|---|
| `lifeTime` | How long the effect lives, in frames at 25 frames per second (200 is 8 seconds). |
| `scale` | Its size; replaces the size the creating code passes (0.9 when it passes none). |
| `velocity` | Its speed; what it does depends on the effect's move code. |
| `gravity` | Its gravity; what it does depends on the effect's move code. |
| `hiddenMesh` | The mesh of the model that is not drawn, by its number in the model; `-1` none, `-2` the whole model. |
| `blendMesh` | The meshes drawn bright and see-through (added light), by the texture number of the model's meshes; `-1` none, `-2` all. |
| `blendMeshLight` | How bright the `blendMesh` meshes are. |
| `alpha` | How opaque the effect is, from 0 (invisible) to 1. |
| `light` | The color it is drawn with, `[red, green, blue]`; replaces the color the creating code passes. |
| `lightEnable`, `alphaEnable` | Flags (`true`/`false`) the move and draw code of the effect use; what they do depends on it. |
| `kind`, `skill`, `pkKey`, `timer`, `distance`, `collisionRange` | Values the move code of the effect uses; what they do depends on it. `kind` (0 to 255) and `skill` (0 to 65535) replace the skill values the creating code passes, `pkKey` its PK key. |
| `alphaTarget`, `animation` | Values the move and draw code of the effect use (`alphaTarget` from 0 to 1, `animation` 0 or more); what they do depends on it. |
| `renderType` | `"dark"`: a model drawn dark. `"alphaBlendMinus"`: the render type the old code of `BITMAP_SHINY+6` set; the draw code of the effect decides what it does. |
| `position` | Where it starts; replaces the position the creating code passes. |
| `angle` | How it is turned, in degrees; replaces the angle the creating code passes. |
| `direction` | A vector its move code uses, often the way it moves. |
| `startPosition` | A vector its move code uses, often where it started. |
| `offset` | Values added after the ones above: to `lifeTime`, `position`, `angle` or `startPosition`. |
| `copy` | Fields that get the value of another field, after the offsets (see below). |

```json
"create": {
  "lifeTime": 1000,
  "angle": {"y": 0},
  "direction": [0, -35, 0],
  "offset": {
    "position": {"z": 3400}
  },
  "copy": {
    "startPosition": "position"
  }
}
```

- `position`, `angle`, `direction`, `startPosition` and their offsets are
  vectors: a list of three numbers (x, y, z), or an object with only the
  components that change (`{"y": 0}`); the others keep their value. The
  offset of `lifeTime` is a number.
- An offset (a component, or the lifeTime) can be multiplied by the frame
  factor, as some of the original creation code did: `{"value": 280,
  "timesFrameFactor": true}`. The frame factor is 1 at 25 frames per second
  and smaller above, so such effects start at other places at higher frame
  rates (sven-n/MuMain#681).
- `copy` lists `"field": "source"`. The fields and their sources:
  - `direction`: `"light"` (the move code gets the color too; some effects
    fade it back in from there) or `"callAngle"`;
  - `startPosition`: `"position"` (the position after the offsets),
    `"light"` or `"callPosition"`;
  - `headTargetAngle`: `"callLight"`;
  - `eyeRight`: `"light"`;
  - `deadPosition`: `"callAngle"`;
  - `scale`: `"callScale"`.
  `light` and `position` are the effect's own after the values and offsets;
  the sources starting with `call` are what the creating code passes (the
  scale also when it passes none, 0). A field gets a value or a copy, not
  both; an offset of `startPosition` adds to the copy.
- A field that is left out keeps what the game sets for every new effect, or
  what the creating code passes. `lifeTime`, `gravity`, `timer`, `distance`,
  `collisionRange`, `alphaTarget`, `animation`, `startPosition`,
  `headTargetAngle`, `eyeRight` and `deadPosition` are not set for every new
  effect: left out, they keep the value of the effect that used the slot
  before, as in the original client.
- An effect with `create` starts with these values **instead of its
  creation code**. Only effects whose creation code set nothing but these
  values have one; adding `create` to another effect drops what its code
  did (for example the effects it spawns) and changes how it looks.
- A value that is not a number, or too large for the game (which keeps the
  values as float), is an error; so are mesh fields that are not whole
  numbers from -2 to 32767, `kind`, `skill` and `animation` outside their
  ranges, an `alpha` or `alphaTarget` outside 0 to 1, another `renderType`
  than the two names, flags that are not `true`/`false`, vectors that are
  not three numbers or an object of `x`, `y` and `z`, the frame factor
  outside an offset, a copy from another source than the ones listed, a
  field with both a value and a copy, and the old field
  `copyLightToDirection` (now `"copy": {"direction": "light"}`). An unknown
  field is a warning, and so is a `create` (or `offset`, `copy`, vector)
  that sets no value; a `create` without values still replaces the creation
  code. An offset of a component of `startPosition` that nothing sets or
  copies, and an offset of `lifeTime` without a `lifeTime`, are warnings:
  they add to what the slot's previous effect left.
- `kind` in `create` is a value of the effect, not the `kind` of the file.

### Variants by SubType

The code that creates an effect also passes a SubType, a number whose meaning
depends on the effect (a level, a direction, a step of a skill). A `create`
can give some SubTypes other values:

```json
"create": {
  "lifeTime": 30,
  "scale": 0.7,
  "velocity": 0.1,
  "blendMesh": -2,
  "variants": [
    {
      "subType": 1,
      "lifeTime": 20,
      "hiddenMesh": 0
    },
    {
      "subTypes": [2, 3],
      "lifeTime": 15,
      "velocity": 0.3
    }
  ]
}
```

- A variant names its SubTypes with `subType` (one) or `subTypes` (a list;
  the writer writes a list of one as `subType`) and holds the fields of
  `create`, except `variants`.
- A SubType of a variant starts with the values of `create` and the
  variant's on top: a value replaces the value of its field and a copy into
  it, a copy replaces the value and the copy of its field, and vectors and
  offsets are replaced component by component. A SubType without a variant
  gets the values of `create` alone.
- A variant only adds or replaces: it cannot leave a field to the common
  setup when `create` sets it, or drop a copy into `headTargetAngle`,
  `eyeRight` or `deadPosition` that `create` makes (only `scale`,
  `direction` and `startPosition` have values that replace a copy). Such
  cases need a `create` with only variants.
- A `create` can hold only variants. The SubTypes without a variant then
  start with what the game sets for every new effect, and with the old
  values of the slot for the fields it does not set, as the original code
  did.
- A SubType in two variants or twice in one list, a variant without SubTypes
  or with a negative one, a variant with variants, and a variant that sets
  part of `direction` or `startPosition` while `create` copies into it are
  errors. A variant without values and an empty `variants` list are
  warnings.

## How the names were chosen

- The enum name in camelCase without `MODEL_`/`BITMAP_`:
  `MODEL_ARROW_HOLY` → `arrowHoly`, `BITMAP_FLOWER01` → `flower1`.
- Words written together are split and misspellings corrected:
  `MODEL_CURSEDTEMPLE_HOLYITEM` → `cursedTempleHolyItem`,
  `BITMAP_EXPLOTION` → `explosion`, `MODEL_WINDFOCE` → `windForce`,
  `MODEL_CUNDUN_GHOST` → `kundunGhost`.
- Types without an enum name (`BASE+n`) are named after the file loaded
  into their slot (`MODEL_SKILL_FURY_STRIKE+1` loads `EarthQuake01.bmd` →
  `earthQuake1`), or the base name with `n + 1` when nothing is loaded there
  (`BITMAP_BOSS_LASER+1` → `bossLaser2`).
- An effect with a model number and one with a texture number that would
  get the same name: the model one gets `Model` at the end (`MODEL_FIRE` →
  `fireModel`, `BITMAP_FIRE` → `fire`), because texture numbers are types
  in several kinds and keep one name in all of them.
- A few names are chosen by hand where the rules give nothing useful, for
  example `glitter` for `BITMAP_LIGHT+2`, whose file `cra_04.jpg` says
  nothing.

A name is meant to stay: once other data uses it, a rename has to update
every user.

## When the code changes

A new type in the code gets a line in the list of its kind in
`Data/GameData/EffectData/EffectTypeSymbols.cpp` (at the place of its
number) and a name in its catalogue file, chosen by the rules above. A type
that the code no longer uses is removed from both. The tests of
`test_effect_types` check that every type of the list has exactly one name,
that each kind lists every number once and in order, and that the files are
in the written format. A type that the code uses but the list misses has no
name: data that names it fails to load with an error.

`tests/effects/baseline/EffectCreation.txt` holds what the effects whose
creation moved into the catalogue create, as their old code did, one line
per recorded call starting with the name of the type; the test
`test_effect_creation` compares it on every run. A `create` value changed on
purpose fails it: write it anew with `MU_EFFECT_RECORDER_WRITE=1` set and
say in the PR why the effect changes. The file and its test are removed once
the catalogue is edited on purpose (see Verification in the design).
