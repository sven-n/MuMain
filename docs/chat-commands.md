# Chat Commands Window

The chat commands a server offers - `/post`, `/item`, `/move`, whatever the
server operator activated - are usually invisible: you have to know that they
exist, know how they are spelled, and know which values they take. `/list` only
prints them as blue chat messages, which scroll away.

The chat commands window lists them instead, fills in their values, and sends
them for you.

> Requires a server which answers the command list request (OpenMU 0.9.11 and
> newer). Against an older server the window stays empty and says so.

---

## Opening it

- Press **J**, or
- press **D** for the command window and click **Special**.

Both routes lead to the same window. **Escape** goes back one page and closes
the window on the first page.

## The command list

The first page lists every command you are allowed to use - a game master sees
more than a normal player. A command is shown by its name only, because that is
all that fits:

```
/post ...
/move ...
/resetinfo
* /item ...
```

- Three dots mean the command takes values. Clicking it opens the value page.
- No dots means there is nothing to fill in. Clicking it sends the command
  right away and closes the window.
- A leading `*` marks a favourite. Favourites are listed first.

**Templates** opens the list of commands you saved with their values.

## Entering values

The value page shows what the command does, followed by one box per value.
A value marked with `*` is required - the command isn't sent without it.

- Values which only accept a few options (yes/no, a class, a level) are
  **clicked through**: every click steps to the next option, and one step past
  the last one clears it again. Clearing is how an optional value is left out.
- Everything else is **typed**. Values which are numbers only accept digits.
- **Enter** finishes a box, **Escape** leaves it.

**Execute** sends the command. It is also added to your chat history, so the
arrow keys in the chat box repeat it without opening the window again.

## Favourites and templates

Both are stored per installation in `config.ini`, not per character - the
commands you reach for don't change when your character does.

- **Add to favourites** on the value page moves the command to the top of the
  list.
- **Save as template** stores the command together with the values which are
  currently entered. It shows up under **Templates** as the command line it
  produces; clicking it sends that line, and the `x` at its right removes it.

The `[ChatCommands]` section of `config.ini` holds both lists. A template is
stored as `label|command|value|value|...`, so a value can't contain a `|` - one
that does is stored with a space in its place.
