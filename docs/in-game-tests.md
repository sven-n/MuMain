# In-game tests

Some client bugs only show in the running game with a server, often with two
clients: an item that cannot be put into the trade window, or equipment that
can be taken off where it should stay on. The in-game tests start real clients,
drive them through the [control socket](control-socket.md) like players and
report each scenario as passed or failed.

## What you need

- An editor build of the client (a `-mueditor` preset, which turns on
  `ENABLE_CONTROL_SOCKET`), e.g. `out/build/windows-x64-mueditor/src/Release/Main.exe`.
- podman or docker with compose for the test server below (on Windows and
  macOS a running `podman machine`), or any other
  [OpenMU](https://github.com/MUnique/OpenMU) server with its test data.
- The .NET 10 SDK, which the client build needs anyway.

## The test server

`tools/InGameTests/docker-compose.yml` runs OpenMU in demo mode (`-demo`): it
uses no database, keeps all data in memory and creates OpenMU's test data on
every start. Recreating the container therefore brings back the same accounts,
characters and items, whatever the last run changed; the `InGameTests` build
target does that before every run.

Only the game server of channel 1 is published, on host port `56901`. The test
clients log in to it directly, without the connect server, so no IP resolver
setting is needed, and a local OpenMU on its default ports can keep running
next to it.

```sh
podman compose -f tools/InGameTests/docker-compose.yml up -d --force-recreate   # start with fresh data
podman compose -f tools/InGameTests/docker-compose.yml down                     # stop
```

`docker compose` works the same way. When the runner recreates the server and
podman answers "Cannot connect to Podman" although its machine runs, podman's
file with the connection to the machine is missing (`podman system connection
list` is empty); the runner then connects as `podman machine inspect` describes
the machine, so nothing has to be set up by hand.

The tests also run against any other
OpenMU with the test data, but then they start from whatever state the last
run left: `trade`, for one, moves a jewel from `test300Dl` to `socketElf`
each time.

## Test accounts

Every scenario logs in with accounts of its own from OpenMU's test data (the
password is the account name), so two scenarios never share one. The game
master accounts `testgm` and `testgm2` stay free for people. The test data
puts every character in the safe zone of its class's home map, e.g. an Elf in
Noria.

| Scenario | Account | Character |
|---|---|---|
| `trade` | `test300`, `socket` | `test300Dl` (level 300 Dark Lord with Jewels of Bless) sells to `socketElf` |
| `icarus-take-off` | `test400` | `test400Elf` (level 400 High Elf with a Wing of Illusion, a Horn of Fenrir and room in the inventory) |

A new scenario takes an account no other scenario uses. When the test data has
no account with what a scenario needs, OpenMU's test data gets a new one
(`VersionSeasonSix/TestAccounts`).

## Running them

### The window

The `InGameTestsGui` build target builds `Main` and opens a window to run the
tests with it:

```sh
cmake --build out/build/windows-x64-mueditor --config Release --target InGameTestsGui
```

It lists every scenario with a checkbox (**Check all** toggles them all) and
has a **Wait after each action** field: the milliseconds each client pauses
after every click, key, walk or warp, so a person can follow what happens;
`1000` is easy to watch, `0` runs at full speed. A scenario row can have its
own wait, which then wins over the field. **Run and write report** runs the
checked scenarios one after the other, shows each one's current step, and
**Open report** opens the report of the run. The window remembers its
settings between runs; the runner started with `--gui`, or without any
option, opens it too.

### The report

Every run, from the window or the command line, writes a folder named after
its start time into the output folder (`in-game-test-results` in the build
folder for the build targets):

- `report.html`: every scenario step by step. A step says what is done, what
  should happen then, whether it did and how long it took, with a screenshot
  of every client taken right after it; a failed step has the failure and the
  screenshot of what the clients showed then. The screenshots are embedded, so
  the file can be attached to a pull request or an issue as it is.
- `results.json`: the same for tools.
- a folder per scenario with the screenshots, and on a failure the recent
  events and the `state` of every client.

### The command line

The `InGameTests` build target builds `Main`, recreates the test server and
plays the scenarios:

```sh
cmake --build out/build/windows-x64-mueditor --config Release --target InGameTests
```

Cache variables choose what it runs; set them once with `-D` when configuring,
or in CLion's CMake options:

| Variable | Meaning |
|---|---|
| `MU_IN_GAME_TEST_SCENARIOS` | the scenarios to run, e.g. `trade` or `trade;icarus-take-off`; empty runs all |
| `MU_IN_GAME_TEST_STEP_DELAY` | milliseconds to pause after every client action, e.g. `1000` to watch a run; default `0` |
| `MU_IN_GAME_TEST_SERVER` | the game server the clients log in to, `host:port`; default `127.0.0.1:56901`, the test server |
| `MU_IN_GAME_TEST_FRESH_SERVER` | recreate the test server before every run; default `ON`; turn it off for another server |

```sh
cmake -B out/build/windows-x64-mueditor "-DMU_IN_GAME_TEST_SCENARIOS=trade" "-DMU_IN_GAME_TEST_STEP_DELAY=1000"
```

Keep the quotes in PowerShell, which otherwise splits a value like
`127.0.0.1:56901` at the first dot. The build fails when a scenario fails.

The runner can also be started directly:

```sh
dotnet run --project tools/InGameTests -- --client out/build/windows-x64-mueditor/src/Release/Main.exe --fresh-server -t 1000
```

| Option | Meaning |
|---|---|
| `--client` | the editor build of `Main` to start |
| `--server` | the game server the clients log in to, `host:port`; default `127.0.0.1:56901` |
| `--fresh-server` | recreate the test server with podman or docker before the run |
| `--scenario` | run only this scenario; repeat it for several; default all |
| `--out` | where the run folders go; default `in-game-test-results` |
| `-t` | milliseconds to pause after every client action, so a person can follow; default `0` |
| `--gui` | open the window instead |
| `--list` | list the scenarios |

Each scenario starts its own clients, runs, and closes them again. The runner
prints every step and `PASS` or `FAIL` per scenario, and exits with `0` when
all passed, `1` when one failed and `2` for a wrong command line.

Keep the client windows visible while the tests run: a client whose window is
fully hidden may stop rendering, and its socket stops answering then.

## Scenarios

| Scenario | Checks |
|---|---|
| `trade` | Two clients warp to Lorencia, walk up to each other and open a trade. The seller puts a jewel into the trade window with two clicks, both press the confirm button, and the jewel ends up in the buyer's inventory (sven-n/MuMain#588). |
| `icarus-take-off` | An Elf with wings and a Horn of Fenrir warps to Icarus. Right-clicking the wings takes them off, because the Fenrir flies; the Fenrir, now the last flying item, stays on both on a right-click and when dragged (sven-n/MuMain#631). The wings are put back on afterwards. |

## Writing a scenario

A scenario is a class in `tools/InGameTests/Scenarios` that derives from
`Scenario` and is listed in `Program.AllScenarios`. It names the clients it
needs by role and gets them started and logged out.

- **Write it as steps.** Everything a scenario does goes through
  `context.StepAsync(title, expectation, action)`: the title says what is
  done ("The buyer accepts with Enter"), the expectation what should happen
  then, for someone who reads the report without knowing the code ("The trade
  window opens for both characters"), and the action does it and checks that
  it happened. After each step every client takes a screenshot, so a step is
  one thing a person can see.

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
- **Leave the test accounts as they were** where it is cheap, so a run against
  a server that keeps its data can be repeated (see `IcarusTakeOffScenario`,
  which puts the wings back on).
- **Don't count on where a character stands.** A warp to a town lands anywhere
  in it, so walks take different times from run to run (see
  `TradeScenario.WalkUpToAsync`).

## Not covered yet

- NPC windows: the chaos machine, storage and the NPC shop need a way to talk
  to an NPC and open its window.
- The item rule flags, e.g. that an item with `"tradable": false` cannot be put
  into the trade window.
- Running the tests in CI next to an OpenMU container.
