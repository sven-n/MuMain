# Discord

While the game runs, your Discord profile can show what you are doing in it:

```
MU Online
Lv 380 / ML 120 Blade Knight
In Blood Castle 5 · Party 3/5
00:42 elapsed
```

The client talks to the Discord app running on the same computer. Nothing is
sent anywhere else, and the game never waits on Discord: when Discord isn't
running, nothing happens, and when it is started later, the presence appears
within a few seconds.

> Requires the Discord desktop app and a server that set up a Discord
> application (see [For server operators](#for-server-operators)). Without an
> application id - from the server or in `config.ini` - the presence stays off.

Servers with a Discord integration, like OpenMU, offer more: a button to
[join their Discord](#joining-the-servers-discord), a dialog to
[link your account](#linking-your-account), and
[messages from Discord](#messages-from-discord) shown as such.

---

## What is shown

| Where you are         | First line                         | Second line                        |
|-----------------------|------------------------------------|------------------------------------|
| Login, server list    | Logging in                         | -                                  |
| Character selection   | Selecting a character              | -                                  |
| In the world          | `In game`, or with **On**: `Lv <level> [/ ML <master level>] <class>` | `<map>`, plus `· Party <n>/5` in a party |
| In an event           | (as in the world)                  | `In <event>`, e.g. `In Blood Castle 5`, `In Chaos Castle 3`, `In Illusion Temple 2`, `In Devil Square`, `In Castle Siege` |

The timer counts from the moment you entered the current screen - in the
world, from entering it, not from the last map change. The text follows the
game's language.

Changes - a map change, a level-up, someone joining the party - reach Discord
within a few seconds. Discord accepts only a few updates per 20 seconds, so
quick changes in a row are combined into the newest one.

## Turning it off

The options window has a **Discord** setting:

- **Hide details** (the default) - your character's class and level are left
  out; the first line only says *In game*. The location and the party stay.
- **On** - everything in the table above, including the level and class of
  your character.
- **Off** - no presence at all; the client does not even connect to Discord.

Your character's name is never shown, in any setting.

The setting applies immediately and is stored in `config.ini`.

## Joining the server's Discord

When the server has a Discord, the options window shows a button next to the
Discord setting: **Join** opens the server's invite in the Discord app, or in
the browser when Discord isn't installed. On a server with a Discord
integration the button says **Discord...** and opens the
[account dialog](#linking-your-account), which has the **Join** button.

Only real Discord invite links are opened (`https://discord.gg/...` or
`https://discord.com/invite/...`, also with a query like `?event=...`); with
anything else the button is not shown, and the client log says why.

## Linking your account

Linking your game account to your Discord user lets you write into your guild
chat from Discord, as your character. On a server with a Discord integration,
**Discord...** in the options window opens a dialog that shows whether your
account is linked, and which of your chats are mirrored to Discord.

1. Click **Link**. The dialog shows a one-time code like `ABCD-EFGH` and how
   long it is valid. The code is already in the clipboard; **Copy** copies it
   again.
2. In Discord, enter `/link ABCD-EFGH` (paste the code).

**Unlink** removes the link again. The chat command `/discord` does the same
without the dialog: `/discord link`, `/discord unlink`.

### Mirrored guild chat

When the chat of your guild is mirrored to a Discord channel, the guild window
says *Guild chat mirrored to Discord* under the guild name; the alliance tab
does the same for the alliance chat. What you write there can be read in
Discord. A guild master whose chat isn't mirrored is told how to do it: with
`/guildchat` in Discord.

## Messages from Discord

> Requires a server with a Discord chat bridge, like OpenMU's (its guild and
> alliance chat can be bound to Discord channels).

A message written in a Discord channel that the server bridges into the game
appears in the chat of its scope - guild, alliance or world - with that chat's
colours. Its sender is shown as `[Discord] <name>`, with a name that isn't
limited to a character's length. With OpenMU, the name is the character the
Discord user is linked to.

- Discord users are not characters: right-clicking their line does not start a
  whisper, and they are not added to your whisper list.
- **F2** switches the chat log between all messages and whispers. Once a
  message from Discord has arrived, it also offers a view with only the
  messages from Discord: all → whispers → Discord → all.

## For server operators

Each server uses its own Discord application, so the name your players see
above the presence ("MU Online", "MyMU", ...) and its pictures are yours to
choose. The client has none built in.

1. Create an application in the
   [Discord developer portal](https://discord.com/developers/applications).
   Its name is the title shown above the presence.
2. Optional: under *Rich Presence → Art Assets*, upload a large image (e.g. your
   server logo) and, if you want, a small one. Note their keys.
3. Optional: create an invite link to your Discord server that does not
   expire.
4. Ship a `config.ini` with your client that contains:

```ini
[Discord]
Presence=HideDetails
ApplicationId=123456789012345678
LargeImageKey=logo
SmallImageKey=
InviteUrl=https://discord.gg/yourcode
```

| Key             | Meaning |
|-----------------|---------|
| `Presence`      | `HideDetails` (default), `On` or `Off` - what the options window changes. It belongs to the player; ship it only to change the default. |
| `ApplicationId` | The application id from the developer portal. Empty: presence off. |
| `LargeImageKey` | Key of the large image. Its tooltip is the current map. Empty: no image. |
| `SmallImageKey` | Key of the small image, shown at the corner of the large one. Its tooltip is the character's class. Empty: no image. |
| `InviteUrl`     | Invite link to your Discord server, opened by the **Join** button. Empty: no button. |

A server with a Discord integration can send these values to the client
instead (OpenMU: the feature plugin *Discord integration of the client*); then
they don't have to be shipped in `config.ini`. The server's values win; the
ones in `config.ini` are used when the server sends none.

## Platforms and builds

- Windows, Linux and macOS. On Linux the Flatpak and Snap packages of Discord
  are found as well.
- Android and iOS have no Discord app to talk to; the presence is left out
  there.
- The CMake option `ENABLE_DISCORD` (default `ON`) builds the presence in.
  With `-DENABLE_DISCORD=OFF` none of it is compiled and the options window
  has no Discord setting.
