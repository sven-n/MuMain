# Buffs as Data (Area BF) Design (WIP)

> **Draft, 2026-11-08.** Area BF of the
> [data-driven content roadmap](2026-09-29-data-driven-content-roadmap-design.md).
> Written from the code of MuMain `upstream/main` @ `f381344cc` and OpenMU
> `master` @ `00f737374`. Line numbers are those of these commits.
>
> **Agreed with the owner on 2026-11-08** (Q-BF1 to Q-BF13, decisions B1 to
> B15). All questions are answered.
>
> This document is temporary. Its last task deletes it, after what stays true
> has moved into `docs/buff-data.md` and the code (see "Tasks").

## Goal

Two gaps in the game, and the data both need:

- **Skill tooltips don't say what a buff skill grants.** The Dark Lord's
  Increase Critical Damage shows its name, mana, AG and requirements, but not
  that it adds critical damage (1 per 30 energy and 1 per 25 command, for
  60 s + 1 s per 10 energy, at most 180 s).
- **The buff icons on top show no remaining time and no values.** Their
  tooltip is a fixed sentence ("Critical damage increases.").

Every buff and debuff becomes data: its names and descriptions in every client
language, whether it is a buff or a debuff, the item that gives it, and what it
adds and for how long, as OpenMU's effect definitions say. The skill tooltip,
the buff icons and their tooltips read that data. OpenMU tells the client the
time left and the value of each buff on the hero.

## Scope

- **In:** `Data/Buffs/Buffs.json` and `Data/Buffs/BuffTexts.json`, replacing
  `BuffEffect_<lang>.bmd`; the time of each buff on the hero; a time bar under
  each buff icon; value lines in the buff tooltip and in the skill tooltip; an
  OpenMU change that sends the time left and the values.
- **Later:** a Buffs tool in MuEditor; syncing the buff data with OpenMU's
  configuration (like items phase 7); buff visuals (the roadmap's "later" row).
- **Out:** skills as data (SK1); this area only reads the skill data's `Effect`
  field, which names a skill's buff.

## Decisions

| D | Topic | Decision |
|---|---|---|
| B1 | Where buff facts live (Q-BF1) | A data file with one entry per buff, mirroring OpenMU's `MagicEffectDefinition`: duration and power-ups with the value format of the item options design (section 4 there, `per` terms, O7). Texts in every client language. A MuEditor tool and the OpenMU sync come later. The skill tooltip and the buff strip read the same entry, so they can't disagree. |
| B2 | Where the remaining time comes from (Q-BF2) | Both: the client computes the duration of the hero's own casts from the data (B1) and restarts it on the hero's own recast; a time the server sends wins. An OpenMU pull request makes the server send the time left for every effect on its owner, at activation and whenever the effect restarts. |
| B3 | A buff someone else cast (Q-BF3) | The server sends each power-up's finished value next to the time (B2), only at activation or restart, in the same OpenMU pull request. Only the server knows the caster's stats and master skills. The client's formulas stay for the skill tooltip, for the hero's own casts on servers without the change, and as fallback; formulas that differ from a server's are fixed by the later sync. |
| B4 | The time bar (Q-BF4) | As wide as the icon (20 dp), 2 dp tall, 1 dp under it, inside the strip's 5 dp row gap, so rows keep their places. It empties from right to left. Buffs green, debuffs red; the colors are theme tokens. |
| B5 | An unknown time (Q-BF4b) | No bar and no time line: a bar always means "this much time left". |
| B6 | The last seconds (Q-BF4c) | Under 10 s the icon darkens a little and shows the whole seconds left (8, 7, 6 …). No blinking. The limit is a player option (B14). |
| B7 | The buff tooltip (Q-BF5) | Name (blue, bold), then one white value line per power-up ("Critical damage +126"), then a purple "Time left: 2 min 41 s" ("12 s" under a minute). An effect without values (Poison) shows its description instead of value lines. |
| B8 | No value from the server (Q-BF5b) | On a server without B3, a buff someone else cast shows its value lines with "?" ("Critical damage +?") and a gray "The value depends on its caster". |
| B9 | The skill tooltip (Q-BF6) | A buff skill (one whose skill data names a buff) shows its buff's value lines with the hero's stats and master skills. Each total gets gray part lines, one per part above 0, each naming its rule: "energy 2000: +66 (1 per 30)". The duration gets the base, its terms, the cap and the master skill parts. This replaces the hand-written Elf and Soul Barrier lines. |
| B10 | Names, descriptions, kind and item links (Q-BF7) | All 158 entries of `BuffEffect_<lang>.bmd` move into `Buffs.json`: names and descriptions in every client language, buff or debuff, group, the item that gives it. The bmd is no longer read. Broken English, `%%` and `%d` are fixed on the way. |
| B11 | Roadmap (Q-BF8) | A new area "BF Buffs as data" before SK1, with this document. The roadmap's "later" row keeps only buff visuals. SK1 names buffs from `Buffs.json`. |
| B12 | The strip's rules (Q-BF10) | They move into `Buffs.json` too: a buff's `family` and `tier` (only the highest tier of a family shows, today `NormalizeBuffState` and `BuffTier`), `hidden` (no icon, today `SetDisableRenderBuff`) and `cancelSkill` (a right-click asks to cancel it and sends that skill, today `CancellableSkill`). The Buffs tool notes that the server must accept a cancel too (OpenMU: skills 77, 441, 233, 380, 383). |
| B13 | Item buffs' values and durations (Q-BF11) | `Buffs.json` is the one place (D26), as in OpenMU, where an item's `ConsumeEffect` points at the magic effect that holds the duration and the boosts. The item tooltip's `{addOptionValue1}`, `{addOptionValue2}` and `{addOptionDuration}` read the item's buff (the buff whose `item` is that item); the buff part of `ItemAddOption.bmd` goes. The item options design (O13) and the items design (phase 11) get a note that this part moved here. |
| B14 | Where the countdown limit lives (Q-BF12) | A player option in the options window's Interface tab, "Buff countdown", 0 to 30 s (0 turns it off), saved in `config.ini`, default 10. |
| B15 | Who brings the value terms (Q-BF13) | Item options 9a3, as its design plans it (O7, O20). Buffs PR 1 and the OpenMU pull request can come any time; buffs PR 2 and PR 3 follow 9a3. |

## Current state

### Client

- **Skill tooltip.** `UI::Skills::Tooltip::BuildModel`
  (`src/source/UI/HUD/Skills/SkillTooltipModel.cpp`): name, damage, range,
  mana, AG, cooldown, requirements, banners, element. Values of a buff only for
  Soul Barrier (`:219-236`: absorb 10 % + agility/50 + energy/200, mana 2 % of
  max mana, 60 + energy/40 s) and the Elf's Heal, Greater Defense, Greater
  Damage and Recovery (`EmitElfBuffValues`, `:356`). These match OpenMU. The
  other buff skills return early (`EmitPhysicalDamage`, `:262`, the list from
  `:305`). The master skill tree's node hint
  (`CMasterLevel::BuildNodeHintLines`, `UI/HUD/MasterLevel.cpp:780`) is the
  only place a power-up's value shows, from `MasterSkillTooltip_<lang>.bmd`.
- **Skill data.** `skill_<lang>.bmd` (650 records of 108 bytes). Its `Effect`
  field is the buff a skill gives (64 Increase Critical Damage → 5, 511 and 515
  → 5, 517 → 148, 522 → 149, 48 Swell Life → 8, 16 Soul Barrier → 4, 27 → 2,
  28 → 1, 77 → 6, 218 → 81, 233 → 82, 217 → 71). No code reads it.
- **Buff strip.** `mu::ui::window::CBuffStrip` (`UI/HUD/BuffStrip.cpp`),
  `buff_strip.rml`, `themes/legacy/buff_strip.rcss` (classic inherits it) and
  `themes/modern/buff_strip.rcss`: 20 × 28 dp slots, 5 dp gaps, 8 per row,
  rebuilt every frame. Only the highest tier of a buff family shows
  (`NormalizeBuffState`, `:28`), four damage debuffs are hidden
  (`SetDisableRenderBuff`, `:81`), right-click cancels Infinity Arrow and
  Expansion of Wizardry (`CancellableSkill`, `:153`). The tooltip
  (`BuildTooltip`, `:168`): name, the description lines, "Duration period"
  when a time is known.
- **Buff texts.** `BuffScriptLoader` reads
  `Data/Local/<lang>/BuffEffect_<lang>.bmd`: 158 records with index, effect
  type (group), item, name, buff or debuff, description (lines split at `/`).
  Only `Eng`, `Por` and `Spn` exist, chosen by the old data-language setting
  (`Winmain.cpp:2091-2094`), not the interface language. Other users: quest
  rewards (`QuestMng.cpp:489`, `:726`, the name) and the cash shop
  (`MsgBoxIGSUseItemConfirm.cpp:30`, the group of an item's buff).
- **Time.** `BuffTimeControl` (`GameLogic/Buffs/w_BuffTimeControl.cpp`). The
  only packet with a time is 0x2D (`ReceiveBuffState`, `WSclient.cpp:10214`,
  seconds, the hero only); the per-character 0x07 (`ReceiveSkillStatus`,
  `:3886`) has none. With time 0 an item buff takes `ITEM_ADD_OPTION::m_Time`
  from `ItemAddOption.bmd`.

### Server (OpenMU)

- **Definitions.** `src/Persistence/Initialization/Skills/*EffectInitializer.cs`
  and, for items, `VersionSeasonSix/Items/BuffItems.cs` and `Potions.cs`. A
  power-up's value is a constant plus terms on attributes, with a maximum; the
  duration too. Skill buffs read the **caster's** attributes
  (`GameLogic/MagicEffectPowerUpExtensions.cs:40-75`), items the receiver's.
  Master skills add to a power-up or to the duration (`:73-127`), the duration
  after its cap.
- **Live values.** A skill buff's value is an element over the caster's
  attributes (`GameLogic/Attributes/AttributeSystemExtensions.cs:68-92`): when
  the caster's energy changes later, the buff on the party member changes at
  once, without a packet.
- **Packets.** Activation sends 0x07 `MagicEffectStatus` (on/off, player,
  effect), or, to the owner, 0x2D `EffectItemConsumption` for an effect with a
  power-up on one of five stats (`GameServer/RemoteView/World/DeActivateMagicEffectPlugIn.cs:27-35,67-80`).
  `SendDuration` only decides whether 0x2D's `RemainingSeconds` is the time or
  0. In practice 11 item effects get a time (35, 36, 38, 39, 78, 79, 80, 44,
  48, 49, 121). No packet carries a value.
- **Recast.** A recast of an active effect, or a stronger one from someone
  else, restarts the server's timer and may swap the value, without a packet
  (`GameLogic/MagicEffectsList.cs:367-417`). Expiry sends 0x07 off.
- **Logout.** All effects are removed when a player leaves the world
  (`GameLogic/Player.cs:1376`); nothing is stored.

The full survey (every effect, its flags, duration, power-ups and master skills)
was written for this design; its numbers go into `Buffs.json` with pull request 2.

### Problems found

| P | Problem |
|---|---|
| P1 | The client keeps a time from the server only for a fixed list of buffs (`InsertBuffLogicalEffect`, `WSclient.cpp:15063`); any other buff's time is dropped. |
| P2 | Timers are keyed by the buff's group (`1005 + s_BuffEffectType`) and `RegisterBuffTime` (`w_BuffTimeControl.cpp:90`) returns when that key runs, so a new time doesn't restart it. |
| P3 | Under a minute the time reads "Less than 1 minute" (`GetStringTime`, `:205`). |
| P4 | Descriptions are not formatted: `%%` shows (Potion of Bless "Attack power +20%%", the seals) and Bless shows "Increases all stats by %d." (buffs 142, 161). |
| P5 | Buff texts exist only in English, Portuguese and Spanish, so a German player reads English names. |
| P6 | OpenMU: Jack O'Lantern Cry (37) and Scroll of Defense (45) add `DefenseFinal`, but 0x2D maps `DefenseBase`, so their time is never sent; `SendDuration` on Infinity Arrow and Defense Reduction has no effect. |
| P7 | OpenMU: a recast or a re-buff restarts the effect without telling the client. |

## Design

### 1. Files

- `Data/Buffs/Buffs.json`: one entry per buff or debuff number (B1, B10).
- `Data/Buffs/BuffTexts.json`: the texts the tooltips build lines from, with
  placeholders, like `Data/Items/Tooltips/OptionTexts.json`: one value line per
  power-up kind (`"criticalDamageBonus": "Critical damage +{value}"`), the time
  line, "The value depends on its caster", the part lines and rules of B9.

Both have `formatVersion` and follow the format rules of items phase 2.

### 2. An entry

```json
{
  "number": 5,
  "name": "criticalDamageIncrease",
  "kind": "buff",
  "group": 5,
  "displayName": {"en": "Critical Damage Increase", "de": "Kritischer Schaden erhöht", ...},
  "openMu": {"number": 5, "name": "Critical Damage Increase"},
  "valuesFrom": "caster",
  "duration": {"values": 60, "per": [{"attribute": "totalEnergy", "divisor": 10}], "maximum": 180},
  "powerUps": [
    {"attribute": "criticalDamageBonus",
     "add": {"values": 0, "per": [{"attribute": "totalEnergy", "divisor": 30},
                                  {"attribute": "totalLeadership", "divisor": 25}]},
     "text": "criticalDamageBonus"}
  ],
  "masterSkills": [
    {"skill": 511, "adds": "criticalDamageBonus"},
    {"skill": 515, "adds": "duration"}
  ]
}
```

```json
{"number": 55, "name": "poisoned", "kind": "debuff", "group": 27,
 "displayName": {"en": "Poison", ...},
 "description": {"en": "Loses HP over time", ...}}

{"number": 44, "name": "scrollOfQuickness", "kind": "buff", "group": 67,
 "item": {"group": 14, "number": 72}, "valuesFrom": "owner",
 "duration": {"values": 1800},
 "powerUps": [{"attribute": "attackSpeedAny", "add": {"values": 15}, "text": "attackSpeed"}]}

{"number": 136, "name": "swellLifeMastery", "family": "swellLife", "tier": 2, ...}
{"number": 83, "name": "flameStrikeDamage", "hidden": true, ...}
{"number": 6, "name": "infinityArrow", "cancelSkill": 77, ...}
```

- `number`: the buff number the server sends (`eBuffState`, OpenMU's effect
  number). `name`: the identifier other data uses.
- `kind`: `buff` or `debuff` (today's buff class). `group`: today's effect type;
  the cash shop asks whether a buff of the same group is active.
- `displayName`, `description`: every client language; `description` only where
  the tooltip shows it (B7).
- `item`: the item that gives the buff (today's item type and index).
- `valuesFrom`: `caster` (skill buffs) or `owner` (items, the Elf Soldier).
- `duration`, `powerUps[].add` / `percent`: the value format of the item options
  design, section 4 (O6, O7, O11), plus `maximum`. The attribute names are
  OpenMU's, as the item options use them.
- `masterSkills`: the hero's master skills that add to a power-up or to the
  duration; their values come from the server's master skill list
  (`CharacterAttribute->MasterSkillInfo`), as the critical damage line already
  uses them.
- `family`, `tier`, `hidden`, `cancelSkill`: the strip's rules (B12).
- An item buff's numbers are here too (B13); the item's tooltip reads them.

### 3. Values

The value of a term is the attribute times or divided by its number, as OpenMU
computes it; the shown number is rounded down, like the character window. The
evaluator is the one of the item options' `per` terms, which item options 9a3
brings (B15). The hero's
attributes are those the character window shows (total energy, total command,
…). Swell Life also counts the party members near the caster; how the client
counts them like the server is settled in pull request 2.

### 4. Time

For each buff on the hero the client keeps its full duration, the time left
and where the time came from (computed or sent).

- **Own cast:** when the hero casts a skill whose skill data names the buff,
  and the buff comes on or is already on, the time starts or restarts at the
  computed duration (B2).
- **Sent:** a time from the server (0x2D today, the new packet of section 5)
  sets both and wins over a computed one.
- **Unknown:** neither: no bar, no time line (B5).
- The fixed list of `InsertBuffLogicalEffect` goes (P1); timers are keyed by the
  buff number and every new time restarts them (P2).
- An effect ends with 0x07 off. Nothing carries across logins, since OpenMU
  removes every effect when a player leaves the world.

### 5. OpenMU change

A pull request to MUnique/OpenMU, proposed separately:

- At activation, and when `UpdateEffect` restarts an effect, the owner gets the
  time left and each power-up's current value (B2, B3).
- Only clients that understand it get it: OpenMU picks view plug-ins by client
  version. The original client keeps getting 0x07 and 0x2D as today.
- The packet's shape is decided in that pull request (an extended 0x2D or a new
  custom packet). A value is what it is at that moment; a later change of the
  caster's stats shows at the next cast or restart.
- It can also fix P6.

### 6. Buff strip

- Each slot gets: how full its bar is (or none), whether it is a debuff, and the
  seconds to show (or none). The RML adds a bar element under the icon and a
  seconds overlay.
- Both themes (classic inherits legacy) draw the bar at 20 × 2 dp, 1 dp under
  the icon, inside the row gap (B4). Colors as tokens in each theme's
  `tokens.ini`: the buff bar, the debuff bar, the empty track.
- Under the countdown limit (the player's option, 10 s by default, B6 and B14)
  the icon gets a dark overlay and the whole seconds left; with 0 there is none.
- The families, hidden buffs and right-click cancels come from the data (B12).
- The model is rebuilt every frame today; the new values change it only when the
  shown percent or second changes (coding rules, section 12).

### 7. Buff tooltip

| Case | Lines |
|---|---|
| Own buff | Critical Damage Increase · Critical damage +126 · Time left: 2 min 41 s |
| Several power-ups | Swell Life · Maximum HP +33% · Maximum mana +3% · Time left: 1 min 5 s |
| No values | Poison · Loses HP over time · Time left: 12 s |
| Someone else's, value sent (B3) | Critical Damage Increase · Critical damage +126 · Time left: 3 min |
| Someone else's, no value (B8) | Critical Damage Increase · Critical damage +? · *The value depends on its caster* |

### 8. Skill tooltip

A Dark Lord with 2000 energy and 1500 command, PowUp at level 10 (+39) and
PowUp (2) at level 5 (+31 s):

```
Critical DMG Increase PowUp
Critical damage +165
  energy 2000: +66 (1 per 30)
  command 1500: +60 (1 per 25)
  Critical DMG Increase PowUp: +39
Duration: 211 seconds
  base: 60 s
  energy 2000: +200 s (1 s per 10)
  at most 180 s before master skills
  Critical DMG Increase PowUp (2): +31 s
Mana: 75
AG use: 75
```

- The value lines come right after the name, before range and mana.
- Gray lines show the parts above 0; master skills by their names. The cap line
  shows only when the cap cut the duration.
- The Elf and Soul Barrier lines written in code go; the same numbers come from
  the data (a test checks them, see "Tests").
- MuEditor's skill tooltip preview has no hero, so it shows the rules without
  numbers.

### 9. Texts

- Names and descriptions in every client language (12 today), in `Buffs.json`.
- The line texts of B7 to B9 in `BuffTexts.json`; the stat names are the
  character window's.
- The English of the bmd is rewritten where it is broken (P4, "Poisoned status
  of condition"), then translated like the item texts (P5).

### 10. Loading and checks

- Loaded at start with the other game data. An unknown attribute, text key or
  skill, or a number used twice, is reported when loading (D26).
- It replaces `BuffScriptLoader`; the buff class, the names (strip, quest
  rewards), the group (cash shop) and the item links read the new data.
- The buff part of `ItemAddOption.bmd` (`m_Time`, the values) is no longer read
  for buffs and item tooltips (B13); a comparison test checks that the item
  tooltips show the same numbers as before, and lists where OpenMU stores a
  number another way (the Scroll of Defense shows +60 base defense; OpenMU adds
  half of it to the final defense, `BuffItems.cs:50-57`).

## Pull requests

A proposal; each one says what it changes in game.

1. **Buff data.** `Buffs.json`, `BuffTexts.json`, the loader and its checks; all
   158 buffs with names, descriptions, kind, group, item link and the strip's
   rules (B12) in every language; the bmd is no longer read. A comparison test:
   kind, group, item link, family, tier, hidden and cancel equal today's for
   every buff. Visible change: the texts (P4, P5). Needs no value terms, so it
   can come any time.
2. **Values and durations**, after item options 9a3 (B15). The OpenMU numbers
   for the skill and item effects; the item tooltips' add option values read
   the item's buff (B13); the skill tooltip lines (B9) replace the hand-written
   ones.
3. **Time and the strip.** Time tracking (B2, section 4), the bars and countdown
   (B4 to B6) with the player option (B14), the buff tooltip (B7, B8); fixes P1
   to P3.
4. **OpenMU** (MUnique/OpenMU): time and values for the owner (section 5); any
   time.
5. **The client reads it** (B3): sent times and values win.

Later: the Buffs tool in MuEditor ("used by": the skills whose data names the
buff, the items that give it); the OpenMU sync.

## Tests

- **Data:** every entry loads; kind, group and item link equal the bmd's; every
  OpenMU effect number has an entry.
- **Values:** Increase Critical Damage with 2000 energy and 1500 command gives
  126 (165 with PowUp at +39), 180 s at the cap, 211 s with PowUp (2); Greater
  Damage with 1200 energy gives 174; the Elf and Soul Barrier numbers equal the
  old tooltip code for a range of stats.
- **Time:** an own cast starts at the computed duration and a recast restarts it;
  a sent time wins; no time gives no bar; 0x07 off ends it.
- **Layout:** the bar sits in the row gap and no row moves, in both themes (like
  the critical damage branch's layout test).
- **Texts:** the tooltips in English and German.
- **In game:** own buffs (Dark Lord, Elf), a party member's buff, an item buff and
  a debuff, with and without the OpenMU change.

## Open questions

Answered on 2026-11-08: Q-BF1 (B1), Q-BF2 (B2), Q-BF3 (B3), Q-BF4 (B4),
Q-BF4b (B5), Q-BF4c (B6), Q-BF5 (B7), Q-BF5b (B8), Q-BF6 (B9), Q-BF7 (B10),
Q-BF8 (B11), Q-BF10 (B12), Q-BF11 (B13), Q-BF12 (B14) and Q-BF13 (B15); Q-BF9
placed this document on its own branch. New questions from the pull requests
are added here.

Left to the pull requests:

- The OpenMU packet's shape (section 5).
- How the client counts Swell Life's nearby party members like the server
  (section 3).

## Tasks

1. [x] Answer Q-BF1 to Q-BF9 (B1 to B11).
2. [x] Answer Q-BF10 to Q-BF13 (B12 to B15).
3. [x] Add the note of B13 to the item options design (O13) and the items design
   (phase 11).
4. [ ] The pull requests of "Pull requests".
5. [ ] Move what stays true into `docs/buff-data.md`, and point the roadmap's row
   BF at it.
6. [ ] Delete this document.
