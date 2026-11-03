# Build Guide

How to build the MU client on each platform, with one file per setup (terminal,
WSL, and the common IDEs). Pick the guide that matches your environment.

Desktop rendering uses SDL GPU: Metal on macOS, Vulkan on Linux, D3D12 on
Windows. The application policy is shared; see the
[rendering parity guide](rendering-parity.md) for diagnostics and native
verification requirements.

## Pick your setup

### Linux (native)

A native Linux build produces a real, playable client: the engine plus the
`linux-x64` network library, rendered through SDL GPU/Vulkan.

| Setup | Guide |
|-------|-------|
| Terminal / command line | [linux/console.md](linux/console.md) |
| CLion | [linux/clion.md](linux/clion.md) |
| Rider | [linux/rider.md](linux/rider.md) |
| VS Code | [linux/vscode.md](linux/vscode.md) |

### Windows

The Windows client can be built natively on Windows, or cross-compiled from
Linux/WSL with MinGW-w64.

| Setup | Guide |
|-------|-------|
| Visual Studio | [windows/visual-studio.md](windows/visual-studio.md) |
| Terminal (CMake presets) | [windows/console.md](windows/console.md) |
| WSL + MinGW (cross-compile) | [windows/wsl.md](windows/wsl.md) |
| CLion | [windows/clion.md](windows/clion.md) |
| Rider | [windows/rider.md](windows/rider.md) |
| VS Code | [windows/vscode.md](windows/vscode.md) |

### Planned

`android/` and `iOS/` are placeholders for future ports; the engine is not yet
buildable for them. See the per-platform notes when that work lands.

## Supported targets

| OS | Arch | Editor | Network library | Status |
|----|------|--------|-----------------|--------|
| Linux | x64 | on / off | `MUnique.Client.Library.so` (linux-x64 AOT) | Full client |
| Windows | x64 | on / off | `MUnique.Client.Library.dll` (win-x64 AOT) | Full client |
| Windows | x86 | on / off | `MUnique.Client.Library.dll` (win-x86 AOT) | Full client |
| Linux | x86 | - | none | Not supported (see below) |

Actions validate Windows native x64 Release, Linux x64 Release, and macOS arm64
Release, all with the editor OFF, and upload one no-data runtime artifact per OS.
Build every other supported row locally using the setup guides above.

> **Why no 32-bit Linux?** .NET has no 32-bit Linux runtime at all - `dotnet
> publish -r linux-x86` fails with `NETSDK1203` ("AOT is not supported"). A
> 32-bit Linux client could compile but could never load the network library,
> so it cannot connect to a server. 32-bit *Windows* is unaffected (`win-x86`
> AOT is supported).

## Common concepts

These apply to every setup; the per-setup guides only cover what differs.

### Dependencies and first configuration

Use CMake 3.25 or newer, the .NET 10 SDK, and the compiler and system packages listed
in your platform guide. Windows presets also require vcpkg with `VCPKG_ROOT` set.
Initial configuration needs network access unless dependency sources are already cached.

CMake initializes missing Git submodules under `src/ThirdParty/`: SDL3 and RmlUi for
all builds, and ImGui when `ENABLE_EDITOR=ON`. RmlUi uses the
[nitoygo/RmlUi fork](https://github.com/nitoygo/RmlUi), with the revision pinned by
the repository's submodule entry, for the SDL GPU renderer integration.

If automatic initialization fails, run from the repository root:

```sh
git submodule update --init -- src/ThirdParty/SDL src/ThirdParty/RmlUi

# For editor builds:
git submodule update --init -- src/ThirdParty/imgui
```

CMake downloads miniaudio, GLM, spdlog, SDL3_ttf and SDL3_image through `FetchContent`.
Audio uses miniaudio. SDL3_ttf supplies the vendored FreeType used by RmlUi; neither
SDL_mixer nor a separate FreeType submodule is required.

### Build options

| CMake option | Values | Meaning |
|--------------|--------|---------|
| `CMAKE_BUILD_TYPE` | `Release` / `Debug` / `RelWithDebInfo` | Build configuration for single-config generators. With Ninja Multi-Config, select it through a build preset or `--config` instead. |
| `ENABLE_EDITOR` | `ON` / `OFF` | Builds the in-app ImGui editor (admin tooling). When `OFF`, nothing under `src/MuEditor/` is compiled in - enforced by the `editor_leak` test. |
| `ENABLE_CONTROL_SOCKET` | `ON` / `OFF` | Builds the developer control socket that lets test scripts drive the client (see [`control-socket.md`](../control-socket.md)). Default `OFF`; never for player builds. When `OFF`, nothing under `src/source/App/Control/` and no local-socket transport is compiled in, and the `MU_CONTROL_SOCKET` variable is ignored - enforced by the `control_socket_leak` test. The `-mueditor` and `-in-game-tests` presets turn it `ON`, and so does `ENABLE_IN_GAME_TESTS`. |
| `ENABLE_IN_GAME_TESTS` | `ON` / `OFF` | Publishes the in-game tester next to `Main` and adds the `InGameTests` / `InGameTestsGui` targets (see [`in-game-tests.md`](../in-game-tests.md)); both need the .NET SDK, like the client's network library. Default `OFF`; while `ON`, it turns `ENABLE_CONTROL_SOCKET` on too, which the tester drives the client through. The `-mueditor` and `-in-game-tests` presets turn it `ON`, the player presets `OFF`. |
| `BUILD_TESTING` | `ON` / `OFF` | Builds and registers the unit tests (run with `ctest`). |
| `RMLUI_SDL_GPU_NUM_MSAA_SAMPLES` | `1` / `2` / `4` / `8` | RmlUi UI-layer sample count. Default `1` disables MSAA to avoid reported NVIDIA/Vulkan device loss. Higher values are opt-in for diagnostics; see [rendering parity](rendering-parity.md#rmlui-multisampling). |
| `MU_COPY_RUNTIME_ASSETS` | `ON` / `OFF` | Copies `Data/` and `fonts/` beside the executable. Defaults to `ON` for local runnable builds. |
| `MU_LINK_SDL_PLATFORM_BACKENDS` | `ON` / `OFF` | Linux only: makes SDL link its video-backend libraries (X11, Wayland, libdecor, KMS/DRM) at build time instead of loading them at run time, and links the Vulkan loader into `Main` (SDL still loads that one at run time). A packaged runtime then finds all of them through `Main`'s recorded `RUNPATH`, without a global library path. Defaults to `OFF`. |

For Linux Wayland reports, run [`scripts/wayland-diag.sh`](../../scripts/wayland-diag.sh) and attach its sanitized `summary.txt`.

If configuration reports that SDL_ttf or RmlUi cannot find FreeType after removing
cached dependencies, reconfigure the same build directory with
`-DFETCHCONTENT_FULLY_DISCONNECTED=OFF`. SDL_ttf supplies the vendored FreeType;
disconnected mode requires its sources to already exist. Let configuration finish
successfully before building or running the tests.

### The network library

The client talks to the server through OpenMU's `MUnique.Client.Library`, a C#
project compiled to a native shared library with .NET Native AOT
(`ClientLibrary/`). The C++ client loads it at runtime (`dlopen`/`LoadLibrary`),
so `Main` links without it, but it must sit next to the executable to connect.
The build produces it automatically and copies it next to `Main`:

- Linux target -> `MUnique.Client.Library.so`
- Windows target -> `MUnique.Client.Library.dll`

Native AOT cannot cross-compile across OS boundaries: a Linux-hosted SDK cannot
produce a Windows `.dll`, and vice versa. A MinGW-on-Linux build compiles and
links the client but does **not** produce the `.dll`; build the
Windows client on Windows to get a connectable one.

### Output layout

A post-build step copies the game assets (from `src/bin/`) and the network
library next to the executable, so the build output directory is runnable:

```
<build-dir>/src/
  Main (or Main.exe)
  MUnique.Client.Library.so (or .dll)
  Data/ ...                          <-- copied assets
  config.ini                         <-- seeded from config.ini.template
```

Run the client from that directory so it finds its assets and the library.

## Hosted releases

Feature branches target `main`. Every PR runs Quality Gates, which include the
[static checks](reference-tests.md#static-checks-and-the-pre-push-hook); ready code
PRs then run Linux x64 and macOS arm64 Release builds and tests. Draft PRs defer native builds until
**Ready for review**; documentation-only changes run the cheap checks without
native compilation. Mixed changes, build files, shaders, tests and asset PRs still
get native builds; asset tests load game data and render shipped documents.
Code merges to main run Linux integration checks. Newer revisions cancel older
PR/integration runs. Releases finish the running revision and keep only the newest
pending trigger; another nightly/manual trigger can replace an older pending one.
This deliberately coalesces work rather than guaranteeing a run for every request.

Full Windows/Linux/macOS release builds run daily at approximately **02:17 UTC+8**
(18:17 UTC) and on demand. In **Actions > CI > Run workflow**, select **main** to
force a full build without waiting for the nightly run. Before scheduled builds,
CI scans commits since the latest reachable version tag and skips all native jobs
when no release-producing change exists. It reads the configured release types,
checks scoped subjects and breaking-change notes, and builds conservatively for
an initial release or unsupported release rules. Manual dispatch bypasses this skip.
All jobs use the same revision selected when the workflow is triggered. Semantic
Release publishes only when those commits warrant a version change; main merges
no longer publish immediately. Windows integration failures may first appear in
the nightly run. The separate Data Assets workflow still publishes data on merge.
Publishing also waits for **CI Required**. If no release is published and main has
advanced during the build, the release job fails with a manual-rerun instruction
instead of silently reporting success. It never substitutes untested newer code.

### Required PR check (maintainer setup)

After merging this workflow change, an administrator must edit the existing main
branch ruleset in **Settings > Rules > Rulesets**, enable **Require status checks
to pass**, and require **CI Required** from GitHub Actions. Preserve the existing
pull-request, deletion and force-push protections. Repository files cannot enable
this GitHub setting. If the check is not listed yet, run a PR with the new workflow
first, then select it.

`CI Required` verifies Quality Gates and every native job selected for the change.
It rejects failures, cancellations and unexpected skips, while allowing the
intentional native-build skips above. Require this aggregate check instead of
individual platform jobs, which do not run on every event. Do not enable **Require
branches to be up to date** just to reduce builds: it can force another build after
each main merge. Update a PR when its integration state needs revalidation.

Linux uses a compiler-cache trial. Its job log and **Linux compiler cache** step
summary show hits and misses; compare the Build step duration on a warm run with
the first run. All builds restore the compiler cache; only successful main pushes
save it, so PR-only entries do not displace shared caches. macOS uses the same
compiler cache (**macOS compiler cache** summary); main pushes skip macOS, so
successful nightly/manual release builds save it instead. Windows vcpkg primary
keys include the runner image, compiler, vcpkg revision, manifest and toolchain.
A broader fallback also restores older binary archives; vcpkg checks compatibility
before reusing packages across image or compiler updates.

### Download artifacts

Actions expose builds through CI artifacts and two coordinated release streams:

- Semantic Release publishes these no-data runtime archives:
  `MuMain-windows-native-x64-release-editor-off-no-data.tar.gz`,
  `MuMain-linux-native-x64-release-editor-off-no-data.tar.gz`, and
  `MuMain-macos-native-arm64-release-editor-off-no-data.tar.gz`.
- Each nightly/manual release build also uploads
  `mu-client-windows-native-x64-release-editor-off-no-data-main`,
  `mu-client-linux-native-x64-release-editor-off-no-data-main`, and
  `mu-client-macos-native-arm64-release-editor-off-no-data-main` artifacts;
  each contains its corresponding compressed runtime archive.
- The data workflow publishes `MuMain-data-<id>.tar.gz` and its
  `.sha256` file under tag `data-<id>`. The identifier is derived from the Git
  trees for `src/bin/Data/` and `src/bin/fonts/`, so unchanged data is reused
  across code releases.
- Each client release includes a **Compatible game data** link to its exact
  `data-<id>` release.

Windows x86, Debug, editor-ON, MinGW, and other configurations are not built by
Actions; build locally when one is required.

### Assemble a release

Download the no-data runtime for your OS plus the linked `data-<id>` archive and
checksum into one working directory. On Windows through Git Bash:

```bash
mkdir MuMain-runtime
tar -xzf MuMain-windows-native-x64-release-editor-off-no-data.tar.gz -C MuMain-runtime
sha256sum --check MuMain-data-<id>.tar.gz.sha256
tar -xzf MuMain-data-<id>.tar.gz -C MuMain-runtime
```

For Linux, use the same commands with
`MuMain-linux-native-x64-release-editor-off-no-data.tar.gz`.

On macOS, place the data beside the executable inside the application bundle:

```bash
mkdir MuMain-runtime
tar -xzf MuMain-macos-native-arm64-release-editor-off-no-data.tar.gz -C MuMain-runtime
sha256sum --check MuMain-data-<id>.tar.gz.sha256
tar -xzf MuMain-data-<id>.tar.gz -C MuMain-runtime/Main.app/Contents/MacOS
```

On Windows PowerShell:

```powershell
New-Item -ItemType Directory MuMain-runtime
tar -xzf MuMain-windows-native-x64-release-editor-off-no-data.tar.gz -C MuMain-runtime
$expected = (Get-Content MuMain-data-<id>.tar.gz.sha256).Split()[0].ToUpperInvariant()
$actual = (Get-FileHash MuMain-data-<id>.tar.gz -Algorithm SHA256).Hash
if ($actual -ne $expected) { throw "Data archive checksum mismatch." }
tar -xzf MuMain-data-<id>.tar.gz -C MuMain-runtime
```

The assembled directory contains `Main.exe`, the network and runtime DLLs,
configuration, shaders, `Data/`, and `fonts/`.

If a music file is missing or cannot be decoded, the client logs the failure and leaves
that track silent for the rest of the audio session. Scene changes do not repeatedly
retry it. After restoring or replacing a file under `Data/Music/`, restart the client
to load it again.

### Build without copying data

Local builds copy `Data/` and `fonts/` by default. Packaging or compile-only
builds can skip them:

```bash
cmake --preset windows-x64 -DMU_COPY_RUNTIME_ASSETS=OFF
cmake --build --preset windows-x64-release
```

Switching the option to `OFF` also removes stale `Data/` and `fonts/` from
the selected build output.

## Reference

### NuGet cache

Defaults to `<repo>/.nuget` on all platforms (kept in-repo, already
`.gitignore`d), which avoids failures when the user profile path contains
special characters (e.g. umlauts break the AOT compiler `ilc`). Override with
`-DMU_NUGET_CACHE_DIR=<path>`.

### How dotnet is found

The build picks the SDK that can target the current OS:

- **Linux target:** prefers native `dotnet` (only it can AOT-compile the
  `linux-x64` `.so`), falling back to `dotnet.exe`.
- **Windows target (incl. WSL/MinGW):** prefers `dotnet.exe`, falling back to
  native `dotnet`.

If no SDK is found, CMake skips the network-library build with a warning; the
client still compiles and links.

### Tests

Unit tests live under `tests/` and use [doctest](https://github.com/doctest/doctest).
Enable with `-DBUILD_TESTING=ON`, then `ctest --test-dir <build-dir>
--output-on-failure`. On a native Linux build they run directly; for
MinGW/Windows binaries on a Linux host, CTest runs them through `wine`. See
[the test reference](reference-tests.md) for adding a module.
