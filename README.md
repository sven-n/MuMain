# MU Online Client Sources

[![Ask DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/sven-n/MuMain)

A fork of the Season 5.2 client sources [uploaded by Louis](https://github.com/LouisEmulator/Main5.2),
modernized for Season 6 Episode 3 and playable with [OpenMU](https://github.com/MUnique/OpenMU).
The project improves rendering, UI, networking and developer tooling while continuing work
on Season 6 compatibility.

## Features

### Gameplay and compatibility

- 🔥 Inventory and vault extensions, the Season 6 master skill tree, and Rage Fighter support.
- 🔥 MU Helper UI and core logic, with further improvements still planned.
- 🔥 Automatic reconnection and monster health bars after attacking.
- 🔥 Qubit's improvements to right-click equipping, Dark Lord's Raven visuals, Fenrir glow,
  and screen resolutions.
- 🔥 Extended Season 6 networking with larger damage/EXP values and improved item and
  appearance serialization. The protocol includes custom extensions.

### Rendering and UI

- 🔥 Higher frame rates than the original's fixed 25 FPS: VSync by default, uncapped when VSync
  is off (`$fps <value>` sets a cap), while animation and movement keep their 25 FPS timing
  ([ADR 0001](docs/adr/0001-preserve-25-fps-reference-timing.md)).
- 🔥 SDL GPU rendering with deferred draw batching, indexed geometry, growable buffers,
  packed bone palettes, allocation-free sprites, and detailed performance statistics.
- 🔥 A unified RmlUi system with layout and styling in RML/RCSS, centralized UI scaling,
  theme inheritance, and theme-defined window placement.
- 🔥 Unicode text and a generated translation system for runtime language switching.
- 🔥 A C# .NET 10 network library built with Native AOT, using MUnique.OpenMU.Network.

### Developer tools

- 🔥 Socket-based automation for gameplay and UI interactions, state inspection, and screenshots;
  the in-game tester generates pass/fail reports.
- 🔥 An in-game MU Editor for camera and rendering tuning, terrain and world object editing,
  and minimap capture.
- 🔥 Chat diagnostics for frame timing, rendering statistics and per-system rendering cost,
  plus UI previews of any window or event; see [Chat commands](#chat-commands).

Remaining Season 6 work includes Lucky Items.

## Release downloads

Hosted releases provide Windows native x64 Release, Linux x64 Release and macOS arm64 Release
builds with the editor disabled; Semantic Release publishes all three as GitHub Release assets.
Each runtime excludes `Data/` and `fonts/`: its **Compatible game data** link points to the
separate data release tagged `data-<id>`. Follow the platform-specific assembly steps in the
[release instructions](docs/build/README.md#hosted-releases).

x86, Debug, editor-enabled and MinGW configurations are not hosted; build locally where
supported. See the [supported targets](docs/build/README.md#supported-targets).

## Build quick start

Use **CMake 3.25+**, a platform C++ toolchain, and the **.NET 10 SDK**. Platform-specific
prerequisites, IDE setup, dependency troubleshooting and cross-compilation instructions
live in the [build guide](docs/build/README.md).

| Environment             | Guide                                                                   |
| ----------------------- | ----------------------------------------------------------------------- |
| Windows / Visual Studio | [Visual Studio setup](docs/build/windows/visual-studio.md)              |
| Windows / terminal      | [CMake presets and prerequisites](docs/build/windows/console.md)        |
| Linux / terminal        | [Native Linux setup](docs/build/linux/console.md)                       |
| Linux or Windows IDEs   | [CLion, Rider and VS Code guides](docs/build/README.md#pick-your-setup) |
| WSL / MinGW             | [Windows cross-compilation](docs/build/windows/wsl.md)                  |

For a Windows x64 player build, install the prerequisites in the Windows terminal guide,
set `VCPKG_ROOT`, then run from the repository root in a Visual Studio Developer PowerShell:

```powershell
cmake --preset windows-x64
cmake --build --preset windows-x64-release
cd out/build/windows-x64/src/Release
.\Main.exe
```

CMake initializes required submodules and downloads dependencies during configuration.
Native builds also build the network library and stage game assets beside the executable.
Run from that output directory so the client finds its data and libraries.

Player presets disable the editor and control socket. The `-mueditor` presets enable
both, plus the in-game tester. These configure-time choices are independent of Debug,
Release or RelWithDebInfo; see [build options](docs/build/README.md#build-options).

## Running the client

Run `Main.exe` from its output directory. It connects to the server in `config.ini`
(`[CONNECTION SETTINGS]`, localhost port `44406` by default); `Main.exe connect /u<IP> /p<PORT>`
overrides it for that run, for example `Main.exe connect /u192.168.0.20 /p55902`. The
[OpenMU launcher](https://github.com/MUnique/OpenMU/releases) passes the same arguments.
The client identifies itself with version `2.04d` and serial `k1Pk2jcET48mxL3b`.

### Client configuration (`config.ini`)

The client reads `config.ini` from the executable directory and creates it on first save.
Missing keys take the defaults below. The options window writes most of them; see
[Options window and config](docs/options-window.md).

| Section                     | Key                                                                                                   | Default              | Description                                                                                                                                                      |
| :-------------------------- | :---------------------------------------------------------------------------------------------------- | :------------------- | :--------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **`[Window]`**              | `Width`, `Height`                                                                                     | `1024`, `768`        | Window or fullscreen resolution.                                                                                                                                 |
| **`[Window]`**              | `Windowed`                                                                                            | `1`                  | `0` runs fullscreen.                                                                                                                                             |
| **`[Audio]`**               | `SoundVolume`, `MusicVolume`                                                                          | `5`                  | `0` mutes.                                                                                                                                                       |
| **`[LOGIN]`**               | `Language`                                                                                            | `Eng`                | Game data language: the `Data/Local/<Language>/` folder (quest, skill and minimap texts, map name art). Item, monster and NPC names follow `[UI] Locale`.        |
| **`[LOGIN]`**               | `RememberMe`, `SavePassword`                                                                          | `0`                  | Remember the account name, and the password too. Both are stored encrypted for this machine and user (`EncryptedUsername`, `EncryptedPassword`).                 |
| **`[CONNECTION SETTINGS]`** | `ServerIP`, `ServerPort`                                                                              | `localhost`, `44406` | Server to connect to when no `connect` argument is given.                                                                                                        |
| **`[UI]`**                  | `Locale`                                                                                              | `en`                 | UI language (`en`, `de`, `es`, `id`, `ja`, `pl`, `pt`, `ru`, `tl`, `uk`, `zh-CN`, `zh-TW`). Switchable at runtime in the options window.                                  |
| **`[UI]`**                  | `Font`                                                                                                | empty                | UI font family; empty uses the platform's default.                                                                                                               |
| **`[UI]`**                  | `RmlTheme`                                                                                            | `legacy`             | UI theme: `legacy` (the original look) or `modern`. `$theme <name>` switches it for the session.                                                                 |
| **`[UI]`**                  | `UIScalePercent`                                                                                      | `100`                | UI scale, clamped to `75`-`150`.                                                                                                                                 |
| **`[UI]`**                  | `EnableAnimationTaskPool`                                                                             | `0`                  | `1` updates character animation through `AnimationTaskPool` when at least 20 characters are visible.                                                             |
| **`[Camera]`**              | `Zoom`                                                                                                | `1735`               | Persisted Orbital-camera distance.                                                                                                                               |
| **`[Render]`**              | `VSync`                                                                                               | `1`                  | `0` disables VSync across restarts and resolution changes. `$vsync on` / `$vsync off` update it.                                                                 |
| **`[Render]`**              | `FpsCap`                                                                                              | `-1`                 | Frame cap while VSync is off; `-1` is uncapped. `$fps <value>` updates it.                                                                                       |
| **`[Render]`**              | `Look`                                                                                                | `Classic`            | `Enhanced` turns on the Enhanced look ([docs](docs/enhanced-look.md)); the options window's "Enhanced graphics" checkbox and `$look` update it.                  |
| **`[Render]`**              | `Antialiasing`                                                                                        | `4`                  | Samples of the Enhanced look: `0` (off), `2`, `4`, `8`. Classic renders without. The options window and `$msaa` update it.                                       |
| **`[Render]`**              | `Anisotropy`                                                                                          | `8`                  | Anisotropic filtering (`1` is off, up to `16`) of every texture with mip levels. Set in the options window.                                                      |
| **`[Render]`**              | `EnhancedLighting`, `DynamicLights`, `LightMood`, `PostProcessing` | `1` | `0` switches that feature of the Enhanced look off; also the rows under "Enhanced graphics" in the options window. |
| **`[Render]`**              | `Backend`                                                                                             | `default`            | SDL GPU driver: `default`, `vulkan`, `direct3d12` (or `d3d12`), `metal`. Falls back to SDL's choice with a logged warning when unavailable. Edited by hand only. |
| **`[Render]`**              | `DisableEffects`, `DisableParticles`, `DisableSkillEffectModels`, `DisableBoids`, `DisableWingShadow`, `DisableBodyShadows` | `0`                  | Rendering-cost reductions for low-end PCs, also in the options window and `$effects`.                                                                            |
| **`[Render]`**              | `SortParticleDraws`                                                                                   | `0`                  | Opt-in depth sorting of particle draws.                                                                                                                          |
| **`[Render]`**              | `TexturePack`                                                                                         | empty                | Active [texture pack](docs/texture-packs.md), a folder in `Data/TexturePacks/`; empty uses the original textures. Read at startup; set in the options window.    |
| **`[ChatCommands]`**        | `Favourite<N>`, `Template<N>`                                                                         | none                 | Written by the [chat commands window](docs/chat-commands.md).                                                                                                    |

A new install starts from `src/bin/config.ini.template`. Retired keys, such as
`[Render] CoreProfile`, are removed from the file when it loads.

### Command-line flags and options

- **`connect /u<IP> /p<PORT>`**: connects to that server instead of `[CONNECTION SETTINGS]`.
- **`--enable-taskpool`**: enables `AnimationTaskPool` regardless of `config.ini`; the
  20-character threshold still applies.
- **`--editor`**: starts with the ImGui editor open in editor-enabled builds (the `-mueditor`
  presets); **F12** toggles it.

### Chat commands

Typed into the chat input, in any build:

- `$fps <value>`, `$vsync on|off`, `$fpscounter on|off`, `$details on|off`, `$glstats on|off`:
  frame timing and rendering statistics.
- `$effects on|off` and `$effects <system> on|off`, where `<system>` is `sprites`, `particles`,
  `skillmodels`, `boids`, `wingshadow`, `joints` or `wingextralayers`: turn rendering systems
  off to compare their cost.
- `$theme <name>`: switch the UI theme for this session.
- `$win <name> [full]` (`$win list` names them), `$preview <event>` (`$preview` lists them,
  `$preview off` ends one) and `$dialog`: open any window, event windows with sample data, and
  test dialogs, for UI work.

### Environment variables

Developer diagnostics, read at startup:

| Variable                                                            | Effect                                                                                                                                          |
| :------------------------------------------------------------------ | :---------------------------------------------------------------------------------------------------------------------------------------------- |
| `MU_CONTROL_SOCKET=<path>`                                          | Opens the [control socket](docs/control-socket.md) there (builds with `ENABLE_CONTROL_SOCKET`).                                                 |
| `MU_CAPTURE_FRAME=<N>`, `MU_CAPTURE_PATH=<file>`                    | Dumps the Nth presented frame as a PPM (default `mu-frame.ppm`), for headless setups.                                                           |
| `MU_RENDER_TIMING`                                                  | Logs frame timing and renderer counters every 60 frames; see [rendering parity](docs/build/rendering-parity.md).                                |
| `MU_INPUT_DIAGNOSTICS`                                              | Logs window scale metrics and input routing.                                                                                                    |
| `MU_NETWORK_DIAGNOSTICS`                                            | Logs every sent and received packet.                                                                                                            |
| `MU_D3D12_DISABLE_CULLING=1`, `MU_D3D12_DISABLE_TRIANGLE_MERGING=1` | Direct3D 12 driver only: turn culling or triangle merging off to isolate driver issues; see [rendering parity](docs/build/rendering-parity.md). |

## Documentation

- [RmlUi UI system](docs/rmlui-ui-system/README.md) - UI architecture, building
  windows, layout and scaling, theming, and reusable components.
- [GPU skinning and SDL GPU rendering](docs/GPU%20Skinning/README.md) - the
  downstream mapping for upstream Core Profile, GPU skinning, and DXP work.
- [Camera system](docs/camera-system.md) - modes, switching (F9), config,
  frustum culling, `$details` overlay, and the gameplay behaviour changes
  from the 3D camera rework.
- [Chat commands window](docs/chat-commands.md) - the commands the server
  offers (J), how their values are entered, and how favourites and
  templates are stored.
- [DevEditor](docs/dev-editor.md) - the in-game tuning UI (F12, editor-enabled
  builds), including the [Map Editor tab](docs/dev-editor.md#33-map-editor-tab)
  for terrain textures/height/attributes, world objects, and minimap capture.
- [Options window and config](docs/options-window.md) - runtime
  resolution / windowed toggle, slider rounding, and what the options
  window stores in `config.ini`.
- [Texture packs](docs/texture-packs.md) - installing, choosing and removing
  packs of replacement or 2x/4x textures in `Data/TexturePacks/`, and how to
  make one.
- [GLB monster models](docs/glb-models.md) - drawing a monster from a glTF
  binary named in `Data/Models/ModelFiles.json`: the table, the developer
  overlay, what the loader expects of a file and its log lines.
- [Build guide](docs/build/README.md) - platform-specific build notes.
- [In-game tests](docs/in-game-tests.md) - the tester next to developer
  builds that plays test scenarios with real clients against a fresh OpenMU
  test server and writes a report with a screenshot per step.
- [Control socket](docs/control-socket.md) - the developer socket through
  which scripts and the in-game tests drive the client.
- [Translation system](docs/translation-system.md) - how the .resx ->
  generated C++ accessors pipeline works, how to add a string or a locale,
  runtime locale switching, and observer hooks for cached UI strings.

## Contributing

### Coding rules

All code changes - by humans and AI assistants - should follow
[`docs/CODING_RULES.md`](docs/CODING_RULES.md). Read it before opening a PR.

AI coding assistants (Claude Code, Cursor, Codex, etc.) should also read
[`AGENTS.md`](AGENTS.md), which points at the same rules and at the build guide.

## Credits

- Webzen
- Louis
- Qubit (tuservermu.com.ve)
- Community members of RaGEZONE and tuservermu.com.ve for posting fixes
- [Nitoy](https://github.com/nitoygo) for the MU Helper and RmlUi UI System
- [janblade](https://github.com/janblade) for the work on GPU Skinning and some rendering optimizations
- [yesid](https://github.com/yesid-bocanegra) for the SDL3 and SDL_GPU porting
- [valentino](https://github.com/valentinoConti) for the Map Editor
- [StinkyFax](https://github.com/alandarev) for the Socket Control automation
- [Mosch](https://github.com/Mosch0512) for general maintenance, code quality and organizational fixes, and various feature support (such as MU Editor, data driven framework, etc)
- And to all the contributors who helped test, report issues, suggest improvements, and send PRs. 🔥
