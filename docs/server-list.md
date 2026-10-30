# Server List

The server groups on the login screen (Valhalla, Helheim, ...), their channels,
the VIP types of the channels (Gold, Silver, ...) and the texts that name a
channel come from `Data/Local/ServerList.json`. It replaced the encrypted
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

  | PvP | VIP type | Server list button                   | Login window                  |
  |-----|----------|--------------------------------------|-------------------------------|
  | on  | none     | `Valhalla-9 (PvP) Connect`           | `[Valhalla-9 (PvP)]`          |
  | off | none     | `Valhalla-6 (Non-PvP) Connect`       | `[Valhalla-6 (Non-PvP)]`      |
  | on  | Gold     | `Valhalla-5 (Gold PvP) Connect`      | `[Valhalla-5 (Gold PvP)]`     |
  | off | Gold     | `Valhalla-1 (Gold Non-PvP) Connect`  | `[Valhalla-1 (Gold Non-PvP)]` |

- **Button color:** a channel with a VIP type gets that type's theme color; one
  without gets gray with PvP on and the theme's default color with PvP off.
- **PvP notice:** the warning about crowded PvP servers is shown for a group
  that has no channel with PvP off.
- **Description:** shown below the channels while the group is selected.

The PvP setting also changes two things in the client while you play on that
channel: with PvP off, the move command cannot take you to Vulcanus; with PvP
on, the PK field counts as a strife map.

The file only names and colors the channels. Whether PvP is really on is
decided by the server: on OpenMU, *PvP enabled* in the game server's settings
in the admin panel. Set both the same way. A VIP type is a name and a color in
the client; OpenMU does not keep players without VIP out of a server.

## Where a channel's name appears

| Place                     | Template        | Example (Valhalla-1, Gold, PvP off)                  |
|---------------------------|-----------------|------------------------------------------------------|
| Server list button        | `button`        | `Valhalla-1 (Gold Non-PvP) Connect`                  |
| Login window title        | `login`         | `[Valhalla-1 (Gold Non-PvP)]`                        |
| Character info title      | `characterInfo` | `[Valhalla-1 (Gold Non-PvP)]`, fading with the class name |
| Screenshot message (chat) | `screenshot`    | `Screen(10_31-14_02)-0001.jpg: Screenshot Saved [Valhalla / MaxiDK]` |

The login window and the character info window name the channel picked on the
server list. The character info window is narrow (190 pixels in the legacy
theme), so a long name there can run past its edges.

## The file

```json
{
  "formatVersion": 2,
  "vipTypes": [
    {"id": "gold", "name": {"en": "Gold", "es": "Oro"}, "themeColor": "orange"},
    {"id": "silver", "name": {"en": "Silver"}, "themeColor": "silver"}
  ],
  "names": {
    "pvp": {"en": "PvP"},
    "nonPvp": {"en": "Non-PvP", "de": "Nicht-PvP", "es": "no PvP"},
    "button": {"en": "{name}-{channel} ({vip} {pvp}) {load}"},
    "login": {"en": "[{name}-{channel} ({vip} {pvp})]", "es": "[{name}-{channel} ({pvp} {vip})]"},
    "characterInfo": {"en": "[{name}-{channel} ({vip} {pvp})]"},
    "screenshot": {"en": "[{name} / {character}]"}
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
| `names`         | Optional; the texts that name a channel (below). A text left out uses the built-in English one. |
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
| `themeColor` | The button color: `default`, `gray`, `orange` or `silver`. Each theme (legacy, modern) draws its own shade. |

A name or description that is too long is cut (a warning in the log). Most
emoji count as two characters, as the client stores them on Windows.

### Texts in several languages

Each text in `names` and each VIP type's `name` holds one text per language
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
| `{load}`      | Connect, (Full) or Now preparing, by the server's load (game texts) | `button`   |
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

### Problems

| Problem                                              | Level   | What the game does                   |
|------------------------------------------------------|---------|--------------------------------------|
| `formatVersion` is not 2                             | fatal   | stops                                |
| A VIP type without an id, or an id used twice        | fatal   | stops                                |
| A channel's `vip` is not the id of a VIP type        | error   | the channel has no VIP type          |
| An unknown `themeColor`                              | warning | the theme's default color            |
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

**Channel names** shows the VIP types on the left, with their color, and on the
right:

- the selected VIP type: its id, its name and its theme color;
- the texts: a **Language** list, then the PvP and Non-PvP texts and the four
  templates in that language. A field left empty uses the English text, which
  it shows greyed. A note lists the placeholders each template can use;
- a preview of every combination (PvP on and off, with no VIP type and with
  each VIP type) in all four places, with the selected group's name.

For both tabs:

- The login screen shows each change right away, while the window is open.
- **Add**, **Remove**, **Move up** and **Move down** change the list of groups,
  or of VIP types. Removing a VIP type that channels use first asks, and lists
  the channels; they then have no VIP type.
- When the connect server sent servers of groups the list does not have, the
  Groups tab lists them with their server IDs; **Add** there adds a group with
  that ID.
- **Save** writes `Data/Local/ServerList.json` in the folder the game runs
  from. It refuses a list with fatal problems (a group without a name, an ID
  used twice, a VIP type without an id) and lists them. When the file itself
  had fatal problems as it was read (changed by hand while the game ran), the
  window has only what could be read: Save stays off then, so it cannot remove
  the rest, until the file is fixed and **Revert** reads it again.
- **Revert** reads the file again.
- **Undo** and **Redo** in the editor bar (or Ctrl+Z and Ctrl+Y in the window)
  work through the changes of the session. Each change is one step: a group or
  VIP type added, removed or moved, a box, list or color changed, and what was
  typed into a field until it is left. The bar's buttons act on the tool used
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
any number, and every text is a template in the file.

## Plan (removed when everything has landed)

This section is the working plan for sven-n/MuMain#895. It goes once the work
is done; the sections above then describe what was built.

### Decisions (2026-10-31)

1. Everything goes into #895: the login title fix, then this feature, with this
   document as its spec.
2. A channel has a PvP box and a VIP type, not one of four fixed types.
3. The texts live in `ServerList.json`, one text per language, English as
   fallback, like the texts of `Tooltip.json`.
4. Templates use placeholders; empty parts drop with their spaces and brackets.
   Commas are not handled; an optional form like `{vip, }` can be added later
   without breaking these templates.
5. One template per place: `button`, `login`, `characterInfo`, `screenshot`.
6. A VIP type has a name per language and a theme color from a fixed list.
7. A version 1 file is not read.

### Decided while writing the plan (to confirm)

- **"Server" goes in every language**, not only in English: "Servidor",
  "Serwer", "サーバー", "Сервер" and the others are left out of the templates too.
- **Word order per language** follows today's titles: Spanish, Indonesian and
  Portuguese put PvP first (`(PvP Oro)`), the others the VIP type first.
- **The screenshot message stays as it is** by default: `[{name} / {character}]`.
- **Channels without a VIP type** are gray with PvP on and the default color
  with PvP off, as `normal` and `nonPvp` are today.
- **The theme colors** are today's `default`, `gray` and `orange`, plus a new
  `silver` in both themes.
- **zh-CN** has no Gold texts yet, so its Gold name falls back to English until
  it is translated.

### Bundled texts

| Language | `pvp` | `nonPvp`  | Gold       | Order          |
|----------|-------|-----------|------------|----------------|
| en, tl   | PvP   | Non-PvP   | Gold       | `({vip} {pvp})` |
| de       | PvP   | Nicht-PvP | Gold       | `({vip} {pvp})` |
| es       | PvP   | no PvP    | Oro        | `({pvp} {vip})` |
| id       | PvP   | Non-PvP   | Emas       | `({pvp} {vip})` |
| ja       | PvP   | Non-PVP   | ゴールド   | `({vip} {pvp})` |
| pl       | PvP   | nie PvP   | Gold       | `({vip} {pvp})` |
| pt       | PvP   | não PvP   | Ouro       | `({pvp} {vip})` |
| ru       | PvP   | не PvP    | Золотой    | `({vip} {pvp})` |
| uk       | PvP   | не PvP    | Золотий    | `({vip} {pvp})` |
| zh-CN    | PvP   | 非 PvP    | (English)  | `({vip} {pvp})` |
| zh-TW    | PvP   | 非PvP     | 黃金       | `({vip}{pvp})`  |

### Work

1. **Data:** the channel settings, VIP types and texts in
   `ServerListGroup.h`; reading and writing version 2 in `ServerListJson.cpp`,
   with the problems above; version 1 refused.
2. **Name builder:** one function that fills a template for a channel, with the
   empty-part rules, used by every place; its own tests.
3. **Game:** `ServerListManager` keeps PvP and the VIP type per channel; the
   server list button, login window, character info window and screenshot
   message use their templates; button colors from the VIP type; the PvP
   notice and the move and strife map checks read the PvP box. The four old
   title texts (`Game.*.resx`) and `GetChannelSuffix` go.
4. **Themes:** the `silver` button color in the legacy and modern themes.
5. **Bundled file:** `Data/Local/ServerList.json` as version 2, with the texts
   above.
6. **Editor:** the Groups tab's channel table gets the PvP box and VIP list;
   the new Channel names tab; undo steps; Save checks.
7. **Tests** for each step; this document; then this section is removed and
   #895's title and description describe the whole change.
