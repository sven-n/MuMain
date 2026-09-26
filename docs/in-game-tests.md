# In-game tests

Some client bugs only show in the running game with a server, often with two
clients: an item that cannot be put into the trade window, or equipment that
can be taken off where it should stay on. The in-game tests start real clients,
drive them through the [control socket](control-socket.md) like players and
report each scenario as passed or failed.

## What you need

- An editor build of the client (a `-mueditor` preset, which turns on
  `ENABLE_CONTROL_SOCKET`), e.g. `out/build/windows-x64-mueditor/src/Release/Main.exe`.
- An [OpenMU](https://github.com/MUnique/OpenMU) server with its test data,
  best the test server below.
- The .NET 10 SDK, which the client build needs anyway.

## The test server

`tools/InGameTests/docker-compose.yml` runs OpenMU in demo mode: the data lives
in memory, and every start creates OpenMU's test data again. A run therefore
always starts from the same accounts, characters and items, whatever the last
run changed. It uses OpenMU's default ports, so stop any other local OpenMU
first.

```sh
podman compose -f tools/InGameTests/docker-compose.yml up -d      # start
podman compose -f tools/InGameTests/docker-compose.yml restart    # reset the data
podman compose -f tools/InGameTests/docker-compose.yml down       # stop
```

`docker compose` works the same way. The tests also run against any other
OpenMU with the test data, but then they start from whatever state the last
run left.

## Test accounts

Every scenario logs in with accounts of its own from OpenMU's test data (the
password is the account name), so two scenarios never share one. The game
master accounts `testgm` and `testgm2` stay free for people.

| Scenario | Account | Character |
|---|---|---|
| `trade` | `test300`, `socket` | `test300Dl` (level 300 Dark Lord with Jewels of Bless) sells to `socketElf` |
| `icarus-take-off` | `test400` | `test400Elf` (level 400 High Elf with a Wing of Illusion, a Horn of Fenrir and room in the inventory) |

A new scenario takes an account no other scenario uses. When the test data has
no account with what a scenario needs, OpenMU's test data gets a new one
(`VersionSeasonSix/TestAccounts`).

## Running them

```sh
dotnet run --project tools/InGameTests -- --client out/build/windows-x64-mueditor/src/Release/Main.exe --server 127.0.0.1:55901
```

| Option | Meaning |
|---|---|
| `--client` | the editor build of `Main` to start |
| `--server` | the server the clients connect to, `host:port`; default `127.0.0.1:44405` |
| `--scenario` | run only this scenario; repeat it for several; default all |
| `--out` | where failure details go; default `in-game-test-results` |
| `--list` | list the scenarios |

Each scenario starts its own clients, runs, and closes them again. The runner
prints `PASS` or `FAIL` per scenario and exits with `0` when all passed, `1`
when one failed and `2` for a wrong command line.

When a scenario fails, the runner saves for every client a screenshot, the
last 200 events and the `state` into a folder named after the scenario and
the time.

Keep the client windows visible while the tests run: a client whose window is
fully hidden may stop rendering, and its socket stops answering then.

## Scenarios

| Scenario | Checks |
|---|---|
| `trade` | Two clients warp to Lorencia, meet and open a trade. The seller puts a jewel into the trade window with two clicks, both press the confirm button, and the jewel ends up in the buyer's inventory (sven-n/MuMain#588). |
| `icarus-take-off` | An Elf with wings and a Horn of Fenrir warps to Icarus. Right-clicking the wings takes them off, because the Fenrir flies; the Fenrir, now the last flying item, stays on both on a right-click and when dragged (sven-n/MuMain#631). The wings are put back on afterwards. |

## Writing a scenario

A scenario is a class in `tools/InGameTests/Scenarios` that derives from
`Scenario` and is listed in `Program.AllScenarios`. It names the clients it
needs by role and gets them started and logged out.

- **Test the real input path.** What the scenario checks has to go through the
  same code a player's click or key goes through: `ClickSlotAsync`,
  `MoveItemAsync` (two clicks: pick up, put down), `ClickElementAsync` and the
  `hotkey` command. Direct commands (`login`, `warp`, `move`, `trade
  request`, `equip`) are fine for the setup.
- **Ask for pixels, don't hard-code them.** The windows follow the responsive
  layout; `slot-pixel` and `ui` give the pixels for the current window size.
- **Wait for the server.** A click is answered some frames later. Use
  `Expect.EventuallyAsync` for something that has to happen, and
  `Expect.StillAfterAsync` for a refusal, where nothing happening is the
  answer. For an event, take `LastEventSequenceAsync` before the action and
  pass it to `WaitForEventAsync`, so an early answer is not missed.
- **Leave the test accounts as they were**, so the next run starts from the
  same data (see `IcarusTakeOffScenario`, which puts the wings back on).

## Not covered yet

- NPC windows: the chaos machine, storage and the NPC shop need a way to talk
  to an NPC and open its window.
- The item rule flags, e.g. that an item with `"tradable": false` cannot be put
  into the trade window.
- Running the tests in CI next to an OpenMU container.
