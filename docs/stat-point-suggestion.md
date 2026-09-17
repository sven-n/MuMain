# Stat Point Suggestion

The character info window (default key `C`) can propose how to spend the
free level-up points and apply the whole split in one click.

---

## Using it

The bar between the last stat row and the button strip at the bottom has two
buttons:

- **Suggest / preset name** - cycles through the builds available for the
  character's class, starting at "off": `Suggest -> PvE -> PvP -> ... -> Suggest`.
  While a preset is selected, every stat row shows a green `+N` badge with the
  number of points the preset would put there.
- **Apply** - spends all free level-up points according to the badges. It is
  only shown while a preset is selected.

The badges disappear when there are no free points left. Switching character
resets the selection - a preset belongs to a class, not to an account.

## The builds

Each preset is a set of **weights**, one per stat, and the free points are
split proportionally between them. A weight is a ratio, not a number of
points: for a Dark Knight PvE (`4 STR / 1 AGI / 1 VIT`), every 6 points become
4 strength, 1 agility and 1 vitality, whatever the size of the pool.

| Class | Presets (STR / AGI / VIT / ENE / CMD) |
|---|---|
| Dark Wizard | PvE `0/1/1/4`, PvP `0/2/2/3`, Full Energy `0/0/0/1` |
| Dark Knight | PvE `4/1/1/0`, PvP `3/2/2/0`, Full Strength `1/0/0/0` |
| Fairy Elf | PvE `1/4/0/1`, PvP `1/3/2/1`, Support `0/1/1/4` |
| Magic Gladiator | PvE `3/1/1/2`, PvP `2/2/2/2` |
| Dark Lord | PvE `2/1/1/1/2`, PvP `2/2/2/1/2`, Full Command `0/0/0/0/1` |
| Summoner | PvE `0/1/1/4`, PvP `0/2/2/3`, Full Energy `0/0/0/1` |
| Rage Fighter | PvE `3/1/2/0`, PvP `2/2/3/0` |

The split uses the largest-remainder method: each stat first gets its floored
share, then the points lost to rounding go to the stats with the biggest
remainder. The distribution therefore always adds up to exactly the number of
free points - nothing is left behind.

Only the Dark Lord has a Command row, so its presets are the only ones with a
fifth weight.

The weights live in the preset tables at the top of
`src/source/UI/NewUI/Character/NewUICharacterInfoWindow.cpp`; changing a build
means changing one row there.

## Protocol note

The original S6 client sends one `0xF3, 0x06` packet **per point**, so
spending a few hundred points means a few hundred round trips, each one
triggering a full stat recalculation on the client.

Apply instead sends one `0xF3, 0xE0` packet per stat with the amount as a
2-byte field, and the server adds all of them at once. The sub code `0xE0` is
not used by the original client; the server handler
(`CharacterStatIncreaseMultiplePacketHandlerPlugIn` in OpenMU) caps the
requested amount at the level-up points the character actually has, so a
stale client cannot overspend. Game masters, who may raise stats without
spending points, are exempt from that cap.

The single `+` button next to each stat row still uses the original one-point
packet.
