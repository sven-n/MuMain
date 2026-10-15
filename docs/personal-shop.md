# Personal shop prices in items and account coins

A personal shop sells its items for zen. When the server allows it (the
OpenMU plugin *Player shop currencies*), the price of each item can also be
set in:

- items which the server configured as price items, by default the Jewels of
  Bless, Soul, Chaos, Creation and Life;
- the account coins WCoin (C), WCoin (P) and Goblin Points, if the server
  allows them.

The server tells the client which price items and coins it allows when the
character enters the game. Without that, the shop works with zen only, as in
the original client.

## Setting a price

Right-click an item in the own shop (or drop one into it) to enter its price.
When the server allows other prices, the price dialog names the currency of the
price in its own line ("Currency: Jewel of Bless"), and its "Currency" button
opens a menu of the allowed currencies:

- the common ones first: zen, the account coins, and the jewels (Bless, Soul,
  Chaos, Life, Creation, Guardian, Harmony);
- the other items the server allows are listed in a second menu, behind an
  "Other items" button, in the order the server sent them.

A long menu takes two columns. Choosing a currency, or closing the menu, returns
to the price dialog and keeps the typed amount. Zen prices below the item's value
still ask for a confirmation; the other currencies don't.

Which items can be prices is the server's choice: OpenMU offers the five jewels
Bless, Soul, Chaos, Creation and Life by default, and its administrators can add
any other item.

The server checks every price: a price item or coin that isn't allowed, or an
amount above the server's maximum, is refused with a message.

## Buying

The tooltip of an item in a shop shows its price with the currency, e.g.
`Selling Price: 12 Jewel of Bless`. The "short of zen" warning is only shown
for zen prices.

Item prices are paid with the price items of the inventory: a single item
counts as one piece, and the jewel bundles of the jewel mix as their number of
pieces. The server doesn't split bundles and gives no change, so the buyer
needs items which match the price exactly. When the items or coins are not
enough, the server says which amount is missing.
