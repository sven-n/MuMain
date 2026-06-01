# Equipping from the inventory

How items move from the bag onto the character. Covers right-click and
drag-drop equipping and how occupied slots are handled.

## Right-click to equip

Right-clicking an equippable item in the inventory equips it immediately,
choosing the slot for you:

- **One-handed weapon** — goes to the main hand. If the main hand is taken
  but the off-hand is free (dual-wield classes), it fills the off-hand
  instead. If both hands are full, it replaces the main-hand weapon.
- **Shield / off-hand** — goes to the off-hand, replacing whatever is there.
- **Ring** — fills a free ring slot; if both are full, replaces the first.
- **Two-handed weapon** — needs both hands, so it unequips **both** the
  main-hand weapon and the off-hand/shield, then equips.
- Equipping a shield (or any off-hand item) while a two-handed weapon is
  held unequips the two-handed weapon.

Anything it replaces is moved back into the inventory.

## Drag-drop to equip

Dragging an item onto an equipment slot equips it into **that exact slot**.
Dropping onto an already-occupied slot now **replaces** the occupant
(previously it did nothing): the occupant returns to the bag and the dropped
item is equipped. The two-handed rules above apply here too.

## When the bag is full

Replacing an item only works if the inventory has room for everything being
taken off. The whole action is all-or-nothing: if a two-handed weapon would
displace two items but only one fits, **nothing** is unequipped and a "not
enough space" message is shown. The item you were equipping is never lost.

## Why it works this way

The server only moves an item onto an **empty** equipment slot, one move at a
time, and confirms each move before the next. A direct slot-to-slot swap is
not possible, so the client unequips the conflicting item(s) to the bag first
and equips the new item once every required slot is free. In the original S6
client an occupied slot simply rejected the equip; here it resolves the
conflict for you.
