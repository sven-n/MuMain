# MU Helper

## Attack skills

Assign a primary attack and, optionally, additional attacks in the Helper settings.
With Combo disabled, an additional attack with neither Delay nor Condition enabled
joins the primary in a round-robin rotation. No artificial delay is required.
Clearing the additional slots keeps the primary-only behavior.

The rotation skips skills that are not learned, are on cooldown, or do not
meet their attribute or skill-use requirements. Selection does not start a
cooldown. Mana and AG remain checked by the normal cast path, including automatic
mana-potion use when the selected skill is unaffordable. Busy attacks, movement,
and target reacquisition retain the selected slot. The existing animation gate
is reported as busy for its full duration. When the character is ready, four
consecutive no-action attempts yield to the next slot
so a persistently unusable action cannot stall the rotation. Cast and refused
attempts advance it.
The existing basic-attack fallback remains available when skills cannot be used.

Additional attacks with Delay or Condition enabled keep their existing priority
over the rotation while they can be cast; an unlearned or cooling-down skill
leaves the turn to the rotation. When both are enabled, either a due timer or a
satisfied monster condition can select the skill, as before. A Delay is a
deadline: it stays due until a cast, a refusal, or four attempts that produce no
action consume it, then waits a full interval. A condition skill that is refused
or keeps producing no action yields to the rotation for one second. Monster counts include only live monsters within hunting range, and a
monster counts as attacking only when its attack targets the character. Combo
mode keeps its existing sequence and does not use the automatic rotation.

This also applies to saved settings: previously configured additional attacks
without Delay or Condition now rotate when Combo is off. The saved packet format
is unchanged, and inactive monster-condition subfields do not enable a condition.
These are the online client's rules; server-side offline leveling has its own
skill-selection implementation.

## Timing and order

Helper seconds follow the clock, so delays, buff intervals, and the return delay
keep real time when frames are slow. Each tick drinks a healing potion when
health is below the threshold before anything else. A buff, heal, or Drain Life
that cannot be cast moves on instead of holding up the actions after it; only a
cast still under way does.

## Targets, movement, and loot

Starting the Helper picks up monsters already in view. With self-defense on,
only a monster attacking the character replaces the current target; monsters
that leave hunting range are dropped unless long-range counterattack keeps an
attacker. Logging out, returning to character selection, a warp, or a reconnect stops the
Helper and forgets its targets and drops; a reconnect resumes it afterwards. A
monster that cannot be reached, behind a wall or with no path, is skipped for two
seconds so reachable ones are fought first.
Range changes apply immediately while the Helper runs.

Automated movement finishes its current path before planning another one, even
when low frame rates make a walk last longer than a Helper tick. Once the return
delay expires, returning to the starting position takes priority over collecting
more drops, but a drop the Helper is already walking to is picked up first. A
scheduled buff refresh waits for a cast still under way and is not skipped. A drop with no available path waits one second before another pickup
attempt; combat and other reachable loot continue during that delay. If the
starting position cannot be reached, combat continues and the Helper retries
the return on later ticks. A requested pickup stays tracked until the server
removes the drop; if the server refuses it, for example because the inventory is
full, the Helper tries again after three and then six seconds, and gives the drop up
after a third refused request. Monsters that leave hunting range stay tracked but
are not chased or counted until they are back in range.
