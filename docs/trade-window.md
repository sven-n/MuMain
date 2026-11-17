# Trade window: the change warning

The trade window warns when the trade partner swaps an item for a worse copy
of it, for example a +9 sword taken out and a +7 sword of the same kind put in.

## Which items are remembered

The window remembers items the partner offered, at most one copy of each item
kind:

- when the partner takes an item out of the trade window;
- when the trade is cancelled, all items they offered. This holds for every
  way of cancelling: the close button, Escape, walking away, or the partner
  cancelling.

These items are remembered (`CTrade::BackUpYourInven`):

- pets and riding items, the Jewels of Bless, Soul, Life and Chaos, wings and
  capes;
- jewel bundles (for example ten Jewels of Bless packed into one item);
- weapons and armour above +4;
- any item with an excellent option;
- for now, every item from the Cape of Lord (13,30) on in the item list,
  which takes in all potions (group 14) and scrolls (group 15). Issue #946
  tracks narrowing this down.

When an item of a kind that is already remembered comes in again, the better
copy is kept. The memory lasts across trades with the same partner. It is
cleared when a trade with another partner opens, or when the trade window
closes after you pressed the trade button, unless the trade was cancelled. A
trade request from someone else that you decline leaves the memory alone.

## When an item is marked

An offered item is shown in red when it is worse than the remembered copy of
its kind. Copies are compared (`CompareItem`) in this order, and the first
difference decides:

1. the level;
2. whether the item has its skill;
3. the excellent options, taken as one bit-flag number, not counted: a copy
   with one high-bit option beats a copy with two low-bit options;
4. the number of other options, then the additional option (the one a Jewel
   of Life raises): a copy without it, or with a lower value, is worse, so a
   +16 to +4 swap is marked;
5. the durability, so a copy with less durability is also marked.

The comparison never reads the Jewel of Harmony option, the 380 option, the
sockets or the ancient set. Copies that differ only there count as the same:
the worse one is not marked, and it clears the marks of its kind.

The mark is cleared when the partner also offers an item of that kind that is
the same as the remembered copy. Marks of other kinds stay. The marks are
worked out again each time the partner adds or takes out an item.

## The confirm button

While any offered item is marked, pressing the trade button first asks
whether the items are really the ones you want to trade. The trade goes on
only if you accept. With no item marked, the button never asks. The question
closes with the trade window, so it cannot press the button of a trade that
has already ended.

The original client asked again after you had pressed the trade button once,
even with nothing marked, because one flag stood both for the warning and for
forgetting the remembered items when the window closes. These are two flags
now.
