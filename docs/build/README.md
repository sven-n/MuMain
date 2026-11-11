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

A macOS build runs on the macOS version of the Mac that built it and newer: the
Homebrew libraries it links (OpenSSL, libjpeg-turbo, Crypto++) are built for that
version. The CI artifact needs macOS 26, which every Apple silicon Mac can install.
CI builds it on the `macos-26` image by name, so that minimum changes only when the
workflow names a newer image, not when GitHub moves `macos-latest`.

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
When a change in `CMakeLists.txt` changes how a library is downloaded (its version, URL
or archive), the next configure downloads it again, in an existing build folder too.
Offline, configure with `-DFETCHCONTENT_FULLY_DISCONNECTED=ON` to keep using the copy
that is already there.

CMake initializes missing Git submodules under `src/ThirdParty/`: SDL3 and RmlUi for
all builds, and ImGui when `ENABLE_EDITOR=ON`. RmlUi uses the
[nitoygo/RmlUi fork](https://github.com/nitoygo/RmlUi), with the revision pinned by
the repository's submodule entry, for the SDL GPU renderer integration.

CMake fetches the pinned commits without their history; git also brings the newest
commit of each submodule's default branch. That is about 30 MB instead of 300 MB in
every checkout and worktree. A submodule that already has its history keeps it.
To work on a submodule with its history and all its branches, fetch them there,
e.g. for RmlUi:

```sh
git -C src/ThirdParty/RmlUi remote set-branches origin "*"
git -C src/ThirdParty/RmlUi fetch --unshallow
```

A clone can bring the submodules the same way from the start; `--depth 1` also
leaves out MuMain's own history, for a checkout that only builds:

```sh
git clone --recurse-submodules --shallow-submodules https://github.com/sven-n/MuMain.git
```

If automatic initialization fails, run from the repository root:

```sh
git submodule update --init --depth 1 -- src/ThirdParty/SDL src/ThirdParty/RmlUi

# For editor builds:
git submodule update --init --depth 1 -- src/ThirdParty/imgui
```

A git server that sends only branch tips refuses `--depth 1` here for a pinned commit
that is none, as SDL's and ImGui's are. Without `--depth 1`, git fetches the whole
history; CMake falls back to that by itself.

CMake downloads miniaudio, GLM, spdlog, SDL3_ttf and SDL3_image through `FetchContent`.
Audio uses miniaudio. SDL3_ttf supplies the vendored FreeType used by RmlUi; neither
SDL_mixer nor a separate FreeType submodule is required.

The libraries that come as archives unpack with the time of unpacking, not the
archives' own older dates, so the build after a version change compiles the new
sources. Each `FetchContent_Declare` with a URL sets `DOWNLOAD_EXTRACT_TIMESTAMP FALSE`
for it, and `cmake/UnpackArchive.cmake`, which unpacks SDL3_ttf's libraries, passes
`TOUCH`; the `archive_extract_times_contract` static check keeps both.

SDL3_image comes as its release archive, without the libraries it can vendor (libpng,
libjpeg, zlib, libtiff, libwebp, libjxl, libavif): PNG and JPEG load through
stb_image, and the formats that would need one of them are off. Turning such a format
on also means fetching SDL_image with that library; the comment above
`FetchContent_Declare(SDL3_image)` in `CMakeLists.txt` says how, and the
`sdl_image_archive_contract` static check names what is missing.

SDL3_ttf comes as its release archive too, and the four libraries it vendors
(FreeType, HarfBuzz, plutosvg, plutovg) as archives of the commits SDL3_ttf pins
them to, each checked by its SHA-256. SDL3_ttf's patch step unpacks them into its
`external/` folder before the patches in `cmake/patches/` apply; the
`sdl_ttf_archives_contract` static check keeps each one there. Updating SDL3_ttf
means updating those four commits and hashes as well, and the list of the
release's commits in `tests/test_sdl_ttf_archives.cmake`, which the check
compares them with; the comment above `FetchContent_Declare(SDL3_ttf)` says
where to find them.

Each run of that patch step starts from the archives' sources: it unpacks
SDL3_ttf's own archive, kept in the sub-build's download folder, again first.
Each patch's SHA-256 is part of its patch command, so an edited patch in
`cmake/patches/` runs the step again, also in an existing build folder and in
CI's restored library caches, and applies to the unpatched sources.

### Build options

| CMake option | Values | Meaning |
|--------------|--------|---------|
| `CMAKE_BUILD_TYPE` | `Release` / `Debug` / `RelWithDebInfo` | Build configuration for single-config generators. With Ninja Multi-Config, select it through a build preset or `--config` instead. |
| `ENABLE_EDITOR` | `ON` / `OFF` | Builds the in-app ImGui editor (admin tooling). When `OFF`, nothing under `src/MuEditor/` is compiled in - enforced by the `editor_leak` test. |
| `ENABLE_CONTROL_SOCKET` | `ON` / `OFF` | Builds the developer control socket that lets test scripts drive the client (see [`control-socket.md`](../control-socket.md)). Default `OFF`; never for player builds. When `OFF`, nothing under `src/source/App/Control/` and no local-socket transport is compiled in, and the `MU_CONTROL_SOCKET` variable is ignored - enforced by the `control_socket_leak` test. The `-mueditor` and `-in-game-tests` presets turn it `ON`, and so does `ENABLE_IN_GAME_TESTS`. |
| `ENABLE_IN_GAME_TESTS` | `ON` / `OFF` | Publishes the in-game tester next to `Main` and adds the `InGameTests` / `InGameTestsGui` targets (see [`in-game-tests.md`](../in-game-tests.md)); both need the .NET SDK, like the client's network library. Default `OFF`; while `ON`, it turns `ENABLE_CONTROL_SOCKET` on too, which the tester drives the client through. The `-mueditor` and `-in-game-tests` presets turn it `ON`, the player presets `OFF`. |
| `BUILD_TESTING` | `ON` / `OFF` | Builds and registers the unit tests (run with `ctest`). |
| `CMAKE_DISABLE_PRECOMPILE_HEADERS` | `ON` / `OFF` | `ON` compiles without precompiled headers: the game's `stdafx.h` (also in the tests), SDL's and RmlUi's. Default `OFF`: the game's files that include `stdafx.h` share one precompiled copy of it, so a cold build of the game library takes about half the time on Windows and 30% less on Linux. The test files that include it, in tests that link `MuClient`, share another, so a cold build of the tests takes about 28% less time on Windows and 19% less on Linux. Which files include it is checked at configure time: a file that adds or drops the include follows after the next configure. CI's PR and main push builds turn it `ON`, so their compiler cache covers every file (see [Hosted releases](#hosted-releases)). |
| `RMLUI_SDL_GPU_NUM_MSAA_SAMPLES` | `1` / `2` / `4` / `8` | RmlUi UI-layer sample count. Default `1` disables MSAA to avoid reported NVIDIA/Vulkan device loss. Higher values are opt-in for diagnostics; see [rendering parity](rendering-parity.md#rmlui-multisampling). |
| `MU_COPY_RUNTIME_ASSETS` | `ON` / `OFF` | Copies `Data/` and `fonts/` beside the executable. Defaults to `ON` for local runnable builds. |
| `MU_WARNINGS_AS_ERRORS` | `ON` / `OFF` | Turns compiler, linker and DXC shader warnings into errors in the game's own code and its tests; the third-party libraries keep their own settings. Linker warnings need CMake 4.0 or newer (an older CMake says so and keeps them warnings). Default `OFF`; CI's player builds turn it `ON` (see [No warnings](#no-warnings)). |
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
PRs then run Windows x64, Linux x64 and macOS arm64 Release builds and tests. Draft PRs defer native builds until
**Ready for review**; documentation-only changes run the cheap checks without
native compilation. Mixed changes, build files, shaders, tests and asset PRs still
get native builds; asset tests load game data and render shipped documents.
Code merges to main run the same three builds, which save the caches the next PRs
start from (see the CI caches below). A main push compares against the commit main's
last successful run built, not the previous push: a push still waiting for its run
is replaced by a newer one, and would otherwise never be built. Without such a run,
everything builds. A PR's newer revision cancels its older run.
Main pushes and releases finish the running revision and keep only the newest
pending trigger, so a main push's caches get saved; a newer push or nightly/manual
trigger can replace an older pending one.
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
no longer publish immediately. The separate Data Assets workflow still publishes data on merge.
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

### No warnings

The three player builds (Windows, Linux and macOS, editor and control socket OFF)
configure and build without a single warning, note or deprecation message, and CI
fails as soon as one comes back:

- The game's own code, its tests and the DXC shader compiles build with
  `MU_WARNINGS_AS_ERRORS=ON`, so a compiler or linker warning stops the build where
  it happens. Configure with `-DMU_WARNINGS_AS_ERRORS=ON` to get the same locally.
- [`scripts/ci_warnings.py`](../../scripts/ci_warnings.py) runs the configure and
  build commands and fails the step for any other such line in their output: CMake
  and the libraries' CMake scripts, vcpkg's notes, glslang, wayland-scanner and the
  .NET build. `python3 scripts/ci_warnings.py scan <job log>` lists them in a
  downloaded log.
- **CI Required** (and Fork CI's last job) fails when a job of the run ends with a
  warning annotation, such as GitHub's notice that an action targets a deprecated
  Node.js version. Notices, e.g. about runner queue times, are listed only, and so
  are the warnings actions/cache logs when GitHub's cache service is busy, down or
  unreachable ("Failed to restore: Cache service responded with 503", timeouts): the
  build goes on without the cache. Its other warnings, such as a cache over the size
  limit, fail the check.

The libraries built from source (SDL3_ttf, SDL3_image, FreeType, HarfBuzz) compile
with their warnings turned off (`-w`, `/w`), and miniaudio and glm are included as
system headers: their warnings are for their authors to fix. A new compiler or CMake on a runner image can bring new warnings
without any change here; fix them in a PR like any other failure. The jobs name
their image (`ubuntu-26.04`, `macos-26`, `windows-2025-vs2026`) instead of a
`-latest` label: GitHub moves repositories to a new latest image one by one and
warns about the move in every job meanwhile.

All three builds use a compiler cache (ccache). The job logs and the **Windows
compiler cache**, **Linux compiler cache** and **macOS compiler cache** step
summaries show hits and misses; compare the Build step duration on a warm run with
the first run. Windows downloads a pinned ccache release and checks it against its
SHA-256 (**Install compiler cache**). Before a compiler cache is saved, **Trim
compiler cache** removes the objects the build did not use (ccache marks each object
it finds), so an entry holds one build's objects, not every build's since the cache
began: about 0.5 GB on Windows, whose objects carry their debug info (`/Z7`). PR
and main push builds configure with `-DCMAKE_DISABLE_PRECOMPILE_HEADERS=ON`: SDL,
RmlUi and the game (`stdafx.h`) use precompiled headers, which ccache does not
cache, so their files would compile again on every run (SDL and RmlUi alone are over
a quarter of each build: 503 of 1666 compile calls on Linux, 421 of 1584 on macOS).
The nightly and manual release builds keep the precompiled headers, as local builds
do, so a change that breaks only with them fails there; those builds save no cache,
so SDL, RmlUi and the game files that include `stdafx.h` compile without one. All
builds restore the compiler cache; only
successful main pushes save it, so PR-only entries do not displace shared caches,
and a PR starts from the latest main. Windows vcpkg primary
keys include the runner image, compiler, vcpkg revision, manifest and toolchain.
A broader fallback also restores older binary archives; vcpkg checks compatibility
before reusing packages across image or compiler updates.

The library caches keep the downloaded library sources (`_deps/*-src`,
`_deps/*-subbuild`) and the NuGet packages, not the libraries' build folders: ninja
recompiles restored build folders anyway. Their keys hash the root `CMakeLists.txt`
(which declares the libraries), `cmake/patches`, the scripts that unpack and patch the
sources (`cmake/UnpackArchive.cmake`, `cmake/ApplyGitPatch.cmake`) and the ClientLibrary
projects, and name the .NET SDK version (which picks the NativeAOT compiler packages in `.nuget`),
so a new entry is stored only when one of those changes, not with every new source
file in `src/CMakeLists.txt`.

The Linux build also caches the apt packages it installs (about 45 MB): the image's
Ubuntu mirror sometimes sends them at 35–160 kB/s, and **Install build tools** then
took 5–24 minutes instead of about 20 seconds. **Find apt packages** updates the
index and names the cache after the runner image and the names, sizes and hashes of
the files apt would fetch (`apt-get install --print-uris`), so a new package version
gives a new key and another mirror for the same files does not. Which files apt
fetches depends on what the image has installed: **Restore apt packages** reads the
image's newest entry first, then any image's. **Fetch apt packages**
downloads them (`--download-only`): apt uses the restored files that still match the
index and fetches only the others. In the runs that save the cache, **Gather apt
packages** then collects the listed files for **Save apt packages**
([`scripts/ci_apt_cache.py`](../../scripts/ci_apt_cache.py)), before **Install build
tools** installs them, so no clean-up after dpkg can remove them first. When one is
missing, the save stores nothing and a notice names it. Quality Gates gets cppcheck,
which the image lacks, from an apt cache of its own the same way, and only in runs
that change C++ files: only its formatting check and static analysis use cppcheck
and clang-format, and they skip the other runs.

PR runs only read the library, vcpkg and apt caches, as they do the compiler caches:
**Restore dependencies**, **Restore vcpkg packages** and **Restore apt packages** read
them, and **Save dependencies** (after the build), **Save vcpkg packages** (after the
configure step) and **Save apt packages** (after the download) store them only in main
pushes, when the exact key was missing. A
PR's own entry would serve that PR alone, and the repository's caches share 10 GB:
past that, GitHub deletes the least recently used entries, main's included. The
nightly and manual release runs save nothing either: they run beside the pushes,
so with both saving, an older commit's entry could be saved last and be the one the
prune job keeps.

A save under a new key leaves the older entries of that cache behind, such as a
compiler cache for each main push. After the builds of a run on main, the
**Prune caches** job ([`scripts/ci_cache_prune.py`](../../scripts/ci_cache_prune.py))
deletes all but the newest entry of each cache on main. The compiler caches keep
the newest entry of each compiler: a job restores only entries of its own compiler,
and while GitHub moves the runners to a new image, jobs land on both images. The apt
caches keep the newest entry of each image for the same reason. An entry a job read
in the last 15 minutes is left for a later run, as that job may still be downloading
it, unless the last read was the run's own: each build job, and Quality Gates for its
cppcheck, notes when its restores ended (**Note when the caches were read**), so the
entry a job started from goes in the same run. Prune caches is the only job with a
token that can delete caches.
CI Required waits for it: a prune that fails or warns fails the run like a build,
since without it the caches fill up again. A new cache that is restored by a
key prefix needs its prefix in the script's `KINDS`, and in `PER_COMPILER` when that
prefix names the compiler; the workflow contract test checks that.

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

### Which client a log came from

`MuError.log` starts with the client's identity, so a bug report shows what it ran:

```text
Release       : v1.37.0
Build commit  : <the commit the client was built from>
Build         : Release NoEditor MSVC x64, built 2026-11-06 13:02:11 UTC
Data          : data-<id> (matches this client)
```

- **Release** comes from `release-version.txt`, which the release job adds to each
  runtime archive. Self-built clients and CI artifacts say `none`.
- **Build commit** ends in `(with local changes)` when tracked files differed from
  the commit. `https://github.com/sven-n/MuMain/compare/<commit>...main` lists what
  main has gained since.
- **Build** names the configuration, editor, compiler and architecture, and when the
  executable was written (linked, or in a download the time the archive keeps), in UTC.
- **Data** compares `Data/DataVersion.txt`, which the data workflow adds to the data
  archive, with the data release the client was built for. A different id means the
  runtime and data downloads don't belong together; data archives from before the
  file, and self-built clients, say `unknown`.
- Either line says `unreadable` when its file is there but holds no version, for
  example after someone edited it.

The log goes on with the OS, CPU and RAM, the `ClientLibrary` line, the data language
(`Eng`, `Por` or `Spn`, which picks the `Data/Local` files), and, once the renderer is
up, the GPU, its graphics API and driver version, and the UI language the player reads
(`en`, `de`, `zh-CN`, ...) with whether item names are translated. A change later adds
a line: `> UI language changed to ...` for a switch in the options window or the editor,
`> Item names now translated.` or `> Item names now in English.` for the options window's
item name setting.
`ClientLibrary` names the commit the .NET library was built from and its OpenMU
packets version. In a release download that commit is the client's, so a different
one means a library from another download. A local build rebuilds the library only
when its sources change, so there it can name an older commit.

The log names the connect server (`> Server = ..., from config.ini` or `from the
command line`) and every server the client connects to (`NET: connecting to ...`).
Players post their logs in public, so an address is written out only when it can't
lead to a server:

| Address | Written as |
| --- | --- |
| The local machine: `127.0.0.0`–`127.255.255.255`, `0.0.0.0`–`0.255.255.255`, `localhost` and `*.localhost`, `::1`, `::` | as configured, e.g. `127.127.127.127:44405` |
| A private network: `10.x`, `172.16.x`–`172.31.x`, `192.168.x`, `169.254.x`, `100.64.x`–`100.127.x`, `fc00::/7`, `fe80::/10` | as configured, with `(private network)` |
| Any other IP address | `a public IPv4 address:44405` or `a public IPv6 address:44405` |
| A host name, e.g. `mu.example.com` | `a host name:44405` |

An IPv4 address in IPv6 form, e.g. `::ffff:192.168.1.20`, counts as that IPv4 address.

A failed connection logs the socket error (`SocketException ConnectionRefused (10061)`)
instead of the exception text, which would name the address.

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
