# Zen in the vault

The vault's Deposit and Withdraw buttons open a dialog to enter an amount of
zen. Its Max button fills in the most that a single move can carry right now:
the zen on hand, limited by the room left at the target. The amount is read
when Max is clicked, so it follows zen that arrives while the dialog is open.

If nothing can move, the dialog doesn't open and a message says why: "You are
short of Zen" when the source is empty, "Cannot store in vault" when the vault
is full, and "Exceeded maximum amount of Zen you can possess" when the
inventory is full.

## Limits

The client checks two limits before it sends a move to the server:

- **Vault: 2,100,000,000 zen.** A deposit that would take the vault above it
  is refused with "Cannot store in vault".
- **Inventory: 2,000,000,000 zen**, the same limit the NPC shop uses. A
  withdrawal that would take the inventory above it is refused with "Exceeded
  maximum amount of Zen you can possess". The original client dropped it
  without a message.

The server has its own limits, `MaximumVaultMoney` and `MaximumInventoryMoney`
in OpenMU's game configuration (both 2,147,483,647 by default), and refuses
any move above them. The client limits sit below those defaults, so a move the
client allows also passes the server. On a server configured with lower limits,
the server's limits win: the client may offer an amount that the server then
refuses.

The vault limit is a round number below the server default, so a full vault is
easy to recognize.

The original client accepted 8 digits (up to 99,999,999 zen per move) and
had no vault limit of its own. The dialog now accepts up to 10 digits, enough
for the vault limit.
