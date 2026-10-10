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
`empty_slot`, `move_refused`, `not_open`, `failed`.

## Commands

| Command | What it does |
|---|---|
| `ping` | build identifier, the git `commit` the client was built from (`commit_changed` when tracked files differed from it), and the current scene |
| `scene` | which screen the client is on: `login`, `character_list`, `world`, … |
| `state` | the character and everything around it (see below) |
| `nearby` | the objects the client can see |
| `events` (`since`, `follow`) | recorded events, or a live stream of them |
| `wait-for` (`event`, `match`, `timeout`) | block until a matching event arrives |
| `screenshot` (`out`, `quality`) | capture the next frame as a JPEG to a path; without `out` it names itself, uniquely per capture; `quality` 1 to 100, default 100 |
| `hotkey` (`key`) | press one game key for a frame: `esc`, `i`, `home`, `f1`, … |
| `click-ui` (`x`, `y`, `button`) | click a window pixel (`left` by default) |
| `hover-ui` (`x`, `y`) | move the pointer to a window pixel with no button, as physical motion: RmlUi hover and the game's own pointer (native hover, tooltips) both follow it |
| `drag-ui` (`x`, `y`, `to_x`, `to_y`, `button`) | press at one window pixel, move to the other over eight frames with the button held, release there |
| `type` (`text`, `enter`) | deliver committed UTF-8 to the focused field; optional boolean `enter` submits on a later frame |
| `ui` | the open windows by name — `inventory`, `inventory_extension`, `character`, `trade`, `storage`, `storage_extension`, `mix`, `npc_shop`, `lucky_item`, `chat_input`, `party`, `command`, `my_shop`, `purchase_shop`, `npc_quest` (the quest dialog of Sebina, Marlon and Devin), and `message_box` while a dialog waits for Enter or Esc — and the window pixels of the named elements that are shown: `trade.confirm`, `trade.zen`, `inventory.repair`, `inventory.my_shop`, `npc_shop.repair`, `npc_shop.repair_all`, `my_shop.title`, `my_shop.open`, `my_shop.close`, `command.trade`, `command.purchase`, `command.party`, `npc_quest.answer.0` and on (the rows of the quest dialog's answers), `npc_quest.complete`, `npc_quest.close`, `character.stat.strength` (`agility`, `vitality`, `energy`, `command`: the character window's "+" buttons, while there are level-up points) |
| `slot-pixel` (`grid`, `slot`) | the window pixel of a slot's square: `inventory` and `equipment` (the slot numbers `state` reports), `trade`, `trade_partner`, `storage`, `mix`, `npc_shop`, and the personal shops `my_shop` and `purchase_shop` (slots 204 and up, as `state` and the server number them); `not_open` while that window is closed, `bad_request` for a slot the grid does not have |
| `login` (`account`, `password`, `server`) | server selection, credentials, character list |
| `select-char` (`name` or `slot`) | enter the world with that character |
| `logout`, `quit` | back to the character list; close the client |
| `move` (`x`, `y`) | walk there with the client's own path finder |
| `warp` (`gate`) | use a warp-list entry by name |
| `teleport` (`x`, `y`, `map`) | the game master's own move command; `map` is an index, the current map when omitted |
| `attack` (`target`, `times`, `interval`) | plain attacks on an id or character name |
| `skill` (`skill`, `target`) | cast a skill the character owns — with `target` it is aimed at that object, without one it is cast where the character stands |
| `pickup` (`item`) | walk to a drop and take it, by the id `nearby` reports for it |
| `use` (`slot`), `equip` (`slot`, `target_slot`) | inventory actions |
| `say` (`text`), `whisper` (`name`, `text`) | chat, including `/` commands |
| `party` (`action`, `target`) | `invite`, `accept`, `decline`, `leave` |
| `trade` (`action`, `target`) | `request` a trade with another player at most one tile away (`not_allowed` otherwise), or `cancel` the open one; the partner accepts with the Enter key, and items go in with `click-ui` |
| `halt` | stop the walk or repeated attack in progress |

`state` reports the scene and account on every screen, and in the world adds:
character name, class, level, experience, zen, HP/mana/SD/AG with their
maxima, map number and name, position, alive flag, safe-zone flag, current
target, the skills the character owns, equipment, inventory, buffs, party,
the open trade (partner, both offers with their items and zen, both confirm buttons and `my_confirm_wait`, the frames until my button takes clicks again, or `null`) and
`nearby`. An item carries `slot`, `name`, `level`, `durability`,
`max_durability`, and its `width` and `height` in inventory squares; one that
covers several squares is listed once for each of them. While a click would
repair it (the inventory's repair mode, or an NPC that repairs), a worn item
also carries `repair_price`, and while an NPC shop is open an inventory item
carries `sell_price`: both as the item's tooltip shows them.

The shops: `npc_shop` is the open NPC shop (`repair_shop`, `tax_rate`,
`repair_all_price` at an NPC that repairs, and its goods with `price`, tax
included) or `null`; `my_shop` is the player's own personal shop (`open`, the
`title` in its title field, its goods with `price`), `purchase_shop` the
personal shop the player looks into (`seller`, `title`, goods with `price`) or
`null`. `repair_mode` says whether a click repairs instead of picking up.

For the quests and their rewards: `class_name` (e.g. `Blade Knight`; `class`
is the client's number, e.g. 1 Dark Knight, 8 Blade Knight, 12 Blade Master),
`level_up_points`, `stats` (`strength`, `agility`, `vitality`, `energy`,
`command`, without bonuses), `combo` (the Blade Knight's combo from Marlon),
`quests` (the seven legacy quests with `index`, `name` and `state`:
`active`, `complete`, `not_started`, `none`, or `unknown` for a value outside
these, as in the `quest` events), and `npc_quest`, the quest
dialog on screen (`quest`, `page`, `text`, `need_zen`, and its `answers`, each
with its `text` and `action`: `next` turns the page, `accept` takes the quest,
`complete` hands it in, `close` ends the talk) or `null`.

Each `nearby` object carries `id`, `kind`, `name`, `position`, and a player,
monster or NPC also `alive`, `level` and `hp_percent`. `hp_percent` is a
percentage of full health (`100` is untouched) and is `null` when the server
has not told the client that object's health — a threshold test has to allow
for the null rather than read it as zero. A player, monster or NPC also
carries `pixel`, the window pixel `{x, y}` at the middle of the box the mouse
picks it by, or `null` while it is not drawn: `click-ui` there talks to an NPC
or targets a player the way a player's click does. The pixel is taken from the
last rendered frame, so it lags a moving object by a frame, and a window drawn
over it catches the click instead.

### Synthetic input

`hotkey` and `click-ui` deliver input to RmlUi first, then to the older
key/button readers only if the UI did not consume it. They do not move the OS
pointer or change window focus. A click rests on its pixel for two frames
before the press, as a real mouse does: an item grid only picks up an item it
saw hovered. Use screenshot pixels as `click-ui` coordinates: for
example, click the Menu button, then take another screenshot to inspect the
panel. The older `CInput` widgets still hit-test the OS cursor and cannot be
clicked remotely; use their keyboard navigation instead. Keys are
case-insensitive: letters, digits, `esc`, `enter`, `tab`, `space`, `backspace`,
`home`, `end`, `insert`, `delete`, `pageup`, `pagedown`, arrows,
`printscreen` and `f1`–`f12`.

Focus a text field with a supported UI click or keyboard navigation before
`type`. For example, `{"cmd":"type","text":"hello"}` inserts committed text;
`{"cmd":"type","text":"hello","enter":true}` delivers Return on a later rendered frame.
`text` must be 1–256 UTF-8 bytes with no NUL, ASCII control character or DEL;
`enter` must be boolean. A successful reply confirms delivery, **not** that a
field accepted the characters or a login succeeded. Its result reports only
byte count and whether Enter was requested, never the text (which may be a
password). No IME composition or key chords are synthesized; `drag-ui` is the only drag.

Only one injection runs at a time; another answers `busy`. Injection commands
are observational and do not interrupt an ongoing world act. They answer after
release processing on rendered frames. A stalled client times out and retracts
pending input rather than delivering it later. Cancellation is not a rollback:
text or effects already delivered before cancellation remain. An abandoned
held RmlUi click is cleared without activating the pressed element. Independent
physical input continues to work; a physical press of the same button cancels
an outstanding scripted click before that press is routed, returning a
`failed` response naming the physical press; a lost window/UI delivery target
instead reports `failed` with a target-disappeared message. UI hover can remain
at the injected pixel after a completed click or a cancelled click whose UI
press did not need clearing (a non-primary click or a primary click the UI did
not consume). A later physical or scripted motion updates it; the OS pointer
itself never moves.
A scripted key also uses the human's current physical modifiers for RmlUi
navigation; focused portable fields receive the key without those modifiers.

## Events

The packet handlers record what a scenario asserts on. Each event carries a
strictly increasing `seq`, a UTC `time` and its own fields:

| Event | Fields |
|---|---|
| `hit` | `direction` (`dealt`/`received`), `attacker`/`target` `{id,name,kind}`, `damage`, `shield_damage`, `critical`, `missed` |
| `killed` | `victim`, `killer` |
| `stat` | `stat` (`life`, `mana`, `sd`, `ag`, `level`, `experience_gained` — the experience of one kill, and `damage_dealt` with it — the cumulative total is `state.experience`), `value`, `max` |
| `chat` | `sender`, `text`, `kind` (`public`, `whisper`, `party`, `guild`, `union`, `gens`, `gm`) |
| `drop` / `drop_gone` | `id` (the id `pickup` takes), `item`, `position` / `reason` |
| `map` | `map`, `map_name`, `position` |
| `scene` | `scene` |
| `view_enter` / `view_leave` | `object` |
| `quest` | `change`: `state` with the legacy `quest` and its new `state` (`active`, `complete`, `not_started`, `none`, `unknown`), or `reward` with the character's `name`, the `reward` (`level_up_points`, `second_class`, `points_per_level`, `combo`, `third_class`), its `amount` (none for the two class changes, whose new class is `class`) and the character's `class` afterwards; the client records the rewards of every player in view, so a script matches its own `name` |
| `party` | `change` (`invited` with the inviter's `name`; `list` with the leader's `name` after every change of the members; `left`; `result` with `result` — `failed`, `denied`, `full`, `user_left`, `other_party`, `left`, `opposing_gens`, `battle_zone`, `battle_zone_off` — when an invitation formed no party) |
| `trade` | `change` (`requested`, `opened`, `refused`, `unavailable`, `partner_confirm`, `closed`), `name` for a request or an opened trade, `state` (`checked`, `unchecked`, `reset`; `unknown` for a value outside the protocol) for the partner's button, `result` (`completed`, `cancelled`, `inventory_full`, `request_cancelled`, `reinforced_item`) when it closes; `refused` also on the asked side, when a window that forbids trading is open and the client says no by itself |
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
invitations and answers, quest states and rewards,
trade steps, logout) plus the scene and map watcher in `App/Control/ControlServer.cpp`.
When one of those functions is rewritten:

1. `rg -c 'App::Control::Events::' src/source/Network/Server/WSclient.cpp` —
   the count is 32; a lower one means a tap was dropped. Compare it against
   `git show upstream/main:…` when the number itself is in doubt: the count
   is a smoke test, the list above is the contract.
2. Re-run the live checks that cover the dropped tap (a fight records `hit`,
   `killed` and `stat`; a pickup records `drop` and `drop_gone`; the `trade`
   in-game test records the trade steps, see [in-game-tests.md](in-game-tests.md)).

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
