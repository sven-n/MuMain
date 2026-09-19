# Control socket

The client can be driven from a shell: logging in, selecting a character,
walking, fighting, chatting, reporting what it sees and taking screenshots,
without anyone at the keyboard. It is developer tooling for scripted tests and
demos, off unless a launcher asks for it, and it works the same on Windows and
on Linux.

## Turning it on

The socket is a build-time feature. It exists only in a client configured
with `ENABLE_CONTROL_SOCKET=ON` — the `-mueditor` presets and hand-configured
developer builds; the plain presets (`linux-x64`, `windows-x64`, …) and any
build that does not opt in compile none of it, and the `control_socket_leak`
test in their test suite proves it. A player build ignores the variable
below entirely: no socket, no log line, no change of behaviour.

In a build that has it, the client opens the socket only when
`MU_CONTROL_SOCKET` names a path:

```sh
MU_CONTROL_SOCKET=/run/user/1000/clients/default.sock ./Main /u127.0.0.1 /p44405
```

Without the variable nothing is created, nothing is logged and the client
behaves exactly as before. With it, the client logs one line
(`control socket listening on <path>`), creates the socket with owner-only
permissions (`0600`), replaces a stale file left behind by a crashed client,
and removes the file when it exits normally.

An option would not do: the client's command line is split on spaces, so a
path containing one could not be passed.

## Talking to it

Newline-delimited JSON, one request object per line, exactly one response per
line:

```sh
printf '{"cmd":"ping"}\n' | socat - UNIX-CONNECT:/run/user/1000/clients/default.sock
{"ok":true,"result":{"build":"Sep 16 2026 16:22:46","scene":"world"}}
```

- A request carries `cmd` and the command's own fields, plus an optional `id`
  that is echoed back so a caller can match answers to requests. `id` belongs
  to the framing: a drop is named by `item`, not by `id`.
- A response is `{"ok":true,"result":{…}}` or
  `{"ok":false,"error":"<code>","message":"…"}` — with a `result` holding the
  progress made when a long-running command was interrupted or timed out.
- Requests are served on the main thread, once per frame, after the packets of
  that frame have been processed. Nothing runs concurrently with game logic.
- Several connections may be open at once. Commands that drive the character
  (`login`, `move`, `attack`, …) run one at a time: a second one interrupts the
  first, whose caller is told how far it got. Reading commands (`state`,
  `events`, `wait-for`, `screenshot`) run alongside.

Error codes: `bad_request`, `unknown_command`, `wrong_scene`, `busy`,
`interrupted`, `timeout`, `not_connected`, `login_failed`, `no_such_character`,
`no_such_skill`, `not_in_view`, `not_attackable`, `no_path`, `not_allowed`,
`warp_refused`, `skill_refused`, `insufficient_mana`, `not_pickable`,
`empty_slot`, `move_refused`, `failed`.

## Commands

| Command | What it does |
|---|---|
| `ping` | build identifier and current scene |
| `scene` | which screen the client is on: `login`, `character_list`, `world`, … |
| `state` | the character and everything around it (see below) |
| `nearby` | the objects the client can see |
| `events` (`since`, `follow`) | recorded events, or a live stream of them |
| `wait-for` (`event`, `match`, `timeout`) | block until a matching event arrives |
| `screenshot` (`out`) | capture the next frame to a path; without `out` it names itself, uniquely per capture |
| `hotkey` (`key`) | press one game key for a frame: `esc`, `i`, `home`, `f1`, … |
| `click-ui` (`x`, `y`, `button`) | click a window pixel (`left` by default) |
| `login` (`account`, `password`, `server`) | server selection, credentials, character list |
| `select-char` (`name` or `slot`) | enter the world with that character |
| `logout`, `quit` | back to the character list; close the client |
| `move` (`x`, `y`) | walk there with the client's own path finder |
| `warp` (`gate`) | use a warp-list entry by name |
| `teleport` (`x`, `y`, `map`) | the game master's own move command |
| `attack` (`target`, `times`, `interval`) | plain attacks on an id or character name |
| `skill` (`skill`, `target`) | cast a skill the character owns |
| `pickup` (`item`) | walk to a drop and take it, by the id `nearby` reports for it |
| `use` (`slot`), `equip` (`slot`, `target_slot`) | inventory actions |
| `say` (`text`), `whisper` (`name`, `text`) | chat, including `/` commands |
| `party` (`action`, `target`) | `invite`, `accept`, `decline`, `leave` |
| `halt` | stop the walk or repeated attack in progress |

`state` reports the scene and account on every screen, and in the world adds:
character name, class, level, experience, zen, HP/mana/SD/AG with their
maxima, map number and name, position, alive flag, safe-zone flag, current
target, the skills the character owns, equipment, inventory, buffs, party and
`nearby`.

### Synthetic input

`hotkey` and `click-ui` inject a key or a click *below* the game's own input
readers: the key counts as down for one rendered frame in the same key-state
scan a physical key goes through, and the click writes the same mouse
variables the event loop fills from real mouse events, at a window pixel. The
window system is never involved — no pointer movement, no focus change, no
synthetic OS events — so a scripted session can open the system menu, the
inventory or start the MU Helper from its HUD button while the human keeps
working elsewhere. Coordinates for `click-ui` are the pixels of the client
area, the same space a `screenshot` image is in, so a script can capture,
locate and click. Key names are case-insensitive: the letters, the digits,
`esc`, `enter`, `tab`, `space`, `backspace`, `home`, `end`, `insert`,
`delete`, `pageup`, `pagedown`, `up`, `down`, `left`, `right`, `printscreen`
and `f1`–`f12`; anything else answers `bad_request`. Both answer once the
release frame has run; a second injection while one is in flight answers
`busy`, while a walk or an attack in flight is left alone — injecting a key is
an observation of the act slot, not a claim on it. Not covered: typing text (`say` sends chat), key chords, drags.

## Events

The packet handlers record what a scenario asserts on. Each event carries a
strictly increasing `seq`, a UTC `time` and its own fields:

| Event | Fields |
|---|---|
| `hit` | `direction` (`dealt`/`received`), `attacker`/`target` `{id,name,kind}`, `damage`, `shield_damage`, `critical`, `missed` |
| `killed` | `victim`, `killer` |
| `stat` | `stat` (`life`, `mana`, `sd`, `ag`, `experience`, `level`), `value`, `max` |
| `chat` | `sender`, `text`, `kind` |
| `drop` / `drop_gone` | `id` (the id `pickup` takes), `item`, `position` / `reason` |
| `map` | `map`, `map_name`, `position` |
| `scene` | `scene` |
| `view_enter` / `view_leave` | `object` |
| `party` | `change`, `name` |
| `disconnect` | `reason` |
| `error` | `command`, `error`, `message` |

The last 2,048 events are kept. Names and fields are plain lower snake case,
so a test script can parse them next to a server-side event stream.

## Security posture

**Build-time gate first.** A player build contains no control code at all:
`ENABLE_CONTROL_SOCKET` is off by default, the plain presets and CI keep it
off, none of `App/Control/` or the local-socket transport is compiled, and
the activation variable's name does not appear in the executable. What a
player can reach by setting an environment variable is therefore nothing.
What stays in every build is the mouse-free automation the in-game helper
already had (`GameLogic/Automation`) and the extracted login/character entry
points — refactorings of existing code, not new surface. The gate removes the
ready-made API and the flip-a-switch path; it does not defend against a
patched or rebuilt binary.

Within a developer build the socket is unauthenticated: any process of the
same user can drive the client, like any other local developer endpoint. It is
a filesystem socket with mode `0600` in the user's runtime directory, never a
network address, never enabled from `config.ini`, and never on unless the
launcher sets the variable.

## Keeping the taps when `WSclient.cpp` changes

The event recorders are one-line calls named `App::Control::Events::Record…`,
sitting at the end of the packet receive functions in
`src/source/Network/Server/WSclient.cpp` (hits, deaths, experience, stats,
chat, whisper, drops appearing and vanishing, view enter/leave, party changes,
logout) plus the scene and map watcher in `App/Control/ControlServer.cpp`.
When one of those functions is rewritten:

1. `rg -c 'App::Control::Events::' src/source/Network/Server/WSclient.cpp` —
   the count is 19; a lower one means a tap was dropped.
2. Re-run the live checks that cover the dropped tap (a fight records `hit`,
   `killed` and `stat`; a pickup records `drop` and `drop_gone`).

## Notes from the field

- The client's main loop stops running while its window is fully occluded (the
  compositor stops sending frame callbacks), and with it the socket stops
  answering. Two clients side by side must both stay visible; tiled windows
  answer `ping` in about 4 ms.
- The server's speed check bans an account after four warnings in an hour, so
  every command that walks paces its steps (300 ms between walk packets,
  250 ms between automation steps). Do not remove that pacing.
- Attacks are refused inside a safe zone; `state`'s `safe_zone` flag says when
  the character is in one, and `attack` answers `not_allowed` there.
