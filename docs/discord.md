# Discord Rich Presence

While the game runs, your Discord profile can show what you are doing in it:

```
MU Online
Lv 380 / ML 120 Blade Knight
In Blood Castle 5 · Party 3/5
00:42 elapsed
```

The client talks to the Discord app running on the same computer. Nothing is
sent anywhere else, no Discord account is linked, and the game never waits on
Discord: when Discord isn't running, nothing happens, and when it is started
later, the presence appears within a few seconds.

> Requires the Discord desktop app and a server that set up a Discord
> application (see [For server operators](#for-server-operators)). Without an
> application id in `config.ini` the presence stays off.

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

## Messages from Discord

> Requires a server with a Discord chat bridge, like OpenMU's (its guild and
> alliance chat can be bound to Discord channels).

A message written in a Discord channel that the server bridges into the game
appears in the chat of its scope - guild or alliance - with that chat's
colours. Its sender is shown as `[Discord] <name>`: with OpenMU's bridge the
name is the character the Discord user is linked to (OpenMU sends it as
`@<name>`, which no character can be called). A server that sends the Discord
name itself can show it with up to 32 characters.

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
3. Ship a `config.ini` with your client that contains:

```ini
[Discord]
Presence=HideDetails
ApplicationId=123456789012345678
LargeImageKey=logo
SmallImageKey=
```

| Key             | Meaning |
|-----------------|---------|
| `Presence`      | `HideDetails` (default), `On` or `Off` - what the options window changes. It belongs to the player; ship it only to change the default. |
| `ApplicationId` | The application id from the developer portal. Empty: presence off. |
| `LargeImageKey` | Key of the large image. Its tooltip is the current map. Empty: no image. |
| `SmallImageKey` | Key of the small image, shown at the corner of the large one. Its tooltip is the character's class. Empty: no image. |

A later client version will be able to take these values from the server
instead, so they don't have to be shipped in `config.ini`.

## Platforms and builds

- Windows, Linux and macOS. On Linux the Flatpak and Snap packages of Discord
  are found as well.
- Android and iOS have no Discord app to talk to; the presence is left out
  there.
- The CMake option `ENABLE_DISCORD` (default `ON`) builds the presence in.
  With `-DENABLE_DISCORD=OFF` none of it is compiled and the options window
  has no Discord setting.
