# Server List

The server groups on the login screen (Valhalla, Helheim, ...) and the names
and colors of their channels come from `Data/Local/ServerList.json`. It
replaced the encrypted `Data/Local/ServerList.bmd` of the original client. The
game reads it at start; with fatal problems in it the game shows them and
stops, as it does for broken item data. All problems are also written to the log.

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
- **Channels:** each channel is shown as `<group name>-<channel>`, followed by
  its type:

  | Type      | Shown as               | Button  |
  |-----------|------------------------|---------|
  | `normal`  | `Valhalla-1`           | gray    |
  | `nonPvp`  | `Valhalla-1(Non-PVP)`  | default |
  | `goldPvp` | `Valhalla-1(Gold PVP)` | orange  |
  | `gold`    | `Valhalla-1(Gold)`     | orange  |

  The type only names and colors the channel; whether PvP is on is decided by
  the server.
- **PvP notice:** the warning about crowded PvP servers is shown for a group
  that has no `nonPvp` and no `gold` channel.
- **Description:** shown below the channels while the group is selected.

## The file

```json
{
  "formatVersion": 1,
  "groups": [
    {
      "id": 0,
      "name": "Valhalla",
      "channels": {"1": "gold", "5": "goldPvp", "6": "nonPvp"}
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

| Field         | Meaning                                                                  |
|---------------|--------------------------------------------------------------------------|
| `id`          | The group: server ID / 20, from 0 to 3276. Each group once.               |
| `name`        | The button text and the start of each channel name; up to 32 characters. |
| `description` | Optional; up to 254 characters.                                           |
| `testServer`  | Optional; `true` marks a test server group: only the first is shown.      |
| `channels`    | Optional; the type of a channel by its number (1 to 20). Channels not listed are `normal`. |

A name or description that is too long is cut (a warning in the log). Most
emoji count as two characters, as the client stores them on Windows.

## Editing in MU Editor

**Windows → Server List** shows the groups in their order on the left and the
selected group on the right: its ID (with the server IDs it covers), name,
description, the test server mark, and a type for each of the 20 channels with
the name the login screen gives it.

- The login screen shows each change right away, while the window is open.
- **Add**, **Remove**, **Move up** and **Move down** change the list.
- When the connect server sent servers of groups the list does not have, the
  window lists them with their server IDs; **Add** there adds a group with that
  ID. The channel table shows which channels the connect server sent, with
  their load.
- **Save** writes `Data/Local/ServerList.json` in the folder the game runs
  from. It refuses a list with fatal problems (a group without a name, an ID
  used twice) and lists them. When the file itself had fatal problems as it
  was read
  (changed by hand while the game ran), the window has only the groups that
  could be read: Save stays off then, so it cannot remove the others, until
  the file is fixed and **Revert** reads it again.
- **Revert** reads the file again.
- **Undo** and **Redo** in the editor bar (or Ctrl+Z and Ctrl+Y in the window)
  work through the changes of the session. Each change is one step: a group
  added, removed or moved, a box or channel type changed, and what was typed
  into a field until it is left. The bar's buttons act on the tool used last:
  after a click into the Server List window they undo server list changes,
  after a click into an item tool item edits.

## Compared to the original client

`ServerList.bmd` held the same groups, encrypted, with channel types for the
first 15 channels only and a left, right or center position for each button.
The original screen drew the center one, a test server, as a button of its own;
the login screen now lists all groups in one column in the list's order, so
only "test server" is kept, for the rule that only the first is shown. The
original loader read each group's description but never kept it, so the login
screen showed none; the JSON description is shown.
