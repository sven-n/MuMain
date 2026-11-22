# Monster and NPC Looks in Data (MN8) Design (WIP)

> **Draft, 2026-11-13.** Phase MN8 of the
> [monsters design](2026-10-23-data-driven-monsters-design.md) and decision D25
> of the [data-driven content roadmap](2026-09-29-data-driven-content-roadmap-design.md):
> one look format for items, monsters and NPCs. It builds on the item looks of
> the [looks design](2026-11-01-looks-in-data-design.md) (items phase 13),
> whose render styles, item effects and Style Editor are on main since
> sven-n/MuMain#1022 and #1050. Written from the code of MuMain `upstream/main`
> @ `1211f0313`, after the monsters' PR 3c (#1103); line numbers are those of
> that commit.
>
> This document is temporary. Its last task deletes it, after what stays true
> has moved into `docs/monster-data.md`, `docs/effect-data.md` and the code.

## Goal

What makes a monster or NPC look the way it does, beyond its model, size and
equipment (which are data since MN3 and MN4), moves from code into data, and
is edited in MuEditor: its glow and shine, the effects on its bones, what its
attacks create, what it leaves when it dies, its shadow and its cloth. A look
made for an item can be put on a monster and the other way round (D25).

As in every phase before: nothing changes in game when a look moves from code
into data, each move is compared with the old code before the old code goes
(D50), and known bugs are fixed in commits of their own (D51).

## Scope

- **In:** the draw-time looks of monsters and NPCs (body light, tints,
  layers, extra models, outlines, ground decals, shadows), the per-frame
  things they spawn while alive (things on bones, links between bones,
  dust, terrain light, material values), their attack effects, the trap
  effects of `SetPlayerAttack`, death effects and debris, cloth, the item
  render styles only monsters use (`helperNpcPlate`, `monsterBattleBow`) and
  `RenderLinkObject`'s monster check (monsters design, MN8 row).
- **Also in:** the sounds those visual functions still play with `PlayBuffer`
  (about 210 calls in 15 files), as sound events of the sound sets (MNS, MNA:
  `during`, `once` and cues), as the monsters design's MN8 row says.
- **Out:** effect behaviour (how an effect moves and fades) stays code until
  FX2; looks name FX1 effect types. Mount looks (`mountLook`, the Dark Horse)
  are the roadmap's "pets and mounts" area. Cutscenes (Crywolf, Kanturu 3rd
  scripts) are their own later area (monsters design Q-MN58). Player
  characters' looks are not monsters.

## Decisions

Inherited, not asked again:

| # | Decision | From |
|---|---|---|
| D25 | One look format and one look editor for all areas; looks are named and referenced by name. | roadmap |
| D26 | Shared definitions: "used by", a warning before a shared change, copies for variants, no delete while used. | roadmap |
| D49 | Data first; code stays only as a named hook the entry names, where logic hands over to code. | monsters design |
| D50 | Each move compared with the old code before it goes; MN8 with the look recorder of items phase 13. | monsters design |
| D51 | Known bugs move as they are and are fixed in commits of their own. | monsters design |
| L15 | Monsters use the same look files; renaming the "item effect" kind for them is this design's call. | looks design |

New decisions (ML1, ML2, …) are added here as the open questions below are
answered.

| # | Topic | Decision |
|---|---|---|
| ML1 | Where a monster names its looks (Q-ML1a, 2026-11-13) | **The model names its usual looks, a monster can name others.** A model entry in `Models.json` names the looks every monster of that model gets, as an item model entry names its render style; a monster entry, or one of its numbers (D61), can name another look in its place. So the 17 monsters that today get a variant on a shared model through the `ExtraMon` number (Metal Balrog on the Balrog model, Great Drakan on the Drakan model, …) name their own, and the others name nothing. Monster Setup shows the model's look in its dropdown ("(model: balrogRedChrome)") until a monster picks another. Rejected: looks only on the monster entries (monsters sharing a model and a look repeat the name), and one look per monster holding all its parts (an item's render style could not be put on a monster as it is, D25, and every variant would be a copy). |
| ML2 | The kinds of looks (Q-ML1b, 2026-11-13) | **Two kinds, as for the items, and an effect's parts say when they run.** A **render style** draws (L13); an **effect** spawns, in the update (ADR 0003). Each part of an effect has a **when**: always (every frame or by its own timing, what an item effect does), at an attack (an attack animation and tick, Q-ML3), at death. So an item effect is an effect whose parts are all "always", and a monster names at most one render style and one effect, on its model or itself (ML1). The kind "item effect" is called **effect** for items too (L15 left the name to this design): the Style Editor's root and the Data Checks area become Effects; whether the file and the items' field `itemEffect` are renamed with it is settled in the target design. Rejected: four kinds by moment (alive, attack, death effects as kinds and fields of their own: up to four names per monster and four roots), and monsters reusing the item kinds as they are with attack and death effects on the monster entry (they could not be shared as looks). |
| ML3 | Conditions (Q-ML2, 2026-11-13) | **Whole looks by the entry, details in the look.** What picks a whole look is the monster entry's: its `mapVariants` and `stateVariants` (MN4, MN5) and its numbers (D61) name another render style or effect, so Crywolf's green chrome is one look, `crywolfGreenChrome`, that the map variants of its eight monsters name, the Guardian Statue's siege glow is a state variant (the siege a named state like `crywolfElfTime`), and Zaikan's look is Tantallos' entry with its own number. What changes inside a look while the monster lives is the look's: L11's **only when** on a layer or an effect part gains the animation (and "not"), a frame range of it, and buffs on the monster (poison, freeze, the Hellas defense buffs, …), so the Castle Gate's layers are "not while dying" and Destler's meshes 3 and 4 "not from frame 2.5 of attack 2". A look then knows no maps or event states and can be put on an item or another monster as it is. Rejected: every condition in the look (looks would know maps and states), and every condition as a variant of the entry naming a whole look (a look copied per animation and per buff, the freeze tint for every monster). |
| ML4 | When an attack effect fires (Q-ML3, 2026-11-13) | **At a time, as today, or at a pose.** Today an attack starts two clocks: the animation at the model's speed (`animations` in `Models.json`, editable in Monster Model) and the attack counter, 1 to 15 at one step per 1/25 s at every speed (`ZzzCharacter.cpp:4076`); the effects fire at a step of the counter ("tick 11" is 0.4 s after the start), the sound cues (MNS) at a frame of the animation. At the original speeds the two meet; when an animation's speed changes, a tick effect stays at its time and leaves the swing, a cue follows it. An attack part's when says **tick** (moved effects, so the move is exact and the recorder matches) or **frame** of its attack animation (a pose, like a cue), and the Style Editor switches a part from one to the other at the frame where the tick lands at the model's speed. Monster Preview shows both on the frame ruler, tick parts where they land at the current speed. Rejected: ticks only (effects never follow a changed animation speed), and frames only (the move would shift timings by rounding, and effects after an animation's end need a rule). |
| ML5 | Things on bones (Q-ML4, 2026-11-13) | **Full detail in the data, recipes in the editor.** An effect part stores what it creates as it is: a sprite, particle or effect by its FX1 name, with subtype, size, colour (fixed, random or one of a list), turning and pulsing (L9's shapes), on bones (numbers or names, lists, ranges, "all but", a random bone) with an offset and jitter; a link (a joint between bones, from a point near a bone, to the origin) or a trail between two bones; things around the monster (at a random point within a radius, terrain light). So everything the code does can be written, and nothing needs new code to add. The Style Editor shows these parts as recipes, as L35 does for layers: **Add effect** offers a gallery (lights on bones, eye lights, thunder chain, fire on bones, walk dust, trail, …) that writes the parts, the list names them back in designer words ("Lights on bones: 13-26, blue"), and the fields are the advanced view. The same parts serve the items' things on bones (looks pull request 4). Rejected: the parts in engine terms in the editor too (hard to read), and only named recipes in the data (every odd case a new recipe in code). |
| ML6 | Logic that is more than a look (Q-ML5, 2026-11-13) | **As much as possible becomes data.** Of the about 40 rows of "Logic that is more than a look", only what talks to the server or runs a whole scene stays a hook (D49): the Kanturu gateway NPC's enter request and the Maya battle (with the falling monsters of Kanturu 3rd), and any row the target design finds that cannot be written without code, each named with its reason. Everything else becomes blocks of the effect kind (ML2), which runs in the update (ADR 0003: the draw stays read-only): presets that create several things (the inferno ring of eight bombs, a ring of N copies), values from the attack tick (the Hydra laser's light), counters and once-per-loop or once-per-action latches, alternating bones, copies of the model at offsets, screen shake, breaking into meshes, and also the blocks that act on the monster: start an animation (the Ballista's own attack, the Doppelganger's attack after appearing, the Butchers walking instead of standing), set a flag a later part reads (the Empire Guardian bosses' rage), set a buff the client shows (the Hellas skills), change height, selectability or the hidden mesh by a named state (the Battle Castle objects). Because looks then change game state, the target design gives these acting blocks their own Data Checks (an action on a monster that has no such animation, a flag nothing reads) and the monster recorder records their effects on the object, not only what is drawn. Rejected: what only shows becomes data and what changes the game stays a hook (about 15 hooks left), and all 40 rows staying hooks. |
| ML7 | Looks on other maps (Q-ML6, 2026-11-13) | **The look follows the monster.** A monster's looks are its own on every map; the map checks of today's hooks go, whether a hook checked its map (Raklion, Karutan, PK Field, Elbeland, Cursed Temple, …) or not (Kanturu, Aida, Hunting Ground, …). A map variant (ML3) is kept only where a map changes a monster's look on purpose: Crywolf's green chrome on its eight monsters, the Doppelganger maps' colours and opacity, the dragons in Heaven, the Ice Queen in Devil Square, and any others the target design finds by the same test (the monster also appears on maps where it looks different). On the maps where OpenMU's Season 6 data places a monster (`maps` per number, D61) nothing changes, which the recorder shows by drawing each monster on its maps; elsewhere, where a server places it, it now looks as at home instead of plain or half-drawn (a change in its own commit with its list). Rejected: today's mix kept as map variants that only name the home map (about 40), and every look limited to its home map (a moved monster loses its look). |
| ML8 | Cloth (Q-ML7, 2026-11-13) | **A cloth part in the effect, every value data.** An effect part `cloth` (when "always") holds what `UpdateCharacterCloth` and `CloakOf` set in code: the bone (or a bone of the second model, Dark Phoenix's hair) and offset, the shape (a grid of N × M points and its size, a cylinder, or made from a mesh of the model: the Magic Skeleton's cape from mesh 2), the texture, the feel (heavy, stretchy, curved, stuck), see-through edges, the collision spheres on bones, the lights spawned from the cloth, and with L11's only when (ML3) when it freezes or is left out (stunned or asleep, the cloaking buff, Chaos Castle, the Doppelganger monsters). The Style Editor edits it with the live preview, so a robe's length or stiffness is tuned while it moves. The seven monster cloths move (Magic Skeleton, Mega and Alpha Crust, Omega Wing, Death King, Nightmare, Dark Phoenix's hair, Fred); Molt's side hair (`CSideHair`, drawn from the drawn vertices, its model file missing) is checked by the target design. Item capes may use the part later. Rejected: named cloth kinds in code with a few settings (size and feel fixed per kind), and cloth staying code named by a hook. |
| ML9 | The sounds of the visual code (Q-ML8, 2026-11-15) | **Into the sound sets, as MNS, MNA and MNK did.** The about 210 `PlayBuffer` calls left in the monsters' visual code (Crywolf 45, Hunting Ground 21, the Balgass barracks 21, Kanturu 3rd 19, Hellas 17, Raklion 11, …, and about 20 in `AttackEffect`) become events of the monster's sound set in `Sounds.json`: a chance while an animation plays with a pick of sounds (`during`: Balgass' steps, 1 in 15 frames), once per action or per loop (`once`: the attack and death sounds that reuse `SubType` as a "played" flag today), at a frame (a cue on the model), edited in Monster Sounds and played in Monster Preview. They move with the visuals of the same monster, compared by the monster recorder's sound part. Effects play no sounds (ML2's effect parts create, the sound sets sound), so a monster's sounds stay in one place. Rejected: a sound part in the effect (monster sounds in two places), and leaving them in code for a later step. |
| ML10 | The editor (Q-ML9, 2026-11-15) | **One Style Editor for items and monsters (D25, L18).** The Style Editor edits every look, with its roots Render styles and Effects (ML2); "used by" lists items and monsters, and Apply's choices (L36) work for a monster as for an item (all its users, only this monster as a new look, a new monster). Its preview shows a look on any item or monster: for a monster, its model with an animation picked, playing or stopped at a frame, the frame ruler with the attack parts (ticks where they land, frames, ML4), the cloth moving (ML8), and the map variants (ML3, ML7) to try. Monster Setup (for a monster or a number) and Monster Model (for the model's defaults, ML1) show the render style and effect it uses in dropdowns, the model's marked "(model)", with an Edit button that opens the look in the Style Editor with that monster in the preview. Rejected: a Monster Looks window of its own (two editors for the same look files), and the looks split over Monster Model and Monster Attacks (a look in several windows, harder to share with items). |
| ML11 | Which problems are fixed (Q-ML10, 2026-11-15) | **All of them.** ML-P1 to ML-P15 are fixed in this work and ML-P16 is checked, each in a commit of its own after the move of its look (D51), so the move itself stays exact. The eight that change what players see (the Blood Castle light in castles 1 to 3, the dead tints, the golden monsters' colour, the Kalima gates' shadows, the grey Little Santa and Satyros, Crywolf's Beam Knight thunder, Hell Maine's five storms, the effects and sounds once where every frame was meant and the chances scaled to the frame rate) carry the recorder's expected difference and an in-game before and after; the original client is not the spec for looks. The invisible ones (the gate drawn twice, uninitialised values, the mesh setting left behind, dead cases, death effects nothing reads) carry the recorder's difference where there is one. Rejected: fixing only the invisible ones (the visible quirks kept as issues), and moving all as they are with every fix later. |

## Current state

Three surveys of the code (2026-11-13, at `1211f0313`) found about **500
places** where code decides how a monster or NPC looks. This section gives
their shape; the pull requests list each case they move.

### Where it happens

The draw and the update were split by ADR 0003 (#917 and after): the draw
only reads, spawns happen in the update.

| Stage | Entry point | Map hooks | Rows |
|---|---|---|---|
| Draw: body passes | `RenderCharacter` (`ZzzCharacter.cpp:9614`), `Draw_RenderObject` (`ZzzObject.cpp:537`) | `RenderObjectMesh` / `RenderMonsterObjectMesh` of 22 map and event files | about 150 |
| Draw: overlays after the weapons | `Render*MonsterVisual` (5 maps, then `TheMapProcess().RenderMonsterVisual`) | Battle Castle, Crywolf, Kanturu, Doppelganger, Empire Guardian 3 | 10 |
| Update: things spawned while alive | `UpdateDrawnCharacterEffects` (`ZzzCharacter.cpp:12149`) → `UpdateMonsterBodyEffects`, `UpdateCharacterTypeEffects`, `UpdateRenderObjectEffects`, `UpdateCharacterCloth`, the map `Update*MonsterEffects` | 20 map files | about 110 |
| Update: movement visuals | `MoveCharacterVisual` (`:5501`) and the map `MoveMonsterVisual` chain | 18 map files | about 60 |
| Weapon trails | `ZzzCharacter.cpp:3780-3954` and the map `MoveBlurEffect` functions | 15 map files | about 45, all but 2 during attacks |
| Attack effects | `AttackEffect` (`:1586`, 87 monster cases) and its chain of 15 map hooks | Hellas (53 cases), Kanturu, Crywolf, Aida, … | about 80 |
| Traps | `SetPlayerAttack` (`:1240-1270`), `SetPlayerShock` (`:1429`) | – | 4 |
| Death | `SetPlayerDie` (`:1448`), `UpdateRenderObjectEffects` debris, the `deathEffect` consumers in 9 map files | – | about 30 |

A map hook claims a monster by returning true; the first map that claims it
wins. Many hooks are **not limited to their own map** (Hunting Ground, Aida,
Kanturu 1st to 3rd, Swamp of Quiet, the Balgass barracks and the monster part
of Hellas), so their looks show wherever the model appears; others are
(Battle Castle, Crywolf, Cursed Temple, Elbeland, Doppelganger, Karutan, PK
Field, Raklion).

### What is data already

`Monsters.json` and `Models.json` carry the values the setup applies
(`MonsterSetupApply.cpp`): `glowMesh` (27 entries), `glowLight`, `hiddenMesh`
(15), `lookLevel` (16), `light`, `shadow: false` (37), `outline` (32),
`alphaTarget`, `pkColor`, `helperPlate`, `subType` and `randomLook`,
`spawnEffects` (25) and the `appear` and `teleport` setups that replaced
`AppearMonster`, `deathEffect` (19, which only arms a flag), and on the
models `streamMesh`, `noneBlendMeshes`, `linkBones`, `namedBones`, `parts`
and `bitmaps`. There is no `renderStyle` or other look name on a monster.

The item looks are data: `Data/Effects/RenderStyles.json` and
`ItemEffects.json`, read by `LookJson.cpp` into `LookDefinition`
(`Data/GameData/LookData/LookDefinition.h`) and drawn by `LookDrawing.cpp`.
Their drawing takes any model (`DrawRenderStyle(style, BMD*, OBJECT*, …)`);
what is item-specific is how a look is found (by the item model entry), the
conditions (worn, inventory, ground, item level) and the per-item values.
Things on bones have no data format yet; for items they come with looks
pull request 4 (FX1.7c and the 12 looks with things on bones).

### What the looks are made of

From the surveys, by kind. "Fits" means an existing block of the item looks
does it once the monster can name a look; "new" means a block or condition
the look format does not have.

**Draw-time (about 160 rows).**

- Fits: layers of texture, reflection (all chrome motions), glow, scroll,
  pulse (wave, bounce), tint and light, per mesh (about 45 %).
- New conditions: the map or map group (about 45 rows: Crywolf draws
  Tantallos, Zaikan, Bloody Wolf, Beam Knight and the dragons in green
  chrome), the animation and frame (about 25: hidden while dying, glow while
  attacking), `SubType` (6), buffs on the monster (8: poison, freeze, the
  Hellas defense buffs, the Crywolf altar states), the map event's state (7:
  the siege, Selupan's pattern, the Maya battle, the Life Stone's build step).
- New blocks: a different look per monster on one shared model (17 rows,
  today the `ExtraMon` number passed into the drawing), an extra model drawn
  with the monster (6: the Red Dragon's boss head and princess on bones, the
  Phoenix of Darkness' second model and its two enlarged copies, Spider Eggs
  drawn three times), an outline shell (5), ground decals and auroras (10:
  the Archangel's ring, the Wolf Altars' circles, the Maya battle rings), an
  afterimage (Dreadfear), a shadow without a mesh (3), the blends dark
  (subtract), wave (vertex wobble), Doppelganger and no depth test, texture
  frames from time, an opacity cap.

**Per frame while alive (about 170 rows, plus 45 attack trails).**

- Things on bones: sprites (about 95 rows), particles (75), effect models
  (14), with offsets, sizes, colours (fixed, random, from a list), turning
  and pulsing; bones by number or by name, lists, ranges, "all but", a
  random bone.
- Links: joints between two bones (8), from a random point near a bone, a
  weapon trail between two bones (2 always on, 45 while attacking).
- Around the monster: walk dust at a random point near it (30), terrain light
  (12), a ground decal.
- Material values: texture scroll, glow-mesh strength pulsing, flickering or
  ramping while attacking (15), eye positions (`MoveEye`, 17).
- Timing: every frame, once per reference frame, a chance of 1 in N, frames
  a to b of an animation, once per animation loop.

**Attacks (about 80 rows).** Attack effects do not follow animation frames:
they run while `c->AttackTime` counts from 1 to 15 (`ZzzCharacter.cpp:4076`),
once per tick (`CheckAttackTime`). So the trigger is "at tick N" or "every
tick from N", filtered by the attack animation and by the skill
(`AT_SKILL_BOSS` for 13 cases, the energy ball, the lightning family, the
Hellas skills, the monster skills 51 and 52 of Dark Ghost). The positions:
the monster, a bone with an offset, a weapon bone, the target, a line from a
bone to the target (joints), a ring of N copies, a random point on the
ground within a radius.

**Death (about 30 rows).** Bones and stones thrown at death (Death Cow, Stone
Golem), parts at bones that take over the body (`deathEffect`: Kundun,
Snowman, Lion Gate, Condra, Totem Golem, the Shadow family, the Ice Giants),
mesh debris (Statue of Saint, Castle Gate), a body hidden while its debris
shows, fades over frames.

**Cloth (7).** The Magic Skeleton's cape (from its mesh 2), the robes of the
Crust family, the Death King and Nightmare, the Dark Phoenix's hair on its
second model, Fred's cloth; the cloak rules (frozen while stunned, none in
Chaos Castle).

### Logic that is more than a look

About 40 rows do more than "create X at Y when Z" (D49's hooks): the Battle
Castle objects that read the siege state, the Kanturu machines and the Maya
battle, Crywolf's Ballista starting its own attack, the Kanturu gateway NPC
that sends the enter request, Kundun's two-stage death with screen shake,
the Cursed Santa's staged death, `CreateInferno` (a ring of eight bombs), the
Hellas skill helpers that also set buffs, the Doppelganger appearing, the
Empire Guardian bosses' rage flags, Dark Phoenix's second-model bones,
`MoveHumming` (particles aimed from bone to bone).

## Problems found

Kept as they are in the move and fixed in commits of their own (D51), unless
a question below decides otherwise.

| # | Problem | Where |
|---|---|---|
| ML-P1 | The Blood Castle red light can never be on in castle 3: the clause compares one number with three (`==` && `==`). Castles 1 and 2 get no light either (`nCastle / 3` is 0). This is the monsters design's P25. | `ZzzCharacter.cpp:12276-12283` |
| ML-P2 | The tints of Alquamos, Drakan and the Cherry Blossom Spirit are dead: `Light` is overwritten at 9907 before the body reads it. | `ZzzCharacter.cpp:9817, 9831, 9883` |
| ML-P3 | The golden monsters' body light swap is a no-op (overwritten by `RenderPartObjectBodyColor`); the golden colours for 493–502 in `ZzzObject.cpp:7026` are never reached. | `ZzzCharacter.cpp:9784-9815` |
| ML-P4 | The shadow list names Gate to Kalima 1 only; gates 2–7 cast shadows (the monsters design's P19). The list is kept twice. | `ZzzCharacter.cpp:9739`, `12229` |
| ML-P5 | The Battle Castle gate is drawn twice while alive and still drawn while dying (`success` is never set). | `GMBattleCastle.cpp:1898` |
| ML-P6 | The Wolf Altars' circles read an uninitialised light and rotation when no buff matches. | `GMCrywolf1st.cpp:1990-2025` |
| ML-P7 | Death Spirit and Satyros leave `StreamMesh` set after drawing. | `GMCrywolf1st.cpp:1430`, `GM3rdChangeUp.cpp:666`, `GM_Kanturu_1st.cpp:741` |
| ML-P8 | Dead cases shadowed by an earlier hook: Kanturu 3rd's Persona, Twin Tail and Dreadfear; the Barracks' Balram, Soram and Death Spirit on Crywolf; Empire Guardian 4's Death Angel 3; `GMUnitedMarketPlace::RenderMonster` is never called. | several |
| ML-P9 | Little Santa and Satyros use the red channel for all three (grey). | `ZzzObject.cpp:2150`, `GM_Kanturu_1st.cpp:747` |
| ML-P10 | Crywolf's Beam Knight joints never show (`to` is always null), and Tantallos and Beam Knight fall through into the generic attack effects. | `GMCrywolf1st.cpp:936` |
| ML-P11 | Hell Maine's loop of five storms fires once. | `GMAida.cpp:1110` |
| ML-P12 | Effects and sounds that run every frame where once was meant: the Hellas skill helpers, Dark Ghost's meteor, the Cursed Temple heartbeat, Queen Rainer's joints. | `GMHellas.cpp:700`, `GMEmpireGuardian2.cpp:1057`, `w_CursedTemple.cpp:160`, `ZzzCharacter.cpp:5693` |
| ML-P13 | Random chances not scaled to the frame rate: the Little Santas' particle, the Cursed King's blood, the Fire Golem's step sound. | `ZzzObject.cpp:2545`, `ZzzCharacter.cpp:5752`, `GMHuntingGround.cpp` |
| ML-P14 | Medusa, Moss and Zyro arm `deathEffect`, which nothing reads for them. | `Monsters.json` |
| ML-P15 | Uninitialised values: Alquamos' attack angle, Aticles Head's sprite offset. | `ZzzCharacter.cpp:2147`, `GMEmpireGuardian2.cpp` |
| ML-P16 | The monsters design's P10 (`ZzzCharacter.cpp:10054`) is no longer at that line; it may be ML-P1. To check. | – |
| ML-P17 | Raymond's sword and the Blood Assassins' back are lit from the red channel through `Vector`, a macro that sets one channel after another, so green and blue read the red just set: Raymond's sword is lit `[3, 9, 9]` times the red, the Blood Assassins' back `[0.65, 0.4225, 0.4225]` and its chrome `[1.3, 1.3, 0.52]`. Meant: the light times 3, then times 0.65 and `[2, 1, 0.4]`. The move keeps them with `{"fromRed": ...}`. | `GMEmpireGuardian1.cpp` (Raymond), `GM_PK_Field.cpp` (Blood Assassins) |
| ML-P18 | The Doppelganger (533) is never tinted by poison, freeze or Blow of Destruction on its maps: their draw hooks skip its body where the tint is set and draw it after the character, in the light without the tint. Fixed before its move (as ML-P7), so the move stays exact: the body hook draws it. | `GMDoppelGanger1.cpp` to `GMDoppelGanger4.cpp` |
| ML-P19 | The second body pass of a frozen monster (the glow drawn over its blue body) took its light, opacity and texture scroll from whatever the monster's draw hook left on the model, not from the freeze. Hooks that put their light back (Lizard Warrior, Queen Bee, Fire Golem, Poison Golem, Axe Hero, Erohim) got the blue tint; hooks that left a light got that light (Soram 1/1/1, Balgass, Tantallos, the Scout hero). Erohim's and the Death Spirit's hooks set the opacity to 1 partway, so their glow stayed fully opaque while they faded, and the Death Spirit's left its scrolled texture, so its glow moved. Blow of Destruction was not affected: its pass sets its own blue light. Found in review of PR 1 (round 3), after the code was gone, so the move could not keep it and its baseline lines are drawn from the data. **Fixed:** a frozen monster's second pass is drawn in the freeze tint at the monster's opacity and without a scroll (`DrawDebuffPass`), as Blow of Destruction's, for monsters with a look and without one; the recorder's frozen lines equal the Blow of Destruction lines, and a test checks that they do. Visible: the frozen glow of the monsters above is blue, fades with the monster, and the Death Spirit's no longer moves. | `ZzzObject.cpp` (the second pass); the old hooks in `GMHuntingGround.cpp` (Erohim and the Hunting Ground monsters), `GMCrywolf1st.cpp` (Death Spirit) and others |

## Target design

What the decisions ML1 to ML11 add up to. Field names are proposals the
pull requests may refine; the decisions are not.

### 1. The look files

Two kinds, as for the items (ML2), in `Data/Effects/`, built as today
(`formatVersion`, `kind`, `looks` sorted by name, folders as the Style Editor
keeps them, L31):

- `RenderStyles.json` (kind `renderStyle`): unchanged for items; gains the
  monster blocks of section 3.
- `Effects.json` (kind `effect`), today `ItemEffects.json` (kind
  `itemEffect`): the file, the kind, the Data Checks area and the Style
  Editor's root are renamed **Effects**, and the items' field `itemEffect`
  becomes `effect` in the same commit, the item files rewritten by their
  writer (the client ships its data, so no old name is read). An effect keeps
  `itemValuesBefore` (renamed `valuesBefore`: they are the monster's values
  too: glow mesh, glow strength, scroll, hidden mesh) and `objectLight`, and
  gains `parts` (section 3).

### 2. How a monster names its looks

`renderStyle` and `effect` on (ML1, ML3, ML7):

| Where | Means |
|---|---|
| a model entry in `Models.json` | the looks of every monster of that model |
| a monster entry in `Monsters.json` | its own instead of the model's |
| one of its `numbers` | that number's own (D61) |
| a `mapVariants` entry | on those maps only, where a map changes the look on purpose (ML7) |
| a `stateVariants` entry | while that named state holds (`castleSiegeRunning` exists; the Maya battle, Selupan's pattern and the Life Stone's build step become states) |

They are resolved in the order the setup already uses for its other fields
(MN4): state variant, map variant, number, entry, model. `"none"` draws
plainly where the model has a look. Names are resolved when loading and at
spawn (D26): the drawing and the update never look a name up, they hold the
resolved looks on the character, as items hold theirs on `DrawLook`. The
`ExtraMon` number that `RenderCharacter` passes into the drawing for 17
monsters goes: those monsters name their own render style.

### 3. What a look holds

**Render style (draws).** The item layers as they are (L8 to L11), plus:

| Block | For | Rows |
|---|---|---|
| `onlyWhen` gains `animation`, `notAnimation`, `frames` (from, to), `buff`, `notBuff` (the monster's buffs: poison, freeze, the Hellas defenses, …) and `opacityAbove` (ML3) | hidden while dying, glowing while attacking, blue while frozen | about 35 |
| blends `dark` (subtract), `wave` (vertex wobble), `doppelganger`, and a layer field `depthTest: false` | Kundun's ghost, Queen Rainer, the Doppelgangers, the Zombie Fighters | 9 |
| `alsoDraw`: another model (by model slot, at a bone, scale, in a frame range), copies of the model (offsets or bone scales), a shell (the model larger, chrome or flat colour), an afterimage (earlier frames at an opacity) | the Red Dragon's head and princess, Phoenix of Darkness, Spider Eggs, the outlines of Hellas, Dreadfear | about 12 |
| `decals`: a texture on the ground or at a bone, size, colour, turning (L9 shapes) | the Archangel's ring, the Wolf Altars, the Maya battle rings, Santa's auroras | 10 |
| `shadow`: `false`, or the meshes it leaves out | the shadow list (ML-P4), the Archangel's wings | about 15 |
| `textureFrames`: a texture slot stepping by time | the Battle Castle shield | 1 |

**Effect (spawns, in the update).** `valuesBefore`, `objectLight` and
`parts`. A part is one thing, a `when`, and optionally an `onlyWhen`:

- **when** (ML2, ML4): `"always"` (with its own timing: every frame, once per
  reference frame, a chance of one in N, every N frames, a frame range of an
  animation, once per loop, once per action), an attack
  (`{"attack": "attack1" | "any", "skill": "boss", "tick": 11}` or
  `"fromTick"`, `"everyTick"`, or `"frame": 6.4` instead of the tick), or
  `"death"` (with a frame range).
- **things** (ML5, with FX1 names): `sprite`, `particle`, `effect`,
  `joint` (between two bones, from a point near a bone, from a bone to the
  target), `trail` (between two bones, sampled between frames), `dust` (at a
  random point within a radius), `terrainLight`, `decal`, each with bones
  (numbers or names, lists, ranges, "all but", a random bone, the second
  model's bones), offset, jitter, size, colour (fixed, random, from a list),
  turning and pulsing (L9).
- **where** for attack and death parts: the monster, a bone, a weapon bone,
  the target, a line from a bone to the target, a ring of N copies, a random
  point on the ground within a radius.
- **presets** (ML6): `inferno` (the ring of eight bombs of `CreateInferno`)
  and others the move finds used by several monsters; a preset is a named
  list of parts the editor can open.
- **values from the attack** (ML6): a size or light that follows the tick
  (the Hydra laser).
- **cloth** (ML8): bone (or the second model's), offset, shape (grid N × M
  and size, cylinder, from a mesh), texture, feel, see-through edges,
  collision spheres, lights from the cloth, and its `onlyWhen`.
- **acting blocks** (ML6): `setAnimation`, `setFlag` and an `onlyWhen` on a
  flag, `setBuff`, `setHeight`, `selectable`, `hideMesh`, `screenShake`,
  `breakIntoMeshes`, `parts on death` that take over the body (the
  `deathEffect` consumers).

The two hooks left (ML6) are named in the monster entry's `hooks` as today
(D49): `kanturuGatewayEnter` and the Maya battle's.

### 4. Drawing and update

- `RenderCharacter` and `Draw_RenderObject` draw a monster with its resolved
  render style through `DrawRenderStyle` (`LookDrawing.cpp`), as items are
  drawn; the monster cases of `Draw_RenderObject`, the map
  `RenderObjectMesh` / `RenderMonsterObjectMesh` hooks and the
  `Render*MonsterVisual` overlays lose each case as it moves.
- `UpdateDrawnCharacterEffects` runs the resolved effect's "always" and
  death parts (ADR 0003: spawns in the update); the attack parts run where
  `AttackEffect` runs today, once per tick (`CheckAttackTime`) or at their
  frame. The map `Update*MonsterEffects`, `MoveMonsterVisual`,
  `MoveBlurEffect` and `AttackEffect*` hooks lose each case as it moves, and
  their map checks go (ML7).
- Cloth parts replace `UpdateCharacterCloth` and `CloakOf` for monsters.

### 5. Sounds

The visual code's sounds become events of the monster's sound set (ML9), in
the format MNA and MNK use (`during`, `once`, cues, `chance`, `pick`), moved
with the visuals of the same monster.

### 6. Checks

The three levels of L23, in the Data Checks areas Render styles and Effects
and in Monsters:

- **Fatal:** a part, `when`, blend, block or condition that does not exist;
  a value of the wrong type; a tick outside 1 to 15; a cloth without a shape.
- **Error:** a look name nobody defines; an FX1 name that does not exist; a
  bone, mesh or animation the monster's model does not have; a preset that
  does not exist.
- **Warning:** an acting block on a monster without that animation; a flag
  that is set and never read, or read and never set; an attack part on a
  monster without attacks; a map variant that only names the monster's own
  maps (ML7); a look no item or monster uses.

### 7. The editor

ML10: the Style Editor edits all looks. For monsters it adds the preview
(model, animation, playing or stopped, the frame ruler with the attack parts,
cloth, map variants), the recipes of ML5 (lights on bones, eye lights,
thunder chain, fire on bones, walk dust, trail, cloth, inferno ring) and
"used by" with monsters. Monster Setup and Monster Model show the two
dropdowns with Edit. Monster Sounds gets the moved events (ML9).

### 8. The move and its proof

As D50 and L25: for each group of looks, a test draws, updates and attacks
with every monster of the group on each map its numbers are placed on (D61's
`maps`), with the old code and with the data in the same build, and compares:

- the draws with the draw recorder of items phase 13 (`tests/data/LookRecorder`);
- the spawned effects, particles, sprites and joints with FX1's effect
  recorder (`tests/effects`), per animation, frame and attack tick;
- what acting blocks and cloth change on the object (action, flags, height,
  selectable, buffs) with the monster recorder (`tests/monsters`);
- the sounds with the monster recorder's sound part.

Then the old code goes. The fixes of ML11 follow in commits of their own,
each with the recorder's expected difference, and the visible ones with an
in-game before and after.

### 9. Pull requests

Proposed (Q-ML11), each as a fork pull request for CI and an upstream one
for sven-n (the fork one closed after the upstream merge), each after the one
before is merged. This design has no pull request of its own: it goes in
with PR 1, the first one that changes code.

| PR | Contents | Waits for |
|---|---|---|
| 1 | This design, and the machinery: monsters name looks (section 2) and `ExtraMon` goes; the rename to Effects; `onlyWhen`'s monster conditions; the Style Editor's monster preview and the two dropdowns; the draw-time looks that need only layers and conditions (about 90 rows: chrome, glow, scroll, pulse, per mesh). | – |
| 2 | The other draw-time blocks: blends, `alsoDraw`, decals, shadow, texture frames; the map variants of ML7 and the map checks gone. | 1 |
| 3 | Things on bones: the parts of section 3 with "always", the recipes, the per-frame rows (about 170) and the always-on trails. | 2 and looks PR 4 (FX1.7c and the items' things on bones, the same parts) |
| 4 | Attacks: attack parts with tick and frame (ML4), the frame ruler, presets, the attack trails, `AttackEffect` and its map hooks (about 80 rows) and the traps. | 3 |
| 5 | Death, cloth and the acting blocks: death parts and debris, the `deathEffect` consumers, the cloth part (ML8), the acting blocks and the two hooks (ML6). | 4 |
| 6 | The sounds (ML9). | 5, or beside 3 to 5 |

The fixes of ML11 go into the pull request that moves their look. MN9 (the
monsters' cleanup) follows.

## Open questions

Asked one at a time, each with a mockup of the data and the editor, and
answered here as decisions.

| # | Question |
|---|---|
| Q-ML1a | *Answered: ML1.* Where a monster names its looks. |
| Q-ML1b | *Answered: ML2.* Which kinds of looks. |
| Q-ML2 | *Answered: ML3.* Conditions. |
| Q-ML3 | *Answered: ML4.* When an attack effect fires. Where the trigger names the attack (animation, skill) is part of the target design. |
| Q-ML4 | *Answered: ML5.* Things on bones. |
| Q-ML5 | *Answered: ML6.* Logic that is more than a look. |
| Q-ML6 | *Answered: ML7.* Looks on other maps. |
| Q-ML7 | *Answered: ML8.* Cloth. |
| Q-ML8 | *Answered: ML9.* The sounds of the visual code. |
| Q-ML9 | *Answered: ML10.* The editor. |
| Q-ML10 | *Answered: ML11.* Which problems are fixed. |
| Q-ML11 | **The pull requests.** Proposed in section 9 of the target design; the owner may merge or reorder steps in the review of this document. |

## Tasks

1. ~~Answer Q-ML1 to Q-ML10 and write the decisions~~ (ML1 to ML11).
2. ~~Write the target design~~ (sections 1 to 9).
3. ~~Link this document from the roadmap and the monsters design's MN8 row.~~
4. The design as a docs pull request, reviewed before code (pull request 0),
   with Q-ML11 settled in its review.
5. Pull requests 1 to 6.
6. Last: what stays true into `docs/monster-data.md` and
   `docs/effect-data.md`; this document deleted.
