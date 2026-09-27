# In-game tests

Some client bugs only show in the running game with a server, often with two
clients: an item that cannot be put into the trade window, or equipment that
can be taken off where it should stay on. The in-game tests start real clients,
drive them through the [control socket](control-socket.md) like players and
report each scenario as passed or failed.

## Quick start

1. Build a developer client: a `-mueditor` preset, e.g.
   `cmake --preset windows-x64-mueditor` and
   `cmake --build out/build/windows-x64-mueditor --config Release`.
2. Start `InGameTests` next to the new `Main`, e.g.
   `out/build/windows-x64-mueditor/src/Release/InGameTests.exe`.
3. In its window, leave **Recreate the test server** checked (it needs podman
   or docker), check the scenarios, and press **Run and write report**;
   **Open report** shows every step with screenshots, in one file, and **Open
   report folder** shows that file, to attach it to a pull request.

The rest of this page explains each part.

## What you need

- A developer build of the client: a `-mueditor` preset, which turns on
  `ENABLE_CONTROL_SOCKET` and `ENABLE_IN_GAME_TESTS`. Player builds have
  neither, so the tester never reaches players.
- podman or docker with compose for the test server below (on Windows and
  macOS a running `podman machine`), or any other
  [OpenMU](https://github.com/MUnique/OpenMU) server with its test data.
- The .NET 10 runtime, which comes with the SDK the client build needs anyway.
  On Linux, a .NET installed with Microsoft's `dotnet-install.sh` sits in a
  folder the tester does not look in. The build targets tell it where; to
  start it by hand, set `DOTNET_ROOT` to that folder, or write the folder into
  `/etc/dotnet/install_location_x64`. Distribution packages register
  themselves.

## The tester

With `ENABLE_IN_GAME_TESTS` the build publishes the tester as one file next to
the client: `InGameTests.exe` next to `Main.exe` on Windows, `InGameTests`
next to `Main` on Linux and next to `Main.app` on macOS, e.g.
`out/build/windows-x64-mueditor/src/Release/InGameTests.exe`. It is published
again only when its sources change.

Start it without anything (a double-click, or `./InGameTests`) and it opens
its window with the client next to it; the reports go to
`in-game-test-results` next to it. The file can also be copied next to
another developer build of the same platform. `ENABLE_IN_GAME_TESTS` needs
`ENABLE_CONTROL_SOCKET`; CMake stops when only the first is on.

## The test server

`tools/InGameTests/docker-compose.yml` runs OpenMU 0.9.10 in demo mode (`-demo`): it
uses no database, keeps all data in memory and creates OpenMU's test data on
every start. Recreating the container therefore brings back the same accounts,
characters and items, whatever the last run changed; the `InGameTests` build
target does that before every run.

The image is pinned, because the scenarios depend on the exact test data; move
it to a newer OpenMU release on purpose, once the tests pass with it. Only the
game server of channel 1 is published, on `127.0.0.1:56901` (the test
accounts have public passwords, so it is not reachable from the network). The test
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
| `icarus-flying-item-take-off` | `test400` | `test400Elf` (level 400 High Elf with a Wing of Illusion, a Horn of Fenrir and room in the inventory) |

A new scenario takes an account no other scenario uses. When the test data has
no account with what a scenario needs, OpenMU's test data gets a new one
(`VersionSeasonSix/TestAccounts`).

## Running them

### The window

Start the tester next to the client, or let the `InGameTestsGui` build target
build everything and open it:

```sh
cmake --build out/build/windows-x64-mueditor --config Release --target InGameTestsGui
```

The window lists every scenario with a checkbox, grouped by category in boxes
that fold in and out (it remembers which are folded in); **Check all** toggles
them all. It has a **Wait after each action** field: the milliseconds each
client pauses after every click, key, walk or warp, so a person can follow what happens;
`1000` is easy to watch, `0` runs at full speed. The **Screenshot quality**
field sets the JPEG quality of the step screenshots (1 to 100, default 70):
lower makes the report smaller, e.g. 40 takes a run of both scenarios from
about 8 MB to about 5 MB. A scenario row can have its own wait and its own
quality, which then win over the fields. **Run and write report** runs the
checked scenarios one after the other; a bar over the list shows the steps of
all of them together (and at the end PASSED in green, or FAILED in red), and
each row its own progress and current step. **Open report** opens the report of
the run, and **Open report folder** shows the file in the file browser, ready to
be dragged into a pull request comment (before a run: the folder the reports go
to). The window remembers its
settings between runs. A line above the list says which server the tests run
against, and after a run which client commit they tested (in yellow when it
had changes that were not committed).

### The report

Every run, from the window or the command line, writes one file into the
output folder (`in-game-test-results` next to the tester):
`in-game-report-<date>-<time>.html`. Everything is in it, so it can be
attached to a pull request or an issue as it is (GitHub takes `.html` files up
to 25 MB in a comment; the tester warns when a report is bigger):

- On top, the run's result in large: PASSED in green when every test of the
  run passed, FAILED in red when one failed, or STOPPED when the run was
  stopped before its last test. Scenarios that were not selected do not count,
  but a yellow line below says which were not run; another says so when the
  client was built with changes that were not committed.
- What was tested against what: the git commit the client was built from
  (and whether it had changes that were not committed), and the server's
  OpenMU version and commit. The client says its commit itself (`ping`); the
  test server's version and commit are labels in its compose file, so another
  server shows as unknown.
- A table of every scenario there is, with PASS, FAIL, or SKIPPED for the ones
  the run did not include, and below it a section per scenario that opens on a
  click. Both are grouped by category, and a click on a category folds it in
  or out; **Expand all** and **Collapse all** open or close everything.
- An open section shows the scenario step by step: a step says what is done,
  what should happen then, whether it did and how long it took, with a
  screenshot of every client taken right after it; a failed step has the
  failure and the screenshot of what the clients showed then.
- A failed scenario also has the `state` and the last 200 events of every
  client, in blocks that open on a click.
- The results as JSON for tools, in the page's
  `<script type="application/json" id="results">`.

The screenshots are taken into a work folder in the temp folder, which the
run deletes once the report has them.

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
| `MU_IN_GAME_TEST_SCENARIOS` | the scenarios to run, e.g. `trade` or `trade;icarus-flying-item-take-off`; empty runs all |
| `MU_IN_GAME_TEST_STEP_DELAY` | milliseconds to pause after every client action, e.g. `1000` to watch a run; default `0` |
| `MU_IN_GAME_TEST_JPEG_QUALITY` | JPEG quality of the step screenshots, 1 to 100; default `70`; lower makes the report smaller |
| `MU_IN_GAME_TEST_SERVER` | the game server the clients log in to, `host:port`; default `127.0.0.1:56901`, the test server |
| `MU_IN_GAME_TEST_FRESH_SERVER` | recreate the test server before every run; default `ON`; turn it off for another server |

```sh
cmake -B out/build/windows-x64-mueditor "-DMU_IN_GAME_TEST_SCENARIOS=trade" "-DMU_IN_GAME_TEST_STEP_DELAY=1000"
```

Keep the quotes in PowerShell, which otherwise splits a value like
`127.0.0.1:56901` at the first dot. The build fails when a scenario fails.

The tester takes the same settings as options, e.g. next to the client:

```sh
./InGameTests --fresh-server --scenario trade -t 1000
```

From the sources, without a build, `dotnet run --project tools/InGameTests --`
followed by the options runs it too; it then needs `--client`.

| Option | Meaning |
|---|---|
| `--client` | the developer build of `Main` to start; default the `Main` next to the tester |
| `--server` | the game server the clients log in to, `host:port`; default `127.0.0.1:56901` |
| `--fresh-server` | recreate the test server with podman or docker before the run |
| `--scenario` | run only this scenario; repeat it for several; default all |
| `--out` | where the reports go; default `in-game-test-results`, next to the tester when it sits next to `Main` |
| `-t` | milliseconds to pause after every client action, so a person can follow; default `0` |
| `-q` | JPEG quality of the step screenshots, 1 to 100; default `70`; lower makes the report smaller |
| `--gui` | open the window; also without any option |
| `--list` | list the scenarios |

On Windows the tester is a window program, so no empty console window opens
with it; typed into a terminal it writes its log there, but the prompt comes
back at once. To wait for the run and its exit code, pipe its output, e.g.
`.\InGameTests.exe --fresh-server | Out-Host` in PowerShell, or use the
`InGameTests` target.

Each scenario starts its own clients, runs, and closes them again. The runner
prints every step and `PASS` or `FAIL` per scenario, and exits with `0` when
all passed, `1` when one failed and `2` for a wrong command line.

Keep the client windows visible while the tests run: a client whose window is
fully hidden may stop rendering, and its socket stops answering then.

## Scenarios

Scenarios are grouped by what they check; the window and the report list
them under these categories.

| Category | Scenario | Checks |
|---|---|---|
| Player Interactions | `trade` | Two clients warp to Lorencia, walk up to each other and open a trade. The seller puts a jewel into the trade window with two clicks, the buyer types an amount of zen into the trade's zen box, both press the confirm button, and both inventories show that jewel and zen changed owner: the buyer has one more jewel and that much less zen, the seller one jewel fewer and that much more zen (sven-n/MuMain#588). |
| Game Behaviour | `icarus-flying-item-take-off` | An Elf with wings and a Horn of Fenrir warps to Icarus. Right-clicking the wings takes them off, because the Fenrir flies; the Fenrir, now the last flying item, stays on both on a right-click and when dragged. Then the other way round: with the wings back on, right-clicking the Fenrir takes it off, and the wings, now the last flying item, stay on both ways (sven-n/MuMain#631). Both are put back on afterwards. |

## Writing a scenario

A scenario is a class in `tools/InGameTests/Scenarios` that derives from
`Scenario` and is listed in `Program.AllScenarios`. It names the clients it
needs by role and gets them started and logged out, says its `Category`
(`PlayerInteractions` for players doing something with each other,
`GameBehaviour` for how the game treats a player, e.g. the rules of a map; a
new kind gets a new value in `ScenarioCategory`), and its `StepCount` for
the progress bar.

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
  a server that keeps its data can be repeated (see `IcarusFlyingItemTakeOffScenario`,
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
