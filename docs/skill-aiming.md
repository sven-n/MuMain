# Skill Aiming

How a held right button aims a skill at the cursor.

## Direction skills beyond their range

Some skills only take a *direction* from the cursor: they fire toward it or around the caster and hit whatever is in the way, rather than a chosen creature or spot. When the cursor is beyond such a skill's range, the aim point is pulled back toward the hero along the same line, so the skill fires in that direction instead of doing nothing.

- **Range**: the skill's distance from the skill data, plus two tiles while riding a Dark Horse.
- **Where the aim ends up**: one tile inside the range, on the line from the hero to the cursor. The attack code tests the *centre* of the target tile against the range, so the extra tile keeps that centre inside it whatever the direction.
- **What counts as the cursor**: the ground under it, or a character under it that can't be attacked (a party member, an NPC). A monster or other attackable character under the cursor stays the target. Beyond the range, only the Dark Wizard skills, Twisting Slash and Chaotic Diseier walk the hero into range; the other skills listed here do nothing until the monster is in range (#1100).
- **Auto-attack**: it repeats a skill only while an attackable character is under the cursor. A party member or NPC under the cursor stops it, near or far.
- **Which skills**:
  - Dark Wizard / Magic Gladiator: Storm, Evil Spirit, Hellfire, Aqua Beam (Flash), Inferno. A Dark Wizard's Hellfire and Inferno never check their range, so for them the pull only matters to the Magic Gladiator.
  - Magic Gladiator: Flame Strike, Gigantic Storm.
  - Fairy Elf: Triple Shot, Penetration, Multi-Shot and the crossbow blast.
  - Dark Knight / Magic Gladiator: Fire Slash, Power Slash, Twisting Slash, Rageful Blow.
  - Dark Lord: Chaotic Diseier, Fire Scream, Earthshake, Electric Spike.
  - Summoner: Lightning Shock.
  - Including their strengthened and mastery versions.

Skills cast on a chosen creature or spot (Decay, Ice Storm, Nova, buffs and heals) keep their range: beyond it the hero walks closer first.

**Original S6 client:** holding a direction skill on the ground beyond its range did nothing, or walked the hero to the cursor and left them standing there with the button held. Party members and NPCs under the cursor stopped the skill the same way. Other MU clients keep firing, which is what this matches.
