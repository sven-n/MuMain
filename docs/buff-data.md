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
| `item` | The item that gives the buff (`group`, `number`); left out for none. When the server sends no duration, the countdown takes the item's duration (its add option's time). Moving the item in the item tools does not move this link yet (the Buffs tool will); the buff then names a missing item, an error. |
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
