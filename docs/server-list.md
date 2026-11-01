# Server List

The server groups on the login screen (Valhalla, Helheim, ...), their channels,
the VIP types of the channels (Gold, Silver, ...), the texts that name a
channel and the colors of the channel buttons come from
`Data/Local/ServerList.json`. It replaced the encrypted
`Data/Local/ServerList.bmd` of the original client. The game reads it at start;
with fatal problems in it the game shows them and stops, as it does for broken
item data. All problems are also written to the log.

## How the login screen uses it

The connect server sends the IDs of its game servers and their load. The
client splits each ID into a group and a channel:

- group = server ID / 20
- channel = server ID % 20 + 1

So servers 0 to 19 are the channels 1 to 20 of group 0, servers 20 to 39 those
of group 1, and so on. On OpenMU the ID is the game server's *Server ID* in the
admin panel; to put a server into a second group, give it an ID from 20 to 39.

- **Groups:** one button per group, in the order of the list. A server whose
  group is not in the list is not shown at all.
- **Test server:** of the groups marked as test server, the login screen
  shows only the first, at its place in the list; the others are hidden.
- **Channels:** each channel has two settings: **PvP** on or off, and a **VIP
  type** or none. The channel's name is built from a template (see
  [Templates](#templates)). With the bundled file:

  | PvP | VIP type | Server list button          | Login window                  |
  |-----|----------|-----------------------------|-------------------------------|
  | on  | none     | `Valhalla-9 (PvP)`          | `[Valhalla-9 (PvP)]`          |
  | off | none     | `Valhalla-6 (Non-PvP)`      | `[Valhalla-6 (Non-PvP)]`      |
  | on  | Gold     | `Valhalla-5 (Gold PvP)`     | `[Valhalla-5 (Gold PvP)]`     |
  | off | Gold     | `Valhalla-1 (Gold Non-PvP)` | `[Valhalla-1 (Gold Non-PvP)]` |

- **Load:** the connect server sends each server's load in percent. A *full*
  server (100 to 127) shows the list's full text, `(Full)` with the bundled
  file, an *overloaded* one (128 and more) its overloaded text, `Overloaded`;
  each in a color, bold and italic of the list's (see [Load](#load)). The text
  sits centered on the load bar, whose segments under it are hidden while the
  others stay where every bar has them, and the button's top line keeps the rest
  of the button text, wherever the template puts `{load}`. A click on either
  says that the server is full.
- **Load bar:** below the button text, ten segments of 10 % each. A segment is
  lit as far as the load reaches into it: 35 % lights three and half of the
  fourth. A full or overloaded server has every segment lit in the full color,
  those under its load text hidden.
- **Button width:** all channel buttons are as wide as the longest name any of
  them can have, of every channel of every group shown (the 20 of each, whether
  the connect server sent them or not), with space on both sides; wider when a
  load text needs it to keep a segment beside it; never narrower than the theme's
  own button. A load change or another group never resizes them.
- **Button color:** set by the button color rules, by VIP type, PvP or both
  (see [Button colors](#button-colors)). With the bundled file Gold channels are
  orange, channels without a VIP type gray with PvP on and the theme's default
  color with PvP off.
- **PvP notice:** the warning about crowded PvP servers is shown for a group
  that has no channel with PvP off.
- **Description:** shown below the channels while the group is selected.

The PvP setting also changes two things in the client while you play on that
channel: with PvP off, the move command cannot take you to Vulcanus; with PvP
on, the PK field counts as a strife map.

The file only names and colors the channels. Whether PvP is really on is
decided by the server: on OpenMU, *PvP enabled* in the game server's settings
in the admin panel. Set both the same way. A VIP type only names and colors
channels in the client; OpenMU does not keep players without VIP out of a
server.

## Where a channel's name appears

| Place                     | Template        | Example (Valhalla-1, Gold, PvP off)                  |
|---------------------------|-----------------|------------------------------------------------------|
| Server list button        | `button`        | `Valhalla-1 (Gold Non-PvP)`                          |
| Login window title        | `login`         | `[Valhalla-1 (Gold Non-PvP)]`                        |
| Character info title      | `characterInfo` | `[Valhalla-1 (Gold Non-PvP)]`, fading with the class name |
| Screenshot message (chat) | `screenshot`    | `Screen(10_31-14_02)-0001.jpg: Screenshot Saved [Valhalla / MaxiDK]` |

The login window and the character info window name the channel picked on the
server list. The character info window is narrow (190 pixels in the legacy
theme), so a long name there can run past its edges.

## When a server drops the login screen

OpenMU's connect server closes a connection that sends nothing for a while
(its *Timeout* in the admin panel, one minute by default). An open server list
asks for a fresh list every 30 seconds, which keeps that connection up and the
load figures current. The connect server sends only so many lists over one
connection (*MaxServerListRequests*, 20 by default) and closes it on the next
request, so before that the list connects anew in the background, about every
ten minutes. When the connection drops anyway (a Timeout below 30
seconds, a restarted server), the login screen doesn't tell the player: the next
click that needs the server (a group, a channel, the credits, OK in the login
window) connects again first and then carries on. The login window does the
same when its game server went away, e.g. because it was restarted.

After the third wrong password the game server closes the connection on
purpose. OK on its message goes back to the server list instead of logging in
again over a new connection to the same server.

Only when the server can't be reached does the message "You are disconnected
from the server." appear, with **Reconnect** (back to the server list, which
shows the servers that are up) and **Exit**. The original client showed this
message with an OK button that closed the game.

## The file

```json
{
  "formatVersion": 2,
  "vipTypes": [
    {"id": "gold", "name": {"en": "Gold", "es": "Oro"}},
    {"id": "silver", "name": {"en": "Silver"}}
  ],
  "colors": [
    {"id": "goldRed", "name": "Gold red", "shades": {"legacy": "#FF5A5A", "modern": "#C0564E"}}
  ],
  "buttonColors": [
    {"vip": "gold", "pvp": false, "color": "goldRed"},
    {"vip": "gold", "color": "orange"},
    {"vip": true, "color": "silver"},
    {"vip": false, "pvp": true, "color": "gray"}
  ],
  "names": {
    "pvp": {"en": "PvP"},
    "nonPvp": {"en": "Non-PvP", "de": "Nicht-PvP", "es": "no PvP"},
    "button": {"en": "{name}-{channel} ({vip} {pvp}) {load}"},
    "login": {"en": "[{name}-{channel} ({vip} {pvp})]", "es": "[{name}-{channel} ({pvp} {vip})]"},
    "characterInfo": {"en": "[{name}-{channel} ({vip} {pvp})]"},
    "screenshot": {"en": "[{name} / {character}]"}
  },
  "load": {
    "bar": {"base": "default", "full": "goldRed", "empty": "default"},
    "full": {"text": {"en": "(Full)", "de": "(Voll)"}, "color": "goldRed", "bold": true},
    "overloaded": {"text": {"en": "Overloaded"}, "color": "goldRed", "bold": true, "italic": true}
  },
  "groups": [
    {
      "id": 0,
      "name": "Valhalla",
      "channels": {
        "1": {"vip": "gold", "pvp": false},
        "5": {"vip": "gold"},
        "6": {"pvp": false},
        "7": {"vip": "silver"}
      }
    },
    {
      "id": 1,
      "name": "Helheim",
      "description": "PvP everywhere"
    },
    {
      "id": 9,
      "name": "Test",
      "testServer": true
    }
  ]
}
```

| Field           | Meaning                                                                  |
|-----------------|--------------------------------------------------------------------------|
| `formatVersion` | `2`. A version 1 file (the four fixed channel types) is not read: the game stops and names the problem. |
| `vipTypes`      | Optional; the VIP types, in the order the editor lists them.              |
| `colorFolders`  | Optional; the folders the editor's Colors window sorts your colors in (each a `name`, its `folders` and its `colors`). The game reads and checks it but doesn't use it. |
| `colors`        | Optional; your own colors, each with a shade per theme, for the buttons and the load. |
| `buttonColors`  | Optional; the rules that color the channel buttons. Without a fitting rule a button has the theme's default color. |
| `names`         | Optional; the texts that name a channel (below). A text left out uses the built-in English one. |
| `load`          | Optional; the load texts and the load bar's colors (see [Load](#load)).   |
| `groups`        | The groups, in the order of their buttons.                               |

A group:

| Field         | Meaning                                                                  |
|---------------|--------------------------------------------------------------------------|
| `id`          | The group: server ID / 20, from 0 to 3276. Each group once.               |
| `name`        | The button text and `{name}` in the templates; up to 32 characters.       |
| `description` | Optional; up to 254 characters.                                           |
| `testServer`  | Optional; `true` marks a test server group: only the first is shown.      |
| `channels`    | Optional; the settings of a channel by its number (1 to 20). Channels not listed have PvP on and no VIP type. |

A channel:

| Field | Meaning                                                    |
|-------|------------------------------------------------------------|
| `pvp` | Optional; `false` turns PvP off. Default `true`.           |
| `vip` | Optional; the `id` of a VIP type. Default: no VIP type.    |

A VIP type:

| Field        | Meaning                                                                 |
|--------------|-------------------------------------------------------------------------|
| `id`         | What channels use in `vip`; each id once. Letters and digits, starting with a lowercase letter. |
| `name`       | `{vip}` in the templates, one text per language; up to 32 characters.   |

A name or description that is too long is cut (a warning in the log). Most
emoji count as two characters, as the client stores them on Windows.

### Texts in several languages

Each text in `names`, each VIP type's `name` and each load text holds one text per language
code: `en`, `de`, `es`, `id`, `ja`, `pl`, `pt`, `ru`, `tl`, `uk`, `zh-CN` and
`zh-TW`. The game uses the text of its language; a language that is missing
uses `en`, and a text without `en` uses the built-in English one. The bundled
file carries the translations of all twelve languages.

### Templates

| Placeholder   | Filled with                                                      | In            |
|---------------|------------------------------------------------------------------|---------------|
| `{name}`      | The group name                                                   | all           |
| `{channel}`   | The channel number                                               | all           |
| `{vip}`       | The VIP type's name in the game's language; empty without a VIP type | all       |
| `{pvp}`       | The `pvp` text with PvP on, the `nonPvp` text with PvP off       | all           |
| `{load}`      | The load text of a full or overloaded server (see [Load](#load)); empty while the server can be joined | `button` |
| `{character}` | The name of your character                                       | `screenshot`  |

Empty parts take their spaces with them:

- An empty placeholder is removed together with one space next to it:
  `({vip} {pvp})` becomes `(PvP)` without a VIP type, not `( PvP)`.
- Brackets, `()` or `[]`, that are left empty are removed together with one
  space next to them: with the `pvp` text set to nothing,
  `[{name}-{channel} ({vip} {pvp})]` becomes `[Valhalla-9]` on a channel with PvP
  on and no VIP type.
- Other text stays as written, commas too: `{vip}, {pvp}` becomes `, PvP`
  without a VIP type.

A placeholder the template does not know (`{vipName}`, or `{load}` outside
`button`) is a problem in the log and shown as written, so it is easy to spot.
Each language has its own template, so a language can put the words in its own
order (`({pvp} {vip})`).

### Button colors

Each rule of `buttonColors` says which channels it fits and their color:

| Field   | Meaning                                                                       |
|---------|-------------------------------------------------------------------------------|
| `vip`   | Optional; the `id` of a VIP type, `true` for any VIP type, `false` for no VIP type. Left out: any channel. |
| `pvp`   | Optional; `true` for PvP on, `false` for PvP off. Left out: either.            |
| `color` | `default`, `gray`, `orange`, `silver` (the theme colors), or the `id` of one of your `colors`. |

When several rules fit a channel, the most specific one colors it, so the
order of the rules does not matter:

- a rule that names a VIP type beats one for any VIP type or for none, which
  beats one that leaves `vip` out;
- with the same `vip`, a rule that gives `pvp` beats one that leaves it out.

So `{"vip": "gold", "pvp": false}` beats `{"vip": "gold"}`, and any rule with
`vip` beats one with only `pvp`. Two rules with the same `vip` and `pvp` are a
problem; the second is not used.

The theme colors are drawn by each theme in its own shade, hover too: the
tokens `server-button`, `server-button-gray`, `server-button-orange` and
`server-button-silver` in the theme's `tokens.ini`, which the editor's swatches
show as well. A color
of your own has an `id` (letters and digits, starting with a lowercase letter,
not one of the theme colors), a `name` the editor shows, and a shade
(`#RRGGBB`) per theme folder (`legacy`, `modern`, ...; capitals do not matter);
a theme it has no shade for draws the theme's default color. A button in your own color keeps it on
hover; the button's frame still changes.

### Load

`load` has the two load texts and the colors of the load bar:

| Field                  | Meaning                                                                 |
|------------------------|-------------------------------------------------------------------------|
| `bar.base`, `bar.full` | The colors the segments are mixed from and toward. `default`: the theme's own. |
| `bar.empty`            | The unlit part of the segments. `default`: the theme's track.           |
| `full`, `overloaded`   | `{load}` of a full server (load 100 to 127) and of an overloaded one (128 and more). |
| `….text`               | One text per language, up to 32 characters; without English, "(Full)" and "Overloaded". |
| `….color`              | `default`: the button's color. Else a theme color or one of your `colors`. |
| `….bold`, `….italic`   | Optional; `true` turns it on.                                            |

Each color is a theme color or the `id` of one of your `colors`, as in the
button color rules. The bar's `default` colors are tokens of the theme's
`tokens.ini`: `server-load-base`, `server-load-full` and `server-load-empty`.
The legacy theme mixes from `#00BD8B` to `#E0483E`, the modern theme from its
success to its danger color (`#8EB06B` to `#A22C1E`), the classic theme from
`#00FFFF` to `#FF0000`.

The segments: segment *k* (1 to 10) has the base color moved (*k* − 1) × 10 %
of the way to the full color, so the tenth is at 90 % and the full color
itself is left for a full server, whose segments are all in it. The colors are
mixed in OKLCH, a color model in which equal steps look equally far apart: the
hue goes the shorter way around the color wheel (green to red through yellow
and orange) and the brightness evenly from one color to the other, so no
segment looks brighter than both ends. Mixing the red, green and blue parts
evenly instead would make the middle segments a gray brown.

Italic text is drawn with the italic face of the font: the game bundles those
of Liberation Sans and DejaVu Sans. Where the font has none (Chinese and
Japanese, whose characters come from fonts without italic, or a font picked in
the options without one), the text is slanted instead.

The bundled file shows both load texts bold in a red of its own (`loadRed`),
"Overloaded" also italic, and keeps each theme's bar.

### Problems

| Problem                                              | Level   | What the game does                   |
|------------------------------------------------------|---------|--------------------------------------|
| `formatVersion` is not 2                             | fatal   | stops                                |
| A VIP type without an id, or an id used twice        | fatal   | stops                                |
| A VIP type without an English name                   | fatal   | stops                                |
| A channel's `vip` is not the id of a VIP type        | error   | the channel has no VIP type          |
| A color without an id, an id used twice or a theme color's | fatal | stops                         |
| A color's shade that is not `#RRGGBB`                | error   | the theme's default color there      |
| A rule's `vip` is not the id of a VIP type           | error   | the rule is not used                 |
| Two rules with the same `vip` and `pvp`              | error   | the second is not used               |
| A rule's `color` is not a theme color or one of `colors` | warning | the theme's default color        |
| `load`, its `bar` or a load text is not an object, `bold` or `italic` not `true` or `false` | fatal | stops |
| A load color is not a theme color or one of `colors` | warning | the default color                    |
| An unknown placeholder in a template                 | error   | shows it as written                  |
| An unknown language code                             | warning | the text is not used                 |
| A text or name that is too long                      | warning | cut                                  |

## Editing in MU Editor

**Windows → Server List** has two tabs.

**Groups** shows the groups in their order on the left and the selected group
on the right: its ID (with the server IDs it covers), name, description, the
test server mark, and a table of the 20 channels. Each row has a **PvP** box,
a **VIP** list (none or one of the VIP types), the button text the login screen
gives the channel and, when the connect server sent it, its load.

**Channel names** shows the VIP types on the left and on the right:

- the selected VIP type: its ID and its name. A new type gets
  an ID made from its name (New VIP type is `newVipType`); a changed ID goes to
  the channels with the type too;
- the texts: a **Language** list, then the PvP and Non-PvP texts, the four
  templates and the Full and Overloaded texts in that language. A field left
  empty uses the English text, which it shows greyed. A note lists the
  placeholders each template can use;
- the button color rules: one row per rule with a **VIP** list (any, any VIP
  type, none, or a VIP type), a **PvP** list (any, on, off) and a **Color**
  list. A rule that would repeat another's VIP type and PvP is refused;
- the load: **Base**, **Full** and **Unlit** color lists for the load bar, and
  for the full and the overloaded server a **Color** list with **Bold** and
  **Italic** boxes;
- **Edit colors...**: your own colors are the Server list root of the
  [Colors](item-data.md#colors) window. There a picked color has its ID, name
  and a shade for each theme, with **Apply to the color**; a change lists the
  rules and load parts that use the color first, and a color is removed only
  while none uses it. Folders sort the colors as for the item colors. The
  changes are steps of this window's undo history, and its **Save** writes
  them. The root also lists the themes' colors the list uses (`default`,
  `gray`, `orange`, `silver`, and the load bar's `server-load-base`,
  `server-load-full` and `server-load-empty`) with each theme's value, a theme
  built on another showing whose value it takes. They can't be removed;
  **Apply** writes a changed value into that theme's `tokens.ini` at once (a
  theme without a `tokens.ini` of its own gets one) and the game redraws with
  it (Undo doesn't take it back);
- a preview of every combination (PvP on and off, with no VIP type and with
  each VIP type) in all four places, with the selected group's name and the
  button's color in each theme (named on hover). The button's `{load}` is
  empty there, as for a server that can be joined. Below it, the button of
  channel 1 at 35 % load, full and overloaded, and its load bar in each theme
  with the load text centered on it in its color, the segments under it
  hidden, as on the login screen; bold and italic show only in game. The
  previews show every theme folder.

The Groups tab's channel table shows each channel's button text in its color.

For both tabs:

- The login screen shows each change right away, while the window is open, and
  so do the login window's and the character info's titles and the screenshot
  message of the channel the game joined.
- **Add**, **Remove**, **Move up** and **Move down** change the list of groups,
  of VIP types or of colors. Removing a VIP type that channels or button color
  rules use first asks, and lists the channels and the number of rules; the
  channels then have no VIP type, and the rules for the type go.
- When the connect server sent servers of groups the list does not have, the
  Groups tab lists them with their server IDs; **Add** there adds a group with
  that ID.
- **Save** writes `Data/Local/ServerList.json` in the folder the game runs
  from. It refuses a list with fatal problems (a group without a name, an ID
  used twice, a VIP type without an English name) and lists them. When reading
  the file itself left something out (changed by hand while the game ran): a
  fatal problem, or a rule, shade or channel VIP type the list could not use,
  or a color it replaced with the default one, the window has only what could
  be read: Save stays off then, so it cannot remove or replace the rest, until
  the file is fixed and **Revert** reads it again.
- **Revert** reads the file again.
- **Show offline servers in the game** (editor only): the login screen also
  lists every channel 1 to 20 of every group that the connect server did not
  send, at the load set next to it (**Their load**, 0 to 140 %), so their
  names, colors, load bars and Full / Overloaded texts can be checked without
  the servers running. A click on one only says that the editor shows it. The
  switch and the load are kept in `MuEditor/MuEditor.ini` (`[ServerList]`).
- **Undo** and **Redo** in the editor bar (or Ctrl+Z and Ctrl+Y in the window)
  work through the changes of the session. Each change is one step: a group,
  VIP type, color or rule added, removed or moved, a box, list or shade
  changed, and what was typed into a field until it is left. The bar's buttons act on the tool used
  last: after a click into the Server List window they undo server list
  changes, after a click into an item tool item edits.

## Compared to the original client

`ServerList.bmd` held the same groups, encrypted, with channel types for the
first 15 channels only and a left, right or center position for each button.
The original screen drew the center one, a test server, as a button of its own;
the login screen now lists all groups in one column in the list's order, so
only "test server" is kept, for the rule that only the first is shown. The
original loader read each group's description but never kept it, so the login
screen showed none; the JSON description is shown.

A channel type was one of four values, really two switches: bit 0 turned PvP
off, bit 1 made the channel gold. The login window and character info titles
were fixed texts by type (461, 460, 3130 and 3131: "[%s-%d Server]",
"[%s-%d(Non-PvP) Server]", "[%s-%d(Gold PvP) Server]", "[%s-%d(Gold) Server]").
Now the two switches are the PvP box and the VIP type, Gold is one VIP type of
any number, and every text is a template in the file. The original buttons
also ended in "Connect" for every server that could be joined; now only a full
or overloaded server says so.

The original called a load of 128 and more "Now preparing", and a click on such
a server did nothing; it is "Overloaded" now, and a click says that the server
is full, as for a full one. Its gauge was one bar as long as the load.
