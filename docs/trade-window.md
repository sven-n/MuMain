# Trade window: the change warning

The trade window warns when the trade partner swaps an item for a worse copy
of it, for example a +9 sword taken out and a +7 sword of the same kind put in.

## Which items are remembered

The window remembers items the partner offered, at most one copy of each item
kind:

- when the partner takes an item out of the trade window;
- when a trade with the partner is cancelled, all items they offered.

Only valuable items are remembered: pets and riding items, jewels, wings and
capes, socketed (combined) jewels, and items above +4 or with excellent
options.

When an item of a kind that is already remembered comes in again, the better
copy is kept. The memory lasts across trades with the same partner. It is
cleared when a trade with another partner opens, or when a trade window that
showed the warning closes.

## When an item is marked

An offered item is shown in red when it is worse than the remembered copy of
its kind. Copies are compared by level first; at the same level, by the skill,
then the excellent options, then the other options.

The mark is cleared when the partner also offers an item of that kind that is
the same as the remembered copy. Marks of other kinds stay. The marks are
worked out again each time the partner adds or takes out an item.

## The confirm button

While any offered item is marked, pressing the trade button first asks
whether the items are really the ones you want to trade. The trade goes on
only if you accept.
