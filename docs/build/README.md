# Build Guide

How to build the MU client on each platform, with one file per setup (terminal,
WSL, and the common IDEs). Pick the guide that matches your environment.

## Pick your setup

### Linux (native)

A native Linux build produces a real, playable client: the engine plus the
`linux-x64` network library, rendered through SDL/OpenGL.

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

CI builds every supported row: see [windows-build.yml](../../.github/workflows/windows-build.yml)
and [linux-build.yml](../../.github/workflows/linux-build.yml).

> **Why no 32-bit Linux?** .NET has no 32-bit Linux runtime at all - `dotnet
> publish -r linux-x86` fails with `NETSDK1203` ("AOT is not supported"). A
> 32-bit Linux client could compile but could never load the network library,
> so it cannot connect to a server. 32-bit *Windows* is unaffected (`win-x86`
> AOT is supported).

## Common concepts

These apply to every setup; the per-setup guides only cover what differs.

### Build options

| CMake option | Values | Meaning |
|--------------|--------|---------|
| `CMAKE_BUILD_TYPE` | `Release` / `Debug` | Optimized vs. debuggable. |
| `ENABLE_EDITOR` | `ON` / `OFF` | Builds the in-app ImGui editor (admin tooling). When `OFF`, nothing under `src/MuEditor/` is compiled in - enforced by the `editor_leak` test. |
| `BUILD_TESTING` | `ON` / `OFF` | Builds and registers the unit tests (run with `ctest`). |

### The network library

The client talks to the server through OpenMU's `MUnique.Client.Library`, a C#
project compiled to a native shared library with .NET Native AOT
(`ClientLibrary/`). The C++ client loads it at runtime (`dlopen`/`LoadLibrary`),
so `Main` links without it, but it must sit next to the executable to connect.
The build produces it automatically and copies it next to `Main`:

- Linux target -> `MUnique.Client.Library.so`
- Windows target -> `MUnique.Client.Library.dll`

Native AOT cannot cross-compile across OS boundaries: a Linux-hosted SDK cannot
produce a Windows `.dll`, and vice versa. So a MinGW-on-Linux build (and CI)
compiles and links the client but does **not** produce the `.dll`; build the
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
