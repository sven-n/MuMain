# MU Helper attack skills

Assign a primary attack and, optionally, additional attacks in the Helper settings.
With Combo disabled, an additional attack with neither Delay nor Condition enabled
joins the primary in a round-robin rotation. No artificial delay is required.
Clearing the additional slots keeps the primary-only behavior.

The rotation skips skills that are not learned, are on cooldown, lack mana or AG,
or do not meet their attribute or skill-use requirements. Selection does not start
a cooldown. The normal attack path still checks range, movement and casting state;
an attack that is busy or approaching its target keeps its turn. Other attempts
advance the rotation even when no cast was issued, so one unusable skill cannot
hold up the other slots.
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
