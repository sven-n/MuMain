# Buff data

The buffs and debuffs the game shows are data in `Data/Buffs/Buffs.json`,
loaded at start; a problem in the file stops the game with a message that
names the entry and the field. The file replaces `BuffEffect_<lang>.bmd`.
The design and the steps still to come are in
[the buffs design](superpowers/specs/2026-11-08-buffs-design.md).

## The file

```json
{
  "formatVersion": 1,
  "buffs": [
    {
      "number": 6,
      "name": "infinityArrow",
      "kind": "buff",
      "group": 6,
      "displayName": {"en": "Infinity Arrow", "es": "Flecha Infinita", "pt": "Flecha do Infinito"},
      "description": {"en": "Arrows are not consumed.", "es": "Las flechas no se consumen."},
      "family": "infinityArrow",
      "cancelSkill": 77
    }
  ]
}
```

| Field | Meaning |
|---|---|
| `number` | The effect number the server sends (1 to 255); the icon is the one of that number. The game shows buffs 1 to 205 only (`eBuff_Count`); a higher number is a warning. An effect number the server sends that has no entry gets no icon or tooltip, and none of the effects the game starts when a buff arrives (its countdown, its arrival effects); checks of whether a character has the buff still see it. |
| `name` | The name other data uses for the buff: letters and digits, used once. |
| `kind` | `buff` or `debuff`. On the buff strip, buffs come first and debuffs last. |
| `group` | Buffs of one group are the same kind of effect. The cash shop asks before an item replaces an active buff of its group. Buffs of one group share one countdown: a second buff of the group shows no time left, and the end of either ends the countdown. |
| `displayName` | The name in the tooltip, in every language the game has; a language without one shows the English. |
| `description` | The tooltip lines under the name, separated by `\n`; left out for none. |
| `item` | The item that gives the buff (`group`, `number`); left out for none. The item's tooltip shows the buff's values and duration (see [Values](#values)). Moving the item in the item tools does not move this link yet (the Buffs tool will); the buff then names a missing item, an error. |
| `duration` | How long the buff lasts in seconds, in the [value format of the item options](item-data.md#item-options) (`{"values": 1800}`, without `per` terms), when the server sends no time; left out when only the server knows it. Only an item buff's countdown uses it. |
| `powerUps` | What the buff raises, in their order: each an `attribute` and its value in the item options' format under `add`. `addFinal` and `percent` are read, but the client adds every value as `add` (a warning). See [Values](#values). |
| `family`, `tier` | Buffs of one family are tiers of the same buff: when several are on, the buff strip shows only the highest tier. `tier` is left out for 0, the lowest. |
| `hidden` | `true`: the effect gets no icon on the buff strip. |
| `cancelSkill` | A right-click on the icon asks to cancel the buff and sends this skill to the server. The server decides whether it may be cancelled; OpenMU allows it for Infinity Arrow and Expansion of Wizardry and their strengthened tiers (skills 77, 441, 233, 380, 383). |

A number or a name used twice, one tier of a family used twice, or a family of
buffs and debuffs stops the game. An item the item list does not have is an
error: the buff gets no duration or values from it. A field the game doesn't
know, a text in a language the game has no texts in, and a name longer than 49
letters (it is cut) are warnings. All of them go to the log and show in the
editor's Data Checks (area **Buffs**,
[dev-editor.md](dev-editor.md#data-checks)).

## Values

The values and durations of the 45 buffs that items give (scrolls, elixirs,
seals, the Jack O'Lantern and cherry blossom items, talismans) are Webzen's
Season 6 numbers, as the item add option table (`ItemAddOption.bmd`) has them.
The file is unchanged, but the client reads only its guardian (380) options
now; a test checks that every item buff keeps its row. The character stats, the
Scroll of Battle's critical damage rate, the countdown of an item buff without
a time from the server and the item tooltips read them from here; an item's
tooltip shows its buff's first value as `{addOptionValue1}`, the second as
`{addOptionValue2}` and its duration as `{addOptionDuration}`.

```json
{"number": 45, "name": "scrollOfDefense", "kind": "buff", "group": 68,
 "item": {"group": 14, "number": 73},
 "powerUps": [{"attribute": "defenseBase", "add": {"values": 60}}]}
```

- **Attributes.** OpenMU's attribute names in camelCase where OpenMU has the
  attribute (`defenseBase`, `attackSpeedAny`, `experienceRate`), else a name
  of Webzen's add option (`disableExperience`, `mobility`,
  `vulcanusItemDropRate`, `partyExperienceBonus`, `vitalityBonus`). The list
  is `BuffAttributes.h`; another name is an error. A `per` term on a hero
  attribute the client doesn't read is a warning: the term adds nothing.
- **The character stats.** They add up the power-ups of all the hero's buffs
  by attribute: physical damage `physicalBaseDmg` and `baseDamageBonus`,
  wizardry `wizardryBaseDmg` and `baseDamageBonus`, defense `defenseBase`,
  the stats `totalStrength`, `totalAgility`, `totalVitality`, `totalEnergy`
  and `totalLeadership`. A new buff with one of them counts without code; a
  debuff doesn't: a value only raises its attribute, and the format has no
  decrease yet, so the stats don't count a debuff's power-ups, and they are a
  warning (an item's tooltip still shows them). The
  Seal of Strength (13/96) counts this way; the original client had no check
  for it, so its +50 damage and +60 defense never showed. Elixirs of different
  stats add up too, where the old checks counted only the first of them;
  OpenMU lets a character have one elixir at a time, so nothing changes there.
  The Scroll of Battle adds `criticalDamageChance` the same way. The character
  window adds `greaterDamageBonus` to the shown damage range and `defenseFinal`
  to the shown defense, as OpenMU's Elf Soldier buff (number 3) raises them:
  `{"values": 45, "per": [{"attribute": "level", "divisor": 3}]}` and
  `{"values": 50, "per": [{"attribute": "level", "divisor": 5}]}`, so a hero of
  level 200 sees 111 more damage and 90 more defense. OpenMU adds the defense
  as a final value; the client adds it as `add`, like every value.
- **The Christmas buffs still in code.** The Santa NPC's event buffs
  (Blessing of Xmas, Strength of Santa, Defense of Santa) are still added by
  their own checks in `ZzzInfomation.cpp` and have no `powerUps` yet. A
  power-up given to one of them counts twice unless the same change removes
  those checks.
- **Numbers.** As Webzen's data gives them: a rate in percent
  (`experienceRate` 150 is 150 %), the auras' value per level of the hero
  (`{"values": 0, "per": [{"attribute": "level", "times": 10}]}`). Each is one
  number: a list (`[1800]`) is one value per item level, and a buff has none,
  so a list stops the game.
- **No duration.** The cash shop's scrolls and seals (Scroll of Quickness,
  Scroll of Defense, Seal of Wealth) have none: Webzen's server takes their
  time from the purchase, so only the time the server sends shows.
- **OpenMU.** OpenMU stores some of these another way or with other numbers:
  the Scroll of Defense and Jack O'Lantern Cry as final defense (+30 and +50),
  Jack O'Lantern Blessings for 32 minutes, Gladiator's Honor for 30, the cash
  shop's items for 30. The planned sync with the server compares them.

## Compared with the original client

- The original client read the buff names and descriptions from
  `BuffEffect_<lang>.bmd`, which existed only in English, Portuguese and
  Spanish. The Portuguese and Spanish files were written in Windows-1252 and
  read as UTF-8, so every accented letter showed wrong; the JSON is UTF-8.
- Descriptions were never formatted: "%%" showed as two percent signs.
- The buff strip's tiers, hidden effects and right-click cancels were written
  in code (`BuffStrip.cpp`).
- A right-click on the strengthened Infinity Arrow and Expansion of Wizardry
  (buffs 143, 138, 139) cancelled nothing: the code knew only the first tier's
  skill.
- The Elf Soldier's buff (number 3) was a level check in the character
  window, which stopped at level 180, so above it the window showed less than
  the server gives. She gives the buff up to level 220.
