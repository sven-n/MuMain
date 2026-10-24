# MU Helper attack skills

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
over the rotation. When both are enabled, either a due timer or a satisfied monster
condition can select the skill, as before. Combo mode keeps its existing sequence
and does not use the automatic rotation.

This also applies to saved settings: previously configured additional attacks
without Delay or Condition now rotate when Combo is off. The saved packet format
is unchanged, and inactive monster-condition subfields do not enable a condition.
These are the online client's rules; server-side offline leveling has its own
skill-selection implementation.
