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
| `copyLightToDirection` | `true`: the move code of the effect gets the color too (some effects fade it back in from there). |

- A field that is left out keeps what the game sets for every new effect,
  or what the creating code passes. `lifeTime` and `gravity` are not set for
  every new effect: left out, they keep the value of the effect that used
  the slot before, as in the original client.
- An effect with `create` starts with these values **instead of its
  creation code**. Only effects whose creation code set nothing but these
  values have one; adding `create` to another effect drops what its code
  did (for example the effects it spawns) and changes how it looks.
- A value that is not a number, or too large for the game (which keeps
  the values as float), is an error; so are mesh fields that are not whole
  numbers from -2 to 32767, an `alpha` outside 0 to 1, a
  `copyLightToDirection` that is not `true`/`false` and a `light` that is
  not three numbers. An unknown field is a warning, and so is a `create`
  that sets no value (it still replaces the creation code).

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
